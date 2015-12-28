/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_DMA_H__
#define __VR_DMA_H__

#include "vr_osk.h"
#include "vr_osk_vr.h"
#include "vr_hw_core.h"

#define VR_DMA_CMD_BUF_SIZE 1024

typedef struct vr_dma_cmd_buf {
	u32 *virt_addr;           /**< CPU address of command buffer */
	u32 phys_addr;            /**< Physical address of command buffer */
	u32 size;                 /**< Number of prepared words in command buffer */
} vr_dma_cmd_buf;

/** @brief Create a new DMA unit
 *
 * This is called from entry point of the driver in order to create and
 * intialize the DMA resource
 *
 * @param resource it will be a pointer to a DMA resource
 * @return DMA object on success, NULL on failure
 */
struct vr_dma_core *vr_dma_create(_vr_osk_resource_t *resource);

/** @brief Delete DMA unit
 *
 * This is called on entry point of driver if the driver initialization fails
 * after initialization of the DMA unit. It is also called on the exit of the
 * driver to delete the DMA resource
 *
 * @param dma Pointer to DMA unit object
 */
void vr_dma_delete(struct vr_dma_core *dma);

/** @brief Retrieves the VR DMA core object (if there is)
 *
 * @return The Vr DMA object otherwise NULL
 */
struct vr_dma_core *vr_dma_get_global_dma_core(void);

/**
 * @brief Run a command buffer on the DMA unit
 *
 * @param dma Pointer to the DMA unit to use
 * @param buf Pointer to the command buffer to use
 * @return _VR_OSK_ERR_OK if the buffer was started successfully,
 *         _VR_OSK_ERR_BUSY if the DMA unit is busy.
 */
_vr_osk_errcode_t vr_dma_start(struct vr_dma_core* dma, vr_dma_cmd_buf *buf);

/**
 * @brief Create a DMA command
 *
 * @param core Vr core
 * @param reg offset to register of core
 * @param n number of registers to write
 */
VR_STATIC_INLINE u32 vr_dma_command_write(struct vr_hw_core *core, u32 reg, u32 n)
{
	u32 core_offset = core->phys_offset;

	VR_DEBUG_ASSERT(reg < 0x2000);
	VR_DEBUG_ASSERT(n < 0x800);
	VR_DEBUG_ASSERT(core_offset < 0x30000);
	VR_DEBUG_ASSERT(0 == ((core_offset + reg) & ~0x7FFFF));

	return (n << 20) | (core_offset + reg);
}

/**
 * @brief Add a array write to DMA command buffer
 *
 * @param buf DMA command buffer to fill in
 * @param core Core to do DMA to
 * @param reg Register on core to start writing to
 * @param data Pointer to data to write
 * @param count Number of 4 byte words to write
 */
VR_STATIC_INLINE void vr_dma_write_array(vr_dma_cmd_buf *buf, struct vr_hw_core *core,
        u32 reg, u32 *data, u32 count)
{
	VR_DEBUG_ASSERT((buf->size + 1 + count ) < VR_DMA_CMD_BUF_SIZE / 4);

	buf->virt_addr[buf->size++] = vr_dma_command_write(core, reg, count);

	_vr_osk_memcpy(buf->virt_addr + buf->size, data, count * sizeof(*buf->virt_addr));

	buf->size += count;
}

/**
 * @brief Add a conditional array write to DMA command buffer
 *
 * @param buf DMA command buffer to fill in
 * @param core Core to do DMA to
 * @param reg Register on core to start writing to
 * @param data Pointer to data to write
 * @param count Number of 4 byte words to write
 * @param ref Pointer to referance data that can be skipped if equal
 */
VR_STATIC_INLINE void vr_dma_write_array_conditional(vr_dma_cmd_buf *buf, struct vr_hw_core *core,
        u32 reg, u32 *data, u32 count, const u32 *ref)
{
	/* Do conditional array writes are not yet implemented, fallback to a
	 * normal array write. */
	vr_dma_write_array(buf, core, reg, data, count);
}

/**
 * @brief Add a conditional register write to the DMA command buffer
 *
 * If the data matches the reference the command will be skipped.
 *
 * @param buf DMA command buffer to fill in
 * @param core Core to do DMA to
 * @param reg Register on core to start writing to
 * @param data Pointer to data to write
 * @param ref Pointer to referance data that can be skipped if equal
 */
VR_STATIC_INLINE void vr_dma_write_conditional(vr_dma_cmd_buf *buf, struct vr_hw_core *core,
        u32 reg, u32 data, const u32 ref)
{
	/* Skip write if reference value is equal to data. */
	if (data == ref) return;

	buf->virt_addr[buf->size++] = vr_dma_command_write(core, reg, 1);

	buf->virt_addr[buf->size++] = data;

	VR_DEBUG_ASSERT(buf->size < VR_DMA_CMD_BUF_SIZE / 4);
}

/**
 * @brief Add a register write to the DMA command buffer
 *
 * @param buf DMA command buffer to fill in
 * @param core Core to do DMA to
 * @param reg Register on core to start writing to
 * @param data Pointer to data to write
 */
VR_STATIC_INLINE void vr_dma_write(vr_dma_cmd_buf *buf, struct vr_hw_core *core,
                                       u32 reg, u32 data)
{
	buf->virt_addr[buf->size++] = vr_dma_command_write(core, reg, 1);

	buf->virt_addr[buf->size++] = data;

	VR_DEBUG_ASSERT(buf->size < VR_DMA_CMD_BUF_SIZE / 4);
}

/**
 * @brief Prepare DMA command buffer for use
 *
 * This function allocates the DMA buffer itself.
 *
 * @param buf The vr_dma_cmd_buf to prepare
 * @return _VR_OSK_ERR_OK if the \a buf is ready to use
 */
_vr_osk_errcode_t vr_dma_get_cmd_buf(vr_dma_cmd_buf *buf);

/**
 * @brief Check if a DMA command buffer is ready for use
 *
 * @param buf The vr_dma_cmd_buf to check
 * @return VR_TRUE if buffer is usable, VR_FALSE otherwise
 */
VR_STATIC_INLINE vr_bool vr_dma_cmd_buf_is_valid(vr_dma_cmd_buf *buf)
{
	return NULL != buf->virt_addr;
}

/**
 * @brief Return a DMA command buffer
 *
 * @param buf Pointer to DMA command buffer to return
 */
void vr_dma_put_cmd_buf(vr_dma_cmd_buf *buf);

#endif /* __VR_DMA_H__ */
