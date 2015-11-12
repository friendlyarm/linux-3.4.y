/*
 * driver/mfd/nxe2000-irq.c
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/mfd/nxe2000.h>


static int gpedge_add[] = {
	NXE2000_GPIO_GPEDGE1,
	NXE2000_GPIO_GPEDGE2
};

static int irq_en_add[] = {
	NXE2000_INT_EN_SYS,
	NXE2000_INT_EN_DCDC,
	NXE2000_INT_EN_RTC,
	NXE2000_INT_EN_ADC1,
	NXE2000_INT_EN_ADC2,
	NXE2000_INT_EN_ADC3,
	NXE2000_INT_EN_GPIO,
	NXE2000_INT_EN_GPIO2,
	NXE2000_INT_MSK_CHGCTR,
	NXE2000_INT_MSK_CHGSTS1,
	NXE2000_INT_MSK_CHGSTS2,
	NXE2000_INT_MSK_CHGERR,
	NXE2000_INT_MSK_CHGEXTIF
};

static int irq_mon_add[] = {
	NXE2000_INT_IR_SYS,		/* NXE2000_INT_MON_SYS, */
	NXE2000_INT_IR_DCDC,		/* NXE2000_INT_MON_DCDC, */
	NXE2000_INT_IR_RTC,		/* NXE2000_INT_MON_RTC, */
	NXE2000_INT_IR_ADCL,
	NXE2000_INT_IR_ADCH,
	NXE2000_INT_IR_ADCEND,
	NXE2000_INT_IR_GPIOR,
	NXE2000_INT_IR_GPIOF,
	NXE2000_INT_IR_CHGCTR,		/* NXE2000_INT_MON_CHGCTR, */
	NXE2000_INT_IR_CHGSTS1,	/* NXE2000_INT_MON_CHGSTS1, */
	NXE2000_INT_IR_CHGSTS2,	/* NXE2000_INT_MON_CHGSTS2, */
	NXE2000_INT_IR_CHGERR,		/* NXE2000_INT_MON_CHGERR */
	NXE2000_INT_IR_CHGEXTIF	/* NXE2000_INT_MON_CHGEXTIF */
};

static int irq_clr_add[] = {
	NXE2000_INT_IR_SYS,
	NXE2000_INT_IR_DCDC,
	NXE2000_INT_IR_RTC,
	NXE2000_INT_IR_ADCL,
	NXE2000_INT_IR_ADCH,
	NXE2000_INT_IR_ADCEND,
	NXE2000_INT_IR_GPIOR,
	NXE2000_INT_IR_GPIOF,
	NXE2000_INT_IR_CHGCTR,
	NXE2000_INT_IR_CHGSTS1,
	NXE2000_INT_IR_CHGSTS2,
	NXE2000_INT_IR_CHGERR,
	NXE2000_INT_IR_CHGEXTIF
};

static int main_int_type[] = {
#ifdef CONFIG_NXE2000_WDG_TEST
	WDG_INT|
#endif
	SYS_INT,
	DCDC_INT,
	RTC_INT,
	ADC_INT,
	ADC_INT,
	ADC_INT,
	GPIO_INT,
	GPIO_INT,
	CHG_INT,
	CHG_INT,
	CHG_INT,
	CHG_INT,
	CHG_INT,
};

struct nxe2000_irq_data {
	u8	int_type;
	u8	master_bit;
	u8	int_en_bit;
	u8	mask_reg_index;
	int	grp_index;
};

#define NXE2000_IRQ(_int_type, _master_bit, _grp_index, _int_bit, _mask_ind) \
	{						\
		.int_type	= _int_type,		\
		.master_bit	= _master_bit,		\
		.grp_index	= _grp_index,		\
		.int_en_bit	= _int_bit,		\
		.mask_reg_index	= _mask_ind,		\
	}

static const struct nxe2000_irq_data nxe2000_irqs[NXE2000_NR_IRQS] = {
	[NXE2000_IRQ_POWER_ON]		= NXE2000_IRQ(SYS_INT,  0, 0, 0, 0),
	[NXE2000_IRQ_EXTIN]			= NXE2000_IRQ(SYS_INT,  0, 1, 1, 0),
	[NXE2000_IRQ_PRE_VINDT]		= NXE2000_IRQ(SYS_INT,  0, 2, 2, 0),
	[NXE2000_IRQ_PREOT]			= NXE2000_IRQ(SYS_INT,  0, 3, 3, 0),
	[NXE2000_IRQ_POWER_OFF]		= NXE2000_IRQ(SYS_INT,  0, 4, 4, 0),
	[NXE2000_IRQ_NOE_OFF]		= NXE2000_IRQ(SYS_INT,  0, 5, 5, 0),
	[NXE2000_IRQ_WD]			= NXE2000_IRQ(SYS_INT,  0, 6, 6, 0),

	[NXE2000_IRQ_DC1LIM]		= NXE2000_IRQ(DCDC_INT, 1, 0, 0, 1),
	[NXE2000_IRQ_DC2LIM]		= NXE2000_IRQ(DCDC_INT, 1, 1, 1, 1),
	[NXE2000_IRQ_DC3LIM]		= NXE2000_IRQ(DCDC_INT, 1, 2, 2, 1),
	[NXE2000_IRQ_DC4LIM]		= NXE2000_IRQ(DCDC_INT, 1, 3, 3, 1),
	[NXE2000_IRQ_DC5LIM]		= NXE2000_IRQ(DCDC_INT, 1, 4, 4, 1),

	[NXE2000_IRQ_CTC]			= NXE2000_IRQ(RTC_INT,  2, 0, 0, 2),
	[NXE2000_IRQ_DALE]			= NXE2000_IRQ(RTC_INT,  2, 1, 6, 2),

	[NXE2000_IRQ_ILIMLIR]		= NXE2000_IRQ(ADC_INT,  3, 0, 0, 3),
	[NXE2000_IRQ_VBATLIR]		= NXE2000_IRQ(ADC_INT,  3, 1, 1, 3),
	[NXE2000_IRQ_VADPLIR]		= NXE2000_IRQ(ADC_INT,  3, 2, 2, 3),
	[NXE2000_IRQ_VUSBLIR]		= NXE2000_IRQ(ADC_INT,  3, 3, 3, 3),
	[NXE2000_IRQ_VSYSLIR]		= NXE2000_IRQ(ADC_INT,  3, 4, 4, 3),
	[NXE2000_IRQ_VTHMLIR]		= NXE2000_IRQ(ADC_INT,  3, 5, 5, 3),
	[NXE2000_IRQ_AIN1LIR]		= NXE2000_IRQ(ADC_INT,  3, 6, 6, 3),
	[NXE2000_IRQ_AIN0LIR]		= NXE2000_IRQ(ADC_INT,  3, 7, 7, 3),

	[NXE2000_IRQ_ILIMHIR]		= NXE2000_IRQ(ADC_INT,  3, 8, 0, 4),
	[NXE2000_IRQ_VBATHIR]		= NXE2000_IRQ(ADC_INT,  3, 9, 1, 4),
	[NXE2000_IRQ_VADPHIR]		= NXE2000_IRQ(ADC_INT,  3, 10, 2, 4),
	[NXE2000_IRQ_VUSBHIR]		= NXE2000_IRQ(ADC_INT,  3, 11, 3, 4),
	[NXE2000_IRQ_VSYSHIR]		= NXE2000_IRQ(ADC_INT,  3, 12, 4, 4),
	[NXE2000_IRQ_VTHMHIR]		= NXE2000_IRQ(ADC_INT,  3, 13, 5, 4),
	[NXE2000_IRQ_AIN1HIR]		= NXE2000_IRQ(ADC_INT,  3, 14, 6, 4),
	[NXE2000_IRQ_AIN0HIR]		= NXE2000_IRQ(ADC_INT,  3, 15, 7, 4),

	[NXE2000_IRQ_ADC_ENDIR]		= NXE2000_IRQ(ADC_INT,  3, 16, 0, 5),

	[NXE2000_IRQ_GPIO0]			= NXE2000_IRQ(GPIO_INT, 4, 0, 0, 6),
	[NXE2000_IRQ_GPIO1]			= NXE2000_IRQ(GPIO_INT, 4, 1, 1, 6),
	[NXE2000_IRQ_GPIO2]			= NXE2000_IRQ(GPIO_INT, 4, 2, 2, 6),
	[NXE2000_IRQ_GPIO3]			= NXE2000_IRQ(GPIO_INT, 4, 3, 3, 6),
	[NXE2000_IRQ_GPIO4]			= NXE2000_IRQ(GPIO_INT, 4, 4, 4, 6),

	[NXE2000_IRQ_FVADPDETSINT]	= NXE2000_IRQ(CHG_INT, 6, 0, 0, 8),
	[NXE2000_IRQ_FVUSBDETSINT]	= NXE2000_IRQ(CHG_INT, 6, 1, 1, 8),
	[NXE2000_IRQ_FVADPLVSINT]	= NXE2000_IRQ(CHG_INT, 6, 2, 2, 8),
	[NXE2000_IRQ_FVUSBLVSINT]	= NXE2000_IRQ(CHG_INT, 6, 3, 3, 8),
	[NXE2000_IRQ_FWVADPSINT]	= NXE2000_IRQ(CHG_INT, 6, 4, 4, 8),
	[NXE2000_IRQ_FWVUSBSINT]	= NXE2000_IRQ(CHG_INT, 6, 5, 5, 8),

	[NXE2000_IRQ_FONCHGINT]		= NXE2000_IRQ(CHG_INT, 6, 6, 0, 9),
	[NXE2000_IRQ_FCHGCMPINT]	= NXE2000_IRQ(CHG_INT, 6, 7, 1, 9),
	[NXE2000_IRQ_FBATOPENINT]	= NXE2000_IRQ(CHG_INT, 6, 8, 2, 9),
	[NXE2000_IRQ_FSLPMODEINT]	= NXE2000_IRQ(CHG_INT, 6, 9, 3, 9),
	[NXE2000_IRQ_FBTEMPJTA1INT]	= NXE2000_IRQ(CHG_INT, 6, 10, 4, 9),
	[NXE2000_IRQ_FBTEMPJTA2INT]	= NXE2000_IRQ(CHG_INT, 6, 11, 5, 9),
	[NXE2000_IRQ_FBTEMPJTA3INT]	= NXE2000_IRQ(CHG_INT, 6, 12, 6, 9),
	[NXE2000_IRQ_FBTEMPJTA4INT]	= NXE2000_IRQ(CHG_INT, 6, 13, 7, 9),

	[NXE2000_IRQ_FCURTERMINT]	= NXE2000_IRQ(CHG_INT, 6, 14, 0, 10),
	[NXE2000_IRQ_FVOLTERMINT]	= NXE2000_IRQ(CHG_INT, 6, 15, 1, 10),
	[NXE2000_IRQ_FICRVSINT]		= NXE2000_IRQ(CHG_INT, 6, 16, 2, 10),
	[NXE2000_IRQ_FPOOR_CHGCURINT]	= NXE2000_IRQ(CHG_INT, 6, 17, 3, 10),
	[NXE2000_IRQ_FOSCFDETINT1]	= NXE2000_IRQ(CHG_INT, 6, 18, 4, 10),
	[NXE2000_IRQ_FOSCFDETINT2]	= NXE2000_IRQ(CHG_INT, 6, 19, 5, 10),
	[NXE2000_IRQ_FOSCFDETINT3]	= NXE2000_IRQ(CHG_INT, 6, 20, 6, 10),
	[NXE2000_IRQ_FOSCMDETINT]	= NXE2000_IRQ(CHG_INT, 6, 21, 7, 10),

	[NXE2000_IRQ_FDIEOFFINT]	= NXE2000_IRQ(CHG_INT, 6, 22, 0, 11),
	[NXE2000_IRQ_FDIEERRINT]	= NXE2000_IRQ(CHG_INT, 6, 23, 1, 11),
	[NXE2000_IRQ_FBTEMPERRINT]	= NXE2000_IRQ(CHG_INT, 6, 24, 2, 11),
	[NXE2000_IRQ_FVBATOVINT]	= NXE2000_IRQ(CHG_INT, 6, 25, 3, 11),
	[NXE2000_IRQ_FTTIMOVINT]	= NXE2000_IRQ(CHG_INT, 6, 26, 4, 11),
	[NXE2000_IRQ_FRTIMOVINT]	= NXE2000_IRQ(CHG_INT, 6, 27, 5, 11),
	[NXE2000_IRQ_FVADPOVSINT]	= NXE2000_IRQ(CHG_INT, 6, 28, 6, 11),
	[NXE2000_IRQ_FVUSBOVSINT]	= NXE2000_IRQ(CHG_INT, 6, 29, 7, 11),

	[NXE2000_IRQ_FGCDET]		= NXE2000_IRQ(CHG_INT, 6, 30, 0, 12),
	[NXE2000_IRQ_FPCDET]		= NXE2000_IRQ(CHG_INT, 6, 31, 1, 12),
	[NXE2000_IRQ_FWARN_ADP]		= NXE2000_IRQ(CHG_INT, 6, 32, 3, 12),

};

static void nxe2000_irq_lock(struct irq_data *irq_data)
{
	struct nxe2000 *nxe2000 = irq_data_get_irq_chip_data(irq_data);

	mutex_lock(&nxe2000->irq_lock);
}

static void nxe2000_irq_unmask(struct irq_data *irq_data)
{
	struct nxe2000 *nxe2000 = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - nxe2000->irq_base;
	const struct nxe2000_irq_data *data = &nxe2000_irqs[__irq];

	nxe2000->group_irq_en[data->master_bit] |= (1 << data->grp_index);
	if (nxe2000->group_irq_en[data->master_bit])
		nxe2000->intc_inten_reg |= 1 << data->master_bit;

	if (data->master_bit == 6)	/* if Charger */
		nxe2000->irq_en_reg[data->mask_reg_index]
						&= ~(1 << data->int_en_bit);
	else
		nxe2000->irq_en_reg[data->mask_reg_index]
						|= 1 << data->int_en_bit;
}

static void nxe2000_irq_mask(struct irq_data *irq_data)
{
	struct nxe2000 *nxe2000 = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - nxe2000->irq_base;
	const struct nxe2000_irq_data *data = &nxe2000_irqs[__irq];

	nxe2000->group_irq_en[data->master_bit] &= ~(1 << data->grp_index);
	if (!nxe2000->group_irq_en[data->master_bit])
		nxe2000->intc_inten_reg &= ~(1 << data->master_bit);

	if (data->master_bit == 6)	/* if Charger */
		nxe2000->irq_en_reg[data->mask_reg_index]
						|= 1 << data->int_en_bit;
	else
		nxe2000->irq_en_reg[data->mask_reg_index]
						&= ~(1 << data->int_en_bit);
}

static void nxe2000_irq_sync_unlock(struct irq_data *irq_data)
{
	struct nxe2000 *nxe2000 = irq_data_get_irq_chip_data(irq_data);
	int i;

	for (i = 0; i < ARRAY_SIZE(nxe2000->gpedge_reg); i++) {
		if (nxe2000->gpedge_reg[i] != nxe2000->gpedge_cache[i]) {
			if (!WARN_ON(nxe2000_write(nxe2000->dev,
						    gpedge_add[i],
						    nxe2000->gpedge_reg[i])))
				nxe2000->gpedge_cache[i] =
						nxe2000->gpedge_reg[i];
		}
	}

	for (i = 0; i < ARRAY_SIZE(nxe2000->irq_en_reg); i++) {
		if (nxe2000->irq_en_reg[i] != nxe2000->irq_en_cache[i]) {
			if (!WARN_ON(nxe2000_write(nxe2000->dev,
						irq_en_add[i],
						nxe2000->irq_en_reg[i])))
				nxe2000->irq_en_cache[i] =
						nxe2000->irq_en_reg[i];
		}
	}

	if (nxe2000->intc_inten_reg != nxe2000->intc_inten_cache) {
		if (!WARN_ON(nxe2000_write(nxe2000->dev,
				NXE2000_INTC_INTEN, nxe2000->intc_inten_reg)))
			nxe2000->intc_inten_cache = nxe2000->intc_inten_reg;
	}

	mutex_unlock(&nxe2000->irq_lock);
}

static int nxe2000_irq_set_type(struct irq_data *irq_data, unsigned int type)
{
	struct nxe2000 *nxe2000 = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - nxe2000->irq_base;
	const struct nxe2000_irq_data *data = &nxe2000_irqs[__irq];
	int val = 0;
	int gpedge_index;
	int gpedge_bit_pos;

	if (data->int_type & GPIO_INT) {
		gpedge_index = data->int_en_bit / 4;
		gpedge_bit_pos = data->int_en_bit % 4;

		if (type & IRQ_TYPE_EDGE_FALLING)
			val |= 0x2;

		if (type & IRQ_TYPE_EDGE_RISING)
			val |= 0x1;

		nxe2000->gpedge_reg[gpedge_index] &= ~(3 << gpedge_bit_pos);
		nxe2000->gpedge_reg[gpedge_index] |= (val << gpedge_bit_pos);
		nxe2000_irq_unmask(irq_data);
	}
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int nxe2000_irq_set_wake(struct irq_data *irq_data, unsigned int on)
{
	struct nxe2000 *nxe2000 = irq_data_get_irq_chip_data(irq_data);
	return irq_set_irq_wake(nxe2000->chip_irq, on);	/* i2c->irq */
}
#else
#define nxe2000_irq_set_wake NULL
#endif

static irqreturn_t nxe2000_irq_isr(int irq, void *data)
{
	struct nxe2000 *nxe2000 = data;
	u8 int_sts[MAX_INTERRUPT_MASKS];
	u8 master_int;
	int i;
	int ret;
	unsigned int rtc_int_sts = 0;

	/* disable_irq_nosync(irq); */
	/* Clear the status */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)
		int_sts[i] = 0;

	ret = nxe2000_read(nxe2000->dev, NXE2000_INTC_INTMON,
						&master_int);
	if (ret < 0) {
		dev_err(nxe2000->dev, "Error in reading reg 0x%02x "
			"error: %d\n", NXE2000_INTC_INTMON, ret);
		return IRQ_HANDLED;
	}

	for (i = 0; i < MAX_INTERRUPT_MASKS; ++i) {
		/* Even if INTC_INTMON register = 1, INT signal might not
		 * output because INTC_INTMON register indicates only interrupt
		 * facter level.
		 * So remove the following procedure
		 */
		if (!(master_int & main_int_type[i])) 
			continue;

		ret = nxe2000_read(nxe2000->dev,
				irq_mon_add[i], &int_sts[i]);
		if (ret < 0) {
			dev_err(nxe2000->dev, "Error in reading reg 0x%02x "
				"error: %d\n", irq_mon_add[i], ret);
			int_sts[i] = 0;
			continue;
		}
		if (!int_sts[i])
			continue;

		if (main_int_type[i] & RTC_INT) {
			/* Changes status bit position
				 from RTCCNT2 to RTCCNT1 */
			rtc_int_sts = 0;
			if (int_sts[i] & 0x1)
				rtc_int_sts |= BIT(6);
			if (int_sts[i] & 0x4)
				rtc_int_sts |= BIT(0);
		}

		if(irq_clr_add[i] == NXE2000_INT_IR_RTC)
		{
			int_sts[i] &= ~0x85;
			ret = nxe2000_write(nxe2000->dev,
				irq_clr_add[i], int_sts[i]);
			if (ret < 0) {
				dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_clr_add[i], ret);
			}
		}
#ifdef CONFIG_NXE2000_WDG_TEST
		else if (main_int_type[i] & WDG_INT) /* Mask Watchdog Interrupt */
		{
			printk(KERN_ERR "## \e[31m%s\e[0m() WDG_INT \n", __func__);
		}
#endif
		else
		{
			ret = nxe2000_write(nxe2000->dev,
				irq_clr_add[i], ~int_sts[i]);
			if (ret < 0) {
				dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_clr_add[i], ret);
			}
		}
		
		/* Mask Charger Interrupt */
		if (main_int_type[i] & CHG_INT) {
			if (int_sts[i])
				ret = nxe2000_write(nxe2000->dev,
							irq_en_add[i], 0xff);
				if (ret < 0) {
					dev_err(nxe2000->dev,
						"Error in write reg 0x%02x error: %d\n",
							irq_en_add[i], ret);
				}
		}
		/* Mask ADC Interrupt */
		if (main_int_type[i] & ADC_INT) {
			if (int_sts[i])
				ret = nxe2000_write(nxe2000->dev,
							irq_en_add[i], 0);
				if (ret < 0) {
					dev_err(nxe2000->dev,
						"Error in write reg 0x%02x error: %d\n",
							irq_en_add[i], ret);
				}
		}

		if (main_int_type[i] & RTC_INT)
			int_sts[i] = rtc_int_sts;

	}

	/* Merge gpio interrupts  for rising and falling case*/
	int_sts[6] |= int_sts[7];

	/* Call interrupt handler if enabled */
	for (i = 0; i < NXE2000_NR_IRQS; ++i) {
		const struct nxe2000_irq_data *data = &nxe2000_irqs[i];
		if ((int_sts[data->mask_reg_index] & (1 << data->int_en_bit)) &&
			(nxe2000->group_irq_en[data->master_bit] &
					(1 << data->grp_index)))
			handle_nested_irq(nxe2000->irq_base + i);
	}

	return IRQ_HANDLED;
}

static struct irq_chip nxe2000_irq_chip = {
	.name = "nxe2000",
	.irq_mask = nxe2000_irq_mask,
	.irq_unmask = nxe2000_irq_unmask,
	.irq_bus_lock = nxe2000_irq_lock,
	.irq_bus_sync_unlock = nxe2000_irq_sync_unlock,
	.irq_set_type = nxe2000_irq_set_type,
	.irq_set_wake = nxe2000_irq_set_wake,
};

int nxe2000_irq_init(struct nxe2000 *nxe2000)
{
	int i, ret;
	u8 reg_data = 0;

	if (!nxe2000->irq_base) {
		dev_warn(nxe2000->dev, "No interrupt support on IRQ base\n");
		return -EINVAL;
	}

	mutex_init(&nxe2000->irq_lock);

#ifdef CONFIG_NXE2000_WDG_TEST
	nxe2000->irq_en_cache[0] = 0x40;
	nxe2000->irq_en_reg[0] = 0x40;

	nxe2000->irq_en_cache[1] = 0;
	nxe2000->irq_en_reg[1] = 0;
#else
	/* Initialize all locals to 0 */
	for (i = 0; i < 2; i++) {
		nxe2000->irq_en_cache[i] = 0;
		nxe2000->irq_en_reg[i] = 0;
	}
#endif

	/* Initialize rtc */
	nxe2000->irq_en_cache[2] = 0x20;
	nxe2000->irq_en_reg[2] = 0x20;

	/* Initialize all locals to 0 */
	for (i = 3; i < 8; i++) {
		nxe2000->irq_en_cache[i] = 0;
		nxe2000->irq_en_reg[i] = 0;
	}
	/* Charger Mask register must be set to 0xff for masking Int output. */
	for (i = 8; i < MAX_INTERRUPT_MASKS; i++) {
		nxe2000->irq_en_cache[i] = 0xff;
		nxe2000->irq_en_reg[i] = 0xff;
	}

	nxe2000->intc_inten_cache = 0;
	nxe2000->intc_inten_reg = 0;
	for (i = 0; i < MAX_GPEDGE_REG; i++) {
		nxe2000->gpedge_cache[i] = 0;
		nxe2000->gpedge_reg[i] = 0;
	}

	/* Initailize all int register to 0 */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)  {
		ret = nxe2000_write(nxe2000->dev,
				irq_en_add[i],
				nxe2000->irq_en_reg[i]);
		if (ret < 0)
			dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_en_add[i], ret);
	}

	for (i = 0; i < MAX_GPEDGE_REG; i++)  {
		ret = nxe2000_write(nxe2000->dev,
				gpedge_add[i],
				nxe2000->gpedge_reg[i]);
		if (ret < 0)
			dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
				"error: %d\n", gpedge_add[i], ret);
	}

	ret = nxe2000_write(nxe2000->dev, NXE2000_INTC_INTEN, 0x0);
	if (ret < 0)
		dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
				"error: %d\n", NXE2000_INTC_INTEN, ret);

	/* Clear all interrupts in case they woke up active. */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)  {
		if (irq_clr_add[i] != NXE2000_INT_IR_RTC) {
			ret = nxe2000_write(nxe2000->dev,
						irq_clr_add[i], 0);
			if (ret < 0)
				dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
					"error: %d\n", irq_clr_add[i], ret);
		} else {
			ret = nxe2000_read(nxe2000->dev,
					NXE2000_INT_IR_RTC, &reg_data);
			if (ret < 0)
				dev_err(nxe2000->dev, "Error in reading reg 0x%02x "
					"error: %d\n", NXE2000_INT_IR_RTC, ret);
			reg_data &= 0xf0;
			ret = nxe2000_write(nxe2000->dev,
					NXE2000_INT_IR_RTC, reg_data);
			if (ret < 0)
				dev_err(nxe2000->dev, "Error in writing reg 0x%02x "
					"error: %d\n", NXE2000_INT_IR_RTC, ret);
		}
	}


	for (i = 0; i < NXE2000_NR_IRQS; i++) {
		int __irq = i + nxe2000->irq_base;
		irq_set_chip_data(__irq, nxe2000);
		irq_set_chip_and_handler(__irq, &nxe2000_irq_chip,
					 handle_simple_irq);
		irq_set_nested_thread(__irq, 1);
#ifdef CONFIG_ARM
		set_irq_flags(__irq, IRQF_VALID);
#endif
	}

	ret = request_threaded_irq(nxe2000->chip_irq, NULL, nxe2000_irq_isr,
			nxe2000->chip_irq_type|IRQF_DISABLED|IRQF_ONESHOT,
						"nxe2000", nxe2000);
	if (ret < 0)
		dev_err(nxe2000->dev, "Error in registering interrupt "
				"error: %d\n", ret);
	if (!ret) {
		device_init_wakeup(nxe2000->dev, 1);
		enable_irq_wake(nxe2000->chip_irq);
	}

	return ret;
}

int nxe2000_irq_exit(struct nxe2000 *nxe2000)
{
	if (nxe2000->chip_irq)
		free_irq(nxe2000->chip_irq, nxe2000);
	return 0;
}

