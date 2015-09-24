/* linux/arch/arm/mach-exynos/dev-ion.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/ion.h>
#include <linux/exynos_ion.h>
#include <linux/slab.h>
#include <mach/exynos-ion.h>


struct ion_platform_heap heaps[] = {
	{	.type = ION_HEAP_TYPE_SYSTEM,
		.name = "ion_noncontig_heap",
		.id = EXYNOS_ION_HEAP_SYSTEM_ID,
	},
	{	.type = ION_HEAP_TYPE_EXYNOS,
		.name = "exynos_noncontig_heap",
		.id = EXYNOS_ION_HEAP_EXYNOS_ID,
	},
	{	.type = ION_HEAP_TYPE_EXYNOS_CONTIG,
		.name = "exynos_contig_heap",
		.id = EXYNOS_ION_HEAP_EXYNOS_CONTIG_ID,
	},
	{	.type = ION_HEAP_TYPE_CHUNK,
		.name = "ion_chunk_heap",
		.id = EXYNOS_ION_HEAP_CHUNK_ID,
		.size = SZ_256M,
		.align = SZ_1M,
		.priv = (void *)SZ_1M,
	},
};

struct ion_platform_data exynos_ion_pdata = {
	.nr = ARRAY_SIZE(heaps),
	.heaps = &heaps,
};

struct platform_device exynos_device_ion = {
	.name		= "ion-exynos",
	.id		= -1,
	.dev		= {
		.platform_data = &exynos_ion_pdata,
	}
};
