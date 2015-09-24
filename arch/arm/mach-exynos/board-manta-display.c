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
#include <linux/fb.h>
#include <linux/platform_device.h>
#include <linux/pwm_backlight.h>

#include <video/platform_lcd.h>
#include <video/s5p-dp.h>

#include <plat/backlight.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/dp.h>
#include <plat/fb.h>
#include <plat/gpio-cfg.h>
#include <plat/iic.h>
#include <plat/regs-fb-v4.h>
#include <plat/tv-core.h>

#include <mach/gpio.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/sysmmu.h>

#include "common.h"

#define GPIO_LCD_EN		EXYNOS5_GPH1(7)
#define GPIO_LCD_ID		EXYNOS5_GPD1(4)
#define GPIO_LCD_PWM_IN_18V	EXYNOS5_GPB2(0)
#define GPIO_LED_BL_RST		EXYNOS5_GPG0(5)

#define GPIO_LCDP_SCL__18V	EXYNOS5_GPD0(6)
#define GPIO_LCDP_SDA__18V	EXYNOS5_GPD0(7)

#define GPIO_HDMI_HPD		EXYNOS5_GPX3(7)
#define GPIO_HDMI_DCDC_EN	EXYNOS5_GPA0(4)
#define GPIO_HDMI_LS_EN		EXYNOS5_GPG0(7)
#define GPIO_APS_EN_18V		EXYNOS5_GPH1(6)

#define LCD_POWER_OFF_TIME_US   (500 * USEC_PER_MSEC)

extern phys_addr_t manta_bootloader_fb_start;
extern phys_addr_t manta_bootloader_fb_size;

static ktime_t lcd_on_time;

static void manta_lcd_on(void)
{
	s64 us = ktime_us_delta(lcd_on_time, ktime_get_boottime());
	if (us > LCD_POWER_OFF_TIME_US) {
		pr_warn("lcd on sleep time too long\n");
		us = LCD_POWER_OFF_TIME_US;
	}

	if (us > 0)
		usleep_range(us, us);

	gpio_set_value(GPIO_LCD_EN, 1);
	usleep_range(200000, 200000);
}

static void manta_lcd_off(void)
{
	gpio_set_value(GPIO_LCD_EN, 0);

	lcd_on_time = ktime_add_us(ktime_get_boottime(), LCD_POWER_OFF_TIME_US);
}

static void manta_backlight_on(void)
{
	usleep_range(97000, 97000);
	gpio_set_value(GPIO_LED_BL_RST, 1);
}

static void manta_backlight_off(void)
{
	/* LED_BACKLIGHT_RESET: XCI1RGB_5 => GPG0_5 */
	gpio_set_value(GPIO_LED_BL_RST, 0);
}

static void manta_lcd_set_power(struct plat_lcd_data *pd,
				unsigned int power)
{
	if (power)
		manta_lcd_on();
	else
		manta_lcd_off();
}

static struct plat_lcd_data manta_lcd_data = {
	.set_power	= manta_lcd_set_power,
};

static struct platform_device manta_lcd = {
	.name	= "platform-lcd",
	.dev	= {
		.parent		= &s5p_device_fimd1.dev,
		.platform_data	= &manta_lcd_data,
	},
};

static void manta_fimd_gpio_setup_24bpp(void)
{
	u32 reg;

	/* basic fimd init */
	exynos5_fimd1_gpio_setup_24bpp();

	/* Reference clcok selection for DPTX_PHY: pad_osc_clk_24M */
	reg = __raw_readl(S3C_VA_SYS + 0x04d4);
	reg = (reg & ~(0x1 << 0)) | (0x0 << 0);
	__raw_writel(reg, S3C_VA_SYS + 0x04d4);

	/* DPTX_PHY: XXTI */
	reg = __raw_readl(S3C_VA_SYS + 0x04d8);
	reg = (reg & ~(0x1 << 3)) | (0x0 << 3);
	__raw_writel(reg, S3C_VA_SYS + 0x04d8);
}

static struct s3c_fb_pd_win manta_fb_win2 = {
	.win_mode = {
		.left_margin	= 80,
		.right_margin	= 48,
		.upper_margin	= 37,
		.lower_margin	= 3,
		.hsync_len	= 32,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,
	},
	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,
	.width			= 218,
	.height			= 136,
};

static struct s3c_fb_platdata manta_lcd1_pdata __initdata = {
	.win[0]		= &manta_fb_win2,
	.win[1]		= &manta_fb_win2,
	.win[2]		= &manta_fb_win2,
	.win[3]		= &manta_fb_win2,
	.win[4]		= &manta_fb_win2,
	.default_win	= 0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= 0,
	.setup_gpio	= manta_fimd_gpio_setup_24bpp,
	.backlight_off  = manta_backlight_off,
	.lcd_off	= manta_lcd_off,
};

static struct video_info manta_dp_config = {
	.name			= "WQXGA(2560x1600) LCD",

	.h_sync_polarity	= 0,
	.v_sync_polarity	= 0,
	.interlaced		= 0,

	.color_space		= COLOR_RGB,
	.dynamic_range		= VESA,
	.ycbcr_coeff		= COLOR_YCBCR601,
	.color_depth		= COLOR_8,

	.link_rate		= LINK_RATE_2_70GBPS,
	.lane_count		= LANE_COUNT4,
};

static struct s5p_dp_platdata manta_dp_data __initdata = {
	.video_info     = &manta_dp_config,
	.phy_init       = s5p_dp_phy_init,
	.phy_exit       = s5p_dp_phy_exit,
	.backlight_on   = manta_backlight_on,
	.backlight_off  = manta_backlight_off,
	.lcd_on		= manta_lcd_on,
	.lcd_off	= manta_lcd_off,
};

/* LCD Backlight data */
static struct samsung_bl_gpio_info manta_bl_gpio_info = {
	.no	= GPIO_LCD_PWM_IN_18V,
	.func	= S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data manta_bl_data = {
	.pwm_id		= 0,
	.pwm_period_ns	= 1000000,
	.dft_brightness = 102,
};

static struct platform_device exynos_device_md0 = {
	.name = "exynos-mdev",
	.id = 0,
};

static struct platform_device exynos_device_md1 = {
	.name = "exynos-mdev",
	.id = 1,
};

static struct platform_device exynos_device_md2 = {
	.name = "exynos-mdev",
	.id = 2,
};

/* HDMI */
static void manta_hdmiphy_enable(struct platform_device *pdev, int en)
{
	s5p_hdmiphy_enable(pdev, en ? 1 : 0);
}

static struct s5p_hdmi_platdata hdmi_platdata __initdata = {
	.hdmiphy_enable = manta_hdmiphy_enable,
};

static struct platform_device *manta_display_devices[] __initdata = {
	&exynos_device_md0,
	&exynos_device_md1,
	&exynos_device_md2,

	&s5p_device_fimd1,
	&manta_lcd,
	&s5p_device_dp,

	&s5p_device_i2c_hdmiphy,
	&s5p_device_mixer,
	&s5p_device_hdmi,
};

void __init exynos5_manta_display_init(void)
{
	struct resource *res;

	/* LCD_EN , XMMC2CDN => GPC2_2 */
	gpio_request_one(GPIO_LCD_EN, GPIOF_OUT_INIT_LOW, "LCD_EN");
	/* LED_BACKLIGHT_RESET: XCI1RGB_5 => GPG0_5 */
	gpio_request_one(GPIO_LED_BL_RST, GPIOF_OUT_INIT_LOW, "LED_BL_RST");
	s5p_gpio_set_pd_cfg(GPIO_LED_BL_RST, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(GPIO_LED_BL_RST, S5P_GPIO_PD_UPDOWN_DISABLE);

	gpio_request_one(GPIO_LCD_PWM_IN_18V, GPIOF_OUT_INIT_LOW, "PWM_IN_18V");
	s5p_gpio_set_pd_cfg(GPIO_LCD_PWM_IN_18V, S5P_GPIO_PD_INPUT);
	s5p_gpio_set_pd_pull(GPIO_LCD_PWM_IN_18V, S5P_GPIO_PD_UP_ENABLE);
	gpio_free(GPIO_LCD_PWM_IN_18V);

	gpio_request_one(GPIO_APS_EN_18V, GPIOF_OUT_INIT_LOW, "APS_EN_18V");
	s5p_gpio_set_pd_cfg(GPIO_APS_EN_18V, S5P_GPIO_PD_INPUT);
	s5p_gpio_set_pd_pull(GPIO_APS_EN_18V, S5P_GPIO_PD_UP_ENABLE);
	gpio_export(GPIO_APS_EN_18V, true);

	samsung_bl_set(&manta_bl_gpio_info, &manta_bl_data);
	s5p_fimd1_set_platdata(&manta_lcd1_pdata);
	dev_set_name(&s5p_device_fimd1.dev, "exynos5-fb.1");
	clk_add_alias("lcd", "exynos5-fb.1", "fimd", &s5p_device_fimd1.dev);
	s5p_dp_set_platdata(&manta_dp_data);

	gpio_request_one(GPIO_HDMI_HPD, GPIOF_IN, "HDMI_HPD");
	/* HDMI Companion DC/DC converter and HPD circuitry */
	gpio_request_one(GPIO_HDMI_DCDC_EN, GPIOF_OUT_INIT_HIGH, "HDMI_DCDC_EN");
	/* HDMI Companion level shifters and LDO */
	gpio_request_one(GPIO_HDMI_LS_EN, GPIOF_OUT_INIT_HIGH, "HDMI_LS_EN");

	s5p_hdmi_set_platdata(&hdmi_platdata);
	dev_set_name(&s5p_device_hdmi.dev, "exynos5-hdmi");
	clk_add_alias("hdmi", "s5p-hdmi", "hdmi", &s5p_device_hdmi.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(tv).dev, &s5p_device_mixer.dev);
	s5p_i2c_hdmiphy_set_platdata(NULL);

	platform_add_devices(manta_display_devices,
			     ARRAY_SIZE(manta_display_devices));

	exynos5_fimd1_setup_clock(&s5p_device_fimd1.dev,
				"sclk_fimd", "sclk_vpll", 268 * MHZ);

	res = platform_get_resource(&s5p_device_fimd1, IORESOURCE_MEM, 1);
	if (res) {
		res->start = manta_bootloader_fb_start;
		res->end = res->start + manta_bootloader_fb_size - 1;
		pr_info("bootloader fb located at %8X-%8X\n", res->start,
				res->end);
	} else {
		pr_err("failed to find bootloader fb resource\n");
	}
}
