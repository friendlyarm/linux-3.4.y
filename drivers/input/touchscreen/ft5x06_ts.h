/*
 * include/linux/ft5x06_touch.h
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __LINUX_FT5X0X_TS_H__
#define __LINUX_FT5X0X_TS_H__


#define FT5X0X_NAME				"ft5x0x_ts"
#define FT5X0X_PT_MAX			5

#if 0
#define SCREEN_MAX_X			480
#define SCREEN_MAX_Y			272
#define PRESS_MAX				255
#endif


//---------------------------------------------------------

enum ft5x0x_ts_regs {
	FT5X0X_REG_THGROUP			= 0x80,
	FT5X0X_REG_THPEAK			= 0x81,
//	FT5X0X_REG_THCAL					= 0x82,
//	FT5X0X_REG_THWATER					= 0x83,
//	FT5X0X_REG_THTEMP					= 0x84,
//	FT5X0X_REG_THDIFF					= 0x85,
//	FT5X0X_REG_CTRL						= 0x86,
	FT5X0X_REG_TIMEENTERMONITOR	= 0x87,
	FT5X0X_REG_PERIODACTIVE		= 0x88,
	FT5X0X_REG_PERIODMONITOR	= 0x89,
//	FT5X0X_REG_HEIGHT_B					= 0x8a,
//	FT5X0X_REG_MAX_FRAME				= 0x8b,
//	FT5X0X_REG_DIST_MOVE				= 0x8c,
//	FT5X0X_REG_DIST_POINT				= 0x8d,
//	FT5X0X_REG_FEG_FRAME				= 0x8e,
//	FT5X0X_REG_SINGLE_CLICK_OFFSET		= 0x8f,
//	FT5X0X_REG_DOUBLE_CLICK_TIME_MIN	= 0x90,
//	FT5X0X_REG_SINGLE_CLICK_TIME		= 0x91,
//	FT5X0X_REG_LEFT_RIGHT_OFFSET		= 0x92,
//	FT5X0X_REG_UP_DOWN_OFFSET			= 0x93,
//	FT5X0X_REG_DISTANCE_LEFT_RIGHT		= 0x94,
//	FT5X0X_REG_DISTANCE_UP_DOWN			= 0x95,
//	FT5X0X_REG_ZOOM_DIS_SQR				= 0x96,
//	FT5X0X_REG_RADIAN_VALUE				= 0x97,
//	FT5X0X_REG_MAX_X_HIGH				= 0x98,
//	FT5X0X_REG_MAX_X_LOW				= 0x99,
//	FT5X0X_REG_MAX_Y_HIGH				= 0x9a,
//	FT5X0X_REG_MAX_Y_LOW				= 0x9b,
//	FT5X0X_REG_K_X_HIGH					= 0x9c,
//	FT5X0X_REG_K_X_LOW					= 0x9d,
//	FT5X0X_REG_K_Y_HIGH					= 0x9e,
//	FT5X0X_REG_K_Y_LOW					= 0x9f,
	FT5X0X_REG_AUTO_CLB_MODE	= 0xa0,
//	FT5X0X_REG_LIB_VERSION_H			= 0xa1,
//	FT5X0X_REG_LIB_VERSION_L			= 0xa2,
//	FT5X0X_REG_CIPHER					= 0xa3,
//	FT5X0X_REG_MODE						= 0xa4,
	FT5X0X_REG_PMODE			= 0xa5,			/* Power Consume Mode */
	FT5X0X_REG_FIRMID			= 0xa6,
//	FT5X0X_REG_STATE					= 0xa7,
//	FT5X0X_REG_FT5201ID					= 0xa8,
	FT5X0X_REG_ERR				= 0xa9,
	FT5X0X_REG_CLB				= 0xaa,
};

// FT5X0X_REG_PMODE
#define PMODE_ACTIVE			0x00
#define PMODE_MONITOR			0x01
#define PMODE_STANDBY			0x02
#define PMODE_HIBERNATE			0x03


//---------------------------------------------------------

struct ft5x0x_event {
	int touch_point;

	u16 x[FT5X0X_PT_MAX];
	u16 y[FT5X0X_PT_MAX];

	u16 pressure;
};

struct ft5x0x_ts_data {
	struct input_dev *input_dev;
	struct ft5x0x_event event;

	uint32_t gpio_irq;
	uint32_t gpio_wakeup;
	uint32_t gpio_reset;

	int screen_max_x;
	int screen_max_y;
	int pressure_max;

	struct work_struct work;
	struct workqueue_struct *queue;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};


#endif	// __LINUX_FT5X0X_TS_H__

