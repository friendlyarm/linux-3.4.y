#ifndef FIMC_IS_VIDEO_ISP_H
#define FIMC_IS_VIDEO_ISP_H

#include "fimc-is-video.h"

#define VIDEO_ISP_READY_BUFFERS 0

struct fimc_is_video_isp {
	struct fimc_is_video_common common;
};

int fimc_is_isp_video_probe(void *data);

#endif
