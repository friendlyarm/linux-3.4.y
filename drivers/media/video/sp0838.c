
/*
 * sp0838 Camera Driver
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
 * fixed by swpark@nexell.co.kr for compatibility with general v4l2 layer (remove soc-camera interface)
 */
//Suruibin 2015-04-03 V1.0 heq sharp ae

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

#define MODULE_NAME "SP0838"

#ifdef SP0838_DEBUG
#define assert(expr) \
    if (unlikely(!(expr))) {				\
        pr_err("Assertion failed! %s,%s,%s,line=%d\n",	\
#expr, __FILE__, __func__, __LINE__);	\
    }

#define SP0838_DEBUG(fmt,args...) printk(KERN_ALERT fmt, ##args)
#else

#define assert(expr) do {} while (0)

#define SP0838_DEBUG(fmt,args...)
#endif

#define PID                 0x02 /* Product ID Number  *///caichsh
#define SENSOR_ID           0x27
#define NUM_CTRLS           6
#define V4L2_IDENT_SP0838   64113

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

/********************************************************************************
 * reg define
 */
#define SP0838 0x27 //pga

#define OUTTO_SENSO_CLOCK 24000000


/********************************************************************************
 * predefine reg values
 */
struct regval_list {
    unsigned char reg_num;
    unsigned char value;
};

#define ENDMARKER { 0xff, 0xff }

/*
 * init setting
 */
#define  Pre_Value_P0_0x30  0x00
//Filter en&dis
#define  Pre_Value_P0_0x56  0x71 //xg test 73
#define  Pre_Value_P0_0x57  0x10  //filter outdoor
#define  Pre_Value_P0_0x58  0x10  //filter indoor
#define  Pre_Value_P0_0x59  0x10  //filter night
#define  Pre_Value_P0_0x5a  0x02  //smooth outdoor 06
#define  Pre_Value_P0_0x5b  0x05 //smooth indoor  //0x0a xg 20121013
#define  Pre_Value_P0_0x5c  0x2a  //smooht night	//0x20 xg 20121013
//outdoor sharpness
#define  Pre_Value_P0_0x65  0x04  //0x03 xg 20121013
#define  Pre_Value_P0_0x66  0x01
#define  Pre_Value_P0_0x67  0x03
#define  Pre_Value_P0_0x68  0x43//0x46 xg 20121013
//indoor sharpness
#define  Pre_Value_P0_0x6b  0x06//0x04 xg 20121013
#define  Pre_Value_P0_0x6c  0x01
#define  Pre_Value_P0_0x6d  0x03
#define  Pre_Value_P0_0x6e  0x43//0x46 xg 20121013
//night sharpness
#define  Pre_Value_P0_0x71  0x07//0x05 xg 20121013
#define  Pre_Value_P0_0x72  0x01
#define  Pre_Value_P0_0x73  0x03
#define  Pre_Value_P0_0x74  0x43//0x46 xg 20121013
//color
#define  Pre_Value_P0_0x7f  0xd7  //R  0xd7
#define  Pre_Value_P0_0x87  0xf7  //B   0xf8
//satutation
#define  Pre_Value_P0_0xd8  0x58
#define  Pre_Value_P0_0xd9  0x58//0x48 xg 20121013 0x68
#define  Pre_Value_P0_0xda  0x58//0x48 xg 20121013 0x68
#define  Pre_Value_P0_0xdb  0x58//0x48 xg 20121013
//AE target
#define  Pre_Value_P0_0xf7  0x78+0x20
#define  Pre_Value_P0_0xf8  0x63+0x20
#define  Pre_Value_P0_0xf9  0x68+0x20
#define  Pre_Value_P0_0xfa  0x53+0x20
//HEQ
#define  Pre_Value_P0_0xdd  0x70 //70
#define  Pre_Value_P0_0xde  0x98//0x98 xg 20121013
//AWB pre gain
#define  Pre_Value_P1_0x28  0x75
#define  Pre_Value_P1_0x29  0x4a//0x4e

//VBLANK
#define  Pre_Value_P0_0x05  0x00
#define  Pre_Value_P0_0x06  0x00
//HBLANK
#define  Pre_Value_P0_0x09  0x01
#define  Pre_Value_P0_0x0a  0x76

static struct regval_list sp0838_init_regs[] =
{
    //SP0838 ini
    {0xfd,0x00},//P0
    {0x36,0x20},
    {0x1B,0x02},
    {0x1c,0x07},
    {0x27,0xe8},
    {0x28,0x0B},
#ifndef CONFIG_VIDEO_SP0838_FIXED_FRAMERATE
    {0x30,0x02},  // variable frame rate
#else
    {0x30,0x00},  // pclk is same to mclk
#endif
    {0x31,0x10},
    {0x32,0x00},
    {0x22,0xc0},
    {0x26,0x10},
    {0x5f,0x11},//Bayer order
    {0xfd,0x01},//P1
    {0x25,0x1a},//Awb start
    {0x26,0xfb},
    {0x28,Pre_Value_P1_0x28},
    {0x29,Pre_Value_P1_0x29},
    {0xfd,0x00},
    {0xe7,0x03},
    {0xe7,0x00},
    {0xfd,0x01},
    {0x31,0x60},//64
    {0x32,0x18},
    {0x4d,0xdc},
    {0x4e,0x53},
    {0x41,0x8c},
    {0x42,0x57},
    {0x55,0xff},
    {0x56,0x00},
    {0x59,0x82},
    {0x5a,0x00},
    {0x5d,0xff},
    {0x5e,0x6f},
    {0x57,0xff},
    {0x58,0x00},
    {0x5b,0xff},
    {0x5c,0xa8},
    {0x5f,0x75},
    {0x60,0x00},
    {0x2d,0x00},
    {0x2e,0x00},
    {0x2f,0x00},
    {0x30,0x00},
    {0x33,0x00},
    {0x34,0x00},
    {0x37,0x00},
    {0x38,0x00},//awb end
    {0xfd,0x00},//P0
    {0x33,0x6f},//LSC BPC EN
    {0x51,0x3f},//BPC debug start
    {0x52,0x09},
    {0x53,0x00},
    {0x54,0x00},
    {0x55,0x10},//BPC debug end
    {0x4f,0x08},//blueedge
    {0x50,0x08},
    {0x57,Pre_Value_P0_0x57},//Raw filter debut start
    {0x58,Pre_Value_P0_0x58},
    {0x59,Pre_Value_P0_0x59},
    {0x56,Pre_Value_P0_0x56},
    {0x5a,Pre_Value_P0_0x5a},
    {0x5b,Pre_Value_P0_0x5b},
    {0x5c,Pre_Value_P0_0x5c},//Raw filter debut end
    {0x65,Pre_Value_P0_0x65},//Sharpness debug start
    {0x66,Pre_Value_P0_0x66},
    {0x67,Pre_Value_P0_0x67},
    {0x68,Pre_Value_P0_0x68},
    {0x69,0x7f},
    {0x6a,0x01},
    {0x6b,Pre_Value_P0_0x6b},
    {0x6c,Pre_Value_P0_0x6c},
    {0x6d,Pre_Value_P0_0x6d},//Edge gain normal
    {0x6e,Pre_Value_P0_0x6e},//Edge gain normal
    {0x6f,0x7f},
    {0x70,0x01},
    {0x71,Pre_Value_P0_0x71}, //\C8\F1\BB\AF\E3\D0ֵ
    {0x72,Pre_Value_P0_0x72}, //\C8\F5\C2\D6\C0\AA\E3\D0ֵ
    {0x73,Pre_Value_P0_0x73}, //\B1\DFԵ\D5\FD\CF\F2\D4\F6\D2\E6ֵ
    {0x74,Pre_Value_P0_0x74}, //\B1\DFԵ\B7\B4\CF\F2\D4\F6\D2\E6ֵ
    {0x75,0x7f},              //ʹ\C4\DCλ
    {0x76,0x01},//Sharpness debug end
    {0xcb,0x07},//HEQ&Saturation debug start
    {0xcc,0x04},
    {0xce,0xff},
    {0xcf,0x10},
    {0xd0,0x20},
    {0xd1,0x00},
    {0xd2,0x1c},
    {0xd3,0x16},
    {0xd4,0x00},
    {0xd6,0x1c},
    {0xd7,0x16},
    {0xdd,Pre_Value_P0_0xdd},//Contrast
    {0xde,Pre_Value_P0_0xde},//HEQ&Saturation debug end
    {0x7f,Pre_Value_P0_0x7f},//Color Correction start
    {0x80,0xbc},
    {0x81,0xed},
    {0x82,0xd7},
    {0x83,0xd4},
    {0x84,0xd6},
    {0x85,0xff},
    {0x86,0x89},
    {0x87,Pre_Value_P0_0x87},
    {0x88,0x3c},
    {0x89,0x33},
    {0x8a,0x0f},//Color Correction end
    {0x8b,0x0 },//gamma start
    {0x8c,0x1a},
    {0x8d,0x29},
    {0x8e,0x41},
    {0x8f,0x62},
    {0x90,0x7c},
    {0x91,0x90},
    {0x92,0xa2},
    {0x93,0xaf},
    {0x94,0xbc},
    {0x95,0xc5},
    {0x96,0xcd},
    {0x97,0xd5},
    {0x98,0xdd},
    {0x99,0xe5},
    {0x9a,0xed},
    {0x9b,0xf5},
    {0xfd,0x01},//P1
    {0x8d,0xfd},
    {0x8e,0xff},//gamma end
    {0xfd,0x00},//P0
    {0xca,0xcf},
    {0xd8,Pre_Value_P0_0xd8},//UV outdoor
    {0xd9,Pre_Value_P0_0xd9},//UV indoor
    {0xda,Pre_Value_P0_0xda},//UV dummy
    {0xdb,Pre_Value_P0_0xdb},//UV lowlight
    {0xb9,0x00},//Ygamma start
    {0xba,0x04},
    {0xbb,0x08},
    {0xbc,0x10},
    {0xbd,0x20},
    {0xbe,0x30},
    {0xbf,0x40},
    {0xc0,0x50},
    {0xc1,0x60},
    {0xc2,0x70},
    {0xc3,0x80},
    {0xc4,0x90},
    {0xc5,0xA0},
    {0xc6,0xB0},
    {0xc7,0xC0},
    {0xc8,0xD0},
    {0xc9,0xE0},
    {0xfd,0x01},//P1
    {0x89,0xf0},
    {0x8a,0xff},//Ygamma end
    {0xfd,0x00},//P0
    {0xe8,0x30},//AEdebug start
    {0xe9,0x30},
    {0xea,0x40},//Alc Window sel
    {0xf4,0x1b},//outdoor mode sel
    {0xf5,0x80},
    {0xf7,Pre_Value_P0_0xf7},//AE target
    {0xf8,Pre_Value_P0_0xf8},
    {0xf9,Pre_Value_P0_0xf9},//AE target
    {0xfa,Pre_Value_P0_0xfa},
    {0xfd,0x01},//P1
    {0x09,0x31},//AE Step 3.0
    {0x0a,0x85},
    {0x0b,0x0b},//AE Step 3.0
    {0x14,0x20},
    {0x15,0x0f},
    {0xfd,0x00},//P0
    {0x05,0x00},
    {0x06,0x00},
    {0x09,0x01},
    {0x0a,0x76},
    {0xf0,0x62},
    {0xf1,0x00},
    {0xf2,0x5f},
    {0xf5,0x78},
    {0xfd,0x01},//P1
    {0x00,0xba},
    {0x0f,0x60},
    {0x16,0x60},
    {0x17,0xa2},
    {0x18,0xaa},
    {0x1b,0x60},
    {0x1c,0xaa},
    {0xb4,0x20},
    {0xb5,0x3a},
    {0xb6,0x5e},
    {0xb9,0x40},
    {0xba,0x4f},
    {0xbb,0x47},
    {0xbc,0x45},
    {0xbd,0x43},
    {0xbe,0x42},
    {0xbf,0x42},
    {0xc0,0x42},
    {0xc1,0x41},
    {0xc2,0x41},
    {0xc3,0x41},
    {0xc4,0x41},
    {0xc5,0x78},
    {0xc6,0x41},
    {0xca,0x78},
    {0xcb,0x0c},//AEdebug end
#ifndef CONFIG_VIDEO_SP0838_FIXED_FRAMERATE
    // 12M 50HZ 8-12    ///whl121126
    {0xfd , 0x00},
    {0x05 , 0x00},
    {0x06 , 0x00},
    {0x09 , 0x00},
    {0x0a , 0x76},
    {0xf0 , 0x3b},
    {0xf1 , 0x00},
    {0xf2 , 0x56},
    {0xf5 , 0x6f},
    {0xfd , 0x01},
    {0x00 , 0xa9},
    {0x0f , 0x57},
    {0x16 , 0x57},
    {0x17 , 0x99},
    {0x18 , 0xa1},
    {0x1b , 0x57},
    {0x1c , 0xa1},
    {0xb4 , 0x21},
    {0xb5 , 0x39},
    {0xb6 , 0x39},
    {0xb9 , 0x40},
    {0xba , 0x4f},
    {0xbb , 0x47},
    {0xbc , 0x45},
    {0xbd , 0x43},
    {0xbe , 0x42},
    {0xbf , 0x42},
    {0xc0 , 0x42},
    {0xc1 , 0x41},
    {0xc2 , 0x41},
    {0xc3 , 0x41},
    {0xc4 , 0x41},
    {0xc5 , 0x70},
    {0xc6 , 0x41},
    {0xca , 0x70},
    {0xcb , 0x0c},
#else
//sensor AE settings:    24M 9fps
    {0xfd,0x00},
    {0x05,0x00},
    {0x06,0x00},
    {0x09,0x07},
    {0x0a,0x19},
    {0xf0,0x2d},
    {0xf1,0x00},
    {0xf2,0x4f},
    {0xf5,0x68},
    {0xfd,0x01},
    {0x00,0xa0},
    {0x0f,0x50},
    {0x16,0x50},
    {0x17,0x90},
    {0x18,0x98},
    {0x1b,0x50},
    {0x1c,0x98},
    {0xb4,0x21},
    {0xb5,0x2b},
    {0xb6,0x2b},
    {0xb9,0x40},
    {0xba,0x4f},
    {0xbb,0x47},
    {0xbc,0x45},
    {0xbd,0x43},
    {0xbe,0x42},
    {0xbf,0x42},
    {0xc0,0x42},
    {0xc1,0x41},
    {0xc2,0x41},
    {0xc3,0x41},
    {0xc4,0x70},
    {0xc5,0x41},
    {0xc6,0x41},
    {0xca,0x70},
    {0xcb,0x0b},
    {0xfd,0x00},
#endif
    {0xfd,0x00},  //P0
    {0x32,0x15},  //Auto_mode set
    {0x34,0x66},  //Isp_mode set
    {0x35,0x40},  //out format
    {0x36,0x00},  //psw0523 : vsync invert bit7, hsync invert bit6
    /* {0x36,0x4d},  //psw0523 : vsync invert bit7, hsync invert bit6 */
    /* {0x36,0x00},  //psw0523 : vsync invert bit7, hsync invert bit6 */
    ENDMARKER,
};

static struct regval_list sp0830_enable_regs[] = {
    {0xfd,0x00},
    {0x36,0x00},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp0830_disable_regs[] = {
    {0xfd,0x00},
    {0x36,0x20}, /* Disable HSYNC & VSYNC */
    {0xfd,0x00},
    ENDMARKER,
};

/*
 * color code
 */
static struct regval_list sp0838_fmt_yuv422_yuyv[] =
{
    //YCbYCr
    {0xfd,0x00},
    // psw0523 fix
    {0x35,0x80},
    /* {0x35,0xc0}, */
    ENDMARKER,
};

static struct regval_list sp0838_fmt_yuv422_yvyu[] =
{
    //YCrYCb
    {0xfd,0x00},
    {0x35,0x41},
    ENDMARKER,
};

static struct regval_list sp0838_fmt_yuv422_vyuy[] =
{
    //CrYCbY
    {0xfd,0x00},
    {0x35,0x01},
    ENDMARKER,
};

static struct regval_list sp0838_fmt_yuv422_uyvy[] =
{
    //CbYCrY
    {0xfd,0x00},
    {0x35,0x00},
    ENDMARKER,
};

/* static struct regval_list sp0838_fmt_raw[] = */
/* { */
/*     {0xfd,0x00}, */
/*     ENDMARKER, */
/* }; */

/*
 *AWB
 */
static const struct regval_list sp0838_awb_regs_enable[] =
{
    {0xfd,0x01},
    {0x28,Pre_Value_P1_0x28},
    {0x29,Pre_Value_P1_0x29},
    {0xfd,0x00},
    {0x32,0x15},
    {0xfd,0x00},
    ENDMARKER,
};
static const struct regval_list sp0838_awb_regs_diable[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp0838_wb_cloud_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x71},
    {0x29,0x41},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp0838_wb_daylight_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x6b},
    {0x29,0x48},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp0838_wb_incandescence_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x41},
    {0x29,0x71},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp0838_wb_fluorescent_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x5a},
    {0x29,0x62},
    {0xfd,0x00},
    ENDMARKER,
};

static struct regval_list sp0838_wb_tungsten_regs[] =
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x57},
    {0x29,0x66},
    {0xfd,0x00},
    ENDMARKER,
};

/*
 * colorfx
 */
static struct regval_list sp0838_colorfx_none_regs[] =
{
    {0xfd, 0x00},
    {0x62, 0x00},
    {0x63, 0x80},
    {0x64, 0x80},
    ENDMARKER,
};

static struct regval_list sp0838_colorfx_bw_regs[] =
{
    {0xfd, 0x00},
    {0x62, 0x40},
    {0x63, 0x80},
    {0x64, 0x80},
    ENDMARKER,
};

static struct regval_list sp0838_colorfx_sepia_regs[] =
{
    {0xfd, 0x00},
    {0x62, 0x20},
    {0x63, 0xc0},
    {0x64, 0x20},
    ENDMARKER,
};

static struct regval_list sp0838_colorfx_negative_regs[] =
{
    {0xfd, 0x00},
    {0x62, 0x10},
    {0x63, 0x80},
    {0x64, 0x80},
    ENDMARKER,
};

/*
 * window size
 */
static const struct regval_list sp0838_vga_regs[] = {
    {0xfd, 0x00},
    {0x47, 0x00},
    {0x48, 0x00},
    {0x49, 0x01},
    {0x4a, 0xe0},
    {0x4b, 0x00},
    {0x4c, 0x00},
    {0x4d, 0x02},
    {0x4e, 0x80},
    /* {0x31, 0x10}, */
    ENDMARKER,
};

static const struct regval_list sp0838_qvga_regs[] = {
#if 0
    {0xfd, 0x00},
    {0x47, 0x00},
    {0x48, 0x00},
    {0x49, 0x00},
    {0x4a, 0xf0},
    {0x4b, 0x00},
    {0x4c, 0x00},
    {0x4d, 0x01},
    {0x4e, 0x40},

    // psw0523 fix for PCLK
    /* {0x31, 0x10}, */
    {0x31, 0x98},
#endif

	{0xfd,0x00},
	{0x47,0x00},
	{0x48,0x78},
	{0x49,0x00},
	{0x4a,0xf0},
	{0x4b,0x00},
	{0x4c,0xa0},
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

    ENDMARKER,
};

static const struct regval_list sp0838_cif_regs[] = {
    {0xfd,0x00},
    {0x47,0x00},
    {0x48,0x00},
    {0x49,0x01},
    {0x4a,0x20},
    {0x4b,0x00},
    {0x4c,0x00},
    {0x4d,0x01},
    {0x4e,0x60},
    /* {0x31, 0x10}, */
    ENDMARKER,
};

static const struct regval_list sp0838_qcif_regs[] = {
    {0xfd,0x00},
    {0x47,0x00},
    {0x48,0x00},
    {0x49,0x00},
    {0x4a,0x90},
    {0x4b,0x00},
    {0x4c,0x00},
    {0x4d,0x00},
    {0x4e,0xb0},
    /* {0x31, 0x10}, */
    ENDMARKER,
};

static const struct regval_list sp0838_semiqcif_regs[] = {
    {0xfd, 0x00},
    {0x47, 0x00},
    {0x48, 0xa7},
    {0x49, 0x00},
    {0x4a, 0x91},
    {0x4b, 0x00},
    {0x4c, 0xd8},
    {0x4d, 0x00},
    {0x4e, 0xd0},
    ENDMARKER,
};

static const struct regval_list sp0838_semicif_regs[] = {
    {0xfd, 0x00},
    {0x47, 0x00},
    {0x48, 0x5f},
    {0x49, 0x01},
    {0x4a, 0x21},
    {0x4b, 0x00},
    {0x4c, 0x80},
    {0x4d, 0x01},
    {0x4e, 0x80},
    ENDMARKER,
};


/********************************************************************************
 * structures
 */
struct sp0838_win_size {
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
    unsigned int dummy_line;
    unsigned int dummy_pixel;
    unsigned int extra_line;
} exposure_param_t;

enum prev_capt {
    PREVIEW_MODE = 0,
    CAPTURE_MODE
};

struct sp0838_priv {
    struct v4l2_subdev                  subdev;
    struct media_pad                    pad;
    struct v4l2_ctrl_handler            hdl;
    const struct sp0838_color_format    *cfmt;
    const struct sp0838_win_size        *win;
    int                                 model;
    bool                                initialized;

    /**
     * ctrls
     */
    /* standard */
    struct v4l2_ctrl *auto_white_balance;
    struct v4l2_ctrl *exposure;
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

struct sp0838_color_format {
    enum v4l2_mbus_pixelcode code;
    enum v4l2_colorspace colorspace;
};

/********************************************************************************
 * tables
 */
static const struct sp0838_color_format sp0838_cfmts[] = {
    {
        .code       = V4L2_MBUS_FMT_YUYV8_2X8,
        .colorspace = V4L2_COLORSPACE_JPEG,
    },
    {
        .code       = V4L2_MBUS_FMT_UYVY8_2X8,
        .colorspace = V4L2_COLORSPACE_JPEG,
    },
    {
        .code       = V4L2_MBUS_FMT_YVYU8_2X8,
        .colorspace = V4L2_COLORSPACE_JPEG,
    },
    {
        .code       = V4L2_MBUS_FMT_VYUY8_2X8,
        .colorspace = V4L2_COLORSPACE_JPEG,
    },
};

/*
 * window size list
 */
#define VGA_WIDTH           640
#define VGA_HEIGHT          480
#define SP0838_MAX_WIDTH    VGA_WIDTH
#define SP0838_MAX_HEIGHT   VGA_HEIGHT
#define AHEAD_LINE_NUM      15    //10\D0\D0 = 50\B4\CEѭ\BB\B7(sp0838)
#define DROP_NUM_CAPTURE    3
#define DROP_NUM_PREVIEW    16


static unsigned int frame_rate_vga[] = {30,};

/* 640x480 */
static const struct sp0838_win_size sp0838_win_vga = {
    .name     = "VGA",
    .width    = VGA_WIDTH,
    .height   = VGA_HEIGHT,
    .win_regs = sp0838_vga_regs,
    .frame_rate_array = frame_rate_vga,
};

/* 320x240 */
static const struct sp0838_win_size sp0838_win_qvga = {
    .name     = "QVGA",
    .width    = 320,
    .height   = 240,
    .win_regs = sp0838_qvga_regs,
    .frame_rate_array = frame_rate_vga,
};

/* 352x288 */
static const struct sp0838_win_size sp0838_win_cif = {
    .name     = "CIF",
    .width    = 352,
    .height   = 288,
    .win_regs = sp0838_cif_regs,
    .frame_rate_array = frame_rate_vga,
};

/* 176x144 */
static const struct sp0838_win_size sp0838_win_qcif = {
    .name     = "QCIF",
    .width    = 176,
    .height   = 144,
    .win_regs = sp0838_qcif_regs,
    .frame_rate_array = frame_rate_vga,
};

/* 384x289 */
static const struct sp0838_win_size sp0838_win_semicif = {
    .name     = "SEMICIF",
    .width    = 384,
    .height   = 289,
    .win_regs = sp0838_semicif_regs,
    .frame_rate_array = frame_rate_vga,
};

/* 208x145 */
static const struct sp0838_win_size sp0838_win_semiqcif = {
    .name     = "SEMIQCIF",
    .width    = 208,
    .height   = 145,
    .win_regs = sp0838_semiqcif_regs,
    .frame_rate_array = frame_rate_vga,
};

static const struct sp0838_win_size *sp0838_win[] = {
    &sp0838_win_vga,
    &sp0838_win_qvga,
    &sp0838_win_cif,
    &sp0838_win_qcif,
    &sp0838_win_semicif,
    &sp0838_win_semiqcif,
};

/********************************************************************************
 * general functions
 */
static inline struct sp0838_priv *to_priv(struct v4l2_subdev *subdev)
{
    return container_of(subdev, struct sp0838_priv, subdev);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct sp0838_priv, hdl)->subdev;
}

static bool check_id(struct i2c_client *client)
{
    u8 pid = i2c_smbus_read_byte_data(client, PID);
    if (pid == SENSOR_ID)
        return true;

    printk(KERN_ERR "failed to check id: 0x%x\n", pid);
    return false;
}

static int sp0838_write_array(struct i2c_client *client, const struct regval_list *vals)
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

static int sp0838_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set) __attribute__((unused));
static int sp0838_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set)
{
    s32 val = i2c_smbus_read_byte_data(client, command);
    if (val < 0)
        return val;

    val &= ~mask;
    val |= set & mask;

    return i2c_smbus_write_byte_data(client, command, val);
}

static const struct sp0838_win_size *sp0838_select_win(u32 width, u32 height)
{
    const struct sp0838_win_size *win;
    int i;

    for (i = 0; i < ARRAY_SIZE(sp0838_win); i++) {
        win = sp0838_win[i];
        if (width == win->width && height == win->height)
            return win;
    }

    printk(KERN_ERR "%s: unsupported width, height (%dx%d)\n", __func__, width, height);
    return NULL;
}

static int sp0838_set_mbusformat(struct i2c_client *client, const struct sp0838_color_format *cfmt) __attribute__((unused));
static int sp0838_set_mbusformat(struct i2c_client *client, const struct sp0838_color_format *cfmt)
{
    enum v4l2_mbus_pixelcode code;
    int ret = -1;
    code = cfmt->code;
    printk("%s: code 0x%x\n", __func__, code);
    switch (code) {
        case V4L2_MBUS_FMT_YUYV8_2X8:
            ret  = sp0838_write_array(client, sp0838_fmt_yuv422_yuyv);
            break;
        case V4L2_MBUS_FMT_UYVY8_2X8:
            ret  = sp0838_write_array(client, sp0838_fmt_yuv422_uyvy);
            break;
        case V4L2_MBUS_FMT_YVYU8_2X8:
            ret  = sp0838_write_array(client, sp0838_fmt_yuv422_yvyu);
            break;
        case V4L2_MBUS_FMT_VYUY8_2X8:
            ret  = sp0838_write_array(client, sp0838_fmt_yuv422_vyuy);
            break;
        default:
            printk(KERN_ERR "mbus code error in %s() line %d\n",__FUNCTION__, __LINE__);
    }
    return ret;
}

static int sp0838_set_params(struct v4l2_subdev *sd, u32 *width, u32 *height, enum v4l2_mbus_pixelcode code)
{
    struct sp0838_priv *priv = to_priv(sd);
    const struct sp0838_win_size *old_win, *new_win;
    int i;

    priv->cfmt = NULL;
    for (i = 0; i < ARRAY_SIZE(sp0838_cfmts); i++) {
        if (code == sp0838_cfmts[i].code) {
            priv->cfmt = sp0838_cfmts + i;
            break;
        }
    }
    if (!priv->cfmt) {
        printk(KERN_ERR "Unsupported sensor format.\n");
        return -EINVAL;
    }

    old_win = priv->win;
    new_win = sp0838_select_win(*width, *height);
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

/********************************************************************************
 * control functions
 */
static int sp0838_set_auto_white_balance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int auto_white_balance = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, auto_white_balance);
    if (auto_white_balance < 0 || auto_white_balance > 1) {
        dev_err(&client->dev, "set auto_white_balance over range, auto_white_balance = %d\n", auto_white_balance);
        return -ERANGE;
    }

    switch(auto_white_balance) {
        case 0:
            ret = sp0838_write_array(client, sp0838_awb_regs_diable);
            break;
        case 1:
            ret = sp0838_write_array(client, sp0838_awb_regs_enable);
            break;
    }

    assert(ret == 0);

    return 0;
}

static int sp0838_set_white_balance_temperature(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int white_balance_temperature = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch(white_balance_temperature) {
        case V4L2_WHITE_BALANCE_INCANDESCENT:
            ret = sp0838_write_array(client, sp0838_wb_incandescence_regs);
            break;
        case V4L2_WHITE_BALANCE_FLUORESCENT:
            ret = sp0838_write_array(client,sp0838_wb_fluorescent_regs);
            break;
        case V4L2_WHITE_BALANCE_DAYLIGHT:
            ret = sp0838_write_array(client,sp0838_wb_daylight_regs);
            break;
        case V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT:
            ret = sp0838_write_array(client,sp0838_wb_cloud_regs);
            break;
        case V4L2_WHITE_BALANCE_TUNGSTEN:
            ret = sp0838_write_array(client,sp0838_wb_tungsten_regs);
            break;
        default:
            dev_err(&client->dev, "set white_balance_temperature over range, white_balance_temperature = %d\n", white_balance_temperature);
            return -ERANGE;
    }

    assert(ret == 0);

    return 0;
}

static int sp0838_set_colorfx(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int colorfx = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch (colorfx) {
        case V4L2_COLORFX_NONE: /* normal */
            ret = sp0838_write_array(client, sp0838_colorfx_none_regs);
            break;
        case V4L2_COLORFX_BW: /* black and white */
            ret = sp0838_write_array(client, sp0838_colorfx_bw_regs);
            break;
        case V4L2_COLORFX_SEPIA: /* antique ,\B8\B4\B9\C5*/
            ret = sp0838_write_array(client, sp0838_colorfx_sepia_regs);

            break;
        case V4L2_COLORFX_NEGATIVE: /* negative\A3\AC\B8\BAƬ */
            ret = sp0838_write_array(client, sp0838_colorfx_negative_regs);
            break;
        default:
            dev_err(&client->dev, "set colorfx over range, colorfx = %d\n", colorfx);
            return -ERANGE;
    }


    assert(ret == 0);

    return 0;
}

/* TODO */
static int sp0838_set_exposure_auto(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int exposure_auto = ctrl->val;

    if (exposure_auto < 0 || exposure_auto > 1) {
        dev_err(&client->dev, "set exposure_auto over range, exposure_auto = %d\n", exposure_auto);
        return -ERANGE;
    }

    return 0;
}

/* TODO */
static int sp0838_set_scene_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* int scene_exposure = ctrl->val; */

    return 0;
}

/* TODO */
static int sp0838_set_prev_capt_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* int mode = ctrl->val; */
    /* sp0838_priv *priv = to_priv(sd); */

    /* switch(mode) { */
    /*     case PREVIEW_MODE: */
    /*         priv->prev_capt_mode = mode; */
    /*         break; */
    /*     case CAPTURE_MODE: */
    /*         priv->prev_capt_mode = mode; */
    /*         break; */
    /*     default: */
    /*         dev_err(&client->dev, "set_prev_capt_mode over range, prev_capt_mode = %d\n", mode); */
    /*         return -ERANGE; */
    /* } */

    return 0;
}

static int sp0838_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret = 0;

    switch (ctrl->id) {
        case V4L2_CID_AUTO_WHITE_BALANCE:
            sp0838_set_auto_white_balance(sd, ctrl);
            break;

        case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
            sp0838_set_white_balance_temperature(sd, ctrl);
            break;

        case V4L2_CID_COLORFX:
            sp0838_set_colorfx(sd, ctrl);
            break;

        case V4L2_CID_EXPOSURE_AUTO:
            sp0838_set_exposure_auto(sd, ctrl);
            break;

        case V4L2_CID_SCENE_EXPOSURE:
            sp0838_set_scene_exposure(sd, ctrl);
            break;

        case V4L2_CID_PRIVATE_PREV_CAPT:
            sp0838_set_prev_capt_mode(sd, ctrl);
            break;

        default:
            dev_err(&client->dev, "%s: invalid control id %d\n", __func__, ctrl->id);
            return -EINVAL;
    }

    return ret;
}

static const struct v4l2_ctrl_ops sp0838_ctrl_ops = {
    .s_ctrl = sp0838_s_ctrl,
};

static const struct v4l2_ctrl_config sp0838_custom_ctrls[] = {
    {
        .ops    = &sp0838_ctrl_ops,
        .id     = V4L2_CID_SCENE_EXPOSURE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "SceneExposure",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &sp0838_ctrl_ops,
        .id     = V4L2_CID_PRIVATE_PREV_CAPT,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "PrevCapt",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }
};

static int sp0838_initialize_ctrls(struct sp0838_priv *priv)
{
    v4l2_ctrl_handler_init(&priv->hdl, NUM_CTRLS);

    /* standard ctrls */
    priv->auto_white_balance = v4l2_ctrl_new_std(&priv->hdl, &sp0838_ctrl_ops,
            V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
    if (!priv->auto_white_balance) {
        printk(KERN_ERR "%s: failed to create auto_white_balance ctrl\n", __func__);
        return -ENOENT;
    }

    priv->white_balance_temperature = v4l2_ctrl_new_std(&priv->hdl, &sp0838_ctrl_ops,
            V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 3, 1, 1);
    if (!priv->white_balance_temperature) {
        printk(KERN_ERR "%s: failed to create white_balance_temperature ctrl\n", __func__);
        return -ENOENT;
    }

    /* standard menus */
    priv->colorfx = v4l2_ctrl_new_std_menu(&priv->hdl, &sp0838_ctrl_ops,
            V4L2_CID_COLORFX, 3, 0, 0);
    if (!priv->colorfx) {
        printk(KERN_ERR "%s: failed to create colorfx ctrl\n", __func__);
        return -ENOENT;
    }

    priv->exposure_auto = v4l2_ctrl_new_std_menu(&priv->hdl, &sp0838_ctrl_ops,
            V4L2_CID_EXPOSURE_AUTO, 1, 0, 1);
    if (!priv->exposure_auto) {
        printk(KERN_ERR "%s: failed to create exposure_auto ctrl\n", __func__);
        return -ENOENT;
    }

    /* custom ctrls */
    priv->scene_exposure = v4l2_ctrl_new_custom(&priv->hdl, &sp0838_custom_ctrls[0], NULL);
    if (!priv->scene_exposure) {
        printk(KERN_ERR "%s: failed to create scene_exposure ctrl\n", __func__);
        return -ENOENT;
    }

    priv->prev_capt = v4l2_ctrl_new_custom(&priv->hdl, &sp0838_custom_ctrls[1], NULL);
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

/********************************************************************************
 * v4l2 subdev ops
 */

/**
 * core ops
 */
static int sp0838_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *id)
{
    struct sp0838_priv *priv = to_priv(sd);
    id->ident    = priv->model;
    id->revision = 0;
    return 0;
}

static int sp0838_s_power(struct v4l2_subdev *sd, int on)
{
    /* used when suspending */
    /* printk("%s: on %d\n", __func__, on); */
    if (!on) {
        struct sp0838_priv *priv = to_priv(sd);
        priv->initialized = false;
    }
    return 0;
}

static const struct v4l2_subdev_core_ops sp0838_subdev_core_ops = {
    .g_chip_ident   = sp0838_g_chip_ident,
    .s_power        = sp0838_s_power,
    .s_ctrl         = v4l2_subdev_s_ctrl,
};

/**
 * video ops
 */
static int sp0838_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sp0838_priv *priv = to_priv(sd);
    int ret = 0;

    printk( "%s: enable %d, initialized %d\n", __func__, enable, priv->initialized);

    if (enable) {
        if (!priv->win || !priv->cfmt) {
            dev_err(&client->dev, "norm or win select error\n");
            return -EPERM;
        }

        if (!priv->initialized) {
            if (!check_id(client))
                return -EINVAL;

            ret = sp0838_write_array(client, sp0838_init_regs);
			udelay(10);
            if (ret < 0) {
                printk(KERN_ERR "%s: failed to sp0838_write_array init regs\n", __func__);
                return -EIO;
            }

            priv->initialized = true;
        }

        ret = sp0838_write_array(client, priv->win->win_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to sp0838_write_array win regs\n", __func__);
            return -EIO;
        }

        // psw0523 test for 6818
#ifdef CONFIG_ARCH_S5P6818
        sp0838_write_array(client, sp0838_fmt_yuv422_uyvy);
#else
        sp0838_write_array(client, sp0838_fmt_yuv422_yuyv);
#endif
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to sp0830_write_array format regs\n", __func__);
            return -EIO;
        }

        ret = sp0838_write_array(client, sp0830_enable_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to sp0830_write_array enable regs\n", __func__);
            return -EIO;
        }
    } else {
        ret = sp0838_write_array(client, sp0830_disable_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to sp0830_write_array disable regs\n", __func__);
            return -EIO;
        }
    }

    return 0;
}

static const struct v4l2_subdev_video_ops sp0838_subdev_video_ops= {
    .s_stream = sp0838_s_stream,
};

/**
 * pad ops
 */
static int sp0838_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    struct sp0838_priv *priv = to_priv(sd);
    struct v4l2_mbus_framefmt *mf = &fmt->format;
    int ret = 0;

    printk("%s: %dx%d\n", __func__, mf->width, mf->height);

    ret = sp0838_set_params(sd, &mf->width, &mf->height, mf->code);
    if(!ret)
        mf->colorspace = priv->cfmt->colorspace;

    return ret;
}

static const struct v4l2_subdev_pad_ops sp0838_subdev_pad_ops = {
    .set_fmt = sp0838_s_fmt,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops sp0838_subdev_ops = {
    .core   = &sp0838_subdev_core_ops,
    .video  = &sp0838_subdev_video_ops,
    .pad    = &sp0838_subdev_pad_ops,
};

/**
 * media_entity_operations
 */
static int sp0838_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    printk("%s\n", __func__);
    return 0;
}

static const struct media_entity_operations sp0838_media_ops = {
    .link_setup = sp0838_link_setup,
};

/********************************************************************************
 * initialize
 */
static void sp0838_priv_init(struct sp0838_priv * priv)
{
    priv->model = V4L2_IDENT_SP0838;
    priv->prev_capt_mode = PREVIEW_MODE;
    priv->timeperframe.denominator =12;//30;
    priv->timeperframe.numerator = 1;
    priv->win = &sp0838_win_vga;
}

static int sp0838_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct sp0838_priv *priv;
    struct v4l2_subdev *sd;
    int ret;

    priv = kzalloc(sizeof(struct sp0838_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    sp0838_priv_init(priv);

    sd = &priv->subdev;
    strcpy(sd->name, MODULE_NAME);

    /* register subdev */
    v4l2_i2c_subdev_init(sd, client, &sp0838_subdev_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    priv->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    sd->entity.ops  = &sp0838_media_ops;
    if (media_entity_init(&sd->entity, 1, &priv->pad, 0)) {
        dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
        kfree(priv);
        return -ENOENT;
    }

    ret = sp0838_initialize_ctrls(priv);
    if (ret < 0) {
        printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
        kfree(priv);
        return ret;
    }

    return 0;
}

static int sp0838_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    v4l2_device_unregister_subdev(sd);
    v4l2_ctrl_handler_free(sd->ctrl_handler);
    media_entity_cleanup(&sd->entity);
    kfree(to_priv(sd));
    return 0;
}

static const struct i2c_device_id sp0838_id[] = {
    { MODULE_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, sp0838_id);

static struct i2c_driver sp0838_i2c_driver = {
    .driver = {
        .name = MODULE_NAME,
    },
    .probe    = sp0838_probe,
    .remove   = sp0838_remove,
    .id_table = sp0838_id,
};

module_i2c_driver(sp0838_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for sp0838");
MODULE_AUTHOR("pangga(panggah@artekmicro.com)");
MODULE_LICENSE("GPL v2");
