/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_wq.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include <linux/slab.h>	/* For memory allocation */
#include <linux/workqueue.h>
#include <linux/version.h>
#include <linux/sched.h>

#include "vr_osk.h"
#include "vr_kernel_common.h"
#include "vr_kernel_license.h"
#include "vr_kernel_linux.h"

typedef struct _vr_osk_wq_work_s {
	_vr_osk_wq_work_handler_t handler;
	void *data;
	vr_bool high_pri;
	struct work_struct work_handle;
} vr_osk_wq_work_object_t;

typedef struct _vr_osk_wq_delayed_work_s {
	_vr_osk_wq_work_handler_t handler;
	void *data;
	struct delayed_work work;
} vr_osk_wq_delayed_work_object_t;

#if VR_LICENSE_IS_GPL
struct workqueue_struct *vr_wq_normal = NULL;
struct workqueue_struct *vr_wq_high = NULL;
#endif

static void _vr_osk_wq_work_func(struct work_struct *work);

_vr_osk_errcode_t _vr_osk_wq_init(void)
{
#if VR_LICENSE_IS_GPL
	VR_DEBUG_ASSERT(NULL == vr_wq_normal);
	VR_DEBUG_ASSERT(NULL == vr_wq_high);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	vr_wq_normal = alloc_workqueue("vr", WQ_UNBOUND, 0);
	vr_wq_high = alloc_workqueue("vr_high_pri", WQ_HIGHPRI, 0);
#else
	vr_wq_normal = create_workqueue("vr");
	vr_wq_high = create_workqueue("vr_high_pri");
#endif
	if (NULL == vr_wq_normal || NULL == vr_wq_high) {
		VR_PRINT_ERROR(("Unable to create Vr workqueues\n"));

		if (vr_wq_normal) destroy_workqueue(vr_wq_normal);
		if (vr_wq_high)   destroy_workqueue(vr_wq_high);

		vr_wq_normal = NULL;
		vr_wq_high   = NULL;

		return _VR_OSK_ERR_FAULT;
	}
#endif /* VR_LICENSE_IS_GPL */

	return _VR_OSK_ERR_OK;
}

void _vr_osk_wq_flush(void)
{
#if VR_LICENSE_IS_GPL
	flush_workqueue(vr_wq_high);
	flush_workqueue(vr_wq_normal);
#else
	flush_scheduled_work();
#endif
}

void _vr_osk_wq_term(void)
{
#if VR_LICENSE_IS_GPL
	VR_DEBUG_ASSERT(NULL != vr_wq_normal);
	VR_DEBUG_ASSERT(NULL != vr_wq_high);

	flush_workqueue(vr_wq_normal);
	destroy_workqueue(vr_wq_normal);

	flush_workqueue(vr_wq_high);
	destroy_workqueue(vr_wq_high);

	vr_wq_normal = NULL;
	vr_wq_high   = NULL;
#else
	flush_scheduled_work();
#endif
}

_vr_osk_wq_work_t *_vr_osk_wq_create_work( _vr_osk_wq_work_handler_t handler, void *data )
{
	vr_osk_wq_work_object_t *work = kmalloc(sizeof(vr_osk_wq_work_object_t), GFP_KERNEL);

	if (NULL == work) return NULL;

	work->handler = handler;
	work->data = data;
	work->high_pri = VR_FALSE;

	INIT_WORK( &work->work_handle, _vr_osk_wq_work_func);

	return work;
}

_vr_osk_wq_work_t *_vr_osk_wq_create_work_high_pri( _vr_osk_wq_work_handler_t handler, void *data )
{
	vr_osk_wq_work_object_t *work = kmalloc(sizeof(vr_osk_wq_work_object_t), GFP_KERNEL);

	if (NULL == work) return NULL;

	work->handler = handler;
	work->data = data;
	work->high_pri = VR_TRUE;

	INIT_WORK( &work->work_handle, _vr_osk_wq_work_func );

	return work;
}

void _vr_osk_wq_delete_work( _vr_osk_wq_work_t *work )
{
	vr_osk_wq_work_object_t *work_object = (vr_osk_wq_work_object_t *)work;
	_vr_osk_wq_flush();
	kfree(work_object);
}

void _vr_osk_wq_delete_work_nonflush( _vr_osk_wq_work_t *work )
{
	vr_osk_wq_work_object_t *work_object = (vr_osk_wq_work_object_t *)work;
	kfree(work_object);
}

void _vr_osk_wq_schedule_work( _vr_osk_wq_work_t *work )
{
	vr_osk_wq_work_object_t *work_object = (vr_osk_wq_work_object_t *)work;
#if VR_LICENSE_IS_GPL
	queue_work(vr_wq_normal, &work_object->work_handle);
#else
	schedule_work(&work_object->work_handle);
#endif
}

void _vr_osk_wq_schedule_work_high_pri( _vr_osk_wq_work_t *work )
{
	vr_osk_wq_work_object_t *work_object = (vr_osk_wq_work_object_t *)work;
#if VR_LICENSE_IS_GPL
	queue_work(vr_wq_high, &work_object->work_handle);
#else
	schedule_work(&work_object->work_handle);
#endif
}

static void _vr_osk_wq_work_func( struct work_struct *work )
{
	vr_osk_wq_work_object_t *work_object;

	work_object = _VR_OSK_CONTAINER_OF(work, vr_osk_wq_work_object_t, work_handle);

	/* We want higher priority than the Dynamic Priority, setting it to the lowest of the RT priorities */
	if (VR_TRUE == work_object->high_pri) {
		set_user_nice(current, -19);
	}

	work_object->handler(work_object->data);
}

static void _vr_osk_wq_delayed_work_func( struct work_struct *work )
{
	vr_osk_wq_delayed_work_object_t *work_object;

	work_object = _VR_OSK_CONTAINER_OF(work, vr_osk_wq_delayed_work_object_t, work.work);
	work_object->handler(work_object->data);
}

vr_osk_wq_delayed_work_object_t *_vr_osk_wq_delayed_create_work( _vr_osk_wq_work_handler_t handler, void *data)
{
	vr_osk_wq_delayed_work_object_t *work = kmalloc(sizeof(vr_osk_wq_delayed_work_object_t), GFP_KERNEL);

	if (NULL == work) return NULL;

	work->handler = handler;
	work->data = data;

	INIT_DELAYED_WORK(&work->work, _vr_osk_wq_delayed_work_func);

	return work;
}

void _vr_osk_wq_delayed_delete_work_nonflush( _vr_osk_wq_delayed_work_t *work )
{
	vr_osk_wq_delayed_work_object_t *work_object = (vr_osk_wq_delayed_work_object_t *)work;
	kfree(work_object);
}

void _vr_osk_wq_delayed_cancel_work_async( _vr_osk_wq_delayed_work_t *work )
{
	vr_osk_wq_delayed_work_object_t *work_object = (vr_osk_wq_delayed_work_object_t *)work;
	cancel_delayed_work(&work_object->work);
}

void _vr_osk_wq_delayed_cancel_work_sync( _vr_osk_wq_delayed_work_t *work )
{
	vr_osk_wq_delayed_work_object_t *work_object = (vr_osk_wq_delayed_work_object_t *)work;
	cancel_delayed_work_sync(&work_object->work);
}

void _vr_osk_wq_delayed_schedule_work( _vr_osk_wq_delayed_work_t *work, u32 delay )
{
	vr_osk_wq_delayed_work_object_t *work_object = (vr_osk_wq_delayed_work_object_t *)work;

#if VR_LICENSE_IS_GPL
	queue_delayed_work(vr_wq_normal, &work_object->work, delay);
#else
	schedule_delayed_work(&work_object->work, delay);
#endif

}
