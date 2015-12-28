/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_PROFILING_INTERNAL_H__
#define __VR_PROFILING_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vr_osk.h"

int _vr_internal_profiling_init(vr_bool auto_start);
void _vr_internal_profiling_term(void);

vr_bool _vr_internal_profiling_is_recording(void);
vr_bool _vr_internal_profiling_have_recording(void);
_vr_osk_errcode_t _vr_internal_profiling_clear(void);
_vr_osk_errcode_t _vr_internal_profiling_get_event(u32 index, u64* timestamp, u32* event_id, u32 data[5]);
u32 _vr_internal_profiling_get_count(void);
int _vr_internal_profiling_stop(u32 * count);
int _vr_internal_profiling_start(u32 * limit);

#ifdef __cplusplus
}
#endif

#endif /* __VR_PROFILING_INTERNAL_H__ */
