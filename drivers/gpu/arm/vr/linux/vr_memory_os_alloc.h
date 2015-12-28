/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MEMORY_OS_ALLOC_H__
#define __VR_MEMORY_OS_ALLOC_H__

#include "vr_osk.h"
#include "vr_session.h"

#include "vr_memory_types.h"

/* OS memory allocator */
/** @brief Allocate memory from OS
 *
 * This function will create a descriptor, allocate pages and map these on the CPU and Vr.
 *
 * @param vr_addr Vr virtual address to use for Vr mapping
 * @param size Size to allocate
 * @param vma Pointer to vma for CPU mapping
 * @param session Pointer to session doing the allocation
 */
vr_mem_allocation *vr_mem_os_alloc(u32 vr_addr, u32 size, struct vm_area_struct *vma, struct vr_session_data *session);

/** @brief Release Vr OS memory
 *
 * The session memory_lock must be held when calling this function.
 *
 * @param descriptor Pointer to the descriptor to release
 */
void vr_mem_os_release(vr_mem_allocation *descriptor);

_vr_osk_errcode_t vr_mem_os_get_table_page(u32 *phys, vr_io_address *mapping);

void vr_mem_os_release_table_page(u32 phys, void *virt);

_vr_osk_errcode_t vr_mem_os_init(void);
void vr_mem_os_term(void);
u32 vr_mem_os_stat(void);

#endif /* __VR_MEMORY_OS_ALLOC_H__ */
