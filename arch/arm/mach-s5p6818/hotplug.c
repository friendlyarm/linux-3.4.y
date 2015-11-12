/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <asm/cacheflush.h>
#include <asm/cp15.h>
#include <asm/smp_plat.h>

#include <mach/platform.h>
#include <mach/soc.h>

/*
#define pr_debug 	printk
*/

extern volatile int pen_release;

static void __cpuinit write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static inline void cpu_enter_lowpower(void)
{
	unsigned int lv, mv;
	flush_cache_all();

	asm volatile(
	"	mcr	p15, 0, %1, c7, c5, 0\n"
	"	mcr	p15, 0, %1, c7, c10, 4\n"
	/*
	 * Turn off coherency(dcache off)
	 */
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	bic	%0, %0, %2\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	  : "=&r" (lv)
	  : "r" (0), "Ir" (CR_C)
	  : "cc");

	asm volatile(
	/*
	 * Turn off smp
	 */
	"	mrrc	p15, 0, %0, %1, c15\n"
	"	bic	%0, %0, #0x40\n"
	"	mcrr	p15, 0, %0, %1, c15\n"
	  : "=&r" (lv), "=&r" (mv)
	  : "r" (0)
	  : "cc");
}

static inline void cpu_leave_lowpower(void)
{
	unsigned int v;

	asm volatile(
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	orr	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	orr	%0, %0, #0x20\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	  : "=&r" (v)
	  : "Ir" (CR_C)
	  : "cc");
}

static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
	/*
	 * there is no power-control hardware on this platform, so all
	 * we can do is put the core into WFI; this is safe as the calling
	 * code will have already disabled interrupts
	 */
	for (;;) {
		/*
		 * here's the WFI
		 */
		__asm__ __volatile(
		"    wfi\n"
		  :
		  :
		  : "memory", "cc");

		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}

		/*
		 * Getting here, means that we have come out of WFI without
		 * having been woken up - this shouldn't happen
		 *
		 * Just note it happening - when we're woken, we can report
		 * its occurrence.
		 */
		(*spurious)++;
	}
}

int platform_cpu_kill(unsigned int cpu)
{
	return 1;
}

extern bool pm_suspend_enter;
extern void (*core_do_suspend)(ulong, ulong);

static inline void platform_cpu_lowpower(int cpu)
{
	void (*power_down)(ulong, ulong) = (void*)(core_do_suspend + 0x220);
	int spurious = 0;

	/*
	 * enter WFI when poweroff or restart
	 */
	if (false == pm_suspend_enter) {
		cpu_enter_lowpower();
		platform_do_lowpower(cpu, &spurious);
		halt();
	}

	/*
	 * enter SRAM text when suspend
	 */
	if (NULL == core_do_suspend)
		lldebugout("SMP: Fail, cpu.%d ioremap for suspend callee\n", cpu);

	dmb();
	power_down(IO_ADDRESS(PHY_BASEADDR_ALIVE), IO_ADDRESS(PHY_BASEADDR_DREX));
	nop(); nop(); nop();
	dmb();
}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void platform_cpu_die(unsigned int cpu)
{
	int spurious = 0;

#if !defined (CONFIG_S5P6818_PM_IDLE)
	platform_cpu_lowpower(cpu);
#endif

	/*
	 * we're ready for shutdown now, so do it
	 */
	cpu_enter_lowpower();
	platform_do_lowpower(cpu, &spurious);

	/*
	 * bring this CPU back into the world of cache
	 * coherency, and then restore interrupts
	 */
	cpu_leave_lowpower();

	/* wakeup form idle */
	write_pen_release(-1);
}

int platform_cpu_disable(unsigned int cpu)
{
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */
	pr_debug("%s cpu.%d\n", __func__, cpu);
	return cpu == 0 ? -EPERM : 0;
}
