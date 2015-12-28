/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_KERNEL_L2_CACHE_H__
#define __VR_KERNEL_L2_CACHE_H__

#include "vr_osk.h"
#include "vr_hw_core.h"

#define VR_MAX_NUMBER_OF_L2_CACHE_CORES  3
/* Maximum 1 GP and 4 PP for an L2 cache core (Vr-400 Quad-core) */
#define VR_MAX_NUMBER_OF_GROUPS_PER_L2_CACHE 5

struct vr_group;
struct vr_pm_domain;

/* Flags describing state of the L2 */
typedef enum vr_l2_power_status {
	VR_L2_NORMAL, /**< L2 is in normal state and operational */
	VR_L2_PAUSE,  /**< L2 may not be accessed and may be powered off */
} vr_l2_power_status;

/**
 * Definition of the L2 cache core struct
 * Used to track a L2 cache unit in the system.
 * Contains information about the mapping of the registers
 */
struct vr_l2_cache_core {
	struct vr_hw_core  hw_core;      /**< Common for all HW cores */
	u32                  core_id;      /**< Unique core ID */
#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_t    *command_lock; /**< Serialize all L2 cache commands */
	_vr_osk_spinlock_irq_t    *counter_lock; /**< Synchronize L2 cache counter access */
#else
	_vr_osk_spinlock_t        *command_lock;
	_vr_osk_spinlock_t        *counter_lock;
#endif
	u32                  counter_src0; /**< Performance counter 0, VR_HW_CORE_NO_COUNTER for disabled */
	u32                  counter_src1; /**< Performance counter 1, VR_HW_CORE_NO_COUNTER for disabled */
	u32                  last_invalidated_id;
	struct vr_pm_domain *pm_domain;
	vr_l2_power_status   vr_l2_status; /**< Indicate whether the L2 is paused or not */
};

_vr_osk_errcode_t vr_l2_cache_initialize(void);
void vr_l2_cache_terminate(void);
/**
 * L2 pause is just a status that the L2 can't be accessed temporarily.
*/
void vr_l2_cache_pause_all(vr_bool pause);
struct vr_l2_cache_core *vr_l2_cache_create(_vr_osk_resource_t * resource);
void vr_l2_cache_delete(struct vr_l2_cache_core *cache);

VR_STATIC_INLINE void vr_l2_cache_set_pm_domain(struct vr_l2_cache_core *cache, struct vr_pm_domain *domain)
{
	cache->pm_domain = domain;
}

u32 vr_l2_cache_get_id(struct vr_l2_cache_core *cache);

void vr_l2_cache_core_set_counter_src0(struct vr_l2_cache_core *cache, u32 counter);
void vr_l2_cache_core_set_counter_src1(struct vr_l2_cache_core *cache, u32 counter);
u32 vr_l2_cache_core_get_counter_src0(struct vr_l2_cache_core *cache);
u32 vr_l2_cache_core_get_counter_src1(struct vr_l2_cache_core *cache);
void vr_l2_cache_core_get_counter_values(struct vr_l2_cache_core *cache, u32 *src0, u32 *value0, u32 *src1, u32 *value1);
struct vr_l2_cache_core *vr_l2_cache_core_get_glob_l2_core(u32 index);
u32 vr_l2_cache_core_get_glob_num_l2_cores(void);

void vr_l2_cache_reset(struct vr_l2_cache_core *cache);
void vr_l2_cache_reset_all(void);

struct vr_group *vr_l2_cache_get_group(struct vr_l2_cache_core *cache, u32 index);

void vr_l2_cache_invalidate(struct vr_l2_cache_core *cache);
vr_bool vr_l2_cache_invalidate_conditional(struct vr_l2_cache_core *cache, u32 id);
void vr_l2_cache_invalidate_all(void);
void vr_l2_cache_invalidate_all_pages(u32 *pages, u32 num_pages);

vr_bool vr_l2_cache_lock_power_state(struct vr_l2_cache_core *cache);
void vr_l2_cache_unlock_power_state(struct vr_l2_cache_core *cache);

#endif /* __VR_KERNEL_L2_CACHE_H__ */
