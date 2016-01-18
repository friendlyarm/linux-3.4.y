/*
 * (C) Copyright 2010
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
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>

/* SLsiAP soc headers */
#include <mach/platform.h>
#include <mach/devices.h>

#if (0)
#define pr_debug(msg...)		{ printk(KERN_INFO "RTC: " msg); }
#endif

/*------------------------------------------------------------------------------
 * 	local data and macro
 */
#define	RTC_COUNT_BIT	(0)
#define	RTC_ALARM_BIT	(1)

static spinlock_t	rtc_lock;
static int			rtc_enable_irq = 0;
static int			alm_enable_irq = 0;

#define	RTC_TIME_YEAR 	(1970)	/* Jan 1 1970 00:00:00 */
#define RTC_TIME_MAX	0x69546780	// 2025.12.31 00:00:00
#define RTC_TIME_MIN	0x52c35a80	// 2014.01.01 00:00:00
#define RTC_TIME_DFT	0x56863200	// 2016.01.01 08:00:00

static unsigned long	rtc_time_offs;

static void nxp_rtc_setup(void)
{
	unsigned long rtc, curr;
	struct rtc_time rtc_tm;

	pr_debug("%s\n", __func__);

	NX_RTC_Initialize();
	NX_RTC_SetBaseAddress((void*)IO_ADDRESS(NX_RTC_GetPhysicalAddress()));
	NX_RTC_OpenModule();

	NX_RTC_ClearInterruptPendingAll();
	NX_RTC_SetInterruptEnableAll(CFALSE);

	rtc_time_offs = mktime(RTC_TIME_YEAR, 1, 1, 0, 0, 0);

	rtc = NX_RTC_GetRTCCounter();

	curr = rtc + rtc_time_offs;

	if ((curr > RTC_TIME_MAX) || (curr < RTC_TIME_MIN)) {
		
		/* set hw rtc */
		NX_RTC_SetRTCCounterWriteEnable(CTRUE);
		NX_RTC_SetRTCCounter(RTC_TIME_DFT - rtc_time_offs);
		while(NX_RTC_IsBusyRTCCounter()) { ; }

		NX_RTC_SetRTCCounterWriteEnable(CFALSE);

		/* Confirm the write value. */
		while(NX_RTC_IsBusyRTCCounter()) { ; }
	
		rtc = NX_RTC_GetRTCCounter();
	}
	
	rtc_time_to_tm(rtc + rtc_time_offs, &rtc_tm);
	printk("[RTC] day=%04d.%02d.%02d time=%02d:%02d:%02d\n",
		 rtc_tm.tm_year + 1900, rtc_tm.tm_mon + 1, rtc_tm.tm_mday,
		 rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
}

static int nxp_rtc_irq_enable(int alarm, struct device *dev, unsigned int enable)
{
	int bit = alarm ? RTC_ALARM_BIT : RTC_COUNT_BIT;
	pr_debug("%s %s (enb:%d)\n", __func__, alarm?"alarm":"count", enable);

	if(enable)
		NX_RTC_SetInterruptEnable(bit, CTRUE);
	else
		NX_RTC_SetInterruptEnable(bit, CFALSE);

	if (alarm)
		alm_enable_irq = enable;
	else
		rtc_enable_irq = enable ;

	return 0;
}

static irqreturn_t nxp_rtc_interrupt(int irq, void *id)
{
	struct rtc_device *rtc = (struct rtc_device *)id;
	int pend = NX_RTC_GetInterruptPending32();

	if(rtc_enable_irq && (pend & (1 << RTC_COUNT_BIT)))	{
		rtc_update_irq(rtc, 1, RTC_PF | RTC_UF | RTC_IRQF);
		NX_RTC_ClearInterruptPending(RTC_COUNT_BIT);
		pr_debug("IRQ: RTC Count (PND:0x%x, ENB:%d)\n", pend, NX_RTC_GetInterruptEnable(RTC_COUNT_BIT));
		return IRQ_HANDLED;
	}

	if(alm_enable_irq && (pend & (1 << RTC_ALARM_BIT)))	{
		rtc_update_irq(rtc, 1, RTC_AF | RTC_IRQF);
		NX_RTC_ClearInterruptPending(RTC_ALARM_BIT);
		pr_debug("IRQ: RTC Alarm (PND:0x%x, ENB:%d)\n", pend, NX_RTC_GetInterruptEnable(RTC_ALARM_BIT));
		return IRQ_HANDLED;
	}

	printk("IRQ: RTC Unknown (PND:0x%x, RTC ENB:%d, ALARM ENB:%d)\n", pend,
		NX_RTC_GetInterruptEnable(RTC_COUNT_BIT), NX_RTC_GetInterruptEnable(RTC_ALARM_BIT));

	NX_RTC_ClearInterruptPendingAll();
	return IRQ_NONE;
}

/*
 * 	RTC OPS
 */
static int nxp_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	unsigned long rtc;

	spin_lock_irq(&rtc_lock);

	rtc = NX_RTC_GetRTCCounter();
	rtc_time_to_tm(rtc + rtc_time_offs, tm);
	pr_debug("read time day=%04d.%02d.%02d time=%02d:%02d:%02d, rtc 0x%x\n",
		 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec, (uint)rtc);

	spin_unlock_irq(&rtc_lock);

	return 0;
}

static int nxp_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	unsigned long rtc, curr_sec;

	spin_lock_irq(&rtc_lock);

	curr_sec = mktime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	             tm->tm_hour, tm->tm_min, tm->tm_sec);

	rtc = curr_sec - rtc_time_offs;

	pr_debug("set time day=%02d.%02d.%02d time=%02d:%02d:%02d, rtc 0x%x\n",
		 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec, (uint)rtc);

	/* set hw rtc */
	NX_RTC_SetRTCCounterWriteEnable(CTRUE);
	NX_RTC_SetRTCCounter(rtc);
	while(NX_RTC_IsBusyRTCCounter()) { ; }

	NX_RTC_SetRTCCounterWriteEnable(CFALSE);

	/* Confirm the write value. */
	while(NX_RTC_IsBusyRTCCounter()) { ; }

	spin_unlock_irq(&rtc_lock);
	return 0;
}

static int nxp_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time;
	unsigned long count;

	spin_lock_irq(&rtc_lock);

	count = NX_RTC_GetAlarmCounter();

	rtc_time_to_tm(count + rtc_time_offs, tm);

	alrm->enabled = (CTRUE == NX_RTC_GetInterruptEnable (RTC_ALARM_BIT) ? 1 : 0);
	alrm->pending = (CTRUE == NX_RTC_GetInterruptPending(RTC_ALARM_BIT) ? 1 : 0);

	pr_debug("read alarm day=%04d.%02d.%02d time=%02d:%02d:%02d, alarm 0x%08x\n",
		 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec, (uint)count);

	spin_unlock_irq(&rtc_lock);

	return 0;
}

static int nxp_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time;
	unsigned long count, seconds;

	spin_lock_irq(&rtc_lock);

	seconds = mktime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec);

	count = seconds - rtc_time_offs;

	pr_debug("set alarm day=%04d.%02d.%02d time=%02d:%02d:%02d, alarm %lu, %s\n",
		 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec, count, alrm->enabled?"ON":"OFF");

	/* set hw rtc */
	while(NX_RTC_IsBusyAlarmCounter()) { ; }

	NX_RTC_SetAlarmCounter(count);

	/* Confirm the write value. */
	while(NX_RTC_IsBusyAlarmCounter()) { ; }

	NX_RTC_ClearInterruptPending(RTC_ALARM_BIT);

	NX_RTC_SetInterruptEnable(RTC_ALARM_BIT, alrm->enabled ? CTRUE : CFALSE);	// 0: RTC Counter, 1: RTC Alarm
	NX_CLKPWR_SetRTCWakeUpEnable(alrm->enabled ? CTRUE : CFALSE);	// set wakeup source form stop mode

	alm_enable_irq = alrm->enabled;

	spin_unlock_irq(&rtc_lock);
	return 0;
}

static int nxp_rtc_open(struct device *dev)
{
	pr_debug("%s\n", __func__);

	return 0;
}

static void nxp_rtc_release(struct device *dev)
{
	pr_debug("%s\n", __func__);
}

static int nxp_rtc_alarm_irq_enable(struct device *dev, unsigned int enable)
{
	pr_debug("%s (enb:%d)\n", __func__, enable);
	return nxp_rtc_irq_enable(1, dev, enable);
}

static int nxp_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	pr_debug("%s cmd=%08x, arg=%08lx \n", __func__, cmd, arg);

	spin_lock_irq(&rtc_lock);

	switch (cmd) {
	case RTC_AIE_OFF: nxp_rtc_irq_enable(1, dev, 0); break;
	case RTC_AIE_ON : nxp_rtc_irq_enable(1, dev, 1); break;
	case RTC_UIE_OFF: nxp_rtc_irq_enable(0, dev, 0); break;
	case RTC_UIE_ON : nxp_rtc_irq_enable(0, dev, 1); break;
	case RTC_IRQP_SET: ret = ENOTTY; break;
	default: ret = -ENOIOCTLCMD;
	}

	spin_unlock_irq(&rtc_lock);
	return ret;
}

/*
 * Provide additional RTC information in /proc/driver/rtc
 */
static int nxp_rtc_proc(struct device *dev, struct seq_file *seq)
{
	CBOOL birq = NX_RTC_GetInterruptEnable(RTC_COUNT_BIT);
	seq_printf(seq, "update_IRQ\t: %s\n", birq ? "yes" : "no");
	seq_printf(seq, "periodic_IRQ\t: %s\n", birq ? "yes" : "no");
	return 0;
}

static const struct rtc_class_ops nxp_rtc_ops = {
	.open				= nxp_rtc_open,
	.release			= nxp_rtc_release,
	.ioctl 				= nxp_rtc_ioctl,
	.read_time			= nxp_rtc_read_time,
	.set_time			= nxp_rtc_set_time,
	.read_alarm			= nxp_rtc_read_alarm,
	.set_alarm			= nxp_rtc_set_alarm,
	.proc				= nxp_rtc_proc,
	.alarm_irq_enable	= nxp_rtc_alarm_irq_enable,
};

/*
 * RTC platform driver functions
 */
static int nxp_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	PM_DBGOUT("+%s (rtc irq:%s, alarm irq:%s, wakeup=%d)\n",
		__func__, rtc_enable_irq?"on":"off", alm_enable_irq?"on":"off", device_may_wakeup(&pdev->dev));


	if(rtc_enable_irq) {
		NX_RTC_ClearInterruptPending(RTC_COUNT_BIT);
		NX_RTC_SetInterruptEnable(RTC_COUNT_BIT, CFALSE);
	}

	if (alm_enable_irq) {
		unsigned long count = NX_RTC_GetAlarmCounter();
		struct rtc_time tm;
		rtc_time_to_tm(count, &tm);
		PM_DBGOUT(" %s (alarm day=%04d.%02d.%02d time=%02d:%02d:%02d, alarm %lu)\n",
			__func__, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, count);
	}

	PM_DBGOUT("-%s\n", __func__);
	return 0;
}

static int nxp_rtc_resume(struct platform_device *pdev)
{
	PM_DBGOUT("+%s (rtc irq:%s, alarm irq:%s)\n",
		__func__, rtc_enable_irq?"on":"off", alm_enable_irq?"on":"off");

	if(rtc_enable_irq) {
		NX_RTC_ClearInterruptPending(RTC_COUNT_BIT);
		NX_RTC_SetInterruptEnable(RTC_COUNT_BIT, CTRUE);
	}
	PM_DBGOUT("-%s\n", __func__);
	return 0;
}

static int __devinit nxp_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	int ret;

	spin_lock_init(&rtc_lock);
	nxp_rtc_setup();

	/* cpu init code should really have flagged this device as
	 * being wake-capable; if it didn't, do that here.
	 */
	if (!device_can_wakeup(&pdev->dev))
		device_init_wakeup(&pdev->dev, 1);

	/* register RTC and exit */
	rtc = rtc_device_register(pdev->name, &pdev->dev, &nxp_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		dev_err(&pdev->dev, "cannot attach rtc\n");
		ret = PTR_ERR(rtc);
		return ret;
	}

	platform_set_drvdata(pdev, rtc);

	/* set rtc frequency value */
	rtc->irq_freq 	   = 1;
	rtc->max_user_freq = 1;

	/* register disabled irq */
	ret = request_irq(IRQ_PHY_CLKPWR_RTCIRQ, nxp_rtc_interrupt, IRQF_DISABLED,  "rtc 1hz", rtc);
	if (ret) {
		dev_err(&pdev->dev, "IRQ%d error %d\n", (uint)IRQ_PHY_CLKPWR_RTCIRQ, ret);
		return ret;
	}
	pr_debug("done: rtc probe ...\n");

	return 0;
}

static int __devexit nxp_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);

	pr_debug("%s\n", __func__);

	free_irq(IRQ_PHY_CLKPWR_RTCIRQ, rtc);

	rtc_device_unregister(rtc);
	platform_set_drvdata(pdev, NULL);

	kfree(rtc);

	return 0;
}

static struct platform_driver rtc_plat_driver = {
	.probe		= nxp_rtc_probe,
	.remove		= __devexit_p(nxp_rtc_remove),
	.suspend	= nxp_rtc_suspend,
	.resume		= nxp_rtc_resume,
	.driver		= {
		.name	= DEV_NAME_RTC,
		.owner	= THIS_MODULE,
	},
};

static int __init nxp_rtc_init(void)
{
	pr_debug("%s\n", __func__);
	return platform_driver_probe(&rtc_plat_driver, nxp_rtc_probe);
}

static void __exit nxp_rtc_exit(void)
{
	pr_debug("%s\n", __func__);
	platform_driver_unregister(&rtc_plat_driver);
}

module_init(nxp_rtc_init);
module_exit(nxp_rtc_exit);

MODULE_DESCRIPTION("RTC driver for the Nexell");
MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rtc");
