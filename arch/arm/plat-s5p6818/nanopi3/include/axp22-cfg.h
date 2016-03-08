/*
 * (C) Copyright 2015
 *  Jongshin Park, Nexell Co, <pjsin865@nexell.co.kr>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __AXP228_POWER_H_
#define __AXP228_POWER_H_

/*
 *	Config Value
 */

/*i2c channel */
#define	AXP_I2CBUS							3

/* interrupt */
#define AXP_IRQNO							CFG_GPIO_PMIC_INTR // 164


/*
 *	Default Value
 */

/* DCDC-LDO Name */
#define AXP_DCDC1_NAME						"vdd_sys_3.3V"		/* VCC3P3_SYS			*/
#define AXP_DCDC2_NAME						"vdd_arm_1.3V"		/* VCC1P1_ARM			*/
#define AXP_DCDC3_NAME						"vdd_core_1.2V"		/* VCC1P1_ARM(CORE)	*/
#define AXP_DCDC4_NAME						"vdd_sys_1.6V"		/* VCC1P5_SYS			*/
#define AXP_DCDC5_NAME						"vdd_ddr_1.6V"		/* VCC1P5_DDR			*/
#define AXP_ALDO1_NAME						"valive_3.3V"		/* VCC3P3_ALIVE		*/
#define AXP_ALDO2_NAME						"valive_1.8V"		/* VCC1P8_ALIVE		*/
#define AXP_ALDO3_NAME						"valive_1.0V"		/* VCC1P0_ALIVE		*/
#define AXP_DLDO1_NAME						"vwide_3.3V"		/* VCC_WIDE			*/
#define AXP_DLDO2_NAME						"vcam1_1.8V"		/* VCC1P8_CAM			*/
#define AXP_DLDO3_NAME						"vdumy1_0.7V"		/* NC					*/
#define AXP_DLDO4_NAME						"vdumy2_0.7V"		/* NC					*/
#define AXP_ELDO1_NAME						"vsys1_1.8V"		/* VCC1P8_SYS			*/
#define AXP_ELDO2_NAME						"vwifi_3.3V"		/* VCC3P3_WIFI			*/
#define AXP_ELDO3_NAME						"vdumy3_0.7V"		/* NC					*/
#define AXP_DC5LDO_NAME						"vcvbs_1.2V"		/* VCC1P2_CVBS		*/

/* DCDC-LDO Voltage Value */
#define AXP_DCDC1_VALUE						3300000		/* VCC3P3_SYS			DCDC1	 : 	AXP22:1600~3400, 100/setp*/
#define AXP_DCDC2_VALUE						1180000		/* VCC1P1_ARM			DCDC2	 : 	AXP22:  600~1540,   20/step*/
#define AXP_DCDC3_VALUE						1180000		/* VCC1P0_CORE		DCDC3	 : 	AXP22:  600~1860,   20/step*/
#define AXP_DCDC4_VALUE						1500000		/* VCC1P5_SYS			DCDC4	 : 	AXP22:  600~1540,   20/step*/
#define AXP_DCDC5_VALUE						1500000		/* VCC1P5_DDR			DCDC5	 : 	AXP22:1000~2550,   50/step*/
#define AXP_ALDO1_VALUE						3300000		/* VCC3P3_ALIVE		ALDO1	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ALDO2_VALUE						1800000		/* VCC1P8_ALIVE		ALDO2	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ALDO3_VALUE						1000000		/* VCC1P0_ALIVE		ALDO3	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DLDO1_VALUE						3300000		/* VCC_WIDE			DLDO1	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DLDO2_VALUE						1800000		/* VCC1P8_CAM			DLDO2	 : 	AXP22 : 700~3300, 100/step*/
#define AXP_DLDO3_VALUE						 700000		/* NC					DLDO3	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DLDO4_VALUE						 700000		/* NC					DLDO4	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ELDO1_VALUE						1800000		/* VCC1P8_SYS			ELDO1	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ELDO2_VALUE						3300000		/* VCC3P3_WIFI			ELDO2	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ELDO3_VALUE						 700000		/* NC					ELDO3	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DC5LDO_VALUE					1200000		/* VCC1P2_CVBS		DC5LDO	 : 	AXP22:  700~1400, 100/step*/

/* DCDC-LDO Voltage Enable Value */
#define AXP_DCDC1_ENABLE					1		/* VCC3P3_SYS			DCDC1	 : 	AXP22:1600~3400, 100/setp*/
#define AXP_DCDC2_ENABLE					1		/* VCC1P1_ARM			DCDC2	 : 	AXP22:  600~1540,   20/step*/
#define AXP_DCDC3_ENABLE					1		/* VCC1P0_CORE		DCDC3	 : 	AXP22:  600~1860,   20/step*/
#define AXP_DCDC4_ENABLE					1		/* VCC1P5_SYS			DCDC4	 : 	AXP22:  600~1540,   20/step*/
#define AXP_DCDC5_ENABLE					1		/* VCC1P5_DDR			DCDC5	 : 	AXP22:1000~2550,   50/step*/
#define AXP_ALDO1_ENABLE					1		/* VCC3P3_ALIVE		ALDO1	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ALDO2_ENABLE					1		/* VCC1P8_ALIVE		ALDO2	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ALDO3_ENABLE					1		/* VCC1P0_ALIVE		ALDO3	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DLDO1_ENABLE					1		/* VCC_WIDE			DLDO1	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DLDO2_ENABLE					0		/* VCC1P8_CAM			DLDO2	 : 	AXP22 : 700~3300, 100/step*/
#define AXP_DLDO3_ENABLE					0		/* NC					DLDO3	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DLDO4_ENABLE					0		/* NC					DLDO4	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ELDO1_ENABLE					1		/* VCC1P8_SYS			ELDO1	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ELDO2_ENABLE					1		/* VCC3P3_WIFI			ELDO2	 : 	AXP22:  700~3300, 100/step*/
#define AXP_ELDO3_ENABLE					0		/* NC					ELDO3	 : 	AXP22:  700~3300, 100/step*/
#define AXP_DC5LDO_ENABLE					0		/* VCC1P2_CVBS		DC5LDO	 : 	AXP22:  700~1400, 100/step*/
#define AXP_DC1SW_ENABLE					0		/* NC					DC1SW	 : 	AXP22: */

#endif	/* __AXP228_POWER_H_ */
