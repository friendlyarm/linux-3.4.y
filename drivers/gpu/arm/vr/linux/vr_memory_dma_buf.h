/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MEMORY_DMA_BUF_H__
#define __VR_MEMORY_DMA_BUF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vr_osk.h"
#include "vr_memory.h"

struct vr_pp_job;

struct vr_dma_buf_attachment;

int vr_attach_dma_buf(struct vr_session_data *session, _vr_uk_attach_dma_buf_s __user *arg);
int vr_release_dma_buf(struct vr_session_data *session, _vr_uk_release_dma_buf_s __user *arg);
int vr_dma_buf_get_size(struct vr_session_data *session, _vr_uk_dma_buf_get_size_s __user *arg);

void vr_mem_dma_buf_release(vr_mem_allocation *descriptor);

#if !defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
int vr_dma_buf_map_job(struct vr_pp_job *job);
void vr_dma_buf_unmap_job(struct vr_pp_job *job);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VR_MEMORY_DMA_BUF_H__ */
