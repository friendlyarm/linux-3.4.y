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
 * @file vr_osk_timers.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include <linux/timer.h>
#include <linux/slab.h>
#include "vr_osk.h"
#include "vr_kernel_common.h"

struct _vr_osk_timer_t_struct {
	struct timer_list timer;
};

typedef void (*timer_timeout_function_t)(unsigned long);

_vr_osk_timer_t *_vr_osk_timer_init(void)
{
	_vr_osk_timer_t *t = (_vr_osk_timer_t*)kmalloc(sizeof(_vr_osk_timer_t), GFP_KERNEL);
	if (NULL != t) init_timer(&t->timer);
	return t;
}

void _vr_osk_timer_add( _vr_osk_timer_t *tim, u32 ticks_to_expire )
{
	VR_DEBUG_ASSERT_POINTER(tim);
	tim->timer.expires = jiffies + ticks_to_expire;
	add_timer(&(tim->timer));
}

void _vr_osk_timer_mod( _vr_osk_timer_t *tim, u32 ticks_to_expire)
{
	VR_DEBUG_ASSERT_POINTER(tim);
	mod_timer(&(tim->timer), jiffies + ticks_to_expire);
}

void _vr_osk_timer_del( _vr_osk_timer_t *tim )
{
	VR_DEBUG_ASSERT_POINTER(tim);
	del_timer_sync(&(tim->timer));
}

void _vr_osk_timer_del_async( _vr_osk_timer_t *tim )
{
	VR_DEBUG_ASSERT_POINTER(tim);
	del_timer(&(tim->timer));
}

vr_bool _vr_osk_timer_pending( _vr_osk_timer_t *tim )
{
	VR_DEBUG_ASSERT_POINTER(tim);
	return 1 == timer_pending(&(tim->timer));
}

void _vr_osk_timer_setcallback( _vr_osk_timer_t *tim, _vr_osk_timer_callback_t callback, void *data )
{
	VR_DEBUG_ASSERT_POINTER(tim);
	tim->timer.data = (unsigned long)data;
	tim->timer.function = (timer_timeout_function_t)callback;
}

void _vr_osk_timer_term( _vr_osk_timer_t *tim )
{
	VR_DEBUG_ASSERT_POINTER(tim);
	kfree(tim);
}
