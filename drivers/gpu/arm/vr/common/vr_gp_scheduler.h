/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_GP_SCHEDULER_H__
#define __VR_GP_SCHEDULER_H__

#include "vr_osk.h"
#include "vr_gp_job.h"
#include "vr_group.h"

_vr_osk_errcode_t vr_gp_scheduler_initialize(void);
void vr_gp_scheduler_terminate(void);

void vr_gp_scheduler_job_done(struct vr_group *group, struct vr_gp_job *job, vr_bool success);
void vr_gp_scheduler_oom(struct vr_group *group, struct vr_gp_job *job);
u32 vr_gp_scheduler_dump_state(char *buf, u32 size);

void vr_gp_scheduler_suspend(void);
void vr_gp_scheduler_resume(void);

/**
 * @brief Abort all running and queued GP jobs from session.
 *
* This functions aborts all GP jobs from the specified session. Queued jobs are removed from the
* queue and jobs currently running on a core will be aborted.
 *
 * @param session Session that is aborting.
 */
void vr_gp_scheduler_abort_session(struct vr_session_data *session);

/**
 * @brief Reset all groups
 *
 * This function resets all groups known by the GP scheuduler. This must be
 * called after the Vr HW has been powered on in order to reset the HW.
 */
void vr_gp_scheduler_reset_all_groups(void);

/**
 * @brief Zap TLB on all groups with \a session active
 *
 * The scheculer will zap the session on all groups it owns.
 */
void vr_gp_scheduler_zap_all_active(struct vr_session_data *session);

/**
 * @brief Re-enable a group that has been disabled with vr_gp_scheduler_disable_group
 *
 * If a Vr PMU is present, the group will be powered back on and added back
 * into the GP scheduler.
 *
 * @param group Pointer to the group to enable
 */
void vr_gp_scheduler_enable_group(struct vr_group *group);

/**
 * @brief Disable a group
 *
 * The group will be taken out of the GP scheduler and powered off, if a Vr
 * PMU is present.
 *
 * @param group Pointer to the group to disable
 */
void vr_gp_scheduler_disable_group(struct vr_group *group);

/**
 * @brief Used by the Timeline system to queue a GP job.
 *
 * @note @ref vr_scheduler_schedule_from_mask() should be called if this function returns non-zero.
 *
 * @param job The GP job that is being activated.
 *
 * @return A scheduling bitmask that can be used to decide if scheduling is necessary after this
 * call.
 */
vr_scheduler_mask vr_gp_scheduler_activate_job(struct vr_gp_job *job);

/**
 * @brief Schedule queued jobs on idle cores.
 */
void vr_gp_scheduler_schedule(void);

/**
 * @brief Submit a GP job to the GP scheduler.
 *
 * This will add the GP job to the Timeline system.
 *
 * @param session Session this job belongs to.
 * @param job GP job that will be submitted
 * @return Point on GP timeline for job.
 */
vr_timeline_point vr_gp_scheduler_submit_job(struct vr_session_data *session, struct vr_gp_job *job);

#endif /* __VR_GP_SCHEDULER_H__ */
