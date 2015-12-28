/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_pp_scheduler.h"
#include "vr_kernel_common.h"
#include "vr_kernel_core.h"
#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_scheduler.h"
#include "vr_pp.h"
#include "vr_pp_job.h"
#include "vr_group.h"
#include "vr_pm.h"
#include "vr_timeline.h"
#include "vr_osk_profiling.h"
#include "vr_kernel_utilization.h"
#include "vr_session.h"
#include "vr_pm_domain.h"
#include "linux/vr/vr_utgard.h"

#if defined(CONFIG_DMA_SHARED_BUFFER)
#include "vr_memory_dma_buf.h"
#endif
#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
#include <linux/sched.h>
#include <trace/events/gpu.h>
#endif

/* Queue type used for physical and virtual job queues. */
struct vr_pp_scheduler_job_queue {
	_VR_OSK_LIST_HEAD(normal_pri); /* List of jobs with some unscheduled work. */
	_VR_OSK_LIST_HEAD(high_pri);   /* List of high priority jobs with some unscheduled work. */
	u32 depth;                       /* Depth of combined queues. */
};

/* If dma_buf with map on demand is used, we defer job deletion and job queue if in atomic context,
 * since both might sleep. */
#if defined(CONFIG_DMA_SHARED_BUFFER) && !defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
#define VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE 1
#define VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE 1
#endif /* !defined(CONFIG_DMA_SHARED_BUFFER) && !defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH) */

static void vr_pp_scheduler_job_queued(void);
static void vr_pp_scheduler_job_completed(void);

/* Maximum of 8 PP cores (a group can only have maximum of 1 PP core) */
#define VR_MAX_NUMBER_OF_PP_GROUPS 9

static vr_bool vr_pp_scheduler_is_suspended(void *data);

static u32 pp_version = 0;

/* Physical job queue */
static struct vr_pp_scheduler_job_queue job_queue;

/* Physical groups */
static _VR_OSK_LIST_HEAD_STATIC_INIT(group_list_working);     /* List of physical groups with working jobs on the pp core */
static _VR_OSK_LIST_HEAD_STATIC_INIT(group_list_idle);        /* List of physical groups with idle jobs on the pp core */
static _VR_OSK_LIST_HEAD_STATIC_INIT(group_list_disabled);    /* List of disabled physical groups */

/* Virtual job queue (Vr-450 only) */
static struct vr_pp_scheduler_job_queue virtual_job_queue;

/**
 * Add job to scheduler queue.
 *
 * @param job Job to queue.
 * @return Schedule mask.
 */
static vr_scheduler_mask vr_pp_scheduler_queue_job(struct vr_pp_job *job);

/* Virtual group (Vr-450 only) */
static struct vr_group *virtual_group = NULL;                 /* Virtual group (if any) */
static enum {
	VIRTUAL_GROUP_IDLE,
	VIRTUAL_GROUP_WORKING,
	VIRTUAL_GROUP_DISABLED,
}
virtual_group_state = VIRTUAL_GROUP_IDLE;            /* Flag which indicates whether the virtual group is working or idle */

/* Number of physical cores */
static u32 num_cores = 0;

/* Number of physical cores which are enabled */
static u32 enabled_cores = 0;

/* Enable or disable core scaling */
static vr_bool core_scaling_enabled = VR_TRUE;

/* Variables to allow safe pausing of the scheduler */
static _vr_osk_wait_queue_t *pp_scheduler_working_wait_queue = NULL;
static u32 pause_count = 0;

#if defined(VR_UPPER_HALF_SCHEDULING)
static _vr_osk_spinlock_irq_t *pp_scheduler_lock = NULL;
#else
static _vr_osk_spinlock_t *pp_scheduler_lock = NULL;
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */

VR_STATIC_INLINE void vr_pp_scheduler_lock(void)
{
#if defined(VR_UPPER_HALF_SCHEDULING)
	_vr_osk_spinlock_irq_lock(pp_scheduler_lock);
#else
	_vr_osk_spinlock_lock(pp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
	VR_DEBUG_PRINT(5, ("Vr PP scheduler: PP scheduler lock taken.\n"));
}

VR_STATIC_INLINE void vr_pp_scheduler_unlock(void)
{
	VR_DEBUG_PRINT(5, ("Vr PP scheduler: Releasing PP scheduler lock.\n"));
#if defined(VR_UPPER_HALF_SCHEDULING)
	_vr_osk_spinlock_irq_unlock(pp_scheduler_lock);
#else
	_vr_osk_spinlock_unlock(pp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
}

#if defined(DEBUG)
#define VR_ASSERT_PP_SCHEDULER_LOCKED() VR_DEBUG_ASSERT_LOCK_HELD(pp_scheduler_lock)
#else
#define VR_ASSERT_PP_SCHEDULER_LOCKED() do {} while (0)
#endif /* defined(DEBUG) */

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)

static _vr_osk_wq_work_t *pp_scheduler_wq_job_delete = NULL;
static _vr_osk_spinlock_irq_t *pp_scheduler_job_delete_lock = NULL;
static _VR_OSK_LIST_HEAD_STATIC_INIT(pp_scheduler_job_deletion_queue);

static void vr_pp_scheduler_deferred_job_delete(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);

	_vr_osk_spinlock_irq_lock(pp_scheduler_job_delete_lock);

	/* This job object should not be on any lists. */
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->list));
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->session_list));
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->session_fb_lookup_list));

	_vr_osk_list_addtail(&job->list, &pp_scheduler_job_deletion_queue);

	_vr_osk_spinlock_irq_unlock(pp_scheduler_job_delete_lock);

	_vr_osk_wq_schedule_work(pp_scheduler_wq_job_delete);
}

static void vr_pp_scheduler_do_job_delete(void *arg)
{
	_VR_OSK_LIST_HEAD_STATIC_INIT(list);
	struct vr_pp_job *job;
	struct vr_pp_job *tmp;

	VR_IGNORE(arg);

	_vr_osk_spinlock_irq_lock(pp_scheduler_job_delete_lock);

	/*
	 * Quickly "unhook" the jobs pending to be deleted, so we can release the lock before
	 * we start deleting the job objects (without any locks held
	 */
	_vr_osk_list_move_list(&pp_scheduler_job_deletion_queue, &list);

	_vr_osk_spinlock_irq_unlock(pp_scheduler_job_delete_lock);

	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &list, struct vr_pp_job, list) {
		vr_pp_job_delete(job); /* delete the job object itself */
	}
}

#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE) */

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE)

static _vr_osk_wq_work_t *pp_scheduler_wq_job_queue = NULL;
static _vr_osk_spinlock_irq_t *pp_scheduler_job_queue_lock = NULL;
static _VR_OSK_LIST_HEAD_STATIC_INIT(pp_scheduler_job_queue_list);

static void vr_pp_scheduler_deferred_job_queue(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);

	_vr_osk_spinlock_irq_lock(pp_scheduler_job_queue_lock);
	_vr_osk_list_addtail(&job->list, &pp_scheduler_job_queue_list);
	_vr_osk_spinlock_irq_unlock(pp_scheduler_job_queue_lock);

	_vr_osk_wq_schedule_work(pp_scheduler_wq_job_queue);
}

static void vr_pp_scheduler_do_job_queue(void *arg)
{
	_VR_OSK_LIST_HEAD_STATIC_INIT(list);
	struct vr_pp_job *job;
	struct vr_pp_job *tmp;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_IGNORE(arg);

	_vr_osk_spinlock_irq_lock(pp_scheduler_job_queue_lock);

	/*
	 * Quickly "unhook" the jobs pending to be queued, so we can release the lock before
	 * we start queueing the job objects (without any locks held)
	 */
	_vr_osk_list_move_list(&pp_scheduler_job_queue_list, &list);

	_vr_osk_spinlock_irq_unlock(pp_scheduler_job_queue_lock);

	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &list, struct vr_pp_job, list) {
		_vr_osk_list_delinit(&job->list);
		schedule_mask |= vr_pp_scheduler_queue_job(job);
	}

	vr_scheduler_schedule_from_mask(schedule_mask, VR_FALSE);
}

#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE) */

VR_STATIC_INLINE vr_bool vr_pp_scheduler_has_virtual_group(void)
{
#if defined(CONFIG_VR450)
	return NULL != virtual_group;
#else
	return VR_FALSE;
#endif /* defined(CONFIG_VR450) */
}

_vr_osk_errcode_t vr_pp_scheduler_initialize(void)
{
	_VR_OSK_INIT_LIST_HEAD(&job_queue.normal_pri);
	_VR_OSK_INIT_LIST_HEAD(&job_queue.high_pri);
	job_queue.depth = 0;

	_VR_OSK_INIT_LIST_HEAD(&virtual_job_queue.normal_pri);
	_VR_OSK_INIT_LIST_HEAD(&virtual_job_queue.high_pri);
	virtual_job_queue.depth = 0;

#if defined(VR_UPPER_HALF_SCHEDULING)
	pp_scheduler_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER);
#else
	pp_scheduler_lock = _vr_osk_spinlock_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
	if (NULL == pp_scheduler_lock) goto cleanup;

	pp_scheduler_working_wait_queue = _vr_osk_wait_queue_init();
	if (NULL == pp_scheduler_working_wait_queue) goto cleanup;

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)
	pp_scheduler_wq_job_delete = _vr_osk_wq_create_work(vr_pp_scheduler_do_job_delete, NULL);
	if (NULL == pp_scheduler_wq_job_delete) goto cleanup;

	pp_scheduler_job_delete_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER_DEFERRED);
	if (NULL == pp_scheduler_job_delete_lock) goto cleanup;
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE) */

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE)
	pp_scheduler_wq_job_queue = _vr_osk_wq_create_work(vr_pp_scheduler_do_job_queue, NULL);
	if (NULL == pp_scheduler_wq_job_queue) goto cleanup;

	pp_scheduler_job_queue_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_SCHEDULER_DEFERRED);
	if (NULL == pp_scheduler_job_queue_lock) goto cleanup;
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE) */

	return _VR_OSK_ERR_OK;

cleanup:
#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE)
	if (NULL != pp_scheduler_job_queue_lock) {
		_vr_osk_spinlock_irq_term(pp_scheduler_job_queue_lock);
		pp_scheduler_job_queue_lock = NULL;
	}

	if (NULL != pp_scheduler_wq_job_queue) {
		_vr_osk_wq_delete_work(pp_scheduler_wq_job_queue);
		pp_scheduler_wq_job_queue = NULL;
	}
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE) */

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)
	if (NULL != pp_scheduler_job_delete_lock) {
		_vr_osk_spinlock_irq_term(pp_scheduler_job_delete_lock);
		pp_scheduler_job_delete_lock = NULL;
	}

	if (NULL != pp_scheduler_wq_job_delete) {
		_vr_osk_wq_delete_work(pp_scheduler_wq_job_delete);
		pp_scheduler_wq_job_delete = NULL;
	}
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE) */

	if (NULL != pp_scheduler_working_wait_queue) {
		_vr_osk_wait_queue_term(pp_scheduler_working_wait_queue);
		pp_scheduler_working_wait_queue = NULL;
	}

	if (NULL != pp_scheduler_lock) {
#if defined(VR_UPPER_HALF_SCHEDULING)
		_vr_osk_spinlock_irq_term(pp_scheduler_lock);
#else
		_vr_osk_spinlock_term(pp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
		pp_scheduler_lock = NULL;
	}

	return _VR_OSK_ERR_NOMEM;
}

void vr_pp_scheduler_terminate(void)
{
#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE)
	_vr_osk_spinlock_irq_term(pp_scheduler_job_queue_lock);
	_vr_osk_wq_delete_work(pp_scheduler_wq_job_queue);
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE) */

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)
	_vr_osk_spinlock_irq_term(pp_scheduler_job_delete_lock);
	_vr_osk_wq_delete_work(pp_scheduler_wq_job_delete);
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE) */

	_vr_osk_wait_queue_term(pp_scheduler_working_wait_queue);

#if defined(VR_UPPER_HALF_SCHEDULING)
	_vr_osk_spinlock_irq_term(pp_scheduler_lock);
#else
	_vr_osk_spinlock_term(pp_scheduler_lock);
#endif /* defined(VR_UPPER_HALF_SCHEDULING) */
}

void vr_pp_scheduler_populate(void)
{
	struct vr_group *group;
	struct vr_pp_core *pp_core;
	u32 num_groups;
	u32 i;

	num_groups = vr_group_get_glob_num_groups();

	/* Do we have a virtual group? */
	for (i = 0; i < num_groups; i++) {
		group = vr_group_get_glob_group(i);

		if (vr_group_is_virtual(group)) {
			VR_DEBUG_PRINT(3, ("Vr PP scheduler: Found virtual group %p.\n", group));

			virtual_group = group;
			break;
		}
	}

	/* Find all the available PP cores */
	for (i = 0; i < num_groups; i++) {
		group = vr_group_get_glob_group(i);
		pp_core = vr_group_get_pp_core(group);

		if (NULL != pp_core && !vr_group_is_virtual(group)) {
			if (0 == pp_version) {
				/* Retrieve PP version from the first available PP core */
				pp_version = vr_pp_core_get_version(pp_core);
			}

			if (vr_pp_scheduler_has_virtual_group()) {
				/* Add all physical PP cores to the virtual group */
				vr_group_lock(virtual_group);
				group->state = VR_GROUP_STATE_JOINING_VIRTUAL;
				vr_group_add_group(virtual_group, group, VR_TRUE);
				vr_group_unlock(virtual_group);
			} else {
				_vr_osk_list_add(&group->pp_scheduler_list, &group_list_idle);
			}

			num_cores++;
		}
	}

	enabled_cores = num_cores;
}

void vr_pp_scheduler_depopulate(void)
{
	struct vr_group *group, *temp;

	VR_DEBUG_ASSERT(_vr_osk_list_empty(&group_list_working));
	VR_DEBUG_ASSERT(VIRTUAL_GROUP_WORKING != virtual_group_state);

	/* Delete all groups owned by scheduler */
	if (vr_pp_scheduler_has_virtual_group()) {
		vr_group_delete(virtual_group);
	}

	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_idle, struct vr_group, pp_scheduler_list) {
		vr_group_delete(group);
	}
	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_disabled, struct vr_group, pp_scheduler_list) {
		vr_group_delete(group);
	}
}

VR_STATIC_INLINE void vr_pp_scheduler_disable_empty_virtual(void)
{
	VR_ASSERT_GROUP_LOCKED(virtual_group);

	if (vr_group_virtual_disable_if_empty(virtual_group)) {
		VR_DEBUG_PRINT(4, ("Disabling empty virtual group\n"));

		VR_DEBUG_ASSERT(VIRTUAL_GROUP_IDLE == virtual_group_state);

		virtual_group_state = VIRTUAL_GROUP_DISABLED;
	}
}

VR_STATIC_INLINE void vr_pp_scheduler_enable_empty_virtual(void)
{
	VR_ASSERT_GROUP_LOCKED(virtual_group);

	if (vr_group_virtual_enable_if_empty(virtual_group)) {
		VR_DEBUG_PRINT(4, ("Re-enabling empty virtual group\n"));

		VR_DEBUG_ASSERT(VIRTUAL_GROUP_DISABLED == virtual_group_state);

		virtual_group_state = VIRTUAL_GROUP_IDLE;
	}
}

static struct vr_pp_job *vr_pp_scheduler_get_job(struct vr_pp_scheduler_job_queue *queue)
{
	struct vr_pp_job *job = NULL;

	VR_ASSERT_PP_SCHEDULER_LOCKED();
	VR_DEBUG_ASSERT_POINTER(queue);

	/* Check if we have a normal priority job. */
	if (!_vr_osk_list_empty(&queue->normal_pri)) {
		VR_DEBUG_ASSERT(queue->depth > 0);
		job = _VR_OSK_LIST_ENTRY(queue->normal_pri.next, struct vr_pp_job, list);
	}

	/* Prefer normal priority job if it is in progress. */
	if (NULL != job && 0 < job->sub_jobs_started) {
		return job;
	}

	/* Check if we have a high priority job. */
	if (!_vr_osk_list_empty(&queue->high_pri)) {
		VR_DEBUG_ASSERT(queue->depth > 0);
		job = _VR_OSK_LIST_ENTRY(queue->high_pri.next, struct vr_pp_job, list);
	}

	return job;
}

/**
 * Returns a physical job if a physical job is ready to run
 */
VR_STATIC_INLINE struct vr_pp_job *vr_pp_scheduler_get_physical_job(void)
{
	VR_ASSERT_PP_SCHEDULER_LOCKED();
	return vr_pp_scheduler_get_job(&job_queue);
}

VR_STATIC_INLINE void vr_pp_scheduler_dequeue_physical_job(struct vr_pp_job *job)
{
	VR_ASSERT_PP_SCHEDULER_LOCKED();
	VR_DEBUG_ASSERT(job_queue.depth > 0);

	/* Remove job from queue */
	if (!vr_pp_job_has_unstarted_sub_jobs(job)) {
		/* All sub jobs have been started: remove job from queue */
		_vr_osk_list_delinit(&job->list);
		_vr_osk_list_delinit(&job->session_fb_lookup_list);
	}

	--job_queue.depth;
}

/**
 * Returns a virtual job if a virtual job is ready to run
 */
VR_STATIC_INLINE struct vr_pp_job *vr_pp_scheduler_get_virtual_job(void)
{
	VR_ASSERT_PP_SCHEDULER_LOCKED();
	VR_DEBUG_ASSERT_POINTER(virtual_group);
	return vr_pp_scheduler_get_job(&virtual_job_queue);
}

VR_STATIC_INLINE void vr_pp_scheduler_dequeue_virtual_job(struct vr_pp_job *job)
{
	VR_ASSERT_PP_SCHEDULER_LOCKED();
	VR_DEBUG_ASSERT(virtual_job_queue.depth > 0);

	/* Remove job from queue */
	_vr_osk_list_delinit(&job->list);
	_vr_osk_list_delinit(&job->session_fb_lookup_list);
	--virtual_job_queue.depth;
}

/**
 * Checks if the criteria is met for removing a physical core from virtual group
 */
VR_STATIC_INLINE vr_bool vr_pp_scheduler_can_move_virtual_to_physical(void)
{
	VR_ASSERT_PP_SCHEDULER_LOCKED();
	VR_DEBUG_ASSERT(vr_pp_scheduler_has_virtual_group());
	VR_ASSERT_GROUP_LOCKED(virtual_group);
	/*
	 * The criteria for taking out a physical group from a virtual group are the following:
	 * - There virtual group is idle
	 * - There are currently no physical groups (idle and working)
	 * - There are physical jobs to be scheduled
	 */
	return (VIRTUAL_GROUP_IDLE == virtual_group_state) &&
	       _vr_osk_list_empty(&group_list_idle) &&
	       _vr_osk_list_empty(&group_list_working) &&
	       (NULL != vr_pp_scheduler_get_physical_job());
}

VR_STATIC_INLINE struct vr_group *vr_pp_scheduler_acquire_physical_group(void)
{
	VR_ASSERT_PP_SCHEDULER_LOCKED();

	if (!_vr_osk_list_empty(&group_list_idle)) {
		VR_DEBUG_PRINT(4, ("Vr PP scheduler: Acquiring physical group from idle list.\n"));
		return _VR_OSK_LIST_ENTRY(group_list_idle.next, struct vr_group, pp_scheduler_list);
	} else if (vr_pp_scheduler_has_virtual_group()) {
		VR_ASSERT_GROUP_LOCKED(virtual_group);
		if (vr_pp_scheduler_can_move_virtual_to_physical()) {
			struct vr_group *group;
			VR_DEBUG_PRINT(4, ("Vr PP scheduler: Acquiring physical group from virtual group.\n"));
			group = vr_group_acquire_group(virtual_group);

			if (vr_pp_scheduler_has_virtual_group()) {
				vr_pp_scheduler_disable_empty_virtual();
			}

			return group;
		}
	}

	return NULL;
}

static void vr_pp_scheduler_return_job_to_user(struct vr_pp_job *job, vr_bool deferred)
{
	if (VR_FALSE == vr_pp_job_use_no_notification(job)) {
		u32 i;
		u32 num_counters_to_copy;
		vr_bool success = vr_pp_job_was_success(job);

		_vr_uk_pp_job_finished_s *jobres = job->finished_notification->result_buffer;
		_vr_osk_memset(jobres, 0, sizeof(_vr_uk_pp_job_finished_s)); /* @@@@ can be removed once we initialize all members in this struct */
		jobres->user_job_ptr = vr_pp_job_get_user_id(job);
		if (VR_TRUE == success) {
			jobres->status = _VR_UK_JOB_STATUS_END_SUCCESS;
		} else {
			jobres->status = _VR_UK_JOB_STATUS_END_UNKNOWN_ERR;
		}

		if (vr_pp_job_is_virtual(job)) {
			num_counters_to_copy = num_cores; /* Number of physical cores available */
		} else {
			num_counters_to_copy = vr_pp_job_get_sub_job_count(job);
		}

		for (i = 0; i < num_counters_to_copy; i++) {
			jobres->perf_counter0[i] = vr_pp_job_get_perf_counter_value0(job, i);
			jobres->perf_counter1[i] = vr_pp_job_get_perf_counter_value1(job, i);
			jobres->perf_counter_src0 = vr_pp_job_get_pp_counter_global_src0();
			jobres->perf_counter_src1 = vr_pp_job_get_pp_counter_global_src1();
		}

		vr_session_send_notification(vr_pp_job_get_session(job), job->finished_notification);
		job->finished_notification = NULL;
	}

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)
	if (VR_TRUE == deferred) {
		/* The deletion of the job object (releasing sync refs etc) must be done in a different context */
		vr_pp_scheduler_deferred_job_delete(job);
	} else {
		vr_pp_job_delete(job);
	}
#else
	VR_DEBUG_ASSERT(VR_FALSE == deferred); /* no use cases need this in this configuration */
	vr_pp_job_delete(job);
#endif
}

static void vr_pp_scheduler_finalize_job(struct vr_pp_job * job)
{
	/* This job object should not be on any lists. */
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->list));
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->session_list));
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->session_fb_lookup_list));

	/* Send notification back to user space */
#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)
	vr_pp_scheduler_return_job_to_user(job, VR_TRUE);
#else
	vr_pp_scheduler_return_job_to_user(job, VR_FALSE);
#endif

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	if (_VR_PP_JOB_FLAG_IS_WINDOW_SURFACE & job->uargs.flags) {
		_vr_osk_atomic_inc(&job->session->number_of_window_jobs);
	}
#endif

	vr_pp_scheduler_job_completed();
}

void vr_pp_scheduler_schedule(void)
{
	struct vr_group* physical_groups_to_start[VR_MAX_NUMBER_OF_PP_GROUPS - 1];
	struct vr_pp_job* physical_jobs_to_start[VR_MAX_NUMBER_OF_PP_GROUPS - 1];
	u32 physical_sub_jobs_to_start[VR_MAX_NUMBER_OF_PP_GROUPS - 1];
	int num_physical_jobs_to_start = 0;
	int i;

	if (vr_pp_scheduler_has_virtual_group()) {
		/* Lock the virtual group since we might have to grab physical groups. */
		vr_group_lock(virtual_group);
	}

	vr_pp_scheduler_lock();
	if (pause_count > 0) {
		/* Scheduler is suspended, don't schedule any jobs. */
		vr_pp_scheduler_unlock();
		if (vr_pp_scheduler_has_virtual_group()) {
			vr_group_unlock(virtual_group);
		}
		return;
	}

	/* Find physical job(s) to schedule first. */
	while (1) {
		struct vr_group *group;
		struct vr_pp_job *job;
		u32 sub_job;

		job = vr_pp_scheduler_get_physical_job();
		if (NULL == job) {
			break; /* No job, early out. */
		}

		if (vr_scheduler_hint_is_enabled(VR_SCHEDULER_HINT_GP_BOUND) &&
		    vr_pp_job_is_large_and_unstarted(job) && !_vr_osk_list_empty(&group_list_working)) {
			/* Since not all groups are idle, don't schedule yet. */
			break;
		}

		VR_DEBUG_ASSERT(!vr_pp_job_is_virtual(job));
		VR_DEBUG_ASSERT(vr_pp_job_has_unstarted_sub_jobs(job));
		VR_DEBUG_ASSERT(1 <= vr_pp_job_get_sub_job_count(job));

		/* Acquire a physical group, either from the idle list or from the virtual group.
		 * In case the group was acquired from the virtual group, it's state will be
		 * LEAVING_VIRTUAL and must be set to IDLE before it can be used. */
		group = vr_pp_scheduler_acquire_physical_group();
		if (NULL == group) {
			/* Could not get a group to run the job on, early out. */
			VR_DEBUG_PRINT(4, ("Vr PP scheduler: No more physical groups available.\n"));
			break;
		}

		VR_DEBUG_PRINT(4, ("Vr PP scheduler: Acquired physical group %p.\n", group));

		/* Mark sub job as started. */
		sub_job = vr_pp_job_get_first_unstarted_sub_job(job);
		vr_pp_job_mark_sub_job_started(job, sub_job);

		/* Remove job from queue (if this was the last sub job). */
		vr_pp_scheduler_dequeue_physical_job(job);

		/* Move group to working list. */
		_vr_osk_list_move(&(group->pp_scheduler_list), &group_list_working);

		/* Keep track of this group, so that we actually can start the job once we are done with the scheduler lock we are now holding. */
		physical_groups_to_start[num_physical_jobs_to_start] = group;
		physical_jobs_to_start[num_physical_jobs_to_start] = job;
		physical_sub_jobs_to_start[num_physical_jobs_to_start] = sub_job;
		++num_physical_jobs_to_start;

		VR_DEBUG_ASSERT(num_physical_jobs_to_start < VR_MAX_NUMBER_OF_PP_GROUPS);
	}

	if (vr_pp_scheduler_has_virtual_group()) {
		if (VIRTUAL_GROUP_IDLE == virtual_group_state) {
			/* We have a virtual group and it is idle. */

			struct vr_pp_job *job;

			/* Find a virtual job we can start. */
			job = vr_pp_scheduler_get_virtual_job();

			if (NULL != job) {
				VR_DEBUG_ASSERT(vr_pp_job_is_virtual(job));
				VR_DEBUG_ASSERT(vr_pp_job_has_unstarted_sub_jobs(job));
				VR_DEBUG_ASSERT(1 == vr_pp_job_get_sub_job_count(job));

				/* Mark the one and only sub job as started. */
				vr_pp_job_mark_sub_job_started(job, 0);

				/* Remove job from queue. */
				vr_pp_scheduler_dequeue_virtual_job(job);

				/* Virtual group is now working. */
				virtual_group_state = VIRTUAL_GROUP_WORKING;

				/* We no longer need the scheduler lock, but we still need the virtual lock
				 * in order to start the virtual job. */
				vr_pp_scheduler_unlock();

				/* Start job. */
				vr_group_start_pp_job(virtual_group, job, 0);

				VR_DEBUG_PRINT(4, ("Vr PP scheduler: Virtual job %u (0x%08X) part %u/%u started (from schedule).\n",
				                     vr_pp_job_get_id(job), job, 1,
				                     vr_pp_job_get_sub_job_count(job)));

				vr_group_unlock(virtual_group);
			} else {
				/* No virtual job to start. */
				vr_pp_scheduler_unlock();
				vr_group_unlock(virtual_group);
			}
		} else {
			/* We have a virtual group, but it is busy or disabled. */
			VR_DEBUG_ASSERT(VIRTUAL_GROUP_IDLE != virtual_group_state);

			vr_pp_scheduler_unlock();
			vr_group_unlock(virtual_group);
		}
	} else {
		/* There is no virtual group. */
		vr_pp_scheduler_unlock();
	}

	/* We have now released the scheduler lock, and we are ready to start the physical jobs.
	 * The reason we want to wait until we have released the scheduler lock is that job start
	 * may take quite a bit of time (many registers have to be written). This will allow new
	 * jobs from user space to come in, and post-processing of other PP jobs to happen at the
	 * same time as we start jobs. */
	for (i = 0; i < num_physical_jobs_to_start; i++) {
		struct vr_group *group = physical_groups_to_start[i];
		struct vr_pp_job *job  = physical_jobs_to_start[i];
		u32 sub_job              = physical_sub_jobs_to_start[i];

		VR_DEBUG_ASSERT_POINTER(group);
		VR_DEBUG_ASSERT_POINTER(job);
		VR_DEBUG_ASSERT(!vr_group_is_virtual(group));
		VR_DEBUG_ASSERT(!vr_pp_job_is_virtual(job));

		vr_group_lock(group);

		/* Set state to IDLE if group was acquired from the virtual group. */
		group->state = VR_GROUP_STATE_IDLE;

		vr_group_start_pp_job(group, job, sub_job);

		VR_DEBUG_PRINT(4, ("Vr PP scheduler: Physical job %u (0x%08X) part %u/%u started (from schedule).\n",
		                     vr_pp_job_get_id(job), job, sub_job + 1,
		                     vr_pp_job_get_sub_job_count(job)));

		vr_group_unlock(group);
	}
}

/**
 * Set group idle.
 *
 * If @ref group is the virtual group, nothing is done since the virtual group should be idle
 * already.
 *
 * If @ref group is a physical group we rejoin the virtual group, if it exists.  If not, we move the
 * physical group to the idle list.
 *
 * @note The group and the scheduler must both be locked when entering this function.  Both will be
 * unlocked before exiting.
 *
 * @param group The group to set idle.
 */
static void vr_pp_scheduler_set_group_idle_and_unlock(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);

	VR_ASSERT_GROUP_LOCKED(group);
	VR_DEBUG_ASSERT_LOCK_HELD(pp_scheduler_lock);

	if (vr_group_is_virtual(group)) {
		/* The virtual group should have been set to non-working already. */
		VR_DEBUG_ASSERT(VIRTUAL_GROUP_IDLE == virtual_group_state);

		vr_pp_scheduler_unlock();
		vr_group_unlock(group);

		return;
	} else {
		if (vr_pp_scheduler_has_virtual_group()) {
			/* Rejoin virtual group. */

			/* We're no longer needed on the scheduler list. */
			_vr_osk_list_delinit(&(group->pp_scheduler_list));

			/* Make sure no interrupts are handled for this group during the transition
			 * from physical to virtual. */
			group->state = VR_GROUP_STATE_JOINING_VIRTUAL;

			vr_pp_scheduler_unlock();
			vr_group_unlock(group);

			vr_group_lock(virtual_group);

			if (vr_pp_scheduler_has_virtual_group()) {
				vr_pp_scheduler_enable_empty_virtual();
			}

			/* We need to recheck the group state since it is possible that someone has
			 * modified the group before we locked the virtual group. */
			if (VR_GROUP_STATE_JOINING_VIRTUAL == group->state) {
				vr_group_add_group(virtual_group, group, VR_TRUE);
			}

			vr_group_unlock(virtual_group);
		} else {
			/* Move physical group back to idle list. */
			_vr_osk_list_move(&(group->pp_scheduler_list), &group_list_idle);

#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
			trace_gpu_sched_switch(vr_pp_get_hw_core_desc(group->pp_core), sched_clock(), 0, 0, 0);
#endif

			vr_pp_scheduler_unlock();
			vr_group_unlock(group);
		}
	}
}

/**
 * Schedule job on locked group.
 *
 * @note The group and the scheduler must both be locked when entering this function.  Both will be
 * unlocked before exiting.
 *
 * @param group The group to schedule on.
 */
static void vr_pp_scheduler_schedule_on_group_and_unlock(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);

	VR_ASSERT_GROUP_LOCKED(group);
	VR_DEBUG_ASSERT_LOCK_HELD(pp_scheduler_lock);

	if (vr_group_is_virtual(group)) {
		/* Now that the virtual group is idle, check if we should reconfigure. */

		struct vr_pp_job *virtual_job = NULL;
		struct vr_pp_job *physical_job = NULL;
		struct vr_group *physical_group = NULL;
		u32 physical_sub_job = 0;

		VR_DEBUG_ASSERT(VIRTUAL_GROUP_IDLE == virtual_group_state);

		if (vr_pp_scheduler_can_move_virtual_to_physical()) {
			/* There is a runnable physical job and we can acquire a physical group. */
			physical_job = vr_pp_scheduler_get_physical_job();
			VR_DEBUG_ASSERT_POINTER(physical_job);
			VR_DEBUG_ASSERT(vr_pp_job_has_unstarted_sub_jobs(physical_job));

			/* Mark sub job as started. */
			physical_sub_job = vr_pp_job_get_first_unstarted_sub_job(physical_job);
			vr_pp_job_mark_sub_job_started(physical_job, physical_sub_job);

			/* Remove job from queue (if this was the last sub job). */
			vr_pp_scheduler_dequeue_physical_job(physical_job);

			/* Acquire a physical group from the virtual group.  Its state will
			 * be LEAVING_VIRTUAL and must be set to IDLE before it can be
			 * used. */
			physical_group = vr_group_acquire_group(virtual_group);

			/* Move physical group to the working list, as we will soon start a job on it. */
			_vr_osk_list_move(&(physical_group->pp_scheduler_list), &group_list_working);

			vr_pp_scheduler_disable_empty_virtual();
		}

		/* Get next virtual job. */
		virtual_job = vr_pp_scheduler_get_virtual_job();
		if (NULL != virtual_job && VIRTUAL_GROUP_IDLE == virtual_group_state) {
			/* There is a runnable virtual job. */

			VR_DEBUG_ASSERT(vr_pp_job_is_virtual(virtual_job));
			VR_DEBUG_ASSERT(vr_pp_job_has_unstarted_sub_jobs(virtual_job));
			VR_DEBUG_ASSERT(1 == vr_pp_job_get_sub_job_count(virtual_job));

			vr_pp_job_mark_sub_job_started(virtual_job, 0);

			/* Remove job from queue. */
			vr_pp_scheduler_dequeue_virtual_job(virtual_job);

			/* Virtual group is now working. */
			virtual_group_state = VIRTUAL_GROUP_WORKING;

			vr_pp_scheduler_unlock();

			/* Start job. */
			vr_group_start_pp_job(group, virtual_job, 0);

			VR_DEBUG_PRINT(4, ("Vr PP scheduler: Virtual job %u (0x%08X) part %u/%u started (from job_done).\n",
			                     vr_pp_job_get_id(virtual_job), virtual_job, 1,
			                     vr_pp_job_get_sub_job_count(virtual_job)));
		} else {
#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
			trace_gpu_sched_switch("Vr_Virtual_PP", sched_clock(), 0, 0, 0);
#endif

			vr_pp_scheduler_unlock();
		}

		/* Releasing the virtual group lock that was held when entering the function. */
		vr_group_unlock(group);

		/* Start a physical job (if we acquired a physical group earlier). */
		if (NULL != physical_job && NULL != physical_group) {
			vr_group_lock(physical_group);

			/* Change the group state from LEAVING_VIRTUAL to IDLE to complete the transition. */
			physical_group->state = VR_GROUP_STATE_IDLE;

			/* Start job. */
			vr_group_start_pp_job(physical_group, physical_job, physical_sub_job);

			VR_DEBUG_PRINT(4, ("Vr PP scheduler: Physical job %u (0x%08X) part %u/%u started (from job_done).\n",
			                     vr_pp_job_get_id(physical_job), physical_job, physical_sub_job + 1,
			                     vr_pp_job_get_sub_job_count(physical_job)));

			vr_group_unlock(physical_group);
		}
	} else {
		/* Physical group. */
		struct vr_pp_job *job = NULL;
		u32 sub_job = 0;

		job = vr_pp_scheduler_get_physical_job();
		if (NULL != job) {
			/* There is a runnable physical job. */
			VR_DEBUG_ASSERT(vr_pp_job_has_unstarted_sub_jobs(job));

			/* Mark sub job as started. */
			sub_job = vr_pp_job_get_first_unstarted_sub_job(job);
			vr_pp_job_mark_sub_job_started(job, sub_job);

			/* Remove job from queue (if this was the last sub job). */
			vr_pp_scheduler_dequeue_physical_job(job);

			vr_pp_scheduler_unlock();

			/* Group is already on the working list, so start the new job. */
			vr_group_start_pp_job(group, job, sub_job);

			VR_DEBUG_PRINT(4, ("Vr PP scheduler: Physical job %u (0x%08X) part %u/%u started (from job_done).\n",
			                     vr_pp_job_get_id(job), job, sub_job + 1, vr_pp_job_get_sub_job_count(job)));

			vr_group_unlock(group);
		} else {
			vr_pp_scheduler_set_group_idle_and_unlock(group);
		}
	}
}

void vr_pp_scheduler_job_done(struct vr_group *group, struct vr_pp_job *job, u32 sub_job, vr_bool success, vr_bool in_upper_half)
{
	vr_bool job_is_done = VR_FALSE;
	vr_bool schedule_on_group = VR_FALSE;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_PRINT(3, ("Vr PP scheduler: %s job %u (0x%08X) part %u/%u completed (%s).\n",
	                     vr_pp_job_is_virtual(job) ? "Virtual" : "Physical",
	                     vr_pp_job_get_id(job),
	                     job, sub_job + 1,
	                     vr_pp_job_get_sub_job_count(job),
	                     success ? "success" : "failure"));

	VR_ASSERT_GROUP_LOCKED(group);
	vr_pp_scheduler_lock();

	vr_pp_job_mark_sub_job_completed(job, success);

	VR_DEBUG_ASSERT(vr_pp_job_is_virtual(job) == vr_group_is_virtual(group));

	job_is_done = vr_pp_job_is_complete(job);

	if (job_is_done) {
		/* Job is removed from these lists when the last sub job is scheduled. */
		VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->list));
		VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->session_fb_lookup_list));

		/* Remove job from session list. */
		_vr_osk_list_delinit(&job->session_list);

		VR_DEBUG_PRINT(4, ("Vr PP scheduler: All parts completed for %s job %u (0x%08X).\n",
		                     vr_pp_job_is_virtual(job) ? "virtual" : "physical",
		                     vr_pp_job_get_id(job), job));

		vr_pp_scheduler_unlock();

		/* Release tracker.  If other trackers are waiting on this tracker, this could
		 * trigger activation.  The returned scheduling mask can be used to determine if we
		 * have to schedule GP, PP or both. */
		schedule_mask = vr_timeline_tracker_release(&job->tracker);

		vr_pp_scheduler_lock();
	}

	if (vr_group_is_virtual(group)) {
		/* Obey the policy. */
		virtual_group_state = VIRTUAL_GROUP_IDLE;
	}

	/* If paused, then this was the last job, so wake up sleeping workers and return. */
	if (pause_count > 0) {
		/* Wake up sleeping workers. Their wake-up condition is that
		 * num_slots == num_slots_idle, so unless we are done working, no
		 * threads will actually be woken up.
		 */
		if (!vr_group_is_virtual(group)) {
			/* Move physical group to idle list. */
			_vr_osk_list_move(&(group->pp_scheduler_list), &group_list_idle);
		}

#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
		trace_gpu_sched_switch(vr_pp_get_hw_core_desc(group->pp_core), sched_clock(), 0, 0, 0);
#endif

		_vr_osk_wait_queue_wake_up(pp_scheduler_working_wait_queue);

		vr_pp_scheduler_unlock();
		vr_group_unlock(group);

		if (job_is_done) {
			/* Return job to user and delete it. */
			vr_pp_scheduler_finalize_job(job);
		}

		/* A GP job might be queued by tracker release above,
		 * make sure GP scheduler gets a chance to schedule this (if possible)
		 */
		vr_scheduler_schedule_from_mask(schedule_mask & ~VR_SCHEDULER_MASK_PP, in_upper_half);

		return;
	}

	/* Since this group just finished running a job, we can reschedule a new job on it
	 * immediately. */

	/* By default, don't schedule on group. */
	schedule_on_group = VR_FALSE;

	if (vr_group_is_virtual(group)) {
		/* Always schedule immediately on virtual group. */
		schedule_mask &= ~VR_SCHEDULER_MASK_PP;
		schedule_on_group = VR_TRUE;
	} else if (0 < job_queue.depth && (!vr_scheduler_mask_is_set(schedule_mask, VR_SCHEDULER_MASK_PP) || _vr_osk_list_empty(&group_list_idle))) {
		struct vr_pp_job *next_job = NULL;

		next_job = vr_pp_scheduler_get_physical_job();
		VR_DEBUG_ASSERT_POINTER(next_job);

		/* If no new jobs have been queued or if this group is the only idle group, we can
		 * schedule immediately on this group, unless we are GP bound and the next job would
		 * benefit from all its sub jobs being started concurrently. */

		if (vr_scheduler_hint_is_enabled(VR_SCHEDULER_HINT_GP_BOUND) && vr_pp_job_is_large_and_unstarted(next_job)) {
			/* We are GP bound and the job would benefit from all sub jobs being started
			 * concurrently.  Postpone scheduling until after group has been unlocked. */
			schedule_mask |= VR_SCHEDULER_MASK_PP;
			schedule_on_group = VR_FALSE;
		} else {
			/* Schedule job immediately since we are not GP bound. */
			schedule_mask &= ~VR_SCHEDULER_MASK_PP;
			schedule_on_group = VR_TRUE;
		}
	}

	if (schedule_on_group) {
		/* Schedule a new job on this group. */
		vr_pp_scheduler_schedule_on_group_and_unlock(group);
	} else {
		/* Set group idle.  Will rejoin virtual group, under appropriate conditions. */
		vr_pp_scheduler_set_group_idle_and_unlock(group);
	}

	if (!schedule_on_group || VR_SCHEDULER_MASK_EMPTY != schedule_mask) {
		if (VR_SCHEDULER_MASK_PP & schedule_mask) {
			/* Schedule PP directly. */
			vr_pp_scheduler_schedule();
			schedule_mask &= ~VR_SCHEDULER_MASK_PP;
		}

		/* Schedule other jobs that were activated. */
		vr_scheduler_schedule_from_mask(schedule_mask, in_upper_half);
	}

	if (job_is_done) {
		/* Return job to user and delete it. */
		vr_pp_scheduler_finalize_job(job);
	}
}

void vr_pp_scheduler_suspend(void)
{
	vr_pp_scheduler_lock();
	pause_count++; /* Increment the pause_count so that no more jobs will be scheduled */
	vr_pp_scheduler_unlock();

	/* Go to sleep. When woken up again (in vr_pp_scheduler_job_done), the
	 * vr_pp_scheduler_suspended() function will be called. This will return true
	 * if state is idle and pause_count > 0, so if the core is active this
	 * will not do anything.
	 */
	_vr_osk_wait_queue_wait_event(pp_scheduler_working_wait_queue, vr_pp_scheduler_is_suspended, NULL);
}

void vr_pp_scheduler_resume(void)
{
	vr_pp_scheduler_lock();
	pause_count--; /* Decrement pause_count to allow scheduling again (if it reaches 0) */
	vr_pp_scheduler_unlock();
	if (0 == pause_count) {
		vr_pp_scheduler_schedule();
	}
}

vr_timeline_point vr_pp_scheduler_submit_job(struct vr_session_data *session, struct vr_pp_job *job)
{
	vr_timeline_point point;
	u32 fb_lookup_id = 0;

	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_ASSERT_POINTER(job);

	vr_pp_scheduler_lock();

	fb_lookup_id = vr_pp_job_get_fb_lookup_id(job);
	VR_DEBUG_ASSERT(VR_PP_JOB_FB_LOOKUP_LIST_SIZE > fb_lookup_id);

	/* Adding job to the lookup list used to quickly discard writeback units of queued jobs. */
	_vr_osk_list_addtail(&job->session_fb_lookup_list, &session->pp_job_fb_lookup_list[fb_lookup_id]);

	vr_pp_scheduler_unlock();

	vr_pp_scheduler_job_queued();

	/* Add job to Timeline system. */
	point = vr_timeline_system_add_tracker(session->timeline_system, &job->tracker, VR_TIMELINE_PP);

	return point;
}

_vr_osk_errcode_t _vr_ukk_pp_start_job(void *ctx, _vr_uk_pp_start_job_s *uargs)
{
	struct vr_session_data *session;
	struct vr_pp_job *job;
	vr_timeline_point point;
	u32 __user *timeline_point_ptr = NULL;

	VR_DEBUG_ASSERT_POINTER(uargs);
	VR_DEBUG_ASSERT_POINTER(ctx);

	session = (struct vr_session_data*)ctx;

	job = vr_pp_job_create(session, uargs, vr_scheduler_get_new_id());
	if (NULL == job) {
		VR_PRINT_ERROR(("Failed to create PP job.\n"));
		return _VR_OSK_ERR_NOMEM;
	}

	timeline_point_ptr = (u32 __user *) job->uargs.timeline_point_ptr;

	point = vr_pp_scheduler_submit_job(session, job);
	job = NULL;

	if (0 != _vr_osk_put_user(((u32) point), timeline_point_ptr)) {
		/* Let user space know that something failed after the job was started. */
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t _vr_ukk_pp_and_gp_start_job(void *ctx, _vr_uk_pp_and_gp_start_job_s *uargs)
{
	struct vr_session_data *session;
	_vr_uk_pp_and_gp_start_job_s kargs;
	struct vr_pp_job *pp_job;
	struct vr_gp_job *gp_job;
	u32 __user *timeline_point_ptr = NULL;
	vr_timeline_point point;

	VR_DEBUG_ASSERT_POINTER(ctx);
	VR_DEBUG_ASSERT_POINTER(uargs);

	session = (struct vr_session_data *) ctx;

	if (0 != _vr_osk_copy_from_user(&kargs, uargs, sizeof(_vr_uk_pp_and_gp_start_job_s))) {
		return _VR_OSK_ERR_NOMEM;
	}

	pp_job = vr_pp_job_create(session, kargs.pp_args, vr_scheduler_get_new_id());
	if (NULL == pp_job) {
		VR_PRINT_ERROR(("Failed to create PP job.\n"));
		return _VR_OSK_ERR_NOMEM;
	}

	gp_job = vr_gp_job_create(session, kargs.gp_args, vr_scheduler_get_new_id(), vr_pp_job_get_tracker(pp_job));
	if (NULL == gp_job) {
		VR_PRINT_ERROR(("Failed to create GP job.\n"));
		vr_pp_job_delete(pp_job);
		return _VR_OSK_ERR_NOMEM;
	}

	timeline_point_ptr = (u32 __user *) pp_job->uargs.timeline_point_ptr;

	/* Submit GP job. */
	vr_gp_scheduler_submit_job(session, gp_job);
	gp_job = NULL;

	/* Submit PP job. */
	point = vr_pp_scheduler_submit_job(session, pp_job);
	pp_job = NULL;

	if (0 != _vr_osk_put_user(((u32) point), timeline_point_ptr)) {
		/* Let user space know that something failed after the jobs were started. */
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t _vr_ukk_get_pp_number_of_cores(_vr_uk_get_pp_number_of_cores_s *args)
{
	VR_DEBUG_ASSERT_POINTER(args);
	VR_DEBUG_ASSERT_POINTER(args->ctx);
	args->number_of_total_cores = num_cores;
	args->number_of_enabled_cores = enabled_cores;
	return _VR_OSK_ERR_OK;
}

u32 vr_pp_scheduler_get_num_cores_total(void)
{
	return num_cores;
}

u32 vr_pp_scheduler_get_num_cores_enabled(void)
{
	return enabled_cores;
}

_vr_osk_errcode_t _vr_ukk_get_pp_core_version(_vr_uk_get_pp_core_version_s *args)
{
	VR_DEBUG_ASSERT_POINTER(args);
	VR_DEBUG_ASSERT_POINTER(args->ctx);
	args->version = pp_version;
	return _VR_OSK_ERR_OK;
}

void _vr_ukk_pp_job_disable_wb(_vr_uk_pp_disable_wb_s *args)
{
	struct vr_session_data *session;
	struct vr_pp_job *job;
	struct vr_pp_job *tmp;
	u32 fb_lookup_id;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_DEBUG_ASSERT_POINTER(args->ctx);

	session = (struct vr_session_data*)args->ctx;

	fb_lookup_id = args->fb_id & VR_PP_JOB_FB_LOOKUP_LIST_MASK;

	vr_pp_scheduler_lock();

	/* Iterate over all jobs for given frame builder_id. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp, &session->pp_job_fb_lookup_list[fb_lookup_id], struct vr_pp_job, session_fb_lookup_list) {
		VR_DEBUG_CODE(u32 disable_mask = 0);

		if (vr_pp_job_get_frame_builder_id(job) == (u32) args->fb_id) {
			VR_DEBUG_CODE(disable_mask |= 0xD<<(4*3));
			if (args->wb0_memory == job->uargs.wb0_registers[VR200_REG_ADDR_WB_SOURCE_ADDR/sizeof(u32)]) {
				VR_DEBUG_CODE(disable_mask |= 0x1<<(4*1));
				vr_pp_job_disable_wb0(job);
			}
			if (args->wb1_memory == job->uargs.wb1_registers[VR200_REG_ADDR_WB_SOURCE_ADDR/sizeof(u32)]) {
				VR_DEBUG_CODE(disable_mask |= 0x2<<(4*2));
				vr_pp_job_disable_wb1(job);
			}
			if (args->wb2_memory == job->uargs.wb2_registers[VR200_REG_ADDR_WB_SOURCE_ADDR/sizeof(u32)]) {
				VR_DEBUG_CODE(disable_mask |= 0x3<<(4*3));
				vr_pp_job_disable_wb2(job);
			}
			VR_DEBUG_PRINT(3, ("Vr PP scheduler: Disable WB: 0x%X.\n", disable_mask));
		} else {
			VR_DEBUG_PRINT(4, ("Vr PP scheduler: Disable WB mismatching FB.\n"));
		}
	}

	vr_pp_scheduler_unlock();
}

void vr_pp_scheduler_abort_session(struct vr_session_data *session)
{
	u32 i = 0;
	struct vr_pp_job *job, *tmp_job;
	struct vr_group *group, *tmp_group;
	struct vr_group *groups[VR_MAX_NUMBER_OF_GROUPS];
	_VR_OSK_LIST_HEAD_STATIC_INIT(removed_jobs);

	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_ASSERT(session->is_aborting);

	VR_DEBUG_PRINT(3, ("Vr PP scheduler: Aborting all jobs from session 0x%08X.\n", session));

	vr_pp_scheduler_lock();

	/* Find all jobs from the aborting session. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp_job, &session->pp_job_list, struct vr_pp_job, session_list) {
		/* Remove job from queue. */
		if (vr_pp_job_is_virtual(job)) {
			VR_DEBUG_ASSERT(1 == vr_pp_job_get_sub_job_count(job));
			if (0 == vr_pp_job_get_first_unstarted_sub_job(job)) {
				--virtual_job_queue.depth;
			}
		} else {
			job_queue.depth -= vr_pp_job_get_sub_job_count(job) - vr_pp_job_get_first_unstarted_sub_job(job);
		}

		_vr_osk_list_delinit(&job->list);
		_vr_osk_list_delinit(&job->session_fb_lookup_list);

		vr_pp_job_mark_unstarted_failed(job);

		if (vr_pp_job_is_complete(job)) {
			/* Job is complete, remove from session list. */
			_vr_osk_list_delinit(&job->session_list);

			/* Move job to local list for release and deletion. */
			_vr_osk_list_add(&job->list, &removed_jobs);

			VR_DEBUG_PRINT(3, ("Vr PP scheduler: Aborted PP job %u (0x%08X).\n", vr_pp_job_get_id(job), job));
		} else {
			VR_DEBUG_PRINT(3, ("Vr PP scheduler: Keeping partially started PP job %u (0x%08X) in session.\n", vr_pp_job_get_id(job), job));
		}
	}

	_VR_OSK_LIST_FOREACHENTRY(group, tmp_group, &group_list_working, struct vr_group, pp_scheduler_list) {
		groups[i++] = group;
	}

	_VR_OSK_LIST_FOREACHENTRY(group, tmp_group, &group_list_idle, struct vr_group, pp_scheduler_list) {
		groups[i++] = group;
	}

	vr_pp_scheduler_unlock();

	/* Release and delete all found jobs from the aborting session. */
	_VR_OSK_LIST_FOREACHENTRY(job, tmp_job, &removed_jobs, struct vr_pp_job, list) {
		vr_timeline_tracker_release(&job->tracker);
		vr_pp_job_delete(job);
		vr_pp_scheduler_job_completed();
	}

	/* Abort any running jobs from the session. */
	while (i > 0) {
		vr_group_abort_session(groups[--i], session);
	}

	if (vr_pp_scheduler_has_virtual_group()) {
		vr_group_abort_session(virtual_group, session);
	}
}

static vr_bool vr_pp_scheduler_is_suspended(void *data)
{
	vr_bool ret;

	/* This callback does not use the data pointer. */
	VR_IGNORE(data);

	vr_pp_scheduler_lock();

	ret = pause_count > 0
	      && _vr_osk_list_empty(&group_list_working)
	      && VIRTUAL_GROUP_WORKING != virtual_group_state;

	vr_pp_scheduler_unlock();

	return ret;
}

struct vr_pp_core *vr_pp_scheduler_get_virtual_pp(void)
{
	if (vr_pp_scheduler_has_virtual_group()) {
		return vr_group_get_pp_core(virtual_group);
	} else {
		return NULL;
	}
}

#if VR_STATE_TRACKING
u32 vr_pp_scheduler_dump_state(char *buf, u32 size)
{
	int n = 0;
	struct vr_group *group;
	struct vr_group *temp;

	n += _vr_osk_snprintf(buf + n, size - n, "PP:\n");
	n += _vr_osk_snprintf(buf + n, size - n, "\tQueue is %s\n", _vr_osk_list_empty(&job_queue.normal_pri) ? "empty" : "not empty");
	n += _vr_osk_snprintf(buf + n, size - n, "\tHigh priority queue is %s\n", _vr_osk_list_empty(&job_queue.high_pri) ? "empty" : "not empty");
	n += _vr_osk_snprintf(buf + n, size - n, "\n");

	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_working, struct vr_group, pp_scheduler_list) {
		n += vr_group_dump_state(group, buf + n, size - n);
	}

	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_idle, struct vr_group, pp_scheduler_list) {
		n += vr_group_dump_state(group, buf + n, size - n);
	}

	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_disabled, struct vr_group, pp_scheduler_list) {
		n += vr_group_dump_state(group, buf + n, size - n);
	}

	if (vr_pp_scheduler_has_virtual_group()) {
		n += vr_group_dump_state(virtual_group, buf + n, size -n);
	}

	n += _vr_osk_snprintf(buf + n, size - n, "\n");
	return n;
}
#endif

/* This function is intended for power on reset of all cores.
 * No locking is done for the list iteration, which can only be safe if the
 * scheduler is paused and all cores idle. That is always the case on init and
 * power on. */
void vr_pp_scheduler_reset_all_groups(void)
{
	struct vr_group *group, *temp;
	struct vr_group *groups[VR_MAX_NUMBER_OF_GROUPS];
	s32 i = 0;

	if (vr_pp_scheduler_has_virtual_group()) {
		vr_group_lock(virtual_group);
		vr_group_reset(virtual_group);
		vr_group_unlock(virtual_group);
	}

	VR_DEBUG_ASSERT(_vr_osk_list_empty(&group_list_working));
	VR_DEBUG_ASSERT(VIRTUAL_GROUP_WORKING != virtual_group_state);
	vr_pp_scheduler_lock();
	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_idle, struct vr_group, pp_scheduler_list) {
		groups[i++] = group;
	}
	vr_pp_scheduler_unlock();

	while (i > 0) {
		group = groups[--i];

		vr_group_lock(group);
		vr_group_reset(group);
		vr_group_unlock(group);
	}
}

void vr_pp_scheduler_zap_all_active(struct vr_session_data *session)
{
	struct vr_group *group, *temp;
	struct vr_group *groups[VR_MAX_NUMBER_OF_GROUPS];
	s32 i = 0;

	if (vr_pp_scheduler_has_virtual_group()) {
		vr_group_zap_session(virtual_group, session);
	}

	vr_pp_scheduler_lock();
	_VR_OSK_LIST_FOREACHENTRY(group, temp, &group_list_working, struct vr_group, pp_scheduler_list) {
		groups[i++] = group;
	}
	vr_pp_scheduler_unlock();

	while (i > 0) {
		vr_group_zap_session(groups[--i], session);
	}
}

/* A pm reference must be taken with _vr_osk_pm_dev_ref_add_no_power_on
 * before calling this function to avoid Vr powering down as HW is accessed.
 */
static void vr_pp_scheduler_enable_group_internal(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);

	vr_group_lock(group);

	if (VR_GROUP_STATE_DISABLED != group->state) {
		vr_group_unlock(group);
		VR_DEBUG_PRINT(4, ("Vr PP scheduler: PP group %p already enabled.\n", group));
		return;
	}

	VR_DEBUG_PRINT(3, ("Vr PP scheduler: Enabling PP group %p.\n", group));

	vr_pp_scheduler_lock();

	VR_DEBUG_ASSERT(VR_GROUP_STATE_DISABLED == group->state);
	++enabled_cores;

	if (vr_pp_scheduler_has_virtual_group()) {
		vr_bool update_hw;

		/* Add group to virtual group. */
		_vr_osk_list_delinit(&(group->pp_scheduler_list));
		group->state = VR_GROUP_STATE_JOINING_VIRTUAL;

		vr_pp_scheduler_unlock();
		vr_group_unlock(group);

		vr_group_lock(virtual_group);

		update_hw = vr_pm_is_power_on();
		/* Get ref of group domain */
		vr_group_get_pm_domain_ref(group);

		VR_DEBUG_ASSERT(NULL == group->pm_domain ||
		                  VR_PM_DOMAIN_ON == vr_pm_domain_state_get(group->pm_domain));

		if (update_hw) {
			vr_group_lock(group);
			vr_group_power_on_group(group);
			vr_group_reset(group);
			vr_group_unlock(group);
		}

		vr_pp_scheduler_enable_empty_virtual();
		vr_group_add_group(virtual_group, group, update_hw);
		VR_DEBUG_PRINT(4, ("Vr PP scheduler: Done enabling group %p. Added to virtual group.\n", group));

		vr_group_unlock(virtual_group);
	} else {
		/* Get ref of group domain */
		vr_group_get_pm_domain_ref(group);

		VR_DEBUG_ASSERT(NULL == group->pm_domain ||
		                  VR_PM_DOMAIN_ON == vr_pm_domain_state_get(group->pm_domain));

		/* Put group on idle list. */
		if (vr_pm_is_power_on()) {
			vr_group_power_on_group(group);
			vr_group_reset(group);
		}

		_vr_osk_list_move(&(group->pp_scheduler_list), &group_list_idle);
		group->state = VR_GROUP_STATE_IDLE;

		VR_DEBUG_PRINT(4, ("Vr PP scheduler: Done enabling group %p. Now on idle list.\n", group));
		vr_pp_scheduler_unlock();
		vr_group_unlock(group);
	}
}

void vr_pp_scheduler_enable_group(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);

	_vr_osk_pm_dev_ref_add_no_power_on();

	vr_pp_scheduler_enable_group_internal(group);

	_vr_osk_pm_dev_ref_dec_no_power_on();

	/* Pick up any jobs that might have been queued if all PP groups were disabled. */
	vr_pp_scheduler_schedule();
}

static void vr_pp_scheduler_disable_group_internal(struct vr_group *group)
{
	if (vr_pp_scheduler_has_virtual_group()) {
		vr_group_lock(virtual_group);

		VR_DEBUG_ASSERT(VIRTUAL_GROUP_WORKING != virtual_group_state);
		if (VR_GROUP_STATE_JOINING_VIRTUAL == group->state) {
			/* The group was in the process of being added to the virtual group.  We
			 * only need to change the state to reverse this. */
			group->state = VR_GROUP_STATE_LEAVING_VIRTUAL;
		} else if (VR_GROUP_STATE_IN_VIRTUAL == group->state) {
			/* Remove group from virtual group.  The state of the group will be
			 * LEAVING_VIRTUAL and the group will not be on any scheduler list. */
			vr_group_remove_group(virtual_group, group);

			vr_pp_scheduler_disable_empty_virtual();
		}

		vr_group_unlock(virtual_group);
	}

	vr_group_lock(group);
	vr_pp_scheduler_lock();

	VR_DEBUG_ASSERT(   VR_GROUP_STATE_IDLE            == group->state
	                     || VR_GROUP_STATE_LEAVING_VIRTUAL == group->state
	                     || VR_GROUP_STATE_DISABLED        == group->state);

	if (VR_GROUP_STATE_DISABLED == group->state) {
		VR_DEBUG_PRINT(4, ("Vr PP scheduler: PP group %p already disabled.\n", group));
	} else {
		VR_DEBUG_PRINT(3, ("Vr PP scheduler: Disabling PP group %p.\n", group));

		--enabled_cores;
		_vr_osk_list_move(&(group->pp_scheduler_list), &group_list_disabled);
		group->state = VR_GROUP_STATE_DISABLED;

		vr_group_power_off_group(group, VR_TRUE);
		vr_group_put_pm_domain_ref(group);
	}

	vr_pp_scheduler_unlock();
	vr_group_unlock(group);
}

void vr_pp_scheduler_disable_group(struct vr_group *group)
{
	VR_DEBUG_ASSERT_POINTER(group);

	vr_pp_scheduler_suspend();

	_vr_osk_pm_dev_ref_add_no_power_on();

	vr_pp_scheduler_disable_group_internal(group);

	_vr_osk_pm_dev_ref_dec_no_power_on();

	vr_pp_scheduler_resume();
}

static void vr_pp_scheduler_notify_core_change(u32 num_cores)
{
	vr_bool done = VR_FALSE;

	if (vr_is_vr450()) {
		return;
	}

	/*
	 * This function gets a bit complicated because we can't hold the session lock while
	 * allocating notification objects.
	 */

	while (!done) {
		u32 i;
		u32 num_sessions_alloc;
		u32 num_sessions_with_lock;
		u32 used_notification_objects = 0;
		_vr_osk_notification_t **notobjs;

		/* Pre allocate the number of notifications objects we need right now (might change after lock has been taken) */
		num_sessions_alloc = vr_session_get_count();
		if (0 == num_sessions_alloc) {
			/* No sessions to report to */
			return;
		}

		notobjs = (_vr_osk_notification_t **)_vr_osk_malloc(sizeof(_vr_osk_notification_t *) * num_sessions_alloc);
		if (NULL == notobjs) {
			VR_PRINT_ERROR(("Failed to notify user space session about num PP core change (alloc failure)\n"));
			/* there is probably no point in trying again, system must be really low on memory and probably unusable now anyway */
			return;
		}

		for (i = 0; i < num_sessions_alloc; i++) {
			notobjs[i] = _vr_osk_notification_create(_VR_NOTIFICATION_PP_NUM_CORE_CHANGE, sizeof(_vr_uk_pp_num_cores_changed_s));
			if (NULL != notobjs[i]) {
				_vr_uk_pp_num_cores_changed_s *data = notobjs[i]->result_buffer;
				data->number_of_enabled_cores = num_cores;
			} else {
				VR_PRINT_ERROR(("Failed to notify user space session about num PP core change (alloc failure %u)\n", i));
			}
		}

		vr_session_lock();

		/* number of sessions will not change while we hold the lock */
		num_sessions_with_lock = vr_session_get_count();

		if (num_sessions_alloc >= num_sessions_with_lock) {
			/* We have allocated enough notification objects for all the sessions atm */
			struct vr_session_data *session, *tmp;
			VR_SESSION_FOREACH(session, tmp, link) {
				VR_DEBUG_ASSERT(used_notification_objects < num_sessions_alloc);
				if (NULL != notobjs[used_notification_objects]) {
					vr_session_send_notification(session, notobjs[used_notification_objects]);
					notobjs[used_notification_objects] = NULL; /* Don't track this notification object any more */
				}
				used_notification_objects++;
			}
			done = VR_TRUE;
		}

		vr_session_unlock();

		/* Delete any remaining/unused notification objects */
		for (; used_notification_objects < num_sessions_alloc; used_notification_objects++) {
			if (NULL != notobjs[used_notification_objects]) {
				_vr_osk_notification_delete(notobjs[used_notification_objects]);
			}
		}

		_vr_osk_free(notobjs);
	}
}

static void vr_pp_scheduler_core_scale_up(unsigned int target_core_nr)
{
	VR_DEBUG_PRINT(2, ("Requesting %d cores: enabling %d cores\n", target_core_nr, target_core_nr - enabled_cores));

	_vr_osk_pm_dev_ref_add_no_power_on();
	_vr_osk_pm_dev_barrier();

	while (target_core_nr > enabled_cores) {
		/*
		 * If there are any cores which do not belong to any domain,
		 * then these will always be found at the head of the list and
		 * we'll thus enabled these first.
		 */

		vr_pp_scheduler_lock();

		if (!_vr_osk_list_empty(&group_list_disabled)) {
			struct vr_group *group;

			group = _VR_OSK_LIST_ENTRY(group_list_disabled.next, struct vr_group, pp_scheduler_list);

			VR_DEBUG_ASSERT_POINTER(group);
			VR_DEBUG_ASSERT(VR_GROUP_STATE_DISABLED == group->state);

			vr_pp_scheduler_unlock();

			vr_pp_scheduler_enable_group_internal(group);
		} else {
			vr_pp_scheduler_unlock();
			break; /* no more groups on disabled list */
		}
	}

	_vr_osk_pm_dev_ref_dec_no_power_on();

	vr_pp_scheduler_schedule();
}

static void vr_pp_scheduler_core_scale_down(unsigned int target_core_nr)
{
	VR_DEBUG_PRINT(2, ("Requesting %d cores: disabling %d cores\n", target_core_nr, enabled_cores - target_core_nr));

	vr_pp_scheduler_suspend();

	VR_DEBUG_ASSERT(_vr_osk_list_empty(&group_list_working));

	_vr_osk_pm_dev_ref_add_no_power_on();

	if (NULL != vr_pmu_get_global_pmu_core()) {
		int i;

		for (i = VR_MAX_NUMBER_OF_DOMAINS - 1; i >= 0; i--) {
			if (target_core_nr < enabled_cores) {
				struct vr_pm_domain *domain;

				domain = vr_pm_domain_get_from_index(i);

				/* Domain is valid and has pp cores */
				if ((NULL != domain) && (NULL != domain->group_list)) {
					struct vr_group *group;

					VR_PM_DOMAIN_FOR_EACH_GROUP(group, domain) {
						/* If group is pp core */
						if (NULL != vr_group_get_pp_core(group)) {
							vr_pp_scheduler_disable_group_internal(group);
							if (target_core_nr >= enabled_cores) {
								break;
							}
						}
					}
				}
			} else {
				break;
			}
		}
	}

	/*
	 * Didn't find enough cores associated with a power domain,
	 * so we need to disable cores which we can't power off with the PMU.
	 * Start with physical groups used by the scheduler,
	 * then remove physical from virtual if even more groups are needed.
	 */

	while (target_core_nr < enabled_cores) {
		vr_pp_scheduler_lock();
		if (!_vr_osk_list_empty(&group_list_idle)) {
			struct vr_group *group;

			group = _VR_OSK_LIST_ENTRY(group_list_idle.next, struct vr_group, pp_scheduler_list);
			VR_DEBUG_ASSERT_POINTER(group);

			vr_pp_scheduler_unlock();

			vr_pp_scheduler_disable_group_internal(group);
		} else {
			vr_pp_scheduler_unlock();
			break; /* No more physical groups */
		}
	}

	if (vr_pp_scheduler_has_virtual_group()) {
		while (target_core_nr < enabled_cores) {
			vr_group_lock(virtual_group);
			if (!_vr_osk_list_empty(&virtual_group->group_list)) {
				struct vr_group *group;

				group = _VR_OSK_LIST_ENTRY(virtual_group->group_list.next, struct vr_group, group_list);
				VR_DEBUG_ASSERT_POINTER(group);

				vr_group_unlock(virtual_group);

				vr_pp_scheduler_disable_group_internal(group);
			} else {
				vr_group_unlock(virtual_group);
				break; /* No more physical groups in virtual group */
			}
		}
	}

	_vr_osk_pm_dev_ref_dec_no_power_on();

	vr_pp_scheduler_resume();
}

int vr_pp_scheduler_set_perf_level(unsigned int target_core_nr, vr_bool override)
{
	if (target_core_nr == enabled_cores) return 0;
	if (VR_FALSE == core_scaling_enabled && VR_FALSE == override) return -EPERM;
	if (target_core_nr > num_cores) return -EINVAL;
	if (0 == target_core_nr) return -EINVAL;

	if (target_core_nr > enabled_cores) {
		vr_pp_scheduler_core_scale_up(target_core_nr);
	} else if (target_core_nr < enabled_cores) {
		vr_pp_scheduler_core_scale_down(target_core_nr);
	}

	if (target_core_nr != enabled_cores) {
		VR_DEBUG_PRINT(2, ("Core scaling failed, target number: %d, actual number: %d\n", target_core_nr, enabled_cores));
	}

	vr_pp_scheduler_notify_core_change(enabled_cores);

	return 0;
}

void vr_pp_scheduler_core_scaling_enable(void)
{
	/* PS: Core scaling is by default enabled */
	core_scaling_enabled = VR_TRUE;
}

void vr_pp_scheduler_core_scaling_disable(void)
{
	core_scaling_enabled = VR_FALSE;
}

vr_bool vr_pp_scheduler_core_scaling_is_enabled(void)
{
	return core_scaling_enabled;
}

static void vr_pp_scheduler_job_queued(void)
{
	/* We hold a PM reference for every job we hold queued (and running) */
	_vr_osk_pm_dev_ref_add();

	if (vr_utilization_enabled()) {
		/*
		 * We cheat a little bit by counting the PP as busy from the time a PP job is queued.
		 * This will be fine because we only loose the tiny idle gap between jobs, but
		 * we will instead get less utilization work to do (less locks taken)
		 */
		vr_utilization_pp_start();
	}
}

static void vr_pp_scheduler_job_completed(void)
{
	/* Release the PM reference we got in the vr_pp_scheduler_job_queued() function */
	_vr_osk_pm_dev_ref_dec();

	if (vr_utilization_enabled()) {
		vr_utilization_pp_end();
	}
}

static void vr_pp_scheduler_abort_job_and_unlock_scheduler(struct vr_pp_job *job)
{
	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_LOCK_HELD(pp_scheduler_lock);

	/* This job should not be on any lists. */
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->list));
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&job->session_list));

	_vr_osk_list_delinit(&job->session_fb_lookup_list);

	vr_pp_scheduler_unlock();

	/* Release tracker. */
	vr_timeline_tracker_release(&job->tracker);
}

static vr_scheduler_mask vr_pp_scheduler_queue_job(struct vr_pp_job *job)
{
	_vr_osk_list_t *queue = NULL;
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;
	struct vr_pp_job *iter, *tmp;

	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->session);

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE)
	if (vr_pp_job_needs_dma_buf_mapping(job)) {
		vr_dma_buf_map_job(job);
	}
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE) */

	vr_pp_scheduler_lock();

	if (unlikely(job->session->is_aborting)) {
		/* Before checking if the session is aborting, the scheduler must be locked. */
		VR_DEBUG_ASSERT_LOCK_HELD(pp_scheduler_lock);

		VR_DEBUG_PRINT(2, ("Vr PP scheduler: Job %u (0x%08X) queued while session is aborting.\n", vr_pp_job_get_id(job), job));

		vr_pp_scheduler_abort_job_and_unlock_scheduler(job);

		/* Delete job. */
#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE)
		vr_pp_scheduler_deferred_job_delete(job);
#else
		vr_pp_job_delete(job);
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_DELETE) */
		vr_pp_scheduler_job_completed();

		/* Since we are aborting we ignore the scheduler mask. */
		return VR_SCHEDULER_MASK_EMPTY;
	}

#if defined(CONFIG_GPU_TRACEPOINTS) && defined(CONFIG_TRACEPOINTS)
	trace_gpu_job_enqueue(vr_pp_job_get_tid(job), vr_pp_job_get_id(job), "PP");
#endif

	_vr_osk_profiling_add_event(VR_PROFILING_EVENT_TYPE_SINGLE | VR_PROFILING_EVENT_CHANNEL_SOFTWARE | VR_PROFILING_EVENT_REASON_SINGLE_SW_PP_ENQUEUE, job->pid, job->tid, job->uargs.frame_builder_id, job->uargs.flush_id, 0);

	job->cache_order = vr_scheduler_get_new_cache_order();

	/* Determine which queue the job should be added to. */
	if (vr_pp_job_is_virtual(job)) {
		if (job->session->use_high_priority_job_queue) {
			queue = &virtual_job_queue.high_pri;
		} else {
			queue = &virtual_job_queue.normal_pri;
		}

		virtual_job_queue.depth += 1;

		/* Set schedule bitmask if the virtual group is idle. */
		if (VIRTUAL_GROUP_IDLE == virtual_group_state) {
			schedule_mask |= VR_SCHEDULER_MASK_PP;
		}
	} else {
		if (job->session->use_high_priority_job_queue) {
			queue = &job_queue.high_pri;
		} else {
			queue = &job_queue.normal_pri;
		}

		job_queue.depth += vr_pp_job_get_sub_job_count(job);

		/* Set schedule bitmask if there are physical PP cores available, or if there is an
		 * idle virtual group. */
		if (!_vr_osk_list_empty(&group_list_idle)
		    || (vr_pp_scheduler_has_virtual_group()
		        && (VIRTUAL_GROUP_IDLE == virtual_group_state))) {
			schedule_mask |= VR_SCHEDULER_MASK_PP;
		}
	}

	/* Find position in queue where job should be added. */
	_VR_OSK_LIST_FOREACHENTRY_REVERSE(iter, tmp, queue, struct vr_pp_job, list) {
		if (vr_pp_job_should_start_after(job, iter)) {
			break;
		}
	}

	/* Add job to queue. */
	_vr_osk_list_add(&job->list, &iter->list);

	/* Add job to session list. */
	_vr_osk_list_addtail(&job->session_list, &(job->session->pp_job_list));

	VR_DEBUG_PRINT(3, ("Vr PP scheduler: %s job %u (0x%08X) with %u parts queued.\n",
	                     vr_pp_job_is_virtual(job) ? "Virtual" : "Physical",
	                     vr_pp_job_get_id(job), job, vr_pp_job_get_sub_job_count(job)));

	vr_pp_scheduler_unlock();

	return schedule_mask;
}

vr_scheduler_mask vr_pp_scheduler_activate_job(struct vr_pp_job *job)
{
	vr_scheduler_mask schedule_mask = VR_SCHEDULER_MASK_EMPTY;

	VR_DEBUG_ASSERT_POINTER(job);
	VR_DEBUG_ASSERT_POINTER(job->session);

	VR_DEBUG_PRINT(4, ("Vr PP scheduler: Timeline activation for job %u (0x%08X).\n", vr_pp_job_get_id(job), job));

	if (VR_TIMELINE_ACTIVATION_ERROR_FATAL_BIT & job->tracker.activation_error) {
		VR_DEBUG_PRINT(2, ("Vr PP scheduler: Job %u (0x%08X) activated with error, aborting.\n", vr_pp_job_get_id(job), job));

		vr_pp_scheduler_lock();
		vr_pp_scheduler_abort_job_and_unlock_scheduler(job);

		vr_pp_job_mark_sub_job_completed(job, VR_FALSE); /* Flagging the job as failed. */
		vr_pp_scheduler_finalize_job(job);

		return VR_SCHEDULER_MASK_EMPTY;
	}

	/* PP job is ready to run, queue it. */

#if defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE)
	if (vr_pp_job_needs_dma_buf_mapping(job)) {
		vr_pp_scheduler_deferred_job_queue(job);

		return VR_SCHEDULER_MASK_EMPTY;
	}
#endif /* defined(VR_PP_SCHEDULER_USE_DEFERRED_JOB_QUEUE) */

	schedule_mask = vr_pp_scheduler_queue_job(job);

	return schedule_mask;
}
