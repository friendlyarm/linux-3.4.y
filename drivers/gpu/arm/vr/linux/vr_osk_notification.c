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
 * @file vr_osk_notification.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include "vr_osk.h"
#include "vr_kernel_common.h"

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

/**
 * Declaration of the notification queue object type
 * Contains a linked list of notification pending delivery to user space.
 * It also contains a wait queue of exclusive waiters blocked in the ioctl
 * When a new notification is posted a single thread is resumed.
 */
struct _vr_osk_notification_queue_t_struct {
	spinlock_t mutex; /**< Mutex protecting the list */
	wait_queue_head_t receive_queue; /**< Threads waiting for new entries to the queue */
	struct list_head head; /**< List of notifications waiting to be picked up */
};

typedef struct _vr_osk_notification_wrapper_t_struct {
	struct list_head list;           /**< Internal linked list variable */
	_vr_osk_notification_t data;   /**< Notification data */
} _vr_osk_notification_wrapper_t;

_vr_osk_notification_queue_t *_vr_osk_notification_queue_init( void )
{
	_vr_osk_notification_queue_t *	result;

	result = (_vr_osk_notification_queue_t *)kmalloc(sizeof(_vr_osk_notification_queue_t), GFP_KERNEL);
	if (NULL == result) return NULL;

	spin_lock_init(&result->mutex);
	init_waitqueue_head(&result->receive_queue);
	INIT_LIST_HEAD(&result->head);

	return result;
}

_vr_osk_notification_t *_vr_osk_notification_create( u32 type, u32 size )
{
	/* OPT Recycling of notification objects */
	_vr_osk_notification_wrapper_t *notification;

	notification = (_vr_osk_notification_wrapper_t *)kmalloc( sizeof(_vr_osk_notification_wrapper_t) + size,
	               GFP_KERNEL | __GFP_HIGH | __GFP_REPEAT);
	if (NULL == notification) {
		VR_DEBUG_PRINT(1, ("Failed to create a notification object\n"));
		return NULL;
	}

	/* Init the list */
	INIT_LIST_HEAD(&notification->list);

	if (0 != size) {
		notification->data.result_buffer = ((u8*)notification) + sizeof(_vr_osk_notification_wrapper_t);
	} else {
		notification->data.result_buffer = NULL;
	}

	/* set up the non-allocating fields */
	notification->data.notification_type = type;
	notification->data.result_buffer_size = size;

	/* all ok */
	return &(notification->data);
}

void _vr_osk_notification_delete( _vr_osk_notification_t *object )
{
	_vr_osk_notification_wrapper_t *notification;
	VR_DEBUG_ASSERT_POINTER( object );

	notification = container_of( object, _vr_osk_notification_wrapper_t, data );

	/* Free the container */
	kfree(notification);
}

void _vr_osk_notification_queue_term( _vr_osk_notification_queue_t *queue )
{
	_vr_osk_notification_t *result;
	VR_DEBUG_ASSERT_POINTER( queue );

	while (_VR_OSK_ERR_OK == _vr_osk_notification_queue_dequeue(queue, &result)) {
		_vr_osk_notification_delete( result );
	}

	/* not much to do, just free the memory */
	kfree(queue);
}
void _vr_osk_notification_queue_send( _vr_osk_notification_queue_t *queue, _vr_osk_notification_t *object )
{
#if defined(VR_UPPER_HALF_SCHEDULING)
	unsigned long irq_flags;
#endif

	_vr_osk_notification_wrapper_t *notification;
	VR_DEBUG_ASSERT_POINTER( queue );
	VR_DEBUG_ASSERT_POINTER( object );

	notification = container_of( object, _vr_osk_notification_wrapper_t, data );

#if defined(VR_UPPER_HALF_SCHEDULING)
	spin_lock_irqsave(&queue->mutex, irq_flags);
#else
	spin_lock(&queue->mutex);
#endif

	list_add_tail(&notification->list, &queue->head);

#if defined(VR_UPPER_HALF_SCHEDULING)
	spin_unlock_irqrestore(&queue->mutex, irq_flags);
#else
	spin_unlock(&queue->mutex);
#endif

	/* and wake up one possible exclusive waiter */
	wake_up(&queue->receive_queue);
}

_vr_osk_errcode_t _vr_osk_notification_queue_dequeue( _vr_osk_notification_queue_t *queue, _vr_osk_notification_t **result )
{
#if defined(VR_UPPER_HALF_SCHEDULING)
	unsigned long irq_flags;
#endif

	_vr_osk_errcode_t ret = _VR_OSK_ERR_ITEM_NOT_FOUND;
	_vr_osk_notification_wrapper_t *wrapper_object;

#if defined(VR_UPPER_HALF_SCHEDULING)
	spin_lock_irqsave(&queue->mutex, irq_flags);
#else
	spin_lock(&queue->mutex);
#endif

	if (!list_empty(&queue->head)) {
		wrapper_object = list_entry(queue->head.next, _vr_osk_notification_wrapper_t, list);
		*result = &(wrapper_object->data);
		list_del_init(&wrapper_object->list);
		ret = _VR_OSK_ERR_OK;
	}

#if defined(VR_UPPER_HALF_SCHEDULING)
	spin_unlock_irqrestore(&queue->mutex, irq_flags);
#else
	spin_unlock(&queue->mutex);
#endif

	return ret;
}

_vr_osk_errcode_t _vr_osk_notification_queue_receive( _vr_osk_notification_queue_t *queue, _vr_osk_notification_t **result )
{
	/* check input */
	VR_DEBUG_ASSERT_POINTER( queue );
	VR_DEBUG_ASSERT_POINTER( result );

	/* default result */
	*result = NULL;

	if (wait_event_interruptible(queue->receive_queue,
	                             _VR_OSK_ERR_OK == _vr_osk_notification_queue_dequeue(queue, result))) {
		return _VR_OSK_ERR_RESTARTSYSCALL;
	}

	return _VR_OSK_ERR_OK; /* all ok */
}
