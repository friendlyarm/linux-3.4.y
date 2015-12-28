/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file arm_core_scaling.c
 * Example core scaling policy.
 */

#include "arm_core_scaling.h"

#include <linux/vr/vr_utgard.h>
#include "vr_kernel_common.h"

#include <linux/workqueue.h>

static int num_cores_total;
static int num_cores_enabled;

static struct work_struct wq_work;

static void set_num_cores(struct work_struct *work)
{
	int err = vr_perf_set_num_pp_cores(num_cores_enabled);
	VR_DEBUG_ASSERT(0 == err);
	VR_IGNORE(err);
}

static void enable_one_core(void)
{
	if (num_cores_enabled < num_cores_total) {
		++num_cores_enabled;
		schedule_work(&wq_work);
		VR_DEBUG_PRINT(3, ("Core scaling: Enabling one more core\n"));
	}

	VR_DEBUG_ASSERT(              1 <= num_cores_enabled);
	VR_DEBUG_ASSERT(num_cores_total >= num_cores_enabled);
}

static void disable_one_core(void)
{
	if (1 < num_cores_enabled) {
		--num_cores_enabled;
		schedule_work(&wq_work);
		VR_DEBUG_PRINT(3, ("Core scaling: Disabling one core\n"));
	}

	VR_DEBUG_ASSERT(              1 <= num_cores_enabled);
	VR_DEBUG_ASSERT(num_cores_total >= num_cores_enabled);
}

static void enable_max_num_cores(void)
{
	if (num_cores_enabled < num_cores_total) {
		num_cores_enabled = num_cores_total;
		schedule_work(&wq_work);
		VR_DEBUG_PRINT(3, ("Core scaling: Enabling maximum number of cores\n"));
	}

	VR_DEBUG_ASSERT(num_cores_total == num_cores_enabled);
}

void vr_core_scaling_init(int num_pp_cores)
{
	INIT_WORK(&wq_work, set_num_cores);

	num_cores_total   = num_pp_cores;
	num_cores_enabled = num_pp_cores;

	/* NOTE: Vr is not fully initialized at this point. */
}

void vr_core_scaling_sync(int num_cores)
{
	num_cores_enabled = num_cores;
}

void vr_core_scaling_term(void)
{
	flush_scheduled_work();
}

#define PERCENT_OF(percent, max) ((int) ((percent)*(max)/100.0 + 0.5))

void vr_core_scaling_update(struct vr_gpu_utilization_data *data)
{
	/*
	 * This function implements a very trivial PP core scaling algorithm.
	 *
	 * It is _NOT_ of production quality.
	 * The only intention behind this algorithm is to exercise and test the
	 * core scaling functionality of the driver.
	 * It is _NOT_ tuned for neither power saving nor performance!
	 *
	 * Other metrics than PP utilization need to be considered as well
	 * in order to make a good core scaling algorithm.
	 */

	VR_DEBUG_PRINT(3, ("Utilization: (%3d, %3d, %3d), cores enabled: %d/%d\n", data->utilization_gpu, data->utilization_gp, data->utilization_pp, num_cores_enabled, num_cores_total));

	/* NOTE: this function is normally called directly from the utilization callback which is in
	 * timer context. */

	if (     PERCENT_OF(90, 256) < data->utilization_pp) {
		enable_max_num_cores();
	} else if (PERCENT_OF(50, 256) < data->utilization_pp) {
		enable_one_core();
	} else if (PERCENT_OF(40, 256) < data->utilization_pp) {
		/* do nothing */
	} else if (PERCENT_OF( 0, 256) < data->utilization_pp) {
		disable_one_core();
	} else {
		/* do nothing */
	}
}
