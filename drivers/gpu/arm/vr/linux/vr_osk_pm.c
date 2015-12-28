/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_pm.c
 * Implementation of the callback functions from common power management
 */

#include <linux/sched.h>

#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif /* CONFIG_PM_RUNTIME */
#include <linux/platform_device.h>
#include <linux/version.h>
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_kernel_linux.h"

static _vr_osk_atomic_t vr_pm_ref_count;

void _vr_osk_pm_dev_enable(void)
{
	_vr_osk_atomic_init(&vr_pm_ref_count, 0);
}

void _vr_osk_pm_dev_disable(void)
{
	_vr_osk_atomic_term(&vr_pm_ref_count);
}

/* Can NOT run in atomic context */
_vr_osk_errcode_t _vr_osk_pm_dev_ref_add(void)
{
#ifdef CONFIG_PM_RUNTIME
	int err;
	VR_DEBUG_ASSERT_POINTER(vr_platform_device);
	err = pm_runtime_get_sync(&(vr_platform_device->dev));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	pm_runtime_mark_last_busy(&(vr_platform_device->dev));
#endif
	if (0 > err) {
		VR_PRINT_ERROR(("Vr OSK PM: pm_runtime_get_sync() returned error code %d\n", err));
		return _VR_OSK_ERR_FAULT;
	}
	_vr_osk_atomic_inc(&vr_pm_ref_count);
	VR_DEBUG_PRINT(4, ("Vr OSK PM: Power ref taken (%u)\n", _vr_osk_atomic_read(&vr_pm_ref_count)));
#endif
	return _VR_OSK_ERR_OK;
}

/* Can run in atomic context */
void _vr_osk_pm_dev_ref_dec(void)
{
#ifdef CONFIG_PM_RUNTIME
	VR_DEBUG_ASSERT_POINTER(vr_platform_device);
	_vr_osk_atomic_dec(&vr_pm_ref_count);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	pm_runtime_mark_last_busy(&(vr_platform_device->dev));
	pm_runtime_put_autosuspend(&(vr_platform_device->dev));
#else
	pm_runtime_put(&(vr_platform_device->dev));
#endif
	VR_DEBUG_PRINT(4, ("Vr OSK PM: Power ref released (%u)\n", _vr_osk_atomic_read(&vr_pm_ref_count)));
#endif
}

/* Can run in atomic context */
vr_bool _vr_osk_pm_dev_ref_add_no_power_on(void)
{
#ifdef CONFIG_PM_RUNTIME
	u32 ref;
	VR_DEBUG_ASSERT_POINTER(vr_platform_device);
	pm_runtime_get_noresume(&(vr_platform_device->dev));
	ref = _vr_osk_atomic_read(&vr_pm_ref_count);
	VR_DEBUG_PRINT(4, ("Vr OSK PM: No-power ref taken (%u)\n", _vr_osk_atomic_read(&vr_pm_ref_count)));
	return ref > 0 ? VR_TRUE : VR_FALSE;
#else
	return VR_TRUE;
#endif
}

/* Can run in atomic context */
void _vr_osk_pm_dev_ref_dec_no_power_on(void)
{
#ifdef CONFIG_PM_RUNTIME
	VR_DEBUG_ASSERT_POINTER(vr_platform_device);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	pm_runtime_put_autosuspend(&(vr_platform_device->dev));
#else
	pm_runtime_put(&(vr_platform_device->dev));
#endif
	VR_DEBUG_PRINT(4, ("Vr OSK PM: No-power ref released (%u)\n", _vr_osk_atomic_read(&vr_pm_ref_count)));
#endif
}

void _vr_osk_pm_dev_barrier(void)
{
#ifdef CONFIG_PM_RUNTIME
	pm_runtime_barrier(&(vr_platform_device->dev));
#endif
}
