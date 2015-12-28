/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_SESSION_H__
#define __VR_SESSION_H__

#include "vr_mmu_page_directory.h"
#include "vr_kernel_descriptor_mapping.h"
#include "vr_osk.h"
#include "vr_osk_list.h"

struct vr_timeline_system;
struct vr_soft_system;

/* Number of frame builder job lists per session. */
#define VR_PP_JOB_FB_LOOKUP_LIST_SIZE 16
#define VR_PP_JOB_FB_LOOKUP_LIST_MASK (VR_PP_JOB_FB_LOOKUP_LIST_SIZE - 1)

struct vr_session_data {
	_vr_osk_notification_queue_t * ioctl_queue;

	_vr_osk_mutex_t *memory_lock; /**< Lock protecting the vm manipulation */
	vr_descriptor_mapping * descriptor_mapping; /**< Mapping between userspace descriptors and our pointers */
	_vr_osk_list_t memory_head; /**< Track all the memory allocated in this session, for freeing on abnormal termination */

	struct vr_page_directory *page_directory; /**< MMU page directory for this session */

	_VR_OSK_LIST_HEAD(link); /**< Link for list of all sessions */
	_VR_OSK_LIST_HEAD(pp_job_list); /**< List of all PP jobs on this session */

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	_vr_osk_atomic_t number_of_window_jobs; /**< Record the window jobs completed on this session in a period */
#endif

	_vr_osk_list_t pp_job_fb_lookup_list[VR_PP_JOB_FB_LOOKUP_LIST_SIZE]; /**< List of PP job lists per frame builder id.  Used to link jobs from same frame builder. */

	struct vr_soft_job_system *soft_job_system; /**< Soft job system for this session. */
	struct vr_timeline_system *timeline_system; /**< Timeline system for this session. */

	vr_bool is_aborting; /**< VR_TRUE if the session is aborting, VR_FALSE if not. */
	vr_bool use_high_priority_job_queue; /**< If VR_TRUE, jobs added from this session will use the high priority job queues. */
};

_vr_osk_errcode_t vr_session_initialize(void);
void vr_session_terminate(void);

/* List of all sessions. Actual list head in vr_kernel_core.c */
extern _vr_osk_list_t vr_sessions;
/* Lock to protect modification and access to the vr_sessions list */
extern _vr_osk_spinlock_irq_t *vr_sessions_lock;

VR_STATIC_INLINE void vr_session_lock(void)
{
	_vr_osk_spinlock_irq_lock(vr_sessions_lock);
}

VR_STATIC_INLINE void vr_session_unlock(void)
{
	_vr_osk_spinlock_irq_unlock(vr_sessions_lock);
}

void vr_session_add(struct vr_session_data *session);
void vr_session_remove(struct vr_session_data *session);
u32 vr_session_get_count(void);

#define VR_SESSION_FOREACH(session, tmp, link) \
	_VR_OSK_LIST_FOREACHENTRY(session, tmp, &vr_sessions, struct vr_session_data, link)

VR_STATIC_INLINE struct vr_page_directory *vr_session_get_page_directory(struct vr_session_data *session)
{
	return session->page_directory;
}

VR_STATIC_INLINE void vr_session_send_notification(struct vr_session_data *session, _vr_osk_notification_t *object)
{
	_vr_osk_notification_queue_send(session->ioctl_queue, object);
}

/*
 * Get the max completed window jobs from all active session,
 * which will be used in  window render frame per sec calculate
 */
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
u32 vr_session_max_window_num(void);
#endif

#endif /* __VR_SESSION_H__ */
