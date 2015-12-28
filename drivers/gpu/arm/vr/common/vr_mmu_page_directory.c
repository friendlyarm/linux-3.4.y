/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_uk_types.h"
#include "vr_mmu_page_directory.h"
#include "vr_memory.h"
#include "vr_l2_cache.h"

static _vr_osk_errcode_t fill_page(vr_io_address mapping, u32 data);

u32 vr_allocate_empty_page(vr_io_address *virt_addr)
{
	_vr_osk_errcode_t err;
	vr_io_address mapping;
	u32 address;

	if(_VR_OSK_ERR_OK != vr_mmu_get_table_page(&address, &mapping)) {
		/* Allocation failed */
		VR_DEBUG_PRINT(2, ("Vr MMU: Failed to get table page for empty pgdir\n"));
		return 0;
	}

	VR_DEBUG_ASSERT_POINTER( mapping );

	err = fill_page(mapping, 0);
	if (_VR_OSK_ERR_OK != err) {
		vr_mmu_release_table_page(address, mapping);
		VR_DEBUG_PRINT(2, ("Vr MMU: Failed to zero page\n"));
		return 0;
	}

	*virt_addr = mapping;
	return address;
}

void vr_free_empty_page(u32 address, vr_io_address virt_addr)
{
	if (VR_INVALID_PAGE != address) {
		vr_mmu_release_table_page(address, virt_addr);
	}
}

_vr_osk_errcode_t vr_create_fault_flush_pages(u32 *page_directory, vr_io_address *page_directory_mapping,
        u32 *page_table, vr_io_address *page_table_mapping,
        u32 *data_page, vr_io_address *data_page_mapping)
{
	_vr_osk_errcode_t err;

	err = vr_mmu_get_table_page(data_page, data_page_mapping);
	if (_VR_OSK_ERR_OK == err) {
		err = vr_mmu_get_table_page(page_table, page_table_mapping);
		if (_VR_OSK_ERR_OK == err) {
			err = vr_mmu_get_table_page(page_directory, page_directory_mapping);
			if (_VR_OSK_ERR_OK == err) {
				fill_page(*data_page_mapping, 0);
				fill_page(*page_table_mapping, *data_page | VR_MMU_FLAGS_DEFAULT);
				fill_page(*page_directory_mapping, *page_table | VR_MMU_FLAGS_PRESENT);
				VR_SUCCESS;
			}
			vr_mmu_release_table_page(*page_table, *page_table_mapping);
			*page_table = VR_INVALID_PAGE;
		}
		vr_mmu_release_table_page(*data_page, *data_page_mapping);
		*data_page = VR_INVALID_PAGE;
	}
	return err;
}

void vr_destroy_fault_flush_pages(u32 *page_directory, vr_io_address *page_directory_mapping,
                                    u32 *page_table, vr_io_address *page_table_mapping,
                                    u32 *data_page, vr_io_address *data_page_mapping)
{
	if (VR_INVALID_PAGE != *page_directory) {
		vr_mmu_release_table_page(*page_directory, *page_directory_mapping);
		*page_directory = VR_INVALID_PAGE;
		*page_directory_mapping = NULL;
	}

	if (VR_INVALID_PAGE != *page_table) {
		vr_mmu_release_table_page(*page_table, *page_table_mapping);
		*page_table = VR_INVALID_PAGE;
		*page_table_mapping = NULL;
	}

	if (VR_INVALID_PAGE != *data_page) {
		vr_mmu_release_table_page(*data_page, *data_page_mapping);
		*data_page = VR_INVALID_PAGE;
		*data_page_mapping = NULL;
	}
}

static _vr_osk_errcode_t fill_page(vr_io_address mapping, u32 data)
{
	int i;
	VR_DEBUG_ASSERT_POINTER( mapping );

	for(i = 0; i < VR_MMU_PAGE_SIZE/4; i++) {
		_vr_osk_mem_iowrite32_relaxed( mapping, i * sizeof(u32), data);
	}
	_vr_osk_mem_barrier();
	VR_SUCCESS;
}

_vr_osk_errcode_t vr_mmu_pagedir_map(struct vr_page_directory *pagedir, u32 vr_address, u32 size)
{
	const int first_pde = VR_MMU_PDE_ENTRY(vr_address);
	const int last_pde = VR_MMU_PDE_ENTRY(vr_address + size - 1);
	_vr_osk_errcode_t err;
	vr_io_address pde_mapping;
	u32 pde_phys;
	int i;

	if (last_pde < first_pde) {
		VR_ERROR(_VR_OSK_ERR_INVALID_ARGS);
	}

	for(i = first_pde; i <= last_pde; i++) {
		if(0 == (_vr_osk_mem_ioread32(pagedir->page_directory_mapped, i*sizeof(u32)) & VR_MMU_FLAGS_PRESENT)) {
			/* Page table not present */
			VR_DEBUG_ASSERT(0 == pagedir->page_entries_usage_count[i]);
			VR_DEBUG_ASSERT(NULL == pagedir->page_entries_mapped[i]);

			err = vr_mmu_get_table_page(&pde_phys, &pde_mapping);
			if(_VR_OSK_ERR_OK != err) {
				VR_PRINT_ERROR(("Failed to allocate page table page.\n"));
				return err;
			}
			pagedir->page_entries_mapped[i] = pde_mapping;

			/* Update PDE, mark as present */
			_vr_osk_mem_iowrite32_relaxed(pagedir->page_directory_mapped, i*sizeof(u32),
			                                pde_phys | VR_MMU_FLAGS_PRESENT);

			VR_DEBUG_ASSERT(0 == pagedir->page_entries_usage_count[i]);
			pagedir->page_entries_usage_count[i] = 1;
		} else {
			pagedir->page_entries_usage_count[i]++;
		}
	}
	_vr_osk_write_mem_barrier();

	VR_SUCCESS;
}

VR_STATIC_INLINE void vr_mmu_zero_pte(vr_io_address page_table, u32 vr_address, u32 size)
{
	int i;
	const int first_pte = VR_MMU_PTE_ENTRY(vr_address);
	const int last_pte = VR_MMU_PTE_ENTRY(vr_address + size - 1);

	for (i = first_pte; i <= last_pte; i++) {
		_vr_osk_mem_iowrite32_relaxed(page_table, i * sizeof(u32), 0);
	}
}

_vr_osk_errcode_t vr_mmu_pagedir_unmap(struct vr_page_directory *pagedir, u32 vr_address, u32 size)
{
	const int first_pde = VR_MMU_PDE_ENTRY(vr_address);
	const int last_pde = VR_MMU_PDE_ENTRY(vr_address + size - 1);
	u32 left = size;
	int i;
	vr_bool pd_changed = VR_FALSE;
	u32 pages_to_invalidate[3]; /* hard-coded to 3: max two pages from the PT level plus max one page from PD level */
	u32 num_pages_inv = 0;
	vr_bool invalidate_all = VR_FALSE; /* safety mechanism in case page_entries_usage_count is unreliable */

	/* For all page directory entries in range. */
	for (i = first_pde; i <= last_pde; i++) {
		u32 size_in_pde, offset;

		VR_DEBUG_ASSERT_POINTER(pagedir->page_entries_mapped[i]);
		VR_DEBUG_ASSERT(0 != pagedir->page_entries_usage_count[i]);

		/* Offset into page table, 0 if vr_address is 4MiB aligned */
		offset = (vr_address & (VR_MMU_VIRTUAL_PAGE_SIZE - 1));
		if (left < VR_MMU_VIRTUAL_PAGE_SIZE - offset) {
			size_in_pde = left;
		} else {
			size_in_pde = VR_MMU_VIRTUAL_PAGE_SIZE - offset;
		}

		pagedir->page_entries_usage_count[i]--;

		/* If entire page table is unused, free it */
		if (0 == pagedir->page_entries_usage_count[i]) {
			u32 page_phys;
			void *page_virt;
			VR_DEBUG_PRINT(4, ("Releasing page table as this is the last reference\n"));
			/* last reference removed, no need to zero out each PTE  */

			page_phys = VR_MMU_ENTRY_ADDRESS(_vr_osk_mem_ioread32(pagedir->page_directory_mapped, i*sizeof(u32)));
			page_virt = pagedir->page_entries_mapped[i];
			pagedir->page_entries_mapped[i] = NULL;
			_vr_osk_mem_iowrite32_relaxed(pagedir->page_directory_mapped, i*sizeof(u32), 0);

			vr_mmu_release_table_page(page_phys, page_virt);
			pd_changed = VR_TRUE;
		} else {
			VR_DEBUG_ASSERT(num_pages_inv < 2);
			if (num_pages_inv < 2) {
				pages_to_invalidate[num_pages_inv] = vr_page_directory_get_phys_address(pagedir, i);
				num_pages_inv++;
			} else {
				invalidate_all = VR_TRUE;
			}

			/* If part of the page table is still in use, zero the relevant PTEs */
			vr_mmu_zero_pte(pagedir->page_entries_mapped[i], vr_address, size_in_pde);
		}

		left -= size_in_pde;
		vr_address += size_in_pde;
	}
	_vr_osk_write_mem_barrier();

	/* L2 pages invalidation */
	if (VR_TRUE == pd_changed) {
		VR_DEBUG_ASSERT(num_pages_inv < 3);
		if (num_pages_inv < 3) {
			pages_to_invalidate[num_pages_inv] = pagedir->page_directory;
			num_pages_inv++;
		} else {
			invalidate_all = VR_TRUE;
		}
	}

	if (invalidate_all) {
		vr_l2_cache_invalidate_all();
	} else {
		vr_l2_cache_invalidate_all_pages(pages_to_invalidate, num_pages_inv);
	}

	VR_SUCCESS;
}

struct vr_page_directory *vr_mmu_pagedir_alloc(void)
{
	struct vr_page_directory *pagedir;

	pagedir = _vr_osk_calloc(1, sizeof(struct vr_page_directory));
	if(NULL == pagedir) {
		return NULL;
	}

	if(_VR_OSK_ERR_OK != vr_mmu_get_table_page(&pagedir->page_directory, &pagedir->page_directory_mapped)) {
		_vr_osk_free(pagedir);
		return NULL;
	}

	/* Zero page directory */
	fill_page(pagedir->page_directory_mapped, 0);

	return pagedir;
}

void vr_mmu_pagedir_free(struct vr_page_directory *pagedir)
{
	const int num_page_table_entries = sizeof(pagedir->page_entries_mapped) / sizeof(pagedir->page_entries_mapped[0]);
	int i;

	/* Free referenced page tables and zero PDEs. */
	for (i = 0; i < num_page_table_entries; i++) {
		if (pagedir->page_directory_mapped && (_vr_osk_mem_ioread32(pagedir->page_directory_mapped, sizeof(u32)*i) & VR_MMU_FLAGS_PRESENT)) {
			u32 phys = _vr_osk_mem_ioread32(pagedir->page_directory_mapped, i*sizeof(u32)) & ~VR_MMU_FLAGS_MASK;
			_vr_osk_mem_iowrite32_relaxed(pagedir->page_directory_mapped, i * sizeof(u32), 0);
			vr_mmu_release_table_page(phys, pagedir->page_entries_mapped[i]);
		}
	}
	_vr_osk_write_mem_barrier();

	/* Free the page directory page. */
	vr_mmu_release_table_page(pagedir->page_directory, pagedir->page_directory_mapped);

	_vr_osk_free(pagedir);
}


void vr_mmu_pagedir_update(struct vr_page_directory *pagedir, u32 vr_address, u32 phys_address, u32 size, u32 permission_bits)
{
	u32 end_address = vr_address + size;

	/* Map physical pages into MMU page tables */
	for ( ; vr_address < end_address; vr_address += VR_MMU_PAGE_SIZE, phys_address += VR_MMU_PAGE_SIZE) {
		VR_DEBUG_ASSERT_POINTER(pagedir->page_entries_mapped[VR_MMU_PDE_ENTRY(vr_address)]);
		_vr_osk_mem_iowrite32_relaxed(pagedir->page_entries_mapped[VR_MMU_PDE_ENTRY(vr_address)],
		                                VR_MMU_PTE_ENTRY(vr_address) * sizeof(u32),
		                                phys_address | permission_bits);
	}
}

u32 vr_page_directory_get_phys_address(struct vr_page_directory *pagedir, u32 index)
{
	return (_vr_osk_mem_ioread32(pagedir->page_directory_mapped, index*sizeof(u32)) & ~VR_MMU_FLAGS_MASK);
}

/* For instrumented */
struct dump_info {
	u32 buffer_left;
	u32 register_writes_size;
	u32 page_table_dump_size;
	u32 *buffer;
};

static _vr_osk_errcode_t writereg(u32 where, u32 what, const char *comment, struct dump_info *info)
{
	if (NULL != info) {
		info->register_writes_size += sizeof(u32)*2; /* two 32-bit words */

		if (NULL != info->buffer) {
			/* check that we have enough space */
			if (info->buffer_left < sizeof(u32)*2) VR_ERROR(_VR_OSK_ERR_NOMEM);

			*info->buffer = where;
			info->buffer++;

			*info->buffer = what;
			info->buffer++;

			info->buffer_left -= sizeof(u32)*2;
		}
	}

	VR_SUCCESS;
}

static _vr_osk_errcode_t vr_mmu_dump_page(vr_io_address page, u32 phys_addr, struct dump_info * info)
{
	if (NULL != info) {
		/* 4096 for the page and 4 bytes for the address */
		const u32 page_size_in_elements = VR_MMU_PAGE_SIZE / 4;
		const u32 page_size_in_bytes = VR_MMU_PAGE_SIZE;
		const u32 dump_size_in_bytes = VR_MMU_PAGE_SIZE + 4;

		info->page_table_dump_size += dump_size_in_bytes;

		if (NULL != info->buffer) {
			if (info->buffer_left < dump_size_in_bytes) VR_ERROR(_VR_OSK_ERR_NOMEM);

			*info->buffer = phys_addr;
			info->buffer++;

			_vr_osk_memcpy(info->buffer, page, page_size_in_bytes);
			info->buffer += page_size_in_elements;

			info->buffer_left -= dump_size_in_bytes;
		}
	}

	VR_SUCCESS;
}

static _vr_osk_errcode_t dump_mmu_page_table(struct vr_page_directory *pagedir, struct dump_info * info)
{
	VR_DEBUG_ASSERT_POINTER(pagedir);
	VR_DEBUG_ASSERT_POINTER(info);

	if (NULL != pagedir->page_directory_mapped) {
		int i;

		VR_CHECK_NO_ERROR(
		    vr_mmu_dump_page(pagedir->page_directory_mapped, pagedir->page_directory, info)
		);

		for (i = 0; i < 1024; i++) {
			if (NULL != pagedir->page_entries_mapped[i]) {
				VR_CHECK_NO_ERROR(
				    vr_mmu_dump_page(pagedir->page_entries_mapped[i],
				                       _vr_osk_mem_ioread32(pagedir->page_directory_mapped,
				                               i * sizeof(u32)) & ~VR_MMU_FLAGS_MASK, info)
				);
			}
		}
	}

	VR_SUCCESS;
}

static _vr_osk_errcode_t dump_mmu_registers(struct vr_page_directory *pagedir, struct dump_info * info)
{
	VR_CHECK_NO_ERROR(writereg(0x00000000, pagedir->page_directory,
	                             "set the page directory address", info));
	VR_CHECK_NO_ERROR(writereg(0x00000008, 4, "zap???", info));
	VR_CHECK_NO_ERROR(writereg(0x00000008, 0, "enable paging", info));
	VR_SUCCESS;
}

_vr_osk_errcode_t _vr_ukk_query_mmu_page_table_dump_size( _vr_uk_query_mmu_page_table_dump_size_s *args )
{
	struct dump_info info = { 0, 0, 0, NULL };
	struct vr_session_data * session_data;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	session_data = (struct vr_session_data *)(args->ctx);

	VR_CHECK_NO_ERROR(dump_mmu_registers(session_data->page_directory, &info));
	VR_CHECK_NO_ERROR(dump_mmu_page_table(session_data->page_directory, &info));
	args->size = info.register_writes_size + info.page_table_dump_size;
	VR_SUCCESS;
}

_vr_osk_errcode_t _vr_ukk_dump_mmu_page_table( _vr_uk_dump_mmu_page_table_s * args )
{
	struct dump_info info = { 0, 0, 0, NULL };
	struct vr_session_data * session_data;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);
	VR_CHECK_NON_NULL(args->buffer, _VR_OSK_ERR_INVALID_ARGS);

	session_data = (struct vr_session_data *)(args->ctx);

	info.buffer_left = args->size;
	info.buffer = args->buffer;

	args->register_writes = info.buffer;
	VR_CHECK_NO_ERROR(dump_mmu_registers(session_data->page_directory, &info));

	args->page_table_dump = info.buffer;
	VR_CHECK_NO_ERROR(dump_mmu_page_table(session_data->page_directory, &info));

	args->register_writes_size = info.register_writes_size;
	args->page_table_dump_size = info.page_table_dump_size;

	VR_SUCCESS;
}
