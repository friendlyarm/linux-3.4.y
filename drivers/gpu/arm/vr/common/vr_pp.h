/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_PP_H__
#define __VR_PP_H__

#include "vr_osk.h"
#include "vr_pp_job.h"
#include "vr_hw_core.h"
#include "vr_dma.h"

struct vr_group;

#define VR_MAX_NUMBER_OF_PP_CORES        9

/**
 * Definition of the PP core struct
 * Used to track a PP core in the system.
 */
struct vr_pp_core {
	struct vr_hw_core  hw_core;           /**< Common for all HW cores */
	_vr_osk_irq_t     *irq;               /**< IRQ handler */
	u32                  core_id;           /**< Unique core ID */
	u32                  bcast_id;          /**< The "flag" value used by the Vr-450 broadcast and DLBU unit */
};

_vr_osk_errcode_t vr_pp_initialize(void);
void vr_pp_terminate(void);

struct vr_pp_core *vr_pp_create(const _vr_osk_resource_t * resource, struct vr_group *group, vr_bool is_virtual, u32 bcast_id);
void vr_pp_delete(struct vr_pp_core *core);

void vr_pp_stop_bus(struct vr_pp_core *core);
_vr_osk_errcode_t vr_pp_stop_bus_wait(struct vr_pp_core *core);
void vr_pp_reset_async(struct vr_pp_core *core);
_vr_osk_errcode_t vr_pp_reset_wait(struct vr_pp_core *core);
_vr_osk_errcode_t vr_pp_reset(struct vr_pp_core *core);
_vr_osk_errcode_t vr_pp_hard_reset(struct vr_pp_core *core);

void vr_pp_job_start(struct vr_pp_core *core, struct vr_pp_job *job, u32 sub_job, vr_bool restart_virtual);

/**
 * @brief Add commands to DMA command buffer to start PP job on core.
 */
void vr_pp_job_dma_cmd_prepare(struct vr_pp_core *core, struct vr_pp_job *job, u32 sub_job,
                                 vr_bool restart_virtual, vr_dma_cmd_buf *buf);

u32 vr_pp_core_get_version(struct vr_pp_core *core);

VR_STATIC_INLINE u32 vr_pp_core_get_id(struct vr_pp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);
	return core->core_id;
}

VR_STATIC_INLINE u32 vr_pp_core_get_bcast_id(struct vr_pp_core *core)
{
	VR_DEBUG_ASSERT_POINTER(core);
	return core->bcast_id;
}

struct vr_pp_core* vr_pp_get_global_pp_core(u32 index);
u32 vr_pp_get_glob_num_pp_cores(void);

/* Debug */
u32 vr_pp_dump_state(struct vr_pp_core *core, char *buf, u32 size);

/**
 * Put instrumented HW counters from the core(s) to the job object (if enabled)
 *
 * parent and child is always the same, except for virtual jobs on Vr-450.
 * In this case, the counters will be enabled on the virtual core (parent),
 * but values need to be read from the child cores.
 *
 * @param parent The core used to see if the counters was enabled
 * @param child The core to actually read the values from
 * @job Job object to update with counter values (if enabled)
 * @subjob Which subjob the counters are applicable for (core ID for virtual jobs)
 */
void vr_pp_update_performance_counters(struct vr_pp_core *parent, struct vr_pp_core *child, struct vr_pp_job *job, u32 subjob);

VR_STATIC_INLINE const char *vr_pp_get_hw_core_desc(struct vr_pp_core *core)
{
	return core->hw_core.description;
}

/*** Register reading/writing functions ***/
VR_STATIC_INLINE u32 vr_pp_get_int_stat(struct vr_pp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_STATUS);
}

VR_STATIC_INLINE u32 vr_pp_read_rawstat(struct vr_pp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_INT_RAWSTAT) & VR200_REG_VAL_IRQ_MASK_USED;
}

VR_STATIC_INLINE u32 vr_pp_read_status(struct vr_pp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VR200_REG_ADDR_MGMT_STATUS);
}

VR_STATIC_INLINE void vr_pp_mask_all_interrupts(struct vr_pp_core *core)
{
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, VR200_REG_VAL_IRQ_MASK_NONE);
}

VR_STATIC_INLINE void vr_pp_clear_hang_interrupt(struct vr_pp_core *core)
{
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_CLEAR, VR200_REG_VAL_IRQ_HANG);
}

VR_STATIC_INLINE void vr_pp_enable_interrupts(struct vr_pp_core *core)
{
	vr_hw_core_register_write(&core->hw_core, VR200_REG_ADDR_MGMT_INT_MASK, VR200_REG_VAL_IRQ_MASK_USED);
}

VR_STATIC_INLINE void vr_pp_write_addr_stack(struct vr_pp_core *core, struct vr_pp_job *job)
{
	u32 addr = vr_pp_job_get_addr_stack(job, core->core_id);
	vr_hw_core_register_write_relaxed(&core->hw_core, VR200_REG_ADDR_STACK, addr);
}

#endif /* __VR_PP_H__ */
