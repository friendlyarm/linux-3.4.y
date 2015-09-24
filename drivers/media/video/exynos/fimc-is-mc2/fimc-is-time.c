#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/time.h>

#include "fimc-is-time.h"

#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME

static u32 time_count;
static u32 time1_min;
static u32 time1_max;
static u32 time1_avg;
static u32 time2_min;
static u32 time2_max;
static u32 time2_avg;
static u32 time3_min;
static u32 time3_max;
static u32 time3_avg;
static u32 time4_cur;
static u32 time4_old;
static u32 time4_avg;

void measure_init(void)
{
	time_count = 0;
	time1_min = 0;
	time1_max = 0;
	time1_avg = 0;
	time2_min = 0;
	time2_max = 0;
	time2_avg = 0;
	time3_min = 0;
	time3_max = 0;
	time3_avg = 0;
	time4_cur = 0;
	time4_old = 0;
	time4_avg = 0;
}

void measure_internal_time(struct timeval *time_queued,
	struct timeval *time_shot,
	struct timeval *time_shotdone,
	struct timeval *time_dequeued)
{
	u32 temp1, temp2, temp3;

	temp1 = (time_shot->tv_sec - time_queued->tv_sec)*1000000 +
		(time_shot->tv_usec - time_queued->tv_usec);
	temp2 = (time_shotdone->tv_sec - time_shot->tv_sec)*1000000 +
		(time_shotdone->tv_usec - time_shot->tv_usec);
	temp3 = (time_dequeued->tv_sec - time_shotdone->tv_sec)*1000000 +
		(time_dequeued->tv_usec - time_shotdone->tv_usec);

	if (!time_count) {
		time1_min = temp1;
		time1_max = temp1;
		time2_min = temp2;
		time2_max = temp2;
		time3_min = temp3;
		time3_max = temp3;
	} else {
		if (time1_min > temp1)
			time1_min = temp1;

		if (time1_max < temp1)
			time1_max = temp1;

		if (time2_min > temp2)
			time2_min = temp2;

		if (time2_max < temp2)
			time2_max = temp2;

		if (time3_min > temp3)
			time3_min = temp3;

		if (time3_max < temp3)
			time3_max = temp3;
	}

	time1_avg += temp1;
	time2_avg += temp2;
	time3_avg += temp3;

	time4_cur = time_queued->tv_sec*1000000 + time_queued->tv_usec;
	time4_avg += (time4_cur - time4_old);
	time4_old = time4_cur;

	time_count++;

	if (time_count % 33)
		return;

	printk(KERN_INFO "t1(%d,%d,%d), t2(%d,%d,%d), t3(%d,%d,%d) : %d(%dfps)",
		temp1, time1_max, time1_avg/time_count,
		temp2, time2_max, time2_avg/time_count,
		temp3, time3_max, time3_avg/time_count,
		time4_avg/33, 33000000/time4_avg);

	time4_avg = 0;
}

#endif
#endif
