/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_USER_SETTINGS_DB_H__
#define __VR_USER_SETTINGS_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vr_uk_types.h"

/** @brief Set Vr user setting in DB
 *
 * Update the DB with a new value for \a setting. If the value is different from theprevious set value running sessions will be notified of the change.
 *
 * @param setting the setting to be changed
 * @param value the new value to set
 */
void vr_set_user_setting(_vr_uk_user_setting_t setting, u32 value);

/** @brief Get current Vr user setting value from DB
 *
 * @param setting the setting to extract
 * @return the value of the selected setting
 */
u32 vr_get_user_setting(_vr_uk_user_setting_t setting);

#ifdef __cplusplus
}
#endif
#endif  /* __VR_KERNEL_USER_SETTING__ */
