/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_HW_CORE_H__
#define __VR_HW_CORE_H__

#include "vr_osk.h"
#include "vr_kernel_common.h"

/**
 * The common parts for all Vr HW cores (GP, PP, MMU, L2 and PMU)
 * This struct is embedded inside all core specific structs.
 */
struct vr_hw_core {
	u32 phys_addr;                    /**< Physical address of the registers */
	u32 phys_offset;                  /**< Offset from start of Vr to registers */
	u32 size;                         /**< Size of registers */
	vr_io_address mapped_registers; /**< Virtual mapping of the registers */
	const char* description;          /**< Name of unit (as specified in device configuration) */
};

#define VR_REG_POLL_COUNT_FAST 1000
#define VR_REG_POLL_COUNT_SLOW 1000000

_vr_osk_errcode_t vr_hw_core_create(struct vr_hw_core *core, const _vr_osk_resource_t *resource, u32 reg_size);
void vr_hw_core_delete(struct vr_hw_core *core);

VR_STATIC_INLINE u32 vr_hw_core_register_read(struct vr_hw_core *core, u32 relative_address)
{
	u32 read_val;
	read_val = _vr_osk_mem_ioread32(core->mapped_registers, relative_address);
	VR_DEBUG_PRINT(6, ("register_read for core %s, relative addr=0x%04X, val=0x%08X\n",
	                     core->description, relative_address, read_val));
	return read_val;
}

VR_STATIC_INLINE void vr_hw_core_register_write_relaxed(struct vr_hw_core *core, u32 relative_address, u32 new_val)
{
	VR_DEBUG_PRINT(6, ("register_write_relaxed for core %s, relative addr=0x%04X, val=0x%08X\n",
	                     core->description, relative_address, new_val));
	_vr_osk_mem_iowrite32_relaxed(core->mapped_registers, relative_address, new_val);
}

/* Conditionally write a register.
 * The register will only be written if the new value is different from the old_value.
 * If the new value is different, the old value will also be updated */
VR_STATIC_INLINE void vr_hw_core_register_write_relaxed_conditional(struct vr_hw_core *core, u32 relative_address, u32 new_val, const u32 old_val)
{
	VR_DEBUG_PRINT(6, ("register_write_relaxed for core %s, relative addr=0x%04X, val=0x%08X\n",
	                     core->description, relative_address, new_val));
	if(old_val != new_val) {
		_vr_osk_mem_iowrite32_relaxed(core->mapped_registers, relative_address, new_val);
	}
}


VR_STATIC_INLINE void vr_hw_core_register_write(struct vr_hw_core *core, u32 relative_address, u32 new_val)
{
	VR_DEBUG_PRINT(6, ("register_write for core %s, relative addr=0x%04X, val=0x%08X\n",
	                     core->description, relative_address, new_val));
	_vr_osk_mem_iowrite32(core->mapped_registers, relative_address, new_val);
}

VR_STATIC_INLINE void vr_hw_core_register_write_array_relaxed(struct vr_hw_core *core, u32 relative_address, u32 *write_array, u32 nr_of_regs)
{
	u32 i;
	VR_DEBUG_PRINT(6, ("register_write_array: for core %s, relative addr=0x%04X, nr of regs=%u\n",
	                     core->description,relative_address, nr_of_regs));

	/* Do not use burst writes against the registers */
	for (i = 0; i< nr_of_regs; i++) {
		vr_hw_core_register_write_relaxed(core, relative_address + i*4, write_array[i]);
	}
}

/* Conditionally write a set of registers.
 * The register will only be written if the new value is different from the old_value.
 * If the new value is different, the old value will also be updated */
VR_STATIC_INLINE void vr_hw_core_register_write_array_relaxed_conditional(struct vr_hw_core *core, u32 relative_address, u32 *write_array, u32 nr_of_regs, const u32* old_array)
{
	u32 i;
	VR_DEBUG_PRINT(6, ("register_write_array: for core %s, relative addr=0x%04X, nr of regs=%u\n",
	                     core->description,relative_address, nr_of_regs));

	/* Do not use burst writes against the registers */
	for (i = 0; i< nr_of_regs; i++) {
		if(old_array[i] != write_array[i]) {
			vr_hw_core_register_write_relaxed(core, relative_address + i*4, write_array[i]);
		}
	}
}

#endif /* __VR_HW_CORE_H__ */
