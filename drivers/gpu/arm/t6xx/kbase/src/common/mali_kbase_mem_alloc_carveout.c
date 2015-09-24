/*
 *
 * (C) COPYRIGHT 2010-2012 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */



/**
 * @file mali_kbase_mem.c
 * Base kernel memory APIs
 */
#include <kbase/src/common/mali_kbase.h>
#include <linux/highmem.h>
#include <linux/mempool.h>
#include <linux/mm.h>
#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <linux/memblock.h>
#include <linux/seq_file.h>

static unsigned long kbase_carveout_start_pfn = ~0UL;
static unsigned long kbase_carveout_end_pfn;
static LIST_HEAD(kbase_carveout_free_list);
static DEFINE_MUTEX(kbase_carveout_free_list_lock);
static unsigned int kbase_carveout_pages;
static atomic_t kbase_carveout_used_pages;
static atomic_t kbase_carveout_system_pages;

static struct page *kbase_carveout_get_page(void)
{
	struct page *p = NULL;

	mutex_lock(&kbase_carveout_free_list_lock);
	if (!list_empty(&kbase_carveout_free_list)) {
		p = list_first_entry(&kbase_carveout_free_list, struct page, lru);
		list_del(&p->lru);
		atomic_inc(&kbase_carveout_used_pages);
	}
	mutex_unlock(&kbase_carveout_free_list_lock);

	if (!p) {
		p = alloc_page(GFP_HIGHUSER);
		if (p) {
			atomic_inc(&kbase_carveout_system_pages);
		}
	}

	return p;
}

static void kbase_carveout_put_page(struct page *p)
{
	if (page_to_pfn(p) >= kbase_carveout_start_pfn &&
			page_to_pfn(p) <= kbase_carveout_end_pfn) {
		mutex_lock(&kbase_carveout_free_list_lock);
		list_add(&p->lru, &kbase_carveout_free_list);
		atomic_dec(&kbase_carveout_used_pages);
		mutex_unlock(&kbase_carveout_free_list_lock);
	} else {
		__free_page(p);
		atomic_dec(&kbase_carveout_system_pages);
	}
}

static int kbase_carveout_seq_show(struct seq_file *s, void *data)
{
	seq_printf(s, "carveout pages: %u\n", kbase_carveout_pages);
	seq_printf(s, "used carveout pages: %u\n",
			atomic_read(&kbase_carveout_used_pages));
	seq_printf(s, "used system pages: %u\n",
			atomic_read(&kbase_carveout_system_pages));
	return 0;
}

static int kbasep_carveout_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, kbase_carveout_seq_show, NULL);
}

static const struct file_operations kbase_carveout_debugfs_fops = {
	.open           = kbasep_carveout_debugfs_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = seq_release_private,
};

static int kbase_carveout_init(void)
{
	unsigned long pfn;

	mutex_lock(&kbase_carveout_free_list_lock);

	for (pfn = kbase_carveout_start_pfn; pfn <= kbase_carveout_end_pfn; pfn++) {
		struct page *p = pfn_to_page(pfn);
		list_add_tail(&p->lru, &kbase_carveout_free_list);
	}

	mutex_unlock(&kbase_carveout_free_list_lock);

	debugfs_create_file("kbase_carveout", S_IRUGO, NULL, NULL,
		    &kbase_carveout_debugfs_fops);

	return 0;
}
subsys_initcall(kbase_carveout_init);

int __init kbase_carveout_mem_reserve(phys_addr_t size)
{
	phys_addr_t mem;

	mem = memblock_alloc_base(size, PAGE_SIZE, MEMBLOCK_ALLOC_ANYWHERE);
	if (mem == 0) {
		pr_warning("%s: Failed to allocate %d for kbase carveout\n",
				__func__, size);
		return -ENOMEM;
	}

	kbase_carveout_start_pfn = page_to_pfn(phys_to_page(mem));
	kbase_carveout_end_pfn = page_to_pfn(phys_to_page(mem + size - 1));
	kbase_carveout_pages = kbase_carveout_end_pfn - kbase_carveout_start_pfn + 1;

	return 0;
}

STATIC int kbase_mem_allocator_shrink(struct shrinker *s, struct shrink_control *sc)
{
	kbase_mem_allocator * allocator;
	int i;
	int freed;

	allocator = container_of(s, kbase_mem_allocator, free_list_reclaimer);

	if (sc->nr_to_scan == 0)
		return atomic_read(&allocator->free_list_size);

	might_sleep();

	mutex_lock(&allocator->free_list_lock);
	i = MIN(atomic_read(&allocator->free_list_size), sc->nr_to_scan);
	freed = i;

	atomic_sub(i, &allocator->free_list_size);

	while (i--)
	{
		struct page * p;

		BUG_ON(list_empty(&allocator->free_list_head));
		p = list_first_entry(&allocator->free_list_head, struct page, lru);
		list_del(&p->lru);
		kbase_carveout_put_page(p);
	}
	mutex_unlock(&allocator->free_list_lock);
	return atomic_read(&allocator->free_list_size);
}

mali_error kbase_mem_allocator_init(kbase_mem_allocator * const allocator, unsigned int max_size)
{
	KBASE_DEBUG_ASSERT(NULL != allocator);

	INIT_LIST_HEAD(&allocator->free_list_head);

	mutex_init(&allocator->free_list_lock);

	atomic_set(&allocator->free_list_size, 0);

	allocator->free_list_max_size = max_size;
	allocator->free_list_reclaimer.shrink = kbase_mem_allocator_shrink;
	allocator->free_list_reclaimer.seeks = DEFAULT_SEEKS;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0) /* Kernel versions prior to 3.1 : struct shrinker does not define batch */
	allocator->free_list_reclaimer.batch = 0;
#endif

	register_shrinker(&allocator->free_list_reclaimer);

	return MALI_ERROR_NONE;
}

void kbase_mem_allocator_term(kbase_mem_allocator *allocator)
{
	KBASE_DEBUG_ASSERT(NULL != allocator);

	unregister_shrinker(&allocator->free_list_reclaimer);

	while (!list_empty(&allocator->free_list_head))
	{
		struct page * p;
		p = list_first_entry(&allocator->free_list_head, struct page, lru);
		list_del(&p->lru);
		kbase_carveout_put_page(p);
	}
	mutex_destroy(&allocator->free_list_lock);
}


mali_error kbase_mem_allocator_alloc(kbase_mem_allocator *allocator, u32 nr_pages, phys_addr_t *pages, int flags)
{
	struct page * p;
	void * mp;
	int i;
	int num_from_free_list;
	struct list_head from_free_list = LIST_HEAD_INIT(from_free_list);

	might_sleep();

	KBASE_DEBUG_ASSERT(NULL != allocator);

	/* take from the free list first */
	mutex_lock(&allocator->free_list_lock);
	num_from_free_list = MIN(nr_pages, atomic_read(&allocator->free_list_size));
	atomic_sub(num_from_free_list, &allocator->free_list_size);
	for (i = 0; i < num_from_free_list; i++)
	{
		BUG_ON(list_empty(&allocator->free_list_head));
		p = list_first_entry(&allocator->free_list_head, struct page, lru);
		list_move(&p->lru, &from_free_list);
	}
	mutex_unlock(&allocator->free_list_lock);
	i = 0;

	/* Allocate as many pages from the pool of already allocated pages. */
	list_for_each_entry(p, &from_free_list, lru)
	{
		pages[i] = PFN_PHYS(page_to_pfn(p));
		i++;
	}

	if (i == nr_pages)
		return MALI_ERROR_NONE;

	/* If not all pages were sourced from the pool, request new ones. */
	for (; i < nr_pages; i++)
	{
		p = kbase_carveout_get_page();
		if (NULL == p)
		{
			goto err_out_roll_back;
		}
		mp = kmap(p);
		if (NULL == mp)
		{
			kbase_carveout_put_page(p);
			goto err_out_roll_back;
		}
		memset(mp, 0x00, PAGE_SIZE); /* instead of __GFP_ZERO, so we can do cache maintenance */
		kbase_sync_to_memory(PFN_PHYS(page_to_pfn(p)), mp, PAGE_SIZE);
		kunmap(p);
		pages[i] = PFN_PHYS(page_to_pfn(p));
	}

	return MALI_ERROR_NONE;

err_out_roll_back:
	while (i--)
	{
		struct page * p;
		p = pfn_to_page(PFN_DOWN(pages[i]));
		pages[i] = (phys_addr_t)0;
		kbase_carveout_put_page(p);
	}

	return MALI_ERROR_OUT_OF_MEMORY;
}

void kbase_mem_allocator_free(kbase_mem_allocator *allocator, u32 nr_pages, phys_addr_t *pages, mali_bool sync_back)
{
	int i = 0;
	int page_count = 0;
	int tofree;

	LIST_HEAD(new_free_list_items);

	KBASE_DEBUG_ASSERT(NULL != allocator);

	might_sleep();

	/* Starting by just freeing the overspill.
	* As we do this outside of the lock we might spill too many pages
	* or get too many on the free list, but the max_size is just a ballpark so it is ok
	*/
	tofree = atomic_read(&allocator->free_list_size) + nr_pages - allocator->free_list_max_size;
	/* if tofree became negative this first for loop will be ignored */
	for (; i < tofree; i++)
	{
		if (likely(0 != pages[i]))
		{
			struct page * p;

			p = pfn_to_page(PFN_DOWN(pages[i]));
			pages[i] = (phys_addr_t)0;
			kbase_carveout_put_page(p);
		}
	}

	for (; i < nr_pages; i++)
	{
		if (likely(0 != pages[i]))
		{
			struct page * p;

			p = pfn_to_page(PFN_DOWN(pages[i]));
			pages[i] = (phys_addr_t)0;
			/* Sync back the memory to ensure that future cache invalidations
			 * don't trample on memory.
			 */
			if( sync_back )
			{
				void* mp = kmap(p);
				if( NULL != mp)
				{
					kbase_sync_to_cpu(PFN_PHYS(page_to_pfn(p)), mp, PAGE_SIZE);
					kunmap(p);
				}
			}
			list_add(&p->lru, &new_free_list_items);
			page_count++;
		}
	}
	mutex_lock(&allocator->free_list_lock);
	list_splice(&new_free_list_items, &allocator->free_list_head);
	atomic_add(page_count, &allocator->free_list_size);
	mutex_unlock(&allocator->free_list_lock);
}
KBASE_EXPORT_TEST_API(kbase_mem_allocator_free)

