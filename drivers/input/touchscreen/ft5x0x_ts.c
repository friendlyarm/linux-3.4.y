/*
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
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
 *
 *	note: only support mulititouch	Wenfs 2010-10-01
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/i2c.h>
#include <linux/input.h>

#include <linux/interrupt.h>
#include <linux/delay.h>

#include <mach/platform.h>
#include "ft5x06_ts.h"

//#undef CONFIG_FT5X0X_MULTITOUCH
//#define DEBUG
//#define DEBUG_1

#ifdef DEBUG
	#define TS_DEBUG(fmt,args...) 	do { printk(fmt, ##args); } while (0)
	#else
	#define TS_DEBUG(fmt,args...)	do { } while (0)
#endif

#ifdef DEBUG_1
	#define TS_DEBUG1(fmt,args...) printk(fmt, ##args )
	#else
	#define TS_DEBUG1(fmt,args...)	do { } while (0)
#endif

#include <mach/devices.h>

#ifdef CONFIG_FT5X0X_MULTITOUCH
#define	MAX_SUPPORT_POINT		10
#define	DEF_SUPPORT_POINT		5
#else
#define	DEF_SUPPORT_POINT 		1
#define	MAX_SUPPORT_POINT		1
#endif

#define	CAL_POINTER_NUM			7

struct ts_pointercal {
	/* for attribute kobject */
	struct attribute_group *attr;
	struct kobject *kobj;
	/* pointer calibration */
	int touch_point_max;
	int	enable;
	int x_resol;
	int y_resol;
	int rotate;
	int	pointercal[CAL_POINTER_NUM];
};
static inline void ts_adjust_calibration(u16 *x, u16 *y);

static struct i2c_client *this_client;

struct ts_event {
    u16  touch_points;
	u16	x[MAX_SUPPORT_POINT];
	u16	y[MAX_SUPPORT_POINT];
	s16 ID[MAX_SUPPORT_POINT];
	u16	st[MAX_SUPPORT_POINT];
	u16	press[MAX_SUPPORT_POINT];
};

struct ft5x0x_ts_data {
	struct input_dev *input_dev;
	struct ts_event	event;
	struct workqueue_struct *ts_work;
	struct delayed_work event_work;
	struct mutex lock;
    int touch_point_max;
	struct ts_pointercal pointer;
};
#define	DELAY_WORK_JIFFIES 		1

/*----------------------------------------------------------------------------*/
/*
 *  #> cat /sys/devices/platform/touch/calibration
 *
 *	[SET calibration]
 * step 1:
 *	- clear default calibration value
 * 	#> echo > /sys/devices/platform/touch/calibration
 *
 * step 2:
 *	- calibration with "ts_calibrate"
 *
 * step 3:
 *	- get new calibration value
 *	#> cat .../tslib/etc/pointercal
 *
 * step 4:
 *	- set new calibration value
 *	#> echo xxxx,xxx,xx,,, > /sys/devices/platform/touch/calibration
*
 * step 5:
 *	- delete .../tslib/etc/pointercal
 *	#> rm .../tslib/etc/pointercal
 *
 */
static ssize_t ts_calibration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_pointercal *point = &ts->pointer;
	char *s = buf;

	TS_DEBUG("ts_calibration_show\n");
	s += sprintf(s, "%d,%d,%d,%d,%d,%d,%d\n",
		point->pointercal[0], point->pointercal[1], point->pointercal[2],
		point->pointercal[3], point->pointercal[4], point->pointercal[5],
		point->pointercal[6]);

	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t ts_calibration_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_pointercal *point = &ts->pointer;
	int	pointercal[CAL_POINTER_NUM] = { 0, };
	int len = sizeof(int)*CAL_POINTER_NUM;

	TS_DEBUG("ts_calibration_store\n");
	if (NULL == buf)
		return n;

	sscanf(buf,"%d,%d,%d,%d,%d,%d,%d",
		&pointercal[0], &pointercal[1], &pointercal[2],
		&pointercal[3], &pointercal[4], &pointercal[5],
		&pointercal[6]);

	point->enable = 0;
	memset(&point->pointercal, 0, len);

	if (pointercal[0] != 0 || pointercal[1] != 0 || pointercal[2] != 0 ||
		pointercal[3] != 0 || pointercal[4] != 0 || pointercal[5] != 0 ||
		pointercal[6] != 0)
	{
		point->enable = 1;
		memcpy(point->pointercal, pointercal, len);
	}
	return n;
}

static ssize_t ts_rotate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_pointercal *point = &ts->pointer;
	char *s = buf;

	TS_DEBUG("ts_rotate_show\n");
	s += sprintf(s, "%d\n", point->rotate);
	if (s != buf)
		*(s-1) = '\n';
	return (s - buf);
}

static ssize_t ts_rotate_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_pointercal *point = &ts->pointer;
	int	rotate= 0;
	TS_DEBUG("ts_rotate_store\n");
	if (NULL == buf)
		return n;

	point->rotate = 0;
	sscanf(buf,"%d", &rotate);
	if ( 90 == rotate ||
		180 == rotate ||
		270 == rotate) {
		point->rotate = rotate;
	}
	return n;
}

static DEVICE_ATTR(calibration, S_IRUGO | S_IWUSR, ts_calibration_show, ts_calibration_store);
static DEVICE_ATTR(rotate, S_IRUGO | S_IWUSR, ts_rotate_show, ts_rotate_store);

static ssize_t ts_points_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	char *s = buf;

	TS_DEBUG("ts_points_show\n");
	s += sprintf(s, "%d\n", ts->touch_point_max);
	if (s != buf)
		*(s-1) = '\n';
	return (s - buf);
}

static ssize_t ts_points_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_pointercal *point = &ts->pointer;
	int	points= 0;
	TS_DEBUG("ts_points_store\n");
	if (NULL == buf)
		return n;

	sscanf(buf,"%d", &points);

	if (points > 0 && (MAX_SUPPORT_POINT + 1) > points) {
		ts->touch_point_max  = points;
		point->touch_point_max = points;
	}
	return n;
}

static DEVICE_ATTR(points, S_IRUGO | S_IWUSR,
		ts_points_show, ts_points_store);

static struct attribute *ts_attrs[] = {
	&dev_attr_calibration.attr,
	&dev_attr_rotate.attr,
	&dev_attr_points.attr,
	NULL,
};

static struct attribute_group ts_attr_group = {	.attrs = (struct attribute **) ts_attrs, };

static inline void ts_adjust_calibration(u16 *x, u16 *y)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_pointercal *point = &ts->pointer;
	TS_DEBUG1("[x:%4d,y:%4d]", *x, *y);
	if (90 == point->rotate) {
		int rx = (int)*x, ry = (int)*y;
		int tx = (int)*x;
		int yres = point->y_resol;
		rx = ry;
		ry = yres - tx;
		*x = (u16)rx, *y = (u16)ry;
		TS_DEBUG1("-> ro90 [x:%4d,y:%4d]", *x, *y);
	} else {
		if (point->enable) {
			int rx = (int)*x, ry = (int)*y;
			int tx = (int)*x, ty = (int)*y;
      		rx = ((point->pointercal[2] + point->pointercal[0]*tx + point->pointercal[1]*ty)
      				/ point->pointercal[6]);
      		ry = ((point->pointercal[5] + point->pointercal[3]*tx + point->pointercal[4]*ty)
      				/ point->pointercal[6]);
      		*x = (u16)rx, *y = (u16)ry;
	    	TS_DEBUG1("-> [x:%4d,y:%4d]", *x, *y);
		}
	}
	TS_DEBUG1("\n");
}

static void ts_setup_calibration(struct i2c_client *client)
{
	struct nxp_ts_cali_plat_data *plat = client->dev.platform_data;
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(client);
	struct ts_pointercal *point = &ts->pointer;
	struct kobject *kobj = NULL;
	int len = sizeof(int)*CAL_POINTER_NUM;
	int ret;

	TS_DEBUG("ts_setup_calibration\n");

	ts->touch_point_max = DEF_SUPPORT_POINT;

	/*
	 *  create attribute interface
	 */
	kobj = kobject_create_and_add("touch", &platform_bus.kobj);
	if (! kobj) {
		printk(KERN_ERR "Fail, create kobject for touch calibration ...\n");
		return;
	}
	ret = sysfs_create_group(kobj, &ts_attr_group);
	if (ret) {
		printk(KERN_ERR "Fail, create sysfs group for touch calibration...\n");
		kobject_del(kobj);
		return;
	}
	point->attr = &ts_attr_group;
	point->kobj = kobj;
    point->enable = 0;

	if (! plat)
		return;

	point->x_resol = plat->x_resol;
    point->y_resol = plat->y_resol;
    if ( 90 == plat->rotate ||
    	180 == plat->rotate ||
    	270 == plat->rotate) {
    	point->rotate = plat->rotate;
    }

	/* set calibration */
	if (plat->pointercal[0] != 0 || plat->pointercal[1] != 0 ||
		plat->pointercal[2] != 0 || plat->pointercal[3] != 0 ||
		plat->pointercal[4] != 0 || plat->pointercal[5] != 0 ||
		plat->pointercal[6] != 0 ) {
		memcpy(point->pointercal, plat->pointercal, len);
		point->enable = 1;
		TS_DEBUG("TS: pointercal {%d, %d, %d, %d, %d, %d, %d }\n",
			point->pointercal[0], point->pointercal[1], point->pointercal[2],
			point->pointercal[3], point->pointercal[4], point->pointercal[5],
			point->pointercal[6]);
	}

	if (0 > plat->touch_points ||
		plat->touch_points> MAX_SUPPORT_POINT) {
		printk("FT5X0X TS support touch points 1 ~ %d, default %d\n",
			MAX_SUPPORT_POINT, ts->touch_point_max);
		return;
	}

	point->touch_point_max = plat->touch_points;
	ts->touch_point_max = plat->touch_points;
}

static void ts_release_calibration(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(client);
	struct ts_pointercal *point = &ts->pointer;

	if (point->attr)
		sysfs_remove_group(point->kobj, point->attr);

    if (point->kobj)
    	kobject_del(point->kobj);
}

static int ft5x0x_i2c_read(char *rxdata, int length)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		}, {
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	return ret;
}

#if (0)
static int ft5x0x_i2c_write(char *txdata, int length)
{
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}

static int ft5x0x_i2c_set_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = ft5x0x_i2c_write(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    return 0;
}
#endif

static void ft5x0x_ts_release(void)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);

	TS_DEBUG("[R]");
#ifdef CONFIG_FT5X0X_MULTITOUCH
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_mt_sync(ts->input_dev);
#else
	input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
#endif
	input_sync(ts->input_dev);
}

#define	TOUCH_RELEASE		(0)
#define	TOUCH_PRESS			(1<<0)
#define	TOUCH_PRESSED		(1<<1)

static int ft5x0x_ts_value(void)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_event *event = &ts->event;
	u8 buf[64] = {0};
	int cnt = ts->touch_point_max;
	int ret = -1;

#ifdef CONFIG_FT5X0X_MULTITOUCH
	int len = (cnt * 6) + 1;
	int i = 0;
	ret = ft5x0x_i2c_read(buf, len);
#else
    ret = ft5x0x_i2c_read(buf, 7);
#endif
    if (0 > ret) {
		printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return ret;
	}

	event->touch_points = buf[2] & 0x0F;	// 000 0111
    if (event->touch_points == 0) {
        ft5x0x_ts_release();
        return 1;
    }

	if (event->touch_points > cnt)
		event->touch_points = cnt;

#ifdef CONFIG_FT5X0X_MULTITOUCH
	mutex_lock(&ts->lock);
    for (i = 0; event->touch_points > i; i++) {
    	short reg = 0x03 + (i*6);
		event->x [i] = (s16)((buf[reg + 0] & 0x0F) << 8) | (s16)buf[reg + 1];
		event->y [i] = (s16)((buf[reg + 2] & 0x0F) << 8) | (s16)buf[reg + 3];
		event->ID[i] = (i+1);
		event->st[i] = (s16)((buf[reg] & 0xc0) >> 6);
		if (1 != event->st[i])
			ts_adjust_calibration(&event->x[i], &event->y[i]);	/* Add by jhkim */
	}
	mutex_unlock(&ts->lock);
#else
    if (event->touch_points == 1) {
    	event->x[0] = (s16)(buf[0x03] & 0x0F)<<8 | (s16)buf[0x04];
		event->y[0] = (s16)(buf[0x05] & 0x0F)<<8 | (s16)buf[0x06];
		event->press[0] |= TOUCH_PRESS;
		/* Add by jhkim */
		ts_adjust_calibration(&event->x[0], &event->y[0]);
    }
#endif
    return 0;
}

static void ft5x0x_ts_report(void)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_event *event = &ts->event;
	int i = 0;

	TS_DEBUG("ft5x0x_ts_report: ");
#ifdef CONFIG_FT5X0X_MULTITOUCH
	mutex_lock(&ts->lock);
	for (i = 0; event->touch_points > i; i++) {
		if (1 != event->st[i]) {
			if (TOUCH_PRESS == event->press[i]) {
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, event->press[i]);
				event->press[i] |= TOUCH_PRESS;
			}
			event->press[i] |= TOUCH_PRESS;
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, event->ID[i]);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, event->x[i]);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, event->y[i]);
			input_mt_sync(ts->input_dev);
			TS_DEBUG("T%02d [ID %2d, x=%4d, y=%4d]\n", i, event->ID[i], event->x[i], event->y[i]);
		} else {
			TS_DEBUG("T%02d [ID %2d] release\n", i, event->ID[i]);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_mt_sync(ts->input_dev);
		}
	}
	input_sync(ts->input_dev);
	mutex_unlock(&ts->lock);

#else	/* CONFIG_FT5X0X_MULTITOUCH*/
	if (event->touch_points == 1) {
		input_report_abs(ts->input_dev, ABS_X, event->x[i]);
		input_report_abs(ts->input_dev, ABS_Y, event->y[i]);
		input_report_abs(ts->input_dev, ABS_PRESSURE, event->press[i]);
	}
	input_report_key(ts->input_dev, BTN_TOUCH, 1);
	input_sync(ts->input_dev);
#endif	/* CONFIG_FT5X0X_MULTITOUCH*/

#if 1
	TS_DEBUG1(" 1:(%4d, %4d)  2:(%4d, %4d)  3:(%4d, %4d)  4:(%4d, %4d)  5:(%4d, %4d)\n",
		event->x[0], event->y[0], event->x[1], event->y[1], event->x[2], event->y[2],
		event->x[3], event->y[3], event->x[4], event->y[4]);
	TS_DEBUG1(" 6:(%4d, %4d)  7:(%4d, %4d)  8:(%4d, %4d)  9:(%4d, %4d) 10:(%4d, %4d)\n",
		event->x[5], event->y[5], event->x[6], event->y[6], event->x[7], event->y[7],
		event->x[8], event->y[8], event->x[9], event->y[9]);
#endif

}	/*end ft5x0x_ts_report*/

static void ft5x0x_ts_event(struct work_struct *work)
{
	int ret = -1;
	TS_DEBUG("ft5x0x_ts_event\n");
	ret = ft5x0x_ts_value();
	if (0 == ret)
		ft5x0x_ts_report();
	else
		TS_DEBUG("ts package read error\n");

	//enable_irq(this_client->irq);
}

static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;

	TS_DEBUG("[I]\n");

  //disable_irq_nosync(irq);
	queue_delayed_work(ft5x0x_ts->ts_work,
		&ft5x0x_ts->event_work, DELAY_WORK_JIFFIES);
	return IRQ_HANDLED;
}

static int
ft5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x0x_ts_data *ts;
	struct input_dev *input_dev;
	int err = 0;

	TS_DEBUG("ft5x0x_ts_probe\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	this_client = client;
	i2c_set_clientdata(client, ts);

	INIT_DELAYED_WORK(&ts->event_work, ft5x0x_ts_event);
	ts->ts_work = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ts->ts_work) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}
	mutex_init(&ts->lock);

	err = request_irq(client->irq, ft5x0x_ts_interrupt,
				IRQF_DISABLED | IRQF_TRIGGER_FALLING, "ft5x0x_ts", ts);
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ts->input_dev = input_dev;

#ifdef CONFIG_FT5X0X_MULTITOUCH

    set_bit(EV_ABS, input_dev->evbit);

    input_set_abs_params(input_dev, ABS_MT_POSITION_X , 0, SCREEN_MAX_X-1, 0, 0);
    input_set_abs_params(input_dev, ABS_MT_POSITION_Y , 0, SCREEN_MAX_Y-1, 0, 0);
    input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, PRESS_MAX, 0 , 0);

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
#endif

	input_dev->name = FT5X0X_NAME;		//dev_name(&client->dev)
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
		"ft5x0x_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

	/* Add by jhkim */
	ts_setup_calibration(client);

	printk("touch: ft5x0x support %d point \n", ts->touch_point_max);
    return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	free_irq(client->irq, ts);
exit_irq_request_failed:
	cancel_work_sync(&ts->event_work.work);
	destroy_workqueue(ts->ts_work);
exit_create_singlethread:
	i2c_set_clientdata(client, NULL);
	kfree(ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(client);
	TS_DEBUG("ft5x0x_ts_remove\n");

	/* Add by jhkim */
	ts_release_calibration(client);

	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	cancel_work_sync(&ts->event_work.work);
	destroy_workqueue(ts->ts_work);
	i2c_set_clientdata(client, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int ft5x0x_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}

static int ft5x0x_ts_resume(struct i2c_client *client)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}

#else
#define ft5x0x_ts_suspend		NULL
#define ft5x0x_ts_resume		NULL
#endif

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{ FT5X0X_NAME, 0 },{ }
};
MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe		= ft5x0x_ts_probe,
	.remove		= __devexit_p(ft5x0x_ts_remove),
   	.suspend  	= ft5x0x_ts_suspend,
	.resume   	= ft5x0x_ts_resume,
	.id_table	= ft5x0x_ts_id,
	.driver	= {
		.name	= FT5X0X_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init ft5x0x_ts_init(void)
{
	return i2c_add_driver(&ft5x0x_ts_driver);
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");


