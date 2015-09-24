/*
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_data/bh1721fvc.h>
#include "../iio.h"
#include "../sysfs.h"
#include "../events.h"

#define BH1721FVC_MODE_DATA(_cmd, _delay, _freq, _name) \
	{.cmd = (_cmd), .delay = (_delay), .freq = (_freq), .name = _name}

#define SAMPLING_FREQUENCY_DEFAULT 5
#define POLL_DELAY_DEFAULT (NSEC_PER_SEC / SAMPLING_FREQUENCY_DEFAULT)

enum BH1721FVC_STATE_AND_MODE {
	STATE_POWER_DOWN,
	STATE_POWER_ON,
	STATE_AUTO_MEASURE,
	STATE_H_MEASURE,
	STATE_L_MEASURE,
	STATE_MAX,
};

struct bh1721fvc_mode_data_params {
	const u8 cmd;
	const u8 delay;
	const u8 freq;
	const char *name;
};

static const struct bh1721fvc_mode_data_params
			bh1721fvc_mode_data[STATE_MAX] = {
	[STATE_POWER_DOWN]	= BH1721FVC_MODE_DATA(0x00, 0, 0, "invalid"),
	[STATE_POWER_ON]	= BH1721FVC_MODE_DATA(0x01, 0, 0, "invalid"),
	[STATE_AUTO_MEASURE]	= BH1721FVC_MODE_DATA(0x10, 136, 7, "auto"),
	[STATE_H_MEASURE]	= BH1721FVC_MODE_DATA(0x12, 120, 8, "high"),
	[STATE_L_MEASURE]	= BH1721FVC_MODE_DATA(0x13, 16, 62, "low"),
};

struct bh1721fvc_data {
	int reset_pin;
	struct i2c_client *client;
	struct iio_dev *indio_dev;
	struct work_struct work_light;
	bool is_measuring;
	bool event_en;
	u16 lux;
	struct hrtimer timer;
	struct mutex lock;
	struct workqueue_struct *wq;
	ktime_t light_poll_delay;
	int light_sampling_frequency;
	enum BH1721FVC_STATE_AND_MODE state;
	enum BH1721FVC_STATE_AND_MODE measure_mode;
};

static int bh1721fvc_light_sensor_reset(int reset_pin)
{
	int err;

	err = gpio_direction_output(reset_pin, 0);
	if (err) {
		pr_err("Failed to make GPIO go low (%d)\n", err);
		return err;
	}
	udelay(2);
	err = gpio_direction_output(reset_pin, 1);
	if (err) {
		pr_err("Failed to make GPIO go high (%d)\n", err);
		return err;
	}
	return 0;
}

static int bh1721fvc_get_luxvalue(struct i2c_client *client)
{
	int err;
	u16 value;

	err = i2c_master_recv(client, (u8 *)&value, 2);
	if (err != 2) {
		pr_err("Light sensor read failed");
		return err >= 0 ? -EIO : err;
	}

	be16_to_cpus(&value);
	/* Scale by 1/1.2 to convert counts to lux */
	value = value * 5 / 6;
	return value;
}

static int bh1721fvc_set_mode(struct bh1721fvc_data *bh1721fvc,
				enum BH1721FVC_STATE_AND_MODE measure_mode)
{
	int err;

	err = i2c_smbus_write_byte(bh1721fvc->client,
		bh1721fvc_mode_data[measure_mode].cmd);
	if (err)
		goto err_write_mode;

	pr_debug("starting poll timer, delay %ldns\n",
			bh1721fvc_mode_data[measure_mode].delay *
			NSEC_PER_MSEC);

	hrtimer_start(&bh1721fvc->timer, ns_to_ktime(bh1721fvc_mode_data[
		measure_mode].delay * NSEC_PER_MSEC),
		HRTIMER_MODE_REL);

	bh1721fvc->state = measure_mode;

	return 0;

err_write_mode:
	pr_err("Error writing mode %s to device",
		bh1721fvc_mode_data[measure_mode].name);
	i2c_smbus_write_byte(bh1721fvc->client,
				bh1721fvc_mode_data[STATE_POWER_DOWN].cmd);
	return err;

}

static int bh1721fvc_disable(struct bh1721fvc_data *bh1721fvc)
{
	int err;

	pr_debug("cancelling poll timer\n");
	hrtimer_cancel(&bh1721fvc->timer);
	cancel_work_sync(&bh1721fvc->work_light);

	err = i2c_smbus_write_byte(bh1721fvc->client,
				 bh1721fvc_mode_data[STATE_POWER_DOWN].cmd);
	if (unlikely(err != 0)) {
		pr_err("Failed to write byte (STATE_POWER_DOWN)\n");
		return err;
	}

	bh1721fvc->state = STATE_POWER_DOWN;

	return 0;
}

static void bh1721fvc_work_func_light(struct work_struct *work)
{
	u16 lux;
	int err;
	struct bh1721fvc_data *bh1721fvc = container_of(work,
							struct bh1721fvc_data,
							work_light);

	lux = bh1721fvc_get_luxvalue(bh1721fvc->client);
	if (lux < 0) {
		pr_err("read word failed! (errno=%d)\n", lux);
		return;
	}
	bh1721fvc->lux = lux;

	pr_debug("lux %#04x (%u)\n", lux, lux);
	err = iio_push_event(iio_priv_to_dev(bh1721fvc),
			IIO_UNMOD_EVENT_CODE(IIO_LIGHT,
				0,
				IIO_EV_TYPE_THRESH,
				IIO_EV_DIR_EITHER),
			iio_get_time_ns());
	if (err)
		pr_err("Could not push IIO_LIGHT event");
}

static enum hrtimer_restart bh1721fvc_timer_func(struct hrtimer *timer)
{
	struct bh1721fvc_data *bh1721fvc = container_of(timer,
							struct bh1721fvc_data,
							timer);

	queue_work(bh1721fvc->wq, &bh1721fvc->work_light);
	hrtimer_forward_now(&bh1721fvc->timer, bh1721fvc->light_poll_delay);
	return HRTIMER_RESTART;
}

static ssize_t bh1721fvc_mode_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct bh1721fvc_data *bh1721fvc = iio_priv(dev_get_drvdata(dev));

	return sprintf(buf, "%s\n",
			bh1721fvc_mode_data[bh1721fvc->measure_mode].name);
}

static ssize_t bh1721fvc_mode_store(struct device *dev,
			struct device_attribute *attr, const char *buf,
			size_t size)
{
	u8 new_measure_mode;
	s64 delay_lowbound;
	struct bh1721fvc_data *bh1721fvc = iio_priv(dev_get_drvdata(dev));
	int err;

	if (sysfs_streq(buf, bh1721fvc_mode_data[STATE_AUTO_MEASURE].name)) {
		new_measure_mode = STATE_AUTO_MEASURE;
	} else if (sysfs_streq(buf,
				bh1721fvc_mode_data[STATE_H_MEASURE].name)) {
		new_measure_mode = STATE_H_MEASURE;
	} else if (sysfs_streq(buf,
				bh1721fvc_mode_data[STATE_L_MEASURE].name)) {
		new_measure_mode = STATE_L_MEASURE;
	} else {
		pr_err("invalid value %s\n", buf);
		return -EINVAL;
	}

	mutex_lock(&bh1721fvc->lock);
	if (bh1721fvc->measure_mode != new_measure_mode) {
		delay_lowbound =
			bh1721fvc_mode_data[new_measure_mode].delay
			* NSEC_PER_MSEC;
		if (ktime_to_ns(bh1721fvc->light_poll_delay) < delay_lowbound) {
			bh1721fvc->light_poll_delay
				= ns_to_ktime(delay_lowbound);
			bh1721fvc->light_sampling_frequency =
				bh1721fvc_mode_data[new_measure_mode].freq;
		}
		if (bh1721fvc->state != STATE_POWER_DOWN) {
			hrtimer_cancel(&bh1721fvc->timer);
			cancel_work_sync(&bh1721fvc->work_light);
			err = bh1721fvc_set_mode(bh1721fvc, new_measure_mode);
			if (err) {
				pr_err("Failed to change to mode %s",
					bh1721fvc_mode_data[new_measure_mode].
					name);
				mutex_unlock(&bh1721fvc->lock);
				return -EIO;
			}
		}
		bh1721fvc->measure_mode = new_measure_mode;
	}
	mutex_unlock(&bh1721fvc->lock);

	return size;
}

static ssize_t bh1721fvc_sampling_frequency_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct bh1721fvc_data *bh1721fvc = iio_priv(dev_get_drvdata(dev));

	pr_debug("delay: %lld delay_hz: %d",
		ktime_to_ns(bh1721fvc->light_poll_delay),
		bh1721fvc->light_sampling_frequency);
	return sprintf(buf, "%d\n", bh1721fvc->light_sampling_frequency);
}

static ssize_t bh1721fvc_sampling_frequency_store(struct device *dev,
				struct device_attribute *attr, const char *buf,
				size_t size)
{
	struct bh1721fvc_data *bh1721fvc = iio_priv(dev_get_drvdata(dev));

	int new_freq_hz;
	s64 new_delay_ns;
	s64 delay_lowbound;
	int err;

	err = kstrtoint(buf, 10, &new_freq_hz);
	if (err < 0)
		return err;

	if (new_freq_hz <= 0)
		return -EINVAL;

	/* Conversion from Hz to ns */
	new_delay_ns = NSEC_PER_SEC / new_freq_hz;
	delay_lowbound = bh1721fvc_mode_data[bh1721fvc->measure_mode].delay
				* NSEC_PER_MSEC;

	if (new_delay_ns < delay_lowbound) {
		new_delay_ns = delay_lowbound;
		new_freq_hz =
			bh1721fvc_mode_data[bh1721fvc->measure_mode].freq;
	}
	pr_debug("new delay = %lldns, old delay = %lldns\n", new_delay_ns,
		ktime_to_ns(bh1721fvc->light_poll_delay));


	mutex_lock(&bh1721fvc->lock);
	if (new_delay_ns != ktime_to_ns(bh1721fvc->light_poll_delay)) {
		bh1721fvc->light_poll_delay = ns_to_ktime(new_delay_ns);
		bh1721fvc->light_sampling_frequency = new_freq_hz;
	}
	mutex_unlock(&bh1721fvc->lock);

	return size;
}

static IIO_DEVICE_ATTR(mode, S_IRUGO | S_IWUSR,
	bh1721fvc_mode_show, bh1721fvc_mode_store, 0);
static IIO_CONST_ATTR(mode_available, "auto high low");
static IIO_DEV_ATTR_SAMP_FREQ(S_IRUGO | S_IWUSR,
	bh1721fvc_sampling_frequency_show, bh1721fvc_sampling_frequency_store);
static IIO_CONST_ATTR_SAMP_FREQ_AVAIL("auto: <=7Hz, high: <=8Hz, low <=62Hz");

#define BH1721FVC_DEV_ATTR(name) (&iio_dev_attr_##name.dev_attr.attr)
#define BH1721FVC_CONST_ATTR(name) (&iio_const_attr_##name.dev_attr.attr)
static struct attribute *bh1721fvc_attributes[] = {
	BH1721FVC_DEV_ATTR(mode),
	BH1721FVC_CONST_ATTR(mode_available),
	BH1721FVC_DEV_ATTR(sampling_frequency),
	BH1721FVC_CONST_ATTR(sampling_frequency_available),
	NULL
};

static const struct attribute_group bh1721fvc_attribute_group = {
	.attrs = bh1721fvc_attributes,
};

static const struct iio_chan_spec bh1721fvc_channels[] = {
	{
		.type = IIO_LIGHT,
		.indexed = 1,
		.channel = 0,
		.processed_val = IIO_PROCESSED,
		.event_mask = (IIO_EV_BIT(IIO_EV_TYPE_THRESH,
				IIO_EV_DIR_EITHER)),
	}
};

static int bh1721fvc_read_raw(struct iio_dev *indio_dev,
			      struct iio_chan_spec const *chan,
			      int *val, int *val2, long m)
{
	struct bh1721fvc_data *bh1721fvc = iio_priv(indio_dev);

	pr_debug("Reporting lux(%d) to user", bh1721fvc->lux);
	*val = bh1721fvc->lux;

	return IIO_VAL_INT;
}

static int bh1721fvc_read_event_value(struct iio_dev *indio_dev,
			       u64 event_code,
			       int *val)
{
	struct bh1721fvc_data *bh1721fvc = iio_priv(indio_dev);

	*val = bh1721fvc->lux;

	return 0;
}

static int bh1721fvc_read_event_config(struct iio_dev *indio_dev,
				       u64 event_code)
{
	struct bh1721fvc_data *bh1721fvc = iio_priv(indio_dev);

	pr_debug("is_measuring = %d", bh1721fvc->state != STATE_POWER_DOWN);
	return (bh1721fvc->state != STATE_POWER_DOWN);
}

static int bh1721fvc_write_event_config(struct iio_dev *indio_dev,
				       u64 event_code,
				       int state)
{
	int err = 0;
	struct bh1721fvc_data *bh1721fvc = iio_priv(indio_dev);

	pr_debug("state %d->%d\n", (bh1721fvc->state != STATE_POWER_DOWN),
			state);

	mutex_lock(&bh1721fvc->lock);
	if (state && (bh1721fvc->state == STATE_POWER_DOWN)) {
		err = bh1721fvc_set_mode(bh1721fvc, bh1721fvc->measure_mode);
		if (err)
			goto err_set_mode;
	} else if (!state && (bh1721fvc->state != STATE_POWER_DOWN)) {
		err = bh1721fvc_disable(bh1721fvc);
		if (err)
			goto err_set_mode;
	}
	bh1721fvc->is_measuring = state;
	mutex_unlock(&bh1721fvc->lock);

	return 0;

err_set_mode:
	mutex_unlock(&bh1721fvc->lock);
	return err;
}

static const struct iio_info bh1721fvc_info = {
	.attrs = &bh1721fvc_attribute_group,
	.driver_module = THIS_MODULE,
	.read_raw = bh1721fvc_read_raw,
	.read_event_value = bh1721fvc_read_event_value,
	.read_event_config = bh1721fvc_read_event_config,
	.write_event_config = bh1721fvc_write_event_config,
};

static int __devinit bh1721fvc_i2c_probe(struct i2c_client *client,
					 const struct i2c_device_id *id)
{
	int err;
	struct bh1721fvc_data *bh1721fvc;
	struct iio_dev *indio_dev;
	struct bh1721fvc_platform_data *pdata;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C |
					I2C_FUNC_SMBUS_WRITE_BYTE))
		return -ENOSYS;

	pdata = client->dev.platform_data;
	if (!pdata) {
		pr_err("no platform data\n");
		return -EINVAL;
	}

	indio_dev = iio_allocate_device(sizeof(*bh1721fvc));
	if (!indio_dev)
		return -ENOMEM;

	bh1721fvc = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	bh1721fvc->client = client;

	bh1721fvc->reset_pin = pdata->reset_pin;
	if (bh1721fvc->reset_pin < 0) {
		pr_err("reset pin is invalid\n");
		err = -EINVAL;
		goto err_reset_invalid;
	}

	err = gpio_request_one(bh1721fvc->reset_pin, GPIOF_OUT_INIT_HIGH,
			"ALS_NRST");
	if (err) {
		pr_err("Failed to request ALS_NRST for light sensor reset\n");
		goto err_reset_request;
	}

	err = bh1721fvc_light_sensor_reset(bh1721fvc->reset_pin);
	if (err) {
		pr_err("Failed to reset\n");
		goto err_reset_failed;
	}

	mutex_init(&bh1721fvc->lock);
	hrtimer_init(&bh1721fvc->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	bh1721fvc->light_poll_delay = ns_to_ktime(POLL_DELAY_DEFAULT);
	bh1721fvc->light_sampling_frequency = SAMPLING_FREQUENCY_DEFAULT;
	bh1721fvc->state = STATE_POWER_DOWN;
	bh1721fvc->measure_mode = STATE_AUTO_MEASURE;
	bh1721fvc->is_measuring = false;
	bh1721fvc->timer.function = bh1721fvc_timer_func;

	bh1721fvc->wq = alloc_workqueue("bh1721fvc_wq",
					WQ_UNBOUND | WQ_RESCUER, 1);
	if (!bh1721fvc->wq) {
		err = -ENOMEM;
		pr_err("could not create workqueue\n");
		goto err_create_workqueue;
	}

	INIT_WORK(&bh1721fvc->work_light, bh1721fvc_work_func_light);

	indio_dev->name = "lightsensor-level";
	indio_dev->channels = bh1721fvc_channels;
	indio_dev->num_channels = ARRAY_SIZE(bh1721fvc_channels);
	indio_dev->dev.parent = &client->dev;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = &bh1721fvc_info;

	pr_info("registering lightsensor-level input device\n");
	err = iio_device_register(indio_dev);
	if (err)
		goto err_iio_register_device_light;
	return 0;

err_iio_register_device_light:
	destroy_workqueue(bh1721fvc->wq);
err_create_workqueue:
	mutex_destroy(&bh1721fvc->lock);
err_reset_failed:
	gpio_free(bh1721fvc->reset_pin);
err_reset_request:
err_reset_invalid:
	iio_free_device(indio_dev);
	return err;
}

static int __devexit bh1721fvc_i2c_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct bh1721fvc_data *bh1721fvc = iio_priv(indio_dev);

	iio_device_unregister(indio_dev);

	if (bh1721fvc->is_measuring)
		bh1721fvc_disable(bh1721fvc);

	destroy_workqueue(bh1721fvc->wq);
	mutex_destroy(&bh1721fvc->lock);
	gpio_free(bh1721fvc->reset_pin);
	iio_free_device(indio_dev);

	return 0;
}

static int bh1721fvc_suspend(struct device *dev)
{
	int err = 0;
	struct bh1721fvc_data *bh1721fvc = iio_priv(i2c_get_clientdata(
							to_i2c_client(dev)));

	if (bh1721fvc->is_measuring) {
		err = bh1721fvc_disable(bh1721fvc);
		if (err)
			pr_err("could not disable\n");
	}

	return err;
}

static int bh1721fvc_resume(struct device *dev)
{
	int err;
	struct bh1721fvc_data *bh1721fvc = iio_priv(i2c_get_clientdata(
							to_i2c_client(dev)));

	if (bh1721fvc->is_measuring) {
		err = bh1721fvc_set_mode(bh1721fvc, bh1721fvc->measure_mode);
		if (err) {
			pr_err("could not enable\n");
			return err;
		}
	}

	return 0;
}

static const struct i2c_device_id bh1721fvc_device_id[] = {
	{"bh1721fvc", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bh1721fvc_device_id);

static const struct dev_pm_ops bh1721fvc_pm_ops = {
	.suspend = bh1721fvc_suspend,
	.resume = bh1721fvc_resume,
};

static struct i2c_driver bh1721fvc_driver = {
	.driver = {
		   .name = "bh1721fvc",
		   .owner = THIS_MODULE,
		   .pm = &bh1721fvc_pm_ops,
	},
	.probe = bh1721fvc_i2c_probe,
	.remove = __devexit_p(bh1721fvc_i2c_remove),
	.id_table = bh1721fvc_device_id,
};

module_i2c_driver(bh1721fvc_driver);

MODULE_AUTHOR("Veeren Mandalia <v.mandalia@sta.samsung.com>");
MODULE_DESCRIPTION("BH1721FVC Ambient light sensor driver");
MODULE_LICENSE("GPL v2");
