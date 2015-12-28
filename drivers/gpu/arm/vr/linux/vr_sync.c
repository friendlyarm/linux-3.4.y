/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_sync.h"

#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_timeline.h"

#include <linux/file.h>
#include <linux/seq_file.h>
#include <linux/module.h>

struct vr_sync_pt {
	struct sync_pt         sync_pt;
	struct vr_sync_flag *flag;
};

/**
 * The sync flag is used to connect sync fences to the Vr Timeline system.  Sync fences can be
 * created from a sync flag, and when the flag is signaled, the sync fences will also be signaled.
 */
struct vr_sync_flag {
	struct sync_timeline *sync_tl;  /**< Sync timeline this flag is connected to. */
	u32                   point;    /**< Point on timeline. */
	int                   status;   /**< 0 if unsignaled, 1 if signaled without error or negative if signaled with error. */
	struct kref           refcount; /**< Reference count. */
};

VR_STATIC_INLINE struct vr_sync_pt *to_vr_sync_pt(struct sync_pt *pt)
{
	return container_of(pt, struct vr_sync_pt, sync_pt);
}

static struct sync_pt *timeline_dup(struct sync_pt *pt)
{
	struct vr_sync_pt *mpt, *new_mpt;
	struct sync_pt *new_pt;

	VR_DEBUG_ASSERT_POINTER(pt);
	mpt = to_vr_sync_pt(pt);

	new_pt = sync_pt_create(pt->parent, sizeof(struct vr_sync_pt));
	if (NULL == new_pt) return NULL;

	new_mpt = to_vr_sync_pt(new_pt);

	vr_sync_flag_get(mpt->flag);
	new_mpt->flag = mpt->flag;

	return new_pt;
}

static int timeline_has_signaled(struct sync_pt *pt)
{
	struct vr_sync_pt *mpt;

	VR_DEBUG_ASSERT_POINTER(pt);
	mpt = to_vr_sync_pt(pt);

	VR_DEBUG_ASSERT_POINTER(mpt->flag);

	return mpt->flag->status;
}

static int timeline_compare(struct sync_pt *pta, struct sync_pt *ptb)
{
	struct vr_sync_pt *mpta;
	struct vr_sync_pt *mptb;
	u32 a, b;

	VR_DEBUG_ASSERT_POINTER(pta);
	VR_DEBUG_ASSERT_POINTER(ptb);
	mpta = to_vr_sync_pt(pta);
	mptb = to_vr_sync_pt(ptb);

	VR_DEBUG_ASSERT_POINTER(mpta->flag);
	VR_DEBUG_ASSERT_POINTER(mptb->flag);

	a = mpta->flag->point;
	b = mpta->flag->point;

	if (a == b) return 0;

	return ((b - a) < (a - b) ? -1 : 1);
}

static void timeline_free_pt(struct sync_pt *pt)
{
	struct vr_sync_pt *mpt;

	VR_DEBUG_ASSERT_POINTER(pt);
	mpt = to_vr_sync_pt(pt);

	vr_sync_flag_put(mpt->flag);
}

static void timeline_release(struct sync_timeline *sync_timeline)
{
	module_put(THIS_MODULE);
}

static void timeline_print_pt(struct seq_file *s, struct sync_pt *sync_pt)
{
	struct vr_sync_pt *mpt;

	VR_DEBUG_ASSERT_POINTER(s);
	VR_DEBUG_ASSERT_POINTER(sync_pt);

	mpt = to_vr_sync_pt(sync_pt);
	VR_DEBUG_ASSERT_POINTER(mpt->flag);

	seq_printf(s, "%u", mpt->flag->point);
}

static struct sync_timeline_ops vr_timeline_ops = {
	.driver_name    = "Vr",
	.dup            = timeline_dup,
	.has_signaled   = timeline_has_signaled,
	.compare        = timeline_compare,
	.free_pt        = timeline_free_pt,
	.release_obj    = timeline_release,
	.print_pt       = timeline_print_pt,
};

struct sync_timeline *vr_sync_timeline_create(const char *name)
{
	struct sync_timeline *sync_tl;

	sync_tl = sync_timeline_create(&vr_timeline_ops, sizeof(struct sync_timeline), name);
	if (NULL == sync_tl) return NULL;

	/* Grab a reference on the module to ensure the callbacks are present
	 * as long some timeline exists. The reference is released when the
	 * timeline is freed.
	 * Since this function is called from a ioctl on an open file we know
	 * we already have a reference, so using __module_get is safe. */
	__module_get(THIS_MODULE);

	return sync_tl;
}

vr_bool vr_sync_timeline_is_ours(struct sync_timeline *sync_tl)
{
	VR_DEBUG_ASSERT_POINTER(sync_tl);
	return (sync_tl->ops == &vr_timeline_ops) ? VR_TRUE : VR_FALSE;
}

s32 vr_sync_fence_fd_alloc(struct sync_fence *sync_fence)
{
	s32 fd = -1;

	fd = get_unused_fd();
	if (fd < 0) {
		sync_fence_put(sync_fence);
		return -1;
	}
	sync_fence_install(sync_fence, fd);

	return fd;
}

struct sync_fence *vr_sync_fence_merge(struct sync_fence *sync_fence1, struct sync_fence *sync_fence2)
{
	struct sync_fence *sync_fence;

	VR_DEBUG_ASSERT_POINTER(sync_fence1);
	VR_DEBUG_ASSERT_POINTER(sync_fence1);

	sync_fence = sync_fence_merge("vr_merge_fence", sync_fence1, sync_fence2);
	sync_fence_put(sync_fence1);
	sync_fence_put(sync_fence2);

	return sync_fence;
}

struct sync_fence *vr_sync_timeline_create_signaled_fence(struct sync_timeline *sync_tl)
{
	struct vr_sync_flag *flag;
	struct sync_fence *sync_fence;

	VR_DEBUG_ASSERT_POINTER(sync_tl);

	flag = vr_sync_flag_create(sync_tl, 0);
	if (NULL == flag) return NULL;

	sync_fence = vr_sync_flag_create_fence(flag);

	vr_sync_flag_signal(flag, 0);
	vr_sync_flag_put(flag);

	return sync_fence;
}

struct vr_sync_flag *vr_sync_flag_create(struct sync_timeline *sync_tl, vr_timeline_point point)
{
	struct vr_sync_flag *flag;

	if (NULL == sync_tl) return NULL;

	flag = _vr_osk_calloc(1, sizeof(*flag));
	if (NULL == flag) return NULL;

	flag->sync_tl = sync_tl;
	flag->point = point;

	flag->status = 0;
	kref_init(&flag->refcount);

	return flag;
}

void vr_sync_flag_get(struct vr_sync_flag *flag)
{
	VR_DEBUG_ASSERT_POINTER(flag);
	kref_get(&flag->refcount);
}

/**
 * Free sync flag.
 *
 * @param ref kref object embedded in sync flag that should be freed.
 */
static void vr_sync_flag_free(struct kref *ref)
{
	struct vr_sync_flag *flag;

	VR_DEBUG_ASSERT_POINTER(ref);
	flag = container_of(ref, struct vr_sync_flag, refcount);

	_vr_osk_free(flag);
}

void vr_sync_flag_put(struct vr_sync_flag *flag)
{
	VR_DEBUG_ASSERT_POINTER(flag);
	kref_put(&flag->refcount, vr_sync_flag_free);
}

void vr_sync_flag_signal(struct vr_sync_flag *flag, int error)
{
	VR_DEBUG_ASSERT_POINTER(flag);

	VR_DEBUG_ASSERT(0 == flag->status);
	flag->status = (0 > error) ? error : 1;

	_vr_osk_write_mem_barrier();

	sync_timeline_signal(flag->sync_tl);
}

/**
 * Create a sync point attached to given sync flag.
 *
 * @note Sync points must be triggered in *exactly* the same order as they are created.
 *
 * @param flag Sync flag.
 * @return New sync point if successful, NULL if not.
 */
static struct sync_pt *vr_sync_flag_create_pt(struct vr_sync_flag *flag)
{
	struct sync_pt *pt;
	struct vr_sync_pt *mpt;

	VR_DEBUG_ASSERT_POINTER(flag);
	VR_DEBUG_ASSERT_POINTER(flag->sync_tl);

	pt = sync_pt_create(flag->sync_tl, sizeof(struct vr_sync_pt));
	if (NULL == pt) return NULL;

	vr_sync_flag_get(flag);

	mpt = to_vr_sync_pt(pt);
	mpt->flag = flag;

	return pt;
}

struct sync_fence *vr_sync_flag_create_fence(struct vr_sync_flag *flag)
{
	struct sync_pt    *sync_pt;
	struct sync_fence *sync_fence;

	VR_DEBUG_ASSERT_POINTER(flag);
	VR_DEBUG_ASSERT_POINTER(flag->sync_tl);

	sync_pt = vr_sync_flag_create_pt(flag);
	if (NULL == sync_pt) return NULL;

	sync_fence = sync_fence_create("vr_flag_fence", sync_pt);
	if (NULL == sync_fence) {
		sync_pt_free(sync_pt);
		return NULL;
	}

	return sync_fence;
}
