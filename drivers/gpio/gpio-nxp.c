/*
 * Copyright (C) 2008, 2009 Provigent Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Driver for the ARM PrimeCell(tm) General Purpose Input/Output (PL061)
 *
 * Data sheet: ARM DDI 0190B, September 2000
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/pm.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

/*
#define pr_debug(msg...)		{ printk(KERN_INFO "gpio: " msg); }
*/

#if (0)
#define CHECK_ALTFUNC_RET(_gpio, _offs, _ret)	do {		\
		int io = (_gpio->index * GPIO_NUM_PER_BANK) + _offs;		\
		int cn = nxp_soc_gpio_get_io_func(io);						\
		int fn = GET_GPIO_ALTFUNC(_gpio->index, _offs);				\
		if (cn != fn) {												\
			printk("Fail : io used alt function %d [%s:%u=%d]\n",	\
			cn, io_name[_gpio->index], _offs, io);					\
			return _ret; 	\
		}					\
	} while(0)
#else
#define	CHECK_ALTFUNC_RET(_gpio, _offs, _ret)	do { } while (0)
#endif

struct nxp_gpio {
	int 		index; 		/* Bank Index : A(0), B(1), C(2), D(3), E(4), ALIVE(5) */
	spinlock_t	lock;		/* GPIO registers */
	struct 	gpio_chip chip;
};

static const char *io_name[] = { "GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "GPIOALV" };
#define	GET_GPIO(c)	container_of(chip, struct nxp_gpio, chip)

static int nxp_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	struct nxp_gpio *gpio = GET_GPIO(chip);
	int io, fn;

	io = (gpio->index * GPIO_NUM_PER_BANK) + offset;
	fn = GET_GPIO_ALTFUNC(gpio->index, offset);

	CHECK_ALTFUNC_RET(gpio, offset, EINVAL);

	nxp_soc_gpio_set_io_func(io, fn);
	pr_debug("%s: io [%s:%d=%d]\n", __func__, io_name[gpio->index], offset, io);

	return 0;
}

static int nxp_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct nxp_gpio *gpio = GET_GPIO(chip);
	int io, fn;

	io = (gpio->index * GPIO_NUM_PER_BANK) + offset;
	fn = GET_GPIO_ALTFUNC(gpio->index, offset);

	CHECK_ALTFUNC_RET(gpio, offset, EINVAL);

	nxp_soc_gpio_set_io_func(io, fn);
	nxp_soc_gpio_set_io_dir(io, 0);

	pr_debug("%s: io [%s:%d=%d]\n", __func__, io_name[gpio->index], offset, io);
	return 0;
}

static int nxp_gpio_direction_output(struct gpio_chip *chip,
				unsigned offset, int value)
{
	struct nxp_gpio *gpio = GET_GPIO(chip);
	int io, fn;

	io = (gpio->index * GPIO_NUM_PER_BANK) + offset;
	fn = GET_GPIO_ALTFUNC(gpio->index, offset);

	CHECK_ALTFUNC_RET(gpio, offset, EINVAL);

	nxp_soc_gpio_set_io_func(io, fn);
	nxp_soc_gpio_set_out_value(io, value);
	nxp_soc_gpio_set_io_dir(io, 1);

	pr_debug("%s: io [%s:%d=%d], val=%d\n",
		__func__, io_name[gpio->index], offset, io, value);
	return 0;
}

static int nxp_gpio_get_value(struct gpio_chip *chip, unsigned offset)
{
	struct nxp_gpio *gpio = GET_GPIO(chip);
	int io = gpio->index * GPIO_NUM_PER_BANK + offset;

	pr_debug("%s: io [%s:%d=%d]\n", __func__, io_name[gpio->index], offset, io);

	return nxp_soc_gpio_get_in_value(io);
}

static void nxp_gpio_set_value(struct gpio_chip *chip, unsigned offset, int value)
{
	struct nxp_gpio *gpio = GET_GPIO(chip);
	int io, fn, cn;

	io = gpio->index * GPIO_NUM_PER_BANK + offset;
	fn = GET_GPIO_ALTFUNC(gpio->index, offset);
	cn = nxp_soc_gpio_get_io_func(io);
#if (0)
	if (cn != fn) {
		printk("Fail : io used alt function %d [%s:%u=%d]\n",
			cn, io_name[gpio->index], offset, io);
		return;
	}
#endif
	nxp_soc_gpio_set_io_dir(io, 1);
	nxp_soc_gpio_set_out_value(io, value);

	pr_debug("%s: io [%s:%d=%d], val=%d\n",
		__func__, io_name[gpio->index], offset, io, value);
}

static int nxp_gpio_to_irq( struct gpio_chip *chip , unsigned offset )
{
	struct nxp_gpio *gpio = GET_GPIO(chip);
	unsigned int io = gpio->index * GPIO_NUM_PER_BANK + offset;

	return (io + IRQ_GPIO_START);
}

#ifdef CONFIG_PM
static int nxp_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}

static int nxp_gpio_resume(struct platform_device *pdev)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}
#else
#define	nxp_gpio_suspend	NULL
#define	nxp_gpio_resume		NULL
#endif

static int nxp_gpio_probe(struct platform_device *pdev)
{
	struct resource *res = pdev->resource;
	struct nxp_gpio *gpio = NULL;
	int ret;

	pr_debug("%s: %s, %d ~ %2d\n",
		__func__, io_name[pdev->id], res->start, res->end);
	if (!res) {
		printk("Error: not allocated gpio resource [%d]\n", pdev->id);
		return -EINVAL;
	}

	gpio = kzalloc(sizeof(*gpio), GFP_KERNEL);
	if (gpio == NULL)
		return -ENOMEM;

	spin_lock_init(&gpio->lock);

	gpio->index = pdev->id;
	gpio->chip.request = nxp_gpio_request;
	gpio->chip.to_irq = nxp_gpio_to_irq;
	gpio->chip.direction_input = nxp_gpio_direction_input;
	gpio->chip.direction_output = nxp_gpio_direction_output;
	gpio->chip.get = nxp_gpio_get_value;
	gpio->chip.set = nxp_gpio_set_value;
	gpio->chip.ngpio = res->end - res->start;
	gpio->chip.label = dev_name(&pdev->dev);
	gpio->chip.dev = &pdev->dev;
	gpio->chip.owner = THIS_MODULE;
	gpio->chip.base = pdev->id * GPIO_NUM_PER_BANK;

	/* register GPIOLib */
	ret = gpiochip_add(&gpio->chip);
	if (ret)
		goto free_mem;

	return ret;

free_mem:
	kfree(gpio);
	return ret;
}

static int nxp_gpio_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver nxp_gpio_driver = {
	.probe		= nxp_gpio_probe,
	.remove		= __devexit_p(nxp_gpio_remove),
	.suspend	= nxp_gpio_suspend,
	.resume		= nxp_gpio_resume,
	.driver		= {
		.name	= DEV_NAME_GPIO,
		.owner	= THIS_MODULE,
	},
};

static int __init nxp_gpio_init(void)
{
	return platform_driver_register(&nxp_gpio_driver);
}
subsys_initcall(nxp_gpio_init);

MODULE_DESCRIPTION("GPIO driver for the Nexell");
MODULE_LICENSE("GPL");
