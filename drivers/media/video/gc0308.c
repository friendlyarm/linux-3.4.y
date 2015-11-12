
/*
 * gc0308 Camera Driver
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


#define MODULE_NAME "GC0308"

#ifdef GC0308_DEBUG
#define assert(expr) \
    if (unlikely(!(expr))) {				\
        pr_err("Assertion failed! %s,%s,%s,line=%d\n",	\
#expr, __FILE__, __func__, __LINE__);	\
    }

#define GC0308_DEBUG(fmt,args...) printk(KERN_ALERT fmt, ##args)
#else

#define assert(expr) do {} while (0)

#define GC0308_DEBUG(fmt,args...)
#endif

#define PID                 0x00 /* Product ID Number  *///caichsh
#define SENSOR_ID           0x9b
#define NUM_CTRLS           6
#define V4L2_IDENT_GC0308   64113
#define GC0308_12M_MCLK

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

/********************************************************************************
 * reg define
 */
#define GC0308 0x27 //pga

#define OUTTO_SENSO_CLOCK 24000000


/********************************************************************************
 * predefine reg values
 */
struct regval_list {
    unsigned char reg_num;
    unsigned char value;
};

#define ENDMARKER { 0xff, 0xff }


static struct regval_list gc0308_init_regs[] =
{
    #if 0
    {0xfe , 0x80},  	
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
		
	{0xfe , 0x00},   // set page0
	
	{0xd2 , 0x10},   // close AEC
	{0x22 , 0x55},   // close AWB

	{0x03 , 0x01},                                  
	{0x04 , 0x2c},                                  
	{0x5a , 0x56},
	{0x5b , 0x40},
	{0x5c , 0x4a},			

	{0x22 , 0x57},   // Open AWB

	{0x01 , 0xfa},                                    
	{0x02 , 0x70},                                  
	{0x0f , 0x01},                                  
                                                                   
                                                                   
	{0xe2 , 0x00},   //anti-flicker step [11:8]     
	{0xe3 , 0x64},   //anti-flicker step [7:0]      
		                                                               
	{0xe4 , 0x02},   //exp level 1  16.67fps        
	{0xe5 , 0x58},                                  
	{0xe6 , 0x03},   //exp level 2  12.5fps         
	{0xe7 , 0x20},                                  
	{0xe8 , 0x04},   //exp level 3  8.33fps         
	{0xe9 , 0xb0},                                  
	{0xea , 0x09},   //exp level 4  4.00fps         
	{0xeb , 0xc4},                                  

	{0xec , 0x20},
	   
	{0x05 , 0x00},                                  
	{0x06 , 0x00},                                  
	{0x07 , 0x00},                                  
	{0x08 , 0x00},                                  
	{0x09 , 0x01},                                  
	{0x0a , 0xe8},                                  
	{0x0b , 0x02},                                  
	{0x0c , 0x88},                                  
	{0x0d , 0x02},                                  
	{0x0e , 0x02},                                  
	{0x10 , 0x22},                                  
	{0x11 , 0xfd},                                  
	{0x12 , 0x2a},  //2a                                
	{0x13 , 0x00},                                  
	{0x14 , 0x10},  //0x10
	//-------------H_V_Switch(4)---------------//
	/*	1:  // normal
	    		{0x14 , 0x10},    		
		2:  // IMAGE_H_MIRROR
	    		{0x14 , 0x11},
	    		
		3:  // IMAGE_V_MIRROR
	    		{0x14 , 0x12},
	    		
		4:  // IMAGE_HV_MIRROR
	    		{0x14 , 0x13},*/			                            
	{0x15 , 0x0a},                                  
	{0x16 , 0x05},                                  
	{0x17 , 0x01},                                  
	{0x18 , 0x44},                                  
	{0x19 , 0x44},                                  
	{0x1a , 0x1e},                                  
	{0x1b , 0x00},                                  
	{0x1c , 0xc1},                                  
	{0x1d , 0x08},                                  
	{0x1e , 0x60},                                  
	{0x1f , 0x17},                                  
                                                                   
	                                                                 
	{0x20 , 0xff},                                  
	{0x21 , 0xf8},                                  
	{0x22 , 0x57},                                  
	{0x24 , 0xa3},                                  
	{0x25 , 0x0f},                                  
	                                                                 
	//output sync_mode                                               
	{0x26 , 0x03},   //0x03  20101016 zhj                                 
	{0x2f , 0x01},                                  
	{0x30 , 0xf7},                                  
	{0x31 , 0x50},
	{0x32 , 0x00},
	{0x39 , 0x04},
	{0x3a , 0x18},
	{0x3b , 0x20},                                  
	{0x3c , 0x00},                                  
	{0x3d , 0x00},                                  
	{0x3e , 0x00},                                  
	{0x3f , 0x00},                                  
	{0x50 , 0x10},                                  
	{0x53 , 0x82},                                  
	{0x54 , 0x80},                                  
	{0x55 , 0x80},                                  
	{0x56 , 0x82},                                  
	{0x8b , 0x40},                                  
	{0x8c , 0x40},                                  
	{0x8d , 0x40},                                  
	{0x8e , 0x2e},                                  
	{0x8f , 0x2e},                                  
	{0x90 , 0x2e},                                  
	{0x91 , 0x3c},                                  
	{0x92 , 0x50},                                  
	{0x5d , 0x12},                                  
	{0x5e , 0x1a},                                  
	{0x5f , 0x24},                                  
	{0x60 , 0x07},                                  
	{0x61 , 0x15},                                  
	{0x62 , 0x08},                                  
	{0x64 , 0x03},                                  
	{0x66 , 0xe8},                                  
	{0x67 , 0x86},                                  
	{0x68 , 0xa2},                                  
	{0x69 , 0x18},                                  
	{0x6a , 0x0f},                                  
	{0x6b , 0x00},                                  
	{0x6c , 0x5f},                                  
	{0x6d , 0x8f},                                  
	{0x6e , 0x55},                                  
	{0x6f , 0x38},                                  
	{0x70 , 0x15},                                  
	{0x71 , 0x33},                                  
	{0x72 , 0xdc},                                  
	{0x73 , 0x80},                                  
	{0x74 , 0x02},                                  
	{0x75 , 0x3f},                                  
	{0x76 , 0x02},                                  
	{0x77 , 0x54},                                  
	{0x78 , 0x88},                                  
	{0x79 , 0x81},                                  
	{0x7a , 0x81},                                  
	{0x7b , 0x22},                                  
	{0x7c , 0xff},                                  
	{0x93 , 0x48},                                  
	{0x94 , 0x00},                                  
	{0x95 , 0x05},                                  
	{0x96 , 0xe8},                                  
	{0x97 , 0x40},                                  
	{0x98 , 0xf0},                                  
	{0xb1 , 0x38},                                  
	{0xb2 , 0x38},                                  
	{0xbd , 0x38},                                  
	{0xbe , 0x36},                                  
	{0xd0 , 0xc9},                                  
	{0xd1 , 0x10},                                  
	//{0xd2 , 0x90},                                
	{0xd3 , 0x80},                                  
	{0xd5 , 0xf2},                                  
	{0xd6 , 0x16},                                  
	{0xdb , 0x92},                                  
	{0xdc , 0xa5},                                  
	{0xdf , 0x23},                                  
	{0xd9 , 0x00},                                  
	{0xda , 0x00},                                  
	{0xe0 , 0x09},                                  
                               
	{0xed , 0x04},                                  
	{0xee , 0xa0},                                  
	{0xef , 0x40},                                  
	{0x80 , 0x03},                                  
	{0x80 , 0x03},                                  
	{0x9F , 0x10},                                  
	{0xA0 , 0x20},                                  
	{0xA1 , 0x38},                                  
	{0xA2 , 0x4E},                                  
	{0xA3 , 0x63},                                  
	{0xA4 , 0x76},                                  
	{0xA5 , 0x87},                                  
	{0xA6 , 0xA2},                                  
	{0xA7 , 0xB8},                                  
	{0xA8 , 0xCA},                                  
	{0xA9 , 0xD8},                                  
	{0xAA , 0xE3},                                  
	{0xAB , 0xEB},                                  
	{0xAC , 0xF0},                                  
	{0xAD , 0xF8},                                  
	{0xAE , 0xFD},                                  
	{0xAF , 0xFF},                                  

//-----------GAMMA Select End--------------//
	{0xc0 , 0x00},                                  
	{0xc1 , 0x10},                                  
	{0xc2 , 0x1C},                                  
	{0xc3 , 0x30},                                  
	{0xc4 , 0x43},                                  
	{0xc5 , 0x54},                                  
	{0xc6 , 0x65},                                  
	{0xc7 , 0x75},                                  
	{0xc8 , 0x93},                                  
	{0xc9 , 0xB0},                                  
	{0xca , 0xCB},                                  
	{0xcb , 0xE6},                                  
	{0xcc , 0xFF},                                  
	{0xf0 , 0x02},                                  
	{0xf1 , 0x01},                                  
	{0xf2 , 0x01},                                  
	{0xf3 , 0x30},                                  
	{0xf9 , 0x9f},                                  
	{0xfa , 0x78},                                  
                                                                   
	//---------------------------------------------------------------
	{0xfe , 0x01},// set page1                                            
                                                                   
	{0x00 , 0xf5},                                  
	{0x02 , 0x1a},                                  
	{0x0a , 0xa0},                                  
	{0x0b , 0x60},                                  
	{0x0c , 0x08},                                  
	{0x0e , 0x4c},                                  
	{0x0f , 0x39},                                  
	{0x11 , 0x3f},                                  
	{0x12 , 0x72},                                  
	{0x13 , 0x13},                                  
	{0x14 , 0x42},                                  
	{0x15 , 0x43},                                  
	{0x16 , 0xc2},                                  
	{0x17 , 0xa8},                                  
	{0x18 , 0x18},                                  
	{0x19 , 0x40},                                  
	{0x1a , 0xd0},                                  
	{0x1b , 0xf5},                                  
	{0x70 , 0x40},                                  
	{0x71 , 0x58},                                  
	{0x72 , 0x30},                                  
	{0x73 , 0x48},                                  
	{0x74 , 0x20},                                  
	{0x75 , 0x60},                                  
	{0x77 , 0x20},                                  
	{0x78 , 0x32},                                  
	{0x30 , 0x03},                                  
	{0x31 , 0x40},                                  
	{0x32 , 0xe0},                                  
	{0x33 , 0xe0},                                  
	{0x34 , 0xe0},                                  
	{0x35 , 0xb0},                                  
	{0x36 , 0xc0},                                  
	{0x37 , 0xc0},                                  
	{0x38 , 0x04},                                  
	{0x39 , 0x09},                                  
	{0x3a , 0x12},                                  
	{0x3b , 0x1C},                                  
	{0x3c , 0x28},                                  
	{0x3d , 0x31},                                  
	{0x3e , 0x44},                                  
	{0x3f , 0x57},                                  
	{0x40 , 0x6C},                                  
	{0x41 , 0x81},                                  
	{0x42 , 0x94},                                  
	{0x43 , 0xA7},                                  
	{0x44 , 0xB8},                                  
	{0x45 , 0xD6},                                  
	{0x46 , 0xEE},                                  
	{0x47 , 0x0d},                                  

	//-----------Update the registers end---------//


	{0xfe , 0x00}, // set page0
	{0xd2 , 0x90},	
#endif
#if 0
         {0xfe , 0x80},  	
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
		
	{0xfe , 0x00},   // set page0
	
	{0xd2 , 0x10},   // close AEC
	{0x22 , 0x55},   // close AWB

	{0x03 , 0x01},                                  
	{0x04 , 0x2c},                                  
	{0x5a , 0x56},
	{0x5b , 0x40},
	{0x5c , 0x4a},			

	{0x22 , 0x57},   // Open AWB

    {0x28 , 0x11},// cyrille

	{0x01 , 0xfa},                                    
	{0x02 , 0x70},                                  
	{0x0f , 0x01},                                  
                                                                   
                                                                   
	{0xe2 , 0x00},   //anti-flicker step [11:8]     
	{0xe3 , 0x64},   //anti-flicker step [7:0]      
		                                                               
	{0xe4 , 0x02},   //exp level 1  16.67fps        
	{0xe5 , 0x58},                                  
	{0xe6 , 0x02},   //exp level 2  12.5fps         
	{0xe7 , 0x58},                                  
	{0xe8 , 0x02},   //exp level 3  8.33fps         
	{0xe9 , 0x58},                                  
	{0xea , 0x04},   //exp level 4  4.00fps         
	{0xeb , 0xb0},                                  

	{0xec , 0x20},
	   
	{0x05 , 0x00},                                  
	{0x06 , 0x00},                                  
	{0x07 , 0x00},                                  
	{0x08 , 0x00},                                  
	{0x09 , 0x01},                                  
	{0x0a , 0xe8},                                  
	{0x0b , 0x02},                                  
	{0x0c , 0x88},                                  
	{0x0d , 0x02},                                  
	{0x0e , 0x02},                                  
	{0x10 , 0x22},                                  
	{0x11 , 0xfd},                                  
	{0x12 , 0x2a},  //2a                                
	{0x13 , 0x00},                                  
	{0x14 , 0x10},  //0x10
	//-------------H_V_Switch(4)---------------//
	/*	1:  // normal
	    		{0x14 , 0x10},    		
		2:  // IMAGE_H_MIRROR
	    		{0x14 , 0x11},
	    		
		3:  // IMAGE_V_MIRROR
	    		{0x14 , 0x12},
	    		
		4:  // IMAGE_HV_MIRROR
	    		{0x14 , 0x13},*/			                            
	{0x15 , 0x0a},                                  
	{0x16 , 0x05},                                  
	{0x17 , 0x01},                                  
	{0x18 , 0x44},                                  
	{0x19 , 0x44},                                  
	{0x1a , 0x1e},                                  
	{0x1b , 0x00},                                  
	{0x1c , 0xc1},                                  
	{0x1d , 0x08},                                  
	{0x1e , 0x60},                                  
	{0x1f , 0x13},                                  
                                                                   
	                                                                 
	{0x20 , 0xff},                                  
	{0x21 , 0xf8},                                  
	{0x22 , 0x57},                                  
	{0x24 , 0xa3},                                  
	{0x25 , 0x0f},                                  
	                                                                 
	//output sync_mode                                               
	{0x26 , 0x03},   //0x03  20101016 zhj                                 
	{0x2f , 0x01},                                  
	{0x30 , 0xf7},                                  
	{0x31 , 0x50},
	{0x32 , 0x00},
	{0x39 , 0x04},
	{0x3a , 0x18},
	{0x3b , 0x20},                                  
	{0x3c , 0x00},                                  
	{0x3d , 0x00},                                  
	{0x3e , 0x00},                                  
	{0x3f , 0x00},                                  
	{0x50 , 0x10},                                  
	{0x53 , 0x82},                                  
	{0x54 , 0x80},                                  
	{0x55 , 0x80},                                  
	{0x56 , 0x82},                                  
	{0x8b , 0x40},                                  
	{0x8c , 0x40},                                  
	{0x8d , 0x40},                                  
	{0x8e , 0x2e},                                  
	{0x8f , 0x2e},                                  
	{0x90 , 0x2e},                                  
	{0x91 , 0x3c},                                  
	{0x92 , 0x50},                                  
	{0x5d , 0x12},                                  
	{0x5e , 0x1a},                                  
	{0x5f , 0x24},                                  
	{0x60 , 0x07},                                  
	{0x61 , 0x15},                                  
	{0x62 , 0x08},                                  
	{0x64 , 0x03},                                  
	{0x66 , 0xe8},                                  
	{0x67 , 0x86},                                  
	{0x68 , 0xa2},                                  
	{0x69 , 0x18},                                  
	{0x6a , 0x0f},                                  
	{0x6b , 0x00},                                  
	{0x6c , 0x5f},                                  
	{0x6d , 0x8f},             {0xfe , 0x80},  	
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
    {0xfe , 0x80},  
		
	{0xfe , 0x00},   // set page0
	
	{0xd2 , 0x10},   // close AEC
	{0x22 , 0x55},   // close AWB

	{0x03 , 0x01},                                  
	{0x04 , 0x2c},                                  
	{0x5a , 0x56},
	{0x5b , 0x40},
	{0x5c , 0x4a},			

	{0x22 , 0x57},   // Open AWB

{0x28 , 0x11},

	{0x01 , 0xfa},                                    
	{0x02 , 0x70},                                  
	{0x0f , 0x01},                                  
                                                                   
                                                                   
	{0xe2 , 0x00},   //anti-flicker step [11:8]     
	{0xe3 , 0x64},   //anti-flicker step [7:0]      
		                                                               
	{0xe4 , 0x02},   //exp level 1  16.67fps        
	{0xe5 , 0x58},                                  
	{0xe6 , 0x02},   //exp level 2  12.5fps         
	{0xe7 , 0x58},                                  
	{0xe8 , 0x02},   //exp level 3  8.33fps         
	{0xe9 , 0x58},                                  
	{0xea , 0x04},   //exp level 4  4.00fps         
	{0xeb , 0xb0},                                  

	{0xec , 0x20},
	   
	{0x05 , 0x00},                                  
	{0x06 , 0x00},                                  
	{0x07 , 0x00},                                  
	{0x08 , 0x00},                                  
	{0x09 , 0x01},                                  
	{0x0a , 0xe8},                                  
	{0x0b , 0x02},                                  
	{0x0c , 0x88},                                  
	{0x0d , 0x02},                                  
	{0x0e , 0x02},                                  
	{0x10 , 0x22},                                  
	{0x11 , 0xfd},                                  
	{0x12 , 0x2a},  //2a                                
	{0x13 , 0x00},                                  
	{0x14 , 0x10},  //0x10
	//-------------H_V_Switch(4)---------------//
	/*	1:  // normal
	    		{0x14 , 0x10},    		
		2:  // IMAGE_H_MIRROR
	    		{0x14 , 0x11},
	    		
		3:  // IMAGE_V_MIRROR
	    		{0x14 , 0x12},
	    		
		4:  // IMAGE_HV_MIRROR
	    		{0x14 , 0x13},*/			                            
	{0x15 , 0x0a},                                  
	{0x16 , 0x05},                                  
	{0x17 , 0x01},                                  
	{0x18 , 0x44},                                  
	{0x19 , 0x44},                                  
	{0x1a , 0x1e},                                  
	{0x1b , 0x00},                                  
	{0x1c , 0xc1},                                  
	{0x1d , 0x08},                                  
	{0x1e , 0x60},                                  
	{0x1f , 0x13},                                  
                                                                   
	                                                                 
	{0x20 , 0xff},                                  
	{0x21 , 0xf8},                                  
	{0x22 , 0x57},                                  
	{0x24 , 0xa3},                                  
	{0x25 , 0x0f},                                  
	                                                                 
	//output sync_mode                                               
	{0x26 , 0x03},   //0x03  20101016 zhj                                 
	{0x2f , 0x01},                                  
	{0x30 , 0xf7},                                  
	{0x31 , 0x50},
	{0x32 , 0x00},
	{0x39 , 0x04},
	{0x3a , 0x18},
	{0x3b , 0x20},                                  
	{0x3c , 0x00},                                  
	{0x3d , 0x00},                                  
	{0x3e , 0x00},                                  
	{0x3f , 0x00},                                  
	{0x50 , 0x10},                                  
	{0x53 , 0x82},                                  
	{0x54 , 0x80},                                  
	{0x55 , 0x80},                                  
	{0x56 , 0x82},                                  
	{0x8b , 0x40},                                  
	{0x8c , 0x40},                                  
	{0x8d , 0x40},                                  
	{0x8e , 0x2e},                                  
	{0x8f , 0x2e},                                  
	{0x90 , 0x2e},                                  
	{0x91 , 0x3c},                                  
	{0x92 , 0x50},                                  
	{0x5d , 0x12},                                  
	{0x5e , 0x1a},                                  
	{0x5f , 0x24},                                  
	{0x60 , 0x07},                                  
	{0x61 , 0x15},                                  
	{0x62 , 0x08},                                  
	{0x64 , 0x03},                                  
	{0x66 , 0xe8},                                  
	{0x67 , 0x86},                                  
	{0x68 , 0xa2},                                  
	{0x69 , 0x18},                                  
	{0x6a , 0x0f},                                  
	{0x6b , 0x00},                                  
	{0x6c , 0x5f},                                  
	{0x6d , 0x8f},                                  
	{0x6e , 0x55},                                  
	{0x6f , 0x38},                                  
	{0x70 , 0x15},                                  
	{0x71 , 0x33},                                  
	{0x72 , 0xdc},                                  
	{0x73 , 0x80},                                  
	{0x74 , 0x02},                                  
	{0x75 , 0x3f},                                  
	{0x76 , 0x02},                                  
	{0x77 , 0x54},                                  
	{0x78 , 0x88},                                  
	{0x79 , 0x81},                                  
	{0x7a , 0x81},                                  
	{0x7b , 0x22},                                  
	{0x7c , 0xff},                                  
	{0x93 , 0x48},                                  
	{0x94 , 0x00},                                  
	{0x95 , 0x05},                                  
	{0x96 , 0xe8},                                  
	{0x97 , 0x40},                                  
	{0x98 , 0xf0},                                  
	{0xb1 , 0x38},                                  
	{0xb2 , 0x38},                                  
	{0xbd , 0x38},                                  
	{0xbe , 0x36},                                  
	{0xd0 , 0xc9},                                  
	{0xd1 , 0x10},                                  
	//{0xd2 , 0x90},                                
	{0xd3 , 0x80},                                  
	{0xd5 , 0xf2},                                  
	{0xd6 , 0x16},                                  
	{0xdb , 0x92},                                  
	{0xdc , 0xa5},                                  
	{0xdf , 0x23},                                  
	{0xd9 , 0x00},                                  
	{0xda , 0x00},                                  
	{0xe0 , 0x09},                                  
                               
	{0xed , 0x04},                                  
	{0xee , 0xa0},                                  
	{0xef , 0x40},                                  
	{0x80 , 0x03},                                  
	{0x80 , 0x03},                                  
	{0x9F , 0x10},                                  
	{0xA0 , 0x20},                                  
	{0xA1 , 0x38},                                  
	{0xA2 , 0x4E},                                  
	{0xA3 , 0x63},                                  
	{0xA4 , 0x76},                                  
	{0xA5 , 0x87},                                  
	{0xA6 , 0xA2},                                  
	{0xA7 , 0xB8},                                  
	{0xA8 , 0xCA},                                  
	{0xA9 , 0xD8},                                  
	{0xAA , 0xE3},                                  
	{0xAB , 0xEB},                                  
	{0xAC , 0xF0},                                  
	{0xAD , 0xF8},                                  
	{0xAE , 0xFD},                                  
	{0xAF , 0xFF},                                  

//-----------GAMMA Select End--------------//
	{0xc0 , 0x00},                                  
	{0xc1 , 0x10},                                  
	{0xc2 , 0x1C},                                  
	{0xc3 , 0x30},                                  
	{0xc4 , 0x43},                                  
	{0xc5 , 0x54},                                  
	{0xc6 , 0x65},                                  
	{0xc7 , 0x75},                                  
	{0xc8 , 0x93},                                  
	{0xc9 , 0xB0},                                  
	{0xca , 0xCB},                                  
	{0xcb , 0xE6},                                  
	{0xcc , 0xFF},                                  
	{0xf0 , 0x02},                                  
	{0xf1 , 0x01},                                  
	{0xf2 , 0x01},                                  
	{0xf3 , 0x30},                                  
	{0xf9 , 0x9f},                                  
	{0xfa , 0x78},                                  
                                                                   
	//---------------------------------------------------------------
	{0xfe , 0x01},// set page1                                            
                                                                   
	{0x00 , 0xf5},                                  
	{0x02 , 0x1a},                                  
	{0x0a , 0xa0},                                  
	{0x0b , 0x60},                                  
	{0x0c , 0x08},                                  
	{0x0e , 0x4c},                                  
	{0x0f , 0x39},                                  
	{0x11 , 0x3f},                                  
	{0x12 , 0x72},                                  
	{0x13 , 0x13},                                  
	{0x14 , 0x42},                                  
	{0x15 , 0x43},                                  
	{0x16 , 0xc2},                                  
	{0x17 , 0xa8},                                  
	{0x18 , 0x18},                                  
	{0x19 , 0x40},                                  
	{0x1a , 0xd0},                                  
	{0x1b , 0xf5},                                  
	{0x70 , 0x40},                                  
	{0x71 , 0x58},                                  
	{0x72 , 0x30},                                  
	{0x73 , 0x48},                                  
	{0x74 , 0x20},                                  
	{0x75 , 0x60},                                  
	{0x77 , 0x20},                                  
	{0x78 , 0x32},                                  
	{0x30 , 0x03},                                  
	{0x31 , 0x40},                                  
	{0x32 , 0xe0},                                  
	{0x33 , 0xe0},                                  
	{0x34 , 0xe0},                                  
	{0x35 , 0xb0},                                  
	{0x36 , 0xc0},                                  
	{0x37 , 0xc0},                                  
	{0x38 , 0x04},                                  
	{0x39 , 0x09},                                  
	{0x3a , 0x12},                                  
	{0x3b , 0x1C},                                  
	{0x3c , 0x28},                                  
	{0x3d , 0x31},                                  
	{0x3e , 0x44},                                  
	{0x3f , 0x57},                                  
	{0x40 , 0x6C},                                  
	{0x41 , 0x81},                                  
	{0x42 , 0x94},                                  
	{0x43 , 0xA7},                                  
	{0x44 , 0xB8},                                  
	{0x45 , 0xD6},                                  
	{0x46 , 0xEE},                                  
	{0x47 , 0x0d},                                  

	//-----------Update the registers end---------//


	{0xfe , 0x00}, // set page0
	{0xd2 , 0x90},	                      
	{0x6e , 0x55},                                  
	{0x6f , 0x38},                                  
	{0x70 , 0x15},                                  
	{0x71 , 0x33},                                  
	{0x72 , 0xdc},                                  
	{0x73 , 0x80},                                  
	{0x74 , 0x02},                                  
	{0x75 , 0x3f},                                  
	{0x76 , 0x02},                                  
	{0x77 , 0x54},                                  
	{0x78 , 0x88},                                  
	{0x79 , 0x81},                                  
	{0x7a , 0x81},                                  
	{0x7b , 0x22},                                  
	{0x7c , 0xff},                                  
	{0x93 , 0x48},                                  
	{0x94 , 0x00},                                  
	{0x95 , 0x05},                                  
	{0x96 , 0xe8},                                  
	{0x97 , 0x40},                                  
	{0x98 , 0xf0},                                  
	{0xb1 , 0x38},                                  
	{0xb2 , 0x38},                                  
	{0xbd , 0x38},                                  
	{0xbe , 0x36},                                  
	{0xd0 , 0xc9},                                  
	{0xd1 , 0x10},                                  
	//{0xd2 , 0x90},                                
	{0xd3 , 0x80},                                  
	{0xd5 , 0xf2},                                  
	{0xd6 , 0x16},                                  
	{0xdb , 0x92},                                  
	{0xdc , 0xa5},                                  
	{0xdf , 0x23},                                  
	{0xd9 , 0x00},                                  
	{0xda , 0x00},                                  
	{0xe0 , 0x09},                                  
                               
	{0xed , 0x04},                                  
	{0xee , 0xa0},                                  
	{0xef , 0x40},                                  
	{0x80 , 0x03},                                  
	{0x80 , 0x03},                                  
	{0x9F , 0x10},                                  
	{0xA0 , 0x20},                                  
	{0xA1 , 0x38},                                  
	{0xA2 , 0x4E},                                  
	{0xA3 , 0x63},                                  
	{0xA4 , 0x76},                                  
	{0xA5 , 0x87},                                  
	{0xA6 , 0xA2},                                  
	{0xA7 , 0xB8},                                  
	{0xA8 , 0xCA},                                  
	{0xA9 , 0xD8},                                  
	{0xAA , 0xE3},                                  
	{0xAB , 0xEB},                                  
	{0xAC , 0xF0},                                  
	{0xAD , 0xF8},                                  
	{0xAE , 0xFD},                                  
	{0xAF , 0xFF},                                  

//-----------GAMMA Select End--------------//
	{0xc0 , 0x00},                                  
	{0xc1 , 0x10},                                  
	{0xc2 , 0x1C},                                  
	{0xc3 , 0x30},                                  
	{0xc4 , 0x43},                                  
	{0xc5 , 0x54},                                  
	{0xc6 , 0x65},                                  
	{0xc7 , 0x75},                                  
	{0xc8 , 0x93},                                  
	{0xc9 , 0xB0},                                  
	{0xca , 0xCB},                                  
	{0xcb , 0xE6},                                  
	{0xcc , 0xFF},                                  
	{0xf0 , 0x02},                                  
	{0xf1 , 0x01},                                  
	{0xf2 , 0x01},                                  
	{0xf3 , 0x30},                                  
	{0xf9 , 0x9f},                                  
	{0xfa , 0x78},                                  
                                                                   
	//---------------------------------------------------------------
	{0xfe , 0x01},// set page1                                            
                                                                   
	{0x00 , 0xf5},                                  
	{0x02 , 0x1a},                                  
	{0x0a , 0xa0},                                  
	{0x0b , 0x60},                                  
	{0x0c , 0x08},                                  
	{0x0e , 0x4c},                                  
	{0x0f , 0x39},                                  
	{0x11 , 0x3f},                                  
	{0x12 , 0x72},                                  
	{0x13 , 0x13},                                  
	{0x14 , 0x42},                                  
	{0x15 , 0x43},                                  
	{0x16 , 0xc2},                                  
	{0x17 , 0xa8},                                  
	{0x18 , 0x18},                                  
	{0x19 , 0x40},                                  
	{0x1a , 0xd0},                                  
	{0x1b , 0xf5},                                  
	{0x70 , 0x40},                                  
	{0x71 , 0x58},                                  
	{0x72 , 0x30},                                  
	{0x73 , 0x48},                                  
	{0x74 , 0x20},                                  
	{0x75 , 0x60},                                  
	{0x77 , 0x20},                                  
	{0x78 , 0x32},                                  
	{0x30 , 0x03},                                  
	{0x31 , 0x40},                                  
	{0x32 , 0xe0},                                  
	{0x33 , 0xe0},                                  
	{0x34 , 0xe0},                                  
	{0x35 , 0xb0},                                  
	{0x36 , 0xc0},                                  
	{0x37 , 0xc0},                                  
	{0x38 , 0x04},                                  
	{0x39 , 0x09},                                  
	{0x3a , 0x12},                                  
	{0x3b , 0x1C},                                  
	{0x3c , 0x28},                                  
	{0x3d , 0x31},                                  
	{0x3e , 0x44},                                  
	{0x3f , 0x57},                                  
	{0x40 , 0x6C},                                  
	{0x41 , 0x81},                                  
	{0x42 , 0x94},                                  
	{0x43 , 0xA7},                                  
	{0x44 , 0xB8},                                  
	{0x45 , 0xD6},                                  
	{0x46 , 0xEE},                                  
	{0x47 , 0x0d},                                  

	//-----------Update the registers end---------//


	{0xfe , 0x00}, // set page0
	{0xd2 , 0x90},	
    {0x14 , 0x10},
        	{0x14 , 0x10},
            	{0x14 , 0x10},
    
    #endif
    {0xfe , 0x80},		
			
		{0xfe , 0x00},	 // set page0
		{0x28 , 0x11},
		{0xd2 , 0x10},	 // close AEC
		{0x22 , 0x55},	 // close AWB
	
		{0x03 , 0x01},									
		{0x04 , 0x2c},									
		{0x5a , 0x56},
		{0x5b , 0x40},
		{0x5c , 0x4a},			
	
		{0x22 , 0x57},	 // Open AWB
	
		{0x01 , 0x32},									  
		{0x02 , 0x70},									
		{0x0f , 0x01},									
																	   
																	   
		{0xe2 , 0x00},	 //anti-flicker step [11:8] 	
		{0xe3 , 0x3c},	 //anti-flicker step [7:0]		
																		   
		{0xe4 , 0x02},	 //exp level 1	16.67fps		
		{0xe5 , 0x58},									
		{0xe6 , 0x02},	 //exp level 2	12.5fps 		
		{0xe7 , 0x58},									
		{0xe8 , 0x02},	 //exp level 3	8.33fps 		
		{0xe9 , 0x58},									
		{0xea , 0x09},	 //exp level 4	4.00fps 		
		{0xeb , 0xc4},									
	
	    {0xec , 0x00},
		   
		{0x05 , 0x00},									
		{0x06 , 0x00},									
		{0x07 , 0x00},									
		{0x08 , 0x00},									
		{0x09 , 0x01},									
		{0x0a , 0xe8},									
		{0x0b , 0x02},									
		{0x0c , 0x88},									
		{0x0d , 0x02},									
		{0x0e , 0x02},									
		{0x10 , 0x22},									
		{0x11 , 0xfd},									
		{0x12 , 0x2a},									
		{0x13 , 0x00},									
		{0x14 , 0x10},	//0x10
		//-------------H_V_Switch(4)---------------//
		/*	1:	// normal
					{0x14 , 0x10},			
			2:	// IMAGE_H_MIRROR
					{0x14 , 0x11},
					
			3:	// IMAGE_V_MIRROR
					{0x14 , 0x12},
					
			4:	// IMAGE_HV_MIRROR
					{0x14 , 0x13},*/										
		{0x15 , 0x0a},									
		{0x16 , 0x05},									
		{0x17 , 0x01},									
		{0x18 , 0x44},									
		{0x19 , 0x44},									
		{0x1a , 0x1e},									
		{0x1b , 0x00},									
		{0x1c , 0xc1},									
		{0x1d , 0x08},									
		{0x1e , 0x60},									
		{0x1f , 0x13},									
																	   
																		 
		{0x20 , 0xff},									
		{0x21 , 0xf8},									
		{0x22 , 0x57},									
		{0x24 , 0xa3},									
		{0x25 , 0x0f},									
																		 
		//output sync_mode												 
		{0x26 , 0x07},	 //0x03  20101016 zhj								  
		{0x2f , 0x01},									
		{0x30 , 0xf7},									
		{0x31 , 0x50},
		{0x32 , 0x00},
		{0x39 , 0x04},
		{0x3a , 0x18},
		{0x3b , 0x20},									
		{0x3c , 0x00},									
		{0x3d , 0x00},									
		{0x3e , 0x00},									
		{0x3f , 0x00},									
		{0x50 , 0x10},									
		{0x53 , 0x82},									
		{0x54 , 0x80},									
		{0x55 , 0x80},									
		{0x56 , 0x82},									
		{0x8b , 0x40},									
		{0x8c , 0x40},									
		{0x8d , 0x40},									
		{0x8e , 0x2e},									
		{0x8f , 0x2e},									
		{0x90 , 0x2e},									
		{0x91 , 0x3c},									
		{0x92 , 0x50},									
		{0x5d , 0x12},									
		{0x5e , 0x1a},									
		{0x5f , 0x24},									
		{0x60 , 0x07},									
		{0x61 , 0x15},									
		{0x62 , 0x08},									
		{0x64 , 0x03},									
		{0x66 , 0xe8},									
		{0x67 , 0x86},									
		{0x68 , 0xa2},									
		{0x69 , 0x18},									
		{0x6a , 0x0f},									
		{0x6b , 0x00},									
		{0x6c , 0x5f},									
		{0x6d , 0x8f},									
		{0x6e , 0x55},									
		{0x6f , 0x38},									
		{0x70 , 0x15},									
		{0x71 , 0x33},									
		{0x72 , 0xdc},									
		{0x73 , 0x80},									
		{0x74 , 0x02},									
		{0x75 , 0x3f},									
		{0x76 , 0x02},									
	{0x77 , 0x54},                                  
		{0x78 , 0x88},									
		{0x79 , 0x81},									
		{0x7a , 0x81},									
		{0x7b , 0x22},									
		{0x7c , 0xff},									
		{0x93 , 0x48},									
		{0x94 , 0x00},									
		{0x95 , 0x05},									
		{0x96 , 0xe8},									
		{0x97 , 0x40},									
		{0x98 , 0xf0},									
		{0xb1 , 0x38},									
		{0xb2 , 0x38},									
		{0xbd , 0x38},									
		{0xbe , 0x36},									
		{0xd0 , 0xc9},									
		{0xd1 , 0x10},									
		//{0xd2 , 0x90},								
		{0xd3 , 0x80},									
		{0xd5 , 0xf2},									
		{0xd6 , 0x16},									
		{0xdb , 0x92},									
		{0xdc , 0xa5},									
		{0xdf , 0x23},									
		{0xd9 , 0x00},									
		{0xda , 0x00},									
		{0xe0 , 0x09},									
								   
		{0xed , 0x04},									
		{0xee , 0xa0},									
		{0xef , 0x40},									
		{0x80 , 0x03},									
		{0x80 , 0x03},									
		{0x9F , 0x10},									
		{0xA0 , 0x20},									
		{0xA1 , 0x38},									
		{0xA2 , 0x4E},									
		{0xA3 , 0x63},									
		{0xA4 , 0x76},									
		{0xA5 , 0x87},									
		{0xA6 , 0xA2},									
		{0xA7 , 0xB8},									
		{0xA8 , 0xCA},									
		{0xA9 , 0xD8},									
		{0xAA , 0xE3},									
		{0xAB , 0xEB},									
		{0xAC , 0xF0},									
		{0xAD , 0xF8},									
		{0xAE , 0xFD},									
		{0xAF , 0xFF},									
		 /*GC0308_GAMMA_Select,
			1:											   //smallest gamma curve
				{0x9F , 0x0B},
				{0xA0 , 0x16},
				{0xA1 , 0x29},
				{0xA2 , 0x3C},
				{0xA3 , 0x4F},
				{0xA4 , 0x5F},
				{0xA5 , 0x6F},
				{0xA6 , 0x8A},
				{0xA7 , 0x9F},
				{0xA8 , 0xB4}, 
				{0xA9 , 0xC6},
				{0xAA , 0xD3},
				{0xAB , 0xDD},
				{0xAC , 0xE5},
				{0xAD , 0xF1},
				{0xAE , 0xFA},
				{0xAF , 0xFF},	
				
			2:			
				{0x9F , 0x0E},
				{0xA0 , 0x1C},
				{0xA1 , 0x34},
				{0xA2 , 0x48},
				{0xA3 , 0x5A},
				{0xA4 , 0x6B},
				{0xA5 , 0x7B},
				{0xA6 , 0x95},
				{0xA7 , 0xAB},
				{0xA8 , 0xBF},
				{0xA9 , 0xCE},
				{0xAA , 0xD9},
				{0xAB , 0xE4},
				{0xAC , 0xEC},
				{0xAD , 0xF7},
				{0xAE , 0xFD},
				{0xAF , 0xFF},
				
			3:
				{0x9F , 0x10},
				{0xA0 , 0x20},
				{0xA1 , 0x38},
				{0xA2 , 0x4E},
				{0xA3 , 0x63},
				{0xA4 , 0x76},
				{0xA5 , 0x87},
				{0xA6 , 0xA2},
				{0xA7 , 0xB8},
				{0xA8 , 0xCA},
				{0xA9 , 0xD8},
				{0xAA , 0xE3},
				{0xAB , 0xEB},
				{0xAC , 0xF0},
				{0xAD , 0xF8},
				{0xAE , 0xFD},
				{0xAF , 0xFF},
	
			4:
				{0x9F , 0x14},
				{0xA0 , 0x28},
				{0xA1 , 0x44},
				{0xA2 , 0x5D},
				{0xA3 , 0x72},
				{0xA4 , 0x86},
				{0xA5 , 0x95},
				{0xA6 , 0xB1},
				{0xA7 , 0xC6},
				{0xA8 , 0xD5},
				{0xA9 , 0xE1},
				{0xAA , 0xEA},
				{0xAB , 0xF1},
				{0xAC , 0xF5},
				{0xAD , 0xFB},
				{0xAE , 0xFE},
				{0xAF , 0xFF},
				
			5:								//largest gamma curve
				{0x9F , 0x15},
				{0xA0 , 0x2A},
				{0xA1 , 0x4A},
				{0xA2 , 0x67},
				{0xA3 , 0x79},
				{0xA4 , 0x8C},
				{0xA5 , 0x9A},
				{0xA6 , 0xB3},
				{0xA7 , 0xC5},
				{0xA8 , 0xD5},
				{0xA9 , 0xDF},
				{0xAA , 0xE8},
				{0xAB , 0xEE},
				{0xAC , 0xF3},
				{0xAD , 0xFA},
				{0xAE , 0xFD},
				{0xAF , 0xFF}, */
		//-----------GAMMA Select End--------------//
									
		{0xc0 , 0x00},									
		{0xc1 , 0x10},									
		{0xc2 , 0x1C},									
		{0xc3 , 0x30},									
		{0xc4 , 0x43},									
		{0xc5 , 0x54},									
		{0xc6 , 0x65},									
		{0xc7 , 0x75},									
		{0xc8 , 0x93},									
		{0xc9 , 0xB0},									
		{0xca , 0xCB},									
		{0xcb , 0xE6},									
		{0xcc , 0xFF},									
		{0xf0 , 0x02},									
		{0xf1 , 0x01},									
		{0xf2 , 0x01},									
		{0xf3 , 0x30},									
		{0xf9 , 0x9f},									
		{0xfa , 0x78},									
																	   
		//---------------------------------------------------------------
		{0xfe , 0x01},// set page1											  
																	   
		{0x00 , 0xf5},									
		{0x02 , 0x1a},									
		{0x0a , 0xa0},									
		{0x0b , 0x60},									
		{0x0c , 0x08},									
		{0x0e , 0x4c},									
		{0x0f , 0x39},									
		{0x11 , 0x3f},									
		{0x12 , 0x72},									
		{0x13 , 0x13},									
		{0x14 , 0x42},									
		{0x15 , 0x43},									
		{0x16 , 0xc2},									
		{0x17 , 0xa8},									
		{0x18 , 0x18},									
		{0x19 , 0x40},									
		{0x1a , 0xd0},									
		{0x1b , 0xf5},									
		{0x70 , 0x40},									
		{0x71 , 0x58},									
		{0x72 , 0x30},									
		{0x73 , 0x48},									
		{0x74 , 0x20},									
		{0x75 , 0x60},									
		{0x77 , 0x20},									
		{0x78 , 0x32},									
		{0x30 , 0x03},									
		{0x31 , 0x40},									
		{0x32 , 0xe0},									
		{0x33 , 0xe0},									
		{0x34 , 0xe0},									
		{0x35 , 0xb0},									
		{0x36 , 0xc0},									
		{0x37 , 0xc0},									
		{0x38 , 0x04},									
		{0x39 , 0x09},									
		{0x3a , 0x12},									
		{0x3b , 0x1C},									
		{0x3c , 0x28},									
		{0x3d , 0x31},									
		{0x3e , 0x44},									
		{0x3f , 0x57},									
		{0x40 , 0x6C},									
		{0x41 , 0x81},									
		{0x42 , 0x94},									
		{0x43 , 0xA7},									
		{0x44 , 0xB8},									
		{0x45 , 0xD6},									
		{0x46 , 0xEE},									
		{0x47 , 0x0d},									
		{0xfe , 0x00}, // set page0
		 
		//-----------Update the registers 2010/07/06-------------//
		//Registers of Page0
		{0xfe , 0x00}, // set page0
		{0x10 , 0x26},								   
		{0x11 , 0x0d},	// fd,modified by mormo 2010/07/06								 
		{0x1a , 0x2a},	// 1e,modified by mormo 2010/07/06									
	
		{0x1c , 0x49}, // c1,modified by mormo 2010/07/06								  
		{0x1d , 0x9a}, // 08,modified by mormo 2010/07/06								  
		{0x1e , 0x61}, // 60,modified by mormo 2010/07/06								  
	
		{0x3a , 0x20},
	
		{0x50 , 0x14},	// 10,modified by mormo 2010/07/06								 
		{0x53 , 0x80},									
		{0x56 , 0x80},
		
		{0x8b , 0x20}, //LSC								 
		{0x8c , 0x20},									
		{0x8d , 0x20},									
		{0x8e , 0x14},									
		{0x8f , 0x10},									
		{0x90 , 0x14},									
	
		{0x94 , 0x02},									
		{0x95 , 0x07},									
		{0x96 , 0xe0},									
	
		{0xb1 , 0x40}, // YCPT								   
		{0xb2 , 0x40},									
		{0xb3 , 0x40},
		{0xb6 , 0xe0},
	
		{0xd0 , 0xcb}, // AECT	c9,modifed by mormo 2010/07/06								  
		{0xd3 , 0x48}, // 80,modified by mormor 2010/07/06							 
	
		{0xf2 , 0x02},									
		{0xf7 , 0x12},
		{0xf8 , 0x0a},
	
		//Registers of Page1
		{0xfe , 0x01},// set page1	  
		{0x02 , 0x20},
		{0x04 , 0x10},
		{0x05 , 0x08},
		{0x06 , 0x20},
		{0x08 , 0x0a},
	
		{0x0e , 0x44},									
		{0x0f , 0x32},
		{0x10 , 0x41},									
		{0x11 , 0x37},									
		{0x12 , 0x22},									
		{0x13 , 0x19},									
		{0x14 , 0x44},									
		{0x15 , 0x44},	
		
		{0x19 , 0x50},									
		{0x1a , 0xd8}, 
		
		{0x32 , 0x10}, 
		
		{0x35 , 0x00},									
		{0x36 , 0x80},									
		{0x37 , 0x00}, 
		//-----------Update the registers end---------//
	
	
		{0xfe , 0x00}, // set page0
		{0xd2 , 0x90},	
    ENDMARKER,
};

static struct regval_list gc0308_enable_regs[] = {
    ENDMARKER,
};

static struct regval_list gc0308_disable_regs[] = {
    ENDMARKER,
};

/*
 * color code
 */
static struct regval_list gc0308_fmt_yuv422_yuyv[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_fmt_yuv422_yvyu[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_fmt_yuv422_vyuy[] =
{
    ENDMARKER,
};

static struct regval_list gc0308_fmt_yuv422_uyvy[] =
{

    ENDMARKER,
};

/* static struct regval_list gc0308_fmt_raw[] = */
/* { */
/*     {0xfd,0x00}, */
/*     ENDMARKER, */
/* }; */

/*
 *AWB
 */
static const struct regval_list gc0308_awb_regs_enable[] =
{

    ENDMARKER,
};
static const struct regval_list gc0308_awb_regs_diable[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_wb_cloud_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_wb_daylight_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_wb_incandescence_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_wb_fluorescent_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_wb_tungsten_regs[] =
{

    ENDMARKER,
};

/*
 * colorfx
 */
static struct regval_list gc0308_colorfx_none_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_colorfx_bw_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_colorfx_sepia_regs[] =
{

    ENDMARKER,
};

static struct regval_list gc0308_colorfx_negative_regs[] =
{

    ENDMARKER,
};

/*
 * window size
 */
static const struct regval_list gc0308_vga_regs[] = {


    ENDMARKER,
};

static const struct regval_list gc0308_qvga_regs[] = {


    ENDMARKER,
};

/********************************************************************************
 * structures
 */
struct gc0308_win_size {
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

struct gc0308_priv {
    struct v4l2_subdev                  subdev;
    struct media_pad                    pad;
    struct v4l2_ctrl_handler            hdl;
    const struct gc0308_color_format    *cfmt;
    const struct gc0308_win_size        *win;
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

struct gc0308_color_format {
    enum v4l2_mbus_pixelcode code;
    enum v4l2_colorspace colorspace;
};

/********************************************************************************
 * tables
 */
static const struct gc0308_color_format gc0308_cfmts[] = {
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
#define gc0308_MAX_WIDTH    VGA_WIDTH
#define gc0308_MAX_HEIGHT   VGA_HEIGHT
#define AHEAD_LINE_NUM      15    //10\D0\D0 = 50\B4\CEѭ\BB\B7(gc0308)
#define DROP_NUM_CAPTURE    3
#define DROP_NUM_PREVIEW    16


static unsigned int frame_rate_vga[] = {30,};

/* 640x480 */
static const struct gc0308_win_size gc0308_win_vga = {
    .name     = "VGA",
    .width    = VGA_WIDTH,
    .height   = VGA_HEIGHT,
    .win_regs = gc0308_vga_regs,
    .frame_rate_array = frame_rate_vga,
};

static const struct gc0308_win_size gc0308_win_qvga = {
    .name     = "QVGA",
    .width    = 320,
    .height   = 240,
    .win_regs = gc0308_qvga_regs,
    .frame_rate_array = frame_rate_vga,
};

static const struct gc0308_win_size *gc0308_win[] = {
    &gc0308_win_vga,
    &gc0308_win_qvga,
};

/********************************************************************************
 * general functions
 */
static inline struct gc0308_priv *to_priv(struct v4l2_subdev *subdev)
{
    return container_of(subdev, struct gc0308_priv, subdev);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct gc0308_priv, hdl)->subdev;
}

static bool check_id(struct i2c_client *client)
{
    u8 pid = i2c_smbus_read_byte_data(client, PID);
    printk(KERN_ERR "gc0308 id is: 0x%x\n", pid);
    if (pid == SENSOR_ID)
        return true;

    printk(KERN_ERR "failed to check id: 0x%x\n", pid);
    return false;
}
//#define DBG_IIC  // to show wether regisers are written correctly
static int gc0308_write_array(struct i2c_client *client, const struct regval_list *vals)
{
    int ret;
    while (vals->reg_num != 0xff) {
        ret = i2c_smbus_write_byte_data(client, vals->reg_num, vals->value);
        if (ret < 0)
            return ret;
       // mdelay(1);
#ifdef DBG_IIC
        printk("REG[%02x],DATA[%02x]\n",vals->reg_num,i2c_smbus_read_byte_data(client,vals->reg_num));
#endif 
        vals++;
    }
    return 0;
}

static int gc0308_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set) __attribute__((unused));
static int gc0308_mask_set(struct i2c_client *client, u8 command, u8 mask, u8 set)
{
    s32 val = i2c_smbus_read_byte_data(client, command);
    if (val < 0)
        return val;

    val &= ~mask;
    val |= set & mask;

    return i2c_smbus_write_byte_data(client, command, val);
}

static const struct gc0308_win_size *gc0308_select_win(u32 width, u32 height)
{
    const struct gc0308_win_size *win;
    int i;
    printk("%s .............\n",__func__);
    for (i = 0; i < ARRAY_SIZE(gc0308_win); i++) {
        win = gc0308_win[i];
        if (width == win->width && height == win->height)
            return win;
    }

    printk(KERN_ERR "%s: unsupported width, height (%dx%d)\n", __func__, width, height);
    return NULL;
}

static int gc0308_set_mbusformat(struct i2c_client *client, const struct gc0308_color_format *cfmt) __attribute__((unused));
static int gc0308_set_mbusformat(struct i2c_client *client, const struct gc0308_color_format *cfmt)
{
    enum v4l2_mbus_pixelcode code;
    int ret = -1;
    code = cfmt->code;
    printk("%s: code 0x%x\n", __func__, code);
    switch (code) {
        case V4L2_MBUS_FMT_YUYV8_2X8:
            ret  = gc0308_write_array(client, gc0308_fmt_yuv422_yuyv);
            break;
        case V4L2_MBUS_FMT_UYVY8_2X8:
            ret  = gc0308_write_array(client, gc0308_fmt_yuv422_uyvy);
            break;
        case V4L2_MBUS_FMT_YVYU8_2X8:
            ret  = gc0308_write_array(client, gc0308_fmt_yuv422_yvyu);
            break;
        case V4L2_MBUS_FMT_VYUY8_2X8:
            ret  = gc0308_write_array(client, gc0308_fmt_yuv422_vyuy);
            break;
        default:
            printk(KERN_ERR "mbus code error in %s() line %d\n",__FUNCTION__, __LINE__);
    }
    return ret;
}

static int gc0308_set_params(struct v4l2_subdev *sd, u32 *width, u32 *height, enum v4l2_mbus_pixelcode code)
{
    struct gc0308_priv *priv = to_priv(sd);
    const struct gc0308_win_size *old_win, *new_win;
    int i;
    printk("%s .............\n",__func__);
    priv->cfmt = NULL;
    for (i = 0; i < ARRAY_SIZE(gc0308_cfmts); i++) {
        if (code == gc0308_cfmts[i].code) {
            priv->cfmt = gc0308_cfmts + i;
            break;
        }
    }
    if (!priv->cfmt) {
        printk(KERN_ERR "Unsupported sensor format.\n");
        return -EINVAL;
    }

    old_win = priv->win;
    new_win = gc0308_select_win(*width, *height);
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
static int gc0308_set_auto_white_balance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
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
            ret = gc0308_write_array(client, gc0308_awb_regs_diable);
            break;
        case 1:
            ret = gc0308_write_array(client, gc0308_awb_regs_enable);
            break;
    }

    assert(ret == 0);

    return 0;
}

static int gc0308_set_white_balance_temperature(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int white_balance_temperature = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch(white_balance_temperature) {
        case V4L2_WHITE_BALANCE_INCANDESCENT:
            ret = gc0308_write_array(client, gc0308_wb_incandescence_regs);
            break;
        case V4L2_WHITE_BALANCE_FLUORESCENT:
            ret = gc0308_write_array(client,gc0308_wb_fluorescent_regs);
            break;
        case V4L2_WHITE_BALANCE_DAYLIGHT:
            ret = gc0308_write_array(client,gc0308_wb_daylight_regs);
            break;
        case V4L2_WHITE_BALANCE_CLOUDY_DAYLIGHT:
            ret = gc0308_write_array(client,gc0308_wb_cloud_regs);
            break;
        case V4L2_WHITE_BALANCE_TUNGSTEN:
            ret = gc0308_write_array(client,gc0308_wb_tungsten_regs);
            break;
        default:
            dev_err(&client->dev, "set white_balance_temperature over range, white_balance_temperature = %d\n", white_balance_temperature);
            return -ERANGE;
    }

    assert(ret == 0);

    return 0;
}

static int gc0308_set_colorfx(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int colorfx = ctrl->val;
    int ret;

    printk("%s: val %d\n", __func__, ctrl->val);

    switch (colorfx) {
        case V4L2_COLORFX_NONE: /* normal */
            ret = gc0308_write_array(client, gc0308_colorfx_none_regs);
            break;
        case V4L2_COLORFX_BW: /* black and white */
            ret = gc0308_write_array(client, gc0308_colorfx_bw_regs);
            break;
        case V4L2_COLORFX_SEPIA: /* antique ,\B8\B4\B9\C5*/
            ret = gc0308_write_array(client, gc0308_colorfx_sepia_regs);

            break;
        case V4L2_COLORFX_NEGATIVE: /* negative\A3\AC\B8\BAƬ */
            ret = gc0308_write_array(client, gc0308_colorfx_negative_regs);
            break;
        default:
            dev_err(&client->dev, "set colorfx over range, colorfx = %d\n", colorfx);
            return -ERANGE;
    }


    assert(ret == 0);

    return 0;
}

/* TODO */
static int gc0308_set_exposure_auto(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int exposure_auto = ctrl->val;
    printk("%s .............\n",__func__);
    if (exposure_auto < 0 || exposure_auto > 1) {
        dev_err(&client->dev, "set exposure_auto over range, exposure_auto = %d\n", exposure_auto);
        return -ERANGE;
    }

    return 0;
}

/* TODO */
static int gc0308_set_scene_exposure(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* int scene_exposure = ctrl->val; */

    return 0;
}

/* TODO */
static int gc0308_set_prev_capt_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
    /* struct i2c_client *client = v4l2_get_subdevdata(sd); */
    /* int mode = ctrl->val; */
    /* gc0308_priv *priv = to_priv(sd); */

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

static int gc0308_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret = 0;
    printk("%s .............\n",__func__);
    switch (ctrl->id) {
        case V4L2_CID_AUTO_WHITE_BALANCE:
            gc0308_set_auto_white_balance(sd, ctrl);
            break;

        case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
            gc0308_set_white_balance_temperature(sd, ctrl);
            break;

        case V4L2_CID_COLORFX:
            gc0308_set_colorfx(sd, ctrl);
            break;

        case V4L2_CID_EXPOSURE_AUTO:
            gc0308_set_exposure_auto(sd, ctrl);
            break;

        case V4L2_CID_SCENE_EXPOSURE:
            gc0308_set_scene_exposure(sd, ctrl);
            break;

        case V4L2_CID_PRIVATE_PREV_CAPT:
            gc0308_set_prev_capt_mode(sd, ctrl);
            break;

        default:
            dev_err(&client->dev, "%s: invalid control id %d\n", __func__, ctrl->id);
            return -EINVAL;
    }

    return ret;
}

static const struct v4l2_ctrl_ops gc0308_ctrl_ops = {
    .s_ctrl = gc0308_s_ctrl,
};

static const struct v4l2_ctrl_config gc0308_custom_ctrls[] = {
    {
        .ops    = &gc0308_ctrl_ops,
        .id     = V4L2_CID_SCENE_EXPOSURE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "SceneExposure",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &gc0308_ctrl_ops,
        .id     = V4L2_CID_PRIVATE_PREV_CAPT,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "PrevCapt",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    }
};

static int gc0308_initialize_ctrls(struct gc0308_priv *priv)
{
    v4l2_ctrl_handler_init(&priv->hdl, NUM_CTRLS);
    printk("%s .............\n",__func__);
    /* standard ctrls */
    priv->auto_white_balance = v4l2_ctrl_new_std(&priv->hdl, &gc0308_ctrl_ops,
            V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
    if (!priv->auto_white_balance) {
        printk(KERN_ERR "%s: failed to create auto_white_balance ctrl\n", __func__);
        return -ENOENT;
    }

    priv->white_balance_temperature = v4l2_ctrl_new_std(&priv->hdl, &gc0308_ctrl_ops,
            V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 3, 1, 1);
    if (!priv->white_balance_temperature) {
        printk(KERN_ERR "%s: failed to create white_balance_temperature ctrl\n", __func__);
        return -ENOENT;
    }

    /* standard menus */
    priv->colorfx = v4l2_ctrl_new_std_menu(&priv->hdl, &gc0308_ctrl_ops,
            V4L2_CID_COLORFX, 3, 0, 0);
    if (!priv->colorfx) {
        printk(KERN_ERR "%s: failed to create colorfx ctrl\n", __func__);
        return -ENOENT;
    }

    priv->exposure_auto = v4l2_ctrl_new_std_menu(&priv->hdl, &gc0308_ctrl_ops,
            V4L2_CID_EXPOSURE_AUTO, 1, 0, 1);
    if (!priv->exposure_auto) {
        printk(KERN_ERR "%s: failed to create exposure_auto ctrl\n", __func__);
        return -ENOENT;
    }

    /* custom ctrls */
    priv->scene_exposure = v4l2_ctrl_new_custom(&priv->hdl, &gc0308_custom_ctrls[0], NULL);
    if (!priv->scene_exposure) {
        printk(KERN_ERR "%s: failed to create scene_exposure ctrl\n", __func__);
        return -ENOENT;
    }

    priv->prev_capt = v4l2_ctrl_new_custom(&priv->hdl, &gc0308_custom_ctrls[1], NULL);
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
static int gc0308_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *id)
{
    struct gc0308_priv *priv = to_priv(sd);
    id->ident    = priv->model;
    id->revision = 0;
    return 0;
}

static int gc0308_s_power(struct v4l2_subdev *sd, int on)
{
    /* used when suspending */
     printk("%s: on %d\n", __func__, on); 

    if (!on) {
        struct gc0308_priv *priv = to_priv(sd);
        priv->initialized = false;
    }
    return 0;
}

static const struct v4l2_subdev_core_ops gc0308_subdev_core_ops = {
    .g_chip_ident   = gc0308_g_chip_ident,
    .s_power        = gc0308_s_power,
    .s_ctrl         = v4l2_subdev_s_ctrl,
};

/**
 * video ops
 */
static int gc0308_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct gc0308_priv *priv = to_priv(sd);
    int ret = 0;

    printk( "%s: enable %d, initialized %d\n.................", __func__, enable, priv->initialized);

    if (enable) {
        if (!priv->win || !priv->cfmt) {
            dev_err(&client->dev, "norm or win select error\n");
            return -EPERM;
        }

        if (!priv->initialized) {
            if (!check_id(client))
                return -EINVAL;

            ret = gc0308_write_array(client, gc0308_init_regs);
			udelay(10);
            if (ret < 0) {
                printk(KERN_ERR "%s: failed to gc0308_write_array init regs\n", __func__);
                return -EIO;
            }

            priv->initialized = true;
        }

        ret = gc0308_write_array(client, priv->win->win_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to gc0308_write_array win regs\n", __func__);
            return -EIO;
        }

        gc0308_write_array(client, gc0308_fmt_yuv422_yuyv);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to gc0308_write_array format regs\n", __func__);
            return -EIO;
        }

        ret = gc0308_write_array(client, gc0308_enable_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to gc0308_write_array enable regs\n", __func__);
            return -EIO;
        }
    } else {
        ret = gc0308_write_array(client, gc0308_disable_regs);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to gc0308_write_array disable regs\n", __func__);
            return -EIO;
        }
    }

    return 0;
}

static const struct v4l2_subdev_video_ops gc0308_subdev_video_ops= {
    .s_stream = gc0308_s_stream,
};

/**
 * pad ops
 */
static int gc0308_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    struct gc0308_priv *priv = to_priv(sd);
    struct v4l2_mbus_framefmt *mf = &fmt->format;
    int ret = 0;

    printk("%s: %dx%d\n", __func__, mf->width, mf->height);

    ret = gc0308_set_params(sd, &mf->width, &mf->height, mf->code);
    if(!ret)
        mf->colorspace = priv->cfmt->colorspace;

    return ret;
}

static const struct v4l2_subdev_pad_ops gc0308_subdev_pad_ops = {
    .set_fmt = gc0308_s_fmt,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops gc0308_subdev_ops = {
    .core   = &gc0308_subdev_core_ops,
    .video  = &gc0308_subdev_video_ops,
    .pad    = &gc0308_subdev_pad_ops,
};

/**
 * media_entity_operations
 */
static int gc0308_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    printk("%s\n", __func__);
    return 0;
}

static const struct media_entity_operations gc0308_media_ops = {
    .link_setup = gc0308_link_setup,
};

/********************************************************************************
 * initialize
 */
static void gc0308_priv_init(struct gc0308_priv * priv)
{
    priv->model = V4L2_IDENT_GC0308;
    priv->prev_capt_mode = PREVIEW_MODE;
    priv->timeperframe.denominator =12;//30;
    priv->timeperframe.numerator = 1;
    priv->win = &gc0308_win_vga;
}

static int gc0308_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct gc0308_priv *priv;
    struct v4l2_subdev *sd;
    int ret;

    priv = kzalloc(sizeof(struct gc0308_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    gc0308_priv_init(priv);

    sd = &priv->subdev;
    strcpy(sd->name, MODULE_NAME);

    /* register subdev */
    v4l2_i2c_subdev_init(sd, client, &gc0308_subdev_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    priv->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    sd->entity.ops  = &gc0308_media_ops;
    if (media_entity_init(&sd->entity, 1, &priv->pad, 0)) {
        dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
        kfree(priv);
        return -ENOENT;
    }

    ret = gc0308_initialize_ctrls(priv);
    if (ret < 0) {
        printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
        kfree(priv);
        return ret;
    }

    return 0;
}

static int gc0308_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    v4l2_device_unregister_subdev(sd);
    v4l2_ctrl_handler_free(sd->ctrl_handler);
    media_entity_cleanup(&sd->entity);
    kfree(to_priv(sd));
    return 0;
}

static const struct i2c_device_id gc0308_id[] = {
    { MODULE_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, gc0308_id);

static struct i2c_driver gc0308_i2c_driver = {
    .driver = {
        .name = MODULE_NAME,
    },
    .probe    = gc0308_probe,
    .remove   = gc0308_remove,
    .id_table = gc0308_id,
};

module_i2c_driver(gc0308_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for gc0308");
MODULE_AUTHOR("Rio");
MODULE_LICENSE("GPL v2");

