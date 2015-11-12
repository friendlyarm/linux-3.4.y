/*
 * 1-Wire implementation for the ds2784 chip
 *
 * Copyright (C) 2012 Invensense, Inc.
 * Copyright (C) 2012 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "../w1.h"
#include "../w1_int.h"
#include "../w1_family.h"
#include "w1_ds2784.h"

static int w1_ds2784_io(struct device *dev, char *buf, int addr, size_t count,
			int io)
{
	struct w1_slave *sl = container_of(dev, struct w1_slave, dev);

	mutex_lock(&sl->master->mutex);

	if (addr > DS2784_DATA_SIZE || addr < 0) {
		count = 0;
		goto out;
	}
	if (addr + count > DS2784_DATA_SIZE)
		count = DS2784_DATA_SIZE - addr;

	if (!w1_reset_select_slave(sl)) {
		if (!io) {
			w1_write_8(sl->master, W1_DS2784_READ_DATA);
			w1_write_8(sl->master, addr);
			count = w1_read_block(sl->master, buf, count);
		} else {
			w1_write_8(sl->master, W1_DS2784_WRITE_DATA);
			w1_write_8(sl->master, addr);
			w1_write_block(sl->master, buf, count);
		}
	}

out:
	mutex_unlock(&sl->master->mutex);

	return count;
}

int w1_ds2784_read(struct device *dev, char *buf, int addr, size_t count)
{
	return w1_ds2784_io(dev, buf, addr, count, 0);
}
EXPORT_SYMBOL(w1_ds2784_read);

static int w1_ds2784_add_slave(struct w1_slave *sl)
{
	struct platform_device *pdev;
	int ret;

	pdev = platform_device_alloc("ds2784-fuelgauge", -1);
	if (!pdev)
		return -ENOMEM;

	pdev->dev.parent = &sl->dev;
	ret = platform_device_add(pdev);

	if (ret)
		goto add_failed;

	dev_set_drvdata(&sl->dev, pdev);
	return 0;

add_failed:
	platform_device_unregister(pdev);
	return ret;
}

static void w1_ds2784_remove_slave(struct w1_slave *sl)
{
	struct platform_device *pdev = dev_get_drvdata(&sl->dev);

	platform_device_unregister(pdev);
}

static struct w1_family_ops w1_ds2784_fops = {
	.add_slave = w1_ds2784_add_slave,
	.remove_slave = w1_ds2784_remove_slave,
};

static struct w1_family w1_ds2784_family = {
	.fid = W1_FAMILY_DS2784,
	.fops = &w1_ds2784_fops,
};

int __init w1_ds2784_init(void)
{
	return w1_register_family(&w1_ds2784_family);
}

static void __exit w1_ds2784_exit(void)
{
	w1_unregister_family(&w1_ds2784_family);
}

module_init(w1_ds2784_init);
module_exit(w1_ds2784_exit);

MODULE_AUTHOR("Samsung");
MODULE_DESCRIPTION("1-wire Driver for Maxim/Dallas DS2784 Fuel Gauge IC");
MODULE_LICENSE("GPL");
