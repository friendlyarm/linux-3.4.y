/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - ASV(Adaptive Support Voltage) driver
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
#include <mach/asv-exynos.h>

static struct asv_common exynos_asv;

unsigned int asv_get_volt(enum asv_type_id target_type, unsigned int target_freq)
{
	if (exynos_asv.init_done)
		return exynos_asv.get_voltage(target_type, target_freq);

	return 0;
}

static int __init asv_init(void)
{
	int ret;

	if (soc_is_exynos5250()) {
		ret = exynos5250_init_asv(&exynos_asv);
	} else {
		pr_err("%s: Unknown SoC type\n", __func__);
		return -ENODEV;
	}

	if (ret)
		pr_err("%s: initialization failed\n", __func__);

	return ret;
}
device_initcall(asv_init);
