#ifndef _MEDIA_VIDEOBUF2_ION_NXP_H
#define _MEDIA_VIDEOBUF2_ION_NXP_H

#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/err.h>

/* flags to vb2_ion_create_context
 * 
 * bit 0 ~ ION_NUM_HEAPS: ion heap flags
 * bit ION_NUM_HEAPS+1 ~ 20: non-ion flags (cached, iommu)
 * bit 21 ~ BITS_PER_LONG - 1: ion specific flags
 */

/* allocate physical continuous memory */
#define VB2ION_CTX_PHCONTIG ION_HEAP_CARVEOUT_MASK
/* allocate virtual memory */
#define VB2ION_CTX_VMCONTIG ION_HEAP_SYSTEM_MASK
/* use iommu */
#define VB2ION_CTX_IOMMU    (1 << (ION_NUM_HEAPS + 1)) /* not used */
/* use noncached memory */
#define VB2ION_CTX_UNCACHED (1 << (ION_NUM_HEAPS + 2))

#define VB2ION_CTX_MASK_ION (~((1 << (BITS_PER_LONG - 12)) - 1) \
                            | ((1 << ION_NUM_HEAPS) - 1))

/* mask out all non-ion flags */
#define ion_heapflag(flag)  (flag & VB2ION_CTX_MASK_ION)

struct device;
struct vb2_buffer;

void *vb2_ion_create_context(struct device *dev, size_t alignment, long flags);
void vb2_ion_destroy_context(void *ctx);

/*
 * data type of the cookie returned by vb2_plane_cookie()
 */
struct vb2_ion_cookie {
    dma_addr_t ioaddr;
    struct sg_table *sgt;
    off_t offset;
};

/*
 * help functions
 */
int vb2_ion_set_alignement(void *ctx, size_t alignment);
void *vb2_ion_private_vaddr(void *cookie);
void *vb2_ion_private_alloc(void *alloc_ctx, size_t size);
void vb2_ion_private_free(void *cookie);

/*
 * inline functions
 */
static inline int vb2_ion_phys_address(void *cookie, phys_addr_t *phys_addr)
{
    struct vb2_ion_cookie *vb2cookie = cookie;

    if (WARN_ON(!phys_addr || IS_ERR_OR_NULL(cookie)))
        return -EINVAL;

    if (vb2cookie->sgt->nents == 1)
        *phys_addr = sg_phys(vb2cookie->sgt->sgl) + vb2cookie->offset;
    else
        return -EINVAL;

    return 0;
}

static inline int vb2_ion_dma_address(void *cookie, dma_addr_t *dma_addr)
{
    struct vb2_ion_cookie *vb2cookie = cookie;

    if (WARN_ON(!dma_addr || IS_ERR_OR_NULL(cookie)))
        return -EINVAL;

    if (vb2cookie->ioaddr == 0)
        return vb2_ion_phys_address(cookie, (phys_addr_t *)dma_addr);

    *dma_addr = vb2cookie->ioaddr;

    return 0;
}

static inline struct scatterlist *vb2_ion_get_sg(void *cookie, int *nents)
{
    struct vb2_ion_cookie *vb2cookie = cookie;

    if (WARN_ON(!nents || IS_ERR_OR_NULL(cookie)))
        return NULL;

    *nents = vb2cookie->sgt->nents;
    return vb2cookie->sgt->sgl;
}

extern const struct vb2_mem_ops vb2_ion_memops;
struct ion_device *get_global_ion_device(void); /* drivers/gpu/ion/nexell/nxp-ion.c */

#endif
