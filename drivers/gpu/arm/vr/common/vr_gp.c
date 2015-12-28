/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_gp.h"
#include "vr_hw_core.h"
#include "vr_group.h"
#include "vr_osk.h"
#include "regs/vr_gp_regs.h"
#include "vr_kernel_common.h"
#include "vr_kernel_core.h"
#if defined(CONFIG_VR400_PROFILING)
#include "vr_osk_profiling.h"
#endif

static struct vr_gp_core *vr_global_gp_core = NULL;

/* Interrupt handlers */
static void vr_gp_irq_probe_trigger(void *data);
static _vr_osk_errcode_t vr_gp_irq_probe_ack(void *data);

struct vr_gp_core *vr_gp_create(const _vr_osk_resource_t * resource, struct vr_group *group)
{
	struct vr_gp_core* core = NULL;

	VR_DEBUG_ASSERT(NULL == vr_global_gp_core);
	VR_DEBUG_PRINT(2, ("Vr GP: Creating Vr GP core: %s\n", resource->description));

	core = _vr_osk_malloc(sizeof(struct vr_gp_core));
	if (NULL != core) {
		if (_VR_OSK_ERR_OK == vr_hw_core_create(&core->hw_core, resource, VRGP2_REGISTER_ADDRESS_SPACE_SIZE)) {
			_vr_osk_errcode_t ret;

			ret = vr_gp_reset(core);

			if (_VR_OSK_ERR_OK == ret) {
				ret = vr_group_add_gp_core(group, core);
				if (_VR_OSK_ERR_OK == ret) {
					/* Setup IRQ handlers (which will do IRQ probing if needed) */
					core->irq = _vr_osk_irq_init(resource->irq,
					                               vr_group_upper_half_gp,
					                               group,
					                               vr_gp_irq_probe_trigger,
					                               vr_gp_irq_probe_ack,
					                               core,
					                               resource->description);
					if (NULL != core->irq) {
						VR_DEBUG_PRINT(4, ("Vr GP: set global gp core from 0x%08X to 0x%08X\n", vr_global_gp_core, core));
						vr_global_gp_core = core;

						return core;
					} else {
						VR_PRINT_ERROR(("Vr GP: Failed to setup interrupt handlers for GP core %s\n", core->hw_core.description));
					}
					vr_group_remove_gp_core(group);
				} else {
					VR_PRINT_ERROR(("Vr GP: Failed to add core %s to group\n", core->hw_core.description));
				}
			}
			vr_hw_core_delete(&core->hw_core);
		}

		_vr_osk_free(core);
	} else {
		VR_PRINT_ERROR(("Failed to allocate memory for GP core\n"));
	}

	return NULL;
}

void vr_gp_delete(struct vr_gp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);

	_vr_osk_irq_term(core->irq);
	vr_hw_core_delete(&core->hw_core);
	vr_global_gp_core = NULL;
	_vr_osk_free(core);
}

void vr_gp_stop_bus(struct vr_gp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);

	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_CMD, VRGP2_REG_VAL_CMD_STOP_BUS);
}

_vr_osk_errcode_t vr_gp_stop_bus_wait(struct vr_gp_core *core)
{
	int i;

	VR_DEBUG_ASSERT_POINTER(core);

	/* Send the stop bus command. */
	vr_gp_stop_bus(core);

	/* Wait for bus to be stopped */
	for (i = 0; i < VR_REG_POLL_COUNT_FAST; i++) {
		if (vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_STATUS) & VRGP2_REG_VAL_STATUS_BUS_STOPPED) {
			break;
		}
	}

	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_PRINT_ERROR(("Vr GP: Failed to stop bus on %s\n", core->hw_core.description));
		return _VR_OSK_ERR_FAULT;
	}
	return _VR_OSK_ERR_OK;
}

void vr_gp_hard_reset(struct vr_gp_core *core)
{
	const u32 reset_wait_target_register = VRGP2_REG_ADDR_MGMT_WRITE_BOUND_LOW;
	const u32 reset_invalid_value = 0xC0FFE000;
	const u32 reset_check_value = 0xC01A0000;
	const u32 reset_default_value = 0;
	int i;

	VR_DEBUG_ASSERT_POINTER(core);
	VR_DEBUG_PRINT(4, ("Vr GP: Hard reset of core %s\n", core->hw_core.description));

	vr_hw_core_register_write(&core->hw_core, reset_wait_target_register, reset_invalid_value);

	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_CMD, VRGP2_REG_VAL_CMD_RESET);

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; i++) {
		vr_hw_core_register_write(&core->hw_core, reset_wait_target_register, reset_check_value);
		if (reset_check_value == vr_hw_core_register_read(&core->hw_core, reset_wait_target_register)) {
			break;
		}
	}

	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_PRINT_ERROR(("Vr GP: The hard reset loop didn't work, unable to recover\n"));
	}

	vr_hw_core_register_write(&core->hw_core, reset_wait_target_register, reset_default_value); /* set it back to the default */
	/* Re-enable interrupts */
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_CLEAR, VRGP2_REG_VAL_IRQ_MASK_ALL);
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK, VRGP2_REG_VAL_IRQ_MASK_USED);

}

void vr_gp_reset_async(struct vr_gp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);

	VR_DEBUG_PRINT(4, ("Vr GP: Reset of core %s\n", core->hw_core.description));

	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK, 0); /* disable the IRQs */
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_CLEAR, VR400GP_REG_VAL_IRQ_RESET_COMPLETED);
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_CMD, VR400GP_REG_VAL_CMD_SOFT_RESET);

}

_vr_osk_errcode_t vr_gp_reset_wait(struct vr_gp_core *core)
{
	int i;
	u32 rawstat = 0;

	VR_DEBUG_ASSERT_POINTER(core);

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; i++) {
		rawstat = vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_RAWSTAT);
		if (rawstat & VR400GP_REG_VAL_IRQ_RESET_COMPLETED) {
			break;
		}
	}

	if (i == VR_REG_POLL_COUNT_FAST) {
		VR_PRINT_ERROR(("Vr GP: Failed to reset core %s, rawstat: 0x%08x\n",
		                  core->hw_core.description, rawstat));
		return _VR_OSK_ERR_FAULT;
	}

	/* Re-enable interrupts */
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_CLEAR, VRGP2_REG_VAL_IRQ_MASK_ALL);
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK, VRGP2_REG_VAL_IRQ_MASK_USED);

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t vr_gp_reset(struct vr_gp_core *core)
{
	vr_gp_reset_async(core);
	return vr_gp_reset_wait(core);
}

void vr_gp_job_start(struct vr_gp_core *core, struct vr_gp_job *job)
{
	u32 startcmd = 0;
	u32 *frame_registers = vr_gp_job_get_frame_registers(job);
	u32 counter_src0 = vr_gp_job_get_perf_counter_src0(job);
	u32 counter_src1 = vr_gp_job_get_perf_counter_src1(job);

	VR_DEBUG_ASSERT_POINTER(core);

	if (vr_gp_job_has_vs_job(job)) {
		startcmd |= (u32) VRGP2_REG_VAL_CMD_START_VS;
	}

	if (vr_gp_job_has_plbu_job(job)) {
		startcmd |= (u32) VRGP2_REG_VAL_CMD_START_PLBU;
	}

	VR_DEBUG_ASSERT(0 != startcmd);

	vr_hw_core_register_write_array_relaxed(&core->hw_core, VRGP2_REG_ADDR_MGMT_VSCL_START_ADDR, frame_registers, VRGP2_NUM_REGS_FRAME);

	if (VR_HW_CORE_NO_COUNTER != counter_src0) {
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_PERF_CNT_0_SRC, counter_src0);
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_PERF_CNT_0_ENABLE, VRGP2_REG_VAL_PERF_CNT_ENABLE);
	}
	if (VR_HW_CORE_NO_COUNTER != counter_src1) {
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_PERF_CNT_1_SRC, counter_src1);
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_PERF_CNT_1_ENABLE, VRGP2_REG_VAL_PERF_CNT_ENABLE);
	}

	VR_DEBUG_PRINT(3, ("Vr GP: Starting job (0x%08x) on core %s with command 0x%08X\n", job, core->hw_core.description, startcmd));

	vr_hw_core_register_write_relaxed(&core->hw_core, VRGP2_REG_ADDR_MGMT_CMD, VRGP2_REG_VAL_CMD_UPDATE_PLBU_ALLOC);

	/* Barrier to make sure the previous register write is finished */
	_vr_osk_write_mem_barrier();

	/* This is the command that starts the core. */
	vr_hw_core_register_write_relaxed(&core->hw_core, VRGP2_REG_ADDR_MGMT_CMD, startcmd);

	/* Barrier to make sure the previous register write is finished */
	_vr_osk_write_mem_barrier();
}

void vr_gp_resume_with_new_heap(struct vr_gp_core *core, u32 start_addr, u32 end_addr)
{
	u32 irq_readout;

	VR_DEBUG_ASSERT_POINTER(core);

	irq_readout = vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_RAWSTAT);

	if (irq_readout & VRGP2_REG_VAL_IRQ_PLBU_OUT_OF_MEM) {
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_CLEAR, (VRGP2_REG_VAL_IRQ_PLBU_OUT_OF_MEM | VRGP2_REG_VAL_IRQ_HANG));
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK, VRGP2_REG_VAL_IRQ_MASK_USED); /* re-enable interrupts */
		vr_hw_core_register_write_relaxed(&core->hw_core, VRGP2_REG_ADDR_MGMT_PLBU_ALLOC_START_ADDR, start_addr);
		vr_hw_core_register_write_relaxed(&core->hw_core, VRGP2_REG_ADDR_MGMT_PLBU_ALLOC_END_ADDR, end_addr);

		VR_DEBUG_PRINT(3, ("Vr GP: Resuming job\n"));

		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_CMD, VRGP2_REG_VAL_CMD_UPDATE_PLBU_ALLOC);
		_vr_osk_write_mem_barrier();
	}
	/*
	 * else: core has been reset between PLBU_OUT_OF_MEM interrupt and this new heap response.
	 * A timeout or a page fault on Vr-200 PP core can cause this behaviour.
	 */
}

u32 vr_gp_core_get_version(struct vr_gp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);
	return vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_VERSION);
}

struct vr_gp_core *vr_gp_get_global_gp_core(void)
{
	return vr_global_gp_core;
}

/* ------------- interrupt handling below ------------------ */
static void vr_gp_irq_probe_trigger(void *data)
{
	struct vr_gp_core *core = (struct vr_gp_core *)data;

	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK, VRGP2_REG_VAL_IRQ_MASK_USED);
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_RAWSTAT, VRGP2_REG_VAL_CMD_FORCE_HANG);
	_vr_osk_mem_barrier();
}

static _vr_osk_errcode_t vr_gp_irq_probe_ack(void *data)
{
	struct vr_gp_core *core = (struct vr_gp_core *)data;
	u32 irq_readout;

	irq_readout = vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_STAT);
	if (VRGP2_REG_VAL_IRQ_FORCE_HANG & irq_readout) {
		vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_CLEAR, VRGP2_REG_VAL_IRQ_FORCE_HANG);
		_vr_osk_mem_barrier();
		return _VR_OSK_ERR_OK;
	}

	return _VR_OSK_ERR_FAULT;
}

/* ------ local helper functions below --------- */
#if VR_STATE_TRACKING
u32 vr_gp_dump_state(struct vr_gp_core *core, char *buf, u32 size)
{
	int n = 0;

	n += _vr_osk_snprintf(buf + n, size - n, "\tGP: %s\n", core->hw_core.description);

	return n;
}
#endif

void vr_gp_update_performance_counters(struct vr_gp_core *core, struct vr_gp_job *job, vr_bool suspend)
{
	u32 val0 = 0;
	u32 val1 = 0;
	u32 counter_src0 = vr_gp_job_get_perf_counter_src0(job);
	u32 counter_src1 = vr_gp_job_get_perf_counter_src1(job);

	if (VR_HW_CORE_NO_COUNTER != counter_src0) {
		val0 = vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_PERF_CNT_0_VALUE);
		vr_gp_job_set_perf_counter_value0(job, val0);

#if defined(CONFIG_VR400_PROFILING)
		_vr_osk_profiling_report_hw_counter(COUNTER_VP_0_C0, val0);
#endif

	}

	if (VR_HW_CORE_NO_COUNTER != counter_src1) {
		val1 = vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_PERF_CNT_1_VALUE);
		vr_gp_job_set_perf_counter_value1(job, val1);

#if defined(CONFIG_VR400_PROFILING)
		_vr_osk_profiling_report_hw_counter(COUNTER_VP_0_C1, val1);
#endif
	}
}
