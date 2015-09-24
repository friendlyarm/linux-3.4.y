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

#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mfd/wm8994/gpio.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/platform_data/es305.h>
#include <linux/platform_device.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>

#include <mach/gpio.h>

#include <plat/devs.h>
#include <plat/gpio-cfg.h>

#include "board-manta.h"

#define GPIO_ES305_WAKEUP	EXYNOS5_GPG0(3)
#define GPIO_ES305_RESET	EXYNOS5_GPG0(4)
#define GPIO_ES305_CLK_EN	EXYNOS5_GPG0(6)
#define GPIO_CODEC_LDO_EN	EXYNOS5_GPH1(1)

static struct clk *clkout;

static void manta_es305_clk_enable(bool enable)
{
	int hw_rev = exynos5_manta_get_revision();

	if (enable) {
		if (hw_rev >= MANTA_REV_PRE_ALPHA)
			gpio_set_value(GPIO_ES305_CLK_EN, 1);
		else
			clk_enable(clkout);
	} else {
		if (hw_rev >= MANTA_REV_PRE_ALPHA)
			gpio_set_value(GPIO_ES305_CLK_EN, 0);
		else
			clk_disable(clkout);
	}
}

static struct es305_platform_data es305_pdata = {
	.gpio_wakeup = GPIO_ES305_WAKEUP,
	.gpio_reset = GPIO_ES305_RESET,
	.clk_enable = manta_es305_clk_enable,
	.passthrough_src = 1, /* port A */
	.passthrough_dst = 4, /* port D */
};

static struct i2c_board_info i2c_devs4[] __initdata = {
	{
		I2C_BOARD_INFO("audience_es305", 0x3e),
		.platform_data = &es305_pdata,
	},
};

static struct regulator_consumer_supply vbatt_supplies[] = {
	REGULATOR_SUPPLY("LDO1VDD", "7-001a"),
	REGULATOR_SUPPLY("SPKVDD1", "7-001a"),
	REGULATOR_SUPPLY("SPKVDD2", "7-001a"),
};

static struct regulator_init_data vbatt_initdata = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(vbatt_supplies),
	.consumer_supplies = vbatt_supplies,
};

static struct fixed_voltage_config vbatt_config = {
	.init_data = &vbatt_initdata,
	.microvolts = 3700000,
	.supply_name = "VBATT",
	.gpio = -EINVAL,
};

static struct platform_device vbatt_device = {
	.name = "reg-fixed-voltage",
	.id = -1,
	.dev = {
		.platform_data = &vbatt_config,
	},
};

static struct regulator_consumer_supply wm1811_ldo1_supplies[] = {
	REGULATOR_SUPPLY("AVDD1", "7-001a"),
};

static struct regulator_init_data wm1811_ldo1_initdata = {
	.constraints = {
		.name = "WM1811 LDO1",
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(wm1811_ldo1_supplies),
	.consumer_supplies = wm1811_ldo1_supplies,
};

static struct regulator_consumer_supply wm1811_ldo2_supplies[] = {
	REGULATOR_SUPPLY("DCVDD", "7-001a"),
};

static struct regulator_init_data wm1811_ldo2_initdata = {
	.constraints = {
		.name = "WM1811 LDO2",
		.always_on = true,  /* Actually status changed by LDO1 */
	},
	.num_consumer_supplies = ARRAY_SIZE(wm1811_ldo2_supplies),
	.consumer_supplies = wm1811_ldo2_supplies,
};

static struct wm8994_drc_cfg wm1811_drc_cfgs[] = {
	{
		.name = "Default",
		.regs = {0x0098, 0x0845, 0x0, 0x0, 0x0 }
	},
	{
		.name = "Speakers Media",
		.regs = {0x0098, 0x0245, 0x0028, 0x00c6, 0x0 }
	}
};

static struct wm8958_micd_rate manta_micd_rates[] = {
	{ 32768,	true,	0,	1 },
	{ 32768,	false,	0,	1 },
	{ 44100 * 256,	true,	8,	8 },
	{ 44100 * 256,	false,	8,	8 },
};

static struct wm8994_pdata wm1811_pdata = {
	.gpio_defaults = {
		[0] = WM8994_GP_FN_IRQ, /* GPIO1 IRQ output, CMOS mode */
		[7] = WM8994_GPN_DIR |
				WM8994_GP_FN_PIN_SPECIFIC, /* DACDAT3 */
		[8] = WM8994_CONFIGURE_GPIO |
				WM8994_GP_FN_PIN_SPECIFIC, /* ADCDAT3 */
		[9] = WM8994_CONFIGURE_GPIO |
				WM8994_GP_FN_PIN_SPECIFIC, /* LRCLK3 */
		[10] = WM8994_CONFIGURE_GPIO |
				WM8994_GP_FN_PIN_SPECIFIC, /* BCLK3 */
	},

	/* The enable is shared but assign it to LDO1 for software */
	.ldo = {
		{
			.enable = GPIO_CODEC_LDO_EN,
			.init_data = &wm1811_ldo1_initdata,
		},
		{
			.init_data = &wm1811_ldo2_initdata,
		},
	},

	.num_drc_cfgs = ARRAY_SIZE(wm1811_drc_cfgs),
	.drc_cfgs = wm1811_drc_cfgs,

	.num_micd_rates = ARRAY_SIZE(manta_micd_rates),
	.micd_rates = manta_micd_rates,

	/* Regulated mode at highest output voltage */
	.micbias = {0x2f, 0x2f},
	.micd_lvl_sel = 0xff,

	/* Support external capacitors*/
	.jd_ext_cap = 1,

	.ldo_ena_always_driven = true,
	.irq_base = MANTA_IRQ_BOARD_AUDIO_START,

	/*
	 * Restrict the i2s clock for a maximum of 2 channels to keep
	 * the bus within spec since i2s0 is shared with the HDMI block
	 */
	.max_channels_clocked = {2, 2, 2},
};

static struct i2c_board_info i2c_devs7[] __initdata = {
	{
		I2C_BOARD_INFO("wm1811", (0x34 >> 1)), /* Audio codec */
		.platform_data = &wm1811_pdata,
		.irq = IRQ_EINT(29),
	},
};

static struct platform_device manta_i2s_device = {
	.name	= "manta-i2s",
	.id	= -1,
};

static struct platform_device manta_spdif_device = {
	.name	= "manta-spdif",
	.id	= -1,
};

static struct platform_device manta_spdif_dit_device = {
	.name	= "spdif-dit",
	.id	= -1,
};

static struct platform_device *manta_audio_devices[] __initdata = {
	&vbatt_device,
	&samsung_asoc_dma,
	&samsung_asoc_idma,
	&exynos5_device_srp,
	&exynos5_device_i2s0,
	&exynos5_device_pcm0,
	&exynos5_device_spdif,
	&manta_i2s_device,
	&manta_spdif_device,
	&manta_spdif_dit_device,
};

static void manta_audio_setup_clocks(void)
{
	struct clk *fout_epll, *mout_epll;
	struct clk *sclk_audio, *sclk_spdif;
	struct clk *xxti;
	int ret;

	fout_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(fout_epll)) {
		pr_err("%s:cannot get fout_epll clock\n", __func__);
		return;
	}

	mout_epll = clk_get(NULL, "mout_epll");
	if (IS_ERR(mout_epll)) {
		pr_err("%s: cannot get mout_epll clock\n", __func__);
		goto out1;
	}

	sclk_audio = clk_get(NULL, "sclk_audio");
	if (IS_ERR(sclk_audio)) {
		pr_err("%s: cannot get sclk_audio clock\n", __func__);
		goto out2;
	}

	sclk_spdif = clk_get(NULL, "sclk_spdif");
	if (IS_ERR(sclk_spdif)) {
		pr_err("%s: cannot get sclk_spdif clock\n", __func__);
		goto out3;
	}

	xxti = clk_get(NULL, "xxti");
	if (IS_ERR(xxti)) {
		pr_err("%s: cannot get xxti clock\n", __func__);
		goto out4;
	}

	clkout = clk_get(NULL, "clkout");
	if (IS_ERR(clkout)) {
		pr_err("%s: cannot get clkout\n", __func__);
		goto out5;
	}

	clk_set_parent(mout_epll, fout_epll);
	clk_set_parent(sclk_audio, mout_epll);
	clk_set_parent(sclk_spdif, sclk_audio);
	clk_set_parent(clkout, xxti);
	clk_add_alias("system_clk", "manta-i2s", "clkout", NULL);

	ret = gpio_request(GPIO_ES305_CLK_EN, "ES305 clk_en");
	if (ret < 0) {
		pr_err("%s: error requesting ES305 clk_en gpio\n", __func__);
		goto out6;
	}
	gpio_direction_output(GPIO_ES305_CLK_EN, 0);

	clk_put(fout_epll);
	clk_put(mout_epll);
	clk_put(sclk_audio);
	clk_put(sclk_spdif);
	clk_put(xxti);

	return;
out6:
	clk_put(clkout);
out5:
	clk_put(xxti);
out4:
	clk_put(sclk_spdif);
out3:
	clk_put(sclk_audio);
out2:
	clk_put(mout_epll);
out1:
	clk_put(fout_epll);
}

void __init exynos5_manta_audio_init(void)
{
	manta_audio_setup_clocks();

	s5p_gpio_set_pd_cfg(GPIO_ES305_WAKEUP, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_cfg(GPIO_ES305_RESET, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_cfg(GPIO_ES305_CLK_EN, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_cfg(GPIO_CODEC_LDO_EN, S5P_GPIO_PD_PREV_STATE);

	i2c_register_board_info(4, i2c_devs4, ARRAY_SIZE(i2c_devs4));
	i2c_register_board_info(7, i2c_devs7, ARRAY_SIZE(i2c_devs7));

	platform_add_devices(manta_audio_devices,
				ARRAY_SIZE(manta_audio_devices));
}
