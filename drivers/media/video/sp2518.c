
/*
 * sp2518 Camera Driver
 *
 * Copyright (C) 2011 Actions Semiconductor Co.,LTD
 * Wang Xin <wangxin@actions-semi.com>
 *
 * Based on ov227x driver
 *
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Kuninori Morimoto <morimoto.kuninori@renesas.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * fixed by swpark@nexell.co.kr for compatibility with general v4l2 layer (not using soc camera interface)
 */

//Suruibin 2015-04-03 V1.0 ae 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>

#define MODULE_NAME "SP2518"

// TODO : move this to PLAT/device.c
#if 0
static struct i2c_board_info asoc_i2c_camera = {
    I2C_BOARD_INFO(MODULE_NAME, 0x30),//caichsh
};
#endif

#ifdef SP2518_DEBUG
#define assert(expr) \
    if (unlikely(!(expr))) {				\
        pr_err("Assertion failed! %s,%s,%s,line=%d\n",	\
#expr, __FILE__, __func__, __LINE__);	\
    }

#define SP2518_DEBUG(fmt,args...) printk(KERN_ALERT fmt, ##args)
#else

#define assert(expr) do {} while (0)

#define SP2518_DEBUG(fmt,args...)
#endif

#define PID                 0x02 /* Product ID Number  *///caichsh
#define SP2518              0x53
#define OUTTO_SENSO_CLOCK   24000000
#define NUM_CTRLS           11
#define V4L2_IDENT_SP2518   64112

/* private ctrls */
#define V4L2_CID_SCENE_EXPOSURE         (V4L2_CTRL_CLASS_CAMERA | 0x1001)
#define V4L2_CID_PRIVATE_PREV_CAPT      (V4L2_CTRL_CLASS_CAMERA | 0x1002)

#if 0
enum {
    V4L2_WHITE_BALANCE_INCANDESCENT = 0,
    V4L2_WHITE_BALANCE_FLUORESCENT,
    V4L2_WHITE_BALANCE_DAYLIGHT,
    V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT,
    V4L2_WHITE_BALANCE_TUNGSTEN
};
#else
enum {
    V4L2_WHITE_BALANCE_INCANDESCENT = 0,
    /*V4L2_WHITE_BALANCE_FLUORESCENT,*/
    V4L2_WHITE_BALANCE_DAYLIGHT,
    V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT,
    /*V4L2_WHITE_BALANCE_TUNGSTEN*/
};
#endif

struct regval_list {
    unsigned char reg_num;
    unsigned char value;
};

/****************************************************************************************
 * predefined reg values
 */
#define ENDMARKER { 0xff, 0xff }

static struct regval_list sp2518_fmt_yuv422_yuyv[] =
{
    //YCbYCr
    {0xfd,0x00},
    {0x35,0x40},
    ENDMARKER,
};

static struct regval_list sp2518_fmt_yuv422_yvyu[] =
{
    //YCrYCb
    {0xfd,0x00},
    {0x35,0x41},
    ENDMARKER,
};

static struct regval_list sp2518_fmt_yuv422_vyuy[] =
{
    //CrYCbY
    {0xfd,0x00},
    {0x35,0x01},
    ENDMARKER,
};

static struct regval_list sp2518_fmt_yuv422_uyvy[] =
{
    //CbYCrY
    {0xfd,0x00},
    {0x35,0x00},
    ENDMARKER,
};

static struct regval_list sp2518_fmt_raw[] __attribute__((unused)) =
{
    {0xfd,0x00},
    ENDMARKER,
};

/*
 *AWB
 */
static const struct regval_list sp2518_awb_regs_enable[] =
{
    {0xfd,0x01},
    {0x28,0xce},
    {0x29,0x8a},
    {0xfd,0x00},
    {0x32,0x0d},
    ENDMARKER,
};

static const struct regval_list sp2518_awb_regs_diable[] =
{
    //{0xfd,0x01},
    //{0x28,0xce},
    //{0x29,0x8a},
    {0xfd,0x00},
    {0x32,0x05},
    ENDMARKER,
};

static struct regval_list sp2518_wb_cloud_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xe2},
    {0x29,0x82},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp2518_wb_daylight_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xc1},
    {0x29,0x88},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp2518_wb_incandescence_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x7b},
    {0x29,0xd3},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp2518_wb_fluorescent_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xb4},
    {0x29,0xc4},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp2518_wb_tungsten_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xae},
    {0x29,0xcc},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp2518_colorfx_none_regs[] =
{
    {0xfd,0x00},
    {0x62,0x00},
    {0x63,0x80},
    {0x64,0x80},
    ENDMARKER,
};

static struct regval_list sp2518_colorfx_bw_regs[] =
{
    {0xfd,0x00},
    {0x62,0x20},
    {0x63,0x80},
    {0x64,0x80},
    ENDMARKER,
};

static struct regval_list sp2518_colorfx_sepia_regs[] =
{
    {0xfd,0x00},
    {0x62,0x10},
    {0x63,0xb0},
    {0x64,0x40},
    ENDMARKER,
};

static struct regval_list sp2518_colorfx_negative_regs[] =
{
    {0xfd,0x00},
    {0x62,0x04},
    {0x63,0x80},
    {0x64,0x80},
    ENDMARKER,
};

static struct regval_list sp2518_whitebance_auto[] __attribute__((unused)) =
{
    {0xfd, 0x01},
    {0x28, 0xce},
    {0x29, 0x8a},
    {0xfd, 0x00},
    {0x32, 0x0d},
	ENDMARKER,
};

static struct regval_list sp2518_whitebance_cloudy[] __attribute__((unused)) =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xe2},
    {0x29,0x82},
	{0xfd, 0x00},
	ENDMARKER,
};

static  struct regval_list sp2518_whitebance_sunny[] __attribute__((unused)) =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xc1},
    {0x29,0x88},
	ENDMARKER,
};
/* Office Colour Temperature : 3500K - 5000K ,荧光灯 */
static  struct regval_list sp2518_whitebance_fluorescent[] __attribute__((unused)) =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xb4},
    {0x29,0xc4},
	ENDMARKER,

};
/* Home Colour Temperature : 2500K - 3500K ，白炽灯 */
static  struct regval_list sp2518_whitebance_incandescent[] __attribute__((unused)) =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x7b},
    {0x29,0xd3},
	ENDMARKER,
};

/*正常模式*/
static  struct regval_list sp2518_effect_normal[] __attribute__((unused)) =
{
	{0xfd,0x00},
	{0x62,0x00},
	{0x63,0x80},
	{0x64,0x80},
    ENDMARKER,
};
/*单色，黑白照片*/
static  struct regval_list sp2518_effect_white_black[] __attribute__((unused)) =
{
	{0xfd,0x00},
	{0x62,0x20},
	{0x63,0x80},
	{0x64,0x80},
  ENDMARKER,
};

/* Effect */
static  struct regval_list sp2518_effect_negative[] __attribute__((unused)) =
{
    //Negative
	{0xfd,0x00},
	{0x62,0x04},
	{0x63,0x80},
	{0x64,0x80},
	ENDMARKER,
};
/*复古效果*/
static  struct regval_list sp2518_effect_antique[] __attribute__((unused)) =
{

	{0xfd,0x00},
	{0x62,0x10},
	{0x63,0xa8},
	{0x64,0x58},
	ENDMARKER,
};

/* Scene */
static  struct regval_list sp2518_scene_auto[] __attribute__((unused)) =
{
	{0xfd,0x00},
	{0xb2,0x10},
	{0xb3,0x1f},
	ENDMARKER,
};

static  struct regval_list sp2518_scene_night[] __attribute__((unused)) =
{
	{0xfd,0x00},
	{0xb2,0x20},
	{0xb3,0x1f},
	ENDMARKER,
};

/*
 * register setting for window size
 */
// mediatech
static const struct regval_list sp2518_svga_init_regs[] =
{
	{0xfd,0x00},
	{0x1b,0x1a},//maximum drv ability
	{0x0e,0x01},
	{0x0f,0x2f},
	{0x10,0x2e},
	{0x11,0x00},
	{0x12,0x4f},
	{0x14,0x40},//20
	{0x16,0x02},
	{0x17,0x10},
	{0x1a,0x1f},
	{0x1e,0x81},
	{0x21,0x00},
	{0x22,0x1b},
	{0x25,0x10},
	{0x26,0x25},
	{0x27,0x6d},
	{0x2c,0x23},//31 Ronlus remove balck dot0x45},
	{0x2d,0x75},
	{0x2e,0x38},//sxga 0x18
    // psw0523 fix
#ifndef CONFIG_VIDEO_SP2518_FIXED_FRAMERATE
	{0x31,0x10},//mirror upside down
    // fix for 30frame
	/* {0x31,0x00},//mirror upside down */
#else
	{0x31,0x18},//mirror upside down
#endif // CONFIG_VIDEO_SP2518_FIXED_FRAMERATE
    // end psw0523
    // psw0523 add for 656
    /* {0x36, 0x1f}, // bit1: ccir656 output enable */
    // end psw0523
	{0x44,0x03},
	{0x6f,0x00},
	{0xa0,0x04},
	{0x5f,0x01},
	{0x32,0x00},
	{0xfd,0x01},
	{0x2c,0x00},
	{0x2d,0x00},
	{0xfd,0x00},
	{0xfb,0x83},
	{0xf4,0x09},
	//Pregain
	{0xfd,0x01},
	{0xc6,0x90},
	{0xc7,0x90},
	{0xc8,0x90},
	{0xc9,0x90},
	//blacklevel
	{0xfd,0x00},
	{0x65,0x08},
	{0x66,0x08},
	{0x67,0x08},
	{0x68,0x08},

	//bpc
	{0x46,0xff},
	//rpc
	{0xfd,0x00},
	{0xe0,0x6c},
	{0xe1,0x54},
	{0xe2,0x48},
	{0xe3,0x40},
	{0xe4,0x40},
	{0xe5,0x3e},
	{0xe6,0x3e},
	{0xe8,0x3a},
	{0xe9,0x3a},
	{0xea,0x3a},
	{0xeb,0x38},
	{0xf5,0x38},
	{0xf6,0x38},
	{0xfd,0x01},
	{0x94,0xcC},//f8 C0
	{0x95,0x38},
	{0x9c,0x74},//6C
	{0x9d,0x38},
#ifndef CONFIG_VIDEO_SP2518_FIXED_FRAMERATE
	/*24*3pll 8~13fps 50hz*/
	{0xfd , 0x00},
	{0x03 , 0x03},
	{0x04 , 0xf6},
	{0x05 , 0x00},
	{0x06 , 0x00},
	{0x07 , 0x00},
	{0x08 , 0x00},
	{0x09 , 0x00},
	{0x0a , 0x8b},
	{0x2f , 0x00},
	{0x30 , 0x08},
	{0xf0 , 0xa9},
	{0xf1 , 0x00},
	{0xfd , 0x01},
	{0x90 , 0x0c},
	{0x92 , 0x01},
	{0x98 , 0xa9},
	{0x99 , 0x00},
	{0x9a , 0x01},
	{0x9b , 0x00},
	//Status
	{0xfd , 0x01},
	{0xce , 0xec},
	{0xcf , 0x07},
	{0xd0 , 0xec},
	{0xd1 , 0x07},
	{0xd7 , 0xab},
	{0xd8 , 0x00},
	{0xd9 , 0xaf},
	{0xda , 0x00},
	{0xfd , 0x00},


	{0xfd , 0x00},
	{0x03 , 0x07},
	{0x04 , 0x9e},
	{0x05 , 0x00},
	{0x06 , 0x00},
	{0x07 , 0x00},
	{0x08 , 0x00},
	{0x09 , 0x00},
	{0x0a , 0xd4},
	{0x2f , 0x00},
	{0x30 , 0x0c},
	{0xf0 , 0x45},
	{0xf1 , 0x01},
	{0xfd , 0x01},
	{0x90 , 0x04},
	{0x92 , 0x01},
	{0x98 , 0x45},
	{0x99 , 0x01},
	{0x9a , 0x01},
	{0x9b , 0x00},
	//Status
	{0xfd , 0x01},
	{0xce , 0x14},
	{0xcf , 0x05},
	{0xd0 , 0x14},
	{0xd1 , 0x05},
	{0xd7 , 0x41},
	{0xd8 , 0x01},
	{0xd9 , 0x45},
	{0xda , 0x01},
	{0xfd , 0x00},
#else
    /* 24M Fixed 10frame */
	{0xfd , 0x00},
	{0x03 , 0x03},
	{0x04 , 0x0C},
	{0x05 , 0x00},
	{0x06 , 0x00},
	{0x07 , 0x00},
	{0x08 , 0x00},
	{0x09 , 0x00},
	{0x0a , 0xE4},
	{0x2f , 0x00},
	{0x30 , 0x11},
	{0xf0 , 0x82},
	{0xf1 , 0x00},
	{0xfd , 0x01},
	{0x90 , 0x0A},
	{0x92 , 0x01},
	{0x98 , 0x82},
	{0x99 , 0x01},
	{0x9a , 0x01},
	{0x9b , 0x00},
	//Status
	{0xfd , 0x01},
	{0xce , 0x14},
	{0xcf , 0x05},
	{0xd0 , 0x14},
	{0xd1 , 0x05},
	{0xd7 , 0x7E},
	{0xd8 , 0x00},
	{0xd9 , 0x82},
	{0xda , 0x00},
	{0xfd , 0x00},
#endif /* CONFIG_VIDEO_SP2518_FIXED_FRAMERATE */

	{0xfd,0x01},
	{0xca,0x30},//mean dummy2low
	{0xcb,0x50},//mean low2dummy
	{0xcc,0xc0},//f8,rpc low
	{0xcd,0xc0},//rpc dummy
	{0xd5,0x80},//mean normal2dummy
	{0xd6,0x90},//mean dummy2normal
	{0xfd,0x00},
	//lens shading for 舜泰979C-171A\181A
	{0xfd,0x00},
	{0xa1,0x20},
	{0xa2,0x20},
	{0xa3,0x20},
	{0xa4,0xff},
	{0xa5,0x80},
	{0xa6,0x80},
	{0xfd,0x01},
	{0x64,0x1e},//28
	{0x65,0x1c},//25
	{0x66,0x1c},//2a
	{0x67,0x16},//25
	{0x68,0x1c},//25
	{0x69,0x1c},//29
	{0x6a,0x1a},//28
	{0x6b,0x16},//20
	{0x6c,0x1a},//22
	{0x6d,0x1a},//22
	{0x6e,0x1a},//22
	{0x6f,0x16},//1c
	{0xb8,0x04},//0a
	{0xb9,0x13},//0a
	{0xba,0x00},//23
	{0xbb,0x03},//14
	{0xbc,0x03},//08
	{0xbd,0x11},//08
	{0xbe,0x00},//12
	{0xbf,0x02},//00
	{0xc0,0x04},//05
	{0xc1,0x0e},//05
	{0xc2,0x00},//18
	{0xc3,0x05},//08
	//raw filter
	{0xfd,0x01},
	{0xde,0x0f},
	{0xfd,0x00},
	{0x57,0x08},//raw_dif_thr
	{0x58,0x08},//a
	{0x56,0x08},//a
	{0x59,0x10},
	//R\B通道间平滑
	{0x5a,0xa0},//raw_rb_fac_outdoor
	{0xc4,0xa0},//60raw_rb_fac_indoor
	{0x43,0xa0},//40raw_rb_fac_dummy
	{0xad,0x40},//raw_rb_fac_low
	//Gr、Gb 通道内部平滑
	{0x4f,0xa0},//raw_gf_fac_outdoor
	{0xc3,0xa0},//60raw_gf_fac_indoor
	{0x3f,0xa0},//40raw_gf_fac_dummy
	{0x42,0x40},//raw_gf_fac_low
	{0xc2,0x15},
	//Gr、Gb通道间平滑
	{0xb6,0x80},//raw_gflt_fac_outdoor
	{0xb7,0x80},//60raw_gflt_fac_normal
	{0xb8,0x40},//40raw_gflt_fac_dummy
	{0xb9,0x20},//raw_gflt_fac_low
	//Gr、Gb通道阈值
	{0xfd,0x01},
	{0x50,0x0c},//raw_grgb_thr
	{0x51,0x0c},
	{0x52,0x10},
	{0x53,0x10},
	{0xfd,0x00},
	// awb1
	{0xfd,0x01},
	{0x11,0x10},
	{0x12,0x1f},
	{0x16,0x1c},
	{0x18,0x00},
	{0x19,0x00},
	{0x1b,0x96},
	{0x1a,0x9a},//95
	{0x1e,0x2f},
	{0x1f,0x29},
	{0x20,0xff},
	{0x22,0xff},
	{0x28,0xce},
	{0x29,0x8a},
	{0xfd,0x00},
	{0xe7,0x03},
	{0xe7,0x00},
	{0xfd,0x01},
	{0x2a,0xf0},
	{0x2b,0x10},
	{0x2e,0x04},
	{0x2f,0x18},
	{0x21,0x60},
	{0x23,0x60},
	{0x8b,0xab},
	{0x8f,0x12},
	//awb2
	{0xfd,0x01},
	{0x1a,0x80},
	{0x1b,0x80},
	{0x43,0x80},
	 //outdoor
    {0x00,0xd4},
    {0x01,0xb0},
    {0x02,0x90},
    {0x03,0x78},
	//d65
	{0x35,0xd6},//d6,b0
	{0x36,0xf0},//f0,d1,e9
	{0x37,0x7a},//8a,70
	{0x38,0x9a},//dc,9a,af
	//indoor
	{0x39,0xab},
	{0x3a,0xca},
	{0x3b,0xa3},
	{0x3c,0xc1},
	//f
	{0x31,0x82},//7d
	{0x32,0xa5},//a0,74
	{0x33,0xd6},//d2
	{0x34,0xec},//e8
	{0x3d,0xa5},//a7,88
	{0x3e,0xc2},//be,bb
	{0x3f,0xa7},//b3,ad
	{0x40,0xc5},//c5,d0
	//Color Correction
	{0xfd,0x01},
	{0x1c,0xc0},
	{0x1d,0x95},
	{0xa0,0xa6},//b8
	{0xa1,0xda},//,d5
	{0xa2,0x00},//,f2
	{0xa3,0x06},//,e8
	{0xa4,0xb2},//,95
	{0xa5,0xc7},//,03
	{0xa6,0x00},//,f2
	{0xa7,0xce},//,c4
	{0xa8,0xb2},//,ca
	{0xa9,0x0c},//,3c
	{0xaa,0x30},//,03
	{0xab,0x0c},//,0f
	{0xac,0xc0},//b8
	{0xad,0xc0},//d5
	{0xae,0x00},//f2
	{0xaf,0xf2},//e8
	{0xb0,0xa6},//95
	{0xb1,0xe8},//03
	{0xb2,0x00},//f2
	{0xb3,0xe7},//c4
	{0xb4,0x99},//ca
	{0xb5,0x0c},//3c
	{0xb6,0x33},//03
	{0xb7,0x0c},//0f
	//Saturation
	{0xfd,0x00},
	{0xbf,0x01},
	{0xbe,0xbb},
	{0xc0,0xb0},
	{0xc1,0xf0},
	{0xd3,0x77},
	{0xd4,0x77},
	{0xd6,0x77},
	{0xd7,0x77},
	{0xd8,0x77},
	{0xd9,0x77},
	{0xda,0x77},
	{0xdb,0x77},
	//uv_dif
	{0xfd,0x00},
	{0xf3,0x03},
	{0xb0,0x00},
	{0xb1,0x23},
	//gamma1
	{0xfd,0x00},//
	{0x8b,0x0 },//0 ,0
	{0x8c,0xA },//14,A
	{0x8d,0x13},//24,13
	{0x8e,0x25},//3a,25
	{0x8f,0x43},//59,43
	{0x90,0x5D},//6f,5D
	{0x91,0x74},//84,74
	{0x92,0x88},//95,88
	{0x93,0x9A},//a3,9A
	{0x94,0xA9},//b1,A9
	{0x95,0xB5},//be,B5
	{0x96,0xC0},//c7,C0
	{0x97,0xCA},//d1,CA
	{0x98,0xD4},//d9,D4
	{0x99,0xDD},//e1,DD
	{0x9a,0xE6},//e9,E6
	{0x9b,0xEF},//f1,EF
	{0xfd,0x01},//01,01
	{0x8d,0xF7},//f9,F7
	{0x8e,0xFF},//ff,FF
	//gamma2
	{0xfd,0x00},//
	{0x78,0x0 },//0
	{0x79,0xA },//14
	{0x7a,0x13},//24
	{0x7b,0x25},//3a
	{0x7c,0x43},//59
	{0x7d,0x5D},//6f
	{0x7e,0x74},//84
	{0x7f,0x88},//95
	{0x80,0x9A},//a3
	{0x81,0xA9},//b1
	{0x82,0xB5},//be
	{0x83,0xC0},//c7
	{0x84,0xCA},//d1
	{0x85,0xD4},//d9
	{0x86,0xDD},//e1
	{0x87,0xE6},//e9
	{0x88,0xEF},//f1
	{0x89,0xF7},//f9
	{0x8a,0xFF},//ff
      /*//光斑过度好
 //gamma1
	{0xfd,0x00},
	{0x8b,0x00},
	{0x8c,0x14},
	{0x8d,0x24},
	{0x8e,0x3A},
	{0x8f,0x59},
	{0x90,0x70},
	{0x91,0x85},
	{0x92,0x96},
	{0x93,0xA6},
	{0x94,0xB3},
	{0x95,0xBE},
	{0x96,0xC9},
	{0x97,0xD2},
	{0x98,0xDB},
	{0x99,0xE3},
	{0x9a,0xEB},
	{0x9b,0xF2},
	{0xfd,0x01},
	{0x8d,0xF9},
	{0x8e,0xFF},
//gamma2
	{0xfd,0x00},
	{0x78,0x00},
	{0x79,0x14},
	{0x7a,0x24},
	{0x7b,0x3A},
	{0x7c,0x59},
	{0x7d,0x70},
	{0x7e,0x85},
	{0x7f,0x96},
	{0x80,0xA6},
	{0x81,0xB3},
	{0x82,0xBE},
	{0x83,0xC9},
	{0x84,0xD2},
	{0x85,0xDB},
	{0x86,0xE3},
	{0x87,0xEB},
	{0x88,0xF2},
	{0x89,0xF9},
	{0x8a,0xFF},
*/
	//gamma_ae
	{0xfd,0x01},
	{0x96,0x46},
	{0x97,0x14},
	{0x9f,0x06},
	//HEQ
	{0xfd,0x00},//
	{0xdd,0x80},//
	{0xde,0x95},//a0
	{0xdf,0x80},//
	//Ytarget
	{0xfd,0x00},//
	{0xec,0x70},//6a
	{0xed,0x86},//7c
	{0xee,0x70},//65
	{0xef,0x86},//78
	{0xf7,0x80},//78
	{0xf8,0x74},//6e
	{0xf9,0x80},//74
	{0xfa,0x74},//6a
	//sharpen
	{0xfd,0x01},
	{0xdf,0x0f},
	{0xe5,0x10},
	{0xe7,0x10},
	{0xe8,0x20},
	{0xec,0x20},
	{0xe9,0x20},
	{0xed,0x20},
	{0xea,0x10},
	{0xef,0x10},
	{0xeb,0x10},
	{0xf0,0x10},
	//,gw
	{0xfd,0x01},//
	{0x70,0x76},//
	{0x7b,0x40},//
	{0x81,0x30},//
	//,Y_offset
	{0xfd,0x00},
	{0xb2,0x10},
	{0xb3,0x1f},
	{0xb4,0x30},
	{0xb5,0x50},
	//,CNR
	{0xfd,0x00},
	{0x5b,0x20},
	{0x61,0x80},
	{0x77,0x80},
	{0xca,0x80},
	//,YNR
	{0xab,0x00},
	{0xac,0x02},
	{0xae,0x08},
	{0xaf,0x20},
	{0xfd,0x00},
	{0x31,0x10},
	{0x32,0x0d},
	{0x33,0xcf},//ef
	{0x34,0x7f},//3f
	{0xe7,0x03},
	{0xe7,0x00},

    // psw0523 test
#if 0
    {0xfd, 0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
    {0x4b, 0x00},
    {0x4c, 0x00},
    {0x47, 0x00},
    {0x48, 0x00},
    {0xfd, 0x01},
    {0x06, 0x00},
    {0x07, 0x40},
    {0x08, 0x00},
    {0x09, 0x40},
    {0x0a, 0x02},
    {0x0b, 0x58},
    {0x0c, 0x03},
    {0x0d, 0x20},
    {0x0e, 0x21},
    {0xfd, 0x00},
#endif

    ENDMARKER
};

/**
 * 1280x720
 */
static const struct regval_list sp2518_720P_regs[] = {
    {0xfd,0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
    {0x47,0x00},
    {0x48,0xf0},
    {0x49,0x02},
    {0x4a,0xd0},
    {0x4b,0x00},
    {0x4c,0xa0},
    {0x4d,0x05},
    {0x4e,0x00},
    {0xfd,0x01},
    {0x0e,0x00},
    {0xfd,0x00},

    ENDMARKER
};

#if 0
static const struct regval_list sp2518_svga_regs[] = {
    {0xfd, 0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
    {0x47, 0x00},
    {0x48, 0x00},
    {0x49, 0x02},
    {0x4a, 0x58},
    {0x4b, 0x00},
    {0x4c, 0x00},
#if 0
    {0x4d, 0x03},
    {0x4e, 0x20},
#else
    /*{0x4d, 0x06},*/
    /*{0x4e, 0x40},*/
#endif
    {0xfd, 0x01},
    {0x06, 0x00},
    {0x07, 0x40},
    {0x08, 0x00},
    {0x09, 0x40},
    {0x0a, 0x02},
    {0x0b, 0x58},
    {0x0c, 0x03},
    {0x0d, 0x20},
    {0x0e, 0x21},
    {0xfd, 0x00},
    ENDMARKER
};
#endif

/**
 * 720x480
 */
static const struct regval_list sp2518_480P_regs[] = {
    {0xfd,0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
#if 0
    {0x47,0x00},
    {0x48,0x00},
    {0x49,0x04},
    {0x4a,0xb0},
    {0x4b,0x00},
    {0x4c,0x00},
    {0x4d,0x06},
    {0x4e,0x40},
    {0xfd,0x01},
    {0x06,0x00},
    {0x07,0x50},
    {0x08,0x00},
    {0x09,0x47},
    {0x0a,0x01},
    {0x0b,0xe0},
    {0x0c,0x02},
    {0x0d,0xd0},
    {0x0e,0x01},
#endif
    {0x47,0x01},
    {0x48,0x68},
    {0x49,0x01},
    {0x4a,0xe0},
    {0x4b,0x01},
    {0x4c,0xb8},
    {0x4d,0x02},
    {0x4e,0xd0},
    {0xfd,0x01},
    {0x06,0x00},
    {0x07,0x25},
    {0x08,0x00},
    {0x09,0x28},
    {0x0a,0x04},
    {0x0b,0x00},
    {0x0c,0x05},
    {0x0d,0x00},
    {0x0e,0x00},
    {0xfd,0x00},

    ENDMARKER
};

/**
 * 704x480
 */
static const struct regval_list sp2518_704X480_regs[] = {
	{0xfd,0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable

	{0x47,0x01},
	{0x48,0x68},
	{0x49,0x01},
	{0x4a,0xe0},
	{0x4b,0x01},
	{0x4c,0xc0},
	{0x4d,0x02},
	{0x4e,0xc0},
	{0xfd,0x01},
	{0x06,0x00},
	{0x07,0x25},
	{0x08,0x00},
	{0x09,0x28},
	{0x0a,0x04},
	{0x0b,0x00},
	{0x0c,0x05},
	{0x0d,0x00},
    {0x0e,0x00},
    ENDMARKER
};

/**
 * 640x480
 */
static const struct regval_list sp2518_640X480_regs[] = {
	{0xfd,0x00},
    {0x36,0x1f}, // bit1: ccir656 output enable
	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},
	{0x4a,0xb0},
	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},
	{0xfd,0x01},
	{0x06,0x00},
	{0x07,0x50},
	{0x08,0x00},
	{0x09,0x50},
	{0x0a,0x01},
	{0x0b,0xe0},
	{0x0c,0x02},
	{0x0d,0x80},
	{0x0e,0x01},

    ENDMARKER
};

/**
 * 320x240
 */
static const struct regval_list sp2518_320X240_regs[] = {
	{0xfd,0x00},
    {0x36,0x1f}, // bit1: ccir656 output enable
	{0x47,0x01},
	{0x48,0xe0},
	{0x49,0x00},
	{0x4a,0xf0},
	{0x4b,0x02},
	{0x4c,0x80},
	{0x4d,0x01},
	{0x4e,0x40},
	{0xfd,0x01},
	{0x06,0x00},
	{0x07,0x25},
	{0x08,0x00},
	{0x09,0x28},
	{0x0a,0x04},
	{0x0b,0x00},
	{0x0c,0x05},
	{0x0d,0x00},
	{0x0e,0x00},

    ENDMARKER
};

/**
 * 352x288
 */
static const struct regval_list sp2518_CIF_regs[] = {
	{0xfd,0x00},
  {0x36,0x1f}, // bit1: ccir656 output enable
	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},
	{0x4a,0xb0},
	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},
	{0xfd,0x01},
	{0x06,0x00},
	{0x07,0x85},
	{0x08,0x00},
	{0x09,0x91},
	{0x0a,0x01},
	{0x0b,0x20},
	{0x0c,0x01},
	{0x0d,0x60},
	{0x0e,0x01},

    	ENDMARKER
};

/**
 * 176x144
 */
static const struct regval_list sp2518_QCIF_regs[] = {
	{0xfd,0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},
	{0x4a,0xb0},
	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},
	{0xfd,0x01},
	{0x06,0x01},
	{0x07,0x0a},
	{0x08,0x01},
	{0x09,0x22},
	{0x0a,0x00},
	{0x0b,0x90},
	{0x0c,0x00},
	{0x0d,0xb0},
	{0x0e,0x01},

    ENDMARKER
};

/**
 * 800x600
 */
#if 1
static const struct regval_list sp2518_svga_regs[] = {
    {0xfd, 0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
    {0x47, 0x00},
    {0x48, 0x00},
    {0x49, 0x04},
    {0x4a, 0xb0},
    {0x4b, 0x00},
    {0x4c, 0x00},
    {0x4d, 0x06},
    {0x4e, 0x40},
    {0xfd, 0x01},
    {0x06, 0x00},
    {0x07, 0x40},
    {0x08, 0x00},
    {0x09, 0x40},
    {0x0a, 0x02},
    {0x0b, 0x58},
    {0x0c, 0x03},
    {0x0d, 0x20},
    {0x0e, 0x21},
    {0xfd, 0x00},
    ENDMARKER
};
#else
static const struct regval_list sp2518_svga_regs[] = {
    {0xfd, 0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
    {0x47, 0x01},
    {0x48, 0x2c},
    {0x49, 0x02},
    {0x4a, 0x58},
    {0x4b, 0x01},
    {0x4c, 0x90},
    {0x4d, 0x03},
    {0x4e, 0x20},
    {0xfd, 0x01},
    {0x06, 0x00},
    {0x07, 0x40},
    {0x08, 0x00},
    {0x09, 0x40},
    {0x0a, 0x02},
    {0x0b, 0x58},
    {0x0c, 0x03},
    {0x0d, 0x20},
    {0x0e, 0x21},
    {0xfd, 0x00},
    ENDMARKER
};
#endif

/**
 * 1600x1200
 */
static const struct regval_list sp2518_uxga_regs[] = {//caichsh
    //Resolution Setting : 1600*1200
    {0xfd, 0x00},
    {0x36, 0x1f}, // bit1: ccir656 output enable
    {0x47, 0x00},
    {0x48, 0x00},
    {0x49, 0x04},
    {0x4a, 0xb0},
    {0x4b, 0x00},
    {0x4c, 0x00},
    {0x4d, 0x06},
    {0x4e, 0x40},
    {0xfd, 0x01},
    {0x0e, 0x00},
    {0x0f, 0x00},
    {0xfd, 0x00},
    ENDMARKER,
};

static const struct regval_list sp2518_disable_regs[] = {
    {0xfd, 0x00},
    {0x36, 0x00},
    {0xfd, 0x00},
    ENDMARKER,
};


/****************************************************************************************
 * structures
 */
struct sp2518_win_size {
    char                        *name;
    __u32                       width;
    __u32                       height;
    __u32                       exposure_line_width;
    __u32                       capture_maximum_shutter;
    const struct regval_list    *win_regs;
    const struct regval_list    *lsc_regs;
    unsigned int                *frame_rate_array;
};

typedef struct {
    unsigned int max_shutter;
    unsigned int shutter;
    unsigned int gain;
    unsigned int dummy_line;
    unsigned int dummy_pixel;
    unsigned int extra_line;
} exposure_param_t;

enum prev_capt {
    PREVIEW_MODE = 0,
    CAPTURE_MODE
};

struct sp2518_priv {
    struct v4l2_subdev                  subdev;
    struct media_pad                    pad;
    struct v4l2_ctrl_handler            hdl;
    const struct sp2518_color_format    *cfmt;
    const struct sp2518_win_size        *win;
    int                                 model;
    bool                                initialized;

    /**
     * ctrls
    */
    /* standard */
    struct v4l2_ctrl *brightness;
    struct v4l2_ctrl *contrast;
    struct v4l2_ctrl *auto_white_balance;
    struct v4l2_ctrl *exposure;
    struct v4l2_ctrl *gain;
    struct v4l2_ctrl *hflip;
    struct v4l2_ctrl *white_balance_temperature;
    /* menu */
    struct v4l2_ctrl *colorfx;
    struct v4l2_ctrl *exposure_auto;
    /* custom */
    struct v4l2_ctrl *scene_exposure;
    struct v4l2_ctrl *prev_capt;

    struct v4l2_rect rect; /* Sensor window */
    struct v4l2_fract timeperframe;
    enum prev_capt prev_capt_mode;
    exposure_param_t preview_exposure_param;
    exposure_param_t capture_exposure_param;
};

struct sp2518_color_format {
    enum v4l2_mbus_pixelcode code;
    enum v4l2_colorspace colorspace;
};

/****************************************************************************************
 * tables
 */
static const struct sp2518_color_format sp2518_cfmts[] = {
    {
        .code		= V4L2_MBUS_FMT_YUYV8_2X8,
        .colorspace	= V4L2_COLORSPACE_JPEG,
    },
    {
        .code		= V4L2_MBUS_FMT_UYVY8_2X8,
        .colorspace	= V4L2_COLORSPACE_JPEG,
    },
    {
        .code		= V4L2_MBUS_FMT_YVYU8_2X8,
        .colorspace	= V4L2_COLORSPACE_JPEG,
    },
    {
        .code		= V4L2_MBUS_FMT_VYUY8_2X8,
        .colorspace	= V4L2_COLORSPACE_JPEG,
    },
};

/*
 * window size list
 */
#define QCIF_WIDTH          176
#define QCIF_HEIGHT         144
#define CIF_WIDTH           352
#define CIF_HEIGHT          288
#define X480P_WIDTH         720
#define X480P_HEIGHT        480
#define VGA_WIDTH           640
#define VGA_HEIGHT          480
#define X720P_WIDTH         1280
#define X720P_HEIGHT        720
#define UXGA_WIDTH          1600
#define UXGA_HEIGHT         1200
#define SVGA_WIDTH          800
#define SVGA_HEIGHT         600
#define SP2518_MAX_WIDTH    UXGA_WIDTH
#define SP2518_MAX_HEIGHT   UXGA_HEIGHT
#define AHEAD_LINE_NUM		15    //10行 = 50次循环(sp2518)
#define DROP_NUM_CAPTURE	0
#define DROP_NUM_PREVIEW	0

static unsigned int frame_rate_svga[] = {12,};
static unsigned int frame_rate_uxga[] = {12,};

/* 1280X720 */
static const struct sp2518_win_size sp2518_win_720P = {
    .name     = "720P",
    .width    = X720P_WIDTH,
    .height   = X720P_HEIGHT,
    .win_regs = sp2518_720P_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 720X480 */
static const struct sp2518_win_size sp2518_win_480P = {
    .name     = "480P",
    .width    = X480P_WIDTH,
    .height   = X480P_HEIGHT,
    .win_regs = sp2518_480P_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 704X480 */
static const struct sp2518_win_size sp2518_win_704X480 = {
    .name     = "704X480",
    .width    = 704,
    .height   = 480,
    .win_regs = sp2518_704X480_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 640X480 */
static const struct sp2518_win_size sp2518_win_640X480 = {
    .name     = "640X480",
    .width    = VGA_WIDTH,
    .height   = VGA_HEIGHT,
    .win_regs = sp2518_640X480_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 320X240 */
static const struct sp2518_win_size sp2518_win_320X240 = {
    .name     = "CIF",
    .width    = 320,
    .height   = 240,
    .win_regs = sp2518_320X240_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 352X288 */
static const struct sp2518_win_size sp2518_win_CIF = {
    .name     = "CIF",
    .width    = CIF_WIDTH,
    .height   = CIF_HEIGHT,
    .win_regs = sp2518_CIF_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 176X144 */
static const struct sp2518_win_size sp2518_win_QCIF = {
    .name     = "QCIF",
    .width    = QCIF_WIDTH,
    .height   = QCIF_HEIGHT,
    .win_regs = sp2518_QCIF_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 800X600 */
static const struct sp2518_win_size sp2518_win_svga = {
    .name     = "SVGA",
    .width    = SVGA_WIDTH,
    .height   = SVGA_HEIGHT,
    .win_regs = sp2518_svga_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 1600X1200 */
static const struct sp2518_win_size sp2518_win_uxga = {
    .name     = "UXGA",
    .width    = UXGA_WIDTH,
    .height   = UXGA_HEIGHT,
    .win_regs = sp2518_uxga_regs,
    .frame_rate_array = frame_rate_uxga,
};

static const struct sp2518_win_size *sp2518_win[] = {
	&sp2518_win_720P,
	&sp2518_win_480P,
	&sp2518_win_704X480,
	&sp2518_win_640X480,
	&sp2518_win_CIF,
	&sp2518_win_320X240,
	&sp2518_win_QCIF,
	&sp2518_win_svga,
    &sp2518_win_uxga,
};

/****************************************************************************************
 * general functions
 */
static inline struct sp2518_priv *to_priv(struct v4l2_subdev *subdev)
{
    return container_of(subdev, struct sp2518_priv, subdev);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct sp2518_priv, hdl)->subdev;
}

static bool check_id(struct i2c_client *client)
{
    u8 pid = i2c_smbus_read_byte_data(client, PID);
    if (pid == SP2518)
        return true;

    printk(KERN_ERR "failed to check id: 0x%x\n", pid);
    return false;
}

static int sp2518_write_array(struct i2c_client *client, const struct regval_list *vals)
{
    int ret;
    while (vals->reg_num != 0xff) {
        ret = i2c_smbus_write_byte_data(client, vals->reg_num, vals->value);
        if (ret < 0)
            return ret;
        vals++;
    }
    return 0;
}

static int sp2518_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set) __attribute__((unused));
static int sp2518_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set)
{
    s32 val = i2c_smbus_read_byte_data(client, command);
    if (val < 0)
        return val;

    val &= ~mask;
    val |= set & mask;

    return i2c_smbus_write_byte_data(client, command, val);
}

static const struct sp2518_win_size *sp2518_select_win(u32 width, u32 height)
{
	const struct sp2518_win_size *win;
    int i;

    for (i = 0; i < ARRAY_SIZE(sp2518_win); i++) {
        win = sp2518_win[i];
        if (width == win->width && height == win->height)
            return win;
    }

    printk(KERN_ERR "%s: unsupported width, height (%dx%d)\n", __func__, width, height);
    return NULL;
}

static int sp2518_set_mbusformat(struct i2c_client *client, const struct sp2518_color_format *cfmt)
{
    enum v4l2_mbus_pixelcode code;
    int ret = -1;
    code = cfmt->code;
    switch (code) {
        case V4L2_MBUS_FMT_YUYV8_2X8:
            ret  = sp2518_write_array(client, sp2518_fmt_yuv422_yuyv);
            break;
        case V4L2_MBUS_FMT_UYVY8_2X8:
            ret  = sp2518_write_array(client, sp2518_fmt_yuv422_uyvy);
            break;
        case V4L2_MBUS_FMT_YVYU8_2X8:
            ret  = sp2518_write_array(client, sp2518_fmt_yuv422_yvyu);
            break;
        case V4L2_MBUS_FMT_VYUY8_2X8:
            ret  = sp2518_write_array(client, sp2518_fmt_yuv422_vyuy);
            break;
        default:
            printk(KERN_ERR "mbus code error in %s() line %d\n",__FUNCTION__, __LINE__);
    }
    return ret;
}

static int sp2518_set_params(struct v4l2_subdev *sd, u32 *width, u32 *height, enum v4l2_mbus_pixelcode code)
{
    struct sp2518_priv *priv = to_priv(sd);
    const struct sp2518_win_size *old_win, *new_win;
    int i;

    /*
     * select format
     */
    priv->cfmt = NULL;
    for (i = 0; i < ARRAY_SIZE(sp2518_cfmts); i++) {
        if (code == sp2518_cfmts[i].code) {
            priv->cfmt = sp2518_cfmts + i;
            break;
        }
    }
    if (!priv->cfmt) {
        printk(KERN_ERR "Unsupported sensor format.\n");
        return -EINVAL;
    }

    /*
     * select win
     */
    old_win = priv->win;
    new_win = sp2518_select_win(*width, *height);
    if (!new_win) {
        printk(KERN_ERR "Unsupported win size\n");
        return -EINVAL;
    }
    priv->win = new_win;

    priv->rect.left = 0;
    priv->rect.top = 0;
    priv->rect.width = priv->win->width;
    priv->rect.height = priv->win->height;

    *width = priv->win->width;
    *height = priv->win->height;

    return 0;
}

/****************************************************************************************
 * control functions
 */
static int sp2518_set_brightness(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned int reg_0xb5, reg_0xd3;
    int ret;

    int val = ctrl->val;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val > 6) {
        dev_err(&client->dev, "set brightness over range, brightness = %d\n", val);
        return -ERANGE;
    }

    switch(val) {
        case 0:
            reg_0xb5 = 0xd0;
            reg_0xd3 = 0x68;
            break;
        case 1:
            reg_0xb5 = 0xe0;
            reg_0xd3 = 0x70;
            break;
        case 2:
            reg_0xb5 = 0xf0;
            reg_0xd3 = 0x78;
            break;
        case 3:
            reg_0xb5 = 0x10;
            reg_0xd3 = 0x88;//80
            break;
        case 4:
            reg_0xb5 = 0x20;
            reg_0xd3 = 0x88;
            break;
        case 5:
            reg_0xb5 = 0x30;
            reg_0xd3 = 0x90;
            break;
        case 6:
            reg_0xb5 = 0x40;
            reg_0xd3 = 0x98;
            break;
    }

    ret = i2c_smbus_write_byte_data(client, 0xb5, reg_0xb5);
    ret |= i2c_smbus_write_byte_data(client, 0xd3, reg_0xd3);
    assert(ret == 0);

    return 0;
}

static int sp2518_set_contrast(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* TODO */
    int contrast = ctrl->val;
    printk("%s: val %d\n", __func__, contrast);

    return 0;
}

static int sp2518_set_auto_white_balance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct sp2518_priv *priv = to_priv(sd); */
    int auto_white_balance = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, auto_white_balance);
    if (auto_white_balance < 0 || auto_white_balance > 1) {
        dev_err(&client->dev, "set auto_white_balance over range, auto_white_balance = %d\n", auto_white_balance);
        return -ERANGE;
    }

    switch(auto_white_balance) {
        case 0:
            SP2518_DEBUG(KERN_ERR "===awb disable===\n");
            ret = sp2518_write_array(client, sp2518_awb_regs_diable);
            break;
        case 1:
            SP2518_DEBUG(KERN_ERR "===awb enable===\n");
            ret = sp2518_write_array(client, sp2518_awb_regs_enable);
            break;
    }

    assert(ret == 0);

    return 0;
}

/* TODO : exposure */
static int sp2518_set_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* struct sp2518_priv *priv = to_priv(sd); */
    printk("%s: val %d\n", __func__, ctrl->val);
#if 0
    unsigned int reg_0x13, reg_0x02, reg_0x03;
    int ret;

    if (exposure < 0 || exposure > 0xFFFFU) {
        dev_err(&client->dev, "set exposure over range, exposure = %d\n", exposure);
        return -ERANGE;
    }

    reg_0x13 = i2c_smbus_read_byte_data(client, 0x13);
    assert(reg_0x13 >= 0);
    reg_0x13 &= ~(1U << 1); // AUTO1[1]: AEC_en

    reg_0x03 = exposure & 0xFFU;
    reg_0x02 = (exposure >> 8) & 0xFFU;

    ret = i2c_smbus_write_byte_data(client, 0x13, reg_0x13);
    ret |= i2c_smbus_write_byte_data(client, 0x02, reg_0x02);
    ret |= i2c_smbus_write_byte_data(client, 0x03, reg_0x03);
    assert(ret == 0);

    priv->exposure_auto = 0;
    priv->exposure = exposure;
    ctrl->cur.val = exposure;
#endif

    return 0;
}

/* TODO */
static int sp2518_set_gain(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* struct sp2518_priv *priv = to_priv(sd); */
    printk("%s: val %d\n", __func__, ctrl->val);
    return 0;
}

/* TODO */
static int sp2518_set_hflip(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* struct sp2518_priv *priv = to_priv(sd); */
    printk("%s: val %d\n", __func__, ctrl->val);
    return 0;
}

static int sp2518_set_white_balance_temperature(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct sp2518_priv *priv = to_priv(sd); */
    int white_balance_temperature = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch(white_balance_temperature) {
        case V4L2_WHITE_BALANCE_INCANDESCENT:
            ret = sp2518_write_array(client, sp2518_wb_incandescence_regs);
            break;
        case V4L2_WHITE_BALANCE_FLUORESCENT:
            ret = sp2518_write_array(client, sp2518_wb_fluorescent_regs);
            break;
        case V4L2_WHITE_BALANCE_DAYLIGHT:
            ret = sp2518_write_array(client, sp2518_wb_daylight_regs);
            break;
        case V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT:
            ret = sp2518_write_array(client, sp2518_wb_cloud_regs);
            break;
        case V4L2_WHITE_BALANCE_TUNGSTEN:
            ret = sp2518_write_array(client, sp2518_wb_tungsten_regs);
            break;
        default:
            dev_err(&client->dev, "set white_balance_temperature over range, white_balance_temperature = %d\n", white_balance_temperature);
            return -ERANGE;
    }

    assert(ret == 0);

    return 0;
}

static int sp2518_set_colorfx(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct sp2518_priv *priv = to_priv(sd); */
    int colorfx = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch (colorfx) {
        case V4L2_COLORFX_NONE: /* normal */
            ret = sp2518_write_array(client, sp2518_colorfx_none_regs);
            break;
        case V4L2_COLORFX_BW: /* black and white */
            ret = sp2518_write_array(client, sp2518_colorfx_bw_regs);
            break;
        case V4L2_COLORFX_SEPIA: /* antique ,复古*/
            ret = sp2518_write_array(client, sp2518_colorfx_sepia_regs);
            break;
        case V4L2_COLORFX_NEGATIVE: /* negative，负片 */
            ret = sp2518_write_array(client, sp2518_colorfx_negative_regs);
            break;
        default:
            dev_err(&client->dev, "set colorfx over range, colorfx = %d\n", colorfx);
            return -ERANGE;
    }

    assert(ret == 0);
    return 0;
}

static int sp2518_set_exposure_auto(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct sp2518_priv *priv = to_priv(sd); */
    int exposure_auto = ctrl->val;

    /* unsigned int reg_0xec; */
    /* int ret; */

    printk("%s: val %d\n", __func__, ctrl->val);

    if (exposure_auto < 0 || exposure_auto > 1) {
        dev_err(&client->dev, "set exposure_auto over range, exposure_auto = %d\n", exposure_auto);
        return -ERANGE;
    }

    return 0;
}

/* TODO */
static int sp2518_set_scene_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
#if 0
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp2518_priv *priv = to_priv(sd);
    int scene_exposure = ctrl->val;

    unsigned int reg_0xec;
    int ret;

    switch(scene_exposure) {
        case V4L2_SCENE_MODE_HOUSE:  //室内
            reg_0xec = 0x30;
            break;
        case V4L2_SCENE_MODE_SUNSET:  //室外
            reg_0xec = 0x20;
            break;
        default:
            dev_err(&client->dev, "set scene_exposure over range, scene_exposure = %d\n", scene_exposure);
            return -ERANGE;
    }

    ret = i2c_smbus_write_byte_data(client, 0xec, reg_0xec);
    assert(ret == 0);
#endif

    printk("%s: val %d\n", __func__, ctrl->val);
    return 0;
}

static int sp2518_set_prev_capt_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp2518_priv *priv = to_priv(sd);
    printk("%s: val %d\n", __func__, ctrl->val);

    switch(ctrl->val) {
        case PREVIEW_MODE:
            priv->prev_capt_mode = ctrl->val;
            break;
        case CAPTURE_MODE:
            priv->prev_capt_mode = ctrl->val;
            break;
        default:
            dev_err(&client->dev, "set_prev_capt_mode over range, prev_capt_mode = %d\n", ctrl->val);
            return -ERANGE;
    }

    return 0;
}

static int sp2518_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret = 0;

    switch (ctrl->id) {
        case V4L2_CID_BRIGHTNESS:
            sp2518_set_brightness(sd, ctrl);
            break;

        case V4L2_CID_CONTRAST:
            sp2518_set_contrast(sd, ctrl);
            break;

        case V4L2_CID_AUTO_WHITE_BALANCE:
            sp2518_set_auto_white_balance(sd, ctrl);
            break;

        case V4L2_CID_EXPOSURE:
            sp2518_set_exposure(sd, ctrl);
            break;

        case V4L2_CID_GAIN:
            sp2518_set_gain(sd, ctrl);
            break;

        case V4L2_CID_HFLIP:
            sp2518_set_hflip(sd, ctrl);
            break;

        case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
            sp2518_set_white_balance_temperature(sd, ctrl);
            break;

        case V4L2_CID_COLORFX:
            sp2518_set_colorfx(sd, ctrl);
            break;

        case V4L2_CID_EXPOSURE_AUTO:
            sp2518_set_exposure_auto(sd, ctrl);
            break;

        case V4L2_CID_SCENE_EXPOSURE:
            sp2518_set_scene_exposure(sd, ctrl);
            break;

        case V4L2_CID_PRIVATE_PREV_CAPT:
            sp2518_set_prev_capt_mode(sd, ctrl);
            break;

        default:
            dev_err(&client->dev, "%s: invalid control id %d\n", __func__, ctrl->id);
            return -EINVAL;
    }

    return ret;
}

static const struct v4l2_ctrl_ops sp2518_ctrl_ops = {
    .s_ctrl = sp2518_s_ctrl,
};

static const struct v4l2_ctrl_config sp2518_custom_ctrls[] = {
    {
        .ops    = &sp2518_ctrl_ops,
        .id     = V4L2_CID_SCENE_EXPOSURE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "SceneExposure",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &sp2518_ctrl_ops,
        .id     = V4L2_CID_PRIVATE_PREV_CAPT,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "PrevCapt",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }
};

static int sp2518_initialize_ctrls(struct sp2518_priv *priv)
{
    v4l2_ctrl_handler_init(&priv->hdl, NUM_CTRLS);

    /* standard ctrls */
    priv->brightness = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_BRIGHTNESS, 0, 6, 1, 0);
    if (!priv->brightness) {
        printk(KERN_ERR "%s: failed to create brightness ctrl\n", __func__);
        return -ENOENT;
    }

    priv->contrast = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_CONTRAST, -6, 6, 1, 0);
    if (!priv->contrast) {
        printk(KERN_ERR "%s: failed to create contrast ctrl\n", __func__);
        return -ENOENT;
    }

    priv->auto_white_balance = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
    if (!priv->auto_white_balance) {
        printk(KERN_ERR "%s: failed to create auto_white_balance ctrl\n", __func__);
        return -ENOENT;
    }

#if 0
    priv->exposure = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_EXPOSURE, 0, 0xFFFF, 1, 500);
    if (!priv->exposure) {
        printk(KERN_ERR "%s: failed to create exposure ctrl\n", __func__);
        return -ENOENT;
    }
#endif

    priv->gain = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_GAIN, 0, 0xFF, 1, 128);
    if (!priv->gain) {
        printk(KERN_ERR "%s: failed to create gain ctrl\n", __func__);
        return -ENOENT;
    }

    priv->hflip = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_HFLIP, 0, 1, 1, 0);
    if (!priv->hflip) {
        printk(KERN_ERR "%s: failed to create hflip ctrl\n", __func__);
        return -ENOENT;
    }

    priv->white_balance_temperature = v4l2_ctrl_new_std(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 3, 1, 1);
    if (!priv->white_balance_temperature) {
        printk(KERN_ERR "%s: failed to create white_balance_temperature ctrl\n", __func__);
        return -ENOENT;
    }

    /* standard menus */
    priv->colorfx = v4l2_ctrl_new_std_menu(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_COLORFX, 3, 0, 0);
    if (!priv->colorfx) {
        printk(KERN_ERR "%s: failed to create colorfx ctrl\n", __func__);
        return -ENOENT;
    }

    priv->exposure_auto = v4l2_ctrl_new_std_menu(&priv->hdl, &sp2518_ctrl_ops,
            V4L2_CID_EXPOSURE_AUTO, 1, 0, 1);
    if (!priv->exposure_auto) {
        printk(KERN_ERR "%s: failed to create exposure_auto ctrl\n", __func__);
        return -ENOENT;
    }

    /* custom ctrls */
    priv->scene_exposure = v4l2_ctrl_new_custom(&priv->hdl, &sp2518_custom_ctrls[0], NULL);
    if (!priv->scene_exposure) {
        printk(KERN_ERR "%s: failed to create scene_exposure ctrl\n", __func__);
        return -ENOENT;
    }

    priv->prev_capt = v4l2_ctrl_new_custom(&priv->hdl, &sp2518_custom_ctrls[1], NULL);
    if (!priv->prev_capt) {
        printk(KERN_ERR "%s: failed to create prev_capt ctrl\n", __func__);
        return -ENOENT;
    }

    priv->subdev.ctrl_handler = &priv->hdl;
    if (priv->hdl.error) {
        printk(KERN_ERR "%s: ctrl handler error(%d)\n", __func__, priv->hdl.error);
        v4l2_ctrl_handler_free(&priv->hdl);
        return -EINVAL;
    }

    return 0;
}

static int sp2518_save_exposure_param(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp2518_priv *priv = to_priv(sd);
    int ret = 0;
    unsigned int reg_0x03 = 0x20;
    unsigned int reg_0x80;
    unsigned int reg_0x81;
    unsigned int reg_0x82;

    i2c_smbus_write_byte_data(client, 0x03, reg_0x03); //page 20
    reg_0x80 = i2c_smbus_read_byte_data(client, 0x80);
    reg_0x81 = i2c_smbus_read_byte_data(client, 0x81);
    reg_0x82 = i2c_smbus_read_byte_data(client, 0x82);

    priv->preview_exposure_param.shutter = (reg_0x80 << 16)|(reg_0x81 << 8)|reg_0x82;
    priv->capture_exposure_param.shutter = (priv->preview_exposure_param.shutter)/2;

    return ret;
}

static int sp2518_set_exposure_param(struct v4l2_subdev *sd) __attribute__((unused));
static int sp2518_set_exposure_param(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp2518_priv *priv = to_priv(sd);
    int ret;
    unsigned int reg_0x03 = 0x20;
    unsigned int reg_0x83;
    unsigned int reg_0x84;
    unsigned int reg_0x85;

    if(priv->capture_exposure_param.shutter < 1)
        priv->capture_exposure_param.shutter = 1;

    reg_0x83 = (priv->capture_exposure_param.shutter)>>16;
    reg_0x84 = ((priv->capture_exposure_param.shutter)>>8) & 0x000000FF;
    reg_0x85 = (priv->capture_exposure_param.shutter) & 0x000000FF;

    ret = i2c_smbus_write_byte_data(client, 0x03, reg_0x03); //page 20
    ret |= i2c_smbus_write_byte_data(client, 0x83, reg_0x83);
    ret |= i2c_smbus_write_byte_data(client, 0x84, reg_0x84);
    ret |= i2c_smbus_write_byte_data(client, 0x85, reg_0x85);

    return 0;
}

/****************************************************************************************
 * v4l2 subdev ops
 */

/**
 * core ops
 */
static int sp2518_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *id)
{
    struct sp2518_priv *priv = to_priv(sd);
    id->ident    = priv->model;
    id->revision = 0;
    return 0;
}

static int sp2518_s_power(struct v4l2_subdev *sd, int on)
{
    /* used when suspending */
    /* printk("%s: on %d\n", __func__, on); */
    if (!on) {
        struct sp2518_priv *priv = to_priv(sd);
        priv->initialized = false;
    }
    return 0;
}

static const struct v4l2_subdev_core_ops sp2518_subdev_core_ops = {
    .g_chip_ident	= sp2518_g_chip_ident,
    .s_power        = sp2518_s_power,
    .s_ctrl         = v4l2_subdev_s_ctrl,
};

/**
 * video ops
 */
static int sp2518_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp2518_priv *priv = to_priv(sd);
    int ret = 0;

    printk("%s: enable %d, initialized %d\n", __func__, enable, priv->initialized);
    if (enable) {
        if (!priv->win || !priv->cfmt) {
            dev_err(&client->dev, "norm or win select error\n");
            return -EPERM;
        }
         /* write init regs */
        if (!priv->initialized) {
            if (!check_id(client))
                return -EINVAL;

            ret = sp2518_write_array(client, sp2518_svga_init_regs);
            if (ret < 0) {
                printk(KERN_ERR "%s: failed to sp2518_write_array init regs\n", __func__);
                return -EIO;
            }
            priv->initialized = true;
        }

        ret = sp2518_write_array(client, priv->win->win_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to sp2518_write_array win regs\n", __func__);
            return -EIO;
        }

        ret = sp2518_set_mbusformat(client, priv->cfmt);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to sp2518_set_mbusformat()\n", __func__);
            return -EIO;
        }
    } else {
        sp2518_write_array(client, sp2518_disable_regs);
    }

    return ret;
}

static int sp2518_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (fsize->index >= ARRAY_SIZE(sp2518_win)) {
        dev_err(&client->dev, "index(%d) is over range %d\n", fsize->index, ARRAY_SIZE(sp2518_win));
        return -EINVAL;
    }

    switch (fsize->pixel_format) {
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUV422P:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
            fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
            fsize->discrete.width = sp2518_win[fsize->index]->width;
            fsize->discrete.height = sp2518_win[fsize->index]->height;
            break;
        default:
            dev_err(&client->dev, "pixel_format(%d) is Unsupported\n", fsize->pixel_format);
            return -EINVAL;
    }

    dev_info(&client->dev, "type %d, width %d, height %d\n", V4L2_FRMSIZE_TYPE_DISCRETE, fsize->discrete.width, fsize->discrete.height);
    return 0;
}

static int sp2518_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned int index,
        enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(sp2518_cfmts))
        return -EINVAL;

    *code = sp2518_cfmts[index].code;
    return 0;
}

static int sp2518_g_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp2518_priv *priv = to_priv(sd);
    if (!priv->win || !priv->cfmt) {
        u32 width = SVGA_WIDTH;
        u32 height = SVGA_HEIGHT;
        int ret = sp2518_set_params(sd, &width, &height, V4L2_MBUS_FMT_UYVY8_2X8);
        if (ret < 0) {
            dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
            return ret;
        }
    }

    mf->width   = priv->win->width;
    mf->height  = priv->win->height;
    mf->code    = priv->cfmt->code;
    mf->colorspace  = priv->cfmt->colorspace;
    mf->field   = V4L2_FIELD_NONE;
    dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
    return 0;
}

static int sp2518_try_mbus_fmt(struct v4l2_subdev *sd,
        struct v4l2_mbus_framefmt *mf)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    struct sp2518_priv *priv = to_priv(sd);
    const struct sp2518_win_size *win;
    int i;

    /*
     * select suitable win
     */
    win = sp2518_select_win(mf->width, mf->height);
    if (!win)
        return -EINVAL;

    mf->width   = win->width;
    mf->height  = win->height;
    mf->field   = V4L2_FIELD_NONE;


    for (i = 0; i < ARRAY_SIZE(sp2518_cfmts); i++)
        if (mf->code == sp2518_cfmts[i].code)
            break;

    if (i == ARRAY_SIZE(sp2518_cfmts)) {
        /* Unsupported format requested. Propose either */
        if (priv->cfmt) {
            /* the current one or */
            mf->colorspace = priv->cfmt->colorspace;
            mf->code = priv->cfmt->code;
        } else {
            /* the default one */
            mf->colorspace = sp2518_cfmts[0].colorspace;
            mf->code = sp2518_cfmts[0].code;
        }
    } else {
        /* Also return the colorspace */
        mf->colorspace	= sp2518_cfmts[i].colorspace;
    }

    return 0;
}

static int sp2518_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    struct sp2518_priv *priv = to_priv(sd);

    int ret = sp2518_set_params(sd, &mf->width, &mf->height, mf->code);
    if (!ret)
        mf->colorspace = priv->cfmt->colorspace;

    return ret;
}

static const struct v4l2_subdev_video_ops sp2518_subdev_video_ops = {
    .s_stream               = sp2518_s_stream,
    .enum_framesizes        = sp2518_enum_framesizes,
    .enum_mbus_fmt          = sp2518_enum_mbus_fmt,
    .g_mbus_fmt             = sp2518_g_mbus_fmt,
    .try_mbus_fmt           = sp2518_try_mbus_fmt,
    .s_mbus_fmt             = sp2518_s_mbus_fmt,
};

/**
 * pad ops
 */
static int sp2518_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    struct v4l2_mbus_framefmt *mf = &fmt->format;
    printk("%s: %dx%d\n", __func__, mf->width, mf->height);
    return sp2518_s_mbus_fmt(sd, mf);
}

static const struct v4l2_subdev_pad_ops sp2518_subdev_pad_ops = {
    .set_fmt    = sp2518_s_fmt,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops sp2518_subdev_ops = {
    .core   = &sp2518_subdev_core_ops,
    .video  = &sp2518_subdev_video_ops,
    .pad    = &sp2518_subdev_pad_ops,
};

/**
 * media_entity_operations
 */
static int sp2518_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    printk("%s\n", __func__);
    return 0;
}

static const struct media_entity_operations sp2518_media_ops = {
    .link_setup = sp2518_link_setup,
};

/****************************************************************************************
 * initialize
 */
static void sp2518_priv_init(struct sp2518_priv * priv)
{
    priv->model = V4L2_IDENT_SP2518;
    priv->prev_capt_mode = PREVIEW_MODE;
    priv->timeperframe.denominator = 12;//30;
    priv->timeperframe.numerator = 1;
    priv->win = &sp2518_win_svga;
}

static int sp2518_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct sp2518_priv *priv;
    struct v4l2_subdev *sd;
    int ret;

    priv = kzalloc(sizeof(struct sp2518_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    sp2518_priv_init(priv);

    sd = &priv->subdev;
    strcpy(sd->name, MODULE_NAME);

    /* register subdev */
    v4l2_i2c_subdev_init(sd, client, &sp2518_subdev_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    priv->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    sd->entity.ops  = &sp2518_media_ops;
    if (media_entity_init(&sd->entity, 1, &priv->pad, 0)) {
        dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
        kfree(priv);
        return -ENOENT;
    }

    ret = sp2518_initialize_ctrls(priv);
    if (ret < 0) {
        printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
        kfree(priv);
        return ret;
    }

    return 0;
}

static int sp2518_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    v4l2_device_unregister_subdev(sd);
    v4l2_ctrl_handler_free(sd->ctrl_handler);
    media_entity_cleanup(&sd->entity);
    kfree(to_priv(sd));
    return 0;
}

static const struct i2c_device_id sp2518_id[] = {
    { MODULE_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, sp2518_id);

static struct i2c_driver sp2518_i2c_driver = {
    .driver = {
        .name = MODULE_NAME,
    },
    .probe    = sp2518_probe,
    .remove   = sp2518_remove,
    .id_table = sp2518_id,
};

module_i2c_driver(sp2518_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for sp2518");
MODULE_AUTHOR("caichsh(caichsh@artekmicro.com)");
MODULE_LICENSE("GPL v2");
