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

#ifndef FIMC_IS_DEVICE_FLITE_H
#define FIMC_IS_DEVICE_FLITE_H

#define EXPECT_FRAME_START	0
#define EXPECT_FRAME_END	1

struct fimc_is_frame_info {
	u32 o_width;
	u32 o_height;
	u32 width;
	u32 height;
	u32 offs_h;
	u32 offs_v;
};

enum fimc_is_flite_state {
	/* buffer state*/
	FLITE_A_SLOT_VALID = 0,
	FLITE_B_SLOT_VALID,
	/* global state */
	FIMC_IS_FLITE_LAST_CAPTURE
};

struct fimc_is_device_flite {
	atomic_t			fcount;
	wait_queue_head_t		wait_queue;

	unsigned long			state;
	spinlock_t			slock_state;

	struct fimc_is_video_common	*video;
	u32				tasklet_param_str;
	struct tasklet_struct		tasklet_flite_str;
	u32				tasklet_param_end;
	struct tasklet_struct		tasklet_flite_end;

	u32				channel;
	u32				regs;

	struct fimc_is_framemgr		*framemgr;

	u32				work;
	u32				sw_checker;
	u32				sw_trigger;
	struct work_struct		work_queue;

	u32				private_data;
};

int fimc_is_flite_probe(struct fimc_is_device_flite *this,
	struct fimc_is_video_common *video,
	struct fimc_is_framemgr *framemgr,
	u32 channel,
	u32 data);
int fimc_is_flite_open(struct fimc_is_device_flite *this);
int fimc_is_flite_start(struct fimc_is_device_flite *this,
	struct fimc_is_frame_info *frame,
	struct fimc_is_video_common *video);
int fimc_is_flite_stop(struct fimc_is_device_flite *this);

extern u32 __iomem *notify_fcount;

#endif
