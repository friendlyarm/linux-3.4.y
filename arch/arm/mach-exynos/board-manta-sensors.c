/*
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
 */

#include <linux/platform_data/bh1721fvc.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/mpu.h>
#include <plat/gpio-cfg.h>

#define GPIO_ACC_INT	EXYNOS5_GPX1(4)
#define EINT_ACC	12
#define GPIO_MSENSE_RST	EXYNOS5_GPG2(0)

static struct mpu_platform_data mpu_data = {
	.int_config  = 0x00,
	.level_shifter = 0,
	.orientation = {
		0,  1,  0,
		1,  0,  0,
		0,  0, -1,
	},
	.sec_slave_type = SECONDARY_SLAVE_TYPE_COMPASS,
	.sec_slave_id   = COMPASS_ID_AK8963,
	.secondary_i2c_addr = 0x0C,
	.secondary_orientation = {
		-1,  0,  0,
		 0,  1,  0,
		 0,  0, -1,
	},
	.key = {
		221,  22, 205,   7, 217, 186, 151, 55,
		206, 254,  35, 144, 225, 102,  47, 50,
	},
};

static struct i2c_board_info __initdata manta_sensors_i2c1_boardinfo[] = {
	{
		I2C_BOARD_INFO("bmp182", 0x77),
	},
	{
		I2C_BOARD_INFO("mpu6050", 0x68),
		.irq = IRQ_EINT(EINT_ACC),
		.platform_data = &mpu_data,
	},
};

#define GPIO_ALS_NRST	EXYNOS5_GPH1(2)

static struct bh1721fvc_platform_data bh1721fvc_pdata = {
	.reset_pin = GPIO_ALS_NRST,
};

static struct i2c_board_info __initdata manta_sensors_i2c2_boardinfo[] = {
	{
		I2C_BOARD_INFO("bh1721fvc", 0x23),
		.platform_data = &bh1721fvc_pdata,
	},
};

void __init exynos5_manta_sensors_init(void)
{
	s3c_gpio_setpull(GPIO_ACC_INT, S3C_GPIO_PULL_UP);

	gpio_request_one(GPIO_ACC_INT, GPIOF_IN, "ACC_INT");
	gpio_request_one(GPIO_MSENSE_RST, GPIOF_OUT_INIT_HIGH, "MSENSE_RST");

	i2c_register_board_info(1, manta_sensors_i2c1_boardinfo,
		ARRAY_SIZE(manta_sensors_i2c1_boardinfo));

	s3c_gpio_setpull(GPIO_ALS_NRST, S3C_GPIO_PULL_NONE);
	i2c_register_board_info(2, manta_sensors_i2c2_boardinfo,
		ARRAY_SIZE(manta_sensors_i2c2_boardinfo));
}
