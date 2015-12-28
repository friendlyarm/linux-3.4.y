/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_SCHEDULER_H__
#define __VR_SCHEDULER_H__

#include "vr_osk.h"
#include "vr_scheduler_types.h"
#include "vr_gp_scheduler.h"
#include "vr_pp_scheduler.h"

_vr_osk_errcode_t vr_scheduler_initialize(void);
void vr_scheduler_terminate(void);

u32 vr_scheduler_get_new_id(void);
u32 vr_scheduler_get_new_cache_order(void);

/**
 * @brief Reset all groups
 *
 * This function resets all groups known by the both the PP and GP scheuduler.
 * This must be called after the Vr HW has been powered on in order to reset
 * the HW.
 */
VR_STATIC_INLINE void vr_scheduler_reset_all_groups(void)
{
	vr_gp_scheduler_reset_all_groups();
	vr_pp_scheduler_reset_all_groups();
}

/**
 * @brief Zap TLB on all active groups running \a session
 *
 * @param session Pointer to the session to zap
 */
VR_STATIC_INLINE void vr_scheduler_zap_all_active(struct vr_session_data *session)
{
	vr_gp_scheduler_zap_all_active(session);
	vr_pp_scheduler_zap_all_active(session);
}

/**
 * Check if bit is set in scheduler mask.
 *
 * @param mask Scheduler mask to check.
 * @param bit Bit to check.
 * @return VR_TRUE if bit is set in scheduler mask, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_scheduler_mask_is_set(vr_scheduler_mask mask, vr_scheduler_mask bit)
{
	return VR_SCHEDULER_MASK_EMPTY != (bit & mask);
}

/**
 * Schedule GP and PP according to bitmask.
 *
 * @param mask A scheduling bitmask.
 * @param deferred_schedule VR_TRUE if schedule should be deferred, VR_FALSE if not.
 */
void vr_scheduler_schedule_from_mask(vr_scheduler_mask mask, vr_bool deferred_schedule);

/* Enable or disable scheduler hint. */
extern vr_bool vr_scheduler_hints[VR_SCHEDULER_HINT_MAX];

VR_STATIC_INLINE void vr_scheduler_hint_enable(vr_scheduler_hint hint)
{
	VR_DEBUG_ASSERT(hint < VR_SCHEDULER_HINT_MAX);
	vr_scheduler_hints[hint] = VR_TRUE;
}

VR_STATIC_INLINE void vr_scheduler_hint_disable(vr_scheduler_hint hint)
{
	VR_DEBUG_ASSERT(hint < VR_SCHEDULER_HINT_MAX);
	vr_scheduler_hints[hint] = VR_FALSE;
}

VR_STATIC_INLINE vr_bool vr_scheduler_hint_is_enabled(vr_scheduler_hint hint)
{
	VR_DEBUG_ASSERT(hint < VR_SCHEDULER_HINT_MAX);
	return vr_scheduler_hints[hint];
}

#endif /* __VR_SCHEDULER_H__ */
