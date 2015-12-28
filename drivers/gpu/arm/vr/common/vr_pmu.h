/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2009-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_platform.h
 * Platform specific Vr driver functions
 */

#ifndef __VR_PMU_H__
#define __VR_PMU_H__

#include "vr_osk.h"

#define VR_GP_DOMAIN_INDEX	0
#define VR_PP0_DOMAIN_INDEX	1
#define VR_PP1_DOMAIN_INDEX	2
#define VR_PP2_DOMAIN_INDEX	3
#define VR_PP3_DOMAIN_INDEX	4
#define VR_PP4_DOMAIN_INDEX	5
#define VR_PP5_DOMAIN_INDEX	6
#define VR_PP6_DOMAIN_INDEX	7
#define VR_PP7_DOMAIN_INDEX	8
#define VR_L20_DOMAIN_INDEX	9
#define VR_L21_DOMAIN_INDEX	10
#define VR_L22_DOMAIN_INDEX	11

#define VR_MAX_NUMBER_OF_DOMAINS	12

/* Record the domain config from the customer or default config */
extern u16 vr_pmu_global_domain_config[];

static inline u16 vr_pmu_get_domain_mask(u32 index)
{
	VR_DEBUG_ASSERT(VR_MAX_NUMBER_OF_DOMAINS > index);

	return vr_pmu_global_domain_config[index];
}

static inline void vr_pmu_set_domain_mask(u32 index, u16 value)
{
	VR_DEBUG_ASSERT(VR_MAX_NUMBER_OF_DOMAINS > index);

	vr_pmu_global_domain_config[index] = value;
}

static inline void vr_pmu_copy_domain_mask(void *src, u32 len)
{
	_vr_osk_memcpy(vr_pmu_global_domain_config, src, len);
}

struct vr_pmu_core;

/** @brief Initialisation of VR PMU
 *
 * This is called from entry point of the driver in order to create and intialize the PMU resource
 *
 * @param resource it will be a pointer to a PMU resource
 * @param number_of_pp_cores Number of found PP resources in configuration
 * @param number_of_l2_caches Number of found L2 cache resources in configuration
 * @return The created PMU object, or NULL in case of failure.
 */
struct vr_pmu_core *vr_pmu_create(_vr_osk_resource_t *resource);

/** @brief It deallocates the PMU resource
 *
 * This is called on the exit of the driver to terminate the PMU resource
 *
 * @param pmu Pointer to PMU core object to delete
 */
void vr_pmu_delete(struct vr_pmu_core *pmu);

/** @brief Reset PMU core
 *
 * @param pmu Pointer to PMU core object to reset
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t vr_pmu_reset(struct vr_pmu_core *pmu);

/** @brief VR GPU power down using VR in-built PMU
 *
 * Called to power down the specified cores. The mask will be saved so that \a
 * vr_pmu_power_up_all will bring the PMU back to the previous state set with
 * this function or \a vr_pmu_power_up.
 *
 * @param pmu Pointer to PMU core object to power down
 * @param mask Mask specifying which power domains to power down
 * @return _VR_OSK_ERR_OK on success otherwise, a suitable _vr_osk_errcode_t error.
 */
_vr_osk_errcode_t vr_pmu_power_down(struct vr_pmu_core *pmu, u32 mask);

/** @brief VR GPU power up using VR in-built PMU
 *
 * Called to power up the specified cores. The mask will be saved so that \a
 * vr_pmu_power_up_all will bring the PMU back to the previous state set with
 * this function or \a vr_pmu_power_down.
 *
 * @param pmu Pointer to PMU core object to power up
 * @param mask Mask specifying which power domains to power up
 * @return _VR_OSK_ERR_OK on success otherwise, a suitable _vr_osk_errcode_t error.
 */
_vr_osk_errcode_t vr_pmu_power_up(struct vr_pmu_core *pmu, u32 mask);

/** @brief VR GPU power down using VR in-built PMU
 *
 * called to power down all cores
 *
 * @param pmu Pointer to PMU core object to power down
 * @return _VR_OSK_ERR_OK on success otherwise, a suitable _vr_osk_errcode_t error.
 */
_vr_osk_errcode_t vr_pmu_power_down_all(struct vr_pmu_core *pmu);

/** @brief VR GPU power up using VR in-built PMU
 *
 * called to power up all cores
 *
 * @param pmu Pointer to PMU core object to power up
 * @return _VR_OSK_ERR_OK on success otherwise, a suitable _vr_osk_errcode_t error.
 */
_vr_osk_errcode_t vr_pmu_power_up_all(struct vr_pmu_core *pmu);

/** @brief Retrieves the Vr PMU core object (if any)
 *
 * @return The Vr PMU object, or NULL if no PMU exists.
 */
struct vr_pmu_core *vr_pmu_get_global_pmu_core(void);

#endif /* __VR_PMU_H__ */
