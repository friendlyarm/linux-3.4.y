/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_KERNEL_UTILIZATION_H__
#define __VR_KERNEL_UTILIZATION_H__

#include <linux/vr/vr_utgard.h>
#include "vr_osk.h"

extern void (*vr_utilization_callback)(struct vr_gpu_utilization_data *data);

/**
 * Initialize/start the Vr GPU utilization metrics reporting.
 *
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t vr_utilization_init(void);

/**
 * Terminate the Vr GPU utilization metrics reporting
 */
void vr_utilization_term(void);

/**
 * Check if Vr utilization is enabled
 */
VR_STATIC_INLINE vr_bool vr_utilization_enabled(void)
{
	return (NULL != vr_utilization_callback);
}

/**
 * Should be called when a job is about to execute a GP job
 */
void vr_utilization_gp_start(void);

/**
 * Should be called when a job has completed executing a GP job
 */
void vr_utilization_gp_end(void);

/**
 * Should be called when a job is about to execute a PP job
 */
void vr_utilization_pp_start(void);

/**
 * Should be called when a job has completed executing a PP job
 */
void vr_utilization_pp_end(void);

/**
 * Should be called to stop the utilization timer during system suspend
 */
void vr_utilization_suspend(void);


#endif /* __VR_KERNEL_UTILIZATION_H__ */
