/* drivers/media/video/thp7212.c
 *
 * Driver for thp7212 (5MP Camera) from SEC
 *
 * Copyright (C) 2010, SAMSUNG ELECTRONICS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/slab.h> //many add
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
///#include <media/v4l2-i2c-drv.h> //many del
#include "thp7212-nx.h"
#include <linux/videodev2_exynos_camera.h>

#define CONFIG_VIDEO_THP7212_V_1_1

/* psw0523 add for test patron preset table */
#define USE_PRESET
#ifdef USE_PRESET
#include "thp7212-nx-preset.h"
#endif

/* #define USE_INITIAL_WORKER_THREAD */

#ifdef USE_INITIAL_WORKER_THREAD
#include <linux/workqueue.h>
#endif


#define DEFAULT_SENSOR_WIDTH		640
#define DEFAULT_SENSOR_HEIGHT		480
#define DEFAULT_SENSOR_CODE		(V4L2_MBUS_FMT_YUYV8_2X8)

#include "thp7212_regs_1_1.h"

#define FORMAT_FLAGS_COMPRESSED		0x3
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x410580

#define DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
/* #define DEFAULT_MCLK		24000000 */
#define DEFAULT_MCLK		27000000
#define POLL_TIME_MS		10
#define CAPTURE_POLL_TIME_MS    1000

#define THINE_I2C_RETRY_CNT	3


/* maximum time for one frame at minimum fps (15fps) in normal mode */
#define NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS     67
/* maximum time for one frame at minimum fps (4fps) in night mode */
#define NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS     250

/* time to move lens to target position before last af mode register write */
#define LENS_MOVE_TIME_MS       100

/* level at or below which we need to enable flash when in auto mode */
#define LOW_LIGHT_LEVEL		0x1D

/* level at or below which we need to use low light capture mode */
#define HIGH_LIGHT_LEVEL	0x80

#define FIRST_AF_SEARCH_COUNT   80
#define SECOND_AF_SEARCH_COUNT  80
#define AE_STABLE_SEARCH_COUNT  4

#define FIRST_SETTING_FOCUS_MODE_DELAY_MS	100
#define SECOND_SETTING_FOCUS_MODE_DELAY_MS	200


#ifdef CONFIG_VIDEO_THP7212_DEBUG
enum {
	THP7212_DEBUG_I2C		= 1U << 0,
	THP7212_DEBUG_I2C_BURSTS	= 1U << 1,
};
static uint32_t thp7212_debug_mask = THP7212_DEBUG_I2C_BURSTS;
module_param_named(debug_mask, thp7212_debug_mask, uint, S_IWUSR | S_IRUGO);

#define thp7212_debug(mask, x...) \
	do { \
		if (thp7212_debug_mask & mask) \
			pr_info(x);	\
	} while (0)
#else

#define thp7212_debug(mask, x...)

#endif

#define THP7212_VERSION_1_1	0x11

enum thp7212_hw_power {
	THP7212_HW_POWER_OFF,
	THP7212_HW_POWER_ON,
};

/* result values returned to HAL */
enum {
	AUTO_FOCUS_FAILED,
	AUTO_FOCUS_DONE,
	AUTO_FOCUS_CANCELLED,
};

enum af_operation_status {
	AF_NONE = 0,
	AF_START,
	AF_CANCEL,
};

enum thp7212_oprmode {
	THP7212_OPRMODE_VIDEO = 0,
	THP7212_OPRMODE_IMAGE = 1,
	THP7212_OPRMODE_MAX,
};

struct thp7212_resolution {
	u8			value;
	enum thp7212_oprmode	type;
	u16			width;
	u16			height;
};

/* M5MOLS default format (codes, sizes, preset values) */
static struct v4l2_mbus_framefmt default_fmt[THP7212_OPRMODE_MAX] = {
	[THP7212_OPRMODE_VIDEO] = {
		.width		= DEFAULT_SENSOR_WIDTH,
		.height		= DEFAULT_SENSOR_HEIGHT,
		.code		= DEFAULT_SENSOR_CODE,
		.field		= V4L2_FIELD_NONE,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	},
	[THP7212_OPRMODE_IMAGE] = {
		.width		= 1920,
		.height		= 1080,
		.code		= V4L2_MBUS_FMT_JPEG_1X8,
		.field		= V4L2_FIELD_NONE,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	},
};

#define SIZE_DEFAULT_FFMT	ARRAY_SIZE(default_fmt)
enum thp7212_preview_frame_size {
	THP7212_PREVIEW_QCIF = 0,	/* 176x144 */
	THP7212_PREVIEW_CIF,		/* 352x288 */
	THP7212_PREVIEW_QVGA,		/* 320x240 */
	THP7212_PREVIEW_VGA,		/* 640x480 */
	THP7212_PREVIEW_D1,		/* 720x480 */
	THP7212_PREVIEW_WVGA,		/* 800x480 */
	THP7212_PREVIEW_SVGA,		/* 800x600 */
	THP7212_PREVIEW_880,		/* 880x720 */
	THP7212_PREVIEW_WSVGA,		/* 1024x600*/
	THP7212_PREVIEW_1024,		/* 1024x768*/
	THP7212_PREVIEW_W1280,		/* 1280x720*/
	THP7212_PREVIEW_1280,		/* 1280x960*/
	THP7212_PREVIEW_1600,		/* 1600x1200*/
	THP7212_PREVIEW_1920,		/* 1920x1080*/
	THP7212_PREVIEW_2048,		/* 2048x1536*/
	THP7212_PREVIEW_2560,		/* 2560x1920*/
	THP7212_PREVIEW_MAX,
};

enum thp7212_capture_frame_size {
	THP7212_CAPTURE_QVGA = 0,	/* 640x480 */
	THP7212_CAPTURE_VGA,		/* 320x240 */
	THP7212_CAPTURE_WVGA,		/* 800x480 */
	THP7212_CAPTURE_SVGA,		/* 800x600 */
	THP7212_CAPTURE_WSVGA,		/* 1024x600 */
	THP7212_CAPTURE_XGA,		/* 1024x768 */
	THP7212_CAPTURE_1MP,		/* 1280x960 */
	THP7212_CAPTURE_W1MP,		/* 1600x960 */
	THP7212_CAPTURE_2MP,		/* UXGA  - 1600x1200 */
	THP7212_CAPTURE_W2MP,		/* 35mm Academy Offset Standard 1.66 */
					/* 2048x1232, 2.4MP */
	THP7212_CAPTURE_3MP,		/* QXGA  - 2048x1536 */
	THP7212_CAPTURE_W4MP,		/* WQXGA - 2560x1536 */
	THP7212_CAPTURE_5MP,		/* 2560x1920 */
	THP7212_CAPTURE_MAX,
};

/* make look-up table */
static const struct thp7212_resolution thp7212_resolutions[] = {
	/* monitor size */
	/* {THP7212_PREVIEW_QCIF 	, THP7212_OPRMODE_VIDEO, 176,  144  }, */
	/* {THP7212_PREVIEW_CIF 	, THP7212_OPRMODE_VIDEO, 352,  288  }, */
	/* {THP7212_PREVIEW_QVGA 	, THP7212_OPRMODE_VIDEO, 320,  240  }, */
	{THP7212_PREVIEW_VGA 	, THP7212_OPRMODE_VIDEO, 640,  480  },
	/* {THP7212_PREVIEW_D1  	, THP7212_OPRMODE_VIDEO, 720,  480  }, */
	/* {THP7212_PREVIEW_WVGA	, THP7212_OPRMODE_VIDEO, 800,  480  }, */
	/* {THP7212_PREVIEW_SVGA	, THP7212_OPRMODE_VIDEO, 800,  600  }, */
	/* {THP7212_PREVIEW_880	, THP7212_OPRMODE_VIDEO, 880,  720  }, */
	/* {THP7212_PREVIEW_WSVGA	, THP7212_OPRMODE_VIDEO, 1024, 600  }, */
	{THP7212_PREVIEW_1024	, THP7212_OPRMODE_VIDEO, 1024, 768  },
	{THP7212_PREVIEW_W1280	, THP7212_OPRMODE_VIDEO, 1280, 720  },
	{THP7212_PREVIEW_1280	, THP7212_OPRMODE_VIDEO, 1280, 960  },
	/* {THP7212_PREVIEW_1600	, THP7212_OPRMODE_VIDEO, 1600, 1200 }, */
	{THP7212_PREVIEW_1920	, THP7212_OPRMODE_VIDEO, 1920, 1080 },
	{THP7212_PREVIEW_2048	, THP7212_OPRMODE_VIDEO, 2048, 1536 },
	{THP7212_PREVIEW_2560	, THP7212_OPRMODE_VIDEO, 2560, 1920 },

	/* capture(JPEG or Bayer RAW or YUV Raw) size */
	{ THP7212_CAPTURE_VGA  , THP7212_OPRMODE_IMAGE, 640, 480   },
	{ THP7212_CAPTURE_WVGA , THP7212_OPRMODE_IMAGE, 800, 480   },
	{ THP7212_CAPTURE_SVGA , THP7212_OPRMODE_IMAGE, 800, 600   },
	{ THP7212_CAPTURE_WSVGA, THP7212_OPRMODE_IMAGE, 1024, 600  },
	{ THP7212_CAPTURE_XGA  , THP7212_OPRMODE_IMAGE, 1024, 768  },
	{ THP7212_CAPTURE_1MP  , THP7212_OPRMODE_IMAGE, 1280, 960  },
	{ THP7212_CAPTURE_W1MP , THP7212_OPRMODE_IMAGE, 1600, 960  },
	{ THP7212_CAPTURE_2MP  , THP7212_OPRMODE_IMAGE, 1600, 1200 },
	{ THP7212_CAPTURE_W2MP , THP7212_OPRMODE_IMAGE, 1920, 1080 },
	{ THP7212_CAPTURE_3MP  , THP7212_OPRMODE_IMAGE, 2048, 1232 },
	{ THP7212_CAPTURE_W4MP , THP7212_OPRMODE_IMAGE, 2048, 1536 },
	{ THP7212_CAPTURE_5MP  , THP7212_OPRMODE_IMAGE, 2560, 1920 },
	{ THP7212_CAPTURE_MAX  , THP7212_OPRMODE_IMAGE, 2560, 1440 },
};

struct thp7212_framesize {
	u32 index;
	u32 width;
	u32 height;
};

static const struct thp7212_framesize thp7212_preview_framesize_list[] = {
	{ THP7212_PREVIEW_VGA,		640,   480  },
	{ THP7212_PREVIEW_1024,	1024,  768  },
	{ THP7212_PREVIEW_W1280,	1280,  720  },
	{ THP7212_PREVIEW_1280,	1280,  960  },
	{ THP7212_PREVIEW_1920,	1920,  1080 },
	{ THP7212_PREVIEW_1600,	1600,  1200 },
	{ THP7212_PREVIEW_2048,	2048,  1536 },
	{ THP7212_PREVIEW_2560,	2560,  1920 },
};

static const struct thp7212_framesize thp7212_capture_framesize_list[] = {
	{ THP7212_CAPTURE_VGA,		640,  480  },
	{ THP7212_CAPTURE_XGA,		1024, 768  },
	{ THP7212_CAPTURE_1MP,		1280, 960  },
	{ THP7212_CAPTURE_2MP,		1600, 1200 },
	{ THP7212_CAPTURE_3MP,		2048, 1536 },
	{ THP7212_CAPTURE_5MP,		2560, 1920 },
};

struct thp7212_version {
	u32 major;
	u32 minor;
};

struct thp7212_date_info {
	u32 year;
	u32 month;
	u32 date;
};

enum thp7212_runmode {
	THP7212_RUNMODE_NOTREADY,
	THP7212_RUNMODE_IDLE,
	THP7212_RUNMODE_RUNNING,
	THP7212_RUNMODE_CAPTURE,
};

struct thp7212_firmware {
	u32 addr;
	u32 size;
};

struct thp7212_jpeg_param {
	u32 enable;
	u32 quality;
	u32 main_size;		/* Main JPEG file size */
	u32 thumb_size;		/* Thumbnail file size */
	u32 main_offset;
	u32 thumb_offset;
	u32 postview_offset;
};

struct thp7212_position {
	int x;
	int y;
};

struct gps_info_common {
	u32 direction;
	u32 dgree;
	u32 minute;
	u32 second;
};

struct thp7212_gps_info {
	unsigned char gps_buf[8];
	unsigned char altitude_buf[4];
	int gps_timeStamp;
};

struct thp7212_regset {
	u32 size;
	u8 *data;
};

struct thp7212_regset_table {
	const u32	*reg;
	int		array_size;
};

#define THP7212_REGSET(x, y)		\
	[(x)] = {					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}

#define THP7212_REGSET_TABLE(y)		\
	{					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}

struct thp7212_regs {
	struct thp7212_regset_table ev[EV_MAX];
	struct thp7212_regset_table metering[METERING_MAX];
	struct thp7212_regset_table iso[ISO_MAX];
	struct thp7212_regset_table effect[V4L2_IMAGE_EFFECT_MAX];
	struct thp7212_regset_table white_balance[V4L2_WHITE_BALANCE_MAX];
	struct thp7212_regset_table preview_size[THP7212_PREVIEW_MAX];
	struct thp7212_regset_table capture_size[THP7212_CAPTURE_MAX];
	struct thp7212_regset_table scene_mode[V4L2_SCENE_MODE_MAX];
	struct thp7212_regset_table saturation[V4L2_SATURATION_MAX];
	struct thp7212_regset_table contrast[V4L2_CONTRAST_MAX];
	struct thp7212_regset_table sharpness[V4L2_SHARPNESS_MAX];
	struct thp7212_regset_table fps[FRAME_RATE_MAX];
	struct thp7212_regset_table preview_return;
	struct thp7212_regset_table jpeg_quality_high;
	struct thp7212_regset_table jpeg_quality_normal;
	struct thp7212_regset_table jpeg_quality_low;
	struct thp7212_regset_table flash_start;
	struct thp7212_regset_table flash_end;
	struct thp7212_regset_table af_assist_flash_start;
	struct thp7212_regset_table af_assist_flash_end;
	struct thp7212_regset_table af_low_light_mode_on;
	struct thp7212_regset_table af_low_light_mode_off;
	struct thp7212_regset_table aeawb_lockunlock[V4L2_AE_AWB_MAX];
	//struct thp7212_regset_table ae_awb_lock_on;
	//struct thp7212_regset_table ae_awb_lock_off;
	struct thp7212_regset_table low_cap_on;
	struct thp7212_regset_table low_cap_off;
	struct thp7212_regset_table wdr_on;
	struct thp7212_regset_table wdr_off;
	struct thp7212_regset_table face_detection_on;
	struct thp7212_regset_table face_detection_off;
	struct thp7212_regset_table capture_start;
	struct thp7212_regset_table af_macro_mode_1;
	struct thp7212_regset_table af_macro_mode_2;
	struct thp7212_regset_table af_macro_mode_3;
	struct thp7212_regset_table af_normal_mode_1;
	struct thp7212_regset_table af_normal_mode_2;
	struct thp7212_regset_table af_normal_mode_3;
	struct thp7212_regset_table af_return_macro_position;
	struct thp7212_regset_table single_af_start;
	struct thp7212_regset_table single_af_off_1;
	struct thp7212_regset_table single_af_off_2;
	struct thp7212_regset_table continuous_af_on;
	struct thp7212_regset_table continuous_af_off;
	struct thp7212_regset_table dtp_start;
	struct thp7212_regset_table dtp_stop;
	struct thp7212_regset_table init_reg_1;
	struct thp7212_regset_table init_reg_2;
	struct thp7212_regset_table init_reg_3; //many add
	struct thp7212_regset_table init_reg_4; //many add
	struct thp7212_regset_table flash_init;
	struct thp7212_regset_table reset_crop;
	struct thp7212_regset_table get_ae_stable_status;
	struct thp7212_regset_table get_light_level;
	struct thp7212_regset_table get_1st_af_search_status;
	struct thp7212_regset_table get_2nd_af_search_status;
	struct thp7212_regset_table get_capture_status;
	struct thp7212_regset_table get_esd_status;
	struct thp7212_regset_table get_iso;
	struct thp7212_regset_table get_shutterspeed;
	struct thp7212_regset_table get_frame_count;
};

#ifdef CONFIG_VIDEO_THP7212_V_1_1
static const struct thp7212_regs regs_for_fw_version_1_1 = {

	.init_reg_1 = THP7212_REGSET_TABLE(thp7212_init_reg1),
	.init_reg_2 = THP7212_REGSET_TABLE(thp7212_init_reg2),
	.init_reg_3 = THP7212_REGSET_TABLE(thp7212_init_reg3),
	.init_reg_4 = THP7212_REGSET_TABLE(thp7212_init_reg4), //many add

	.dtp_start = THP7212_REGSET_TABLE(thp7212_DTP_init),
	.dtp_stop = THP7212_REGSET_TABLE(thp7212_DTP_stop),

#if 1
	.capture_start = THP7212_REGSET_TABLE(thp7212_Capture_Start),
	.get_light_level = THP7212_REGSET_TABLE(thp7212_Get_Light_Level),

	.preview_size = {
			THP7212_REGSET(THP7212_PREVIEW_VGA, thp7212_640_Preview),
			THP7212_REGSET(THP7212_PREVIEW_1024, thp7212_1024_Preview),
			THP7212_REGSET(THP7212_PREVIEW_W1280, thp7212_W1280_Preview),
			THP7212_REGSET(THP7212_PREVIEW_1280, thp7212_1280_Preview),
			THP7212_REGSET(THP7212_PREVIEW_1600, thp7212_1600_Preview),
			THP7212_REGSET(THP7212_PREVIEW_1920, thp7212_1920_Preview),
			THP7212_REGSET(THP7212_PREVIEW_2048, thp7212_2048_Preview),
			THP7212_REGSET(THP7212_PREVIEW_2560, thp7212_max_Preview),
	},

	.capture_size = {
			/*THP7212_REGSET(THP7212_CAPTURE_QVGA, thp7212_QVGA_Capture),*/
			THP7212_REGSET(THP7212_CAPTURE_VGA, thp7212_VGA_Capture),
			THP7212_REGSET(THP7212_CAPTURE_XGA, thp7212_XGA_Capture),
			THP7212_REGSET(THP7212_CAPTURE_1MP, thp7212_1M_Capture),
			THP7212_REGSET(THP7212_CAPTURE_2MP, thp7212_2M_Capture),
			THP7212_REGSET(THP7212_CAPTURE_3MP, thp7212_3M_Capture),
			THP7212_REGSET(THP7212_CAPTURE_5MP, thp7212_5M_Capture),
	},

	.reset_crop = THP7212_REGSET_TABLE(thp7212_Reset_Crop),
	.get_capture_status =
		THP7212_REGSET_TABLE(thp7212_get_capture_status),
#endif
#if 1//many del //zhuxuezhen 2012-12-27 recovery
	.ev = {
		//THP7212_REGSET(EV_MINUS_4, thp7212_EV_Minus_4),
		//THP7212_REGSET(EV_MINUS_3, thp7212_EV_Minus_3),
		//THP7212_REGSET(EV_MINUS_2, thp7212_EV_Minus_2),
		//THP7212_REGSET(EV_MINUS_1, thp7212_EV_Minus_1),
		THP7212_REGSET(EV_DEFAULT, thp7212_EV_Default),
		THP7212_REGSET(EV_PLUS_1, thp7212_EV_Plus_1),
		THP7212_REGSET(EV_PLUS_2, thp7212_EV_Plus_2),
		THP7212_REGSET(EV_PLUS_3, thp7212_EV_Plus_3),
		THP7212_REGSET(EV_PLUS_4, thp7212_EV_Plus_4),
	},
	.metering = {
		THP7212_REGSET(V4L2_METERING_MATRIX, thp7212_Metering_Matrix),
		THP7212_REGSET(V4L2_METERING_CENTER, thp7212_Metering_Center),
		THP7212_REGSET(V4L2_METERING_SPOT, thp7212_Metering_Spot),
	},
	.iso = {
		THP7212_REGSET(ISO_AUTO, thp7212_ISO_Auto),
		THP7212_REGSET(ISO_50, thp7212_ISO_100),     /* map to 100 */
		THP7212_REGSET(ISO_100, thp7212_ISO_100),
		THP7212_REGSET(ISO_200, thp7212_ISO_200),
		THP7212_REGSET(ISO_400, thp7212_ISO_400),
		THP7212_REGSET(ISO_800, thp7212_ISO_400),    /* map to 400 */
		THP7212_REGSET(ISO_1600, thp7212_ISO_400),   /* map to 400 */
		THP7212_REGSET(ISO_SPORTS, thp7212_ISO_Auto),/* map to auto */
		THP7212_REGSET(ISO_NIGHT, thp7212_ISO_Auto), /* map to auto */
		THP7212_REGSET(ISO_MOVIE, thp7212_ISO_Auto), /* map to auto */
	},
	.effect = {
		THP7212_REGSET(V4L2_IMAGE_EFFECT_NORMAL, thp7212_Effect_Normal),
		THP7212_REGSET(V4L2_IMAGE_EFFECT_MONOCHROME, thp7212_Effect_Black_White),
		THP7212_REGSET(V4L2_IMAGE_EFFECT_SEPIA, thp7212_Effect_Sepia),
		THP7212_REGSET(V4L2_IMAGE_EFFECT_NEGATIVE_COLOR,
				thp7212_Effect_Negative),
		THP7212_REGSET(V4L2_IMAGE_EFFECT_SOLARIZE, thp7212_Effect_Solarization),
	},
	.white_balance = {
		THP7212_REGSET(V4L2_WHITE_BALANCE_AUTO, thp7212_WB_Auto),
		THP7212_REGSET(V4L2_WHITE_BALANCE_SUNNY, thp7212_WB_Sunny),
		THP7212_REGSET(V4L2_WHITE_BALANCE_CLOUDY, thp7212_WB_Cloudy),
		THP7212_REGSET(V4L2_WHITE_BALANCE_TUNGSTEN, thp7212_WB_Tungsten),
		THP7212_REGSET(V4L2_WHITE_BALANCE_FLUORESCENT,
				thp7212_WB_Fluorescent),
	},
	.scene_mode = {
		THP7212_REGSET(V4L2_SCENE_MODE_NONE, thp7212_Scene_Default),
		THP7212_REGSET(V4L2_SCENE_MODE_PORTRAIT, thp7212_Scene_Portrait),
		THP7212_REGSET(V4L2_SCENE_MODE_NIGHTSHOT, thp7212_Scene_Nightshot),
		THP7212_REGSET(V4L2_SCENE_MODE_LANDSCAPE, thp7212_Scene_Landscape),
		THP7212_REGSET(V4L2_SCENE_MODE_SPORTS, thp7212_Scene_Sports),
		THP7212_REGSET(V4L2_SCENE_MODE_PARTY_INDOOR,
				thp7212_Scene_Party_Indoor),
		THP7212_REGSET(V4L2_SCENE_MODE_BEACH_SNOW,
				thp7212_Scene_Beach_Snow),
		THP7212_REGSET(V4L2_SCENE_MODE_SUNSET, thp7212_Scene_Sunset),
		THP7212_REGSET(V4L2_SCENE_MODE_FIREWORKS, thp7212_Scene_Fireworks),
		THP7212_REGSET(V4L2_SCENE_MODE_CANDLE_LIGHT,
				thp7212_Scene_Candle_Light),
	},
	.saturation = {
		THP7212_REGSET(V4L2_SATURATION_MINUS_2,
				thp7212_Saturation_Minus_2),
		THP7212_REGSET(V4L2_SATURATION_MINUS_1,
				thp7212_Saturation_Minus_1),
		THP7212_REGSET(V4L2_SATURATION_DEFAULT,
				thp7212_Saturation_Default),
		THP7212_REGSET(V4L2_SATURATION_PLUS_1, thp7212_Saturation_Plus_1),
		THP7212_REGSET(V4L2_SATURATION_PLUS_2, thp7212_Saturation_Plus_2),
	},
	.contrast = {
		THP7212_REGSET(V4L2_CONTRAST_MINUS_2, thp7212_Contrast_Minus_2),
		THP7212_REGSET(V4L2_CONTRAST_MINUS_1, thp7212_Contrast_Minus_1),
		THP7212_REGSET(V4L2_CONTRAST_DEFAULT, thp7212_Contrast_Default),
		THP7212_REGSET(V4L2_CONTRAST_PLUS_1, thp7212_Contrast_Plus_1),
		THP7212_REGSET(V4L2_CONTRAST_PLUS_2, thp7212_Contrast_Plus_2),
	},
	.sharpness = {
		THP7212_REGSET(V4L2_SHARPNESS_MINUS_2, thp7212_Sharpness_Minus_2),
		THP7212_REGSET(V4L2_SHARPNESS_MINUS_1, thp7212_Sharpness_Minus_1),
		THP7212_REGSET(V4L2_SHARPNESS_DEFAULT, thp7212_Sharpness_Default),
		THP7212_REGSET(V4L2_SHARPNESS_PLUS_1, thp7212_Sharpness_Plus_1),
		THP7212_REGSET(V4L2_SHARPNESS_PLUS_2, thp7212_Sharpness_Plus_2),
	},
	.fps = {
		THP7212_REGSET(V4L2_FRAME_RATE_AUTO, thp7212_FPS_Auto),
		THP7212_REGSET(V4L2_FRAME_RATE_7, thp7212_FPS_7),
		THP7212_REGSET(V4L2_FRAME_RATE_15, thp7212_FPS_15),
		THP7212_REGSET(V4L2_FRAME_RATE_30, thp7212_FPS_30),
	},
	.preview_return = THP7212_REGSET_TABLE(thp7212_Preview_Return),
	.jpeg_quality_high = THP7212_REGSET_TABLE(thp7212_Jpeg_Quality_High),
	.jpeg_quality_normal =
		THP7212_REGSET_TABLE(thp7212_Jpeg_Quality_Normal),
	.jpeg_quality_low = THP7212_REGSET_TABLE(thp7212_Jpeg_Quality_Low),
	.flash_start = THP7212_REGSET_TABLE(thp7212_Flash_Start),
	.flash_end = THP7212_REGSET_TABLE(thp7212_Flash_End),
	.af_assist_flash_start =
		THP7212_REGSET_TABLE(thp7212_Pre_Flash_Start),
	.af_assist_flash_end =
		THP7212_REGSET_TABLE(thp7212_Pre_Flash_End),
	.af_low_light_mode_on =
		THP7212_REGSET_TABLE(thp7212_AF_Low_Light_Mode_On),
	.af_low_light_mode_off =
		THP7212_REGSET_TABLE(thp7212_AF_Low_Light_Mode_Off),
	.aeawb_lockunlock = {
		THP7212_REGSET(V4L2_AE_UNLOCK_AWB_UNLOCK, thp7212_AE_AWB_Lock_Off),
		THP7212_REGSET(V4L2_AE_LOCK_AWB_UNLOCK, thp7212_AE_Lock_On_AWB_Lock_Off),
		THP7212_REGSET(V4L2_AE_UNLOCK_AWB_LOCK, thp7212_AE_Lock_Off_AWB_Lock_On),
		THP7212_REGSET(V4L2_AE_LOCK_AWB_LOCK, thp7212_AE_AWB_Lock_On),
	},
		/*
	.ae_awb_lock_on =
		THP7212_REGSET_TABLE(thp7212_AE_AWB_Lock_On),
	.ae_awb_lock_off =
		THP7212_REGSET_TABLE(thp7212_AE_AWB_Lock_Off),
		*/
	.low_cap_on = THP7212_REGSET_TABLE(thp7212_Low_Cap_On),
	.low_cap_off = THP7212_REGSET_TABLE(thp7212_Low_Cap_Off),
	.wdr_on = THP7212_REGSET_TABLE(thp7212_WDR_on),
	.wdr_off = THP7212_REGSET_TABLE(thp7212_WDR_off),
	.face_detection_on = THP7212_REGSET_TABLE(thp7212_Face_Detection_On),
	.face_detection_off =
		THP7212_REGSET_TABLE(thp7212_Face_Detection_Off),
	.capture_start = THP7212_REGSET_TABLE(thp7212_Capture_Start),
	.af_macro_mode_1 = THP7212_REGSET_TABLE(thp7212_AF_Macro_mode_1),
	.af_macro_mode_2 = THP7212_REGSET_TABLE(thp7212_AF_Macro_mode_2),
	.af_macro_mode_3 = THP7212_REGSET_TABLE(thp7212_AF_Macro_mode_3),
	.af_normal_mode_1 = THP7212_REGSET_TABLE(thp7212_AF_Normal_mode_1),
	.af_normal_mode_2 = THP7212_REGSET_TABLE(thp7212_AF_Normal_mode_2),
	.af_normal_mode_3 = THP7212_REGSET_TABLE(thp7212_AF_Normal_mode_3),
	.af_return_macro_position =
		THP7212_REGSET_TABLE(thp7212_AF_Return_Macro_pos),
	.single_af_start = THP7212_REGSET_TABLE(thp7212_Single_AF_Start),
	.single_af_off_1 = THP7212_REGSET_TABLE(thp7212_Single_AF_Off_1),
	.single_af_off_2 = THP7212_REGSET_TABLE(thp7212_Single_AF_Off_2),
	.continuous_af_on = THP7212_REGSET_TABLE(thp7212_Continuous_AF_On),
	.continuous_af_off = THP7212_REGSET_TABLE(thp7212_Continuous_AF_Off),
	.dtp_start = THP7212_REGSET_TABLE(thp7212_DTP_init),
	.dtp_stop = THP7212_REGSET_TABLE(thp7212_DTP_stop),
///	.init_reg_1 = THP7212_REGSET_TABLE(thp7212_init_reg1),
///	.init_reg_2 = THP7212_REGSET_TABLE(thp7212_init_reg2),
///	.init_reg_3 = THP7212_REGSET_TABLE(thp7212_init_reg3), //many add
	.flash_init = THP7212_REGSET_TABLE(thp7212_Flash_init),
	.reset_crop = THP7212_REGSET_TABLE(thp7212_Reset_Crop),
	.get_ae_stable_status =
		THP7212_REGSET_TABLE(thp7212_Get_AE_Stable_Status),
	.get_light_level = THP7212_REGSET_TABLE(thp7212_Get_Light_Level),
	.get_1st_af_search_status =
		THP7212_REGSET_TABLE(thp7212_get_1st_af_search_status),
	.get_2nd_af_search_status =
		THP7212_REGSET_TABLE(thp7212_get_2nd_af_search_status),
	.get_capture_status =
		THP7212_REGSET_TABLE(thp7212_get_capture_status),
	.get_esd_status = THP7212_REGSET_TABLE(thp7212_get_esd_status),
	.get_iso = THP7212_REGSET_TABLE(thp7212_get_iso_reg),
	.get_shutterspeed =
		THP7212_REGSET_TABLE(thp7212_get_shutterspeed_reg),
	.get_frame_count =
		THP7212_REGSET_TABLE(thp7212_get_frame_count_reg),
#endif

};
#endif
struct thp7212_state {
	struct thp7212_platform_data 	*pdata;
	struct media_pad	 	pad; /* for media deivce pad */
	struct v4l2_subdev 		sd;
	struct exynos_md		*mdev; /* for media deivce entity */
	struct v4l2_pix_format		pix;
	struct v4l2_mbus_framefmt	ffmt[2]; /* for media deivce fmt */
	struct v4l2_fract		timeperframe;
	struct thp7212_jpeg_param	jpeg;
	struct thp7212_version		fw;
	struct thp7212_version		prm;
	struct thp7212_date_info	dateinfo;
	struct thp7212_position	position;
	struct v4l2_streamparm		strm;
	struct v4l2_streamparm		stored_parm;
	struct thp7212_gps_info	gps_info;
	struct mutex			ctrl_lock;
	struct completion		af_complete;
	enum thp7212_runmode		runmode;
	enum thp7212_oprmode		oprmode;
	enum af_operation_status	af_status;
	enum v4l2_mbus_pixelcode	code; /* for media deivce code */
	int 				res_type;
	u8 				resolution;
	int				preview_framesize_index;
	int				capture_framesize_index;
	int				sensor_version;
	int				freq;		/* MCLK in Hz */
	int				check_dataline;
	int				check_previewdata;
	bool 				flash_on;
	bool 				torch_on;
	bool 				power_on;
	bool 				sensor_af_in_low_light_mode;
	bool 				flash_state_on_previous_capture;
	bool 				initialized;
	bool 				restore_preview_size_needed;
	int 				one_frame_delay_ms;
	const struct 			thp7212_regs *regs;
#ifdef USE_INITIAL_WORKER_THREAD
    struct workqueue_struct *init_wq;
    struct work_struct init_work;
#endif
};

static const struct v4l2_fmtdesc capture_fmts[] = {
	{
		.index		= 0,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_COMPRESSED,
		.description	= "JPEG + Postview",
		.pixelformat	= V4L2_PIX_FMT_JPEG,
	},
};


//added by keun 2015.05.07
static bool camera_initialized = false;


static int thp7212_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt);


int thp7212_i2c_read_data(struct i2c_client *client, unsigned int _addr, unsigned char *_data, unsigned int _size)
{
	struct i2c_adapter *adap = client->adapter;
	int	cnt;
	unsigned char _reg[2]={0};

	struct i2c_msg msgs[] ={
	{
		.addr = client->addr,
		.flags = 0,
		.len = 2,
		.buf = _reg,
	},
	{
		.addr = client->addr,
		.flags = I2C_M_RD,
		.len = _size,
		.buf = _data,
	}};

	_reg[0] = (unsigned char)(_addr>>8);
	_reg[1] = (unsigned char)_addr;

	for (cnt = 0; cnt < 10; cnt++) 
	{
		if (i2c_transfer(adap, msgs, 2) == 2)
			break;

		mdelay(10);
	}

	if (cnt == 10) 
	{
		printk(KERN_ERR "soc_i2c_read retry\n");
		return -1;
	}

	return	0;
}

static int thp7212_i2c_read_byte(struct i2c_client *client, u8 addr, u8 *data)
{
	s8 i = 0;
	s8 ret = 0;
	u8 buf = 0;
	struct i2c_msg msg[2];

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &buf;

	for(i=0; i<THINE_I2C_RETRY_CNT; i++)
	{
		ret = i2c_transfer(client->adapter, msg, 2);
		if (likely(ret == 2))
			break;
		mdelay(POLL_TIME_MS);
		//dev_err(&client->dev, "\e[31mthp7212_i2c_write_byte failed reg:0x%02x retry:%d\e[0m\n", addr, i);
	}

	if (unlikely(ret != 2)) 
	{
		dev_err(&client->dev, "\e[31mthp7212_i2c_read_byte failed reg:0x%02x \e[0m\n", addr);
		return -EIO;
	}

	*data = buf;
	return 0;
}

static int thp7212_i2c_write_byte(struct i2c_client *client, u8 addr, u8 val)
{
	s8 i = 0;
	s8 ret = 0;
	u8 buf[2];
	u8 read_val = 0;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = buf;

	buf[0] = addr;
	buf[1] = val ;

	for(i=0; i<THINE_I2C_RETRY_CNT; i++)
	{
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		mdelay(POLL_TIME_MS);
		//dev_err(&client->dev, "\e[31mthp7212_i2c_write_byte failed reg:0x%02x write:0x%04x, retry:%d\e[0m\n", addr, val i);
	}

	if (ret != 1) 
	{
		thp7212_i2c_read_byte(client, addr, &read_val);
		dev_err(&client->dev, "\e[31mthp7212_i2c_write_byte failed reg:0x%02x write:0x%04x, read:0x%04x, retry:%d\e[0m\n", addr, val, read_val, i);
		return -EIO;
	}

	return 0;
}

static int thp7212_i2c_write_block(struct v4l2_subdev *sd, u8 *buf, int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	s8 i = 0;
	s8 ret = 0;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = size;
	msg.buf = buf;

	for(i=0; i<THINE_I2C_RETRY_CNT; i++)
	{
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
	}

	if (ret != 1) 
	{
		dev_err(&client->dev, "\e[31mthp7212_i2c_write_block failed size:%d \e[0m\n", size);
		return -EIO;
	}

	return 0;
}

/*
 * Parse the init_reg2 array into a number of register sets that
 * we can send over as i2c burst writes instead of writing each
 * entry of init_reg2 as a single 4 byte write.  Write the
 * new data structures and then free them.
 */
static int thp7212_write_init_reg2_burst(struct v4l2_subdev *sd) __attribute__((unused));
static int thp7212_write_init_reg2_burst(struct v4l2_subdev *sd)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct thp7212_regset *regset_table;
	struct thp7212_regset *regset;
	struct thp7212_regset *end_regset;
	u8 *regset_data;
	u8 *dst_ptr;
	const u32 *end_src_ptr;
	bool flag_copied;
	int init_reg_2_array_size = state->regs->init_reg_2.array_size;
	int init_reg_2_size = init_reg_2_array_size * sizeof(u32);
	const u32 *src_ptr = state->regs->init_reg_2.reg;
	u32 src_value;
	int err;

	pr_debug("%s : start\n", __func__);

	regset_data = vmalloc(init_reg_2_size);
	if (regset_data == NULL)
		return -ENOMEM;
	regset_table = vmalloc(sizeof(struct thp7212_regset) *
			init_reg_2_size);
	if (regset_table == NULL) {
		kfree(regset_data);
		return -ENOMEM;
	}

	dst_ptr = regset_data;
	regset = regset_table;
	end_src_ptr = &state->regs->init_reg_2.reg[init_reg_2_array_size];

	src_value = *src_ptr++;
	while (src_ptr <= end_src_ptr) {
		/* initial value for a regset */
		regset->data = dst_ptr;
		flag_copied = false;
		*dst_ptr++ = src_value >> 24;
		*dst_ptr++ = src_value >> 16;
		*dst_ptr++ = src_value >> 8;
		*dst_ptr++ = src_value;

		/* check subsequent values for a data flag (starts with
		   0x0F12) or something else */
		do {
			src_value = *src_ptr++;
			if ((src_value & 0xFFFF0000) != 0x0F120000) {
				/* src_value is start of next regset */
				regset->size = dst_ptr - regset->data;
				regset++;
				break;
			}
			/* copy the 0x0F12 flag if not done already */
			if (!flag_copied) {
				*dst_ptr++ = src_value >> 24;
				*dst_ptr++ = src_value >> 16;
				flag_copied = true;
			}
			/* copy the data part */
			*dst_ptr++ = src_value >> 8;
			*dst_ptr++ = src_value;
		} while (src_ptr < end_src_ptr);
	}
	pr_debug("%s : finished creating table\n", __func__);

	end_regset = regset;
	pr_debug("%s : first regset = %p, last regset = %p, count = %d\n",
		__func__, regset_table, regset, end_regset - regset_table);
	pr_debug("%s : regset_data = %p, end = %p, dst_ptr = %p\n", __func__,
		regset_data, regset_data + (init_reg_2_size * sizeof(u32)),
		dst_ptr);

#ifdef CONFIG_VIDEO_THP7212_DEBUG
	if (thp7212_debug_mask & THP7212_DEBUG_I2C_BURSTS) {
		int last_regset_end_addr = 0;
		regset = regset_table;
		do {
			thp7212_dump_regset(regset);
			if (regset->size > 4) {
				int regset_addr = (regset->data[2] << 8 |
						regset->data[3]);
				if (last_regset_end_addr == regset_addr)
					pr_info("%s : this regset can be"
						" combined with previous\n",
						__func__);
				last_regset_end_addr = (regset_addr
							+ regset->size - 6);
			}
			regset++;
		} while (regset < end_regset);
	}
#endif
	regset = regset_table;
	pr_debug("%s : start writing init reg 2 bursts\n", __func__);
	do {
		if (regset->size > 4) {
			/* write the address packet */
			err = thp7212_i2c_write_block(sd, regset->data, 4);
			if (err)
				break;
			/* write the data in a burst */
			err = thp7212_i2c_write_block(sd, regset->data+4,
						regset->size-4);

		} else
			err = thp7212_i2c_write_block(sd, regset->data,
						regset->size);
		if (err)
			break;
		regset++;
	} while (regset < end_regset);

	pr_debug("%s : finished writing init reg 2 bursts\n", __func__);

	vfree(regset_data);
	vfree(regset_table);

	return err;
}

static unsigned int thp7212_get_width(void)
{
#if defined(RESOLUTION_HD)	
	return 1280;
#else
	return 1920;
#endif
}

static unsigned int thp7212_get_height(void)
{
#if defined(RESOLUTION_HD)
	return 720;
#else
	return 1080;
#endif
}



static int thp7212_set_from_table(struct v4l2_subdev *sd,
				const char *setting_name,
				const struct thp7212_regset_table *table,
				int table_size, int index)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	/* return if table is not initilized */
	if ((unsigned int)table < (unsigned int)0xc0000000)
		return 0;

	//dev_err(&client->dev, "%s: set %s index %d\n",
	//	__func__, setting_name, index);
	if ((index < 0) || (index >= table_size)) {
		dev_err(&client->dev, "%s: index(%d) out of range[0:%d] for table for %s\n",
							__func__, index, table_size, setting_name);
		return -EINVAL;
	}
	table += index;
	if (table->reg == NULL)
		return -EINVAL;
	return 0;//thp7212_write_regs(sd, table->reg, table->array_size);
}

static int thp7212_set_parameter(struct v4l2_subdev *sd,
				int *current_value_ptr,
				int new_value,
				const char *setting_name,
				const struct thp7212_regset_table *table,
				int table_size)
{
	int err;
/*
	if (*current_value_ptr == new_value)
		return 0;
		*/

	err = thp7212_set_from_table(sd, setting_name, table,
				table_size, new_value);

	if (!err)
		*current_value_ptr = new_value;
	return err;
}

static int thp7212_set_preview_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	if (state->runmode == THP7212_RUNMODE_RUNNING)
		state->runmode = THP7212_RUNMODE_IDLE;

	dev_err(&client->dev, "%s:\n", __func__);

	return 0;
}

static int thp7212_set_preview_start(struct v4l2_subdev *sd)
{
	int err;
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	bool set_size = true;

	dev_err(&client->dev, "%s: runmode = %d\n", __func__, state->runmode);

	if (!state->pix.width || !state->pix.height ||
		!state->strm.parm.capture.timeperframe.denominator)
		return -EINVAL;

	if (state->runmode == THP7212_RUNMODE_CAPTURE) {
		dev_dbg(&client->dev, "%s: sending Preview_Return cmd\n", __func__);
		err = thp7212_set_from_table(sd, "preview return", &state->regs->preview_return, 1, 0);
		if (err < 0) {
			dev_err(&client->dev, "%s: failed: thp7212_Preview_Return\n", __func__);
			return -EIO;
		}
		set_size = state->restore_preview_size_needed;
	}

	if (set_size) {
		err = thp7212_set_from_table(sd, "preview_size",
					state->regs->preview_size, ARRAY_SIZE(state->regs->preview_size), state->preview_framesize_index);
		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed: Could not set preview size\n", __func__);
			return -EIO;
		}
	}

	dev_dbg(&client->dev, "%s: runmode now RUNNING\n", __func__);
	state->runmode = THP7212_RUNMODE_RUNNING;

	return 0;
}

static int thp7212_set_capture_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	int err;

	dev_err(&client->dev, "%s: index:%d\n", __func__, state->capture_framesize_index);

	err = thp7212_set_from_table(sd, "capture_size",
				state->regs->capture_size, ARRAY_SIZE(state->regs->capture_size), state->capture_framesize_index);
	if (err < 0) {
		dev_err(&client->dev,
			"%s: failed: i2c_write for capture_size index %d\n", __func__, state->capture_framesize_index);
	}
	state->runmode = THP7212_RUNMODE_CAPTURE;

	return err;
}

static int thp7212_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	dev_err(&client->dev, "%s: jpeg.quality %d\n", __func__, state->jpeg.quality);
	if (state->jpeg.quality < 0)
		state->jpeg.quality = 0;
	if (state->jpeg.quality > 100)
		state->jpeg.quality = 100;

	switch (state->jpeg.quality) {
		case 90 ... 100:
			dev_dbg(&client->dev, "%s: setting to high jpeg quality\n", __func__);
			return thp7212_set_from_table(sd, "jpeg quality high", &state->regs->jpeg_quality_high, 1, 0);

		case 80 ... 89:
			dev_dbg(&client->dev, "%s: setting to normal jpeg quality\n", __func__);
			return thp7212_set_from_table(sd, "jpeg quality normal", &state->regs->jpeg_quality_normal, 1, 0);

		default:
			dev_dbg(&client->dev, "%s: setting to low jpeg quality\n", __func__);
			return thp7212_set_from_table(sd, "jpeg quality low", &state->regs->jpeg_quality_low, 1, 0);
	}
}

static u16 thp7212_get_light_level(struct v4l2_subdev *sd)
{
	int err;
	u16 read_value = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	err = thp7212_set_from_table(sd, "get light level", &state->regs->get_light_level, 1, 0);
	if (err) {
		dev_err(&client->dev, "%s: write cmd failed, returning 0\n", __func__);
		goto out;
	}
	//err = thp7212_i2c_read_word(client, 0x0F12, &read_value);
	if (err) {
		dev_err(&client->dev, "%s: read cmd failed, returning 0\n", __func__);
		goto out;
	}

	dev_dbg(&client->dev, "%s: read_value = %d (0x%X)\n", __func__, read_value, read_value);

out:
	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);
	return read_value;
}

static int thp7212_set_zoom(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	struct sec_cam_parm *stored_parms =
		(struct sec_cam_parm *)&state->stored_parm.parm.raw_data;
	u16 zoom_ratio;

	zoom_ratio = (unsigned int)(0x100 + value * 12);

	//thp7212_i2c_write_word(client,  0x0028,  0x7000);
	//thp7212_i2c_write_word(client,  0x002A,  0x048E);
	//thp7212_i2c_write_word(client,  0x0F12,  zoom_ratio);
	//thp7212_i2c_write_word(client,  0x002A,  0x04A4);
	//thp7212_i2c_write_word(client,  0x0F12,  0x05);

	stored_parms->zoom_ratio = value;
	parms->zoom_ratio = value;

	return 0;
}

static int thp7212_start_capture(struct v4l2_subdev *sd)
{
	int err;
	u16 read_value = 0;
	u16 light_level;
	int poll_time_ms;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state = container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms __attribute__((unused)) = (struct sec_cam_parm *)&state->strm.parm.raw_data;
    /* psw0523 fix : not use platform data */
	/* struct thp7212_platform_data *pdata = client->dev.platform_data; */

	/* reset cropping if our current preview is not 640x480,
	 * otherwise the capture will be wrong because of the cropping
	 */
	dev_err(&client->dev, "%s:\n", __func__);
	if (state->preview_framesize_index != THP7212_PREVIEW_VGA) {
		int err = thp7212_set_from_table(sd, "reset crop", &state->regs->reset_crop, 1, 0);
		if (err < 0) {
			dev_err(&client->dev, "%s: failed: Could not set preview size\n", __func__);
			return -EIO;
		}
		state->restore_preview_size_needed = true;
	} else {
		state->restore_preview_size_needed = false;
	}

	light_level = thp7212_get_light_level(sd);

	dev_dbg(&client->dev, "%s: light_level = %d\n", __func__, light_level);

	state->flash_state_on_previous_capture = false;
#if 0//many del //zhuxuezhen 2012-12-27 recovery
	if (parms->scene_mode != SCENE_MODE_NIGHTSHOT) {
		switch (parms->flash_mode) {
		case FLASH_MODE_AUTO:
			if (light_level > LOW_LIGHT_LEVEL) {
				/* light level bright enough
				 * that we don't need flash
				 */
				break;
			}
			/* fall through to flash start */
		case FLASH_MODE_ON:
			if (parms->focus_mode == FOCUS_MODE_INFINITY) {
				thp7212_set_from_table(sd,
					"AF assist flash start",
					&state->regs->af_assist_flash_start,
					1, 0);
				thp7212_set_from_table(sd,
					"AF assist flash end",
					&state->regs->af_assist_flash_end,
					1, 0);
				msleep(10);
			}
			thp7212_set_from_table(sd, "flash start",
					&state->regs->flash_start, 1, 0);
			state->flash_on = true;
			state->flash_state_on_previous_capture = true;
			pdata->flash_onoff(1);
			break;
		default:
			break;
		}
	}

	/* if light is low, use low light capture settings, EXCEPT
	 * if scene mode set to NIGHTSHOT or SPORTS because they
	 * have their own settings (though a low light sport setting
	 * could be useful)
	 */
	if ((light_level <= HIGH_LIGHT_LEVEL) &&
		(parms->scene_mode != SCENE_MODE_NIGHTSHOT) &&
		(parms->scene_mode != SCENE_MODE_SPORTS)) {
		thp7212_set_from_table(sd, "low cap on",
					&state->regs->low_cap_on, 1, 0);
	}
#endif
	err = thp7212_set_capture_size(sd);
	if (err < 0) {
		dev_err(&client->dev, "%s: failed: i2c_write for capture_resolution\n", __func__);
		return -EIO;
	}

	dev_err(&client->dev, "%s: send Capture_Start cmd\n", __func__);

	thp7212_set_from_table(sd, "capture start", &state->regs->capture_start, 1, 0);

	/* a shot takes takes at least 50ms so sleep that amount first
	 * and then start polling for completion.
	 */
	msleep(50);
	/* Enter read mode */
	//thp7212_i2c_write_word(client, 0x002C, 0x7000);
	poll_time_ms = 50;
	do {
		thp7212_set_from_table(sd, "get capture status",
					&state->regs->get_capture_status, 1, 0);
		//thp7212_i2c_read_word(client, 0x0F12, &read_value);
		dev_err(&client->dev, "%s: thp7212_Capture_Start check = %#x\n", __func__, read_value);
		if (read_value != 0x00)
			break;
		msleep(POLL_TIME_MS);
		poll_time_ms += POLL_TIME_MS;
	} while (poll_time_ms < CAPTURE_POLL_TIME_MS);

	dev_err(&client->dev, "%s: capture done check finished after %d ms\n",
		__func__, poll_time_ms);

	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

#if 0//many del//zhuxuezhen 2012-12-27 recovery
	thp7212_set_from_table(sd, "ae awb lock off",
				&state->regs->ae_awb_lock_off, 1, 0);

	if ((light_level <= HIGH_LIGHT_LEVEL) &&
		(parms->scene_mode != SCENE_MODE_NIGHTSHOT) &&
		(parms->scene_mode != SCENE_MODE_SPORTS)) {
		thp7212_set_from_table(sd, "low cap off",
					&state->regs->low_cap_off, 1, 0);
	}

	if ((parms->scene_mode != SCENE_MODE_NIGHTSHOT) && (state->flash_on)) {
		state->flash_on = false;
		pdata->flash_onoff(0);
		thp7212_set_from_table(sd, "flash end",
					&state->regs->flash_end, 1, 0);
	}
#endif

	return 0;
}

/* wide dynamic range support */
static int thp7212_set_wdr(struct v4l2_subdev *sd, int value)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	if (value == WDR_ON)
		return thp7212_set_from_table(sd, "wdr on",
					&state->regs->wdr_on, 1, 0);
	return thp7212_set_from_table(sd, "wdr off",
				&state->regs->wdr_off, 1, 0);
}

static int thp7212_set_face_detection(struct v4l2_subdev *sd, int value)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	if (value == V4L2_FACE_DETECTION_ON)
		return thp7212_set_from_table(sd, "face detection on",
				&state->regs->face_detection_on, 1, 0);
	return thp7212_set_from_table(sd, "face detection off",
				&state->regs->face_detection_off, 1, 0);
}

static int thp7212_return_focus(struct v4l2_subdev *sd) __attribute__((unused));
static int thp7212_return_focus(struct v4l2_subdev *sd)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
			container_of(sd, struct thp7212_state, sd);

	err = thp7212_set_from_table(sd,
		"af normal mode 1",
		&state->regs->af_normal_mode_1, 1, 0);
	if (err < 0)
		goto fail;
	msleep(FIRST_SETTING_FOCUS_MODE_DELAY_MS);
	err = thp7212_set_from_table(sd,
		"af normal mode 2",
		&state->regs->af_normal_mode_2, 1, 0);
	if (err < 0)
		goto fail;
	msleep(SECOND_SETTING_FOCUS_MODE_DELAY_MS);
	err = thp7212_set_from_table(sd,
		"af normal mode 3",
		&state->regs->af_normal_mode_3, 1, 0);
	if (err < 0)
		goto fail;

	return 0;
fail:
	dev_err(&client->dev,
		"%s: i2c_write failed\n", __func__);
	return -EIO;
}

#if 1//many del//zhuxuezhen 2012-12-27 recovery
static int thp7212_set_focus_mode(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err;

	if (parms->focus_mode == value)
		return 0;

	dev_err(&client->dev, "%s value(%d)\n", __func__, value);

	switch (value) {
	case V4L2_FOCUS_MODE_MACRO:
	case V4L2_FOCUS_MODE_MACRO_DEFAULT:
	case V4L2_FOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
		dev_dbg(&client->dev,
				"%s: FOCUS_MODE_MACRO\n", __func__);
		err = thp7212_set_from_table(sd, "af macro mode 1",
				&state->regs->af_macro_mode_1, 1, 0);
		if (err < 0)
			goto fail;
		msleep(FIRST_SETTING_FOCUS_MODE_DELAY_MS);
		err = thp7212_set_from_table(sd, "af macro mode 2",
				&state->regs->af_macro_mode_2, 1, 0);
		if (err < 0)
			goto fail;
		msleep(SECOND_SETTING_FOCUS_MODE_DELAY_MS);
		err = thp7212_set_from_table(sd, "af macro mode 3",
				&state->regs->af_macro_mode_3, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = FOCUS_MODE_MACRO;
		break;

	case V4L2_FOCUS_MODE_AUTO:
	case V4L2_FOCUS_MODE_AUTO_DEFAULT:
	case V4L2_FOCUS_MODE_FACEDETECT_DEFAULT:
	case V4L2_FOCUS_MODE_INFINITY:
	case V4L2_FOCUS_MODE_FIXED:
	case V4L2_FOCUS_MODE_CONTINUOUS:
	case V4L2_FOCUS_MODE_CONTINUOUS_PICTURE:
	case V4L2_FOCUS_MODE_CONTINUOUS_VIDEO:
	case V4L2_FOCUS_MODE_TOUCH:
		err = thp7212_set_from_table(sd,
			"af normal mode 1",
			&state->regs->af_normal_mode_1, 1, 0);
		if (err < 0)
			goto fail;
		msleep(FIRST_SETTING_FOCUS_MODE_DELAY_MS);
		err = thp7212_set_from_table(sd,
			"af normal mode 2",
			&state->regs->af_normal_mode_2, 1, 0);
		if (err < 0)
			goto fail;
		msleep(SECOND_SETTING_FOCUS_MODE_DELAY_MS);
		err = thp7212_set_from_table(sd,
			"af normal mode 3",
			&state->regs->af_normal_mode_3, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = value;
		break;
	default:
		return -EINVAL;
		break;
	}

	return 0;
fail:
	dev_err(&client->dev,
		"%s: i2c_write failed\n", __func__);
	return -EIO;
}

static void thp7212_auto_focus_flash_start(struct v4l2_subdev *sd) __attribute__((unused));
static void thp7212_auto_focus_flash_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
    /* psw0523 fix not use platform data */
	/* struct thp7212_platform_data *pdata = client->dev.platform_data; */
	int count;
	u16 read_value;

	thp7212_set_from_table(sd, "AF assist flash start",
				&state->regs->af_assist_flash_start, 1, 0);
	state->flash_on = true;
	/* pdata->af_assist_onoff(1); */

	/* delay 200ms (SLSI value) and then poll to see if AE is stable.
	 * once it is stable, lock it and then return to do AF
	 */
	msleep(200);

	/* enter read mode */
	//thp7212_i2c_write_word(client, 0x002C, 0x7000);
	for (count = 0; count < AE_STABLE_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL)
			break;
		thp7212_set_from_table(sd, "get ae stable status",
				&state->regs->get_ae_stable_status, 1, 0);
		//thp7212_i2c_read_word(client, 0x0F12, &read_value);
		dev_dbg(&client->dev, "%s: ae stable status = %#x\n",
			__func__, read_value);
		if (read_value == 0x1)
			break;
		msleep(state->one_frame_delay_ms);
	}

	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

	/* if we were cancelled, turn off flash */
	if (state->af_status == AF_CANCEL) {
		dev_dbg(&client->dev,
			"%s: AF cancelled\n", __func__);
		thp7212_set_from_table(sd, "AF assist flash end",
				&state->regs->af_assist_flash_end, 1, 0);
		state->flash_on = false;
		/* pdata->af_assist_onoff(0); */
	}
}

static int thp7212_start_continuous_auto_focus(struct v4l2_subdev *sd)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	thp7212_set_from_table(sd, "continuous af start",
				&state->regs->continuous_af_on, 1, 0);

	return 0;
}
static int thp7212_stop_continuous_auto_focus(struct v4l2_subdev *sd)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	thp7212_set_from_table(sd, "continuous af stop",
				&state->regs->continuous_af_off, 1, 0);

	return 0;
}
static int thp7212_start_auto_focus(struct v4l2_subdev *sd)
{
	/* int light_level; */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	/* int err = 0; */

	dev_err(&client->dev, "%s: start SINGLE AF operation, flash mode %d\n",
		__func__, parms->flash_mode);

	/* in case user calls auto_focus repeatedly without a cancel
	 * or a capture, we need to cancel here to allow ae_awb
	 * to work again, or else we could be locked forever while
	 * that app is running, which is not the expected behavior.
	 */
#ifdef ENABLE
	err = thp7212_set_parameter(sd, &parms->aeawb_lockunlock,
				V4L2_AE_LOCK_AWB_LOCK, "aeawb_lockunlock",
				state->regs->aeawb_lockunlock,
				ARRAY_SIZE(state->regs->aeawb_lockunlock));
	if (err < 0) {
		dev_err(&client->dev, "%s: ae & awb lock is fail. \n",
			__func__);
	}
#endif

#ifdef ENABLE
	if (parms->scene_mode == SCENE_MODE_NIGHTSHOT) {
		/* user selected night shot mode, assume we need low light
		 * af mode.  flash is always off in night shot mode
		 */
		goto enable_af_low_light_mode;
	}
	light_level = thp7212_get_light_level(sd);

	switch (parms->flash_mode) {
	case FLASH_MODE_AUTO:
		if (light_level > LOW_LIGHT_LEVEL) {
			/* flash not needed */
			break;
		}
		/* fall through to turn on flash for AF assist */
	case FLASH_MODE_ON:
		thp7212_auto_focus_flash_start(sd);
		if (state->af_status == AF_CANCEL)
			return 0;
		break;
	case FLASH_MODE_OFF:
		break;
	default:
		dev_err(&client->dev,
			"%s: Unknown Flash mode 0x%x\n",
			__func__, parms->flash_mode);
		break;
	}

	if (light_level > LOW_LIGHT_LEVEL) {
		if (state->sensor_af_in_low_light_mode) {
			state->sensor_af_in_low_light_mode = false;
			thp7212_set_from_table(sd, "af low light mode off",
				&state->regs->af_low_light_mode_off, 1, 0);
		}
	} else {
enable_af_low_light_mode:
		if (!state->sensor_af_in_low_light_mode) {
			state->sensor_af_in_low_light_mode = true;
			thp7212_set_from_table(sd, "af low light mode on",
				&state->regs->af_low_light_mode_on, 1, 0);
		}
	}
#endif
	thp7212_set_from_table(sd, "single af start",
				&state->regs->single_af_start, 1, 0);
	state->af_status = AF_START;
	INIT_COMPLETION(state->af_complete);
	dev_dbg(&client->dev, "%s: af_status set to start\n", __func__);

	return 0;
}

static int thp7212_stop_auto_focus(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	/* struct sec_cam_parm *parms = */
	/* 	(struct sec_cam_parm *)&state->strm.parm.raw_data; */
	/* int focus_mode = parms->focus_mode; */
	/* int err = 0; */

	dev_err(&client->dev, "%s: single AF Off command Setting\n", __func__);

	/* always cancel ae_awb, in case AF already finished before
	 * we got called.
	 */

#ifdef ENABLE
	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

	err = thp7212_set_parameter(sd, &parms->aeawb_lockunlock,
				V4L2_AE_UNLOCK_AWB_UNLOCK, "aeawb_lockunlock",
				state->regs->aeawb_lockunlock,
				ARRAY_SIZE(state->regs->aeawb_lockunlock));
	if (err < 0) {
		dev_err(&client->dev, "%s: ae & awb unlock is fail. \n",
			__func__);
	}

	if (state->af_status != AF_START) {
		/* we weren't in the middle auto focus operation, we're done */
		dev_dbg(&client->dev,
			"%s: auto focus not in progress, done\n", __func__);

		if (focus_mode == FOCUS_MODE_MACRO) {
			/* for change focus mode forcely */
			parms->focus_mode = -1;
			thp7212_set_focus_mode(sd, FOCUS_MODE_MACRO);
		} else if (focus_mode == FOCUS_MODE_AUTO) {
			/* for change focus mode forcely */
			parms->focus_mode = -1;
			thp7212_set_focus_mode(sd, FOCUS_MODE_AUTO);
		}

		return 0;
	}
#endif

	/* auto focus was in progress.  the other thread
	 * is either in the middle of get_auto_focus_result()
	 * or will call it shortly.  set a flag to have
	 * it abort it's polling.  that thread will
	 * also do cleanup like restore focus position.
	 *
	 * it might be enough to just send sensor commands
	 * to abort auto focus and the other thread would get
	 * that state from it's polling calls, but I'm not sure.
	 */
	state->af_status = AF_CANCEL;
	dev_dbg(&client->dev,
		"%s: sending Single_AF_Off commands to sensor\n", __func__);

	thp7212_set_from_table(sd, "single af off 1",
				&state->regs->single_af_off_1, 1, 0);

	msleep(state->one_frame_delay_ms);

	thp7212_set_from_table(sd, "single af off 2",
				&state->regs->single_af_off_2, 1, 0);

	msleep(state->one_frame_delay_ms);

	/* wait until the other thread has completed
	 * aborting the auto focus and restored state
	 */
#ifdef ENABLE
	dev_dbg(&client->dev, "%s: wait AF cancel done start\n", __func__);
	mutex_unlock(&state->ctrl_lock);
	wait_for_completion(&state->af_complete);
	mutex_lock(&state->ctrl_lock);
	dev_dbg(&client->dev, "%s: wait AF cancel done finished\n", __func__);
#endif

	return 0;
}

/* called by HAL after auto focus was started to get the result.
 * it might be aborted asynchronously by a call to set_auto_focus
 */
static int thp7212_get_auto_focus_result(struct v4l2_subdev *sd,
					struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err, count;
	u16 read_value = 0;

	dev_err(&client->dev, "%s: Check AF Result\n", __func__);

	if (state->af_status == AF_NONE) {
		dev_dbg(&client->dev,
			"%s: auto focus never started, returning 0x2\n",
			__func__);
		ctrl->value = AUTO_FOCUS_CANCELLED;
		return 0;
	}

	/* must delay 2 frame times before checking result of 1st phase */
	mutex_unlock(&state->ctrl_lock);
	msleep(state->one_frame_delay_ms*2);
	mutex_lock(&state->ctrl_lock);

	/* lock AE and AWB after first AF search */
	err = thp7212_set_parameter(sd, &parms->aeawb_lockunlock,
				V4L2_AE_UNLOCK_AWB_UNLOCK, "aeawb_lockunlock",
				state->regs->aeawb_lockunlock,
				ARRAY_SIZE(state->regs->aeawb_lockunlock));
	if (err < 0) {
		dev_err(&client->dev, "%s: ae & awb unlock is fail. \n",
			__func__);
	}
	//thp7212_set_from_table(sd, "ae awb lock on",
	//			&state->regs->ae_awb_lock_on, 1, 0);

	dev_dbg(&client->dev, "%s: 1st AF search\n", __func__);
	/* enter read mode */
	//thp7212_i2c_write_word(client, 0x002C, 0x7000);
	for (count = 0; count < FIRST_AF_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL) {
			dev_dbg(&client->dev,
				"%s: AF is cancelled while doing\n", __func__);
			ctrl->value = AUTO_FOCUS_CANCELLED;
			goto check_flash;
		}
		thp7212_set_from_table(sd, "get 1st af search status",
					&state->regs->get_1st_af_search_status,
					1, 0);
		//thp7212_i2c_read_word(client, 0x0F12, &read_value);
		dev_dbg(&client->dev,
			"%s: 1st i2c_read --- read_value == 0x%x\n",
			__func__, read_value);

		/* check for success and failure cases.  0x1 is
		 * auto focus still in progress.  0x2 is success.
		 * 0x0,0x3,0x4,0x6,0x8 are all failures cases
		 */
		if (read_value != 0x01)
			break;
		mutex_unlock(&state->ctrl_lock);
		msleep(50);
		mutex_lock(&state->ctrl_lock);
	}

	if ((count >= FIRST_AF_SEARCH_COUNT) || (read_value != 0x02)) {
		dev_dbg(&client->dev,
			"%s: 1st scan timed out or failed\n", __func__);
		ctrl->value = AUTO_FOCUS_FAILED;
		goto check_flash;
	}

	dev_dbg(&client->dev, "%s: 2nd AF search\n", __func__);

	/* delay 1 frame time before checking for 2nd AF completion */
	mutex_unlock(&state->ctrl_lock);
	msleep(state->one_frame_delay_ms);
	mutex_lock(&state->ctrl_lock);

	/* this is the long portion of AF, can take a second or more.
	 * we poll and wakeup more frequently than 1 second mainly
	 * to see if a cancel was requested
	 */
	for (count = 0; count < SECOND_AF_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL) {
			dev_dbg(&client->dev,
				"%s: AF is cancelled while doing\n", __func__);
			ctrl->value = AUTO_FOCUS_CANCELLED;
			goto check_flash;
		}
		thp7212_set_from_table(sd, "get 2nd af search status",
					&state->regs->get_2nd_af_search_status,
					1, 0);
		//thp7212_i2c_read_word(client, 0x0F12, &read_value);
		dev_dbg(&client->dev,
			"%s: 2nd i2c_read --- read_value == 0x%x\n",
			__func__, read_value);

		/* low byte is garbage.  done when high byte is 0x0 */
		if (!(read_value & 0xff00))
			break;

		mutex_unlock(&state->ctrl_lock);
		msleep(50);
		mutex_lock(&state->ctrl_lock);
	}

	if (count >= SECOND_AF_SEARCH_COUNT) {
		dev_dbg(&client->dev, "%s: 2nd scan timed out\n", __func__);
		ctrl->value = AUTO_FOCUS_FAILED;
		goto check_flash;
	}

	dev_dbg(&client->dev, "%s: AF is success\n", __func__);
	ctrl->value = AUTO_FOCUS_DONE;

check_flash:
	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

	if (state->flash_on) {
		struct thp7212_platform_data *pd = client->dev.platform_data;
		thp7212_set_from_table(sd, "AF assist flash end",
				&state->regs->af_assist_flash_end, 1, 0);
		state->flash_on = false;
		pd->af_assist_onoff(0);
	}

	dev_dbg(&client->dev, "%s: single AF finished\n", __func__);
	state->af_status = AF_NONE;
	complete(&state->af_complete);
	return err;
}
#endif


static void thp7212_init_parameters(struct v4l2_subdev *sd)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	struct sec_cam_parm *stored_parms =
		(struct sec_cam_parm *)&state->stored_parm.parm.raw_data;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_err(&client->dev, "%s: \n", __func__);
	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms->capture.capturemode = 0;
	parms->capture.timeperframe.numerator = 1;
	parms->capture.timeperframe.denominator = 30;
	parms->contrast = V4L2_CONTRAST_DEFAULT;
	parms->effects = V4L2_IMAGE_EFFECT_NORMAL;
	parms->brightness = V4L2_BRIGHTNESS_DEFAULT;
	parms->flash_mode = FLASH_MODE_AUTO;
	parms->focus_mode = V4L2_FOCUS_MODE_AUTO;
	parms->iso = V4L2_ISO_AUTO;
	parms->metering = V4L2_METERING_CENTER;
	parms->saturation = V4L2_SATURATION_DEFAULT;
	parms->scene_mode = V4L2_SCENE_MODE_NONE;
	parms->sharpness = V4L2_SHARPNESS_DEFAULT;
	parms->white_balance = V4L2_WHITE_BALANCE_AUTO;
	parms->aeawb_lockunlock = V4L2_AE_UNLOCK_AWB_UNLOCK;

	stored_parms->effects = V4L2_IMAGE_EFFECT_NORMAL;
	stored_parms->brightness = V4L2_BRIGHTNESS_DEFAULT;
	stored_parms->iso = V4L2_ISO_AUTO;
	stored_parms->metering = V4L2_METERING_CENTER;
	stored_parms->scene_mode = V4L2_SCENE_MODE_NONE;
	stored_parms->white_balance = V4L2_WHITE_BALANCE_AUTO;

	state->jpeg.enable = 0;
	state->jpeg.quality = 100;
	state->jpeg.main_offset = 0;
	state->jpeg.main_size = 0;
	state->jpeg.thumb_offset = 0;
	state->jpeg.thumb_size = 0;
	state->jpeg.postview_offset = 0;

	state->fw.major = 1;

	state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;

    /* psw0523 block this */
	/* thp7212_stop_auto_focus(sd); */
}

static void thp7212_set_framesize(struct v4l2_subdev *sd,
				const struct thp7212_framesize *frmsize,
				int frmsize_count, bool exact_match);



#if 1 //many del//zhuxuezhen 2012-12-27 recovery
static void thp7212_enable_torch(struct v4l2_subdev *sd)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	/* struct thp7212_platform_data *pdata = client->dev.platform_data; */

	thp7212_set_from_table(sd, "torch start",
				&state->regs->flash_start, 1, 0);
	state->torch_on = true;
	/* pdata->torch_onoff(1); */
}

static void thp7212_disable_torch(struct v4l2_subdev *sd)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	/* struct thp7212_platform_data *pdata = client->dev.platform_data; */

	if (state->torch_on) {
		state->torch_on = false;
		/* pdata->torch_onoff(0); */
		thp7212_set_from_table(sd, "torch end",
					&state->regs->flash_end, 1, 0);
	}
}

static int thp7212_set_flash_mode(struct v4l2_subdev *sd, int value)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	if (parms->flash_mode == value)
		return 0;

	if ((value >= FLASH_MODE_OFF) && (value <= FLASH_MODE_TORCH)) {
		pr_debug("%s: setting flash mode to %d\n",
			__func__, value);
		parms->flash_mode = value;
		if (parms->flash_mode == FLASH_MODE_TORCH)
			thp7212_enable_torch(sd);
		else
			thp7212_disable_torch(sd);
		return 0;
	}
	pr_debug("%s: trying to set invalid flash mode %d\n",
		__func__, value);
	return -EINVAL;
}
#endif


static int thp7212_set_stored_parms(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	struct sec_cam_parm *stored_parms =
		(struct sec_cam_parm *)&state->stored_parm.parm.raw_data;
	int err = 0;

	if (parms->effects != stored_parms->effects)
	err |= thp7212_set_parameter(sd, &parms->effects, stored_parms->effects,
				"effect", state->regs->effect,
				ARRAY_SIZE(state->regs->effect));
///	err |= thp7212_set_flash_mode(sd, stored_parms->flash_mode);
///	err |= thp7212_set_focus_mode(sd, stored_parms->focus_mode);
	if (parms->iso != stored_parms->iso)
	err |= thp7212_set_parameter(sd, &parms->iso, stored_parms->iso,
				"iso", state->regs->iso,
				ARRAY_SIZE(state->regs->iso));
	if (parms->metering != stored_parms->metering)
	err |= thp7212_set_parameter(sd, &parms->metering, stored_parms->metering,
				"metering", state->regs->metering,
				ARRAY_SIZE(state->regs->metering));

	err |= thp7212_set_parameter(sd, &parms->scene_mode,
				stored_parms->scene_mode, "scene_mode",
				state->regs->scene_mode,
				ARRAY_SIZE(state->regs->scene_mode));
	if (parms->aeawb_lockunlock != stored_parms->aeawb_lockunlock)
	err |= thp7212_set_parameter(sd, &parms->aeawb_lockunlock,
				stored_parms->aeawb_lockunlock, "aeawb_lockunlock",
				state->regs->aeawb_lockunlock,
				ARRAY_SIZE(state->regs->aeawb_lockunlock));
	if (parms->white_balance != stored_parms->white_balance)
	err |= thp7212_set_parameter(sd, &parms->white_balance,
				stored_parms->white_balance, "white balance",
				state->regs->white_balance,
				ARRAY_SIZE(state->regs->white_balance));
	/*
	if (parms->fps != stored_parms->fps)
	err |= thp7212_set_parameter(sd, &parms->fps,
				stored_parms->fps, "fps",
				state->regs->fps,
				ARRAY_SIZE(state->regs->fps));

	if (stored_parms->scene_mode == SCENE_MODE_NIGHTSHOT)
		state->one_frame_delay_ms = NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
	else
		state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
	*/

	dev_dbg(&client->dev, "%s: return %d\n", __func__, err);
	return err;
}


/* This function is called from the g_ctrl api
 *
 * This function should be called only after the s_fmt call,
 * which sets the required width/height value.
 *
 * It checks a list of available frame sizes and sets the
 * most appropriate frame size.
 *
 * The list is stored in an increasing order (as far as possible).
 * Hence the first entry (searching from the beginning) where both the
 * width and height is more than the required value is returned.
 * In case of no perfect match, we set the last entry (which is supposed
 * to be the largest resolution supported.)
 */
static void thp7212_set_framesize(struct v4l2_subdev *sd,
				const struct thp7212_framesize *frmsize,
				int frmsize_count, bool preview)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	const struct thp7212_framesize *last_frmsize =
		&frmsize[frmsize_count - 1];
	int err;

	dev_err(&client->dev, "%s: Requested Res: %dx%d\n", __func__,
		state->pix.width, state->pix.height);

	do {
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * return the next higher image resolution. */
		if (preview) {
			if (frmsize->width == state->pix.width &&
				frmsize->height == state->pix.height) {
				break;
			}
		} else {
			dev_err(&client->dev,
				"%s: compare frmsize %dx%d to %dx%d\n",
				__func__,
				frmsize->width, frmsize->height,
				state->pix.width, state->pix.height);
			if (frmsize->width >= state->pix.width &&
				frmsize->height >= state->pix.height) {
				dev_err(&client->dev,
					"%s: select frmsize %dx%d, index=%d\n",
					__func__,
					frmsize->width, frmsize->height,
					frmsize->index);
				break;
			}
		}

		frmsize++;
	} while (frmsize <= last_frmsize);

	if (frmsize > last_frmsize)
		frmsize = last_frmsize;

	state->pix.width = frmsize->width;
	state->pix.height = frmsize->height;
	if (preview) {
		state->preview_framesize_index = frmsize->index;
		dev_err(&client->dev, "%s: Preview Res Set: %dx%d, index %d\n",
			__func__, state->pix.width, state->pix.height,
			state->preview_framesize_index);

		err = thp7212_set_from_table(sd, "set preview size",
					state->regs->preview_size,
					ARRAY_SIZE(state->regs->preview_size),
					state->preview_framesize_index);
		if (err < 0) {
			v4l_info(client, "%s: register set failed\n", __func__);
		}

	} else {
		state->capture_framesize_index = frmsize->index;
		dev_err(&client->dev, "%s: Capture Res Set: %dx%d, index %d\n",
			__func__, state->pix.width, state->pix.height,
			state->capture_framesize_index);

		err = thp7212_set_from_table(sd, "set capture size",
					state->regs->capture_size,
					ARRAY_SIZE(state->regs->capture_size),
					state->capture_framesize_index);

		if (err < 0) {
			v4l_info(client, "%s: register set failed\n", __func__);
		}

	}

}

static int thp7212_check_dataline_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	int err;

	dev_dbg(&client->dev, "%s\n", __func__);

	err = thp7212_set_from_table(sd, "DTP stop",
				&state->regs->dtp_stop, 1, 0);
	if (err < 0) {
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}

	state->check_dataline = 0;

	return err;
}

static void thp7212_get_esd_int(struct v4l2_subdev *sd, struct v4l2_control *ctrl) __attribute__((unused));
static void thp7212_get_esd_int(struct v4l2_subdev *sd,
				struct v4l2_control *ctrl)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 read_value;
	int err;

	if ((THP7212_RUNMODE_RUNNING == state->runmode) &&
		(state->af_status != AF_START)) {
		err = thp7212_set_from_table(sd, "get esd status",
					&state->regs->get_esd_status,
					1, 0);
		//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value);
		dev_dbg(&client->dev,
			"%s: read_value == 0x%x\n", __func__, read_value);
		/* return to write mode */
		//err |= thp7212_i2c_write_word(client, 0x0028, 0x7000);

		if (err < 0) {
			v4l_info(client,
				"Failed I2C for getting ESD information\n");
			ctrl->value = 0x01;
		} else {
			if (read_value != 0x0000) {
				v4l_info(client, "ESD interrupt happened!!\n");
				ctrl->value = 0x01;
			} else {
				dev_dbg(&client->dev,
					"%s: No ESD interrupt!!\n", __func__);
				ctrl->value = 0x00;
			}
		}
	} else
		ctrl->value = 0x00;
}

/* returns the real iso currently used by sensor due to lighting
 * conditions, not the requested iso we sent using s_ctrl.
 */
static int thp7212_get_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	u16 read_value1 = 0;
	u16 read_value2 = 0;
	int read_value;

	err = thp7212_set_from_table(sd, "get iso",
				&state->regs->get_iso, 1, 0);
	//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value1);
	//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value2);

	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

	read_value = read_value1 * read_value2 / 384;

	if (read_value > 0x400)
		ctrl->value = ISO_400;
	else if (read_value > 0x200)
		ctrl->value = ISO_200;
	else if (read_value > 0x100)
		ctrl->value = ISO_100;
	else
		ctrl->value = ISO_50;

	dev_dbg(&client->dev, "%s: get iso == %d (0x%x, 0x%x)\n",
		__func__, ctrl->value, read_value1, read_value2);

	return err;
}

static int thp7212_get_shutterspeed(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	u16 read_value_1 = 0;
	u16 read_value_2 = 0;
	u32 read_value;

	printk("LINE:%d", __LINE__);

	err = thp7212_set_from_table(sd, "get shutterspeed",
				&state->regs->get_shutterspeed, 1, 0);
	//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value_1);
	//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value_2);

	read_value = (read_value_2 << 16) | (read_value_1 & 0xffff);
	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

	ctrl->value = read_value * 1000 / 400;
	dev_err(&client->dev,
			"%s: get shutterspeed == %d\n", __func__, ctrl->value);

	return err;
}

static int thp7212_get_frame_count(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	u16 read_value_1 = 0;
	u32 read_value;

	printk("LINE:%d", __LINE__);

	err = thp7212_set_from_table(sd, "get frame count",
				&state->regs->get_frame_count, 1, 0);
	//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value_1);

	//err |= thp7212_i2c_read_word(client, 0x0F12, &read_value_2);

	//read_value = (read_value_2 << 16) | (read_value_1 & 0xffff);
	read_value = read_value_1 & 0xffff;
	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

	//ctrl->value = read_value * 1000 / 400;
	dev_err(&client->dev,
			"%s: get frame count== %d\n", __func__, ctrl->value);
	dev_err(&client->dev,
			"%s: get frame count== %d\n", __func__, (unsigned int)read_value);

	return err;
}



static int thp7212_init(struct v4l2_subdev *sd, u32 val);

#ifdef USE_PRESET
static void thp7212_set_preset(struct v4l2_subdev *sd, struct reg_val *preset, int size)
{
    int i;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct reg_val *regval = preset;

    /* printk("%s: size %d\n", __func__, size); */
    for (i = 0; i < size; i++) {
        if (regval->reg == 0xFFFF)
            mdelay(regval->val);
        else
            //thp7212_i2c_write_word(client, regval->reg, regval->val);
        regval++;
    }
    /* printk("preset end!!!\n"); */
}
#endif

// psw0523 for debugging
#include <mach/platform.h>


#ifdef CONFIG_VIDEO_THP7212_DEBUG
static void thp7212_dump_regset(struct thp7212_regset *regset)
{
	if ((regset->data[0] == 0x00) && (regset->data[1] == 0x2A)) {
		if (regset->size <= 6)
			pr_err("odd regset size %d\n", regset->size);
		pr_info("regset: addr = 0x%02X%02X, data[0,1] = 0x%02X%02X,"
			" total data size = %d\n",
			regset->data[2], regset->data[3],
			regset->data[6], regset->data[7],
			regset->size-6);
	} else {
		pr_info("regset: 0x%02X%02X%02X%02X\n",
			regset->data[0], regset->data[1],
			regset->data[2], regset->data[3]);
		if (regset->size != 4)
			pr_err("odd regset size %d\n", regset->size);
	}
}
#endif



static int thp7212_init_regs(struct v4l2_subdev *sd) __attribute__((unused));
static int thp7212_init_regs(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	u16 read_value;

	/* we'd prefer to do this in probe, but the framework hasn't
	 * turned on the camera yet so our i2c operations would fail
	 * if we tried to do it in probe, so we have to do it here
	 * and keep track if we succeeded or not.
	 */

	/* enter read mode */
	//thp7212_i2c_write_word(client, 0x002C, 0x7000);

	//thp7212_i2c_write_word(client, 0x002E, 0x01A6);
	//thp7212_i2c_read_word(client, 0x0F12, &read_value);

	pr_info("%s : revision %08X\n", __func__, read_value);

	/* restore write mode */
	//thp7212_i2c_write_word(client, 0x0028, 0x7000);

#ifdef CONFIG_VIDEO_THP7212_V_1_0
	if (read_value == THP7212_VERSION_1_0) {
		state->regs = &regs_for_fw_version_1_0;
		state->initialized = true;
		return 0;
	}
#endif
#ifdef CONFIG_VIDEO_THP7212_V_1_1
	if (read_value == THP7212_VERSION_1_1) {
		state->fw.minor = 1;
		state->regs = &regs_for_fw_version_1_1;
		state->initialized = true;
		return 0;
	}
#endif

	dev_err(&client->dev, "%s: unknown fw version 0x%x\n",
		__func__, read_value);
	return -ENODEV;
}


/*
 * s_config subdev ops
 * With camera device, we need to re-initialize
 * every single opening time therefor,
 * it is not necessary to be initialized on probe time.
 * except for version checking
 * NOTE: version checking is optional
 */
static int thp7212_s_config(struct v4l2_subdev *sd, int irq, void *platform_data) __attribute__((unused));


static int thp7212_power(int flag)
{
	printk("%s: sensor is power %s\n", __func__, flag == 1 ?"on":"off");
	//Attention: Power On the all the camera module when system turn on
	//Here only control the reset&&powerdown pin
   //xiebin.wang@20110530
	/* Camera A */
	if(flag == 1) {
	} else {
	}

	return 0;

}

static int thp7212_s_power(struct v4l2_subdev *sd, int on)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	int ret = 0;

    PM_DBGOUT("%s: on %d\n", __func__, on);

    if (on) {
		ret = thp7212_power(THP7212_HW_POWER_ON);

		thp7212_init_parameters(sd);

#if 0
#ifdef USE_PRESET
        if (!state->initialized) {
            printk("%s: init!\n", __func__);
#ifndef USE_INITIAL_WORKER_THREAD
            state->initialized = true;
            thp7212_set_preset(sd, thp7212_reg_init, ARRAY_SIZE(thp7212_reg_init));
#else
            printk("queue_work ---> thp7212_init_work\n");
            queue_work(state->init_wq, &state->init_work);
#endif
        }
#else
		if (thp7212_init_regs(&state->sd) < 0)
			return -ENODEV;

		dev_dbg(&client->dev, "%s: state->check_dataline : %d\n",
			__func__, state->check_dataline);

		msleep(10); //many add
#endif
#endif

		state->power_on = THP7212_HW_POWER_ON;
	} else {
		ret = thp7212_power(THP7212_HW_POWER_OFF);
		state->power_on = THP7212_HW_POWER_OFF;
        state->initialized = false;
	}
		return ret;
}

static int thp7212_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state = container_of(sd, struct thp7212_state, sd);
	/* struct sec_cam_parm *parms =	(struct sec_cam_parm *)&state->strm.parm.raw_data; */
	struct sec_cam_parm *stored_parms = (struct sec_cam_parm *)&state->stored_parm.parm.raw_data;
	int ret = 0;

#if 0
	u8 crc_val[4];
	int i=0;
	u8 read_value = 0;
#endif

	dev_err(&client->dev, "%s: start\n", __func__);

#if 1
	unsigned int reset_en=0;

	// device reset
	reset_en = CFG_IO_CAMERA_BACK_RESET;
	nxp_soc_gpio_set_io_dir(reset_en, 1);
	nxp_soc_gpio_set_io_func(reset_en, nxp_soc_gpio_get_altnum(reset_en));
	nxp_soc_gpio_set_out_value(reset_en, 0);
	mdelay(1);
	nxp_soc_gpio_set_out_value(reset_en, 1);
	mdelay(2);	
#endif

	// Start C0 00 00 ram_7210.elf.s19.1 Stop 
	ret = thp7212_i2c_write_block(sd, ram_7210_elf_s19_1, sizeof(ram_7210_elf_s19_1));
	if(ret < 0) {
		dev_err(&client->dev, "\e[31m%s(ram_7210_elf_s19_1) i2c error\e[0m, ret = %d\n", __func__, ret);
		return ret;
	}
	// Start C0 FF 08 10 00 80 Stop 
	{
		u8 buf[] = { 0xFF, 0x08, 0x10, 0x00, 0x80 };
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 FF 08 10 00 80 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

	// Start C0 00 00 ram_7210.elf.s19.2 Stop
	ret = thp7212_i2c_write_block(sd, ram_7210_elf_s19_2, sizeof(ram_7210_elf_s19_2));
	if(ret < 0) {
		dev_err(&client->dev, "\e[31m%s(ram_7210_elf_s19_2) i2c error\e[0m, ret = %d\n", __func__, ret);
		return ret;
	}
	// Start C0 FF 08 10 01 00 Stop 
	{
		mdelay(100);
		u8 buf[] = { 0xFF, 0x08, 0x10, 0x01, 0x00 };
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s( Start C0 FF 08 10 01 00 Stop ) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

	// Start C0 00 00 ram_7210.elf.s19.3 Stop
	ret = thp7212_i2c_write_block(sd, ram_7210_elf_s19_3, sizeof(ram_7210_elf_s19_3));
	if(ret < 0) {
		dev_err(&client->dev, "\e[31m%s(ram_7210_elf_s19_3) i2c error\e[0m, ret = %d\n", __func__, ret);
		return ret;
	}

//CRC CHECK
#if 0
	// Start C0 FF 38 00 01 21 AC Stop 
	{
		u8 buf[] = { 0xFF, 0x38, 0x00, 0x01, 0x21, 0xAC };
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 FF 38 00 01 22 F4 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

	// Start C0 FF 3C 01 Stop 
	{
		u8 buf[] = { 0xFF, 0x3C, 0x01 };
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 FF 3C 01 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xFF30, crc_val, 4) != 0)
		pr_info("[%s] : Read CRC Error!!!!\n", __func__);
	else
	{
		for(i=0; i<4; i++)		
			pr_info("%s : Read CRC Value[%d]  : 0x %02X\n", __func__, i, crc_val[i] );
	}

#endif
	// Start C0 FF 00 01 Stop 
	{
		u8 buf[] = { 0xFF, 0x00, 0x01 };
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 FF 00 01 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

#if 1//def CONFIG_BB_SDK_FRAME_RATE_CONTROL
	mdelay(100);
	// Start C0 F0 1C 0x Stop 
	{
		u8 buf[] = { 0xF0, 0x1C, 0x00 }; //Frame rate select 0x00-0x02 (0x00:30fps, 0x01:25fps, 0x02:20fps)
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 F0 1C 0x Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}	
#endif

	mdelay(100);
	// Start C0 F0 0B 01 Stop 
	{
		u8 buf[] = { 0xF0, 0x0B, 0x01 }; //	Output ON/OFF 0x00-0x01 (0x00:OFF, 0x01:ON)
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 F0 0B 01 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

#if 0//defined(RESOLUTION_HD)
		mdelay(200);
		// Start C0 F0 16 0A Stop 
		{
			u8 buf[] = { 0xF0, 0x16, 0x0A }; //resolution select
			ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
			if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 F0 16 0A Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
				return ret;
			}
		}
#endif

//firmware verification
#if 0
	if(thp7212_i2c_read_data(client, 0xF001, &read_value, 1) != 0)
		pr_info("[%s] :  Keun Revision Error!!!!\n", __func__);
	else
		pr_info("%s : Keun Revision : 0x %02X\n", __func__, read_value);

	read_value = 0;

	if(thp7212_i2c_read_data(client, 0xF016, &read_value, 1) != 0)
		pr_info("[%s] :  Keun Frame Size Error!!!!\n", __func__);
	else
		pr_info("%s : Keun Frame Size : 0x %02X\n", __func__, read_value);

	if(thp7212_i2c_read_data(client, 0xF01C, &read_value, 1) != 0)
		pr_info("[%s] :  Keun Frame Rate  Error!!!!\n", __func__);
	else
		pr_info("%s : Keun Frame Rate : 0x %02X\n", __func__, read_value);

	//AWB
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF011, &read_value, 1) != 0)
		pr_info("[%s] :  AWB Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : AWB Auto Enable : 0x %02X\n", __func__, read_value);

	//AE
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF00F, &read_value, 1) != 0)
		pr_info("[%s] :  AE Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : AE Auto Enable : 0x %02X\n", __func__, read_value);

	//Lens Shading
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF034, &read_value, 1) != 0)
		pr_info("[%s] : Lens Shading  Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : Lens Auto Enable : 0x %02X\n", __func__, read_value);

	//Defect Pixel Corection
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF00E, &read_value, 1) != 0)
		pr_info("[%s] : Defect Pixel Corection Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : Defect Pixel Corection Auto Enable : 0x %02X\n", __func__, read_value);

	//Dark Area Compendation
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF013, &read_value, 1) != 0)
		pr_info("[%s] : Dark Area Compendation Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : Dark Area Compendation Auto Enable : 0x %02X\n", __func__, read_value);

	//Output
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF00B, &read_value, 1) != 0)
		pr_info("[%s] : Output Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : Output Auto Enable : 0x %02X\n", __func__, read_value);

	//Custom AE Program
	read_value = -1;
	if(thp7212_i2c_read_data(client, 0xF020, &read_value, 1) != 0)
		pr_info("[%s] : Custom AE Program Auto Enable Error!!!!\n", __func__);
	else
		pr_info("%s : Custom AE Program Auto Enable : 0x %02X\n", __func__, read_value);

	//thp7212_i2c_write_word(client, 0x002E, 0x01A6);
	//thp7212_i2c_read_word(client, 0x0F16, &read_value);
	//thp7212_i2c_read_addr16_byte(client, 0xF001, &read_value);
//	pr_info("%s : Keun Revision : 0x %02X\n", __func__, read_value);
#endif

	return 0;
}

static int thp7212_restart(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state = container_of(sd, struct thp7212_state, sd);

	int ret = 0;

	dev_err(&client->dev, "%s: start\n", __func__);

	// Start C0 FF 00 01 Stop 
	{
		u8 buf[] = { 0xFF, 0x00, 0x01 };
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 FF 00 01 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

#if 1//def CONFIG_BB_SDK_FRAME_RATE_CONTROL
	mdelay(100);
	// Start C0 F0 1C 0x Stop 
	{
		u8 buf[] = { 0xF0, 0x1C, 0x00 }; //Frame rate select 0x00-0x02 (0x00:30fps, 0x01:25fps, 0x02:20fps)
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 F0 1C 0x Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}	
#endif

	mdelay(100);
	// Start C0 F0 0B 01 Stop 
	{
		u8 buf[] = { 0xF0, 0x0B, 0x01 }; //	Output ON/OFF 0x00-0x01 (0x00:OFF, 0x01:ON)
		ret = thp7212_i2c_write_block(sd, buf, sizeof(buf));
		if(ret < 0) {
			dev_err(&client->dev, "\e[31m%s(Start C0 F0 0B 01 Stop) i2c error\e[0m, ret = %d\n", __func__, ret);
			return ret;
		}
	}

	dev_err(&client->dev, "%s: end\n", __func__);

	return 0;
}

static int thp7212_s_config(struct v4l2_subdev *sd,
			int irq, void *platform_data)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	/* struct thp7212_platform_data *pdata = client->dev.platform_data; */

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */

#if 0
	state->pix.width = pdata->default_width;
	state->pix.height = pdata->default_height;
	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_PIX_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = DEFAULT_MCLK;	/* 24MHz default */
	else
		state->freq = pdata->freq;
#else
    state->pix.width = DEFAULT_SENSOR_WIDTH;
    state->pix.height = DEFAULT_SENSOR_HEIGHT;
    state->pix.pixelformat = DEFAULT_PIX_FMT;
    state->freq = DEFAULT_MCLK; /* 27MHz default */
#endif

	return 0;
}

static int thp7212_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err = 0;

	if (!state->initialized) {
		dev_err(&client->dev,
			"%s: return error because uninitialized\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&state->ctrl_lock);

	switch (ctrl->id) {
	case V4L2_CID_WHITE_BALANCE_PRESET:
		ctrl->value = parms->white_balance;
		break;
	case V4L2_CID_IMAGE_EFFECT:
		ctrl->value = parms->effects;
		break;
	case V4L2_CID_CAM_CONTRAST:
		ctrl->value = parms->contrast;
		break;
	case V4L2_CID_CAM_SATURATION:
		ctrl->value = parms->saturation;
		break;
	case V4L2_CID_CAM_SHARPNESS:
		ctrl->value = parms->sharpness;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		ctrl->value = state->jpeg.main_size;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		ctrl->value = state->jpeg.main_offset;
		break;
	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
		ctrl->value = state->jpeg.thumb_size;
		break;
	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
		ctrl->value = state->jpeg.thumb_offset;
		break;
	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		ctrl->value = state->jpeg.postview_offset;
		break;
#if 0
	case V4L2_CID_CAM_JPEG_MEMSIZE:
		ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE;
		break;
#endif
	case V4L2_CID_CAM_JPEG_QUALITY:
		ctrl->value = state->jpeg.quality;
		break;
	case V4L2_CID_CAM_AUTO_FOCUS_RESULT:
		err = thp7212_get_auto_focus_result(sd, ctrl);
		break;
	case V4L2_CID_CAM_GET_ISO:
		err = thp7212_get_iso(sd, ctrl);
		break;
	case V4L2_CID_CAM_GET_SHT_TIME:
		err = thp7212_get_shutterspeed(sd, ctrl);
		break;
	case V4L2_CID_CAM_GET_FRAME_COUNT:
		err = thp7212_get_frame_count(sd, ctrl);
		break;

#if 0 //many del

	case V4L2_CID_CAM_DATE_INFO_YEAR:
		ctrl->value = 2010;
		break;
	case V4L2_CID_CAM_DATE_INFO_MONTH:
		ctrl->value = 2;
		break;
	case V4L2_CID_CAM_DATE_INFO_DATE:
		ctrl->value = 25;
		break;
	case V4L2_CID_CAM_SENSOR_VER:
		ctrl->value = 1;
		break;
	case V4L2_CID_CAM_FW_MINOR_VER:
		ctrl->value = state->fw.minor;
		break;
	case V4L2_CID_CAM_FW_MAJOR_VER:
		ctrl->value = state->fw.major;
		break;
	case V4L2_CID_CAM_PRM_MINOR_VER:
		ctrl->value = state->prm.minor;
		break;
	case V4L2_CID_CAM_PRM_MAJOR_VER:
		ctrl->value = state->prm.major;
		break;
	case V4L2_CID_ESD_INT:
		thp7212_get_esd_int(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_GET_FLASH_ONOFF:
		ctrl->value = state->flash_state_on_previous_capture;
		break;
#endif

	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
	case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
		break;
	default:
		err = -ENOIOCTLCMD;
		dev_err(&client->dev, "%s: unknown ctrl id 0x%x\n",
			__func__, ctrl->id);
		break;
	}

	mutex_unlock(&state->ctrl_lock);

	return err;
}

static int thp7212_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	struct sec_cam_parm *stored_parms =
		(struct sec_cam_parm *)&state->stored_parm.parm.raw_data;
	int err = 0;
	int value = ctrl->value;

	dev_err(&client->dev, "%s: V4l2 control ID = 0x%08x, val = %d\n",
			__func__,
			(unsigned int)(ctrl->id - V4L2_CID_PRIVATE_BASE),
			value);

	/* It is the temp code to be free to the condition of line 2288 */
	/* Initial squence should be changed */
	state->initialized = true;

	if (!state->initialized &&
		(ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE)) {
		dev_err(&client->dev,
			"%s: return error because uninitialized\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&state->ctrl_lock);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FLASH_MODE:
		err = thp7212_set_flash_mode(sd, value);
		dev_err(&client->dev, "V4L2_CID_CAMERA_FLASH_MODE\n");
		break;
	case V4L2_CID_CAM_BRIGHTNESS:
		dev_err(&client->dev, "V4L2_CID_CAM_BRIGHTNESS\n");
		if (state->runmode == THP7212_RUNMODE_RUNNING) {
			value+=2;
			err = thp7212_set_parameter(sd, &parms->brightness,
						value, "brightness",
						state->regs->ev,
						ARRAY_SIZE(state->regs->ev));
		} else {
			dev_err(&client->dev,
				"%s: trying to set brightness when not "
				"in preview mode\n",
				__func__);
			stored_parms->brightness = value;
			err = 0;
		}
		break;
	case V4L2_CID_WHITE_BALANCE_PRESET:
		dev_err(&client->dev, "V4L2_CID_WHITE_BALANCE_PRESET\n");
		if (state->runmode == THP7212_RUNMODE_RUNNING) {
			err = thp7212_set_parameter(sd, &parms->white_balance,
					value, "white balance",
					state->regs->white_balance,
					ARRAY_SIZE(state->regs->white_balance));
		} else {
			dev_err(&client->dev,
				"%s: trying to set white balance when not "
				"in preview mode\n",
				__func__);
			stored_parms->white_balance = value;
			err = 0;
		}
		break;
	case V4L2_CID_IMAGE_EFFECT:
		dev_err(&client->dev, "V4L2_CID_CAMERA_EFFECT\n");
		if (state->runmode == THP7212_RUNMODE_RUNNING) {
			err = thp7212_set_parameter(sd, &parms->effects,
					value, "effects", state->regs->effect,
					ARRAY_SIZE(state->regs->effect));
		} else {
			dev_err(&client->dev,
				"%s: trying to set effect when not "
				"in preview mode\n",
				__func__);
			stored_parms->effects = value;
			err = 0;
		}
		break;
	case V4L2_CID_CAM_ISO:
		dev_err(&client->dev, "V4L2_CID_CAMERA_ISO\n");
		if (state->runmode == THP7212_RUNMODE_RUNNING) {
			err = thp7212_set_parameter(sd, &parms->iso,
						value, "iso",
						state->regs->iso,
						ARRAY_SIZE(state->regs->iso));
		} else {
			dev_err(&client->dev,
				"%s: trying to set iso when not "
				"in preview mode\n",
				__func__);
			stored_parms->iso = value;
			err = 0;
		}
		break;
	case V4L2_CID_CAM_METERING:
		dev_err(&client->dev, "V4L2_CID_CAM_METERING\n");
		if (state->runmode == THP7212_RUNMODE_RUNNING) {
			err = thp7212_set_parameter(sd, &parms->metering,
					value, "metering",
					state->regs->metering,
					ARRAY_SIZE(state->regs->metering));
		} else {
			dev_err(&client->dev,
				"%s: trying to set metering when not "
				"in preview mode\n",
				__func__);
			stored_parms->metering = value;
			err = 0;
		}
		break;
	case V4L2_CID_CAM_CONTRAST:
		dev_err(&client->dev, "V4L2_CID_CAMERA_CONTRAST\n");
		err = thp7212_set_parameter(sd, &parms->contrast,
					value, "contrast",
					state->regs->contrast,
					ARRAY_SIZE(state->regs->contrast));
		break;
	case V4L2_CID_CAM_SATURATION:
		dev_err(&client->dev, "V4L2_CID_CAMERA_SATURATION\n");
		err = thp7212_set_parameter(sd, &parms->saturation,
					value, "saturation",
					state->regs->saturation,
					ARRAY_SIZE(state->regs->saturation));
		break;
	case V4L2_CID_CAM_SHARPNESS:
		dev_err(&client->dev, "V4L2_CID_CAMERA_SHARPNESS\n");
		err = thp7212_set_parameter(sd, &parms->sharpness,
					value, "sharpness",
					state->regs->sharpness,
					ARRAY_SIZE(state->regs->sharpness));
		break;
	case V4L2_CID_CAM_WDR:
		err = thp7212_set_wdr(sd, value);
		break;
	case V4L2_CID_CAM_FACE_DETECTION:
		err = thp7212_set_face_detection(sd, value);
		break;
	case V4L2_CID_FOCUS_MODE:
		dev_err(&client->dev, "V4L2_CID_FOCUS_MODE\n");
		err = thp7212_set_focus_mode(sd, value);
		break;
	case V4L2_CID_JPEG_QUALITY:
		dev_err(&client->dev, "V4L2_CID_JPEG_QUALITY\n");
		if (state->runmode == THP7212_RUNMODE_RUNNING) {
			state->jpeg.quality = value;
			err = thp7212_set_jpeg_quality(sd);
		} else {
			dev_err(&client->dev,
				"%s: trying to set jpeg quality when not "
				"in preview mode\n",
				__func__);
			err = 0;
		}
		break;
	case V4L2_CID_SCENEMODE:
		dev_err(&client->dev, "V4L2_CID_SCENEMODE\n");
		err = thp7212_set_parameter(sd, &parms->scene_mode,
					SCENE_MODE_NONE, "scene_mode",
					state->regs->scene_mode,
					ARRAY_SIZE(state->regs->scene_mode));

		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed to set scene-mode default value\n",
				__func__);
			break;
		}
		if (value != SCENE_MODE_NONE) {
			err = thp7212_set_parameter(sd, &parms->scene_mode,
					value, "scene_mode",
					state->regs->scene_mode,
					ARRAY_SIZE(state->regs->scene_mode));
		}
		if (parms->scene_mode == SCENE_MODE_NIGHTSHOT) {
			state->one_frame_delay_ms =
				NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
		} else {
			state->one_frame_delay_ms =
				NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
		}
		stored_parms->scene_mode = value;

		break;
	case V4L2_CID_CAMERA_GPS_LATITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_LATITUDE: not implemented\n",
			__func__);
		break;
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_LONGITUDE: not implemented\n",
			__func__);
		break;
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_TIMESTAMP: not implemented\n",
			__func__);
		break;
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_ALTITUDE: not implemented\n",
			__func__);
		break;
	case V4L2_CID_CAM_OBJECT_POSITION_X:
		state->position.x = value;
		break;
	case V4L2_CID_CAM_OBJECT_POSITION_Y:
		state->position.y = value;
		break;
	case V4L2_CID_CAM_CAF_START_STOP:
		dev_err(&client->dev, "V4L2_CID_CAM_CAF_START_STOP\n");
		if (value == V4L2_CAF_START) {
			err = thp7212_start_continuous_auto_focus(sd);
		} else if (value == V4L2_CAF_STOP) {
			err = thp7212_stop_continuous_auto_focus(sd);

		} else {
			err = -EINVAL;
			dev_err(&client->dev,
				"%s: bad focus value requestion %d\n",
				__func__, value);
		}
		break;
	case V4L2_CID_CAM_SET_AUTO_FOCUS:
		dev_err(&client->dev, "V4L2_CID_CAM_SET_AUTO_FOCUS\n");
		if (value == V4L2_AUTO_FOCUS_ON) {
			err = thp7212_start_auto_focus(sd);
		} else if (value == V4L2_AUTO_FOCUS_OFF) {
			err = thp7212_stop_auto_focus(sd);

		} else {
			err = -EINVAL;
			dev_err(&client->dev,
				"%s: bad focus value requestion %d\n",
				__func__, value);
		}
		break;
	case V4L2_CID_CAM_FRAME_RATE:
		dev_err(&client->dev, "V4L2_CID_CAM_SET_FRAME_RATE\n");

		parms->capture.timeperframe.numerator = 1;
		parms->capture.timeperframe.denominator = value;

		dev_err(&client->dev,
			"%s: camera frame rate request for %d fps\n",
			__func__, value);

		err = thp7212_set_parameter(sd, &parms->fps,
					value, "fps",
					state->regs->fps,
					ARRAY_SIZE(state->regs->fps));
		break;
	case V4L2_CID_CAPTURE:
		dev_err(&client->dev, "V4L2_CID_CAPTURE\n");
		err = thp7212_start_capture(sd);
		break;

	case V4L2_CID_CAM_ZOOM:
		dev_err(&client->dev, "V4L2_CID_CAM_ZOOM\n");
		err = thp7212_set_zoom(sd, value);
		break;

	/* Used to start / stop preview operation.
	 * This call can be modified to START/STOP operation,
	 * which can be used in image capture also
	 */
	case V4L2_CID_CAM_PREVIEW_ONOFF:
		dev_err(&client->dev, "V4L2_CID_CAM_PREVIEW_ONOFF\n");
		if (value)
			err = thp7212_set_preview_start(sd);
		else
			err = thp7212_set_preview_stop(sd);
		break;
	case V4L2_CID_CAMERA_CHECK_DATALINE:
		dev_dbg(&client->dev, "%s: check_dataline set to %d\n",
			__func__, value);
		state->check_dataline = value;
		break;
	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
		err = thp7212_check_dataline_stop(sd);
		break;
	case V4L2_CID_CAM_AEAWB_LOCK_UNLOCK:
		dev_err(&client->dev, "V4L2_CID_CAM_AEAWB_LOCK_UNLOCK\n");
		dev_err(&client->dev,
			"%s: ae_awb lock_unlock %d (0:unlock, 1:ae lock, 2:awb lock, 3:lock) /n",
			__func__, value);
		err = thp7212_set_parameter(sd, &parms->aeawb_lockunlock,
					value, "aeawb_lockunlock",
					state->regs->aeawb_lockunlock,
					ARRAY_SIZE(state->regs->aeawb_lockunlock));
///	case V4L2_CID_CAMERA_RETURN_FOCUS:
///		if (parms->focus_mode != FOCUS_MODE_MACRO)
///			err = thp7212_return_focus(sd);
///		break;
	default:
		dev_err(&client->dev, "%s: unknown set ctrl id 0x%x\n",
			__func__, ctrl->id);
		err = -ENOIOCTLCMD;
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: videoc_s_ctrl failed %d\n", __func__,
			err);

	mutex_unlock(&state->ctrl_lock);

	dev_dbg(&client->dev, "%s: videoc_s_ctrl returning %d\n",
		__func__, err);

	return err;
}

static int thp7212_s_ext_ctrl(struct v4l2_subdev *sd,
			      struct v4l2_ext_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	int err = 0;
	struct gps_info_common *tempGPSType = NULL;

    /* for compile warning: array subscript is above array bounds */
    u32 *preserved = ctrl->reserved2;
    preserved++;
    tempGPSType = (struct gps_info_common *)preserved;

	switch (ctrl->id) {

	case V4L2_CID_CAMERA_GPS_LATITUDE:
		/* tempGPSType = (struct gps_info_common *)ctrl->reserved2[1]; */
		state->gps_info.gps_buf[0] = tempGPSType->direction;
		state->gps_info.gps_buf[1] = tempGPSType->dgree;
		state->gps_info.gps_buf[2] = tempGPSType->minute;
		state->gps_info.gps_buf[3] = tempGPSType->second;
		break;
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
		/* tempGPSType = (struct gps_info_common *)ctrl->reserved2[1]; */
		state->gps_info.gps_buf[4] = tempGPSType->direction;
		state->gps_info.gps_buf[5] = tempGPSType->dgree;
		state->gps_info.gps_buf[6] = tempGPSType->minute;
		state->gps_info.gps_buf[7] = tempGPSType->second;
		break;
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
		/* tempGPSType = (struct gps_info_common *)ctrl->reserved2[1]; */
		state->gps_info.altitude_buf[0] = tempGPSType->direction;
		state->gps_info.altitude_buf[1] =
					(tempGPSType->dgree) & 0x00ff;
		state->gps_info.altitude_buf[2] =
					((tempGPSType->dgree) & 0xff00) >> 8;
		state->gps_info.altitude_buf[3] = tempGPSType->minute;
		break;
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		/* state->gps_info.gps_timeStamp = *((int *)ctrl->reserved2[1]); */
		state->gps_info.gps_timeStamp = *preserved;
		err = 0;
		break;
	default:
		dev_err(&client->dev, "%s: unknown ctrl->id %d\n",
			__func__, ctrl->id);
		err = -ENOIOCTLCMD;
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: vidioc_s_ext_ctrl failed %d\n",
			__func__, err);

	return err;
}

static int thp7212_s_ext_ctrls(struct v4l2_subdev *sd,
				struct v4l2_ext_controls *ctrls)
{
	struct v4l2_ext_control *ctrl = ctrls->controls;
	int ret = 0;
	int i;

	for (i = 0; i < ctrls->count; i++, ctrl++) {
		ret = thp7212_s_ext_ctrl(sd, ctrl);

		if (ret) {
			ctrls->error_idx = i;
			break;
		}
	}

	return ret;
}

static const struct v4l2_subdev_core_ops thp7212_core_ops = {
	.s_power		= thp7212_s_power,
	.init 			= thp7212_init,/* initializing API */
	///.s_config 		= thp7212_s_config,	/* Fetch platform data *///many del
	.g_ctrl 		= thp7212_g_ctrl,
	.s_ctrl 		= thp7212_s_ctrl,
	.s_ext_ctrls 		= thp7212_s_ext_ctrls,
};


static int thp7212_s_mbus_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_err(&client->dev, "%s: \n", __func__);

#if 0//many del
	dev_err(&client->dev, "%s: pixelformat = 0x%x (%c%c%c%c),"
		" colorspace = 0x%x, width = %d, height = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat >> 8,
		fmt->fmt.pix.pixelformat >> 16,
		fmt->fmt.pix.pixelformat >> 24,
		fmt->fmt.pix.colorspace,
		fmt->fmt.pix.width, fmt->fmt.pix.height);

	if (fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG &&
		fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG) {
		dev_err(&client->dev,
			"%s: mismatch in pixelformat and colorspace\n",
			__func__);
		return -EINVAL;
	}
#endif

	if (fmt->width < fmt->height) {
		int temp;
		temp  = fmt->width;
		fmt->width = fmt->height;
		fmt->height = temp;
	}

	state->pix.width = fmt->width;
	state->pix.height = fmt->height;
	//state->pix.pixelformat = fmt->fmt.pix.pixelformat;
#if 1
//	if (fmt->colorspace == V4L2_COLORSPACE_JPEG) {
	if (state->oprmode == THP7212_OPRMODE_IMAGE) {
#else
	if((state->pix.width == 2560) && (state->pix.height == 1920)){
#endif
		state->oprmode = THP7212_OPRMODE_IMAGE;
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * use the next higher image resolution. */
		thp7212_set_framesize(sd, thp7212_capture_framesize_list,
				ARRAY_SIZE(thp7212_capture_framesize_list),
				false);

	} else {
		state->oprmode = THP7212_OPRMODE_VIDEO;
		/*
		 * In case of video mode,
		 * if the given video resolution is not matching, use
		 * the default rate (currently THP7212_PREVIEW_WVGA).
		 */
#if 1
		thp7212_set_framesize(sd, thp7212_preview_framesize_list,
				ARRAY_SIZE(thp7212_preview_framesize_list),
				true);
#else
		thp7212_set_framesize(sd, thp7212_capture_framesize_list,
				ARRAY_SIZE(thp7212_capture_framesize_list),
				false);
#endif
	}

	state->jpeg.enable = state->pix.pixelformat == V4L2_PIX_FMT_JPEG;

	return 0;
}

static int thp7212_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	/* The camera interface should read this value, this is the resolution
	 * at which the sensor would provide framedata to the camera i/f
	 *
	 * In case of image capture,
	 * this returns the default camera resolution (SVGA)
	 */

	 dev_err(&client->dev, "%s: wid=%d\t height=%d\n", __func__,state->pix.width,state->pix.height);
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = state->pix.width;
	fsize->discrete.height = state->pix.height;

	return 0;
}

static int thp7212_enum_fmt(struct v4l2_subdev *sd,
			struct v4l2_fmtdesc *fmtdesc)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	pr_debug("%s: index = %d\n", __func__, fmtdesc->index);

	dev_err(&client->dev, "%s: \n", __func__);
	if (fmtdesc->index >= ARRAY_SIZE(capture_fmts))
		return -EINVAL;

	memcpy(fmtdesc, &capture_fmts[fmtdesc->index], sizeof(*fmtdesc));

	return 0;
}

static int thp7212_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int num_entries;
	int i;

	num_entries = ARRAY_SIZE(capture_fmts);

	pr_err("%s: pixelformat = 0x%x (%c%c%c%c), num_entries = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat >> 8,
		fmt->fmt.pix.pixelformat >> 16,
		fmt->fmt.pix.pixelformat >> 24,
		num_entries);

	for (i = 0; i < num_entries; i++) {
		if (capture_fmts[i].pixelformat == fmt->fmt.pix.pixelformat) {
			pr_debug("%s: match found, returning 0\n", __func__);
			return 0;
		}
	}

	pr_debug("%s: no match found, returning -EINVAL\n", __func__);
	return -EINVAL;
}

static int thp7212_g_parm(struct v4l2_subdev *sd,
			struct v4l2_streamparm *param)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	memcpy(param, &state->strm, sizeof(param));
	return 0;
}

static int thp7212_s_parm(struct v4l2_subdev *sd,
			struct v4l2_streamparm *param)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct sec_cam_parm *new_parms =
		(struct sec_cam_parm *)&param->parm.raw_data;
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	dev_err(&client->dev, "%s: start\n", __func__);

	if (param->parm.capture.timeperframe.numerator !=
		parms->capture.timeperframe.numerator ||
		param->parm.capture.timeperframe.denominator !=
		parms->capture.timeperframe.denominator) {

		int fps = 0;
		int fps_max = 30;

		if (param->parm.capture.timeperframe.numerator &&
			param->parm.capture.timeperframe.denominator)
			fps =
			    (int)(param->parm.capture.timeperframe.denominator /
				  param->parm.capture.timeperframe.numerator);
		else
			fps = 0;

		if (fps <= 0 || fps > fps_max) {
			dev_err(&client->dev,
				"%s: Framerate %d not supported,"
				" setting it to %d fps.\n",
				__func__, fps, fps_max);
			fps = fps_max;
		}

		/*
		 * Don't set the fps value, just update it in the state
		 * We will set the resolution and
		 * fps in the start operation (preview/capture) call
		 */
		parms->capture.timeperframe.numerator = 1;
		parms->capture.timeperframe.denominator = fps;
	}

	/* we return an error if one happened but don't stop trying to
	 * set all parameters passed
	 */
	err = thp7212_set_parameter(sd, &parms->contrast, new_parms->contrast,
				"contrast", state->regs->contrast,
				ARRAY_SIZE(state->regs->contrast));
	err |= thp7212_set_parameter(sd, &parms->effects, new_parms->effects,
				"effect", state->regs->effect,
				ARRAY_SIZE(state->regs->effect));
	err |= thp7212_set_parameter(sd, &parms->brightness,
				new_parms->brightness, "brightness",
				state->regs->ev, ARRAY_SIZE(state->regs->ev));
///	err |= thp7212_set_flash_mode(sd, new_parms->flash_mode);
///	err |= thp7212_set_focus_mode(sd, new_parms->focus_mode);
	err |= thp7212_set_parameter(sd, &parms->iso, new_parms->iso,
				"iso", state->regs->iso,
				ARRAY_SIZE(state->regs->iso));
	err |= thp7212_set_parameter(sd, &parms->metering, new_parms->metering,
				"metering", state->regs->metering,
				ARRAY_SIZE(state->regs->metering));
	err |= thp7212_set_parameter(sd, &parms->saturation,
				new_parms->saturation, "saturation",
				state->regs->saturation,
				ARRAY_SIZE(state->regs->saturation));
	err |= thp7212_set_parameter(sd, &parms->scene_mode,
				new_parms->scene_mode, "scene_mode",
				state->regs->scene_mode,
				ARRAY_SIZE(state->regs->scene_mode));
	err |= thp7212_set_parameter(sd, &parms->sharpness,
				new_parms->sharpness, "sharpness",
				state->regs->sharpness,
				ARRAY_SIZE(state->regs->sharpness));
	err |= thp7212_set_parameter(sd, &parms->aeawb_lockunlock,
				new_parms->aeawb_lockunlock, "aeawb_lockunlock",
				state->regs->aeawb_lockunlock,
				ARRAY_SIZE(state->regs->aeawb_lockunlock));
	err |= thp7212_set_parameter(sd, &parms->white_balance,
				new_parms->white_balance, "white balance",
				state->regs->white_balance,
				ARRAY_SIZE(state->regs->white_balance));
	err |= thp7212_set_parameter(sd, &parms->fps,
				new_parms->fps, "fps",
				state->regs->fps,
				ARRAY_SIZE(state->regs->fps));

	if (parms->scene_mode == SCENE_MODE_NIGHTSHOT)
		state->one_frame_delay_ms = NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
	else
		state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;

	dev_dbg(&client->dev, "%s: returning %d\n", __func__, err);
	return err;
}

static int thp7212_s_stream(struct v4l2_subdev *sd, int enable)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	struct thp7212_state *state = container_of(sd, struct thp7212_state, sd);

	PM_DBGOUT("%s: enable %d\n", __func__, enable);

	if (enable)	/* stream on */
	{
#if 0//def USE_PRESET
		PM_DBGOUT("%s %d\n", __func__, enable);

		if (!state->initialized) {
			PM_DBGOUT("initialize!!!\n");
			state->initialized = true;
			//thp7212_set_preset(sd, thp7212_reg_init, ARRAY_SIZE(thp7212_reg_init));
		}
#if 0
		/* iso auto */
		thp7212_set_preset(sd, thp7212_reg_iso_auto, ARRAY_SIZE(thp7212_reg_iso_auto));
		/* effect normal */
		thp7212_set_preset(sd, thp7212_reg_effect_normal, ARRAY_SIZE(thp7212_reg_effect_normal));
		/* wb auto */
		thp7212_set_preset(sd, thp7212_reg_wb_auto, ARRAY_SIZE(thp7212_reg_wb_auto));
#endif

		/* preview size */
		switch (state->pix.width) {
			case 1024: // 1024x768
				//thp7212_set_preset(sd, thp7212_1024_preview, ARRAY_SIZE(thp7212_1024_preview));
				break;

			case 1920: // 1920x1080
				//thp7212_set_preset(sd, thp7212_1920_preview, ARRAY_SIZE(thp7212_1920_preview));
				break;

			case 2048: // 2048x1536
				//thp7212_set_preset(sd, thp7212_2048_preview, ARRAY_SIZE(thp7212_2048_preview));
				break;

			case 2560: // 2560x1920
				//thp7212_set_preset(sd, thp7212_2560_preview, ARRAY_SIZE(thp7212_2560_preview));
				break;

			default:
				//thp7212_set_preset(sd, thp7212_reg_preview_template, ARRAY_SIZE(thp7212_reg_preview_template));
				break;
		}
#else
		//added by keun 2015.05.07
		if( !camera_initialized )
		{
			printk(KERN_INFO "Camera FW Initialization....!!!\n");
			thp7212_init(sd, enable);
			camera_initialized = true;
		}
		else
		{
			printk(KERN_INFO "Camera FW is already initialized....!!!\n");
			thp7212_restart(sd);
		}
#endif
		state->runmode = THP7212_RUNMODE_RUNNING;
	} 
	else 
	{
#if 0
		/* TODO: implementation stream off */
		if (thp7212_set_from_table(sd, "init reg 4", &state->regs->init_reg_4, 1, 0) < 0)
			return -EIO;
		if (state->runmode == THP7212_RUNMODE_RUNNING)
			state->runmode = THP7212_RUNMODE_IDLE;
#endif
	}

	return 0;
}

static const struct v4l2_subdev_video_ops thp7212_video_ops = {
	//.g_mbus_fmt 		= thp7212_g_mbus_fmt,		//hwang: to fix
	.s_mbus_fmt 		= thp7212_s_mbus_fmt,//many s_fmt->s_mbus_fmt
	.enum_framesizes 	= thp7212_enum_framesizes,
	///.enum_fmt 		= thp7212_enum_fmt, //many del
	///.try_fmt 		= thp7212_try_fmt, //many del
	.g_parm 		= thp7212_g_parm,
	.s_parm 		= thp7212_s_parm,
	.s_stream 		= thp7212_s_stream,
};

/**
 * __find_oprmode - Lookup THP7212 resolution type according to pixel code
 * @code: pixel code
 */
static enum thp7212_oprmode __find_oprmode(enum v4l2_mbus_pixelcode code)
{
	enum thp7212_oprmode type = THP7212_OPRMODE_VIDEO;

	do {
		if (code == default_fmt[type].code)
			return type;
	} while (type++ != SIZE_DEFAULT_FFMT);

	return 0;
}

/**
 * __find_resolution - Lookup preset and type of M-5MOLS's resolution
 * @mf: pixel format to find/negotiate the resolution preset for
 * @type: M-5MOLS resolution type
 * @resolution:	M-5MOLS resolution preset register value
 *
 * Find nearest resolution matching resolution preset and adjust mf
 * to supported values.
 */
static int __find_resolution(struct v4l2_subdev *sd,
			     struct v4l2_mbus_framefmt *mf,
			     enum thp7212_oprmode *type,
			     u32 *resolution)
{
	const struct thp7212_resolution *fsize = &thp7212_resolutions[0];
	const struct thp7212_resolution *match = NULL;
	enum thp7212_oprmode stype = __find_oprmode(mf->code);
	int i = ARRAY_SIZE(thp7212_resolutions);
	unsigned int min_err = ~0;
	int err;

	while (i--) {
		if (stype == fsize->type) {
			err = abs(fsize->width - mf->width)
				+ abs(fsize->height - mf->height);

			if (err < min_err) {
				min_err = err;
				match = fsize;
				stype = fsize->type;
			}
		}
		fsize++;
	}
	printk("LINE(%d): mf width: %d, mf height: %d, mf code: %d\n", __LINE__,
		mf->width, mf->height, stype);
	printk("LINE(%d): match width: %d, match height: %d, match code: %d\n", __LINE__,
		match->width, match->height, stype);
	if (match) {
		mf->width  = match->width;
		mf->height = match->height;
		*resolution = match->value;
		*type = stype;
		return 0;
	}
	printk("LINE(%d): mf width: %d, mf height: %d, mf code: %d\n", __LINE__,
		mf->width, mf->height, stype);

	return -EINVAL;
}

static struct v4l2_mbus_framefmt *__find_format(struct thp7212_state *state,
				struct v4l2_subdev_fh *fh,
				enum v4l2_subdev_format_whence which,
				enum thp7212_oprmode type)
{
	if (which == V4L2_SUBDEV_FORMAT_TRY)
		return fh ? v4l2_subdev_get_try_format(fh, 0) : NULL;

	return &state->ffmt[type];
}

/* enum code by flite video device command */
static int thp7212_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_fh *fh,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	if (!code || code->index >= SIZE_DEFAULT_FFMT)
		return -EINVAL;

	code->code = default_fmt[code->index].code;

	return 0;
}

/* get format by flite video device command */
static int thp7212_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
			  struct v4l2_subdev_format *fmt)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct v4l2_mbus_framefmt *format;

	if (fmt->pad != 0)
		return -EINVAL;

	format = __find_format(state, fh, fmt->which, state->res_type);
	if (!format)
		return -EINVAL;

	fmt->format = *format;
	return 0;
}

/* set format by flite video device command */
static int thp7212_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
			  struct v4l2_subdev_format *fmt)
{
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);
	struct v4l2_mbus_framefmt *format = &fmt->format;
	struct v4l2_mbus_framefmt *sfmt;
	enum thp7212_oprmode type;
	u32 resolution = 0;
	int ret;

	if (fmt->pad != 0)
		return -EINVAL;

	ret = __find_resolution(sd, format, &type, &resolution);
	if (ret < 0)
		return ret;

	sfmt = __find_format(state, fh, fmt->which, type);
	if (!sfmt)
		return 0;
	sfmt		= &default_fmt[type];
	sfmt->width	= format->width;
	sfmt->height	= format->height;

	if (fmt->which == V4L2_SUBDEV_FORMAT_ACTIVE) {
		/* for enum size of entity by flite */
		state->oprmode  		= type;
		state->ffmt[type].width 	= format->width;
		state->ffmt[type].height 	= format->height;
#ifndef CONFIG_VIDEO_THP7212_SENSOR_JPEG
		state->ffmt[type].code 		= V4L2_MBUS_FMT_YUYV8_2X8;
#else
		state->ffmt[type].code 		= format->code;
#endif

		/* find adaptable resolution */
		state->resolution 		= resolution;
#ifndef CONFIG_VIDEO_THP7212_SENSOR_JPEG
		state->code 			= V4L2_MBUS_FMT_YUYV8_2X8;
#else
		state->code 			= format->code;
#endif
		state->res_type 		= type;

		/* for set foramat */
		state->pix.width 		= format->width;
		state->pix.height 		= format->height;

		if (state->power_on == THP7212_HW_POWER_ON)
			thp7212_s_mbus_fmt(sd, sfmt);  /* set format */
	}

	return 0;
}


static struct v4l2_subdev_pad_ops thp7212_pad_ops = {
	.enum_mbus_code	= thp7212_enum_mbus_code,
	.get_fmt		= thp7212_get_fmt,
	.set_fmt		= thp7212_set_fmt,
};

static const struct v4l2_subdev_ops thp7212_ops = {
	.core = &thp7212_core_ops,
	.video = &thp7212_video_ops,
	.pad	= &thp7212_pad_ops,
};

static int thp7212_link_setup(struct media_entity *entity,
			    const struct media_pad *local,
			    const struct media_pad *remote, u32 flags)
{
	printk("%s\n", __func__);
	return 0;
}

static const struct media_entity_operations thp7212_media_ops = {
	.link_setup = thp7212_link_setup,
};

/* internal ops for media controller */
static int thp7212_init_formats(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct v4l2_subdev_format format;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_err(&client->dev, "%s: \n", __func__);
	memset(&format, 0, sizeof(format));
	format.pad = 0;
	format.which = fh ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE;
	format.format.code = DEFAULT_SENSOR_CODE;
	format.format.width = DEFAULT_SENSOR_WIDTH;
	format.format.height = DEFAULT_SENSOR_HEIGHT;

	//thp7212_set_fmt(sd, fh, &format);

//	thp7212_s_parm(sd, &state->strm);

	return 0;
}

static int thp7212_subdev_close(struct v4l2_subdev *sd,
			      struct v4l2_subdev_fh *fh)
{
	thp7212_debug(THP7212_DEBUG_I2C, "%s", __func__);
	printk("%s", __func__);
	return 0;
}

static int thp7212_subdev_registered(struct v4l2_subdev *sd)
{
	thp7212_debug(THP7212_DEBUG_I2C, "%s", __func__);
	return 0;
}

static void thp7212_subdev_unregistered(struct v4l2_subdev *sd)
{
	thp7212_debug(THP7212_DEBUG_I2C, "%s", __func__);
}

static const struct v4l2_subdev_internal_ops thp7212_v4l2_internal_ops = {
	.open = thp7212_init_formats,
	.close = thp7212_subdev_close,
	.registered = thp7212_subdev_registered,
	.unregistered = thp7212_subdev_unregistered,
};


#ifdef USE_INITIAL_WORKER_THREAD
static void thp7212_init_work(struct work_struct *work)
{
    struct thp7212_state *state = container_of(work, struct thp7212_state, init_work);
    //thp7212_set_preset(&state->sd, thp7212_reg_init, ARRAY_SIZE(thp7212_reg_init));
    printk("%s\n", __func__);
    state->initialized = true;
}
#endif

/*
 * thp7212_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int thp7212_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct thp7212_state *state;
	int ret = 0;

    /* psw0523 fix : not use platform data */
#if 0
	struct thp7212_platform_data *pdata = client->dev.platform_data;
	if ((pdata == NULL) /*|| (pdata->flash_onoff == NULL)*/) {
		dev_err(&client->dev, "%s: bad platform data\n", __func__);
		return -ENODEV;
	}
#endif

	state = kzalloc(sizeof(struct thp7212_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	mutex_init(&state->ctrl_lock);
	init_completion(&state->af_complete);

	state->runmode = THP7212_RUNMODE_NOTREADY;
	sd = &state->sd;
	strcpy(sd->name, THP7212_DRIVER_NAME);

	v4l2_i2c_subdev_init(sd, client, &thp7212_ops);
	state->pad.flags = MEDIA_PAD_FL_SOURCE;
	ret = media_entity_init(&sd->entity, 1, &state->pad, 0);
	if (ret < 0) {
        dev_err(&client->dev, "%s: failed\n", __func__);
        return ret;
    }

	thp7212_init_formats(sd, NULL);

	thp7212_init_parameters(sd);

	sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
	sd->flags = V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->internal_ops = &thp7212_v4l2_internal_ops;
	sd->entity.ops = &thp7212_media_ops;

#ifdef USE_INITIAL_WORKER_THREAD
    state->init_wq = create_singlethread_workqueue("thp7212-init");
    if (!state->init_wq) {
        pr_err("%s: error create work queue for init\n", __func__);
        return -ENOMEM;
    }
    INIT_WORK(&state->init_work, thp7212_init_work);
#endif

	/* dev_dbg(&client->dev, "5MP camera THP7212 loaded.\n"); */
	printk("13MP camera THP7212 loaded.\n");

    return 0;
}

static int thp7212_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct thp7212_state *state =
		container_of(sd, struct thp7212_state, sd);

	v4l2_device_unregister_subdev(sd);
	mutex_destroy(&state->ctrl_lock);
	kfree(state);
	thp7212_power(0);
	dev_dbg(&client->dev, "Unloaded camera sensor THP7212.\n");

	return 0;
}

static const struct i2c_device_id thp7212_id[] = {
	{ THP7212_DRIVER_NAME, 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, thp7212_id);

//many v4l2_i2c_driver_data->i2c_driver / v4l2_i2c_data->thp7212_i2c_driver
static struct i2c_driver thp7212_i2c_driver = {
	.driver = {
		.name = THP7212_DRIVER_NAME,
	},
	.probe = thp7212_probe,
	.remove = thp7212_remove,
	.id_table = thp7212_id,
};

static int __init thp7212_mod_init(void)
{
	printk("many: %s\n",__func__);
	return i2c_add_driver(&thp7212_i2c_driver);
}

static void __exit thp7212_mod_exit(void)
{
	printk("many: %s\n",__func__);
	i2c_del_driver(&thp7212_i2c_driver);
}

module_init(thp7212_mod_init);
module_exit(thp7212_mod_exit);


MODULE_DESCRIPTION("THP7212 camera driver");
MODULE_AUTHOR("   <    @nexell.co.kr>");
MODULE_LICENSE("GPL");
