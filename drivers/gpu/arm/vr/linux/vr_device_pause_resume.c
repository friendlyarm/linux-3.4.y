/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_device_pause_resume.c
 * Implementation of the Vr pause/resume functionality
 */

#include <linux/module.h>
#include <linux/vr/vr_utgard.h>
#include "vr_gp_scheduler.h"
#include "vr_pp_scheduler.h"

void vr_dev_pause(void)
{
	vr_gp_scheduler_suspend();
	vr_pp_scheduler_suspend();
	vr_group_power_off(VR_FALSE);
	vr_l2_cache_pause_all(VR_TRUE);
}

EXPORT_SYMBOL(vr_dev_pause);

void vr_dev_resume(void)
{
	vr_l2_cache_pause_all(VR_FALSE);
	vr_gp_scheduler_resume();
	vr_pp_scheduler_resume();
}

EXPORT_SYMBOL(vr_dev_resume);
