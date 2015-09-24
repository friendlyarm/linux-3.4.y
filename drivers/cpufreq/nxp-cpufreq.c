/*
 * (C) Copyright 2009
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
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/kthread.h>
#include <linux/sysrq.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/tags.h>

//#define pr_debug     printk

/*
 * DVFS info
 */
struct cpufreq_asv_ops {
	int  (*setup_table)(unsigned long (*tables)[2]);
	long (*get_voltage)(long freqkhz);
	int  (*modify_vol_table)(unsigned long (*tables)[2], int table_size,
							long val, bool dn, bool percent);
	int  (*current_label)(char *string);
	long (*get_vol_margin)(long uV, long val, bool dn, bool percent);
};

#include "nxp-cpufreq.h"

struct cpufreq_dvfs_time {
	unsigned long start;
	unsigned long duration;
};

struct cpufreq_dvfs_limits {
	int  run_monitor;
    long new_freq;		/* khz */
    long max_freq;		/* khz */
    long max_retent;	/* msec */
    long rest_freq;		/* khz */
    long rest_retent;	/* msec */
    long rest_period;
   	ktime_t rest_ktime;
   	struct hrtimer rest_hrtimer;
   	struct hrtimer restore_hrtimer;
   	struct task_struct *proc;
};

struct cpufreq_dvfs_info {
	struct cpufreq_frequency_table *freq_table;
	unsigned long (*freq_volts)[2];	/* khz freq (khz): voltage(uV): voltage (us) */
	struct clk *clk;
	cpumask_var_t cpus;
	int cpu;
	long target_freq;
	int  freq_id;
	struct mutex lock;
	/* limited max frequency */
	struct cpufreq_dvfs_limits limits;
    /* voltage control */
    struct regulator *volt;
    int table_size;
    long supply_delay_us;
    /* for suspend/resume */
    struct notifier_block pm_notifier;
    unsigned long resume_state;
    long boot_freq;
    int  boot_voltage;
    /* check frequency duration */
	int  pre_freq_id;
    unsigned long check_state;
    struct cpufreq_dvfs_time *freq_times;
    /* ASV operation */
    struct cpufreq_asv_ops *asv_ops;
};

#define	FREQ_TABLE_MAX			(30)
#define	FREQ_STATE_RESUME 		(0)	/* bit num */
#define	FREQ_STATE_TIME_RUN   	(0)	/* bit num */

static struct cpufreq_dvfs_info	*ptr_current_dvfs = NULL;
static unsigned long __freq_dvfs_tables[FREQ_TABLE_MAX][2];
static struct cpufreq_dvfs_time __freq_times[FREQ_TABLE_MAX] = { {0,}, };
#define	ms_to_ktime(m)			 ns_to_ktime((u64)m * 1000 * 1000)

static inline void set_dvfs_ptr(void *ptr) 	{ ptr_current_dvfs = ptr; }
static inline void *get_dvfs_ptr(void) 		{ return ptr_current_dvfs;	}

static struct tag_asv_margin tag_margin = { 0, };

static int __init parse_tag_arm_margin(const struct tag *tag)
{
	struct tag_asv_margin *t = (struct tag_asv_margin *)&tag->u;
	struct tag_asv_margin *p = &tag_margin;

	p->value = t->value;
	p->minus = t->minus;
	p->percent = t->percent;
	printk("ASV: Arm margin:%s%d%s\n",
		p->minus?"-":"+", p->value, p->percent?"%":"mV");
	return 0;
}
__tagtable(ATAG_ARM_MARGIN, parse_tag_arm_margin);

static int nxp_cpufreq_set_freq_id(struct cpufreq_dvfs_info *dvfs, unsigned long frequency)
{
	unsigned long (*freq_tables)[2] = (unsigned long(*)[2])dvfs->freq_volts;
	int len = dvfs->table_size;
	int id = 0;

	for (id = 0; len > id; id++) {
		if (frequency == freq_tables[id][0])
			break;
	}

	if (id == len) {
		printk("Fail : invalid frequency (%ld:%d) id !!!\n", frequency, len);
		return -EINVAL;
	}

	dvfs->freq_id = id;

	return 0;
}

static enum hrtimer_restart nxp_cpufreq_restore_timer(struct hrtimer *hrtimer)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();

	dvfs->limits.rest_ktime = ktime_set(0, 0);	/* clear */
	dvfs->limits.new_freq = dvfs->target_freq;	/* restore */

	pr_debug("cpufreq : restore %ldkhz after rest %4ldms\n",
		dvfs->target_freq, dvfs->limits.rest_retent);

	if (dvfs->target_freq > dvfs->limits.rest_freq) {
		wake_up_process(dvfs->limits.proc);

		/* to rest frequency after end of rest time */
		hrtimer_start(&dvfs->limits.rest_hrtimer,
			ms_to_ktime(dvfs->limits.max_retent), HRTIMER_MODE_REL_PINNED);
	}
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart nxp_cpufreq_rest_timer(struct hrtimer *hrtimer)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();

	dvfs->limits.rest_ktime = ktime_get();
	dvfs->limits.new_freq = dvfs->limits.rest_freq;

	pr_debug("cpufreq : %ldkhz (%4ldms) -> %ldkhz rest (%4ldms) \n",
		dvfs->limits.max_freq, dvfs->limits.max_retent,
		dvfs->limits.new_freq, dvfs->limits.rest_retent);

	wake_up_process(dvfs->limits.proc);

	/* to restore frequency after end of rest time */
	hrtimer_start(&dvfs->limits.restore_hrtimer,
		ms_to_ktime(dvfs->limits.rest_retent), HRTIMER_MODE_REL_PINNED);

	return HRTIMER_NORESTART;
}

static long nxp_cpufreq_voltage(unsigned long frequency, bool margin)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
 	unsigned long (*freq_volts)[2] = (unsigned long(*)[2])dvfs->freq_volts;
	long mS = 0, uS = 0, uV = 0, wT = 0;
	int id = dvfs->freq_id;

	if (!dvfs->volt)
		return 0;

	/* when rest duration */
	if (frequency != freq_volts[id][0]) {
		printk("%s: failed invalid freq %ld and id [%d] %ld !!!\n",
			__func__, frequency, id, freq_volts[id][0]);
		return -EINVAL;
	}

	uV = freq_volts[id][1];
	wT = dvfs->supply_delay_us;

	if (false == margin && dvfs->asv_ops->get_voltage) {
		uV = dvfs->asv_ops->get_voltage(freq_volts[id][0]);
		/*
		if (dvfs->asv_ops->get_vol_margin)
			uV = dvfs->asv_ops->get_vol_margin(uV, 13, false, false);
		*/
	}

	regulator_set_voltage(dvfs->volt, uV, uV);

	if (wT) {
		mS = wT/1000;
		uS = wT%1000;
		if (mS) mdelay(mS);
		if (uS) udelay(uS);
	}

#ifdef CONFIG_ARM_NXP_CPUFREQ_VOLTAGE_DEBUG
	printk(" volt (%lukhz %ld.%06ld v, delay %ld.%03ld us)\n",
			freq_volts[id][0], uV/1000000, uV%1000000, mS, uS);
#endif
	return uV;
}

static unsigned long nxp_cpufreq_update(struct cpufreq_dvfs_info *dvfs,
				struct cpufreq_freqs *freqs, bool margin)
{
	struct clk *clk = dvfs->clk;
	unsigned long rate = 0;
	int id = dvfs->freq_id;

	if (!test_bit(FREQ_STATE_RESUME, &dvfs->resume_state))
		return freqs->old;

	/* pre voltage */
	if (freqs->new >= freqs->old)
		nxp_cpufreq_voltage(freqs->new, margin);

	for_each_cpu(freqs->cpu, dvfs->cpus)
		cpufreq_notify_transition(freqs, CPUFREQ_PRECHANGE);

	rate = clk_set_rate(clk, freqs->new*1000);
	pr_debug(" set rate %ukhz\n", freqs->new);

	if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state)) {
		int prev = dvfs->pre_freq_id;
		long ms = ktime_to_ms(ktime_get());

		dvfs->freq_times[prev].duration += (ms - dvfs->freq_times[prev].start);
		dvfs->freq_times[id].start = ms;
		dvfs->pre_freq_id = id;
	}

	for_each_cpu(freqs->cpu, dvfs->cpus)
		cpufreq_notify_transition(freqs, CPUFREQ_POSTCHANGE);

	/* post voltage */
	if (freqs->old > freqs->new)
		nxp_cpufreq_voltage(freqs->new, margin);

	return rate;
}

static int nxp_cpufreq_pm_notify(struct notifier_block *this,
        unsigned long mode, void *unused)
{
	struct cpufreq_dvfs_info *dvfs = container_of(this,
					struct cpufreq_dvfs_info, pm_notifier);
	struct clk *clk = dvfs->clk;
	struct cpufreq_freqs freqs;
	long max_freq = cpufreq_quick_get_max(dvfs->cpu);

    switch(mode) {
    case PM_SUSPEND_PREPARE:	/* set initial frequecny */
		mutex_lock(&dvfs->lock);

		freqs.new = dvfs->boot_freq;
		if (freqs.new > max_freq) {
			freqs.new = max_freq;
			printk("DVFS: max freq %ldkhz less than boot %ldkz..\n",
				max_freq, dvfs->boot_freq);
		}

		freqs.old = clk_get_rate(clk)/1000;

		dvfs->target_freq = freqs.new;

		nxp_cpufreq_set_freq_id(dvfs, freqs.new);
		nxp_cpufreq_update(dvfs, &freqs, false);

    	clear_bit(FREQ_STATE_RESUME, &dvfs->resume_state);
		mutex_unlock(&dvfs->lock);
    	break;

    case PM_POST_SUSPEND:	/* set restore frequecny */
		mutex_lock(&dvfs->lock);
    	set_bit(FREQ_STATE_RESUME, &dvfs->resume_state);

		freqs.new = dvfs->target_freq;
		freqs.old = clk_get_rate(clk)/1000;

		nxp_cpufreq_set_freq_id(dvfs, freqs.new);
		nxp_cpufreq_update(dvfs, &freqs, true);

		mutex_unlock(&dvfs->lock);
    	break;
    }
    return 0;
}

static int nxp_cpufreq_proc_update(void *unused)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	struct cpufreq_freqs freqs;
	struct clk *clk = dvfs->clk;

	set_current_state(TASK_INTERRUPTIBLE);

	while (!kthread_should_stop()) {
		if (dvfs->limits.new_freq) {
			mutex_lock(&dvfs->lock);
			set_current_state(TASK_UNINTERRUPTIBLE);

			freqs.new = dvfs->limits.new_freq;
			freqs.old = clk_get_rate(clk)/1000;;
			freqs.cpu = dvfs->cpu;

			nxp_cpufreq_set_freq_id(dvfs, freqs.new);
			nxp_cpufreq_update(dvfs, &freqs, true);

			set_current_state(TASK_INTERRUPTIBLE);
			mutex_unlock(&dvfs->lock);
		}

		schedule();

		if (kthread_should_stop())
			break;

		set_current_state(TASK_INTERRUPTIBLE);
	}

	set_current_state(TASK_RUNNING);
	return 0;
}

static inline int nxp_cpufreq_setup(struct cpufreq_dvfs_info *dvfs)
{
	struct hrtimer *hrtimer = &dvfs->limits.rest_hrtimer;
	int cpu = 0;
	struct task_struct *p;

	dvfs->limits.run_monitor = 0;
	mutex_init(&dvfs->lock);

	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = nxp_cpufreq_rest_timer;

	hrtimer = &dvfs->limits.restore_hrtimer;
	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = nxp_cpufreq_restore_timer;

	p = kthread_create_on_node(nxp_cpufreq_proc_update,
				NULL, cpu_to_node(cpu), "cpufreq-update");
	if (IS_ERR(p)) {
		pr_err("%s: cpu%d: failed rest thread for cpufreq\n", __func__, cpu);
		return PTR_ERR(p);
	}
	kthread_bind(p, cpu);
	wake_up_process(p);

	dvfs->limits.proc = p;

	return 0;
}

/*
 * Attribute sys interfaces
 */
static ssize_t show_speed_duration(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	int id = dvfs->freq_id;
	ssize_t count = 0;
	int i = 0;

	if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state)) {
		long ms = ktime_to_ms(ktime_get());
		if (dvfs->freq_times[id].start)
			dvfs->freq_times[id].duration += (ms - dvfs->freq_times[id].start);
		dvfs->freq_times[id].start = ms;
		dvfs->pre_freq_id = id;
	}

	for (; dvfs->table_size > i; i++)
		count += sprintf(&buf[count], "%8ld ", dvfs->freq_times[i].duration);

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t store_speed_duration(struct cpufreq_policy *policy,
			const char *buf, size_t count)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	int id = dvfs->freq_id;
	long ms = ktime_to_ms(ktime_get());
	const char *s = buf;

	mutex_lock(&dvfs->lock);

	if (0 == strncmp(s, "run", strlen("run"))) {
		dvfs->pre_freq_id = id;
		dvfs->freq_times[id].start = ms;
		set_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state);
	}
	else if (0 == strncmp(s, "stop", strlen("stop"))) {
		clear_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state);
	}
	else if (0 == strncmp(s, "clear", strlen("clear"))) {
		memset(dvfs->freq_times, 0, sizeof(__freq_times));
		if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state)) {
			dvfs->freq_times[id].start = ms;
			dvfs->pre_freq_id = id;
		}
	} else {
		count = -1;
	}

	mutex_unlock(&dvfs->lock);

	return count;
}

static ssize_t show_available_voltages(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
 	unsigned long (*freq_volts)[2] = (unsigned long(*)[2])dvfs->freq_volts;
 	ssize_t count = 0;
	int i = 0;

	for (; dvfs->table_size > i; i++) {
		long uV = freq_volts[i][1];
		if (dvfs->asv_ops->get_voltage)
			uV = dvfs->asv_ops->get_voltage(freq_volts[i][0]);
		count += sprintf(&buf[count], "%ld ", uV);
	}

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t show_cur_voltages(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
 	unsigned long (*freq_volts)[2] = (unsigned long(*)[2])dvfs->freq_volts;
 	ssize_t count = 0;
	int i = 0;

	for (; dvfs->table_size > i; i++)
		count += sprintf(&buf[count], "%ld ", freq_volts[i][1]);

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t store_cur_voltages(struct cpufreq_policy *policy,
			const char *buf, size_t count)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	unsigned long (*dvfs_tables)[2] = (unsigned long(*)[2])__freq_dvfs_tables;
	bool percent = false, down = false;
	const char *s = strchr(buf, '-');
	long val;

	if (s)
		down = true;
	else
		s = strchr(buf, '+');

	if (!s)
		s = buf;
	else
		s++;

	if (strchr(buf, '%'))
		percent = 1;

	val = simple_strtol(s, NULL, 10);

	mutex_lock(&dvfs->lock);

	if (dvfs->asv_ops->modify_vol_table)
		dvfs->asv_ops->modify_vol_table(dvfs_tables, dvfs->table_size,
							val, down, percent);

	nxp_cpufreq_set_freq_id(dvfs, dvfs->target_freq);
	nxp_cpufreq_voltage(dvfs->target_freq, true);

	mutex_unlock(&dvfs->lock);
	return count;
}

static ssize_t show_asv_level(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	int ret = 0;

	if (dvfs->asv_ops->current_label)
		ret = dvfs->asv_ops->current_label(buf);

	return ret;
}

/*
 * show/store frequency duration time status
 */
static struct freq_attr cpufreq_freq_attr_scaling_speed_duration = {
    .attr = {
    	.name = "scaling_speed_duration",
		.mode = 0664,
	},
    .show  = show_speed_duration,
    .store = store_speed_duration,
};

/*
 * show available voltages each frequency
 */
static struct freq_attr cpufreq_freq_attr_scaling_available_voltages = {
    .attr = {
    	.name = "scaling_available_voltages",
		.mode = 0664,
	},
    .show  = show_available_voltages,
};

/*
 * show/store ASV current voltage adjust margin
 */
static struct freq_attr cpufreq_freq_attr_scaling_cur_voltages = {
    .attr = {
    	.name = "scaling_cur_voltages",
		.mode = 0664,
	},
    .show  = show_cur_voltages,
    .store = store_cur_voltages,
};

/*
 * show ASV level status
 */
static struct freq_attr cpufreq_freq_attr_scaling_asv_level = {
    .attr = {
    	.name = "scaling_asv_level",
		.mode = 0664,
	},
    .show  = show_asv_level,
};

static struct freq_attr *nxp_cpufreq_attr[] = {
	/* kernel attribute */
	&cpufreq_freq_attr_scaling_available_freqs,
	/* new sttribute */
	&cpufreq_freq_attr_scaling_speed_duration,
	&cpufreq_freq_attr_scaling_available_voltages,
	&cpufreq_freq_attr_scaling_cur_voltages,
	&cpufreq_freq_attr_scaling_asv_level,
	NULL,
};

static int nxp_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;

	if (!freq_table)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, freq_table);
}

static unsigned int nxp_cpufreq_get_speed(unsigned int cpu)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	struct clk *clk = dvfs->clk;
	long rate_khz = clk_get_rate(clk)/1000;

	return rate_khz;
}

static int nxp_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int target_freq,
				unsigned int relation)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;
	struct cpufreq_frequency_table *table;
	struct cpufreq_freqs freqs;
	unsigned long rate = 0;
	long ts;
	int ret = 0, i = 0;

	ret = cpufreq_frequency_table_target(policy, freq_table,
						target_freq, relation, &i);
	if (ret) {
		pr_err("%s: cpu%d: no freq match for %d khz(ret=%d)\n",
			__func__, policy->cpu, target_freq, ret);
		return ret;
	}

	mutex_lock(&dvfs->lock);

	table = &freq_table[i];
	freqs.new = table->frequency;

	if (!freqs.new) {
		pr_err("%s: cpu%d: no match for freq %d khz\n",
			__func__, policy->cpu, target_freq);
		mutex_unlock(&dvfs->lock);
		return -EINVAL;
	}

	freqs.old = nxp_cpufreq_get_speed(policy->cpu);
	freqs.cpu = policy->cpu;
	pr_debug("cpufreq : target %u -> %u khz mon(%s) ",
		freqs.old, freqs.new, dvfs->limits.run_monitor?"run":"no");

	if (freqs.old == freqs.new && policy->cur == freqs.new) {
		pr_debug("PASS\n");
		mutex_unlock(&dvfs->lock);
		return ret;
	}

	dvfs->cpu = policy->cpu;
	dvfs->target_freq = freqs.new;

	/* rest period */
	if (ktime_to_ms(dvfs->limits.rest_ktime) && freqs.new > dvfs->limits.rest_freq) {
		ts = (ktime_to_ms(ktime_get()) - ktime_to_ms(dvfs->limits.rest_ktime));
		if (dvfs->limits.rest_retent > ts) {
			/* set with rest freq */
			freqs.new = dvfs->limits.rest_freq;
			pr_debug("rest %4ld:%4ldms (%u khz)\n", dvfs->limits.rest_retent, ts, freqs.new);
			goto freq_up;	/* retry */
		}
		dvfs->limits.rest_ktime = ktime_set(0, 0);	/* clear rest time */
	}

	if (dvfs->limits.max_freq && dvfs->limits.run_monitor && freqs.new < dvfs->limits.max_freq ) {
		dvfs->limits.run_monitor = 0;
		hrtimer_cancel(&dvfs->limits.rest_hrtimer);
		pr_debug("stop monitor");
	}

	if (dvfs->limits.max_freq && !dvfs->limits.run_monitor && freqs.new >= dvfs->limits.max_freq) {
		dvfs->limits.run_monitor = 1;
		hrtimer_start(&dvfs->limits.rest_hrtimer, ms_to_ktime(dvfs->limits.max_retent),
			      HRTIMER_MODE_REL_PINNED);
		pr_debug("run  monitor");
	}

freq_up:
	nxp_cpufreq_set_freq_id(dvfs, freqs.new);
	rate = nxp_cpufreq_update(dvfs, &freqs, true);

	mutex_unlock(&dvfs->lock);

	return rate;
}

static int __cpuinit nxp_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;
	int res;

	pr_debug("nxp-cpufreq: available frequencies cpus (%d) \n",
		num_online_cpus());

	/* get policy fields based on the table */
	res = cpufreq_frequency_table_cpuinfo(policy, freq_table);
	if (!res) {
		cpufreq_frequency_table_get_attr(freq_table, policy->cpu);
	} else {
		pr_err("nxp-cpufreq: Failed to read policy table\n");
		return res;
	}

	policy->cur = nxp_cpufreq_get_speed(policy->cpu);
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;

	/*
	 * FIXME : Need to take time measurement across the target()
	 *	   function with no/some/all drivers in the notification
	 *	   list.
	 */
	policy->cpuinfo.transition_latency = 100000; /* in ns */

	/*
	 * multi-core processors has 2 cores
	 * that the frequency cannot be set independently.
	 * Each cpu is bound to the same speed.
	 * So the affected cpu is all of the cpus.
	 */
	if (num_online_cpus() == 1) {
		cpumask_copy(policy->related_cpus, cpu_possible_mask);
		cpumask_copy(policy->cpus, cpu_online_mask);
		cpumask_copy(dvfs->cpus, cpu_online_mask);
	} else {
		cpumask_setall(policy->cpus);
		cpumask_setall(dvfs->cpus);
	}

	return 0;
}

static struct cpufreq_driver nxp_cpufreq_driver = {
	.flags   = CPUFREQ_STICKY,
	.verify  = nxp_cpufreq_verify_speed,
	.target  = nxp_cpufreq_target,
	.get     = nxp_cpufreq_get_speed,
	.init    = nxp_cpufreq_init,
	.name    = "nxp-cpufreq",
	.attr    = nxp_cpufreq_attr,
};

static int nxp_cpufreq_probe(struct platform_device *pdev)
{
	struct nxp_cpufreq_plat_data *plat = pdev->dev.platform_data;
	unsigned long (*dvfs_tables)[2] = (unsigned long(*)[2])__freq_dvfs_tables;
	unsigned long (*plat_tables)[2] = NULL;
	struct cpufreq_asv_ops *asv_ops = &asv_freq_ops;
	static struct notifier_block *pm_notifier;
	struct cpufreq_dvfs_info *dvfs;
	struct cpufreq_frequency_table *table;
	struct tag_asv_margin *tag = &tag_margin;
	int cpu = raw_smp_processor_id();
	char name[16];
	int asv_tb_len = -1;
	int tbsize = 0, i = 0, n = 0;

	/* user defined dvfs */
	if (plat->freq_table && plat->table_size)
		plat_tables = (unsigned long(*)[2])plat->freq_table;

	/* asv dvfs tables */
	if (asv_ops->setup_table)
		asv_tb_len = asv_ops->setup_table(dvfs_tables);

	if (0 > asv_tb_len && NULL == plat_tables) {
		dev_err(&pdev->dev, "%s: failed no freq table !!!\n", __func__);
		return -EINVAL;
	}

	dvfs = kzalloc(sizeof(*dvfs), GFP_KERNEL);
	if (!dvfs) {
		dev_err(&pdev->dev, "%s: failed allocate DVFS data !!!\n", __func__);
		return -ENOMEM;
	}

	sprintf(name, "pll%d", plat->pll_dev);
	dvfs->clk = clk_get(NULL, name);
	if (IS_ERR(dvfs->clk))
		return PTR_ERR(dvfs->clk);

	tbsize = (NULL == plat_tables ? asv_tb_len : plat->table_size);
	table = kzalloc((sizeof(*table)*tbsize) + 1, GFP_KERNEL);
	if (!table) {
		dev_err(&pdev->dev, "%s: failed allocate freq table !!!\n", __func__);
		return -ENOMEM;
	}
	set_dvfs_ptr(dvfs);

	dvfs->asv_ops = asv_ops;
	dvfs->freq_table = table;
	dvfs->freq_volts = (unsigned long(*)[2])(dvfs_tables);
	dvfs->table_size = tbsize;
	dvfs->supply_delay_us = plat->supply_delay_us;
	dvfs->boot_freq = nxp_cpufreq_get_speed(cpu);
	dvfs->target_freq = dvfs->boot_freq;
	dvfs->pre_freq_id = -1;
	dvfs->check_state = 0;
	dvfs->freq_times = __freq_times;
	dvfs->limits.max_freq = plat->max_cpufreq;
	dvfs->limits.max_retent = plat->max_retention;
	dvfs->limits.rest_freq = plat->rest_cpufreq;
	dvfs->limits.rest_retent = plat->rest_retention;
	dvfs->limits.rest_ktime = ktime_set(0, 0);
	dvfs->limits.run_monitor = 0;

	/*
     * make frequency table with platform data
	 */
	if (asv_tb_len > 0) {
		for (n = 0, i = 0; tbsize > i && asv_tb_len > n; n++) {
			if (plat_tables) {
				for (n = 0; asv_tb_len > n; n++) {
					if (plat_tables[i][0] == dvfs_tables[n][0]) {
						dvfs_tables[i][0] = dvfs_tables[n][0];	/* frequency */
						dvfs_tables[i][1] = dvfs_tables[n][1];	/* voltage */
						break;
					}
				}
			} else {
				if (dvfs_tables[n][0] > FREQ_MAX_FREQ_KHZ)
					continue;
				dvfs_tables[i][0] = dvfs_tables[n][0];	/* frequency */
				dvfs_tables[i][1] = dvfs_tables[n][1];	/* voltage */
			}
			table->index = i;
			table->frequency = dvfs_tables[i][0];
			pr_debug("[%s] %2d = %8ldkhz, %8ld uV (%lu us)\n",
				name, i, dvfs_tables[i][0], dvfs_tables[i][1], dvfs->supply_delay_us);
			/* next */
			i++, table++;
		}
	} else {
		for (i = 0; tbsize > i; i++, table++) {
			dvfs_tables[i][0] = plat_tables[i][0];	/* frequency */
			dvfs_tables[i][1] = plat_tables[i][1];	/* voltage */
			table->index = i;
			table->frequency = dvfs_tables[i][0];
			pr_debug("[%s] %2d = %8ldkhz, %8ld uV (%lu us)\n",
				name, i, dvfs_tables[i][0], dvfs_tables[i][1], dvfs->supply_delay_us);
		}
	}

	table->index = i;
	table->frequency = CPUFREQ_TABLE_END;
	dvfs->table_size = table->index;	/* must be reset table size with table index */

	/* get voltage regulator */
	if (plat->supply_name) {
		dvfs->volt = regulator_get(NULL, plat->supply_name);
		if (IS_ERR(dvfs->volt)) {
			dev_err(&pdev->dev, "%s: Cannot get regulator for DVS supply %s\n",
				__func__, plat->supply_name);
			kfree(table);
			kfree(dvfs);
			return -1;
		}
		dvfs->boot_voltage = regulator_get_voltage(dvfs->volt);

		pm_notifier = &dvfs->pm_notifier;
		pm_notifier->notifier_call = nxp_cpufreq_pm_notify;
		if (register_pm_notifier(pm_notifier)) {
			dev_err(&pdev->dev, "%s: Cannot pm notifier %s\n",
				__func__, plat->supply_name);
			return -1;
		}

		/* set margin voltage */
		if (tag->value && dvfs->asv_ops->modify_vol_table)
			dvfs->asv_ops->modify_vol_table(dvfs_tables, tbsize,
								tag->value, tag->minus, tag->percent);

		/* chnage to margin voltage */
		if (tag->value) {
			nxp_cpufreq_voltage(dvfs->boot_freq, true);
			printk("DVFS: adjust %ld margin %s%d%s \n",
				dvfs->boot_freq, tag->minus?"-":"+", tag->value, tag->percent?"%":"mV");
		}
	}

	set_bit(FREQ_STATE_RESUME, &dvfs->resume_state);
	nxp_cpufreq_set_freq_id(dvfs, dvfs->boot_freq);

	if (0 > nxp_cpufreq_setup(dvfs))
		return -EINVAL;

	printk("DVFS: cpu %s with PLL.%d [tables=%d]\n",
		dvfs->volt?"DVFS":"DFS", plat->pll_dev, dvfs->table_size);

	return cpufreq_register_driver(&nxp_cpufreq_driver);
}

static struct platform_driver cpufreq_driver = {
	.driver	= {
	.name	= DEV_NAME_CPUFREQ,
	.owner	= THIS_MODULE,
	},
	.probe	= nxp_cpufreq_probe,
};
module_platform_driver(cpufreq_driver);
