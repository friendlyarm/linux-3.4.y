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
 * @file vr_osk_time.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include "vr_osk.h"
#include <linux/jiffies.h>
#include <linux/time.h>
#include <asm/delay.h>

int	_vr_osk_time_after( u32 ticka, u32 tickb )
{
	return time_after((unsigned long)ticka, (unsigned long)tickb);
}

u32	_vr_osk_time_mstoticks( u32 ms )
{
	return msecs_to_jiffies(ms);
}

u32	_vr_osk_time_tickstoms( u32 ticks )
{
	return jiffies_to_msecs(ticks);
}

u32	_vr_osk_time_tickcount( void )
{
	return jiffies;
}

void _vr_osk_time_ubusydelay( u32 usecs )
{
	udelay(usecs);
}

u64 _vr_osk_time_get_ns( void )
{
	struct timespec tsval;
	getnstimeofday(&tsval);
	return (u64)timespec_to_ns(&tsval);
}
