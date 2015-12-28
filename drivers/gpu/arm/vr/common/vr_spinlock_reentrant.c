/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_spinlock_reentrant.h"

#include "vr_osk.h"
#include "vr_kernel_common.h"

struct vr_spinlock_reentrant *vr_spinlock_reentrant_init(_vr_osk_lock_order_t lock_order)
{
	struct vr_spinlock_reentrant *spinlock;

	spinlock = _vr_osk_calloc(1, sizeof(struct vr_spinlock_reentrant));
	if (NULL == spinlock) {
		return NULL;
	}

	spinlock->lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, lock_order);
	if (NULL == spinlock->lock) {
		vr_spinlock_reentrant_term(spinlock);
		return NULL;
	}

	return spinlock;
}

void vr_spinlock_reentrant_term(struct vr_spinlock_reentrant *spinlock)
{
	VR_DEBUG_ASSERT_POINTER(spinlock);
	VR_DEBUG_ASSERT(0 == spinlock->counter && 0 == spinlock->owner);

	if (NULL != spinlock->lock) {
		_vr_osk_spinlock_irq_term(spinlock->lock);
	}

	_vr_osk_free(spinlock);
}

void vr_spinlock_reentrant_wait(struct vr_spinlock_reentrant *spinlock, u32 tid)
{
	VR_DEBUG_ASSERT_POINTER(spinlock);
	VR_DEBUG_ASSERT_POINTER(spinlock->lock);
	VR_DEBUG_ASSERT(0 != tid);

	VR_DEBUG_PRINT(5, ("%s ^\n", __FUNCTION__));

	if (tid != spinlock->owner) {
		_vr_osk_spinlock_irq_lock(spinlock->lock);
		VR_DEBUG_ASSERT(0 == spinlock->owner && 0 == spinlock->counter);
		spinlock->owner = tid;
	}

	VR_DEBUG_PRINT(5, ("%s v\n", __FUNCTION__));

	++spinlock->counter;
}

void vr_spinlock_reentrant_signal(struct vr_spinlock_reentrant *spinlock, u32 tid)
{
	VR_DEBUG_ASSERT_POINTER(spinlock);
	VR_DEBUG_ASSERT_POINTER(spinlock->lock);
	VR_DEBUG_ASSERT(0 != tid && tid == spinlock->owner);

	--spinlock->counter;
	if (0 == spinlock->counter) {
		spinlock->owner = 0;
		VR_DEBUG_PRINT(5, ("%s release last\n", __FUNCTION__));
		_vr_osk_spinlock_irq_unlock(spinlock->lock);
	}
}
