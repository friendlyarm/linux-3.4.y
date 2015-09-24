/* linux/arch/arm/mach-exynos/reserve-mem.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Reserve mem helper functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef CONFIG_CMA_DEBUG
#define DEBUG
#endif

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/cma.h>

void __init exynos_cma_region_reserve(struct cma_region *regions_normal,
				      struct cma_region *regions_secure,
				      size_t align_secure, const char *map)
{
	struct cma_region *reg;
	phys_addr_t paddr_last = PHYS_OFFSET + SZ_1G;

	for (reg = regions_normal; reg->size != 0; reg++) {
		phys_addr_t paddr;

		if (!IS_ALIGNED(reg->size, PAGE_SIZE)) {
			pr_debug("S5P/CMA: size of '%s' is NOT page-aligned\n",
								reg->name);
			reg->size = PAGE_ALIGN(reg->size);
		}


		if (reg->reserved) {
			pr_err("S5P/CMA: '%s' alread reserved\n", reg->name);
			continue;
		}

		if (reg->alignment) {
			if ((reg->alignment & ~PAGE_MASK) ||
				(reg->alignment & ~reg->alignment)) {
				pr_err("S5P/CMA: Failed to reserve '%s': "
						"incorrect alignment 0x%08x.\n",
						reg->name, reg->alignment);
				continue;
			}
		} else {
			reg->alignment = PAGE_SIZE;
		}

		if (reg->start) {
			if (!memblock_is_region_reserved(reg->start, reg->size)
			    && (memblock_reserve(reg->start, reg->size) == 0))
				reg->reserved = 1;
			else
				pr_err("S5P/CMA: Failed to reserve '%s'\n",
								reg->name);

			if (reg->reserved)
				pr_debug("S5P/CMA: "
					"Reserved 0x%08x/0x%08x for '%s'\n",
					reg->start, reg->size, reg->name);
			continue;
		}

		paddr = memblock_find_in_range(0, paddr_last,
						reg->size, reg->alignment);
		if (paddr) {
			if (memblock_reserve(paddr, reg->size)) {
				pr_err("S5P/CMA: Failed to reserve '%s'\n",
								reg->name);
				continue;
			}

			reg->start = paddr;
			reg->reserved = 1;
		} else {
			pr_err("S5P/CMA: No free space in memory for '%s'\n",
								reg->name);
		}

		pr_debug("S5P/CMA: Reserved 0x%08x/0x%08x for '%s'\n",
					reg->start, reg->size, reg->name);

		if (cma_early_region_register(reg)) {
			pr_err("S5P/CMA: Failed to register '%s'\n",
								reg->name);
			memblock_free(reg->start, reg->size);
		} else {
			paddr_last = min(paddr, paddr_last);
		}
	}

	if (align_secure & ~align_secure) {
		pr_err("S5P/CMA: "
			"Wrong alignment requirement for secure region.\n");
	} else if (regions_secure && regions_secure->size) {
		size_t size_secure = 0;
		size_t align_secure_end = align_secure;

		for (reg = regions_secure; reg->size != 0; reg++)
			size_secure += reg->size;

		reg--;

		/* Entire secure regions will be merged into 2
		 * consecutive regions. */
		if (align_secure == 0) {
			size_t size_region2;
			size_t order_region2;
			size_t aug_size;

			align_secure = 1 <<
				(get_order((size_secure + 1) / 2) + PAGE_SHIFT);
			/* Calculation of a subregion size */
			size_region2 = size_secure - align_secure;
			order_region2 = get_order(size_region2) + PAGE_SHIFT;
			if (order_region2 < 20)
				order_region2 = 20; /* 1MB */
			order_region2 -= 3; /* divide by 8 */
			size_region2 = ALIGN(size_region2, 1 << order_region2);

			aug_size = align_secure + size_region2 - size_secure;
			if (aug_size > 0) {
				reg->size += aug_size;
				pr_debug("S5P/CMA: "
					"Augmented size of '%s' by %#x B.\n",
					reg->name, aug_size);
			}
			align_secure_end = 1 << order_region2;
		}

		size_secure = ALIGN(size_secure, align_secure_end);
		pr_debug("S5P/CMA: Reserving 0x%08x (0x%08x) for secure region\n",
								size_secure, align_secure);

		paddr_last = memblock_alloc_base(size_secure, align_secure,
						 paddr_last);
		if (paddr_last) {
			do {
				reg->start = paddr_last;
				reg->reserved = 1;
				paddr_last += reg->size;

				pr_debug("S5P/CMA: "
					"Reserved 0x%08x/0x%08x for '%s'\n",
					reg->start, reg->size, reg->name);
				if (cma_early_region_register(reg)) {
					memblock_free(reg->start, reg->size);
					pr_err("S5P/CMA: "
					"Failed to register secure region "
					"'%s'\n", reg->name);
				} else {
					size_secure -= reg->size;
				}
			} while (reg-- != regions_secure);

			if (size_secure > 0)
				memblock_free(paddr_last, size_secure);
		} else {
			pr_err("S5P/CMA: Failed to reserve secure regions\n");
		}
	}

	if (map)
		cma_set_defaults(NULL, map);
}
