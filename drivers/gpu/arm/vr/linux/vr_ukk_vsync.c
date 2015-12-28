/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
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


int vsync_event_report_wrapper(struct vr_session_data *session_data, _vr_uk_vsync_event_report_s __user *uargs)
{
	_vr_uk_vsync_event_report_s kargs;
	_vr_osk_errcode_t err;

	VR_CHECK_NON_NULL(uargs, -EINVAL);

	if (0 != copy_from_user(&kargs, uargs, sizeof(_vr_uk_vsync_event_report_s))) {
		return -EFAULT;
	}

	kargs.ctx = session_data;
	err = _vr_ukk_vsync_event_report(&kargs);
	if (_VR_OSK_ERR_OK != err) {
		return map_errcode(err);
	}

	return 0;
}

