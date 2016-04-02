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
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/hwmon.h>
#include <linux/hwmon-vid.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/cpufreq.h>
#include <linux/platform_device.h>

#include <mach/platform.h>
#include <mach/devices.h>

/*
#define	pr_debug	printk
*/

#define DRVNAME	"nxp-tmu"
#define	CHECK_CHARGE_STATE			0
#define	ADJUST_EFUSE_TRIM			1
/*
 * if efuse is zero, assume limit 85 ~= 115 : C = R - efuse + 25
 */
#define	EFUSE_ZERO_TRIM				55

#define	CHECK_CHARGE_DURATION		(500)

struct tmu_trigger {
	int	  trig_degree;
	long  trig_duration;
	long  trig_cpufreq;
	long  new_cpufreq;
	ulong expire_time;
	bool  triggered;
   	bool  limited;
};

struct tmu_info {
	struct device *hwmon_dev;
	const char *name;
	int channel;
	struct tmu_trigger *triggers;
	int trigger_size;
	int poll_duration;
	int temp_label;
	int temp_max;
	long max_cpufreq;
	long min_cpufreq;
	long old_cpufreq;
	long new_cpufreq;
	long limit_cpufreq;
	int  is_limited;
	struct mutex mlock;
	/* TMU HW */
	int tmu_trimv;
	int tmu_trimv85;
	/* TMU func */
	struct delayed_work mon_work;
#if (CHECK_CHARGE_STATE)
	struct delayed_work chg_work;
#endif
	unsigned long state;
	void (*callback)(int ch, int temp, bool run);
};

#define	STATE_SUSPEND_ENTER		(0)		/* bit position */
#define	STATE_STOP_ENTER		(1)		/* bit position */
#define	STATE_MON_RUNNING		(1<<1)	/* bit position */
#define TMU_POLL_TIME			(500)	/* ms */

#define TMU_IRQ_MAX				3	/* ms */
#define TMU_CPU_FREQ_STEP		100000	/* khz */

static int nxp_tmu_frequency(long new)
{
	char *file = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
	mm_segment_t old_fs;
	char buf[32];
	long max = 0;

	int fd = sys_open(file, O_RDWR, 0);
	if (0 > fd)
		return -EINVAL;

   	old_fs = get_fs();
	set_fs(KERNEL_DS);
	sys_read(fd, (void*)buf, sizeof(buf));

	max = simple_strtoul(buf, NULL, 10);
	if (max != new) {
		sprintf(buf, "%ld", new);
		sys_write(fd, (void*)buf, sizeof(buf));
		pr_debug("Max Freq %8ld khz\n", new);
	}

	set_fs(old_fs);
	sys_close(fd);
	return 0;
}

#if (CHECK_CHARGE_STATE)
extern int axp_get_charging_type(void);
static bool is_charging(void)
{
	int chg_type = axp_get_charging_type();

	/*
	 * 0 = no charging
	 * 1 = AC USB Charger
	 * 2 = PC USB Charger
	 * 3 = AC Charger
	 */
	if (1 == chg_type || 3 == chg_type)
		return true;
	return false;
}

#define	ms_to_ktime(m)	 ns_to_ktime((u64)m * 1000 * 1000)
static void check_charger_state(struct work_struct *work)
{
	struct tmu_info *info = container_of(work, struct tmu_info, chg_work.work);

	mutex_lock(&info->mlock);

	/* get charge status */
	if (info->limit_cpufreq && false == is_charging())
		info->new_cpufreq = info->limit_cpufreq;
	else
		info->new_cpufreq = info->max_cpufreq;

	pr_debug("tmu limited:%s, %ld, %ld chg %s\n", info->is_limited?"O":"X",
		info->old_cpufreq, info->new_cpufreq, is_charging()?"O":"X");

	if (!info->is_limited &&
		(info->new_cpufreq != info->old_cpufreq)) {
		if (0 == nxp_tmu_frequency(info->new_cpufreq))
			info->old_cpufreq = info->new_cpufreq;
	}

	mutex_unlock(&info->mlock);

	schedule_delayed_work(&info->chg_work,
		msecs_to_jiffies(CHECK_CHARGE_DURATION));
}
#endif

/*
 * TMU operation
 */
#define	TIME_100US	0x6B3	// 0x4305
#define	TIME_20us 	0x16A	// 0xE29
#define	TIME_2us 	0x024	// 0x170

static int nxp_tmu_start(struct tmu_info *info)
{
	int channel = info->channel;
	u32 mode = 7;
	int time = 1000;

	NX_TMU_SetBaseAddress(channel, (void*)IO_ADDRESS(NX_TMU_GetPhysicalAddress(channel)));
	NX_TMU_ClearInterruptPendingAll(channel);
	NX_TMU_SetInterruptEnableAll(channel, CFALSE);

	// Set CounterValue0, CounterValue1
	NX_TMU_SetCounterValue0(channel, ((TIME_20us<<16) | TIME_100US));
	NX_TMU_SetCounterValue1(channel, ((TIME_100US<<16) | TIME_2us));
	NX_TMU_SetSamplingInterval(channel, 0x1);

	// Emulstion mode enable
	NX_TMU_SetTmuEmulEn(channel, CFALSE);

	// Interrupt Enable
	NX_TMU_SetP0IntEn(channel, 0x0);

	// Thermal tripping mode selection
	NX_TMU_SetTmuTripMode(channel, mode);

	// Thermal tripping enable
	NX_TMU_SetTmuTripEn(channel, 0x0);

	// Check sensing operation is idle
	while (time-- > 0 && NX_TMU_IsBusy(channel)) { msleep(1); }

	NX_TMU_SetTmuStart(channel, CTRUE);
	return 0;
}

static void nxp_tmu_stop(struct tmu_info *info)
{
	int channel = info->channel;
	NX_TMU_SetTmuStart(channel, CFALSE);
}

static inline void nxp_tmu_trim_ready(struct tmu_info *info)
{
	int channel = info->channel;
	int trimv = 0, trimv85 = 0;
	u32 done = 0;

#if ADJUST_EFUSE_TRIM
	int count = 10;

	while (count-- > 0) {
	    // Program the measured data to e-fuse
    	u32 val = readl((void*)IO_ADDRESS((PHY_BASEADDR_TIEOFF_MODULE + (76*4))));
    	val = val | 0x3;

    	writel(val, (void*)IO_ADDRESS((PHY_BASEADDR_TIEOFF_MODULE + (76*4))));

    	// e-fuse Sensing Done. Check.
    	val = readl((void*)IO_ADDRESS((PHY_BASEADDR_TIEOFF_MODULE + (76*4))));
    	done = (((val>>3) & 0x3) == 0x3);
    	if (done)
    		break;
    	mdelay(1);
    }

	trimv = NX_TMU_GetTriminfo25(channel);
	trimv85 = NX_TMU_GetTriminfo85(channel);
#endif

	if (0 == trimv || !done)
		trimv = EFUSE_ZERO_TRIM;

	info->tmu_trimv = trimv;
	info->tmu_trimv85 = trimv85;
	printk("tmu[%d] = trim %d:%d, ret %d\n", channel, trimv, trimv85, done);
}

static inline int nxp_tmu_temp(struct tmu_info *info)
{
	int channel = info->channel;
	int val = NX_TMU_GetCurrentTemp0(channel);	// can't use temp1
	int ret = (val - info->tmu_trimv + 25);

	pr_debug("tmu[%d] = %d (%d - %d + 25)\n", channel, ret, val, info->tmu_trimv);
	return ret;
}

static irqreturn_t nxp_tmu_interrupt(int irq, void *desc)
{
	struct tmu_info *info = desc;
	struct tmu_trigger *trig = info->triggers;
	int channel = info->channel;
	u32 mask = NX_TMU_GetP0IntEn(channel);
	int i = 0;

	pr_debug("tmu[%d] irq temp %d stat 0x%x\n",
		channel, nxp_tmu_temp(info), NX_TMU_GetP0IntStat(channel));

	/* disable triggered irq */
	for (i = 0; TMU_IRQ_MAX > i; i++) {
		if ((1<<(i*4)) & NX_TMU_GetP0IntStat(channel)) {
			NX_TMU_SetP0IntClear(channel, 1<<(i*4));
			NX_TMU_SetP0IntEn(channel, mask & ~(1<<(i*4)));
			trig[i].triggered = true;
		}
	}
	schedule_work(&info->mon_work.work);

	return IRQ_HANDLED;
}

static int nxp_tmu_triggers(struct nxp_tmu_platdata *plat, struct tmu_info *info)
{
	struct tmu_trigger *trig = NULL;
	struct nxp_tmu_trigger *data = plat->triggers;
	int channel = info->channel;
	u32 temp_rise = 0, temp_intr = 0, trim_rise = 0;
	int err = 0, i = 0;

	nxp_tmu_start(info);

	if (!plat->triggers || !plat->trigger_size)
		return 0;

	if (plat->trigger_size > 3) {
		printk("tmu.%d max trigger count %d\n", channel, TMU_IRQ_MAX);
		return -EINVAL;
	}

	trig = kzalloc(sizeof(*trig)*plat->trigger_size, GFP_KERNEL);
	if (!trig) {
		pr_err("%s: Out of memory\n", __func__);
		return -ENOMEM;
	}

	info->triggers = trig;
	info->trigger_size = plat->trigger_size;
	nxp_tmu_trim_ready(info);

	err = request_irq(IRQ_PHY_TMU0, &nxp_tmu_interrupt, IRQF_DISABLED, DRVNAME, info);
	if (err) {
		pr_err("Fail, tmu.%d request interrupt %d ...\n", channel, IRQ_PHY_TMU0);
		return -EINVAL;
	}

	for (i = 0; plat->trigger_size > i; i++) {
		trig[i].trig_degree = data[i].trig_degree & 0xFF;
		trig[i].trig_duration = data[i].trig_duration;
		trig[i].trig_cpufreq = data[i].trig_cpufreq ?
				data[i].trig_cpufreq : info->max_cpufreq-TMU_CPU_FREQ_STEP;
		trig[i].new_cpufreq = trig[i].trig_cpufreq;
		trig[i].expire_time = 0;

		/*
		 * Calibrated threshold	temperature is written
		 * into THRES_TEMP_RISE and THRES_TEMP_FALL
		 */
		if (TMU_IRQ_MAX > i) {
			trim_rise = trig[i].trig_degree - 25 + info->tmu_trimv;
			temp_rise |= trim_rise << (i*8);
			temp_intr |= 1<<(i*4);
		}
		pr_debug("tmu[%d] = %3d (%ldms) -> %8ld kzh (0x%08x: rise %d)\n",
			i, trig[i].trig_degree, trig[i].trig_duration,
			trig[i].trig_cpufreq, temp_rise, trim_rise);
	}

#if (CHECK_CHARGE_STATE)
	if (info->limit_cpufreq) {
		INIT_DELAYED_WORK(&info->chg_work, check_charger_state);
		schedule_delayed_work(&info->chg_work,
			msecs_to_jiffies(CHECK_CHARGE_DURATION));
	}
#endif
	NX_TMU_SetThresholdTempRise(channel, temp_rise);
	NX_TMU_ClearInterruptPendingAll(channel);
	NX_TMU_SetP0IntEn(channel, temp_intr);

	/* to check init temp */
	for (i = 0; info->trigger_size > i; i++)
		trig[i].triggered = 1;

	schedule_delayed_work(&info->mon_work, msecs_to_jiffies(100));

	return 0;
}

static void nxp_tmu_monitor(struct work_struct *work)
{
	struct tmu_info *info = container_of(work, struct tmu_info, mon_work.work);
	struct tmu_trigger *trig = info->triggers;
	int size = info->trigger_size;
	int channel = info->channel;
	int temp, i = 0;
	u32 need_reschedule = (1<<size)-1;
	ulong current_time = 0;

	mutex_lock(&info->mlock);

	if (test_bit(STATE_SUSPEND_ENTER, &info->state)) {
		mutex_unlock(&info->mlock);
		return;
	}

	current_time = ktime_to_ms(ktime_get());
	temp = nxp_tmu_temp(info);
	info->temp_label = temp;
	if (temp > info->temp_max)
		info->temp_max = temp;

	for (i = 0; size > i; i++)
		info->is_limited |= trig[i].limited ? (1<<i): 0;

	for (i = 0; size > i; i++, trig++) {

		if (false == trig->triggered) {
			need_reschedule &= ~(1<<i);
			continue;
		}

		if (0 == trig->expire_time)
			trig->expire_time = current_time + trig->trig_duration;

		pr_debug("tmu[%d] = %3d:%3d~%6ldms (%s:0x%x)\n", i, temp, trig->trig_degree,
			trig->trig_duration-(trig->expire_time-current_time),
			trig->limited?"O":"X", info->is_limited);

		if (temp >= trig->trig_degree) {
			if (current_time >= trig->expire_time) {

				if (info->min_cpufreq > trig->new_cpufreq)
					continue;

				if (0 > nxp_tmu_frequency(trig->new_cpufreq))
					continue;

				/*
				 * disable irq to check polling
				 */
				NX_TMU_SetP0IntClear(channel, 1<<(i*4));
				NX_TMU_SetP0IntEn(channel, NX_TMU_GetP0IntEn(channel) & ~(1<<(i*4)));

				trig->new_cpufreq -= TMU_CPU_FREQ_STEP;
				trig->limited = true;
				info->is_limited |= (1<<i);
			}
		} else {

			info->is_limited &= ~(1<<i);

			/*
			 * enable irq to detect over temp
			 */
			NX_TMU_SetP0IntClear(channel, 1<<(i*4));
			NX_TMU_SetP0IntEn(channel, NX_TMU_GetP0IntEn(channel) | 1<<(i*4));

			if ((0 == info->is_limited) && trig->limited) {
				if (0 > nxp_tmu_frequency(info->new_cpufreq))
					continue;
			}

			trig->new_cpufreq = trig->trig_cpufreq;
			trig->expire_time = 0;
			trig->triggered = false;
			trig->limited = false;

			need_reschedule &= ~(1<<i);
		}
	}
	clear_bit(STATE_MON_RUNNING, &info->state);

	if (need_reschedule) {
		set_bit(STATE_MON_RUNNING, &info->state);
		schedule_delayed_work(&info->mon_work,
				msecs_to_jiffies(info->poll_duration));
	}

	mutex_unlock(&info->mlock);
	return;
}

#ifdef CONFIG_PM
static int nxp_tmu_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct tmu_info *info = platform_get_drvdata(pdev);
	int channel = info->channel;

	mutex_lock(&info->mlock);
	set_bit(STATE_SUSPEND_ENTER, &info->state);

	nxp_tmu_stop(info);
	nxp_tmu_frequency(info->max_cpufreq);

	NX_TMU_ClearInterruptPendingAll(channel);
	NX_TMU_SetP0IntEn(channel, 0x0);

	mutex_unlock(&info->mlock);
	return 0;
}

static int nxp_tmu_resume(struct platform_device *pdev)
{
	struct tmu_info *info = platform_get_drvdata(pdev);
	struct tmu_trigger *trig = info->triggers;
	int channel = info->channel;
	u32 temp_rise = 0, temp_intr = 0, trim_rise = 0;
	int i = 0;

	clear_bit(STATE_SUSPEND_ENTER, &info->state);

	for (i = 0; info->trigger_size > i; i++) {
		if (TMU_IRQ_MAX > i) {
			trim_rise = trig[i].trig_degree - 25 + info->tmu_trimv;
			temp_rise |= trim_rise << (i*8);
			temp_intr |= 1<<(i*4);

		}
	}

	nxp_tmu_start(info);

	NX_TMU_SetThresholdTempRise(channel, temp_rise);
	NX_TMU_ClearInterruptPendingAll(channel);
	NX_TMU_SetP0IntEn(channel, temp_intr);

	if (test_bit(STATE_MON_RUNNING, &info->state))
		schedule_delayed_work(&info->mon_work,
				msecs_to_jiffies(info->poll_duration));
	return 0;
}
#else
#define nxp_tmu_suspend NULL
#define nxp_tmu_resume NULL
#endif

/*
 * Sysfs
 */
enum { SHOW_TEMP, SHOW_LABEL, SHOW_NAME };

static ssize_t tmu_show_temp(struct device *dev,
			 struct device_attribute *devattr, char *buf)
{
	struct tmu_info *info = dev_get_drvdata(dev);
	char *s = buf;

	mutex_lock(&info->mlock);
	s += sprintf(s, "%4d\n", nxp_tmu_temp(info));
	mutex_unlock(&info->mlock);

	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t tmu_show_max(struct device *dev,
			 struct device_attribute *devattr, char *buf)
{
	struct tmu_info *info = dev_get_drvdata(dev);
	char *s = buf;
	int temp;

	mutex_lock(&info->mlock);

	temp = nxp_tmu_temp(info);

	if (temp > info->temp_max)
		info->temp_max = temp;
	else
		temp = info->temp_max;

	s += sprintf(s, "%4d\n", temp);

	mutex_unlock(&info->mlock);

	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t tmu_show_trim(struct device *dev,
			 struct device_attribute *devattr, char *buf)
{
	struct tmu_info *info = dev_get_drvdata(dev);
	char *s = buf;

	mutex_lock(&info->mlock);
	s += sprintf(s, "%d:%d\n", info->tmu_trimv, info->tmu_trimv85);
	mutex_unlock(&info->mlock);

	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static SENSOR_DEVICE_ATTR(temp_label, 0666, tmu_show_temp , NULL, SHOW_LABEL);
static SENSOR_DEVICE_ATTR(temp_max  , 0666, tmu_show_max  , NULL, SHOW_LABEL);
static SENSOR_DEVICE_ATTR(temp_trim , 0666, tmu_show_trim , NULL, SHOW_LABEL);

static struct attribute *temp_attr[] = {
	&sensor_dev_attr_temp_label.dev_attr.attr,
	&sensor_dev_attr_temp_max.dev_attr.attr,
	&sensor_dev_attr_temp_trim.dev_attr.attr,
	NULL
};

static const struct attribute_group tmu_attr_group = {
	.attrs = temp_attr,
};

static int __devinit nxp_tmu_probe(struct platform_device *pdev)
{
	struct nxp_tmu_platdata *plat = pdev->dev.platform_data;
	struct tmu_info *info = NULL;
	struct cpufreq_policy policy = { .cpuinfo = { .min_freq = 0, .min_freq = 0 }, };
	int err = -1;
	char name[16] ;

	if (!plat) {
		pr_err("%s: no platform data ....\n", __func__);
		return -EINVAL;
	}

	info = kzalloc(sizeof(struct tmu_info), GFP_KERNEL);
	if (!info) {
		pr_err("%s: Out of memory\n", __func__);
		return -ENOMEM;
	}

	sprintf(name, "tmu.%d", plat->channel);

	cpufreq_get_policy(&policy, 0);

	info->name = DRVNAME;
	info->channel = plat->channel;
	info->poll_duration = plat->poll_duration ? plat->poll_duration : TMU_POLL_TIME;
	info->callback = plat->callback;
	info->min_cpufreq = policy.cpuinfo.min_freq;
	info->max_cpufreq = policy.cpuinfo.max_freq;
	info->new_cpufreq = info->max_cpufreq;
	info->old_cpufreq = info->max_cpufreq;
#if (CHECK_CHARGE_STATE)
	info->limit_cpufreq = plat->limit_cpufreq;
#endif
	clear_bit(STATE_SUSPEND_ENTER, &info->state);
	clear_bit(STATE_MON_RUNNING, &info->state);

	mutex_init(&info->mlock);

	INIT_DELAYED_WORK(&info->mon_work, nxp_tmu_monitor);

	if (0 > nxp_tmu_triggers(plat, info))
		goto exit_free;

	platform_set_drvdata(pdev, info);

	err = sysfs_create_group(&pdev->dev.kobj, &tmu_attr_group);
	if (err)
		goto exit_free;

	info->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(info->hwmon_dev)) {
		err = PTR_ERR(info->hwmon_dev);
		pr_err("%s: Class registration failed (%d)\n", __func__, err);
		goto exit_remove;
	}

	printk("TMU: register %s to hwmon (max %ldkhz)\n", name, info->max_cpufreq);
	return 0;

exit_remove:
	sysfs_remove_group(&pdev->dev.kobj, &tmu_attr_group);

exit_free:
	platform_set_drvdata(pdev, NULL);
	if (info->triggers)
		kfree(info->triggers);

	kfree(info);

	return err;
}

static int __devexit nxp_tmu_remove(struct platform_device *pdev)
{
	struct tmu_info *info = platform_get_drvdata(pdev);
	struct tmu_trigger *trig = info->triggers;

	nxp_tmu_stop(info);

	hwmon_device_unregister(info->hwmon_dev);
	sysfs_remove_group(&pdev->dev.kobj, &tmu_attr_group);
	platform_set_drvdata(pdev, NULL);
	kfree(info);
	kfree(trig);

	return 0;
}

static struct platform_driver nxp_tmu_driver = {
	.driver = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
	.probe = nxp_tmu_probe,
	.remove	= __devexit_p(nxp_tmu_remove),
	.suspend = nxp_tmu_suspend,
	.resume = nxp_tmu_resume,
};

static int __init nxp_tmp_init(void) {
    return platform_driver_register(&nxp_tmu_driver);
}
late_initcall(nxp_tmp_init);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("SLsiAP temperature monitor");
MODULE_LICENSE("GPL");
