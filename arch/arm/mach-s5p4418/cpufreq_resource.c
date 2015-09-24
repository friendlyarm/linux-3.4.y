/*
 * (C) Copyright 2009
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
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/oom.h>
#include <linux/platform_device.h>
#include <mach/platform.h>
#include <mach/devices.h>

/*
#define pr_debug	printk
*/

// Limited Temperature Throthold for Drone.
#define TEMPERTURE_LIMIT_NONE			45
#define TEMPERTURE_LIMIT_LEVEL0			45
#define TEMPERTURE_LIMIT_LEVEL1			52 //55 //52
#define TEMPERTURE_LIMIT_LEVEL2			62
#define TEMPERTURE_LIMIT_LEVEL2_DOWN	TEMPERTURE_LIMIT_LEVEL2-5

#define TEMPERTURE_CHARGING_STOP		57
#define TEMPERTURE_CHARGING_CONTINUE	56

#define TEMPERTURE_CPU_DOWN1			57
#define TEMPERTURE_CPU_DOWN2			67

#define TEMPERTURE_BOOST_MAX			64
#define TEMPERTURE_BOOST_LIMIT			68

// CPU Usage Throthold for Drone.
#define	CPU_FULL_USED					96
#define	CPU_OVER_USAGELEVEL				79
#define	CPU_UNDER_USAGELEVEL			48
#define	CPU_MIN_USAGELEVEL				30

struct cpufreq_limit_data {
	char **limit_name;
	int limit_num;
	long aval_max_freq; 	/* unit Khz */
	long op_max_freq;		/* unit Khz */
	long limit_level0_freq; 	/* unit Khz */
	long limit_level1_freq; 	/* unit Khz */
	long min_max_freq;			/* unit Khz */
	long prev_max_freq;			/* unit Khz */
	long timer_duration;	/* unit ms */
	long op_timeout;		/* unit ms */
	int timer_chkcpu_mod;
	long time_stamp;
	long frequency; 		/* unit Khz */
	long pre_max_freq;		/* unit Khz */
	struct hrtimer limit_timer;
	struct work_struct limit_work;
	struct notifier_block limit_nb;

};


extern int curMaxCpu;
int isCpuMaxFrequency(void)
{
	return curMaxCpu;
}
EXPORT_SYMBOL_GPL(isCpuMaxFrequency);


#define SIZE_RD_STAT 128
extern int NXP_Get_BoardTemperature(void);
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE) && defined(CONFIG_BATTERY_NXE2000)
extern int isOccured_dieError(void);
#endif

typedef struct {
    unsigned long user;
    unsigned long system;
    unsigned long nice;
    unsigned long idle;
    unsigned long wait;
    unsigned long hi;
    unsigned long si;
    unsigned long zero;
} procstat_t;

typedef struct nxp_cpuLimit_control {
	int cpuUsageNx4418;
	procstat_t m_prev;
	procstat_t m_curr;
	bool occuredError; // occur unexpected, when occured, no use Maxclock --> Max:1.0G
	long temperature_limit_level2; // TEMPERTURE_LIMIT_LEVEL2;
	bool waring_level;
	int downFreqCount;
	int stopped_cpu;
	int board_temperature;
	int charging_stop;
	int downUsageCount;
}NXP_LIMITCPU_CTRL;
NXP_LIMITCPU_CTRL ctrl_cpuTemp = {0, };


/* d = a - b */
void diff_proc_stat(procstat_t *a, procstat_t *b, procstat_t *d)
{
    d->user     = a->user   - b->user;
    d->system   = a->system - b->system;
    d->nice     = a->nice   - b->nice;
    d->idle     = a->idle   - b->idle;
    d->wait     = a->wait   - b->wait;
    d->hi       = a->hi     - b->hi;
    d->si       = a->si     - b->si;
    d->zero     = a->zero   - b->zero;
}

int cpuUsage_getToken(char *buf, int *offset_usage)
{
	int cnt = 0;

	while(1)
	{
		if(buf[cnt]==' ')
			break;
		cnt++;
	}
	*offset_usage = cnt+1;

	if(buf[cnt+1] == ' ')
	{
		cnt++;
		while(1)
		{
			cnt++;
			if(buf[cnt]!=' ')
			{
				*offset_usage = cnt;
				break;
			}
		}
	}

	return *offset_usage;
}


void cpuUsage_getValue(char *buf, int *offsetUsage)
{
	int cnt = 0;
	unsigned long long value[11];
	int value_cnt =0;
	int bFinish=0;

	value_cnt = 0;
_reParsing:

	value[value_cnt] = 0;

	// get the value
	while(1)
	{
		if(buf[cnt]==' ')
			break;
		if(buf[cnt]=='\n' || buf[cnt]=='\r' || buf[cnt]=='\0')
		{
			bFinish = 1;
			break;
		}

		value[value_cnt] = 10*value[value_cnt] + (buf[cnt]-'0');
		cnt++;
	}
	if(bFinish)
	{
	    ctrl_cpuTemp.m_curr.user = value[0];
	    ctrl_cpuTemp.m_curr.system = value[1];
	    ctrl_cpuTemp.m_curr.nice = value[2];
	    ctrl_cpuTemp.m_curr.idle = value[3];
	    ctrl_cpuTemp.m_curr.wait = value[4];
	    ctrl_cpuTemp.m_curr.hi = value[5];
	    ctrl_cpuTemp.m_curr.si = value[6];
	    ctrl_cpuTemp.m_curr.zero = value[7];
		return;
	}

	cnt++;
	while(1)
	{
		if(buf[cnt]=='\n' || buf[cnt]=='\r' || buf[cnt]=='\0')
		{
			bFinish = 1;
			break;
		}

		if(buf[cnt]!=' ')
			break;
		cnt++;
	}

	*offsetUsage += cnt;

	if(bFinish)
	{
	    ctrl_cpuTemp.m_curr.user = value[0];
	    ctrl_cpuTemp.m_curr.system = value[1];
	    ctrl_cpuTemp.m_curr.nice = value[2];
	    ctrl_cpuTemp.m_curr.idle = value[3];
	    ctrl_cpuTemp.m_curr.wait = value[4];
	    ctrl_cpuTemp.m_curr.hi = value[5];
	    ctrl_cpuTemp.m_curr.si = value[6];
	    ctrl_cpuTemp.m_curr.zero = value[7];
		return;
	}

	value_cnt++;
goto _reParsing;

	return ;
}

static int _read_statfile(char *path, char *buf, int size)
{
	int fd;
	mm_segment_t old_fs;

	fd = sys_open(path, O_RDONLY, 0);
	old_fs = get_fs();
	if (0 > fd)
		return 1;

	set_fs(KERNEL_DS);
	sys_read(fd, (void*)buf, size);

	set_fs(old_fs);
	sys_close(fd);

	return 0;
}

/*
// CPU Core Down
*/
static int cpu_down_force_byResource(void)
{
    int cpu, cur = raw_smp_processor_id();
    int stopped = 0;
	int err;

    for_each_present_cpu(cpu) {
        if (cpu == 0 || cpu == 3)
            continue;
        if (cpu == cur)
            continue;
        if (!cpu_online(cpu))
            continue;
        stopped |= 1<<cpu;
        err = cpu_down(cpu);

		if(err == 0)
		return stopped;

    }

    return 0;
}

static void cpu_up_force_byResource(int stopped)
{
    int cpu;
    for_each_present_cpu(cpu) {
        if (stopped & 1<<cpu)
            cpu_up(cpu);

    }

}
///////////////////////////////////////

void GetCPUInfo(char *buf)
{
    procstat_t d;
    procstat_t u100; /* percentages(%x100) per total */
    long total, usage100;
	int offset_usage;
	int cpuUsage;

	cpuUsage_getToken(buf, &offset_usage);
	cpuUsage_getValue(&buf[offset_usage], &offset_usage);

    diff_proc_stat(&ctrl_cpuTemp.m_curr, &ctrl_cpuTemp.m_prev, &d);
    total = d.user + d.system + d.nice + d.idle + d.wait + d.hi + d.si;
    u100.user   = d.user * 10000 / total;
    u100.system = d.system * 10000 / total;
    u100.nice   = d.nice * 10000 / total;
    u100.idle   = (d.idle * 10000) / total;
    u100.wait   = d.wait * 10000 / total;
    u100.hi     = d.hi * 10000 / total;
    u100.si     = d.si * 10000 / total;
    usage100 = 10000 - u100.idle;

	cpuUsage = usage100/100;
	if(cpuUsage<0 || cpuUsage>100) return;

 	ctrl_cpuTemp.cpuUsageNx4418 = usage100;

    pr_debug("cpu:%02d.%02d %% MaxCpu(%d) temperature(%d) die(%d)stop(0x%x)\n",
    	(usage100/100), (usage100%100), curMaxCpu, ctrl_cpuTemp.board_temperature,
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE) && defined(CONFIG_BATTERY_NXE2000)
    	isOccured_dieError(),
#else
		0,
#endif
    	ctrl_cpuTemp.stopped_cpu);

    ctrl_cpuTemp.m_prev.user = ctrl_cpuTemp.m_curr.user ;
    ctrl_cpuTemp.m_prev.system =  ctrl_cpuTemp.m_curr.system ;
    ctrl_cpuTemp.m_prev.nice = ctrl_cpuTemp.m_curr.nice;
    ctrl_cpuTemp.m_prev.idle = ctrl_cpuTemp.m_curr.idle;
    ctrl_cpuTemp.m_prev.wait = ctrl_cpuTemp.m_curr.wait;
    ctrl_cpuTemp.m_prev.hi = ctrl_cpuTemp.m_curr.hi;
    ctrl_cpuTemp.m_prev.si = ctrl_cpuTemp.m_curr.si;
    ctrl_cpuTemp.m_prev.zero = ctrl_cpuTemp.m_curr.zero;

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE) && defined(CONFIG_BATTERY_NXE2000)
	if(isOccured_dieError() && (ctrl_cpuTemp.board_temperature<TEMPERTURE_LIMIT_LEVEL2))
		ctrl_cpuTemp.occuredError = 1;
#endif

}

void _GetCupInfomation(void)
{
	char buffer[SIZE_RD_STAT];

	if(_read_statfile("/proc/stat", buffer, SIZE_RD_STAT) == 0)
		GetCPUInfo(buffer);
}


int isCheck_ChargeStop_byResource(void)
{
	int cpuUsage = ctrl_cpuTemp.cpuUsageNx4418/100;

	if((cpuUsage < CPU_OVER_USAGELEVEL) && (ctrl_cpuTemp.board_temperature<TEMPERTURE_CPU_DOWN2))
	{
		ctrl_cpuTemp.charging_stop = 0;
		return 0;
	}

	if(ctrl_cpuTemp.board_temperature > TEMPERTURE_CHARGING_STOP)
	{
		ctrl_cpuTemp.charging_stop = 1;
		return 1;
	}
	else
	{
		if(ctrl_cpuTemp.charging_stop == 0)
			return 0;
		else if(ctrl_cpuTemp.board_temperature < TEMPERTURE_CHARGING_CONTINUE)
		{
			ctrl_cpuTemp.charging_stop = 0;
			return 0;
		}
		else
			return 1;
	}
}
EXPORT_SYMBOL_GPL(isCheck_ChargeStop_byResource);

void isCheck_CPUDown_byResource(void)
{
	int cpuUsage = ctrl_cpuTemp.cpuUsageNx4418/100;

	if(cpuUsage<CPU_OVER_USAGELEVEL)
		ctrl_cpuTemp.downUsageCount++;
	else
		ctrl_cpuTemp.downUsageCount=0;

	if(ctrl_cpuTemp.stopped_cpu && (ctrl_cpuTemp.downUsageCount>5) && (ctrl_cpuTemp.board_temperature < TEMPERTURE_CPU_DOWN2))
	{
		cpu_up_force_byResource(ctrl_cpuTemp.stopped_cpu);
		ctrl_cpuTemp.stopped_cpu=0;

		return;
	}

	if(ctrl_cpuTemp.board_temperature < TEMPERTURE_LIMIT_LEVEL1)
	{
		if(ctrl_cpuTemp.stopped_cpu)
		{
			cpu_up_force_byResource(ctrl_cpuTemp.stopped_cpu);
			ctrl_cpuTemp.stopped_cpu=0;
		}
		return;
	}

	if(ctrl_cpuTemp.board_temperature >= TEMPERTURE_CPU_DOWN2)
	{
		ctrl_cpuTemp.stopped_cpu |= cpu_down_force_byResource();
		return;
	}

	if(ctrl_cpuTemp.board_temperature >= TEMPERTURE_CPU_DOWN1)
	{
		if((ctrl_cpuTemp.stopped_cpu == 0) && (cpuUsage>CPU_OVER_USAGELEVEL))
			ctrl_cpuTemp.stopped_cpu |= cpu_down_force_byResource();
	}

}

long funcGetMaxFreq(struct cpufreq_limit_data *limit)
{
	long max_freq = 0;
	int cpuUsage = ctrl_cpuTemp.cpuUsageNx4418/100;

	if(ctrl_cpuTemp.temperature_limit_level2 == 0)
		ctrl_cpuTemp.temperature_limit_level2 = TEMPERTURE_LIMIT_LEVEL2;

	if(ctrl_cpuTemp.board_temperature < TEMPERTURE_LIMIT_LEVEL0) // 1.4G , normally booting and first used time.
	{
		if(cpuUsage>CPU_OVER_USAGELEVEL)
		{
				max_freq = limit->aval_max_freq;
				}
		else if(cpuUsage>CPU_UNDER_USAGELEVEL)
		{
			max_freq = limit->limit_level0_freq;
		}
		else
		{
			max_freq = limit->min_max_freq;
		}
	}
	else if(ctrl_cpuTemp.board_temperature < TEMPERTURE_LIMIT_LEVEL1) // 1.2G
	{
		if(cpuUsage>CPU_OVER_USAGELEVEL)
		{
				max_freq = limit->limit_level0_freq;
		}
		else if(cpuUsage>CPU_UNDER_USAGELEVEL)
		{
				max_freq = limit->limit_level1_freq;
		}
		else
		{
			max_freq = limit->min_max_freq;
		}
		/*
		if(ctrl_cpuTemp.stopped_cpu)
		{
			cpu_up_force_byResource(ctrl_cpuTemp.stopped_cpu);
			ctrl_cpuTemp.stopped_cpu=0;
		}
		*/
	}
	else if(ctrl_cpuTemp.board_temperature < ctrl_cpuTemp.temperature_limit_level2)//TEMPERTURE_LIMIT_LEVEL2) // 1.0G
	{
		if(cpuUsage>CPU_OVER_USAGELEVEL)
		{
			max_freq = limit->limit_level1_freq;
		}
		else
		{
			max_freq = limit->min_max_freq;
	}
	}
	else // Over Threthold -> set the min max_freq.
	{
		max_freq = limit->min_max_freq;
		ctrl_cpuTemp.temperature_limit_level2 = TEMPERTURE_LIMIT_LEVEL2_DOWN;
		ctrl_cpuTemp.waring_level = 1;
	}

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE) && defined(CONFIG_BATTERY_NXE2000)
	if(isOccured_dieError() || ctrl_cpuTemp.occuredError) // DieError
	{
		limit->prev_max_freq = limit->min_max_freq;
		ctrl_cpuTemp.downFreqCount = 0;

		return limit->min_max_freq;
	}
#endif

	/*
	// check  whether need to down cpu or not.
	if(ctrl_cpuTemp.board_temperature >= TEMPERTURE_CPU_DOWN1)
	{
		if(ctrl_cpuTemp.stopped_cpu == 0)
			ctrl_cpuTemp.stopped_cpu |= cpu_down_force_byResource();
		else if(ctrl_cpuTemp.board_temperature >= TEMPERTURE_CPU_DOWN2)
			ctrl_cpuTemp.stopped_cpu |= cpu_down_force_byResource();
	}
	*/
	isCheck_CPUDown_byResource();

	if(ctrl_cpuTemp.waring_level)
	{
		if(limit->prev_max_freq < max_freq)
		{
			if(ctrl_cpuTemp.downFreqCount++ < 60)
				return limit->prev_max_freq;
			else
			{
				ctrl_cpuTemp.waring_level = 0;
				ctrl_cpuTemp.temperature_limit_level2 = TEMPERTURE_LIMIT_LEVEL2;
			}
		}
		ctrl_cpuTemp.downFreqCount = 0;
		limit->prev_max_freq = max_freq;
	}

	return max_freq;

}


long funcGetMaxFreq_OnBoost(struct cpufreq_limit_data *limit)//, int *t)
{
	long max_freq = 0;

	if(ctrl_cpuTemp.stopped_cpu)
	{
		cpu_up_force_byResource(ctrl_cpuTemp.stopped_cpu);
		ctrl_cpuTemp.stopped_cpu=0;
	}
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE) && defined(CONFIG_BATTERY_NXE2000)
	if(isOccured_dieError() || ctrl_cpuTemp.occuredError) // over temperature.
		return limit->min_max_freq;
#endif
	if(ctrl_cpuTemp.board_temperature < TEMPERTURE_BOOST_MAX)
	{
		max_freq = limit->aval_max_freq;
	}
	else if(ctrl_cpuTemp.board_temperature < TEMPERTURE_BOOST_LIMIT)
	{
		max_freq = limit->limit_level0_freq;
	}
	else
	{
		max_freq = limit->limit_level1_freq;
	}

	return max_freq;
}


long cpuUsage_Process(struct cpufreq_limit_data *limit, int boost)
{
	ctrl_cpuTemp.board_temperature = NXP_Get_BoardTemperature();

	if(boost) // if over temperature, can't use max_clock.
		return funcGetMaxFreq_OnBoost(limit);

	_GetCupInfomation();

	return funcGetMaxFreq(limit);
}

