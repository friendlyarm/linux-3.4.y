/*
 * linux/drivers/media/video/exynos/mfc/s5p_mfc_mem.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __S5P_MFC_MEM_H_
#define __S5P_MFC_MEM_H_ __FILE__

#include <linux/platform_device.h>

#if defined(CONFIG_VIDEOBUF2_CMA_PHYS)
#include <media/videobuf2-cma-phys.h>
#elif defined(CONFIG_VIDEOBUF2_ION)
#include <media/videobuf2-ion.h>
#endif

/* Offset base used to differentiate between CAPTURE and OUTPUT
*  while mmaping */
#define DST_QUEUE_OFF_BASE      (TASK_SIZE / 2)

#if defined(CONFIG_VIDEOBUF2_CMA_PHYS)
/* Define names for CMA memory kinds used by MFC */
#define MFC_CMA_BANK1		"a"
#define MFC_CMA_BANK2		"b"
#define MFC_CMA_FW		"f"

#define MFC_CMA_FW_ALLOC_CTX	0
#define MFC_CMA_BANK1_ALLOC_CTX 1
#define MFC_CMA_BANK2_ALLOC_CTX 2

#define MFC_CMA_BANK1_ALIGN	0x2000	/* 8KB */
#define MFC_CMA_BANK2_ALIGN	0x2000	/* 8KB */
#define MFC_CMA_FW_ALIGN	0x20000	/* 128KB */

#define s5p_mfc_mem_plane_addr(c, v, n)					\
		do {							\
			(dma_addr_t)vb2_cma_phys_plane_paddr(v, n)	\
		} while (0)

static inline void *s5p_mfc_mem_alloc(void *alloc_ctx, unsigned long size)
{
	return vb2_cma_phys_memops.alloc(alloc_ctx, size);
}

static inline void s5p_mfc_mem_free(void *vb_priv)
{
	vb2_cma_phys_memops.put(vb_priv);
}

static inline dma_addr_t s5p_mfc_mem_daddr(void *vb_priv)
{
	return (dma_addr_t)vb2_cma_phys_memops.cookie(vb_priv);
}

static inline void *s5p_mfc_mem_vaddr(void *vb_priv)
{
	return vb2_cma_phys_memops.vaddr(vb_priv);
}
#elif defined(CONFIG_VIDEOBUF2_ION)
#define MFC_ALLOC_CTX_NUM	2

#define MFC_BANK_A_ALLOC_CTX	0
#define MFC_BANK_B_ALLOC_CTX	1

#define MFC_BANK_A_ALIGN_ORDER	11
#define MFC_BANK_B_ALIGN_ORDER	11

#define MFC_CMA_BANK1_ALLOC_CTX MFC_BANK_A_ALLOC_CTX
#define MFC_CMA_BANK2_ALLOC_CTX MFC_BANK_B_ALLOC_CTX
#define MFC_CMA_FW_ALLOC_CTX	MFC_BANK_A_ALLOC_CTX

static inline dma_addr_t s5p_mfc_mem_plane_addr(
	struct s5p_mfc_ctx *c, struct vb2_buffer *v, unsigned int n)
{
	void *cookie = vb2_plane_cookie(v, n);
	dma_addr_t addr = 0;

#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
	if (c->is_drm)
		WARN_ON(vb2_ion_phys_address(cookie,
					(phys_addr_t *)&addr) != 0);
	else
		WARN_ON(vb2_ion_dma_address(cookie, &addr) != 0);
#else
	WARN_ON(vb2_ion_dma_address(cookie, &addr) != 0);
#endif

	return (unsigned long)addr;
}

static inline void *s5p_mfc_mem_alloc(void *alloc_ctx, unsigned int size)
{
	return vb2_ion_private_alloc(alloc_ctx, size);
}

static inline void s5p_mfc_mem_free(void *cookie)
{
	vb2_ion_private_free(cookie);
}

static inline dma_addr_t s5p_mfc_mem_daddr(void *cookie)
{
	dma_addr_t addr = 0;

	WARN_ON(vb2_ion_dma_address(cookie, &addr) != 0);

	return addr;
}

static inline void *s5p_mfc_mem_vaddr(void *cookie)
{
	return vb2_ion_private_vaddr(cookie);
}
#endif

struct vb2_mem_ops *s5p_mfc_mem_ops(void);
void **s5p_mfc_mem_init_multi(struct device *dev, unsigned int ctx_num);
void s5p_mfc_mem_cleanup_multi(void **alloc_ctxes, unsigned int ctx_num);

void s5p_mfc_cache_clean_priv(void *cookie);
void s5p_mfc_cache_inv_priv(void *cookie);
void s5p_mfc_cache_clean(struct vb2_buffer *vb, int plane_no);
void s5p_mfc_cache_inv(struct vb2_buffer *vb, int plane_no);

void s5p_mfc_mem_suspend(void *alloc_ctx);
int s5p_mfc_mem_resume(void *alloc_ctx);

void s5p_mfc_mem_set_cacheable(void *alloc_ctx, bool cacheable);
int s5p_mfc_mem_cache_flush(struct vb2_buffer *vb, u32 plane_no);
#endif /* __S5P_MFC_MEM_H_ */
