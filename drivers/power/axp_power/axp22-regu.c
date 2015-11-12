/*
 * Regulators driver for X-Powers AXP
 *
 * Copyright (C) 2013 X-Powers, Ltd.
 *  Zhang Donglu <zhangdonglu@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/module.h>
#include <linux/version.h>
#include <mach/platform.h>

#include "axp-regu.h"
#include "axp-cfg.h"

static unsigned int axp_suspend_status;

static inline struct device *to_axp_dev(struct regulator_dev *rdev)
{
	return rdev_get_dev(rdev)->parent->parent;
}

static inline int check_range(struct axp_regulator_info *info,
				int min_uV, int max_uV)
{
	if (min_uV < info->min_uV || min_uV > info->max_uV)
		return -EINVAL;

	return 0;
}


/* AXP common operations */
static int axp_set_voltage(struct regulator_dev *rdev,
				  int min_uV, int max_uV,unsigned *selector)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	uint8_t val, mask;
	int ret = 0;

	if (axp_suspend_status)
		return -EBUSY;	

	if (check_range(info, min_uV, max_uV)) {
		pr_err("invalid voltage range (%d, %d) uV\n", min_uV, max_uV);
		return -EINVAL;
	}
	val = (min_uV - info->min_uV + info->step_uV - 1) / info->step_uV;
	if(selector)
		*selector = val;
	val <<= info->vol_shift;
	mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;

	info->vout_reg_cache = val;

	ret = axp_update(axp_dev, info->vol_reg, val, mask);

#ifdef CONFIG_PM_DBGOUT
	{
		uint8_t reg_val = 0;
		ret = axp_read(axp_dev, info->vol_reg, &reg_val);
		if((info->vout_reg_cache != reg_val) || ret)
			printk(KERN_ERR "## \e[31m%s() Data is different!\e[0m set:0x%02x, read:0x%02x, ret:%d \n", __func__, val, reg_val, ret);
		}
#endif
	return ret;
}


static int axp_get_voltage(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	uint8_t val, mask;
	int ret;

	ret = axp_read(axp_dev, info->vol_reg, &val);
	if (ret)
		return ret;
  
	mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	val = (val & mask) >> info->vol_shift;

	return info->min_uV + info->step_uV * val;
	
}

static int axp_set_voltage_time_sel(struct regulator_dev *rdev,
					     unsigned int old_sel,
					     unsigned int new_sel)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);

	if (old_sel < new_sel)
		return ((new_sel - old_sel) * info->vrc_ramp_delay);

	return 0;
}

static int axp_set_voltage_sel(struct regulator_dev *rdev,
				       unsigned selector)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	int uV;

	if (axp_suspend_status)
		return -EBUSY;

	uV = info->min_uV + (info->step_uV * selector);

	return axp_set_voltage(rdev, uV, uV, NULL);
}
static int axp_get_voltage_sel(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	uint8_t val, vsel, mask;
	int ret;

	ret = axp_read(axp_dev, info->vol_reg, &val);
	if (ret)
		return ret;

	mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	vsel = (val & mask) >> info->vol_shift;

	return vsel;
}

static int axp_enable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	return axp_set_bits(axp_dev, info->enable_reg,
					1 << info->enable_bit);
}

static int axp_disable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	return axp_clr_bits(axp_dev, info->enable_reg,
					1 << info->enable_bit);
}

static int axp_is_enabled(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	uint8_t reg_val;
	int ret;

	ret = axp_read(axp_dev, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	return !!(reg_val & (1 << info->enable_bit));
}

static int axp_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	ret = info->min_uV + info->step_uV * selector;

	if (ret > info->max_uV)
		return -EINVAL;
	return ret;
}

static int axp_set_suspend_voltage(struct regulator_dev *rdev, int uV)
{
	return axp_set_voltage(rdev, uV, uV,NULL);
}


static struct regulator_ops axp22_dcdc_ops = {
	.set_voltage_sel	= axp_set_voltage_sel,
	.set_voltage_time_sel = axp_set_voltage_time_sel,
	.get_voltage_sel	= axp_get_voltage_sel,
	.list_voltage	= axp_list_voltage,
	.enable		= axp_enable,
	.disable	= axp_disable,
	.is_enabled	= axp_is_enabled,
	.set_suspend_enable		= axp_enable,
	.set_suspend_disable	= axp_disable,
	.set_suspend_voltage	= axp_set_suspend_voltage,
};


static struct regulator_ops axp22_ops = {
	.set_voltage	= axp_set_voltage,
	.get_voltage	= axp_get_voltage,
	.list_voltage	= axp_list_voltage,
	.enable		= axp_enable,
	.disable	= axp_disable,
	.is_enabled	= axp_is_enabled,
	.set_suspend_enable		= axp_enable,
	.set_suspend_disable	= axp_disable,
	.set_suspend_voltage	= axp_set_suspend_voltage,
};


static int axp_ldoio01_enable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	 axp_set_bits(axp_dev, info->enable_reg,0x03);
	 return axp_clr_bits(axp_dev, info->enable_reg,0x04);
}

static int axp_ldoio01_disable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	return axp_clr_bits(axp_dev, info->enable_reg,0x07);
}

static int axp_ldoio01_is_enabled(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	uint8_t reg_val;
	int ret;

	ret = axp_read(axp_dev, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	return (((reg_val &= 0x07)== 0x03)?1:0);
}

static struct regulator_ops axp22_ldoio01_ops = {
	.set_voltage	= axp_set_voltage,
	.get_voltage	= axp_get_voltage,
	.list_voltage	= axp_list_voltage,
	.enable		= axp_ldoio01_enable,
	.disable	= axp_ldoio01_disable,
	.is_enabled	= axp_ldoio01_is_enabled,
	.set_suspend_enable		= axp_ldoio01_enable,
	.set_suspend_disable	= axp_ldoio01_disable,
	.set_suspend_voltage	= axp_set_suspend_voltage,
};


#define AXP22_LDO(_id, min, max, step, vreg, shift, nbits, ereg, ebit, vrc_ramp)	\
	AXP_LDO(AXP22, _id, min, max, step, vreg, shift, nbits, ereg, ebit, vrc_ramp)

#define AXP22_DCDC(_id, min, max, step, vreg, shift, nbits, ereg, ebit, vrc_ramp)	\
	AXP_DCDC(AXP22, _id, min, max, step, vreg, shift, nbits, ereg, ebit, vrc_ramp)

static struct axp_regulator_info axp_regulator_info[] = {
	AXP22_LDO(  1, 3000,	3000,   0, LDO1,   0, 0,	LDO1EN,   0, 0),//ldo1 for rtc
	AXP22_LDO(  2,  700,	3300, 100, LDO2,   0,	 5,	LDO2EN,   6, 0),//ldo2 for aldo1 
	AXP22_LDO(  3,  700,	3300, 100, LDO3,   0,	 5,	LDO3EN,   7, 0),//ldo3 for aldo2
	AXP22_LDO(  4,  700,	3300, 100, LDO4,   0,	 5,	LDO4EN,   7, 0),//ldo3 for aldo3
	AXP22_LDO(  5,  700,	3300, 100, LDO5,   0,	 5,	LDO5EN,   3, 0),//ldo5 for dldo1
	AXP22_LDO(  6,  700,	3300, 100, LDO6,   0,	 5,	LDO6EN,   4, 0),//ldo6 for dldo2
	AXP22_LDO(  7,  700,	3300, 100, LDO7,   0,	 5,	LDO7EN,   5, 0),//ldo7 for dldo3
	AXP22_LDO(  8,  700,	3300, 100, LDO8,   0,	 5,	LDO8EN,   6, 0),//ldo8 for dldo4
	AXP22_LDO(  9,  700,	3300, 100, LDO9,   0,	 5,	LDO9EN,   0, 0),//ldo9 for eldo1
	AXP22_LDO( 10,  700,	3300, 100, LDO10,  0,	 5,	LDO10EN,  1, 0),//ldo10 for eldo2 
	AXP22_LDO( 11,  700,	3300, 100, LDO11,  0,	 5,	LDO11EN,  2, 0),//ldo11 for eldo3
	AXP22_LDO( 12,  700,	3300, 100, LDO12,  0,	 3,	LDO12EN,  0, 0),//ldo12 for dc5ldo
	AXP22_DCDC( 1, 1600,	3400, 100, DCDC1,  0,	 5,	DCDC1EN,  1, 0),//buck1 for io
	AXP22_DCDC( 2,  600,	1540,  20, DCDC2,  0, 6,	DCDC2EN,  2, 16),//buck2 for cpu
	AXP22_DCDC( 3,  600,	1860,  20, DCDC3,  0, 6,	DCDC3EN,  3, 16),//buck3 for gpu
	AXP22_DCDC( 4,  600,	1540,  20, DCDC4,  0, 6,	DCDC4EN,  4, 0),//buck4 for core
	AXP22_DCDC( 5, 1000,	2550,  50, DCDC5,  0, 5,	DCDC5EN,  5, 0),//buck5 for ddr
	AXP22_LDO(IO0,  700,	3300, 100, LDOIO0, 0,	 5,	LDOIO0EN, 0, 0),//ldoio0 
	AXP22_LDO(IO1,  700,	3300, 100, LDOIO1, 0,	 5,	LDOIO1EN, 0, 0),//ldoio1
};

static ssize_t workmode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	int ret;
	uint8_t val;
	ret = axp_read(axp_dev, AXP22_BUCKMODE, &val);
	if (ret)
		return sprintf(buf, "IO ERROR\n");
	
	if(info->desc.id == AXP22_ID_DCDC1){
		switch (val & 0x04) {
			case 0:return sprintf(buf, "AUTO\n");
			case 4:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	}
	else if(info->desc.id == AXP22_ID_DCDC2){
		switch (val & 0x02) {
			case 0:return sprintf(buf, "AUTO\n");
			case 2:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	}
	else if(info->desc.id == AXP22_ID_DCDC3){
		switch (val & 0x02) {
			case 0:return sprintf(buf, "AUTO\n");
			case 2:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	}
	else if(info->desc.id == AXP22_ID_DCDC4){
		switch (val & 0x02) {
			case 0:return sprintf(buf, "AUTO\n");
			case 2:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	}
	else if(info->desc.id == AXP22_ID_DCDC5){
		switch (val & 0x02) {
			case 0:return sprintf(buf, "AUTO\n");
			case 2:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	}
	else
		return sprintf(buf, "IO ID ERROR\n");
}

static ssize_t workmode_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{	
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	char mode;
	uint8_t val;
	if(  buf[0] > '0' && buf[0] < '9' )// 1/AUTO: auto mode; 2/PWM: pwm mode;
		mode = buf[0];
	else
		mode = buf[1];
	
	switch(mode){
	 case 'U':
	 case 'u':
	 case '1':
		val = 0;break;
	 case 'W':
	 case 'w':
	 case '2':
	 	val = 1;break;
	 default:
	    val =0;	
	}
	
	if(info->desc.id == AXP22_ID_DCDC1){
		if(val)
			axp_set_bits(axp_dev, AXP22_BUCKMODE,0x01);
		else
			axp_clr_bits(axp_dev, AXP22_BUCKMODE,0x01);
	}
	else if(info->desc.id == AXP22_ID_DCDC2){
		if(val)
			axp_set_bits(axp_dev, AXP22_BUCKMODE,0x02);
		else
			axp_clr_bits(axp_dev, AXP22_BUCKMODE,0x02);
	}
	else if(info->desc.id == AXP22_ID_DCDC3){
		if(val)
			axp_set_bits(axp_dev, AXP22_BUCKMODE,0x04);
		else
			axp_clr_bits(axp_dev, AXP22_BUCKMODE,0x04);
	}
	else if(info->desc.id == AXP22_ID_DCDC4){
		if(val)
			axp_set_bits(axp_dev, AXP22_BUCKMODE,0x08);
		else
			axp_clr_bits(axp_dev, AXP22_BUCKMODE,0x08);
	}
	else if(info->desc.id == AXP22_ID_DCDC5){
		if(val)
			axp_set_bits(axp_dev, AXP22_BUCKMODE,0x10);
		else
			axp_clr_bits(axp_dev, AXP22_BUCKMODE,0x10);
	}
	return count;
}

static ssize_t frequency_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct device *axp_dev = to_axp_dev(rdev);
	int ret;
	uint8_t val;
	ret = axp_read(axp_dev, AXP22_BUCKFREQ, &val);
	if (ret)
		return ret;
	ret = val & 0x0F;
	return sprintf(buf, "%d\n",(ret*75 + 750));
}

static ssize_t frequency_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{	
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct device *axp_dev = to_axp_dev(rdev);
	uint8_t val,tmp;
	int var;
	var = simple_strtoul(buf, NULL, 10);
	if(var < 750)
		var = 750;
	if(var > 1875)
		var = 1875;
		
	val = (var -750)/75;
	val &= 0x0F;
	
	axp_read(axp_dev, AXP22_BUCKFREQ, &tmp);
	tmp &= 0xF0;
	val |= tmp;
	axp_write(axp_dev, AXP22_BUCKFREQ, val);
	return count;
}


static struct device_attribute axp_regu_attrs[] = {
	AXP_REGU_ATTR(workmode),
	AXP_REGU_ATTR(frequency),
};

int axp_regu_create_attrs(struct platform_device *pdev)
{
	int j,ret;
	for (j = 0; j < ARRAY_SIZE(axp_regu_attrs); j++) {
		ret = device_create_file(&pdev->dev,&axp_regu_attrs[j]);
		if (ret)
			goto sysfs_failed;
	}
    goto succeed;
	
sysfs_failed:
	while (j--)
		device_remove_file(&pdev->dev,&axp_regu_attrs[j]);
succeed:
	return ret;
}

static inline struct axp_regulator_info *find_regulator_info(int id)
{
	struct axp_regulator_info *ri;
	int i;

	for (i = 0; i < ARRAY_SIZE(axp_regulator_info); i++) {
		ri = &axp_regulator_info[i];
		if (ri->desc.id == id)
			return ri;
	}
	return NULL;
}

static inline int axp_set_sleep_mod(struct regulator_dev *rdev)
{
	//Shut down DCDC5 ALDO123 when in sleep.
	struct device *axp_dev = to_axp_dev(rdev);
	axp_set_bits(axp_dev, 0x8C,0x0f);
	//axp_set_bits(axp_dev, 0x8D,0x01);
	axp_set_bits(axp_dev, 0x92,0x07);
	axp_set_bits(axp_dev, 0x100,0x04);
	printk("[AXP228] :Set PMIC sleep mode  \n");
	return 0;
}

static int __devinit axp_regulator_probe(struct platform_device *pdev)
{
	struct axp_regulator_info *ri = NULL;
	struct regulator_dev *rdev;
	int ret;

	ri = find_regulator_info(pdev->id);
	if (ri == NULL) {
		dev_err(&pdev->dev, "invalid regulator ID specified\n");
		return -EINVAL;
	}

	if (ri->desc.id == AXP22_ID_LDO1 || ri->desc.id == AXP22_ID_LDO2 \
		|| ri->desc.id == AXP22_ID_LDO3 || ri->desc.id == AXP22_ID_LDO4 \
		|| ri->desc.id == AXP22_ID_LDO5 || ri->desc.id == AXP22_ID_LDO6 \
		|| ri->desc.id == AXP22_ID_LDO7 || ri->desc.id == AXP22_ID_LDO8 \
		|| ri->desc.id == AXP22_ID_LDO9 || ri->desc.id == AXP22_ID_LDO10 \
		|| ri->desc.id == AXP22_ID_LDO11 || ri->desc.id == AXP22_ID_LDO12 \
		|| ri->desc.id == AXP22_ID_DCDC1 ||ri->desc.id == AXP22_ID_DCDC4 \
		|| ri->desc.id == AXP22_ID_DCDC5)
		ri->desc.ops = &axp22_ops;

	if (ri->desc.id == AXP22_ID_LDOIO0|| ri->desc.id == AXP22_ID_LDOIO1 )
		ri->desc.ops = &axp22_ldoio01_ops;

	if (ri->desc.id == AXP22_ID_DCDC2|| ri->desc.id == AXP22_ID_DCDC3 )
		ri->desc.ops = &axp22_dcdc_ops;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
        rdev = regulator_register(&ri->desc, &pdev->dev, pdev->dev.platform_data, ri);
#else
	rdev = regulator_register(&ri->desc, &pdev->dev, pdev->dev.platform_data, ri, NULL);
#endif

	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "failed to register regulator %s\n",
				ri->desc.name);
		return PTR_ERR(rdev);
	}
	platform_set_drvdata(pdev, rdev);

	//axp_set_sleep_mod(rdev);
	
	if(ri->desc.id == AXP22_ID_DCDC1 ||ri->desc.id == AXP22_ID_DCDC2 \
		|| ri->desc.id == AXP22_ID_DCDC3 ||ri->desc.id == AXP22_ID_DCDC4 \
		|| ri->desc.id == AXP22_ID_DCDC5){
		ret = axp_regu_create_attrs(pdev);
		if(ret){
			return ret;
		}
	}

	return 0;
}

static int __devexit axp_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

	regulator_unregister(rdev);
	return 0;
}

#ifdef CONFIG_PM
static int axp_regulator_suspend(struct device *dev)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct regulation_constraints *constraints = rdev->constraints;
	struct regulator_state *state_standby = &constraints->state_standby;
	int ret = 0;

	axp_suspend_status = 0;

	if(!strcmp(constraints->name, "axp22_dcdc2"))
	{
		if(state_standby)
			ret = axp_set_voltage(rdev, AXP_DCDC2_VALUE, AXP_DCDC2_VALUE,NULL);
	}

	axp_suspend_status = 1;
	return ret;
}

static int axp_regulator_resume(struct device *dev)
{
	axp_suspend_status = 0;

	return 0;
}

static const struct dev_pm_ops axp_regulator_pm_ops = {
	.suspend	= axp_regulator_suspend,
	.resume		= axp_regulator_resume,
};
#endif

static struct platform_driver axp_regulator_driver = {
	.driver	= {
		.name	= "axp22-regulator",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &axp_regulator_pm_ops,
#endif
	},
	.probe		= axp_regulator_probe,
	.remove		= axp_regulator_remove,
};

static int __init axp_regulator_init(void)
{
	return platform_driver_register(&axp_regulator_driver);
}
subsys_initcall(axp_regulator_init);

static void __exit axp_regulator_exit(void)
{
	platform_driver_unregister(&axp_regulator_driver);
}
module_exit(axp_regulator_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("King Zhong");
MODULE_DESCRIPTION("Regulator Driver for X-powers AXP22 PMIC");
MODULE_ALIAS("platform:axp-regulator");
