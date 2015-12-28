/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2010, 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */
#include <linux/fs.h>       /* file system operations */
#include <asm/uaccess.h>    /* user space access */

#include "vr_ukk.h"
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_ukk_wrappers.h"

int pp_start_job_wrapper(struct vr_session_data *session_data, _vr_uk_pp_start_job_s __user *uargs)
{
	_vr_osk_errcode_t err;

	/* If the job was started successfully, 0 is returned.  If there was an error, but the job
	 * was started, we return -ENOENT.  For anything else returned, the job was not started. */

	VR_CHECK_NON_NULL(uargs, -EINVAL);
	VR_CHECK_NON_NULL(session_data, -EINVAL);

	err = _vr_ukk_pp_start_job(session_data, uargs);
	if (_VR_OSK_ERR_OK != err) return map_errcode(err);

	return 0;
}

int pp_and_gp_start_job_wrapper(struct vr_session_data *session_data, _vr_uk_pp_and_gp_start_job_s __user *uargs)
{
	_vr_osk_errcode_t err;

	/* If the jobs were started successfully, 0 is returned.  If there was an error, but the
	 * jobs were started, we return -ENOENT.  For anything else returned, the jobs were not
	 * started. */

	VR_CHECK_NON_NULL(uargs, -EINVAL);
	VR_CHECK_NON_NULL(session_data, -EINVAL);

	err = _vr_ukk_pp_and_gp_start_job(session_data, uargs);
	if (_VR_OSK_ERR_OK != err) return map_errcode(err);

	return 0;
}

int pp_get_number_of_cores_wrapper(struct vr_session_data *session_data, _vr_uk_get_pp_number_of_cores_s __user *uargs)
{
	_vr_uk_get_pp_number_of_cores_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);
	VR_CHECK_NON_NULL(session_data, -EINVAL);

	kargs.ctx = session_data;

	err = _vr_ukk_get_pp_number_of_cores(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	kargs.ctx = NULL; /* prevent kernel address to be returned to user space */
	if (0 != copy_to_user(uargs, &kargs, sizeof(_vr_uk_get_pp_number_of_cores_s))) {
		return -EFAULT;
	}

	return 0;
}

int pp_get_core_version_wrapper(struct vr_session_data *session_data, _vr_uk_get_pp_core_version_s __user *uargs)
{
	_vr_uk_get_pp_core_version_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);
	VR_CHECK_NON_NULL(session_data, -EINVAL);

	kargs.ctx = session_data;
	err = _vr_ukk_get_pp_core_version(&kargs);
	if (_VR_OSK_ERR_OK != err) return map_errcode(err);

	if (0 != put_user(kargs.version, &uargs->version)) return -EFAULT;

	return 0;
}

int pp_disable_wb_wrapper(struct vr_session_data *session_data, _vr_uk_pp_disable_wb_s __user *uargs)
{
	_vr_uk_pp_disable_wb_s kargs;

	VR_CHECK_NON_NULL(uargs, -EINVAL);
	VR_CHECK_NON_NULL(session_data, -EINVAL);

	if (0 != copy_from_user(&kargs, uargs, sizeof(_vr_uk_pp_disable_wb_s))) return -EFAULT;

	kargs.ctx = session_data;
	_vr_ukk_pp_job_disable_wb(&kargs);

	return 0;
}
