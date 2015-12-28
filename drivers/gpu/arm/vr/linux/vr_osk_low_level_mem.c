/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_low_level_mem.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/slab.h>

#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_ukk.h"

void _vr_osk_mem_barrier( void )
{
	mb();
}

void _vr_osk_write_mem_barrier( void )
{
	wmb();
}

vr_io_address _vr_osk_mem_mapioregion( u32 phys, u32 size, const char *description )
{
	return (vr_io_address)ioremap_nocache(phys, size);
}

void _vr_osk_mem_unmapioregion( u32 phys, u32 size, vr_io_address virt )
{
	iounmap((void*)virt);
}

_vr_osk_errcode_t inline _vr_osk_mem_reqregion( u32 phys, u32 size, const char *description )
{
#if VR_LICENSE_IS_GPL
	return _VR_OSK_ERR_OK; /* GPL driver gets the mem region for the resources registered automatically */
#else
	return ((NULL == request_mem_region(phys, size, description)) ? _VR_OSK_ERR_NOMEM : _VR_OSK_ERR_OK);
#endif
}

void inline _vr_osk_mem_unreqregion( u32 phys, u32 size )
{
#if !VR_LICENSE_IS_GPL
	release_mem_region(phys, size);
#endif
}

void inline _vr_osk_mem_iowrite32_relaxed( volatile vr_io_address addr, u32 offset, u32 val )
{
	__raw_writel(cpu_to_le32(val),((u8*)addr) + offset);
}

u32 inline _vr_osk_mem_ioread32( volatile vr_io_address addr, u32 offset )
{
	return ioread32(((u8*)addr) + offset);
}

void inline _vr_osk_mem_iowrite32( volatile vr_io_address addr, u32 offset, u32 val )
{
	iowrite32(val, ((u8*)addr) + offset);
}

void _vr_osk_cache_flushall( void )
{
	/** @note Cached memory is not currently supported in this implementation */
}

void _vr_osk_cache_ensure_uncached_range_flushed( void *uncached_mapping, u32 offset, u32 size )
{
	_vr_osk_write_mem_barrier();
}

u32 _vr_osk_mem_write_safe(void *dest, const void *src, u32 size)
{
#define VR_MEM_SAFE_COPY_BLOCK_SIZE 4096
	u32 retval = 0;
	void *temp_buf;

	temp_buf = kmalloc(VR_MEM_SAFE_COPY_BLOCK_SIZE, GFP_KERNEL);
	if (NULL != temp_buf) {
		u32 bytes_left_to_copy = size;
		u32 i;
		for (i = 0; i < size; i += VR_MEM_SAFE_COPY_BLOCK_SIZE) {
			u32 size_to_copy;
			u32 size_copied;
			u32 bytes_left;

			if (bytes_left_to_copy > VR_MEM_SAFE_COPY_BLOCK_SIZE) {
				size_to_copy = VR_MEM_SAFE_COPY_BLOCK_SIZE;
			} else {
				size_to_copy = bytes_left_to_copy;
			}

			bytes_left = copy_from_user(temp_buf, ((char*)src) + i, size_to_copy);
			size_copied = size_to_copy - bytes_left;

			bytes_left = copy_to_user(((char*)dest) + i, temp_buf, size_copied);
			size_copied -= bytes_left;

			bytes_left_to_copy -= size_copied;
			retval += size_copied;

			if (size_copied != size_to_copy) {
				break; /* Early out, we was not able to copy this entire block */
			}
		}

		kfree(temp_buf);
	}

	return retval;
}

_vr_osk_errcode_t _vr_ukk_mem_write_safe(_vr_uk_mem_write_safe_s *args)
{
	VR_DEBUG_ASSERT_POINTER(args);

	if (NULL == args->ctx) {
		return _VR_OSK_ERR_INVALID_ARGS;
	}

	/* Return number of bytes actually copied */
	args->size = _vr_osk_mem_write_safe(args->dest, args->src, args->size);
	return _VR_OSK_ERR_OK;
}
