/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_mem_validation.h"
#include "vr_osk.h"
#include "vr_kernel_common.h"

#define VR_INVALID_MEM_ADDR 0xFFFFFFFF

typedef struct {
	u32 phys_base;        /**< Vr physical base of the memory, page aligned */
	u32 size;             /**< size in bytes of the memory, multiple of page size */
} _vr_mem_validation_t;

static _vr_mem_validation_t vr_mem_validator = { VR_INVALID_MEM_ADDR, VR_INVALID_MEM_ADDR };

_vr_osk_errcode_t vr_mem_validation_add_range(u32 start, u32 size)
{
	/* Check that no other MEM_VALIDATION resources exist */
	if (VR_INVALID_MEM_ADDR != vr_mem_validator.phys_base) {
		VR_PRINT_ERROR(("Failed to add frame buffer memory; another range is already specified\n"));
		return _VR_OSK_ERR_FAULT;
	}

	/* Check restrictions on page alignment */
	if ((0 != (start & (~_VR_OSK_CPU_PAGE_MASK))) ||
	    (0 != (size & (~_VR_OSK_CPU_PAGE_MASK)))) {
		VR_PRINT_ERROR(("Failed to add frame buffer memory; incorrect alignment\n"));
		return _VR_OSK_ERR_FAULT;
	}

	vr_mem_validator.phys_base = start;
	vr_mem_validator.size = size;
	VR_DEBUG_PRINT(2, ("Memory Validator installed for Vr physical address base=0x%08X, size=0x%08X\n",
	                     vr_mem_validator.phys_base, vr_mem_validator.size));

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t vr_mem_validation_check(u32 phys_addr, u32 size)
{
	if (phys_addr < (phys_addr + size)) { /* Don't allow overflow (or zero size) */
		if ((0 == ( phys_addr & (~_VR_OSK_CPU_PAGE_MASK))) &&
		    (0 == ( size & (~_VR_OSK_CPU_PAGE_MASK)))) {
			if ((phys_addr          >= vr_mem_validator.phys_base) &&
			    ((phys_addr + (size - 1)) >= vr_mem_validator.phys_base) &&
			    (phys_addr          <= (vr_mem_validator.phys_base + (vr_mem_validator.size - 1))) &&
			    ((phys_addr + (size - 1)) <= (vr_mem_validator.phys_base + (vr_mem_validator.size - 1))) ) {
				VR_DEBUG_PRINT(3, ("Accepted range 0x%08X + size 0x%08X (= 0x%08X)\n", phys_addr, size, (phys_addr + size - 1)));
				return _VR_OSK_ERR_OK;
			}
		}
	}

	VR_PRINT_ERROR(("VR PHYSICAL RANGE VALIDATION ERROR: The range supplied was: phys_base=0x%08X, size=0x%08X\n", phys_addr, size));

	return _VR_OSK_ERR_FAULT;
}
