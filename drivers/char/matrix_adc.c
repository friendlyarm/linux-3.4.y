/*
 * linux/drivers/char/matrix_adc.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <mach/devices.h>
#include "../staging/iio/consumer.h"

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK(x...) {printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...) (void)(0)
#endif

#define DEVICE_NAME	"iio-adc"

typedef struct {
	struct mutex lock;
	struct iio_channel *iio;
	int channel;
} ADC_DEV;

static ADC_DEV adcdev;

static inline int iio_adc_read_ch(void) {
	char name[16];
	int val = 0;
	int ret;

	ret = mutex_lock_interruptible(&adcdev.lock);
	if (ret < 0)
		return ret;

	if (!adcdev.iio) {
		sprintf(name, "adc.%d", adcdev.channel);

		adcdev.iio = iio_st_channel_get(DEV_NAME_ADC, name);
		if (IS_ERR_OR_NULL(adcdev.iio)) {
			printk("iio-adc: failed to get iio channel %s\n", name);
			return -ENODEV;
		}
	}

	ret = iio_st_read_channel_raw(adcdev.iio, &val);
	if (ret < 0) {
		printk("iio-adc: failed to read channel raw %s, ret = %d\n", name, ret);
		return ret;
	}

	mutex_unlock(&adcdev.lock);

	return val;
}

static inline void iio_adc_set_channel(int channel) {

	if (channel < 0 || channel > 7)
		return;

	adcdev.channel = channel;
}

static ssize_t iio_adc_read(struct file *filp, char *buffer,
		size_t count, loff_t *ppos)
{
	char str[20];
	int value;
	size_t len;

	value = iio_adc_read_ch();
	if (value < 0)
		return -EIO;

	len = sprintf(str, "%d\n", value);
	if (count >= len) {
		int r = copy_to_user(buffer, str, len);
		return r ? r : len;
	} else {
		return -EINVAL;
	}
}

static long iio_adc_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
#define ADC_SET_CHANNEL		0xc000fa01
#define ADC_SET_ADCTSC		0xc000fa02

	switch (cmd) {
		case ADC_SET_CHANNEL:
			iio_adc_set_channel(arg);
			break;
		case ADC_SET_ADCTSC:
			/* do nothing */
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int iio_adc_open(struct inode *inode, struct file *filp)
{
	iio_adc_set_channel(1);

	DPRINTK("adc opened\n");
	return 0;
}

static int iio_adc_release(struct inode *inode, struct file *filp)
{
	if (adcdev.iio) {
		mutex_lock(&adcdev.lock);
		iio_st_channel_release(adcdev.iio);
		adcdev.iio = NULL;
		mutex_unlock(&adcdev.lock);
	}

	DPRINTK("adc closed\n");
	return 0;
}

static struct file_operations adc_dev_fops = {
	owner:	THIS_MODULE,
	open:	iio_adc_open,
	read:	iio_adc_read,	
	unlocked_ioctl:	iio_adc_ioctl,
	release:	iio_adc_release,
};

static struct miscdevice misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "adc",
	.fops	= &adc_dev_fops,
};

static int __devinit iio_adc_probe(struct platform_device *dev)
{
	int ret;

	mutex_init(&adcdev.lock);

	ret = misc_register(&misc);

	printk(DEVICE_NAME"\tinitialized\n");

	return ret;
}

static int __devexit iio_adc_remove(struct platform_device *dev)
{
	misc_deregister(&misc);

	if (adcdev.iio) {
		iio_st_channel_release(adcdev.iio);
		adcdev.iio = NULL;
	}

	return 0;
}

static struct platform_driver iio_adc_driver = {
	.driver = {
		.name		= "iio_adc",
		.owner		= THIS_MODULE,
	},
	.probe		= iio_adc_probe,
	.remove		= __devexit_p(iio_adc_remove),
};

static int __init iio_adc_init(void)
{
	return platform_driver_register(&iio_adc_driver);
}

static void __exit iio_adc_exit(void)
{
	platform_driver_unregister(&iio_adc_driver);
}

module_init(iio_adc_init);
module_exit(iio_adc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");

