/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MEM_VALIDATION_H__
#define __VR_MEM_VALIDATION_H__

#include "vr_osk.h"

_vr_osk_errcode_t vr_mem_validation_add_range(u32 start, u32 size);
_vr_osk_errcode_t vr_mem_validation_check(u32 phys_addr, u32 size);

#endif /* __VR_MEM_VALIDATION_H__ */
