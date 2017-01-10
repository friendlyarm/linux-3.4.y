/*
 * Touch Screen driver for Himax touchscreen controllers used in
 * DataImage's I2C connected touchscreen panels.
 *   Copyright (c) 2012 Anders Electronics
 *   Copyright 2012 CompuLab Ltd, Dmitry Lifshitz <lifshitz@compulab.co.il>
 *
 * Based on migor_ts.c
 *   Copyright (c) 2008 Magnus Damm
 *   Copyright (c) 2007 Ujjwal Pande <ujjwal@kenati.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU  General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/input/mt.h>
#include <linux/platform_data/ctouch.h>

#define HX_MAX_X        480
#define HX_MAX_Y        800

#define HX_PNT_SIZE     4
#define HX_EMPTY        0xFFFF

#ifndef CONFIG_TOUCHSCREEN_HIMAX_SINGLE
#define CONFIG_HIMAX_MULTITOUCH		1
#endif

struct himax_ts_initseq_entry {
    char *cmd;
    int count;
    int delay_ms;
};

struct himax_ts_props_entry {
    int model;
    struct himax_ts_initseq_entry *initseq;
    int initseq_size;
    int packet_size;
    int touch_points;
    bool invert_x;
    bool invert_y;
    bool xy_order;
};

struct himax_ts_priv {
    struct i2c_client *client;
    struct himax_ts_props_entry *ts_props;
    struct input_dev *input;
    int prev_touches;
    int irq;
    char *buf;
};

static char hx85x_ic_poweron_cmd[]  = {0x81};
static char hx85x_mcu_poweron_cmd[] = {0x35, 0x02};
static char hx85x_senseon_cmd[]     = {0x83};
static char hx85x_senseoff_cmd[]    = {0x82};
static char hx85x_get_id_cmd[]      = {0x31};
static char hx85x_get_event_cmd[]   = {0x85};
static char hx85x_get_sleep_cmd[]   = {0x63};

static char hx8526_flash_poweron_cmd[]  = {0x36, 0x0F, 0x53};
static char hx8526_fetch_flash_cmd[]    = {0xDD, 0x04, 0x02};

static char hx8520_flash_poweron_cmd[]  = {0x36, 0x01};
static char hx8520_speed_mode_cmd[] = {0x9D, 0x80};

static struct himax_ts_initseq_entry hx8526_initseq[] = {
    {hx85x_ic_poweron_cmd,     ARRAY_SIZE(hx85x_ic_poweron_cmd), 120},
    {hx85x_mcu_poweron_cmd,    ARRAY_SIZE(hx85x_mcu_poweron_cmd), 10},
    {hx8526_flash_poweron_cmd, ARRAY_SIZE(hx8526_flash_poweron_cmd), 10},
    {hx8526_fetch_flash_cmd,   ARRAY_SIZE(hx8526_fetch_flash_cmd), 10},
};

static struct himax_ts_initseq_entry hx8520_initseq[] = {
    {hx85x_ic_poweron_cmd,     ARRAY_SIZE(hx85x_ic_poweron_cmd), 120},
    {hx8520_speed_mode_cmd,    ARRAY_SIZE(hx8520_speed_mode_cmd), 10},
    {hx85x_mcu_poweron_cmd,    ARRAY_SIZE(hx85x_mcu_poweron_cmd), 10},
    {hx8520_flash_poweron_cmd, ARRAY_SIZE(hx8520_flash_poweron_cmd), 10},
};

static struct himax_ts_props_entry himax_ts_props[] = {
    {
        .model      = 0x8520,
        .initseq    = hx8520_initseq,
        .initseq_size   = ARRAY_SIZE(hx8520_initseq),
        .packet_size    = 16,
        .touch_points   = 2,
        .invert_y   = true,
    },
    {
        .model      = 0x8526,
        .initseq    = hx8526_initseq,
        .initseq_size   = ARRAY_SIZE(hx8526_initseq),
        .packet_size    = 32,
        .touch_points   = 5,
        .xy_order   = true,
    },
    {
        .model      = 0x8528,
        .initseq    = hx8526_initseq,
        .initseq_size   = ARRAY_SIZE(hx8526_initseq),
        .packet_size    = 16,
        .touch_points   = 2,
        .xy_order   = true,
    },
};

static void himax_ts_set_coords(struct himax_ts_props_entry *props,
                u32 *px, u32 *py)
{
    u32 x = *px;
    u32 y = *py;

    if (!props->xy_order)
        swap(x, y);

    if (props->invert_x && x != HX_EMPTY)
        x = HX_MAX_X - x;

    if (props->invert_y && y != HX_EMPTY)
        y = HX_MAX_Y - y;

    *px = x;
    *py = y;
}

static irqreturn_t himax_ts_isr(int irq, void *data)
{
    struct himax_ts_priv *priv = data;
    struct himax_ts_props_entry *props = priv->ts_props;
    struct input_dev *input = priv->input;
    int packet_size = props->packet_size;
    char *buf = priv->buf;
    int curr_touches, touch_count;
    u32 x, y;

    memset(buf, 0, packet_size);

    if (i2c_master_send(priv->client, hx85x_get_event_cmd, 1) != 1) {
        dev_err(&priv->client->dev, "Unable to write get event cmd\n");
        return IRQ_HANDLED;
    }

    if (i2c_master_recv(priv->client, buf, packet_size) != packet_size) {
        dev_err(&priv->client->dev, "Unable to read events data\n");
        return IRQ_HANDLED;
    }

    /*
     * Two last bytes in the buffer correspond to invalid data. Next two
     * from the end, correspond to touch counter and touch points ids.
     */

    /* Retrieve touch points counter. 0x0F corresponds to 0 touches */
    touch_count = buf[packet_size - 2 - 2] & 0x0F;
    if (touch_count == 0x0F)
        touch_count = 0;

    /* According to the Himax code examples, this value can be invalid */
    if (touch_count > props->touch_points)
        return IRQ_HANDLED;

    /* Retrieve touch points ids. 0xFF corresponds to 0 touches */
    curr_touches = buf[packet_size - 2 - 1];
    if (curr_touches == 0xFF)
        curr_touches = 0;

#ifdef CONFIG_HIMAX_MULTITOUCH
	bool was_touched, now_touched, report_event;
	int i ;
    for (i = 0; i < props->touch_points; i++) {
        x = (buf[i * HX_PNT_SIZE + 0] << 8) | buf[i * HX_PNT_SIZE + 1];
        y = (buf[i * HX_PNT_SIZE + 2] << 8) | buf[i * HX_PNT_SIZE + 3];

        himax_ts_set_coords(props, &x, &y);

        report_event = false;
        was_touched = priv->prev_touches & (1 << i);
        now_touched = curr_touches & (1 << i);

        /* Check for touch state and coordinates consistency */
        if (now_touched && (x <= HX_MAX_X && y <= HX_MAX_Y)) {
            report_event = true;
			dev_dbg(&priv->client->dev, "# %d x=%d y=%d", i, x, y);
        } else if (was_touched && (x == HX_EMPTY && y == HX_EMPTY)) {
            report_event = true;
            dev_dbg(&priv->client->dev, "# %d released", i);
        }

        if (report_event) {
            if (now_touched) {
				input_report_abs(input, ABS_MT_POSITION_X, x);
				input_report_abs(input, ABS_MT_POSITION_Y, y);

				input_report_abs(input, ABS_MT_PRESSURE, 200);
				input_report_abs(input, ABS_MT_TOUCH_MAJOR, 200);
				input_report_abs(input, ABS_MT_TRACKING_ID, i);
            }
            input_mt_sync(input);
        }
    }
#else
	if (touch_count == 1) {
		x = (buf[0 * HX_PNT_SIZE + 0] << 8) | buf[0 * HX_PNT_SIZE + 1];
    	y = (buf[0 * HX_PNT_SIZE + 2] << 8) | buf[0 * HX_PNT_SIZE + 3];

    	himax_ts_set_coords(props, &x, &y);
    	input_report_abs(input, ABS_X, x);
		input_report_abs(input, ABS_Y, y);
		input_report_abs(input, ABS_PRESSURE, 200);
		input_report_key(input, BTN_TOUCH, 1);
    } else if (touch_count == 0) {
    	input_report_abs(input, ABS_PRESSURE, 0);
		input_report_key(input, BTN_TOUCH, 0);
    }
#endif
    input_sync(input);

    priv->prev_touches = curr_touches;

    return IRQ_HANDLED;
}

static int himax_ts_setup(struct himax_ts_priv *priv)
{
    struct himax_ts_props_entry *props = priv->ts_props;
    struct i2c_client *client = priv->client;
    char *cmd = hx85x_get_sleep_cmd;
    char buf = 0x00;
    int count, i;

    if (i2c_master_send(client, cmd, 1) != 1)
        goto err_stop_seq;

    if (i2c_master_recv(client, &buf, 1) != 1) {
        dev_err(&client->dev, "Failed to read get sleep data\n");
        return -EBUSY;
    }

    if (buf != 0) {
        dev_dbg(&client->dev, "already initialized 0x%02X\n", buf);
        return 0;
    }

    for (i = 0; i < props->initseq_size; i++) {
        cmd = props->initseq[i].cmd;
        count = props->initseq[i].count;

        if (i2c_master_send(client, cmd, count) != count)
            goto err_stop_seq;

        msleep(props->initseq[i].delay_ms);
    }

    return 0;

err_stop_seq:
    dev_err(&client->dev, "Failed to send I2C command 0x%02X\n", cmd[0]);
    return -EBUSY;
}

static int himax_ts_open(struct input_dev *dev)
{
    struct himax_ts_priv *priv = input_get_drvdata(dev);
    struct i2c_client *client = priv->client;

    if (i2c_master_send(client, hx85x_senseon_cmd, 1) != 1) {
        dev_err(&priv->client->dev, "failed to write sense on cmd\n");
        return -EBUSY;
    }

    msleep(100);

    return 0;
}

static void himax_ts_close(struct input_dev *dev)
{
    struct himax_ts_priv *priv = input_get_drvdata(dev);
    struct i2c_client *client = priv->client;

    if (i2c_master_send(client, hx85x_senseoff_cmd, 1) != 1)
        dev_err(&priv->client->dev, "failed to write sense off cmd\n");
}

static struct input_dev *himax_ts_init_input(struct himax_ts_priv *priv)
{
    struct input_dev *input;

    input = input_allocate_device();
    if (!input) {
        dev_err(&priv->client->dev, "Failed to allocate input dev\n");
        return NULL;
    }

    input->name = priv->client->name;
    input->phys = priv->client->adapter->name,
    input->id.bustype = BUS_I2C;
    input->dev.parent = &priv->client->dev;
    input->open = himax_ts_open;
    input->close = himax_ts_close;

	set_bit(EV_SYN, input->evbit);
	set_bit(EV_ABS, input->evbit);
	set_bit(EV_KEY, input->evbit);

#ifdef CONFIG_HIMAX_MULTITOUCH
	set_bit(ABS_MT_TRACKING_ID, input->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, input->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input->absbit);
	set_bit(ABS_MT_POSITION_X, input->absbit);
	set_bit(ABS_MT_POSITION_Y, input->absbit);
	set_bit(INPUT_PROP_DIRECT, input->propbit);

	input_set_abs_params(input, ABS_MT_POSITION_X, 0, HX_MAX_X, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, HX_MAX_Y, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 0xFF, 0, 0);
	input_set_abs_params(input, ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, priv->ts_props->touch_points, 0, 0);
#else
	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	set_bit(ABS_PRESSURE, input->absbit);
	set_bit(BTN_TOUCH, input->keybit);

	input_set_abs_params(input, ABS_X, 0, HX_MAX_X, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, HX_MAX_Y, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, 0, 0xFF, 0, 0);
#endif
    input_set_drvdata(input, priv);

    return input;
}

static int himax_ts_probe(struct i2c_client *client,
            const struct i2c_device_id *idp)
{
    struct himax_ts_priv *priv;
    struct himax_ts_props_entry *ts_props = NULL;
    int error, i;
    char buf[3];
    int chip_model;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->dev, "I2C_FUNC_I2C check failed\n");
        return -EBUSY;
    }

    if (i2c_master_send(client, hx85x_get_id_cmd, 1) == 1 &&
        i2c_master_recv(client, buf, 3) == 3) {
        dev_info(&client->dev, "Found device ID: 0x%02X%02X%02X\n",
             buf[0], buf[1], buf[2]);
    } else {
        dev_err(&client->dev, "Unable to get DevId\n");
        return -ENODEV;
    }

    chip_model = (buf[0] << 8) | buf[1];

    for (i = 0; i < ARRAY_SIZE(himax_ts_props); i++) {
        if (chip_model == himax_ts_props[i].model) {
            ts_props = &himax_ts_props[i];
            break;
        }
    }

    if (!ts_props) {
        dev_err(&client->dev, "Unsupported device model\n");
        return -ENODEV;
    } else if (ts_props->model != idp->driver_data) {
        dev_warn(&client->dev,
        "Requested model 0x%04X not found, proceed with 0x%04X setup\n",
            (unsigned int)idp->driver_data, ts_props->model);
    }

    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&client->dev, "failed to allocate driver data\n");
        return -ENOMEM;
    }

    priv->client = client;
    priv->irq = client->irq;
    priv->ts_props = ts_props;

    error = himax_ts_setup(priv);
    if (error)
        goto err_free_mem;

    priv->buf = kzalloc(priv->ts_props->packet_size, GFP_KERNEL);
    if (!priv->buf) {
        dev_err(&client->dev, "failed to allocate read buffer\n");
        error = -ENOMEM;
        goto err_free_mem;
    }

    priv->input = himax_ts_init_input(priv);
    if (!priv->input) {
        error = -ENOMEM;
        goto err_free_mem;
    }

    error = input_register_device(priv->input);
    if (error) {
        dev_err(&client->dev, "Failed to register input device.\n");
        goto err_free_mem;
    }

    error = request_threaded_irq(priv->irq, NULL, himax_ts_isr,
                    IRQF_TRIGGER_LOW | IRQF_ONESHOT,
                    client->name, priv);
    if (error) {
        dev_err(&client->dev, "Unable to request touchscreen IRQ.\n");
        goto err_free_dev;
    }

    i2c_set_clientdata(client, priv);
    device_init_wakeup(&client->dev, 1);

	board_set_ctp(CTP_HIMAX);
    return 0;

err_free_dev:
    input_unregister_device(priv->input);
    priv->input = NULL;
err_free_mem:
    input_free_device(priv->input);
    kfree(priv->buf);
    kfree(priv);

    return error;
}

static int himax_ts_remove(struct i2c_client *client)
{
    struct himax_ts_priv *priv = i2c_get_clientdata(client);

    free_irq(priv->irq, priv);
    i2c_set_clientdata(client, NULL);
    input_unregister_device(priv->input);
    kfree(priv->buf);
    kfree(priv);

    return 0;
}

static int himax_ts_suspend(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct himax_ts_priv *priv = i2c_get_clientdata(client);

    if (device_may_wakeup(&client->dev))
        enable_irq_wake(priv->irq);
    else if (i2c_master_send(client, hx85x_senseoff_cmd, 1) != 1)
        dev_err(&priv->client->dev, "failed to write sense off cmd\n");

    return 0;
}

static int himax_ts_resume(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct himax_ts_priv *priv = i2c_get_clientdata(client);

    himax_ts_setup(priv);

    if (i2c_master_send(client, hx85x_senseon_cmd, 1) != 1)
        dev_err(&priv->client->dev, "failed to write sense on cmd\n");

    msleep(100);

    if (device_may_wakeup(&client->dev))
        disable_irq_wake(priv->irq);

    return 0;
}

static SIMPLE_DEV_PM_OPS(himax_ts_pm, himax_ts_suspend, himax_ts_resume);

static const struct i2c_device_id himax_ts_id[] = {
    { "hx8520-c"    , 0x8520 },
    { "hx8526-a"    , 0x8526 },
    { "hx8528-a"    , 0x8528 },
    { },
};

MODULE_DEVICE_TABLE(i2c, himax_ts_id);

static struct i2c_driver himax_ts_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "himax_ts",
        .pm = &himax_ts_pm,
    },
    .probe = himax_ts_probe,
    .remove = himax_ts_remove,
    .id_table = himax_ts_id,
};

static int __init himax_ts_init(void)
{
    return i2c_add_driver(&himax_ts_driver);
}

static void __exit himax_ts_exit(void)
{
    i2c_del_driver(&himax_ts_driver);
}

module_init(himax_ts_init);
module_exit(himax_ts_exit);

MODULE_DESCRIPTION("Himax Touchscreen driver");
MODULE_LICENSE("GPL v2");
