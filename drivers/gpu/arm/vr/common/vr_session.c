/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_session.h"

_VR_OSK_LIST_HEAD(vr_sessions);
static u32 vr_session_count = 0;

_vr_osk_spinlock_irq_t *vr_sessions_lock;

_vr_osk_errcode_t vr_session_initialize(void)
{
	_VR_OSK_INIT_LIST_HEAD(&vr_sessions);

	vr_sessions_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SESSIONS);

	if (NULL == vr_sessions_lock) return _VR_OSK_ERR_NOMEM;

	return _VR_OSK_ERR_OK;
}

void vr_session_terminate(void)
{
	_vr_osk_spinlock_irq_term(vr_sessions_lock);
}

void vr_session_add(struct vr_session_data *session)
{
	vr_session_lock();
	_vr_osk_list_add(&session->link, &vr_sessions);
	vr_session_count++;
	vr_session_unlock();
}

void vr_session_remove(struct vr_session_data *session)
{
	vr_session_lock();
	_vr_osk_list_delinit(&session->link);
	vr_session_count--;
	vr_session_unlock();
}

u32 vr_session_get_count(void)
{
	return vr_session_count;
}

/*
 * Get the max completed window jobs from all active session,
 * which will be used in window render frame per sec calculate
 */
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
u32 vr_session_max_window_num(void)
{
	struct vr_session_data *session, *tmp;
	u32 max_window_num = 0;
	u32 tmp_number = 0;

	vr_session_lock();

	VR_SESSION_FOREACH(session, tmp, link) {
		tmp_number = _vr_osk_atomic_xchg(&session->number_of_window_jobs, 0);
		if (max_window_num < tmp_number) {
			max_window_num = tmp_number;
		}
	}

	vr_session_unlock();

	return max_window_num;
}
#endif
