/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_SCHEDULER_TYPES_H__
#define __VR_SCHEDULER_TYPES_H__

#include "vr_osk.h"

#define VR_SCHEDULER_JOB_ID_SPAN 65535

/**
 * Bitmask used for defered scheduling of subsystems.
 */
typedef u32 vr_scheduler_mask;

#define VR_SCHEDULER_MASK_GP (1<<0)
#define VR_SCHEDULER_MASK_PP (1<<1)

#define VR_SCHEDULER_MASK_EMPTY 0
#define VR_SCHEDULER_MASK_ALL (VR_SCHEDULER_MASK_GP | VR_SCHEDULER_MASK_PP)

typedef enum {
	VR_SCHEDULER_HINT_GP_BOUND = 0
#define VR_SCHEDULER_HINT_MAX        1
} vr_scheduler_hint;

#endif /* __VR_SCHEDULER_TYPES_H__ */
