/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_gp_job.h"
#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_uk_types.h"

static u32 gp_counter_src0 = VR_HW_CORE_NO_COUNTER;      /**< Performance counter 0, VR_HW_CORE_NO_COUNTER for disabled */
static u32 gp_counter_src1 = VR_HW_CORE_NO_COUNTER;		/**< Performance counter 1, VR_HW_CORE_NO_COUNTER for disabled */

struct vr_gp_job *vr_gp_job_create(struct vr_session_data *session, _vr_uk_gp_start_job_s *uargs, u32 id, struct vr_timeline_tracker *pp_tracker)
{
	struct vr_gp_job *job;
	u32 perf_counter_flag;

	job = _vr_osk_malloc(sizeof(struct vr_gp_job));
	if (NULL != job) {
		job->finished_notification = _vr_osk_notification_create(_VR_NOTIFICATION_GP_FINISHED, sizeof(_vr_uk_gp_job_finished_s));
		if (NULL == job->finished_notification) {
			_vr_osk_free(job);
			return NULL;
		}

		job->oom_notification = _vr_osk_notification_create(_VR_NOTIFICATION_GP_STALLED, sizeof(_vr_uk_gp_job_suspended_s));
		if (NULL == job->oom_notification) {
			_vr_osk_notification_delete(job->finished_notification);
			_vr_osk_free(job);
			return NULL;
		}

		if (0 != _vr_osk_copy_from_user(&job->uargs, uargs, sizeof(_vr_uk_gp_start_job_s))) {
			_vr_osk_notification_delete(job->finished_notification);
			_vr_osk_notification_delete(job->oom_notification);
			_vr_osk_free(job);
			return NULL;
		}

		perf_counter_flag = vr_gp_job_get_perf_counter_flag(job);

		/* case when no counters came from user space
		 * so pass the debugfs / DS-5 provided global ones to the job object */
		if (!((perf_counter_flag & _VR_PERFORMANCE_COUNTER_FLAG_SRC0_ENABLE) ||
		      (perf_counter_flag & _VR_PERFORMANCE_COUNTER_FLAG_SRC1_ENABLE))) {
			vr_gp_job_set_perf_counter_src0(job, vr_gp_job_get_gp_counter_src0());
			vr_gp_job_set_perf_counter_src1(job, vr_gp_job_get_gp_counter_src1());
		}

		_vr_osk_list_init(&job->list);
		job->session = session;
		job->id = id;
		job->heap_current_addr = job->uargs.frame_registers[4];
		job->perf_counter_value0 = 0;
		job->perf_counter_value1 = 0;
		job->pid = _vr_osk_get_pid();
		job->tid = _vr_osk_get_tid();

		job->pp_tracker = pp_tracker;
		if (NULL != job->pp_tracker) {
			/* Take a reference on PP job's tracker that will be released when the GP
			   job is done. */
			vr_timeline_system_tracker_get(session->timeline_system, pp_tracker);
		}

		vr_timeline_tracker_init(&job->tracker, VR_TIMELINE_TRACKER_GP, NULL, job);
		vr_timeline_fence_copy_uk_fence(&(job->tracker.fence), &(job->uargs.fence));

		return job;
	}

	return NULL;
}

void vr_gp_job_delete(struct vr_gp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT(NULL == job->pp_tracker);

	/* de-allocate the pre-allocated oom notifications */
	if (NULL != job->oom_notification) {
		_vr_osk_notification_delete(job->oom_notification);
		job->oom_notification = NULL;
	}
	if (NULL != job->finished_notification) {
		_vr_osk_notification_delete(job->finished_notification);
		job->finished_notification = NULL;
	}

	_vr_osk_free(job);
}

u32 vr_gp_job_get_gp_counter_src0(void)
{
	return gp_counter_src0;
}

void vr_gp_job_set_gp_counter_src0(u32 counter)
{
	gp_counter_src0 = counter;
}

u32 vr_gp_job_get_gp_counter_src1(void)
{
	return gp_counter_src1;
}

void vr_gp_job_set_gp_counter_src1(u32 counter)
{
	gp_counter_src1 = counter;
}

vr_scheduler_mask vr_gp_job_signal_pp_tracker(struct vr_gp_job *job, vr_bool success)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(job);

	if (NULL != job->pp_tracker) {
		schedule_mask |= vr_timeline_system_tracker_put(job->session->timeline_system, job->pp_tracker, VR_FALSE == success);
		job->pp_tracker = NULL;
	}

	return schedule_mask;
}
