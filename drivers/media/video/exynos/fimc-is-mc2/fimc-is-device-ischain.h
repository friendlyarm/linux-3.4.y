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

#ifndef FIMC_IS_DEVICE_ISCHAIN_H
#define FIMC_IS_DEVICE_ISCHAIN_H

#include "fimc-is-mem.h"

#define FIMC_IS_A5_MEM_SIZE		(0x00A00000)
#define FIMC_IS_A5_SEN_SIZE		(0x00100000)
#define FIMC_IS_REGION_SIZE		(0x5000)
#define FIMC_IS_SETFILE_SIZE		(0xc0d8)
#define FIMC_IS_TDNR_MEM_SIZE		(1920*1080*4)
#define FIMC_IS_DEBUG_REGION_ADDR	(0x00840000)
#define FIMC_IS_SHARED_REGION_ADDR	(0x008C0000)

#define MAX_ODC_INTERNAL_BUF_WIDTH	(2560)  /* 4808 in HW */
#define MAX_ODC_INTERNAL_BUF_HEIGHT	(1920)  /* 3356 in HW */
#define SIZE_ODC_INTERNAL_BUF \
	(MAX_ODC_INTERNAL_BUF_WIDTH * MAX_ODC_INTERNAL_BUF_HEIGHT * 3)

#define MAX_DIS_INTERNAL_BUF_WIDTH	(2400)
#define MAX_DIS_INTERNAL_BUF_HEIGHT	(1360)
#define SIZE_DIS_INTERNAL_BUF \
	(MAX_DIS_INTERNAL_BUF_WIDTH * MAX_DIS_INTERNAL_BUF_HEIGHT * 2)

#define MAX_3DNR_INTERNAL_BUF_WIDTH	(1920)
#define MAX_3DNR_INTERNAL_BUF_HEIGHT	(1088)
#define SIZE_DNR_INTERNAL_BUF \
	(MAX_3DNR_INTERNAL_BUF_WIDTH * MAX_3DNR_INTERNAL_BUF_HEIGHT * 2)

#define NUM_ODC_INTERNAL_BUF		(2)
#define NUM_DIS_INTERNAL_BUF		(5)
#define NUM_DNR_INTERNAL_BUF		(2)

#define SENSOR_MAX_CTL			0x10
#define SENSOR_MAX_CTL_MASK		(SENSOR_MAX_CTL-1)

/*global state*/
enum fimc_is_ischain_state {
	FIMC_IS_ISCHAIN_OPEN,
	FIMC_IS_ISCHAIN_LOADED,
	FIMC_IS_ISCHAIN_POWER_ON,
	FIMC_IS_ISCHAIN_RUN
};

struct fimc_is_ishcain_mem {
	dma_addr_t	base;		/* buffer base */
	size_t		size;		/* total length */
	dma_addr_t	vaddr_base;	/* buffer base */
	dma_addr_t	vaddr_curr;	/* current addr */
	void		*fw_cookie;

	u32		dvaddr;
	u32		kvaddr;
	u32		dvaddr_debug;
	u32		kvaddr_debug;
	u32		dvaddr_fshared;
	u32		kvaddr_fshared;
	u32		dvaddr_region;
	u32		kvaddr_region;
	u32		dvaddr_shared; /*shared region of is region*/
	u32		kvaddr_shared;
	u32		dvaddr_odc;
	u32		kvaddr_odc;
	u32		dvaddr_dis;
	u32		kvaddr_dis;
	u32		dvaddr_3dnr;
	u32		kvaddr_3dnr;
};

/*device state*/
enum fimc_is_isdev_state {
	FIMC_IS_ISDEV_DSTART
};

struct fimc_is_ischain_dev {
	enum is_entry				entry;
	unsigned long				state;
	struct mutex				mutex_state;

	struct fimc_is_framemgr			framemgr;
	struct fimc_is_video_common		*video;
};

struct fimc_is_device_ischain {
	struct platform_device			*pdev;
	struct device				*bus_dev;
	struct exynos5_platform_fimc_is		*pdata;
	void __iomem				*regs;

	struct fimc_is_ishcain_mem		minfo;

	struct fimc_is_interface		*interface;
	struct fimc_is_framemgr			*framemgr;
	struct fimc_is_mem			*mem;

	struct is_region			*is_region;

	bool					force_down;
	unsigned long				state;
	struct mutex				mutex_state;
	spinlock_t				slock_state;

	u32					instance;
	u32					dzoom_width;
	u32					setfile;
	u32					fcount;
	u32					debug_cnt;

	struct camera2_sm			capability;
	struct camera2_uctl			cur_peri_ctl;
	struct camera2_uctl			peri_ctls[SENSOR_MAX_CTL];

	/*isp margin*/
	u32					sensor_width;
	u32					sensor_height;
	u32					margin_left;
	u32					margin_right;
	u32					margin_width;
	u32					margin_top;
	u32					margin_bottom;
	u32					margin_height;

	/*isp ~ scc*/
	u32					chain0_width;
	u32					chain0_height;
	struct fimc_is_ischain_dev		isp;
	struct fimc_is_ischain_dev		drc;

	/*scc ~ dis*/
	u32					chain1_width;
	u32					chain1_height;
	u32					crop_x;
	u32					crop_y;
	u32					crop_width;
	u32					crop_height;
	struct fimc_is_ischain_dev		scc;
	struct fimc_is_ischain_dev		dis;

	/*dis ~ scp*/
	u32					chain2_width;
	u32					chain2_height;
	struct fimc_is_ischain_dev		dnr;

	/*scp ~ fd*/
	u32					chain3_width;
	u32					chain3_height;
	struct fimc_is_ischain_dev		scp;
	struct fimc_is_ischain_dev		fd;

	u32					lindex;
	u32					hindex;
	u32					indexes;

	u32					private_data;
};

/*global function*/
int fimc_is_ischain_probe(struct fimc_is_device_ischain *this,
	struct fimc_is_interface *interface,
	struct fimc_is_framemgr *framemgr,
	struct fimc_is_mem *mem,
	struct platform_device *pdev,
	u32 regs);
int fimc_is_ischain_open(struct fimc_is_device_ischain *this,
	struct fimc_is_video_common *video);
int fimc_is_ischain_close(struct fimc_is_device_ischain *this);
int fimc_is_ischain_init(struct fimc_is_device_ischain *this,
	u32 input, u32 channel, struct sensor_open_extended *ext,
	char *setfile_name);
int fimc_is_ischain_g_capability(struct fimc_is_device_ischain *this,
	u32 user_ptr);
int fimc_is_ischain_print_status(struct fimc_is_device_ischain *this);

/*isp subdev*/
int fimc_is_ischain_isp_start(struct fimc_is_device_ischain *this,
	struct fimc_is_video_common *video);
int fimc_is_ischain_isp_stop(struct fimc_is_device_ischain *this);
int fimc_is_ischain_isp_s_format(struct fimc_is_device_ischain *this,
	u32 width, u32 height);
int fimc_is_ischain_isp_buffer_queue(struct fimc_is_device_ischain *this,
	u32 index);
int fimc_is_ischain_isp_buffer_finish(struct fimc_is_device_ischain *this,
	u32 index);

/*scc subdev*/
int fimc_is_ischain_scc_start(struct fimc_is_device_ischain *this);
int fimc_is_ischain_scc_stop(struct fimc_is_device_ischain *this);

/*scp subdev*/
int fimc_is_ischain_scp_start(struct fimc_is_device_ischain *this);
int fimc_is_ischain_scp_stop(struct fimc_is_device_ischain *this);
int fimc_is_ischain_scp_s_format(struct fimc_is_device_ischain *this,
	u32 width, u32 height);

/*common subdev*/
int fimc_is_ischain_dev_start(struct fimc_is_ischain_dev *this);
int fimc_is_ischain_dev_stop(struct fimc_is_ischain_dev *this);
int fimc_is_ischain_dev_open(struct fimc_is_ischain_dev *this,
	enum is_entry entry,
	struct fimc_is_video_common *video,
	const struct param_control *init_ctl);
int fimc_is_ischain_dev_buffer_queue(struct fimc_is_ischain_dev *this,
	u32 index);
int fimc_is_ischain_dev_buffer_finish(struct fimc_is_ischain_dev *this,
	u32 index);

/*special api for sensor*/
int fimc_is_ischain_callback(struct fimc_is_device_ischain *this);
int fimc_is_ischain_camctl(struct fimc_is_device_ischain *this,
	struct fimc_is_frame_shot *frame,
	u32 fcount);
int fimc_is_ischain_tag(struct fimc_is_device_ischain *ischain,
	struct fimc_is_frame_shot *frame);

int fimc_is_itf_process_on(struct fimc_is_device_ischain *this);
int fimc_is_itf_process_off(struct fimc_is_device_ischain *this);
int fimc_is_itf_stream_on(struct fimc_is_device_ischain *this);
int fimc_is_itf_stream_off(struct fimc_is_device_ischain *this);

#endif
