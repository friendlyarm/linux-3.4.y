/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_uk_types.h"
#include "vr_user_settings_db.h"
#include "vr_session.h"

static u32 vr_user_settings[_VR_UK_USER_SETTING_MAX];
const char *_vr_uk_user_setting_descriptions[] = _VR_UK_USER_SETTING_DESCRIPTIONS;

static void vr_user_settings_notify(_vr_uk_user_setting_t setting, u32 value)
{
	vr_bool done = VR_FALSE;

	/*
	 * This function gets a bit complicated because we can't hold the session lock while
	 * allocating notification objects.
	 */

	while (!done) {
		u32 i;
		u32 num_sessions_alloc;
		u32 num_sessions_with_lock;
		u32 used_notification_objects = 0;
		_vr_osk_notification_t **notobjs;

		/* Pre allocate the number of notifications objects we need right now (might change after lock has been taken) */
		num_sessions_alloc = vr_session_get_count();
		if (0 == num_sessions_alloc) {
			/* No sessions to report to */
			return;
		}

		notobjs = (_vr_osk_notification_t **)_vr_osk_malloc(sizeof(_vr_osk_notification_t *) * num_sessions_alloc);
		if (NULL == notobjs) {
			VR_PRINT_ERROR(("Failed to notify user space session about num PP core change (alloc failure)\n"));
			return;
		}

		for (i = 0; i < num_sessions_alloc; i++) {
			notobjs[i] = _vr_osk_notification_create(_VR_NOTIFICATION_SETTINGS_CHANGED,
			             sizeof(_vr_uk_settings_changed_s));
			if (NULL != notobjs[i]) {
				_vr_uk_settings_changed_s *data;
				data = notobjs[i]->result_buffer;

				data->setting = setting;
				data->value = value;
			} else {
				VR_PRINT_ERROR(("Failed to notify user space session about setting change (alloc failure %u)\n", i));
			}
		}

		vr_session_lock();

		/* number of sessions will not change while we hold the lock */
		num_sessions_with_lock = vr_session_get_count();

		if (num_sessions_alloc >= num_sessions_with_lock) {
			/* We have allocated enough notification objects for all the sessions atm */
			struct vr_session_data *session, *tmp;
			VR_SESSION_FOREACH(session, tmp, link) {
				VR_DEBUG_ASSERT(used_notification_objects < num_sessions_alloc);
				if (NULL != notobjs[used_notification_objects]) {
					vr_session_send_notification(session, notobjs[used_notification_objects]);
					notobjs[used_notification_objects] = NULL; /* Don't track this notification object any more */
				}
				used_notification_objects++;
			}
			done = VR_TRUE;
		}

		vr_session_unlock();

		/* Delete any remaining/unused notification objects */
		for (; used_notification_objects < num_sessions_alloc; used_notification_objects++) {
			if (NULL != notobjs[used_notification_objects]) {
				_vr_osk_notification_delete(notobjs[used_notification_objects]);
			}
		}

		_vr_osk_free(notobjs);
	}
}

void vr_set_user_setting(_vr_uk_user_setting_t setting, u32 value)
{
	vr_bool notify = VR_FALSE;

	if (setting >= _VR_UK_USER_SETTING_MAX) {
		VR_DEBUG_PRINT_ERROR(("Invalid user setting %ud\n"));
		return;
	}

	if (vr_user_settings[setting] != value) {
		notify = VR_TRUE;
	}

	vr_user_settings[setting] = value;

	if (notify) {
		vr_user_settings_notify(setting, value);
	}
}

u32 vr_get_user_setting(_vr_uk_user_setting_t setting)
{
	if (setting >= _VR_UK_USER_SETTING_MAX) {
		return 0;
	}

	return vr_user_settings[setting];
}

_vr_osk_errcode_t _vr_ukk_get_user_setting(_vr_uk_get_user_setting_s *args)
{
	_vr_uk_user_setting_t setting;
	VR_DEBUG_ASSERT_POINTER(args);

	setting = args->setting;

	if (_VR_UK_USER_SETTING_MAX > setting) {
		args->value = vr_user_settings[setting];
		return _VR_OSK_ERR_OK;
	} else {
		return _VR_OSK_ERR_INVALID_ARGS;
	}
}

_vr_osk_errcode_t _vr_ukk_get_user_settings(_vr_uk_get_user_settings_s *args)
{
	VR_DEBUG_ASSERT_POINTER(args);

	_vr_osk_memcpy(args->settings, vr_user_settings, sizeof(vr_user_settings));

	return _VR_OSK_ERR_OK;
}
