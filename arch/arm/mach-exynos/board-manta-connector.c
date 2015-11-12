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

#define pr_fmt(fmt) "manta_otg %s: " fmt, __func__

#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/hcd.h>
#include <linux/usb/otg.h>

#include <plat/gpio-cfg.h>
#include <plat/devs.h>
#include <plat/usb-phy.h>

#include "board-manta.h"

#define GPIO_USB_VBUS	EXYNOS5_GPX1(0)
#define GPIO_USB_ID	EXYNOS5_GPX1(1)

struct manta_otg {
	struct usb_otg		otg;
	struct usb_phy		phy;
	struct delayed_work	work;
	struct mutex		lock;
	bool			usb_connected;
	struct usb_bus		*ohci;

	/* HACK: s5p phy interface requires passing a pdev pointer */
	struct platform_device	pdev;
};
static struct manta_otg manta_otg;


static int manta_phy_init(struct usb_phy *phy)
{
	struct manta_otg *motg = container_of(phy, struct manta_otg, phy);

	if (phy->last_event == USB_EVENT_VBUS)
		return s5p_usb_phy_init(&motg->pdev, S5P_USB_PHY_DEVICE);
	else
		return s5p_usb_phy_init(&motg->pdev, S5P_USB_PHY_HOST);
}

static void manta_phy_shutdown(struct usb_phy *phy)
{
	struct manta_otg *motg = container_of(phy, struct manta_otg, phy);

	if (motg->phy.state == OTG_STATE_B_PERIPHERAL)
		s5p_usb_phy_exit(&motg->pdev, S5P_USB_PHY_DEVICE);
	else
		s5p_usb_phy_exit(&motg->pdev, S5P_USB_PHY_HOST);
}

static int manta_phy_set_power(struct usb_phy *phy, unsigned mA)
{
	if (mA > 3)
		atomic_notifier_call_chain(&phy->notifier, USB_EVENT_ENUMERATED,
					   phy->otg->gadget);

	return 0;
}

static int manta_otg_host_enable(struct manta_otg *motg)
{
#ifdef CONFIG_USB
	struct usb_hcd *hcd;
	struct usb_hcd *ohci_hcd = NULL;
	int err;

	hcd = bus_to_hcd(motg->otg.host);
	if (motg->ohci)
		ohci_hcd = bus_to_hcd(motg->ohci);

	usb_phy_init(&motg->phy);

	err = usb_add_hcd(hcd, hcd->irq, IRQF_SHARED);
	if (err) {
		pr_err("failed to add ehci: %d\n", err);
		goto err_ehci;
	}

	if (ohci_hcd) {
		err = usb_add_hcd(ohci_hcd, hcd->irq, IRQF_SHARED);
		if (err) {
			pr_err("failed to add ohci: %d\n", err);
			goto err_ohci;
		}
	}

	err = otg_set_vbus(&motg->otg, true);
	if (err) {
		pr_err("failed to enable vbus: %d\n", err);
		goto err_vbus;
	}

	return 0;

err_vbus:
	if (ohci_hcd)
		usb_remove_hcd(ohci_hcd);

err_ohci:
	usb_remove_hcd(hcd);

err_ehci:
	usb_phy_shutdown(&motg->phy);

	return err;
#else
	return 0;
#endif
}

static void manta_otg_host_disable(struct manta_otg *motg)
{
#ifdef CONFIG_USB
	struct usb_hcd *hcd;
	struct usb_hcd *ohci_hcd = NULL;
	int err;

	hcd = bus_to_hcd(motg->otg.host);
	if (motg->ohci)
		ohci_hcd = bus_to_hcd(motg->ohci);

	err = otg_set_vbus(&motg->otg, false);
	if (err)
		pr_err("failed to disable vbus: %d\n", err);

	if (ohci_hcd)
		usb_remove_hcd(ohci_hcd);
	usb_remove_hcd(hcd);

	usb_phy_shutdown(&motg->phy);
#endif
}

static int manta_otg_set_host(struct usb_otg *otg, struct usb_bus *host)
{
#ifdef CONFIG_USB
	struct manta_otg *motg = container_of(otg, struct manta_otg, otg);
	struct usb_hcd *hcd = bus_to_hcd(host);

	mutex_lock(&motg->lock);

	/* HACK to support for ohci */
	if (host && otg->host) {
		motg->ohci = host;
		usb_remove_hcd(hcd);
		usb_phy_shutdown(&motg->phy);
		goto out;
	}

	otg->host = host;
	if (host) {
		usb_remove_hcd(bus_to_hcd(host));
		usb_phy_shutdown(&motg->phy);
	} else {
		if (otg->phy->state == OTG_STATE_A_HOST)
			otg->phy->state = OTG_STATE_UNDEFINED;
	}

out:
	mutex_unlock(&motg->lock);
#endif
	return 0;
}

static int manta_otg_set_peripheral(struct usb_otg *otg,
				    struct usb_gadget *gadget)
{
	struct manta_otg *motg = container_of(otg, struct manta_otg, otg);

	mutex_lock(&motg->lock);

	otg->gadget = gadget;
	if (gadget) {
		if (otg->phy->state == OTG_STATE_B_PERIPHERAL)
			usb_gadget_vbus_connect(motg->otg.gadget);
	} else {
		if (otg->phy->state == OTG_STATE_B_PERIPHERAL)
			otg->phy->state = OTG_STATE_UNDEFINED;
	}

	mutex_unlock(&motg->lock);

	return 0;
}

static int manta_otg_set_vbus(struct usb_otg *otg, bool enabled)
{
	pr_debug("vbus %s\n", enabled ? "on" : "off");
	return manta_bat_otg_enable(enabled);
}

static void manta_otg_work(struct work_struct *work)
{
	struct manta_otg *motg = container_of(work, struct manta_otg, work.work);
	enum usb_otg_state prev_state;
	int id, vbus, err;

	mutex_lock(&motg->lock);

	prev_state = motg->phy.state;
	vbus = motg->usb_connected;
	id = gpio_get_value(GPIO_USB_ID);

	pr_debug("vbus=%d id=%d\n", vbus, id);

	if (!id) {
		if (prev_state == OTG_STATE_A_HOST)
			goto out;

		if (!motg->otg.host)
			goto out;

		motg->phy.state = OTG_STATE_A_HOST;
		motg->phy.last_event = USB_EVENT_ID;
		atomic_notifier_call_chain(&motg->phy.notifier,
					USB_EVENT_ID, NULL);

		err = manta_otg_host_enable(motg);
		if (err) {
			motg->phy.last_event = USB_EVENT_NONE;
			motg->phy.state = OTG_STATE_B_IDLE;
			atomic_notifier_call_chain(&motg->phy.notifier,
						USB_EVENT_NONE, NULL);
			goto out;
		}

	} else if (vbus) {
		if (!motg->otg.gadget)
			goto out;

		if (prev_state == OTG_STATE_B_PERIPHERAL)
			goto out;

		motg->phy.state = OTG_STATE_B_PERIPHERAL;
		motg->phy.last_event = USB_EVENT_VBUS;
		atomic_notifier_call_chain(&motg->phy.notifier,
					USB_EVENT_VBUS, NULL);
		usb_gadget_vbus_connect(motg->otg.gadget);
	} else {
		if (prev_state == OTG_STATE_B_IDLE)
			goto out;

		if (prev_state == OTG_STATE_B_PERIPHERAL && motg->otg.gadget)
			usb_gadget_vbus_disconnect(motg->otg.gadget);

		if (prev_state == OTG_STATE_A_HOST && motg->otg.host)
			manta_otg_host_disable(motg);

		motg->phy.state = OTG_STATE_B_IDLE;
		motg->phy.last_event = USB_EVENT_NONE;
		atomic_notifier_call_chain(&motg->phy.notifier,
					USB_EVENT_NONE, NULL);
	}

	pr_info("%s -> %s\n", otg_state_string(prev_state),
			      otg_state_string(motg->phy.state));

out:
	mutex_unlock(&motg->lock);
}

static irqreturn_t manta_otg_irq(int irq, void *data)
{
	struct manta_otg *motg = data;
	queue_delayed_work(system_nrt_wq, &motg->work, msecs_to_jiffies(20));
	return IRQ_HANDLED;
}

void manta_otg_set_usb_state(bool connected)
{
	struct manta_otg *motg = &manta_otg;
	motg->usb_connected = connected;
	queue_delayed_work(system_nrt_wq, &motg->work,
			   connected ? msecs_to_jiffies(20) : 0);
	if (!connected)
		flush_delayed_work(&motg->work);
}

void exynos5_manta_connector_init(void)
{
	struct manta_otg *motg = &manta_otg;
	struct device *dev = &motg->pdev.dev;

	INIT_DELAYED_WORK(&motg->work, manta_otg_work);
	ATOMIC_INIT_NOTIFIER_HEAD(&motg->phy.notifier);
	mutex_init(&motg->lock);

	device_initialize(dev);
	dev_set_name(dev, "%s", "manta_otg");
	if (device_add(dev)) {
		dev_err(dev, "%s: cannot reg device\n", __func__);
		return;
	}
	dev_set_drvdata(dev, motg);

	motg->phy.dev		= dev;
	motg->phy.label		= "manta_otg";
	motg->phy.otg		= &motg->otg;
	motg->phy.init		= manta_phy_init;
	motg->phy.shutdown	= manta_phy_shutdown;
	motg->phy.set_power	= manta_phy_set_power;

	motg->otg.phy		 = &motg->phy;
	motg->otg.set_host	 = manta_otg_set_host;
	motg->otg.set_peripheral = manta_otg_set_peripheral;
	motg->otg.set_vbus	 = manta_otg_set_vbus;

	usb_set_transceiver(&motg->phy);
}

static int __init manta_connector_init(void)
{
	struct manta_otg *motg = &manta_otg;
	int ret;

	s3c_gpio_cfgpin(GPIO_USB_ID, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_USB_ID, S3C_GPIO_PULL_NONE);

	ret = request_irq(gpio_to_irq(GPIO_USB_ID), manta_otg_irq,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
			IRQF_ONESHOT, "usb_id", motg);
	if (ret)
		pr_err("request_irq ID failed: %d\n", ret);

	/*
	 * HACK: delay the initial otg detection by 4 secs so that it happens
	 * after the battery driver is ready (this fixes booting with otg cable
	 * attached)
	 */
	queue_delayed_work(system_nrt_wq, &motg->work, msecs_to_jiffies(4000));

	return ret;
}
device_initcall(manta_connector_init);
