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
#include "vr_osk.h"
#include "vr_osk_list.h"
#include "vr_ukk.h"

#include "vr_mmu.h"
#include "vr_hw_core.h"
#include "vr_group.h"
#include "vr_mmu_page_directory.h"

/**
 * Size of the MMU registers in bytes
 */
#define VR_MMU_REGISTERS_SIZE 0x24

/**
 * MMU commands
 * These are the commands that can be sent
 * to the MMU unit.
 */
typedef enum vr_mmu_command {
	VR_MMU_COMMAND_ENABLE_PAGING = 0x00, /**< Enable paging (memory translation) */
	VR_MMU_COMMAND_DISABLE_PAGING = 0x01, /**< Disable paging (memory translation) */
	VR_MMU_COMMAND_ENABLE_STALL = 0x02, /**<  Enable stall on page fault */
	VR_MMU_COMMAND_DISABLE_STALL = 0x03, /**< Disable stall on page fault */
	VR_MMU_COMMAND_ZAP_CACHE = 0x04, /**< Zap the entire page table cache */
	VR_MMU_COMMAND_PAGE_FAULT_DONE = 0x05, /**< Page fault processed */
	VR_MMU_COMMAND_HARD_RESET = 0x06 /**< Reset the MMU back to power-on settings */
} vr_mmu_command;

static void vr_mmu_probe_trigger(void *data);
static _vr_osk_errcode_t vr_mmu_probe_ack(void *data);

VR_STATIC_INLINE _vr_osk_errcode_t vr_mmu_raw_reset(struct vr_mmu_core *mmu);

/* page fault queue flush helper pages
 * note that the mapping pointers are currently unused outside of the initialization functions */
static u32 vr_page_fault_flush_page_directory = VR_INVALID_PAGE;
static vr_io_address vr_page_fault_flush_page_directory_mapping = NULL;
static u32 vr_page_fault_flush_page_table = VR_INVALID_PAGE;
static vr_io_address vr_page_fault_flush_page_table_mapping = NULL;
static u32 vr_page_fault_flush_data_page = VR_INVALID_PAGE;
static vr_io_address vr_page_fault_flush_data_page_mapping = NULL;

/* an empty page directory (no address valid) which is active on any MMU not currently marked as in use */
static u32 vr_empty_page_directory_phys   = VR_INVALID_PAGE;
static vr_io_address vr_empty_page_directory_virt = NULL;


_vr_osk_errcode_t vr_mmu_initialize(void)
{
	/* allocate the helper pages */
	vr_empty_page_directory_phys = vr_allocate_empty_page(&vr_empty_page_directory_virt);
	if(0 == vr_empty_page_directory_phys) {
		VR_DEBUG_PRINT_ERROR(("Vr MMU: Could not allocate empty page directory.\n"));
		vr_empty_page_directory_phys = VR_INVALID_PAGE;
		return _VR_OSK_ERR_NOMEM;
	}

	if (_VR_OSK_ERR_OK != vr_create_fault_flush_pages(&vr_page_fault_flush_page_directory,
	        &vr_page_fault_flush_page_directory_mapping,
	        &vr_page_fault_flush_page_table,
	        &vr_page_fault_flush_page_table_mapping,
	        &vr_page_fault_flush_data_page,
	        &vr_page_fault_flush_data_page_mapping)) {
		VR_DEBUG_PRINT_ERROR(("Vr MMU: Could not allocate fault flush pages\n"));
		vr_free_empty_page(vr_empty_page_directory_phys, vr_empty_page_directory_virt);
		vr_empty_page_directory_phys = VR_INVALID_PAGE;
		vr_empty_page_directory_virt = NULL;
		return _VR_OSK_ERR_NOMEM;
	}

	return _VR_OSK_ERR_OK;
}

void vr_mmu_terminate(void)
{
	VR_DEBUG_PRINT(3, ("Vr MMU: terminating\n"));

	/* Free global helper pages */
	vr_free_empty_page(vr_empty_page_directory_phys, vr_empty_page_directory_virt);
	vr_empty_page_directory_phys = VR_INVALID_PAGE;
	vr_empty_page_directory_virt = NULL;

	/* Free the page fault flush pages */
	vr_destroy_fault_flush_pages(&vr_page_fault_flush_page_directory, &vr_page_fault_flush_page_directory_mapping,
	                               &vr_page_fault_flush_page_table, &vr_page_fault_flush_page_table_mapping,
	                               &vr_page_fault_flush_data_page, &vr_page_fault_flush_data_page_mapping);
}

struct vr_mmu_core *vr_mmu_create(_vr_osk_resource_t *resource, struct vr_group *group, vr_bool is_virtual)
{
	struct vr_mmu_core* mmu = NULL;

	VR_DEBUG_ASSERT_POINTER(resource);

	VR_DEBUG_PRINT(2, ("Vr MMU: Creating Vr MMU: %s\n", resource->description));

	mmu = _vr_osk_calloc(1,sizeof(struct vr_mmu_core));
	if (NULL != mmu) {
		if (_VR_OSK_ERR_OK == vr_hw_core_create(&mmu->hw_core, resource, VR_MMU_REGISTERS_SIZE)) {
			if (_VR_OSK_ERR_OK == vr_group_add_mmu_core(group, mmu)) {
				if (is_virtual) {
					/* Skip reset and IRQ setup for virtual MMU */
					return mmu;
				}

				if (_VR_OSK_ERR_OK == vr_mmu_reset(mmu)) {
					/* Setup IRQ handlers (which will do IRQ probing if needed) */
					mmu->irq = _vr_osk_irq_init(resource->irq,
					                              vr_group_upper_half_mmu,
					                              group,
					                              vr_mmu_probe_trigger,
					                              vr_mmu_probe_ack,
					                              mmu,
					                              resource->description);
					if (NULL != mmu->irq) {
						return mmu;
					} else {
						VR_PRINT_ERROR(("Vr MMU: Failed to setup interrupt handlers for MMU %s\n", mmu->hw_core.description));
					}
				}
				vr_group_remove_mmu_core(group);
			} else {
				VR_PRINT_ERROR(("Vr MMU: Failed to add core %s to group\n", mmu->hw_core.description));
			}
			vr_hw_core_delete(&mmu->hw_core);
		}

		_vr_osk_free(mmu);
	} else {
		VR_PRINT_ERROR(("Failed to allocate memory for MMU\n"));
	}

	return NULL;
}

void vr_mmu_delete(struct vr_mmu_core *mmu)
{
	if (NULL != mmu->irq) {
		_vr_osk_irq_term(mmu->irq);
	}

	vr_hw_core_delete(&mmu->hw_core);
	_vr_osk_free(mmu);
}

static void vr_mmu_enable_paging(struct vr_mmu_core *mmu)
{
	int i;

	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_ENABLE_PAGING);

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; ++i) {
		if (vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS) & VR_MMU_STATUS_BIT_PAGING_ENABLED) {
			break;
		}
	}
	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_PRINT_ERROR(("Enable paging request failed, MMU status is 0x%08X\n", vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS)));
	}
}

/**
 * Issues the enable stall command to the MMU and waits for HW to complete the request
 * @param mmu The MMU to enable paging for
 * @return VR_TRUE if HW stall was successfully engaged, otherwise VR_FALSE (req timed out)
 */
static vr_bool vr_mmu_enable_stall(struct vr_mmu_core *mmu)
{
	int i;
	u32 mmu_status = vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS);

	if ( 0 == (mmu_status & VR_MMU_STATUS_BIT_PAGING_ENABLED) ) {
		VR_DEBUG_PRINT(4, ("MMU stall is implicit when Paging is not enabled.\n"));
		return VR_TRUE;
	}

	if ( mmu_status & VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE ) {
		VR_DEBUG_PRINT(3, ("Aborting MMU stall request since it is in pagefault state.\n"));
		return VR_FALSE;
	}

	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_ENABLE_STALL);

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; ++i) {
		mmu_status = vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS);
		if (mmu_status & VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE) {
			break;
		}
		if ((mmu_status & VR_MMU_STATUS_BIT_STALL_ACTIVE) && (0 == (mmu_status & VR_MMU_STATUS_BIT_STALL_NOT_ACTIVE))) {
			break;
		}
		if (0 == (mmu_status & ( VR_MMU_STATUS_BIT_PAGING_ENABLED ))) {
			break;
		}
	}
	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_DEBUG_PRINT(2, ("Enable stall request failed, MMU status is 0x%08X\n", vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS)));
		return VR_FALSE;
	}

	if ( mmu_status & VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE ) {
		VR_DEBUG_PRINT(2, ("Aborting MMU stall request since it has a pagefault.\n"));
		return VR_FALSE;
	}

	return VR_TRUE;
}

/**
 * Issues the disable stall command to the MMU and waits for HW to complete the request
 * @param mmu The MMU to enable paging for
 */
static void vr_mmu_disable_stall(struct vr_mmu_core *mmu)
{
	int i;
	u32 mmu_status = vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS);

	if ( 0 == (mmu_status & VR_MMU_STATUS_BIT_PAGING_ENABLED )) {
		VR_DEBUG_PRINT(3, ("MMU disable skipped since it was not enabled.\n"));
		return;
	}
	if (mmu_status & VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE) {
		VR_DEBUG_PRINT(2, ("Aborting MMU disable stall request since it is in pagefault state.\n"));
		return;
	}

	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_DISABLE_STALL);

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; ++i) {
		u32 status = vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS);
		if ( 0 == (status & VR_MMU_STATUS_BIT_STALL_ACTIVE) ) {
			break;
		}
		if ( status &  VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE ) {
			break;
		}
		if ( 0 == (mmu_status & VR_MMU_STATUS_BIT_PAGING_ENABLED )) {
			break;
		}
	}
	if (VR_REG_POLL_COUNT_FAST == i) VR_DEBUG_PRINT(1,("Disable stall request failed, MMU status is 0x%08X\n", vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS)));
}

void vr_mmu_page_fault_done(struct vr_mmu_core *mmu)
{
	VR_DEBUG_PRINT(4, ("Vr MMU: %s: Leaving page fault mode\n", mmu->hw_core.description));
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_PAGE_FAULT_DONE);
}

VR_STATIC_INLINE _vr_osk_errcode_t vr_mmu_raw_reset(struct vr_mmu_core *mmu)
{
	int i;

	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_DTE_ADDR, 0xCAFEBABE);
	VR_DEBUG_ASSERT(0xCAFEB000 == vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_DTE_ADDR));
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_HARD_RESET);

	for (i = 0; i < VR_REG_POLL_COUNT_FAST; ++i) {
		if (vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_DTE_ADDR) == 0) {
			break;
		}
	}
	if (VR_REG_POLL_COUNT_FAST == i) {
		VR_PRINT_ERROR(("Reset request failed, MMU status is 0x%08X\n", vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS)));
		return _VR_OSK_ERR_FAULT;
	}

	return _VR_OSK_ERR_OK;
}

_vr_osk_errcode_t vr_mmu_reset(struct vr_mmu_core *mmu)
{
	_vr_osk_errcode_t err = _VR_OSK_ERR_FAULT;
	vr_bool stall_success;
	VR_DEBUG_ASSERT_POINTER(mmu);

	stall_success = vr_mmu_enable_stall(mmu);
	if (!stall_success) {
		err = _VR_OSK_ERR_BUSY;
	}

	VR_DEBUG_PRINT(3, ("Vr MMU: vr_kernel_mmu_reset: %s\n", mmu->hw_core.description));

	if (_VR_OSK_ERR_OK == vr_mmu_raw_reset(mmu)) {
		vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_INT_MASK, VR_MMU_INTERRUPT_PAGE_FAULT | VR_MMU_INTERRUPT_READ_BUS_ERROR);
		/* no session is active, so just activate the empty page directory */
		vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_DTE_ADDR, vr_empty_page_directory_phys);
		vr_mmu_enable_paging(mmu);
		err = _VR_OSK_ERR_OK;
	}
	vr_mmu_disable_stall(mmu);

	return err;
}

vr_bool vr_mmu_zap_tlb(struct vr_mmu_core *mmu)
{
	vr_bool stall_success = vr_mmu_enable_stall(mmu);

	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_ZAP_CACHE);

	if (VR_FALSE == stall_success) {
		/* False means that it is in Pagefault state. Not possible to disable_stall then */
		return VR_FALSE;
	}

	vr_mmu_disable_stall(mmu);
	return VR_TRUE;
}

void vr_mmu_zap_tlb_without_stall(struct vr_mmu_core *mmu)
{
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_ZAP_CACHE);
}


void vr_mmu_invalidate_page(struct vr_mmu_core *mmu, u32 vr_address)
{
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_ZAP_ONE_LINE, VR_MMU_PDE_ENTRY(vr_address));
}

static void vr_mmu_activate_address_space(struct vr_mmu_core *mmu, u32 page_directory)
{
	/* The MMU must be in stalled or page fault mode, for this writing to work */
	VR_DEBUG_ASSERT( 0 != ( vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS)
	                          & (VR_MMU_STATUS_BIT_STALL_ACTIVE|VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE) ) );
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_DTE_ADDR, page_directory);
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_COMMAND, VR_MMU_COMMAND_ZAP_CACHE);

}

void vr_mmu_activate_page_directory(struct vr_mmu_core *mmu, struct vr_page_directory *pagedir)
{
	vr_bool stall_success;
	VR_DEBUG_ASSERT_POINTER(mmu);

	VR_DEBUG_PRINT(5, ("Asked to activate page directory 0x%x on MMU %s\n", pagedir, mmu->hw_core.description));

	stall_success = vr_mmu_enable_stall(mmu);
	VR_DEBUG_ASSERT(stall_success);
	VR_IGNORE(stall_success);
	vr_mmu_activate_address_space(mmu, pagedir->page_directory);
	vr_mmu_disable_stall(mmu);
}

void vr_mmu_activate_empty_page_directory(struct vr_mmu_core* mmu)
{
	vr_bool stall_success;

	VR_DEBUG_ASSERT_POINTER(mmu);
	VR_DEBUG_PRINT(3, ("Activating the empty page directory on MMU %s\n", mmu->hw_core.description));

	stall_success = vr_mmu_enable_stall(mmu);

	/* This function can only be called when the core is idle, so it could not fail. */
	VR_DEBUG_ASSERT(stall_success);
	VR_IGNORE(stall_success);

	vr_mmu_activate_address_space(mmu, vr_empty_page_directory_phys);
	vr_mmu_disable_stall(mmu);
}

void vr_mmu_activate_fault_flush_page_directory(struct vr_mmu_core* mmu)
{
	vr_bool stall_success;
	VR_DEBUG_ASSERT_POINTER(mmu);

	VR_DEBUG_PRINT(3, ("Activating the page fault flush page directory on MMU %s\n", mmu->hw_core.description));
	stall_success = vr_mmu_enable_stall(mmu);
	/* This function is expect to fail the stalling, since it might be in PageFault mode when it is called */
	vr_mmu_activate_address_space(mmu, vr_page_fault_flush_page_directory);
	if ( VR_TRUE==stall_success ) vr_mmu_disable_stall(mmu);
}

/* Is called when we want the mmu to give an interrupt */
static void vr_mmu_probe_trigger(void *data)
{
	struct vr_mmu_core *mmu = (struct vr_mmu_core *)data;
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_INT_RAWSTAT, VR_MMU_INTERRUPT_PAGE_FAULT|VR_MMU_INTERRUPT_READ_BUS_ERROR);
}

/* Is called when the irq probe wants the mmu to acknowledge an interrupt from the hw */
static _vr_osk_errcode_t vr_mmu_probe_ack(void *data)
{
	struct vr_mmu_core *mmu = (struct vr_mmu_core *)data;
	u32 int_stat;

	int_stat = vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_INT_STATUS);

	VR_DEBUG_PRINT(2, ("vr_mmu_probe_irq_acknowledge: intstat 0x%x\n", int_stat));
	if (int_stat & VR_MMU_INTERRUPT_PAGE_FAULT) {
		VR_DEBUG_PRINT(2, ("Probe: Page fault detect: PASSED\n"));
		vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_INT_CLEAR, VR_MMU_INTERRUPT_PAGE_FAULT);
	} else {
		VR_DEBUG_PRINT(1, ("Probe: Page fault detect: FAILED\n"));
	}

	if (int_stat & VR_MMU_INTERRUPT_READ_BUS_ERROR) {
		VR_DEBUG_PRINT(2, ("Probe: Bus read error detect: PASSED\n"));
		vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_INT_CLEAR, VR_MMU_INTERRUPT_READ_BUS_ERROR);
	} else {
		VR_DEBUG_PRINT(1, ("Probe: Bus read error detect: FAILED\n"));
	}

	if ( (int_stat & (VR_MMU_INTERRUPT_PAGE_FAULT|VR_MMU_INTERRUPT_READ_BUS_ERROR)) ==
	     (VR_MMU_INTERRUPT_PAGE_FAULT|VR_MMU_INTERRUPT_READ_BUS_ERROR)) {
		return _VR_OSK_ERR_OK;
	}

	return _VR_OSK_ERR_FAULT;
}

#if 0
void vr_mmu_print_state(struct vr_mmu_core *mmu)
{
	VR_DEBUG_PRINT(2, ("MMU: State of %s is 0x%08x\n", mmu->hw_core.description, vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS)));
}
#endif
