/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_ukk.h"
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_kernel_linux.h"

#include "vr_memory.h"

#include "ump_kernel_interface.h"

static int vr_ump_map(struct vr_session_data *session, vr_mem_allocation *descriptor)
{
	ump_dd_handle ump_mem;
	u32 nr_blocks;
	u32 i;
	ump_dd_physical_block *ump_blocks;
	struct vr_page_directory *pagedir;
	u32 offset = 0;
	u32 prop;
	_vr_osk_errcode_t err;

	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_ASSERT_POINTER(descriptor);
	VR_DEBUG_ASSERT(VR_MEM_UMP == descriptor->type);

	ump_mem = descriptor->ump_mem.handle;
	VR_DEBUG_ASSERT(UMP_DD_HANDLE_INVALID != ump_mem);

	nr_blocks = ump_dd_phys_block_count_get(ump_mem);
	if (nr_blocks == 0) {
		VR_DEBUG_PRINT(1, ("No block count\n"));
		return -EINVAL;
	}

	ump_blocks = _vr_osk_malloc(sizeof(*ump_blocks)*nr_blocks);
	if (NULL == ump_blocks) {
		return -ENOMEM;
	}

	if (UMP_DD_INVALID == ump_dd_phys_blocks_get(ump_mem, ump_blocks, nr_blocks)) {
		_vr_osk_free(ump_blocks);
		return -EFAULT;
	}

	pagedir = session->page_directory;
	prop = descriptor->vr_mapping.properties;

	err = vr_mem_vr_map_prepare(descriptor);
	if (_VR_OSK_ERR_OK != err) {
		VR_DEBUG_PRINT(1, ("Mapping of UMP memory failed\n"));

		_vr_osk_free(ump_blocks);
		return -ENOMEM;
	}

	for(i = 0; i < nr_blocks; ++i) {
		u32 virt = descriptor->vr_mapping.addr + offset;

		VR_DEBUG_PRINT(7, ("Mapping in 0x%08x size %d\n", ump_blocks[i].addr , ump_blocks[i].size));

		vr_mmu_pagedir_update(pagedir, virt, ump_blocks[i].addr,
		                        ump_blocks[i].size, prop);

		offset += ump_blocks[i].size;
	}

	if (descriptor->flags & _VR_MAP_EXTERNAL_MAP_GUARD_PAGE) {
		u32 virt = descriptor->vr_mapping.addr + offset;

		/* Map in an extra virtual guard page at the end of the VMA */
		VR_DEBUG_PRINT(6, ("Mapping in extra guard page\n"));

		vr_mmu_pagedir_update(pagedir, virt, ump_blocks[0].addr, _VR_OSK_VR_PAGE_SIZE, prop);

		offset += _VR_OSK_VR_PAGE_SIZE;
	}

	_vr_osk_free(ump_blocks);

	return 0;
}

void vr_ump_unmap(struct vr_session_data *session, vr_mem_allocation *descriptor)
{
	ump_dd_handle ump_mem;
	struct vr_page_directory *pagedir;

	ump_mem = descriptor->ump_mem.handle;
	pagedir = session->page_directory;

	VR_DEBUG_ASSERT(UMP_DD_HANDLE_INVALID != ump_mem);

	vr_mem_vr_map_free(descriptor);

	ump_dd_reference_release(ump_mem);
	return;
}

_vr_osk_errcode_t _vr_ukk_attach_ump_mem(_vr_uk_attach_ump_mem_s *args)
{
	ump_dd_handle ump_mem;
	struct vr_session_data *session;
	vr_mem_allocation *descriptor;
	int md, ret;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	session = (struct vr_session_data *)args->ctx;
	VR_CHECK_NON_NULL(session, _VR_OSK_ERR_INVALID_ARGS);

	/* check arguments */
	/* NULL might be a valid Vr address */
	if (!args->size) VR_ERROR(_VR_OSK_ERR_INVALID_ARGS);

	/* size must be a multiple of the system page size */
	if (args->size % _VR_OSK_VR_PAGE_SIZE) VR_ERROR(_VR_OSK_ERR_INVALID_ARGS);

	VR_DEBUG_PRINT(3,
	                 ("Requested to map ump memory with secure id %d into virtual memory 0x%08X, size 0x%08X\n",
	                  args->secure_id, args->vr_address, args->size));

	ump_mem = ump_dd_handle_create_from_secure_id((int)args->secure_id);

	if (UMP_DD_HANDLE_INVALID == ump_mem) VR_ERROR(_VR_OSK_ERR_FAULT);

	descriptor = vr_mem_descriptor_create(session, VR_MEM_UMP);
	if (NULL == descriptor) {
		ump_dd_reference_release(ump_mem);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	descriptor->ump_mem.handle = ump_mem;
	descriptor->vr_mapping.addr = args->vr_address;
	descriptor->size = args->size;
	descriptor->vr_mapping.properties = VR_MMU_FLAGS_DEFAULT;
	descriptor->flags |= VR_MEM_FLAG_DONT_CPU_MAP;

	if (args->flags & _VR_MAP_EXTERNAL_MAP_GUARD_PAGE) {
		descriptor->flags = VR_MEM_FLAG_VR_GUARD_PAGE;
	}

	_vr_osk_mutex_wait(session->memory_lock);

	ret = vr_ump_map(session, descriptor);
	if (0 != ret) {
		_vr_osk_mutex_signal(session->memory_lock);
		ump_dd_reference_release(ump_mem);
		vr_mem_descriptor_destroy(descriptor);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	_vr_osk_mutex_signal(session->memory_lock);


	if (_VR_OSK_ERR_OK != vr_descriptor_mapping_allocate_mapping(session->descriptor_mapping, descriptor, &md)) {
		ump_dd_reference_release(ump_mem);
		vr_mem_descriptor_destroy(descriptor);
		VR_ERROR(_VR_OSK_ERR_FAULT);
	}

	args->cookie = md;

	VR_DEBUG_PRINT(5,("Returning from UMP attach\n"));

	VR_SUCCESS;
}

void vr_mem_ump_release(vr_mem_allocation *descriptor)
{
	struct vr_session_data *session = descriptor->session;

	VR_DEBUG_ASSERT(VR_MEM_UMP == descriptor->type);

	vr_ump_unmap(session, descriptor);
}

_vr_osk_errcode_t _vr_ukk_release_ump_mem(_vr_uk_release_ump_mem_s *args)
{
	vr_mem_allocation * descriptor;
	struct vr_session_data *session;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	session = (struct vr_session_data *)args->ctx;
	VR_CHECK_NON_NULL(session, _VR_OSK_ERR_INVALID_ARGS);

	if (_VR_OSK_ERR_OK != vr_descriptor_mapping_get(session->descriptor_mapping, args->cookie, (void**)&descriptor)) {
		VR_DEBUG_PRINT(1, ("Invalid memory descriptor %d used to release ump memory\n", args->cookie));
		VR_ERROR(_VR_OSK_ERR_FAULT);
	}

	descriptor = vr_descriptor_mapping_free(session->descriptor_mapping, args->cookie);

	if (NULL != descriptor) {
		_vr_osk_mutex_wait(session->memory_lock);
		vr_mem_ump_release(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);

		vr_mem_descriptor_destroy(descriptor);
	}

	VR_SUCCESS;
}
