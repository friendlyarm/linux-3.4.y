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
#include <media/videobuf2-ion-nxp.h>

#include <asm/cacheflush.h>

struct vb2_ion_context {
    struct device       *dev;
    struct ion_client   *client;
    unsigned long        alignment;
    long                 flags;
};

struct vb2_ion_buf {
    struct vb2_ion_context      *ctx;
    struct vb2_vmarea_handler    handler;
    struct ion_handle           *handle;
    struct dma_buf              *dma_buf;
    struct dma_buf_attachment   *attachment;
    enum dma_data_direction      direction;
    void                        *kva;
    unsigned long                size;
    atomic_t                     ref;
    /* bool                      cached; */
    struct vb2_ion_cookie        cookie;
};

/*
 * core functions
 */
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
    struct ion_device *ion_dev = get_global_ion_device();

    if (!ion_dev) {
        pr_err("%s error: can't get global ion device!!!\n", __func__);
        return ERR_PTR(-EINVAL);
    }

    /*
     * ion_client_create() expects the current thread to be a kernel thread
     * to create a new ion_client
     */
    WARN_ON(!(current->group_leader->flags & PF_KTHREAD));

    /*
     * only support continuous memory
     */
    if (flags & VB2ION_CTX_VMCONTIG) {
        pr_err("%s error: not support vmalloc ion context\n", __func__);
        return ERR_PTR(-EINVAL);
    }

    ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
    if (!ctx) {
        pr_err("%s error: fail to kzalloc size: %d\n", __func__, sizeof(*ctx));
        return ERR_PTR(-ENOMEM);
    }

    ctx->dev    = dev;
    ctx->client = ion_client_create(ion_dev, dev_name(dev));
    if (IS_ERR(ctx->client)) {
        void *retp = ctx->client;
        kfree(ctx);
        return retp;
    }

    vb2_ion_set_alignment(ctx, alignment);

    return ctx;
}
EXPORT_SYMBOL(vb2_ion_create_context);

void vb2_ion_destroy_context(void *ctx)
{
    struct vb2_ion_context *vb2ctx = ctx;
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
    if (!buf) {
        pr_err("%s error: fail to kzalloc size(%d)\n", __func__, sizeof(*buf));
        return ERR_PTR(-ENOMEM);
    }

    size = PAGE_ALIGN(size);

    buf->handle = ion_alloc(ctx->client, size, ctx->alignment, flags, flags);
    if (IS_ERR(buf->handle)) {
        ret = -ENOMEM;
        goto err_alloc;
    }

    buf->cookie.sgt = ion_sg_table(ctx->client, buf->handle);

    buf->ctx  = ctx;
    buf->size = size;

    buf->kva  = ion_map_kernel(ctx->client, buf->handle);
    if (IS_ERR(buf->kva)) {
        ret = PTR_ERR(buf->kva);
        buf->kva = NULL;
        goto err_map_kernel;
    }

    return &buf->cookie;

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

    ion_unmap_kernel(ctx->client, buf->handle);
    ion_free(ctx->client, buf->handle);
    kfree(buf);
}

/*
 * vb2_mem_ops
 */
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
    buf->handler.put      = vb2_ion_put;
    buf->handler.arg      = buf;
    atomic_set(&buf->ref, 1);

    return buf;
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
        dma_buf_end_cpu_access(buf->dma_buf,
                0, buf->size, buf->direction);

    return buf->kva;
}

static void *vb2_ion_attach_dmabuf(void *alloc_ctx, struct dma_buf *dbuf,
                    unsigned long size, int write)
{
    struct vb2_ion_buf *buf;
    struct dma_buf_attachment *attachment;

    if (dbuf->size < size) {
        printk(KERN_ERR "%s: size error(buf->size %lu/request size %lu)\n",
                __func__, (unsigned long)dbuf->size, size);
        return ERR_PTR(-EFAULT);
    }

    buf = kzalloc(sizeof(*buf), GFP_KERNEL);
    if (!buf) {
        printk(KERN_ERR "%s: error line %d\n", __func__, __LINE__);
        return ERR_PTR(-ENOMEM);
    }

    buf->ctx = alloc_ctx;

    attachment = dma_buf_attach(dbuf, buf->ctx->dev);
    if (IS_ERR(attachment)) {
        printk(KERN_ERR "%s error: fail to attach dmabuf\n", __func__);
        kfree(buf);
        return attachment;
    }

    buf->direction = write ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
    buf->size      = size;
    buf->dma_buf   = dbuf;
    buf->attachment = attachment;

    return buf;
}

static void vb2_ion_detach_dmabuf(void *buf_priv)
{
    struct vb2_ion_buf *buf = buf_priv;

    if (buf->kva != NULL) {
        dma_buf_kunmap(buf->dma_buf, 0, buf->kva);
        dma_buf_end_cpu_access(buf->dma_buf, 0, buf->size, 0);
    }

    dma_buf_detach(buf->dma_buf, buf->attachment);
    kfree(buf);
}

static int vb2_ion_map_dmabuf(void *buf_priv)
{
    struct vb2_ion_buf *buf = buf_priv;

    if (WARN_ON(!buf->attachment)) {
        pr_err("%s error: trying to map a non attached buffer\n", __func__);
        return -EINVAL;
    }

    if (WARN_ON(buf->cookie.sgt)) {
        pr_err("%s error: dmabuf buffer is already mapped\n", __func__);
        return 0;
    }

    buf->cookie.sgt = dma_buf_map_attachment(buf->attachment,
            buf->direction);
    if (IS_ERR_OR_NULL(buf->cookie.sgt)) {
        pr_err("%s error: fail to get dmabuf scatterlist\n", __func__);
        return -EINVAL;
    }

    buf->cookie.offset = 0;

    return 0;
}

static void vb2_ion_unmap_dmabuf(void *buf_priv)
{
    struct vb2_ion_buf *buf = buf_priv;

    if (WARN_ON(!buf)) {
        pr_err("%s: NO BUF!!!\n", __func__);
        return;
    }

    if (WARN_ON(!buf->attachment)) {
        pr_err("%s error: trying to unmap a non attached buffer\n", __func__);
        return;
    }

    if (WARN_ON(!buf->cookie.sgt)) {
        pr_err("%s error: dmabuf buffer is already unmapped\n", __func__);
        return;
    }

    dma_buf_unmap_attachment(buf->attachment,
            buf->cookie.sgt, buf->direction);
    buf->cookie.sgt = NULL;
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
    unsigned long vm_start  = vma->vm_start;
    unsigned long vm_end    = vma->vm_end;
    struct scatterlist *sg  = buf->cookie.sgt->sgl;
    unsigned long size;
    int ret;

    if (buf->size < (vm_end - vm_start))
        return -EINVAL;

    /* always noncached buffer */
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

    size = min_t(size_t, vm_end - vm_start, sg_dma_len(sg));

    ret = remap_pfn_range(vma, vm_start, page_to_pfn(sg_page(sg)),
            size, vma->vm_page_prot);

    for (sg = sg_next(sg), vm_start +=size;
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

    vma->vm_flags       |= VM_DONTEXPAND;
    vma->vm_private_data = &buf->handler;
    vma->vm_ops          = &vb2_common_vm_ops;

    vma->vm_ops->open(vma);

    return ret;
}

const struct vb2_mem_ops vb2_ion_memops = {
    .alloc          = vb2_ion_alloc,
    .put            = vb2_ion_put,
    .cookie         = vb2_ion_cookie,
    .vaddr          = vb2_ion_vaddr,
    .attach_dmabuf  = vb2_ion_attach_dmabuf,
    .detach_dmabuf  = vb2_ion_detach_dmabuf,
    .map_dmabuf     = vb2_ion_map_dmabuf,
    .unmap_dmabuf   = vb2_ion_unmap_dmabuf,
    .num_users      = vb2_ion_num_users,
    .mmap           = vb2_ion_mmap,
};
EXPORT_SYMBOL(vb2_ion_memops);

MODULE_AUTHOR("swpark@nexell.co.kr");
MODULE_DESCRIPTION("NEXELL ION allocator handling routines for videobuf2");
MODULE_LICENSE("GPL");
