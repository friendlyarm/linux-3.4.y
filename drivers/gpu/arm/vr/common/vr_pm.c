/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_pm.h"
#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_gp_scheduler.h"
#include "vr_pp_scheduler.h"
#include "vr_scheduler.h"
#include "vr_kernel_utilization.h"
#include "vr_group.h"
#include "vr_pm_domain.h"
#include "vr_pmu.h"

static vr_bool vr_power_on = VR_FALSE;

_vr_osk_errcode_t vr_pm_initialize(void)
{
	_vr_osk_pm_dev_enable();
	return _VR_OSK_ERR_OK;
}

void vr_pm_terminate(void)
{
	vr_pm_domain_terminate();
	_vr_osk_pm_dev_disable();
}

/* Reset GPU after power up */
static void vr_pm_reset_gpu(void)
{
	/* Reset all L2 caches */
	vr_l2_cache_reset_all();

	/* Reset all groups */
	vr_scheduler_reset_all_groups();
}

void vr_pm_os_suspend(void)
{
	VR_DEBUG_PRINT(3, ("Vr PM: OS suspend\n"));
	vr_gp_scheduler_suspend();
	vr_pp_scheduler_suspend();
	vr_utilization_suspend();
	vr_group_power_off(VR_TRUE);
	vr_power_on = VR_FALSE;
}

void vr_pm_os_resume(void)
{
	struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();
	vr_bool do_reset = VR_FALSE;

	VR_DEBUG_PRINT(3, ("Vr PM: OS resume\n"));

	if (VR_TRUE != vr_power_on) {
		do_reset = VR_TRUE;
	}

	if (NULL != pmu) {
		vr_pmu_reset(pmu);
	}

	vr_power_on = VR_TRUE;
	_vr_osk_write_mem_barrier();

	if (do_reset) {
		vr_pm_reset_gpu();
		vr_group_power_on();
	}

	vr_gp_scheduler_resume();
	vr_pp_scheduler_resume();
}

void vr_pm_runtime_suspend(void)
{
	VR_DEBUG_PRINT(3, ("Vr PM: Runtime suspend\n"));
	vr_group_power_off(VR_TRUE);
	vr_power_on = VR_FALSE;
}

void vr_pm_runtime_resume(void)
{
	struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();
	vr_bool do_reset = VR_FALSE;

	VR_DEBUG_PRINT(3, ("Vr PM: Runtime resume\n"));

	if (VR_TRUE != vr_power_on) {
		do_reset = VR_TRUE;
	}

	if (NULL != pmu) {
		vr_pmu_reset(pmu);
	}

	vr_power_on = VR_TRUE;
	_vr_osk_write_mem_barrier();

	if (do_reset) {
		vr_pm_reset_gpu();
		vr_group_power_on();
	}
}

void vr_pm_set_power_is_on(void)
{
	vr_power_on = VR_TRUE;
}

vr_bool vr_pm_is_power_on(void)
{
	return vr_power_on;
}
