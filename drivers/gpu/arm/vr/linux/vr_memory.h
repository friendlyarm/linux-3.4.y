/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MEMORY_H__
#define __VR_MEMORY_H__

#include "vr_osk.h"
#include "vr_session.h"

#include <linux/list.h>
#include <linux/mm.h>

#include "vr_memory_types.h"
#include "vr_memory_os_alloc.h"

_vr_osk_errcode_t vr_memory_initialize(void);
void vr_memory_terminate(void);

/** @brief Allocate a page table page
 *
 * Allocate a page for use as a page directory or page table. The page is
 * mapped into kernel space.
 *
 * @return _VR_OSK_ERR_OK on success, otherwise an error code
 * @param table_page GPU pointer to the allocated page
 * @param mapping CPU pointer to the mapping of the allocated page
 */
VR_STATIC_INLINE _vr_osk_errcode_t vr_mmu_get_table_page(u32 *table_page, vr_io_address *mapping)
{
	return vr_mem_os_get_table_page(table_page, mapping);
}

/** @brief Release a page table page
 *
 * Release a page table page allocated through \a vr_mmu_get_table_page
 *
 * @param pa the GPU address of the page to release
 */
VR_STATIC_INLINE void vr_mmu_release_table_page(u32 phys, void *virt)
{
	vr_mem_os_release_table_page(phys, virt);
}

/** @brief mmap function
 *
 * mmap syscalls on the Vr device node will end up here.
 *
 * This function allocates Vr memory and maps it on CPU and Vr.
 */
int vr_mmap(struct file *filp, struct vm_area_struct *vma);

/** @brief Allocate and initialize a Vr memory descriptor
 *
 * @param session Pointer to the session allocating the descriptor
 * @param type Type of memory the descriptor will represent
 */
vr_mem_allocation *vr_mem_descriptor_create(struct vr_session_data *session, vr_mem_type type);

/** @brief Destroy a Vr memory descriptor
 *
 * This function will only free the descriptor itself, and not the memory it
 * represents.
 *
 * @param descriptor Pointer to the descriptor to destroy
 */
void vr_mem_descriptor_destroy(vr_mem_allocation *descriptor);

/** @brief Start a new memory session
 *
 * Called when a process opens the Vr device node.
 *
 * @param session Pointer to session to initialize
 */
_vr_osk_errcode_t vr_memory_session_begin(struct vr_session_data *session);

/** @brief Close a memory session
 *
 * Called when a process closes the Vr device node.
 *
 * Memory allocated by the session will be freed
 *
 * @param session Pointer to the session to terminate
 */
void vr_memory_session_end(struct vr_session_data *session);

/** @brief Prepare Vr page tables for mapping
 *
 * This function will prepare the Vr page tables for mapping the memory
 * described by \a descriptor.
 *
 * Page tables will be reference counted and allocated, if not yet present.
 *
 * @param descriptor Pointer to the memory descriptor to the mapping
 */
_vr_osk_errcode_t vr_mem_vr_map_prepare(vr_mem_allocation *descriptor);

/** @brief Free Vr page tables for mapping
 *
 * This function will unmap pages from Vr memory and free the page tables
 * that are now unused.
 *
 * The updated pages in the Vr L2 cache will be invalidated, and the MMU TLBs will be zapped if necessary.
 *
 * @param descriptor Pointer to the memory descriptor to unmap
 */
void vr_mem_vr_map_free(vr_mem_allocation *descriptor);

/** @brief Parse resource and prepare the OS memory allocator
 *
 * @param size Maximum size to allocate for Vr GPU.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t vr_memory_core_resource_os_memory(u32 size);

/** @brief Parse resource and prepare the dedicated memory allocator
 *
 * @param start Physical start address of dedicated Vr GPU memory.
 * @param size Size of dedicated Vr GPU memory.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t vr_memory_core_resource_dedicated_memory(u32 start, u32 size);


void vr_mem_ump_release(vr_mem_allocation *descriptor);
void vr_mem_external_release(vr_mem_allocation *descriptor);

#endif /* __VR_MEMORY_H__ */
