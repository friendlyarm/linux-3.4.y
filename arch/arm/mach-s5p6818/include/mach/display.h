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
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

enum disp_dev_type {
	DISP_DEVICE_RESCONV		= 0,
	DISP_DEVICE_LCD			= 1,
	DISP_DEVICE_HDMI  		= 2,
	DISP_DEVICE_MIPI  		= 3,
	DISP_DEVICE_LVDS  		= 4,
    DISP_DEVICE_TVOUT       = 5,
	DISP_DEVICE_SYNCGEN0	= 6,
	DISP_DEVICE_SYNCGEN1	= 7,
	DISP_DEVICE_END			,
};

enum {
	DISP_CLOCK_RESCONV		= 0,
	DISP_CLOCK_LCD			= 1,
	DISP_CLOCK_MIPI  		= 2,
	DISP_CLOCK_LVDS  		= 3,
	DISP_CLOCK_HDMI  		= 4,
	DISP_CLOCK_END			,
};

#define DISPLAY_SYNCGEN_NUM		2	/* primary, secondary */

/*
 * video sync info
 */
struct disp_vsync_info {
	int	interlace;
	int	h_active_len;
	int	h_sync_width;
	int	h_back_porch;
	int	h_front_porch;
	int	h_sync_invert;		/* default active low */
	int	v_active_len;
	int	v_sync_width;
	int	v_back_porch;
	int	v_front_porch;
	int	v_sync_invert;		/* default active low */
	int pixel_clock_hz;		/* HZ */
	int clock_gen_num;
	/* clock generator */
	int clk_src_lv0;
	int clk_div_lv0;
	int clk_src_lv1;
	int clk_div_lv1;
};

/*
 * syncgen control (DPC)
 */
#define	DISP_SYNCGEN_DELAY_RGB_PVD		(1<<0)
#define	DISP_SYNCGEN_DELAY_HSYNC_CP1	(1<<1)
#define	DISP_SYNCGEN_DELAY_VSYNC_FRAM	(1<<2)
#define	DISP_SYNCGEN_DELAY_DE_CP		(1<<3)

struct disp_syncgen_par {
	/* multilayer format */
	int 		 interlace;
	/* syncgen format */
	unsigned int out_format;
	int			 invert_field;		/* 0= Normal Field(Low is odd field), 1: Invert Field(low is even field) */
	int			 swap_RB;
	unsigned int yc_order;			/* for CCIR output */
	/* exten sync delay  */
	int			delay_mask;			/* if not 0, set defalut delays (rgb_pvd, hsync_cp1, vsync_fram, de_cp2) */
	int 		d_rgb_pvd;			/* the delay value for RGB/PVD signal   , 0 ~ 16, default  0 */
	int			d_hsync_cp1;		/* the delay value for HSYNC/CP1 signal , 0 ~ 63, default 12 */
	int			d_vsync_fram;		/* the delay value for VSYNC/FRAM signal, 0 ~ 63, default 12 */
	int			d_de_cp2;			/* the delay value for DE/CP2 signal    , 0 ~ 63, default 12 */
	/* exten sync delay  */
	int			vs_start_offset;	/* start veritcal sync offset, defatult 0 */
	int			vs_end_offset;		/* end veritcla sync offset  , defatult 0 */
	int			ev_start_offset;	/* start even veritcal sync offset, defatult 0 */
	int			ev_end_offset;		/* end even veritcal sync offset  , defatult 0 */
	/* pad clock seletor */
	int			vclk_select;		/* 0=vclk0, 1=vclk2 */
	int			clk_inv_lv0;		/* OUTCLKINVn */
	int			clk_delay_lv0;		/* OUTCLKDELAYn */
	int			clk_inv_lv1;		/* OUTCLKINVn */
	int			clk_delay_lv1;		/* OUTCLKDELAYn */
	int			clk_sel_div1;		/* 0=clk1_inv, 1=clk1_div_2_ns */
};

struct disp_process_dev;
struct disp_process_ops {
	int	 (*set_vsync)	(struct disp_process_dev *dev, struct disp_vsync_info *vsync);
	int  (*get_vsync)	(struct disp_process_dev *dev, struct disp_vsync_info *vsync);
	int	 (*prepare)  	(struct disp_process_dev *dev);				/* last  -> first */
	int	 (*enable) 	 	(struct disp_process_dev *dev, int enable);	/* enable: first -> last, disable: last  -> first  */
	int  (*stat_enable)	(struct disp_process_dev *dev);
	int	 (*suspend)	 	(struct disp_process_dev *dev);				/* last  -> first */
	void (*pre_resume)	(struct disp_process_dev *dev);				/* last  -> first */
	void (*resume)	 	(struct disp_process_dev *dev);				/* first -> last  */
	int	 (*capability)	(struct disp_process_dev *dev, struct disp_vsync_info *vsync);
	struct disp_process_dev *dev;
};

#define	PROC_STATUS_READY		(1<<0)
#define	PROC_STATUS_ENABLE		(1<<1)
#define	PROC_STATUS_SUSPEND		(1<<2)

/* display device instance (syncgen, lvds, lcd,..) */
struct disp_process_dev {
	const char *name;
	int dev_id;
	int	dev_in;
	int	dev_out;
	unsigned int save_addr;
	unsigned int base_addr;
	struct list_head list;
	unsigned int status;
	spinlock_t lock;
	struct disp_vsync_info	 vsync;
	struct disp_syncgen_par	 sync_gen;
	struct disp_process_ops *disp_ops;
	void * dev_param;
	void * dev_info;
    void * priv;
};

/*
 * ASSERT
 */
#define RET_ASSERT_VAL(_expr_, _ret_)	{			\
	if (!(_expr_)) {								\
		printk(KERN_ERR "%s(%d) : %s %s \n",		\
			__func__, __LINE__, "ASSERT", #_expr_);	\
		if ( _ret_) return _ret_; else return 0;	\
	}											\
}

#define RET_ASSERT(_expr_)	{			\
	if (!(_expr_)) {								\
		printk(KERN_ERR "%s(%d) : %s %s \n",		\
			__func__, __LINE__, "ASSERT", #_expr_);	\
		return;										\
	}											\
}

#define RET_ASSERT_NULL(_expr_)	{			\
	if (!(_expr_)) {								\
		printk(KERN_ERR "%s(%d) : %s %s \n",		\
			__func__, __LINE__, "ASSERT", #_expr_);	\
		return NULL;										\
	}											\
}

/* device string */
extern const char *dev_to_str(enum disp_dev_type device);

/*
 * multilayer control (MLC)
 */
#define	LAYER_RGB_NUM			2	/* number of RGB layer  : 0 = RGB0, 1 =RGB1 */
#define	LAYER_VID_NUM			1	/* number of Video layer: 3 = VIDEO */
#define	MULTI_LAYER_NUM			(LAYER_RGB_NUM + LAYER_VID_NUM)

enum {
	LAYER_RGB_0_IDX	= 0,
	LAYER_RGB_1_IDX	= 1,
	LAYER_VIDEO_IDX	= 2,
};

/* for prototype layer index */
#define	MLC_LAYER_RGB_0			0	/* number of RGB layer 0 */
#define	MLC_LAYER_RGB_1			1	/* number of RGB layer 1 */
#define	MLC_LAYER_VIDEO			3	/* number of Video layer: 3 = VIDEO */

#define	VIDEO_COLOR_ALPHA		(1)
#define	VIDEO_COLOR_BRIGHT		(2)
#define	VIDEO_COLOR_HUE			(3)
#define	VIDEO_COLOR_CONTRAST	(4)
#define	VIDEO_COLOR_SATURATION	(5)
#define	VIDEO_COLOR_GAMMA		(6)

#define	RGB_COLOR_ALPHA			(1)
#define	RGB_COLOR_TRANSP		(2)
#define	RGB_COLOR_INVERT		(3)

struct mlc_layer_info {
	char	     name[16];
	unsigned int type;
	unsigned int format;
	int			 width;
	int			 height;
	int			 pixelbyte;
	int			 stride;
	unsigned int address;
	int			 enable;
	unsigned int hFilter;
	unsigned int vFilter;
	int			 pos_x;
	int			 pos_y;
	int			 clipped;
	int			 left;
	int			 top;
	int			 right;
	int			 bottom;

    /* psw0523 add for source crop */
    bool         en_source_crop;
    int          src_crop_left;
    int          src_crop_top;
    int          src_crop_width;
    int          src_crop_height;

	/* color */
	union {
		struct {
			unsigned int transcolor;
			unsigned int invertcolor;
			unsigned int alphablend;
		};
		struct {
			int		alpha;			/* Default= 15,    0 <= Range <= 16		*/
			int		bright;			/* Default= 0,  -128 <= Range <= 128	*/
			int		contrast;		/* Default= 0,     0 <= Range <= 8		*/
			double	hue;			/* Default= 0,     0 <= Range <= 360	*/
			double	saturation;		/* Default= 0,  -100 <= Range <= 100	*/
			int		satura;
			int		gamma;
		};
	} color;
};

struct disp_multily_dev {
	int	x_resol;
	int	y_resol;
	int	fb_layer;
	int	video_prior;			/* 0 = video > RGB0.., 1 = RGB0 > vidoe > RGB1 .., 2 = RGB0 > RGB1 > vidoe .. */
	int	enable;
	unsigned int bg_color;
	int	interlace;
	unsigned int mem_lock_len;	/* lock size for memory access, 4, 8, 16 only valid (default 8byter) */
	unsigned int base_addr;
	unsigned int save_addr;
	struct mlc_layer_info layer[MULTI_LAYER_NUM];		/* RGB/Video layer */
};

/*
 * LCD control
 */
struct lcd_operation {
	int	 (*lcd_poweron)		(int module, void *data);
	int	 (*lcd_poweroff) 	(int module, void *data);
	int	 (*lcd_suspend)		(int module, void *data);
	int	 (*lcd_resume) 		(int module, void *data);
	int	 (*backlight_on)	(int module, void *data);
	int	 (*backlight_off) 	(int module, void *data);
	int	 (*backlight_suspend)(int module, void *data);
	int	 (*backlight_resume)(int module, void *data);
	void *data;
};

/*
 * LVDS
 */
#define	LVDS_PCLK_L_MIN	 	 40000000
#define	LVDS_PCLK_L_MAX	 	 80000000
#define	LVDS_PCLK_H_MIN	 	 80000000
#define	LVDS_PCLK_H_MAX		160000000

#define	LVDS_PMS_L_P			(10)
#define	LVDS_PMS_L_M			(10)
#define	LVDS_PMS_L_S			(1)
#define	LVDS_PMS_L_VSEL			(0)
#define	LVDS_PMS_H_P			(14)
#define	LVDS_PMS_H_M			(14)
#define	LVDS_PMS_H_S			(1)
#define	LVDS_PMS_H_VSEL			(1)

/* LVDS register position */
#define	LVDSCTRL0_LCDFORMAT_POS	(19)
#define	LVDSCTRL0_VSYNC_POL_POS	(21)
#define	LVDSCTRL0_HSYNC_POL_POS	(22)
#define	LVDSCTRL0_DE_POL_POS	(23)

#define	LVDSCTRL2_PMS_P_POS		(0)
#define	LVDSCTRL2_PMS_M_POS		(6)
#define	LVDSCTRL2_PMS_S_POS		(12)
#define	LVDSCTRL2_VSEL_POS		(14)
#define	LVDSCTRL2_CK_POL_POS	(15)

/*
 * LCD private parameters
 */

// RGB LCD Param
struct disp_lcd_param {
	unsigned int lcd_format;
	int 		 lcd_mpu_type;		/* set when lcd type is mpu */
	int			 invert_field;		/* 0= Normal Field(Low is odd field), 1: Invert Field(low is even field) */
	int			 swap_RB;
	unsigned int yc_order;			/* for CCIR output */
	int	(*lcd_init)	(int width, int height, void *private_data);
	int	(*lcd_exit)	(int width, int height, void *private_data);
};

// LVDS
struct disp_lvds_param {
	unsigned int lcd_format;		/* 0:VESA, 1:JEIDA, 2: Location Setting */
	int			 inv_hsync;			/* hsync polarity invert for VESA, JEIDA */
	int			 inv_vsync;			/* bsync polarity invert for VESA, JEIDA */
	int			 inv_de;			/* de polarity invert for VESA, JEIDA */
	int			 inv_inclk_pol;		/* input clock(pixel clock) polarity invert */
	/* Location settting */
	int			 loc_enable;
	unsigned int loc_map[9];		/* when lcd format is "Location Setting", LCDn = 8bit * 35 = 35byte := 9dword */
	unsigned int loc_mask[2];		/* when lcd format is "Location Setting", 0 ~ 34 */
	unsigned int loc_pol[2];		/* when lcd format is "Location Setting", 0 ~ 34 */
};

// MIPI
struct disp_mipi_param {
	unsigned int pllpms;			/* Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. */
	unsigned int bandctl;			/* [3:0] Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. */
	unsigned int pllctl;
	unsigned int phyctl;			/* Refer to 10.2.3 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf or NX_MIPI_PHY_B_DPHYCTL enum or LN28LPP_MipiDphyCore1p5Gbps_Supplement. */
	int	(*lcd_init)	(int width, int height, void *private_data);
	int	(*lcd_exit)	(int width, int height, void *private_data);
	void *private_data;
};

// HDMI
struct disp_hdmi_param {
    int preset;         	/* 0 = 1280 * 720p, 1=1920 * 1080p */
    int internal_irq;
    int external_irq;
};

// Resolution convertor
struct disp_resc_param {
	/* clipping */
	int	clip_left;
	int	clip_right;
	/* extend factor */
	int hs_delay;
	int sync2in_vs;
	/* fine-scaler filter */
	int	h_f_on;
	int	v_f_on;
	unsigned int h_f_coe;	/* horizontal filter coefficient value */
	unsigned int v_f_coe;	/* vertical   filter coefficient value */
};

// display device params */
union disp_dev_param {
	struct disp_lcd_param 	lcd;
	struct disp_lvds_param 	lvds;
	struct disp_mipi_param 	mipi;
	struct disp_hdmi_param 	hdmi;
	struct disp_resc_param	resc;
};

/**
 * for callback list
 */
typedef void (*disp_irq_handler)(void *);
struct disp_irq_callback {
    struct list_head list;
    disp_irq_handler handler;
    void *data;
};

#endif /* __DISPLAY_H__ */
