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
 * @file vr_osk_vr.h
 * Defines the OS abstraction layer which is specific for the Vr kernel device driver (OSK)
 */

#ifndef __VR_OSK_VR_H__
#define __VR_OSK_VR_H__

#include <linux/vr/vr_utgard.h>
#include <vr_osk.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup _vr_osk_miscellaneous
 * @{ */

/** @brief Struct with device specific configuration data
 */
struct _vr_osk_device_data {
	/* Dedicated GPU memory range (physical). */
	u32 dedicated_mem_start;
	u32 dedicated_mem_size;

	/* Shared GPU memory */
	u32 shared_mem_size;

	/* Frame buffer memory to be accessible by Vr GPU (physical) */
	u32 fb_start;
	u32 fb_size;

	/* Max runtime [ms] for jobs */
	int max_job_runtime;

	/* Report GPU utilization in this interval (specified in ms) */
	u32 utilization_interval;

	/* Function that will receive periodic GPU utilization numbers */
	void (*utilization_callback)(struct vr_gpu_utilization_data *data);

	/*
	 * Vr PMU switch delay.
	 * Only needed if the power gates are connected to the PMU in a high fanout
	 * network. This value is the number of Vr clock cycles it takes to
	 * enable the power gates and turn on the power mesh.
	 * This value will have no effect if a daisy chain implementation is used.
	 */
	u32 pmu_switch_delay;

	/* Vr Dynamic power domain configuration in sequence from 0-11
	 *  GP  PP0 PP1  PP2  PP3  PP4  PP5  PP6  PP7, L2$0 L2$1 L2$2
	 */
	u16 pmu_domain_config[12];

	/* Fuction that platform callback for freq tunning, needed when VR400_POWER_PERFORMANCE_POLICY enabled */
	int (*set_freq_callback)(unsigned int mhz);
};

/** @brief Find Vr GPU HW resource
 *
 * @param addr Address of Vr GPU resource to find
 * @param res Storage for resource information if resource is found.
 * @return _VR_OSK_ERR_OK on success, _VR_OSK_ERR_ITEM_NOT_FOUND if resource is not found
 */
_vr_osk_errcode_t _vr_osk_resource_find(u32 addr, _vr_osk_resource_t *res);


/** @brief Find Vr GPU HW base address
 *
 * @return 0 if resources are found, otherwise the Vr GPU component with lowest address.
 */
u32 _vr_osk_resource_base_address(void);

/** @brief Retrieve the Vr GPU specific data
 *
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t _vr_osk_device_data_get(struct _vr_osk_device_data *data);

/** @brief Determines if Vr GPU has been configured with shared interrupts.
 *
 * @return VR_TRUE if shared interrupts, VR_FALSE if not.
 */
vr_bool _vr_osk_shared_interrupts(void);

/** @} */ /* end group _vr_osk_miscellaneous */

/** @addtogroup _vr_osk_low_level_memory
 * @{ */

/** @brief Copy as much data as possible from src to dest, do not crash if src or dest isn't available.
 *
 * @param dest Destination buffer (limited to user space mapped Vr memory)
 * @param src Source buffer
 * @param size Number of bytes to copy
 * @return Number of bytes actually copied
 */
u32 _vr_osk_mem_write_safe(void *dest, const void *src, u32 size);

/** @} */ /* end group _vr_osk_low_level_memory */


#ifdef __cplusplus
}
#endif

#endif /* __VR_OSK_VR_H__ */
