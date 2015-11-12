/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/errno.h>
#include <linux/cma.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/ion.h>
#include <linux/i2c.h>
#include <linux/keyreset.h>
#include <linux/mmc/host.h>
#include <linux/memblock.h>
#include <linux/persistent_ram.h>
#include <linux/platform_device.h>
#include <linux/platform_data/exynos_usb3_drd.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/serial_core.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/sys_soc.h>
#include <linux/platform_data/stmpe811-adc.h>
#include <linux/leds-as3668.h>

#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>
#include <asm/system_info.h>
#include <asm/system_misc.h>

#include <plat/adc.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/regs-serial.h>
#include <plat/gpio-cfg.h>
#include <plat/devs.h>
#include <plat/iic.h>
#include <plat/sdhci.h>
#include <plat/udc-hs.h>
#include <plat/ehci.h>

#include <mach/map.h>
#include <mach/sysmmu.h>
#include <mach/exynos_fiq_debugger.h>
#include <mach/exynos-ion.h>
#include <mach/dwmci.h>
#include <mach/ohci.h>
#include <mach/tmu.h>
#include <mach/exynos5_bus.h>

#include "../../../drivers/staging/android/ram_console.h"
#include "board-manta.h"
#include "common.h"
#include "resetreason.h"

#define MANTA_CPU0_DEBUG_PA		0x10890000
#define MANTA_CPU1_DEBUG_PA		0x10892000
#define MANTA_CPU_DBGPCSR		0xa0

static int manta_hw_rev;
phys_addr_t manta_bootloader_fb_start;
phys_addr_t manta_bootloader_fb_size = 2560 * 1600 * 4;
static bool manta_charger_mode;
static void __iomem *manta_cpu0_debug;
static void __iomem *manta_cpu1_debug;

static int __init s3cfb_bootloaderfb_arg(char *options)
{
	char *p = options;

	manta_bootloader_fb_start = memparse(p, &p);
	pr_debug("bootloader framebuffer found at %8X\n",
			manta_bootloader_fb_start);

	return 0;
}
early_param("s3cfb.bootloaderfb", s3cfb_bootloaderfb_arg);

static int __init manta_androidboot_mode_arg(char *options)
{
	if (!strcmp(options, "charger"))
		manta_charger_mode = true;
	return 0;
}
early_param("androidboot.mode", manta_androidboot_mode_arg);

static struct gpio manta_hw_rev_gpios[] = {
	{EXYNOS5_GPV1(4), GPIOF_IN, "hw_rev0"},
	{EXYNOS5_GPV1(3), GPIOF_IN, "hw_rev1"},
	{EXYNOS5_GPV1(2), GPIOF_IN, "hw_rev2"},
	{EXYNOS5_GPV1(1), GPIOF_IN, "hw_rev3"},
};

int exynos5_manta_get_revision(void)
{
	return manta_hw_rev;
}

static char manta_board_info_string[255];

static void manta_init_hw_rev(void)
{
	int ret;
	int i;

	ret = gpio_request_array(manta_hw_rev_gpios,
		ARRAY_SIZE(manta_hw_rev_gpios));

	BUG_ON(ret);

	for (i = 0; i < ARRAY_SIZE(manta_hw_rev_gpios); i++)
		manta_hw_rev |= gpio_get_value(manta_hw_rev_gpios[i].gpio) << i;

	snprintf(manta_board_info_string, sizeof(manta_board_info_string) - 1,
		"Manta HW revision: %d, CPU EXYNOS5250 Rev%d.%d",
		manta_hw_rev,
		samsung_rev() >> 4,
		samsung_rev() & 0xf);
	pr_info("%s\n", manta_board_info_string);
	mach_panic_string = manta_board_info_string;
}

static struct ram_console_platform_data ramconsole_pdata;

static struct platform_device ramconsole_device = {
	.name           = "ram_console",
	.id             = -1,
	.dev		= {
		.platform_data = &ramconsole_pdata,
	},
};

static struct platform_device persistent_trace_device = {
	.name           = "persistent_trace",
	.id             = -1,
};

static struct resource persistent_clock_resource[] = {
	[0] = DEFINE_RES_MEM(S3C_PA_RTC, SZ_256),
};


static struct platform_device persistent_clock = {
	.name           = "persistent_clock",
	.id             = -1,
	.num_resources	= ARRAY_SIZE(persistent_clock_resource),
	.resource	= persistent_clock_resource,
};

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define MANTA_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define MANTA_ULCON_DEFAULT	S3C2410_LCON_CS8

#define MANTA_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg manta_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= MANTA_UCON_DEFAULT,
		.ulcon		= MANTA_ULCON_DEFAULT,
		.ufcon		= MANTA_UFCON_DEFAULT,
		.wake_peer	= bcm_bt_lpm_exit_lpm_locked,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= MANTA_UCON_DEFAULT,
		.ulcon		= MANTA_ULCON_DEFAULT,
		.ufcon		= MANTA_UFCON_DEFAULT,
	},
	/* Do not initialize hwport 2, it will be handled by fiq_debugger */
	[2] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= MANTA_UCON_DEFAULT,
		.ulcon		= MANTA_ULCON_DEFAULT,
		.ufcon		= MANTA_UFCON_DEFAULT,
	},
};

static struct gpio_event_direct_entry manta_keypad_key_map[] = {
	{
		.gpio   = EXYNOS5_GPX2(7),
		.code   = KEY_POWER,
		.dev    = 0,
	},
	{
		.gpio   = EXYNOS5_GPX2(0),
		.code   = KEY_VOLUMEUP,
		.dev    = 0,
	},
	{
		.gpio   = EXYNOS5_GPX2(1),
		.code   = KEY_VOLUMEDOWN,
		.dev    = 0,
	}
};

static struct gpio_event_direct_entry manta_switch_map[] = {
	{
		.gpio   = EXYNOS5_GPX1(3),
		.code   = SW_LID,
		.dev    = 1,
	}
};

static struct gpio_event_input_info manta_keypad_key_info = {
	.info.func              = gpio_event_input_func,
	.info.no_suspend        = true,
	.debounce_time.tv64     = 5 * NSEC_PER_MSEC,
	.type                   = EV_KEY,
	.keymap                 = manta_keypad_key_map,
	.keymap_size            = ARRAY_SIZE(manta_keypad_key_map)
};

static struct gpio_event_input_info manta_switch_info = {
	.info.func              = gpio_event_input_func,
	.info.no_suspend        = true,
	.debounce_time.tv64     = 10 * NSEC_PER_MSEC,
	.type                   = EV_SW,
	.keymap                 = manta_switch_map,
	.keymap_size            = ARRAY_SIZE(manta_switch_map)
};

static struct gpio_event_info *manta_event_input_info[] = {
	&manta_keypad_key_info.info,
	&manta_switch_info.info,
};

static struct gpio_event_platform_data manta_event_data = {
	.names  = {
		"manta-keypad",
		"manta-switch",
		NULL,
	},
	.info           = manta_event_input_info,
	.info_count     = ARRAY_SIZE(manta_event_input_info),
};

static struct platform_device manta_event_device = {
	.name   = GPIO_EVENT_DEV_NAME,
	.id     = 0,
	.dev    = {
		.platform_data = &manta_event_data,
	},
};

static void __init manta_gpio_power_init(void)
{
	int err = 0;

	err = gpio_request_one(EXYNOS5_GPX2(7), 0, "GPX2(7)");
	if (err) {
		printk(KERN_ERR "failed to request GPX2(7) for "
				"suspend/resume control\n");
		return;
	}
	s3c_gpio_setpull(EXYNOS5_GPX2(7), S3C_GPIO_PULL_NONE);

	gpio_free(EXYNOS5_GPX2(7));
}

static int manta_keyreset_fn(void)
{
	arm_pm_restart('h', NULL);
	return 1;
}

static struct keyreset_platform_data manta_reset_keys_pdata = {
	.keys_down	= {
		KEY_POWER,
		0,
	},
	.down_time_ms	= 1500,
	.reset_fn	= manta_keyreset_fn,
};

struct platform_device manta_keyreset_device = {
	.name	= KEYRESET_NAME,
	.dev	= {
		.platform_data	= &manta_reset_keys_pdata,
	},
};

static struct as3668_platform_data as3668_pdata = {
	.led_array = {AS3668_RED, AS3668_GREEN, AS3668_BLUE, AS3668_WHITE},
	.vbat_monitor_voltage_index = AS3668_VMON_VBAT_3_0V,
	.shutdown_enable = AS3668_SHUTDOWN_ENABLE_OFF,
	.pattern_start_source = AS3668_PATTERN_START_SOURCE_SW,
	.pwm_source = AS3668_PWM_SOURCE_INTERNAL,
	.gpio_input_invert = AS3668_GPIO_INPUT_NONINVERT,
	.gpio_input_mode = AS3668_GPIO_INPUT_MODE_ANALOG,
	.gpio_mode = AS3668_GPIO_MODE_INPUT_ONLY,
	.audio_input_pin = AS3668_AUDIO_CTRL_INPUT_CURR4,
	.audio_pulldown_off = AS3668_AUDIO_CTRL_PLDN_ENABLE,
	.audio_adc_characteristic = AS3668_AUDIO_CTRL_ADC_CHAR_250,
	.audio_dis_start = AS3668_AUDIO_INPUT_CAP_PRECHARGE,
	.audio_man_start = AS3668_AUDIO_INPUT_AUTO_PRECHARGE,
};

/* I2C0 */
static struct i2c_board_info i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO("exynos_hdcp", (0x74 >> 1)),
	},
	{
		I2C_BOARD_INFO("exynos_edid", (0xA0 >> 1)),
	},
};

struct s3c2410_platform_i2c i2c0_data __initdata = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 100*1000,
	.sda_delay	= 100,
};

/* I2C1 */
static struct i2c_board_info i2c_devs1[] __initdata = {
	{
		I2C_BOARD_INFO("as3668", 0x42),
		.platform_data = &as3668_pdata,
	},
};

static struct stmpe811_callbacks *stmpe811_cbs;
static void stmpe811_register_callback(struct stmpe811_callbacks *cb)
{
	stmpe811_cbs = cb;
}

int manta_stmpe811_read_adc_data(u8 channel)
{
	if (stmpe811_cbs && stmpe811_cbs->get_adc_data)
		return stmpe811_cbs->get_adc_data(channel);

	return -EINVAL;
}

struct stmpe811_platform_data stmpe811_pdata = {
	.register_cb = stmpe811_register_callback,
};

/* ADC */
static struct s3c_adc_platdata manta_adc_data __initdata = {
	.phy_init       = s3c_adc_phy_init,
	.phy_exit       = s3c_adc_phy_exit,
};

/* I2C2 */
static struct i2c_board_info i2c_devs2[] __initdata = {
	{
		I2C_BOARD_INFO("stmpe811-adc", (0x82 >> 1)),
		.platform_data  = &stmpe811_pdata,
	},
};

/* TMU */
static struct tmu_data manta_tmu_pdata __initdata = {
	.ts = {
		.stop_throttle		= 78,
		.start_throttle		= 80,
		.start_tripping		= 110,
		.start_emergency	= 120,
		.stop_mem_throttle	= 80,
		.start_mem_throttle	= 85,
	},

	.efuse_value	= 80,
	.slope		= 0x10608802,
};

static struct persistent_ram_descriptor manta_prd[] __initdata = {
	{
		.name = "ram_console",
		.size = SZ_2M,
	},
#ifdef CONFIG_PERSISTENT_TRACER
	{
		.name = "persistent_trace",
		.size = SZ_1M,
	},
#endif
};

static struct persistent_ram manta_pr __initdata = {
	.descs = manta_prd,
	.num_descs = ARRAY_SIZE(manta_prd),
	.start = PLAT_PHYS_OFFSET + SZ_1G + SZ_512M,
#ifdef CONFIG_PERSISTENT_TRACER
	.size = 3 * SZ_1M,
#else
	.size = SZ_2M,
#endif
};


/* defined in arch/arm/mach-exynos/reserve-mem.c */
extern void exynos_cma_region_reserve(struct cma_region *,
				struct cma_region *, size_t, const char *);
extern int kbase_carveout_mem_reserve(phys_addr_t size);

static void __init exynos_reserve_mem(void)
{
	static struct cma_region regions[] = {
		{
			.name = "ion",
#ifdef CONFIG_ION_EXYNOS_CONTIGHEAP_SIZE
			.size = CONFIG_ION_EXYNOS_CONTIGHEAP_SIZE * SZ_1K,
#endif
			{
				.alignment = SZ_1M
			}
		},
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
#ifdef CONFIG_ION_EXYNOS_DRM_MFC_SH
		{
			.name = "drm_mfc_sh",
			.size = SZ_1M,
			.alignment = SZ_1M,
		},
#endif
#ifdef CONFIG_ION_EXYNOS_DRM_MSGBOX_SH
		{
			.name = "drm_msgbox_sh",
			.size = SZ_1M,
			.alignment = SZ_1M,
		},
#endif
#endif
		{
			.size = 0 /* END OF REGION DEFINITIONS */
		}
	};
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
       static struct cma_region regions_secure[] = {
#ifdef CONFIG_ION_EXYNOS_DRM_MEMSIZE_FIMD_VIDEO
	       {
		       .name = "drm_fimd_video",
		       .size = CONFIG_ION_EXYNOS_DRM_MEMSIZE_FIMD_VIDEO *
			       SZ_1K,
		       .alignment = SZ_1M,
	       },
#endif
#ifdef CONFIG_ION_EXYNOS_DRM_MEMSIZE_MFC_OUTPUT
	       {
		       .name = "drm_mfc_output",
		       .size = CONFIG_ION_EXYNOS_DRM_MEMSIZE_MFC_OUTPUT *
			       SZ_1K,
		       .alignment = SZ_1M,
	       },
#endif
#ifdef CONFIG_ION_EXYNOS_DRM_MEMSIZE_MFC_INPUT
	       {
		       .name = "drm_mfc_input",
		       .size = CONFIG_ION_EXYNOS_DRM_MEMSIZE_MFC_INPUT *
			       SZ_1K,
		       .alignment = SZ_1M,
	       },
#endif
#ifdef CONFIG_ION_EXYNOS_DRM_MFC_FW
               {
                       .name = "drm_mfc_fw",
                       .size = SZ_1M,
		       .alignment = SZ_1M,
               },
#endif
#ifdef CONFIG_ION_EXYNOS_DRM_SECTBL
               {
                       .name = "drm_sectbl",
                       .size = SZ_1M,
		       .alignment = SZ_1M,
               },
#endif
               {
                       .size = 0
               },
	};
#else /* !CONFIG_EXYNOS_CONTENT_PATH_PROTECTION */
	struct cma_region *regions_secure = NULL;
#endif /* CONFIG_EXYNOS_CONTENT_PATH_PROTECTION */

       static const char map[] __initconst =
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
		"ion-exynos/mfc_sh=drm_mfc_sh;"
		"ion-exynos/msgbox_sh=drm_msgbox_sh;"
		"ion-exynos/fimd_video=drm_fimd_video;"
		"ion-exynos/mfc_output=drm_mfc_output;"
		"ion-exynos/mfc_input=drm_mfc_input;"
		"ion-exynos/mfc_fw=drm_mfc_fw;"
		"ion-exynos/sectbl=drm_sectbl;"
		"s5p-smem/mfc_sh=drm_mfc_sh;"
		"s5p-smem/msgbox_sh=drm_msgbox_sh;"
		"s5p-smem/fimd_video=drm_fimd_video;"
		"s5p-smem/mfc_output=drm_mfc_output;"
		"s5p-smem/mfc_input=drm_mfc_input;"
		"s5p-smem/mfc_fw=drm_mfc_fw;"
		"s5p-smem/sectbl=drm_sectbl;"
#endif
		"ion-exynos=ion;"
		"s5p-mfc-v6/f=fw;"
		"s5p-mfc-v6/a=b1;";

	persistent_ram_early_init(&manta_pr);
	if (manta_bootloader_fb_start) {
		int err = memblock_reserve(manta_bootloader_fb_start,
				manta_bootloader_fb_size);
		if (err)
			pr_warn("failed to reserve old framebuffer location\n");
	} else {
		pr_warn("bootloader framebuffer start address not set\n");
	}

	exynos_cma_region_reserve(regions, regions_secure, 0, map);
	kbase_carveout_mem_reserve(384 * SZ_1M);
	ion_reserve(&exynos_ion_pdata);
}

static void exynos_dwmci0_cfg_gpio(int width)
{
	unsigned int gpio;

	for (gpio = EXYNOS5_GPC0(0); gpio < EXYNOS5_GPC0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
	}

	switch (width) {
	case 8:
		for (gpio = EXYNOS5_GPC1(0); gpio <= EXYNOS5_GPC1(3); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
		}
	case 4:
		for (gpio = EXYNOS5_GPC0(3); gpio <= EXYNOS5_GPC0(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
		}
		break;
	case 1:
		gpio = EXYNOS5_GPC0(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
	default:
		break;
	}
}

static void exynos_dwmci0_hw_reset(u32 slot_id)
{
	unsigned int emmc_en, gpio;

	if (slot_id != 0)
		return;

	emmc_en = EXYNOS5_GPC0(2);
	s3c_gpio_cfgpin(emmc_en, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(emmc_en, S3C_GPIO_PULL_NONE);

	/* eMMC Card Power Off */
	for (gpio = EXYNOS5_GPC0(0); gpio < EXYNOS5_GPC0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio_set_value(gpio, 0);
	}
	for (gpio = EXYNOS5_GPC1(0); gpio <= EXYNOS5_GPC1(3); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio_set_value(gpio, 0);
	}
	gpio_set_value(emmc_en, 0);

	/* waiting ramp down time for certainly power off */
	msleep(100);

	/* eMMC Card Power On */
	for (gpio = EXYNOS5_GPC0(0); gpio < EXYNOS5_GPC0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio_set_value(gpio, 0);
	}
	for (gpio = EXYNOS5_GPC1(0); gpio <= EXYNOS5_GPC1(3); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio_set_value(gpio, 0);
	}
	gpio_set_value(emmc_en, 1);

	/* waiting ramp up time for certainly power on */
	msleep(50);
}

static struct dw_mci_board exynos_dwmci0_pdata __initdata = {
	.num_slots		= 1,
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
				  DW_MCI_QUIRK_HIGHSPEED |
                                  DW_MMC_QUIRK_HW_RESET_PW |
				  DW_MCI_QUIRK_NO_DETECT_EBIT,
	.bus_hz			= 50 * 1000 * 1000,
	.max_bus_hz		= 200 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR |
				  MMC_CAP_8_BIT_DATA | MMC_CAP_CMD23 | MMC_CAP_ERASE |
				  MMC_CAP_HW_RESET,
	.caps2 			= MMC_CAP2_HS200_1_8V_SDR,
	.fifo_depth             = 0x80,
	.detect_delay_ms	= 200,
	.hclk_name		= "dwmci",
	.cclk_name		= "sclk_dwmci",
	.cfg_gpio		= exynos_dwmci0_cfg_gpio,
	.hw_reset		= exynos_dwmci0_hw_reset,
	.sdr_timing		= 0x03020001,
	.ddr_timing		= 0x03030002,
	.clk_drv		= 0x3,
};

/* DEVFREQ controlling mif */
static struct exynos5_bus_mif_platform_data manta_bus_mif_platform_data;

static struct platform_device exynos_bus_mif_devfreq = {
	.name                   = "exynos5-bus-mif",
	.dev = {
		.platform_data	= &manta_bus_mif_platform_data,
	},
};

/* DEVFREQ controlling int */
static struct platform_device exynos_bus_int_devfreq = {
	.name                   = "exynos5-bus-int",
};

static struct platform_device *manta_devices[] __initdata = {
	&ramconsole_device,
	&persistent_trace_device,
	&persistent_clock,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_i2c3,
	&s3c_device_i2c4,
	&s3c_device_i2c5,
	&s3c_device_i2c7,
	&s3c_device_adc,
	&s3c_device_wdt,
	&exynos5_device_dwmci0,
	&exynos_device_ion,
	&exynos_device_tmu,
	&s3c_device_usb_hsotg,
	&s5p_device_ehci,
	&exynos4_device_ohci,
	&exynos_bus_mif_devfreq,
	&exynos_bus_int_devfreq,
	&exynos5_device_g3d,
};

static struct s3c_hsotg_plat manta_hsotg_pdata;

static void __init manta_udc_init(void)
{
	struct s3c_hsotg_plat *pdata = &manta_hsotg_pdata;

	s3c_hsotg_set_platdata(pdata);
}

static void __init manta_dwmci_init(void)
{
	exynos_dwmci_set_platdata(&exynos_dwmci0_pdata, 0);
	dev_set_name(&exynos5_device_dwmci0.dev, "exynos4-sdhci.0");
	clk_add_alias("dwmci", "dw_mmc.0", "hsmmc", &exynos5_device_dwmci0.dev);
	clk_add_alias("sclk_dwmci", "dw_mmc.0", "sclk_mmc",
		      &exynos5_device_dwmci0.dev);
}

static void __init manta_map_io(void)
{
	clk_xusbxti.rate = 24000000;
	clk_xxti.rate = 24000000;
	exynos_init_io(NULL, 0);
	s3c24xx_init_clocks(clk_xusbxti.rate);
	s3c24xx_init_uarts(manta_uartcfgs, ARRAY_SIZE(manta_uartcfgs));
}

static void __init manta_sysmmu_init(void)
{
}

static void __init manta_init_early(void)
{
}

static void __init soc_info_populate(struct soc_device_attribute *soc_dev_attr)
{
	soc_dev_attr->soc_id = kasprintf(GFP_KERNEL, "%08x%08x\n",
			                 system_serial_high, system_serial_low);
	soc_dev_attr->machine = kasprintf(GFP_KERNEL, "Exynos 5250\n");
	soc_dev_attr->family = kasprintf(GFP_KERNEL, "Exynos 5\n");
	soc_dev_attr->revision = kasprintf(GFP_KERNEL, "%d.%d\n",
					   samsung_rev() >> 4,
					   samsung_rev() & 0xf);
}

static ssize_t manta_get_board_revision(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%d\n", manta_hw_rev);
}

struct device_attribute manta_soc_attr =
	__ATTR(board_rev,  S_IRUGO, manta_get_board_revision,  NULL);

static void __init exynos5_manta_sysfs_soc_init(void)
{
	struct device *parent;
	struct soc_device *soc_dev;
	struct soc_device_attribute *soc_dev_attr;

	soc_dev_attr = kzalloc(sizeof(*soc_dev_attr), GFP_KERNEL);
	if (!soc_dev_attr) {
		printk(KERN_ERR "Failed to allocate memory for soc_dev_attr\n");
		return;
	}

	soc_info_populate(soc_dev_attr);

	soc_dev = soc_device_register(soc_dev_attr);
	if (IS_ERR_OR_NULL(soc_dev)) {
		kfree(soc_dev_attr);
		printk(KERN_ERR "Failed to register a soc device under /sys\n");
		return;
	}

	parent = soc_device_to_device(soc_dev);
	if (!IS_ERR_OR_NULL(parent))
		device_create_file(parent, &manta_soc_attr);

	return;  /* Or return parent should you need to use one later */
}

/* USB EHCI */
static struct s5p_ehci_platdata exynos5_manta_ehci_pdata;

static void __init exynos5_manta_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &exynos5_manta_ehci_pdata;

	s5p_ehci_set_platdata(pdata);
}

/* USB OHCI */
static struct exynos4_ohci_platdata exynos5_manta_ohci_pdata;

static void __init exynos5_manta_ohci_init(void)
{
	struct exynos4_ohci_platdata *pdata = &exynos5_manta_ohci_pdata;

	exynos4_ohci_set_platdata(pdata);
}

void manta_panic_dump_cpu_pc(int cpu, unsigned long dbgpcsr)
{
	void *pc = NULL;

	pr_err("CPU%d DBGPCSR: %08lx\n", cpu, dbgpcsr);
	if ((dbgpcsr & 3) == 0)
		pc = (void *)(dbgpcsr - 8);
	else if ((dbgpcsr & 1) == 1)
		pc = (void *)((dbgpcsr & ~1) - 4);

	pr_err("CPU%d PC: <%p> %pF\n", cpu, pc, pc);
}

int manta_panic_notify(struct notifier_block *nb, unsigned long event, void *p)
{
	unsigned long dbgpcsr;

	if (manta_cpu0_debug && cpu_online(0)) {
		dbgpcsr = __raw_readl(manta_cpu0_debug + MANTA_CPU_DBGPCSR);
		manta_panic_dump_cpu_pc(0, dbgpcsr);
	}
	if (manta_cpu1_debug && cpu_online(1)) {
		dbgpcsr = __raw_readl(manta_cpu1_debug + MANTA_CPU_DBGPCSR);
		manta_panic_dump_cpu_pc(1, dbgpcsr);
	}
	return NOTIFY_OK;
}

struct notifier_block manta_panic_nb = {
	.notifier_call = manta_panic_notify,
};

static void __init manta_panic_init(void)
{
	manta_cpu0_debug = ioremap(MANTA_CPU0_DEBUG_PA, SZ_4K);
	manta_cpu1_debug = ioremap(MANTA_CPU1_DEBUG_PA, SZ_4K);

	atomic_notifier_chain_register(&panic_notifier_list, &manta_panic_nb);
}

static void __init manta_machine_init(void)
{
	manta_init_hw_rev();

	if (manta_hw_rev <= MANTA_REV_DOGFOOD02)
		manta_bus_mif_platform_data.max_freq = 667000;

	exynos_serial_debug_init(2, 0);
	manta_panic_init();

	manta_gpio_power_init();
	platform_device_register(&manta_event_device);

	manta_sysmmu_init();
	manta_dwmci_init();

	if (manta_charger_mode)
		platform_device_register(&manta_keyreset_device);

	s3c_i2c0_set_platdata(&i2c0_data);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	s3c_i2c3_set_platdata(NULL);
	s3c_i2c4_set_platdata(NULL);
	s3c_i2c5_set_platdata(NULL);
	s3c_i2c7_set_platdata(NULL);

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
	if (exynos5_manta_get_revision() <= MANTA_REV_LUNCHBOX)
		i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
	else
		s3c_adc_set_platdata(&manta_adc_data);

	exynos_tmu_set_platdata(&manta_tmu_pdata);

	manta_udc_init();
	exynos5_manta_ehci_init();
	exynos5_manta_ohci_init();
	ramconsole_pdata.bootinfo = exynos_get_resetreason();
	platform_add_devices(manta_devices, ARRAY_SIZE(manta_devices));

	exynos5_manta_power_init();
	exynos5_manta_display_init();
	exynos5_manta_input_init();
	exynos5_manta_battery_init();
	exynos5_manta_pogo_init();
	exynos5_manta_wlan_init();
	exynos5_manta_audio_init();
	exynos5_manta_media_init();
	exynos5_manta_camera_init();
	exynos5_manta_sensors_init();
	exynos5_manta_gps_init();
	exynos5_manta_jack_init();
	exynos5_manta_vib_init();
	exynos5_manta_sysfs_soc_init();
	exynos5_manta_nfc_init();
	exynos5_manta_bt_init();
	exynos5_manta_connector_init();
	exynos5_manta_adjust_mif_asv_table();
}

MACHINE_START(MANTA, "Manta")
	.atag_offset	= 0x100,
	.init_early	= manta_init_early,
	.init_irq	= exynos5_init_irq,
	.map_io		= manta_map_io,
	.handle_irq	= gic_handle_irq,
	.init_machine	= manta_machine_init,
	.timer		= &exynos4_timer,
	.restart	= exynos5_restart,
	.reserve	= exynos_reserve_mem,
MACHINE_END
