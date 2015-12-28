/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_PM_DOMAIN_H__
#define __VR_PM_DOMAIN_H__

#include "vr_kernel_common.h"
#include "vr_osk.h"

#include "vr_l2_cache.h"
#include "vr_group.h"
#include "vr_pmu.h"

typedef enum {
	VR_PM_DOMAIN_ON,
	VR_PM_DOMAIN_OFF,
} vr_pm_domain_state;

struct vr_pm_domain {
	vr_pm_domain_state state;
	_vr_osk_spinlock_irq_t *lock;

	s32 use_count;

	u32 pmu_mask;

	int group_count;
	struct vr_group *group_list;

	struct vr_l2_cache_core *l2;
};

struct vr_pm_domain *vr_pm_domain_create(u32 pmu_mask);

void vr_pm_domain_add_group(u32 mask, struct vr_group *group);

void vr_pm_domain_add_l2(u32 mask, struct vr_l2_cache_core *l2);
void vr_pm_domain_delete(struct vr_pm_domain *domain);

void vr_pm_domain_terminate(void);

/** Get PM domain from domain ID
 */
struct vr_pm_domain *vr_pm_domain_get_from_mask(u32 mask);
struct vr_pm_domain *vr_pm_domain_get_from_index(u32 id);

/* Ref counting */
void vr_pm_domain_ref_get(struct vr_pm_domain *domain);
void vr_pm_domain_ref_put(struct vr_pm_domain *domain);

VR_STATIC_INLINE struct vr_l2_cache_core *vr_pm_domain_l2_get(struct vr_pm_domain *domain)
{
	return domain->l2;
}

VR_STATIC_INLINE vr_pm_domain_state vr_pm_domain_state_get(struct vr_pm_domain *domain)
{
	return domain->state;
}

vr_bool vr_pm_domain_lock_state(struct vr_pm_domain *domain);
void vr_pm_domain_unlock_state(struct vr_pm_domain *domain);

#define VR_PM_DOMAIN_FOR_EACH_GROUP(group, domain) for ((group) = (domain)->group_list;\
		NULL != (group); (group) = (group)->pm_domain_list)

#endif /* __VR_PM_DOMAIN_H__ */
