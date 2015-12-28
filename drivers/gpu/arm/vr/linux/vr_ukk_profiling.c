/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */
#include <linux/fs.h>       /* file system operations */
#include <asm/uaccess.h>    /* user space access */
#include <linux/slab.h>

#include "vr_ukk.h"
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_ukk_wrappers.h"

int profiling_start_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_start_s __user *uargs)
{
	_vr_uk_profiling_start_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	if (0 != copy_from_user(&kargs, uargs, sizeof(_vr_uk_profiling_start_s))) {
		return -EFAULT;
	}

	kargs.ctx = session_data;
	err = _vr_ukk_profiling_start(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	if (0 != put_user(kargs.limit, &uargs->limit)) {
		return -EFAULT;
	}

	return 0;
}

int profiling_add_event_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_add_event_s __user *uargs)
{
	_vr_uk_profiling_add_event_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	if (0 != copy_from_user(&kargs, uargs, sizeof(_vr_uk_profiling_add_event_s))) {
		return -EFAULT;
	}

	kargs.ctx = session_data;
	err = _vr_ukk_profiling_add_event(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	return 0;
}

int profiling_stop_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_stop_s __user *uargs)
{
	_vr_uk_profiling_stop_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	kargs.ctx = session_data;
	err = _vr_ukk_profiling_stop(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	if (0 != put_user(kargs.count, &uargs->count)) {
		return -EFAULT;
	}

	return 0;
}

int profiling_get_event_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_get_event_s __user *uargs)
{
	_vr_uk_profiling_get_event_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	if (0 != get_user(kargs.index, &uargs->index)) {
		return -EFAULT;
	}

	kargs.ctx = session_data;

	err = _vr_ukk_profiling_get_event(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	kargs.ctx = NULL; /* prevent kernel address to be returned to user space */
	if (0 != copy_to_user(uargs, &kargs, sizeof(_vr_uk_profiling_get_event_s))) {
		return -EFAULT;
	}

	return 0;
}

int profiling_clear_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_clear_s __user *uargs)
{
	_vr_uk_profiling_clear_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	kargs.ctx = session_data;
	err = _vr_ukk_profiling_clear(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	return 0;
}

int profiling_report_sw_counters_wrapper(struct vr_session_data *session_data, _vr_uk_sw_counters_report_s __user *uargs)
{
	_vr_uk_sw_counters_report_s kargs;
	_vr_osk_errcode_t err;
	u32 *counter_buffer;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	if (0 != copy_from_user(&kargs, uargs, sizeof(_vr_uk_sw_counters_report_s))) {
		return -EFAULT;
	}

	/* make sure that kargs.num_counters is [at least somewhat] sane */
	if (kargs.num_counters > 10000) {
		VR_DEBUG_PRINT(1, ("User space attempted to allocate too many counters.\n"));
		return -EINVAL;
	}

	counter_buffer = (u32*)kmalloc(sizeof(u32) * kargs.num_counters, GFP_KERNEL);
	if (NULL == counter_buffer) {
		return -ENOMEM;
	}

	if (0 != copy_from_user(counter_buffer, kargs.counters, sizeof(u32) * kargs.num_counters)) {
		kfree(counter_buffer);
		return -EFAULT;
	}

	kargs.ctx = session_data;
	kargs.counters = counter_buffer;

	err = _vr_ukk_sw_counters_report(&kargs);

	kfree(counter_buffer);

	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	return 0;
}


