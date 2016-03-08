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
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/tags.h>

/*
#define pr_debug     printk
*/

#define DEV_NAME_CPUFREQ	"nxp-cpufreq"

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

#if    defined (CONFIG_ARCH_S5P4418)
#include "s5p4418-cpufreq.h"
#elif defined (CONFIG_ARCH_S5P6818)
#include "s5p6818-cpufreq.h"
#else
#define	FREQ_MAX_FREQ_KHZ		(1400*1000)
#define	FREQ_ARRAY_SIZE			(11)
static struct cpufreq_asv_ops 	asv_freq_ops = { };
#endif

struct cpufreq_dvfs_timestamp {
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
	unsigned long (*dvfs_table)[2];	/* khz freq (khz): voltage(uV): voltage (us) */
	struct clk *clk;
	cpumask_var_t cpus;
	struct cpufreq_policy *policy;
	int cpu;
	long target_freq;
	int  freq_point;
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
    long boot_frequency;
    int  boot_voltage;
    int  fixed_uV;
    /* check frequency duration */
	int  pre_freq_point;
    unsigned long check_state;
    struct cpufreq_dvfs_timestamp *time_stamp;
    /* ASV operation */
    struct cpufreq_asv_ops *asv_ops;
};

#define	FREQ_TABLE_MAX			(30)
#define	FREQ_STATE_RESUME 		(0)	/* bit num */
#define	FREQ_STATE_TIME_RUN   	(0)	/* bit num */

static struct cpufreq_dvfs_info	*ptr_current_dvfs = NULL;
static unsigned long dvfs_freq_voltage[FREQ_TABLE_MAX][2];
static struct cpufreq_dvfs_timestamp dvfs_timestamp[FREQ_TABLE_MAX] = { {0,}, };
#define	ms_to_ktime(m)	ns_to_ktime((u64)m * 1000 * 1000)

static inline void set_dvfs_ptr(void *dvfs) 	{ ptr_current_dvfs = dvfs; }
static inline void *get_dvfs_ptr(void) 			{ return ptr_current_dvfs;	}

static struct tag_asv_margin asv_margin = { 0, };

static int __init parse_tag_arm_margin(const struct tag *tag)
{
	struct tag_asv_margin *t = (struct tag_asv_margin *)&tag->u;
	struct tag_asv_margin *p = &asv_margin;

	p->value = t->value;
	p->minus = t->minus;
	p->percent = t->percent;
	printk("ASV: Arm margin:%s%d%s\n",
		p->minus?"-":"+", p->value, p->percent?"%":"mV");
	return 0;
}
__tagtable(ATAG_ARM_MARGIN, parse_tag_arm_margin);

static inline unsigned long cpufreq_get_voltage(struct cpufreq_dvfs_info *dvfs,
									unsigned long frequency)
{
	unsigned long (*dvfs_table)[2] = (unsigned long(*)[2])dvfs->dvfs_table;
	int i = 0;

	for (i = 0; dvfs->table_size > i; i++) {
		if (frequency == dvfs_table[i][0])
			return dvfs_table[i][1];
	}

	printk("Fail : invalid frequency (%ld:%d) id !!!\n", frequency, dvfs->table_size);
	return -EINVAL;
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

static int nxp_cpufreq_set_freq_point(struct cpufreq_dvfs_info *dvfs,
					unsigned long frequency)
{
	unsigned long (*dvfs_tables)[2] = (unsigned long(*)[2])dvfs->dvfs_table;
	int len = dvfs->table_size;
	int id = 0;

	for (id = 0; len > id; id++)
	if (frequency == dvfs_tables[id][0])
		break;

	if (id == len) {
		printk("Fail : invalid frequency (%ld:%d) id !!!\n", frequency, len);
		return -EINVAL;
	}

	dvfs->freq_point = id;
	return 0;
}

static long nxp_cpufreq_change_voltage(struct cpufreq_dvfs_info *dvfs,
						unsigned long frequency, bool margin)
{
	long mS = 0, uS = 0;
	long uV = 0, wT = 0;

	if (!dvfs->volt)
		return 0;

	uV = cpufreq_get_voltage(dvfs, frequency);
	wT = dvfs->supply_delay_us;

	/* when rest duration */
	if (0 > uV) {
		printk("%s: failed invalid freq %ld uV %ld !!!\n", __func__, frequency, uV);
		return -EINVAL;
	}

	if (false == margin && dvfs->asv_ops->get_voltage) {
		uV = dvfs->asv_ops->get_voltage(frequency);
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
			frequency, uV/1000000, uV%1000000, mS, uS);
#endif
	return uV;
}

static unsigned long nxp_cpufreq_change_frequency(struct cpufreq_dvfs_info *dvfs,
				struct cpufreq_freqs *freqs, bool margin)
{
	struct clk *clk = dvfs->clk;
	unsigned long rate_khz = 0;

	nxp_cpufreq_set_freq_point(dvfs, freqs->new);

	if (!test_bit(FREQ_STATE_RESUME, &dvfs->resume_state))
		return freqs->old;

	/* pre voltage */
	if (freqs->new > freqs->old && !dvfs->fixed_uV)
		nxp_cpufreq_change_voltage(dvfs, freqs->new, margin);

#ifdef CONFIG_LOCAL_TIMERS
	for_each_cpu(freqs->cpu, dvfs->cpus)
#endif
		cpufreq_notify_transition(freqs, CPUFREQ_PRECHANGE);

	clk_set_rate(clk, freqs->new*1000);
	rate_khz = clk_get_rate(clk)/1000;

#ifdef CONFIG_ARM_NXP_CPUFREQ_DEBUG
	printk(" set rate %u:%lukhz\n", freqs->new, rate_khz);
#endif

	if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state)) {
		int id = dvfs->freq_point;
		int prev = dvfs->pre_freq_point;
		long ms = ktime_to_ms(ktime_get());
		dvfs->time_stamp[prev].duration += (ms - dvfs->time_stamp[prev].start);
		dvfs->time_stamp[id].start = ms;
		dvfs->pre_freq_point = id;
	}

#ifdef CONFIG_LOCAL_TIMERS
	for_each_cpu(freqs->cpu, dvfs->cpus)
#endif
		cpufreq_notify_transition(freqs, CPUFREQ_POSTCHANGE);

	/* post voltage */
	if (freqs->old > freqs->new && !dvfs->fixed_uV)
		nxp_cpufreq_change_voltage(dvfs, freqs->new, margin);

	return rate_khz;
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

		freqs.cpu = 0;
		freqs.new = dvfs->boot_frequency;
		if (freqs.new > max_freq) {
			freqs.new = max_freq;
			printk("DVFS: max freq %ldkhz less than boot %ldkz..\n",
				max_freq, dvfs->boot_frequency);
		}

		freqs.old = clk_get_rate(clk)/1000;

		dvfs->target_freq = freqs.new;
		nxp_cpufreq_change_frequency(dvfs, &freqs, false);

    	clear_bit(FREQ_STATE_RESUME, &dvfs->resume_state);
		mutex_unlock(&dvfs->lock);
    	break;

    case PM_POST_SUSPEND:	/* set restore frequecny */
		mutex_lock(&dvfs->lock);
    	set_bit(FREQ_STATE_RESUME, &dvfs->resume_state);

		freqs.cpu = 0;
		freqs.new = dvfs->target_freq;
		freqs.old = clk_get_rate(clk)/1000;
		nxp_cpufreq_change_frequency(dvfs, &freqs, true);

		mutex_unlock(&dvfs->lock);
    	break;
    }
    return 0;
}

static int nxp_cpufreq_process(void *data)
{
	struct cpufreq_dvfs_info *dvfs = data;
	struct cpufreq_freqs freqs;
	struct clk *clk = dvfs->clk;

	set_current_state(TASK_INTERRUPTIBLE);

	while (1) {

		if (kthread_should_stop())
			break;

		if (!dvfs->limits.new_freq) {
			schedule();
			continue;
		}

		mutex_lock(&dvfs->lock);
		set_current_state(TASK_UNINTERRUPTIBLE);

		freqs.new = dvfs->limits.new_freq;
		freqs.old = clk_get_rate(clk)/1000;;
		freqs.cpu = dvfs->cpu;

		nxp_cpufreq_change_frequency(dvfs, &freqs, true);

		dvfs->limits.new_freq = 0;

		set_current_state(TASK_INTERRUPTIBLE);
		mutex_unlock(&dvfs->lock);

		schedule();
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

	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = nxp_cpufreq_rest_timer;

	hrtimer = &dvfs->limits.restore_hrtimer;
	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = nxp_cpufreq_restore_timer;

	p = kthread_create_on_node(nxp_cpufreq_process,
					dvfs, cpu_to_node(cpu), "cpufreq-update");
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
	int id = dvfs->freq_point;
	ssize_t count = 0;
	int i = 0;

	if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state)) {
		long ms = ktime_to_ms(ktime_get());
		if (dvfs->time_stamp[id].start)
			dvfs->time_stamp[id].duration += (ms - dvfs->time_stamp[id].start);
		dvfs->time_stamp[id].start = ms;
		dvfs->pre_freq_point = id;
	}

	for (; dvfs->table_size > i; i++)
		count += sprintf(&buf[count], "%8ld ", dvfs->time_stamp[i].duration);

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t store_speed_duration(struct cpufreq_policy *policy,
			const char *buf, size_t count)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	int id = dvfs->freq_point;
	long ms = ktime_to_ms(ktime_get());
	const char *s = buf;

	mutex_lock(&dvfs->lock);

	if (0 == strncmp(s, "run", strlen("run"))) {
		dvfs->pre_freq_point = id;
		dvfs->time_stamp[id].start = ms;
		set_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state);
	}
	else if (0 == strncmp(s, "stop", strlen("stop"))) {
		clear_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state);
	}
	else if (0 == strncmp(s, "clear", strlen("clear"))) {
		memset(dvfs->time_stamp, 0, sizeof(dvfs_timestamp));
		if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->check_state)) {
			dvfs->time_stamp[id].start = ms;
			dvfs->pre_freq_point = id;
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
 	unsigned long (*dvfs_table)[2] = (unsigned long(*)[2])dvfs->dvfs_table;
 	ssize_t count = 0;
	int i = 0;

	for (; dvfs->table_size > i; i++) {
		long uV = dvfs_table[i][1];
		if (dvfs->asv_ops->get_voltage)
			uV = dvfs->asv_ops->get_voltage(dvfs_table[i][0]);
		count += sprintf(&buf[count], "%ld ", uV);
	}

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t show_cur_voltages(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
 	unsigned long (*dvfs_table)[2] = (unsigned long(*)[2])dvfs->dvfs_table;
 	ssize_t count = 0;
	int i = 0;

	for (; dvfs->table_size > i; i++)
		count += sprintf(&buf[count], "%ld ", dvfs_table[i][1]);

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t store_cur_voltages(struct cpufreq_policy *policy,
			const char *buf, size_t count)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	unsigned long (*dvfs_tables)[2] = (unsigned long(*)[2])dvfs_freq_voltage;
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

	nxp_cpufreq_change_voltage(dvfs, dvfs->target_freq, true);

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
	unsigned long rate_khz = 0;
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
	if (ktime_to_ms(dvfs->limits.rest_ktime) &&
		freqs.new > dvfs->limits.rest_freq) {
		ts = (ktime_to_ms(ktime_get()) - ktime_to_ms(dvfs->limits.rest_ktime));
		if (dvfs->limits.rest_retent > ts) {
			/* set with rest freq */
			freqs.new = dvfs->limits.rest_freq;
			pr_debug("rest %4ld:%4ldms (%u khz)\n",
			dvfs->limits.rest_retent, ts, freqs.new);
			goto freq_up;	/* retry */
		}
		dvfs->limits.rest_ktime = ktime_set(0, 0);	/* clear rest time */
	}

	if (dvfs->limits.max_freq && dvfs->limits.run_monitor &&
		freqs.new < dvfs->limits.max_freq ) {
		dvfs->limits.run_monitor = 0;
		hrtimer_cancel(&dvfs->limits.rest_hrtimer);
		pr_debug("stop monitor");
	}

	if (dvfs->limits.max_freq && !dvfs->limits.run_monitor &&
		freqs.new >= dvfs->limits.max_freq) {
		dvfs->limits.run_monitor = 1;
		hrtimer_start(&dvfs->limits.rest_hrtimer, ms_to_ktime(dvfs->limits.max_retent),
			      HRTIMER_MODE_REL_PINNED);
		pr_debug("run  monitor");
	}
	pr_debug("\n");

freq_up:
	dvfs->policy = policy;

	rate_khz = nxp_cpufreq_change_frequency(dvfs, &freqs, true);

	mutex_unlock(&dvfs->lock);

	return ret;
}

static int __cpuinit nxp_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_dvfs_info *dvfs = get_dvfs_ptr();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;
	int res;

	pr_debug("nxp-cpufreq: available cpus (%d)\n", num_online_cpus());

	res = cpufreq_frequency_table_cpuinfo(policy, freq_table);
	if (res) {
		pr_err("nxp-cpufreq: Failed to read policy table\n");
		return res;
	}

	policy->cur = nxp_cpufreq_get_speed(policy->cpu);
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;

	/* set the transition latency value */
	policy->cpuinfo.transition_latency = 100000;

	cpufreq_frequency_table_get_attr(freq_table, policy->cpu);

	/*
	 * multi-core processors has 2 cores
	 * that the frequency cannot be set independently.
	 * Each cpu is bound to the same speed.
	 * So the affected cpu is all of the cpus.
	 */
#ifdef CONFIG_LOCAL_TIMERS
	if (num_online_cpus() == 1) {
		cpumask_copy(policy->related_cpus, cpu_possible_mask);
		cpumask_copy(policy->cpus, cpu_online_mask);
		cpumask_copy(dvfs->cpus, cpu_online_mask);
	} else {
		cpumask_setall(policy->cpus);
		cpumask_setall(dvfs->cpus);
	}
#else
	cpumask_copy(policy->related_cpus, cpu_possible_mask);
	cpumask_copy(policy->cpus, cpu_online_mask);
	cpumask_set_cpu(0, dvfs->cpus);
#endif

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

#ifdef CONFIG_OF
static unsigned long dt_dvfs_table[FREQ_TABLE_MAX][2];

struct nxp_cpufreq_plat_data dt_cpufreq_data = {
	.pll_dev = CONFIG_NXP_CPUFREQ_PLLDEV,
	.dvfs_table = dt_dvfs_table,
};

static const struct of_device_id dvfs_dt_match[] = {
	{
	.compatible = "nexell,nxp-cpufreq",
	.data = (void*)&dt_cpufreq_data,
	}, {},
};
MODULE_DEVICE_TABLE(of, dvfs_dt_match);

#define	FN_SIZE		4
static void *nxp_cpufreq_get_dt_data(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *match;
	struct nxp_cpufreq_plat_data *pdata;
	unsigned long (*plat_tbs)[2] = NULL;
	const __be32 *list;
	char *supply;
	int value, i, size = 0;

	match = of_match_node(dvfs_dt_match, node);
	if (!match)
		return NULL;

	pdata = (struct nxp_cpufreq_plat_data *)match->data;
	plat_tbs = (unsigned long(*)[2])pdata->dvfs_table;

	if (!of_property_read_string(node, "supply_name", (const char**)&supply)) {
		pdata->supply_name = supply;
		if (!of_property_read_u32(node, "supply_delay_us", &value))
			pdata->supply_delay_us = value;
		printk("voltage supply : %s\n", pdata->supply_name);
	}

	list = of_get_property(node, "dvfs-tables", &size);
	size /= FN_SIZE;

	if (size) {
		for (i = 0; size/2 > i; i++) {
			plat_tbs[i][0] = be32_to_cpu(*list++);
			plat_tbs[i][1] = be32_to_cpu(*list++);
			pr_debug("DTS %2d = %8ldkhz, %8ld uV \n", i, plat_tbs[i][0], plat_tbs[i][1]);
		}
		pdata->table_size = size/2;
	}

	if (!of_property_read_u32(node, "max_cpufreq", &value))
		pdata->max_cpufreq = value;

	if (!of_property_read_u32(node, "max_retention", &value))
		pdata->max_retention = value;

	if (!of_property_read_u32(node, "rest_cpufreq", &value))
		pdata->rest_cpufreq = value;

	if (!of_property_read_u32(node, "rest_retention", &value))
		pdata->rest_retention = value;

	return pdata;
}
#else
#define dvfs_dt_match NULL
#endif

static void *nxp_cpufreq_make_table(struct platform_device *pdev,
							int *table_size, unsigned long (*dvfs_tables)[2])
{
	struct nxp_cpufreq_plat_data *pdata = pdev->dev.platform_data;
	struct cpufreq_frequency_table *freq_table;
	struct cpufreq_asv_ops *ops = &asv_ops;
	unsigned long (*plat_tbs)[2] = NULL;
	unsigned long plat_n_voltage = 0;
 	int tb_size, asv_size = 0;
	int id = 0, n = 0;

	/* user defined dvfs */
	if (pdata->freq_table && pdata->table_size)
		plat_tbs = (unsigned long(*)[2])pdata->freq_table;

	/* asv dvfs tables */
	if (ops->setup_table)
		asv_size = ops->setup_table(dvfs_tables);

	if (!pdata->table_size && !asv_size) {
		dev_err(&pdev->dev, "failed no freq table !!!\n");
		return NULL;
	}

	tb_size = (pdata->table_size ? pdata->table_size : asv_size);

	/* alloc with end table */
	freq_table = kzalloc((sizeof(*freq_table) * (tb_size + 1)), GFP_KERNEL);
	if (!freq_table) {
		dev_err(&pdev->dev, "failed allocate freq table !!!\n");
		return NULL;
	}

	/* make frequency table with platform data */
	if (asv_size > 0) {
		for (n = 0, id = 0; tb_size > id && asv_size > n; n++) {
			if (plat_tbs && plat_tbs[id][1] > 0)
				plat_n_voltage = plat_tbs[id][1];

			if (plat_n_voltage) {
				dvfs_tables[id][0] = plat_tbs[id][0];	/* frequency */
				dvfs_tables[id][1] = plat_n_voltage;	/* voltage */

			} else if (plat_tbs) {
				for (n = 0; asv_size > n; n++) {
					if (plat_tbs[id][0] == dvfs_tables[n][0]) {
						dvfs_tables[id][0] = dvfs_tables[n][0];	/* frequency */
						dvfs_tables[id][1] = dvfs_tables[n][1];	/* voltage */
						break;
					}
				}
			} else {
				if (dvfs_tables[n][0] > FREQ_MAX_FREQ_KHZ)
					continue;
				dvfs_tables[id][0] = dvfs_tables[n][0];	/* frequency */
				dvfs_tables[id][1] = dvfs_tables[n][1];	/* voltage */
			}
			freq_table[id].index = id;
			freq_table[id].frequency = dvfs_tables[id][0];
			printk(KERN_DEBUG "ASV %2d = %8ld khz, %4ld mV\n", id,
					dvfs_tables[id][0], dvfs_tables[id][1]/1000);
			/* next */
			id++;
		}
	} else {
		for (id = 0; tb_size > id; id++) {
			dvfs_tables[id][0] = plat_tbs[id][0];	/* frequency */
			dvfs_tables[id][1] = plat_tbs[id][1];	/* voltage */
			freq_table[id].index = id;
			freq_table[id].frequency = dvfs_tables[id][0];
			printk("DTB %2d = %8ldkhz, %8ld uV \n", id, dvfs_tables[id][0], dvfs_tables[id][1]);
		}
	}

	/* End table */
	freq_table[id].index = id;
	freq_table[id].frequency = CPUFREQ_TABLE_END;

	*table_size = id;

	return (void*)freq_table;
}

static int nxp_cpufreq_set_supply(struct platform_device *pdev,
							struct cpufreq_dvfs_info *dvfs)
{
	struct nxp_cpufreq_plat_data *pdata = pdev->dev.platform_data;
	static struct notifier_block *pm_notifier;
	struct tag_asv_margin *margin = &asv_margin;

	/* get voltage regulator */
	dvfs->volt = regulator_get(&pdev->dev, pdata->supply_name);
	if (IS_ERR(dvfs->volt)) {
		dev_err(&pdev->dev, "Cannot get regulator for DVS supply %s\n",
				pdata->supply_name);
		return -1;
	}

	pm_notifier = &dvfs->pm_notifier;
	pm_notifier->notifier_call = nxp_cpufreq_pm_notify;
	if (register_pm_notifier(pm_notifier)) {
		dev_err(&pdev->dev, "Cannot pm notifier %s\n", pdata->supply_name);
		return -1;
	}

	/* set margin voltage */
	if (margin->value && dvfs->asv_ops->modify_vol_table)
		dvfs->asv_ops->modify_vol_table(dvfs->dvfs_table, dvfs->table_size,
							margin->value, margin->minus, margin->percent);

	/* change to margin voltage */
	if (margin->value) {
		nxp_cpufreq_change_voltage(dvfs, dvfs->boot_frequency, true);
		printk("DVFS: adjust %ld margin %s%d%s \n",
				dvfs->boot_frequency, margin->minus?"-":"+", margin->value,
				margin->percent?"%":"mV");
	}

	if (pdata->fixed_voltage > 0) {
		dvfs->fixed_uV = pdata->fixed_voltage;
		regulator_set_voltage(dvfs->volt, dvfs->fixed_uV, dvfs->fixed_uV);
	}

	printk("DVFS: regulator %s\n", pdata->supply_name);
	return 0;
}

static int nxp_cpufreq_probe(struct platform_device *pdev)
{
	struct nxp_cpufreq_plat_data *pdata = pdev->dev.platform_data;
	unsigned long (*dvfs_tables)[2] = (unsigned long(*)[2])dvfs_freq_voltage;
	struct cpufreq_dvfs_info *dvfs = NULL;
	struct cpufreq_frequency_table *freq_table = NULL;
	struct cpufreq_freqs freqs;
	int cpu = raw_smp_processor_id();
	char name[16];
	int table_size = 0, ret = 0;

	dvfs = kzalloc(sizeof(*dvfs), GFP_KERNEL);
	if (!dvfs) {
		dev_err(&pdev->dev, "failed allocate DVFS data !!!\n");
		return -ENOMEM;
	}

#ifdef CONFIG_OF
	if (pdev->dev.of_node) {
		pdata = nxp_cpufreq_get_dt_data(pdev);
		if (!pdata)
			goto err_free_table;
		pdev->dev.platform_data = pdata;
	}
#endif

	freq_table = nxp_cpufreq_make_table(pdev, &table_size, dvfs_tables);
	if (!freq_table)
		goto err_free_table;

	sprintf(name, "pll%d", pdata->pll_dev);
	dvfs->clk = clk_get(NULL, name);
	if (IS_ERR(dvfs->clk))
		goto err_free_table;

	set_dvfs_ptr(dvfs);
	mutex_init(&dvfs->lock);

	dvfs->asv_ops = &asv_ops;
	dvfs->freq_table = freq_table;
	dvfs->dvfs_table = (unsigned long(*)[2])(dvfs_tables);
	dvfs->table_size = table_size;
	dvfs->supply_delay_us = pdata->supply_delay_us;
	dvfs->boot_frequency = nxp_cpufreq_get_speed(cpu);
	dvfs->target_freq = dvfs->boot_frequency;
	dvfs->pre_freq_point = -1;
	dvfs->check_state = 0;
	dvfs->time_stamp = dvfs_timestamp;
	dvfs->limits.max_freq = pdata->max_cpufreq;
	dvfs->limits.max_retent = pdata->max_retention;
	dvfs->limits.rest_freq = pdata->rest_cpufreq;
	dvfs->limits.rest_retent = pdata->rest_retention;
	dvfs->limits.rest_ktime = ktime_set(0, 0);
	dvfs->limits.run_monitor = 0;

	set_bit(FREQ_STATE_RESUME, &dvfs->resume_state);
	nxp_cpufreq_set_freq_point(dvfs, dvfs->target_freq);

	if (pdata->supply_name) {
		ret = nxp_cpufreq_set_supply(pdev, dvfs);
		if (0 > ret)
			goto err_free_table;
	}

	if (0 > nxp_cpufreq_setup(dvfs))
		goto err_free_table;

	printk("DVFS: cpu %s with PLL.%d [tables=%d]\n",
		dvfs->volt?"DVFS":"DFS", pdata->pll_dev, dvfs->table_size);

	ret = cpufreq_register_driver(&nxp_cpufreq_driver);

	/* change boot frequency & voltage */
	if (!ret && dvfs->volt) {
		freqs.cpu = 0;
		freqs.new = dvfs->boot_frequency;
		freqs.old = clk_get_rate(dvfs->clk)/1000;
		nxp_cpufreq_change_frequency(dvfs, &freqs, false);
		dvfs->boot_voltage = regulator_get_voltage(dvfs->volt);
	}

	return ret;

err_free_table:
	if (dvfs)
		kfree(dvfs);

	if (freq_table)
		kfree(freq_table);

	return ret;
}

static struct platform_driver cpufreq_driver = {
	.probe	= nxp_cpufreq_probe,
	.driver	= {
		.name	= DEV_NAME_CPUFREQ,
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(dvfs_dt_match),
	},
};
module_platform_driver(cpufreq_driver);

static int __init early_asv_margin(char *str)
{
	struct tag_asv_margin *p = &asv_margin;
	const char *s = strchr(str, '-');

	if (s)
		p->minus = true;
	else
		s = strchr(str, '+');

	if (!s)
		s = str;
	else
		s++;

	if (strchr(str, '%'))
		p->percent = true;

	p->value = simple_strtol(s, NULL, 10);

	printk("ASV: Arm margin:%s%d%s\n", p->minus?"-":"+", p->value, p->percent?"%":"mV");
	return 1;
}
__setup("asv_margin", early_asv_margin);
