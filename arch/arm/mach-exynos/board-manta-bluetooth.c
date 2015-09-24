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
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/wakelock.h>

#include <net/bluetooth/bluetooth.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#include "board-manta.h"

#define GPIO_BT_WAKE		EXYNOS5_GPH1(3)
#define GPIO_BT_HOST_WAKE	EXYNOS5_GPX2(6)
#define GPIO_BTREG_ON		EXYNOS5_GPH0(0)

#define GPIO_BT_UART_RXD	EXYNOS5_GPA0(0)
#define GPIO_BT_UART_TXD	EXYNOS5_GPA0(1)
#define GPIO_BT_UART_CTS	EXYNOS5_GPA0(2)
#define GPIO_BT_UART_RTS	EXYNOS5_GPA0(3)

#define BT_LPM_ENABLE

static struct rfkill *bt_rfkill;

static DEFINE_MUTEX(manta_bt_wlan_sync);

struct bcm_bt_lpm {
	int wake;
	int host_wake;

	struct hrtimer enter_lpm_timer;
	ktime_t enter_lpm_delay;

	struct uart_port *uport;

	struct wake_lock wake_lock;
	struct wake_lock host_wake_lock;
} bt_lpm;

struct gpio_init_data {
	uint num;
	uint cfg;
	uint pull;
	uint drv;
};

struct gpio_sleep_data {
	uint num;
	uint cfg;
	uint pull;
};

static struct gpio_init_data manta_init_bt_gpios[] = {
	/* BT_UART_RXD */
	{GPIO_BT_UART_RXD, S3C_GPIO_SFN(2), S3C_GPIO_PULL_UP,
							S5P_GPIO_DRVSTR_LV2},
	/* BT_UART_TXD */
	{GPIO_BT_UART_TXD, S3C_GPIO_SFN(2), S3C_GPIO_PULL_NONE,
							S5P_GPIO_DRVSTR_LV2},
	/* BT_UART_CTS */
	{GPIO_BT_UART_CTS, S3C_GPIO_SFN(2), S3C_GPIO_PULL_NONE,
							S5P_GPIO_DRVSTR_LV2},
	/* BT_UART_RTS */
	{GPIO_BT_UART_RTS, S3C_GPIO_SFN(2), S3C_GPIO_PULL_NONE,
							S5P_GPIO_DRVSTR_LV2},
	/* BT_HOST_WAKE */
	{GPIO_BT_HOST_WAKE, S3C_GPIO_INPUT, S3C_GPIO_PULL_NONE,
							S5P_GPIO_DRVSTR_LV1}
};

static struct gpio_sleep_data manta_sleep_bt_gpios[] = {
	/* BT_UART_RXD */
	{GPIO_BT_UART_RXD, S5P_GPIO_PD_INPUT, S5P_GPIO_PD_UP_ENABLE},
	/* BT_UART_TXD */
	{GPIO_BT_UART_TXD, S5P_GPIO_PD_OUTPUT0, S5P_GPIO_PD_UPDOWN_DISABLE},
	/* BT_UART_CTS */
	{GPIO_BT_UART_CTS, S5P_GPIO_PD_INPUT, S5P_GPIO_PD_UPDOWN_DISABLE},
	/* BT_UART_RTS */
	{GPIO_BT_UART_RTS, S5P_GPIO_PD_OUTPUT1, S5P_GPIO_PD_UPDOWN_DISABLE},
	/* BTREG_ON */
	{GPIO_BTREG_ON, S5P_GPIO_PD_PREV_STATE, S5P_GPIO_PD_UPDOWN_DISABLE},
	/* BT_WAKE */
	{GPIO_BT_WAKE, S5P_GPIO_PD_PREV_STATE, S5P_GPIO_PD_UPDOWN_DISABLE},
};

static struct platform_device bcm43241_bluetooth_platform_device = {
	.name		= "bcm43241_bluetooth",
	.id		= -1,
};

static struct platform_device *manta_bt_devs[] __initdata = {
	&bcm43241_bluetooth_platform_device,
};

void __init exynos5_manta_bt_init(void)
{
	int i;
	int gpio;

	for (i = 0; i < ARRAY_SIZE(manta_init_bt_gpios); i++) {
		gpio = manta_init_bt_gpios[i].num;
		s3c_gpio_cfgpin(gpio, manta_init_bt_gpios[i].cfg);
		s3c_gpio_setpull(gpio, manta_init_bt_gpios[i].pull);
		s5p_gpio_set_drvstr(gpio, manta_init_bt_gpios[i].drv);
	}

	for (i = 0; i < ARRAY_SIZE(manta_sleep_bt_gpios); i++) {
		gpio = manta_sleep_bt_gpios[i].num;
		s5p_gpio_set_pd_cfg(gpio, manta_sleep_bt_gpios[i].cfg);
		s5p_gpio_set_pd_pull(gpio, manta_sleep_bt_gpios[i].pull);
	}

	platform_add_devices(manta_bt_devs, ARRAY_SIZE(manta_bt_devs));
}

void bt_wlan_lock(void)
{
	mutex_lock(&manta_bt_wlan_sync);
}

void bt_wlan_unlock(void)
{
	mutex_unlock(&manta_bt_wlan_sync);
}

static int bcm43241_bt_rfkill_set_power(void *data, bool blocked)
{
	/* rfkill_ops callback. Turn transmitter on when blocked is false */
	bt_wlan_lock();
	msleep(300);
	if (!blocked) {
		pr_info("[BT] Bluetooth Power On.\n");
		gpio_set_value(GPIO_BTREG_ON, 1);
		s3c_gpio_setpull(GPIO_BT_HOST_WAKE, S3C_GPIO_PULL_NONE);
	} else {
		pr_info("[BT] Bluetooth Power Off.\n");
		gpio_set_value(GPIO_BTREG_ON, 0);
		s3c_gpio_setpull(GPIO_BT_HOST_WAKE, S3C_GPIO_PULL_DOWN);
	}
	msleep(50);
	bt_wlan_unlock();
	return 0;
}

static const struct rfkill_ops bcm43241_bt_rfkill_ops = {
	.set_block = bcm43241_bt_rfkill_set_power,
};

#ifdef BT_LPM_ENABLE
static void set_wake_locked(int wake)
{
	if (wake == bt_lpm.wake)
		return;

	bt_lpm.wake = wake;

	if (wake) {
		wake_lock(&bt_lpm.wake_lock);
		gpio_set_value(GPIO_BT_WAKE, wake);
	} else {
		gpio_set_value(GPIO_BT_WAKE, wake);
		wake_unlock(&bt_lpm.wake_lock);
	}
}

static enum hrtimer_restart enter_lpm(struct hrtimer *timer)
{
	set_wake_locked(0);

	return HRTIMER_NORESTART;
}

void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport)
{
	bt_lpm.uport = uport;

	hrtimer_try_to_cancel(&bt_lpm.enter_lpm_timer);
	set_wake_locked(1);

	hrtimer_start(&bt_lpm.enter_lpm_timer, bt_lpm.enter_lpm_delay,
		HRTIMER_MODE_REL);
}

static void update_host_wake_locked(int host_wake)
{
	if (host_wake == bt_lpm.host_wake)
		return;

	bt_lpm.host_wake = host_wake;

	if (host_wake) {
		wake_lock(&bt_lpm.host_wake_lock);
	} else  {
		/* Take a timed wakelock, so that upper layers can take it.
		* The chipset deasserts the hostwake lock, when there is no
		* more data to send.
		*/
		wake_lock_timeout(&bt_lpm.host_wake_lock, HZ/2);
	}
}

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	int host_wake;

	host_wake = gpio_get_value(GPIO_BT_HOST_WAKE);
	irq_set_irq_type(irq, host_wake ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);

	if (!bt_lpm.uport) {
		bt_lpm.host_wake = host_wake;
		return IRQ_HANDLED;
	}

	update_host_wake_locked(host_wake);

	return IRQ_HANDLED;
}

static int bcm_bt_lpm_init(struct platform_device *pdev)
{
	int irq;
	int ret;

	pr_info("[BT] bcm_bt_lpm_init\n");
	hrtimer_init(&bt_lpm.enter_lpm_timer, CLOCK_MONOTONIC,
			HRTIMER_MODE_REL);
	bt_lpm.enter_lpm_delay = ktime_set(3, 0);
	bt_lpm.enter_lpm_timer.function = enter_lpm;

	bt_lpm.host_wake = 0;

	wake_lock_init(&bt_lpm.wake_lock, WAKE_LOCK_SUSPEND,
			 "BTWakeLowPower");
	wake_lock_init(&bt_lpm.host_wake_lock, WAKE_LOCK_SUSPEND,
			 "BTHostWakeLowPower");

	irq = gpio_to_irq(GPIO_BT_HOST_WAKE);
	ret = request_threaded_irq(irq, NULL, host_wake_isr,
		IRQF_TRIGGER_HIGH | IRQF_ONESHOT, "bt_host_wake", NULL);
	if (ret) {
		pr_err("[BT] Request host_wake irq failed.\n");
		goto err_lpm_init;
	}

	ret = irq_set_irq_wake(irq, 1);
	if (ret) {
		pr_err("[BT] Set_irq_wake failed.\n");
		free_irq(irq, NULL);
		goto err_lpm_init;
	}

	return 0;

err_lpm_init:
	wake_lock_destroy(&bt_lpm.wake_lock);
	wake_lock_destroy(&bt_lpm.host_wake_lock);
	return ret;
}
#endif

static int bcm43241_bluetooth_probe(struct platform_device *pdev)
{
	int rc;

	rc = gpio_request(GPIO_BTREG_ON, "bcm43241_bten_gpio");
	if (unlikely(rc)) {
		pr_err("[BT] GPIO_BTREG_ON request failed.\n");
		goto err_gpio_btreg_on;
	}

	rc = gpio_request(GPIO_BT_WAKE, "bcm43241_btwake_gpio");
	if (unlikely(rc)) {
		pr_err("[BT] GPIO_BT_WAKE request failed.\n");
		goto err_gpio_bt_wake;
	}

	rc = gpio_request(GPIO_BT_HOST_WAKE, "bcm43241_bthostwake_gpio");
	if (unlikely(rc)) {
		pr_err("[BT] GPIO_BT_HOST_WAKE request failed.\n");
		goto err_gpio_bt_host_wake;
	}

	gpio_direction_input(GPIO_BT_HOST_WAKE);
	gpio_direction_output(GPIO_BT_WAKE, 0);
	gpio_direction_output(GPIO_BTREG_ON, 0);

	bt_rfkill = rfkill_alloc("bcm43241 Bluetooth", &pdev->dev,
				RFKILL_TYPE_BLUETOOTH, &bcm43241_bt_rfkill_ops,
				NULL);
	if (unlikely(!bt_rfkill)) {
		pr_err("[BT] bt_rfkill alloc failed.\n");
		rc =  -ENOMEM;
		goto err_rfkill_alloc;
	}

	rfkill_init_sw_state(bt_rfkill, false);
	rc = rfkill_register(bt_rfkill);
	if (unlikely(rc)) {
		pr_err("[BT] bt_rfkill register failed.\n");
		rc = -1;
		goto err_rfkill_register;
	}
	rfkill_set_sw_state(bt_rfkill, true);

#ifdef BT_LPM_ENABLE
	rc = bcm_bt_lpm_init(pdev);
	if (rc)
		goto err_lpm_init;
#endif
	return rc;

#ifdef BT_LPM_ENABLE
err_lpm_init:
	rfkill_unregister(bt_rfkill);
#endif
err_rfkill_register:
	rfkill_destroy(bt_rfkill);
err_rfkill_alloc:
	gpio_free(GPIO_BT_HOST_WAKE);
err_gpio_bt_host_wake:
	gpio_free(GPIO_BT_WAKE);
err_gpio_bt_wake:
	gpio_free(GPIO_BTREG_ON);
err_gpio_btreg_on:
	return rc;
}

static int bcm43241_bluetooth_remove(struct platform_device *pdev)
{
#ifdef BT_LPM_ENABLE
	int irq;

	irq = gpio_to_irq(GPIO_BT_HOST_WAKE);
	irq_set_irq_wake(irq, 0);
	free_irq(irq, NULL);
	set_wake_locked(0);
	hrtimer_try_to_cancel(&bt_lpm.enter_lpm_timer);
	wake_lock_destroy(&bt_lpm.wake_lock);
	wake_lock_destroy(&bt_lpm.host_wake_lock);
#endif

	rfkill_unregister(bt_rfkill);
	rfkill_destroy(bt_rfkill);

	gpio_free(GPIO_BT_HOST_WAKE);
	gpio_free(GPIO_BT_WAKE);
	gpio_free(GPIO_BTREG_ON);

	return 0;
}

static struct platform_driver bcm43241_bluetooth_platform_driver = {
	.probe = bcm43241_bluetooth_probe,
	.remove = bcm43241_bluetooth_remove,
	.driver = {
		   .name = "bcm43241_bluetooth",
		   .owner = THIS_MODULE,
	},
};

static int __init bcm43241_bluetooth_init(void)
{
	return platform_driver_register(&bcm43241_bluetooth_platform_driver);
}

static void __exit bcm43241_bluetooth_exit(void)
{
	platform_driver_unregister(&bcm43241_bluetooth_platform_driver);
}


module_init(bcm43241_bluetooth_init);
module_exit(bcm43241_bluetooth_exit);

MODULE_ALIAS("platform:bcm43241");
MODULE_DESCRIPTION("bcm43241_bluetooth");
MODULE_LICENSE("GPL");
