/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include <linux/fs.h>	   /* file system operations */
#include <asm/uaccess.h>	/* user space access */
#include <linux/dma-buf.h>
#include <linux/scatterlist.h>
#include <linux/rbtree.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#include "vr_ukk.h"
#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_kernel_linux.h"

#include "vr_memory.h"
#include "vr_memory_dma_buf.h"

#include "vr_pp_job.h"

static void vr_dma_buf_unmap(struct vr_dma_buf_attachment *mem);

struct vr_dma_buf_attachment {
	struct dma_buf *buf;
	struct dma_buf_attachment *attachment;
	struct sg_table *sgt;
	struct vr_session_data *session;
	int map_ref;
	struct mutex map_lock;
	vr_bool is_mapped;
	wait_queue_head_t wait_queue;
};

static void vr_dma_buf_release(struct vr_dma_buf_attachment *mem)
{
	VR_DEBUG_PRINT(3, ("Vr DMA-buf: release attachment %p\n", mem));

	VR_DEBUG_ASSERT_POINTER(mem);
	VR_DEBUG_ASSERT_POINTER(mem->attachment);
	VR_DEBUG_ASSERT_POINTER(mem->buf);

#if defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
	/* We mapped implicitly on attach, so we need to unmap on release */
	vr_dma_buf_unmap(mem);
#endif

	/* Wait for buffer to become unmapped */
	wait_event(mem->wait_queue, !mem->is_mapped);
	VR_DEBUG_ASSERT(!mem->is_mapped);

	dma_buf_detach(mem->buf, mem->attachment);
	dma_buf_put(mem->buf);

	_vr_osk_free(mem);
}

void vr_mem_dma_buf_release(vr_mem_allocation *descriptor)
{
	struct vr_dma_buf_attachment *mem = descriptor->dma_buf.attachment;

	vr_dma_buf_release(mem);
}

/*
 * Map DMA buf attachment \a mem into \a session at virtual address \a virt.
 */
static int vr_dma_buf_map(struct vr_dma_buf_attachment *mem, struct vr_session_data *session, u32 virt, u32 flags)
{
	struct vr_page_directory *pagedir;
	struct scatterlist *sg;
	int i;

	VR_DEBUG_ASSERT_POINTER(mem);
	VR_DEBUG_ASSERT_POINTER(session);
	VR_DEBUG_ASSERT(mem->session == session);

	mutex_lock(&mem->map_lock);

	mem->map_ref++;

	VR_DEBUG_PRINT(5, ("Vr DMA-buf: map attachment %p, new map_ref = %d\n", mem, mem->map_ref));

	if (1 == mem->map_ref) {
		/* First reference taken, so we need to map the dma buf */
		VR_DEBUG_ASSERT(!mem->is_mapped);

		pagedir = vr_session_get_page_directory(session);
		VR_DEBUG_ASSERT_POINTER(pagedir);

		mem->sgt = dma_buf_map_attachment(mem->attachment, DMA_BIDIRECTIONAL);
		if (IS_ERR_OR_NULL(mem->sgt)) {
			VR_DEBUG_PRINT_ERROR(("Failed to map dma-buf attachment\n"));
			return -EFAULT;
		}

		for_each_sg(mem->sgt->sgl, sg, mem->sgt->nents, i) {
			u32 size = sg_dma_len(sg);
			dma_addr_t phys = sg_dma_address(sg);

			/* sg must be page aligned. */
			VR_DEBUG_ASSERT(0 == size % VR_MMU_PAGE_SIZE);

			vr_mmu_pagedir_update(pagedir, virt, phys, size, VR_MMU_FLAGS_DEFAULT);

			virt += size;
		}

		if (flags & VR_MEM_FLAG_VR_GUARD_PAGE) {
			u32 guard_phys;
			VR_DEBUG_PRINT(7, ("Mapping in extra guard page\n"));

			guard_phys = sg_dma_address(mem->sgt->sgl);
			vr_mmu_pagedir_update(pagedir, virt, guard_phys, VR_MMU_PAGE_SIZE, VR_MMU_FLAGS_DEFAULT);
		}

		mem->is_mapped = VR_TRUE;
		mutex_unlock(&mem->map_lock);

		/* Wake up any thread waiting for buffer to become mapped */
		wake_up_all(&mem->wait_queue);
	} else {
		VR_DEBUG_ASSERT(mem->is_mapped);
		mutex_unlock(&mem->map_lock);
	}

	return 0;
}

static void vr_dma_buf_unmap(struct vr_dma_buf_attachment *mem)
{
	VR_DEBUG_ASSERT_POINTER(mem);
	VR_DEBUG_ASSERT_POINTER(mem->attachment);
	VR_DEBUG_ASSERT_POINTER(mem->buf);

	mutex_lock(&mem->map_lock);

	mem->map_ref--;

	VR_DEBUG_PRINT(5, ("Vr DMA-buf: unmap attachment %p, new map_ref = %d\n", mem, mem->map_ref));

	if (0 == mem->map_ref) {
		dma_buf_unmap_attachment(mem->attachment, mem->sgt, DMA_BIDIRECTIONAL);

		mem->is_mapped = VR_FALSE;
	}

	mutex_unlock(&mem->map_lock);

	/* Wake up any thread waiting for buffer to become unmapped */
	wake_up_all(&mem->wait_queue);
}

#if !defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
int vr_dma_buf_map_job(struct vr_pp_job *job)
{
	vr_mem_allocation *descriptor;
	struct vr_dma_buf_attachment *mem;
	_vr_osk_errcode_t err;
	int i;
	int ret = 0;

	_vr_osk_mutex_wait(job->session->memory_lock);

	for (i = 0; i < job->num_memory_cookies; i++) {
		int cookie = job->memory_cookies[i];

		if (0 == cookie) {
			/* 0 is not a valid cookie */
			VR_DEBUG_ASSERT(NULL == job->dma_bufs[i]);
			continue;
		}

		VR_DEBUG_ASSERT(0 < cookie);

		err = vr_descriptor_mapping_get(job->session->descriptor_mapping,
		                                  cookie, (void**)&descriptor);

		if (_VR_OSK_ERR_OK != err) {
			VR_DEBUG_PRINT_ERROR(("Vr DMA-buf: Failed to get descriptor for cookie %d\n", cookie));
			ret = -EFAULT;
			VR_DEBUG_ASSERT(NULL == job->dma_bufs[i]);
			continue;
		}

		if (VR_MEM_DMA_BUF != descriptor->type) {
			/* Not a DMA-buf */
			VR_DEBUG_ASSERT(NULL == job->dma_bufs[i]);
			continue;
		}

		mem = descriptor->dma_buf.attachment;

		VR_DEBUG_ASSERT_POINTER(mem);
		VR_DEBUG_ASSERT(mem->session == job->session);

		err = vr_dma_buf_map(mem, mem->session, descriptor->vr_mapping.addr, descriptor->flags);
		if (0 != err) {
			VR_DEBUG_PRINT_ERROR(("Vr DMA-buf: Failed to map dma-buf for cookie %d at vr address %x\b",
			                        cookie, descriptor->vr_mapping.addr));
			ret = -EFAULT;
			VR_DEBUG_ASSERT(NULL == job->dma_bufs[i]);
			continue;
		}

		/* Add mem to list of DMA-bufs mapped for this job */
		job->dma_bufs[i] = mem;
	}

	_vr_osk_mutex_signal(job->session->memory_lock);

	return ret;
}

void vr_dma_buf_unmap_job(struct vr_pp_job *job)
{
	int i;
	for (i = 0; i < job->num_dma_bufs; i++) {
		if (NULL == job->dma_bufs[i]) continue;

		vr_dma_buf_unmap(job->dma_bufs[i]);
		job->dma_bufs[i] = NULL;
	}
}
#endif /* !CONFIG_VR_DMA_BUF_MAP_ON_ATTACH */

int vr_attach_dma_buf(struct vr_session_data *session, _vr_uk_attach_dma_buf_s __user *user_arg)
{
	struct dma_buf *buf;
	struct vr_dma_buf_attachment *mem;
	_vr_uk_attach_dma_buf_s args;
	vr_mem_allocation *descriptor;
	int md;
	int fd;

	/* Get call arguments from user space. copy_from_user returns how many bytes which where NOT copied */
	if (0 != copy_from_user(&args, (void __user *)user_arg, sizeof(_vr_uk_attach_dma_buf_s))) {
		return -EFAULT;
	}

	if (args.vr_address & ~PAGE_MASK) {
		VR_DEBUG_PRINT_ERROR(("Requested address (0x%08x) is not page aligned\n", args.vr_address));
		return -EINVAL;
	}

	if (args.vr_address >= args.vr_address + args.size) {
		VR_DEBUG_PRINT_ERROR(("Requested address and size (0x%08x + 0x%08x) is too big\n", args.vr_address, args.size));
		return -EINVAL;
	}

	fd = args.mem_fd;

	buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf)) {
		VR_DEBUG_PRINT_ERROR(("Failed to get dma-buf from fd: %d\n", fd));
		return PTR_RET(buf);
	}

	/* Currently, mapping of the full buffer are supported. */
	if (args.size != buf->size) {
		VR_DEBUG_PRINT_ERROR(("dma-buf size doesn't match mapping size.\n"));
		dma_buf_put(buf);
		return -EINVAL;
	}

	mem = _vr_osk_calloc(1, sizeof(struct vr_dma_buf_attachment));
	if (NULL == mem) {
		VR_DEBUG_PRINT_ERROR(("Failed to allocate dma-buf tracing struct\n"));
		dma_buf_put(buf);
		return -ENOMEM;
	}

	mem->buf = buf;
	mem->session = session;
	mem->map_ref = 0;
	mutex_init(&mem->map_lock);
	init_waitqueue_head(&mem->wait_queue);

	mem->attachment = dma_buf_attach(mem->buf, &vr_platform_device->dev);
	if (NULL == mem->attachment) {
		VR_DEBUG_PRINT_ERROR(("Failed to attach to dma-buf %d\n", fd));
		dma_buf_put(mem->buf);
		_vr_osk_free(mem);
		return -EFAULT;
	}

	/* Set up Vr memory descriptor */
	descriptor = vr_mem_descriptor_create(session, VR_MEM_DMA_BUF);
	if (NULL == descriptor) {
		VR_DEBUG_PRINT_ERROR(("Failed to allocate descriptor dma-buf %d\n", fd));
		vr_dma_buf_release(mem);
		return -ENOMEM;
	}

	descriptor->size = args.size;
	descriptor->vr_mapping.addr = args.vr_address;

	descriptor->dma_buf.attachment = mem;

	descriptor->flags |= VR_MEM_FLAG_DONT_CPU_MAP;
	if (args.flags & _VR_MAP_EXTERNAL_MAP_GUARD_PAGE) {
		descriptor->flags = VR_MEM_FLAG_VR_GUARD_PAGE;
	}

	_vr_osk_mutex_wait(session->memory_lock);

	/* Map dma-buf into this session's page tables */
	if (_VR_OSK_ERR_OK != vr_mem_vr_map_prepare(descriptor)) {
		_vr_osk_mutex_signal(session->memory_lock);
		VR_DEBUG_PRINT_ERROR(("Failed to map dma-buf on Vr\n"));
		vr_mem_descriptor_destroy(descriptor);
		vr_dma_buf_release(mem);
		return -ENOMEM;
	}

#if defined(CONFIG_VR_DMA_BUF_MAP_ON_ATTACH)
	/* Map memory into session's Vr virtual address space. */

	if (0 != vr_dma_buf_map(mem, session, descriptor->vr_mapping.addr, descriptor->flags)) {
		vr_mem_vr_map_free(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);

		VR_DEBUG_PRINT_ERROR(("Failed to map dma-buf %d into Vr address space\n", fd));
		vr_mem_descriptor_destroy(descriptor);
		vr_dma_buf_release(mem);
		return -ENOMEM;
	}

#endif

	_vr_osk_mutex_signal(session->memory_lock);

	/* Get descriptor mapping for memory. */
	if (_VR_OSK_ERR_OK != vr_descriptor_mapping_allocate_mapping(session->descriptor_mapping, descriptor, &md)) {
		_vr_osk_mutex_wait(session->memory_lock);
		vr_mem_vr_map_free(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);

		VR_DEBUG_PRINT_ERROR(("Failed to create descriptor mapping for dma-buf %d\n", fd));
		vr_mem_descriptor_destroy(descriptor);
		vr_dma_buf_release(mem);
		return -EFAULT;
	}

	/* Return stuff to user space */
	if (0 != put_user(md, &user_arg->cookie)) {
		_vr_osk_mutex_wait(session->memory_lock);
		vr_mem_vr_map_free(descriptor);
		_vr_osk_mutex_signal(session->memory_lock);

		VR_DEBUG_PRINT_ERROR(("Failed to return descriptor to user space for dma-buf %d\n", fd));
		vr_descriptor_mapping_free(session->descriptor_mapping, md);
		vr_dma_buf_release(mem);
		return -EFAULT;
	}

	return 0;
}

int vr_release_dma_buf(struct vr_session_data *session, _vr_uk_release_dma_buf_s __user *user_arg)
{
	int ret = 0;
	_vr_uk_release_dma_buf_s args;
	vr_mem_allocation *descriptor;

	/* get call arguments from user space. copy_from_user returns how many bytes which where NOT copied */
	if ( 0 != copy_from_user(&args, (void __user *)user_arg, sizeof(_vr_uk_release_dma_buf_s)) ) {
		return -EFAULT;
	}

	VR_DEBUG_PRINT(3, ("Vr DMA-buf: release descriptor cookie %d\n", args.cookie));

	_vr_osk_mutex_wait(session->memory_lock);

	descriptor = vr_descriptor_mapping_free(session->descriptor_mapping, args.cookie);

	if (NULL != descriptor) {
		VR_DEBUG_PRINT(3, ("Vr DMA-buf: Releasing dma-buf at vr address %x\n", descriptor->vr_mapping.addr));

		vr_mem_vr_map_free(descriptor);

		vr_dma_buf_release(descriptor->dma_buf.attachment);

		vr_mem_descriptor_destroy(descriptor);
	} else {
		VR_DEBUG_PRINT_ERROR(("Invalid memory descriptor %d used to release dma-buf\n", args.cookie));
		ret = -EINVAL;
	}

	_vr_osk_mutex_signal(session->memory_lock);

	/* Return the error that _vr_ukk_map_external_ump_mem produced */
	return ret;
}

int vr_dma_buf_get_size(struct vr_session_data *session, _vr_uk_dma_buf_get_size_s __user *user_arg)
{
	_vr_uk_dma_buf_get_size_s args;
	int fd;
	struct dma_buf *buf;

	/* get call arguments from user space. copy_from_user returns how many bytes which where NOT copied */
	if ( 0 != copy_from_user(&args, (void __user *)user_arg, sizeof(_vr_uk_dma_buf_get_size_s)) ) {
		return -EFAULT;
	}

	/* Do DMA-BUF stuff */
	fd = args.mem_fd;

	buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf)) {
		VR_DEBUG_PRINT_ERROR(("Failed to get dma-buf from fd: %d\n", fd));
		return PTR_RET(buf);
	}

	if (0 != put_user(buf->size, &user_arg->size)) {
		dma_buf_put(buf);
		return -EFAULT;
	}

	dma_buf_put(buf);

	return 0;
}
