/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_dlbu.h"
#include "vr_memory.h"
#include "vr_pp.h"
#include "vr_group.h"
#include "vr_osk.h"
#include "vr_hw_core.h"

/**
 * Size of DLBU registers in bytes
 */
#define VR_DLBU_SIZE 0x400

u32 vr_dlbu_phys_addr = 0;
static vr_io_address vr_dlbu_cpu_addr = 0;

/**
 * DLBU register numbers
 * Used in the register read/write routines.
 * See the hardware documentation for more information about each register
 */
typedef enum vr_dlbu_register {
	VR_DLBU_REGISTER_MASTER_TLLIST_PHYS_ADDR = 0x0000, /**< Master tile list physical base address;
	                                                     31:12 Physical address to the page used for the DLBU
	                                                     0 DLBU enable - set this bit to 1 enables the AXI bus
	                                                     between PPs and L2s, setting to 0 disables the router and
	                                                     no further transactions are sent to DLBU */
	VR_DLBU_REGISTER_MASTER_TLLIST_VADDR     = 0x0004, /**< Master tile list virtual base address;
	                                                     31:12 Virtual address to the page used for the DLBU */
	VR_DLBU_REGISTER_TLLIST_VBASEADDR        = 0x0008, /**< Tile list virtual base address;
	                                                     31:12 Virtual address to the tile list. This address is used when
	                                                     calculating the call address sent to PP.*/
	VR_DLBU_REGISTER_FB_DIM                  = 0x000C, /**< Framebuffer dimension;
	                                                     23:16 Number of tiles in Y direction-1
	                                                     7:0 Number of tiles in X direction-1 */
	VR_DLBU_REGISTER_TLLIST_CONF             = 0x0010, /**< Tile list configuration;
	                                                     29:28 select the size of each allocated block: 0=128 bytes, 1=256, 2=512, 3=1024
	                                                     21:16 2^n number of tiles to be binned to one tile list in Y direction
	                                                     5:0 2^n number of tiles to be binned to one tile list in X direction */
	VR_DLBU_REGISTER_START_TILE_POS          = 0x0014, /**< Start tile positions;
	                                                     31:24 start position in Y direction for group 1
	                                                     23:16 start position in X direction for group 1
	                                                     15:8 start position in Y direction for group 0
	                                                     7:0 start position in X direction for group 0 */
	VR_DLBU_REGISTER_PP_ENABLE_MASK          = 0x0018, /**< PP enable mask;
	                                                     7 enable PP7 for load balancing
	                                                     6 enable PP6 for load balancing
	                                                     5 enable PP5 for load balancing
	                                                     4 enable PP4 for load balancing
	                                                     3 enable PP3 for load balancing
	                                                     2 enable PP2 for load balancing
	                                                     1 enable PP1 for load balancing
	                                                     0 enable PP0 for load balancing */
} vr_dlbu_register;

typedef enum {
	PP0ENABLE = 0,
	PP1ENABLE,
	PP2ENABLE,
	PP3ENABLE,
	PP4ENABLE,
	PP5ENABLE,
	PP6ENABLE,
	PP7ENABLE
} vr_dlbu_pp_enable;

struct vr_dlbu_core {
	struct vr_hw_core     hw_core;           /**< Common for all HW cores */
	u32                     pp_cores_mask;     /**< This is a mask for the PP cores whose operation will be controlled by LBU
	                                              see VR_DLBU_REGISTER_PP_ENABLE_MASK register */
};

_vr_osk_errcode_t vr_dlbu_initialize(void)
{

	VR_DEBUG_PRINT(2, ("Vr DLBU: Initializing\n"));

	if (_VR_OSK_ERR_OK == vr_mmu_get_table_page(&vr_dlbu_phys_addr, &vr_dlbu_cpu_addr)) {
		VR_SUCCESS;
	}

	return _VR_OSK_ERR_FAULT;
}

void vr_dlbu_terminate(void)
{
	VR_DEBUG_PRINT(3, ("Vr DLBU: terminating\n"));

	vr_mmu_release_table_page(vr_dlbu_phys_addr, vr_dlbu_cpu_addr);
}

struct vr_dlbu_core *vr_dlbu_create(const _vr_osk_resource_t * resource)
{
	struct vr_dlbu_core *core = NULL;

	VR_DEBUG_PRINT(2, ("Vr DLBU: Creating Vr dynamic load balancing unit: %s\n", resource->description));

	core = _vr_osk_malloc(sizeof(struct vr_dlbu_core));
	if (NULL != core) {
		if (_VR_OSK_ERR_OK == vr_hw_core_create(&core->hw_core, resource, VR_DLBU_SIZE)) {
			core->pp_cores_mask = 0;
			if (_VR_OSK_ERR_OK == vr_dlbu_reset(core)) {
				return core;
			}
			VR_PRINT_ERROR(("Failed to reset DLBU %s\n", core->hw_core.description));
			vr_hw_core_delete(&core->hw_core);
		}

		_vr_osk_free(core);
	} else {
		VR_PRINT_ERROR(("Vr DLBU: Failed to allocate memory for DLBU core\n"));
	}

	return NULL;
}

void vr_dlbu_delete(struct vr_dlbu_core *dlbu)
{
	VR_DEBUG_ASSERT_POINTER(dlbu);

	vr_dlbu_reset(dlbu);
	vr_hw_core_delete(&dlbu->hw_core);
	_vr_osk_free(dlbu);
}

_vr_osk_errcode_t vr_dlbu_reset(struct vr_dlbu_core *dlbu)
{
	u32 dlbu_registers[7];
	_vr_osk_errcode_t err = _VR_OSK_ERR_FAULT;
	VR_DEBUG_ASSERT_POINTER(dlbu);

	VR_DEBUG_PRINT(4, ("Vr DLBU: vr_dlbu_reset: %s\n", dlbu->hw_core.description));

	dlbu_registers[0] = vr_dlbu_phys_addr | 1; /* bit 0 enables the whole core */
	dlbu_registers[1] = VR_DLBU_VIRT_ADDR;
	dlbu_registers[2] = 0;
	dlbu_registers[3] = 0;
	dlbu_registers[4] = 0;
	dlbu_registers[5] = 0;
	dlbu_registers[6] = dlbu->pp_cores_mask;

	/* write reset values to core registers */
	vr_hw_core_register_write_array_relaxed(&dlbu->hw_core, VR_DLBU_REGISTER_MASTER_TLLIST_PHYS_ADDR, dlbu_registers, 7);

	err = _VR_OSK_ERR_OK;

	return err;
}

void vr_dlbu_update_mask(struct vr_dlbu_core *dlbu)
{
	VR_DEBUG_ASSERT_POINTER(dlbu);

	vr_hw_core_register_write(&dlbu->hw_core, VR_DLBU_REGISTER_PP_ENABLE_MASK, dlbu->pp_cores_mask);
}

void vr_dlbu_add_group(struct vr_dlbu_core *dlbu, struct vr_group *group)
{
	struct vr_pp_core *pp_core;
	u32 bcast_id;

	VR_DEBUG_ASSERT_POINTER( dlbu );
	VR_DEBUG_ASSERT_POINTER( group );

	pp_core = vr_group_get_pp_core(group);
	bcast_id = vr_pp_core_get_bcast_id(pp_core);

	dlbu->pp_cores_mask |= bcast_id;
	VR_DEBUG_PRINT(3, ("Vr DLBU: Adding core[%d] New mask= 0x%02x\n", bcast_id , dlbu->pp_cores_mask));
}

/* Remove a group from the DLBU */
void vr_dlbu_remove_group(struct vr_dlbu_core *dlbu, struct vr_group *group)
{
	struct vr_pp_core *pp_core;
	u32 bcast_id;

	VR_DEBUG_ASSERT_POINTER( dlbu );
	VR_DEBUG_ASSERT_POINTER( group );

	pp_core = vr_group_get_pp_core(group);
	bcast_id = vr_pp_core_get_bcast_id(pp_core);

	dlbu->pp_cores_mask &= ~bcast_id;
	VR_DEBUG_PRINT(3, ("Vr DLBU: Removing core[%d] New mask= 0x%02x\n", bcast_id, dlbu->pp_cores_mask));
}

/* Configure the DLBU for \a job. This needs to be done before the job is started on the groups in the DLBU. */
void vr_dlbu_config_job(struct vr_dlbu_core *dlbu, struct vr_pp_job *job)
{
	u32 *registers;
	VR_DEBUG_ASSERT(job);
	registers = vr_pp_job_get_dlbu_registers(job);
	VR_DEBUG_PRINT(4, ("Vr DLBU: Starting job\n"));

	/* Writing 4 registers:
	 * DLBU registers except the first two (written once at DLBU initialisation / reset) and the PP_ENABLE_MASK register */
	vr_hw_core_register_write_array_relaxed(&dlbu->hw_core, VR_DLBU_REGISTER_TLLIST_VBASEADDR, registers, 4);

}
