/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_kernel_linux.c
 * Implementation of the Linux device driver entrypoints
 */
#include <linux/module.h>   /* kernel module definitions */
#include <linux/fs.h>       /* file system operations */
#include <linux/cdev.h>     /* character device definitions */
#include <linux/mm.h>       /* memory manager definitions */
#include <linux/vr/vr_utgard_ioctl.h>
#include <linux/version.h>
#include <linux/device.h>
#include "vr_kernel_license.h"
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/vr/vr_utgard.h>
#include "vr_kernel_common.h"
#include "vr_session.h"
#include "vr_kernel_core.h"
#include "vr_osk.h"
#include "vr_kernel_linux.h"
#include "vr_ukk.h"
#include "vr_ukk_wrappers.h"
#include "vr_kernel_sysfs.h"
#include "vr_pm.h"
#include "vr_kernel_license.h"
#include "vr_memory.h"
#include "vr_memory_dma_buf.h"
#if defined(CONFIG_VR400_INTERNAL_PROFILING)
#include "vr_profiling_internal.h"
#endif

/* Streamline support for the Vr driver */
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_VR400_PROFILING)
/* Ask Linux to create the tracepoints */
#define CREATE_TRACE_POINTS
#include "vr_linux_trace.h"
#endif /* CONFIG_TRACEPOINTS */

/* from the __vrdrv_build_info.c file that is generated during build */
extern const char *__vrdrv_build_info(void);

/* Module parameter to control log level */
int vr_debug_level = 2;
module_param(vr_debug_level, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */
MODULE_PARM_DESC(vr_debug_level, "Higher number, more dmesg output");

extern int vr_max_job_runtime;
module_param(vr_max_job_runtime, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_max_job_runtime, "Maximum allowed job runtime in msecs.\nJobs will be killed after this no matter what");

extern int vr_l2_max_reads;
module_param(vr_l2_max_reads, int, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_l2_max_reads, "Maximum reads for Vr L2 cache");

extern unsigned int vr_dedicated_mem_start;
module_param(vr_dedicated_mem_start, uint, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_dedicated_mem_start, "Physical start address of dedicated Vr GPU memory.");

extern unsigned int vr_dedicated_mem_size;
module_param(vr_dedicated_mem_size, uint, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_dedicated_mem_size, "Size of dedicated Vr GPU memory.");

extern unsigned int vr_shared_mem_size;
module_param(vr_shared_mem_size, uint, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_shared_mem_size, "Size of shared Vr GPU memory.");

#if defined(CONFIG_VR400_PROFILING)
extern int vr_boot_profiling;
module_param(vr_boot_profiling, int, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_boot_profiling, "Start profiling as a part of Vr driver initialization");
#endif

extern int vr_max_pp_cores_group_1;
module_param(vr_max_pp_cores_group_1, int, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_max_pp_cores_group_1, "Limit the number of PP cores to use from first PP group.");

extern int vr_max_pp_cores_group_2;
module_param(vr_max_pp_cores_group_2, int, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_max_pp_cores_group_2, "Limit the number of PP cores to use from second PP group (Vr-450 only).");

#if defined(CONFIG_VR400_POWER_PERFORMANCE_POLICY)
/** the max fps the same as display vsync default 60, can set by module insert parameter */
extern int vr_max_system_fps;
module_param(vr_max_system_fps, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_max_system_fps, "Max system fps the same as display VSYNC.");

/** a lower limit on their desired FPS default 58, can set by module insert parameter*/
extern int vr_desired_fps;
module_param(vr_desired_fps, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vr_desired_fps, "A bit lower than max_system_fps which user desired fps");
#endif

#if VR_ENABLE_CPU_CYCLES
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <asm/smp.h>
static struct timer_list vr_init_cpu_clock_timers[8];
static u32 vr_cpu_clock_last_value[8] = {0,};
#endif

/* Export symbols from common code: vr_user_settings.c */
#include "vr_user_settings_db.h"
EXPORT_SYMBOL(vr_set_user_setting);
EXPORT_SYMBOL(vr_get_user_setting);

static char vr_dev_name[] = "vr"; /* should be const, but the functions we call requires non-cost */

/* This driver only supports one Vr device, and this variable stores this single platform device */
struct platform_device *vr_platform_device = NULL;

/* This driver only supports one Vr device, and this variable stores the exposed misc device (/dev/vr) */
static struct miscdevice vr_miscdevice = { 0, };

static int vr_miscdevice_register(struct platform_device *pdev);
static void vr_miscdevice_unregister(void);

static int vr_open(struct inode *inode, struct file *filp);
static int vr_release(struct inode *inode, struct file *filp);
#ifdef HAVE_UNLOCKED_IOCTL
static long vr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int vr_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif

static int vr_probe(struct platform_device *pdev);
static int vr_remove(struct platform_device *pdev);

static int vr_driver_suspend_scheduler(struct device *dev);
static int vr_driver_resume_scheduler(struct device *dev);

#ifdef CONFIG_PM_RUNTIME
static int vr_driver_runtime_suspend(struct device *dev);
static int vr_driver_runtime_resume(struct device *dev);
static int vr_driver_runtime_idle(struct device *dev);
#endif

#if defined(VR_FAKE_PLATFORM_DEVICE)
extern int vr_platform_device_register(void);
extern int vr_platform_device_unregister(void);
#endif

/* Linux power management operations provided by the Vr device driver */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
struct pm_ext_ops vr_dev_ext_pm_ops = {
	.base =
	{
		.suspend = vr_driver_suspend_scheduler,
		.resume = vr_driver_resume_scheduler,
		.freeze = vr_driver_suspend_scheduler,
		.thaw =   vr_driver_resume_scheduler,
	},
};
#else
static const struct dev_pm_ops vr_dev_pm_ops = {
#ifdef CONFIG_PM_RUNTIME
	.runtime_suspend = vr_driver_runtime_suspend,
	.runtime_resume = vr_driver_runtime_resume,
	.runtime_idle = vr_driver_runtime_idle,
#endif
	.suspend = vr_driver_suspend_scheduler,
	.resume = vr_driver_resume_scheduler,
	.freeze = vr_driver_suspend_scheduler,
	.thaw = vr_driver_resume_scheduler,
	.poweroff = vr_driver_suspend_scheduler,
};
#endif

/* The Vr device driver struct */
static struct platform_driver vr_platform_driver = {
	.probe  = vr_probe,
	.remove = vr_remove,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
	.pm = &vr_dev_ext_pm_ops,
#endif
	.driver =
	{
		.name   = VR_GPU_NAME_UTGARD,
		.owner  = THIS_MODULE,
		.bus = &platform_bus_type,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
		.pm = &vr_dev_pm_ops,
#endif
	},
};

/* Linux misc device operations (/dev/vr) */
struct file_operations vr_fops = {
	.owner = THIS_MODULE,
	.open = vr_open,
	.release = vr_release,
#ifdef HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl = vr_ioctl,
#else
	.ioctl = vr_ioctl,
#endif
	.mmap = vr_mmap
};

/* NEXELL_FEATURE_PORTING */
//added by nexell
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/delay.h>	/* mdelay */

#include <asm/mach/time.h>
#include <asm/mach/irq.h>
#include <asm/smp_twd.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

//#define PHY_BASEADDR_VR_GP			(0xC0070000 + 0x0)
//#define PHY_BASEADDR_VR_MMU_GP		(0xC0070000 + 0x3000)
//#define PHY_BASEADDR_VR_MMU_PP0		(0xC0070000 + 0x4000)
//#define PHY_BASEADDR_VR_MMU_PP1		(0xC0070000 + 0x5000)
//#define PHY_BASEADDR_VR_PP0			(0xC0070000 + 0x8000)
//#define PHY_BASEADDR_VR_PP1			(0xC0070000 + 0xA000)
#define PHY_BASEADDR_VR_PMU					(0xC0070000 + 0x2000) //~ + 0x10
#define PHY_BASEADDR_VR_PMU_REG_SIZE		0x10
#define PHY_BASEADDR_PMU_ISOLATE			(0xC0010D00) //~ + 0x10
#define PHY_BASEADDR_PMU_ISOLATE_REG_SIZE	0x10
#define PHY_BASEADDR_POWER_GATE				(0xC0010800) //~ + 0x4
#define PHY_BASEADDR_POWER_GATE_REG_SIZE	0x4
#define PHY_BASEADDR_CLOCK_GATE				(0xC00C3000) //~ + 0x4
#define PHY_BASEADDR_CLOCK_GATE_REG_SIZE	0x4
#define PHY_BASEADDR_RESET  				(0xC0012000) //~ + 0xC
#define PHY_BASEADDR_RESET_REG_SIZE  		0xC
#ifdef CONFIG_ARCH_S5P6818
#define PHY_BASEADDR_LPI_ACTIVE				0xC001120C
#define PHY_BASEADDR_LPI_ACTIVE_REG_SIZE	0x4
#define PHY_BASEADDR_LPI_REQ				0xC0011114
#define PHY_BASEADDR_LPI_REQ_REG_SIZE		0x4
#endif

enum
{
	PHY_BASEADDR_VR_PMU_IDX,
	PHY_BASEADDR_PMU_ISOLATE_IDX,
	PHY_BASEADDR_POWER_GATE_IDX,
	PHY_BASEADDR_CLOCK_GATE_IDX,
	PHY_BASEADDR_RESET_IDX,
#ifdef CONFIG_ARCH_S5P6818
	PHY_BASEADDR_LPI_ACTIVE_IDX,
	PHY_BASEADDR_LPI_REQ_IDX,
#endif
	PHY_BASEADDR_IDX_MAX
};
typedef struct
{
	unsigned int reg_addr;
	unsigned int reg_size;
}VR_REG_MAPS;

static VR_REG_MAPS __g_VRRegPhysMaps[PHY_BASEADDR_IDX_MAX] = {
	{PHY_BASEADDR_VR_PMU, 		PHY_BASEADDR_VR_PMU_REG_SIZE},
	{PHY_BASEADDR_PMU_ISOLATE, 	PHY_BASEADDR_PMU_ISOLATE_REG_SIZE},
	{PHY_BASEADDR_POWER_GATE, 	PHY_BASEADDR_POWER_GATE_REG_SIZE},
	{PHY_BASEADDR_CLOCK_GATE, 	PHY_BASEADDR_CLOCK_GATE_REG_SIZE},
	{PHY_BASEADDR_RESET, 		PHY_BASEADDR_RESET_REG_SIZE},
#ifdef CONFIG_ARCH_S5P6818
	{PHY_BASEADDR_LPI_ACTIVE,	PHY_BASEADDR_LPI_ACTIVE_REG_SIZE},
	{PHY_BASEADDR_LPI_REQ,		PHY_BASEADDR_LPI_REQ_REG_SIZE},
#endif
};
static unsigned int __g_VRRegVirtMaps[PHY_BASEADDR_IDX_MAX];

#define POWER_DELAY_MS	100
#if 1
#define VR_DBG printk
#define VR_PM_DBG PM_DBGOUT
#define VR_IOCTL_DBG printk
#else
#define VR_DBG
#define VR_PM_DBG PM_DBGOUT
#define VR_IOCTL_DBG
#endif


#if 1 /* new code */
static void nx_vr_make_reg_virt_maps(void)
{
	int i;
	for(i = 0 ; i < PHY_BASEADDR_IDX_MAX ; i++)
	{
		__g_VRRegVirtMaps[i] = ioremap_nocache(__g_VRRegPhysMaps[i].reg_addr, __g_VRRegPhysMaps[i].reg_size);
		if(!__g_VRRegVirtMaps[i])
		{
			VR_PRINT(("ERROR! can't run 'ioremap_nocache()'\n"));
			break;
		}
	}
}

static void nx_vr_release_reg_virt_maps(void)
{
	int i;
	for(i = 0 ; i < PHY_BASEADDR_IDX_MAX ; i++)
	{
		iounmap(__g_VRRegVirtMaps[i]);
	}
}

#if defined( CONFIG_ARCH_S5P4418 )
static void nx_vr_power_down_all_s5p4418(void)
{
	u32 virt_addr_page;

	//reset
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_RESET_IDX] + 8;
	{
		unsigned int temp32 = ioread32(((u8*)virt_addr_page));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)virt_addr_page);

		temp32 &= ~bit_mask;
		iowrite32(temp32, ((u8*)virt_addr_page));
	}

	//clk disalbe
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_CLOCK_GATE_IDX];
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting ClockGen, set 0\n");
		iowrite32(read_val & ~0x3, ((u8*)virt_addr_page));
	}

	//ready
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_POWER_GATE_IDX];
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)virt_addr_page));
	}

	//enable ISolate
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX];
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 0\n");
		iowrite32(read_val & ~1, ((u8*)virt_addr_page));
	}

	//pre charge down
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 4;
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 1\n");
		iowrite32(read_val | 1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//powerdown
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 8;
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 1\n");
		iowrite32(read_val | 1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 0xC;
	do{
		unsigned int powerUpAck = ioread32((u8*)virt_addr_page);
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( (powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);
}

static void nx_vr_power_up_all_s5p4418(void)
{
	u32 virt_addr_page;

	//ready
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_POWER_GATE_IDX];
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)virt_addr_page));
	}

	//pre charge up
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 4;
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)virt_addr_page));
	}

	//power up
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 8;
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//disable ISolate
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX];
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 1\n");
		iowrite32(read_val | 1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 0xC;
	do{
		unsigned int powerUpAck = ioread32((u8*)virt_addr_page);
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( !(powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);

	//clk enable
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_CLOCK_GATE_IDX];
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting ClockGen, set 1\n");
		iowrite32(0x3 | read_val, ((u8*)virt_addr_page));
	}

	//reset release
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_RESET_IDX] + 8;
	{
		unsigned int temp32 = ioread32(((u8*)virt_addr_page));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)virt_addr_page);
		temp32 |= bit_mask;
		iowrite32(temp32, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//mask vr400 PMU interrupt
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_VR_PMU_IDX] + 0xC;
	{
		VR_PM_DBG("mask PMU INT, addr(0x%x)\n", (int)virt_addr_page);
		iowrite32(0x0, ((u8*)virt_addr_page));
	}

	//power up vr400
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_VR_PMU_IDX];
	{
		VR_DBG("setting PHY_BASEADDR_VR_PMU addr(0x%x)\n", (int)virt_addr_page);
		iowrite32(0xF/*GP, L2C, PP0, PP1*/, ((u8*)virt_addr_page));
	}
}
#endif
#if defined(CONFIG_ARCH_S5P6818)
static void nx_vr_power_down_enter_reset_s5p6818(void)
{
	u32 virt_addr_page, read32;

	//=========================
	// [ mali PBUS ]
	//=========================
	// wait until LPI ACTIVE HIGH
	//=========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_ACTIVE_IDX];
	//printk("LPI ACTIVE HIGH waitting start...\n");
	do{
		read32 = ioread32((u8*)virt_addr_page);
		if( (read32>>12) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACTIVE HIGH waitting done\n");

	//==========================
	// LPI REQ, Set LOW
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_REQ_IDX];
	read32 = ioread32((u8*)virt_addr_page);
	read32 = read32 & (~(1<<2)); // CSYSREQ LOW
	iowrite32(read32, ((u8*)virt_addr_page));

	//==========================
	// wait until LPI ACK LOW
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_ACTIVE_IDX];
	//printk("LPI ACK LOW waitting start...\n");
	do{
		read32 = ioread32((u8*)virt_addr_page);
		if( !((read32>>13) & 0x01) )
			break;
	}while(1);
	//printk("LPI ACK LOW waitting done\n");

	//=========================
	// [ mali MBUS ]
	//=========================
	// wait until LPI ACTIVE HIGH
	//=========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_ACTIVE_IDX];
	//printk("LPI ACTIVE HIGH waitting start...\n");
	do{
		read32 = ioread32((u8*)virt_addr_page);
		if( (read32>>20) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACTIVE HIGH waitting done\n");

	//==========================
	// LPI REQ, Set LOW
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_REQ_IDX];
	read32 = ioread32((u8*)virt_addr_page);
	read32 = read32 & (~(1<<1)); // CSYSREQ LOW
	iowrite32(read32, ((u8*)virt_addr_page));

	//==========================
	// wait until LPI ACK LOW
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_ACTIVE_IDX];
	//printk("LPI ACTIVE LOW waitting start...\n");
	do{
		read32 = ioread32((u8*)virt_addr_page);
		if( !((read32>>21) & 0x01) )
			break;
	}while(1);
	//printk("LPI ACTIVE LOW waitting done\n");
}

static void nx_vr_power_up_leave_reset_s5p6818(void)
{
	u32 virt_addr_page, read32;

	//==========================
	// [ mali PBUS ]
	//==========================
	// LPI REQ, Set HIGH
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_REQ_IDX];
	read32 = ioread32((u8*)virt_addr_page);
	read32 = read32 | (1<<2); // CSYSREQ HIGH
	iowrite32(read32, ((u8*)virt_addr_page));

	//==========================
	// wait until LPI ACK HIGH
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_ACTIVE_IDX];
	//printk("LPI ACK HIGH waitting start...\n");
	do{
		read32 = ioread32((u8*)virt_addr_page);
		if( (read32>>13) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACK HIGH waitting done\n");

	//==========================
	// [ mali MBUS ]
	//==========================
	// LPI REQ, Set HIGH
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_REQ_IDX];
	read32 = ioread32((u8*)virt_addr_page);
	read32 = read32 | (1<<1); // CSYSREQ HIGH
	iowrite32(read32, ((u8*)virt_addr_page));

	//==========================
	// wait until LPI ACK HIGH
	//==========================
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_LPI_ACTIVE_IDX];
	//printk("LPI ACK ACK waitting start...\n");
	do{
		read32 = ioread32((u8*)virt_addr_page);
		if( (read32>>21) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACK ACK waitting done\n");

}

static void nx_vr_power_down_all_s5p6818(void)
{
	u32 virt_addr_page, map_size;

	//reset
	nx_vr_power_down_enter_reset_s5p6818();
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_RESET_IDX] + 8;
	if (NULL != virt_addr_page)
	{
		unsigned int temp32 = ioread32(((u8*)virt_addr_page));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)virt_addr_page);
		temp32 &= ~bit_mask;
		iowrite32(temp32, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//clk disalbe
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_CLOCK_GATE_IDX];
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting ClockGen, set 0\n");
		iowrite32(read_val & ~0x3, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//ready
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_POWER_GATE_IDX];
	if (NULL != virt_addr_page)
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)virt_addr_page));
	}

	//enable ISolate
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX];
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 0\n");
		iowrite32(read_val & ~1, ((u8*)virt_addr_page));
	}

	//pre charge down
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 4;
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 1\n");
		iowrite32(read_val | 1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//powerdown
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 8;
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 1\n");
		iowrite32(read_val | 1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 0xC;
	do{
		unsigned int powerUpAck = ioread32((u8*)virt_addr_page);
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( (powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);
}

static void nx_vr_power_up_all_s5p6818(void)
{
	u32 virt_addr_page;

	//ready
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_POWER_GATE_IDX];
	if (NULL != virt_addr_page)
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)virt_addr_page));
	}

	//pre charge up
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 4;
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)virt_addr_page));
	}

	//power up
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 8;
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//disable ISolate
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX];
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 1\n");
		iowrite32(read_val | 1, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_PMU_ISOLATE_IDX] + 0xC;
	do{
		unsigned int powerUpAck = ioread32((u8*)virt_addr_page);
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( !(powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);

	//clk enable
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_CLOCK_GATE_IDX];
	if (NULL != virt_addr_page)
	{
		unsigned int read_val = ioread32((u8*)virt_addr_page);
		VR_DBG("setting ClockGen, set 1\n");
		iowrite32(0x3 | read_val, ((u8*)virt_addr_page));
	}

	//reset
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_RESET_IDX] + 8;
	if (NULL != virt_addr_page)
	{
		unsigned int temp32 = ioread32(((u8*)virt_addr_page));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)virt_addr_page);

		//reset leave
		nx_vr_power_up_leave_reset_s5p6818();
		temp32 |= bit_mask;
		iowrite32(temp32, ((u8*)virt_addr_page));
	}
	mdelay(1);

	//mask vr400 PMU interrupt
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_VR_PMU_IDX] + 0xC;
	if (NULL != virt_addr_page)
	{
		VR_PM_DBG("mask PMU INT, addr(0x%x)\n", (int)virt_addr_page);
		iowrite32(0x0, ((u8*)virt_addr_page));
	}

	//power up vr400
	virt_addr_page = __g_VRRegVirtMaps[PHY_BASEADDR_VR_PMU_IDX];
	if (NULL != virt_addr_page)
	{
		VR_DBG("setting PHY_BASEADDR_VR_PMU addr(0x%x)\n", (int)virt_addr_page);
		iowrite32(0x3F/*GP, L2C, PP0, PP1, PP2, PP3*/, ((u8*)virt_addr_page));
	}
}
#endif

#else /* legacy code */
#if defined( CONFIG_ARCH_S5P4418 )
static void nx_vr_power_down_all_s5p4418(void)
{
	u32 phys_addr_page, map_size;
	void *mem_mapped;

	//temp test
#if 0
	//clear vr400 pmu interrupt
	phys_addr_page = PHY_BASEADDR_VR_PMU + 0x18;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_PM_DBG("clear PMU int, addr(0x%x)\n", (int)mem_mapped);
		iowrite32(1, ((u8*)mem_mapped));
		iounmap(mem_mapped);
	}
#endif
#if 0
	//clear s5p4418 interrupt controller
	phys_addr_page = 0xC0003014;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_PM_DBG("clear INTCLEAR, addr(0x%x)\n", (int)mem_mapped);
		iowrite32(0x100, ((u8*)mem_mapped));
		iounmap(mem_mapped);
	}
#else
#endif

	//clk disalbe
	phys_addr_page = PHY_BASEADDR_CLOCK_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting ClockGen, set 0\n");
		iowrite32(read_val & ~0x3, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//ready
	phys_addr_page = PHY_BASEADDR_POWER_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//enable ISolate
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 0\n");
		iowrite32(read_val & ~1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//pre charge down
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 4;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 1\n");
		iowrite32(read_val | 1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

#if 1
	//powerdown
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 8;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 1\n");
		iowrite32(read_val | 1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 0xC;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	do{
		unsigned int powerUpAck = ioread32((u8*)mem_mapped);
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( (powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);
	iounmap(mem_mapped);
#endif
}

static void nx_vr_power_up_all_s5p4418(void)
{
	u32 phys_addr_page, map_size;
	void *mem_mapped;

	//ready
	phys_addr_page = PHY_BASEADDR_POWER_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//pre charge up
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 4;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//power up
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 8;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//disable ISolate
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 1\n");
		iowrite32(read_val | 1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 0xC;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	do{
		unsigned int powerUpAck = ioread32((u8*)mem_mapped);
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( !(powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);
	iounmap(mem_mapped);

	//clk enable
	phys_addr_page = PHY_BASEADDR_CLOCK_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting ClockGen, set 1\n");
		iowrite32(0x3 | read_val, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//reset release
	phys_addr_page = PHY_BASEADDR_RESET + 8;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int temp32 = ioread32(((u8*)mem_mapped));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)mem_mapped);
		temp32 &= ~bit_mask;
		iowrite32(temp32, ((u8*)mem_mapped));
		temp32 |= bit_mask;
		iowrite32(temp32, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//mask vr400 PMU interrupt
	phys_addr_page = PHY_BASEADDR_VR_PMU + 0xC;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_PM_DBG("mask PMU INT, addr(0x%x)\n", (int)mem_mapped);
		iowrite32(0x0, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//power up vr400
	phys_addr_page = PHY_BASEADDR_VR_PMU;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_DBG("setting PHY_BASEADDR_VR_PMU addr(0x%x)\n", (int)mem_mapped);
		iowrite32(0xF/*GP, L2C, PP0, PP1*/, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//ocurr ERROR ! at 10ms delay
	//VR_PM_DBG("mdelay 20ms\n");
	//mdelay(20);

#if 0
	//mask vr400 GP2 AXI_BUS_ERROR
	phys_addr_page = PHY_BASEADDR_VR_GP + 0x2C;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int temp32 = ioread32(((u8*)mem_mapped));
		VR_PM_DBG("mask GP2 BUSERR_INT, addr(0x%x, 0x%x)\n", (int)mem_mapped, temp32);
		iounmap(mem_mapped);
	}

	//temp test
	VR_PM_DBG("\n============================\n", (int)mem_mapped);
	//clear vr400 pmu interrupt
	//PHY_BASEADDR_RESET + 8 에 reset 해줘야만 한다
	phys_addr_page = PHY_BASEADDR_VR_PMU + 0x18;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_PM_DBG("clear PMU int, addr(0x%x)\n", (int)mem_mapped);
		iowrite32(1, ((u8*)mem_mapped));
		iounmap(mem_mapped);
	}
#endif
#if 0
	//clear s5p4418 interrupt controller
	phys_addr_page = 0xC0003014;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_PM_DBG("clear INTCLEAR, addr(0x%x)\n", (int)mem_mapped);
		iowrite32(0x100, ((u8*)mem_mapped));
		iounmap(mem_mapped);
	}
	VR_PM_DBG("============================\n", (int)mem_mapped);
#else
#endif

	#if 0
	//temp test
	phys_addr_page = 0xC0010000;
	map_size       = 0x30;
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		int i = 0;
		for(i = 0 ; i < 12 ; i++)
		{
			unsigned int read_val = ioread32((u8*)mem_mapped + (i*4));
			VR_DBG("read reg addr(0x%x), data(0x%x)\n", (int)mem_mapped + (i*4), read_val);
		}
		iounmap(mem_mapped);
	}
	phys_addr_page = 0xC0010D00;
	map_size       = 0x10;
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		int i = 0;
		for(i = 0 ; i < 4 ; i++)
		{
			unsigned int read_val = ioread32((u8*)mem_mapped + (i*4));
			VR_DBG("read reg addr(0x%x), data(0x%x)\n", (int)mem_mapped + (i*4), read_val);
		}
		iounmap(mem_mapped);
	}
	#endif
}
#endif

#if defined(CONFIG_ARCH_S5P6818)
#if 0
static void NX_RESET_EnterReset ( uint RSTIndex )
{
	//=========================
	// mali LPI protocol use
	//=========================
	volatile uint * reg;

	//=========================
	// [ mali PBUS ]
	//=========================
	// wait until LPI ACTIVE HIGH
	//=========================
	reg =(uint*) PHY_BASEADDR_LPI_ACTIVE;
	while( (*reg>>12) & 0x01 != 1 ) ;

	//==========================
	// LPI REQ, Set LOW
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_REQ;
	*reg = *reg & (~(1<<2)); // CSYSREQ LOW

	//==========================
	// wait until LPI ACK LOW
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_ACTIVE;
	while( (*reg>>13) & 0x01 != 0 );

	//=========================
	// [ mali MBUS ]
	//=========================
	// wait until LPI ACTIVE HIGH
	//=========================
	reg =(uint*) PHY_BASEADDR_LPI_ACTIVE;
	while( (*reg>>20) & 0x01 != 1 ) ;
	//==========================
	// LPI REQ, Set LOW
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_REQ;
	*reg = *reg & (~(1<<1)); // CSYSREQ LOW
	//==========================
	// wait until LPI ACK LOW
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_ACTIVE;
	while( (*reg>>21) & 0x01 != 0 );

	//not eco
	//NX_CLKPWR_SetGRP3DClockEnable(CFALSE);
}

static void NX_RESET_LeaveReset ( uint RSTIndex )
{
	//=========================
	// mali LPI protocol use
	//=========================
	volatile uint *reg;

	//==========================
	// [ mali PBUS ]
	//==========================
	// LPI REQ, Set HIGH
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_REQ;
	*reg = *reg | (1<<2); // CSYSREQ HIGH
	//==========================
	// wait until LPI ACK HIGH
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_ACTIVE;
	while( (*reg>>13) & 0x01 != 1 );

	//==========================
	// [ mali MBUS ]
	//==========================
	// LPI REQ, Set HIGH
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_REQ;
	*reg = *reg | (1<<1); // CSYSREQ HIGH
	//==========================
	// wait until LPI ACK HIGH
	//==========================
	reg =(uint*) PHY_BASEADDR_LPI_ACTIVE;
	while( (*reg>>21) & 0x01 != 1 );
}
#endif
static void nx_vr_power_down_enter_reset_s5p6818(void)
{
	u32 phys_addr_page, map_size, read32;
	void *mem_mapped;

	//=========================
	// [ mali PBUS ]
	//=========================
	// wait until LPI ACTIVE HIGH
	//=========================
	phys_addr_page = PHY_BASEADDR_LPI_ACTIVE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	//printk("LPI ACTIVE HIGH waitting start...\n");
	do{
		read32 = ioread32((u8*)mem_mapped);
		if( (read32>>12) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACTIVE HIGH waitting done\n");
	iounmap(mem_mapped);

	//==========================
	// LPI REQ, Set LOW
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_REQ;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	read32 = ioread32((u8*)mem_mapped);
	read32 = read32 & (~(1<<2)); // CSYSREQ LOW
	iowrite32(read32, ((u8*)mem_mapped));
	iounmap(mem_mapped);

	//==========================
	// wait until LPI ACK LOW
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_ACTIVE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	//printk("LPI ACK LOW waitting start...\n");
	do{
		read32 = ioread32((u8*)mem_mapped);
		if( !((read32>>13) & 0x01) )
			break;
	}while(1);
	//printk("LPI ACK LOW waitting done\n");
	iounmap(mem_mapped);

	//=========================
	// [ mali MBUS ]
	//=========================
	// wait until LPI ACTIVE HIGH
	//=========================
	phys_addr_page = PHY_BASEADDR_LPI_ACTIVE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	//printk("LPI ACTIVE HIGH waitting start...\n");
	do{
		read32 = ioread32((u8*)mem_mapped);
		if( (read32>>20) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACTIVE HIGH waitting done\n");
	iounmap(mem_mapped);

	//==========================
	// LPI REQ, Set LOW
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_REQ;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	read32 = ioread32((u8*)mem_mapped);
	read32 = read32 & (~(1<<1)); // CSYSREQ LOW
	iowrite32(read32, ((u8*)mem_mapped));
	iounmap(mem_mapped);

	//==========================
	// wait until LPI ACK LOW
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_ACTIVE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	//printk("LPI ACTIVE LOW waitting start...\n");
	do{
		read32 = ioread32((u8*)mem_mapped);
		if( !((read32>>21) & 0x01) )
			break;
	}while(1);
	//printk("LPI ACTIVE LOW waitting done\n");
	iounmap(mem_mapped);
}

static void nx_vr_power_up_leave_reset_s5p6818(void)
{
	u32 phys_addr_page, map_size, read32;
	void *mem_mapped;

	//==========================
	// [ mali PBUS ]
	//==========================
	// LPI REQ, Set HIGH
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_REQ;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	read32 = ioread32((u8*)mem_mapped);
	read32 = read32 | (1<<2); // CSYSREQ HIGH
	iowrite32(read32, ((u8*)mem_mapped));
	iounmap(mem_mapped);

	//==========================
	// wait until LPI ACK HIGH
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_ACTIVE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	//printk("LPI ACK HIGH waitting start...\n");
	do{
		read32 = ioread32((u8*)mem_mapped);
		if( (read32>>13) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACK HIGH waitting done\n");
	iounmap(mem_mapped);

	//==========================
	// [ mali MBUS ]
	//==========================
	// LPI REQ, Set HIGH
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_REQ;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	read32 = ioread32((u8*)mem_mapped);
	read32 = read32 | (1<<1); // CSYSREQ HIGH
	iowrite32(read32, ((u8*)mem_mapped));
	iounmap(mem_mapped);

	//==========================
	// wait until LPI ACK HIGH
	//==========================
	phys_addr_page = PHY_BASEADDR_LPI_ACTIVE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	//printk("LPI ACK ACK waitting start...\n");
	do{
		read32 = ioread32((u8*)mem_mapped);
		if( (read32>>21) & 0x01 )
			break;
	}while(1);
	//printk("LPI ACK ACK waitting done\n");
	iounmap(mem_mapped);

}

static void nx_vr_power_down_all_s5p6818(void)
{
	u32 phys_addr_page, map_size;
	void *mem_mapped;

	//reset
	nx_vr_power_down_enter_reset_s5p6818();
	phys_addr_page = PHY_BASEADDR_RESET + 8;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int temp32 = ioread32(((u8*)mem_mapped));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)mem_mapped);
		temp32 &= ~bit_mask;
		iowrite32(temp32, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//clk disalbe
	phys_addr_page = PHY_BASEADDR_CLOCK_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting ClockGen, set 0\n");
		iowrite32(read_val & ~0x3, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//ready
	phys_addr_page = PHY_BASEADDR_POWER_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//enable ISolate
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 0\n");
		iowrite32(read_val & ~1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//pre charge down
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 4;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 1\n");
		iowrite32(read_val | 1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//powerdown
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 8;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 1\n");
		iowrite32(read_val | 1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 0xC;
	map_size       = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	do{
		unsigned int powerUpAck = ioread32((u8*)mem_mapped);
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( (powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power Down Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);
	iounmap(mem_mapped);
}

static void nx_vr_power_up_all_s5p6818(void)
{
	u32 phys_addr_page, map_size;
	void *mem_mapped;

	//ready
	phys_addr_page = PHY_BASEADDR_POWER_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_DBG("setting PHY_BASEADDR_POWER_GATE, set 1\n");
		iowrite32(0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//pre charge up
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 4;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+4, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//power up
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 8;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE+8, set 0\n");
		iowrite32(read_val & ~0x1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//disable ISolate
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting PHY_BASEADDR_PMU_ISOLATE, set 1\n");
		iowrite32(read_val | 1, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//wait ack
	VR_DBG("read PHY_BASEADDR_PMU_ISOLATE + 0xC\n");
	phys_addr_page = PHY_BASEADDR_PMU_ISOLATE + 0xC;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	do{
		unsigned int powerUpAck = ioread32((u8*)mem_mapped);
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
		if( !(powerUpAck & 0x1) )
			break;
		VR_DBG("Wait Power UP Ack(powerUpAck=0x%08x)\n", powerUpAck);
	}while(1);

	//clk enable
	phys_addr_page = PHY_BASEADDR_CLOCK_GATE;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int read_val = ioread32((u8*)mem_mapped);
		VR_DBG("setting ClockGen, set 1\n");
		iowrite32(0x3 | read_val, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//reset
	phys_addr_page = PHY_BASEADDR_RESET + 8;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		unsigned int temp32 = ioread32(((u8*)mem_mapped));
		unsigned int bit_mask = 1<<1; //65th
		VR_DBG("setting Reset VR addr(0x%x)\n", (int)mem_mapped);

		//reset leave
		nx_vr_power_up_leave_reset_s5p6818();
		temp32 |= bit_mask;
		iowrite32(temp32, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
	mdelay(1);

	//mask vr400 PMU interrupt
	phys_addr_page = PHY_BASEADDR_VR_PMU + 0xC;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_PM_DBG("mask PMU INT, addr(0x%x)\n", (int)mem_mapped);
		iowrite32(0x0, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);

	//power up vr400
	phys_addr_page = PHY_BASEADDR_VR_PMU;
	map_size	   = sizeof(u32);
	mem_mapped = ioremap_nocache(phys_addr_page, map_size);
	if (NULL != mem_mapped)
	{
		VR_DBG("setting PHY_BASEADDR_VR_PMU addr(0x%x)\n", (int)mem_mapped);
		iowrite32(0x3F/*GP, L2C, PP0, PP1, PP2, PP3*/, ((u8*)mem_mapped));
	}
	iounmap(mem_mapped);
}
#endif
#endif

static void nx_vr_power_down_all(void)
{
	#if defined( CONFIG_ARCH_S5P4418 )
	nx_vr_power_down_all_s5p4418();
	#elif defined(CONFIG_ARCH_S5P6818)
	nx_vr_power_down_all_s5p6818();
	#else
	printk("=============================================================\n");
	printk("ERROR!!! No Supported Platform\n");
	printk("=============================================================\n");
	#endif
}

static void nx_vr_power_up_first(void)
{
#if defined( CONFIG_ARCH_S5P4418 )
	nx_vr_power_up_all_s5p4418();
#elif defined(CONFIG_ARCH_S5P6818)
	nx_vr_power_up_all_s5p6818();
#else
	printk("=============================================================\n");
	printk("ERROR!!! No Supported Platform\n");
	printk("=============================================================\n");
#endif
}

static void nx_vr_power_up_all(void)
{
	#if defined( CONFIG_ARCH_S5P4418 )
	nx_vr_power_up_all_s5p4418();
	#elif defined(CONFIG_ARCH_S5P6818)
	nx_vr_power_up_all_s5p6818();
	#else
	printk("=============================================================\n");
	printk("ERROR!!! select platform type at build.sh(s5p4418 or s5p6818)\n");
	printk("=============================================================\n");
	#endif
}

#if VR_ENABLE_CPU_CYCLES
void vr_init_cpu_time_counters(int reset, int enable_divide_by_64)
{
	/* The CPU assembly reference used is: ARM Architecture Reference Manual ARMv7-AR C.b */
	u32 write_value;

	/* See B4.1.116 PMCNTENSET, Performance Monitors Count Enable Set register, VMSA */
	/* setting p15 c9 c12 1 to 0x8000000f==CPU_CYCLE_ENABLE |EVENT_3_ENABLE|EVENT_2_ENABLE|EVENT_1_ENABLE|EVENT_0_ENABLE */
	asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(0x8000000f));


	/* See B4.1.117 PMCR, Performance Monitors Control Register. Writing to p15, c9, c12, 0 */
	write_value = 1<<0; /* Bit 0 set. Enable counters */
	if (reset) {
		write_value |= 1<<1; /* Reset event counters */
		write_value |= 1<<2; /* Reset cycle counter  */
	}
	if (enable_divide_by_64) {
		write_value |= 1<<3; /* Enable the Clock divider by 64 */
	}
	write_value |= 1<<4; /* Export enable. Not needed */
	asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(write_value ));

	/* PMOVSR Overflow Flag Status Register - Clear Clock and Event overflows */
	asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));


	/* See B4.1.124 PMUSERENR - setting p15 c9 c14 to 1" */
	/* User mode access to the Performance Monitors enabled. */
	/* Lets User space read cpu clock cycles */
	asm volatile( "mcr p15, 0, %0, c9, c14, 0" :: "r"(1) );
}

/** A timer function that configures the cycle clock counter on current CPU.
	The function \a vr_init_cpu_time_counters_on_all_cpus sets up this function
	to trigger on all Cpus during module load. */
static void vr_init_cpu_clock_timer_func(unsigned long data)
{
	int reset_counters, enable_divide_clock_counter_by_64;
	int current_cpu = raw_smp_processor_id();
	unsigned int sample0;
	unsigned int sample1;

	VR_IGNORE(data);

	reset_counters= 1;
	enable_divide_clock_counter_by_64 = 0;
	vr_init_cpu_time_counters(reset_counters, enable_divide_clock_counter_by_64);

	sample0 = vr_get_cpu_cyclecount();
	sample1 = vr_get_cpu_cyclecount();

	VR_DEBUG_PRINT(3, ("Init Cpu %d cycle counter- First two samples: %08x %08x \n", current_cpu, sample0, sample1));
}

/** A timer functions for storing current time on all cpus.
    Used for checking if the clocks have similar values or if they are drifting. */
static void vr_print_cpu_clock_timer_func(unsigned long data)
{
	int current_cpu = raw_smp_processor_id();
	unsigned int sample0;

	VR_IGNORE(data);
	sample0 = vr_get_cpu_cyclecount();
	if ( current_cpu<8 ) {
		vr_cpu_clock_last_value[current_cpu] = sample0;
	}
}

/** Init the performance registers on all CPUs to count clock cycles.
	For init \a print_only should be 0.
    If \a print_only is 1, it will intead print the current clock value of all CPUs.*/
void vr_init_cpu_time_counters_on_all_cpus(int print_only)
{
	int i = 0;
	int cpu_number;
	int jiffies_trigger;
	int jiffies_wait;

	jiffies_wait = 2;
	jiffies_trigger = jiffies + jiffies_wait;

	for ( i=0 ; i < 8 ; i++ ) {
		init_timer(&vr_init_cpu_clock_timers[i]);
		if (print_only) vr_init_cpu_clock_timers[i].function = vr_print_cpu_clock_timer_func;
		else            vr_init_cpu_clock_timers[i].function = vr_init_cpu_clock_timer_func;
		vr_init_cpu_clock_timers[i].expires = jiffies_trigger ;
	}
	cpu_number = cpumask_first(cpu_online_mask);
	for ( i=0 ; i < 8 ; i++ ) {
		int next_cpu;
		add_timer_on(&vr_init_cpu_clock_timers[i], cpu_number);
		next_cpu = cpumask_next(cpu_number, cpu_online_mask);
		if (next_cpu >= nr_cpu_ids) break;
		cpu_number = next_cpu;
	}

	while (jiffies_wait) jiffies_wait= schedule_timeout_uninterruptible(jiffies_wait);

	for ( i=0 ; i < 8 ; i++ ) {
		del_timer_sync(&vr_init_cpu_clock_timers[i]);
	}

	if (print_only) {
		if ( (0==vr_cpu_clock_last_value[2]) &&  (0==vr_cpu_clock_last_value[3]) ) {
			/* Diff can be printed if we want to check if the clocks are in sync
			int diff = vr_cpu_clock_last_value[0] - vr_cpu_clock_last_value[1];*/
			VR_DEBUG_PRINT(2, ("CPU cycle counters readout all: %08x %08x\n", vr_cpu_clock_last_value[0], vr_cpu_clock_last_value[1]));
		} else {
			VR_DEBUG_PRINT(2, ("CPU cycle counters readout all: %08x %08x %08x %08x\n", vr_cpu_clock_last_value[0], vr_cpu_clock_last_value[1], vr_cpu_clock_last_value[2], vr_cpu_clock_last_value[3] ));
		}
	}
}
#endif


int vr_module_init(void)
{
	int err = 0;

	/* NEXELL_FEATURE_PORTING */
	//added by nexell
	nx_vr_make_reg_virt_maps();
	nx_vr_power_up_first();
	//vr_pmu_powerup();

	VR_DEBUG_PRINT(2, ("Inserting Vr v%d device driver. \n",_VR_API_VERSION));
	VR_DEBUG_PRINT(2, ("Compiled: %s, time: %s.\n", __DATE__, __TIME__));
	VR_DEBUG_PRINT(2, ("Driver revision: %s\n", SVN_REV_STRING));

#if VR_ENABLE_CPU_CYCLES
	vr_init_cpu_time_counters_on_all_cpus(0);
	VR_DEBUG_PRINT(2, ("CPU cycle counter setup complete\n"));
	/* Printing the current cpu counters */
	vr_init_cpu_time_counters_on_all_cpus(1);
#endif

	/* Initialize module wide settings */
#if defined(VR_FAKE_PLATFORM_DEVICE)
	VR_DEBUG_PRINT(2, ("vr_module_init() registering device\n"));
	err = vr_platform_device_register();
	if (0 != err) {
		return err;
	}
#endif

	VR_DEBUG_PRINT(2, ("vr_module_init() registering driver\n"));

	err = platform_driver_register(&vr_platform_driver);

	if (0 != err) {
		VR_DEBUG_PRINT(2, ("vr_module_init() Failed to register driver (%d)\n", err));
#if defined(VR_FAKE_PLATFORM_DEVICE)
		vr_platform_device_unregister();
#endif
		vr_platform_device = NULL;
		return err;
	}

#if defined(CONFIG_VR400_INTERNAL_PROFILING)
	err = _vr_internal_profiling_init(vr_boot_profiling ? VR_TRUE : VR_FALSE);
	if (0 != err) {
		/* No biggie if we wheren't able to initialize the profiling */
		VR_PRINT_ERROR(("Failed to initialize profiling, feature will be unavailable\n"));
	}
#endif

	#if defined( CONFIG_ARCH_S5P4418 )
	VR_PRINT(("VR device driver loaded(ver1.2) for s5p4418\n"));
	#elif defined( CONFIG_ARCH_S5P6818 )
	VR_PRINT(("VR device driver loaded(ver1.2) for s5p6818\n"));
	#else
	VR_PRINT(("unvalid VR device driver loaded. please check build.sh\n"));
	#endif
	return 0; /* Success */
}

void vr_module_exit(void)
{
	VR_DEBUG_PRINT(2, ("Unloading Vr v%d device driver.\n",_VR_API_VERSION));

	VR_DEBUG_PRINT(2, ("vr_module_exit() unregistering driver\n"));

#if defined(CONFIG_VR400_INTERNAL_PROFILING)
	_vr_internal_profiling_term();
#endif

	platform_driver_unregister(&vr_platform_driver);

#if defined(VR_FAKE_PLATFORM_DEVICE)
	VR_DEBUG_PRINT(2, ("vr_module_exit() unregistering device\n"));
	vr_platform_device_unregister();
#endif

	/* NEXELL_FEATURE_PORTING */
	//added by nexell
	/*
	this function occurs segfualt error in case of rmmod.
        but it's ok because nx_vr_power_down_all() will shut down all power.
	vr_pmu_powerdown();
	*/

	//added by nexell
	nx_vr_power_down_all();
	nx_vr_release_reg_virt_maps();
	VR_PRINT(("Vr device driver unloaded\n"));
}

static int vr_probe(struct platform_device *pdev)
{
	int err;

	VR_DEBUG_PRINT(2, ("vr_probe(): Called for platform device %s\n", pdev->name));

	if (NULL != vr_platform_device) {
		/* Already connected to a device, return error */
		VR_PRINT_ERROR(("vr_probe(): The Vr driver is already connected with a Vr device."));
		return -EEXIST;
	}

	vr_platform_device = pdev;

	if (_VR_OSK_ERR_OK == _vr_osk_wq_init()) {
		/* Initialize the Vr GPU HW specified by pdev */
		if (_VR_OSK_ERR_OK == vr_initialize_subsystems()) {
			/* Register a misc device (so we are accessible from user space) */
			err = vr_miscdevice_register(pdev);
			if (0 == err) {
				/* Setup sysfs entries */
				err = vr_sysfs_register(vr_dev_name);
				if (0 == err) {
					VR_DEBUG_PRINT(2, ("vr_probe(): Successfully initialized driver for platform device %s\n", pdev->name));
					return 0;
				} else {
					VR_PRINT_ERROR(("vr_probe(): failed to register sysfs entries"));
				}
				vr_miscdevice_unregister();
			} else {
				VR_PRINT_ERROR(("vr_probe(): failed to register Vr misc device."));
			}
			vr_terminate_subsystems();
		} else {
			VR_PRINT_ERROR(("vr_probe(): Failed to initialize Vr device driver."));
		}
		_vr_osk_wq_term();
	}

	vr_platform_device = NULL;
	return -EFAULT;
}

static int vr_remove(struct platform_device *pdev)
{
	VR_DEBUG_PRINT(2, ("vr_remove() called for platform device %s\n", pdev->name));
	vr_sysfs_unregister();
	vr_miscdevice_unregister();
	vr_terminate_subsystems();
	_vr_osk_wq_term();
	vr_platform_device = NULL;
	return 0;
}

static int vr_miscdevice_register(struct platform_device *pdev)
{
	int err;

	vr_miscdevice.minor = MISC_DYNAMIC_MINOR;
	vr_miscdevice.name = vr_dev_name;
	vr_miscdevice.fops = &vr_fops;
	vr_miscdevice.parent = get_device(&pdev->dev);

	err = misc_register(&vr_miscdevice);
	if (0 != err) {
		VR_PRINT_ERROR(("Failed to register misc device, misc_register() returned %d\n", err));
	}

	return err;
}

static void vr_miscdevice_unregister(void)
{
	misc_deregister(&vr_miscdevice);
}

static int vr_driver_suspend_scheduler(struct device *dev)
{
	VR_PM_DBG("-----------------------------------------------------\n");
	VR_PM_DBG("	VR POWERDown Start\n");

	vr_pm_os_suspend();
	nx_vr_power_down_all();

	VR_PM_DBG("	VR POWERDown End\n");
	VR_PM_DBG("-----------------------------------------------------\n");
	return 0;
}

static int vr_driver_resume_scheduler(struct device *dev)
{
	VR_PM_DBG("-----------------------------------------------------\n");
	VR_PM_DBG("	VR POWERUp Start\n");

	nx_vr_power_up_all();
	vr_pm_os_resume();

	VR_PM_DBG("	VR POWERUp End\n");
	VR_PM_DBG("-----------------------------------------------------\n");
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
#ifdef CONFIG_NEXELL_DFS_BCLK
#include <mach/nxp-dfs-bclk.h>
#endif
static int vr_driver_runtime_suspend(struct device *dev)
{
	//VR_PM_DBG("-----------------------------------------------------\n");
	//VR_PM_DBG("	VR POWERDown Start(CONFIG_PM_RUNTIME)\n");

	vr_pm_runtime_suspend();
#ifdef CONFIG_NEXELL_DFS_BCLK
    bclk_put(BCLK_USER_OGL);
#endif

	//VR_PM_DBG("	VR POWERDown End(CONFIG_PM_RUNTIME)\n");
	//VR_PM_DBG("-----------------------------------------------------\n");
	return 0;
}

static int vr_driver_runtime_resume(struct device *dev)
{
	//VR_PM_DBG("-----------------------------------------------------\n");
	//VR_PM_DBG("	VR POWERUp Start(CONFIG_PM_RUNTIME)\n");

#ifdef CONFIG_NEXELL_DFS_BCLK
    bclk_get(BCLK_USER_OGL);
#endif
	vr_pm_runtime_resume();

	//VR_PM_DBG("	VR POWERUp End(CONFIG_PM_RUNTIME)\n");
	//VR_PM_DBG("-----------------------------------------------------\n");
	return 0;
}

static int vr_driver_runtime_idle(struct device *dev)
{
	/* Nothing to do */
	return 0;
}
#endif

static int vr_open(struct inode *inode, struct file *filp)
{
	struct vr_session_data * session_data;
	_vr_osk_errcode_t err;

	/* input validation */
#if 0
	if (vr_miscdevice.minor != iminor(inode)) {
		VR_PRINT_ERROR(("vr_open() Minor does not match\n"));
		return -ENODEV;
	}
#endif

	/* allocated struct to track this session */
	err = _vr_ukk_open((void **)&session_data);
	if (_VR_OSK_ERR_OK != err) return map_errcode(err);

	/* initialize file pointer */
	filp->f_pos = 0;

	/* link in our session data */
	filp->private_data = (void*)session_data;

	return 0;
}

static int vr_release(struct inode *inode, struct file *filp)
{
	_vr_osk_errcode_t err;

	/* input validation */
    // psw0523 fix
#if 0
	if (vr_miscdevice.minor != iminor(inode)) {
		VR_PRINT_ERROR(("vr_release() Minor does not match\n"));
		return -ENODEV;
	}
#endif

	err = _vr_ukk_close((void **)&filp->private_data);
	if (_VR_OSK_ERR_OK != err) return map_errcode(err);

	return 0;
}

int map_errcode( _vr_osk_errcode_t err )
{
	switch(err) {
	case _VR_OSK_ERR_OK :
		return 0;
	case _VR_OSK_ERR_FAULT:
		return -EFAULT;
	case _VR_OSK_ERR_INVALID_FUNC:
		return -ENOTTY;
	case _VR_OSK_ERR_INVALID_ARGS:
		return -EINVAL;
	case _VR_OSK_ERR_NOMEM:
		return -ENOMEM;
	case _VR_OSK_ERR_TIMEOUT:
		return -ETIMEDOUT;
	case _VR_OSK_ERR_RESTARTSYSCALL:
		return -ERESTARTSYS;
	case _VR_OSK_ERR_ITEM_NOT_FOUND:
		return -ENOENT;
	default:
		return -EFAULT;
	}
}

#ifdef HAVE_UNLOCKED_IOCTL
static long vr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int vr_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	int err;
	struct vr_session_data *session_data;

#ifndef HAVE_UNLOCKED_IOCTL
	/* inode not used */
	(void)inode;
#endif

	VR_DEBUG_PRINT(7, ("Ioctl received 0x%08X 0x%08lX\n", cmd, arg));

	session_data = (struct vr_session_data *)filp->private_data;
	if (NULL == session_data) {
		VR_DEBUG_PRINT(7, ("filp->private_data was NULL\n"));
		return -ENOTTY;
	}

	if (NULL == (void *)arg) {
		VR_DEBUG_PRINT(7, ("arg was NULL\n"));
		return -ENOTTY;
	}

	switch(cmd) {
	case VR_IOC_WAIT_FOR_NOTIFICATION:
		err = wait_for_notification_wrapper(session_data, (_vr_uk_wait_for_notification_s __user *)arg);
		break;

	case VR_IOC_GET_API_VERSION:
		err = get_api_version_wrapper(session_data, (_vr_uk_get_api_version_s __user *)arg);
		break;

	case VR_IOC_POST_NOTIFICATION:
		err = post_notification_wrapper(session_data, (_vr_uk_post_notification_s __user *)arg);
		break;

	case VR_IOC_GET_USER_SETTINGS:
		err = get_user_settings_wrapper(session_data, (_vr_uk_get_user_settings_s __user *)arg);
		break;

	case VR_IOC_REQUEST_HIGH_PRIORITY:
		err = request_high_priority_wrapper(session_data, (_vr_uk_request_high_priority_s __user *)arg);
		break;

#if defined(CONFIG_VR400_PROFILING)
	case VR_IOC_PROFILING_START:
		err = profiling_start_wrapper(session_data, (_vr_uk_profiling_start_s __user *)arg);
		break;

	case VR_IOC_PROFILING_ADD_EVENT:
		err = profiling_add_event_wrapper(session_data, (_vr_uk_profiling_add_event_s __user *)arg);
		break;

	case VR_IOC_PROFILING_STOP:
		err = profiling_stop_wrapper(session_data, (_vr_uk_profiling_stop_s __user *)arg);
		break;

	case VR_IOC_PROFILING_GET_EVENT:
		err = profiling_get_event_wrapper(session_data, (_vr_uk_profiling_get_event_s __user *)arg);
		break;

	case VR_IOC_PROFILING_CLEAR:
		err = profiling_clear_wrapper(session_data, (_vr_uk_profiling_clear_s __user *)arg);
		break;

	case VR_IOC_PROFILING_GET_CONFIG:
		/* Deprecated: still compatible with get_user_settings */
		err = get_user_settings_wrapper(session_data, (_vr_uk_get_user_settings_s __user *)arg);
		break;

	case VR_IOC_PROFILING_REPORT_SW_COUNTERS:
		err = profiling_report_sw_counters_wrapper(session_data, (_vr_uk_sw_counters_report_s __user *)arg);
		break;

#else

	case VR_IOC_PROFILING_START:              /* FALL-THROUGH */
	case VR_IOC_PROFILING_ADD_EVENT:          /* FALL-THROUGH */
	case VR_IOC_PROFILING_STOP:               /* FALL-THROUGH */
	case VR_IOC_PROFILING_GET_EVENT:          /* FALL-THROUGH */
	case VR_IOC_PROFILING_CLEAR:              /* FALL-THROUGH */
	case VR_IOC_PROFILING_GET_CONFIG:         /* FALL-THROUGH */
	case VR_IOC_PROFILING_REPORT_SW_COUNTERS: /* FALL-THROUGH */
		VR_DEBUG_PRINT(2, ("Profiling not supported\n"));
		err = -ENOTTY;
		break;

#endif

	case VR_IOC_MEM_WRITE_SAFE:
		err = mem_write_safe_wrapper(session_data, (_vr_uk_mem_write_safe_s __user *)arg);
		break;

	case VR_IOC_MEM_MAP_EXT:
		err = mem_map_ext_wrapper(session_data, (_vr_uk_map_external_mem_s __user *)arg);
		break;

	case VR_IOC_MEM_UNMAP_EXT:
		err = mem_unmap_ext_wrapper(session_data, (_vr_uk_unmap_external_mem_s __user *)arg);
		break;

	case VR_IOC_MEM_QUERY_MMU_PAGE_TABLE_DUMP_SIZE:
		err = mem_query_mmu_page_table_dump_size_wrapper(session_data, (_vr_uk_query_mmu_page_table_dump_size_s __user *)arg);
		break;

	case VR_IOC_MEM_DUMP_MMU_PAGE_TABLE:
		err = mem_dump_mmu_page_table_wrapper(session_data, (_vr_uk_dump_mmu_page_table_s __user *)arg);
		break;

#if defined(CONFIG_VR400_UMP)

	case VR_IOC_MEM_ATTACH_UMP:
		err = mem_attach_ump_wrapper(session_data, (_vr_uk_attach_ump_mem_s __user *)arg);
		break;

	case VR_IOC_MEM_RELEASE_UMP:
		err = mem_release_ump_wrapper(session_data, (_vr_uk_release_ump_mem_s __user *)arg);
		break;

#else

	case VR_IOC_MEM_ATTACH_UMP:
	case VR_IOC_MEM_RELEASE_UMP: /* FALL-THROUGH */
		VR_DEBUG_PRINT(2, ("UMP not supported\n"));
		err = -ENOTTY;
		break;
#endif

#ifdef CONFIG_DMA_SHARED_BUFFER
	case VR_IOC_MEM_ATTACH_DMA_BUF:
		err = vr_attach_dma_buf(session_data, (_vr_uk_attach_dma_buf_s __user *)arg);
		break;

	case VR_IOC_MEM_RELEASE_DMA_BUF:
		err = vr_release_dma_buf(session_data, (_vr_uk_release_dma_buf_s __user *)arg);
		break;

	case VR_IOC_MEM_DMA_BUF_GET_SIZE:
		err = vr_dma_buf_get_size(session_data, (_vr_uk_dma_buf_get_size_s __user *)arg);
		break;
#else

	case VR_IOC_MEM_ATTACH_DMA_BUF:   /* FALL-THROUGH */
	case VR_IOC_MEM_RELEASE_DMA_BUF:  /* FALL-THROUGH */
	case VR_IOC_MEM_DMA_BUF_GET_SIZE: /* FALL-THROUGH */
		VR_DEBUG_PRINT(2, ("DMA-BUF not supported\n"));
		err = -ENOTTY;
		break;
#endif

	case VR_IOC_PP_START_JOB:
		err = pp_start_job_wrapper(session_data, (_vr_uk_pp_start_job_s __user *)arg);
		break;

	case VR_IOC_PP_AND_GP_START_JOB:
		err = pp_and_gp_start_job_wrapper(session_data, (_vr_uk_pp_and_gp_start_job_s __user *)arg);
		break;

	case VR_IOC_PP_NUMBER_OF_CORES_GET:
		err = pp_get_number_of_cores_wrapper(session_data, (_vr_uk_get_pp_number_of_cores_s __user *)arg);
		break;

	case VR_IOC_PP_CORE_VERSION_GET:
		err = pp_get_core_version_wrapper(session_data, (_vr_uk_get_pp_core_version_s __user *)arg);
		break;

	case VR_IOC_PP_DISABLE_WB:
		err = pp_disable_wb_wrapper(session_data, (_vr_uk_pp_disable_wb_s __user *)arg);
		break;

	case VR_IOC_GP2_START_JOB:
		err = gp_start_job_wrapper(session_data, (_vr_uk_gp_start_job_s __user *)arg);
		break;

	case VR_IOC_GP2_NUMBER_OF_CORES_GET:
		err = gp_get_number_of_cores_wrapper(session_data, (_vr_uk_get_gp_number_of_cores_s __user *)arg);
		break;

	case VR_IOC_GP2_CORE_VERSION_GET:
		err = gp_get_core_version_wrapper(session_data, (_vr_uk_get_gp_core_version_s __user *)arg);
		break;

	case VR_IOC_GP2_SUSPEND_RESPONSE:
		err = gp_suspend_response_wrapper(session_data, (_vr_uk_gp_suspend_response_s __user *)arg);
		break;

	case VR_IOC_VSYNC_EVENT_REPORT:
		err = vsync_event_report_wrapper(session_data, (_vr_uk_vsync_event_report_s __user *)arg);
		break;

	case VR_IOC_TIMELINE_GET_LATEST_POINT:
		err = timeline_get_latest_point_wrapper(session_data, (_vr_uk_timeline_get_latest_point_s __user *)arg);
		break;
	case VR_IOC_TIMELINE_WAIT:
		err = timeline_wait_wrapper(session_data, (_vr_uk_timeline_wait_s __user *)arg);
		break;
	case VR_IOC_TIMELINE_CREATE_SYNC_FENCE:
		err = timeline_create_sync_fence_wrapper(session_data, (_vr_uk_timeline_create_sync_fence_s __user *)arg);
		break;
	case VR_IOC_SOFT_JOB_START:
		err = soft_job_start_wrapper(session_data, (_vr_uk_soft_job_start_s __user *)arg);
		break;
	case VR_IOC_SOFT_JOB_SIGNAL:
		err = soft_job_signal_wrapper(session_data, (_vr_uk_soft_job_signal_s __user *)arg);
		break;

	case VR_IOC_MEM_INIT: /* Fallthrough */
	case VR_IOC_MEM_TERM: /* Fallthrough */
		VR_DEBUG_PRINT(2, ("Deprecated ioctls called\n"));
		err = -ENOTTY;
		break;

	case VR_IOC_MEM_GET_BIG_BLOCK: /* Fallthrough */
	case VR_IOC_MEM_FREE_BIG_BLOCK:
		VR_PRINT_ERROR(("Non-MMU mode is no longer supported.\n"));
		err = -ENOTTY;
		break;

	default:
		VR_DEBUG_PRINT(2, ("No handler for ioctl 0x%08X 0x%08lX\n", cmd, arg));
		err = -ENOTTY;
	};

	return err;
}


module_init(vr_module_init);
module_exit(vr_module_exit);

MODULE_LICENSE(VR_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("NEXELL Ltd.");
MODULE_VERSION(SVN_REV_STRING);

