/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_scheduler.h"

#include "vr_kernel_common.h"
#include "vr_osk.h"

vr_bool vr_scheduler_hints[VR_SCHEDULER_HINT_MAX];

static _vr_osk_atomic_t vr_job_id_autonumber;
static _vr_osk_atomic_t vr_job_cache_order_autonumber;

static _vr_osk_wq_work_t *pp_scheduler_wq_high_pri = NULL;
static _vr_osk_wq_work_t *gp_scheduler_wq_high_pri = NULL;

static void vr_scheduler_wq_schedule_pp(void *arg)
{
	VR_IGNORE(arg);

	vr_pp_scheduler_schedule();
}

static void vr_scheduler_wq_schedule_gp(void *arg)
{
	VR_IGNORE(arg);

	vr_gp_scheduler_schedule();
}

_vr_osk_errcode_t vr_scheduler_initialize(void)
{
	if ( _VR_OSK_ERR_OK != _vr_osk_atomic_init(&vr_job_id_autonumber, 0)) {
		VR_DEBUG_PRINT(1,  ("Initialization of atomic job id counter failed.\n"));
		return _VR_OSK_ERR_FAULT;
	}

	if ( _VR_OSK_ERR_OK != _vr_osk_atomic_init(&vr_job_cache_order_autonumber, 0)) {
		VR_DEBUG_PRINT(1,  ("Initialization of atomic job cache order counter failed.\n"));
		_vr_osk_atomic_term(&vr_job_id_autonumber);
		return _VR_OSK_ERR_FAULT;
	}

	pp_scheduler_wq_high_pri = _vr_osk_wq_create_work_high_pri(vr_scheduler_wq_schedule_pp, NULL);
	if (NULL == pp_scheduler_wq_high_pri) {
		_vr_osk_atomic_term(&vr_job_cache_order_autonumber);
		_vr_osk_atomic_term(&vr_job_id_autonumber);
		return _VR_OSK_ERR_NOMEM;
	}

	gp_scheduler_wq_high_pri = _vr_osk_wq_create_work_high_pri(vr_scheduler_wq_schedule_gp, NULL);
	if (NULL == gp_scheduler_wq_high_pri) {
		_vr_osk_wq_delete_work(pp_scheduler_wq_high_pri);
		_vr_osk_atomic_term(&vr_job_cache_order_autonumber);
		_vr_osk_atomic_term(&vr_job_id_autonumber);
		return _VR_OSK_ERR_NOMEM;
	}

	return _VR_OSK_ERR_OK;
}

void vr_scheduler_terminate(void)
{
	_vr_osk_wq_delete_work(gp_scheduler_wq_high_pri);
	_vr_osk_wq_delete_work(pp_scheduler_wq_high_pri);
	_vr_osk_atomic_term(&vr_job_cache_order_autonumber);
	_vr_osk_atomic_term(&vr_job_id_autonumber);
}

u32 vr_scheduler_get_new_id(void)
{
	u32 job_id = _vr_osk_atomic_inc_return(&vr_job_id_autonumber);
	return job_id;
}

u32 vr_scheduler_get_new_cache_order(void)
{
	u32 job_cache_order = _vr_osk_atomic_inc_return(&vr_job_cache_order_autonumber);
	return job_cache_order;
}

void vr_scheduler_schedule_from_mask(vr_scheduler_mask mask, vr_bool deferred_schedule)
{
	if (VR_SCHEDULER_MASK_GP & mask) {
		/* GP needs scheduling. */
		if (deferred_schedule) {
			/* Schedule GP deferred. */
			_vr_osk_wq_schedule_work_high_pri(gp_scheduler_wq_high_pri);
		} else {
			/* Schedule GP now. */
			vr_gp_scheduler_schedule();
		}
	}

	if (VR_SCHEDULER_MASK_PP & mask) {
		/* PP needs scheduling. */
		if (deferred_schedule) {
			/* Schedule PP deferred. */
			_vr_osk_wq_schedule_work_high_pri(pp_scheduler_wq_high_pri);
		} else {
			/* Schedule PP now. */
			vr_pp_scheduler_schedule();
		}
	}
}
