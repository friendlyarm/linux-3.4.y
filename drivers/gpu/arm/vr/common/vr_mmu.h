/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2007-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_MMU_H__
#define __VR_MMU_H__

#include "vr_osk.h"
#include "vr_mmu_page_directory.h"
#include "vr_hw_core.h"

/* Forward declaration from vr_group.h */
struct vr_group;

/**
 * MMU register numbers
 * Used in the register read/write routines.
 * See the hardware documentation for more information about each register
 */
typedef enum vr_mmu_register {
	VR_MMU_REGISTER_DTE_ADDR = 0x0000, /**< Current Page Directory Pointer */
	VR_MMU_REGISTER_STATUS = 0x0004, /**< Status of the MMU */
	VR_MMU_REGISTER_COMMAND = 0x0008, /**< Command register, used to control the MMU */
	VR_MMU_REGISTER_PAGE_FAULT_ADDR = 0x000C, /**< Logical address of the last page fault */
	VR_MMU_REGISTER_ZAP_ONE_LINE = 0x010, /**< Used to invalidate the mapping of a single page from the MMU */
	VR_MMU_REGISTER_INT_RAWSTAT = 0x0014, /**< Raw interrupt status, all interrupts visible */
	VR_MMU_REGISTER_INT_CLEAR = 0x0018, /**< Indicate to the MMU that the interrupt has been received */
	VR_MMU_REGISTER_INT_MASK = 0x001C, /**< Enable/disable types of interrupts */
	VR_MMU_REGISTER_INT_STATUS = 0x0020 /**< Interrupt status based on the mask */
} vr_mmu_register;

/**
 * MMU interrupt register bits
 * Each cause of the interrupt is reported
 * through the (raw) interrupt status registers.
 * Multiple interrupts can be pending, so multiple bits
 * can be set at once.
 */
typedef enum vr_mmu_interrupt {
	VR_MMU_INTERRUPT_PAGE_FAULT = 0x01, /**< A page fault occured */
	VR_MMU_INTERRUPT_READ_BUS_ERROR = 0x02 /**< A bus read error occured */
} vr_mmu_interrupt;

typedef enum vr_mmu_status_bits {
	VR_MMU_STATUS_BIT_PAGING_ENABLED      = 1 << 0,
	VR_MMU_STATUS_BIT_PAGE_FAULT_ACTIVE   = 1 << 1,
	VR_MMU_STATUS_BIT_STALL_ACTIVE        = 1 << 2,
	VR_MMU_STATUS_BIT_IDLE                = 1 << 3,
	VR_MMU_STATUS_BIT_REPLAY_BUFFER_EMPTY = 1 << 4,
	VR_MMU_STATUS_BIT_PAGE_FAULT_IS_WRITE = 1 << 5,
	VR_MMU_STATUS_BIT_STALL_NOT_ACTIVE    = 1 << 31,
} vr_mmu_status_bits;

/**
 * Definition of the MMU struct
 * Used to track a MMU unit in the system.
 * Contains information about the mapping of the registers
 */
struct vr_mmu_core {
	struct vr_hw_core hw_core; /**< Common for all HW cores */
	_vr_osk_irq_t *irq;        /**< IRQ handler */
};

_vr_osk_errcode_t vr_mmu_initialize(void);

void vr_mmu_terminate(void);

struct vr_mmu_core *vr_mmu_create(_vr_osk_resource_t *resource, struct vr_group *group, vr_bool is_virtual);
void vr_mmu_delete(struct vr_mmu_core *mmu);

_vr_osk_errcode_t vr_mmu_reset(struct vr_mmu_core *mmu);
vr_bool vr_mmu_zap_tlb(struct vr_mmu_core *mmu);
void vr_mmu_zap_tlb_without_stall(struct vr_mmu_core *mmu);
void vr_mmu_invalidate_page(struct vr_mmu_core *mmu, u32 vr_address);

void vr_mmu_activate_page_directory(struct vr_mmu_core* mmu, struct vr_page_directory *pagedir);
void vr_mmu_activate_empty_page_directory(struct vr_mmu_core* mmu);
void vr_mmu_activate_fault_flush_page_directory(struct vr_mmu_core* mmu);

void vr_mmu_page_fault_done(struct vr_mmu_core *mmu);

/*** Register reading/writing functions ***/
VR_STATIC_INLINE u32 vr_mmu_get_int_status(struct vr_mmu_core *mmu)
{
	return vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_INT_STATUS);
}

VR_STATIC_INLINE u32 vr_mmu_get_rawstat(struct vr_mmu_core *mmu)
{
	return vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_INT_RAWSTAT);
}

VR_STATIC_INLINE void vr_mmu_mask_all_interrupts(struct vr_mmu_core *mmu)
{
	vr_hw_core_register_write(&mmu->hw_core, VR_MMU_REGISTER_INT_MASK, 0);
}

VR_STATIC_INLINE u32 vr_mmu_get_status(struct vr_mmu_core *mmu)
{
	return vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_STATUS);
}

VR_STATIC_INLINE u32 vr_mmu_get_page_fault_addr(struct vr_mmu_core *mmu)
{
	return vr_hw_core_register_read(&mmu->hw_core, VR_MMU_REGISTER_PAGE_FAULT_ADDR);
}

#endif /* __VR_MMU_H__ */
