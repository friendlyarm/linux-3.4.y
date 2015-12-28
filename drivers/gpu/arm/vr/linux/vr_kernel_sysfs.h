/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_KERNEL_SYSFS_H__
#define __VR_KERNEL_SYSFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/device.h>

#define VR_PROC_DIR "driver/vr"

int vr_sysfs_register(const char *vr_dev_name);
int vr_sysfs_unregister(void);

#ifdef __cplusplus
}
#endif

#endif /* __VR_KERNEL_LINUX_H__ */
