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

#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>

#include <plat/s3c64xx-spi.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/gpio-cfg.h>

#include <mach/spi-clocks.h>
#include <mach/gpio.h>
#include <mach/sysmmu.h>

#include <media/exynos_fimc_is.h>


static struct exynos5_platform_fimc_is exynos5_fimc_is_data;

#if defined CONFIG_VIDEO_S5K4E5
static struct exynos5_fimc_is_sensor_info s5k4e5 = {
	.sensor_name = "S5K4E5",
	.sensor_id = SENSOR_NAME_S5K4E5,
#if defined CONFIG_S5K4E5_POSITION_FRONT
	.sensor_position = SENSOR_POSITION_FRONT,
#elif defined CONFIG_S5K4E5_POSITION_REAR
	.sensor_position = SENSOR_POSITION_REAR,
#endif
#if defined CONFIG_S5K4E5_CSI_C
	.csi_id = CSI_ID_A,
	.flite_id = FLITE_ID_A,
	.i2c_channel = SENSOR_CONTROL_I2C0,
#elif defined CONFIG_S5K4E5_CSI_D
	.csi_id = CSI_ID_B,
	.flite_id = FLITE_ID_B,
	.i2c_channel = SENSOR_CONTROL_I2C1,
#endif
	.max_width = 2560,
	.max_height = 1920,
	.max_frame_rate = 30,

	.mipi_lanes = 2,
	.mipi_settle = 12,
	.mipi_align = 24,
	.sensor_power = {
		.cam_core = "5m_core_1.5v",
		.cam_io_myself = "cam_io_1.8v",
		.cam_io_peer = "vt_cam_1.8v",
		.cam_af = "cam_af_2.8v",
	},
	.sensor_gpio = {
		.cfg[0] = { /* ISP_TXD */
			.pin = EXYNOS5_GPE0(7),
			.name = "GPE0",
			.value = (3<<28),
			.act = GPIO_PULL_NONE,
		},
		.cfg[1] = { /* ISP_RXD */
			.pin = EXYNOS5_GPE1(1),
			.name = "GPE1",
			.value = (3<<4),
			.act = GPIO_PULL_NONE,
		},
		.cfg[2] = { /* 5M_CAM_SDA_18V */
			.pin = EXYNOS5_GPF0(0),
			.name = "GPF0",
			.value = (2<<0),
			.act = GPIO_PULL_NONE,
		},
		.cfg[3] = { /* 5M_CAM_SCL_18V */
			.pin = EXYNOS5_GPF0(1),
			.name = "GPF0",
			.value = (2<<4),
			.act = GPIO_PULL_NONE,
		},
		.cfg[4] = { /* CAM_FLASH_EN */
			.pin = EXYNOS5_GPE0(1),
			.name = "GPE0",
			.value = (2<<4),
			.act = GPIO_PULL_NONE,
		},
		.cfg[5] = { /* CAM_FLASH_SET */
			.pin = EXYNOS5_GPE0(2),
			.name = "GPE0",
			.value = (2<<8),
			.act = GPIO_PULL_NONE,
		},
		.cfg[6] = { /* CAM_MCLK */
			.pin = EXYNOS5_GPH0(3),
			.name = "GPH0",
			.value = (2<<12),
			.act = GPIO_PULL_NONE,
		},
		.power = { /* CAM_IO_EN - VDDA_2.8V*/
			.pin = EXYNOS5_GPV0(3),
			.name = "GPV0",
			.value = 1,
			.act = GPIO_OUTPUT,
		},
		.reset_myself = { /* 5M_CAM_RESET */
			.pin = EXYNOS5_GPE0(0),
			.name = "GPE0",
			.value = 0,
			.act = GPIO_RESET,
		},
		.reset_peer = { /* CAM_VT_nRST */
			.pin = EXYNOS5_GPG1(6),
			.name = "GPG1",
			.value = 0,
			.act = GPIO_RESET,
		},
	},
};
#endif

#if defined CONFIG_VIDEO_S5K6A3
static struct exynos5_fimc_is_sensor_info s5k6a3 = {
	.sensor_name = "S5K6A3",
	.sensor_id = SENSOR_NAME_S5K6A3,
#if defined CONFIG_S5K6A3_POSITION_FRONT
	.sensor_position = SENSOR_POSITION_FRONT,
#elif defined CONFIG_S5K6A3_POSITION_REAR
	.sensor_position = SENSOR_POSITION_REAR,
#endif
#if defined CONFIG_S5K6A3_CSI_C
	.csi_id = CSI_ID_A,
	.flite_id = FLITE_ID_A,
	.i2c_channel = SENSOR_CONTROL_I2C0,
#elif defined CONFIG_S5K6A3_CSI_D
	.csi_id = CSI_ID_B,
	.flite_id = FLITE_ID_B,
	.i2c_channel = SENSOR_CONTROL_I2C1,
#endif
	.max_width = 1280,
	.max_height = 720,
	.max_frame_rate = 30,

	.mipi_lanes = 1,
	.mipi_settle = 12,
	.mipi_align = 24,
	.sensor_power = {
		.cam_core = "5m_core_1.5v",
		.cam_io_myself = "vt_cam_1.8v",
		.cam_io_peer = "cam_io_1.8v",
	},
	.sensor_gpio = {
		.cfg[0] = { /* ISP_TXD */
			.pin = EXYNOS5_GPE0(7),
			.name = "GPE0",
			.value = (3<<28),
			.act = GPIO_PULL_NONE,
		},
		.cfg[1] = { /* ISP_RXD */
			.pin = EXYNOS5_GPE1(1),
			.name = "GPE1",
			.value = (3<<4),
			.act = GPIO_PULL_NONE,
		},
		.cfg[2] = { /* VT_CAM_SDA_18V */
			.pin = EXYNOS5_GPF0(2),
			.name = "GPF0",
			.value = (2<<8),
			.act = GPIO_PULL_NONE,
		},
		.cfg[3] = { /* VT_CAM_SCL_18V */
			.pin = EXYNOS5_GPF0(3),
			.name = "GPF0",
			.value = (2<<12),
			.act = GPIO_PULL_NONE,
		},
		.cfg[4] = { /* VTCAM_MCLK */
			.pin = EXYNOS5_GPG2(1),
			.name = "GPG2",
			.value = (2<<4),
			.act = GPIO_PULL_NONE,
		},
		.power = { /* CAM_IO_EN - VDDA_2.8V*/
			.pin = EXYNOS5_GPV0(3),
			.name = "GPV0",
			.value = 1,
			.act = GPIO_OUTPUT,
		},
		.reset_myself = { /* CAM_VT_nRST */
			.pin = EXYNOS5_GPG1(6),
			.name = "GPG1",
			.value = 0,
			.act = GPIO_RESET,
		},
		.reset_peer = { /* 5M_CAM_RESET */
			.pin = EXYNOS5_GPE0(0),
			.name = "GPE0",
			.value = 0,
			.act = GPIO_RESET,
		},
	},
};
#endif

static struct s3c64xx_spi_csinfo spi1_csi[] = {
	[0] = {
		.line           = EXYNOS5_GPA2(5),
		.set_level      = gpio_set_value,
		.fb_delay       = 0x2,
	},
};

static struct spi_board_info spi1_board_info[] __initdata = {
	{
		.modalias               = "fimc_is_spi",
		.platform_data          = NULL,
		.max_speed_hz           = 10 * 1000 * 1000,
		.bus_num                = 1,
		.chip_select            = 0,
		.mode                   = SPI_MODE_0,
		.controller_data        = &spi1_csi[0],
	}
};

static void manta_gpio_pull_up(bool pull_up)
{
	if (pull_up) {
		s3c_gpio_cfgpin(EXYNOS5_GPA2(4), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(EXYNOS5_GPA2(6), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(EXYNOS5_GPA2(7), S3C_GPIO_SFN(2));
		pr_debug("GPIO : spi function\n");
	} else {
		s3c_gpio_setpull(EXYNOS5_GPA2(4), S3C_GPIO_PULL_DOWN);
		s3c_gpio_setpull(EXYNOS5_GPA2(6), S3C_GPIO_PULL_DOWN);
		s3c_gpio_setpull(EXYNOS5_GPA2(7), S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(EXYNOS5_GPA2(4), S3C_GPIO_SFN(0));
		s3c_gpio_cfgpin(EXYNOS5_GPA2(6), S3C_GPIO_SFN(0));
		s3c_gpio_cfgpin(EXYNOS5_GPA2(7), S3C_GPIO_SFN(0));
		pr_debug("GPIO : input\n");
	}
}

static struct platform_device *camera_devices[] __initdata = {
	&s3c64xx_device_spi1,
	&exynos5_device_fimc_is,
};

static void __init manta_camera_sysmmu_init(void)
{
	platform_set_sysmmu(&SYSMMU_PLATDEV(isp).dev,
					&exynos5_device_fimc_is.dev);

}

void __init exynos5_manta_camera_init(void)
{
	manta_camera_sysmmu_init();
	platform_add_devices(camera_devices, ARRAY_SIZE(camera_devices));

	/* SPI */
	exynos_spi_clock_setup(&s3c64xx_device_spi1.dev, 1);

	if (!exynos_spi_cfg_cs(spi1_csi[0].line, 1)) {
		s3c64xx_spi1_pdata.gpio_pull_up = manta_gpio_pull_up;
		s3c64xx_spi1_set_platdata(&s3c64xx_spi1_pdata,
			EXYNOS_SPI_SRCCLK_SCLK, ARRAY_SIZE(spi1_csi));

		spi_register_board_info(spi1_board_info,
			ARRAY_SIZE(spi1_board_info));
	} else {
		pr_err("%s: Error requesting gpio for SPI-CH1 CS\n", __func__);
	}

	/* FIMC-IS-MC */
	dev_set_name(&exynos5_device_fimc_is.dev, "s5p-mipi-csis.0");
	clk_add_alias("gscl_wrap0", FIMC_IS_MODULE_NAME, "gscl_wrap0",
			&exynos5_device_fimc_is.dev);
	clk_add_alias("sclk_gscl_wrap0", FIMC_IS_MODULE_NAME, "sclk_gscl_wrap0",
			&exynos5_device_fimc_is.dev);

	dev_set_name(&exynos5_device_fimc_is.dev, "s5p-mipi-csis.1");
	clk_add_alias("gscl_wrap1", FIMC_IS_MODULE_NAME, "gscl_wrap1",
			&exynos5_device_fimc_is.dev);
	clk_add_alias("sclk_gscl_wrap1", FIMC_IS_MODULE_NAME, "sclk_gscl_wrap1",
			&exynos5_device_fimc_is.dev);

	dev_set_name(&exynos5_device_fimc_is.dev, "exynos-gsc.0");
	clk_add_alias("gscl", FIMC_IS_MODULE_NAME, "gscl",
			&exynos5_device_fimc_is.dev);
	dev_set_name(&exynos5_device_fimc_is.dev, FIMC_IS_MODULE_NAME);

#if defined CONFIG_VIDEO_S5K6A3
	exynos5_fimc_is_data.sensor_info[s5k6a3.sensor_position] = &s5k6a3;
#endif
#if defined CONFIG_VIDEO_S5K4E5
	exynos5_fimc_is_data.sensor_info[s5k4e5.sensor_position] = &s5k4e5;
#endif

	exynos5_fimc_is_set_platdata(&exynos5_fimc_is_data);
}

