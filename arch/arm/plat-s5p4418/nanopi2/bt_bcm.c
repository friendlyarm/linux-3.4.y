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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <mach/platform.h>
#include <mach/soc.h>
#include "bt_bcm.h"


#define	BT_GPIO_REG_ON			(PAD_GPIO_B + 8)
#define	BT_GPIO_WAKE_DEVICE 	(-1)
#define	BT_GPIO_WAKE_HOST 		(-1)

#define	BT_UART_PORT_LINE		(1)	/* ttyAMA1 */

#define	UART1_RTS_IO_NUM		(PAD_GPIO_C + 6)
#define	UART1_RTS_ALT_RTS		2
#define	UART1_RTS_ALT_IO		1

//#define	SUPPOR_BT_BCM_LPM

//extern int micom_bt_cmd(int sub, int on);

/*
 * Bluetooth BCM platform data
 */
static struct bt_ctl_gpio bt_gpios[] = {
	{
		.name 		= "bt power on/off",
		.type		= BT_TYPE_POWER,
		.gpio 		= BT_GPIO_REG_ON,
		.direction	= 1,
		.init_val	= 0,
	},
#if (BT_GPIO_WAKE_DEVICE > 0)
	{
		.name 		= "bt device wake",
		.type		= BT_TYPE_WAKE_DEVICE,
		.gpio 		= BT_GPIO_WAKE_DEVICE,
		.direction	= 1,
		.init_val	= 1,
	},
#endif
#if (BT_GPIO_WAKE_HOST > 0)
	{
		.name 		= "bt host wakeup",
		.type		= BT_TYPE_WAKE_HOST,
		.gpio 		= BT_GPIO_WAKE_HOST,
		.direction	= 0,
	},
#endif
};

static struct plat_bt_bcm_data bt_plat_data = {
	.gpios 	 = bt_gpios,
	.gpio_nr = ARRAY_SIZE(bt_gpios),
};

static const char *ioname[] = { "GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "ALIVE" };
#define	STR_GR(n)	(ioname[n/32])
#define	BIT_NR(n)	(n&0x1F)

#define	BTI_TYPE(_bti_, _type_, _bcm_)	{	\
		int i; _bti_ = _bcm_->gpios;			\
		for (i = 0; _bcm_->gpio_nr > i; i++, _bti_++) {	\
			if (_type_ == _bti_->type) break;			\
		}												\
		if (_bcm_->gpio_nr == i) _bti_ = NULL;			\
		}

/*
 * Check uart data transfer
 */
static inline int bt_bcm_request_io(struct bt_ctl_gpio *bti)
{
	int ret = 0;

	if(bti->gpio > -1)
		ret = gpio_request(bti->gpio, bti->name);

	return ret;
}

static inline void bt_bcm_free_io(struct bt_ctl_gpio *bti)
{
	if(bti->gpio > -1)
		gpio_free(bti->gpio);
}

static inline int bt_bcm_set_direction(struct bt_ctl_gpio *bti, int dir, int val)
{
	int ret = 0;

	if(bti->gpio > -1)
	{
		if (dir)
			ret = gpio_direction_output(bti->gpio, val);
		else
			ret = gpio_direction_input(bti->gpio);
	}
	return ret;
}

static  void bt_bcm_set_io_val(struct bt_ctl_gpio *bti, int on)
{
	int sub;

	pr_debug("[pchen***]bt_bcm: bt_bcm_set_io_val (%s.%d) [%s]\n",
		STR_GR(bti->gpio), BIT_NR(bti->gpio), on?"on":"off");

	if(bti->gpio > -1)
	{
//		gpio_set_value(bti->gpio, (on ? 1: 0));
		gpio_direction_output(bti->gpio, (on ? 1: 0));
/*		if(on){
//			gpio_direction_output(bti->gpio, 0);
//			msleep(100);
			gpio_direction_output(bti->gpio, 1);
//			msleep(100);
		}else{
			gpio_direction_output(bti->gpio, 0);
//			msleep(10);
		}*/
	}else{
		switch (bti->type)
		{
			case BT_TYPE_POWER:	
				sub = BT_TYPE_POWER;
				break;
			case BT_TYPE_WAKE_DEVICE:
				sub = BT_TYPE_WAKE_DEVICE;
				break;
			default:
				pr_err("bt_bcm: unkonwn bt control type ...\n");
				return;
		}		
//		micom_bt_cmd(sub, on);
	}
}

static inline int bt_bcm_get_io_val(struct bt_ctl_gpio *bti)
{
	return gpio_get_value(bti->gpio);
}

static void bt_bcm_rts_ctrl(int flag)
{
	if (flag) {
		/* BT RTS Set to HIGH */
		nxp_soc_gpio_set_io_dir (UART1_RTS_IO_NUM, 1);
		nxp_soc_gpio_set_io_func(UART1_RTS_IO_NUM, UART1_RTS_ALT_IO);
		nxp_soc_gpio_set_out_value(UART1_RTS_IO_NUM, 1);
	} else {
		/* restore BT RTS state */
		nxp_soc_gpio_set_io_func(UART1_RTS_IO_NUM, UART1_RTS_ALT_RTS);
		nxp_soc_gpio_set_io_dir (UART1_RTS_IO_NUM, 1);
	}
}

#if defined (SUPPOR_BT_BCM_LPM)
static struct bt_bcm_lpm *__bt_lpm = NULL;
static void bt_bcm_lpm_wake_dev(struct bt_bcm_info *bcm, int wake)
{
	struct bt_bcm_lpm *lpm = &bcm->lpm;
	struct bt_ctl_gpio *btd;

	if (wake == lpm->wake)
		return;

	BTI_TYPE(btd, BT_TYPE_WAKE_DEVICE, bcm);
	if (NULL == btd)
		return;

	lpm->wake = wake;

	pr_debug("bt_bcm: lpm wake dev (%s.%d) [%s]\n",
		STR_GR(btd->gpio), BIT_NR(btd->gpio), wake?"on":"off");

	if (wake) {
		//wake_lock(&lpm->wake_lock);
		bt_bcm_set_io_val(btd, 1);
	} else {
		bt_bcm_set_io_val(btd, 0);
		//wake_unlock(&lpm->wake_lock);
	}
}

static enum hrtimer_restart bt_bcm_lpm_timer_func(struct hrtimer *timer)
{
	struct bt_bcm_info *bcm = container_of(timer, struct bt_bcm_info, lpm.lpm_timer);

	pr_debug("bt_bcm: lpm timer dev [off]\n");
	bt_bcm_lpm_wake_dev(bcm, 0);
	return HRTIMER_NORESTART;
}

#define	NX_UART_CH_INIT(ch) do { \
	struct clk *clk;									\
	char name[16];										\
	sprintf(name, "nxp-uart.%d", ch);					\
	clk = clk_get(NULL, name);							\
	if (!nxp_soc_rsc_status(RESET_ID_UART## ch)) {		\
		NX_TIEOFF_Set(TIEOFF_UART## ch ##_USERSMC , 0);	\
		NX_TIEOFF_Set(TIEOFF_UART## ch ##_SMCTXENB, 0);	\
		NX_TIEOFF_Set(TIEOFF_UART## ch ##_SMCRXENB, 0);	\
		nxp_soc_rsc_reset(RESET_ID_UART## ch);			\
	}													\
	clk_set_rate(clk, CFG_UART_CLKGEN_CLOCK_HZ);		\
	clk_enable(clk);								\
	} while (0)

void pl011_uart1_prepare(void)
{
	NX_UART_CH_INIT(1);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 15, NX_GPIO_PADFUNC_1);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 19, NX_GPIO_PADFUNC_1);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 15, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 19, CTRUE);

	nxp_soc_gpio_set_io_func(UART1_RTS_IO_NUM, UART1_RTS_ALT_RTS);
	nxp_soc_gpio_set_io_dir (UART1_RTS_IO_NUM, 1);
}

void pl011_uart1_wake_peer(void *uport)
{
	struct uart_port *port = uport;
	struct bt_bcm_lpm *lpm = __bt_lpm;
	struct bt_bcm_info *bcm = container_of(lpm, struct bt_bcm_info, lpm);
	
	pr_debug("bt_bcm: lpm uart.%d trans [%s]\n", port->line,
		bcm->running?"run":"stopped");
	
	if (BT_UART_PORT_LINE != port->line) {
		pr_debug("bt_bcm: Error UART.%d is not BT UART.%d\n", port->line, BT_UART_PORT_LINE);
		return;
	}
	lpm->uport = port;

	bt_bcm_lpm_wake_dev(bcm, 1);

	/* restart timer */
	if (lpm->lpm_timer.function) {
		hrtimer_try_to_cancel(&lpm->lpm_timer);
		hrtimer_start(&lpm->lpm_timer, lpm->lpm_delay, HRTIMER_MODE_REL);
	}
}

static void bt_bcm_lpm_wake_lock(struct bt_bcm_lpm *lpm, int host_wake)
{
	pr_debug("bt_bcm: lpm host wake [%s]\n", host_wake?"Lock":"Unlock");
	if (host_wake == lpm->host_wake)
		return;

	lpm->host_wake = host_wake;

	if (host_wake) {
		wake_lock(&lpm->host_wake_lock);
	} else  {
		/* Take a timed wakelock, so that upper layers can take it.
		 * The chipset deasserts the hostwake lock, when there is no
		 * more data to send.
		 */
		wake_lock_timeout(&lpm->host_wake_lock, HZ/2);
	}
}

static irqreturn_t bt_bcm_lpm_wake_intr(int irq, void *dev)
{
	struct bt_bcm_info *bcm = dev;
	struct bt_bcm_lpm *lpm = &bcm->lpm;
	struct bt_ctl_gpio *bth = bcm->gpios;
	int host_wake;

	BTI_TYPE(bth, BT_TYPE_WAKE_HOST, bcm);
	if (NULL == bth)
		return IRQ_HANDLED;

	host_wake = bt_bcm_get_io_val(bth);
	pr_debug("bt_bcm: lpm host irq [%s]\n", host_wake?"up":"down");

	if (NULL == lpm->uport || false == bcm->running) {
		lpm->wake = host_wake;
		return IRQ_HANDLED;
	}

	bt_bcm_lpm_wake_lock(lpm, host_wake);
	return IRQ_HANDLED;
}

static int bt_bcm_lpm_host_wake(struct bt_bcm_info *bcm)
{
	struct bt_bcm_lpm *lpm = &bcm->lpm;
	struct bt_ctl_gpio *bth;
	int ret;

	BTI_TYPE(bth, BT_TYPE_WAKE_HOST, bcm);
	if (NULL == bth)
		return -EINVAL;

	ret = request_irq(gpio_to_irq(bth->gpio), bt_bcm_lpm_wake_intr,
				IRQF_SHARED | IRQ_TYPE_EDGE_BOTH, "bt host wake", bcm);
	if (0 > ret) {
		pr_err("bt_bcm: failed request irq.%d (%s.%d)...\n",
			gpio_to_irq(bth->gpio), STR_GR(bth->gpio), BIT_NR(bth->gpio));
		return ret;
	}

	wake_lock_init(&lpm->host_wake_lock, WAKE_LOCK_SUSPEND, "BTHostWakeLowPower");
	return 0;
}

static int bt_bcm_lpm_init(struct bt_bcm_info *bcm)
{
	struct bt_bcm_lpm *lpm = &bcm->lpm;
	struct bt_ctl_gpio *bth;
 	int ret = 0;

	hrtimer_init(&lpm->lpm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lpm->lpm_delay = ktime_set(3, 0);
	lpm->lpm_timer.function = bt_bcm_lpm_timer_func;
	lpm->host_wake = 0;

	BTI_TYPE(bth, BT_TYPE_WAKE_HOST, bcm);
	if (bth)
		bt_bcm_lpm_host_wake(bcm);

	//wake_lock_init(&lpm->wake_lock, WAKE_LOCK_SUSPEND, "BTWakeLowPower");
	__bt_lpm = lpm;

	pr_info("bt_bcm: lpm register ....\n");
	return ret;
}

static void bt_bcm_lpm_exit(struct bt_bcm_info *bcm)
{
	struct bt_bcm_lpm *lpm = &bcm->lpm;
	struct bt_ctl_gpio *bth;

	BTI_TYPE(bth, BT_TYPE_WAKE_HOST, bcm);
	if (bth) {
		free_irq(gpio_to_irq(bth->gpio), bcm);
		wake_lock_destroy(&lpm->host_wake_lock);
	}

	bt_bcm_lpm_wake_dev(bcm, 0);
	hrtimer_try_to_cancel(&lpm->lpm_timer);
	//wake_lock_destroy(&lpm->wake_lock);
}

#endif /*  SUPPOR_BT_BCM_LPM */

/*
 * Bluetooth BCM Rfkill
 */
static int bt_bcm_rfkill_set_block(void *data, bool blocked)
{
	struct bt_bcm_info *bcm = data;
	struct bt_ctl_gpio *btp;

	if (NULL == bcm) {
		pr_err("bt_bcm: Failed %s, no data...\n", __func__);
		return -EINVAL;
	}

	BTI_TYPE(btp, BT_TYPE_POWER, bcm);
	if (NULL == btp)
		return -EINVAL;

	pr_info("bt_bcm: rfkill set_block %s %s.%d\n",
		blocked?"Off":"On ", STR_GR(btp->gpio), BIT_NR(btp->gpio));
		
	msleep(10);

	if (!blocked) {
//		nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D + 20, 0);
//		nxp_soc_gpio_set_io_pull_sel(PAD_GPIO_D + 20, 0);
		bt_bcm_set_io_val(btp, 1);	/* on */
//		msleep(100);
//		nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D + 20, 1);
//		nxp_soc_gpio_set_io_pull_sel(PAD_GPIO_D + 20, 0);
		bcm->running = true;
	} else {
//		nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D + 20, 1);
//		nxp_soc_gpio_set_io_pull_sel(PAD_GPIO_D + 20, 0);
		bt_bcm_set_io_val(btp, 0);	/* off */
		bcm->running = false;
	}
	
	msleep(50);
	return 0;
}


static int bt_bcm_rfkill_suspend(struct platform_device *pdev, pm_message_t state)
{
#if defined (SUPPOR_BT_BCM_LPM)
	struct bt_bcm_lpm *lpm = __bt_lpm;   

	nxp_soc_gpio_set_io_func(UART1_RTS_IO_NUM, UART1_RTS_ALT_IO);
	nxp_soc_gpio_set_io_dir (UART1_RTS_IO_NUM, 1);
	nxp_soc_gpio_set_out_value(UART1_RTS_IO_NUM, 1);
    lpm->wake = 0;
#endif
	
	return 0;
}

static const struct rfkill_ops bt_bcm_rfkill_ops = {
	.set_block 	= bt_bcm_rfkill_set_block,
};

static const char on_string[] = "1";
static const char off_string[] = "0";
static ssize_t show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct bt_bcm_info *bcm = dev_get_drvdata(dev);
	struct bt_ctl_gpio *btp;

	BTI_TYPE(btp, BT_TYPE_POWER, bcm);
	
	return sprintf(buf, "%s\n", bt_bcm_get_io_val(btp) ? on_string: off_string);
}

static ssize_t set_onoff(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct bt_bcm_info *bcm = dev_get_drvdata(dev);
	struct bt_ctl_gpio *btp;
	int len = count;
	char *cp;
	int rc = count;

	cp = memchr(buf, '\n', count);
	if (cp)
		len = cp - buf;

	BTI_TYPE(btp, BT_TYPE_POWER, bcm);
	if ((len == sizeof on_string - 1 &&
			strncmp(buf, on_string, len) == 0) ||
			(count == sizeof(on_string) &&
			strncmp(buf, on_string, count) == 0)){
//		bt_bcm_rfkill_set_block(bcm,true);
		pr_err("[BT---pchen] set on \n");
//		bt_bcm_set_io_val(btp, 1);	/* on */
		nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D + 20, 1);
		nxp_soc_gpio_set_io_pull_sel(PAD_GPIO_D + 20, 0);
		msleep(10);
	}
	else if ((len == sizeof off_string - 1 &&
			strncmp(buf, off_string, len) == 0) ||
			(count == sizeof(off_string) &&
			strncmp(buf, off_string, count) == 0)){
//		bt_bcm_rfkill_set_block(bcm,false);
		pr_err("[BT---pchen] set off \n");
	
//		bt_bcm_set_io_val(btp, 0);	/* off */
		nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D + 20, 0);
//		nxp_soc_gpio_set_io_pull_sel(PAD_GPIO_D + 20, 0);
		msleep(100);

	}else{
		pr_err("[BT---pchen] error------------- \n");
		rc = -EINVAL;
	}

	return rc;
}

static DEVICE_ATTR(onoff,  S_IRUGO | S_IWUGO, show_onoff, set_onoff);
static struct attribute *gpio_attrs[] = {
	&dev_attr_onoff.attr,
	NULL,
};

static struct attribute_group gpio_attr_group = {
	.name	= "gpio",
	.attrs	= gpio_attrs,
};

static int bt_bcm_probe(struct platform_device *pdev)
{
	struct plat_bt_bcm_data *pdata = pdev->dev.platform_data;
	struct bt_bcm_info *bcm = NULL;
	struct bt_ctl_gpio *bti;
	struct rfkill *rfkill = NULL;
	int i = 0, ret = 0;

	if (!pdata || 0 == pdata->gpio_nr)
		return -EINVAL;

	bcm = kzalloc(sizeof(*bcm), GFP_KERNEL);
	if (!bcm)
		return -ENOMEM;

	bcm->gpios = pdata->gpios;
	bcm->gpio_nr = pdata->gpio_nr;

	for (i = 0, bti = bcm->gpios; bcm->gpio_nr > i; i++, bti++) {
		ret = bt_bcm_request_io(bti);
		if (unlikely(ret)) {
			pr_err("bt_bcm: Cannot %s get %s.%d ...\n",
				bti->name, STR_GR(bti->gpio), BIT_NR(bti->gpio));
			goto err_gpio;
		}

		bt_bcm_set_direction(bti, bti->direction, bti->init_val);
		pr_debug("bt_bcm: %s %s.%d %s ...\n", bti->name,
			STR_GR(bti->gpio), BIT_NR(bti->gpio), bti->direction?"output":"input");
	}

	rfkill = rfkill_alloc("BCM Bluetooth", &pdev->dev,
				RFKILL_TYPE_BLUETOOTH, &bt_bcm_rfkill_ops, bcm);
	if (unlikely(!rfkill)) {
		ret =  -ENOMEM;
		goto err_gpio;
	}

	rfkill_init_sw_state(rfkill, false);
	ret = rfkill_register(rfkill);
	if (unlikely(ret)) {
		ret = -1;
		goto err_rfkill;
	}

	bcm->rfkill = rfkill;
	mutex_init(&bcm->lock);

	rfkill_set_sw_state(rfkill, true);
	platform_set_drvdata(pdev, bcm);

	ret = sysfs_create_group(&pdev->dev.kobj, &gpio_attr_group);
	if (unlikely(ret < 0)) {
		ret = -1;
		pr_err("[BT]sysfs_merge_group failed!\n");
		goto err_sysfs;
	}

	/* set lpm */
#if defined (SUPPOR_BT_BCM_LPM)
	bt_bcm_lpm_init(bcm);
#endif

	pr_info("bt_bcm: rfkill register ....\n");
	return 0;

err_sysfs:
	rfkill_unregister(rfkill);
err_rfkill:
	rfkill_destroy(rfkill);
err_gpio:
	for (i = 0, bti = bcm->gpios; bcm->gpio_nr > i; i++, bti++)
		bt_bcm_free_io(bti);

	kfree(bcm);
	return ret;
}

static int bt_bcm_remove(struct platform_device *pdev)
{
	struct bt_bcm_info *bcm = platform_get_drvdata(pdev);
	struct rfkill *rfkill = bcm->rfkill;
	int i;

	rfkill_unregister(rfkill);
	rfkill_destroy(rfkill);

	for (i = 0; bcm->gpio_nr > i; i++) {
		struct bt_ctl_gpio *bti = &bcm->gpios[i];
		bt_bcm_free_io(bti);
	}

#if defined (SUPPOR_BT_BCM_LPM)
	bt_bcm_lpm_exit(bcm);
#endif

	kfree(bcm);
	return 0;
}

static struct platform_device  bt_bcm_device = {
	.name			= "bt_bcm",
	.id				= 0,
	.dev			= {
		.platform_data	= &bt_plat_data,
	}
};

static struct platform_driver  bt_bcm_driver = {
	.probe 	= bt_bcm_probe,
	.remove = bt_bcm_remove,
  	.suspend	= bt_bcm_rfkill_suspend,
	.driver = {
		   .name = "bt_bcm",
		   .owner = THIS_MODULE,
	},
};

static int __init bt_bcm_init(void)
{
	int ret;

	platform_device_register(&bt_bcm_device);
    ret = platform_driver_register(&bt_bcm_driver);
	return ret;
}
static void __exit bt_bcm_exit(void)
{
	platform_driver_unregister(&bt_bcm_driver);
	platform_device_unregister(&bt_bcm_device);
}

module_init(bt_bcm_init);
module_exit(bt_bcm_exit);

MODULE_ALIAS("platform:bcm43241");
MODULE_DESCRIPTION("bcm");
MODULE_LICENSE("GPL");
