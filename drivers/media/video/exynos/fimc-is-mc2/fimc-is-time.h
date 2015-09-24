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

#ifndef FIMC_IS_TIME_H
#define FIMC_IS_TIME_H

/*#define MEASURE_TIME*/
#define INTERNAL_TIME

#define TM_FLITE_STR	0
#define TM_FLITE_END	1
#define TM_SHOT		2
#define TM_SHOT_D	3
#define TM_META_D	4
#define TM_MAX_INDEX	5

#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME
void measure_init(void);
void measure_internal_time(struct timeval *time_queued,
	struct timeval *time_shot,
	struct timeval *time_shotdone,
	struct timeval *time_dequeued);
#endif
#endif

#endif
