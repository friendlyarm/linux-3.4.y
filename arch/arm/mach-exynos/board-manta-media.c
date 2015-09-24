/*
 * Copyright (C) 2012 Google, Inc.
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/errno.h>
#include <linux/platform_device.h>
#include <media/exynos_gscaler.h>

#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/fimg2d.h>
#include <plat/jpeg.h>

#include <mach/exynos-ion.h>
#include <mach/exynos-mfc.h>
#include <mach/sysmmu.h>

static struct platform_device *media_devices[] __initdata = {
	&s5p_device_mfc,
	&exynos5_device_gsc0,
	&exynos5_device_gsc1,
	&exynos5_device_gsc2,
	&exynos5_device_gsc3,
	&s5p_device_jpeg,
	&s5p_device_fimg2d,
};

static struct s5p_mfc_platdata manta_mfc_pd = {
	.clock_rate = 333 * MHZ,
};

static struct fimg2d_platdata fimg2d_data __initdata = {
	.hw_ver		= 0x42,
	.gate_clkname	= "fimg2d",
};

static void __init manta_media_sysmmu_init(void)
{
	platform_set_sysmmu(&SYSMMU_PLATDEV(mfc_lr).dev, &s5p_device_mfc.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(gsc0).dev,
			    &exynos5_device_gsc0.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(gsc1).dev,
			    &exynos5_device_gsc1.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(gsc2).dev,
			    &exynos5_device_gsc2.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(gsc3).dev,
			    &exynos5_device_gsc3.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(jpeg).dev,
			    &s5p_device_jpeg.dev);
	platform_set_sysmmu(&SYSMMU_PLATDEV(2d).dev,
			    &s5p_device_fimg2d.dev);
}

void __init exynos5_manta_media_init(void)
{
	manta_media_sysmmu_init();
	s5p_mfc_set_platdata(&manta_mfc_pd);

	s3c_set_platdata(&exynos_gsc0_default_data,
			 sizeof(exynos_gsc0_default_data),
			 &exynos5_device_gsc0);
	s3c_set_platdata(&exynos_gsc1_default_data,
			 sizeof(exynos_gsc1_default_data),
			 &exynos5_device_gsc1);
	s3c_set_platdata(&exynos_gsc2_default_data,
			 sizeof(exynos_gsc2_default_data),
			 &exynos5_device_gsc2);
	s3c_set_platdata(&exynos_gsc3_default_data,
			 sizeof(exynos_gsc3_default_data),
			 &exynos5_device_gsc3);

	s5p_fimg2d_set_platdata(&fimg2d_data);
	exynos5_jpeg_setup_clock(&s5p_device_jpeg.dev, 150000000);

	platform_add_devices(media_devices, ARRAY_SIZE(media_devices));
}

