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

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/spi/spi.h>
#include <linux/nfc/bcm2079x.h>

#include <plat/devs.h>
#include <plat/gpio-cfg.h>
#include <plat/s3c64xx-spi.h>

#include <mach/spi-clocks.h>

#define GPIO_NFC_WAKE		EXYNOS5_GPX1(5)
#define GPIO_SPI_INT		EXYNOS5_GPX1(6)
#define GPIO_NFC_EN		EXYNOS5_GPD1(7)
#define GPIO_NFC_CS		EXYNOS5_GPB1(2)

static void bcm2079x_nfc_setup_gpio(void)
{
	s3c_gpio_cfgpin(GPIO_SPI_INT, S3C_GPIO_SFN(S3C_GPIO_INPUT));
	s3c_gpio_setpull(GPIO_SPI_INT, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_NFC_EN, S3C_GPIO_SFN(S3C_GPIO_OUTPUT));
	s3c_gpio_setpull(GPIO_NFC_EN, S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(GPIO_NFC_WAKE, S3C_GPIO_SFN(S3C_GPIO_OUTPUT));
	s3c_gpio_setpull(GPIO_NFC_WAKE, S3C_GPIO_PULL_UP);
}

static struct bcm2079x_platform_data bcm2079x_spi_pdata = {
	.irq_gpio = GPIO_SPI_INT,
	.en_gpio = GPIO_NFC_EN,
	.wake_gpio = GPIO_NFC_WAKE,
};

static struct s3c64xx_spi_csinfo spi2_csi[] = {
	[0] = {
		.line		= GPIO_NFC_CS,
		.set_level	= gpio_set_value,
		.fb_delay	= 0x0,
	},
};

static struct spi_board_info spi2_board_info[] __initdata = {
	{
		.modalias		= "bcm2079x-spi",
		.max_speed_hz		= 4 * MHZ,
		.bus_num		= 2,
		.chip_select		= 0,
		.mode			= SPI_MODE_0,
		.irq			= IRQ_EINT(14),
		.controller_data	= &spi2_csi[0],
		.platform_data		= &bcm2079x_spi_pdata,
	}
};

void __init exynos5_manta_nfc_init(void)
{
	bcm2079x_nfc_setup_gpio();
	exynos_spi_clock_setup(&s3c64xx_device_spi2.dev, 2);
	if (!exynos_spi_cfg_cs(spi2_csi[0].line, 2)) {
		s3c64xx_spi2_set_platdata(&s3c64xx_spi2_pdata,
				EXYNOS_SPI_SRCCLK_SCLK, ARRAY_SIZE(spi2_csi));

		spi_register_board_info(spi2_board_info,
				ARRAY_SIZE(spi2_board_info));
	} else {
		pr_err("%s : Error requesting gpio for SPI-CH%d CS",
			__func__, spi2_board_info->bus_num);
	}

	platform_device_register(&s3c64xx_device_spi2);
}
