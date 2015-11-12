/*
 * haptic_isa1200.c - Haptic Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co. Ltd. All Rights Reserved.
 * Author: Vishnudev Ramakrishnan <vramakri@sta.samsung.com>
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

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <../../../drivers/staging/android/timed_output.h>
#include <linux/wakelock.h>
#include <linux/platform_data/haptic_isa1200.h>

#define ISA1200_HCTRL0		0x30
#define ISA1200_HCTRL0_DEFAULT	1 /* power down mode, default */
#define ISA1200_HCTRL0_HAPDREN			BIT(7)
#define ISA1200_HCTRL0_HAPDIGMOD_PWM_IN		BIT(3)
#define ISA1200_HCTRL0_PWMMOD_DIVIDER_128	0
#define ISA1200_HCTRL0_PWMMOD_DIVIDER_256	1
#define ISA1200_HCTRL0_PWMMOD_DIVIDER_512	2
#define ISA1200_HCTRL0_PWMMOD_DIVIDER_1024	3

#define ISA1200_HCTRL1		0x31
#define ISA1200_HCTRL1_RESERVED_BIT6_ON		BIT(6)

#define ISA1200_HCTRL1_MOTTYP_ERM	BIT(5)
#define ISA1200_HCTRL1_MOTTYP_LRA	0

#define PWM_HAPTIC_PERIOD	38676 /* 202 Hz with 128 divider */
/* duty cycle for max vibration effect */
#define PWM_MAX_VIBRATE_DUTY	(PWM_HAPTIC_PERIOD * 99/100)
/* duty cycle for no vibration effect */
#define PWM_NO_VIBRATE_DUTY	(PWM_HAPTIC_PERIOD * 50/100)

struct isa1200_data {
	struct i2c_client *client;
	struct isa1200_platform_data *pdata;
	struct hrtimer timer;
	struct timed_output_dev dev;
	struct mutex lock;
	struct wake_lock wklock;
	struct pwm_device *pwm;
	bool vibrate;
};

static int isa1200_vibrate_on(struct isa1200_data *haptic)
{
	int ret;
	u8 value;

	pr_debug("vibrate is %d\n", haptic->vibrate);
	if (!haptic->vibrate) {
		wake_lock(&haptic->wklock);
		ret = pwm_config(haptic->pwm, PWM_NO_VIBRATE_DUTY,
				PWM_HAPTIC_PERIOD);
		if (ret) {
			pr_err("pwm_config failed %d\n", ret);
			goto pwm_no_duty_cfg_failed;
		}
		pwm_enable(haptic->pwm);
		gpio_set_value(haptic->pdata->hap_en_gpio, 1);
		usleep_range(100, 200);
		value = ISA1200_HCTRL0_HAPDREN |
			ISA1200_HCTRL0_HAPDIGMOD_PWM_IN |
			ISA1200_HCTRL0_PWMMOD_DIVIDER_128;
		ret = i2c_smbus_write_byte_data(haptic->client, ISA1200_HCTRL0,
						value);
		if (ret < 0) {
			pr_err("write HCTRL0 failed %d\n", ret);
			goto err_return;
		}
		value = ISA1200_HCTRL1_RESERVED_BIT6_ON |
			ISA1200_HCTRL1_MOTTYP_LRA;
		ret = i2c_smbus_write_byte_data(haptic->client, ISA1200_HCTRL1,
						value);
		if (ret < 0) {
			pr_err("write HCTRL1 failed %d\n", ret);
			goto err_return;
		}
		ret = pwm_config(haptic->pwm, PWM_MAX_VIBRATE_DUTY,
				PWM_HAPTIC_PERIOD);
		if (ret) {
			pr_err("pwm_config for max duty failed %d\n", ret);
			goto err_return;
		}
		haptic->vibrate = true;
	}
	return 0;

err_return:
	gpio_set_value(haptic->pdata->hap_en_gpio, 0);
	pwm_disable(haptic->pwm);
pwm_no_duty_cfg_failed:
	wake_unlock(&haptic->wklock);
	return ret;
}

static void isa1200_vibrate_off(struct isa1200_data *haptic)
{
	int ret;
	pr_debug("vibrate is %d\n", haptic->vibrate);
	if (haptic->vibrate) {
		ret = pwm_config(haptic->pwm, PWM_NO_VIBRATE_DUTY,
				PWM_HAPTIC_PERIOD);
		if (ret)
			pr_err("pwm_config failed %d\n", ret);
		gpio_set_value(haptic->pdata->hap_en_gpio, 0);
		pwm_disable(haptic->pwm);
		haptic->vibrate = false;
		wake_unlock(&haptic->wklock);
	}
}

static void isa1200_enable(struct timed_output_dev *dev, int timeout)
{
	int ret;
	struct isa1200_data *haptic = container_of(dev, struct isa1200_data,
					dev);

	mutex_lock(&haptic->lock);
	hrtimer_cancel(&haptic->timer);
	pr_debug("timeout is %d msec\n", timeout);
	if (timeout > 0) {
		ret = isa1200_vibrate_on(haptic);
		if (ret)
			goto vibrate_error;
		if (timeout > haptic->pdata->max_timeout)
			timeout = haptic->pdata->max_timeout;
		hrtimer_start(&haptic->timer,
			ns_to_ktime((u64)timeout * NSEC_PER_MSEC),
			HRTIMER_MODE_REL);
	} else {
		isa1200_vibrate_off(haptic);
	}

vibrate_error:
	mutex_unlock(&haptic->lock);
}

static int isa1200_get_time(struct timed_output_dev *dev)
{
	struct isa1200_data *haptic = container_of(dev, struct isa1200_data,
					dev);

	if (hrtimer_active(&haptic->timer)) {
		ktime_t r = hrtimer_get_remaining(&haptic->timer);
		return ktime_to_ms(r);
	}
	return 0;
}

static enum hrtimer_restart isa1200_timer_func(struct hrtimer *timer)
{
	struct isa1200_data *haptic = container_of(timer, struct isa1200_data,
					timer);
	pr_debug("vibrate is %d\n", haptic->vibrate);
	/* no lock required below, as isa1200_enable cancels timer first */
	isa1200_vibrate_off(haptic);
	return HRTIMER_NORESTART;
}

static int __devinit isa1200_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct isa1200_data *haptic;
	struct isa1200_platform_data *pdata;
	int ret;
	pr_debug("\n");
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
		pr_err("no support for i2c write byte data\n");
		return -ENOSYS;
	}

	pdata = client->dev.platform_data;
	if (!pdata) {
		pr_err("no platform data\n");
		return -EINVAL;
	}

	haptic = kmalloc(sizeof(*haptic), GFP_KERNEL);
	if (!haptic)
		return -ENOMEM;
	haptic->client = client;
	haptic->pdata = pdata;
	haptic->vibrate = false;

	ret = gpio_request_one(pdata->hap_en_gpio, GPIOF_OUT_INIT_LOW,
				"haptic_gpio");
	if (ret) {
		printk(KERN_ERR "%s: gpio %d request failed with error %d\n",
			__func__, pdata->hap_en_gpio, ret);
		goto enable_gpio_fail;
	}

	haptic->pwm = pwm_request(pdata->pwm_ch, id->name);
	if (IS_ERR(haptic->pwm)) {
		pr_err("pwm request failed\n");
		ret = PTR_ERR(haptic->pwm);
		goto pwm_req_fail;
	}

	mutex_init(&haptic->lock);
	wake_lock_init(&haptic->wklock, WAKE_LOCK_SUSPEND, "vibrator");

	hrtimer_init(&haptic->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	haptic->timer.function = isa1200_timer_func;

	i2c_set_clientdata(client, haptic);

	/* register with timed output class */
	haptic->dev.name = "vibrator";
	haptic->dev.get_time = isa1200_get_time;
	haptic->dev.enable = isa1200_enable;
	ret = timed_output_dev_register(&haptic->dev);
	if (ret < 0) {
		pr_err("timed output register failed %d\n", ret);
		goto setup_fail;
	}
	pr_debug("%s registered\n", id->name);
	return 0;

setup_fail:
	mutex_destroy(&haptic->lock);
	wake_lock_destroy(&haptic->wklock);
	pwm_free(haptic->pwm);
pwm_req_fail:
	gpio_free(pdata->hap_en_gpio);
enable_gpio_fail:
	kfree(haptic);
	return ret;
}

static int __devexit isa1200_remove(struct i2c_client *client)
{
	struct isa1200_data *haptic = i2c_get_clientdata(client);
	gpio_set_value(haptic->pdata->hap_en_gpio, 0);
	timed_output_dev_unregister(&haptic->dev);
	hrtimer_cancel(&haptic->timer);
	mutex_destroy(&haptic->lock);
	wake_lock_destroy(&haptic->wklock);
	pwm_free(haptic->pwm);
	gpio_free(haptic->pdata->hap_en_gpio);
	kfree(haptic);
	return 0;
}

static const struct i2c_device_id isa1200_id[] = {
	{"isa1200", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, isa1200_id);

static struct i2c_driver isa1200_driver = {
	.driver = {
		.name = "isa1200",
		.owner = THIS_MODULE,
	},
	.probe = isa1200_probe,
	.remove = __devexit_p(isa1200_remove),
	.id_table = isa1200_id,
};

module_i2c_driver(isa1200_driver);

MODULE_AUTHOR("Vishnudev Ramakrishnan <vramakri@sta.samsung.com>");
MODULE_DESCRIPTION("ISA1200 Haptic Controller driver");
MODULE_LICENSE("GPL v2");
