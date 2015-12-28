/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MMU_PAGE_DIRECTORY_H__
#define __VR_MMU_PAGE_DIRECTORY_H__

#include "vr_osk.h"

/**
 * Size of an MMU page in bytes
 */
#define VR_MMU_PAGE_SIZE 0x1000

/*
 * Size of the address space referenced by a page table page
 */
#define VR_MMU_VIRTUAL_PAGE_SIZE 0x400000 /* 4 MiB */

/**
 * Page directory index from address
 * Calculates the page directory index from the given address
 */
#define VR_MMU_PDE_ENTRY(address) (((address)>>22) & 0x03FF)

/**
 * Page table index from address
 * Calculates the page table index from the given address
 */
#define VR_MMU_PTE_ENTRY(address) (((address)>>12) & 0x03FF)

/**
 * Extract the memory address from an PDE/PTE entry
 */
#define VR_MMU_ENTRY_ADDRESS(value) ((value) & 0xFFFFFC00)

#define VR_INVALID_PAGE ((u32)(~0))

/**
 *
 */
typedef enum vr_mmu_entry_flags {
	VR_MMU_FLAGS_PRESENT = 0x01,
	VR_MMU_FLAGS_READ_PERMISSION = 0x02,
	VR_MMU_FLAGS_WRITE_PERMISSION = 0x04,
	VR_MMU_FLAGS_OVERRIDE_CACHE  = 0x8,
	VR_MMU_FLAGS_WRITE_CACHEABLE  = 0x10,
	VR_MMU_FLAGS_WRITE_ALLOCATE  = 0x20,
	VR_MMU_FLAGS_WRITE_BUFFERABLE  = 0x40,
	VR_MMU_FLAGS_READ_CACHEABLE  = 0x80,
	VR_MMU_FLAGS_READ_ALLOCATE  = 0x100,
	VR_MMU_FLAGS_MASK = 0x1FF,
} vr_mmu_entry_flags;


#define VR_MMU_FLAGS_FORCE_GP_READ_ALLOCATE ( \
VR_MMU_FLAGS_PRESENT | \
	VR_MMU_FLAGS_READ_PERMISSION |  \
	VR_MMU_FLAGS_WRITE_PERMISSION | \
	VR_MMU_FLAGS_OVERRIDE_CACHE | \
	VR_MMU_FLAGS_WRITE_CACHEABLE | \
	VR_MMU_FLAGS_WRITE_BUFFERABLE | \
	VR_MMU_FLAGS_READ_CACHEABLE | \
	VR_MMU_FLAGS_READ_ALLOCATE )

#define VR_MMU_FLAGS_DEFAULT ( \
	VR_MMU_FLAGS_PRESENT | \
	VR_MMU_FLAGS_READ_PERMISSION |  \
	VR_MMU_FLAGS_WRITE_PERMISSION )


struct vr_page_directory {
	u32 page_directory; /**< Physical address of the memory session's page directory */
	vr_io_address page_directory_mapped; /**< Pointer to the mapped version of the page directory into the kernel's address space */

	vr_io_address page_entries_mapped[1024]; /**< Pointers to the page tables which exists in the page directory mapped into the kernel's address space */
	u32   page_entries_usage_count[1024]; /**< Tracks usage count of the page table pages, so they can be releases on the last reference */
};

/* Map Vr virtual address space (i.e. ensure page tables exist for the virtual range)  */
_vr_osk_errcode_t vr_mmu_pagedir_map(struct vr_page_directory *pagedir, u32 vr_address, u32 size);
_vr_osk_errcode_t vr_mmu_pagedir_unmap(struct vr_page_directory *pagedir, u32 vr_address, u32 size);

/* Back virtual address space with actual pages. Assumes input is contiguous and 4k aligned. */
void vr_mmu_pagedir_update(struct vr_page_directory *pagedir, u32 vr_address, u32 phys_address, u32 size, u32 cache_settings);

u32 vr_page_directory_get_phys_address(struct vr_page_directory *pagedir, u32 index);

u32 vr_allocate_empty_page(vr_io_address *virtual);
void vr_free_empty_page(u32 address, vr_io_address virtual);
_vr_osk_errcode_t vr_create_fault_flush_pages(u32 *page_directory, vr_io_address *page_directory_mapping,
        u32 *page_table, vr_io_address *page_table_mapping,
        u32 *data_page, vr_io_address *data_page_mapping);
void vr_destroy_fault_flush_pages(u32 *page_directory, vr_io_address *page_directory_mapping,
                                    u32 *page_table, vr_io_address *page_table_mapping,
                                    u32 *data_page, vr_io_address *data_page_mapping);

struct vr_page_directory *vr_mmu_pagedir_alloc(void);
void vr_mmu_pagedir_free(struct vr_page_directory *pagedir);

#endif /* __VR_MMU_PAGE_DIRECTORY_H__ */
