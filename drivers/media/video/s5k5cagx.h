/* linux/drivers/media/video/s5k5cagx.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 * 		http://www.samsung.com/
 *
 * Driver for S5K5CAGX (UXGA camera) from Samsung Electronics
 * 1/4" 2.0Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define S5K5CAGX_COMPLETE
#ifndef __S5K5CAGX_H__
#define __S5K5CAGX_H__

struct s5k5cagx_reg {
    unsigned char addr;
    unsigned char val;
};

struct s5k5cagx_regset_type {
    unsigned char *regset;
    int len;
};

/*
 * Macro
 */
#define REGSET_LENGTH(x)	(sizeof(x)/sizeof(s5k5cagx_reg))

/*
 * User defined commands
 */
/* S/W defined features for tune */
#define REG_DELAY	0xFF00	/* in ms */
#define REG_CMD		0xFFFF	/* Followed by command */

/* Following order should not be changed */
enum image_size_s5k5cagx {
    /* This SoC supports upto UXGA (1600*1200) */
#if 0
    QQVGA,	/* 160*120*/
    QCIF,	/* 176*144 */
    QVGA,	/* 320*240 */
    CIF,	/* 352*288 */
    VGA,	/* 640*480 */
#endif
    SVGA,	/* 800*600 */
#if 0
    HD720P,	/* 1280*720 */
    SXGA,	/* 1280*1024 */
    UXGA,	/* 1600*1200 */
#endif
};

/*
 * Following values describe controls of camera
 * in user aspect and must be match with index of s5k5cagx_regset[]
 * These values indicates each controls and should be used
 * to control each control
 */
enum s5k5cagx_control {
    S5K5CAGX_INIT,
    S5K5CAGX_EV,
    S5K5CAGX_AWB,
    S5K5CAGX_MWB,
    S5K5CAGX_EFFECT,
    S5K5CAGX_CONTRAST,
    S5K5CAGX_SATURATION,
    S5K5CAGX_SHARPNESS,
};

#define S5K5CAGX_REGSET(x)	{	\
    .regset = x,			\
    .len = sizeof(x)/sizeof(s5k5cagx_reg),}

static int read_device_id_for_s5k4eagx(struct i2c_client *client);
static int read_device_id_for_s5k5cagx(struct i2c_client *client);


/*
 * User tuned register setting values
 */
static unsigned short s5k5cagx_init_reg_short[]={

    //=================================================================================================
    // * Name: S5K5CAGX EVT1.0 Initial Setfile
    // * important : TnP and Analog Setting Version is 091216
    // * PLL mode: MCLK=24.5MHz / SYSCLK=40MHz / PCLK=60MHz
    // * FPS : Preview YUV QVGA 22-10fps / Capture YUV QXGA 7.5fps
    //=================================================================================================
    // ARM GO
    // Direct mode
    0xFCFC, 0xD000
        ,
    0x0010, 0x0001, //Reset
    0x1030, 0x0000, //Clear host interrupt so main will wait
    0x0014, 0x0001 //ARM go
        //p10 //Min.10ms delay is required
};

static unsigned short s5k5cagx_init_reg_short2[]={
    // Start T&P part
    // DO NOT DELETE T&P SECTION COMMENTS! They are required to debug T&P related issues.
    // svn://transrdsrv/svn/svnroot/System/Software/tcevb/SDK+FW/ISP_5CA/Firmware
    // Rev: 32375-32375
    // Signature:
    // md5 78c1a0d32ef22ba270994f08d64a05a0 .btp
    // md5 6765ffc40fde4420aab81f0039a60c38 .htp
    // md5 956e8c724c34dd8b76dd297b92f59677 .RegsMap.h
    // md5 7db8e8f88de22128b8b909128f087a53 .RegsMap.bin
    // md5 506b4144bd48cdb79cbecdda4f7176ba .base.RegsMap.h
    // md5 fd8f92f13566c1a788746b23691c5f5f .base.RegsMap.bin
    //
    // End T&P part
    0x0028, 0x7000,
    0x002A, 0x2CF8,
    0x0F12, 0xB510,
    0x0F12, 0x4827,
    0x0F12, 0x21C0,
    0x0F12, 0x8041,
    0x0F12, 0x4825,
    0x0F12, 0x4A26,
    0x0F12, 0x3020,
    0x0F12, 0x8382,
    0x0F12, 0x1D12,
    0x0F12, 0x83C2,
    0x0F12, 0x4822,
    0x0F12, 0x3040,
    0x0F12, 0x8041,
    0x0F12, 0x4821,
    0x0F12, 0x4922,
    0x0F12, 0x3060,
    0x0F12, 0x8381,
    0x0F12, 0x1D09,
    0x0F12, 0x83C1,
    0x0F12, 0x4821,
    0x0F12, 0x491D,
    0x0F12, 0x8802,
    0x0F12, 0x3980,
    0x0F12, 0x804A,
    0x0F12, 0x8842,
    0x0F12, 0x808A,
    0x0F12, 0x8882,
    0x0F12, 0x80CA,
    0x0F12, 0x88C2,
    0x0F12, 0x810A,
    0x0F12, 0x8902,
    0x0F12, 0x491C,
    0x0F12, 0x80CA,
    0x0F12, 0x8942,
    0x0F12, 0x814A,
    0x0F12, 0x8982,
    0x0F12, 0x830A,
    0x0F12, 0x89C2,
    0x0F12, 0x834A,
    0x0F12, 0x8A00,
    0x0F12, 0x4918,
    0x0F12, 0x8188,
    0x0F12, 0x4918,
    0x0F12, 0x4819,
    0x0F12, 0xF000,
    0x0F12, 0xFA0E,
    0x0F12, 0x4918,
    0x0F12, 0x4819,
    0x0F12, 0x6341,
    0x0F12, 0x4919,
    0x0F12, 0x4819,
    0x0F12, 0xF000,
    0x0F12, 0xFA07,
    0x0F12, 0x4816,
    0x0F12, 0x4918,
    0x0F12, 0x3840,
    0x0F12, 0x62C1,
    0x0F12, 0x4918,
    0x0F12, 0x3880,
    0x0F12, 0x63C1,
    0x0F12, 0x4917,
    0x0F12, 0x6301,
    0x0F12, 0x4917,
    0x0F12, 0x3040,
    0x0F12, 0x6181,
    0x0F12, 0x4917,
    0x0F12, 0x4817,
    0x0F12, 0xF000,
    0x0F12, 0xF9F7,
    0x0F12, 0x4917,
    0x0F12, 0x4817,
    0x0F12, 0xF000,
    0x0F12, 0xF9F3,
    0x0F12, 0x4917,
    0x0F12, 0x4817,
    0x0F12, 0xF000,
    0x0F12, 0xF9EF,
    0x0F12, 0xBC10,
    0x0F12, 0xBC08,
    0x0F12, 0x4718,
    0x0F12, 0x1100,
    0x0F12, 0xD000,
    0x0F12, 0x267C,
    0x0F12, 0x0000,
    0x0F12, 0x2CE8,
    0x0F12, 0x0000,
    0x0F12, 0x3274,
    0x0F12, 0x7000,
    0x0F12, 0xF400,
    0x0F12, 0xD000,
    0x0F12, 0xF520,
    0x0F12, 0xD000,
    0x0F12, 0x2DF1,
    0x0F12, 0x7000,
    0x0F12, 0x89A9,
    0x0F12, 0x0000,
    0x0F12, 0x2E43,
    0x0F12, 0x7000,
    0x0F12, 0x0140,
    0x0F12, 0x7000,
    0x0F12, 0x2E7D,
    0x0F12, 0x7000,
    0x0F12, 0xB4F7,
    0x0F12, 0x0000,
    0x0F12, 0x2F07,
    0x0F12, 0x7000,
    0x0F12, 0x2F2B,
    0x0F12, 0x7000,
    0x0F12, 0x2FD1,
    0x0F12, 0x7000,
    0x0F12, 0x2FE5,
    0x0F12, 0x7000,
    0x0F12, 0x2FB9,
    0x0F12, 0x7000,
    0x0F12, 0x013D,
    0x0F12, 0x0001,
    0x0F12, 0x306B,
    0x0F12, 0x7000,
    0x0F12, 0x5823,
    0x0F12, 0x0000,
    0x0F12, 0x30B9,
    0x0F12, 0x7000,
    0x0F12, 0xD789,
    0x0F12, 0x0000,
    0x0F12, 0xB570,
    0x0F12, 0x6804,
    0x0F12, 0x6845,
    0x0F12, 0x6881,
    0x0F12, 0x6840,
    0x0F12, 0x2900,
    0x0F12, 0x6880,
    0x0F12, 0xD007,
    0x0F12, 0x49C3,
    0x0F12, 0x8949,
    0x0F12, 0x084A,
    0x0F12, 0x1880,
    0x0F12, 0xF000,
    0x0F12, 0xF9BA,
    0x0F12, 0x80A0,
    0x0F12, 0xE000,
    0x0F12, 0x80A0,
    0x0F12, 0x88A0,
    0x0F12, 0x2800,
    0x0F12, 0xD010,
    0x0F12, 0x68A9,
    0x0F12, 0x6828,
    0x0F12, 0x084A,
    0x0F12, 0x1880,
    0x0F12, 0xF000,
    0x0F12, 0xF9AE,
    0x0F12, 0x8020,
    0x0F12, 0x1D2D,
    0x0F12, 0xCD03,
    0x0F12, 0x084A,
    0x0F12, 0x1880,
    0x0F12, 0xF000,
    0x0F12, 0xF9A7,
    0x0F12, 0x8060,
    0x0F12, 0xBC70,
    0x0F12, 0xBC08,
    0x0F12, 0x4718,
    0x0F12, 0x2000,
    0x0F12, 0x8060,
    0x0F12, 0x8020,
    0x0F12, 0xE7F8,
    0x0F12, 0xB510,
    0x0F12, 0xF000,
    0x0F12, 0xF9A2,
    0x0F12, 0x48B2,
    0x0F12, 0x8A40,
    0x0F12, 0x2800,
    0x0F12, 0xD00C,
    0x0F12, 0x48B1,
    0x0F12, 0x49B2,
    0x0F12, 0x8800,
    0x0F12, 0x4AB2,
    0x0F12, 0x2805,
    0x0F12, 0xD003,
    0x0F12, 0x4BB1,
    0x0F12, 0x795B,
    0x0F12, 0x2B00,
    0x0F12, 0xD005,
    0x0F12, 0x2001,
    0x0F12, 0x8008,
    0x0F12, 0x8010,
    0x0F12, 0xBC10,
    0x0F12, 0xBC08,
    0x0F12, 0x4718,
    0x0F12, 0x2800,
    0x0F12, 0xD1FA,
    0x0F12, 0x2000,
    0x0F12, 0x8008,
    0x0F12, 0x8010,
    0x0F12, 0xE7F6,
    0x0F12, 0xB5F8,
    0x0F12, 0x2407,
    0x0F12, 0x2C06,
    0x0F12, 0xD035,
    0x0F12, 0x2C07,
    0x0F12, 0xD033,
    0x0F12, 0x48A3,
    0x0F12, 0x8BC1,
    0x0F12, 0x2900,
    0x0F12, 0xD02A,
    0x0F12, 0x00A2,
    0x0F12, 0x1815,
    0x0F12, 0x4AA4,
    0x0F12, 0x6DEE,
    0x0F12, 0x8A92,
    0x0F12, 0x4296,
    0x0F12, 0xD923,
    0x0F12, 0x0028,
    0x0F12, 0x3080,
    0x0F12, 0x0007,
    0x0F12, 0x69C0,
    0x0F12, 0xF000,
    0x0F12, 0xF96B,
    0x0F12, 0x1C71,
    0x0F12, 0x0280,
    0x0F12, 0xF000,
    0x0F12, 0xF967,
    0x0F12, 0x0006,
    0x0F12, 0x4898,
    0x0F12, 0x0061,
    0x0F12, 0x1808,
    0x0F12, 0x8D80,
    0x0F12, 0x0A01,
    0x0F12, 0x0600,
    0x0F12, 0x0E00,
    0x0F12, 0x1A08,
    0x0F12, 0xF000,
    0x0F12, 0xF96A,
    0x0F12, 0x0002,
    0x0F12, 0x6DE9,
    0x0F12, 0x6FE8,
    0x0F12, 0x1A08,
    0x0F12, 0x4351,
    0x0F12, 0x0300,
    0x0F12, 0x1C49,
    0x0F12, 0xF000,
    0x0F12, 0xF953,
    0x0F12, 0x0401,
    0x0F12, 0x0430,
    0x0F12, 0x0C00,
    0x0F12, 0x4301,
    0x0F12, 0x61F9,
    0x0F12, 0xE004,
    0x0F12, 0x00A2,
    0x0F12, 0x4990,
    0x0F12, 0x1810,
    0x0F12, 0x3080,
    0x0F12, 0x61C1,
    0x0F12, 0x1E64,
    0x0F12, 0xD2C5,
    0x0F12, 0x2006,
    0x0F12, 0xF000,
    0x0F12, 0xF959,
    0x0F12, 0x2007,
    0x0F12, 0xF000,
    0x0F12, 0xF956,
    0x0F12, 0xBCF8,
    0x0F12, 0xBC08,
    0x0F12, 0x4718,
    0x0F12, 0xB510,
    0x0F12, 0xF000,
    0x0F12, 0xF958,
    0x0F12, 0x2800,
    0x0F12, 0xD00A,
    0x0F12, 0x4881,
    0x0F12, 0x8B81,
    0x0F12, 0x0089,
    0x0F12, 0x1808,
    0x0F12, 0x6DC1,
    0x0F12, 0x4883,
    0x0F12, 0x8A80,
    0x0F12, 0x4281,
    0x0F12, 0xD901,
    0x0F12, 0x2001,
    0x0F12, 0xE7A1,
    0x0F12, 0x2000,
    0x0F12, 0xE79F,
    0x0F12, 0xB5F8,
    0x0F12, 0x0004,
    0x0F12, 0x4F80,
    0x0F12, 0x227D,
    0x0F12, 0x8938,
    0x0F12, 0x0152,
    0x0F12, 0x4342,
    0x0F12, 0x487E,
    0x0F12, 0x9000,
    0x0F12, 0x8A01,
    0x0F12, 0x0848,
    0x0F12, 0x1810,
    0x0F12, 0xF000,
    0x0F12, 0xF91D,
    0x0F12, 0x210F,
    0x0F12, 0xF000,
    0x0F12, 0xF940,
    0x0F12, 0x497A,
    0x0F12, 0x8C49,
    0x0F12, 0x090E,
    0x0F12, 0x0136,
    0x0F12, 0x4306,
    0x0F12, 0x4979,
    0x0F12, 0x2C00,
    0x0F12, 0xD003,
    0x0F12, 0x2001,
    0x0F12, 0x0240,
    0x0F12, 0x4330,
    0x0F12, 0x8108,
    0x0F12, 0x4876,
    0x0F12, 0x2C00,
    0x0F12, 0x8D00,
    0x0F12, 0xD001,
    0x0F12, 0x2501,
    0x0F12, 0xE000,
    0x0F12, 0x2500,
    0x0F12, 0x4972,
    0x0F12, 0x4328,
    0x0F12, 0x8008,
    0x0F12, 0x207D,
    0x0F12, 0x00C0,
    0x0F12, 0xF000,
    0x0F12, 0xF92E,
    0x0F12, 0x2C00,
    0x0F12, 0x496E,
    0x0F12, 0x0328,
    0x0F12, 0x4330,
    0x0F12, 0x8108,
    0x0F12, 0x88F8,
    0x0F12, 0x2C00,
    0x0F12, 0x01AA,
    0x0F12, 0x4310,
    0x0F12, 0x8088,
    0x0F12, 0x9800,
    0x0F12, 0x8A01,
    0x0F12, 0x486A,
    0x0F12, 0xF000,
    0x0F12, 0xF8F1,
    0x0F12, 0x496A,
    0x0F12, 0x8809,
    0x0F12, 0x4348,
    0x0F12, 0x0400,
    0x0F12, 0x0C00,
    0x0F12, 0xF000,
    0x0F12, 0xF918,
    0x0F12, 0x0020,
    0x0F12, 0xF000,
    0x0F12, 0xF91D,
    0x0F12, 0x4866,
    0x0F12, 0x7004,
    0x0F12, 0xE7A3,
    0x0F12, 0xB510,
    0x0F12, 0x0004,
    0x0F12, 0xF000,
    0x0F12, 0xF91E,
    0x0F12, 0x6020,
    0x0F12, 0x4963,
    0x0F12, 0x8B49,
    0x0F12, 0x0789,
    0x0F12, 0xD001,
    0x0F12, 0x0040,
    0x0F12, 0x6020,
    0x0F12, 0xE74C,
    0x0F12, 0xB510,
    0x0F12, 0xF000,
    0x0F12, 0xF91B,
    0x0F12, 0x485F,
    0x0F12, 0x8880,
    0x0F12, 0x0601,
    0x0F12, 0x4854,
    0x0F12, 0x1609,
    0x0F12, 0x8141,
    0x0F12, 0xE742,
    0x0F12, 0xB5F8,
    0x0F12, 0x000F,
    0x0F12, 0x4C55,
    0x0F12, 0x3420,
    0x0F12, 0x2500,
    0x0F12, 0x5765,
    0x0F12, 0x0039,
    0x0F12, 0xF000,
    0x0F12, 0xF913,
    0x0F12, 0x9000,
    0x0F12, 0x2600,
    0x0F12, 0x57A6,
    0x0F12, 0x4C4C,
    0x0F12, 0x42AE,
    0x0F12, 0xD01B,
    0x0F12, 0x4D54,
    0x0F12, 0x8AE8,
    0x0F12, 0x2800,
    0x0F12, 0xD013,
    0x0F12, 0x484D,
    0x0F12, 0x8A01,
    0x0F12, 0x8B80,
    0x0F12, 0x4378,
    0x0F12, 0xF000,
    0x0F12, 0xF8B5,
    0x0F12, 0x89A9,
    0x0F12, 0x1A41,
    0x0F12, 0x484E,
    0x0F12, 0x3820,
    0x0F12, 0x8AC0,
    0x0F12, 0x4348,
    0x0F12, 0x17C1,
    0x0F12, 0x0D89,
    0x0F12, 0x1808,
    0x0F12, 0x1280,
    0x0F12, 0x8961,
    0x0F12, 0x1A08,
    0x0F12, 0x8160,
    0x0F12, 0xE003,
    0x0F12, 0x88A8,
    0x0F12, 0x0600,
    0x0F12, 0x1600,
    0x0F12, 0x8160,
    0x0F12, 0x200A,
    0x0F12, 0x5E20,
    0x0F12, 0x42B0,
    0x0F12, 0xD011,
    0x0F12, 0xF000,
    0x0F12, 0xF8AB,
    0x0F12, 0x1D40,
    0x0F12, 0x00C3,
    0x0F12, 0x1A18,
    0x0F12, 0x214B,
    0x0F12, 0xF000,
    0x0F12, 0xF897,
    0x0F12, 0x211F,
    0x0F12, 0xF000,
    0x0F12, 0xF8BA,
    0x0F12, 0x210A,
    0x0F12, 0x5E61,
    0x0F12, 0x0FC9,
    0x0F12, 0x0149,
    0x0F12, 0x4301,
    0x0F12, 0x483D,
    0x0F12, 0x81C1,
    0x0F12, 0x9800,
    0x0F12, 0xE74A,
    0x0F12, 0xB5F1,
    0x0F12, 0xB082,
    0x0F12, 0x2500,
    0x0F12, 0x483A,
    0x0F12, 0x9001,
    0x0F12, 0x2400,
    0x0F12, 0x2028,
    0x0F12, 0x4368,
    0x0F12, 0x4A39,
    0x0F12, 0x4925,
    0x0F12, 0x1887,
    0x0F12, 0x1840,
    0x0F12, 0x9000,
    0x0F12, 0x9800,
    0x0F12, 0x0066,
    0x0F12, 0x9A01,
    0x0F12, 0x1980,
    0x0F12, 0x218C,
    0x0F12, 0x5A09,
    0x0F12, 0x8A80,
    0x0F12, 0x8812,
    0x0F12, 0xF000,
    0x0F12, 0xF8CA,
    0x0F12, 0x53B8,
    0x0F12, 0x1C64,
    0x0F12, 0x2C14,
    0x0F12, 0xDBF1,
    0x0F12, 0x1C6D,
    0x0F12, 0x2D03,
    0x0F12, 0xDBE6,
    0x0F12, 0x9802,
    0x0F12, 0x6800,
    0x0F12, 0x0600,
    0x0F12, 0x0E00,
    0x0F12, 0xF000,
    0x0F12, 0xF8C5,
    0x0F12, 0xBCFE,
    0x0F12, 0xBC08,
    0x0F12, 0x4718,
    0x0F12, 0xB570,
    0x0F12, 0x6805,
    0x0F12, 0x2404,
    0x0F12, 0xF000,
    0x0F12, 0xF8C5,
    0x0F12, 0x2800,
    0x0F12, 0xD103,
    0x0F12, 0xF000,
    0x0F12, 0xF8C9,
    0x0F12, 0x2800,
    0x0F12, 0xD000,
    0x0F12, 0x2400,
    0x0F12, 0x3540,
    0x0F12, 0x88E8,
    0x0F12, 0x0500,
    0x0F12, 0xD403,
    0x0F12, 0x4822,
    0x0F12, 0x89C0,
    0x0F12, 0x2800,
    0x0F12, 0xD002,
    0x0F12, 0x2008,
    0x0F12, 0x4304,
    0x0F12, 0xE001,
    0x0F12, 0x2010,
    0x0F12, 0x4304,
    0x0F12, 0x481F,
    0x0F12, 0x8B80,
    0x0F12, 0x0700,
    0x0F12, 0x0F81,
    0x0F12, 0x2001,
    0x0F12, 0x2900,
    0x0F12, 0xD000,
    0x0F12, 0x4304,
    0x0F12, 0x491C,
    0x0F12, 0x8B0A,
    0x0F12, 0x42A2,
    0x0F12, 0xD004,
    0x0F12, 0x0762,
    0x0F12, 0xD502,
    0x0F12, 0x4A19,
    0x0F12, 0x3220,
    0x0F12, 0x8110,
    0x0F12, 0x830C,
    0x0F12, 0xE691,
    0x0F12, 0x0C3C,
    0x0F12, 0x7000,
    0x0F12, 0x3274,
    0x0F12, 0x7000,
    0x0F12, 0x26E8,
    0x0F12, 0x7000,
    0x0F12, 0x6100,
    0x0F12, 0xD000,
    0x0F12, 0x6500,
    0x0F12, 0xD000,
    0x0F12, 0x1A7C,
    0x0F12, 0x7000,
    0x0F12, 0x1120,
    0x0F12, 0x7000,
    0x0F12, 0xFFFF,
    0x0F12, 0x0000,
    0x0F12, 0x3374,
    0x0F12, 0x7000,
    0x0F12, 0x1D6C,
    0x0F12, 0x7000,
    0x0F12, 0x167C,
    0x0F12, 0x7000,
    0x0F12, 0xF400,
    0x0F12, 0xD000,
    0x0F12, 0x2C2C,
    0x0F12, 0x7000,
    0x0F12, 0x40A0,
    0x0F12, 0x00DD,
    0x0F12, 0xF520,
    0x0F12, 0xD000,
    0x0F12, 0x2C29,
    0x0F12, 0x7000,
    0x0F12, 0x1A54,
    0x0F12, 0x7000,
    0x0F12, 0x1564,
    0x0F12, 0x7000,
    0x0F12, 0xF2A0,
    0x0F12, 0xD000,
    0x0F12, 0x2440,
    0x0F12, 0x7000,
    0x0F12, 0x05A0,
    0x0F12, 0x7000,
    0x0F12, 0x2894,
    0x0F12, 0x7000,
    0x0F12, 0x1224,
    0x0F12, 0x7000,
    0x0F12, 0xB000,
    0x0F12, 0xD000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x1A3F,
    0x0F12, 0x0001,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xF004,
    0x0F12, 0xE51F,
    0x0F12, 0x1F48,
    0x0F12, 0x0001,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x24BD,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x36DD,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xB4CF,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xB5D7,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x36ED,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xF53F,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xF5D9,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x013D,
    0x0F12, 0x0001,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xF5C9,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xFAA9,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x3723,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0x5823,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xD771,
    0x0F12, 0x0000,
    0x0F12, 0x4778,
    0x0F12, 0x46C0,
    0x0F12, 0xC000,
    0x0F12, 0xE59F,
    0x0F12, 0xFF1C,
    0x0F12, 0xE12F,
    0x0F12, 0xD75B,
    0x0F12, 0x0000,
    0x0F12, 0x8117,
    0x0F12, 0x0000,

    //============================================================
    // CIS/APS/Analog setting- 400LSBSYSCLK 45MHz -091127
    //============================================================
    // This registers are for FACTORY ONLY. If you change it without prior notification,
    // YOU are RESPONSIBLE for the FAILURE that will happen in the future.
    //============================================================
    0x0028, 0x7000,
    0x002A, 0x157A,
    0x0F12, 0x0001,
    0x002A, 0x1578,
    0x0F12, 0x0001,
    0x002A, 0x1576,
    0x0F12, 0x0020,
    0x002A, 0x1574,
    0x0F12, 0x0006,
    0x002A, 0x156E,
    0x0F12, 0x0001, // Slope calibration tolerance in units of 1/256
    0x002A, 0x1568,
    0x0F12, 0x00FC,

    //ADC control
    0x002A, 0x155A,
    0x0F12, 0x01CC, //ADC SAT of 450mV for 10bit default in EVT1
    0x002A, 0x157E,
    0x0F12, 0x0C80, // 3200 Max. Reset ramp DCLK counts (default 2048 0x800)
    0x0F12, 0x0578, // 1400 Max. Reset ramp DCLK counts for x3.5
    0x002A, 0x157C,
    0x0F12, 0x0190, // 400 Reset ramp for x1 in DCLK counts
    0x002A, 0x1570,
    0x0F12, 0x00A0, // 256 LSB
    0x0F12, 0x0010, // reset threshold
    0x002A, 0x12C4,
    0x0F12, 0x006A, // 106 additional timing columns.
    0x002A, 0x12C8,
    0x0F12, 0x08AC, // 2220 ADC columns in normal mode including Hold & Latch
    0x0F12, 0x0050, // 80 addition of ADC columns in Y-ave mode (default 244 0x74)

    //WRITE #senHal_ForceModeType 0001 // Long exposure mode
    0x002A, 0x1696,
    0x0F12, 0x0000, // based on APS guidelines
    0x0F12, 0x0000, // based on APS guidelines
    0x0F12, 0x00C6, // default. 1492 used for ADC dark characteristics
    0x0F12, 0x00C6,

    0x002A, 0x1690,
    0x0F12, 0x0001, // when set double sampling is activated - requires different set of pointers

    0x002A, 0x12B0,
    0x0F12, 0x0055, // comp and pixel bias control 0xF40E - default for EVT1
    0x0F12, 0x005A, // comp and pixel bias control 0xF40E for binning mode

    0x002A, 0x337A,
    0x0F12, 0x0006, // [7] - is used for rest-only mode (EVT0 value is 0xD and HW 0x6)
    0x0F12, 0x0068, // 104M

    0x002A, 0x169E,
    0x0F12, 0x0007,
    0x002A, 0x0BF6,
    0x0F12, 0x0000,

    0x002A, 0x327C,
    0x0F12, 0x1000, // [11]: Enable DBLR Regulation
    0x0F12, 0x6998, // [3:0]: VPIX ~2.8V ****
    0x0F12, 0x0078, // [0]: Static RC-filter
    0x0F12, 0x04FE, // [7:4]: Full RC-filter
    0x0F12, 0x8800, // [11]: Add load to CDS block

    0x002A, 0x3274,
    0x0F12, 0x0000, //#Tune_TP_IO_DrivingCurrent_D0_D4_cs10Set IO driving current
    0x0F12, 0x0000, //#Tune_TP_IO_DrivingCurrent_D9_D5_cs10Set IO driving current
    0x0F12, 0x1555, //#Tune_TP_IO_DrivingCurrent_GPIO_cd10 Set IO driving current
    0x0F12, 0x0510, //#Tune_TP_IO_DrivingCurrent_CLKs_cd10 Set IO driving current

    //Asserting CDS pointers - Long exposure MS Normal
    //Conditions: 10bit, ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60
    0x002A, 0x12D2,
    0x0F12, 0x0003, //#senHal_pContSenModesRegsArray[0][0]2 700012D2
    0x0F12, 0x0003, //#senHal_pContSenModesRegsArray[0][1]2 700012D4
    0x0F12, 0x0003, //#senHal_pContSenModesRegsArray[0][2]2 700012D6
    0x0F12, 0x0003, //#senHal_pContSenModesRegsArray[0][3]2 700012D8
    0x0F12, 0x0884, //#senHal_pContSenModesRegsArray[1][0]2 700012DA
    0x0F12, 0x08CF, //#senHal_pContSenModesRegsArray[1][1]2 700012DC
    0x0F12, 0x0500, //#senHal_pContSenModesRegsArray[1][2]2 700012DE
    0x0F12, 0x054B, //#senHal_pContSenModesRegsArray[1][3]2 700012E0
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[2][0]2 700012E2
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[2][1]2 700012E4
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[2][2]2 700012E6
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[2][3]2 700012E8
    0x0F12, 0x0885, //#senHal_pContSenModesRegsArray[3][0]2 700012EA
    0x0F12, 0x0467, //#senHal_pContSenModesRegsArray[3][1]2 700012EC
    0x0F12, 0x0501, //#senHal_pContSenModesRegsArray[3][2]2 700012EE
    0x0F12, 0x02A5, //#senHal_pContSenModesRegsArray[3][3]2 700012F0
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[4][0]2 700012F2
    0x0F12, 0x046A, //#senHal_pContSenModesRegsArray[4][1]2 700012F4
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[4][2]2 700012F6
    0x0F12, 0x02A8, //#senHal_pContSenModesRegsArray[4][3]2 700012F8
    0x0F12, 0x0885, //#senHal_pContSenModesRegsArray[5][0]2 700012FA
    0x0F12, 0x08D0, //#senHal_pContSenModesRegsArray[5][1]2 700012FC
    0x0F12, 0x0501, //#senHal_pContSenModesRegsArray[5][2]2 700012FE
    0x0F12, 0x054C, //#senHal_pContSenModesRegsArray[5][3]2 70001300
    0x0F12, 0x0006, //#senHal_pContSenModesRegsArray[6][0]2 70001302
    0x0F12, 0x0020, //#senHal_pContSenModesRegsArray[6][1]2 70001304
    0x0F12, 0x0006, //#senHal_pContSenModesRegsArray[6][2]2 70001306
    0x0F12, 0x0020, //#senHal_pContSenModesRegsArray[6][3]2 70001308
    0x0F12, 0x0881, //#senHal_pContSenModesRegsArray[7][0]2 7000130A
    0x0F12, 0x0463, //#senHal_pContSenModesRegsArray[7][1]2 7000130C
    0x0F12, 0x04FD, //#senHal_pContSenModesRegsArray[7][2]2 7000130E
    0x0F12, 0x02A1, //#senHal_pContSenModesRegsArray[7][3]2 70001310
    0x0F12, 0x0006, //#senHal_pContSenModesRegsArray[8][0]2 70001312
    0x0F12, 0x0489, //#senHal_pContSenModesRegsArray[8][1]2 70001314
    0x0F12, 0x0006, //#senHal_pContSenModesRegsArray[8][2]2 70001316
    0x0F12, 0x02C7, //#senHal_pContSenModesRegsArray[8][3]2 70001318
    0x0F12, 0x0881, //#senHal_pContSenModesRegsArray[9][0]2 7000131A
    0x0F12, 0x08CC, //#senHal_pContSenModesRegsArray[9][1]2 7000131C
    0x0F12, 0x04FD, //#senHal_pContSenModesRegsArray[9][2]2 7000131E
    0x0F12, 0x0548, //#senHal_pContSenModesRegsArray[9][3]2 70001320
    0x0F12, 0x03A2, //#senHal_pContSenModesRegsArray[10][0] 2 70001322
    0x0F12, 0x01D3, //#senHal_pContSenModesRegsArray[10][1] 2 70001324
    0x0F12, 0x01E0, //#senHal_pContSenModesRegsArray[10][2] 2 70001326
    0x0F12, 0x00F2, //#senHal_pContSenModesRegsArray[10][3] 2 70001328
    0x0F12, 0x03F2, //#senHal_pContSenModesRegsArray[11][0] 2 7000132A
    0x0F12, 0x0223, //#senHal_pContSenModesRegsArray[11][1] 2 7000132C
    0x0F12, 0x0230, //#senHal_pContSenModesRegsArray[11][2] 2 7000132E
    0x0F12, 0x0142, //#senHal_pContSenModesRegsArray[11][3] 2 70001330
    0x0F12, 0x03A2, //#senHal_pContSenModesRegsArray[12][0] 2 70001332
    0x0F12, 0x063C, //#senHal_pContSenModesRegsArray[12][1] 2 70001334
    0x0F12, 0x01E0, //#senHal_pContSenModesRegsArray[12][2] 2 70001336
    0x0F12, 0x0399, //#senHal_pContSenModesRegsArray[12][3] 2 70001338
    0x0F12, 0x03F2, //#senHal_pContSenModesRegsArray[13][0] 2 7000133A
    0x0F12, 0x068C, //#senHal_pContSenModesRegsArray[13][1] 2 7000133C
    0x0F12, 0x0230, //#senHal_pContSenModesRegsArray[13][2] 2 7000133E
    0x0F12, 0x03E9, //#senHal_pContSenModesRegsArray[13][3] 2 70001340
    0x0F12, 0x0002, //#senHal_pContSenModesRegsArray[14][0] 2 70001342
    0x0F12, 0x0002, //#senHal_pContSenModesRegsArray[14][1] 2 70001344
    0x0F12, 0x0002, //#senHal_pContSenModesRegsArray[14][2] 2 70001346
    0x0F12, 0x0002, //#senHal_pContSenModesRegsArray[14][3] 2 70001348
    0x0F12, 0x003C, //#senHal_pContSenModesRegsArray[15][0] 2 7000134A
    0x0F12, 0x003C, //#senHal_pContSenModesRegsArray[15][1] 2 7000134C
    0x0F12, 0x003C, //#senHal_pContSenModesRegsArray[15][2] 2 7000134E
    0x0F12, 0x003C, //#senHal_pContSenModesRegsArray[15][3] 2 70001350
    0x0F12, 0x01D3, //#senHal_pContSenModesRegsArray[16][0] 2 70001352
    0x0F12, 0x01D3, //#senHal_pContSenModesRegsArray[16][1] 2 70001354
    0x0F12, 0x00F2, //#senHal_pContSenModesRegsArray[16][2] 2 70001356
    0x0F12, 0x00F2, //#senHal_pContSenModesRegsArray[16][3] 2 70001358
    0x0F12, 0x020B, //#senHal_pContSenModesRegsArray[17][0] 2 7000135A
    0x0F12, 0x024A, //#senHal_pContSenModesRegsArray[17][1] 2 7000135C
    0x0F12, 0x012A, //#senHal_pContSenModesRegsArray[17][2] 2 7000135E
    0x0F12, 0x0169, //#senHal_pContSenModesRegsArray[17][3] 2 70001360
    0x0F12, 0x0002, //#senHal_pContSenModesRegsArray[18][0] 2 70001362
    0x0F12, 0x046B, //#senHal_pContSenModesRegsArray[18][1] 2 70001364
    0x0F12, 0x0002, //#senHal_pContSenModesRegsArray[18][2] 2 70001366
    0x0F12, 0x02A9, //#senHal_pContSenModesRegsArray[18][3] 2 70001368
    0x0F12, 0x0419, //#senHal_pContSenModesRegsArray[19][0] 2 7000136A
    0x0F12, 0x04A5, //#senHal_pContSenModesRegsArray[19][1] 2 7000136C
    0x0F12, 0x0257, //#senHal_pContSenModesRegsArray[19][2] 2 7000136E
    0x0F12, 0x02E3, //#senHal_pContSenModesRegsArray[19][3] 2 70001370
    0x0F12, 0x0630, //#senHal_pContSenModesRegsArray[20][0] 2 70001372
    0x0F12, 0x063C, //#senHal_pContSenModesRegsArray[20][1] 2 70001374
    0x0F12, 0x038D, //#senHal_pContSenModesRegsArray[20][2] 2 70001376
    0x0F12, 0x0399, //#senHal_pContSenModesRegsArray[20][3] 2 70001378
    0x0F12, 0x0668, //#senHal_pContSenModesRegsArray[21][0] 2 7000137A
    0x0F12, 0x06B3, //#senHal_pContSenModesRegsArray[21][1] 2 7000137C
    0x0F12, 0x03C5, //#senHal_pContSenModesRegsArray[21][2] 2 7000137E
    0x0F12, 0x0410, //#senHal_pContSenModesRegsArray[21][3] 2 70001380
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[22][0] 2 70001382
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[22][1] 2 70001384
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[22][2] 2 70001386
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[22][3] 2 70001388
    0x0F12, 0x03A2, //#senHal_pContSenModesRegsArray[23][0] 2 7000138A
    0x0F12, 0x01D3, //#senHal_pContSenModesRegsArray[23][1] 2 7000138C
    0x0F12, 0x01E0, //#senHal_pContSenModesRegsArray[23][2] 2 7000138E
    0x0F12, 0x00F2, //#senHal_pContSenModesRegsArray[23][3] 2 70001390
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[24][0] 2 70001392
    0x0F12, 0x0461, //#senHal_pContSenModesRegsArray[24][1] 2 70001394
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[24][2] 2 70001396
    0x0F12, 0x029F, //#senHal_pContSenModesRegsArray[24][3] 2 70001398
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[25][0] 2 7000139A
    0x0F12, 0x063C, //#senHal_pContSenModesRegsArray[25][1] 2 7000139C
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[25][2] 2 7000139E
    0x0F12, 0x0399, //#senHal_pContSenModesRegsArray[25][3] 2 700013A0
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[26][0] 2 700013A2
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[26][1] 2 700013A4
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[26][2] 2 700013A6
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[26][3] 2 700013A8
    0x0F12, 0x01D0, //#senHal_pContSenModesRegsArray[27][0] 2 700013AA
    0x0F12, 0x01D0, //#senHal_pContSenModesRegsArray[27][1] 2 700013AC
    0x0F12, 0x00EF, //#senHal_pContSenModesRegsArray[27][2] 2 700013AE
    0x0F12, 0x00EF, //#senHal_pContSenModesRegsArray[27][3] 2 700013B0
    0x0F12, 0x020C, //#senHal_pContSenModesRegsArray[28][0] 2 700013B2
    0x0F12, 0x024B, //#senHal_pContSenModesRegsArray[28][1] 2 700013B4
    0x0F12, 0x012B, //#senHal_pContSenModesRegsArray[28][2] 2 700013B6
    0x0F12, 0x016A, //#senHal_pContSenModesRegsArray[28][3] 2 700013B8
    0x0F12, 0x039F, //#senHal_pContSenModesRegsArray[29][0] 2 700013BA
    0x0F12, 0x045E, //#senHal_pContSenModesRegsArray[29][1] 2 700013BC
    0x0F12, 0x01DD, //#senHal_pContSenModesRegsArray[29][2] 2 700013BE
    0x0F12, 0x029C, //#senHal_pContSenModesRegsArray[29][3] 2 700013C0
    0x0F12, 0x041A, //#senHal_pContSenModesRegsArray[30][0] 2 700013C2
    0x0F12, 0x04A6, //#senHal_pContSenModesRegsArray[30][1] 2 700013C4
    0x0F12, 0x0258, //#senHal_pContSenModesRegsArray[30][2] 2 700013C6
    0x0F12, 0x02E4, //#senHal_pContSenModesRegsArray[30][3] 2 700013C8
    0x0F12, 0x062D, //#senHal_pContSenModesRegsArray[31][0] 2 700013CA
    0x0F12, 0x0639, //#senHal_pContSenModesRegsArray[31][1] 2 700013CC
    0x0F12, 0x038A, //#senHal_pContSenModesRegsArray[31][2] 2 700013CE
    0x0F12, 0x0396, //#senHal_pContSenModesRegsArray[31][3] 2 700013D0
    0x0F12, 0x0669, //#senHal_pContSenModesRegsArray[32][0] 2 700013D2
    0x0F12, 0x06B4, //#senHal_pContSenModesRegsArray[32][1] 2 700013D4
    0x0F12, 0x03C6, //#senHal_pContSenModesRegsArray[32][2] 2 700013D6
    0x0F12, 0x0411, //#senHal_pContSenModesRegsArray[32][3] 2 700013D8
    0x0F12, 0x087C, //#senHal_pContSenModesRegsArray[33][0] 2 700013DA
    0x0F12, 0x08C7, //#senHal_pContSenModesRegsArray[33][1] 2 700013DC
    0x0F12, 0x04F8, //#senHal_pContSenModesRegsArray[33][2] 2 700013DE
    0x0F12, 0x0543, //#senHal_pContSenModesRegsArray[33][3] 2 700013E0
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[34][0] 2 700013E2
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[34][1] 2 700013E4
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[34][2] 2 700013E6
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[34][3] 2 700013E8
    0x0F12, 0x01D0, //#senHal_pContSenModesRegsArray[35][0] 2 700013EA
    0x0F12, 0x01D0, //#senHal_pContSenModesRegsArray[35][1] 2 700013EC
    0x0F12, 0x00EF, //#senHal_pContSenModesRegsArray[35][2] 2 700013EE
    0x0F12, 0x00EF, //#senHal_pContSenModesRegsArray[35][3] 2 700013F0
    0x0F12, 0x020F, //#senHal_pContSenModesRegsArray[36][0] 2 700013F2
    0x0F12, 0x024E, //#senHal_pContSenModesRegsArray[36][1] 2 700013F4
    0x0F12, 0x012E, //#senHal_pContSenModesRegsArray[36][2] 2 700013F6
    0x0F12, 0x016D, //#senHal_pContSenModesRegsArray[36][3] 2 700013F8
    0x0F12, 0x039F, //#senHal_pContSenModesRegsArray[37][0] 2 700013FA
    0x0F12, 0x045E, //#senHal_pContSenModesRegsArray[37][1] 2 700013FC
    0x0F12, 0x01DD, //#senHal_pContSenModesRegsArray[37][2] 2 700013FE
    0x0F12, 0x029C, //#senHal_pContSenModesRegsArray[37][3] 2 70001400
    0x0F12, 0x041D, //#senHal_pContSenModesRegsArray[38][0] 2 70001402
    0x0F12, 0x04A9, //#senHal_pContSenModesRegsArray[38][1] 2 70001404
    0x0F12, 0x025B, //#senHal_pContSenModesRegsArray[38][2] 2 70001406
    0x0F12, 0x02E7, //#senHal_pContSenModesRegsArray[38][3] 2 70001408
    0x0F12, 0x062D, //#senHal_pContSenModesRegsArray[39][0] 2 7000140A
    0x0F12, 0x0639, //#senHal_pContSenModesRegsArray[39][1] 2 7000140C
    0x0F12, 0x038A, //#senHal_pContSenModesRegsArray[39][2] 2 7000140E
    0x0F12, 0x0396, //#senHal_pContSenModesRegsArray[39][3] 2 70001410
    0x0F12, 0x066C, //#senHal_pContSenModesRegsArray[40][0] 2 70001412
    0x0F12, 0x06B7, //#senHal_pContSenModesRegsArray[40][1] 2 70001414
    0x0F12, 0x03C9, //#senHal_pContSenModesRegsArray[40][2] 2 70001416
    0x0F12, 0x0414, //#senHal_pContSenModesRegsArray[40][3] 2 70001418
    0x0F12, 0x087C, //#senHal_pContSenModesRegsArray[41][0] 2 7000141A
    0x0F12, 0x08C7, //#senHal_pContSenModesRegsArray[41][1] 2 7000141C
    0x0F12, 0x04F8, //#senHal_pContSenModesRegsArray[41][2] 2 7000141E
    0x0F12, 0x0543, //#senHal_pContSenModesRegsArray[41][3] 2 70001420
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[42][0] 2 70001422
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[42][1] 2 70001424
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[42][2] 2 70001426
    0x0F12, 0x0040, //#senHal_pContSenModesRegsArray[42][3] 2 70001428
    0x0F12, 0x01D0, //#senHal_pContSenModesRegsArray[43][0] 2 7000142A
    0x0F12, 0x01D0, //#senHal_pContSenModesRegsArray[43][1] 2 7000142C
    0x0F12, 0x00EF, //#senHal_pContSenModesRegsArray[43][2] 2 7000142E
    0x0F12, 0x00EF, //#senHal_pContSenModesRegsArray[43][3] 2 70001430
    0x0F12, 0x020F, //#senHal_pContSenModesRegsArray[44][0] 2 70001432
    0x0F12, 0x024E, //#senHal_pContSenModesRegsArray[44][1] 2 70001434
    0x0F12, 0x012E, //#senHal_pContSenModesRegsArray[44][2] 2 70001436
    0x0F12, 0x016D, //#senHal_pContSenModesRegsArray[44][3] 2 70001438
    0x0F12, 0x039F, //#senHal_pContSenModesRegsArray[45][0] 2 7000143A
    0x0F12, 0x045E, //#senHal_pContSenModesRegsArray[45][1] 2 7000143C
    0x0F12, 0x01DD, //#senHal_pContSenModesRegsArray[45][2] 2 7000143E
    0x0F12, 0x029C, //#senHal_pContSenModesRegsArray[45][3] 2 70001440
    0x0F12, 0x041D, //#senHal_pContSenModesRegsArray[46][0] 2 70001442
    0x0F12, 0x04A9, //#senHal_pContSenModesRegsArray[46][1] 2 70001444
    0x0F12, 0x025B, //#senHal_pContSenModesRegsArray[46][2] 2 70001446
    0x0F12, 0x02E7, //#senHal_pContSenModesRegsArray[46][3] 2 70001448
    0x0F12, 0x062D, //#senHal_pContSenModesRegsArray[47][0] 2 7000144A
    0x0F12, 0x0639, //#senHal_pContSenModesRegsArray[47][1] 2 7000144C
    0x0F12, 0x038A, //#senHal_pContSenModesRegsArray[47][2] 2 7000144E
    0x0F12, 0x0396, //#senHal_pContSenModesRegsArray[47][3] 2 70001450
    0x0F12, 0x066C, //#senHal_pContSenModesRegsArray[48][0] 2 70001452
    0x0F12, 0x06B7, //#senHal_pContSenModesRegsArray[48][1] 2 70001454
    0x0F12, 0x03C9, //#senHal_pContSenModesRegsArray[48][2] 2 70001456
    0x0F12, 0x0414, //#senHal_pContSenModesRegsArray[48][3] 2 70001458
    0x0F12, 0x087C, //#senHal_pContSenModesRegsArray[49][0] 2 7000145A
    0x0F12, 0x08C7, //#senHal_pContSenModesRegsArray[49][1] 2 7000145C
    0x0F12, 0x04F8, //#senHal_pContSenModesRegsArray[49][2] 2 7000145E
    0x0F12, 0x0543, //#senHal_pContSenModesRegsArray[49][3] 2 70001460
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[50][0] 2 70001462
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[50][1] 2 70001464
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[50][2] 2 70001466
    0x0F12, 0x003D, //#senHal_pContSenModesRegsArray[50][3] 2 70001468
    0x0F12, 0x01D2, //#senHal_pContSenModesRegsArray[51][0] 2 7000146A
    0x0F12, 0x01D2, //#senHal_pContSenModesRegsArray[51][1] 2 7000146C
    0x0F12, 0x00F1, //#senHal_pContSenModesRegsArray[51][2] 2 7000146E
    0x0F12, 0x00F1, //#senHal_pContSenModesRegsArray[51][3] 2 70001470
    0x0F12, 0x020C, //#senHal_pContSenModesRegsArray[52][0] 2 70001472
    0x0F12, 0x024B, //#senHal_pContSenModesRegsArray[52][1] 2 70001474
    0x0F12, 0x012B, //#senHal_pContSenModesRegsArray[52][2] 2 70001476
    0x0F12, 0x016A, //#senHal_pContSenModesRegsArray[52][3] 2 70001478
    0x0F12, 0x03A1, //#senHal_pContSenModesRegsArray[53][0] 2 7000147A
    0x0F12, 0x0460, //#senHal_pContSenModesRegsArray[53][1] 2 7000147C
    0x0F12, 0x01DF, //#senHal_pContSenModesRegsArray[53][2] 2 7000147E
    0x0F12, 0x029E, //#senHal_pContSenModesRegsArray[53][3] 2 70001480
    0x0F12, 0x041A, //#senHal_pContSenModesRegsArray[54][0] 2 70001482
    0x0F12, 0x04A6, //#senHal_pContSenModesRegsArray[54][1] 2 70001484
    0x0F12, 0x0258, //#senHal_pContSenModesRegsArray[54][2] 2 70001486
    0x0F12, 0x02E4, //#senHal_pContSenModesRegsArray[54][3] 2 70001488
    0x0F12, 0x062F, //#senHal_pContSenModesRegsArray[55][0] 2 7000148A
    0x0F12, 0x063B, //#senHal_pContSenModesRegsArray[55][1] 2 7000148C
    0x0F12, 0x038C, //#senHal_pContSenModesRegsArray[55][2] 2 7000148E
    0x0F12, 0x0398, //#senHal_pContSenModesRegsArray[55][3] 2 70001490
    0x0F12, 0x0669, //#senHal_pContSenModesRegsArray[56][0] 2 70001492
    0x0F12, 0x06B4, //#senHal_pContSenModesRegsArray[56][1] 2 70001494
    0x0F12, 0x03C6, //#senHal_pContSenModesRegsArray[56][2] 2 70001496
    0x0F12, 0x0411, //#senHal_pContSenModesRegsArray[56][3] 2 70001498
    0x0F12, 0x087E, //#senHal_pContSenModesRegsArray[57][0] 2 7000149A
    0x0F12, 0x08C9, //#senHal_pContSenModesRegsArray[57][1] 2 7000149C
    0x0F12, 0x04FA, //#senHal_pContSenModesRegsArray[57][2] 2 7000149E
    0x0F12, 0x0545, //#senHal_pContSenModesRegsArray[57][3] 2 700014A0
    0x0F12, 0x03A2, //#senHal_pContSenModesRegsArray[58][0] 2 700014A2
    0x0F12, 0x01D3, //#senHal_pContSenModesRegsArray[58][1] 2 700014A4
    0x0F12, 0x01E0, //#senHal_pContSenModesRegsArray[58][2] 2 700014A6
    0x0F12, 0x00F2, //#senHal_pContSenModesRegsArray[58][3] 2 700014A8
    0x0F12, 0x03AF, //#senHal_pContSenModesRegsArray[59][0] 2 700014AA
    0x0F12, 0x01E0, //#senHal_pContSenModesRegsArray[59][1] 2 700014AC
    0x0F12, 0x01ED, //#senHal_pContSenModesRegsArray[59][2] 2 700014AE
    0x0F12, 0x00FF, //#senHal_pContSenModesRegsArray[59][3] 2 700014B0
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[60][0] 2 700014B2
    0x0F12, 0x0461, //#senHal_pContSenModesRegsArray[60][1] 2 700014B4
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[60][2] 2 700014B6
    0x0F12, 0x029F, //#senHal_pContSenModesRegsArray[60][3] 2 700014B8
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[61][0] 2 700014BA
    0x0F12, 0x046E, //#senHal_pContSenModesRegsArray[61][1] 2 700014BC
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[61][2] 2 700014BE
    0x0F12, 0x02AC, //#senHal_pContSenModesRegsArray[61][3] 2 700014C0
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[62][0] 2 700014C2
    0x0F12, 0x063C, //#senHal_pContSenModesRegsArray[62][1] 2 700014C4
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[62][2] 2 700014C6
    0x0F12, 0x0399, //#senHal_pContSenModesRegsArray[62][3] 2 700014C8
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[63][0] 2 700014CA
    0x0F12, 0x0649, //#senHal_pContSenModesRegsArray[63][1] 2 700014CC
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[63][2] 2 700014CE
    0x0F12, 0x03A6, //#senHal_pContSenModesRegsArray[63][3] 2 700014D0
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[64][0] 2 700014D2
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[64][1] 2 700014D4
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[64][2] 2 700014D6
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[64][3] 2 700014D8
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[65][0] 2 700014DA
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[65][1] 2 700014DC
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[65][2] 2 700014DE
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[65][3] 2 700014E0
    0x0F12, 0x03AA, //#senHal_pContSenModesRegsArray[66][0] 2 700014E2
    0x0F12, 0x01DB, //#senHal_pContSenModesRegsArray[66][1] 2 700014E4
    0x0F12, 0x01E8, //#senHal_pContSenModesRegsArray[66][2] 2 700014E6
    0x0F12, 0x00FA, //#senHal_pContSenModesRegsArray[66][3] 2 700014E8
    0x0F12, 0x03B7, //#senHal_pContSenModesRegsArray[67][0] 2 700014EA
    0x0F12, 0x01E8, //#senHal_pContSenModesRegsArray[67][1] 2 700014EC
    0x0F12, 0x01F5, //#senHal_pContSenModesRegsArray[67][2] 2 700014EE
    0x0F12, 0x0107, //#senHal_pContSenModesRegsArray[67][3] 2 700014F0
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[68][0] 2 700014F2
    0x0F12, 0x0469, //#senHal_pContSenModesRegsArray[68][1] 2 700014F4
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[68][2] 2 700014F6
    0x0F12, 0x02A7, //#senHal_pContSenModesRegsArray[68][3] 2 700014F8
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[69][0] 2 700014FA
    0x0F12, 0x0476, //#senHal_pContSenModesRegsArray[69][1] 2 700014FC
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[69][2] 2 700014FE
    0x0F12, 0x02B4, //#senHal_pContSenModesRegsArray[69][3] 2 70001500
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[70][0] 2 70001502
    0x0F12, 0x0644, //#senHal_pContSenModesRegsArray[70][1] 2 70001504
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[70][2] 2 70001506
    0x0F12, 0x03A1, //#senHal_pContSenModesRegsArray[70][3] 2 70001508
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[71][0] 2 7000150A
    0x0F12, 0x0651, //#senHal_pContSenModesRegsArray[71][1] 2 7000150C
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[71][2] 2 7000150E
    0x0F12, 0x03AE, //#senHal_pContSenModesRegsArray[71][3] 2 70001510
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[72][0] 2 70001512
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[72][1] 2 70001514
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[72][2] 2 70001516
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[72][3] 2 70001518
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[73][0] 2 7000151A
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[73][1] 2 7000151C
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[73][2] 2 7000151E
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[73][3] 2 70001520
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[74][0] 2 70001522
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[74][1] 2 70001524
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[74][2] 2 70001526
    0x0F12, 0x0001, //#senHal_pContSenModesRegsArray[74][3] 2 70001528
    0x0F12, 0x000F, //#senHal_pContSenModesRegsArray[75][0] 2 7000152A
    0x0F12, 0x000F, //#senHal_pContSenModesRegsArray[75][1] 2 7000152C
    0x0F12, 0x000F, //#senHal_pContSenModesRegsArray[75][2] 2 7000152E
    0x0F12, 0x000F, //#senHal_pContSenModesRegsArray[75][3] 2 70001530
    0x0F12, 0x05AD, //#senHal_pContSenModesRegsArray[76][0] 2 70001532
    0x0F12, 0x03DE, //#senHal_pContSenModesRegsArray[76][1] 2 70001534
    0x0F12, 0x030A, //#senHal_pContSenModesRegsArray[76][2] 2 70001536
    0x0F12, 0x021C, //#senHal_pContSenModesRegsArray[76][3] 2 70001538
    0x0F12, 0x062F, //#senHal_pContSenModesRegsArray[77][0] 2 7000153A
    0x0F12, 0x0460, //#senHal_pContSenModesRegsArray[77][1] 2 7000153C
    0x0F12, 0x038C, //#senHal_pContSenModesRegsArray[77][2] 2 7000153E
    0x0F12, 0x029E, //#senHal_pContSenModesRegsArray[77][3] 2 70001540
    0x0F12, 0x07FC, //#senHal_pContSenModesRegsArray[78][0] 2 70001542
    0x0F12, 0x0847, //#senHal_pContSenModesRegsArray[78][1] 2 70001544
    0x0F12, 0x0478, //#senHal_pContSenModesRegsArray[78][2] 2 70001546
    0x0F12, 0x04C3, //#senHal_pContSenModesRegsArray[78][3] 2 70001548
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[79][0] 2 7000154A
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[79][1] 2 7000154C
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[79][2] 2 7000154E
    0x0F12, 0x0000, //#senHal_pContSenModesRegsArray[79][3] 2 70001550
    //============================================================
    // Analog Setting END
    //============================================================
};

static unsigned short s5k5cagx_init_reg_short3[]={
    //============================================================
    // AF Interface setting
    //============================================================
    0x002A, 0x01D4,
    0x0F12, 0x0000, //REG_TC_IPRM_AuxGpios : 0 - no Flash
    0x002A, 0x01DE,
    0x0F12, 0x0003, //REG_TC_IPRM_CM_Init_AfModeType : 3 - AFD_VCM_I2C
    0x0F12, 0x0000, //REG_TC_IPRM_CM_Init_PwmConfig1 : 0 - no PWM
    0x002A, 0x01E4,
    0x0F12, 0x0000, //REG_TC_IPRM_CM_Init_GpioConfig1 : 0 - no GPIO
    0x002A, 0x01E8,
    0x0F12, 0x2A0C, //REG_TC_IPRM_CM_Init_Mi2cBits : MSCL - GPIO1 MSDA - GPIO2 Device ID (0C)
    0x0F12, 0x0190, //REG_TC_IPRM_CM_Init_Mi2cRateKhz : MI2C Speed - 400KHz

    //#============================================================
    //#AF Parameter setting
    //#============================================================
    //#AF Window Settings
    0x002A, 0x025A,
    0x0F12, 0x0100, //#REG_TC_AF_FstWinStartX //#2nd : 1200-900(OUT)_640-480(IN) window setting
    0x0F12, 0x0110, //#REG_TC_AF_FstWinStartY
    0x0F12, 0x0200, //#REG_TC_AF_FstWinSizeX 512
    0x0F12, 0x0238, //#REG_TC_AF_FstWinSizeY
    0x0F12, 0x018C, //#REG_TC_AF_ScndWinStartX
    0x0F12, 0x01C0, //#REG_TC_AF_ScndWinStartY
    0x0F12, 0x00E6, //#REG_TC_AF_ScndWinSizeX
    0x0F12, 0x0132, //#REG_TC_AF_ScndWinSizeY
    0x0F12, 0x0001, //#REG_TC_AF_WinSizesUpdated

    //#AF Setot Settings
    0x002A, 0x0586,
    0x0F12, 0x00FF, //#skl_af_StatOvlpExpFactor

    //#AF Scene Settings
    0x002A, 0x115E,
    0x0F12, 0x0003, //#af_scene_usSaturatedScene

    //#AF Fine Search Settings
    0x002A, 0x10D4,
    0x0F12, 0x1000, //#FineSearch Disable //#af_search_usSingleAfFlags
    0x002A, 0x10DE,
    0x0F12, 0x0004, //#af_search_usFinePeakCount
    0x002A, 0x106C,
    0x0F12, 0x0202, //#af_pos_usFineStepNumSize

    //#AF Peak Threshold Setting
    0x002A, 0x10CA, //#af_search_usPeakThr
    0x0F12, 0x00C0,

    //#AF Default Position
    0x002A, 0x1060,
    0x0F12, 0x0000, //#af_pos_usHomePos
    0x0F12, 0xB300, //#af_pos_usLowConfPos

    //#AF LowConfThr Setting
    0x002A, 0x10F4, //#LowEdgeBoth GRAD
    0x0F12, 0x0280,
    0x002A, 0x1100, //#LowLight HPF
    0x0F12, 0x03A0,
    0x0F12, 0x0320,

    0x002A, 0x1134,
    0x0F12, 0x0030, //#af_stat_usMinStatVal

    //#AF low Br Th
    0x002A, 0x1154, //#normBrThr
    0x0F12, 0x0060,

    //#AF Policy
    0x002A, 0x10E2,
    0x0F12, 0x0000, //#af_search_usCapturePolicy: Focus_Priority, 0002 : Shutter_Priority_Fixed, 0001 : Shutter_Priority_Last_BFP 0000: Shutter_Priority_Current
    0x002A, 0x1072,
    0x0F12, 0x003C, //#af_pos_usCaptureFixedPos//#0x0008

    //#AF Lens Position Table Settings
    0x002A, 0x1074,
    0x0F12, 0x0013, //0015 //#af_pos_usTableLastInd//#22 Steps
    0x0F12, 0x002C, //002E //#af_pos_usTable_0_//#af_pos_usTable
    0x0F12, 0x0030, //0030 //#af_pos_usTable_1_
    0x0F12, 0x0033, //0033 //#af_pos_usTable_2_
    0x0F12, 0x0036, //0036 //#af_pos_usTable_3_
    0x0F12, 0x0039, //0039 //#af_pos_usTable_4_
    0x0F12, 0x003D, //003C //#af_pos_usTable_5_
    0x0F12, 0x0041, //003F //#af_pos_usTable_6_
    0x0F12, 0x0045, //0042 //#af_pos_usTable_7_
    0x0F12, 0x004F, //0045 //#af_pos_usTable_8_
    0x0F12, 0x0055, //0048 //#af_pos_usTable_9_
    0x0F12, 0x005D, //004B //#af_pos_usTable_10_
    0x0F12, 0x0065, //004E //#af_pos_usTable_11_
    0x0F12, 0x006D, //0051 //#af_pos_usTable_12_
    0x0F12, 0x0077, //0054 //#af_pos_usTable_13_
    0x0F12, 0x0083, //0057 //#af_pos_usTable_14_
    0x0F12, 0x008F, //005A //#af_pos_usTable_15_
    0x0F12, 0x009B, //005E //#af_pos_usTable_16_
    0x0F12, 0x00A7,
    0x0F12, 0x00B3,

    0x002A, 0x1198,
    0x0F12, 0x8000,
    0x0F12, 0x0006,
    0x0F12, 0x3FF0,
    0x0F12, 0x03E8,
    0x0F12, 0x0000,
    0x0F12, 0x0020,
    0x0F12, 0x0010,
    0x0F12, 0x0008,
    0x0F12, 0x0040,
    0x0F12, 0x0080,
    0x0F12, 0x00C0,
    0x0F12, 0x00E0,

    0x002A, 0x0252,
    0x0F12, 0x0003, //#init  //AF

    0x002A, 0x12B8, // disable CINTR 0
    0x0F12, 0x1000,

    //============================================================
    // ISP-FE Setting
    //============================================================
    0x002A, 0x158A,
    0x0F12, 0xEAF0,
    0x002A, 0x15C6,
    0x0F12, 0x0020,
    0x0F12, 0x0060,
    0x002A, 0x15BC,
    0x0F12, 0x0200,

    //Analog Offset for MSM
    0x002A, 0x1608,
    0x0F12, 0x0100, //#gisp_msm_sAnalogOffset[0]
    0x0F12, 0x0100, //#gisp_msm_sAnalogOffset[1]
    0x0F12, 0x0100, //#gisp_msm_sAnalogOffset[2]
    0x0F12, 0x0100, //#gisp_msm_sAnalogOffset[3]

    //============================================================
    // ISP-FE Setting END
    //============================================================

    //============================================================
    // Frame rate setting
    //============================================================
    // How to set
    // 1. Exposure value
    // dec2hex((1 / (frame rate you want(ms))) * 100d * 4d)
    // 2. Analog Digital gain
    // dec2hex((Analog gain you want) * 256d)
    //============================================================
    // Set preview exposure time
    0x002A, 0x0530,
    0x0F12, 0x3E80, //#lt_uMaxExp1 40ms
    0x0F12, 0x0000,
    0x0F12, 0x6D60, //#lt_uMaxExp2 70ms
    0x0F12, 0x0000,
    0x002A, 0x167C,
    0x0F12, 0x8CA0, //#evt1_lt_uMaxExp3 90ms
    0x0F12, 0x0000,
    0x0F12, 0x9C40, //#evt1_lt_uMaxExp4 100ms
    0x0F12, 0x0000,

    // Set capture exposure time
    0x002A, 0x0538,
    0x0F12, 0x3E80, //#lt_uCapMaxExp1 40ms
    0x0F12, 0x0000,
    0x0F12, 0x6D60, //#lt_uCapMaxExp2 70ms
    0x0F12, 0x0000,
    0x002A, 0x1684,
    0x0F12, 0x8CA0, //#evt1_lt_uCapMaxExp3 90ms
    0x0F12, 0x0000,
    0x0F12, 0x9C40, //#evt1_lt_uCapMaxExp4 100ms
    0x0F12, 0x0000,

    // Set gain
    0x002A, 0x0540,
    0x0F12, 0x0150, //#lt_uMaxAnGain1
    0x0F12, 0x0280, //#lt_uMaxAnGain2
    0x002A, 0x168C,
    0x0F12, 0x02A0, //#evt1_lt_uMaxAnGain3
    0x0F12, 0x0800, //#evt1_lt_uMaxAnGain4
    0x002A, 0x0544,
    0x0F12, 0x0100, //#lt_uMaxDigGain
    0x0F12, 0x0A00, //#lt_uMaxTotGain

    0x002A, 0x1694,
    0x0F12, 0x0001, //#evt1_senHal_bExpandForbid

    0x002A, 0x051A,
    0x0F12, 0x0111, //#lt_uLimitHigh
    0x0F12, 0x00F0, //#lt_uLimitLow


    //================================================================================================
    // SET AE
    //================================================================================================
    // AE target
    0x002A, 0x0F70,
    0x0F12, 0x003B, //#TVAR_ae_BrAve 091222
    // AE mode
    0x002A, 0x0F76,
    0x0F12, 0x000F, //Disable illumination & contrast// #ae_StatMode
    // AE weight
    0x002A, 0x0F7E,
    0x0F12, 0x0101, //#ae_WeightTbl_16_0_
    0x0F12, 0x0101, //#ae_WeightTbl_16_1_
    0x0F12, 0x0101, //#ae_WeightTbl_16_2_
    0x0F12, 0x0101, //#ae_WeightTbl_16_3_
    0x0F12, 0x0101, //#ae_WeightTbl_16_4_
    0x0F12, 0x0101, //#ae_WeightTbl_16_5_
    0x0F12, 0x0101, //#ae_WeightTbl_16_6_
    0x0F12, 0x0101, //#ae_WeightTbl_16_7_
    0x0F12, 0x0101, //#ae_WeightTbl_16_8_
    0x0F12, 0x0303, //#ae_WeightTbl_16_9_
    0x0F12, 0x0303, //#ae_WeightTbl_16_10
    0x0F12, 0x0101, //#ae_WeightTbl_16_11
    0x0F12, 0x0101, //#ae_WeightTbl_16_12
    0x0F12, 0x0303, //#ae_WeightTbl_16_13
    0x0F12, 0x0303, //#ae_WeightTbl_16_14
    0x0F12, 0x0101, //#ae_WeightTbl_16_15
    0x0F12, 0x0101, //#ae_WeightTbl_16_16
    0x0F12, 0x0303, //#ae_WeightTbl_16_17
    0x0F12, 0x0303, //#ae_WeightTbl_16_18
    0x0F12, 0x0101, //#ae_WeightTbl_16_19
    0x0F12, 0x0101, //#ae_WeightTbl_16_20
    0x0F12, 0x0303, //#ae_WeightTbl_16_21
    0x0F12, 0x0303, //#ae_WeightTbl_16_22
    0x0F12, 0x0101, //#ae_WeightTbl_16_23
    0x0F12, 0x0101, //#ae_WeightTbl_16_24
    0x0F12, 0x0101, //#ae_WeightTbl_16_25
    0x0F12, 0x0101, //#ae_WeightTbl_16_26
    0x0F12, 0x0101, //#ae_WeightTbl_16_27
    0x0F12, 0x0101, //#ae_WeightTbl_16_28
    0x0F12, 0x0101, //#ae_WeightTbl_16_29
    0x0F12, 0x0101, //#ae_WeightTbl_16_30
    0x0F12, 0x0101, //#ae_WeightTbl_16_31

    //================================================================================================
    // SET FLICKER
    //================================================================================================
    0x002A, 0x04D2,
    0x0F12, 0x065F, //#REG_TC_DBG_AutoAlgEnBits
    0x002A, 0x04BA,
    0x0F12, 0x0002, //#REG_SF_USER_FlickerQuant
    0x0F12, 0x0001, //#REG_SF_USER_FlickerQuantChanged
    //#================================================================================================
    //#SETGAS
    //#================================================================================================
    //#GASalpha
    //#RGrGbBperlightsource
    0x002A, 0x06CE,
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[0][0] Hor
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[0][1]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[0][2]
    0x0F12, 0x00E0, //#TVAR_ash_GASAlpha[0][3]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[1][0] Inca
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[1][1]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[1][2]
    0x0F12, 0x00E0, //#TVAR_ash_GASAlpha[1][3]
    0x0F12, 0x0110, //#TVAR_ash_GASAlpha[2][0] CWF, TL84
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[2][1]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[2][2]
    0x0F12, 0x00F0, //#TVAR_ash_GASAlpha[2][3]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[3][0] Indoor
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[3][1]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[3][2]
    0x0F12, 0x00F0, //#TVAR_ash_GASAlpha[3][3]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[4][0] D50,DNP
    0x0F12, 0x0101, //#TVAR_ash_GASAlpha[4][1]
    0x0F12, 0x0101, //#TVAR_ash_GASAlpha[4][2]
    0x0F12, 0x00E0, //#TVAR_ash_GASAlpha[4][3]
    0x0F12, 0x0118, //#TVAR_ash_GASAlpha[5][0] D65
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[5][1]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[5][2]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[5][3]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[6][0] D75
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[6][1]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[6][2]
    0x0F12, 0x0100, //#TVAR_ash_GASAlpha[6][3]
    0x0F12, 0x0100, //#TVAR_ash_GASOutdoorAlpha[0]//#Outdoor //##Only, applied when 0x06B4=[1]
    0x0F12, 0x0100, //#TVAR_ash_GASOutdoorAlpha[1]
    0x0F12, 0x0100, //#TVAR_ash_GASOutdoorAlpha[2]
    0x0F12, 0x0100, //#TVAR_ash_GASOutdoorAlpha[3]
    //#GASbeta
    0x0F12, 0x0000, //#ash_GASBeta[0]//#Horizon
    0x0F12, 0x0000, //#ash_GASBeta[1]
    0x0F12, 0x0000, //#ash_GASBeta[2]
    0x0F12, 0x0000, //#ash_GASBeta[3]
    0x0F12, 0x0000, //#ash_GASBeta[4]//#IncandA
    0x0F12, 0x0000, //#ash_GASBeta[5]
    0x0F12, 0x0000, //#ash_GASBeta[6]
    0x0F12, 0x0000, //#ash_GASBeta[7]
    0x0F12, 0x0000, //#ash_GASBeta[8]//#CWF, TL84
    0x0F12, 0x0000, //#ash_GASBeta[9]
    0x0F12, 0x0000, //#ash_GASBeta[10]
    0x0F12, 0x0000, //#ash_GASBeta[11]
    0x0F12, 0x0000, //#ash_GASBeta[12]//#Indoor
    0x0F12, 0x0000, //#ash_GASBeta[13]
    0x0F12, 0x0000, //#ash_GASBeta[14]
    0x0F12, 0x0000, //#ash_GASBeta[15]
    0x0F12, 0x0000, //#ash_GASBeta[16]//#D50,DNP
    0x0F12, 0x0000, //#ash_GASBeta[17]
    0x0F12, 0x0000, //#ash_GASBeta[18]
    0x0F12, 0x0000, //#ash_GASBeta[19]
    0x0F12, 0x0000, //#ash_GASBeta[20]//#D65
    0x0F12, 0x0000, //#ash_GASBeta[21]
    0x0F12, 0x0000, //#ash_GASBeta[22]
    0x0F12, 0x0000, //#ash_GASBeta[23]
    0x0F12, 0x0000, //#ash_GASBeta[24]//#D75
    0x0F12, 0x0000, //#ash_GASBeta[25]
    0x0F12, 0x0000, //#ash_GASBeta[26]
    0x0F12, 0x0000, //#ash_GASBeta[27]
    0x0F12, 0x0000, //#ash_GASOutdoorBeta[0]//#Outdoor
    0x0F12, 0x0000, //#ash_GASOutdoorBeta[1]
    0x0F12, 0x0000, //#ash_GASOutdoorBeta[2]
    0x0F12, 0x0000, //#ash_GASOutdoorBeta[3]
    0x002A, 0x06B4,
    0x0F12, 0x0001, //#wbt_bUseOutdoorASHON:1OFF:0 //##Outdoor GAS_Alpha Enable[1] / Disable[0]
    //#Parabloicfunction
    0x002A, 0x075A,
    0x0F12, 0x0001, //#ash_bParabolicEstimation
    0x0F12, 0x0400, //#ash_uParabolicCenterX
    0x0F12, 0x0300, //#ash_uParabolicCenterY
    0x0F12, 0x0010, //#ash_uParabolicScalingA
    0x0F12, 0x0011, //#ash_uParabolicScalingB
    0x002A, 0x06C6,
    0x0F12, 0x0100, //#ash_CGrasAlphas_0_
    0x0F12, 0x0100, //#ash_CGrasAlphas_1_
    0x0F12, 0x0100, //#ash_CGrasAlphas_2_
    0x0F12, 0x0100, //#ash_CGrasAlphas_3_
    0x002A, 0x0E3C,
    0x0F12, 0x00C0, //#awbb_Alpha_Comp_Mode
    0x002A, 0x074E,
    0x0F12, 0x0000, //#ash_bLumaMode//#useBeta:0001notuseBeta:0000
    //#GASLUTstartaddress//#7000_347C
    0x002A, 0x0754,
    0x0F12, 0x347C,
    0x0F12, 0x7000,
    //#GASLUT
    0x002A, 0x347C,
    0x0F12, 0x01C6, //#TVAR_ash_pGAS[0]
    0x0F12, 0x0190, //#TVAR_ash_pGAS[1]
    0x0F12, 0x0151, //#TVAR_ash_pGAS[2]
    0x0F12, 0x0119, //#TVAR_ash_pGAS[3]
    0x0F12, 0x00EC, //#TVAR_ash_pGAS[4]
    0x0F12, 0x00D4, //#TVAR_ash_pGAS[5]
    0x0F12, 0x00CA, //#TVAR_ash_pGAS[6]
    0x0F12, 0x00D0, //#TVAR_ash_pGAS[7]
    0x0F12, 0x00E5, //#TVAR_ash_pGAS[8]
    0x0F12, 0x010B, //#TVAR_ash_pGAS[9]
    0x0F12, 0x013F, //#TVAR_ash_pGAS[10]
    0x0F12, 0x0179, //#TVAR_ash_pGAS[11]
    0x0F12, 0x01AC, //#TVAR_ash_pGAS[12]
    0x0F12, 0x01AB, //#TVAR_ash_pGAS[13]
    0x0F12, 0x016E, //#TVAR_ash_pGAS[14]
    0x0F12, 0x0123, //#TVAR_ash_pGAS[15]
    0x0F12, 0x00E3, //#TVAR_ash_pGAS[16]
    0x0F12, 0x00B5, //#TVAR_ash_pGAS[17]
    0x0F12, 0x0099, //#TVAR_ash_pGAS[18]
    0x0F12, 0x008F, //#TVAR_ash_pGAS[19]
    0x0F12, 0x0096, //#TVAR_ash_pGAS[20]
    0x0F12, 0x00AD, //#TVAR_ash_pGAS[21]
    0x0F12, 0x00D5, //#TVAR_ash_pGAS[22]
    0x0F12, 0x0110, //#TVAR_ash_pGAS[23]
    0x0F12, 0x0154, //#TVAR_ash_pGAS[24]
    0x0F12, 0x018C, //#TVAR_ash_pGAS[25]
    0x0F12, 0x017A, //#TVAR_ash_pGAS[26]
    0x0F12, 0x0135, //#TVAR_ash_pGAS[27]
    0x0F12, 0x00E3, //#TVAR_ash_pGAS[28]
    0x0F12, 0x00A1, //#TVAR_ash_pGAS[29]
    0x0F12, 0x0073, //#TVAR_ash_pGAS[30]
    0x0F12, 0x0058, //#TVAR_ash_pGAS[31]
    0x0F12, 0x004E, //#TVAR_ash_pGAS[32]
    0x0F12, 0x0055, //#TVAR_ash_pGAS[33]
    0x0F12, 0x006C, //#TVAR_ash_pGAS[34]
    0x0F12, 0x0096, //#TVAR_ash_pGAS[35]
    0x0F12, 0x00D0, //#TVAR_ash_pGAS[36]
    0x0F12, 0x011A, //#TVAR_ash_pGAS[37]
    0x0F12, 0x0160, //#TVAR_ash_pGAS[38]
    0x0F12, 0x0154, //#TVAR_ash_pGAS[39]
    0x0F12, 0x0108, //#TVAR_ash_pGAS[40]
    0x0F12, 0x00B6, //#TVAR_ash_pGAS[41]
    0x0F12, 0x0074, //#TVAR_ash_pGAS[42]
    0x0F12, 0x0047, //#TVAR_ash_pGAS[43]
    0x0F12, 0x002D, //#TVAR_ash_pGAS[44]
    0x0F12, 0x0023, //#TVAR_ash_pGAS[45]
    0x0F12, 0x002A, //#TVAR_ash_pGAS[46]
    0x0F12, 0x0042, //#TVAR_ash_pGAS[47]
    0x0F12, 0x0069, //#TVAR_ash_pGAS[48]
    0x0F12, 0x00A3, //#TVAR_ash_pGAS[49]
    0x0F12, 0x00F1, //#TVAR_ash_pGAS[50]
    0x0F12, 0x013C, //#TVAR_ash_pGAS[51]
    0x0F12, 0x0138, //#TVAR_ash_pGAS[52]
    0x0F12, 0x00EA, //#TVAR_ash_pGAS[53]
    0x0F12, 0x0097, //#TVAR_ash_pGAS[54]
    0x0F12, 0x0057, //#TVAR_ash_pGAS[55]
    0x0F12, 0x002C, //#TVAR_ash_pGAS[56]
    0x0F12, 0x0012, //#TVAR_ash_pGAS[57]
    0x0F12, 0x0009, //#TVAR_ash_pGAS[58]
    0x0F12, 0x0010, //#TVAR_ash_pGAS[59]
    0x0F12, 0x0027, //#TVAR_ash_pGAS[60]
    0x0F12, 0x004F, //#TVAR_ash_pGAS[61]
    0x0F12, 0x0088, //#TVAR_ash_pGAS[62]
    0x0F12, 0x00D7, //#TVAR_ash_pGAS[63]
    0x0F12, 0x0125, //#TVAR_ash_pGAS[64]
    0x0F12, 0x012C, //#TVAR_ash_pGAS[65]
    0x0F12, 0x00DE, //#TVAR_ash_pGAS[66]
    0x0F12, 0x008A, //#TVAR_ash_pGAS[67]
    0x0F12, 0x004C, //#TVAR_ash_pGAS[68]
    0x0F12, 0x0021, //#TVAR_ash_pGAS[69]
    0x0F12, 0x0008, //#TVAR_ash_pGAS[70]
    0x0F12, 0x0000, //#TVAR_ash_pGAS[71]
    0x0F12, 0x0007, //#TVAR_ash_pGAS[72]
    0x0F12, 0x001E, //#TVAR_ash_pGAS[73]
    0x0F12, 0x0045, //#TVAR_ash_pGAS[74]
    0x0F12, 0x007F, //#TVAR_ash_pGAS[75]
    0x0F12, 0x00CE, //#TVAR_ash_pGAS[76]
    0x0F12, 0x011C, //#TVAR_ash_pGAS[77]
    0x0F12, 0x012B, //#TVAR_ash_pGAS[78]
    0x0F12, 0x00DF, //#TVAR_ash_pGAS[79]
    0x0F12, 0x008E, //#TVAR_ash_pGAS[80]
    0x0F12, 0x0051, //#TVAR_ash_pGAS[81]
    0x0F12, 0x0027, //#TVAR_ash_pGAS[82]
    0x0F12, 0x000F, //#TVAR_ash_pGAS[83]
    0x0F12, 0x0007, //#TVAR_ash_pGAS[84]
    0x0F12, 0x000F, //#TVAR_ash_pGAS[85]
    0x0F12, 0x0025, //#TVAR_ash_pGAS[86]
    0x0F12, 0x004D, //#TVAR_ash_pGAS[87]
    0x0F12, 0x0087, //#TVAR_ash_pGAS[88]
    0x0F12, 0x00D5, //#TVAR_ash_pGAS[89]
    0x0F12, 0x0123, //#TVAR_ash_pGAS[90]
    0x0F12, 0x013B, //#TVAR_ash_pGAS[91]
    0x0F12, 0x00F0, //#TVAR_ash_pGAS[92]
    0x0F12, 0x00A3, //#TVAR_ash_pGAS[93]
    0x0F12, 0x0067, //#TVAR_ash_pGAS[94]
    0x0F12, 0x003D, //#TVAR_ash_pGAS[95]
    0x0F12, 0x0025, //#TVAR_ash_pGAS[96]
    0x0F12, 0x001E, //#TVAR_ash_pGAS[97]
    0x0F12, 0x0027, //#TVAR_ash_pGAS[98]
    0x0F12, 0x003E, //#TVAR_ash_pGAS[99]
    0x0F12, 0x0066, //#TVAR_ash_pGAS[100]
    0x0F12, 0x00A1, //#TVAR_ash_pGAS[101]
    0x0F12, 0x00ED, //#TVAR_ash_pGAS[102]
    0x0F12, 0x0136, //#TVAR_ash_pGAS[103]
    0x0F12, 0x0158, //#TVAR_ash_pGAS[104]
    0x0F12, 0x0113, //#TVAR_ash_pGAS[105]
    0x0F12, 0x00C9, //#TVAR_ash_pGAS[106]
    0x0F12, 0x008F, //#TVAR_ash_pGAS[107]
    0x0F12, 0x0064, //#TVAR_ash_pGAS[108]
    0x0F12, 0x004D, //#TVAR_ash_pGAS[109]
    0x0F12, 0x0046, //#TVAR_ash_pGAS[110]
    0x0F12, 0x0050, //#TVAR_ash_pGAS[111]
    0x0F12, 0x0067, //#TVAR_ash_pGAS[112]
    0x0F12, 0x0091, //#TVAR_ash_pGAS[113]
    0x0F12, 0x00CA, //#TVAR_ash_pGAS[114]
    0x0F12, 0x0114, //#TVAR_ash_pGAS[115]
    0x0F12, 0x0157, //#TVAR_ash_pGAS[116]
    0x0F12, 0x017C, //#TVAR_ash_pGAS[117]
    0x0F12, 0x0146, //#TVAR_ash_pGAS[118]
    0x0F12, 0x0100, //#TVAR_ash_pGAS[119]
    0x0F12, 0x00C7, //#TVAR_ash_pGAS[120]
    0x0F12, 0x009E, //#TVAR_ash_pGAS[121]
    0x0F12, 0x0087, //#TVAR_ash_pGAS[122]
    0x0F12, 0x0082, //#TVAR_ash_pGAS[123]
    0x0F12, 0x008C, //#TVAR_ash_pGAS[124]
    0x0F12, 0x00A5, //#TVAR_ash_pGAS[125]
    0x0F12, 0x00CF, //#TVAR_ash_pGAS[126]
    0x0F12, 0x0106, //#TVAR_ash_pGAS[127]
    0x0F12, 0x014B, //#TVAR_ash_pGAS[128]
    0x0F12, 0x0184, //#TVAR_ash_pGAS[129]
    0x0F12, 0x019B, //#TVAR_ash_pGAS[130]
    0x0F12, 0x016C, //#TVAR_ash_pGAS[131]
    0x0F12, 0x0132, //#TVAR_ash_pGAS[132]
    0x0F12, 0x00FD, //#TVAR_ash_pGAS[133]
    0x0F12, 0x00D9, //#TVAR_ash_pGAS[134]
    0x0F12, 0x00C6, //#TVAR_ash_pGAS[135]
    0x0F12, 0x00C2, //#TVAR_ash_pGAS[136]
    0x0F12, 0x00CE, //#TVAR_ash_pGAS[137]
    0x0F12, 0x00E4, //#TVAR_ash_pGAS[138]
    0x0F12, 0x010B, //#TVAR_ash_pGAS[139]
    0x0F12, 0x013F, //#TVAR_ash_pGAS[140]
    0x0F12, 0x017A, //#TVAR_ash_pGAS[141]
    0x0F12, 0x01AF, //#TVAR_ash_pGAS[142]
    0x0F12, 0x0175, //#TVAR_ash_pGAS[143]
    0x0F12, 0x0146, //#TVAR_ash_pGAS[144]
    0x0F12, 0x0118, //#TVAR_ash_pGAS[145]
    0x0F12, 0x00EA, //#TVAR_ash_pGAS[146]
    0x0F12, 0x00C6, //#TVAR_ash_pGAS[147]
    0x0F12, 0x00AF, //#TVAR_ash_pGAS[148]
    0x0F12, 0x00A5, //#TVAR_ash_pGAS[149]
    0x0F12, 0x00A8, //#TVAR_ash_pGAS[150]
    0x0F12, 0x00B9, //#TVAR_ash_pGAS[151]
    0x0F12, 0x00D7, //#TVAR_ash_pGAS[152]
    0x0F12, 0x0102, //#TVAR_ash_pGAS[153]
    0x0F12, 0x0131, //#TVAR_ash_pGAS[154]
    0x0F12, 0x015E, //#TVAR_ash_pGAS[155]
    0x0F12, 0x015F, //#TVAR_ash_pGAS[156]
    0x0F12, 0x012D, //#TVAR_ash_pGAS[157]
    0x0F12, 0x00F2, //#TVAR_ash_pGAS[158]
    0x0F12, 0x00BF, //#TVAR_ash_pGAS[159]
    0x0F12, 0x009A, //#TVAR_ash_pGAS[160]
    0x0F12, 0x0083, //#TVAR_ash_pGAS[161]
    0x0F12, 0x0079, //#TVAR_ash_pGAS[162]
    0x0F12, 0x007C, //#TVAR_ash_pGAS[163]
    0x0F12, 0x008D, //#TVAR_ash_pGAS[164]
    0x0F12, 0x00AD, //#TVAR_ash_pGAS[165]
    0x0F12, 0x00DE, //#TVAR_ash_pGAS[166]
    0x0F12, 0x0115, //#TVAR_ash_pGAS[167]
    0x0F12, 0x0146, //#TVAR_ash_pGAS[168]
    0x0F12, 0x0139, //#TVAR_ash_pGAS[169]
    0x0F12, 0x00FD, //#TVAR_ash_pGAS[170]
    0x0F12, 0x00BC, //#TVAR_ash_pGAS[171]
    0x0F12, 0x0089, //#TVAR_ash_pGAS[172]
    0x0F12, 0x0064, //#TVAR_ash_pGAS[173]
    0x0F12, 0x004E, //#TVAR_ash_pGAS[174]
    0x0F12, 0x0044, //#TVAR_ash_pGAS[175]
    0x0F12, 0x0047, //#TVAR_ash_pGAS[176]
    0x0F12, 0x0059, //#TVAR_ash_pGAS[177]
    0x0F12, 0x007A, //#TVAR_ash_pGAS[178]
    0x0F12, 0x00A9, //#TVAR_ash_pGAS[179]
    0x0F12, 0x00E5, //#TVAR_ash_pGAS[180]
    0x0F12, 0x0126, //#TVAR_ash_pGAS[181]
    0x0F12, 0x011A, //#TVAR_ash_pGAS[182]
    0x0F12, 0x00D7, //#TVAR_ash_pGAS[183]
    0x0F12, 0x0095, //#TVAR_ash_pGAS[184]
    0x0F12, 0x0063, //#TVAR_ash_pGAS[185]
    0x0F12, 0x003F, //#TVAR_ash_pGAS[186]
    0x0F12, 0x002A, //#TVAR_ash_pGAS[187]
    0x0F12, 0x0020, //#TVAR_ash_pGAS[188]
    0x0F12, 0x0024, //#TVAR_ash_pGAS[189]
    0x0F12, 0x0036, //#TVAR_ash_pGAS[190]
    0x0F12, 0x0057, //#TVAR_ash_pGAS[191]
    0x0F12, 0x0085, //#TVAR_ash_pGAS[192]
    0x0F12, 0x00C3, //#TVAR_ash_pGAS[193]
    0x0F12, 0x0108, //#TVAR_ash_pGAS[194]
    0x0F12, 0x0102, //#TVAR_ash_pGAS[195]
    0x0F12, 0x00BC, //#TVAR_ash_pGAS[196]
    0x0F12, 0x007B, //#TVAR_ash_pGAS[197]
    0x0F12, 0x004B, //#TVAR_ash_pGAS[198]
    0x0F12, 0x0028, //#TVAR_ash_pGAS[199]
    0x0F12, 0x0012, //#TVAR_ash_pGAS[200]
    0x0F12, 0x0009, //#TVAR_ash_pGAS[201]
    0x0F12, 0x000D, //#TVAR_ash_pGAS[202]
    0x0F12, 0x0020, //#TVAR_ash_pGAS[203]
    0x0F12, 0x0041, //#TVAR_ash_pGAS[204]
    0x0F12, 0x0070, //#TVAR_ash_pGAS[205]
    0x0F12, 0x00B0, //#TVAR_ash_pGAS[206]
    0x0F12, 0x00F6, //#TVAR_ash_pGAS[207]
    0x0F12, 0x00F4, //#TVAR_ash_pGAS[208]
    0x0F12, 0x00AF, //#TVAR_ash_pGAS[209]
    0x0F12, 0x006F, //#TVAR_ash_pGAS[210]
    0x0F12, 0x0040, //#TVAR_ash_pGAS[211]
    0x0F12, 0x001D, //#TVAR_ash_pGAS[212]
    0x0F12, 0x0008, //#TVAR_ash_pGAS[213]
    0x0F12, 0x0000, //#TVAR_ash_pGAS[214]
    0x0F12, 0x0005, //#TVAR_ash_pGAS[215]
    0x0F12, 0x0018, //#TVAR_ash_pGAS[216]
    0x0F12, 0x0039, //#TVAR_ash_pGAS[217]
    0x0F12, 0x0069, //#TVAR_ash_pGAS[218]
    0x0F12, 0x00AA, //#TVAR_ash_pGAS[219]
    0x0F12, 0x00F2, //#TVAR_ash_pGAS[220]
    0x0F12, 0x00F5, //#TVAR_ash_pGAS[221]
    0x0F12, 0x00B0, //#TVAR_ash_pGAS[222]
    0x0F12, 0x0071, //#TVAR_ash_pGAS[223]
    0x0F12, 0x0042, //#TVAR_ash_pGAS[224]
    0x0F12, 0x0020, //#TVAR_ash_pGAS[225]
    0x0F12, 0x000C, //#TVAR_ash_pGAS[226]
    0x0F12, 0x0005, //#TVAR_ash_pGAS[227]
    0x0F12, 0x000B, //#TVAR_ash_pGAS[228]
    0x0F12, 0x001F, //#TVAR_ash_pGAS[229]
    0x0F12, 0x0040, //#TVAR_ash_pGAS[230]
    0x0F12, 0x0070, //#TVAR_ash_pGAS[231]
    0x0F12, 0x00B2, //#TVAR_ash_pGAS[232]
    0x0F12, 0x00F8, //#TVAR_ash_pGAS[233]
    0x0F12, 0x0101, //#TVAR_ash_pGAS[234]
    0x0F12, 0x00BD, //#TVAR_ash_pGAS[235]
    0x0F12, 0x0080, //#TVAR_ash_pGAS[236]
    0x0F12, 0x0052, //#TVAR_ash_pGAS[237]
    0x0F12, 0x0031, //#TVAR_ash_pGAS[238]
    0x0F12, 0x001D, //#TVAR_ash_pGAS[239]
    0x0F12, 0x0016, //#TVAR_ash_pGAS[240]
    0x0F12, 0x001D, //#TVAR_ash_pGAS[241]
    0x0F12, 0x0032, //#TVAR_ash_pGAS[242]
    0x0F12, 0x0054, //#TVAR_ash_pGAS[243]
    0x0F12, 0x0085, //#TVAR_ash_pGAS[244]
    0x0F12, 0x00C6, //#TVAR_ash_pGAS[245]
    0x0F12, 0x010A, //#TVAR_ash_pGAS[246]
    0x0F12, 0x0115, //#TVAR_ash_pGAS[247]
    0x0F12, 0x00D6, //#TVAR_ash_pGAS[248]
    0x0F12, 0x009C, //#TVAR_ash_pGAS[249]
    0x0F12, 0x006F, //#TVAR_ash_pGAS[250]
    0x0F12, 0x004E, //#TVAR_ash_pGAS[251]
    0x0F12, 0x003B, //#TVAR_ash_pGAS[252]
    0x0F12, 0x0035, //#TVAR_ash_pGAS[253]
    0x0F12, 0x003C, //#TVAR_ash_pGAS[254]
    0x0F12, 0x0051, //#TVAR_ash_pGAS[255]
    0x0F12, 0x0074, //#TVAR_ash_pGAS[256]
    0x0F12, 0x00A4, //#TVAR_ash_pGAS[257]
    0x0F12, 0x00E3, //#TVAR_ash_pGAS[258]
    0x0F12, 0x0125, //#TVAR_ash_pGAS[259]
    0x0F12, 0x012E, //#TVAR_ash_pGAS[260]
    0x0F12, 0x00FA, //#TVAR_ash_pGAS[261]
    0x0F12, 0x00C3, //#TVAR_ash_pGAS[262]
    0x0F12, 0x0097, //#TVAR_ash_pGAS[263]
    0x0F12, 0x0078, //#TVAR_ash_pGAS[264]
    0x0F12, 0x0066, //#TVAR_ash_pGAS[265]
    0x0F12, 0x0061, //#TVAR_ash_pGAS[266]
    0x0F12, 0x0069, //#TVAR_ash_pGAS[267]
    0x0F12, 0x007E, //#TVAR_ash_pGAS[268]
    0x0F12, 0x00A1, //#TVAR_ash_pGAS[269]
    0x0F12, 0x00D1, //#TVAR_ash_pGAS[270]
    0x0F12, 0x010B, //#TVAR_ash_pGAS[271]
    0x0F12, 0x0141, //#TVAR_ash_pGAS[272]
    0x0F12, 0x013E, //#TVAR_ash_pGAS[273]
    0x0F12, 0x0117, //#TVAR_ash_pGAS[274]
    0x0F12, 0x00E9, //#TVAR_ash_pGAS[275]
    0x0F12, 0x00BF, //#TVAR_ash_pGAS[276]
    0x0F12, 0x00A3, //#TVAR_ash_pGAS[277]
    0x0F12, 0x0094, //#TVAR_ash_pGAS[278]
    0x0F12, 0x008F, //#TVAR_ash_pGAS[279]
    0x0F12, 0x0099, //#TVAR_ash_pGAS[280]
    0x0F12, 0x00AF, //#TVAR_ash_pGAS[281]
    0x0F12, 0x00D1, //#TVAR_ash_pGAS[282]
    0x0F12, 0x00FD, //#TVAR_ash_pGAS[283]
    0x0F12, 0x012E, //#TVAR_ash_pGAS[284]
    0x0F12, 0x0160, //#TVAR_ash_pGAS[285]
    0x0F12, 0x0161, //#TVAR_ash_pGAS[286]
    0x0F12, 0x012F, //#TVAR_ash_pGAS[287]
    0x0F12, 0x0102, //#TVAR_ash_pGAS[288]
    0x0F12, 0x00DA, //#TVAR_ash_pGAS[289]
    0x0F12, 0x00BB, //#TVAR_ash_pGAS[290]
    0x0F12, 0x00A9, //#TVAR_ash_pGAS[291]
    0x0F12, 0x00A6, //#TVAR_ash_pGAS[292]
    0x0F12, 0x00B0, //#TVAR_ash_pGAS[293]
    0x0F12, 0x00C7, //#TVAR_ash_pGAS[294]
    0x0F12, 0x00E9, //#TVAR_ash_pGAS[295]
    0x0F12, 0x0114, //#TVAR_ash_pGAS[296]
    0x0F12, 0x0143, //#TVAR_ash_pGAS[297]
    0x0F12, 0x0179, //#TVAR_ash_pGAS[298]
    0x0F12, 0x0149, //#TVAR_ash_pGAS[299]
    0x0F12, 0x0113, //#TVAR_ash_pGAS[300]
    0x0F12, 0x00DE, //#TVAR_ash_pGAS[301]
    0x0F12, 0x00B0, //#TVAR_ash_pGAS[302]
    0x0F12, 0x0090, //#TVAR_ash_pGAS[303]
    0x0F12, 0x007C, //#TVAR_ash_pGAS[304]
    0x0F12, 0x0077, //#TVAR_ash_pGAS[305]
    0x0F12, 0x007F, //#TVAR_ash_pGAS[306]
    0x0F12, 0x0097, //#TVAR_ash_pGAS[307]
    0x0F12, 0x00BB, //#TVAR_ash_pGAS[308]
    0x0F12, 0x00EC, //#TVAR_ash_pGAS[309]
    0x0F12, 0x0122, //#TVAR_ash_pGAS[310]
    0x0F12, 0x0158, //#TVAR_ash_pGAS[311]
    0x0F12, 0x0127, //#TVAR_ash_pGAS[312]
    0x0F12, 0x00E9, //#TVAR_ash_pGAS[313]
    0x0F12, 0x00AD, //#TVAR_ash_pGAS[314]
    0x0F12, 0x007D, //#TVAR_ash_pGAS[315]
    0x0F12, 0x005C, //#TVAR_ash_pGAS[316]
    0x0F12, 0x0048, //#TVAR_ash_pGAS[317]
    0x0F12, 0x0042, //#TVAR_ash_pGAS[318]
    0x0F12, 0x004A, //#TVAR_ash_pGAS[319]
    0x0F12, 0x0061, //#TVAR_ash_pGAS[320]
    0x0F12, 0x0085, //#TVAR_ash_pGAS[321]
    0x0F12, 0x00B6, //#TVAR_ash_pGAS[322]
    0x0F12, 0x00F3, //#TVAR_ash_pGAS[323]
    0x0F12, 0x0134, //#TVAR_ash_pGAS[324]
    0x0F12, 0x010B, //#TVAR_ash_pGAS[325]
    0x0F12, 0x00C9, //#TVAR_ash_pGAS[326]
    0x0F12, 0x008C, //#TVAR_ash_pGAS[327]
    0x0F12, 0x005C, //#TVAR_ash_pGAS[328]
    0x0F12, 0x003B, //#TVAR_ash_pGAS[329]
    0x0F12, 0x0026, //#TVAR_ash_pGAS[330]
    0x0F12, 0x001F, //#TVAR_ash_pGAS[331]
    0x0F12, 0x0026, //#TVAR_ash_pGAS[332]
    0x0F12, 0x003D, //#TVAR_ash_pGAS[333]
    0x0F12, 0x0060, //#TVAR_ash_pGAS[334]
    0x0F12, 0x008F, //#TVAR_ash_pGAS[335]
    0x0F12, 0x00CE, //#TVAR_ash_pGAS[336]
    0x0F12, 0x0115, //#TVAR_ash_pGAS[337]
    0x0F12, 0x00FA, //#TVAR_ash_pGAS[338]
    0x0F12, 0x00B5, //#TVAR_ash_pGAS[339]
    0x0F12, 0x0078, //#TVAR_ash_pGAS[340]
    0x0F12, 0x0048, //#TVAR_ash_pGAS[341]
    0x0F12, 0x0026, //#TVAR_ash_pGAS[342]
    0x0F12, 0x0010, //#TVAR_ash_pGAS[343]
    0x0F12, 0x0008, //#TVAR_ash_pGAS[344]
    0x0F12, 0x0010, //#TVAR_ash_pGAS[345]
    0x0F12, 0x0025, //#TVAR_ash_pGAS[346]
    0x0F12, 0x0048, //#TVAR_ash_pGAS[347]
    0x0F12, 0x0077, //#TVAR_ash_pGAS[348]
    0x0F12, 0x00B6, //#TVAR_ash_pGAS[349]
    0x0F12, 0x00FE, //#TVAR_ash_pGAS[350]
    0x0F12, 0x00F6, //#TVAR_ash_pGAS[351]
    0x0F12, 0x00B0, //#TVAR_ash_pGAS[352]
    0x0F12, 0x0072, //#TVAR_ash_pGAS[353]
    0x0F12, 0x0042, //#TVAR_ash_pGAS[354]
    0x0F12, 0x0020, //#TVAR_ash_pGAS[355]
    0x0F12, 0x0009, //#TVAR_ash_pGAS[356]
    0x0F12, 0x0001, //#TVAR_ash_pGAS[357]
    0x0F12, 0x0008, //#TVAR_ash_pGAS[358]
    0x0F12, 0x001D, //#TVAR_ash_pGAS[359]
    0x0F12, 0x003F, //#TVAR_ash_pGAS[360]
    0x0F12, 0x006E, //#TVAR_ash_pGAS[361]
    0x0F12, 0x00AC, //#TVAR_ash_pGAS[362]
    0x0F12, 0x00F5, //#TVAR_ash_pGAS[363]
    0x0F12, 0x00FE, //#TVAR_ash_pGAS[364]
    0x0F12, 0x00B7, //#TVAR_ash_pGAS[365]
    0x0F12, 0x0079, //#TVAR_ash_pGAS[366]
    0x0F12, 0x0049, //#TVAR_ash_pGAS[367]
    0x0F12, 0x0027, //#TVAR_ash_pGAS[368]
    0x0F12, 0x0010, //#TVAR_ash_pGAS[369]
    0x0F12, 0x0007, //#TVAR_ash_pGAS[370]
    0x0F12, 0x000E, //#TVAR_ash_pGAS[371]
    0x0F12, 0x0022, //#TVAR_ash_pGAS[372]
    0x0F12, 0x0043, //#TVAR_ash_pGAS[373]
    0x0F12, 0x0073, //#TVAR_ash_pGAS[374]
    0x0F12, 0x00B1, //#TVAR_ash_pGAS[375]
    0x0F12, 0x00F8, //#TVAR_ash_pGAS[376]
    0x0F12, 0x0110, //#TVAR_ash_pGAS[377]
    0x0F12, 0x00CD, //#TVAR_ash_pGAS[378]
    0x0F12, 0x008F, //#TVAR_ash_pGAS[379]
    0x0F12, 0x005F, //#TVAR_ash_pGAS[380]
    0x0F12, 0x003B, //#TVAR_ash_pGAS[381]
    0x0F12, 0x0024, //#TVAR_ash_pGAS[382]
    0x0F12, 0x001B, //#TVAR_ash_pGAS[383]
    0x0F12, 0x0021, //#TVAR_ash_pGAS[384]
    0x0F12, 0x0034, //#TVAR_ash_pGAS[385]
    0x0F12, 0x0056, //#TVAR_ash_pGAS[386]
    0x0F12, 0x0085, //#TVAR_ash_pGAS[387]
    0x0F12, 0x00C4, //#TVAR_ash_pGAS[388]
    0x0F12, 0x0108, //#TVAR_ash_pGAS[389]
    0x0F12, 0x012C, //#TVAR_ash_pGAS[390]
    0x0F12, 0x00ED, //#TVAR_ash_pGAS[391]
    0x0F12, 0x00B1, //#TVAR_ash_pGAS[392]
    0x0F12, 0x0080, //#TVAR_ash_pGAS[393]
    0x0F12, 0x005B, //#TVAR_ash_pGAS[394]
    0x0F12, 0x0044, //#TVAR_ash_pGAS[395]
    0x0F12, 0x003B, //#TVAR_ash_pGAS[396]
    0x0F12, 0x003F, //#TVAR_ash_pGAS[397]
    0x0F12, 0x0053, //#TVAR_ash_pGAS[398]
    0x0F12, 0x0074, //#TVAR_ash_pGAS[399]
    0x0F12, 0x00A3, //#TVAR_ash_pGAS[400]
    0x0F12, 0x00E0, //#TVAR_ash_pGAS[401]
    0x0F12, 0x0121, //#TVAR_ash_pGAS[402]
    0x0F12, 0x014D, //#TVAR_ash_pGAS[403]
    0x0F12, 0x0115, //#TVAR_ash_pGAS[404]
    0x0F12, 0x00DD, //#TVAR_ash_pGAS[405]
    0x0F12, 0x00AD, //#TVAR_ash_pGAS[406]
    0x0F12, 0x0088, //#TVAR_ash_pGAS[407]
    0x0F12, 0x0071, //#TVAR_ash_pGAS[408]
    0x0F12, 0x0068, //#TVAR_ash_pGAS[409]
    0x0F12, 0x006C, //#TVAR_ash_pGAS[410]
    0x0F12, 0x007F, //#TVAR_ash_pGAS[411]
    0x0F12, 0x00A1, //#TVAR_ash_pGAS[412]
    0x0F12, 0x00CE, //#TVAR_ash_pGAS[413]
    0x0F12, 0x0107, //#TVAR_ash_pGAS[414]
    0x0F12, 0x013E, //#TVAR_ash_pGAS[415]
    0x0F12, 0x0161, //#TVAR_ash_pGAS[416]
    0x0F12, 0x0137, //#TVAR_ash_pGAS[417]
    0x0F12, 0x0104, //#TVAR_ash_pGAS[418]
    0x0F12, 0x00D8, //#TVAR_ash_pGAS[419]
    0x0F12, 0x00B5, //#TVAR_ash_pGAS[420]
    0x0F12, 0x00A0, //#TVAR_ash_pGAS[421]
    0x0F12, 0x0098, //#TVAR_ash_pGAS[422]
    0x0F12, 0x009C, //#TVAR_ash_pGAS[423]
    0x0F12, 0x00AE, //#TVAR_ash_pGAS[424]
    0x0F12, 0x00CE, //#TVAR_ash_pGAS[425]
    0x0F12, 0x00F9, //#TVAR_ash_pGAS[426]
    0x0F12, 0x012A, //#TVAR_ash_pGAS[427]
    0x0F12, 0x015E, //#TVAR_ash_pGAS[428]
    0x0F12, 0x0134, //#TVAR_ash_pGAS[429]
    0x0F12, 0x010D, //#TVAR_ash_pGAS[430]
    0x0F12, 0x00DF, //#TVAR_ash_pGAS[431]
    0x0F12, 0x00BC, //#TVAR_ash_pGAS[432]
    0x0F12, 0x00A5, //#TVAR_ash_pGAS[433]
    0x0F12, 0x0097, //#TVAR_ash_pGAS[434]
    0x0F12, 0x0094, //#TVAR_ash_pGAS[435]
    0x0F12, 0x009B, //#TVAR_ash_pGAS[436]
    0x0F12, 0x00A9, //#TVAR_ash_pGAS[437]
    0x0F12, 0x00C2, //#TVAR_ash_pGAS[438]
    0x0F12, 0x00E7, //#TVAR_ash_pGAS[439]
    0x0F12, 0x0113, //#TVAR_ash_pGAS[440]
    0x0F12, 0x013A, //#TVAR_ash_pGAS[441]
    0x0F12, 0x011E, //#TVAR_ash_pGAS[442]
    0x0F12, 0x00F1, //#TVAR_ash_pGAS[443]
    0x0F12, 0x00BD, //#TVAR_ash_pGAS[444]
    0x0F12, 0x0096, //#TVAR_ash_pGAS[445]
    0x0F12, 0x007D, //#TVAR_ash_pGAS[446]
    0x0F12, 0x006F, //#TVAR_ash_pGAS[447]
    0x0F12, 0x006C, //#TVAR_ash_pGAS[448]
    0x0F12, 0x0072, //#TVAR_ash_pGAS[449]
    0x0F12, 0x0081, //#TVAR_ash_pGAS[450]
    0x0F12, 0x009A, //#TVAR_ash_pGAS[451]
    0x0F12, 0x00C1, //#TVAR_ash_pGAS[452]
    0x0F12, 0x00F1, //#TVAR_ash_pGAS[453]
    0x0F12, 0x011D, //#TVAR_ash_pGAS[454]
    0x0F12, 0x00F6, //#TVAR_ash_pGAS[455]
    0x0F12, 0x00C3, //#TVAR_ash_pGAS[456]
    0x0F12, 0x008D, //#TVAR_ash_pGAS[457]
    0x0F12, 0x0066, //#TVAR_ash_pGAS[458]
    0x0F12, 0x004C, //#TVAR_ash_pGAS[459]
    0x0F12, 0x003F, //#TVAR_ash_pGAS[460]
    0x0F12, 0x003B, //#TVAR_ash_pGAS[461]
    0x0F12, 0x0040, //#TVAR_ash_pGAS[462]
    0x0F12, 0x004E, //#TVAR_ash_pGAS[463]
    0x0F12, 0x0068, //#TVAR_ash_pGAS[464]
    0x0F12, 0x008D, //#TVAR_ash_pGAS[465]
    0x0F12, 0x00C0, //#TVAR_ash_pGAS[466]
    0x0F12, 0x00F4, //#TVAR_ash_pGAS[467]
    0x0F12, 0x00DA, //#TVAR_ash_pGAS[468]
    0x0F12, 0x00A4, //#TVAR_ash_pGAS[469]
    0x0F12, 0x006D, //#TVAR_ash_pGAS[470]
    0x0F12, 0x0046, //#TVAR_ash_pGAS[471]
    0x0F12, 0x002D, //#TVAR_ash_pGAS[472]
    0x0F12, 0x001F, //#TVAR_ash_pGAS[473]
    0x0F12, 0x001A, //#TVAR_ash_pGAS[474]
    0x0F12, 0x001F, //#TVAR_ash_pGAS[475]
    0x0F12, 0x002D, //#TVAR_ash_pGAS[476]
    0x0F12, 0x0044, //#TVAR_ash_pGAS[477]
    0x0F12, 0x006A, //#TVAR_ash_pGAS[478]
    0x0F12, 0x009F, //#TVAR_ash_pGAS[479]
    0x0F12, 0x00D4, //#TVAR_ash_pGAS[480]
    0x0F12, 0x00C8, //#TVAR_ash_pGAS[481]
    0x0F12, 0x0091, //#TVAR_ash_pGAS[482]
    0x0F12, 0x005B, //#TVAR_ash_pGAS[483]
    0x0F12, 0x0034, //#TVAR_ash_pGAS[484]
    0x0F12, 0x001A, //#TVAR_ash_pGAS[485]
    0x0F12, 0x000B, //#TVAR_ash_pGAS[486]
    0x0F12, 0x0007, //#TVAR_ash_pGAS[487]
    0x0F12, 0x000B, //#TVAR_ash_pGAS[488]
    0x0F12, 0x0018, //#TVAR_ash_pGAS[489]
    0x0F12, 0x0031, //#TVAR_ash_pGAS[490]
    0x0F12, 0x0054, //#TVAR_ash_pGAS[491]
    0x0F12, 0x0089, //#TVAR_ash_pGAS[492]
    0x0F12, 0x00C0, //#TVAR_ash_pGAS[493]
    0x0F12, 0x00C5, //#TVAR_ash_pGAS[494]
    0x0F12, 0x008E, //#TVAR_ash_pGAS[495]
    0x0F12, 0x0056, //#TVAR_ash_pGAS[496]
    0x0F12, 0x002F, //#TVAR_ash_pGAS[497]
    0x0F12, 0x0015, //#TVAR_ash_pGAS[498]
    0x0F12, 0x0005, //#TVAR_ash_pGAS[499]
    0x0F12, 0x0000, //#TVAR_ash_pGAS[500]
    0x0F12, 0x0004, //#TVAR_ash_pGAS[501]
    0x0F12, 0x0011, //#TVAR_ash_pGAS[502]
    0x0F12, 0x0029, //#TVAR_ash_pGAS[503]
    0x0F12, 0x004C, //#TVAR_ash_pGAS[504]
    0x0F12, 0x0083, //#TVAR_ash_pGAS[505]
    0x0F12, 0x00BA, //#TVAR_ash_pGAS[506]
    0x0F12, 0x00CF, //#TVAR_ash_pGAS[507]
    0x0F12, 0x0097, //#TVAR_ash_pGAS[508]
    0x0F12, 0x005F, //#TVAR_ash_pGAS[509]
    0x0F12, 0x0036, //#TVAR_ash_pGAS[510]
    0x0F12, 0x001C, //#TVAR_ash_pGAS[511]
    0x0F12, 0x000C, //#TVAR_ash_pGAS[512]
    0x0F12, 0x0007, //#TVAR_ash_pGAS[513]
    0x0F12, 0x000A, //#TVAR_ash_pGAS[514]
    0x0F12, 0x0018, //#TVAR_ash_pGAS[515]
    0x0F12, 0x002F, //#TVAR_ash_pGAS[516]
    0x0F12, 0x0053, //#TVAR_ash_pGAS[517]
    0x0F12, 0x0089, //#TVAR_ash_pGAS[518]
    0x0F12, 0x00BF, //#TVAR_ash_pGAS[519]
    0x0F12, 0x00E4, //#TVAR_ash_pGAS[520]
    0x0F12, 0x00AD, //#TVAR_ash_pGAS[521]
    0x0F12, 0x0074, //#TVAR_ash_pGAS[522]
    0x0F12, 0x004C, //#TVAR_ash_pGAS[523]
    0x0F12, 0x0030, //#TVAR_ash_pGAS[524]
    0x0F12, 0x0020, //#TVAR_ash_pGAS[525]
    0x0F12, 0x0019, //#TVAR_ash_pGAS[526]
    0x0F12, 0x001D, //#TVAR_ash_pGAS[527]
    0x0F12, 0x002A, //#TVAR_ash_pGAS[528]
    0x0F12, 0x0041, //#TVAR_ash_pGAS[529]
    0x0F12, 0x0067, //#TVAR_ash_pGAS[530]
    0x0F12, 0x009C, //#TVAR_ash_pGAS[531]
    0x0F12, 0x00D1, //#TVAR_ash_pGAS[532]
    0x0F12, 0x0102, //#TVAR_ash_pGAS[533]
    0x0F12, 0x00CD, //#TVAR_ash_pGAS[534]
    0x0F12, 0x0096, //#TVAR_ash_pGAS[535]
    0x0F12, 0x006C, //#TVAR_ash_pGAS[536]
    0x0F12, 0x004E, //#TVAR_ash_pGAS[537]
    0x0F12, 0x003C, //#TVAR_ash_pGAS[538]
    0x0F12, 0x0036, //#TVAR_ash_pGAS[539]
    0x0F12, 0x0039, //#TVAR_ash_pGAS[540]
    0x0F12, 0x0046, //#TVAR_ash_pGAS[541]
    0x0F12, 0x005F, //#TVAR_ash_pGAS[542]
    0x0F12, 0x0085, //#TVAR_ash_pGAS[543]
    0x0F12, 0x00B8, //#TVAR_ash_pGAS[544]
    0x0F12, 0x00EB, //#TVAR_ash_pGAS[545]
    0x0F12, 0x0126, //#TVAR_ash_pGAS[546]
    0x0F12, 0x00F9, //#TVAR_ash_pGAS[547]
    0x0F12, 0x00C2, //#TVAR_ash_pGAS[548]
    0x0F12, 0x0097, //#TVAR_ash_pGAS[549]
    0x0F12, 0x0079, //#TVAR_ash_pGAS[550]
    0x0F12, 0x0065, //#TVAR_ash_pGAS[551]
    0x0F12, 0x005E, //#TVAR_ash_pGAS[552]
    0x0F12, 0x0060, //#TVAR_ash_pGAS[553]
    0x0F12, 0x006F, //#TVAR_ash_pGAS[554]
    0x0F12, 0x008A, //#TVAR_ash_pGAS[555]
    0x0F12, 0x00AF, //#TVAR_ash_pGAS[556]
    0x0F12, 0x00E2, //#TVAR_ash_pGAS[557]
    0x0F12, 0x010F, //#TVAR_ash_pGAS[558]
    0x0F12, 0x013B, //#TVAR_ash_pGAS[559]
    0x0F12, 0x0119, //#TVAR_ash_pGAS[560]
    0x0F12, 0x00E8, //#TVAR_ash_pGAS[561]
    0x0F12, 0x00BD, //#TVAR_ash_pGAS[562]
    0x0F12, 0x00A0, //#TVAR_ash_pGAS[563]
    0x0F12, 0x008F, //#TVAR_ash_pGAS[564]
    0x0F12, 0x0087, //#TVAR_ash_pGAS[565]
    0x0F12, 0x008B, //#TVAR_ash_pGAS[566]
    0x0F12, 0x0099, //#TVAR_ash_pGAS[567]
    0x0F12, 0x00B3, //#TVAR_ash_pGAS[568]
    0x0F12, 0x00D7, //#TVAR_ash_pGAS[569]
    0x0F12, 0x0105, //#TVAR_ash_pGAS[570]
    0x0F12, 0x0130, //#TVAR_ash_pGAS[571]
    0x002A, 0x0D30,
    0x0F12, 0x02A7, //#awbb_GLocusR
    0x0F12, 0x0343, //#awbb_GLocusB
    0x002A, 0x06B8, //##GAS_Alpha separated
    0x0F12, 0x00D0, //#TVAR_ash_AwbAshCord_0_Hor
    0x0F12, 0x00F0, //#TVAR_ash_AwbAshCord_1_Inca
    0x0F12, 0x0120, //#TVAR_ash_AwbAshCord_2_CWF,TL84
    0x0F12, 0x0150, //#TVAR_ash_AwbAshCord_3_ì‹¤ë‚´
    0x0F12, 0x0158, //0160 //#TVAR_ash_AwbAshCord_4_D50, DNP
    0x0F12, 0x016A, //#TVAR_ash_AwbAshCord_5_D65
    0x0F12, 0x01A0, //#TVAR_ash_AwbAshCord_6_D75

    0x002A, 0x0688, //#seti_bUseEfuseCorrection
    0x0F12, 0x0000, //# 0 : Not use, 1 : Use

};

static unsigned short s5k5cagx_init_reg_short4[]={


    //#================================================================================================
    //#SET CCM
    //#================================================================================================
    //#CCM start address //#7000_33A4
    0x002A, 0x0698,
    0x0F12, 0x33A4,
    0x0F12, 0x7000,
    //#Horizon[0]
    0x002A, 0x33A4,
    0x0F12, 0x01FE, //#TVAR_wbt_pBaseCcms [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pBaseCcms [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pBaseCcms [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pBaseCcms [04]
    0x0F12, 0x0210, //#TVAR_wbt_pBaseCcms [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pBaseCcms [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pBaseCcms [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pBaseCcms [08]
    0x0F12, 0x021F, //#TVAR_wbt_pBaseCcms [09]
    0x0F12, 0x0120, //#TVAR_wbt_pBaseCcms [10]
    0x0F12, 0x0156, //#TVAR_wbt_pBaseCcms [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pBaseCcms [12]
    0x0F12, 0x0161, //#TVAR_wbt_pBaseCcms [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pBaseCcms [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pBaseCcms [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pBaseCcms [16]
    0x0F12, 0x0206, //#TVAR_wbt_pBaseCcms [17]
    0x0F12, 0x0175, //#TVAR_wbt_pBaseCcms [18]
    //#Inca[1]
    0x0F12, 0x01FE, //#TVAR_wbt_pBaseCcms [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pBaseCcms [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pBaseCcms [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pBaseCcms [04]
    0x0F12, 0x0210, //#TVAR_wbt_pBaseCcms [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pBaseCcms [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pBaseCcms [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pBaseCcms [08]
    0x0F12, 0x021F, //#TVAR_wbt_pBaseCcms [09]
    0x0F12, 0x0120, //#TVAR_wbt_pBaseCcms [10]
    0x0F12, 0x0156, //#TVAR_wbt_pBaseCcms [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pBaseCcms [12]
    0x0F12, 0x0161, //#TVAR_wbt_pBaseCcms [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pBaseCcms [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pBaseCcms [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pBaseCcms [16]
    0x0F12, 0x0206, //#TVAR_wbt_pBaseCcms [17]
    0x0F12, 0x0175, //#TVAR_wbt_pBaseCcms [18]
    //#TL84[2]
    0x0F12, 0x01FE, //#TVAR_wbt_pBaseCcms [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pBaseCcms [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pBaseCcms [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pBaseCcms [04]
    0x0F12, 0x0210, //#TVAR_wbt_pBaseCcms [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pBaseCcms [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pBaseCcms [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pBaseCcms [08]
    0x0F12, 0x021F, //#TVAR_wbt_pBaseCcms [09]
    0x0F12, 0x0120, //#TVAR_wbt_pBaseCcms [10]
    0x0F12, 0x0156, //#TVAR_wbt_pBaseCcms [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pBaseCcms [12]
    0x0F12, 0x0161, //#TVAR_wbt_pBaseCcms [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pBaseCcms [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pBaseCcms [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pBaseCcms [16]
    0x0F12, 0x0206, //#TVAR_wbt_pBaseCcms [17]
    0x0F12, 0x0175, //#TVAR_wbt_pBaseCcms [18]
    //#CWF[3]
    0x0F12, 0x01FE, //#TVAR_wbt_pBaseCcms [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pBaseCcms [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pBaseCcms [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pBaseCcms [04]
    0x0F12, 0x0210, //#TVAR_wbt_pBaseCcms [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pBaseCcms [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pBaseCcms [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pBaseCcms [08]
    0x0F12, 0x021F, //#TVAR_wbt_pBaseCcms [09]
    0x0F12, 0x0120, //#TVAR_wbt_pBaseCcms [10]
    0x0F12, 0x0156, //#TVAR_wbt_pBaseCcms [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pBaseCcms [12]
    0x0F12, 0x0161, //#TVAR_wbt_pBaseCcms [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pBaseCcms [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pBaseCcms [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pBaseCcms [16]
    0x0F12, 0x0206, //#TVAR_wbt_pBaseCcms [17]
    0x0F12, 0x0175, //#TVAR_wbt_pBaseCcms [18]
    //#D50[4]
    0x0F12, 0x01FE, //#TVAR_wbt_pBaseCcms [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pBaseCcms [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pBaseCcms [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pBaseCcms [04]
    0x0F12, 0x0210, //#TVAR_wbt_pBaseCcms [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pBaseCcms [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pBaseCcms [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pBaseCcms [08]
    0x0F12, 0x021F, //#TVAR_wbt_pBaseCcms [09]
    0x0F12, 0x0120, //#TVAR_wbt_pBaseCcms [10]
    0x0F12, 0x0156, //#TVAR_wbt_pBaseCcms [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pBaseCcms [12]
    0x0F12, 0x0161, //#TVAR_wbt_pBaseCcms [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pBaseCcms [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pBaseCcms [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pBaseCcms [16]
    0x0F12, 0x0206, //#TVAR_wbt_pBaseCcms [17]
    0x0F12, 0x0175, //#TVAR_wbt_pBaseCcms [18]
    //#D65[5]
    0x0F12, 0x01FE, //#TVAR_wbt_pBaseCcms [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pBaseCcms [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pBaseCcms [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pBaseCcms [04]
    0x0F12, 0x0210, //#TVAR_wbt_pBaseCcms [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pBaseCcms [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pBaseCcms [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pBaseCcms [08]
    0x0F12, 0x021F, //#TVAR_wbt_pBaseCcms [09]
    0x0F12, 0x0120, //#TVAR_wbt_pBaseCcms [10]
    0x0F12, 0x0156, //#TVAR_wbt_pBaseCcms [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pBaseCcms [12]
    0x0F12, 0x0161, //#TVAR_wbt_pBaseCcms [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pBaseCcms [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pBaseCcms [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pBaseCcms [16]
    0x0F12, 0x0206, //#TVAR_wbt_pBaseCcms [17]
    0x0F12, 0x0175, //#TVAR_wbt_pBaseCcms [18]
    //#Outdoor CCM address //#7000_3380
    0x002A, 0x06A0,
    0x0F12, 0x3380,
    0x0F12, 0x7000,
    //#Outdoor CCM
    0x002A, 0x3380,
    0x0F12, 0x01FE, //#TVAR_wbt_pOutdoorCcm [01]
    0x0F12, 0xFF50, //#TVAR_wbt_pOutdoorCcm [02]
    0x0F12, 0xFFDC, //#TVAR_wbt_pOutdoorCcm [03]
    0x0F12, 0xFF21, //#TVAR_wbt_pOutdoorCcm [04]
    0x0F12, 0x0210, //#TVAR_wbt_pOutdoorCcm [05]
    0x0F12, 0xFF15, //#TVAR_wbt_pOutdoorCcm [06]
    0x0F12, 0xFFCA, //#TVAR_wbt_pOutdoorCcm [07]
    0x0F12, 0xFF9B, //#TVAR_wbt_pOutdoorCcm [08]
    0x0F12, 0x021F, //#TVAR_wbt_pOutdoorCcm [09]
    0x0F12, 0x0120, //#TVAR_wbt_pOutdoorCcm [10]
    0x0F12, 0x0156, //#TVAR_wbt_pOutdoorCcm [11]
    0x0F12, 0xFEFF, //#TVAR_wbt_pOutdoorCcm [12]
    0x0F12, 0x0161, //#TVAR_wbt_pOutdoorCcm [13]
    0x0F12, 0xFF88, //#TVAR_wbt_pOutdoorCcm [14]
    0x0F12, 0x01C8, //#TVAR_wbt_pOutdoorCcm [15]
    0x0F12, 0xFE58, //#TVAR_wbt_pOutdoorCcm [16]
    0x0F12, 0x0206, //#TVAR_wbt_pOutdoorCcm [17]
    0x0F12, 0x0175, //#TVAR_wbt_pOutdoorCcm [18]
    0x002A, 0x06A6,
    0x0F12, 0x00D8, //#SARR_AwbCcmCord_0_Horizon (00E7)
    0x0F12, 0x0100, //#SARR_AwbCcmCord_1_Inca (0111)
    0x0F12, 0x0120, //#SARR_AwbCcmCord_2_TL84 (012F)
    0x0F12, 0x0140, //#SARR_AwbCcmCord_3_CWF (0145)
    0x0F12, 0x0160, //#SARR_AwbCcmCord_4_D50 (0177)
    0x0F12, 0x0180, //#SARR_AwbCcmCord_5_D65 (0178)
    //================================================================================================
    // SET AWB
    //================================================================================================
    // Indoor boundary
    0x002A, 0x0C48,
    0x0F12, 0x03C8,  //034C //awbb_IndoorGrZones_m_BGrid[0]
    0x0F12, 0x03E2,  //037E //awbb_IndoorGrZones_m_BGrid[1]
    0x0F12, 0x038A,  //0307 //awbb_IndoorGrZones_m_BGrid[2]
    0x0F12, 0x03F4,  //037C //awbb_IndoorGrZones_m_BGrid[3]
    0x0F12, 0x034E,  //02D5 //awbb_IndoorGrZones_m_BGrid[4]
    0x0F12, 0x03E2,  //035F //awbb_IndoorGrZones_m_BGrid[5]
    0x0F12, 0x030A,  //02A9 //awbb_IndoorGrZones_m_BGrid[6]
    0x0F12, 0x03B8,  //033A //awbb_IndoorGrZones_m_BGrid[7]
    0x0F12, 0x02C8,  //0283 //awbb_IndoorGrZones_m_BGrid[8]
    0x0F12, 0x038A,  //031A //awbb_IndoorGrZones_m_BGrid[9]
    0x0F12, 0x029C,  //0261 //awbb_IndoorGrZones_m_BGrid[10]
    0x0F12, 0x034E,  //02FE //awbb_IndoorGrZones_m_BGrid[11]
    0x0F12, 0x0286,  //0244 //awbb_IndoorGrZones_m_BGrid[12]
    0x0F12, 0x0310,  //02E3 //awbb_IndoorGrZones_m_BGrid[13]
    0x0F12, 0x026C,  //0225 //awbb_IndoorGrZones_m_BGrid[14]
    0x0F12, 0x02E4,  //02D2 //awbb_IndoorGrZones_m_BGrid[15]
    0x0F12, 0x0254,  //0211 //awbb_IndoorGrZones_m_BGrid[16]
    0x0F12, 0x02CA,  //02BE //awbb_IndoorGrZones_m_BGrid[17]
    0x0F12, 0x023E,  //01FD //awbb_IndoorGrZones_m_BGrid[18]
    0x0F12, 0x02B8,  //02AC //awbb_IndoorGrZones_m_BGrid[19]
    0x0F12, 0x022E,  //01F2 //awbb_IndoorGrZones_m_BGrid[20]
    0x0F12, 0x02A4,  //0299 //awbb_IndoorGrZones_m_BGrid[21]
    0x0F12, 0x0226,  //01E3 //awbb_IndoorGrZones_m_BGrid[22]
    0x0F12, 0x0294,  //0286 //awbb_IndoorGrZones_m_BGrid[23]
    0x0F12, 0x0220,  //01D8 //awbb_IndoorGrZones_m_BGrid[24]
    0x0F12, 0x027E,  //026B //awbb_IndoorGrZones_m_BGrid[25]
    0x0F12, 0x022A,  //01D8 //awbb_IndoorGrZones_m_BGrid[26]
    0x0F12, 0x025E,  //0257 //awbb_IndoorGrZones_m_BGrid[27]
    0x0F12, 0x0000,  //01F2 //awbb_IndoorGrZones_m_BGrid[28]
    0x0F12, 0x0000,  //0230 //awbb_IndoorGrZones_m_BGrid[29]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[30]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[31]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[32]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[33]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[34]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[35]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[36]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[37]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[38]
    0x0F12, 0x0000,  //0000 //awbb_IndoorGrZones_m_BGrid[39]
    0x0F12, 0x0005,  //0005 // #awbb_IndoorGrZones_m_GridStep
    0x002A, 0x0CA0, //0CA0
    0x0F12, 0x00FE, //0169 // #awbb_IndoorGrZones_m_Boffs

    // Outdoor boundary
    0x002A, 0x0CA4,
    0x0F12, 0x027E,  //0281 // #awbb_OutdoorGrZones_m_BGrid[0]
    0x0F12, 0x02AE,  //02BC // #awbb_OutdoorGrZones_m_BGrid[1]
    0x0F12, 0x025C,  //0258 // #awbb_OutdoorGrZones_m_BGrid[2]
    0x0F12, 0x02B2,  //02C9 // #awbb_OutdoorGrZones_m_BGrid[3]
    0x0F12, 0x0244,  //023E // #awbb_OutdoorGrZones_m_BGrid[4]
    0x0F12, 0x02A0,  //02C3 // #awbb_OutdoorGrZones_m_BGrid[5]
    0x0F12, 0x0236,  //022F // #awbb_OutdoorGrZones_m_BGrid[6]
    0x0F12, 0x0290,  //02B5 // #awbb_OutdoorGrZones_m_BGrid[7]
    0x0F12, 0x0230,  //022F // #awbb_OutdoorGrZones_m_BGrid[8]
    0x0F12, 0x027A,  //029A // #awbb_OutdoorGrZones_m_BGrid[9]
    0x0F12, 0x0236,  //0251 // #awbb_OutdoorGrZones_m_BGrid[10]
    0x0F12, 0x025E,  //0272 // #awbb_OutdoorGrZones_m_BGrid[11]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[12]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[13]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[14]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[15]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[16]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[17]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[18]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[19]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[20]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[21]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[22]
    0x0F12, 0x0000,  //0000 // #awbb_OutdoorGrZones_m_BGrid[23]
    0x0F12, 0x0005,  //0005 // #awbb_OutdoorGrZones_m_GridStep
    0x002A, 0x0CDC,  //0CDC
    0x0F12, 0x01EE,  //0216 // #awbb_OutdoorGrZones_m_Boffs

    // Outdoor detection zone??
    0x002A, 0x0D88,
    0x0F12, 0xFFB6, //#awbb_OutdoorDetectionZone_m_BGrid[0]_m_left
    0x0F12, 0x00B6, //#awbb_OutdoorDetectionZone_m_BGrid[0]_m_right
    0x0F12, 0xFF38, //#awbb_OutdoorDetectionZone_m_BGrid[1]_m_left
    0x0F12, 0x0118, //#awbb_OutdoorDetectionZone_m_BGrid[1]_m_right
    0x0F12, 0xFEF1, //#awbb_OutdoorDetectionZone_m_BGrid[2]_m_left
    0x0F12, 0x015F, //#awbb_OutdoorDetectionZone_m_BGrid[2]_m_right
    0x0F12, 0xFEC0, //#awbb_OutdoorDetectionZone_m_BGrid[3]_m_left
    0x0F12, 0x0199, //#awbb_OutdoorDetectionZone_m_BGrid[3]_m_right
    0x0F12, 0xFE91, //#awbb_OutdoorDetectionZone_m_BGrid[4]_m_left
    0x0F12, 0x01CF, //#awbb_OutdoorDetectionZone_m_BGrid[4]_m_right
    0x0F12, 0x1388, //#awbb_OutdoorDetectionZone_ZInfo_m_AbsGridStep
    0x0F12, 0x0000,
    0x0F12, 0x0005, //#awbb_OutdoorDetectionZone_ZInfo_m_GridSz
    0x0F12, 0x0000,
    0x0F12, 0x1387, //#awbb_OutdoorDetectionZone_ZInfo_m_NBoffs
    0x0F12, 0x0000,
    0x0F12, 0x1388, //#awbb_OutdoorDetectionZone_ZInfo_m_MaxNB
    0x0F12, 0x0000,

    // LowBr boundary
    0x002A, 0x0CE0,
    0x0F12, 0x03EA,  //03C6 //#awbb_LowBrGrZones_m_BGrid[0]
    0x0F12, 0x044E,  //03F1 //#awbb_LowBrGrZones_m_BGrid[1]
    0x0F12, 0x035E,  //0329 //#awbb_LowBrGrZones_m_BGrid[2]
    0x0F12, 0x044C,  //03F6 //#awbb_LowBrGrZones_m_BGrid[3]
    0x0F12, 0x02FA,  //02BF //#awbb_LowBrGrZones_m_BGrid[4]
    0x0F12, 0x0434,  //03B6 //#awbb_LowBrGrZones_m_BGrid[5]
    0x0F12, 0x02AA,  //0268 //#awbb_LowBrGrZones_m_BGrid[6]
    0x0F12, 0x03F2,  //037A //#awbb_LowBrGrZones_m_BGrid[7]
    0x0F12, 0x0266,  //022A //#awbb_LowBrGrZones_m_BGrid[8]
    0x0F12, 0x03AE,  //0332 //#awbb_LowBrGrZones_m_BGrid[9]
    0x0F12, 0x022C,  //01F6 //#awbb_LowBrGrZones_m_BGrid[10]
    0x0F12, 0x035A,  //02F9 //#awbb_LowBrGrZones_m_BGrid[11]
    0x0F12, 0x020E,  //01D6 //#awbb_LowBrGrZones_m_BGrid[12]
    0x0F12, 0x0314,  //02CC //#awbb_LowBrGrZones_m_BGrid[13]
    0x0F12, 0x01F4,  //01C2 //#awbb_LowBrGrZones_m_BGrid[14]
    0x0F12, 0x02E0,  //02A7 //#awbb_LowBrGrZones_m_BGrid[15]
    0x0F12, 0x01E2,  //01B2 //#awbb_LowBrGrZones_m_BGrid[16]
    0x0F12, 0x02AA,  //0280 //#awbb_LowBrGrZones_m_BGrid[17]
    0x0F12, 0x01E6,  //01BD //#awbb_LowBrGrZones_m_BGrid[18]
    0x0F12, 0x0264,  //024E //#awbb_LowBrGrZones_m_BGrid[19]
    0x0F12, 0x0000,  //01E2 //#awbb_LowBrGrZones_m_BGrid[20]
    0x0F12, 0x0000,  //020C //#awbb_LowBrGrZones_m_BGrid[21]
    0x0F12, 0x0000,  //0000 //#awbb_LowBrGrZones_m_BGrid[22]
    0x0F12, 0x0000,  //0000 //#awbb_LowBrGrZones_m_BGrid[23]
    0x0F12, 0x0006,  //0006 //#awbb_LowBrGrZones_m_GridStep
    0x002A, 0x0D18,  //0D18
    0x0F12, 0x009A,  //00FE //#awbb_LowBrGrZones_m_Boffs

    // AWB ETC
    0x002A, 0x0D1C,
    0x0F12, 0x036C, //#awbb_CrclLowT_R_c
    0x002A, 0x0D20,
    0x0F12, 0x011D, //#awbb_CrclLowT_B_c
    0x002A, 0x0D24,
    0x0F12, 0x62B8, //#awbb_CrclLowT_Rad_c

    0x002A, 0x0D2C,  //0D2C
    0x0F12, 0x0135,  //0141 //#awbb_IntcR
    0x0F12, 0x012B,  //013B //#awbb_IntcB

    0x002A, 0x0D28,  //0D28
    0x0F12, 0x023B,  //0269 //#awbb_OutdoorWP_r
    0x0F12, 0x0274,  //0240 //#awbb_OutdoorWP_b

    0x002A, 0x0E4C,
    0x0F12, 0x0000, //#awbboost_useBoosting4Outdoor
    0x002A, 0x0D4C,  //0D4C
    0x0F12, 0x05C6,  //0187 //#awbb_GamutWidthThr1
    0x0F12, 0x02DF,  //00CF //#awbb_GamutHeightThr1
    0x0F12, 0x0030,  //000D //#awbb_GamutWidthThr2
    0x0F12, 0x0020,  //000A //#awbb_GamutHeightThr2

    0x002A, 0x0D5C,
    0x0F12, 0x7FFF, //#awbb_LowTempRB
    0x0F12, 0x0050, //#awbb_LowTemp_RBzone

    0x002A, 0x0D46, //0D46
    0x0F12, 0x0553, //0420//#awbb_MvEq_RBthresh

    0x002A, 0x0D4A,
    0x0F12, 0x000A, //#awbb_MovingScale10

    0x002A, 0x0E3E,
    0x0F12, 0x0000, //#awbb_rpl_InvalidOutdoor off
    //002A 22DE
    //0F12 0004 //#Mon_AWB_ByPassMode // [0]Outdoor [1]LowBr [2]LowTemp

    //002A 337C
    //0F12 00B3 //#Tune_TP_ChMoveToNearR
    //0F12 0040 //#Tune_TP_AvMoveToGamutDist

    // AWB initial point
    0x002A, 0x0E44,
    0x0F12, 0x053C, //#define awbb_GainsInit_0_
    0x0F12, 0x0400, //#define awbb_GainsInit_1_
    0x0F12, 0x055C, //#define awbb_GainsInit_2_
    // Set AWB global offset
    0x002A, 0x0E36,
    0x0F12, 0x0000, //#awbb_RGainOff
    0x0F12, 0x0000, //#awbb_BGainOff
    0x0F12, 0x0000, //#awbb_GGainOff

    //================================================================================================
    // SET GRID OFFSET
    //================================================================================================
    // Not used
    0x002A, 0x0E4A,
    0x0F12, 0x0000, // #awbb_GridEnable

    //================================================================================================
    // SET GAMMA
    //================================================================================================
    0x002A, 0x3288,
    0x0F12, 0x0000, //#SARR_usDualGammaLutRGBIndoor_0__0_ 0x70003288
    0x0F12, 0x0004, //#SARR_usDualGammaLutRGBIndoor_0__1_ 0x7000328A
    0x0F12, 0x0010, //#SARR_usDualGammaLutRGBIndoor_0__2_ 0x7000328C
    0x0F12, 0x002A, //#SARR_usDualGammaLutRGBIndoor_0__3_ 0x7000328E
    0x0F12, 0x0062, //#SARR_usDualGammaLutRGBIndoor_0__4_ 0x70003290
    0x0F12, 0x00D5, //#SARR_usDualGammaLutRGBIndoor_0__5_ 0x70003292
    0x0F12, 0x0138, //#SARR_usDualGammaLutRGBIndoor_0__6_ 0x70003294
    0x0F12, 0x0161, //#SARR_usDualGammaLutRGBIndoor_0__7_ 0x70003296
    0x0F12, 0x0186, //#SARR_usDualGammaLutRGBIndoor_0__8_ 0x70003298
    0x0F12, 0x01BC, //#SARR_usDualGammaLutRGBIndoor_0__9_ 0x7000329A
    0x0F12, 0x01E8, //#SARR_usDualGammaLutRGBIndoor_0__10_ 0x7000329C
    0x0F12, 0x020F, //#SARR_usDualGammaLutRGBIndoor_0__11_ 0x7000329E
    0x0F12, 0x0232, //#SARR_usDualGammaLutRGBIndoor_0__12_ 0x700032A0
    0x0F12, 0x0273, //#SARR_usDualGammaLutRGBIndoor_0__13_ 0x700032A2
    0x0F12, 0x02AF, //#SARR_usDualGammaLutRGBIndoor_0__14_ 0x700032A4
    0x0F12, 0x0309, //#SARR_usDualGammaLutRGBIndoor_0__15_ 0x700032A6
    0x0F12, 0x0355, //#SARR_usDualGammaLutRGBIndoor_0__16_ 0x700032A8
    0x0F12, 0x0394, //#SARR_usDualGammaLutRGBIndoor_0__17_ 0x700032AA
    0x0F12, 0x03CE, //#SARR_usDualGammaLutRGBIndoor_0__18_ 0x700032AC
    0x0F12, 0x03FF, //#SARR_usDualGammaLutRGBIndoor_0__19_ 0x700032AE
    0x0F12, 0x0000, //#SARR_usDualGammaLutRGBIndoor_1__0_ 0x700032B0
    0x0F12, 0x0004, //#SARR_usDualGammaLutRGBIndoor_1__1_ 0x700032B2
    0x0F12, 0x0010, //#SARR_usDualGammaLutRGBIndoor_1__2_ 0x700032B4
    0x0F12, 0x002A, //#SARR_usDualGammaLutRGBIndoor_1__3_ 0x700032B6
    0x0F12, 0x0062, //#SARR_usDualGammaLutRGBIndoor_1__4_ 0x700032B8
    0x0F12, 0x00D5, //#SARR_usDualGammaLutRGBIndoor_1__5_ 0x700032BA
    0x0F12, 0x0138, //#SARR_usDualGammaLutRGBIndoor_1__6_ 0x700032BC
    0x0F12, 0x0161, //#SARR_usDualGammaLutRGBIndoor_1__7_ 0x700032BE
    0x0F12, 0x0186, //#SARR_usDualGammaLutRGBIndoor_1__8_ 0x700032C0
    0x0F12, 0x01BC, //#SARR_usDualGammaLutRGBIndoor_1__9_ 0x700032C2
    0x0F12, 0x01E8, //#SARR_usDualGammaLutRGBIndoor_1__10_ 0x700032C4
    0x0F12, 0x020F, //#SARR_usDualGammaLutRGBIndoor_1__11_ 0x700032C6
    0x0F12, 0x0232, //#SARR_usDualGammaLutRGBIndoor_1__12_ 0x700032C8
    0x0F12, 0x0273, //#SARR_usDualGammaLutRGBIndoor_1__13_ 0x700032CA
    0x0F12, 0x02AF, //#SARR_usDualGammaLutRGBIndoor_1__14_ 0x700032CC
    0x0F12, 0x0309, //#SARR_usDualGammaLutRGBIndoor_1__15_ 0x700032CE
    0x0F12, 0x0355, //#SARR_usDualGammaLutRGBIndoor_1__16_ 0x700032D0
    0x0F12, 0x0394, //#SARR_usDualGammaLutRGBIndoor_1__17_ 0x700032D2
    0x0F12, 0x03CE, //#SARR_usDualGammaLutRGBIndoor_1__18_ 0x700032D4
    0x0F12, 0x03FF, //#SARR_usDualGammaLutRGBIndoor_1__19_ 0x700032D6
    0x0F12, 0x0000, //#SARR_usDualGammaLutRGBIndoor_2__0_ 0x700032D8
    0x0F12, 0x0004, //#SARR_usDualGammaLutRGBIndoor_2__1_ 0x700032DA
    0x0F12, 0x0010, //#SARR_usDualGammaLutRGBIndoor_2__2_ 0x700032DC
    0x0F12, 0x002A, //#SARR_usDualGammaLutRGBIndoor_2__3_ 0x700032DE
    0x0F12, 0x0062, //#SARR_usDualGammaLutRGBIndoor_2__4_ 0x700032E0
    0x0F12, 0x00D5, //#SARR_usDualGammaLutRGBIndoor_2__5_ 0x700032E2
    0x0F12, 0x0138, //#SARR_usDualGammaLutRGBIndoor_2__6_ 0x700032E4
    0x0F12, 0x0161, //#SARR_usDualGammaLutRGBIndoor_2__7_ 0x700032E6
    0x0F12, 0x0186, //#SARR_usDualGammaLutRGBIndoor_2__8_ 0x700032E8
    0x0F12, 0x01BC, //#SARR_usDualGammaLutRGBIndoor_2__9_ 0x700032EA
    0x0F12, 0x01E8, //#SARR_usDualGammaLutRGBIndoor_2__10_ 0x700032EC
    0x0F12, 0x020F, //#SARR_usDualGammaLutRGBIndoor_2__11_ 0x700032EE
    0x0F12, 0x0232, //#SARR_usDualGammaLutRGBIndoor_2__12_ 0x700032F0
    0x0F12, 0x0273, //#SARR_usDualGammaLutRGBIndoor_2__13_ 0x700032F2
    0x0F12, 0x02AF, //#SARR_usDualGammaLutRGBIndoor_2__14_ 0x700032F4
    0x0F12, 0x0309, //#SARR_usDualGammaLutRGBIndoor_2__15_ 0x700032F6
    0x0F12, 0x0355, //#SARR_usDualGammaLutRGBIndoor_2__16_ 0x700032F8
    0x0F12, 0x0394, //#SARR_usDualGammaLutRGBIndoor_2__17_ 0x700032FA
    0x0F12, 0x03CE, //#SARR_usDualGammaLutRGBIndoor_2__18_ 0x700032FC
    0x0F12, 0x03FF, //#SARR_usDualGammaLutRGBIndoor_2__19_ 0x700032FE

    0x0F12, 0x0000, //#SARR_usDualGammaLutRGBOutdoor_0__0_ 0x70003300
    0x0F12, 0x0004, //#SARR_usDualGammaLutRGBOutdoor_0__1_ 0x70003302
    0x0F12, 0x0010, //#SARR_usDualGammaLutRGBOutdoor_0__2_ 0x70003304
    0x0F12, 0x002A, //#SARR_usDualGammaLutRGBOutdoor_0__3_ 0x70003306
    0x0F12, 0x0062, //#SARR_usDualGammaLutRGBOutdoor_0__4_ 0x70003308
    0x0F12, 0x00D5, //#SARR_usDualGammaLutRGBOutdoor_0__5_ 0x7000330A
    0x0F12, 0x0138, //#SARR_usDualGammaLutRGBOutdoor_0__6_ 0x7000330C
    0x0F12, 0x0161, //#SARR_usDualGammaLutRGBOutdoor_0__7_ 0x7000330E
    0x0F12, 0x0186, //#SARR_usDualGammaLutRGBOutdoor_0__8_ 0x70003310
    0x0F12, 0x01BC, //#SARR_usDualGammaLutRGBOutdoor_0__9_ 0x70003312
    0x0F12, 0x01E8, //#SARR_usDualGammaLutRGBOutdoor_0__10_0x70003314
    0x0F12, 0x020F, //#SARR_usDualGammaLutRGBOutdoor_0__11_0x70003316
    0x0F12, 0x0232, //#SARR_usDualGammaLutRGBOutdoor_0__12_0x70003318
    0x0F12, 0x0273, //#SARR_usDualGammaLutRGBOutdoor_0__13_0x7000331A
    0x0F12, 0x02AF, //#SARR_usDualGammaLutRGBOutdoor_0__14_0x7000331C
    0x0F12, 0x0309, //#SARR_usDualGammaLutRGBOutdoor_0__15_0x7000331E
    0x0F12, 0x0355, //#SARR_usDualGammaLutRGBOutdoor_0__16_0x70003320
    0x0F12, 0x0394, //#SARR_usDualGammaLutRGBOutdoor_0__17_0x70003322
    0x0F12, 0x03CE, //#SARR_usDualGammaLutRGBOutdoor_0__18_0x70003324
    0x0F12, 0x03FF, //#SARR_usDualGammaLutRGBOutdoor_0__19_0x70003326
    0x0F12, 0x0000, //#SARR_usDualGammaLutRGBOutdoor_1__0_ 0x70003328
    0x0F12, 0x0004, //#SARR_usDualGammaLutRGBOutdoor_1__1_ 0x7000332A
    0x0F12, 0x0010, //#SARR_usDualGammaLutRGBOutdoor_1__2_ 0x7000332C
    0x0F12, 0x002A, //#SARR_usDualGammaLutRGBOutdoor_1__3_ 0x7000332E
    0x0F12, 0x0062, //#SARR_usDualGammaLutRGBOutdoor_1__4_ 0x70003330
    0x0F12, 0x00D5, //#SARR_usDualGammaLutRGBOutdoor_1__5_ 0x70003332
    0x0F12, 0x0138, //#SARR_usDualGammaLutRGBOutdoor_1__6_ 0x70003334
    0x0F12, 0x0161, //#SARR_usDualGammaLutRGBOutdoor_1__7_ 0x70003336
    0x0F12, 0x0186, //#SARR_usDualGammaLutRGBOutdoor_1__8_ 0x70003338
    0x0F12, 0x01BC, //#SARR_usDualGammaLutRGBOutdoor_1__9_ 0x7000333A
    0x0F12, 0x01E8, //#SARR_usDualGammaLutRGBOutdoor_1__10_0x7000333C
    0x0F12, 0x020F, //#SARR_usDualGammaLutRGBOutdoor_1__11_0x7000333E
    0x0F12, 0x0232, //#SARR_usDualGammaLutRGBOutdoor_1__12_0x70003340
    0x0F12, 0x0273, //#SARR_usDualGammaLutRGBOutdoor_1__13_0x70003342
    0x0F12, 0x02AF, //#SARR_usDualGammaLutRGBOutdoor_1__14_0x70003344
    0x0F12, 0x0309, //#SARR_usDualGammaLutRGBOutdoor_1__15_0x70003346
    0x0F12, 0x0355, //#SARR_usDualGammaLutRGBOutdoor_1__16_0x70003348
    0x0F12, 0x0394, //#SARR_usDualGammaLutRGBOutdoor_1__17_0x7000334A
    0x0F12, 0x03CE, //#SARR_usDualGammaLutRGBOutdoor_1__18_0x7000334C
    0x0F12, 0x03FF, //#SARR_usDualGammaLutRGBOutdoor_1__19_0x7000334E
    0x0F12, 0x0000, //#SARR_usDualGammaLutRGBOutdoor_2__0_ 0x70003350
    0x0F12, 0x0004, //#SARR_usDualGammaLutRGBOutdoor_2__1_ 0x70003352
    0x0F12, 0x0010, //#SARR_usDualGammaLutRGBOutdoor_2__2_ 0x70003354
    0x0F12, 0x002A, //#SARR_usDualGammaLutRGBOutdoor_2__3_ 0x70003356
    0x0F12, 0x0062, //#SARR_usDualGammaLutRGBOutdoor_2__4_ 0x70003358
    0x0F12, 0x00D5, //#SARR_usDualGammaLutRGBOutdoor_2__5_ 0x7000335A
    0x0F12, 0x0138, //#SARR_usDualGammaLutRGBOutdoor_2__6_ 0x7000335C
    0x0F12, 0x0161, //#SARR_usDualGammaLutRGBOutdoor_2__7_ 0x7000335E
    0x0F12, 0x0186, //#SARR_usDualGammaLutRGBOutdoor_2__8_ 0x70003360
    0x0F12, 0x01BC, //#SARR_usDualGammaLutRGBOutdoor_2__9_ 0x70003362
    0x0F12, 0x01E8, //#SARR_usDualGammaLutRGBOutdoor_2__10_0x70003364
    0x0F12, 0x020F, //#SARR_usDualGammaLutRGBOutdoor_2__11_0x70003366
    0x0F12, 0x0232, //#SARR_usDualGammaLutRGBOutdoor_2__12_0x70003368
    0x0F12, 0x0273, //#SARR_usDualGammaLutRGBOutdoor_2__13_0x7000336A
    0x0F12, 0x02AF, //#SARR_usDualGammaLutRGBOutdoor_2__14_0x7000336C
    0x0F12, 0x0309, //#SARR_usDualGammaLutRGBOutdoor_2__15_0x7000336E
    0x0F12, 0x0355, //#SARR_usDualGammaLutRGBOutdoor_2__16_0x70003370
    0x0F12, 0x0394, //#SARR_usDualGammaLutRGBOutdoor_2__17_0x70003372
    0x0F12, 0x03CE, //#SARR_usDualGammaLutRGBOutdoor_2__18_0x70003374
    0x0F12, 0x03FF, //#SARR_usDualGammaLutRGBOutdoor_2__19_0x70003376
    //================================================================================================
    // SET AFIT
    //================================================================================================
    // Noise index
    0x002A, 0x0764,
    0x0F12, 0x0041, //#afit_uNoiseIndInDoor[0] // 65
    0x0F12, 0x0063, //#afit_uNoiseIndInDoor[1] // 99
    0x0F12, 0x00C8, //#afit_uNoiseIndInDoor[2] // 187
    0x0F12, 0x01F4, //#afit_uNoiseIndInDoor[3] // 403->380
    0x0F12, 0x028A, //#afit_uNoiseIndInDoor[4] // 700
    // AFIT table start address // 7000_07C4
    0x002A, 0x0770,
    0x0F12, 0x07C4,
    0x0F12, 0x7000,
    // AFIT table (Variables)
    0x002A, 0x07C4,
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[0]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[1]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[2]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[3]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[4]
    0x0F12, 0x00C4, //#TVAR_afit_pBaseVals[5]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[6]
    0x0F12, 0x009C, //#TVAR_afit_pBaseVals[7]
    0x0F12, 0x017C, //#TVAR_afit_pBaseVals[8]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[9]
    0x0F12, 0x000C, //#TVAR_afit_pBaseVals[10]
    0x0F12, 0x0010, //#TVAR_afit_pBaseVals[11]
    0x0F12, 0x012C, //#TVAR_afit_pBaseVals[12]
    0x0F12, 0x03E8, //#TVAR_afit_pBaseVals[13]
    0x0F12, 0x0046, //#TVAR_afit_pBaseVals[14]
    0x0F12, 0x005A, //#TVAR_afit_pBaseVals[15]
    0x0F12, 0x0070, //#TVAR_afit_pBaseVals[16]
    0x0F12, 0x0019, //#TVAR_afit_pBaseVals[17]
    0x0F12, 0x0019, //#TVAR_afit_pBaseVals[18]
    0x0F12, 0x01AA, //#TVAR_afit_pBaseVals[19]
    0x0F12, 0x0064, //#TVAR_afit_pBaseVals[20]
    0x0F12, 0x0064, //#TVAR_afit_pBaseVals[21]
    0x0F12, 0x000A, //#TVAR_afit_pBaseVals[22]
    0x0F12, 0x000A, //#TVAR_afit_pBaseVals[23]
    0x0F12, 0x0096, //#TVAR_afit_pBaseVals[24]
    0x0F12, 0x0019, //#TVAR_afit_pBaseVals[25]
    0x0F12, 0x002A, //#TVAR_afit_pBaseVals[26]
    0x0F12, 0x0024, //#TVAR_afit_pBaseVals[27]
    0x0F12, 0x002A, //#TVAR_afit_pBaseVals[28]
    0x0F12, 0x0024, //#TVAR_afit_pBaseVals[29]
    0x0F12, 0x0A24, //#TVAR_afit_pBaseVals[30]
    0x0F12, 0x1701, //#TVAR_afit_pBaseVals[31]
    0x0F12, 0x0229, //#TVAR_afit_pBaseVals[32]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[33]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[34]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[35]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[36]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[37]
    0x0F12, 0x043B, //#TVAR_afit_pBaseVals[38]
    0x0F12, 0x1414, //#TVAR_afit_pBaseVals[39]
    0x0F12, 0x0301, //#TVAR_afit_pBaseVals[40]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[41]
    0x0F12, 0x051E, //#TVAR_afit_pBaseVals[42]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[43]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[44]
    0x0F12, 0x0A05, //#TVAR_afit_pBaseVals[45]
    0x0F12, 0x143C, //#TVAR_afit_pBaseVals[46]
    0x0F12, 0x0A14, //#TVAR_afit_pBaseVals[47]
    0x0F12, 0x0002, //#TVAR_afit_pBaseVals[48]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[49]
    0x0F12, 0x1102, //#TVAR_afit_pBaseVals[50]
    0x0F12, 0x001B, //#TVAR_afit_pBaseVals[51]
    0x0F12, 0x0900, //#TVAR_afit_pBaseVals[52]
    0x0F12, 0x0600, //#TVAR_afit_pBaseVals[53]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[54]
    0x0F12, 0x0305, //#TVAR_afit_pBaseVals[55]
    0x0F12, 0x3C03, //#TVAR_afit_pBaseVals[56]
    0x0F12, 0x006E, //#TVAR_afit_pBaseVals[57]
    0x0F12, 0x0078, //#TVAR_afit_pBaseVals[58]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[59]
    0x0F12, 0x1414, //#TVAR_afit_pBaseVals[60]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[61]
    0x0F12, 0x5002, //#TVAR_afit_pBaseVals[62]
    0x0F12, 0x7850, //#TVAR_afit_pBaseVals[63]
    0x0F12, 0x2878, //#TVAR_afit_pBaseVals[64]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[65]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[66]
    0x0F12, 0x1E0C, //#TVAR_afit_pBaseVals[67]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[68]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[69]
    0x0F12, 0x4104, //#TVAR_afit_pBaseVals[70]
    0x0F12, 0x123C, //#TVAR_afit_pBaseVals[71]
    0x0F12, 0x4012, //#TVAR_afit_pBaseVals[72]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[73]
    0x0F12, 0x1E03, //#TVAR_afit_pBaseVals[74]
    0x0F12, 0x011E, //#TVAR_afit_pBaseVals[75]
    0x0F12, 0x0201, //#TVAR_afit_pBaseVals[76]
    0x0F12, 0x5050, //#TVAR_afit_pBaseVals[77]
    0x0F12, 0x3C3C, //#TVAR_afit_pBaseVals[78]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[79]
    0x0F12, 0x030A, //#TVAR_afit_pBaseVals[80]
    0x0F12, 0x0714, //#TVAR_afit_pBaseVals[81]
    0x0F12, 0x0A1E, //#TVAR_afit_pBaseVals[82]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[83]
    0x0F12, 0x0432, //#TVAR_afit_pBaseVals[84]
    0x0F12, 0x4050, //#TVAR_afit_pBaseVals[85]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[86]
    0x0F12, 0x0440, //#TVAR_afit_pBaseVals[87]
    0x0F12, 0x0302, //#TVAR_afit_pBaseVals[88]
    0x0F12, 0x1E1E, //#TVAR_afit_pBaseVals[89]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[90]
    0x0F12, 0x5002, //#TVAR_afit_pBaseVals[91]
    0x0F12, 0x3C50, //#TVAR_afit_pBaseVals[92]
    0x0F12, 0x283C, //#TVAR_afit_pBaseVals[93]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[94]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[95]
    0x0F12, 0x1E07, //#TVAR_afit_pBaseVals[96]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[97]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[98]
    0x0F12, 0x5004, //#TVAR_afit_pBaseVals[99]
    0x0F12, 0x0F40, //#TVAR_afit_pBaseVals[100]
    0x0F12, 0x400F, //#TVAR_afit_pBaseVals[101]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[102]
    0x0F12, 0x0003, //#TVAR_afit_pBaseVals[103]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[104]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[105]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[106]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[107]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[108]
    0x0F12, 0x00C4, //#TVAR_afit_pBaseVals[109]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[110]
    0x0F12, 0x009C, //#TVAR_afit_pBaseVals[111]
    0x0F12, 0x017C, //#TVAR_afit_pBaseVals[112]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[113]
    0x0F12, 0x000C, //#TVAR_afit_pBaseVals[114]
    0x0F12, 0x0010, //#TVAR_afit_pBaseVals[115]
    0x0F12, 0x012C, //#TVAR_afit_pBaseVals[116]
    0x0F12, 0x03E8, //#TVAR_afit_pBaseVals[117]
    0x0F12, 0x0046, //#TVAR_afit_pBaseVals[118]
    0x0F12, 0x005A, //#TVAR_afit_pBaseVals[119]
    0x0F12, 0x0070, //#TVAR_afit_pBaseVals[120]
    0x0F12, 0x000F, //#TVAR_afit_pBaseVals[121]
    0x0F12, 0x000F, //#TVAR_afit_pBaseVals[122]
    0x0F12, 0x01AA, //#TVAR_afit_pBaseVals[123]
    0x0F12, 0x003C, //#TVAR_afit_pBaseVals[124]
    0x0F12, 0x003C, //#TVAR_afit_pBaseVals[125]
    0x0F12, 0x0005, //#TVAR_afit_pBaseVals[126]
    0x0F12, 0x0005, //#TVAR_afit_pBaseVals[127]
    0x0F12, 0x0096, //#TVAR_afit_pBaseVals[128]
    0x0F12, 0x0023, //#TVAR_afit_pBaseVals[129]
    0x0F12, 0x002A, //#TVAR_afit_pBaseVals[130]
    0x0F12, 0x0024, //#TVAR_afit_pBaseVals[131]
    0x0F12, 0x002A, //#TVAR_afit_pBaseVals[132]
    0x0F12, 0x0024, //#TVAR_afit_pBaseVals[133]
    0x0F12, 0x0A24, //#TVAR_afit_pBaseVals[134]
    0x0F12, 0x1701, //#TVAR_afit_pBaseVals[135]
    0x0F12, 0x0229, //#TVAR_afit_pBaseVals[136]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[137]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[138]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[139]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[140]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[141]
    0x0F12, 0x043B, //#TVAR_afit_pBaseVals[142]
    0x0F12, 0x1414, //#TVAR_afit_pBaseVals[143]
    0x0F12, 0x0301, //#TVAR_afit_pBaseVals[144]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[145]
    0x0F12, 0x051E, //#TVAR_afit_pBaseVals[146]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[147]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[148]
    0x0F12, 0x0A05, //#TVAR_afit_pBaseVals[149]
    0x0F12, 0x143C, //#TVAR_afit_pBaseVals[150]
    0x0F12, 0x0A14, //#TVAR_afit_pBaseVals[151]
    0x0F12, 0x0002, //#TVAR_afit_pBaseVals[152]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[153]
    0x0F12, 0x1102, //#TVAR_afit_pBaseVals[154]
    0x0F12, 0x001B, //#TVAR_afit_pBaseVals[155]
    0x0F12, 0x0900, //#TVAR_afit_pBaseVals[156]
    0x0F12, 0x0600, //#TVAR_afit_pBaseVals[157]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[158]
    0x0F12, 0x0305, //#TVAR_afit_pBaseVals[159]
    0x0F12, 0x4603, //#TVAR_afit_pBaseVals[160]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[161]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[162]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[163]
    0x0F12, 0x1919, //#TVAR_afit_pBaseVals[164]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[165]
    0x0F12, 0x3C02, //#TVAR_afit_pBaseVals[166]
    0x0F12, 0x643C, //#TVAR_afit_pBaseVals[167]
    0x0F12, 0x2864, //#TVAR_afit_pBaseVals[168]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[169]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[170]
    0x0F12, 0x1E0C, //#TVAR_afit_pBaseVals[171]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[172]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[173]
    0x0F12, 0x4104, //#TVAR_afit_pBaseVals[174]
    0x0F12, 0x123C, //#TVAR_afit_pBaseVals[175]
    0x0F12, 0x4012, //#TVAR_afit_pBaseVals[176]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[177]
    0x0F12, 0x1E03, //#TVAR_afit_pBaseVals[178]
    0x0F12, 0x011E, //#TVAR_afit_pBaseVals[179]
    0x0F12, 0x0201, //#TVAR_afit_pBaseVals[180]
    0x0F12, 0x3232, //#TVAR_afit_pBaseVals[181]
    0x0F12, 0x3C3C, //#TVAR_afit_pBaseVals[182]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[183]
    0x0F12, 0x030A, //#TVAR_afit_pBaseVals[184]
    0x0F12, 0x0714, //#TVAR_afit_pBaseVals[185]
    0x0F12, 0x0A1E, //#TVAR_afit_pBaseVals[186]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[187]
    0x0F12, 0x0432, //#TVAR_afit_pBaseVals[188]
    0x0F12, 0x4050, //#TVAR_afit_pBaseVals[189]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[190]
    0x0F12, 0x0440, //#TVAR_afit_pBaseVals[191]
    0x0F12, 0x0302, //#TVAR_afit_pBaseVals[192]
    0x0F12, 0x1E1E, //#TVAR_afit_pBaseVals[193]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[194]
    0x0F12, 0x3202, //#TVAR_afit_pBaseVals[195]
    0x0F12, 0x3C32, //#TVAR_afit_pBaseVals[196]
    0x0F12, 0x283C, //#TVAR_afit_pBaseVals[197]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[198]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[199]
    0x0F12, 0x1E07, //#TVAR_afit_pBaseVals[200]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[201]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[202]
    0x0F12, 0x5004, //#TVAR_afit_pBaseVals[203]
    0x0F12, 0x0F40, //#TVAR_afit_pBaseVals[204]
    0x0F12, 0x400F, //#TVAR_afit_pBaseVals[205]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[206]
    0x0F12, 0x0003, //#TVAR_afit_pBaseVals[207]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[208]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[209]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[210]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[211]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[212]
    0x0F12, 0x00C4, //#TVAR_afit_pBaseVals[213]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[214]
    0x0F12, 0x009C, //#TVAR_afit_pBaseVals[215]
    0x0F12, 0x017C, //#TVAR_afit_pBaseVals[216]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[217]
    0x0F12, 0x000C, //#TVAR_afit_pBaseVals[218]
    0x0F12, 0x0010, //#TVAR_afit_pBaseVals[219]
    0x0F12, 0x012C, //#TVAR_afit_pBaseVals[220]
    0x0F12, 0x03E8, //#TVAR_afit_pBaseVals[221]
    0x0F12, 0x0046, //#TVAR_afit_pBaseVals[222]
    0x0F12, 0x0078, //#TVAR_afit_pBaseVals[223]
    0x0F12, 0x0070, //#TVAR_afit_pBaseVals[224]
    0x0F12, 0x0004, //#TVAR_afit_pBaseVals[225]
    0x0F12, 0x0004, //#TVAR_afit_pBaseVals[226]
    0x0F12, 0x01AA, //#TVAR_afit_pBaseVals[227]
    0x0F12, 0x001E, //#TVAR_afit_pBaseVals[228]
    0x0F12, 0x001E, //#TVAR_afit_pBaseVals[229]
    0x0F12, 0x0005, //#TVAR_afit_pBaseVals[230]
    0x0F12, 0x0005, //#TVAR_afit_pBaseVals[231]
    0x0F12, 0x0096, //#TVAR_afit_pBaseVals[232]
    0x0F12, 0x0023, //#TVAR_afit_pBaseVals[233]
    0x0F12, 0x002A, //#TVAR_afit_pBaseVals[234]
    0x0F12, 0x0024, //#TVAR_afit_pBaseVals[235]
    0x0F12, 0x002A, //#TVAR_afit_pBaseVals[236]
    0x0F12, 0x0024, //#TVAR_afit_pBaseVals[237]
    0x0F12, 0x0A24, //#TVAR_afit_pBaseVals[238]
    0x0F12, 0x1701, //#TVAR_afit_pBaseVals[239]
    0x0F12, 0x0229, //#TVAR_afit_pBaseVals[240]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[241]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[242]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[243]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[244]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[245]
    0x0F12, 0x043B, //#TVAR_afit_pBaseVals[246]
    0x0F12, 0x1414, //#TVAR_afit_pBaseVals[247]
    0x0F12, 0x0301, //#TVAR_afit_pBaseVals[248]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[249]
    0x0F12, 0x051E, //#TVAR_afit_pBaseVals[250]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[251]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[252]
    0x0F12, 0x0A05, //#TVAR_afit_pBaseVals[253]
    0x0F12, 0x143C, //#TVAR_afit_pBaseVals[254]
    0x0F12, 0x0514, //#TVAR_afit_pBaseVals[255]
    0x0F12, 0x0002, //#TVAR_afit_pBaseVals[256]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[257]
    0x0F12, 0x1102, //#TVAR_afit_pBaseVals[258]
    0x0F12, 0x001B, //#TVAR_afit_pBaseVals[259]
    0x0F12, 0x0900, //#TVAR_afit_pBaseVals[260]
    0x0F12, 0x0600, //#TVAR_afit_pBaseVals[261]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[262]
    0x0F12, 0x0305, //#TVAR_afit_pBaseVals[263]
    0x0F12, 0x4603, //#TVAR_afit_pBaseVals[264]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[265]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[266]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[267]
    0x0F12, 0x2323, //#TVAR_afit_pBaseVals[268]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[269]
    0x0F12, 0x2A02, //#TVAR_afit_pBaseVals[270]
    0x0F12, 0x462A, //#TVAR_afit_pBaseVals[271]
    0x0F12, 0x2846, //#TVAR_afit_pBaseVals[272]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[273]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[274]
    0x0F12, 0x1E0C, //#TVAR_afit_pBaseVals[275]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[276]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[277]
    0x0F12, 0x4B04, //#TVAR_afit_pBaseVals[278]
    0x0F12, 0x0F40, //#TVAR_afit_pBaseVals[279]
    0x0F12, 0x400F, //#TVAR_afit_pBaseVals[280]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[281]
    0x0F12, 0x2303, //#TVAR_afit_pBaseVals[282]
    0x0F12, 0x0123, //#TVAR_afit_pBaseVals[283]
    0x0F12, 0x0201, //#TVAR_afit_pBaseVals[284]
    0x0F12, 0x262A, //#TVAR_afit_pBaseVals[285]
    0x0F12, 0x2C2C, //#TVAR_afit_pBaseVals[286]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[287]
    0x0F12, 0x030A, //#TVAR_afit_pBaseVals[288]
    0x0F12, 0x0714, //#TVAR_afit_pBaseVals[289]
    0x0F12, 0x0A1E, //#TVAR_afit_pBaseVals[290]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[291]
    0x0F12, 0x0432, //#TVAR_afit_pBaseVals[292]
    0x0F12, 0x4050, //#TVAR_afit_pBaseVals[293]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[294]
    0x0F12, 0x0440, //#TVAR_afit_pBaseVals[295]
    0x0F12, 0x0302, //#TVAR_afit_pBaseVals[296]
    0x0F12, 0x2323, //#TVAR_afit_pBaseVals[297]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[298]
    0x0F12, 0x2A02, //#TVAR_afit_pBaseVals[299]
    0x0F12, 0x2C26, //#TVAR_afit_pBaseVals[300]
    0x0F12, 0x282C, //#TVAR_afit_pBaseVals[301]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[302]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[303]
    0x0F12, 0x1E07, //#TVAR_afit_pBaseVals[304]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[305]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[306]
    0x0F12, 0x5004, //#TVAR_afit_pBaseVals[307]
    0x0F12, 0x0F40, //#TVAR_afit_pBaseVals[308]
    0x0F12, 0x400F, //#TVAR_afit_pBaseVals[309]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[310]
    0x0F12, 0x0003, //#TVAR_afit_pBaseVals[311]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[312]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[313]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[314]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[315]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[316]
    0x0F12, 0x00C4, //#TVAR_afit_pBaseVals[317]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[318]
    0x0F12, 0x009C, //#TVAR_afit_pBaseVals[319]
    0x0F12, 0x017C, //#TVAR_afit_pBaseVals[320]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[321]
    0x0F12, 0x000C, //#TVAR_afit_pBaseVals[322]
    0x0F12, 0x0010, //#TVAR_afit_pBaseVals[323]
    0x0F12, 0x00C8, //#TVAR_afit_pBaseVals[324]
    0x0F12, 0x0384, //#TVAR_afit_pBaseVals[325]
    0x0F12, 0x0046, //#TVAR_afit_pBaseVals[326]
    0x0F12, 0x0082, //#TVAR_afit_pBaseVals[327]
    0x0F12, 0x0070, //#TVAR_afit_pBaseVals[328]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[329]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[330]
    0x0F12, 0x01AA, //#TVAR_afit_pBaseVals[331]
    0x0F12, 0x001E, //#TVAR_afit_pBaseVals[332]
    0x0F12, 0x001E, //#TVAR_afit_pBaseVals[333]
    0x0F12, 0x000A, //#TVAR_afit_pBaseVals[334]
    0x0F12, 0x000A, //#TVAR_afit_pBaseVals[335]
    0x0F12, 0x010E, //#TVAR_afit_pBaseVals[336]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[337]
    0x0F12, 0x0032, //#TVAR_afit_pBaseVals[338]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[339]
    0x0F12, 0x0032, //#TVAR_afit_pBaseVals[340]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[341]
    0x0F12, 0x0A24, //#TVAR_afit_pBaseVals[342]
    0x0F12, 0x1701, //#TVAR_afit_pBaseVals[343]
    0x0F12, 0x0229, //#TVAR_afit_pBaseVals[344]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[345]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[346]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[347]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[348]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[349]
    0x0F12, 0x043B, //#TVAR_afit_pBaseVals[350]
    0x0F12, 0x1414, //#TVAR_afit_pBaseVals[351]
    0x0F12, 0x0301, //#TVAR_afit_pBaseVals[352]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[353]
    0x0F12, 0x051E, //#TVAR_afit_pBaseVals[354]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[355]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[356]
    0x0F12, 0x0A05, //#TVAR_afit_pBaseVals[357]
    0x0F12, 0x143C, //#TVAR_afit_pBaseVals[358]
    0x0F12, 0x0514, //#TVAR_afit_pBaseVals[359]
    0x0F12, 0x0002, //#TVAR_afit_pBaseVals[360]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[361]
    0x0F12, 0x1002, //#TVAR_afit_pBaseVals[362]
    0x0F12, 0x001E, //#TVAR_afit_pBaseVals[363]
    0x0F12, 0x0900, //#TVAR_afit_pBaseVals[364]
    0x0F12, 0x0600, //#TVAR_afit_pBaseVals[365]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[366]
    0x0F12, 0x0305, //#TVAR_afit_pBaseVals[367]
    0x0F12, 0x4602, //#TVAR_afit_pBaseVals[368]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[369]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[370]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[371]
    0x0F12, 0x2328, //#TVAR_afit_pBaseVals[372]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[373]
    0x0F12, 0x2A02, //#TVAR_afit_pBaseVals[374]
    0x0F12, 0x2228, //#TVAR_afit_pBaseVals[375]
    0x0F12, 0x2822, //#TVAR_afit_pBaseVals[376]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[377]
    0x0F12, 0x1903, //#TVAR_afit_pBaseVals[378]
    0x0F12, 0x1E0F, //#TVAR_afit_pBaseVals[379]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[380]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[381]
    0x0F12, 0x9604, //#TVAR_afit_pBaseVals[382]
    0x0F12, 0x0F42, //#TVAR_afit_pBaseVals[383]
    0x0F12, 0x400F, //#TVAR_afit_pBaseVals[384]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[385]
    0x0F12, 0x2805, //#TVAR_afit_pBaseVals[386]
    0x0F12, 0x0123, //#TVAR_afit_pBaseVals[387]
    0x0F12, 0x0201, //#TVAR_afit_pBaseVals[388]
    0x0F12, 0x2024, //#TVAR_afit_pBaseVals[389]
    0x0F12, 0x1C1C, //#TVAR_afit_pBaseVals[390]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[391]
    0x0F12, 0x030A, //#TVAR_afit_pBaseVals[392]
    0x0F12, 0x0A0A, //#TVAR_afit_pBaseVals[393]
    0x0F12, 0x0A2D, //#TVAR_afit_pBaseVals[394]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[395]
    0x0F12, 0x0432, //#TVAR_afit_pBaseVals[396]
    0x0F12, 0x4050, //#TVAR_afit_pBaseVals[397]
    0x0F12, 0x0F0F, //#TVAR_afit_pBaseVals[398]
    0x0F12, 0x0440, //#TVAR_afit_pBaseVals[399]
    0x0F12, 0x0302, //#TVAR_afit_pBaseVals[400]
    0x0F12, 0x2328, //#TVAR_afit_pBaseVals[401]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[402]
    0x0F12, 0x3C02, //#TVAR_afit_pBaseVals[403]
    0x0F12, 0x1C3C, //#TVAR_afit_pBaseVals[404]
    0x0F12, 0x281C, //#TVAR_afit_pBaseVals[405]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[406]
    0x0F12, 0x0A03, //#TVAR_afit_pBaseVals[407]
    0x0F12, 0x2D0A, //#TVAR_afit_pBaseVals[408]
    0x0F12, 0x070A, //#TVAR_afit_pBaseVals[409]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[410]
    0x0F12, 0x5004, //#TVAR_afit_pBaseVals[411]
    0x0F12, 0x0F40, //#TVAR_afit_pBaseVals[412]
    0x0F12, 0x400F, //#TVAR_afit_pBaseVals[413]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[414]
    0x0F12, 0x0003, //#TVAR_afit_pBaseVals[415]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[416]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[417]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[418]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[419]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[420]
    0x0F12, 0x00C4, //#TVAR_afit_pBaseVals[421]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[422]
    0x0F12, 0x009C, //#TVAR_afit_pBaseVals[423]
    0x0F12, 0x017C, //#TVAR_afit_pBaseVals[424]
    0x0F12, 0x03FF, //#TVAR_afit_pBaseVals[425]
    0x0F12, 0x000C, //#TVAR_afit_pBaseVals[426]
    0x0F12, 0x0010, //#TVAR_afit_pBaseVals[427]
    0x0F12, 0x00C8, //#TVAR_afit_pBaseVals[428]
    0x0F12, 0x0320, //#TVAR_afit_pBaseVals[429]
    0x0F12, 0x0046, //#TVAR_afit_pBaseVals[430]
    0x0F12, 0x015E, //#TVAR_afit_pBaseVals[431]
    0x0F12, 0x0070, //#TVAR_afit_pBaseVals[432]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[433]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[434]
    0x0F12, 0x01AA, //#TVAR_afit_pBaseVals[435]
    0x0F12, 0x0014, //#TVAR_afit_pBaseVals[436]
    0x0F12, 0x0014, //#TVAR_afit_pBaseVals[437]
    0x0F12, 0x000A, //#TVAR_afit_pBaseVals[438]
    0x0F12, 0x000A, //#TVAR_afit_pBaseVals[439]
    0x0F12, 0x0140, //#TVAR_afit_pBaseVals[440]
    0x0F12, 0x003C, //#TVAR_afit_pBaseVals[441]
    0x0F12, 0x0032, //#TVAR_afit_pBaseVals[442]
    0x0F12, 0x0023, //#TVAR_afit_pBaseVals[443]
    0x0F12, 0x0023, //#TVAR_afit_pBaseVals[444]
    0x0F12, 0x0032, //#TVAR_afit_pBaseVals[445]
    0x0F12, 0x0A24, //#TVAR_afit_pBaseVals[446]
    0x0F12, 0x1701, //#TVAR_afit_pBaseVals[447]
    0x0F12, 0x0229, //#TVAR_afit_pBaseVals[448]
    0x0F12, 0x1403, //#TVAR_afit_pBaseVals[449]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[450]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[451]
    0x0F12, 0x0505, //#TVAR_afit_pBaseVals[452]
    0x0F12, 0x00FF, //#TVAR_afit_pBaseVals[453]
    0x0F12, 0x043B, //#TVAR_afit_pBaseVals[454]
    0x0F12, 0x1414, //#TVAR_afit_pBaseVals[455]
    0x0F12, 0x0301, //#TVAR_afit_pBaseVals[456]
    0x0F12, 0xFF07, //#TVAR_afit_pBaseVals[457]
    0x0F12, 0x051E, //#TVAR_afit_pBaseVals[458]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[459]
    0x0F12, 0x0000, //#TVAR_afit_pBaseVals[460]
    0x0F12, 0x0A05, //#TVAR_afit_pBaseVals[461]
    0x0F12, 0x143C, //#TVAR_afit_pBaseVals[462]
    0x0F12, 0x0514, //#TVAR_afit_pBaseVals[463]
    0x0F12, 0x0002, //#TVAR_afit_pBaseVals[464]
    0x0F12, 0x0096, //#TVAR_afit_pBaseVals[465]
    0x0F12, 0x1002, //#TVAR_afit_pBaseVals[466]
    0x0F12, 0x001E, //#TVAR_afit_pBaseVals[467]
    0x0F12, 0x0900, //#TVAR_afit_pBaseVals[468]
    0x0F12, 0x0600, //#TVAR_afit_pBaseVals[469]
    0x0F12, 0x0504, //#TVAR_afit_pBaseVals[470]
    0x0F12, 0x0305, //#TVAR_afit_pBaseVals[471]
    0x0F12, 0x6402, //#TVAR_afit_pBaseVals[472]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[473]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[474]
    0x0F12, 0x0080, //#TVAR_afit_pBaseVals[475]
    0x0F12, 0x5050, //#TVAR_afit_pBaseVals[476]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[477]
    0x0F12, 0x1C02, //#TVAR_afit_pBaseVals[478]
    0x0F12, 0x191C, //#TVAR_afit_pBaseVals[479]
    0x0F12, 0x2819, //#TVAR_afit_pBaseVals[480]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[481]
    0x0F12, 0x1E03, //#TVAR_afit_pBaseVals[482]
    0x0F12, 0x1E0F, //#TVAR_afit_pBaseVals[483]
    0x0F12, 0x0508, //#TVAR_afit_pBaseVals[484]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[485]
    0x0F12, 0xAA04, //#TVAR_afit_pBaseVals[486]
    0x0F12, 0x1452, //#TVAR_afit_pBaseVals[487]
    0x0F12, 0x4015, //#TVAR_afit_pBaseVals[488]
    0x0F12, 0x0604, //#TVAR_afit_pBaseVals[489]
    0x0F12, 0x5006, //#TVAR_afit_pBaseVals[490]
    0x0F12, 0x0150, //#TVAR_afit_pBaseVals[491]
    0x0F12, 0x0201, //#TVAR_afit_pBaseVals[492]
    0x0F12, 0x1E1E, //#TVAR_afit_pBaseVals[493]
    0x0F12, 0x1212, //#TVAR_afit_pBaseVals[494]
    0x0F12, 0x0028, //#TVAR_afit_pBaseVals[495]
    0x0F12, 0x030A, //#TVAR_afit_pBaseVals[496]
    0x0F12, 0x0A10, //#TVAR_afit_pBaseVals[497]
    0x0F12, 0x0819, //#TVAR_afit_pBaseVals[498]
    0x0F12, 0xFF05, //#TVAR_afit_pBaseVals[499]
    0x0F12, 0x0432, //#TVAR_afit_pBaseVals[500]
    0x0F12, 0x4052, //#TVAR_afit_pBaseVals[501]
    0x0F12, 0x1514, //#TVAR_afit_pBaseVals[502]
    0x0F12, 0x0440, //#TVAR_afit_pBaseVals[503]
    0x0F12, 0x0302, //#TVAR_afit_pBaseVals[504]
    0x0F12, 0x5050, //#TVAR_afit_pBaseVals[505]
    0x0F12, 0x0101, //#TVAR_afit_pBaseVals[506]
    0x0F12, 0x1E02, //#TVAR_afit_pBaseVals[507]
    0x0F12, 0x121E, //#TVAR_afit_pBaseVals[508]
    0x0F12, 0x2812, //#TVAR_afit_pBaseVals[509]
    0x0F12, 0x0A00, //#TVAR_afit_pBaseVals[510]
    0x0F12, 0x1003, //#TVAR_afit_pBaseVals[511]
    0x0F12, 0x190A, //#TVAR_afit_pBaseVals[512]
    0x0F12, 0x0508, //#TVAR_afit_pBaseVals[513]
    0x0F12, 0x32FF, //#TVAR_afit_pBaseVals[514]
    0x0F12, 0x5204, //#TVAR_afit_pBaseVals[515]
    0x0F12, 0x1440, //#TVAR_afit_pBaseVals[516]
    0x0F12, 0x4015, //#TVAR_afit_pBaseVals[517]
    0x0F12, 0x0204, //#TVAR_afit_pBaseVals[518]
    0x0F12, 0x0003, //#TVAR_afit_pBaseVals[519]

    // AFIT table (Constants)
    0x0F12, 0x7F7A, //#afit_pConstBaseVals[0]
    0x0F12, 0x7FBD, //#afit_pConstBaseVals[1]
    0x0F12, 0xBEFC, //#afit_pConstBaseVals[2]
    0x0F12, 0xF7BC, //#afit_pConstBaseVals[3]
    0x0F12, 0x7E06, //#afit_pConstBaseVals[4]
    0x0F12, 0x0053, //#afit_pConstBaseVals[5]

    // Update Changed Registers
    0x002A, 0x0664,
    0x0F12, 0x013E, //seti_uContrastCenter


};

static unsigned short s5k5cagx_init_reg_short5[]={

    //================================================================================================
    // SET PLL
    //================================================================================================
    // How to set
    // 1. MCLK
    //hex(CLK you want) * 1000)
    //// 2. System CLK
    //hex((CLK you want) * 1000 / 4)
    //// 3. PCLK
    //hex((CLK you want) * 1000 / 4)
    //================================================================================================
    // Set input CLK // 24.5MHz
    0x002A, 0x01CC,
    //0x0F12, 0x5DC0, //#REG_TC_IPRM_InClockLSBs  24M
    0x0F12, 0xDAC0, //#REG_TC_IPRM_InClockLSBs  56M
    0x0F12, 0x0000, //#REG_TC_IPRM_InClockMSBs

    0x002A, 0x01EE,
    0x0F12, 0x0001, //#REG_TC_IPRM_UseNPviClocks // Number of PLL setting
    // Set system CLK
    0x002A, 0x01F6,
    0x0F12, 0x6D60, //#REG_TC_IPRM_OpClk4KHz_0, 112MHz
    0x0F12, 0x6D10, //#REG_TC_IPRM_MinOutRate4KHz_0, 111.68MHz
    0x0F12, 0x6D60, //#REG_TC_IPRM_MaxOutRate4KHz_0, 112MHz

    // Update PLL
    0x002A, 0x0208,
    0x0F12, 0x0001, //#REG_TC_IPRM_InitParamsUpdated

    //================================================================================================
    // SET PREVIEW CONFIGURATION_0
    // # Foramt : YUV422
    // # Size: VGA
    // # FPS : 22~10fps
    //================================================================================================
    0x002A, 0x026C,
    0x0F12, 800, //#REG_0TC_PCFG_usWidth//800
    0x0F12, 600, //#REG_0TC_PCFG_usHeight//600
    0x0F12, 0x0005, //#REG_0TC_PCFG_Format
    0x0F12, 0x3AA4, //#REG_0TC_PCFG_usMaxOut4KHzRate
    0x0F12, 0x3A8B, //#REG_0TC_PCFG_usMinOut4KHzRate
    0x0F12, 0x0100, //#REG_0TC_PCFG_OutClkPerPix88
    0x0F12, 0x0800, //#REG_0TC_PCFG_uMaxBpp88
    0x0F12, 0x0092, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg
    0x0F12, 0x0018, //#REG_0TC_PCFG_OIFMask
    0x0F12, 0x01E0, //#REG_0TC_PCFG_usJpegPacketSize
    0x0F12, 0x0000, //#REG_0TC_PCFG_usJpegTotalPackets
    0x0F12, 0x0000, //#REG_0TC_PCFG_uClockInd
    0x0F12, 0x0000, //#REG_0TC_PCFG_usFrTimeType
    0x0F12, 0x0001, //#REG_0TC_PCFG_FrRateQualityType
    0x0F12, 0x0294, //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //15fps
    0x0F12, 0x014A, //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //30fps
    0x0F12, 0x0000, //#REG_0TC_PCFG_bSmearOutput
    0x0F12, 0x0000, //#REG_0TC_PCFG_sSaturation
    0x0F12, 0x0000, //#REG_0TC_PCFG_sSharpBlur
    0x0F12, 0x0000, //#REG_0TC_PCFG_sColorTemp
    0x0F12, 0x0000, //#REG_0TC_PCFG_uDeviceGammaIndex
    0x0F12, 0x0000, //#REG_0TC_PCFG_uPrevMirror
    0x0F12, 0x0000, //#REG_0TC_PCFG_uCaptureMirror
    0x0F12, 0x0000, //#REG_0TC_PCFG_uRotation

    // 656format
    0x002A, 0x04CC,
    0x0F12, 0x0011,

    0x002A, 0x0208,
    0x0F12, 0x0001,

    //================================================================================================
    // APPLY PREVIEW CONFIGURATION & RUN PREVIEW
    //================================================================================================
    0x002A, 0x023C,
    0x0F12, 0x0000, //#REG_TC_GP_ActivePrevConfig // Select preview configuration_0
    0x002A, 0x0240,
    0x0F12, 0x0001, //#REG_TC_GP_PrevOpenAfterChange
    0x002A, 0x0230,
    0x0F12, 0x0001, //#REG_TC_GP_NewConfigSync // Update preview configuration
    0x002A, 0x023E,
    0x0F12, 0x0001, //#REG_TC_GP_PrevConfigChanged
    0x002A, 0x0220,
    0x0F12, 0x0001, //#REG_TC_GP_EnablePreview // Start preview
    0x0F12, 0x0001, //#REG_TC_GP_EnablePreviewChanged

    // hidden 656
    0x0028, 0xD000,
    0x002A, 0xB056,
    0x0F12, 0x0060,

    //================================================================================================
    // SET CAPTURE CONFIGURATION_0
    // # Foramt :JPEG
    // # Size: QXGA
    // # FPS : 7.5fps
    //================================================================================================
    0x002A, 0x035C,
    0x0F12, 0x0000, //#REG_0TC_CCFG_uCaptureModeJpEG
    0x0F12, 2084, //#REG_0TC_CCFG_usWidth
    0x0F12, 1536, //#REG_0TC_CCFG_usHeight
    0x0F12, 0x0005, //#REG_0TC_CCFG_Format//5:YUV 9:JPEG
    0x0F12, 0x3AA4, //#REG_0TC_CCFG_usMaxOut4KHzRate
    0x0F12, 0x3A8B, //#REG_0TC_CCFG_usMinOut4KHzRate
    0x0F12, 0x0100, //#REG_0TC_CCFG_OutClkPerPix88
    0x0F12, 0x0800, //#REG_0TC_CCFG_uMaxBpp88
    0x0F12, 0x0092, //#REG_0TC_CCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800
    0x0F12, 0x0018, //#REG_0TC_CCFG_OIFMask
    0x0F12, 0x01E0, //#REG_0TC_CCFG_usJpegPacketSize
    0x0F12, 0x0000, //#REG_0TC_CCFG_usJpegTotalPackets
    0x0F12, 0x0000, //#REG_0TC_CCFG_uClockInd
    0x0F12, 0x0000, //#REG_0TC_CCFG_usFrTimeType
    0x0F12, 0x0002, //#REG_0TC_CCFG_FrRateQualityType
    0x0F12, 0x0294, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //15fps
    0x0F12, 0x0535, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //7.5fps
    0x0F12, 0x0000, //#REG_0TC_CCFG_bSmearOutput
    0x0F12, 0x0000, //#REG_0TC_CCFG_sSaturation
    0x0F12, 0x0000, //#REG_0TC_CCFG_sSharpBlur
    0x0F12, 0x0000, //#REG_0TC_CCFG_sColorTemp
    0x0F12, 0x0000, //#REG_0TC_CCFG_uDeviceGammaIndex
    0x002A, 0x021A,
    0x0F12, 0x0000, //disable //#REG_TC_bUseMBR

    // Fill RAM with alternative op-codes
    0x0028, 0x7000, //start add MSW
    0x002A, 0x2CE8, //start add LSW
    0x0F12, 0x0007, //Modify LSB to control AWBB_YThreshLow
    0x0F12, 0x00e2,
    0x0F12, 0x0005, //Modify LSB to control AWBB_YThreshLowBrLow
    0x0F12, 0x00e2,

    0x0028, 0xD000,
    0x002A, 0x1000,
    0x0F12, 0x0001,

    0x0028, 0x7000,

    0xffff, 200, // delay

    0x002A, 0x0252,
    0x0F12, 0x0006, //#init  //AF

};

// capture preset
static unsigned short  s5k5cagx_capture_preset_0[] = {
    //This code is for 3M Full YUV Capture Mode.
    //If you want initial(Preview)mode, comment below code.

    0x0028 ,0x7000,
    0x002a ,0x0244,
    0x0f12 ,0x0000, // #REG_TC_GP_ActiveCapConfig//capture config
    0x0f12 ,0x0001, // #REG_TC_GP_CapConfigChanged
    0x002a ,0x0230,
    0x0f12 ,0x0001, // #REG_TC_GP_NewConfigSync//config ¨¬?¡Æ©¡Au¢¯e
    0x002a ,0x0224,
    0x0f12 ,0x0001, // #REG_TC_GP_EnableCapture
    0x0f12 ,0x0001, // #REG_TC_GP_EnableCaptureChanged
};

static unsigned short s5k5cagx_preview_preset_0[]={
    // Select and apply preview configuration

    0x0028 ,0x7000,
    0x002A, 0x026C,
    0x0F12, 640, //#REG_0TC_PCFG_usWidth
    0x0F12, 480, //#REG_0TC_PCFG_usHeight

    0x002A ,0x168E,
    0x0f12 ,0x0800,
    0x002A ,0x023C,		//#REG_TC_GP_ActivePrevConfig
    0x0F12 ,0x0000,
    0x002A ,0x0240,		//#REG_TC_GP_PrevOpenAfterChange
    0x0F12 ,0x0001,
    0x002A ,0x0230,		//#REG_TC_GP_NewConfigSync
    0x0F12 ,0x0001,
    0x002A ,0x023E,		//#REG_TC_GP_PrevConfigChanged
    0x0F12 ,0x0001,

    0x002A, 0x0252,
    0x0F12, 0x0006, //  //AF
};

static unsigned short s5k5cagx_movie_preset_0[]={
    // Select and apply preview configuration
    0x0028 ,0x7000,

    0x002A, 0x026C,
    0x0F12, 720, //#REG_0TC_PCFG_usWidth
    0x0F12, 480, //#REG_0TC_PCFG_usHeight

    0x002A ,0x168E,
    0x0f12 ,0x0800,
    0x002A ,0x023C,		//#REG_TC_GP_ActivePrevConfig
    0x0F12 ,0x0000,
    0x002A ,0x0240,		//#REG_TC_GP_PrevOpenAfterChange
    0x0F12 ,0x0001,
    0x002A ,0x0230,		//#REG_TC_GP_NewConfigSync
    0x0F12 ,0x0001,
    0x002A ,0x023E,		//#REG_TC_GP_PrevConfigChanged
    0x0F12 ,0x0001,

    0x002A, 0x0252,
    0x0F12, 0x0005, //  //MF
};

static unsigned short s5k5cagx_preview_preset_800x600[]={
    // Select and apply preview configuration

    0x0028 ,0x7000,

    0x002A, 0x026C,
    0x0F12, 800, //#REG_0TC_PCFG_usWidth
    0x0F12, 600, //#REG_0TC_PCFG_usHeight

    0x002A ,0x168E,
    0x0f12 ,0x0800,
    0x002A ,0x023C,		//#REG_TC_GP_ActivePrevConfig
    0x0F12 ,0x0000,
    0x002A ,0x0240,		//#REG_TC_GP_PrevOpenAfterChange
    0x0F12 ,0x0001,
    0x002A ,0x0230,		//#REG_TC_GP_NewConfigSync
    0x0F12 ,0x0001,
    0x002A ,0x023E,		//#REG_TC_GP_PrevConfigChanged
    0x0F12 ,0x0001,

    0x002A, 0x0252,
    0x0F12, 0x0006, //  //AF

};

#define S5K5CAGX_INIT_REGS	\
    (sizeof(s5k5cagx_init_reg_short) / sizeof(s5k5cagx_init_reg_short[0]))

#define S5K5CAGX_INIT_REGS2	\
    (sizeof(s5k5cagx_init_reg_short2) / sizeof(s5k5cagx_init_reg_short2[0]))

#define S5K5CAGX_INIT_REGS3	\
    (sizeof(s5k5cagx_init_reg_short3) / sizeof(s5k5cagx_init_reg_short3[0]))

#define S5K5CAGX_INIT_REGS4	\
    (sizeof(s5k5cagx_init_reg_short4) / sizeof(s5k5cagx_init_reg_short4[0]))

#define S5K5CAGX_INIT_REGS5	\
    (sizeof(s5k5cagx_init_reg_short5) / sizeof(s5k5cagx_init_reg_short5[0]))


#define S5K5CAGX_CAPTURE_PRESET_0	\
    (sizeof(s5k5cagx_capture_preset_0) / sizeof(s5k5cagx_capture_preset_0[0]))

#define S5K5CAGX_PREVIEW_PRESET_0	\
    (sizeof(s5k5cagx_preview_preset_0) / sizeof(s5k5cagx_preview_preset_0[0]))

#define S5K5CAGX_PREVIEW_PRESET_800x600	\
    (sizeof(s5k5cagx_preview_preset_800x600) / sizeof(s5k5cagx_preview_preset_800x600[0]))

#define S5K5CAGX_MOVIE_PRESET_0	\
    (sizeof(s5k5cagx_movie_preset_0) / sizeof(s5k5cagx_movie_preset_0[0]))


/*
 * EV bias
 */
static const struct s5k5cagx_reg s5k5cagx_ev_m6[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_m5[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_m4[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_m3[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_m2[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_m1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_default[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_p1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_p2[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_p3[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_p4[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_p5[] = {
};

static const struct s5k5cagx_reg s5k5cagx_ev_p6[] = {
};

#ifdef S5K5CAGX_COMPLETE
/* Order of this array should be following the querymenu data */
static const unsigned char *s5k5cagx_regs_ev_bias[] = {
    (unsigned char *)s5k5cagx_ev_m6, (unsigned char *)s5k5cagx_ev_m5,
    (unsigned char *)s5k5cagx_ev_m4, (unsigned char *)s5k5cagx_ev_m3,
    (unsigned char *)s5k5cagx_ev_m2, (unsigned char *)s5k5cagx_ev_m1,
    (unsigned char *)s5k5cagx_ev_default, (unsigned char *)s5k5cagx_ev_p1,
    (unsigned char *)s5k5cagx_ev_p2, (unsigned char *)s5k5cagx_ev_p3,
    (unsigned char *)s5k5cagx_ev_p4, (unsigned char *)s5k5cagx_ev_p5,
    (unsigned char *)s5k5cagx_ev_p6,
};
/* auto focus */

unsigned short s5k5cagx_focus_auto[] = {
    0x0028, 0x7000,
    0x002A, 0x0254, //REG_TC_AF_AfCmdParam
    0x0F12, 0x0000, //write lens position from 0000 to 00FF.
    //0000 means infinity and 00FF means macro
    //	P133	  //Delay 133ms
    0xffff, 133,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0004, //0004 - Manual AF

    //P200	  //Delay 200ms
    0xffff, 200,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0006, //0005 - Single AF

};

unsigned short s5k5cagx_zoom_fixed[] = {
    0x0028, 0x7000,
    0x002A, 0x0254, //REG_TC_AF_AfCmdParam
    0x0F12, 0x0000, //write lens position from 0000 to 00FF.
    //0000 means infinity and 00FF means macro
    //	P133	  //Delay 133ms
    0xffff, 133,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0004, //0004 - Manual AF

    //P200	  //Delay 200ms
    0xffff, 200,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0005, //0005 - Single AF
};

unsigned short s5k5cagx_zoom_infinity[] = {
    0x0028, 0x7000,
    0x002A, 0x0254, //REG_TC_AF_AfCmdParam
    0x0F12, 0x0000, //write lens position from 0000 to 00FF.
    //0000 means infinity and 00FF means macro
    //	P133	  //Delay 133ms
    0xffff, 133,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0004, //0004 - Manual AF

    //P200	  //Delay 200ms
    0xffff, 200,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0005, //0005 - Single AF
};
unsigned short s5k5cagx_zoom_macro[] = {
    0x0028, 0x7000,
    0x002A, 0x0254, //REG_TC_AF_AfCmdParam
    0x0F12, 0x0000, //write lens position from 0000 to 00FF.
    //0000 means infinity and 00FF means macro
    //	P133	  //Delay 133ms
    0xffff, 133,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0004, //0004 - Manual AF

    //P200	  //Delay 200ms
    0xffff, 200,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0005, //0005 - Single AF
};

unsigned short s5k5cagx_focus_init[] = {
    0x0028, 0x7000,
    0x002A, 0x0254, //REG_TC_AF_AfCmdParam
    0x0F12, 0x0000, //write lens position from 0000 to 00FF.
    //0000 means infinity and 00FF means macro
    //	P133	  //Delay 133ms
    0xffff, 133,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0004, //0004 - Manual AF

    //P200	  //Delay 200ms
    0xffff, 200,
    0x002A, 0x0252, //REG_TC_AF_AfCmd
    0x0F12, 0x0003, //0005 - init
};

#define S5K5CAGX_FOCUS	(sizeof(s5k5cagx_focus_auto) / sizeof(s5k5cagx_focus_auto[0]))

static const unsigned short *s5k5cagx_regs_focus[] = {
    (unsigned short *)s5k5cagx_focus_auto,
    (unsigned short *)s5k5cagx_zoom_fixed,
    (unsigned short *)s5k5cagx_zoom_infinity,
    (unsigned short *)s5k5cagx_zoom_macro,
    (unsigned short *)s5k5cagx_focus_init,
};


/*
 * Auto White Balance configure
 */
static const struct s5k5cagx_reg s5k5cagx_awb_off[] = {
};

static const struct s5k5cagx_reg s5k5cagx_awb_on[] = {
};

static const unsigned char *s5k5cagx_regs_awb_enable[] = {
    (unsigned char *)s5k5cagx_awb_off,
    (unsigned char *)s5k5cagx_awb_on,
};

/*
 * Manual White Balance (presets)
 */
static unsigned short s5k5cagx_wb_tungsten[] = {
    //================================================================================================
    // SET MWB_Incandescent
    //================================================================================================
    // Indoor boundary
    0x0028 ,0x7000,
    0x002A ,0x246E,
    0x0F12 ,0x0000, //#Mon_AAIO_bAWB_Off
    0x002A ,0x22CA,
    0x0F12 ,0x0433, //#Mon_AWB_GainsOut_R
    0x0F12 ,0x0400, //#Mon_AWB_GainsOut_G
    0x0F12 ,0x0800, //#Mon_AWB_GainsOut_B
    0x002A ,0x0C48,
    0x0F12 ,0x032E, //#awbb_IndoorGrZones_m_BGrid[0]
    0x0F12 ,0x035A, //#awbb_IndoorGrZones_m_BGrid[1]
    0x0F12 ,0x0311, //#awbb_IndoorGrZones_m_BGrid[2]
    0x0F12 ,0x0356, //#awbb_IndoorGrZones_m_BGrid[3]
    0x0F12 ,0x030E, //#awbb_IndoorGrZones_m_BGrid[4]
    0x0F12 ,0x033B, //#awbb_IndoorGrZones_m_BGrid[5]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[6]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[7]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[8]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[9]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[10]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[11]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[12]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[13]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[14]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[15]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[16]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[17]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[18]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[19]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[20]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[21]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[22]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[23]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[24]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[25]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[26]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[27]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[28]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[29]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[30]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[31]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[32]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[33]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[34]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[35]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[36]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[37]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[38]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[39]
    0x0F12 ,0x0005, //#awbb_IndoorGrZones_m_GridStep
    0x002A ,0x0CA0,
    0x0F12 ,0x017F, //#awbb_IndoorGrZones_m_Boffs
    0xffff ,200,
    0x002A ,0x246E,
    0x0F12 ,0x0001, //#Mon_AAIO_bAWB_On
};

static unsigned short s5k5cagx_wb_fluorescent[] = {
    //================================================================================================
    // SET MWB_Fluorescent
    //================================================================================================
    // Indoor boundary
    0x0028 ,0x7000,
    0x002A ,0x246E,
    0x0F12 ,0x0000, //#Mon_AAIO_bAWB_Off
    0x002A ,0x22CA,
    0x0F12 ,0x047A, //#Mon_AWB_GainsOut_R
    0x0F12 ,0x0400, //#Mon_AWB_GainsOut_G
    0x0F12 ,0x06EB, //#Mon_AWB_GainsOut_B
    0x002A ,0x0C48,
    0x0F12 ,0x02F2, //#awbb_IndoorGrZones_m_BGrid[0]
    0x0F12 ,0x031E, //#awbb_IndoorGrZones_m_BGrid[1]
    0x0F12 ,0x02D5, //#awbb_IndoorGrZones_m_BGrid[2]
    0x0F12 ,0x031A, //#awbb_IndoorGrZones_m_BGrid[3]
    0x0F12 ,0x02D2, //#awbb_IndoorGrZones_m_BGrid[4]
    0x0F12 ,0x02FF, //#awbb_IndoorGrZones_m_BGrid[5]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[6]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[7]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[8]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[9]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[10]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[11]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[12]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[13]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[14]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[15]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[16]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[17]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[18]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[19]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[20]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[21]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[22]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[23]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[24]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[25]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[26]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[27]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[28]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[29]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[30]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[31]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[32]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[33]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[34]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[35]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[36]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[37]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[38]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[39]
    0x0F12 ,0x0005, //#awbb_IndoorGrZones_m_GridStep
    0x002A ,0x0CA0,
    0x0F12 ,0x01B2, //#awbb_IndoorGrZones_m_Boffs
    0xffff ,200,
    0x002A ,0x246E,
    0x0F12 ,0x0001, //#Mon_AAIO_bAWB_On
};

static unsigned short s5k5cagx_wb_sunny[] = {
    //================================================================================================

    // SET MWB_DayLight

    //================================================================================================

    // Indoor boundary

    0x0028 ,0x7000,
    0x002A ,0x246E,
    0x0F12 ,0x0000, //#Mon_AAIO_bAWB_Off
    0x002A ,0x22CA,
    0x0F12 ,0x0547, //#Mon_AWB_GainsOut_R
    0x0F12 ,0x0400, //#Mon_AWB_GainsOut_G
    0x0F12 ,0x0585, //#Mon_AWB_GainsOut_B
    0x002A ,0x0C48,
    0x0F12 ,0x026C, //#awbb_IndoorGrZones_m_BGrid[0]
    0x0F12 ,0x0298, //#awbb_IndoorGrZones_m_BGrid[1]
    0x0F12 ,0x024F, //#awbb_IndoorGrZones_m_BGrid[2]
    0x0F12 ,0x0295, //#awbb_IndoorGrZones_m_BGrid[3]
    0x0F12 ,0x024C, //#awbb_IndoorGrZones_m_BGrid[4]
    0x0F12 ,0x0279, //#awbb_IndoorGrZones_m_BGrid[5]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[6]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[7]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[8]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[9]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[10]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[11]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[12]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[13]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[14]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[15]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[16]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[17]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[18]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[19]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[20]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[21]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[22]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[23]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[24]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[25]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[26]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[27]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[28]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[29]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[30]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[31]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[32]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[33]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[34]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[35]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[36]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[37]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[38]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[39]
    0x0F12 ,0x0005, //#awbb_IndoorGrZones_m_GridStep
    0x002A ,0x0CA0,
    0x0F12 ,0x0234, //#awbb_IndoorGrZones_m_Boffs
    0xffff,	200, //delay
    0x002A ,0x246E,
    0x0F12 ,0x0001, //#Mon_AAIO_bAWB_On
};


static unsigned short s5k5cagx_wb_cloudy[] = {
    //================================================================================================

    // SET MWB_Cloudy

    //================================================================================================

    // Indoor boundary
    0x0028 ,0x7000,
    0x002A ,0x246E,
    0x0F12 ,0x0000, //#Mon_AAIO_bAWB_Off
    0x002A ,0x22CA,
    0x0F12 ,0x057A, //#Mon_AWB_GainsOut_R

    0x0F12 ,0x0400, //#Mon_AWB_GainsOut_G
    0x0F12 ,0x05A3, //#Mon_AWB_GainsOut_B
    0x002A ,0x0C48,
    0x0F12 ,0x022C, //#awbb_IndoorGrZones_m_BGrid[0]
    0x0F12 ,0x0258, //#awbb_IndoorGrZones_m_BGrid[1]
    0x0F12 ,0x020F, //#awbb_IndoorGrZones_m_BGrid[2]
    0x0F12 ,0x0255, //#awbb_IndoorGrZones_m_BGrid[3]
    0x0F12 ,0x020C, //#awbb_IndoorGrZones_m_BGrid[4]
    0x0F12 ,0x0239, //#awbb_IndoorGrZones_m_BGrid[5]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[6]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[7]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[8]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[9]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[10]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[11]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[12]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[13]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[14]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[15]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[16]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[17]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[18]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[19]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[20]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[21]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[22]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[23]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[24]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[25]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[26]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[27]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[28]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[29]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[30]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[31]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[32]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[33]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[34]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[35]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[36]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[37]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[38]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[39]
    0x0F12 ,0x0005, //#awbb_IndoorGrZones_m_GridStep
    0x002A ,0x0CA0,
    0x0F12 ,0x0273, //#awbb_IndoorGrZones_m_Boffs
    0xffff, 200,
    0x002A ,0x246E,
    0x0F12 ,0x0001, //#Mon_AAIO_bAWB_On
};

static unsigned short s5k5cagx_wb_auto[] = {
    //================================================================================================
    // SET MWB_Auto
    //================================================================================================
    // Indoor boundary
    0x0028 ,0x7000,
    0x002A ,0x246E,
    0x0F12 ,0x0000, //#Mon_AAIO_bAWB_Off
    0x002A ,0x22CA,
    0x0F12 ,0x053C, //#Mon_AWB_GainsOut_R
    0x0F12 ,0x0400, //#Mon_AWB_GainsOut_G
    0x0F12 ,0x055C, //#Mon_AWB_GainsOut_B
    0x002A ,0x0C48,
    0x0F12 ,0x0384, //#awbb_IndoorGrZones_m_BGrid[0]
    0x0F12 ,0x03A3, //#awbb_IndoorGrZones_m_BGrid[1]
    0x0F12 ,0x034C, //#awbb_IndoorGrZones_m_BGrid[2]
    0x0F12 ,0x037B, //#awbb_IndoorGrZones_m_BGrid[3]
    0x0F12 ,0x0311, //#awbb_IndoorGrZones_m_BGrid[4]
    0x0F12 ,0x0355, //#awbb_IndoorGrZones_m_BGrid[5]
    0x0F12 ,0x02E7, //#awbb_IndoorGrZones_m_BGrid[6]
    0x0F12 ,0x0334, //#awbb_IndoorGrZones_m_BGrid[7]
    0x0F12 ,0x02BD, //#awbb_IndoorGrZones_m_BGrid[8]
    0x0F12 ,0x0312, //#awbb_IndoorGrZones_m_BGrid[9]
    0x0F12 ,0x029B, //#awbb_IndoorGrZones_m_BGrid[10]
    0x0F12 ,0x02F6, //#awbb_IndoorGrZones_m_BGrid[11]
    0x0F12 ,0x0278, //#awbb_IndoorGrZones_m_BGrid[12]
    0x0F12 ,0x02DC, //#awbb_IndoorGrZones_m_BGrid[13]
    0x0F12 ,0x025E, //#awbb_IndoorGrZones_m_BGrid[14]
    0x0F12 ,0x02BB, //#awbb_IndoorGrZones_m_BGrid[15]
    0x0F12 ,0x0246, //#awbb_IndoorGrZones_m_BGrid[16]
    0x0F12 ,0x02AA, //#awbb_IndoorGrZones_m_BGrid[17]
    0x0F12 ,0x022C, //#awbb_IndoorGrZones_m_BGrid[18]
    0x0F12 ,0x029B, //#awbb_IndoorGrZones_m_BGrid[19]
    0x0F12 ,0x0214, //#awbb_IndoorGrZones_m_BGrid[20]
    0x0F12 ,0x028E, //#awbb_IndoorGrZones_m_BGrid[21]
    0x0F12 ,0x01FF, //#awbb_IndoorGrZones_m_BGrid[22]
    0x0F12 ,0x0286, //#awbb_IndoorGrZones_m_BGrid[23]
    0x0F12 ,0x01E3, //#awbb_IndoorGrZones_m_BGrid[24]
    0x0F12 ,0x0272, //#awbb_IndoorGrZones_m_BGrid[25]
    0x0F12 ,0x01D8, //#awbb_IndoorGrZones_m_BGrid[26]
    0x0F12 ,0x025C, //#awbb_IndoorGrZones_m_BGrid[27]
    0x0F12 ,0x01D8, //#awbb_IndoorGrZones_m_BGrid[28]
    0x0F12 ,0x0248, //#awbb_IndoorGrZones_m_BGrid[29]
    0x0F12 ,0x01F2, //#awbb_IndoorGrZones_m_BGrid[30]
    0x0F12 ,0x0230, //#awbb_IndoorGrZones_m_BGrid[31]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[32]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[33]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[34]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[35]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[36]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[37]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[38]
    0x0F12 ,0x0000, //#awbb_IndoorGrZones_m_BGrid[39]
    0x0F12 ,0x0005, //#awbb_IndoorGrZones_m_GridStep
    0x002A ,0x0CA0,
    0x0F12 ,0x0149, //#awbb_IndoorGrZones_m_Boffs
    0xffff,200,
    0x002A ,0x246E,
    0x0F12 ,0x0001, //#Mon_AAIO_bAWB_On
};

/* Order of this array should be following the querymenu data */
static const unsigned short *s5k5cagx_regs_wb_preset[] = {
    (unsigned short *)s5k5cagx_wb_tungsten,
    (unsigned short *)s5k5cagx_wb_fluorescent,
    (unsigned short *)s5k5cagx_wb_sunny,
    (unsigned short *)s5k5cagx_wb_cloudy,
    (unsigned short *)s5k5cagx_wb_auto,
};
#define S5K5CAGX_WB_PRESET	(sizeof(s5k5cagx_wb_auto) / sizeof(s5k5cagx_wb_auto[0]))

/*
 * Color Effect (COLORFX)
 */
static unsigned short s5k5cagx_color_none[] = {
    0x0028 ,0x7000,	// set page
    0x002A ,0x021E,	// set address
    0x0F12 ,0x0000,	// No effect
};

static unsigned short s5k5cagx_color_sepia[] = {
    0x0028 ,0x7000,	// set page
    0x002A ,0x021E,	// set address
    0x0F12 ,0x0004,	// Sepia
};

static unsigned short s5k5cagx_color_aqua[] = {
    0x0028 ,0x7000,	// set page
    0x002A ,0x021E,	// set address
    0x0F12 ,0x0005,	// Aqua
};

static unsigned short s5k5cagx_color_monochrome[] = {
    0x0028 ,0x7000,	// set page
    0x002A ,0x021E,	// set address
    0x0F12 ,0x0001,	// Mono (Black & White)
};

static unsigned short s5k5cagx_color_negative[] = {
    0x0028 ,0x7000,	// set page
    0x002A ,0x021E,	// set address
    0x0F12 ,0x0003,	// Color + Negative
};

static unsigned short s5k5cagx_color_sketch[] = {
    0x0028 ,0x7000,	// set page
    0x002A ,0x021E,	// set address
    0x0F12 ,0x0006,	// Sketch
};

/* Order of this array should be following the querymenu data */
static const unsigned short *s5k5cagx_regs_color_effect[] = {
    (unsigned short *)s5k5cagx_color_none,
    (unsigned short *)s5k5cagx_color_sepia,
    (unsigned short *)s5k5cagx_color_aqua,
    (unsigned short *)s5k5cagx_color_monochrome,
    (unsigned short *)s5k5cagx_color_negative,
    (unsigned short *)s5k5cagx_color_sketch,
};
#define S5K5CAGX_COLOR_FX	(sizeof(s5k5cagx_color_none) / sizeof(s5k5cagx_color_none[0]))


/*
 * Contrast bias
 */
static const struct s5k5cagx_reg s5k5cagx_contrast_m2[] = {
};

static const struct s5k5cagx_reg s5k5cagx_contrast_m1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_contrast_default[] = {
};

static const struct s5k5cagx_reg s5k5cagx_contrast_p1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_contrast_p2[] = {
};

static const unsigned char *s5k5cagx_regs_contrast_bias[] = {
    (unsigned char *)s5k5cagx_contrast_m2,
    (unsigned char *)s5k5cagx_contrast_m1,
    (unsigned char *)s5k5cagx_contrast_default,
    (unsigned char *)s5k5cagx_contrast_p1,
    (unsigned char *)s5k5cagx_contrast_p2,
};

/*
 * Saturation bias
 */
static const struct s5k5cagx_reg s5k5cagx_saturation_m2[] = {
};

static const struct s5k5cagx_reg s5k5cagx_saturation_m1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_saturation_default[] = {
};

static const struct s5k5cagx_reg s5k5cagx_saturation_p1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_saturation_p2[] = {
};

static const unsigned char *s5k5cagx_regs_saturation_bias[] = {
    (unsigned char *)s5k5cagx_saturation_m2,
    (unsigned char *)s5k5cagx_saturation_m1,
    (unsigned char *)s5k5cagx_saturation_default,
    (unsigned char *)s5k5cagx_saturation_p1,
    (unsigned char *)s5k5cagx_saturation_p2,
};

/*
 * Sharpness bias
 */
static const struct s5k5cagx_reg s5k5cagx_sharpness_m2[] = {
};

static const struct s5k5cagx_reg s5k5cagx_sharpness_m1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_sharpness_default[] = {
};

static const struct s5k5cagx_reg s5k5cagx_sharpness_p1[] = {
};

static const struct s5k5cagx_reg s5k5cagx_sharpness_p2[] = {
};

static const unsigned char *s5k5cagx_regs_sharpness_bias[] = {
    (unsigned char *)s5k5cagx_sharpness_m2,
    (unsigned char *)s5k5cagx_sharpness_m1,
    (unsigned char *)s5k5cagx_sharpness_default,
    (unsigned char *)s5k5cagx_sharpness_p1,
    (unsigned char *)s5k5cagx_sharpness_p2,
};
#endif /* S5K5CAGX_COMPLETE */



//////
#define __S54EAGX__
#ifdef __S54EAGX__
unsigned char s5k4eagx_init_reg41[][4] = {
    /*
       0010 0001 //// Reset
       1030 0000
       0014 0001
       p100
       */
    {0x00, 0x2c, 0xD0, 0x00}, //offset setting
    {0x00, 0x2A, 0x00, 0x12},	{0x0F, 0x12, 0x00,0x01},	// Reset
    {0x00, 0x2A, 0x10, 0x30},	{0x0F, 0x12, 0x00,0x00},
    {0x00, 0x2A, 0x00, 0x14},	{0x0F, 0x12, 0x00,0x01},
    {0x00, 0xff, 100,  0},	// p100
};

unsigned short s5k4eagx_init_reg42[][2] = {

    {0x0028, 0x7000}, //// SET T&P (AF, StandBy leakge)
    {0x002A, 0x31BC},
    {0x0F12, 0xB538}, // 700031BC
    {0x0F12, 0x2218}, // 700031BE
    {0x0F12, 0xA00D}, // 700031C0
    {0x0F12, 0x6800}, // 700031C2
    {0x0F12, 0x9000}, // 700031C4
    {0x0F12, 0x466B}, // 700031C6
    {0x0F12, 0x8819}, // 700031C8
    {0x0F12, 0x480C}, // 700031CA
    {0x0F12, 0x8441}, // 700031CC
    {0x0F12, 0x8859}, // 700031CE
    {0x0F12, 0x8481}, // 700031D0
    {0x0F12, 0x2400}, // 700031D2
    {0x0F12, 0x84C4}, // 700031D4
    {0x0F12, 0x2103}, // 700031D6
    {0x0F12, 0x8501}, // 700031D8
    {0x0F12, 0x4909}, // 700031DA
    {0x0F12, 0x4809}, // 700031DC
    {0x0F12, 0xF000}, // 700031DE
    {0x0F12, 0xF8A1}, // 700031E0
    {0x0F12, 0x4909}, // 700031E2
    {0x0F12, 0x4809}, // 700031E4
    {0x0F12, 0x0022}, // 700031E6
    {0x0F12, 0xF000}, // 700031E8
    {0x0F12, 0xF89C}, // 700031EA
    {0x0F12, 0x4908}, // 700031EC
    {0x0F12, 0x4809}, // 700031EE
    {0x0F12, 0x6201}, // 700031F0
    {0x0F12, 0xBC38}, // 700031F2
    {0x0F12, 0xBC08}, // 700031F4
    {0x0F12, 0x4718}, // 700031F6
    {0x0F12, 0x4EA3}, // 700031F8
    {0x0F12, 0x00D6}, // 700031FA
    {0x0F12, 0x1C18}, // 700031FC
    {0x0F12, 0x7000}, // 700031FE
    {0x0F12, 0x3219}, // 70003200
    {0x0F12, 0x7000}, // 70003202
    {0x0F12, 0x0D2B}, // 70003204
    {0x0F12, 0x0001}, // 70003206
    {0x0F12, 0x3261}, // 70003208
    {0x0F12, 0x7000}, // 7000320A
    {0x0F12, 0xB375}, // 7000320C
    {0x0F12, 0x0000}, // 7000320E
    {0x0F12, 0x32E9}, // 70003210
    {0x0F12, 0x7000}, // 70003212
    {0x0F12, 0x00C0}, // 70003214
    {0x0F12, 0x7000}, // 70003216
    {0x0F12, 0xB5F8}, // 70003218
    {0x0F12, 0x0004}, // 7000321A
    {0x0F12, 0x2601}, // 7000321C
    {0x0F12, 0x0035}, // 7000321E
    {0x0F12, 0x0020}, // 70003220
    {0x0F12, 0x3810}, // 70003222
    {0x0F12, 0x4086}, // 70003224
    {0x0F12, 0x40A5}, // 70003226
    {0x0F12, 0x4F39}, // 70003228
    {0x0F12, 0x2C10}, // 7000322A
    {0x0F12, 0xDA03}, // 7000322C
    {0x0F12, 0x8838}, // 7000322E
    {0x0F12, 0x43A8}, // 70003230
    {0x0F12, 0x8038}, // 70003232
    {0x0F12, 0xE002}, // 70003234
    {0x0F12, 0x8878}, // 70003236
    {0x0F12, 0x43B0}, // 70003238
    {0x0F12, 0x8078}, // 7000323A
    {0x0F12, 0xF000}, // 7000323C
    {0x0F12, 0xF87A}, // 7000323E
    {0x0F12, 0x2C10}, // 70003240
    {0x0F12, 0xDA03}, // 70003242
    {0x0F12, 0x8838}, // 70003244
    {0x0F12, 0x4328}, // 70003246
    {0x0F12, 0x8038}, // 70003248
    {0x0F12, 0xE002}, // 7000324A
    {0x0F12, 0x8878}, // 7000324C
    {0x0F12, 0x4330}, // 7000324E
    {0x0F12, 0x8078}, // 70003250
    {0x0F12, 0x4930}, // 70003252
    {0x0F12, 0x2000}, // 70003254
    {0x0F12, 0x8188}, // 70003256
    {0x0F12, 0x80C8}, // 70003258
    {0x0F12, 0xBCF8}, // 7000325A
    {0x0F12, 0xBC08}, // 7000325C
    {0x0F12, 0x4718}, // 7000325E
    {0x0F12, 0xB5F8}, // 70003260
    {0x0F12, 0x2407}, // 70003262
    {0x0F12, 0x2C06}, // 70003264
    {0x0F12, 0xD036}, // 70003266
    {0x0F12, 0x2C07}, // 70003268
    {0x0F12, 0xD034}, // 7000326A
    {0x0F12, 0x4A2A}, // 7000326C
    {0x0F12, 0x8BD1}, // 7000326E
    {0x0F12, 0x2900}, // 70003270
    {0x0F12, 0xD02B}, // 70003272
    {0x0F12, 0x00A0}, // 70003274
    {0x0F12, 0x1885}, // 70003276
    {0x0F12, 0x4828}, // 70003278
    {0x0F12, 0x6DEE}, // 7000327A
    {0x0F12, 0x8A80}, // 7000327C
    {0x0F12, 0x4286}, // 7000327E
    {0x0F12, 0xD924}, // 70003280
    {0x0F12, 0x002F}, // 70003282
    {0x0F12, 0x3780}, // 70003284
    {0x0F12, 0x69F8}, // 70003286
    {0x0F12, 0xF000}, // 70003288
    {0x0F12, 0xF85C}, // 7000328A
    {0x0F12, 0x1C71}, // 7000328C
    {0x0F12, 0x0280}, // 7000328E
    {0x0F12, 0xF000}, // 70003290
    {0x0F12, 0xF858}, // 70003292
    {0x0F12, 0x0006}, // 70003294
    {0x0F12, 0x4A20}, // 70003296
    {0x0F12, 0x0060}, // 70003298
    {0x0F12, 0x1880}, // 7000329A
    {0x0F12, 0x8D80}, // 7000329C
    {0x0F12, 0x0A01}, // 7000329E
    {0x0F12, 0x0600}, // 700032A0
    {0x0F12, 0x0E00}, // 700032A2
    {0x0F12, 0x1A08}, // 700032A4
    {0x0F12, 0x0400}, // 700032A6
    {0x0F12, 0x1400}, // 700032A8
    {0x0F12, 0xF000}, // 700032AA
    {0x0F12, 0xF851}, // 700032AC
    {0x0F12, 0x0001}, // 700032AE
    {0x0F12, 0x6DEA}, // 700032B0
    {0x0F12, 0x6FE8}, // 700032B2
    {0x0F12, 0x4351}, // 700032B4
    {0x0F12, 0x1A10}, // 700032B6
    {0x0F12, 0x0300}, // 700032B8
    {0x0F12, 0x1C49}, // 700032BA
    {0x0F12, 0xF000}, // 700032BC
    {0x0F12, 0xF842}, // 700032BE
    {0x0F12, 0x0401}, // 700032C0
    {0x0F12, 0x0430}, // 700032C2
    {0x0F12, 0x0C00}, // 700032C4
    {0x0F12, 0x4308}, // 700032C6
    {0x0F12, 0x61F8}, // 700032C8
    {0x0F12, 0xE004}, // 700032CA
    {0x0F12, 0x00A1}, // 700032CC
    {0x0F12, 0x4814}, // 700032CE
    {0x0F12, 0x1889}, // 700032D0
    {0x0F12, 0x3180}, // 700032D2
    {0x0F12, 0x61C8}, // 700032D4
    {0x0F12, 0x1E64}, // 700032D6
    {0x0F12, 0xD2C4}, // 700032D8
    {0x0F12, 0x2006}, // 700032DA
    {0x0F12, 0xF000}, // 700032DC
    {0x0F12, 0xF840}, // 700032DE
    {0x0F12, 0x2007}, // 700032E0
    {0x0F12, 0xF000}, // 700032E2
    {0x0F12, 0xF83D}, // 700032E4
    {0x0F12, 0xE7B8}, // 700032E6
    {0x0F12, 0xB510}, // 700032E8
    {0x0F12, 0xF000}, // 700032EA
    {0x0F12, 0xF841}, // 700032EC
    {0x0F12, 0x2800}, // 700032EE
    {0x0F12, 0xD00C}, // 700032F0
    {0x0F12, 0x4809}, // 700032F2
    {0x0F12, 0x8B81}, // 700032F4
    {0x0F12, 0x0089}, // 700032F6
    {0x0F12, 0x1808}, // 700032F8
    {0x0F12, 0x6DC1}, // 700032FA
    {0x0F12, 0x4807}, // 700032FC
    {0x0F12, 0x8A80}, // 700032FE
    {0x0F12, 0x4281}, // 70003300
    {0x0F12, 0xD903}, // 70003302
    {0x0F12, 0x2001}, // 70003304
    {0x0F12, 0xBC10}, // 70003306
    {0x0F12, 0xBC08}, // 70003308
    {0x0F12, 0x4718}, // 7000330A
    {0x0F12, 0x2000}, // 7000330C
    {0x0F12, 0xE7FA}, // 7000330E
    {0x0F12, 0x1100}, // 70003310
    {0x0F12, 0xD000}, // 70003312
    {0x0F12, 0x0060}, // 70003314
    {0x0F12, 0xD000}, // 70003316
    {0x0F12, 0x2B14}, // 70003318
    {0x0F12, 0x7000}, // 7000331A
    {0x0F12, 0x158C}, // 7000331C
    {0x0F12, 0x7000}, // 7000331E
    {0x0F12, 0xFFFF}, // 70003320
    {0x0F12, 0x0000}, // 70003322
    {0x0F12, 0x4778}, // 70003324
    {0x0F12, 0x46C0}, // 70003326
    {0x0F12, 0xC000}, // 70003328
    {0x0F12, 0xE59F}, // 7000332A
    {0x0F12, 0xFF1C}, // 7000332C
    {0x0F12, 0xE12F}, // 7000332E
    {0x0F12, 0x1283}, // 70003330
    {0x0F12, 0x0001}, // 70003332
    {0x0F12, 0x4778}, // 70003334
    {0x0F12, 0x46C0}, // 70003336
    {0x0F12, 0xC000}, // 70003338
    {0x0F12, 0xE59F}, // 7000333A
    {0x0F12, 0xFF1C}, // 7000333C
    {0x0F12, 0xE12F}, // 7000333E
    {0x0F12, 0x0D2B}, // 70003340
    {0x0F12, 0x0001}, // 70003342
    {0x0F12, 0x4778}, // 70003344
    {0x0F12, 0x46C0}, // 70003346
    {0x0F12, 0xF004}, // 70003348
    {0x0F12, 0xE51F}, // 7000334A
    {0x0F12, 0x24B8}, // 7000334C
    {0x0F12, 0x0001}, // 7000334E
    {0x0F12, 0x4778}, // 70003350
    {0x0F12, 0x46C0}, // 70003352
    {0x0F12, 0xC000}, // 70003354
    {0x0F12, 0xE59F}, // 70003356
    {0x0F12, 0xFF1C}, // 70003358
    {0x0F12, 0xE12F}, // 7000335A
    {0x0F12, 0x2E39}, // 7000335C
    {0x0F12, 0x0000}, // 7000335E
    {0x0F12, 0x4778}, // 70003360
    {0x0F12, 0x46C0}, // 70003362
    {0x0F12, 0xC000}, // 70003364
    {0x0F12, 0xE59F}, // 70003366
    {0x0F12, 0xFF1C}, // 70003368
    {0x0F12, 0xE12F}, // 7000336A
    {0x0F12, 0xB34D}, // 7000336C
    {0x0F12, 0x0000}, // 7000336E
    {0x0F12, 0x4778}, // 70003370
    {0x0F12, 0x46C0}, // 70003372
    {0x0F12, 0xC000}, // 70003374
    {0x0F12, 0xE59F}, // 70003376
    {0x0F12, 0xFF1C}, // 70003378
    {0x0F12, 0xE12F}, // 7000337A
    {0x0F12, 0xB459}, // 7000337C // End of Trap and Patch (Last : 7000337Eh)// TNP_HW_STBY_PLL_WR
    {0x0F12, 0x0000}, // 7000337E // Total Size 452 (0x01C4)// TNP_AF_MINSTAT_FIX

    //{0x1000, 0x0001},

};


unsigned short s5k4eagx_init_reg43[][2] = {

    {0x0028, 0xD000}, //// SET CIS/APS/ANALOG
    {0x002A, 0xF404},
    {0x0F12, 0x002A}, // ADC SAT(450mV): revised by Ana 090130
    {0x002A, 0xF46E},
    {0x0F12, 0x0002}, // CDS TEST [0]SR/SS EN: revised by Ana 090126// [1]S1 H, [2]LDB H, [3]clp H// [4]S34 P X, [5]S24 N X
    {0x002A, 0xF45A},
    {0x0F12, 0x0002}, // LD LDB EN : revised by Ana 090126
    {0x002A, 0xF40E},
    {0x0F12, 0x0004}, // RMP REG 1.8V: revised by Ana 090126
    {0x002A, 0xF40C},
    {0x0F12, 0x0020}, // rmp_option(RMP_INIT_DAC): revised by Ana 090126
    {0x002A, 0xF420},
    {0x0F12, 0x0076}, // COMP(CDS) bias [7:4] comp2, [3:0] comp1: revised by Ana 090126
    {0x0F12, 0x0005}, // pix current bias
    {0x002A, 0xF426},
    {0x0F12, 0x00D4}, // CLP level
    {0x002A, 0xF460},
    {0x0F12, 0x0001}, // CLP on: revised by Ana 090126
    {0x002A, 0xE304},
    {0x0F12, 0x0081}, // ADC OFFSET 128: revised by Ana 090126
    {0x002A, 0xE308},
    {0x0F12, 0x0081}, // ADC DEFAULT 128: revised by Ana 090209
    {0x002A, 0xE506}, // SET ADLC
    {0x0F12, 0x0093}, // ADLC [7]FL,[6:5]FM,: revised by Ana 090126 // [4]F,[3:2]LM,[1]L,[0]CH
    {0x002A, 0xE402},
    {0x0F12, 0x040C}, // ADLC BPR EN[10], th 12: revised by Ana 090126
    {0x002A, 0xF42A},
    {0x0F12, 0x0080}, // ALL TX mode enable(ref_option[7]): revised by Ana 090130
    {0x002A, 0xF408},
    {0x0F12, 0x000E}, // aig_sig_mx: revised by Ana 090209
    {0x0F12, 0x0007}, // aig_rst_mx: revised by Ana 090209
    {0x002A, 0xF400},
    {0x0F12, 0x0007}, // aig_off_rst1: revised by Ana 090209
    {0x002A, 0xF43C}, // SET DOUBLER
    {0x0F12, 0x0001}, // aig_pd_inrush_ctrl: revised by Ana 090126
    {0x002A, 0xF440},
    {0x0F12, 0x0042}, // aig_rosc_tune_ncp[7:4], aig_rosc_tune_cp[3:0]: revised by Ana 090126
    {0x002A, 0xF444},
    {0x0F12, 0x0008}, // aig_reg_tune_pix
    {0x002A, 0xF448},
    {0x0F12, 0x0008}, // aig_reg_tune_ntg
    {0x002A, 0xF45C},
    {0x0F12, 0x0001}, // aig_dshut_en
    {0x002A, 0xF406}, // MULTIPLE SAMPLING
    {0x0F12, 0x0001}, // MS[2:0], 2 times pseudo-multiple sampling: revised by Ana 090130
    {0x002A, 0xF410},
    {0x0F12, 0x0000}, // MSOFF_EN=0 : no Multiple sampling if gain < 2 : revised by Ana 090609
    {0x002A, 0xF472}, // APS TIMING
    {0x0F12, 0x005C}, // aig_dstx_width(1us@46MHz)  : revised by Ana 090216
    {0x002A, 0xF470},
    {0x0F12, 0x0004},
    {0x002A, 0x1082},
    {0x0F12, 0x0155}, // D0_D4_cs10Set IO driving current
    {0x0F12, 0x0155}, // D9_D5_cs10Set IO driving current
    {0x0F12, 0x0555}, // GPIO_cd10 Set IO driving current
    {0x0F12, 0x0555}, // CLKs_cd10 Set IO driving current
    {0x0028, 0x7000},
    {0x002A, 0x182C},
    {0x0F12, 0x0007},
    {0x002A, 0x1834},
    {0x0F12, 0x0001},
    {0x002A, 0x183C},
    {0x0F12, 0x005C},
    {0x002A, 0x17B8}, // CDS TIMING
    {0x0F12, 0x34B0},
    {0x0F12, 0x7000}, // senHal_ContPtrs_senModesDataArr
    {0x002A, 0x34B0}, // High speed mode
    {0x0F12, 0x0003},
    {0x0F12, 0x05BA},
    {0x0F12, 0x0001},
    {0x0F12, 0x05BC},
    {0x0F12, 0x0001},
    {0x0F12, 0x05BC},
    {0x0F12, 0x0014},
    {0x0F12, 0x05BC},
    {0x0F12, 0x0014},
    {0x0F12, 0x05BC},
    {0x0F12, 0x01AE},
    {0x0F12, 0x0238},
    {0x0F12, 0x01AE},
    {0x0F12, 0x0238},
    {0x0F12, 0x0238},
    {0x0F12, 0x05BA},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0001},
    {0x0F12, 0x01AC},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x01AC},
    {0x0F12, 0x0240},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0001},
    {0x0F12, 0x008C},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0001},
    {0x0F12, 0x009E},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0001},
    {0x0F12, 0x01AC},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x00A6},
    {0x0F12, 0x05BA},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0292},
    {0x0F12, 0x05BA},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x00D8},
    {0x0F12, 0x01A8},
    {0x0F12, 0x02C4},
    {0x0F12, 0x05B4},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x01AA},
    {0x0F12, 0x01D6},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x01BB},
    {0x0F12, 0x01EF},
    {0x0F12, 0x01F9},
    {0x0F12, 0x0203},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x01CC},
    {0x0F12, 0x01EF},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0001},
    {0x0F12, 0x000D},
    {0x0F12, 0x0001},
    {0x0F12, 0x000D},
    {0x0F12, 0x01AE},
    {0x0F12, 0x01B6},
    {0x0F12, 0x05BA},
    {0x0F12, 0x05C6},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x00D4},
    {0x0F12, 0x01AC},
    {0x0F12, 0x02C0},
    {0x0F12, 0x05B8},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x01AE},
    {0x0F12, 0x05C9},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x002A, 0x17BC},
    {0x0F12, 0x358A},
    {0x0F12, 0x7000},
    {0x002A, 0x358A},
    {0x0F12, 0x0003},
    {0x0F12, 0x081E},
    {0x0F12, 0x0001},
    {0x0F12, 0x0414},
    {0x0F12, 0x040F},
    {0x0F12, 0x0820},
    {0x0F12, 0x0014},
    {0x0F12, 0x040F},
    {0x0F12, 0x0422},
    {0x0F12, 0x0820},
    {0x0F12, 0x0146},
    {0x0F12, 0x01D0},
    {0x0F12, 0x0554},
    {0x0F12, 0x05DE},
    {0x0F12, 0x01D0},
    {0x0F12, 0x0410},
    {0x0F12, 0x05DE},
    {0x0F12, 0x081E},
    {0x0F12, 0x0001},
    {0x0F12, 0x0144},
    {0x0F12, 0x0414},
    {0x0F12, 0x0552},
    {0x0F12, 0x0144},
    {0x0F12, 0x01D8},
    {0x0F12, 0x0552},
    {0x0F12, 0x05E6},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},
    {0x0F12, 0x0001},
    {0x0F12, 0x008C},
    {0x0F12, 0x040F},
    {0x0F12, 0x049A},
    {0x0F12, 0x0001},
    {0x0F12, 0x009E},
    {0x0F12, 0x040F},
    {0x0F12, 0x04AC},
    {0x0F12, 0x0001},
    {0x0F12, 0x0144},
    {0x0F12, 0x040F},
    {0x0F12, 0x0552},
    {0x0F12, 0x00A6},
    {0x0F12, 0x0410},
    {0x0F12, 0x04B4},
    {0x0F12, 0x081E},
    {0x0F12, 0x022A},
    {0x0F12, 0x0410},
    {0x0F12, 0x0638},
    {0x0F12, 0x081E},
    {0x0F12, 0x00D8},
    {0x0F12, 0x0140},
    {0x0F12, 0x0292},
    {0x0F12, 0x040A},
    {0x0F12, 0x04E6},
    {0x0F12, 0x054E},
    {0x0F12, 0x06A0},
    {0x0F12, 0x0818},
    {0x0F12, 0x0142},
    {0x0F12, 0x016E},
    {0x0F12, 0x040C},
    {0x0F12, 0x0438},
    {0x0F12, 0x0550},
    {0x0F12, 0x057C},
    {0x0F12, 0x014D},
    {0x0F12, 0x0182},
    {0x0F12, 0x018C},
    {0x0F12, 0x0196},
    {0x0F12, 0x0417},
    {0x0F12, 0x044C},
    {0x0F12, 0x0456},
    {0x0F12, 0x0460},
    {0x0F12, 0x055B},
    {0x0F12, 0x0590},
    {0x0F12, 0x059A},
    {0x0F12, 0x05A4},
    {0x0F12, 0x0158},
    {0x0F12, 0x0182},
    {0x0F12, 0x0422},
    {0x0F12, 0x044C},
    {0x0F12, 0x0566},
    {0x0F12, 0x0590},
    {0x0F12, 0x0001},
    {0x0F12, 0x000D},
    {0x0F12, 0x0001},
    {0x0F12, 0x000D},
    {0x0F12, 0x0146},
    {0x0F12, 0x014E},
    {0x0F12, 0x0410},
    {0x0F12, 0x0418},
    {0x0F12, 0x0554},
    {0x0F12, 0x055C},
    {0x0F12, 0x081E},
    {0x0F12, 0x082A},
    {0x0F12, 0x00D4},
    {0x0F12, 0x0144},
    {0x0F12, 0x028E},
    {0x0F12, 0x040E},
    {0x0F12, 0x04E2},
    {0x0F12, 0x0552},
    {0x0F12, 0x069C},
    {0x0F12, 0x081C},
    {0x0F12, 0x0146},
    {0x0F12, 0x0410},
    {0x0F12, 0x0554},
    {0x0F12, 0x0830},
    {0x0F12, 0x0000},
    {0x0F12, 0x0000},

};


unsigned short s5k4eagx_init_reg44[][2] = {
    {0x0028, 0x7000},
    {0x002A, 0x023C}, //// SET AF
    {0x0F12, 0x0000}, // #REG_TC_IPRM_LedGpio// No Led Gpio
    {0x0F12, 0x0003}, // #REG_TC_IPRM_CM_Init_AfModeType  // AFModeType - 0:NONE, 2:VCM_PWM, 3:VCM_I2C
    {0x0F12, 0x0000}, // #REG_TC_IPRM_CM_Init_PwmConfig1  // No PWM
    {0x0F12, 0x0000}, // #REG_TC_IPRM_CM_Init_PwmConfig2
    {0x0F12, 0x0031}, // #REG_TC_IPRM_CM_Init_GpioConfig1 // No GPIO Port,31 // Use GPIO3 for Enable Port
    {0x0F12, 0x0000}, // #REG_TC_IPRM_CM_Init_GpioConfig2
    {0x002A, 0x024C},
    {0x0F12, 0x200C}, // #REG_TC_IPRM_CM_Init_Mi2cBits // Use GPIO1 for SCL, GPIO2 for SDA
    {0x0F12, 0x0320}, // #REG_TC_IPRM_CM_Init_Mi2cRateKhz // MI2C Speed : 400KHz
    {0x002A, 0x02C4},
    {0x0F12, 0x0100}, // #REG_TC_AF_FstWinStartX
    {0x0F12, 0x00E3}, // #REG_TC_AF_FstWinStartY
    {0x0F12, 0x0200}, // #REG_TC_AF_FstWinSizeX
    {0x0F12, 0x0238}, // #REG_TC_AF_FstWinSizeY
    {0x0F12, 0x018C}, // #REG_TC_AF_ScndWinStartX
    {0x0F12, 0x0166}, // #REG_TC_AF_ScndWinStartY
    {0x0F12, 0x00E6}, // #REG_TC_AF_ScndWinSizeX
    {0x0F12, 0x0132}, // #REG_TC_AF_ScndWinSizeY
    {0x0F12, 0x0001}, // #REG_TC_AF_WinSizesUpdated
    {0x002A, 0x0732},
    {0x0F12, 0x00FF}, // #skl_af_StatOvlpExpFactor
    {0x002A, 0x15CA},
    {0x0F12, 0x0003}, // #af_scene_usSaturatedScene
    {0x002A, 0x1540},
    {0x0F12, 0x1000}, // FineSearch Disable // af_search_usSingleAfFlags
    {0x002A, 0x154A},
    {0x0F12, 0x0004}, // #af_search_usFinePeakCount
    {0x002A, 0x14D8},
    {0x0F12, 0x0302}, // #af_pos_usFineStepNumSize
    {0x002A, 0x1536},
    {0x0F12, 0x00C0}, // #af_search_usPeakThr
    {0x002A, 0x15C2}, // #AF High contrast Scene
    {0x0F12, 0x0200}, // #af_scene_usHighContrastThr
    {0x0F12, 0x0000}, // #af_scene_usHighContrastWin (0 : Use Inner Window, 1 : Use Outer Window)
    {0x002A, 0x15C0},
    {0x0F12, 0x0060}, // #af_scene_usSceneLowNormBrThr
    {0x002A, 0x1560},
    {0x0F12, 0x0280}, // #LowEdgeBoth GRAD af_search_usConfThr_4_
    {0x002A, 0x156C},
    {0x0F12, 0x0390}, // #LowLight HPF af_search_usConfThr_10_
    {0x0F12, 0x0320},
    {0x002A, 0x15A0},
    {0x0F12, 0x0230}, // #af_stat_usMinStatVal
    {0x002A, 0x14CC},
    {0x0F12, 0x0000}, // #af_pos_usHomePos
    {0x0F12, 0x9400}, // #af_pos_usLowConfPos
    {0x002A, 0x1640},
    {0x0F12, 0x00A0}, // #afd_usParam[5]SlowMotionDelay
    {0x0F12, 0x0010}, // #afd_usParam[6]SlowMotionThreshold
    {0x002A, 0x14E0},
    {0x0F12, 0x0010}, // #af_pos_usTableLastInd // Steps
    {0x0F12, 0x0000}, // #af_pos_usTable_0_   // af_pos_usTable
    {0x0F12, 0x003C}, // #af_pos_usTable_1_
    {0x0F12, 0x0041}, // #af_pos_usTable_2_
    {0x0F12, 0x0046}, // #af_pos_usTable_3_
    {0x0F12, 0x004B}, // #af_pos_usTable_4_
    {0x0F12, 0x0050}, // #af_pos_usTable_5_
    {0x0F12, 0x0055}, // #af_pos_usTable_6_
    {0x0F12, 0x005A}, // #af_pos_usTable_7_
    {0x0F12, 0x005F}, // #af_pos_usTable_8_
    {0x0F12, 0x0064}, // #af_pos_usTable_9_
    {0x0F12, 0x0069}, // #af_pos_usTable_10_
    {0x0F12, 0x006E}, // #af_pos_usTable_11_
    {0x0F12, 0x0073}, // #af_pos_usTable_12_
    {0x0F12, 0x0078}, // #af_pos_usTable_13_
    {0x0F12, 0x007D}, // #af_pos_usTable_14_
    {0x0F12, 0x0086}, // #af_pos_usTable_15_
    {0x0F12, 0x0090}, // #af_pos_usTable_16_ AF Fail
    {0x002A, 0x14D2},
    {0x0F12, 0x0B00}, // #af_pos_usMacroStartEnd
    {0x002A, 0x02BC},
    {0x0F12, 0x0003}, // #REG_TC_AF_AfCmd
    {0x002A, 0x2A2C},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0303},
    {0x0F12, 0x0303},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0303},
    {0x0F12, 0x0303},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0303},
    {0x0F12, 0x0303},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0303},
    {0x0F12, 0x0303},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x0F12, 0x0101},
    {0x002A, 0x13C8},
    {0x0F12, 0x003A}, // #TVAR_ae_BrAve
    {0x002A, 0x13CE},
    {0x0F12, 0x000F}, // #ae_StatMode
    {0x002A, 0x05B8}, //// SET EXPOSURE&GAIN
    {0x0F12, 0x59D8}, // 6D60 // #lt_uMaxExp1
    {0x0F12, 0x0000}, // (x4 8000 / 70ms)
    {0x0F12, 0xAFC9}, // C350 // #lt_uMaxExp2
    {0x0F12, 0x0000}, // (x4 25000 / 250ms)
    {0x0F12, 0x59D8}, // 6D60 // #lt_uCapMaxExp1
    {0x0F12, 0x0000}, // (x4 8000 / 70ms)
    {0x0F12, 0xAFC9}, // C350 // #lt_uCapMaxExp2
    {0x0F12, 0x0000}, // (x4 25000 / 250ms)
    {0x002A, 0x05C8},
    {0x0F12, 0x0300}, // #lt_uMaxAnGain1
    {0x0F12, 0x0500}, // #lt_uMaxAnGain2
    {0x0F12, 0x0100}, // #lt_uMaxDGain
    {0x0F12, 0x1000}, // #lt_uMaxTotGain (Limit Gain)
    {0x002A, 0x059C},
    {0x0F12, 0x0111}, // #lt_uLimitHigh
    {0x0F12, 0x00EF}, // #lt_uLimitLow
    {0x002A, 0x14A6}, // #ae_GainsOut (for fast AE)
    {0x0F12, 0x0001}, // #ae_GainOut_0_
    {0x0F12, 0x0008}, // #ae_GainOut_1_
    {0x0F12, 0x0015}, // #ae_GainOut_2_
    {0x002A, 0x0532},
    {0x0F12, 0x077F}, // Auto algorithm on
    {0x002A, 0x0EE2},
    {0x0F12, 0x0001}, // #AFC_Default60Hz start auto
    {0x002A, 0x0630},
    {0x0F12, 0x0000}, // #lt_ExpGain_uSubsamplingmode // Preview0: table off, 1: table[1] on, 2: table[2] on
    {0x0F12, 0x0000}, // #lt_ExpGain_uNonSubsampling // capture
    {0x002A, 0x05FC},
    {0x0F12, 0x0800}, // #lt_MBR_uMaxAnGain // set as 2 times lagerer than Normal Total gain
    {0x0F12, 0x0200}, // #lt_MBR_uMaxDigGain
    {0x0F12, 0x0001}, // #lt_MBR_ulExpIn_0_
    {0x0F12, 0x0000},
    {0x0F12, 0x0A3C}, // #lt_MBR_ulExpIn_1_
    {0x0F12, 0x0000},
    {0x0F12, 0x3408}, // #lt_MBR_ulExpIn_2_
    {0x0F12, 0x0000},
    {0x0F12, 0x764E}, // #lt_MBR_ulExpIn_3_
    {0x0F12, 0x0000},
    {0x0F12, 0xCB26}, // #lt_MBR_ulExpIn_4_
    {0x0F12, 0x0000},
    {0x002A, 0x0614},
    {0x0F12, 0x0001}, // #lt_MBR_ulExpOut_0_
    {0x0F12, 0x0000},
    {0x0F12, 0x0516}, // #lt_MBR_ulExpOut_1_
    {0x0F12, 0x0000},
    {0x0F12, 0x1A04}, // #lt_MBR_ulExpOut_2_
    {0x0F12, 0x0000},
    {0x0F12, 0x3408}, // #lt_MBR_ulExpOut_3_
    {0x0F12, 0x0000},
    {0x0F12, 0x6810}, // #lt_MBR_ulExpOut_4_
    {0x0F12, 0x0000},
    {0x002A, 0x162C},
    {0x0F12, 0x0001}, // #DIS_usHorThres
    {0x0F12, 0x0001}, // #DIS_usVerThres
    {0x002A, 0x162A},
    {0x0F12, 0x0000}, // 1:enable DIS_DIS_enable
    {0x002A, 0x08D4},
    {0x0F12, 0x0000}, // #wbt_bUseOutdoorASH
    {0x002A, 0x08F4},
    {0x0F12, 0x0CE6}, // #TVAR_ash_pGAS_high
    {0x0F12, 0x7000},
    {0x002A, 0x08F8},
    {0x0F12, 0x0D76}, // #TVAR_ash_pGAS_low
    {0x0F12, 0x7000},
    {0x002A, 0x0CE6},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f0f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x0000},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x000f},
    {0x0F12, 0x0f00},
    {0x0F12, 0x0f00},
    {0x0F12, 0x000f},
    {0x002A, 0x0D76},
    {0x0F12, 0x9cdd},
    {0x0F12, 0xe1b8},
    {0x0F12, 0x08e7},
    {0x0F12, 0x0fb3},
    {0x0F12, 0xea38},
    {0x0F12, 0x0e9e},
    {0x0F12, 0xdda1},
    {0x0F12, 0xfd5b},
    {0x0F12, 0xf5c6},
    {0x0F12, 0x09b4},
    {0x0F12, 0x11c3},
    {0x0F12, 0xe431},
    {0x0F12, 0xf693},
    {0x0F12, 0xf60c},
    {0x0F12, 0x27c2},
    {0x0F12, 0xdcb0},
    {0x0F12, 0xe770},
    {0x0F12, 0x34b8},
    {0x0F12, 0x391a},
    {0x0F12, 0x0913},
    {0x0F12, 0xd5c2},
    {0x0F12, 0x22d3},
    {0x0F12, 0x1af3},
    {0x0F12, 0xcf1e},
    {0x0F12, 0xc4b5},
    {0x0F12, 0x1128},
    {0x0F12, 0xfe6c},
    {0x0F12, 0x0086},
    {0x0F12, 0xfb0b},
    {0x0F12, 0xf4c7},
    {0x0F12, 0x1dc9},
    {0x0F12, 0xe5a4},
    {0x0F12, 0x1dd9},
    {0x0F12, 0xeafb},
    {0x0F12, 0xed7f},
    {0x0F12, 0x330b},
    {0x0F12, 0xb1da},
    {0x0F12, 0xda70},
    {0x0F12, 0x0d86},
    {0x0F12, 0x0b57},
    {0x0F12, 0xf54f},
    {0x0F12, 0x0486},
    {0x0F12, 0xd7f3},
    {0x0F12, 0xf8ea},
    {0x0F12, 0xf731},
    {0x0F12, 0x0e8c},
    {0x0F12, 0x056f},
    {0x0F12, 0xf11a},
    {0x0F12, 0xf781},
    {0x0F12, 0xf87a},
    {0x0F12, 0x256b},
    {0x0F12, 0xdd39},
    {0x0F12, 0xec49},
    {0x0F12, 0x2ade},
    {0x0F12, 0x3287},
    {0x0F12, 0x0d2c},
    {0x0F12, 0xd779},
    {0x0F12, 0x1a85},
    {0x0F12, 0x1f9f},
    {0x0F12, 0xd54e},
    {0x0F12, 0xe015},
    {0x0F12, 0x055b},
    {0x0F12, 0xfd06},
    {0x0F12, 0x0d58},
    {0x0F12, 0xf992},
    {0x0F12, 0xe6c8},
    {0x0F12, 0x0261},
    {0x0F12, 0xf1a9},
    {0x0F12, 0x1d9f},
    {0x0F12, 0xe32c},
    {0x0F12, 0xe8ab},
    {0x0F12, 0x425a},
    {0x0F12, 0x917b},
    {0x0F12, 0xdf7a},
    {0x0F12, 0x0f4f},
    {0x0F12, 0x067d},
    {0x0F12, 0xf36f},
    {0x0F12, 0x0a16},
    {0x0F12, 0xdec4},
    {0x0F12, 0x0285},
    {0x0F12, 0xee55},
    {0x0F12, 0x1297},
    {0x0F12, 0x0c86},
    {0x0F12, 0xe269},
    {0x0F12, 0x011e},
    {0x0F12, 0xf7ec},
    {0x0F12, 0x256e},
    {0x0F12, 0xdafa},
    {0x0F12, 0xe6db},
    {0x0F12, 0x3d01},
    {0x0F12, 0x2373},
    {0x0F12, 0x02a5},
    {0x0F12, 0xdb67},
    {0x0F12, 0x2791},
    {0x0F12, 0x14cc},
    {0x0F12, 0xc7dc},
    {0x0F12, 0xdd70},
    {0x0F12, 0x10f3},
    {0x0F12, 0x00f3},
    {0x0F12, 0xf5bd},
    {0x0F12, 0xff36},
    {0x0F12, 0x02f0},
    {0x0F12, 0x0f2e},
    {0x0F12, 0xebba},
    {0x0F12, 0x1614},
    {0x0F12, 0xf1ba},
    {0x0F12, 0xf573},
    {0x0F12, 0x1f72},
    {0x0F12, 0x9af9},
    {0x0F12, 0xde48},
    {0x0F12, 0x12a1},
    {0x0F12, 0x0046},
    {0x0F12, 0xfcb9},
    {0x0F12, 0x02ff},
    {0x0F12, 0xdc48},
    {0x0F12, 0x0214},
    {0x0F12, 0xefde},
    {0x0F12, 0x1406},
    {0x0F12, 0x04c1},
    {0x0F12, 0xeab8},
    {0x0F12, 0xfa74},
    {0x0F12, 0xf092},
    {0x0F12, 0x2843},
    {0x0F12, 0xd7b7},
    {0x0F12, 0xef72},
    {0x0F12, 0x35a0},
    {0x0F12, 0x3366},
    {0x0F12, 0x0f4c},
    {0x0F12, 0xd958},
    {0x0F12, 0x2351},
    {0x0F12, 0x186c},
    {0x0F12, 0xc47a},
    {0x0F12, 0xce8a},
    {0x0F12, 0x0a48},
    {0x0F12, 0xf859},
    {0x0F12, 0x06d8},
    {0x0F12, 0xf14c},
    {0x0F12, 0x0b15},
    {0x0F12, 0x1508},
    {0x0F12, 0xea85},
    {0x0F12, 0x213c},
    {0x0F12, 0xe468},
    {0x0F12, 0xfa5f},
    {0x0F12, 0x2039},

    {0x002A, 0x0906}, //// SET GAS ALPHA
    {0x0F12, 0x0001}, // #ash_nGasAlphaStep
    {0x0F12, 0x0001}, // #ash_bUseGasAlpha
    {0x0F12, 0x1400}, // #TVAR_ash_GASAlpha_0__0_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_0__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_0__2_
    {0x0F12, 0x0D80}, // #TVAR_ash_GASAlpha_0__3_
    {0x0F12, 0x1380}, // #TVAR_ash_GASAlpha_1__0_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_1__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_1__2_
    {0x0F12, 0x0EA0}, // #TVAR_ash_GASAlpha_1__3_
    {0x0F12, 0x1300}, // #TVAR_ash_GASAlpha_2__0_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_2__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_2__2_
    {0x0F12, 0x0F00}, // #TVAR_ash_GASAlpha_2__3_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_3__0_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_3__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_3__2_
    {0x0F12, 0x0F00}, // #TVAR_ash_GASAlpha_3__3_
    {0x0F12, 0x0F00}, // #TVAR_ash_GASAlpha_4__0_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_4__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_4__2_
    {0x0F12, 0x0F00}, // #TVAR_ash_GASAlpha_4__3_
    {0x0F12, 0x0F00}, // #TVAR_ash_GASAlpha_5__0_//DNP
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_5__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_5__2_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_5__3_
    {0x0F12, 0x0F00}, // #TVAR_ash_GASAlpha_6__0_//D65
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_6__1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_6__2_
    {0x0F12, 0x1000}, // #TVAR_ash_GASAlpha_6__3_
    {0x0F12, 0x1000}, // #TVAR_ash_GASOutdoorAlpha
    {0x0F12, 0x1000}, // #TVAR_ash_GASOutdoorAlpha_0_
    {0x0F12, 0x1000}, // #TVAR_ash_GASOutdoorAlpha_1_
    {0x0F12, 0x1000}, // #TVAR_ash_GASOutdoorAlpha_2_
    {0x0F12, 0x1000}, // #TVAR_ash_GASOutdoorAlpha_3_
    {0x002A, 0x08DC},
    {0x0F12, 0x00C0}, // #TVAR_ash_AwbAshCord_0_
    {0x0F12, 0x00DA}, // #TVAR_ash_AwbAshCord_1_
    {0x0F12, 0x00FA}, // #TVAR_ash_AwbAshCord_2_
    {0x0F12, 0x0105}, // #TVAR_ash_AwbAshCord_3_
    {0x0F12, 0x0120}, // #TVAR_ash_AwbAshCord_4_
    {0x0F12, 0x0140}, // #TVAR_ash_AwbAshCord_5_
    {0x0F12, 0x0180}, // #TVAR_ash_AwbAshCord_6_
    {0x002A, 0x116C},
    {0x0F12, 0x012D}, // #awbb_IntcR
    {0x0F12, 0x012E}, // #awbb_IntcB
    {0x002A, 0x1186},
    {0x0F12, 0x0420}, // #awbb_MvEq_RBthresh
    {0x002A, 0x12DE},
    {0x0F12, 0x2710}, // #awbb_GainsMaxMove
    {0x002A, 0x139E},
    {0x0F12, 0x0034}, // 0034 : Bypass Gain convergence 0134 : No Check AE Stable  // awbb_Use_Filters
    {0x002A, 0x13A8}, // AWB Convergence Speed
    {0x0F12, 0x0004}, // #awbb_WpFilterMinThr Stable
    {0x0F12, 0x0190}, // #awbb_WpFilterMaxThr boost
    {0x0F12, 0x00A0}, // #awbb_WpFilterCoef
    {0x002A, 0x13A2},
    {0x0F12, 0x0500}, // #awbb_GainsInit_0_
    {0x0F12, 0x0400}, // #awbb_GainsInit_1_
    {0x0F12, 0x0650}, // #awbb_GainsInit_2_
    {0x002A, 0x118C},
    {0x0F12, 0x0680}, // #awbb_GamutWidthThr1
    {0x0F12, 0x0338}, // #awbb_GamutHeightThr1
    {0x0F12, 0x0023}, // #awbb_GamutWidthThr2
    {0x0F12, 0x0019}, // #awbb_GamutHeightThr2
    {0x002A, 0x11EE},
    {0x0F12, 0xFEF7}, // #awbb_SCDetectionMap_SEC_StartR_B
    {0x0F12, 0x0021}, // #awbb_SCDetectionMap_SEC_StepR_B
    {0x0F12, 0x1388}, // #awbb_SCDetectionMap_SEC_SunnyNB
    {0x0F12, 0x0122}, // #awbb_SCDetectionMap_SEC_StepNB
    {0x0F12, 0x018F}, // #awbb_SCDetectionMap_SEC_LowTempR_B
    {0x0F12, 0x0096}, // #awbb_SCDetectionMap_SEC_SunnyNBZone
    {0x0F12, 0x000E}, // #awbb_SCDetectionMap_SEC_LowTempR_BZone
    {0x002A, 0x11A0},
    {0x0F12, 0x0032}, // #awbb_LowBr
    {0x0F12, 0x001E}, // #awbb_LowBr_NBzone
    {0x002A, 0x0FD4}, //// SET AWB INDOOR BOUNDARY
    {0x0F12, 0x0376}, // #awbb_IndoorGrZones_m_BGrid_0__m_left
    {0x0F12, 0x0391}, // #awbb_IndoorGrZones_m_BGrid_0__m_right
    {0x0F12, 0x032E}, // #awbb_IndoorGrZones_m_BGrid_1__m_left
    {0x0F12, 0x0386}, // #awbb_IndoorGrZones_m_BGrid_1__m_right
    {0x0F12, 0x030A}, // #awbb_IndoorGrZones_m_BGrid_2__m_left
    {0x0F12, 0x036F}, // #awbb_IndoorGrZones_m_BGrid_2__m_right
    {0x0F12, 0x02DB}, // #awbb_IndoorGrZones_m_BGrid_3__m_left
    {0x0F12, 0x0349}, // #awbb_IndoorGrZones_m_BGrid_3__m_right
    {0x0F12, 0x02B6}, // #awbb_IndoorGrZones_m_BGrid_4__m_left
    {0x0F12, 0x0325}, // #awbb_IndoorGrZones_m_BGrid_4__m_right
    {0x0F12, 0x028F}, // #awbb_IndoorGrZones_m_BGrid_5__m_left
    {0x0F12, 0x0308}, // #awbb_IndoorGrZones_m_BGrid_5__m_right
    {0x0F12, 0x026C}, // #awbb_IndoorGrZones_m_BGrid_6__m_left
    {0x0F12, 0x02E8}, // #awbb_IndoorGrZones_m_BGrid_6__m_right
    {0x0F12, 0x0246}, // #awbb_IndoorGrZones_m_BGrid_7__m_left
    {0x0F12, 0x02C0}, // #awbb_IndoorGrZones_m_BGrid_7__m_right
    {0x0F12, 0x0226}, // #awbb_IndoorGrZones_m_BGrid_8__m_left
    {0x0F12, 0x02A6}, // #awbb_IndoorGrZones_m_BGrid_8__m_right
    {0x0F12, 0x0205}, // #awbb_IndoorGrZones_m_BGrid_9__m_left
    {0x0F12, 0x028C}, // #awbb_IndoorGrZones_m_BGrid_9__m_right
    {0x0F12, 0x01F6}, // #awbb_IndoorGrZones_m_BGrid_10__m_left
    {0x0F12, 0x0273}, // #awbb_IndoorGrZones_m_BGrid_10__m_right
    {0x0F12, 0x01EB}, // #awbb_IndoorGrZones_m_BGrid_11__m_left
    {0x0F12, 0x0261}, // #awbb_IndoorGrZones_m_BGrid_11__m_right
    {0x0F12, 0x01ED}, // #awbb_IndoorGrZones_m_BGrid_12__m_left
    {0x0F12, 0x0247}, // #awbb_IndoorGrZones_m_BGrid_12__m_right
    {0x0F12, 0x0203}, // #awbb_IndoorGrZones_m_BGrid_13__m_left
    {0x0F12, 0x021F}, // #awbb_IndoorGrZones_m_BGrid_13__m_right
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_14__m_left
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_14__m_right
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_15__m_left
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_15__m_right
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_16__m_left
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_16__m_right
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_17__m_left
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_17__m_right
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_18__m_left
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_18__m_right
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_19__m_left
    {0x0F12, 0x0000}, // #awbb_IndoorGrZones_m_BGrid_19__m_right
    {0x0F12, 0x0005}, // #awbb_IndoorGrZones_m_GridStep
    {0x002A, 0x1028},
    {0x0F12, 0x000E}, // #awbb_IndoorGrZones_ZInfo_m_GridSz
    {0x002A, 0x102C},
    {0x0F12, 0x013B}, // #awbb_IndoorGrZones_m_Boffs
    {0x002A, 0x1030}, //// SET AWB OUTDOOR BOUNDARY
    {0x0F12, 0x0277}, // #awbb_OutdoorGrZones_m_BGrid_0__m_left
    {0x0F12, 0x0294}, // #awbb_OutdoorGrZones_m_BGrid_0__m_right
    {0x0F12, 0x0247}, // #awbb_OutdoorGrZones_m_BGrid_1__m_left
    {0x0F12, 0x029B}, // #awbb_OutdoorGrZones_m_BGrid_1__m_right
    {0x0F12, 0x0236}, // #awbb_OutdoorGrZones_m_BGrid_2__m_left
    {0x0F12, 0x0294}, // #awbb_OutdoorGrZones_m_BGrid_2__m_right
    {0x0F12, 0x0222}, // #awbb_OutdoorGrZones_m_BGrid_3__m_left
    {0x0F12, 0x0286}, // #awbb_OutdoorGrZones_m_BGrid_3__m_right
    {0x0F12, 0x020E}, // #awbb_OutdoorGrZones_m_BGrid_4__m_left
    {0x0F12, 0x0276}, // #awbb_OutdoorGrZones_m_BGrid_4__m_right
    {0x0F12, 0x020C}, // #awbb_OutdoorGrZones_m_BGrid_5__m_left
    {0x0F12, 0x0264}, // #awbb_OutdoorGrZones_m_BGrid_5__m_right
    {0x0F12, 0x0210}, // #awbb_OutdoorGrZones_m_BGrid_6__m_left
    {0x0F12, 0x0259}, // #awbb_OutdoorGrZones_m_BGrid_6__m_right
    {0x0F12, 0x0210}, // #awbb_OutdoorGrZones_m_BGrid_7__m_left
    {0x0F12, 0x024E}, // #awbb_OutdoorGrZones_m_BGrid_7__m_right
    {0x0F12, 0x021F}, // #awbb_OutdoorGrZones_m_BGrid_8__m_left
    {0x0F12, 0x0235}, // #awbb_OutdoorGrZones_m_BGrid_8__m_right
    {0x0F12, 0x0000}, // #awbb_OutdoorGrZones_m_BGrid_9__m_left
    {0x0F12, 0x0000}, // #awbb_OutdoorGrZones_m_BGrid_9__m_right
    {0x0F12, 0x0000}, // #awbb_OutdoorGrZones_m_BGrid_10__m_left
    {0x0F12, 0x0000}, // #awbb_OutdoorGrZones_m_BGrid_10__m_right
    {0x0F12, 0x0000}, // #awbb_OutdoorGrZones_m_BGrid_11__m_left
    {0x0F12, 0x0000}, // #awbb_OutdoorGrZones_m_BGrid_11__m_right
    {0x0F12, 0x0004}, // #awbb_OutdoorGrZones_m_GridStep
    {0x002A, 0x1064},
    {0x0F12, 0x0006}, // #awbb_OutdoorGrZones_ZInfo_m_GridSz
    {0x002A, 0x1068},
    {0x0F12, 0x0221}, // #awbb_OutdoorGrZones_m_Boffs
    {0x002A, 0x106C}, //// SET AWB LOWBR BOUNDARY
    {0x0F12, 0x038E}, // #awbb_LowBrGrZones_m_BGrid_0__m_left
    {0x0F12, 0x03DE}, // #awbb_LowBrGrZones_m_BGrid_0__m_right
    {0x0F12, 0x0310}, // #awbb_LowBrGrZones_m_BGrid_1__m_left
    {0x0F12, 0x03D0}, // #awbb_LowBrGrZones_m_BGrid_1__m_right
    {0x0F12, 0x02B7}, // #awbb_LowBrGrZones_m_BGrid_2__m_left
    {0x0F12, 0x0393}, // #awbb_LowBrGrZones_m_BGrid_2__m_right
    {0x0F12, 0x0265}, // #awbb_LowBrGrZones_m_BGrid_3__m_left
    {0x0F12, 0x035F}, // #awbb_LowBrGrZones_m_BGrid_3__m_right
    {0x0F12, 0x0226}, // #awbb_LowBrGrZones_m_BGrid_4__m_left
    {0x0F12, 0x031B}, // #awbb_LowBrGrZones_m_BGrid_4__m_right
    {0x0F12, 0x01EB}, // #awbb_LowBrGrZones_m_BGrid_5__m_left
    {0x0F12, 0x02D2}, // #awbb_LowBrGrZones_m_BGrid_5__m_right
    {0x0F12, 0x01C7}, // #awbb_LowBrGrZones_m_BGrid_6__m_left
    {0x0F12, 0x02A7}, // #awbb_LowBrGrZones_m_BGrid_6__m_right
    {0x0F12, 0x01CC}, // #awbb_LowBrGrZones_m_BGrid_7__m_left
    {0x0F12, 0x0273}, // #awbb_LowBrGrZones_m_BGrid_7__m_right
    {0x0F12, 0x01ED}, // #awbb_LowBrGrZones_m_BGrid_8__m_left
    {0x0F12, 0x023D}, // #awbb_LowBrGrZones_m_BGrid_8__m_right
    {0x0F12, 0x0000}, // #awbb_LowBrGrZones_m_BGrid_9__m_left
    {0x0F12, 0x0000}, // #awbb_LowBrGrZones_m_BGrid_9__m_right
    {0x0F12, 0x0000}, // #awbb_LowBrGrZones_m_BGrid_10__m_left
    {0x0F12, 0x0000}, // #awbb_LowBrGrZones_m_BGrid_10__m_right
    {0x0F12, 0x0000}, // #awbb_LowBrGrZones_m_BGrid_11__m_left
    {0x0F12, 0x0000}, // #awbb_LowBrGrZones_m_BGrid_11__m_right
    {0x0F12, 0x0006}, // #awbb_LowBrGrZones_m_GridStep
    {0x002A, 0x10A0},
    {0x0F12, 0x000C}, // #awbb_LowBrGrZones_ZInfo_m_GridSz
    {0x002A, 0x10A4},
    {0x0F12, 0x0116}, // #awbb_LowBrGrZones_m_Boffs
    {0x002A, 0x13B0},
    {0x0F12, 0x0000}, // #awbb_GridEnable
    {0x002A, 0x083C},
    {0x0F12, 0x0000}, // #seti_bUseOutdoorGamma
    {0x002A, 0x074C}, //// SET GAMMA (INDOOR, OUTDOOR)
    {0x0F12, 0x0000}, // #SARR_usGammaLutRGBIndoor_0__0_
    {0x0F12, 0x0002}, // #SARR_usGammaLutRGBIndoor_0__1_
    {0x0F12, 0x0008}, // #SARR_usGammaLutRGBIndoor_0__2_
    {0x0F12, 0x0020}, // #SARR_usGammaLutRGBIndoor_0__3_
    {0x0F12, 0x0059}, // #SARR_usGammaLutRGBIndoor_0__4_
    {0x0F12, 0x00CF}, // #SARR_usGammaLutRGBIndoor_0__5_
    {0x0F12, 0x0132}, // #SARR_usGammaLutRGBIndoor_0__6_
    {0x0F12, 0x015C}, // #SARR_usGammaLutRGBIndoor_0__7_
    {0x0F12, 0x0180}, // #SARR_usGammaLutRGBIndoor_0__8_
    {0x0F12, 0x01BC}, // #SARR_usGammaLutRGBIndoor_0__9_
    {0x0F12, 0x01E8}, // #SARR_usGammaLutRGBIndoor_0__10_
    {0x0F12, 0x020F}, // #SARR_usGammaLutRGBIndoor_0__11_
    {0x0F12, 0x0232}, // #SARR_usGammaLutRGBIndoor_0__12_
    {0x0F12, 0x0273}, // #SARR_usGammaLutRGBIndoor_0__13_
    {0x0F12, 0x02AF}, // #SARR_usGammaLutRGBIndoor_0__14_
    {0x0F12, 0x0309}, // #SARR_usGammaLutRGBIndoor_0__15_
    {0x0F12, 0x0355}, // #SARR_usGammaLutRGBIndoor_0__16_
    {0x0F12, 0x0394}, // #SARR_usGammaLutRGBIndoor_0__17_
    {0x0F12, 0x03CE}, // #SARR_usGammaLutRGBIndoor_0__18_
    {0x0F12, 0x03FF}, // #SARR_usGammaLutRGBIndoor_0__19_
    {0x0F12, 0x0000}, // #SARR_usGammaLutRGBIndoor_1__0_
    {0x0F12, 0x0002}, // #SARR_usGammaLutRGBIndoor_1__1_
    {0x0F12, 0x0008}, // #SARR_usGammaLutRGBIndoor_1__2_
    {0x0F12, 0x0020}, // #SARR_usGammaLutRGBIndoor_1__3_
    {0x0F12, 0x0059}, // #SARR_usGammaLutRGBIndoor_1__4_
    {0x0F12, 0x00CF}, // #SARR_usGammaLutRGBIndoor_1__5_
    {0x0F12, 0x0132}, // #SARR_usGammaLutRGBIndoor_1__6_
    {0x0F12, 0x015C}, // #SARR_usGammaLutRGBIndoor_1__7_
    {0x0F12, 0x0180}, // #SARR_usGammaLutRGBIndoor_1__8_
    {0x0F12, 0x01BC}, // #SARR_usGammaLutRGBIndoor_1__9_
    {0x0F12, 0x01E8}, // #SARR_usGammaLutRGBIndoor_1__10_
    {0x0F12, 0x020F}, // #SARR_usGammaLutRGBIndoor_1__11_
    {0x0F12, 0x0232}, // #SARR_usGammaLutRGBIndoor_1__12_
    {0x0F12, 0x0273}, // #SARR_usGammaLutRGBIndoor_1__13_
    {0x0F12, 0x02AF}, // #SARR_usGammaLutRGBIndoor_1__14_
    {0x0F12, 0x0309}, // #SARR_usGammaLutRGBIndoor_1__15_
    {0x0F12, 0x0355}, // #SARR_usGammaLutRGBIndoor_1__16_
    {0x0F12, 0x0394}, // #SARR_usGammaLutRGBIndoor_1__17_
    {0x0F12, 0x03CE}, // #SARR_usGammaLutRGBIndoor_1__18_
    {0x0F12, 0x03FF}, // #SARR_usGammaLutRGBIndoor_1__19_
    {0x0F12, 0x0000}, // #SARR_usGammaLutRGBIndoor_2__0_
    {0x0F12, 0x0002}, // #SARR_usGammaLutRGBIndoor_2__1_
    {0x0F12, 0x0008}, // #SARR_usGammaLutRGBIndoor_2__2_
    {0x0F12, 0x0020}, // #SARR_usGammaLutRGBIndoor_2__3_
    {0x0F12, 0x0059}, // #SARR_usGammaLutRGBIndoor_2__4_
    {0x0F12, 0x00CF}, // #SARR_usGammaLutRGBIndoor_2__5_
    {0x0F12, 0x0132}, // #SARR_usGammaLutRGBIndoor_2__6_
    {0x0F12, 0x015C}, // #SARR_usGammaLutRGBIndoor_2__7_
    {0x0F12, 0x0180}, // #SARR_usGammaLutRGBIndoor_2__8_
    {0x0F12, 0x01BC}, // #SARR_usGammaLutRGBIndoor_2__9_
    {0x0F12, 0x01E8}, // #SARR_usGammaLutRGBIndoor_2__10_
    {0x0F12, 0x020F}, // #SARR_usGammaLutRGBIndoor_2__11_
    {0x0F12, 0x0232}, // #SARR_usGammaLutRGBIndoor_2__12_
    {0x0F12, 0x0273}, // #SARR_usGammaLutRGBIndoor_2__13_
    {0x0F12, 0x02AF}, // #SARR_usGammaLutRGBIndoor_2__14_
    {0x0F12, 0x0309}, // #SARR_usGammaLutRGBIndoor_2__15_
    {0x0F12, 0x0355}, // #SARR_usGammaLutRGBIndoor_2__16_
    {0x0F12, 0x0394}, // #SARR_usGammaLutRGBIndoor_2__17_
    {0x0F12, 0x03CE}, // #SARR_usGammaLutRGBIndoor_2__18_
    {0x0F12, 0x03FF}, // #SARR_usGammaLutRGBIndoor_2__19_
    {0x0F12, 0x0000}, // #SARR_usGammaLutRGBOutdoor_0__0_
    {0x0F12, 0x0002}, // #SARR_usGammaLutRGBOutdoor_0__1_
    {0x0F12, 0x0008}, // #SARR_usGammaLutRGBOutdoor_0__2_
    {0x0F12, 0x0020}, // #SARR_usGammaLutRGBOutdoor_0__3_
    {0x0F12, 0x0059}, // #SARR_usGammaLutRGBOutdoor_0__4_
    {0x0F12, 0x00CF}, // #SARR_usGammaLutRGBOutdoor_0__5_
    {0x0F12, 0x0132}, // #SARR_usGammaLutRGBOutdoor_0__6_
    {0x0F12, 0x015C}, // #SARR_usGammaLutRGBOutdoor_0__7_
    {0x0F12, 0x0180}, // #SARR_usGammaLutRGBOutdoor_0__8_
    {0x0F12, 0x01BC}, // #SARR_usGammaLutRGBOutdoor_0__9_
    {0x0F12, 0x01E8}, // #SARR_usGammaLutRGBOutdoor_0__10_
    {0x0F12, 0x020F}, // #SARR_usGammaLutRGBOutdoor_0__11_
    {0x0F12, 0x0232}, // #SARR_usGammaLutRGBOutdoor_0__12_
    {0x0F12, 0x0273}, // #SARR_usGammaLutRGBOutdoor_0__13_
    {0x0F12, 0x02AF}, // #SARR_usGammaLutRGBOutdoor_0__14_
    {0x0F12, 0x0309}, // #SARR_usGammaLutRGBOutdoor_0__15_
    {0x0F12, 0x0355}, // #SARR_usGammaLutRGBOutdoor_0__16_
    {0x0F12, 0x0394}, // #SARR_usGammaLutRGBOutdoor_0__17_
    {0x0F12, 0x03CE}, // #SARR_usGammaLutRGBOutdoor_0__18_
    {0x0F12, 0x03FF}, // #SARR_usGammaLutRGBOutdoor_0__19_
    {0x0F12, 0x0000}, // #SARR_usGammaLutRGBOutdoor_1__0_
    {0x0F12, 0x0002}, // #SARR_usGammaLutRGBOutdoor_1__1_
    {0x0F12, 0x0008}, // #SARR_usGammaLutRGBOutdoor_1__2_
    {0x0F12, 0x0020}, // #SARR_usGammaLutRGBOutdoor_1__3_
    {0x0F12, 0x0059}, // #SARR_usGammaLutRGBOutdoor_1__4_
    {0x0F12, 0x00CF}, // #SARR_usGammaLutRGBOutdoor_1__5_
    {0x0F12, 0x0132}, // #SARR_usGammaLutRGBOutdoor_1__6_
    {0x0F12, 0x015C}, // #SARR_usGammaLutRGBOutdoor_1__7_
    {0x0F12, 0x0180}, // #SARR_usGammaLutRGBOutdoor_1__8_
    {0x0F12, 0x01BC}, // #SARR_usGammaLutRGBOutdoor_1__9_
    {0x0F12, 0x01E8}, // #SARR_usGammaLutRGBOutdoor_1__10_
    {0x0F12, 0x020F}, // #SARR_usGammaLutRGBOutdoor_1__11_
    {0x0F12, 0x0232}, // #SARR_usGammaLutRGBOutdoor_1__12_
    {0x0F12, 0x0273}, // #SARR_usGammaLutRGBOutdoor_1__13_
    {0x0F12, 0x02AF}, // #SARR_usGammaLutRGBOutdoor_1__14_
    {0x0F12, 0x0309}, // #SARR_usGammaLutRGBOutdoor_1__15_
    {0x0F12, 0x0355}, // #SARR_usGammaLutRGBOutdoor_1__16_
    {0x0F12, 0x0394}, // #SARR_usGammaLutRGBOutdoor_1__17_
    {0x0F12, 0x03CE}, // #SARR_usGammaLutRGBOutdoor_1__18_
    {0x0F12, 0x03FF}, // #SARR_usGammaLutRGBOutdoor_1__19_
    {0x0F12, 0x0000}, // #SARR_usGammaLutRGBOutdoor_2__0_
    {0x0F12, 0x0002}, // #SARR_usGammaLutRGBOutdoor_2__1_
    {0x0F12, 0x0008}, // #SARR_usGammaLutRGBOutdoor_2__2_
    {0x0F12, 0x0020}, // #SARR_usGammaLutRGBOutdoor_2__3_
    {0x0F12, 0x0059}, // #SARR_usGammaLutRGBOutdoor_2__4_
    {0x0F12, 0x00CF}, // #SARR_usGammaLutRGBOutdoor_2__5_
    {0x0F12, 0x0132}, // #SARR_usGammaLutRGBOutdoor_2__6_
    {0x0F12, 0x015C}, // #SARR_usGammaLutRGBOutdoor_2__7_
    {0x0F12, 0x0180}, // #SARR_usGammaLutRGBOutdoor_2__8_
    {0x0F12, 0x01BC}, // #SARR_usGammaLutRGBOutdoor_2__9_
    {0x0F12, 0x01E8}, // #SARR_usGammaLutRGBOutdoor_2__10_
    {0x0F12, 0x020F}, // #SARR_usGammaLutRGBOutdoor_2__11_
    {0x0F12, 0x0232}, // #SARR_usGammaLutRGBOutdoor_2__12_
    {0x0F12, 0x0273}, // #SARR_usGammaLutRGBOutdoor_2__13_
    {0x0F12, 0x02AF}, // #SARR_usGammaLutRGBOutdoor_2__14_
    {0x0F12, 0x0309}, // #SARR_usGammaLutRGBOutdoor_2__15_
    {0x0F12, 0x0355}, // #SARR_usGammaLutRGBOutdoor_2__16_
    {0x0F12, 0x0394}, // #SARR_usGammaLutRGBOutdoor_2__17_
    {0x0F12, 0x03CE}, // #SARR_usGammaLutRGBOutdoor_2__18_
    {0x0F12, 0x03FF}, // #SARR_usGammaLutRGBOutdoor_2__19_


};


unsigned short s5k4eagx_init_reg45[][2] = {
    {0x0028, 0x7000},
    {0x002A, 0x08B8}, //// SET CCM
    {0x0F12, 0x3700}, // #TVAR_wbt_pBaseCcms
    {0x0F12, 0x7000},
    {0x002A, 0x3700},
    {0x0F12, 0x01CB}, // #TVAR_wbt_pBaseCcms[0]// R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pBaseCcms[1]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pBaseCcms[2]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pBaseCcms[3]// G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pBaseCcms[4]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pBaseCcms[5]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pBaseCcms[6]// B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pBaseCcms[7]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pBaseCcms[8]
    {0x0F12, 0x011C}, // #TVAR_wbt_pBaseCcms[9]// Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pBaseCcms[10]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pBaseCcms[11]
    {0x0F12, 0x019D}, // #TVAR_wbt_pBaseCcms[12]  // M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pBaseCcms[13]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pBaseCcms[14]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pBaseCcms[15]  // C
    {0x0F12, 0x0173}, // #TVAR_wbt_pBaseCcms[16]
    {0x0F12, 0x012F}, // #TVAR_wbt_pBaseCcms[17]
    {0x0F12, 0x01CB}, // #TVAR_wbt_pBaseCcms[18]  // R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pBaseCcms[19]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pBaseCcms[20]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pBaseCcms[21]  // G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pBaseCcms[22]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pBaseCcms[23]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pBaseCcms[24]  // B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pBaseCcms[25]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pBaseCcms[26]
    {0x0F12, 0x011C}, // #TVAR_wbt_pBaseCcms[27]  // Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pBaseCcms[28]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pBaseCcms[29]
    {0x0F12, 0x019D}, // #TVAR_wbt_pBaseCcms[30]  // M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pBaseCcms[31]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pBaseCcms[32]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pBaseCcms[33]  // C
    {0x0F12, 0x0173}, // #TVAR_wbt_pBaseCcms[34]
    {0x0F12, 0x012F}, // #TVAR_wbt_pBaseCcms[35]
    {0x0F12, 0x01CB}, // #TVAR_wbt_pBaseCcms[36] // R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pBaseCcms[37]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pBaseCcms[38]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pBaseCcms[39]  // G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pBaseCcms[40]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pBaseCcms[41]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pBaseCcms[42]  // B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pBaseCcms[43]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pBaseCcms[44]
    {0x0F12, 0x011C}, // #TVAR_wbt_pBaseCcms[45]  // Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pBaseCcms[46]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pBaseCcms[47]
    {0x0F12, 0x019D}, // #TVAR_wbt_pBaseCcms[48]  // M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pBaseCcms[49]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pBaseCcms[50]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pBaseCcms[51]  // C
    {0x0F12, 0x0173}, // #TVAR_wbt_pBaseCcms[52]
    {0x0F12, 0x012F}, // #TVAR_wbt_pBaseCcms[53]
    {0x0F12, 0x01CB}, // #TVAR_wbt_pBaseCcms[54] // R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pBaseCcms[55]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pBaseCcms[56]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pBaseCcms[57]  // G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pBaseCcms[58]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pBaseCcms[59]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pBaseCcms[60]  // B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pBaseCcms[61]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pBaseCcms[62]
    {0x0F12, 0x011C}, // #TVAR_wbt_pBaseCcms[63]  // Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pBaseCcms[64]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pBaseCcms[65]
    {0x0F12, 0x019D}, // #TVAR_wbt_pBaseCcms[66]  // M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pBaseCcms[67]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pBaseCcms[68]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pBaseCcms[69]  // C
    {0x0F12, 0x0173}, // #TVAR_wbt_pBaseCcms[70]
    {0x0F12, 0x012F}, // #TVAR_wbt_pBaseCcms[71]
    {0x0F12, 0x01CB}, // #TVAR_wbt_pBaseCcms[72]  // R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pBaseCcms[73]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pBaseCcms[74]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pBaseCcms[75]  // G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pBaseCcms[76]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pBaseCcms[77]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pBaseCcms[78]  // B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pBaseCcms[79]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pBaseCcms[80]
    {0x0F12, 0x011C}, // #TVAR_wbt_pBaseCcms[81]  // Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pBaseCcms[82]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pBaseCcms[83]
    {0x0F12, 0x019D}, // #TVAR_wbt_pBaseCcms[84]  // M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pBaseCcms[85]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pBaseCcms[86]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pBaseCcms[87]  // C
    {0x0F12, 0x0173}, // #TVAR_wbt_pBaseCcms[88]
    {0x0F12, 0x012F}, // #TVAR_wbt_pBaseCcms[89]
    {0x0F12, 0x01CB}, // #TVAR_wbt_pBaseCcms[90] // R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pBaseCcms[91]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pBaseCcms[92]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pBaseCcms[93]  // G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pBaseCcms[94]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pBaseCcms[95]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pBaseCcms[96]  // B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pBaseCcms[97]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pBaseCcms[98]
    {0x0F12, 0x011C}, // #TVAR_wbt_pBaseCcms[99]  // Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pBaseCcms[100]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pBaseCcms[101]
    {0x0F12, 0x019D}, // #TVAR_wbt_pBaseCcms[102] // M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pBaseCcms[103]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pBaseCcms[104]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pBaseCcms[105] // C
    {0x0F12, 0x0173}, // #TVAR_wbt_pBaseCcms[106]
    {0x0F12, 0x012F}, // #TVAR_wbt_pBaseCcms[107]
    {0x002A, 0x08C0},
    {0x0F12, 0x37D8}, // #TVAR_wbt_pOutdoorCcm
    {0x0F12, 0x7000},
    {0x002A, 0x37D8},
    {0x0F12, 0x01CB}, // #TVAR_wbt_pOutdoorCcm[0] // R
    {0x0F12, 0xFF8E}, // #TVAR_wbt_pOutdoorCcm[1]
    {0x0F12, 0xFFD2}, // #TVAR_wbt_pOutdoorCcm[2]
    {0x0F12, 0xFF64}, // #TVAR_wbt_pOutdoorCcm[3] // G
    {0x0F12, 0x01B2}, // #TVAR_wbt_pOutdoorCcm[4]
    {0x0F12, 0xFF35}, // #TVAR_wbt_pOutdoorCcm[5]
    {0x0F12, 0xFFDF}, // #TVAR_wbt_pOutdoorCcm[6] // B
    {0x0F12, 0xFFE9}, // #TVAR_wbt_pOutdoorCcm[7]
    {0x0F12, 0x01BD}, // #TVAR_wbt_pOutdoorCcm[8]
    {0x0F12, 0x011C}, // #TVAR_wbt_pOutdoorCcm[9] // Y
    {0x0F12, 0x011B}, // #TVAR_wbt_pOutdoorCcm[10]
    {0x0F12, 0xFF43}, // #TVAR_wbt_pOutdoorCcm[11]
    {0x0F12, 0x019D}, // #TVAR_wbt_pOutdoorCcm[12]// M
    {0x0F12, 0xFF4C}, // #TVAR_wbt_pOutdoorCcm[13]
    {0x0F12, 0x01CC}, // #TVAR_wbt_pOutdoorCcm[14]
    {0x0F12, 0xFF33}, // #TVAR_wbt_pOutdoorCcm[15]// C
    {0x0F12, 0x0173}, // #TVAR_wbt_pOutdoorCcm[16]
    {0x0F12, 0x012F}, // #TVAR_wbt_pOutdoorCcm[17]
    {0x002A, 0x08C6},
    {0x0F12, 0x00D0}, // #SARR_AwbCcmCord_0_
    {0x0F12, 0x00E0}, // #SARR_AwbCcmCord_1_
    {0x0F12, 0x0100}, // #SARR_AwbCcmCord_2_
    {0x0F12, 0x0110}, // #SARR_AwbCcmCord_3_
    {0x0F12, 0x0130}, // #SARR_AwbCcmCord_4_
    {0x0F12, 0x0180}, // #SARR_AwbCcmCord_5_
    {0x002A, 0x099A},
    {0x0F12, 0x0000}, // #afit_bAfitExOffMBR
    {0x002A, 0x0958},
    {0x0F12, 0x0041}, // #afit_uNoiseIndInDoor_0_
    {0x0F12, 0x0063}, // #afit_uNoiseIndInDoor_1_
    {0x0F12, 0x00C8}, // #afit_uNoiseIndInDoor_2_
    {0x0F12, 0x0151}, // #afit_uNoiseIndInDoor_3_
    {0x0F12, 0x02A3}, // #afit_uNoiseIndInDoor_4_
    {0x002A, 0x0964},
    {0x0F12, 0x099C}, // #TVAR_afit_pBaseVals

    {0x002A, 0x099C}, //// SET AFIT
    {0x0F12, 0x0000}, //0000// #AFIT16_BRIGHTNESS //x5
    {0x0F12, 0x0000}, //0000// #AFIT16_CONTRAST
    {0x0F12, 0x0000}, //0000// #AFIT16_SATURATION
    {0x0F12, 0x0005}, //0005// #AFIT16_SHARP_BLUR
    {0x0F12, 0xFFFB}, //FFFB// #AFIT16_GLAMOUR
    {0x0F12, 0x0080}, //0080// #AFIT16_DDD_edge_high
    {0x0F12, 0x03FF}, //03FF// #AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x0000}, //0000// #AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x005A}, //005A// #AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0000}, //0000// #AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0040}, //0040// #AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0019}, //0019// #AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iRGBOffset
    {0x0F12, 0x01F4}, //01F4// #AFIT16_DemSharpMix_iDemClamp
    {0x0F12, 0x00D5}, //00D5// #AFIT16_DemSharpMix_iLowThreshold
    {0x0F12, 0x00DE}, //00DE// #AFIT16_DemSharpMix_iHighThreshold
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iLowBright
    {0x0F12, 0x03E8}, //03E8// #AFIT16_DemSharpMix_iHighBright
    {0x0F12, 0x0020}, //0020// #AFIT16_DemSharpMix_iLowSat
    {0x0F12, 0x0078}, //0078// #AFIT16_DemSharpMix_iHighSat
    {0x0F12, 0x0070}, //0070// #AFIT16_DemSharpMix_iTune
    {0x0F12, 0x000A}, //000A// #AFIT16_DemSharpMix_iHystThLow
    {0x0F12, 0x000A}, //000A// #AFIT16_DemSharpMix_iHystThHigh
    {0x0F12, 0x01CE}, //01CE// #AFIT16_DemSharpMix_iHystCenter
    {0x0F12, 0x0008}, //0008// #AFIT8_DDD_edge_low, AFIT8_DDD_repl_thresh
    {0x0F12, 0xF804}, //F804// #AFIT8_DDD_repl_force, AFIT8_DDD_sat_level
    {0x0F12, 0x010C}, //010C// #AFIT8_DDD_sat_thr , AFIT8_DDD_sat_mpl
    {0x0F12, 0x0003}, //0003// #AFIT8_DDD_sat_noise , AFIT8_DDD_iClustThresh_H
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iClustThresh_H_Bin,  AFIT8_DDD_iClustThresh_C
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iClustThresh_C_Bin,  AFIT8_DDD_iClustMulT_H
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iClustMulT_H_Bin  ,  AFIT8_DDD_iClustMulT_C
    {0x0F12, 0x0100}, //0100// #AFIT8_DDD_iClustMulT_C_Bin  ,  AFIT8_DDD_nClustLevel_H
    {0x0F12, 0x0002}, //0002// #AFIT8_DDD_nClustLevel_H_Bin ,  AFIT8_DDD_iMaxSlopeAllowed
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iHotThreshHigh ,  AFIT8_DDD_iHotThreshLow
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iColdThreshHigh,  AFIT8_DDD_iColdThreshLow
    {0x0F12, 0x8EFF}, //8EFF// #AFIT8_DDD_DispTH_Low, AFIT8_DDD_DispTH_Low_Bin
    {0x0F12, 0x8EFF}, //8EFF// #AFIT8_DDD_DispTH_High ,  AFIT8_DDD_DispTH_High_Bin
    {0x0F12, 0xC8FB}, //C8FB// #AFIT8_DDD_iDenThreshLow  ,  AFIT8_DDD_iDenThreshLow_Bin
    {0x0F12, 0xC8FA}, //C8FA// #AFIT8_DDD_iDenThreshHigh ,  AFIT8_DDD_iDenThreshHigh_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_AddNoisePower1 ,  AFIT8_DDD_AddNoisePower2
    {0x0F12, 0x00FF}, //00FF// #AFIT8_DDD_iSatSat , AFIT8_DDD_iRadialTune
    {0x0F12, 0x0F32}, //0F32// #AFIT8_DDD_iRadialLimit,  AFIT8_DDD_iRadialPower
    {0x0F12, 0x2828}, //2828// #AFIT8_DDD_iLowMaxSlopeAllowed  , AFIT8_DDD_iHighMaxSlopeAllowed
    {0x0F12, 0x0606}, //0606// #AFIT8_DDD_iLowSlopeThresh,  AFIT8_DDD_iHighSlopeThresh
    {0x0F12, 0x8002}, //8002// #AFIT8_DDD_iSquaresRounding  ,  AFIT8_Demosaicing_iCentGrad
    {0x0F12, 0x0620}, //0620// #AFIT8_Demosaicing_iMonochrom,  AFIT8_Demosaicing_iDecisionThresh
    {0x0F12, 0x001E}, //001E// #AFIT8_Demosaicing_iDesatThresh ,  AFIT8_Demosaicing_iEnhThresh
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iGRDenoiseVal,  AFIT8_Demosaicing_iGBDenoiseVal
    {0x0F12, 0x0505}, //0505// #AFIT8_Demosaicing_iEdgeDesat,  AFIT8_Demosaicing_iEdgeDesat_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iEdgeDesatThrLow , AFIT8_Demosaicing_iEdgeDesatThrLow_Bin
    {0x0F12, 0x0A0A}, //0A0A// #AFIT8_Demosaicing_iEdgeDesatThrHigh , AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin
    {0x0F12, 0x0404}, //0404// #AFIT8_Demosaicing_iEdgeDesatLimit , AFIT8_Demosaicing_iEdgeDesatLimit_Bin
    {0x0F12, 0x2306}, //2303// #AFIT8_Demosaicing_iNearGrayDesat  , AFIT8_Demosaicing_iDemSharpenLow
    {0x0F12, 0x3214}, //3214// #AFIT8_Demosaicing_iDemSharpenLow_Bin , AFIT8_Demosaicing_iDemSharpenHigh
    {0x0F12, 0x370A}, //370A// #AFIT8_Demosaicing_iDemSharpenHigh_Bin , AFIT8_Demosaicing_iDemSharpThresh
    {0x0F12, 0x2080}, //2080// #AFIT8_Demosaicing_iDemSharpThresh_Bin , AFIT8_Demosaicing_iDemShLowLimit
    {0x0F12, 0x0408}, //0408// #AFIT8_Demosaicing_iDemShLowLimit_Bin , AFIT8_Demosaicing_iDespeckleForDemsharp
    {0x0F12, 0x1008}, //1008// #AFIT8_Demosaicing_iDespeckleForDemsharp_Bin , AFIT8_Demosaicing_iDFD_ReduceCoeff
    {0x0F12, 0xFF1E}, //FF1E// #AFIT8_Demosaicing_iDemBlurLow  ,  AFIT8_Demosaicing_iDemBlurLow_Bin
    {0x0F12, 0xFF1E}, //FF1E// #AFIT8_Demosaicing_iDemBlurHigh ,  AFIT8_Demosaicing_iDemBlurHigh_Bin
    {0x0F12, 0x0606}, //0606// #AFIT8_Demosaicing_iDemBlurRange,  AFIT8_Demosaicing_iDemBlurRange_Bin
    {0x0F12, 0x1900}, //1900// #AFIT8_Sharpening_iLowSharpPower,  AFIT8_Sharpening_iLowSharpPower_Bin
    {0x0F12, 0x4B00}, //4B00// #AFIT8_Sharpening_iHighSharpPower  , AFIT8_Sharpening_iHighSharpPower_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_Sharpening_iMSharpen  ,  AFIT8_Sharpening_iMShThresh
    {0x0F12, 0x2200}, //2200// #AFIT8_Sharpening_iWSharpen  ,  AFIT8_Sharpening_iWShThresh
    {0x0F12, 0x9A00}, //9A00// #AFIT8_Sharpening_nSharpWidth,  AFIT8_Sharpening_iLowShDenoise
    {0x0F12, 0x7C0A}, //7C0A// #AFIT8_Sharpening_iLowShDenoise_Bin , AFIT8_Sharpening_iHighShDenoise
    {0x0F12, 0x023C}, //023C// #AFIT8_Sharpening_iHighShDenoise_Bin , AFIT8_Sharpening_iReduceNegative
    {0x0F12, 0x4000}, //4000// #AFIT8_Sharpening_iShDespeckle  ,  AFIT8_Sharpening_iReduceEdgeMinMult
    {0x0F12, 0x0F10}, //0F10// #AFIT8_Sharpening_iReduceEdgeMinMult_Bin , AFIT8_Sharpening_iReduceEdgeSlope
    {0x0F12, 0x0004}, //0004// #AFIT8_Sharpening_iReduceEdgeSlope_Bin , AFIT8_DemSharpMix_iRGBMultiplier
    {0x0F12, 0x0F00}, //0F00// #AFIT8_DemSharpMix_iFilterPower,  AFIT8_DemSharpMix_iBCoeff
    {0x0F12, 0x0A18}, //0A18// #AFIT8_DemSharpMix_iGCoeff  ,  AFIT8_DemSharpMix_iWideMult
    {0x0F12, 0x0900}, //0900// #AFIT8_DemSharpMix_iNarrMult,  AFIT8_DemSharpMix_iWideFiltReduce
    {0x0F12, 0x0902}, //0902// #AFIT8_DemSharpMix_iWideFiltReduce_Bin , AFIT8_DemSharpMix_iNarrFiltReduce
    {0x0F12, 0x0403}, //0403// #AFIT8_DemSharpMix_iNarrFiltReduce_Bin , AFIT8_DemSharpMix_iHystFalloff
    {0x0F12, 0x0600}, //0600// #AFIT8_DemSharpMix_iHystMinMult,  AFIT8_DemSharpMix_iHystWidth
    {0x0F12, 0x0201}, //0201// #AFIT8_DemSharpMix_iHystFallLow,  AFIT8_DemSharpMix_iHystFallHigh
    {0x0F12, 0x5004}, //5004// #AFIT8_DemSharpMix1_iHystTune,  AFIT8_byr_gras_iShadingPower
    {0x0F12, 0x0080}, //0080// #AFIT8_RGBGamma2_iLinearity  ,  AFIT8_RGBGamma2_iDarkReduce
    {0x0F12, 0x003C}, //003C// #AFIT8_ccm_oscar_iSaturation ,  AFIT8_RGB2YUV_iYOffset
    {0x0F12, 0x0080}, //0080// #AFIT8_RGB2YUV_iRGBGain , AFIT8_RGB2YUV_iSaturation
    {0x0F12, 0x0080}, //0080// #AFIT8_DRx_iBlendingSupress
    {0x0F12, 0x0000}, //0000// #AFIT16_BRIGHTNESS //x4
    {0x0F12, 0x0000}, //0000// #AFIT16_CONTRAST
    {0x0F12, 0x0000}, //0000// #AFIT16_SATURATION
    {0x0F12, 0x0005}, //0005// #AFIT16_SHARP_BLUR
    {0x0F12, 0xFFFB}, //FFFB// #AFIT16_GLAMOUR
    {0x0F12, 0x0080}, //0080// #AFIT16_DDD_edge_high
    {0x0F12, 0x03FF}, //03FF// #AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x0081}, //0081// #AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x005A}, //005A// #AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0016}, //0016// #AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0040}, //0040// #AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0019}, //0019// #AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iRGBOffset
    {0x0F12, 0x01F4}, //01F4// #AFIT16_DemSharpMix_iDemClamp
    {0x0F12, 0x00D5}, //00D5// #AFIT16_DemSharpMix_iLowThreshold
    {0x0F12, 0x00DE}, //00DE// #AFIT16_DemSharpMix_iHighThreshold
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iLowBright
    {0x0F12, 0x03E8}, //03E8// #AFIT16_DemSharpMix_iHighBright
    {0x0F12, 0x0020}, //0020// #AFIT16_DemSharpMix_iLowSat
    {0x0F12, 0x0078}, //0078// #AFIT16_DemSharpMix_iHighSat
    {0x0F12, 0x0070}, //0070// #AFIT16_DemSharpMix_iTune
    {0x0F12, 0x0005}, //0005// #AFIT16_DemSharpMix_iHystThLow
    {0x0F12, 0x000A}, //000A// #AFIT16_DemSharpMix_iHystThHigh
    {0x0F12, 0x01CE}, //01CE// #AFIT16_DemSharpMix_iHystCenter
    {0x0F12, 0x0008}, //0008// #AFIT8_DDD_edge_low, AFIT8_DDD_repl_thresh
    {0x0F12, 0xF804}, //F804// #AFIT8_DDD_repl_force, AFIT8_DDD_sat_level
    {0x0F12, 0x010C}, //010C// #AFIT8_DDD_sat_thr , AFIT8_DDD_sat_mpl
    {0x0F12, 0x1003}, //1003// #AFIT8_DDD_sat_noise , AFIT8_DDD_iClustThresh_H
    {0x0F12, 0x2010}, //2010// #AFIT8_DDD_iClustThresh_H_Bin,  AFIT8_DDD_iClustThresh_C
    {0x0F12, 0x05FF}, //05FF// #AFIT8_DDD_iClustThresh_C_Bin,  AFIT8_DDD_iClustMulT_H
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iClustMulT_H_Bin  ,  AFIT8_DDD_iClustMulT_C
    {0x0F12, 0x0100}, //0100// #AFIT8_DDD_iClustMulT_C_Bin  ,  AFIT8_DDD_nClustLevel_H
    {0x0F12, 0x0002}, //0002// #AFIT8_DDD_nClustLevel_H_Bin ,  AFIT8_DDD_iMaxSlopeAllowed
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iHotThreshHigh ,  AFIT8_DDD_iHotThreshLow
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iColdThreshHigh,  AFIT8_DDD_iColdThreshLow
    {0x0F12, 0x8E32}, //8E32// #AFIT8_DDD_DispTH_Low, AFIT8_DDD_DispTH_Low_Bin
    {0x0F12, 0x8E32}, //8E32// #AFIT8_DDD_DispTH_High ,  AFIT8_DDD_DispTH_High_Bin
    {0x0F12, 0x6450}, //6450// #AFIT8_DDD_iDenThreshLow  ,  AFIT8_DDD_iDenThreshLow_Bin
    {0x0F12, 0x6432}, //6432// #AFIT8_DDD_iDenThreshHigh ,  AFIT8_DDD_iDenThreshHigh_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_AddNoisePower1 ,  AFIT8_DDD_AddNoisePower2
    {0x0F12, 0x00FF}, //00FF// #AFIT8_DDD_iSatSat , AFIT8_DDD_iRadialTune
    {0x0F12, 0x0F32}, //0F32// #AFIT8_DDD_iRadialLimit,  AFIT8_DDD_iRadialPower
    {0x0F12, 0x2828}, //2828// #AFIT8_DDD_iLowMaxSlopeAllowed  , AFIT8_DDD_iHighMaxSlopeAllowed
    {0x0F12, 0x0606}, //0606// #AFIT8_DDD_iLowSlopeThresh,  AFIT8_DDD_iHighSlopeThresh
    {0x0F12, 0x8002}, //8002// #AFIT8_DDD_iSquaresRounding  ,  AFIT8_Demosaicing_iCentGrad
    {0x0F12, 0x0620}, //0620// #AFIT8_Demosaicing_iMonochrom,  AFIT8_Demosaicing_iDecisionThresh
    {0x0F12, 0x001E}, //001E// #AFIT8_Demosaicing_iDesatThresh ,  AFIT8_Demosaicing_iEnhThresh
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iGRDenoiseVal,  AFIT8_Demosaicing_iGBDenoiseVal
    {0x0F12, 0x0505}, //0505// #AFIT8_Demosaicing_iEdgeDesat,  AFIT8_Demosaicing_iEdgeDesat_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iEdgeDesatThrLow , AFIT8_Demosaicing_iEdgeDesatThrLow_Bin
    {0x0F12, 0x0A0A}, //0A0A// #AFIT8_Demosaicing_iEdgeDesatThrHigh , AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin
    {0x0F12, 0x0404}, //0404// #AFIT8_Demosaicing_iEdgeDesatLimit , AFIT8_Demosaicing_iEdgeDesatLimit_Bin
    {0x0F12, 0x2306}, //2303// #AFIT8_Demosaicing_iNearGrayDesat  , AFIT8_Demosaicing_iDemSharpenLow
    {0x0F12, 0x3214}, //3214// #AFIT8_Demosaicing_iDemSharpenLow_Bin , AFIT8_Demosaicing_iDemSharpenHigh
    {0x0F12, 0x370A}, //370A// #AFIT8_Demosaicing_iDemSharpenHigh_Bin , AFIT8_Demosaicing_iDemSharpThresh
    {0x0F12, 0x2080}, //2080// #AFIT8_Demosaicing_iDemSharpThresh_Bin , AFIT8_Demosaicing_iDemShLowLimit
    {0x0F12, 0x0808}, //0808// #AFIT8_Demosaicing_iDemShLowLimit_Bin , AFIT8_Demosaicing_iDespeckleForDemsharp
    {0x0F12, 0x1008}, //1008// #AFIT8_Demosaicing_iDespeckleForDemsharp_Bin , AFIT8_Demosaicing_iDFD_ReduceCoeff
    {0x0F12, 0xFF1E}, //FF1E// #AFIT8_Demosaicing_iDemBlurLow  ,  AFIT8_Demosaicing_iDemBlurLow_Bin
    {0x0F12, 0xFF19}, //FF19// #AFIT8_Demosaicing_iDemBlurHigh ,  AFIT8_Demosaicing_iDemBlurHigh_Bin
    {0x0F12, 0x0604}, //0604// #AFIT8_Demosaicing_iDemBlurRange,  AFIT8_Demosaicing_iDemBlurRange_Bin
    {0x0F12, 0x191E}, //191E// #AFIT8_Sharpening_iLowSharpPower,  AFIT8_Sharpening_iLowSharpPower_Bin
    {0x0F12, 0x4B1E}, //4B1E// #AFIT8_Sharpening_iHighSharpPower  , AFIT8_Sharpening_iHighSharpPower_Bin
    {0x0F12, 0x0064}, //0064// #AFIT8_Sharpening_iMSharpen  ,  AFIT8_Sharpening_iMShThresh
    {0x0F12, 0x2228}, //2228// #AFIT8_Sharpening_iWSharpen  ,  AFIT8_Sharpening_iWShThresh
    {0x0F12, 0x3200}, //3200// #AFIT8_Sharpening_nSharpWidth,  AFIT8_Sharpening_iLowShDenoise
    {0x0F12, 0x140A}, //140A// #AFIT8_Sharpening_iLowShDenoise_Bin , AFIT8_Sharpening_iHighShDenoise
    {0x0F12, 0x023C}, //023C// #AFIT8_Sharpening_iHighShDenoise_Bin , AFIT8_Sharpening_iReduceNegative
    {0x0F12, 0x4000}, //4000// #AFIT8_Sharpening_iShDespeckle  ,  AFIT8_Sharpening_iReduceEdgeMinMult
    {0x0F12, 0x0F10}, //0F10// #AFIT8_Sharpening_iReduceEdgeMinMult_Bin , AFIT8_Sharpening_iReduceEdgeSlope
    {0x0F12, 0x0004}, //0004// #AFIT8_Sharpening_iReduceEdgeSlope_Bin , AFIT8_DemSharpMix_iRGBMultiplier
    {0x0F12, 0x0F00}, //0F00// #AFIT8_DemSharpMix_iFilterPower,  AFIT8_DemSharpMix_iBCoeff
    {0x0F12, 0x0A18}, //0A18// #AFIT8_DemSharpMix_iGCoeff  ,  AFIT8_DemSharpMix_iWideMult
    {0x0F12, 0x0900}, //0900// #AFIT8_DemSharpMix_iNarrMult,  AFIT8_DemSharpMix_iWideFiltReduce
    {0x0F12, 0x0902}, //0902// #AFIT8_DemSharpMix_iWideFiltReduce_Bin , AFIT8_DemSharpMix_iNarrFiltReduce
    {0x0F12, 0x0003}, //0003// #AFIT8_DemSharpMix_iNarrFiltReduce_Bin , AFIT8_DemSharpMix_iHystFalloff
    {0x0F12, 0x0600}, //0600// #AFIT8_DemSharpMix_iHystMinMult,  AFIT8_DemSharpMix_iHystWidth
    {0x0F12, 0x0201}, //0201// #AFIT8_DemSharpMix_iHystFallLow,  AFIT8_DemSharpMix_iHystFallHigh
    {0x0F12, 0x5A03}, //5A03// #AFIT8_DemSharpMix1_iHystTune,  AFIT8_byr_gras_iShadingPower
    {0x0F12, 0x0080}, //0080// #AFIT8_RGBGamma2_iLinearity  ,  AFIT8_RGBGamma2_iDarkReduce
    {0x0F12, 0x0050}, //0050// #AFIT8_ccm_oscar_iSaturation ,  AFIT8_RGB2YUV_iYOffset
    {0x0F12, 0x0080}, //0080// #AFIT8_RGB2YUV_iRGBGain , AFIT8_RGB2YUV_iSaturation
    {0x0F12, 0x0040}, //0040// #AFIT8_DRx_iBlendingSupress
    {0x0F12, 0x0000}, //0000// #AFIT16_BRIGHTNESS //x3
    {0x0F12, 0x0005}, //0005// #AFIT16_CONTRAST
    {0x0F12, 0x0000}, //0000// #AFIT16_SATURATION
    {0x0F12, 0x0005}, //0005// #AFIT16_SHARP_BLUR
    {0x0F12, 0xFFFB}, //FFFB// #AFIT16_GLAMOUR
    {0x0F12, 0x0080}, //0080// #AFIT16_DDD_edge_high
    {0x0F12, 0x03FF}, //03FF// #AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x0081}, //0081// #AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x005A}, //005A// #AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0031}, //0031// #AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0040}, //0040// #AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0019}, //0019// #AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iRGBOffset
    {0x0F12, 0x01F4}, //01F4// #AFIT16_DemSharpMix_iDemClamp
    {0x0F12, 0x0009}, //0009// #AFIT16_DemSharpMix_iLowThreshold
    {0x0F12, 0x0012}, //0012// #AFIT16_DemSharpMix_iHighThreshold
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iLowBright
    {0x0F12, 0x03E8}, //03E8// #AFIT16_DemSharpMix_iHighBright
    {0x0F12, 0x0020}, //0020// #AFIT16_DemSharpMix_iLowSat
    {0x0F12, 0x0078}, //0078// #AFIT16_DemSharpMix_iHighSat
    {0x0F12, 0x0070}, //0070// #AFIT16_DemSharpMix_iTune
    {0x0F12, 0x0000}, //0000// #AFIT16_DemSharpMix_iHystThLow
    {0x0F12, 0x0005}, //0005// #AFIT16_DemSharpMix_iHystThHigh
    {0x0F12, 0x01CE}, //01CE// #AFIT16_DemSharpMix_iHystCenter
    {0x0F12, 0x0008}, //0008// #AFIT8_DDD_edge_low, AFIT8_DDD_repl_thresh
    {0x0F12, 0xF804}, //F804// #AFIT8_DDD_repl_force, AFIT8_DDD_sat_level
    {0x0F12, 0x010C}, //010C// #AFIT8_DDD_sat_thr , AFIT8_DDD_sat_mpl
    {0x0F12, 0x1003}, //1003// #AFIT8_DDD_sat_noise , AFIT8_DDD_iClustThresh_H
    {0x0F12, 0x2010}, //2010// #AFIT8_DDD_iClustThresh_H_Bin,  AFIT8_DDD_iClustThresh_C
    {0x0F12, 0x05FF}, //05FF// #AFIT8_DDD_iClustThresh_C_Bin,  AFIT8_DDD_iClustMulT_H
    {0x0F12, 0x0305}, //0305// #AFIT8_DDD_iClustMulT_H_Bin  ,  AFIT8_DDD_iClustMulT_C
    {0x0F12, 0x0103}, //0103// #AFIT8_DDD_iClustMulT_C_Bin  ,  AFIT8_DDD_nClustLevel_H
    {0x0F12, 0x0001}, //0001// #AFIT8_DDD_nClustLevel_H_Bin ,  AFIT8_DDD_iMaxSlopeAllowed
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iHotThreshHigh ,  AFIT8_DDD_iHotThreshLow
    {0x0F12, 0x0007}, //0007// #AFIT8_DDD_iColdThreshHigh,  AFIT8_DDD_iColdThreshLow
    {0x0F12, 0x2823}, //2823// #AFIT8_DDD_DispTH_Low, AFIT8_DDD_DispTH_Low_Bin
    {0x0F12, 0x2823}, //2823// #AFIT8_DDD_DispTH_High ,  AFIT8_DDD_DispTH_High_Bin
    {0x0F12, 0x4628}, //4628// #AFIT8_DDD_iDenThreshLow  ,  AFIT8_DDD_iDenThreshLow_Bin
    {0x0F12, 0x4628}, //4628// #AFIT8_DDD_iDenThreshHigh ,  AFIT8_DDD_iDenThreshHigh_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_AddNoisePower1 ,  AFIT8_DDD_AddNoisePower2
    {0x0F12, 0x00FF}, //00FF// #AFIT8_DDD_iSatSat , AFIT8_DDD_iRadialTune
    {0x0F12, 0x0F32}, //0F32// #AFIT8_DDD_iRadialLimit,  AFIT8_DDD_iRadialPower
    {0x0F12, 0x2828}, //2828// #AFIT8_DDD_iLowMaxSlopeAllowed  , AFIT8_DDD_iHighMaxSlopeAllowed
    {0x0F12, 0x0606}, //0606// #AFIT8_DDD_iLowSlopeThresh,  AFIT8_DDD_iHighSlopeThresh
    {0x0F12, 0x8007}, //8007// #AFIT8_DDD_iSquaresRounding  ,  AFIT8_Demosaicing_iCentGrad
    {0x0F12, 0x0620}, //0620// #AFIT8_Demosaicing_iMonochrom,  AFIT8_Demosaicing_iDecisionThresh
    {0x0F12, 0x001E}, //001E// #AFIT8_Demosaicing_iDesatThresh ,  AFIT8_Demosaicing_iEnhThresh
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iGRDenoiseVal,  AFIT8_Demosaicing_iGBDenoiseVal
    {0x0F12, 0x0505}, //0505// #AFIT8_Demosaicing_iEdgeDesat,  AFIT8_Demosaicing_iEdgeDesat_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iEdgeDesatThrLow , AFIT8_Demosaicing_iEdgeDesatThrLow_Bin
    {0x0F12, 0x0A0A}, //0A0A// #AFIT8_Demosaicing_iEdgeDesatThrHigh , AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin
    {0x0F12, 0x0404}, //0404// #AFIT8_Demosaicing_iEdgeDesatLimit , AFIT8_Demosaicing_iEdgeDesatLimit_Bin
    {0x0F12, 0x2306}, //2303// #AFIT8_Demosaicing_iNearGrayDesat  , AFIT8_Demosaicing_iDemSharpenLow
    {0x0F12, 0x3214}, //3214// #AFIT8_Demosaicing_iDemSharpenLow_Bin , AFIT8_Demosaicing_iDemSharpenHigh
    {0x0F12, 0x2D0A}, //2D0A// #AFIT8_Demosaicing_iDemSharpenHigh_Bin , AFIT8_Demosaicing_iDemSharpThresh
    {0x0F12, 0x0880}, //0880// #AFIT8_Demosaicing_iDemSharpThresh_Bin , AFIT8_Demosaicing_iDemShLowLimit
    {0x0F12, 0x0808}, //0808// #AFIT8_Demosaicing_iDemShLowLimit_Bin , AFIT8_Demosaicing_iDespeckleForDemsharp
    {0x0F12, 0x1008}, //1008// #AFIT8_Demosaicing_iDespeckleForDemsharp_Bin , AFIT8_Demosaicing_iDFD_ReduceCoeff
    {0x0F12, 0xFF1E}, //FF1E// #AFIT8_Demosaicing_iDemBlurLow  ,  AFIT8_Demosaicing_iDemBlurLow_Bin
    {0x0F12, 0xFF19}, //FF19// #AFIT8_Demosaicing_iDemBlurHigh ,  AFIT8_Demosaicing_iDemBlurHigh_Bin
    {0x0F12, 0x0604}, //0604// #AFIT8_Demosaicing_iDemBlurRange,  AFIT8_Demosaicing_iDemBlurRange_Bin
    {0x0F12, 0x191E}, //191E// #AFIT8_Sharpening_iLowSharpPower,  AFIT8_Sharpening_iLowSharpPower_Bin
    {0x0F12, 0x4B1E}, //4B1E// #AFIT8_Sharpening_iHighSharpPower  , AFIT8_Sharpening_iHighSharpPower_Bin
    {0x0F12, 0x006F}, //006F// #AFIT8_Sharpening_iMSharpen  ,  AFIT8_Sharpening_iMShThresh
    {0x0F12, 0x223C}, //223C// #AFIT8_Sharpening_iWSharpen  ,  AFIT8_Sharpening_iWShThresh
    {0x0F12, 0x2800}, //2800// #AFIT8_Sharpening_nSharpWidth,  AFIT8_Sharpening_iLowShDenoise
    {0x0F12, 0x0A0A}, //0A0A// #AFIT8_Sharpening_iLowShDenoise_Bin , AFIT8_Sharpening_iHighShDenoise
    {0x0F12, 0x013C}, //013C// #AFIT8_Sharpening_iHighShDenoise_Bin , AFIT8_Sharpening_iReduceNegative
    {0x0F12, 0x4000}, //4000// #AFIT8_Sharpening_iShDespeckle  ,  AFIT8_Sharpening_iReduceEdgeMinMult
    {0x0F12, 0x0F10}, //0F10// #AFIT8_Sharpening_iReduceEdgeMinMult_Bin , AFIT8_Sharpening_iReduceEdgeSlope
    {0x0F12, 0x0004}, //0004// #AFIT8_Sharpening_iReduceEdgeSlope_Bin , AFIT8_DemSharpMix_iRGBMultiplier
    {0x0F12, 0x0F00}, //0F00// #AFIT8_DemSharpMix_iFilterPower,  AFIT8_DemSharpMix_iBCoeff
    {0x0F12, 0x0A18}, //0A18// #AFIT8_DemSharpMix_iGCoeff  ,  AFIT8_DemSharpMix_iWideMult
    {0x0F12, 0x0900}, //0900// #AFIT8_DemSharpMix_iNarrMult,  AFIT8_DemSharpMix_iWideFiltReduce
    {0x0F12, 0x0902}, //0902// #AFIT8_DemSharpMix_iWideFiltReduce_Bin , AFIT8_DemSharpMix_iNarrFiltReduce
    {0x0F12, 0x0003}, //0003// #AFIT8_DemSharpMix_iNarrFiltReduce_Bin , AFIT8_DemSharpMix_iHystFalloff
    {0x0F12, 0x0600}, //0600// #AFIT8_DemSharpMix_iHystMinMult,  AFIT8_DemSharpMix_iHystWidth
    {0x0F12, 0x0201}, //0201// #AFIT8_DemSharpMix_iHystFallLow,  AFIT8_DemSharpMix_iHystFallHigh
    {0x0F12, 0x5A03}, //5A03// #AFIT8_DemSharpMix1_iHystTune,  AFIT8_byr_gras_iShadingPower
    {0x0F12, 0x0080}, //0080// #AFIT8_RGBGamma2_iLinearity  ,  AFIT8_RGBGamma2_iDarkReduce
    {0x0F12, 0x0080}, //0080// #AFIT8_Ccm_oscar_iSaturation ,  AFIT8_RGB2YUV_iYOffset
    {0x0F12, 0x0080}, //0080// #AFIT8_RGB2YUV_iRGBGain , AFIT8_RGB2YUV_iSaturation
    {0x0F12, 0x0020}, //0020// #AFIT8_DRx_iBlendingSupress
    {0x0F12, 0x0000}, //0000// #AFIT16_BRIGHTNESS //x2
    {0x0F12, 0x0005}, //0005// #AFIT16_CONTRAST
    {0x0F12, 0x0000}, //0000// #AFIT16_SATURATION
    {0x0F12, 0x0005}, //0005// #AFIT16_SHARP_BLUR
    {0x0F12, 0xFFFB}, //FFFB// #AFIT16_GLAMOUR
    {0x0F12, 0x0080}, //0080// #AFIT16_DDD_edge_high
    {0x0F12, 0x03FF}, //03FF// #AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x00F3}, //00F3// #AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x005A}, //005A// #AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x004B}, //004B// #AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0040}, //0040// #AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0019}, //0019// #AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iRGBOffset
    {0x0F12, 0x01F4}, //01F4// #AFIT16_DemSharpMix_iDemClamp
    {0x0F12, 0x0009}, //0009// #AFIT16_DemSharpMix_iLowThreshold
    {0x0F12, 0x0012}, //0012// #AFIT16_DemSharpMix_iHighThreshold
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iLowBright
    {0x0F12, 0x03E8}, //03E8// #AFIT16_DemSharpMix_iHighBright
    {0x0F12, 0x0020}, //0020// #AFIT16_DemSharpMix_iLowSat
    {0x0F12, 0x0078}, //0078// #AFIT16_DemSharpMix_iHighSat
    {0x0F12, 0x0070}, //0070// #AFIT16_DemSharpMix_iTune
    {0x0F12, 0x0000}, //0000// #AFIT16_DemSharpMix_iHystThLow
    {0x0F12, 0x0000}, //0000// #AFIT16_DemSharpMix_iHystThHigh
    {0x0F12, 0x01CE}, //01CE// #AFIT16_DemSharpMix_iHystCenter
    {0x0F12, 0x0008}, //0008// #AFIT8_DDD_edge_low, AFIT8_DDD_repl_thresh
    {0x0F12, 0xF804}, //F804// #AFIT8_DDD_repl_force, AFIT8_DDD_sat_level
    {0x0F12, 0x010C}, //010C// #AFIT8_DDD_sat_thr , AFIT8_DDD_sat_mpl
    {0x0F12, 0x1003}, //1003// #AFIT8_DDD_sat_noise , AFIT8_DDD_iClustThresh_H
    {0x0F12, 0x2010}, //2010// #AFIT8_DDD_iClustThresh_H_Bin,  AFIT8_DDD_iClustThresh_C
    {0x0F12, 0x05FF}, //05FF// #AFIT8_DDD_iClustThresh_C_Bin,  AFIT8_DDD_iClustMulT_H
    {0x0F12, 0x0305}, //0305// #AFIT8_DDD_iClustMulT_H_Bin  ,  AFIT8_DDD_iClustMulT_C
    {0x0F12, 0x0103}, //0103// #AFIT8_DDD_iClustMulT_C_Bin  ,  AFIT8_DDD_nClustLevel_H
    {0x0F12, 0x0001}, //0001// #AFIT8_DDD_nClustLevel_H_Bin ,  AFIT8_DDD_iMaxSlopeAllowed
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iHotThreshHigh ,  AFIT8_DDD_iHotThreshLow
    {0x0F12, 0x0007}, //0007// #AFIT8_DDD_iColdThreshHigh,  AFIT8_DDD_iColdThreshLow
    {0x0F12, 0x2823}, //2823// #AFIT8_DDD_DispTH_Low, AFIT8_DDD_DispTH_Low_Bin
    {0x0F12, 0x2823}, //2823// #AFIT8_DDD_DispTH_High ,  AFIT8_DDD_DispTH_High_Bin
    {0x0F12, 0x3228}, //3228// #AFIT8_DDD_iDenThreshLow  ,  AFIT8_DDD_iDenThreshLow_Bin
    {0x0F12, 0x3228}, //3228// #AFIT8_DDD_iDenThreshHigh ,  AFIT8_DDD_iDenThreshHigh_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_AddNoisePower1 ,  AFIT8_DDD_AddNoisePower2
    {0x0F12, 0x00FF}, //00FF// #AFIT8_DDD_iSatSat , AFIT8_DDD_iRadialTune
    {0x0F12, 0x0F32}, //0F32// #AFIT8_DDD_iRadialLimit,  AFIT8_DDD_iRadialPower
    {0x0F12, 0x2828}, //2828// #AFIT8_DDD_iLowMaxSlopeAllowed  , AFIT8_DDD_iHighMaxSlopeAllowed
    {0x0F12, 0x0606}, //0606// #AFIT8_DDD_iLowSlopeThresh,  AFIT8_DDD_iHighSlopeThresh
    {0x0F12, 0x8007}, //8007// #AFIT8_DDD_iSquaresRounding  ,  AFIT8_Demosaicing_iCentGrad
    {0x0F12, 0x0620}, //0620// #AFIT8_Demosaicing_iMonochrom,  AFIT8_Demosaicing_iDecisionThresh
    {0x0F12, 0x001E}, //001E// #AFIT8_Demosaicing_iDesatThresh ,  AFIT8_Demosaicing_iEnhThresh
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iGRDenoiseVal,  AFIT8_Demosaicing_iGBDenoiseVal
    {0x0F12, 0x0505}, //0505// #AFIT8_Demosaicing_iEdgeDesat,  AFIT8_Demosaicing_iEdgeDesat_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iEdgeDesatThrLow , AFIT8_Demosaicing_iEdgeDesatThrLow_Bin
    {0x0F12, 0x0A0A}, //0A0A// #AFIT8_Demosaicing_iEdgeDesatThrHigh , AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin
    {0x0F12, 0x0404}, //0404// #AFIT8_Demosaicing_iEdgeDesatLimit , AFIT8_Demosaicing_iEdgeDesatLimit_Bin
    {0x0F12, 0x2306}, //2303// #AFIT8_Demosaicing_iNearGrayDesat  , AFIT8_Demosaicing_iDemSharpenLow
    {0x0F12, 0x3214}, //3214// #AFIT8_Demosaicing_iDemSharpenLow_Bin , AFIT8_Demosaicing_iDemSharpenHigh
    {0x0F12, 0x2D0A}, //2D0A// #AFIT8_Demosaicing_iDemSharpenHigh_Bin , AFIT8_Demosaicing_iDemSharpThresh
    {0x0F12, 0x0880}, //0880// #AFIT8_Demosaicing_iDemSharpThresh_Bin , AFIT8_Demosaicing_iDemShLowLimit
    {0x0F12, 0x0508}, //0508// #AFIT8_Demosaicing_iDemShLowLimit_Bin , AFIT8_Demosaicing_iDespeckleForDemsharp
    {0x0F12, 0x0A08}, //0A08// #AFIT8_Demosaicing_iDespeckleForDemsharp_Bin , AFIT8_Demosaicing_iDFD_ReduceCoeff
    {0x0F12, 0x801E}, //801E// #AFIT8_Demosaicing_iDemBlurLow  ,  AFIT8_Demosaicing_iDemBlurLow_Bin
    {0x0F12, 0x0019}, //0019// #AFIT8_Demosaicing_iDemBlurHigh ,  AFIT8_Demosaicing_iDemBlurHigh_Bin
    {0x0F12, 0x0604}, //0604// #AFIT8_Demosaicing_iDemBlurRange,  AFIT8_Demosaicing_iDemBlurRange_Bin
    {0x0F12, 0x191E}, //191E// #AFIT8_Sharpening_iLowSharpPower,  AFIT8_Sharpening_iLowSharpPower_Bin
    {0x0F12, 0x4B1E}, //4B1E// #AFIT8_Sharpening_iHighSharpPower  , AFIT8_Sharpening_iHighSharpPower_Bin
    {0x0F12, 0x006F}, //006F// #AFIT8_Sharpening_iMSharpen  ,  AFIT8_Sharpening_iMShThresh
    {0x0F12, 0x113C}, //113C// #AFIT8_Sharpening_iWSharpen  ,  AFIT8_Sharpening_iWShThresh
    {0x0F12, 0x1E00}, //1E00// #AFIT8_Sharpening_nSharpWidth,  AFIT8_Sharpening_iLowShDenoise
    {0x0F12, 0x0A0A}, //0A0A// #AFIT8_Sharpening_iLowShDenoise_Bin , AFIT8_Sharpening_iHighShDenoise
    {0x0F12, 0x013C}, //013C// #AFIT8_Sharpening_iHighShDenoise_Bin , AFIT8_Sharpening_iReduceNegative
    {0x0F12, 0x4000}, //4000// #AFIT8_Sharpening_iShDespeckle  ,  AFIT8_Sharpening_iReduceEdgeMinMult
    {0x0F12, 0x0F10}, //0F10// #AFIT8_Sharpening_iReduceEdgeMinMult_Bin , AFIT8_Sharpening_iReduceEdgeSlope
    {0x0F12, 0x0004}, //0004// #AFIT8_Sharpening_iReduceEdgeSlope_Bin , AFIT8_DemSharpMix_iRGBMultiplier
    {0x0F12, 0x0F00}, //0F00// #AFIT8_DemSharpMix_iFilterPower,  AFIT8_DemSharpMix_iBCoeff
    {0x0F12, 0x0A18}, //0A18// #AFIT8_DemSharpMix_iGCoeff  ,  AFIT8_DemSharpMix_iWideMult
    {0x0F12, 0x0900}, //0900// #AFIT8_DemSharpMix_iNarrMult,  AFIT8_DemSharpMix_iWideFiltReduce
    {0x0F12, 0x0903}, //0903// #AFIT8_DemSharpMix_iWideFiltReduce_Bin , AFIT8_DemSharpMix_iNarrFiltReduce
    {0x0F12, 0x0002}, //0002// #AFIT8_DemSharpMix_iNarrFiltReduce_Bin , AFIT8_DemSharpMix_iHystFalloff
    {0x0F12, 0x0800}, //0800// #AFIT8_DemSharpMix_iHystMinMult,  AFIT8_DemSharpMix_iHystWidth
    {0x0F12, 0x0104}, //0104// #AFIT8_DemSharpMix_iHystFallLow,  AFIT8_DemSharpMix_iHystFallHigh
    {0x0F12, 0x6402}, //6402// #AFIT8_DemSharpMix1_iHystTune,  AFIT8_byr_gras_iShadingPower
    {0x0F12, 0x0080}, //0080// #AFIT8_RGBGamma2_iLinearity  ,  AFIT8_RGBGamma2_iDarkReduce
    {0x0F12, 0x0080}, //0080// #AFIT8_ccm_oscar_iSaturation ,  AFIT8_RGB2YUV_iYOffset
    {0x0F12, 0x0080}, //0080// #AFIT8_RGB2YUV_iRGBGain , AFIT8_RGB2YUV_iSaturation
    {0x0F12, 0x0010}, //0010// #AFIT8_DRx_iBlendingSupress
    {0x0F12, 0x0000}, //0000// #AFIT16_BRIGHTNESS //x1
    {0x0F12, 0x0005}, //0005// #AFIT16_CONTRAST
    {0x0F12, 0x0000}, //0000// #AFIT16_SATURATION
    {0x0F12, 0x0005}, //0005// #AFIT16_SHARP_BLUR
    {0x0F12, 0xFFFB}, //FFFB// #AFIT16_GLAMOUR
    {0x0F12, 0x00C0}, //00C0// #AFIT16_DDD_edge_high
    {0x0F12, 0x03FF}, //03FF// #AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x0144}, //0144// #AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x005A}, //005A// #AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x004B}, //004B// #AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0040}, //0040// #AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0235}, //0235// #AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x0100}, //0100// #AFIT16_DemSharpMix_iRGBOffset
    {0x0F12, 0x01F4}, //01F4// #AFIT16_DemSharpMix_iDemClamp
    {0x0F12, 0x0009}, //0009// #AFIT16_DemSharpMix_iLowThreshold
    {0x0F12, 0x0012}, //0012// #AFIT16_DemSharpMix_iHighThreshold
    {0x0F12, 0x0101}, //0101// #AFIT16_DemSharpMix_iLowBright
    {0x0F12, 0x0341}, //0341// #AFIT16_DemSharpMix_iHighBright
    {0x0F12, 0x0020}, //0020// #AFIT16_DemSharpMix_iLowSat
    {0x0F12, 0x0078}, //0078// #AFIT16_DemSharpMix_iHighSat
    {0x0F12, 0x0070}, //0070// #AFIT16_DemSharpMix_iTune
    {0x0F12, 0x0000}, //0000// #AFIT16_DemSharpMix_iHystThLow
    {0x0F12, 0x0000}, //0000// #AFIT16_DemSharpMix_iHystThHigh
    {0x0F12, 0x01CE}, //01CE// #AFIT16_DemSharpMix_iHystCenter
    {0x0F12, 0x0008}, //0008// #AFIT8_DDD_edge_low, AFIT8_DDD_repl_thresh
    {0x0F12, 0xF804}, //F804// #AFIT8_DDD_repl_force, AFIT8_DDD_sat_level
    {0x0F12, 0x010C}, //010C// #AFIT8_DDD_sat_thr , AFIT8_DDD_sat_mpl
    {0x0F12, 0x2003}, //2003// #AFIT8_DDD_sat_noise , AFIT8_DDD_iClustThresh_H
    {0x0F12, 0x4020}, //4020// #AFIT8_DDD_iClustThresh_H_Bin,  AFIT8_DDD_iClustThresh_C
    {0x0F12, 0x05FF}, //05FF// #AFIT8_DDD_iClustThresh_C_Bin,  AFIT8_DDD_iClustMulT_H
    {0x0F12, 0x0305}, //0305// #AFIT8_DDD_iClustMulT_H_Bin  ,  AFIT8_DDD_iClustMulT_C
    {0x0F12, 0x0103}, //0103// #AFIT8_DDD_iClustMulT_C_Bin  ,  AFIT8_DDD_nClustLevel_H
    {0x0F12, 0x0001}, //0001// #AFIT8_DDD_nClustLevel_H_Bin ,  AFIT8_DDD_iMaxSlopeAllowed
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_iHotThreshHigh ,  AFIT8_DDD_iHotThreshLow
    {0x0F12, 0x0007}, //0007// #AFIT8_DDD_iColdThreshHigh,  AFIT8_DDD_iColdThreshLow
    {0x0F12, 0x1923}, //1923// #AFIT8_DDD_DispTH_Low, AFIT8_DDD_DispTH_Low_Bin
    {0x0F12, 0x1923}, //1923// #AFIT8_DDD_DispTH_High ,  AFIT8_DDD_DispTH_High_Bin
    {0x0F12, 0x141E}, //141E// #AFIT8_DDD_iDenThreshLow  ,  AFIT8_DDD_iDenThreshLow_Bin
    {0x0F12, 0x141E}, //141E// #AFIT8_DDD_iDenThreshHigh ,  AFIT8_DDD_iDenThreshHigh_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_DDD_AddNoisePower1 ,  AFIT8_DDD_AddNoisePower2
    {0x0F12, 0x00FF}, //00FF// #AFIT8_DDD_iSatSat , AFIT8_DDD_iRadialTune
    {0x0F12, 0x0F32}, //0F32// #AFIT8_DDD_iRadialLimit,  AFIT8_DDD_iRadialPower
    {0x0F12, 0x2878}, //2878// #AFIT8_DDD_iLowMaxSlopeAllowed  , AFIT8_DDD_iHighMaxSlopeAllowed
    {0x0F12, 0x0006}, //0006// #AFIT8_DDD_iLowSlopeThresh,  AFIT8_DDD_iHighSlopeThresh
    {0x0F12, 0x8107}, //8107// #AFIT8_DDD_iSquaresRounding  ,  AFIT8_Demosaicing_iCentGrad
    {0x0F12, 0x0620}, //0620// #AFIT8_Demosaicing_iMonochrom,  AFIT8_Demosaicing_iDecisionThresh
    {0x0F12, 0x041E}, //041E// #AFIT8_Demosaicing_iDesatThresh ,  AFIT8_Demosaicing_iEnhThresh
    {0x0F12, 0x0202}, //0202// #AFIT8_Demosaicing_iGRDenoiseVal,  AFIT8_Demosaicing_iGBDenoiseVal
    {0x0F12, 0x0532}, //0532// #AFIT8_Demosaicing_iEdgeDesat,  AFIT8_Demosaicing_iEdgeDesat_Bin
    {0x0F12, 0x0000}, //0000// #AFIT8_Demosaicing_iEdgeDesatThrLow , AFIT8_Demosaicing_iEdgeDesatThrLow_Bin
    {0x0F12, 0x0A00}, //0A00// #AFIT8_Demosaicing_iEdgeDesatThrHigh , AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin
    {0x0F12, 0x0404}, //0404// #AFIT8_Demosaicing_iEdgeDesatLimit , AFIT8_Demosaicing_iEdgeDesatLimit_Bin
    {0x0F12, 0x2306}, //2303// #AFIT8_Demosaicing_iNearGrayDesat  , AFIT8_Demosaicing_iDemSharpenLow
    {0x0F12, 0x3214}, //3214// #AFIT8_Demosaicing_iDemSharpenLow_Bin , AFIT8_Demosaicing_iDemSharpenHigh
    {0x0F12, 0x1E0A}, //1E0A// #AFIT8_Demosaicing_iDemSharpenHigh_Bin , AFIT8_Demosaicing_iDemSharpThresh
    {0x0F12, 0x0880}, //0880// #AFIT8_Demosaicing_iDemSharpThresh_Bin , AFIT8_Demosaicing_iDemShLowLimit
    {0x0F12, 0x0008}, //0008// #AFIT8_Demosaicing_iDemShLowLimit_Bin , AFIT8_Demosaicing_iDespeckleForDemsharp
    {0x0F12, 0x0A08}, //0A08// #AFIT8_Demosaicing_iDespeckleForDemsharp_Bin , AFIT8_Demosaicing_iDFD_ReduceCoeff
    {0x0F12, 0x0005}, //0005// #AFIT8_Demosaicing_iDemBlurLow  ,  AFIT8_Demosaicing_iDemBlurLow_Bin
    {0x0F12, 0x0005}, //0005// #AFIT8_Demosaicing_iDemBlurHigh ,  AFIT8_Demosaicing_iDemBlurHigh_Bin
    {0x0F12, 0x0602}, //0602// #AFIT8_Demosaicing_iDemBlurRange,  AFIT8_Demosaicing_iDemBlurRange_Bin
    {0x0F12, 0x1928}, //1928// #AFIT8_Sharpening_iLowSharpPower,  AFIT8_Sharpening_iLowSharpPower_Bin
    {0x0F12, 0x4B28}, //4B28// #AFIT8_Sharpening_iHighSharpPower , AFIT8_Sharpening_iHighSharpPower_Bin
    {0x0F12, 0x006F}, //006F// #AFIT8_Sharpening_iMSharpen  ,  AFIT8_Sharpening_iMShThresh
    {0x0F12, 0x0028}, //0028// #AFIT8_Sharpening_iWSharpen  ,  AFIT8_Sharpening_iWShThresh
    {0x0F12, 0x0A00}, //0A00// #AFIT8_Sharpening_nSharpWidth,  AFIT8_Sharpening_iLowShDenoise
    {0x0F12, 0x0005}, //0005// #AFIT8_Sharpening_iLowShDenoise_Bin , AFIT8_Sharpening_iHighShDenoise
    {0x0F12, 0x0005}, //0005// #AFIT8_Sharpening_iHighShDenoise_Bin , AFIT8_Sharpening_iReduceNegative
    {0x0F12, 0x13FF}, //13FF// #AFIT8_Sharpening_iShDespeckle  ,  AFIT8_Sharpening_iReduceEdgeMinMult
    {0x0F12, 0x0910}, //0910// #AFIT8_Sharpening_iReduceEdgeMinMult_Bin , AFIT8_Sharpening_iReduceEdgeSlope
    {0x0F12, 0x0004}, //0004// #AFIT8_Sharpening_iReduceEdgeSlope_Bin , AFIT8_DemSharpMix_iRGBMultiplier
    {0x0F12, 0x0F00}, //0F00// #AFIT8_DemSharpMix_iFilterPower,  AFIT8_DemSharpMix_iBCoeff
    {0x0F12, 0x0518}, //0518// #AFIT8_DemSharpMix_iGCoeff  ,  AFIT8_DemSharpMix_iWideMult
    {0x0F12, 0x0900}, //0900// #AFIT8_DemSharpMix_iNarrMult,  AFIT8_DemSharpMix_iWideFiltReduce
    {0x0F12, 0x0903}, //0903// #AFIT8_DemSharpMix_iWideFiltReduce_Bin , AFIT8_DemSharpMix_iNarrFiltReduce
    {0x0F12, 0x0002}, //0002// #AFIT8_DemSharpMix_iNarrFiltReduce_Bin , AFIT8_DemSharpMix_iHystFalloff
    {0x0F12, 0x0800}, //0800// #AFIT8_DemSharpMix_iHystMinMult,  AFIT8_DemSharpMix_iHystWidth
    {0x0F12, 0x0104}, //0104// #AFIT8_DemSharpMix_iHystFallLow,  AFIT8_DemSharpMix_iHystFallHigh
    {0x0F12, 0x6402}, //6402// #AFIT8_DemSharpMix1_iHystTune,  AFIT8_byr_gras_iShadingPower
    {0x0F12, 0x0080}, //0080// #AFIT8_RGBGamma2_iLinearity  ,  AFIT8_RGBGamma2_iDarkReduce
    {0x0F12, 0x0080}, //0080// #AFIT8_ccm_oscar_iSaturation ,  AFIT8_RGB2YUV_iYOffset
    {0x0F12, 0x0080}, //0080// #AFIT8_RGB2YUV_iRGBGain , AFIT8_RGB2YUV_iSaturation
    {0x0F12, 0x0000}, //0000// #AFIT8_DRx_iBlendingSupress
    {0x0F12, 0x3E1A},
    {0x0F12, 0xFFBF},
    {0x0F12, 0xBE7C},
    {0x0F12, 0xB5BD},
    {0x0F12, 0x3C02},
    {0x0F12, 0x0001},
    {0x002A, 0x193E},
    {0x0F12, 0x0001}, // #DRx_bDRxBypass:DRxfunctionoff
};


unsigned short s5k4eagx_init_reg46[][2] = {
    {0x0028, 0x7000},
    {0x002A, 0x0238},
    {0x0F12, 0x5DC0},	// #REG_TC_IPRM_InClockLSBs  //24MHZ
    //{0x0F12, 0x9c40},	// #REG_TC_IPRM_InClockLSBs  //40MHZ

    {0x0F12, 0x0000},
    {0x002A, 0x0252},
    {0x0F12, 0x0003},	// #REG_TC_IPRM_UseNPviClocks
    {0x0F12, 0x0000},	// #REG_TC_IPRM_UseNMipiClocks
    {0x0F12, 0x0000},	// #REG_TC_IPRM_NumberOfMipiLanes
    {0x002A, 0x025A},
    {0x0F12, 0x2EE0},    // #REG_TC_IPRM_sysClocks_0
    {0x0F12, 0x2ED0},	// #REG_TC_IPRM_MinOutRate4KHz_0
    {0x0F12, 0x2EF0},	// #REG_TC_IPRM_MaxOutRate4KHz_0
    {0x0F12, 0x2EE0},	// #REG_TC_IPRM_sysClocks_1                               4
    {0x0F12, 0x59d8},	// #REG_TC_IPRM4_MinOutRate4KHz_1
    {0x0F12, 0x59d8},	// #REG_TC_IPRM1_MaxOutRate4KHz_1
    {0x0F12, 0x59D8},	// #REG_TC_IPRM_sysClocks_2
    {0x0F12, 0x445C},	// #REG_TC_IPRM_MinOutRate4KHz_2
    {0x0F12, 0x445C},	// #REG_TC_IPRM_MaxOutRate4KHz_2

    {0x002A, 0x026C},
    {0x0F12, 0x0001},	// #REG_TC_IPRM_InitParamsUpdated

    {0x002A, 0x1680},	//Jpeg/////////////
    {0x0F12, 0x018E},	//jpeg_BrcMaxQuality							0x70001690	// 0x017C
    {0x002A, 0x16C6
    },
    {0x0F12, 0x0001},	//jpeg_MinQtblValue
    {0x002A, 0x04BE
    },
    {0x0F12, 0x005D},	//REG_TC_BRC_usCaptureQuality					0x700004C0	// 0x0055

    {0x002A, 0x02E0},	//PREVIEW CONFIGURATION 0 (1280x960, YUV, 15fps)
    {0x0F12, 0x0500},	// #REG_0TC_PCFG_usWidth
    {0x0F12, 0x03C0},	// #REG_0TC_PCFG_usHeight
    {0x0F12, 0x0005},	// #REG_0TC_PCFG_Format
    {0x0F12, 0x2EF0},	// #REG_0TC_PCFG_usMaxOut4KHzRate
    {0x0F12, 0x2ED0},	// #REG_0TC_PCFG_usMinOut4KHzRate
    {0x0F12, 0x0100},	// #REG_0TC_PCFG_OutClkPerPix88
    {0x0F12, 0x0300},	// #REG_0TC_PCFG_uBpp88
    {0x0F12, 0x0042},	// #REG_0TC_PCFG_PVIMask
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_OIFMask
    {0x0F12, 0x01E0},	// #REG_0TC_PCF_usJpegPacketSize
    {0x0F12, 0x0000},	// #REG_0TC_PCF_usJpegTotalPackets
    {0x0F12, 0x0000},	// #REG_0TC_PCF_uClockInd
    {0x0F12, 0x0000},	// #REG_0TC_PCF_usFrTimeType
    {0x0F12, 0x0001},	// #REG_0TC_PCF_FrRateQualityType
    {0x0F12, 0x029A},	// #REG_0TC_PCFG_usMaxFrTimeMsecMult10 //15fps
    {0x0F12, 0x029A},	// #REG_0TC_PCFG_usMinFrTimeMsecMult10 //15 fps
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_sSaturation
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_sSharpBlur
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_sGlamour
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_sColorTemp
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_uDeviceGammaIndex
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_uPrevMirror
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_uCaptureMirror
    {0x0F12, 0x0000},	// #REG_0TC_PCFG_uRotation


    {0x002A, 0x0310},	//PREVIEW CONFIGURATION 1 (2592x1944, YUV, 7.5fps)
    {0x0F12, 0x0A20}, // #REG_0TC_PCFG_usWidth
    {0x0F12, 0x0798}, // #REG_0TC_PCFG_usHeight
    {0x0F12, 0x0005}, // #REG_0TC_PCFG_Format
    {0x0F12, 0x59d8}, // #REG_0TC_PCFG_usMaxOut4KHzRate
    {0x0F12, 0x59d8}, // #REG_0TC_PCFG_usMinOut4KHzRate
    {0x0F12, 0x0100}, // #REG_0TC_PCFG_OutClkPerPix88
    {0x0F12, 0x0300}, // #REG_0TC_PCFG_uBpp88
    {0x0F12, 0x0042}, // #REG_0TC_PCFG_PVIMask
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_OIFMask
    {0x0F12, 0x01E0}, // #REG_0TC_PCF_usJpegPacketSize
    {0x0F12, 0x0000}, // #REG_0TC_PCF_usJpegTotalPackets
    {0x0F12, 0x0001}, // #REG_0TC_PCF_uClockInd
    {0x0F12, 0x0000}, // #REG_0TC_PCF_usFrTimeType
    {0x0F12, 0x0002}, // #REG_0TC_PCF_FrRateQualityType
    {0x0F12, 0x0535}, // #REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps
    {0x0F12, 0x0535}, // #REG_0TC_PCFG_usMinFrTimeMsecMult10 //20 fps
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_sSaturation
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_sSharpBlur
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_sGlamour
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_sColorTemp
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_uDeviceGammaIndex
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_uPrevMirror
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_uCaptureMirror
    {0x0F12, 0x0000}, // #REG_0TC_PCFG_uRotation



    //002A, 0310	//PREVIEW CONFIGURATION 1 (1280x960, YUV, 24fps)
    //0F12, 0500	// #REG_1TC_PCFG_usWidth
    //0F12, 03C0	// #REG_1TC_PCFG_usHeight
    //0F12, 0005	// #REG_1TC_PCFG_Format
    //0F12, 445C	// #REG_1TC_PCFG_usMaxOut4KHzRate
    //0F12, 445C	// #REG_1TC_PCFG_usMinOut4KHzRate
    //0F12, 0100	// #REG_1TC_PCFG_OutClkPerPix88
    //0F12, 0300	// #REG_1TC_PCFG_uBpp88
    //0F12, 0042	// #REG_1TC_PCFG_PVIMask
    //0F12, 0000	// #REG_1TC_PCFG_OIFMask
    //0F12, 01E0	// #REG_1TC_PCF_usJpegPacketSize
    //0F12, 0000	// #REG_1TC_PCF_usJpegTotalPackets
    //0F12, 0001	// #REG_1TC_PCF_uClockInd
    //0F12, 0000	// #REG_1TC_PCF_usFrTimeType
    //0F12, 0001	// #REG_1TC_PCF_FrRateQualityType
    //0F12, 01A0	// #REG_1TC_PCFG_usMaxFrTimeMsecMult10 //24fps
    //0F12, 01A0	// #REG_1TC_PCFG_usMinFrTimeMsecMult10 //24fps
    //0F12, 0000	// #REG_1TC_PCFG_sSaturation
    //0F12, 0000	// #REG_1TC_PCFG_sSharpBlur
    //0F12, 0000	// #REG_1TC_PCFG_sGlamour
    //0F12, 0000	// #REG_1TC_PCFG_sColorTemp
    //0F12, 0000	// #REG_1TC_PCFG_uDeviceGammaIndex
    //0F12, 0000	// #REG_1TC_PCFG_uPrevMirror
    //0F12, 0000	// #REG_1TC_PCFG_uCaptureMirror
    //0F12, 0000	// #REG_1TC_PCFG_uRotation

    {0x002A, 0x0340},	//PREVIEW CONFIGURATION 2 (640x480, YUV, 30fps)
    {0x0F12, 640},//0x0280},	// #REG_2TC_PCFG_usWidth
{0x0F12, 480},//0x01E0},	// #REG_2TC_PCFG_usHeight
{0x0F12, 0x0005},	// #REG_2TC_PCFG_Format
{0x0F12, 0x445C},	// #REG_2TC_PCFG_usMaxOut4KHzRate
{0x0F12, 0x445C},	// #REG_2TC_PCFG_usMinOut4KHzRate
{0x0F12, 0x0100},	// #REG_2TC_PCFG_OutClkPerPix88
{0x0F12, 0x0300},	// #REG_2TC_PCFG_uBpp88
{0x0F12, 0x0042},	// #REG_2TC_PCFG_PVIMask
{0x0F12, 0x0000},	// #REG_2TC_PCFG_OIFMask
{0x0F12, 0x01E0},	// #REG_2TC_PCF_usJpegPacketSize
{0x0F12, 0x0000},	// #REG_2TC_PCF_usJpegTotalPackets
{0x0F12, 0x0002},	// #REG_2TC_PCF_uClockInd
{0x0F12, 0x0000},	// #REG_2TC_PCF_usFrTimeType
{0x0F12, 0x0001},	// #REG_2TC_PCF_FrRateQualityType
{0x0F12, 0x015E},	// #REG_2TC_PCFG_usMaxFrTimeMsecMult10 //30fps
{0x0F12, 0x015E},	// #REG_2TC_PCFG_usMinFrTimeMsecMult10 //30 fps
{0x0F12, 0x0000},	// #REG_2TC_PCFG_sSaturation
{0x0F12, 0x0000},	// #REG_2TC_PCFG_sSharpBlur
{0x0F12, 0x0000},	// #REG_2TC_PCFG_sGlamour
{0x0F12, 0x0000},	// #REG_2TC_PCFG_sColorTemp
{0x0F12, 0x0000},	// #REG_2TC_PCFG_uDeviceGammaIndex
{0x0F12, 0x000f},	// #REG_2TC_PCFG_uPrevMirror
{0x0F12, 0x000f},	// #REG_2TC_PCFG_uCaptureMirror
{0x0F12, 0x0000},	// #REG_2TC_PCFG_uRotation

{0x002A, 0x1782},
{0x0F12, 0x0001},	// #senHal_SenBinShifter
{0x002A, 0x053E},
{0x0F12, 0x0001},	// #REG_HIGH_FPS_UseHighSpeedAng
{0x002A, 0x1792},
{0x0F12, 0x05C9},	// #senHal_uHighSpeedMinColsBin
{0x002A, 0x1796},
{0x0F12, 0x05C9},	// #senHal_uHighSpeedMinColsNoBin
{0x002A, 0x179A},
{0x0F12, 0x0260},	// #senHal_uHighSpeedMinColsAddAnalogBin
{0x002A, 0x06F0},
{0x0F12, 0x0003},	// #skl_usConfigStbySettings //for STBY current
{0x0028, 0xD000},
{0x002A, 0x109C},
{0x0F12, 0x0000},	// For PKG Bayer

{0x0028, 0x7000},
{0x002A, 0x03D0},
{0x0F12, 0x0001},	// #REG_0TC_CCFG_uCaptureMode
{0x0F12, 0x0000},	// #REG_0TC_CCFG_bUseMechShut
{0x0F12, 0x0A20},	// #REG_0TC_CCFG_usWidth
{0x0F12, 0x0798},	// #REG_0TC_CCFG_usHeight
{0x0F12, 0x0005},	// #REG_0TC_CCFG_Format
{0x0F12, 0x59d8},	// #REG_0TC_CCFG_usMaxOut4KHzRate
{0x0F12, 0x59d8},	// #REG_0TC_CCFG_usMinOut4KHzRate
{0x0F12, 0x0100},	// #REG_0TC_CCFG_OutClkPerPix88
{0x0F12, 0x0300},	// #REG_0TC_CCFG_uBpp88
{0x0F12, 0x0042},	// #REG_0TC_CCFG_PVIMask
{0x0F12, 0x0000},	// #REG_0TC_CCFG_OIFMask
{0x0F12, 0x01E0},	// #REG_0TC_CCFG_usJpegPacketSize
{0x0F12, 0x0960},	// #REG_0TC_CCFG_usJpegTotalPackets
{0x0F12, 0x0001},	// #REG_0TC_CCFG_uClockInd
{0x0F12, 0x0000},	// #REG_0TC_CCFG_usFrTimeType
{0x0F12, 0x0002},	// #REG_0TC_CCFG_FrRateQualityType
{0x0F12, 0x0535},	// #REG_0TC_CCFG_usMaxFrTimeMsecMult10  //7.5fps
{0x0F12, 0x0535},	// #REG_0TC_CCFG_usMinFrTimeMsecMult10  //7.5fps
{0x0F12, 0x0000},	// #REG_0TC_CCFG_sSaturation
{0x0F12, 0x0000},	// #REG_0TC_CCFG_sSharpBlur
{0x0F12, 0x0000},	// #REG_0TC_CCFG_sGlamour
{0x0F12, 0x0000},	// #REG_0TC_CCFG_sColorTemp
{0x0F12, 0x0000},	// #REG_0TC_CCFG_uDeviceGammaIndex

{0x002A, 0x02A2},
{0x0F12, 0x0002},	// #REG_TC_GP_ActivePrevConfig
{0x002A, 0x02A6},
{0x0F12, 0x0001},	// #REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x028E},
{0x0F12, 0x0001},	// #REG_TC_GP_NewConfigSync
{0x002A, 0x02A4},
{0x0F12, 0x0001},	// #REG_TC_GP_PrevConfigChanged
{0x002A, 0x027E},
{0x0F12, 0x0001},	// #REG_TC_GP_EnablePreview
{0x0F12, 0x0001},	// #REG_TC_GP_EnablePreviewChanged
{0x002A, 0x02AC},
{0x0F12, 0x0001},	// #REG_TC_GP_CapConfigChanged
{0x002A, 0x04E0},
{0x0F12, 0x0A00},	// #REG_TC_PZOOM_CapZoomReqInputWidth
{0x0F12, 0x0780},	// #REG_TC_PZOOM_CapZoomReqInputHeight
{0x0F12, 0x0010},	// #REG_TC_PZOOM_CapZoomReqInputWidthOfs
{0x0F12, 0x000C},	// #REG_TC_PZOOM_CapZoomReqInputHeightOfs
{0x002A, 0x02A0},
{0x0F12, 0x0001},	// #REG_TC_GP_InputsChangeRequest

    };



// preview preset
unsigned char s5k4eagx_preview_preset_0[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx
    {0x00, 0x2A, 0x02, 0xA2},	{0x0F, 0x12, 0x00, 0x00},	//REG_TC_GP_ActivePrevConfig
    {0x00, 0x2A, 0x02, 0xA6},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevOpenAfterChange
    {0x00, 0x2A, 0x02, 0x8E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_NewConfigSync
    {0x00, 0x2A, 0x02, 0xA4},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevConfigChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_CapConfigChanged
    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01}	//REG_TC_GP_EnablePreviewChanged
};
unsigned char s5k4eagx_preview_preset_1[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx
    {0x00, 0x2A, 0x02, 0xA2},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_ActivePrevConfig
    {0x00, 0x2A, 0x02, 0xA6},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevOpenAfterChange
    {0x00, 0x2A, 0x02, 0x8E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_NewConfigSync
    {0x00, 0x2A, 0x02, 0xA4},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevConfigChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_CapConfigChanged
    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01}	//REG_TC_GP_EnablePreviewChanged
};

unsigned char s5k4eagx_preview_preset_2[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx
    //{0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x00},	//REG_TC_GP_EnablePreview
    //{0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged
    //{REG_DELAY, 500, 0, 0}, // p300

    {0x00, 0x2A, 0x03, 0x40},	{0x0F, 0x12, 0x02, 0x80},// #REG_2TC_PCFG_usWidth //640
    {0x00, 0x2A, 0x03, 0x42},	{0x0F, 0x12, 0x01, 0xe0},// #REG_2TC_PCFG_usHeight //480


    {0x00, 0x2A, 0x02, 0xA2},	{0x0F, 0x12, 0x00, 0x02},	//REG_TC_GP_ActivePrevConfig
    {0x00, 0x2A, 0x02, 0xA6},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevOpenAfterChange
    {0x00, 0x2A, 0x02, 0x8E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_NewConfigSync
    {0x00, 0x2A, 0x02, 0xA4},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevConfigChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_CapConfigChanged

    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged

    {0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x03},	//REG_TC_AF_AfCmd
    {0x00, 0xff, 200,  0}, // p300
    {0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x06},	//5:single AF, 6:continus AF
};

unsigned char s5k4eagx_preview_preset_800x480[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx
    //{0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x00},	//REG_TC_GP_EnablePreview
    //{0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged
    //{REG_DELAY, 500, 0, 0}, // p300

    {0x00, 0x2A, 0x03, 0x40},	{0x0F, 0x12, 0x03, 0x20},// #REG_2TC_PCFG_usWidth //800
    {0x00, 0x2A, 0x03, 0x42},	{0x0F, 0x12, 0x01, 0xe0},// #REG_2TC_PCFG_usHeight //480

    {0x00, 0x2A, 0x02, 0xA2},	{0x0F, 0x12, 0x00, 0x02},	//REG_TC_GP_ActivePrevConfig
    {0x00, 0x2A, 0x02, 0xA6},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevOpenAfterChange
    {0x00, 0x2A, 0x02, 0x8E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_NewConfigSync
    {0x00, 0x2A, 0x02, 0xA4},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevConfigChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_CapConfigChanged

    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged

    {0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x03},	//REG_TC_AF_AfCmd
    {0x00, 0xff, 200,  0}, // p300
    {0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x06},	//5:single AF, 6:continus AF
};


unsigned char s5k4eagx_movie_preset_0[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx
    //{0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x00},	//REG_TC_GP_EnablePreview
    //{0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged
    //{REG_DELAY, 500, 0, 0}, // p300

    {0x00, 0x2A, 0x03, 0x40},	{0x0F, 0x12, 0x02, 0xd0},// #REG_2TC_PCFG_usWidth //720
    {0x00, 0x2A, 0x03, 0x42},	{0x0F, 0x12, 0x01, 0xe0},// #REG_2TC_PCFG_usHeight //480

    {0x00, 0x2A, 0x02, 0xA2},	{0x0F, 0x12, 0x00, 0x02},	//REG_TC_GP_ActivePrevConfig
    {0x00, 0x2A, 0x02, 0xA6},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevOpenAfterChange
    {0x00, 0x2A, 0x02, 0x8E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_NewConfigSync
    {0x00, 0x2A, 0x02, 0xA4},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevConfigChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_CapConfigChanged

    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged

    {0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x03},	//REG_TC_AF_AfCmd
    {0x00, 0xff, 200,  0}, // p300
    {0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x05},	//5:single AF, 6:continus AF
};


unsigned char s5k4eagx_preview_preset_3[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx

    //{0x00, 0x2A, 0x03, 0x9A},	{0x0F, 0x12, 0x00, 0x0f},	//REG_3TC_PCFG_uPrevMirror
    //{0x00, 0x2A, 0x03, 0x9C},	{0x0F, 0x12, 0x00, 0x0f},	//REG_3TC_PCFG_uCaptureMirror

    {0x00, 0x2A, 0x02, 0xA2},	{0x0F, 0x12, 0x00, 0x03},	//REG_TC_GP_ActivePrevConfig
    {0x00, 0x2A, 0x02, 0xA6},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevOpenAfterChange
    {0x00, 0x2A, 0x02, 0x8E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_NewConfigSync
    {0x00, 0x2A, 0x02, 0xA4},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_PrevConfigChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_CapConfigChanged
    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged

    //{0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x03},	//REG_TC_AF_AfCmd
    //{REG_DELAY, 500, 0, 0}, // p300
    //{0x00, 0x2A, 0x02, 0xBC},	{0x0F, 0x12, 0x00, 0x06},	//5:single AF, 6:continus AF
};

// capture preset
unsigned char s5k4eagx_capture_preset_0[][4] = {
    {0x00, 0x2c, 0x70, 0x00}, //offset setting 0x7000 xxxx
    {0x00, 0x2A, 0x02, 0x7E},	{0x0F, 0x12, 0x00, 0x00},	//REG_TC_GP_EnablePreview
    {0x00, 0x2A, 0x02, 0x80},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnablePreviewChanged

    {0x00, 0x2A, 0x02, 0xAA},	{0x0F, 0x12, 0x00, 0x00},	//REG_TC_GP_ActiveCaptureConfig
    {0x00, 0x2A, 0x02, 0x82},	{0x0F, 0x12, 0x00, 0x01},	//REG_TC_GP_EnableCapture
    {0x00, 0x2A, 0x02, 0x84},	{0x0F, 0x12, 0x00, 0x01},	// REG_TC_GP_EnableCaptureChanged
    {0x00, 0x2A, 0x02, 0xAC},	{0x0F, 0x12, 0x00, 0x01},	//Synchronize FW with new capture configuration
};

unsigned char s5k4eagx_capture_preset_1[][4] = {
};
unsigned char s5k4eagx_capture_preset_2[][4] = {
};
unsigned char s5k4eagx_capture_preset_3[][4] = {
};

#define S5K4EAGX_INIT_REGS41	(sizeof(s5k4eagx_init_reg41) / sizeof(s5k4eagx_init_reg41[0]))
#define S5K4EAGX_INIT_REGS42	(sizeof(s5k4eagx_init_reg42) / sizeof(s5k4eagx_init_reg42[0]))
#define S5K4EAGX_INIT_REGS43	(sizeof(s5k4eagx_init_reg43) / sizeof(s5k4eagx_init_reg43[0]))
#define S5K4EAGX_INIT_REGS44	(sizeof(s5k4eagx_init_reg44) / sizeof(s5k4eagx_init_reg44[0]))
#define S5K4EAGX_INIT_REGS45	(sizeof(s5k4eagx_init_reg45) / sizeof(s5k4eagx_init_reg45[0]))
#define S5K4EAGX_INIT_REGS46	(sizeof(s5k4eagx_init_reg46) / sizeof(s5k4eagx_init_reg46[0]))


#define S5K4EAGX_PREVIEW_PRESET_0	(sizeof(s5k4eagx_preview_preset_0) / sizeof(s5k4eagx_preview_preset_0[0]))
#define S5K4EAGX_PREVIEW_PRESET_1	(sizeof(s5k4eagx_preview_preset_1) / sizeof(s5k4eagx_preview_preset_1[0]))
#define S5K4EAGX_PREVIEW_PRESET_2	(sizeof(s5k4eagx_preview_preset_2) / sizeof(s5k4eagx_preview_preset_2[0]))
#define S5K4EAGX_PREVIEW_PRESET_3	(sizeof(s5k4eagx_preview_preset_3) / sizeof(s5k4eagx_preview_preset_3[0]))

#define S5K4EAGX_CAPTURE_PRESET_0	(sizeof(s5k4eagx_capture_preset_0) / sizeof(s5k4eagx_capture_preset_0[0]))
#define S5K4EAGX_CAPTURE_PRESET_1	(sizeof(s5k4eagx_capture_preset_1) / sizeof(s5k4eagx_capture_preset_1[0]))
#define S5K4EAGX_CAPTURE_PRESET_2	(sizeof(s5k4eagx_capture_preset_2) / sizeof(s5k4eagx_capture_preset_2[0]))
#define S5K4EAGX_CAPTURE_PRESET_3	(sizeof(s5k4eagx_capture_preset_3) / sizeof(s5k4eagx_capture_preset_3[0]))
#define S5K4EAGX_PREVIEW_PRESET_800x480	(sizeof(s5k4eagx_preview_preset_800x480) / sizeof(s5k4eagx_preview_preset_800x480[0]))

#define S5K4EAGX_MOVIE_PRESET_0	(sizeof(s5k4eagx_movie_preset_0) / sizeof(s5k4eagx_movie_preset_0[0]))

#endif
#endif
