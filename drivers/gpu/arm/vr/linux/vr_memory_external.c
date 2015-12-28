/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_osk.h"
#include "vr_memory.h"
#include "vr_kernel_descriptor_mapping.h"
#include "vr_mem_validation.h"
#include "vr_uk_types.h"

void vr_mem_external_release(vr_mem_allocation *descriptor)
{
	VR_DEBUG_ASSERT(VR_MEM_EXTERNAL == descriptor->type);

	vr_mem_vr_map_free(descriptor);
}

_vr_osk_errcode_t _vr_ukk_map_external_mem(_vr_uk_map_external_mem_s *args)
{
	struct vr_session_data *session;
	vr_mem_allocation * descriptor;
	int md;
	_vr_osk_errcode_t err;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	session = (struct vr_session_data *)args->ctx;
	VR_CHECK_NON_NULL(session, _VR_OSK_ERR_INVALID_ARGS);

	/* check arguments */
	/* NULL might be a valid Vr address */
	if (! args->size) VR_ERROR(_VR_OSK_ERR_INVALID_ARGS);

	/* size must be a multiple of the system page size */
	if (args->size % _VR_OSK_VR_PAGE_SIZE) VR_ERROR(_VR_OSK_ERR_INVALID_ARGS);

	VR_DEBUG_PRINT(3,
	                 ("Requested to map physical memory 0x%x-0x%x into virtual memory 0x%x\n",
	                  (void*)args->phys_addr,
	                  (void*)(args->phys_addr + args->size -1),
	                  (void*)args->vr_address)
	                );

	/* Validate the vr physical range */
	if (_VR_OSK_ERR_OK != vr_mem_validation_check(args->phys_addr, args->size)) {
		return _VR_OSK_ERR_FAULT;
	}

	descriptor = vr_mem_descriptor_create(session, VR_MEM_EXTERNAL);
	if (NULL == descriptor) VR_ERROR(_VR_OSK_ERR_NOMEM);

	descriptor->vr_mapping.addr = args->vr_address;
	descriptor->size = args->size;

	if (args->flags & _VR_MAP_EXTERNAL_MAP_GUARD_PAGE) {
		descriptor->flags = VR_MEM_FLAG_VR_GUARD_PAGE;
	}

	_vr_osk_mutex_wait(session->memory_lock);
	{
		u32 virt = descriptor->vr_mapping.addr;
		u32 phys = args->phys_addr;
		u32 size = args->size;

		err = vr_mem_vr_map_prepare(descriptor);
		if (_VR_OSK_ERR_OK != err) {
			_vr_osk_mutex_signal(session->memory_lock);
			vr_mem_descriptor_destroy(descriptor);
			return _VR_OSK_ERR_NOMEM;
		}

		vr_mmu_pagedir_update(session->page_directory, virt, phys, size, VR_MMU_FLAGS_DEFAULT);

		if (descriptor->flags & VR_MEM_FLAG_VR_GUARD_PAGE) {
			vr_mmu_pagedir_update(session->page_directory, virt + size, phys, _VR_OSK_VR_PAGE_SIZE, VR_MMU_FLAGS_DEFAULT);
		}
	}
	_vr_osk_mutex_signal(session->memory_lock);

	if (_VR_OSK_ERR_OK != vr_descriptor_mapping_allocate_mapping(session->descriptor_mapping, descriptor, &md)) {
		_vr_osk_mutex_wait(session->memory_lock);
		vr_mem_external_release(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);
		vr_mem_descriptor_destroy(descriptor);
		VR_ERROR(_VR_OSK_ERR_FAULT);
	}

	args->cookie = md;

	VR_SUCCESS;
}

_vr_osk_errcode_t _vr_ukk_unmap_external_mem( _vr_uk_unmap_external_mem_s *args )
{
	vr_mem_allocation * descriptor;
	void* old_value;
	struct vr_session_data *session;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	session = (struct vr_session_data *)args->ctx;
	VR_CHECK_NON_NULL(session, _VR_OSK_ERR_INVALID_ARGS);

	if (_VR_OSK_ERR_OK != vr_descriptor_mapping_get(session->descriptor_mapping, args->cookie, (void**)&descriptor)) {
		VR_DEBUG_PRINT(1, ("Invalid memory descriptor %d used to unmap external memory\n", args->cookie));
		VR_ERROR(_VR_OSK_ERR_FAULT);
	}

	old_value = vr_descriptor_mapping_free(session->descriptor_mapping, args->cookie);

	if (NULL != old_value) {
		_vr_osk_mutex_wait(session->memory_lock);
		vr_mem_external_release(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);
		vr_mem_descriptor_destroy(descriptor);
	}

	VR_SUCCESS;
}
