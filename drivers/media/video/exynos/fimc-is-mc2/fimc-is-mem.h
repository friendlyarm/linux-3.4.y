/*
 * Samsung Exynos5 SoC series FIMC-IS driver
 *
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef FIMC_IS_MEM_H
#define FIMC_IS_MEM_H

struct fimc_is_vb2 {
	const struct vb2_mem_ops *ops;
	void *(*init)(struct platform_device *pdev);
	void (*cleanup)(void *alloc_ctx);

	unsigned long (*plane_addr)(struct vb2_buffer *vb, u32 plane_no);
	unsigned long (*plane_kvaddr)(struct vb2_buffer *vb, u32 plane_no);

	int (*resume)(void *alloc_ctx);
	void (*suspend)(void *alloc_ctx);

	int (*cache_flush)(struct vb2_buffer *vb, u32 num_planes);
	void (*set_cacheable)(void *alloc_ctx, bool cacheable);
};

struct fimc_is_mem {
	struct platform_device		*pdev;
	struct vb2_alloc_ctx		*alloc_ctx;

	const struct fimc_is_vb2	*vb2;
};

int fimc_is_mem_probe(struct fimc_is_mem *this,
	struct platform_device *pdev);

#endif
