/* linux/arch/arm/mach-exynos/setup-gsc.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com
 *
 * Base Exynos5 G-Scaler clock configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <mach/regs-clock.h>
#include <mach/map.h>
#include <media/exynos_gscaler.h>

void __init exynos5_gsc_set_pdev_name(int id, char *name)
{
	switch (id) {
	case 0:
		exynos5_device_gsc0.name = name;
		break;
	case 1:
		exynos5_device_gsc1.name = name;
		break;
	case 2:
		exynos5_device_gsc2.name = name;
		break;
	case 3:
		exynos5_device_gsc3.name = name;
		break;
	}
}
