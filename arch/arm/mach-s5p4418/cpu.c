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
#include <asm/hardware/vic.h>
#include <asm/hardware/gic.h>
#include <asm/hardware/cache-l2x0.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/map_desc.h>

#if (0)
#define DBGOUT(msg...)		do { printk("s5p4418: " msg); } while (0)
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

#if defined (CONFIG_SMP)
//	for ( ; (4 - cores) > 1; cores++)
//		nxp_cpu_core_shutdown(4-cores);
#else
//	for ( ; cores > 1; cores--)
//		nxp_cpu_core_shutdown(cores-1);
#endif

	nxp_cpu_base_init();
	nxp_board_base_init();

	nxp_cpu_clock_init();
	nxp_cpu_clock_print();
}

static void __init cpu_init_machine(void)
{
	/* set shutdown */
	pm_power_off   = nxp_cpu_shutdown;
	arm_pm_restart = nxp_cpu_reset;

	/*
	 * register platform device
	 */
	nxp_cpu_devices_register();
	nxp_board_devices_register();
}

#ifdef CONFIG_CACHE_L2X0
static int __init cpu_l2cach_init(void)
{
	#define PL310_DYNAMIC_CLK_GATING_EN (1 << 1)
	#define PL310_STNDBY_MODE_EN       	(1 << 0)

	#define _AUX_BRESP					(1<<30)
	#define _AUX_PREFETCH_I				(1<<29)
	#define _AUX_PREFETCH_D				(1<<28)
	#define _AUX_INTERRUPT				(1<<27)
	#define _AUX_LOCKDOWN				(1<<26)
	#define _AUX_ROUND_ROBIN			(1<<25)
	#define _AUX_WRITE_ALLOCATE(x)		((x&3)<<23)	// 1 = force no write
	#define _AUX_SHARED					(1<<22)
	#define _AUX_PARITY					(1<<21)
	#define _AUX_EVENT_MONITOR			(1<<20)
	#define _AUX_WAY_SIZE(x)			((x&7)<<17)
	#define _AUX_16_WAY					(1<<16)
	#define _AUX_SHARED_INVALIDATE		(1<<13)
	#define _AUX_EXCLUSIVE_CACHE		(1<<12)
	#define _AUX_STORE_BUFFER_DEVICE	(1<<11)
	#define _AUX_DEV_READS				(1<<10)
	#define _AUX_FULL_LINE_OF_ZERO		(1<< 0)

	#define L2_AUX_DEF_MASK 0xfc000fff
	#define L2_AUX_NEW_VAL 	_AUX_BRESP				|	\
							_AUX_PREFETCH_I			|	\
							_AUX_PREFETCH_D			|	\
							_AUX_SHARED				|	\
							_AUX_WAY_SIZE(3)		|	\
							_AUX_16_WAY			 	|	\
							_AUX_FULL_LINE_OF_ZERO

	unsigned int tag_latency  = (1<<8) | (2<<4) | (1<<0);
	unsigned int data_latency = (1<<8) | (2<<4) | (1<<0);
	unsigned int prefetch_ctrl = 0x30000007;
	unsigned int pwr_ctrl = (PL310_DYNAMIC_CLK_GATING_EN | PL310_STNDBY_MODE_EN);
	unsigned int tieoff = __raw_readl((void*)IO_ADDRESS(PHY_BASEADDR_TIEOFF));	/* L2C SRAM TIEOFF */

	/* tieoff power on */
	__raw_writel(tieoff | 0x3000, (void*)IO_ADDRESS(PHY_BASEADDR_TIEOFF));

	/* l2cache ctrl */
	__raw_writel(tag_latency, (__PB_IO_MAP_L2C_VIRT + L2X0_TAG_LATENCY_CTRL));
	__raw_writel(data_latency, (__PB_IO_MAP_L2C_VIRT + L2X0_DATA_LATENCY_CTRL));
	__raw_writel(prefetch_ctrl, (__PB_IO_MAP_L2C_VIRT + L2X0_PREFETCH_CTRL));
	__raw_writel(pwr_ctrl, (__PB_IO_MAP_L2C_VIRT + L2X0_POWER_CTRL));

#if defined (CONFIG_CPU_S5P4418_EX_PERI_BUS)
	__raw_writel(0xCFF00000, __PB_IO_MAP_L2C_VIRT + L2X0_ADDR_FILTER_END);        // FILTERING END
	__raw_writel(0xC0000001, __PB_IO_MAP_L2C_VIRT + L2X0_ADDR_FILTER_START);      // FILTERING START + ENABLE
#endif

	/* l2cache init */
	l2x0_init((void __iomem *)__PB_IO_MAP_L2C_VIRT, L2_AUX_NEW_VAL, L2_AUX_DEF_MASK);

	return 0;
}

early_initcall(cpu_l2cach_init);
#endif

#if defined CONFIG_CMA && defined CONFIG_ION
extern void nxp_reserve_mem(void);
static void __init cpu_mem_reserve(void)
{
	#ifdef CONFIG_CMA
	nxp_reserve_mem();
	#endif

	#ifdef CONFIG_ANDROID_RAM_CONSOLE
	persistent_ram_console_reserve();
	#endif
}
#endif

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

MACHINE_START(S5P4418, NXP_MACH_NAME)
	.atag_offset	=  0x00000100,
	.fixup			=  cpu_fixup,
	.map_io			=  cpu_map_io,
	.init_irq		=  nxp_cpu_init_irq,
#ifdef CONFIG_ARM_GIC
	.handle_irq 	=  gic_handle_irq,
#else
	.handle_irq 	=  vic_handle_irq,
#endif
	.timer			= &nxp_cpu_sys_timer,
	.init_machine	=  cpu_init_machine,
#if defined CONFIG_CMA && defined CONFIG_ION
	.reserve		=  cpu_mem_reserve,
#endif
MACHINE_END
