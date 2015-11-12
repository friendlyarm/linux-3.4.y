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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

#include <mach/platform.h>
#include <mach/devices.h>

/*
#define pr_debug(msg...)		printk(msg)
*/

#define	KEY_STAT_PRESS			(0)
#define	KEY_STAT_RELEASE		(1)
#define	DELAY_WORK_JIFFIES 		(1)
#define	PWR_KEY_RESUME_DELAY 	(500)	/* ms */

struct key_code {
	struct delayed_work kcode_work;
	struct workqueue_struct *kcode_wq;
	struct key_info *info;
	unsigned int io;
	unsigned int keycode;
	unsigned int val;
	unsigned int keystat;		/* current detect mode */
	unsigned int detect_high;	/* detect edge */
	int irq_disabled;
};

struct key_info {
	struct input_dev *input;
	int	keys;
	struct key_code *code;
	struct delayed_work	resume_work;
	int	resume_delay_ms;
};

#define	CHECK_PWR_KEY_EVENT(io)		nxp_check_pm_wakeup_dev("power key", io)

struct input_dev *key_input = NULL;
EXPORT_SYMBOL_GPL(key_input);

void nxp_key_power_event(void)
{
	if (key_input) {
		input_report_key(key_input, KEY_POWER, 1);
    	input_sync(key_input);
    	input_report_key(key_input, KEY_POWER, 0);
    	input_sync(key_input);
    }
}
EXPORT_SYMBOL(nxp_key_power_event);

static void nxp_key_event_wq(struct work_struct *work)
{
	struct key_code *code = (struct key_code *)work;
	struct key_info *key = code->info;
	unsigned int keycode = code->keycode;
	int press = 0;
	u_long flags;

	local_irq_save(flags);

	press = gpio_get_value_cansleep(code->io);
	if (code->detect_high)
		press = !press;

	local_irq_restore(flags);

	if(press != code->keystat) {
		code->keystat = press;
		if (KEY_STAT_PRESS == press) {
			input_report_key(key->input, keycode, 1);
			input_sync(key->input);
		} else {
			input_report_key(key->input, keycode, 0);
			input_sync(key->input);
		}
		pr_debug("key io:%d, code:%4d %s\n", code->io, keycode,
			(KEY_STAT_PRESS==press?"DN":"UP"));
	}
}

static irqreturn_t nxp_key_irqhnd(int irqno, void *dev_id)
{
	struct key_code *code = dev_id;

	queue_delayed_work(code->kcode_wq,
				&code->kcode_work, DELAY_WORK_JIFFIES);

	return IRQ_HANDLED;
}

static void nxp_key_resume_work(struct work_struct *work)
{
	struct key_info *key = container_of(work, struct key_info, resume_work.work);
	struct key_code *code = key->code;
	int i = 0;

	for (i = 0; key->keys > i; i++, code++) {
		if (code->keycode == KEY_POWER &&
			code->irq_disabled) {
			code->irq_disabled = 0;
			enable_irq(gpio_to_irq(code->io));
			break;
		}
	}
}

static int nxp_key_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct key_info *key = platform_get_drvdata(pdev);
	struct key_code *code = key->code;
	int i = 0;

	for (i = 0; key->keys > i; i++, code++) {
		if (code->keycode == KEY_POWER &&
			code->irq_disabled) {
			code->irq_disabled = 0;
			enable_irq(gpio_to_irq(code->io));
			break;
		}
	}

	return 0;
}

static int nxp_key_resume(struct platform_device *pdev)
{
	struct key_info *key = platform_get_drvdata(pdev);
	struct key_code *code = key->code;
	int i = 0;

	PM_DBGOUT("%s\n", __func__);

	for (i = 0; key->keys > i; i++, code++) {
		if (code->keycode == KEY_POWER &&
			CHECK_PWR_KEY_EVENT(code->io)) {

			int delay = key->resume_delay_ms ? key->resume_delay_ms : PWR_KEY_RESUME_DELAY;

			input_report_key(key->input, KEY_POWER, 1);
			input_sync(key->input);
			input_report_key(key->input, KEY_POWER, 0);
	    	input_sync(key->input);

	    	code->irq_disabled = 1;
			disable_irq(gpio_to_irq(code->io));

			schedule_delayed_work(&key->resume_work, msecs_to_jiffies(delay));
		}
	}

	return 0;
}

static int nxp_key_probe(struct platform_device *pdev)
{
	struct nxp_key_plat_data * plat = pdev->dev.platform_data;
	struct key_info *key = NULL;
	struct key_code *code = NULL;
	struct input_dev *input = NULL;
	int i, keys;
	int ret = 0;

	pr_debug("%s (device name:%s, id:%d)\n", __func__, pdev->name, pdev->id);

	key = kzalloc(sizeof(struct key_info), GFP_KERNEL);
	if (! key) {
		pr_err("fail, %s allocate driver info ...\n", pdev->name);
		return -ENOMEM;
	}

	keys = plat->bt_count;
	code = kzalloc(sizeof(struct key_code)*keys, GFP_KERNEL);
	if (NULL == code) {
		pr_err("fail, %s allocate key code ...\n", pdev->name);
		ret = -ENOMEM;
		goto err_mm;
	}

	input = input_allocate_device();
	if (NULL == input) {
		pr_err("fail, %s allocate input device\n", pdev->name);
		ret = -ENOMEM;
		goto err_mm;
	}

	key->input = input;
	key->keys = keys;
	key->code = code;
	key->resume_delay_ms = plat->resume_delay_ms;
	key_input = key->input;

	INIT_DELAYED_WORK(&key->resume_work, nxp_key_resume_work);

	input->name	= "Nexell Keypad";
	input->phys = "nexell/input0";
	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0002;
	input->id.version = 0x0100;
	input->dev.parent = &pdev->dev;
	input->keycode = plat->bt_code;
	input->keycodesize = sizeof(plat->bt_code[0]);
	input->keycodemax = plat->bt_count * 2;	// for long key
	input->evbit[0] = BIT_MASK(EV_KEY);

	if (plat->bt_repeat)
		 input->evbit[0] |= BIT_MASK(EV_REP);

	input_set_capability(input, EV_MSC, MSC_SCAN);
	input_set_drvdata(input, key);

	ret = input_register_device(input);
	if (ret) {
		pr_err("fail, %s register for input device ...\n", pdev->name);
		goto err_mm;
	}

	for (i=0; keys > i; i++, code++) {
		code->io = plat->bt_io[i];
		code->keycode = plat->bt_code[i];
		code->detect_high = plat->bt_detect_high ? plat->bt_detect_high[i]: 0;
		code->val = i;
		code->info = key;
		code->keystat = KEY_STAT_RELEASE;

	    code->kcode_wq = create_singlethread_workqueue(pdev->name);
		if (!code->kcode_wq) {
    	   ret = -ESRCH;
    	   goto err_irq;
	    }

		ret = request_irq(gpio_to_irq(code->io), nxp_key_irqhnd,
					(IRQF_SHARED | IRQ_TYPE_EDGE_BOTH), pdev->name, code);
		if (ret) {
			pr_err("fail, gpio[%d] %s request irq...\n", code->io, pdev->name);
			goto err_irq;
		}

		__set_bit(code->keycode, input->keybit);
		INIT_DELAYED_WORK(&code->kcode_work, nxp_key_event_wq);
		pr_debug("[%d] key io=%3d, code=%4d\n", i, code->io, code->keycode);
	}

	platform_set_drvdata(pdev, key);

	return ret;

err_irq:
	for (--i; i >= 0; i--) {
		cancel_work_sync(&code[i].kcode_work.work);
	    destroy_workqueue(code[i].kcode_wq);
		free_irq(gpio_to_irq(code[i].io), &code[i]);
	}
	input_free_device(input);

err_mm:
	if (code)
		kfree(code);

	if (key)
		kfree(key);

	return ret;
}

static int nxp_key_remove(struct platform_device *pdev)
{
	struct key_info *key = platform_get_drvdata(pdev);
	struct key_code *code = key->code;
	int i = 0, irq;

	pr_debug("%s\n", __func__);

	input_free_device(key->input);

	for (i = 0; i < key->keys; i++) {
		cancel_work_sync(&code[i].kcode_work.work);
	    destroy_workqueue(code[i].kcode_wq);
		irq = gpio_to_irq(code[i].io);
		free_irq(irq, &code[i]);
	}

	if (code)
		kfree(code);

	if (key)
		kfree(key);

	return 0;
}

static struct platform_driver key_plat_driver = {
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME_KEYPAD,
	},
	.probe		= nxp_key_probe,
	.remove		= nxp_key_remove,
	.suspend	= nxp_key_suspend,
	.resume		= nxp_key_resume,
};

static int __init nxp_key_init(void)
{
	return platform_driver_register(&key_plat_driver);
}

static void __exit nxp_key_exit(void)
{
	platform_driver_unregister(&key_plat_driver);
}

module_init(nxp_key_init);
module_exit(nxp_key_exit);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Keypad driver for the Nexell board");
MODULE_LICENSE("GPL");

