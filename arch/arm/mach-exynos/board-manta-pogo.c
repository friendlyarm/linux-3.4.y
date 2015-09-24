/* arch/arm/mach-exynos/board-manta-pogo.c
 *
 * Copyright (C) 2012 Samsung Electronics.
 * Copyright (C) 2012 Google, Inc.
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
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/cpufreq.h>

#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/switch.h>
#include <linux/wakelock.h>
#include <linux/sched.h>

#include <plat/gpio-cfg.h>
#include "board-manta.h"

/* including start and stop bits */
#define CMD_READ_SZ		8
#define CMD_WRITE_SZ		16
#define RESP_READ_SZ		12
#define RESP_WRITE_SZ		5

#define RW_OFFSET		2
#define ID_OFFSET		3
#define STOP_R_OFFSET		6
#define DATA_OFFSET		6
#define STOP_W_OFFSET		14

#define START_BITS		0x3
#define STOP_BITS		0x1

/* states for dock */
#define POGO_UNDOCKED			0
#define POGO_DESK_DOCK			1

/* states for usb_audio */
#define POGO_NO_AUDIO			0
#define POGO_DIGITAL_AUDIO		2

#define DOCK_STAT_POWER_MASK		3
#define DOCK_STAT_POWER_OFFSET		2
#define DOCK_STAT_POWER_NONE		0
#define DOCK_STAT_POWER_500MA		1
#define DOCK_STAT_POWER_2A		2
#define DOCK_STAT_POWER_1A		3

#define DOCK_STAT_AUDIO_MASK		3
#define DOCK_STAT_AUDIO_OFFSET		0
#define DOCK_STAT_AUDIO_CONNECTED	1
#define DOCK_STAT_AUDIO_DISCONNECTED	0

#define MFM_DELAY_NS_DEFAULT		(1000 * 10)
#define RESPONSE_INTERVAL		5
#define CMD_DELAY_USEC			100		/* 100us */
#define TX_ERR_DELAY_USEC(delay_ns)	((delay_ns) / 1000 * 7 + CMD_DELAY_USEC)
#define check_stop_bits(x, n) ((((x) >> ((n) - 2)) & 1) == (STOP_BITS & 1))
#define raw_resp(x, n) ((x) & ((1 << ((n) - 2)) - 1));

#define DEBOUNCE_DELAY_MS		250

#define DOCK_CPU_FREQ_MIN		800000

/* GPIO configuration */
#define GPIO_POGO_DATA_BETA		EXYNOS5_GPB0(0)
#define GPIO_POGO_DATA_POST_BETA	EXYNOS5_GPX0(5)
#define GPIO_DOCK_PD_INT		EXYNOS5_GPX0(6)
#define GPIO_POGO_SPDIF			EXYNOS5_GPB1(0)

#define GPIO_POGO_SEL1			EXYNOS5_GPG1(0)
#define GPIO_TA_CHECK_SEL		EXYNOS5_GPD1(5)
#define GPIO_LDO20_EN			EXYNOS5_GPD1(4)

static struct switch_dev dock_switch = {
	.name = "dock",
};

static struct switch_dev usb_audio_switch = {
	.name = "usb_audio",
};

enum pogo_debounce_state {
	POGO_DEBOUNCE_UNDOCKED,		/* interrupt enabled,  timer stopped */
	POGO_DEBOUNCE_UNSTABLE,		/* interrupt disabled, timer running */
	POGO_DEBOUNCE_WAIT_STABLE,	/* interrupt enabled,  timer running */
	POGO_DEBOUNCE_DOCKED,		/* interrupt enabled,  timer stopped */
};

struct dock_state {
	struct mutex lock;
	u32 t;
	u32 last_edge_t[2];
	u32 last_edge_i[2];
	bool level;
	bool dock_connected_unknown;
	bool ignore_data_irq_once;

	u32 mfm_delay_ns;
	bool powered_dock_present;
	struct workqueue_struct *dock_wq;
	struct work_struct dock_work;
	struct wake_lock wake_lock;

	wait_queue_head_t wait;
	int dock_pd_irq;
	bool debounce_pending;
	int spdif_mux_state;
	enum pogo_debounce_state debounce_state;
	struct timer_list debounce_timer;
	struct work_struct debounce_work;
	struct wake_lock debounce_wake_lock;

	int ta_check_wait;
	bool ta_check_mode;

	unsigned int cpufreq_min;
};

static struct dock_state ds = {
	.lock		= __MUTEX_INITIALIZER(ds.lock),
	.mfm_delay_ns	= MFM_DELAY_NS_DEFAULT,
	.wait		= __WAIT_QUEUE_HEAD_INITIALIZER(ds.wait),
	.debounce_state	= POGO_DEBOUNCE_UNDOCKED,
};

static struct gpio manta_pogo_gpios[] = {
	{GPIO_POGO_DATA_POST_BETA, GPIOF_IN, "dock"},
	{GPIO_POGO_SEL1, GPIOF_OUT_INIT_HIGH, "pogo_sel1"},
	{GPIO_TA_CHECK_SEL, GPIOF_OUT_INIT_LOW, "pogo_spdif_sel"},
	{GPIO_DOCK_PD_INT, GPIOF_IN, "dock_pd_int"},
	{GPIO_LDO20_EN, GPIOF_OUT_INIT_HIGH, "ldo20_en"},
};

#define _GPIO_DOCK		(manta_pogo_gpios[0].gpio)

#define dock_out(n) gpio_direction_output(_GPIO_DOCK, n);
#define dock_out2(n) gpio_set_value(_GPIO_DOCK, n)
#define dock_in() gpio_direction_input(_GPIO_DOCK);
#define dock_read() gpio_get_value(_GPIO_DOCK)
#define dock_irq() s3c_gpio_cfgpin(_GPIO_DOCK, S3C_GPIO_SFN(0xF))

static u32 pogo_read_fast_timer(void)
{
	return sched_clock();
}

static void pogo_set_min_cpu_freq(unsigned int khz)
{
	struct dock_state *s = &ds;
	int i;

	s->cpufreq_min = khz;

	for_each_online_cpu(i)
		cpufreq_update_policy(i);
}

static u16 make_cmd(int id, bool write, int data)
{
	u16 cmd = (id & 0x7) << ID_OFFSET | write << RW_OFFSET | START_BITS;
	cmd |= STOP_BITS << (write ? STOP_W_OFFSET : STOP_R_OFFSET);
	if (write)
		cmd |= (data & 0xFF) << DATA_OFFSET;
	return cmd;
}

static int dock_get_edge(struct dock_state *s, u32 timeout, u32 tmin, u32 tmax)
{
	bool lin;
	bool in = s->level;
	u32 t;
	do {
		lin = in;
		in = dock_read();
		t = pogo_read_fast_timer();
		if (in != lin) {
			s->last_edge_t[in] = t;
			s->last_edge_i[in] = 0;
			s->level = in;
			if ((s32) (t - tmin) < 0 || (s32) (t - tmax) > 0)
				return -1;
			return 1;
		}
	} while ((s32) (t - timeout) < 0);
	return 0;
}

static bool dock_sync(struct dock_state *s, u32 timeout)
{
	u32 t;

	s->level = dock_read();
	t = pogo_read_fast_timer();

	if (!dock_get_edge(s, t + timeout, 0, 0))
		return false;
	s->last_edge_i[s->level] = 2;
	return !!dock_get_edge(s, s->last_edge_t[s->level] + s->mfm_delay_ns *
			4, 0, 0);
}

static int dock_get_next_bit(struct dock_state *s)
{
	u32 i = s->last_edge_i[!s->level] + ++s->last_edge_i[s->level];
	u32 target = s->last_edge_t[!s->level] + s->mfm_delay_ns * i;
	u32 timeout = target + s->mfm_delay_ns / 2;
	u32 tmin = target - s->mfm_delay_ns / 4;
	u32 tmax = target + s->mfm_delay_ns / 4;
	return dock_get_edge(s, timeout, tmin, tmax);
}

static u32 dock_get_bits(struct dock_state *s, int count, int *errp)
{
	u32 data = 0;
	u32 m = 1;
	int ret;
	int err = 0;
	while (count--) {
		ret = dock_get_next_bit(s);
		if (ret)
			data |= m;
		if (ret < 0)
			err++;
		m <<= 1;
	}
	if (errp)
		*errp = err;
	return data;
}

static int dock_send_bits(struct dock_state *s, u32 data, int count, int period)
{
	u32 t, t0, to;

	dock_out(s->level);
	t = to = 0;
	t0 = pogo_read_fast_timer();

	while (count--) {
		if (data & 1)
			dock_out2((s->level = !s->level));

		t = pogo_read_fast_timer() - t0;
		if (t - to > period / 2) {
			pr_debug("dock: to = %d, t = %d\n", to, t);
			return -EIO;
		}

		data >>= 1;

		to += period;
		do {
			t = pogo_read_fast_timer() - t0;
		} while (t < to);
		if (t - to > period / 4) {
			pr_debug("dock: to = %d, t = %d\n", to, t);
			return -EIO;
		}
	}
	return 0;
}

static u32 mfm_encode(u16 data, int count, bool p)
{
	u32 mask;
	u32 mfm = 0;
	u32 clock = ~data & ~(data << 1 | !!p);
	for (mask = 1UL << (count - 1); mask; mask >>= 1) {
		mfm |= (data & mask);
		mfm <<= 1;
		mfm |= (clock & mask);
	}
	return mfm;
}

static u32 mfm_decode(u32 mfm)
{
	u32 data = 0;
	u32 clock = 0;
	u32 mask = 1;
	while (mfm) {
		if (mfm & 1)
			clock |= mask;
		mfm >>= 1;
		if (mfm & 1)
			data |= mask;
		mfm >>= 1;
		mask <<= 1;
	}
	return data;
}

static int dock_command(struct dock_state *s, u16 cmd, int len, int retlen)
{
	u32 mfm;
	int count;
	u32 data = cmd;
	int tx, ret;
	int err = -1;
	unsigned long flags;

	if (!s->cpufreq_min)
		pogo_set_min_cpu_freq(DOCK_CPU_FREQ_MIN);

	mfm = mfm_encode(data, len, false);
	count = len * 2;

	pr_debug("%s: data 0x%x mfm 0x%x\n", __func__, cmd, mfm);

	local_irq_save(flags);
	tx = dock_send_bits(s, mfm, count, s->mfm_delay_ns);

	if (!tx) {
		dock_in();
		if (dock_sync(s, s->mfm_delay_ns * RESPONSE_INTERVAL)) {
			ret = dock_get_bits(s, retlen * 2, &err);
		} else {
			pr_debug("%s: response sync error\n", __func__);
			s->ignore_data_irq_once = s->level;
			ret = -1;
		}
		dock_irq();
	} else if (!s->level)
		dock_irq();

	local_irq_restore(flags);

	udelay(tx < 0 ? TX_ERR_DELAY_USEC(s->mfm_delay_ns) : CMD_DELAY_USEC);

	if (tx < 0) {
		pr_debug("dock_command: %x: transmission err %d\n", cmd, tx);
		return tx;
	}
	if (ret < 0) {
		pr_debug("dock_command: %x: no response\n", cmd);
		return ret;
	}
	data = mfm_decode(ret);
	mfm = mfm_encode(data, retlen, true);
	if (mfm != ret || err || !check_stop_bits(data, retlen)) {
		pr_debug("dock_command: %x: bad response, data %x, mfm %x %x, err %d\n",
						cmd, data, mfm, ret, err);
		return -EIO;
	}

	return raw_resp(data, retlen);
}

static int dock_command_retry(struct dock_state *s, u16 cmd, size_t len,
		size_t retlen)
{
	int retry = 20;
	int ret;
	while (retry--) {
		ret = dock_command(s, cmd, len, retlen);
		if (ret >= 0)
			return ret;
		if (retry != 19)
			usleep_range(10000, 11000);
	}
	s->dock_connected_unknown = true;
	if (s->level) {
		/* put the line low so we can receive dock interrupts */
		dock_out(s->level = false);
		udelay(1);
		dock_irq();
	}
	return -EIO;
}

static int dock_send_cmd(struct dock_state *s, int id, bool write, int data)
{
	int ret;
	u16 cmd = make_cmd(id, write, data);

	ret = dock_command_retry(s, cmd, write ? CMD_WRITE_SZ : CMD_READ_SZ,
			write ? RESP_WRITE_SZ - 2 : RESP_READ_SZ - 2);

	if (ret < 0)
		pr_warning("%s: failed, cmd: %x, write: %d, data: %x\n",
			__func__, cmd, write, data);

	return ret;
}

static int dock_read_multi(struct dock_state *s, int addr, u8 *data, size_t len)
{
	int ret;
	int i;
	u8 suml, sumr = -1;

	int retry = 20;
	while (retry--) {
		suml = 0;
		for (i = 0; i <= len; i++) {
			ret = dock_send_cmd(s, addr + i, false, 0);
			if (ret < 0)
				return ret;
			if (i < len) {
				data[i] = ret;
				suml += ret;
			} else
				sumr = ret;
		}
		if (sumr == suml)
			return 0;

		pr_warning("dock_read_multi(%x): bad checksum, %x != %x\n",
			   addr, sumr, suml);
	}
	return -EIO;
}

static int dock_write_multi(struct dock_state *s, int addr, u8 *data,
		size_t len)
{
	int ret;
	int i;
	u8 sum;

	int retry = 20;
	while (retry--) {
		sum = 0;
		for (i = 0; i < len; i++) {
			sum += data[i];
			ret = dock_send_cmd(s, addr + i, true, data[i]);
			if (ret < 0)
				return ret;
		}
		ret = dock_send_cmd(s, addr + len, true, sum);
		if (ret < 0)
			return ret;
		/* check sum error */
		if (ret == 0)
			continue;
		return 0;
	}
	return -EIO;
}

static void manta_pogo_spdif_config(bool charger_detect_mode)
{
	if (charger_detect_mode) {
		s3c_gpio_cfgpin(GPIO_POGO_SPDIF, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_POGO_SPDIF, S3C_GPIO_PULL_NONE);
	} else {
		s3c_gpio_cfgpin(GPIO_POGO_SPDIF, S3C_GPIO_SFN(0x4));
		s3c_gpio_setpull(GPIO_POGO_SPDIF, S3C_GPIO_PULL_NONE);
	}
}

static void pogo_dock_pd_interrupt_locked(int irq, void *data)
{
	struct dock_state *s = data;

	switch (s->debounce_state) {
	case POGO_DEBOUNCE_UNDOCKED:
	case POGO_DEBOUNCE_DOCKED:
		wake_lock(&s->debounce_wake_lock);
		mod_timer(&s->debounce_timer,
			jiffies + msecs_to_jiffies(DEBOUNCE_DELAY_MS));
		s->debounce_state = POGO_DEBOUNCE_WAIT_STABLE;
		break;
	case POGO_DEBOUNCE_WAIT_STABLE:
		/*
		 * Disable IRQ line in case there is noise. It will be
		 * re-enabled when the timer expires.
		 */
		s->debounce_state = POGO_DEBOUNCE_UNSTABLE;
		disable_irq_nosync(s->dock_pd_irq);
		break;
	case POGO_DEBOUNCE_UNSTABLE:
		break;
	}
}

static void dock_spdif_switch_set(bool spdif_mode)
{
	struct dock_state *s = &ds;

	pr_debug("%s: %d pending %d\n", __func__, spdif_mode,
		s->debounce_pending);

	s->spdif_mux_state = spdif_mode;
	gpio_set_value(GPIO_TA_CHECK_SEL, spdif_mode);

	if (!spdif_mode && s->debounce_pending) {
		pogo_dock_pd_interrupt_locked(s->dock_pd_irq, s);
		s->debounce_pending = false;
	}
}

static void dock_set_audio_switch(bool audio_on)
{
	struct dock_state *s = &ds;
	unsigned long flags;

	spin_lock_irqsave(&s->wait.lock, flags);
	dock_spdif_switch_set(audio_on);
	if (audio_on && s->debounce_state == POGO_DEBOUNCE_UNDOCKED)
		pogo_dock_pd_interrupt_locked(s->dock_pd_irq, s);
	spin_unlock_irqrestore(&s->wait.lock, flags);

	switch_set_state(&usb_audio_switch, audio_on ?
			POGO_DIGITAL_AUDIO : POGO_NO_AUDIO);
}

int manta_pogo_charge_detect_start(bool spdif_mode_and_gpio_in)
{
	struct dock_state *s = &ds;
	unsigned long flags;
	int ret = 0;

	pr_debug("%s: %d\n", __func__, spdif_mode_and_gpio_in);

	mutex_lock(&s->lock);

	spin_lock_irqsave(&s->wait.lock, flags);

	if (s->debounce_state == POGO_DEBOUNCE_UNSTABLE ||
		s->debounce_state == POGO_DEBOUNCE_WAIT_STABLE) {

		s->ta_check_wait = 10;
		ret = wait_event_interruptible_locked(s->wait,
			s->debounce_state == POGO_DEBOUNCE_DOCKED ||
			s->debounce_state == POGO_DEBOUNCE_UNDOCKED ||
			!s->ta_check_wait);

		if (!s->ta_check_wait || ret) {
			mutex_unlock(&s->lock);
			ret = -EBUSY;
			goto done;
		}
	}

	s->ta_check_mode = true;
	s->ta_check_wait = 0;

	if (spdif_mode_and_gpio_in)
		manta_pogo_spdif_config(true);
	dock_spdif_switch_set(spdif_mode_and_gpio_in);
	gpio_set_value(GPIO_POGO_SEL1, 0);
done:
	spin_unlock_irqrestore(&s->wait.lock, flags);

	return ret;
}

void manta_pogo_charge_detect_end(void)
{
	struct dock_state *s = &ds;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	BUG_ON(!s->ta_check_mode);

	spin_lock_irqsave(&s->wait.lock, flags);

	s->ta_check_mode = false;

	gpio_set_value(GPIO_POGO_SEL1, 1);
	dock_spdif_switch_set(false);
	manta_pogo_spdif_config(false);

	spin_unlock_irqrestore(&s->wait.lock, flags);

	mutex_unlock(&s->lock);
}

static int dock_acquire(struct dock_state *s, bool check_docked)
{
	int ret = 0;

	mutex_lock(&s->lock);

	if (check_docked && !s->powered_dock_present) {
		mutex_unlock(&s->lock);
		return -ENODEV;
	}

	pr_debug("%s: acquired dock\n", __func__);

	s->level = dock_read();
	if (s->level) {
		udelay(s->mfm_delay_ns / 1000 * 2);
		s->level = dock_read();
	}

	return ret;
}

static void dock_release(struct dock_state *s)
{
	if (s->cpufreq_min)
		pogo_set_min_cpu_freq(0);

	mutex_unlock(&s->lock);
	pr_debug("%s: released dock\n", __func__);
}

enum {
	DOCK_STATUS = 0x1,
	DOCK_ID_ADDR = 0x2,
	DOCK_VERSION = 0x7,
};

static int dock_check_status(struct dock_state *s,
			     enum manta_charge_source *charge_source)
{
	int ret = 0;
	int dock_stat, power;

	if (s->dock_connected_unknown) {
		/* force a new dock notification if a command failed */
		dock_set_audio_switch(false);
		s->dock_connected_unknown = false;
	}

	pr_debug("%s: sending status command\n", __func__);

	dock_stat = dock_send_cmd(s, DOCK_STATUS, false, 0);

	pr_debug("%s: Dock status %02x\n", __func__, dock_stat);
	if (dock_stat >= 0) {
		dock_set_audio_switch(dock_stat & DOCK_STAT_AUDIO_CONNECTED);

		if (charge_source) {
			s->powered_dock_present = true;
			power = (dock_stat >> DOCK_STAT_POWER_OFFSET) &
					DOCK_STAT_POWER_MASK;
			switch (power) {
			case DOCK_STAT_POWER_500MA:
				*charge_source = MANTA_CHARGE_SOURCE_USB;
				break;
			case DOCK_STAT_POWER_1A:
				*charge_source = MANTA_CHARGE_SOURCE_AC_OTHER;
				break;
			case DOCK_STAT_POWER_2A:
				*charge_source = MANTA_CHARGE_SOURCE_AC_SAMSUNG;
				break;
			default:
				pr_warn("%s: unknown dock power state %d, default to USB\n",
							__func__, power);
				*charge_source = MANTA_CHARGE_SOURCE_USB;
			}
		}

		goto done;
	}

	dock_in();
	ret = -ENOENT;
	dock_set_audio_switch(false);
done:
	return ret;
}

int manta_pogo_set_vbus(bool status, enum manta_charge_source *charge_source)
{
	struct dock_state *s = &ds;
	int ret = 0;

	dock_acquire(s, false);

	pr_debug("%s: status %d\n", __func__, status ? 1 : 0);

	if (status) {
		ret = dock_check_status(s, charge_source);
	} else {
		dock_in();
		dock_set_audio_switch(false);
		s->powered_dock_present = false;
		s->dock_connected_unknown = false;

		if (charge_source)
			*charge_source = MANTA_CHARGE_SOURCE_NONE;
	}

	dock_release(s);
	return ret;
}

static void dock_work_proc(struct work_struct *work)
{
	struct dock_state *s = container_of(work, struct dock_state,
			dock_work);
	wake_lock(&s->wake_lock);
	manta_force_update_pogo_charger();
	wake_unlock(&s->wake_lock);
}

static irqreturn_t pogo_data_interrupt(int irq, void *data)
{
	struct dock_state *s = data;
	pr_debug("%s: irq %d\n", __func__, irq);

	if (s->ignore_data_irq_once) {
		pr_debug("%s: ignored unwanted data_irq\n", __func__);
		s->ignore_data_irq_once = false;
		goto done;
	}

	if (s->powered_dock_present) {
		wake_lock(&s->wake_lock);
		queue_work(s->dock_wq, &s->dock_work);
	}
done:
	return IRQ_HANDLED;
}

static irqreturn_t pogo_dock_pd_interrupt(int irq, void *data)
{
	struct dock_state *s = data;
	unsigned long flags;

	pr_debug("%s: irq %d, state %d, gpio %d, mux %d, ta_mode %d\n",
		__func__, irq, s->debounce_state,
		gpio_get_value(GPIO_DOCK_PD_INT), s->spdif_mux_state,
		s->ta_check_mode);

	spin_lock_irqsave(&s->wait.lock, flags);

	if (s->spdif_mux_state || s->ta_check_mode)
		s->debounce_pending = true;
	else
		pogo_dock_pd_interrupt_locked(irq, data);

	spin_unlock_irqrestore(&s->wait.lock, flags);

	return IRQ_HANDLED;
}

static void pogo_debounce_timer(unsigned long data)
{
	struct dock_state *s = (struct dock_state *) data;
	unsigned long flags;

	pr_debug("%s: state %d, gpio %d\n", __func__, s->debounce_state,
		gpio_get_value(GPIO_DOCK_PD_INT));

	spin_lock_irqsave(&s->wait.lock, flags);

	switch (s->debounce_state) {
	case POGO_DEBOUNCE_UNSTABLE:
		/*
		 * The detect gpio changed in one the previous timeslots,
		 * so enable the irq, reset the timer, and wait again. If the
		 * detect gpio changed after we last disabled the interrupt we
		 * will get anther interrupt right away and the state will go
		 * back to POGO_DET_UNSTABLE.
		 */
		s->debounce_state = POGO_DEBOUNCE_WAIT_STABLE;
		enable_irq(s->dock_pd_irq);
		mod_timer(&s->debounce_timer,
				jiffies + msecs_to_jiffies(DEBOUNCE_DELAY_MS));

		if (s->ta_check_wait && !(--s->ta_check_wait))
			wake_up_locked(&s->wait);

		break;
	case POGO_DEBOUNCE_WAIT_STABLE:
		s->debounce_state = (s->spdif_mux_state ||
			gpio_get_value(GPIO_DOCK_PD_INT)) ?
			POGO_DEBOUNCE_DOCKED : POGO_DEBOUNCE_UNDOCKED;

		if (s->ta_check_wait)
			wake_up_locked(&s->wait);

		queue_work(s->dock_wq, &s->debounce_work);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&s->wait.lock, flags);
}

static void debounce_work_proc(struct work_struct *work)
{
	struct dock_state *s = container_of(work, struct dock_state,
			debounce_work);
	unsigned long flags;
	int state;

	spin_lock_irqsave(&s->wait.lock, flags);
	state = s->debounce_state;
	spin_unlock_irqrestore(&s->wait.lock, flags);

	switch (state) {
	case POGO_DEBOUNCE_DOCKED:
		switch_set_state(&dock_switch, POGO_DESK_DOCK);
		break;
	case POGO_DEBOUNCE_UNDOCKED:
		switch_set_state(&dock_switch, POGO_UNDOCKED);
		break;
	default:
		break;
	}

	spin_lock_irqsave(&s->wait.lock, flags);
	if (state == s->debounce_state)
		wake_unlock(&s->debounce_wake_lock);
	spin_unlock_irqrestore(&s->wait.lock, flags);
}

#ifdef DEBUG
static ssize_t dev_attr_vbus_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", ds.powered_dock_present ? 1 : 0);
}

static ssize_t dev_attr_powered_dock_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	if (size) {
		manta_pogo_set_vbus(buf[0] == '1', NULL);
		return size;
	} else
		return -EINVAL;
}
static DEVICE_ATTR(powered_dock, S_IRUGO | S_IWUSR,
	dev_attr_powered_dock_show, dev_attr_powered_dock_store);
#endif

#ifdef DEBUG
static ssize_t dev_attr_delay_ns_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", ds.mfm_delay_ns);
}

static ssize_t dev_attr_delay_ns_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	if (size) {
		ds.mfm_delay_ns = simple_strtoul(buf, NULL, 10);
		return size;
	} else
		return -EINVAL;
}
static DEVICE_ATTR(delay_ns, S_IRUGO | S_IWUSR, dev_attr_delay_ns_show,
		dev_attr_delay_ns_store);
#endif

static ssize_t dev_attr_dock_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;
	u8 dock_id[4];

	ret = dock_acquire(&ds, true);
	if (ret < 0)
		goto fail;
	ret = dock_read_multi(&ds, DOCK_ID_ADDR, dock_id, 4);
	dock_release(&ds);
	if (ret < 0)
		goto fail;

	ret = sprintf(buf, "%02x:%02x:%02x:%02x\n\n",
		dock_id[0], dock_id[1], dock_id[2], dock_id[3]);
fail:
	return ret;
}

static ssize_t dev_attr_dock_id_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int ret, i;
	int val[4];
	u8 dock_id[4];

	if (size < 11 || sscanf(buf, "%2x:%2x:%2x:%2x", &val[0], &val[1],
		&val[2], &val[3]) != 4)
		return -EINVAL;

	for (i = 0; i < 4; i++)
		dock_id[i] = val[i];

	ret = dock_acquire(&ds, true);
	if (ret < 0)
		goto fail;
	ret = dock_write_multi(&ds, DOCK_ID_ADDR, dock_id, 4);
	dock_release(&ds);
	if (ret < 0)
		goto fail;

	ret = size;
fail:
	return ret;
}
static DEVICE_ATTR(dock_id, S_IRUGO | S_IWUSR, dev_attr_dock_id_show,
		dev_attr_dock_id_store);

static ssize_t dev_attr_dock_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;

	ret = dock_acquire(&ds, true);
	if (ret < 0)
		goto fail;
	ret = dock_send_cmd(&ds, DOCK_VERSION, false, 0);
	dock_release(&ds);
	if (ret < 0)
		goto fail;

	ret = sprintf(buf, "0x%02x\n", ret);
fail:
	return ret;
}
static DEVICE_ATTR(dock_ver, S_IRUGO, dev_attr_dock_ver_show, NULL);

static ssize_t dev_attr_dock_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;

	ret = dock_acquire(&ds, true);
	if (ret < 0)
		goto fail;
	ret = dock_send_cmd(&ds, DOCK_STATUS, false, 0);
	dock_release(&ds);
	if (ret < 0)
		goto fail;

	ret = sprintf(buf, "0x%02x\n", ret);
fail:
	return ret;
}
static DEVICE_ATTR(dock_status, S_IRUGO, dev_attr_dock_status_show, NULL);

static int pogo_cpufreq_notifier(struct notifier_block *nb,
				 unsigned long event, void *data)
{
	struct dock_state *s = &ds;
	struct cpufreq_policy *policy = data;

	if (event != CPUFREQ_ADJUST)
		goto done;

	pr_debug("%s: adjusting cpu%d cpufreq to %d", __func__,
			policy->cpu, s->cpufreq_min);

	cpufreq_verify_within_limits(policy, s->cpufreq_min,
			policy->cpuinfo.max_freq);

done:
	return 0;
}

static struct notifier_block pogo_cpufreq_notifier_block = {
	.notifier_call = pogo_cpufreq_notifier,
};

void __init exynos5_manta_pogo_init(void)
{
	struct dock_state *s = &ds;
	int ret;

	if (exynos5_manta_get_revision() <= MANTA_REV_BETA)
		manta_pogo_gpios[0].gpio = GPIO_POGO_DATA_BETA;

	wake_lock_init(&s->wake_lock, WAKE_LOCK_SUSPEND, "dock");
	wake_lock_init(&s->debounce_wake_lock, WAKE_LOCK_SUSPEND, "dock_pd");

	INIT_WORK(&s->dock_work, dock_work_proc);
	INIT_WORK(&s->debounce_work, debounce_work_proc);
	s->dock_wq = create_singlethread_workqueue("dock");

	setup_timer(&s->debounce_timer, pogo_debounce_timer, (unsigned long)s);

	s3c_gpio_cfgpin(GPIO_POGO_SEL1, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_POGO_SEL1, S3C_GPIO_PULL_NONE);
	s5p_gpio_set_pd_cfg(GPIO_POGO_SEL1, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(GPIO_POGO_SEL1, S5P_GPIO_PD_UPDOWN_DISABLE);

	s5p_gpio_set_pd_cfg(GPIO_TA_CHECK_SEL, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(GPIO_TA_CHECK_SEL, S5P_GPIO_PD_UPDOWN_DISABLE);

	ret = gpio_request_array(manta_pogo_gpios,
				 ARRAY_SIZE(manta_pogo_gpios));
	if (ret)
		pr_err("%s: cannot request gpios\n", __func__);

	ret = gpio_request(GPIO_POGO_SPDIF, "pogo_spdif");
	if (ret)
		pr_err("%s: cannot request gpio POGO_SPDIF\n", __func__);

	manta_pogo_spdif_config(false);

	if (switch_dev_register(&dock_switch) == 0) {
		ret = device_create_file(dock_switch.dev, &dev_attr_dock_id);
		WARN_ON(ret);
		ret = device_create_file(dock_switch.dev, &dev_attr_dock_ver);
		WARN_ON(ret);
		ret = device_create_file(dock_switch.dev,
					 &dev_attr_dock_status);
		WARN_ON(ret);
#ifdef DEBUG
		ret = device_create_file(dock_switch.dev, &dev_attr_delay_ns);
		WARN_ON(ret);
		ret = device_create_file(dock_switch.dev,
			&dev_attr_unpowered_dock);
		WARN_ON(ret);
#endif
	}

	WARN_ON(switch_dev_register(&usb_audio_switch));

	ret = cpufreq_register_notifier(&pogo_cpufreq_notifier_block,
				  CPUFREQ_POLICY_NOTIFIER);
	if (ret)
		pr_warn("%s: cannot register cpufreq notifier\n", __func__);

	manta_pogo_set_vbus(0, NULL);
}

int __init pogo_data_irq_subsys_init(void)
{
	struct dock_state *s = &ds;
	int ret, data_irq;

	dock_in();
	s3c_gpio_setpull(_GPIO_DOCK, S3C_GPIO_PULL_NONE);

	data_irq = gpio_to_irq(_GPIO_DOCK);

	ret = request_irq(data_irq, pogo_data_interrupt, IRQF_TRIGGER_FALLING,
			"dock", &ds);
	if (ret < 0) {
		pr_err("%s: failed to request irq %d, rc: %d\n", __func__,
			data_irq, ret);
		goto done;
	}

	ret = enable_irq_wake(data_irq);
	if (ret) {
		pr_err("%s: failed to enable irq_wake for POGO_DATA\n",
			__func__);
		goto fail_data_irq_wake;
	}

	s3c_gpio_cfgpin(GPIO_DOCK_PD_INT, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(GPIO_DOCK_PD_INT, S3C_GPIO_PULL_NONE);

	s->dock_pd_irq = gpio_to_irq(GPIO_DOCK_PD_INT);

	ret = request_irq(s->dock_pd_irq, pogo_dock_pd_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "dock_pd", &ds);

	if (ret < 0) {
		pr_err("%s: failed to request dock_pd_int irq %d, rc: %d\n",
			__func__, s->dock_pd_irq, ret);
		goto fail_pd_irq_request;
	}

	ret = enable_irq_wake(s->dock_pd_irq);
	if (ret < 0) {
		pr_err("%s: failed to enable irq_wake for DOCK_PD_INT\n",
			__func__);
		goto fail_pd_irq_wake;
	}

	return 0;

fail_pd_irq_wake:
	free_irq(s->dock_pd_irq, &ds);

fail_pd_irq_request:
	disable_irq_wake(data_irq);

fail_data_irq_wake:
	free_irq(data_irq, &ds);

done:
	return ret;
}

subsys_initcall_sync(pogo_data_irq_subsys_init);
