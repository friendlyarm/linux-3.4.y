/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_PP_JOB_H__
#define __VR_PP_JOB_H__

#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_uk_types.h"
#include "vr_session.h"
#include "vr_kernel_common.h"
#include "regs/vr_200_regs.h"
#include "vr_kernel_core.h"
#include "vr_dma.h"
#include "vr_dlbu.h"
#include "vr_timeline.h"
#if defined(CONFIG_DMA_SHARED_BUFFER) && !defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
#include "linux/vr_memory_dma_buf.h"
#endif

/**
 * The structure represents a PP job, including all sub-jobs
 * (This struct unfortunately needs to be public because of how the _vr_osk_list_*
 * mechanism works)
 */
struct vr_pp_job {
	_vr_osk_list_t list;                             /**< Used to link jobs together in the scheduler queue */
	struct vr_session_data *session;                 /**< Session which submitted this job */
	_vr_osk_list_t session_list;                     /**< Used to link jobs together in the session job list */
	_vr_osk_list_t session_fb_lookup_list;           /**< Used to link jobs together from the same frame builder in the session */
	_vr_uk_pp_start_job_s uargs;                     /**< Arguments from user space */
	vr_dma_cmd_buf dma_cmd_buf;                      /**< Command buffer for starting job using Vr-450 DMA unit */
	u32 id;                                            /**< Identifier for this job in kernel space (sequential numbering) */
	u32 cache_order;                                   /**< Cache order used for L2 cache flushing (sequential numbering) */
	u32 perf_counter_value0[_VR_PP_MAX_SUB_JOBS];    /**< Value of performance counter 0 (to be returned to user space), one for each sub job */
	u32 perf_counter_value1[_VR_PP_MAX_SUB_JOBS];    /**< Value of performance counter 1 (to be returned to user space), one for each sub job */
	u32 sub_jobs_num;                                  /**< Number of subjobs; set to 1 for Vr-450 if DLBU is used, otherwise equals number of PP cores */
	u32 sub_jobs_started;                              /**< Total number of sub-jobs started (always started in ascending order) */
	u32 sub_jobs_completed;                            /**< Number of completed sub-jobs in this superjob */
	u32 sub_job_errors;                                /**< Bitfield with errors (errors for each single sub-job is or'ed together) */
	u32 pid;                                           /**< Process ID of submitting process */
	u32 tid;                                           /**< Thread ID of submitting thread */
	_vr_osk_notification_t *finished_notification;   /**< Notification sent back to userspace on job complete */
	u32 num_memory_cookies;                            /**< Number of memory cookies attached to job */
	u32 *memory_cookies;                               /**< Memory cookies attached to job */
#if defined(CONFIG_DMA_SHARED_BUFFER) && !defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
	struct vr_dma_buf_attachment **dma_bufs;         /**< Array of DMA-bufs used by job */
	u32 num_dma_bufs;                                  /**< Number of DMA-bufs used by job */
#endif
	struct vr_timeline_tracker tracker;              /**< Timeline tracker for this job */
	u32 perf_counter_per_sub_job_count;                /**< Number of values in the two arrays which is != VR_HW_CORE_NO_COUNTER */
	u32 perf_counter_per_sub_job_src0[_VR_PP_MAX_SUB_JOBS]; /**< Per sub job counters src0 */
	u32 perf_counter_per_sub_job_src1[_VR_PP_MAX_SUB_JOBS]; /**< Per sub job counters src1 */
};

void vr_pp_job_initialize(void);
void vr_pp_job_terminate(void);

struct vr_pp_job *vr_pp_job_create(struct vr_session_data *session, _vr_uk_pp_start_job_s *uargs, u32 id);
void vr_pp_job_delete(struct vr_pp_job *job);

u32 vr_pp_job_get_perf_counter_src0(struct vr_pp_job *job, u32 sub_job);
u32 vr_pp_job_get_perf_counter_src1(struct vr_pp_job *job, u32 sub_job);

void vr_pp_job_set_pp_counter_global_src0(u32 counter);
void vr_pp_job_set_pp_counter_global_src1(u32 counter);
void vr_pp_job_set_pp_counter_sub_job_src0(u32 sub_job, u32 counter);
void vr_pp_job_set_pp_counter_sub_job_src1(u32 sub_job, u32 counter);

u32 vr_pp_job_get_pp_counter_global_src0(void);
u32 vr_pp_job_get_pp_counter_global_src1(void);
u32 vr_pp_job_get_pp_counter_sub_job_src0(u32 sub_job);
u32 vr_pp_job_get_pp_counter_sub_job_src1(u32 sub_job);

VR_STATIC_INLINE u32 vr_pp_job_get_id(struct vr_pp_job *job)
{
	return (NULL == job) ? 0 : job->id;
}

VR_STATIC_INLINE u32 vr_pp_job_get_cache_order(struct vr_pp_job *job)
{
	return (NULL == job) ? 0 : job->cache_order;
}

VR_STATIC_INLINE u32 vr_pp_job_get_user_id(struct vr_pp_job *job)
{
	return job->uargs.user_job_ptr;
}

VR_STATIC_INLINE u32 vr_pp_job_get_frame_builder_id(struct vr_pp_job *job)
{
	return job->uargs.frame_builder_id;
}

VR_STATIC_INLINE u32 vr_pp_job_get_flush_id(struct vr_pp_job *job)
{
	return job->uargs.flush_id;
}

VR_STATIC_INLINE u32 vr_pp_job_get_pid(struct vr_pp_job *job)
{
	return job->pid;
}

VR_STATIC_INLINE u32 vr_pp_job_get_tid(struct vr_pp_job *job)
{
	return job->tid;
}

VR_STATIC_INLINE u32* vr_pp_job_get_frame_registers(struct vr_pp_job *job)
{
	return job->uargs.frame_registers;
}

VR_STATIC_INLINE u32* vr_pp_job_get_dlbu_registers(struct vr_pp_job *job)
{
	return job->uargs.dlbu_registers;
}

VR_STATIC_INLINE vr_bool vr_pp_job_is_virtual(struct vr_pp_job *job)
{
#if defined(CONFIG_VR450)
	return 0 == job->uargs.num_cores;
#else
	return VR_FALSE;
#endif
}

VR_STATIC_INLINE u32 vr_pp_job_get_addr_frame(struct vr_pp_job *job, u32 sub_job)
{
	if (vr_pp_job_is_virtual(job)) {
		return VR_DLBU_VIRT_ADDR;
	} else if (0 == sub_job) {
		return job->uargs.frame_registers[VR200_REG_ADDR_FRAME / sizeof(u32)];
	} else if (sub_job < _VR_PP_MAX_SUB_JOBS) {
		return job->uargs.frame_registers_addr_frame[sub_job - 1];
	}

	return 0;
}

VR_STATIC_INLINE u32 vr_pp_job_get_addr_stack(struct vr_pp_job *job, u32 sub_job)
{
	if (0 == sub_job) {
		return job->uargs.frame_registers[VR200_REG_ADDR_STACK / sizeof(u32)];
	} else if (sub_job < _VR_PP_MAX_SUB_JOBS) {
		return job->uargs.frame_registers_addr_stack[sub_job - 1];
	}

	return 0;
}

VR_STATIC_INLINE u32* vr_pp_job_get_wb0_registers(struct vr_pp_job *job)
{
	return job->uargs.wb0_registers;
}

VR_STATIC_INLINE u32* vr_pp_job_get_wb1_registers(struct vr_pp_job *job)
{
	return job->uargs.wb1_registers;
}

VR_STATIC_INLINE u32* vr_pp_job_get_wb2_registers(struct vr_pp_job *job)
{
	return job->uargs.wb2_registers;
}

VR_STATIC_INLINE void vr_pp_job_disable_wb0(struct vr_pp_job *job)
{
	job->uargs.wb0_registers[VR200_REG_ADDR_WB_SOURCE_SELECT] = 0;
}

VR_STATIC_INLINE void vr_pp_job_disable_wb1(struct vr_pp_job *job)
{
	job->uargs.wb1_registers[VR200_REG_ADDR_WB_SOURCE_SELECT] = 0;
}

VR_STATIC_INLINE void vr_pp_job_disable_wb2(struct vr_pp_job *job)
{
	job->uargs.wb2_registers[VR200_REG_ADDR_WB_SOURCE_SELECT] = 0;
}

VR_STATIC_INLINE vr_bool vr_pp_job_all_writeback_unit_disabled(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);

	if ( job->uargs.wb0_registers[VR200_REG_ADDR_WB_SOURCE_SELECT] ||
	     job->uargs.wb1_registers[VR200_REG_ADDR_WB_SOURCE_SELECT] ||
	     job->uargs.wb2_registers[VR200_REG_ADDR_WB_SOURCE_SELECT]
	   ) {
		/* At least one output unit active */
		return VR_FALSE;
	}

	/* All outputs are disabled - we can abort the job */
	return VR_TRUE;
}

VR_STATIC_INLINE u32 vr_pp_job_get_fb_lookup_id(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);

	return VR_PP_JOB_FB_LOOKUP_LIST_MASK & job->uargs.frame_builder_id;
}

VR_STATIC_INLINE struct vr_session_data *vr_pp_job_get_session(struct vr_pp_job *job)
{
	return job->session;
}

VR_STATIC_INLINE vr_bool vr_pp_job_has_unstarted_sub_jobs(struct vr_pp_job *job)
{
	return (job->sub_jobs_started < job->sub_jobs_num) ? VR_TRUE : VR_FALSE;
}

/* Function used when we are terminating a session with jobs. Return TRUE if it has a rendering job.
   Makes sure that no new subjobs are started. */
VR_STATIC_INLINE void vr_pp_job_mark_unstarted_failed(struct vr_pp_job *job)
{
	u32 jobs_remaining = job->sub_jobs_num - job->sub_jobs_started;
	job->sub_jobs_started   += jobs_remaining;
	job->sub_jobs_completed += jobs_remaining;
	job->sub_job_errors     += jobs_remaining;
}

VR_STATIC_INLINE void vr_pp_job_mark_unstarted_success(struct vr_pp_job *job)
{
	u32 jobs_remaining = job->sub_jobs_num - job->sub_jobs_started;
	job->sub_jobs_started   += jobs_remaining;
	job->sub_jobs_completed += jobs_remaining;
}

VR_STATIC_INLINE vr_bool vr_pp_job_is_complete(struct vr_pp_job *job)
{
	return (job->sub_jobs_num == job->sub_jobs_completed) ? VR_TRUE : VR_FALSE;
}

VR_STATIC_INLINE u32 vr_pp_job_get_first_unstarted_sub_job(struct vr_pp_job *job)
{
	return job->sub_jobs_started;
}

VR_STATIC_INLINE u32 vr_pp_job_get_sub_job_count(struct vr_pp_job *job)
{
	return job->sub_jobs_num;
}

VR_STATIC_INLINE vr_bool vr_pp_job_needs_dma_buf_mapping(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT(job);

	if (0 != job->num_memory_cookies) {
		return VR_TRUE;
	}

	return VR_FALSE;
}

VR_STATIC_INLINE void vr_pp_job_mark_sub_job_started(struct vr_pp_job *job, u32 sub_job)
{
	VR_DEBUG_ASSERT_POINTER(job);

	/* Assert that we are marking the "first unstarted sub job" as started */
	VR_DEBUG_ASSERT(job->sub_jobs_started == sub_job);

	job->sub_jobs_started++;
}

VR_STATIC_INLINE void vr_pp_job_mark_sub_job_completed(struct vr_pp_job *job, vr_bool success)
{
	job->sub_jobs_completed++;
	if ( VR_FALSE == success ) {
		job->sub_job_errors++;
	}
}

VR_STATIC_INLINE vr_bool vr_pp_job_was_success(struct vr_pp_job *job)
{
	if ( 0 == job->sub_job_errors ) {
		return VR_TRUE;
	}
	return VR_FALSE;
}

VR_STATIC_INLINE vr_bool vr_pp_job_use_no_notification(struct vr_pp_job *job)
{
	return job->uargs.flags & _VR_PP_JOB_FLAG_NO_NOTIFICATION ? VR_TRUE : VR_FALSE;
}

VR_STATIC_INLINE u32 vr_pp_job_get_perf_counter_flag(struct vr_pp_job *job)
{
	return job->uargs.perf_counter_flag;
}


VR_STATIC_INLINE u32 vr_pp_job_get_perf_counter_value0(struct vr_pp_job *job, u32 sub_job)
{
	return job->perf_counter_value0[sub_job];
}

VR_STATIC_INLINE u32 vr_pp_job_get_perf_counter_value1(struct vr_pp_job *job, u32 sub_job)
{
	return job->perf_counter_value1[sub_job];
}

VR_STATIC_INLINE void vr_pp_job_set_perf_counter_value0(struct vr_pp_job *job, u32 sub_job, u32 value)
{
	job->perf_counter_value0[sub_job] = value;
}

VR_STATIC_INLINE void vr_pp_job_set_perf_counter_value1(struct vr_pp_job *job, u32 sub_job, u32 value)
{
	job->perf_counter_value1[sub_job] = value;
}

VR_STATIC_INLINE _vr_osk_errcode_t vr_pp_job_check(struct vr_pp_job *job)
{
	if (vr_pp_job_is_virtual(job) && job->sub_jobs_num != 1) {
		return _VR_OSK_ERR_FAULT;
	}
	return _VR_OSK_ERR_OK;
}

/**
 * Returns VR_TRUE if first job should be started after second job.
 *
 * @param first First job.
 * @param second Second job.
 * @return VR_TRUE if first job should be started after second job, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_pp_job_should_start_after(struct vr_pp_job *first, struct vr_pp_job *second)
{
	VR_DEBUG_ASSERT_POINTER(first);
	VR_DEBUG_ASSERT_POINTER(second);

	/* First job should be started after second job if second job is in progress. */
	if (0 < second->sub_jobs_started) {
		return VR_TRUE;
	}

	/* First job should be started after second job if first job has a higher job id.  A span is
	   used to handle job id wrapping. */
	if ((vr_pp_job_get_id(first) - vr_pp_job_get_id(second)) < VR_SCHEDULER_JOB_ID_SPAN) {
		return VR_TRUE;
	}

	/* Second job should be started after first job. */
	return VR_FALSE;
}

/**
 * Returns VR_TRUE if this job has more than two sub jobs and all sub jobs are unstarted.
 *
 * @param job Job to check.
 * @return VR_TRUE if job has more than two sub jobs and all sub jobs are unstarted, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_pp_job_is_large_and_unstarted(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT(!vr_pp_job_is_virtual(job));

	return (0 == job->sub_jobs_started && 2 < job->sub_jobs_num);
}

/**
 * Get PP job's Timeline tracker.
 *
 * @param job PP job.
 * @return Pointer to Timeline tracker for the job.
 */
VR_STATIC_INLINE struct vr_timeline_tracker *vr_pp_job_get_tracker(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	return &(job->tracker);
}

#endif /* __VR_PP_JOB_H__ */
