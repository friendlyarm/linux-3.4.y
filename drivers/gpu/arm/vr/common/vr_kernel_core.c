/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2007-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_osk.h"
#include "vr_osk_vr.h"
#include "vr_ukk.h"
#include "vr_kernel_core.h"
#include "vr_memory.h"
#include "vr_mem_validation.h"
#include "vr_mmu.h"
#include "vr_mmu_page_directory.h"
#include "vr_dlbu.h"
#include "vr_broadcast.h"
#include "vr_gp.h"
#include "vr_pp.h"
#include "vr_gp_scheduler.h"
#include "vr_pp_scheduler.h"
#include "vr_pp_job.h"
#include "vr_group.h"
#include "vr_pm.h"
#include "vr_pmu.h"
#include "vr_scheduler.h"
#include "vr_kernel_utilization.h"
#include "vr_l2_cache.h"
#include "vr_dma.h"
#include "vr_timeline.h"
#include "vr_soft_job.h"
#include "vr_pm_domain.h"
#if defined(CONFIG_VR400_PROFILING)
#include "vr_osk_profiling.h"
#endif
#if defined(CONFIG_VR400_INTERNAL_PROFILING)
#include "vr_profiling_internal.h"
#endif


/* Vr GPU memory. Real values come from module parameter or from device specific data */
unsigned int vr_dedicated_mem_start = 0;
unsigned int vr_dedicated_mem_size = 0;
unsigned int vr_shared_mem_size = 0;

/* Frame buffer memory to be accessible by Vr GPU */
int vr_fb_start = 0;
int vr_fb_size = 0;

/* Vr max job runtime */
extern int vr_max_job_runtime;

/** Start profiling from module load? */
int vr_boot_profiling = 0;

/** Limits for the number of PP cores behind each L2 cache. */
int vr_max_pp_cores_group_1 = 0xFF;
int vr_max_pp_cores_group_2 = 0xFF;

int vr_inited_pp_cores_group_1 = 0;
int vr_inited_pp_cores_group_2 = 0;

static _vr_product_id_t global_product_id = _VR_PRODUCT_ID_UNKNOWN;
static u32 global_gpu_base_address = 0;
static u32 global_gpu_major_version = 0;
static u32 global_gpu_minor_version = 0;

vr_bool vr_gpu_class_is_vr450 = VR_FALSE;

static _vr_osk_errcode_t vr_set_global_gpu_base_address(void)
{
	global_gpu_base_address = _vr_osk_resource_base_address();
	if (0 == global_gpu_base_address) {
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}

	return _VR_OSK_ERR_OK;
}

static u32 vr_get_bcast_id(_vr_osk_resource_t *resource_pp)
{
	switch (resource_pp->base - global_gpu_base_address) {
	case 0x08000:
	case 0x20000: /* fall-through for aliased mapping */
		return 0x01;
	case 0x0A000:
	case 0x22000: /* fall-through for aliased mapping */
		return 0x02;
	case 0x0C000:
	case 0x24000: /* fall-through for aliased mapping */
		return 0x04;
	case 0x0E000:
	case 0x26000: /* fall-through for aliased mapping */
		return 0x08;
	case 0x28000:
		return 0x10;
	case 0x2A000:
		return 0x20;
	case 0x2C000:
		return 0x40;
	case 0x2E000:
		return 0x80;
	default:
		return 0;
	}
}

static _vr_osk_errcode_t vr_parse_product_info(void)
{
	/*
	 * Vr-200 has the PP core first, while Vr-300, Vr-400 and Vr-450 have the GP core first.
	 * Look at the version register for the first PP core in order to determine the GPU HW revision.
	 */

	u32 first_pp_offset;
	_vr_osk_resource_t first_pp_resource;

	/* Find out where the first PP core is located */
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x8000, NULL)) {
		/* Vr-300/400/450 */
		first_pp_offset = 0x8000;
	} else {
		/* Vr-200 */
		first_pp_offset = 0x0000;
	}

	/* Find the first PP core resource (again) */
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + first_pp_offset, &first_pp_resource)) {
		/* Create a dummy PP object for this core so that we can read the version register */
		struct vr_group *group = vr_group_create(NULL, NULL, NULL);
		if (NULL != group) {
			struct vr_pp_core *pp_core = vr_pp_create(&first_pp_resource, group, VR_FALSE, vr_get_bcast_id(&first_pp_resource));
			if (NULL != pp_core) {
				u32 pp_version = vr_pp_core_get_version(pp_core);
				vr_group_delete(group);

				global_gpu_major_version = (pp_version >> 8) & 0xFF;
				global_gpu_minor_version = pp_version & 0xFF;

				switch (pp_version >> 16) {
				case VR200_PP_PRODUCT_ID:
					global_product_id = _VR_PRODUCT_ID_VR200;
					VR_DEBUG_PRINT(2, ("Found Vr GPU Vr-200 r%up%u\n", global_gpu_major_version, global_gpu_minor_version));
					VR_PRINT_ERROR(("Vr-200 is not supported by this driver.\n"));
					_vr_osk_abort();
					break;
				case VR300_PP_PRODUCT_ID:
					global_product_id = _VR_PRODUCT_ID_VR300;
					VR_DEBUG_PRINT(2, ("Found Vr GPU Vr-300 r%up%u\n", global_gpu_major_version, global_gpu_minor_version));
					break;
				case VR400_PP_PRODUCT_ID:
					global_product_id = _VR_PRODUCT_ID_VR400;
					VR_DEBUG_PRINT(2, ("Found Vr GPU Vr-400 MP r%up%u\n", global_gpu_major_version, global_gpu_minor_version));
					break;
				case VR450_PP_PRODUCT_ID:
					global_product_id = _VR_PRODUCT_ID_VR450;
					VR_DEBUG_PRINT(2, ("Found Vr GPU Vr-450 MP r%up%u\n", global_gpu_major_version, global_gpu_minor_version));
					break;
				default:
					VR_DEBUG_PRINT(2, ("Found unknown Vr GPU (r%up%u)\n", global_gpu_major_version, global_gpu_minor_version));
					return _VR_OSK_ERR_FAULT;
				}

				return _VR_OSK_ERR_OK;
			} else {
				VR_PRINT_ERROR(("Failed to create initial PP object\n"));
			}
		} else {
			VR_PRINT_ERROR(("Failed to create initial group object\n"));
		}
	} else {
		VR_PRINT_ERROR(("First PP core not specified in config file\n"));
	}

	return _VR_OSK_ERR_FAULT;
}


static void vr_resource_count(u32 *pp_count, u32 *l2_count)
{
	*pp_count = 0;
	*l2_count = 0;

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x08000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x0A000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x0C000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x0E000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x28000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x2A000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x2C000, NULL)) {
		++(*pp_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x2E000, NULL)) {
		++(*pp_count);
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x1000, NULL)) {
		++(*l2_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x10000, NULL)) {
		++(*l2_count);
	}
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x11000, NULL)) {
		++(*l2_count);
	}
}

static void vr_delete_groups(void)
{
	struct vr_group *group;

	group = vr_group_get_glob_group(0);
	while (NULL != group) {
		vr_group_delete(group);
		group = vr_group_get_glob_group(0);
	}

	VR_DEBUG_ASSERT(0 == vr_group_get_glob_num_groups());
}

static void vr_delete_l2_cache_cores(void)
{
	struct vr_l2_cache_core *l2;

	l2 = vr_l2_cache_core_get_glob_l2_core(0);
	while (NULL != l2) {
		vr_l2_cache_delete(l2);
		l2 = vr_l2_cache_core_get_glob_l2_core(0);
	}

	VR_DEBUG_ASSERT(0 == vr_l2_cache_core_get_glob_num_l2_cores());
}

static struct vr_l2_cache_core *vr_create_l2_cache_core(_vr_osk_resource_t *resource)
{
	struct vr_l2_cache_core *l2_cache = NULL;

	if (NULL != resource) {

		VR_DEBUG_PRINT(3, ("Found L2 cache %s\n", resource->description));

		l2_cache = vr_l2_cache_create(resource);
		if (NULL == l2_cache) {
			VR_PRINT_ERROR(("Failed to create L2 cache object\n"));
			return NULL;
		}
	}
	VR_DEBUG_PRINT(3, ("Created L2 cache core object\n"));

	return l2_cache;
}

static _vr_osk_errcode_t vr_parse_config_l2_cache(void)
{
	struct vr_l2_cache_core *l2_cache = NULL;

	if (vr_is_vr400()) {
		_vr_osk_resource_t l2_resource;
		if (_VR_OSK_ERR_OK != _vr_osk_resource_find(global_gpu_base_address + 0x1000, &l2_resource)) {
			VR_DEBUG_PRINT(3, ("Did not find required Vr L2 cache in config file\n"));
			return _VR_OSK_ERR_FAULT;
		}

		l2_cache = vr_create_l2_cache_core(&l2_resource);
		if (NULL == l2_cache) {
			return _VR_OSK_ERR_FAULT;
		}
		vr_pm_domain_add_l2(vr_pmu_get_domain_mask(VR_L20_DOMAIN_INDEX), l2_cache);
	} else if (vr_is_vr450()) {
		/*
		 * L2 for GP    at 0x10000
		 * L2 for PP0-3 at 0x01000
		 * L2 for PP4-7 at 0x11000 (optional)
		 */

		_vr_osk_resource_t l2_gp_resource;
		_vr_osk_resource_t l2_pp_grp0_resource;
		_vr_osk_resource_t l2_pp_grp1_resource;

		/* Make cluster for GP's L2 */
		if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x10000, &l2_gp_resource)) {
			VR_DEBUG_PRINT(3, ("Creating Vr-450 L2 cache core for GP\n"));
			l2_cache = vr_create_l2_cache_core(&l2_gp_resource);
			if (NULL == l2_cache) {
				return _VR_OSK_ERR_FAULT;
			}
			vr_pm_domain_add_l2(vr_pmu_get_domain_mask(VR_L20_DOMAIN_INDEX), l2_cache);
		} else {
			VR_DEBUG_PRINT(3, ("Did not find required Vr L2 cache for GP in config file\n"));
			return _VR_OSK_ERR_FAULT;
		}

		/* Find corresponding l2 domain */
		if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x1000, &l2_pp_grp0_resource)) {
			VR_DEBUG_PRINT(3, ("Creating Vr-450 L2 cache core for PP group 0\n"));
			l2_cache = vr_create_l2_cache_core(&l2_pp_grp0_resource);
			if (NULL == l2_cache) {
				return _VR_OSK_ERR_FAULT;
			}
			vr_pm_domain_add_l2(vr_pmu_get_domain_mask(VR_L21_DOMAIN_INDEX), l2_cache);
		} else {
			VR_DEBUG_PRINT(3, ("Did not find required Vr L2 cache for PP group 0 in config file\n"));
			return _VR_OSK_ERR_FAULT;
		}

		/* Second PP core group is optional, don't fail if we don't find it */
		if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x11000, &l2_pp_grp1_resource)) {
			VR_DEBUG_PRINT(3, ("Creating Vr-450 L2 cache core for PP group 1\n"));
			l2_cache = vr_create_l2_cache_core(&l2_pp_grp1_resource);
			if (NULL == l2_cache) {
				return _VR_OSK_ERR_FAULT;
			}
			vr_pm_domain_add_l2(vr_pmu_get_domain_mask(VR_L22_DOMAIN_INDEX), l2_cache);
		}
	}

	return _VR_OSK_ERR_OK;
}

static struct vr_group *vr_create_group(struct vr_l2_cache_core *cache,
        _vr_osk_resource_t *resource_mmu,
        _vr_osk_resource_t *resource_gp,
        _vr_osk_resource_t *resource_pp)
{
	struct vr_mmu_core *mmu;
	struct vr_group *group;

	VR_DEBUG_PRINT(3, ("Starting new group for MMU %s\n", resource_mmu->description));

	/* Create the group object */
	group = vr_group_create(cache, NULL, NULL);
	if (NULL == group) {
		VR_PRINT_ERROR(("Failed to create group object for MMU %s\n", resource_mmu->description));
		return NULL;
	}

	/* Create the MMU object inside group */
	mmu = vr_mmu_create(resource_mmu, group, VR_FALSE);
	if (NULL == mmu) {
		VR_PRINT_ERROR(("Failed to create MMU object\n"));
		vr_group_delete(group);
		return NULL;
	}

	if (NULL != resource_gp) {
		/* Create the GP core object inside this group */
		struct vr_gp_core *gp_core = vr_gp_create(resource_gp, group);
		if (NULL == gp_core) {
			/* No need to clean up now, as we will clean up everything linked in from the cluster when we fail this function */
			VR_PRINT_ERROR(("Failed to create GP object\n"));
			vr_group_delete(group);
			return NULL;
		}
	}

	if (NULL != resource_pp) {
		struct vr_pp_core *pp_core;

		/* Create the PP core object inside this group */
		pp_core = vr_pp_create(resource_pp, group, VR_FALSE, vr_get_bcast_id(resource_pp));
		if (NULL == pp_core) {
			/* No need to clean up now, as we will clean up everything linked in from the cluster when we fail this function */
			VR_PRINT_ERROR(("Failed to create PP object\n"));
			vr_group_delete(group);
			return NULL;
		}
	}

	/* Reset group */
	vr_group_lock(group);
	vr_group_reset(group);
	vr_group_unlock(group);

	return group;
}

static _vr_osk_errcode_t vr_create_virtual_group(_vr_osk_resource_t *resource_mmu_pp_bcast,
        _vr_osk_resource_t *resource_pp_bcast,
        _vr_osk_resource_t *resource_dlbu,
        _vr_osk_resource_t *resource_bcast)
{
	struct vr_mmu_core *mmu_pp_bcast_core;
	struct vr_pp_core *pp_bcast_core;
	struct vr_dlbu_core *dlbu_core;
	struct vr_bcast_unit *bcast_core;
	struct vr_group *group;

	VR_DEBUG_PRINT(2, ("Starting new virtual group for MMU PP broadcast core %s\n", resource_mmu_pp_bcast->description));

	/* Create the DLBU core object */
	dlbu_core = vr_dlbu_create(resource_dlbu);
	if (NULL == dlbu_core) {
		VR_PRINT_ERROR(("Failed to create DLBU object \n"));
		return _VR_OSK_ERR_FAULT;
	}

	/* Create the Broadcast unit core */
	bcast_core = vr_bcast_unit_create(resource_bcast);
	if (NULL == bcast_core) {
		VR_PRINT_ERROR(("Failed to create Broadcast unit object!\n"));
		vr_dlbu_delete(dlbu_core);
		return _VR_OSK_ERR_FAULT;
	}

	/* Create the group object */
	group = vr_group_create(NULL, dlbu_core, bcast_core);
	if (NULL == group) {
		VR_PRINT_ERROR(("Failed to create group object for MMU PP broadcast core %s\n", resource_mmu_pp_bcast->description));
		vr_bcast_unit_delete(bcast_core);
		vr_dlbu_delete(dlbu_core);
		return _VR_OSK_ERR_FAULT;
	}

	/* Create the MMU object inside group */
	mmu_pp_bcast_core = vr_mmu_create(resource_mmu_pp_bcast, group, VR_TRUE);
	if (NULL == mmu_pp_bcast_core) {
		VR_PRINT_ERROR(("Failed to create MMU PP broadcast object\n"));
		vr_group_delete(group);
		return _VR_OSK_ERR_FAULT;
	}

	/* Create the PP core object inside this group */
	pp_bcast_core = vr_pp_create(resource_pp_bcast, group, VR_TRUE, 0);
	if (NULL == pp_bcast_core) {
		/* No need to clean up now, as we will clean up everything linked in from the cluster when we fail this function */
		VR_PRINT_ERROR(("Failed to create PP object\n"));
		vr_group_delete(group);
		return _VR_OSK_ERR_FAULT;
	}

	return _VR_OSK_ERR_OK;
}

static _vr_osk_errcode_t vr_parse_config_groups(void)
{
	struct vr_group *group;
	int cluster_id_gp = 0;
	int cluster_id_pp_grp0 = 0;
	int cluster_id_pp_grp1 = 0;
	int i;

	_vr_osk_resource_t resource_gp;
	_vr_osk_resource_t resource_gp_mmu;
	_vr_osk_resource_t resource_pp[8];
	_vr_osk_resource_t resource_pp_mmu[8];
	_vr_osk_resource_t resource_pp_mmu_bcast;
	_vr_osk_resource_t resource_pp_bcast;
	_vr_osk_resource_t resource_dlbu;
	_vr_osk_resource_t resource_bcast;
	_vr_osk_errcode_t resource_gp_found;
	_vr_osk_errcode_t resource_gp_mmu_found;
	_vr_osk_errcode_t resource_pp_found[8];
	_vr_osk_errcode_t resource_pp_mmu_found[8];
	_vr_osk_errcode_t resource_pp_mmu_bcast_found;
	_vr_osk_errcode_t resource_pp_bcast_found;
	_vr_osk_errcode_t resource_dlbu_found;
	_vr_osk_errcode_t resource_bcast_found;

	if (!(vr_is_vr400() || vr_is_vr450())) {
		/* No known HW core */
		return _VR_OSK_ERR_FAULT;
	}

	if (VR_MAX_JOB_RUNTIME_DEFAULT == vr_max_job_runtime) {
		/* Group settings are not overridden by module parameters, so use device settings */
		struct _vr_osk_device_data data = { 0, };

		if (_VR_OSK_ERR_OK == _vr_osk_device_data_get(&data)) {
			/* Use device specific settings (if defined) */
			if (0 != data.max_job_runtime) {
				vr_max_job_runtime = data.max_job_runtime;
			}
		}
	}

	if (vr_is_vr450()) {
		/* Vr-450 have separate L2s for GP, and PP core group(s) */
		cluster_id_pp_grp0 = 1;
		cluster_id_pp_grp1 = 2;
	}

	resource_gp_found = _vr_osk_resource_find(global_gpu_base_address + 0x00000, &resource_gp);
	resource_gp_mmu_found = _vr_osk_resource_find(global_gpu_base_address + 0x03000, &resource_gp_mmu);
	resource_pp_found[0] = _vr_osk_resource_find(global_gpu_base_address + 0x08000, &(resource_pp[0]));
	resource_pp_found[1] = _vr_osk_resource_find(global_gpu_base_address + 0x0A000, &(resource_pp[1]));
	resource_pp_found[2] = _vr_osk_resource_find(global_gpu_base_address + 0x0C000, &(resource_pp[2]));
	resource_pp_found[3] = _vr_osk_resource_find(global_gpu_base_address + 0x0E000, &(resource_pp[3]));
	resource_pp_found[4] = _vr_osk_resource_find(global_gpu_base_address + 0x28000, &(resource_pp[4]));
	resource_pp_found[5] = _vr_osk_resource_find(global_gpu_base_address + 0x2A000, &(resource_pp[5]));
	resource_pp_found[6] = _vr_osk_resource_find(global_gpu_base_address + 0x2C000, &(resource_pp[6]));
	resource_pp_found[7] = _vr_osk_resource_find(global_gpu_base_address + 0x2E000, &(resource_pp[7]));
	resource_pp_mmu_found[0] = _vr_osk_resource_find(global_gpu_base_address + 0x04000, &(resource_pp_mmu[0]));
	resource_pp_mmu_found[1] = _vr_osk_resource_find(global_gpu_base_address + 0x05000, &(resource_pp_mmu[1]));
	resource_pp_mmu_found[2] = _vr_osk_resource_find(global_gpu_base_address + 0x06000, &(resource_pp_mmu[2]));
	resource_pp_mmu_found[3] = _vr_osk_resource_find(global_gpu_base_address + 0x07000, &(resource_pp_mmu[3]));
	resource_pp_mmu_found[4] = _vr_osk_resource_find(global_gpu_base_address + 0x1C000, &(resource_pp_mmu[4]));
	resource_pp_mmu_found[5] = _vr_osk_resource_find(global_gpu_base_address + 0x1D000, &(resource_pp_mmu[5]));
	resource_pp_mmu_found[6] = _vr_osk_resource_find(global_gpu_base_address + 0x1E000, &(resource_pp_mmu[6]));
	resource_pp_mmu_found[7] = _vr_osk_resource_find(global_gpu_base_address + 0x1F000, &(resource_pp_mmu[7]));


	if (vr_is_vr450()) {
		resource_bcast_found = _vr_osk_resource_find(global_gpu_base_address + 0x13000, &resource_bcast);
		resource_dlbu_found = _vr_osk_resource_find(global_gpu_base_address + 0x14000, &resource_dlbu);
		resource_pp_mmu_bcast_found = _vr_osk_resource_find(global_gpu_base_address + 0x15000, &resource_pp_mmu_bcast);
		resource_pp_bcast_found = _vr_osk_resource_find(global_gpu_base_address + 0x16000, &resource_pp_bcast);

		if (_VR_OSK_ERR_OK != resource_bcast_found ||
		    _VR_OSK_ERR_OK != resource_dlbu_found ||
		    _VR_OSK_ERR_OK != resource_pp_mmu_bcast_found ||
		    _VR_OSK_ERR_OK != resource_pp_bcast_found) {
			/* Missing mandatory core(s) for Vr-450 */
			VR_DEBUG_PRINT(2, ("Missing mandatory resources, Vr-450 needs DLBU, Broadcast unit, virtual PP core and virtual MMU\n"));
			return _VR_OSK_ERR_FAULT;
		}
	}

	if (_VR_OSK_ERR_OK != resource_gp_found ||
	    _VR_OSK_ERR_OK != resource_gp_mmu_found ||
	    _VR_OSK_ERR_OK != resource_pp_found[0] ||
	    _VR_OSK_ERR_OK != resource_pp_mmu_found[0]) {
		/* Missing mandatory core(s) */
		VR_DEBUG_PRINT(2, ("Missing mandatory resource, need at least one GP and one PP, both with a separate MMU\n"));
		return _VR_OSK_ERR_FAULT;
	}

	VR_DEBUG_ASSERT(1 <= vr_l2_cache_core_get_glob_num_l2_cores());
	group = vr_create_group(vr_l2_cache_core_get_glob_l2_core(cluster_id_gp), &resource_gp_mmu, &resource_gp, NULL);
	if (NULL == group) {
		return _VR_OSK_ERR_FAULT;
	}

	/* Add GP in group, for PMU ref count */
	vr_pm_domain_add_group(vr_pmu_get_domain_mask(VR_GP_DOMAIN_INDEX), group);

	/* Create group for first (and mandatory) PP core */
	VR_DEBUG_ASSERT(vr_l2_cache_core_get_glob_num_l2_cores() >= (cluster_id_pp_grp0 + 1)); /* >= 1 on Vr-300 and Vr-400, >= 2 on Vr-450 */
	group = vr_create_group(vr_l2_cache_core_get_glob_l2_core(cluster_id_pp_grp0), &resource_pp_mmu[0], NULL, &resource_pp[0]);
	if (NULL == group) {
		return _VR_OSK_ERR_FAULT;
	}

	/* Find corresponding pp domain */
	vr_pm_domain_add_group(vr_pmu_get_domain_mask(VR_PP0_DOMAIN_INDEX), group);

	vr_inited_pp_cores_group_1++;

	/* Create groups for rest of the cores in the first PP core group */
	for (i = 1; i < 4; i++) { /* First half of the PP cores belong to first core group */
		if (vr_inited_pp_cores_group_1 < vr_max_pp_cores_group_1) {
			if (_VR_OSK_ERR_OK == resource_pp_found[i] && _VR_OSK_ERR_OK == resource_pp_mmu_found[i]) {
				group = vr_create_group(vr_l2_cache_core_get_glob_l2_core(cluster_id_pp_grp0), &resource_pp_mmu[i], NULL, &resource_pp[i]);
				if (NULL == group) {
					return _VR_OSK_ERR_FAULT;
				}

				vr_pm_domain_add_group(vr_pmu_get_domain_mask(i + VR_PP0_DOMAIN_INDEX), group);

				vr_inited_pp_cores_group_1++;
			}
		}
	}

	/* Create groups for cores in the second PP core group */
	for (i = 4; i < 8; i++) { /* Second half of the PP cores belong to second core group */
		if (vr_inited_pp_cores_group_2 < vr_max_pp_cores_group_2) {
			if (_VR_OSK_ERR_OK == resource_pp_found[i] && _VR_OSK_ERR_OK == resource_pp_mmu_found[i]) {
				VR_DEBUG_ASSERT(vr_l2_cache_core_get_glob_num_l2_cores() >= 2); /* Only Vr-450 have a second core group */
				group = vr_create_group(vr_l2_cache_core_get_glob_l2_core(cluster_id_pp_grp1), &resource_pp_mmu[i], NULL, &resource_pp[i]);
				if (NULL == group) {
					return _VR_OSK_ERR_FAULT;
				}
				vr_pm_domain_add_group(vr_pmu_get_domain_mask(i + VR_PP0_DOMAIN_INDEX), group);
				vr_inited_pp_cores_group_2++;
			}
		}
	}

	if(vr_is_vr450()) {
		_vr_osk_errcode_t err = vr_create_virtual_group(&resource_pp_mmu_bcast, &resource_pp_bcast, &resource_dlbu, &resource_bcast);
		if (_VR_OSK_ERR_OK != err) {
			return err;
		}
	}

	vr_max_pp_cores_group_1 = vr_inited_pp_cores_group_1;
	vr_max_pp_cores_group_2 = vr_inited_pp_cores_group_2;
	VR_DEBUG_PRINT(2, ("%d+%d PP cores initialized\n", vr_inited_pp_cores_group_1, vr_inited_pp_cores_group_2));

	return _VR_OSK_ERR_OK;
}

static _vr_osk_errcode_t vr_check_shared_interrupts(void)
{
#if !defined(CONFIG_VR_SHARED_INTERRUPTS)
	if (VR_TRUE == _vr_osk_shared_interrupts()) {
		VR_PRINT_ERROR(("Shared interrupts detected, but driver support is not enabled\n"));
		return _VR_OSK_ERR_FAULT;
	}
#endif /* !defined(CONFIG_VR_SHARED_INTERRUPTS) */

	/* It is OK to compile support for shared interrupts even if Vr is not using it. */
	return _VR_OSK_ERR_OK;
}

static _vr_osk_errcode_t vr_create_pm_domains(void)
{
	int i;

	for (i = 0; i < VR_MAX_NUMBER_OF_DOMAINS; i++) {
		if (0x0 == vr_pmu_get_domain_mask(i)) continue;

		if (NULL == vr_pm_domain_create(vr_pmu_get_domain_mask(i))) {
			return _VR_OSK_ERR_NOMEM;
		}
	}

	return _VR_OSK_ERR_OK;
}

static void vr_use_default_pm_domain_config(void)
{
	u32 pp_count_gr1 = 0;
	u32 pp_count_gr2 = 0;
	u32 l2_count = 0;

	VR_DEBUG_ASSERT(0 != global_gpu_base_address);

	/* GP core */
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x00000, NULL)) {
		vr_pmu_set_domain_mask(VR_GP_DOMAIN_INDEX, 0x01);
	}

	/* PP0 - PP3 core */
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x08000, NULL)) {
		++pp_count_gr1;

		if (vr_is_vr400()) {
			vr_pmu_set_domain_mask(VR_PP0_DOMAIN_INDEX, 0x01<<2);
		} else if (vr_is_vr450()) {
			vr_pmu_set_domain_mask(VR_PP0_DOMAIN_INDEX, 0x01<<1);
		}
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x0A000, NULL)) {
		++pp_count_gr1;

		if (vr_is_vr400()) {
			vr_pmu_set_domain_mask(VR_PP1_DOMAIN_INDEX, 0x01<<3);
		} else if (vr_is_vr450()) {
			vr_pmu_set_domain_mask(VR_PP1_DOMAIN_INDEX, 0x01<<2);
		}
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x0C000, NULL)) {
		++pp_count_gr1;

		if (vr_is_vr400()) {
			vr_pmu_set_domain_mask(VR_PP2_DOMAIN_INDEX, 0x01<<4);
		} else if (vr_is_vr450()) {
			vr_pmu_set_domain_mask(VR_PP2_DOMAIN_INDEX, 0x01<<2);
		}
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x0E000, NULL)) {
		++pp_count_gr1;

		if (vr_is_vr400()) {
			vr_pmu_set_domain_mask(VR_PP3_DOMAIN_INDEX, 0x01<<5);
		} else if (vr_is_vr450()) {
			vr_pmu_set_domain_mask(VR_PP3_DOMAIN_INDEX, 0x01<<2);
		}
	}

	/* PP4 - PP7 */
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x28000, NULL)) {
		++pp_count_gr2;

		vr_pmu_set_domain_mask(VR_PP4_DOMAIN_INDEX, 0x01<<3);
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x2A000, NULL)) {
		++pp_count_gr2;

		vr_pmu_set_domain_mask(VR_PP5_DOMAIN_INDEX, 0x01<<3);
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x2C000, NULL)) {
		++pp_count_gr2;

		vr_pmu_set_domain_mask(VR_PP6_DOMAIN_INDEX, 0x01<<3);
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x2E000, NULL)) {
		++pp_count_gr2;

		vr_pmu_set_domain_mask(VR_PP7_DOMAIN_INDEX, 0x01<<3);
	}

	/* L2gp/L2PP0/L2PP4 */
	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x10000, NULL)) {
		++l2_count;

		if (vr_is_vr400()) {
			vr_pmu_set_domain_mask(VR_L20_DOMAIN_INDEX, 0x01<<1);
		} else if (vr_is_vr450()) {
			vr_pmu_set_domain_mask(VR_L20_DOMAIN_INDEX, 0x01<<0);
		}
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x1000, NULL)) {
		++l2_count;

		vr_pmu_set_domain_mask(VR_L21_DOMAIN_INDEX, 0x01<<1);
	}

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x11000, NULL)) {
		++l2_count;

		vr_pmu_set_domain_mask(VR_L22_DOMAIN_INDEX, 0x01<<3);
	}

	VR_DEBUG_PRINT(2, ("Using default PMU domain config: (%d) gr1_pp_cores, (%d) gr2_pp_cores, (%d) l2_count. \n", pp_count_gr1, pp_count_gr2, l2_count));
}

static void vr_set_pmu_global_domain_config(void)
{
	struct _vr_osk_device_data data = { 0, };
	int i = 0;

	if (_VR_OSK_ERR_OK == _vr_osk_device_data_get(&data)) {
		/* Check whether has customized pmu domain configure */
		for (i = 0; i < VR_MAX_NUMBER_OF_DOMAINS; i++) {
			if (0 != data.pmu_domain_config[i]) break;
		}

		if (VR_MAX_NUMBER_OF_DOMAINS == i) {
			vr_use_default_pm_domain_config();
		} else {
			/* Copy the customer config to global config */
			vr_pmu_copy_domain_mask(data.pmu_domain_config, sizeof(data.pmu_domain_config));
		}
	}
}

static _vr_osk_errcode_t vr_parse_config_pmu(void)
{
	_vr_osk_resource_t resource_pmu;

	VR_DEBUG_ASSERT(0 != global_gpu_base_address);

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x02000, &resource_pmu)) {
		struct vr_pmu_core *pmu;

		vr_set_pmu_global_domain_config();

		pmu = vr_pmu_create(&resource_pmu);
		if (NULL == pmu) {
			VR_PRINT_ERROR(("Failed to create PMU\n"));
			return _VR_OSK_ERR_FAULT;
		}
	}

	/* It's ok if the PMU doesn't exist */
	return _VR_OSK_ERR_OK;
}

static _vr_osk_errcode_t vr_parse_config_dma(void)
{
	_vr_osk_resource_t resource_dma;

	if (_VR_OSK_ERR_OK == _vr_osk_resource_find(global_gpu_base_address + 0x12000, &resource_dma)) {
		if (NULL == vr_dma_create(&resource_dma)) {
			return _VR_OSK_ERR_FAULT;
		}
		return _VR_OSK_ERR_OK;
	} else {
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}
}

static _vr_osk_errcode_t vr_parse_config_memory(void)
{
	_vr_osk_errcode_t ret;

	if (0 == vr_dedicated_mem_start && 0 == vr_dedicated_mem_size && 0 == vr_shared_mem_size) {
		/* Memory settings are not overridden by module parameters, so use device settings */
		struct _vr_osk_device_data data = { 0, };

		if (_VR_OSK_ERR_OK == _vr_osk_device_data_get(&data)) {
			/* Use device specific settings (if defined) */
			vr_dedicated_mem_start = data.dedicated_mem_start;
			vr_dedicated_mem_size = data.dedicated_mem_size;
			vr_shared_mem_size = data.shared_mem_size;
		}

		if (0 == vr_dedicated_mem_start && 0 == vr_dedicated_mem_size && 0 == vr_shared_mem_size) {
			/* No GPU memory specified */
			return _VR_OSK_ERR_INVALID_ARGS;
		}

		VR_DEBUG_PRINT(2, ("Using device defined memory settings (dedicated: 0x%08X@0x%08X, shared: 0x%08X)\n",
		                     vr_dedicated_mem_size, vr_dedicated_mem_start, vr_shared_mem_size));
	} else {
		VR_DEBUG_PRINT(2, ("Using module defined memory settings (dedicated: 0x%08X@0x%08X, shared: 0x%08X)\n",
		                     vr_dedicated_mem_size, vr_dedicated_mem_start, vr_shared_mem_size));
	}

	if (0 < vr_dedicated_mem_size && 0 != vr_dedicated_mem_start) {
		/* Dedicated memory */
		ret = vr_memory_core_resource_dedicated_memory(vr_dedicated_mem_start, vr_dedicated_mem_size);
		if (_VR_OSK_ERR_OK != ret) {
			VR_PRINT_ERROR(("Failed to register dedicated memory\n"));
			vr_memory_terminate();
			return ret;
		}
	}

	if (0 < vr_shared_mem_size) {
		/* Shared OS memory */
		ret = vr_memory_core_resource_os_memory(vr_shared_mem_size);
		if (_VR_OSK_ERR_OK != ret) {
			VR_PRINT_ERROR(("Failed to register shared OS memory\n"));
			vr_memory_terminate();
			return ret;
		}
	}

	if (0 == vr_fb_start && 0 == vr_fb_size) {
		/* Frame buffer settings are not overridden by module parameters, so use device settings */
		struct _vr_osk_device_data data = { 0, };

		if (_VR_OSK_ERR_OK == _vr_osk_device_data_get(&data)) {
			/* Use device specific settings (if defined) */
			vr_fb_start = data.fb_start;
			vr_fb_size = data.fb_size;
		}

		VR_DEBUG_PRINT(2, ("Using device defined frame buffer settings (0x%08X@0x%08X)\n",
		                     vr_fb_size, vr_fb_start));
	} else {
		VR_DEBUG_PRINT(2, ("Using module defined frame buffer settings (0x%08X@0x%08X)\n",
		                     vr_fb_size, vr_fb_start));
	}

	if (0 != vr_fb_size) {
		/* Register frame buffer */
		ret = vr_mem_validation_add_range(vr_fb_start, vr_fb_size);
		if (_VR_OSK_ERR_OK != ret) {
			VR_PRINT_ERROR(("Failed to register frame buffer memory region\n"));
			vr_memory_terminate();
			return ret;
		}
	}

	return _VR_OSK_ERR_OK;
}

static void vr_detect_gpu_class(void)
{
	u32 number_of_pp_cores = 0;
	u32 number_of_l2_caches = 0;

	vr_resource_count(&number_of_pp_cores, &number_of_l2_caches);
	if (number_of_l2_caches > 1) {
		vr_gpu_class_is_vr450 = VR_TRUE;
	}
}

_vr_osk_errcode_t vr_initialize_subsystems(void)
{
	_vr_osk_errcode_t err;
	struct vr_pmu_core *pmu;

	vr_pp_job_initialize();

	err = vr_session_initialize();
	if (_VR_OSK_ERR_OK != err) goto session_init_failed;

#if defined(CONFIG_VR400_PROFILING)
	err = _vr_osk_profiling_init(vr_boot_profiling ? VR_TRUE : VR_FALSE);
	if (_VR_OSK_ERR_OK != err) {
		/* No biggie if we weren't able to initialize the profiling */
		VR_PRINT_ERROR(("Failed to initialize profiling, feature will be unavailable\n"));
	}
#endif

	err = vr_memory_initialize();
	if (_VR_OSK_ERR_OK != err) goto memory_init_failed;

	/* Configure memory early. Memory allocation needed for vr_mmu_initialize. */
	err = vr_parse_config_memory();
	if (_VR_OSK_ERR_OK != err) goto parse_memory_config_failed;

	err = vr_set_global_gpu_base_address();
	if (_VR_OSK_ERR_OK != err) goto set_global_gpu_base_address_failed;

	/* Detect gpu class according to l2 cache number */
	vr_detect_gpu_class();

	err = vr_check_shared_interrupts();
	if (_VR_OSK_ERR_OK != err) goto check_shared_interrupts_failed;

	err = vr_pp_scheduler_initialize();
	if (_VR_OSK_ERR_OK != err) goto pp_scheduler_init_failed;

	/* Initialize the power management module */
	err = vr_pm_initialize();
	if (_VR_OSK_ERR_OK != err) goto pm_init_failed;

	/* Initialize the VR PMU */
	err = vr_parse_config_pmu();
	if (_VR_OSK_ERR_OK != err) goto parse_pmu_config_failed;

	/* Make sure the power stays on for the rest of this function */
	err = _vr_osk_pm_dev_ref_add();
	if (_VR_OSK_ERR_OK != err) goto pm_always_on_failed;

	/*
	 * If run-time PM is used, then the vr_pm module has now already been
	 * notified that the power now is on (through the resume callback functions).
	 * However, if run-time PM is not used, then there will probably not be any
	 * calls to the resume callback functions, so we need to explicitly tell it
	 * that the power is on.
	 */
	vr_pm_set_power_is_on();

	/* Reset PMU HW and ensure all Vr power domains are on */
	pmu = vr_pmu_get_global_pmu_core();
	if (NULL != pmu) {
		err = vr_pmu_reset(pmu);
		if (_VR_OSK_ERR_OK != err) goto pmu_reset_failed;
	}

	/* Detect which Vr GPU we are dealing with */
	err = vr_parse_product_info();
	if (_VR_OSK_ERR_OK != err) goto product_info_parsing_failed;

	/* The global_product_id is now populated with the correct Vr GPU */

	/* Create PM domains only if PMU exists */
	if (NULL != pmu) {
		err = vr_create_pm_domains();
		if (_VR_OSK_ERR_OK != err) goto pm_domain_failed;
	}

	/* Initialize MMU module */
	err = vr_mmu_initialize();
	if (_VR_OSK_ERR_OK != err) goto mmu_init_failed;

	if (vr_is_vr450()) {
		err = vr_dlbu_initialize();
		if (_VR_OSK_ERR_OK != err) goto dlbu_init_failed;

		err = vr_parse_config_dma();
		if (_VR_OSK_ERR_OK != err) goto dma_parsing_failed;
	}

	/* Start configuring the actual Vr hardware. */
	err = vr_parse_config_l2_cache();
	if (_VR_OSK_ERR_OK != err) goto config_parsing_failed;
	err = vr_parse_config_groups();
	if (_VR_OSK_ERR_OK != err) goto config_parsing_failed;

	/* Initialize the schedulers */
	err = vr_scheduler_initialize();
	if (_VR_OSK_ERR_OK != err) goto scheduler_init_failed;
	err = vr_gp_scheduler_initialize();
	if (_VR_OSK_ERR_OK != err) goto gp_scheduler_init_failed;

	/* PP scheduler population can't fail */
	vr_pp_scheduler_populate();

	/* Initialize the GPU utilization tracking */
	err = vr_utilization_init();
	if (_VR_OSK_ERR_OK != err) goto utilization_init_failed;

	/* Allowing the system to be turned off */
	_vr_osk_pm_dev_ref_dec();

	VR_SUCCESS; /* all ok */

	/* Error handling */

utilization_init_failed:
	vr_pp_scheduler_depopulate();
	vr_gp_scheduler_terminate();
gp_scheduler_init_failed:
	vr_scheduler_terminate();
scheduler_init_failed:
config_parsing_failed:
	vr_delete_groups(); /* Delete any groups not (yet) owned by a scheduler */
	vr_delete_l2_cache_cores(); /* Delete L2 cache cores even if config parsing failed. */
	{
		struct vr_dma_core *dma = vr_dma_get_global_dma_core();
		if (NULL != dma) vr_dma_delete(dma);
	}
dma_parsing_failed:
	vr_dlbu_terminate();
dlbu_init_failed:
	vr_mmu_terminate();
mmu_init_failed:
	vr_pm_domain_terminate();
pm_domain_failed:
	/* Nothing to roll back */
product_info_parsing_failed:
	/* Nothing to roll back */
pmu_reset_failed:
	/* Allowing the system to be turned off */
	_vr_osk_pm_dev_ref_dec();
pm_always_on_failed:
	pmu = vr_pmu_get_global_pmu_core();
	if (NULL != pmu) {
		vr_pmu_delete(pmu);
	}
parse_pmu_config_failed:
	vr_pm_terminate();
pm_init_failed:
	vr_pp_scheduler_terminate();
pp_scheduler_init_failed:
check_shared_interrupts_failed:
	global_gpu_base_address = 0;
set_global_gpu_base_address_failed:
	/* undoing vr_parse_config_memory() is done by vr_memory_terminate() */
parse_memory_config_failed:
	vr_memory_terminate();
memory_init_failed:
#if defined(CONFIG_VR400_PROFILING)
	_vr_osk_profiling_term();
#endif
	vr_session_terminate();
session_init_failed:
	vr_pp_job_terminate();
	return err;
}

void vr_terminate_subsystems(void)
{
	struct vr_pmu_core *pmu = vr_pmu_get_global_pmu_core();
	struct vr_dma_core *dma = vr_dma_get_global_dma_core();

	VR_DEBUG_PRINT(2, ("terminate_subsystems() called\n"));

	/* shut down subsystems in reverse order from startup */

	/* We need the GPU to be powered up for the terminate sequence */
	_vr_osk_pm_dev_ref_add();

	vr_utilization_term();
	vr_pp_scheduler_depopulate();
	vr_gp_scheduler_terminate();
	vr_scheduler_terminate();
	vr_delete_l2_cache_cores();
	if (vr_is_vr450()) {
		vr_dlbu_terminate();
	}
	vr_mmu_terminate();
	if (NULL != pmu) {
		vr_pmu_delete(pmu);
	}
	if (NULL != dma) {
		vr_dma_delete(dma);
	}
	vr_pm_terminate();
	vr_memory_terminate();
#if defined(CONFIG_VR400_PROFILING)
	_vr_osk_profiling_term();
#endif

	/* Allowing the system to be turned off */
	_vr_osk_pm_dev_ref_dec();

	vr_pp_scheduler_terminate();
	vr_session_terminate();

	vr_pp_job_terminate();
}

_vr_product_id_t vr_kernel_core_get_product_id(void)
{
	return global_product_id;
}

u32 vr_kernel_core_get_gpu_major_version(void)
{
	return global_gpu_major_version;
}

u32 vr_kernel_core_get_gpu_minor_version(void)
{
	return global_gpu_minor_version;
}

_vr_osk_errcode_t _vr_ukk_get_api_version( _vr_uk_get_api_version_s *args )
{
	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	/* check compatability */
	if ( args->version == _VR_UK_API_VERSION ) {
		args->compatible = 1;
	} else {
		args->compatible = 0;
	}

	args->version = _VR_UK_API_VERSION; /* report our version */

	/* success regardless of being compatible or not */
	VR_SUCCESS;
}

_vr_osk_errcode_t _vr_ukk_wait_for_notification( _vr_uk_wait_for_notification_s *args )
{
	_vr_osk_errcode_t err;
	_vr_osk_notification_t * notification;
	_vr_osk_notification_queue_t *queue;

	/* check input */
	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	queue = ((struct vr_session_data *)args->ctx)->ioctl_queue;

	/* if the queue does not exist we're currently shutting down */
	if (NULL == queue) {
		VR_DEBUG_PRINT(1, ("No notification queue registered with the session. Asking userspace to stop querying\n"));
		args->type = _VR_NOTIFICATION_CORE_SHUTDOWN_IN_PROGRESS;
		VR_SUCCESS;
	}

	/* receive a notification, might sleep */
	err = _vr_osk_notification_queue_receive(queue, &notification);
	if (_VR_OSK_ERR_OK != err) {
		VR_ERROR(err); /* errcode returned, pass on to caller */
	}

	/* copy the buffer to the user */
	args->type = (_vr_uk_notification_type)notification->notification_type;
	_vr_osk_memcpy(&args->data, notification->result_buffer, notification->result_buffer_size);

	/* finished with the notification */
	_vr_osk_notification_delete( notification );

	VR_SUCCESS; /* all ok */
}

_vr_osk_errcode_t _vr_ukk_post_notification( _vr_uk_post_notification_s *args )
{
	_vr_osk_notification_t * notification;
	_vr_osk_notification_queue_t *queue;

	/* check input */
	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	queue = ((struct vr_session_data *)args->ctx)->ioctl_queue;

	/* if the queue does not exist we're currently shutting down */
	if (NULL == queue) {
		VR_DEBUG_PRINT(1, ("No notification queue registered with the session. Asking userspace to stop querying\n"));
		VR_SUCCESS;
	}

	notification = _vr_osk_notification_create(args->type, 0);
	if (NULL == notification) {
		VR_PRINT_ERROR( ("Failed to create notification object\n"));
		return _VR_OSK_ERR_NOMEM;
	}

	_vr_osk_notification_queue_send(queue, notification);

	VR_SUCCESS; /* all ok */
}

_vr_osk_errcode_t _vr_ukk_request_high_priority( _vr_uk_request_high_priority_s *args )
{
	struct vr_session_data *session;

	VR_DEBUG_ASSERT_POINTER(args);
	VR_CHECK_NON_NULL(args->ctx, _VR_OSK_ERR_INVALID_ARGS);

	session = (struct vr_session_data *) args->ctx;

	if (!session->use_high_priority_job_queue) {
		session->use_high_priority_job_queue = VR_TRUE;
		VR_DEBUG_PRINT(2, ("Session 0x%08X with pid %d was granted higher priority.\n", session, _vr_osk_get_pid()));
	}

	VR_SUCCESS;
}

_vr_osk_errcode_t _vr_ukk_open(void **context)
{
	u32 i;
	struct vr_session_data *session;

	/* allocated struct to track this session */
	session = (struct vr_session_data *)_vr_osk_calloc(1, sizeof(struct vr_session_data));
	VR_CHECK_NON_NULL(session, _VR_OSK_ERR_NOMEM);

	VR_DEBUG_PRINT(3, ("Session starting\n"));

	/* create a response queue for this session */
	session->ioctl_queue = _vr_osk_notification_queue_init();
	if (NULL == session->ioctl_queue) {
		_vr_osk_free(session);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	session->page_directory = vr_mmu_pagedir_alloc();
	if (NULL == session->page_directory) {
		_vr_osk_notification_queue_term(session->ioctl_queue);
		_vr_osk_free(session);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	if (_VR_OSK_ERR_OK != vr_mmu_pagedir_map(session->page_directory, VR_DLBU_VIRT_ADDR, _VR_OSK_VR_PAGE_SIZE)) {
		VR_PRINT_ERROR(("Failed to map DLBU page into session\n"));
		_vr_osk_notification_queue_term(session->ioctl_queue);
		_vr_osk_free(session);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	if (0 != vr_dlbu_phys_addr) {
		vr_mmu_pagedir_update(session->page_directory, VR_DLBU_VIRT_ADDR, vr_dlbu_phys_addr,
		                        _VR_OSK_VR_PAGE_SIZE, VR_MMU_FLAGS_DEFAULT);
	}

	if (_VR_OSK_ERR_OK != vr_memory_session_begin(session)) {
		vr_mmu_pagedir_free(session->page_directory);
		_vr_osk_notification_queue_term(session->ioctl_queue);
		_vr_osk_free(session);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	/* Create soft system. */
	session->soft_job_system = vr_soft_job_system_create(session);
	if (NULL == session->soft_job_system) {
		vr_memory_session_end(session);
		vr_mmu_pagedir_free(session->page_directory);
		_vr_osk_notification_queue_term(session->ioctl_queue);
		_vr_osk_free(session);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

	/* Create timeline system. */
	session->timeline_system = vr_timeline_system_create(session);
	if (NULL == session->timeline_system) {
		vr_soft_job_system_destroy(session->soft_job_system);
		vr_memory_session_end(session);
		vr_mmu_pagedir_free(session->page_directory);
		_vr_osk_notification_queue_term(session->ioctl_queue);
		_vr_osk_free(session);
		VR_ERROR(_VR_OSK_ERR_NOMEM);
	}

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	if ( _VR_OSK_ERR_OK != _vr_osk_atomic_init(&session->number_of_window_jobs, 0)) {
		VR_DEBUG_PRINT_ERROR(("Initialization of atomic number_of_window_jobs failed.\n"));
		vr_timeline_system_destroy(session->timeline_system);
		vr_soft_job_system_destroy(session->soft_job_system);
		vr_memory_session_end(session);
		vr_mmu_pagedir_free(session->page_directory);
		_vr_osk_notification_queue_term(session->ioctl_queue);
		_vr_osk_free(session);
		return _VR_OSK_ERR_FAULT;
	}
#endif

	session->use_high_priority_job_queue = VR_FALSE;

	/* Initialize list of PP jobs on this session. */
	_VR_OSK_INIT_LIST_HEAD(&session->pp_job_list);

	/* Initialize the pp_job_fb_lookup_list array used to quickly lookup jobs from a given frame builder */
	for (i = 0; i < VR_PP_JOB_FB_LOOKUP_LIST_SIZE; ++i) {
		_VR_OSK_INIT_LIST_HEAD(&session->pp_job_fb_lookup_list[i]);
	}

	*context = (void*)session;

	/* Add session to the list of all sessions. */
	vr_session_add(session);

	VR_DEBUG_PRINT(2, ("Session started\n"));
	VR_SUCCESS;
}

_vr_osk_errcode_t _vr_ukk_close(void **context)
{
	struct vr_session_data *session;
	VR_CHECK_NON_NULL(context, _VR_OSK_ERR_INVALID_ARGS);
	session = (struct vr_session_data *)*context;

	VR_DEBUG_PRINT(3, ("Session ending\n"));

	VR_DEBUG_ASSERT_POINTER(session->soft_job_system);
	VR_DEBUG_ASSERT_POINTER(session->timeline_system);

	/* Remove session from list of all sessions. */
	vr_session_remove(session);

	/* This flag is used to prevent queueing of jobs due to activation. */
	session->is_aborting = VR_TRUE;

	/* Stop the soft job timer. */
	vr_timeline_system_stop_timer(session->timeline_system);

	/* Abort queued and running GP and PP jobs. */
	vr_gp_scheduler_abort_session(session);
	vr_pp_scheduler_abort_session(session);

	/* Abort the soft job system. */
	vr_soft_job_system_abort(session->soft_job_system);

	/* Force execution of all pending bottom half processing for GP and PP. */
	_vr_osk_wq_flush();

	/* The session PP list should now be empty. */
	VR_DEBUG_ASSERT(_vr_osk_list_empty(&session->pp_job_list));

	/* At this point the GP and PP scheduler no longer has any jobs queued or running from this
	 * session, and all soft jobs in the soft job system has been destroyed. */

	/* Any trackers left in the timeline system are directly or indirectly waiting on external
	 * sync fences.  Cancel all sync fence waiters to trigger activation of all remaining
	 * trackers.  This call will sleep until all timelines are empty. */
	vr_timeline_system_abort(session->timeline_system);

	/* Flush pending work.
	 * Needed to make sure all bottom half processing related to this
	 * session has been completed, before we free internal data structures.
	 */
	_vr_osk_wq_flush();

	/* Destroy timeline system. */
	vr_timeline_system_destroy(session->timeline_system);
	session->timeline_system = NULL;

	/* Destroy soft system. */
	vr_soft_job_system_destroy(session->soft_job_system);
	session->soft_job_system = NULL;

	VR_DEBUG_CODE( {
		/* Check that the pp_job_fb_lookup_list array is empty. */
		u32 i;
		for (i = 0; i < VR_PP_JOB_FB_LOOKUP_LIST_SIZE; ++i)
		{
			VR_DEBUG_ASSERT(_vr_osk_list_empty(&session->pp_job_fb_lookup_list[i]));
		}
	});

	/* Free remaining memory allocated to this session */
	vr_memory_session_end(session);

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
	_vr_osk_atomic_term(&session->number_of_window_jobs);
#endif

	/* Free session data structures */
	vr_mmu_pagedir_free(session->page_directory);
	_vr_osk_notification_queue_term(session->ioctl_queue);
	_vr_osk_free(session);

	*context = NULL;

	VR_DEBUG_PRINT(2, ("Session has ended\n"));

	VR_SUCCESS;
}

#if VR_STATE_TRACKING
u32 _vr_kernel_core_dump_state(char* buf, u32 size)
{
	int n = 0; /* Number of bytes written to buf */

	n += vr_gp_scheduler_dump_state(buf + n, size - n);
	n += vr_pp_scheduler_dump_state(buf + n, size - n);

	return n;
}
#endif
