/*
 * drivers/misc/es305.c - Audience ES305 Voice Processor driver
 *
 * Copyright (C) 2012 Google, Inc.
 * Copyright (C) 2012 Samsung Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/platform_data/es305.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define ES305_FIRMWARE_NAME		"es305_fw.bin"
#define MAX_CMD_SEND_SIZE		32
#define RETRY_COUNT			5

/* ES305 commands and values */
#define ES305_BOOT			0x0001
#define ES305_BOOT_ACK			0x01

#define ES305_SYNC_POLLING		0x80000000

#define ES305_RESET_IMMEDIATE		0x80020000

#define ES305_SET_POWER_STATE_SLEEP	0x80100001

#define ES305_GET_ALGORITHM_PARM	0x80160000
#define ES305_SET_ALGORITHM_PARM_ID	0x80170000
#define ES305_SET_ALGORITHM_PARM	0x80180000
#define ES305_AEC_MODE			0x0003
#define ES305_TX_AGC			0x0004
#define ES305_RX_AGC			0x0028
#define ES305_TX_NS_LEVEL		0x004B
#define ES305_RX_NS_LEVEL		0x004C
#define ES305_ALGORITHM_RESET		0x001C

#define ES305_GET_VOICE_PROCESSING	0x80430000
#define ES305_SET_VOICE_PROCESSING	0x801C0000

#define ES305_GET_AUDIO_ROUTING		0x80270000
#define ES305_SET_AUDIO_ROUTING		0x80260000

#define ES305_SET_PRESET		0x80310000

#define ES305_GET_MIC_SAMPLE_RATE	0x80500000
#define ES305_SET_MIC_SAMPLE_RATE	0x80510000
#define ES305_8KHZ			0x0008
#define ES305_16KHZ			0x000A
#define ES305_48KHZ			0x0030

#define ES305_DIGITAL_PASSTHROUGH	0x80520000

struct es305_data {
	struct es305_platform_data	*pdata;
	struct device			*dev;
	struct i2c_client		*client;
	const struct firmware		*fw;
	struct mutex			lock;
	u32				passthrough;
	bool				passthrough_on;
	bool				asleep;
	bool				device_ready;
};

static int es305_send_cmd(struct es305_data *es305, u32 command, u16 *response)
{
	u8 send[4];
	u8 recv[4];
	int ret = 0;
	int retry = RETRY_COUNT;

	send[0] = (command >> 24) & 0xff;
	send[1] = (command >> 16) & 0xff;
	send[2] = (command >> 8) & 0xff;
	send[3] = command & 0xff;

	ret = i2c_master_send(es305->client, send, 4);
	if (ret < 0) {
		dev_err(es305->dev, "i2c_master_send failed\n");
		return ret;
	}

	/* The sleep command cannot be acked before the device goes to sleep */
	if (command == ES305_SET_POWER_STATE_SLEEP)
		return ret;

	usleep_range(1000, 2000);
	while (retry--) {
		ret = i2c_master_recv(es305->client, recv, 4);
		if (ret < 0) {
			dev_err(es305->dev, "i2c_master_recv failed\n");
			return ret;
		}
		/*
		 * Check that the first two bytes of the response match
		 * (the ack is in those bytes)
		 */
		if ((send[0] == recv[0]) && (send[1] == recv[1])) {
			if (response)
				*response = (recv[2] << 8) | recv[3];
			ret = 0;
			break;
		} else {
			dev_err(es305->dev,
				"incorrect ack (got 0x%.2x%.2x)\n",
				recv[0], recv[1]);
			ret = -EINVAL;
		}

		/* Wait before polling again */
		if (retry > 0)
			msleep(20);
	}

	return ret;
}

static int es305_load_firmware(struct es305_data *es305)
{
	int ret = 0;
	const u8 *i2c_cmds;
	int size;

	i2c_cmds = es305->fw->data;
	size = es305->fw->size;

	while (size > 0) {
		ret = i2c_master_send(es305->client, i2c_cmds,
				min(size, MAX_CMD_SEND_SIZE));
		if (ret < 0) {
			dev_err(es305->dev, "i2c_master_send failed\n");
			break;
		}
		size -= MAX_CMD_SEND_SIZE;
		i2c_cmds += MAX_CMD_SEND_SIZE;
	}

	return ret;
}

static int es305_reset(struct es305_data *es305)
{
	int ret = 0;
	struct es305_platform_data *pdata = es305->pdata;
	static const u8 boot[2] = {ES305_BOOT >> 8, ES305_BOOT};
	u8 ack;
	int retry = RETRY_COUNT;

	while (retry--) {
		/* Reset ES305 chip */
		gpio_set_value(pdata->gpio_reset, 0);
		usleep_range(200, 400);
		gpio_set_value(pdata->gpio_reset, 1);

		/* Delay before sending i2c commands */
		msleep(50);

		/*
		 * Send boot command and check response. The boot command
		 * is different from the others in that it's only 2 bytes,
		 * and the ack retry mechanism is different too.
		 */
		ret = i2c_master_send(es305->client, boot, 2);
		if (ret < 0) {
			dev_err(es305->dev, "i2c_master_send failed\n");
			continue;
		}
		usleep_range(1000, 2000);
		ret = i2c_master_recv(es305->client, &ack, 1);
		if (ret < 0) {
			dev_err(es305->dev, "i2c_master_recv failed\n");
			continue;
		}
		if (ack != ES305_BOOT_ACK) {
			dev_err(es305->dev, "boot ack incorrect (got 0x%.2x)\n",
									ack);
			continue;
		}

		ret = es305_load_firmware(es305);
		if (ret < 0) {
			dev_err(es305->dev, "load firmware error\n");
			continue;
		}

		/* Delay before issuing a sync command */
		msleep(120);

		ret = es305_send_cmd(es305, ES305_SYNC_POLLING, NULL);
		if (ret < 0) {
			dev_err(es305->dev, "sync error\n");
			continue;
		}

		break;
	}

	return ret;
}

static int es305_sleep(struct es305_data *es305)
{
	int ret = 0;
	struct es305_platform_data *pdata = es305->pdata;

	if (es305->asleep)
		return ret;

	ret = es305_send_cmd(es305, ES305_SET_POWER_STATE_SLEEP, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set power state error\n");
		return ret;
	}

	/* The clock can be disabled after the device has had time to sleep */
	msleep(20);
	pdata->clk_enable(false);
	gpio_set_value(pdata->gpio_wakeup, 1);

	es305->asleep = true;

	return ret;
}

static int es305_wake(struct es305_data *es305)
{
	int ret = 0;
	struct es305_platform_data *pdata = es305->pdata;

	if (!es305->asleep)
		return ret;

	pdata->clk_enable(true);
	gpio_set_value(pdata->gpio_wakeup, 0);
	msleep(30);

	ret = es305_send_cmd(es305, ES305_SYNC_POLLING, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "sync error\n");

		/* Go back to sleep */
		pdata->clk_enable(false);
		gpio_set_value(pdata->gpio_wakeup, 1);
		return ret;
	}

	es305->asleep = false;

	return ret;
}

static int es305_set_passthrough(struct es305_data *es305, u32 path)
{
	int ret;

	ret = es305_send_cmd(es305, ES305_DIGITAL_PASSTHROUGH | path, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set passthrough error\n");
		return ret;
	}

	es305->passthrough_on = !!path;

	return ret;
}

static ssize_t es305_audio_routing_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	int ret;
	u16 val;

	if (!es305->device_ready)
		return -EAGAIN;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	ret = es305_send_cmd(es305, ES305_GET_AUDIO_ROUTING, &val);
	if (ret < 0) {
		dev_err(es305->dev, "get audio routing error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return sprintf(buf, "%u\n", val);
}

static ssize_t es305_audio_routing_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	unsigned long val;
	int ret;

	if (!es305->device_ready)
		return -EAGAIN;

	ret = kstrtoul(buf, 0, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	ret = es305_send_cmd(es305, ES305_SET_AUDIO_ROUTING | (u32)val, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set audio routing error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return size;
}

static ssize_t es305_preset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	unsigned long val;
	int ret;

	if (!es305->device_ready)
		return -EAGAIN;

	ret = kstrtoul(buf, 0, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	ret = es305_send_cmd(es305, ES305_SET_PRESET | (u32)val, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set preset error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return size;
}

static ssize_t es305_voice_processing_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	int ret;
	u16 val;

	if (!es305->device_ready)
		return -EAGAIN;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	ret = es305_send_cmd(es305, ES305_GET_VOICE_PROCESSING, &val);
	if (ret < 0) {
		dev_err(es305->dev, "get voice processing error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return sprintf(buf, "%u\n", val);
}

static ssize_t es305_voice_processing_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	unsigned long val;
	int ret;

	if (!es305->device_ready)
		return -EAGAIN;

	ret = kstrtoul(buf, 0, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	/*
	 * If voice processing is being switched on and passthrough is
	 * enabled, disable passthrough first.
	 */
	if (val && es305->passthrough_on) {
		ret = es305_set_passthrough(es305, 0);
		if (ret < 0) {
			dev_err(es305->dev, "unable to disable passthrough\n");
			mutex_unlock(&es305->lock);
			return ret;
		}
	}

	ret = es305_send_cmd(es305, ES305_SET_VOICE_PROCESSING | (u32)val,
									NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set voice processing error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return size;
}

static ssize_t es305_sleep_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct es305_data *es305 = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", es305->asleep ? 1 : 0);
}

static ssize_t es305_sleep_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	unsigned long state;
	int ret;

	if (!es305->device_ready)
		return -EAGAIN;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return -EINVAL;

	if (!!state ^ es305->asleep) {
		/* requested sleep state is different to current state */
		mutex_lock(&es305->lock);
		if (state) {
			if (es305->passthrough != 0) {
				ret = es305_set_passthrough(es305,
							es305->passthrough);
				if (ret < 0)
					dev_err(es305->dev,
						"unable to set passthrough\n");
			}
			ret = es305_sleep(es305);
		} else {
			ret = es305_wake(es305);
		}
		if (ret < 0) {
			dev_err(es305->dev, "unable to change sleep state\n");
			mutex_unlock(&es305->lock);
			return ret;
		}
		mutex_unlock(&es305->lock);
	}

	return size;
}

static ssize_t es305_algorithm_parm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	int ret;
	u16 val;
	u32 parm;

	if (!es305->device_ready)
		return -EAGAIN;

	if (strcmp(attr->attr.name, "aec_enable") == 0)
		parm = ES305_AEC_MODE;
	else if (strcmp(attr->attr.name, "tx_agc_enable") == 0)
		parm = ES305_TX_AGC;
	else if (strcmp(attr->attr.name, "rx_agc_enable") == 0)
		parm = ES305_RX_AGC;
	else if (strcmp(attr->attr.name, "tx_ns_level") == 0)
		parm = ES305_TX_NS_LEVEL;
	else if (strcmp(attr->attr.name, "rx_ns_level") == 0)
		parm = ES305_RX_NS_LEVEL;
	else
		return -EINVAL;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	ret = es305_send_cmd(es305, ES305_GET_ALGORITHM_PARM | parm, &val);
	if (ret < 0) {
		dev_err(es305->dev, "get algorithm parm error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return sprintf(buf, "%u\n", val);
}

static ssize_t es305_algorithm_parm_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct es305_data *es305 = dev_get_drvdata(dev);
	unsigned long val;
	unsigned long val_max = 1;
	int ret;
	u32 parm;

	if (!es305->device_ready)
		return -EAGAIN;

	if (strcmp(attr->attr.name, "aec_enable") == 0) {
		parm = ES305_AEC_MODE;
	} else if (strcmp(attr->attr.name, "tx_agc_enable") == 0) {
		parm = ES305_TX_AGC;
	} else if (strcmp(attr->attr.name, "rx_agc_enable") == 0) {
		parm = ES305_RX_AGC;
	} else if (strcmp(attr->attr.name, "tx_ns_level") == 0) {
		parm = ES305_TX_NS_LEVEL;
		val_max = 10;
	} else if (strcmp(attr->attr.name, "rx_ns_level") == 0) {
		parm = ES305_RX_NS_LEVEL;
		val_max = 10;
	} else if (strcmp(attr->attr.name, "algorithm_reset") == 0) {
		parm = ES305_ALGORITHM_RESET;
		val_max = 0xffff;
	} else {
		return -EINVAL;
	}

	/* Check that a valid value was obtained */
	ret = kstrtoul(buf, 0, &val);
	if (ret || (val > val_max))
		return -EINVAL;

	mutex_lock(&es305->lock);
	ret = es305_wake(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to wake\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	ret = es305_send_cmd(es305, ES305_SET_ALGORITHM_PARM_ID | parm, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set algorithm parm id error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}
	ret = es305_send_cmd(es305, ES305_SET_ALGORITHM_PARM | (u32)val, NULL);
	if (ret < 0) {
		dev_err(es305->dev, "set algorithm parm error\n");
		mutex_unlock(&es305->lock);
		return ret;
	}

	mutex_unlock(&es305->lock);
	return size;
}

static DEVICE_ATTR(audio_routing, S_IRUGO | S_IWUSR,
		es305_audio_routing_show, es305_audio_routing_store);
static DEVICE_ATTR(preset, S_IWUSR,
		NULL, es305_preset_store);
static DEVICE_ATTR(voice_processing, S_IRUGO | S_IWUSR,
		es305_voice_processing_show, es305_voice_processing_store);
static DEVICE_ATTR(sleep, S_IRUGO | S_IWUSR,
		es305_sleep_show, es305_sleep_store);
static DEVICE_ATTR(aec_enable, S_IRUGO | S_IWUSR,
		es305_algorithm_parm_show, es305_algorithm_parm_store);
static DEVICE_ATTR(tx_agc_enable, S_IRUGO | S_IWUSR,
		es305_algorithm_parm_show, es305_algorithm_parm_store);
static DEVICE_ATTR(rx_agc_enable, S_IRUGO | S_IWUSR,
		es305_algorithm_parm_show, es305_algorithm_parm_store);
static DEVICE_ATTR(tx_ns_level, S_IRUGO | S_IWUSR,
		es305_algorithm_parm_show, es305_algorithm_parm_store);
static DEVICE_ATTR(rx_ns_level, S_IRUGO | S_IWUSR,
		es305_algorithm_parm_show, es305_algorithm_parm_store);
static DEVICE_ATTR(algorithm_reset, S_IWUSR,
		NULL, es305_algorithm_parm_store);

static struct attribute *es305_attributes[] = {
	&dev_attr_audio_routing.attr,
	&dev_attr_preset.attr,
	&dev_attr_voice_processing.attr,
	&dev_attr_sleep.attr,
	&dev_attr_aec_enable.attr,
	&dev_attr_tx_agc_enable.attr,
	&dev_attr_rx_agc_enable.attr,
	&dev_attr_tx_ns_level.attr,
	&dev_attr_rx_ns_level.attr,
	&dev_attr_algorithm_reset.attr,
	NULL,
};

static const struct attribute_group es305_attribute_group = {
	.attrs = es305_attributes,
};

/*
 * This is the callback function passed to request_firmware_nowait(),
 * and will be called as soon as the firmware is ready.
 */
static void es305_firmware_ready(const struct firmware *fw, void *context)
{
	struct es305_data *es305 = (struct es305_data *)context;
	struct es305_platform_data *pdata = es305->pdata;
	int ret;

	if (!fw) {
		dev_err(es305->dev, "firmware request failed\n");
		return;
	}
	es305->fw = fw;

	pdata->clk_enable(true);

	ret = es305_reset(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to reset device\n");
		goto err;
	}

	/* Enable passthrough if needed */
	if (es305->passthrough != 0) {
		ret = es305_set_passthrough(es305, es305->passthrough);
		if (ret < 0) {
			dev_err(es305->dev,
				"unable to enable digital passthrough\n");
			goto err;
		}
	}

	ret = es305_sleep(es305);
	if (ret < 0) {
		dev_err(es305->dev, "unable to sleep\n");
		goto err;
	}

	es305->device_ready = true;

err:
	release_firmware(es305->fw);
	es305->fw = NULL;
}

static int __devinit es305_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct es305_data *es305;
	struct es305_platform_data *pdata = client->dev.platform_data;
	int ret = 0;

	es305 = kzalloc(sizeof(*es305), GFP_KERNEL);
	if (es305 == NULL) {
		ret = -ENOMEM;
		goto err_kzalloc;
	}

	es305->client = client;
	i2c_set_clientdata(client, es305);

	es305->dev = &client->dev;
	dev_set_drvdata(es305->dev, es305);

	es305->pdata = pdata;

	if ((pdata->passthrough_src < 0) || (pdata->passthrough_src > 4) ||
					(pdata->passthrough_dst < 0) ||
					(pdata->passthrough_dst > 4) ||
					!pdata->clk_enable) {
		dev_err(es305->dev, "invalid pdata\n");
		ret = -EINVAL;
		goto err_pdata;
	} else if ((pdata->passthrough_src != 0) &&
					(pdata->passthrough_dst != 0)) {
		es305->passthrough = ((pdata->passthrough_src + 3) << 4) |
					((pdata->passthrough_dst - 1) << 2);
	}

	ret = gpio_request(pdata->gpio_wakeup, "ES305 wakeup");
	if (ret < 0) {
		dev_err(es305->dev, "error requesting wakeup gpio\n");
		goto err_gpio_wakeup;
	}
	gpio_direction_output(pdata->gpio_wakeup, 0);

	ret = gpio_request(pdata->gpio_reset, "ES305 reset");
	if (ret < 0) {
		dev_err(es305->dev, "error requesting reset gpio\n");
		goto err_gpio_reset;
	}
	gpio_direction_output(pdata->gpio_reset, 0);

	mutex_init(&es305->lock);

	request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
				ES305_FIRMWARE_NAME, es305->dev, GFP_KERNEL,
				es305, es305_firmware_ready);

	/*
	 * This is not a critical failure, since the device can still be left
	 * in passthrough mode.
	 */
	ret = sysfs_create_group(&es305->dev->kobj, &es305_attribute_group);
	if (ret)
		dev_err(es305->dev, "failed to create sysfs group\n");

	return 0;

err_gpio_reset:
	gpio_free(pdata->gpio_wakeup);
err_gpio_wakeup:
err_pdata:
	kfree(es305);
err_kzalloc:
	return ret;
}

static int __devexit es305_remove(struct i2c_client *client)
{
	struct es305_data *es305 = i2c_get_clientdata(client);

	sysfs_remove_group(&es305->dev->kobj, &es305_attribute_group);

	i2c_set_clientdata(client, NULL);

	gpio_free(es305->pdata->gpio_wakeup);
	gpio_free(es305->pdata->gpio_reset);
	kfree(es305);

	return 0;
}

static const struct i2c_device_id es305_id[] = {
	{"audience_es305", 0},
	{},
};

static struct i2c_driver es305_driver = {
	.driver = {
		.name = "audience_es305",
		.owner = THIS_MODULE,
	},
	.probe = es305_probe,
	.remove = __devexit_p(es305_remove),
	.id_table = es305_id,
};

static int __init es305_init(void)
{
	return i2c_add_driver(&es305_driver);
}

static void __exit es305_exit(void)
{
	i2c_del_driver(&es305_driver);
}

module_init(es305_init);
module_exit(es305_exit);

MODULE_DESCRIPTION("Audience ES305 Voice Processor driver");
MODULE_LICENSE("GPL");
