/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/platform_device.h>

#include "vr_osk.h"
#include "vr_osk_vr.h"
#include "vr_kernel_linux.h"
#include "vr_scheduler.h"
#include "vr_kernel_descriptor_mapping.h"

#include "vr_memory.h"
#include "vr_memory_dma_buf.h"
#include "vr_memory_os_alloc.h"
#include "vr_memory_block_alloc.h"

/* session->memory_lock must be held when calling this function */
static void vr_mem_release(vr_mem_allocation *descriptor)
{
	VR_DEBUG_ASSERT_POINTER(descriptor);
	VR_DEBUG_ASSERT_LOCK_HELD(descriptor->session->memory_lock);

	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);

	switch (descriptor->type) {
	case VR_MEM_OS:
		vr_mem_os_release(descriptor);
		break;
	case VR_MEM_DMA_BUF:
#if defined(CONFIG_DMA_SHARED_BUFFER)
		vr_mem_dma_buf_release(descriptor);
#endif
		break;
	case VR_MEM_UMP:
#if defined(CONFIG_VR400_UMP)
		vr_mem_ump_release(descriptor);
#endif
		break;
	case VR_MEM_EXTERNAL:
		vr_mem_external_release(descriptor);
		break;
	case VR_MEM_BLOCK:
		vr_mem_block_release(descriptor);
		break;
	}
}

static void vr_mem_vma_open(struct vm_area_struct * vma)
{
	vr_mem_allocation *descriptor = (vr_mem_allocation*)vma->vm_private_data;
	VR_DEBUG_PRINT(4, ("Open called on vma %p\n", vma));

	descriptor->cpu_mapping.ref++;

	return;
}

static void vr_mem_vma_close(struct vm_area_struct *vma)
{
	vr_mem_allocation *descriptor;
	struct vr_session_data *session;
	vr_mem_virt_cpu_mapping *mapping;

	VR_DEBUG_PRINT(3, ("Close called on vma %p\n", vma));

	descriptor = (vr_mem_allocation*)vma->vm_private_data;
	BUG_ON(!descriptor);

	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);

	mapping = &descriptor->cpu_mapping;
	BUG_ON(0 == mapping->ref);

	mapping->ref--;
	if (0 != mapping->ref) {
		VR_DEBUG_PRINT(3, ("Ignoring this close, %d references still exists\n", mapping->ref));
		return;
	}

	session = descriptor->session;

	vr_descriptor_mapping_free(session->descriptor_mapping, descriptor->id);

	_vr_osk_mutex_wait(session->memory_lock);
	vr_mem_release(descriptor);
	_vr_osk_mutex_signal(session->memory_lock);

	vr_mem_descriptor_destroy(descriptor);
}

static int vr_kernel_memory_cpu_page_fault_handler(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	void __user * address;
	vr_mem_allocation *descriptor;

	address = vmf->virtual_address;
	descriptor = (vr_mem_allocation *)vma->vm_private_data;

	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);

	/*
	 * We always fail the call since all memory is pre-faulted when assigned to the process.
	 * Only the Vr cores can use page faults to extend buffers.
	*/

	VR_DEBUG_PRINT(1, ("Page-fault in Vr memory region caused by the CPU.\n"));
	VR_DEBUG_PRINT(1, ("Tried to access %p (process local virtual address) which is not currently mapped to any Vr memory.\n", (void*)address));

	VR_IGNORE(address);
	VR_IGNORE(descriptor);

	return VM_FAULT_SIGBUS;
}

struct vm_operations_struct vr_kernel_vm_ops = {
	.open = vr_mem_vma_open,
	.close = vr_mem_vma_close,
	.fault = vr_kernel_memory_cpu_page_fault_handler
};

/** @note munmap handler is done by vma close handler */
int vr_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct vr_session_data *session;
	vr_mem_allocation *descriptor;
	u32 size = vma->vm_end - vma->vm_start;
	u32 vr_addr = vma->vm_pgoff << PAGE_SHIFT;

	session = (struct vr_session_data *)filp->private_data;
	if (NULL == session) {
		VR_PRINT_ERROR(("mmap called without any session data available\n"));
		return -EFAULT;
	}

	VR_DEBUG_PRINT(4, ("MMap() handler: start=0x%08X, phys=0x%08X, size=0x%08X vma->flags 0x%08x\n",
	                     (unsigned int)vma->vm_start, (unsigned int)(vma->vm_pgoff << PAGE_SHIFT),
	                     (unsigned int)(vma->vm_end - vma->vm_start), vma->vm_flags));

	/* Set some bits which indicate that, the memory is IO memory, meaning
	 * that no paging is to be performed and the memory should not be
	 * included in crash dumps. And that the memory is reserved, meaning
	 * that it's present and can never be paged out (see also previous
	 * entry)
	 */
	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_DONTCOPY;
	vma->vm_flags |= VM_PFNMAP;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	vma->vm_flags |= VM_RESERVED;
#else
	vma->vm_flags |= VM_DONTDUMP;
	vma->vm_flags |= VM_DONTEXPAND;
#endif

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	vma->vm_ops = &vr_kernel_vm_ops; /* Operations used on any memory system */

	descriptor = vr_mem_block_alloc(vr_addr, size, vma, session);
	if (NULL == descriptor) {
		descriptor = vr_mem_os_alloc(vr_addr, size, vma, session);
		if (NULL == descriptor) {
			VR_DEBUG_PRINT(3, ("MMAP failed\n"));
			return -ENOMEM;
		}
	}

	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);

	vma->vm_private_data = (void*)descriptor;

	/* Put on descriptor map */
	if (_VR_OSK_ERR_OK != vr_descriptor_mapping_allocate_mapping(session->descriptor_mapping, descriptor, &descriptor->id)) {
		_vr_osk_mutex_wait(session->memory_lock);
		vr_mem_os_release(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);
		return -EFAULT;
	}

	return 0;
}


/* Prepare memory descriptor */
vr_mem_allocation *vr_mem_descriptor_create(struct vr_session_data *session, vr_mem_type type)
{
	vr_mem_allocation *descriptor;

	descriptor = (vr_mem_allocation*)kzalloc(sizeof(vr_mem_allocation), GFP_KERNEL);
	if (NULL == descriptor) {
		VR_DEBUG_PRINT(3,("vr_ukk_mem_mmap: descriptor was NULL\n"));
		return NULL;
	}

	VR_DEBUG_CODE(descriptor->magic = VR_MEM_ALLOCATION_VALID_MAGIC);

	descriptor->flags = 0;
	descriptor->type = type;
	descriptor->session = session;

	return descriptor;
}

void vr_mem_descriptor_destroy(vr_mem_allocation *descriptor)
{
	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);
	VR_DEBUG_CODE(descriptor->magic = VR_MEM_ALLOCATION_FREED_MAGIC);

	kfree(descriptor);
}

_vr_osk_errcode_t vr_mem_vr_map_prepare(vr_mem_allocation *descriptor)
{
	u32 size = descriptor->size;
	struct vr_session_data *session = descriptor->session;

	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);

	/* Map dma-buf into this session's page tables */

	if (descriptor->flags & VR_MEM_FLAG_VR_GUARD_PAGE) {
		size += VR_MMU_PAGE_SIZE;
	}

	return vr_mmu_pagedir_map(session->page_directory, descriptor->vr_mapping.addr, size);
}

void vr_mem_vr_map_free(vr_mem_allocation *descriptor)
{
	u32 size = descriptor->size;
	struct vr_session_data *session = descriptor->session;

	VR_DEBUG_ASSERT(VR_MEM_ALLOCATION_VALID_MAGIC == descriptor->magic);

	if (descriptor->flags & VR_MEM_FLAG_VR_GUARD_PAGE) {
		size += VR_MMU_PAGE_SIZE;
	}

	/* Umap and flush L2 */
	vr_mmu_pagedir_unmap(session->page_directory, descriptor->vr_mapping.addr, descriptor->size);

	vr_scheduler_zap_all_active(session);
}

u32 _vr_ukk_report_memory_usage(void)
{
	u32 sum = 0;

	sum += vr_mem_block_allocator_stat();
	sum += vr_mem_os_stat();

	return sum;
}

/**
 * Per-session memory descriptor mapping table sizes
 */
#define VR_MEM_DESCRIPTORS_INIT 64
#define VR_MEM_DESCRIPTORS_MAX 65536

_vr_osk_errcode_t vr_memory_session_begin(struct vr_session_data * session_data)
{
	VR_DEBUG_PRINT(5, ("Memory session begin\n"));

	/* Create descriptor mapping table */
	session_data->descriptor_mapping = vr_descriptor_mapping_create(VR_MEM_DESCRIPTORS_INIT, VR_MEM_DESCRIPTORS_MAX);

	if (NULL == session_data->descriptor_mapping) {
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	session_data->memory_lock = _vr_osk_mutex_init(_VR_OSK_LOCKFLAG_ORDERED,
	                            _VR_OSK_LOCK_ORDER_MEM_SESSION);

	if (NULL == session_data->memory_lock) {
		vr_descriptor_mapping_destroy(session_data->descriptor_mapping);
		_vr_osk_free(session_data);
		VR_ERROR(_VR_OSK_ERR_FAULT);
	}

	VR_DEBUG_PRINT(5, ("MMU session begin: success\n"));
	VR_SUCCESS;
}

/** @brief Callback function that releases memory
 *
 * session->memory_lock must be held when calling this function.
 */
static void descriptor_table_cleanup_callback(int descriptor_id, void* map_target)
{
	vr_mem_allocation *descriptor;

	descriptor = (vr_mem_allocation*)map_target;

	VR_DEBUG_ASSERT_LOCK_HELD(descriptor->session->memory_lock);

	VR_DEBUG_PRINT(3, ("Cleanup of descriptor %d mapping to 0x%x in descriptor table\n", descriptor_id, map_target));
	VR_DEBUG_ASSERT(descriptor);

	vr_mem_release(descriptor);
	vr_mem_descriptor_destroy(descriptor);
}

void vr_memory_session_end(struct vr_session_data *session)
{
	VR_DEBUG_PRINT(3, ("MMU session end\n"));

	if (NULL == session) {
		VR_DEBUG_PRINT(1, ("No session data found during session end\n"));
		return;
	}

	/* Lock the session so we can modify the memory list */
	_vr_osk_mutex_wait(session->memory_lock);

	/* Free all allocations still in the descriptor map, and terminate the map */
	if (NULL != session->descriptor_mapping) {
		vr_descriptor_mapping_call_for_each(session->descriptor_mapping, descriptor_table_cleanup_callback);
		vr_descriptor_mapping_destroy(session->descriptor_mapping);
		session->descriptor_mapping = NULL;
	}

	_vr_osk_mutex_signal(session->memory_lock);

	/* Free the lock */
	_vr_osk_mutex_term(session->memory_lock);

	return;
}

_vr_osk_errcode_t vr_memory_initialize(void)
{
	return vr_mem_os_init();
}

void vr_memory_terminate(void)
{
	vr_mem_os_term();
	vr_mem_block_allocator_destroy(NULL);
}
