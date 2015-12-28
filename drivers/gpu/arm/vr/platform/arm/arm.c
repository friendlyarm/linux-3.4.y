/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2009-2010, 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_platform.c
 * Platform specific Vr driver functions for:
 * - Realview Versatile platforms with ARM11 Mpcore and virtex 5.
 * - Versatile Express platforms with ARM Cortex-A9 and virtex 6.
 */
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/pm.h>
#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif
#include <asm/io.h>
#include <linux/vr/vr_utgard.h>
#include "vr_kernel_common.h"
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>

#include "arm_core_scaling.h"
#include "vr_pp_scheduler.h"

static void vr_platform_device_release(struct device *device);
static u32 vr_read_phys(u32 phys_addr);
#if defined(CONFIG_ARCH_REALVIEW)
static void vr_write_phys(u32 phys_addr, u32 value);
#endif

static int vr_core_scaling_enable = 1;

void vr_gpu_utilization_callback(struct vr_gpu_utilization_data *data);

#if defined(CONFIG_ARCH_VEXPRESS)

static struct resource vr_gpu_resources_m450_mp8[] = {
	VR_GPU_RESOURCES_VR450_MP8_PMU(0xFC040000, -1, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 68)
};

#elif defined(CONFIG_ARCH_REALVIEW)

static struct resource vr_gpu_resources_m300[] = {
	VR_GPU_RESOURCES_VR300_PMU(0xC0000000, -1, -1, -1, -1)
};

static struct resource vr_gpu_resources_m400_mp1[] = {
	VR_GPU_RESOURCES_VR400_MP1_PMU(0xC0000000, -1, -1, -1, -1)
};

static struct resource vr_gpu_resources_m400_mp2[] = {
	VR_GPU_RESOURCES_VR400_MP2_PMU(0xC0000000, -1, -1, -1, -1, -1, -1)
};

#endif

static struct vr_gpu_device_data vr_gpu_data = {
#if defined(CONFIG_ARCH_VEXPRESS)
	.shared_mem_size =256 * 1024 * 1024, /* 256MB */
#elif defined(CONFIG_ARCH_REALVIEW)
	.dedicated_mem_start = 0x80000000, /* Physical start address (use 0xD0000000 for old indirect setup) */
	.dedicated_mem_size = 0x10000000, /* 256MB */
#endif
	.fb_start = 0xe0000000,
	.fb_size = 0x01000000,
	.max_job_runtime = 60000, /* 60 seconds */
	.utilization_interval = 1000, /* 1000ms */
	.utilization_callback = vr_gpu_utilization_callback,
	.pmu_switch_delay = 0xFF, /* do not have to be this high on FPGA, but it is good for testing to have a delay */
	.pmu_domain_config = {0x1, 0x2, 0x4, 0x4, 0x4, 0x8, 0x8, 0x8, 0x8, 0x1, 0x2, 0x8},
};

static struct platform_device vr_gpu_device = {
	.name = VR_GPU_NAME_UTGARD,
	.id = 0,
	.dev.release = vr_platform_device_release,
	.dev.coherent_dma_mask = DMA_BIT_MASK(32),

	.dev.platform_data = &vr_gpu_data,
};

int vr_platform_device_register(void)
{
	int err = -1;
	int num_pp_cores = 0;
#if defined(CONFIG_ARCH_REALVIEW)
	u32 m400_gp_version;
#endif

	VR_DEBUG_PRINT(4, ("vr_platform_device_register() called\n"));

	/* Detect present Vr GPU and connect the correct resources to the device */
#if defined(CONFIG_ARCH_VEXPRESS)

	if (vr_read_phys(0xFC020000) == 0x00010100) {
		VR_DEBUG_PRINT(4, ("Registering Vr-450 MP8 device\n"));
		num_pp_cores = 8;
		vr_gpu_device.num_resources = ARRAY_SIZE(vr_gpu_resources_m450_mp8);
		vr_gpu_device.resource = vr_gpu_resources_m450_mp8;
	}

#elif defined(CONFIG_ARCH_REALVIEW)

	m400_gp_version = vr_read_phys(0xC000006C);
	if ((m400_gp_version & 0xFFFF0000) == 0x0C070000) {
		VR_DEBUG_PRINT(4, ("Registering Vr-300 device\n"));
		num_pp_cores = 1;
		vr_gpu_device.num_resources = ARRAY_SIZE(vr_gpu_resources_m300);
		vr_gpu_device.resource = vr_gpu_resources_m300;
		vr_write_phys(0xC0010020, 0xA); /* Enable direct memory mapping for FPGA */
	} else if ((m400_gp_version & 0xFFFF0000) == 0x0B070000) {
		u32 fpga_fw_version = vr_read_phys(0xC0010000);
		if (fpga_fw_version == 0x130C008F || fpga_fw_version == 0x110C008F) {
			/* Vr-400 MP1 r1p0 or r1p1 */
			VR_DEBUG_PRINT(4, ("Registering Vr-400 MP1 device\n"));
			num_pp_cores = 1;
			vr_gpu_device.num_resources = ARRAY_SIZE(vr_gpu_resources_m400_mp1);
			vr_gpu_device.resource = vr_gpu_resources_m400_mp1;
			vr_write_phys(0xC0010020, 0xA); /* Enable direct memory mapping for FPGA */
		} else if (fpga_fw_version == 0x130C000F) {
			/* Vr-400 MP2 r1p1 */
			VR_DEBUG_PRINT(4, ("Registering Vr-400 MP2 device\n"));
			num_pp_cores = 2;
			vr_gpu_device.num_resources = ARRAY_SIZE(vr_gpu_resources_m400_mp2);
			vr_gpu_device.resource = vr_gpu_resources_m400_mp2;
			vr_write_phys(0xC0010020, 0xA); /* Enable direct memory mapping for FPGA */
		}
	}

#endif
	/* Register the platform device */
	err = platform_device_register(&vr_gpu_device);
	if (0 == err) {
#ifdef CONFIG_PM_RUNTIME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		pm_runtime_set_autosuspend_delay(&(vr_gpu_device.dev), 1000);
		pm_runtime_use_autosuspend(&(vr_gpu_device.dev));
#endif
		pm_runtime_enable(&(vr_gpu_device.dev));
#endif
		VR_DEBUG_ASSERT(0 < num_pp_cores);
		vr_core_scaling_init(num_pp_cores);

		return 0;
	}

	return err;
}

void vr_platform_device_unregister(void)
{
	VR_DEBUG_PRINT(4, ("vr_platform_device_unregister() called\n"));

	vr_core_scaling_term();
	platform_device_unregister(&vr_gpu_device);

	platform_device_put(&vr_gpu_device);

#if defined(CONFIG_ARCH_REALVIEW)
	vr_write_phys(0xC0010020, 0x9); /* Restore default (legacy) memory mapping */
#endif
}

static void vr_platform_device_release(struct device *device)
{
	VR_DEBUG_PRINT(4, ("vr_platform_device_release() called\n"));
}

static u32 vr_read_phys(u32 phys_addr)
{
	u32 phys_addr_page = phys_addr & 0xFFFFE000;
	u32 phys_offset    = phys_addr & 0x00001FFF;
	u32 map_size       = phys_offset + sizeof(u32);
	u32 ret = 0xDEADBEEF;
	void *mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped) {
		ret = (u32)ioread32(((u8*)mem_mapped) + phys_offset);
		iounmap(mem_mapped);
	}

	return ret;
}

#if defined(CONFIG_ARCH_REALVIEW)
static void vr_write_phys(u32 phys_addr, u32 value)
{
	u32 phys_addr_page = phys_addr & 0xFFFFE000;
	u32 phys_offset    = phys_addr & 0x00001FFF;
	u32 map_size       = phys_offset + sizeof(u32);
	void *mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped) {
		iowrite32(value, ((u8*)mem_mapped) + phys_offset);
		iounmap(mem_mapped);
	}
}
#endif

static int param_set_core_scaling(const char *val, const struct kernel_param *kp)
{
	int ret = param_set_int(val, kp);

	if (1 == vr_core_scaling_enable) {
		vr_core_scaling_sync(vr_pp_scheduler_get_num_cores_enabled());
	}
	return ret;
}

static struct kernel_param_ops param_ops_core_scaling = {
	.set = param_set_core_scaling,
	.get = param_get_int,
};

module_param_cb(vr_core_scaling_enable, &param_ops_core_scaling, &vr_core_scaling_enable, 0644);
MODULE_PARM_DESC(vr_core_scaling_enable, "1 means to enable core scaling policy, 0 means to disable core scaling policy");

void vr_gpu_utilization_callback(struct vr_gpu_utilization_data *data)
{
	if (1 == vr_core_scaling_enable) {
		vr_core_scaling_update(data);
	}
}
