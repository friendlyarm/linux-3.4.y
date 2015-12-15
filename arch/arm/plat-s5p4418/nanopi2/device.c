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
#include <linux/i2c.h>
#include <linux/amba/pl022.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#if defined(CONFIG_NXP_HDMI_CEC)
#include <mach/nxp-hdmi-cec.h>
#endif

#include <nxp-fb.h>

/*------------------------------------------------------------------------------
 * BUS Configure
 */
#if (CFG_BUS_RECONFIG_ENB == 1)
#include <mach/s5p4418_bus.h>

const u8 g_DrexBRB_RD[2] = {
	0x1,            // Port0
	0xF             // Port1
};

const u8 g_DrexBRB_WR[2] = {
	0x1,            // Port0
	0xF             // Port1
};

const u16 g_DrexQoS[2] = {
	0x100,          // S0
	0xFFF           // S1, Default value
};

#if (CFG_BUS_RECONFIG_TOPBUSSI == 1)
const u8 g_TopBusSI[8] = {
	TOPBUS_SI_SLOT_DMAC0,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST0,
	TOPBUS_SI_SLOT_DMAC1,
	TOPBUS_SI_SLOT_SDMMC,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST1,
	TOPBUS_SI_SLOT_USBOTG
};
#endif

#if (CFG_BUS_RECONFIG_TOPBUSQOS == 1)
const u8 g_TopQoSSI[2] = {
	1,      // Tidemark
	(1<<TOPBUS_SI_SLOT_DMAC0) |     // Control
		(1<<TOPBUS_SI_SLOT_MP2TS) |
		(1<<TOPBUS_SI_SLOT_DMAC1) |
		(1<<TOPBUS_SI_SLOT_SDMMC) |
		(1<<TOPBUS_SI_SLOT_USBOTG) |
		(1<<TOPBUS_SI_SLOT_USBHOST0) |
		(1<<TOPBUS_SI_SLOT_USBHOST1)
};
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1)
const u8 g_BottomBusSI[8] = {
	BOTBUS_SI_SLOT_1ST_ARM,
	BOTBUS_SI_SLOT_MALI,
	BOTBUS_SI_SLOT_DEINTERLACE,
	BOTBUS_SI_SLOT_1ST_CODA,
	BOTBUS_SI_SLOT_2ND_ARM,
	BOTBUS_SI_SLOT_SCALER,
	BOTBUS_SI_SLOT_TOP,
	BOTBUS_SI_SLOT_2ND_CODA
};
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSQOS == 1)
const u8 g_BottomQoSSI[2] = {
	1,      // Tidemark
	(1<<BOTBUS_SI_SLOT_1ST_ARM) |   // Control
		(1<<BOTBUS_SI_SLOT_2ND_ARM) |
		(1<<BOTBUS_SI_SLOT_MALI) |
		(1<<BOTBUS_SI_SLOT_TOP) |
		(1<<BOTBUS_SI_SLOT_DEINTERLACE) |
		(1<<BOTBUS_SI_SLOT_1ST_CODA)
};
#endif

#if (CFG_BUS_RECONFIG_DISPBUSSI == 1)
const u8 g_DispBusSI[3] = {
	DISBUS_SI_SLOT_1ST_DISPLAY,
	DISBUS_SI_SLOT_2ND_DISPLAY,
	DISBUS_SI_SLOT_GMAC
};
#endif
#endif /* #if (CFG_BUS_RECONFIG_ENB == 1) */

/*------------------------------------------------------------------------------
 * CPU Frequence
 */
#if defined(CONFIG_ARM_NXP_CPUFREQ)
static unsigned long dfs_freq_table[][2] = {
	{ 1400000, },
	{ 1200000, },
	{ 1000000, },
	{  800000, },
	{  700000, },
	{  600000, },
	{  500000, },
	{  400000, },
};

struct nxp_cpufreq_plat_data dfs_plat_data = {
	.pll_dev		= CONFIG_NXP_CPUFREQ_PLLDEV,
	.supply_name	= "vdd_arm_1.3V",	// refer to CONFIG_REGULATOR_NXE2000
	.freq_table		= dfs_freq_table,
	.table_size		= ARRAY_SIZE(dfs_freq_table),
	.max_cpufreq	= 1400*1000,
	.max_retention	=   20*1000,
	.rest_cpufreq	=  400*1000,
	.rest_retention	=    1*1000,
};

static struct platform_device dfs_plat_device = {
	.name			= DEV_NAME_CPUFREQ,
	.dev			= {
		.platform_data	= &dfs_plat_data,
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
#endif /* CONFIG_DM9000 || CONFIG_DM9000_MODULE */

/*------------------------------------------------------------------------------
 * DW GMAC board config
 */
#if defined(CONFIG_NXPMAC_ETH)
#include <linux/phy.h>
#include <linux/nxpmac.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#if defined(CFG_NXPMAC_INIT_ENB)
static int nxpmac_init(struct platform_device *pdev)
{
	u32 addr;

	// Clock control
	NX_CLKGEN_Initialize();
	addr = NX_CLKGEN_GetPhysicalAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE);
	NX_CLKGEN_SetBaseAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE, (void*)IO_ADDRESS(addr) );

	NX_CLKGEN_SetClockSource(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 4);		// Sync mode for 100 & 10Base-T : External RX_clk
	NX_CLKGEN_SetClockDivisor(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 1);		// Sync mode for 100 & 10Base-T

	NX_CLKGEN_SetClockOutInv(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CFALSE);	// TX Clk invert off : 100 & 10Base-T

	NX_CLKGEN_SetClockDivisorEnable(CLOCKINDEX_OF_DWC_GMAC_MODULE, CTRUE);

	// Reset control
	NX_RSTCON_Initialize();
	addr = NX_RSTCON_GetPhysicalAddress();
	NX_RSTCON_SetBaseAddress( (void*)IO_ADDRESS(addr) );
	NX_RSTCON_SetnRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ENABLE);
	udelay(100);
	NX_RSTCON_SetnRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_DISABLE);
	udelay(100);
	NX_RSTCON_SetnRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ENABLE);
	udelay(100);

	gpio_request(CFG_ETHER_GMAC_PHY_RST_NUM,"Ethernet Rst pin");
	gpio_direction_output(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	udelay(100);
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0 );
	udelay(100);
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );

	gpio_free(CFG_ETHER_GMAC_PHY_RST_NUM);

	printk("NXP Ethernet MAC initialized.\n");
	return 0;
}
#endif

static int gmac_phy_reset(void *priv)
{
	// Set GPIO nReset
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	udelay(100);
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0 );
	udelay(100);
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	msleep( 30);

	return 0;
}

static struct stmmac_mdio_bus_data nxpmac0_mdio_bus = {
	.phy_reset	= gmac_phy_reset,
	.phy_mask	= 0,
	.probed_phy_irq	= CFG_ETHER_GMAC_PHY_IRQ_NUM,
};

static struct plat_stmmacenet_data nxpmac_plat_data = {
	.phy_addr	= 7,
	.clk_csr	= 0x4,				/* PCLK 150~250 Mhz */
	.speed		= SPEED_1000,
	.interface	= PHY_INTERFACE_MODE_RGMII,
	.autoneg	= AUTONEG_ENABLE,	/* or AUTONEG_DISABLE */
	.duplex		= DUPLEX_FULL,
	.pbl		= 16,				/* burst 16 */
	.has_gmac	= 1,				/* GMAC ethernet */
	.enh_desc	= 1,
	.mdio_bus_data	= &nxpmac0_mdio_bus,
#if defined(CFG_NXPMAC_INIT_ENB)
	.init		= &nxpmac_init,
#endif
};

/* DWC GMAC Controller registration */

static struct resource nxpmac_resource[] = {
	[0] = DEFINE_RES_MEM(PHY_BASEADDR_GMAC, SZ_8K),
	[1] = DEFINE_RES_IRQ_NAMED(IRQ_PHY_GMAC, "macirq"),
};

static u64 nxpmac_dmamask = DMA_BIT_MASK(32);

struct platform_device nxp_gmac_dev = {
	.name			= "stmmaceth",
	.id				= -1,
	.num_resources	= ARRAY_SIZE(nxpmac_resource),
	.resource		= nxpmac_resource,
	.dev			= {
		.dma_mask			= &nxpmac_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &nxpmac_plat_data,
	}
};
#endif /* CONFIG_NXPMAC_ETH */

/*------------------------------------------------------------------------------
 * DISPLAY (LVDS) / FB
 */
#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
static struct nxp_fb_plat_data fb0_plat_data = {
	.module			= CONFIG_FB0_NXP_DISPOUT,
	.layer			= CFG_DISP_PRI_SCREEN_LAYER,
	#ifdef CONFIG_FB_NXP_X8R8G8B8
	.format			= MLC_RGBFMT_X8R8G8B8,
	#else
	.format			= CFG_DISP_PRI_SCREEN_RGB_FORMAT,
	#endif
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
	.dev	= {
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

static void nxp_platform_fb_data(struct nxp_lcd *lcd)
{
#if defined (CONFIG_FB0_NXP)
	struct nxp_fb_plat_data *pdata = &fb0_plat_data;

	if (lcd) {
		pdata->x_resol = lcd->width;
		pdata->y_resol = lcd->height;

		pdata->lcd_with_mm = lcd->p_width;
		pdata->lcd_height_mm = lcd->p_height;
	}
#endif
}
#endif /* CONFIG_FB_NXP */

#if defined (CONFIG_NXP_DISPLAY_LCD)
static void nxp_platform_disp_init(struct nxp_lcd *lcd)
{
	struct disp_vsync_info vsync;
	struct nxp_lcd_timing *timing;
	u32 clk = 800000000;
	u32 div;

	if (lcd) {
		timing = &lcd->timing;

		vsync.h_active_len	= lcd->width;
		vsync.h_sync_width	= timing->h_sw;
		vsync.h_back_porch	= timing->h_bp;
		vsync.h_front_porch	= timing->h_fp;
		vsync.h_sync_invert	= !lcd->polarity.inv_hsync;

		vsync.v_active_len	= lcd->height;
		vsync.v_sync_width	= timing->v_sw;
		vsync.v_back_porch	= timing->v_bp;
		vsync.v_front_porch	= timing->v_fp;
		vsync.v_sync_invert	= !lcd->polarity.inv_vsync;

		/* calculates pixel clock */
		div  = timing->h_sw + timing->h_bp + timing->h_fp + lcd->width;
		div *= timing->v_sw + timing->v_bp + timing->v_fp + lcd->height;
		div *= lcd->freq ? : 60;
		do_div(clk, div);

		vsync.pixel_clock_hz= div;
		vsync.clk_src_lv0	= CFG_DISP_PRI_CLKGEN0_SOURCE;
		vsync.clk_div_lv0	= clk;
		vsync.clk_src_lv1	= CFG_DISP_PRI_CLKGEN1_SOURCE;
		vsync.clk_div_lv1	= CFG_DISP_PRI_CLKGEN1_DIV;
		vsync.clk_out_inv	= lcd->polarity.rise_vclk;

		if (lcd->gpio_init)
			lcd->gpio_init();

		nxp_platform_disp_device_data(DISP_DEVICE_LCD, &vsync, NULL, NULL);
	}
}
#endif

/*------------------------------------------------------------------------------
 * backlight : generic pwm device
 */
#if defined(CONFIG_BACKLIGHT_PWM)
#include <linux/pwm_backlight.h>

static struct platform_pwm_backlight_data bl_plat_data = {
	.pwm_id			= CFG_LCD_PRI_PWM_CH,
	.max_brightness	= 400,	/* 255 is 100%, set over 100% */
	.dft_brightness	= 128,	/* 50% */
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
	.input_polarity	= NX_PPM_INPUTPOL_INVERT,	/* or  NX_PPM_INPUTPOL_BYPASS */
};

static struct platform_device ppm_device = {
	.name			= DEV_NAME_PPM,
	.dev			= {
		.platform_data	= &ppm_plat_data,
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
};

static struct nxp_nand_plat_data nand_plat_data = {
	.parts		= nxp_nand_parts,
	.nr_parts	= ARRAY_SIZE(nxp_nand_parts),
	.chip_delay	= 10,
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
		.platform_data	= &nand_plat_data,
	},
};
#endif /* CONFIG_MTD_NAND_NXP */

/*------------------------------------------------------------------------------
 * Touch screen
 */
#if defined(CONFIG_TOUCHSCREEN_GSLX680)
#define	GSLX680_I2C_BUS		(1)

static struct i2c_board_info __initdata gslX680_i2c_bdi = {
	.type	= "gslX680",
	.addr	= (0x40),
	.irq	= PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),
};
#endif

#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
#include <linux/platform_data/ft5x0x_touch.h>

#define	FT5X0X_I2C_BUS		(2)

static struct ft5x0x_i2c_platform_data ft5x0x_pdata = {
	.gpio_irq		= PB_PIO_IRQ(CFG_IO_TOUCH_IRQ),
	.irq_cfg		= 0,
	.screen_max_x	= 1280,
	.screen_max_y	= 800,
	.pressure_max	= 255,
};

static struct i2c_board_info __initdata ft5x0x_i2c_bdi = {
	I2C_BOARD_INFO("ft5x0x_ts", (0x70 >> 1)),
	.platform_data = &ft5x0x_pdata,
};
#endif

#if defined(CONFIG_TOUCHSCREEN_1WIRE)
#include <linux/platform_data/touchscreen-one-wire.h>

static struct ts_onewire_platform_data onewire_ts_pdata = {
	.timer_irq	= IRQ_PHY_PWM_INT3,
	.pwm_id		= 3,
	.pwm_reg_tint = (volatile void *)(IO_ADDRESS(PHY_BASEADDR_PWM) + 0x44),
	.gpio		= (PAD_GPIO_C + 15),
};

static struct platform_device onewire_device = {
	.name		= "onewire_ts",
	.id			= -1,
	.dev		= {
		.platform_data	= &onewire_ts_pdata,
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
	.bt_repeat	= CFG_KEYPAD_REPEAT,
};

static struct platform_device key_plat_device = {
	.name	= DEV_NAME_KEYPAD,
	.id		= -1,
	.dev	= {
		.platform_data	= &key_plat_data
	},
};
#endif /* CONFIG_KEYBOARD_NXP_KEY || CONFIG_KEYBOARD_NXP_KEY_MODULE */

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

#if defined(CONFIG_SND_CODEC_ES8316) || defined(CONFIG_SND_CODEC_ES8316_MODULE)
#define	ES8316_I2C_BUS		(0)

/* CODEC */
static struct i2c_board_info __initdata es8316_i2c_bdi = {
	.type	= "es8316",
	.addr	= (0x22>>1),		// 0x11 (7BIT), 0x22(8BIT)
};

/* DAI */
struct nxp_snd_dai_plat_data i2s_dai_data = {
	.i2s_ch	= 0,
	.sample_rate	= 48000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
#if 1
	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_B + 27,
		.detect_level	= 1,
	},
#endif
};

static struct platform_device es8316_dai = {
	.name			= "es8316-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &i2s_dai_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * G-Sensor platform device
 */
#if defined(CONFIG_SENSORS_MMA865X) || defined(CONFIG_SENSORS_MMA865X_MODULE)
#define	MMA865X_I2C_BUS		(2)

/* CODEC */
static struct i2c_board_info __initdata mma865x_i2c_bdi = {
	.type	= "mma8653",
	.addr	= 0x1D//(0x3a),
};
#endif /* CONFIG_SENSORS_MMA865X */

#if defined(CONFIG_SENSORS_STK831X) || defined(CONFIG_SENSORS_STK831X_MODULE)
#define	STK831X_I2C_BUS		(2)

/* CODEC */
static struct i2c_board_info __initdata stk831x_i2c_bdi = {
#if   defined CONFIG_SENSORS_STK8312
	.type	= "stk8312",
	.addr	= (0x3d),
#elif defined CONFIG_SENSORS_STK8313
	.type	= "stk8313",
	.addr	= (0x22),
#endif
};
#endif /* CONFIG_SENSORS_STK831X */

/*------------------------------------------------------------------------------
 * reserve mem
 */
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
#endif /* CONFIG_CMA */

#if defined(CONFIG_I2C_NXP_PORT3)
#define I2CUDELAY(x)	(1000000/x)
/* gpio i2c 3 */
#define	I2C3_SCL	(PAD_GPIO_E + 30)
#define	I2C3_SDA	(PAD_GPIO_E + 31)

static struct i2c_gpio_platform_data nxp_i2c_gpio_port3 = {
	.sda_pin	= I2C3_SDA,
	.scl_pin	= I2C3_SCL,
	.udelay		= I2CUDELAY(CFG_I2C3_CLK),		/* Gpio_mode CLK Rate = 1/( udelay*2) * 1000000 */
	.timeout	= 10,
};

static struct platform_device i2c_device_ch3 = {
	.name	= "i2c-gpio",
	.id		= 3,
	.dev    = {
		.platform_data	= &nxp_i2c_gpio_port3,
	},
};

static struct platform_device *i2c_devices[] = {
	&i2c_device_ch3,
};
#endif /* CONFIG_I2C_NXP_PORT3 */

/*------------------------------------------------------------------------------
 * PMIC platform device
 */
#if defined(CONFIG_REGULATOR_FIXED_VOLTAGE)
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>

/* Dummy supplies */
static struct regulator_consumer_supply fixed_dummy_supplies[] = {
	REGULATOR_SUPPLY("vdd_arm_1.3V", NULL),
	REGULATOR_SUPPLY("vdd_ddr_1.6V", NULL),
	REGULATOR_SUPPLY("vcam1_1.8V",   NULL),
};

struct regulator_init_data fixed_dummy_initdata = {
	.consumer_supplies	= fixed_dummy_supplies,
	.num_consumer_supplies	= ARRAY_SIZE(fixed_dummy_supplies),
	.constraints	= {
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
	},
};

static struct fixed_voltage_config fixed_dummy_config = {
	.supply_name	= "fixed_vdds",
	.microvolts	= 1350000,
	.gpio		= -EINVAL,
	.init_data	= &fixed_dummy_initdata,
};

static struct platform_device fixed_supply_dummy_device = {
	.name		= "reg-fixed-voltage",
	.id		= 0,
	.dev		= {
		.platform_data	= &fixed_dummy_config,
	},
};
#endif /* CONFIG_REGULATOR_FIXED_VOLTAGE */

/*------------------------------------------------------------------------------
 * v4l2 platform device
 */
#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <mach/nxp-v4l2-platformdata.h>

static int camera_common_set_clock(ulong clk_rate)
{
#ifndef CONFIG_VIDEO_OV5640
	PM_DBGOUT("%s: %d\n", __func__, (int)clk_rate);
	if (clk_rate > 0)
		nxp_soc_pwm_set_frequency(1, clk_rate, 50);
	else
		nxp_soc_pwm_set_frequency(1, 0, 0);
	msleep(1);
#endif
	return 0;
}

#if defined(CONFIG_VIDEO_OV5640)
#define CAMERA_USE_VID0		1
#elif defined(CONFIG_VIDEO_SP2518)
#define CAMERA_USE_VID1		1
#endif

static bool is_camera_port_configured = false;
static void camera_common_vin_setup_io(int module, bool force)
{
	if (!force && is_camera_port_configured)
		return;
	else {
		u_int *pad;
		int i, len;
		u_int io, fn;

		const u_int port[][2] = {
#ifdef CAMERA_USE_VID0
			/* VIP1:0 = VCLK, VID0 ~ 7 */
			{ PAD_GPIO_E +  4, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_E +  5, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_E +  6, NX_GPIO_PADFUNC_1 },
			/* DATA */
			{ PAD_GPIO_D + 28, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 29, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_D + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 31, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_E +  0, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  1, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_E +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  3, NX_GPIO_PADFUNC_1 },
#endif
#ifdef CAMERA_USE_VID1
			/* VIP0:0 = VCLK, VID0 ~ 7 */
			{ PAD_GPIO_A + 28, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_E + 13, NX_GPIO_PADFUNC_2 },
			{ PAD_GPIO_E +  7, NX_GPIO_PADFUNC_2 },

			{ PAD_GPIO_A + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  0, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_B +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  4, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_B +  6, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  8, NX_GPIO_PADFUNC_1 },
			{ PAD_GPIO_B +  9, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B + 10, NX_GPIO_PADFUNC_1 },
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

static bool camera_power_enabled = false;
static void camera_power_control(int enable)
{
#ifndef CONFIG_VIDEO_OV5640
    struct regulator *cam_core_18V = NULL;

    if (enable && camera_power_enabled)
        return;
    if (!enable && !camera_power_enabled)
        return;

    cam_core_18V = regulator_get(NULL, "vcam1_1.8V");
    if (IS_ERR(cam_core_18V)) {
        printk(KERN_ERR "%s: failed to regulator_get() for vcam1_1.8V", __func__);
        return;
    }
    printk("%s: %d\n", __func__, enable);
    if (enable) {
        regulator_enable(cam_core_18V);
    } else {
        regulator_disable(cam_core_18V);
    }

    regulator_put(cam_core_18V);
#endif

    camera_power_enabled = enable ? true : false;
}

static bool is_back_camera_enabled = false;
static bool is_back_camera_power_state_changed = false;

static bool is_front_camera_enabled = false;
#if defined(CONFIG_VIDEO_SP0838)
static bool is_front_camera_power_state_changed = false;
static int front_camera_power_enable(bool on);
#endif

static int back_camera_power_enable(bool on)
{
	unsigned int io = CFG_IO_CAMERA_BACK_POWER_DOWN;
	unsigned int reset_io = CFG_IO_CAMERA_RESET;

	PM_DBGOUT("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);

	if (on) {
#if defined(CONFIG_VIDEO_SP0838)
		front_camera_power_enable(0);
#endif
		if (!is_back_camera_enabled) {
			camera_power_control(1);
			/* PD signal */
			nxp_soc_gpio_set_out_value(io, 0);
			nxp_soc_gpio_set_io_dir(io, 1);
			nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
			nxp_soc_gpio_set_out_value(io, 1);
			camera_common_set_clock(24000000);
			/* mdelay(10); */
			mdelay(1);
			nxp_soc_gpio_set_out_value(io, 0);
			/* RST signal */
			nxp_soc_gpio_set_out_value(reset_io, 1);
			nxp_soc_gpio_set_io_dir(reset_io, 1);
			nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
			nxp_soc_gpio_set_out_value(reset_io, 0);
			/* mdelay(100); */
			mdelay(1);
			nxp_soc_gpio_set_out_value(reset_io, 1);
			/* mdelay(100); */
			mdelay(1);
			is_back_camera_enabled = true;
			is_back_camera_power_state_changed = true;
		} else {
			is_back_camera_power_state_changed = false;
		}
	} else {
		if (is_back_camera_enabled) {
			nxp_soc_gpio_set_out_value(io, 1);
			nxp_soc_gpio_set_out_value(reset_io, 0);
			is_back_camera_enabled = false;
			is_back_camera_power_state_changed = true;
		} else {
			nxp_soc_gpio_set_out_value(io, 1);
			nxp_soc_gpio_set_io_dir(io, 1);
			nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
			nxp_soc_gpio_set_out_value(io, 1);
			is_back_camera_power_state_changed = false;
		}

		if (!(is_back_camera_enabled || is_front_camera_enabled)) {
			camera_power_control(0);
		}
	}

	return 0;
}

static bool back_camera_power_state_changed(void)
{
	return is_back_camera_power_state_changed;
}

static struct i2c_board_info back_camera_i2c_boardinfo[] = {
#if defined(CONFIG_VIDEO_OV5640)
	{
		I2C_BOARD_INFO("ov5640", (0x78 >> 1)),
	},
#elif defined(CONFIG_VIDEO_SP2518)
	{
		I2C_BOARD_INFO("SP2518", 0x60>>1),
	},
#endif
};

#if defined(CONFIG_VIDEO_SP0838)
static int front_camera_power_enable(bool on)
{
	unsigned int io = CFG_IO_CAMERA_FRONT_POWER_DOWN;
	unsigned int reset_io = CFG_IO_CAMERA_RESET;

	PM_DBGOUT("%s: is_front_camera_enabled %d, on %d\n", __func__, is_front_camera_enabled, on);
	if (on) {
		back_camera_power_enable(0);
		if (!is_front_camera_enabled) {
			camera_power_control(1);
			/* First RST signal to low */
			nxp_soc_gpio_set_out_value(reset_io, 1);
			nxp_soc_gpio_set_io_dir(reset_io, 1);
			nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
			nxp_soc_gpio_set_out_value(reset_io, 0);
			mdelay(1);

			/* PWDN signal High to Low */
			nxp_soc_gpio_set_out_value(io, 0);
			nxp_soc_gpio_set_io_dir(io, 1);
			nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
			nxp_soc_gpio_set_out_value(io, 1);
			camera_common_set_clock(24000000);
			mdelay(10);
			/* mdelay(1); */
			nxp_soc_gpio_set_out_value(io, 0);
			/* mdelay(10); */
			mdelay(10);

			/* RST signal  to High */
			nxp_soc_gpio_set_out_value(reset_io, 1);
			/* mdelay(100); */
			mdelay(5);

			is_front_camera_enabled = true;
			is_front_camera_power_state_changed = true;
		} else {
			is_front_camera_power_state_changed = false;
		}
	} else {
		if (is_front_camera_enabled) {
			nxp_soc_gpio_set_out_value(io, 1);
			is_front_camera_enabled = false;
			is_front_camera_power_state_changed = true;
		} else {
			nxp_soc_gpio_set_out_value(io, 1);
			is_front_camera_power_state_changed = false;
		}
		if (!(is_back_camera_enabled || is_front_camera_enabled)) {
			camera_power_control(0);
		}
	}

	return 0;
}

static bool front_camera_power_state_changed(void)
{
	return is_front_camera_power_state_changed;
}
#endif

static struct i2c_board_info front_camera_i2c_boardinfo[] = {
#if defined(CONFIG_VIDEO_SP0838)
	{
		I2C_BOARD_INFO("SP0838", 0x18),
	},
#endif
};

static struct nxp_v4l2_i2c_board_info sensor[] = {
	{
		.board_info = &back_camera_i2c_boardinfo[0],
		.i2c_adapter_id = 0,
	},
	{
		.board_info = &front_camera_i2c_boardinfo[0],
		.i2c_adapter_id = 0,
	},
};

static struct nxp_capture_platformdata capture_plat_data[] = {
#if defined(CONFIG_VIDEO_OV5640)
	{
		/* back_camera 656 interface */
		.module = 1,
		.sensor = &sensor[0],
		.type = NXP_CAPTURE_INF_PARALLEL,
		.parallel = {
			/* for 656 */
			.is_mipi        = false,
			.external_sync  = false, /* 656 interface */
			.h_active       = 640,
			.h_frontporch   = 0,
			.h_syncwidth    = 0,
			.h_backporch    = 2,
			.v_active       = 480,
			.v_frontporch   = 0,
			.v_syncwidth    = 0,
			.v_backporch    = 13,
			.clock_invert   = false,
			.port           = 0,
			.data_order     = NXP_VIN_CBY0CRY1,
			.interlace      = false,
			.clk_rate       = 24000000,
			.late_power_down = true,
			.power_enable   = NULL,
			.power_state_changed = back_camera_power_state_changed,
			.set_clock      = camera_common_set_clock,
			.setup_io       = camera_common_vin_setup_io,
		},
		.deci = {
			.start_delay_ms = 0,
			.stop_delay_ms  = 0,
		},
	},
#elif defined(CONFIG_VIDEO_SP2518)
	{
		/* back_camera 656 interface */
		.module = 0,
		.sensor = &sensor[0],
		.type = NXP_CAPTURE_INF_PARALLEL,
		.parallel = {
			/* for 656 */
			.is_mipi        = false,
			.external_sync  = false, /* 656 interface */
			.h_active       = 800,
			.h_frontporch   = 7,
			.h_syncwidth    = 1,
			.h_backporch    = 10,
			.v_active       = 600,
			.v_frontporch   = 0,
			.v_syncwidth    = 2,
			.v_backporch    = 3,
			.clock_invert   = true,
			.port           = 0,
			.data_order     = NXP_VIN_Y0CBY1CR,
			.interlace      = false,
			.clk_rate       = 24000000,
			.late_power_down = true,
			.power_enable   = back_camera_power_enable,
			.power_state_changed = back_camera_power_state_changed,
			.set_clock      = camera_common_set_clock,
			.setup_io       = camera_common_vin_setup_io,
		},
		.deci = {
			.start_delay_ms = 0,
			.stop_delay_ms  = 0,
		},
	},
#endif
#if defined(CONFIG_VIDEO_SP0838)
	{
		/* front_camera 601 interface */
		.module = 0,
		.sensor = &sensor[1],
		.type = NXP_CAPTURE_INF_PARALLEL,
		.parallel = {
			.is_mipi        = false,
			.external_sync  = true,
			.h_active       = 640,
			.h_frontporch   = 0,
			.h_syncwidth    = 0,
			.h_backporch    = 2,
			.v_active       = 480,
			.v_frontporch   = 0,
			.v_syncwidth    = 0,
			.v_backporch    = 2,
			.clock_invert   = false,
			.port           = 0,
			.data_order     = NXP_VIN_CBY0CRY1,
			.interlace      = false,
			.clk_rate       = 24000000,
			.late_power_down = true,
			.power_enable   = front_camera_power_enable,
			.power_state_changed = front_camera_power_state_changed,
			.set_clock      = camera_common_set_clock,
			.setup_io       = camera_common_vin_setup_io,
		},
		.deci = {
			.start_delay_ms = 0,
			.stop_delay_ms  = 0,
		},
	},
#endif
	{ 0, NULL, 0, },
};

/* out platformdata */
static struct i2c_board_info hdmi_edid_i2c_boardinfo = {
	I2C_BOARD_INFO("nxp_edid", 0xA0>>1),
};

static struct nxp_v4l2_i2c_board_info edid = {
	.board_info = &hdmi_edid_i2c_boardinfo,
	.i2c_adapter_id = CFG_HDMI_EDID_I2C,
};

static struct i2c_board_info hdmi_hdcp_i2c_boardinfo = {
	I2C_BOARD_INFO("nxp_hdcp", 0x74>>1),
};

static struct nxp_v4l2_i2c_board_info hdcp = {
	.board_info = &hdmi_hdcp_i2c_boardinfo,
	.i2c_adapter_id = CFG_HDMI_HDCP_I2C,
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

/*------------------------------------------------------------------------------
 * SSP/SPI
 */
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
#include <linux/spi/spi.h>

static void spi0_cs(u32 chipselect)
{
#if (CFG_SPI0_CS_GPIO_MODE)
	if (nxp_soc_gpio_get_io_func( CFG_SPI0_CS ) != nxp_soc_gpio_get_altnum( CFG_SPI0_CS ))
		nxp_soc_gpio_set_io_func( CFG_SPI0_CS, nxp_soc_gpio_get_altnum( CFG_SPI0_CS ));

	nxp_soc_gpio_set_io_dir( CFG_SPI0_CS, 1);
	nxp_soc_gpio_set_out_value( CFG_SPI0_CS , chipselect);
#else
	;
#endif
}

struct pl022_config_chip spi0_info = {
	/* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
	.com_mode = CFG_SPI0_COM_MODE,
	.iface = SSP_INTERFACE_MOTOROLA_SPI,
	/* We can only act as master but SSP_SLAVE is possible in theory */
	.hierarchy = SSP_MASTER,
	/* 0 = drive TX even as slave, 1 = do not drive TX as slave */
	.slave_tx_disable = 1,
	.rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
	.tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
	.ctrl_len = SSP_BITS_8,
	.wait_state = SSP_MWIRE_WAIT_ZERO,
	.duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
	/*
	 * This is where you insert a call to a function to enable CS
	 * (usually GPIO) for a certain chip.
	 */
#if (CFG_SPI0_CS_GPIO_MODE)
	.cs_control = spi0_cs,
#endif
	.clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,
};

static struct spi_board_info spi_plat_board[] __initdata = {
	[0] = {
		.modalias        = "spidev",    /* fixup */
		.max_speed_hz    = 3125000,     /* max spi clock (SCK) speed in HZ */
		.bus_num         = 0,           /* Note> set bus num, must be smaller than ARRAY_SIZE(spi_plat_device) */
		.chip_select     = 0,           /* Note> set chip select num, must be smaller than spi cs_num */
		.controller_data = &spi0_info,
		.mode            = SPI_MODE_3 | SPI_CPOL | SPI_CPHA,
	},
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
	int id  = 0, ret;

	printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

	ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			DEV_NAME_SDHC "0", (void*)host->slot[slot_id]);
	if (ret < 0)
		pr_err("dw_mmc dw_mmc.%d: failed to request irq %d\n", id, irq);
	return 0;
}

static int _dwmci0_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC0_DETECT_IO;

	return nxp_soc_gpio_get_in_value(io);
}

static struct dw_mci_board _dwmci0_data = {
	.quirks			= DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23,
	.caps2			= MMC_CAP2_BROKEN_VOLTAGE,
	.detect_delay_ms= 200,
	.cd_type		= DW_MCI_CD_EXTERNAL,
	.clk_dly		= DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),

	.init			= _dwmci0_init,
	.get_cd			= _dwmci0_get_cd,
	.get_ro			= _dwmci_get_ro,
	.ext_cd_init	= _dwmci_ext_cd_init,
	.ext_cd_cleanup	= _dwmci_ext_cd_cleanup,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH0_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode 			= PIO_MODE,
#endif
};
#endif

#ifdef CONFIG_MMC_NXP_CH1
static struct dw_mci_board _dwmci1_data = {
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
					  DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23 | MMC_CAP_NONREMOVABLE,
	.detect_delay_ms= 200,
	.cd_type		= DW_MCI_CD_NONE,
	.pm_caps		= MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY,
	.clk_dly		= DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH1_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode			= PIO_MODE,
#endif
};
#endif

#ifdef CONFIG_MMC_NXP_CH2
static int _dwmci2_init(u32 slot_id, irq_handler_t handler, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	int io  = CFG_SDMMC2_DETECT_IO;
	int irq = IRQ_GPIO_START + io;
	int id  = 2, ret;

	printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

	ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			DEV_NAME_SDHC "2", (void*)host->slot[slot_id]);
	if (ret < 0)
		pr_err("dw_mmc dw_mmc.%d: failed to request irq %d\n", id, irq);
	return 0;
}

static int _dwmci2_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC2_DETECT_IO;

	return nxp_soc_gpio_get_in_value(io);
}

static struct dw_mci_board _dwmci2_data = {
	.quirks			= DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23 | MMC_CAP_HW_RESET,
	.cd_type		= DW_MCI_CD_EXTERNAL,
	.clk_dly		= DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),

	.init			= _dwmci2_init,
	.get_cd			= _dwmci2_get_cd,
	.get_ro			= _dwmci_get_ro,
	.desc_sz		= 4,
	.detect_delay_ms= 200,
	.sdr_timing		= 0x01010001,
	.ddr_timing		= 0x03030002,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH2_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode			= PIO_MODE,
#endif
};

static void __init board_fixup_dwmci2(void)
{
	struct dw_mci_board *pd = &_dwmci2_data;

	/* Settings applied for eMMC only */
	pd->quirks		= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
					  DW_MCI_QUIRK_HIGHSPEED |
					  DW_MMC_QUIRK_HW_RESET_PW |
					  DW_MCI_QUIRK_NO_DETECT_EBIT;
	pd->bus_hz		= 100 * 1000 * 1000;
	pd->caps		= MMC_CAP_UHS_DDR50 |
					  MMC_CAP_NONREMOVABLE |
					  MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23 |
					  MMC_CAP_ERASE | MMC_CAP_HW_RESET;
	pd->clk_dly		= DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(3) | DW_MMC_SAMPLE_PHASE(2);
	pd->init		= NULL;
	pd->get_cd		= NULL;
}
#endif

#endif /* CONFIG_MMC_DW */

/*------------------------------------------------------------------------------
 * RFKILL driver
 */
#if defined(CONFIG_NXP_RFKILL)
struct rfkill_dev_data  rfkill_dev_data =
{
	.supply_name 	= "vgps_3.3V",	// vwifi_3.3V, vgps_3.3V
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
#endif /* CONFIG_RFKILL_NXP */

/*------------------------------------------------------------------------------
 * USB HSIC power control.
 */
int nxp_hsic_phy_pwr_on(struct platform_device *pdev, bool on)
{
	return 0;
}
EXPORT_SYMBOL(nxp_hsic_phy_pwr_on);

/*------------------------------------------------------------------------------
 * HDMI CEC driver
 */
#if defined(CONFIG_NXP_HDMI_CEC)
static struct platform_device hdmi_cec_device = {
	.name			= NXP_HDMI_CEC_DRV_NAME,
};
#endif /* CONFIG_NXP_HDMI_CEC */

/*------------------------------------------------------------------------------
 * LED
 */
#if defined(CONFIG_LEDS_GPIO)
#include <linux/leds.h>

static struct gpio_led board_leds[] = {
	{
		.name		= "led1",
		.gpio		= (PAD_GPIO_B + 12),
		.active_low	= 1,
		.default_trigger	= "heartbeat",
	},
};

static struct gpio_led_platform_data gpio_led_pdata = {
	.num_leds	= ARRAY_SIZE(board_leds),
	.leds		= board_leds,
};

static struct platform_device gpio_led_device = {
	.name		= "leds-gpio",
	.id			= -1,
	.dev		= {
		.platform_data	= &gpio_led_pdata,
	},
};
#endif

/*------------------------------------------------------------------------------
 * HW revision
 */
#include <asm/system_info.h>
#include <board-revision.h>

int board_get_revision(void)
{
	return system_rev;
}

static void __init board_hwrev_init(void)
{
	int rev;

	rev  = nxp_soc_gpio_get_in_value(CFG_IO_HW_PCB1);
	rev |= nxp_soc_gpio_get_in_value(CFG_IO_HW_PCB2) << 1;
	rev |= nxp_soc_gpio_get_in_value(CFG_IO_HW_PCB3) << 2;

	/* Initialize system Revision & Serial */
	system_rev = rev;
	system_serial_high = 0xFA4418DB;
	system_serial_low  = 0xA4420544;

	printk("plat: board revision %d\n", rev);
}

/*------------------------------------------------------------------------------
 * register board platform devices
 */
void __init nxp_board_devices_register(void)
{
	struct nxp_lcd *lcd = nanopi2_get_lcd();

	printk("[Register board platform devices]\n");

	board_hwrev_init();

	if (board_is_nanopc() || board_is_smart4418()) {
#ifdef CONFIG_MMC_NXP_CH2
		board_fixup_dwmci2();
#endif
	}

#if defined(CONFIG_ARM_NXP_CPUFREQ)
	printk("plat: add dynamic frequency (pll.%d)\n", dfs_plat_data.pll_dev);
	platform_device_register(&dfs_plat_device);
#endif

#if defined (CONFIG_NXP_DISPLAY_LCD)
	nxp_platform_disp_init(lcd);
#endif

#if defined (CONFIG_FB_NXP)
	printk("plat: add framebuffer\n");
	nxp_platform_fb_data(lcd);
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

#if defined(CONFIG_MTD_NAND_NXP)
	platform_device_register(&nand_plat_device);
#endif

#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)
	printk("plat: add device keypad\n");
	platform_device_register(&key_plat_device);
#endif

	if (board_is_nanopi()) {
#if defined(CONFIG_REGULATOR_FIXED_VOLTAGE)
		printk("plat: add device fixed voltage\n");
		platform_device_register(&fixed_supply_dummy_device);
#endif
	} else {
#if defined(CONFIG_I2C_NXP_PORT3)
		platform_add_devices(i2c_devices, ARRAY_SIZE(i2c_devices));
#endif
	}

#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
	printk("plat: add device spdif playback\n");
	platform_device_register(&spdif_transciever);
	platform_device_register(&spdif_trans_dai);
#endif

#if defined(CONFIG_SND_CODEC_ES8316) || defined(CONFIG_SND_CODEC_ES8316_MODULE)
	if (board_is_nanopc() || board_is_smart4418()) {
		printk("plat: add device asoc-es8316\n");
		i2c_register_board_info(ES8316_I2C_BUS, &es8316_i2c_bdi, 1);
		platform_device_register(&es8316_dai);
	}
#endif

#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
	printk("plat: add device nxp-v4l2\n");
	back_camera_power_enable(1);
	is_back_camera_power_state_changed = false;
	platform_device_register(&nxp_v4l2_dev);
#endif

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
	spi_register_board_info(spi_plat_board, ARRAY_SIZE(spi_plat_board));
	printk("plat: register spidev\n");
#endif

#if defined(CONFIG_TOUCHSCREEN_GSLX680)
	printk("plat: add touch(gslX680) device\n");
	i2c_register_board_info(GSLX680_I2C_BUS, &gslX680_i2c_bdi, 1);
#endif

#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
	printk("plat: add touch(ft5x0x) device\n");
	ft5x0x_pdata.screen_max_x = lcd->width;
	ft5x0x_pdata.screen_max_y = lcd->height;
	i2c_register_board_info(FT5X0X_I2C_BUS, &ft5x0x_i2c_bdi, 1);
#endif

#if defined(CONFIG_TOUCHSCREEN_1WIRE)
	printk("plat: add onewire ts device\n");
	platform_device_register(&onewire_device);
#endif

#if defined(CONFIG_SENSORS_MMA865X) || defined(CONFIG_SENSORS_MMA865X_MODULE)
	printk("plat: add g-sensor mma865x\n");
	i2c_register_board_info(2, &mma865x_i2c_bdi, 1);
#elif defined(CONFIG_SENSORS_MMA7660) || defined(CONFIG_SENSORS_MMA7660_MODULE)
	printk("plat: add g-sensor mma7660\n");
	i2c_register_board_info(MMA7660_I2C_BUS, &mma7660_i2c_bdi, 1);
#endif

#if defined(CONFIG_RFKILL_NXP)
	printk("plat: add device rfkill\n");
	platform_device_register(&rfkill_device);
#endif

#if defined(CONFIG_NXP_HDMI_CEC)
	printk("plat: add device hdmi-cec\n");
	platform_device_register(&hdmi_cec_device);
#endif

#if defined(CONFIG_NXPMAC_ETH)
	if (board_is_nanopc() || board_is_smart4418()) {
		printk("plat: add device nxp-gmac\n");
		platform_device_register(&nxp_gmac_dev);
	}
#endif

#if defined(CONFIG_PPM_NXP)
	printk("plat: add device ppm\n");
	platform_device_register(&ppm_device);
#endif

#if defined(CONFIG_LEDS_GPIO)
	printk("plat: add device gpio_led\n");
	platform_device_register(&gpio_led_device);
#endif

	/* END */
	printk("\n");
}
