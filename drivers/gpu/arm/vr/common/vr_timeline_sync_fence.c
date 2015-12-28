/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_timeline_sync_fence.h"

#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_sync.h"

#if defined(CONFIG_SYNC)

/**
 * Creates a sync fence tracker and a sync fence.  Adds sync fence tracker to Timeline system and
 * returns sync fence.  The sync fence will be signaled when the sync fence tracker is activated.
 *
 * @param timeline Timeline.
 * @param point Point on timeline.
 * @return Sync fence that will be signaled when tracker is activated.
 */
static struct sync_fence *vr_timeline_sync_fence_create_and_add_tracker(struct vr_timeline *timeline, vr_timeline_point point)
{
	struct vr_timeline_sync_fence_tracker *sync_fence_tracker;
	struct sync_fence                       *sync_fence;
	struct vr_timeline_fence               fence;

	VR_DEBUG_ASSERT_POINTER(timeline);
	VR_DEBUG_ASSERT(VR_TIMELINE_NO_POINT != point);

	/* Allocate sync fence tracker. */
	sync_fence_tracker = _vr_osk_calloc(1, sizeof(struct vr_timeline_sync_fence_tracker));
	if (NULL == sync_fence_tracker) {
		VR_PRINT_ERROR(("Vr Timeline: sync_fence_tracker allocation failed\n"));
		return NULL;
	}

	/* Create sync flag. */
	VR_DEBUG_ASSERT_POINTER(timeline->sync_tl);
	sync_fence_tracker->flag = vr_sync_flag_create(timeline->sync_tl, point);
	if (NULL == sync_fence_tracker->flag) {
		VR_PRINT_ERROR(("Vr Timeline: sync_flag creation failed\n"));
		_vr_osk_free(sync_fence_tracker);
		return NULL;
	}

	/* Create sync fence from sync flag. */
	sync_fence = vr_sync_flag_create_fence(sync_fence_tracker->flag);
	if (NULL == sync_fence) {
		VR_PRINT_ERROR(("Vr Timeline: sync_fence creation failed\n"));
		vr_sync_flag_put(sync_fence_tracker->flag);
		_vr_osk_free(sync_fence_tracker);
		return NULL;
	}

	/* Setup fence for tracker. */
	_vr_osk_memset(&fence, 0, sizeof(struct vr_timeline_fence));
	fence.sync_fd = -1;
	fence.points[timeline->id] = point;

	/* Finally, add the tracker to Timeline system. */
	vr_timeline_tracker_init(&sync_fence_tracker->tracker, VR_TIMELINE_TRACKER_SYNC, &fence, sync_fence_tracker);
	point = vr_timeline_system_add_tracker(timeline->system, &sync_fence_tracker->tracker, VR_TIMELINE_NONE);
	VR_DEBUG_ASSERT(VR_TIMELINE_NO_POINT == point);

	return sync_fence;
}

s32 vr_timeline_sync_fence_create(struct vr_timeline_system *system, struct vr_timeline_fence *fence)
{
	u32 i;
	struct sync_fence *sync_fence_acc = NULL;

	VR_DEBUG_ASSERT_POINTER(system);
	VR_DEBUG_ASSERT_POINTER(fence);

	for (i = 0; i < VR_TIMELINE_MAX; ++i) {
		struct vr_timeline *timeline;
		struct sync_fence *sync_fence;

		if (VR_TIMELINE_NO_POINT == fence->points[i]) continue;

		timeline = system->timelines[i];
		VR_DEBUG_ASSERT_POINTER(timeline);

		sync_fence = vr_timeline_sync_fence_create_and_add_tracker(timeline, fence->points[i]);
		if (NULL == sync_fence)
		{
			/* temp test */
			printk("[MALI] error(%d)\n", __LINE__);
			goto error;
		}

		if (NULL != sync_fence_acc) {
			/* Merge sync fences. */
			sync_fence_acc = vr_sync_fence_merge(sync_fence_acc, sync_fence);
			if (NULL == sync_fence_acc)
			{
				/* temp test */
				printk("[MALI] error(%d)\n", __LINE__);
				goto error;
			}
		} else {
			/* This was the first sync fence created. */
			sync_fence_acc = sync_fence;
		}
	}

	if (-1 != fence->sync_fd) {
		struct sync_fence *sync_fence;

		sync_fence = sync_fence_fdget(fence->sync_fd);
		if (NULL == sync_fence)
		{
			/* temp test */
			printk("[MALI] error(%d)\n", __LINE__);
			goto error;
		}

		if (NULL != sync_fence_acc) {
			sync_fence_acc = vr_sync_fence_merge(sync_fence_acc, sync_fence);
			if (NULL == sync_fence_acc)
			{
				/* temp test */
				printk("[MALI] error(%d)\n", __LINE__);
				goto error;
			}
		} else {
			sync_fence_acc = sync_fence;
		}
	}

	if (NULL == sync_fence_acc) {
		VR_DEBUG_ASSERT_POINTER(system->signaled_sync_tl);

		/* There was nothing to wait on, so return an already signaled fence. */

		sync_fence_acc = vr_sync_timeline_create_signaled_fence(system->signaled_sync_tl);
		if (NULL == sync_fence_acc)
		{
			/* temp test */
			printk("[MALI] error(%d)\n", __LINE__);
			goto error;
		}
	}

	/* Return file descriptor for the accumulated sync fence. */
	return vr_sync_fence_fd_alloc(sync_fence_acc);

error:
	if (NULL != sync_fence_acc) {
		sync_fence_put(sync_fence_acc);
	}

	return -1;
}

void vr_timeline_sync_fence_activate(struct vr_timeline_sync_fence_tracker *sync_fence_tracker)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(sync_fence_tracker);
	VR_DEBUG_ASSERT_POINTER(sync_fence_tracker->flag);

	VR_DEBUG_PRINT(4, ("Vr Timeline: activation for sync fence tracker\n"));

	/* Signal flag and release reference. */
	vr_sync_flag_signal(sync_fence_tracker->flag, 0);
	vr_sync_flag_put(sync_fence_tracker->flag);

	/* Nothing can wait on this tracker, so nothing to schedule after release. */
	schedule_mask = vr_timeline_tracker_release(&sync_fence_tracker->tracker);
	VR_DEBUG_ASSERT(VR_SCHEDULER_MASK_EMPTY == schedule_mask);

	_vr_osk_free(sync_fence_tracker);
}

#endif /* defined(CONFIG_SYNC) */
