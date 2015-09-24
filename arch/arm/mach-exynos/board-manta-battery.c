/* linux/arch/arm/mach-exynos/board-manta-battery.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/suspend.h>
#include <linux/pm_wakeup.h>
#include <linux/wakelock.h>
#include <linux/notifier.h>
#include <linux/timer.h>
#include <linux/usb/otg.h>

#include <plat/adc.h>
#include <plat/gpio-cfg.h>

#include <linux/power/smb347-charger.h>
#include <linux/platform_data/ds2482.h>

#include "board-manta.h"

#include <linux/slab.h>

#define TA_ADC_LOW		700
#define TA_ADC_HIGH		1750

#define ADC_NUM_SAMPLES		5
#define ADC_LIMIT_ERR_COUNT	5

#define	GPIO_USB_SEL1		EXYNOS5_GPH0(1)
#define	GPIO_TA_EN		EXYNOS5_GPG1(5)
#define	GPIO_TA_INT		EXYNOS5_GPX0(0)
#define	GPIO_TA_NCHG		EXYNOS5_GPX0(4)
#define GPIO_OTG_VBUS_SENSE	EXYNOS5_GPX1(0)
#define GPIO_VBUS_POGO_5V	EXYNOS5_GPX1(2)
#define GPIO_OTG_VBUS_SENSE_FAC	EXYNOS5_GPB0(1)
#define GPIO_1WIRE_SLEEP	EXYNOS5_GPG0(0)

enum charge_connector {
	CHARGE_CONNECTOR_NONE,
	CHARGE_CONNECTOR_POGO,
	CHARGE_CONNECTOR_USB,
	CHARGE_CONNECTOR_MAX,
};

static int manta_bat_last_smb347_status;
static enum manta_charge_source manta_bat_charge_source[CHARGE_CONNECTOR_MAX];
static enum charge_connector manta_bat_charge_conn;
static union power_supply_propval manta_bat_apsd_results;
static int manta_bat_ta_adc;
static bool manta_bat_dock;
static bool manta_bat_usb_online;
static bool manta_bat_pogo_online;
static bool manta_bat_otg_enabled;
static bool manta_bat_chg_enabled;
static bool manta_bat_chg_enable_synced;
static struct power_supply *manta_bat_smb347_mains;
static struct power_supply *manta_bat_smb347_usb;
static struct power_supply *manta_bat_smb347_battery;
static struct power_supply *manta_bat_ds2784_battery;

static struct s3c_adc_client *ta_adc_client;

static bool manta_bat_suspended;

static struct delayed_work redetect_work;
static struct wake_lock manta_bat_redetect_wl;

static struct wake_lock manta_bat_chgdetect_wakelock;

static DEFINE_MUTEX(manta_bat_charger_detect_lock);
static DEFINE_MUTEX(manta_bat_adc_lock);

static void manta_bat_send_uevent(void);

static char *manta_charge_source_str(enum manta_charge_source charge_source)
{
	switch (charge_source) {
	case MANTA_CHARGE_SOURCE_NONE:
		return "none";
	case MANTA_CHARGE_SOURCE_AC_SAMSUNG:
		return "ac-samsung";
	case MANTA_CHARGE_SOURCE_AC_OTHER:
		return "ac-other";
	case MANTA_CHARGE_SOURCE_USB:
		return "usb";
	case MANTA_CHARGE_SOURCE_UNKNOWN:
		return "unknown";
	default:
		break;
	}

	return "?";
}

static inline int manta_bat_get_ds2784(void)
{
	if (!manta_bat_ds2784_battery)
		manta_bat_ds2784_battery =
			power_supply_get_by_name("ds2784-fuelgauge");

	if (!manta_bat_ds2784_battery) {
		pr_err_once("%s: failed to get ds2784-fuelgauge power supply\n",
			    __func__);
		return -ENODEV;
	}

	return 0;
}

static inline int manta_bat_get_smb347_usb(void)
{
	if (!manta_bat_smb347_usb)
		manta_bat_smb347_usb =
			power_supply_get_by_name("smb347-usb");

	if (!manta_bat_smb347_usb) {
		pr_err("%s: failed to get smb347-usb power supply\n",
		       __func__);
		return -ENODEV;
	}

	return 0;
}

static void charger_gpio_init(void)
{
	int ret;

	s3c_gpio_cfgpin(GPIO_TA_INT, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TA_INT, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_OTG_VBUS_SENSE, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_OTG_VBUS_SENSE, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_VBUS_POGO_5V, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_VBUS_POGO_5V, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_OTG_VBUS_SENSE_FAC, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_OTG_VBUS_SENSE_FAC, S3C_GPIO_PULL_NONE);
	s5p_gpio_set_pd_cfg(GPIO_OTG_VBUS_SENSE_FAC, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(GPIO_OTG_VBUS_SENSE_FAC,
			     S5P_GPIO_PD_UPDOWN_DISABLE);

	s3c_gpio_cfgpin(GPIO_TA_NCHG, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TA_NCHG, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_TA_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TA_EN, S3C_GPIO_PULL_NONE);
	s5p_gpio_set_pd_cfg(GPIO_TA_EN, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(GPIO_TA_EN, S5P_GPIO_PD_UPDOWN_DISABLE);

	s3c_gpio_cfgpin(GPIO_USB_SEL1, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_USB_SEL1, S3C_GPIO_PULL_NONE);
	s5p_gpio_set_pd_pull(GPIO_USB_SEL1, S5P_GPIO_PD_UPDOWN_DISABLE);
	ret = gpio_request_one(GPIO_USB_SEL1, GPIOF_OUT_INIT_HIGH, "usb_sel1");
	if (ret)
		pr_err("%s: cannot request gpio%d\n", __func__, GPIO_USB_SEL1);
}

static int read_ta_adc(enum charge_connector conn, int ta_check_sel)
{
	int adc_max = -1;
	int adc_min = 1 << 11;
	int adc_total = 0;
	int i, j;
	int ret;

	mutex_lock(&manta_bat_adc_lock);

	/* switch to check adc */
	if (conn == CHARGE_CONNECTOR_USB)
		gpio_set_value(GPIO_USB_SEL1, 0);
	else {
		ret = manta_pogo_charge_detect_start(ta_check_sel);
		if (ret < 0) {
			pr_err("%s: Failed to start pogo charger detection\n",
				__func__);
			goto fail_gpio;
		}
	}

	msleep(100);

	for (i = 0; i < ADC_NUM_SAMPLES; i++) {
		ret = s3c_adc_read(ta_adc_client, 0);

		if (ret == -ETIMEDOUT) {
			for (j = 0; j < ADC_LIMIT_ERR_COUNT; j++) {
				msleep(20);
				ret = s3c_adc_read(ta_adc_client, 0);
				if (ret > 0)
					break;
			}
			if (j >= ADC_LIMIT_ERR_COUNT) {
				pr_err("%s: Retry count exceeded\n", __func__);
				goto out;
			}
		} else if (ret < 0) {
			pr_err("%s: Failed read adc value : %d\n",
				__func__, ret);
			goto out;
		}

		if (ret > adc_max)
			adc_max = ret;
		if (ret < adc_min)
			adc_min = ret;

		adc_total += ret;
	}

	ret = (adc_total - adc_max - adc_min) / (ADC_NUM_SAMPLES - 2);

out:
	msleep(50);

	/* switch back to normal */
	if (conn == CHARGE_CONNECTOR_USB)
		gpio_set_value(GPIO_USB_SEL1, 1);
	else
		manta_pogo_charge_detect_end();

fail_gpio:
	mutex_unlock(&manta_bat_adc_lock);
	return ret;
}

int manta_bat_otg_enable(bool enable)
{
	int ret;
	union power_supply_propval value;

	if (manta_bat_get_smb347_usb())
		return -ENODEV;

	mutex_lock(&manta_bat_charger_detect_lock);

	if (manta_bat_otg_enabled == enable) {
		mutex_unlock(&manta_bat_charger_detect_lock);
		return 0;
	}

	value.intval = enable ? 1 : 0;
	ret = manta_bat_smb347_usb->set_property(manta_bat_smb347_usb,
						 POWER_SUPPLY_PROP_USB_OTG,
						 &value);
	if (ret)
		pr_err("%s: failed to set smb347-usb OTG mode\n",
		       __func__);
	else
		manta_bat_otg_enabled = enable;

	mutex_unlock(&manta_bat_charger_detect_lock);
	return ret;
}

static enum manta_charge_source check_samsung_charger(
	enum charge_connector conn, bool usbin_redetect)
{
	int ret;
	bool samsung_ac_detect;
	enum manta_charge_source charge_source;
	union power_supply_propval prop_zero = {0,};
	union power_supply_propval prop_one = {1,};

	if (conn == CHARGE_CONNECTOR_POGO) {
		manta_bat_ta_adc = read_ta_adc(conn, 0);
		pr_debug("%s: ta_adc conn=%d ta_check=0 val=%d\n", __func__,
			 conn, manta_bat_ta_adc);
		samsung_ac_detect = manta_bat_ta_adc > TA_ADC_LOW &&
			manta_bat_ta_adc < TA_ADC_HIGH;
		charge_source = samsung_ac_detect ?
			MANTA_CHARGE_SOURCE_AC_OTHER :
			MANTA_CHARGE_SOURCE_USB;
	} else {
		if (manta_bat_get_smb347_usb())
			return MANTA_CHARGE_SOURCE_UNKNOWN;
		ret = manta_bat_smb347_usb->set_property(
			manta_bat_smb347_usb,
			POWER_SUPPLY_PROP_CHARGER_DETECTION,
			&prop_one);
		if (ret)
			pr_err("%s: failed to enable smb347-usb charger detect\n",
			       __func__);
		ret = manta_bat_smb347_usb->get_property(
			manta_bat_smb347_usb,
			POWER_SUPPLY_PROP_REMOTE_TYPE, &manta_bat_apsd_results);
		pr_debug("%s: type=%d ret=%d\n", __func__,
			 manta_bat_apsd_results.intval, ret);
		samsung_ac_detect =
			manta_bat_apsd_results.intval ==
			POWER_SUPPLY_TYPE_USB_DCP;

		switch (manta_bat_apsd_results.intval) {
		case POWER_SUPPLY_TYPE_USB:
			charge_source = MANTA_CHARGE_SOURCE_USB;
			break;
		case POWER_SUPPLY_TYPE_UNKNOWN:
			charge_source = MANTA_CHARGE_SOURCE_UNKNOWN;
			break;
		default:
			charge_source = MANTA_CHARGE_SOURCE_AC_OTHER;
			break;
		}

		/*
		 * Leave APSD on if unknown power source (possibly timeout)
		 * and not re-detecting USBIN.  If redetecting USBIN and
		 * power source is still unknown then disable APSD and assume
		 * a slow charger.
		 */

		if (usbin_redetect ||
		    manta_bat_apsd_results.intval != POWER_SUPPLY_TYPE_UNKNOWN)
			ret = manta_bat_smb347_usb->set_property(
				manta_bat_smb347_usb,
				POWER_SUPPLY_PROP_CHARGER_DETECTION,
				&prop_zero);
	}

	if (samsung_ac_detect) {
		bool samsung_ta_detected;

		manta_bat_ta_adc = read_ta_adc(conn, 1);
		pr_debug("%s: ta_adc conn=%d ta_check=1 val=%d\n",
			 __func__, conn, manta_bat_ta_adc);
		samsung_ta_detected =
			manta_bat_ta_adc > TA_ADC_LOW &&
			manta_bat_ta_adc < TA_ADC_HIGH;

		if (samsung_ta_detected)
			charge_source = MANTA_CHARGE_SOURCE_AC_SAMSUNG;
	}

	return charge_source;
}

static enum manta_charge_source
detect_charge_source(enum charge_connector conn, bool online,
		     bool force_dock_redetect, bool usbin_redetect)
{
	enum manta_charge_source charge_source;
	int ret;

	manta_bat_dock = false;

	if (!online) {
		if (conn == CHARGE_CONNECTOR_POGO)
			manta_pogo_set_vbus(online, NULL);
		return MANTA_CHARGE_SOURCE_NONE;
	}

	charge_source = force_dock_redetect ?
		MANTA_CHARGE_SOURCE_USB :
		check_samsung_charger(conn, usbin_redetect);

	if (conn == CHARGE_CONNECTOR_POGO &&
	    charge_source == MANTA_CHARGE_SOURCE_USB) {
		ret = manta_pogo_set_vbus(online, &charge_source);
		manta_bat_dock = ret >= 0;
	}

	return charge_source;
}

static int update_charging_status(bool usb_connected, bool pogo_connected,
				  bool force_dock_redetect,
				  bool usbin_redetect)
{
	enum charge_connector old_charge_conn;
	int ret = 0;

	if (manta_bat_usb_online != usb_connected || usbin_redetect) {
		bool usb_conn_src_usb;

		manta_bat_usb_online = usb_connected;

		if (usbin_redetect)
			manta_otg_set_usb_state(false);

		manta_bat_charge_source[CHARGE_CONNECTOR_USB] =
			detect_charge_source(CHARGE_CONNECTOR_USB,
					     usb_connected, false,
					     usbin_redetect);
		usb_conn_src_usb =
			manta_bat_charge_source[CHARGE_CONNECTOR_USB] ==
			MANTA_CHARGE_SOURCE_USB;

		/*
		 * If USB disconnected, cancel any pending USB charger
		 * redetect.
		 */

		if (!usb_conn_src_usb) {
			ret = cancel_delayed_work(&redetect_work);
			if (ret)
				wake_unlock(&manta_bat_redetect_wl);
		}

		manta_otg_set_usb_state(usb_conn_src_usb);

		if (!usbin_redetect &&
		    (usb_conn_src_usb ||
		     manta_bat_charge_source[CHARGE_CONNECTOR_USB] ==
		     MANTA_CHARGE_SOURCE_UNKNOWN)) {
			cancel_delayed_work(&redetect_work);
			wake_lock(&manta_bat_redetect_wl);
			schedule_delayed_work(&redetect_work,
					      msecs_to_jiffies(5000));
		}

		ret = 1;
	}

	if (manta_bat_pogo_online != pogo_connected || force_dock_redetect) {
		manta_bat_pogo_online = pogo_connected;
		manta_bat_charge_source[CHARGE_CONNECTOR_POGO] =
			detect_charge_source(CHARGE_CONNECTOR_POGO,
					     pogo_connected,
					     force_dock_redetect, false);
		ret = 1;
	}

	old_charge_conn = manta_bat_charge_conn;

	if (manta_bat_charge_source[CHARGE_CONNECTOR_POGO] ==
	    MANTA_CHARGE_SOURCE_NONE &&
	    manta_bat_charge_source[CHARGE_CONNECTOR_USB] ==
	    MANTA_CHARGE_SOURCE_NONE)
		manta_bat_charge_conn = CHARGE_CONNECTOR_NONE;
	else
		manta_bat_charge_conn =
			(manta_bat_charge_source[CHARGE_CONNECTOR_POGO] >=
			 manta_bat_charge_source[CHARGE_CONNECTOR_USB]) ?
			CHARGE_CONNECTOR_POGO : CHARGE_CONNECTOR_USB;

	if (old_charge_conn != manta_bat_charge_conn)
		ret = 1;

	return ret;
}

static void manta_bat_set_charging_enable(int en)
{
	union power_supply_propval value;

	manta_bat_chg_enabled = en;

	if (!manta_bat_smb347_battery)
		manta_bat_smb347_battery =
			power_supply_get_by_name("smb347-battery");

	if (!manta_bat_smb347_battery)
		return;

	value.intval = en ? 1 : 0;
	manta_bat_smb347_battery->set_property(
		manta_bat_smb347_battery, POWER_SUPPLY_PROP_CHARGE_ENABLED,
		&value);
	manta_bat_chg_enable_synced = true;
}

static void manta_bat_sync_charge_enable(void)
{
	union power_supply_propval chg_enabled = {0,};

	if (!manta_bat_smb347_battery)
		return;

	manta_bat_smb347_battery->get_property(
		manta_bat_smb347_battery,
		POWER_SUPPLY_PROP_CHARGE_ENABLED,
		&chg_enabled);

	if (chg_enabled.intval != manta_bat_chg_enabled) {
		if (manta_bat_chg_enable_synced) {
			pr_info("%s: charger changed enable state to %d\n",
				__func__, chg_enabled.intval);
			manta_bat_chg_enabled = chg_enabled.intval;
		}

		manta_bat_set_charging_enable(manta_bat_chg_enabled);
	}
}

static void exynos5_manta_set_priority(void)
{
	int ret;
	union power_supply_propval value;

	if (!manta_bat_smb347_battery)
		manta_bat_smb347_battery =
			power_supply_get_by_name("smb347-battery");

	if (!manta_bat_smb347_battery) {
		pr_err("%s: failed to get smb347-battery power supply\n",
		       __func__);
		return;
	}

	/* set SMB347 INPUT SOURCE PRIORITY = DCIN or USBIN */
	value.intval = manta_bat_charge_conn == CHARGE_CONNECTOR_USB;
	ret = manta_bat_smb347_battery->set_property(
		manta_bat_smb347_battery, POWER_SUPPLY_PROP_USB_INPRIORITY,
		&value);
	if (ret)
		pr_err("%s: failed to set smb347 input source priority: %d\n",
		       __func__, ret);

	/* enable SMB347 AICL for other TA */
	value.intval =
		manta_bat_charge_source[manta_bat_charge_conn] ==
		MANTA_CHARGE_SOURCE_AC_OTHER;
	ret = manta_bat_smb347_battery->set_property(
		manta_bat_smb347_battery, POWER_SUPPLY_PROP_AUTO_CURRENT_LIMIT,
		&value);
	if (ret)
		pr_err("%s: failed to set smb347 AICL: %d\n",
		       __func__, ret);
}

static void manta_bat_charge_source_changed(
	enum manta_charge_source charge_source)
{
	pr_info("battery: charge source type was changed: %s\n",
		manta_charge_source_str(charge_source));

	wake_lock_timeout(&manta_bat_chgdetect_wakelock, HZ * 2);
	manta_bat_send_uevent();
}

static void change_charger_status(bool force_dock_redetect,
				  bool usbin_redetect)
{
	int ta_int;
	union power_supply_propval pogo_connected = {0,};
	union power_supply_propval usb_connected = {0,};
	union power_supply_propval smb347_status = {0,};
	int status_change = 0;
	int ret;

	mutex_lock(&manta_bat_charger_detect_lock);
	ta_int = gpio_get_value(GPIO_OTG_VBUS_SENSE) |
		gpio_get_value(GPIO_VBUS_POGO_5V);

	if (!manta_bat_smb347_mains || !manta_bat_smb347_usb ||
	    !manta_bat_smb347_battery) {
		manta_bat_smb347_mains =
			power_supply_get_by_name("smb347-mains");
		manta_bat_get_smb347_usb();
		manta_bat_smb347_battery =
			power_supply_get_by_name("smb347-battery");

		if (!manta_bat_smb347_mains || !manta_bat_smb347_usb ||
		    !manta_bat_smb347_battery)
			pr_err("%s: failed to get power supplies\n", __func__);
	}

	if (!manta_bat_otg_enabled)
		usb_connected.intval = gpio_get_value(GPIO_OTG_VBUS_SENSE);

	pogo_connected.intval = gpio_get_value(GPIO_VBUS_POGO_5V);

	if (manta_bat_smb347_usb &&
	    usb_connected.intval != manta_bat_usb_online) {
		ret = manta_bat_smb347_usb->set_property(
			manta_bat_smb347_usb, POWER_SUPPLY_PROP_ONLINE,
			&usb_connected);
		if (ret)
			pr_err("%s: failed to change smb347-usb online\n",
			       __func__);
	}

	if (manta_bat_smb347_mains &&
	    pogo_connected.intval != manta_bat_pogo_online) {
		ret = manta_bat_smb347_mains->set_property(
			manta_bat_smb347_mains, POWER_SUPPLY_PROP_ONLINE,
			&pogo_connected);
		if (ret)
			pr_err("%s: failed to change smb347-mains online\n",
			       __func__);
	}

	if (manta_bat_smb347_battery) {
		manta_bat_smb347_battery->get_property(
			manta_bat_smb347_battery, POWER_SUPPLY_PROP_STATUS,
			&smb347_status);

		if (smb347_status.intval != manta_bat_last_smb347_status) {
			if (smb347_status.intval == POWER_SUPPLY_STATUS_FULL)
				manta_bat_send_uevent();

			manta_bat_last_smb347_status = smb347_status.intval;
		}
	}

	if (ta_int) {
		status_change = update_charging_status(
			usb_connected.intval, pogo_connected.intval,
			force_dock_redetect, usbin_redetect);
		manta_bat_sync_charge_enable();
	} else {
		status_change = update_charging_status(false, false, false,
						       false);
	}

	pr_debug("%s: ta_int(%d), charge_conn(%d), charge_source(%d)\n",
		 __func__, ta_int, manta_bat_charge_conn,
		 manta_bat_charge_source[manta_bat_charge_conn]);

	if (status_change)
		exynos5_manta_set_priority();

	if (status_change)
		manta_bat_charge_source_changed(
			manta_bat_charge_source[manta_bat_charge_conn]);
	else
		wake_unlock(&manta_bat_chgdetect_wakelock);

	mutex_unlock(&manta_bat_charger_detect_lock);
}

void manta_force_update_pogo_charger(void)
{
	change_charger_status(true, false);
}

static char *manta_battery_supplicant[] = { "manta-battery" };

static struct smb347_charger_platform_data smb347_chg_pdata = {
	.use_mains = true,
	.use_usb = true,
	.enable_control = SMB347_CHG_ENABLE_PIN_ACTIVE_LOW,
	.usb_mode_pin_ctrl = false,
	.max_charge_current = 2500000,
	.max_charge_voltage = 4300000,
	.disable_automatic_recharge = true,
	.pre_charge_current = 200000,
	.termination_current = 250000,
	.pre_to_fast_voltage = 2600000,
	.mains_current_limit = 2000000,
	.usb_hc_current_limit = 1800000,
	.irq_gpio = GPIO_TA_NCHG,
	.disable_stat_interrupts = true,
	.en_gpio = GPIO_TA_EN,
	.supplied_to = manta_battery_supplicant,
	.num_supplicants = ARRAY_SIZE(manta_battery_supplicant),
};

static void exynos5_manta_set_mains_current(void)
{
	int ret;
	union power_supply_propval value;

	if (!manta_bat_smb347_mains)
		manta_bat_smb347_mains =
			power_supply_get_by_name("smb347-mains");

	if (!manta_bat_smb347_mains) {
		pr_err("%s: failed to get smb347-mains power supply\n",
		       __func__);
		return;
	}

	value.intval =
		manta_bat_charge_source[CHARGE_CONNECTOR_POGO] ==
		   MANTA_CHARGE_SOURCE_USB ? 500000 : 2000000;

	ret = manta_bat_smb347_mains->set_property(manta_bat_smb347_mains,
					 POWER_SUPPLY_PROP_CURRENT_MAX,
					 &value);
	if (ret)
		pr_err("%s: failed to set smb347-mains current limit\n",
		       __func__);
}

static void exynos5_manta_set_usb_hc(void)
{
	int ret;
	union power_supply_propval value;

	if (manta_bat_get_smb347_usb())
		return;

	value.intval =
		manta_bat_charge_source[CHARGE_CONNECTOR_USB] ==
		MANTA_CHARGE_SOURCE_USB ? 0 : 1;
	ret = manta_bat_smb347_usb->set_property(manta_bat_smb347_usb,
						 POWER_SUPPLY_PROP_USB_HC,
						 &value);
	if (ret)
		pr_err("%s: failed to set smb347-usb USB/HC mode\n",
		       __func__);
}

static void manta_bat_set_charging_current(void)
{
	exynos5_manta_set_priority();
	exynos5_manta_set_usb_hc();
	exynos5_manta_set_mains_current();
}

static irqreturn_t ta_int_intr(int irq, void *arg)
{
	wake_lock(&manta_bat_chgdetect_wakelock);
	msleep(600);
	change_charger_status(false, false);
	return IRQ_HANDLED;
}

static int manta_power_debug_dump(struct seq_file *s, void *unused)
{
	seq_printf(s, "ta_en=%d ta_nchg=%d ta_int=%d usbin=%d, dcin=%d st=%d\n",
		   gpio_get_value(GPIO_TA_EN),
		   gpio_get_value(GPIO_TA_NCHG),
		   gpio_get_value(GPIO_TA_INT),
		   gpio_get_value(GPIO_OTG_VBUS_SENSE),
		   gpio_get_value(GPIO_VBUS_POGO_5V),
		   manta_bat_last_smb347_status);
	seq_printf(s, "%susb: type=%s (apsd=%d); %spogo: type=%s%s; ta_adc=%d\n",
		   manta_bat_charge_conn == CHARGE_CONNECTOR_USB ? "*" : "",
		   manta_bat_otg_enabled ? "otg" :
		   manta_charge_source_str(
			   manta_bat_charge_source[CHARGE_CONNECTOR_USB]),
		   manta_bat_apsd_results.intval,
		   manta_bat_charge_conn == CHARGE_CONNECTOR_POGO ? "*" : "",
		   manta_charge_source_str(
			   manta_bat_charge_source[CHARGE_CONNECTOR_POGO]),
		   manta_bat_dock ? "(d)" : "", manta_bat_ta_adc);
	return 0;
}

static int manta_power_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, manta_power_debug_dump, inode->i_private);
}

static const struct file_operations manta_power_debug_fops = {
	.open = manta_power_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int manta_power_adc_debug_dump(struct seq_file *s, void *unused)
{
	seq_printf(s, "usb=%d,%d pogo=%d,%d\n",
		   read_ta_adc(CHARGE_CONNECTOR_USB, 0),
		   read_ta_adc(CHARGE_CONNECTOR_USB, 1),
		   read_ta_adc(CHARGE_CONNECTOR_POGO, 0),
		   read_ta_adc(CHARGE_CONNECTOR_POGO, 1));
	return 0;
}

static int manta_power_adc_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, manta_power_adc_debug_dump, inode->i_private);
}

static const struct file_operations manta_power_adc_debug_fops = {
	.open = manta_power_adc_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct ds2482_platform_data ds2483_pdata = {
	.slpz_gpio = -1,
};

static struct i2c_board_info i2c_devs2[] __initdata = {
	{
		I2C_BOARD_INFO("ds2482", 0x30 >> 1),
		.platform_data = &ds2483_pdata,
	},
	{
		I2C_BOARD_INFO("smb347", 0x0c >> 1),
		.platform_data  = &smb347_chg_pdata,
	},
};

static void redetect_work_proc(struct work_struct *work)
{
	change_charger_status(false, true);
	wake_unlock(&manta_bat_redetect_wl);
}

/*
 * manta-battery power_supply...
 */

static int manta_bat_enable_charging(bool enable)
{
	if (enable)
		manta_bat_set_charging_current();

	manta_bat_set_charging_enable(enable);
	return 0;
}

static int manta_bat_set_property(struct power_supply *ps,
				  enum power_supply_property prop,
				  const union power_supply_propval *val)
{
	int ret;

	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		ret = manta_bat_enable_charging(val->intval);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int manta_bat_get_property(struct power_supply *ps,
				  enum power_supply_property prop,
				  union power_supply_propval *val)
{
	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		val->intval = manta_bat_chg_enabled;
		return 0;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (manta_bat_get_ds2784())
			return -EIO;
		return manta_bat_ds2784_battery->get_property(
			manta_bat_ds2784_battery, POWER_SUPPLY_PROP_CAPACITY,
			val);

	default:
		return -EINVAL;
	}
};

static int manta_bat_property_is_writeable(struct power_supply *psy,
					   enum power_supply_property prop)
{
	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		return 1;
	default:
		break;
	}

	return 0;
}
static enum power_supply_property manta_battery_props[] = {
	POWER_SUPPLY_PROP_CHARGE_ENABLED,
	POWER_SUPPLY_PROP_CAPACITY,
};

static void manta_bat_power_changed(struct power_supply *psy)
{
	change_charger_status(false, false);
}

static struct power_supply manta_battery_psy = {
	.name = "manta-battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = manta_battery_props,
	.num_properties = ARRAY_SIZE(manta_battery_props),
	.external_power_changed = manta_bat_power_changed,
	.get_property = manta_bat_get_property,
	.set_property = manta_bat_set_property,
	.property_is_writeable = manta_bat_property_is_writeable,
};

static void manta_bat_send_uevent(void)
{
	power_supply_changed(&manta_battery_psy);
}

static struct platform_device manta_bat_s3c_client_dev = {
	.name = "manta-battery",
	.id = -1,
};

void __init exynos5_manta_battery_init(void)
{
	int hw_rev = exynos5_manta_get_revision();

	charger_gpio_init();
	INIT_DELAYED_WORK(&redetect_work, redetect_work_proc);
	wake_lock_init(&manta_bat_chgdetect_wakelock, WAKE_LOCK_SUSPEND,
		       "manta-chgdetect");
	wake_lock_init(&manta_bat_redetect_wl, WAKE_LOCK_SUSPEND,
		       "manta-chgredetect");

	if (hw_rev >= MANTA_REV_DOGFOOD02) {
		s3c_gpio_cfgpin(GPIO_1WIRE_SLEEP, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(GPIO_1WIRE_SLEEP, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_pd_cfg(GPIO_1WIRE_SLEEP, S5P_GPIO_PD_INPUT);
		s5p_gpio_set_pd_pull(GPIO_1WIRE_SLEEP,
				     S5P_GPIO_PD_UPDOWN_DISABLE);
		ds2483_pdata.slpz_gpio = GPIO_1WIRE_SLEEP;
	}

	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));

	ta_adc_client =
		s3c_adc_register(&manta_bat_s3c_client_dev, NULL, NULL, 0);

	if (IS_ERR_OR_NULL(debugfs_create_file("manta-power", S_IRUGO, NULL,
					       NULL, &manta_power_debug_fops)))
		pr_err("failed to create manta-power debugfs entry\n");

	if (IS_ERR_OR_NULL(debugfs_create_file("manta-power-adc", S_IRUGO, NULL,
					       NULL,
					       &manta_power_adc_debug_fops)))
		pr_err("failed to create manta-power-adc debugfs entry\n");
}

static int exynos5_manta_battery_pm_event(struct notifier_block *notifier,
					  unsigned long pm_event,
					  void *unused)
{
	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
		disable_irq(gpio_to_irq(GPIO_OTG_VBUS_SENSE));
		disable_irq(gpio_to_irq(GPIO_VBUS_POGO_5V));
		manta_bat_suspended = true;
		break;

	case PM_POST_SUSPEND:
		if (manta_bat_suspended) {
			enable_irq(gpio_to_irq(GPIO_OTG_VBUS_SENSE));
			enable_irq(gpio_to_irq(GPIO_VBUS_POGO_5V));
			manta_bat_suspended = false;
		}
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

static int manta_bat_usb_event(struct notifier_block *nb,
			       unsigned long event, void *unused)
{
	int ret;

	if (event == USB_EVENT_ENUMERATED) {
		ret = __cancel_delayed_work(&redetect_work);
		if (ret)
			wake_unlock(&manta_bat_redetect_wl);
	}

	return NOTIFY_OK;
}

static struct notifier_block exynos5_manta_battery_pm_notifier_block = {
	.notifier_call = exynos5_manta_battery_pm_event,
};

static struct notifier_block manta_bat_usb_nb = {
	.notifier_call = manta_bat_usb_event,
};

static int __init exynos5_manta_battery_late_init(void)
{
	int ret;
	struct usb_phy *usb_xceiv;

	ret = power_supply_register(NULL, &manta_battery_psy);
	if (ret)
		pr_err("%s: failed to register power_supply\n",
		       __func__);

	ret = request_threaded_irq(gpio_to_irq(GPIO_OTG_VBUS_SENSE),
				   NULL, ta_int_intr,
				   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				   IRQF_ONESHOT, "usb_vbus", NULL);
	if (ret) {
		pr_err("%s: usb_vbus irq register failed, ret=%d\n",
		       __func__, ret);
	} else {
		ret = enable_irq_wake(gpio_to_irq(GPIO_OTG_VBUS_SENSE));
		if (ret)
			pr_warn("%s: failed to enable irq_wake for usb_vbus\n",
				__func__);
	}

	ret = request_threaded_irq(gpio_to_irq(GPIO_VBUS_POGO_5V), NULL,
				   ta_int_intr,
				   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				   IRQF_ONESHOT, "pogo_vbus", NULL);
	if (ret) {
		pr_err("%s: pogo_vbus irq register failed, ret=%d\n",
		       __func__, ret);
	} else {
		ret = enable_irq_wake(gpio_to_irq(GPIO_VBUS_POGO_5V));
		if (ret)
			pr_warn("%s: failed to enable irq_wake for pogo_vbus\n",
				__func__);
	}

	ret = register_pm_notifier(&exynos5_manta_battery_pm_notifier_block);
	if (ret)
		pr_warn("%s: failed to register PM notifier; ret=%d\n",
			__func__, ret);

	usb_xceiv = usb_get_transceiver();

	if (!usb_xceiv) {
		pr_err("%s: No USB transceiver found\n", __func__);
	} else {
		ret = usb_register_notifier(usb_xceiv, &manta_bat_usb_nb);

		if (ret) {
			pr_err("%s: usb_register_notifier on transceiver %s failed\n",
			       __func__, dev_name(usb_xceiv->dev));
		}
	}

	/* get initial charger status */
	change_charger_status(false, false);
	return 0;
}

late_initcall(exynos5_manta_battery_late_init);
