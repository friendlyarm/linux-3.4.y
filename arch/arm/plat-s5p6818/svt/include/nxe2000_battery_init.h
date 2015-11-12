/*
 * nxe2000_battery_init.h
 *
 *  Copyright (C) 2013 Nexell
 *  bong kwan kook <kook@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#ifndef __LINUX_POWER_NXE2000_BATTERY_INIT_H
#define __LINUX_POWER_NXE2000_BATTERY_INIT_H

/* Capacity             : 3000mAh */
/* Cut-off Voltage      : 3.45V */
/* Full Voltage         : 4.20V */
/* Thermistor B Value   : 3450 */
/* Board Impedance      : 0 ohm */

uint8_t battery_init_para[][32] = {
#if 0   // org
	[0]={
		0x08, 0xa3, 0x0a, 0xf9, 0x0b, 0x4b, 0x0b, 0x74,
		0x0b, 0x94, 0x0b, 0xb5, 0x0b, 0xe6, 0x0c, 0x30,
		0x0c, 0x92, 0x0c, 0xf4, 0x0d, 0x6f, 0x08, 0xca,
		0x00, 0x36, 0x0f, 0xc8, 0x05, 0x2c, 0x22, 0x56
	},
#endif
#if 0
	[0]={
		0x0B, 0x53, 0x0B, 0x96, 0x0B, 0xD3, 0x0B, 0xF5,
		0x0C, 0x14, 0x0C, 0x2E, 0x0C, 0x4B, 0x0C, 0x74,
		0x0C, 0xAC, 0x0C, 0xE5, 0x0D, 0x52, 0x09, 0xF7,
		0x00, 0x53, 0x0F, 0xCC, 0x04, 0xE6, 0x22, 0x56
#endif
#if 1	// 20140508
	[0]={
		0x0B, 0x68, 0x0B, 0xCF, 0x0B, 0xF8, 0x0C, 0x0E,
		0x0C, 0x1D, 0x0C, 0x35, 0x0C, 0x5B, 0x0C, 0x92,
		0x0C, 0xC7, 0x0D, 0x03, 0x0D, 0x55, 0x0B, 0x5D,
		0x00, 0x48, 0x0F, 0xC8, 0x05, 0x29, 0x22, 0x56
	},
#endif
};


#define	NXE2000_CUTOFF_VOL			(0)		/* mV "0" means cutoff voltage = original OCV table value */
#define NXE2000_REL1_SEL_VALUE		(64)	/* mV Range 0~240 * (Step 16) */
#define NXE2000_REL2_SEL_VALUE		(0)		/* mV Range 0~120 * (Step 8) */
#define NXE2000_TARGET_VSYS			(3300)	/* mV */

#endif  // __LINUX_POWER_NXE2000_BATTERY_INIT_H
