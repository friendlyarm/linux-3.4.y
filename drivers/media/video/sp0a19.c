
/*
 * Driver for MT9M001 CMOS Image Sensor from Micron
 *
 * Copyright (C) 2008, Guennadi Liakhovetski <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * fixed by swpark@nexell.co.kr for compatibility with general v4l2 layer (remove soc-camera interface)
 */

#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>

static int debug;
module_param(debug, int, S_IRUGO|S_IWUSR);

#define dprintk(level, fmt, arg...) do {			\
	if (debug >= level) 					\
	printk(KERN_WARNING fmt , ## arg); } while (0)

#define SENSOR_TR(format, ...)  printk(KERN_ERR format, ## __VA_ARGS__)
#define SENSOR_DG(format, ...)  dprintk(1, format, ## __VA_ARGS__)

#define _CONS(a,b)              a##b
#define CONS(a,b)               _CONS(a,b)

#define __STR(x)                #x
#define _STR(x)                 __STR(x)
#define STR(x)                  _STR(x)

#define MIN(x,y)                ((x<y) ? x: y)
#define MAX(x,y)                ((x>y) ? x: y)

/* private ctrls */
#define V4L2_CID_EFFECT         (V4L2_CTRL_CLASS_CAMERA | 0x1001)
#define V4L2_CID_SCENE          (V4L2_CTRL_CLASS_CAMERA | 0x1002)
#define V4L2_CID_FLASH          (V4L2_CTRL_CLASS_CAMERA | 0x1003)

/********************************************************************************
 * define config
 */
// this is org
/* #define CONFIG_SENSOR_WhiteBalance      1 */
/* #define CONFIG_SENSOR_Brightness        0 */
/* #define CONFIG_SENSOR_Contrast          0 */
/* #define CONFIG_SENSOR_Saturation        0 */
/* #define CONFIG_SENSOR_Effect            1 */
/* #define CONFIG_SENSOR_Scene             1 */
/* #define CONFIG_SENSOR_DigitalZoom       0 */
/* #define CONFIG_SENSOR_Focus             0 */
/* #define CONFIG_SENSOR_Exposure          0 */
/* #define CONFIG_SENSOR_Flash             0 */
/* #define CONFIG_SENSOR_Mirror            0 */
/* #define CONFIG_SENSOR_Flip              0 */

#define CONFIG_SENSOR_WhiteBalance      1
#define CONFIG_SENSOR_Brightness        1
#define CONFIG_SENSOR_Contrast          1
#define CONFIG_SENSOR_Saturation        1
#define CONFIG_SENSOR_Effect            1
#define CONFIG_SENSOR_Scene             1
#define CONFIG_SENSOR_DigitalZoom       1
#define CONFIG_SENSOR_Focus             1
#define CONFIG_SENSOR_Exposure          1
#define CONFIG_SENSOR_Flash             1
#define CONFIG_SENSOR_Mirror            1
#define CONFIG_SENSOR_Flip              1

#define NUM_CTRLS                       12

/* Sensor Driver Configuration */
#define SENSOR_NAME                     "SP0A19"
#define SENSOR_V4L2_IDENT               64111
#define SENSOR_ID                       0xa6
#define SENSOR_MIN_WIDTH                640
#define SENSOR_MIN_HEIGHT               480
#define SENSOR_MAX_WIDTH                640
#define SENSOR_MAX_HEIGHT               480
#define SENSOR_INIT_WIDTH	            640
#define SENSOR_INIT_HEIGHT              480
#define SENSOR_INIT_WINSEQADR           sensor_vga
#define SENSOR_INIT_PIXFMT              V4L2_MBUS_FMT_YUYV8_2X8

#define SENSOR_WIDTH                    640
#define SENSOR_HEIGHT                   480


#define CONFIG_SENSOR_I2C_SPEED         100000       /* Hz */
/* Sensor write register continues by preempt_disable/preempt_enable for current process not be scheduled */
#define CONFIG_SENSOR_I2C_NOSCHED       0
#define CONFIG_SENSOR_I2C_RDWRCHK       0

#define SENSOR_NAME_STRING(a)           STR(CONS(SENSOR_NAME, a))
#define SENSOR_NAME_VARFUN(a)           CONS(SENSOR_NAME, a)

#define SENSOR_AF_IS_ERR                (0x00<<0)
#define SENSOR_AF_IS_OK		            (0x01<<0)
#define SENSOR_INIT_IS_ERR              (0x00<<28)
#define SENSOR_INIT_IS_OK               (0x01<<28)

//AE target
#define  SP0A19_P0_0xf7                 0x88
#define  SP0A19_P0_0xf8                 0x80
#define  SP0A19_P0_0xf9                 0x70
#define  SP0A19_P0_0xfa                 0x68
//HEQ
#define  SP0A19_P0_0xdd                 0x80
#define  SP0A19_P0_0xde                 0xa8

/********************************************************************************
 * predefine regs
 */
#define ENDMARKER { 0xff, 0xff }

struct reginfo {
    u8 reg;
    u8 val;
};

/* init 640X480 VGA */
static struct reginfo sensor_init_data[] =
{
    //init
    {0xfd,0x00},
    {0x1C,0x28},
    {0x32,0x00},
    {0x0f,0x2f},
    {0x10,0x2e},
    {0x11,0x00},
    {0x12,0x18},
    {0x13,0x2f},
    {0x14,0x00},
    {0x15,0x3f},
    {0x16,0x00},
    {0x17,0x18},
    {0x25,0x40},
    {0x1a,0x0b},
    {0x1b,0xc },
    {0x1e,0xb },
    {0x20,0x3f},
    {0x21,0x13},
    {0x22,0x19},
    {0x26,0x1a},
    {0x27,0xab},
    {0x28,0xfd},
    {0x30,0x00},
    //{0x31,0x10},
    {0xfb,0x31},
    {0x1f,0x08},

    //Blacklevel
    {0xfd,0x00},
    {0x65,0x00},//06
    {0x66,0x00},//06
    {0x67,0x00},//06
    {0x68,0x00},//06

    //ae setting
    {0xfd,0x00},
    {0x03,0x01},
    {0x04,0x32},
    {0x06,0x00},
    {0x09,0x01},
    {0x0a,0x46},
    {0xf0,0x66},
    {0xf1,0x00},
    {0xfd,0x01},
    {0x90,0x0c},
    {0x92,0x01},
    {0x98,0x66},
    {0x99,0x00},
    {0x9a,0x01},
    {0x9b,0x00},

    //Status
    {0xfd,0x01},
    {0xce,0xc8},
    {0xcf,0x04},
    {0xd0,0xc8},
    {0xd1,0x04},

    {0xfd,0x01},
    {0xc4,0x56},
    {0xc5,0x8f},
    {0xca,0x30},
    {0xcb,0x45},
    {0xcc,0x70},
    {0xcd,0x48},
    {0xfd,0x00},

    //lsc  for st
    {0xfd,0x01},
    {0x35,0x15},
    {0x36,0x15},
    {0x37,0x15},
    {0x38,0x15},
    {0x39,0x15},
    {0x3a,0x15},
    {0x3b,0x13},
    {0x3c,0x15},
    {0x3d,0x15},
    {0x3e,0x15},
    {0x3f,0x15},
    {0x40,0x18},
    {0x41,0x00},
    {0x42,0x04},
    {0x43,0x04},
    {0x44,0x00},
    {0x45,0x00},
    {0x46,0x00},
    {0x47,0x00},
    {0x48,0x00},
    {0x49,0xfd},
    {0x4a,0x00},
    {0x4b,0x00},
    {0x4c,0xfd},
    {0xfd,0x00},
    //awb 1
    {0xfd,0x01},
    {0x28,0xc5},
    {0x29,0x9b},
    {0x2e,0x02},
    {0x2f,0x16},
    {0x17,0x17},
    {0x18,0x19},
    {0x19,0x45},
    {0x2a,0xef},
    {0x2b,0x15},
    //awb2
    {0xfd,0x01},
    {0x73,0x80},
    {0x1a,0x80},
    {0x1b,0x80},
    //d65
    {0x65,0xd5},
    {0x66,0xfa},
    {0x67,0x72},
    {0x68,0x8a},
    //indoor
    {0x69,0xc6},
    {0x6a,0xee},
    {0x6b,0x94},
    {0x6c,0xab},
    //f
    {0x61,0x7a},
    {0x62,0x98},
    {0x63,0xc5},
    {0x64,0xe6},
    //cwf
    {0x6d,0xb9},
    {0x6e,0xde},
    {0x6f,0xb2},
    {0x70,0xd5},

    //////////////////skin detect//////
    {0xfd,0x01},
    {0x08,0x15},
    {0x09,0x04},
    {0x0a,0x20},
    {0x0b,0x12},
    {0x0c,0x27},
    {0x0d,0x06},
    {0x0f,0x63},
    //BPC_grad
    {0xfd,0x00},
    {0x79,0xf0},
    {0x7a,0xf0},
    {0x7b,0xf0},
    {0x7c,0xf0},

    //smooth
    {0xfd,0x00},
    //单通道间平滑阈值
    {0x57,0x06},	//raw_dif_thr_outdoor
    {0x58,0x0d}, //raw_dif_thr_normal
    {0x56,0x10}, //raw_dif_thr_dummy
    {0x59,0x10}, //raw_dif_thr_lowlight
    //GrGb平滑阈值
    {0x89,0x06},	//raw_grgb_thr_outdoor
    {0x8a,0x0d}, //raw_grgb_thr_normal
    {0x9c,0x10}, //raw_grgb_thr_dummy
    {0x9d,0x10}, //raw_grgb_thr_lowlight

    //Gr\Gb之间平滑强度
    {0x81,0xe0},    //raw_gflt_fac_outdoor
    {0x82,0xe0}, //80//raw_gflt_fac_normal
    {0x83,0x80},    //raw_gflt_fac_dummy
    {0x84,0x40},    //raw_gflt_fac_lowlight
    //Gr、Gb单通道内平滑强度
    {0x85,0xe0}, //raw_gf_fac_outdoor
    {0x86,0xc0}, //raw_gf_fac_normal
    {0x87,0x80}, //raw_gf_fac_dummy
    {0x88,0x40}, //raw_gf_fac_lowlight
    //R、B平滑强度
    {0x5a,0xff},		 //raw_rb_fac_outdoor
    {0x5b,0xe0}, //40//raw_rb_fac_normal
    {0x5c,0x80}, 	 //raw_rb_fac_dummy
    {0x5d,0x00}, 	 //raw_rb_fac_lowlight

    //sharpen
    {0xfd,0x01},
    {0xe2,0x30},	//sharpen_y_base
    {0xe4,0xa0},	//sharpen_y_max

    {0xe5,0x04}, //rangek_neg_outdoor
    {0xd3,0x04}, //rangek_pos_outdoor
    {0xd7,0x04}, //range_base_outdoor

    {0xe6,0x04}, //rangek_neg_normal
    {0xd4,0x04}, //rangek_pos_normal
    {0xd8,0x04}, //range_base_normal

    {0xe7,0x08}, //rangek_neg_dummy
    {0xd5,0x08}, //rangek_pos_dummy
    {0xd9,0x08}, //range_base_dummy
    {0xd2,0x10}, //rangek_neg_lowlight
    {0xd6,0x10}, //rangek_pos_lowlight
    {0xda,0x10}, //range_base_lowlight
    {0xe8,0x20},//sharp_fac_pos_outdoor
    {0xec,0x35},//sharp_fac_neg_outdoor
    {0xe9,0x20},//sharp_fac_pos_nr
    {0xed,0x35},//sharp_fac_neg_nr
    {0xea,0x20},//sharp_fac_pos_dummy
    {0xef,0x30},//sharp_fac_neg_dummy
    {0xeb,0x10},//sharp_fac_pos_low
    {0xf0,0x20},//sharp_fac_neg_low

    //CCM
    {0xfd,0x01},
    {0xa0,0x80},//80(红色接近，肤色不理想)
    {0xa1,0x0 },//0
    {0xa2,0x0 },//0
    {0xa3,0xf3},//f0
    {0xa4,0x8e},//a6
    {0xa5,0x0 },//ea
    {0xa6,0x0 },//0
    {0xa7,0xe6},//e6
    {0xa8,0x9a},//9a
    {0xa9,0x0 },//0
    {0xaa,0x3 },//33
    {0xab,0xc },//c
    {0xfd,0x00},

    //gamma
    {0xfd,0x00},
    {0x8b,0x0 },//0
    {0x8c,0xC },//11
    {0x8d,0x19},//19
    {0x8e,0x2C},//28
    {0x8f,0x49},//46
    {0x90,0x61},//61
    {0x91,0x77},//78
    {0x92,0x8A},//8A
    {0x93,0x9B},//9B
    {0x94,0xA9},//A9
    {0x95,0xB5},//B5
    {0x96,0xC0},//C0
    {0x97,0xCA},//CA
    {0x98,0xD4},//D4
    {0x99,0xDD},//DD
    {0x9a,0xE6},//E6
    {0x9b,0xEF},//EF
    {0xfd,0x01},//01
    {0x8d,0xF7},//F7
    {0x8e,0xFF},//FF

    //rpc
    {0xfd,0x00},
    {0xe0,0x4c},
    {0xe1,0x3c},
    {0xe2,0x34},
    {0xe3,0x2e},
    {0xe4,0x2e},
    {0xe5,0x2c},
    {0xe6,0x2c},
    {0xe8,0x2a},
    {0xe9,0x2a},
    {0xea,0x2a},
    {0xeb,0x28},
    {0xf5,0x28},
    {0xf6,0x28},
    //ae min gain
    {0xfd,0x01},
    {0x94,0xa0},//rpc_max_indr
    {0x95,0x28},//1e//rpc_min_indr
    {0x9c,0xa0},//rpc_max_outdr
    {0x9d,0x28},//rpc_min_outdr

    //ae target
    {0xfd,0x00},
    {0xed,0x8c},//80
    {0xf7,0x88},//7c
    {0xf8,0x80},//70
    {0xec,0x7c},//6c
    {0xef,0x74},//99
    {0xf9,0x70},//90
    {0xfa,0x68},//80
    {0xee,0x64},//78
    //gray detect
    {0xfd,0x01},
    {0x30,0x40},
    {0x31,0x70},//
    {0x32,0x40},//80
    {0x33,0xef},//
    {0x34,0x05},//04
    {0x4d,0x2f},//40
    {0x4e,0x20},//
    {0x4f,0x16},//13
    //lowlight lum
    {0xfd,0x00},
    {0xb2,0x20},//lum_limit
    {0xb3,0x1f},//lum_set
    {0xb4,0x30},//black_vt
    {0xb5,0x45},//white_vt
    //saturation
    {0xfd,0x00},
    {0xbe,0xff},
    {0xbf,0x01},
    {0xc0,0xff},
    {0xc1,0xd8},
    {0xd3,0x88},//0x78
    {0xd4,0x88},//0x78
    {0xd6,0x80},//0x78
    {0xd7,0x60},//0x78
    //HEQ
    {0xfd,0x00},
    {0xdc,0x00},
    {0xdd,0x80},
    {0xde,0xa8},//80
    {0xdf,0x80},
    //func enable
    {0xfd,0x00},
    // psw0523 check
    {0x31,0x10},// bit4 PCLK inversion, bit5 mirror, bit6 upside down
    {0x32,0x15},//d
    {0x34,0x16},
    {0x33,0xef},
    {0x5f,0x51},
    ENDMARKER
};

/* 640X480 VGA */
static struct reginfo sensor_vga[] =
{
    {0xfd,0x00},
    ENDMARKER
};

static struct reginfo sensor_ClrFmt_YUYV[] =
{
    {0xfd, 0x00},
    {0x35, 0x00},
    ENDMARKER
};

static struct reginfo sensor_ClrFmt_UYVY[] =
{
    {0xfd, 0x00},
    {0x35, 0x40},
    ENDMARKER
};

static struct reginfo sensor_ClrFmt_YVYU[] =
{
    {0xfd, 0x00},
    {0x35, 0x01},
    ENDMARKER
};

static struct reginfo sensor_ClrFmt_VYUY[] =
{
    {0xfd, 0x00},
    {0x35, 0x41},
    ENDMARKER
};

#if CONFIG_SENSOR_WhiteBalance
static struct reginfo sensor_WhiteB_Auto[]=
{
    {0xfd,0x01},
    {0x28,0xc5},
    {0x29,0x9b},
    {0xfd,0x00},
    {0x32,0x15},   //awb & ae  opened
    {0xfd,0x00},
    ENDMARKER
};

/* Cloudy Colour Temperature : 6500K - 8000K  */
static struct reginfo sensor_WhiteB_Cloudy[]=
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xbf},
    {0x29,0x89},
    {0xfd,0x00},
    ENDMARKER
};

/* ClearDay Colour Temperature : 5000K - 6500K  *///Sunny
static struct reginfo sensor_WhiteB_ClearDay[]=
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xbc},
    {0x29,0x5d},
    {0xfd,0x00},
    ENDMARKER
};

/* Office Colour Temperature : 3500K - 5000K  */
static struct reginfo sensor_WhiteB_TungstenLamp1[]=
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0xaf},
    {0x29,0x99},
    {0xfd,0x00},
    ENDMARKER
};

/* Home Colour Temperature : 2500K - 3500K  */
static struct reginfo sensor_WhiteB_TungstenLamp2[]=
{
    {0xfd,0x00},
    {0x32,0x05},
    {0xfd,0x01},
    {0x28,0x89},
    {0x29,0xb8},
    {0xfd,0x00},
    ENDMARKER
};

static struct reginfo *sensor_WhiteBalanceSeqe[] = {
    sensor_WhiteB_Auto,
    sensor_WhiteB_TungstenLamp1,
    sensor_WhiteB_TungstenLamp2,
    sensor_WhiteB_ClearDay,
    sensor_WhiteB_Cloudy,
};
#endif

#if CONFIG_SENSOR_Brightness
static struct reginfo sensor_Brightness0[]=
{
    // Brightness -2
    {0xfd,0x00},
    {0xdc,0xe0},//level -2
    ENDMARKER
};

static struct reginfo sensor_Brightness1[]=
{
    // Brightness -1
    {0xfd,0x00},
    {0xdc,0xf0},//level -1
    ENDMARKER
};

static struct reginfo sensor_Brightness2[]=
{
    //  Brightness 0
    {0xfd,0x00},
    {0xdc,0x00},//level 0
    ENDMARKER
};

static struct reginfo sensor_Brightness3[]=
{
    // Brightness +1
    {0xfd,0x00},
    {0xdc,0x10},//level +1
    ENDMARKER
};

static struct reginfo sensor_Brightness4[]=
{
    //  Brightness +2
    {0xfd,0x00},
    {0xdc,0x20},//level +2
    ENDMARKER
};

static  struct reginfo sensor_Brightness5[]=
{
    //  Brightness +3
    {0xfd,0x00},
    {0xdc,0x30},//level +3
    ENDMARKER
};

static struct reginfo *sensor_BrightnessSeqe[] = {
    sensor_Brightness0,
    sensor_Brightness1,
    sensor_Brightness2,
    sensor_Brightness3,
    sensor_Brightness4,
    sensor_Brightness5,
};
#endif

#if CONFIG_SENSOR_Effect
static struct reginfo sensor_Effect_Normal[] =
{
    {0xfd, 0x00},
    {0x62, 0x00},
    {0x63, 0x80},
    {0x64, 0x80},
    ENDMARKER
};

static  struct reginfo sensor_Effect_WandB[] =//SEPIABLUE
{
    {0xfd, 0x00},
    {0x62, 0x20},
    {0x63, 0x80},
    {0x64, 0x80},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Sepia[] =
{
    {0xfd, 0x00},
    {0x62, 0x10},
    {0x63, 0xc0},
    {0x64, 0x20},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Negative[] =
{
    //Negative
    {0xfd, 0x00},
    {0x62, 0x04},
    {0x63, 0x80},
    {0x64, 0x80},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Bluish[] =//SEPIABLUE
{
    // Bluish
    {0xfd, 0x00},
    {0x62, 0x10},
    {0x63, 0x20},
    {0x64, 0xf0},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Green[] =//SEPIAGREEN
{
    //  Greenish
    {0xfd, 0x00},
    {0x62, 0x10},
    {0x63, 0x20},
    {0x64, 0x20},
    ENDMARKER
};

static struct reginfo *sensor_EffectSeqe[] = {
    sensor_Effect_Normal,
    sensor_Effect_WandB,
    sensor_Effect_Negative,
    sensor_Effect_Sepia,
    sensor_Effect_Bluish,
    sensor_Effect_Green,
};
#endif

#if CONFIG_SENSOR_Exposure
static  struct reginfo sensor_Exposure0[]=
{
    //level -3
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7-0x18+0x04},
    {0xf7,SP0A19_P0_0xf7-0x18},
    {0xf8,SP0A19_P0_0xf8-0x18},
    {0xec,SP0A19_P0_0xf8-0x18-0x04},
    {0xef,SP0A19_P0_0xf9-0x18+0x04},
    {0xf9,SP0A19_P0_0xf9-0x18},
    {0xfa,SP0A19_P0_0xfa-0x18},
    {0xee,SP0A19_P0_0xfa-0x18-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static  struct reginfo sensor_Exposure1[]=
{
    //level -2
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7-0x10+0x04},
    {0xf7,SP0A19_P0_0xf7-0x10},
    {0xf8,SP0A19_P0_0xf8-0x10},
    {0xec,SP0A19_P0_0xf8-0x10-0x04},
    {0xef,SP0A19_P0_0xf9-0x10+0x04},
    {0xf9,SP0A19_P0_0xf9-0x10},
    {0xfa,SP0A19_P0_0xfa-0x10},
    {0xee,SP0A19_P0_0xfa-0x10-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static  struct reginfo sensor_Exposure2[]=
{
    //level -1
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7-0x08+0x04},
    {0xf7,SP0A19_P0_0xf7-0x08},
    {0xf8,SP0A19_P0_0xf8-0x08},
    {0xec,SP0A19_P0_0xf8-0x08-0x04},
    {0xef,SP0A19_P0_0xf9-0x08+0x04},
    {0xf9,SP0A19_P0_0xf9-0x08},
    {0xfa,SP0A19_P0_0xfa-0x08},
    {0xee,SP0A19_P0_0xfa-0x08-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static  struct reginfo sensor_Exposure3[]=
{
    //level 0
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7+0x04},
    {0xf7,SP0A19_P0_0xf7},
    {0xf8,SP0A19_P0_0xf8},
    {0xec,SP0A19_P0_0xf8-0x04},
    {0xef,SP0A19_P0_0xf9+0x04},
    {0xf9,SP0A19_P0_0xf9},
    {0xfa,SP0A19_P0_0xfa},
    {0xee,SP0A19_P0_0xfa-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static  struct reginfo sensor_Exposure4[]=
{
    //level +1
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7+0x08+0x04},
    {0xf7,SP0A19_P0_0xf7+0x08},
    {0xf8,SP0A19_P0_0xf8+0x08},
    {0xec,SP0A19_P0_0xf8+0x08-0x04},
    {0xef,SP0A19_P0_0xf9+0x08+0x04},
    {0xf9,SP0A19_P0_0xf9+0x08},
    {0xfa,SP0A19_P0_0xfa+0x08},
    {0xee,SP0A19_P0_0xfa+0x08-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static  struct reginfo sensor_Exposure5[]=
{
    //level +2
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7+0x10+0x04},
    {0xf7,SP0A19_P0_0xf7+0x10},
    {0xf8,SP0A19_P0_0xf8+0x10},
    {0xec,SP0A19_P0_0xf8+0x10-0x04},
    {0xef,SP0A19_P0_0xf9+0x10+0x04},
    {0xf9,SP0A19_P0_0xf9+0x10},
    {0xfa,SP0A19_P0_0xfa+0x10},
    {0xee,SP0A19_P0_0xfa+0x10-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static  struct reginfo sensor_Exposure6[]=
{
    //level +3
    {0xfd,0x00},
    {0xed,SP0A19_P0_0xf7+0x18+0x04},
    {0xf7,SP0A19_P0_0xf7+0x18},
    {0xf8,SP0A19_P0_0xf8+0x18},
    {0xec,SP0A19_P0_0xf8+0x18-0x04},
    {0xef,SP0A19_P0_0xf9+0x18+0x04},
    {0xf9,SP0A19_P0_0xf9+0x18},
    {0xfa,SP0A19_P0_0xfa+0x18},
    {0xee,SP0A19_P0_0xfa+0x18-0x04},

    {0xfd, 0x00},
    ENDMARKER
};

static struct reginfo *sensor_ExposureSeqe[] = {
    sensor_Exposure0,
    sensor_Exposure1,
    sensor_Exposure2,
    sensor_Exposure3,
    sensor_Exposure4,
    sensor_Exposure5,
    sensor_Exposure6,
};
#endif

#if CONFIG_SENSOR_Saturation
static struct reginfo sensor_Saturation0[]=
{
    ENDMARKER
};

static struct reginfo sensor_Saturation1[]=
{
    ENDMARKER
};

static struct reginfo sensor_Saturation2[]=
{
    ENDMARKER
};
static struct reginfo *sensor_SaturationSeqe[] = {
    sensor_Saturation0,
    sensor_Saturation1,
    sensor_Saturation2,
};
#endif

#if CONFIG_SENSOR_Contrast
static struct reginfo sensor_Contrast0[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd-0x30},	//level -3
    {0xde, SP0A19_P0_0xde-0x30},
    ENDMARKER
};

static struct reginfo sensor_Contrast1[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd-0x20},	//level -2
    {0xde, SP0A19_P0_0xde-0x20},
    ENDMARKER
};

static struct reginfo sensor_Contrast2[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd-0x10},	//level -1
    {0xde, SP0A19_P0_0xde-0x10},
    ENDMARKER
};

static struct reginfo sensor_Contrast3[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd}, //level 0
    {0xde, SP0A19_P0_0xde},
    ENDMARKER
};

static struct reginfo sensor_Contrast4[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd+0x10},	//level +1
    {0xde, SP0A19_P0_0xde+0x10},
    ENDMARKER
};

static struct reginfo sensor_Contrast5[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd+0x20},	//level +2
    {0xde, SP0A19_P0_0xde+0x20},
    ENDMARKER
};

static struct reginfo sensor_Contrast6[]=
{
    {0xfd, 0x00},
    {0xdd, SP0A19_P0_0xdd+0x30},	//level +3
    {0xde, SP0A19_P0_0xde+0x30},
    ENDMARKER
};

static struct reginfo *sensor_ContrastSeqe[] = {
    sensor_Contrast0,
    sensor_Contrast1,
    sensor_Contrast2,
    sensor_Contrast3,
    sensor_Contrast4,
    sensor_Contrast5,
    sensor_Contrast6,
};
#endif

#if CONFIG_SENSOR_Mirror
static struct reginfo sensor_MirrorOn[]=
{
    ENDMARKER
};

static struct reginfo sensor_MirrorOff[]=
{
    ENDMARKER
};

static struct reginfo *sensor_MirrorSeqe[] = {
    sensor_MirrorOff,
    sensor_MirrorOn,
};
#endif

#if CONFIG_SENSOR_Flip
static struct reginfo sensor_FlipOn[]=
{
    ENDMARKER
};

static struct reginfo sensor_FlipOff[]=
{
    ENDMARKER
};

static struct reginfo *sensor_FlipSeqe[] = {
    sensor_FlipOff,
    sensor_FlipOn,
};
#endif

#if CONFIG_SENSOR_Scene//zyy
static struct reginfo sensor_SceneAuto[] =
{
    //capture preview daylight 24M 50hz 20-8FPS maxgain:0x70
    {0xfd,0x0 },
    {0xb2,0x20},
    {0xb3,0x1f},
    {0xfd,0x00},
    {0x03,0x01},
    {0x04,0x32},
    {0x06,0x00},
    {0x09,0x01},
    {0x0a,0x46},
    {0xf0,0x66},
    {0xf1,0x00},
    {0xfd,0x01},
    {0x90,0x0c},
    {0x92,0x01},
    {0x98,0x66},
    {0x99,0x00},
    {0x9a,0x01},
    {0x9b,0x00},
    {0xfd,0x01},
    {0xce,0xc8},
    {0xcf,0x04},
    {0xd0,0xc8},
    {0xd1,0x04},
    {0xfd,0x00},
    ENDMARKER
};

static struct reginfo sensor_SceneNight[] =
{
    //capture preview night 24M 50hz 20-6FPS maxgain:0x78
    {0xfd,0x0 },
    {0xb2,0x25},
    {0xb3,0x1f},
    {0xfd,0x00},
    {0x03,0x01},
    {0x04,0x32},
    {0x06,0x00},
    {0x09,0x01},
    {0x0a,0x46},
    {0xf0,0x66},
    {0xf1,0x00},
    {0xfd,0x01},
    {0x90,0x10},
    {0x92,0x01},
    {0x98,0x66},
    {0x99,0x00},
    {0x9a,0x01},
    {0x9b,0x00},
    {0xfd,0x01},
    {0xce,0x60},
    {0xcf,0x06},
    {0xd0,0x60},
    {0xd1,0x06},
    {0xfd,0x00},
    ENDMARKER
};
static struct reginfo *sensor_SceneSeqe[] = {
    sensor_SceneAuto,
    sensor_SceneNight,
};
#endif

#if CONFIG_SENSOR_DigitalZoom
static struct reginfo sensor_Zoom0[] =
{
    ENDMARKER
};

static struct reginfo sensor_Zoom1[] =
{
    ENDMARKER
};

static struct reginfo sensor_Zoom2[] =
{
    ENDMARKER
};

static struct reginfo sensor_Zoom3[] =
{
    ENDMARKER
};
static struct reginfo *sensor_ZoomSeqe[] = {
    sensor_Zoom0,
    sensor_Zoom1,
    sensor_Zoom2,
    sensor_Zoom3,
};
#endif

static struct reginfo sensor_power_down_sequence[] __attribute__((unused)) =
{
    {0x00,0x00},
    ENDMARKER
};

/********************************************************************************
 * structures
 */
/* only one fixed colorspace per pixelcode */
struct sensor_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

typedef struct sensor_info_priv_s
{
    int digitalzoom;
    bool snap2preview;
    bool video2preview;
    unsigned int winseqe_cur_addr;
    struct sensor_datafmt fmt;
    unsigned int funmodule_state;
} sensor_info_priv_t;

struct sensor
{
    struct v4l2_subdev          subdev;
    struct media_pad            pad;
    struct v4l2_ctrl_handler    hdl;
    sensor_info_priv_t          info_priv;
    int                         model;
    bool                        initialized;
    int                         width;
    int                         height;
#if CONFIG_SENSOR_I2C_NOSCHED
    atomic_t tasklock_cnt;
#endif

    /**
     * ctrls
     */
    /* menu */
#if CONFIG_SENSOR_WhiteBalance
    struct v4l2_ctrl *white_balance;
#endif

#if CONFIG_SENSOR_Effect
    struct v4l2_ctrl *effect;
#endif

#if CONFIG_SENSOR_Scene
    struct v4l2_ctrl *scene;
#endif

#if CONFIG_SENSOR_Flash
    struct v4l2_ctrl *flash;
#endif

    /* standard */
#if CONFIG_SENSOR_Brightness
    struct v4l2_ctrl *brightness;
#endif

#if CONFIG_SENSOR_Exposure
    struct v4l2_ctrl *exposure;
#endif

#if CONFIG_SENSOR_Saturation
    struct v4l2_ctrl *saturation;
#endif

#if CONFIG_SENSOR_Contrast
    struct v4l2_ctrl *contrast;
#endif

#if CONFIG_SENSOR_Mirror
    struct v4l2_ctrl *hflip;
#endif

#if CONFIG_SENSOR_Flip
    struct v4l2_ctrl *vflip;
#endif

#if CONFIG_SENSOR_DigitalZoom
    struct v4l2_ctrl *zoom_relative;
    struct v4l2_ctrl *zoom_absolute;
#endif

#if CONFIG_SENSOR_Focus
    struct v4l2_ctrl *focus_relative;
    struct v4l2_ctrl *focus_absolute;
#endif
};

/********************************************************************************
 * tables
 */
static const struct sensor_datafmt sensor_colour_fmts[] = {
    /* YCBYCR */
    {V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
    /* CBYCRY */
    {V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
    /* YCRYCB */
    {V4L2_MBUS_FMT_YVYU8_2X8, V4L2_COLORSPACE_JPEG},
    /* CRYCBY */
    {V4L2_MBUS_FMT_VYUY8_2X8, V4L2_COLORSPACE_JPEG}
};

/********************************************************************************
 * general functions
 */
static inline struct sensor *subdev_to_sensor(struct v4l2_subdev *subdev)
{
    return container_of(subdev, struct sensor, subdev);
}

static inline struct sensor *i2c_client_to_sensor(struct i2c_client *client)
{
    return container_of(i2c_get_clientdata(client), struct sensor, subdev);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct sensor, hdl)->subdev;
}

static int sensor_task_lock(struct i2c_client *client, int lock)
{
#if CONFIG_SENSOR_I2C_NOSCHED
    int cnt = 3;
    struct sensor *sensor = i2c_client_to_sensor(client);

    if (lock) {
        if (atomic_read(&sensor->tasklock_cnt) == 0) {
            while ((atomic_read(&client->adapter->bus_lock.count) < 1) && (cnt>0)) {
                SENSOR_TR("\n %s will obtain i2c in atomic, but i2c bus is locked! Wait...\n",SENSOR_NAME_STRING());
                msleep(35);
                cnt--;
            }
            if ((atomic_read(&client->adapter->bus_lock.count) < 1) && (cnt<=0)) {
                SENSOR_TR("\n %s obtain i2c fail in atomic!!\n",SENSOR_NAME_STRING());
                goto sensor_task_lock_err;
            }
            preempt_disable();
        }

        atomic_add(1, &sensor->tasklock_cnt);
    } else {
        if (atomic_read(&sensor->tasklock_cnt) > 0) {
            atomic_sub(1, &sensor->tasklock_cnt);

            if (atomic_read(&sensor->tasklock_cnt) == 0)
                preempt_enable();
        }
    }
    return 0;
sensor_task_lock_err:
    return -1;
#else
    return 0;
#endif

}

/* sensor register write */
static int sensor_write(struct i2c_client *client, u8 reg, u8 val)
{
    int err,cnt;
    u8 buf[2];
    struct i2c_msg msg[1];

    buf[0] = reg;
    buf[1] = val;

    msg->addr = client->addr;
    msg->flags = client->flags;
    msg->buf = buf;
    msg->len = sizeof(buf);

    cnt = 3;
    err = -EAGAIN;

    while ((cnt-- > 0) && (err < 0)) {
        err = i2c_transfer(client->adapter, msg, 1);

        if (err >= 0) {
            return 0;
        } else {
            SENSOR_TR("\n %s write reg(0x%x, val:0x%x) failed, try to write again!\n",SENSOR_NAME_STRING(),reg, val);
            udelay(10);
        }
    }

    return err;
}

/* sensor register read */
static int sensor_read(struct i2c_client *client, u8 reg, u8 *val)
{
    int err,cnt;
    u8 buf[1];
    struct i2c_msg msg[2];

    buf[0] = reg;

    msg[0].addr = client->addr;
    msg[0].flags = client->flags;
    msg[0].buf = buf;
    msg[0].len = sizeof(buf);

    msg[1].addr = client->addr;
    msg[1].flags = client->flags|I2C_M_RD;
    msg[1].buf = buf;
    msg[1].len = 1;

    cnt = 1;
    err = -EAGAIN;
    while ((cnt-- > 0) && (err < 0)) {
        err = i2c_transfer(client->adapter, msg, 2);

        if (err >= 0) {
            *val = buf[0];
            return 0;
        } else {
            SENSOR_TR("\n %s read reg(0x%x val:0x%x) failed, try to read again! \n",SENSOR_NAME_STRING(),reg, *val);
            udelay(10);
        }
    }

    return err;
}

/* write a array of registers  */
static int sensor_write_array(struct i2c_client *client, struct reginfo *regarray)
{
    int err;
    int i = 0;

    //for(i=0; i < sizeof(sensor_init_data) / 2;i++)
	while((regarray[i].reg != 0xff) || (regarray[i].val != 0xff))
    {
        err = sensor_write(client, regarray[i].reg, regarray[i].val);
        if (err != 0)
        {
            SENSOR_TR("%s..write failed current i = %d\n", SENSOR_NAME_STRING(),i);
            return err;
        }
		i++;
    }

    return 0;
}

#if CONFIG_SENSOR_I2C_RDWRCHK
static int sensor_check_array(struct i2c_client *client, struct reginfo *regarray)
{
    int ret;
    int i = 0;

    u8 value;

    SENSOR_DG("%s >>>>>>>>>>>>>>>>>>>>>>\n",__FUNCTION__);
    for(i = 0; i < sizeof(sensor_init_data) / 2; i++) {
        ret = sensor_read(client, regarray[i].reg, &value);
        if(ret !=0) {
            SENSOR_TR("read value failed\n");
        }
        if(regarray[i].val != value) {
            SENSOR_DG("%s reg[0x%x] check err,writte :0x%x  read:0x%x\n",__FUNCTION__,regarray[i].reg,regarray[i].val,value);
        }
    }

    return 0;
}
#endif

static const struct sensor_datafmt *sensor_find_datafmt(
        enum v4l2_mbus_pixelcode code, const struct sensor_datafmt *fmt,
        int n)
{
    int i;
    for (i = 0; i < n; i++)
        if (fmt[i].code == code)
            return fmt + i;

    return NULL;
}

static bool check_id(struct i2c_client *client)
{
    u8 id = 0;
    sensor_read(client, 0x02, &id);
    if (likely(id == SENSOR_ID))
        return true;

    printk(KERN_ERR "%s: invalid chip id 0x%x\n", __func__, id);
    return false;
}

/********************************************************************************
 * control functions
 */
#if CONFIG_SENSOR_Brightness
static int sensor_set_brightness(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_BrightnessSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_BrightnessSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Effect
static int sensor_set_effect(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_EffectSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_EffectSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Exposure
static int sensor_set_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_ExposureSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_ExposureSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Saturation
static int sensor_set_saturation(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_SaturationSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_SaturationSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Contrast
static int sensor_set_contrast(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_ContrastSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_ContrastSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Mirror
static int sensor_set_mirror(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_MirrorSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_MirrorSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Flip
static int sensor_set_flip(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_FlipSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_FlipSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Scene
static int sensor_set_scene(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_SceneSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_SceneSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_WhiteBalance
static int sensor_set_whiteBalance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int val = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, val);

    if (val < 0 || val >= ARRAY_SIZE(sensor_WhiteBalanceSeqe)) {
        printk(KERN_ERR "%s: invalid value %d\n", __func__, val);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_WhiteBalanceSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_DigitalZoom
static int sensor_set_digitalzoom(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor *sensor = subdev_to_sensor(sd);
    int val = ctrl->val;
    int ret;
    int digitalzoom_cur;

    printk("%s: val %d\n", __func__, val);

    digitalzoom_cur = sensor->info_priv.digitalzoom + val;

    if (digitalzoom_cur < 0 || digitalzoom_cur >= ARRAY_SIZE(sensor_ZoomSeqe)) {
        printk(KERN_ERR "%s: invalid value --> val %d, cur %d\n", __func__, val, digitalzoom_cur);
        return -EINVAL;
    }

    ret = sensor_write_array(client, sensor_ZoomSeqe[val]);
    if (ret) {
        printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
        return -EIO;
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_Flash
static int sensor_set_flash(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    int val = ctrl->val;

    printk("%s: val %d\n", __func__, val);

    return 0;
}
#endif

static int sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);

    switch (ctrl->id) {
#if CONFIG_SENSOR_Brightness
        case V4L2_CID_BRIGHTNESS:
            sensor_set_brightness(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Exposure
        case V4L2_CID_EXPOSURE:
            sensor_set_exposure(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Saturation
        case V4L2_CID_SATURATION:
            sensor_set_saturation(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Contrast
        case V4L2_CID_CONTRAST:
            sensor_set_contrast(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_WhiteBalance
        case V4L2_CID_DO_WHITE_BALANCE:
            sensor_set_whiteBalance(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Mirror
        case V4L2_CID_HFLIP:
            sensor_set_mirror(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Flip
        case V4L2_CID_VFLIP:
            sensor_set_flip(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Effect
        case V4L2_CID_EFFECT:
            sensor_set_effect(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Scene
        case V4L2_CID_SCENE:
            sensor_set_scene(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_DigitalZoom
        case V4L2_CID_ZOOM_ABSOLUTE:
            sensor_set_digitalzoom(sd, ctrl);
            break;
#endif

#if CONFIG_SENSOR_Flash
        case V4L2_CID_FLASH:
            sensor_set_flash(sd, ctrl);
            break;
#endif

        default:
            printk(KERN_ERR "%s: invalid ctrl id 0x%x\n", __func__, ctrl->id);
            break;
    }

    return 0;
}

static const struct v4l2_ctrl_ops sensor_ctrl_ops = {
    .s_ctrl = sensor_s_ctrl,
};

static const struct v4l2_ctrl_config sensor_custom_ctrls[] = {
    {
        .ops    = &sensor_ctrl_ops,
        .id     = V4L2_CID_EFFECT,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "Effect",
        .min    = 0,
        .max    = 5,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &sensor_ctrl_ops,
        .id     = V4L2_CID_SCENE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "Scene",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &sensor_ctrl_ops,
        .id     = V4L2_CID_FLASH,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "Flash",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }
};

static int sensor_initialize_ctrls(struct sensor *sensor)
{
    v4l2_ctrl_handler_init(&sensor->hdl, NUM_CTRLS);

#if CONFIG_SENSOR_WhiteBalance
    /* sensor->white_balance = v4l2_ctrl_new_std_menu(&sensor->hdl, &sensor_ctrl_ops, */
    /*         V4L2_CID_DO_WHITE_BALANCE, 4, 0, 0); */
    sensor->white_balance = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_DO_WHITE_BALANCE, 0, 4, 1, 0);
    if (!sensor->white_balance) {
        printk(KERN_ERR "%s: failed to create white_balance ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Effect
    sensor->effect = v4l2_ctrl_new_custom(&sensor->hdl, &sensor_custom_ctrls[0], NULL);
    if (!sensor->effect) {
        printk(KERN_ERR "%s: failed to create effect ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Scene
    sensor->scene = v4l2_ctrl_new_custom(&sensor->hdl, &sensor_custom_ctrls[1], NULL);
    if (!sensor->scene) {
        printk(KERN_ERR "%s: failed to create scene ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Flash
    sensor->flash = v4l2_ctrl_new_custom(&sensor->hdl, &sensor_custom_ctrls[2], NULL);
    if (!sensor->flash) {
        printk(KERN_ERR "%s: failed to create flash ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Brightness
    sensor->brightness = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_BRIGHTNESS, 0, 4, 1, 0);
    if (!sensor->brightness) {
        printk(KERN_ERR "%s: failed to create brightness ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if 0
#if CONFIG_SENSOR_Exposure
    sensor->exposure = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_EXPOSURE, 0, 6, 1, 0);
    if (!sensor->exposure) {
        printk(KERN_ERR "%s: failed to create exposure ctrl\n", __func__);
        return -ENOENT;
    }
#endif
#endif

#if CONFIG_SENSOR_Saturation
    sensor->saturation = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_SATURATION, 0, 2, 1, 0);
    if (!sensor->saturation) {
        printk(KERN_ERR "%s: failed to create saturation ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Contrast
    sensor->contrast = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_CONTRAST, -3, 3, 1, 0);
    if (!sensor->contrast) {
        printk(KERN_ERR "%s: failed to create contrast ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Mirror
    sensor->hflip = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_HFLIP, 0, 1, 1, 0);
    if (!sensor->hflip) {
        printk(KERN_ERR "%s: failed to create hflip ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Flip
    sensor->vflip = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_VFLIP, 0, 1, 1, 0);
    if (!sensor->vflip) {
        printk(KERN_ERR "%s: failed to create vflip ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_DigitalZoom
    sensor->zoom_relative = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_ZOOM_RELATIVE, -1, 1, 1, 0);
    if (!sensor->zoom_relative) {
        printk(KERN_ERR "%s: failed to create zoom relative ctrl\n", __func__);
        return -ENOENT;
    }
    sensor->zoom_absolute = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_ZOOM_ABSOLUTE, 0, 3, 1, 0);
    if (!sensor->zoom_absolute) {
        printk(KERN_ERR "%s: failed to create zoom absolute ctrl\n", __func__);
        return -ENOENT;
    }
#endif

#if CONFIG_SENSOR_Focus
    sensor->focus_relative = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_FOCUS_RELATIVE, -1, 1, 1, 0);
    if (!sensor->focus_relative) {
        printk(KERN_ERR "%s: failed to create focus relative ctrl\n", __func__);
        return -ENOENT;
    }
    sensor->focus_absolute = v4l2_ctrl_new_std(&sensor->hdl, &sensor_ctrl_ops,
            V4L2_CID_FOCUS_ABSOLUTE, 0, 255, 1, 125);
    if (!sensor->focus_absolute) {
        printk(KERN_ERR "%s: failed to create focus absolute ctrl\n", __func__);
        return -ENOENT;
    }
#endif

    sensor->subdev.ctrl_handler = &sensor->hdl;
    if (sensor->hdl.error) {
        printk(KERN_ERR "%s: ctrl handler error(%d)\n", __func__, sensor->hdl.error);
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
static int sensor_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *id)
{
    struct sensor *sensor = subdev_to_sensor(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
        return -EINVAL;

    if (id->match.addr != client->addr)
        return -ENODEV;

    id->ident = sensor->model;
    id->revision = 0;
    return 0;
}

static int sensor_s_power(struct v4l2_subdev *sd, int on)
{
    struct sensor *sensor = subdev_to_sensor(sd);

    printk("%s: on %d\n", __func__, on);

    if (on) {
        sensor->info_priv.winseqe_cur_addr  = (int)SENSOR_INIT_WINSEQADR;
        sensor->info_priv.fmt = sensor_colour_fmts[0];
        sensor->info_priv.funmodule_state |= SENSOR_INIT_IS_OK;
    } else {
        sensor->initialized = false;
    }

    return 0;
}

static struct v4l2_subdev_core_ops sensor_subdev_core_ops = {
    .g_chip_ident   = sensor_g_chip_ident,
    .s_power        = sensor_s_power,
    .s_ctrl         = v4l2_subdev_s_ctrl,
};

/**
 * video ops
 */
static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor *sensor = subdev_to_sensor(sd);
    int ret = 0;

    printk("%s: enable %d\n", __func__, enable);
    if (enable) {
        if (sensor_task_lock(client, 1) < 0)
            goto sensor_INIT_ERR;

        if (!sensor->initialized) {
            if (!check_id(client))
                return -EINVAL;

            ret = sensor_write_array(client, sensor_init_data);
            if (ret != 0) {
                printk(KERN_ERR "%s: failed to initialize\n", __func__);
                goto sensor_INIT_ERR;
            }
            sensor->initialized = true;
        }

        ret = sensor_write_array(client, (struct reginfo *)sensor->info_priv.winseqe_cur_addr);
        if (ret) {
            printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
            return ret;
        }

        ret = sensor_write_array(client, (struct reginfo *)sensor_ClrFmt_YVYU);
        if (ret) {
            printk(KERN_ERR "%s: failed to sensor_write_array() for format\n", __func__);
            return ret;
        }
        sensor_task_lock(client, 0);
    }

    return 0;

sensor_INIT_ERR:
    sensor->info_priv.funmodule_state &= ~SENSOR_INIT_IS_OK;
    sensor_task_lock(client, 0);
    return ret;
}

static int sensor_g_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
    struct sensor *sensor = subdev_to_sensor(sd);

    mf->width   = sensor->width;
    mf->height  = sensor->height;
    mf->code    = sensor->info_priv.fmt.code;
    mf->colorspace  = sensor->info_priv.fmt.colorspace;
    mf->field   = V4L2_FIELD_NONE;

    return 0;
}

static int sensor_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    struct sensor *sensor = subdev_to_sensor(sd);
    struct reginfo *winseqe_set_addr = NULL;
    const struct sensor_datafmt *fmt;

    printk("%s\n", __func__);

    fmt = sensor_find_datafmt(mf->code, sensor_colour_fmts, ARRAY_SIZE(sensor_colour_fmts));
    if (!fmt) {
        printk(KERN_ERR "%s: unsupported mbus format 0x%x\n", __func__, mf->code);
        return -EINVAL;
    }

    if (mf->width != SENSOR_WIDTH || mf->height != SENSOR_HEIGHT) {
        printk(KERN_ERR "%s: unsuppored wxh(%dx%d)\n", __func__, mf->width, mf->height);
        return -EINVAL;
    }

    if (sensor->info_priv.fmt.code != mf->code) {
        switch (mf->code) {
            case V4L2_MBUS_FMT_YUYV8_2X8:
                winseqe_set_addr = sensor_ClrFmt_YUYV;
                break;
            case V4L2_MBUS_FMT_UYVY8_2X8:
                winseqe_set_addr = sensor_ClrFmt_UYVY;
                break;
            case V4L2_MBUS_FMT_YVYU8_2X8:
                winseqe_set_addr = sensor_ClrFmt_YVYU;
                break;
            case V4L2_MBUS_FMT_VYUY8_2X8:
                winseqe_set_addr = sensor_ClrFmt_VYUY;
                break;
            default:
                printk("%s: invalid code 0x%x\n", __func__, mf->code);
                return -EINVAL;
        }

        sensor->info_priv.winseqe_cur_addr  = (int)winseqe_set_addr;
        sensor->info_priv.fmt.code = mf->code;
        sensor->info_priv.fmt.colorspace= mf->colorspace;
#if 0
        if (winseqe_set_addr != NULL) {
            ret = sensor_write_array(client, winseqe_set_addr);
            if (ret) {
                printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
                return ret;
            }
            sensor->info_priv.fmt.code = mf->code;
            sensor->info_priv.fmt.colorspace= mf->colorspace;
            SENSOR_DG("%s v4l2_mbus_code:%d set success!\n", SENSOR_NAME_STRING(),mf->code);
        } else {
            SENSOR_TR("%s v4l2_mbus_code:%d is invalidate!\n", SENSOR_NAME_STRING(),mf->code);
        }
#endif
    }

    sensor->width = mf->width;
    sensor->height = mf->height;

    return 0;
}

static const struct v4l2_subdev_video_ops sensor_subdev_video_ops = {
    .s_stream   = sensor_s_stream,
    .s_mbus_fmt	= sensor_s_mbus_fmt,
    .g_mbus_fmt	= sensor_g_mbus_fmt,
};

/**
 * pad ops
 */
static int sensor_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    struct v4l2_mbus_framefmt *mf = &fmt->format;
    return sensor_s_mbus_fmt(sd, mf);
}

static const struct v4l2_subdev_pad_ops sensor_subdev_pad_ops = {
    .set_fmt = sensor_s_fmt,
};

/**
 * subde vops
 */
static const struct v4l2_subdev_ops sensor_subdev_ops = {
    .core   = &sensor_subdev_core_ops,
    .video  = &sensor_subdev_video_ops,
    .pad    = &sensor_subdev_pad_ops,
};

/**
 * media_entity_operations
 */
static int sensor_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    printk("%s\n", __func__);
    return 0;
}

static const struct media_entity_operations sensor_media_ops = {
    .link_setup = sensor_link_setup,
};

/********************************************************************************
 * intialize
 */
static int sensor_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct sensor *sensor;
    struct v4l2_subdev *sd;
    int ret;

    sensor = kzalloc(sizeof(struct sensor), GFP_KERNEL);
    if (!sensor)
        return -ENOMEM;

    sensor->model = SENSOR_V4L2_IDENT;

    sd = &sensor->subdev;
    strcpy(sd->name, SENSOR_NAME);

    /* register subdev */
    v4l2_i2c_subdev_init(sd, client, &sensor_subdev_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    sensor->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    sd->entity.ops  = &sensor_media_ops;
    if (media_entity_init(&sd->entity, 1, &sensor->pad, 0)) {
        printk(KERN_ERR "%s: failed to media_entity_init()\n", __func__);
        kfree(sensor);
        return -ENOENT;
    }

    ret = sensor_initialize_ctrls(sensor);
    if (ret < 0) {
        printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
        kfree(sensor);
        return ret;
    }

    return 0;
}

static int sensor_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    v4l2_device_unregister_subdev(sd);
    v4l2_ctrl_handler_free(sd->ctrl_handler);
    media_entity_cleanup(&sd->entity);
    kfree(subdev_to_sensor(sd));
    return 0;
}

static const struct i2c_device_id sensor_id[] = {
    { SENSOR_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, sensor_id);

static struct i2c_driver sensor_i2c_driver = {
    .driver = {
        .name = SENSOR_NAME,
    },
    .probe      = sensor_probe,
    .remove     = sensor_remove,
    .id_table   = sensor_id,
};

module_i2c_driver(sensor_i2c_driver);

MODULE_DESCRIPTION("SoC camera driver fo sp0a19");
MODULE_AUTHOR("swpark(swpark@nexell.co.kr)");
MODULE_LICENSE("GPL");
