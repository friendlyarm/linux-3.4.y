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
 * @file vr_osk_math.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include "vr_osk.h"
#include <linux/bitops.h>

u32 _vr_osk_clz( u32 input )
{
	return 32-fls(input);
}

u32 _vr_osk_fls( u32 input )
{
	return fls(input);
}
