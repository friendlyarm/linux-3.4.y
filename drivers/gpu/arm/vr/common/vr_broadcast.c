/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_broadcast.h"
#include "vr_kernel_common.h"
#include "vr_osk.h"

static const int bcast_unit_reg_size = 0x1000;
static const int bcast_unit_addr_broadcast_mask = 0x0;
static const int bcast_unit_addr_irq_override_mask = 0x4;

struct vr_bcast_unit {
	struct vr_hw_core hw_core;
	u32 current_mask;
};

struct vr_bcast_unit *vr_bcast_unit_create(const _vr_osk_resource_t *resource)
{
	struct vr_bcast_unit *bcast_unit = NULL;

	VR_DEBUG_ASSERT_POINTER(resource);
	VR_DEBUG_PRINT(2, ("Vr Broadcast unit: Creating Vr Broadcast unit: %s\n", resource->description));

	bcast_unit = _vr_osk_malloc(sizeof(struct vr_bcast_unit));
	if (NULL == bcast_unit) {
		VR_PRINT_ERROR(("Vr Broadcast unit: Failed to allocate memory for Broadcast unit\n"));
		return NULL;
	}

	if (_VR_OSK_ERR_OK == vr_hw_core_create(&bcast_unit->hw_core, resource, bcast_unit_reg_size)) {
		bcast_unit->current_mask = 0;
		vr_bcast_reset(bcast_unit);

		return bcast_unit;
	} else {
		VR_PRINT_ERROR(("Vr Broadcast unit: Failed map broadcast unit\n"));
	}

	_vr_osk_free(bcast_unit);

	return NULL;
}

void vr_bcast_unit_delete(struct vr_bcast_unit *bcast_unit)
{
	VR_DEBUG_ASSERT_POINTER(bcast_unit);

	vr_hw_core_delete(&bcast_unit->hw_core);
	_vr_osk_free(bcast_unit);
}

void vr_bcast_add_group(struct vr_bcast_unit *bcast_unit, struct vr_group *group)
{
	u32 bcast_id;
	u32 broadcast_mask;

	VR_DEBUG_ASSERT_POINTER(bcast_unit);
	VR_DEBUG_ASSERT_POINTER(group);

	bcast_id = vr_pp_core_get_bcast_id(vr_group_get_pp_core(group));

	broadcast_mask = bcast_unit->current_mask;

	broadcast_mask |= (bcast_id); /* add PP core to broadcast */
	broadcast_mask |= (bcast_id << 16); /* add MMU to broadcast */

	/* store mask so we can restore on reset */
	bcast_unit->current_mask = broadcast_mask;
}

void vr_bcast_remove_group(struct vr_bcast_unit *bcast_unit, struct vr_group *group)
{
	u32 bcast_id;
	u32 broadcast_mask;

	VR_DEBUG_ASSERT_POINTER(bcast_unit);
	VR_DEBUG_ASSERT_POINTER(group);

	bcast_id = vr_pp_core_get_bcast_id(vr_group_get_pp_core(group));

	broadcast_mask = bcast_unit->current_mask;

	broadcast_mask &= ~((bcast_id << 16) | bcast_id);

	/* store mask so we can restore on reset */
	bcast_unit->current_mask = broadcast_mask;
}

void vr_bcast_reset(struct vr_bcast_unit *bcast_unit)
{
	VR_DEBUG_ASSERT_POINTER(bcast_unit);

	/* set broadcast mask */
	vr_hw_core_register_write(&bcast_unit->hw_core,
	                            bcast_unit_addr_broadcast_mask,
	                            bcast_unit->current_mask);

	/* set IRQ override mask */
	vr_hw_core_register_write(&bcast_unit->hw_core,
	                            bcast_unit_addr_irq_override_mask,
	                            bcast_unit->current_mask & 0xFF);
}

void vr_bcast_disable(struct vr_bcast_unit *bcast_unit)
{
	VR_DEBUG_ASSERT_POINTER(bcast_unit);

	/* set broadcast mask */
	vr_hw_core_register_write(&bcast_unit->hw_core,
	                            bcast_unit_addr_broadcast_mask,
	                            0x0);

	/* set IRQ override mask */
	vr_hw_core_register_write(&bcast_unit->hw_core,
	                            bcast_unit_addr_irq_override_mask,
	                            0x0);
}
