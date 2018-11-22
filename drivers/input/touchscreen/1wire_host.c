/*
 * 1wire_host.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LCD-CPU one wire communication for Mini2451 from
 *         FriendlyARM Guangzhou CO., LTD.
 *
 * Copyright (c) 2010 FriendlyARM Guangzhou CO., LTD.  <http://www.arm9.net>
 *
 * ChangeLog
 * 2017-03-23: Support for I2C interface
 *
 * 2015-12-23: Support for s5p4418
 *
 * 2015-06-23: Uses CH2 and legacy PWM interface
 *
 * 2010-10-14: Russell Guo <russell.grey@gmail.com>
 *      - Initial version
 *      -- request touch-screen data
 *      -- request LCD type, Firmware version
 *      -  Backlight control
 *
 * the CRC-8 functions is based on web page from http://lfh1986.blogspot.com
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/pwm.h>
#include <linux/i2c.h>
#include <linux/reboot.h>

#include <linux/platform_data/touchscreen-one-wire.h>
#include <linux/platform_data/ctouch.h>


#define TOUCH_DEVICE_NAME	"touchscreen-1wire"
#define BACKLIGHT_DEVICE_NAME	"backlight-1wire"

#define SAMPLE_BPS		9600

#define REQ_KEY			0x30U
#define REQ_TS			0x40U
#define REQ_INFO		0x60U

struct onewire_ts_priv {
	struct i2c_client *client;
	struct workqueue_struct *queue;
	struct work_struct work;
	struct mutex mutex;
	int sample_delay_ms;
	int irq;
};

/* Driver data */
static int bus_type = -1;
static struct onewire_ts_priv *onewire_priv;
static struct ts_onewire_platform_data *pdata;
static struct pwm_device *pwm;
static struct notifier_block reboot_nb;

#ifdef CONFIG_AUTO_REPORT_1WIRE_INPUT
static int invert_x, invert_y, swap_xy;
static int abs_x[2], abs_y[2];

extern void onewire_input_report(int x, int y, int pressed);
extern void onewire_input_set_params(int min_x, int max_x, int min_y, int max_y);
#endif

//---------------------------------------------------------
// Touch Screen driver interface

static DECLARE_WAIT_QUEUE_HEAD(ts_waitq);
static int ts_ready;
static int has_ts_data = 1;
static int has_key_data = 0;
static unsigned ts_status;
static int resumed = 0;

static struct input_dev *ts_if = NULL;
static int key_codes[] = { 172, KEY_MENU, KEY_BACK };
static int last_key = 0;

static void ts_if_report_key(int key) {
	int changed = last_key ^ key;
	int down = (key > last_key);
	int i;

	if (!changed || !ts_if)
		return;

	last_key = key;

	for (i = 0; i < ARRAY_SIZE(key_codes); i++) {
		if (changed & (1 << i)) {
			//printk("ts-if: %02x, %d [%d] ... %s\n",
			//              changed, i, key_codes[i], down ? "down" : "UP");
			input_report_key(ts_if, key_codes[i], down);
		}
	}

	input_sync(ts_if);
	return;
}

void register_ts_if_dev(struct input_dev *dev) {
	int i;

	set_bit(EV_KEY, dev->evbit);

	for (i = 0; i < ARRAY_SIZE(key_codes); i++) {
		input_set_capability(dev, EV_KEY, key_codes[i]);
	}

	ts_if = dev;
}

static inline void notify_ts_data(unsigned x, unsigned y, unsigned down)
{
	if (!down && !(ts_status & (1U << 31))) {
		// up repeat, give it up
		return;
	}

	ts_status = ((x << 16) | (y)) | (down << 31);
	ts_ready = 1;
	wake_up_interruptible(&ts_waitq);

#ifdef CONFIG_AUTO_REPORT_1WIRE_INPUT
	if (swap_xy)
		swap(x, y);
	if (invert_x)
		x = 4095 - x;
	if (invert_y)
		y = 4095 - y;
	pr_debug("onewire: %d, %d  %s\n", x, y, down ? "down" : "up");

	onewire_input_report(x, y, down);
#endif
}

static ssize_t ts_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	unsigned long err;

	if (!ts_ready) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else
			wait_event_interruptible(ts_waitq, ts_ready);
	}

	ts_ready = 0;

	if (count < sizeof ts_status) {
		return -EINVAL;
	} else {
		count = sizeof ts_status;
	}

	err = copy_to_user((void *)buffer, (const void *)(&ts_status), sizeof ts_status);
	return err ? -EFAULT : sizeof ts_status;
}

static unsigned int ts_poll( struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &ts_waitq, wait);

	if (ts_ready)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static struct file_operations ts_fops = {
	.owner		= THIS_MODULE,
	.read		= ts_read,
	.poll		= ts_poll,
};

static struct miscdevice ts_misc = {
	.minor		= 181,
	.name		= TOUCH_DEVICE_NAME,
	.fops		= &ts_fops,
};

//---------------------------------------------------------
// backlight

static DECLARE_WAIT_QUEUE_HEAD(bl_waitq);
static int bl_ready;
static unsigned char backlight_req = 0;
static unsigned char backlight_init_success;

static inline void notify_bl_data(unsigned char a, unsigned char b, unsigned char c)
{
	bl_ready = 1;
	backlight_init_success = 1;
	wake_up_interruptible(&bl_waitq);
}

static ssize_t bl_write(struct file *file, const char *buffer, size_t count, loff_t * ppos)
{
	int ret;
	char buf[4] = {0, 0, 0, 0};
	unsigned v;
	unsigned len;

	if (count == 0) {
		return -EINVAL;
	}

	if (count > sizeof buf - 1) {
		len = sizeof buf - 1;
	} else {
		len = count;
	}

	ret = copy_from_user(buf, buffer, len);
	if (ret) {
		return -EFAULT;
	}

	if (sscanf(buf, "%u", &v) != 1) {
		return -EINVAL;
	}

	if (v > 127) {
		v = 127;
	}

	bl_ready = 0;
	backlight_req = v + 0x80U;

	if (bus_type == BUS_I2C) {
		queue_work(onewire_priv->queue, &onewire_priv->work);
	}

	ret = wait_event_interruptible_timeout(bl_waitq, bl_ready, HZ / 10);
	if (ret < 0) {
		return ret;
	}
	if (ret == 0) {
		return -ETIMEDOUT;
	}

	return count;
}

static struct file_operations bl_fops = {
	.owner		= THIS_MODULE,
	.write		= bl_write,
};

static struct miscdevice bl_misc = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= BACKLIGHT_DEVICE_NAME,
	.fops		= &bl_fops,
};

//---------------------------------------------------------
// for query base info

static unsigned lcd_type, firmware_ver;

static inline void notify_info_data(unsigned char _lcd_type,
		unsigned char ver_year, unsigned char week)
{
	if (_lcd_type != 0xFF) {
		lcd_type = _lcd_type;
		firmware_ver = ver_year * 100 + week;

		/* Currently only S702 has hard key */
		if (lcd_type == 24) {
			has_key_data = 1;
		}
	}
}

//---------------------------------------------------------
// Pin access

static inline void set_pin_up(void) {
#ifdef CONFIG_PLAT_SAMSUNG
	s3c_gpio_setpull(pdata->gpio, S3C_GPIO_PULL_UP);
#endif
}

static inline void set_pin_as_input(void) {
	gpio_direction_input(pdata->gpio);
}

static inline void set_pin_as_output(void) {
	gpio_direction_output(pdata->gpio, 1);
}

static inline void set_pin_value(int v) {
	if (v) {
		gpio_set_value(pdata->gpio, 1);
	} else {
		gpio_set_value(pdata->gpio, 0);
	}
}

static inline int get_pin_value(void) {
	return gpio_get_value(pdata->gpio);
}

//---------------------------------------------------------
// CRC

static const unsigned char crc8_tab[] = {
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
	0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
	0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
	0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
	0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
	0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
	0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
	0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
	0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
	0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
	0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
	0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
	0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
	0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
	0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
	0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
	0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3,
};

#define crc8_init(crc) ((crc) = 0XACU)
#define crc8(crc, v) ( (crc) = crc8_tab[(crc) ^(v)])

//---------------------------------------------------------
// once a session complete

static unsigned total_received, total_error;
static unsigned last_req, last_res;

static void one_wire_session_complete(unsigned char req, unsigned int res)
{
	unsigned char crc;
	const unsigned char *p = (const unsigned char*)&res;

	total_received++;
	last_res = res;

	crc8_init(crc);
	crc8(crc, p[3]);
	crc8(crc, p[2]);
	crc8(crc, p[1]);

	if (crc != p[0]) {
		// CRC dismatch
		if (total_received > 100) {
			total_error++;
		}
		return;
	}

	switch (req) {
		case REQ_KEY:
			ts_if_report_key(p[1]);
			break;

		case REQ_TS:
			{
				unsigned short x,y;
				unsigned pressed;
				x =  ((p[3] >>   4U) << 8U) + p[2];
				y =  ((p[3] &  0xFU) << 8U) + p[1];
				pressed = (x != 0xFFFU) && (y != 0xFFFU);
				notify_ts_data(x, y, pressed);
			}
			break;

		case REQ_INFO:
			notify_info_data(p[3], p[2], p[1]);
			break;
		default:
			notify_bl_data(p[3], p[2], p[1]);
			break;
	}
}

//---------------------------------------------------------
// pwm helper

static int init_timer_for_1wire(void)
{
	int period_ns = NSEC_PER_SEC / SAMPLE_BPS;
	return pwm_config(pwm, period_ns >> 1, period_ns);
}

static inline void stop_timer_for_1wire(void)
{
	pwm_disable(pwm);
}

static inline void enable_tint(void)
{
	if (pdata->pwm_reg_tint) {
		unsigned int tint;
		tint = __raw_readl(pdata->pwm_reg_tint);
		tint |= (1 << (pdata->pwm_id + 5) | (1 << pdata->pwm_id));
		__raw_writel(tint, pdata->pwm_reg_tint);
	}
}

static inline void clear_tint(void)
{
	if (pdata->pwm_reg_tint) {
		unsigned int tint;
		tint = __raw_readl(pdata->pwm_reg_tint);
		tint |= (1 << (pdata->pwm_id + 5));
		__raw_writel(tint, pdata->pwm_reg_tint);
	}
}

//---------------------------------------------------------
// one-wire protocol core

enum {
	IDLE,
	START,
	REQUEST,
	WAITING,
	RESPONSE,
	STOPING,
} one_wire_status = IDLE;

static volatile unsigned int io_bit_count;
static volatile unsigned int io_data;
static volatile unsigned char one_wire_request;

static irqreturn_t timer_for_1wire_interrupt(int irq, void *dev_id)
{
	clear_tint();

	io_bit_count--;
	switch(one_wire_status) {
	case START:
		if (io_bit_count == 0) {
			io_bit_count = 16;
			one_wire_status = REQUEST;
		}
		break;

	case REQUEST:
		// Send a bit
		set_pin_value(io_data & (1U << 31));
		io_data <<= 1;
		if (io_bit_count == 0) {
			io_bit_count = 2;
			one_wire_status = WAITING;
		}
		break;

	case WAITING:
		if (io_bit_count == 0) {
			io_bit_count = 32;
			one_wire_status = RESPONSE;
		}
		if (io_bit_count == 1) {
			set_pin_as_input();
			set_pin_value(1);
		}
		break;

	case RESPONSE:
		// Get a bit
		io_data = (io_data << 1) | get_pin_value();
		if (io_bit_count == 0) {
			io_bit_count = 2;
			one_wire_status = STOPING;
			set_pin_value(1);
			set_pin_as_output();
			one_wire_session_complete(one_wire_request, io_data);
		}
		break;

	case STOPING:
		if (io_bit_count == 0) {
			one_wire_status = IDLE;
			stop_timer_for_1wire();
		}
		break;

	default:
		stop_timer_for_1wire();
	}

	return IRQ_HANDLED;
}

static struct irqaction timer_for_1wire_irq = {
	.name    = "onewire_pwm_irq",
	.flags   = IRQF_IRQPOLL,
	.handler = timer_for_1wire_interrupt,
	.dev_id  = &timer_for_1wire_irq,
};

static void start_one_wire_session(unsigned char req)
{
	unsigned char crc;
	unsigned long flags;

	if (one_wire_status != IDLE) {
		//printk("one_wire_status: %d\n", one_wire_status);
		return;
	}

	// IDLE to START
	one_wire_status = START;

	set_pin_value(1);
	set_pin_as_output();

	// prepare data for transfering
	crc8_init(crc);
	crc8(crc, req);
	io_data = (req << 8) + crc;
	io_data <<= 16;

	last_req = (io_data >> 16);
	one_wire_request = req;
	io_bit_count = 1;

	// init transfer and start timer
	local_irq_save(flags);
	set_pin_value(0);
	pwm_enable(pwm);
	enable_tint();
	local_irq_restore(flags);
}

//---------------------------------------------------------
// poll the device using linux timer

static int exitting;
static struct timer_list one_wire_timer;

static int timer_interval = HZ / 50;

void one_wire_timer_proc(unsigned long v)
{
	unsigned char req;

	if (exitting) {
		return;
	}

	if (lcd_type == 0 && total_received > 15) {
		return;
	}

	one_wire_timer.expires = jiffies + timer_interval;
	add_timer(&one_wire_timer);

	if (lcd_type == 0) {
		req = REQ_INFO;
	} else if (!backlight_init_success) {
		req = 127;
	} else if (backlight_req) {
		req = backlight_req;
		backlight_req = 0;
	} else if (has_key_data) {
		req = REQ_KEY;
	} else if (has_ts_data) {
		req = REQ_TS;
	} else {
		return;
	}

	start_one_wire_session(req);
}

static struct timer_list one_wire_timer = {
	.function = one_wire_timer_proc,
};

//---------------------------------------------------------
// driver interface

static int ts_1wire_probe(struct platform_device *pdev)
{
	int ret;

	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}

	ret = gpio_request(pdata->gpio, TOUCH_DEVICE_NAME);
	if (ret) {
		dev_err(&pdev->dev, "failed to request gpio %d\n", pdata->gpio);
		return ret;
	}

	set_pin_as_output();
	set_pin_value(1);
	set_pin_up();

	/* PWM */
	pwm = pwm_request(pdata->pwm_id, TOUCH_DEVICE_NAME);
	if (IS_ERR(pwm)) {
		dev_err(&pdev->dev, "unable to request legacy PWM\n");
		ret = PTR_ERR(pwm);
		goto err_alloc;
	}

	dev_dbg(&pdev->dev, "got pwm for %s\n", TOUCH_DEVICE_NAME);

	ret = setup_irq(pdata->timer_irq, &timer_for_1wire_irq);
	if (ret) {
		dev_err(&pdev->dev, "request irq %d failed\n", pdata->timer_irq);
		goto err_irq;
	}

	ret = init_timer_for_1wire();
	if (ret) {
		dev_err(&pdev->dev, "setup timer failed\n");
		goto err_timer;
	}

	init_timer(&one_wire_timer);
	one_wire_timer_proc(0);

	return 0;

err_timer:
	free_irq(pdata->timer_irq, &timer_for_1wire_irq);
err_irq:
	pwm_free(pwm);
err_alloc:
	set_pin_value(0);
	gpio_free(pdata->gpio);

	return ret;
}

static int ts_1wire_remove(struct platform_device *pdev)
{
	del_timer_sync(&one_wire_timer);
	free_irq(pdata->timer_irq, &timer_for_1wire_irq);

	pwm_free(pwm);

	set_pin_value(0);
	gpio_free(pdata->gpio);

	return 0;
}

#ifdef CONFIG_PM
static int ts_1wire_suspend(struct device *dev)
{
	printk("ts_1wire_suspend: before \n");

	del_timer_sync(&one_wire_timer);
	disable_irq(pdata->timer_irq);
	gpio_free(pdata->gpio);

	printk("ts_1wire_suspend: ok \n");

	return 0;
}

static int ts_1wire_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	int ret;

	printk("ts_1wire_resume: \n");

	ret = gpio_request(pdata->gpio, TOUCH_DEVICE_NAME);
	if (ret) {
		dev_err(&pdev->dev, "failed to request gpio %d\n", pdata->gpio);
	}

	set_pin_as_output();
	set_pin_value(1);
	set_pin_up();

	lcd_type = 0;
	backlight_req = 0;
	backlight_init_success = 0;
	one_wire_status = IDLE;
	io_bit_count = 0;
	io_data = 0;
	one_wire_request = 0;
	resumed = 1;

	ret = init_timer_for_1wire();
	init_timer(&one_wire_timer);
	one_wire_timer_proc(0);

	enable_irq(pdata->timer_irq);
	enable_tint();

	return 0;
}

static const struct dev_pm_ops ts_1wire_pm_ops = {
	.suspend	= ts_1wire_suspend,
	.resume		= ts_1wire_resume,
};
#endif

static struct platform_driver ts_1wire_device_driver = {
	.probe		= ts_1wire_probe,
	.remove		= ts_1wire_remove,
	.driver		= {
		.name	= "onewire_ts",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm		= &ts_1wire_pm_ops,
#endif
	}
};

//---------------------------------------------------------

#ifdef CONFIG_PROC_FS
static int ts_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%u %u %u %u %04X %08X\n",
			lcd_type, firmware_ver,
			total_received, total_error, last_req, last_res);
	return 0;
}

static int ts_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ts_proc_show, NULL);
}

static const struct file_operations ts_proc_fops = {
	.open		= ts_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init ts_proc_init(void)
{
	struct proc_dir_entry *r;

	r = proc_create("driver/one-wire-info", 0, NULL, &ts_proc_fops);
	if (!r)
		return -ENOMEM;
	return 0;
}
#else
static inline int ts_proc_init(void) { return 0; }
#endif /* CONFIG_PROC_FS */

//---------------------------------------------------------

static int onewire_handle_request(struct i2c_client *client,
		unsigned char req, unsigned char *buf)
{
	unsigned char tx[4];
	unsigned char crc;
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr   = client->addr,
#if defined(CONFIG_ARCH_S5P4418) && \
   !defined(CONFIG_I2C_NXP_PORT2_GPIO_MODE)
			.flags  = I2C_M_IGNORE_NAK,
#else
			.flags  = 0,
#endif
			.len    = 2,
			.buf    = tx,
		}, {
			.addr   = client->addr,
			.flags  = I2C_M_RD,
			.len    = 4,
			.buf    = buf,
		},
	};

	crc8_init(crc);
	crc8(crc, req);
	tx[0] = req;
	tx[1] = crc;

	ret = i2c_transfer(client->adapter, &msgs[0], 1);
	if (ret < 0) {
		pr_err("onewire: REQ 0x%02x: i2c write error %d\n", req, ret);
		return ret;
	}

	if (!buf)
		return 0;

	ret = i2c_transfer(client->adapter, &msgs[1], 1);
	if (ret < 0) {
		pr_err("onewire: REQ 0x%02x: i2c read error %d\n", req, ret);
		return ret;
	}

	crc8_init(crc);
	crc8(crc, buf[0]);
	crc8(crc, buf[1]);
	crc8(crc, buf[2]);
	pr_debug("onewire: resp %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);

	if (crc != buf[3]) {
		pr_err("onewire: REQ 0x%02X: crc error (%02x <--> %02x)\n",
				req, crc, buf[3]);
		total_error++;
		return -EIO;
	}

	return 0;
}

static void onewire_ts_work_func(struct work_struct *work) {
	struct onewire_ts_priv *priv = container_of(work,
			struct onewire_ts_priv, work);
	unsigned char buf[4];
	unsigned short x,y;
	unsigned pressed;
	int ret;

	do {
		ret = onewire_handle_request(priv->client, REQ_TS, buf);
		if (!ret) {
			x =  ((buf[0] >>   4U) << 8U) + buf[1];
			y =  ((buf[0] &  0xFU) << 8U) + buf[2];
			pressed = (x != 0xFFFU) && (y != 0xFFFU);

			notify_ts_data(x, y, pressed);
		}

		/* handle backlight request */
		if (backlight_req) {
			onewire_handle_request(priv->client, backlight_req, NULL);

			backlight_req = 0;
			bl_ready = 1;
			backlight_init_success = 1;
			wake_up_interruptible(&bl_waitq);
		}

		msleep(priv->sample_delay_ms);

	} while (pressed);
}

static irqreturn_t onewire_ts_isr(int irq, void *dev_id)
{
	struct onewire_ts_priv *priv = dev_id;

	last_req = REQ_TS;
	if (!work_pending(&priv->work)) {
		queue_work(priv->queue, &priv->work);
	}

	return IRQ_HANDLED;
}

static int onewire_identify_chip(struct i2c_client *client)
{
	unsigned char id[4];

	if (onewire_handle_request(client, REQ_INFO, id) < 0)
		return -1;

	notify_info_data(id[0], id[1], id[2]);

	return 0;
}

static int onewire_ts_probe(struct i2c_client *client,
		const struct i2c_device_id *idp)
{
	struct onewire_ts_priv *priv;
	unsigned int ctp_id;
	int ret;

#if defined(CONFIG_TOUCHSCREEN_GOODIX) || \
	defined(CONFIG_TOUCHSCREEN_FT5X0X) || \
	defined(CONFIG_TOUCHSCREEN_HIMAX)  || \
	defined(CONFIG_TOUCHSCREEN_IT7260)
	ctp_id = board_get_ctp();
	if (ctp_id != CTP_NONE && ctp_id != CTP_AUTO) {
		has_ts_data = 0;
		timer_interval = HZ / 25;
	}
#endif

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		goto err_nodev;

	if (onewire_identify_chip(client) < 0)
		goto err_nodev;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		ret = -ENOMEM;
		goto err_nomem;
	}

	priv->client = client;
	priv->sample_delay_ms = 12;
	mutex_init(&priv->mutex);
	INIT_WORK(&priv->work, onewire_ts_work_func);

	priv->queue = create_singlethread_workqueue("onewire-ts");
	if (!priv->queue) {
		ret = -ENOMEM;
		goto err_wq;
	}

	if (has_ts_data) {
		/* tell user app (tscal.sh) to do calibrate */
		total_received = 256;

		priv->irq = client->irq;
	}

	if (priv->irq > 0) {
		ret = request_irq(priv->irq, onewire_ts_isr,
				IRQ_TYPE_EDGE_FALLING, client->name, priv);
		if (ret) {
			dev_err(&client->dev, "failed to request IRQ %d\n", priv->irq);
			goto err_irq;
		}
	}

	onewire_priv = priv;
	dev_set_drvdata(&client->dev, priv);

	bus_type = BUS_I2C;
	dev_info(&client->dev, "found panel %d, rev %04d\n", lcd_type, firmware_ver);

	return 0;

err_irq:
	cancel_work_sync(&priv->work);
	destroy_workqueue(priv->queue);

err_wq:
	kfree(priv);

err_nomem:
	dev_err(&client->dev, "probe Onewire touchscreen failed, %d\n", ret);
	return ret;

err_nodev:
	/* fallback to 1-wire protocol */
	return platform_driver_register(&ts_1wire_device_driver);
}

static int __devexit onewire_ts_remove(struct i2c_client *client) {
	struct onewire_ts_priv *priv = dev_get_drvdata(&client->dev);

	if (bus_type == BUS_I2C) {
		if (priv->irq) {
			free_irq(priv->irq, priv);
		}

		cancel_work_sync(&priv->work);
		destroy_workqueue(priv->queue);

		kfree(priv);

		dev_set_drvdata(&client->dev, NULL);

	} else {
		exitting = 1;
		platform_driver_unregister(&ts_1wire_device_driver);
	}

	return 0;
}

static const struct i2c_device_id onewire_ts_id[] = {
	{ "ONEWIRE", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, onewire_ts_id);

static struct i2c_driver onewire_ts_driver = {
	.driver = {
		.name = "ONEWIRE-ts",
	},
	.probe = onewire_ts_probe,
	.remove = __devexit_p(onewire_ts_remove),
	.id_table = onewire_ts_id,
};

//---------------------------------------------------------

static int onewire_reboot_handler(struct notifier_block *this,
		unsigned long mode, void *cmd)
{
	backlight_req = 0x80U;
	if (bus_type == BUS_I2C)
		queue_work(onewire_priv->queue, &onewire_priv->work);

	wait_event_interruptible_timeout(bl_waitq, bl_ready, HZ / 10);
	pr_info("onewire: backlight off\n");

	return NOTIFY_DONE;
}

static int __init onewire_dev_init(void)
{
	int ret;

	ret = misc_register(&ts_misc);
	if (ret)
		goto fail_ts;

	printk(TOUCH_DEVICE_NAME "\tinitialized\n");

	ret = misc_register(&bl_misc);
	if (ret)
		goto fail_bl;

	printk(BACKLIGHT_DEVICE_NAME "\tinitialized\n");

	ts_proc_init();

	ret = i2c_add_driver(&onewire_ts_driver);
	if (ret)
		goto fail_drv;

	reboot_nb.notifier_call = onewire_reboot_handler;
	reboot_nb.priority = 192;
	if (register_reboot_notifier(&reboot_nb)) {
		printk("onewire: failed to register reboot notifier\n");
		reboot_nb.notifier_call = NULL;
	}

	return 0;

fail_drv:
	misc_deregister(&bl_misc);
fail_bl:
	misc_deregister(&ts_misc);
fail_ts:
	return ret;
}

static void __exit onewire_dev_exit(void)
{
	if (reboot_nb.notifier_call)
		unregister_reboot_notifier(&reboot_nb);

	i2c_del_driver(&onewire_ts_driver);

	remove_proc_entry("driver/one-wire-info", NULL);

	misc_deregister(&ts_misc);
	misc_deregister(&bl_misc);
}

module_init(onewire_dev_init);
module_exit(onewire_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");
MODULE_DESCRIPTION("One-wire host and Touch Screen Driver");

