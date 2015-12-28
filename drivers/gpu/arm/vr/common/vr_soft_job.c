/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_soft_job.h"
#include "vr_osk.h"
#include "vr_osk_vr.h"
#include "vr_timeline.h"
#include "vr_session.h"
#include "vr_kernel_common.h"
#include "vr_uk_types.h"
#include "vr_scheduler.h"

VR_STATIC_INLINE void vr_soft_job_system_lock(struct vr_soft_job_system *system)
{
	VR_DEBUG_ASSERT_POINTER(system);
	_vr_osk_spinlock_irq_lock(system->lock);
	VR_DEBUG_PRINT(5, ("Vr Soft Job: soft system %p lock taken\n", system));
	VR_DEBUG_ASSERT(0 == system->lock_owner);
	VR_DEBUG_CODE(system->lock_owner = _vr_osk_get_tid());
}

VR_STATIC_INLINE void vr_soft_job_system_unlock(struct vr_soft_job_system *system)
{
	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_PRINT(5, ("Vr Soft Job: releasing soft system %p lock\n", system));
	VR_DEBUG_ASSERT(_vr_osk_get_tid() == system->lock_owner);
	VR_DEBUG_CODE(system->lock_owner = 0);
	_vr_osk_spinlock_irq_unlock(system->lock);
}

#if defined(DEBUG)
VR_STATIC_INLINE void vr_soft_job_system_assert_locked(struct vr_soft_job_system *system)
{
	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT(_vr_osk_get_tid() == system->lock_owner);
}
#define VR_ASSERT_SOFT_JOB_SYSTEM_LOCKED(system) vr_soft_job_system_assert_locked(system)
#else
#define VR_ASSERT_SOFT_JOB_SYSTEM_LOCKED(system)
#endif /* defined(DEBUG) */

struct vr_soft_job_system *vr_soft_job_system_create(struct vr_session_data *session)
{
	u32 i;
	struct vr_soft_job_system *system;
	struct vr_soft_job *job;

	VR_DEBUG_ASSERT_POINTER(session);

	system = (struct vr_soft_job_system *) _vr_osk_calloc(1, sizeof(struct vr_soft_job_system));
	if (NULL == system) {
		return NULL;
	}

	system->session = session;

	system->lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER);
	if (NULL == system->lock) {
		vr_soft_job_system_destroy(system);
		return NULL;
	}
	system->lock_owner = 0;

	_VR_OSK_INIT_LIST_HEAD(&(system->jobs_free));
	_VR_OSK_INIT_LIST_HEAD(&(system->jobs_used));

	for (i = 0; i < VR_MAX_NUM_SOFT_JOBS; ++i) {
		job = &(system->jobs[i]);
		_vr_osk_list_add(&(job->system_list), &(system->jobs_free));
		job->system = system;
		job->state = VR_SOFT_JOB_STATE_FREE;
		job->id = i;
	}

	return system;
}

void vr_soft_job_system_destroy(struct vr_soft_job_system *system)
{
	VR_DEBUG_ASSERT_POINTER(system);

	/* All jobs should be free at this point. */
	VR_DEBUG_CODE( {
		u32 i;
		struct vr_soft_job *job;

		for (i = 0; i < VR_MAX_NUM_SOFT_JOBS; ++i)
		{
			job = &(system->jobs[i]);
			VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_FREE == job->state);
		}
	});

	if (NULL != system) {
		if (NULL != system->lock) {
			_vr_osk_spinlock_irq_term(system->lock);
		}
		_vr_osk_free(system);
	}
}

static struct vr_soft_job *vr_soft_job_system_alloc_job(struct vr_soft_job_system *system)
{
	struct vr_soft_job *job;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_ASSERT_SOFT_JOB_SYSTEM_LOCKED(system);

	if (_vr_osk_list_empty(&(system->jobs_free))) {
		/* No jobs available. */
		return NULL;
	}

	/* Grab first job and move it to the used list. */
	job = _VR_OSK_LIST_ENTRY(system->jobs_free.next, struct vr_soft_job, system_list);
	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_FREE == job->state);

	_vr_osk_list_move(&(job->system_list), &(system->jobs_used));
	job->state = VR_SOFT_JOB_STATE_ALLOCATED;

	VR_DEBUG_ASSERT(VR_SOFT_JOB_INVALID_ID != job->id);
	VR_DEBUG_ASSERT(system == job->system);

	return job;
}

static void vr_soft_job_system_free_job(struct vr_soft_job_system *system, struct vr_soft_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(system);

	vr_soft_job_system_lock(job->system);

	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_FREE != job->state);
	VR_DEBUG_ASSERT(VR_SOFT_JOB_INVALID_ID != job->id);
	VR_DEBUG_ASSERT(system == job->system);

	job->state = VR_SOFT_JOB_STATE_FREE;
	_vr_osk_list_move(&(job->system_list), &(system->jobs_free));

	vr_soft_job_system_unlock(job->system);
}

VR_STATIC_INLINE struct vr_soft_job *vr_soft_job_system_lookup_job(struct vr_soft_job_system *system, u32 job_id)
{
	VR_DEBUG_ASSERT_POINTER(system);
	VR_ASSERT_SOFT_JOB_SYSTEM_LOCKED(system);

	if (job_id < VR_MAX_NUM_SOFT_JOBS) {
		return &system->jobs[job_id];
	}

	return NULL;
}

void vr_soft_job_destroy(struct vr_soft_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->system);

	VR_DEBUG_PRINT(4, ("Vr Soft Job: destroying soft job %u (0x%08X)\n", job->id, job));

	if (NULL != job) {
		if (0 < _vr_osk_atomic_dec_return(&job->refcount)) return;

		_vr_osk_atomic_term(&job->refcount);

		if (NULL != job->activated_notification) {
			_vr_osk_notification_delete(job->activated_notification);
			job->activated_notification = NULL;
		}

		vr_soft_job_system_free_job(job->system, job);
	}
}

struct vr_soft_job *vr_soft_job_create(struct vr_soft_job_system *system, vr_soft_job_type type, u32 user_job)
{
	struct vr_soft_job *job;
	_vr_osk_notification_t *notification = NULL;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT(VR_SOFT_JOB_TYPE_USER_SIGNALED >= type);

	if (VR_SOFT_JOB_TYPE_USER_SIGNALED == type) {
		notification = _vr_osk_notification_create(_VR_NOTIFICATION_SOFT_ACTIVATED, sizeof(_vr_uk_soft_job_activated_s));
		if (unlikely(NULL == notification)) {
			VR_PRINT_ERROR(("Vr Soft Job: failed to allocate notification"));
			return NULL;
		}
	}

	vr_soft_job_system_lock(system);

	job = vr_soft_job_system_alloc_job(system);
	if (NULL == job) {
		vr_soft_job_system_unlock(system);
		VR_PRINT_ERROR(("Vr Soft Job: failed to allocate job"));
		_vr_osk_notification_delete(notification);
		return NULL;
	}

	job->type = type;
	job->user_job = user_job;
	job->activated = VR_FALSE;

	if (VR_SOFT_JOB_TYPE_USER_SIGNALED == type) {
		job->activated_notification = notification;
	}

	_vr_osk_atomic_init(&job->refcount, 1);

	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_ALLOCATED == job->state);
	VR_DEBUG_ASSERT(system == job->system);
	VR_DEBUG_ASSERT(VR_SOFT_JOB_INVALID_ID != job->id);

	vr_soft_job_system_unlock(system);

	return job;
}

vr_timeline_point vr_soft_job_start(struct vr_soft_job *job, struct vr_timeline_fence *fence)
{
	vr_timeline_point point;
	struct vr_soft_job_system *system;

	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(fence);

	VR_DEBUG_ASSERT_POINTER(job->system);
	system = job->system;

	VR_DEBUG_ASSERT_POINTER(system->session);
	VR_DEBUG_ASSERT_POINTER(system->session->timeline_system);

	vr_soft_job_system_lock(system);

	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_ALLOCATED == job->state);
	job->state = VR_SOFT_JOB_STATE_STARTED;

	vr_soft_job_system_unlock(system);

	VR_DEBUG_PRINT(4, ("Vr Soft Job: starting soft job %u (0x%08X)\n", job->id, job));

	vr_timeline_tracker_init(&job->tracker, VR_TIMELINE_TRACKER_SOFT, fence, job);
	point = vr_timeline_system_add_tracker(system->session->timeline_system, &job->tracker, VR_TIMELINE_SOFT);

	return point;
}

static vr_bool vr_soft_job_is_activated(void *data)
{
	struct vr_soft_job *job;

	job = (struct vr_soft_job *) data;
	VR_DEBUG_ASSERT_POINTER(job);

	return job->activated;
}

_vr_osk_errcode_t vr_soft_job_system_signal_job(struct vr_soft_job_system *system, u32 job_id)
{
	struct vr_soft_job *job;
	struct vr_timeline_system *timeline_system;
	vr_scheduler_mask schedule_mask;

	VR_DEBUG_ASSERT_POINTER(system);

	vr_soft_job_system_lock(system);

	job = vr_soft_job_system_lookup_job(system, job_id);

	if (NULL == job || !(VR_SOFT_JOB_STATE_STARTED == job->state || VR_SOFT_JOB_STATE_TIMED_OUT == job->state)) {
		vr_soft_job_system_unlock(system);
		VR_PRINT_ERROR(("Vr Soft Job: invalid soft job id %u", job_id));
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}

	if (VR_SOFT_JOB_STATE_TIMED_OUT == job->state) {
		job->state = VR_SOFT_JOB_STATE_SIGNALED;
		vr_soft_job_system_unlock(system);

		VR_DEBUG_ASSERT(VR_TRUE == job->activated);
		VR_DEBUG_PRINT(4, ("Vr Soft Job: soft job %u (0x%08X) was timed out\n", job->id, job));
		vr_soft_job_destroy(job);

		return _VR_OSK_ERR_TIMEOUT;
	}

	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_STARTED == job->state);

	job->state = VR_SOFT_JOB_STATE_SIGNALED;
	vr_soft_job_system_unlock(system);

	/* Since the job now is in signaled state, timeouts from the timeline system will be
	 * ignored, and it is not possible to signal this job again. */

	timeline_system = system->session->timeline_system;
	VR_DEBUG_ASSERT_POINTER(timeline_system);

	/* Wait until activated. */
	_vr_osk_wait_queue_wait_event(timeline_system->wait_queue, vr_soft_job_is_activated, (void *) job);

	VR_DEBUG_PRINT(4, ("Vr Soft Job: signaling soft job %u (0x%08X)\n", job->id, job));

	schedule_mask = vr_timeline_tracker_release(&job->tracker);
	vr_scheduler_schedule_from_mask(schedule_mask, VR_FALSE);

	vr_soft_job_destroy(job);

	return _VR_OSK_ERR_OK;
}

static void vr_soft_job_send_activated_notification(struct vr_soft_job *job)
{
	if (NULL != job->activated_notification) {
		_vr_uk_soft_job_activated_s *res = job->activated_notification->result_buffer;
		res->user_job = job->user_job;
		vr_session_send_notification(job->system->session, job->activated_notification);
	}
	job->activated_notification = NULL;
}

void vr_soft_job_system_activate_job(struct vr_soft_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->system);
	VR_DEBUG_ASSERT_POINTER(job->system->session);

	VR_DEBUG_PRINT(4, ("Vr Soft Job: Timeline activation for soft job %u (0x%08X).\n", job->id, job));

	vr_soft_job_system_lock(job->system);

	if (unlikely(job->system->session->is_aborting)) {
		VR_DEBUG_PRINT(3, ("Vr Soft Job: Soft job %u (0x%08X) activated while session is aborting.\n", job->id, job));

		vr_soft_job_system_unlock(job->system);

		/* Since we are in shutdown, we can ignore the scheduling bitmask. */
		vr_timeline_tracker_release(&job->tracker);
		vr_soft_job_destroy(job);
		return;
	}

	/* Send activated notification. */
	vr_soft_job_send_activated_notification(job);

	/* Wake up sleeping signaler. */
	job->activated = VR_TRUE;
	_vr_osk_wait_queue_wake_up(job->tracker.system->wait_queue);

	vr_soft_job_system_unlock(job->system);
}

vr_scheduler_mask vr_soft_job_system_timeout_job(struct vr_soft_job *job)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->system);
	VR_DEBUG_ASSERT_POINTER(job->system->session);
	VR_DEBUG_ASSERT(VR_TRUE == job->activated);

	VR_DEBUG_PRINT(4, ("Vr Soft Job: Timeline timeout for soft job %u (0x%08X).\n", job->id, job));

	vr_soft_job_system_lock(job->system);

	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_STARTED  == job->state ||
	                  VR_SOFT_JOB_STATE_SIGNALED == job->state);

	if (unlikely(job->system->session->is_aborting)) {
		/* The session is aborting.  This job will be released and destroyed by @ref
		 * vr_soft_job_system_abort(). */
		vr_soft_job_system_unlock(job->system);

		return VR_SCHEDULER_MASK_EMPTY;
	}

	if (VR_SOFT_JOB_STATE_STARTED != job->state) {
		VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_SIGNALED == job->state);

		/* The job is about to be signaled, ignore timeout. */
		VR_DEBUG_PRINT(4, ("Vr Soft Job: Timeout on soft job %u (0x%08X) in signaled state.\n", job->id, job));
		vr_soft_job_system_unlock(job->system);
		return schedule_mask;
	}

	VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_STARTED == job->state);

	job->state = VR_SOFT_JOB_STATE_TIMED_OUT;
	_vr_osk_atomic_inc(&job->refcount);

	vr_soft_job_system_unlock(job->system);

	schedule_mask = vr_timeline_tracker_release(&job->tracker);

	vr_soft_job_destroy(job);

	return schedule_mask;
}

void vr_soft_job_system_abort(struct vr_soft_job_system *system)
{
	u32 i;
	struct vr_soft_job *job, *tmp;
	_VR_OSK_LIST_HEAD_STATIC_INIT(jobs);

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(system->session);
	VR_DEBUG_ASSERT(system->session->is_aborting);

	VR_DEBUG_PRINT(3, ("Vr Soft Job: Aborting soft job system for session 0x%08X.\n", system->session));

	vr_soft_job_system_lock(system);

	for (i = 0; i < VR_MAX_NUM_SOFT_JOBS; ++i) {
		job = &(system->jobs[i]);

		VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_FREE      == job->state ||
		                  VR_SOFT_JOB_STATE_STARTED   == job->state ||
		                  VR_SOFT_JOB_STATE_TIMED_OUT == job->state);

		if (VR_SOFT_JOB_STATE_STARTED == job->state) {
			/* If the job has been activated, we have to release the tracker and destroy
			 * the job.  If not, the tracker will be released and the job destroyed when
			 * it is activated. */
			if (VR_TRUE == job->activated) {
				VR_DEBUG_PRINT(3, ("Vr Soft Job: Aborting unsignaled soft job %u (0x%08X).\n", job->id, job));

				job->state = VR_SOFT_JOB_STATE_SIGNALED;
				_vr_osk_list_move(&job->system_list, &jobs);
			}
		} else if (VR_SOFT_JOB_STATE_TIMED_OUT == job->state) {
			VR_DEBUG_PRINT(3, ("Vr Soft Job: Aborting timed out soft job %u (0x%08X).\n", job->id, job));

			/* We need to destroy this soft job. */
			_vr_osk_list_move(&job->system_list, &jobs);
		}
	}

	vr_soft_job_system_unlock(system);

	/* Release and destroy jobs. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &jobs, struct vr_soft_job, system_list) {
		VR_DEBUG_ASSERT(VR_SOFT_JOB_STATE_SIGNALED  == job->state ||
		                  VR_SOFT_JOB_STATE_TIMED_OUT == job->state);

		if (VR_SOFT_JOB_STATE_SIGNALED == job->state) {
			vr_timeline_tracker_release(&job->tracker);
		}

		/* Move job back to used list before destroying. */
		_vr_osk_list_move(&job->system_list, &system->jobs_used);

		vr_soft_job_destroy(job);
	}
}
