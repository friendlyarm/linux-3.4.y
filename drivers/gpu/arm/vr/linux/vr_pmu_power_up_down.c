/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010, 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_pmu_power_up_down.c
 */

#include <linux/version.h>
#include <linux/sched.h>
#include <linux/module.h>
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_pmu.h"
#include "vr_pp_scheduler.h"
#include "linux/vr/vr_utgard.h"

/* Vr PMU power up/down APIs */

int vr_pmu_powerup(void)
{
	struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();

	VR_DEBUG_PRINT(5, ("Vr PMU: Power up\n"));

	VR_DEBUG_ASSERT_POINTER(pmu);
	if (NULL == pmu) {
		return -ENXIO;
	}

	if (_VR_OSK_ERR_OK != vr_pmu_power_up_all(pmu)) {
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL(vr_pmu_powerup);

int vr_pmu_powerdown(void)
{
	struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();

	VR_DEBUG_PRINT(5, ("Vr PMU: Power down\n"));

	VR_DEBUG_ASSERT_POINTER(pmu);
	if (NULL == pmu) {
		return -ENXIO;
	}

	if (_VR_OSK_ERR_OK != vr_pmu_power_down_all(pmu)) {
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL(vr_pmu_powerdown);

int vr_perf_set_num_pp_cores(unsigned int num_cores)
{
	return vr_pp_scheduler_set_perf_level(num_cores, VR_FALSE);
}

EXPORT_SYMBOL(vr_perf_set_num_pp_cores);
