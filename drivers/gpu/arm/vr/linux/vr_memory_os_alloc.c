/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#include "vr_osk.h"
#include "vr_memory.h"
#include "vr_memory_os_alloc.h"
#include "vr_kernel_linux.h"

/* Minimum size of allocator page pool */
#define VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES (VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB * 256)
#define VR_OS_MEMORY_POOL_TRIM_JIFFIES (10 * CONFIG_HZ) /* Default to 10s */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int vr_mem_os_shrink(int nr_to_scan, gfp_t gfp_mask);
#else
static int vr_mem_os_shrink(struct shrinker *shrinker, int nr_to_scan, gfp_t gfp_mask);
#endif
#else
static int vr_mem_os_shrink(struct shrinker *shrinker, struct shrink_control *sc);
#endif
static void vr_mem_os_trim_pool(struct work_struct *work);

static struct vr_mem_os_allocator {
	spinlock_t pool_lock;
	struct list_head pool_pages;
	size_t pool_count;

	atomic_t allocated_pages;
	size_t allocation_limit;

	struct shrinker shrinker;
	struct delayed_work timed_shrinker;
	struct workqueue_struct *wq;
} vr_mem_os_allocator = {
	.pool_lock = __SPIN_LOCK_UNLOCKED(pool_lock),
	.pool_pages = LIST_HEAD_INIT(vr_mem_os_allocator.pool_pages),
	.pool_count = 0,

	.allocated_pages = ATOMIC_INIT(0),
	.allocation_limit = 0,

	.shrinker.shrink = vr_mem_os_shrink,
	.shrinker.seeks = DEFAULT_SEEKS,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	.timed_shrinker = __DELAYED_WORK_INITIALIZER(vr_mem_os_allocator.timed_shrinker, vr_mem_os_trim_pool, TIMER_DEFERRABLE),
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	.timed_shrinker = __DEFERRED_WORK_INITIALIZER(vr_mem_os_allocator.timed_shrinker, vr_mem_os_trim_pool),
#else
	.timed_shrinker = __DELAYED_WORK_INITIALIZER(vr_mem_os_allocator.timed_shrinker, vr_mem_os_trim_pool),
#endif
};

static void vr_mem_os_free(vr_mem_allocation *descriptor)
{
	LIST_HEAD(pages);

	VR_DEBUG_ASSERT(VR_MEM_OS == descriptor->type);

	atomic_sub(descriptor->os_mem.count, &vr_mem_os_allocator.allocated_pages);

	/* Put pages on pool. */
	list_cut_position(&pages, &descriptor->os_mem.pages, descriptor->os_mem.pages.prev);

	spin_lock(&vr_mem_os_allocator.pool_lock);

	list_splice(&pages, &vr_mem_os_allocator.pool_pages);
	vr_mem_os_allocator.pool_count += descriptor->os_mem.count;

	spin_unlock(&vr_mem_os_allocator.pool_lock);

	if (VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES < vr_mem_os_allocator.pool_count) {
		VR_DEBUG_PRINT(5, ("OS Mem: Starting pool trim timer %u\n", vr_mem_os_allocator.pool_count));
		queue_delayed_work(vr_mem_os_allocator.wq, &vr_mem_os_allocator.timed_shrinker, VR_OS_MEMORY_POOL_TRIM_JIFFIES);
	}
}

static int vr_mem_os_alloc_pages(vr_mem_allocation *descriptor, u32 size)
{
	struct page *new_page, *tmp;
	LIST_HEAD(pages);
	size_t page_count = PAGE_ALIGN(size) / _VR_OSK_VR_PAGE_SIZE;
	size_t remaining = page_count;
	u32 i;

	VR_DEBUG_ASSERT_POINTER(descriptor);
	VR_DEBUG_ASSERT(VR_MEM_OS == descriptor->type);

	INIT_LIST_HEAD(&descriptor->os_mem.pages);
	descriptor->os_mem.count = page_count;

	/* Grab pages from pool. */
	{
		size_t pool_pages;
		spin_lock(&vr_mem_os_allocator.pool_lock);
		pool_pages = min(remaining, vr_mem_os_allocator.pool_count);
		for (i = pool_pages; i > 0; i--) {
			BUG_ON(list_empty(&vr_mem_os_allocator.pool_pages));
			list_move(vr_mem_os_allocator.pool_pages.next, &pages);
		}
		vr_mem_os_allocator.pool_count -= pool_pages;
		remaining -= pool_pages;
		spin_unlock(&vr_mem_os_allocator.pool_lock);
	}

	/* Process pages from pool. */
	i = 0;
	list_for_each_entry_safe(new_page, tmp, &pages, lru) {
		BUG_ON(NULL == new_page);

		list_move_tail(&new_page->lru, &descriptor->os_mem.pages);
	}

	/* Allocate new pages, if needed. */
	for (i = 0; i < remaining; i++) {
		dma_addr_t dma_addr;

		new_page = alloc_page(GFP_HIGHUSER | __GFP_ZERO | __GFP_REPEAT | __GFP_NOWARN | __GFP_COLD);

		if (unlikely(NULL == new_page)) {
			/* Calculate the number of pages actually allocated, and free them. */
			descriptor->os_mem.count = (page_count - remaining) + i;
			atomic_add(descriptor->os_mem.count, &vr_mem_os_allocator.allocated_pages);
			vr_mem_os_free(descriptor);
			return -ENOMEM;
		}

		/* Ensure page is flushed from CPU caches. */
		dma_addr = dma_map_page(&vr_platform_device->dev, new_page,
		                        0, _VR_OSK_VR_PAGE_SIZE, DMA_TO_DEVICE);

		/* Store page phys addr */
		SetPagePrivate(new_page);
		set_page_private(new_page, dma_addr);

		list_add_tail(&new_page->lru, &descriptor->os_mem.pages);
	}

	atomic_add(page_count, &vr_mem_os_allocator.allocated_pages);

	if (VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES > vr_mem_os_allocator.pool_count) {
		VR_DEBUG_PRINT(4, ("OS Mem: Stopping pool trim timer, only %u pages on pool\n", vr_mem_os_allocator.pool_count));
		cancel_delayed_work(&vr_mem_os_allocator.timed_shrinker);
	}

	return 0;
}

static int vr_mem_os_vr_map(vr_mem_allocation *descriptor, struct vr_session_data *session)
{
	struct vr_page_directory *pagedir = session->page_directory;
	struct page *page;
	_vr_osk_errcode_t err;
	u32 virt = descriptor->vr_mapping.addr;
	u32 prop = descriptor->vr_mapping.properties;

	VR_DEBUG_ASSERT(VR_MEM_OS == descriptor->type);

	err = vr_mem_vr_map_prepare(descriptor);
	if (_VR_OSK_ERR_OK != err) {
		return -ENOMEM;
	}

	list_for_each_entry(page, &descriptor->os_mem.pages, lru) {
		u32 phys = page_private(page);
		vr_mmu_pagedir_update(pagedir, virt, phys, VR_MMU_PAGE_SIZE, prop);
		virt += VR_MMU_PAGE_SIZE;
	}

	return 0;
}

static void vr_mem_os_vr_unmap(struct vr_session_data *session, vr_mem_allocation *descriptor)
{
	vr_mem_vr_map_free(descriptor);
}

static int vr_mem_os_cpu_map(vr_mem_allocation *descriptor, struct vm_area_struct *vma)
{
	struct page *page;
	int ret;
	unsigned long addr = vma->vm_start;

	list_for_each_entry(page, &descriptor->os_mem.pages, lru) {
		/* We should use vm_insert_page, but it does a dcache
		 * flush which makes it way slower than remap_pfn_range or vm_insert_pfn.
		ret = vm_insert_page(vma, addr, page);
		*/
		ret = vm_insert_pfn(vma, addr, page_to_pfn(page));

		if (unlikely(0 != ret)) {
			return -EFAULT;
		}
		addr += _VR_OSK_VR_PAGE_SIZE;
	}

	return 0;
}

vr_mem_allocation *vr_mem_os_alloc(u32 vr_addr, u32 size, struct vm_area_struct *vma, struct vr_session_data *session)
{
	vr_mem_allocation *descriptor;
	int err;

	if (atomic_read(&vr_mem_os_allocator.allocated_pages) * _VR_OSK_VR_PAGE_SIZE + size > vr_mem_os_allocator.allocation_limit) {
		VR_DEBUG_PRINT(2, ("Vr Mem: Unable to allocate %u bytes. Currently allocated: %lu, max limit %lu\n",
		                     size,
		                     atomic_read(&vr_mem_os_allocator.allocated_pages) * _VR_OSK_VR_PAGE_SIZE,
		                     vr_mem_os_allocator.allocation_limit));
		return NULL;
	}

	descriptor = vr_mem_descriptor_create(session, VR_MEM_OS);
	if (NULL == descriptor) return NULL;

	descriptor->vr_mapping.addr = vr_addr;
	descriptor->size = size;
	descriptor->cpu_mapping.addr = (void __user*)vma->vm_start;
	descriptor->cpu_mapping.ref = 1;

	if (VM_SHARED == (VM_SHARED & vma->vm_flags)) {
		descriptor->vr_mapping.properties = VR_MMU_FLAGS_DEFAULT;
	} else {
		/* Cached Vr memory mapping */
		descriptor->vr_mapping.properties = VR_MMU_FLAGS_FORCE_GP_READ_ALLOCATE;
		vma->vm_flags |= VM_SHARED;
	}

	err = vr_mem_os_alloc_pages(descriptor, size); /* Allocate pages */
	if (0 != err) goto alloc_failed;

	/* Take session memory lock */
	_vr_osk_mutex_wait(session->memory_lock);

	err = vr_mem_os_vr_map(descriptor, session); /* Map on Vr */
	if (0 != err) goto vr_map_failed;

	_vr_osk_mutex_signal(session->memory_lock);

	err = vr_mem_os_cpu_map(descriptor, vma); /* Map on CPU */
	if (0 != err) goto cpu_map_failed;

	return descriptor;

cpu_map_failed:
	vr_mem_os_vr_unmap(session, descriptor);
vr_map_failed:
	_vr_osk_mutex_signal(session->memory_lock);
	vr_mem_os_free(descriptor);
alloc_failed:
	vr_mem_descriptor_destroy(descriptor);
	VR_DEBUG_PRINT(2, ("OS allocator: Failed to allocate memory (%d)\n", err));
	return NULL;
}

void vr_mem_os_release(vr_mem_allocation *descriptor)
{
	struct vr_session_data *session = descriptor->session;

	/* Unmap the memory from the vr virtual address space. */
	vr_mem_os_vr_unmap(session, descriptor);

	/* Free pages */
	vr_mem_os_free(descriptor);
}


#define VR_MEM_OS_PAGE_TABLE_PAGE_POOL_SIZE 128
static struct {
	struct {
		u32 phys;
		vr_io_address mapping;
	} page[VR_MEM_OS_PAGE_TABLE_PAGE_POOL_SIZE];
	u32 count;
	spinlock_t lock;
} vr_mem_page_table_page_pool = {
	.count = 0,
	.lock = __SPIN_LOCK_UNLOCKED(pool_lock),
};

_vr_osk_errcode_t vr_mem_os_get_table_page(u32 *phys, vr_io_address *mapping)
{
	_vr_osk_errcode_t ret = _VR_OSK_ERR_NOMEM;

	spin_lock(&vr_mem_page_table_page_pool.lock);
	if (0 < vr_mem_page_table_page_pool.count) {
		u32 i = --vr_mem_page_table_page_pool.count;
		*phys = vr_mem_page_table_page_pool.page[i].phys;
		*mapping = vr_mem_page_table_page_pool.page[i].mapping;

		ret = _VR_OSK_ERR_OK;
	}
	spin_unlock(&vr_mem_page_table_page_pool.lock);

	if (_VR_OSK_ERR_OK != ret) {
		*mapping = dma_alloc_writecombine(&vr_platform_device->dev, _VR_OSK_VR_PAGE_SIZE, phys, GFP_KERNEL);
		if (NULL != *mapping) {
			ret = _VR_OSK_ERR_OK;
		}
	}

	return ret;
}

void vr_mem_os_release_table_page(u32 phys, void *virt)
{
	spin_lock(&vr_mem_page_table_page_pool.lock);
	if (VR_MEM_OS_PAGE_TABLE_PAGE_POOL_SIZE > vr_mem_page_table_page_pool.count) {
		u32 i = vr_mem_page_table_page_pool.count;
		vr_mem_page_table_page_pool.page[i].phys = phys;
		vr_mem_page_table_page_pool.page[i].mapping = virt;

		++vr_mem_page_table_page_pool.count;

		spin_unlock(&vr_mem_page_table_page_pool.lock);
	} else {
		spin_unlock(&vr_mem_page_table_page_pool.lock);

		dma_free_writecombine(&vr_platform_device->dev, _VR_OSK_VR_PAGE_SIZE, virt, phys);
	}
}

static void vr_mem_os_free_page(struct page *page)
{
	BUG_ON(page_count(page) != 1);

	dma_unmap_page(&vr_platform_device->dev, page_private(page),
	               _VR_OSK_VR_PAGE_SIZE, DMA_TO_DEVICE);

	ClearPagePrivate(page);

	__free_page(page);
}

/* The maximum number of page table pool pages to free in one go. */
#define VR_MEM_OS_CHUNK_TO_FREE 64UL

/* Free a certain number of pages from the page table page pool.
 * The pool lock must be held when calling the function, and the lock will be
 * released before returning.
 */
static void vr_mem_os_page_table_pool_free(size_t nr_to_free)
{
	u32 phys_arr[VR_MEM_OS_CHUNK_TO_FREE];
	void *virt_arr[VR_MEM_OS_CHUNK_TO_FREE];
	u32 i;

	VR_DEBUG_ASSERT(nr_to_free <= VR_MEM_OS_CHUNK_TO_FREE);

	/* Remove nr_to_free pages from the pool and store them locally on stack. */
	for (i = 0; i < nr_to_free; i++) {
		u32 pool_index = vr_mem_page_table_page_pool.count - i - 1;

		phys_arr[i] = vr_mem_page_table_page_pool.page[pool_index].phys;
		virt_arr[i] = vr_mem_page_table_page_pool.page[pool_index].mapping;
	}

	vr_mem_page_table_page_pool.count -= nr_to_free;

	spin_unlock(&vr_mem_page_table_page_pool.lock);

	/* After releasing the spinlock: free the pages we removed from the pool. */
	for (i = 0; i < nr_to_free; i++) {
		dma_free_writecombine(&vr_platform_device->dev, _VR_OSK_VR_PAGE_SIZE, virt_arr[i], phys_arr[i]);
	}
}

static void vr_mem_os_trim_page_table_page_pool(void)
{
	size_t nr_to_free = 0;
	size_t nr_to_keep;

	/* Keep 2 page table pages for each 1024 pages in the page cache. */
	nr_to_keep = vr_mem_os_allocator.pool_count / 512;
	/* And a minimum of eight pages, to accomodate new sessions. */
	nr_to_keep += 8;

	if (0 == spin_trylock(&vr_mem_page_table_page_pool.lock)) return;

	if (nr_to_keep < vr_mem_page_table_page_pool.count) {
		nr_to_free = vr_mem_page_table_page_pool.count - nr_to_keep;
		nr_to_free = min((size_t)VR_MEM_OS_CHUNK_TO_FREE, nr_to_free);
	}

	/* Pool lock will be released by the callee. */
	vr_mem_os_page_table_pool_free(nr_to_free);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int vr_mem_os_shrink(int nr_to_scan, gfp_t gfp_mask)
#else
static int vr_mem_os_shrink(struct shrinker *shrinker, int nr_to_scan, gfp_t gfp_mask)
#endif
#else
static int vr_mem_os_shrink(struct shrinker *shrinker, struct shrink_control *sc)
#endif
{
	struct page *page, *tmp;
	unsigned long flags;
	struct list_head *le, pages;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
	int nr = nr_to_scan;
#else
	int nr = sc->nr_to_scan;
#endif

	if (0 == nr) {
		return vr_mem_os_allocator.pool_count + vr_mem_page_table_page_pool.count;
	}

	if (0 == vr_mem_os_allocator.pool_count) {
		/* No pages availble */
		return 0;
	}

	if (0 == spin_trylock_irqsave(&vr_mem_os_allocator.pool_lock, flags)) {
		/* Not able to lock. */
		return -1;
	}

	/* Release from general page pool */
	nr = min((size_t)nr, vr_mem_os_allocator.pool_count);
	vr_mem_os_allocator.pool_count -= nr;
	list_for_each(le, &vr_mem_os_allocator.pool_pages) {
		--nr;
		if (0 == nr) break;
	}
	list_cut_position(&pages, &vr_mem_os_allocator.pool_pages, le);
	spin_unlock_irqrestore(&vr_mem_os_allocator.pool_lock, flags);

	list_for_each_entry_safe(page, tmp, &pages, lru) {
		vr_mem_os_free_page(page);
	}

	/* Release some pages from page table page pool */
	vr_mem_os_trim_page_table_page_pool();

	if (VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES > vr_mem_os_allocator.pool_count) {
		/* Pools are empty, stop timer */
		VR_DEBUG_PRINT(5, ("Stopping timer, only %u pages on pool\n", vr_mem_os_allocator.pool_count));
		cancel_delayed_work(&vr_mem_os_allocator.timed_shrinker);
	}

	return vr_mem_os_allocator.pool_count + vr_mem_page_table_page_pool.count;
}

static void vr_mem_os_trim_pool(struct work_struct *data)
{
	struct page *page, *tmp;
	struct list_head *le;
	LIST_HEAD(pages);
	size_t nr_to_free;

	VR_IGNORE(data);

	VR_DEBUG_PRINT(3, ("OS Mem: Trimming pool %u\n", vr_mem_os_allocator.pool_count));

	/* Release from general page pool */
	spin_lock(&vr_mem_os_allocator.pool_lock);
	if (VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES < vr_mem_os_allocator.pool_count) {
		size_t count = vr_mem_os_allocator.pool_count - VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES;
		/* Free half the pages on the pool above the static limit. Or 64 pages, 256KB. */
		nr_to_free = max(count / 2, (size_t)64);

		vr_mem_os_allocator.pool_count -= nr_to_free;
		list_for_each(le, &vr_mem_os_allocator.pool_pages) {
			--nr_to_free;
			if (0 == nr_to_free) break;
		}
		list_cut_position(&pages, &vr_mem_os_allocator.pool_pages, le);
	}
	spin_unlock(&vr_mem_os_allocator.pool_lock);

	list_for_each_entry_safe(page, tmp, &pages, lru) {
		vr_mem_os_free_page(page);
	}

	/* Release some pages from page table page pool */
	vr_mem_os_trim_page_table_page_pool();

	if (VR_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_PAGES < vr_mem_os_allocator.pool_count) {
		VR_DEBUG_PRINT(4, ("OS Mem: Starting pool trim timer %u\n", vr_mem_os_allocator.pool_count));
		queue_delayed_work(vr_mem_os_allocator.wq, &vr_mem_os_allocator.timed_shrinker, VR_OS_MEMORY_POOL_TRIM_JIFFIES);
	}
}

_vr_osk_errcode_t vr_mem_os_init(void)
{
	vr_mem_os_allocator.wq = alloc_workqueue("vr-mem", WQ_UNBOUND, 1);
	if (NULL == vr_mem_os_allocator.wq) {
		return _VR_OSK_ERR_NOMEM;
	}

	register_shrinker(&vr_mem_os_allocator.shrinker);

	return _VR_OSK_ERR_OK;
}

void vr_mem_os_term(void)
{
	struct page *page, *tmp;

	unregister_shrinker(&vr_mem_os_allocator.shrinker);
	cancel_delayed_work_sync(&vr_mem_os_allocator.timed_shrinker);
	destroy_workqueue(vr_mem_os_allocator.wq);

	spin_lock(&vr_mem_os_allocator.pool_lock);
	list_for_each_entry_safe(page, tmp, &vr_mem_os_allocator.pool_pages, lru) {
		vr_mem_os_free_page(page);

		--vr_mem_os_allocator.pool_count;
	}
	BUG_ON(vr_mem_os_allocator.pool_count);
	spin_unlock(&vr_mem_os_allocator.pool_lock);

	/* Release from page table page pool */
	do {
		u32 nr_to_free;

		spin_lock(&vr_mem_page_table_page_pool.lock);

		nr_to_free = min((size_t)VR_MEM_OS_CHUNK_TO_FREE, vr_mem_page_table_page_pool.count);

		/* Pool lock will be released by the callee. */
		vr_mem_os_page_table_pool_free(nr_to_free);
	} while (0 != vr_mem_page_table_page_pool.count);
}

_vr_osk_errcode_t vr_memory_core_resource_os_memory(u32 size)
{
	vr_mem_os_allocator.allocation_limit = size;

	VR_SUCCESS;
}

u32 vr_mem_os_stat(void)
{
	return atomic_read(&vr_mem_os_allocator.allocated_pages) * _VR_OSK_VR_PAGE_SIZE;
}
