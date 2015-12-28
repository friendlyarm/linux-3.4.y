/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2011-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_PM_H__
#define __VR_PM_H__

#include "vr_osk.h"

_vr_osk_errcode_t vr_pm_initialize(void);
void vr_pm_terminate(void);

/* Callback functions registered for the runtime PMM system */
void vr_pm_os_suspend(void);
void vr_pm_os_resume(void);
void vr_pm_runtime_suspend(void);
void vr_pm_runtime_resume(void);

void vr_pm_set_power_is_on(void);
vr_bool vr_pm_is_power_on(void);

#endif /* __VR_PM_H__ */
