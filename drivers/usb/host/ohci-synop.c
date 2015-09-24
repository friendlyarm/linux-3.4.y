/*
 * NEXELL USB HOST OHCI Controller
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
#include <linux/usb/otg.h>

#include <mach/devices.h>
#include <mach/usb-phy.h>

struct nxp_ohci_hcd {
	struct device *dev;
	struct usb_hcd *hcd;
	struct clk *clk;
	struct usb_phy *phy;
};

static int ohci_nxp_init(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;

	ohci_dbg(ohci, "ohci_nxp_init, ohci:%p", ohci);

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

	return 0;
}

static int ohci_nxp_start(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;

	ohci_dbg(ohci, "ohci_nxp_start, ohci:%p", ohci);

	ret = ohci_run(ohci);
	if (ret < 0) {
		err("can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}

	return 0;
}

static const struct hc_driver nxp_ohci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "SLsi Synopsys OHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ohci_hcd),

	.irq			= ohci_irq,
	.flags			= HCD_MEMORY|HCD_USB11,

	.reset			= ohci_nxp_init,
	.start			= ohci_nxp_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,

	.get_frame_number	= ohci_get_frame,

	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,

	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend		= ohci_bus_suspend,
	.bus_resume		= ohci_bus_resume,
#endif
	.start_port_reset	= ohci_start_port_reset,
};

static int __devinit nxp_ohci_probe(struct platform_device *pdev)
{
	struct nxp_ohci_platdata *pdata;
	struct nxp_ohci_hcd *nxp_ohci;
	struct usb_hcd *hcd;
	struct ohci_hcd *ohci;
	struct resource *res;
	int irq;
	int err;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "No platform data defined\n");
		return -EINVAL;
	}

	nxp_ohci = kzalloc(sizeof(struct nxp_ohci_hcd), GFP_KERNEL);
	if (!nxp_ohci)
		return -ENOMEM;

	nxp_ohci->dev = &pdev->dev;

	hcd = usb_create_hcd(&nxp_ohci_hc_driver, &pdev->dev,
					dev_name(&pdev->dev));
	if (!hcd) {
		dev_err(&pdev->dev, "Unable to create HCD\n");
		err = -ENOMEM;
		goto fail_hcd;
	}

	nxp_ohci->hcd = hcd;
//	nxp_ohci->clk = clk_get(&pdev->dev, DEV_NAME_USB2HOST);
	nxp_ohci->phy = usb_get_transceiver();

#if 0
	if (IS_ERR(nxp_ohci->clk)) {
		dev_err(&pdev->dev, "Failed to get usbhost clock\n");
		err = PTR_ERR(nxp_ohci->clk);
		goto fail_clk;
	}

	err = clk_enable(nxp_ohci->clk);
	if (err)
		goto fail_clken;
#endif

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

	irq = platform_get_irq(pdev, 0);
	if (!irq) {
		dev_err(&pdev->dev, "Failed to get IRQ\n");
		err = -ENODEV;
		goto fail;
	}

#if 0
	if (nxp_ohci->phy)
		usb_phy_init(nxp_ohci->phy);
	else
#endif
	if (pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_OHCI);

	ohci = hcd_to_ohci(hcd);
	ohci_hcd_init(ohci);

	err = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (err) {
		dev_err(&pdev->dev, "Failed to add USB HCD\n");
		goto fail;
	}

	if (nxp_ohci->phy)
		otg_set_host(nxp_ohci->phy->otg, &hcd->self);

	platform_set_drvdata(pdev, nxp_ohci);

//	clk_disable(nxp_ohci->clk);

	return 0;

fail:
	iounmap(hcd->regs);
fail_io:
	if (nxp_ohci->clk)
		clk_disable(nxp_ohci->clk);
#if 0
fail_clken:
#endif
	if (nxp_ohci->clk)
		clk_put(nxp_ohci->clk);

	nxp_ohci->clk = NULL;
#if 0
fail_clk:
#endif
	usb_put_hcd(hcd);
fail_hcd:
	if (nxp_ohci->phy)
		usb_put_transceiver(nxp_ohci->phy);
	kfree(nxp_ohci);
	return err;
}

static int __devexit nxp_ohci_remove(struct platform_device *pdev)
{
	struct nxp_ohci_platdata *pdata = pdev->dev.platform_data;
	struct nxp_ohci_hcd *nxp_ohci = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = nxp_ohci->hcd;

	usb_remove_hcd(hcd);

	if (nxp_ohci->phy)
		usb_put_transceiver(nxp_ohci->phy);
	else if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, NXP_USB_PHY_OHCI);

	iounmap(hcd->regs);

	if (nxp_ohci->clk) {
		clk_disable(nxp_ohci->clk);
		clk_put(nxp_ohci->clk);
		nxp_ohci->clk = NULL;
	}

	usb_put_hcd(hcd);
	kfree(nxp_ohci);

	return 0;
}

static void nxp_ohci_shutdown(struct platform_device *pdev)
{
	struct nxp_ohci_hcd *nxp_ohci = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = nxp_ohci->hcd;

	if (!hcd->rh_registered)
		return;

	if (hcd->driver->shutdown)
		hcd->driver->shutdown(hcd);
}

#ifdef CONFIG_PM
static int nxp_ohci_suspend(struct device *dev)
{
	struct nxp_ohci_hcd *nxp_ohci = dev_get_drvdata(dev);
	struct usb_hcd *hcd = nxp_ohci->hcd;
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	struct platform_device *pdev = to_platform_device(dev);
	struct nxp_ohci_platdata *pdata = pdev->dev.platform_data;
	unsigned long flags;
	int rc = 0;

	if (nxp_ohci->phy)
		return 0;

	/*
	 * Root hub was already suspended. Disable irq emission and
	 * mark HW unaccessible, bail out if RH has been resumed. Use
	 * the spinlock to properly synchronize with possible pending
	 * RH suspend or resume activity.
	 */
	spin_lock_irqsave(&ohci->lock, flags);
	if (ohci->rh_state != OHCI_RH_SUSPENDED &&
			ohci->rh_state != OHCI_RH_HALTED) {
		rc = -EINVAL;
		goto fail;
	}

	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, NXP_USB_PHY_OHCI);
fail:
	spin_unlock_irqrestore(&ohci->lock, flags);

	return rc;
}

static int nxp_ohci_resume(struct device *dev)
{
	struct nxp_ohci_hcd *nxp_ohci = dev_get_drvdata(dev);
	struct usb_hcd *hcd = nxp_ohci->hcd;
	struct platform_device *pdev = to_platform_device(dev);
	struct nxp_ohci_platdata *pdata = pdev->dev.platform_data;

	if (nxp_ohci->phy)
		return 0;

	if (pdata && pdata->phy_init)
		pdata->phy_init(pdev, NXP_USB_PHY_OHCI);

	/* Mark hardware accessible again as we are out of D3 state by now */
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	ohci_finish_controller_resume(hcd);

	return 0;
}
#else
#define nxp_ohci_suspend	NULL
#define nxp_ohci_resume	NULL
#endif

static const struct dev_pm_ops nxp_ohci_pm_ops = {
	.suspend	= nxp_ohci_suspend,
	.resume		= nxp_ohci_resume,
};

static struct platform_driver nxp_ohci_driver = {
	.probe		= nxp_ohci_probe,
	.remove		= __devexit_p(nxp_ohci_remove),
	.shutdown	= nxp_ohci_shutdown,
	.driver = {
		.name	= "nxp-ohci",
		.owner	= THIS_MODULE,
		.pm	= &nxp_ohci_pm_ops,
	}
};

MODULE_ALIAS("platform:nxp-ohci");
MODULE_AUTHOR("Nexell Co., Ltd.");
MODULE_AUTHOR("BongKwan Kook <kook@nexell.co.kr>");
