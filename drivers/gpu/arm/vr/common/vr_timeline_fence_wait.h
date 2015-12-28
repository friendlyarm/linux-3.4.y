/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_timeline_fence_wait.h
 *
 * This file contains functions used to wait until a Timeline fence is signaled.
 */

#ifndef __VR_TIMELINE_FENCE_WAIT_H__
#define __VR_TIMELINE_FENCE_WAIT_H__

#include "vr_osk.h"
#include "vr_timeline.h"

/**
 * If used as the timeout argument in @ref vr_timeline_fence_wait, a timer is not used and the
 * function only returns when the fence is signaled.
 */
#define VR_TIMELINE_FENCE_WAIT_TIMEOUT_NEVER ((u32) -1)

/**
 * If used as the timeout argument in @ref vr_timeline_fence_wait, the function will return
 * immediately with the current state of the fence.
 */
#define VR_TIMELINE_FENCE_WAIT_TIMEOUT_IMMEDIATELY 0

/**
 * Fence wait tracker.
 *
 * The fence wait tracker is added to the Timeline system with the fence we are waiting on as a
 * dependency.  We will then perform a blocking wait, possibly with a timeout, until the tracker is
 * activated, which happens when the fence is signaled.
 */
struct vr_timeline_fence_wait_tracker {
	vr_bool activated;                  /**< VR_TRUE if the tracker has been activated, VR_FALSE if not. */
	_vr_osk_atomic_t refcount;          /**< Reference count. */
	struct vr_timeline_system *system;  /**< Timeline system. */
	struct vr_timeline_tracker tracker; /**< Timeline tracker. */
};

/**
 * Wait for a fence to be signaled, or timeout is reached.
 *
 * @param system Timeline system.
 * @param fence Fence to wait on.
 * @param timeout Timeout in ms, or VR_TIMELINE_FENCE_WAIT_TIMEOUT_NEVER or
 * VR_TIMELINE_FENCE_WAIT_TIMEOUT_IMMEDIATELY.
 * @return VR_TRUE if signaled, VR_FALSE if timed out.
 */
vr_bool vr_timeline_fence_wait(struct vr_timeline_system *system, struct vr_timeline_fence *fence, u32 timeout);

/**
 * Used by the Timeline system to activate a fence wait tracker.
 *
 * @param fence_wait_tracker Fence waiter tracker.
 */
void vr_timeline_fence_wait_activate(struct vr_timeline_fence_wait_tracker *fence_wait_tracker);

#endif /* __VR_TIMELINE_FENCE_WAIT_H__ */
