/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_pp_job.h"
#include "vr_pp.h"
#include "vr_hw_core.h"
#include "vr_group.h"
#include "regs/vr_200_regs.h"
#include "vr_kernel_common.h"
#include "vr_kernel_core.h"
#include "vr_dma.h"
#if defined(CONFIG_VR400_PROFILING)
#include "vr_osk_profiling.h"
#endif

/* Number of frame registers on Vr-200 */
#define VR_PP_VR200_NUM_FRAME_REGISTERS ((0x04C/4)+1)
/* Number of frame registers on Vr-300 and later */
#define VR_PP_VR400_NUM_FRAME_REGISTERS ((0x058/4)+1)

static struct vr_pp_core* vr_global_pp_cores[VR_MAX_NUMBER_OF_PP_CORES] = { NULL };
static u32 vr_global_num_pp_cores = 0;

/* Interrupt handlers */
static void vr_pp_irq_probe_trigger(void *data);
static _vr_osk_errcode_t vr_pp_irq_probe_ack(void *data);

struct vr_pp_core *vr_pp_create(const _vr_osk_resource_t *resource, struct vr_group *group, vr_bool is_virtual, u32 bcast_id)
{
	struct vr_pp_core* core = NULL;

	VR_DEBUG_PRINT(2, ("Vr PP: Creating Vr PP core: %s\n", resource->description));
	VR_DEBUG_PRINT(2, ("Vr PP: Base address of PP core: 0x%x\n", resource->base));

	if (vr_global_num_pp_cores >= VR_MAX_NUMBER_OF_PP_CORES) {
		VR_PRINT_ERROR(("Vr PP: Too many PP core objects created\n"));
		return NULL;
	}

	core = _vr_osk_malloc(sizeof(struct vr_pp_core));
	if (NULL != core) {
		core->core_id = vr_global_num_pp_cores;
		core->bcast_id = bcast_id;

		if (_VR_OSK_ERR_OK == vr_hw_core_create(&core->hw_core, resource, VR200_REG_SIZEOF_REGISTER_BANK)) {
			_vr_osk_errcode_t ret;

			if (!is_virtual) {
				ret = vr_pp_reset(core);
			} else {
				ret = _VR_OSK_ERR_OK;
			}

			if (_VR_OSK_ERR_OK == ret) {
				ret = vr_group_add_pp_core(group, core);
				if (_VR_OSK_ERR_OK == ret) {
					/* Setup IRQ handlers (which will do IRQ probing if needed) */
					VR_DEBUG_ASSERT(!is_virtual || -1 != resource->irq);

					core->irq = _vr_osk_irq_init(resource->irq,
					                               vr_group_upper_half_pp,
					                               group,
					                               vr_pp_irq_probe_trigger,
					                               vr_pp_irq_probe_ack,
					                               core,
					                               resource->description);
					if (NULL != core->irq) {
						vr_global_pp_cores[vr_global_num_pp_cores] = core;
						vr_global_num_pp_cores++;

						return core;
					} else {
						VR_PRINT_ERROR(("Vr PP: Failed to setup interrupt handlers for PP core %s\n", core->hw_core.description));
					}
					vr_group_remove_pp_core(group);
				} else {
					VR_PRINT_ERROR(("Vr PP: Failed to add core %s to group\n", core->hw_core.description));
				}
			}
			vr_hw_core_delete(&core->hw_core);
		}

		_vr_osk_free(core);
	} else {
		VR_PRINT_ERROR(("Vr PP: Failed to allocate memory for PP core\n"));
	}

	return NULL;
}

void vr_pp_delete(struct vr_pp_core *core)
{
	u32 i;

	VR_DEBUG_ASSERT_POINTER(core);

	_vr_osk_irq_term(core->irq);
	vr_hw_core_delete(&core->hw_core);

	/* Remove core from global list */
	for (i = 0; i < vr_global_num_pp_cores; i++) {
		if (vr_global_pp_cores[i] == core) {
			vr_global_pp_cores[i] = NULL;
			vr_global_num_pp_cores--;

			if (i != vr_global_num_pp_cores) {
				/* We removed a PP core from the middle of the array -- move the last
				 * PP core to the current position to close the gap */
				vr_global_pp_cores[i] = vr_global_pp_cores[vr_global_num_pp_cores];
				vr_global_pp_cores[vr_global_num_pp_cores] = NULL;
			}

			break;
		}
	}

	_vr_osk_free(core);
}

void vr_pp_stop_bus(struct vr_pp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);
	/* Will only send the stop bus command, and not wait for it to complete */
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_CTRL_MGMT, VR200_REG_VAL_CTRL_MGMT_STOP_BUS);
}

_vr_osk_errcode_t vr_pp_stop_bus_wait(struct vr_pp_core *core)
{
	int i;

	VR_DEBUG_ASSERT_POINTER(core);

	/* Send the stop bus command. */
	vr_pp_stop_bus(core);

	/* Wait for bus to be stopped */
	for (i = 0; i < VR_REG_POLL_COUNT_FAST; i++) {
		if (vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_STATUS) & VR200_REG_VAL_STATUS_BUS_STOPPED)
			break;
	}

	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_PRINT_ERROR(("Vr PP: Failed to stop bus on %s. Status: 0x%08x\n", core->hw_core.description, vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_STATUS)));
		return _VR_OSK_ERR_FAULT;
	}
	return _VR_OSK_ERR_OK;
}

/* Frame register reset values.
 * Taken from the Vr400 TRM, 3.6. Pixel processor control register summary */
static const u32 vr_frame_registers_reset_values[_VR_PP_MAX_FRAME_REGISTERS] = {
	0x0, /* Renderer List Address Register */
	0x0, /* Renderer State Word Base Address Register */
	0x0, /* Renderer Vertex Base Register */
	0x2, /* Feature Enable Register */
	0x0, /* Z Clear Value Register */
	0x0, /* Stencil Clear Value Register */
	0x0, /* ABGR Clear Value 0 Register */
	0x0, /* ABGR Clear Value 1 Register */
	0x0, /* ABGR Clear Value 2 Register */
	0x0, /* ABGR Clear Value 3 Register */
	0x0, /* Bounding Box Left Right Register */
	0x0, /* Bounding Box Bottom Register */
	0x0, /* FS Stack Address Register */
	0x0, /* FS Stack Size and Initial Value Register */
	0x0, /* Reserved */
	0x0, /* Reserved */
	0x0, /* Origin Offset X Register */
	0x0, /* Origin Offset Y Register */
	0x75, /* Subpixel Specifier Register */
	0x0, /* Tiebreak mode Register */
	0x0, /* Polygon List Format Register */
	0x0, /* Scaling Register */
	0x0 /* Tilebuffer configuration Register */
};

/* WBx register reset values */
static const u32 vr_wb_registers_reset_values[_VR_PP_MAX_WB_REGISTERS] = {
	0x0, /* WBx Source Select Register */
	0x0, /* WBx Target Address Register */
	0x0, /* WBx Target Pixel Format Register */
	0x0, /* WBx Target AA Format Register */
	0x0, /* WBx Target Layout */
	0x0, /* WBx Target Scanline Length */
	0x0, /* WBx Target Flags Register */
	0x0, /* WBx MRT Enable Register */
	0x0, /* WBx MRT Offset Register */
	0x0, /* WBx Global Test Enable Register */
	0x0, /* WBx Global Test Reference Value Register */
	0x0  /* WBx Global Test Compare Function Register */
};

/* Performance Counter 0 Enable Register reset value */
static const u32 vr_perf_cnt_enable_reset_value = 0;

_vr_osk_errcode_t vr_pp_hard_reset(struct vr_pp_core *core)
{
	/* Bus must be stopped before calling this function */
	const u32 reset_invalid_value = 0xC0FFE000;
	const u32 reset_check_value = 0xC01A0000;
	int i;

	VR_DEBUG_ASSERT_POINTER(core);
	VR_DEBUG_PRINT(2, ("Vr PP: Hard reset of core %s\n", core->hw_core.description));

	/* Set register to a bogus value. The register will be used to detect when reset is complete */
	vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_MGMT_WRITE_BOUNDARY_LOW, reset_invalid_value);
	vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, VR200_REG_VAL_IRQ_MASK_NONE);

	/* Force core to reset */
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_CTRL_MGMT, VR200_REG_VAL_CTRL_MGMT_FORCE_RESET);

	/* Wait for reset to be complete */
	for (i = 0; i < VR_REG_POLL_COUNT_FAST; i++) {
		vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_WRITE_BOUNDARY_LOW, reset_check_value);
		if (reset_check_value == vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_WRITE_BOUNDARY_LOW)) {
			break;
		}
	}

	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_PRINT_ERROR(("Vr PP: The hard reset loop didn't work, unable to recover\n"));
	}

	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_WRITE_BOUNDARY_LOW, 0x00000000); /* set it back to the default */
	/* Re-enable interrupts */
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_CLEAR, VR200_REG_VAL_IRQ_MASK_ALL);
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, VR200_REG_VAL_IRQ_MASK_USED);

	return _VR_OSK_ERR_OK;
}

void vr_pp_reset_async(struct vr_pp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);

	VR_DEBUG_PRINT(4, ("Vr PP: Reset of core %s\n", core->hw_core.description));

	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, 0); /* disable the IRQs */
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_RAWSTAT, VR200_REG_VAL_IRQ_MASK_ALL);
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_CTRL_MGMT, VR400PP_REG_VAL_CTRL_MGMT_SOFT_RESET);
}

_vr_osk_errcode_t vr_pp_reset_wait(struct vr_pp_core *core)
{
	int i;
	u32 rawstat = 0;

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; i++) {
		if (!(vr_pp_read_status(core) & VR200_REG_VAL_STATUS_RENDERING_ACTIVE)) {
			rawstat = vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_RAWSTAT);
			if (rawstat == VR400PP_REG_VAL_IRQ_RESET_COMPLETED) {
				break;
			}
		}
	}

	if (i == VR_REG_POLL_COUNT_FAST) {
		VR_PRINT_ERROR(("Vr PP: Failed to reset core %s, rawstat: 0x%08x\n",
		                  core->hw_core.description, rawstat));
		return _VR_OSK_ERR_FAULT;
	}

	/* Re-enable interrupts */
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_CLEAR, VR200_REG_VAL_IRQ_MASK_ALL);
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, VR200_REG_VAL_IRQ_MASK_USED);

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t vr_pp_reset(struct vr_pp_core *core)
{
	vr_pp_reset_async(core);
	return vr_pp_reset_wait(core);
}

void vr_pp_job_dma_cmd_prepare(struct vr_pp_core *core, struct vr_pp_job *job, u32 sub_job,
                                 vr_bool restart_virtual, vr_dma_cmd_buf *buf)
{
	u32 relative_address;
	u32 start_index;
	u32 nr_of_regs;
	u32 *frame_registers = vr_pp_job_get_frame_registers(job);
	u32 *wb0_registers = vr_pp_job_get_wb0_registers(job);
	u32 *wb1_registers = vr_pp_job_get_wb1_registers(job);
	u32 *wb2_registers = vr_pp_job_get_wb2_registers(job);
	u32 counter_src0 = vr_pp_job_get_perf_counter_src0(job, sub_job);
	u32 counter_src1 = vr_pp_job_get_perf_counter_src1(job, sub_job);

	VR_DEBUG_ASSERT_POINTER(core);

	/* Write frame registers */

	/*
	 * There are two frame registers which are different for each sub job:
	 * 1. The Renderer List Address Register (VR200_REG_ADDR_FRAME)
	 * 2. The FS Stack Address Register (VR200_REG_ADDR_STACK)
	 */
	vr_dma_write_conditional(buf, &core->hw_core, VR200_REG_ADDR_FRAME, vr_pp_job_get_addr_frame(job, sub_job), vr_frame_registers_reset_values[VR200_REG_ADDR_FRAME / sizeof(u32)]);

	/* For virtual jobs, the stack address shouldn't be broadcast but written individually */
	if (!vr_pp_job_is_virtual(job) || restart_virtual) {
		vr_dma_write_conditional(buf, &core->hw_core, VR200_REG_ADDR_STACK, vr_pp_job_get_addr_stack(job, sub_job), vr_frame_registers_reset_values[VR200_REG_ADDR_STACK / sizeof(u32)]);
	}

	/* Write registers between VR200_REG_ADDR_FRAME and VR200_REG_ADDR_STACK */
	relative_address = VR200_REG_ADDR_RSW;
	start_index = VR200_REG_ADDR_RSW / sizeof(u32);
	nr_of_regs = (VR200_REG_ADDR_STACK - VR200_REG_ADDR_RSW) / sizeof(u32);

	vr_dma_write_array_conditional(buf, &core->hw_core,
	                                 relative_address, &frame_registers[start_index],
	                                 nr_of_regs, &vr_frame_registers_reset_values[start_index]);

	/* VR200_REG_ADDR_STACK_SIZE */
	relative_address = VR200_REG_ADDR_STACK_SIZE;
	start_index = VR200_REG_ADDR_STACK_SIZE / sizeof(u32);

	vr_dma_write_conditional(buf, &core->hw_core,
	                           relative_address, frame_registers[start_index],
	                           vr_frame_registers_reset_values[start_index]);

	/* Skip 2 reserved registers */

	/* Write remaining registers */
	relative_address = VR200_REG_ADDR_ORIGIN_OFFSET_X;
	start_index = VR200_REG_ADDR_ORIGIN_OFFSET_X / sizeof(u32);
	nr_of_regs = VR_PP_VR400_NUM_FRAME_REGISTERS - VR200_REG_ADDR_ORIGIN_OFFSET_X / sizeof(u32);

	vr_dma_write_array_conditional(buf, &core->hw_core,
	                                 relative_address, &frame_registers[start_index],
	                                 nr_of_regs, &vr_frame_registers_reset_values[start_index]);

	/* Write WBx registers */
	if (wb0_registers[0]) { /* M200_WB0_REG_SOURCE_SELECT register */
		vr_dma_write_array_conditional(buf, &core->hw_core, VR200_REG_ADDR_WB0, wb0_registers, _VR_PP_MAX_WB_REGISTERS, vr_wb_registers_reset_values);
	}

	if (wb1_registers[0]) { /* M200_WB1_REG_SOURCE_SELECT register */
		vr_dma_write_array_conditional(buf, &core->hw_core, VR200_REG_ADDR_WB1, wb1_registers, _VR_PP_MAX_WB_REGISTERS, vr_wb_registers_reset_values);
	}

	if (wb2_registers[0]) { /* M200_WB2_REG_SOURCE_SELECT register */
		vr_dma_write_array_conditional(buf, &core->hw_core, VR200_REG_ADDR_WB2, wb2_registers, _VR_PP_MAX_WB_REGISTERS, vr_wb_registers_reset_values);
	}

	if (VR_HW_CORE_NO_COUNTER != counter_src0) {
		vr_dma_write(buf, &core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_SRC, counter_src0);
		vr_dma_write_conditional(buf, &core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_ENABLE, VR200_REG_VAL_PERF_CNT_ENABLE, vr_perf_cnt_enable_reset_value);
	}
	if (VR_HW_CORE_NO_COUNTER != counter_src1) {
		vr_dma_write(buf, &core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_SRC, counter_src1);
		vr_dma_write_conditional(buf, &core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_ENABLE, VR200_REG_VAL_PERF_CNT_ENABLE, vr_perf_cnt_enable_reset_value);
	}

	/* This is the command that starts the core. */
	vr_dma_write(buf, &core->hw_core, VR200_REG_ADDR_MGMT_CTRL_MGMT, VR200_REG_VAL_CTRL_MGMT_START_RENDERING);
}

void vr_pp_job_start(struct vr_pp_core *core, struct vr_pp_job *job, u32 sub_job, vr_bool restart_virtual)
{
	u32 relative_address;
	u32 start_index;
	u32 nr_of_regs;
	u32 *frame_registers = vr_pp_job_get_frame_registers(job);
	u32 *wb0_registers = vr_pp_job_get_wb0_registers(job);
	u32 *wb1_registers = vr_pp_job_get_wb1_registers(job);
	u32 *wb2_registers = vr_pp_job_get_wb2_registers(job);
	u32 counter_src0 = vr_pp_job_get_perf_counter_src0(job, sub_job);
	u32 counter_src1 = vr_pp_job_get_perf_counter_src1(job, sub_job);

	VR_DEBUG_ASSERT_POINTER(core);

	/* Write frame registers */

	/*
	 * There are two frame registers which are different for each sub job:
	 * 1. The Renderer List Address Register (VR200_REG_ADDR_FRAME)
	 * 2. The FS Stack Address Register (VR200_REG_ADDR_STACK)
	 */
	vr_hw_core_register_write_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_FRAME, vr_pp_job_get_addr_frame(job, sub_job), vr_frame_registers_reset_values[VR200_REG_ADDR_FRAME / sizeof(u32)]);

	/* For virtual jobs, the stack address shouldn't be broadcast but written individually */
	if (!vr_pp_job_is_virtual(job) || restart_virtual) {
		vr_hw_core_register_write_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_STACK, vr_pp_job_get_addr_stack(job, sub_job), vr_frame_registers_reset_values[VR200_REG_ADDR_STACK / sizeof(u32)]);
	}

	/* Write registers between VR200_REG_ADDR_FRAME and VR200_REG_ADDR_STACK */
	relative_address = VR200_REG_ADDR_RSW;
	start_index = VR200_REG_ADDR_RSW / sizeof(u32);
	nr_of_regs = (VR200_REG_ADDR_STACK - VR200_REG_ADDR_RSW) / sizeof(u32);

	vr_hw_core_register_write_array_relaxed_conditional(&core->hw_core,
	        relative_address, &frame_registers[start_index],
	        nr_of_regs, &vr_frame_registers_reset_values[start_index]);

	/* VR200_REG_ADDR_STACK_SIZE */
	relative_address = VR200_REG_ADDR_STACK_SIZE;
	start_index = VR200_REG_ADDR_STACK_SIZE / sizeof(u32);

	vr_hw_core_register_write_relaxed_conditional(&core->hw_core,
	        relative_address, frame_registers[start_index],
	        vr_frame_registers_reset_values[start_index]);

	/* Skip 2 reserved registers */

	/* Write remaining registers */
	relative_address = VR200_REG_ADDR_ORIGIN_OFFSET_X;
	start_index = VR200_REG_ADDR_ORIGIN_OFFSET_X / sizeof(u32);
	nr_of_regs = VR_PP_VR400_NUM_FRAME_REGISTERS - VR200_REG_ADDR_ORIGIN_OFFSET_X / sizeof(u32);

	vr_hw_core_register_write_array_relaxed_conditional(&core->hw_core,
	        relative_address, &frame_registers[start_index],
	        nr_of_regs, &vr_frame_registers_reset_values[start_index]);

	/* Write WBx registers */
	if (wb0_registers[0]) { /* M200_WB0_REG_SOURCE_SELECT register */
		vr_hw_core_register_write_array_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_WB0, wb0_registers, _VR_PP_MAX_WB_REGISTERS, vr_wb_registers_reset_values);
	}

	if (wb1_registers[0]) { /* M200_WB1_REG_SOURCE_SELECT register */
		vr_hw_core_register_write_array_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_WB1, wb1_registers, _VR_PP_MAX_WB_REGISTERS, vr_wb_registers_reset_values);
	}

	if (wb2_registers[0]) { /* M200_WB2_REG_SOURCE_SELECT register */
		vr_hw_core_register_write_array_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_WB2, wb2_registers, _VR_PP_MAX_WB_REGISTERS, vr_wb_registers_reset_values);
	}

	if (VR_HW_CORE_NO_COUNTER != counter_src0) {
		vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_SRC, counter_src0);
		vr_hw_core_register_write_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_ENABLE, VR200_REG_VAL_PERF_CNT_ENABLE, vr_perf_cnt_enable_reset_value);
	}
	if (VR_HW_CORE_NO_COUNTER != counter_src1) {
		vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_SRC, counter_src1);
		vr_hw_core_register_write_relaxed_conditional(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_ENABLE, VR200_REG_VAL_PERF_CNT_ENABLE, vr_perf_cnt_enable_reset_value);
	}

#ifdef CONFIG_VR400_HEATMAPS_ENABLED
	if(job->uargs.perf_counter_flag & _VR_PERFORMANCE_COUNTER_FLAG_HEATMAP_ENABLE) {
		vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_MGMT_PERFMON_CONTR, ((job->uargs.tilesx & 0x3FF) << 16) | 1);
		vr_hw_core_register_write_relaxed(&core->hw_core,  VR200_REG_ADDR_MGMT_PERFMON_BASE, job->uargs.heatmap_mem & 0xFFFFFFF8);
	}
#endif /* CONFIG_VR400_HEATMAPS_ENABLED */

	VR_DEBUG_PRINT(3, ("Vr PP: Starting job 0x%08X part %u/%u on PP core %s\n", job, sub_job + 1, vr_pp_job_get_sub_job_count(job), core->hw_core.description));

	/* Adding barrier to make sure all rester writes are finished */
	_vr_osk_write_mem_barrier();

	/* This is the command that starts the core. */
	vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_MGMT_CTRL_MGMT, VR200_REG_VAL_CTRL_MGMT_START_RENDERING);

	/* Adding barrier to make sure previous rester writes is finished */
	_vr_osk_write_mem_barrier();
}

u32 vr_pp_core_get_version(struct vr_pp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);
	return vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_VERSION);
}

struct vr_pp_core* vr_pp_get_global_pp_core(u32 index)
{
	if (vr_global_num_pp_cores > index) {
		return vr_global_pp_cores[index];
	}

	return NULL;
}

u32 vr_pp_get_glob_num_pp_cores(void)
{
	return vr_global_num_pp_cores;
}

/* ------------- interrupt handling below ------------------ */
static void vr_pp_irq_probe_trigger(void *data)
{
	struct vr_pp_core *core = (struct vr_pp_core *)data;
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, VR200_REG_VAL_IRQ_MASK_USED);
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_RAWSTAT, VR200_REG_VAL_IRQ_FORCE_HANG);
	_vr_osk_mem_barrier();
}

static _vr_osk_errcode_t vr_pp_irq_probe_ack(void *data)
{
	struct vr_pp_core *core = (struct vr_pp_core *)data;
	u32 irq_readout;

	irq_readout = vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_STATUS);
	if (VR200_REG_VAL_IRQ_FORCE_HANG & irq_readout) {
		vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_CLEAR, VR200_REG_VAL_IRQ_FORCE_HANG);
		_vr_osk_mem_barrier();
		return _VR_OSK_ERR_OK;
	}

	return _VR_OSK_ERR_FAULT;
}


#if 0
static void vr_pp_print_registers(struct vr_pp_core *core)
{
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_VERSION = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_VERSION)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_CURRENT_REND_LIST_ADDR = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_CURRENT_REND_LIST_ADDR)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_STATUS = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_STATUS)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_INT_RAWSTAT = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_RAWSTAT)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_INT_MASK = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_INT_STATUS = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_STATUS)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_BUS_ERROR_STATUS = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_BUS_ERROR_STATUS)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_PERF_CNT_0_ENABLE = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_ENABLE)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_PERF_CNT_0_SRC = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_SRC)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_PERF_CNT_0_VALUE = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_VALUE)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_PERF_CNT_1_ENABLE = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_ENABLE)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_PERF_CNT_1_SRC = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_SRC)));
	VR_DEBUG_PRINT(2, ("Vr PP: Register VR200_REG_ADDR_MGMT_PERF_CNT_1_VALUE = 0x%08X\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_VALUE)));
}
#endif

#if 0
void vr_pp_print_state(struct vr_pp_core *core)
{
	VR_DEBUG_PRINT(2, ("Vr PP: State: 0x%08x\n", vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_STATUS) ));
}
#endif

void vr_pp_update_performance_counters(struct vr_pp_core *parent, struct vr_pp_core *child, struct vr_pp_job *job, u32 subjob)
{
	u32 val0 = 0;
	u32 val1 = 0;
	u32 counter_src0 = vr_pp_job_get_perf_counter_src0(job, subjob);
	u32 counter_src1 = vr_pp_job_get_perf_counter_src1(job, subjob);
#if defined(CONFIG_VR400_PROFILING)
	int counter_index = COUNTER_FP_0_C0 + (2 * child->core_id);
#endif

	if (VR_HW_CORE_NO_COUNTER != counter_src0) {
		val0 = vr_hw_core_register_read(&child->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_0_VALUE);
		vr_pp_job_set_perf_counter_value0(job, subjob, val0);

#if defined(CONFIG_VR400_PROFILING)
		_vr_osk_profiling_report_hw_counter(counter_index, val0);
#endif
	}

	if (VR_HW_CORE_NO_COUNTER != counter_src1) {
		val1 = vr_hw_core_register_read(&child->hw_core, VR200_REG_ADDR_MGMT_PERF_CNT_1_VALUE);
		vr_pp_job_set_perf_counter_value1(job, subjob, val1);

#if defined(CONFIG_VR400_PROFILING)
		_vr_osk_profiling_report_hw_counter(counter_index + 1, val1);
#endif
	}
}

#if VR_STATE_TRACKING
u32 vr_pp_dump_state(struct vr_pp_core *core, char *buf, u32 size)
{
	int n = 0;

	n += _vr_osk_snprintf(buf + n, size - n, "\tPP #%d: %s\n", core->core_id, core->hw_core.description);

	return n;
}
#endif
