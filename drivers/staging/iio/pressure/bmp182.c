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
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include "../iio.h"
#include "../events.h"
#include "../sysfs.h"

#define DRIVER_VERSION		"1.0"

/* Register definitions */
#define BMP182_TAKE_MEAS_REG		0xf4
#define BMP182_READ_MEAS_REG_U		0xf6
#define BMP182_READ_MEAS_REG_L		0xf7
#define BMP182_READ_MEAS_REG_XL		0xf8

/*
 * Bytes defined by the spec to take measurements
 * Temperature will take 4.5ms before EOC
 */
#define BMP182_MEAS_TEMP		0x2e
/* 4.5ms wait for measurement */
#define BMP182_MEAS_PRESS_OVERSAMP_0	0x34
/* 7.5ms wait for measurement */
#define BMP182_MEAS_PRESS_OVERSAMP_1	0x74
/* 13.5ms wait for measurement */
#define BMP182_MEAS_PRESS_OVERSAMP_2	0xb4
/* 25.5ms wait for measurement */
#define BMP182_MEAS_PRESS_OVERSAMP_3	0xf4

/*
 * EEPROM registers each is a two byte value so there is
 * an upper byte and a lower byte
 */
#define BMP182_EEPROM_AC1_U	0xaa
#define BMP182_EEPROM_AC1_L	0xab
#define BMP182_EEPROM_AC2_U	0xac
#define BMP182_EEPROM_AC2_L	0xad
#define BMP182_EEPROM_AC3_U	0xae
#define BMP182_EEPROM_AC3_L	0xaf
#define BMP182_EEPROM_AC4_U	0xb0
#define BMP182_EEPROM_AC4_L	0xb1
#define BMP182_EEPROM_AC5_U	0xb2
#define BMP182_EEPROM_AC5_L	0xb3
#define BMP182_EEPROM_AC6_U	0xb4
#define BMP182_EEPROM_AC6_L	0xb5
#define BMP182_EEPROM_B1_U	0xb6
#define BMP182_EEPROM_B1_L	0xb7
#define BMP182_EEPROM_B2_U	0xb8
#define BMP182_EEPROM_B2_L	0xb9
#define BMP182_EEPROM_MB_U	0xba
#define BMP182_EEPROM_MB_L	0xbb
#define BMP182_EEPROM_MC_U	0xbc
#define BMP182_EEPROM_MC_L	0xbd
#define BMP182_EEPROM_MD_U	0xbe
#define BMP182_EEPROM_MD_L	0xbf

#define SAMP_FREQ_MAX		20
#define SAMP_FREQ_MIN		1
#define SAMP_FREQ_DEFAULT	5

#define PRESSURE_MAX		125000
#define PRESSURE_MIN		95000
#define PRESSURE_FUZZ		5
#define PRESSURE_FLAT		5

struct bmp182_eeprom_data {
	s16 AC1, AC2, AC3;
	u16 AC4, AC5, AC6;
	s16 B1, B2;
	s16 MB, MC, MD;
} __aligned(2) __packed;

struct bmp182_data {
	struct i2c_client *client;
	struct mutex lock;
	struct workqueue_struct *wq;
	struct work_struct work_pressure;
	bool on_before_suspend;
	bool enabled;
	u8 oversampling_rate;
	int pressure;
	struct hrtimer timer;
	int sampling_freq;
	ktime_t poll_delay;
	struct bmp182_eeprom_data bmp182_eeprom_vals;
};

static void bmp182_enable(struct bmp182_data *barom)
{
	if (!barom->enabled) {
		barom->enabled = true;
		pr_debug("start timer\n");
		hrtimer_start(&barom->timer, barom->poll_delay,
			HRTIMER_MODE_REL);
	}
}

static void bmp182_disable(struct bmp182_data *barom)
{
	if (barom->enabled) {
		barom->enabled = false;
		pr_debug("stop timer\n");
		hrtimer_cancel(&barom->timer);
		cancel_work_sync(&barom->work_pressure);
	}
}

static int bmp182_get_raw_temperature(struct bmp182_data *barom,
					u16 *raw_temperature)
{
	int err;
	u16 buf;

	err = i2c_smbus_write_byte_data(barom->client,
				BMP182_TAKE_MEAS_REG,
				BMP182_MEAS_TEMP);
	if (err) {
		pr_err("can't write BMP182_TAKE_MEAS_REG\n");
		return err;
	}

	usleep_range(5000, 6000);

	err = i2c_smbus_read_i2c_block_data(barom->client,
				BMP182_READ_MEAS_REG_U,
				sizeof(buf), (u8 *)&buf);
	if (err != sizeof(buf)) {
		pr_err("Fail to read uncompensated temperature\n");
		return err >= 0 ? -EIO : err;
	}
	*raw_temperature = be16_to_cpu(buf);
	pr_debug("uncompensated temperature:  %d\n", *raw_temperature);
	return 0;
}

static int bmp182_get_raw_pressure(struct bmp182_data *barom,
					u32 *raw_pressure)
{
	int err;
	u32 buf = 0;
	int range;

	err = i2c_smbus_write_byte_data(barom->client,
				BMP182_TAKE_MEAS_REG,
				BMP182_MEAS_PRESS_OVERSAMP_0 |
				(barom->oversampling_rate << 6));
	if (err) {
		pr_err("can't write BMP182_TAKE_MEAS_REG\n");
		return err;
	}

	range = 2 + (3 << barom->oversampling_rate);
	usleep_range(range * 1000, (range + 1) * 1000);

	err = i2c_smbus_read_i2c_block_data(barom->client,
			BMP182_READ_MEAS_REG_U, 3, ((u8 *)&buf) + 1);
	if (err != 3) {
		pr_err("Fail to read uncompensated pressure\n");
		return err >= 0 ? -EIO : err;
	}

	*raw_pressure = be32_to_cpu(buf);
	*raw_pressure >>= (8 - barom->oversampling_rate);
	pr_debug("uncompensated pressure:  %d\n", *raw_pressure);
	return 0;
}

static void bmp182_get_pressure_data(struct work_struct *work)
{
	u16 raw_temperature;
	u32 raw_pressure;
	s32 x1, x2, x3, b3, b5, b6;
	u32 b4;
	s32 p;

	struct bmp182_data *barom =
	    container_of(work, struct bmp182_data, work_pressure);

	if (bmp182_get_raw_temperature(barom, &raw_temperature)) {
		pr_err("can't read uncompensated temperature\n");
		return;
	}

	if (bmp182_get_raw_pressure(barom, &raw_pressure)) {
		pr_err("Fail to read uncompensated pressure\n");
		return;
	}

	/* voodoo from BMP182 data sheet, BST-BMP182-DS000-00, page 15 */
	x1 = ((raw_temperature - barom->bmp182_eeprom_vals.AC6) *
	      barom->bmp182_eeprom_vals.AC5) >> 15;
	x2 = (barom->bmp182_eeprom_vals.MC << 11) /
	    (x1 + barom->bmp182_eeprom_vals.MD);

	b5 = (x1 + x2 - 4000);
	x1 = (barom->bmp182_eeprom_vals.B2 * ((b5 * b5) >> 12));
	x2 = (barom->bmp182_eeprom_vals.AC2 * b5);
	x3 = (x1 + x2) >> 11;
	b3 = (((((s32)barom->bmp182_eeprom_vals.AC1) * 4 +
		x3) << barom->oversampling_rate) + 2) >> 2;
	x1 = (barom->bmp182_eeprom_vals.AC3 * b5) >> 13;
	x2 = (barom->bmp182_eeprom_vals.B1 * (b5 * b5 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (barom->bmp182_eeprom_vals.AC4 *
	      (u32)(x3 + 32768)) >> 15;
	b6 = (raw_pressure - b3) *
		(50000 >> barom->oversampling_rate);
	if (b6 < 0x80000000)
		p = (b6 * 2) / b4;
	else
		p = (b6 / b4) * 2;

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;

	barom->pressure = p + ((x1 + x2 + 3791) >> 4);
	pr_debug("calibrated pressure: %d\n", barom->pressure);

	if (iio_push_event(iio_priv_to_dev(barom),
			IIO_UNMOD_EVENT_CODE(IIO_PRESSURE,
				0,
				IIO_EV_TYPE_THRESH,
				IIO_EV_DIR_EITHER),
			iio_get_time_ns()))
		pr_err("Could not push IIO_PRESSURE event");
}

static int __devinit bmp182_read_store_eeprom_val(struct bmp182_data *barom)
{
	int err;

	err = i2c_smbus_read_i2c_block_data(barom->client,
				BMP182_EEPROM_AC1_U,
				sizeof(barom->bmp182_eeprom_vals),
				(u8 *)&(barom->bmp182_eeprom_vals));
	if (err != sizeof(barom->bmp182_eeprom_vals)) {
		pr_err("Cannot read EEPROM values\n");
		return err >= 0 ? -EIO : err;
	}
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.AC1));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.AC2));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.AC3));
	be16_to_cpus(&(barom->bmp182_eeprom_vals.AC4));
	be16_to_cpus(&(barom->bmp182_eeprom_vals.AC5));
	be16_to_cpus(&(barom->bmp182_eeprom_vals.AC6));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.B1));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.B2));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.MB));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.MC));
	be16_to_cpus((u16 *)&(barom->bmp182_eeprom_vals.MD));
	return 0;
}

static enum hrtimer_restart bmp182_timer_func(struct hrtimer *timer)
{
	struct bmp182_data *barom = container_of(timer,
		struct bmp182_data, timer);

	pr_debug("start\n");
	queue_work(barom->wq, &barom->work_pressure);
	hrtimer_forward_now(&barom->timer, barom->poll_delay);
	return HRTIMER_RESTART;
}

static ssize_t bmp182_sampling_frequency_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct bmp182_data *barom = iio_priv(dev_get_drvdata(dev));

	return sprintf(buf, "%d Hz\n", barom->sampling_freq);
}

static ssize_t bmp182_sampling_frequency_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t size)
{
	unsigned int new_value;
	struct bmp182_data *barom = iio_priv(dev_get_drvdata(dev));
	int err;

	err = kstrtouint(buf, 10, &new_value);
	if (err)
		return err;

	if (new_value < SAMP_FREQ_MIN)
		new_value = SAMP_FREQ_MIN;
	else if (new_value > SAMP_FREQ_MAX)
		new_value = SAMP_FREQ_MAX;

	pr_debug("new frequency = %dHz\n", new_value);

	mutex_lock(&barom->lock);
	if (new_value != barom->sampling_freq) {
		barom->sampling_freq = new_value;
		barom->poll_delay = ns_to_ktime(NSEC_PER_SEC / new_value);
	}
	mutex_unlock(&barom->lock);

	return size;
}

static ssize_t bmp182_oversampling_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct bmp182_data *barom = iio_priv(dev_get_drvdata(dev));

	return sprintf(buf, "%d\n", barom->oversampling_rate);
}

static ssize_t bmp182_oversampling_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	struct bmp182_data *barom = iio_priv(dev_get_drvdata(dev));
	unsigned long oversampling;
	int err = kstrtoul(buf, 10, &oversampling);

	if (err)
		return err;
	if (oversampling > 3)
		oversampling = 3;
	barom->oversampling_rate = oversampling;
	return count;
}

static int bmp182_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int *val,
				int *val2,
				long mask)
{
	struct bmp182_data *barom = iio_priv(indio_dev);

	*val = barom->pressure;
	return IIO_VAL_INT;
}

static int bmp182_read_event_config(struct iio_dev *indio_dev,
						u64 event_code)
{
	struct bmp182_data *barom = iio_priv(indio_dev);

	return barom->enabled;
}

static int bmp182_write_event_config(struct iio_dev *indio_dev,
						u64 event_code,
						int state)
{
	struct bmp182_data *barom = iio_priv(indio_dev);

	pr_debug("enable = %d, old state = %d\n",
		state, barom->enabled);

	mutex_lock(&barom->lock);
	if (state)
		bmp182_enable(barom);
	else
		bmp182_disable(barom);
	mutex_unlock(&barom->lock);

	return 0;
}

static int bmp182_read_event_value(struct iio_dev *indio_dev,
						u64 event_code,
						int *val)
{
	struct bmp182_data *barom = iio_priv(indio_dev);

	*val = barom->pressure;
	return 0;
}

static IIO_DEV_ATTR_SAMP_FREQ(S_IRUGO | S_IWUSR,
		bmp182_sampling_frequency_show,
		bmp182_sampling_frequency_store);
static IIO_CONST_ATTR_SAMP_FREQ_AVAIL("1 ~ 20Hz");
static IIO_DEVICE_ATTR(oversampling, S_IRUGO | S_IWUSR,
		bmp182_oversampling_show,
		bmp182_oversampling_store,
		0);
static IIO_CONST_ATTR(oversampling_modes, "0 1 2 3");

#define BMP182_DEV_ATTR(name) (&iio_dev_attr_##name.dev_attr.attr)
#define BMP182_CONST_ATTR(name) (&iio_const_attr_##name.dev_attr.attr)
static struct attribute *bmp182_sysfs_attrs[] = {
	BMP182_DEV_ATTR(sampling_frequency),
	BMP182_CONST_ATTR(sampling_frequency_available),
	BMP182_DEV_ATTR(oversampling),
	BMP182_CONST_ATTR(oversampling_modes),
	NULL
};

static struct attribute_group bmp182_attribute_group = {
	.attrs = bmp182_sysfs_attrs,
};

static const struct iio_info bmp182_info = {
	.attrs = &bmp182_attribute_group,
	.read_raw = bmp182_read_raw,
	.read_event_config = bmp182_read_event_config,
	.write_event_config = bmp182_write_event_config,
	.read_event_value = bmp182_read_event_value,
	.driver_module = THIS_MODULE,
};

static const struct iio_chan_spec bmp182_channels[] = {
	{
		.type = IIO_PRESSURE,
		.indexed = 1,
		.channel = 0,
		.processed_val = IIO_PROCESSED,
		.event_mask = IIO_EV_BIT(IIO_EV_TYPE_THRESH,
					IIO_EV_DIR_EITHER),
	}
};

static int __devinit bmp182_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err;
	struct bmp182_data *barom;
	struct iio_dev *indio_dev;

	pr_debug("enter\n");
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_WRITE_BYTE |
				I2C_FUNC_SMBUS_READ_I2C_BLOCK)) {
		pr_err("client not i2c capable\n");
		return -ENOSYS;
	}

	indio_dev = iio_allocate_device(sizeof(*barom));
	if (!indio_dev) {
		pr_err("failed to allocate memory for iio device\n");
		return -ENOMEM;
	}

	barom = iio_priv(indio_dev);
	mutex_init(&barom->lock);
	barom->client = client;

	i2c_set_clientdata(client, indio_dev);

	err = bmp182_read_store_eeprom_val(barom);
	if (err) {
		pr_err("Reading the EEPROM failed\n");
		err = -ENODEV;
		goto err_read_eeprom;
	}

	hrtimer_init(&barom->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	barom->sampling_freq = SAMP_FREQ_DEFAULT;
	barom->poll_delay = ns_to_ktime(NSEC_PER_SEC / SAMP_FREQ_DEFAULT);
	barom->timer.function = bmp182_timer_func;

	barom->wq = alloc_workqueue("bmp182_wq",
		WQ_UNBOUND | WQ_RESCUER, 1);
	if (!barom->wq) {
		err = -ENOMEM;
		pr_err("could not create workqueue\n");
		goto err_create_workqueue;
	}

	INIT_WORK(&barom->work_pressure, bmp182_get_pressure_data);

	indio_dev->name = "barometer";
	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &bmp182_info;
	indio_dev->channels = bmp182_channels;
	indio_dev->num_channels = ARRAY_SIZE(bmp182_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	err = iio_device_register(indio_dev);
	if (err)
		goto err_iio_register;

	pr_debug("%s sensor registered\n", id->name);
	return 0;

err_iio_register:
	destroy_workqueue(barom->wq);
err_create_workqueue:
err_read_eeprom:
	mutex_destroy(&barom->lock);
	iio_free_device(indio_dev);
	return err;
}

static int __devexit bmp182_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct bmp182_data *barom = iio_priv(indio_dev);

	bmp182_disable(barom);
	destroy_workqueue(barom->wq);
	mutex_destroy(&barom->lock);
	iio_device_unregister(indio_dev);
	iio_free_device(indio_dev);
	return 0;
}

static int bmp182_resume(struct device *dev)
{
	struct bmp182_data *barom = iio_priv(i2c_get_clientdata(
					to_i2c_client(dev)));

	pr_debug("on_before_suspend %d\n", barom->on_before_suspend);

	if (barom->on_before_suspend)
		bmp182_enable(barom);
	return 0;
}

static int bmp182_suspend(struct device *dev)
{
	struct bmp182_data *barom = iio_priv(i2c_get_clientdata(
					to_i2c_client(dev)));

	barom->on_before_suspend = barom->enabled;
	pr_debug("on_before_suspend %d\n", barom->on_before_suspend);
	bmp182_disable(barom);
	return 0;
}

static const struct i2c_device_id bmp182_id[] = {
	{"bmp182", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, bmp182_id);
static const struct dev_pm_ops bmp182_pm_ops = {
	.suspend	= bmp182_suspend,
	.resume		= bmp182_resume,
};

static struct i2c_driver bmp182_driver = {
	.driver = {
		.name	= "bmp182",
		.owner	= THIS_MODULE,
		.pm	= &bmp182_pm_ops,
	},
	.probe		= bmp182_probe,
	.remove		= __devexit_p(bmp182_remove),
	.id_table	= bmp182_id,
};

module_i2c_driver(bmp182_driver);

MODULE_AUTHOR("Hyoung Wook Ham <hwham@sta.samsung.com>");
MODULE_DESCRIPTION("BMP182 Pressure sensor driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRIVER_VERSION);
