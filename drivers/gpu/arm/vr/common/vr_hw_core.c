/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_hw_core.h"
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_osk_vr.h"

_vr_osk_errcode_t vr_hw_core_create(struct vr_hw_core *core, const _vr_osk_resource_t *resource, u32 reg_size)
{
	core->phys_addr = resource->base;
	core->phys_offset = resource->base - _vr_osk_resource_base_address();
	core->description = resource->description;
	core->size = reg_size;

	VR_DEBUG_ASSERT(core->phys_offset < core->phys_addr);

	if (_VR_OSK_ERR_OK == _vr_osk_mem_reqregion(core->phys_addr, core->size, core->description)) {
		core->mapped_registers = _vr_osk_mem_mapioregion(core->phys_addr, core->size, core->description);
		if (NULL != core->mapped_registers) {
			return _VR_OSK_ERR_OK;
		} else {
			VR_PRINT_ERROR(("Failed to map memory region for core %s at phys_addr 0x%08X\n", core->description, core->phys_addr));
		}
		_vr_osk_mem_unreqregion(core->phys_addr, core->size);
	} else {
		VR_PRINT_ERROR(("Failed to request memory region for core %s at phys_addr 0x%08X\n", core->description, core->phys_addr));
	}

	return _VR_OSK_ERR_FAULT;
}

void vr_hw_core_delete(struct vr_hw_core *core)
{
	_vr_osk_mem_unmapioregion(core->phys_addr, core->size, core->mapped_registers);
	core->mapped_registers = NULL;
	_vr_osk_mem_unreqregion(core->phys_addr, core->size);
}
