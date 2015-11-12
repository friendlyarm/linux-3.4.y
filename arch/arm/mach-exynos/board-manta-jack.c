/*
 * Copyright (C) 2012 Google, Inc.
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
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/sec_jack.h>

#include <plat/gpio-cfg.h>

#include "board-manta.h"

#define GPIO_DET_35		EXYNOS5_GPX0(1)

static void sec_jack_set_micbias_state(bool on)
{
}

static struct sec_jack_zone sec_jack_zones[] = {
	{
		/* adc < 50, unstable zone, default to 3pole if it stays
		* in this range for a half second (20ms delays, 25 samples)
		*/
		.adc_high = 50,
		.delay_ms = 20,
		.check_count = 25,
		.jack_type = SEC_HEADSET_3POLE,
	},
	{
		/* 50 < adc <= 490, unstable zone, default to 3pole if it stays
		* in this range for a second (10ms delays, 100 samples)
		*/
		.adc_high = 490,
		.delay_ms = 10,
		.check_count = 100,
		.jack_type = SEC_HEADSET_3POLE,
	},
	{
		/* 490 < adc <= 900, unstable zone, default to 4pole if it
		* stays in this range for a second (10ms delays, 100 samples)
		*/
		.adc_high = 900,
		.delay_ms = 10,
		.check_count = 100,
		.jack_type = SEC_HEADSET_4POLE,
	},
	{
		/* 900 < adc <= 1500, 4 pole zone, default to 4pole if it
		* stays in this range for 200ms (20ms delays, 10 samples)
		*/
		.adc_high = 1500,
		.delay_ms = 20,
		.check_count = 10,
		.jack_type = SEC_HEADSET_4POLE,
	},
	{
		/* adc > 1500, unstable zone, default to 3pole if it stays
		* in this range for a second (10ms delays, 100 samples)
		*/
		.adc_high = 0x7fffffff,
		.delay_ms = 10,
		.check_count = 100,
		.jack_type = SEC_HEADSET_3POLE,
	},
};

/* To support 3-buttons earjack */
static struct sec_jack_buttons_zone sec_jack_buttons_zones[] = {
	{
		/* 0 <= adc <= 93, stable zone */
		.code           = KEY_MEDIA,
		.adc_low        = 0,
		.adc_high       = 93,
	},
	{
		/* 94 <= adc <= 167, stable zone */
		.code           = KEY_PREVIOUSSONG,
		.adc_low        = 94,
		.adc_high       = 167,
	},
	{
		/* 168 <= adc <= 370, stable zone */
		.code           = KEY_NEXTSONG,
		.adc_low        = 168,
		.adc_high       = 370,
	},
};

static int sec_jack_get_adc_value(void)
{
	return 0;
}

struct sec_jack_platform_data sec_jack_pdata = {
	.set_micbias_state = sec_jack_set_micbias_state,
	.get_adc_value = sec_jack_get_adc_value,
	.zones = sec_jack_zones,
	.num_zones = ARRAY_SIZE(sec_jack_zones),
	.buttons_zones = sec_jack_buttons_zones,
	.num_buttons_zones = ARRAY_SIZE(sec_jack_buttons_zones),
	.det_gpio = GPIO_DET_35,
};

static struct platform_device sec_device_jack = {
	.name                   = "sec_jack",
	.id                     = 1, /* will be used also for gpio_event id */
	.dev.platform_data      = &sec_jack_pdata,
};

void __init exynos5_manta_jack_init(void)
{
	s3c_gpio_cfgpin(GPIO_DET_35, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_DET_35, S3C_GPIO_PULL_NONE);

	/* GPIO_DET_35 is inverted on <= PRE_ALPHA boards */
	if (exynos5_manta_get_revision() <= MANTA_REV_PRE_ALPHA)
		sec_jack_pdata.det_active_high = true;

	platform_device_register(&sec_device_jack);
}
