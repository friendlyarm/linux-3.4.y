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
#ifdef CONFIG_BUSFREQ_OPP
#ifdef CONFIG_CPU_EXYNOS5250
#include <mach/dev.h>
#endif
#endif
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

#include <mach/map.h>
#include <mach/regs-clock.h>

#include "fimc-is-time.h"
#include "fimc-is-core.h"
#include "fimc-is-regs.h"
#include "fimc-is-interface.h"
#include "fimc-is-device-flite.h"

#define FIMCLITE0_REG_BASE	(S5P_VA_FIMCLITE0)  /* phy : 0x13c0_0000 */
#define FIMCLITE1_REG_BASE	(S5P_VA_FIMCLITE1)  /* phy : 0x13c1_0000 */
#define FIMCLITE2_REG_BASE	(S5P_VA_FIMCLITE2)  /* phy : 0x13c9_0000 */

#define FLITE_MAX_RESET_READY_TIME	(20) /* 100ms */
#define FLITE_MAX_WIDTH_SIZE		(8192)
#define FLITE_MAX_HEIGHT_SIZE		(8192)

/*FIMCLite*/
/* Camera Source size */
#define FLITE_REG_CISRCSIZE				(0x00)
#define FLITE_REG_CISRCSIZE_SIZE_H(x)			((x) << 16)
#define FLITE_REG_CISRCSIZE_SIZE_V(x)			((x) << 0)
#define FLITE_REG_CISRCSIZE_ORDER422_IN_YCBYCR		(0 << 14)
#define FLITE_REG_CISRCSIZE_ORDER422_IN_YCRYCB		(1 << 14)
#define FLITE_REG_CISRCSIZE_ORDER422_IN_CBYCRY		(2 << 14)
#define FLITE_REG_CISRCSIZE_ORDER422_IN_CRYCBY		(3 << 14)

/* Global control */
#define FLITE_REG_CIGCTRL				0x04
#define FLITE_REG_CIGCTRL_YUV422_1P			(0x1E << 24)
#define FLITE_REG_CIGCTRL_RAW8				(0x2A << 24)
#define FLITE_REG_CIGCTRL_RAW10				(0x2B << 24)
#define FLITE_REG_CIGCTRL_RAW12				(0x2C << 24)
#define FLITE_REG_CIGCTRL_RAW14				(0x2D << 24)

/* User defined formats. x = 0...0xF. */
#define FLITE_REG_CIGCTRL_USER(x)			(0x30 + x - 1)
#define FLITE_REG_CIGCTRL_OLOCAL_DISABLE		(1 << 22)
#define FLITE_REG_CIGCTRL_SHADOWMASK_DISABLE		(1 << 21)
#define FLITE_REG_CIGCTRL_ODMA_DISABLE			(1 << 20)
#define FLITE_REG_CIGCTRL_SWRST_REQ			(1 << 19)
#define FLITE_REG_CIGCTRL_SWRST_RDY			(1 << 18)
#define FLITE_REG_CIGCTRL_SWRST				(1 << 17)
#define FLITE_REG_CIGCTRL_TEST_PATTERN_COLORBAR		(1 << 15)
#define FLITE_REG_CIGCTRL_INVPOLPCLK			(1 << 14)
#define FLITE_REG_CIGCTRL_INVPOLVSYNC			(1 << 13)
#define FLITE_REG_CIGCTRL_INVPOLHREF			(1 << 12)
#define FLITE_REG_CIGCTRL_IRQ_LASTEN0_ENABLE		(0 << 8)
#define FLITE_REG_CIGCTRL_IRQ_LASTEN0_DISABLE		(1 << 8)
#define FLITE_REG_CIGCTRL_IRQ_ENDEN0_ENABLE		(0 << 7)
#define FLITE_REG_CIGCTRL_IRQ_ENDEN0_DISABLE		(1 << 7)
#define FLITE_REG_CIGCTRL_IRQ_STARTEN0_ENABLE		(0 << 6)
#define FLITE_REG_CIGCTRL_IRQ_STARTEN0_DISABLE		(1 << 6)
#define FLITE_REG_CIGCTRL_IRQ_OVFEN0_ENABLE		(0 << 5)
#define FLITE_REG_CIGCTRL_IRQ_OVFEN0_DISABLE		(1 << 5)
#define FLITE_REG_CIGCTRL_SELCAM_MIPI			(1 << 3)

/* Image Capture Enable */
#define FLITE_REG_CIIMGCPT				(0x08)
#define FLITE_REG_CIIMGCPT_IMGCPTEN			(1 << 31)
#define FLITE_REG_CIIMGCPT_CPT_FREN			(1 << 25)
#define FLITE_REG_CIIMGCPT_CPT_FRPTR(x)			((x) << 19)
#define FLITE_REG_CIIMGCPT_CPT_MOD_FRCNT		(1 << 18)
#define FLITE_REG_CIIMGCPT_CPT_MOD_FREN			(0 << 18)
#define FLITE_REG_CIIMGCPT_CPT_FRCNT(x)			((x) << 10)

/* Capture Sequence */
#define FLITE_REG_CICPTSEQ				(0x0C)
#define FLITE_REG_CPT_FRSEQ(x)				((x) << 0)

/* Camera Window Offset */
#define FLITE_REG_CIWDOFST				(0x10)
#define FLITE_REG_CIWDOFST_WINOFSEN			(1 << 31)
#define FLITE_REG_CIWDOFST_CLROVIY			(1 << 31)
#define FLITE_REG_CIWDOFST_WINHOROFST(x)		((x) << 16)
#define FLITE_REG_CIWDOFST_HOROFF_MASK			(0x1fff << 16)
#define FLITE_REG_CIWDOFST_CLROVFICB			(1 << 15)
#define FLITE_REG_CIWDOFST_CLROVFICR			(1 << 14)
#define FLITE_REG_CIWDOFST_WINVEROFST(x)		((x) << 0)
#define FLITE_REG_CIWDOFST_VEROFF_MASK			(0x1fff << 0)

/* Cmaera Window Offset2 */
#define FLITE_REG_CIWDOFST2				(0x14)
#define FLITE_REG_CIWDOFST2_WINHOROFST2(x)		((x) << 16)
#define FLITE_REG_CIWDOFST2_WINVEROFST2(x)		((x) << 0)

/* Camera Output DMA Format */
#define FLITE_REG_CIODMAFMT				(0x18)
#define FLITE_REG_CIODMAFMT_1D_DMA			(1 << 15)
#define FLITE_REG_CIODMAFMT_2D_DMA			(0 << 15)
#define FLITE_REG_CIODMAFMT_PACK12			(1 << 14)
#define FLITE_REG_CIODMAFMT_NORMAL			(0 << 14)
#define FLITE_REG_CIODMAFMT_CRYCBY			(0 << 4)
#define FLITE_REG_CIODMAFMT_CBYCRY			(1 << 4)
#define FLITE_REG_CIODMAFMT_YCRYCB			(2 << 4)
#define FLITE_REG_CIODMAFMT_YCBYCR			(3 << 4)

/* Camera Output Canvas */
#define FLITE_REG_CIOCAN				(0x20)
#define FLITE_REG_CIOCAN_OCAN_V(x)			((x) << 16)
#define FLITE_REG_CIOCAN_OCAN_H(x)			((x) << 0)

/* Camera Output DMA Offset */
#define FLITE_REG_CIOOFF				(0x24)
#define FLITE_REG_CIOOFF_OOFF_V(x)			((x) << 16)
#define FLITE_REG_CIOOFF_OOFF_H(x)			((x) << 0)

/* Camera Output DMA Address */
#define FLITE_REG_CIOSA					(0x30)
#define FLITE_REG_CIOSA_OSA(x)				((x) << 0)

/* Camera Status */
#define FLITE_REG_CISTATUS				(0x40)
#define FLITE_REG_CISTATUS_MIPI_VVALID			(1 << 22)
#define FLITE_REG_CISTATUS_MIPI_HVALID			(1 << 21)
#define FLITE_REG_CISTATUS_MIPI_DVALID			(1 << 20)
#define FLITE_REG_CISTATUS_ITU_VSYNC			(1 << 14)
#define FLITE_REG_CISTATUS_ITU_HREFF			(1 << 13)
#define FLITE_REG_CISTATUS_OVFIY			(1 << 10)
#define FLITE_REG_CISTATUS_OVFICB			(1 << 9)
#define FLITE_REG_CISTATUS_OVFICR			(1 << 8)
#define FLITE_REG_CISTATUS_IRQ_SRC_OVERFLOW		(1 << 7)
#define FLITE_REG_CISTATUS_IRQ_SRC_LASTCAPEND		(1 << 6)
#define FLITE_REG_CISTATUS_IRQ_SRC_FRMSTART		(1 << 5)
#define FLITE_REG_CISTATUS_IRQ_SRC_FRMEND		(1 << 4)
#define FLITE_REG_CISTATUS_IRQ_CAM			(1 << 0)
#define FLITE_REG_CISTATUS_IRQ_MASK			(0xf << 4)

/* Camera Status2 */
#define FLITE_REG_CISTATUS2				(0x44)
#define FLITE_REG_CISTATUS2_LASTCAPEND			(1 << 1)
#define FLITE_REG_CISTATUS2_FRMEND			(1 << 0)

/* Camera Status3 */
#define FLITE_REG_CISTATUS3				0x48
#define FLITE_REG_CISTATUS3_PRESENT_MASK		(0x3F)

/* Qos Threshold */
#define FLITE_REG_CITHOLD				(0xF0)
#define FLITE_REG_CITHOLD_W_QOS_EN			(1 << 30)
#define FLITE_REG_CITHOLD_WTH_QOS(x)			((x) << 0)

/* Camera General Purpose */
#define FLITE_REG_CIGENERAL				(0xFC)
#define FLITE_REG_CIGENERAL_CAM_A			(0 << 0)
#define FLITE_REG_CIGENERAL_CAM_B			(1 << 0)

#define FLITE_REG_CIFCNTSEQ				0x100

static void flite_hw_set_cam_source_size(unsigned long flite_reg_base,
					struct fimc_is_frame_info *f_frame)
{
	u32 cfg = 0;

	cfg = readl(flite_reg_base + FLITE_REG_CISRCSIZE);

	cfg |= FLITE_REG_CISRCSIZE_SIZE_H(f_frame->o_width);
	cfg |= FLITE_REG_CISRCSIZE_SIZE_V(f_frame->o_height);

	writel(cfg, flite_reg_base + FLITE_REG_CISRCSIZE);

	cfg = readl(flite_reg_base + FLITE_REG_CIOCAN);
	cfg |= FLITE_REG_CIOCAN_OCAN_H(f_frame->o_width);
	cfg |= FLITE_REG_CIOCAN_OCAN_V(f_frame->o_height);

	writel(cfg, flite_reg_base + FLITE_REG_CIOCAN);
}

static void flite_hw_set_cam_channel(unsigned long flite_reg_base)
{
	u32 cfg = 0;

	if (flite_reg_base == (unsigned long)FIMCLITE0_REG_BASE) {
		cfg = FLITE_REG_CIGENERAL_CAM_A;
		writel(cfg, FIMCLITE0_REG_BASE + FLITE_REG_CIGENERAL);
		writel(cfg, FIMCLITE1_REG_BASE + FLITE_REG_CIGENERAL);
		writel(cfg, FIMCLITE2_REG_BASE + FLITE_REG_CIGENERAL);
	} else {
		cfg = FLITE_REG_CIGENERAL_CAM_B;
		writel(cfg, FIMCLITE0_REG_BASE + FLITE_REG_CIGENERAL);
		writel(cfg, FIMCLITE1_REG_BASE + FLITE_REG_CIGENERAL);
		writel(cfg, FIMCLITE2_REG_BASE + FLITE_REG_CIGENERAL);
	}
}

void flite_hw_set_capture_start(unsigned long flite_reg_base)
{
	u32 cfg = 0;

	cfg = readl(flite_reg_base + FLITE_REG_CIIMGCPT);
	cfg |= FLITE_REG_CIIMGCPT_IMGCPTEN;

	writel(cfg, flite_reg_base + FLITE_REG_CIIMGCPT);
}

static void flite_hw_set_capture_stop(unsigned long flite_reg_base)
{
	u32 cfg = 0;

	cfg = readl(flite_reg_base + FLITE_REG_CIIMGCPT);
	cfg &= ~FLITE_REG_CIIMGCPT_IMGCPTEN;

	writel(cfg, flite_reg_base + FLITE_REG_CIIMGCPT);
}

static int flite_hw_set_source_format(unsigned long flite_reg_base)
{
	u32 cfg = 0;

	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);
	cfg |= FLITE_REG_CIGCTRL_RAW10;
	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);

	return 0;
}

void flite_hw_set_output_dma(unsigned long flite_reg_base, bool enable)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);

	if (enable)
		cfg &= ~FLITE_REG_CIGCTRL_ODMA_DISABLE;
	else
		cfg |= FLITE_REG_CIGCTRL_ODMA_DISABLE;

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}

void flite_hw_set_output_local(unsigned long flite_reg_base, bool enable)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);

	if (enable)
		cfg &= ~FLITE_REG_CIGCTRL_OLOCAL_DISABLE;
	else
		cfg |= FLITE_REG_CIGCTRL_OLOCAL_DISABLE;

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}

/* will use for pattern generation testing
static void flite_hw_set_test_pattern_enable(void)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);
	cfg |= FLITE_REG_CIGCTRL_TEST_PATTERN_COLORBAR;

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}
*/

static void flite_hw_set_config_irq(unsigned long flite_reg_base)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);
	cfg &= ~(FLITE_REG_CIGCTRL_INVPOLPCLK | FLITE_REG_CIGCTRL_INVPOLVSYNC
			| FLITE_REG_CIGCTRL_INVPOLHREF);

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}

static void flite_hw_set_interrupt_source(unsigned long flite_reg_base)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);

	/*
	for checking stop complete
	*/
	cfg &= ~FLITE_REG_CIGCTRL_IRQ_LASTEN0_DISABLE;

	/*
	for checking frame done
	*/
	cfg &= ~FLITE_REG_CIGCTRL_IRQ_ENDEN0_DISABLE;

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}

static void flite_hw_set_interrupt_starten0_disable
					(unsigned long flite_reg_base)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);
	cfg |= FLITE_REG_CIGCTRL_IRQ_STARTEN0_DISABLE;

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}

static void flite_hw_set_camera_type(unsigned long flite_reg_base)
{
	u32 cfg = 0;
	cfg = readl(flite_reg_base + FLITE_REG_CIGCTRL);

	cfg |= FLITE_REG_CIGCTRL_SELCAM_MIPI;

	writel(cfg, flite_reg_base + FLITE_REG_CIGCTRL);
}

static void flite_hw_set_window_offset(unsigned long flite_reg_base,
					struct fimc_is_frame_info *f_frame)
{
	u32 cfg = 0;
	u32 hoff2, voff2;

	cfg = readl(flite_reg_base + FLITE_REG_CIWDOFST);
	cfg &= ~(FLITE_REG_CIWDOFST_HOROFF_MASK |
		FLITE_REG_CIWDOFST_VEROFF_MASK);
	cfg |= FLITE_REG_CIWDOFST_WINOFSEN |
		FLITE_REG_CIWDOFST_WINHOROFST(f_frame->offs_h) |
		FLITE_REG_CIWDOFST_WINVEROFST(f_frame->offs_v);

	writel(cfg, flite_reg_base + FLITE_REG_CIWDOFST);

	hoff2 = f_frame->o_width - f_frame->width - f_frame->offs_h;
	voff2 = f_frame->o_height - f_frame->height - f_frame->offs_v;
	cfg = FLITE_REG_CIWDOFST2_WINHOROFST2(hoff2) |
		FLITE_REG_CIWDOFST2_WINVEROFST2(voff2);

	writel(cfg, flite_reg_base + FLITE_REG_CIWDOFST2);
}

static void flite_hw_set_last_capture_end_clear(unsigned long flite_reg_base)
{
	u32 cfg = 0;

	cfg = readl(flite_reg_base + FLITE_REG_CISTATUS2);
	cfg &= ~FLITE_REG_CISTATUS2_LASTCAPEND;

	writel(cfg, flite_reg_base + FLITE_REG_CISTATUS2);
}

int flite_hw_get_present_frame_buffer(unsigned long flite_reg_base)
{
	u32 status = 0;

	status = readl(flite_reg_base + FLITE_REG_CISTATUS3);
	status &= FLITE_REG_CISTATUS3_PRESENT_MASK;

	return status;
}

int flite_hw_get_status2(unsigned long flite_reg_base)
{
	u32 status = 0;

	status = readl(flite_reg_base + FLITE_REG_CISTATUS2);

	return status;
}

void flite_hw_set_status1(unsigned long flite_reg_base, u32 val)
{
	writel(val, flite_reg_base + FLITE_REG_CISTATUS);
}

int flite_hw_get_status1(unsigned long flite_reg_base)
{
	u32 status = 0;

	status = readl(flite_reg_base + FLITE_REG_CISTATUS);

	return status;
}

int flite_hw_getnclr_status1(unsigned long flite_reg_base)
{
	u32 status = 0;

	status = readl(flite_reg_base + FLITE_REG_CISTATUS);
	writel(0, flite_reg_base + FLITE_REG_CISTATUS);

	return status;
}

void flite_hw_set_status2(unsigned long flite_reg_base, u32 val)
{
	writel(val, flite_reg_base + FLITE_REG_CISTATUS2);
}

void flite_hw_set_start_addr(unsigned long flite_reg_base, u32 number, u32 addr)
{
	u32 target;
	if (number == 0) {
		target = flite_reg_base + 0x30;
	} else if (number >= 1) {
		number--;
		target = flite_reg_base + 0x200 + (0x4*number);
	} else
		target = 0;

	writel(addr, target);
}

void flite_hw_set_use_buffer(unsigned long flite_reg_base, u32 number)
{
	u32 buffer;
	buffer = readl(flite_reg_base + FLITE_REG_CIFCNTSEQ);
	buffer |= 1<<number;
	writel(buffer, flite_reg_base + FLITE_REG_CIFCNTSEQ);
}

void flite_hw_set_unuse_buffer(unsigned long flite_reg_base, u32 number)
{
	u32 buffer;
	buffer = readl(flite_reg_base + FLITE_REG_CIFCNTSEQ);
	buffer &= ~(1<<number);
	writel(buffer, flite_reg_base + FLITE_REG_CIFCNTSEQ);
}

u32 flite_hw_get_buffer_seq(unsigned long flite_reg_base)
{
	u32 buffer;
	buffer = readl(flite_reg_base + FLITE_REG_CIFCNTSEQ);
	return buffer;
}

int init_fimc_lite(unsigned long mipi_reg_base)
{
	int i;

	writel(0, mipi_reg_base + FLITE_REG_CIFCNTSEQ);

	for (i = 0; i < 32; i++)
		flite_hw_set_start_addr(mipi_reg_base , i, 0xffffffff);

	return 0;
}

int start_fimc_lite(unsigned long mipi_reg_base,
	struct fimc_is_frame_info *f_frame)
{
	flite_hw_set_cam_channel(mipi_reg_base);
	flite_hw_set_cam_source_size(mipi_reg_base, f_frame);
	flite_hw_set_camera_type(mipi_reg_base);
	flite_hw_set_source_format(mipi_reg_base);
	/*flite_hw_set_output_dma(mipi_reg_base, false);
	flite_hw_set_output_local(base_reg, false);*/

	flite_hw_set_interrupt_source(mipi_reg_base);
	/*flite_hw_set_interrupt_starten0_disable(mipi_reg_base);*/
	flite_hw_set_config_irq(mipi_reg_base);
	flite_hw_set_window_offset(mipi_reg_base, f_frame);
	/* flite_hw_set_test_pattern_enable(); */

	flite_hw_set_last_capture_end_clear(mipi_reg_base);
	flite_hw_set_capture_start(mipi_reg_base);

	/*dbg_front("lite config : %08X\n",
		*((unsigned int*)(base_reg + FLITE_REG_CIFCNTSEQ)));*/

	return 0;
}

int stop_fimc_lite(unsigned long mipi_reg_base)
{
	flite_hw_set_capture_stop(mipi_reg_base);
	return 0;
}

static void wq_func_automode(struct work_struct *data)
{
	struct fimc_is_device_flite *flite;
	struct fimc_is_device_sensor *sensor;
	struct fimc_is_device_ischain *ischain;

	flite = container_of(data, struct fimc_is_device_flite, work_queue);
	sensor = (struct fimc_is_device_sensor *)flite->private_data;
	ischain = sensor->ischain;

	fimc_is_ischain_isp_buffer_queue(ischain, flite->work);
}

static void tasklet_func_flite_str(unsigned long data)
{
	struct fimc_is_device_flite *flite;
	struct fimc_is_device_sensor *sensor;
	struct fimc_is_device_ischain *ischain;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_frame_shot *frame;
	u32 bstart;
	u32 fcount;

	flite = (struct fimc_is_device_flite *)data;
	sensor = (struct fimc_is_device_sensor *)flite->private_data;
	ischain = sensor->ischain;
	framemgr = flite->framemgr;

	bstart = flite->tasklet_param_str;
	fcount = atomic_read(&flite->fcount);

#ifdef DBG_STREAMING
	printk(KERN_INFO "S%d %d\n", bstart, fcount);
#endif

	framemgr_e_barrier(framemgr, FMGR_IDX_0 + bstart);

	fimc_is_frame_process_head(framemgr, &frame);
	if (frame) {
#ifdef MEASURE_TIME
#ifndef INTERNAL_TIME
		do_gettimeofday(&frame->tzone[TM_FLITE_STR]);
#endif
#endif
		frame->fcount = fcount;
		fimc_is_ischain_camctl(ischain, frame, fcount);
		fimc_is_ischain_tag(ischain, frame);
	} else {
		fimc_is_ischain_camctl(ischain, NULL, fcount);

#ifdef TASKLET_MSG
		err("process shot is empty");
#endif
		fimc_is_frame_print_all(framemgr);
	}

	framemgr_x_barrier(framemgr, FMGR_IDX_0 + bstart);
}

static void tasklet_func_flite_end(unsigned long data)
{
	struct fimc_is_device_flite *flite;
	struct fimc_is_device_sensor *sensor;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_frame_shot *frame;
	u32 index, bdone;
	u32 fcount;

	flite = (struct fimc_is_device_flite *)data;
	sensor = (struct fimc_is_device_sensor *)flite->private_data;
	framemgr = flite->framemgr;
	bdone = flite->tasklet_param_end;

	fcount = atomic_read(&flite->fcount);

#ifdef DBG_STREAMING
	printk(KERN_INFO "E%d %d\n", bdone, fcount);
#endif

	framemgr_e_barrier(framemgr, FMGR_IDX_1 + bdone);
	spin_lock(&flite->slock_state);

	if (test_bit(bdone, &flite->state)) {
		fimc_is_frame_process_head(framemgr, &frame);
		if (frame) {
#ifdef MEASURE_TIME
#ifndef INTERNAL_TIME
			do_gettimeofday(&frame->tzone[TM_FLITE_END]);
#endif
#endif
			index = frame->index;
			fimc_is_frame_trans_pro_to_com(framemgr, frame);

#ifdef AUTO_MODE
			if (!work_pending(&flite->work_queue)) {
				flite->work = index;
				schedule_work(&flite->work_queue);
			} else {
				struct fimc_is_device_ischain *ischain;
				ischain = sensor->ischain;

				frame->shot_ext->request_scc = 0;
				frame->shot_ext->request_scp = 0;
				buffer_done(flite->video, index);
				err("work pending, %d frame drop", index);
				fimc_is_frame_print_all(ischain->framemgr);
			}
#else
			buffer_done(flite->video, index);
#endif
			fimc_is_frame_request_head(framemgr, &frame);
			if (frame) {
				flite_hw_set_start_addr(flite->regs, bdone,
					frame->dvaddr_buffer[0]);
				set_bit(bdone, &flite->state);
				fimc_is_frame_trans_req_to_pro(framemgr, frame);
			} else {
				flite_hw_set_unuse_buffer(flite->regs, bdone);
				clear_bit(bdone, &flite->state);
#ifdef TASKLET_MSG
				err("request shot is empty0(%d slot)", bdone);
#endif
				fimc_is_frame_print_all(framemgr);

				/*this is debugging ponit for deadlock*/
				/*
				fimc_is_ischain_print_status(sensor->ischain);
				*/
			}
		} else {
#ifdef TASKLET_MSG
			err("process shot is empty(state is invalid(%d, %ld))",
				bdone, flite->state);
#endif
			fimc_is_frame_print_all(framemgr);
		}
	} else {
		fimc_is_frame_request_head(framemgr, &frame);
		if (frame) {
			flite_hw_set_start_addr(flite->regs, bdone,
				frame->dvaddr_buffer[0]);
			flite_hw_set_use_buffer(flite->regs, bdone);
			set_bit(bdone, &flite->state);
			fimc_is_frame_trans_req_to_pro(framemgr, frame);
		} else {
#ifdef TASKLET_MSG
			err("request shot is empty1(%d slot)", bdone);
#endif
			fimc_is_frame_print_all(framemgr);
		}
	}

	spin_unlock(&flite->slock_state);
	framemgr_x_barrier(framemgr, FMGR_IDX_1 + bdone);
}

static irqreturn_t fimc_is_flite_irq_handler(int irq, void *data)
{
	u32 status, status1, status2;
	struct fimc_is_device_flite *flite;

	flite = data;
	status1 = flite_hw_getnclr_status1(flite->regs);
	status = status1 & (3<<4);
	/* status3 = flite_hw_get_present_frame_buffer(flite->regs); */

	if (test_bit(FIMC_IS_FLITE_LAST_CAPTURE, &flite->state))
		goto exit;

	if (status) {
		if (status == (3<<4)) {
#ifdef DBG_FLITEISR
			printk(KERN_CONT "*");
#endif
			/* frame both interrupt since latency */
			if (flite->sw_checker) {
#ifdef DBG_FLITEISR
				printk(KERN_CONT ">");
#endif
				/* frame end interrupt */
				flite->sw_checker = EXPECT_FRAME_START;
				*notify_fcount = atomic_read(&flite->fcount);
				flite->tasklet_param_end = flite->sw_trigger;
				tasklet_schedule(&flite->tasklet_flite_end);

#ifdef DBG_FLITEISR
				printk(KERN_CONT "<");
#endif
				/* frame start interrupt */
				flite->sw_checker = EXPECT_FRAME_END;
				if (flite->sw_trigger)
					flite->sw_trigger = FLITE_A_SLOT_VALID;
				else
					flite->sw_trigger = FLITE_B_SLOT_VALID;
				flite->tasklet_param_str = flite->sw_trigger;
				atomic_inc(&flite->fcount);
				tasklet_schedule(&flite->tasklet_flite_str);
			} else {
#ifdef DBG_FLITEISR
				printk(KERN_CONT "<");
#endif
				/* frame start interrupt */
				flite->sw_checker = EXPECT_FRAME_END;
				if (flite->sw_trigger)
					flite->sw_trigger = FLITE_A_SLOT_VALID;
				else
					flite->sw_trigger = FLITE_B_SLOT_VALID;
				flite->tasklet_param_str = flite->sw_trigger;
				atomic_inc(&flite->fcount);
				tasklet_schedule(&flite->tasklet_flite_str);
#ifdef DBG_FLITEISR
				printk(KERN_CONT ">");
#endif
				/* frame end interrupt */
				flite->sw_checker = EXPECT_FRAME_START;
				*notify_fcount = atomic_read(&flite->fcount);
				flite->tasklet_param_end = flite->sw_trigger;
				tasklet_schedule(&flite->tasklet_flite_end);
			}
		} else if (status == (2<<4)) {
#ifdef DBG_FLITEISR
			printk(KERN_CONT "<");
#endif
			/* frame start interrupt */
			flite->sw_checker = EXPECT_FRAME_END;
			if (flite->sw_trigger)
				flite->sw_trigger = FLITE_A_SLOT_VALID;
			else
				flite->sw_trigger = FLITE_B_SLOT_VALID;
			flite->tasklet_param_str = flite->sw_trigger;
			atomic_inc(&flite->fcount);
			tasklet_schedule(&flite->tasklet_flite_str);
		} else {
#ifdef DBG_FLITEISR
			printk(KERN_CONT ">");
#endif
			/* frame end interrupt */
			flite->sw_checker = EXPECT_FRAME_START;
			*notify_fcount = atomic_read(&flite->fcount);
			flite->tasklet_param_end = flite->sw_trigger;
			tasklet_schedule(&flite->tasklet_flite_end);
		}
	}

	if (status1 & (1<<6)) {
		/* Last Frame Capture Interrupt */
		printk(KERN_INFO "[CamIF_0]Last Frame Capture\n");

		/* Clear LastCaptureEnd bit */
		status2 = flite_hw_get_status2(flite->regs);
		status2 &= ~(0x1 << 1);
		flite_hw_set_status2(flite->regs, status2);

		/* Notify last capture */
		set_bit(FIMC_IS_FLITE_LAST_CAPTURE, &flite->state);
		wake_up(&flite->wait_queue);
	}

	if (status1 & (1<<8)) {
		u32 ciwdofst;

		err("[CamIF_0]Overflow Cr\n");
		ciwdofst = readl(flite->regs + 0x10);
		ciwdofst  |= (0x1 << 14);
		writel(ciwdofst, flite->regs + 0x10);
		ciwdofst  &= ~(0x1 << 14);
		writel(ciwdofst, flite->regs + 0x10);
		/*uCIWDOFST |= (0x1 << 14);*/
	}

	if (status1 & (1<<9)) {
		u32 ciwdofst;

		err("[CamIF_0]Overflow Cb\n");
		ciwdofst = readl(flite->regs + 0x10);
		ciwdofst  |= (0x1 << 15);
		writel(ciwdofst, flite->regs + 0x10);
		ciwdofst  &= ~(0x1 << 15);
		writel(ciwdofst, flite->regs + 0x10);
		/*uCIWDOFST |= (0x1 << 15);*/
	}

	if (status1 & (1<<10)) {
		u32 ciwdofst;

		err("[CamIF_0]Overflow Y\n");
		ciwdofst = readl(flite->regs + 0x10);
		ciwdofst  |= (0x1 << 30);
		writel(ciwdofst, flite->regs + 0x10);
		ciwdofst  &= ~(0x1 << 30);
		writel(ciwdofst, flite->regs + 0x10);
		/*uCIWDOFST |= (0x1 << 30);*/
	}

exit:
	return IRQ_HANDLED;
}


int fimc_is_flite_probe(struct fimc_is_device_flite *this,
	struct fimc_is_video_common *video,
	struct fimc_is_framemgr *framemgr,
	u32 channel,
	u32 data)
{
	int ret = 0;

	this->channel = channel;
	this->framemgr = framemgr;
	this->video = video;
	this->private_data = data;
	spin_lock_init(&this->slock_state);
	init_waitqueue_head(&this->wait_queue);

	tasklet_init(&this->tasklet_flite_str,
		tasklet_func_flite_str,
		(unsigned long)this);

	tasklet_init(&this->tasklet_flite_end,
		tasklet_func_flite_end,
		(unsigned long)this);

	if (channel == FLITE_ID_A) {
		this->regs = (unsigned long)S5P_VA_FIMCLITE0;

		ret = request_irq(IRQ_FIMC_LITE0,
			fimc_is_flite_irq_handler,
			0,
			"fimc-lite0",
			this);
		if (ret)
			err("request_irq(L0) failed\n");
	} else if (channel == FLITE_ID_B) {
		this->regs = (unsigned long)S5P_VA_FIMCLITE1;

		ret = request_irq(IRQ_FIMC_LITE1,
			fimc_is_flite_irq_handler,
			0,
			"fimc-lite1",
			this);
		if (ret)
			err("request_irq(L1) failed\n");
	} else
		err("unresolved channel input");

	INIT_WORK(&this->work_queue, wq_func_automode);

	return ret;
}

int fimc_is_flite_open(struct fimc_is_device_flite *this)
{
	int ret = 0;

	atomic_set(&this->fcount, 0);

	clear_bit(FIMC_IS_FLITE_LAST_CAPTURE, &this->state);
	clear_bit(FLITE_A_SLOT_VALID, &this->state);
	clear_bit(FLITE_B_SLOT_VALID, &this->state);

	return ret;
}

int fimc_is_flite_start(struct fimc_is_device_flite *this,
	struct fimc_is_frame_info *frame,
	struct fimc_is_video_common *video)
{
	int ret = 0;
	unsigned long flags;
	struct fimc_is_frame_shot *item;
	struct fimc_is_framemgr *framemgr;

	framemgr = this->framemgr;

	this->sw_trigger = FLITE_B_SLOT_VALID;
	this->sw_checker = EXPECT_FRAME_START;
	this->tasklet_param_str = 0;
	this->tasklet_param_end = 0;

	clear_bit(FIMC_IS_FLITE_LAST_CAPTURE, &this->state);
	init_fimc_lite(this->regs);

	framemgr_e_barrier_irqs(framemgr, 0, flags);

	flite_hw_set_use_buffer(this->regs, 0);
	flite_hw_set_start_addr(this->regs, 0, video->buf_dva[0][0]);
	set_bit(FLITE_A_SLOT_VALID, &this->state);
	fimc_is_frame_request_head(framemgr, &item);
	fimc_is_frame_trans_req_to_pro(framemgr, item);

	flite_hw_set_use_buffer(this->regs, 1);
	flite_hw_set_start_addr(this->regs, 1, video->buf_dva[1][0]);
	set_bit(FLITE_B_SLOT_VALID, &this->state);
	fimc_is_frame_request_head(framemgr, &item);
	fimc_is_frame_trans_req_to_pro(framemgr, item);

	framemgr_x_barrier_irqr(framemgr, 0, flags);

	/*flite_hw_set_use_buffer(this->regs, 0);*/
	flite_hw_set_output_dma(this->regs, true);
	flite_hw_set_output_local(this->regs, false);

	start_fimc_lite(this->regs, frame);

	return ret;
}

int fimc_is_flite_stop(struct fimc_is_device_flite *this)
{
	int ret = 0;

	stop_fimc_lite(this->regs);

	dbg_back("waiting last capture\n");
	ret = wait_event_timeout(this->wait_queue,
		test_bit(FIMC_IS_FLITE_LAST_CAPTURE, &this->state),
		FIMC_IS_FLITE_STOP_TIMEOUT);
	if (!ret) {
		/* forcely stop */
		err("last capture timeout:%s\n", __func__);
		stop_fimc_lite(this->regs);
		msleep(60);
		set_bit(FIMC_IS_FLITE_LAST_CAPTURE, &this->state);
		ret = -ETIME;
	} else
		ret = 0;

	return ret;
}
