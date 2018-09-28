/*
 * ST1232 Touchscreen Controller Driver
 *
 * Copyright (C) 2010 Renesas Solutions Corp.
 *	Tony SIM <chinyeow.sim.xt@renesas.com>
 *
 * Using code from:
 *  - android.git.kernel.org: projects/kernel/common.git: synaptics_i2c_rmi.c
 *	Copyright (C) 2007 Google, Inc.
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
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm_qos.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/platform_data/ctouch.h>

#define ST1572_TS_NAME	"st1572-ts"

#define MIN_X		0x00
#define MIN_Y		0x00
#define MAX_X		0x4FF	/* (1280 - 1) */
#define MAX_Y		0x31F	/* (800 - 1) */
#define MAX_AREA	0xff
#define MAX_FINGERS	5
#define FINGES_DATA_REGS (4*MAX_FINGERS+2)

#ifndef CONFIG_TOUCHSCREEN_ST1572_SINGLE
#define CONFIG_ST1572_MULTITOUCH		1
#endif

/*
 * Optional platform data
 *
 * Use this if you want the driver to drive the reset pin.
 */
struct st1232_pdata {
	int reset_gpio;
};

struct st1232_ts_finger {
	u16 x;
	u16 y;
	u8 t;
	bool is_valid;
};

struct st1232_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct st1232_ts_finger finger[MAX_FINGERS];
	struct dev_pm_qos_request low_latency_req;
	int reset_gpio;
	struct mutex mutex;
	struct delayed_work work;
};

static int st1232_ts_reset(struct st1232_ts_data *ts)
{
	int error;
	struct i2c_client *client = ts->client;
	struct i2c_msg msg[2];
	u8 reset[2] = {0x2, 0x1};

	/* read touchscreen reg from ST1232 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = sizeof(reset);
	msg[0].buf = reset;

	error = i2c_transfer(client->adapter, msg, 1);
	if (error < 0) {
		return error;
	}
	return 0;
}

#if 0
static int st1232_ts_read_reg(struct st1232_ts_data *ts, u8 reg_addr)
{
	u8 buf[1];
	int error;
	struct i2c_client *client = ts->client;
	struct i2c_msg msg[2];

	/* read touchscreen reg from ST1232 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg_addr;

	msg[1].addr = ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = sizeof(buf);
	msg[1].buf = buf;

	error = i2c_transfer(client->adapter, msg, 2);
	if (error < 0) {
		dev_err(&client->dev, "fail to read reg %d\n", reg_addr);
		return error;
	}
	dev_err(&client->dev, "addr=%d value=%x\n", reg_addr, buf[0]);
	return 0;
}
#endif

static int st1232_ts_read_data(struct st1232_ts_data *ts)
{
	struct st1232_ts_finger *finger = ts->finger;
	struct i2c_client *client = ts->client;
	struct i2c_msg msg[2];
	int error;
	u8 start_reg;
	u8 buf[FINGES_DATA_REGS];
	int i=0;

	memset(buf, 0, sizeof(buf));
	/* read touchscreen data from ST1232 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &start_reg;
	start_reg = 0x10;

	msg[1].addr = ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = sizeof(buf);
	msg[1].buf = buf;

	error = i2c_transfer(client->adapter, msg, 2);
	if (error < 0)
		return error;

	memset(finger, 0, sizeof(struct st1232_ts_finger) * MAX_FINGERS);
	for (i=0; i<MAX_FINGERS; i++) {
		/* get "valid" bits */
		finger[i].is_valid = buf[2+i*4] >> 7;

		/* get xy coordinate */
		if (finger[i].is_valid) {
			finger[i].x = ((buf[2+i*4] & 0x0070) << 4) | buf[3+i*4];
			finger[i].y = ((buf[2+i*4] & 0x0007) << 8) | buf[4+i*4];
			finger[i].t = buf[5+i*4];
			if (finger[i].t == 0)
				finger[i].t = 200; /* if finger is valid , then must have a area */
		}
	}

	return 0;
}

static void st1232_ts_poscheck(struct work_struct *work)
{
	struct st1232_ts_data *ts = container_of(work,
							struct st1232_ts_data, work.work);
	struct st1232_ts_finger *finger = ts->finger;
	struct input_dev *input_dev = ts->input_dev;
	int touch_point;
	int i, ret;

	mutex_lock(&ts->mutex);

	touch_point = 0;
	ret = st1232_ts_read_data(ts);
	if (ret < 0)
		goto end;

#ifdef CONFIG_ST1572_MULTITOUCH
	for (i = 0; i < MAX_FINGERS; i++) {
		if (finger[i].is_valid) {
			touch_point++;
			input_report_abs(input_dev, ABS_MT_POSITION_X, finger[i].x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y, finger[i].y);
			input_report_abs(input_dev, ABS_MT_PRESSURE, finger[i].t);
			input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, finger[i].t);
			input_report_abs(input_dev, ABS_MT_TRACKING_ID, i);
		}
		input_mt_sync(input_dev);
// printk(KERN_ERR"%d: %d %d %d\n", i, finger[i].x, finger[i].y, finger[i].t);
	}
#else
	if (finger[0].is_valid) {
		input_report_abs(input_dev, ABS_X, finger[0].x);
		input_report_abs(input_dev, ABS_Y, finger[0].y);
		input_report_abs(input_dev, ABS_PRESSURE, finger[0].t);
		input_report_key(input_dev, BTN_TOUCH, 1);
	} else {
		input_report_abs(input_dev, ABS_PRESSURE, 0);
		input_report_key(input_dev, BTN_TOUCH, 0);
	}
#endif
	/* SYN_REPORT */
	input_sync(input_dev);

end:
	mutex_unlock(&ts->mutex);
}

static irqreturn_t st1232_ts_isr(int irq, void *dev_id)
{
	struct st1232_ts_data *ts = dev_id;

	schedule_delayed_work(&ts->work, HZ / 80);
	return IRQ_HANDLED;
}

static void st1232_ts_power(struct st1232_ts_data *ts, bool poweron)
{
	if (gpio_is_valid(ts->reset_gpio))
		gpio_direction_output(ts->reset_gpio, poweron);
}

static int st1232_ts_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct st1232_ts_data *ts;
	struct st1232_pdata *pdata = dev_get_platdata(&client->dev);
	struct input_dev *input_dev;
	int error;
	unsigned int ctp_id;

	ctp_id = board_get_ctp();
	if (ctp_id != CTP_ST1572 && ctp_id != CTP_AUTO) {
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "need I2C_FUNC_I2C\n");
		return -EIO;
	}

	if (!client->irq) {
		dev_err(&client->dev, "no IRQ?\n");
		return -EINVAL;
	}

	ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	input_dev = input_allocate_device();
	if (!input_dev) {
		error = -ENOMEM;
		goto err_free_mem;
	}
	ts->client = client;
	ts->input_dev = input_dev;

	if (pdata)
		ts->reset_gpio = pdata->reset_gpio;
	else if (client->dev.of_node)
		ts->reset_gpio = of_get_gpio(client->dev.of_node, 0);
	else
		ts->reset_gpio = -ENODEV;

	if (gpio_is_valid(ts->reset_gpio)) {
		error = devm_gpio_request(&client->dev, ts->reset_gpio, NULL);
		if (error) {
			dev_err(&client->dev,
				"Unable to request GPIO pin %d.\n",
				ts->reset_gpio);
				return error;
		}
	}

	st1232_ts_power(ts, true);

	input_dev->name = "st1232-touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

#ifdef CONFIG_ST1572_MULTITOUCH
	set_bit(ABS_MT_TRACKING_ID, input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, MIN_X, MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, MIN_Y, MAX_Y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 0xFF, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGERS, 0, 0);
#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, MIN_X, MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, MIN_Y, MAX_Y, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 0xFF, 0, 0);
#endif

	mutex_init(&ts->mutex);
	INIT_DELAYED_WORK(&ts->work, st1232_ts_poscheck);
	error = devm_request_threaded_irq(&client->dev, client->irq,
					  NULL, st1232_ts_isr,
					  IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					  client->name, ts);
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		return error;
	}

	error = input_register_device(ts->input_dev);
	if (error) {
		dev_err(&client->dev, "Unable to register %s input device\n",
			input_dev->name);
		goto err_free_irq;
	}

	i2c_set_clientdata(client, ts);
	device_init_wakeup(&client->dev, 1);

	error = st1232_ts_reset(ts);
	if (error) {
		dev_err(&client->dev, "Unable to reset st1572\n");
		return error;
	}
	return 0;

err_free_irq:
	free_irq(client->irq, ts);
	input_free_device(input_dev);
err_free_mem:
	kfree(ts);
	return error;
}

static int st1232_ts_remove(struct i2c_client *client)
{
	struct st1232_ts_data *ts = i2c_get_clientdata(client);

	device_init_wakeup(&client->dev, 0);
	st1232_ts_power(ts, false);

	return 0;
}

static int __maybe_unused st1232_ts_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *ts = i2c_get_clientdata(client);

	if (device_may_wakeup(&client->dev)) {
		enable_irq_wake(client->irq);
	} else {
		disable_irq(client->irq);
		st1232_ts_power(ts, false);
	}

	return 0;
}

static int __maybe_unused st1232_ts_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *ts = i2c_get_clientdata(client);

	if (device_may_wakeup(&client->dev)) {
		disable_irq_wake(client->irq);
	} else {
		st1232_ts_power(ts, true);
		enable_irq(client->irq);
	}

	return 0;
}

static SIMPLE_DEV_PM_OPS(st1232_ts_pm_ops,
			 st1232_ts_suspend, st1232_ts_resume);

static const struct i2c_device_id st1232_ts_id[] = {
	{ ST1572_TS_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, st1232_ts_id);

#ifdef CONFIG_OF
static const struct of_device_id st1232_ts_dt_ids[] = {
	{ .compatible = "sitronix,st1572", },
	{ }
};
MODULE_DEVICE_TABLE(of, st1232_ts_dt_ids);
#endif

static struct i2c_driver st1232_ts_driver = {
	.probe		= st1232_ts_probe,
	.remove		= st1232_ts_remove,
	.id_table	= st1232_ts_id,
	.driver = {
		.name	= ST1572_TS_NAME,
		.of_match_table = of_match_ptr(st1232_ts_dt_ids),
		.pm	= &st1232_ts_pm_ops,
	},
};

module_i2c_driver(st1232_ts_driver);

MODULE_AUTHOR("Tony SIM <chinyeow.sim.xt@renesas.com>");
MODULE_DESCRIPTION("SITRONIX ST1232 Touchscreen Controller Driver");
MODULE_LICENSE("GPL");
