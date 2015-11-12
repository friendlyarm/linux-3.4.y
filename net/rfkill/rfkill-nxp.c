/*
 * (C) Copyright 2010
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
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

#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/rfkill.h>

#include <mach/platform.h>
#include <mach/devices.h>

/*
#define	pr_debug	printk
*/

#if defined CONFIG_MODULES
#define	MODULE_IN	 MODULE_STATE_COMING	/* (1): first insmod */
#define	MODULE_LIV	 MODULE_STATE_LIVE		/* (0): after insmod */
#define	MODULE_DEL	 MODULE_STATE_GOING		/* (2): rmmod  */
#else
#define	MODULE_IN	 1						/* (1): first insmod */
#define	MODULE_LIV	 0						/* (0): after insmod */
#define	MODULE_DEL	 2						/* (2): rmmod  */
#endif

#define	NOT_GPIO	(-1)

static const char *gpio_name[] = { "GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "ALIVE" };
#define	GPIO_GROUP(n)	(gpio_name[n/32])
#define	GPIO_BITNR(n)	(n&0x1F)

struct nxp_rfkill_dev {
	char *name;
	/* rfkill config */
	int power_supply;		/* set 1 if vcc power, set 0 if gpio power */
	char supply_name[32];
	char module_name[32];
    int gpio;
    int initval;
    int invert;			/* 0: high active, 1: low active */
    int delay_time_on;
    int delay_time_off;
    /* rfkill satus */
    int enable;
    struct regulator *vcc;
    struct notifier_block nb;
	struct list_head link;
	struct delayed_work work;
	bool run_dealy_work;
};

struct nxp_rfkill_data {
	struct rfkill *rfkill;
    char *name;             /* the name for the rfkill switch */
    enum rfkill_type type;  /* the type as specified in rfkill.h */
    struct nxp_rfkill_dev *rf_dev;
    int rf_dev_num;
    struct list_head head;
    int initstatus;
    int support_suspend;
    struct nxp_rfkill_plat_data *pdata;
    struct rfkill_ops rf_ops;
};

static void nxp_rfkill_work(struct work_struct *work)
{
	struct nxp_rfkill_dev *rfdev = container_of(work, struct nxp_rfkill_dev, work.work);
	int enable = rfdev->enable;

	if (enable) {
		/* ON */
		if (rfdev->vcc) {
			if (!regulator_is_enabled(rfdev->vcc)) {
				pr_info("rfkill: %s supply %s delay enable (%dms)\n",
					rfdev->name, rfdev->supply_name, rfdev->delay_time_off);
				regulator_enable(rfdev->vcc);
			}
		} else
		if (NOT_GPIO != rfdev->gpio) {
			pr_info("rfkill: %s power %s.%d delay enable (%dms)\n", rfdev->name,
				GPIO_GROUP(rfdev->gpio), GPIO_BITNR(rfdev->gpio),
				rfdev->delay_time_off);
			gpio_set_value(rfdev->gpio, rfdev->invert ? 0 : 1);
		}
	} else {
		/* OFF */
		if (rfdev->vcc) {
			if (regulator_is_enabled(rfdev->vcc)) {
				pr_info("rfkill: %s supply %s delay disable (%dms)\n",
					rfdev->name, rfdev->supply_name, rfdev->delay_time_off);
				regulator_disable(rfdev->vcc);
			}
		} else
		if (NOT_GPIO != rfdev->gpio) {
			pr_info("rfkill: %s power %s.%d delay disable (%dms)\n", rfdev->name,
				GPIO_GROUP(rfdev->gpio), GPIO_BITNR(rfdev->gpio),
				rfdev->delay_time_off);
			gpio_set_value(rfdev->gpio, rfdev->invert ? 1 : 0);
		}
	}
}

/*
 * /sys/class/rfkill/rfkillN/state
 */
static int nxp_rfkill_set_block(void *data, bool blocked)
{
	struct nxp_rfkill_data *rf_data = data;
	struct list_head *head = &rf_data->head;
	struct list_head *list;

	if (!rf_data->initstatus)
		return 0;

	list_for_each_prev(list, head) {
		struct nxp_rfkill_dev *rfdev =
				container_of(list, struct nxp_rfkill_dev, link);

		pr_debug("rfkill: %s [%s] set_block %d %s:%s\n",
			rf_data->name, rfdev->power_supply?rfdev->supply_name:"io",
			blocked, blocked?"Off":"On ",
			rfdev->enable?"Enabled":"Disabled");

		if (rfdev->run_dealy_work)
			cancel_delayed_work_sync(&rfdev->work);

		if (blocked) {
			rfdev->enable = false;	/* OFF */
			if (rfdev->delay_time_off) {
				/* delay */
				schedule_delayed_work(&rfdev->work, msecs_to_jiffies(rfdev->delay_time_off));
				rfdev->run_dealy_work = true;
			} else {
				if (rfdev->vcc) {
					if (regulator_is_enabled(rfdev->vcc)) {
						pr_info("rfkill: %s supply %s disable\n", rf_data->name, rfdev->supply_name);
						regulator_disable(rfdev->vcc);
					}
				} else
				if (NOT_GPIO != rfdev->gpio) {
					pr_info("rfkill: %s power %s.%d disable\n", rf_data->name,
						GPIO_GROUP(rfdev->gpio), GPIO_BITNR(rfdev->gpio));
					gpio_set_value(rfdev->gpio, rfdev->invert ? 1 : 0);
				}
			}
		} else {
			rfdev->enable = true;	/* ON */
			if (rfdev->delay_time_on) {
				/* delay */
				schedule_delayed_work(&rfdev->work, msecs_to_jiffies(rfdev->delay_time_on));
				rfdev->run_dealy_work = true;
			} else {
				if (rfdev->vcc) {
					if (!regulator_is_enabled(rfdev->vcc)) {
						pr_info("rfkill: %s supply %s enable\n", rf_data->name, rfdev->supply_name);
						regulator_enable(rfdev->vcc);
					}
				} else
				if (NOT_GPIO != rfdev->gpio) {
					pr_info("rfkill: %s power %s.%d enable\n", rf_data->name,
						GPIO_GROUP(rfdev->gpio), GPIO_BITNR(rfdev->gpio));
					gpio_set_value(rfdev->gpio, rfdev->invert ? 0 : 1);
				}
			}
		}
	}

	return 0;
}

static int nxp_rfkill_module_notify(struct notifier_block *self, unsigned long val, void *data)
{
	struct nxp_rfkill_dev *rfdev = container_of(self, struct nxp_rfkill_dev, nb);
	struct module *mod = data;

	if (strcmp(mod->name, rfdev->module_name))
		return 0;

	if (MODULE_IN != val && MODULE_DEL != val)
		return 0;

	pr_debug("rfkill: notify mod [%s] %s - %s:%s (%lu)\n",
		mod->name, rfdev->power_supply?rfdev->supply_name:"io",
		val==MODULE_IN?"ON ":"OFF", rfdev->enable?"enabled":"disabled", val);

	if (rfdev->run_dealy_work) {
		rfdev->run_dealy_work = false;
		cancel_delayed_work_sync(&rfdev->work);
	}

	if (MODULE_IN == val) {	/* power on */

		rfdev->enable = true;
		if (rfdev->delay_time_on) {
			/* delay */
			schedule_delayed_work(&rfdev->work, msecs_to_jiffies(rfdev->delay_time_on));
			rfdev->run_dealy_work = true;
		} else {
			if (rfdev->vcc) {
				if (!regulator_is_enabled(rfdev->vcc)) {
					pr_info("rfkill: %s supply %s enable\n", mod->name, rfdev->supply_name);
					regulator_enable(rfdev->vcc);
				}
			} else
			if (NOT_GPIO != rfdev->gpio) {
				pr_info("rfkill: %s power %s.%d enable\n", mod->name,
					GPIO_GROUP(rfdev->gpio), GPIO_BITNR(rfdev->gpio));
				gpio_set_value(rfdev->gpio, rfdev->invert ? 0 : 1);
			}
		}
	} else {

		rfdev->enable = false;	/* OFF */
		if (rfdev->delay_time_off) {
			/* delay */
			schedule_delayed_work(&rfdev->work, msecs_to_jiffies(rfdev->delay_time_off));
			rfdev->run_dealy_work = true;
		} else {
			if (rfdev->vcc) {
				if (regulator_is_enabled(rfdev->vcc)) {
					pr_info("rfkill: %s supply %s disable\n", mod->name, rfdev->supply_name);
					regulator_disable(rfdev->vcc);
				}
			} else
			if (NOT_GPIO != rfdev->gpio) {
				pr_info("rfkill: %s power %s.%d disable\n", mod->name,
					GPIO_GROUP(rfdev->gpio), GPIO_BITNR(rfdev->gpio));
				gpio_set_value(rfdev->gpio, rfdev->invert ? 1 : 0);
			}
		}
	}

	return 0;
}

static void nxp_rfkill_free(struct nxp_rfkill_data *rf_data)
{
	struct nxp_rfkill_dev *rfdev = rf_data->rf_dev;
	struct list_head *head = &rf_data->head;
	struct list_head *list;

	pr_debug("rfkill: %s free \n", rf_data->name);
	list_for_each_prev(list, head) {
		rfdev = container_of(list, struct nxp_rfkill_dev, link);

		if (rfdev->run_dealy_work)
			cancel_delayed_work_sync(&rfdev->work);

		if (rfdev->vcc) {
			if (rfdev->vcc)
				regulator_put(rfdev->vcc);
		} else {
			if (NOT_GPIO != rfdev->gpio)
				gpio_free(rfdev->gpio);
		}

		list_del(list);
		kfree(rfdev);
	}
}

static int nxp_rfkill_setup(struct nxp_rfkill_data *rf_data, struct platform_device *pdev)
{
	struct nxp_rfkill_plat_data *pdata = pdev->dev.platform_data;
	struct rfkill_dev_data *pldev = pdata->rf_dev;
	struct nxp_rfkill_dev *rfdev;
	struct regulator *vcc;
	int i, ret = 0, on;

	pr_debug("rfkill: register device (%d:0x%p)\n", pdata->rf_dev_num, pdata->rf_dev);
	if (0 == pdata->rf_dev_num || !pdata->rf_dev)
		return -EINVAL;

	for (i = 0; pdata->rf_dev_num > i; i++, pldev++) {

		rfdev = kzalloc(sizeof(*rfdev), GFP_KERNEL);
		if (rfdev == NULL) {
			ret = -ENOMEM;
			goto err_dev_alloc;
		}

		rfdev->name = rf_data->name;
		rfdev->gpio = pldev->gpio;
		rfdev->initval = pldev->initval;
		rfdev->invert = pldev->invert;
		rfdev->delay_time_on = pldev->delay_time_on;
		rfdev->delay_time_off = pldev->delay_time_off;
		INIT_LIST_HEAD(&rfdev->link);

		if (pldev->module_name)
			strcpy(rfdev->module_name, pldev->module_name);

		if (pldev->supply_name) {
			strcpy(rfdev->supply_name, pldev->supply_name);
			vcc = regulator_get_exclusive(&pdev->dev, rfdev->supply_name);
			if (IS_ERR(vcc)) {
				pr_err("rfkill: Cannot get regulator for %s supply %s\n",
					pdata->name, rfdev->supply_name);
				continue;
			}

			if (regulator_is_enabled(vcc))
				pr_info("rfkill: %s Regulator already enabled\n", rfdev->supply_name);

			if (rfdev->initval & RFKILL_INIT_SET) {
				if (rfdev->initval & RFKILL_INIT_ON) {
					if (!regulator_is_enabled(vcc)) {
						pr_info("rfkill: %s Regulator enalbe\n", rfdev->supply_name);
						regulator_enable(rfdev->vcc);
					}
				} else {
					if (regulator_is_enabled(vcc)) {
						pr_info("rfkill: %s Regulator disalbe\n", rfdev->supply_name);
						regulator_disable(vcc);
					}
				}
			}
			rfdev->enable = regulator_is_enabled(vcc) ? true : false;
			rfdev->vcc = vcc;	/* set regulator */
			rfdev->power_supply = 1;
		} else {
			ret = gpio_request(rfdev->gpio, rf_data->name);
			if (0 > ret) {
				pr_err("rfkill: Cannot get gpio for %s supply %s\n",
					pdata->name, rf_data->name);
				rfdev->gpio = NOT_GPIO;
				continue;
			}

			if (rfdev->initval & RFKILL_INIT_SET) {
				on  = rfdev->initval & RFKILL_INIT_ON;
				ret = gpio_direction_output(rfdev->gpio, rfdev->invert ? !on : on);
				if (ret) {
					pr_err("rfkill: Cannot set direction on gpio %u\n", rfdev->gpio);
					rfdev->gpio = NOT_GPIO;
					continue;
				}
			}

			ret = gpio_get_value(rfdev->gpio);
			ret = rfdev->invert ? !ret : ret;
			if (ret) {
				pr_info("rfkill:%s Power gpio already enabled\n", rf_data->name);
				rfdev->enable = true;
			}
		}

		if (rfdev->module_name) {
			struct notifier_block *nb = &rfdev->nb;
			nb->notifier_call = nxp_rfkill_module_notify;
			register_module_notifier(nb);
		}

		if (rfdev->delay_time_on || rfdev->delay_time_off) {
			INIT_DELAYED_WORK_DEFERRABLE(&rfdev->work, nxp_rfkill_work);
			rfdev->run_dealy_work = false;
		}

		list_add_tail(&rfdev->link, &rf_data->head);
		pr_debug("rfkill: %s [%s]\n", rf_data->name, rfdev->vcc?"regulator":"power switch");
	}
	return 0;

err_dev_alloc:

	nxp_rfkill_free(rf_data);
	return ret;
}

static int nxp_rfkill_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct nxp_rfkill_data *rf_data = platform_get_drvdata(pdev);
	struct nxp_rfkill_plat_data *pdata = rf_data->pdata;
	struct list_head *head = &rf_data->head;
	struct list_head *list;
	int ret;

	PM_DBGOUT("rfkill: %s suspend\n", rf_data->name);

	if (pdata->suspend) {
		platform_set_drvdata(pdev, pdata);
		ret = pdata->suspend(pdev, state);
		platform_set_drvdata(pdev, rf_data);
		return ret;
	}

	if (!rf_data->support_suspend)
		return 0;

	list_for_each_prev(list, head) {
		struct nxp_rfkill_dev *rfdev =
			container_of(list, struct nxp_rfkill_dev, link);
		if (rfdev->enable) {
			if (rfdev->vcc) {
				if (regulator_is_enabled(rfdev->vcc))
					regulator_disable(rfdev->vcc);
			} else
			if (NOT_GPIO != rfdev->gpio) {
				gpio_set_value(rfdev->gpio, rfdev->invert ? 1 : 0);
			}
		}
	}
	return 0;
}

static int nxp_rfkill_resume(struct platform_device *pdev)
{
	struct nxp_rfkill_data *rf_data = platform_get_drvdata(pdev);
	struct nxp_rfkill_plat_data *pdata = rf_data->pdata;
	struct list_head *head = &rf_data->head;
	struct list_head *list;
	int ret;

	PM_DBGOUT("rfkill: %s resume\n", rf_data->name);

	if (pdata->suspend) {
		platform_set_drvdata(pdev, pdata);
		ret = pdata->resume(pdev);
		platform_set_drvdata(pdev, rf_data);
		return ret;
	}

	if (!rf_data->support_suspend)
		return 0;

	list_for_each_prev(list, head) {
		struct nxp_rfkill_dev *rfdev =
			container_of(list, struct nxp_rfkill_dev, link);
		if (rfdev->enable) {
			if (rfdev->vcc) {
				if (!regulator_is_enabled(rfdev->vcc))
					regulator_enable(rfdev->vcc);
			} else
			if (NOT_GPIO != rfdev->gpio) {
				gpio_set_value(rfdev->gpio, rfdev->invert ? 0 : 1);
			}
		}
	}

	return 0;
}

static int __devinit nxp_rfkill_probe(struct platform_device *pdev)
{
	struct nxp_rfkill_plat_data *pdata = pdev->dev.platform_data;
	struct nxp_rfkill_data *rf_data;
	struct rfkill *rfkill;
	struct rfkill_ops *rf_ops;
	void *ops_data;
	int ret = 0;

	pr_debug("\n%s (name=%s, id=%d)\n", __func__, dev_name(&pdev->dev), pdev->id);

	if (pdata == NULL) {
		pr_err("rfkill: no platform data\n");
		return -ENODEV;
	}

	if (pdata->name == NULL || pdata->type == 0 ||
		pdata->rf_dev == NULL ||
		pdata->rf_dev_num == 0) {
		pr_err("rfkill: invalid name or type in platform data\n");
		return -EINVAL;
	}

	rf_data = kzalloc(sizeof(*rf_data), GFP_KERNEL);
	if (rf_data == NULL)
		return -ENOMEM;

	rf_ops = &rf_data->rf_ops;

	/*
	 * replace set_block ops and rfkill private data
	 */
	if (pdata->set_block) {
		rf_ops->set_block = pdata->set_block;
		ops_data = (void*)pdata;
	} else {
		rf_ops->set_block = nxp_rfkill_set_block;
		ops_data = rf_data;
	}

	rfkill = rfkill_alloc(pdata->name, &pdev->dev, pdata->type, rf_ops, ops_data);
	if (rfkill == NULL) {
		ret = -ENOMEM;
		goto err_rfkill_alloc;
	}

	rf_data->rfkill = rfkill;
	rf_data->name = pdata->name;
	rf_data->support_suspend = pdata->support_suspend;
	rf_data->pdata = pdata;

	INIT_LIST_HEAD(&rf_data->head);

	if (pdata->init)
		pdata->init(pdata);

	if (0 > nxp_rfkill_setup(rf_data, pdev))
		goto err_rfkill_register;

	ret = rfkill_register(rfkill);
	if (ret) {
		pr_err("rfkill: Cannot register rfkill device\n");
		goto err_rfkill_register;
	}

	rf_data->initstatus = 1;
	platform_set_drvdata(pdev, rf_data);

	pr_info("rfkill: rfkill.%d %s probed\n", pdev->id, pdata->name);
	return 0;

err_rfkill_register:
	nxp_rfkill_free(rf_data);
	rfkill_destroy(rfkill);
err_rfkill_alloc:
	kfree(rf_data);

	return ret;
}

static int __devexit nxp_rfkill_remove(struct platform_device *pdev)
{
	struct nxp_rfkill_data *rf_data = platform_get_drvdata(pdev);
	struct rfkill *rfkill = rf_data->rfkill;

	nxp_rfkill_free(rf_data);
	rfkill_unregister(rfkill);
	rfkill_destroy(rfkill);
	kfree(rf_data);

	return 0;
}

static struct platform_driver nxp_rfkill_driver = {
	.probe = nxp_rfkill_probe,
	.remove = __devexit_p(nxp_rfkill_remove),
	.driver = {
		.name = DEV_NAME_RFKILL,
		.owner = THIS_MODULE,
	},
	.suspend = nxp_rfkill_suspend,
	.resume  = nxp_rfkill_resume,
};

module_platform_driver(nxp_rfkill_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Rfkill driver for network devices");
MODULE_LICENSE("GPL");

