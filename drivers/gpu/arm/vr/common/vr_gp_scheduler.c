/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_gp_scheduler.h"
#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_scheduler.h"
#include "vr_gp.h"
#include "vr_gp_job.h"
#include "vr_group.h"
#include "vr_timeline.h"
#include "vr_osk_profiling.h"
#include "vr_kernel_utilization.h"
#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
#include <linux/sched.h>
#include <trace/events/gpu.h>
#endif

enum vr_gp_slot_state {
	VR_GP_SLOT_STATE_IDLE,
	VR_GP_SLOT_STATE_WORKING,
	VR_GP_SLOT_STATE_DISABLED,
};

/* A render slot is an entity which jobs can be scheduled onto */
struct vr_gp_slot {
	struct vr_group *group;
	/*
	 * We keep track of the state here as well as in the group object
	 * so we don't need to take the group lock so often (and also avoid clutter with the working lock)
	 */
	enum vr_gp_slot_state state;
	u32 returned_cookie;
};

static u32 gp_version = 0;
static _VR_OSK_LIST_HEAD_STATIC_INIT(job_queue);      /* List of unscheduled jobs. */
static _VR_OSK_LIST_HEAD_STATIC_INIT(job_queue_high); /* List of unscheduled high priority jobs. */
static struct vr_gp_slot slot;

/* Variables to allow safe pausing of the scheduler */
static _vr_osk_wait_queue_t *gp_scheduler_working_wait_queue = NULL;
static u32 pause_count = 0;

static vr_bool vr_gp_scheduler_is_suspended(void *data);
static void vr_gp_scheduler_job_queued(void);
static void vr_gp_scheduler_job_completed(void);

#if defined(VR_UPPER_HALF_SCHEDULING)
static _vr_osk_spinlock_irq_t *gp_scheduler_lock = NULL;
#else
static _vr_osk_spinlock_t *gp_scheduler_lock = NULL;
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */

_vr_osk_errcode_t vr_gp_scheduler_initialize(void)
{
	u32 num_groups;
	u32 i;
	_vr_osk_errcode_t ret = _VR_OSK_ERR_OK;

#if defined(VR_UPPER_HALF_SCHEDULING)
	gp_scheduler_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER);
#else
	gp_scheduler_lock = _vr_osk_spinlock_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
	if (NULL == gp_scheduler_lock) {
		ret = _VR_OSK_ERR_NOMEM;
		goto cleanup;
	}

	gp_scheduler_working_wait_queue = _vr_osk_wait_queue_init();
	if (NULL == gp_scheduler_working_wait_queue) {
		ret = _VR_OSK_ERR_NOMEM;
		goto cleanup;
	}

	/* Find all the available GP cores */
	num_groups = vr_group_get_glob_num_groups();
	for (i = 0; i < num_groups; i++) {
		struct vr_group *group = vr_group_get_glob_group(i);
		VR_DEBUG_ASSERT(NULL != group);
		if (NULL != group) {
			struct vr_gp_core *gp_core = vr_group_get_gp_core(group);
			if (NULL != gp_core) {
				if (0 == gp_version) {
					/* Retrieve GP version */
					gp_version = vr_gp_core_get_version(gp_core);
				}
				slot.group = group;
				slot.state = VR_GP_SLOT_STATE_IDLE;
				break; /* There is only one GP, no point in looking for more */
			}
		} else {
			ret = _VR_OSK_ERR_ITEM_NOT_FOUND;
			goto cleanup;
		}
	}

	return _VR_OSK_ERR_OK;

cleanup:
	if (NULL != gp_scheduler_working_wait_queue) {
		_vr_osk_wait_queue_term(gp_scheduler_working_wait_queue);
		gp_scheduler_working_wait_queue = NULL;
	}

	if (NULL != gp_scheduler_lock) {
#if defined(VR_UPPER_HALF_SCHEDULING)
		_vr_osk_spinlock_irq_term(gp_scheduler_lock);
#else
		_vr_osk_spinlock_term(gp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
		gp_scheduler_lock = NULL;
	}

	return ret;
}

void vr_gp_scheduler_terminate(void)
{
	VR_DEBUG_ASSERT(   VR_GP_SLOT_STATE_IDLE     == slot.state
	                     || VR_GP_SLOT_STATE_DISABLED == slot.state);
	VR_DEBUG_ASSERT_POINTER(slot.group);
	vr_group_delete(slot.group);

	_vr_osk_wait_queue_term(gp_scheduler_working_wait_queue);

#if defined(VR_UPPER_HALF_SCHEDULING)
	_vr_osk_spinlock_irq_term(gp_scheduler_lock);
#else
	_vr_osk_spinlock_term(gp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
}

VR_STATIC_INLINE void vr_gp_scheduler_lock(void)
{
#if defined(VR_UPPER_HALF_SCHEDULING)
	_vr_osk_spinlock_irq_lock(gp_scheduler_lock);
#else
	_vr_osk_spinlock_lock(gp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
	VR_DEBUG_PRINT(5, ("Vr GP scheduler: GP scheduler lock taken\n"));
}

VR_STATIC_INLINE void vr_gp_scheduler_unlock(void)
{
	VR_DEBUG_PRINT(5, ("Vr GP scheduler: Releasing GP scheduler lock\n"));
#if defined(VR_UPPER_HALF_SCHEDULING)
	_vr_osk_spinlock_irq_unlock(gp_scheduler_lock);
#else
	_vr_osk_spinlock_unlock(gp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
}

#if defined(DEBUG)
#define VR_ASSERT_GP_SCHEDULER_LOCKED() VR_DEBUG_ASSERT_LOCK_HELD(gp_scheduler_lock)
#else
#define VR_ASSERT_GP_SCHEDULER_LOCKED() do {} while (0)
#endif /* defined(DEBUG) */

/* Group and scheduler must be locked when entering this function.  Both will be unlocked before
 * exiting. */
static void vr_gp_scheduler_schedule_internal_and_unlock(void)
{
	struct vr_gp_job *job = NULL;

	VR_DEBUG_ASSERT_LOCK_HELD(slot.group->lock);
	VR_DEBUG_ASSERT_LOCK_HELD(gp_scheduler_lock);

	if (0 < pause_count || VR_GP_SLOT_STATE_IDLE != slot.state ||
	    (_vr_osk_list_empty(&job_queue) && _vr_osk_list_empty(&job_queue_high))) {
		vr_gp_scheduler_unlock();
		vr_group_unlock(slot.group);
		VR_DEBUG_PRINT(4, ("Vr GP scheduler: Nothing to schedule (paused=%u, idle slots=%u)\n",
		                     pause_count, VR_GP_SLOT_STATE_IDLE == slot.state ? 1 : 0));
#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
		trace_gpu_sched_switch(vr_gp_get_hw_core_desc(group->gp_core), sched_clock(), 0, 0, 0);
#endif
		return; /* Nothing to do, so early out */
	}

	/* Get next job in queue */
	if (!_vr_osk_list_empty(&job_queue_high)) {
		job = _VR_OSK_LIST_ENTRY(job_queue_high.next, struct vr_gp_job, list);
	} else {
		VR_DEBUG_ASSERT(!_vr_osk_list_empty(&job_queue));
		job = _VR_OSK_LIST_ENTRY(job_queue.next, struct vr_gp_job, list);
	}

	VR_DEBUG_ASSERT_POINTER(job);

	/* Remove the job from queue */
	_vr_osk_list_del(&job->list);

	/* Mark slot as busy */
	slot.state = VR_GP_SLOT_STATE_WORKING;

	vr_gp_scheduler_unlock();

	VR_DEBUG_PRINT(3, ("Vr GP scheduler: Starting job %u (0x%08X)\n", vr_gp_job_get_id(job), job));

	vr_group_start_gp_job(slot.group, job);
	vr_group_unlock(slot.group);
}

void vr_gp_scheduler_schedule(void)
{
	vr_group_lock(slot.group);
	vr_gp_scheduler_lock();

	vr_gp_scheduler_schedule_internal_and_unlock();
}

static void vr_gp_scheduler_return_job_to_user(struct vr_gp_job *job, vr_bool success)
{
	_vr_uk_gp_job_finished_s *jobres = job->finished_notification->result_buffer;
	_vr_osk_memset(jobres, 0, sizeof(_vr_uk_gp_job_finished_s)); /* @@@@ can be removed once we initialize all members in this struct */
	jobres->user_job_ptr = vr_gp_job_get_user_id(job);
	if (VR_TRUE == success) {
		jobres->status = _VR_UK_JOB_STATUS_END_SUCCESS;
	} else {
		jobres->status = _VR_UK_JOB_STATUS_END_UNKNOWN_ERR;
	}

	jobres->heap_current_addr = vr_gp_job_get_current_heap_addr(job);
	jobres->perf_counter0 = vr_gp_job_get_perf_counter_value0(job);
	jobres->perf_counter1 = vr_gp_job_get_perf_counter_value1(job);

	vr_session_send_notification(vr_gp_job_get_session(job), job->finished_notification);
	job->finished_notification = NULL;

	vr_gp_job_delete(job);
	vr_gp_scheduler_job_completed();
}

/* Group must be locked when entering this function.  Will be unlocked before exiting. */
void vr_gp_scheduler_job_done(struct vr_group *group, struct vr_gp_job *job, vr_bool success)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(group);
	VR_DEBUG_ASSERT_POINTER(job);

	VR_DEBUG_ASSERT_LOCK_HELD(group->lock);
	VR_DEBUG_ASSERT(slot.group == group);

	VR_DEBUG_PRINT(3, ("Vr GP scheduler: Job %u (0x%08X) completed (%s)\n", vr_gp_job_get_id(job), job, success ? "success" : "failure"));

	/* Release tracker. */
	schedule_mask |= vr_timeline_tracker_release(&job->tracker);

	/* Signal PP job. */
	schedule_mask |= vr_gp_job_signal_pp_tracker(job, success);

	vr_gp_scheduler_lock();

	/* Mark slot as idle again */
	slot.state = VR_GP_SLOT_STATE_IDLE;

	/* If paused, then this was the last job, so wake up sleeping workers */
	if (pause_count > 0) {
		_vr_osk_wait_queue_wake_up(gp_scheduler_working_wait_queue);
	}

	/* Schedule any queued GP jobs on this group. */
	vr_gp_scheduler_schedule_internal_and_unlock();

	/* GP is now scheduled, removing it from the mask. */
	schedule_mask &= ~VR_SCHEDULER_MASK_GP;

	if (VR_SCHEDULER_MASK_EMPTY != schedule_mask) {
		/* Releasing the tracker activated other jobs that need scheduling. */
		vr_scheduler_schedule_from_mask(schedule_mask, VR_FALSE);
	}

	/* Sends the job end message to user space and free the job object */
	vr_gp_scheduler_return_job_to_user(job, success);
}

void vr_gp_scheduler_oom(struct vr_group *group, struct vr_gp_job *job)
{
	_vr_uk_gp_job_suspended_s * jobres;
	_vr_osk_notification_t * notification;

	vr_gp_scheduler_lock();

	notification = job->oom_notification;
	job->oom_notification = NULL;
	slot.returned_cookie = vr_gp_job_get_id(job);

	jobres = (_vr_uk_gp_job_suspended_s *)notification->result_buffer;
	jobres->user_job_ptr = vr_gp_job_get_user_id(job);
	jobres->cookie = vr_gp_job_get_id(job);

	vr_gp_scheduler_unlock();

	vr_session_send_notification(vr_gp_job_get_session(job), notification);

	/*
	* If this function failed, then we could return the job to user space right away,
	* but there is a job timer anyway that will do that eventually.
	* This is not exactly a common case anyway.
	*/
}

void vr_gp_scheduler_suspend(void)
{
	vr_gp_scheduler_lock();
	pause_count++; /* Increment the pause_count so that no more jobs will be scheduled */
	vr_gp_scheduler_unlock();

	_vr_osk_wait_queue_wait_event(gp_scheduler_working_wait_queue, vr_gp_scheduler_is_suspended, NULL);
}

void vr_gp_scheduler_resume(void)
{
	vr_gp_scheduler_lock();
	pause_count--; /* Decrement pause_count to allow scheduling again (if it reaches 0) */
	vr_gp_scheduler_unlock();
	if (0 == pause_count) {
		vr_gp_scheduler_schedule();
	}
}

vr_timeline_point vr_gp_scheduler_submit_job(struct vr_session_data *session, struct vr_gp_job *job)
{
	vr_timeline_point point;

	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_ASSERT_POINTER(job);

	vr_gp_scheduler_job_queued();

	/* Add job to Timeline system. */
	point = vr_timeline_system_add_tracker(session->timeline_system, &job->tracker, VR_TIMELINE_GP);

	return point;
}

_vr_osk_errcode_t _vr_ukk_gp_start_job(void *ctx, _vr_uk_gp_start_job_s *uargs)
{
	struct vr_session_data *session;
	struct vr_gp_job *job;
	vr_timeline_point point;
	u32 __user *timeline_point_ptr = NULL;

	VR_DEBUG_ASSERT_POINTER(uargs);
	VR_DEBUG_ASSERT_POINTER(ctx);

	session = (struct vr_session_data*)ctx;

	job = vr_gp_job_create(session, uargs, vr_scheduler_get_new_id(), NULL);
	if (NULL == job) {
		VR_PRINT_ERROR(("Failed to create GP job.\n"));
		return _VR_OSK_ERR_NOMEM;
	}

	timeline_point_ptr = (u32 __user *) job->uargs.timeline_point_ptr;

	point = vr_gp_scheduler_submit_job(session, job);

	if (0 != _vr_osk_put_user(((u32) point), timeline_point_ptr)) {
		/* Let user space know that something failed after the job was started. */
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t _vr_ukk_get_gp_number_of_cores(_vr_uk_get_gp_number_of_cores_s *args)
{
	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);
	args->number_of_cores = 1;
	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t _vr_ukk_get_gp_core_version(_vr_uk_get_gp_core_version_s *args)
{
	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);
	args->version = gp_version;
	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t _vr_ukk_gp_suspend_response(_vr_uk_gp_suspend_response_s *args)
{
	struct vr_session_data *session;
	struct vr_gp_job *resumed_job;
	_vr_osk_notification_t *new_notification = 0;

	VR_DEBUG_ASSERT_POINTER(args);

	if (NULL == args->ctx) {
		return _VR_OSK_ERR_INVALID_ARGS;
	}

	session = (struct vr_session_data*)args->ctx;
	if (NULL == session) {
		return _VR_OSK_ERR_FAULT;
	}

	if (_VRGP_JOB_RESUME_WITH_NEW_HEAP == args->code) {
		new_notification = _vr_osk_notification_create(_VR_NOTIFICATION_GP_STALLED, sizeof(_vr_uk_gp_job_suspended_s));

		if (NULL == new_notification) {
			VR_PRINT_ERROR(("Vr GP scheduler: Failed to allocate notification object. Will abort GP job.\n"));
			vr_group_lock(slot.group);
			vr_group_abort_gp_job(slot.group, args->cookie);
			vr_group_unlock(slot.group);
			return _VR_OSK_ERR_FAULT;
		}
	}

	vr_group_lock(slot.group);

	if (_VRGP_JOB_RESUME_WITH_NEW_HEAP == args->code) {
		VR_DEBUG_PRINT(3, ("Vr GP scheduler: Resuming job %u with new heap; 0x%08X - 0x%08X\n", args->cookie, args->arguments[0], args->arguments[1]));

		resumed_job = vr_group_resume_gp_with_new_heap(slot.group, args->cookie, args->arguments[0], args->arguments[1]);
		if (NULL != resumed_job) {
			resumed_job->oom_notification = new_notification;
			vr_group_unlock(slot.group);
			return _VR_OSK_ERR_OK;
		} else {
			vr_group_unlock(slot.group);
			_vr_osk_notification_delete(new_notification);
			return _VR_OSK_ERR_FAULT;
		}
	}

	VR_DEBUG_PRINT(2, ("Vr GP scheduler: Aborting job %u, no new heap provided\n", args->cookie));
	vr_group_abort_gp_job(slot.group, args->cookie);
	vr_group_unlock(slot.group);
	return _VR_OSK_ERR_OK;
}

void vr_gp_scheduler_abort_session(struct vr_session_data *session)
{
	struct vr_gp_job *job, *tmp;
	_VR_OSK_LIST_HEAD_STATIC_INIT(removed_jobs);

	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_ASSERT(session->is_aborting);

	VR_DEBUG_PRINT(3, ("Vr GP scheduler: Aborting all jobs from session 0x%08X.\n", session));

	vr_gp_scheduler_lock();

	/* Find all jobs from the aborting session. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &job_queue, struct vr_gp_job, list) {
		if (job->session == session) {
			VR_DEBUG_PRINT(3, ("Vr GP scheduler: Removing job %u (0x%08X) from queue.\n", vr_gp_job_get_id(job), job));
			_vr_osk_list_move(&job->list, &removed_jobs);
		}
	}

	/* Find all high priority jobs from the aborting session. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &job_queue_high, struct vr_gp_job, list) {
		if (job->session == session) {
			VR_DEBUG_PRINT(3, ("Vr GP scheduler: Removing job %u (0x%08X) from queue.\n", vr_gp_job_get_id(job), job));
			_vr_osk_list_move(&job->list, &removed_jobs);
		}
	}

	vr_gp_scheduler_unlock();

	/* Release and delete all found jobs from the aborting session. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &removed_jobs, struct vr_gp_job, list) {
		vr_timeline_tracker_release(&job->tracker);
		vr_gp_job_signal_pp_tracker(job, VR_FALSE);
		vr_gp_job_delete(job);
		vr_gp_scheduler_job_completed();
	}

	/* Abort any running jobs from the session. */
	vr_group_abort_session(slot.group, session);
}

static vr_bool vr_gp_scheduler_is_suspended(void *data)
{
	vr_bool ret;

	/* This callback does not use the data pointer. */
	VR_IGNORE(data);

	vr_gp_scheduler_lock();
	ret = pause_count > 0 && (slot.state == VR_GP_SLOT_STATE_IDLE || slot.state == VR_GP_SLOT_STATE_DISABLED);
	vr_gp_scheduler_unlock();

	return ret;
}


#if VR_STATE_TRACKING
u32 vr_gp_scheduler_dump_state(char *buf, u32 size)
{
	int n = 0;

	n += _vr_osk_snprintf(buf + n, size - n, "GP\n");
	n += _vr_osk_snprintf(buf + n, size - n, "\tQueue is %s\n", _vr_osk_list_empty(&job_queue) ? "empty" : "not empty");
	n += _vr_osk_snprintf(buf + n, size - n, "\tHigh priority queue is %s\n", _vr_osk_list_empty(&job_queue_high) ? "empty" : "not empty");

	n += vr_group_dump_state(slot.group, buf + n, size - n);
	n += _vr_osk_snprintf(buf + n, size - n, "\n");

	return n;
}
#endif

void vr_gp_scheduler_reset_all_groups(void)
{
	if (NULL != slot.group) {
		vr_group_lock(slot.group);
		vr_group_reset(slot.group);
		vr_group_unlock(slot.group);
	}
}

void vr_gp_scheduler_zap_all_active(struct vr_session_data *session)
{
	if (NULL != slot.group) {
		vr_group_zap_session(slot.group, session);
	}
}

void vr_gp_scheduler_enable_group(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);
	VR_DEBUG_ASSERT(slot.group == group);
	VR_DEBUG_PRINT(2, ("Vr GP scheduler: enabling gp group %p\n", group));

	vr_group_lock(group);

	if (VR_GROUP_STATE_DISABLED != group->state) {
		vr_group_unlock(group);
		VR_DEBUG_PRINT(2, ("Vr GP scheduler: gp group %p already enabled\n", group));
		return;
	}

	vr_gp_scheduler_lock();

	VR_DEBUG_ASSERT(VR_GROUP_STATE_DISABLED == group->state);
	VR_DEBUG_ASSERT(VR_GP_SLOT_STATE_DISABLED == slot.state);
	slot.state = VR_GP_SLOT_STATE_IDLE;
	group->state = VR_GROUP_STATE_IDLE;

	vr_group_power_on_group(group);
	vr_group_reset(group);

	/* Pick up any jobs that might have been queued while the GP group was disabled. */
	vr_gp_scheduler_schedule_internal_and_unlock();
}

void vr_gp_scheduler_disable_group(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);
	VR_DEBUG_ASSERT(slot.group == group);
	VR_DEBUG_PRINT(2, ("Vr GP scheduler: disabling gp group %p\n", group));

	vr_gp_scheduler_suspend();
	vr_group_lock(group);
	vr_gp_scheduler_lock();

	VR_DEBUG_ASSERT(   VR_GROUP_STATE_IDLE     == group->state
	                     || VR_GROUP_STATE_DISABLED == group->state);

	if (VR_GROUP_STATE_DISABLED == group->state) {
		VR_DEBUG_ASSERT(VR_GP_SLOT_STATE_DISABLED == slot.state);
		VR_DEBUG_PRINT(2, ("Vr GP scheduler: gp group %p already disabled\n", group));
	} else {
		VR_DEBUG_ASSERT(VR_GP_SLOT_STATE_IDLE == slot.state);
		slot.state = VR_GP_SLOT_STATE_DISABLED;
		group->state = VR_GROUP_STATE_DISABLED;

		vr_group_power_off_group(group, VR_TRUE);
	}

	vr_gp_scheduler_unlock();
	vr_group_unlock(group);
	vr_gp_scheduler_resume();
}

static vr_scheduler_mask vr_gp_scheduler_queue_job(struct vr_gp_job *job)
{
	_vr_osk_list_t *queue = NULL;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
	struct vr_gp_job *iter, *tmp;

	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->session);

	VR_DEBUG_ASSERT_LOCK_HELD(gp_scheduler_lock);

	_vr_osk_profiling_add_event(VR_PROFILING_EVENT_TYPE_SINGLE | VR_PROFILING_EVENT_CHANNEL_SOFTWARE | VR_PROFILING_EVENT_REASON_SINGLE_SW_GP_ENQUEUE, job->pid, job->tid, job->uargs.frame_builder_id, job->uargs.flush_id, 0);

	job->cache_order = vr_scheduler_get_new_cache_order();

	/* Determine which queue the job should be added to. */
	if (job->session->use_high_priority_job_queue) {
		queue = &job_queue_high;
	} else {
		queue = &job_queue;
	}

	/* Find position in queue where job should be added. */
	_VR_OSK_LIST_FOREACHENTRY_REVERSE(iter, tmp, queue, struct vr_gp_job, list) {
		if (vr_gp_job_is_after(job, iter)) {
			break;
		}
	}

	/* Add job to queue. */
	_vr_osk_list_add(&job->list, &iter->list);

	/* Set schedule bitmask if the GP core is idle. */
	if (VR_GP_SLOT_STATE_IDLE == slot.state) {
		schedule_mask |= VR_SCHEDULER_MASK_GP;
	}

#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
	trace_gpu_job_enqueue(vr_gp_job_get_tid(job), vr_gp_job_get_id(job), "GP");
#endif

	VR_DEBUG_PRINT(3, ("Vr GP scheduler: Job %u (0x%08X) queued\n", vr_gp_job_get_id(job), job));

	return schedule_mask;
}

vr_scheduler_mask vr_gp_scheduler_activate_job(struct vr_gp_job *job)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->session);

	VR_DEBUG_PRINT(4, ("Vr GP scheduler: Timeline activation for job %u (0x%08X).\n", vr_gp_job_get_id(job), job));

	vr_gp_scheduler_lock();

	if (unlikely(job->session->is_aborting)) {
		/* Before checking if the session is aborting, the scheduler must be locked. */
		VR_DEBUG_ASSERT_LOCK_HELD(gp_scheduler_lock);

		VR_DEBUG_PRINT(3, ("Vr GP scheduler: Job %u (0x%08X) activated while session is aborting.\n", vr_gp_job_get_id(job), job));

		/* This job should not be on any list. */
		VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->list));

		vr_gp_scheduler_unlock();

		/* Release tracker and delete job. */
		vr_timeline_tracker_release(&job->tracker);
		vr_gp_job_signal_pp_tracker(job, VR_FALSE);
		vr_gp_job_delete(job);
		vr_gp_scheduler_job_completed();

		/* Since we are aborting we ignore the scheduler mask. */
		return VR_SCHEDULER_MASK_EMPTY;
	}

	/* GP job is ready to run, queue it. */
	schedule_mask = vr_gp_scheduler_queue_job(job);

	vr_gp_scheduler_unlock();

	return schedule_mask;
}

static void vr_gp_scheduler_job_queued(void)
{
	/* We hold a PM reference for every job we hold queued (and running) */
	_vr_osk_pm_dev_ref_add();

	if (vr_utilization_enabled()) {
		/*
		 * We cheat a little bit by counting the PP as busy from the time a GP job is queued.
		 * This will be fine because we only loose the tiny idle gap between jobs, but
		 * we will instead get less utilization work to do (less locks taken)
		 */
		vr_utilization_gp_start();
	}
}

static void vr_gp_scheduler_job_completed(void)
{
	/* Release the PM reference we got in the vr_gp_scheduler_job_queued() function */
	_vr_osk_pm_dev_ref_dec();

	if (vr_utilization_enabled()) {
		vr_utilization_gp_end();
	}
}
