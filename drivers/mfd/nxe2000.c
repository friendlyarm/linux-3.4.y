/*
 * driver/mfd/nxe2000.c
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
/*#define DEBUG			1*/
/*#define VERBOSE_DEBUG		1*/
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/nxe2000.h>
#include <mach/platform.h>
#include <mach/pm.h>

#define NXE2000_PM_RESTART		(0)

static struct i2c_client *nxe2000_i2c_client;

static inline int __nxe2000_read(struct i2c_client *client,
				  u8 reg, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading at 0x%02x\n", reg);
		return ret;
	}

	*val = (uint8_t)ret;
	dev_dbg(&client->dev, "nxe2000: reg read  reg=%x, val=%x\n",
				reg, *val);
	return 0;
}

static inline int __nxe2000_bulk_reads(struct i2c_client *client, u8 reg,
				int len, uint8_t *val)
{
	int ret;
	int i;

	ret = i2c_smbus_read_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading from 0x%02x\n", reg);
		return ret;
	}
	for (i = 0; i < len; ++i) {
		dev_dbg(&client->dev, "nxe2000: reg read  reg=%x, val=%x\n",
				reg + i, *(val + i));
	}
	return 0;
}

static inline int __nxe2000_write(struct i2c_client *client,
				 u8 reg, uint8_t val)
{
	int ret;

	dev_dbg(&client->dev, "nxe2000: reg write  reg=%x, val=%x\n",
				reg, val);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writing 0x%02x to 0x%02x\n",
				val, reg);
		return ret;
	}

	return 0;
}

static inline int __nxe2000_bulk_writes(struct i2c_client *client, u8 reg,
				  int len, uint8_t *val)
{
	int ret;
	int i;

	for (i = 0; i < len; ++i) {
		dev_dbg(&client->dev, "nxe2000: reg write  reg=%x, val=%x\n",
				reg + i, *(val + i));
	}

	ret = i2c_smbus_write_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writings to 0x%02x\n", reg);
		return ret;
	}

	return 0;
}

static inline int set_bank_nxe2000(struct device *dev, int bank)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret;

	if (bank != (bank & 1))
		return -EINVAL;
	if (bank == nxe2000->bank_num)
		return 0;
	ret = __nxe2000_write(to_i2c_client(dev), NXE2000_REG_BANKSEL, bank);
	if (!ret)
		nxe2000->bank_num = bank;

	return ret;
}

int nxe2000_pm_write(u8 reg, uint8_t val)
{
	int ret = 0;

	ret = nxe2000_write(&nxe2000_i2c_client->dev, reg, val);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_pm_write);

int nxe2000_write(struct device *dev, u8 reg, uint8_t val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret)
		ret = __nxe2000_write(to_i2c_client(dev), reg, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_write);

int nxe2000_write_bank1(struct device *dev, u8 reg, uint8_t val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 1);
	if (!ret)
		ret = __nxe2000_write(to_i2c_client(dev), reg, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_write_bank1);

int nxe2000_bulk_writes(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret)
		ret = __nxe2000_bulk_writes(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_bulk_writes);

int nxe2000_bulk_writes_bank1(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 1);
	if (!ret)
		ret = __nxe2000_bulk_writes(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_bulk_writes_bank1);

int nxe2000_pm_read(u8 reg, uint8_t *val)
{
	int ret = 0;

	ret = nxe2000_read(&nxe2000_i2c_client->dev, reg, val);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_pm_read);

int nxe2000_read(struct device *dev, u8 reg, uint8_t *val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret)
		ret = __nxe2000_read(to_i2c_client(dev), reg, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_read);

int nxe2000_read_bank1(struct device *dev, u8 reg, uint8_t *val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 1);
	if (!ret)
		ret =  __nxe2000_read(to_i2c_client(dev), reg, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_read_bank1);

int nxe2000_bulk_reads(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret)
		ret = __nxe2000_bulk_reads(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_bulk_reads);

int nxe2000_bulk_reads_bank1(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 1);
	if (!ret)
		ret = __nxe2000_bulk_reads(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&nxe2000->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_bulk_reads_bank1);

int nxe2000_set_bits(struct device *dev, u8 reg, uint8_t bit_mask)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret) {
		ret = __nxe2000_read(to_i2c_client(dev), reg, &reg_val);
		if (ret)
			goto out;

		if ((reg_val & bit_mask) != bit_mask) {
			reg_val |= bit_mask;
			ret = __nxe2000_write(to_i2c_client(dev), reg,
								 reg_val);
		}
	}
out:
	mutex_unlock(&nxe2000->io_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_set_bits);

int nxe2000_clr_bits(struct device *dev, u8 reg, uint8_t bit_mask)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret) {
		ret = __nxe2000_read(to_i2c_client(dev), reg, &reg_val);
		if (ret)
			goto out;

		if (reg_val & bit_mask) {
			reg_val &= ~bit_mask;
			ret = __nxe2000_write(to_i2c_client(dev), reg,
								 reg_val);
		}
	}
out:
	mutex_unlock(&nxe2000->io_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_clr_bits);

int nxe2000_update(struct device *dev, u8 reg, uint8_t val, uint8_t mask)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 0);
	if (!ret) {
		ret = __nxe2000_read(nxe2000->client, reg, &reg_val);
		if (ret)
			goto out;

		if ((reg_val & mask) != val) {
			reg_val = (reg_val & ~mask) | (val & mask);
			ret = __nxe2000_write(nxe2000->client, reg, reg_val);
		}
	}
out:
	mutex_unlock(&nxe2000->io_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(nxe2000_update);

int nxe2000_update_bank1(struct device *dev, u8 reg, uint8_t val, uint8_t mask)
{
	struct nxe2000 *nxe2000 = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&nxe2000->io_lock);
	ret = set_bank_nxe2000(dev, 1);
	if (!ret) {
		ret = __nxe2000_read(nxe2000->client, reg, &reg_val);
		if (ret)
			goto out;

		if ((reg_val & mask) != val) {
			reg_val = (reg_val & ~mask) | (val & mask);
			ret = __nxe2000_write(nxe2000->client, reg, reg_val);
		}
	}
out:
	mutex_unlock(&nxe2000->io_lock);
	return ret;
}

//extern void (*pm_power_off)(void);
//static void (*backup_pm_power_off)(void);

#if (NXE2000_PM_RESTART == 1)
extern void (*arm_pm_restart)(char str, const char *cmd);
static void (*backup_pm_restart)(char str, const char *cmd);
#endif
void nxe2000_power_off(void)
{
	int ret;
	uint8_t reg_val = 0;

	if (!nxe2000_i2c_client)
		return;

#if defined(CONFIG_BATTERY_NXE2000)
	reg_val = g_soc;
	reg_val &= 0x7f;

	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PSWR, reg_val);
	if (ret < 0)
		dev_err(&nxe2000_i2c_client->dev, 
					"Error in writing PSWR_REG\n");

	if (g_fg_on_mode == 0) {
		/* Clear NXE2000_FG_CTRL 0x01 bit */
		ret = nxe2000_read(&nxe2000_i2c_client->dev,
						NXE2000_FG_CTRL, &reg_val);
		if (reg_val & 0x01) {
			reg_val &= ~0x01;
			ret = nxe2000_write(&nxe2000_i2c_client->dev,
						NXE2000_FG_CTRL, reg_val);
		}
		if (ret < 0)
			dev_err(&nxe2000_i2c_client->dev, 
					"Error in writing FG_CTRL\n");
	}
#endif

	/* set rapid timer 300 min */
	ret = nxe2000_read(&nxe2000_i2c_client->dev,
						TIMSET_REG, &reg_val);
	if (ret < 0)
		ret = nxe2000_read(&nxe2000_i2c_client->dev,
							TIMSET_REG, &reg_val);

	reg_val |= 0x03;

	ret = nxe2000_write(&nxe2000_i2c_client->dev,
						TIMSET_REG, reg_val);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev,
							TIMSET_REG, reg_val);
	if (ret < 0)
		dev_err(&nxe2000_i2c_client->dev, 
				"Error in writing the TIMSET_Reg\n");

	/* Disable all Interrupt */
	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_INTC_INTEN, 0);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_INTC_INTEN, 0);

	/* Not repeat power ON after power off(Power Off/N_OE) */
	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_REP_CNT, 0x0);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_REP_CNT, 0x0);

	/* Power OFF */
	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_SLP_CNT, 0x1);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_SLP_CNT, 0x1);

	//if (backup_pm_power_off)
	//	backup_pm_power_off();
	//halt();
}

#if (NXE2000_PM_RESTART == 1)
void nxe2000_restart(char str, const char *cmd)
{
	int ret;
	uint8_t reg_val = 0;

	if (!nxe2000_i2c_client)
		return;

#if defined(CONFIG_BATTERY_NXE2000)
	reg_val = g_soc;
	reg_val &= 0x7f;

	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PSWR, reg_val);
	if (ret < 0)
		dev_err(&nxe2000_i2c_client->dev, 
					"Error in writing PSWR_REG\n");

	if (g_fg_on_mode == 0) {
		/* Clear NXE2000_FG_CTRL 0x01 bit */
		ret = nxe2000_read(&nxe2000_i2c_client->dev,
						NXE2000_FG_CTRL, &reg_val);
		if (reg_val & 0x01) {
			reg_val &= ~0x01;
			ret = nxe2000_write(&nxe2000_i2c_client->dev,
						NXE2000_FG_CTRL, reg_val);
		}
		if (ret < 0)
			dev_err(&nxe2000_i2c_client->dev, 
					"Error in writing FG_CTRL\n");
	}
#endif

	/* set rapid timer 300 min */
	ret = nxe2000_read(&nxe2000_i2c_client->dev,
						TIMSET_REG, &reg_val);
	if (ret < 0)
		ret = nxe2000_read(&nxe2000_i2c_client->dev,
							TIMSET_REG, &reg_val);

	reg_val |= 0x03;

	ret = nxe2000_write(&nxe2000_i2c_client->dev,
						TIMSET_REG, reg_val);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev,
							TIMSET_REG, reg_val);
	if (ret < 0)
		dev_err(&nxe2000_i2c_client->dev, 
				"Error in writing the TIMSET_Reg\n");

	/* Disable all Interrupt */
	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_INTC_INTEN, 0);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_INTC_INTEN, 0);

	/* Not repeat power ON after power off(Power Off/N_OE) */
	reg_val = (0x1<<1) | 1;
	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_REP_CNT, reg_val);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_REP_CNT, reg_val);

	/* Power OFF */
	ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_SLP_CNT, 0x1);
	if (ret < 0)
		ret = nxe2000_write(&nxe2000_i2c_client->dev, NXE2000_PWR_SLP_CNT, 0x1);

	if (backup_pm_restart)
		backup_pm_restart(str, cmd);
}
#endif	/* #if (NXE2000_PM_RESTART == 1) */

#ifdef CONFIG_NXE2000_WDG_TEST
static irqreturn_t nxe2000_watchdog_isr(int irq, void *data)
{
	struct nxe2000 *info = data;
	printk(KERN_ERR "## \e[31m%s\e[0m() \n", __func__);

	nxe2000_clr_bits(info->dev, NXE2000_INT_IR_SYS, 0x40);

	return IRQ_HANDLED;
}

static void nxe2000_watchdog_init(struct nxe2000 *nxe2000)
{
	int ret;

	printk(KERN_ERR "## \e[31m%s\e[0m() \n", __func__);
	
	ret = request_threaded_irq((IRQ_SYSTEM_END + NXE2000_IRQ_WD),
					NULL, nxe2000_watchdog_isr, IRQF_ONESHOT, "nxe2000_watchdog_isr", nxe2000);

	nxe2000_set_bits(nxe2000->dev, NXE2000_PWR_REP_CNT, 0x01);
	nxe2000_write(nxe2000->dev, NXE2000_PWR_WD, 0x05);
	return;
}
#endif

static int nxe2000_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct nxe2000 *nxe2000 = container_of(gc, struct nxe2000,
								 gpio_chip);
	uint8_t val;
	int ret;

	ret = nxe2000_read(nxe2000->dev, NXE2000_GPIO_MON_IOIN, &val);
	if (ret < 0)
		return ret;

	return ((val & (0x1 << offset)) != 0);
}

static void nxe2000_gpio_set(struct gpio_chip *gc, unsigned offset,
			int value)
{
	struct nxe2000 *nxe2000 = container_of(gc, struct nxe2000,
								 gpio_chip);
	if (value)
		nxe2000_set_bits(nxe2000->dev, NXE2000_GPIO_IOOUT,
						1 << offset);
	else
		nxe2000_clr_bits(nxe2000->dev, NXE2000_GPIO_IOOUT,
						1 << offset);
}

static int nxe2000_gpio_input(struct gpio_chip *gc, unsigned offset)
{
	struct nxe2000 *nxe2000 = container_of(gc, struct nxe2000,
								 gpio_chip);

	return nxe2000_clr_bits(nxe2000->dev, NXE2000_GPIO_IOSEL,
						1 << offset);
}

static int nxe2000_gpio_output(struct gpio_chip *gc, unsigned offset,
				int value)
{
	struct nxe2000 *nxe2000 = container_of(gc, struct nxe2000,
								 gpio_chip);

	nxe2000_gpio_set(gc, offset, value);
	return nxe2000_set_bits(nxe2000->dev, NXE2000_GPIO_IOSEL,
						1 << offset);
}

static int nxe2000_gpio_to_irq(struct gpio_chip *gc, unsigned off)
{
	struct nxe2000 *nxe2000 = container_of(gc, struct nxe2000,
								 gpio_chip);

	if ((off >= 0) && (off < 8))
		return nxe2000->irq_base + NXE2000_IRQ_GPIO0 + off;

	return -EIO;
}


static void nxe2000_gpio_init(struct nxe2000 *nxe2000,
	struct nxe2000_platform_data *pdata)
{
	int ret;
	int i;
	struct nxe2000_gpio_init_data *ginit;

	if (pdata->gpio_base  <= 0)
		return;

	for (i = 0; i < pdata->num_gpioinit_data; ++i) {
		ginit = &pdata->gpio_init_data[i];

		if (!ginit->init_apply)
			continue;

		if (ginit->output_mode_en) {
			/* GPIO output mode */
			if (ginit->output_val)
				/* output H */
				ret = nxe2000_set_bits(nxe2000->dev,
					NXE2000_GPIO_IOOUT, 1 << i);
			else
				/* output L */
				ret = nxe2000_clr_bits(nxe2000->dev,
					NXE2000_GPIO_IOOUT, 1 << i);
			if (!ret)
				ret = nxe2000_set_bits(nxe2000->dev,
					NXE2000_GPIO_IOSEL, 1 << i);
		} else
			/* GPIO input mode */
			ret = nxe2000_clr_bits(nxe2000->dev,
					NXE2000_GPIO_IOSEL, 1 << i);

		/* if LED function enabled in OTP */
		if (ginit->led_mode) {
			/* LED Mode 1 */
			if (i == 0)	/* GP0 */
				ret = nxe2000_set_bits(nxe2000->dev,
					 NXE2000_GPIO_LED_FUNC,
					 0x04 | (ginit->led_func & 0x03));
			if (i == 1)	/* GP1 */
				ret = nxe2000_set_bits(nxe2000->dev,
					 NXE2000_GPIO_LED_FUNC,
					 0x40 | (ginit->led_func & 0x03) << 4);

		}

		if (ret < 0)
			dev_err(nxe2000->dev, "Gpio %d init "
				"dir configuration failed: %d\n", i, ret);
	}

	nxe2000->gpio_chip.owner		= THIS_MODULE;
	nxe2000->gpio_chip.label		= nxe2000->client->name;
	nxe2000->gpio_chip.dev			= nxe2000->dev;
	nxe2000->gpio_chip.base			= pdata->gpio_base;
	nxe2000->gpio_chip.ngpio		= pdata->num_gpioinit_data;	//NXE2000_NR_GPIO;
	nxe2000->gpio_chip.can_sleep	= 1;

	nxe2000->gpio_chip.direction_input	= nxe2000_gpio_input;
	nxe2000->gpio_chip.direction_output	= nxe2000_gpio_output;
	nxe2000->gpio_chip.set			= nxe2000_gpio_set;
	nxe2000->gpio_chip.get			= nxe2000_gpio_get;
	nxe2000->gpio_chip.to_irq		= nxe2000_gpio_to_irq;

	ret = gpiochip_add(&nxe2000->gpio_chip);
	if (ret)
		dev_warn(nxe2000->dev, "GPIO registration failed: %d\n", ret);
}

static int nxe2000_remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static int nxe2000_remove_subdevs(struct nxe2000 *nxe2000)
{
	return device_for_each_child(nxe2000->dev, NULL,
				     nxe2000_remove_subdev);
}

static int nxe2000_add_subdevs(struct nxe2000 *nxe2000,
				struct nxe2000_platform_data *pdata)
{
	struct nxe2000_subdev_info *subdev;
	struct platform_device *pdev;
	int i, ret = 0;

	for (i = 0; i < pdata->num_subdevs; i++) {
		subdev = &pdata->subdevs[i];

		pdev = platform_device_alloc(subdev->name, subdev->id);

		pdev->dev.parent = nxe2000->dev;
		pdev->dev.platform_data = subdev->platform_data;

		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}
	return 0;

failed:
	nxe2000_remove_subdevs(nxe2000);
	return ret;
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
static void print_regs(const char *header, struct seq_file *s,
		struct i2c_client *client, int start_offset,
		int end_offset)
{
	uint8_t reg_val;
	int i;
	int ret;

	seq_printf(s, "%s\n", header);
	for (i = start_offset; i <= end_offset; ++i) {
		ret = __nxe2000_read(client, i, &reg_val);
		if (ret >= 0)
			seq_printf(s, "Reg 0x%02x Value 0x%02x\n", i, reg_val);
	}
	seq_printf(s, "------------------\n");
}

static int dbg_nxe2000_show(struct seq_file *s, void *unused)
{
	struct nxe2000 *nxe2000 = s->private;
	struct i2c_client *client = nxe2000->client;

	seq_printf(s, "NXE2000 Registers\n");
	seq_printf(s, "------------------\n");

	print_regs("System Regs",		s, client, 0x0, 0x05);
	print_regs("Power Control Regs",	s, client, 0x07, 0x2B);
	print_regs("DCDC  Regs",		s, client, 0x2C, 0x43);
	print_regs("LDO   Regs",		s, client, 0x44, 0x61);
	print_regs("ADC   Regs",		s, client, 0x64, 0x8F);
	print_regs("GPIO  Regs",		s, client, 0x90, 0x98);
	print_regs("INTC  Regs",		s, client, 0x9C, 0x9E);
	print_regs("RTC   Regs",		s, client, 0xA0, 0xAF);
	print_regs("OPT   Regs",		s, client, 0xB0, 0xB1);
	print_regs("CHG   Regs",		s, client, 0xB2, 0xDF);
	print_regs("FUEL  Regs",		s, client, 0xE0, 0xFC);
	return 0;
}

static int dbg_nxe2000_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_nxe2000_show, inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= dbg_nxe2000_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static void nxe2000_debuginit(struct nxe2000 *nxe2000)
{
	(void)debugfs_create_file("nxe2000", S_IRUGO, NULL,
			nxe2000, &debug_fops);
}
#else
static void print_regs(const char *header, struct i2c_client *client,
		int start_offset, int end_offset)
{
	uint8_t reg_val;
	int i;
	int ret;

	printk(KERN_INFO "%s\n", header);
	for (i = start_offset; i <= end_offset; ++i) {
		ret = __nxe2000_read(client, i, &reg_val);
		if (ret >= 0)
			printk(KERN_INFO "Reg 0x%02x Value 0x%02x\n",
							 i, reg_val);
	}
	printk(KERN_INFO "------------------\n");
}
static void nxe2000_debuginit(struct nxe2000 *nxe2000)
{
	struct i2c_client *client = nxe2000->client;

	printk(KERN_INFO "NXE2000 Registers\n");
	printk(KERN_INFO "------------------\n");

	print_regs("System Regs",		client, 0x0, 0x05);
	print_regs("Power Control Regs",	client, 0x07, 0x2B);
	print_regs("DCDC  Regs",		client, 0x2C, 0x43);
	print_regs("LDO   Regs",		client, 0x44, 0x5C);
	print_regs("ADC   Regs",		client, 0x64, 0x8F);
	print_regs("GPIO  Regs",		client, 0x90, 0x9B);
	print_regs("INTC  Regs",		client, 0x9C, 0x9E);
	print_regs("OPT   Regs",		client, 0xB0, 0xB1);
	print_regs("CHG   Regs",		client, 0xB2, 0xDF);
	print_regs("FUEL  Regs",		client, 0xE0, 0xFC);

	return 0;
}
#endif

static void nxe2000_noe_init(struct nxe2000 *nxe2000)
{
#if 0
	struct i2c_client *client = nxe2000->client;

	/* N_OE timer setting to 128mS */
	__nxe2000_write(client, NXE2000_PWR_NOE_TIMSET, 0x0);
	/* Repeat power ON after reset (Power Off/N_OE) */
	__nxe2000_write(client, NXE2000_PWR_REP_CNT, 0x1);
#endif
}

static int nxe2000_i2c_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct nxe2000 *nxe2000;
	struct nxe2000_platform_data *pdata = client->dev.platform_data;
	int ret;

	nxe2000 = kzalloc(sizeof(struct nxe2000), GFP_KERNEL);
	if (nxe2000 == NULL)
		return -ENOMEM;

	nxe2000->client = client;
	nxe2000->dev = &client->dev;
	i2c_set_clientdata(client, nxe2000);

	mutex_init(&nxe2000->io_lock);

	nxe2000->bank_num = 0;

	/* For init PMIC_IRQ port */
//	ret = pdata->init_port(client->irq);

	if (client->irq) {
		nxe2000->irq_base       = pdata->irq_base;
		nxe2000->chip_irq       = gpio_to_irq(client->irq);
		nxe2000->chip_irq_type  = pdata->irq_type;

		ret = nxe2000_irq_init(nxe2000);
		if (ret) {
			dev_err(&client->dev, "IRQ init failed: %d\n", ret);
			goto err_irq_init;
		}
	}

	ret = nxe2000_add_subdevs(nxe2000, pdata);
	if (ret) {
		dev_err(&client->dev, "add devices failed: %d\n", ret);
		goto err_add_devs;
	}

	nxe2000_noe_init(nxe2000);

	nxe2000_gpio_init(nxe2000, pdata);

	nxe2000_debuginit(nxe2000);

#ifdef CONFIG_NXE2000_WDG_TEST
	nxe2000_watchdog_init(nxe2000);
#endif

	nxe2000_i2c_client = client;

	//backup_pm_power_off = pm_power_off;
	//pm_power_off = nxe2000_power_off;
	nxp_board_shutdown = nxe2000_power_off;

#if (NXE2000_PM_RESTART == 1)
	backup_pm_restart = arm_pm_restart;
	arm_pm_restart = nxe2000_restart;
#endif

	return 0;

err_add_devs:
	if (client->irq)
		nxe2000_irq_exit(nxe2000);
err_irq_init:
	kfree(nxe2000);
	return ret;
}

static int  __devexit nxe2000_i2c_remove(struct i2c_client *client)
{
	struct nxe2000 *nxe2000 = i2c_get_clientdata(client);

#ifdef CONFIG_NXE2000_WDG_TEST
	free_irq((IRQ_SYSTEM_END + NXE2000_IRQ_WD), nxe2000);
#endif

	if (client->irq)
		nxe2000_irq_exit(nxe2000);

	nxe2000_remove_subdevs(nxe2000);
	kfree(nxe2000);
	return 0;
}

#ifdef CONFIG_PM
static int nxe2000_i2c_suspend(struct i2c_client *client, pm_message_t state)
{
	if (client->irq)
		disable_irq(client->irq);

	return 0;
}

int pwrkey_wakeup;
static int nxe2000_i2c_resume(struct i2c_client *client)
{
	uint8_t reg_val = 0;
	int ret;

	/* Disable all Interrupt */
	__nxe2000_write(client, NXE2000_INTC_INTEN, 0x0);

	ret = __nxe2000_read(client, NXE2000_INT_IR_SYS, &reg_val);
	if (reg_val & 0x01) { /* If PWR_KEY wakeup */
		pwrkey_wakeup = 1;
		/* Clear PWR_KEY IRQ */
		__nxe2000_write(client, NXE2000_INT_IR_SYS, 0x0);
	}
	enable_irq(client->irq);
	
	/* Enable all Interrupt */
	__nxe2000_write(client, NXE2000_INTC_INTEN, 0xff);

	return 0;
}

#endif

static const struct i2c_device_id nxe2000_i2c_id[] = {
	{"nxe2000", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, nxe2000_i2c_id);

static struct i2c_driver nxe2000_i2c_driver = {
	.driver = {
		   .name = "nxe2000",
		   .owner = THIS_MODULE,
		   },
	.probe = nxe2000_i2c_probe,
	.remove = __devexit_p(nxe2000_i2c_remove),
#ifdef CONFIG_PM
	.suspend = nxe2000_i2c_suspend,
	.resume = nxe2000_i2c_resume,
#endif
	.id_table = nxe2000_i2c_id,
};


static int __init nxe2000_i2c_init(void)
{
	int ret = -ENODEV;

	ret = i2c_add_driver(&nxe2000_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register I2C driver: %d\n", ret);

	return ret;
}

subsys_initcall_sync(nxe2000_i2c_init);

static void __exit nxe2000_i2c_exit(void)
{
	i2c_del_driver(&nxe2000_i2c_driver);
}

module_exit(nxe2000_i2c_exit);

MODULE_DESCRIPTION("NEXELL NXE2000 PMU multi-function core driver");
MODULE_LICENSE("GPL");
