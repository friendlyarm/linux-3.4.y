/*
 * Samsung Exynos5 SoC series FIMC-IS driver
 *
 * exynos5 fimc-is video functions
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/videonode.h>
#include <media/exynos_mc.h>
#include <linux/cma.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#include <linux/firmware.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/videodev2_exynos_media.h>
#include <linux/v4l2-mediabus.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/pm_qos.h>
#include <linux/debugfs.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/exynos5_bus.h>
#include <plat/bts.h>

#include "fimc-is-time.h"
#include "fimc-is-core.h"
#include "fimc-is-param.h"
#include "fimc-is-cmd.h"
#include "fimc-is-regs.h"
#include "fimc-is-err.h"
#include "fimc-is-video.h"

#include "fimc-is-device-ischain.h"

#define SDCARD_FW
#define FIMC_IS_SETFILE_SDCARD_PATH		"/data/"
#define FIMC_IS_FW				"fimc_is_fw2.bin"
#define FIMC_IS_FW_SDCARD			"/data/fimc_is_fw2.bin"
#define FIMC_IS_FW_BASE_MASK			((1 << 26) - 1)
#define FIMC_IS_VERSION_SIZE			39

#define FIMC_IS_MAX_CAL_SIZE			(1196)
#define FIMC_IS_CAL_START_ADDR			(0x8D0000)

/* Default setting values */
#define DEFAULT_PREVIEW_STILL_WIDTH		(1280) /* sensor margin : 16 */
#define DEFAULT_PREVIEW_STILL_HEIGHT		(720) /* sensor margin : 12 */
#define DEFAULT_CAPTURE_VIDEO_WIDTH		(1920)
#define DEFAULT_CAPTURE_VIDEO_HEIGHT		(1080)
#define DEFAULT_CAPTURE_STILL_WIDTH		(2560)
#define DEFAULT_CAPTURE_STILL_HEIGHT		(1920)
#define DEFAULT_CAPTURE_STILL_CROP_WIDTH	(2560)
#define DEFAULT_CAPTURE_STILL_CROP_HEIGHT	(1440)
#define DEFAULT_PREVIEW_VIDEO_WIDTH		(640)
#define DEFAULT_PREVIEW_VIDEO_HEIGHT		(480)

static struct pm_qos_request pm_qos_req_cpu;
static struct exynos5_bus_int_handle *isp_int_handle_min;
static struct exynos5_bus_mif_handle *isp_mif_handle_min;

#ifdef FW_DEBUG
#define DEBUG_FS_ROOT_NAME	"fimc-is"
#define DEBUG_FS_FILE_NAME	"isfw-msg"
static struct dentry		*debugfs_root;
static struct dentry		*debugfs_file;

static int isfw_debug_open(struct inode *inode, struct file *file)
{
	if (inode->i_private)
		file->private_data = inode->i_private;

	return 0;
}

static int isfw_debug_read(struct file *file, char __user *user_buf,
	size_t buf_len, loff_t *ppos)
{
	int debug_cnt;
	char *debug;
	int count1, count2;
	struct fimc_is_device_ischain *ischain;

	count1 = 0;
	count2 = 0;
	debug_cnt = 0;
	ischain = (struct fimc_is_device_ischain *)file->private_data;

	if (!ischain) {
		err("file->private_data is null");
		return 0;
	}

	if (!test_bit(FIMC_IS_ISCHAIN_OPEN, &ischain->state)) {
		err("isp video node is not open");
		goto exit;
	}

	if (!test_bit(FIMC_IS_ISCHAIN_POWER_ON, &ischain->state)) {
		err("firmware is not loaded");
		goto exit;
	}

	vb2_ion_sync_for_device(ischain->minfo.fw_cookie, DEBUG_OFFSET,
		DEBUG_CNT, DMA_FROM_DEVICE);

	debug_cnt = *((int *)(ischain->minfo.kvaddr + DEBUGCTL_OFFSET))
			- DEBUG_OFFSET;

	if (ischain->debug_cnt > debug_cnt) {
		count1 = DEBUG_CNT - ischain->debug_cnt;
		count2 = debug_cnt;
	} else {
		count1 = debug_cnt - ischain->debug_cnt;
		count2 = 0;
	}

	if (count1) {
		debug = (char *)(ischain->minfo.kvaddr + DEBUG_OFFSET +
			ischain->debug_cnt);

		if (count1 > buf_len)
			count1 = buf_len;

		memcpy(user_buf, debug, count1);
		ischain->debug_cnt += count1;
	}

	if (count1 == buf_len) {
		count2 = 0;
		goto exit;
	}

	if (count2) {
		ischain->debug_cnt = 0;
		debug = (char *)(ischain->minfo.kvaddr + DEBUG_OFFSET);

		if ((count1 + count2) > buf_len)
			count2 = buf_len -  count1;

		memcpy(user_buf, debug, count2);
		ischain->debug_cnt += count2;
	}

exit:
	/* this printk need to be remained for debugging */
	/*printk(KERN_INFO "buffer point(%d:%d), copy size : %d\n",
		debug_cnt, ischain->debug_cnt, (count1 + count2));*/
	return count1 + count2;
}

static const struct file_operations debug_fops = {
	.open	= isfw_debug_open,
	.read	= isfw_debug_read,
	.llseek	= default_llseek
};

#endif

static const struct sensor_param init_sensor_param = {
	.frame_rate = {
		.frame_rate = 30,
	},
};

static const struct isp_param init_isp_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_DISABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = OTF_INPUT_FORMAT_BAYER,
		.bitwidth = OTF_INPUT_BIT_WIDTH_10BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.crop_offset_x = 0,
		.crop_offset_y = 0,
		.crop_width = 0,
		.crop_height = 0,
		.frametime_min = 0,
		.frametime_max = 33333,
		.err = OTF_INPUT_ERROR_NO,
	},
	.dma1_input = {
		.cmd = DMA_INPUT_COMMAND_DISABLE,
		.width = 0,
		.height = 0,
		.format = 0,
		.bitwidth = 0,
		.plane = 0,
		.order = 0,
		.buffer_number = 0,
		.buffer_address = 0,
		.err = 0,
	},
	.dma2_input = {
		.cmd = DMA_INPUT_COMMAND_DISABLE,
		.width = 0, .height = 0,
		.format = 0, .bitwidth = 0, .plane = 0,
		.order = 0, .buffer_number = 0, .buffer_address = 0,
		.err = 0,
	},
	.aa = {
		.cmd = ISP_AA_COMMAND_START,
		.target = ISP_AA_TARGET_AF | ISP_AA_TARGET_AE |
						ISP_AA_TARGET_AWB,
		.mode = 0,
		.scene = 0,
		.sleep = 0,
		.uiAfFace = 0,
		.touch_x = 0, .touch_y = 0,
		.manual_af_setting = 0,
		.err = ISP_AF_ERROR_NO,
	},
	.flash = {
		.cmd = ISP_FLASH_COMMAND_DISABLE,
		.redeye = ISP_FLASH_REDEYE_DISABLE,
		.err = ISP_FLASH_ERROR_NO,
	},
	.awb = {
		.cmd = ISP_AWB_COMMAND_AUTO,
		.illumination = 0,
		.err = ISP_AWB_ERROR_NO,
	},
	.effect = {
		.cmd = ISP_IMAGE_EFFECT_DISABLE,
		.err = ISP_IMAGE_EFFECT_ERROR_NO,
	},
	.iso = {
		.cmd = ISP_ISO_COMMAND_AUTO,
		.value = 0,
		.err = ISP_ISO_ERROR_NO,
	},
	.adjust = {
		.cmd = ISP_ADJUST_COMMAND_AUTO,
		.contrast = 0,
		.saturation = 0,
		.sharpness = 0,
		.exposure = 0,
		.brightness = 0,
		.hue = 0,
		.err = ISP_ADJUST_ERROR_NO,
	},
	.metering = {
		.cmd = ISP_METERING_COMMAND_CENTER,
		.win_pos_x = 0, .win_pos_y = 0,
		.win_width = DEFAULT_CAPTURE_STILL_WIDTH,
		.win_height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.err = ISP_METERING_ERROR_NO,
	},
	.afc = {
		.cmd = ISP_AFC_COMMAND_AUTO,
		.manual = 0, .err = ISP_AFC_ERROR_NO,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV444,
		.bitwidth = OTF_OUTPUT_BIT_WIDTH_12BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
	.dma1_output = {
		.cmd = DMA_OUTPUT_COMMAND_DISABLE,
		.dma_out_mask = 0,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = DMA_INPUT_FORMAT_YUV444,
		.bitwidth = DMA_INPUT_BIT_WIDTH_8BIT,
		.plane = DMA_INPUT_PLANE_1,
		.order = DMA_INPUT_ORDER_YCbCr,
		.buffer_number = 0,
		.buffer_address = 0,
		.err = DMA_OUTPUT_ERROR_NO,
	},
	.dma2_output = {
		.cmd = DMA_OUTPUT_COMMAND_DISABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = DMA_OUTPUT_FORMAT_BAYER,
		.bitwidth = DMA_OUTPUT_BIT_WIDTH_12BIT,
		.plane = DMA_OUTPUT_PLANE_1,
		.order = DMA_OUTPUT_ORDER_GB_BG,
		.buffer_number = 0,
		.buffer_address = 0,
		.dma_out_mask = 0xFFFFFFFF,
		.err = DMA_OUTPUT_ERROR_NO,
	},
};

static const struct drc_param init_drc_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_ENABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_12BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.err = OTF_INPUT_ERROR_NO,
	},
	.dma_input = {
		.cmd = DMA_INPUT_COMMAND_DISABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = DMA_INPUT_FORMAT_YUV444,
		.bitwidth = DMA_INPUT_BIT_WIDTH_8BIT,
		.plane = DMA_INPUT_PLANE_1,
		.order = DMA_INPUT_ORDER_YCbCr,
		.buffer_number = 0,
		.buffer_address = 0,
		.err = 0,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
};

static const struct scalerc_param init_scalerc_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_ENABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_12BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.crop_offset_x = 0,
		.crop_offset_y = 0,
		.crop_width = 0,
		.crop_height = 0,
		.err = OTF_INPUT_ERROR_NO,
	},
	.effect = {
		.cmd = 0,
		.err = 0,
	},
	.input_crop = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.pos_x = 0,
		.pos_y = 0,
		.crop_width = DEFAULT_CAPTURE_STILL_CROP_WIDTH,
		.crop_height = DEFAULT_CAPTURE_STILL_CROP_HEIGHT,
		.in_width = DEFAULT_CAPTURE_STILL_WIDTH,
		.in_height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.out_width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.out_height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.err = 0,
	},
	.output_crop = {
		.cmd = SCALER_CROP_COMMAND_DISABLE,
		.pos_x = 0,
		.pos_y = 0,
		.crop_width = DEFAULT_CAPTURE_STILL_WIDTH,
		.crop_height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = DMA_OUTPUT_FORMAT_YUV422,
		.err = 0,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV444,
		.bitwidth = OTF_OUTPUT_BIT_WIDTH_8BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
	.dma_output = {
		.cmd = DMA_OUTPUT_COMMAND_DISABLE,
		.width = DEFAULT_CAPTURE_STILL_WIDTH,
		.height = DEFAULT_CAPTURE_STILL_HEIGHT,
		.format = DMA_OUTPUT_FORMAT_YUV422,
		.bitwidth = DMA_OUTPUT_BIT_WIDTH_8BIT,
		.plane = DMA_OUTPUT_PLANE_1,
		.order = DMA_OUTPUT_ORDER_CrYCbY,
		.buffer_number = 0,
		.buffer_address = 0,
		.dma_out_mask = 0xffff,
		.reserved[0] = 2, /* unscaled*/
		.err = DMA_OUTPUT_ERROR_NO,
	},
};

static const struct odc_param init_odc_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_ENABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.crop_offset_x = 0,
		.crop_offset_y = 0,
		.crop_width = 0,
		.crop_height = 0,
		.err = OTF_INPUT_ERROR_NO,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV422,
		.bitwidth = OTF_OUTPUT_BIT_WIDTH_8BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
};

static const struct dis_param init_dis_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_ENABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV422,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.crop_offset_x = 0,
		.crop_offset_y = 0,
		.crop_width = 0,
		.crop_height = 0,
		.err = OTF_INPUT_ERROR_NO,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV422,
		.bitwidth = OTF_OUTPUT_BIT_WIDTH_8BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
};
static const struct tdnr_param init_tdnr_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_ENABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV422,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.err = OTF_INPUT_ERROR_NO,
	},
	.frame = {
		.cmd = 0,
		.err = 0,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
	.dma_output = {
		.cmd = DMA_OUTPUT_COMMAND_DISABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = DMA_OUTPUT_FORMAT_YUV420,
		.bitwidth = DMA_OUTPUT_BIT_WIDTH_8BIT,
		.plane = DMA_OUTPUT_PLANE_2,
		.order = DMA_OUTPUT_ORDER_CbCr,
		.buffer_number = 0,
		.buffer_address = 0,
		.dma_out_mask = 0xffff,
		.err = DMA_OUTPUT_ERROR_NO,
	},
};

static const struct scalerp_param init_scalerp_param = {
	.control = {
		.cmd = CONTROL_COMMAND_START,
		.bypass = CONTROL_BYPASS_ENABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.crop_offset_x = 0,
		.crop_offset_y = 0,
		.crop_width = 0,
		.crop_height = 0,
		.err = OTF_INPUT_ERROR_NO,
	},
	.effect = {
		.cmd = 0,
		.err = 0,
	},
	.input_crop = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.pos_x = 0,
		.pos_y = 0,
		.crop_width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.crop_height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.in_width = DEFAULT_CAPTURE_VIDEO_WIDTH,
		.in_height = DEFAULT_CAPTURE_VIDEO_HEIGHT,
		.out_width = DEFAULT_PREVIEW_STILL_WIDTH,
		.out_height = DEFAULT_PREVIEW_STILL_HEIGHT,
		.err = 0,
	},
	.output_crop = {
		.cmd = SCALER_CROP_COMMAND_DISABLE,
		.pos_x = 0,
		.pos_y = 0,
		.crop_width = DEFAULT_PREVIEW_STILL_WIDTH,
		.crop_height = DEFAULT_PREVIEW_STILL_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV420,
		.err = 0,
	},
	.rotation = {
		.cmd = 0,
		.err = 0,
	},
	.flip = {
		.cmd = 0,
		.err = 0,
	},
	.otf_output = {
		.cmd = OTF_OUTPUT_COMMAND_ENABLE,
		.width = DEFAULT_PREVIEW_STILL_WIDTH,
		.height = DEFAULT_PREVIEW_STILL_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_OUTPUT_ORDER_BAYER_GR_BG,
		.uiCropOffsetX = 0,
		.uiCropOffsetX = 0,
		.err = OTF_OUTPUT_ERROR_NO,
	},
	.dma_output = {
		.cmd = DMA_OUTPUT_COMMAND_DISABLE,
		.width = DEFAULT_PREVIEW_STILL_WIDTH,
		.height = DEFAULT_PREVIEW_STILL_HEIGHT,
		.format = OTF_OUTPUT_FORMAT_YUV420,
		.bitwidth = DMA_OUTPUT_BIT_WIDTH_8BIT,
		.plane = DMA_OUTPUT_PLANE_3,
		.order = DMA_OUTPUT_ORDER_NO,
		.buffer_number = 0,
		.buffer_address = 0,
		.dma_out_mask = 0xffff,
		.err = DMA_OUTPUT_ERROR_NO,
	},
};

static const struct fd_param init_fd_param = {
	.control = {
		.cmd = CONTROL_COMMAND_STOP,
		.bypass = CONTROL_BYPASS_DISABLE,
		.err = CONTROL_ERROR_NO,
	},
	.otf_input = {
		.cmd = OTF_INPUT_COMMAND_ENABLE,
		.width = DEFAULT_PREVIEW_STILL_WIDTH,
		.height = DEFAULT_PREVIEW_STILL_HEIGHT,
		.format = OTF_INPUT_FORMAT_YUV444,
		.bitwidth = OTF_INPUT_BIT_WIDTH_8BIT,
		.order = OTF_INPUT_ORDER_BAYER_GR_BG,
		.err = OTF_INPUT_ERROR_NO,
	},
	.dma_input = {
		.cmd = DMA_INPUT_COMMAND_DISABLE,
		.width = 0, .height = 0,
		.format = 0, .bitwidth = 0, .plane = 0,
		.order = 0, .buffer_number = 0, .buffer_address = 0,
		.err = 0,
	},
	.config = {
		.cmd = FD_CONFIG_COMMAND_MAXIMUM_NUMBER |
			FD_CONFIG_COMMAND_ROLL_ANGLE |
			FD_CONFIG_COMMAND_YAW_ANGLE |
			FD_CONFIG_COMMAND_SMILE_MODE |
			FD_CONFIG_COMMAND_BLINK_MODE |
			FD_CONFIG_COMMAND_EYES_DETECT |
			FD_CONFIG_COMMAND_MOUTH_DETECT |
			FD_CONFIG_COMMAND_ORIENTATION |
			FD_CONFIG_COMMAND_ORIENTATION_VALUE,
		.max_number = CAMERA2_MAX_FACES,
		.roll_angle = FD_CONFIG_ROLL_ANGLE_FULL,
		.yaw_angle = FD_CONFIG_YAW_ANGLE_45_90,
		.smile_mode = FD_CONFIG_SMILE_MODE_DISABLE,
		.blink_mode = FD_CONFIG_BLINK_MODE_DISABLE,
		.eye_detect = FD_CONFIG_EYES_DETECT_ENABLE,
		.mouth_detect = FD_CONFIG_MOUTH_DETECT_DISABLE,
		.orientation = FD_CONFIG_ORIENTATION_DISABLE,
		.orientation_value = 0,
		.err = ERROR_FD_NO,
	},
};

static int testnset_state(struct fimc_is_device_ischain *this,
	unsigned long state)
{
	int ret = 0;

	mutex_lock(&this->mutex_state);

	if (test_bit(state, &this->state)) {
		ret = -EINVAL;
		goto exit;
	}
	set_bit(state, &this->state);

exit:
	mutex_unlock(&this->mutex_state);
	return ret;
}

static int testnclr_state(struct fimc_is_device_ischain *this,
	unsigned long state)
{
	int ret = 0;

	mutex_lock(&this->mutex_state);

	if (!test_bit(state, &this->state)) {
		ret = -EINVAL;
		goto exit;
	}
	clear_bit(state, &this->state);

exit:
	mutex_unlock(&this->mutex_state);
	return ret;
}

static int fimc_is_ischain_allocmem(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	void *fw_cookie;

	dbg_ischain("Allocating memory for FIMC-IS firmware.\n");

	fw_cookie = vb2_ion_private_alloc(this->mem->alloc_ctx,
				FIMC_IS_A5_MEM_SIZE + FIMC_IS_A5_SEN_SIZE +
#ifdef ENABLE_ODC
				SIZE_ODC_INTERNAL_BUF * NUM_ODC_INTERNAL_BUF +
#endif
#ifdef ENABLE_VDIS
				SIZE_DIS_INTERNAL_BUF * NUM_DIS_INTERNAL_BUF +
#endif
#ifdef ENABLE_TDNR
				SIZE_DNR_INTERNAL_BUF * NUM_DNR_INTERNAL_BUF +
#endif
				0);

	if (IS_ERR(fw_cookie)) {
		err("Allocating bitprocessor buffer failed");
		fw_cookie = NULL;
		ret = -ENOMEM;
		goto exit;
	}

	ret = vb2_ion_dma_address(fw_cookie, &this->minfo.dvaddr);
	if ((ret < 0) || (this->minfo.dvaddr  & FIMC_IS_FW_BASE_MASK)) {
		err("The base memory is not aligned to 64MB.");
		vb2_ion_private_free(fw_cookie);
		this->minfo.dvaddr = 0;
		fw_cookie = NULL;
		ret = -EIO;
		goto exit;
	}
	dbg_ischain("Device vaddr = %08x , size = %08x\n",
		this->minfo.dvaddr, FIMC_IS_A5_MEM_SIZE);

	this->minfo.kvaddr = (u32)vb2_ion_private_vaddr(fw_cookie);
	if (IS_ERR((void *)this->minfo.kvaddr)) {
		err("Bitprocessor memory remap failed");
		vb2_ion_private_free(fw_cookie);
		this->minfo.kvaddr = 0;
		fw_cookie = NULL;
		ret = -EIO;
		goto exit;
	}

exit:
	dbg_ischain("Virtual address for FW: %08lx\n",
		(long unsigned int)this->minfo.kvaddr);
	this->minfo.fw_cookie = fw_cookie;

	return ret;
}

static int fimc_is_ishcain_initmem(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	u32 offset;

	dbg_ischain("fimc_is_init_mem - ION\n");

	ret = fimc_is_ischain_allocmem(this);
	if (ret) {
		err("Couldn't alloc for FIMC-IS firmware\n");
		ret = -ENOMEM;
		goto exit;
	}

	offset = FW_SHARED_OFFSET;
	this->minfo.dvaddr_fshared = this->minfo.dvaddr + offset;
	this->minfo.kvaddr_fshared = this->minfo.kvaddr + offset;

	offset = FIMC_IS_A5_MEM_SIZE - FIMC_IS_REGION_SIZE;
	this->minfo.dvaddr_region = this->minfo.dvaddr + offset;
	this->minfo.kvaddr_region = this->minfo.kvaddr + offset;

	this->is_region =
		(struct is_region *)this->minfo.kvaddr_region;

	this->minfo.dvaddr_shared = this->minfo.dvaddr +
		((u32)&this->is_region->shared[0] - this->minfo.kvaddr);
	this->minfo.kvaddr_shared = this->minfo.kvaddr +
		((u32)&this->is_region->shared[0] - this->minfo.kvaddr);

	offset = FIMC_IS_A5_MEM_SIZE;
#ifdef ENABLE_ODC
	this->minfo.dvaddr_odc = this->minfo.dvaddr + offset;
	this->minfo.kvaddr_odc = this->minfo.kvaddr + offset;
	offset += (SIZE_ODC_INTERNAL_BUF * NUM_ODC_INTERNAL_BUF);
#else
	this->minfo.dvaddr_odc = 0;
	this->minfo.kvaddr_odc = 0;
#endif

#ifdef ENABLE_VDIS
	this->minfo.dvaddr_dis = this->minfo.dvaddr + offset;
	this->minfo.kvaddr_dis = this->minfo.kvaddr + offset;
	offset += (SIZE_DIS_INTERNAL_BUF * NUM_DIS_INTERNAL_BUF);
#else
	this->minfo.dvaddr_dis = 0;
	this->minfo.kvaddr_dis = 0;
#endif

#ifdef ENABLE_TDNR
	this->minfo.dvaddr_3dnr = this->minfo.dvaddr + offset;
	this->minfo.kvaddr_3dnr = this->minfo.kvaddr + offset;
#else
	this->minfo.dvaddr_3dnr = 0;
	this->minfo.kvaddr_3dnr = 0;
#endif

	dbg_ischain("fimc_is_init_mem done\n");

exit:
	return ret;
}

#ifndef RESERVED_MEM
static int fimc_is_ishcain_deinitmem(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	vb2_ion_private_free(this->minfo.fw_cookie);

	return ret;
}
#endif

static void fimc_is_ischain_cache_flush(struct fimc_is_device_ischain *this,
	u32 offset, u32 size)
{
	vb2_ion_sync_for_device(this->minfo.fw_cookie,
		offset,
		size,
		DMA_BIDIRECTIONAL);
}

static void fimc_is_ischain_region_invalid(struct fimc_is_device_ischain *this)
{
	vb2_ion_sync_for_device(
		this->minfo.fw_cookie,
		(this->minfo.kvaddr_region - this->minfo.kvaddr),
		sizeof(struct is_region),
		DMA_FROM_DEVICE);
}

static void fimc_is_ischain_region_flush(struct fimc_is_device_ischain *this)
{
	vb2_ion_sync_for_device(
		this->minfo.fw_cookie,
		(this->minfo.kvaddr_region - this->minfo.kvaddr),
		sizeof(struct is_region),
		DMA_TO_DEVICE);
}

static void fimc_is_ischain_version(char *name, const char *load_bin, u32 size)
{
	char version_str[FIMC_IS_VERSION_SIZE];

	memcpy(version_str, &load_bin[size - FIMC_IS_VERSION_SIZE],
		FIMC_IS_VERSION_SIZE);
	version_str[FIMC_IS_VERSION_SIZE-1] = '\0';
	printk(KERN_INFO "%s version : %s\n", name, version_str);
}

static int fimc_is_ischain_loadfirm(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	int location = 0;
	struct firmware *fw_blob;
	u8 *buf = NULL;
#ifdef SDCARD_FW
	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;

	dbg_ischain("%s\n", __func__);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(FIMC_IS_FW_SDCARD, O_RDONLY, 0);
	if (IS_ERR(fp))
		goto request_fw;

	location = 1;
	fw_requested = 0;
	fsize = fp->f_path.dentry->d_inode->i_size;
	dbg_ischain("start, file path %s, size %ld Bytes\n",
		FIMC_IS_FW_SDCARD, fsize);
	buf = vmalloc(fsize);
	if (!buf) {
		dev_err(&this->pdev->dev,
			"failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}
	nread = vfs_read(fp, (char __user *)buf, fsize, &fp->f_pos);
	if (nread != fsize) {
		dev_err(&this->pdev->dev,
			"failed to read firmware file, %ld Bytes\n", nread);
		ret = -EIO;
		goto out;
	}

	memcpy((void *)this->minfo.kvaddr, (void *)buf, fsize);
	fimc_is_ischain_cache_flush(this, 0, fsize + 1);
	fimc_is_ischain_version(FIMC_IS_FW, buf, fsize);

request_fw:
	if (fw_requested) {
		set_fs(old_fs);
#endif
		ret = request_firmware((const struct firmware **)&fw_blob,
			FIMC_IS_FW, &this->pdev->dev);
		if (ret) {
			ret = -EINVAL;
			goto out;
		}

		memcpy((void *)this->minfo.kvaddr, fw_blob->data,
			fw_blob->size);
		fimc_is_ischain_cache_flush(this, 0, fw_blob->size + 1);
		fimc_is_ischain_version(FIMC_IS_FW, fw_blob->data,
			fw_blob->size);

		release_firmware(fw_blob);
#ifdef SDCARD_FW
	}
#endif

out:
#ifdef SDCARD_FW
	if (!fw_requested) {
		vfree(buf);
		filp_close(fp, current->files);
		set_fs(old_fs);
	}
#endif

	if (ret)
		err("firmware loading is fail");
	else
		printk(KERN_INFO "firmware is loaded successfully from %s\n",
			location ? FIMC_IS_FW_SDCARD : "fimc_is_fw2.bin");

	return ret;
}

static int fimc_is_ischain_loadsetf(struct fimc_is_device_ischain *this,
	u32 load_addr, char *setfile_name)
{
	int ret = 0;
	int location = 0;
	void *address;
	struct firmware *fw_blob;
	u8 *buf = NULL;
#ifdef SDCARD_FW
	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;
	char setfile_path[256];

	dbg_ischain("%s\n", __func__);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	memset(setfile_path, 0x00, sizeof(setfile_path));
	sprintf(setfile_path, "%s%s",
		FIMC_IS_SETFILE_SDCARD_PATH, setfile_name);
	fp = filp_open(setfile_path, O_RDONLY, 0);
	if (IS_ERR(fp))
		goto request_fw;

	location = 1;
	fw_requested = 0;
	fsize = fp->f_path.dentry->d_inode->i_size;
	dbg_ischain("start, file path %s, size %ld Bytes\n",
		setfile_path, fsize);
	buf = vmalloc(fsize);
	if (!buf) {
		dev_err(&this->pdev->dev,
			"failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}
	nread = vfs_read(fp, (char __user *)buf, fsize, &fp->f_pos);
	if (nread != fsize) {
		dev_err(&this->pdev->dev,
			"failed to read firmware file, %ld Bytes\n", nread);
		ret = -EIO;
		goto out;
	}

	address = (void *)(this->minfo.kvaddr + load_addr);
	memcpy((void *)address, (void *)buf, fsize);
	fimc_is_ischain_cache_flush(this, load_addr, fsize + 1);
	fimc_is_ischain_version(setfile_name, buf, fsize);

request_fw:
	if (fw_requested) {
		set_fs(old_fs);
#endif

		ret = request_firmware((const struct firmware **)&fw_blob,
					setfile_name, &this->pdev->dev);
		if (ret) {
			ret = -EINVAL;
			goto out;
		}

		address = (void *)(this->minfo.kvaddr + load_addr);
		memcpy(address, fw_blob->data, fw_blob->size);
		fimc_is_ischain_cache_flush(this, load_addr, fw_blob->size + 1);
		fimc_is_ischain_version(setfile_name, fw_blob->data,
			(u32)fw_blob->size);

		release_firmware(fw_blob);
#ifdef SDCARD_FW
	}
#endif

out:
#ifdef SDCARD_FW
	if (!fw_requested) {
		vfree(buf);
		filp_close(fp, current->files);
		set_fs(old_fs);
	}
#endif

	if (ret)
		err("setfile loading is fail");
	else
		printk(KERN_INFO "setfile is loaded successfully from %s\n",
			location ? setfile_path : setfile_name);

	return ret;
}

static int fimc_is_ischain_loadcalb(struct fimc_is_device_ischain *this,
	struct fimc_is_enum_sensor *active_sensor)
{
	int ret = 0;
	char *buf;
	char *cal_ptr;

	buf = kmalloc(FIMC_IS_MAX_CAL_SIZE, GFP_KERNEL);
	if (!buf) {
		err("kmalloc fail");
		ret = -ENOMEM;
		goto exit;
	}

	memset(buf, 0xff, FIMC_IS_MAX_CAL_SIZE);

	ret = fimc_is_spi_read(buf, FIMC_IS_MAX_CAL_SIZE);
	if (ret < 0) {
		err("failed to fimc_is_spi_read (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	cal_ptr = (char *)(this->minfo.kvaddr + FIMC_IS_CAL_START_ADDR);

	if (active_sensor->sensor == SENSOR_NAME_S5K6A3) {
		buf[0x1a] = 0;
		buf[0x1b] = 0;
	}

	if (active_sensor->sensor == SENSOR_NAME_S5K6A3)
		printk(KERN_INFO "sensor name : 6A3\n");
	else if (active_sensor->sensor == SENSOR_NAME_S5K4E5)
		printk(KERN_INFO "sensor name : 4E5\n");
	else {
		printk(KERN_INFO "sensor name : Unknown\n");
		ret = -EINVAL;
		goto exit;
	}

	printk(KERN_INFO "FROM DATA(0x1A): 0x%08x\n", buf[0x1a]);
	printk(KERN_INFO "FROM DATA(0x1B): 0x%08x\n", buf[0x1b]);

	memcpy(cal_ptr, buf, FIMC_IS_MAX_CAL_SIZE);
	fimc_is_ischain_cache_flush(this, FIMC_IS_CAL_START_ADDR,
				FIMC_IS_MAX_CAL_SIZE + 1);

exit:
	kfree(buf);
	return ret;
}

static void fimc_is_ischain_forcedown(struct fimc_is_device_ischain *this,
	bool on)
{
	if (on) {
		printk(KERN_INFO "Set low poweroff mode\n");
		__raw_writel(0x0, PMUREG_ISP_ARM_OPTION);
		__raw_writel(0x1CF82000, PMUREG_ISP_LOW_POWER_OFF);
		this->force_down = true;
	} else {
		printk(KERN_INFO "Clear low poweroff mode\n");
		__raw_writel(0xFFFFFFFF, PMUREG_ISP_ARM_OPTION);
		__raw_writel(0x8, PMUREG_ISP_LOW_POWER_OFF);
		this->force_down = false;
	}
}

int fimc_is_ischain_power(struct fimc_is_device_ischain *this, int on)
{
	int ret = 0;
	u32 timeout;
	struct device *dev = &this->pdev->dev;
	struct fimc_is_core *core
		= (struct fimc_is_core *)platform_get_drvdata(this->pdev);
	struct fimc_is_device_sensor *sensor = &core->sensor;
	struct fimc_is_enum_sensor *sensor_info
		= &sensor->enum_sensor[sensor->id_position];

	printk(KERN_INFO "%s(%d)\n", __func__, on);

	if (on) {
		/* 1. force poweroff setting */
		if (this->force_down)
			fimc_is_ischain_forcedown(this, false);

		/* 2. FIMC-IS local power enable */
#if defined(CONFIG_PM_RUNTIME)
		dbg_ischain("pm_runtime_suspended = %d\n",
			pm_runtime_suspended(dev));
		pm_runtime_get_sync(dev);
#else
		fimc_is_runtime_resume(dev);
#endif

		/* 3. Load calibration data from sensor */
		fimc_is_ischain_loadcalb(this, sensor_info);

		/* 4. A5 start address setting */
		dbg_ischain("minfo.base(dvaddr) : 0x%08x\n",
			this->minfo.dvaddr);
		dbg_ischain("minfo.base(kvaddr) : 0x%08X\n",
			this->minfo.kvaddr);

		if (!this->minfo.dvaddr) {
			err("firmware device virtual is null");
			ret = -ENOMEM;
			goto exit;
		}

		writel(this->minfo.dvaddr, this->regs + BBOAR);

		/* 5. A5 power on*/
		writel(0x1, PMUREG_ISP_ARM_CONFIGURATION);

		/* 6. enable A5 */
		writel(0x00018000, PMUREG_ISP_ARM_OPTION);
		timeout = 1000;
		while ((__raw_readl(PMUREG_ISP_ARM_STATUS) & 0x1) != 0x1) {
			if (timeout == 0)
				err("A5 power on failed\n");
			timeout--;
			udelay(1);
		}

		set_bit(FIMC_IS_ISCHAIN_POWER_ON, &this->state);
	} else {
		/* 1. disable A5 */
		writel(0x10000, PMUREG_ISP_ARM_OPTION);
		/* 2. A5 power off*/
		writel(0x0, PMUREG_ISP_ARM_CONFIGURATION);

		/* 3. Check A5 power off status register */
		timeout = 1000;
		while (__raw_readl(PMUREG_ISP_ARM_STATUS) & 0x1) {
			if (timeout == 0) {
				printk(KERN_ERR "A5 power off failed\n");
				fimc_is_ischain_forcedown(this, true);
			}
			timeout--;
			udelay(1);
		}

		writel(0x0, PMUREG_CMU_RESET_ISP_SYS_PWR_REG);

		/* 4. FIMC-IS local power down */
#if defined(CONFIG_PM_RUNTIME)
		pm_runtime_put_sync(dev);
		dbg_ischain("pm_runtime_suspended = %d\n",
					pm_runtime_suspended(dev));
#else
		fimc_is_runtime_suspend(dev);
#endif
		clear_bit(FIMC_IS_ISCHAIN_POWER_ON, &this->state);
	}

exit:
	return ret;
}

static int fimc_is_itf_s_param(struct fimc_is_device_ischain *this,
	u32 indexes, u32 lindex, u32 hindex)
{
	int ret = 0;

	fimc_is_ischain_region_flush(this);

	if (lindex || hindex) {
		ret = fimc_is_hw_s_param(this->interface,
			this->instance,
			indexes,
			lindex,
			hindex);
	}

	return ret;
}

static int fimc_is_itf_a_param(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	ret = fimc_is_hw_a_param(this->interface, this->instance,
		HIC_PREVIEW_STILL, this->setfile);

	return ret;
}

static int fimc_is_itf_f_param(struct fimc_is_device_ischain *this)
{
	int ret = 0;
#ifdef DEBUG
	u32 navailable = 0;
	struct is_region *region = this->is_region;
#endif

	dbg_ischain(" NAME    ON  BYPASS       SIZE  FORMAT\n");
	dbg_ischain("ISP OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.isp.control.cmd,
		region->parameter.isp.control.bypass,
		region->parameter.isp.otf_output.width,
		region->parameter.isp.otf_output.height,
		region->parameter.isp.otf_output.format
		);
	dbg_ischain("DRC OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.drc.control.cmd,
		region->parameter.drc.control.bypass,
		region->parameter.drc.otf_input.width,
		region->parameter.drc.otf_input.height,
		region->parameter.drc.otf_input.format
		);
	dbg_ischain("DRC OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.drc.control.cmd,
		region->parameter.drc.control.bypass,
		region->parameter.drc.otf_output.width,
		region->parameter.drc.otf_output.height,
		region->parameter.drc.otf_output.format
		);
	dbg_ischain("SCC OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.scalerc.control.cmd,
		region->parameter.scalerc.control.bypass,
		region->parameter.scalerc.otf_input.width,
		region->parameter.scalerc.otf_input.height,
		region->parameter.scalerc.otf_input.format
		);
	dbg_ischain("SCC OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.scalerc.control.cmd,
		region->parameter.scalerc.control.bypass,
		region->parameter.scalerc.otf_output.width,
		region->parameter.scalerc.otf_output.height,
		region->parameter.scalerc.otf_output.format
		);
	dbg_ischain("ODC OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.odc.control.cmd,
		region->parameter.odc.control.bypass,
		region->parameter.odc.otf_input.width,
		region->parameter.odc.otf_input.height,
		region->parameter.odc.otf_input.format
		);
	dbg_ischain("ODC OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.odc.control.cmd,
		region->parameter.odc.control.bypass,
		region->parameter.odc.otf_output.width,
		region->parameter.odc.otf_output.height,
		region->parameter.odc.otf_output.format
		);
	dbg_ischain("DIS OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.dis.control.cmd,
		region->parameter.dis.control.bypass,
		region->parameter.dis.otf_input.width,
		region->parameter.dis.otf_input.height,
		region->parameter.dis.otf_input.format
		);
	dbg_ischain("DIS OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.dis.control.cmd,
		region->parameter.dis.control.bypass,
		region->parameter.dis.otf_output.width,
		region->parameter.dis.otf_output.height,
		region->parameter.dis.otf_output.format
		);
	dbg_ischain("DNR OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.tdnr.control.cmd,
		region->parameter.tdnr.control.bypass,
		region->parameter.tdnr.otf_input.width,
		region->parameter.tdnr.otf_input.height,
		region->parameter.tdnr.otf_input.format
		);
	dbg_ischain("DNR OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.tdnr.control.cmd,
		region->parameter.tdnr.control.bypass,
		region->parameter.tdnr.otf_output.width,
		region->parameter.tdnr.otf_output.height,
		region->parameter.tdnr.otf_output.format
		);
	dbg_ischain("SCP OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.scalerp.control.cmd,
		region->parameter.scalerp.control.bypass,
		region->parameter.scalerp.otf_input.width,
		region->parameter.scalerp.otf_input.height,
		region->parameter.scalerp.otf_input.format
		);
	dbg_ischain("SCP DO : %2d    %4d  %04dx%04d    %d,%d\n",
		region->parameter.scalerp.dma_output.cmd,
		region->parameter.scalerp.control.bypass,
		region->parameter.scalerp.dma_output.width,
		region->parameter.scalerp.dma_output.height,
		region->parameter.scalerp.dma_output.format,
		region->parameter.scalerp.dma_output.plane
		);
	dbg_ischain("SCP OO : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.scalerp.control.cmd,
		region->parameter.scalerp.control.bypass,
		region->parameter.scalerp.otf_output.width,
		region->parameter.scalerp.otf_output.height,
		region->parameter.scalerp.otf_output.format
		);
	dbg_ischain(" FD OI : %2d    %4d  %04dx%04d      %d\n",
		region->parameter.fd.control.cmd,
		region->parameter.fd.control.bypass,
		region->parameter.fd.otf_input.width,
		region->parameter.fd.otf_input.height,
		region->parameter.fd.otf_input.format
		);
	dbg_ischain(" NAME   CMD    IN_SZIE   OT_SIZE      CROP       POS\n");
	dbg_ischain("SCC CI :  %d  %04dx%04d %04dx%04d %04dx%04d %04dx%04d\n",
		region->parameter.scalerc.input_crop.cmd,
		region->parameter.scalerc.input_crop.in_width,
		region->parameter.scalerc.input_crop.in_height,
		region->parameter.scalerc.input_crop.out_width,
		region->parameter.scalerc.input_crop.out_height,
		region->parameter.scalerc.input_crop.crop_width,
		region->parameter.scalerc.input_crop.crop_height,
		region->parameter.scalerc.input_crop.pos_x,
		region->parameter.scalerc.input_crop.pos_y
		);
	dbg_ischain("SCC CO :  %d  %04dx%04d %04dx%04d %04dx%04d %04dx%04d\n",
		region->parameter.scalerc.output_crop.cmd,
		navailable,
		navailable,
		navailable,
		navailable,
		region->parameter.scalerc.output_crop.crop_width,
		region->parameter.scalerc.output_crop.crop_height,
		region->parameter.scalerc.output_crop.pos_x,
		region->parameter.scalerc.output_crop.pos_y
		);
	dbg_ischain("SCP CI :  %d  %04dx%04d %04dx%04d %04dx%04d %04dx%04d\n",
		region->parameter.scalerp.input_crop.cmd,
		region->parameter.scalerp.input_crop.in_width,
		region->parameter.scalerp.input_crop.in_height,
		region->parameter.scalerp.input_crop.out_width,
		region->parameter.scalerp.input_crop.out_height,
		region->parameter.scalerp.input_crop.crop_width,
		region->parameter.scalerp.input_crop.crop_height,
		region->parameter.scalerp.input_crop.pos_x,
		region->parameter.scalerp.input_crop.pos_y
		);
	dbg_ischain("SCP CO :  %d  %04dx%04d %04dx%04d %04dx%04d %04dx%04d\n",
		region->parameter.scalerp.output_crop.cmd,
		navailable,
		navailable,
		navailable,
		navailable,
		region->parameter.scalerp.output_crop.crop_width,
		region->parameter.scalerp.output_crop.crop_height,
		region->parameter.scalerp.output_crop.pos_x,
		region->parameter.scalerp.output_crop.pos_y
		);
	dbg_ischain(" NAME   BUFS		MASK\n");
	dbg_ischain("SCP DO : %2d    %04X\n",
		region->parameter.scalerp.dma_output.buffer_number,
		region->parameter.scalerp.dma_output.dma_out_mask
		);

	ret = fimc_is_hw_f_param(this->interface, this->instance);

	return ret;
}

static int fimc_is_itf_enum(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	dbg_ischain("%s()\n", __func__);

	ret = fimc_is_hw_enum(this->interface);
	if (ret)
		err("fimc_is_itf_enum is fail(%d)", ret);

	return ret;
}

static int fimc_is_itf_open(struct fimc_is_device_ischain *this,
	u32 input, u32 channel, u32 ext)
{
	int ret = 0;
	struct is_region *region = this->is_region;

	fimc_is_ischain_region_flush(this);

	ret = fimc_is_hw_open(this->interface, this->instance, input, channel,
		ext, &this->margin_width, &this->margin_height);
	if (ret) {
		err("fimc_is_hw_open is fail");
		goto exit;
	}

	/*hack*/
	this->margin_left = 8;
	this->margin_right = 8;
	this->margin_top = 6;
	this->margin_bottom = 4;
	this->margin_width = this->margin_left + this->margin_right;
	this->margin_height = this->margin_top + this->margin_bottom;
	dbg_ischain("margin %dx%d\n",
		this->margin_width, this->margin_height);

	if (region->shared[MAX_SHARED_COUNT-1] != MAGIC_NUMBER) {
		err("MAGIC NUMBER error\n");
		ret = 1;
		goto exit;
	}

	memset(&region->parameter, 0x0, sizeof(struct is_param_region));

	memcpy(&region->parameter.sensor, &init_sensor_param,
		sizeof(struct sensor_param));
	memcpy(&region->parameter.isp, &init_isp_param,
		sizeof(struct isp_param));
	memcpy(&region->parameter.drc, &init_drc_param,
		sizeof(struct drc_param));
	memcpy(&region->parameter.scalerc, &init_scalerc_param,
		sizeof(struct scalerc_param));
	memcpy(&region->parameter.odc, &init_odc_param,
		sizeof(struct odc_param));
	memcpy(&region->parameter.dis, &init_dis_param,
		sizeof(struct dis_param));
	memcpy(&region->parameter.tdnr, &init_tdnr_param,
		sizeof(struct tdnr_param));
	memcpy(&region->parameter.scalerp, &init_scalerp_param,
		sizeof(struct scalerp_param));
	memcpy(&region->parameter.fd, &init_fd_param,
		sizeof(struct fd_param));

exit:
	return ret;
}

static int fimc_is_itf_setfile(struct fimc_is_device_ischain *this,
	char *setfile_name)
{
	int ret = 0;
	u32 setfile_addr;

	dbg_ischain("%s(setfile : %s)\n", __func__, setfile_name);

	ret = fimc_is_hw_saddr(this->interface, this->instance, &setfile_addr);
	fimc_is_ischain_loadsetf(this, setfile_addr, setfile_name);
	fimc_is_hw_setfile(this->interface, this->instance);

	return ret;
}

static int fimc_is_itf_cfg_mem(struct fimc_is_device_ischain *this,
	u32 shot_addr, u32 shot_size)
{
	int ret = 0;

	dbg_ischain("%s()\n", __func__);

	ret = fimc_is_hw_cfg_mem(this->interface, this->instance,
		shot_addr, shot_size);

	return ret;
}

static int fimc_is_itf_s_camctrl(struct fimc_is_device_ischain *this,
	u32 address, u32 fnumber)
{
	int ret = 0;

	fimc_is_ischain_region_flush(this);

	ret = fimc_is_hw_s_camctrl_nblk(this->interface,
		this->instance,
		address,
		fnumber);

	return ret;
}

int fimc_is_itf_stream_on(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	ret = fimc_is_hw_stream_on(this->interface, this->instance);

	return ret;
}

int fimc_is_itf_stream_off(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	ret = fimc_is_hw_stream_off(this->interface, this->instance);

	return ret;
}

int fimc_is_itf_process_on(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	ret = fimc_is_hw_process_on(this->interface, this->instance);

	return ret;
}

int fimc_is_itf_process_off(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	ret = fimc_is_hw_process_off(this->interface, this->instance);

	return ret;
}

int fimc_is_itf_g_capability(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	u32 metadata;
	u32 index;
	struct camera2_sm *capability;

	ret = fimc_is_hw_g_capability(this->interface, this->instance,
		(this->minfo.kvaddr_shared - this->minfo.kvaddr));

	fimc_is_ischain_region_invalid(this);

	memcpy(&this->capability, &this->is_region->shared,
		sizeof(struct camera2_sm));
	capability = &this->capability;

#if 1
	dbg_ischain("===ColorC================================\n");
#if 0
	index = 0;
	metadata = capability->color.availableModes[index];
	while (metadata) {
		dbg_ischain("availableModes : %d\n", metadata);
		index++;
		metadata = capability->color.availableModes[index];
	}
#endif

	dbg_ischain("===ToneMapping===========================\n");
	metadata = capability->tonemap.maxCurvePoints;
	dbg_ischain("maxCurvePoints : %d\n", metadata);

	dbg_ischain("===Scaler================================\n");
	dbg_ischain("foramt : %d, %d, %d, %d\n",
		capability->scaler.availableFormats[0],
		capability->scaler.availableFormats[1],
		capability->scaler.availableFormats[2],
		capability->scaler.availableFormats[3]);

	dbg_ischain("===StatisTicsG===========================\n");
	index = 0;
	metadata = capability->stats.availableFaceDetectModes[index];
	while (metadata) {
		dbg_ischain("availableFaceDetectModes : %d\n", metadata);
		index++;
		metadata = capability->stats.availableFaceDetectModes[index];
	}
	dbg_ischain("maxFaceCount : %d\n",
		capability->stats.maxFaceCount);
	dbg_ischain("histogrambucketCount : %d\n",
		capability->stats.histogramBucketCount);
	dbg_ischain("maxHistogramCount : %d\n",
		capability->stats.maxHistogramCount);
	dbg_ischain("sharpnessMapSize : %dx%d\n",
		capability->stats.sharpnessMapSize[0],
		capability->stats.sharpnessMapSize[1]);
	dbg_ischain("maxSharpnessMapValue : %d\n",
		capability->stats.maxSharpnessMapValue);

	dbg_ischain("===3A====================================\n");
#if 0
	index = 0;
	metadata = capability->aa.availableModes[index];
	while (metadata) {
		dbg_ischain("availableModes : %d\n", metadata);
		index++;
		metadata = capability->aa.availableModes[index];
	}
#endif

	dbg_ischain("maxRegions : %d\n", capability->aa.maxRegions);

	index = 0;
	metadata = capability->aa.aeAvailableModes[index];
	while (metadata) {
		dbg_ischain("aeAvailableModes : %d\n", metadata);
		index++;
		metadata = capability->aa.aeAvailableModes[index];
	}
	dbg_ischain("aeCompensationStep : %d,%d\n",
		capability->aa.aeCompensationStep.num,
		capability->aa.aeCompensationStep.den);
	dbg_ischain("aeCompensationRange : %d ~ %d\n",
		capability->aa.aeCompensationRange[0],
		capability->aa.aeCompensationRange[1]);
	index = 0;
	metadata = capability->aa.aeAvailableTargetFpsRanges[index][0];
	while (metadata) {
		dbg_ischain("TargetFpsRanges : %d ~ %d\n", metadata,
			capability->aa.aeAvailableTargetFpsRanges[index][1]);
		index++;
		metadata = capability->aa.aeAvailableTargetFpsRanges[index][0];
	}
	index = 0;
	metadata = capability->aa.aeAvailableAntibandingModes[index];
	while (metadata) {
		dbg_ischain("aeAvailableAntibandingModes : %d\n", metadata);
		index++;
		metadata = capability->aa.aeAvailableAntibandingModes[index];
	}
	index = 0;
	metadata = capability->aa.awbAvailableModes[index];
	while (metadata) {
		dbg_ischain("awbAvailableModes : %d\n", metadata);
		index++;
		metadata = capability->aa.awbAvailableModes[index];
	}
	index = 0;
	metadata = capability->aa.afAvailableModes[index];
	while (metadata) {
		dbg_ischain("afAvailableModes : %d\n", metadata);
		index++;
		metadata = capability->aa.afAvailableModes[index];
	}
#endif

	return ret;
}

static int fimc_is_itf_power_down(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	ret = fimc_is_hw_power_down(this->interface, this->instance);

	return ret;
}

static int fimc_is_itf_shot(struct fimc_is_device_ischain *this,
	struct fimc_is_frame_shot *frame)
{
	int ret = 0;

	this->fcount++;
	if (frame->shot->dm.request.frameCount != this->fcount) {
#ifdef CHECK_FDROP
		printk(KERN_INFO "shot mismatch(%d, %d)",
			frame->shot->dm.request.frameCount, this->fcount);
#endif

		if (!frame->shot->dm.request.frameCount) {
			frame->fcount = this->fcount;
			frame->shot->dm.request.frameCount = this->fcount;
		}

		this->fcount = frame->shot->dm.request.frameCount;
	}

	/* Cache Flush */
	vb2_ion_sync_for_device((void *)frame->cookie_shot, 0, frame->shot_size,
		DMA_TO_DEVICE);

	if (frame->shot->magicNumber != 0x23456789) {
		err("shot magic number error(0x%08X)\n",
			frame->shot->magicNumber);
		ret = 1;
		goto exit;
	}

#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME
	do_gettimeofday(&frame->time_shot);
#else
	do_gettimeofday(&frame->tzone[TM_SHOT]);
#endif
#endif

#ifdef DBG_STREAMING
	printk(KERN_INFO "shot(%d)\n", frame->shot->dm.request.frameCount);
#endif

	ret = fimc_is_hw_shot_nblk(this->interface, this->instance,
		frame->dvaddr_buffer[0],
		frame->dvaddr_shot,
		frame->shot->dm.request.frameCount,
		frame->shot->ctl.request.frameCount);

exit:
	return ret;
}

int fimc_is_ischain_probe(struct fimc_is_device_ischain *this,
	struct fimc_is_interface *interface,
	struct fimc_is_framemgr *framemgr,
	struct fimc_is_mem *mem,
	struct platform_device *pdev,
	u32 regs)
{
	int ret = 0;
	struct fimc_is_ischain_dev *scc, *scp;

	/*this initialization should be just one time*/

	if (interface == NULL) {
		err("interface is null");
		ret = -EINVAL;
		goto exit;
	}

	if (framemgr == NULL) {
		err("framemgr is null");
		ret = -EINVAL;
		goto exit;
	}

	if (mem == NULL) {
		err("mem is null");
		ret = -EINVAL;
		goto exit;
	}

	if (pdev == NULL) {
		err("pdev is null");
		ret = -EINVAL;
		goto exit;
	}

	scc = &this->scc;
	scp = &this->scp;

#if defined(CONFIG_BUSFREQ_OPP) && defined(CONFIG_CPU_EXYNOS5250)
	this->bus_dev		= dev_get("exynos-busfreq");
#endif

	this->force_down	= false;
	this->interface		= interface;
	this->framemgr		= framemgr;
	this->mem		= mem;
	this->pdev		= pdev;
	this->pdata		= pdev->dev.platform_data;
	this->regs		= (void *)regs;

#ifdef RESERVED_MEM
	fimc_is_ishcain_initmem(this);
#endif

	this->margin_left	= 0;
	this->margin_right	= 0;
	this->margin_width	= 0;
	this->margin_top	= 0;
	this->margin_bottom	= 0;
	this->margin_height	= 0;
	this->sensor_width	= 0;
	this->sensor_height	= 0;
	this->chain0_width	= 0;
	this->chain0_height	= 0;
	this->chain1_width	= 0;
	this->chain1_height	= 0;
	this->chain2_width	= 0;
	this->chain2_height	= 0;
	this->chain3_width	= 0;
	this->chain3_height	= 0;
	this->crop_x		= 0;
	this->crop_y		= 0;
	this->crop_width	= 0;
	this->crop_height	= 0;
	this->force_down	= false;

	/*scc probe*/
	fimc_is_frame_probe(&scc->framemgr, FRAMEMGR_ID_SCC);

	/*scp probe*/
	fimc_is_frame_probe(&scp->framemgr, FRAMEMGR_ID_SCP);

	clear_bit(FIMC_IS_ISCHAIN_OPEN, &this->state);
	clear_bit(FIMC_IS_ISCHAIN_LOADED, &this->state);
	clear_bit(FIMC_IS_ISCHAIN_POWER_ON, &this->state);
	clear_bit(FIMC_IS_ISCHAIN_RUN, &this->state);

	mutex_init(&this->mutex_state);
	spin_lock_init(&this->slock_state);

#ifdef FW_DEBUG
	debugfs_root = debugfs_create_dir(DEBUG_FS_ROOT_NAME, NULL);
	if (debugfs_root)
		dbg_ischain("debugfs %s is created", DEBUG_FS_ROOT_NAME);

	debugfs_file = debugfs_create_file(DEBUG_FS_FILE_NAME, S_IRUSR,
		debugfs_root, this, &debug_fops);
	if (debugfs_file)
		dbg_ischain("debugfs %s is created", DEBUG_FS_FILE_NAME);
#endif

exit:
	return 0;
}

int fimc_is_ischain_open(struct fimc_is_device_ischain *this,
	struct fimc_is_video_common *video)
{
	int ret = 0;

	if (testnset_state(this, FIMC_IS_ISCHAIN_OPEN)) {
		err("already open");
		ret = -EMFILE;
		goto exit;
	}

	printk(KERN_INFO "+++%s()\n", __func__);

	clear_bit(FIMC_IS_ISCHAIN_LOADED, &this->state);
	clear_bit(FIMC_IS_ISCHAIN_POWER_ON, &this->state);
	clear_bit(FIMC_IS_ISCHAIN_RUN, &this->state);

#ifndef RESERVED_MEM
	/* 1. init memory */
	ret = fimc_is_ishcain_initmem(this);
	if (ret) {
		err("fimc_is_ishcain_initmem is fail(%d)\n", ret);
		goto exit;
	}
#endif

	/* 2. Init variables */
	memset(&this->cur_peri_ctl, 0,
		sizeof(struct camera2_uctl));
	memset(&this->peri_ctls, 0,
		sizeof(struct camera2_uctl)*SENSOR_MAX_CTL);
	memset(&this->capability, 0,
		sizeof(struct camera2_sm));

	/* initial state, it's real apply to setting when opening*/
	this->margin_left	= 0;
	this->margin_right	= 0;
	this->margin_width	= 0;
	this->margin_top	= 0;
	this->margin_bottom	= 0;
	this->margin_height	= 0;
	/* open function is called later than set input
	so this code is commented.
	this code will be used after sensor and
	ischain power is seperated */
	/*
	this->sensor_width	= 0;
	this->sensor_height	= 0;
	this->chain0_width	= 0;
	this->chain0_height	= 0;
	this->chain1_width	= 0;
	this->chain1_height	= 0;
	this->chain2_width	= 0;
	this->chain2_height	= 0;
	this->chain3_width	= 0;
	this->chain3_height	= 0;
	*/

	this->fcount = 0;
	this->setfile = 0;
	this->debug_cnt = 0;
	this->dzoom_width = 0;

	/* frame manager open */
	fimc_is_interface_open(this->interface);

	/* other device open */
	fimc_is_ischain_dev_open(&this->isp, ENTRY_ISP, video,
		NULL);
	fimc_is_ischain_dev_open(&this->drc, ENTRY_DRC, NULL,
		&init_drc_param.control);
	fimc_is_ischain_dev_open(&this->dis, ENTRY_DIS, NULL,
		&init_dis_param.control);
	fimc_is_ischain_dev_open(&this->dnr, ENTRY_TDNR, NULL,
		&init_tdnr_param.control);
	/* FD see only control.command not bypass */
	fimc_is_ischain_dev_open(&this->fd, ENTRY_LHFD, NULL,
		NULL);

	/*fimc_is_fw_clear_irq1_all(core);*/

	/* 3. Load IS firmware */
	ret = fimc_is_ischain_loadfirm(this);
	if (ret) {
		err("failed to fimc_is_request_firmware (%d)\n", ret);
		clear_bit(FIMC_IS_ISCHAIN_LOADED, &this->state);
		ret = -EINVAL;
		goto exit;
	}
	set_bit(FIMC_IS_ISCHAIN_LOADED, &this->state);

	/* 4. Disable AFTR cpu low power idle enter */
	pm_qos_add_request(&pm_qos_req_cpu, PM_QOS_CPU_DMA_LATENCY, 100);

	/* memory minimun clock : 667Mhz */
	if (!isp_mif_handle_min) {
		isp_mif_handle_min = exynos5_bus_mif_min(667000);
		if (!isp_mif_handle_min)
			err("exynos5_bus_mif_min is fail");
	} else
		err("exynos5_bus_mif_min is already applied");

	/* internal bus lock to 266Mhz */
	if (!isp_int_handle_min) {
		isp_int_handle_min = exynos5_bus_int_min(266000);
		if (!isp_int_handle_min)
			err("exynos5_bus_int_min is fail");
	} else
		err("exynos5_bus_int_min is already applied");

	/* 5. A5 power on */
	ret = fimc_is_ischain_power(this, 1);
	if (ret) {
		err("failed to fimc_is_ischain_power (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
	dbg_ischain("power up and loaded firmware\n");

	/* bts api for bandwidth guarantee */
	bts_change_bus_traffic(&this->pdev->dev, BTS_INCREASE_BW);

	printk(KERN_INFO "---%s(%d)\n", __func__, ret);

exit:
	return ret;
}

int fimc_is_ischain_close(struct fimc_is_device_ischain *this)
{
	int ret = 0;

	if (testnclr_state(this, FIMC_IS_ISCHAIN_OPEN)) {
		err("already close");
		ret = -EMFILE;
		goto exit;
	}

	printk(KERN_INFO "+++%s\n", __func__);

	/* 1. Stop all request */
	ret = fimc_is_ischain_isp_stop(this);
	if (ret)
		err("fimc_is_ischain_isp_stop is fail");

	/* 2. Stop a5 and other devices operation */
	ret = fimc_is_itf_power_down(this);
	if (ret)
		err("power down is failed, retry forcelly");

	/* 3. Deinit variables */
	ret = fimc_is_interface_close(this->interface);
	if (ret)
		err("fimc_is_interface_close is failed");

	/* 4. bus traffic low */
	bts_change_bus_traffic(&this->pdev->dev, BTS_DECREASE_BW);

	/* 5. Power down */
	ret = fimc_is_ischain_power(this, 0);
	if (ret)
		err("fimc_is_ischain_power is failed");

	/* 6. Enable AFTR cpu low power idle enter */
	pm_qos_remove_request(&pm_qos_req_cpu);

	/* memory clock unlock */
	if (isp_mif_handle_min) {
		exynos5_bus_mif_put(isp_mif_handle_min);
		isp_mif_handle_min = NULL;
	} else
		err("exynos5_bus_mif_put is already applied");

	/* internal bus unlock */
	if (isp_int_handle_min) {
		exynos5_bus_int_put(isp_int_handle_min);
		isp_int_handle_min = NULL;
	} else
		err("exynos5_bus_int_put is already applied");

#ifndef RESERVED_MEM
	/* 7. Dealloc memroy */
	fimc_is_ishcain_deinitmem(this);
#endif

	printk(KERN_INFO "---%s(%d)\n", __func__, ret);

exit:
	return ret;
}

int fimc_is_ischain_init(struct fimc_is_device_ischain *this,
	u32 input, u32 channel, struct sensor_open_extended *ext,
	char *setfile_name)
{
	int ret = 0;

	dbg_ischain("%s(input : %d, channel : %d, Af : %d)\n",
		__func__, input, channel, ext->actuator_con.product_name);

	ret = fimc_is_itf_enum(this);
	if (ret) {
		err("enum fail");
		goto exit;
	}

	memcpy(&this->is_region->shared[0], ext,
		sizeof(struct sensor_open_extended));

	ret = fimc_is_itf_open(this, input, channel, this->minfo.dvaddr_shared);
	if (ret) {
		err("open fail");
		goto exit;
	}

	ret = fimc_is_itf_setfile(this, setfile_name);
	if (ret) {
		err("setfile fail");
		goto exit;
	}

	ret = fimc_is_itf_stream_off(this);
	if (ret) {
		err("streamoff fail");
		goto exit;
	}

	ret = fimc_is_itf_process_off(this);
	if (ret) {
		err("processoff fail");
		goto exit;
	}

exit:
	return ret;
}

static int fimc_is_ischain_s_setfile(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	bool process = true;

	printk(KERN_INFO "setfile is %d\n", this->setfile);

	if (this->setfile >= ISS_SUB_END) {
		err("setfile id(%d) is invalid\n", this->setfile);
		goto exit;
	}
	process = false;

	ret = fimc_is_itf_process_off(this);
	if (ret) {
		err("fimc_is_itf_process_off fail\n");
		goto exit;
	}

	ret = fimc_is_itf_a_param(this);
	if (ret) {
		err("fimc_is_itf_a_param is fail\n");
		goto exit;
	}

	ret = fimc_is_itf_process_on(this);
	if (ret) {
		err("fimc_is_itf_process_on fail1\n");
		goto exit;
	}
	process = true;

exit:
	if (!process)
		if (fimc_is_itf_process_on(this))
			err("fimc_is_itf_process_on fail2\n");

	return ret;
}

static int fimc_is_ischain_s_chain0_size(struct fimc_is_device_ischain *this,
	u32 width, u32 height)
{
	int ret = 0;
	struct isp_param *isp_param;
	struct drc_param *drc_param;
	struct scalerc_param *scc_param;

	u32 chain0_width, chain0_height;
	u32 indexes, lindex, hindex;

	isp_param = &this->is_region->parameter.isp;
	drc_param = &this->is_region->parameter.drc;
	scc_param = &this->is_region->parameter.scalerc;
	indexes = lindex = hindex = 0;
	chain0_width = width;
	chain0_height = height;

	dbg_ischain("request chain0 size : %dx%d\n",
		chain0_width, chain0_height);

	isp_param->otf_output.cmd = OTF_OUTPUT_COMMAND_ENABLE;
	isp_param->otf_output.width = chain0_width;
	isp_param->otf_output.height = chain0_height;
	isp_param->otf_output.format = OTF_OUTPUT_FORMAT_YUV444;
	isp_param->otf_output.bitwidth = OTF_OUTPUT_BIT_WIDTH_12BIT;
	isp_param->otf_output.order = OTF_INPUT_ORDER_BAYER_GR_BG;
	lindex |= LOWBIT_OF(PARAM_ISP_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_ISP_OTF_OUTPUT);
	indexes++;

	isp_param->dma1_output.cmd = DMA_OUTPUT_COMMAND_DISABLE;
	lindex |= LOWBIT_OF(PARAM_ISP_DMA1_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_ISP_DMA1_OUTPUT);
	indexes++;

	isp_param->dma2_output.cmd = DMA_OUTPUT_COMMAND_DISABLE;
	lindex |= LOWBIT_OF(PARAM_ISP_DMA2_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_ISP_DMA2_OUTPUT);
	indexes++;

	/* DRC */
	drc_param->otf_input.cmd = OTF_INPUT_COMMAND_ENABLE;
	drc_param->otf_input.width = chain0_width;
	drc_param->otf_input.height = chain0_height;
	lindex |= LOWBIT_OF(PARAM_DRC_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_DRC_OTF_INPUT);
	indexes++;

	drc_param->otf_output.cmd = OTF_OUTPUT_COMMAND_ENABLE;
	drc_param->otf_output.width = chain0_width;
	drc_param->otf_output.height = chain0_height;
	lindex |= LOWBIT_OF(PARAM_DRC_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_DRC_OTF_OUTPUT);
	indexes++;

	/* SCC */
	scc_param->otf_input.cmd = OTF_INPUT_COMMAND_ENABLE;
	scc_param->otf_input.width = chain0_width;
	scc_param->otf_input.height = chain0_height;
	lindex |= LOWBIT_OF(PARAM_SCALERC_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_OTF_INPUT);
	indexes++;

	return ret;
}

static int fimc_is_ischain_s_chain1_size(struct fimc_is_device_ischain *this,
	u32 width, u32 height)
{
	int ret = 0;
	struct scalerc_param *scc_param;
	struct odc_param *odc_param;
	struct dis_param *dis_param;
	u32 chain0_width, chain0_height;
	u32 chain1_width, chain1_height;
	u32 indexes, lindex, hindex;

	scc_param = &this->is_region->parameter.scalerc;
	odc_param = &this->is_region->parameter.odc;
	dis_param = &this->is_region->parameter.dis;
	indexes = lindex = hindex = 0;
	chain0_width = this->chain0_width;
	chain0_height = this->chain0_height;
	chain1_width = width;
	chain1_height = height;

	dbg_ischain("current chain0 size : %dx%d\n",
		chain0_width, chain0_height);
	dbg_ischain("current chain1 size : %dx%d\n",
		this->chain1_width, this->chain1_height);
	dbg_ischain("request chain1 size : %dx%d\n",
		chain1_width, chain1_height);

	if (!chain0_width) {
		err("chain0 width is zero");
		ret = -EINVAL;
		goto exit;
	}

	if (!chain0_height) {
		err("chain0 height is zero");
		ret = -EINVAL;
		goto exit;
	}

	if (!chain1_width) {
		err("chain1 width is zero");
		ret = -EINVAL;
		goto exit;
	}

	if (!chain1_height) {
		err("chain1 height is zero");
		ret = -EINVAL;
		goto exit;
	}

	/* SCC OUTPUT */
	scc_param->input_crop.cmd = SCALER_CROP_COMMAND_ENABLE;
	scc_param->input_crop.pos_x = 0;
	scc_param->input_crop.pos_y = 0;
	scc_param->input_crop.crop_width = chain0_width;
	scc_param->input_crop.crop_height = chain0_height;
	scc_param->input_crop.in_width = chain0_width;
	scc_param->input_crop.in_height = chain0_height;
	scc_param->input_crop.out_width = chain1_width;
	scc_param->input_crop.out_height = chain1_height;
	lindex |= LOWBIT_OF(PARAM_SCALERC_INPUT_CROP);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_INPUT_CROP);
	indexes++;

	scc_param->output_crop.cmd = SCALER_CROP_COMMAND_DISABLE;
	scc_param->output_crop.pos_x = 0;
	scc_param->output_crop.pos_y = 0;
	scc_param->output_crop.crop_width = chain1_width;
	scc_param->output_crop.crop_height = chain1_height;
	lindex |= LOWBIT_OF(PARAM_SCALERC_OUTPUT_CROP);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_OUTPUT_CROP);
	indexes++;

	scc_param->otf_output.cmd = OTF_OUTPUT_COMMAND_ENABLE;
	scc_param->otf_output.width = chain1_width;
	scc_param->otf_output.height = chain1_height;
	lindex |= LOWBIT_OF(PARAM_SCALERC_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_OTF_OUTPUT);
	indexes++;

	/* ODC */
	odc_param->otf_input.width = chain1_width;
	odc_param->otf_input.height = chain1_height;
	lindex |= LOWBIT_OF(PARAM_ODC_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_ODC_OTF_INPUT);
	indexes++;

	odc_param->otf_output.width = chain1_width;
	odc_param->otf_output.height = chain1_height;
	lindex |= LOWBIT_OF(PARAM_ODC_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_ODC_OTF_OUTPUT);
	indexes++;

	/* DIS INPUT */
	dis_param->otf_input.width = chain1_width;
	dis_param->otf_input.height = chain1_height;
	lindex |= LOWBIT_OF(PARAM_DIS_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_DIS_OTF_INPUT);
	indexes++;

	this->lindex |= lindex;
	this->hindex |= hindex;
	this->indexes += indexes;

exit:
	return ret;
}

static int fimc_is_ischain_s_chain2_size(struct fimc_is_device_ischain *this,
	u32 width, u32 height)
{
	int ret = 0;
	struct dis_param *dis_param;
	struct tdnr_param *tdnr_param;
	struct scalerp_param *scp_param;

	u32 chain2_width, chain2_height;
	u32 indexes, lindex, hindex;

	dbg_ischain("request chain2 size : %dx%d\n", width, height);
	dbg_ischain("current chain2 size : %dx%d\n",
		this->chain2_width, this->chain2_height);

	dis_param = &this->is_region->parameter.dis;
	tdnr_param = &this->is_region->parameter.tdnr;
	scp_param = &this->is_region->parameter.scalerp;
	indexes = lindex = hindex = 0;

	/* CALCULATION */
	chain2_width = width;
	chain2_height = height;

	/* DIS OUTPUT */
	dis_param->otf_output.width = chain2_width;
	dis_param->otf_output.height = chain2_height;
	lindex |= LOWBIT_OF(PARAM_DIS_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_DIS_OTF_OUTPUT);
	indexes++;

	/* 3DNR */
	tdnr_param->otf_input.width = chain2_width;
	tdnr_param->otf_input.height = chain2_height;
	lindex |= LOWBIT_OF(PARAM_TDNR_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_TDNR_OTF_INPUT);
	indexes++;

	tdnr_param->dma_output.width = chain2_width;
	tdnr_param->dma_output.height = chain2_height;
	lindex |= LOWBIT_OF(PARAM_TDNR_DMA_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_TDNR_DMA_OUTPUT);
	indexes++;

	tdnr_param->otf_output.width = chain2_width;
	tdnr_param->otf_output.height = chain2_height;
	lindex |= LOWBIT_OF(PARAM_TDNR_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_TDNR_OTF_OUTPUT);
	indexes++;

	/* SCALERP INPUT */
	scp_param->otf_input.width = chain2_width;
	scp_param->otf_input.height = chain2_height;
	lindex |= LOWBIT_OF(PARAM_SCALERP_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_OTF_INPUT);
	indexes++;

	this->lindex |= lindex;
	this->hindex |= hindex;
	this->indexes += indexes;

	return ret;
}

static int fimc_is_ischain_s_chain3_size(struct fimc_is_device_ischain *this,
	u32 width, u32 height)
{
	int ret = 0;
	struct scalerp_param *scp_param;
	struct fd_param *fd_param;
	struct fimc_is_video_common *video;
	u32 chain2_width, chain2_height;
	u32 chain3_width, chain3_height;
	u32 scp_crop_width, scp_crop_height;
	u32 scp_crop_x, scp_crop_y;
	u32 indexes, lindex, hindex;

	scp_param = &this->is_region->parameter.scalerp;
	fd_param = &this->is_region->parameter.fd;
	video = this->scp.video;
	indexes = lindex = hindex = 0;

	chain2_width = this->chain2_width;
	chain2_height = this->chain2_height;

	chain3_width = width;
	chain3_height = height;

	scp_crop_x = 0;
	scp_crop_y = 0;
	scp_crop_width = chain2_width;
	scp_crop_height = chain2_height;

	dbg_ischain("request chain3 size : %dx%d\n", width, height);
	dbg_ischain("current chain3 size : %dx%d\n",
		this->chain3_width, this->chain3_height);

	/*SCALERP*/
	scp_param->input_crop.cmd = SCALER_CROP_COMMAND_ENABLE;
	scp_param->input_crop.pos_x = scp_crop_x;
	scp_param->input_crop.pos_y = scp_crop_y;
	scp_param->input_crop.crop_width = scp_crop_width;
	scp_param->input_crop.crop_height = scp_crop_height;
	scp_param->input_crop.in_width = chain2_width;
	scp_param->input_crop.in_height = chain2_height;
	scp_param->input_crop.out_width = chain3_width;
	scp_param->input_crop.out_height = chain3_height;
	lindex |= LOWBIT_OF(PARAM_SCALERP_INPUT_CROP);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_INPUT_CROP);
	indexes++;

	/* sclaer can't apply stride to each plane, only y plane.
	cb, cr plane should be half of y plane, it's automatically set
	3 plane : all plane can be 32 stride or 16, 8
	2 plane : y plane only can be 32, 16 stride, other should be half of y
	1 plane : all plane can be 8 plane */
	if (video->frame.width_stride[0]) {
		scp_param->output_crop.cmd = SCALER_CROP_COMMAND_ENABLE;
		scp_param->output_crop.pos_x = 0;
		scp_param->output_crop.pos_y = 0;
		scp_param->output_crop.crop_width = chain3_width +
			video->frame.width_stride[0];
		scp_param->output_crop.crop_height = chain3_height;
		lindex |= LOWBIT_OF(PARAM_SCALERP_OUTPUT_CROP);
		hindex |= HIGHBIT_OF(PARAM_SCALERP_OUTPUT_CROP);
		indexes++;
	} else {
		scp_param->output_crop.cmd = SCALER_CROP_COMMAND_DISABLE;
		lindex |= LOWBIT_OF(PARAM_SCALERP_OUTPUT_CROP);
		hindex |= HIGHBIT_OF(PARAM_SCALERP_OUTPUT_CROP);
		indexes++;
	}

	scp_param->otf_output.width = chain3_width;
	scp_param->otf_output.height = chain3_height;
	lindex |= LOWBIT_OF(PARAM_SCALERP_OTF_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_OTF_OUTPUT);
	indexes++;

	scp_param->dma_output.width = chain3_width;
	scp_param->dma_output.height = chain3_height;
	lindex |= LOWBIT_OF(PARAM_SCALERP_DMA_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_DMA_OUTPUT);
	indexes++;

	/* FD */
	fd_param->otf_input.width = chain3_width;
	fd_param->otf_input.height = chain3_height;
	lindex |= LOWBIT_OF(PARAM_FD_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_FD_OTF_INPUT);
	indexes++;

	this->lindex |= lindex;
	this->hindex |= hindex;
	this->indexes += indexes;

	return ret;
}

static int fimc_is_ischain_s_chain1_stop(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	struct scalerc_param *scc_param;
	struct odc_param *odc_param;
	struct dis_param *dis_param;

	u32 indexes, lindex, hindex;

	scc_param = &this->is_region->parameter.scalerc;
	odc_param = &this->is_region->parameter.odc;
	dis_param = &this->is_region->parameter.dis;
	indexes = lindex = hindex = 0;

	/* SCC OUTPUT */
	scc_param->control.cmd = CONTROL_COMMAND_STOP;
	lindex |= LOWBIT_OF(PARAM_SCALERC_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_CONTROL);
	indexes++;

	/* ODC */
	odc_param->control.cmd = CONTROL_COMMAND_STOP;
	lindex |= LOWBIT_OF(PARAM_ODC_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_ODC_CONTROL);
	indexes++;

	/* DIS INPUT */
	dis_param->control.cmd = CONTROL_COMMAND_STOP;
	lindex |= LOWBIT_OF(PARAM_DIS_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_DIS_CONTROL);
	indexes++;

	this->lindex |= lindex;
	this->hindex |= hindex;
	this->indexes += indexes;

	return ret;
}

static int fimc_is_ischain_s_chain1_start(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	struct scalerc_param *scc_param;
	struct odc_param *odc_param;
	struct dis_param *dis_param;
	u32 indexes, lindex, hindex;

	scc_param = &this->is_region->parameter.scalerc;
	odc_param = &this->is_region->parameter.odc;
	dis_param = &this->is_region->parameter.dis;
	indexes = lindex = hindex = 0;

	/* SCC OUTPUT */
	scc_param->control.cmd = CONTROL_COMMAND_START;
	lindex |= LOWBIT_OF(PARAM_SCALERC_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_CONTROL);
	indexes++;

	/* ODC */
	odc_param->control.cmd = CONTROL_COMMAND_START;
	lindex |= LOWBIT_OF(PARAM_ODC_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_ODC_CONTROL);
	indexes++;

	/* DIS INPUT */
	dis_param->control.cmd = CONTROL_COMMAND_START;
	lindex |= LOWBIT_OF(PARAM_DIS_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_DIS_CONTROL);
	indexes++;

	this->lindex |= lindex;
	this->hindex |= hindex;
	this->indexes += indexes;

	return ret;
}

static int fimc_is_ischain_s_dzoom(struct fimc_is_device_ischain *this,
	u32 crop_x, u32 crop_y, u32 crop_width)
{
	int ret = 0;
	u32 indexes, lindex, hindex;
	u32 chain0_width, chain0_height;
	u32 temp_width, temp_height, input_width;
	u32 zoom_input, zoom_target;
	u32 crop_cx, crop_cy, crop_cwidth, crop_cheight;
	struct scalerc_param *scc_param;
#ifdef USE_ADVANCED_DZOOM
	u32 zoom_pre, zoom_post, zoom_pre_max;
	u32 crop_px, crop_py, crop_pwidth, crop_pheight;
	u32 chain1_width, chain1_height;
	u32 chain2_width, chain2_height;
	struct scalerp_param *scp_param;

	scc_param = &this->is_region->parameter.scalerc;
	scp_param = &this->is_region->parameter.scalerp;
	indexes = lindex = hindex = 0;
	chain0_width = this->chain0_width;
	chain0_height = this->chain0_height;
	chain1_width = this->chain1_width;
	chain1_height = this->chain1_height;
	chain2_width = this->chain2_width;
	chain2_height = this->chain2_height;
#ifdef PRINT_DZOOM
	printk(KERN_INFO "chain0(%d, %d), chain1(%d, %d), chain2(%d, %d)\n",
		chain0_width, chain0_height,
		chain1_width, chain1_height,
		chain2_width, chain2_height);
#endif
#else
	scc_param = &this->is_region->parameter.scalerc;
	indexes = lindex = hindex = 0;
	chain0_width = this->chain0_width;
	chain0_height = this->chain0_height;
#ifdef PRINT_DZOOM
	printk(KERN_INFO "chain0(%d, %d)\n", chain0_width, chain0_height);
#endif
#endif

	/* CHECK */
	input_width = crop_width;
	temp_width = crop_width + (crop_x<<1);
	if (temp_width != chain0_width) {
		err("input width is not valid(%d != %d)",
			temp_width, chain0_width);
		/* if invalid input come, dzoom is not apply and
		shot command is sent to firmware */
		ret = 0;
		goto exit;
	}

#ifdef USE_ADVANCED_DZOOM
	zoom_input = (chain0_width * 1000) / crop_width;
	zoom_pre_max = (chain0_width * 1000) / chain1_width;

	if (zoom_pre_max < 1000)
		zoom_pre_max = 1000;

#ifdef PRINT_DZOOM
	printk(KERN_INFO "zoom input : %d, premax-zoom : %d\n",
		zoom_input, zoom_pre_max);
#endif

	if (test_bit(FIMC_IS_ISDEV_DSTART, &this->dis.state))
		zoom_target = (zoom_input * 91 + 34000) / 125;
	else
		zoom_target = zoom_input;

	if (zoom_target > zoom_pre_max) {
		zoom_pre = zoom_pre_max;
		zoom_post = (zoom_target * 1000) / zoom_pre;
	} else {
		zoom_pre = zoom_target;
		zoom_post = 1000;
	}

	/* CALCULATION */
	temp_width = (chain0_width * 1000) / zoom_pre;
	temp_height = (chain0_height * 1000) / zoom_pre;
	crop_cx = (chain0_width - temp_width)>>1;
	crop_cy = (chain0_height - temp_height)>>1;
	crop_cwidth = chain0_width - (crop_cx<<1);
	crop_cheight = chain0_height - (crop_cy<<1);

	scc_param->input_crop.cmd = SCALER_CROP_COMMAND_ENABLE;
	scc_param->input_crop.pos_x = crop_cx;
	scc_param->input_crop.pos_y = crop_cy;
	scc_param->input_crop.crop_width = crop_cwidth;
	scc_param->input_crop.crop_height = crop_cheight;
	lindex |= LOWBIT_OF(PARAM_SCALERC_INPUT_CROP);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_INPUT_CROP);
	indexes++;

#ifdef PRINT_DZOOM
	printk(KERN_INFO "pre-zoom target : %d(%d, %d, %d %d)\n",
		zoom_pre, crop_cx, crop_cy, crop_cwidth, crop_cheight);
#endif

	temp_width = (chain2_width * 1000) / zoom_post;
	temp_height = (chain2_height * 1000) / zoom_post;
	crop_px = (chain2_width - temp_width)>>1;
	crop_py = (chain2_height - temp_height)>>1;
	crop_pwidth = chain2_width - (crop_px<<1);
	crop_pheight = chain2_height - (crop_py<<1);

	scp_param->input_crop.cmd = SCALER_CROP_COMMAND_ENABLE;
	scp_param->input_crop.pos_x = crop_px;
	scp_param->input_crop.pos_y = crop_py;
	scp_param->input_crop.crop_width = crop_pwidth;
	scp_param->input_crop.crop_height = crop_pheight;
	lindex |= LOWBIT_OF(PARAM_SCALERP_INPUT_CROP);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_INPUT_CROP);
	indexes++;

#ifdef PRINT_DZOOM
	printk(KERN_INFO "post-zoom target : %d(%d, %d, %d %d)\n",
		zoom_post, crop_px, crop_py, crop_pwidth, crop_pheight);
#endif
#else
	zoom_input = (chain0_width * 1000) / crop_width;

	if (test_bit(FIMC_IS_ISDEV_DSTART, &this->dis.state))
		zoom_target = (zoom_input * 91 + 34000) / 125;
	else
		zoom_target = zoom_input;

	temp_width = (chain0_width * 1000) / zoom_target;
	temp_height = (chain0_height * 1000) / zoom_target;
	crop_cx = (chain0_width - temp_width)>>1;
	crop_cy = (chain0_height - temp_height)>>1;
	crop_cwidth = chain0_width - (crop_cx<<1);
	crop_cheight = chain0_height - (crop_cy<<1);

	scc_param->input_crop.cmd = SCALER_CROP_COMMAND_ENABLE;
	scc_param->input_crop.pos_x = crop_cx;
	scc_param->input_crop.pos_y = crop_cy;
	scc_param->input_crop.crop_width = crop_cwidth;
	scc_param->input_crop.crop_height = crop_cheight;
	lindex |= LOWBIT_OF(PARAM_SCALERC_INPUT_CROP);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_INPUT_CROP);
	indexes++;

#ifdef PRINT_DZOOM
	printk(KERN_INFO "zoom input : %d, zoom target : %d(%d, %d, %d %d)\n",
		zoom_input, zoom_target,
		crop_cx, crop_cy, crop_cwidth, crop_cheight);
#endif
#endif

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (ret) {
		err("fimc_is_itf_s_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	this->crop_x = crop_cx;
	this->crop_y = crop_cy;
	this->crop_width = crop_cwidth;
	this->crop_height = crop_cheight;
	this->dzoom_width = input_width;

exit:
	return ret;
}

static int fimc_is_ischain_drc_bypass(struct fimc_is_device_ischain *this,
	bool bypass)
{
	int ret = 0;
	struct drc_param *drc_param;
	u32 indexes, lindex, hindex;

	dbg_ischain("%s\n", __func__);

	drc_param = &this->is_region->parameter.drc;
	indexes = lindex = hindex = 0;

	if (bypass)
		drc_param->control.bypass = CONTROL_BYPASS_ENABLE;
	else
		drc_param->control.bypass = CONTROL_BYPASS_DISABLE;

	lindex |= LOWBIT_OF(PARAM_DRC_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_DRC_CONTROL);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (ret) {
		err("fimc_is_itf_s_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	if (bypass) {
		clear_bit(FIMC_IS_ISDEV_DSTART, &this->drc.state);
		dbg_ischain("DRC off\n");
	} else {
		set_bit(FIMC_IS_ISDEV_DSTART, &this->drc.state);
		dbg_ischain("DRC on\n");
	}

exit:
	return ret;
}

static int fimc_is_ischain_dis_bypass(struct fimc_is_device_ischain *this,
	bool bypass)
{
	int ret = 0;
	bool process = true;
	u32 chain1_width, chain1_height;
	struct dis_param *dis_param;

	dbg_ischain("%s(%d)\n", __func__, bypass);

	dis_param = &this->is_region->parameter.dis;

	ret = fimc_is_itf_process_off(this);
	if (ret) {
		err("fimc_is_itf_process_off is fail\n");
		ret = -EINVAL;
		goto exit;
	}
	process = false;

	if (bypass) {
		chain1_width = this->chain2_width;
		chain1_height = this->chain2_height;
	} else {
		chain1_width = ALIGN(this->chain1_width*125/100, 4);
		chain1_height = ALIGN(this->chain1_height*125/100, 2);
	}

	this->lindex = this->hindex = this->indexes = 0;
	fimc_is_ischain_s_chain1_size(this, chain1_width, chain1_height);

	if (bypass)
		dis_param->control.bypass = CONTROL_BYPASS_ENABLE;
	else {
		dis_param->control.bypass = CONTROL_BYPASS_DISABLE;
		dis_param->control.buffer_number =
			SIZE_DIS_INTERNAL_BUF * NUM_DIS_INTERNAL_BUF;
		dis_param->control.buffer_address =
			this->minfo.dvaddr_shared + 300 * sizeof(u32);
		this->is_region->shared[300] = this->minfo.dvaddr_dis;
	}

	this->lindex |= LOWBIT_OF(PARAM_DIS_CONTROL);
	this->hindex |= HIGHBIT_OF(PARAM_DIS_CONTROL);
	this->indexes++;

	ret = fimc_is_itf_s_param(this,
		this->indexes, this->lindex, this->hindex);
	if (ret) {
		err("fimc_is_itf_s_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_itf_a_param(this);
	if (ret) {
		err("fimc_is_itf_a_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_itf_process_on(this);
	if (ret) {
		err("fimc_is_itf_process_on is fail1\n");
		ret = -EINVAL;
		goto exit;
	}
	process = true;

	if (bypass) {
		clear_bit(FIMC_IS_ISDEV_DSTART, &this->dis.state);
		dbg_ischain("DIS off\n");
	} else {
		set_bit(FIMC_IS_ISDEV_DSTART, &this->dis.state);
		dbg_ischain("DIS on\n");
	}

	this->chain1_width = chain1_width;
	this->chain1_height = chain1_height;

exit:
	if (!process)
		if (fimc_is_itf_process_on(this))
			err("fimc_is_itf_process_on fail2\n");

	return ret;
}

static int fimc_is_ischain_dnr_bypass(struct fimc_is_device_ischain *this,
	bool bypass)
{
	int ret = 0;
	struct tdnr_param *dnr_param;
	u32 indexes, lindex, hindex;

	dbg_ischain("%s\n", __func__);

	dnr_param = &this->is_region->parameter.tdnr;
	indexes = lindex = hindex = 0;

	if (bypass)
		dnr_param->control.bypass = CONTROL_BYPASS_ENABLE;
	else {
		dnr_param->control.bypass = CONTROL_BYPASS_DISABLE;
		dnr_param->control.buffer_number =
			SIZE_DNR_INTERNAL_BUF * NUM_DNR_INTERNAL_BUF;
		dnr_param->control.buffer_address =
			this->minfo.dvaddr_shared + 350 * sizeof(u32);
		this->is_region->shared[350] = this->minfo.dvaddr_3dnr;
	}

	lindex |= LOWBIT_OF(PARAM_TDNR_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_TDNR_CONTROL);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (ret) {
		err("fimc_is_itf_s_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	if (bypass) {
		clear_bit(FIMC_IS_ISDEV_DSTART, &this->dnr.state);
		dbg_ischain("TDNR off\n");
	} else {
		set_bit(FIMC_IS_ISDEV_DSTART, &this->dnr.state);
		dbg_ischain("TNDR on\n");
	}

exit:
	return ret;
}

static int fimc_is_ischain_fd_bypass(struct fimc_is_device_ischain *this,
	bool bypass)
{
	int ret = 0;
	bool process = true;
	struct fd_param *fd_param;
	u32 indexes, lindex, hindex;

	dbg_ischain("%s(%d)\n", __func__, bypass);

	fd_param = &this->is_region->parameter.fd;
	indexes = lindex = hindex = 0;

	ret = fimc_is_itf_process_off(this);
	if (ret) {
		err("fimc_is_itf_process_off is fail\n");
		ret = -EINVAL;
		goto exit;
	}
	process = false;

	if (bypass) {
		fd_param->control.cmd = CONTROL_COMMAND_STOP;
		fd_param->control.bypass = CONTROL_BYPASS_DISABLE;
	} else {
		fd_param->control.cmd = CONTROL_COMMAND_START;
		fd_param->control.bypass = CONTROL_BYPASS_DISABLE;
	}

	lindex |= LOWBIT_OF(PARAM_FD_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_FD_CONTROL);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (ret) {
		err("fimc_is_itf_s_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_itf_process_on(this);
	if (ret) {
		err("fimc_is_itf_process_on is fail1\n");
		ret = -EINVAL;
		goto exit;
	}
	process = true;

	if (bypass) {
		clear_bit(FIMC_IS_ISDEV_DSTART, &this->fd.state);
		dbg_ischain("FD off\n");
	} else {
		set_bit(FIMC_IS_ISDEV_DSTART, &this->fd.state);
		dbg_ischain("FD on\n");
	}

exit:
	if (!process)
		if (fimc_is_itf_process_on(this))
			err("fimc_is_itf_process_on fail2\n");

	return ret;
}

int fimc_is_ischain_isp_start(struct fimc_is_device_ischain *this,
	struct fimc_is_video_common *video)
{
	int ret = 0;
	struct isp_param *isp_param;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_ischain_dev *isp;
	u32 crop_x, crop_y, crop_width, crop_height;
	u32 sensor_width, sensor_height, sensor_ratio;
	u32 chain3_width, chain3_height, chain3_ratio;
	u32 chain1_wmin, chain1_hmin;
	u32 lindex, hindex, indexes;

	dbg_isp("%s()\n", __func__);

	indexes = lindex = hindex = 0;

	isp = &this->isp;
	framemgr = this->framemgr;
	isp_param = &this->is_region->parameter.isp;

	if (test_bit(FIMC_IS_ISDEV_DSTART, &isp->state)) {
		err("already start");
		ret = -EINVAL;
		goto exit;
	}

	/* 1. crop calculation */
	sensor_width = this->sensor_width;
	sensor_height = this->sensor_height;
	chain3_width = this->chain3_width;
	chain3_height = this->chain3_height;
	crop_width = sensor_width;
	crop_height = sensor_height;
	crop_x = crop_y = 0;

	sensor_ratio = sensor_width * 1000 / sensor_height;
	chain3_ratio = chain3_width * 1000 / chain3_height;

	if (sensor_ratio == chain3_ratio) {
		crop_width = sensor_width;
		crop_height = sensor_height;
	} else if (sensor_ratio < chain3_ratio) {
		/* isp dma input limitation
			height : 2 times */
		crop_height =
			(sensor_width * chain3_height) / chain3_width;
		crop_height = ALIGN(crop_height, 2);
		crop_y = ((sensor_height - crop_height) >> 1) & 0xFFFFFFFE;
	} else {
		/* isp dma input limitation
			width : 4 times */
		crop_width =
			(sensor_height * chain3_width) / chain3_height;
		crop_width = ALIGN(crop_width, 4);
		crop_x =  ((sensor_width - crop_width) >> 1) & 0xFFFFFFFE;
	}

	this->dzoom_width = crop_width;
	this->chain0_width = crop_width;
	this->chain0_height = crop_height;
	this->crop_width = crop_width;
	this->crop_height = crop_height;
	this->crop_x = crop_x;
	this->crop_y = crop_y;

	dbg_isp("crop_x : %d, crop y : %d\n", crop_x, crop_y);
	dbg_isp("crop width : %d, crop height : %d\n",
		crop_width, crop_height);

	/* 2. scaling calculation */
	chain1_wmin = (crop_width >> 4) & 0xFFFFFFFE;
	chain1_hmin = (crop_height >> 4) & 0xFFFFFFFE;

	if (chain1_wmin > this->chain1_width) {
		printk(KERN_INFO "scc down scale limited : (%d,%d)->(%d,%d)\n",
			this->chain1_width, this->chain1_height,
			chain1_wmin, chain1_hmin);
		this->chain1_width = chain1_wmin;
		this->chain1_height = chain1_hmin;
		this->chain2_width = chain1_wmin;
		this->chain2_height = chain1_hmin;
	}

	fimc_is_ischain_s_chain0_size(this,
		this->chain0_width, this->chain0_height);

	fimc_is_ischain_s_chain1_size(this,
		this->chain1_width, this->chain1_height);

	fimc_is_ischain_s_chain2_size(this,
		this->chain2_width, this->chain2_height);

	fimc_is_ischain_s_chain3_size(this,
		this->chain3_width, this->chain3_height);

	isp_param->control.cmd = CONTROL_COMMAND_START;
	isp_param->control.bypass = CONTROL_BYPASS_DISABLE;
	isp_param->control.run_mode = 1;
	lindex |= LOWBIT_OF(PARAM_ISP_CONTROL);
	hindex |= HIGHBIT_OF(PARAM_ISP_CONTROL);
	indexes++;

	isp_param->otf_input.cmd = OTF_INPUT_COMMAND_DISABLE;
	isp_param->otf_input.format = OTF_INPUT_FORMAT_BAYER_DMA;
	isp_param->otf_input.bitwidth = OTF_INPUT_BIT_WIDTH_10BIT;
	isp_param->otf_input.order = OTF_INPUT_ORDER_BAYER_GR_BG;
	isp_param->otf_input.frametime_max = 33333;
	lindex |= LOWBIT_OF(PARAM_ISP_OTF_INPUT);
	hindex |= HIGHBIT_OF(PARAM_ISP_OTF_INPUT);
	indexes++;

	isp_param->dma1_input.cmd = DMA_INPUT_COMMAND_BUF_MNGR;
	isp_param->dma1_input.width = sensor_width;
	isp_param->dma1_input.height = sensor_height;
	isp_param->dma1_input.uiDmaCropOffsetX = crop_x;
	isp_param->dma1_input.uiDmaCropOffsetY = crop_y;
	isp_param->dma1_input.uiDmaCropWidth = crop_width;
	isp_param->dma1_input.uiDmaCropHeight = crop_height;
	isp_param->dma1_input.uiBayerCropOffsetX = 0;
	isp_param->dma1_input.uiBayerCropOffsetY = 0;
	isp_param->dma1_input.uiBayerCropWidth = 0;
	isp_param->dma1_input.uiBayerCropHeight = 0;
	isp_param->dma1_input.uiUserMinFrameTime = 0;
	isp_param->dma1_input.uiUserMaxFrameTime = 66666;
	isp_param->dma1_input.uiWideFrameGap = 1;
	isp_param->dma1_input.uiFrameGap = 4096;
	isp_param->dma1_input.uiLineGap = 45;
	isp_param->dma1_input.bitwidth = DMA_INPUT_BIT_WIDTH_10BIT;
	isp_param->dma1_input.order = DMA_INPUT_ORDER_GR_BG;
	isp_param->dma1_input.plane = 1;
	isp_param->dma1_input.buffer_number = 1;
	isp_param->dma1_input.buffer_address = 0;
	/* hidden spec
	       [0] : sensor size is dma input size
	       [X] : sneosr size is reserved field */
	isp_param->dma1_input.uiReserved[1] = 0;
	isp_param->dma1_input.uiReserved[2] = 0;
	lindex |= LOWBIT_OF(PARAM_ISP_DMA1_INPUT);
	hindex |= HIGHBIT_OF(PARAM_ISP_DMA1_INPUT);
	indexes++;

	lindex = 0xFFFFFFFF;
	hindex = 0xFFFFFFFF;

	ret = fimc_is_itf_s_param(this , indexes, lindex, hindex);
	if (ret) {
		err("fimc_is_itf_s_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_itf_f_param(this);
	if (ret) {
		err("fimc_is_itf_f_param is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_itf_g_capability(this);
	if (ret) {
		err("fimc_is_itf_g_capability is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_itf_process_on(this);
	if (ret) {
		err("fimc_is_itf_process_on is fail\n");
		ret = -EINVAL;
		goto exit;
	}

	set_bit(FIMC_IS_ISDEV_DSTART, &isp->state);

exit:
	return ret;
}

int fimc_is_ischain_isp_stop(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	int retry;
	struct fimc_is_ischain_dev *isp;
	struct fimc_is_interface *itf;
	struct fimc_is_framemgr *framemgr;

	dbg_isp("%s()\n", __func__);

	isp = &this->isp;
	itf = this->interface;
	framemgr = this->framemgr;

	if (!test_bit(FIMC_IS_ISDEV_DSTART, &isp->state)) {
		warn("already stop");
		goto exit;
	}

	/*
	video node is stream off state if this function is called.
	that is, a request can NOT enter buffer queue.
	mutex can be locked for entering ischain_callback function,
	in this case the below code should be blocked until mutex is released.
	*/
	retry = 10;
	while (retry) {
		mutex_lock(&this->mutex_state);
		if (!test_bit(FIMC_IS_ISCHAIN_RUN, &this->state)) {
			mutex_unlock(&this->mutex_state);
			break;
		}
		mutex_unlock(&this->mutex_state);
		retry--;
	}

	if (!retry)
		err("waiting complete is fail0");

	retry = 10;
	while (framemgr->frame_request_cnt && retry) {
		printk(KERN_INFO "%d frame reqs waiting...\n",
			framemgr->frame_request_cnt);
		msleep(20);
		retry--;
	}

	if (!retry)
		err("waiting complete is fail1");

	retry = 10;
	while (framemgr->frame_process_cnt && retry) {
		printk(KERN_INFO "%d frame pros waiting...\n",
			framemgr->frame_process_cnt);
		msleep(20);
		retry--;
	}

	if (!retry)
		err("waiting complete is fail2");

	ret = fimc_is_itf_process_off(this);
	if (ret) {
		err("fimc_is_itf_process_off is fail");
		ret = -EINVAL;
		goto exit;
	}

	clear_bit(FIMC_IS_ISDEV_DSTART, &isp->state);

#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME
	measure_init();
#endif
#endif

exit:
	return ret;
}

int fimc_is_ischain_isp_s_format(struct fimc_is_device_ischain *this,
		u32 width, u32 height)
{
	int ret = 0;

	this->sensor_width = width - this->margin_width;
	this->sensor_height = height - this->margin_height;

	return ret;
}

int fimc_is_ischain_isp_buffer_queue(struct fimc_is_device_ischain *this,
	u32 index)
{
	int ret = 0;
	unsigned long flags;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_video_common *scc_video, *scp_video;

#ifdef DBG_STREAMING
	/*printk(KERN_INFO "%s\n", __func__);*/
#endif

	if (index >= FRAMEMGR_MAX_REQUEST) {
		err("index(%d) is invalid", index);
		ret = -EINVAL;
		goto exit;
	}

	framemgr = this->framemgr;
	if (framemgr == NULL) {
		err("framemgr is null\n");
		ret = EINVAL;
		goto exit;
	}

	frame = &framemgr->frame[index];
	if (frame == NULL) {
		err("frame is null\n");
		ret = EINVAL;
		goto exit;
	}

#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME
	do_gettimeofday(&frame->time_queued);
#endif
#endif

	if (frame->init == FRAME_UNI_MEM) {
		err("frame %d is NOT init", index);
		ret = EINVAL;
		goto exit;
	}

	scc_video = this->scc.video;
	scp_video = this->scp.video;

	framemgr_e_barrier_irqs(framemgr, index, flags);

	if (frame->state == FIMC_IS_FRAME_STATE_FREE) {
		if (frame->req_flag) {
			dbg_warning("%d request flag is not clear(%08X)\n",
				frame->index, (u32)frame->req_flag);
			frame->req_flag = 0;
		}

		if (frame->scc_out == FIMC_IS_FOUT_REQ)
			err("scc output is not generated");

		if (frame->scp_out == FIMC_IS_FOUT_REQ)
			err("scp output is not generated");

		frame->fcount = frame->shot->dm.request.frameCount;

		if (frame->shot_ext->request_scp &&
			!test_bit(FIMC_IS_VIDEO_STREAM_ON, &scp_video->state)) {
			frame->shot_ext->request_scp = 0;
			err("scp %d frame is drop2", frame->fcount);
		}

		if (frame->shot_ext->request_scc &&
			!test_bit(FIMC_IS_VIDEO_STREAM_ON, &scc_video->state)) {
			frame->shot_ext->request_scc = 0;
			err("scc %d frame is drop2", frame->fcount);
		}

		fimc_is_frame_trans_fre_to_req(framemgr, frame);
	} else {
		err("frame(%d) is not free state(%d)\n", index, frame->state);
		fimc_is_frame_print_all(framemgr);
	}

	framemgr_x_barrier_irqr(framemgr, index, flags);

	mutex_lock(&this->mutex_state);
	if (!test_bit(FIMC_IS_ISCHAIN_RUN, &this->state))
		fimc_is_ischain_callback(this);
	mutex_unlock(&this->mutex_state);

exit:
	return ret;
}

int fimc_is_ischain_isp_buffer_finish(struct fimc_is_device_ischain *this,
	u32 index)
{
	int ret = 0;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_framemgr *framemgr = this->framemgr;

#ifdef DBG_STREAMING
	/*dbg_ischain("%s\n", __func__);*/
#endif

	framemgr_e_barrier_irq(framemgr, index+0xf0);

	fimc_is_frame_complete_head(framemgr, &frame);
	if (frame) {
		if (index == frame->index)
			fimc_is_frame_trans_com_to_fre(framemgr, frame);
		else {
			dbg_warning("buffer index is NOT matched(%d != %d)\n",
				index, frame->index);
			fimc_is_frame_print_all(framemgr);
		}
	} else {
		err("frame is empty from complete");
		fimc_is_frame_print_all(framemgr);
	}

	framemgr_x_barrier_irq(framemgr, index+0xf0);

#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME
	do_gettimeofday(&frame->time_dequeued);
	measure_internal_time(&frame->time_queued, &frame->time_shot,
		&frame->time_shotdone, &frame->time_dequeued);
#endif
#endif

	return ret;
}

int fimc_is_ischain_scc_start(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	u32 planes, i, j, buf_index;
	u32 indexes, lindex, hindex;
	struct scalerc_param *scc_param;
	struct fimc_is_ischain_dev *scc;
	struct fimc_is_video_common *video;

	dbg_ischain("%s\n", __func__);

	scc = &this->scc;
	video = scc->video;

	planes = video->frame.format.num_planes;
	for (i = 0; i < video->buffers; i++) {
		for (j = 0; j < planes; j++) {
			buf_index = i*planes + j;

			/*dbg_ischain("(%d)set buf(%d:%d) = 0x%08x\n",
				buf_index, i, j, video->buf_dva[i][j]);*/

			this->is_region->shared[447+buf_index] =
				video->buf_dva[i][j];
		}
	}

	dbg_ischain("buf_num:%d buf_plane:%d shared[447] : 0x%X\n",
		video->buffers,
		video->frame.format.num_planes,
		this->minfo.kvaddr_shared + 447 * sizeof(u32));

	video->buf_mask = 0;
	for (i = 0; i < video->buffers; i++)
		video->buf_mask |= (1 << i);

	indexes = 0;
	lindex = hindex = 0;

	scc_param = &this->is_region->parameter.scalerc;
	scc_param->dma_output.cmd = DMA_OUTPUT_COMMAND_ENABLE;
	scc_param->dma_output.dma_out_mask = video->buf_mask;
	scc_param->dma_output.buffer_number = video->buffers;
#ifdef USE_FRAME_SYNC
	scc_param->dma_output.plane = video->frame.format.num_planes - 1;
#else
	scc_param->dma_output.plane = video->frame.format.num_planes;
#endif
	scc_param->dma_output.buffer_address =
		this->minfo.dvaddr_shared + 447*sizeof(u32);
	lindex |= LOWBIT_OF(PARAM_SCALERC_DMA_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_DMA_OUTPUT);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (!ret)
		set_bit(FIMC_IS_ISDEV_DSTART, &scc->state);
	else
		err("fimc_is_itf_s_param is fail\n");

	if (!this->interface->streaming)
		fimc_is_itf_a_param(this);

	return ret;
}

int fimc_is_ischain_scc_stop(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	u32 indexes, lindex, hindex;
	struct scalerc_param *scc_param;
	struct fimc_is_ischain_dev *scc;

	dbg_ischain("%s\n", __func__);

	indexes = 0;
	lindex = hindex = 0;
	scc = &this->scc;

	scc_param = &this->is_region->parameter.scalerc;
	scc_param->dma_output.cmd = DMA_OUTPUT_COMMAND_DISABLE;

	lindex |= LOWBIT_OF(PARAM_SCALERC_DMA_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERC_DMA_OUTPUT);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (!ret)
		clear_bit(FIMC_IS_ISDEV_DSTART, &scc->state);
	else
		err("fimc_is_itf_s_param is fail\n");

	if (!this->interface->streaming)
		fimc_is_itf_a_param(this);

	return ret;
}

int fimc_is_ischain_scp_start(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	u32 planes, i, j, buf_index;
	u32 indexes, lindex, hindex;
	struct scalerp_param *scp_param;
	struct fimc_is_ischain_dev *scp;
	struct fimc_is_video_common *video;

	dbg_ischain("%s\n", __func__);

	scp = &this->scp;
	video = scp->video;

	planes = video->frame.format.num_planes;
	for (i = 0; i < video->buffers; i++) {
		for (j = 0; j < planes; j++) {
			buf_index = i*planes + j;

			/*dbg_ischain("(%d)set buf(%d:%d) = 0x%08x\n",
				buf_index, i, j, video->buf_dva[i][j]);*/

			this->is_region->shared[400+buf_index] =
				video->buf_dva[i][j];
		}
	}

	dbg_ischain("buf_num:%d buf_plane:%d shared[400] : 0x%X\n",
		video->buffers,
		video->frame.format.num_planes,
		this->minfo.kvaddr_shared + 400 * sizeof(u32));

	video->buf_mask = 0;
	for (i = 0; i < video->buffers; i++)
		video->buf_mask |= (1 << i);

	indexes = 0;
	lindex = hindex = 0;

	scp_param = &this->is_region->parameter.scalerp;
	scp_param->dma_output.cmd = DMA_OUTPUT_COMMAND_ENABLE;
	scp_param->dma_output.dma_out_mask = video->buf_mask;
	scp_param->dma_output.buffer_number = video->buffers;
#ifdef USE_FRAME_SYNC
	scp_param->dma_output.plane = video->frame.format.num_planes - 1;
#else
	scp_param->dma_output.plane = video->frame.format.num_planes;
#endif
	scp_param->dma_output.buffer_address =
		this->minfo.dvaddr_shared + 400*sizeof(u32);

	lindex |= LOWBIT_OF(PARAM_SCALERP_DMA_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_DMA_OUTPUT);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (!ret)
		set_bit(FIMC_IS_ISDEV_DSTART, &scp->state);
	else
		err("fimc_is_itf_s_param is fail\n");

	if (!this->interface->streaming)
		fimc_is_itf_a_param(this);

	return ret;
}

int fimc_is_ischain_scp_stop(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	u32 indexes, lindex, hindex;
	struct scalerp_param *scp_param;
	struct fimc_is_ischain_dev *scp;

	dbg_ischain("%s\n", __func__);

	indexes = 0;
	lindex = hindex = 0;
	scp = &this->scp;

	scp_param = &this->is_region->parameter.scalerp;
	scp_param->dma_output.cmd = DMA_OUTPUT_COMMAND_DISABLE;

	lindex |= LOWBIT_OF(PARAM_SCALERP_DMA_OUTPUT);
	hindex |= HIGHBIT_OF(PARAM_SCALERP_DMA_OUTPUT);
	indexes++;

	ret = fimc_is_itf_s_param(this, indexes, lindex, hindex);
	if (!ret)
		clear_bit(FIMC_IS_ISDEV_DSTART, &scp->state);
	else
		err("fimc_is_itf_s_param is fail\n");

	if (!this->interface->streaming)
		fimc_is_itf_a_param(this);

	return ret;
}

int fimc_is_ischain_scp_s_format(struct fimc_is_device_ischain *this,
	u32 width, u32 height)
{
	int ret = 0;

	this->chain1_width = width;
	this->chain1_height = height;
	this->chain2_width = width;
	this->chain2_height = height;
	this->chain3_width = width;
	this->chain3_height = height;

	return ret;
}

int fimc_is_ischain_dev_open(struct fimc_is_ischain_dev *this,
	enum is_entry entry,
	struct fimc_is_video_common *video,
	const struct param_control *init_ctl)
{
	int ret = 0;

	mutex_init(&this->mutex_state);
	this->entry = entry;
	this->video = video;

	if (init_ctl) {
		if (init_ctl->cmd != CONTROL_COMMAND_START) {
			err("%d entry is not start", entry);
			ret = -EINVAL;
			goto exit;
		}

		if (init_ctl->bypass == CONTROL_BYPASS_ENABLE)
			clear_bit(FIMC_IS_ISDEV_DSTART, &this->state);
		else if (init_ctl->bypass == CONTROL_BYPASS_DISABLE)
			set_bit(FIMC_IS_ISDEV_DSTART, &this->state);
		else {
			err("%d entry has invalid bypass value(%d)",
				entry, init_ctl->bypass);
			ret = -EINVAL;
			goto exit;
		}
	} else /* isp, scc, scp do not use bypass(memory interface)*/
		clear_bit(FIMC_IS_ISDEV_DSTART, &this->state);

exit:
	return ret;
}

int fimc_is_ischain_dev_start(struct fimc_is_ischain_dev *this)
{
	int ret = 0;
	struct fimc_is_framemgr *framemgr;

	framemgr = &this->framemgr;

	ret = framemgr->frame_request_cnt;
	if (!ret) {
		err("buffer queued is empty, can't start(%d)", ret);
		ret = -EINVAL;
		goto exit;
	} else
		ret = 0;

exit:
	return ret;
}

int fimc_is_ischain_dev_stop(struct fimc_is_ischain_dev *this)
{
	int ret = 0;
	unsigned long flags;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_framemgr *framemgr;

	framemgr = &this->framemgr;

	framemgr_e_barrier_irqs(framemgr, FMGR_IDX_4, flags);

	fimc_is_frame_complete_head(framemgr, &frame);
	while (frame) {
		fimc_is_frame_trans_com_to_fre(framemgr, frame);
		fimc_is_frame_complete_head(framemgr, &frame);
	}

	fimc_is_frame_process_head(framemgr, &frame);
	while (frame) {
		fimc_is_frame_trans_pro_to_fre(framemgr, frame);
		fimc_is_frame_process_head(framemgr, &frame);
	}

	fimc_is_frame_request_head(framemgr, &frame);
	while (frame) {
		fimc_is_frame_trans_req_to_fre(framemgr, frame);
		fimc_is_frame_request_head(framemgr, &frame);
	}

	framemgr_x_barrier_irqr(framemgr, FMGR_IDX_4, flags);

	return ret;
}

int fimc_is_ischain_dev_buffer_queue(struct fimc_is_ischain_dev *this,
	u32 index)
{
	int ret = 0;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_framemgr *framemgr;

#ifdef DBG_STREAMING
	dbg_ischain("%s\n", __func__);
#endif

	if (index >= FRAMEMGR_MAX_REQUEST) {
		err("index(%d) is invalid", index);
		ret = -EINVAL;
		goto exit;
	}

	framemgr = &this->framemgr;
	if (framemgr == NULL) {
		err("framemgr is null\n");
		ret = EINVAL;
		goto exit;
	}

	frame = &framemgr->frame[index];
	if (frame == NULL) {
		err("frame is null\n");
		ret = EINVAL;
		goto exit;
	}

	if (frame->init == FRAME_UNI_MEM) {
		err("frame %d is NOT init", index);
		ret = EINVAL;
		goto exit;
	}

	framemgr_e_barrier_irq(framemgr, index);

	if (frame->state == FIMC_IS_FRAME_STATE_FREE) {
		if (frame->req_flag) {
			dbg_warning("%d request flag is not clear(%08X)\n",
				frame->index, (u32)frame->req_flag);
			frame->req_flag = 0;
		}

		fimc_is_frame_trans_fre_to_req(framemgr, frame);
	} else {
		err("frame(%d) is not free state(%d)\n", index, frame->state);
		fimc_is_frame_print_all(framemgr);
	}

	framemgr_x_barrier_irq(framemgr, index);

exit:
	return ret;
}

int fimc_is_ischain_dev_buffer_finish(struct fimc_is_ischain_dev *this,
	u32 index)
{
	int ret = 0;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_framemgr *framemgr;

#ifdef DBG_STREAMING
	/*dbg_ischain("%s\n", __func__);*/
#endif

	framemgr = &this->framemgr;

	framemgr_e_barrier_irq(framemgr, index);

	fimc_is_frame_complete_head(framemgr, &frame);
	if (frame) {
		if (index == frame->index)
			fimc_is_frame_trans_com_to_fre(framemgr, frame);
		else {
			dbg_warning("buffer index is NOT matched(%d != %d)\n",
				index, frame->index);
			fimc_is_frame_print_all(framemgr);
		}
	} else {
		err("frame is empty from complete");
		fimc_is_frame_print_all(framemgr);
	}

	framemgr_x_barrier_irq(framemgr, index);

	return ret;
}

int fimc_is_ischain_g_capability(struct fimc_is_device_ischain *this,
	u32 user_ptr)
{
	int ret = 0;

	ret = copy_to_user((void *)user_ptr, &this->capability,
		sizeof(struct camera2_sm));

	return ret;
}

int fimc_is_ischain_print_status(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_interface *itf;

	framemgr = this->framemgr;
	itf = this->interface;

	fimc_is_frame_print_free_list(framemgr);
	fimc_is_frame_print_request_list(framemgr);
	fimc_is_frame_print_process_list(framemgr);
	fimc_is_frame_print_complete_list(framemgr);
	print_fre_work_list(&itf->work_list[INTR_META_DONE]);
	print_req_work_list(&itf->work_list[INTR_META_DONE]);

	return ret;
}

int fimc_is_ischain_callback(struct fimc_is_device_ischain *this)
{
	int ret = 0;
	bool scc_req, scp_req;
	unsigned long flags;
	u32 crop_width;
	u32 setfile_save;
	struct fimc_is_framemgr *isp_framemgr, *scc_framemgr, *scp_framemgr;
	struct fimc_is_frame_shot *isp_frame, *scc_frame, *scp_frame;
	struct fimc_is_ischain_dev *scc, *scp;

#ifdef DBG_STREAMING
	dbg_ischain("%s\n", __func__);
#endif

	scc = &this->scc;
	scp = &this->scp;
	isp_framemgr = this->framemgr;
	scc_framemgr = &scc->framemgr;
	scp_framemgr = &scp->framemgr;
	scc_req = scp_req = false;

	/*
	   BE CAREFUL WITH THIS
	1. buffer queue, all compoenent stop, so it's good
	2. interface callback, all component will be stop until new one is came
	   therefore, i expect lock object is not necessary in here
	*/

	if (!isp_framemgr) {
		err("isp_framemgr is NULL");
		return -EINVAL;
	}

	if (!scc_framemgr) {
		err("scc_framemgr is NULL");
		return -EINVAL;
	}

	if (!scp_framemgr) {
		err("scp_framemgr is NULL");
		return -EINVAL;
	}

	fimc_is_frame_request_head(isp_framemgr, &isp_frame);
	if (!isp_frame) {
		clear_bit(FIMC_IS_ISCHAIN_RUN, &this->state);
#ifdef DBG_STREAMING
		dbg_warning("ischain is stopped\n");
#endif
		return 0;
	}

	if (!isp_frame->shot_ext) {
		err("shot_ext is NULL");
		return -EINVAL;
	}

	if (!isp_frame->shot) {
		err("shot is NULL");
		return -EINVAL;
	}

	if (isp_frame->init == FRAME_INI_MEM) {
		fimc_is_itf_cfg_mem(this, isp_frame->dvaddr_shot,
			isp_frame->shot_size);
		isp_frame->init = FRAME_CFG_MEM;
	}

	if (isp_frame->shot_ext->setfile != this->setfile) {
		setfile_save = this->setfile;
		this->setfile = isp_frame->shot_ext->setfile;

		ret = fimc_is_ischain_s_setfile(this);
		if (ret) {
			err("fimc_is_ischain_s_setfile is fail");
			this->setfile = setfile_save;
			goto exit;
		}
	}

	if (isp_frame->shot_ext->request_scc) {
		if (!test_bit(FIMC_IS_ISDEV_DSTART, &this->scc.state)) {
			ret = fimc_is_ischain_scc_start(this);
			if (ret) {
				err("fimc_is_ischain_scc_start is fail");
				goto exit;
			}
		}

		scc_req = true;
	} else {
		if (test_bit(FIMC_IS_ISDEV_DSTART, &this->scc.state)) {
			ret = fimc_is_ischain_scc_stop(this);
			if (ret) {
				err("fimc_is_ischain_scc_stop is fail");
				goto exit;
			}
		}
	}

	if (isp_frame->shot_ext->request_scp) {
		if (!test_bit(FIMC_IS_ISDEV_DSTART, &this->scp.state)) {
			ret = fimc_is_ischain_scp_start(this);
			if (ret) {
				err("fimc_is_ischain_scp_start is fail");
				goto exit;
			}
		}

		scp_req = true;
	} else {
		if (test_bit(FIMC_IS_ISDEV_DSTART, &this->scp.state)) {
			ret = fimc_is_ischain_scp_stop(this);
			if (ret) {
				err("fimc_is_ischain_scp_stop is fail");
				goto exit;
			}
		}
	}

#ifdef ENABLE_DRC
	if (isp_frame->shot_ext->drc_bypass) {
		if (test_bit(FIMC_IS_ISDEV_DSTART, &this->drc.state)) {
			ret = fimc_is_ischain_drc_bypass(this, true);
			if (ret) {
				err("fimc_is_ischain_drc_bypass(1) is fail");
				goto exit;
			}
		}
	} else {
		if (!test_bit(FIMC_IS_ISDEV_DSTART, &this->drc.state)) {
			ret = fimc_is_ischain_drc_bypass(this, false);
			if (ret) {
				err("fimc_is_ischain_drc_bypass(0) is fail");
				goto exit;
			}
		}
	}
#endif

#ifdef ENABLE_VDIS
	if (isp_frame->shot_ext->dis_bypass) {
		if (test_bit(FIMC_IS_ISDEV_DSTART, &this->dis.state)) {
			ret = fimc_is_ischain_dis_bypass(this, true);
			if (ret) {
				err("fimc_is_ischain_dis_bypass(1) is fail");
				goto exit;
			}
		}
	} else {
		if (!test_bit(FIMC_IS_ISDEV_DSTART, &this->dis.state)) {
			ret = fimc_is_ischain_dis_bypass(this, false);
			if (ret) {
				err("fimc_is_ischain_dis_bypass(0) is fail");
				goto exit;
			}
		}
	}
#endif

#ifdef ENABLE_TDNR
	if (isp_frame->shot_ext->dnr_bypass) {
		if (test_bit(FIMC_IS_ISDEV_DSTART, &this->dnr.state)) {
			ret = fimc_is_ischain_dnr_bypass(this, true);
			if (ret) {
				err("fimc_is_ischain_dnr_bypass(1) is fail");
				goto exit;
			}
		}
	} else {
		if (!test_bit(FIMC_IS_ISDEV_DSTART, &this->dnr.state)) {
			ret = fimc_is_ischain_dnr_bypass(this, false);
			if (ret) {
				err("fimc_is_ischain_dnr_bypass(0) is fail");
				goto exit;
			}
		}
	}
#endif

#ifdef ENABLE_FD
	if (isp_frame->shot_ext->fd_bypass) {
		if (test_bit(FIMC_IS_ISDEV_DSTART, &this->fd.state)) {
			ret = fimc_is_ischain_fd_bypass(this, true);
			if (ret) {
				err("fimc_is_ischain_fd_bypass(1) is fail");
				goto exit;
			}
		}
	} else {
		if (!test_bit(FIMC_IS_ISDEV_DSTART, &this->fd.state)) {
			ret = fimc_is_ischain_fd_bypass(this, false);
			if (ret) {
				err("fimc_is_ischain_fd_bypass(0) is fail");
				goto exit;
			}
		}
	}
#endif

	crop_width = isp_frame->shot->ctl.scaler.cropRegion[2];
	if (crop_width && (crop_width != this->dzoom_width)) {
		ret = fimc_is_ischain_s_dzoom(this,
			isp_frame->shot->ctl.scaler.cropRegion[0],
			isp_frame->shot->ctl.scaler.cropRegion[1],
			isp_frame->shot->ctl.scaler.cropRegion[2]);
		if (ret) {
			err("fimc_is_ischain_s_dzoom(%d, %d, %d) is fail",
				isp_frame->shot->ctl.scaler.cropRegion[0],
				isp_frame->shot->ctl.scaler.cropRegion[1],
				isp_frame->shot->ctl.scaler.cropRegion[2]);
			goto exit;
		}
	}

	if (scc_req) {
		framemgr_e_barrier_irqs(scc_framemgr, FMGR_IDX_8, flags);

		fimc_is_frame_request_head(scc_framemgr, &scc_frame);
		if (scc_frame) {
			isp_frame->shot->uctl.scalerUd.sccTargetAddress[0] =
				scc_frame->dvaddr_buffer[0];
			isp_frame->shot->uctl.scalerUd.sccTargetAddress[1] =
				scc_frame->dvaddr_buffer[1];
			isp_frame->shot->uctl.scalerUd.sccTargetAddress[2] =
				scc_frame->dvaddr_buffer[2];
			scc_frame->stream->findex = isp_frame->index;
			isp_frame->scc_out = FIMC_IS_FOUT_REQ;

			set_bit(REQ_FRAME, &scc_frame->req_flag);
			fimc_is_frame_trans_req_to_pro(scc_framemgr, scc_frame);
		} else {
			isp_frame->shot->uctl.scalerUd.sccTargetAddress[0] = 0;
			isp_frame->shot->uctl.scalerUd.sccTargetAddress[1] = 0;
			isp_frame->shot->uctl.scalerUd.sccTargetAddress[2] = 0;
			isp_frame->shot_ext->request_scc = 0;
			err("scc %d frame is drop", isp_frame->fcount);
		}

		framemgr_x_barrier_irqr(scc_framemgr, FMGR_IDX_8, flags);
	} else {
		isp_frame->shot->uctl.scalerUd.sccTargetAddress[0] = 0;
		isp_frame->shot->uctl.scalerUd.sccTargetAddress[1] = 0;
		isp_frame->shot->uctl.scalerUd.sccTargetAddress[2] = 0;
		isp_frame->shot_ext->request_scc = 0;
	}

	if (scp_req) {
		framemgr_e_barrier_irqs(scp_framemgr, FMGR_IDX_9, flags);

		fimc_is_frame_request_head(scp_framemgr, &scp_frame);
		if (scp_frame) {
			isp_frame->shot->uctl.scalerUd.scpTargetAddress[0] =
				scp_frame->dvaddr_buffer[0];
			isp_frame->shot->uctl.scalerUd.scpTargetAddress[1] =
				scp_frame->dvaddr_buffer[1];
			isp_frame->shot->uctl.scalerUd.scpTargetAddress[2] =
				scp_frame->dvaddr_buffer[2];
			scp_frame->stream->findex = isp_frame->index;
			isp_frame->scp_out = FIMC_IS_FOUT_REQ;

			set_bit(REQ_FRAME, &scp_frame->req_flag);
			fimc_is_frame_trans_req_to_pro(scp_framemgr, scp_frame);
		} else {
			isp_frame->shot->uctl.scalerUd.scpTargetAddress[0] = 0;
			isp_frame->shot->uctl.scalerUd.scpTargetAddress[1] = 0;
			isp_frame->shot->uctl.scalerUd.scpTargetAddress[2] = 0;
			isp_frame->shot_ext->request_scp = 0;
			err("scp %d frame is drop", isp_frame->fcount);
		}

		framemgr_x_barrier_irqr(scp_framemgr, FMGR_IDX_9, flags);
	} else {
		isp_frame->shot->uctl.scalerUd.scpTargetAddress[0] = 0;
		isp_frame->shot->uctl.scalerUd.scpTargetAddress[1] = 0;
		isp_frame->shot->uctl.scalerUd.scpTargetAddress[2] = 0;
		isp_frame->shot_ext->request_scp = 0;
	}

exit:
	if (ret) {
		clear_bit(FIMC_IS_ISCHAIN_RUN, &this->state);
		clear_bit(REQ_FRAME, &isp_frame->req_flag);
		isp_frame->shot_ext->request_isp = 0;
		isp_frame->shot_ext->request_scc = 0;
		isp_frame->shot_ext->request_scp = 0;
		fimc_is_frame_trans_req_to_com(isp_framemgr, isp_frame);

		buffer_done(this->isp.video, isp_frame->index);
		err("shot(index : %d) is skipped(error : %d)",
			isp_frame->index, ret);
	} else {
		set_bit(REQ_SHOT, &isp_frame->req_flag);
		set_bit(FIMC_IS_ISCHAIN_RUN, &this->state);
		fimc_is_frame_trans_req_to_pro(isp_framemgr, isp_frame);

		fimc_is_itf_shot(this, isp_frame);
	}

	return ret;
}

#if 0 /* will be used if host control sensor */
int fimc_is_ischain_camctl(struct fimc_is_device_ischain *this,
	struct fimc_is_frame_shot *frame,
	u32 fcount)
{
	int ret = 0;
	u32 updated = 0;
	struct fimc_is_interface *itf;
	struct camera2_uctl *applied_ctl;

	struct camera2_sensor_uctl *cur_sensor_uctl;
	struct camera2_sensor_uctl *isp_sensor_uctl;
	struct camera2_sensor_ctl *cur_sensor_ctl;
	struct camera2_sensor_ctl *isp_sensor_ctl;
	struct camera2_sensor_ctl *req_sensor_ctl;

	struct camera2_lens_uctl *cur_lens_uctl;
	struct camera2_lens_uctl *isp_lens_uctl;
	struct camera2_lens_ctl *cur_lens_ctl;
	struct camera2_lens_ctl *isp_lens_ctl;
	struct camera2_lens_ctl *req_lens_ctl;

	struct camera2_flash_uctl *cur_flash_uctl;
	struct camera2_flash_uctl *isp_flash_uctl;
	struct camera2_flash_ctl *cur_flash_ctl;
	struct camera2_flash_ctl *isp_flash_ctl;
	struct camera2_flash_ctl *req_flash_ctl;

	u32 index;
	u64 frameDuration;
	u64 exposureTime;
	u32 sensitivity;
	u32 focusDistance;
	u32 flashMode;

#ifdef DBG_STREAMING
	dbg_ischain("%s\n", __func__);
#endif

	itf = this->interface;
	cur_sensor_uctl = &this->cur_peri_ctl.sensorUd;
	cur_sensor_ctl = &cur_sensor_uctl->ctl;
	isp_sensor_uctl = &itf->isp_peri_ctl.sensorUd;
	isp_sensor_ctl = &isp_sensor_uctl->ctl;

	cur_lens_uctl = &this->cur_peri_ctl.lensUd;
	cur_lens_ctl = &cur_lens_uctl->ctl;
	isp_lens_uctl = &itf->isp_peri_ctl.lensUd;
	isp_lens_ctl = &isp_lens_uctl->ctl;

	cur_flash_uctl = &this->cur_peri_ctl.flashUd;
	cur_flash_ctl = &cur_flash_uctl->ctl;
	isp_flash_uctl = &itf->isp_peri_ctl.flashUd;
	isp_flash_ctl = &isp_flash_uctl->ctl;

	this->cur_peri_ctl.uUpdateBitMap = 0;

	if (frame) {
		req_sensor_ctl = &frame->shot->ctl.sensor;
		req_lens_ctl = &frame->shot->ctl.lens;
		req_flash_ctl = &frame->shot->ctl.flash;
	} else {
		req_sensor_ctl = NULL;
		req_lens_ctl = NULL;
		req_flash_ctl = NULL;
	}

	/*sensor part*/
	if (req_sensor_ctl && req_sensor_ctl->frameDuration)
		frameDuration = req_sensor_ctl->frameDuration;
	else
		frameDuration = isp_sensor_ctl->frameDuration;

	if (cur_sensor_ctl->frameDuration != frameDuration) {
		cur_sensor_ctl->frameDuration = frameDuration;
		/*dbg_ischain("fduration : %d\n", (u32)duration);*/
		updated |= CAM_SENSOR_CMD;
	}

	if (req_sensor_ctl && req_sensor_ctl->exposureTime)
		exposureTime = req_sensor_ctl->exposureTime;
	else
		exposureTime = isp_sensor_ctl->exposureTime;

	if (cur_sensor_ctl->exposureTime != exposureTime) {
		cur_sensor_ctl->exposureTime = exposureTime;
		/*dbg_ischain("exposure : %d\n", (u32)exposure);*/
		updated |= CAM_SENSOR_CMD;
	}

	if (req_sensor_ctl && req_sensor_ctl->sensitivity)
		sensitivity = req_sensor_ctl->sensitivity;
	else
		sensitivity = isp_sensor_ctl->sensitivity;

	if (cur_sensor_ctl->sensitivity != sensitivity) {
		cur_sensor_ctl->sensitivity = sensitivity;
		/*dbg_ischain("sensitivity : %d\n", (u32)exposure);*/
		updated |= CAM_SENSOR_CMD;
	}

	/*lens part*/
	if (req_lens_ctl && req_lens_ctl->focusDistance)
		focusDistance = req_lens_ctl->focusDistance;
	else
		focusDistance = isp_lens_ctl->focusDistance;

	if (cur_lens_ctl->focusDistance != focusDistance) {
		cur_lens_ctl->focusDistance = focusDistance;
		/*dbg_ischain("focusDistance : %d\n", focusDistance);*/
		updated |= CAM_LENS_CMD;
	}

	if (cur_lens_uctl->slewRate != isp_lens_uctl->slewRate) {
		cur_lens_uctl->slewRate = isp_lens_uctl->slewRate;
		updated |= CAM_LENS_CMD;
	}

	if (cur_lens_uctl->maxPos != isp_lens_uctl->maxPos) {
		cur_lens_uctl->maxPos = isp_lens_uctl->maxPos;
		updated |= CAM_LENS_CMD;
	}

	/*flash part*/
	if (req_flash_ctl && req_flash_ctl->flashMode)
		flashMode = req_flash_ctl->flashMode;
	else
		flashMode = isp_flash_ctl->flashMode;

	if (cur_flash_ctl->flashMode != flashMode) {
		cur_flash_ctl->flashMode = flashMode;
		dbg_ischain("flashMode : %d\n", flashMode);
		updated |= CAM_FLASH_CMD;
	}

	if (cur_flash_ctl->firingPower != isp_flash_ctl->firingPower) {
		cur_flash_ctl->firingPower = isp_flash_ctl->firingPower;
		updated |= CAM_FLASH_CMD;
	}

	if (cur_flash_ctl->firingTime != isp_flash_ctl->firingTime) {
		cur_flash_ctl->firingTime = isp_flash_ctl->firingTime;
		updated |= CAM_FLASH_CMD;
	}
	/*printk("%d %d %d\n", cur_lens_ctl->focusDistance,
		cur_lens_uctl->maxPos,
		cur_lens_uctl->slewRate);*/

	if (updated) {
		this->cur_peri_ctl.uUpdateBitMap = updated;
		memcpy(this->is_region->shared, &this->cur_peri_ctl,
			sizeof(struct camera2_uctl));
		fimc_is_itf_s_camctrl(this, this->minfo.dvaddr_shared, fcount);
	}

	/*lens*/
	index = (fcount + 1) & SENSOR_MAX_CTL_MASK;
	applied_ctl = &this->peri_ctls[index];
	applied_ctl->lensUd.ctl.focusDistance = cur_lens_ctl->focusDistance;
	applied_ctl->lensUd.maxPos = cur_lens_uctl->maxPos;
	applied_ctl->lensUd.slewRate = cur_lens_uctl->slewRate;

	/*sensor*/
	index = (fcount + 3) & SENSOR_MAX_CTL_MASK;
	applied_ctl = &this->peri_ctls[index];
	applied_ctl->sensorUd.ctl.exposureTime = cur_sensor_ctl->exposureTime;
	applied_ctl->sensorUd.ctl.frameDuration = cur_sensor_ctl->frameDuration;
	applied_ctl->sensorUd.ctl.sensitivity = cur_sensor_ctl->sensitivity;

	/*flash*/
	index = (fcount + 0) & SENSOR_MAX_CTL_MASK;
	applied_ctl = &this->peri_ctls[index];
	applied_ctl->flashUd.ctl.flashMode = cur_flash_ctl->flashMode;
	applied_ctl->flashUd.ctl.firingPower = cur_flash_ctl->firingPower;
	applied_ctl->flashUd.ctl.firingTime = cur_flash_ctl->firingTime;

	return ret;
}
#else
int fimc_is_ischain_camctl(struct fimc_is_device_ischain *this,
	struct fimc_is_frame_shot *frame,
	u32 fcount)
{
	int ret = 0;
	struct fimc_is_interface *itf;
	struct camera2_uctl *applied_ctl;

	struct camera2_sensor_ctl *isp_sensor_ctl;
	struct camera2_lens_ctl *isp_lens_ctl;
	struct camera2_flash_ctl *isp_flash_ctl;

	u32 index;

#ifdef DBG_STREAMING
	dbg_ischain("%s\n", __func__);
#endif

	itf = this->interface;
	isp_sensor_ctl = &itf->isp_peri_ctl.sensorUd.ctl;
	isp_lens_ctl = &itf->isp_peri_ctl.lensUd.ctl;
	isp_flash_ctl = &itf->isp_peri_ctl.flashUd.ctl;

	/*lens*/
	index = (fcount + 0) & SENSOR_MAX_CTL_MASK;
	applied_ctl = &this->peri_ctls[index];
	applied_ctl->lensUd.ctl.focusDistance = isp_lens_ctl->focusDistance;

	/*sensor*/
	index = (fcount + 1) & SENSOR_MAX_CTL_MASK;
	applied_ctl = &this->peri_ctls[index];
	applied_ctl->sensorUd.ctl.exposureTime = isp_sensor_ctl->exposureTime;
	applied_ctl->sensorUd.ctl.frameDuration = isp_sensor_ctl->frameDuration;
	applied_ctl->sensorUd.ctl.sensitivity = isp_sensor_ctl->sensitivity;

	/*flash*/
	index = (fcount + 0) & SENSOR_MAX_CTL_MASK;
	applied_ctl = &this->peri_ctls[index];
	applied_ctl->flashUd.ctl.flashMode = isp_flash_ctl->flashMode;
	applied_ctl->flashUd.ctl.firingPower = isp_flash_ctl->firingPower;
	applied_ctl->flashUd.ctl.firingTime = isp_flash_ctl->firingTime;

	return ret;
}
#endif

int fimc_is_ischain_tag(struct fimc_is_device_ischain *ischain,
	struct fimc_is_frame_shot *frame)
{
	int ret = 0;
	struct camera2_uctl *applied_ctl;
	struct timeval curtime;
	u32 fcount;

	fcount = frame->fcount;
	applied_ctl = &ischain->peri_ctls[fcount & SENSOR_MAX_CTL_MASK];

	do_gettimeofday(&curtime);

	/* Request */
	frame->shot->dm.request.frameCount = fcount;

	/* Lens */
	frame->shot->dm.lens.focusDistance =
		applied_ctl->lensUd.ctl.focusDistance;

	/* Sensor */
	frame->shot->dm.sensor.exposureTime =
		applied_ctl->sensorUd.ctl.exposureTime;
	frame->shot->dm.sensor.sensitivity =
		applied_ctl->sensorUd.ctl.sensitivity;
	frame->shot->dm.sensor.frameDuration =
		applied_ctl->sensorUd.ctl.frameDuration;
	frame->shot->dm.sensor.timeStamp =
		curtime.tv_sec*1000000 + curtime.tv_usec;

	/* Flash */
	frame->shot->dm.flash.flashMode =
		applied_ctl->flashUd.ctl.flashMode;
	frame->shot->dm.flash.firingPower =
		applied_ctl->flashUd.ctl.firingPower;
	frame->shot->dm.flash.firingTime =
		applied_ctl->flashUd.ctl.firingTime;

	return ret;
}
