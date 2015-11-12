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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/module.h>
#include <mach/soc.h>

#define NXPFB_SET_POS	 	_IOW('N', 104, __u32)

static ssize_t store_resol(struct device *device, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct fb_info *info = dev_get_drvdata(device);
	struct fb_ops *fops = info->fbops;
	struct fb_var_screeninfo var = { 0, };
	const char *first = buf;
	char *last = NULL;
	int i = 0, val[5] = { 0, }; /* sx, w, sy, h, pixelbyte */
	int ret;

	for (i = 0; ARRAY_SIZE(val) > i; i++) {
		val[i] = simple_strtoul(buf, &last, 10);
		last++;	buf = last;
		if (last - first > count)
			return -EINVAL;
	}

	console_lock();

	info->flags |= FBINFO_MISC_USEREVENT;

	var.xres = val[1];	/* width */
	var.yres = val[3];	/* height */
	var.bits_per_pixel = val[4]*8; 	/* pixelbyte */

	ret = fb_set_var(info, &var);

	info->flags &= ~FBINFO_MISC_USEREVENT;
	if (fops->fb_ioctl) {
		int pos[3] = { val[0], val[2], 1 };	/* sx, xy, waitvsync */
		fops->fb_ioctl(info, (unsigned int)NXPFB_SET_POS, (unsigned long)pos);
	}
	console_unlock();

	unlock_fb_info(info);

	if (0 > ret)
		return -EINVAL;

	return count;
}

static struct device_attribute device_attrs[] = {
	__ATTR(resolution, S_IRUGO|S_IWUSR, NULL, store_resol),
};

int nx_fb_init_sysfs(struct fb_info *fb_info)
{
	int i, err = 0;

	for (i = 0; i < ARRAY_SIZE(device_attrs); i++) {
		err = device_create_file(fb_info->dev, &device_attrs[i]);
		if (err)
			break;
	}

	if (err) {
		while (--i >= 0)
			device_remove_file(fb_info->dev, &device_attrs[i]);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(nx_fb_init_sysfs);

