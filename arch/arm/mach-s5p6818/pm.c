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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/cp15.h>
#include <asm/cacheflush.h>
#include <asm/suspend.h>
#include <asm/memory.h>
#include <asm/system.h>
#include <asm/sections.h> 	/*_stext, _end*/
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/hardware/pl080.h>
#include <mach/platform.h>
#include <mach/pm.h>
#include <mach/gpio.h>
#include <asm/hardware/gic.h>

#define INTC_BASE		(void __iomem *)IO_ADDRESS(PHY_BASEADDR_INTC)
#define GIC_DIST_BASE		(void __iomem *)(INTC_BASE + 0x00001000)		// 0xC0009000

#define SRAM_SAVE_SIZE		(0x10000)	/* 64 Kbyte */

static unsigned int  sramsave[SRAM_SAVE_SIZE/4];
static unsigned int *sramptr;
static unsigned int  sram_length = SRAM_SAVE_SIZE;
bool pm_suspend_enter = false;

#define	FLUSH_CACHE()	do { flush_cache_all();outer_flush_all(); } while(0);

void (*nxp_board_pm_mark)(struct suspend_mark_up *mark, int suspend) = NULL;
void (*core_do_suspend)(ulong, ulong) = NULL;

struct save_gpio {
	unsigned long data;			/* 0x00 */
	unsigned long output;		/* 0x04 */
	unsigned long alfn[2];		/* 0x20, 0x24 */
	unsigned long mode[3];		/* 0x08, 0x0C, 0x28 */
	unsigned long mask;			/* 0x10, 0x3C */
	unsigned long reg_val[10];  /* 0x40 ~ 0x64 */
};

struct save_alive {
	unsigned long  detmod[6];
	unsigned long  detenb;
	unsigned long  irqenb;
	unsigned long  outenb;
	unsigned long  outval;
	unsigned long  pullen;
};

struct pm_saved_regs {
	struct save_gpio  gpio[5];	/* A,B,C,D,E */
	struct save_alive alive;
};

static struct pm_saved_regs saved_regs;
static struct board_pm_ops *board_pm = NULL;

#if (0)
#define	PM_SAVE_ADDR	(u32)virt_to_phys(&saved_regs)
#define	PM_SAVE_VIRT	saved_regs
#define	PM_SAVE_SIZE	SUSPEND_SAVE_SIZE
#else
#define	PM_SAVE_ADDR	__pa(_stext)
#define	PM_SAVE_VIRT	_stext
#define	PM_SAVE_SIZE	SUSPEND_SAVE_SIZE
#endif

#define	SUSPEND_STATUS(s)	(SUSPEND_SUSPEND == s ? "suspend" : "resume")

unsigned int st_wake_events = 0;	/* VDDTOGLE, RTC, ALIVE0, 1, ... */
static const char * __wake_event_name [] = {
	[0] = "VDDPWRTOGGLE",
	[1] = "RTC",
	[2] = "ALIVE 0",
	[3] = "ALIVE 1",
	[4] = "ALIVE 2",
	[5] = "ALIVE 3",
	[6] = "ALIVE 4",
	[7] = "ALIVE 5",
	[8] = "ALIVE 6",
	[9] = "ALIVE 7",
};
#define	WAKE_EVENT_NUM	ARRAY_SIZE(__wake_event_name)

#define	POWER_KEY_MASK		(0x3FC)
#define	RTC_ALARM_INTENB	(0x010)
#define	RTC_ALARM_INTPND	(0x014)


static unsigned long gpio_alfn[5][2];
static int prepare_gpio_suspend(void)
{
	int i, size = 5;

	printk("%s:\n", __func__);

	for (i = 0; size > i; i++) {
		int j;

		for (j = 0; j < GPIO_NUM_PER_BANK/2; j++)
			gpio_alfn[i][0] |= (GET_GPIO_ALTFUNC(i, j) << (j<<1));
		for (j = 0; j < GPIO_NUM_PER_BANK/2; j++)
			gpio_alfn[i][1] |= (GET_GPIO_ALTFUNC(i, j+16) << (j<<1));

		printk("  alfn[%d][0]: 0x%08lx, alfn[%d][1]: 0x%08lx\n",
				i, gpio_alfn[i][0], i, gpio_alfn[i][1]);
	}

	return 0;
}

void watchdog_clear(void)
{
	NX_WDT_Initialize();
	NX_WDT_SetBaseAddress(0, (void*)IO_ADDRESS(NX_WDT_GetPhysicalAddress(0)));
	NX_WDT_OpenModule(0);

	// watchdog disable
	NX_WDT_SetEnable(0, CFALSE);
	NX_WDT_SetResetEnable(0, CFALSE);
	NX_WDT_ClearInterruptPending(0, NX_WDT_GetInterruptNumber(0));
}

static int suspend_machine(void)
{
	const u32 pads[][2] = {
		{ CFG_PWR_WAKEUP_SRC_ALIVE0, CFG_PWR_WAKEUP_MOD_ALIVE0 },
		{ CFG_PWR_WAKEUP_SRC_ALIVE1, CFG_PWR_WAKEUP_MOD_ALIVE1 },
		{ CFG_PWR_WAKEUP_SRC_ALIVE2, CFG_PWR_WAKEUP_MOD_ALIVE2 },
		{ CFG_PWR_WAKEUP_SRC_ALIVE3, CFG_PWR_WAKEUP_MOD_ALIVE3 },
		{ CFG_PWR_WAKEUP_SRC_ALIVE4, CFG_PWR_WAKEUP_MOD_ALIVE4 },
		{ CFG_PWR_WAKEUP_SRC_ALIVE5, CFG_PWR_WAKEUP_MOD_ALIVE5 },
	};

	u32 rtc = IO_ADDRESS(PHY_BASEADDR_RTC);
	int ret = 0, i = 0, n = 0;
	int mask_bits = 0;
	CBOOL MODE;

	NX_ALIVE_SetWriteEnable(CTRUE);
	NX_ALIVE_ClearWakeUpStatus();
	NX_ALIVE_ClearInterruptPendingAll();

	/*
	 * set wakeup device
	 */
	for (i = 0; sizeof(pads)/sizeof(pads[0]) > i; i++) {
		if ((pads[i][0] == CFALSE) || (pads[i][1] > PWR_DECT_BOTHEDGE))
			continue;

		mask_bits |= pads[i][0] ? (1 << i) : 0;

		if (pads[i][1] < PWR_DECT_BOTHEDGE) {
			for (n = 0; PWR_DECT_BOTHEDGE > n; n++) {
				MODE = (n == pads[i][1]) ? CTRUE : CFALSE;
				NX_ALIVE_SetDetectMode(n, i, MODE);
			}
		} else {  /* both edge */
			NX_ALIVE_SetDetectMode(PWR_DECT_FALLINGEDGE, i, CTRUE);
			NX_ALIVE_SetDetectMode(PWR_DECT_RISINGEDGE , i, CTRUE);
		}
	}

	NX_ALIVE_SetInputEnable32(mask_bits);
	NX_ALIVE_SetDetectEnable32(mask_bits);
	NX_ALIVE_SetInterruptEnable32(mask_bits);

	/* disable alarm wakeup */
#if !(PM_RTC_WAKE)
	writel(readl(rtc + RTC_ALARM_INTENB) & ~(3<<0), (rtc + RTC_ALARM_INTENB));
	writel(readl(rtc + RTC_ALARM_INTPND) & ~(3<<0), (rtc + RTC_ALARM_INTPND));
#endif

	/*
	 * check wakeup event(alive, alarm)
	 * before enter sleep mode
	 */
	if (NX_ALIVE_GetInterruptPending32() & mask_bits)
		return -EINVAL;

	if (readl(rtc + RTC_ALARM_INTPND) &
		readl(rtc + RTC_ALARM_INTENB) & (1<<1))
		return -EINVAL;

	/*
	 * wakeup from board.
	 */
	if (board_pm && board_pm->poweroff)
		ret = board_pm->poweroff();

	if (ret < 0)
		return ret;

	return 0;
}

static int resume_machine(void)
{
	u32 status, alive = 0, alarm = 0;
	PM_DBGOUT("%s\n", __func__);

	NX_ALIVE_SetWriteEnable(CTRUE);

	status = NX_ALIVE_GetWakeUpStatus();
	status = 0;	/* TEST */

	/* recheck */
	if (!status) {
		u32 rtc = IO_ADDRESS(PHY_BASEADDR_RTC);
		alive = NX_ALIVE_GetInterruptPending32();
		alarm = readl(rtc + RTC_ALARM_INTPND) &
				readl(rtc + RTC_ALARM_INTENB) & (1<<1);
		status = (alive << 2) | (alarm ? (1<<1) : 0);
	}

	/* set wake event */
	st_wake_events = status & ((1<<WAKE_EVENT_NUM) - 1);

	/* reset machine */
	nxp_cpu_arch_init();

	if (board_pm && board_pm->poweron)
		board_pm->poweron();

	return 0;
}

static void print_wake_event(void)
{
	int i = 0;
	for (i = 0; WAKE_EVENT_NUM > i; i++) {
		if (st_wake_events & 1<<i)
			printk("%s WAKE [%s]\n", __func__, __wake_event_name[i]);
	}
}

static void suspend_cpu_enter(void)
{
	struct save_gpio *gpio = saved_regs.gpio;
	unsigned int base = IO_ADDRESS(PHY_BASEADDR_GPIOA);
	int gic_irqs  = NR_IRQS;
	int i = 0, size = 5;

	for (i = 0; size > i; i++, gpio++, base += 0x1000) {
		if (i == 1) { // except UART 4,5 setting
			writel(gpio_alfn[i][0], (base+0x20));
			writel((gpio_alfn[i][1]&0x33ffffff)|(readl(base+0x24)&0xcc000000), (base+0x24));
			writel(readl(base+0x04)&0xa0000000, (base+0x04));	/* Input */
			writel(readl(base+0x58)&0xa0000000, (base+0x58));	/* GPIOx_PULLSEL - Down */
			writel(readl(base+0x60)&0xa0000000, (base+0x60));	/* GPIOx_PULLENB - Disable */
		} else if (i == 3) { // except UART 0,1,2,3 setting
			writel(gpio_alfn[i][0], (base+0x20));
			writel((gpio_alfn[i][1]&0xfffff00f)|(readl(base+0x24)&0xff0), (base+0x24));
			writel(readl(base+0x04)&0x3c0000, (base+0x04));	/* Input */
			writel(readl(base+0x58)&0x3c0000, (base+0x58));	/* GPIOx_PULLSEL - Down */
			writel(readl(base+0x60)&0x3c0000, (base+0x60));	/* GPIOx_PULLENB - Disable */
		} else {
			writel(gpio_alfn[i][0], (base+0x20));
			writel(gpio_alfn[i][1], (base+0x24));
			writel(0, (base+0x04));	/* Input */
			writel(0, (base+0x58));	/* GPIOx_PULLSEL - Down */
			writel(0, (base+0x60)); /* GPIOx_PULLENB - Disable */
		}
	}

	for (i = 32; i < gic_irqs; i += 4)
		writel_relaxed(0x00, GIC_DIST_BASE + GIC_DIST_TARGET + i * 4 / 4);
}

static void suspend_cores(suspend_state_t stat)
{
#ifndef CONFIG_S5P6818_PM_IDLE
	if (SUSPEND_SUSPEND == stat) {
		NX_CLKPWR_SetBaseAddress((void*)IO_ADDRESS(NX_CLKPWR_GetPhysicalAddress()));
		NX_CLKPWR_SetCPUResetMode(NX_CLKPWR_CPU_RESETMODE_SAFE);
		NX_CLKPWR_SetCPUPowerOn32(0x00);
	#if (0)
	{
		volatile int temp;
		int cpu, num = NR_CPUS;
		unsigned int lv

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

		dmb();
		for (cpu = 1; num > cpu; cpu++) {
			NX_CLKPWR_SetCPUPowerOff(cpu);
			do {
				temp = NX_CLKPWR_GetCPUPowerOnStatus(cpu);
			} while (temp);
			PM_DBGOUT("Power off cpu.%d\n", cpu);
		}
	}
	#endif
	}
#endif
}

#define CHKSTRIDE	(8)
static inline unsigned int __calc_crc(void *addr, int len)
{
	u32 *c = (u32*)addr;
	u32 crc = 0, chkcnt = ((len+3)/4);
	int i, n;

	for (i = 0; chkcnt > i; i += CHKSTRIDE, c += CHKSTRIDE) {
		u32 dat = *c;
		crc ^= dat;
		for(n = 0; 32 > n; n++) {
			if(crc & 0x01) crc ^= (0x04C11DB7L);
			crc >>= 1;
		}
	}
	return crc;
}

#ifndef CONFIG_S5P6818_PM_IDLE
static void suspend_mark(suspend_state_t stat)
{
	struct suspend_mark_up mark = {
		.resume_fn = (u32)virt_to_phys(cpu_resume),
		.signature = SUSPEND_SIGNATURE,
		.save_phy_addr = PM_SAVE_ADDR,
		.save_phy_len = PM_SAVE_SIZE,
	};

	writel((-1UL), SCR_WAKE_FN_RESET);
	writel((-1UL), SCR_CRC_PHY_RESET);
	writel((-1UL), SCR_CRC_RET_RESET);
	writel((-1UL), SCR_CRC_LEN_RESET);
	writel((-1UL), SCR_SIGNAGURE_RESET);

	if (SUSPEND_SUSPEND == stat) {
		u32 len = mark.save_phy_len;
		mark.save_crc_ret = __calc_crc((void*)PM_SAVE_VIRT, len);
	}

	if (nxp_board_pm_mark) {
		nxp_board_pm_mark(&mark, (SUSPEND_SUSPEND == stat ? 1: 0));
		return;
	}

	if (SUSPEND_SUSPEND == stat) {
		PM_DBGOUT("%s Suspend CRC [phy=0x%08x, calc=0x%08x, len=%d]\n",
			__func__, mark.save_phy_addr, mark.save_crc_ret, mark.save_phy_len);
		writel(mark.signature, SCR_SIGNAGURE_SET);
		writel(mark.resume_fn , SCR_WAKE_FN_SET);
		writel(mark.save_phy_addr, SCR_CRC_PHY_SET);
		writel(mark.save_crc_ret, SCR_CRC_RET_SET);
		writel(mark.save_phy_len, SCR_CRC_LEN_SET);
	}
}
#endif

static void suspend_gpio(suspend_state_t stat)
{
	struct save_gpio *gpio = saved_regs.gpio;
	unsigned int base = IO_ADDRESS(PHY_BASEADDR_GPIOA);
	int i = 0, j = 0, size = 5;

	if (SUSPEND_SUSPEND == stat) {
		for (i = 0; size > i; i++, gpio++, base += 0x1000) {
			gpio->data    = readl(base+0x00);
			gpio->output  = readl(base+0x04);
			gpio->alfn[0] = readl(base+0x20);
			gpio->alfn[1] = readl(base+0x24);
			gpio->mode[0] = readl(base+0x08);
			gpio->mode[1] = readl(base+0x0C);
			gpio->mode[2] = readl(base+0x28);
			gpio->mask = readl(base+0x10);

			for (j = 0; j < 10; j++)
				gpio->reg_val[j] = readl(base+0x40+(j<<2));

			writel((-1UL), (base+0x14));	/* clear pend */
		}

		/*
		 * Set GPIO input mode, when suspending
		 */
		gpio = saved_regs.gpio;
		base = IO_ADDRESS(PHY_BASEADDR_GPIOA);
	} else {
		for (i = 0; size > i; i++, gpio++, base += 0x1000) {
#ifndef CONFIG_S5P6818_PM_IDLE
			for (j = 0; j < 10; j++)
				writel(gpio->reg_val[j], (base+0x40+(j<<2)));
#endif
			writel(gpio->output, (base+0x04));
			writel(gpio->data,   (base+0x00));
			writel(gpio->alfn[0],(base+0x20));
			writel(gpio->alfn[1],(base+0x24));
			writel(gpio->mode[0],(base+0x08));
			writel(gpio->mode[1],(base+0x0C)),
			writel(gpio->mode[2],(base+0x28));
			writel(gpio->mask,   (base+0x10));
			writel(gpio->mask,   (base+0x3C));
			writel((-1UL),       (base+0x14));	/* clear pend */
		}
	}
}

static void suspend_alive(suspend_state_t stat)
{
	struct save_alive *alive = &saved_regs.alive;
	unsigned int base = IO_ADDRESS(PHY_BASEADDR_ALIVE);
	int i = 0;

	NX_ALIVE_SetWriteEnable(CTRUE);
	if (SUSPEND_SUSPEND == stat) {
		for (i = 0; 6 > i; i++)
			alive->detmod[i] = readl(base + (i*0x0C) + 0x0C);

		alive->detenb = readl(base + 0x54);
		alive->irqenb = readl(base + 0x60);
		alive->outenb = readl(base + 0x7C);
		alive->outval = readl(base + 0x94);
		alive->pullen = readl(base + 0x88);
	} else {
		if (alive->outenb != readl(base + 0x7C)) {
			writel((-1UL), base + 0x74);		/* reset */
			writel(alive->outenb, base + 0x78);	/* set */
		}
		if (alive->outval != readl(base + 0x94)) {
			writel((-1UL), base + 0x8C);		/* reset */
			writel(alive->outval, base + 0x90);	/* set */
		}
		if (alive->pullen != readl(base + 0x88)) {
			writel((-1UL), base + 0x80);		/* reset */
			writel(alive->pullen, base + 0x84);	/* set */
		}

		writel((-1UL), base + 0x4C);			/* reset */
		writel((-1UL), base + 0x58);			/* reset */

		for (i = 0; 6 > i; i++) {
			writel((-1UL), base + (i*0x0C) + 0x04);			/* reset */
			writel(alive->detmod[i], base + (i*0x0C) + 0x08);	/* set */
		}

		writel(alive->detenb, base + 0x50);	/* set */
		writel(alive->irqenb, base + 0x5C);	/* set */
	}
}

static void suspend_clock(suspend_state_t stat)
{
	if (SUSPEND_RESUME == stat)
		nxp_cpu_clock_resume();
}

static void __power_prepare(void)
{
	unsigned int *src = sramsave;
	unsigned int *dst = sramptr;
	int i = 0;

	for( ; ARRAY_SIZE(sramsave) > i; i++)
		dst[i] = src[i];
}

static int __power_down(unsigned long arg)
{
	int ret = suspend_machine();

#ifndef CONFIG_S5P6818_PM_IDLE
	void (*power_down)(ulong, ulong) =
		(void (*)(ulong, ulong))((ulong)core_do_suspend + 0x220);;
#endif

#ifdef CONFIG_S5P6818_PM_IDLE
	lldebugout("Go to IDLE...\n");
#endif

	FLUSH_CACHE();
	if (0 > ret)
		return ret;	/* wake up */

#ifdef CONFIG_S5P6818_PM_IDLE
	cpu_do_idle();
#else
	if(core_do_suspend == NULL) {
		lldebugout("Fail, inavalid suspend callee\n");
		return 0;
	}

	suspend_cpu_enter();

	lldebugout("suspend machine\n");

	dmb();
	power_down(IO_ADDRESS(PHY_BASEADDR_ALIVE), IO_ADDRESS(PHY_BASEADDR_DREX));
	nop(); nop(); nop();
	dmb();

#endif
	return 0;
}

/*
 * Suspend ops functions
 */

/* return : 0 = wake up, 1 = goto suspend */
static int suspend_valid(suspend_state_t state)
{
	int ret = 1;
	/* clear */
	st_wake_events = 0;

#ifdef CONFIG_SUSPEND
	if (!suspend_valid_only_mem(state)) {
		printk(KERN_ERR "%s: not supported state(%d)\n", __func__, state);
		return 0;
	}
#endif
	if (board_pm && board_pm->valid)
		ret = board_pm->valid(state);

	PM_DBGOUT("%s %s\n", __func__, ret ? "DONE":"WAKE");
	return ret;
}

/* return : 0 = goto suspend, 1 = wake up */
static int suspend_begin(suspend_state_t state)
{
	int ret = 0;
	if (board_pm && board_pm->begin)
		ret = board_pm->begin(state);

	PM_DBGOUT("%s %s\n", __func__, ret ? "WAKE":"DONE");
	return 0;
}

/* return : 0 = goto suspend, 1 = wake up */
static int suspend_prepare(void)
{
	int ret = 0;
	if (board_pm && board_pm->prepare)
		ret = board_pm->prepare();

	if (0 == ret) {
		__power_prepare();
		pm_suspend_enter = true;
	}

	PM_DBGOUT("%s %s\n", __func__, ret ? "WAKE":"DONE");
	return ret;
}

/* return : 0 = goto suspend, 1 = wake up */
#if !defined (CONFIG_ARCH_S5P6818_REV)
extern void clear_fault_bad(int cpu);
#endif

static int suspend_enter(suspend_state_t state)
{
	int ret = 0;
	lldebugout("%s enter\n", __func__);

	if (board_pm && board_pm->enter) {
		if ((ret = board_pm->enter(state)))
			return ret;
	}

	suspend_clock(SUSPEND_SUSPEND);
	suspend_gpio(SUSPEND_SUSPEND);
	suspend_alive(SUSPEND_SUSPEND);

#ifndef CONFIG_S5P6818_PM_IDLE
	suspend_mark(SUSPEND_SUSPEND);
#endif

	/* SMP power down */
	suspend_cores(SUSPEND_SUSPEND);

	/*
	 * goto sleep
	 */
	cpu_suspend(0, __power_down);

	lldebugout("resume machine\n");

	/*
	 * Wakeup status
	 */
#ifndef CONFIG_S5P6818_PM_IDLE
	suspend_mark(SUSPEND_RESUME);
#endif

	resume_machine();

	suspend_alive(SUSPEND_RESUME);
	suspend_gpio(SUSPEND_RESUME);
	suspend_clock(SUSPEND_RESUME);

	suspend_cores(SUSPEND_RESUME);	/* last */

	/* print wakeup evnet */
	print_wake_event();

	return 0;
}

static void suspend_finish(void)
{
	PM_DBGOUT("%s\n", __func__);
	if (board_pm && board_pm->finish)
		board_pm->finish();
	pm_suspend_enter = false;

	watchdog_clear();
}

static void suspend_end(void)
{
	PM_DBGOUT("%s\n", __func__);
	if (board_pm && board_pm->end)
		board_pm->end();
}

static struct platform_suspend_ops suspend_ops = {
	.valid      = suspend_valid,     /* first suspend call */
	.begin      = suspend_begin,     /* before driver suspend */
	.prepare    = suspend_prepare,   /* after driver suspend */
	.enter      = suspend_enter,     /* goto suspend */
	.finish     = suspend_finish,    /* before driver resume */
	.end        = suspend_end,       /* after driver resume */
};

static int __init suspend_ops_init(void)
{
	unsigned int *src, *dst;
	int i = 0;

	sramptr = (unsigned int*)ioremap(0xFFFF0000, sram_length);
	src = sramptr;
	dst = sramsave;

	/* save sram data */
	for(i = 0; ARRAY_SIZE(sramsave) > i; i++)
		dst[i] = src[i];

	suspend_set_ops(&suspend_ops);
	/* prepare GPIO input mode, when suspending */
	prepare_gpio_suspend();

#ifndef CONFIG_S5P6818_PM_IDLE
	core_do_suspend = __arm_ioremap_exec(0xffff0000, 0x10000, 0);
	if (!core_do_suspend)
		printk("Fail, ioremap for suspend callee\n");
#endif
	return 0;
}
core_initcall(suspend_ops_init);

/*
 * 	cpu board suspend fn
 */
void nxp_board_pm_register(struct board_pm_ops *ops)
{
    board_pm = ops;
}

/*
 * 	cpu wakeup source
 */
int nxp_check_pm_wakeup_alive(int num)
{
	int grp = PAD_GET_GROUP(num);
	int io  = PAD_GET_BITNO(num);

	if (PAD_GET_GROUP(PAD_GPIO_ALV) != grp)
		return 0;

	return (st_wake_events & 1<<(io+2)) ? 1 : 0;
}
EXPORT_SYMBOL(nxp_check_pm_wakeup_alive);

int nxp_check_pm_wakeup_dev(char *dev, int io)
{
	printk("Check PM wakeup : %s, io[%d]\n", dev, io);
	return nxp_check_pm_wakeup_alive(io);
}
EXPORT_SYMBOL(nxp_check_pm_wakeup_dev);
