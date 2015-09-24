/*
* Copyright (C) 2012 Invensense, Inc.
* Copyright (C) 2012 Samsung Electronics Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/power_supply.h>
#include <linux/debugfs.h>
#include <linux/math64.h>

#include "../w1/w1.h"
#include "../w1/slaves/w1_ds2784.h"

struct fuelgauge_status {
	int timestamp;

	int voltage_uV;		/* units of uV */
	int current_uA;		/* units of uA */
	int charge_uAh;

	short temp_C;		/* units of 0.1 C */

	u8 percentage;		/* battery percentage */
	u8 charge_source;
	u8 status_reg;
	u8 battery_full;	/* battery full (don't charge) */

	u8 cooldown;		/* was overtemp */
	u8 charge_mode;
};

struct ds2784_info {
	struct device			*dev;
	struct device			*w1_dev;
	struct power_supply		bat;
	struct delayed_work		work;
	char				raw[DS2784_DATA_SIZE];
	struct fuelgauge_status		status;
	bool				inited;
	struct dentry			*dentry;
};

static int ds2784_read(struct ds2784_info *di, char *buf, int addr,
		       size_t count)
{
	return w1_ds2784_read(di->w1_dev, buf, addr, count);
}

static int ds2784_get_soc(struct ds2784_info *di, int *soc)
{
	int ret;

	ret = ds2784_read(di, di->raw + DS2784_REG_RARC, DS2784_REG_RARC, 1);

	if (ret < 0)
		return ret;

	di->status.percentage =	di->raw[DS2784_REG_RARC];
	pr_debug("%s: level : %d\n", __func__, di->status.percentage);
	*soc = di->status.percentage;
	return 0;
}

static int ds2784_get_vcell(struct ds2784_info *di, int *vcell)
{
	short n;
	int ret;

	ret = ds2784_read(di, di->raw + DS2784_REG_VOLT_MSB,
			  DS2784_REG_VOLT_MSB, 2);

	if (ret < 0)
		return ret;

	n = (((di->raw[DS2784_REG_VOLT_MSB] << 8) |
	      (di->raw[DS2784_REG_VOLT_LSB])) >> 5);
	di->status.voltage_uV = n * 4886;
	pr_debug("%s: voltage : %d\n", __func__, di->status.voltage_uV);
	*vcell = di->status.voltage_uV;
	return 0;
}

static int ds2784_get_current(struct ds2784_info *di, bool avg, int *ival)
{
	int reg = avg ? DS2784_REG_AVG_CURR_MSB : DS2784_REG_CURR_MSB;
	short n;
	int ret;
	int div_rsnsp;

	if (!di->raw[DS2784_REG_RSNSP]) {
		ret = ds2784_read(di, di->raw + DS2784_REG_RSNSP,
				  DS2784_REG_RSNSP, 1);
		if (ret < 0)
			dev_err(di->dev, "error %d reading RSNSP\n", ret);
	}
	div_rsnsp = 10000 / di->raw[DS2784_REG_RSNSP];

	ret = ds2784_read(di, di->raw + reg, reg, 2);
	if (ret < 0)
		return ret;

	n = ((di->raw[reg] << 8) | (di->raw[reg+1]));

	*ival = div_s64((long long)n * 15625, div_rsnsp);
	return 0;
}

static int ds2784_get_current_now(struct ds2784_info *di, int *i_current)
{
	return ds2784_get_current(di, false, i_current);
}

static int ds2784_get_current_avg(struct ds2784_info *di, int *i_avg)
{
	return ds2784_get_current(di, true, i_avg);
}

static int ds2784_get_temperature(struct ds2784_info *di, int *temp_now)
{
	short n;
	int ret;

	ret = ds2784_read(di, di->raw + DS2784_REG_TEMP_MSB,
			  DS2784_REG_TEMP_MSB, 2);

	if (ret < 0)
		return ret;

	n = (((di->raw[DS2784_REG_TEMP_MSB] << 8) |
			(di->raw[DS2784_REG_TEMP_LSB])) >> 5);

	if (di->raw[DS2784_REG_TEMP_MSB] & (1 << 7))
		n |= 0xf800;

	di->status.temp_C = (n * 10) / 8;
	pr_debug("%s: temp : %d\n", __func__, di->status.temp_C);

	*temp_now = di->status.temp_C;
	return 0;
}

static int ds2784_get_accumulated_current(struct ds2784_info *di, int *acc)
{
	int n;
	int ret;
	int div_rsnsp;

	if (!di->raw[DS2784_REG_RSNSP]) {
		ret = ds2784_read(di, di->raw + DS2784_REG_RSNSP,
				  DS2784_REG_RSNSP, 1);
		if (ret < 0) {
			dev_err(di->dev, "error %d reading RSNSP\n", ret);
			return ret;
		}
	}
	div_rsnsp = 100 / di->raw[DS2784_REG_RSNSP];

	ret = ds2784_read(di, di->raw + DS2784_REG_ACCUMULATE_CURR_MSB,
			  DS2784_REG_ACCUMULATE_CURR_MSB, 2);

	if (ret < 0)
		return ret;

	n = (di->raw[DS2784_REG_ACCUMULATE_CURR_MSB] << 8) |
		di->raw[DS2784_REG_ACCUMULATE_CURR_LSB];
	*acc = n * 625 / div_rsnsp;
	return 0;
}

static int ds2784_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	int ret = 0;
	struct ds2784_info *di = container_of(psy, struct ds2784_info, bat);

	if (!di->inited)
		return -ENODEV;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		ret = ds2784_get_vcell(di, &val->intval);
		break;

	case POWER_SUPPLY_PROP_TEMP:
		ret = ds2784_get_temperature(di, &val->intval);
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = "DS2784";
		break;

	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = "Maxim/Dallas";
		break;

	case POWER_SUPPLY_PROP_CURRENT_NOW:
		ret = ds2784_get_current_now(di, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CURRENT_AVG:
		ret = ds2784_get_current_avg(di, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		ret = ds2784_get_soc(di, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
		ret = ds2784_get_accumulated_current(di, &val->intval);
		break;

	default:
		ret = -EINVAL;
	}

	return ret;
}

static enum power_supply_property ds2784_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
};

static int ds2784_debugfs_show(struct seq_file *s, void *unused)
{
	struct ds2784_info *di = s->private;
	u8 reg;

	ds2784_read(di, di->raw, 0x00, 0x1C);
	ds2784_read(di, di->raw + 0x20, 0x20, 0x10);
	ds2784_read(di, di->raw + 0x60, 0x60, 0x20);
	ds2784_read(di, di->raw + 0xb0, 0xb0, 0x02);

	for (reg = 0x0; reg <= 0xb1; reg++) {
		if ((reg >= 0x1c && reg <= 0x1f) ||
			(reg >= 0x38 && reg <= 0x5f) ||
			(reg >= 0x90 && reg <= 0xaf))
				continue;

		if (!(reg & 0x7))
			seq_printf(s, "\n0x%02x:", reg);

		seq_printf(s, "\t0x%02x", di->raw[reg]);
	}
	seq_printf(s, "\n");
	return 0;
}

static int ds2784_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, ds2784_debugfs_show, inode->i_private);
}

static const struct file_operations ds2784_debugfs_fops = {
	.open = ds2784_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __devinit ds2784_probe(struct platform_device *pdev)
{
	struct ds2784_info *di;
	int ret;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		pr_err("%s:failed to allocate memory for module data\n",
			__func__);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, di);
	di->dev			= &pdev->dev;
	di->w1_dev		= pdev->dev.parent;
	di->bat.name		= dev_name(&pdev->dev);
	di->bat.type		= POWER_SUPPLY_TYPE_BATTERY;
	di->bat.properties	= ds2784_props;
	di->bat.num_properties	= ARRAY_SIZE(ds2784_props);
	di->bat.get_property	= ds2784_get_property;

	ret = power_supply_register(&pdev->dev, &di->bat);
	if (ret) {
		dev_err(di->dev, "failed to register battery power supply\n");
		kfree(di);
		return ret;
	}

	di->dentry = debugfs_create_file("ds2784", S_IRUGO, NULL, di,
					 &ds2784_debugfs_fops);
	di->inited = true;
	return 0;
}

static int __devexit ds2784_remove(struct platform_device *pdev)
{
	struct ds2784_info *di = platform_get_drvdata(pdev);

	power_supply_unregister(&di->bat);
	debugfs_remove(di->dentry);
	kfree(di);
	return 0;
}

static struct platform_driver ds2784_driver = {
	.probe = ds2784_probe,
	.remove   = __devexit_p(ds2784_remove),
	.driver = {
		.owner = THIS_MODULE,
		.name = "ds2784-fuelgauge",
	},
};

module_platform_driver(ds2784_driver);

MODULE_AUTHOR("Samsung");
MODULE_DESCRIPTION("ds2784 driver");
MODULE_LICENSE("GPL");
