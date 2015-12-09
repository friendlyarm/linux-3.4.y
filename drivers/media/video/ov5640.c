/*
 * ov5640 Camera Driver
 *
 * Copyright (C) 2015 FriendlyARM (www.arm9.net)
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
#include <linux/delay.h>

#include "ov5640_af_firmware.h"

#define MODULE_NAME "ov5640"

#ifdef OV5640_DEBUG
#define assert(expr) \
	if (unlikely(!(expr))) {				\
		pr_err("Assertion failed! %s,%s,%s,line=%d\n",	\
#expr, __FILE__, __func__, __LINE__);	\
	}

#define OV5640_DEBUG(fmt,args...) printk(KERN_ALERT fmt, ##args)
#else

#define assert(expr) do {} while (0)

#define OV5640_DEBUG(fmt,args...)
#endif

#define OV5640              0x53
#define REG_DLY 0xffff
#define NUM_CTRLS           11
#define V4L2_IDENT_OV5640   64188

/* private ctrls */
#define V4L2_CID_SCENE_EXPOSURE         (V4L2_CTRL_CLASS_CAMERA | 0x1001)
#define V4L2_CID_PRIVATE_PREV_CAPT      (V4L2_CTRL_CLASS_CAMERA | 0x1002)

enum {
	V4L2_WHITE_BALANCE_INCANDESCENT = 0,
	/*V4L2_WHITE_BALANCE_FLUORESCENT,*/
	V4L2_WHITE_BALANCE_DAYLIGHT,
	V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT,
	/*V4L2_WHITE_BALANCE_TUNGSTEN*/
};

struct regval_list {
	u16 reg;
	u8  val;
};

static int ov5640_video_probe(struct i2c_client *client);

/****************************************************************************************
 * predefined reg values
 */
#define ENDMARKER { 0xffff, 0xff }
#define DELAYMARKER { 0xfffe, 0xff }

static struct regval_list ov5640_fmt_yuv422_yuyv[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_fmt_yuv422_yvyu[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_fmt_yuv422_vyuy[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_fmt_yuv422_uyvy[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_fmt_raw[] __attribute__((unused)) =
{
	ENDMARKER,
};

/*
 *AWB
 */
static const struct regval_list ov5640_awb_regs_enable[] =
{
	ENDMARKER,
};

static const struct regval_list ov5640_awb_regs_diable[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_wb_cloud_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_wb_daylight_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_wb_incandescence_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_wb_fluorescent_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_wb_tungsten_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_colorfx_none_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_colorfx_bw_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_colorfx_sepia_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_colorfx_negative_regs[] =
{
	ENDMARKER,
};

static struct regval_list ov5640_whitebance_auto[] __attribute__((unused)) =
{
	ENDMARKER,
};

static struct regval_list ov5640_whitebance_cloudy[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5640_whitebance_sunny[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5640_whitebance_fluorescent[] __attribute__((unused)) =
{
	ENDMARKER,

};
static  struct regval_list ov5640_whitebance_incandescent[] __attribute__((unused)) =
{
	ENDMARKER,
};


static  struct regval_list ov5640_effect_normal[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5640_effect_white_black[] __attribute__((unused)) =
{
	ENDMARKER,
};

/* Effect */
static  struct regval_list ov5640_effect_negative[] __attribute__((unused)) =
{
	ENDMARKER,
};
/*¸´¹ÅÐ§¹û*/
static  struct regval_list ov5640_effect_antique[] __attribute__((unused)) =
{
	ENDMARKER,
};

/* Scene */
static  struct regval_list ov5640_scene_auto[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5640_scene_night[] __attribute__((unused)) =
{
	ENDMARKER,
};

static const struct regval_list ov5640_init_regs[] = {
	{0x3103, 0x11},
	//	{0x3008, 0x82},
	{0x3008, 0x42},
	{0x3103, 0x03},
	{0x3017, 0xff},
	{0x3018, 0xff},
	{0x3034, 0x1a},
	{0x3035, 0x21},
	{0x3036, 0x46},
	{0x3037, 0x13},
	{0x3108, 0x01},
	{0x3630, 0x36},
	{0x3631, 0x0e},
	{0x3632, 0xe2},
	{0x3633, 0x12},
	{0x3621, 0xe0},
	{0x3704, 0xa0},
	{0x3703, 0x5a},
	{0x3715, 0x78},
	{0x3717, 0x01},
	{0x370b, 0x60},
	{0x3705, 0x1a},
	{0x3905, 0x02},
	{0x3906, 0x10},
	{0x3901, 0x0a},
	{0x3731, 0x12},
	{0x3600, 0x08},
	{0x3601, 0x33},
	{0x302d, 0x60},
	{0x3620, 0x52},
	{0x371b, 0x20},
	{0x471c, 0x50},
	{0x3a13, 0x43},
	{0x3a18, 0x00},
	{0x3a19, 0xf8},
	{0x3635, 0x13},
	{0x3636, 0x03},
	{0x3634, 0x40},
	{0x3622, 0x01},
	{0x3c01, 0x34},
	{0x3c04, 0x28},
	{0x3c05, 0x98},
	{0x3c06, 0x00},
	{0x3c07, 0x08},
	{0x3c08, 0x00},
	{0x3c09, 0x1c},
	{0x3c0a, 0x9c},
	{0x3c0b, 0x40},
	{0x3820, 0x41},
	{0x3821, 0x07},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x0a},
	{0x3805, 0x3f},
	{0x3806, 0x07},
	{0x3807, 0x9b},
	{0x3808, 0x03},
	{0x3809, 0x20},
	{0x380a, 0x02},
	{0x380b, 0x58},
	{0x380c, 0x07},
	{0x380d, 0x68},
	{0x380e, 0x03},
	{0x380f, 0xd8},
	{0x3810, 0x00},
	{0x3811, 0x10},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3618, 0x00},
	{0x3612, 0x29},
	{0x3708, 0x64},
	{0x3709, 0x52},
	{0x370c, 0x03},
	{0x3a02, 0x03},
	{0x3a03, 0xd8},
	{0x3a08, 0x01},
	{0x3a09, 0x27},
	{0x3a0a, 0x00},
	{0x3a0b, 0xf6},
	{0x3a0e, 0x03},
	{0x3a0d, 0x04},
	{0x3a14, 0x03},
	{0x3a15, 0xd8},
	{0x4001, 0x02},
	{0x4004, 0x02},
	{0x3000, 0x00},
	{0x3002, 0x1c},
	{0x3004, 0xff},
	{0x3006, 0xc3},
	{0x300e, 0x58},
	{0x302e, 0x00},
	{0x4740, 0x20},
	{0x4300, 0x32},	//0x30 YUYV  0x31 YVYU 0x33 VYUY 0x32 UYVY
	{0x501f, 0x00},
	{0x4713, 0x03},
	{0x4407, 0x04},
	{0x440e, 0x00},
	{0x460b, 0x35},
	{0x460c, 0x20},
	{0x4837, 0x22},
	{0x3824, 0x02},
	{0x5000, 0xa7},
	{0x5001, 0xa3},
	{0x5180, 0xff},
	{0x5181, 0xf2},
	{0x5182, 0x00},
	{0x5183, 0x14},
	{0x5184, 0x25},
	{0x5185, 0x24},
	{0x5186, 0x09},
	{0x5187, 0x09},
	{0x5188, 0x09},
	{0x5189, 0x75},
	{0x518a, 0x54},
	{0x518b, 0xe0},
	{0x518c, 0xb2},
	{0x518d, 0x42},
	{0x518e, 0x3d},
	{0x518f, 0x56},
	{0x5190, 0x46},
	{0x5191, 0xf8},
	{0x5192, 0x04},
	{0x5193, 0x70},
	{0x5194, 0xf0},
	{0x5195, 0xf0},
	{0x5196, 0x03},
	{0x5197, 0x01},
	{0x5198, 0x04},
	{0x5199, 0x12},
	{0x519a, 0x04},
	{0x519b, 0x00},
	{0x519c, 0x06},
	{0x519d, 0x82},
	{0x519e, 0x38},
	{0x5381, 0x1e},
	{0x5382, 0x5b},
	{0x5383, 0x08},
	{0x5384, 0x0a},
	{0x5385, 0x7e},
	{0x5386, 0x88},
	{0x5387, 0x7c},
	{0x5388, 0x6c},
	{0x5389, 0x10},
	{0x538a, 0x01},
	{0x538b, 0x98},
	{0x5300, 0x08},
	{0x5301, 0x30},
	{0x5302, 0x10},
	{0x5303, 0x00},
	{0x5304, 0x08},
	{0x5305, 0x30},
	{0x5306, 0x08},
	{0x5307, 0x16},
	{0x5309, 0x08},
	{0x530a, 0x30},
	{0x530b, 0x04},
	{0x530c, 0x06},
	{0x5480, 0x01},
	{0x5481, 0x08},
	{0x5482, 0x14},
	{0x5483, 0x28},
	{0x5484, 0x51},
	{0x5485, 0x65},
	{0x5486, 0x71},
	{0x5487, 0x7d},
	{0x5488, 0x87},
	{0x5489, 0x91},
	{0x548a, 0x9a},
	{0x548b, 0xaa},
	{0x548c, 0xb8},
	{0x548d, 0xcd},
	{0x548e, 0xdd},
	{0x548f, 0xea},
	{0x5490, 0x1d},
	{0x5580, 0x02},
	{0x5583, 0x40},
	{0x5584, 0x10},
	{0x5589, 0x10},
	{0x558a, 0x00},
	{0x558b, 0xf8},
	{0x5800, 0x23},
	{0x5801, 0x14},
	{0x5802, 0x0f},
	{0x5803, 0x0f},
	{0x5804, 0x12},
	{0x5805, 0x26},
	{0x5806, 0x0c},
	{0x5807, 0x08},
	{0x5808, 0x05},
	{0x5809, 0x05},
	{0x580a, 0x08},
	{0x580b, 0x0d},
	{0x580c, 0x08},
	{0x580d, 0x03},
	{0x580e, 0x00},
	{0x580f, 0x00},
	{0x5810, 0x03},
	{0x5811, 0x09},
	{0x5812, 0x07},
	{0x5813, 0x03},
	{0x5814, 0x00},
	{0x5815, 0x01},
	{0x5816, 0x03},
	{0x5817, 0x08},
	{0x5818, 0x0d},
	{0x5819, 0x08},
	{0x581a, 0x05},
	{0x581b, 0x06},
	{0x581c, 0x08},
	{0x581d, 0x0e},
	{0x581e, 0x29},
	{0x581f, 0x17},
	{0x5820, 0x11},
	{0x5821, 0x11},
	{0x5822, 0x15},
	{0x5823, 0x28},
	{0x5824, 0x46},
	{0x5825, 0x26},
	{0x5826, 0x08},
	{0x5827, 0x26},
	{0x5828, 0x64},
	{0x5829, 0x26},
	{0x582a, 0x24},
	{0x582b, 0x22},
	{0x582c, 0x24},
	{0x582d, 0x24},
	{0x582e, 0x06},
	{0x582f, 0x22},
	{0x5830, 0x40},
	{0x5831, 0x42},
	{0x5832, 0x24},
	{0x5833, 0x26},
	{0x5834, 0x24},
	{0x5835, 0x22},
	{0x5836, 0x22},
	{0x5837, 0x26},
	{0x5838, 0x44},
	{0x5839, 0x24},
	{0x583a, 0x26},
	{0x583b, 0x28},
	{0x583c, 0x42},
	{0x583d, 0xce},
	{0x5025, 0x00},
	{0x3a0f, 0x30},
	{0x3a10, 0x28},
	{0x3a1b, 0x30},
	{0x3a1e, 0x26},
	{0x3a11, 0x60},
	{0x3a1f, 0x14},

	{0x3008, 0x02},
	//	{0x4730, 0x01},
	ENDMARKER,
};

static const struct regval_list ov5640_qcif_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov5640_cif_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov5640_qvga_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov5640_svga_regs[] = {
	{0x3503, 0x00},
	{0x3c07, 0x08},
	{0x3820, 0x41},
	{0x3821, 0x07},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3803, 0x04},
	{0x3806, 0x07},///
	{0x3807, 0x9b},
	{0x3808, 0x03},
	{0x3809, 0x20},
	{0x380a, 0x02},
	{0x380b, 0x58},
	{0x380c, 0x07},
	{0x380d, 0x68},
	{0x380e, 0x03},
	{0x380f, 0xd8},
	{0x3813, 0x06},
	{0x3618, 0x00},
	{0x3612, 0x29},
	{0x3709, 0x52},
	{0x370c, 0x03},
	{0x3a02, 0x03},
	{0x3a03, 0xd8},
	{0x3a08 ,0x01},///
	{0x3a09, 0x27},///
	{0x3a0a, 0x00},///
	{0x3a0b, 0xf6},///
	{0x3a0e, 0x03},
	{0x3a0d, 0x04},
	{0x3a14, 0x03},
	{0x3a15, 0xd8},
	{0x4004, 0x02},
	{0x3002, 0x1c},////
	{0x4713, 0x03},////
	{0x3035, 0x21},
	{0x3036, 0x46},
	{0x4837, 0x22},
	{0x3824, 0x02},////
	{0x5001, 0xa3},

	ENDMARKER
};


static const struct regval_list ov5640_vga_regs[] = {
	{0x3800, 0x00}, /* image windowing */
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0xA },
	{0x3805, 0x3f},
	{0x3806, 0x7 },
	{0x3807, 0x9b},
	{0x3808, 0x2 }, /* 0x280== 640*/
	{0x3809, 0x80},
	{0x380a, 0x1 }, /* 0x1e0== 480*/
	{0x380b, 0xe0},
	{0x380c, 0x7 },
	{0x380d, 0x68},
	{0x380e, 0x3 },
	{0x380f, 0xd8},

	//test mode
	//{0x503d, 0x80},
	// {0x4741, 0x00},
	ENDMARKER,
};

/* 2592x1944 QSXGA */
static const struct regval_list ov5640_qsxga_regs[] = {//caichsh
	{0x3503, 0x07},
	{0x3a00, 0x78},
	{0x350c, 0x00},
	{0x350d, 0x00},
	{0x3c07, 0x07},
	{0x3820, 0x40},
	{0x3821, 0x06},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3803, 0x00},
	{0x3807, 0x9f},
	{0x3808, 0x0a},
	{0x3809, 0x20},
	{0x380a, 0x07},
	{0x380b, 0x98},
	{0x380c, 0x0b},
	{0x380d, 0x1c},
	{0x380e, 0x07},
	{0x380f, 0xb0},
	{0x3813, 0x04},
	{0x3618, 0x04},
	{0x3612, 0x2b},
	{0x3709, 0x12},
	{0x370c, 0x00},
	{0x3a02, 0x07},
	{0x3a03, 0xb0},
	{0x3a0e, 0x06},
	{0x3a0d, 0x08},
	{0x3a14, 0x07},
	{0x3a15, 0xb0},
	{0x4004, 0x06},
	{0x3035, 0x21},
	{0x3036, 0x46},
	{0x4837, 0x2c},
	{0x5001, 0x83},

	ENDMARKER,
};

/* 2048x1536 QXGA */
static const struct regval_list ov5640_qxga_regs[] = {//caichsh
	{0x3034,0x18},                               
#ifndef FPGA_VER
	{0x3035,0x21},                          
#else
	{0x3035,0x41},                         
#endif                               
	{0x3036,0x54},                               
	{0x3037,0x13},                               
	{0x3108,0x01},                               
	{0x3824,0x01},                               
	{REG_DLY,0x05},//delay 5ms              
	//timing                                           
	//2048*1536                                        
	{0x3808,0x08}, //H size MSB                 
	{0x3809,0x00}, //H size LSB                 
	{0x380a,0x06}, //V size MSB                 
	{0x380b,0x00}, //V size LSB                 
	{0x380c,0x0b}, //HTS MSB                    
	{0x380d,0x1c}, //HTS LSB                    
	{0x380e,0x07}, //VTS MSB                    
	{0x380f,0xb0}, //LSB                        
#ifndef FPGA_VER
	//banding step                                        
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x93}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x7b}, //60HZ step LSB                 
	{0x3a0e,0x0d}, //50HZ step max                 
	{0x3a0d,0x10}, //60HZ step max                 
#else
	//banding step                                        
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x49}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x3d}, //60HZ step LSB                 
	{0x3a0e,0x1a}, //50HZ step max                 
	{0x3a0d,0x20}, //60HZ step max 
#endif        

	//  {0x3503,0x07}, //AEC disable                                             
	{0x350c,0x00},                              
	{0x350d,0x00},                              
	{0x3c07,0x07}, //light meter 1 thereshold   

	{0x3814,0x11}, //horizton subsample
	{0x3815,0x11}, //vertical subsample
	{0x3800,0x00}, //x address start high byte
	{0x3801,0x00}, //x address start low byte  
	{0x3802,0x00},  //y address start high byte 
	{0x3803,0x00}, //y address start low byte 
	{0x3804,0x0a}, //x address end high byte
	{0x3805,0x3f}, //x address end low byte 
	{0x3806,0x07}, //y address end high byte
	{0x3807,0x9f}, //y address end low byte 
	{0x3810,0x00}, //isp hortizontal offset high byte
	{0x3811,0x10}, //isp hortizontal offset low byte
	{0x3812,0x00}, //isp vertical offset high byte
	{0x3813,0x04},  //isp vertical offset low byte 

	//  {0x5308,0x65},    //sharpen manual                                                   
	//  {0x5302,0x20}, //sharpness                    

	{0x4002,0xc5},  //BLC related               
	{0x4005,0x1a}, // BLC related                              

	{0x3618,0x04},                              
	{0x3612,0x2b},                              
	{0x3709,0x12},                              
	{0x370c,0x00}, 
	{0x3a02,0x07}, //60HZ max exposure limit MSB
	{0x3a03,0xb0}, //60HZ max exposure limit LSB
	{0x3a14,0x07}, //50HZ max exposure limit MSB
	{0x3a15,0xb0}, //50HZ max exposure limit LSB
	{0x4004,0x06}, //BLC line number                                         
	{0x4837,0x2c},//PCLK period                              
	{0x5001,0xa3},//ISP effect  
	{0x302c,0x42},//bit[7:6]: output drive capability
	ENDMARKER,
};

/* 1600x1200 UXGA */
static const struct regval_list ov5640_uxga_regs[] = {//caichsh
	{0x3034,0x18},                                                                                                             
#ifndef FPGA_VER
	{0x3035,0x21},                          
#else
	{0x3035,0x41},                         
#endif                                                                                                           
	{0x3036,0x54},                                                                                                             
	{0x3037,0x13},                                                                                                             
	{0x3108,0x01},                                                                                                             
	{0x3824,0x01},                                                                                                             
	{REG_DLY,0x05},//delay 5ms                                        
	//timing                                                                                                                             
	//1600*1200                                                                                                                          
	{0x3808,0x06}, //H size MSB                                                                                             
	{0x3809,0x40}, //H size LSB                                                                                             
	{0x380a,0x04}, //V size MSB                                                                                             
	{0x380b,0xb0}, //V size LSB                                                                                             
	{0x380c,0x0b}, //HTS MSB                                                                                                
	{0x380d,0x1c}, //HTS LSB                                                                                                
	{0x380e,0x07}, //VTS MSB                                                                                                
	{0x380f,0xb0}, //LSB                                                                                                    
#ifndef FPGA_VER
	//banding step                                        
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x93}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x7b}, //60HZ step LSB                 
	{0x3a0e,0x0d}, //50HZ step max                 
	{0x3a0d,0x10}, //60HZ step max                 
#else
	//banding step                                        
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x49}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x3d}, //60HZ step LSB                 
	{0x3a0e,0x1a}, //50HZ step max                 
	{0x3a0d,0x20}, //60HZ step max 
#endif                                                                                          

	//  {0x3503,0x07}, //AEC disable                                                                                                                                                                                                                  
	{0x350c,0x00},                                                                                                                 
	{0x350d,0x00},                                                                                                                 
	{0x3c07,0x07}, //light meter 1 thereshold                                                                                      

	{0x3814,0x11}, //horizton subsample
	{0x3815,0x11}, //vertical subsample
	{0x3800,0x00}, //x address start high byte
	{0x3801,0x00}, //x address start low byte  
	{0x3802,0x00},  //y address start high byte 
	{0x3803,0x00}, //y address start low byte 
	{0x3804,0x0a}, //x address end high byte
	{0x3805,0x3f}, //x address end low byte 
	{0x3806,0x07}, //y address end high byte
	{0x3807,0x9f}, //y address end low byte 
	{0x3810,0x00}, //isp hortizontal offset high byte
	{0x3811,0x10}, //isp hortizontal offset low byte
	{0x3812,0x00}, //isp vertical offset high byte
	{0x3813,0x04},  //isp vertical offset low byte 

	{0x4002,0xc5}, //BLC related                                                                                                     
	{0x4005,0x12}, //BLC related                                                                                        
	//  {0x5308,0x65},    //sharpen manual
	//  {0x5302,0x20},//sharpness                                                                                          

	{0x3618,0x04},                                                                                                               
	{0x3612,0x2b},                                                                                                               
	{0x3709,0x12},                                                                                                               
	{0x370c,0x00},                                                                                                               
	{0x3a02,0x07},//60HZ max exposure limit MSB                                                                                                                  
	{0x3a03,0xb0},//60HZ max exposure limit LSB                                                                                   
	{0x3a14,0x07},//50HZ max exposure limit MSB                                                                                   
	{0x3a15,0xb0},//50HZ max exposure limit LSB                                                                                   
	{0x4004,0x06},//BLC line number                                                                                               


	{0x4837,0x2c}, //PCLK period                                                                                                  
	{0x5001,0xa3}, //ISP effect  
	{0x302c,0x42},//bit[7:6]: output drive capability
	ENDMARKER,
};

/* 1280x1024 SXGA */
static const struct regval_list ov5640_sxga_regs[] = {//caichsh
	//capture 1.3Mega 7.5fps
	//power down
	//  {0x3008,0x42},
	{0x3820,0x40},
	{0x3821,0x06},
	//pll and clock setting                                                                                   
	{0x3034,0x18},                                                      
#ifndef FPGA_VER
	{0x3035,0x11},                          
#else
	{0x3035,0x21},                         
#endif                                                        
	{0x3036,0x54},                                                      
	{0x3037,0x13},                                                      
	{0x3108,0x01},                                                      
	{0x3824,0x01},                                                      
	{REG_DLY,0x05},//delay 5ms                                  
	//timing                                                                                                  
	//1280*960                                                                                                
	{0x3808,0x05}, //H size MSB                                                                  
	{0x3809,0x00}, //H size LSB                                                                  
	{0x380a,0x03}, //V size MSB                                                                  
	{0x380b,0xc0}, //V size LSB                                                                  
	{0x380c,0x0b}, //HTS MSB                                                                     
	{0x380d,0x1c}, //HTS LSB                                                                     
	{0x380e,0x07}, //VTS MSB                                                                     
	{0x380f,0xb0}, //LSB                                                                         
#ifndef FPGA_VER
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x94}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x7b}, //60HZ step LSB                 
	{0x3a0e,0x06}, //50HZ step max                 
	{0x3a0d,0x08}, //60HZ step max                 
#else
	//banding step      0x41                                  
	//{0x3a08,0x00}, //50HZ step MSB                 
	//{0x3a09,0x49}, //50HZ step LSB                 
	//{0x3a0a,0x00}, //60HZ step MSB                 
	//{0x3a0b,0x3d}, //60HZ step LSB                 
	//{0x3a0e,0x1a}, //50HZ step max                 
	//{0x3a0d,0x20}, //60HZ step max 

	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x94}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x7b}, //60HZ step LSB                 
	{0x3a0e,0x06}, //50HZ step max                 
	{0x3a0d,0x08}, //60HZ step max                 

#endif                                                                      

	{0x3503,0x00},  //AEC enable                                                                                                                                                          
	{0x350c,0x00},                                                                                     
	{0x350d,0x00},                                                                                     
	{0x3c07,0x07}, //light meter 1 thereshold                                                          

	{0x3814,0x11}, //horizton subsample
	{0x3815,0x11}, //vertical subsample
	{0x3800,0x00}, //x address start high byte
	{0x3801,0x00}, //x address start low byte  
	{0x3802,0x00},  //y address start high byte 
	{0x3803,0x00}, //y address start low byte 
	{0x3804,0x0a}, //x address end high byte
	{0x3805,0x3f}, //x address end low byte 
	{0x3806,0x07}, //y address end high byte
	{0x3807,0x9f}, //y address end low byte 
	{0x3810,0x00}, //isp hortizontal offset high byte
	{0x3811,0x10}, //isp hortizontal offset low byte
	{0x3812,0x00}, //isp vertical offset high byte
	{0x3813,0x04},  //isp vertical offset low byte                                                                                                          

	{0x4002,0xc5}, //BLC related                                                           
	{0x4005,0x12}, //BLC related                                                             
	//  {0x5308,0x65},    //sharpen manual
	//  {0x5302,0x20},//sharpness                                                                            

	{0x3618,0x04},                                                                                    
	{0x3612,0x2b},                                                                                    
	{0x3709,0x12},                                                                                    
	{0x370c,0x00},                                                       
	{0x3a02,0x07}, //60HZ max exposure limit MSB                                                      
	{0x3a03,0xb0}, //60HZ max exposure limit LSB                                                      
	{0x3a14,0x07}, //50HZ max exposure limit MSB                                                      
	{0x3a15,0xb0}, //50HZ max exposure limit LSB                                                      
	{0x4004,0x06}, //BLC line number                                                                                  

	{0x4837,0x2c}, //PCLK period
	{0x5001,0xa3}, //ISP effect   
	{0x302c,0x42},//bit[7:6]: output drive capability
	//00: 1x   01: 2x  10: 3x  11: 4x   
	{0x3a18,0x00},//
	{0x3a19,0xf8},//  
	//power down release
	//  {0x3008,0x02},     
	//  {REG_DLY,0x32},//delay 50ms
	//{REG_TERM,VAL_TERM},

	ENDMARKER,
};

/* 720p 15fps @ 1280x720 */
static const struct regval_list ov5640_720p_regs[] = {
	{0x3503, 0x00},
		
	{0x3c07,0x07},
	{0x3803,0xfa},
	{0x3806,0x06},////
	{0x3807,0xa9},
	{0x3808,0x05},
	{0x3809,0x00},
	{0x380a,0x02},
	{0x380b,0xd0},
	{0x380c,0x07},
	{0x380d,0x64},
	{0x380e,0x02},
	{0x380f,0xe4},
	{0x3813,0x04},
	{0x3a02,0x02},
	{0x3a03,0xe4},
	{0x3a08,0x01},///
	{0x3a09,0xbc},////
	{0x3a0a,0x01},///
	{0x3a0b,0x72},////
	{0x3a0e,0x01},
	{0x3a0d,0x02},
	{0x3a14,0x02},
	{0x3a15,0xe4},
	{0x3002,0x00},///
	{0x4713,0x02},///
	{0x4837,0x16},
	{0x3824,0x04},///
	{0x5001,0x83},
	{0x3035,0x21},
	{0x3036,0x46},

	{0x4837, 0x22},
	{0x5001, 0xa3},
	
	ENDMARKER,
};

/* 1024x768 XGA */
static const struct regval_list ov5640_xga_regs[] = {//caichsh
	//capture 1Mega 7.5fps
	//power down
	//  {0x3008,0x42},
	//pll and clock setting
	{0x3034,0x18},
#ifndef FPGA_VER
	{0x3035,0x21},                          
#else
	{0x3035,0x41},                         
#endif
	{0x3036,0x54},
	{0x3037,0x13},
	{0x3108,0x01},
	{0x3824,0x01},
	{REG_DLY,0x05},//delay 5ms
	//timing
	//1024*768
	{0x3808,0x04}, //H size MSB
	{0x3809,0x00}, //H size LSB
	{0x380a,0x03}, //V size MSB
	{0x380b,0x00}, //V size LSB
	{0x380c,0x0b}, //HTS MSB    
	{0x380d,0x1c}, //HTS LSB     
	{0x380e,0x07}, //VTS MSB    
	{0x380f,0xb0}, //LSB
#ifndef FPGA_VER
	//banding step                                        
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x93}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x7b}, //60HZ step LSB                 
	{0x3a0e,0x0d}, //50HZ step max                 
	{0x3a0d,0x10}, //60HZ step max                 
#else
	//banding step                                        
	{0x3a08,0x00}, //50HZ step MSB                 
	{0x3a09,0x49}, //50HZ step LSB                 
	{0x3a0a,0x00}, //60HZ step MSB                 
	{0x3a0b,0x3d}, //60HZ step LSB                 
	{0x3a0e,0x1a}, //50HZ step max                 
	{0x3a0d,0x20}, //60HZ step max 
#endif
	//  {0x3503,0x07}, //AEC disable                                                                                              
	{0x350c,0x00},                              
	{0x350d,0x00},                              
	{0x3c07,0x07}, //light meter 1 thereshold   


	{0x3814,0x11}, //horizton subsample
	{0x3815,0x11}, //vertical subsample
	{0x3800,0x00}, //x address start high byte
	{0x3801,0x00}, //x address start low byte  
	{0x3802,0x00},  //y address start high byte 
	{0x3803,0x00}, //y address start low byte 
	{0x3804,0x0a}, //x address end high byte
	{0x3805,0x3f}, //x address end low byte 
	{0x3806,0x07}, //y address end high byte
	{0x3807,0x9f}, //y address end low byte 
	{0x3810,0x00}, //isp hortizontal offset high byte
	{0x3811,0x10}, //isp hortizontal offset low byte
	{0x3812,0x00}, //isp vertical offset high byte
	{0x3813,0x04},  //isp vertical offset low byte

	//  {0x5308,0x65},    //sharpen manual
	//  {0x5302,0x20},    //sharpen offset 1
	{0x4002,0xc5},    //BLC related
	{0x4005,0x12},    //BLC related

	{0x3618,0x00},      
	{0x3612,0x29},      
	{0x3709,0x52},      
	{0x370c,0x03},      
	{0x3a02,0x03},  //60HZ max exposure limit MSB 
	{0x3a03,0xd8},  //60HZ max exposure limit LSB     
	{0x3a14,0x03},  //50HZ max exposure limit MSB     
	{0x3a15,0xd8},  //50HZ max exposure limit LSB     
	{0x4004,0x02},  //BLC line number    

	{0x4837,0x22},  //PCLK period    
	{0x5001,0xa3},  //ISP effect

	{0x3618,0x04},                               
	{0x3612,0x2b},                               
	{0x3709,0x12},                               
	{0x370c,0x00},                               
	{0x3a02,0x07}, //60HZ max exposure limit MSB
	{0x3a03,0xb0}, //60HZ max exposure limit LSB
	{0x3a14,0x07}, //50HZ max exposure limit MSB
	{0x3a15,0xb0}, //50HZ max exposure limit LSB
	{0x4004,0x06}, //BLC line number            
	{0x4837,0x2c}, //PCLK period 
	{0x5001,0xa3}, //ISP effect

	{0x302c,0x42},//bit[7:6]: output drive capability
	//00: 1x   01: 2x  10: 3x  11: 4x 
	ENDMARKER,
};

static const struct regval_list ov5640_disable_regs[] = {
	ENDMARKER,
};

/****************************************************************************************
 * structures
 */

struct ov5640_win_size {
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

struct ov5640_priv {
	struct v4l2_subdev                  subdev;
	struct media_pad                    pad;
	struct v4l2_ctrl_handler            hdl;
	const struct ov5640_color_format    *cfmt;
	const struct ov5640_win_size        *win;
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

struct ov5640_color_format {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

/****************************************************************************************
 * tables
 */
static const struct ov5640_color_format ov5640_cfmts[] = {
	{
		.code		= V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	}, {
		.code		= V4L2_MBUS_FMT_UYVY8_2X8,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	}, {
		.code		= V4L2_MBUS_FMT_YVYU8_2X8,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	}, {
		.code		= V4L2_MBUS_FMT_VYUY8_2X8,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	},
};

/*
 * window size list
 */
#define SVGA_WIDTH           800
#define SVGA_HEIGHT          600

static unsigned int frame_rate_vga[] = {12,};
static unsigned int frame_rate_svga[] = {12,};
static unsigned int frame_rate_uxga[] = {12,};

/* 640X480 */
static const struct ov5640_win_size ov5640_win_vga = {
	.name     = "VGA",
	.width    =  640,
	.height   =  480,
	.win_regs = ov5640_vga_regs,
	.frame_rate_array = frame_rate_vga,
};

/* 800X600 */
static const struct ov5640_win_size ov5640_win_svga = {
	.name     = "SVGA",
	.width    = SVGA_WIDTH,
	.height   = SVGA_HEIGHT,
	.win_regs = ov5640_svga_regs,
	.frame_rate_array = frame_rate_svga,
};

/* 1024X768 */
static const struct ov5640_win_size ov5640_win_xga = {
	.name     = "XGA",
	.width    = 1024,
	.height   =  768,
	.win_regs = ov5640_xga_regs,
	.frame_rate_array = frame_rate_uxga,
};

static const struct ov5640_win_size ov5640_win_720p = {
	.name     = "720P",
	.width    = 1280,
	.height   = 720,
	.win_regs = ov5640_720p_regs,
	.frame_rate_array = frame_rate_uxga,
};

/* 1280X1024 */
static const struct ov5640_win_size ov5640_win_sxga = {
	.name     = "SXGA",
	.width    = 1280,
	.height   = 1024,
	.win_regs = ov5640_sxga_regs,
	.frame_rate_array = frame_rate_uxga,
};

/* 1600X1200 */
static const struct ov5640_win_size ov5640_win_uxga = {
	.name     = "UXGA",
	.width    = 1600,
	.height   = 1200,
	.win_regs = ov5640_uxga_regs,
	.frame_rate_array = frame_rate_uxga,
};

/* 2048X1536 */
static const struct ov5640_win_size ov5640_win_qxga = {
	.name     = "QXGA",
	.width    = 2048,
	.height   = 1536,
	.win_regs = ov5640_qxga_regs,
	.frame_rate_array = frame_rate_uxga,
};

/* 2592x1944 */
static const struct ov5640_win_size ov5640_win_qsxga = {
	.name     = "QSXGA",
	.width    = 2592,
	.height   = 1944,
	.win_regs = ov5640_qsxga_regs,
	.frame_rate_array = frame_rate_uxga,
};

static const struct ov5640_win_size *ov5640_win[] = {
	&ov5640_win_vga,
	&ov5640_win_svga,
	&ov5640_win_xga,
	&ov5640_win_720p,
	&ov5640_win_sxga,
	&ov5640_win_uxga,
	&ov5640_win_qxga,
	&ov5640_win_qsxga,
};

/****************************************************************************************
 * general functions
 */
static inline struct ov5640_priv *to_priv(struct v4l2_subdev *subdev)
{
	return container_of(subdev, struct ov5640_priv, subdev);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct ov5640_priv, hdl)->subdev;
}

static int reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	/* We have 16-bit i2c addresses - care for endianess */
	unsigned char data[2] = { reg >> 8, reg & 0xff };

	ret = i2c_master_send(client, data, 2);
	if (ret < 2) {
		dev_err(&client->dev, "%s: i2c read error, reg: %x\n",
				__func__, reg);
		return ret < 0 ? ret : -EIO;
	}

	ret = i2c_master_recv(client, val, 1);
	if (ret < 1) {
		dev_err(&client->dev, "%s: i2c read error, reg: %x\n",
				__func__, reg);
		return ret < 0 ? ret : -EIO;
	}
	return 0;
}

static int reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	unsigned char data[3] = { reg >> 8, reg & 0xff, val };

	ret = i2c_master_send(client, data, 3);
	if (ret < 3) {
		dev_err(&client->dev, "%s: i2c write error, reg: %x\n",
				__func__, reg);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}

static int reg_write_array(struct i2c_client *client,
		const struct regval_list *vals)
{
	int ret;

	for (; vals->reg != 0; vals++) {
		ret = reg_write(client, vals->reg, vals->val);
		if (ret < 0) {
			return ret;
		}
	}

	dev_dbg(&client->dev, "Register list (%p) loaded\n", vals);
	return 0;
}

static int reg_wait_stat(struct i2c_client *client, u16 reg, u8 mask, u8 stat)
{
	int retry = 45;
	u8 val = 0;
	int ret;

	do {
		ret = reg_read(client, reg, &val);
		if (ret < 0)
			return ret;

		if ((val & mask) == stat)
			break;

		if (retry > 20)
			udelay(10);	/* short delay */
		else
			msleep(50);

	} while (--retry);

	if (!retry)
		return -ETIMEDOUT;

	return 0;
}

static int reg_txdata(struct i2c_client *client, u8 *data, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= data,
		},
	};

	if (i2c_transfer(client->adapter, msg, 1) < 0) {
		dev_err(&client->dev, "%s: i2c transfer data (%d bytes) error\n",
				__func__, length);
		return -EIO;
	}

	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov5640_get_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	u8 val;

	if (reg->reg & ~0xffff)
		return -EINVAL;

	reg->size = 1;

	ret = reg_read(client, reg->reg, &val);
	if (!ret)
		reg->val = (__u64)val;

	return ret;
}

static int ov5640_set_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg & ~0xffff || reg->val & ~0xff)
		return -EINVAL;

	return reg_write(client, reg->reg, reg->val);
}
#endif

static int ov5640_write_array(struct i2c_client *client,
		const struct regval_list *vals)
{
	int ret;
	while (vals->reg != 0xffff || vals->val != 0xff) {
		if (vals->reg == 0xfffe) {
			msleep(5);
			vals++;
			continue;
		}
		ret = reg_write(client, vals->reg, vals->val); 
		if (ret < 0)
			return ret;
		vals++;
	}
	dev_dbg(&client->dev, "Register list loaded\n");
	return 0;
}

static const struct ov5640_win_size *ov5640_select_win(u32 width, u32 height)
{
	const struct ov5640_win_size *win;
	int i;

	for (i = 0; i < ARRAY_SIZE(ov5640_win); i++) {
		win = ov5640_win[i];
		if (width == win->width && height == win->height){
			printk("%s......width:%d height:%d......\n",__func__,width,height);
			return win;
		}
	}

	printk(KERN_ERR "%s: unsupported width, height (%dx%d)\n", __func__, width, height);
	return NULL;
}

static int ov5640_set_mbusformat(struct i2c_client *client, const struct ov5640_color_format *cfmt)
{
	enum v4l2_mbus_pixelcode code;
	int ret = -1;

	code = cfmt->code;
	printk("%s.............%d...........\n",__func__,__LINE__);
	switch (code) {
		case V4L2_MBUS_FMT_YUYV8_2X8:
			ret  = ov5640_write_array(client, ov5640_fmt_yuv422_yuyv);
			break;
		case V4L2_MBUS_FMT_UYVY8_2X8:
			ret  = ov5640_write_array(client, ov5640_fmt_yuv422_uyvy);
			break;
		case V4L2_MBUS_FMT_YVYU8_2X8:
			ret  = ov5640_write_array(client, ov5640_fmt_yuv422_yvyu);
			break;
		case V4L2_MBUS_FMT_VYUY8_2X8:
			ret  = ov5640_write_array(client, ov5640_fmt_yuv422_vyuy);
			break;
		default:
			printk(KERN_ERR "mbus code error in %s() line %d\n",__FUNCTION__, __LINE__);
	}

	return ret;
}

static int ov5640_set_params(struct v4l2_subdev *sd,
		u32 *width, u32 *height, enum v4l2_mbus_pixelcode code)
{
	struct ov5640_priv *priv = to_priv(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	const struct ov5640_win_size *old_win, *new_win;
	int i, ret;

	/* select color format */
	priv->cfmt = NULL;
	for (i = 0; i < ARRAY_SIZE(ov5640_cfmts); i++) {
		printk("%s........parma code:%d support code:%d...........\n",__func__,
				code,ov5640_cfmts[i].code);
		if (code == ov5640_cfmts[i].code) {
			priv->cfmt = ov5640_cfmts + i;
			break;
		}
	}
	if (!priv->cfmt) {
		printk(KERN_ERR "Unsupported sensor format.\n");
		return -EINVAL;
	}

	/* select win size */
	old_win = priv->win;
	new_win = ov5640_select_win(*width, *height);
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

	ret = ov5640_write_array(client, priv->win->win_regs);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to ov5640_write_array win regs\n", __func__);
		return -EIO;
	}
	printk(KERN_ERR "%s: ov5640_write_array win regs\n", __func__);

	ret = ov5640_set_mbusformat(client, priv->cfmt);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to ov5640_set_mbusformat()\n", __func__);
		return -EIO;
	}

	return 0;
}

/* --------------------------------------------------------------------------
 * Chip support
 */

/* OV5640 register set for AF command */
static struct regval_list ov5640_af_cmd_reg[] = {
	{ 0x3022, 0x00 },
	{ 0x3023, 0x00 },
	{ 0x3024, 0x00 },
	{ 0x3025, 0x00 },
	{ 0x3026, 0x00 },
	{ 0x3027, 0x00 },
	{ 0x3028, 0x00 },
	{ 0x3029, 0xFF },
	{ 0x3000, 0x00 },

	{ 0, 0 },
};

static int ov5640_af_setting(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = reg_write(client, 0x3000, 0x20);
	if (ret < 0)
		return ret;

	ret = reg_txdata(client, ov5640_af_firmware, ARRAY_SIZE(ov5640_af_firmware));
	if (ret < 0) {
		v4l_err(client, "Failed to load AF firmware\n");
		return ret;
	}

	ret = reg_write_array(client, ov5640_af_cmd_reg);
	if (ret) {
		v4l_err(client, "Failed to load AF command\n");

	} else {
		ret = reg_wait_stat(client, 0x3029, 0xff, 0x70);
		if (ret)
			v4l_err(client, "Failed to check AF status\n");

		ret = reg_write(client, 0x3022, 0x08);
		if (ret)
			v4l_err(client, "Failed to set initial focus mode\n");
	}

	return ret;
}

static int ov5640_af_continuous(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = reg_write(client, 0x3022, 0x04);
	if (ret)
		v4l_err(client, "Failed to set continuous focus mode\n");

	return ret;
}


/****************************************************************************************
 * control functions
 */
static int ov5640_set_brightness(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	printk("%s.............%d...........\n",__func__,__LINE__);

	return 0;
}

static int ov5640_set_contrast(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	/* TODO */
	int contrast = ctrl->val;
	printk("%s: val %d\n", __func__, contrast);

	return 0;
}

static int ov5640_set_auto_white_balance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	/* struct ov5640_priv *priv = to_priv(sd); */
	int auto_white_balance = ctrl->val;
	int ret;

	printk("%s: val %d\n", __func__, auto_white_balance);
	if (auto_white_balance < 0 || auto_white_balance > 1) {
		dev_err(&client->dev, "set auto_white_balance over range, auto_white_balance = %d\n", auto_white_balance);
		return -ERANGE;
	}

	switch(auto_white_balance) {
		case 0:
			OV5640_DEBUG(KERN_ERR "===awb disable===\n");
			ret = ov5640_write_array(client, ov5640_awb_regs_diable);
			break;
		case 1:
			OV5640_DEBUG(KERN_ERR "===awb enable===\n");
			ret = ov5640_write_array(client, ov5640_awb_regs_enable);
			break;
	}

	assert(ret == 0);

	return 0;
}

/* TODO : exposure */
static int ov5640_set_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	printk("%s: val %d\n", __func__, ctrl->val);
	return 0;
}

/* TODO */
static int ov5640_set_gain(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	printk("%s: val %d\n", __func__, ctrl->val);
	return 0;
}

/* TODO */
static int ov5640_set_hflip(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	printk("%s: val %d\n", __func__, ctrl->val);
	return 0;
}

static int ov5640_set_white_balance_temperature(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	/* struct ov5640_priv *priv = to_priv(sd); */
	int white_balance_temperature = ctrl->val;
	int ret;

	printk("%s: val %d\n", __func__, ctrl->val);

	switch(white_balance_temperature) {
		case V4L2_WHITE_BALANCE_INCANDESCENT:
			ret = ov5640_write_array(client, ov5640_wb_incandescence_regs);
			break;
		case V4L2_WHITE_BALANCE_FLUORESCENT:
			ret = ov5640_write_array(client, ov5640_wb_fluorescent_regs);
			break;
		case V4L2_WHITE_BALANCE_DAYLIGHT:
			ret = ov5640_write_array(client, ov5640_wb_daylight_regs);
			break;
		case V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT:
			ret = ov5640_write_array(client, ov5640_wb_cloud_regs);
			break;
		case V4L2_WHITE_BALANCE_TUNGSTEN:
			ret = ov5640_write_array(client, ov5640_wb_tungsten_regs);
			break;
		default:
			dev_err(&client->dev, "set white_balance_temperature over range, white_balance_temperature = %d\n", white_balance_temperature);
			return -ERANGE;
	}

	assert(ret == 0);

	return 0;
}

static int ov5640_set_colorfx(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int colorfx = ctrl->val;
	int ret;

	printk("%s: val %d\n", __func__, ctrl->val);

	switch (colorfx) {
		case V4L2_COLORFX_NONE: /* normal */
			ret = ov5640_write_array(client, ov5640_colorfx_none_regs);
			break;
		case V4L2_COLORFX_BW: /* black and white */
			ret = ov5640_write_array(client, ov5640_colorfx_bw_regs);
			break;
		case V4L2_COLORFX_SEPIA: /* antique ,¸´¹Å*/
			ret = ov5640_write_array(client, ov5640_colorfx_sepia_regs);
			break;
		case V4L2_COLORFX_NEGATIVE: /* negative£¬¸ºÆ¬ */
			ret = ov5640_write_array(client, ov5640_colorfx_negative_regs);
			break;
		default:
			dev_err(&client->dev, "set colorfx over range, colorfx = %d\n", colorfx);
			return -ERANGE;
	}

	assert(ret == 0);
	return 0;
}

static int ov5640_set_exposure_auto(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	/* struct ov5640_priv *priv = to_priv(sd); */
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
static int ov5640_set_scene_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	printk("%s: val %d\n", __func__, ctrl->val);
	return 0;
}

static int ov5640_set_prev_capt_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640_priv *priv = to_priv(sd);
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

static int ov5640_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;
	printk("%s.............%d...........\n",__func__,__LINE__);

	switch (ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			ov5640_set_brightness(sd, ctrl);
			break;

		case V4L2_CID_CONTRAST:
			ov5640_set_contrast(sd, ctrl);
			break;

		case V4L2_CID_AUTO_WHITE_BALANCE:
			ov5640_set_auto_white_balance(sd, ctrl);
			break;

		case V4L2_CID_EXPOSURE:
			ov5640_set_exposure(sd, ctrl);
			break;

		case V4L2_CID_GAIN:
			ov5640_set_gain(sd, ctrl);
			break;

		case V4L2_CID_HFLIP:
			ov5640_set_hflip(sd, ctrl);
			break;

		case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
			ov5640_set_white_balance_temperature(sd, ctrl);
			break;

		case V4L2_CID_COLORFX:
			ov5640_set_colorfx(sd, ctrl);
			break;

		case V4L2_CID_EXPOSURE_AUTO:
			ov5640_set_exposure_auto(sd, ctrl);
			break;

		case V4L2_CID_SCENE_EXPOSURE:
			ov5640_set_scene_exposure(sd, ctrl);
			break;

		case V4L2_CID_PRIVATE_PREV_CAPT:
			ov5640_set_prev_capt_mode(sd, ctrl);
			break;

		default:
			dev_err(&client->dev, "%s: invalid control id %d\n", __func__, ctrl->id);
			return -EINVAL;
	}

	return ret;
}

static const struct v4l2_ctrl_ops ov5640_ctrl_ops = {
	.s_ctrl = ov5640_s_ctrl,
};

static const struct v4l2_ctrl_config ov5640_custom_ctrls[] = {
	{
		.ops    = &ov5640_ctrl_ops,
		.id     = V4L2_CID_SCENE_EXPOSURE,
		.type   = V4L2_CTRL_TYPE_INTEGER,
		.name   = "SceneExposure",
		.min    = 0,
		.max    = 1,
		.def    = 0,
		.step   = 1,
	}, {
		.ops    = &ov5640_ctrl_ops,
		.id     = V4L2_CID_PRIVATE_PREV_CAPT,
		.type   = V4L2_CTRL_TYPE_INTEGER,
		.name   = "PrevCapt",
		.min    = 0,
		.max    = 1,
		.def    = 0,
		.step   = 1,
	}
};

static int ov5640_initialize_ctrls(struct ov5640_priv *priv)
{
	printk("%s.............%d...........\n",__func__,__LINE__);

	v4l2_ctrl_handler_init(&priv->hdl, NUM_CTRLS);

	/* standard ctrls */
	priv->brightness = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_BRIGHTNESS, 0, 6, 1, 0);
	if (!priv->brightness) {
		printk(KERN_ERR "%s: failed to create brightness ctrl\n", __func__);
		return -ENOENT;
	}

	priv->contrast = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_CONTRAST, -6, 6, 1, 0);
	if (!priv->contrast) {
		printk(KERN_ERR "%s: failed to create contrast ctrl\n", __func__);
		return -ENOENT;
	}

	priv->auto_white_balance = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
	if (!priv->auto_white_balance) {
		printk(KERN_ERR "%s: failed to create auto_white_balance ctrl\n", __func__);
		return -ENOENT;
	}

#if 0
	priv->exposure = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_EXPOSURE, 0, 0xFFFF, 1, 500);
	if (!priv->exposure) {
		printk(KERN_ERR "%s: failed to create exposure ctrl\n", __func__);
		return -ENOENT;
	}
#endif

	priv->gain = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_GAIN, 0, 0xFF, 1, 128);
	if (!priv->gain) {
		printk(KERN_ERR "%s: failed to create gain ctrl\n", __func__);
		return -ENOENT;
	}

	priv->hflip = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_HFLIP, 0, 1, 1, 0);
	if (!priv->hflip) {
		printk(KERN_ERR "%s: failed to create hflip ctrl\n", __func__);
		return -ENOENT;
	}

	priv->white_balance_temperature = v4l2_ctrl_new_std(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 3, 1, 1);
	if (!priv->white_balance_temperature) {
		printk(KERN_ERR "%s: failed to create white_balance_temperature ctrl\n", __func__);
		return -ENOENT;
	}

	/* standard menus */
	priv->colorfx = v4l2_ctrl_new_std_menu(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_COLORFX, 3, 0, 0);
	if (!priv->colorfx) {
		printk(KERN_ERR "%s: failed to create colorfx ctrl\n", __func__);
		return -ENOENT;
	}

	priv->exposure_auto = v4l2_ctrl_new_std_menu(&priv->hdl, &ov5640_ctrl_ops,
			V4L2_CID_EXPOSURE_AUTO, 1, 0, 1);
	if (!priv->exposure_auto) {
		printk(KERN_ERR "%s: failed to create exposure_auto ctrl\n", __func__);
		return -ENOENT;
	}

	/* custom ctrls */
	priv->scene_exposure = v4l2_ctrl_new_custom(&priv->hdl, &ov5640_custom_ctrls[0], NULL);
	if (!priv->scene_exposure) {
		printk(KERN_ERR "%s: failed to create scene_exposure ctrl\n", __func__);
		return -ENOENT;
	}

	priv->prev_capt = v4l2_ctrl_new_custom(&priv->hdl, &ov5640_custom_ctrls[1], NULL);
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

/****************************************************************************************
 * v4l2 subdev ops
 */

/**
 * core ops
 */

static int ov5640_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u32 width = SVGA_WIDTH;
	u32 height = SVGA_HEIGHT;
	int ret;

	ret = ov5640_write_array(client, ov5640_init_regs);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to ov5640_write_array init regs\n", __func__);
		return -EIO;
	}

	ret = ov5640_set_params(sd, &width, &height, V4L2_MBUS_FMT_YUYV8_2X8);
	if (ret < 0) {
		dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
		return ret;
	}

	ret = ov5640_af_setting(sd);
	if (ret)
		goto exit;

	ret = ov5640_af_continuous(sd);

exit:
	return ret;
}

static int ov5640_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *id)
{
	struct ov5640_priv *priv = to_priv(sd);
	id->ident    = priv->model;
	id->revision = 0;
	printk("%s.............%d...........\n",__func__,__LINE__);
	return 0;
}

static int ov5640_s_power(struct v4l2_subdev *sd, int on)
{
	struct ov5640_priv *priv = to_priv(sd);

	if (on && !priv->initialized) {
		priv->initialized = true;

		return ov5640_init(sd, on);
	}

	printk("%s.............%d...........\n",__func__,__LINE__);
	return 0;
}

static const struct v4l2_subdev_core_ops ov5640_subdev_core_ops = {
	.g_chip_ident	= ov5640_g_chip_ident,
	.s_power        = ov5640_s_power,
	.s_ctrl         = v4l2_subdev_s_ctrl,
};

/**
 * video ops
 */
static int ov5640_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640_priv *priv = to_priv(sd);
	int ret = 0;

	printk("%s: enable %d, initialized %d\n", __func__, enable, priv->initialized);
	// ov5640_video_probe(client);
	if (enable) {
		if (!priv->win || !priv->cfmt) {
			dev_err(&client->dev, "norm or win select error\n");
			return -EPERM;
		}
#if 0
		/* write init regs */
		if (1/*!priv->initialized*/) {
			// if (!check_id(client))
			//    return -EINVAL;

			ret = ov5640_write_array(client, ov5640_init_regs);
			if (ret < 0) {
				printk(KERN_ERR "%s: failed to ov5640_write_array init regs\n", __func__);
				return -EIO;
			}

			priv->initialized = true;
			printk(KERN_ERR "%s ov5640_write_array init regs\n", __func__);
		}

		ret = ov5640_write_array(client, priv->win->win_regs);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to ov5640_write_array win regs\n", __func__);
			return -EIO;
		}
		printk(KERN_ERR "%s: ov5640_write_array win regs\n", __func__);

		ret = ov5640_set_mbusformat(client, priv->cfmt);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to ov5640_set_mbusformat()\n", __func__);
			return -EIO;
		}
#endif
	} else {
		ov5640_write_array(client, ov5640_disable_regs);
	}

	return ret;
}

static int ov5640_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk("%s.............%d...........\n",__func__,__LINE__);

	if (fsize->index >= ARRAY_SIZE(ov5640_win)) {
		dev_err(&client->dev, "index(%d) is over range %d\n", fsize->index, ARRAY_SIZE(ov5640_win));
		return -EINVAL;
	}

	switch (fsize->pixel_format) {
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_YUV422P:
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_YUYV:
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = ov5640_win[fsize->index]->width;
			fsize->discrete.height = ov5640_win[fsize->index]->height;
			break;
		default:
			dev_err(&client->dev, "pixel_format(%d) is Unsupported\n", fsize->pixel_format);
			return -EINVAL;
	}

	dev_info(&client->dev, "type %d, width %d, height %d\n", V4L2_FRMSIZE_TYPE_DISCRETE, fsize->discrete.width, fsize->discrete.height);
	return 0;
}

static int ov5640_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov5640_cfmts))
		return -EINVAL;
	printk("%s.............%d...........\n",__func__,__LINE__);

	*code = ov5640_cfmts[index].code;
	return 0;
}

static int ov5640_g_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640_priv *priv = to_priv(sd);
	if (!priv->win || !priv->cfmt) {
		u32 width = SVGA_WIDTH;
		u32 height = SVGA_HEIGHT;
		int ret = ov5640_set_params(sd, &width, &height, V4L2_MBUS_FMT_UYVY8_2X8);
		if (ret < 0) {
			dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
			return ret;
		}
	}
	printk("%s.............%d...........\n",__func__,__LINE__);

	mf->width   = priv->win->width;
	mf->height  = priv->win->height;
	mf->code    = priv->cfmt->code;
	mf->colorspace  = priv->cfmt->colorspace;
	mf->field   = V4L2_FIELD_NONE;
	dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
	return 0;
}

static int ov5640_try_mbus_fmt(struct v4l2_subdev *sd,
		struct v4l2_mbus_framefmt *mf)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct ov5640_priv *priv = to_priv(sd);
	const struct ov5640_win_size *win;
	int i;
	printk("%s.............%d...........\n",__func__,__LINE__);

	/*
	 * select suitable win
	 */
	win = ov5640_select_win(mf->width, mf->height);
	if (!win)
		return -EINVAL;

	mf->width   = win->width;
	mf->height  = win->height;
	mf->field   = V4L2_FIELD_NONE;

	for (i = 0; i < ARRAY_SIZE(ov5640_cfmts); i++)
		if (mf->code == ov5640_cfmts[i].code)
			break;

	if (i == ARRAY_SIZE(ov5640_cfmts)) {
		/* Unsupported format requested. Propose either */
		if (priv->cfmt) {
			/* the current one or */
			mf->colorspace = priv->cfmt->colorspace;
			mf->code = priv->cfmt->code;
		} else {
			/* the default one */
			mf->colorspace = ov5640_cfmts[0].colorspace;
			mf->code = ov5640_cfmts[0].code;
		}
	} else {
		/* Also return the colorspace */
		mf->colorspace	= ov5640_cfmts[i].colorspace;
	}

	return 0;
}

static int ov5640_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct ov5640_priv *priv = to_priv(sd);

	int ret = ov5640_set_params(sd, &mf->width, &mf->height, mf->code);
	if (!ret)
		mf->colorspace = priv->cfmt->colorspace;

	printk("%s.............%d...........\n",__func__,__LINE__);
	return ret;
}

static const struct v4l2_subdev_video_ops ov5640_subdev_video_ops = {
	.s_stream               = ov5640_s_stream,
	.enum_framesizes        = ov5640_enum_framesizes,
	.enum_mbus_fmt          = ov5640_enum_mbus_fmt,
	.g_mbus_fmt             = ov5640_g_mbus_fmt,
	.try_mbus_fmt           = ov5640_try_mbus_fmt,
	.s_mbus_fmt             = ov5640_s_mbus_fmt,
};

/**
 * pad ops
 */
static int ov5640_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_format *fmt)
{
	struct v4l2_mbus_framefmt *mf = &fmt->format;
	printk("%s: %dx%d\n", __func__, mf->width, mf->height);
	return ov5640_s_mbus_fmt(sd, mf);
}

static const struct v4l2_subdev_pad_ops ov5640_subdev_pad_ops = {
	.set_fmt    = ov5640_s_fmt,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops ov5640_subdev_ops = {
	.core   = &ov5640_subdev_core_ops,
	.video  = &ov5640_subdev_video_ops,
	.pad    = &ov5640_subdev_pad_ops,
};

/* --------------------------------------------------------------------------
 * media_entity_operations
 */

static int ov5640_link_setup(struct media_entity *entity,
		const struct media_pad *local,
		const struct media_pad *remote, u32 flags)
{
	printk("%s\n", __func__);
	return 0;
}

static const struct media_entity_operations ov5640_media_ops = {
	.link_setup	= ov5640_link_setup,
};

/****************************************************************************************
 * initialize
 */
static void ov5640_priv_init(struct ov5640_priv * priv)
{
	priv->model = V4L2_IDENT_OV5640;
	priv->prev_capt_mode = PREVIEW_MODE;
	priv->timeperframe.denominator = 12;//30;
	priv->timeperframe.numerator = 1;
	priv->win = &ov5640_win_svga;
}

static int ov5640_video_probe(struct i2c_client *client)
{
	int ret;
	u8 mode;//itu656
	u8 id_high, id_low;
	u16 id;

	printk("...................................%s %d\n",__func__,__LINE__);

	/* Read sensor Model ID */
	ret = reg_read(client, 0x300a, &id_high);
	if (ret < 0) {
		printk("OV5640 read error!!!!!!!!!!!!!!!\n");
		return ret;
	}

	id = id_high << 8;

	ret = reg_read(client, 0x300b, &id_low);
	if (ret < 0) {
		printk("OV5640 read error!!!!!!!!!!!!!!!\n");
		return ret;
	}

	id |= id_low;

	printk("OV5640 Chip ID 0x%04x detected\n", id);

	if (id != 0x5640)
		return -ENODEV;

	/* adjust to ITU656 */
	reg_read(client, 0x4730, &mode);
	reg_write(client, 0x4730,mode|0x01);

	return 0;
}

static int ov5640_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ov5640_priv *priv;
	struct v4l2_subdev *sd;
	int ret;

	priv = kzalloc(sizeof(struct ov5640_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	printk("...................................%s %d\n",__func__,__LINE__);
	ov5640_priv_init(priv);

	sd = &priv->subdev;
	strcpy(sd->name, MODULE_NAME);

	ov5640_video_probe(client);

	/* register subdev */
	v4l2_i2c_subdev_init(sd, client, &ov5640_subdev_ops);

	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
	sd->entity.ops  = &ov5640_media_ops;
	if (media_entity_init(&sd->entity, 1, &priv->pad, 0)) {
		dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
		kfree(priv);
		return -ENOENT;
	}

	ret = ov5640_initialize_ctrls(priv);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
		kfree(priv);
		return ret;
	}

	return 0;
}

static int ov5640_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	v4l2_device_unregister_subdev(sd);
	v4l2_ctrl_handler_free(sd->ctrl_handler);
	media_entity_cleanup(&sd->entity);
	kfree(to_priv(sd));
	return 0;
}

static const struct i2c_device_id ov5640_id[] = {
	{ MODULE_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, ov5640_id);

static struct i2c_driver ov5640_i2c_driver = {
	.driver = {
		.name = MODULE_NAME,
	},
	.probe    = ov5640_probe,
	.remove   = ov5640_remove,
	.id_table = ov5640_id,
};

module_i2c_driver(ov5640_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for ov5640");
MODULE_AUTHOR("caichsh(caichsh@artekmicro.com)");
MODULE_LICENSE("GPL v2");
