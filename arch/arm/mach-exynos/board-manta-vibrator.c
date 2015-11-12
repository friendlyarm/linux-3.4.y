/*
 * Copyright (C) 2012 Samsung Electronics Co. Ltd. All Rights Reserved.
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
#include <linux/platform_data/haptic_isa1200.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/devs.h>

#define VIB_PWM_GPIO EXYNOS5_GPB2(1)

static struct isa1200_platform_data isa1200_pdata = {
	.pwm_ch		= 1, /* XPWMTOUT_1 */
	.hap_en_gpio	= EXYNOS5_GPD1(6),
	.max_timeout	= 10000, /* 10 seconds */
};

/* I2C4 */
static struct i2c_board_info i2c_devs4[] __initdata = {
	{
		I2C_BOARD_INFO("isa1200", (0x90 >> 1)),
		.platform_data  = &isa1200_pdata,
	},
};

void __init exynos5_manta_vib_init(void)
{
	int ret;
	ret = s3c_gpio_cfgpin(VIB_PWM_GPIO, S3C_GPIO_SFN(2));
	if (ret) {
		printk(KERN_ERR "%s s3c_gpio_cfgpin error %d\n",
				__func__, ret);
		goto init_fail;
	}
	ret = i2c_register_board_info(4, i2c_devs4, ARRAY_SIZE(i2c_devs4));
	if (ret) {
		printk(KERN_ERR "%s i2c_register_board_info error %d\n",
				__func__, ret);
		goto init_fail;
	}
	ret = platform_device_register(&s3c_device_timer[isa1200_pdata.pwm_ch]);
	if (ret)
		printk(KERN_ERR "%s failed platform_device_register with error %d\n",
				__func__, ret);

init_fail:
	return;
}
