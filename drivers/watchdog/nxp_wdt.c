/* linux/drivers/watchdog/nxp_wdt.c
 *
 * Copyright (c) 2014 Nexell Co
 *	hyun seok jung <hsjung@nexell.co.kr>
 *
 * NXP Watchdog Timer Support
 *
 * Based on, softdog.c by Alan Cox,
 *     (c) Copyright 1996 Alan Cox <alan@lxorguk.ukuu.org.uk>
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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h> /* for MODULE_ALIAS_MISCDEV */
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#ifdef CONFIG_WDT_TASK
#include <linux/workqueue.h>
#endif

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>
#ifdef CONFIG_NXP_DFS_BCLK
#include <mach/nxp-dfs-bclk.h>
#endif

#define CONFIG_NXP_WATCHDOG_DEFAULT_TIME	(10)
#define CONFIG_NXP_WATCHDOG_MAX_TIME		(10)

#if !defined(CFG_WATCHDOG_MAGICCLOSE_OFF)
#define OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE)
#else
#define OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING)
#endif

#define NXP_WDOGREG(x) ((x) + IO_ADDRESS(PHY_BASEADDR_WDT))

#define NXP_WTCON      NXP_WDOGREG(0x00)
#define NXP_WTDAT      NXP_WDOGREG(0x04)
#define NXP_WTCNT      NXP_WDOGREG(0x08)
#define NXP_WTCLRINT   NXP_WDOGREG(0x0c)

/* the watchdog can either generate a reset pulse, or an
 * interrupt.
 */

#define NXP_WTCON_RSTEN   (0x01)
#define NXP_WTCON_INTEN   (1<<2)
#define NXP_WTCON_ENABLE  (1<<5)

#define NXP_WTCON_DIV16   (0<<3)
#define NXP_WTCON_DIV32   (1<<3)
#define NXP_WTCON_DIV64   (2<<3)
#define NXP_WTCON_DIV128  (3<<3)

#define NXP_WTCON_PRESCALE(x) ((x) << 8)
#define NXP_WTCON_PRESCALE_MASK (0xff00)

static bool nowayout	= WATCHDOG_NOWAYOUT;
static int tmr_margin	= CONFIG_NXP_WATCHDOG_DEFAULT_TIME;
static int soft_noboot;
static int debug;
static unsigned long wdt_freq;

module_param(tmr_margin,  int, 0);
module_param(nowayout,   bool, 0);
module_param(soft_noboot, int, 0);
module_param(debug,	  int, 0);

MODULE_PARM_DESC(tmr_margin, "Watchdog tmr_margin in seconds. (default="
		__MODULE_STRING(CONFIG_NXP_WATCHDOG_DEFAULT_TIME) ")");
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
			__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");
MODULE_PARM_DESC(soft_noboot, "Watchdog action, set to 1 to ignore reboots, "
			"0 to reboot (default 0)");
MODULE_PARM_DESC(debug, "Watchdog debug, set to >1 for debug (default 0)");

static struct device    *wdt_dev;	/* platform device attached to */
static struct resource	*wdt_mem;
static struct resource	*wdt_irq;
static struct clk	*wdt_clock;
static unsigned int	 wdt_count;
static DEFINE_SPINLOCK(wdt_lock);

#ifdef CONFIG_WDT_TASK
static struct workqueue_struct *wdt_wqueue;
static struct delayed_work wdt_task_work;

static void nxp_wdt_task_work(struct work_struct *work)
{
	spin_lock(&wdt_lock);
    writel(0, NXP_WTCLRINT);
	writel(wdt_count, NXP_WTCNT);
	spin_unlock(&wdt_lock);

    queue_delayed_work(wdt_wqueue, &wdt_task_work, msecs_to_jiffies(CONFIG_DEFAULT_WDT_TASK_TIMEOUT-1)*1000);
}
#endif

#ifdef CONFIG_WDT_SYSFS
static ssize_t wdt_show(struct device *dev,
		            struct device_attribute *attr, char *buf)
{
    char *s = buf;

    s += sprintf(s, "%d\n", soft_noboot);
    if (s != buf)
        *(s-1) = '\n';

	return (s - buf);
}

static ssize_t wdt_store(struct device *dev,
		            struct device_attribute *attr, const char *buf, size_t n)
{
    long soft_en = 0;

    sscanf(buf,"%ld", &soft_en);
	soft_noboot = soft_en;

    return n;
}

static struct device_attribute wdt_attr = __ATTR(soft, S_IRUGO | S_IWUSR, wdt_show, wdt_store);

/* sys attribte group */
static struct attribute *attrs[] = { 
	    &wdt_attr.attr, NULL,
};

static struct attribute_group attr_group = { 
	        .attrs = (struct attribute **)attrs,
};
#endif

/* watchdog control routines */
#define DBG(fmt, ...)					\
do {							\
	if (debug)					\
		pr_info(fmt, ##__VA_ARGS__);		\
} while (0)

/* functions */

static int nxp_wdt_keepalive(struct watchdog_device *wdd)
{
	spin_lock(&wdt_lock);
    writel(0, NXP_WTCLRINT);
	writel(wdt_count, NXP_WTCNT);
	spin_unlock(&wdt_lock);

	return 0;
}

static void __nxp_wdt_stop(void)
{
	unsigned long wtcon;

	wtcon = readl(NXP_WTCON);
	wtcon &= ~(NXP_WTCON_ENABLE | NXP_WTCON_RSTEN);
	writel(wtcon, NXP_WTCON);
    writel(0, NXP_WTCLRINT);
}

static int nxp_wdt_stop(struct watchdog_device *wdd)
{
	spin_lock(&wdt_lock);
	__nxp_wdt_stop();
	wdd->irq_data = 0;
	spin_unlock(&wdt_lock);

	return 0;
}

static int nxp_wdt_start(struct watchdog_device *wdd)
{
	unsigned long wtcon;

	spin_lock(&wdt_lock);

	__nxp_wdt_stop();

	wtcon = readl(NXP_WTCON);
	wtcon |= NXP_WTCON_ENABLE | NXP_WTCON_DIV128;

	if (soft_noboot) {
		wtcon |= NXP_WTCON_INTEN;
		wtcon &= ~NXP_WTCON_RSTEN;
	} else {
		wtcon |= NXP_WTCON_INTEN;
		wtcon |= NXP_WTCON_RSTEN;
	}

	DBG("%s: wdt_count=0x%08x, wtcon=%08lx\n",
	    __func__, wdt_count, wtcon);

	writel(wdt_count, NXP_WTDAT);
	writel(wdt_count, NXP_WTCNT);
	writel(wtcon, NXP_WTCON);
	wdd->irq_data = 0;
	spin_unlock(&wdt_lock);

	return 0;
}

static inline int nxp_wdt_is_running(void)
{
	return readl(NXP_WTCON) & NXP_WTCON_ENABLE;
}

static int nxp_wdt_set_heartbeat(struct watchdog_device *wdd, unsigned timeout)
{
	unsigned long freq = wdt_freq;
	unsigned int count;
	unsigned int divisor = 1;
	unsigned long wtcon;


	if ((timeout < 1) || (timeout > CONFIG_NXP_WATCHDOG_MAX_TIME)){
		dev_err(wdt_dev, "timeout %d invalid value\n", timeout);
		return -EINVAL;
	}

	freq /= 128;
	count = timeout * freq;

	DBG("%s: count=%d, timeout=%d, freq=%lu\n",
	    __func__, count, timeout, freq);

	/* if the count is bigger than the watchdog register,
	   then work out what we need to do (and if) we can
	   actually make this value
	*/

	if (count >= 0x10000) {
		for (divisor = 1; divisor <= 0x100; divisor++) {
			if ((count / divisor) < 0x10000)
				break;
		}

		if ((count / divisor) >= 0x10000) {
			dev_err(wdt_dev, "timeout %d too big\n", timeout);
			return -EINVAL;
		}
	}

	DBG("%s: timeout=%d, divisor=%d, count=%d (%08x)\n",
	    __func__, timeout, divisor, count, count/divisor);

	count /= divisor;
	wdt_count = count;

	/* update the pre-scaler */
	wtcon = readl(NXP_WTCON);
	wtcon &= ~NXP_WTCON_PRESCALE_MASK;
	wtcon |= NXP_WTCON_PRESCALE(divisor-1);

	writel(count, NXP_WTDAT);
	writel(wtcon, NXP_WTCON);

	wdd->timeout = timeout;

	return 0;
}

static const struct watchdog_info nxp_wdt_ident = {
	.options          =     OPTIONS,
	.firmware_version =	0,
	.identity         =	"NXP Watchdog",
};

static struct watchdog_ops nxp_wdt_ops = {
	.owner = THIS_MODULE,
	.start = nxp_wdt_start,
	.stop = nxp_wdt_stop,
	.ping = nxp_wdt_keepalive,
	.set_timeout = nxp_wdt_set_heartbeat,
};

static struct watchdog_device nxp_wdd = {
	.info = &nxp_wdt_ident,
	.ops = &nxp_wdt_ops,
};

/* interrupt handler code */

static irqreturn_t nxp_wdt_irq(int irqno, void *param)
{
	if(soft_noboot)
		nxp_wdt_keepalive(&nxp_wdd);
	
	nxp_wdd.irq_data = 1;
	wake_up_interruptible(&nxp_wdd.irq_queue);

	return IRQ_HANDLED;
}

#ifdef CONFIG_NXP_DFS_BCLK
static int nxp_wdt_bclk_dfs_transition(struct notifier_block *nb,
					  unsigned long val, void *data)
{
	int ret;

	DBG("%s val=%lu,data=%u WDT_STA=%d\n", __func__, val, *(uint32_t *)data, (bool)nxp_wdt_is_running());

	wdt_freq = (*(uint32_t *)data)/2;

	if (!nxp_wdt_is_running())
		goto done;

	if (val == BCLK_CHANGED) {
		nxp_wdt_stop(&nxp_wdd);

		ret = nxp_wdt_set_heartbeat(&nxp_wdd, nxp_wdd.timeout);

		if (ret >= 0)
			nxp_wdt_start(&nxp_wdd);
		else
			goto err;
	}
	else {
		/* To ensure that over the change we don't cause the
		 * watchdog to trigger, we perform an keep-alive if
		 * the watchdog is running.
		 */

		nxp_wdt_keepalive(&nxp_wdd);
	}
done:
	return 0;

 err:
	dev_err(wdt_dev, "cannot set new value for timeout %d. and restart(default timeout value) \n",
				nxp_wdd.timeout);
	nxp_wdt_set_heartbeat(&nxp_wdd, CONFIG_NXP_WATCHDOG_DEFAULT_TIME);
	nxp_wdt_start(&nxp_wdd);

	return ret;
}

static struct notifier_block nxp_wdt_bclk_dfs_transition_nb = {
	.notifier_call	= nxp_wdt_bclk_dfs_transition,
};

static inline void nxp_wdt_bclk_dfs_register(void)
{
	bclk_dfs_register_notify(&nxp_wdt_bclk_dfs_transition_nb);
}

static inline void nxp_wdt_bclk_dfs_deregister(void)
{
	bclk_dfs_unregister_notify(&nxp_wdt_bclk_dfs_transition_nb);
}
#else
static inline int nxp_wdt_bclk_dfs_register(void)
{
	return 0;
}

static inline void nxp_wdt_bclk_dfs_deregister(void)
{
}
#endif

static int __devinit nxp_wdt_probe(struct platform_device *pdev)
{
	struct device *dev;
	unsigned int wtcon;
	int started = 0;
	int ret;
	int size;
#ifdef CONFIG_WDT_SYSFS
    struct kobject *kobj = NULL;
#endif

	DBG("%s: probe=%p\n", __func__, pdev);

	dev = &pdev->dev;
	wdt_dev = &pdev->dev;

	wdt_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (wdt_mem == NULL) {
		dev_err(dev, "no memory resource specified\n");
		return -ENOENT;
	}

	wdt_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (wdt_irq == NULL) {
		dev_err(dev, "no irq resource specified\n");
		ret = -ENOENT;
		goto err;
	}

	/* get the memory region for the watchdog timer */

	size = resource_size(wdt_mem);
	if (!request_mem_region(wdt_mem->start, size, pdev->name)) {
		dev_err(dev, "failed to get memory region\n");
		ret = -EBUSY;
		goto err;
	}

	wdt_clock = clk_get(NULL, "pclk");
	if (IS_ERR(wdt_clock)) {
		dev_err(dev, "failed to find watchdog clock source\n");
		ret = PTR_ERR(wdt_clock);
		goto err_map;
	}

	clk_enable(wdt_clock);
	wdt_freq = clk_get_rate(wdt_clock);

	nxp_wdt_bclk_dfs_register();

	nxp_soc_peri_reset_set(RESET_ID_WDT);
	nxp_soc_peri_reset_set(RESET_ID_WDT_POR);

	/* if we're not enabling the watchdog, then ensure it is
	 * disabled if it has been left running from the bootloader
	 * or other source */
	nxp_wdt_stop(&nxp_wdd);

	/* see if we can actually set the requested timer margin, and if
	 * not, try the default value */

	if (nxp_wdt_set_heartbeat(&nxp_wdd, tmr_margin)) {
		started = nxp_wdt_set_heartbeat(&nxp_wdd,
#ifdef CONFIG_WDT_TASK
					CONFIG_DEFAULT_WDT_TASK_TIMEOUT);
#else
					CONFIG_NXP_WATCHDOG_DEFAULT_TIME);
#endif

		if (started == 0)
			dev_info(dev,
			   "tmr_margin value out of range, default %d used\n",
			       CONFIG_NXP_WATCHDOG_DEFAULT_TIME);
		else
			dev_info(dev, "default timer value is out of range, "
							"cannot start\n");
	}

	ret = request_irq(wdt_irq->start, nxp_wdt_irq, 0, pdev->name, pdev);
	if (ret != 0) {
		dev_err(dev, "failed to install irq (%d)\n", ret);
		goto err_clk;
	}

	watchdog_set_nowayout(&nxp_wdd, nowayout);

	ret = watchdog_register_device(&nxp_wdd);
	if (ret) {
		dev_err(dev, "cannot register watchdog (%d)\n", ret);
		goto err_irq;
	}

#ifdef CONFIG_WDT_SYSFS
    kobj = kobject_create_and_add("wdt", &platform_bus.kobj);
    if (! kobj) {
        printk(KERN_ERR "Fail, create kobject for display\n");
        return -ret;
    }
    ret = sysfs_create_group(kobj, &attr_group);
    if (ret) {
        printk(KERN_ERR "Fail, create sysfs group for display\n");
        kobject_del(kobj);
        return -ret;
    }
#endif

	/* print out a statement of readiness */

	wtcon = readl(NXP_WTCON);

	dev_info(dev, "watchdog %sactive, reset %sabled, irq %sabled\n",
		 (wtcon & NXP_WTCON_ENABLE) ?  "" : "in",
		 (wtcon & NXP_WTCON_RSTEN) ? "en" : "dis",
		 (wtcon & NXP_WTCON_INTEN) ? "en" : "dis");

#ifdef CONFIG_WDT_TASK
	wdt_wqueue = create_singlethread_workqueue("nxp_wdt_task_wqueue");
	INIT_DELAYED_WORK_DEFERRABLE(&wdt_task_work, nxp_wdt_task_work);
	nxp_wdt_start(&nxp_wdd);
    queue_delayed_work(wdt_wqueue, &wdt_task_work, msecs_to_jiffies(CONFIG_DEFAULT_WDT_TASK_TIMEOUT-1)*1000);
#endif

	return 0;

 err_irq:
	free_irq(wdt_irq->start, pdev);

 err_clk:
	clk_disable(wdt_clock);
	clk_put(wdt_clock);
	wdt_clock = NULL;

 err_map:
	release_mem_region(wdt_mem->start, size);

 err:
	wdt_irq = NULL;
	wdt_mem = NULL;
	return ret;
}

static int __devexit nxp_wdt_remove(struct platform_device *dev)
{
	watchdog_unregister_device(&nxp_wdd);

	free_irq(wdt_irq->start, dev);

	nxp_wdt_bclk_dfs_deregister();

	clk_disable(wdt_clock);
	clk_put(wdt_clock);
	wdt_clock = NULL;

	release_mem_region(wdt_mem->start, resource_size(wdt_mem));
	wdt_irq = NULL;
	wdt_mem = NULL;
	return 0;
}

static void nxp_wdt_shutdown(struct platform_device *dev)
{
	nxp_wdt_stop(&nxp_wdd);
}

#ifdef CONFIG_PM

static unsigned long wtcon_save;
static unsigned long wtdat_save;

static int nxp_wdt_suspend(struct platform_device *dev, pm_message_t state)
{
#ifdef CONFIG_WDT_TASK
	cancel_delayed_work(&wdt_task_work);
#endif

	/* Save watchdog state, and turn it off. */
	wtcon_save = readl(NXP_WTCON);

	/* Note that WTCNT doesn't need to be saved. */
	nxp_wdt_stop(&nxp_wdd);

	return 0;
}

static int nxp_wdt_resume(struct platform_device *dev)
{
	/* Restore watchdog state. */
	if (!nxp_soc_peri_reset_status(RESET_ID_WDT))
		nxp_soc_peri_reset_set(RESET_ID_WDT);
	if (!nxp_soc_peri_reset_status(RESET_ID_WDT_POR))
		nxp_soc_peri_reset_set(RESET_ID_WDT_POR);

	/* Note that WTCNT doesn't need to be saved. */
	nxp_wdt_stop(&nxp_wdd);

	wdt_freq = clk_get_rate(wdt_clock);
	nxp_wdt_set_heartbeat(&nxp_wdd, nxp_wdd.timeout);

	wtdat_save = readl(NXP_WTDAT);
	writel(wtdat_save, NXP_WTCNT); /* Reset count */

	wtcon_save &= 0xff;
	wtcon_save |= readl(NXP_WTCON)&NXP_WTCON_PRESCALE_MASK;
	writel(wtcon_save, NXP_WTCON);

	pr_info("watchdog %sabled\n",
		(wtcon_save & NXP_WTCON_ENABLE) ? "en" : "dis");

#ifdef CONFIG_WDT_TASK
    queue_delayed_work(wdt_wqueue, &wdt_task_work, msecs_to_jiffies(CONFIG_DEFAULT_WDT_TASK_TIMEOUT-1)*1000);
#endif

	return 0;
}

#else
#define nxp_wdt_suspend NULL
#define nxp_wdt_resume  NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_OF
static const struct of_device_id nxp_wdt_match[] = {
	{ .compatible = "SLsiAP,nxp-wdt" },
	{},
};
MODULE_DEVICE_TABLE(of, nxp_wdt_match);
#else
#define nxp_wdt_match NULL
#endif

static struct platform_driver nxp_wdt_driver = {
	.probe		= nxp_wdt_probe,
	.remove		= __devexit_p(nxp_wdt_remove),
	.shutdown	= nxp_wdt_shutdown,
	.suspend	= nxp_wdt_suspend,
	.resume		= nxp_wdt_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME_WDT,
		.of_match_table	= nxp_wdt_match,
	},
};


static int __init watchdog_init(void)
{
	pr_info("NXP Watchdog Timer, (c) 2014 SLsiAP\n");

	return platform_driver_register(&nxp_wdt_driver);
}

static void __exit watchdog_exit(void)
{
	platform_driver_unregister(&nxp_wdt_driver);
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("hyunseok jung <hsjung@nexell.co.kr>");
MODULE_DESCRIPTION("NXP Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:nxp-wdt");
