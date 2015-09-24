
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


#include <mach/soc.h>
#include <mach/platform.h>
#include <mach/devices.h>
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
#define CONFIG_SENSOR_Brightness        0
#define CONFIG_SENSOR_Contrast          0
#define CONFIG_SENSOR_Saturation        0
#define CONFIG_SENSOR_Effect            1
#define CONFIG_SENSOR_Scene             0
#define CONFIG_SENSOR_DigitalZoom       0
#define CONFIG_SENSOR_Focus             0
#define CONFIG_SENSOR_Exposure          0
#define CONFIG_SENSOR_Flash             0
#define CONFIG_SENSOR_Mirror            0
#define CONFIG_SENSOR_Flip              0

#define NUM_CTRLS                       12

/* Sensor Driver Configuration */
#define SENSOR_NAME                     "HM2057"
#define SENSOR_V4L2_IDENT               64112
#define SENSOR_ID                       0x2056
#define SENSOR_ID1                       0x2057
#define SENSOR_MIN_WIDTH                320
#define SENSOR_MIN_HEIGHT               240
#define SENSOR_MAX_WIDTH                1600
#define SENSOR_MAX_HEIGHT               1200
#define SENSOR_INIT_WIDTH	            640
#define SENSOR_INIT_HEIGHT              480
#define SENSOR_INIT_WINSEQADR           sensor_vga
#define SENSOR_INIT_PIXFMT              V4L2_MBUS_FMT_YUYV8_2X8

#define SENSOR_WIDTH                    640
#define SENSOR_HEIGHT                   480


#define CONFIG_SENSOR_I2C_SPEED         200000       /* Hz */
/* Sensor write register continues by preempt_disable/preempt_enable for current process not be scheduled */
#define CONFIG_SENSOR_I2C_NOSCHED       0
#define CONFIG_SENSOR_I2C_RDWRCHK       0

#define SENSOR_NAME_STRING(a)           STR(CONS(SENSOR_NAME, a))
#define SENSOR_NAME_VARFUN(a)           CONS(SENSOR_NAME, a)

//#define SENSOR_AF_IS_ERR                (0x00<<0)
//#define SENSOR_AF_IS_OK		            (0x01<<0)
#define SENSOR_INIT_IS_ERR              (0x00<<28)
#define SENSOR_INIT_IS_OK               (0x01<<28)



/********************************************************************************
 * predefine regs
 */
#define ENDMARKER { 0xffff, 0xff }

struct reginfo {
    u16 reg;
    u8 val;
};

/* init 640X480 VGA */
static  struct reginfo sensor_init_data[] =
{
//640x480 vga
{0x0005,0x00},
{0x0022,0x00},
{0x0004,0x10},
{0x0006,0x03},
{0x000D,0x11},
{0x000E,0x11},
{0x000F,0x10}, //00, Fix row length - 2012-06-27
{0x0011,0x02},
{0x0012,0x17},//1C
{0x0013,0x0A},//05
{0x0015,0x02},
{0x0016,0x80},
{0x0018,0x00},
{0x001D,0x40},
{0x0020,0x00},  //
{0x0025,0x00},
{0x0026,0x87},
{0x0027,0x10},
{0x0040,0x20},
{0x0053,0x0A},
{0x0044,0x06},
{0x0046,0xD8},
{0x004A,0x0A},
{0x004B,0x72},
{0x0075,0x01},
{0x002A,0x1F},
{0x0070,0x5F},
{0x0071,0xFF},
{0x0072,0x55},
{0x0073,0x50},
{0x0080,0xC8},
{0x0082,0xA2},
{0x0083,0xF0},
{0x0085,0x12},
{0x0086,0x02},
{0x0087,0x80},
{0x0088,0x6C},
{0x0089,0x2E},
{0x008A,0x7D},
{0x008D,0x20},
{0x0090,0x00},
{0x0091,0x10},
{0x0092,0x11},
{0x0093,0x12},
{0x0094,0x16},
{0x0095,0x08},
{0x0096,0x00},
{0x0097,0x10},
{0x0098,0x11},
{0x0099,0x12},
{0x009A,0x06},
{0x009B,0x34},
{0x00A0,0x00},
{0x00A1,0x04},
{0x011F,0xF7},
{0x0120,0x36},
{0x0121,0x83},
{0x0122,0x7B},
{0x0123,0xC2},
{0x0124,0xDE},
{0x0125,0xFF},
{0x0126,0x70},
{0x0128,0x1F},
{0x0132,0x10},
{0x0131,0xBD},
{0x0140,0x14},
{0x0141,0x0A},
{0x0142,0x14},
{0x0143,0x0A},
{0x0144,0x04},
{0x0145,0x00},
{0x0146,0x20},
{0x0147,0x0A},
{0x0148,0x10},
{0x0149,0x0C},
{0x014A,0x80},
{0x014B,0x80},
{0x014C,0x2E},
{0x014D,0x2E},
{0x014E,0x05},
{0x014F,0x05},
{0x0150,0x0D},
{0x0155,0x00},
{0x0156,0x10},
{0x0157,0x0A},
{0x0158,0x0A},
{0x0159,0x0A},
{0x015A,0x05},
{0x015B,0x05},
{0x015C,0x05},
{0x015D,0x05},
{0x015E,0x08},
{0x015F,0xFF},
{0x0160,0x50},
{0x0161,0x20},
{0x0162,0x14},
{0x0163,0x0A},
{0x0164,0x10},
{0x0165,0x0A},
{0x0166,0x0A},
{0x018C,0x24},
{0x018D,0x04},
{0x018E,0x00},
{0x018F,0x11},
{0x0190,0x80},
{0x0191,0x47},
{0x0192,0x48},
{0x0193,0x64},
{0x0194,0x32},
{0x0195,0xc8},
{0x0196,0x96},
{0x0197,0x64},
{0x0198,0x32},
{0x0199,0x14},
{0x019A,0x20},
{0x019B,0x14},
{0x01B0,0x55},
{0x01B1,0x0C},
{0x01B2,0x0A},
{0x01B3,0x10},
{0x01B4,0x0E},
{0x01BA,0x10},
{0x01BB,0x04},
{0x01D8,0x40},
{0x01DE,0x60},
{0x01E4,0x10},
{0x01E5,0x10},
{0x01F2,0x0C},
{0x01F3,0x14},
{0x01F8,0x04},
{0x01F9,0x0C},
{0x01FE,0x02},
{0x01FF,0x04},
{0x0220,0x00},
{0x0221,0xB0},
{0x0222,0x00},
{0x0223,0x80},
{0x0224,0x8E},
{0x0225,0x00},
{0x0226,0x88},
{0x022A,0x88},
{0x022B,0x00},
{0x022C,0x8C},
{0x022D,0x13},
{0x022E,0x0B},
{0x022F,0x13},
{0x0230,0x0B},
{0x0233,0x13},
{0x0234,0x0B},
{0x0235,0x28},
{0x0236,0x03},
{0x0237,0x28},
{0x0238,0x03},
{0x023B,0x28},
{0x023C,0x03},
{0x023D,0x5C},
{0x023E,0x02},
{0x023F,0x5C},
{0x0240,0x02},
{0x0243,0x5C},
{0x0244,0x02},
{0x0251,0x0E},
{0x0252,0x00},
{0x0280,0x0A},
{0x0282,0x14},
{0x0284,0x2A},
{0x0286,0x50},
{0x0288,0x60},
{0x028A,0x6D},
{0x028C,0x79},
{0x028E,0x82},
{0x0290,0x8A},
{0x0292,0x91},
{0x0294,0x9C},
{0x0296,0xA7},
{0x0298,0xBA},
{0x029A,0xCD},
{0x029C,0xE0},
{0x029E,0x2D},
{0x02A0,0x06},
{0x02E0,0x04},
{0x02C0,0x8F},  // ccm check start
{0x02C1,0x01},
{0x02C2,0x8F},
{0x02C3,0x07},
{0x02C4,0xE3},
{0x02C5,0x07},
{0x02C6,0xC1},
{0x02C7,0x07},
{0x02C8,0x70},
{0x02C9,0x01},
{0x02CA,0xD0},
{0x02CB,0x07},
{0x02CC,0xF7},
{0x02CD,0x07},
{0x02CE,0x5A},
{0x02CF,0x07},
{0x02D0,0xB0},
{0x02D1,0x01},
{0x0302,0x00},
{0x0303,0x00},
{0x0304,0x00},
{0x02F0,0x80},
{0x02F1,0x07},
{0x02F2,0x8E},
{0x02F3,0x00},
{0x02F4,0xF2},
{0x02F5,0x07},
{0x02F6,0xCC},
{0x02F7,0x07},
{0x02F8,0x16},
{0x02F9,0x00},
{0x02FA,0x1E},
{0x02FB,0x00},
{0x02FC,0x9D},
{0x02FD,0x07},
{0x02FE,0xA6},
{0x02FF,0x07},
{0x0300,0xBD},
{0x0301,0x00},
{0x0305,0x00},
{0x0306,0x00},
{0x0307,0x00},  // ccm check end
{0x032D,0x00},  // AWB check start 
{0x032E,0x01},
{0x032F,0x00},
{0x0330,0x01},
{0x0331,0x00},
{0x0332,0x01},
{0x0333,0x82},
{0x0334,0x00},
{0x0335,0x84},
{0x0336,0x00},
{0x0337,0x01},
{0x0338,0x00},
{0x0339,0x01},
{0x033A,0x00},
{0x033B,0x01},
{0x033E,0x04},
{0x033F,0x86},
{0x0340,0x30},
{0x0341,0x44},
{0x0342,0x4A},
{0x0343,0x42},
{0x0344,0x74},
{0x0345,0x4F},
{0x0346,0x67},
{0x0347,0x5C},
{0x0348,0x59},
{0x0349,0x67},
{0x034A,0x4D},
{0x034B,0x6E},
{0x034C,0x44},  // AWB check 1 end
{0x0350,0x80},  // AWB check 2 start
{0x0351,0x80},
{0x0352,0x18},
{0x0353,0x18},
{0x0354,0x6E},
{0x0355,0x4A},
{0x0356,0x73},
{0x0357,0xC0},
{0x0358,0x06},
{0x035A,0x06},
{0x035B,0xA0},
{0x035C,0x73},
{0x035D,0x50},
{0x035E,0xC0},
{0x035F,0xA0},
{0x0360,0x02},
{0x0361,0x18},
{0x0362,0x80},
{0x0363,0x6C},
{0x0364,0x00},
{0x0365,0xF0},
{0x0366,0x20},
{0x0367,0x0C},
{0x0369,0x00},
{0x036A,0x10},
{0x036B,0x10},
{0x036E,0x20},
{0x036F,0x00},
{0x0370,0x10},
{0x0371,0x18},
{0x0372,0x0C},
{0x0373,0x38},
{0x0374,0x3A},
{0x0375,0x13},
{0x0376,0x22}, //AWB check 2 end
{0x0380,0xFF}, //AE check start 1
{0x0381,0x4A},
{0x0382,0x36},
{0x038A,0x40},
{0x038B,0x08},
{0x038C,0xC1},
{0x038E,0x40},
{0x038F,0x04},
{0x0390,0x8C},
{0x0391,0x05}, //1st ;min AE step
{0x0392,0x02},
{0x0393,0x80},
{0x0395,0x21}, //AE check end 1
{0x0398,0x02}, //Frame rate check start 1
{0x0399,0x84},
{0x039A,0x03},
{0x039B,0x25},
{0x039C,0x03},
{0x039D,0xC6},
{0x039E,0x05},
{0x039F,0x08},
{0x03A0,0x06},
{0x03A1,0x4A},
{0x03A2,0x07},
{0x03A3,0x8C},
{0x03A4,0x0A},
{0x03A5,0x10},
{0x03A6,0x0C},
{0x03A7,0x0E},
{0x03A8,0x10},
{0x03A9,0x18},
{0x03AA,0x20},
{0x03AB,0x28},
{0x03AC,0x1E},
{0x03AD,0x1A},
{0x03AE,0x13},
{0x03AF,0x0C},
{0x03B0,0x0B},
{0x03B1,0x09},
{0x03B3,0x10},
{0x03B4,0x00},
{0x03B5,0x10},
{0x03B6,0x00},
{0x03B7,0xEA},
{0x03B8,0x00},
{0x03B9,0x3A},
{0x03BA,0x01},
{0x03BB,0x9F},
{0x03BC,0xCF},
{0x03BD,0xE7},
{0x03BE,0xF3},
{0x03BF,0x01},
{0x03D0,0xF8},
{0x03E0,0x04},
{0x03E1,0x01},
{0x03E2,0x04},
{0x03E4,0x10},
{0x03E5,0x12},
{0x03E6,0x00},
{0x03E8,0x21},
{0x03E9,0x23},
{0x03EA,0x01},
{0x03EC,0x21},
{0x03ED,0x23},
{0x03EE,0x01},
{0x03F0,0x20},
{0x03F1,0x22},
{0x03F2,0x00}, //Frame rate check end 1
{0x0420,0x84}, //BLC check start 1
{0x0421,0x00},
{0x0422,0x00},
{0x0423,0x83},
{0x0430,0x08},
{0x0431,0x28},
{0x0432,0x10},
{0x0433,0x08},
{0x0435,0x0C},
{0x0450,0xFF}, //Alpha mode control
{0x0451,0xE8},
{0x0452,0xC4},
{0x0453,0x88},
{0x0454,0x00},
{0x0458,0x70},
{0x0459,0x03},
{0x045A,0x00},
{0x045B,0x30},
{0x045C,0x00},
{0x045D,0x70},
{0x0466,0x14},
{0x047A,0x00},
{0x047B,0x00},
{0x0480,0x58},
{0x0481,0x06},
{0x0482,0x0C},
{0x04B0,0x50},
{0x04B6,0x30},
{0x04B9,0x10},
{0x04B3,0x10},
{0x04B1,0x8E},
{0x04B4,0x20},
{0x0540,0x00}, //Flicker step check 1
{0x0541,0x5A},//9D
{0x0542,0x00},
{0x0543,0x6C},//5A
{0x0391,0x5A},  //2nd ;min AE step
{0x0580,0x08},//01
{0x0581,0x0F},
{0x0582,0x04},
{0x0594,0x00},
{0x0595,0x04},
{0x05A9,0x03},
{0x05AA,0x40},
{0x05AB,0x80},
{0x05AC,0x0A},
{0x05AD,0x10},
{0x05AE,0x0C},
{0x05AF,0x0C},
{0x05B0,0x03},
{0x05B1,0x03},
{0x05B2,0x1C},
{0x05B3,0x02},
{0x05B4,0x00},
{0x05B5,0x0C},
{0x05B8,0x80},
{0x05B9,0x32},
{0x05BA,0x00},
{0x05BB,0x80},
{0x05BC,0x03},
{0x05BD,0x00},
{0x05BF,0x05},
{0x05C0,0x10},
{0x05C3,0x00},
{0x05C4,0x0C},
{0x05C5,0x20},
{0x05C7,0x01},
{0x05C8,0x14},
{0x05C9,0x54},
{0x05CA,0x14},
{0x05CB,0xE0},
{0x05CC,0x20},
{0x05CD,0x00},
{0x05CE,0x08},
{0x05CF,0x60},
{0x05D0,0x10},
{0x05D1,0x05},
{0x05D2,0x03},
{0x05D4,0x00},
{0x05D5,0x05},
{0x05D6,0x05},
{0x05D7,0x05},
{0x05D8,0x08},
{0x05DC,0x0C},
{0x05D9,0x00},
{0x05DB,0x00},
{0x05DD,0x0F},
{0x05DE,0x00},
{0x05DF,0x0A}, //
{0x05E0,0xA0}, // window control check start 1
{0x05E1,0x00},
{0x05E2,0xA0},
{0x05E3,0x00},
{0x05E4,0x04},
{0x05E5,0x00},
{0x05E6,0x83},
{0x05E7,0x02},
{0x05E8,0x06},
{0x05E9,0x00},
{0x05EA,0xE5},
{0x05EB,0x01}, // window control check end 1
{0x0660,0x04}, //SW lens check start 1
{0x0661,0x16},
{0x0662,0x04},
{0x0663,0x28},
{0x0664,0x04},
{0x0665,0x18},
{0x0666,0x04},
{0x0667,0x21},
{0x0668,0x04},
{0x0669,0x0C},
{0x066A,0x04},
{0x066B,0x25},
{0x066C,0x00},
{0x066D,0x12},
{0x066E,0x00},
{0x066F,0x80},
{0x0670,0x00},
{0x0671,0x0A},
{0x0672,0x04},
{0x0673,0x1D},
{0x0674,0x04},
{0x0675,0x1D},
{0x0676,0x00},
{0x0677,0x7E},
{0x0678,0x01},
{0x0679,0x47},
{0x067A,0x00},
{0x067B,0x73},
{0x067C,0x04},
{0x067D,0x14},
{0x067E,0x04},
{0x067F,0x28},
{0x0680,0x00},
{0x0681,0x22},
{0x0682,0x00},
{0x0683,0xA5},
{0x0684,0x00},
{0x0685,0x1E},
{0x0686,0x04},
{0x0687,0x1D},
{0x0688,0x04},
{0x0689,0x19},
{0x068A,0x04},
{0x068B,0x21},
{0x068C,0x04},
{0x068D,0x0A},
{0x068E,0x04},
{0x068F,0x25},
{0x0690,0x04},
{0x0691,0x15},
{0x0698,0x20},
{0x0699,0x20},
{0x069A,0x01},
{0x069C,0x22},
{0x069D,0x10},
{0x069E,0x10},
{0x069F,0x08},  //SW lens check end 1
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},
{0x0005,0x01},	 
           
};

/* 1600X1200 UXGA */
static const struct reginfo sensor_uxga[] =
{
{0x0006,0x00},

{0x000D,0x00},

{0x000E,0x00},

{0x011F,0x88},

{0x0125,0xDF},

{0x0126,0x70},

{0x0131,0xAC},

{0x0366,0x20},

{0x0398,0x01},

{0x0399,0xA8},

{0x039A,0x02},

{0x039B,0x12},

{0x039C,0x02},

{0x039D,0x7C},

{0x039E,0x03},

{0x039F,0x50},

{0x03A0,0x04},

{0x03A1,0x24},

{0x03A2,0x04},

{0x03A3,0xF8},

{0x03A4,0x06},

{0x03A5,0x50},

{0x03A6,0x1A},

{0x03A7,0x1A},

{0x03A8,0x1A},

{0x03A9,0x1A},

{0x03AA,0x1E},

{0x03AB,0x1E},

{0x03AC,0x17},

{0x03AD,0x19},

{0x03AE,0x14},

{0x03AF,0x13},

{0x03B0,0x14},

{0x03B1,0x14},

{0x0433,0x40},

{0x0435,0x50},

//{0x0541,0x69},

//{0x0543,0x7E},

{0x05E4,0x0A},

{0x05E5,0x00},

{0x05E6,0x49},

{0x05E7,0x06},

{0x05E8,0x0A},

{0x05E9,0x00},

{0x05EA,0xB9},

{0x05EB,0x04},

{0x0698,0x20},

{0x0699,0x20},

{0x0000,0x01},

{0x0100,0x01},

{0x0101,0x01},
ENDMARKER
};

/* 1280X1024 SXGA */
static const struct reginfo sensor_sxga[] =
{
{0x0006,0x00},
{0x000D,0x00},
{0x000E,0x00},
{0x011F,0x88},
{0x0125,0xDF},
{0x0126,0x70},
{0x0131,0xAC},
{0x0366,0x20},
{0x0398,0x01},
{0x0399,0xA8},
{0x039A,0x02},
{0x039B,0x12},
{0x039C,0x02},
{0x039D,0x7C},
{0x039E,0x03},
{0x039F,0x50},
{0x03A0,0x04},
{0x03A1,0x24},
{0x03A2,0x04},
{0x03A3,0xF8},
{0x03A4,0x06},
{0x03A5,0x50},
{0x03A6,0x1A},
{0x03A7,0x1A},
{0x03A8,0x1A},
{0x03A9,0x1A},
{0x03AA,0x1E},
{0x03AB,0x1E},
{0x03AC,0x17},
{0x03AD,0x19},
{0x03AE,0x14},
{0x03AF,0x13},
{0x03B0,0x14},
{0x03B1,0x14},
{0x0433,0x40},
{0x0435,0x50},
//{0x0541,0x69},
//{0x0543,0x7E},

{0x05E4,0x00},
{0x05E5,0x00},
{0x05E6,0x9F},
{0x05E7,0x05},
{0x05E8,0x00},
{0x05E9,0x00},
{0x05EA,0x57},
{0x05EB,0x04},

{0x0698,0x20},
{0x0699,0x20},
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},
ENDMARKER
};

//1024x768
static const struct reginfo sensor_xga[] =
{
{0x0006,0x00},
{0x000D,0x00},
{0x000E,0x00},
{0x011F,0x88},
{0x0125,0xDF},
{0x0126,0x70},
{0x0131,0xAC},
{0x0366,0x20},
{0x0398,0x01},
{0x0399,0xA8},
{0x039A,0x02},
{0x039B,0x12},
{0x039C,0x02},
{0x039D,0x7C},
{0x039E,0x03},
{0x039F,0x50},
{0x03A0,0x04},
{0x03A1,0x24},
{0x03A2,0x04},
{0x03A3,0xF8},
{0x03A4,0x06},
{0x03A5,0x50},
{0x03A6,0x1A},
{0x03A7,0x1A},
{0x03A8,0x1A},
{0x03A9,0x1A},
{0x03AA,0x1E},
{0x03AB,0x1E},
{0x03AC,0x17},
{0x03AD,0x19},
{0x03AE,0x14},
{0x03AF,0x13},
{0x03B0,0x14},
{0x03B1,0x14},
{0x0433,0x40},
{0x0435,0x50},
//{0x0541,0x69},
//{0x0543,0x7E},

{0x05E4,0x00},
{0x05E5,0x00},
{0x05E6,0x9F},
{0x05E7,0x05},
{0x05E8,0x00},
{0x05E9,0x00},
{0x05EA,0x57},
{0x05EB,0x04},

{0x0698,0x20},
{0x0699,0x20},
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},
ENDMARKER
/*
{0x0006,0x07},
{0x000D,0x00},
{0x000E,0x00},
{0x011F,0x88},
{0x0125,0xDF},
{0x0126,0x70},
{0x0131,0xAC},
{0x0366,0x20},
{0x0398,0x01},
{0x0399,0xA8},
{0x039A,0x02},
{0x039B,0x12},
{0x039C,0x02},
{0x039D,0x7C},
{0x039E,0x03},
{0x039F,0x50},
{0x03A0,0x04},
{0x03A1,0x24},
{0x03A2,0x04},
{0x03A3,0xF8},
{0x03A4,0x06},
{0x03A5,0x50},
{0x03A6,0x1A},
{0x03A7,0x1A},
{0x03A8,0x1A},
{0x03A9,0x1A},
{0x03AA,0x1E},
{0x03AB,0x1E},
{0x03AC,0x17},
{0x03AD,0x19},
{0x03AE,0x14},
{0x03AF,0x13},
{0x03B0,0x14},
{0x03B1,0x14},
{0x0433,0x40},
{0x0435,0x50},
//{0x0541,0x69},
//{0x0543,0x7E},

{0x05E4,0x0A},
{0x05E5,0x00},
{0x05E6,0x09},
{0x05E7,0x04},
{0x05E8,0x0A},
{0x05E9,0x00},
{0x05EA,0x09},
{0x05EB,0x03},

{0x0698,0x20},
{0x0699,0x20},
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},

ENDMARKER
*/
};


/* 800X600 SVGA*/
static const struct reginfo sensor_svga[] =
{
ENDMARKER
};


/* 640X480 VGA */
static const struct reginfo sensor_vga[] =
{
{0x0006,0x00},
{0x000D,0x11},
{0x000E,0x11},
//{{0x0027,0x18},
{0x011F,0x80},
{0x0125,0xFF},
{0x0126,0x70},
{0x0131,0xAD},
{0x0366,0x08},
{0x0433,0x10},
{0x0435,0x14},
{0x05E0,0xA0},
{0x05E1,0x00},
{0x05E2,0xA0},
{0x05E3,0x00},
{0x05E4,0x04},
{0x05E5,0x00},
{0x05E6,0x83},
{0x05E7,0x02},
{0x05E8,0x06},
{0x05E9,0x00},
{0x05EA,0xE5},
{0x05EB,0x01},
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},
{0xFFFF, 0x0}   //end flag
};
/* 320*240 QVGA */
static const struct reginfo sensor_qvga[] =
{
  {0x0006,0x00}, 
  {0x000D,0x11}, 
  {0x000E,0x11}, 
  {0x011F,0x80}, 
  {0x0125,0xFF}, 
  {0x0126,0x70}, 
  {0x0131,0xAD}, 
  {0x0366,0x08}, 
  {0x0433,0x10}, 
  {0x0435,0x14}, 
  {0x05E0,0x40}, 
  {0x05E1,0x01}, 
  {0x05E2,0x40}, 
  {0x05E3,0x01}, 
  {0x05E4,0x02}, 
  {0x05E5,0x00}, 
  {0x05E6,0x41}, 
  {0x05E7,0x01}, 
  {0x05E8,0x03}, 
  {0x05E9,0x00}, 
  {0x05EA,0xf2}, 
  {0x05EB,0x00}, 
  {0x0000,0x01}, 
  {0x0100,0x01}, 
  {0x0101,0x01}, 
    ENDMARKER
};

static  struct reginfo sensor_ClrFmt_YUYV[]=
{
    {0x0027, 0x10},//0x30
    ENDMARKER
};

static  struct reginfo sensor_ClrFmt_UYVY[]=
{
    {0x0027, 0x10},
    ENDMARKER
};

static  struct reginfo sensor_ClrFmt_YVYU[]=
{
    {0x0027, 0x38},
    ENDMARKER
};

static  struct reginfo sensor_ClrFmt_VYUY[]=
{
    {0x0027, 0x18},
    ENDMARKER
};



#if CONFIG_SENSOR_WhiteBalance
static struct reginfo sensor_WhiteB_Auto[]=
{
    {0x0380, 0xFF},  //AWB auto
    {0x0000, 0x01},
    {0x0100, 0x01},
    {0x0101, 0x01},
    ENDMARKER
};

/* Cloudy Colour Temperature : 6500K - 8000K  */
static struct reginfo sensor_WhiteB_Cloudy[]=
{
    {0x0380, 0xFD},
    {0x032D, 0x70},
    {0x032E, 0x01},
    {0x032F, 0x00},
    {0x0330, 0x01},
    {0x0331, 0x08},
    {0x0332, 0x01},
    {0x0101, 0xFF},
    ENDMARKER
};

/* ClearDay Colour Temperature : 5000K - 6500K  *///Sunny
static struct reginfo sensor_WhiteB_ClearDay[]=
{
    //Sunny
    {0x0380, 0xFD},
    {0x032D, 0x60},
    {0x032E, 0x01},
    {0x032F, 0x00},
    {0x0330, 0x01},
    {0x0331, 0x20},
    {0x0332, 0x01},
    {0x0101, 0xFF},
    ENDMARKER
};

/* Office Colour Temperature : 3500K - 5000K  */
static struct reginfo sensor_WhiteB_TungstenLamp1[]=
{
    //Office
    {0x0380, 0xFD},
    {0x032D, 0x50},
    {0x032E, 0x01},
    {0x032F, 0x00},
    {0x0330, 0x01},
    {0x0331, 0x30},
    {0x0332, 0x01},
    {0x0101, 0xFF},
    
    ENDMARKER
};

/* Home Colour Temperature : 2500K - 3500K  */
static struct reginfo sensor_WhiteB_TungstenLamp2[]=
{
    //Home
    {0x0380, 0xFD},
    {0x032D, 0x10},
    {0x032E, 0x01},
    {0x032F, 0x00},
    {0x0330, 0x01},
    {0x0331, 0xA0},
    {0x0332, 0x01},
    {0x0101, 0xFF},
    
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
    ENDMARKER
};

static struct reginfo sensor_Brightness1[]=
{

    ENDMARKER
};

static struct reginfo sensor_Brightness2[]=
{

    ENDMARKER
};

static struct reginfo sensor_Brightness3[]=
{

    ENDMARKER
};

static struct reginfo sensor_Brightness4[]=
{

    ENDMARKER
};

static  struct reginfo sensor_Brightness5[]=
{

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
    {0x0488, 0x10},
    {0x0486, 0x00},
    {0x0487, 0xFF},
    {0x0101, 0xFF},

    ENDMARKER
};

static  struct reginfo sensor_Effect_WandB[] =//SEPIABLUE
{
    {0x0488, 0x12},
    {0x0486, 0x00},
    {0x0487, 0xFF},
    {0x0101, 0xFF},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Sepia[] =
{
    {0x0488, 0x11},
    {0x0486, 0x40},
    {0x0487, 0x90},
    {0x0101, 0xFF},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Negative[] =
{
    {0x0488, 0x12},
    {0x0486, 0x00},
    {0x0487, 0xFF},
    {0x0101, 0xFF},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Bluish[] =//SEPIABLUE
{
    {0x0488, 0x11},
    {0x0486, 0xB0},
    {0x0487, 0x80},
    {0x0101, 0xFF},
    ENDMARKER
};

static  struct reginfo sensor_Effect_Green[] =//SEPIAGREEN
{
    {0x0488, 0x11},
    {0x0486, 0x60},
    {0x0487, 0x60},
    {0x0101, 0xFF},
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

    ENDMARKER
};

static  struct reginfo sensor_Exposure1[]=
{

    ENDMARKER
};

static  struct reginfo sensor_Exposure2[]=
{

    ENDMARKER
};

static  struct reginfo sensor_Exposure3[]=
{

    ENDMARKER
};

static  struct reginfo sensor_Exposure4[]=
{

    ENDMARKER
};

static  struct reginfo sensor_Exposure5[]=
{

    ENDMARKER
};

static  struct reginfo sensor_Exposure6[]=
{

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

    ENDMARKER
};

static struct reginfo sensor_Contrast1[]=
{

    ENDMARKER
};

static struct reginfo sensor_Contrast2[]=
{

    ENDMARKER
};

static struct reginfo sensor_Contrast3[]=
{

    ENDMARKER
};

static struct reginfo sensor_Contrast4[]=
{

    ENDMARKER
};

static struct reginfo sensor_Contrast5[]=
{

    ENDMARKER
};

static struct reginfo sensor_Contrast6[]=
{

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

    ENDMARKER
};

static struct reginfo sensor_SceneNight[] =
{

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
    unsigned int winseqe_cur_format;
    struct sensor_datafmt fmt;
    unsigned int funmodule_state;
} sensor_info_priv_t;

struct win_size {
    char                        *name;
    __u32                       width;
    __u32                       height;
    __u32                       exposure_line_width;
    __u32                       capture_maximum_shutter;
     struct reginfo    *win_regs;
     struct reginfo    *lsc_regs;
    unsigned int                *frame_rate_array;
};



struct sensor
{
    struct v4l2_subdev          subdev;
    struct media_pad            pad;
    struct v4l2_ctrl_handler    hdl;
    sensor_info_priv_t          info_priv;
	
    const struct win_size        *win;
	 const struct sensor_datafmt    *cfmt;
	
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


#define UXGA_WIDTH          1600
#define UXGA_HEIGHT         1200

#define SXGA_WIDTH          1280
#define SXGA_HEIGHT         1024

#define XGA_WIDTH          1024
#define XGA_HEIGHT         768

#define VGA_WIDTH           640
#define VGA_HEIGHT          480

#define QVGA_WIDTH           320
#define QVGA_HEIGHT          240

#define MAX_WIDTH    UXGA_WIDTH
#define MAX_HEIGHT   UXGA_HEIGHT


static unsigned int frame_rate_uxga[] = {12,};
static unsigned int frame_rate_sxga[] = {12,};
static unsigned int frame_rate_xga[] = {12,};
static unsigned int frame_rate_vga[] = {12,};
static unsigned int frame_rate_qvga[] = {12,};

/* 1600X1200 */
static  struct win_size win_uxga = {
    .name     = "UXGA",
    .width    = UXGA_WIDTH,
    .height   = UXGA_HEIGHT,
    .win_regs = sensor_uxga,
    .frame_rate_array = frame_rate_uxga,
};

/* 1280X1024*/
static  struct win_size win_sxga = {
    .name     = "SXGA",
    .width    = SXGA_WIDTH,
    .height   = SXGA_HEIGHT,
    .win_regs = sensor_uxga,
    .frame_rate_array = frame_rate_sxga,
};
/* 1024X768 */
static  struct win_size win_xga = {
    .name     = "XGA",
    .width    = XGA_WIDTH,
    .height   = XGA_HEIGHT,
    .win_regs = sensor_uxga,
    .frame_rate_array = frame_rate_xga,
};
/* 640X480 */
static  struct win_size win_vga = {
    .name     = "VGA",
    .width    = VGA_WIDTH,
    .height   = VGA_HEIGHT,
    .win_regs = sensor_uxga,
    .frame_rate_array = frame_rate_vga,
};
/* 320X240 */
static  struct win_size win_qvga = {
    .name     = "QVGA",
    .width    = QVGA_WIDTH,
    .height   = QVGA_HEIGHT,
    .win_regs = sensor_qvga,
    .frame_rate_array = frame_rate_qvga,
};



static  struct win_size *hm2057_win[] = {
    &win_uxga,
    &win_sxga,
    &win_xga,
    &win_vga,
    &win_qvga,
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
static const struct win_size *sensor_select_win(u32 width, u32 height)
{
	const struct win_size *win;
    int i;
    SENSOR_DG("%s .............\n",__func__);

    for (i = 0; i < ARRAY_SIZE(hm2057_win); i++) {
        win = hm2057_win[i];
        if (width == win->width && height == win->height)
            return win;
    }

    printk(KERN_ERR "%s: unsupported width, height (%dx%d)\n", __func__, width, height);
    return NULL;
}




/* sensor register write */
static int sensor_write(struct i2c_client *client, u16 reg, u8 val)
{
#if 0
    int err,cnt;
    u8 buf[3];
    struct i2c_msg msg[1];


    buf[0] = reg >> 8;
    buf[1] = reg & 0xFF;
    buf[2] = val;

    msg->addr = client->addr;
    msg->flags =0;
    msg->buf = buf;
    msg->len = 3;

    cnt = 3;
    err = -EAGAIN;

    while ((cnt-->0) && (err < 0)) {                       /* ddl@rock-chips.com :  Transfer again if transent is failed   */
        err = i2c_transfer(client->adapter, msg, 1);

        if (err >= 0) {
            return 0;
        } else {
            SENSOR_TR("\n %s write reg(0x%x, val:0x%x) failed, try to write again!\n",SENSOR_NAME_STRING(),reg, val);
            udelay(10);
        }
    }

    return err;
#else
  struct i2c_msg msg;
  unsigned char data[3];
  int ret;
  
  data[0] = (reg&0xff00)>>8;
  data[1] = (reg&0x00ff);
  data[2] = val;
  
  msg.addr = client->addr;
  msg.flags = 0;
  msg.len = 3;
  msg.buf = data;

  ret = i2c_transfer(client->adapter, &msg, 1);
  if (ret >= 0) {
    ret = 0;
  } else {
    printk("%s error! slave = 0x%x, addr = 0x%4x, value = 0x%4x\n ",__func__, client->addr, reg,val);
  }
  return ret;
#endif
	
}

/* sensor register read */
static int sensor_read(struct i2c_client *client, u16 reg, u8 *val)
{
#if 0
    int err,cnt;
    u8 buf[2];
    struct i2c_msg msg[2];


    buf[0] = (u8)reg >> 8;
    buf[1] = (u8)reg & 0xFF;

    msg[0].addr = client->addr;
    msg[0].flags = client->flags;
    msg[0].buf = buf;
    msg[0].len = sizeof(buf);
   // msg[0].scl_rate = CONFIG_SENSOR_I2C_SPEED;       /* ddl@rock-chips.com : 100kHz */
   // msg[0].read_type = 2;//0x55;   /* fpga i2c:0==I2C_NO_STOP : direct use number not enum for don't want include spi_fpga.h */

    msg[1].addr = client->addr;
    msg[1].flags = client->flags|I2C_M_RD;
    msg[1].buf = buf;
    msg[1].len = 1;
    //msg[1].scl_rate = CONFIG_SENSOR_I2C_SPEED;                       /* ddl@rock-chips.com : 100kHz */
   // msg[1].read_type = 2;                             /* fpga i2c:0==I2C_NO_STOP : direct use number not enum for don't want include spi_fpga.h */

    cnt = 3;
    err = -EAGAIN;
    while ((cnt-->0) && (err < 0)) {                       /* ddl@rock-chips.com :  Transfer again if transent is failed   */
        err = i2c_transfer(client->adapter, msg, 2);

        if (err >= 0) {
            *val = buf[0];
            return 0;
        } else {
        	SENSOR_TR("\n %s 0x%x read reg(0x%x val:0x%x) failed, try to read again! \n",SENSOR_NAME_STRING(),client->addr,reg, *val);
            udelay(10);
        }
    }

    return err;
#else
  unsigned char data[3];
  struct i2c_msg msg[2];
  int ret,cnt;
  
  data[0] = (reg&0xff00)>>8;
  data[1] = (reg&0x00ff);
  data[2] = 0xee;
  /*
   * Send out the register address...
   */ 
  msg[0].addr = client->addr;
  msg[0].flags = 0;
  msg[0].len = 2;
  msg[0].buf = &data[0];
  /*
   * ...then read back the result.
   */
  msg[1].addr = client->addr;
  msg[1].flags = I2C_M_RD;
  msg[1].len = 1;
  msg[1].buf = &data[2];

  cnt = 10;
    ret = -EAGAIN;
    while ((cnt-->0) && (ret < 0)) 
    {
	  ret = i2c_transfer(client->adapter, msg, 2);
	  if (ret >= 0) {
	    *val = data[2];
	    return 0;
	  } else {
	    msleep(10);
	    SENSOR_TR("%s error! slave = 0x%x, addr = 0x%4x, value = 0x%2x ret=%d\n ",__func__, client->addr, client->addr,*val,ret);
	  }
    }
  return ret;
  #endif
}

/* write a array of registers  */
static int sensor_write_array(struct i2c_client *client, struct reginfo *regarray)
{
    int err = 0, cnt;
    int i = 0;
#if CONFIG_SENSOR_I2C_RDWRCHK    
	u8 valchk;
#endif

	cnt = 0;
	if (sensor_task_lock(client, 1) < 0)
		goto sensor_write_array_end;
    while (regarray[i].reg != 0xFFFF)
    {
        err = sensor_write(client, regarray[i].reg, regarray[i].val);
	 
        if (err < 0)
        {
            if (cnt-- > 0) {
			    SENSOR_TR("%s..write failed current reg:0x%x, Write array again !\n", SENSOR_NAME_STRING(),regarray[i].reg);
				i = 0;
				continue;
            } else {
                SENSOR_TR("%s..write array failed!!!\n", SENSOR_NAME_STRING());
                err = -EPERM;
				goto sensor_write_array_end;
            }
        } else {
        #if CONFIG_SENSOR_I2C_RDWRCHK
		mdelay(10);
			sensor_read(client, regarray[i].reg, &valchk);
			if (valchk != regarray[i].val)
				SENSOR_TR("%s Reg:0x%x write(0x%x, 0x%x) fail\n",SENSOR_NAME_STRING(), regarray[i].reg, regarray[i].val, valchk);
		#endif
        }
        i++;
    }
	//mdelay(100);
	msleep(50);

sensor_write_array_end:
	sensor_task_lock(client,0);
	return err;
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
	int ret=0,pid=0;
	 unsigned char  value;
	
		
    ret = sensor_read(client, 0x0001, &value);
    if (ret != 0) {
        SENSOR_TR("read chip id high byte failed\n");
        return -ENODEV;
    }
    else
    {
		SENSOR_TR("read chip id high byte is %d\n",value);
     }
    pid = value << 8;

    ret = sensor_read(client, 0x0002, &value);
    if (ret != 0) {
        SENSOR_TR("read chip id low byte failed\n");
         return -ENODEV;
    }
    else
    {
		SENSOR_TR("read chip id low byte is %d\n",value);
     }
    pid |= (value & 0xff);
    SENSOR_DG("\n %s  pid = 0x%x\n", SENSOR_NAME_STRING(), pid);

    if ((pid == SENSOR_ID)||(pid == SENSOR_ID1)) {
      //  sensor->model = SENSOR_V4L2_IDENT;
       SENSOR_DG("====xiongbiao====hm2057 check_id is ok");
    } else {
        SENSOR_TR("error: %s mismatched   pid = 0x%x\n", SENSOR_NAME_STRING(), pid);
        SENSOR_DG("====xiongbiao====hm2057 check_id is error!");
         return -ENODEV;
    }
    return 0;
}

static int sensor_set_params(struct v4l2_subdev *sd, u32 *width, u32 *height, enum v4l2_mbus_pixelcode code)
{
   struct sensor *sensor = subdev_to_sensor(sd);
   struct i2c_client *client = v4l2_get_subdevdata(sd);
    const struct win_size *old_win, *new_win;
    int i,ret;
struct reginfo *winseqe_set_addr=NULL;
 const struct sensor_datafmt *fmt;
	
    SENSOR_DG("%s .............\n",__func__);
    /*
     * select format
     */
    sensor->cfmt = NULL;
    for (i = 0; i < ARRAY_SIZE(sensor_colour_fmts); i++) {
        if (code == sensor_colour_fmts[i].code) {
            sensor->cfmt = sensor_colour_fmts + i;
            break;
        }
    }
    if (!sensor->cfmt) {
        printk(KERN_ERR "Unsupported sensor format.\n");
        return -EINVAL;
    }

	if (sensor->cfmt->code != code) {
		switch (code)
		{
			case V4L2_MBUS_FMT_YUYV8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_YUYV;
				printk(KERN_ALERT "**lone** YUYV");
				break;
			}
			case V4L2_MBUS_FMT_UYVY8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_UYVY;
				printk(KERN_ALERT "**lone** UYVY");
				break;
			}
			case V4L2_MBUS_FMT_YVYU8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_YVYU;
				printk(KERN_ALERT "**lone** YVYU");
				break;
			}
			case V4L2_MBUS_FMT_VYUY8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_VYUY;
				printk(KERN_ALERT "**lone** VYUY");
				break;
			}
			default:
				break;
		}
		if (winseqe_set_addr != NULL) {
			
            		sensor_write_array(client, winseqe_set_addr);
					
			sensor->info_priv.fmt.code = code;
            		sensor->info_priv.fmt.colorspace= sensor->cfmt->colorspace;            
			SENSOR_DG("%s v4l2_mbus_code:%d set success!\n", SENSOR_NAME_STRING(),code);
		} else {
			SENSOR_TR("%s v4l2_mbus_code:%d is invalidate!\n", SENSOR_NAME_STRING(),code);
		}
	}

	

    /*
     * select win
     */
    old_win = sensor->win;
    new_win = sensor_select_win(*width, *height);
    if (!new_win) {
        printk(KERN_ERR "Unsupported win size %dx*d\n",*width,*height);
        return -EINVAL;
    }
    sensor->win = new_win;


    *width = sensor->win->width;
    *height = sensor->win->height;

    return 0;
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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: val %d\n", __func__, val);

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

    SENSOR_DG("%s: on %d\n", __func__, on);

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
 static int sensor_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{

    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor *sensor = subdev_to_sensor(sd);
    const struct sensor_datafmt *fmt;
    const struct v4l2_queryctrl *qctrl;
    struct reginfo *winseqe_set_addr=NULL;
    int ret=0, set_w,set_h;


	
    SENSOR_DG("===============%dx%d\n", mf->width,mf->height);




    fmt = sensor_find_datafmt(mf->code, sensor_colour_fmts,
				   ARRAY_SIZE(sensor_colour_fmts));
    if (!fmt) {
        ret = -EINVAL;
        goto sensor_s_fmt_end;
    }

    SENSOR_DG (KERN_ALERT "sensor->info_priv.fmt.code is 0x%02x", sensor->info_priv.fmt.code);
    SENSOR_DG (KERN_ALERT "mf->code is 0x%02x", mf->code);
    SENSOR_DG (KERN_ALERT "fmt->code is 0x%02x", fmt->code);
//	if (sensor->info_priv.fmt.code != mf->code) {
		switch (mf->code)
		{
			case V4L2_MBUS_FMT_YUYV8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_YUYV;
				SENSOR_DG(KERN_ALERT "**lone** YUYV");
				break;
			}
			case V4L2_MBUS_FMT_UYVY8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_UYVY;
				SENSOR_DG(KERN_ALERT "**lone** UYVY");
				break;
			}
			case V4L2_MBUS_FMT_YVYU8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_YVYU;
				SENSOR_DG(KERN_ALERT "**lone** YVYU");
				break;
			}
			case V4L2_MBUS_FMT_VYUY8_2X8:
			{
				winseqe_set_addr = sensor_ClrFmt_VYUY;
				SENSOR_DG(KERN_ALERT "**lone** VYUY");
				break;
			}
			
			default:
				break;
		}
		if (winseqe_set_addr != NULL) {
            		//sensor_write_array(client, winseqe_set_addr);
            		sensor->info_priv.winseqe_cur_format= (int)winseqe_set_addr;
			sensor->info_priv.fmt.code = mf->code;
            		sensor->info_priv.fmt.colorspace= mf->colorspace;            
			SENSOR_TR("%s v4l2_mbus_code:%d set success!\n", SENSOR_NAME_STRING(),mf->code);
		} else {
			SENSOR_TR("%s v4l2_mbus_code:%d is invalidate!\n", SENSOR_NAME_STRING(),mf->code);
		}
//	}

    set_w = mf->width;
    set_h = mf->height;
   SENSOR_DG("===============%dx%d\n", set_w,set_h);

 if (((set_w <= 320) && (set_h <= 240)) && sensor_qvga[0].reg)
    {
        winseqe_set_addr = sensor_qvga;
        set_w = 320;
        set_h = 240;
    }
    else if (((set_w <= 640) && (set_h <= 480)) && sensor_vga[0].reg)
    {
        winseqe_set_addr = sensor_vga;
        set_w = 640;
        set_h = 480;
    }
    else if (((set_w <= 800) && (set_h <= 600)) && sensor_svga[0].reg)
    {
        winseqe_set_addr = sensor_svga;
        set_w = 800;
        set_h = 600;
    }
	else if (((set_w <= 1024) && (set_h <= 768)) && sensor_xga[0].reg)
    {
        SENSOR_DG("\nsensor_s_fmt:sensor_xga\n");
        winseqe_set_addr = sensor_xga;
        set_w = 1280;
        set_h = 1024;
    }
    else if (((set_w <= 1280) && (set_h <= 1024)) && sensor_sxga[0].reg)
    {
        SENSOR_DG("\nsensor_s_fmt:sensor_sxga\n");
        winseqe_set_addr = sensor_sxga;
        set_w = 1280;
        set_h = 1024;
    }
    else if (((set_w <= 1600) && (set_h <= 1200)) && sensor_uxga[0].reg)
    {
        winseqe_set_addr = sensor_uxga;
        set_w = 1600;
        set_h = 1200;
    }
    else
    {
        winseqe_set_addr = SENSOR_INIT_WINSEQADR;               /* ddl@rock-chips.com : Sensor output smallest size if  isn't support app  */
        set_w = SENSOR_INIT_WIDTH;
        set_h = SENSOR_INIT_HEIGHT;
		SENSOR_TR("\n %s..%s Format is Invalidate. pix->width = %d.. pix->height = %d\n",SENSOR_NAME_STRING(),__FUNCTION__,mf->width,mf->height);
    }

    
	

        sensor->info_priv.winseqe_cur_addr  = (int)winseqe_set_addr;
        SENSOR_TR("\n%s..%s.. icd->width = %d.. icd->height %d\n",SENSOR_NAME_STRING(),__FUNCTION__,set_w,set_h);


    mf->width = set_w;
    mf->height = set_h;
sensor_s_fmt_end:
    return 0;
}



static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor *sensor = subdev_to_sensor(sd);
    int ret = 0;

    SENSOR_DG("%s: enable %d\n", __func__, enable);
    if (enable) {
        if (sensor_task_lock(client, 1) < 0)
            goto sensor_INIT_ERR;

        if (!sensor->initialized) {
		//	front_camera_power_enable(1);
            check_id(client);
             //   return -EINVAL;

            ret = sensor_write_array(client, sensor_init_data);
            if (ret != 0) {
                printk(KERN_ERR "%s: failed to initialize\n", __func__);
                goto sensor_INIT_ERR;
            }
	     else
	     	{
			 printk( "sensor_write_array  initialize is  ok\n");
		 }
		 
              sensor->initialized = true;
        }

        ret = sensor_write_array(client, (struct reginfo *)sensor->info_priv.winseqe_cur_addr);
        if (ret) {
            printk(KERN_ERR "%s: failed to sensor_write_array()\n", __func__);
            return ret;
        }

	  ret = sensor_write_array(client, (struct reginfo *)sensor->info_priv.winseqe_cur_format);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to hm2057_write_array win regs\n", __func__);
            return -EIO;
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
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    SENSOR_DG("%s .............\n",__func__);
    if (!sensor->win || !sensor->cfmt) {
        u32 width = VGA_WIDTH;
        u32 height = VGA_HEIGHT;
        int ret = sensor_set_params(sd, &width, &height, V4L2_MBUS_FMT_UYVY8_2X8);
        if (ret < 0) {
            dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
            return ret;
        }
    }
	
    mf->width   = sensor->width;
    mf->height  = sensor->height;
    mf->code    = sensor->info_priv.fmt.code;
    mf->colorspace  = sensor->info_priv.fmt.colorspace;
    mf->field   = V4L2_FIELD_NONE;

    return 0;
}
static int sensor_try_mbus_fmt(struct v4l2_subdev *sd,
        struct v4l2_mbus_framefmt *mf)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
     struct sensor *sensor = subdev_to_sensor(sd);
    const struct win_size *win;
    int i;
    SENSOR_DG("%s .............\n",__func__);
    /*
     * select suitable win
     */
    win = sensor_select_win(mf->width, mf->height);
    if (!win)
        return -EINVAL;

    mf->width   = win->width;
    mf->height  = win->height;
    mf->field   = V4L2_FIELD_NONE;


    for (i = 0; i < ARRAY_SIZE(sensor_colour_fmts); i++)
        if (mf->code == sensor_colour_fmts[i].code)
            break;

    if (i == ARRAY_SIZE(sensor_colour_fmts)) {
        /* Unsupported format requested. Propose either */
        if (sensor->cfmt) {
            /* the current one or */
            mf->colorspace = sensor->cfmt->colorspace;
            mf->code = sensor->cfmt->code;
        } else {
            /* the default one */
            mf->colorspace = sensor_colour_fmts[0].colorspace;
            mf->code = sensor_colour_fmts[0].code;
        }
    } else {
        /* Also return the colorspace */
        mf->colorspace	= sensor_colour_fmts[i].colorspace;
    }

    return 0;
}


static int sensor_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    SENSOR_DG("%s .............\n",__func__);
    if (fsize->index >= ARRAY_SIZE(hm2057_win)) {
        dev_err(&client->dev, "index(%d) is over range %d\n", fsize->index, ARRAY_SIZE(hm2057_win));
        return -EINVAL;
    }

    switch (fsize->pixel_format) {
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUV422P:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
            fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
            fsize->discrete.width = hm2057_win[fsize->index]->width;
            fsize->discrete.height = hm2057_win[fsize->index]->height;
            break;
        default:
            dev_err(&client->dev, "pixel_format(%d) is Unsupported\n", fsize->pixel_format);
            return -EINVAL;
    }

    dev_info(&client->dev, "type %d, width %d, height %d\n", V4L2_FRMSIZE_TYPE_DISCRETE, fsize->discrete.width, fsize->discrete.height);
    return 0;
}

static int sensor_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned int index,
        enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(sensor_colour_fmts))
        return -EINVAL;

    *code = sensor_colour_fmts[index].code;
    return 0;
}

static const struct v4l2_subdev_video_ops sensor_subdev_video_ops = {
    .s_stream   = sensor_s_stream,//
    .s_mbus_fmt	= sensor_s_mbus_fmt,//
    .g_mbus_fmt	= sensor_g_mbus_fmt,//

    .try_mbus_fmt           = sensor_try_mbus_fmt,//

    .enum_framesizes        = sensor_enum_framesizes,
    .enum_mbus_fmt          = sensor_enum_mbus_fmt,
};

/**
 * pad ops
 */
static int sensor_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    struct v4l2_mbus_framefmt *mf = &fmt->format;
	SENSOR_DG("%s: %dx%d\n", __func__, mf->width, mf->height);
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
    SENSOR_DG("%s\n", __func__);
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
    u8 val=0xff;
 struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);


    SENSOR_DG("%s:.......................................\n", __func__);
    sensor = kzalloc(sizeof(struct sensor), GFP_KERNEL);
    if (!sensor)
        return -ENOMEM;

    sensor->model = SENSOR_V4L2_IDENT;

    sd = &sensor->subdev;
    strcpy(sd->name, SENSOR_NAME);

    if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
        dev_warn(&adapter->dev,
        	 "I2C-Adapter doesn't support I2C_FUNC_I2C\n");
        return -EIO;
    }
	
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
