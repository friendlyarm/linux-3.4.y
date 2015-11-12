/*
 * Samsung Exynos5 SoC series FIMC-IS driver
 *
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef FIMC_IS_DEVICE_SENSOR_H
#define FIMC_IS_DEVICE_SENSOR_H

#include "fimc-is-framemgr.h"
#include "fimc-is-interface.h"
#include "fimc-is-metadata.h"
#include "fimc-is-video.h"
#include "fimc-is-device-ischain.h"
#include "fimc-is-device-flite.h"

#define SENSOR_MAX_ENUM 10

enum fimc_is_sensor_output_entity {
	FIMC_IS_SENSOR_OUTPUT_NONE = 0,
	FIMC_IS_SENSOR_OUTPUT_FRONT,
};

struct fimc_is_enum_sensor {
	u32 sensor;
	u32 pixel_width;
	u32 pixel_height;
	u32 active_width;
	u32 active_height;
	u32 max_framerate;
	u32 csi_ch;
	u32 flite_ch;
	u32 i2c_ch;
	struct sensor_open_extended ext;
	char *setfile_name;
};

enum fimc_is_sensor_state {
	FIMC_IS_SENSOR_OPEN,
	FIMC_IS_SENSOR_FRONT_START,
	FIMC_IS_SENSOR_BACK_START
};

struct fimc_is_device_sensor {
	struct v4l2_subdev		sd;
	struct media_pad		pads;
	struct v4l2_mbus_framefmt	mbus_fmt;
	enum fimc_is_sensor_output_entity	output;
	int id_dual;			/* for dual camera scenario */
	int id_position;		/* 0 : rear camera, 1: front camera */
	u32 width;
	u32 height;
	u32 offset_x;
	u32 offset_y;

	struct fimc_is_mem		*mem;

	struct fimc_is_video_sensor	*video;
	struct fimc_is_framemgr		*framemgr;
	struct fimc_is_device_ischain   *ischain;

	struct fimc_is_enum_sensor	enum_sensor[SENSOR_MAX_ENUM];
	struct fimc_is_enum_sensor	*active_sensor;

	unsigned long			state;
	spinlock_t			slock_state;

	void *dev_data;

	struct fimc_is_device_flite	*active_flite;
	struct fimc_is_device_flite	flite0;
	struct fimc_is_device_flite	flite1;
};

int fimc_is_sensor_probe(struct fimc_is_device_sensor *this,
	struct fimc_is_video_sensor *video,
	struct fimc_is_framemgr *framemgr,
	struct fimc_is_device_ischain *ischain,
	struct fimc_is_mem *mem);
int fimc_is_sensor_open(struct fimc_is_device_sensor *this);
int fimc_is_sensor_close(struct fimc_is_device_sensor *this);
int fimc_is_sensor_s_active_sensor(struct fimc_is_device_sensor *this,
	u32 input);
int fimc_is_sensor_buffer_queue(struct fimc_is_device_sensor *sensor,
	u32 index);
int fimc_is_sensor_buffer_finish(struct fimc_is_device_sensor *this,
	u32 index);

int fimc_is_sensor_front_start(struct fimc_is_device_sensor *this);
int fimc_is_sensor_front_stop(struct fimc_is_device_sensor *this);
int fimc_is_sensor_back_start(struct fimc_is_device_sensor *this,
	struct fimc_is_video_common *video);
int fimc_is_sensor_back_stop(struct fimc_is_device_sensor *this);

int enable_mipi(void);

#endif
