/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2010, 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_atomics.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include "vr_osk.h"
#include <asm/atomic.h>
#include "vr_kernel_common.h"

void _vr_osk_atomic_dec( _vr_osk_atomic_t *atom )
{
	atomic_dec((atomic_t *)&atom->u.val);
}

u32 _vr_osk_atomic_dec_return( _vr_osk_atomic_t *atom )
{
	return atomic_dec_return((atomic_t *)&atom->u.val);
}

void _vr_osk_atomic_inc( _vr_osk_atomic_t *atom )
{
	atomic_inc((atomic_t *)&atom->u.val);
}

u32 _vr_osk_atomic_inc_return( _vr_osk_atomic_t *atom )
{
	return atomic_inc_return((atomic_t *)&atom->u.val);
}

_vr_osk_errcode_t _vr_osk_atomic_init( _vr_osk_atomic_t *atom, u32 val )
{
	VR_CHECK_NON_NULL(atom, _VR_OSK_ERR_INVALID_ARGS);
	atomic_set((atomic_t *)&atom->u.val, val);
	return _VR_OSK_ERR_OK;
}

u32 _vr_osk_atomic_read( _vr_osk_atomic_t *atom )
{
	return atomic_read((atomic_t *)&atom->u.val);
}

void _vr_osk_atomic_term( _vr_osk_atomic_t *atom )
{
	VR_IGNORE(atom);
}

u32 _vr_osk_atomic_xchg( _vr_osk_atomic_t *atom, u32 val )
{
	return atomic_xchg((atomic_t*)&atom->u.val, val);
}
