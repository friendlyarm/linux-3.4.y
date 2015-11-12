/*
 * Copyright (C) 2012 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define USB_HOST_HUB    (1)

#define pr_fmt(fmt) "s5p4418_otg %s: " fmt, __func__

#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/hcd.h>
#include <linux/usb/otg.h>

#include <mach/usb-phy.h>
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>


struct s5p4418_otg {
    struct usb_otg      otg;
    struct usb_phy      phy;
    struct delayed_work work;
    struct mutex        lock;
    bool                usb_connected;
    struct usb_bus      *ohci;

    /* HACK: s5p4418 phy interface requires passing a pdev pointer */
    struct platform_device  pdev;
};

static struct s5p4418_otg   s_s5p4418_otg;
//static bool                 s_host_init;


static int s5p4418_phy_init(struct usb_phy *phy)
{
#if !defined(USB_HOST_HUB)
	struct s5p4418_otg *nxpotg = container_of(phy, struct s5p4418_otg, phy);

	if (phy->last_event == USB_EVENT_VBUS)
		return nxp_usb_phy_init(&nxpotg->pdev, NXP_USB_PHY_DEVICE);
	else
		return nxp_usb_phy_init(&nxpotg->pdev, NXP_USB_PHY_HOST);
#else

    return 0;
#endif
}

static void s5p4418_phy_shutdown(struct usb_phy *phy)
{
#if !defined(USB_HOST_HUB)
	struct s5p4418_otg *nxpotg = container_of(phy, struct s5p4418_otg, phy);

	if (nxpotg->phy.state == OTG_STATE_B_PERIPHERAL)
		nxp_usb_phy_exit(&nxpotg->pdev, NXP_USB_PHY_DEVICE);
	else
		nxp_usb_phy_exit(&nxpotg->pdev, NXP_USB_PHY_HOST);
#endif
}

static int s5p4418_phy_set_power(struct usb_phy *phy, unsigned mA)
{
	if (mA > 3)
		atomic_notifier_call_chain(&phy->notifier, USB_EVENT_ENUMERATED,
					   phy->otg->gadget);

	return 0;
}

static int s5p4418_otg_host_init(struct s5p4418_otg *nxpotg)
{
#ifdef CONFIG_USB
	struct usb_hcd *hcd = NULL;
	struct usb_hcd *ohci_hcd = NULL;
	int err;

	if (nxpotg->otg.host)
		hcd = bus_to_hcd(nxpotg->otg.host);

	if (hcd) {
		err = usb_add_hcd(hcd, hcd->irq, IRQF_SHARED);
		if (err) {
			pr_err("failed to add ehci: %d\n", err);
			goto err_ehci;
		}
	}

    if (nxpotg->ohci)
        ohci_hcd = bus_to_hcd(nxpotg->ohci);

	if (ohci_hcd) {
		err = usb_add_hcd(ohci_hcd, hcd->irq, IRQF_SHARED);
		if (err) {
			pr_err("failed to add ohci: %d\n", err);
			goto err_ohci;
		}
	}

	return 0;

err_ohci:
	if (hcd)
		usb_remove_hcd(hcd);
err_ehci:

	return err;
#else
	return 0;
#endif  // #ifdef CONFIG_USB
}


static int s5p4418_otg_host_enable(struct s5p4418_otg *nxpotg)
{
#ifdef CONFIG_USB
	struct usb_hcd *hcd = NULL;
	struct usb_hcd *ohci_hcd = NULL;
	int err;

	usb_phy_init(&nxpotg->phy);

#if !defined(USB_HOST_HUB)
    err = s5p4418_otg_host_init(nxpotg);
    if (err) {
        pr_err("failed to s5p4418_otg_host_enable: %d\n", err);
        goto err_vbus;
    }
#endif

	err = otg_set_vbus(&nxpotg->otg, true);
	if (err) {
		pr_err("failed to enable vbus: %d\n", err);
		goto err_vbus;
	}

	return 0;

err_vbus:
	usb_phy_shutdown(&nxpotg->phy);

	return err;
#else
	return 0;
#endif  // #ifdef CONFIG_USB
}

static void s5p4418_otg_host_disable(struct s5p4418_otg *nxpotg)
{
#ifdef CONFIG_USB
#if !defined(USB_HOST_HUB)
	struct usb_hcd *hcd = NULL;
	struct usb_hcd *ohci_hcd = NULL;
#endif
	int err;

#if !defined(USB_HOST_HUB)
	if (nxpotg->otg.host)
		hcd = bus_to_hcd(nxpotg->otg.host);

	if (nxpotg->ohci)
		ohci_hcd = bus_to_hcd(nxpotg->ohci);
#endif

	err = otg_set_vbus(&nxpotg->otg, false);
	if (err)
		pr_err("failed to disable vbus: %d\n", err);

#if !defined(USB_HOST_HUB)
	if (ohci_hcd)
		usb_remove_hcd(ohci_hcd);
	if (hcd)
		usb_remove_hcd(hcd);
#endif

	usb_phy_shutdown(&nxpotg->phy);
#endif
}

static int s5p4418_otg_set_host(struct usb_otg *otg, struct usb_bus *host)
{
#ifdef CONFIG_USB
	struct s5p4418_otg *nxpotg = container_of(otg, struct s5p4418_otg, otg);
#if !defined(USB_HOST_HUB)
	struct usb_hcd *hcd = bus_to_hcd(host);
#endif

	mutex_lock(&nxpotg->lock);


#if defined(USB_HOST_HUB)
    otg->host = host;

#else

	/* HACK to support for ohci */
	if (host && otg->host) {
		nxpotg->ohci = host;
		usb_remove_hcd(hcd);
		usb_phy_shutdown(&nxpotg->phy);
		goto out;
	}

	otg->host = host;
	if (host) {
		usb_remove_hcd(bus_to_hcd(host));
		usb_phy_shutdown(&nxpotg->phy);
	} else {
		if (otg->phy->state == OTG_STATE_A_HOST)
			otg->phy->state = OTG_STATE_UNDEFINED;
	}

out:
#endif
	mutex_unlock(&nxpotg->lock);
#endif
	return 0;
}

static int s5p4418_otg_set_peripheral(struct usb_otg *otg,
				    struct usb_gadget *gadget)
{
	struct s5p4418_otg *nxpotg = container_of(otg, struct s5p4418_otg, otg);

	mutex_lock(&nxpotg->lock);

	otg->gadget = gadget;
	if (gadget) {
		if (otg->phy->state == OTG_STATE_B_PERIPHERAL)
			usb_gadget_vbus_connect(nxpotg->otg.gadget);
	} else {
		if (otg->phy->state == OTG_STATE_B_PERIPHERAL)
			otg->phy->state = OTG_STATE_UNDEFINED;
	}

	mutex_unlock(&nxpotg->lock);

	return 0;
}

static int s5p4418_otg_set_vbus(struct usb_otg *otg, bool enabled)
{
	pr_debug("vbus %s\n", enabled ? "on" : "off");

#if !defined(USB_HOST_HUB)
	if (enabled)
        gpio_set_value(CFG_GPIO_OTG_VBUS_DET, 1);
    else
        gpio_set_value(CFG_GPIO_OTG_VBUS_DET, 0);
#endif

	return 0;
}

static void s5p4418_otg_work(struct work_struct *work)
{
	struct s5p4418_otg *nxpotg = container_of(work, struct s5p4418_otg, work.work);
	enum usb_otg_state prev_state;
	int id, vbus, err;

	mutex_lock(&nxpotg->lock);

	prev_state = nxpotg->phy.state;
//	vbus = nxpotg->usb_connected;
//	vbus = gpio_get_value(CFG_GPIO_PMIC_VUSB_DET);
	vbus = 0;
	id = gpio_get_value(CFG_GPIO_OTG_USBID_DET);

	pr_debug("vbus=%d id=%d\n", vbus, id);

	if (!id) {
		if (prev_state == OTG_STATE_A_HOST)
			goto out;

		if (!nxpotg->otg.host)
			goto out;

		nxpotg->phy.state = OTG_STATE_A_HOST;
		nxpotg->phy.last_event = USB_EVENT_ID;
		atomic_notifier_call_chain(&nxpotg->phy.notifier,
					USB_EVENT_ID, NULL);

		err = s5p4418_otg_host_enable(nxpotg);
		if (err) {
			nxpotg->phy.last_event = USB_EVENT_NONE;
			nxpotg->phy.state = OTG_STATE_B_IDLE;
			atomic_notifier_call_chain(&nxpotg->phy.notifier,
						USB_EVENT_NONE, NULL);
			goto out;
		}
	} else if (vbus) {
		if (!nxpotg->otg.gadget)
			goto out;

		if (prev_state == OTG_STATE_B_PERIPHERAL)
			goto out;

		nxpotg->phy.state = OTG_STATE_B_PERIPHERAL;
		nxpotg->phy.last_event = USB_EVENT_VBUS;
		atomic_notifier_call_chain(&nxpotg->phy.notifier,
					USB_EVENT_VBUS, NULL);
		usb_gadget_vbus_connect(nxpotg->otg.gadget);
	} else {
		if (prev_state == OTG_STATE_B_IDLE)
			goto out;

		if (prev_state == OTG_STATE_B_PERIPHERAL && nxpotg->otg.gadget)
			usb_gadget_vbus_disconnect(nxpotg->otg.gadget);

		if (prev_state == OTG_STATE_A_HOST && nxpotg->otg.host)
			s5p4418_otg_host_disable(nxpotg);

		nxpotg->phy.state = OTG_STATE_B_IDLE;
		nxpotg->phy.last_event = USB_EVENT_NONE;
		atomic_notifier_call_chain(&nxpotg->phy.notifier,
					USB_EVENT_NONE, NULL);
	}

	pr_info("%s -> %s\n", otg_state_string(prev_state),
			      otg_state_string(nxpotg->phy.state));

out:
	mutex_unlock(&nxpotg->lock);
}

static irqreturn_t s5p4418_otg_irq(int irq, void *data)
{
	struct s5p4418_otg *nxpotg = data;
	queue_delayed_work(system_nrt_wq, &nxpotg->work, msecs_to_jiffies(20));
	return IRQ_HANDLED;
}

void s5p4418_otg_set_usb_state(bool connected)
{
	struct s5p4418_otg *nxpotg = &s_s5p4418_otg;

	nxpotg->usb_connected = connected;
	queue_delayed_work(system_nrt_wq, &nxpotg->work,
			   connected ? msecs_to_jiffies(20) : 0);
	if (!connected)
		flush_delayed_work(&nxpotg->work);
}

void nxp_usb_connector_init(void)
{
	struct s5p4418_otg *nxpotg = &s_s5p4418_otg;
	struct device *dev = &nxpotg->pdev.dev;

	INIT_DELAYED_WORK(&nxpotg->work, s5p4418_otg_work);
	ATOMIC_INIT_NOTIFIER_HEAD(&nxpotg->phy.notifier);
	mutex_init(&nxpotg->lock);

	device_initialize(dev);
	dev_set_name(dev, "%s", "s5p4418_otg");
	if (device_add(dev)) {
		dev_err(dev, "%s: cannot reg device\n", __func__);
		return;
	}
	dev_set_drvdata(dev, nxpotg);

	nxpotg->phy.dev         = dev;
	nxpotg->phy.label       = "s5p4418_otg";
	nxpotg->phy.otg         = &nxpotg->otg;
	nxpotg->phy.init        = s5p4418_phy_init;
	nxpotg->phy.shutdown    = s5p4418_phy_shutdown;
	nxpotg->phy.set_power   = s5p4418_phy_set_power;

	nxpotg->otg.phy             = &nxpotg->phy;
	nxpotg->otg.set_host        = s5p4418_otg_set_host;
	nxpotg->otg.set_peripheral  = s5p4418_otg_set_peripheral;
	nxpotg->otg.set_vbus        = s5p4418_otg_set_vbus;

	usb_set_transceiver(&nxpotg->phy);
}

static int __init s5p4418_connector_init(void)
{
	struct s5p4418_otg *nxpotg = &s_s5p4418_otg;
	int ret;

    nxp_usb_connector_init();

    nxp_soc_gpio_set_int_enable(CFG_GPIO_OTG_USBID_DET, 0);

#if 0
	ret = request_irq(gpio_to_irq(CFG_GPIO_OTG_USBID_DET), s5p4418_otg_irq,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
			IRQF_ONESHOT, "usb_id", nxpotg);
#else
	ret = request_irq(gpio_to_irq(CFG_GPIO_OTG_USBID_DET), s5p4418_otg_irq,
			IRQ_TYPE_EDGE_BOTH, "usb_id", nxpotg);
#endif
	if (ret)
	{
		pr_err("request_irq ID failed: %d\n", ret);
	}
    else
	{
//		nxp_soc_gpio_set_int_mode(CFG_GPIO_OTG_USBID_DET, 4);
		nxp_soc_gpio_set_int_enable(CFG_GPIO_OTG_USBID_DET, 1);
		nxp_soc_gpio_clr_int_pend(CFG_GPIO_OTG_USBID_DET);
	}

	/*
	 * HACK: delay the initial otg detection by 4 secs so that it happens
	 * after the battery driver is ready (this fixes booting with otg cable
	 * attached)
	 */
	queue_delayed_work(system_nrt_wq, &nxpotg->work, msecs_to_jiffies(4000));

	return ret;
}
device_initcall(s5p4418_connector_init);
