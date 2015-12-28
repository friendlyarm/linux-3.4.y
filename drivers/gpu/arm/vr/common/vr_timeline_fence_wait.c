/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_timeline_fence_wait.h"

#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_spinlock_reentrant.h"

/**
 * Allocate a fence waiter tracker.
 *
 * @return New fence waiter if successful, NULL if not.
 */
static struct vr_timeline_fence_wait_tracker *vr_timeline_fence_wait_tracker_alloc(void)
{
	return (struct vr_timeline_fence_wait_tracker *) _vr_osk_calloc(1, sizeof(struct vr_timeline_fence_wait_tracker));
}

/**
 * Free fence waiter tracker.
 *
 * @param wait Fence wait tracker to free.
 */
static void vr_timeline_fence_wait_tracker_free(struct vr_timeline_fence_wait_tracker *wait)
{
	VR_DEBUG_ASSERT_POINTER(wait);
	_vr_osk_atomic_term(&wait->refcount);
	_vr_osk_free(wait);
}

/**
 * Check if fence wait tracker has been activated.  Used as a wait queue condition.
 *
 * @param data Fence waiter.
 * @return VR_TRUE if tracker has been activated, VR_FALSE if not.
 */
static vr_bool vr_timeline_fence_wait_tracker_is_activated(void *data)
{
	struct vr_timeline_fence_wait_tracker *wait;

	wait = (struct vr_timeline_fence_wait_tracker *) data;
	VR_DEBUG_ASSERT_POINTER(wait);

	return wait->activated;
}

/**
 * Check if fence has been signaled.
 *
 * @param system Timeline system.
 * @param fence Timeline fence.
 * @return VR_TRUE if fence is signaled, VR_FALSE if not.
 */
static vr_bool vr_timeline_fence_wait_check_status(struct vr_timeline_system *system, struct vr_timeline_fence *fence)
{
	int i;
	u32 tid = _vr_osk_get_tid();
	vr_bool ret = VR_TRUE;
#if defined(CONFIG_SYNC)
	struct sync_fence *sync_fence = NULL;
#endif

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(fence);

	vr_spinlock_reentrant_wait(system->spinlock, tid);

	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		struct vr_timeline *timeline;
		vr_timeline_point   point;

		point = fence->points[i];

		if (likely(VR_TIMELINE_NO_POINT == point)) {
			/* Fence contains no point on this timeline. */
			continue;
		}

		timeline = system->timelines[i];
		VR_DEBUG_ASSERT_POINTER(timeline);

		if (unlikely(!vr_timeline_is_point_valid(timeline, point))) {
			VR_PRINT_ERROR(("Vr Timeline: point %d is not valid (oldest=%d, next=%d)\n", point, timeline->point_oldest, timeline->point_next));
		}

		if (!vr_timeline_is_point_released(timeline, point)) {
			ret = VR_FALSE;
			goto exit;
		}
	}

#if defined(CONFIG_SYNC)
	if (-1 != fence->sync_fd) {
		sync_fence = sync_fence_fdget(fence->sync_fd);
		if (likely(NULL != sync_fence)) {
			if (0 == sync_fence->status) {
				ret = VR_FALSE;
			}
		} else {
			VR_PRINT_ERROR(("Vr Timeline: failed to get sync fence from fd %d\n", fence->sync_fd));
		}
	}
#endif /* defined(CONFIG_SYNC) */

exit:
	vr_spinlock_reentrant_signal(system->spinlock, tid);

#if defined(CONFIG_SYNC)
	if (NULL != sync_fence) {
		sync_fence_put(sync_fence);
	}
#endif /* defined(CONFIG_SYNC) */

	return ret;
}

vr_bool vr_timeline_fence_wait(struct vr_timeline_system *system, struct vr_timeline_fence *fence, u32 timeout)
{
	struct vr_timeline_fence_wait_tracker *wait;
	vr_timeline_point point;
	vr_bool ret;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(fence);

	VR_DEBUG_PRINT(4, ("Vr Timeline: wait on fence\n"));

	if (VR_TIMELINE_FENCE_WAIT_TIMEOUT_IMMEDIATELY == timeout) {
		return vr_timeline_fence_wait_check_status(system, fence);
	}

	wait = vr_timeline_fence_wait_tracker_alloc();
	if (unlikely(NULL == wait)) {
		VR_PRINT_ERROR(("Vr Timeline: failed to allocate data for fence wait\n"));
		return VR_FALSE;
	}

	wait->activated = VR_FALSE;
	wait->system = system;

	/* Initialize refcount to two references.  The reference first will be released by this
	 * function after the wait is over.  The second reference will be released when the tracker
	 * is activated. */
	_vr_osk_atomic_init(&wait->refcount, 2);

	/* Add tracker to timeline system, but not to a timeline. */
	vr_timeline_tracker_init(&wait->tracker, VR_TIMELINE_TRACKER_WAIT, fence, wait);
	point = vr_timeline_system_add_tracker(system, &wait->tracker, VR_TIMELINE_NONE);
	VR_DEBUG_ASSERT(VR_TIMELINE_NO_POINT == point);
	VR_IGNORE(point);

	/* Wait for the tracker to be activated or time out. */
	if (VR_TIMELINE_FENCE_WAIT_TIMEOUT_NEVER == timeout) {
		_vr_osk_wait_queue_wait_event(system->wait_queue, vr_timeline_fence_wait_tracker_is_activated, (void *) wait);
	} else {
		_vr_osk_wait_queue_wait_event_timeout(system->wait_queue, vr_timeline_fence_wait_tracker_is_activated, (void *) wait, timeout);
	}

	ret = wait->activated;

	if (0 == _vr_osk_atomic_dec_return(&wait->refcount)) {
		vr_timeline_fence_wait_tracker_free(wait);
	}

	return ret;
}

void vr_timeline_fence_wait_activate(struct vr_timeline_fence_wait_tracker *wait)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(wait);
	VR_DEBUG_ASSERT_POINTER(wait->system);

	VR_DEBUG_PRINT(4, ("Vr Timeline: activation for fence wait tracker\n"));

	VR_DEBUG_ASSERT(VR_FALSE == wait->activated);
	wait->activated = VR_TRUE;

	_vr_osk_wait_queue_wake_up(wait->system->wait_queue);

	/* Nothing can wait on this tracker, so nothing to schedule after release. */
	schedule_mask = vr_timeline_tracker_release(&wait->tracker);
	VR_DEBUG_ASSERT(VR_SCHEDULER_MASK_EMPTY == schedule_mask);
	VR_IGNORE(schedule_mask);

	if (0 == _vr_osk_atomic_dec_return(&wait->refcount)) {
		vr_timeline_fence_wait_tracker_free(wait);
	}
}
