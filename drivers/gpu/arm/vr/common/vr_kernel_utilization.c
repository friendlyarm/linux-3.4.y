/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_utilization.h"
#include "vr_osk.h"
#include "vr_osk_vr.h"
#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_scheduler.h"

/* Thresholds for GP bound detection. */
#define VR_GP_BOUND_GP_UTILIZATION_THRESHOLD 240
#define VR_GP_BOUND_PP_UTILIZATION_THRESHOLD 250

/* Define how often to calculate and report GPU utilization, in milliseconds */
static _vr_osk_spinlock_irq_t *time_data_lock;

static u32 num_running_gp_cores;
static u32 num_running_pp_cores;

static u64 work_start_time_gpu = 0;
static u64 work_start_time_gp = 0;
static u64 work_start_time_pp = 0;
static u64 accumulated_work_time_gpu = 0;
static u64 accumulated_work_time_gp = 0;
static u64 accumulated_work_time_pp = 0;

static u64 period_start_time = 0;
static _vr_osk_timer_t *utilization_timer = NULL;
static vr_bool timer_running = VR_FALSE;

static u32 last_utilization_gpu = 0 ;
static u32 last_utilization_gp = 0 ;
static u32 last_utilization_pp = 0 ;

static u32 vr_utilization_timeout = 1000;
void (*vr_utilization_callback)(struct vr_gpu_utilization_data *data) = NULL;
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
extern void vr_power_performance_policy_callback(struct vr_gpu_utilization_data *data);
#define NUMBER_OF_NANOSECONDS_PER_SECOND  1000000000ULL

static u32 calculate_window_render_fps(u64 time_period)
{
	u32 max_window_number;
	u64 tmp;
	u64 max = time_period;
	u32 leading_zeroes;
	u32 shift_val;
	u32 time_period_shift;
	u32 max_window_number_shift;
	u32 ret_val;

	max_window_number = vr_session_max_window_num();
	/* To avoid float division, extend the dividend to ns unit */
	tmp = (u64)max_window_number * NUMBER_OF_NANOSECONDS_PER_SECOND;
	if (tmp > time_period) {
		max = tmp;
	}

	/*
	 * We may have 64-bit values, a dividend or a divisor or both
	 * To avoid dependencies to a 64-bit divider, we shift down the two values
	 * equally first.
	 */
	leading_zeroes = _vr_osk_clz((u32)(max >> 32));
	shift_val = 32 - leading_zeroes;

	time_period_shift = (u32)(time_period >> shift_val);
	max_window_number_shift = (u32)(tmp >> shift_val);

	ret_val = max_window_number_shift / time_period_shift;

	return ret_val;
}
#endif  /* defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY) */

static void calculate_gpu_utilization(void* arg)
{
	u64 time_now;
	u64 time_period;
	u32 leading_zeroes;
	u32 shift_val;
	u32 work_norvrzed_gpu;
	u32 work_norvrzed_gp;
	u32 work_norvrzed_pp;
	u32 period_norvrzed;
	u32 utilization_gpu;
	u32 utilization_gp;
	u32 utilization_pp;
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	u32 window_render_fps;
#endif

	_vr_osk_spinlock_irq_lock(time_data_lock);

	if (accumulated_work_time_gpu == 0 && work_start_time_gpu == 0) {
		/*
		 * No work done for this period
		 * - No need to reschedule timer
		 * - Report zero usage
		 */
		timer_running = VR_FALSE;

		last_utilization_gpu = 0;
		last_utilization_gp = 0;
		last_utilization_pp = 0;

		_vr_osk_spinlock_irq_unlock(time_data_lock);

		if (NULL != vr_utilization_callback) {
			struct vr_gpu_utilization_data data = { 0, };
			vr_utilization_callback(&data);
		}

		vr_scheduler_hint_disable(VR_SCHEDULER_HINT_GP_BOUND);

		return;
	}

	time_now = _vr_osk_time_get_ns();

	time_period = time_now - period_start_time;

	/* If we are currently busy, update working period up to now */
	if (work_start_time_gpu != 0) {
		accumulated_work_time_gpu += (time_now - work_start_time_gpu);
		work_start_time_gpu = time_now;

		/* GP and/or PP will also be busy if the GPU is busy at this point */

		if (work_start_time_gp != 0) {
			accumulated_work_time_gp += (time_now - work_start_time_gp);
			work_start_time_gp = time_now;
		}

		if (work_start_time_pp != 0) {
			accumulated_work_time_pp += (time_now - work_start_time_pp);
			work_start_time_pp = time_now;
		}
	}

	/*
	 * We have two 64-bit values, a dividend and a divisor.
	 * To avoid dependencies to a 64-bit divider, we shift down the two values
	 * equally first.
	 * We shift the dividend up and possibly the divisor down, making the result X in 256.
	 */

	/* Shift the 64-bit values down so they fit inside a 32-bit integer */
	leading_zeroes = _vr_osk_clz((u32)(time_period >> 32));
	shift_val = 32 - leading_zeroes;
	work_norvrzed_gpu = (u32)(accumulated_work_time_gpu >> shift_val);
	work_norvrzed_gp = (u32)(accumulated_work_time_gp >> shift_val);
	work_norvrzed_pp = (u32)(accumulated_work_time_pp >> shift_val);
	period_norvrzed = (u32)(time_period >> shift_val);

	/*
	 * Now, we should report the usage in parts of 256
	 * this means we must shift up the dividend or down the divisor by 8
	 * (we could do a combination, but we just use one for simplicity,
	 * but the end result should be good enough anyway)
	 */
	if (period_norvrzed > 0x00FFFFFF) {
		/* The divisor is so big that it is safe to shift it down */
		period_norvrzed >>= 8;
	} else {
		/*
		 * The divisor is so small that we can shift up the dividend, without loosing any data.
		 * (dividend is always smaller than the divisor)
		 */
		work_norvrzed_gpu <<= 8;
		work_norvrzed_gp <<= 8;
		work_norvrzed_pp <<= 8;
	}

	utilization_gpu = work_norvrzed_gpu / period_norvrzed;
	utilization_gp = work_norvrzed_gp / period_norvrzed;
	utilization_pp = work_norvrzed_pp / period_norvrzed;

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	window_render_fps = calculate_window_render_fps(time_period);
#endif

	last_utilization_gpu = utilization_gpu;
	last_utilization_gp = utilization_gp;
	last_utilization_pp = utilization_pp;

	if ((VR_GP_BOUND_GP_UTILIZATION_THRESHOLD < last_utilization_gp) &&
	    (VR_GP_BOUND_PP_UTILIZATION_THRESHOLD > last_utilization_pp)) {
		vr_scheduler_hint_enable(VR_SCHEDULER_HINT_GP_BOUND);
	} else {
		vr_scheduler_hint_disable(VR_SCHEDULER_HINT_GP_BOUND);
	}

	/* starting a new period */
	accumulated_work_time_gpu = 0;
	accumulated_work_time_gp = 0;
	accumulated_work_time_pp = 0;
	period_start_time = time_now;

	_vr_osk_spinlock_irq_unlock(time_data_lock);

	_vr_osk_timer_add(utilization_timer, _vr_osk_time_mstoticks(vr_utilization_timeout));

	if (NULL != vr_utilization_callback) {
		struct vr_gpu_utilization_data data = {
			utilization_gpu, utilization_gp, utilization_pp,
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
			window_render_fps, window_render_fps
#endif
		};
		vr_utilization_callback(&data);
	}
}

_vr_osk_errcode_t vr_utilization_init(void)
{
#if USING_GPU_UTILIZATION
	struct _vr_osk_device_data data;
	if (_VR_OSK_ERR_OK == _vr_osk_device_data_get(&data)) {
		/* Use device specific settings (if defined) */
		if (0 != data.utilization_interval) {
			vr_utilization_timeout = data.utilization_interval;
		}
		if (NULL != data.utilization_callback) {
			vr_utilization_callback = data.utilization_callback;
			VR_DEBUG_PRINT(2, ("Vr GPU Utilization: Platform has it's own policy \n"));
			VR_DEBUG_PRINT(2, ("Vr GPU Utilization: Utilization handler installed with interval %u\n", vr_utilization_timeout));
		}
	}
#endif

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	if (vr_utilization_callback == NULL) {
		VR_DEBUG_PRINT(2, ("Vr GPU Utilization: VR Power Performance Policy Algorithm \n"));
		vr_utilization_callback = vr_power_performance_policy_callback;
	}
#endif

	if (NULL == vr_utilization_callback) {
		VR_DEBUG_PRINT(2, ("Vr GPU Utilization: No utilization handler installed\n"));
	}

	time_data_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_UTILIZATION);

	if (NULL == time_data_lock) {
		return _VR_OSK_ERR_FAULT;
	}

	num_running_gp_cores = 0;
	num_running_pp_cores = 0;

	utilization_timer = _vr_osk_timer_init();
	if (NULL == utilization_timer) {
		_vr_osk_spinlock_irq_term(time_data_lock);
		return _VR_OSK_ERR_FAULT;
	}
	_vr_osk_timer_setcallback(utilization_timer, calculate_gpu_utilization, NULL);

	return _VR_OSK_ERR_OK;
}

void vr_utilization_suspend(void)
{
	_vr_osk_spinlock_irq_lock(time_data_lock);

	if (timer_running == VR_TRUE) {
		timer_running = VR_FALSE;
		_vr_osk_spinlock_irq_unlock(time_data_lock);
		_vr_osk_timer_del(utilization_timer);
		return;
	}

	_vr_osk_spinlock_irq_unlock(time_data_lock);
}

void vr_utilization_term(void)
{
	if (NULL != utilization_timer) {
		_vr_osk_timer_del(utilization_timer);
		timer_running = VR_FALSE;
		_vr_osk_timer_term(utilization_timer);
		utilization_timer = NULL;
	}

	_vr_osk_spinlock_irq_term(time_data_lock);
}

void vr_utilization_gp_start(void)
{
	_vr_osk_spinlock_irq_lock(time_data_lock);

	++num_running_gp_cores;
	if (1 == num_running_gp_cores) {
		u64 time_now = _vr_osk_time_get_ns();

		/* First GP core started, consider GP busy from now and onwards */
		work_start_time_gp = time_now;

		if (0 == num_running_pp_cores) {
			/*
			 * There are no PP cores running, so this is also the point
			 * at which we consider the GPU to be busy as well.
			 */
			work_start_time_gpu = time_now;
		}

		/* Start a new period (and timer) if needed */
		if (timer_running != VR_TRUE) {
			timer_running = VR_TRUE;
			period_start_time = time_now;

			/* Clear session->number_of_window_jobs */
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
			vr_session_max_window_num();
#endif
			_vr_osk_spinlock_irq_unlock(time_data_lock);

			_vr_osk_timer_add(utilization_timer, _vr_osk_time_mstoticks(vr_utilization_timeout));
		} else {
			_vr_osk_spinlock_irq_unlock(time_data_lock);
		}
	} else {
		/* Nothing to do */
		_vr_osk_spinlock_irq_unlock(time_data_lock);
	}
}

void vr_utilization_pp_start(void)
{
	_vr_osk_spinlock_irq_lock(time_data_lock);

	++num_running_pp_cores;
	if (1 == num_running_pp_cores) {
		u64 time_now = _vr_osk_time_get_ns();

		/* First PP core started, consider PP busy from now and onwards */
		work_start_time_pp = time_now;

		if (0 == num_running_gp_cores) {
			/*
			 * There are no GP cores running, so this is also the point
			 * at which we consider the GPU to be busy as well.
			 */
			work_start_time_gpu = time_now;
		}

		/* Start a new period (and timer) if needed */
		if (timer_running != VR_TRUE) {
			timer_running = VR_TRUE;
			period_start_time = time_now;

			/* Clear session->number_of_window_jobs */
#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
			vr_session_max_window_num();
#endif
			_vr_osk_spinlock_irq_unlock(time_data_lock);

			_vr_osk_timer_add(utilization_timer, _vr_osk_time_mstoticks(vr_utilization_timeout));
		} else {
			_vr_osk_spinlock_irq_unlock(time_data_lock);
		}
	} else {
		/* Nothing to do */
		_vr_osk_spinlock_irq_unlock(time_data_lock);
	}
}

void vr_utilization_gp_end(void)
{
	_vr_osk_spinlock_irq_lock(time_data_lock);

	--num_running_gp_cores;
	if (0 == num_running_gp_cores) {
		u64 time_now = _vr_osk_time_get_ns();

		/* Last GP core ended, consider GP idle from now and onwards */
		accumulated_work_time_gp += (time_now - work_start_time_gp);
		work_start_time_gp = 0;

		if (0 == num_running_pp_cores) {
			/*
			 * There are no PP cores running, so this is also the point
			 * at which we consider the GPU to be idle as well.
			 */
			accumulated_work_time_gpu += (time_now - work_start_time_gpu);
			work_start_time_gpu = 0;
		}
	}

	_vr_osk_spinlock_irq_unlock(time_data_lock);
}

void vr_utilization_pp_end(void)
{
	_vr_osk_spinlock_irq_lock(time_data_lock);

	--num_running_pp_cores;
	if (0 == num_running_pp_cores) {
		u64 time_now = _vr_osk_time_get_ns();

		/* Last PP core ended, consider PP idle from now and onwards */
		accumulated_work_time_pp += (time_now - work_start_time_pp);
		work_start_time_pp = 0;

		if (0 == num_running_gp_cores) {
			/*
			 * There are no GP cores running, so this is also the point
			 * at which we consider the GPU to be idle as well.
			 */
			accumulated_work_time_gpu += (time_now - work_start_time_gpu);
			work_start_time_gpu = 0;
		}
	}

	_vr_osk_spinlock_irq_unlock(time_data_lock);
}

u32 _vr_ukk_utilization_gp_pp(void)
{
	return last_utilization_gpu;
}

u32 _vr_ukk_utilization_gp(void)
{
	return last_utilization_gp;
}

u32 _vr_ukk_utilization_pp(void)
{
	return last_utilization_pp;
}
