/* arch/arm/plat-samsung/dev-dwc3-exynos.c
 *
 * Copyright (c) 2011 Samsung Electronics Co. Ltd
 * Author: Anton Tikhomirov <av.tikhomirov@samsung.com>
 *
 * Device definition for DWC EXYNOS SuperSpeed USB 3.0 DRD Controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/exynos_usb3_drd.h>

#include <mach/irqs.h>
#include <mach/map.h>

#include <plat/devs.h>
#include <plat/usb-phy.h>

static struct resource exynos_ss_udc_resources[] = {
	[0] = DEFINE_RES_MEM(EXYNOS5_PA_SS_DRD, SZ_64K),
	[1] = DEFINE_RES_IRQ(IRQ_USB3_DRD),
};

static u64 exynos_ss_udc_dmamask = DMA_BIT_MASK(32);

struct platform_device exynos_device_ss_udc = {
	.name		= "exynos-ss-udc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(exynos_ss_udc_resources),
	.resource	= exynos_ss_udc_resources,
	.dev		= {
		.dma_mask		= &exynos_ss_udc_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

void __init exynos_ss_udc_set_platdata(struct exynos_usb3_drd_pdata *pd)
{
	struct exynos_usb3_drd_pdata *npd;

	npd = s3c_set_platdata(pd, sizeof(struct exynos_usb3_drd_pdata),
			&exynos_device_ss_udc);

	if (!npd->phy_init)
		npd->phy_init = s5p_usb_phy_init;
	if (!npd->phy_exit)
		npd->phy_exit = s5p_usb_phy_exit;
}
