/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_GROUP_H__
#define __VR_GROUP_H__

#include "linux/jiffies.h"
#include "vr_osk.h"
#include "vr_l2_cache.h"
#include "vr_mmu.h"
#include "vr_gp.h"
#include "vr_pp.h"
#include "vr_session.h"

/**
 * @brief Default max runtime [ms] for a core job - used by timeout timers
 */
#define VR_MAX_JOB_RUNTIME_DEFAULT 4000

/** @brief A vr group object represents a MMU and a PP and/or a GP core.
 *
 */
#define VR_MAX_NUMBER_OF_GROUPS 10

enum vr_group_core_state {
	VR_GROUP_STATE_IDLE,
	VR_GROUP_STATE_WORKING,
	VR_GROUP_STATE_OOM,
	VR_GROUP_STATE_IN_VIRTUAL,
	VR_GROUP_STATE_JOINING_VIRTUAL,
	VR_GROUP_STATE_LEAVING_VIRTUAL,
	VR_GROUP_STATE_DISABLED,
};

/* Forward declaration from vr_pm_domain.h */
struct vr_pm_domain;

/**
 * The structure represents a render group
 * A render group is defined by all the cores that share the same Vr MMU
 */

struct vr_group {
	struct vr_mmu_core        *mmu;
	struct vr_session_data    *session;

	vr_bool                   power_is_on;
	enum vr_group_core_state  state;

	struct vr_gp_core         *gp_core;
	struct vr_gp_job          *gp_running_job;

	struct vr_pp_core         *pp_core;
	struct vr_pp_job          *pp_running_job;
	u32                         pp_running_sub_job;

	struct vr_l2_cache_core   *l2_cache_core[2];
	u32                         l2_cache_core_ref_count[2];

	struct vr_dlbu_core       *dlbu_core;
	struct vr_bcast_unit      *bcast_core;

#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_t	*lock;
#else
	_vr_osk_spinlock_t		*lock;
#endif

	_vr_osk_list_t            pp_scheduler_list;

	/* List used for virtual groups. For a virtual group, the list represents the
	 * head element. */
	_vr_osk_list_t            group_list;

	struct vr_group           *pm_domain_list;
	struct vr_pm_domain       *pm_domain;

	/* Parent virtual group (if any) */
	struct vr_group           *parent_group;

	_vr_osk_wq_work_t         *bottom_half_work_mmu;
	_vr_osk_wq_work_t         *bottom_half_work_gp;
	_vr_osk_wq_work_t         *bottom_half_work_pp;

	_vr_osk_timer_t           *timeout_timer;
	vr_bool                   core_timed_out;
};

/** @brief Create a new Vr group object
 *
 * @param cluster Pointer to the cluster to which the group is connected.
 * @param mmu Pointer to the MMU that defines this group
 * @return A pointer to a new group object
 */
struct vr_group *vr_group_create(struct vr_l2_cache_core *core,
                                     struct vr_dlbu_core *dlbu,
                                     struct vr_bcast_unit *bcast);

_vr_osk_errcode_t vr_group_add_mmu_core(struct vr_group *group, struct vr_mmu_core* mmu_core);
void vr_group_remove_mmu_core(struct vr_group *group);

_vr_osk_errcode_t vr_group_add_gp_core(struct vr_group *group, struct vr_gp_core* gp_core);
void vr_group_remove_gp_core(struct vr_group *group);

_vr_osk_errcode_t vr_group_add_pp_core(struct vr_group *group, struct vr_pp_core* pp_core);
void vr_group_remove_pp_core(struct vr_group *group);

void vr_group_set_pm_domain(struct vr_group *group, struct vr_pm_domain *domain);

void vr_group_delete(struct vr_group *group);

/** @brief Virtual groups */
void vr_group_add_group(struct vr_group *parent, struct vr_group *child, vr_bool update_hw);
void vr_group_remove_group(struct vr_group *parent, struct vr_group *child);
struct vr_group *vr_group_acquire_group(struct vr_group *parent);

VR_STATIC_INLINE vr_bool vr_group_is_virtual(struct vr_group *group)
{
#if defined(CONFIG_VR450)
	return (NULL != group->dlbu_core);
#else
	return VR_FALSE;
#endif
}

/** @brief Check if a group is considered as part of a virtual group
 *
 * @note A group is considered to be "part of" a virtual group also during the transition
 *       in to / out of the virtual group.
 */
VR_STATIC_INLINE vr_bool vr_group_is_in_virtual(struct vr_group *group)
{
#if defined(CONFIG_VR450)
	return (VR_GROUP_STATE_IN_VIRTUAL == group->state ||
	        VR_GROUP_STATE_JOINING_VIRTUAL == group->state ||
	        VR_GROUP_STATE_LEAVING_VIRTUAL == group->state);
#else
	return VR_FALSE;
#endif
}

/** @brief Reset group
 *
 * This function will reset the entire group, including all the cores present in the group.
 *
 * @param group Pointer to the group to reset
 */
void vr_group_reset(struct vr_group *group);

/** @brief Zap MMU TLB on all groups
 *
 * Zap TLB on group if \a session is active.
 */
void vr_group_zap_session(struct vr_group* group, struct vr_session_data *session);

/** @brief Get pointer to GP core object
 */
struct vr_gp_core* vr_group_get_gp_core(struct vr_group *group);

/** @brief Get pointer to PP core object
 */
struct vr_pp_core* vr_group_get_pp_core(struct vr_group *group);

/** @brief Lock group object
 *
 * Most group functions will lock the group object themselves. The expection is
 * the group_bottom_half which requires the group to be locked on entry.
 *
 * @param group Pointer to group to lock
 */
void vr_group_lock(struct vr_group *group);

/** @brief Unlock group object
 *
 * @param group Pointer to group to unlock
 */
void vr_group_unlock(struct vr_group *group);
#ifdef DEBUG
void vr_group_assert_locked(struct vr_group *group);
#define VR_ASSERT_GROUP_LOCKED(group) vr_group_assert_locked(group)
#else
#define VR_ASSERT_GROUP_LOCKED(group)
#endif

/** @brief Start GP job
 */
void vr_group_start_gp_job(struct vr_group *group, struct vr_gp_job *job);
/** @brief Start fragment of PP job
 */
void vr_group_start_pp_job(struct vr_group *group, struct vr_pp_job *job, u32 sub_job);

/** @brief Resume GP job that suspended waiting for more heap memory
 */
struct vr_gp_job *vr_group_resume_gp_with_new_heap(struct vr_group *group, u32 job_id, u32 start_addr, u32 end_addr);
/** @brief Abort GP job
 *
 * Used to abort suspended OOM jobs when user space failed to allocte more memory.
 */
void vr_group_abort_gp_job(struct vr_group *group, u32 job_id);
/** @brief Abort all GP jobs from \a session
 *
 * Used on session close when terminating all running and queued jobs from \a session.
 */
void vr_group_abort_session(struct vr_group *group, struct vr_session_data *session);

vr_bool vr_group_power_is_on(struct vr_group *group);
void vr_group_power_on_group(struct vr_group *group);
void vr_group_power_off_group(struct vr_group *group, vr_bool power_status);
void vr_group_power_on(void);

/** @brief Prepare group for power off
 *
 * Update the group's state and prepare for the group to be powered off.
 *
 * If do_power_change is VR_FALSE group session will be set to NULL so that
 * no more activity will happen to this group, but the power state flag will be
 * left unchanged.
 *
 * @do_power_change VR_TRUE if power status is to be updated
 */
void vr_group_power_off(vr_bool do_power_change);

struct vr_group *vr_group_get_glob_group(u32 index);
u32 vr_group_get_glob_num_groups(void);

u32 vr_group_dump_state(struct vr_group *group, char *buf, u32 size);

/* MMU-related functions */
_vr_osk_errcode_t vr_group_upper_half_mmu(void * data);

/* GP-related functions */
_vr_osk_errcode_t vr_group_upper_half_gp(void *data);

/* PP-related functions */
_vr_osk_errcode_t vr_group_upper_half_pp(void *data);

/** @brief Check if group is enabled
 *
 * @param group group to check
 * @return VR_TRUE if enabled, VR_FALSE if not
 */
vr_bool vr_group_is_enabled(struct vr_group *group);

/** @brief Enable group
 *
 * An enabled job is put on the idle scheduler list and can be used to handle jobs.  Does nothing if
 * group is already enabled.
 *
 * @param group group to enable
 */
void vr_group_enable(struct vr_group *group);

/** @brief Disable group
 *
 * A disabled group will no longer be used by the scheduler.  If part of a virtual group, the group
 * will be removed before being disabled.  Cores part of a disabled group is safe to power down.
 *
 * @param group group to disable
 */
void vr_group_disable(struct vr_group *group);

VR_STATIC_INLINE vr_bool vr_group_virtual_disable_if_empty(struct vr_group *group)
{
	vr_bool empty = VR_FALSE;

	VR_ASSERT_GROUP_LOCKED(group);
	VR_DEBUG_ASSERT(vr_group_is_virtual(group));

	if (_vr_osk_list_empty(&group->group_list)) {
		group->state = VR_GROUP_STATE_DISABLED;
		group->session = NULL;

		empty = VR_TRUE;
	}

	return empty;
}

VR_STATIC_INLINE vr_bool vr_group_virtual_enable_if_empty(struct vr_group *group)
{
	vr_bool empty = VR_FALSE;

	VR_ASSERT_GROUP_LOCKED(group);
	VR_DEBUG_ASSERT(vr_group_is_virtual(group));

	if (_vr_osk_list_empty(&group->group_list)) {
		VR_DEBUG_ASSERT(VR_GROUP_STATE_DISABLED == group->state);

		group->state = VR_GROUP_STATE_IDLE;

		empty = VR_TRUE;
	}

	return empty;
}

/* Get group used l2 domain and core domain ref */
void vr_group_get_pm_domain_ref(struct vr_group *group);
/* Put group used l2 domain and core domain ref */
void vr_group_put_pm_domain_ref(struct vr_group *group);

#endif /* __VR_GROUP_H__ */
