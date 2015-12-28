/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MEMORY_TYPES_H__
#define __VR_MEMORY_TYPES_H__

#if defined(CONFIG_VR400_UMP)
#include "ump_kernel_interface.h"
#endif

typedef u32 vr_address_t;

typedef enum vr_mem_type {
	VR_MEM_OS,
	VR_MEM_EXTERNAL,
	VR_MEM_DMA_BUF,
	VR_MEM_UMP,
	VR_MEM_BLOCK,
} vr_mem_type;

typedef struct vr_mem_os_mem {
	struct list_head pages;
	u32 count;
} vr_mem_os_mem;

typedef struct vr_mem_dma_buf {
#if defined(CONFIG_DMA_SHARED_BUFFER)
	struct vr_dma_buf_attachment *attachment;
#endif
} vr_mem_dma_buf;

typedef struct vr_mem_external {
	dma_addr_t phys;
	u32 size;
} vr_mem_external;

typedef struct vr_mem_ump {
#if defined(CONFIG_VR400_UMP)
	ump_dd_handle handle;
#endif
} vr_mem_ump;

typedef struct block_allocator_allocation {
	/* The list will be released in reverse order */
	struct block_info *last_allocated;
	u32 mapping_length;
	struct block_allocator *info;
} block_allocator_allocation;

typedef struct vr_mem_block_mem {
	block_allocator_allocation mem;
} vr_mem_block_mem;

typedef struct vr_mem_virt_vr_mapping {
	vr_address_t addr; /* Virtual Vr address */
	u32 properties;      /* MMU Permissions + cache, must match MMU HW */
} vr_mem_virt_vr_mapping;

typedef struct vr_mem_virt_cpu_mapping {
	void __user *addr;
	u32 ref;
} vr_mem_virt_cpu_mapping;

#define VR_MEM_ALLOCATION_VALID_MAGIC 0xdeda110c
#define VR_MEM_ALLOCATION_FREED_MAGIC 0x10101010

typedef struct vr_mem_allocation {
	VR_DEBUG_CODE(u32 magic);
	vr_mem_type type;                /**< Type of memory */
	int id;                            /**< ID in the descriptor map for this allocation */

	u32 size;                          /**< Size of the allocation */
	u32 flags;                         /**< Flags for this allocation */

	struct vr_session_data *session; /**< Pointer to session that owns the allocation */

	/* Union selected by type. */
	union {
		vr_mem_os_mem os_mem;       /**< VR_MEM_OS */
		vr_mem_external ext_mem;    /**< VR_MEM_EXTERNAL */
		vr_mem_dma_buf dma_buf;     /**< VR_MEM_DMA_BUF */
		vr_mem_ump ump_mem;         /**< VR_MEM_UMP */
		vr_mem_block_mem block_mem; /**< VR_MEM_BLOCK */
	};

	vr_mem_virt_cpu_mapping cpu_mapping; /**< CPU mapping */
	vr_mem_virt_vr_mapping vr_mapping; /**< Vr mapping */
} vr_mem_allocation;

#define VR_MEM_FLAG_VR_GUARD_PAGE (1 << 0)
#define VR_MEM_FLAG_DONT_CPU_MAP    (1 << 1)

#endif /* __VR_MEMORY_TYPES__ */
