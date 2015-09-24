/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - ABB(Adaptive Body Bias) control
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <plat/cpu.h>

#include <mach/map.h>
#include <mach/regs-pmu.h>
#include <mach/abb-exynos.h>

void set_abb_member(enum abb_member abb_target, unsigned int abb_mode_value)
{
	unsigned int tmp;

	if (abb_mode_value != ABB_MODE_BYPASS) {
		tmp = EXYNOS_ABB_INIT;
		tmp |= abb_mode_value;
	} else {
		tmp = EXYNOS_ABB_INIT_BYPASS;
	}

	if (!soc_is_exynos5250()) {
		switch (abb_target) {
		case ABB_INT:
			__raw_writel(tmp, EXYNOS4_ABB_INT);
			break;
		case ABB_ARM:
			__raw_writel(tmp, EXYNOS4_ABB_ARM);
			break;
		case ABB_G3D:
			__raw_writel(tmp, EXYNOS4_ABB_G3D);
			break;
		case ABB_MIF:
			__raw_writel(tmp, EXYNOS4_ABB_MIF);
			break;
		default:
			break;
		}
	} else {
		switch (abb_target) {
		case ABB_INT:
			__raw_writel(tmp, EXYNOS5_ABB_INT);
			break;
		case ABB_ARM:
			__raw_writel(tmp, EXYNOS5_ABB_ARM);
			break;
		case ABB_G3D:
			__raw_writel(tmp, EXYNOS5_ABB_G3D);
			break;
		case ABB_MIF:
			__raw_writel(tmp, EXYNOS5_ABB_MIF);
			break;
		default:
			break;
		}
	}
}
