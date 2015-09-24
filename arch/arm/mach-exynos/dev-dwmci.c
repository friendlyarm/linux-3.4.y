/*
 * linux/arch/arm/mach-exynos/dev-dwmci.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Platform device for Synopsys DesignWare Mobile Storage IP
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/mmc/host.h>
#include <linux/clk.h>

#include <plat/devs.h>
#include <plat/cpu.h>

#include <mach/map.h>

#define DWMCI_CLKSEL	0x09c

static int exynos_dwmci_get_ocr(u32 slot_id)
{
	u32 ocr_avail = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;

	return ocr_avail;
}

static int exynos_dwmci_get_bus_wd(u32 slot_id)
{
	return 4;
}

static int exynos_dwmci_init(u32 slot_id, irq_handler_t handler, void *data)
{
	return 0;
}

static void exynos_dwmci_set_io_timing(void *data, unsigned char timing)
{
	struct dw_mci *host = (struct dw_mci *)data;
	u32 clksel;

	if (timing == MMC_TIMING_MMC_HS200 ||
			timing == MMC_TIMING_UHS_SDR104) {
		if (host->bus_hz != 200 * 1000 * 1000) {
			host->bus_hz = 200 * 1000 * 1000;
			clk_set_rate(host->cclk, 800 * 1000 * 1000);
		}
		clksel = __raw_readl(host->regs + DWMCI_CLKSEL);
		clksel = (clksel & 0xfff8ffff) | (host->pdata->clk_drv << 16);
		__raw_writel(clksel, host->regs + DWMCI_CLKSEL);
	} else if (timing == MMC_TIMING_UHS_SDR50) {
		if (host->bus_hz != 100 * 1000 * 1000) {
			host->bus_hz = 100 * 1000 * 1000;
			clk_set_rate(host->cclk, 400 * 1000 * 1000);
		}
		clksel = __raw_readl(host->regs + DWMCI_CLKSEL);
		clksel = (clksel & 0xfff8ffff) | (host->pdata->clk_drv << 16);
		__raw_writel(clksel, host->regs + DWMCI_CLKSEL);
	} else if (timing == MMC_TIMING_UHS_DDR50) {
		if (host->bus_hz != 100 * 1000 * 1000) {
			host->bus_hz = 100 * 1000 * 1000;
			clk_set_rate(host->cclk, 400 * 1000 * 1000);
			host->current_speed = 0;
		}
		__raw_writel(host->pdata->ddr_timing,
			host->regs + DWMCI_CLKSEL);
	} else {
		if (host->bus_hz != 50 * 1000 * 1000) {
			host->bus_hz = 50 * 1000 * 1000;
			clk_set_rate(host->cclk, 200 * 1000 * 1000);
		}
		__raw_writel(host->pdata->sdr_timing,
			host->regs + DWMCI_CLKSEL);
	}
}

static struct dw_mci_board exynos4_dwmci_pdata = {
	.num_slots		= 1,
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION,
	.bus_hz			= 80 * 1000 * 1000,
	.max_bus_hz		= 200 * 1000 * 1000,
	.detect_delay_ms	= 200,
	.init			= exynos_dwmci_init,
	.get_bus_wd		= exynos_dwmci_get_bus_wd,
	.set_io_timing		= exynos_dwmci_set_io_timing,
};

static u64 exynos_dwmci_dmamask = DMA_BIT_MASK(32);

static struct resource exynos4_dwmci_resources[] = {
	[0] = DEFINE_RES_MEM(EXYNOS4_PA_DWMCI, SZ_4K),
	[1] = DEFINE_RES_IRQ(EXYNOS4_IRQ_DWMCI),
};

struct platform_device exynos4_device_dwmci = {
	.name		= "dw_mmc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(exynos4_dwmci_resources),
	.resource	= exynos4_dwmci_resources,
	.dev		= {
		.dma_mask		= &exynos_dwmci_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &exynos4_dwmci_pdata,
	},
};


#define EXYNOS5_DWMCI_RESOURCE(_channel)			\
static struct resource exynos5_dwmci##_channel##_resource[] = {	\
	[0] = DEFINE_RES_MEM(S3C_PA_HSMMC##_channel, SZ_4K),	\
	[1] = DEFINE_RES_IRQ(IRQ_HSMMC##_channel),		\
}

EXYNOS5_DWMCI_RESOURCE(0);
EXYNOS5_DWMCI_RESOURCE(1);
EXYNOS5_DWMCI_RESOURCE(2);
EXYNOS5_DWMCI_RESOURCE(3);

#define EXYNOS5_DWMCI_DEF_PLATDATA(_channel)			\
struct dw_mci_board exynos5_dwmci##_channel##_def_platdata = {	\
	.num_slots		= 1,				\
	.quirks			=				\
		DW_MCI_QUIRK_BROKEN_CARD_DETECTION,		\
	.bus_hz			= 200 * 1000 * 1000,		\
	.max_bus_hz		= 200 * 1000 * 1000,		\
	.detect_delay_ms	= 200,				\
	.init			= exynos_dwmci_init,		\
	.get_bus_wd		= exynos_dwmci_get_bus_wd,	\
	.set_io_timing		= exynos_dwmci_set_io_timing,	\
	.get_ocr		= exynos_dwmci_get_ocr		\
}

EXYNOS5_DWMCI_DEF_PLATDATA(0);
EXYNOS5_DWMCI_DEF_PLATDATA(1);
EXYNOS5_DWMCI_DEF_PLATDATA(2);
EXYNOS5_DWMCI_DEF_PLATDATA(3);

#define EXYNOS5_DWMCI_PLATFORM_DEVICE(_channel)			\
struct platform_device exynos5_device_dwmci##_channel =		\
{								\
	.name		= "dw_mmc",				\
	.id		= _channel,				\
	.num_resources	=					\
	ARRAY_SIZE(exynos5_dwmci##_channel##_resource),		\
	.resource	= exynos5_dwmci##_channel##_resource,	\
	.dev		= {					\
		.dma_mask		= &exynos_dwmci_dmamask,\
		.coherent_dma_mask	= DMA_BIT_MASK(32),	\
		.platform_data		=			\
			&exynos5_dwmci##_channel##_def_platdata,\
	},							\
}

EXYNOS5_DWMCI_PLATFORM_DEVICE(0);
EXYNOS5_DWMCI_PLATFORM_DEVICE(1);
EXYNOS5_DWMCI_PLATFORM_DEVICE(2);
EXYNOS5_DWMCI_PLATFORM_DEVICE(3);

static struct platform_device *exynos5_dwmci_devs[] = {
	&exynos5_device_dwmci0,
	&exynos5_device_dwmci1,
	&exynos5_device_dwmci2,
	&exynos5_device_dwmci3,
};

void __init exynos_dwmci_set_platdata(struct dw_mci_board *pd, u32 slot_id)
{
	struct dw_mci_board *npd = NULL;

	if ((soc_is_exynos4210()) || soc_is_exynos4212() ||
		soc_is_exynos4412()) {
		npd = s3c_set_platdata(pd, sizeof(struct dw_mci_board),
				&exynos4_device_dwmci);
	} else if (soc_is_exynos5250()) {
		if (slot_id < ARRAY_SIZE(exynos5_dwmci_devs))
			npd = s3c_set_platdata(pd, sizeof(struct dw_mci_board),
					       exynos5_dwmci_devs[slot_id]);
		else
			pr_err("%s: slot %d is not supported\n", __func__,
			       slot_id);
	}

	if (!npd)
		return;

	if (!npd->init)
		npd->init = exynos_dwmci_init;
	if (!npd->get_bus_wd)
		npd->get_bus_wd = exynos_dwmci_get_bus_wd;
	if (!npd->set_io_timing)
		npd->set_io_timing = exynos_dwmci_set_io_timing;
	if (!npd->get_ocr)
		npd->get_ocr = exynos_dwmci_get_ocr;
}
