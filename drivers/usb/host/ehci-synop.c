/*
 * NEXELL NXP4330 USB HOST EHCI Controller
 *
 * Copyright (C) 2013 Nexell Co.Ltd
 * Author: BongKwan Kook <kook@nexell.co.kr>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/wakelock.h>

#include <mach/devices.h>
#include <mach/usb-phy.h>
#include <mach/soc.h>

#define TYPEDEF_BOOL
#ifndef FALSE
#define FALSE   false
#endif
#ifndef TRUE
#define TRUE    true
#endif

/*
 * enumeration at end of resume sequence for fast resume
 */
#define	EHCI_WORK_QUEUE_DELAY	(1000)	/* wait for end usb resume sequence */

#define EHCI_INSNREG00(base)			(base + 0x90)
#define EHCI_INSNREG00_ENA_INCR16		(0x1 << 25)
#define EHCI_INSNREG00_ENA_INCR8		(0x1 << 24)
#define EHCI_INSNREG00_ENA_INCR4		(0x1 << 23)
#define EHCI_INSNREG00_ENA_INCRX_ALIGN		(0x1 << 22)
#define EHCI_INSNREG00_ENABLE_DMA_BURST	\
	(EHCI_INSNREG00_ENA_INCR16 | EHCI_INSNREG00_ENA_INCR8 |	\
	 EHCI_INSNREG00_ENA_INCR4 | EHCI_INSNREG00_ENA_INCRX_ALIGN)

struct nxp_ehci_hcd {
	struct device *dev;
	struct usb_hcd *hcd;
	struct clk *clk;
	struct usb_phy *phy;
#ifdef CONFIG_USB_EHCI_SYNOPSYS_RESUME_WORK
	struct workqueue_struct *resume_wq;
	struct delayed_work	resume_work;
	struct wake_lock resume_lock;
	int delay_time;
	unsigned char backup_state[256];
#endif
};

static const struct hc_driver nxp_ehci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "SLsi Synopsys EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	.reset			= ehci_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	.get_frame_number	= ehci_get_frame,

	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.endpoint_reset		= ehci_endpoint_reset,

	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,

	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete	= ehci_clear_tt_buffer_complete,
};

#ifdef CONFIG_USB_EHCI_SYNOPSYS_RESUME_WORK
#include "../core/usb.h"

static void nxp_ehci_resume_work(struct work_struct *work);

static void nxp_ehci_resume_previous(struct usb_device *hdev, unsigned char *state, int *step)
{
	int port = hdev->maxchild;

	pr_debug("%s: %s, ports=%d, step=%d, state=%d\n",
		__func__, dev_name(&hdev->dev), hdev->maxchild, (int)*step, hdev->state);

	state[*step] = (unsigned char)(hdev->state);
	*step += 1;

	hdev->state = USB_STATE_NOTATTACHED;
	for (port = hdev->maxchild; port > 0; port--) {
		struct usb_device *udev = hdev->children[port-1];
		if (udev)
			nxp_ehci_resume_previous(udev, state, step);
	}
}

static void nxp_ehci_resume_last(struct usb_device *hdev, unsigned char *state, int *step)
{
	int port = hdev->maxchild;

	hdev->state = state[*step];
	*step += 1;

	pr_debug("%s: %s, ports=%d, step=%d, state=%d\n",
		__func__, dev_name(&hdev->dev), hdev->maxchild, (int)*step, hdev->state);

	usb_resume(&hdev->dev, PMSG_RESUME);

	for (port = hdev->maxchild; port > 0; port--) {
		struct usb_device *udev = hdev->children[port-1];
		if (udev) {
			udev->state = USB_STATE_CONFIGURED;
			udev->reset_resume = 1;
			nxp_ehci_resume_last(udev, state, step);
		}
	}
}
#endif

static int __devinit nxp_ehci_probe(struct platform_device *pdev)
{
	struct nxp_ehci_platdata *pdata;
	struct nxp_ehci_hcd *nxp_ehci;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	struct resource *res;
#if defined( CONFIG_USB_HSIC_SYNOPSYS )
	__u32 __iomem	*hsic_status_reg;
#endif
	int irq;
	int err;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "No platform data defined\n");
		return -EINVAL;
	}

	nxp_ehci = kzalloc(sizeof(struct nxp_ehci_hcd), GFP_KERNEL);
	if (!nxp_ehci)
		return -ENOMEM;

	nxp_ehci->dev = &pdev->dev;

	hcd = usb_create_hcd(&nxp_ehci_hc_driver, &pdev->dev,
					dev_name(&pdev->dev));
	if (!hcd) {
		dev_err(&pdev->dev, "Unable to create HCD\n");
		err = -ENOMEM;
		goto fail_hcd;
	}

	nxp_ehci->hcd = hcd;
	nxp_ehci->clk = NULL;
	nxp_ehci->phy = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get I/O memory\n");
		err = -ENXIO;
		goto fail_io;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);
	hcd->regs = ioremap(res->start, resource_size(res));
	if (!hcd->regs) {
		dev_err(&pdev->dev, "Failed to remap I/O memory\n");
		err = -ENOMEM;
		goto fail_io;
	}

#if defined( CONFIG_USB_HSIC_SYNOPSYS )
	hsic_status_reg = ioremap(0xC00300B0, SZ_4);
	if (!hsic_status_reg) {
		dev_err(&pdev->dev, "Failed to remap I/O memory\n");
		err = -ENOMEM;
		goto fail_io;
	}
#endif

	irq = platform_get_irq(pdev, 0);
	if (!irq) {
		dev_err(&pdev->dev, "Failed to get IRQ\n");
		err = -ENODEV;
		goto fail;
	}

#if defined( CONFIG_USB_HSIC_SYNOPSYS)
	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_HSIC);
#else
	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_EHCI);
#endif

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs +
		HC_LENGTH(ehci, readl(&ehci->caps->hc_capbase));

#if defined( CONFIG_USB_HSIC_SYNOPSYS )
	ehci->hsic_status_reg = hsic_status_reg;
#endif

	/* DMA burst Enable */
//	writel(EHCI_INSNREG00_ENABLE_DMA_BURST, EHCI_INSNREG00(hcd->regs));

	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);

	ehci_reset(ehci);

	err = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (err) {
		dev_err(&pdev->dev, "Failed to add USB HCD\n");
		goto fail;
	}

	platform_set_drvdata(pdev, nxp_ehci);

	if (nxp_ehci->phy) {
		err = otg_set_host(nxp_ehci->phy->otg, &hcd->self);
		if (err)
			goto fail;
	}

	/*
	 * add by jhkim to save resume time
	 */
#ifdef CONFIG_USB_EHCI_SYNOPSYS_RESUME_WORK
	nxp_ehci->delay_time = pdata->resume_delay_time;
	if (100 > nxp_ehci->delay_time)
		nxp_ehci->delay_time = EHCI_WORK_QUEUE_DELAY;

	nxp_ehci->resume_wq = alloc_workqueue("nxp-ehci", WQ_MEM_RECLAIM | WQ_NON_REENTRANT, 1);
	if (!nxp_ehci->resume_wq)
		goto fail;

	INIT_DELAYED_WORK(&nxp_ehci->resume_work, nxp_ehci_resume_work);
 	wake_lock_init(&nxp_ehci->resume_lock, WAKE_LOCK_SUSPEND, "nxp-ehci");
#endif
	return 0;

fail:
	iounmap(hcd->regs);
fail_io:
	usb_put_hcd(hcd);
fail_hcd:
	if (nxp_ehci->phy)
		usb_put_transceiver(nxp_ehci->phy);
	kfree(nxp_ehci);
	return err;
}

static int __devexit nxp_ehci_remove(struct platform_device *pdev)
{
	struct nxp_ehci_platdata *pdata = pdev->dev.platform_data;
	struct nxp_ehci_hcd *nxp_ehci = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = nxp_ehci->hcd;

	usb_remove_hcd(hcd);

#if defined( CONFIG_USB_HSIC_SYNOPSYS )
	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, NXP_USB_PHY_HSIC);

	if (pdata && pdata->hsic_phy_pwr_on)
		pdata->hsic_phy_pwr_on(pdev, FALSE);
#else
	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, NXP_USB_PHY_EHCI);
#endif

#ifdef CONFIG_USB_EHCI_SYNOPSYS_RESUME_WORK
	destroy_workqueue(nxp_ehci->resume_wq);
#endif
	iounmap(hcd->regs);

	usb_put_hcd(hcd);
	kfree(nxp_ehci);

	return 0;
}

static void nxp_ehci_shutdown(struct platform_device *pdev)
{
	struct nxp_ehci_hcd *nxp_ehci = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = nxp_ehci->hcd;

	if (!hcd->rh_registered)
		return;

	if (hcd->driver->shutdown)
		hcd->driver->shutdown(hcd);
}

#ifdef CONFIG_PM
static int nxp_ehci_suspend(struct device *dev)
{
	struct nxp_ehci_hcd *nxp_ehci = dev_get_drvdata(dev);
	struct usb_hcd *hcd = nxp_ehci->hcd;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	struct platform_device *pdev = to_platform_device(dev);
	struct nxp_ehci_platdata *pdata = pdev->dev.platform_data;
	unsigned long flags;
	int rc = 0;

	/*
	 * If we have an otg driver, the otg wakelock blocks suspend while
	 * we are in host mode. When you unplug the host cable, the otg driver
	 * stops the ehci controller and shuts down the phy immediately, so the
	 * ehci has already been stopped when this function is called.
	 */
	if (nxp_ehci->phy)
		return 0;

	if (time_before(jiffies, ehci->next_statechange))
		msleep(20);

	/*
	 * Root hub was already suspended. Disable irq emission and
	 * mark HW unaccessible.  The PM and USB cores make sure that
	 * the root hub is either suspended or stopped.
	 */
	ehci_prepare_ports_for_controller_suspend(ehci, device_may_wakeup(dev));
	spin_lock_irqsave(&ehci->lock, flags);
	ehci_writel(ehci, 0, &ehci->regs->intr_enable);
	(void)ehci_readl(ehci, &ehci->regs->intr_enable);

	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	spin_unlock_irqrestore(&ehci->lock, flags);

#if defined( CONFIG_USB_HSIC_SYNOPSYS )
	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, NXP_USB_PHY_HSIC);

	if (pdata && pdata->hsic_phy_pwr_on)
		pdata->hsic_phy_pwr_on(pdev, FALSE);
#else
	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, NXP_USB_PHY_EHCI);
#endif

	return rc;
}

#ifdef CONFIG_USB_EHCI_SYNOPSYS_RESUME_WORK
static void nxp_ehci_resume_work(struct work_struct *work)
{
	struct nxp_ehci_hcd *nxp_ehci = container_of(work, struct nxp_ehci_hcd, resume_work.work);
	struct usb_hcd *hcd = nxp_ehci->hcd;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	struct platform_device *pdev = to_platform_device(nxp_ehci->dev);
	struct nxp_ehci_platdata *pdata = pdev->dev.platform_data;
	struct usb_device *udev = hcd->self.root_hub;

	if (nxp_ehci->phy) {
		wake_unlock(&nxp_ehci->resume_lock);
		return;
	}

#if defined( CONFIG_USB_HSIC_SYNOPSYS )
	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_HSIC);

	if (pdata && pdata->hsic_phy_pwr_on)
		pdata->hsic_phy_pwr_on(pdev, TRUE);
#else

	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_EHCI);
#endif

	/* DMA burst Enable */
//	writel(EHCI_INSNREG00_ENABLE_DMA_BURST, EHCI_INSNREG00(hcd->regs));

	if (time_before(jiffies, ehci->next_statechange))
		msleep(1);		// msleep(100);

	/* Mark hardware accessible again as we are out of D3 state by now */
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	if (ehci_readl(ehci, &ehci->regs->configured_flag) == FLAG_CF) {
		int	mask = INTR_MASK;

		ehci_prepare_ports_for_controller_resume(ehci);
		if (!hcd->self.root_hub->do_remote_wakeup)
			mask &= ~STS_PCD;
		ehci_writel(ehci, mask, &ehci->regs->intr_enable);
		ehci_readl(ehci, &ehci->regs->intr_enable);
		wake_unlock(&nxp_ehci->resume_lock);
		return;
	}

	usb_root_hub_lost_power(hcd->self.root_hub);

	(void) ehci_halt(ehci);
	(void) ehci_reset(ehci);

	/* emptying the schedule aborts any urbs */
	spin_lock_irq(&ehci->lock);
	if (ehci->reclaim)
		end_unlink_async(ehci);
	ehci_work(ehci);
	spin_unlock_irq(&ehci->lock);

	ehci_writel(ehci, ehci->command, &ehci->regs->command);
	ehci_writel(ehci, FLAG_CF, &ehci->regs->configured_flag);
	ehci_readl(ehci, &ehci->regs->command);	/* unblock posted writes */

	/* here we "know" root ports should always stay powered */
	ehci_port_power(ehci, 1);

	ehci->rh_state = EHCI_RH_SUSPENDED;

	/*
	 * usb resume
	 */
	if (udev) {
		int step = 0;
		usb_lock_device(udev);
		nxp_ehci_resume_last(udev, nxp_ehci->backup_state, &step);
		usb_unlock_device(udev);
	}
	wake_unlock(&nxp_ehci->resume_lock);
}
#endif

static int nxp_ehci_resume(struct device *dev)
{
#ifndef CONFIG_USB_EHCI_SYNOPSYS_RESUME_WORK
	struct nxp_ehci_hcd *nxp_ehci = dev_get_drvdata(dev);
	struct usb_hcd *hcd = nxp_ehci->hcd;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	struct platform_device *pdev = to_platform_device(dev);
	struct nxp_ehci_platdata *pdata = pdev->dev.platform_data;

	if (nxp_ehci->phy)
		return 0;

#if defined( CONFIG_USB_HSIC_SYNOPSYS)
	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_HSIC);

	if (pdata && pdata->hsic_phy_pwr_on)
		pdata->hsic_phy_pwr_on(pdev, TRUE);
#else

	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_EHCI);
#endif

	/* DMA burst Enable */
//	writel(EHCI_INSNREG00_ENABLE_DMA_BURST, EHCI_INSNREG00(hcd->regs));

	if (time_before(jiffies, ehci->next_statechange))
		msleep(1);		// msleep(100);

	/* Mark hardware accessible again as we are out of D3 state by now */
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	if (ehci_readl(ehci, &ehci->regs->configured_flag) == FLAG_CF) {
		int	mask = INTR_MASK;

		ehci_prepare_ports_for_controller_resume(ehci);
		if (!hcd->self.root_hub->do_remote_wakeup)
			mask &= ~STS_PCD;
		ehci_writel(ehci, mask, &ehci->regs->intr_enable);
		ehci_readl(ehci, &ehci->regs->intr_enable);
		return 0;
	}

	usb_root_hub_lost_power(hcd->self.root_hub);

	(void) ehci_halt(ehci);
	(void) ehci_reset(ehci);

	/* emptying the schedule aborts any urbs */
	spin_lock_irq(&ehci->lock);
	if (ehci->reclaim)
		end_unlink_async(ehci);
	ehci_work(ehci);
	spin_unlock_irq(&ehci->lock);

	ehci_writel(ehci, ehci->command, &ehci->regs->command);
	ehci_writel(ehci, FLAG_CF, &ehci->regs->configured_flag);
	ehci_readl(ehci, &ehci->regs->command);	/* unblock posted writes */

	/* here we "know" root ports should always stay powered */
	ehci_port_power(ehci, 1);

	ehci->rh_state = EHCI_RH_SUSPENDED;

#else
	struct nxp_ehci_hcd *nxp_ehci = dev_get_drvdata(dev);
	struct usb_hcd *hcd = nxp_ehci->hcd;
	struct usb_device *udev = hcd->self.root_hub;
	int step = 0;

	nxp_ehci_resume_previous(udev, nxp_ehci->backup_state, &step);
	wake_lock(&nxp_ehci->resume_lock);
	queue_delayed_work(nxp_ehci->resume_wq, &nxp_ehci->resume_work,
				msecs_to_jiffies(nxp_ehci->delay_time));
	#if (0)
	schedule_delayed_work(&nxp_ehci->resume_work,
				msecs_to_jiffies(nxp_ehci->delay_time));
	#endif
#endif
	return 0;
}
#else
#define nxp_ehci_suspend	NULL
#define nxp_ehci_resume		NULL
#endif

static const struct dev_pm_ops nxp_ehci_pm_ops = {
	.suspend	= nxp_ehci_suspend,
	.resume		= nxp_ehci_resume,
};

static struct platform_driver nxp_ehci_driver = {
	.probe		= nxp_ehci_probe,
	.remove		= __devexit_p(nxp_ehci_remove),
	.shutdown	= nxp_ehci_shutdown,
	.driver = {
		.name	= "nxp-ehci",
		.owner	= THIS_MODULE,
		.pm	= &nxp_ehci_pm_ops,
	}
};

MODULE_ALIAS("platform:nxp-ehci");
MODULE_AUTHOR("Nexell Co., Ltd.");
MODULE_AUTHOR("BongKwan Kook <kook@nexell.co.kr>");
