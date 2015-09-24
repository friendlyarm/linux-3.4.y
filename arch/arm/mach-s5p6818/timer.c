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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <asm/mach/time.h>
#include <asm/mach/irq.h>
#include <asm/smp_twd.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

/*
#define pr_debug 	printk
*/

#define	TIMER_CLOCK_SOURCE_HZ	(10*1000000)	/* 1MHZ */
#define	TIMER_CLOCK_EVENT_HZ	(10*1000000)	/* 1MHZ */

/*
 * Timer HW
 */
#define	TIMER_CFG0		(0x00)
#define	TIMER_CFG1		(0x04)
#define	TIMER_TCON		(0x08)
#define	TIMER_CNTB		(0x0C)
#define	TIMER_CMPB		(0x10)
#define	TIMER_CNTO		(0x14)
#define	TIMER_STAT		(0x44)

#define	TCON_AUTO		(1<<3)
#define	TCON_INVT		(1<<2)
#define	TCON_UP			(1<<1)
#define	TCON_RUN		(1<<0)
#define CFG0_CH(ch)		(ch == 0 || ch == 1 ? 0 : 8)
#define CFG1_CH(ch)		(ch * 4)
#define TCON_CH(ch)		(ch ? ch * 4  + 4 : 0)
#define TINT_CH(ch)		(ch)
#define TINT_CS_CH(ch)	(ch + 5)
#define	TINT_CS_MASK	(0x1F)
#define TIMER_CH_OFFS	(0xC)

#define	TIMER_BASE		IO_ADDRESS(PHY_BASEADDR_TIMER)
#define	TIMER_READ(ch)	(readl(TIMER_BASE + TIMER_CNTO + (TIMER_CH_OFFS * ch)))

int __timer_sys_mux_val = 0;
int __timer_sys_scl_val = 0;
int __timer_sys_clk_clr = 0;

#if	  (CFG_TIMER_SYS_TICK_CH == 0)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN14)
#elif (CFG_TIMER_SYS_TICK_CH == 1)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN0)
#elif (CFG_TIMER_SYS_TICK_CH == 2)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN1)
#elif (CFG_TIMER_SYS_TICK_CH == 3)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN2)
#endif
#define	CLKGEN_ENB		(0x0)
#define	CLKGEN_CLR		(0x4)


static inline void timer_reset(int ch)
{
	if (!nxp_soc_peri_reset_status(RESET_ID_TIMER))
		nxp_soc_peri_reset_set(RESET_ID_TIMER);
}

static inline void timer_clock(int ch, int mux, int scl)
{
	volatile U32 val;

	val  = readl(TIMER_BASE + TIMER_CFG0);
	val &= ~(0xFF   << CFG0_CH(ch));
	val |=  ((scl-1)<< CFG0_CH(ch));
	writel(val, TIMER_BASE + TIMER_CFG0);

	val  = readl(TIMER_BASE + TIMER_CFG1);
	val &= ~(0xF << CFG1_CH(ch));
	val |=  (mux << CFG1_CH(ch));
	writel(val, TIMER_BASE + TIMER_CFG1);
}

static inline void timer_count(int ch, unsigned int cnt)
{
	writel((cnt-1), TIMER_BASE + TIMER_CNTB + (TIMER_CH_OFFS * ch));
	writel((cnt-1), TIMER_BASE + TIMER_CMPB + (TIMER_CH_OFFS * ch));
}

static inline void timer_start(int ch, int irqon)
{
	volatile U32 val;
	int on = irqon ? 1 : 0;

	val  = readl(TIMER_BASE + TIMER_STAT);
	val &= ~(TINT_CS_MASK<<5 | 0x1 << TINT_CH(ch));
	val |=  (0x1 << TINT_CS_CH(ch) | on << TINT_CH(ch));
	writel(val, TIMER_BASE + TIMER_STAT);

	val = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(0xE << TCON_CH(ch));
	val |=  (TCON_UP << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	val &= ~(TCON_UP << TCON_CH(ch));
	val |=  ((TCON_AUTO | TCON_RUN)  << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);
}

static inline void timer_stop(int ch, int irqon)
{
	volatile U32 val;
	int on = irqon ? 1 : 0;

	val  = readl(TIMER_BASE + TIMER_STAT);
	val &= ~(TINT_CS_MASK<<5 | 0x1 << TINT_CH(ch));
	val |=  (0x1 << TINT_CS_CH(ch) | on << TINT_CH(ch));
	writel(val, TIMER_BASE + TIMER_STAT);

	val  = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(TCON_RUN << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);
}

static inline void timer_irq_clear(int ch)
{
	volatile U32 val;
	val  = readl(TIMER_BASE + TIMER_STAT);
	val &= ~(TINT_CS_MASK<<5);
	val |= (0x1 << TINT_CS_CH(ch));
	writel(val, TIMER_BASE + TIMER_STAT);
}

struct timer_info {
	int channel;
	int	irqno;
	struct clk *clk;
	unsigned long rate;
	int tmmux;
	int prescale;
	unsigned long tcount;
	unsigned long rcount;
	int in_tclk;
};

static struct timer_info 	timer_src = { 0, };
static struct timer_info 	timer_evt = { 0, };
#define	tm_source_info()	(&timer_src)
#define	tm_event_info()		(&timer_evt)

/*
 * Timer clock source
 */
static void timer_clock_select(struct timer_info *info, long frequency)
{
	struct clk *clk = NULL;
	char name[16] = CORECLK_NAME_PCLK;
	ulong rate, tout = 0;
	int tscl = 0, tmux = 5;
	int vers = nxp_cpu_version();

#if !defined(CONFIG_NXP_DFS_BCLK)
	int smux = 0, pscl = 0;
	ulong mout;
	ulong thz, delt = (-1UL);

	/* PCLK */
	info->clk = clk_get(NULL, name);
   	rate = clk_get_rate(info->clk);
   	for (smux = 0; 5 > smux; smux++) {
   		mout = rate/(1<<smux), pscl = mout/frequency;
   		thz  = mout/(pscl?pscl:1);
   		if (!(mout % frequency) && 256 > pscl) {
   			tout = thz, tmux = smux, tscl = pscl;
   			break;
   		}
		if (pscl > 256)
			continue;
		if (abs(frequency-thz) >= delt)
			continue;
		tout = thz, tmux = smux, tscl = pscl;
		delt = abs(frequency-thz);
   	}
#endif

	/* CLKGEN */
	if (vers && tout != frequency) {
		sprintf(name, "%s.%d", DEV_NAME_TIMER, info->channel);
		clk  = clk_get(NULL, name);
		rate = clk_round_rate(clk, frequency);
		if (abs(frequency-tout) >= abs(frequency-rate)) {
			tout = clk_set_rate(clk, rate);
			tmux = 5, tscl = 1;
			info->clk = clk, info->in_tclk = 1;
			clk_enable(info->clk);
		} else {
			clk_put(clk);
			rate = clk_get_rate(info->clk);	/* PCLK */
		}
	}

	info->tmmux = tmux;
	info->prescale = tscl;
	info->tcount = tout/HZ;
	info->rate = tout;

	pr_debug("%s (ch:%d, mux=%d, scl=%d, rate=%ld, %s)\n",
		__func__, info->channel, tmux, tscl, tout, info->in_tclk?"TCLK":"PCLK");
}

static void timer_source_suspend(struct clocksource *cs)
{
	struct timer_info *info = tm_source_info();
	int ch = info->channel;

	info->rcount = (info->tcount - TIMER_READ(ch));
	timer_stop(ch, 0);
}

static void timer_source_resume(struct clocksource *cs)
{
	struct timer_info *info = tm_source_info();
	int ch = info->channel;
	ulong flags;

	local_irq_save(flags);

	if (info->in_tclk) {
		clk_set_rate(info->clk, info->rate);
		clk_enable(info->clk);
	}

	timer_reset(ch);
	timer_stop (ch, 0);
	timer_clock(ch, info->tmmux, info->prescale);
	timer_count(ch, info->rcount + 1);	/* restore count */
	timer_start(ch, 0);
	timer_count(ch, info->tcount + 1);	/* next count */

	local_irq_restore(flags);
}

static cycle_t timer_source_read(struct clocksource *cs)
{
	struct timer_info *info = tm_source_info();
	int ch = info->channel;

	info->rcount = (info->tcount - TIMER_READ(ch));
	return (cycle_t)info->rcount;
}

static struct clocksource tm_source_clk = {
	.name 		= "source timer",
 	.rating		= 300,
 	.read		= timer_source_read,
	.mask		= CLOCKSOURCE_MASK(32),
 	.shift 		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
	.suspend	= timer_source_suspend,
	.resume		= timer_source_resume,
};

static int __init timer_source_init(int ch)
{
	struct clocksource *cs = &tm_source_clk;
	struct timer_info *info = tm_source_info();

	info->channel = ch;
	info->irqno = -1;
	timer_clock_select(info, TIMER_CLOCK_SOURCE_HZ);

	/*
	 * register timer source
	 */
	clocksource_register_hz(cs, info->rate);

	pr_debug("timer.%d: source shift =%u  \n", ch, cs->shift);
	pr_debug("timer.%d: source mult  =%u  \n", ch, cs->mult);

	/*
	 * source timer run
	 */
	info->tcount = -1UL;
	timer_reset(ch);
	timer_stop (ch, 0);
	timer_clock(ch, info->tmmux, info->prescale);
	timer_count(ch, info->tcount + 1);
	timer_start(ch, 0);

	__timer_sys_mux_val = info->tmmux;
	__timer_sys_scl_val = info->prescale;
	__timer_sys_clk_clr = readl(TIMER_SYS_CLKGEN + CLKGEN_CLR);
	printk("timer.%d: source, %9lu(HZ:%d), mult:%u\n", ch, info->rate, HZ, cs->mult);
 	return 0;
}

/*
 * Timer clock event
 */
static inline void timer_event_resume(struct timer_info *info)
{
	int ch = info->channel;
	if (info->in_tclk) {
		clk_set_rate(info->clk, info->rate);
		clk_enable(info->clk);
	}
	timer_stop(ch, 1);
	timer_clock(ch, info->tmmux, info->prescale);
}

static void timer_event_set_mode(enum clock_event_mode mode, struct clock_event_device *evt)
{
	struct timer_info *info = tm_event_info();
	int ch = info->channel;
	unsigned long cnt = info->tcount;
	pr_debug("%s (ch:%d, mode:0x%x, cnt:%ld)\n", __func__, ch, mode, cnt);

	switch(mode) {
	case CLOCK_EVT_MODE_UNUSED:		// 0x0
	case CLOCK_EVT_MODE_ONESHOT:	// 0x3
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:	// 0x1
		timer_stop(ch, 0);
		break;
	case CLOCK_EVT_MODE_RESUME:		// 0x4
		timer_event_resume(info);
	case CLOCK_EVT_MODE_PERIODIC:	// 0x2
		timer_stop (ch, 0);
		timer_count(ch, cnt);
		timer_start(ch, 1);
		break;
	default:
		break;
	}
}

static int timer_event_set_next(unsigned long delta, struct clock_event_device *evt)
{
	struct timer_info *info = tm_event_info();
	int ch = info->channel;
	ulong flags;

	pr_debug("%s (ch:%d,delta:%ld)\n", __func__, ch, delta);
	raw_local_irq_save(flags);

	timer_stop (ch, 0);
	timer_count(ch, delta);
	timer_start(ch, 1);

	raw_local_irq_restore(flags);
	return 0;
}

static struct clock_event_device tm_event_clk = {
	.name			= "event timer",
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode		= timer_event_set_mode,
	.set_next_event	= timer_event_set_next,
	.rating			= 250,
};

#define	TIMER_TICK_MSG(ch, cn) 	{	\
		static long count = 0;			\
		if (0 == (count++ % cn))		\
			printk("[cpu.%d ch.%d evt: %6ld]\n", smp_processor_id(), ch, count-1);	\
		}

static irqreturn_t timer_event_handler(int irq, void *dev_id)
{
	struct clock_event_device *evt = &tm_event_clk;
	struct timer_info *info= tm_event_info();
	int ch = info->channel;

	timer_irq_clear(ch);
	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction timer_event_irqaction = {
	.name		= "Event Timer IRQ",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= timer_event_handler,
};

static int __init timer_event_init(int ch)
{
	struct clock_event_device *evt = &tm_event_clk;
	struct timer_info *info = tm_event_info();

	info->channel = ch;
	info->irqno = IRQ_PHY_TIMER_INT0 + ch;

	timer_clock_select(info, TIMER_CLOCK_EVENT_HZ);

	/*
	 * setup timer as free-running clocksource
	 */
	timer_stop (ch, 1);
	timer_clock(ch, info->tmmux, info->prescale);

	/*
	 * Make irqs happen for the system timer
	 */
	setup_irq(info->irqno, &timer_event_irqaction);

	/*
	 * register timer event device
	 */
	clockevents_calc_mult_shift(evt, info->rate, 5);
	evt->max_delta_ns = clockevent_delta2ns(0xffffffff, evt);
	evt->min_delta_ns = clockevent_delta2ns(0xf, evt);
	evt->cpumask = cpumask_of(0);
	evt->irq = info->irqno;
	clockevents_register_device(evt);

	pr_debug("timer.%d: event  shift =%u  \n", ch, evt->shift);
	pr_debug("timer.%d: event  mult  =%u  \n", ch, evt->mult);
	pr_debug("timer.%d: event  max   =%lld\n", ch, evt->max_delta_ns);
	pr_debug("timer.%d: event  min   =%lld\n", ch, evt->min_delta_ns);

	printk("timer.%d: event , %9lu(HZ:%d), mult:%u\n",
		ch, info->rate, HZ, evt->mult);
	return 0;
}

static void __init timer_initialize(void)
{
	pr_debug("%s\n", __func__);

	timer_source_init(CFG_TIMER_SYS_TICK_CH);
	timer_event_init(CFG_TIMER_EVT_TICK_CH);

	return;
}

struct sys_timer nxp_cpu_sys_timer = {
	.init	= timer_initialize,
};

