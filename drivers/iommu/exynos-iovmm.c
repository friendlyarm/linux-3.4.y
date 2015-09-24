/* linux/drivers/iommu/exynos_iovmm.c
 *
 * Copyright (c) 2011-2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef CONFIG_EXYNOS_IOMMU_DEBUG
#define DEBUG
#endif

#include <linux/kernel.h>
#include <linux/hardirq.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/err.h>

#include <plat/iovmm.h>
#include <plat/sysmmu.h>

#include "exynos-iommu.h"

static struct exynos_vm_region *find_region(
		struct exynos_iovmm *vmm, dma_addr_t iova)
{
	struct exynos_vm_region *region;

	list_for_each_entry(region, &vmm->regions_list, node)
		if (region->start == iova)
			return region;

	return NULL;
}

int iovmm_activate(struct device *dev)
{
	struct exynos_iovmm *vmm = exynos_get_iovmm(dev);

	return iommu_attach_device(vmm->domain, dev);
}

void iovmm_deactivate(struct device *dev)
{
	struct exynos_iovmm *vmm = exynos_get_iovmm(dev);

	iommu_detach_device(vmm->domain, dev);
}

dma_addr_t iovmm_map(struct device *dev, struct scatterlist *sg, off_t offset,
								size_t size)
{
	off_t start_off;
	dma_addr_t addr, start = 0;
	size_t mapped_size = 0;
	struct exynos_vm_region *region;
	struct exynos_iovmm *vmm = exynos_get_iovmm(dev);
	int order;
	int ret;
#ifdef CONFIG_EXYNOS_IOVMM_ALIGN64K
	size_t iova_size = 0;
#endif

	for (; sg_dma_len(sg) < offset; sg = sg_next(sg))
		offset -= sg_dma_len(sg);

	start_off = offset_in_page(sg_phys(sg) + offset);
	size = PAGE_ALIGN(size + start_off);

	order = __fls(min_t(size_t, size, SZ_1M));

	region = kmalloc(sizeof(*region), GFP_KERNEL);
	if (!region) {
		ret = -ENOMEM;
		goto err_map_nomem;
	}

#ifdef CONFIG_EXYNOS_IOVMM_ALIGN64K
	iova_size = ALIGN(size, SZ_64K);
	start = (dma_addr_t)gen_pool_alloc_aligned(vmm->vmm_pool, iova_size,
									order);
#else
	start = (dma_addr_t)gen_pool_alloc_aligned(vmm->vmm_pool, size, order);
#endif
	if (!start) {
		ret = -ENOMEM;
		goto err_map_noiomem;
	}

	addr = start;
	do {
		phys_addr_t phys;
		size_t len;

		phys = sg_phys(sg);
		len = sg_dma_len(sg);

		/* if back to back sg entries are contiguous consolidate them */
		while (sg_next(sg) &&
		       sg_phys(sg) + sg_dma_len(sg) == sg_phys(sg_next(sg))) {
			len += sg_dma_len(sg_next(sg));
			sg = sg_next(sg);
		}

		if (offset > 0) {
			len -= offset;
			phys += offset;
			offset = 0;
		}

		if (offset_in_page(phys)) {
			len += offset_in_page(phys);
			phys = round_down(phys, PAGE_SIZE);
		}

		len = PAGE_ALIGN(len);

		if (len > (size - mapped_size))
			len = size - mapped_size;

		ret = iommu_map(vmm->domain, addr, phys, len, 0);
		if (ret)
			break;

		addr += len;
		mapped_size += len;
	} while ((sg = sg_next(sg)) && (mapped_size < size));

	BUG_ON(mapped_size > size);

	if (mapped_size < size)
		goto err_map_map;

#ifdef CONFIG_EXYNOS_IOVMM_ALIGN64K
	if (iova_size != size) {
		addr = start + size;
		size = iova_size;

		for (; addr < start + size; addr += PAGE_SIZE) {
			ret = iommu_map(vmm->domain, addr,
				page_to_phys(ZERO_PAGE(0)), PAGE_SIZE, 0);
			if (ret)
				goto err_map_map;

			mapped_size += PAGE_SIZE;
		}
	}
#endif

	region->start = start + start_off;
	region->size = size;

	INIT_LIST_HEAD(&region->node);

	spin_lock(&vmm->lock);

	list_add(&region->node, &vmm->regions_list);

	spin_unlock(&vmm->lock);

	dev_dbg(dev, "IOVMM: Allocated VM region @ %#x/%#X bytes.\n",
					region->start, region->size);

	return region->start;

err_map_map:
	iommu_unmap(vmm->domain, start, mapped_size);
	gen_pool_free(vmm->vmm_pool, start, size);
err_map_noiomem:
	kfree(region);
err_map_nomem:
	dev_dbg(dev, "IOVMM: Failed to allocated VM region for %#x bytes.\n",
									size);
	return (dma_addr_t)ret;
}

void iovmm_unmap(struct device *dev, dma_addr_t iova)
{
	struct exynos_vm_region *region;
	struct exynos_iovmm *vmm = exynos_get_iovmm(dev);
	size_t unmapped_size;

	/* This function must not be called in IRQ handlers */
	BUG_ON(in_irq());

	spin_lock(&vmm->lock);

	region = find_region(vmm, iova);
	if (WARN_ON(!region)) {
		spin_unlock(&vmm->lock);
		return;
	}

	list_del(&region->node);

	spin_unlock(&vmm->lock);

	region->start = round_down(region->start, PAGE_SIZE);

	unmapped_size = iommu_unmap(vmm->domain, region->start, region->size);

	exynos_sysmmu_tlb_invalidate(dev);

	gen_pool_free(vmm->vmm_pool, region->start, region->size);

	WARN_ON(unmapped_size != region->size);
	dev_dbg(dev, "IOVMM: Unmapped %#x bytes from %#x.\n",
					unmapped_size, region->start);

	kfree(region);
}

int iovmm_map_oto(struct device *dev, phys_addr_t phys, size_t size)
{
	struct exynos_vm_region *region;
	struct exynos_iovmm *vmm = exynos_get_iovmm(dev);
	int ret;

	if (WARN_ON((phys + size) >= IOVA_START)) {
		dev_err(dev,
			"Unable to create one to one mapping for %#x @ %#x\n",
			size, phys);
		return -EINVAL;
	}

	region = kmalloc(sizeof(*region), GFP_KERNEL);
	if (!region)
		return -ENOMEM;

	if (WARN_ON(phys & ~PAGE_MASK))
		phys = round_down(phys, PAGE_SIZE);


	ret = iommu_map(vmm->domain, (dma_addr_t)phys, phys, size, 0);
	if (ret < 0) {
		kfree(region);
		return ret;
	}

	region->start = (dma_addr_t)phys;
	region->size = size;
	INIT_LIST_HEAD(&region->node);

	spin_lock(&vmm->lock);

	list_add(&region->node, &vmm->regions_list);

	spin_unlock(&vmm->lock);

	return 0;
}

void iovmm_unmap_oto(struct device *dev, phys_addr_t phys)
{
	struct exynos_vm_region *region;
	struct exynos_iovmm *vmm = exynos_get_iovmm(dev);
	size_t unmapped_size;

	/* This function must not be called in IRQ handlers */
	BUG_ON(in_irq());

	if (WARN_ON(phys & ~PAGE_MASK))
		phys = round_down(phys, PAGE_SIZE);

	spin_lock(&vmm->lock);

	region = find_region(vmm, (dma_addr_t)phys);
	if (WARN_ON(!region)) {
		spin_unlock(&vmm->lock);
		return;
	}

	list_del(&region->node);

	spin_unlock(&vmm->lock);

	unmapped_size = iommu_unmap(vmm->domain, region->start, region->size);

	exynos_sysmmu_tlb_invalidate(dev);

	WARN_ON(unmapped_size != region->size);
	dev_dbg(dev, "IOVMM: Unmapped %#x bytes from %#x.\n",
					unmapped_size, region->start);

	kfree(region);
}

int exynos_init_iovmm(struct device *sysmmu, struct exynos_iovmm *vmm)
{
	int ret = 0;

	vmm->vmm_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!vmm->vmm_pool) {
		ret = -ENOMEM;
		goto err_setup_genalloc;
	}

	/* (1GB - 4KB) addr space from 0xC0000000 */
	ret = gen_pool_add(vmm->vmm_pool, IOVA_START, IOVM_SIZE, -1);
	if (ret)
		goto err_setup_domain;

	vmm->domain = iommu_domain_alloc(&platform_bus_type);
	if (!vmm->domain) {
		ret = -ENOMEM;
		goto err_setup_domain;
	}

	spin_lock_init(&vmm->lock);

	INIT_LIST_HEAD(&vmm->regions_list);

	dev_dbg(sysmmu, "IOVMM: Created %#x B IOVMM from %#x.\n",
						IOVM_SIZE, IOVA_START);
	return 0;
err_setup_domain:
	gen_pool_destroy(vmm->vmm_pool);
err_setup_genalloc:
	dev_dbg(sysmmu, "IOVMM: Failed to create IOVMM (%d)\n", ret);

	return ret;
}
