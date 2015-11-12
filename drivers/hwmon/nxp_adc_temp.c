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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/hwmon.h>
#include <linux/hwmon-vid.h>
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <mach/devices.h>
#include "../staging/iio/consumer.h"

/*
#define	pr_debug	printk
*/

#define DRVNAME	"nxp-adc-tmp"

struct nxp_adc_tmp {
	struct device *hwmon_dev;
	const char *name;
	struct iio_channel *iio;
	u32 id;
	u32 adc_value;
	int channel;
	int delay_ms;
	void (*callback)(int ch, int adc, int temp, bool run);
	/* TMU func */
	struct delayed_work mon_work;
	unsigned long state;
	/* ADC tmu info */
	int temperature;
	int tmp_max;
	int tmp_value;
	int tmp_offset;
	int tmp_previous;
	int valid;
	int first;
	int check_count;
};

#define	STATE_SUSPEND_ENTER		(0)		/* bit position */
#define	STATE_STOP_ENTER		(1)		/* bit position */
#define STATE_CHECK_TIME		(100)	/* ms */

/*
 * TMU operation
 */

/* initialize table for register value matching with tmp_value */
static int tmp_table[][2] = {
	[0] = {9900, 40}, // 0
	[1] = {9100, 45},
	[2] = {8400, 50},
	[3] = {7700, 55},
	[4] = {7000, 60}, // 4
	[5] = {6300, 65}, // 5
	[6] = {5700, 70},
	[7] = {5200, 75},
	[8] = {4700, 80},
	[9] = {4200, 85}  // 9
};
#define TEMP_TABLAE_SIZE	ARRAY_SIZE(tmp_table)

static void nxp_adc_tmp_monfn(struct work_struct *work)
{
	struct nxp_adc_tmp *tmp = container_of(work, struct nxp_adc_tmp, mon_work.work);
	int i = 0, j = 0, val = 0;
	int voltage;
	int err = 0;

	if (test_bit(STATE_SUSPEND_ENTER, &tmp->state))
		goto exit_mon;

	/* read adc and convert tmp */
	err = iio_st_read_channel_raw(tmp->iio, &val);
	if (0 > err)
		goto exit_mon;

	tmp->adc_value = val;
	voltage = (18*val*1000)/4096;

	/*
	 * according to Register Voltage table,
	 * calculate board tmp_value.
	 */
	for (i = 0, j = 0; TEMP_TABLAE_SIZE > i; i++) {
		if (voltage > tmp_table[i][0]) {
			if (0 != i) {
				j = (tmp_table[i-1][0] - tmp_table[i][0])/5;
				break;
			}
		}
	}

	if (i == TEMP_TABLAE_SIZE) {
		tmp->tmp_value = 90;
	} else if (j == 0) {
		tmp->tmp_value = 40;
	} else {
		int n = tmp_table[i-1][0] - j;
		tmp->tmp_value = tmp_table[i-1][1];

		for (; n>tmp_table[i][0]; n -= j) {
			if (voltage > n)
				break;
			tmp->tmp_value++;
		}
		if (tmp->tmp_value > tmp_table[i][1])
			tmp->tmp_value = tmp_table[i][1];
	}

	/* ignore the tmp_value value when booting. */
	if (tmp->valid == 0) {
		if (tmp->first == 0) {
			tmp->first = 1;
			tmp->tmp_previous = tmp->tmp_value;
		} else {
			if (tmp->tmp_previous == tmp->tmp_value)
				tmp->check_count++;
			else
				tmp->check_count = 0;
			tmp->tmp_previous = tmp->tmp_value;

			if (tmp->check_count == 3)
				tmp->valid = 1;
		}
		if (tmp->valid == 0) {
			schedule_delayed_work(&tmp->mon_work, msecs_to_jiffies(tmp->delay_ms));
			return;
		}
	}

	/* adjust the tmp_value value . */
	if (tmp->tmp_value >= tmp->tmp_previous) {
		int df = tmp->tmp_value - tmp->tmp_previous;
		if (df >= 5)  // ignore.
			tmp->tmp_value = tmp->tmp_previous;
		else
			tmp->tmp_previous = tmp->tmp_value;
	} else {
		int df = tmp->tmp_previous - tmp->tmp_value;
		if (df >= 5)  // ignore.
			tmp->tmp_value = tmp->tmp_previous;
		else
			tmp->tmp_previous = tmp->tmp_value;
	}

	if (tmp->tmp_value > tmp->tmp_max)
		tmp->tmp_max = tmp->tmp_value;

	tmp->temperature = (tmp->tmp_value - tmp->tmp_offset);
	pr_debug("TMU [%d] %3d:%3d (%4d)\n",
		tmp->channel, tmp->tmp_value, tmp->tmp_max, tmp->adc_value);

exit_mon:
	if (tmp->callback)
		tmp->callback(tmp->channel, tmp->adc_value, tmp->tmp_value, true);

	schedule_delayed_work(&tmp->mon_work, msecs_to_jiffies(tmp->delay_ms));

	return;
}

/*
 * Sysfs
 */
enum { SHOW_TEMP, SHOW_LABEL, SHOW_NAME };

static ssize_t show_value(struct device *dev,
			 struct device_attribute *devattr, char *buf)
{
	struct nxp_adc_tmp *tmp = dev_get_drvdata(dev);
	char *s = buf;

	s += sprintf(s, "%3d:%4d\n", tmp->tmp_value, tmp->adc_value);
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t show_temp(struct device *dev,
			 struct device_attribute *devattr, char *buf)
{
	struct nxp_adc_tmp *tmp = dev_get_drvdata(dev);
	char *s = buf;

	s += sprintf(s, "%4d\n", tmp->temperature);
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t store_temp(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	struct nxp_adc_tmp *tmp = dev_get_drvdata(dev);
	const char *s = buf;

	if (0 == strncmp(s, "run", strlen("run"))) {
		set_bit(STATE_STOP_ENTER, &tmp->state);
		if (test_bit(STATE_STOP_ENTER, &tmp->state)) {
			if (tmp->callback)
				tmp->callback(tmp->channel, tmp->adc_value, tmp->tmp_value, true);
			schedule_delayed_work(&tmp->mon_work, msecs_to_jiffies(tmp->delay_ms));
		}
	}
	else if (0 == strncmp(s, "stop", strlen("stop"))) {
		clear_bit(STATE_STOP_ENTER, &tmp->state);
		cancel_delayed_work_sync(&tmp->mon_work);
		if (tmp->callback)
			tmp->callback(tmp->channel, tmp->adc_value, tmp->tmp_value, false);
	} else {
		count = -1;
	}

	return count;
}

static SENSOR_DEVICE_ATTR(temp_value, S_IRUGO, show_value, NULL, SHOW_LABEL);
static SENSOR_DEVICE_ATTR(temp_label, 0666, show_temp , store_temp, SHOW_LABEL);

static struct attribute *adc_temp_attr[] = {
	&sensor_dev_attr_temp_value.dev_attr.attr,
	&sensor_dev_attr_temp_label.dev_attr.attr,
	NULL
};

static const struct attribute_group adc_temp_group = {
	.attrs = adc_temp_attr,
};

#ifdef CONFIG_PM
static int nxp_adc_tmp_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct nxp_adc_tmp *tmp =  platform_get_drvdata(pdev);
	set_bit(STATE_SUSPEND_ENTER, &tmp->state);
	return 0;
}

static int nxp_adc_tmp_resume(struct platform_device *pdev)
{
	struct nxp_adc_tmp *tmp =  platform_get_drvdata(pdev);
	clear_bit(STATE_SUSPEND_ENTER, &tmp->state);
	return 0;
}
#else
#define nxp_adc_tmp_suspend NULL
#define nxp_adc_tmp_resume NULL
#endif

static int __devinit nxp_adc_tmp_probe(struct platform_device *pdev)
{
	struct nxp_adc_tmp_platdata *plat = pdev->dev.platform_data;
	struct nxp_adc_tmp *tmp = NULL;
	int err = -1;
	char name[16] ;

	if (!plat) {
		dev_err(&pdev->dev, "no platform data ....\n");
		goto exit;
	}

	tmp = kzalloc(sizeof(struct nxp_adc_tmp), GFP_KERNEL);
	if (!tmp) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "Out of memory\n");
		goto exit;
	}

	sprintf(name, "adc.%d", plat->channel);
	tmp->channel =	plat->channel;
	tmp->id = pdev->id;
	tmp->name = DRVNAME;
	tmp->channel = plat->channel;
	tmp->delay_ms = plat->duration ? plat->duration : STATE_CHECK_TIME;
	tmp->tmp_offset = plat->tmp_offset;
	tmp->callback = plat->callback;
	tmp->iio = iio_st_channel_get(DEV_NAME_ADC, name);
	tmp->check_count = 0;
	tmp->valid = 0;
	tmp->first = 0;
	clear_bit(STATE_SUSPEND_ENTER, &tmp->state);

	platform_set_drvdata(pdev, tmp);

	err = sysfs_create_group(&pdev->dev.kobj, &adc_temp_group);
	if (err)
		goto exit_free;

	tmp->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(tmp->hwmon_dev)) {
		err = PTR_ERR(tmp->hwmon_dev);
		dev_err(&pdev->dev, "Class registration failed (%d)\n",
			err);
		goto exit_remove;
	}

	INIT_DELAYED_WORK(&tmp->mon_work, nxp_adc_tmp_monfn);
	schedule_delayed_work(&tmp->mon_work, msecs_to_jiffies(1));
	printk("TMU: register %s to hwmon\n", name);

	return 0;

exit_remove:
	sysfs_remove_group(&pdev->dev.kobj, &adc_temp_group);
exit_free:
	platform_set_drvdata(pdev, NULL);
	kfree(tmp);
exit:

	return err;
}

static int __devexit nxp_adc_tmp_remove(struct platform_device *pdev)
{
	struct nxp_adc_tmp *tmp = platform_get_drvdata(pdev);

	hwmon_device_unregister(tmp->hwmon_dev);
	sysfs_remove_group(&pdev->dev.kobj, &adc_temp_group);
	platform_set_drvdata(pdev, NULL);
	kfree(tmp);

	return 0;
}

static struct platform_driver nxp_adc_tmp_driver = {
	.driver = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
	.probe = nxp_adc_tmp_probe,
	.remove	= __devexit_p(nxp_adc_tmp_remove),
	.suspend = nxp_adc_tmp_suspend,
	.resume = nxp_adc_tmp_resume,
};

static int __init nxp_adc_tmp_init(void)
{
    return platform_driver_register(&nxp_adc_tmp_driver);
}

static void __exit nxp_adc_tmp_exit(void)
{
    return platform_driver_unregister(&nxp_adc_tmp_driver);
}

late_initcall(nxp_adc_tmp_init);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("SLsiAP ADC tmp_value monitor");
MODULE_LICENSE("GPL");

