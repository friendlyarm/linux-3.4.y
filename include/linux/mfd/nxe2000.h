/*
 * include/linux/mfd/nxe2000.h
 *
 *  Copyright (C) 2013 Nexell
 *  bong kwan kook <kook@nexell.co.kr>
 *
 * Based on code
 *	Copyright (C) 2011 NVIDIA Corporation
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

#ifndef __LINUX_MFD_NXE2000_H
#define __LINUX_MFD_NXE2000_H

#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

/* Maximum number of main interrupts */
#define MAX_INTERRUPT_MASKS	13
#define MAX_MAIN_INTERRUPT	7
#define MAX_GPEDGE_REG		2

/* Power control register */
#define NXE2000_PWR_WD			0x0B
#define NXE2000_PWR_WD_COUNT		0x0C
#define NXE2000_PWR_FUNC		0x0D
#define NXE2000_PWR_SLP_CNT		0x0E
#define NXE2000_PWR_REP_CNT		0x0F
#define NXE2000_PWR_ON_TIMSET		0x10
#define NXE2000_PWR_NOE_TIMSET		0x11
#define NXE2000_PWR_IRSEL		0x15
#define NXE2000_PWR_DC4_SLOT	0x19

/* Interrupt enable register */
#define NXE2000_INT_EN_SYS		0x12
#define NXE2000_INT_EN_DCDC		0x40
#define NXE2000_INT_EN_RTC		0xAE
#define NXE2000_INT_EN_ADC1		0x88
#define NXE2000_INT_EN_ADC2		0x89
#define NXE2000_INT_EN_ADC3		0x8A
#define NXE2000_INT_EN_GPIO		0x94
#define NXE2000_INT_EN_GPIO2		0x94 /* dummy */
#define NXE2000_INT_MSK_CHGCTR		0xBE
#define NXE2000_INT_MSK_CHGSTS1	0xBF
#define NXE2000_INT_MSK_CHGSTS2	0xC0
#define NXE2000_INT_MSK_CHGERR		0xC1
#define NXE2000_INT_MSK_CHGEXTIF	0xD1

/* Interrupt select register */
#define NXE2000_PWR_IRSEL			0x15
#define NXE2000_CHG_CTRL_DETMOD1	0xCA
#define NXE2000_CHG_CTRL_DETMOD2	0xCB
#define NXE2000_CHG_STAT_DETMOD1	0xCC
#define NXE2000_CHG_STAT_DETMOD2	0xCD
#define NXE2000_CHG_STAT_DETMOD3	0xCE


/* interrupt status registers (monitor regs)*/
#define NXE2000_INTC_INTPOL		0x9C
#define NXE2000_INTC_INTEN		0x9D
#define NXE2000_INTC_INTMON		0x9E

#define NXE2000_INT_MON_SYS		0x14
#define NXE2000_INT_MON_DCDC		0x42
#define NXE2000_INT_MON_RTC		0xAF

#define NXE2000_INT_MON_CHGCTR		0xC6
#define NXE2000_INT_MON_CHGSTS1	0xC7
#define NXE2000_INT_MON_CHGSTS2	0xC8
#define NXE2000_INT_MON_CHGERR		0xC9
#define NXE2000_INT_MON_CHGEXTIF	0xD3

/* interrupt clearing registers */
#define NXE2000_INT_IR_SYS		0x13
#define NXE2000_INT_IR_DCDC		0x41
#define NXE2000_INT_IR_RTC		0xAF
#define NXE2000_INT_IR_ADCL		0x8C
#define NXE2000_INT_IR_ADCH		0x8D
#define NXE2000_INT_IR_ADCEND		0x8E
#define NXE2000_INT_IR_GPIOR		0x95
#define NXE2000_INT_IR_GPIOF		0x96
#define NXE2000_INT_IR_CHGCTR		0xC2
#define NXE2000_INT_IR_CHGSTS1		0xC3
#define NXE2000_INT_IR_CHGSTS2		0xC4
#define NXE2000_INT_IR_CHGERR		0xC5
#define NXE2000_INT_IR_CHGEXTIF	0xD2

/* GPIO register base address */
#define NXE2000_GPIO_IOSEL		0x90
#define NXE2000_GPIO_IOOUT		0x91
#define NXE2000_GPIO_GPEDGE1		0x92
#define NXE2000_GPIO_GPEDGE2		0x93
/* #define NXE2000_GPIO_EN_GPIR	0x94 */
/* #define NXE2000_GPIO_IR_GPR		0x95 */
/* #define NXE2000_GPIO_IR_GPF		0x96 */
#define NXE2000_GPIO_MON_IOIN		0x97
#define NXE2000_GPIO_LED_FUNC		0x98

#define NXE2000_REG_BANKSEL		0xFF

/* Charger Control register */
#define NXE2000_CHG_CTL1		0xB3
#define	TIMSET_REG			0xB9

/* ADC Control register */
#define NXE2000_ADC_CNT1		0x64
#define NXE2000_ADC_CNT2		0x65
#define NXE2000_ADC_CNT3		0x66
#define NXE2000_ADC_VBAT_THL		0x7A
#define NXE2000_ADC_VADP_THL		0x7C
#define NXE2000_ADC_VSYS_THL		0x80

#define	NXE2000_FG_CTRL		0xE0
#define	NXE2000_PSWR			0x07

/* NXE2000 IRQ definitions */
enum {
	NXE2000_IRQ_POWER_ON,
	NXE2000_IRQ_EXTIN,
	NXE2000_IRQ_PRE_VINDT,
	NXE2000_IRQ_PREOT,
	NXE2000_IRQ_POWER_OFF,
	NXE2000_IRQ_NOE_OFF,
	NXE2000_IRQ_WD,
	NXE2000_IRQ_CLK_STP,

	NXE2000_IRQ_DC1LIM,
	NXE2000_IRQ_DC2LIM,
	NXE2000_IRQ_DC3LIM,
	NXE2000_IRQ_DC4LIM,
	NXE2000_IRQ_DC5LIM,

	NXE2000_IRQ_ILIMLIR,
	NXE2000_IRQ_VBATLIR,
	NXE2000_IRQ_VADPLIR,
	NXE2000_IRQ_VUSBLIR,
	NXE2000_IRQ_VSYSLIR,
	NXE2000_IRQ_VTHMLIR,
	NXE2000_IRQ_AIN1LIR,
	NXE2000_IRQ_AIN0LIR,

	NXE2000_IRQ_ILIMHIR,
	NXE2000_IRQ_VBATHIR,
	NXE2000_IRQ_VADPHIR,
	NXE2000_IRQ_VUSBHIR,
	NXE2000_IRQ_VSYSHIR,
	NXE2000_IRQ_VTHMHIR,
	NXE2000_IRQ_AIN1HIR,
	NXE2000_IRQ_AIN0HIR,

	NXE2000_IRQ_ADC_ENDIR,

	NXE2000_IRQ_GPIO0,
	NXE2000_IRQ_GPIO1,
	NXE2000_IRQ_GPIO2,
	NXE2000_IRQ_GPIO3,
	NXE2000_IRQ_GPIO4,

	NXE2000_IRQ_CTC,
	NXE2000_IRQ_DALE,

	NXE2000_IRQ_FVADPDETSINT,
	NXE2000_IRQ_FVUSBDETSINT,
	NXE2000_IRQ_FVADPLVSINT,
	NXE2000_IRQ_FVUSBLVSINT,
	NXE2000_IRQ_FWVADPSINT,
	NXE2000_IRQ_FWVUSBSINT,

	NXE2000_IRQ_FONCHGINT,
	NXE2000_IRQ_FCHGCMPINT,
	NXE2000_IRQ_FBATOPENINT,
	NXE2000_IRQ_FSLPMODEINT,
	NXE2000_IRQ_FBTEMPJTA1INT,
	NXE2000_IRQ_FBTEMPJTA2INT,
	NXE2000_IRQ_FBTEMPJTA3INT,
	NXE2000_IRQ_FBTEMPJTA4INT,

	NXE2000_IRQ_FCURTERMINT,
	NXE2000_IRQ_FVOLTERMINT,
	NXE2000_IRQ_FICRVSINT,
	NXE2000_IRQ_FPOOR_CHGCURINT,
	NXE2000_IRQ_FOSCFDETINT1,
	NXE2000_IRQ_FOSCFDETINT2,
	NXE2000_IRQ_FOSCFDETINT3,
	NXE2000_IRQ_FOSCMDETINT,

	NXE2000_IRQ_FDIEOFFINT,
	NXE2000_IRQ_FDIEERRINT,
	NXE2000_IRQ_FBTEMPERRINT,
	NXE2000_IRQ_FVBATOVINT,
	NXE2000_IRQ_FTTIMOVINT,
	NXE2000_IRQ_FRTIMOVINT,
	NXE2000_IRQ_FVADPOVSINT,
	NXE2000_IRQ_FVUSBOVSINT,

	NXE2000_IRQ_FGCDET,
	NXE2000_IRQ_FPCDET,
	NXE2000_IRQ_FWARN_ADP,

	/* Should be last entry */
	NXE2000_NR_IRQS,
};

/* NXE2000 gpio definitions */
enum {
	NXE2000_GPIO0,
	NXE2000_GPIO1,
	NXE2000_GPIO2,
	NXE2000_GPIO3,
	NXE2000_GPIO4,

	NXE2000_NR_GPIO,
};

enum nxe2000_sleep_control_id {
	NXE2000_DS_DC1,
	NXE2000_DS_DC2,
	NXE2000_DS_DC3,
	NXE2000_DS_DC4,
	NXE2000_DS_DC5,
	NXE2000_DS_LDO1,
	NXE2000_DS_LDO2,
	NXE2000_DS_LDO3,
	NXE2000_DS_LDO4,
	NXE2000_DS_LDO5,
	NXE2000_DS_LDO6,
	NXE2000_DS_LDO7,
	NXE2000_DS_LDO8,
	NXE2000_DS_LDO9,
	NXE2000_DS_LDO10,
	NXE2000_DS_LDORTC1,
	NXE2000_DS_LDORTC2,
	NXE2000_DS_PSO0,
	NXE2000_DS_PSO1,
	NXE2000_DS_PSO2,
	NXE2000_DS_PSO3,
	NXE2000_DS_PSO4,
};

enum int_type {
	SYS_INT		= 0x01,
	DCDC_INT	= 0x02,
	RTC_INT		= 0x04,
	ADC_INT		= 0x08,
	GPIO_INT	= 0x10,
	WDG_INT		= 0x20,
	CHG_INT		= 0x40,
	FG_INT		= 0x80,
};

struct nxe2000_subdev_info {
	int			id;
	const char	*name;
	void		*platform_data;
};

struct nxe2000_gpio_init_data {
	unsigned output_mode_en:1;	/* Enable output mode during init */
	unsigned output_val:1;	/* Output value if it is in output mode */
	unsigned init_apply:1;	/* Apply init data on configuring gpios*/
	unsigned led_mode:1;	/* Select LED mode during init */
	unsigned led_func:1;	/* Set LED function if LED mode is 1 */
};

struct nxe2000 {
	struct device		*dev;
	struct i2c_client	*client;
	struct mutex		io_lock;
	int			gpio_base;
	struct gpio_chip	gpio_chip;
	int			irq_base;
/*	struct irq_chip		irq_chip; */
	int			chip_irq;
	int			chip_irq_type;
	struct mutex		irq_lock;
	unsigned long		group_irq_en[MAX_MAIN_INTERRUPT];

	/* For main interrupt bits in INTC */
	u8			intc_inten_cache;
	u8			intc_inten_reg;

	/* For group interrupt bits and address */
	u8			irq_en_cache[MAX_INTERRUPT_MASKS];
	u8			irq_en_reg[MAX_INTERRUPT_MASKS];

	/* For gpio edge */
	u8			gpedge_cache[MAX_GPEDGE_REG];
	u8			gpedge_reg[MAX_GPEDGE_REG];

	int			bank_num;
};

struct nxe2000_platform_data {
	int		num_subdevs;
	struct	nxe2000_subdev_info *subdevs;
	int (*init_port)(int irq_num); /* Init GPIO for IRQ pin */
	int		gpio_base;
	int		irq_base;
	int		irq_type;
	struct nxe2000_gpio_init_data *gpio_init_data;
	int num_gpioinit_data;
	bool enable_shutdown_pin;
};

/* ==================================== */
/* NXE2000 Power_Key device data	*/
/* ==================================== */
struct nxe2000_pwrkey_platform_data {
	int irq;
	unsigned long delay_ms;
};
extern int pwrkey_wakeup;
/* ==================================== */
/* NXE2000 battery device data		*/
/* ==================================== */
#if defined(CONFIG_BATTERY_NXE2000)
extern int g_soc;
extern int g_fg_on_mode;
#endif

extern int nxe2000_pm_read(uint8_t reg, uint8_t *val);
extern int nxe2000_read(struct device *dev, uint8_t reg, uint8_t *val);
extern int nxe2000_read_bank1(struct device *dev, uint8_t reg, uint8_t *val);
extern int nxe2000_bulk_reads(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int nxe2000_bulk_reads_bank1(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int nxe2000_pm_write(u8 reg, uint8_t val);
extern int nxe2000_write(struct device *dev, u8 reg, uint8_t val);
extern int nxe2000_write_bank1(struct device *dev, u8 reg, uint8_t val);
extern int nxe2000_bulk_writes(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int nxe2000_bulk_writes_bank1(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int nxe2000_set_bits(struct device *dev, u8 reg, uint8_t bit_mask);
extern int nxe2000_clr_bits(struct device *dev, u8 reg, uint8_t bit_mask);
extern int nxe2000_update(struct device *dev, u8 reg, uint8_t val,
								uint8_t mask);
extern int nxe2000_update_bank1(struct device *dev, u8 reg, uint8_t val,
								uint8_t mask);
extern void nxe2000_power_off(void);
extern int nxe2000_irq_init(struct nxe2000 *nxe2000);
extern int nxe2000_irq_exit(struct nxe2000 *nxe2000);

#endif
