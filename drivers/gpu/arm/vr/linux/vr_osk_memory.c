/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2011, 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_memory.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include "vr_osk.h"
#include <linux/slab.h>
#include <linux/vmalloc.h>

void inline *_vr_osk_calloc( u32 n, u32 size )
{
	return kcalloc(n, size, GFP_KERNEL);
}

void inline *_vr_osk_malloc( u32 size )
{
	return kmalloc(size, GFP_KERNEL);
}

void inline _vr_osk_free( void *ptr )
{
	kfree(ptr);
}

void inline *_vr_osk_valloc( u32 size )
{
	return vmalloc(size);
}

void inline _vr_osk_vfree( void *ptr )
{
	vfree(ptr);
}

void inline *_vr_osk_memcpy( void *dst, const void *src, u32	len )
{
	return memcpy(dst, src, len);
}

void inline *_vr_osk_memset( void *s, u32 c, u32 n )
{
	return memset(s, c, n);
}

vr_bool _vr_osk_mem_check_allocated( u32 max_allocated )
{
	/* No need to prevent an out-of-memory dialogue appearing on Linux,
	 * so we always return VR_TRUE.
	 */
	return VR_TRUE;
}
