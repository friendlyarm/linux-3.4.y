/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_sync.h
 *
 * Vr interface for Linux sync objects.
 */

#ifndef _VR_SYNC_H_
#define _VR_SYNC_H_

#if defined(CONFIG_SYNC)

#include <linux/seq_file.h>
#include <linux/sync.h>

#include "vr_osk.h"

struct vr_sync_flag;

/**
 * Create a sync timeline.
 *
 * @param name Name of the sync timeline.
 * @return The new sync timeline if successful, NULL if not.
 */
struct sync_timeline *vr_sync_timeline_create(const char *name);

/**
 * Check if sync timeline belongs to Vr.
 *
 * @param sync_tl Sync timeline to check.
 * @return VR_TRUE if sync timeline belongs to Vr, VR_FALSE if not.
 */
vr_bool vr_sync_timeline_is_ours(struct sync_timeline *sync_tl);

/**
 * Creates a file descriptor representing the sync fence.  Will release sync fence if allocation of
 * file descriptor fails.
 *
 * @param sync_fence Sync fence.
 * @return File descriptor representing sync fence if successful, or -1 if not.
 */
s32 vr_sync_fence_fd_alloc(struct sync_fence *sync_fence);

/**
 * Merges two sync fences.  Both input sync fences will be released.
 *
 * @param sync_fence1 First sync fence.
 * @param sync_fence2 Second sync fence.
 * @return New sync fence that is the result of the merger if successful, or NULL if not.
 */
struct sync_fence *vr_sync_fence_merge(struct sync_fence *sync_fence1, struct sync_fence *sync_fence2);

/**
 * Create a sync fence that is already signaled.
 *
 * @param tl Sync timeline.
 * @return New signaled sync fence if successful, NULL if not.
 */
struct sync_fence *vr_sync_timeline_create_signaled_fence(struct sync_timeline *sync_tl);

/**
 * Create a sync flag.
 *
 * @param sync_tl Sync timeline.
 * @param point Point on Vr timeline.
 * @return New sync flag if successful, NULL if not.
 */
struct vr_sync_flag *vr_sync_flag_create(struct sync_timeline *sync_tl, u32 point);

/**
 * Grab sync flag reference.
 *
 * @param flag Sync flag.
 */
void vr_sync_flag_get(struct vr_sync_flag *flag);

/**
 * Release sync flag reference.  If this was the last reference, the sync flag will be freed.
 *
 * @param flag Sync flag.
 */
void vr_sync_flag_put(struct vr_sync_flag *flag);

/**
 * Signal sync flag.  All sync fences created from this flag will be signaled.
 *
 * @param flag Sync flag to signal.
 * @param error Negative error code, or 0 if no error.
 */
void vr_sync_flag_signal(struct vr_sync_flag *flag, int error);

/**
 * Create a sync fence attached to given sync flag.
 *
 * @param flag Sync flag.
 * @return New sync fence if successful, NULL if not.
 */
struct sync_fence *vr_sync_flag_create_fence(struct vr_sync_flag *flag);

#endif /* defined(CONFIG_SYNC) */

#endif /* _VR_SYNC_H_ */
