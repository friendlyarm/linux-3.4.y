/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_KERNEL_LINUX_H__
#define __VR_KERNEL_LINUX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/cdev.h>     /* character device definitions */
#include "vr_kernel_license.h"
#include "vr_osk_types.h"

extern struct platform_device *vr_platform_device;

#if VR_LICENSE_IS_GPL
/* Defined in vr_osk_irq.h */
extern struct workqueue_struct * vr_wq_normal;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VR_KERNEL_LINUX_H__ */
