/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_GP_JOB_H__
#define __VR_GP_JOB_H__

#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_uk_types.h"
#include "vr_session.h"
#include "vr_timeline.h"
#include "vr_scheduler_types.h"

/**
 * The structure represents a GP job, including all sub-jobs
 * (This struct unfortunately needs to be public because of how the _vr_osk_list_*
 * mechanism works)
 */
struct vr_gp_job {
	_vr_osk_list_t list;                             /**< Used to link jobs together in the scheduler queue */
	struct vr_session_data *session;                 /**< Session which submitted this job */
	_vr_uk_gp_start_job_s uargs;                     /**< Arguments from user space */
	u32 id;                                            /**< Identifier for this job in kernel space (sequential numbering) */
	u32 cache_order;                                   /**< Cache order used for L2 cache flushing (sequential numbering) */
	u32 heap_current_addr;                             /**< Holds the current HEAP address when the job has completed */
	u32 perf_counter_value0;                           /**< Value of performance counter 0 (to be returned to user space) */
	u32 perf_counter_value1;                           /**< Value of performance counter 1 (to be returned to user space) */
	u32 pid;                                           /**< Process ID of submitting process */
	u32 tid;                                           /**< Thread ID of submitting thread */
	_vr_osk_notification_t *finished_notification;   /**< Notification sent back to userspace on job complete */
	_vr_osk_notification_t *oom_notification;        /**< Notification sent back to userspace on OOM */
	struct vr_timeline_tracker tracker;              /**< Timeline tracker for this job */
	struct vr_timeline_tracker *pp_tracker;          /**< Pointer to Timeline tracker for PP job that depends on this job. */
};

struct vr_gp_job *vr_gp_job_create(struct vr_session_data *session, _vr_uk_gp_start_job_s *uargs, u32 id, struct vr_timeline_tracker *pp_tracker);
void vr_gp_job_delete(struct vr_gp_job *job);

u32 vr_gp_job_get_gp_counter_src0(void);
void vr_gp_job_set_gp_counter_src0(u32 counter);
u32 vr_gp_job_get_gp_counter_src1(void);
void vr_gp_job_set_gp_counter_src1(u32 counter);

VR_STATIC_INLINE u32 vr_gp_job_get_id(struct vr_gp_job *job)
{
	return (NULL == job) ? 0 : job->id;
}

VR_STATIC_INLINE u32 vr_gp_job_get_cache_order(struct vr_gp_job *job)
{
	return (NULL == job) ? 0 : job->cache_order;
}

VR_STATIC_INLINE u32 vr_gp_job_get_user_id(struct vr_gp_job *job)
{
	return job->uargs.user_job_ptr;
}

VR_STATIC_INLINE u32 vr_gp_job_get_frame_builder_id(struct vr_gp_job *job)
{
	return job->uargs.frame_builder_id;
}

VR_STATIC_INLINE u32 vr_gp_job_get_flush_id(struct vr_gp_job *job)
{
	return job->uargs.flush_id;
}

VR_STATIC_INLINE u32 vr_gp_job_get_pid(struct vr_gp_job *job)
{
	return job->pid;
}

VR_STATIC_INLINE u32 vr_gp_job_get_tid(struct vr_gp_job *job)
{
	return job->tid;
}

VR_STATIC_INLINE u32* vr_gp_job_get_frame_registers(struct vr_gp_job *job)
{
	return job->uargs.frame_registers;
}

VR_STATIC_INLINE struct vr_session_data *vr_gp_job_get_session(struct vr_gp_job *job)
{
	return job->session;
}

VR_STATIC_INLINE vr_bool vr_gp_job_has_vs_job(struct vr_gp_job *job)
{
	return (job->uargs.frame_registers[0] != job->uargs.frame_registers[1]) ? VR_TRUE : VR_FALSE;
}

VR_STATIC_INLINE vr_bool vr_gp_job_has_plbu_job(struct vr_gp_job *job)
{
	return (job->uargs.frame_registers[2] != job->uargs.frame_registers[3]) ? VR_TRUE : VR_FALSE;
}

VR_STATIC_INLINE u32 vr_gp_job_get_current_heap_addr(struct vr_gp_job *job)
{
	return job->heap_current_addr;
}

VR_STATIC_INLINE void vr_gp_job_set_current_heap_addr(struct vr_gp_job *job, u32 heap_addr)
{
	job->heap_current_addr = heap_addr;
}

VR_STATIC_INLINE u32 vr_gp_job_get_perf_counter_flag(struct vr_gp_job *job)
{
	return job->uargs.perf_counter_flag;
}

VR_STATIC_INLINE u32 vr_gp_job_get_perf_counter_src0(struct vr_gp_job *job)
{
	return job->uargs.perf_counter_src0;
}

VR_STATIC_INLINE u32 vr_gp_job_get_perf_counter_src1(struct vr_gp_job *job)
{
	return job->uargs.perf_counter_src1;
}

VR_STATIC_INLINE u32 vr_gp_job_get_perf_counter_value0(struct vr_gp_job *job)
{
	return job->perf_counter_value0;
}

VR_STATIC_INLINE u32 vr_gp_job_get_perf_counter_value1(struct vr_gp_job *job)
{
	return job->perf_counter_value1;
}

VR_STATIC_INLINE void vr_gp_job_set_perf_counter_src0(struct vr_gp_job *job, u32 src)
{
	job->uargs.perf_counter_src0 = src;
}

VR_STATIC_INLINE void vr_gp_job_set_perf_counter_src1(struct vr_gp_job *job, u32 src)
{
	job->uargs.perf_counter_src1 = src;
}

VR_STATIC_INLINE void vr_gp_job_set_perf_counter_value0(struct vr_gp_job *job, u32 value)
{
	job->perf_counter_value0 = value;
}

VR_STATIC_INLINE void vr_gp_job_set_perf_counter_value1(struct vr_gp_job *job, u32 value)
{
	job->perf_counter_value1 = value;
}

/**
 * Returns VR_TRUE if first job is after the second job, ordered by job ID.
 *
 * @param first First job.
 * @param second Second job.
 * @return VR_TRUE if first job should be ordered after the second job, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_gp_job_is_after(struct vr_gp_job *first, struct vr_gp_job *second)
{
	/* A span is used to handle job ID wrapping. */
	return (vr_gp_job_get_id(first) - vr_gp_job_get_id(second)) < VR_SCHEDULER_JOB_ID_SPAN;
}

/**
 * Release reference on tracker for PP job that depends on this GP job.
 *
 * @note If GP job has a reference on tracker, this function MUST be called before the GP job is
 * deleted.
 *
 * @param job GP job that is done.
 * @param success VR_TRUE if job completed successfully, VR_FALSE if not.
 * @return A scheduling bitmask indicating whether scheduling needs to be done.
 */
vr_scheduler_mask vr_gp_job_signal_pp_tracker(struct vr_gp_job *job, vr_bool success);

#endif /* __VR_GP_JOB_H__ */
