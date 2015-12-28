/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_pm_domain.h"
#include "vr_pmu.h"
#include "vr_group.h"

static struct vr_pm_domain *vr_pm_domains[VR_MAX_NUMBER_OF_DOMAINS] = { NULL, };

static void vr_pm_domain_lock(struct vr_pm_domain *domain)
{
	_vr_osk_spinlock_irq_lock(domain->lock);
}

static void vr_pm_domain_unlock(struct vr_pm_domain *domain)
{
	_vr_osk_spinlock_irq_unlock(domain->lock);
}

VR_STATIC_INLINE void vr_pm_domain_state_set(struct vr_pm_domain *domain, vr_pm_domain_state state)
{
	domain->state = state;
}

struct vr_pm_domain *vr_pm_domain_create(u32 pmu_mask)
{
	struct vr_pm_domain* domain = NULL;
	u32 domain_id = 0;

	domain = vr_pm_domain_get_from_mask(pmu_mask);
	if (NULL != domain) return domain;

	VR_DEBUG_PRINT(2, ("Vr PM domain: Creating Vr PM domain (mask=0x%08X)\n", pmu_mask));

	domain = (struct vr_pm_domain *)_vr_osk_malloc(sizeof(struct vr_pm_domain));
	if (NULL != domain) {
		domain->lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_PM_DOMAIN);
		if (NULL == domain->lock) {
			_vr_osk_free(domain);
			return NULL;
		}

		domain->state = VR_PM_DOMAIN_ON;
		domain->pmu_mask = pmu_mask;
		domain->use_count = 0;
		domain->group_list = NULL;
		domain->group_count = 0;
		domain->l2 = NULL;

		domain_id = _vr_osk_fls(pmu_mask) - 1;
		/* Verify the domain_id */
		VR_DEBUG_ASSERT(VR_MAX_NUMBER_OF_DOMAINS > domain_id);
		/* Verify that pmu_mask only one bit is set */
		VR_DEBUG_ASSERT((1 << domain_id) == pmu_mask);
		vr_pm_domains[domain_id] = domain;

		return domain;
	} else {
		VR_DEBUG_PRINT_ERROR(("Unable to create PM domain\n"));
	}

	return NULL;
}

void vr_pm_domain_delete(struct vr_pm_domain *domain)
{
	if (NULL == domain) {
		return;
	}
	_vr_osk_spinlock_irq_term(domain->lock);

	_vr_osk_free(domain);
}

void vr_pm_domain_terminate(void)
{
	int i;

	/* Delete all domains */
	for (i = 0; i < VR_MAX_NUMBER_OF_DOMAINS; i++) {
		vr_pm_domain_delete(vr_pm_domains[i]);
	}
}

void vr_pm_domain_add_group(u32 mask, struct vr_group *group)
{
	struct vr_pm_domain *domain = vr_pm_domain_get_from_mask(mask);
	struct vr_group *next;

	if (NULL == domain) return;

	VR_DEBUG_ASSERT_POINTER(group);

	++domain->group_count;
	next = domain->group_list;

	domain->group_list = group;

	group->pm_domain_list = next;

	vr_group_set_pm_domain(group, domain);

	/* Get pm domain ref after vr_group_set_pm_domain */
	vr_group_get_pm_domain_ref(group);
}

void vr_pm_domain_add_l2(u32 mask, struct vr_l2_cache_core *l2)
{
	struct vr_pm_domain *domain = vr_pm_domain_get_from_mask(mask);

	if (NULL == domain) return;

	VR_DEBUG_ASSERT(NULL == domain->l2);
	VR_DEBUG_ASSERT(NULL != l2);

	domain->l2 = l2;

	vr_l2_cache_set_pm_domain(l2, domain);
}

struct vr_pm_domain *vr_pm_domain_get_from_mask(u32 mask)
{
	u32 id = 0;

	if (0 == mask) return NULL;

	id = _vr_osk_fls(mask)-1;

	VR_DEBUG_ASSERT(VR_MAX_NUMBER_OF_DOMAINS > id);
	/* Verify that pmu_mask only one bit is set */
	VR_DEBUG_ASSERT((1 << id) == mask);

	return vr_pm_domains[id];
}

struct vr_pm_domain *vr_pm_domain_get_from_index(u32 id)
{
	VR_DEBUG_ASSERT(VR_MAX_NUMBER_OF_DOMAINS > id);

	return vr_pm_domains[id];
}

void vr_pm_domain_ref_get(struct vr_pm_domain *domain)
{
	if (NULL == domain) return;

	vr_pm_domain_lock(domain);
	++domain->use_count;

	if (VR_PM_DOMAIN_ON != domain->state) {
		/* Power on */
		struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();

		VR_DEBUG_PRINT(3, ("PM Domain: Powering on 0x%08x\n", domain->pmu_mask));

		if (NULL != pmu) {
			_vr_osk_errcode_t err;

			err = vr_pmu_power_up(pmu, domain->pmu_mask);

			if (_VR_OSK_ERR_OK != err && _VR_OSK_ERR_BUSY != err) {
				VR_PRINT_ERROR(("PM Domain: Failed to power up PM domain 0x%08x\n",
				                  domain->pmu_mask));
			}
		}
		vr_pm_domain_state_set(domain, VR_PM_DOMAIN_ON);
	} else {
		VR_DEBUG_ASSERT(VR_PM_DOMAIN_ON == vr_pm_domain_state_get(domain));
	}

	vr_pm_domain_unlock(domain);
}

void vr_pm_domain_ref_put(struct vr_pm_domain *domain)
{
	if (NULL == domain) return;

	vr_pm_domain_lock(domain);
	--domain->use_count;

	if (0 == domain->use_count && VR_PM_DOMAIN_OFF != domain->state) {
		/* Power off */
		struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();

		VR_DEBUG_PRINT(3, ("PM Domain: Powering off 0x%08x\n", domain->pmu_mask));

		vr_pm_domain_state_set(domain, VR_PM_DOMAIN_OFF);

		if (NULL != pmu) {
			_vr_osk_errcode_t err;

			err = vr_pmu_power_down(pmu, domain->pmu_mask);

			if (_VR_OSK_ERR_OK != err && _VR_OSK_ERR_BUSY != err) {
				VR_PRINT_ERROR(("PM Domain: Failed to power down PM domain 0x%08x\n",
				                  domain->pmu_mask));
			}
		}
	}
	vr_pm_domain_unlock(domain);
}

vr_bool vr_pm_domain_lock_state(struct vr_pm_domain *domain)
{
	vr_bool is_powered = VR_TRUE;

	/* Take a reference without powering on */
	if (NULL != domain) {
		vr_pm_domain_lock(domain);
		++domain->use_count;

		if (VR_PM_DOMAIN_ON != domain->state) {
			is_powered = VR_FALSE;
		}
		vr_pm_domain_unlock(domain);
	}

	if(!_vr_osk_pm_dev_ref_add_no_power_on()) {
		is_powered = VR_FALSE;
	}

	return is_powered;
}

void vr_pm_domain_unlock_state(struct vr_pm_domain *domain)
{
	_vr_osk_pm_dev_ref_dec_no_power_on();

	if (NULL != domain) {
		vr_pm_domain_ref_put(domain);
	}
}
