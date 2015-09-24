/*
 * Samsung Exynos5 SoC series FIMC-IS driver
 *
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef FIMC_IS_CORE_H
#define FIMC_IS_CORE_H

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/exynos_fimc_is.h>
#include <media/v4l2-ioctl.h>
#include <media/exynos_mc.h>
#include <media/videobuf2-core.h>
#if defined(CONFIG_VIDEOBUF2_CMA_PHYS)
#include <media/videobuf2-cma-phys.h>
#elif defined(CONFIG_VIDEOBUF2_ION)
#include <media/videobuf2-ion.h>
#endif
#include "fimc-is-param.h"

#include "fimc-is-device-sensor.h"
#include "fimc-is-interface.h"
#include "fimc-is-framemgr.h"
#include "fimc-is-device-ischain.h"

#include "fimc-is-video-sensor.h"
#include "fimc-is-video-isp.h"
#include "fimc-is-video-scc.h"
#include "fimc-is-video-scp.h"
#include "fimc-is-mem.h"

#define FIMC_IS_MODULE_NAME			"exynos5-fimc-is2"
#define FIMC_IS_SENSOR_ENTITY_NAME		"exynos5-fimc-is2-sensor"
#define FIMC_IS_FRONT_ENTITY_NAME		"exynos5-fimc-is2-front"
#define FIMC_IS_BACK_ENTITY_NAME		"exynos5-fimc-is2-back"
#define FIMC_IS_VIDEO_BAYER_NAME		"exynos5-fimc-is2-bayer"
#define FIMC_IS_VIDEO_ISP_NAME			"exynos5-fimc-is2-isp"
#define FIMC_IS_VIDEO_SCALERC_NAME		"exynos5-fimc-is2-scalerc"
#define FIMC_IS_VIDEO_3DNR_NAME			"exynos5-fimc-is2-3dnr"
#define FIMC_IS_VIDEO_SCALERP_NAME		"exynos5-fimc-is2-scalerp"

#define FIMC_IS_COMMAND_TIMEOUT			(3*HZ)
#define FIMC_IS_STARTUP_TIMEOUT			(3*HZ)
#define FIMC_IS_SHUTDOWN_TIMEOUT		(10*HZ)
#define FIMC_IS_FLITE_STOP_TIMEOUT		(3*HZ)

#define FIMC_IS_SENSOR_MAX_ENTITIES		(1)
#define FIMC_IS_SENSOR_PAD_SOURCE_FRONT		(0)
#define FIMC_IS_SENSOR_PADS_NUM			(1)

#define FIMC_IS_FRONT_MAX_ENTITIES		(1)
#define FIMC_IS_FRONT_PAD_SINK			(0)
#define FIMC_IS_FRONT_PAD_SOURCE_BACK		(1)
#define FIMC_IS_FRONT_PAD_SOURCE_BAYER		(2)
#define FIMC_IS_FRONT_PAD_SOURCE_SCALERC	(3)
#define FIMC_IS_FRONT_PADS_NUM			(4)

#define FIMC_IS_BACK_MAX_ENTITIES		(1)
#define FIMC_IS_BACK_PAD_SINK			(0)
#define FIMC_IS_BACK_PAD_SOURCE_3DNR		(1)
#define FIMC_IS_BACK_PAD_SOURCE_SCALERP		(2)
#define FIMC_IS_BACK_PADS_NUM			(3)

#define FIMC_IS_MAX_SENSOR_NAME_LEN		(16)

#define FW_SHARED_OFFSET			(0x8C0000)
#define DEBUG_CNT				(500*1024)
#define DEBUG_OFFSET				(0x840000)
#define DEBUGCTL_OFFSET				(0x8BD000)
#define DEBUG_FCOUNT				(0x8C64C0)

#ifdef err
#undef err
#endif
#define err(fmt, args...) \
	printk(KERN_ERR "ERR:%s:%d: " fmt "\n", __func__, __LINE__, ##args)

#ifdef warn
#undef warn
#endif
#define warn(fmt, args...) \
	printk(KERN_WARNING "%s:%d: " fmt "\n", __func__, __LINE__, ##args)

/* configuration - default post processing */
/*#define ENABLE_DRC*/
/*#define ENABLE_ODC*/
#define ENABLE_VDIS
#define ENABLE_TDNR
#define ENABLE_FD

/*#define DEBUG*/
/*#define DBG_STREAMING*/
/*#define DBG_FLITEISR*/
/*#define AUTO_MODE*/
#define FW_DEBUG
/*#define RESERVED_MEM*/
#define USE_FRAME_SYNC
#define USE_ADVANCED_DZOOM
/*#define TASKLET_MSG*/
/*#define PRINT_BUFADDR*/
/*#define PRINT_DZOOM*/
#define CHECK_FDROP
#define ISDRV_VERSION 111

#ifdef DEBUG
#define dbg(fmt, args...) \
	/*printk(KERN_DEBUG "%s:%d: " fmt "\n", __func__, __LINE__, ##args)*/

#define dbg_warning(fmt, args...) \
	printk(KERN_INFO "%s[WAR] Warning! " fmt, __func__, ##args)

#define dbg_sensor(fmt, args...) \
	printk(KERN_INFO "[SEN] " fmt, ##args)

#define dbg_isp(fmt, args...) \
	printk(KERN_INFO "[ISP] " fmt, ##args)

#define dbg_scp(fmt, args...) \
	printk(KERN_INFO "[SCP] " fmt, ##args)

#define dbg_scc(fmt, args...) \
	printk(KERN_INFO "[SCC] " fmt, ##args)

#define dbg_front(fmt, args...) \
	printk(KERN_INFO "[FRT] " fmt, ##args)

#define dbg_back(fmt, args...) \
	printk(KERN_INFO "[BAK] " fmt, ##args)

#define dbg_ischain(fmt, args...) \
	printk(KERN_INFO "[ISC] " fmt, ##args)

#define dbg_core(fmt, args...) \
	printk(KERN_INFO "[COR] " fmt, ##args)



#ifdef DBG_STREAMING
#define dbg_interface(fmt, args...) \
	printk(KERN_INFO "[ITF] " fmt, ##args)
#define dbg_frame(fmt, args...) \
	printk(KERN_INFO "[FRM] " fmt, ##args)
#else
#define dbg_interface(fmt, args...)
#define dbg_frame(fmt, args...)
#endif
#else
#define dbg(fmt, args...)
#define dbg_warning(fmt, args...)
#define dbg_sensor(fmt, args...)
#define dbg_isp(fmt, args...)
#define dbg_scp(fmt, args...)
#define dbg_scc(fmt, args...)
#define dbg_front(fmt, args...)
#define dbg_back(fmt, args...)
#define dbg_ischain(fmt, args...)
#define dbg_core(fmt, args...)
#define dbg_interface(fmt, args...)
#define dbg_frame(fmt, args...)
#endif

enum fimc_is_debug_device {
	FIMC_IS_DEBUG_MAIN = 0,
	FIMC_IS_DEBUG_EC,
	FIMC_IS_DEBUG_SENSOR,
	FIMC_IS_DEBUG_ISP,
	FIMC_IS_DEBUG_DRC,
	FIMC_IS_DEBUG_FD,
	FIMC_IS_DEBUG_SDK,
	FIMC_IS_DEBUG_SCALERC,
	FIMC_IS_DEBUG_ODC,
	FIMC_IS_DEBUG_DIS,
	FIMC_IS_DEBUG_TDNR,
	FIMC_IS_DEBUG_SCALERP
};

enum fimc_is_debug_target {
	FIMC_IS_DEBUG_UART = 0,
	FIMC_IS_DEBUG_MEMORY,
	FIMC_IS_DEBUG_DCC3
};

enum fimc_is_front_input_entity {
	FIMC_IS_FRONT_INPUT_NONE = 0,
	FIMC_IS_FRONT_INPUT_SENSOR,
};

enum fimc_is_front_output_entity {
	FIMC_IS_FRONT_OUTPUT_NONE = 0,
	FIMC_IS_FRONT_OUTPUT_BACK,
	FIMC_IS_FRONT_OUTPUT_BAYER,
	FIMC_IS_FRONT_OUTPUT_SCALERC,
};

enum fimc_is_back_input_entity {
	FIMC_IS_BACK_INPUT_NONE = 0,
	FIMC_IS_BACK_INPUT_FRONT,
};

enum fimc_is_back_output_entity {
	FIMC_IS_BACK_OUTPUT_NONE = 0,
	FIMC_IS_BACK_OUTPUT_3DNR,
	FIMC_IS_BACK_OUTPUT_SCALERP,
};

enum fimc_is_front_state {
	FIMC_IS_FRONT_ST_POWERED = 0,
	FIMC_IS_FRONT_ST_STREAMING,
	FIMC_IS_FRONT_ST_SUSPENDED,
};

enum fimc_is_video_dev_num {
	FIMC_IS_VIDEO_NUM_BAYER = 0,
	FIMC_IS_VIDEO_NUM_ISP,
	FIMC_IS_VIDEO_NUM_SCALERC,
	FIMC_IS_VIDEO_NUM_3DNR,
	FIMC_IS_VIDEO_NUM_SCALERP,
	FIMC_IS_VIDEO_MAX_NUM,
};

struct fimc_is_core;

struct fimc_is_front_dev {
	struct v4l2_subdev		sd;
	struct media_pad		pads[FIMC_IS_FRONT_PADS_NUM];
	struct v4l2_mbus_framefmt	mbus_fmt[FIMC_IS_FRONT_PADS_NUM];
	enum fimc_is_front_input_entity	input;
	enum fimc_is_front_output_entity	output;
	u32 width;
	u32 height;

};

struct fimc_is_back_dev {
	struct v4l2_subdev		sd;
	struct media_pad		pads[FIMC_IS_BACK_PADS_NUM];
	struct v4l2_mbus_framefmt	mbus_fmt[FIMC_IS_BACK_PADS_NUM];
	enum fimc_is_back_input_entity	input;
	enum fimc_is_back_output_entity	output;
	int	dis_on;
	int	odc_on;
	int	tdnr_on;
	u32 width;
	u32 height;
	u32 dis_width;
	u32 dis_height;
};

struct fimc_is_core {
	struct platform_device			*pdev;
	struct resource				*regs_res;
	void __iomem				*regs;
	int					irq;
	u32					id;

	/* depended on isp */
	struct exynos5_platform_fimc_is		*pdata;
	struct exynos_md			*mdev;

	struct fimc_is_mem			mem;
	struct fimc_is_framemgr			framemgr_sensor;
	struct fimc_is_framemgr			framemgr_ischain;
	struct fimc_is_interface		interface;

	struct fimc_is_device_sensor		sensor;
	struct fimc_is_device_ischain		ischain;
	struct fimc_is_front_dev		front;
	struct fimc_is_back_dev			back;

	/* 0-bayer, 1-scalerC, 2-3DNR, 3-scalerP */
	struct fimc_is_video_sensor		video_sensor;
	struct fimc_is_video_isp		video_isp;
	struct fimc_is_video_scc		video_scc;
	struct fimc_is_video_scp		video_scp;

};

extern const struct v4l2_file_operations fimc_is_bayer_video_fops;
extern const struct v4l2_ioctl_ops fimc_is_bayer_video_ioctl_ops;
extern const struct vb2_ops fimc_is_bayer_qops;

extern const struct v4l2_file_operations fimc_is_isp_video_fops;
extern const struct v4l2_ioctl_ops fimc_is_isp_video_ioctl_ops;
extern const struct vb2_ops fimc_is_isp_qops;

extern const struct v4l2_file_operations fimc_is_scalerc_video_fops;
extern const struct v4l2_ioctl_ops fimc_is_scalerc_video_ioctl_ops;
extern const struct vb2_ops fimc_is_scalerc_qops;

extern const struct v4l2_file_operations fimc_is_scalerp_video_fops;
extern const struct v4l2_ioctl_ops fimc_is_scalerp_video_ioctl_ops;
extern const struct vb2_ops fimc_is_scalerp_qops;

extern const struct v4l2_file_operations fimc_is_3dnr_video_fops;
extern const struct v4l2_ioctl_ops fimc_is_3dnr_video_ioctl_ops;
extern const struct vb2_ops fimc_is_3dnr_qops;

#if defined(CONFIG_VIDEOBUF2_CMA_PHYS)
extern const struct fimc_is_vb2 fimc_is_vb2_cma;
#elif defined(CONFIG_VIDEOBUF2_ION)
extern const struct fimc_is_vb2 fimc_is_vb2_ion;
#endif

void fimc_is_mem_suspend(void *alloc_ctxes);
void fimc_is_mem_resume(void *alloc_ctxes);
void fimc_is_mem_cache_clean(const void *start_addr, unsigned long size);
void fimc_is_mem_cache_inv(const void *start_addr, unsigned long size);
int fimc_is_pipeline_s_stream_preview
	(struct media_entity *start_entity, int on);
int fimc_is_init_set(struct fimc_is_core *dev , u32 val);
int fimc_is_load_fw(struct fimc_is_core *dev);
int fimc_is_load_setfile(struct fimc_is_core *dev);
int fimc_is_spi_read(void *buf, size_t size);
int fimc_is_runtime_suspend(struct device *dev);
int fimc_is_runtime_resume(struct device *dev);
#endif /* FIMC_IS_CORE_H_ */
