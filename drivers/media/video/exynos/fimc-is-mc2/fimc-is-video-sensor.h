#ifndef FIMC_IS_VIDEO_SENSOR_H
#define FIMC_IS_VIDEO_SENSOR_H

#include "fimc-is-video.h"

#define VIDEO_SENSOR_READY_BUFFERS 3

struct fimc_is_video_sensor {
	struct fimc_is_video_common common;
};

int fimc_is_sensor_video_probe(void *data);

#endif
