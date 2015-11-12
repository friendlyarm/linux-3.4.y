/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/irq.h>
#include <linux/amba/pl022.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#if defined(CONFIG_NXP_HDMI_CEC)
#include <mach/nxp-hdmi-cec.h>
#endif

/*------------------------------------------------------------------------------
 * CPU Frequence
 */
#if defined(CONFIG_ARM_NXP_CPUFREQ)

static unsigned long dfs_freq_table[][2] = {
       { 1400000, 1240000, },
       { 1300000, 1180000, },
       { 1200000, 1140000, },
       { 1100000, 1100000, },
       { 1000000, 1060000, },
       {  900000, 1040000, },
       {  800000, 1000000, },
       {  700000,  940000, },
       {  600000,  940000, },
       {  500000,  940000, },
       {  400000,  940000, },
};

struct nxp_cpufreq_plat_data dfs_plat_data = {
	.pll_dev	   	= CONFIG_NXP_CPUFREQ_PLLDEV,
	.supply_name	= "vdd_arm_1.3V",	//refer to CONFIG_REGULATOR_NXE2000
	.supply_delay_us = 0,
	.freq_table	   	= dfs_freq_table,
	.table_size	   	= ARRAY_SIZE(dfs_freq_table),
};

static struct platform_device dfs_plat_device = {
	.name			= DEV_NAME_CPUFREQ,
	.dev			= {
		.platform_data	= &dfs_plat_data,
	}
};

#endif

/*------------------------------------------------------------------------------
 * SLsiAP Thermal Unit
 */

#if defined(CONFIG_SENSORS_NXP_TMU)
struct nxp_tmu_trigger tmu_triggers[] = {
       {
               .trig_degree    =  85,
               .trig_duration  =  10,
               .trig_cpufreq   =  400*1000,    /* Khz */
       },
};

static struct nxp_tmu_platdata tmu_data = {
       .channel  = 0,
       .triggers = tmu_triggers,
       .trigger_size = ARRAY_SIZE(tmu_triggers),
       .poll_duration = 100,
};

static struct platform_device tmu_device = {
       .name                   = "nxp-tmu",
			 .dev			= {
					.platform_data  = &tmu_data,
				}
};
#endif

/*------------------------------------------------------------------------------
 * Network DM9000
 */
#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
#include <linux/dm9000.h>

static struct resource dm9000_resource[] = {
	[0] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR,
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 1,		// 1 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[1] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR + 4,		// + 4 (8/16 BIT)
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 5,		// + 5 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[2] = {
		.start	= CFG_ETHER_EXT_IRQ_NUM,
		.end	= CFG_ETHER_EXT_IRQ_NUM,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	}
};

static struct dm9000_plat_data eth_plat_data = {
	.flags		= DM9000_PLATF_8BITONLY,	// DM9000_PLATF_16BITONLY
};

static struct platform_device dm9000_plat_device = {
	.name			= "dm9000",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(dm9000_resource),
	.resource		= dm9000_resource,
	.dev			= {
		.platform_data	= &eth_plat_data,
	}
};
#endif	/* CONFIG_DM9000 || CONFIG_DM9000_MODULE */

/*------------------------------------------------------------------------------
 * DW GMAC board config
 */
#if defined(CONFIG_NXPMAC_ETH)
#include <linux/phy.h>
#include <linux/nxpmac.h>
#include <linux/delay.h>
#include <linux/gpio.h>
int  nxpmac_init(struct platform_device *pdev)
{
    u32 addr;

	/* Clock control */
	NX_CLKGEN_Initialize();
	addr = NX_CLKGEN_GetPhysicalAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE);
	NX_CLKGEN_SetBaseAddress( CLOCKINDEX_OF_DWC_GMAC_MODULE, (void*)IO_ADDRESS(addr) );

	NX_CLKGEN_SetClockSource( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 4);     // Sync mode for 100 & 10Base-T : External RX_clk
	NX_CLKGEN_SetClockDivisor( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 1);    // Sync mode for 100 & 10Base-T

	NX_CLKGEN_SetClockOutInv( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CFALSE);    // TX Clk invert off : 100 & 10Base-T

	NX_CLKGEN_SetClockDivisorEnable( CLOCKINDEX_OF_DWC_GMAC_MODULE, CTRUE);


	/* Reset control */
	NX_RSTCON_Initialize();
	addr = NX_RSTCON_GetPhysicalAddress();
	NX_RSTCON_SetBaseAddress( (void*)IO_ADDRESS(addr) );
	NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_NEGATE);
	udelay(100);
	NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ASSERT);
	udelay(100);
	NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_NEGATE);
	udelay(100);


    gpio_request(CFG_ETHER_GMAC_PHY_RST_NUM, "Ethernet Rst pin");
	gpio_direction_output(CFG_ETHER_GMAC_PHY_RST_NUM, 1);
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0);
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1);

	gpio_free(CFG_ETHER_GMAC_PHY_RST_NUM);

	printk("NXP mac init ..................\n");
	return 0;
}

int gmac_phy_reset(void *priv)
{
	// Set GPIO nReset
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	msleep( 30 );

	return 0;
}

static struct stmmac_mdio_bus_data nxpmac0_mdio_bus = {
	.phy_reset = gmac_phy_reset,
	.phy_mask = 0,
	.probed_phy_irq = CFG_ETHER_GMAC_PHY_IRQ_NUM,
};

static struct plat_stmmacenet_data nxpmac_plat_data = {
    .phy_addr = 3,// 7 for 8211 3 for 8201
    .clk_csr = 0x4, // PCLK 150~250 Mhz
    .speed = SPEED_1000,	// SPEED_1000
    .interface = PHY_INTERFACE_MODE_RGMII,
	.autoneg = AUTONEG_ENABLE, //AUTONEG_ENABLE or AUTONEG_DISABLE
	.duplex = DUPLEX_FULL,
	.pbl = 16,          /* burst 16 */
	.has_gmac = 1,      /* GMAC ethernet    */
	.enh_desc = 1,
	.mdio_bus_data = &nxpmac0_mdio_bus,
	//.init = &nxpmac_init,
};

/* DWC GMAC Controller registration */

static struct resource nxpmac_resource[] = {
    [0] = DEFINE_RES_MEM(PHY_BASEADDR_GMAC, SZ_8K),
    [1] = DEFINE_RES_IRQ_NAMED(IRQ_PHY_GMAC, "macirq"),
};

static u64 nxpmac_dmamask = DMA_BIT_MASK(32);

struct platform_device nxp_gmac_dev = {
    .name           = "stmmaceth",  //"s5p4418-gmac",
    .id             = -1,
    .num_resources  = ARRAY_SIZE(nxpmac_resource),
    .resource       = nxpmac_resource,
    .dev            = {
        .dma_mask           = &nxpmac_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
        .platform_data      = &nxpmac_plat_data,
    }
};
#endif

/*------------------------------------------------------------------------------
 * DISPLAY (LVDS) / FB
 */
#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
static struct nxp_fb_plat_data fb0_plat_data = {
	.module			= CONFIG_FB0_NXP_DISPOUT,
	.layer			= CFG_DISP_PRI_SCREEN_LAYER,
	.format			= CFG_DISP_PRI_SCREEN_RGB_FORMAT,
	.bgcolor		= CFG_DISP_PRI_BACK_GROUND_COLOR,
	.bitperpixel	= CFG_DISP_PRI_SCREEN_PIXEL_BYTE * 8,
	.x_resol		= CFG_DISP_PRI_RESOL_WIDTH,
	.y_resol		= CFG_DISP_PRI_RESOL_HEIGHT,
	#ifdef CONFIG_ANDROID
	.buffers		= 3,
	.skip_pan_vsync	= 1,
	#else
	.buffers		= 2,
	#endif
	.lcd_with_mm	= CFG_DISP_PRI_LCD_WIDTH_MM,	/* 152.4 */
	.lcd_height_mm	= CFG_DISP_PRI_LCD_HEIGHT_MM,	/* 91.44 */
};

static struct platform_device fb0_device = {
	.name	= DEV_NAME_FB,
	.id		= 0,	/* FB device node num */
	.dev    = {
		.coherent_dma_mask 	= 0xffffffffUL,	/* for DMA allocate */
		.platform_data		= &fb0_plat_data
	},
};
#endif

static struct platform_device *fb_devices[] = {
	#if defined (CONFIG_FB0_NXP)
	&fb0_device,
	#endif
};
#endif /* CONFIG_FB_NXP */

/*------------------------------------------------------------------------------
 * backlight : generic pwm device
 */
#if defined(CONFIG_BACKLIGHT_PWM)
#include <linux/pwm_backlight.h>

static struct platform_pwm_backlight_data bl_plat_data = {
	.pwm_id			= CFG_LCD_PRI_PWM_CH,
	.max_brightness = 350,	/* 255 is 100%, set over 100% */
	.dft_brightness = 128,	/* 50% */
	.pwm_period_ns	= 1000000000/CFG_LCD_PRI_PWM_FREQ,
};

static struct platform_device bl_plat_device = {
	.name	= "pwm-backlight",
	.id		= -1,
	.dev	= {
		.platform_data	= &bl_plat_data,
	},
};
#endif

#if defined(CONFIG_PPM_NXP)
#include <mach/ppm.h>
struct nxp_ppm_platform_data ppm_plat_data = {
    .input_polarity = NX_PPM_INPUTPOL_INVERT,//NX_PPM_INPUTPOL_INVERT  or  NX_PPM_INPUTPOL_BYPASS
};

static struct platform_device ppm_device = {
    .name           = DEV_NAME_PPM,
    .dev            = {
    	.platform_data  = &ppm_plat_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * NAND device
 */
#if defined(CONFIG_MTD_NAND_NXP)
#include <linux/mtd/partitions.h>
#include <asm-generic/sizes.h>

static struct mtd_partition nxp_nand_parts[] = {
#if 1
	{
		.name           = "root",
		.offset         =   0 * SZ_1M,
	},
#else
	{
		.name		= "system",
		.offset		=  64 * SZ_1M,
		.size		= 512 * SZ_1M,
	}, {
		.name		= "cache",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 256 * SZ_1M,
	}, {
		.name		= "userdata",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	}
#endif
};

static struct nxp_nand_plat_data nand_plat_data = {
	.parts		= nxp_nand_parts,
	.nr_parts	= ARRAY_SIZE(nxp_nand_parts),
	.chip_delay = 10,
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
		.platform_data	= &nand_plat_data,
	},
};
#elif defined(CONFIG_NXP_FTL)
static struct resource nand_resource =
{
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
	},
};
#endif	/* CONFIG_NXP_FTL */

/*------------------------------------------------------------------------------
 * Touch platform device
 */
#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
#include <linux/i2c.h>
#define	FT5X0X_I2C_BUS		(1)

struct nxp_ts_cali_plat_data ts_plat_data = {
	.touch_points	= 10,
	.x_resol	   	= CFG_DISP_PRI_RESOL_WIDTH,
	.y_resol	   	= CFG_DISP_PRI_RESOL_HEIGHT,
	.rotate			= 0,
	.pointercal		= {38242, -340, -453008, -184, 39059, -149296, 65536}
};

static struct i2c_board_info __initdata ft5x0x_i2c_bdi = {
	.type	= "ft5x06_ts",
	.addr	= (0x70>>1),
    .irq    = PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),
	.platform_data = &ts_plat_data,
};
#endif


/*------------------------------------------------------------------------------
 * ANDROID timed gpio platform device
 */
#if defined(CONFIG_GPIOLIB) && defined(CONFIG_ANDROID_TIMED_GPIO)

#define CONFIG_ANDROID_VIBRATION
#include <../../../../drivers/staging/android/timed_gpio.h>

#define ANDROID_VIBRATION_GPIO    (PAD_GPIO_A + 18)
static struct timed_gpio android_vibration = {
    .name         = "vibrator",
    .gpio         = ANDROID_VIBRATION_GPIO,
    .max_timeout  = 15000, /* ms */
};

static struct timed_gpio_platform_data timed_gpio_data = {
    .num_gpios    = 1,
    .gpios        = &android_vibration,
};

static struct platform_device android_timed_gpios = {
    .name         = "timed-gpio",
    .id           = -1,
	.dev          = {
		.platform_data = &timed_gpio_data,
	},
};
#endif
/*------------------------------------------------------------------------------
 * Keypad platform device
 */
#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)

#include <linux/input.h>

static unsigned int  button_gpio[] = CFG_KEYPAD_KEY_BUTTON;
static unsigned int  button_code[] = CFG_KEYPAD_KEY_CODE;

struct nxp_key_plat_data key_plat_data = {
	.bt_count	= ARRAY_SIZE(button_gpio),
	.bt_io		= button_gpio,
	.bt_code	= button_code,
	.bt_repeat	= 0,
};

static struct platform_device key_plat_device = {
	.name	= DEV_NAME_KEYPAD,
	.id		= -1,
	.dev    = {
		.platform_data	= &key_plat_data
	},
};
#endif	/* CONFIG_KEYBOARD_NXP_KEY || CONFIG_KEYBOARD_NXP_KEY_MODULE */

/*------------------------------------------------------------------------------
 * ASoC Codec platform device
 */
#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
static struct platform_device spdif_transciever = {
	.name	= "spdif-dit",
	.id		= -1,
};

struct nxp_snd_dai_plat_data spdif_trans_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device spdif_trans_dai = {
	.name	= "spdif-transciever",
	.id		= -1,
	.dev	= {
		.platform_data	= &spdif_trans_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_SPDIF_RECEIVER) || defined(CONFIG_SND_SPDIF_RECEIVER_MODULE)
static struct platform_device spdif_receiver = {
	.name	= "spdif-dit-receiver",
	.id		= -1,
};

struct nxp_snd_dai_plat_data spdif_recev_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device spdif_recev_dai = {
	.name	= "spdif-receiver",
	.id		= -1,
	.dev	= {
		.platform_data	= &spdif_recev_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_CODEC_NULL)
static struct platform_device snd_null = {
	.name = "snd-null",
	.id = -1,
};

struct nxp_snd_dai_plat_data snd_null_dai_data = {
	.i2s_ch = 0,
#if defined(CONFIG_SND_NXP_DFS)
	.sample_rate = SNDRV_PCM_RATE_8000_192000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
#else
	.sample_rate = 48000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
#endif
};

static struct platform_device snd_null_dai = {
	.name = "snd-null-card",
	.id = -1,
	.dev = {
		.platform_data = &snd_null_dai_data,
	}
};

//-------------------------------------
static struct platform_device snd_null_2 = {
	.name = "snd-null",
	.id = 1,
};

struct nxp_snd_dai_plat_data snd_null_dai_data_2 = {
	.i2s_ch = 2,
#if defined(CONFIG_SND_NXP_DFS)
	.sample_rate = SNDRV_PCM_RATE_8000_192000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
#else
	.sample_rate = 48000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
#endif
};

static struct platform_device snd_null_dai_2 = {
	.name = "snd-null-card",
	.id = 2,
	.dev = {
		.platform_data = &snd_null_dai_data_2 ,
	}
};
#endif

#if defined(CONFIG_SND_CODEC_RT5631) || defined(CONFIG_SND_CODEC_RT5631_MODULE)
#include <linux/i2c.h>

#define	RT5631_I2C_BUS		(0)

/* CODEC */
static struct i2c_board_info __initdata rt5631_i2c_bdi = {
	.type	= "rt5631",
	.addr	= (0x34>>1),		// 0x1A (7BIT), 0x34(8BIT)
};

/* DAI */
struct nxp_snd_dai_plat_data i2s_dai_data = {
	.i2s_ch	= 1,
#if defined(CONFIG_SND_NXP_DFS)
	.sample_rate = SNDRV_PCM_RATE_8000_192000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
#else
	.sample_rate	= 48000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
#endif
#if 0
	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_A + 0,
		.detect_level	= 1,
	},
#endif
};

static struct platform_device rt5631_dai = {
	.name			= "rt5631-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &i2s_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_PDM_REC) || defined(CONFIG_SND_PDM_REC_MODULE)
static struct platform_device pdm_recorder = {
	.name	= "pdm-dit-recorder",
	.id		= -1,
};

struct nxp_snd_dai_plat_data pdm_rec_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device pdm_rec_dai = {
	.name	= "pdm-recorder",
	.id		= -1,
	.dev	= {
		.platform_data	= &pdm_rec_dai_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 *  * reserve mem
 *   */
#ifdef CONFIG_CMA
#include <linux/cma.h>
extern void nxp_cma_region_reserve(struct cma_region *, const char *);

void __init nxp_reserve_mem(void)
{
    static struct cma_region regions[] = {
        {
            .name = "ion",
#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
            .size = CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K,
#else
			.size = 0,
#endif
            {
                .alignment = PAGE_SIZE,
            }
        },
        {
            .size = 0
        }
    };

    static const char map[] __initconst =
        "ion-nxp=ion;"
        "nx_vpu=ion;";

#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
    printk("%s: reserve CMA: size %d\n", __func__, CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K);
#endif
    nxp_cma_region_reserve(regions, map);
}
#endif

/*------------------------------------------------------------------------------
 * PMIC platform device
 */
#if defined(CONFIG_REGULATOR_NXE2000)

#include <linux/i2c.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/nxe2000.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/nxe2000-regulator.h>
#include <linux/power/nxe2000_battery.h>
//#include <linux/rtc/rtc-nxe2000.h>
//#include <linux/rtc.h>

#define NXE2000_I2C_BUS		(2)
#define NXE2000_I2C_ADDR	(0x64 >> 1)

/* NXE2000 IRQs */
#define NXE2000_IRQ_BASE	(IRQ_SYSTEM_END)
#define NXE2000_GPIO_BASE	(ARCH_NR_GPIOS) //PLATFORM_NXE2000_GPIO_BASE
#define NXE2000_GPIO_IRQ	(NXE2000_GPIO_BASE + 8)

//#define CONFIG_NXE2000_RTC

#ifndef CONFIG_REGULATOR_MP8845C
static struct regulator_consumer_supply nxe2000_dc1_supply_0[] = {
	REGULATOR_SUPPLY("vdd_arm_1.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_core_1.2V", NULL),
};
#endif
static struct regulator_consumer_supply nxe2000_dc3_supply_0[] = {
	REGULATOR_SUPPLY("vdd_sys_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr_1.6V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc5_supply_0[] = {
	REGULATOR_SUPPLY("vdd_sys_1.6V", NULL),
};

static struct regulator_consumer_supply nxe2000_ldo1_supply_0[] = {
	REGULATOR_SUPPLY("vgps_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo2_supply_0[] = {
	REGULATOR_SUPPLY("vcam1_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo3_supply_0[] = {
	REGULATOR_SUPPLY("vsys1_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vsys_1.9V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo5_supply_0[] = {
	REGULATOR_SUPPLY("vcam_2.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo6_supply_0[] = {
	REGULATOR_SUPPLY("valive_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo7_supply_0[] = {
	REGULATOR_SUPPLY("vvid_2.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo8_supply_0[] = {
	REGULATOR_SUPPLY("vwifi_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo9_supply_0[] = {
	REGULATOR_SUPPLY("vhub_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo10_supply_0[] = {
	REGULATOR_SUPPLY("vhsic_1.2V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldortc1_supply_0[] = {
	REGULATOR_SUPPLY("valive_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldortc2_supply_0[] = {
	REGULATOR_SUPPLY("valive_1.0V", NULL),
};


#define NXE2000_PDATA_INIT(_name, _sname, _minuv, _maxuv, _always_on, _boot_on, \
	_init_uv, _init_enable, _slp_slots) \
	static struct nxe2000_regulator_platform_data pdata_##_name##_##_sname = \
	{	\
		.regulator = {	\
			.constraints = {	\
				.min_uV		= _minuv,	\
				.max_uV		= _maxuv,	\
				.valid_modes_mask	= (REGULATOR_MODE_NORMAL |	\
									REGULATOR_MODE_STANDBY),	\
				.valid_ops_mask		= (REGULATOR_CHANGE_MODE |	\
									REGULATOR_CHANGE_STATUS |	\
									REGULATOR_CHANGE_VOLTAGE),	\
				.always_on	= _always_on,	\
				.boot_on	= _boot_on,		\
				.apply_uV	= 1,			\
			},	\
			.num_consumer_supplies =		\
				ARRAY_SIZE(nxe2000_##_name##_supply_##_sname),	\
			.consumer_supplies	= nxe2000_##_name##_supply_##_sname, \
			.supply_regulator	= 0,	\
		},	\
		.init_uV		= _init_uv,		\
		.init_enable	= _init_enable,	\
		.sleep_slots	= _slp_slots,	\
	}
/* min_uV/max_uV : Please set the appropriate value for the devices that the power supplied within a*/
/*                 range from min to max voltage according to NXE2000 specification. */
#ifndef CONFIG_REGULATOR_MP8845C
NXE2000_PDATA_INIT(dc1,      0,  950000, 2000000, 1, 1, 1100000, 1,  4);	/* 1.1V ARM */
NXE2000_PDATA_INIT(dc2,      0, 1000000, 2000000, 1, 1, 1100000, 1,  4);	/* 1.0V CORE */
#endif
NXE2000_PDATA_INIT(dc3,      0, 1000000, 3500000, 1, 1, 3300000, 1,  0);	/* 3.3V SYS */
NXE2000_PDATA_INIT(dc4,      0, 1000000, 2000000, 1, 1, 1500000, 1, -1);	/* 1.5V DDR */
NXE2000_PDATA_INIT(dc5,      0, 1000000, 2000000, 1, 1, 1500000, 1,  4);	/* 1.5V SYS */

NXE2000_PDATA_INIT(ldo1,     0, 1000000, 3500000, 1, 0, 3300000, 1,  0);	/* 3.3V GPS */
NXE2000_PDATA_INIT(ldo2,     0, 1000000, 3500000, 0, 0, 1800000, 0,  0);	/* 1.8V CAM1 */
NXE2000_PDATA_INIT(ldo3,     0, 1000000, 3500000, 1, 0, 1800000, 1,  2);	/* 1.8V SYS1 */
NXE2000_PDATA_INIT(ldo4,     0, 1000000, 3500000, 1, 0, 1800000, 1,  2);	/* 1.8V SYS */
NXE2000_PDATA_INIT(ldo5,     0, 1000000, 3500000, 0, 0, 2800000, 0,  0);	/* 2.8V VCAM */
NXE2000_PDATA_INIT(ldo6,     0, 1000000, 3500000, 1, 0, 3300000, 1, -1);	/* 3.3V ALIVE */
NXE2000_PDATA_INIT(ldo7,     0, 1000000, 3500000, 1, 0, 2800000, 1,  1);	/* 2.8V VID */
#if defined(CONFIG_NXP_RFKILL)
NXE2000_PDATA_INIT(ldo8,     0, 1000000, 3500000, 0, 0, 3300000, 0,  0);	/* 3.3V WIFI */
#else
NXE2000_PDATA_INIT(ldo8,     0, 1000000, 3500000, 0, 0, 3300000, 1,  0);	/* 3.3V WIFI */
#endif
NXE2000_PDATA_INIT(ldo9,     0, 1000000, 3500000, 1, 0, 3300000, 1,  0);	/* 3.3V HUB */
NXE2000_PDATA_INIT(ldo10,    0, 1000000, 3500000, 1, 0, 1200000, 0,  0);	/* 1.2V HSIC */
NXE2000_PDATA_INIT(ldortc1,  0, 1700000, 3500000, 1, 0, 1800000, 1, -1);	/* 1.8V ALIVE */
NXE2000_PDATA_INIT(ldortc2,  0, 1000000, 3500000, 1, 0, 1000000, 1, -1);	/* 1.0V ALIVE */


/*-------- if nxe2000 RTC exists -----------*/
#ifdef CONFIG_NXE2000_RTC
static struct nxe2000_rtc_platform_data rtc_data = {
	.irq	= NXE2000_IRQ_BASE,
	.time	= {
		.tm_year	= 1970,
		.tm_mon		= 0,
		.tm_mday	= 1,
		.tm_hour	= 0,
		.tm_min		= 0,
		.tm_sec		= 0,
	},
};

#define NXE2000_RTC_REG	\
{	\
	.id		= 0,	\
	.name	= "rtc_nxe2000",	\
	.platform_data	= &rtc_data,	\
}
#endif
/*-------- if Nexell RTC exists -----------*/

#define NXE2000_REG(_id, _name, _sname)	\
{	\
	.id		= NXE2000_ID_##_id,	\
	.name	= "nxe2000-regulator",	\
	.platform_data	= &pdata_##_name##_##_sname,	\
}

#define NXE2000_BATTERY_REG	\
{	\
    .id		= -1,	\
    .name	= "nxe2000-battery",	\
    .platform_data	= &nxe2000_battery_data,	\
}

//==========================================
//NXE2000 Power_Key device data
//==========================================
static struct nxe2000_pwrkey_platform_data nxe2000_pwrkey_data = {
	.irq 		= NXE2000_IRQ_BASE,
	.delay_ms 	= 20,
};
#define NXE2000_PWRKEY_REG		\
{	\
	.id 	= -1,	\
	.name 	= "nxe2000-pwrkey",	\
	.platform_data 	= &nxe2000_pwrkey_data,	\
}


static struct nxe2000_battery_platform_data nxe2000_battery_data = {
	.irq 				= NXE2000_IRQ_BASE,

//	.input_power_type	= INPUT_POWER_TYPE_ADP_UBC_LINKED,
	.input_power_type	= INPUT_POWER_TYPE_UBC,

	.gpio_otg_usbid		= CFG_GPIO_OTG_USBID_DET,
	.gpio_otg_vbus		= CFG_GPIO_OTG_VBUS_DET,
	.gpio_pmic_vbus		= CFG_GPIO_PMIC_VUSB_DET,
	.gpio_pmic_lowbat	= CFG_GPIO_PMIC_LOWBAT_DET,

	.low_vbat_vol_mv	= 3600,
	.low_vsys_vol_mv	= 3600,
	.bat_impe			= 1500,
	.slp_ibat			= 10,
//	.adc_channel		= NXE2000_ADC_CHANNEL_VBAT,
	.multiple			= 100,	//100%
	.monitor_time		= 60,
		/* some parameter is depend of battery type */
	.type[0] = {
		.ch_vfchg		= 0x03,	/* VFCHG	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.ch_vrchg		= 0x03,	/* VRCHG	= 0 - 4 (3.85v, 3.90v, 3.95v, 4.00v, 4.10v) */
		.ch_vbatovset	= 0xFF,	/* VBATOVSET	= 0 or 1 (0 : 4.38v(up)/3.95v(down) 1: 4.53v(up)/4.10v(down)) */
		.ch_ichg 		= 0x0E,	/* ICHG		= 0 - 0x1D (100mA - 3000mA) */
		.ch_ichg_slp	= 0x0E,	/* SLEEP  ICHG	= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_adp 	= 0x18,	/* ILIM_ADP	= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_usb 	= 0x04,	/* ILIM_USB	= 0 - 0x1D (100mA - 3000mA) */
		.ch_icchg		= 0x00,	/* ICCHG	= 0 - 3 (50mA 100mA 150mA 200mA) */
		.fg_target_vsys	= 3250,	/* This value is the target one to DSOC=0% */
		.fg_target_ibat	= 1000,	/* This value is the target one to DSOC=0% */
		.fg_poff_vbat	= 3350,	/* setting value of 0 per Vbat */
		.jt_en			= 0,	/* JEITA Enable	  = 0 or 1 (1:enable, 0:disable) */
		.jt_hw_sw		= 1,	/* JEITA HW or SW = 0 or 1 (1:HardWare, 0:SoftWare) */
		.jt_temp_h		= 50,	/* degree C */
		.jt_temp_l		= 12,	/* degree C */
		.jt_vfchg_h 	= 0x03,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_vfchg_l 	= 0,	/* VFCHG Low  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_ichg_h		= 0x07,	/* ICHG High  	= 0 - 0x1D (100mA - 3000mA) */
		.jt_ichg_l		= 0x04,	/* ICHG Low   	= 0 - 0x1D (100mA - 3000mA) */
	},
	/*
	.type[1] = {
		.ch_vfchg		= 0x0,
		.ch_vrchg		= 0x0,
		.ch_vbatovset	= 0x0,
		.ch_ichg		= 0x0,
		.ch_ilim_adp	= 0x0,
		.ch_ilim_usb	= 0x0,
		.ch_icchg		= 0x00,
		.fg_target_vsys	= 3300,//3000,
		.fg_target_ibat	= 1000,//1000,
		.jt_en			= 0,
		.jt_hw_sw		= 1,
		.jt_temp_h		= 40,
		.jt_temp_l		= 10,
		.jt_vfchg_h		= 0x0,
		.jt_vfchg_l		= 0,
		.jt_ichg_h		= 0x01,
		.jt_ichg_l		= 0x01,
	},
	*/

/*  JEITA Parameter
*
*          VCHG
*            |
* jt_vfchg_h~+~~~~~~~~~~~~~~~~~~~+
*            |                   |
* jt_vfchg_l-| - - - - - - - - - +~~~~~~~~~~+
*            |    Charge area    +          |
*  -------0--+-------------------+----------+--- Temp
*            !                   +
*          ICHG
*            |                   +
*  jt_ichg_h-+ - -+~~~~~~~~~~~~~~+~~~~~~~~~~+
*            +    |              +          |
*  jt_ichg_l-+~~~~+   Charge area           |
*            |    +              +          |
*         0--+----+--------------+----------+--- Temp
*            0   jt_temp_l      jt_temp_h   55
*/
};


#ifndef CONFIG_REGULATOR_MP8845C
#define NXE2000_DEV_REG 		\
	NXE2000_REG(DC1, dc1, 0),	\
	NXE2000_REG(DC2, dc2, 0),	\
	NXE2000_REG(DC3, dc3, 0),	\
	NXE2000_REG(DC4, dc4, 0),	\
	NXE2000_REG(DC5, dc5, 0),	\
	NXE2000_REG(LDO1, ldo1, 0),	\
	NXE2000_REG(LDO2, ldo2, 0),	\
	NXE2000_REG(LDO3, ldo3, 0),	\
	NXE2000_REG(LDO4, ldo4, 0),	\
	NXE2000_REG(LDO5, ldo5, 0),	\
	NXE2000_REG(LDO6, ldo6, 0),	\
	NXE2000_REG(LDO7, ldo7, 0),	\
	NXE2000_REG(LDO8, ldo8, 0),	\
	NXE2000_REG(LDO9, ldo9, 0),	\
	NXE2000_REG(LDO10, ldo10, 0),	\
	NXE2000_REG(LDORTC1, ldortc1, 0),	\
	NXE2000_REG(LDORTC2, ldortc2, 0)
#else
#define NXE2000_DEV_REG 		\
	NXE2000_REG(DC3, dc3, 0),	\
	NXE2000_REG(DC4, dc4, 0),	\
	NXE2000_REG(DC5, dc5, 0),	\
	NXE2000_REG(LDO1, ldo1, 0),	\
	NXE2000_REG(LDO2, ldo2, 0),	\
	NXE2000_REG(LDO3, ldo3, 0),	\
	NXE2000_REG(LDO4, ldo4, 0),	\
	NXE2000_REG(LDO5, ldo5, 0),	\
	NXE2000_REG(LDO6, ldo6, 0),	\
	NXE2000_REG(LDO7, ldo7, 0),	\
	NXE2000_REG(LDO8, ldo8, 0),	\
	NXE2000_REG(LDO9, ldo9, 0),	\
	NXE2000_REG(LDO10, ldo10, 0),	\
	NXE2000_REG(LDORTC1, ldortc1, 0),	\
	NXE2000_REG(LDORTC2, ldortc2, 0)
#endif

static struct nxe2000_subdev_info nxe2000_devs_dcdc[] = {
	NXE2000_DEV_REG,
	NXE2000_BATTERY_REG,
	NXE2000_PWRKEY_REG,
#ifdef CONFIG_NXE2000_RTC
	NXE2000_RTC_REG,
#endif
};


#define NXE2000_GPIO_INIT(_init_apply, _output_mode, _output_val, _led_mode, _led_func) \
	{									\
		.output_mode_en = _output_mode,	\
		.output_val		= _output_val,	\
		.init_apply		= _init_apply,	\
		.led_mode		= _led_mode,	\
		.led_func		= _led_func,	\
	}
struct nxe2000_gpio_init_data nxe2000_gpio_data[] = {
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
};

static struct nxe2000_platform_data nxe2000_platform = {
	.num_subdevs		= ARRAY_SIZE(nxe2000_devs_dcdc),
	.subdevs			= nxe2000_devs_dcdc,
	.irq_base			= NXE2000_IRQ_BASE,
	.irq_type			= IRQ_TYPE_EDGE_FALLING,
	.gpio_base			= NXE2000_GPIO_BASE,
	.gpio_init_data		= nxe2000_gpio_data,
	.num_gpioinit_data	= ARRAY_SIZE(nxe2000_gpio_data),
	.enable_shutdown_pin	= true,
};

static struct i2c_board_info __initdata nxe2000_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("nxe2000", NXE2000_I2C_ADDR),
		.irq			= CFG_GPIO_PMIC_INTR,
		.platform_data	= &nxe2000_platform,
	},
};
#endif  /* CONFIG_REGULATOR_NXE2000 */

#if defined(CONFIG_REGULATOR_MP8845C)
#include <linux/i2c.h>
#include <linux/regulator/machine.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/mp8845c-regulator.h>

#define MP8845C_PDATA_INIT(_name, _sname, _minuv, _maxuv, _always_on, _boot_on, _init_uv, _init_enable, _slp_slots) \
	static struct mp8845c_regulator_platform_data pdata_##_name##_##_sname = \
	{									\
		.regulator = {					\
			.constraints = {			\
				.min_uV		= _minuv,	\
				.max_uV		= _maxuv,	\
				.valid_modes_mask	= (REGULATOR_MODE_NORMAL |	\
									REGULATOR_MODE_STANDBY),	\
				.valid_ops_mask		= (REGULATOR_CHANGE_MODE |	\
									REGULATOR_CHANGE_STATUS |	\
									REGULATOR_CHANGE_VOLTAGE),	\
				.always_on	= _always_on,	\
				.boot_on	= _boot_on,		\
				.apply_uV	= 1,				\
			},								\
			.num_consumer_supplies =		\
				ARRAY_SIZE(mp8845c_##_name##_##_sname),			\
			.consumer_supplies	= mp8845c_##_name##_##_sname, 	\
			.supply_regulator	= 0,			\
		},									\
		.init_uV		= _init_uv,			\
		.init_enable	= _init_enable,		\
		.sleep_slots	= _slp_slots,		\
	}

#define MP8845C_REGULATOR(_dev_id, _name, _sname)	\
{												\
	.id		= MP8845C_##_dev_id##_VOUT,				\
	.name	= "mp8845c-regulator",				\
	.platform_data	= &pdata_##_name##_##_sname,\
}

#define I2C_FLATFORM_INFO(dev_type, dev_addr, dev_data)\
{										\
	.type = dev_type, 					\
	.addr = (dev_addr),					\
	.platform_data = dev_data,			\
}

static struct regulator_consumer_supply mp8845c_vout_0[] = {
	REGULATOR_SUPPLY("vdd_arm_1.3V", NULL),
};

static struct regulator_consumer_supply mp8845c_vout_1[] = {
	REGULATOR_SUPPLY("vdd_core_1.2V", NULL),
};

MP8845C_PDATA_INIT(vout, 0, 600000, 1500000, 1, 1, 1100000, 1, -1);	/* ARM */
MP8845C_PDATA_INIT(vout, 1, 600000, 1500000, 1, 1, 1100000, 1, -1);	/* CORE */

static struct mp8845c_platform_data __initdata mp8845c_platform[] = {
	MP8845C_REGULATOR(0, vout, 0),
	MP8845C_REGULATOR(1, vout, 1),
};

#define MP8845C_I2C_BUS0		(0)
#define MP8845C_I2C_BUS1		(2)
#define MP8845C_I2C_ADDR		(0x1c)

static struct i2c_board_info __initdata mp8845c_regulators[] = {
	I2C_FLATFORM_INFO("mp8845c", MP8845C_I2C_ADDR, &mp8845c_platform[0]),
	I2C_FLATFORM_INFO("mp8845c", MP8845C_I2C_ADDR, &mp8845c_platform[1]),
};
#endif  /* CONFIG_REGULATOR_MP8845C */


/*------------------------------------------------------------------------------
 * MPEGTS platform device
 */
#if defined(CONFIG_NXP_MP2TS_IF)
#include <mach/nxp_mp2ts.h>

#define NXP_TS_PAGE_NUM_0       (36)	// Variable
#define NXP_TS_BUF_SIZE_0       (TS_PAGE_SIZE * NXP_TS_PAGE_NUM_0)

#define NXP_TS_PAGE_NUM_1       (36)	// Variable
#define NXP_TS_BUF_SIZE_1       (TS_PAGE_SIZE * NXP_TS_PAGE_NUM_1)

#define NXP_TS_PAGE_NUM_CORE    (36)	// Variable
#define NXP_TS_BUF_SIZE_CORE    (TS_PAGE_SIZE * NXP_TS_PAGE_NUM_CORE)


static struct nxp_mp2ts_dev_info mp2ts_dev_info[2] = {
    {
        .demod_irq_num = CFG_GPIO_DEMOD_0_IRQ_NUM,
        .demod_rst_num = CFG_GPIO_DEMOD_0_RST_NUM,
        .tuner_rst_num = CFG_GPIO_TUNER_0_RST_NUM,
    },
    {
        .demod_irq_num = CFG_GPIO_DEMOD_1_IRQ_NUM,
        .demod_rst_num = CFG_GPIO_DEMOD_1_RST_NUM,
        .tuner_rst_num = CFG_GPIO_TUNER_1_RST_NUM,
    },
};

static struct nxp_mp2ts_plat_data mpegts_plat_data = {
    .dev_info       = mp2ts_dev_info,
    .ts_dma_size[0] = -1,                   // TS ch 0 - Static alloc size.
    .ts_dma_size[1] = NXP_TS_BUF_SIZE_1,    // TS ch 1 - Static alloc size.
    .ts_dma_size[2] = -1,                   // TS core - Static alloc size.
};

static struct platform_device mpegts_plat_device = {
    .name	= DEV_NAME_MPEGTSI,
    .id		= 0,
    .dev	= {
        .platform_data = &mpegts_plat_data,
    },
};
#endif  /* CONFIG_NXP_MP2TS_IF */

/*------------------------------------------------------------------------------
 * v4l2 platform device
 */
#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>

static int front_camera_set_clock(ulong clk_rate)
{
  	printk(KERN_INFO "%s: %d\n", __func__, (int)clk_rate);

    if (clk_rate > 0)
        nxp_soc_pwm_set_frequency(1, clk_rate, 50);
    else
        nxp_soc_pwm_set_frequency(1, 0, 0);
    msleep(1);
    return 0;
}

static int back_camera_set_clock(ulong clk_rate)
{

#if 0 //KEUN when OSC, doesn't stop front camera
  	printk(KERN_INFO "%s: %d\n", __func__, (int)clk_rate);
    if (clk_rate > 0)
        nxp_soc_pwm_set_frequency(3, clk_rate, 50);
    else
        nxp_soc_pwm_set_frequency(3, 0, 0);
    msleep(1);
#endif

    return 0;
}

static void back_vin_setup_io(int module, bool force)
{
    /* do nothing */
}

static bool is_camera_port_configured = false;
static void front_camera_vin_setup_io(int module, bool force)
{
  	printk(KERN_INFO "%s: module -> %d, force -> %d\n", __func__, module, ((force == true) ? 1 : 0));

    if (!force && is_camera_port_configured)
        return;
    else {
        u_int *pad;
        int i, len;
        u_int io, fn;


        /* VIP0:0 = VCLK, VID0 ~ 7 */
        const u_int port[][2] = {
#if 1 //vid 0
            /* VCLK, HSYNC, VSYNC */
            { PAD_GPIO_E +  4, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  5, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  6, NX_GPIO_PADFUNC_1 },
            /* DATA */
            { PAD_GPIO_D + 28, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 29, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_D + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 31, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  0, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  1, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  3, NX_GPIO_PADFUNC_1 },
#endif

#if 0 //vid1
             /* VCLK, HSYNC, VSYNC */
            { PAD_GPIO_A + 28, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E + 13, NX_GPIO_PADFUNC_2 },
            { PAD_GPIO_E +  7, NX_GPIO_PADFUNC_2 },

            { PAD_GPIO_A + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  0, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_B +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  4, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_B +  6, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  8, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_B +  9, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B + 10, NX_GPIO_PADFUNC_1 },
#endif

#if 0 //vid 2
            /* VCLK, HSYNC, VSYNC */
            { PAD_GPIO_C + 14, NX_GPIO_PADFUNC_3 },
            { PAD_GPIO_C + 15, NX_GPIO_PADFUNC_3 },
            { PAD_GPIO_C + 16, NX_GPIO_PADFUNC_3 },
            /* DATA */
            { PAD_GPIO_C + 17, NX_GPIO_PADFUNC_3 }, { PAD_GPIO_C + 18, NX_GPIO_PADFUNC_3 },
            { PAD_GPIO_C + 19, NX_GPIO_PADFUNC_3 }, { PAD_GPIO_C + 20, NX_GPIO_PADFUNC_3 },
            { PAD_GPIO_C + 21, NX_GPIO_PADFUNC_3 }, { PAD_GPIO_C + 22, NX_GPIO_PADFUNC_3 },
            { PAD_GPIO_C + 23, NX_GPIO_PADFUNC_3 }, { PAD_GPIO_C + 24, NX_GPIO_PADFUNC_3 },
#endif

        };

        printk("%s\n", __func__);

        pad = (u_int *)port;
        len = sizeof(port)/sizeof(port[0]);

        for (i = 0; i < len; i++) {
            io = *pad++;
            fn = *pad++;
            nxp_soc_gpio_set_io_dir(io, 0);
            nxp_soc_gpio_set_io_func(io, fn);
        }

        is_camera_port_configured = true;
    }
}

static int back_camera_power_enable(bool on)
{
  unsigned int reset_io = CFG_IO_CAMERA_BACK_RESET;
  printk(KERN_INFO "%s: on %d\n", __func__, on);

	if(on)
	{
		/* RST signal */
		nxp_soc_gpio_set_out_value(reset_io, 0);
		nxp_soc_gpio_set_io_dir(reset_io, 1);
		nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(reset_io));
		mdelay(1);

		nxp_soc_gpio_set_out_value(reset_io, 1);
		mdelay(1);

		nxp_soc_gpio_set_out_value(reset_io, 0);
		mdelay(1);

		nxp_soc_gpio_set_out_value(reset_io, 1);
		mdelay(10);
	}
	else
	{
		nxp_soc_gpio_set_out_value(reset_io, 0);
		mdelay(1);
	}

	return 0;
}

static struct i2c_board_info back_camera_i2c_boardinfo[] = {
    {
				I2C_BOARD_INFO("THP7212NX", 0xC0>>1),
    },
};

static int mipi_phy_enable(bool en)
{
    return 0;
}

struct nxp_mipi_csi_platformdata thp7212_plat_data = {
    .module     = 0,
    .clk_rate   = 27000000, // 27MHz
    .lanes      = 4,
    .alignment = 0,
    .hs_settle  = 0,
    .width      = 1920,
    .height     = 1080,
    .fixed_phy_vdd = false,
    .irq        = 0, /* not used */
    .base       = 0, /* not used */
    .phy_enable = mipi_phy_enable
};

static int front_camera_power_enable(bool on)
{
  unsigned int reset_io = CFG_IO_CAMERA_FRONT_RESET;
  printk(KERN_INFO "%s: on %d\n", __func__, on);

	if (on)
  {
    nxp_soc_gpio_set_io_dir(reset_io, 0);
    nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(reset_io));

    nxp_soc_gpio_set_out_value(reset_io, 1);
    mdelay(10);

    nxp_soc_gpio_set_out_value(reset_io, 0);
    mdelay(10);

    nxp_soc_gpio_set_out_value(reset_io, 1);
    mdelay(1);
  }
  else
  {
    nxp_soc_gpio_set_out_value(reset_io, 0);
    mdelay(1);
  }

	return 0;
}

static struct i2c_board_info front_camera_i2c_boardinfo[] = {
    {
				I2C_BOARD_INFO("MT9D111", 0xBA>>1),
    },
};

static struct nxp_v4l2_i2c_board_info sensor[] = {
 		{
        .board_info = &back_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 1,
    },
		{
        .board_info = &front_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 2,
    },
};

static struct nxp_capture_platformdata capture_plat_data[] = {
#if defined(CONFIG_VIDEO_MT9D111_CAM)
	{
        /* front_camera 656 interface */
        .module = 2,
        .sensor = &sensor[1],
        .type = NXP_CAPTURE_INF_PARALLEL,
				.parallel = {
            .is_mipi        = false,
            .external_sync  = false, //if external_sync is used(this means that value is true), 601 format else 656 format
            .h_active       = 1280,
            .h_frontporch   = 7,
            .h_syncwidth    = 1,
            .h_backporch    = 10,
            .v_active       = 720,
            .v_frontporch   = 0,
            .v_syncwidth    = 1,
            .v_backporch    = 0,
            .clock_invert   = true,
            .port           = 0,
            .data_order     = NXP_VIN_CBY0CRY1,
            .interlace      = false,
            .clk_rate       = 24000000,
            .late_power_down = false,
            .power_enable   = front_camera_power_enable,
            //.set_clock      = front_camera_set_clock,
            .set_clock      = NULL,
            .setup_io       = front_camera_vin_setup_io,
        },
				.deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
	},
#endif
#if defined(CONFIG_NXP_CAPTURE_MIPI_CSI)
#if defined(CONFIG_VIDEO_THP7212_CAM)
  {
        /* back_camera 656 interface */
        .module = 0,
        .sensor = &sensor[0],
				.type = NXP_CAPTURE_INF_CSI,
				.parallel = {
            .is_mipi        = true,
            .external_sync  = true,
            .h_active       = 1920,
            .h_frontporch   = 4,
            .h_syncwidth    = 4,
            .h_backporch    = 4,
            .v_active       = 1080,
            .v_frontporch   = 1,
            .v_syncwidth    = 1,
            .v_backporch    = 1,
            .clock_invert   = false,
            .port           = 1,
            .data_order     = NXP_VIN_CBY0CRY1,
            .interlace      = false,
            .clk_rate       = 27000000,
            .late_power_down = false,
            .power_enable   = back_camera_power_enable,
            .set_clock      = back_camera_set_clock,
            .setup_io       = back_vin_setup_io,
        },
        .deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
        .csi = &thp7212_plat_data,
	},
#endif
#endif
	{ 0, NULL, 0, },
};


/* out platformdata */
static struct i2c_board_info hdmi_edid_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_edid", 0xA0>>1),
};

static struct nxp_v4l2_i2c_board_info edid = {
    .board_info = &hdmi_edid_i2c_boardinfo,
    .i2c_adapter_id = 1,
};

static struct i2c_board_info hdmi_hdcp_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_hdcp", 0x74>>1),
};

static struct nxp_v4l2_i2c_board_info hdcp = {
    .board_info = &hdmi_hdcp_i2c_boardinfo,
    .i2c_adapter_id = 1,
};

static void hdmi_set_int_external(int gpio)
{
    nxp_soc_gpio_set_int_enable(gpio, 0);
    nxp_soc_gpio_set_int_mode(gpio, 1); /* high level */
    nxp_soc_gpio_set_int_enable(gpio, 1);
    nxp_soc_gpio_clr_int_pend(gpio);
}

static void hdmi_set_int_internal(int gpio)
{
    nxp_soc_gpio_set_int_enable(gpio, 0);
    nxp_soc_gpio_set_int_mode(gpio, 0); /* low level */
    nxp_soc_gpio_set_int_enable(gpio, 1);
    nxp_soc_gpio_clr_int_pend(gpio);
}

static int hdmi_read_hpd_gpio(int gpio)
{
    return nxp_soc_gpio_get_in_value(gpio);
}

static struct nxp_out_platformdata out_plat_data = {
    .hdmi = {
        .internal_irq = 0,
        .external_irq = 0,//PAD_GPIO_A + 19,
        .set_int_external = hdmi_set_int_external,
        .set_int_internal = hdmi_set_int_internal,
        .read_hpd_gpio = hdmi_read_hpd_gpio,
        .edid = &edid,
        .hdcp = &hdcp,
    },
};

static struct nxp_v4l2_platformdata v4l2_plat_data = {
    .captures = &capture_plat_data[0],
    .out = &out_plat_data,
};

static struct platform_device nxp_v4l2_dev = {
    .name       = NXP_V4L2_DEV_NAME,
    .id         = 0,
    .dev        = {
        .platform_data = &v4l2_plat_data,
    },
};
#endif /* CONFIG_V4L2_NXP || CONFIG_V4L2_NXP_MODULE */


#if defined (CONFIG_INV_MPU_IIO) || defined (CONFIG_INV_MPU_IIO_MODULE)

#include <linux/mpu.h>
#include <linux/gpio.h>
#include <linux/akm8975.h>

#define MPUIRQ_GPIO         (PAD_GPIO_A + 20)
#define MPU_I2C_BUS 		(2)
static struct mpu_platform_data mpu_data = {
	.int_config = 0x00,
	.level_shifter = 0,
	.orientation = {
			1 ,0, 0,
			0 ,1 ,0 ,
			0 ,0, 1 },

	.sec_slave_type = SECONDARY_SLAVE_TYPE_COMPASS,
	.sec_slave_id = COMPASS_ID_AK8975,
	.secondary_i2c_addr = 0x0c,
	.secondary_orientation = {
				1, 0, 0,
			    0, 1, 0,
			    0, 0, -1 },
	.key = {0xdd, 0x16, 0xcd, 0x7, 0xd9, 0xba, 0x97, 0x37,
	        0xcd, 0xfe, 0x23, 0x90, 0xe1, 0x66, 0x2f, 0x32},
};
static struct i2c_board_info __initdata inv_mpu_i2c0_boardinfo[] =  {
	{
		I2C_BOARD_INFO("mpu9150", 0x68),
	    .irq            = PB_PIO_IRQ(MPUIRQ_GPIO),
	    .platform_data = &mpu_data,
     },
};


#endif
/*------------------------------------------------------------------------------
 * SSP/SPI
 */

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
#include <mach/slsi-spi.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
static struct s3c64xx_spi_csinfo spi0_csi[] = {
    [0] = {
        .line       = CFG_SPI0_CS,
        .set_level  = gpio_set_value,
        .fb_delay   = 0x2,
    },
};
struct spi_board_info spi0_board_info[] __initdata = {
    {
        .modalias       = "spidev",
        .platform_data  = NULL,
        .max_speed_hz   = 10 * 1000 * 1000,
        .bus_num        = 0,
        .chip_select    = 0,
        .mode           = SPI_MODE_0,
        .controller_data    = &spi0_csi[0],
    }
};
static struct s3c64xx_spi_csinfo spi1_csi[] = {
    [0] = {
        .line       = CFG_SPI1_CS,
        .set_level  = gpio_set_value,
        .fb_delay   = 0x2,
		.hierarchy = SSP_SLAVE,
	//	.hierarchy = SSP_MASTER,
    },
};
struct spi_board_info spi1_board_info[] __initdata = {
    {
        .modalias       = "spidev",
        .platform_data  = NULL,
        .max_speed_hz   = 10 * 1000 * 1000,
        .bus_num        = 1,
        .chip_select    = 0,
        .mode           = SPI_MODE_0,
        .controller_data    = &spi1_csi[0],
    }
};
static struct s3c64xx_spi_csinfo spi2_csi[] = {
    [0] = {
        .line       = CFG_SPI2_CS,
        .set_level  = gpio_set_value,
        .fb_delay   = 0x2,
		//.hierarchy = SSP_SLAVE,
		.hierarchy = SSP_MASTER,
    },
};
struct spi_board_info spi2_board_info[] __initdata = {
    {
        .modalias       = "spidev",
        .platform_data  = NULL,
        .max_speed_hz   = 10 * 1000 * 1000,
        .bus_num        = 2,
        .chip_select    = 0,
        .mode           = SPI_MODE_0,
        .controller_data    = &spi2_csi[0],
    }
};

#endif
/*------------------------------------------------------------------------------
 * DW MMC board config
 */
#if defined(CONFIG_MMC_DW)
static int _dwmci_ext_cd_init(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

static int _dwmci_ext_cd_cleanup(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

static int _dwmci_get_ro(u32 slot_id)
{
	return 0;
}

#ifdef CONFIG_MMC_NXP_CH0
static int _dwmci0_init(u32 slot_id, irq_handler_t handler, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	int io  = CFG_SDMMC0_DETECT_IO;
	int irq = IRQ_GPIO_START + io;
	int id  = 0, ret = 0;

	printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

	ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				DEV_NAME_SDHC "0", (void*)host->slot[slot_id]);
	if (0 > ret)
		pr_err("dw_mmc dw_mmc.%d: fail request interrupt %d ...\n", id, irq);
	return 0;
}

static int _dwmci_get_cd(u32 slot_id)
{
	return 0;
}
static int _dwmci0_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC0_DETECT_IO;
	return nxp_soc_gpio_get_in_value(io);
}

static struct dw_mci_board _dwmci0_data = {
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 80 * 1000 * 1000,
	.caps			= MMC_CAP_CMD23,
	.detect_delay_ms= 200,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
	.cd_type		= DW_MCI_CD_EXTERNAL,
	.init			= _dwmci0_init,
	.get_ro			= _dwmci_get_ro,
	.get_cd			= _dwmci0_get_cd,
	.ext_cd_init	= _dwmci_ext_cd_init,
	.ext_cd_cleanup	= _dwmci_ext_cd_cleanup,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH0_USE_DMA)
    .mode       	= DMA_MODE,
#else
    .mode      		= PIO_MODE,
#endif
};
#endif

#ifdef CONFIG_MMC_NXP_CH1
static struct dw_mci_board _dwmci1_data = {
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
					  DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 50 * 1000 * 1000,
	.caps			= MMC_CAP_CMD23,
	.desc_sz		= 4,
	.detect_delay_ms= 200,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(0),
	.get_ro         = _dwmci_get_ro,
	.get_cd			= _dwmci_get_cd,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH1_USE_DMA)
    .mode       	= DMA_MODE,
#else
    .mode       	= PIO_MODE,
#endif
};
#endif

#ifdef CONFIG_MMC_NXP_CH2
static struct dw_mci_board _dwmci2_data = {
    .quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
				  	  DW_MCI_QUIRK_HIGHSPEED |
				  	  DW_MMC_QUIRK_HW_RESET_PW |
				      DW_MCI_QUIRK_NO_DETECT_EBIT,
	.bus_hz			= 50 * 1000 * 1000,
	.caps			=  MMC_CAP_NONREMOVABLE |
			 	  	   MMC_CAP_CMD23,

	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),

	.get_ro         = _dwmci_get_ro,
	.get_cd			= _dwmci_get_cd,
	.desc_sz		= 4,
	.detect_delay_ms= 200,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH2_USE_DMA)
    .mode       	= DMA_MODE,
#else
    .mode       	= PIO_MODE,
#endif

};
/*
static struct dw_mci_board _dwmci2_data = {
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
					  DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 50 * 1000 * 1000,
	.caps			=	MMC_CAP_CMD23,
	.desc_sz		= 4,
	.detect_delay_ms= 200,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(0),
	.get_ro			= _dwmci_get_ro,
	.get_cd			= _dwmci_get_cd,
};
*/
#endif


#endif /* CONFIG_MMC_DW */

/*------------------------------------------------------------------------------
 * RFKILL driver
 */
#if defined(CONFIG_NXP_RFKILL)

struct rfkill_dev_data  rfkill_dev_data =
{
	.supply_name 	= "vwifi_3.3V",	// vwifi_3.3V, vgps_3.3V
	.module_name 	= "wlan",
	.initval		= RFKILL_INIT_SET | RFKILL_INIT_OFF,
    .delay_time_off	= 1000,
};

struct nxp_rfkill_plat_data rfkill_plat_data = {
	.name		= "WiFi-Rfkill",
	.type		= RFKILL_TYPE_WLAN,
	.rf_dev		= &rfkill_dev_data,
    .rf_dev_num	= 1,
};

static struct platform_device rfkill_device = {
	.name			= DEV_NAME_RFKILL,
	.dev			= {
		.platform_data	= &rfkill_plat_data,
	}
};
#endif	/* CONFIG_RFKILL_NXP */

/*------------------------------------------------------------------------------
 * USB HSIC power control.
 */
#define SOC_PA_EHCI		PHY_BASEADDR_EHCI
#define SOC_VA_EHCI		IO_ADDRESS(SOC_PA_EHCI)

int nxp_hsic_phy_pwr_on(struct platform_device *pdev, bool on)
{
	printk("++%s %d\n", __func__, on);

	if(on)
		writel(0x1 << 1, SOC_VA_EHCI + 0xB0);
	else
		writel(0x0 << 1, SOC_VA_EHCI + 0xB0);

	return 0;
}
EXPORT_SYMBOL(nxp_hsic_phy_pwr_on);

#ifdef CONFIG_BATTERY_NXE2000
/*------------------------------------------------------------------------------
 * USB OTGVBUS power control.
 */
void nxp_otgvbus_pwr_set(int enable)
{
	nxp_soc_gpio_set_out_value(CFG_GPIO_OTG_VBUS_DET, enable);
}
EXPORT_SYMBOL(nxp_otgvbus_pwr_set);
#endif
/*------------------------------------------------------------------------------
 * HDMI CEC driver
 */
#if defined(CONFIG_NXP_HDMI_CEC)
static struct platform_device hdmi_cec_device = {
	.name			= NXP_HDMI_CEC_DRV_NAME,
};
#endif /* CONFIG_NXP_HDMI_CEC */

/*------------------------------------------------------------------------------
 * register board platform devices
 */
void __init nxp_board_devs_register(void)
{
	printk("[Register board platform devices]\n");

#if defined(CONFIG_ARM_NXP_CPUFREQ)
	printk("plat: add dynamic frequency (pll.%d)\n", dfs_plat_data.pll_dev);
	platform_device_register(&dfs_plat_device);
#endif

#if defined(CONFIG_SENSORS_NXP_TMU)
	printk("plat: add device TMU\n");
	platform_device_register(&tmu_device);
#endif

#if defined (CONFIG_FB_NXP)
	printk("plat: add framebuffer\n");
	platform_add_devices(fb_devices, ARRAY_SIZE(fb_devices));
#endif

#if defined(CONFIG_MMC_DW)
	#ifdef CONFIG_MMC_NXP_CH0
	nxp_mmc_add_device(0, &_dwmci0_data);
	#endif
	#ifdef CONFIG_MMC_NXP_CH1
	nxp_mmc_add_device(1, &_dwmci1_data);
	#endif
	#ifdef CONFIG_MMC_NXP_CH2
	nxp_mmc_add_device(2, &_dwmci2_data);
	#endif
#endif

#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
	printk("plat: add device dm9000 net\n");
	platform_device_register(&dm9000_plat_device);
#endif

#if defined(CONFIG_BACKLIGHT_PWM)
	printk("plat: add backlight pwm device\n");
	platform_device_register(&bl_plat_device);
#endif

#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
	printk("plat: add touch(ftx0x) device\n");
	i2c_register_board_info(FT5X0X_I2C_BUS, &ft5x0x_i2c_bdi, 1);
#endif

#if defined(CONFIG_MTD_NAND_NXP) || defined(CONFIG_NXP_FTL)
	platform_device_register(&nand_plat_device);
#endif

#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)
	printk("plat: add device keypad\n");
	platform_device_register(&key_plat_device);
#endif

#if defined(CONFIG_ANDROID_VIBRATION)
	printk("plat: add android timed gpio\n");
	platform_device_register(&android_timed_gpios);
#endif

#if defined(CONFIG_REGULATOR_MP8845C)
	printk("plat: add device mp8845c ARM\n");
	i2c_register_board_info(MP8845C_I2C_BUS0, &mp8845c_regulators[0], 1);

	printk("plat: add device mp8845c CORE\n");
	i2c_register_board_info(MP8845C_I2C_BUS1, &mp8845c_regulators[1], 1);
#endif

#if defined(CONFIG_REGULATOR_NXE2000)
	printk("plat: add device nxe2000 pmic\n");
	i2c_register_board_info(NXE2000_I2C_BUS, nxe2000_i2c_boardinfo, ARRAY_SIZE(nxe2000_i2c_boardinfo));
#endif

#if defined(CONFIG_NXP_MP2TS_IF)
	printk("plat: add device misc mpegts\n");
	platform_device_register(&mpegts_plat_device);
#endif

#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
	printk("plat: add device spdif playback\n");
	platform_device_register(&spdif_transciever);
	platform_device_register(&spdif_trans_dai);
#endif

#if defined(CONFIG_SND_SPDIF_RECEIVER) || defined(CONFIG_SND_SPDIF_RECEIVER_MODULE)
	printk("plat: add device spdif capture\n");
	platform_device_register(&spdif_receiver);
	platform_device_register(&spdif_recev_dai);
#endif

#if defined(CONFIG_SND_CODEC_NULL)
	platform_device_register(&snd_null);
	platform_device_register(&snd_null_dai);
	platform_device_register(&snd_null_2);
	platform_device_register(&snd_null_dai_2);
#endif

#if defined(CONFIG_SND_CODEC_RT5631) || defined(CONFIG_SND_CODEC_RT5631_MODULE)
	printk("plat: add device asoc-rt5631\n");
	i2c_register_board_info(RT5631_I2C_BUS, &rt5631_i2c_bdi, 1);
	platform_device_register(&rt5631_dai);
#endif

#if defined(CONFIG_SND_PDM_REC) || defined(CONFIG_SND_PDM_REC_MODULE)
	printk("plat: add device pdm capture\n");
	platform_device_register(&pdm_recorder);
	platform_device_register(&pdm_rec_dai);
#endif

#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
    printk("plat: add device nxp-v4l2\n");
    platform_device_register(&nxp_v4l2_dev);
#endif

#if defined (CONFIG_INV_MPU_IIO) || defined (CONFIG_INV_MPU_IIO_MODULE)
	printk("plat: add mpu9150\n");
	i2c_register_board_info (MPU_I2C_BUS, inv_mpu_i2c0_boardinfo,1);
#endif

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
    spi_register_board_info(spi0_board_info, ARRAY_SIZE(spi0_board_info));
    spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));
    spi_register_board_info(spi2_board_info, ARRAY_SIZE(spi2_board_info));
    printk("plat: register spidev\n");
#endif

#if defined(CONFIG_RFKILL_NXP)
    printk("plat: add device rfkill\n");
    platform_device_register(&rfkill_device);
#endif

#if defined(CONFIG_NXPMAC_ETH)
    printk("plat: add device nxp-gmac\n");
    platform_device_register(&nxp_gmac_dev);
#endif

#if defined(CONFIG_PPM_NXP)
    printk("plat: add device ppm\n");
    platform_device_register(&ppm_device);
#endif

#if defined(CONFIG_NXP_HDMI_CEC)
    printk("plat: add device hdmi-cec\n");
    platform_device_register(&hdmi_cec_device);
#endif

#if 0//defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	back_camera_power_enable(0);
	front_camera_power_enable(0);
	camera_power_control(0);
#endif

	/* END */
	printk("\n");
}
