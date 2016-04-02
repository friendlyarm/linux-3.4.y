/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <asm/hardware/gic.h>
#include <asm/hardware/cache-l2x0.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/map_desc.h>

#if (0)
#define DBGOUT(msg...)		do { printk("s5p6818: " msg); } while (0)
#else
#define DBGOUT(msg...)		do {} while (0)
#endif

#if (1)
#define _IOMAP()		{							\
	int i = 0;										\
	for (; i<ARRAY_SIZE(cpu_iomap_desc); i++) 		\
			printk(KERN_INFO "CPU : iomap[%2d]: p 0x%08x -> v 0x%08x len=0x%x\n", i,	\
			(u_int)(cpu_iomap_desc[i].pfn<<12),		\
			(u_int)(cpu_iomap_desc[i].virtual),		\
			(u_int)(cpu_iomap_desc[i].length));		\
	}
#else
#define _IOMAP()
#endif

#if defined(CFG_MEM_PHY_DMAZONE_SIZE)
extern void __init init_consistent_dma_size(unsigned long size);
#endif

/*------------------------------------------------------------------------------
 * 	cpu initialize and io/memory map.
 * 	procedure: fixup -> map_io -> init_irq -> timer init -> init_machine
 */
static void __init cpu_fixup(struct tag *tags, char **cmdline, struct meminfo *mi)
{
	DBGOUT("%s\n", __func__);
	/*
	 * system momory  = system mem size + dma zone size
	 */
	mi->nr_banks      = 1;
	mi->bank[0].start = CFG_MEM_PHY_SYSTEM_BASE;
#if !defined(CFG_MEM_PHY_DMAZONE_SIZE)
	mi->bank[0].size  = CFG_MEM_PHY_SYSTEM_SIZE;
#else
	mi->bank[0].size  = CFG_MEM_PHY_SYSTEM_SIZE + CFG_MEM_PHY_DMAZONE_SIZE;
#endif
}

#if	defined(CONFIG_SMP)
#define	LIVE_NR_CPUS 	NR_CPUS
#else
#define	LIVE_NR_CPUS 	4
#endif

static void __init cpu_map_io(void)
{
	int cores = LIVE_NR_CPUS;

	/*
	 * check memory map
	 */
	unsigned long io_end = cpu_iomap_desc[ARRAY_SIZE(cpu_iomap_desc)-1].virtual +
						   cpu_iomap_desc[ARRAY_SIZE(cpu_iomap_desc)-1].length;
#if defined(CFG_MEM_PHY_DMAZONE_SIZE)
	unsigned long dma_start = CONSISTENT_END - CFG_MEM_PHY_DMAZONE_SIZE;
#else
	unsigned long dma_start = CONSISTENT_END - SZ_2M;	// refer to dma-mapping.c
#endif
	if (io_end > dma_start)
		printk(KERN_ERR "\n****** BUG: Overlapped io mmap 0x%lx with dma start 0x%lx ******\n",
			io_end, dma_start);

	/* debug */
	_IOMAP();

	/* make iotable */
	iotable_init(cpu_iomap_desc, ARRAY_SIZE(cpu_iomap_desc));

#if defined(CFG_MEM_PHY_DMAZONE_SIZE)
	printk(KERN_INFO "CPU : DMA Zone Size =%2dM, CORE %d\n", CFG_MEM_PHY_DMAZONE_SIZE>>20, cores);
	init_consistent_dma_size(CFG_MEM_PHY_DMAZONE_SIZE);
#else
	printk(KERN_INFO "CPU : DMA Zone Size =%2dM, CORE %d\n", SZ_2M>>20, cores);
#endif

	nxp_cpu_arch_init();
	nxp_board_base_init();

	nxp_cpu_clock_init();
	nxp_cpu_clock_print();
}

static void __init cpu_init_machine(void)
{
	/* set shutdown */
	pm_power_off = nxp_cpu_shutdown;
	arm_pm_restart = nxp_cpu_reset;

	/*
	 * register platform device
	 */
	nxp_cpu_devs_register();
	nxp_board_devs_register();
}

static void __init cpu_mem_reserve(void)
{
#ifdef CONFIG_CMA
extern void nxp_reserve_mem(void);
	nxp_reserve_mem();
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	persistent_ram_console_reserve();
#endif
}

/*------------------------------------------------------------------------------
 * Maintainer: Nexell Co., Ltd.
 */
#include <mach/iomap.h>
extern struct sys_timer nxp_cpu_sys_timer;

#if defined(CFG_SYS_MACH_NAME)
#define NXP_MACH_NAME	CFG_SYS_MACH_NAME
#else
#define NXP_MACH_NAME	CFG_SYS_CPU_NAME
#endif

MACHINE_START(S5P6818, NXP_MACH_NAME)
	.atag_offset	=  0x00000100,
	.fixup			=  cpu_fixup,
	.map_io			=  cpu_map_io,
	.init_irq		=  nxp_cpu_irq_init,
	.handle_irq 	=  gic_handle_irq,
	.timer			= &nxp_cpu_sys_timer,
	.init_machine	=  cpu_init_machine,
#if defined CONFIG_CMA && defined CONFIG_ION
	.reserve		=  cpu_mem_reserve,
#endif
MACHINE_END
