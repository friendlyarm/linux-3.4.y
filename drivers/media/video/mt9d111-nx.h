/* linux/drivers/media/video/mt9d111-nx.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MT9D111_NX_H__
#define __MT9D111_NX_H__

//#include <mach/devices.h>
#include <mach/soc.h>
//#include <linux/gpio.h>
//#include <linux/io.h>
#include <linux/regulator/machine.h>

#define I2C_DELAY	0xFE	/* delay (msec) */
#define I2C_TERM		0xFF	/* end of list entry */

#define NUM_CTRLS				7

#define PREVIEW_MODE			0
#define CAPTURE_MODE			1

#define MAX_WIDTH				1600
#define MAX_HEIGHT				1200

#define MIN_EXPOSURE			-4
#define MAX_EXPOSURE			4

#define V4L2_CID_CAMERA_SCENE_MODE		(V4L2_CTRL_CLASS_CAMERA | 0x1001)
#define V4L2_CID_CAMERA_ANTI_SHAKE		(V4L2_CTRL_CLASS_CAMERA | 0x1002)
#define V4L2_CID_CAMERA_MODE_CHANGE		(V4L2_CTRL_CLASS_CAMERA | 0x1003)

#define I2C_RETRY_CNT		5


enum MT9D111_ZOOM_TYPE {
	ZPT_IDLE = 0,
	ZPT_INPUT_SAMPLES,
	ZPT_FACTORS,
	ZPT_SINGLE_STEP,
	ZPT_MOVE_TO_TARGET,
	ZPT_MOVE
};

enum {
	WB_AUTO = 0,
	WB_DAYLIGHT,
	WB_CLOUDY,
	WB_FLUORESCENT,
	WB_INCANDESCENT,
	WB_MAX
};

enum {
	COLORFX_NONE = 0,
	COLORFX_SEPIA,
	COLORFX_AQUA,
	COLORFX_MONO,
	COLORFX_NEGATIVE,
	COLORFX_SKETCH,
	COLORFX_MAX
};

enum {
	SCENE_OFF = 0,
	SCENE_PORTRAIT,
	SCENE_LANDSCAPE,
	SCENE_SPORTS,
	SCENE_NIGHTSHOT,
	SCENE_MAX
};

enum {
	ANTI_SHAKE_OFF = 0,
	ANTI_SHAKE_50Hz,
	ANTI_SHAKE_60Hz,
	ANTI_SHAKE_MAX
};

//#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/*
 * Specification
 * Parallel : ITU-R. 656/601 YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Serial : MIPI CSI2 (single lane) YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Resolution : 1280 (H) x 1024 (V)
 * Image control : Brightness, Contrast, Saturation, Sharpness, Glamour
 * Effect : Mono, Negative, Sepia, Aqua, Sketch
 * FPS : 15fps @full resolution, 30fps @VGA, 24fps @720p
 * Max. pixel clock frequency : 48MHz(upto)
 * Internal PLL (6MHz to 27MHz input frequency)
 */

struct reg_val {
	u8 page;		/* page number */
	u8 reg;			/* 8-bit register */
	u16 val;		/* 16-bit value */
};

struct mt9d111_state {
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_ctrl_handler handler;
	/* standard */
	struct v4l2_ctrl *focus;
	struct v4l2_ctrl *wb;
	struct v4l2_ctrl *color_effect;
	struct v4l2_ctrl *exposure;
	/* custom */
	struct v4l2_ctrl *scene_mode;
	struct v4l2_ctrl *anti_shake;
	struct v4l2_ctrl *mode_change;

	bool inited;
	int width;
	int height;
	int mode; // PREVIEW or CAPTURE

	/* for zoom */
	struct v4l2_rect crop;

	struct regulator *cam_core_18V;
	struct regulator *cam_io_28V;

#ifdef USE_INITIAL_WORKER_THREAD
	struct workqueue_struct *init_wq;
	struct work_struct init_work;
#endif
};

#endif
