/*
 * LED Kernel Fading Trigger
 *   based on ledtrig-timer.c
 *
 * Copyright 2016 FriendlyARM (www.friendlyarm.com)
 *
 * Copyright 2005-2006 Openedhand Ltd.
 *
 * Author: Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include "leds.h"

struct led_sw_fade_data {
	int		enabled;
	int		fade_in;
	int		fade_step;
	void	(*timer_fn)(unsigned long data);
};

static void led_timer_function(unsigned long data)
{
	struct led_classdev *led_cdev = (void *)data;
	struct led_sw_fade_data *fade_data = led_get_trigger_data(led_cdev);
	unsigned long brightness;
	unsigned long delay;

	if (!fade_data->enabled) {
		fade_data->timer_fn(data);
		return;
	}

	if (!led_cdev->blink_delay_on || !led_cdev->blink_delay_off) {
		led_set_brightness(led_cdev, LED_OFF);
		return;
	}

	brightness = led_get_brightness(led_cdev);
	if (!brightness) {
		fade_data->fade_in = 1;
		brightness++;
		delay = fade_data->fade_step;

	} else if (fade_data->fade_in) {
		brightness++;
		if (brightness < led_cdev->max_brightness) {
			delay = fade_data->fade_step;
		} else {
			fade_data->fade_in = 0;
			delay = led_cdev->blink_delay_on;
		}

	} else {
		brightness--;
		if (brightness > 0) {
			delay = fade_data->fade_step;
		} else {
			brightness = LED_OFF;
			delay = led_cdev->blink_delay_off;
		}
	}

	led_set_brightness(led_cdev, brightness);

	mod_timer(&led_cdev->blink_timer, jiffies + msecs_to_jiffies(delay));
}

static ssize_t led_delay_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_on);
}

static ssize_t led_delay_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		led_blink_set(led_cdev, &state, &led_cdev->blink_delay_off);
		led_cdev->blink_delay_on = state;
		ret = count;
	}

	return ret;
}

static ssize_t led_delay_off_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_off);
}

static ssize_t led_delay_off_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		led_blink_set(led_cdev, &led_cdev->blink_delay_on, &state);
		led_cdev->blink_delay_off = state;
		ret = count;
	}

	return ret;
}

static ssize_t led_sw_fade_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_sw_fade_data *fade_data = led_get_trigger_data(led_cdev);
	unsigned long state = 0;

	if (fade_data)
		state = fade_data->enabled;

	return sprintf(buf, "%lu\n", state);
}

static ssize_t led_sw_fade_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_sw_fade_data *fade_data = led_get_trigger_data(led_cdev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		fade_data->enabled = state;
		if (state > 1 && state < 128)
			fade_data->fade_step = state;
		ret = count;
	}

	return ret;
}

static DEVICE_ATTR(delay_on, 0644, led_delay_on_show, led_delay_on_store);
static DEVICE_ATTR(delay_off, 0644, led_delay_off_show, led_delay_off_store);
static DEVICE_ATTR(sw_fade, 0644, led_sw_fade_show, led_sw_fade_store);

static void fading_trig_activate(struct led_classdev *led_cdev)
{
	struct led_sw_fade_data *fade_data;
	int rc;

	rc = device_create_file(led_cdev->dev, &dev_attr_delay_on);
	if (rc)
		return;
	rc = device_create_file(led_cdev->dev, &dev_attr_delay_off);
	if (rc)
		goto err_out_delayon;
	rc = device_create_file(led_cdev->dev, &dev_attr_sw_fade);
	if (rc)
		goto err_out_delayoff;

	fade_data = kzalloc(sizeof(*fade_data), GFP_KERNEL);
	if (!fade_data) {
		rc = -ENOMEM;
		goto err_out_sw_fade;
	}

	fade_data->enabled = 1;
	fade_data->fade_step = 32;
	fade_data->timer_fn = led_cdev->blink_timer.function;
	led_cdev->blink_timer.function = led_timer_function;

	if (!led_cdev->blink_delay_on)
		led_cdev->blink_delay_on = 1000;
	if (!led_cdev->blink_delay_off)
		led_cdev->blink_delay_off = 1000;

	led_blink_set(led_cdev, &led_cdev->blink_delay_on,
		      &led_cdev->blink_delay_off);

	led_cdev->trigger_data = fade_data;

	return;

err_out_sw_fade:
	device_remove_file(led_cdev->dev, &dev_attr_sw_fade);
err_out_delayoff:
	device_remove_file(led_cdev->dev, &dev_attr_delay_off);
err_out_delayon:
	device_remove_file(led_cdev->dev, &dev_attr_delay_on);
}

static void fading_trig_deactivate(struct led_classdev *led_cdev)
{
	struct led_sw_fade_data *fade_data = led_cdev->trigger_data;

	if (fade_data) {
		device_remove_file(led_cdev->dev, &dev_attr_delay_on);
		device_remove_file(led_cdev->dev, &dev_attr_delay_off);
		device_remove_file(led_cdev->dev, &dev_attr_sw_fade);

		led_cdev->blink_timer.function = fade_data->timer_fn;
		led_cdev->trigger_data = NULL;
		kfree(fade_data);
	}

	/* Stop blinking */
	led_brightness_set(led_cdev, LED_OFF);
}

static struct led_trigger fading_led_trigger = {
	.name     = "fading",
	.activate = fading_trig_activate,
	.deactivate = fading_trig_deactivate,
};

static int __init fading_trig_init(void)
{
	return led_trigger_register(&fading_led_trigger);
}

static void __exit fading_trig_exit(void)
{
	led_trigger_unregister(&fading_led_trigger);
}

module_init(fading_trig_init);
module_exit(fading_trig_exit);

MODULE_AUTHOR("FriendlyARM (www.friendlyarm.com)");
MODULE_AUTHOR("Richard Purdie <rpurdie@openedhand.com>");
MODULE_DESCRIPTION("Fading LED trigger");
MODULE_LICENSE("GPL");
