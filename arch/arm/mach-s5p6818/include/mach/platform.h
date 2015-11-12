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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef __ASSEMBLY__
#include <linux/version.h>
#include <generated/autoconf.h>

#include <cfg_type.h>
#include <cfg_gpio.h>
#include <cfg_main.h>
#include <cfg_mem.h>
#endif

#include "s5p6818.h"
#include <mach/iomap.h>
#include <mach/io.h>

/*------------------------------------------------------------------------------
 * Interrupt pend register offset (entry-macro.S)
 * refer to "mach/s5p6818_iomap.h"
 */
#define	PB_INTC_PHYS_BASE	 	PHY_BASEADDR_INTC
#define	PB_INTC_VIRT_BASE		IO_ADDRESS(PB_INTC_PHYS_BASE)

/*------------------------------------------------------------------------------
 * For low level uart debug (debug-macro.S)
 */
#if defined(CONFIG_DEBUG_LL) && defined(CONFIG_DEBUG_NXP_UART)
	#if	 defined (CONFIG_DEBUG_NXP_UART_CH0)
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART0
	#elif defined (CONFIG_DEBUG_NXP_UART_CH1)
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART1
	#elif defined (CONFIG_DEBUG_NXP_UART_CH2)
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART2
	#elif defined(CONFIG_DEBUG_NXP_UART_CH3)
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART3
	#elif defined(CONFIG_DEBUG_NXP_UART_CH4)
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART4
	#elif defined(CONFIG_DEBUG_NXP_UART_CH5)
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART5
	#else
		#define	PB_UART_PHYS_BASE		PHY_BASEADDR_UART0
	#endif

	#define	PB_UART_VIRT_BASE			IO_ADDRESS(PB_UART_PHYS_BASE)
#endif	/* CONFIG_DEBUG_LL && CONFIG_DEBUG_LL_UART */

/*------------------------------------------------------------------------------
 * Get virtual interrupt number: gpio, alive, dma
 */
#define	PB_PIO_IRQ(nr)		(IRQ_GPIO_START   + nr)
#define	PB_ALV_IRQ(nr)		(IRQ_ALIVE_START  + PAD_GET_BIT(nr))
#define	PB_DMA_IRQ(nr)		(IRQ_DMA_START    + nr)
#define	PB_SYS_IRQ(nr)		(IRQ_SYSCTL_START + nr)

/*------------------------------------------------------------------------------
 * system functions
 */
#ifndef __ASSEMBLY__

void 	 		nxp_cpu_arch_init(void);			/* No init section for suspend */
void __init		nxp_cpu_clock_init(void);
void __init		nxp_cpu_irq_init(void);

void 			nxp_cpu_clock_resume(void);
void			nxp_cpu_clock_print(void);
void 			nxp_cpu_clock_update_pll(int pll);
unsigned int 	nxp_cpu_clock_hz(int type);			/* 0=PLL0, 1=PLL1, 2=FCLK, 3=MCLK, 4=BCLK, 5=PCLK */
void 		 	nxp_cpu_shutdown(void);
void 		 	nxp_cpu_core_shutdown(int core);
void			nxp_cpu_goto_stop(void);
void 		 	nxp_cpu_reset(char str, const char *cmd);
unsigned int 	nxp_cpu_version(void);

void __init 	nxp_cpu_devs_register(void);
void		 	nxp_cpu_periph_clock_register(int id, long ext1, long ext2);
unsigned long 	nxp_cpu_pll_change_frequency(int pllno, unsigned long rate);

extern int 	 nxp_check_pm_wakeup_dev(char *dev, int io);
extern int   nxp_check_pm_wakeup_alive(int alive_no);
extern void  nxp_key_power_event(void);

void 	 		nxp_board_base_init(void);			/* No init section for suspend */
void __init 	nxp_board_devs_register(void);

/* low level debug message */
void  lldebugout(const char *fmt, ...);
void  lltime_start(void);
void  lltime_stop(void);
void  lltime_reset(void);
ulong lltime_get(void);


/* system stop */
#ifndef halt
#define	halt()	{ 											\
		printk(KERN_ERR "%s(%d) : %s system halt ...\n", 	\
		__FILE__, __LINE__, __FUNCTION__); 					\
		do {} while(1);										\
	}
#endif

#if defined(CONFIG_PM_DBGOUT)
extern bool console_suspend_enabled;
#define	PM_DBGOUT(msg...)	do { \
			if (!console_suspend_enabled) printk(msg); else lldebugout(msg);	\
		} while(0)
#else
#define	PM_DBGOUT(msg...)	do {} while (0)
#endif

#ifdef CONFIG_EARLY_PRINTK
extern asmlinkage void early_printk(const char *fmt, ...);
#endif

#ifdef CONFIG_RTC_DRV_NXP
	#ifdef CFG_PWR_WAKEUP_SRC_ALARM
	#define PM_RTC_WAKE	CFG_PWR_WAKEUP_SRC_ALARM
	#endif
#else
	#define PM_RTC_WAKE	CFALSE
#endif
#ifndef PM_RTC_WAKE
#define PM_RTC_WAKE		CFALSE
#endif

#endif	/* __ASSEMBLY__ */
#endif	/* __PLATFORM_H__ */
