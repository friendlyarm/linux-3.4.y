/* linux/drivers/media/video/videobuf2-ion.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Implementation of Android ION memory allocator for videobuf2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/dma-buf.h>
#include <linux/fs.h>
#include <linux/mutex.h>

#include <media/videobuf2-core.h>
#include <media/videobuf2-memops.h>
#include <media/videobuf2-ion.h>

#include <asm/cacheflush.h>

#include <plat/iovmm.h>
#include <plat/cpu.h>

struct vb2_ion_context {
	struct device		*dev;
	struct ion_client	*client;
	unsigned long		alignment;
	long			flags;

	/* protects iommu_active_cnt and protected */
	struct mutex		lock;
	int			iommu_active_cnt;
	bool			protected;
};

struct vb2_ion_buf {
	struct vb2_ion_context		*ctx;
	struct vb2_vmarea_handler	handler;
	struct ion_handle		*handle;
	struct dma_buf			*dma_buf;
	struct dma_buf_attachment	*attachment;
	enum dma_data_direction		direction;
	void				*kva;
	unsigned long			size;
	atomic_t			ref;
	bool				cached;
	struct vb2_ion_cookie		cookie;
};

#define ctx_cached(ctx) (!(ctx->flags & VB2ION_CTX_UNCACHED))
#define ctx_iommu(ctx) (!!(ctx->flags & VB2ION_CTX_IOMMU))

void vb2_ion_set_cached(void *ctx, bool cached)
{
	struct vb2_ion_context *vb2ctx = ctx;

	if (cached)
		vb2ctx->flags &= ~VB2ION_CTX_UNCACHED;
	else
		vb2ctx->flags |= VB2ION_CTX_UNCACHED;
}
EXPORT_SYMBOL(vb2_ion_set_cached);

/*
 * when a context is protected, we cannot use the IOMMU since
 * secure world is in charge.
 */
void vb2_ion_set_protected(void *ctx, bool ctx_protected)
{
	struct vb2_ion_context *vb2ctx = ctx;

	mutex_lock(&vb2ctx->lock);

	if (vb2ctx->protected == ctx_protected)
		goto out;
	vb2ctx->protected = ctx_protected;

	if (ctx_protected) {
		if (vb2ctx->iommu_active_cnt) {
			dev_dbg(vb2ctx->dev, "detaching active MMU\n");
			iovmm_deactivate(vb2ctx->dev);
		}
	} else {
		if (vb2ctx->iommu_active_cnt) {
			dev_dbg(vb2ctx->dev, "re-attaching active MMU\n");
			iovmm_activate(vb2ctx->dev);
		}
	}

out:
	mutex_unlock(&vb2ctx->lock);
}
EXPORT_SYMBOL(vb2_ion_set_protected);

int vb2_ion_set_alignment(void *ctx, size_t alignment)
{
	struct vb2_ion_context *vb2ctx = ctx;

	if ((alignment != 0) && (alignment < PAGE_SIZE))
		return -EINVAL;

	if (alignment & ~alignment)
		return -EINVAL;

	if (alignment == 0)
		vb2ctx->alignment = PAGE_SIZE;
	else
		vb2ctx->alignment = alignment;

	return 0;
}
EXPORT_SYMBOL(vb2_ion_set_alignment);

void *vb2_ion_create_context(struct device *dev, size_t alignment, long flags)
{
	struct vb2_ion_context *ctx;
	unsigned int heapmask = ion_heapflag(flags);

	/* ion_client_create() expects the current thread to be a kernel thread
	 * to create a new ion_client
	 */
	WARN_ON(!(current->group_leader->flags & PF_KTHREAD));

	 /* non-contigous memory without H/W virtualization is not supported */
	if ((flags & VB2ION_CTX_VMCONTIG) && !(flags & VB2ION_CTX_IOMMU))
		return ERR_PTR(-EINVAL);

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return ERR_PTR(-ENOMEM);

	ctx->dev = dev;
	ctx->client = ion_client_create(ion_exynos, dev_name(dev));
	if (IS_ERR(ctx->client)) {
		void *retp = ctx->client;
		kfree(ctx);
		return retp;
	}

	vb2_ion_set_alignment(ctx, alignment);
	ctx->flags = flags;
	mutex_init(&ctx->lock);

	return ctx;
}
EXPORT_SYMBOL(vb2_ion_create_context);

void vb2_ion_destroy_context(void *ctx)
{
	struct vb2_ion_context *vb2ctx = ctx;

	mutex_destroy(&vb2ctx->lock);
	ion_client_destroy(vb2ctx->client);
	kfree(vb2ctx);
}
EXPORT_SYMBOL(vb2_ion_destroy_context);

void *vb2_ion_private_alloc(void *alloc_ctx, size_t size)
{
	struct vb2_ion_context *ctx = alloc_ctx;
	struct vb2_ion_buf *buf;
	int flags = ion_heapflag(ctx->flags);
	int ret = 0;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	size = PAGE_ALIGN(size);

	buf->handle = ion_alloc(ctx->client, size, ctx->alignment,
				flags, flags);
	if (IS_ERR(buf->handle)) {
		ret = -ENOMEM;
		goto err_alloc;
	}

	buf->cookie.sgt = ion_sg_table(ctx->client, buf->handle);

	buf->ctx = ctx;
	buf->size = size;
	buf->cached = ctx_cached(ctx);

	buf->kva = ion_map_kernel(ctx->client, buf->handle);
	if (IS_ERR(buf->kva)) {
		ret = PTR_ERR(buf->kva);
		buf->kva = NULL;
		goto err_map_kernel;
	}

	mutex_lock(&ctx->lock);
	if (ctx_iommu(ctx) && !ctx->protected) {
		buf->cookie.ioaddr = iovmm_map(ctx->dev,
					       buf->cookie.sgt->sgl, 0,
					       buf->size);
		if (IS_ERR_VALUE(buf->cookie.ioaddr)) {
			ret = (int)buf->cookie.ioaddr;
			mutex_unlock(&ctx->lock);
			goto err_ion_map_io;
		}
	}
	mutex_unlock(&ctx->lock);

	return &buf->cookie;

err_ion_map_io:
	ion_unmap_kernel(ctx->client, buf->handle);
err_map_kernel:
	ion_free(ctx->client, buf->handle);
err_alloc:
	kfree(buf);

	pr_err("%s: Error occured while allocating\n", __func__);
	return ERR_PTR(ret);
}

void vb2_ion_private_free(void *cookie)
{
	struct vb2_ion_buf *buf =
			container_of(cookie, struct vb2_ion_buf, cookie);
	struct vb2_ion_context *ctx;

	if (WARN_ON(IS_ERR_OR_NULL(cookie)))
		return;

	ctx = buf->ctx;
	mutex_lock(&ctx->lock);
	if (ctx_iommu(ctx) && !ctx->protected)
		iovmm_unmap(ctx->dev, buf->cookie.ioaddr);
	mutex_unlock(&ctx->lock);

	ion_unmap_kernel(ctx->client, buf->handle);

	ion_free(ctx->client, buf->handle);

	kfree(buf);
}

static void vb2_ion_put(void *buf_priv)
{
	struct vb2_ion_buf *buf = buf_priv;

	if (atomic_dec_and_test(&buf->ref))
		vb2_ion_private_free(&buf->cookie);
}

static void *vb2_ion_alloc(void *alloc_ctx, unsigned long size)
{
	struct vb2_ion_buf *buf;
	void *cookie;

	cookie = vb2_ion_private_alloc(alloc_ctx, size);
	if (IS_ERR(cookie))
		return cookie;

	buf = container_of(cookie, struct vb2_ion_buf, cookie);

	buf->handler.refcount = &buf->ref;
	buf->handler.put = vb2_ion_put;
	buf->handler.arg = buf;
	atomic_set(&buf->ref, 1);

	return buf;
}


void *vb2_ion_private_vaddr(void *cookie)
{
	if (WARN_ON(IS_ERR_OR_NULL(cookie)))
		return NULL;

	return container_of(cookie, struct vb2_ion_buf, cookie)->kva;
}

static void *vb2_ion_cookie(void *buf_priv)
{
	struct vb2_ion_buf *buf = buf_priv;

	if (WARN_ON(!buf))
		return NULL;

	return (void *)&buf->cookie;
}

static void *vb2_ion_vaddr(void *buf_priv)
{
	struct vb2_ion_buf *buf = buf_priv;

	if (WARN_ON(!buf))
		return NULL;

	if (buf->kva != NULL)
		return buf->kva;

	if (dma_buf_begin_cpu_access(buf->dma_buf,
		0, buf->size, buf->direction))
		return NULL;

	buf->kva = dma_buf_kmap(buf->dma_buf, 0);

	if (buf->kva == NULL)
		dma_buf_end_cpu_access(buf->dma_buf, 0,
			buf->size, buf->direction);

	return buf->kva;
}

static unsigned int vb2_ion_num_users(void *buf_priv)
{
	struct vb2_ion_buf *buf = buf_priv;

	if (WARN_ON(!buf))
		return 0;

	return atomic_read(&buf->ref);
}

static int vb2_ion_mmap(void *buf_priv, struct vm_area_struct *vma)
{
	struct vb2_ion_buf *buf = buf_priv;
	unsigned long vm_start = vma->vm_start;
	unsigned long vm_end = vma->vm_end;
	struct scatterlist *sg = buf->cookie.sgt->sgl;
	unsigned long size;
	int ret = -EINVAL;

	if (buf->size  < (vm_end - vm_start))
		return ret;

	if (!buf->cached)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	size = min_t(size_t, vm_end - vm_start, sg_dma_len(sg));

	ret = remap_pfn_range(vma, vm_start, page_to_pfn(sg_page(sg)),
				size, vma->vm_page_prot);

	for (sg = sg_next(sg), vm_start += size;
			!ret && sg && (vm_start < vm_end);
			vm_start += size, sg = sg_next(sg)) {
		size = min_t(size_t, vm_end - vm_start, sg_dma_len(sg));
		ret = remap_pfn_range(vma, vm_start, page_to_pfn(sg_page(sg)),
						size, vma->vm_page_prot);
	}

	if (ret)
		return ret;

	if (vm_start < vm_end)
		return -EINVAL;

	vma->vm_flags		|= VM_DONTEXPAND;
	vma->vm_private_data	= &buf->handler;
	vma->vm_ops		= &vb2_common_vm_ops;

	vma->vm_ops->open(vma);

	return ret;
}

static int vb2_ion_map_dmabuf(void *mem_priv)
{
	struct vb2_ion_buf *buf = mem_priv;
	struct vb2_ion_context *ctx = buf->ctx;

	if (WARN_ON(!buf->attachment)) {
		pr_err("trying to pin a non attached buffer\n");
		return -EINVAL;
	}

	if (WARN_ON(buf->cookie.sgt)) {
		pr_err("dmabuf buffer is already pinned\n");
		return 0;
	}

	/* get the associated scatterlist for this buffer */
	buf->cookie.sgt = dma_buf_map_attachment(buf->attachment,
		buf->direction);
	if (IS_ERR_OR_NULL(buf->cookie.sgt)) {
		pr_err("Error getting dmabuf scatterlist\n");
		return -EINVAL;
	}

	buf->cookie.offset = 0;
	/* buf->kva = NULL; */

	mutex_lock(&ctx->lock);
	if (ctx_iommu(ctx) && !ctx->protected && buf->cookie.ioaddr == 0) {
		buf->cookie.ioaddr = iovmm_map(ctx->dev,
				       buf->cookie.sgt->sgl, 0, buf->size);
		if (IS_ERR_VALUE(buf->cookie.ioaddr)) {
			mutex_unlock(&ctx->lock);
			dma_buf_unmap_attachment(buf->attachment,
					buf->cookie.sgt, buf->direction);
			return (int)buf->cookie.ioaddr;
		}
	}
	mutex_unlock(&ctx->lock);

	return 0;
}

static void vb2_ion_unmap_dmabuf(void *mem_priv)
{
	struct vb2_ion_buf *buf = mem_priv;

	if (WARN_ON(!buf->attachment)) {
		pr_err("trying to unpin a not attached buffer\n");
		return;
	}

	if (WARN_ON(!buf->cookie.sgt)) {
		pr_err("dmabuf buffer is already unpinned\n");
		return;
	}

	dma_buf_unmap_attachment(buf->attachment,
		buf->cookie.sgt, buf->direction);
	buf->cookie.sgt = NULL;
}

static void vb2_ion_detach_dmabuf(void *mem_priv)
{
	struct vb2_ion_buf *buf = mem_priv;
	struct vb2_ion_context *ctx = buf->ctx;

	mutex_lock(&ctx->lock);
	if (buf->cookie.ioaddr && ctx_iommu(ctx) && !ctx->protected ) {
		iovmm_unmap(ctx->dev, buf->cookie.ioaddr);
		buf->cookie.ioaddr = 0;
	}
	mutex_unlock(&ctx->lock);

	if (buf->kva != NULL) {
		dma_buf_kunmap(buf->dma_buf, 0, buf->kva);
		dma_buf_end_cpu_access(buf->dma_buf, 0, buf->size, 0);
	}

	/* detach this attachment */
	dma_buf_detach(buf->dma_buf, buf->attachment);
	kfree(buf);
}

static void *vb2_ion_attach_dmabuf(void *alloc_ctx, struct dma_buf *dbuf,
				  unsigned long size, int write)
{
	struct vb2_ion_buf *buf;
	struct dma_buf_attachment *attachment;

	if (dbuf->size < size)
		return ERR_PTR(-EFAULT);

	buf = kzalloc(sizeof *buf, GFP_KERNEL);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	buf->ctx = alloc_ctx;
	/* create attachment for the dmabuf with the user device */
	attachment = dma_buf_attach(dbuf, buf->ctx->dev);
	if (IS_ERR(attachment)) {
		pr_err("failed to attach dmabuf\n");
		kfree(buf);
		return attachment;
	}

	buf->direction = write ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
	buf->size = size;
	buf->dma_buf = dbuf;
	buf->attachment = attachment;

	return buf;
}

const struct vb2_mem_ops vb2_ion_memops = {
	.alloc		= vb2_ion_alloc,
	.put		= vb2_ion_put,
	.cookie		= vb2_ion_cookie,
	.vaddr		= vb2_ion_vaddr,
	.mmap		= vb2_ion_mmap,
	.map_dmabuf	= vb2_ion_map_dmabuf,
	.unmap_dmabuf	= vb2_ion_unmap_dmabuf,
	.attach_dmabuf	= vb2_ion_attach_dmabuf,
	.detach_dmabuf	= vb2_ion_detach_dmabuf,
	.num_users	= vb2_ion_num_users,
};
EXPORT_SYMBOL_GPL(vb2_ion_memops);

void vb2_ion_sync_for_device(void *cookie, off_t offset, size_t size,
						enum dma_data_direction dir)
{
	struct vb2_ion_cookie *vb2cookie = cookie;
	struct vb2_ion_context *ctx =
			container_of(cookie, struct vb2_ion_buf, cookie)->ctx;
	struct scatterlist *sg;

	for (sg = vb2cookie->sgt->sgl; sg != NULL; sg = sg_next(sg)) {
		if (sg_dma_len(sg) <= offset)
			offset -= sg_dma_len(sg);
		else
			break;
	}

	if (WARN_ON(sg == NULL)) /* Too big offset */
		return;

	while ((size != 0) && (sg != NULL)) {
		size_t sg_size;

		sg_size = min_t(size_t, size, sg_dma_len(sg) - offset);
		dma_map_page(ctx->dev, sg_page(sg) + PFN_DOWN(offset),
				offset & ~PAGE_MASK, sg_size, dir);

		offset = 0;
		size -= sg_size;
		sg = sg_next(sg);
	}

	WARN_ON(size != 0); /* Too big size */
}
EXPORT_SYMBOL_GPL(vb2_ion_sync_for_device);

void vb2_ion_sync_for_cpu(void *cookie, off_t offset, size_t size,
						enum dma_data_direction dir)
{
	struct vb2_ion_cookie *vb2cookie = cookie;
	struct vb2_ion_context *ctx =
			container_of(cookie, struct vb2_ion_buf, cookie)->ctx;
	struct scatterlist *sg;

	for (sg = vb2cookie->sgt->sgl; sg != NULL; sg = sg_next(sg)) {
		if (sg_dma_len(sg) <= offset)
			offset -= sg_dma_len(sg);
		else
			break;
	}

	if (WARN_ON(sg == NULL)) /* Too big offset */
		return;

	while ((size != 0) && (sg != NULL)) {
		size_t sg_size;

		sg_size = min_t(size_t, size, sg_dma_len(sg) - offset);
		dma_unmap_page(ctx->dev, sg_phys(sg) + offset, sg_size, dir);

		offset = 0;
		size -= sg_size;
		sg = sg_next(sg);
	}

	WARN_ON(size != 0); /* Too big size */
}
EXPORT_SYMBOL_GPL(vb2_ion_sync_for_cpu);

static void flush_entire_dcache(void *p)
{
	flush_cache_all();
}

int vb2_ion_cache_flush(struct vb2_buffer *vb, u32 num_planes)
{
	int i;
	size_t sz = 0;
	struct vb2_ion_buf *buf;
	enum dma_data_direction dir;

	for (i = 0; i < num_planes; i++) {
		buf = vb->planes[i].mem_priv;
		if (!buf->cached)
			continue;

		sz += buf->size;
	}

	if (sz >= SZ_2M) { /* performance tuning */
		smp_call_function(&flush_entire_dcache, NULL, 1);
		outer_flush_all();
		return 0;
	}

	dir = V4L2_TYPE_IS_OUTPUT(vb->v4l2_buf.type) ?
					DMA_TO_DEVICE : DMA_FROM_DEVICE;

	while (num_planes-- > 0) {
		struct scatterlist *sg;
		off_t start_off;
		int num_pages;
		struct page *page;

		buf = vb->planes[num_planes].mem_priv;
		if (!buf->cached)
			continue;

		sg = buf->cookie.sgt->sgl;
		start_off = buf->cookie.offset;
		sz = buf->size;

		while (sg && (start_off >= sg_dma_len(sg))) {
			start_off -= sg_dma_len(sg);
			sg = sg_next(sg);
		}

		for (; (sg != NULL) && (sz > 0); sg = sg_next(sg)) {
			int pages_to_pass;
			void *addr;
			bool kmapped = false;

			start_off += sg->offset;

			num_pages = PFN_DOWN(
				PAGE_ALIGN(sg_dma_len(sg) + sg->offset));
			pages_to_pass = PFN_DOWN(
					round_down(start_off, PAGE_SIZE));
			num_pages -= pages_to_pass;

			page = sg_page(sg) + pages_to_pass;
			start_off &= ~PAGE_MASK;

			for (i = 0; (i < num_pages) && (sz > 0); i++, page++) {
				size_t szflush;

				szflush = min_t(size_t,
						PAGE_SIZE - start_off, sz);

				addr = page_address(page);
				if (!addr) {
					addr = kmap_atomic(page);
					kmapped = true;
				}

				dmac_map_area(addr + start_off, szflush, dir);

				if (dir == DMA_TO_DEVICE)
					outer_clean_range(
						page_to_phys(page) + start_off,
						page_to_phys(page) + szflush);
				else
					outer_inv_range(
						page_to_phys(page) + start_off,
						page_to_phys(page) + szflush);

				if (kmapped) {
					kunmap_atomic(addr);
					kmapped = false;
				}

				sz -= szflush;
				start_off = 0;
			}
		}

		WARN_ON(sz > 0);
	}

	return 0;
}

int vb2_ion_cache_inv(struct vb2_buffer *vb, u32 num_planes)
{
	struct vb2_ion_buf *buf;
	int i;

	for (i = 0; i < num_planes; i++) {
		buf = vb->planes[i].mem_priv;
		if (!buf->cached)
			continue;

		vb2_ion_sync_for_device(&buf->cookie, buf->cookie.offset,
						buf->size, DMA_FROM_DEVICE);
	}

	return 0;
}

void vb2_ion_detach_iommu(void *alloc_ctx)
{
	struct vb2_ion_context *ctx = alloc_ctx;

	if (!ctx_iommu(ctx))
		return;

	mutex_lock(&ctx->lock);
	BUG_ON(ctx->iommu_active_cnt == 0);

	if (--ctx->iommu_active_cnt == 0 && !ctx->protected)
		iovmm_deactivate(ctx->dev);
	mutex_unlock(&ctx->lock);
}
EXPORT_SYMBOL_GPL(vb2_ion_detach_iommu);

int vb2_ion_attach_iommu(void *alloc_ctx)
{
	struct vb2_ion_context *ctx = alloc_ctx;
	int ret = 0;

	if (!ctx_iommu(ctx))
		return -ENOENT;

	mutex_lock(&ctx->lock);
	if (ctx->iommu_active_cnt == 0 && !ctx->protected)
		ret = iovmm_activate(ctx->dev);
	if (!ret)
		ctx->iommu_active_cnt++;
	mutex_unlock(&ctx->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(vb2_ion_attach_iommu);

MODULE_AUTHOR("Jonghun,	Han <jonghun.han@samsung.com>");
MODULE_DESCRIPTION("Android ION allocator handling routines for videobuf2");
MODULE_LICENSE("GPL");
