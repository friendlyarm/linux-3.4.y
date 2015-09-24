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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>

#if defined(CONFIG_NXP_DISPLAY)

#define DISPLAY_INPUT(n)	\
	(n == 0 ? DISP_DEVICE_SYNCGEN0 :	\
	(n == 1 ? DISP_DEVICE_SYNCGEN1 : DISP_CLOCK_RESCONV))

#define	SET_PARAM(s, d, member)	\
	if(s->member && s->member != d->member)	\
		d->member = s->member;

#define	SET_VSYNC_INFO(s, d)	{		\
	if (s && d) {						\
		SET_PARAM(s, d, interlace);         \
		SET_PARAM(s, d, h_active_len);      \
		SET_PARAM(s, d, h_sync_width);      \
		SET_PARAM(s, d, h_back_porch);      \
		SET_PARAM(s, d, h_front_porch);     \
		SET_PARAM(s, d, h_sync_invert);     \
		SET_PARAM(s, d, v_active_len);      \
		SET_PARAM(s, d, v_sync_width);      \
		SET_PARAM(s, d, v_back_porch);      \
		SET_PARAM(s, d, v_front_porch);     \
		SET_PARAM(s, d, v_sync_invert);     \
		SET_PARAM(s, d, pixel_clock_hz);    \
		SET_PARAM(s, d, clock_gen_num);     \
		SET_PARAM(s, d, clk_src_lv0);       \
		SET_PARAM(s, d, clk_div_lv0);       \
		SET_PARAM(s, d, clk_src_lv1);       \
		SET_PARAM(s, d, clk_div_lv1);       \
		}	\
	}

/*------------------------------------------------------------------------------
 * Syncgen 0/1
 */
#if defined(CONFIG_NXP_DISPLAY_LCD)
	#if (0 == CONFIG_NXP_DISPLAY_LCD) && !defined(CONFIG_NXP_DISPLAY_1ST)
	#define CONFIG_NXP_DISPLAY_1ST
	#elif !defined(CONFIG_NXP_DISPLAY_2ST)
	#define CONFIG_NXP_DISPLAY_2ST
	#endif
#endif
#if defined(CONFIG_NXP_DISPLAY_LVDS)
	#if (0 == CONFIG_NXP_DISPLAY_LVDS) && !defined(CONFIG_NXP_DISPLAY_1ST)
	#define CONFIG_NXP_DISPLAY_1ST
	#elif !defined(CONFIG_NXP_DISPLAY_2ST)
	#define CONFIG_NXP_DISPLAY_2ST
	#endif
#endif
#if defined(CONFIG_NXP_DISPLAY_MIPI)
	#if (0 == CONFIG_NXP_DISPLAY_MIPI) && !defined(CONFIG_NXP_DISPLAY_1ST)
	#define CONFIG_NXP_DISPLAY_1ST
	#elif !defined(CONFIG_NXP_DISPLAY_2ST)
	#define CONFIG_NXP_DISPLAY_2ST
	#endif
#endif
#if defined(CONFIG_NXP_DISPLAY_HDMI)
	#if (0 == CONFIG_NXP_DISPLAY_HDMI) && !defined(CONFIG_NXP_DISPLAY_1ST)
	#define CONFIG_NXP_DISPLAY_1ST
	#elif !defined(CONFIG_NXP_DISPLAY_2ST)
	#define CONFIG_NXP_DISPLAY_2ST
	#endif
#endif

#if defined(CONFIG_NXP_DISPLAY_1ST)
static struct platform_device syncgen_1st_dev = {
	.name	= DEV_NAME_DISP,
	.id		= 0,	/* module id */
};
#endif
#if defined(CONFIG_NXP_DISPLAY_2ST)
static struct platform_device syncgen_2st_dev = {
	.name	= DEV_NAME_DISP,
	.id		= 1,	/* module id */
};
#endif

static struct platform_device *syncgen_devices[] __initdata = {
	#if defined(CONFIG_NXP_DISPLAY_1ST)
	&syncgen_1st_dev,
	#endif
	#if defined(CONFIG_NXP_DISPLAY_2ST)
	&syncgen_2st_dev,
	#endif
};


/*------------------------------------------------------------------------------
 * LCD platform device
 */
#if defined (CONFIG_NXP_DISPLAY_LCD)

static struct disp_vsync_info __lcd_vsync = {
	/* default parameters refer to cfg_main.h */
	#if defined(CFG_DISP_PRI_RESOL_WIDTH) && defined(CFG_DISP_PRI_RESOL_HEIGHT)
	.h_active_len	= CFG_DISP_PRI_RESOL_WIDTH,
	.h_sync_width	= CFG_DISP_PRI_HSYNC_SYNC_WIDTH,
	.h_back_porch	= CFG_DISP_PRI_HSYNC_BACK_PORCH,
	.h_front_porch	= CFG_DISP_PRI_HSYNC_FRONT_PORCH,
	.h_sync_invert	= CFG_DISP_PRI_HSYNC_ACTIVE_HIGH,
	.v_active_len	= CFG_DISP_PRI_RESOL_HEIGHT,
	.v_sync_width	= CFG_DISP_PRI_VSYNC_SYNC_WIDTH,
	.v_back_porch	= CFG_DISP_PRI_VSYNC_BACK_PORCH,
	.v_front_porch	= CFG_DISP_PRI_VSYNC_FRONT_PORCH,
	.v_sync_invert	= CFG_DISP_PRI_VSYNC_ACTIVE_HIGH,
	.pixel_clock_hz	= CFG_DISP_PRI_PIXEL_CLOCK,
	.clk_src_lv0	= CFG_DISP_PRI_CLKGEN0_SOURCE,
	.clk_div_lv0	= CFG_DISP_PRI_CLKGEN0_DIV,
	.clk_src_lv1	= CFG_DISP_PRI_CLKGEN1_SOURCE,
	.clk_div_lv1	= CFG_DISP_PRI_CLKGEN1_DIV,
	#endif
};
static struct disp_lcd_param   __lcd_devpar;

static struct nxp_lcd_plat_data lcd_data = {
	.display_in		= DISPLAY_INPUT(CONFIG_NXP_DISPLAY_LCD_IN),
	.display_dev	= DISP_DEVICE_LCD,
	.vsync			= &__lcd_vsync,
	.dev_param	 	= (union disp_dev_param*)&__lcd_devpar,
};

static struct platform_device lcd_device = {
	.name	= DEV_NAME_LCD,
	.id		= -1,
	.dev    = {
		.platform_data	= &lcd_data
	},
};

static void __disp_lcd_dev_data(struct disp_vsync_info *vsync,
					void *dev_par, struct disp_syncgen_par *sgpar)
{
	struct nxp_lcd_plat_data *plcd = &lcd_data;
	struct disp_lcd_param *dst = (struct disp_lcd_param *)plcd->dev_param;
	struct disp_lcd_param *src = dev_par;

	if (src) {
		SET_PARAM(src, dst, lcd_format);
		SET_PARAM(src, dst, lcd_mpu_type);
		SET_PARAM(src, dst, invert_field);
		SET_PARAM(src, dst, swap_RB);
		SET_PARAM(src, dst, yc_order);
		SET_PARAM(src, dst, lcd_init);
		SET_PARAM(src, dst, lcd_exit);
	}

	if (sgpar)
		plcd->sync_gen = sgpar;

	SET_VSYNC_INFO(vsync, plcd->vsync);
}
#else
#define	__disp_lcd_dev_data(s, p, g)
#endif /* LCD */

/*------------------------------------------------------------------------------
 * LVDS platform device
 */
#if defined (CONFIG_NXP_DISPLAY_LVDS)

static struct disp_vsync_info  __lvds_vsync = {
	/* default parameters refer to cfg_main.h */
	#if defined(CFG_DISP_PRI_RESOL_WIDTH) && defined(CFG_DISP_PRI_RESOL_HEIGHT)
	.h_active_len	= CFG_DISP_PRI_RESOL_WIDTH,
	.h_sync_width	= CFG_DISP_PRI_HSYNC_SYNC_WIDTH,
	.h_back_porch	= CFG_DISP_PRI_HSYNC_BACK_PORCH,
	.h_front_porch	= CFG_DISP_PRI_HSYNC_FRONT_PORCH,
	.h_sync_invert	= CFG_DISP_PRI_HSYNC_ACTIVE_HIGH,
	.v_active_len	= CFG_DISP_PRI_RESOL_HEIGHT,
	.v_sync_width	= CFG_DISP_PRI_VSYNC_SYNC_WIDTH,
	.v_back_porch	= CFG_DISP_PRI_VSYNC_BACK_PORCH,
	.v_front_porch	= CFG_DISP_PRI_VSYNC_FRONT_PORCH,
	.v_sync_invert	= CFG_DISP_PRI_VSYNC_ACTIVE_HIGH,
	.pixel_clock_hz	= CFG_DISP_PRI_PIXEL_CLOCK,
	.clk_src_lv0	= CFG_DISP_PRI_CLKGEN0_SOURCE,
	.clk_div_lv0	= CFG_DISP_PRI_CLKGEN0_DIV,
	.clk_src_lv1	= CFG_DISP_PRI_CLKGEN1_SOURCE,
	.clk_div_lv1	= CFG_DISP_PRI_CLKGEN1_DIV,
	#endif
};
static struct disp_lvds_param  __lvds_devpar = {
#if defined (CFG_DISP_LVDS_LCD_FORMAT)
	.lcd_format = CFG_DISP_LVDS_LCD_FORMAT,
#endif
};

static struct nxp_lcd_plat_data lvds_data = {
	.display_in		= DISPLAY_INPUT(CONFIG_NXP_DISPLAY_LVDS_IN),
	.display_dev	= DISP_DEVICE_LVDS,
	.vsync			= &__lvds_vsync,
	.dev_param	 	= (union disp_dev_param*)&__lvds_devpar,
};

static struct platform_device lvds_device = {
	.name	= DEV_NAME_LVDS,
	.id		= -1,
	.dev    = {
		.platform_data	= &lvds_data
	},
};

static void __disp_lvds_dev_data(struct disp_vsync_info *vsync,
				struct disp_lvds_param *dev_par, struct disp_syncgen_par *sgpar)
{
	struct nxp_lcd_plat_data *plcd = &lvds_data;
	struct disp_lvds_param *dst = (struct disp_lvds_param *)plcd->dev_param;
	struct disp_lvds_param *src = dev_par;

	if (dev_par) {
		SET_PARAM(src, dst, lcd_format);
		SET_PARAM(src, dst, inv_hsync);
		SET_PARAM(src, dst, inv_vsync);
		SET_PARAM(src, dst, inv_de);
		SET_PARAM(src, dst, inv_inclk_pol);
		SET_PARAM(src, dst, loc_enable);
		SET_PARAM(src, dst, loc_map[0]);
		SET_PARAM(src, dst, loc_map[1]);
		SET_PARAM(src, dst, loc_map[2]);
		SET_PARAM(src, dst, loc_map[3]);
		SET_PARAM(src, dst, loc_map[4]);
		SET_PARAM(src, dst, loc_map[5]);
		SET_PARAM(src, dst, loc_map[6]);
		SET_PARAM(src, dst, loc_map[7]);
		SET_PARAM(src, dst, loc_map[8]);
		SET_PARAM(src, dst, loc_mask[0]);
		SET_PARAM(src, dst, loc_mask[1]);
		SET_PARAM(src, dst, loc_pol[0]);
		SET_PARAM(src, dst, loc_pol[1]);
	}

	if (sgpar)
		plcd->sync_gen = sgpar;

	SET_VSYNC_INFO(vsync, plcd->vsync);
}

#else
#define	__disp_lvds_dev_data(s, p, g)
#endif /* LVDS */


/*------------------------------------------------------------------------------
 * MiPi platform device
 */
#if defined (CONFIG_NXP_DISPLAY_MIPI)

static struct disp_vsync_info  __mipi_vsync = {
	/* default parameters refer to cfg_main.h */
	#if defined(CFG_DISP_PRI_RESOL_WIDTH) && defined(CFG_DISP_PRI_RESOL_HEIGHT)
	.h_active_len	= CFG_DISP_PRI_RESOL_WIDTH,
	.h_sync_width	= CFG_DISP_PRI_HSYNC_SYNC_WIDTH,
	.h_back_porch	= CFG_DISP_PRI_HSYNC_BACK_PORCH,
	.h_front_porch	= CFG_DISP_PRI_HSYNC_FRONT_PORCH,
	.h_sync_invert	= CFG_DISP_PRI_HSYNC_ACTIVE_HIGH,
	.v_active_len	= CFG_DISP_PRI_RESOL_HEIGHT,
	.v_sync_width	= CFG_DISP_PRI_VSYNC_SYNC_WIDTH,
	.v_back_porch	= CFG_DISP_PRI_VSYNC_BACK_PORCH,
	.v_front_porch	= CFG_DISP_PRI_VSYNC_FRONT_PORCH,
	.v_sync_invert	= CFG_DISP_PRI_VSYNC_ACTIVE_HIGH,
	.pixel_clock_hz	= CFG_DISP_PRI_PIXEL_CLOCK,
	.clk_src_lv0	= CFG_DISP_PRI_CLKGEN0_SOURCE,
	.clk_div_lv0	= CFG_DISP_PRI_CLKGEN0_DIV,
	.clk_src_lv1	= CFG_DISP_PRI_CLKGEN1_SOURCE,
	.clk_div_lv1	= CFG_DISP_PRI_CLKGEN1_DIV,
	#endif
};
static struct disp_mipi_param  __mipi_devpar;

static struct nxp_lcd_plat_data mipi_data = {
	.display_in		= DISPLAY_INPUT(CONFIG_NXP_DISPLAY_MIPI_IN),
	.display_dev	= DISP_DEVICE_MIPI,
	.vsync			= &__mipi_vsync,
	.dev_param	 	= (union disp_dev_param*)&__mipi_devpar,
};

static struct platform_device mipi_device = {
	.name	= DEV_NAME_MIPI,
	.id		= -1,
	.dev    = {
		.platform_data	= &mipi_data
	},
};

static void __disp_mipi_dev_data(struct disp_vsync_info *vsync,
				struct disp_mipi_param *dev_par, struct disp_syncgen_par *sgpar)
{
	struct nxp_lcd_plat_data *plcd = &mipi_data;
	struct disp_mipi_param *src = dev_par;
	struct disp_mipi_param *dst = (struct disp_mipi_param *)plcd->dev_param;

	if (dev_par) {
		SET_PARAM(src, dst, pllpms);
		SET_PARAM(src, dst, bandctl);
		SET_PARAM(src, dst, pllctl);
		SET_PARAM(src, dst, phyctl);
		SET_PARAM(src, dst, lcd_init);
		SET_PARAM(src, dst, lcd_exit);
	}

	if (sgpar)
		plcd->sync_gen = sgpar;

	SET_VSYNC_INFO(vsync, plcd->vsync);
}

#else
#define	__disp_mipi_dev_data(s, p, g)
#endif /* MIPI */


/*------------------------------------------------------------------------------
 * HDMI platform device
 */
#if defined (CONFIG_NXP_DISPLAY_HDMI)

static struct disp_vsync_info  __hdmi_vsync = {
	#if	  defined (CONFIG_NXP_DISPLAY_HDMI_1280_720P)
	.h_active_len	= 1280,
	.v_active_len	=  720,
	#elif defined (CONFIG_NXP_DISPLAY_HDMI_1920_1080P)
	.h_active_len	= 1920,
	.v_active_len	= 1080,
	#else
	#error "***** NOT SUPPORT HDMI RESOLUTION !!! *****"
	#endif
};

static struct disp_hdmi_param  __hdmi_devpar = {
	#if	  defined (CONFIG_NXP_DISPLAY_HDMI_1280_720P)
	.preset = 0,
	#elif defined (CONFIG_NXP_DISPLAY_HDMI_1920_1080P)
	.preset = 1,
	#endif
};

static struct nxp_lcd_plat_data hdmi_data = {
	.display_in		= DISPLAY_INPUT(CONFIG_NXP_DISPLAY_HDMI_IN),
	.display_dev	= DISP_DEVICE_HDMI,
	.vsync			= &__hdmi_vsync,
	.dev_param	 	= (union disp_dev_param*)&__hdmi_devpar,
};

static struct platform_device hdmi_device = {
	.name	= DEV_NAME_HDMI,
	.id		= -1,
	.dev    = {
		.platform_data	= &hdmi_data
	},
};

static void __disp_hdmi_dev_data(struct disp_vsync_info *vsync,
					struct disp_hdmi_param *dev_par, struct disp_syncgen_par *sgpar)
{
	struct nxp_lcd_plat_data *plcd = &hdmi_data;
	struct disp_vsync_info *vsc = &__hdmi_vsync;
	struct disp_hdmi_param *dst = (struct disp_hdmi_param *)plcd->dev_param;
	struct disp_hdmi_param *src = dev_par;

	if (dev_par) {
		switch (src->preset) {
		case  0: vsc->h_active_len = 1280, vsc->v_active_len =  720; break;
		case  1: vsc->h_active_len = 1920, vsc->v_active_len = 1080; break;
		default: printk("***** %s: NOT SUPPORT HDMI RESOLUTION PRESET (%d) *****\n",
					__func__, src->preset);
				return;
		}
		SET_PARAM(src, dst, preset);
	}

	if (sgpar)
		plcd->sync_gen = sgpar;

	SET_VSYNC_INFO(vsync, plcd->vsync);
}

#else
#define	__disp_hdmi_dev_data(s, p, g)
#endif /* HDMI */

/*------------------------------------------------------------------------------
 * Resolution Convertor platform device
 */
#if defined (CONFIG_NXP_DISPLAY_RESC)

static struct disp_vsync_info  __resc_vsync;
static struct disp_resc_param  __resc_devpar;

static struct nxp_lcd_plat_data resc_data = {
	.display_in		= DISPLAY_INPUT(CONFIG_NXP_DISPLAY_RESC_IN),
	.display_dev	= DISP_DEVICE_RESCONV,
	.vsync			= &__resc_vsync,
	.dev_param	 	= (union disp_dev_param*)&__resc_devpar,
};

static struct platform_device resc_device = {
	.name	= DEV_NAME_RESCONV,
	.id		= -1,
	.dev    = {
		.platform_data	= &resc_data
	},
};

static void __disp_resc_dev_data(struct disp_vsync_info *vsync,
					struct disp_resc_param *dev_par, struct disp_syncgen_par *sgpar)
{
	struct nxp_lcd_plat_data *plcd = &resc_data;
	struct disp_vsync_info *vsc = &__resc_vsync;
	struct disp_resc_param *dst = (struct disp_resc_param *)plcd->dev_param;
	struct disp_resc_param *src = dev_par;

	if (dev_par) {
		SET_PARAM(src, dst, clip_left);
		SET_PARAM(src, dst, clip_right);
		SET_PARAM(src, dst, hs_delay);
		SET_PARAM(src, dst, sync2in_vs);
		SET_PARAM(src, dst, h_f_on);
		SET_PARAM(src, dst, v_f_on);
		SET_PARAM(src, dst, h_f_coe);
		SET_PARAM(src, dst, v_f_coe);
	}

	if (sgpar)
		plcd->sync_gen = sgpar;

	SET_VSYNC_INFO(vsync, plcd->vsync);
}

#else
#define	__disp_resc_dev_data(s, p, g)
#endif /* RESC */

/*------------------------------------------------------------------------------
 * set cpu platform devices
 */
void nxp_platform_disp_device_data(enum disp_dev_type device, struct disp_vsync_info *vsync,
				void *dev_par, struct disp_syncgen_par *sgpar)
{
	switch (device) {
	case DISP_DEVICE_LVDS:
			__disp_lvds_dev_data(vsync, dev_par, sgpar); break;
	case DISP_DEVICE_MIPI:
			__disp_mipi_dev_data(vsync, dev_par, sgpar); break;
	case DISP_DEVICE_HDMI:
			__disp_hdmi_dev_data(vsync, dev_par, sgpar); break;
	case DISP_DEVICE_LCD :
			__disp_lcd_dev_data (vsync, dev_par, sgpar); break;
	case DISP_DEVICE_RESCONV:
			__disp_resc_dev_data(vsync, dev_par, sgpar); break;
	default:
		printk("***** %s: not support display device %d *****\n", __func__, device);
		break;
	}
	return;
}
#endif /* CONFIG_NXP_DISPLAY */
