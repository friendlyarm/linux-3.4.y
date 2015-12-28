/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_PP_SCHEDULER_H__
#define __VR_PP_SCHEDULER_H__

#include "vr_osk.h"
#include "vr_pp_job.h"
#include "vr_group.h"
#include "linux/vr/vr_utgard.h"

/** Initalize the HW independent parts of the  PP scheduler
 */
_vr_osk_errcode_t vr_pp_scheduler_initialize(void);
void vr_pp_scheduler_terminate(void);

/** Poplulate the PP scheduler with groups
 */
void vr_pp_scheduler_populate(void);
void vr_pp_scheduler_depopulate(void);

/**
 * @brief Handle job completion.
 *
 * Will attempt to start a new job on the locked group.
 *
 * If all sub jobs have completed the job's tracker will be released, any other resources associated
 * with the job will be freed.  A notification will also be sent to user space.
 *
 * Releasing the tracker might activate other jobs, so if appropriate we also schedule them.
 *
 * @note Group must be locked when entering this function.  Will be unlocked before exiting.
 *
 * @param group The group that completed the job.
 * @param job The job that is done.
 * @param sub_job Sub job of job.
 * @param success VR_TRUE if job completed successfully, VR_FALSE if not.
 * @param in_upper_half VR_TRUE if called from upper half, VR_FALSE if not.
 */
void vr_pp_scheduler_job_done(struct vr_group *group, struct vr_pp_job *job, u32 sub_job, vr_bool success, vr_bool in_upper_half);

void vr_pp_scheduler_suspend(void);
void vr_pp_scheduler_resume(void);

/**
 * @brief Abort all running and queued PP jobs from session.
 *
 * This functions aborts all PP jobs from the specified session. Queued jobs are removed from the
 * queue and jobs currently running on a core will be aborted.
 *
 * @param session Session that is aborting.
 */
void vr_pp_scheduler_abort_session(struct vr_session_data *session);

/**
 * @brief Reset all groups
 *
 * This function resets all groups known by the PP scheuduler. This must be
 * called after the Vr HW has been powered on in order to reset the HW.
 *
 * This function is intended for power on reset of all cores.
 * No locking is done, which can only be safe if the scheduler is paused and
 * all cores idle. That is always the case on init and power on.
 */
void vr_pp_scheduler_reset_all_groups(void);

/**
 * @brief Zap TLB on all groups with \a session active
 *
 * The scheculer will zap the session on all groups it owns.
 */
void vr_pp_scheduler_zap_all_active(struct vr_session_data *session);

/**
 * @brief Get the virtual PP core
 *
 * The returned PP core may only be used to prepare DMA command buffers for the
 * PP core. Other actions must go through the PP scheduler, or the virtual
 * group.
 *
 * @return Pointer to the virtual PP core, NULL if this doesn't exist
 */
struct vr_pp_core *vr_pp_scheduler_get_virtual_pp(void);

u32 vr_pp_scheduler_dump_state(char *buf, u32 size);

void vr_pp_scheduler_enable_group(struct vr_group *group);
void vr_pp_scheduler_disable_group(struct vr_group *group);

/**
 * @brief Used by the Timeline system to queue a PP job.
 *
 * @note @ref vr_scheduler_schedule_from_mask() should be called if this function returns non-zero.
 *
 * @param job The PP job that is being activated.
 *
 * @return A scheduling bitmask that can be used to decide if scheduling is necessary after this
 * call.
 */
vr_scheduler_mask vr_pp_scheduler_activate_job(struct vr_pp_job *job);

/**
 * @brief Schedule queued jobs on idle cores.
 */
void vr_pp_scheduler_schedule(void);

int vr_pp_scheduler_set_perf_level(u32 cores, vr_bool override);

void vr_pp_scheduler_core_scaling_enable(void);
void vr_pp_scheduler_core_scaling_disable(void);
vr_bool vr_pp_scheduler_core_scaling_is_enabled(void);

u32 vr_pp_scheduler_get_num_cores_total(void);
u32 vr_pp_scheduler_get_num_cores_enabled(void);

/**
 * @brief Returns the number of Pixel Processors in the system irrespective of the context
 *
 * @return number of physical Pixel Processor cores in the system
 */
u32 vr_pp_scheduler_get_num_cores_total(void);

#endif /* __VR_PP_SCHEDULER_H__ */
