/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_DLBU_H__
#define __VR_DLBU_H__

#define VR_DLBU_VIRT_ADDR 0xFFF00000 /* master tile virtual address fixed at this value and mapped into every session */

#include "vr_osk.h"

struct vr_pp_job;
struct vr_group;

extern u32 vr_dlbu_phys_addr;

struct vr_dlbu_core;

_vr_osk_errcode_t vr_dlbu_initialize(void);
void vr_dlbu_terminate(void);

struct vr_dlbu_core *vr_dlbu_create(const _vr_osk_resource_t * resource);
void vr_dlbu_delete(struct vr_dlbu_core *dlbu);

_vr_osk_errcode_t vr_dlbu_reset(struct vr_dlbu_core *dlbu);

void vr_dlbu_add_group(struct vr_dlbu_core *dlbu, struct vr_group *group);
void vr_dlbu_remove_group(struct vr_dlbu_core *dlbu, struct vr_group *group);

/** @brief Called to update HW after DLBU state changed
 *
 * This function must be called after \a vr_dlbu_add_group or \a
 * vr_dlbu_remove_group to write the updated mask to hardware, unless the
 * same is accomplished by calling \a vr_dlbu_reset.
 */
void vr_dlbu_update_mask(struct vr_dlbu_core *dlbu);

void vr_dlbu_config_job(struct vr_dlbu_core *dlbu, struct vr_pp_job *job);

#endif /* __VR_DLBU_H__ */
