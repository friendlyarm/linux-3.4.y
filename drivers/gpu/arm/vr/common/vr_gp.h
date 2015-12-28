/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_GP_H__
#define __VR_GP_H__

#include "vr_osk.h"
#include "vr_gp_job.h"
#include "vr_hw_core.h"
#include "regs/vr_gp_regs.h"

struct vr_group;

/**
 * Definition of the GP core struct
 * Used to track a GP core in the system.
 */
struct vr_gp_core {
	struct vr_hw_core  hw_core;           /**< Common for all HW cores */
	_vr_osk_irq_t     *irq;               /**< IRQ handler */
};

_vr_osk_errcode_t vr_gp_initialize(void);
void vr_gp_terminate(void);

struct vr_gp_core *vr_gp_create(const _vr_osk_resource_t * resource, struct vr_group *group);
void vr_gp_delete(struct vr_gp_core *core);

void vr_gp_stop_bus(struct vr_gp_core *core);
_vr_osk_errcode_t vr_gp_stop_bus_wait(struct vr_gp_core *core);
void vr_gp_reset_async(struct vr_gp_core *core);
_vr_osk_errcode_t vr_gp_reset_wait(struct vr_gp_core *core);
void vr_gp_hard_reset(struct vr_gp_core *core);
_vr_osk_errcode_t vr_gp_reset(struct vr_gp_core *core);

void vr_gp_job_start(struct vr_gp_core *core, struct vr_gp_job *job);
void vr_gp_resume_with_new_heap(struct vr_gp_core *core, u32 start_addr, u32 end_addr);

u32 vr_gp_core_get_version(struct vr_gp_core *core);

struct vr_gp_core *vr_gp_get_global_gp_core(void);

u32 vr_gp_dump_state(struct vr_gp_core *core, char *buf, u32 size);

void vr_gp_update_performance_counters(struct vr_gp_core *core, struct vr_gp_job *job, vr_bool suspend);

/*** Accessor functions ***/
VR_STATIC_INLINE const char *vr_gp_get_hw_core_desc(struct vr_gp_core *core)
{
	return core->hw_core.description;
}

/*** Register reading/writing functions ***/
VR_STATIC_INLINE u32 vr_gp_get_int_stat(struct vr_gp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_STAT);
}

VR_STATIC_INLINE void vr_gp_mask_all_interrupts(struct vr_gp_core *core)
{
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK, VRGP2_REG_VAL_IRQ_MASK_NONE);
}

VR_STATIC_INLINE u32 vr_gp_read_rawstat(struct vr_gp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_RAWSTAT) & VRGP2_REG_VAL_IRQ_MASK_USED;
}

VR_STATIC_INLINE u32 vr_gp_read_core_status(struct vr_gp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_STATUS);
}

VR_STATIC_INLINE void vr_gp_enable_interrupts(struct vr_gp_core *core, u32 irq_exceptions)
{
	/* Enable all interrupts, except those specified in irq_exceptions */
	vr_hw_core_register_write(&core->hw_core, VRGP2_REG_ADDR_MGMT_INT_MASK,
	                            VRGP2_REG_VAL_IRQ_MASK_USED & ~irq_exceptions);
}

VR_STATIC_INLINE u32 vr_gp_read_plbu_alloc_start_addr(struct vr_gp_core *core)
{
	return vr_hw_core_register_read(&core->hw_core, VRGP2_REG_ADDR_MGMT_PLBU_ALLOC_START_ADDR);
}

#endif /* __VR_GP_H__ */
