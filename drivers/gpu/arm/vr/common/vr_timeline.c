/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_timeline.h"
#include "vr_kernel_common.h"
#include "vr_osk_vr.h"
#include "vr_scheduler.h"
#include "vr_soft_job.h"
#include "vr_timeline_fence_wait.h"
#include "vr_timeline_sync_fence.h"

#define VR_TIMELINE_SYSTEM_LOCKED(system) (vr_spinlock_reentrant_is_held((system)->spinlock, _vr_osk_get_tid()))

static vr_scheduler_mask vr_timeline_system_release_waiter(struct vr_timeline_system *system,
        struct vr_timeline_waiter *waiter);

#if defined(CONFIG_SYNC)
/* Callback that is called when a sync fence a tracker is waiting on is signaled. */
static void vr_timeline_sync_fence_callback(struct sync_fence *sync_fence, struct sync_fence_waiter *sync_fence_waiter)
{
	struct vr_timeline_system  *system;
	struct vr_timeline_waiter  *waiter;
	struct vr_timeline_tracker *tracker;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
	u32 tid = _vr_osk_get_tid();
	vr_bool is_aborting = VR_FALSE;
	int fence_status = sync_fence->status;

	VR_DEBUG_ASSERT_POINTER(sync_fence);
	VR_DEBUG_ASSERT_POINTER(sync_fence_waiter);

	tracker = _VR_OSK_CONTAINER_OF(sync_fence_waiter, struct vr_timeline_tracker, sync_fence_waiter);
	VR_DEBUG_ASSERT_POINTER(tracker);

	system = tracker->system;
	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(system->session);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	is_aborting = system->session->is_aborting;
	if (!is_aborting && (0 > fence_status)) {
		VR_PRINT_ERROR(("Vr Timeline: sync fence fd %d signaled with error %d\n", tracker->fence.sync_fd, fence_status));
		tracker->activation_error |= VR_TIMELINE_ACTIVATION_ERROR_SYNC_BIT;
	}

	waiter = tracker->waiter_sync;
	VR_DEBUG_ASSERT_POINTER(waiter);

	tracker->sync_fence = NULL;
	schedule_mask |= vr_timeline_system_release_waiter(system, waiter);

	/* If aborting, wake up sleepers that are waiting for sync fence callbacks to complete. */
	if (is_aborting) {
		_vr_osk_wait_queue_wake_up(system->wait_queue);
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

	sync_fence_put(sync_fence);

	if (!is_aborting) {
		vr_scheduler_schedule_from_mask(schedule_mask, VR_TRUE);
	}
}
#endif /* defined(CONFIG_SYNC) */

static vr_scheduler_mask vr_timeline_tracker_time_out(struct vr_timeline_tracker *tracker)
{
	VR_DEBUG_ASSERT_POINTER(tracker);
	VR_DEBUG_ASSERT(VR_TIMELINE_TRACKER_SOFT == tracker->type);

	return vr_soft_job_system_timeout_job((struct vr_soft_job *) tracker->job);
}

static void vr_timeline_timer_callback(void *data)
{
	struct vr_timeline_system *system;
	struct vr_timeline_tracker *tracker;
	struct vr_timeline *timeline;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
	u32 tid = _vr_osk_get_tid();

	timeline = (struct vr_timeline *) data;
	VR_DEBUG_ASSERT_POINTER(timeline);

	system = timeline->system;
	VR_DEBUG_ASSERT_POINTER(system);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	if (!system->timer_enabled) {
		vr_spinlock_reentrant_signal(system->spinlock, tid);
		return;
	}

	tracker = timeline->tracker_tail;
	timeline->timer_active = VR_FALSE;

	if (NULL != tracker && VR_TRUE == tracker->timer_active) {
		/* This is likely the delayed work that has been schedule out before cancelled. */
		if (VR_TIMELINE_TIMEOUT_HZ > (_vr_osk_time_tickcount() - tracker->os_tick_activate)) {
			vr_spinlock_reentrant_signal(system->spinlock, tid);
			return;
		}

		schedule_mask = vr_timeline_tracker_time_out(tracker);
		tracker->timer_active = VR_FALSE;
	} else {
		VR_PRINT_ERROR(("Vr Timeline: Soft job timer callback without a waiting tracker.\n"));
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

	vr_scheduler_schedule_from_mask(schedule_mask, VR_FALSE);
}

void vr_timeline_system_stop_timer(struct vr_timeline_system *system)
{
	u32 i;
	u32 tid = _vr_osk_get_tid();

	VR_DEBUG_ASSERT_POINTER(system);

	vr_spinlock_reentrant_wait(system->spinlock, tid);
	system->timer_enabled = VR_FALSE;
	vr_spinlock_reentrant_signal(system->spinlock, tid);

	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		struct vr_timeline *timeline = system->timelines[i];

		VR_DEBUG_ASSERT_POINTER(timeline);

		if (NULL != timeline->delayed_work) {
			_vr_osk_wq_delayed_cancel_work_sync(timeline->delayed_work);
			timeline->timer_active = VR_FALSE;
		}
	}
}

static void vr_timeline_destroy(struct vr_timeline *timeline)
{
	VR_DEBUG_ASSERT_POINTER(timeline);
	if (NULL != timeline) {
		/* Assert that the timeline object has been properly cleaned up before destroying it. */
		VR_DEBUG_ASSERT(timeline->point_oldest == timeline->point_next);
		VR_DEBUG_ASSERT(NULL == timeline->tracker_head);
		VR_DEBUG_ASSERT(NULL == timeline->tracker_tail);
		VR_DEBUG_ASSERT(NULL == timeline->waiter_head);
		VR_DEBUG_ASSERT(NULL == timeline->waiter_tail);
		VR_DEBUG_ASSERT(NULL != timeline->system);
		VR_DEBUG_ASSERT(VR_TIMELINE_MAX > timeline->id);

#if defined(CONFIG_SYNC)
		if (NULL != timeline->sync_tl) {
			sync_timeline_destroy(timeline->sync_tl);
		}
#endif /* defined(CONFIG_SYNC) */

		if (NULL != timeline->delayed_work) {
			_vr_osk_wq_delayed_cancel_work_sync(timeline->delayed_work);
			_vr_osk_wq_delayed_delete_work_nonflush(timeline->delayed_work);
		}

		_vr_osk_free(timeline);
	}
}

static struct vr_timeline *vr_timeline_create(struct vr_timeline_system *system, enum vr_timeline_id id)
{
	struct vr_timeline *timeline;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT(id < VR_TIMELINE_MAX);

	timeline = (struct vr_timeline *) _vr_osk_calloc(1, sizeof(struct vr_timeline));
	if (NULL == timeline) {
		return NULL;
	}

	/* Initially the timeline is empty. */
#if defined(VR_TIMELINE_DEBUG_START_POINT)
	/* Start the timeline a bit before wrapping when debugging. */
	timeline->point_next = UINT_MAX - VR_TIMELINE_MAX_POINT_SPAN - 128;
#else
	timeline->point_next = 1;
#endif
	timeline->point_oldest = timeline->point_next;

	/* The tracker and waiter lists will initially be empty. */

	timeline->system = system;
	timeline->id = id;

	timeline->delayed_work = _vr_osk_wq_delayed_create_work(vr_timeline_timer_callback, timeline);
	if (NULL == timeline->delayed_work) {
		vr_timeline_destroy(timeline);
		return NULL;
	}

	timeline->timer_active = VR_FALSE;

#if defined(CONFIG_SYNC)
	{
		char timeline_name[32];

		switch (id) {
		case VR_TIMELINE_GP:
			_vr_osk_snprintf(timeline_name, 32, "vr-%u-gp", _vr_osk_get_pid());
			break;
		case VR_TIMELINE_PP:
			_vr_osk_snprintf(timeline_name, 32, "vr-%u-pp", _vr_osk_get_pid());
			break;
		case VR_TIMELINE_SOFT:
			_vr_osk_snprintf(timeline_name, 32, "vr-%u-soft", _vr_osk_get_pid());
			break;
		default:
			VR_PRINT_ERROR(("Vr Timeline: Invalid timeline id %d\n", id));
			vr_timeline_destroy(timeline);
			return NULL;
		}

		timeline->sync_tl = vr_sync_timeline_create(timeline_name);
		if (NULL == timeline->sync_tl) {
			vr_timeline_destroy(timeline);
			return NULL;
		}
	}
#endif /* defined(CONFIG_SYNC) */

	return timeline;
}

static void vr_timeline_insert_tracker(struct vr_timeline *timeline, struct vr_timeline_tracker *tracker)
{
	VR_DEBUG_ASSERT_POINTER(timeline);
	VR_DEBUG_ASSERT_POINTER(tracker);

	if (vr_timeline_is_full(timeline)) {
		/* Don't add tracker if timeline is full. */
		tracker->point = VR_TIMELINE_NO_POINT;
		return;
	}

	tracker->timeline = timeline;
	tracker->point    = timeline->point_next;

	/* Find next available point. */
	timeline->point_next++;
	if (VR_TIMELINE_NO_POINT == timeline->point_next) {
		timeline->point_next++;
	}

	VR_DEBUG_ASSERT(!vr_timeline_is_empty(timeline));

	/* Add tracker as new head on timeline's tracker list. */
	if (NULL == timeline->tracker_head) {
		/* Tracker list is empty. */
		VR_DEBUG_ASSERT(NULL == timeline->tracker_tail);

		timeline->tracker_tail = tracker;

		VR_DEBUG_ASSERT(NULL == tracker->timeline_next);
		VR_DEBUG_ASSERT(NULL == tracker->timeline_prev);
	} else {
		VR_DEBUG_ASSERT(NULL == timeline->tracker_head->timeline_next);

		tracker->timeline_prev = timeline->tracker_head;
		timeline->tracker_head->timeline_next = tracker;

		VR_DEBUG_ASSERT(NULL == tracker->timeline_next);
	}
	timeline->tracker_head = tracker;

	VR_DEBUG_ASSERT(NULL == timeline->tracker_head->timeline_next);
	VR_DEBUG_ASSERT(NULL == timeline->tracker_tail->timeline_prev);
}

/* Inserting the waiter object into the given timeline */
static void vr_timeline_insert_waiter(struct vr_timeline *timeline, struct vr_timeline_waiter *waiter_new)
{
	struct vr_timeline_waiter *waiter_prev;
	struct vr_timeline_waiter *waiter_next;

	/* Waiter time must be between timeline head and tail, and there must
	 * be less than VR_TIMELINE_MAX_POINT_SPAN elements between */
	VR_DEBUG_ASSERT(( waiter_new->point - timeline->point_oldest) < VR_TIMELINE_MAX_POINT_SPAN);
	VR_DEBUG_ASSERT((-waiter_new->point + timeline->point_next) < VR_TIMELINE_MAX_POINT_SPAN);

	/* Finding out where to put this waiter, in the linked waiter list of the given timeline **/
	waiter_prev = timeline->waiter_head; /* Insert new after  waiter_prev */
	waiter_next = NULL;                  /* Insert new before waiter_next */

	/* Iterating backwards from head (newest) to tail (oldest) until we
	 * find the correct spot to insert the new waiter */
	while (waiter_prev && vr_timeline_point_after(waiter_prev->point, waiter_new->point)) {
		waiter_next = waiter_prev;
		waiter_prev = waiter_prev->timeline_prev;
	}

	if (NULL == waiter_prev && NULL == waiter_next) {
		/* list is empty */
		timeline->waiter_head = waiter_new;
		timeline->waiter_tail = waiter_new;
	} else if (NULL == waiter_next) {
		/* insert at head */
		waiter_new->timeline_prev = timeline->waiter_head;
		timeline->waiter_head->timeline_next = waiter_new;
		timeline->waiter_head = waiter_new;
	} else if (NULL == waiter_prev) {
		/* insert at tail */
		waiter_new->timeline_next = timeline->waiter_tail;
		timeline->waiter_tail->timeline_prev = waiter_new;
		timeline->waiter_tail = waiter_new;
	} else {
		/* insert between */
		waiter_new->timeline_next = waiter_next;
		waiter_new->timeline_prev = waiter_prev;
		waiter_next->timeline_prev = waiter_new;
		waiter_prev->timeline_next = waiter_new;
	}
}

static void vr_timeline_update_delayed_work(struct vr_timeline *timeline)
{
	struct vr_timeline_system *system;
	struct vr_timeline_tracker *oldest_tracker;

	VR_DEBUG_ASSERT_POINTER(timeline);
	VR_DEBUG_ASSERT(VR_TIMELINE_SOFT == timeline->id);

	system = timeline->system;
	VR_DEBUG_ASSERT_POINTER(system);

	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	/* Timer is disabled, early out. */
	if (!system->timer_enabled) return;

	oldest_tracker = timeline->tracker_tail;
	if (NULL != oldest_tracker && 0 == oldest_tracker->trigger_ref_count) {
		if (VR_FALSE == oldest_tracker->timer_active) {
			if (VR_TRUE == timeline->timer_active) {
				_vr_osk_wq_delayed_cancel_work_async(timeline->delayed_work);
			}
			_vr_osk_wq_delayed_schedule_work(timeline->delayed_work, VR_TIMELINE_TIMEOUT_HZ);
			oldest_tracker->timer_active = VR_TRUE;
			timeline->timer_active = VR_TRUE;
		}
	} else if (VR_TRUE == timeline->timer_active) {
		_vr_osk_wq_delayed_cancel_work_async(timeline->delayed_work);
		timeline->timer_active = VR_FALSE;
	}
}

static vr_scheduler_mask vr_timeline_update_oldest_point(struct vr_timeline *timeline)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(timeline);

	VR_DEBUG_CODE({
		struct vr_timeline_system *system = timeline->system;
		VR_DEBUG_ASSERT_POINTER(system);

		VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));
	});

	if (NULL != timeline->tracker_tail) {
		/* Set oldest point to oldest tracker's point */
		timeline->point_oldest = timeline->tracker_tail->point;
	} else {
		/* No trackers, mark point list as empty */
		timeline->point_oldest = timeline->point_next;
	}

	/* Release all waiters no longer on the timeline's point list.
	 * Releasing a waiter can trigger this function to be called again, so
	 * we do not store any pointers on stack. */
	while (NULL != timeline->waiter_tail) {
		u32 waiter_time_relative;
		u32 time_head_relative;
		struct vr_timeline_waiter *waiter = timeline->waiter_tail;

		time_head_relative = timeline->point_next - timeline->point_oldest;
		waiter_time_relative = waiter->point - timeline->point_oldest;

		if (waiter_time_relative < time_head_relative) {
			/* This and all following waiters are on the point list, so we are done. */
			break;
		}

		/* Remove waiter from timeline's waiter list. */
		if (NULL != waiter->timeline_next) {
			waiter->timeline_next->timeline_prev = NULL;
		} else {
			/* This was the last waiter */
			timeline->waiter_head = NULL;
		}
		timeline->waiter_tail = waiter->timeline_next;

		/* Release waiter.  This could activate a tracker, if this was
		 * the last waiter for the tracker. */
		schedule_mask |= vr_timeline_system_release_waiter(timeline->system, waiter);
	}

	return schedule_mask;
}

void vr_timeline_tracker_init(struct vr_timeline_tracker *tracker,
                                vr_timeline_tracker_type type,
                                struct vr_timeline_fence *fence,
                                void *job)
{
	VR_DEBUG_ASSERT_POINTER(tracker);
	VR_DEBUG_ASSERT_POINTER(job);

	VR_DEBUG_ASSERT(VR_TIMELINE_TRACKER_MAX > type);

	/* Zero out all tracker members. */
	_vr_osk_memset(tracker, 0, sizeof(*tracker));

	tracker->type = type;
	tracker->job = job;
	tracker->trigger_ref_count = 1;  /* Prevents any callback from trigging while adding it */
	tracker->os_tick_create = _vr_osk_time_tickcount();
	VR_DEBUG_CODE(tracker->magic = VR_TIMELINE_TRACKER_MAGIC);

	tracker->activation_error = VR_TIMELINE_ACTIVATION_ERROR_NONE;

	/* Copy fence. */
	if (NULL != fence) {
		_vr_osk_memcpy(&tracker->fence, fence, sizeof(struct vr_timeline_fence));
	}
}

vr_scheduler_mask vr_timeline_tracker_release(struct vr_timeline_tracker *tracker)
{
	struct vr_timeline *timeline;
	struct vr_timeline_system *system;
	struct vr_timeline_tracker *tracker_next, *tracker_prev;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
	u32 tid = _vr_osk_get_tid();

	/* Upon entry a group lock will be held, but not a scheduler lock. */
	VR_DEBUG_ASSERT_POINTER(tracker);
	VR_DEBUG_ASSERT(VR_TIMELINE_TRACKER_MAGIC == tracker->magic);

	/* Tracker should have been triggered */
	VR_DEBUG_ASSERT(0 == tracker->trigger_ref_count);

	/* All waiters should have been released at this point */
	VR_DEBUG_ASSERT(NULL == tracker->waiter_head);
	VR_DEBUG_ASSERT(NULL == tracker->waiter_tail);

	VR_DEBUG_PRINT(3, ("Vr Timeline: releasing tracker for job 0x%08X\n", tracker->job));

	timeline = tracker->timeline;
	if (NULL == timeline) {
		/* Tracker was not on a timeline, there is nothing to release. */
		return VR_SCHEDULER_MASK_EMPTY;
	}

	system = timeline->system;
	VR_DEBUG_ASSERT_POINTER(system);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	/* Tracker should still be on timeline */
	VR_DEBUG_ASSERT(!vr_timeline_is_empty(timeline));
	VR_DEBUG_ASSERT( vr_timeline_is_point_on(timeline, tracker->point));

	/* Tracker is no longer valid. */
	VR_DEBUG_CODE(tracker->magic = 0);

	tracker_next = tracker->timeline_next;
	tracker_prev = tracker->timeline_prev;
	tracker->timeline_next = NULL;
	tracker->timeline_prev = NULL;

	/* Removing tracker from timeline's tracker list */
	if (NULL == tracker_next) {
		/* This tracker was the head */
		timeline->tracker_head = tracker_prev;
	} else {
		tracker_next->timeline_prev = tracker_prev;
	}

	if (NULL == tracker_prev) {
		/* This tracker was the tail */
		timeline->tracker_tail = tracker_next;
		VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));
		/* Update the timeline's oldest time and release any waiters */
		schedule_mask |= vr_timeline_update_oldest_point(timeline);
		VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));
	} else {
		tracker_prev->timeline_next = tracker_next;
	}

	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	/* Update delayed work only when it is the soft job timeline */
	if (VR_TIMELINE_SOFT == tracker->timeline->id) {
		vr_timeline_update_delayed_work(tracker->timeline);
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

	return schedule_mask;
}

void vr_timeline_system_release_waiter_list(struct vr_timeline_system *system,
        struct vr_timeline_waiter *tail,
        struct vr_timeline_waiter *head)
{
	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(head);
	VR_DEBUG_ASSERT_POINTER(tail);
	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	head->tracker_next = system->waiter_empty_list;
	system->waiter_empty_list = tail;
}

static vr_scheduler_mask vr_timeline_tracker_activate(struct vr_timeline_tracker *tracker)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
	struct vr_timeline_system *system;
	struct vr_timeline *timeline;
	u32 tid = _vr_osk_get_tid();

	VR_DEBUG_ASSERT_POINTER(tracker);
	VR_DEBUG_ASSERT(VR_TIMELINE_TRACKER_MAGIC == tracker->magic);

	system = tracker->system;
	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	tracker->os_tick_activate = _vr_osk_time_tickcount();

	if (NULL != tracker->waiter_head) {
		vr_timeline_system_release_waiter_list(system, tracker->waiter_tail, tracker->waiter_head);
		tracker->waiter_head = NULL;
		tracker->waiter_tail = NULL;
	}

	switch (tracker->type) {
	case VR_TIMELINE_TRACKER_GP:
		schedule_mask = vr_gp_scheduler_activate_job((struct vr_gp_job *) tracker->job);
		break;
	case VR_TIMELINE_TRACKER_PP:
		schedule_mask = vr_pp_scheduler_activate_job((struct vr_pp_job *) tracker->job);
		break;
	case VR_TIMELINE_TRACKER_SOFT:
		timeline = tracker->timeline;
		VR_DEBUG_ASSERT_POINTER(timeline);

		vr_soft_job_system_activate_job((struct vr_soft_job *) tracker->job);

		/* Start a soft timer to make sure the soft job be released in a limited time */
		vr_spinlock_reentrant_wait(system->spinlock, tid);
		vr_timeline_update_delayed_work(timeline);
		vr_spinlock_reentrant_signal(system->spinlock, tid);
		break;
	case VR_TIMELINE_TRACKER_WAIT:
		vr_timeline_fence_wait_activate((struct vr_timeline_fence_wait_tracker *) tracker->job);
		break;
	case VR_TIMELINE_TRACKER_SYNC:
#if defined(CONFIG_SYNC)
		vr_timeline_sync_fence_activate((struct vr_timeline_sync_fence_tracker *) tracker->job);
#else
		VR_PRINT_ERROR(("Vr Timeline: sync tracker not supported\n", tracker->type));
#endif /* defined(CONFIG_SYNC) */
		break;
	default:
		VR_PRINT_ERROR(("Vr Timeline - Illegal tracker type: %d\n", tracker->type));
		break;
	}

	return schedule_mask;
}

void vr_timeline_system_tracker_get(struct vr_timeline_system *system, struct vr_timeline_tracker *tracker)
{
	u32 tid = _vr_osk_get_tid();

	VR_DEBUG_ASSERT_POINTER(tracker);
	VR_DEBUG_ASSERT_POINTER(system);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	VR_DEBUG_ASSERT(0 < tracker->trigger_ref_count);
	tracker->trigger_ref_count++;

	vr_spinlock_reentrant_signal(system->spinlock, tid);
}

vr_scheduler_mask vr_timeline_system_tracker_put(struct vr_timeline_system *system, struct vr_timeline_tracker *tracker, vr_timeline_activation_error activation_error)
{
	u32 tid = _vr_osk_get_tid();
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(tracker);
	VR_DEBUG_ASSERT_POINTER(system);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	VR_DEBUG_ASSERT(0 < tracker->trigger_ref_count);
	tracker->trigger_ref_count--;

	tracker->activation_error |= activation_error;

	if (0 == tracker->trigger_ref_count) {
		schedule_mask |= vr_timeline_tracker_activate(tracker);
		tracker = NULL;
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

	return schedule_mask;
}

void vr_timeline_fence_copy_uk_fence(struct vr_timeline_fence *fence, _vr_uk_fence_t *uk_fence)
{
	u32 i;

	VR_DEBUG_ASSERT_POINTER(fence);
	VR_DEBUG_ASSERT_POINTER(uk_fence);

	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		fence->points[i] = uk_fence->points[i];
	}

	fence->sync_fd = uk_fence->sync_fd;
}

struct vr_timeline_system *vr_timeline_system_create(struct vr_session_data *session)
{
	u32 i;
	struct vr_timeline_system *system;

	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_PRINT(4, ("Vr Timeline: creating timeline system\n"));

	system = (struct vr_timeline_system *) _vr_osk_calloc(1, sizeof(struct vr_timeline_system));
	if (NULL == system) {
		return NULL;
	}

	system->spinlock = vr_spinlock_reentrant_init(_VR_OSK_LOCK_ORDER_TIMELINE_SYSTEM);
	if (NULL == system->spinlock) {
		vr_timeline_system_destroy(system);
		return NULL;
	}

	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		system->timelines[i] = vr_timeline_create(system, (enum vr_timeline_id)i);
		if (NULL == system->timelines[i]) {
			vr_timeline_system_destroy(system);
			return NULL;
		}
	}

#if defined(CONFIG_SYNC)
	system->signaled_sync_tl = vr_sync_timeline_create("vr-always-signaled");
	if (NULL == system->signaled_sync_tl) {
		vr_timeline_system_destroy(system);
		return NULL;
	}
#endif /* defined(CONFIG_SYNC) */

	system->waiter_empty_list = NULL;
	system->session = session;
	system->timer_enabled = VR_TRUE;

	system->wait_queue = _vr_osk_wait_queue_init();
	if (NULL == system->wait_queue) {
		vr_timeline_system_destroy(system);
		return NULL;
	}

	return system;
}

#if defined(CONFIG_SYNC)

/**
 * Check if there are any trackers left on timeline.
 *
 * Used as a wait queue conditional.
 *
 * @param data Timeline.
 * @return VR_TRUE if there are no trackers on timeline, VR_FALSE if not.
 */
static vr_bool vr_timeline_has_no_trackers(void *data)
{
	struct vr_timeline *timeline = (struct vr_timeline *) data;

	VR_DEBUG_ASSERT_POINTER(timeline);

	return vr_timeline_is_empty(timeline);
}

/**
 * Cancel sync fence waiters waited upon by trackers on all timelines.
 *
 * Will return after all timelines have no trackers left.
 *
 * @param system Timeline system.
 */
static void vr_timeline_cancel_sync_fence_waiters(struct vr_timeline_system *system)
{
	u32 i;
	u32 tid = _vr_osk_get_tid();
	struct vr_timeline_tracker *tracker, *tracker_next;
	_VR_OSK_LIST_HEAD_STATIC_INIT(tracker_list);

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(system->session);
	VR_DEBUG_ASSERT(system->session->is_aborting);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	/* Cancel sync fence waiters. */
	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		struct vr_timeline *timeline = system->timelines[i];

		VR_DEBUG_ASSERT_POINTER(timeline);

		tracker_next = timeline->tracker_tail;
		while (NULL != tracker_next) {
			tracker = tracker_next;
			tracker_next = tracker->timeline_next;

			if (NULL == tracker->sync_fence) continue;

			VR_DEBUG_PRINT(3, ("Vr Timeline: Cancelling sync fence wait for tracker 0x%08X.\n", tracker));

			/* Cancel sync fence waiter. */
			if (0 == sync_fence_cancel_async(tracker->sync_fence, &tracker->sync_fence_waiter)) {
				/* Callback was not called, move tracker to local list. */
				_vr_osk_list_add(&tracker->sync_fence_cancel_list, &tracker_list);
			}
		}
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

	/* Manually call sync fence callback in order to release waiter and trigger activation of tracker. */
	_VR_OSK_LIST_FOREACHENTRY(tracker, tracker_next, &tracker_list, struct vr_timeline_tracker, sync_fence_cancel_list) {
		vr_timeline_sync_fence_callback(tracker->sync_fence, &tracker->sync_fence_waiter);
	}

	/* Sleep until all sync fence callbacks are done and all timelines are empty. */
	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		struct vr_timeline *timeline = system->timelines[i];

		VR_DEBUG_ASSERT_POINTER(timeline);

		_vr_osk_wait_queue_wait_event(system->wait_queue, vr_timeline_has_no_trackers, (void *) timeline);
	}
}

#endif /* defined(CONFIG_SYNC) */

void vr_timeline_system_abort(struct vr_timeline_system *system)
{
	VR_DEBUG_CODE(u32 tid = _vr_osk_get_tid(););

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(system->session);
	VR_DEBUG_ASSERT(system->session->is_aborting);

	VR_DEBUG_PRINT(3, ("Vr Timeline: Aborting timeline system for session 0x%08X.\n", system->session));

#if defined(CONFIG_SYNC)
	vr_timeline_cancel_sync_fence_waiters(system);
#endif /* defined(CONFIG_SYNC) */

	/* Should not be any waiters or trackers left at this point. */
	VR_DEBUG_CODE( {
		u32 i;
		vr_spinlock_reentrant_wait(system->spinlock, tid);
		for (i = 0; i < VR_TIMELINE_MAX; ++i)
		{
			struct vr_timeline *timeline = system->timelines[i];
			VR_DEBUG_ASSERT_POINTER(timeline);
			VR_DEBUG_ASSERT(timeline->point_oldest == timeline->point_next);
			VR_DEBUG_ASSERT(NULL == timeline->tracker_head);
			VR_DEBUG_ASSERT(NULL == timeline->tracker_tail);
			VR_DEBUG_ASSERT(NULL == timeline->waiter_head);
			VR_DEBUG_ASSERT(NULL == timeline->waiter_tail);
		}
		vr_spinlock_reentrant_signal(system->spinlock, tid);
	});
}

void vr_timeline_system_destroy(struct vr_timeline_system *system)
{
	u32 i;
	struct vr_timeline_waiter *waiter, *next;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(system->session);

	VR_DEBUG_PRINT(4, ("Vr Timeline: destroying timeline system\n"));

	if (NULL != system) {
		/* There should be no waiters left on this queue. */
		if (NULL != system->wait_queue) {
			_vr_osk_wait_queue_term(system->wait_queue);
			system->wait_queue = NULL;
		}

		/* Free all waiters in empty list */
		waiter = system->waiter_empty_list;
		while (NULL != waiter) {
			next = waiter->tracker_next;
			_vr_osk_free(waiter);
			waiter = next;
		}

#if defined(CONFIG_SYNC)
		if (NULL != system->signaled_sync_tl) {
			sync_timeline_destroy(system->signaled_sync_tl);
		}
#endif /* defined(CONFIG_SYNC) */

		for (i = 0; i < VR_TIMELINE_MAX; ++i) {
			if (NULL != system->timelines[i]) {
				vr_timeline_destroy(system->timelines[i]);
			}
		}
		if (NULL != system->spinlock) {
			vr_spinlock_reentrant_term(system->spinlock);
		}

		_vr_osk_free(system);
	}
}

/**
 * Find how many waiters are needed for a given fence.
 *
 * @param fence The fence to check.
 * @return Number of waiters needed for fence.
 */
static u32 vr_timeline_fence_num_waiters(struct vr_timeline_fence *fence)
{
	u32 i, num_waiters = 0;

	VR_DEBUG_ASSERT_POINTER(fence);

	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		if (VR_TIMELINE_NO_POINT != fence->points[i]) {
			++num_waiters;
		}
	}

#if defined(CONFIG_SYNC)
	if (-1 != fence->sync_fd) ++num_waiters;
#endif /* defined(CONFIG_SYNC) */

	return num_waiters;
}

static struct vr_timeline_waiter *vr_timeline_system_get_zeroed_waiter(struct vr_timeline_system *system)
{
	struct vr_timeline_waiter *waiter;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	waiter = system->waiter_empty_list;
	if (NULL != waiter) {
		/* Remove waiter from empty list and zero it */
		system->waiter_empty_list = waiter->tracker_next;
		_vr_osk_memset(waiter, 0, sizeof(*waiter));
	}

	/* Return NULL if list was empty. */
	return waiter;
}

static void vr_timeline_system_allocate_waiters(struct vr_timeline_system *system,
        struct vr_timeline_waiter **tail,
        struct vr_timeline_waiter **head,
        int max_num_waiters)
{
	u32 i, tid = _vr_osk_get_tid();
	vr_bool do_alloc;
	struct vr_timeline_waiter *waiter;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(tail);
	VR_DEBUG_ASSERT_POINTER(head);

	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	*head = *tail = NULL;
	do_alloc = VR_FALSE;
	i = 0;
	while (i < max_num_waiters) {
		if (VR_FALSE == do_alloc) {
			waiter = vr_timeline_system_get_zeroed_waiter(system);
			if (NULL == waiter) {
				do_alloc = VR_TRUE;
				vr_spinlock_reentrant_signal(system->spinlock, tid);
				continue;
			}
		} else {
			waiter = _vr_osk_calloc(1, sizeof(struct vr_timeline_waiter));
			if (NULL == waiter) break;
		}
		++i;
		if (NULL == *tail) {
			*tail = waiter;
			*head = waiter;
		} else {
			(*head)->tracker_next = waiter;
			*head = waiter;
		}
	}
	if (VR_TRUE == do_alloc) {
		vr_spinlock_reentrant_wait(system->spinlock, tid);
	}
}

/**
 * Create waiters for the given tracker. The tracker is activated when all waiters are release.
 *
 * @note Tracker can potentially be activated before this function returns.
 *
 * @param system Timeline system.
 * @param tracker Tracker we will create waiters for.
 * @param waiter_tail List of pre-allocated waiters.
 * @param waiter_head List of pre-allocated waiters.
 */
static void vr_timeline_system_create_waiters_and_unlock(struct vr_timeline_system *system,
        struct vr_timeline_tracker *tracker,
        struct vr_timeline_waiter *waiter_tail,
        struct vr_timeline_waiter *waiter_head)
{
	int i;
	u32 tid = _vr_osk_get_tid();
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
#if defined(CONFIG_SYNC)
	struct sync_fence *sync_fence = NULL;
#endif /* defined(CONFIG_SYNC) */

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(tracker);

	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	VR_DEBUG_ASSERT(NULL == tracker->waiter_head);
	VR_DEBUG_ASSERT(NULL == tracker->waiter_tail);
	VR_DEBUG_ASSERT(NULL != tracker->job);

	/* Creating waiter object for all the timelines the fence is put on. Inserting this waiter
	 * into the timelines sorted list of waiters */
	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		vr_timeline_point point;
		struct vr_timeline *timeline;
		struct vr_timeline_waiter *waiter;

		/* Get point on current timeline from tracker's fence. */
		point = tracker->fence.points[i];

		if (likely(VR_TIMELINE_NO_POINT == point)) {
			/* Fence contains no point on this timeline so we don't need a waiter. */
			continue;
		}

		timeline = system->timelines[i];
		VR_DEBUG_ASSERT_POINTER(timeline);

		if (unlikely(!vr_timeline_is_point_valid(timeline, point))) {
			VR_PRINT_ERROR(("Vr Timeline: point %d is not valid (oldest=%d, next=%d)\n",
			                  point, timeline->point_oldest, timeline->point_next));
			continue;
		}

		if (likely(vr_timeline_is_point_released(timeline, point))) {
			/* Tracker representing the point has been released so we don't need a
			 * waiter. */
			continue;
		}

		/* The point is on timeline. */
		VR_DEBUG_ASSERT(vr_timeline_is_point_on(timeline, point));

		/* Get a new zeroed waiter object. */
		if (likely(NULL != waiter_tail)) {
			waiter = waiter_tail;
			waiter_tail = waiter_tail->tracker_next;
		} else {
			VR_PRINT_ERROR(("Vr Timeline: failed to allocate memory for waiter\n"));
			continue;
		}

		/* Yanking the trigger ref count of the tracker. */
		tracker->trigger_ref_count++;

		waiter->point   = point;
		waiter->tracker = tracker;

		/* Insert waiter on tracker's singly-linked waiter list. */
		if (NULL == tracker->waiter_head) {
			/* list is empty */
			VR_DEBUG_ASSERT(NULL == tracker->waiter_tail);
			tracker->waiter_tail = waiter;
		} else {
			tracker->waiter_head->tracker_next = waiter;
		}
		tracker->waiter_head = waiter;

		/* Add waiter to timeline. */
		vr_timeline_insert_waiter(timeline, waiter);
	}
#if defined(CONFIG_SYNC)
	if (-1 != tracker->fence.sync_fd) {
		int ret;
		struct vr_timeline_waiter *waiter;

		sync_fence = sync_fence_fdget(tracker->fence.sync_fd);
		if (unlikely(NULL == sync_fence)) {
			VR_PRINT_ERROR(("Vr Timeline: failed to get sync fence from fd %d\n", tracker->fence.sync_fd));
			goto exit;
		}

		/* Check if we have a zeroed waiter object available. */
		if (unlikely(NULL == waiter_tail)) {
			VR_PRINT_ERROR(("Vr Timeline: failed to allocate memory for waiter\n"));
			goto exit;
		}

		/* Start asynchronous wait that will release waiter when the fence is signaled. */
		sync_fence_waiter_init(&tracker->sync_fence_waiter, vr_timeline_sync_fence_callback);
		ret = sync_fence_wait_async(sync_fence, &tracker->sync_fence_waiter);
		if (1 == ret) {
			/* Fence already signaled, no waiter needed. */
			goto exit;
		} else if (0 != ret) {
			VR_PRINT_ERROR(("Vr Timeline: sync fence fd %d signaled with error %d\n", tracker->fence.sync_fd, ret));
			tracker->activation_error |= VR_TIMELINE_ACTIVATION_ERROR_SYNC_BIT;
			goto exit;
		}

		/* Grab new zeroed waiter object. */
		waiter = waiter_tail;
		waiter_tail = waiter_tail->tracker_next;

		/* Increase the trigger ref count of the tracker. */
		tracker->trigger_ref_count++;

		waiter->point   = VR_TIMELINE_NO_POINT;
		waiter->tracker = tracker;

		/* Insert waiter on tracker's singly-linked waiter list. */
		if (NULL == tracker->waiter_head) {
			/* list is empty */
			VR_DEBUG_ASSERT(NULL == tracker->waiter_tail);
			tracker->waiter_tail = waiter;
		} else {
			tracker->waiter_head->tracker_next = waiter;
		}
		tracker->waiter_head = waiter;

		/* Also store waiter in separate field for easy access by sync callback. */
		tracker->waiter_sync = waiter;

		/* Store the sync fence in tracker so we can retrieve in abort session, if needed. */
		tracker->sync_fence = sync_fence;

		sync_fence = NULL;
	}
exit:
#endif /* defined(CONFIG_SYNC) */

	if (NULL != waiter_tail) {
		vr_timeline_system_release_waiter_list(system, waiter_tail, waiter_head);
	}

	/* Release the initial trigger ref count. */
	tracker->trigger_ref_count--;

	/* If there were no waiters added to this tracker we activate immediately. */
	if (0 == tracker->trigger_ref_count) {
		schedule_mask |= vr_timeline_tracker_activate(tracker);
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

#if defined(CONFIG_SYNC)
	if (NULL != sync_fence) {
		sync_fence_put(sync_fence);
	}
#endif /* defined(CONFIG_SYNC) */

	vr_scheduler_schedule_from_mask(schedule_mask, VR_FALSE);
}

vr_timeline_point vr_timeline_system_add_tracker(struct vr_timeline_system *system,
        struct vr_timeline_tracker *tracker,
        enum vr_timeline_id timeline_id)
{
	int num_waiters = 0;
	struct vr_timeline_waiter *waiter_tail, *waiter_head;
	u32 tid = _vr_osk_get_tid();
	vr_timeline_point point = VR_TIMELINE_NO_POINT;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(system->session);
	VR_DEBUG_ASSERT_POINTER(tracker);

	VR_DEBUG_ASSERT(VR_FALSE == system->session->is_aborting);
	VR_DEBUG_ASSERT(VR_TIMELINE_TRACKER_MAX > tracker->type);
	VR_DEBUG_ASSERT(VR_TIMELINE_TRACKER_MAGIC == tracker->magic);

	VR_DEBUG_PRINT(4, ("Vr Timeline: adding tracker for job %p, timeline: %d\n", tracker->job, timeline_id));

	VR_DEBUG_ASSERT(0 < tracker->trigger_ref_count);
	tracker->system = system;

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	num_waiters = vr_timeline_fence_num_waiters(&tracker->fence);

	/* Allocate waiters. */
	vr_timeline_system_allocate_waiters(system, &waiter_tail, &waiter_head, num_waiters);
	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	/* Add tracker to timeline.  This will allocate a point for the tracker on the timeline. If
	 * timeline ID is VR_TIMELINE_NONE the tracker will NOT be added to a timeline and the
	 * point will be VR_TIMELINE_NO_POINT.
	 *
	 * NOTE: the tracker can fail to be added if the timeline is full.  If this happens, the
	 * point will be VR_TIMELINE_NO_POINT. */
	VR_DEBUG_ASSERT(timeline_id < VR_TIMELINE_MAX || timeline_id == VR_TIMELINE_NONE);
	if (likely(timeline_id < VR_TIMELINE_MAX)) {
		struct vr_timeline *timeline = system->timelines[timeline_id];
		vr_timeline_insert_tracker(timeline, tracker);
		VR_DEBUG_ASSERT(!vr_timeline_is_empty(timeline));
	}

	point = tracker->point;

	/* Create waiters for tracker based on supplied fence.  Each waiter will increase the
	 * trigger ref count. */
	vr_timeline_system_create_waiters_and_unlock(system, tracker, waiter_tail, waiter_head);
	tracker = NULL;

	/* At this point the tracker object might have been freed so we should no longer
	 * access it. */


	/* The tracker will always be activated after calling add_tracker, even if NO_POINT is
	 * returned. */
	return point;
}

static vr_scheduler_mask vr_timeline_system_release_waiter(struct vr_timeline_system *system,
        struct vr_timeline_waiter *waiter)
{
	struct vr_timeline_tracker *tracker;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(waiter);

	VR_DEBUG_ASSERT(VR_TIMELINE_SYSTEM_LOCKED(system));

	tracker = waiter->tracker;
	VR_DEBUG_ASSERT_POINTER(tracker);

	/* At this point the waiter has been removed from the timeline's waiter list, but it is
	 * still on the tracker's waiter list.  All of the tracker's waiters will be released when
	 * the tracker is activated. */

	waiter->point   = VR_TIMELINE_NO_POINT;
	waiter->tracker = NULL;

	tracker->trigger_ref_count--;
	if (0 == tracker->trigger_ref_count) {
		/* This was the last waiter; activate tracker */
		schedule_mask |= vr_timeline_tracker_activate(tracker);
		tracker = NULL;
	}

	return schedule_mask;
}

vr_timeline_point vr_timeline_system_get_latest_point(struct vr_timeline_system *system,
        enum vr_timeline_id timeline_id)
{
	vr_timeline_point point;
	struct vr_timeline *timeline;
	u32 tid = _vr_osk_get_tid();

	VR_DEBUG_ASSERT_POINTER(system);

	if (VR_TIMELINE_MAX <= timeline_id) {
		return VR_TIMELINE_NO_POINT;
	}

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	timeline = system->timelines[timeline_id];
	VR_DEBUG_ASSERT_POINTER(timeline);

	point = VR_TIMELINE_NO_POINT;
	if (timeline->point_oldest != timeline->point_next) {
		point = timeline->point_next - 1;
		if (VR_TIMELINE_NO_POINT == point) point--;
	}

	vr_spinlock_reentrant_signal(system->spinlock, tid);

	return point;
}

#if defined(VR_TIMELINE_DEBUG_FUNCTIONS)

static vr_bool is_waiting_on_timeline(struct vr_timeline_tracker *tracker, enum vr_timeline_id id)
{
	struct vr_timeline *timeline;
	struct vr_timeline_system *system;

	VR_DEBUG_ASSERT_POINTER(tracker);

	VR_DEBUG_ASSERT_POINTER(tracker->timeline);
	timeline = tracker->timeline;

	VR_DEBUG_ASSERT_POINTER(timeline->system);
	system = timeline->system;

	if (VR_TIMELINE_MAX > id) {
		return vr_timeline_is_point_on(system->timelines[id], tracker->fence.points[id]);
	} else {
		VR_DEBUG_ASSERT(VR_TIMELINE_NONE == id);
		return VR_FALSE;
	}
}

static const char *timeline_id_to_string(enum vr_timeline_id id)
{
	switch (id) {
	case VR_TIMELINE_GP:
		return "  GP";
	case VR_TIMELINE_PP:
		return "  PP";
	case VR_TIMELINE_SOFT:
		return "SOFT";
	default:
		return "NONE";
	}
}

static const char *timeline_tracker_type_to_string(enum vr_timeline_tracker_type type)
{
	switch (type) {
	case VR_TIMELINE_TRACKER_GP:
		return "  GP";
	case VR_TIMELINE_TRACKER_PP:
		return "  PP";
	case VR_TIMELINE_TRACKER_SOFT:
		return "SOFT";
	case VR_TIMELINE_TRACKER_WAIT:
		return "WAIT";
	case VR_TIMELINE_TRACKER_SYNC:
		return "SYNC";
	default:
		return "INVALID";
	}
}

vr_timeline_tracker_state vr_timeline_debug_get_tracker_state(struct vr_timeline_tracker *tracker)
{
	struct vr_timeline *timeline = NULL;

	VR_DEBUG_ASSERT_POINTER(tracker);
	timeline = tracker->timeline;

	if (0 != tracker->trigger_ref_count) {
		return VR_TIMELINE_TS_WAITING;
	}

	if (timeline && (timeline->tracker_tail == tracker || NULL != tracker->timeline_prev)) {
		return VR_TIMELINE_TS_ACTIVE;
	}

	if (timeline && (VR_TIMELINE_NO_POINT == tracker->point)) {
		return VR_TIMELINE_TS_INIT;
	}

	return VR_TIMELINE_TS_FINISH;
}

void vr_timeline_debug_print_tracker(struct vr_timeline_tracker *tracker)
{
	const char *tracker_state = "IWAF";

	VR_DEBUG_ASSERT_POINTER(tracker);

	if (0 != tracker->trigger_ref_count) {
		VR_PRINTF(("TL:  %s %u %c - ref_wait:%u [%s%u,%s%u,%s%u,%d]  (0x%08X)\n",
		             timeline_tracker_type_to_string(tracker->type), tracker->point,
		             *(tracker_state + vr_timeline_debug_get_tracker_state(tracker)),
		             tracker->trigger_ref_count,
		             is_waiting_on_timeline(tracker, VR_TIMELINE_GP) ? "W" : " ", tracker->fence.points[0],
		             is_waiting_on_timeline(tracker, VR_TIMELINE_PP) ? "W" : " ", tracker->fence.points[1],
		             is_waiting_on_timeline(tracker, VR_TIMELINE_SOFT) ? "W" : " ", tracker->fence.points[2],
		             tracker->fence.sync_fd, tracker->job));
	} else {
		VR_PRINTF(("TL:  %s %u %c  (0x%08X)\n",
		             timeline_tracker_type_to_string(tracker->type), tracker->point,
		             *(tracker_state + vr_timeline_debug_get_tracker_state(tracker)),
		             tracker->job));
	}
}

void vr_timeline_debug_print_timeline(struct vr_timeline *timeline)
{
	struct vr_timeline_tracker *tracker = NULL;
	int i_max = 30;

	VR_DEBUG_ASSERT_POINTER(timeline);

	tracker = timeline->tracker_tail;
	while (NULL != tracker && 0 < --i_max) {
		vr_timeline_debug_print_tracker(tracker);
		tracker = tracker->timeline_next;
	}

	if (0 == i_max) {
		VR_PRINTF(("TL: Too many trackers in list to print\n"));
	}
}

void vr_timeline_debug_print_system(struct vr_timeline_system *system)
{
	int i;
	int num_printed = 0;

	VR_DEBUG_ASSERT_POINTER(system);

	/* Print all timelines */
	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		struct vr_timeline *timeline = system->timelines[i];

		VR_DEBUG_ASSERT_POINTER(timeline);

		if (NULL == timeline->tracker_head) continue;

		VR_PRINTF(("TL: Timeline %s:\n",
		             timeline_id_to_string((enum vr_timeline_id)i)));
		vr_timeline_debug_print_timeline(timeline);
		num_printed++;
	}

	if (0 == num_printed) {
		VR_PRINTF(("TL: All timelines empty\n"));
	}
}

#endif /* defined(VR_TIMELINE_DEBUG_FUNCTIONS) */
