/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2007-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_KERNEL_CORE_H__
#define __VR_KERNEL_CORE_H__

#include "vr_osk.h"

typedef enum {
	_VR_PRODUCT_ID_UNKNOWN,
	_VR_PRODUCT_ID_VR200,
	_VR_PRODUCT_ID_VR300,
	_VR_PRODUCT_ID_VR400,
	_VR_PRODUCT_ID_VR450,
} _vr_product_id_t;

extern vr_bool vr_gpu_class_is_vr450;

_vr_osk_errcode_t vr_initialize_subsystems(void);

void vr_terminate_subsystems(void);

_vr_product_id_t vr_kernel_core_get_product_id(void);

u32 vr_kernel_core_get_gpu_major_version(void);

u32 vr_kernel_core_get_gpu_minor_version(void);

u32 _vr_kernel_core_dump_state(char* buf, u32 size);

VR_STATIC_INLINE vr_bool vr_is_vr450(void)
{
#if defined(CONFIG_VR450)
	return vr_gpu_class_is_vr450;
#else
	return VR_FALSE;
#endif
}

VR_STATIC_INLINE vr_bool vr_is_vr400(void)
{
#if !defined(CONFIG_VR450)
	return VR_TRUE;
#else
	return !vr_gpu_class_is_vr450;
#endif
}

#endif /* __VR_KERNEL_CORE_H__ */
