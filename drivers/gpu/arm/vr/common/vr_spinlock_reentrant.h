/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_SPINLOCK_REENTRANT_H__
#define __VR_SPINLOCK_REENTRANT_H__

#include "vr_osk.h"
#include "vr_kernel_common.h"

/**
 * Reentrant spinlock.
 */
struct vr_spinlock_reentrant {
	_vr_osk_spinlock_irq_t *lock;
	u32               owner;
	u32               counter;
};

/**
 * Create a new reentrant spinlock.
 *
 * @param lock_order Lock order.
 * @return New reentrant spinlock.
 */
struct vr_spinlock_reentrant *vr_spinlock_reentrant_init(_vr_osk_lock_order_t lock_order);

/**
 * Terminate reentrant spinlock and free any associated resources.
 *
 * @param spinlock Reentrant spinlock to terminate.
 */
void vr_spinlock_reentrant_term(struct vr_spinlock_reentrant *spinlock);

/**
 * Wait for reentrant spinlock to be signaled.
 *
 * @param spinlock Reentrant spinlock.
 * @param tid Thread ID.
 */
void vr_spinlock_reentrant_wait(struct vr_spinlock_reentrant *spinlock, u32 tid);

/**
 * Signal reentrant spinlock.
 *
 * @param spinlock Reentrant spinlock.
 * @param tid Thread ID.
 */
void vr_spinlock_reentrant_signal(struct vr_spinlock_reentrant *spinlock, u32 tid);

/**
 * Check if thread is holding reentrant spinlock.
 *
 * @param spinlock Reentrant spinlock.
 * @param tid Thread ID.
 * @return VR_TRUE if thread is holding spinlock, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_spinlock_reentrant_is_held(struct vr_spinlock_reentrant *spinlock, u32 tid)
{
	VR_DEBUG_ASSERT_POINTER(spinlock->lock);
	return (tid == spinlock->owner && 0 < spinlock->counter);
}

#endif /* __VR_SPINLOCK_REENTRANT_H__ */
