/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2011, 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_TIMESTAMP_H__
#define __VR_TIMESTAMP_H__

#include "vr_osk.h"

VR_STATIC_INLINE _vr_osk_errcode_t _vr_timestamp_reset(void)
{
	return _VR_OSK_ERR_OK;
}

VR_STATIC_INLINE u64 _vr_timestamp_get(void)
{
	return _vr_osk_time_get_ns();
}

#endif /* __VR_TIMESTAMP_H__ */
