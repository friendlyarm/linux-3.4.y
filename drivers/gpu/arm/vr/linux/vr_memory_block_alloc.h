/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2010, 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_BLOCK_ALLOCATOR_H__
#define __VR_BLOCK_ALLOCATOR_H__

#include "vr_session.h"
#include "vr_memory.h"

#include "vr_memory_types.h"

typedef struct vr_mem_allocator vr_mem_allocator;

vr_mem_allocator *vr_block_allocator_create(u32 base_address, u32 cpu_usage_adjust, u32 size);
void vr_mem_block_allocator_destroy(vr_mem_allocator *allocator);

vr_mem_allocation *vr_mem_block_alloc(u32 vr_addr, u32 size, struct vm_area_struct *vma, struct vr_session_data *session);
void vr_mem_block_release(vr_mem_allocation *descriptor);

u32 vr_mem_block_allocator_stat(void);

#endif /* __VR_BLOCK_ALLOCATOR_H__ */
