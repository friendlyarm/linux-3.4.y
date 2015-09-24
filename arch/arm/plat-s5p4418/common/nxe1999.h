/*
 * nxe2000.h - Voltage regulator driver for the Nexell NXE2000
 *
 *  Copyright (C) 2013 Nexell
 *  bong kwan kook <kook@nexell.co.kr>
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

#ifndef __LINUX_MFD_NXE2000_H
#define __LINUX_MFD_NXE2000_H

#include <linux/regulator/machine.h>

#define NXE2000_NUM_BUCK	5
#define NXE2000_NUM_LDO		10
#define NXE2000_NUM_PSO		5

/* Nexell 2000 regulator ids */
enum {
	NXE2000_LDO1 = 0,
	NXE2000_LDO2,
	NXE2000_LDO3,
	NXE2000_LDO4,
	NXE2000_LDO5,
    NXE2000_LDO6,
    NXE2000_LDO7,
    NXE2000_LDO8,
    NXE2000_LDO9,
    NXE2000_LDO10,
    NXE2000_LDORTC1,
    NXE2000_LDORTC2,
	NXE2000_BUCK1,			/* DCDC1 */
	NXE2000_BUCK2,			/* DCDC2 */
	NXE2000_BUCK3,			/* DCDC3 */
    NXE2000_BUCK4,          /* DCDC4 */
    NXE2000_BUCK5,          /* DCDC5 */
#if 0
//	NXE2000_CHARGER_CV,		/* control MBCCV of MBCCTRL3 */
	NXE2000_CHARGER,		/* charger current, MBCCTRL4 */
	NXE2000_CHARGER_TOPOFF,	/* MBCCTRL5 */
#endif

	NXE2000_REG_MAX,
};

/**
 * nxe2000_regulator_data - regulator data
 * @id: regulator id
 * @initdata: regulator init data (contraints, supplies, ...)
 */
struct nxe2000_regulator_data {
	int				id;
	struct regulator_init_data	*initdata;
};

/**
 * struct nxe2000_board - packages regulator init data
 * @regulators: array of defined regulators
 * @num_regulators: number of regultors used
 * @irq_base: base IRQ number for nxe2000, required for IRQs
 * @ono: power onoff IRQ number for nxe2000
 * @buck_voltage_lock: Do NOT change the values of the following six
 *   registers set by buck?_voltage?. The voltage of BUCK1/2 cannot
 *   be other than the preset values.
 * @buck1_voltage1: BUCK1 DVS mode 1 voltage register
 * @buck1_voltage2: BUCK1 DVS mode 2 voltage register
 * @buck1_voltage3: BUCK1 DVS mode 3 voltage register
 * @buck1_voltage4: BUCK1 DVS mode 4 voltage register
 * @buck2_voltage1: BUCK2 DVS mode 1 voltage register
 * @buck2_voltage2: BUCK2 DVS mode 2 voltage register
 * @buck1_set1: BUCK1 gpio pin 1 to set output voltage
 * @buck1_set2: BUCK1 gpio pin 2 to set output voltage
 * @buck1_default_idx: Default for BUCK1 gpio pin 1, 2
 * @buck2_set3: BUCK2 gpio pin to set output voltage
 * @buck2_default_idx: Default for BUCK2 gpio pin.
 * @wakeup: Allow to wake up from suspend
 * @rtc_delay: LP3974 RTC chip bug that requires delay after a register
 * write before reading it.
 * @eoc: End of Charge Level in percent: 10% ~ 45% by 5% step
 *   If it equals 0, leave it unchanged.
 *   Otherwise, it is a invalid value.
 * @restart: Restart Level in mV: 100, 150, 200, and -1 for disable.
 *   If it equals 0, leave it unchanged.
 *   Otherwise, it is a invalid value.
 * @timeout: Full Timeout in hours: 5, 6, 7, and -1 for disable.
 *   If it equals 0, leave it unchanged.
 *   Otherwise, leave it unchanged.
 */

struct nxe2000_pdata {
	struct nxe2000_regulator_data	*regulators;

	int				num_regulators;

	int				gpio_eint;
	int				gpio_otg_usbid;
	int				gpio_otg_vbus;
	int				gpio_pmic_vbus;
	int				gpio_pmic_lowbat;

	int				bat_low_uV;
	int				bat_max_uV;
	int				bat_min_uV;

	int				irq_base;
	int				rdstate_periodic;
	int				ono;
	bool			have_battery;
	bool			buck_voltage_lock;

	uint8_t			slp_prio_buck[NXE2000_NUM_BUCK];   // 0 ~ 14, off => 15
	uint8_t			slp_prio_ldo[NXE2000_NUM_LDO];
	uint8_t			slp_prio_pso[NXE2000_NUM_PSO];    // power-supply shutoff 

	int				slp_buck_vol[NXE2000_NUM_BUCK];
	int				slp_ldo_vol[NXE2000_NUM_LDO];

	int				adp_ilim_current;
	int				adp_chg_current;

	int				usb_ilim_current;
	int				usb_chg_current;

	bool			wakeup;

	/* ---- Charger control ---- */
	/* eoc stands for 'end of charge' */
	int				eoc_mA; 	/* 50 ~ 200mA by 10mA step */
	int				restart;
	/* charge Full Timeout */
	int				timeout;	/* 0 (no timeout), 5, 6, 7 hours */
};

#endif /*  __LINUX_MFD_NXE2000_H */
