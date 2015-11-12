
/*
 * ov5645 Camera Driver
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

#define MODULE_NAME "OV5645"

// TODO : move this to PLAT/device.c
#if 0
static struct i2c_board_info asoc_i2c_camera = {
    I2C_BOARD_INFO(MODULE_NAME, 0x30),//caichsh
};
#endif

#ifdef OV5645_DEBUG
#define assert(expr) \
    if (unlikely(!(expr))) {				\
        pr_err("Assertion failed! %s,%s,%s,line=%d\n",	\
#expr, __FILE__, __func__, __LINE__);	\
    }

#define OV5645_DEBUG(fmt,args...) printk(KERN_ALERT fmt, ##args)
#else

#define assert(expr) do {} while (0)

#define OV5645_DEBUG(fmt,args...)
#endif

#define PID                 0x02 /* Product ID Number  *///caichsh
#define OV5645              0x53
#define OUTTO_SENSO_CLOCK   24000000
#define NUM_CTRLS           11
#define V4L2_IDENT_OV5645   64188

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
	u16 reg_num;
	u8 value;
};

static int ov5645_video_probe(struct i2c_client *client);
/****************************************************************************************
 * predefined reg values
 */
#define ENDMARKER { 0xffff, 0xff }
#define DELAYMARKER { 0xfffe, 0xff }


static struct regval_list ov5645_fmt_yuv422_yuyv[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_fmt_yuv422_yvyu[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_fmt_yuv422_vyuy[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_fmt_yuv422_uyvy[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_fmt_raw[] __attribute__((unused)) =
{
    ENDMARKER,
};

/*
 *AWB
 */
static const struct regval_list ov5645_awb_regs_enable[] =
{
    ENDMARKER,
};

static const struct regval_list ov5645_awb_regs_diable[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_wb_cloud_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_wb_daylight_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_wb_incandescence_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_wb_fluorescent_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_wb_tungsten_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_colorfx_none_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_colorfx_bw_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_colorfx_sepia_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_colorfx_negative_regs[] =
{
    ENDMARKER,
};

static struct regval_list ov5645_whitebance_auto[] __attribute__((unused)) =
{
	ENDMARKER,
};

static struct regval_list ov5645_whitebance_cloudy[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5645_whitebance_sunny[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5645_whitebance_fluorescent[] __attribute__((unused)) =
{
	ENDMARKER,

};
static  struct regval_list ov5645_whitebance_incandescent[] __attribute__((unused)) =
{
	ENDMARKER,
};


static  struct regval_list ov5645_effect_normal[] __attribute__((unused)) =
{
    ENDMARKER,
};

static  struct regval_list ov5645_effect_white_black[] __attribute__((unused)) =
{
  ENDMARKER,
};

/* Effect */
static  struct regval_list ov5645_effect_negative[] __attribute__((unused)) =
{
	ENDMARKER,
};
/*复古效果*/
static  struct regval_list ov5645_effect_antique[] __attribute__((unused)) =
{
	ENDMARKER,
};

/* Scene */
static  struct regval_list ov5645_scene_auto[] __attribute__((unused)) =
{
	ENDMARKER,
};

static  struct regval_list ov5645_scene_night[] __attribute__((unused)) =
{
	ENDMARKER,
};

/*
 * register setting for window size
 */
// mediatech
static const struct regval_list ov5645_svga_init_regs[] =
{
    //;OV5645MIPI 1280x960,30fps
	//56Mhz, 224Mbps/Lane, 2 Lane
	{0x3103, 0x11},//	; PLL clock selection
	{0x3008, 0x82},//	; software reset	
     DELAYMARKER,              //          										//; delay 2ms                           					
	{0x3008, 0x42},//	; software power down
	{0x3103, 0x03},//	; clock from PLL
	{0x3503, 0x07},//	; AGC manual, AEC manual
    {0x3000, 0x30},
    {0x3004, 0xef},
	{0x3002, 0x1c},//	; system reset
	{0x3006, 0xc3},//	; clock enable
	{0x300e, 0x45},//	; MIPI 2 lane
	{0x3017, 0x40},//	; Frex, CSK input, Vsync output
	{0x3018, 0x00},//	; GPIO input
	{0x302c, 0x02},//	; GPIO input
	{0x302e, 0x0b},//
	{0x3031, 0x08},//
	{0x3611, 0x06},//
	{0x3612, 0xab},//
	{0x3614, 0x50},//
	{0x3618, 0x00},//
	{0x4800, 0x04},//chage mipi data free/gate
	{0x3034, 0x18},//	; PLL, MIPI 8-bit mode
	{0x3035, 0x21},//	; PLL
	{0x3036, 0x70},//	; PLL
	{0x3037, 0x13},// ; PLL
	{0x3108, 0x01},// ; PLL
	{0x3824, 0x01},// ; PLL
	{0x460c, 0x20},// ; PLL
	{0x3500, 0x00},//	; exposure = 0x100
	{0x3501, 0x01},//	; exposure
	{0x3502, 0x00},//	; exposure
	{0x350a, 0x00},//	; gain = 0x3f
	{0x350b, 0x3f},//	; gain
	{0x3600, 0x09},//
	{0x3601, 0x43},//
	{0x3620, 0x33},//
	{0x3621, 0xe0},//
	{0x3622, 0x01},//
	{0x3630, 0x2d},//
	{0x3631, 0x00},//
	{0x3632, 0x32},//
	{0x3633, 0x52},//
	{0x3634, 0x70},//
	{0x3635, 0x13},//
	{0x3636, 0x03},//
	{0x3702, 0x6e},//
	{0x3703, 0x52},//
	{0x3704, 0xa0},//
	{0x3705, 0x33},//
	{0x3708, 0x66},//
	{0x3709, 0x12},//
	{0x370b, 0x61},//
	{0x370c, 0xc3},//
	{0x370f, 0x10},//
	{0x3715, 0x08},//
	{0x3717, 0x01},//
	{0x371b, 0x20},//
	{0x3731, 0x22},//
	{0x3739, 0x70},//
	{0x3901, 0x0a},//
	{0x3905, 0x02},//
	{0x3906, 0x10},//
	{0x3719, 0x86},//
	{0x3800, 0x00},//	; HS = 0
	{0x3801, 0x00},//	; HS
	{0x3802, 0x00},//	; VS = 250
	{0x3803, 0x06},//	; VS
	{0x3804, 0x0a},//	; HW = 2623
	{0x3805, 0x3f},//	; HW
	{0x3806, 0x07},//	; VH = 1705
	{0x3807, 0x9d},//	; VH
	{0x3808, 0x05},//	; DVPHO = 1280
	{0x3809, 0x00},//	; DVPHO
	{0x380a, 0x03},//	; DVPHO
	{0x380b, 0xc0},//	; DVPVO
	{0x380c, 0x07},//	; HTS = 2160
	{0x380d, 0x68},//	; HTS
	{0x380e, 0x03},//	; VTS = 740
	{0x380f, 0xd8},//	; VTS
	{0x3810, 0x00},//	; H OFF = 16
	{0x3811, 0x10},//	; H OFF
	{0x3812, 0x00},//	; V OFF = 4
	{0x3813, 0x06},//	; V OFF
	{0x3814, 0x31},//	; X INC
	{0x3815, 0x31},//	; Y INC
	{0x3820, 0x47},//	; flip off, V bin on
	{0x3821, 0x07},//	; mirror on, H bin on
	{0x3826, 0x03},// 
	{0x3828, 0x08},//
	{0x3a02, 0x03},//	; max exp 60 = 740
	{0x3a03, 0xd8},//	; max exp 60
	{0x3a08, 0x01},//	; B50 = 222
	{0x3a09, 0x27},// ; B50
	{0x3a0a, 0x00},// ; B60 = 185
	{0x3a0b, 0xf6},// ; B60
	{0x3a0e, 0x03},// ; max 50
	{0x3a0d, 0x04},// ; max 60
	{0x3a14, 0x03},// ; max exp 50 = 740
	{0x3a15, 0xd8},//	; max exp 50
	{0x3a18, 0x00},//	; gain ceiling = 15.5x
	{0x3a19, 0x60},//	; gain ceiling
	{0x3a05, 0x30},//	; enable band insert, ken,  
	{0x3c01, 0xb4},// ;manual banding mode
	{0x3c00, 0x04},// ;50 Banding mode 
	{0x3c04, 0x28},//
	{0x3c05, 0x98},//
	{0x3c07, 0x07},//
	{0x3c08, 0x01},//
	{0x3c09, 0xc2},//
	{0x3c0a, 0x9c},//
	{0x3c0b, 0x40},//
	{0x4001, 0x02},//	; BLC start line
	{0x4004, 0x02},//	; BLC line number
	{0x4005, 0x18},//	; BLC update triggered by gain change
	{0x4050, 0x6e},//	; BLC line number
	{0x4051, 0x8f},//	; BLC update triggered by gain change
	{0x4300, 0x32},//	; YUV 422, YUYV
	{0x4514, 0x00},//
	{0x4520, 0xb0},//
	{0x460b, 0x37},//
	{0x4818, 0x01},//
	{0x481d, 0xf0},//
	{0x481f, 0x50},//
	{0x4823, 0x70},//
	{0x4831, 0x14},//
	{0x4837, 0x11},//
	{0x5000, 0xa7},//	; Lenc on, raw gamma on, BPC on, WPC on, color interpolation on
	{0x5001, 0xa3},//	; SDE on, scale off, UV adjust off, color matrix on, AWB on
	{0x5002, 0x80},//   
	{0x501d, 0x00},//
	{0x501f, 0x00},//	; select ISP YUV 422
	{0x503d, 0x00},//
	{0x505c, 0x30},//
	{0x5181, 0x59},//
	{0x5183, 0x00},//
	{0x5191, 0xf0},//
	{0x5192, 0x03},//    
	{0x5684, 0x10},//
	{0x5685, 0xa0},//
	{0x5686, 0x0c},//
	{0x5687, 0x78},//
	{0x5a00, 0x08},//
	{0x5a21, 0x00},//
	{0x5a24, 0x00},//
	{0x3008, 0x02},//	; wake up from software standby
	{0x3503, 0x00},//	; AGC on, AEC on
	{0x5180, 0xff},// ;awb
	{0x5181, 0xf3},//
	{0x5182, 0x0 },//
	{0x5183, 0x14},//
	{0x5184, 0x25},//
	{0x5185, 0x24},//
	{0x5186, 0xe },//
	{0x5187, 0x10},//
	{0x5188, 0xb },//
	{0x5189, 0x74},//
	{0x518a, 0x54},//
	{0x518b, 0xeb},//
	{0x518c, 0xa8},//
	{0x518d, 0x36},//
	{0x518e, 0x2d},//
	{0x518f, 0x51},//
	{0x5190, 0x40},//
	{0x5191, 0xf8},//
	{0x5192, 0x4 },//
	{0x5193, 0x70},//
	{0x5194, 0xf0},//
	{0x5195, 0xf0},//
	{0x5196, 0x3 },//
	{0x5197, 0x1 },//
	{0x5198, 0x5 },//
	{0x5199, 0xe5},//
	{0x519a, 0x4 },//
	{0x519b, 0x0 },//
	{0x519c, 0x4 },//
	{0x519d, 0x8f},//
	{0x519e, 0x38},//
	{0x5381, 0x1e},//ccm
	{0x5382, 0x5b},//
	{0x5383, 0x08},//
	{0x5384, 0x0a},//
	{0x5385, 0x7e},//
	{0x5386, 0x88},//
	{0x5387, 0x7c},//
	{0x5388, 0x6c},//
	{0x5389, 0x10},//
	{0x538a, 0x01},//
	{0x538b, 0x98},//
	{0x5300, 0x08},//	; sharpen MT th1
	{0x5301, 0x30},//	; sharpen MT th2
	{0x5302, 0x18},//	; sharpen MT off1
	{0x5303, 0x08},//	; sharpen MT off2
	{0x5304, 0x08},//	; DNS th1
	{0x5305, 0x30},//	; DNS th2
	{0x5306, 0x08},//	; DNS off1
	{0x5307, 0x16},//	; DNS off2
	{0x5309, 0x08},//	; sharpen TH th1
	{0x530a, 0x30},//	; sharpen TH th2
	{0x530b, 0x04},//	; sharpen TH th2
	{0x530c, 0x06},//	; sharpen TH off2
	{0x5480, 0x01},//	; bias on
	{0x5481, 0x08},//	; Y yst 00
	{0x5482, 0x14},//
	{0x5483, 0x28},//
	{0x5484, 0x51},//
	{0x5485, 0x65},//
	{0x5486, 0x71},//
	{0x5487, 0x7d},//
	{0x5488, 0x87},//
	{0x5489, 0x91},//
	{0x548a, 0x9a},//
	{0x548b, 0xaa},//
	{0x548c, 0xb8},//
	{0x548d, 0xcd},//
	{0x548e, 0xdd},//
	{0x548f, 0xea},//
	{0x5490, 0x1d},//
	{0x5588, 0x01},//
	{0x5580, 0x06},//
	{0x5583, 0x40},//
	{0x5584, 0x20},//
	{0x5589, 0x18},//
	{0x558a, 0x00},//
	{0x558b, 0x3c},//
	{0x5800, 0x3F},//lsc
	{0x5801, 0x1D},//
	{0x5802, 0x19},//
	{0x5803, 0x18},//
	{0x5804, 0x1E},//
	{0x5805, 0x3F},//
	{0x5806, 0x12},//
	{0x5807, 0x0D},//
	{0x5808, 0x09},//
	{0x5809, 0x09},//
	{0x580a, 0x0D},//
	{0x580b, 0x11},//
	{0x580c, 0x0E},//
	{0x580d, 0x04},//
	{0x580e, 0x01},//
	{0x580f, 0x01},//
	{0x5810, 0x05},//
	{0x5811, 0x0D},//
	{0x5812, 0x0B},//
	{0x5813, 0x04},//
	{0x5814, 0x00},//
	{0x5815, 0x00},//
	{0x5816, 0x04},//
	{0x5817, 0x0B},//
	{0x5818, 0x12},//
	{0x5819, 0x0A},//
	{0x581a, 0x06},//
	{0x581b, 0x06},//
	{0x581c, 0x0A},//
	{0x581d, 0x10},//
	{0x581e, 0x2F},//
	{0x581f, 0x17},//
	{0x5820, 0x13},//
	{0x5821, 0x13},//
	{0x5822, 0x18},//
	{0x5823, 0x33},//
	{0x5824, 0x84},//
	{0x5825, 0x27},//
	{0x5826, 0x29},//
	{0x5827, 0x27},//
	{0x5828, 0x56},//
	{0x5829, 0x47},//
	{0x582a, 0x45},//
	{0x582b, 0x54},//
	{0x582c, 0x55},//
	{0x582d, 0x27},//
	{0x582e, 0x46},//
	{0x582f, 0x62},//
	{0x5830, 0x60},//
	{0x5831, 0x62},//
	{0x5832, 0x26},//
	{0x5833, 0x37},//
	{0x5834, 0x55},//
	{0x5835, 0x63},//
	{0x5836, 0x45},//
	{0x5837, 0x17},//
	{0x5838, 0x56},//
	{0x5839, 0x2A},//
	{0x583a, 0x0A},//
	{0x583b, 0x18},//
	{0x583c, 0x36},//
	{0x583d, 0xAE},//
	{0x583e, 0x10},//
	{0x583f, 0x08},//
	{0x5840, 0x00},//
	{0x5025, 0x00},//
	{0x3a00, 0x38},//	; ae mode	
	{0x3a0f, 0x30},//	; AEC in H
	{0x3a10, 0x28},//	; AEC in L
	{0x3a1b, 0x30},//	; AEC out H
	{0x3a1e, 0x26},//	; AEC out L
	{0x3a11, 0x60},//	; control zone H
	{0x3a1f, 0x14},//	; control zone L
	{0x501d,0x10},
    {0x5680,0x00}, 
    {0x5681,0x00},
    {0x5682,0x00},  
    {0x5683,0x00},
    {0x5684,0x05},//width=256  
    {0x5685,0x00},
    {0x5686,0x03},//heght=256
    {0x5687,0xc0},
    ENDMARKER//The end flag
};

static const struct regval_list ov5645_svga_regs[] = {

    #if 0
    	//;OV5645MIPI 1280x960,30fps
	//56Mhz, 224Mbps/Lane, 2Lane.
	{0x4202, 0x0f},//	; stop mipi stream
	{0x300e, 0x45},//	; MIPI 2 lane
	{0x3034, 0x18},// PLL, MIPI 8-bit mode
	{0x3035, 0x21},// PLL
	{0x3036, 0x70},// PLL
	{0x3037, 0x13},// PLL
	{0x3108, 0x01},// PLL
	{0x3824, 0x01},// PLL
	{0x460c, 0x20},// PLL
	{0x3618, 0x00},//
	{0x3600, 0x09},//
	{0x3601, 0x43},//
	{0x3708, 0x66},//
	{0x3709, 0x12},//
	{0x370c, 0xc3},//
	{0x3800, 0x00}, // HS = 0
	{0x3801, 0x00}, // HS
	{0x3802, 0x00}, // VS = 250
	{0x3803, 0x06}, // VS
	{0x3804, 0x0a}, // HW = 2623
	{0x3805, 0x3f},//	; HW
	{0x3806, 0x07},//	; VH = 
	{0x3807, 0x9d},//	; VH
	{0x3808, 0x05},//	; DVPHO = 1280
	{0x3809, 0x00},//	; DVPHO
	{0x380a, 0x03},//	; DVPVO = 960
	{0x380b, 0xc0},//	; DVPVO
	{0x380c, 0x07},//	; HTS = 2160
	{0x380d, 0x68},//	; HTS
	{0x380e, 0x03},//	; VTS = 740
	{0x380f, 0xd8},//	; VTS
	{0x3810, 0x00}, // H OFF = 16
	{0x3811, 0x10}, // H OFF
	{0x3812, 0x00}, // V OFF = 4
	{0x3813, 0x06},//	; V OFF
	{0x3814, 0x31},//	; X INC
	{0x3815, 0x31},//	; Y INC
	{0x3820, 0x47},//	; flip off, V bin on
	{0x3821, 0x07},//	; mirror on, H bin on
	{0x4514, 0x00},
	{0x3a02, 0x03},//	; max exp 60 = 740
	{0x3a03, 0xd8},//	; max exp 60
	{0x3a08, 0x01},//	; B50 = 222
	{0x3a09, 0x27},//	; B50
	{0x3a0a, 0x00},//	; B60 = 185
	{0x3a0b, 0xf6},//	; B60
	{0x3a0e, 0x03},//	; max 50
	{0x3a0d, 0x04},//	; max 60
	{0x3a14, 0x03},//	; max exp 50 = 740
	{0x3a15, 0xd8},//	; max exp 50
	{0x3c07, 0x07},//	; 50/60 auto detect
	{0x3c08, 0x01},//	; 50/60 auto detect
	{0x3c09, 0xc2},//	; 50/60 auto detect
	{0x4004, 0x02},//	; BLC line number
	{0x4005, 0x18},//	; BLC triggered by gain change
	{0x4837, 0x11}, // MIPI global timing 16           
	{0x503d, 0x00},//
	{0x5000, 0xa7},//
	{0x5001, 0x83},//
	{0x5002, 0x80},//
	{0x5003, 0x08},//
	{0x3032, 0x00},//
	{0x4000, 0x89},//
	{0x5583, 0x40},//
	{0x5584, 0x20},//
	{0x3a00, 0x3c},//	; ae mode	
	{0x4202, 0x00},//	; open mipi stream

    {0x503d,0x80},// color bar test
	//OV5645MIPIWriteExtraShutter(OV5645MIPISensor.PreviewExtraShutter);


    #else
        	{0x4202, 0x0f},//	; stop mipi stream
        {0x300e, 0x45},//	; MIPI 2 lane
		{0x3034, 0x18},// PLL, MIPI 8-bit mode
		{0x3035, 0x21}, // PLL
		{0x3036, 0x38}, // PLL
		{0x3037, 0x13}, // PLL
		{0x3108, 0x01}, // PLL
		{0x3824, 0x01}, // PLL
		{0x460c, 0x20}, // PLL
		{0x3618, 0x00},//
		{0x3600, 0x09},//
		{0x3601, 0x43},//
		{0x3708, 0x66},//
		{0x3709, 0x12},//
		{0x370c, 0xc3},//
		{0x3800, 0x00}, // HS = 0
		{0x3801, 0x00}, // HS
		{0x3802, 0x00}, // VS = 250
		{0x3803, 0x06}, // VS
		{0x3804, 0x0a}, // HW = 2623
		{0x3805, 0x3f},//	; HW
		{0x3806, 0x07},//	; VH = 
		{0x3807, 0x9d},//	; VH
		{0x3808, 0x05},//	; DVPHO = 1280
		{0x3809, 0x00},//	; DVPHO
		{0x380a, 0x03},//	; DVPVO = 960
		{0x380b, 0xc0},//	; DVPVO
		{0x380c, 0x07},//	; HTS = 2160
		{0x380d, 0x68},//	; HTS
		{0x380e, 0x03},//	; VTS = 740
		{0x380f, 0xd8},//	; VTS
		{0x3810, 0x00}, // H OFF = 16
		{0x3811, 0x10}, // H OFF
		{0x3812, 0x00}, // V OFF = 4
		{0x3813, 0x06},//	; V OFF
		{0x3814, 0x31},//	; X INC
		{0x3815, 0x31},//	; Y INC
		{0x3820, 0x47},//	; flip off, V bin on
		{0x3821, 0x07},//	; mirror on, H bin on
		{0x4514, 0x00},
		{0x3a00, 0x38},//	; ae mode	
		{0x3a02, 0x03},//	; max exp 60 = 740
		{0x3a03, 0xd8},//	; max exp 60
		{0x3a08, 0x00},//	; B50 = 222
		{0x3a09, 0x94},//	; B50
		{0x3a0a, 0x00},//	; B60 = 185
		{0x3a0b, 0x7b},//	; B60
		{0x3a0e, 0x06},//	; max 50
		{0x3a0d, 0x07},//	; max 60
		{0x3a14, 0x03},//	; max exp 50 = 740
		{0x3a15, 0xd8},//	; max exp 50
		{0x3c07, 0x08},//	; 50/60 auto detect
		{0x3c08, 0x00},//	; 50/60 auto detect
		{0x3c09, 0x1c},//	; 50/60 auto detect
		{0x4004, 0x02},//	; BLC line number
		{0x4005, 0x18},//	; BLC triggered by gain change
		{0x4837, 0x11}, // MIPI global timing 16           
		{0x503d, 0x00},//
		{0x5000, 0xa7},//
		{0x5001, 0xa3},//
		{0x5002, 0x80},//
		{0x5003, 0x08},//
		{0x3032, 0x00},//
		{0x4000, 0x89},//
		{0x350c, 0x00},//
		{0x350d, 0x00},//
        {0x5583, 0x40},//
	{0x5584, 0x20},//
	{0x3a00, 0x3c},//	; ae mode	
	{0x4202, 0x00},//	; open mipi stream

   //      {0x503d,0x80},// color bar test
    #endif
    ENDMARKER
};

static const struct regval_list ov5645_uxga_regs[] = {//caichsh
    ENDMARKER,
};

static const struct regval_list ov5645_disable_regs[] = {
    ENDMARKER,
};

/****************************************************************************************
 * structures
 */
struct ov5645_win_size {
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

struct ov5645_priv {
    struct v4l2_subdev                  subdev;
    struct media_pad                    pad;
    struct v4l2_ctrl_handler            hdl;
    const struct ov5645_color_format    *cfmt;
    const struct ov5645_win_size        *win;
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

struct ov5645_color_format {
    enum v4l2_mbus_pixelcode code;
    enum v4l2_colorspace colorspace;
};

/****************************************************************************************
 * tables
 */
static const struct ov5645_color_format ov5645_cfmts[] = {
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
#define VGA_WIDTH           640
#define VGA_HEIGHT          480
#define UXGA_WIDTH          1600
#define UXGA_HEIGHT         1200
#define SVGA_WIDTH          1280
#define SVGA_HEIGHT         960
#define OV5645_MAX_WIDTH    UXGA_WIDTH
#define OV5645_MAX_HEIGHT   UXGA_HEIGHT
#define AHEAD_LINE_NUM		15    //10行 = 50次循环(OV5645)
#define DROP_NUM_CAPTURE			0
#define DROP_NUM_PREVIEW			0

static unsigned int frame_rate_svga[] = {12,};
static unsigned int frame_rate_uxga[] = {12,};

/* 800X600 */
static const struct ov5645_win_size ov5645_win_svga = {
    .name     = "SVGA",
    .width    = SVGA_WIDTH,
    .height   = SVGA_HEIGHT,
    .win_regs = ov5645_svga_regs,
    .frame_rate_array = frame_rate_svga,
};

/* 1600X1200 */
static const struct ov5645_win_size ov5645_win_uxga = {
    .name     = "UXGA",
    .width    = UXGA_WIDTH,
    .height   = UXGA_HEIGHT,
    .win_regs = ov5645_uxga_regs,
    .frame_rate_array = frame_rate_uxga,
};

static const struct ov5645_win_size *ov5645_win[] = {
    &ov5645_win_svga,
    &ov5645_win_uxga,
};

/****************************************************************************************
 * general functions
 */
static inline struct ov5645_priv *to_priv(struct v4l2_subdev *subdev)
{
    return container_of(subdev, struct ov5645_priv, subdev);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct ov5645_priv, hdl)->subdev;
}
/*
static bool check_id(struct i2c_client *client)
{
    u8 pid = i2c_smbus_read_byte_data(client, PID);
    if (pid == ov5645)
        return true;

    printk(KERN_ERR "failed to check id: 0x%x\n", pid);
    return false;
}*/
/*
static int ov5645_write_array(struct i2c_client *client, const struct regval_list *vals)
{
    int ret;
    while (vals->reg_num != 0xff) {
        ret = i2c_smbus_write_byte_data(client, vals->reg_num, vals->value);
        if (ret < 0)
            return ret;
        vals++;
    }
    return 0;
}*/
/*
static int ov5645_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set) __attribute__((unused));
static int ov5645_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set)
{
    s32 val = i2c_smbus_read_byte_data(client, command);
    if (val < 0)
        return val;

    val &= ~mask;
    val |= set & mask;

    return i2c_smbus_write_byte_data(client, command, val);
}
*/



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

/*
 * convenience function to write 16 bit register values that are split up
 * into two consecutive high and low parts
 */
static int reg_write16(struct i2c_client *client, u16 reg, u16 val16)
{
	int ret;

	ret = reg_write(client, reg, val16 >> 8);
	if (ret)
		return ret;
	return reg_write(client, reg + 1, val16 & 0x00ff);
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov5645_get_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
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

static int ov5645_set_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg & ~0xffff || reg->val & ~0xff)
		return -EINVAL;

	return reg_write(client, reg->reg, reg->val);
}
#endif

static int ov5645_write_array(struct i2c_client *client,
				const struct regval_list *vals)
{
	while (vals->reg_num != 0xffff || vals->value != 0xff) {
        if (vals->reg_num == 0xfffe) {
            msleep(5);
            vals++;
            continue;
        }
        int ret = reg_write(client, vals->reg_num, vals->value); 
		if (ret < 0)
			return ret;
		vals++;
	}
	dev_dbg(&client->dev, "Register list loaded\n");
	return 0;
}




static const struct ov5645_win_size *ov5645_select_win(u32 width, u32 height)
{
	const struct ov5645_win_size *win;
    int i;
    printk("%s.............%d...........\n",__func__,__LINE__);

    for (i = 0; i < ARRAY_SIZE(ov5645_win); i++) {
        win = ov5645_win[i];
        if (width == win->width && height == win->height)
            return win;
    }

    printk(KERN_ERR "%s: unsupported width, height (%dx%d)\n", __func__, width, height);
    return NULL;
}

static int ov5645_set_mbusformat(struct i2c_client *client, const struct ov5645_color_format *cfmt)
{

    printk("%s.............%d...........\n",__func__,__LINE__);
    enum v4l2_mbus_pixelcode code;
    int ret = -1;
    code = cfmt->code;
    switch (code) {
        case V4L2_MBUS_FMT_YUYV8_2X8:
            ret  = ov5645_write_array(client, ov5645_fmt_yuv422_yuyv);
            break;
        case V4L2_MBUS_FMT_UYVY8_2X8:
            ret  = ov5645_write_array(client, ov5645_fmt_yuv422_uyvy);
            break;
        case V4L2_MBUS_FMT_YVYU8_2X8:
            ret  = ov5645_write_array(client, ov5645_fmt_yuv422_yvyu);
            break;
        case V4L2_MBUS_FMT_VYUY8_2X8:
            ret  = ov5645_write_array(client, ov5645_fmt_yuv422_vyuy);
            break;
        default:
            printk(KERN_ERR "mbus code error in %s() line %d\n",__FUNCTION__, __LINE__);
    }
    return ret;
}

static int ov5645_set_params(struct v4l2_subdev *sd, u32 *width, u32 *height, enum v4l2_mbus_pixelcode code)
{
    struct ov5645_priv *priv = to_priv(sd);
    const struct ov5645_win_size *old_win, *new_win;
    int i;
    printk("%s.............%d...........\n",__func__,__LINE__);

    /*
     * select format
     */
    priv->cfmt = NULL;
    for (i = 0; i < ARRAY_SIZE(ov5645_cfmts); i++) {
        if (code == ov5645_cfmts[i].code) {
            priv->cfmt = ov5645_cfmts + i;
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
    new_win = ov5645_select_win(*width, *height);
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
static int ov5645_set_brightness(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    printk("%s.............%d...........\n",__func__,__LINE__);

    return 0;
}

static int ov5645_set_contrast(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* TODO */
    int contrast = ctrl->val;
    printk("%s: val %d\n", __func__, contrast);

    return 0;
}

static int ov5645_set_auto_white_balance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct ov5645_priv *priv = to_priv(sd); */
    int auto_white_balance = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, auto_white_balance);
    if (auto_white_balance < 0 || auto_white_balance > 1) {
        dev_err(&client->dev, "set auto_white_balance over range, auto_white_balance = %d\n", auto_white_balance);
        return -ERANGE;
    }

    switch(auto_white_balance) {
        case 0:
            OV5645_DEBUG(KERN_ERR "===awb disable===\n");
            ret = ov5645_write_array(client, ov5645_awb_regs_diable);
            break;
        case 1:
            OV5645_DEBUG(KERN_ERR "===awb enable===\n");
            ret = ov5645_write_array(client, ov5645_awb_regs_enable);
            break;
    }

    assert(ret == 0);

    return 0;
}

/* TODO : exposure */
static int ov5645_set_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* struct ov5645_priv *priv = to_priv(sd); */
    printk("%s: val %d\n", __func__, ctrl->val);

    return 0;
}

/* TODO */
static int ov5645_set_gain(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* struct ov5645_priv *priv = to_priv(sd); */
    printk("%s: val %d\n", __func__, ctrl->val);
    return 0;
}

/* TODO */
static int ov5645_set_hflip(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* struct ov5645_priv *priv = to_priv(sd); */
    printk("%s: val %d\n", __func__, ctrl->val);
    return 0;
}

static int ov5645_set_white_balance_temperature(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct ov5645_priv *priv = to_priv(sd); */
    int white_balance_temperature = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch(white_balance_temperature) {
        case V4L2_WHITE_BALANCE_INCANDESCENT:
            ret = ov5645_write_array(client, ov5645_wb_incandescence_regs);
            break;
        case V4L2_WHITE_BALANCE_FLUORESCENT:
            ret = ov5645_write_array(client, ov5645_wb_fluorescent_regs);
            break;
        case V4L2_WHITE_BALANCE_DAYLIGHT:
            ret = ov5645_write_array(client, ov5645_wb_daylight_regs);
            break;
        case V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT:
            ret = ov5645_write_array(client, ov5645_wb_cloud_regs);
            break;
        case V4L2_WHITE_BALANCE_TUNGSTEN:
            ret = ov5645_write_array(client, ov5645_wb_tungsten_regs);
            break;
        default:
            dev_err(&client->dev, "set white_balance_temperature over range, white_balance_temperature = %d\n", white_balance_temperature);
            return -ERANGE;
    }

    assert(ret == 0);

    return 0;
}

static int ov5645_set_colorfx(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct ov5645_priv *priv = to_priv(sd); */
    int colorfx = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch (colorfx) {
        case V4L2_COLORFX_NONE: /* normal */
            ret = ov5645_write_array(client, ov5645_colorfx_none_regs);
            break;
        case V4L2_COLORFX_BW: /* black and white */
            ret = ov5645_write_array(client, ov5645_colorfx_bw_regs);
            break;
        case V4L2_COLORFX_SEPIA: /* antique ,复古*/
            ret = ov5645_write_array(client, ov5645_colorfx_sepia_regs);
            break;
        case V4L2_COLORFX_NEGATIVE: /* negative，负片 */
            ret = ov5645_write_array(client, ov5645_colorfx_negative_regs);
            break;
        default:
            dev_err(&client->dev, "set colorfx over range, colorfx = %d\n", colorfx);
            return -ERANGE;
    }

    assert(ret == 0);
    return 0;
}

static int ov5645_set_exposure_auto(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /* struct ov5645_priv *priv = to_priv(sd); */
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
static int ov5645_set_scene_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
#if 0
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov5645_priv *priv = to_priv(sd);
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

static int ov5645_set_prev_capt_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov5645_priv *priv = to_priv(sd);
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

static int ov5645_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret = 0;
    printk("%s.............%d...........\n",__func__,__LINE__);

    switch (ctrl->id) {
        case V4L2_CID_BRIGHTNESS:
            ov5645_set_brightness(sd, ctrl);
            break;

        case V4L2_CID_CONTRAST:
            ov5645_set_contrast(sd, ctrl);
            break;

        case V4L2_CID_AUTO_WHITE_BALANCE:
            ov5645_set_auto_white_balance(sd, ctrl);
            break;

        case V4L2_CID_EXPOSURE:
            ov5645_set_exposure(sd, ctrl);
            break;

        case V4L2_CID_GAIN:
            ov5645_set_gain(sd, ctrl);
            break;

        case V4L2_CID_HFLIP:
            ov5645_set_hflip(sd, ctrl);
            break;

        case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
            ov5645_set_white_balance_temperature(sd, ctrl);
            break;

        case V4L2_CID_COLORFX:
            ov5645_set_colorfx(sd, ctrl);
            break;

        case V4L2_CID_EXPOSURE_AUTO:
            ov5645_set_exposure_auto(sd, ctrl);
            break;

        case V4L2_CID_SCENE_EXPOSURE:
            ov5645_set_scene_exposure(sd, ctrl);
            break;

        case V4L2_CID_PRIVATE_PREV_CAPT:
            ov5645_set_prev_capt_mode(sd, ctrl);
            break;

        default:
            dev_err(&client->dev, "%s: invalid control id %d\n", __func__, ctrl->id);
            return -EINVAL;
    }

    return ret;
}

static const struct v4l2_ctrl_ops ov5645_ctrl_ops = {
    .s_ctrl = ov5645_s_ctrl,
};

static const struct v4l2_ctrl_config ov5645_custom_ctrls[] = {
    {
        .ops    = &ov5645_ctrl_ops,
        .id     = V4L2_CID_SCENE_EXPOSURE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "SceneExposure",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &ov5645_ctrl_ops,
        .id     = V4L2_CID_PRIVATE_PREV_CAPT,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "PrevCapt",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }
};

static int ov5645_initialize_ctrls(struct ov5645_priv *priv)
{
    printk("%s.............%d...........\n",__func__,__LINE__);

    v4l2_ctrl_handler_init(&priv->hdl, NUM_CTRLS);

    /* standard ctrls */
    priv->brightness = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_BRIGHTNESS, 0, 6, 1, 0);
    if (!priv->brightness) {
        printk(KERN_ERR "%s: failed to create brightness ctrl\n", __func__);
        return -ENOENT;
    }

    priv->contrast = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_CONTRAST, -6, 6, 1, 0);
    if (!priv->contrast) {
        printk(KERN_ERR "%s: failed to create contrast ctrl\n", __func__);
        return -ENOENT;
    }

    priv->auto_white_balance = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
    if (!priv->auto_white_balance) {
        printk(KERN_ERR "%s: failed to create auto_white_balance ctrl\n", __func__);
        return -ENOENT;
    }

#if 0
    priv->exposure = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_EXPOSURE, 0, 0xFFFF, 1, 500);
    if (!priv->exposure) {
        printk(KERN_ERR "%s: failed to create exposure ctrl\n", __func__);
        return -ENOENT;
    }
#endif

    priv->gain = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_GAIN, 0, 0xFF, 1, 128);
    if (!priv->gain) {
        printk(KERN_ERR "%s: failed to create gain ctrl\n", __func__);
        return -ENOENT;
    }

    priv->hflip = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_HFLIP, 0, 1, 1, 0);
    if (!priv->hflip) {
        printk(KERN_ERR "%s: failed to create hflip ctrl\n", __func__);
        return -ENOENT;
    }

    priv->white_balance_temperature = v4l2_ctrl_new_std(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 3, 1, 1);
    if (!priv->white_balance_temperature) {
        printk(KERN_ERR "%s: failed to create white_balance_temperature ctrl\n", __func__);
        return -ENOENT;
    }

    /* standard menus */
    priv->colorfx = v4l2_ctrl_new_std_menu(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_COLORFX, 3, 0, 0);
    if (!priv->colorfx) {
        printk(KERN_ERR "%s: failed to create colorfx ctrl\n", __func__);
        return -ENOENT;
    }

    priv->exposure_auto = v4l2_ctrl_new_std_menu(&priv->hdl, &ov5645_ctrl_ops,
            V4L2_CID_EXPOSURE_AUTO, 1, 0, 1);
    if (!priv->exposure_auto) {
        printk(KERN_ERR "%s: failed to create exposure_auto ctrl\n", __func__);
        return -ENOENT;
    }

    /* custom ctrls */
    priv->scene_exposure = v4l2_ctrl_new_custom(&priv->hdl, &ov5645_custom_ctrls[0], NULL);
    if (!priv->scene_exposure) {
        printk(KERN_ERR "%s: failed to create scene_exposure ctrl\n", __func__);
        return -ENOENT;
    }

    priv->prev_capt = v4l2_ctrl_new_custom(&priv->hdl, &ov5645_custom_ctrls[1], NULL);
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

static int ov5645_save_exposure_param(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov5645_priv *priv = to_priv(sd);
    int ret = 0;
  /*  unsigned int reg_0x03 = 0x20;
    unsigned int reg_0x80;
    unsigned int reg_0x81;
    unsigned int reg_0x82;

    i2c_smbus_write_byte_data(client, 0x03, reg_0x03); //page 20
    reg_0x80 = i2c_smbus_read_byte_data(client, 0x80);
    reg_0x81 = i2c_smbus_read_byte_data(client, 0x81);
    reg_0x82 = i2c_smbus_read_byte_data(client, 0x82);

    priv->preview_exposure_param.shutter = (reg_0x80 << 16)|(reg_0x81 << 8)|reg_0x82;
    priv->capture_exposure_param.shutter = (priv->preview_exposure_param.shutter)/2;
*/
    return ret;
}

static int ov5645_set_exposure_param(struct v4l2_subdev *sd) __attribute__((unused));
static int ov5645_set_exposure_param(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov5645_priv *priv = to_priv(sd);
    int ret;
 /*   unsigned int reg_0x03 = 0x20;
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
*/
    return 0;
}

/****************************************************************************************
 * v4l2 subdev ops
 */

/**
 * core ops
 */
static int ov5645_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *id)
{
    printk("%s.............%d...........\n",__func__,__LINE__);

    struct ov5645_priv *priv = to_priv(sd);
    id->ident    = priv->model;
    id->revision = 0;
    return 0;
}

static int ov5645_s_power(struct v4l2_subdev *sd, int on)
{
    /* used when suspending */
    /* printk("%s: on %d\n", __func__, on); */
    if (!on) {
        struct ov5645_priv *priv = to_priv(sd);
        priv->initialized = false;
    }
    printk("%s.............%d...........\n",__func__,__LINE__);

    return 0;
}

static const struct v4l2_subdev_core_ops ov5645_subdev_core_ops = {
    .g_chip_ident	= ov5645_g_chip_ident,
    .s_power        = ov5645_s_power,
    .s_ctrl         = v4l2_subdev_s_ctrl,
};

/**
 * video ops
 */
static int ov5645_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov5645_priv *priv = to_priv(sd);
    int ret = 0;

    printk("%s: enable %d, initialized %d\n", __func__, enable, priv->initialized);
    ov5645_video_probe(client);
    if (enable) {
        if (!priv->win || !priv->cfmt) {
            dev_err(&client->dev, "norm or win select error\n");
            return -EPERM;
        }
         /* write init regs */
        if (!priv->initialized) {
           // if (!check_id(client))
            //    return -EINVAL;

            ret = ov5645_write_array(client, ov5645_svga_init_regs);
            if (ret < 0) {
                printk(KERN_ERR "%s: failed to ov5645_write_array init regs\n", __func__);
                return -EIO;
            }
            priv->initialized = true;
            printk(KERN_ERR "%s ov5645_write_array init regs\n", __func__);
        }

        ret = ov5645_write_array(client, priv->win->win_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to ov5645_write_array win regs\n", __func__);
            return -EIO;
        }
        printk(KERN_ERR "%s: ov5645_write_array win regs\n", __func__);

        ret = ov5645_set_mbusformat(client, priv->cfmt);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to ov5645_set_mbusformat()\n", __func__);
            return -EIO;
        }
    } else {
        ov5645_write_array(client, ov5645_disable_regs);
    }

    return ret;
}

static int ov5645_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    printk("%s.............%d...........\n",__func__,__LINE__);

    if (fsize->index >= ARRAY_SIZE(ov5645_win)) {
        dev_err(&client->dev, "index(%d) is over range %d\n", fsize->index, ARRAY_SIZE(ov5645_win));
        return -EINVAL;
    }

    switch (fsize->pixel_format) {
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUV422P:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
            fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
            fsize->discrete.width = ov5645_win[fsize->index]->width;
            fsize->discrete.height = ov5645_win[fsize->index]->height;
            break;
        default:
            dev_err(&client->dev, "pixel_format(%d) is Unsupported\n", fsize->pixel_format);
            return -EINVAL;
    }

    dev_info(&client->dev, "type %d, width %d, height %d\n", V4L2_FRMSIZE_TYPE_DISCRETE, fsize->discrete.width, fsize->discrete.height);
    return 0;
}

static int ov5645_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned int index,
        enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(ov5645_cfmts))
        return -EINVAL;
    printk("%s.............%d...........\n",__func__,__LINE__);

    *code = ov5645_cfmts[index].code;
    return 0;
}

static int ov5645_g_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov5645_priv *priv = to_priv(sd);
    if (!priv->win || !priv->cfmt) {
        u32 width = SVGA_WIDTH;
        u32 height = SVGA_HEIGHT;
        int ret = ov5645_set_params(sd, &width, &height, V4L2_MBUS_FMT_UYVY8_2X8);
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

static int ov5645_try_mbus_fmt(struct v4l2_subdev *sd,
        struct v4l2_mbus_framefmt *mf)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    struct ov5645_priv *priv = to_priv(sd);
    const struct ov5645_win_size *win;
    int i;
    printk("%s.............%d...........\n",__func__,__LINE__);

    /*
     * select suitable win
     */
    win = ov5645_select_win(mf->width, mf->height);
    if (!win)
        return -EINVAL;

    mf->width   = win->width;
    mf->height  = win->height;
    mf->field   = V4L2_FIELD_NONE;


    for (i = 0; i < ARRAY_SIZE(ov5645_cfmts); i++)
        if (mf->code == ov5645_cfmts[i].code)
            break;

    if (i == ARRAY_SIZE(ov5645_cfmts)) {
        /* Unsupported format requested. Propose either */
        if (priv->cfmt) {
            /* the current one or */
            mf->colorspace = priv->cfmt->colorspace;
            mf->code = priv->cfmt->code;
        } else {
            /* the default one */
            mf->colorspace = ov5645_cfmts[0].colorspace;
            mf->code = ov5645_cfmts[0].code;
        }
    } else {
        /* Also return the colorspace */
        mf->colorspace	= ov5645_cfmts[i].colorspace;
    }

    return 0;
}

static int ov5645_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    struct ov5645_priv *priv = to_priv(sd);
    printk("%s.............%d...........\n",__func__,__LINE__);

    int ret = ov5645_set_params(sd, &mf->width, &mf->height, mf->code);
    if (!ret)
        mf->colorspace = priv->cfmt->colorspace;

    return ret;
}

static const struct v4l2_subdev_video_ops ov5645_subdev_video_ops = {
    .s_stream               = ov5645_s_stream,
    .enum_framesizes        = ov5645_enum_framesizes,
    .enum_mbus_fmt          = ov5645_enum_mbus_fmt,
    .g_mbus_fmt             = ov5645_g_mbus_fmt,
    .try_mbus_fmt           = ov5645_try_mbus_fmt,
    .s_mbus_fmt             = ov5645_s_mbus_fmt,
};

/**
 * pad ops
 */
static int ov5645_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    struct v4l2_mbus_framefmt *mf = &fmt->format;
    printk("%s: %dx%d\n", __func__, mf->width, mf->height);
    return ov5645_s_mbus_fmt(sd, mf);
}

static const struct v4l2_subdev_pad_ops ov5645_subdev_pad_ops = {
    .set_fmt    = ov5645_s_fmt,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops ov5645_subdev_ops = {
    .core   = &ov5645_subdev_core_ops,
    .video  = &ov5645_subdev_video_ops,
    .pad    = &ov5645_subdev_pad_ops,
};

/**
 * media_entity_operations
 */
static int ov5645_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    printk("%s\n", __func__);
    return 0;
}

static const struct media_entity_operations ov5645_media_ops = {
    .link_setup = ov5645_link_setup,
};

/****************************************************************************************
 * initialize
 */
static void ov5645_priv_init(struct ov5645_priv * priv)
{
    priv->model = V4L2_IDENT_OV5645;
    priv->prev_capt_mode = PREVIEW_MODE;
    priv->timeperframe.denominator = 12;//30;
    priv->timeperframe.numerator = 1;
    priv->win = &ov5645_win_svga;
}

static int ov5645_video_probe(struct i2c_client *client)
{
	int ret;
	u8 id_high, id_low;
	u16 id;

	/* Read sensor Model ID */
	ret = reg_read(client, 0x300a, &id_high);
	if (ret < 0)
    {
        printk("OV5645 read error!!!!!!!!!!!!!!!\n");
		return ret;
    }

	id = id_high << 8;

	ret = reg_read(client, 0x300b, &id_low);
	if (ret < 0)
    {
        printk("OV5645 read error!!!!!!!!!!!!!!!\n");
		return ret;
    }

	id |= id_low;

	printk("OV5645 Chip ID 0x%04x detected\n", id);

	if (id != 0x5642)
		return -ENODEV;

	return 0;
}

static int ov5645_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct ov5645_priv *priv;
    struct v4l2_subdev *sd;
    int ret;

    priv = kzalloc(sizeof(struct ov5645_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;


    printk("%s...................................\n",__func__);
    ov5645_priv_init(priv);

    sd = &priv->subdev;
    strcpy(sd->name, MODULE_NAME);

    ov5645_video_probe(client);

    /* register subdev */
    v4l2_i2c_subdev_init(sd, client, &ov5645_subdev_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    priv->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    sd->entity.ops  = &ov5645_media_ops;
    if (media_entity_init(&sd->entity, 1, &priv->pad, 0)) {
        dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
        kfree(priv);
        return -ENOENT;
    }

    ret = ov5645_initialize_ctrls(priv);
    if (ret < 0) {
        printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
        kfree(priv);
        return ret;
    }

    return 0;
}

static int ov5645_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    v4l2_device_unregister_subdev(sd);
    v4l2_ctrl_handler_free(sd->ctrl_handler);
    media_entity_cleanup(&sd->entity);
    kfree(to_priv(sd));
    return 0;
}

static const struct i2c_device_id ov5645_id[] = {
    { MODULE_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, ov5645_id);

static struct i2c_driver ov5645_i2c_driver = {
    .driver = {
        .name = MODULE_NAME,
    },
    .probe    = ov5645_probe,
    .remove   = ov5645_remove,
    .id_table = ov5645_id,
};

module_i2c_driver(ov5645_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for ov5645");
MODULE_AUTHOR("caichsh(caichsh@artekmicro.com)");
MODULE_LICENSE("GPL v2");
