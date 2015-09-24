/*
 * stmpe811-adc.c
 *
 * Copyright (C) 2012 Samsung Electronics
 * SangYoung Son <hello.son@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/platform_data/stmpe811-adc.h>

#define STMPE811_CHIP_ID	0x00
#define STMPE811_ID_VER		0x02
#define STMPE811_SYS_CTRL1	0x03
#define STMPE811_SYS_CTRL2	0x04
#define STMPE811_INT_CTRL	0x09
#define STMPE811_INT_EN		0x0A
#define STMPE811_INT_STA	0x0B
#define STMPE811_ADC_INT_EN	0x0E
#define STMPE811_ADC_INT_STA	0x0F
#define STMPE811_ADC_CTRL1	0x20
#define STMPE811_ADC_CTRL2	0x21
#define STMPE811_ADC_CAPT	0x22
#define STMPE811_ADC_DATA_CH0	0x30
#define STMPE811_ADC_DATA_CH1	0x32
#define STMPE811_ADC_DATA_CH2	0x34
#define STMPE811_ADC_DATA_CH3	0x36
#define STMPE811_ADC_DATA_CH4	0x38
#define STMPE811_ADC_DATA_CH5	0x3A
#define STMPE811_ADC_DATA_CH6	0x3C
#define STMPE811_ADC_DATA_CH7	0x3E
#define STMPE811_GPIO_AF	0x17
#define STMPE811_TSC_CTRL	0x40

static struct i2c_client *stmpe811_adc_i2c_client;

struct stmpe811_adc_data {
	struct i2c_client	*client;
	struct stmpe811_platform_data	*pdata;
	struct stmpe811_callbacks		callbacks;

	struct mutex		adc_lock;
};

static int stmpe811_i2c_read(struct i2c_client *client, u8 reg, u8 *data,
			     u8 length)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(client, reg, length, data);
	if (ret != length) {
		dev_err(&client->dev, "%s: err %d, reg: 0x%02x\n", __func__,
			ret, reg);
		return -EIO;
	}

	return 0;
}

static int stmpe811_write_register(struct i2c_client *client, u8 reg,
				   u16 w_data)
{
	int ret;

	ret = i2c_smbus_write_word_data(client, reg, w_data);
	if (ret < 0) {
		dev_err(&client->dev, "%s: err %d, reg: 0x%02x\n", __func__,
			ret, reg);
		return ret;
	}

	return 0;
}

static int stmpe811_get_adc_data(u8 channel)
{
	struct i2c_client *client = stmpe811_adc_i2c_client;
	u8 data[2];
	u16 w_data;
	u8 data_channel_addr;
	int ret;

	ret = stmpe811_write_register(client, STMPE811_ADC_CAPT,
				      (1 << channel));
	if (ret < 0)
		return ret;

	msleep(20);
	data_channel_addr = STMPE811_ADC_DATA_CH0 + (channel * 2);
	ret = stmpe811_i2c_read(client, data_channel_addr, data, 2);
	if (ret < 0)
		return ret;

	w_data = ((data[0]<<8) | data[1]) & 0x0FFF;
	pr_debug("%s: STMPE811_ADC_DATA_CH%d(0x%x, %d)\n", __func__,
				channel, w_data, w_data);

	return w_data;
}

static int stmpe811_reg_init(struct stmpe811_adc_data *adc_data)
{
	struct i2c_client *client = adc_data->client;
	int ret;

	/* clock control: only adc on */
	ret = stmpe811_write_register(client, STMPE811_SYS_CTRL2, 0x0E);
	if (ret < 0)
		goto reg_init_error;

	/* interrupt enable: disable interrupt */
	ret = stmpe811_write_register(client, STMPE811_INT_EN, 0x00);
	if (ret < 0)
		goto reg_init_error;

	/* adc control: 64 sample time, 12bit adc, internel referance*/
	ret = stmpe811_write_register(client, STMPE811_ADC_CTRL1, 0x38);
	if (ret < 0)
		goto reg_init_error;

	/* adc control: 1.625MHz typ */
	ret = stmpe811_write_register(client, STMPE811_ADC_CTRL2, 0x03);
	if (ret < 0)
		goto reg_init_error;

	/* alt func: use for adc */
	ret = stmpe811_write_register(client, STMPE811_GPIO_AF, 0x00);
	if (ret < 0)
		goto reg_init_error;

	/* ts control: tsc disable */
	ret = stmpe811_write_register(client, STMPE811_TSC_CTRL, 0x00);
	if (ret < 0)
		goto reg_init_error;

	return 0;

reg_init_error:
	dev_err(&client->dev, "%s: reg init error: %d\n", __func__, ret);
	return ret;
}

static const struct file_operations stmpe811_fops = {
	.owner = THIS_MODULE,
};

static struct miscdevice stmpe811_adc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sec_adc",
	.fops = &stmpe811_fops,
};

static int __devinit stmpe811_adc_i2c_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct stmpe811_platform_data *pdata = client->dev.platform_data;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct stmpe811_adc_data *adc_data;
	int ret;
	u8 i2c_data[2];
	u8 rev[2];

	if (pdata == NULL) {
		dev_err(&client->dev, "%s: no pdata\n", __func__);
		return -ENODEV;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	adc_data = kzalloc(sizeof(*adc_data), GFP_KERNEL);
	if (!adc_data)
		return -ENOMEM;

	adc_data->client = client;
	adc_data->pdata = pdata;
	adc_data->callbacks.get_adc_data = stmpe811_get_adc_data;
	if (pdata->register_cb)
		pdata->register_cb(&adc_data->callbacks);

	i2c_set_clientdata(client, adc_data);
	stmpe811_adc_i2c_client = client;

	ret = misc_register(&stmpe811_adc_device);
	if (ret)
		goto misc_register_fail;

	mutex_init(&adc_data->adc_lock);

	/* initialize adc registers */
	ret = stmpe811_reg_init(adc_data);
	if (ret < 0)
		goto reg_init_error;

	/* TODO: ADC_INT setting */

	ret = stmpe811_i2c_read(client, STMPE811_CHIP_ID, i2c_data, 2);
	if (ret < 0)
		goto reg_init_error;
	/* read revision number, 0x01 for es, 0x03 for final silicon */
	ret = stmpe811_i2c_read(client, STMPE811_ID_VER, rev, 2);
	if (ret < 0)
		goto reg_init_error;

	dev_info(&client->dev, "stmpe811 adc (id 0x%x rev 0x%x)\n",
		 ((i2c_data[0]<<8) | i2c_data[1]), rev[0]);
	return 0;

reg_init_error:
	mutex_destroy(&adc_data->adc_lock);
	misc_deregister(&stmpe811_adc_device);
misc_register_fail:
	if (pdata->register_cb)
		pdata->register_cb(NULL);

	dev_err(&client->dev, "stmpe811 probe fail: %d\n", ret);
	kfree(adc_data);
	return ret;
}

static int __devexit stmpe811_adc_i2c_remove(struct i2c_client *client)
{
	struct stmpe811_adc_data *adc = i2c_get_clientdata(client);

	misc_deregister(&stmpe811_adc_device);
	mutex_destroy(&adc->adc_lock);
	kfree(adc);

	return 0;
}

static const struct i2c_device_id stmpe811_adc_device_id[] = {
	{"stmpe811-adc", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, stmpe811_adc_device_id);

static struct i2c_driver stmpe811_adc_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "stmpe811-adc",
	},
	.probe		= stmpe811_adc_i2c_probe,
	.remove		= stmpe811_adc_i2c_remove,
	.id_table	= stmpe811_adc_device_id,
};

static int __init stmpe811_adc_init(void)
{
	return i2c_add_driver(&stmpe811_adc_i2c_driver);
}

static void __exit stmpe811_adc_exit(void)
{
	i2c_del_driver(&stmpe811_adc_i2c_driver);
}

module_init(stmpe811_adc_init);
module_exit(stmpe811_adc_exit);

MODULE_AUTHOR("SangYoung Son <hello.son@samsung.com>");
MODULE_DESCRIPTION("stmpe811 adc driver");
MODULE_LICENSE("GPL");
