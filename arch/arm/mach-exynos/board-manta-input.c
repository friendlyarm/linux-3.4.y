/*
 * Copyright (C) 2012 Google, Inc.
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
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

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/interrupt.h>

#include <plat/devs.h>
#include <plat/iic.h>
#include <plat/gpio-cfg.h>

#define GPIO_TOUCH_CHG		EXYNOS5_GPG1(2)
#define GPIO_TOUCH_RESET	EXYNOS5_GPG1(3)
#define GPIO_TOUCH_EN_BOOSTER	EXYNOS5_GPD1(1)
#define GPIO_TOUCH_EN_XVDD	EXYNOS5_GPG0(1)

static struct mxt_platform_data atmel_mxt_ts_pdata = {
	.x_line         = 32,
	.y_line         = 52,
	.x_size         = 4095,
	.y_size         = 4095,
	.orient         = MXT_DIAGONAL,
	.irqflags       = IRQF_TRIGGER_LOW | IRQF_ONESHOT,
	.boot_address   = 0x26,
	.gpio_reset     = GPIO_TOUCH_RESET,
	.reset_msec     = 75,
};

static struct i2c_board_info i2c_devs3[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data	= &atmel_mxt_ts_pdata,
	},
};

void __init exynos5_manta_input_init(void)
{
	int hw_rev;

	gpio_request(GPIO_TOUCH_CHG, "TSP_INT");
	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(GPIO_TOUCH_CHG);

	s5p_gpio_set_pd_cfg(GPIO_TOUCH_CHG, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(GPIO_TOUCH_CHG, S5P_GPIO_PD_UPDOWN_DISABLE);
	s5p_gpio_set_pd_cfg(GPIO_TOUCH_RESET, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_cfg(GPIO_TOUCH_EN_XVDD, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_cfg(GPIO_TOUCH_EN_BOOSTER, S5P_GPIO_PD_PREV_STATE);

	i2c_devs3[0].irq = gpio_to_irq(GPIO_TOUCH_CHG);
	i2c_register_board_info(3, i2c_devs3, ARRAY_SIZE(i2c_devs3));
}
