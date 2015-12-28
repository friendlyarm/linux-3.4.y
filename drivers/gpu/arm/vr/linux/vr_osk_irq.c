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
 * @file vr_osk_irq.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include <linux/slab.h>	/* For memory allocation */
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include "vr_osk.h"
#include "vr_kernel_common.h"

typedef struct _vr_osk_irq_t_struct {
	u32 irqnum;
	void *data;
	_vr_osk_irq_uhandler_t uhandler;
} vr_osk_irq_object_t;

typedef irqreturn_t (*irq_handler_func_t)(int, void *, struct pt_regs *);
static irqreturn_t irq_handler_upper_half (int port_name, void* dev_id ); /* , struct pt_regs *regs*/

#if defined(DEBUG)
#if 0

struct test_interrupt_data {
	_vr_osk_irq_ack_t ack_func;
	void *probe_data;
	vr_bool interrupt_received;
	wait_queue_head_t wq;
};

static irqreturn_t test_interrupt_upper_half(int port_name, void *dev_id)
{
	irqreturn_t ret = IRQ_NONE;
	struct test_interrupt_data *data = (struct test_interrupt_data *)dev_id;

	if (_VR_OSK_ERR_OK == data->ack_func(data->probe_data)) {
		data->interrupt_received = VR_TRUE;
		wake_up(&data->wq);
		ret = IRQ_HANDLED;
	}

	return ret;
}

static _vr_osk_errcode_t test_interrupt(u32 irqnum,
        _vr_osk_irq_trigger_t trigger_func,
        _vr_osk_irq_ack_t ack_func,
        void *probe_data,
        const char *description)
{
	unsigned long irq_flags = 0;
	struct test_interrupt_data data = {
		.ack_func = ack_func,
		.probe_data = probe_data,
		.interrupt_received = VR_FALSE,
	};

#if defined(CONFIG_VR_SHARED_INTERRUPTS)
	irq_flags |= IRQF_SHARED;
#endif /* defined(CONFIG_VR_SHARED_INTERRUPTS) */

	if (0 != request_irq(irqnum, test_interrupt_upper_half, irq_flags, description, &data)) {
		VR_DEBUG_PRINT(2, ("Unable to install test IRQ handler for core '%s'\n", description));
		return _VR_OSK_ERR_FAULT;
	}

	init_waitqueue_head(&data.wq);

	trigger_func(probe_data);
	wait_event_timeout(data.wq, data.interrupt_received, 100);

	free_irq(irqnum, &data);

	if (data.interrupt_received) {
		VR_DEBUG_PRINT(3, ("%s: Interrupt test OK\n", description));
		return _VR_OSK_ERR_OK;
	} else {
		VR_PRINT_ERROR(("%s: Failed interrupt test on %u\n", description, irqnum));
		return _VR_OSK_ERR_FAULT;
	}
}
#endif

#endif /* defined(DEBUG) */

_vr_osk_irq_t *_vr_osk_irq_init( u32 irqnum, _vr_osk_irq_uhandler_t uhandler, void *int_data, _vr_osk_irq_trigger_t trigger_func, _vr_osk_irq_ack_t ack_func, void *probe_data, const char *description )
{
	vr_osk_irq_object_t *irq_object;
	unsigned long irq_flags = 0;

#if defined(CONFIG_VR_SHARED_INTERRUPTS)
	irq_flags |= IRQF_SHARED;
#endif /* defined(CONFIG_VR_SHARED_INTERRUPTS) */

	irq_object = kmalloc(sizeof(vr_osk_irq_object_t), GFP_KERNEL);
	if (NULL == irq_object) {
		return NULL;
	}

	if (-1 == irqnum) {
		/* Probe for IRQ */
		if ( (NULL != trigger_func) && (NULL != ack_func) ) {
			unsigned long probe_count = 3;
			_vr_osk_errcode_t err;
			int irq;

			VR_DEBUG_PRINT(2, ("Probing for irq\n"));

			do {
				unsigned long mask;

				mask = probe_irq_on();
				trigger_func(probe_data);

				_vr_osk_time_ubusydelay(5);

				irq = probe_irq_off(mask);
				err = ack_func(probe_data);
			} while (irq < 0 && (err == _VR_OSK_ERR_OK) && probe_count--);

			if (irq < 0 || (_VR_OSK_ERR_OK != err)) irqnum = -1;
			else irqnum = irq;
		} else irqnum = -1; /* no probe functions, fault */

		if (-1 != irqnum) {
			/* found an irq */
			VR_DEBUG_PRINT(2, ("Found irq %d\n", irqnum));
		} else {
			VR_DEBUG_PRINT(2, ("Probe for irq failed\n"));
		}
	}

	irq_object->irqnum = irqnum;
	irq_object->uhandler = uhandler;
	irq_object->data = int_data;

	if (-1 == irqnum) {
		VR_DEBUG_PRINT(2, ("No IRQ for core '%s' found during probe\n", description));
		kfree(irq_object);
		return NULL;
	}

#if defined(DEBUG)
#if 0
	/* Verify that the configured interrupt settings are working */
	if (_VR_OSK_ERR_OK != test_interrupt(irqnum, trigger_func, ack_func, probe_data, description)) {
		VR_DEBUG_PRINT(2, ("Test of IRQ handler for core '%s' failed\n", description));
		kfree(irq_object);
		return NULL;
	}
#endif
#endif

	if (0 != request_irq(irqnum, irq_handler_upper_half, irq_flags, description, irq_object)) {
		VR_DEBUG_PRINT(2, ("Unable to install IRQ handler for core '%s'\n", description));
		kfree(irq_object);
		return NULL;
	}

	return irq_object;
}

void _vr_osk_irq_term( _vr_osk_irq_t *irq )
{
	vr_osk_irq_object_t *irq_object = (vr_osk_irq_object_t *)irq;
	free_irq(irq_object->irqnum, irq_object);
	kfree(irq_object);
}


/** This function is called directly in interrupt context from the OS just after
 * the CPU get the hw-irq from vr, or other devices on the same IRQ-channel.
 * It is registered one of these function for each vr core. When an interrupt
 * arrives this function will be called equal times as registered vr cores.
 * That means that we only check one vr core in one function call, and the
 * core we check for each turn is given by the \a dev_id variable.
 * If we detect an pending interrupt on the given core, we mask the interrupt
 * out by settging the core's IRQ_MASK register to zero.
 * Then we schedule the vr_core_irq_handler_bottom_half to run as high priority
 * work queue job.
 */
static irqreturn_t irq_handler_upper_half (int port_name, void* dev_id ) /* , struct pt_regs *regs*/
{
	irqreturn_t ret = IRQ_NONE;
	vr_osk_irq_object_t *irq_object = (vr_osk_irq_object_t *)dev_id;

	if (_VR_OSK_ERR_OK == irq_object->uhandler(irq_object->data)) {
		ret = IRQ_HANDLED;
	}

	return ret;
}
