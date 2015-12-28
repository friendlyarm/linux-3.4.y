/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_hw_core.h"
#include "vr_dma.h"

/**
 * Size of the Vr-450 DMA unit registers in bytes.
 */
#define VR450_DMA_REG_SIZE 0x08

/**
 * Value that appears in MEMSIZE if an error occurs when reading the command list.
 */
#define VR450_DMA_BUS_ERR_VAL 0xffffffff

/**
 * Vr DMA registers
 * Used in the register read/write routines.
 * See the hardware documentation for more information about each register.
 */
typedef enum vr_dma_register {

	VR450_DMA_REG_SOURCE_ADDRESS = 0x0000,
	VR450_DMA_REG_SOURCE_SIZE = 0x0004,
} vr_dma_register;

struct vr_dma_core {
	struct vr_hw_core  hw_core;      /**< Common for all HW cores */
	_vr_osk_spinlock_t *lock;            /**< Lock protecting access to DMA core */
	vr_dma_pool pool;                /**< Memory pool for command buffers */
};

static struct vr_dma_core *vr_global_dma_core = NULL;

struct vr_dma_core *vr_dma_create(_vr_osk_resource_t *resource)
{
	struct vr_dma_core* dma;
	_vr_osk_errcode_t err;

	VR_DEBUG_ASSERT(NULL == vr_global_dma_core);

	dma = _vr_osk_malloc(sizeof(struct vr_dma_core));
	if (dma == NULL) goto alloc_failed;

	dma->lock = _vr_osk_spinlock_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_DMA_COMMAND);
	if (NULL == dma->lock) goto lock_init_failed;

	dma->pool = vr_dma_pool_create(VR_DMA_CMD_BUF_SIZE, 4, 0);
	if (NULL == dma->pool) goto dma_pool_failed;

	err = vr_hw_core_create(&dma->hw_core, resource, VR450_DMA_REG_SIZE);
	if (_VR_OSK_ERR_OK != err) goto hw_core_failed;

	vr_global_dma_core = dma;
	VR_DEBUG_PRINT(2, ("Vr DMA: Created Vr APB DMA unit\n"));
	return dma;

	/* Error handling */

hw_core_failed:
	vr_dma_pool_destroy(dma->pool);
dma_pool_failed:
	_vr_osk_spinlock_term(dma->lock);
lock_init_failed:
	_vr_osk_free(dma);
alloc_failed:
	VR_DEBUG_PRINT(2, ("Vr DMA: Failed to create APB DMA unit\n"));
	return NULL;
}

void vr_dma_delete(struct vr_dma_core *dma)
{
	VR_DEBUG_ASSERT_POINTER(dma);

	VR_DEBUG_PRINT(2, ("Vr DMA: Deleted Vr APB DMA unit\n"));

	vr_hw_core_delete(&dma->hw_core);
	_vr_osk_spinlock_term(dma->lock);
	vr_dma_pool_destroy(dma->pool);
	_vr_osk_free(dma);
}

static void vr_dma_bus_error(struct vr_dma_core *dma)
{
	u32 addr = vr_hw_core_register_read(&dma->hw_core, VR450_DMA_REG_SOURCE_ADDRESS);

	VR_PRINT_ERROR(("Vr DMA: Bus error when reading command list from 0x%lx\n", addr));

	/* Clear the bus error */
	vr_hw_core_register_write(&dma->hw_core, VR450_DMA_REG_SOURCE_SIZE, 0);
}

static vr_bool vr_dma_is_busy(struct vr_dma_core *dma)
{
	u32 val;
	vr_bool dma_busy_flag = VR_FALSE;

	VR_DEBUG_ASSERT_POINTER(dma);

	val = vr_hw_core_register_read(&dma->hw_core, VR450_DMA_REG_SOURCE_SIZE);

	if (VR450_DMA_BUS_ERR_VAL == val) {
		/* Bus error reading command list */
		vr_dma_bus_error(dma);
		return VR_FALSE;
	}
	if (val > 0) {
		dma_busy_flag = VR_TRUE;
	}

	return dma_busy_flag;
}

static void vr_dma_start_transfer(struct vr_dma_core* dma, vr_dma_cmd_buf *buf)
{
	u32 memsize = buf->size * 4;
	u32 addr = buf->phys_addr;

	VR_DEBUG_ASSERT_POINTER(dma);
	VR_DEBUG_ASSERT(memsize < (1 << 16));
	VR_DEBUG_ASSERT(0 == (memsize & 0x3)); /* 4 byte aligned */

	VR_DEBUG_ASSERT(!vr_dma_is_busy(dma));

	/* Writes the physical source memory address of chunk containing command headers and data */
	vr_hw_core_register_write(&dma->hw_core, VR450_DMA_REG_SOURCE_ADDRESS, addr);

	/* Writes the length of transfer */
	vr_hw_core_register_write(&dma->hw_core, VR450_DMA_REG_SOURCE_SIZE, memsize);
}

_vr_osk_errcode_t vr_dma_get_cmd_buf(vr_dma_cmd_buf *buf)
{
	VR_DEBUG_ASSERT_POINTER(buf);

	buf->virt_addr = (u32*)vr_dma_pool_alloc(vr_global_dma_core->pool, &buf->phys_addr);
	if (NULL == buf->virt_addr) {
		return _VR_OSK_ERR_NOMEM;
	}

	/* size contains the number of words in the buffer and is incremented
	 * as commands are added to the buffer. */
	buf->size = 0;

	return _VR_OSK_ERR_OK;
}

void vr_dma_put_cmd_buf(vr_dma_cmd_buf *buf)
{
	VR_DEBUG_ASSERT_POINTER(buf);

	if (NULL == buf->virt_addr) return;

	vr_dma_pool_free(vr_global_dma_core->pool, buf->virt_addr, buf->phys_addr);

	buf->virt_addr = NULL;
}

_vr_osk_errcode_t vr_dma_start(struct vr_dma_core* dma, vr_dma_cmd_buf *buf)
{
	_vr_osk_errcode_t err = _VR_OSK_ERR_OK;

	_vr_osk_spinlock_lock(dma->lock);

	if (vr_dma_is_busy(dma)) {
		err = _VR_OSK_ERR_BUSY;
		goto out;
	}

	vr_dma_start_transfer(dma, buf);

out:
	_vr_osk_spinlock_unlock(dma->lock);
	return err;
}

void vr_dma_debug(struct vr_dma_core *dma)
{
	VR_DEBUG_ASSERT_POINTER(dma);
	VR_DEBUG_PRINT(1, ("DMA unit registers:\n\t%08x, %08x\n",
	                     vr_hw_core_register_read(&dma->hw_core, VR450_DMA_REG_SOURCE_ADDRESS),
	                     vr_hw_core_register_read(&dma->hw_core, VR450_DMA_REG_SOURCE_SIZE)
	                    ));

}

struct vr_dma_core *vr_dma_get_global_dma_core(void)
{
	/* Returns the global dma core object */
	return vr_global_dma_core;
}
