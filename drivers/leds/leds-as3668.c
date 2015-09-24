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

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/leds-as3668.h>

#define AS3668_DRV_NAME				"as3668"
#define AS3668_CHIPID1				0xA5

/* AS3668 registers */
#define AS3668_REG_CURRX_CTRL			0x01
#define AS3668_REG_CURR1_CURRENT		0x02
#define AS3668_REG_CURR2_CURRENT		0x03
#define AS3668_REG_CURR3_CURRENT		0x04
#define AS3668_REG_CURR4_CURRENT		0x05
#define AS3668_REG_GPIO_CTRL			0x06
#define AS3668_REG_PWM_CTRL			0x15
#define AS3668_REG_PWM_TIMING			0x16
#define AS3668_REG_PATTERN_TIMING		0x18
#define AS3668_REG_FRAME_MASK			0x1A
#define AS3668_REG_PATTERN_START_CTRL		0x1B
#define AS3668_REG_FRAME_START_DELAY		0x1C
#define AS3668_REG_OVERTEMP_CTRL		0x29
#define AS3668_REG_CHIPID1			0x3E
#define AS3668_REG_CHIPID2			0x3F
#define AS3668_REG_AUDIO_INPUT_BUFFER		0x41
#define AS3668_REG_AUDIO_CTRL			0x42
#define AS3668_REG_AUDIO_OUTPUT			0x43

#define AS3668_PATTERN_START_ON			0x02

struct as3668_led {
	struct i2c_client *client;
	struct mutex lock;
	struct led_classdev ldev;
	u32 color;		/* 0xRRGGBBWW */
	u8 led_array[AS3668_LED_NUM];	/* C1 C2 C3 C4 */
	u8 pwm_dim_speed;
	u8 pattern_frame_mask;	/* 0b F4 F4 F3 F3 F2 F2 F1 F1 */
	u8 pattern_frame_delay;	/* 0b D4 D4 D3 D3 D2 D2 D1 D0 */
	bool blinking;
	u32 pattern;
};

enum AS3668_MODE {
	MODE_OFF		= 0x00,
	MODE_ON			= 0x55,
	MODE_PWM		= 0xAA,
	MODE_PATTERN		= 0xFF,
};

/* voodoo from AS3668 data sheet Revision 1.11, page 41 */
static const u16 pattern_toff_time[8] = {80, 150, 280, 540,
			1100, 2100, 4200, 8400};
static const u16 pattern_ton_time[8] = {40, 70, 140, 270,
			530, 1100, 2100, 4200};
/* voodoo from AS3668 data sheet Revision 1.11, page 39 */
static const u16 pattern_dim_speed_time[16] = {0, 120, 250,
			380, 510, 770, 1000, 1600, 2100, 2600,
			3100, 4200, 5200, 6200, 7300, 8300};

static s32 as3668_i2c_update_bits(struct i2c_client *client,
		u8 addr, u8 mask, u8 val)
{
	s32 ret;
	u8 value;
	ret = i2c_smbus_read_byte_data(client, addr);
	if (ret < 0) {
		pr_err("Can't read 0x%02x via i2c\n", addr);
		return ret;
	}

	value = ret & ~mask;
	value |= (val & mask);

	ret = i2c_smbus_write_byte_data(client, addr, value);
	if (ret) {
		pr_err("Can't write to 0x%02x via i2c\n", addr);
		return ret;
	}
	return 0;
}

static int as3668_binary_search(u16 value, const u16 array[], int low, int high)
{
	int mid;
	if (value >= array[high])
		return high;
	else if (value <= array[low])
		return low;

	while (high > low + 1) {
		mid = (high + low) / 2;
		if (value < array[mid])
			high = mid;
		else
			low = mid;
	}
	if ((array[high] - value) > (value - array[low]))
		mid = low;
	else
		mid = high;

	return mid;
}

static s32 as3668_set_color(const struct as3668_led *led, u8 weight,
		u32 color)
{
	int ret;
	int i;
	for (i = AS3668_REG_CURR1_CURRENT; i <= AS3668_REG_CURR4_CURRENT; i++) {
		ret = i2c_smbus_write_byte_data(led->client, i,
			((weight + 1) * (u8)(color >>
			led->led_array[i - AS3668_REG_CURR1_CURRENT]))
			>> 8);
		if (ret) {
			pr_err("Fail to set color\n");
			return ret;
		}
	}
	return 0;
}

static int as3668_update(struct as3668_led *led)
{
	int ret;
	enum AS3668_MODE mode;

	if (led->ldev.brightness == 0)
		mode = MODE_OFF;
	else if (led->blinking)
		mode = MODE_PATTERN;
	else
		mode = MODE_ON;

	if (led->blinking) {
		ret = i2c_smbus_write_byte_data(led->client,
			AS3668_REG_PATTERN_TIMING, led->pattern);
		if (ret) {
			pr_err("Can't update AS3668_REG_CURRX_CTRL\n");
			return ret;
		}
	}

	ret = i2c_smbus_write_byte_data(led->client, AS3668_REG_CURRX_CTRL,
		mode);
	if (ret) {
		pr_err("Can't write AS3668_REG_CURRX_CTRL\n");
		return ret;
	}

	ret = as3668_set_color(led, led->ldev.brightness, led->color);
	if (ret) {
		pr_err("Can't set color\n");
		return ret;
	}

	return 0;
}

static void as3668_set_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);

	mutex_lock(&led->lock);
	if (value == 0)
		led->blinking = false;
	led->ldev.brightness = (u8)value;
	as3668_update(led);
	mutex_unlock(&led->lock);
}

static int as3668_set_led_blink(struct led_classdev *led_cdev,
		unsigned long *delay_on,
		unsigned long *delay_off)
{
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	int ret;
	int pattern_on;
	int pattern_off;

	mutex_lock(&led->lock);
	led->blinking = true;

	pattern_on = as3668_binary_search(*delay_on, pattern_ton_time,
				0, ARRAY_SIZE(pattern_ton_time) - 1);
	*delay_on = pattern_ton_time[pattern_on];

	pattern_off = as3668_binary_search(*delay_off,
			pattern_toff_time, 0,
			ARRAY_SIZE(pattern_toff_time) - 1);
	*delay_off = pattern_toff_time[pattern_off];

	led->pattern = (0x07 & pattern_on) | (0x38 & (pattern_off << 3));
	ret = as3668_update(led);
	mutex_unlock(&led->lock);

	return ret;
}

static ssize_t as3668_color_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);

	return sprintf(buf, "0x%08x\n", led->color);
}

static ssize_t as3668_color_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	unsigned int new_value;
	int err;

	err = kstrtouint(buf, 16, &new_value);
	if (err)
		return err;

	mutex_lock(&led->lock);
	led->color = new_value;
	as3668_update(led);
	mutex_unlock(&led->lock);
	return size;
}

static ssize_t as3668_slope_up_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	return sprintf(buf, "%d\n",
		pattern_dim_speed_time[(led->pwm_dim_speed & 0xF0) >> 4]);
}

static ssize_t as3668_slope_up_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	u16 new_value;
	int err;
	int index;

	err = kstrtou16(buf, 10, &new_value);
	if (err)
		return err;

	index = as3668_binary_search(new_value, pattern_dim_speed_time,
			0, ARRAY_SIZE(pattern_dim_speed_time) - 1);

	mutex_lock(&led->lock);
	led->pwm_dim_speed &= 0x0F;
	led->pwm_dim_speed |= (0xF0 & index << 4);
	i2c_smbus_write_byte_data(led->client, AS3668_REG_PWM_TIMING,
				led->pwm_dim_speed);
	mutex_unlock(&led->lock);

	return size;
}

static ssize_t as3668_slope_down_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
		container_of(led_cdev, struct as3668_led, ldev);
	return sprintf(buf, "%d\n",
		pattern_dim_speed_time[led->pwm_dim_speed & 0x0F]);
}

static ssize_t as3668_slope_down_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
		container_of(led_cdev, struct as3668_led, ldev);
	u16 new_value;
	int err;
	int index;

	err = kstrtou16(buf, 10, &new_value);
	if (err)
		return err;

	index = as3668_binary_search(new_value, pattern_dim_speed_time,
			0, ARRAY_SIZE(pattern_dim_speed_time) - 1);

	mutex_lock(&led->lock);
	led->pwm_dim_speed &= 0xF0;
	led->pwm_dim_speed |= 0x0F & index;
	i2c_smbus_write_byte_data(led->client, AS3668_REG_PWM_TIMING,
				led->pwm_dim_speed);
	mutex_unlock(&led->lock);

	return size;
}

static ssize_t as3668_frame_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	return sprintf(buf, "0x%08x\n", led->pattern_frame_mask);
}

static ssize_t as3668_frame_mask_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	u8 new_value;
	int err;

	err = kstrtou8(buf, 16, &new_value);
	if (err)
		return err;

	mutex_lock(&led->lock);
	led->pattern_frame_mask = new_value;
	i2c_smbus_write_byte_data(led->client, AS3668_REG_FRAME_MASK,
					new_value);
	mutex_unlock(&led->lock);

	return size;
}

static ssize_t as3668_frame_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	return sprintf(buf, "0x%08x\n", led->pattern_frame_delay);
}

static ssize_t as3668_frame_delay_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3668_led *led =
	    container_of(led_cdev, struct as3668_led, ldev);
	u8 new_value;
	int err;

	err = kstrtou8(buf, 16, &new_value);
	if (err)
		return err;

	mutex_lock(&led->lock);
	led->pattern_frame_delay = new_value;
	i2c_smbus_write_byte_data(led->client, AS3668_REG_FRAME_START_DELAY,
					new_value);
	mutex_unlock(&led->lock);

	return size;
}

static int __devinit as3668_initialize(struct i2c_client *client,
		struct as3668_platform_data *pdata)
{
	s32 ret;

	ret = i2c_smbus_read_byte_data(client, AS3668_REG_CHIPID1);
	if (ret < 0) {
		pr_err("Fail to read chip ID1\n");
		goto err_init;
	}
	if (ret != AS3668_CHIPID1) {
		pr_err("Unsupported chip_id=0x%02x).\n", ret);
		ret = -ENODEV;
		goto err_init;
	}
	ret = i2c_smbus_read_byte_data(client, AS3668_REG_CHIPID2);
	if (ret < 0) {
		pr_err("Fail to read chip ID2\n");
		goto err_init;
	}
	pr_debug("AS3668 chip id2:0x%02x rev:%d\n", ret & 0xF0, ret & 0x0F);

	ret = i2c_smbus_write_byte_data(client, AS3668_REG_CURRX_CTRL,
			 MODE_OFF);
	if (ret) {
		pr_err("Can't write AS3668_REG_CURRX_CTRL\n");
		goto err_init;
	}
	ret = as3668_i2c_update_bits(client, AS3668_REG_PATTERN_START_CTRL,
			0x03, pdata->pattern_start_source |
			AS3668_PATTERN_START_ON);
	if (ret) {
		pr_err("Can't update AS3668_REG_PATTERN_START_CTRL\n");
		goto err_init;
	}
	ret = as3668_i2c_update_bits(client, AS3668_REG_PWM_CTRL, 0x01,
			pdata->pwm_source);
	if (ret) {
		pr_err("Can't update AS3668_REG_PWM_CTRL\n");
		goto err_init;
	}
	ret = as3668_i2c_update_bits(client, AS3668_REG_GPIO_CTRL, 0x07,
			pdata->gpio_input_invert << 2 |
			pdata->gpio_input_mode << 1 |
			pdata->gpio_mode);
	if (ret) {
		pr_err("Can't update AS3668_REG_GPIO_CTRL\n");
		goto err_init;
	}
	ret = as3668_i2c_update_bits(client, AS3668_REG_AUDIO_CTRL, 0xE0,
			pdata->audio_input_pin << 7 |
			pdata->audio_pulldown_off << 6 |
			pdata->audio_adc_characteristic << 5);
	if (ret) {
		pr_err("Can't update AS3668_REG_AUDIO_CTRL\n");
		goto err_init;
	}
	ret = as3668_i2c_update_bits(client, AS3668_REG_AUDIO_INPUT_BUFFER,
			0xC0,
			pdata->audio_dis_start << 7 |
			pdata->audio_man_start << 6);
	if (ret) {
		pr_err("Can't update AS3668_REG_AUDIO_INPUT_BUFFER\n");
		goto err_init;
	}

	ret = as3668_i2c_update_bits(client, AS3668_REG_OVERTEMP_CTRL, 0x70,
			pdata->vbat_monitor_voltage_index << 5 |
			pdata->shutdown_enable << 4);
	if (ret) {
		pr_err("Can't update AS3668_REG_OVERTEMP_CTRL\n");
		goto err_init;
	}

	return 0;

err_init:
	return ret;
}

static int __devinit as3668_led_initialize(struct i2c_client *client,
		struct as3668_led *led, struct as3668_platform_data *pdata)
{
	int ret;

	led->client = client;
	led->color = 0xFFFFFFFF;
	led->ldev.name = AS3668_DRV_NAME;
	led->ldev.max_brightness = LED_FULL;
	led->ldev.brightness = LED_OFF;
	led->ldev.brightness_set = as3668_set_led_brightness;
	led->ldev.blink_brightness = LED_FULL;
	led->ldev.blink_set = as3668_set_led_blink;
	memcpy(led->led_array, pdata->led_array, AS3668_LED_NUM);

	ret = i2c_smbus_read_byte_data(client, AS3668_REG_PWM_TIMING);
	if (ret < 0) {
		pr_err("Fail to read AS3668_REG_PWM_TIMING\n");
		return ret;
	}
	led->pwm_dim_speed = ret;
	led->ldev.default_trigger = "none";
	ret = i2c_smbus_read_byte_data(client, AS3668_REG_FRAME_MASK);
	if (ret < 0) {
		pr_err("Fail to read AS3668_REG_FRAME_MASK\n");
		return ret;
	}
	led->pattern_frame_mask = ret;
	ret = i2c_smbus_read_byte_data(client, AS3668_REG_FRAME_START_DELAY);
	if (ret < 0) {
		pr_err("Fail to read AS3668_REG_FRAME_START_DELAY\n");
		return ret;
	}
	led->pattern_frame_delay = ret;

	ret = led_classdev_register(&client->dev, &led->ldev);
	if (ret) {
		pr_err("Couldn't register LED %s\n", led->ldev.name);
		return ret;
	}

	return 0;
}

static void as3668_led_cleanup(struct as3668_led *led)
{
	as3668_set_led_brightness(&led->ldev, LED_OFF);
	led_classdev_unregister(&led->ldev);
}

static DEVICE_ATTR(color, S_IRUGO | S_IWUSR | S_IWGRP,
		as3668_color_show, as3668_color_store);
static DEVICE_ATTR(slope_up, S_IRUGO | S_IWUSR | S_IWGRP,
		as3668_slope_up_show, as3668_slope_up_store);
static DEVICE_ATTR(slope_down, S_IRUGO | S_IWUSR | S_IWGRP,
		as3668_slope_down_show, as3668_slope_down_store);
static DEVICE_ATTR(frame_mask, S_IRUGO | S_IWUSR | S_IWGRP,
		as3668_frame_mask_show, as3668_frame_mask_store);
static DEVICE_ATTR(frame_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		as3668_frame_delay_show, as3668_frame_delay_store);

static struct attribute *as3668_sysfs_attrs[] = {
	&dev_attr_color.attr,
	&dev_attr_slope_up.attr,
	&dev_attr_slope_down.attr,
	&dev_attr_frame_mask.attr,
	&dev_attr_frame_delay.attr,
	NULL
};

static struct attribute_group as3668_attribute_group = {
	.attrs = as3668_sysfs_attrs,
};

static int __devinit as3668_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct as3668_led *led;
	struct as3668_platform_data *pdata = client->dev.platform_data;
	int ret;

	if (pdata == NULL) {
		pr_err("No platform data\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		pr_err("Client not i2c capable\n");
		return -ENOSYS;
	}

	led = kzalloc(sizeof(*led), GFP_KERNEL);
	if (!led) {
		pr_err("Failed to allocate memory for module\n");
		return -ENOMEM;
	}

	ret = as3668_initialize(client, pdata);
	if (ret) {
		pr_err("Fail to initialize as3668\n");
		return ret;
	}

	i2c_set_clientdata(client, led);
	mutex_init(&led->lock);

	ret = as3668_led_initialize(client, led, pdata);
	if (ret) {
		pr_err("Fail to initialize led device\n");
		goto err_led_init;
	}

	ret = sysfs_create_group(&led->ldev.dev->kobj,
			&as3668_attribute_group);
	if (ret) {
		pr_err("could not create sysfs group\n");
		goto err_sysfs_create_group;
	}
	return 0;

err_sysfs_create_group:
	as3668_led_cleanup(led);
err_led_init:
	mutex_destroy(&led->lock);
	kfree(led);
	return ret;
}

static int __devexit as3668_remove(struct i2c_client *client)
{
	struct as3668_led *led = i2c_get_clientdata(client);

	sysfs_remove_group(&led->ldev.dev->kobj,
			&as3668_attribute_group);
	as3668_led_cleanup(led);
	mutex_destroy(&led->lock);
	kfree(led);
	return 0;
}

static void as3668_shutdown(struct i2c_client *client)
{
	struct as3668_led *led = i2c_get_clientdata(client);

	as3668_set_led_brightness(&led->ldev, LED_OFF);
}

static const struct i2c_device_id as3668_id[] = {
	{"as3668", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, as3668_id);
static struct i2c_driver as3668_driver = {
	.driver = {
		.name = AS3668_DRV_NAME,
		.owner  = THIS_MODULE,
	},
	.probe = as3668_probe,
	.remove = __devexit_p(as3668_remove),
	.shutdown = as3668_shutdown,
	.id_table = as3668_id,
};

module_i2c_driver(as3668_driver);

MODULE_AUTHOR("Hyoung Wook Ham <hwham@sta.samsung.com>");
MODULE_DESCRIPTION("AS3668 LED driver");
MODULE_LICENSE("GPL v2");
