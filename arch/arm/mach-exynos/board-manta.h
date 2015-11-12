/*
 * Copyright (C) 2012 Google, Inc.
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
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

#ifndef __MACH_EXYNOS_BOARD_MANTA_H
#define __MACH_EXYNOS_BOARD_MANTA_H

#include <mach/irqs.h>
#include <linux/serial_core.h>

#define MANTA_REV_LUNCHBOX	0x1
#define MANTA_REV_PRE_ALPHA	0x2
#define MANTA_REV_ALPHA		0x3
#define MANTA_REV_BETA		0x4
#define MANTA_REV_DOGFOOD01	0x5
#define MANTA_REV_DOGFOOD02	0x6
#define MANTA_REV_DOGFOOD03	0x7
#define MANTA_REV_DOGFOOD04	0x8
#define MANTA_REV_DOGFOOD05	0x9

/* board IRQ allocations */
#define MANTA_IRQ_BOARD_PMIC_START	IRQ_BOARD_START
#define MANTA_IRQ_BOARD_PMIC_NR		16
#define MANTA_IRQ_BOARD_AUDIO_START	(IRQ_BOARD_START + \
					MANTA_IRQ_BOARD_PMIC_NR)
#define MANTA_IRQ_BOARD_AUDIO_NR	27

/* Manta-specific charger info */

enum manta_charge_source {
	MANTA_CHARGE_SOURCE_NONE,
	MANTA_CHARGE_SOURCE_UNKNOWN,
	MANTA_CHARGE_SOURCE_USB,
	MANTA_CHARGE_SOURCE_AC_OTHER,
	MANTA_CHARGE_SOURCE_AC_SAMSUNG,
};


void exynos5_manta_audio_init(void);
void exynos5_manta_display_init(void);
void exynos5_manta_input_init(void);
void exynos5_manta_power_init(void);
void exynos5_manta_battery_init(void);
void exynos5_manta_pogo_init(void);
void exynos5_manta_wlan_init(void);
void exynos5_manta_media_init(void);
void exynos5_manta_camera_init(void);
void exynos5_manta_sensors_init(void);
void exynos5_manta_gps_init(void);
void exynos5_manta_jack_init(void);
void exynos5_manta_vib_init(void);
void exynos5_manta_nfc_init(void);
void exynos5_manta_bt_init(void);
void exynos5_manta_connector_init(void);

int exynos5_manta_get_revision(void);
int manta_stmpe811_read_adc_data(u8 channel);
extern int manta_bat_otg_enable(bool enable);
void manta_otg_set_usb_state(bool connected);
int manta_pogo_set_vbus(bool status, enum manta_charge_source *charge_source);
extern int manta_pogo_charge_detect_start(bool spdif_mode_and_gpio_in);
extern void manta_pogo_charge_detect_end(void);
void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport);
void exynos5_manta_adjust_mif_asv_table(void);
void manta_force_update_pogo_charger(void);

#endif
