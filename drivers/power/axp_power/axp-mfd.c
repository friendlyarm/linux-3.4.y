/*
 * Base driver for X-Powers AXP
 *
 * Copyright (C) 2013 X-Powers, Ltd.
 *  Zhang Donglu <zhangdonglu@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <mach/pm.h>
//#include <mach/system.h>

//#include "axp-cfg.h"
#include "axp22-mfd.h"

#ifdef ENABLE_DEBUG
#define DBG_MSG(format,args...)   printk(KERN_ERR format,##args)
#else
#define DBG_MSG(format,args...)   do {} while (0)
#endif

struct axp_mfd_chip *g_chip;

int axp_otg_power_control(int enable)
{
	DBG_MSG("## [\e[31m%s\e[0m():%d] enable:%d\n", __func__, __LINE__, enable);

	if (enable)
	{
		if (CFG_GPIO_OTG_VBUS_DET > -1)
			gpio_set_value(CFG_GPIO_OTG_VBUS_DET, 1);
	}
	else
	{
		if (CFG_GPIO_OTG_VBUS_DET > -1)
			gpio_set_value(CFG_GPIO_OTG_VBUS_DET, 0);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(axp_otg_power_control);

void axp_run_irq_handler(void)
{
	DBG_MSG("## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
	(void)schedule_work(&g_chip->irq_work);
	return;
}
EXPORT_SYMBOL_GPL(axp_run_irq_handler);

static void axp_mfd_register_dump(struct device *dev)
{
	int ret=0;
	u16 i=0;
	u8 value=0;

	printk("##########################################################\n");
	printk("##\e[31m %s()\e[0m                               #\n", __func__);
	printk("##########################################################\n");
	printk("##      0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F\n");

	for(i=0; i<=0xff; i++)
	{
		if(i%16 == 0)
			printk("## %02X:", i);

		if(i%4 == 0)
			printk(" ");

		ret = axp_read(dev, i, &value);
		if(!ret)
			printk("%02x ", value);
		else
			printk("\e[31mxx\e[0m ");

		if((i+1)%16 == 0)
			printk("\n");
	}
	printk("##########################################################\n");
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int axp_dbg_show(struct seq_file *s, void *unused)
{
	struct axp_mfd_chip *chip = s->private;
	struct device *dev = chip->dev;

	axp_mfd_register_dump(dev);

	return 0;
}

static int axp_dbg_open(struct inode *inode, struct file *file)
{
	return single_open(file, axp_dbg_show, inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= axp_dbg_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static void axp_debuginit(struct axp_mfd_chip *chip)
{
	(void)debugfs_create_file("axp228", S_IRUGO, NULL,
			chip, &debug_fops);
}
#else
static void axp_debuginit(struct axp_mfd_chip *chip)
{
	struct device *dev = chip->dev;

	axp_mfd_register_dump(dev);
	return 0;
}
#endif

static void axp_mfd_irq_work(struct work_struct *work)
{
	struct axp_mfd_chip *chip = container_of(work, struct axp_mfd_chip, irq_work);
	uint64_t irqs = 0;

	DBG_MSG("## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);

	while (1) {
		if (chip->ops->read_irqs(chip, &irqs)){
			printk("read irq fail\n");
			break;
		}
		irqs &= chip->irqs_enabled;
		if (irqs == 0){
			break;
		}
		
		if(irqs > 0xffffffff){
			blocking_notifier_call_chain(&chip->notifier_list, (uint32_t)(irqs>>32), (void *)1);
		}
		else{
			blocking_notifier_call_chain(&chip->notifier_list, (uint32_t)irqs, (void *)0);
		}
	}
	//enable_irq(chip->client->irq);
}

static irqreturn_t axp_mfd_irq_handler(int irq, void *data)
{
	struct axp_mfd_chip *chip = data;

	DBG_MSG("## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);

	//disable_irq_nosync(irq);
	(void)schedule_work(&chip->irq_work);

	return IRQ_HANDLED;
}

static struct axp_mfd_chip_ops axp_mfd_ops[] = {
	[0] = {
		.init_chip    = axp22_init_chip,
		.enable_irqs  = axp22_enable_irqs,
		.disable_irqs = axp22_disable_irqs,
		.read_irqs    = axp22_read_irqs,
	},
};

static const struct i2c_device_id axp_mfd_id_table[] = {
	{ "axp22_mfd", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, axp_mfd_id_table);

int axp_mfd_create_attrs(struct axp_mfd_chip *chip)
{
	int j,ret;
	if (chip->type ==  AXP22){
		for (j = 0; j < ARRAY_SIZE(axp22_mfd_attrs); j++) {
			ret = device_create_file(chip->dev,&axp22_mfd_attrs[j]);
			if (ret)
			goto sysfs_failed;
		}
	}
	else
		ret = 0;
	goto succeed;

sysfs_failed:
	while (j--)
		device_remove_file(chip->dev,&axp22_mfd_attrs[j]);
succeed:
	return ret;
}

static int __remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static int axp_mfd_remove_subdevs(struct axp_mfd_chip *chip)
{
	return device_for_each_child(chip->dev, NULL, __remove_subdev);
}

static int __devinit axp_mfd_add_subdevs(struct axp_mfd_chip *chip,
					struct axp_platform_data *pdata)
{
	struct axp_funcdev_info *regl_dev;
	struct axp_funcdev_info *sply_dev;
	struct axp_funcdev_info *gpio_dev;
	struct platform_device *pdev;
	int i, ret = 0;

	/* register for regultors */
	for (i = 0; i < pdata->num_regl_devs; i++) {
		regl_dev = &pdata->regl_devs[i];
		pdev = platform_device_alloc(regl_dev->name, regl_dev->id);
		pdev->dev.parent = chip->dev;
		pdev->dev.platform_data = regl_dev->platform_data;
		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}

	/* register for power supply */
	for (i = 0; i < pdata->num_sply_devs; i++) {
		sply_dev = &pdata->sply_devs[i];
		pdev = platform_device_alloc(sply_dev->name, sply_dev->id);
		pdev->dev.parent = chip->dev;
		pdev->dev.platform_data = sply_dev->platform_data;
		ret = platform_device_add(pdev);
		if (ret)
			goto failed;

	}

	/* register for gpio */
	for (i = 0; i < pdata->num_gpio_devs; i++) {
		gpio_dev = &pdata->gpio_devs[i];
		pdev = platform_device_alloc(gpio_dev->name, gpio_dev->id);
		pdev->dev.parent = chip->dev;
		pdev->dev.platform_data = gpio_dev->platform_data;
		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}

	return 0;

failed:
	axp_mfd_remove_subdevs(chip);
	return ret;
}

static void axp_power_off(void)
{
	uint8_t val;
	int ret = 0;

#if defined (CONFIG_KP_AXP22)
	if(SHUTDOWNVOL >= 2600 && SHUTDOWNVOL <= 3300)
	{
		if (SHUTDOWNVOL > 3200){
			val = 0x7;
		}
		else if (SHUTDOWNVOL > 3100){
			val = 0x6;
		}
		else if (SHUTDOWNVOL > 3000){
			val = 0x5;
		}
		else if (SHUTDOWNVOL > 2900){
			val = 0x4;
		}
		else if (SHUTDOWNVOL > 2800){
			val = 0x3;
		}
		else if (SHUTDOWNVOL > 2700){
			val = 0x2;
		}
		else if (SHUTDOWNVOL > 2600){
			val = 0x1;
		}
		else
			val = 0x0;

		axp_update(&axp->dev, AXP22_VOFF_SET, val, 0x7);
	}

	val = 0xff;

    printk("[axp] send power-off command!\n");

    mdelay(20);

    if(POWER_START != 1)
	{
		axp_read(&axp->dev, AXP22_STATUS, &val);
		if(val & 0xF0)
		{
	    	axp_read(&axp->dev, AXP22_MODE_CHGSTATUS, &val);
	    	if(val & 0x20)
			{
            	//printk("[axp] set flag!\n");
	        	axp_write(&axp->dev, AXP22_BUFFERC, 0x0f);
            	mdelay(20);
		    	//printk("[axp] reboot!\n");
		    	//arch_reset(0,NULL);
		    	//printk("[axp] warning!!! arch can't ,reboot, maybe some error happend!\n");
	    	}
		}
	}
    axp_write(&axp->dev, AXP22_BUFFERC, 0x00);
    mdelay(20);
    ret = axp_set_bits(&axp->dev, AXP22_OFF_CTL, 0x80);
    if(ret < 0){
        printk("[axp] power-off cmd error!, retry!");
        ret = axp_set_bits(&axp->dev, AXP22_OFF_CTL, 0x80);
    }
    //mdelay(20);
    //printk("[axp] warning!!! axp can't power-off, maybe some error happend!\n");
#endif
}

static int __devinit axp_mfd_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct axp_platform_data *pdata = client->dev.platform_data;
	struct axp_mfd_chip *chip;
	int ret;

	DBG_MSG("## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);

	chip = kzalloc(sizeof(struct axp_mfd_chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;

	axp = client;

	chip->client = client;
	chip->dev = &client->dev;
	chip->ops = &axp_mfd_ops[id->driver_data];

	mutex_init(&chip->lock);
	INIT_WORK(&chip->irq_work, axp_mfd_irq_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&chip->notifier_list);

	i2c_set_clientdata(client, chip);

#ifdef ENABLE_DEBUG
	axp_mfd_register_dump(chip->dev);
#endif

	axp_debuginit(chip);

	ret = chip->ops->init_chip(chip);
	if (ret)
		goto out_free_chip;

	g_chip = chip;

#if 1
	ret = request_threaded_irq(gpio_to_irq(client->irq), NULL, 
								axp_mfd_irq_handler,
								IRQ_TYPE_EDGE_FALLING|IRQF_DISABLED|IRQF_ONESHOT,
								"axp_mfd", chip);
#else
	ret = request_irq(client->irq, axp_mfd_irq_handler,
		IRQF_SHARED|IRQF_DISABLED, "axp_mfd", chip);
#endif

  	if (ret) {
  		dev_err(&client->dev, "failed to request irq %d\n",
  				client->irq);
  		goto out_free_chip;
  	}

	ret = axp_mfd_add_subdevs(chip, pdata);
	if (ret)
		goto out_free_irq;

	/* PM hookup */
#if 1
	if(!nxp_board_shutdown)
		nxp_board_shutdown = axp_power_off;
#else
	if(!pm_power_off)
		pm_power_off = axp_power_off;
#endif

	ret = axp_mfd_create_attrs(chip);
	if(ret){
		return ret;
	}
	return 0;

out_free_irq:
	free_irq(client->irq, chip);

out_free_chip:
	i2c_set_clientdata(client, NULL);
	kfree(chip);

	return ret;
}

static int __devexit axp_mfd_remove(struct i2c_client *client)
{
	struct axp_mfd_chip *chip = i2c_get_clientdata(client);

	//pm_power_off = NULL;
	//axp = NULL;

	axp_mfd_remove_subdevs(chip);
	kfree(chip);
	return 0;
}

static struct i2c_driver axp_mfd_driver = {
	.driver	= {
		.name	= "axp_mfd",
		.owner	= THIS_MODULE,
	},
	.probe		= axp_mfd_probe,
	.remove		= __devexit_p(axp_mfd_remove),
	.id_table	= axp_mfd_id_table,
};

static int __init axp_mfd_init(void)
{
	return i2c_add_driver(&axp_mfd_driver);
}
subsys_initcall(axp_mfd_init);

static void __exit axp_mfd_exit(void)
{
	i2c_del_driver(&axp_mfd_driver);
}
module_exit(axp_mfd_exit);

MODULE_DESCRIPTION("MFD Driver for X-Powers AXP PMIC");
MODULE_AUTHOR("Weijin Zhong X-POWERS");
MODULE_LICENSE("GPL");
