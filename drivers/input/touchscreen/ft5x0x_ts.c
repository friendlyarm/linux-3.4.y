/*
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2012 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2010 Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <mach/gpio.h>
#include <linux/platform_data/ctouch.h>
#include <linux/platform_data/ft5x0x_touch.h>
#if defined(CONFIG_PLAT_SAMSUNG)
#include <plat/gpio-cfg.h>
#endif

#include "ft5x06_ts.h"

#ifndef CONFIG_TOUCHSCREEN_FT5X0X_SINGLE
#define CONFIG_FT5X0X_MULTITOUCH		1
#endif

#define TOUCH_MAX_X						0x700
#define TOUCH_MAX_Y						0x400

static int swap_xy = 0;
static int scal_xy = 0;


/*---------------------------------------------------------
 * Chip I/O operations
 */

static struct i2c_client *this_client;

static int ft5x0x_i2c_rxdata(char *rxdata, int length) {
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("%s: i2c read error: %d\n", __func__, ret);

	return ret;
}

#if 0
static int ft5x0x_i2c_txdata(char *txdata, int length) {
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s: i2c write error: %d\n", __func__, ret);

	return ret;
}

static int ft5x0x_write_reg(u8 addr, u8 val) {
	u8 buf[4];
	int ret;

	buf[0] = addr;
	buf[1] = val;
	ret = ft5x0x_i2c_txdata(buf, 2);
	if (ret < 0) {
		pr_err("write 0x%02x to reg (0x%02x) failed, %d", addr, val, ret);
		return -1;
	}

	return 0;
}
#endif

static int ft5x0x_read_reg(u8 addr, u8 *pdata) {
	u8 buf[4] = { 0 };
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};
	int ret;

	buf[0] = addr;

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0) {
		pr_err("read reg (0x%02x) error, %d\n", addr, ret);
	} else {
		*pdata = buf[0];
	}

	return ret;
}

static int ft5x0x_read_fw_ver(unsigned char *val)
{
	int ret;

	*val = 0xff;
	ret = ft5x0x_read_reg(FT5X0X_REG_FIRMID, val);
	if (*val == 0x06) {
#if 0
		swap_xy = 1;
		scal_xy = 1;
#endif
	} else {
		/* TODO: Add support for other version */
	}

	return ret;
}


/*---------------------------------------------------------
 * Touch core support
 */

static void ft5x0x_ts_report(struct ft5x0x_ts_data *ts) {
	struct ft5x0x_event *event = &ts->event;
	int x, y;
	int i = 0;

#ifdef CONFIG_FT5X0X_MULTITOUCH
	for (; i < event->touch_point; i++) {
		if (swap_xy) {
			x = event->y[i];
			y = event->x[i];
		} else {
			x = event->x[i];
			y = event->y[i];
		}

		if (scal_xy) {
			x = (x * ts->screen_max_x) / TOUCH_MAX_X;
			y = (y * ts->screen_max_y) / TOUCH_MAX_Y;
		}

		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);

		input_report_abs(ts->input_dev, ABS_MT_PRESSURE, event->pressure);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);

		input_mt_sync(ts->input_dev);
	}
#else
	if (event->touch_point == 1) {
		if (swap_xy) {
			x = event->y[i];
			y = event->x[i];
		} else {
			x = event->x[i];
			y = event->y[i];
		}

		if (scal_xy) {
			x = (x * ts->screen_max_x) / TOUCH_MAX_X;
			y = (y * ts->screen_max_y) / TOUCH_MAX_Y;
		}

		input_report_abs(ts->input_dev, ABS_X, x);
		input_report_abs(ts->input_dev, ABS_Y, y);
		input_report_abs(ts->input_dev, ABS_PRESSURE, event->pressure);
	}

	input_report_key(ts->input_dev, BTN_TOUCH, 1);
#endif

	input_sync(ts->input_dev);
}

static void ft5x0x_ts_release(struct ft5x0x_ts_data *ts) {
#ifdef CONFIG_FT5X0X_MULTITOUCH
#if 0
	/* NOT needed for ICS */
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
#endif
	input_mt_sync(ts->input_dev);
#else
	input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
#endif

	input_sync(ts->input_dev);
}

static int ft5x0x_read_data(struct ft5x0x_ts_data *ts) {
	struct ft5x0x_event *event = &ts->event;
	u8 buf[32] = { 0 };
	int ret;

#ifdef CONFIG_FT5X0X_MULTITOUCH
	ret = ft5x0x_i2c_rxdata(buf, 31);
#else
	ret = ft5x0x_i2c_rxdata(buf, 7);
#endif
	if (ret < 0) {
		printk("%s: read touch data failed, %d\n", __func__, ret);
		return ret;
	}

	memset(event, 0, sizeof(struct ft5x0x_event));

	event->touch_point = buf[2] & 0x07;

	if (!event->touch_point) {
		ft5x0x_ts_release(ts);
		return 1;
	}

#ifdef CONFIG_FT5X0X_MULTITOUCH
	switch (event->touch_point) {
		case 5:
			event->x[4] = (s16)(buf[0x1b] & 0x0F)<<8 | (s16)buf[0x1c];
			event->y[4] = (s16)(buf[0x1d] & 0x0F)<<8 | (s16)buf[0x1e];
		case 4:
			event->x[3] = (s16)(buf[0x15] & 0x0F)<<8 | (s16)buf[0x16];
			event->y[3] = (s16)(buf[0x17] & 0x0F)<<8 | (s16)buf[0x18];
		case 3:
			event->x[2] = (s16)(buf[0x0f] & 0x0F)<<8 | (s16)buf[0x10];
			event->y[2] = (s16)(buf[0x11] & 0x0F)<<8 | (s16)buf[0x12];
		case 2:
			event->x[1] = (s16)(buf[0x09] & 0x0F)<<8 | (s16)buf[0x0a];
			event->y[1] = (s16)(buf[0x0b] & 0x0F)<<8 | (s16)buf[0x0c];
		case 1:
			event->x[0] = (s16)(buf[0x03] & 0x0F)<<8 | (s16)buf[0x04];
			event->y[0] = (s16)(buf[0x05] & 0x0F)<<8 | (s16)buf[0x06];
			break;
		default:
			printk("%s: invalid touch data, %d\n", __func__, event->touch_point);
			return -1;
	}
#else
	if (event->touch_point == 1) {
		event->x[0] = (s16)(buf[0x03] & 0x0F)<<8 | (s16)buf[0x04];
		event->y[0] = (s16)(buf[0x05] & 0x0F)<<8 | (s16)buf[0x06];
	}
#endif

	event->pressure = 200;

	return 0;
}

static void ft5x0x_ts_pen_irq_work(struct work_struct *work) {
	struct ft5x0x_ts_data *ts = container_of(work, struct ft5x0x_ts_data, work);

	if (!ft5x0x_read_data(ts)) {
		ft5x0x_ts_report(ts);
	}

	enable_irq(this_client->irq);
}

static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id) {
	struct ft5x0x_ts_data *ts = dev_id;

	disable_irq_nosync(this_client->irq);

	if (!work_pending(&ts->work)) {
		queue_work(ts->queue, &ts->work);
	}

	return IRQ_HANDLED;
}


/*---------------------------------------------------------
 * I2C client driver functions
 */

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x0x_ts_suspend(struct early_suspend *handler)
{
#if 0
	struct ft5x0x_ts_data *ts;

	ts = container_of(handler, struct ft5x0x_ts_data, early_suspend);

	disable_irq(this_client->irq);
	cancel_work_sync(&ts->work);
	flush_workqueue(ts->queue);

	ft5x0x_set_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
#endif

	printk("ft5x0x_ts: suspended\n");
}

static void ft5x0x_ts_resume(struct early_suspend *handler)
{
#if 0
	/* Wakeup: output_L --> 100ms --> output_H --> 100ms */
	enable_irq(this_client->irq);
#endif

	printk("ft5x0x_ts: resumed\n");
}
#endif

static int ft5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x0x_i2c_platform_data *pdata;
	struct ft5x0x_ts_data *ts;
	struct input_dev *input_dev;
	unsigned char val;
	unsigned int ctp_id;
	int err = -EINVAL;

	ctp_id = board_get_ctp();
	if (ctp_id != CTP_FT5X06 && ctp_id != CTP_AUTO) {
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	pdata = client->dev.platform_data;
	if (!pdata) {
		dev_err(&client->dev, "failed to get platform data\n");
		goto exit_no_pdata;
	}

	ts->screen_max_x = pdata->screen_max_x;
	ts->screen_max_y = pdata->screen_max_y;
	ts->pressure_max = pdata->pressure_max;

	ts->gpio_irq = pdata->gpio_irq;
	if (ts->gpio_irq != -EINVAL) {
#if defined(CONFIG_PLAT_SAMSUNG)
		client->irq = gpio_to_irq(ts->gpio_irq);
#else
		client->irq = ts->gpio_irq;
#endif
	} else {
		goto exit_no_pdata;
	}
#if defined(CONFIG_PLAT_SAMSUNG)
	if (pdata->irq_cfg) {
		s3c_gpio_cfgpin(ts->gpio_irq, pdata->irq_cfg);
		s3c_gpio_setpull(ts->gpio_irq, S3C_GPIO_PULL_NONE);
	}
#endif

	ts->gpio_wakeup = pdata->gpio_wakeup;
	ts->gpio_reset = pdata->gpio_reset;

	INIT_WORK(&ts->work, ft5x0x_ts_pen_irq_work);
	this_client = client;
	i2c_set_clientdata(client, ts);

	ts->queue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ts->queue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ts->input_dev = input_dev;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

#ifdef CONFIG_FT5X0X_MULTITOUCH
	set_bit(ABS_MT_TRACKING_ID, input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ts->screen_max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ts->screen_max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, ts->pressure_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, FT5X0X_PT_MAX, 0, 0);
#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0, ts->screen_max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, ts->screen_max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, ts->pressure_max, 0 , 0);
#endif

	input_dev->name = FT5X0X_NAME;
	input_dev->phys = "input(mt)";
	input_dev->id.bustype = BUS_I2C;
	input_dev->id.vendor = 0x12FA;
	input_dev->id.product = 0x2143;
	input_dev->id.version = 0x0100;

	err = input_register_device(input_dev);
	if (err) {
		input_free_device(input_dev);
		dev_err(&client->dev, "failed to register input device %s, %d\n",
				dev_name(&client->dev), err);
		goto exit_input_dev_alloc_failed;
	}

	msleep(3);
	err = ft5x0x_read_fw_ver(&val);
	if (err < 0) {
		dev_err(&client->dev, "chip not found\n");
		goto exit_irq_request_failed;
	}

	err = request_irq(client->irq, ft5x0x_ts_interrupt,
			IRQ_TYPE_EDGE_FALLING /*IRQF_TRIGGER_FALLING*/, "ft5x0x_ts", ts);
	if (err < 0) {
		dev_err(&client->dev, "Request IRQ %d failed, %d\n", client->irq, err);
		goto exit_irq_request_failed;
	}

	disable_irq(client->irq);

	dev_info(&client->dev, "Firmware version 0x%02x\n", val);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = ft5x0x_ts_suspend;
	ts->early_suspend.resume = ft5x0x_ts_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	enable_irq(client->irq);

	board_set_ctp(CTP_FT5X06);
	dev_info(&client->dev, "FocalTech ft5x0x TouchScreen initialized\n");
	return 0;

exit_irq_request_failed:
	input_unregister_device(input_dev);

exit_input_dev_alloc_failed:
	cancel_work_sync(&ts->work);
	destroy_workqueue(ts->queue);

exit_create_singlethread:
	i2c_set_clientdata(client, NULL);

exit_no_pdata:
	kfree(ts);

exit_alloc_data_failed:
exit_check_functionality_failed:
	dev_err(&client->dev, "probe ft5x0x TouchScreen failed, %d\n", err);

	return err;
}

static int __devexit ft5x0x_ts_remove(struct i2c_client *client) {
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif

	if (client->irq) {
		free_irq(client->irq, ts);
	}

	cancel_work_sync(&ts->work);
	destroy_workqueue(ts->queue);

	i2c_set_clientdata(client, NULL);
	input_unregister_device(ts->input_dev);
	if (ts->input_dev)
		kfree(ts->input_dev);

	kfree(ts);

	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{ FT5X0X_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe		= ft5x0x_ts_probe,
	.remove		= __devexit_p(ft5x0x_ts_remove),
	.id_table	= ft5x0x_ts_id,
	.driver	= {
		.name	= FT5X0X_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init ft5x0x_ts_init(void)
{
	return i2c_add_driver(&ft5x0x_ts_driver);
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");

