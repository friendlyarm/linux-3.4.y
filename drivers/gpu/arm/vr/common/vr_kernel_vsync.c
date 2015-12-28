/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_ukk.h"

#if defined(CONFIG_VR400_PROFILING)
#include "vr_osk_profiling.h"
#endif

_vr_osk_errcode_t _vr_ukk_vsync_event_report(_vr_uk_vsync_event_report_s *args)
{
	_vr_uk_vsync_event event = (_vr_uk_vsync_event)args->event;
	VR_IGNORE(event); /* event is not used for release code, and that is OK */

#if defined(CONFIG_VR400_PROFILING)
	/*
	 * Manually generate user space events in kernel space.
	 * This saves user space from calling kernel space twice in this case.
	 * We just need to remember to add pid and tid manually.
	 */
	if ( event==_VR_UK_VSYNC_EVENT_BEGIN_WAIT) {
		_vr_osk_profiling_add_event(VR_PROFILING_EVENT_TYPE_SUSPEND |
		                              VR_PROFILING_EVENT_CHANNEL_SOFTWARE |
		                              VR_PROFILING_EVENT_REASON_SUSPEND_RESUME_SW_VSYNC,
		                              _vr_osk_get_pid(), _vr_osk_get_tid(), 0, 0, 0);
	}

	if (event==_VR_UK_VSYNC_EVENT_END_WAIT) {

		_vr_osk_profiling_add_event(VR_PROFILING_EVENT_TYPE_RESUME |
		                              VR_PROFILING_EVENT_CHANNEL_SOFTWARE |
		                              VR_PROFILING_EVENT_REASON_SUSPEND_RESUME_SW_VSYNC,
		                              _vr_osk_get_pid(), _vr_osk_get_tid(), 0, 0, 0);
	}
#endif

	VR_DEBUG_PRINT(4, ("Received VSYNC event: %d\n", event));
	VR_SUCCESS;
}

