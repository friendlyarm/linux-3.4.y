/*
 * drivers/power/nxe2000-battery.c
 *
 *  Copyright (C) 2013 Nexell
 *  bong kwan kook <kook@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
///////////////////////////////////////////////////////////////////////
// Date		ID				Description
//--------------------------------------------------------------------
//06-08-2015			modify	- Remove OTGID check.
//
//01-21-2015			modify	- Fuelgauge Patch.
//
//11-18-2014			modify	- modification of CONFIG_ARM_NXP4330_CPUFREQ_BY_RESOURCE. -> CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE
//
//11-05-2014			modify	- modification of CONFIG_ARM_NXP4330_CPUFREQ_BY_RESOURCE.
//
//10-21-2014			modify	- Changed the POWER_SUPPLY_PROP_ONLINE status.
//
//10-20-2014			modify	- Changed the CHGCTL1_REG -> NXE2000_REG_CHGCTL1.
//							- Added to INPUT_POWER_TYPE_ADP_NOBATTERY.
//							- Removed the INPUT_POWER_TYPE_ADP_UBC_LINKED.
//							- Removed the NXE2000_POS_CHGCTL1_NOBATOVLIM.
//
//10-14-2014			modify	- Charge control change when suspend.
//							- Power Supply Prop Online status change.
//
//10-13-2014			modify	- ENABLE_LOW_BATTERY_VSYS_DETECTION enable.
//
//10-07-2014			modify	- When Low Battery Detection disable,
// 							   fg_target_vsys setting default value.
//
//09-25-2014			modify	- Set the higher-priority VADP charge.
//
///////////////////////////////////////////////////////////////////////

#define NXE2000_BATTERY_VERSION "NXE2000_BATTERY_VERSION: 2014.11.27 V3.1.3.3(2015.06.08:modify)"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/power_supply.h>
#include <linux/mfd/nxe2000.h>
#include <linux/power/nxe2000_battery.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mfd/nxe2000.h>

/* nexell soc headers */
#include <mach/platform.h>
//#include <mach/devices.h>
#include <mach/soc.h>

#include <nxe2000-private.h>

////////////////////////////////////////
//  Debug
////////////////////////////////////////

// #define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define PM_LOGOUT 						printk
#else
#define PM_LOGOUT(fmt, arg...)			do {} while (0)
#endif

#ifdef ENABLE_DEBUG
#define DBGOUT(dev, fmt, arg...)		dev_info(dev, fmt, arg)
#else
#define DBGOUT(dev, fmt, arg...)		do {} while (0)
#endif

#ifdef ENABLE_DEBUG
#define NXE2000_REG_DUMP
#endif


////////////////////////////////////////
//  LOCAL define
////////////////////////////////////////
//#define BAT_RESUME_WORK_QUEUE
#define ENABLE_FUEL_GAUGE_FUNCTION
#define ENABLE_LOW_BATTERY_VSYS_DETECTION
//#define ENABLE_LOW_BATTERY_VBAT_DETECTION
//#define ENABLE_FACTORY_MODE
#define DISABLE_CHARGER_TIMER
/* #define ENABLE_FG_KEEP_ON_MODE */
#define ENABLE_OCV_TABLE_CALIB
/* #define ENABLE_MASKING_INTERRUPT_IN_SLEEP */
#define KOOK_UBC_CHECK
//#define KOOK_ADP_ONLY_MODE
//#define KOOK_LOW_VOL_DET_TEST
//#define KOOK_FAKE_POWER_ON_EVENT
#define PMIC_VBUSCHK_COUNT 		3


#if (CFG_PMIC_BAT_CHG_SUPPORT == 1)
#include <nxe2000_battery_init.h>
#endif

#define NXE2000_CHG_MASK    ( (0x1 << NXE2000_POS_CHGCTL1_CHGP)         \
                            | (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN)    \
                            | (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN)    \
                            | (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN) \
                            | (0x1 << NXE2000_POS_CHGCTL1_SUSPEND) )



/* FG setting */
//#define NXE2000_REL1_SEL_VALUE		64
//#define NXE2000_REL2_SEL_VALUE		0

#ifndef MAX
#define MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#endif
#ifndef MIN
#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))
#endif

#ifdef ENABLE_FUEL_GAUGE_FUNCTION
/* define for FG delayed time */
#define NXE2000_MONITOR_START_TIME			15
#define NXE2000_FG_RESET_TIME				6
#define NXE2000_FG_STABLE_TIME				120
#define NXE2000_FIRST_DISPLAY_TIME		12
#define NXE2000_DISPLAY_UPDATE_TIME		15
#define NXE2000_LOW_VOL_DOWN_TIME			10
#define NXE2000_CHARGE_MONITOR_TIME		20
#define NXE2000_CHARGE_RESUME_TIME			1
#define NXE2000_CHARGE_CALC_TIME			1
#define NXE2000_JEITA_UPDATE_TIME			60
#define NXE2000_DELAY_TIME					60
/* define for FG parameter */
#define NXE2000_MAX_RESET_SOC_DIFF			5
#define NXE2000_GET_CHARGE_NUM				10
#define NXE2000_UPDATE_COUNT_DISP			4
#define NXE2000_UPDATE_COUNT_FULL			4
#define NXE2000_UPDATE_COUNT_FULL_RESET		7
#define NXE2000_CHARGE_UPDATE_TIME			3
#define NXE2000_FULL_WAIT_TIME				1	/* 4 */
#define NXE2000_RE_CAP_GO_DOWN				10	/* 40 */
#define NXE2000_ENTER_LOW_VOL				70
#define NXE2000_TAH_SEL2					5
#define NXE2000_TAL_SEL2					6

#define NXE2000_OCV_OFFSET_BOUND			3
#define NXE2000_OCV_OFFSET_RATIO			2

#define NXE2000_ENTER_FULL_STATE_OCV		9
#define NXE2000_ENTER_FULL_STATE_DSOC		90

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
#define LOW_BATTERY_DETECTION_TIME		1
#endif

/* define for FG status */
enum {
	NXE2000_SOCA_START,
	NXE2000_SOCA_UNSTABLE,
	NXE2000_SOCA_FG_RESET,
	NXE2000_SOCA_DISP,
	NXE2000_SOCA_STABLE,
	NXE2000_SOCA_ZERO,
	NXE2000_SOCA_FULL,
	NXE2000_SOCA_LOW_VOL,
};
#endif

struct nxe2000_soca_info {
	int Rbat;
	int n_cap;
	int ocv_table_def[11];
	int ocv_table[11];
	int ocv_table_low[11];
	int soc;		/* Latest FG SOC value */
	int displayed_soc;
	int suspend_soc;
	int suspend_cc;
	int status;		/* SOCA status 0: Not initial; 5: Finished */
	int stable_count;
	int chg_status;		/* chg_status */
	int soc_delta;		/* soc delta for status3(DISP) */
	int cc_delta;
	int cc_cap_offset;
	int last_soc;
	int last_displayed_soc;
	int ready_fg;
	int reset_count;
	int reset_soc[3];
	int chg_cmp_times;
	int dischg_state;
	int Vbat[NXE2000_GET_CHARGE_NUM];
	int Vsys[NXE2000_GET_CHARGE_NUM];
	int Ibat[NXE2000_GET_CHARGE_NUM];
	int Ocv[NXE2000_GET_CHARGE_NUM];
	int Vbat_ave;
	int Vbat_old;
	int Vsys_ave;
	int Ibat_ave;
	int chg_count;
	int full_reset_count;
	int soc_full;
	int fc_cap;
	/* for LOW VOL state */
	int target_use_cap;
	int hurry_up_flg;
	int zero_flg;
	int re_cap_old;
	int cutoff_ocv;
	int Rsys;
	int target_vsys;
	int target_ibat;
	int jt_limit;
	int OCV100_min;
	int OCV100_max;
	int R_low;
	int rsoc_ready_flag;
	int init_pswr;
	int last_soc_full;
};

struct nxe2000_battery_info {
	struct device      *dev;
	struct power_supply	battery;
	struct delayed_work	monitor_work;
	struct delayed_work	displayed_work;
	struct delayed_work	charge_stable_work;
	struct delayed_work	changed_work;
#if (CFG_SW_UBC_ENABLE == 1)
	struct delayed_work	sw_ubc_work;
	bool				flag_set_ilimit;
	int					is_sdp_type;
	int					pmic_vbuschk_count;
#endif
#ifdef KOOK_UBC_CHECK
	struct delayed_work	get_charger_work;
	int					chg_work_recheck_count;
#endif
	struct delayed_work	otgid_detect_work;
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	struct delayed_work	low_battery_work;
#endif
	struct delayed_work	charge_monitor_work;
	struct delayed_work	get_charge_work;
	struct delayed_work	jeita_work;

	struct work_struct	irq_work;	/* for Charging & VADP/VUSB */

	struct workqueue_struct *monitor_wqueue;
	struct workqueue_struct *workqueue;	/* for Charging & VUSB/VADP */

#ifdef ENABLE_FACTORY_MODE
	struct delayed_work	factory_mode_work;
	struct workqueue_struct *factory_mode_wqueue;
#endif

	struct mutex	lock;
	unsigned long	monitor_time;
	int				adc_vdd_mv;
	int				multiple;
	int				status;
	int				input_power_type;
	int				gpio_otg_usbid;
	int				gpio_otg_usbid_status;
	int				gpio_otg_vbus;
	int				gpio_pmic_vbus;
	int				gpio_pmic_lowbat;
	int				min_voltage;
	int				max_voltage;
	int				cur_voltage;
	int				capacity;
	int				full_capacity;
	int				battery_temp;
	int				time_to_empty;
	int				time_to_full;
	int				chg_ctr;
	int				chg_stat1;
	int				chg_extif;
	int				extif_type;
	unsigned		present:1;
	u16				delay;
	struct nxe2000_soca_info *soca;
	int				first_pwon;
	int				first_soc;
	bool			factory_mode_complete;
	bool			entry_factory_mode;
	int				ch_vfchg;
	int				ch_vrchg;
	int				ch_vbatovset;
	int				ch_ichg;
	int				ch_ilim_adp;
	int				ch_ilim_usb;
	int				ch_ilim_dcp;
	int				ch_icchg;
	int				ch_ichg_slp;
	int				fg_target_vsys;
	int				fg_target_ibat;
	int				fg_poff_vbat;
	int				jt_en;
	int				jt_hw_sw;
	int				jt_temp_h;
	int				jt_temp_l;
	int				jt_vfchg_h;
	int				jt_vfchg_l;
	int				jt_ichg_h;
	int				jt_ichg_l;

	int				online_state;
	int				ubc_check_count;

	int				low_vbat_vol_mv;
	int				low_vbat_thl;
	int				low_vbat_val;
	int				low_vsys_vol_mv;
	int				low_vsys_thl;
	int				low_vsys_val;

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	bool			low_bat_power_off;
#endif

	int 			num;

	struct work_struct	resume_work;	/* add by jhkim */
};

int g_full_flag;
int charger_irq;
int g_soc;
int g_fg_on_mode;

//static volatile int nxe2000_power_suspend_status;
#ifdef KOOK_FAKE_POWER_ON_EVENT
static volatile int nxe2000_power_resume_status;
static volatile int nxe2000_power_lowbat;
#endif

/*This is for full state*/
static int BatteryTableFlageDef=0;
static int BatteryTypeDef=0;

#if defined(CONFIG_USB_DWCOTG)
static struct nxe2000_battery_info *info_by_dwc;
static int vbus_irq_disabled = 0;
extern void otg_phy_init(void);
extern void otg_phy_off(void);
extern void otg_phy_suspend(void);

extern void otg_clk_enable(void);
extern void otg_clk_disable(void);

#if (CFG_SW_UBC_ENABLE == 1)
extern int  dwc_otg_pcd_get_ep0_state(void);
extern void dwc_otg_pcd_clear_ep0_state(void);
#endif
#endif

static int Battery_Type(void)
{
	return BatteryTypeDef;
}

static int Battery_Table(void)
{
	return BatteryTableFlageDef;
}

static void nxe2000_battery_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, monitor_work.work);

	power_supply_changed(&info->battery);
	queue_delayed_work(info->monitor_wqueue, &info->monitor_work,
					 info->monitor_time * HZ);
}

#ifdef NXE2000_REG_DUMP
void nxe2000_register_dump(struct device *dev)
{
	struct nxe2000_battery_info *info = dev_get_drvdata(dev);

	s32 ret=0;
	u16 i=0;
	u8 value[NXE2000_NUM_OF_REGS]={0};

	PM_DBGOUT("##########################################################\n");
	PM_DBGOUT("##\e[31m %s()\e[0m                               #\n", __func__);
	PM_DBGOUT("##########################################################\n");
	PM_DBGOUT("       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F\n");

	for(i=0; i<=NXE2000_NUM_OF_REGS; i++)
	{
		if(i%16 == 0)
			PM_DBGOUT("  %02X:", i);

		if(i%4 == 0)
			PM_DBGOUT(" ");

		ret = nxe2000_read(dev, i, &value[i]);
		if(!ret)
			PM_DBGOUT("%02x ", value[i]);
		else
			PM_DBGOUT("\e[31mxx\e[0m ");

		if((i+1)%16 == 0)
			PM_DBGOUT("\n");
	}

	PM_DBGOUT("##########################################################\n");
	PM_DBGOUT("## : ADC_CNT1(0x%02x)    : 0x%02x \n", NXE2000_ADC_CNT1, value[NXE2000_ADC_CNT1]);
	PM_DBGOUT("## : ADC_CNT2(0x%02x)    : 0x%02x \n", NXE2000_ADC_CNT2, value[NXE2000_ADC_CNT2]);
	PM_DBGOUT("## : ADC_CNT3(0x%02x)    : 0x%02x \n", NXE2000_ADC_CNT3, value[NXE2000_ADC_CNT3]);
	PM_DBGOUT("## : INT_EN_ADC1(0x%02x) : 0x%02x \n", NXE2000_INT_EN_ADC1, value[NXE2000_INT_EN_ADC1]);
	PM_DBGOUT("## : VBATDATAH(0x%02x)   : 0x%02x \n", NXE2000_REG_VBATDATAH, value[NXE2000_REG_VBATDATAH]);
	PM_DBGOUT("## : VBAT_THL(0x%02x)    : 0x%02x \n", NXE2000_ADC_VBAT_THL, value[NXE2000_ADC_VBAT_THL]);
	PM_DBGOUT("## : VSYSDATAH(0x%02x)   : 0x%02x \n", NXE2000_REG_VSYSDATAH, value[NXE2000_REG_VSYSDATAH]);
	PM_DBGOUT("## : VSYS_THL(0x%02x)    : 0x%02x \n", NXE2000_ADC_VSYS_THL, value[NXE2000_ADC_VSYS_THL]);

	//if(info->gpio_otg_usbid > -1)
	PM_DBGOUT("## : gpio_otg_usbid    : 0x%02x \n", info->gpio_otg_usbid_status);

	if (info->gpio_pmic_vbus > -1)
		PM_DBGOUT("## : gpio_pmic_vbus    : 0x%02x \n", gpio_get_value(info->gpio_pmic_vbus));

	PM_DBGOUT("##########################################################\n");
}
#endif

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
//#define pr_debug	printk

extern int NXP_Get_BoardTemperature(void);
extern int isCheck_ChargeStop_byResource(void);

int pmic_occur_dieError=0;
int isOccured_dieError(void)
{
	return pmic_occur_dieError;
}
EXPORT_SYMBOL_GPL(isOccured_dieError);


char strChargingState[15][32] = {
	"CHG OFF",
	"Charge Ready(VADP)",
	"Trickle Charge",
	"Rapid Charge",
	"Charge Complete",
	"SUSPEND",
	"VCHG Over Voltage",
	"Battery Error",
	"No Battery",
	"Battery Over Voltage",
	"Battery Temp Error",
	"Die Error",
	"Die Shutdown",
	"No Battery2",
	"Charge Ready(VUSB)"
};

char strSupply[3][16]={
	"Battery",
	"Adapter",
	"Usb"
};

#endif


#ifdef ENABLE_FUEL_GAUGE_FUNCTION
static int measure_vbatt_FG(struct nxe2000_battery_info *info, int *data);
static int measure_Ibatt_FG(struct nxe2000_battery_info *info, int *data);
static int calc_capacity(struct nxe2000_battery_info *info);
static int calc_capacity_2(struct nxe2000_battery_info *info);
static int calc_first_soc(struct nxe2000_battery_info *info);
static void suspend_charge4first_soc(struct nxe2000_battery_info *info);
static int get_OCV_init_Data(struct nxe2000_battery_info *info, int index);
static int get_OCV_voltage(struct nxe2000_battery_info *info, int index);
static int get_check_fuel_gauge_reg(struct nxe2000_battery_info *info,
					 int Reg_h, int Reg_l, int enable_bit);
static int calc_capacity_in_period(struct nxe2000_battery_info *info,
				 int *cc_cap, bool *is_charging, int cc_rst);
static int get_power_supply_status(struct nxe2000_battery_info *info);
static int get_power_supply_Android_status(struct nxe2000_battery_info *info);
static int measure_vsys_ADC(struct nxe2000_battery_info *info, int *data);
static int set_low_bat_dsoc(struct nxe2000_battery_info *info, int val);
static int Calc_Linear_Interpolation(int x0, int y0, int x1, int y1, int y);
#ifndef CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE
static int get_battery_temp(struct nxe2000_battery_info *info);
#endif
static int get_battery_temp_2(struct nxe2000_battery_info *info);
static int check_jeita_status(struct nxe2000_battery_info *info, bool *is_jeita_updated);
static void nxe2000_scaling_OCV_table(struct nxe2000_battery_info *info, int cutoff_vol, int full_vol, int *start_per, int *end_per);

static int calc_ocv(struct nxe2000_battery_info *info)
{
	int Vbat = 0;
	int Ibat = 0;
	int ret;
	int ocv;

	ret = measure_vbatt_FG(info, &Vbat);
	ret = measure_Ibatt_FG(info, &Ibat);

	ocv = Vbat - Ibat * info->soca->Rbat;

	return ocv;
}

static int calc_soc_on_ocv(struct nxe2000_battery_info *info, int Ocv)
{
	int i;
	int capacity=0;

	/* capacity is 0.01% unit */
	if (info->soca->ocv_table[0] >= Ocv) {
		capacity = 1 * 100;
	} else if (info->soca->ocv_table[10] <= Ocv) {
		capacity = 100 * 100;
	} else {
		for (i = 1; i < 11; i++) {
			if (info->soca->ocv_table[i] >= Ocv) {
				/* unit is 0.01% */
				capacity = Calc_Linear_Interpolation(
					(i-1)*10 * 100, info->soca->ocv_table[i-1], i*10 * 100,
					 info->soca->ocv_table[i], Ocv);
				if(capacity < 100){
					capacity = 100;
				}
				break;
			}
		}
	}

	PM_LOGOUT(KERN_INFO "PMU: %s capacity(%d)\n",
			 __func__, capacity);

	return capacity;
}

static int reset_FG_process(struct nxe2000_battery_info *info)
{
	int err;

	//err = set_Rlow(info);
	//err = nxe2000_Check_OCV_Offset(info);
	err = nxe2000_write(info->dev->parent,
					 FG_CTRL_REG, 0x51);
	if (err < 0)
		err = nxe2000_write(info->dev->parent,
						FG_CTRL_REG, 0x51);

	info->soca->ready_fg = 0;
	info->soca->rsoc_ready_flag = 1;
	return err;
}


static int check_charge_status_2(struct nxe2000_battery_info *info, int displayed_soc_temp)
{
	if (displayed_soc_temp < 0)
			displayed_soc_temp = 0;

	get_power_supply_status(info);
	info->soca->soc = calc_capacity(info) * 100;

	PM_LOGOUT(KERN_INFO "PMU: %s chg_status(%x), dsoc_temp(%d), g_full(%d), Ibat(%d)\n",
		 __func__, info->soca->chg_status, displayed_soc_temp, g_full_flag, info->soca->Ibat_ave);
	if (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status) {
		PM_LOGOUT(KERN_INFO "PMU: %s CHGCMP --> Set 100%%\n", __func__);
		g_full_flag = 1;
		info->soca->soc_full = info->soca->soc;
		info->soca->displayed_soc = 100*100;
		info->soca->full_reset_count = 0;
		info->soca->fc_cap = 0;
		info->soca->last_soc_full = 0;
	}
	if (info->soca->Ibat_ave >= 0) {
		if (g_full_flag == 1) {
			info->soca->displayed_soc = 100*100;
			PM_LOGOUT(KERN_INFO "PMU: %s Ibat>0, g_full=1 --> Set 100%%\n", __func__);
		} else {
			if ((displayed_soc_temp+50)/100 >= 99) {
				info->soca->displayed_soc = 99 * 100;
				PM_LOGOUT(KERN_INFO "PMU: %s Ibat>0, g_full=0 --> Set 99%%\n", __func__);
			} else {
				info->soca->displayed_soc = displayed_soc_temp;
				PM_LOGOUT(KERN_INFO "PMU: %s Ibat>0, g_full=0 --> Set dsoc_temp\n", __func__);
			}
		}
	}
	if (info->soca->Ibat_ave < 0) {
		if (g_full_flag == 1) {
			if (calc_ocv(info) < get_OCV_voltage(info, 9)) {
				g_full_flag = 0;
				//info->soca->displayed_soc = 100*100;
				info->soca->displayed_soc = displayed_soc_temp;
				PM_LOGOUT(KERN_INFO "PMU: %s Ibat<0, g_full=1->0 --> Set dsoc_temp\n", __func__);
			} else {
				info->soca->displayed_soc = 100*100;
				PM_LOGOUT(KERN_INFO "PMU: %s Ibat<0, g_full=1 --> Set 100%%\n", __func__);
			}
		} else {
			g_full_flag = 0;
			if ((displayed_soc_temp+50)/100 >= 99) {
				info->soca->displayed_soc = 99 * 100;
				PM_LOGOUT(KERN_INFO "PMU: %s Ibat<0, g_full=0 --> Set 99%%\n", __func__);
			} else {
				info->soca->displayed_soc = displayed_soc_temp;
				PM_LOGOUT(KERN_INFO "PMU: %s Ibat<0, g_full=0 --> Set dsoc_temp\n", __func__);
			}
		}
	}

	if (NXE2000_SOCA_START == info->soca->status) {
		if ((info->first_pwon == 0) && !g_fg_on_mode) {
			if ((info->soca->init_pswr == 100)
				&& (info->soca->cc_delta > -100)) {
				PM_LOGOUT(KERN_INFO "PMU: %s Set 100%%\n", __func__);
				g_full_flag = 1;
				info->soca->soc_full = info->soca->soc;
				info->soca->displayed_soc = 100*100;
				info->soca->full_reset_count = 0;
			}
		}
	} else {
		if ((info->soca->suspend_soc == 10000)
			&& (info->soca->cc_delta > -100)) {
			PM_LOGOUT(KERN_INFO "PMU: %s Set 100%%\n", __func__);
			info->soca->displayed_soc = 100*100;
		}
	}

	return info->soca->displayed_soc;
}

/**
* Calculate Capacity in a period
* - read CC_SUM & FA_CAP from Coulom Counter
* -  and calculate Capacity.
* @cc_cap: capacity in a period, unit 0.01%
* @is_charging: Flag of charging current direction
*               TRUE : charging (plus)
*               FALSE: discharging (minus)
* @cc_rst: reset CC_SUM or not
*               0 : not reset
*               1 : reset
*               2 : half reset (Leave under 1% of FACAP)
**/
static int calc_capacity_in_period(struct nxe2000_battery_info *info,
				 int *cc_cap, bool *is_charging, int cc_rst)
{
	int err;
	uint8_t 	cc_sum_reg[4];
	uint8_t 	cc_clr[4] = {0, 0, 0, 0};
	uint8_t 	fa_cap_reg[2];
	uint16_t 	fa_cap;
	uint32_t 	cc_sum;
	int		cc_stop_flag;
	uint8_t 	status;
	uint8_t 	charge_state;
	uint8_t charge_backup;
	int 		Ocv;
	uint32_t 	cc_cap_temp;
	uint32_t 	cc_cap_min;
	int		cc_cap_res;
	int		fa_cap_int;
	long		cc_sum_int;
	long		cc_sum_dec;

	*is_charging = true;	/* currrent state initialize -> charging */

	if (info->entry_factory_mode)
		return 0;

	/* Read FA_CAP */
	err = nxe2000_bulk_reads(info->dev->parent,
				 FA_CAP_H_REG, 2, fa_cap_reg);
	if (err < 0)
		goto out;

	/* fa_cap = *(uint16_t*)fa_cap_reg & 0x7fff; */
	fa_cap = (fa_cap_reg[0] << 8 | fa_cap_reg[1]) & 0x7fff;


	/* get  power supply status */
	err = nxe2000_read(info->dev->parent, CHGSTATE_REG, &status);
	if (err < 0)
		goto out;
	charge_state = (status & 0x1F);
	Ocv = calc_ocv(info);
	if (charge_state == CHG_STATE_CHG_COMPLETE) {
		/* Check CHG status is complete or not */
		cc_stop_flag = 0;
	}
	else if (Ocv < get_OCV_voltage(info, 9))
	{
		/* Check VBAT is high level or not */
		cc_stop_flag = 0;
	} else {
		cc_stop_flag = 1;
	}

	if (cc_stop_flag == 1)
	{
		/* Disable Charging/Completion Interrupt */
		err = nxe2000_set_bits(info->dev->parent,
						NXE2000_INT_MSK_CHGSTS1, 0x01);
		if (err < 0)
			goto out;

		/* disable charging */
		nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &charge_backup);
		err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
		if (err < 0)
			goto out;
	}

	/* Read CC_SUM */
	err = nxe2000_bulk_reads(info->dev->parent,
					CC_SUMREG3_REG, 4, cc_sum_reg);
	if (err < 0)
		goto out;

	/* cc_sum = *(uint32_t*)cc_sum_reg; */
	cc_sum = cc_sum_reg[0] << 24 | cc_sum_reg[1] << 16 |
				cc_sum_reg[2] << 8 | cc_sum_reg[3];

	/* calculation  two's complement of CC_SUM */
	if (cc_sum & 0x80000000) {
		cc_sum = (cc_sum^0xffffffff)+0x01;
		*is_charging = false;		/* discharge */
	}

	if (cc_rst == 1) {
		/* CC_pause enter */
		err = nxe2000_write(info->dev->parent, NXE2000_REG_CC_CTRL, 0x01);
		if (err < 0)
			goto out;

		/* CC_SUM <- 0 */
		err = nxe2000_bulk_writes(info->dev->parent,
						CC_SUMREG3_REG, 4, cc_clr);
		if (err < 0)
			goto out;
	} else if (cc_rst == 2) {
		/* Check 1%[mAs] of FA_CAP (FA_CAP * 3600 /100) */
		fa_cap_int = fa_cap * 36;
		cc_sum_int = cc_sum / fa_cap_int;
		cc_sum_dec = cc_sum % fa_cap_int;

		if (*is_charging == false) {
			cc_sum_dec = (cc_sum_dec^0xffffffff) + 1;
 		}

		PM_LOGOUT(KERN_INFO "PMU %s 1%%FACAP(%d)[mAs], cc_sum(%u)[mAs], cc_sum_dec(%ld)\n",
							 __func__, fa_cap_int, cc_sum, cc_sum_dec);

		if (cc_sum_int != 0) {
			cc_clr[0] = (uint8_t)(cc_sum_dec >> 24) & 0xff;
			cc_clr[1] = (uint8_t)(cc_sum_dec >> 16) & 0xff;
			cc_clr[2] = (uint8_t)(cc_sum_dec >> 8) & 0xff;
			cc_clr[3] = (uint8_t)cc_sum_dec & 0xff;

			/* CC_pause enter */
			err = nxe2000_write(info->dev->parent, NXE2000_REG_CC_CTRL, 0x01);
			if (err < 0)
				goto out;

			/* CC_SUM <- 0 */
			err = nxe2000_bulk_writes(info->dev->parent,
							CC_SUMREG3_REG, 4, cc_clr);
			if (err < 0)
				goto out;
			PM_LOGOUT(KERN_INFO "PMU %s Half-Clear CC, cc_sum is over 1%%\n",__func__);
		}
	}

	/* CC_pause exist */
	err = nxe2000_write(info->dev->parent, CC_CTRL_REG, 0);
	if (err < 0)
		goto out;
	if (cc_stop_flag == 1)
	{

		/* Enable charging */
		err = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, charge_backup);
		if (err < 0)
			goto out;

		udelay(1000);

		/* Clear Charging Interrupt status */
		err = nxe2000_clr_bits(info->dev->parent,
					NXE2000_INT_IR_CHGSTS1, 0x01);
		if (err < 0)
			goto out;

//		nxe2000_read(info->dev->parent, NXE2000_INT_IR_CHGSTS1, &val);

		/* Enable Charging Interrupt */
		err = nxe2000_clr_bits(info->dev->parent,
						NXE2000_INT_MSK_CHGSTS1, 0x01);
		if (err < 0)
			goto out;
	}


	/* (CC_SUM x 10000)/3600/FA_CAP */
	if(fa_cap == 0)
		goto out;
	else
		*cc_cap = cc_sum*25/9/fa_cap;       /* unit is 0.01% */

	//////////////////////////////////////////////////////////////////
	if (cc_rst == 1) {
		cc_cap_min = fa_cap*3600/100/100/100;   /* Unit is 0.0001% */

		if(cc_cap_min == 0)
			goto out;
		else
			cc_cap_temp = cc_sum / cc_cap_min;

		cc_cap_res = cc_cap_temp % 100;

		if(*is_charging) {
			info->soca->cc_cap_offset += cc_cap_res;
			if (info->soca->cc_cap_offset >= 100) {
				*cc_cap += 1;
				info->soca->cc_cap_offset %= 100;
			}
		} else {
			info->soca->cc_cap_offset -= cc_cap_res;
			if (info->soca->cc_cap_offset <= -100) {
				*cc_cap += 1;
				info->soca->cc_cap_offset %= 100;
			}
		}
	} else {
		info->soca->cc_cap_offset = 0;
	}

	//////////////////////////////////////////////////////////////////
	return 0;
out:
	/* CC_pause exist */
	err = nxe2000_write(info->dev->parent, CC_CTRL_REG, 0);

	dev_err(info->dev, "Error !!-----\n");
	return err;
}
/**
* Calculate target using capacity
**/
static int get_target_use_cap(struct nxe2000_battery_info *info)
{
	int i,j;
	int ocv_table[11];
	int temp;
	int Ocv_ZeroPer_now;
	int Ibat_now;
	int fa_cap,use_cap;
	int FA_CAP_now;
	int start_per = 0;
	int RE_CAP_now;
	int CC_OnePer_step;
	int Ibat_min;

	int Ocv_now_table;
	int Rsys_now;

	/* get const value */
	Ibat_min = -1 * info->soca->target_ibat;
	if (info->soca->Ibat_ave > Ibat_min) /* I bat is minus */
	{
		Ibat_now = Ibat_min;
	} else {
		Ibat_now = info->soca->Ibat_ave;
	}
	fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
								0x7fff);
	use_cap = fa_cap - info->soca->re_cap_old;

	/* get OCV table % */
	for (i = 0; i <= 10; i = i+1) {
		temp = (battery_init_para[info->num][i*2]<<8)
			 | (battery_init_para[info->num][i*2+1]);
		/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
		temp = ((temp * 50000 * 10 / 4095) + 5) / 10;
		ocv_table[i] = temp;
	}

	/* Find out Current OCV */
	i = info->soca->soc/1000;
	j = info->soca->soc - info->soca->soc/1000*1000;
	Ocv_now_table = ocv_table[i]*100+(ocv_table[i+1]-ocv_table[i])*j/10;

	//if (info->soca->Ibat_ave < -1000)
		Rsys_now = (info->soca->Vsys_ave - Ocv_now_table) / info->soca->Ibat_ave;
	//else
	//	Rsys_now = info->soca->Rsys;
	//	Rsys_now = MAX(info->soca->Rsys/2, Rsys_now);


	Ocv_ZeroPer_now = info->soca->target_vsys * 1000 - Ibat_now * Rsys_now;

	PM_LOGOUT("PMU: ------- Ocv_now_table= %d: Rsys_now= %d =======\n",
	       Ocv_now_table, Rsys_now);

	PM_LOGOUT("PMU: ------- Rsys= %d: cutoff_ocv= %d: Ocv_ZeroPer_now= %d =======\n",
	       info->soca->Rsys, info->soca->cutoff_ocv, Ocv_ZeroPer_now);

	/* get FA_CAP_now */
	for (i = 1; i < 11; i++) {
		if (ocv_table[i] >= Ocv_ZeroPer_now / 100) {
			/* unit is 0.001% */
			start_per = Calc_Linear_Interpolation(
				(i-1)*1000, ocv_table[i-1], i*1000,
				 ocv_table[i], (Ocv_ZeroPer_now / 100));
			i = 11;
		}
	}

	start_per = MAX(0, start_per);

	FA_CAP_now = fa_cap * ((10000 - start_per) / 100 ) / 100;

	PM_LOGOUT("PMU: ------- Target_Cutoff_Vol= %d: Ocv_ZeroPer_now= %d: start_per= %d =======\n",
	       info->soca->cutoff_ocv, Ocv_ZeroPer_now, start_per);

	/* get RE_CAP_now */
	RE_CAP_now = FA_CAP_now - use_cap;

	if (RE_CAP_now < NXE2000_RE_CAP_GO_DOWN) {
		info->soca->hurry_up_flg = 1;
	} else if (info->soca->Vsys_ave < info->soca->target_vsys*1000) {
		info->soca->hurry_up_flg = 1;
	} else if (info->fg_poff_vbat != 0) {
		if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
			info->soca->hurry_up_flg = 1;
		} else {
			info->soca->hurry_up_flg = 0;
		}
	} else {
		info->soca->hurry_up_flg = 0;
	}

	/* get CC_OnePer_step */
	if (info->soca->displayed_soc > 0) { /* avoid divide-by-0 */
		CC_OnePer_step = RE_CAP_now / (info->soca->displayed_soc / 100 + 1);
	} else {
		CC_OnePer_step = 0;
	}
	/* get info->soca->target_use_cap */
	info->soca->target_use_cap = use_cap + CC_OnePer_step;

	PM_LOGOUT("PMU: ------- FA_CAP_now= %d: RE_CAP_now= %d: CC_OnePer_step= %d: target_use_cap= %d: hurry_up_flg= %d -------\n",
	       FA_CAP_now, RE_CAP_now, CC_OnePer_step, info->soca->target_use_cap, info->soca->hurry_up_flg);

	return 0;
}
#ifdef ENABLE_OCV_TABLE_CALIB
/**
* Calibration OCV Table
* - Update the value of VBAT on 100% in OCV table
*    if battery is Full charged.
* - int vbat_ocv <- unit is uV
**/
static int calib_ocvTable(struct nxe2000_battery_info *info, int vbat_ocv)
{
	int ret;
	int cutoff_ocv;
	int i;
	int ocv100_new;
	int start_per = 0;
	int end_per = 0;

	if (info->soca->Ibat_ave > NXE2000_REL1_SEL_VALUE) {
		return 0;
	}

	if (vbat_ocv < info->soca->OCV100_max) {
		if (vbat_ocv < info->soca->OCV100_min)
			ocv100_new = info->soca->OCV100_min;
		else
			ocv100_new = vbat_ocv;
	} else {
		ocv100_new = info->soca->OCV100_max;
	}

	/* FG_En Off */
	ret = nxe2000_clr_bits(info->dev->parent, FG_CTRL_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev,"Error in FG_En OFF\n");
		goto err;
	}


	//cutoff_ocv = (battery_init_para[info->num][0]<<8) | (battery_init_para[info->num][1]);
	cutoff_ocv = get_OCV_voltage(info, 0);

	info->soca->ocv_table_def[10] = info->soca->OCV100_max;

	nxe2000_scaling_OCV_table(info, cutoff_ocv/1000, ocv100_new/1000, &start_per, &end_per);

	ret = nxe2000_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 22, battery_init_para[info->num]);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	for (i = 0; i <= 10; i = i+1) {
		info->soca->ocv_table[i] = get_OCV_voltage(info, i);
	}

	/* FG_En on & Reset*/
	ret = reset_FG_process(info);
	if (ret < 0) {
		dev_err(info->dev, "Error in FG_En On & Reset %d\n", ret);
		goto err;
	}

	return 0;
err:
	return ret;

}

#endif

static void nxe2000_displayed_work(struct work_struct *work)
{
	int err;
	uint8_t val;
	uint8_t val2;
	int soc_round;
	int last_soc_round;
	int last_disp_round;
	int displayed_soc_temp;
	int disp_dec;
	int cc_cap = 0;
	bool is_charging = true;
	int re_cap,fa_cap,use_cap;
	bool is_jeita_updated;
	uint8_t reg_val;
	int delay_flag = 0;
	int Vbat = 0;
	int Ibat = 0;
	int Vsys = 0;
	int temp_ocv;
	int fc_delta = 0;
	int temp_soc;
	int current_soc_full;
	int calculated_ocv;
	int full_rate = 0;
	int soc_now;
	int cc_poff_term;
	int cc_delta_offset;
	int cc_correct_value;

	struct nxe2000_battery_info *info = container_of(work, struct nxe2000_battery_info, displayed_work.work);

	if (info->entry_factory_mode) {
		info->soca->status = NXE2000_SOCA_STABLE;
		info->soca->displayed_soc = -EINVAL;
		info->soca->ready_fg = 0;
		return;
	}

	PM_LOGOUT("\n######################################################\n");
	PM_LOGOUT("## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);

	mutex_lock(&info->lock);

	is_jeita_updated = false;

	if ((NXE2000_SOCA_START == info->soca->status)
		 || (NXE2000_SOCA_STABLE == info->soca->status)
		 || (NXE2000_SOCA_FULL == info->soca->status))
		{
			info->soca->ready_fg = 1;
		}
	//if (NXE2000_SOCA_FG_RESET != info->soca->status)
	//	Set_back_ocv_table(info);

	/* judge Full state or Moni Vsys state */
	calculated_ocv = calc_ocv(info);
	if ((NXE2000_SOCA_DISP == info->soca->status)
		 || (NXE2000_SOCA_STABLE == info->soca->status)) {
		/* caluc 95% ocv */
		temp_ocv = get_OCV_voltage(info, 10) -
					(get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))/2;

		if(g_full_flag == 1){	/* for issue 1 solution start*/
			info->soca->status = NXE2000_SOCA_FULL;
			info->soca->last_soc_full = 0;
		} else if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
			&& (calculated_ocv > temp_ocv)) {
			info->soca->status = NXE2000_SOCA_FULL;
			g_full_flag = 0;
			info->soca->last_soc_full = 0;
		} else if (info->soca->Ibat_ave >= -20) {
			/* for issue1 solution end */
			/* check Full state or not*/
			if ((calculated_ocv > get_OCV_voltage(info, NXE2000_ENTER_FULL_STATE_OCV))
				|| (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
				|| (info->soca->displayed_soc > NXE2000_ENTER_FULL_STATE_DSOC * 100))
			{
				info->soca->status = NXE2000_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_soc_full = 0;
			} else if ((calculated_ocv > get_OCV_voltage(info, 9))
				&& (info->soca->Ibat_ave < 300))
			{
				info->soca->status = NXE2000_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_soc_full = 0;
			}
		} else { /* dis-charging */
			if (info->soca->displayed_soc/100 < NXE2000_ENTER_LOW_VOL) {
				info->soca->target_use_cap = 0;
				info->soca->status = NXE2000_SOCA_LOW_VOL;
			}
		}
	}

	if (NXE2000_SOCA_STABLE == info->soca->status) {
		info->soca->soc = calc_capacity_2(info);
		info->soca->soc_delta = info->soca->soc - info->soca->last_soc;

		if (info->soca->soc_delta >= -100 && info->soca->soc_delta <= 100) {
			info->soca->displayed_soc = info->soca->soc;
		} else {
			info->soca->status = NXE2000_SOCA_DISP;
		}
		info->soca->last_soc = info->soca->soc;
		info->soca->soc_delta = 0;
	} else if (NXE2000_SOCA_FULL == info->soca->status) {
		err = check_jeita_status(info, &is_jeita_updated);
		if (err < 0) {
			dev_err(info->dev, "Error in updating JEITA %d\n", err);
			goto end_flow;
		}
		info->soca->soc = calc_capacity(info) * 100;
		info->soca->last_soc = calc_capacity_2(info);	/* for DISP */

		if (info->soca->Ibat_ave >= -20) { /* charging */
			if (0 == info->soca->jt_limit) {
				if (g_full_flag == 1) {

					if (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status) {
						if(info->soca->full_reset_count < NXE2000_UPDATE_COUNT_FULL_RESET) {
							info->soca->full_reset_count++;
						} else if (info->soca->full_reset_count < (NXE2000_UPDATE_COUNT_FULL_RESET + 1)) {
							err = reset_FG_process(info);
							if (err < 0)
								dev_err(info->dev, "Error in writing the control register\n");
							info->soca->full_reset_count++;
							info->soca->rsoc_ready_flag =1;
							goto end_flow;
						} else if(info->soca->full_reset_count < (NXE2000_UPDATE_COUNT_FULL_RESET + 2)) {
							info->soca->full_reset_count++;
							info->soca->fc_cap = 0;
							info->soca->soc_full = info->soca->soc;
						}
					} else {
						if(info->soca->fc_cap < -1 * 200) {
							g_full_flag = 0;
							info->soca->displayed_soc = 99 * 100;
						}
						info->soca->full_reset_count = 0;
					}

					info->soca->chg_cmp_times = 0;
					if (info->soca->rsoc_ready_flag ==1) {
						err = calc_capacity_in_period(info, &cc_cap, &is_charging, 1);
						if (err < 0)
							dev_err(info->dev, "Read cc_sum Error !!-----\n");

						fc_delta = (is_charging == true) ? cc_cap : -cc_cap;

						info->soca->fc_cap = info->soca->fc_cap + fc_delta;
					}

					if (g_full_flag == 1){
						info->soca->displayed_soc = 100*100;
					}
				} else {
					if ((calculated_ocv < get_OCV_voltage(info, (NXE2000_ENTER_FULL_STATE_OCV - 1)))
						&& (info->soca->displayed_soc < (NXE2000_ENTER_FULL_STATE_DSOC - 10) * 100))	/* fail safe*/
					{
						g_full_flag = 0;
						info->soca->status = NXE2000_SOCA_DISP;
						info->soca->soc_delta = 0;
						info->soca->full_reset_count = 0;
						info->soca->last_soc = info->soca->soc;
					} else if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
						&& (info->soca->displayed_soc >= 9890)){
						if(info->soca->chg_cmp_times > NXE2000_FULL_WAIT_TIME) {
							info->soca->displayed_soc = 100*100;
							g_full_flag = 1;
							info->soca->full_reset_count = 0;
							info->soca->soc_full = info->soca->soc;
							info->soca->fc_cap = 0;
							info->soca->last_soc_full = 0;
#ifdef ENABLE_OCV_TABLE_CALIB
							err = calib_ocvTable(info,calculated_ocv);
							if (err < 0)
								dev_err(info->dev, "Calibration OCV Error !!\n");
#endif
						} else {
							info->soca->chg_cmp_times++;
						}
					} else {
						fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
							0x7fff);

						if (info->soca->displayed_soc >= 9950) {
							if((info->soca->soc_full - info->soca->soc) < 200) {
								goto end_flow;
							}
						}
						info->soca->chg_cmp_times = 0;

						if (info->soca->rsoc_ready_flag == 1) {
							err = calc_capacity_in_period(info, &cc_cap, &is_charging, 1);
							if (err < 0)
								dev_err(info->dev, "Read cc_sum Error !!-----\n");
							info->soca->cc_delta
								 = (is_charging == true) ? cc_cap : -cc_cap;
						} else {
							if (info->soca->last_soc_full == 0) { /* initial setting of last cc sum */
								info->soca->last_soc_full = calc_capacity_2(info);
								info->soca->cc_delta = 0;
								PM_LOGOUT(KERN_INFO "PMU: %s 1st last_soc_full(%d), cc_delta=0\n",
									 __func__, info->soca->last_soc_full);
							} else {
								if (info->soca->init_pswr == 100) {
									info->soca->cc_delta = 13;
									info->soca->last_soc_full = calc_capacity_2(info);
									PM_LOGOUT(KERN_INFO "PMU: %s init_pswr=100, last_soc_full(%d), cc_delta=13\n",
										 __func__, info->soca->last_soc_full);
								} else {
									current_soc_full = calc_capacity_2(info);
									info->soca->cc_delta = current_soc_full - info->soca->last_soc_full;
									info->soca->cc_delta = MIN(13, info->soca->cc_delta);
									PM_LOGOUT(KERN_INFO "PMU: %s current_soc_full(%d), last_soc_full(%d), cc_delta(%d)\n",
										 __func__, current_soc_full, info->soca->last_soc_full, info->soca->cc_delta);
									info->soca->last_soc_full = current_soc_full;
								}
							}
						}

						if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
							|| (info->soca->Ibat_ave < info->ch_icchg*50 + 100) )
						{
							info->soca->displayed_soc += 25 * 3000 / fa_cap;
						}
						else
						{
							if (calculated_ocv < get_OCV_voltage(info, 10))
							{
								full_rate = 100 * (10000 - info->soca->displayed_soc) /
									((1000* ((get_OCV_voltage(info, 10)) - calculated_ocv)
									/(get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))));
							}
							else full_rate = 301;

							full_rate = MIN(300, MAX(40,full_rate));

							info->soca->displayed_soc
								 = info->soca->displayed_soc + info->soca->cc_delta* full_rate / 100;
						}

						info->soca->displayed_soc
							 = MIN(10000, info->soca->displayed_soc);
						info->soca->displayed_soc = MAX(0, info->soca->displayed_soc);

						if (info->soca->displayed_soc >= 9890) {
							info->soca->displayed_soc = 99 * 100;
						}
					}
				}
			} else {
				info->soca->full_reset_count = 0;
			}
		} else { /* discharging */
			if (info->soca->displayed_soc >= 9950) {
				if (info->soca->Ibat_ave <= -1 * NXE2000_REL1_SEL_VALUE) {
					if ((calculated_ocv < (get_OCV_voltage(info, 9) + (get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))*3/10))
						|| ((info->soca->soc_full - info->soca->soc) > 200)) {

						g_full_flag = 0;
						info->soca->full_reset_count = 0;
						info->soca->displayed_soc = 100 * 100;
						info->soca->status = NXE2000_SOCA_DISP;
						info->soca->last_soc = info->soca->soc;
						info->soca->soc_delta = 0;
					} else {
						info->soca->displayed_soc = 100 * 100;
					}
				} else { /* into relaxation state */
					nxe2000_read(info->dev->parent, CHGSTATE_REG, &reg_val);
					if (reg_val & 0xc0) {
						info->soca->displayed_soc = 100 * 100;
					} else {
						g_full_flag = 0;
						info->soca->full_reset_count = 0;
						info->soca->displayed_soc = 100 * 100;
						info->soca->status = NXE2000_SOCA_DISP;
						info->soca->last_soc = info->soca->soc;
						info->soca->soc_delta = 0;
					}
				}
			} else {
				g_full_flag = 0;
				info->soca->status = NXE2000_SOCA_DISP;
				info->soca->soc_delta = 0;
				info->soca->full_reset_count = 0;
				info->soca->last_soc = info->soca->soc;
			}
		}
	} else if (NXE2000_SOCA_LOW_VOL == info->soca->status) {
		if(info->soca->Ibat_ave >= 0) {
			info->soca->soc = calc_capacity(info) * 100;
			info->soca->status = NXE2000_SOCA_DISP;
			info->soca->last_soc = info->soca->soc;
			info->soca->soc_delta = 0;
			info->soca->hurry_up_flg = 0;
		} else {
			fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
								0x7fff);

			if (info->soca->rsoc_ready_flag == 0) {
				temp_soc = calc_capacity_2(info);
				re_cap = fa_cap * temp_soc / (100 * 100);
				PM_LOGOUT("PMU FG_RESET : %s : facap is %d re_cap is %d soc is %d\n",__func__,fa_cap,re_cap,temp_soc);
			} else {
				re_cap = get_check_fuel_gauge_reg(info, RE_CAP_H_REG, RE_CAP_L_REG,
								0x7fff);
			}

			use_cap = fa_cap - re_cap;

			if (info->soca->target_use_cap == 0) {
				info->soca->re_cap_old = re_cap;
				get_target_use_cap(info);
			}

			if(use_cap >= info->soca->target_use_cap) {
				info->soca->displayed_soc = info->soca->displayed_soc - 100;
				info->soca->displayed_soc = MAX(0, info->soca->displayed_soc);
				info->soca->re_cap_old = re_cap;
			} else if (info->soca->hurry_up_flg == 1) {
				info->soca->displayed_soc = info->soca->displayed_soc - 100;
				info->soca->displayed_soc = MAX(0, info->soca->displayed_soc);
				info->soca->re_cap_old = re_cap;
			}
			get_target_use_cap(info);
			info->soca->soc = calc_capacity(info) * 100;
		}
	}

	if (NXE2000_SOCA_DISP == info->soca->status) {

		info->soca->soc = calc_capacity_2(info);

		soc_round = (info->soca->soc + 50) / 100;
		last_soc_round = (info->soca->last_soc + 50) / 100;
		last_disp_round = (info->soca->displayed_soc + 50) / 100;

		info->soca->soc_delta =
			info->soca->soc_delta + (info->soca->soc - info->soca->last_soc);

		info->soca->last_soc = info->soca->soc;
		/* six case */
		if (last_disp_round == soc_round) {
			/* if SOC == DISPLAY move to stable */
			info->soca->displayed_soc = info->soca->soc ;
			info->soca->status = NXE2000_SOCA_STABLE;
			delay_flag = 1;
		} else if (info->soca->Ibat_ave > 0) {
			if ((0 == info->soca->jt_limit) ||
			(POWER_SUPPLY_STATUS_FULL != info->soca->chg_status)) {
				/* Charge */
				if (last_disp_round < soc_round) {
					/* Case 1 : Charge, Display < SOC */
					if (info->soca->soc_delta >= 100) {
						info->soca->displayed_soc
							= last_disp_round * 100 + 50;
	 					info->soca->soc_delta -= 100;
						if (info->soca->soc_delta >= 100)
		 					delay_flag = 1;
					} else {
						info->soca->displayed_soc += 25;
						disp_dec = info->soca->displayed_soc % 100;
						if ((50 <= disp_dec) && (disp_dec <= 74))
							info->soca->soc_delta = 0;
					}
					if ((info->soca->displayed_soc + 50)/100
								 >= soc_round) {
						info->soca->displayed_soc
							= info->soca->soc ;
						info->soca->status
							= NXE2000_SOCA_STABLE;
						delay_flag = 1;
					}
				} else if (last_disp_round > soc_round) {
					/* Case 2 : Charge, Display > SOC */
					if (info->soca->soc_delta >= 300) {
						info->soca->displayed_soc += 100;
						info->soca->soc_delta -= 300;
					}
					if ((info->soca->displayed_soc + 50)/100
								 <= soc_round) {
						info->soca->displayed_soc
							= info->soca->soc ;
						info->soca->status
						= NXE2000_SOCA_STABLE;
						delay_flag = 1;
					}
				}
			} else {
				info->soca->soc_delta = 0;
			}
		} else {
			/* Dis-Charge */
			if (last_disp_round > soc_round) {
				/* Case 3 : Dis-Charge, Display > SOC */
				if (info->soca->soc_delta <= -100) {
					info->soca->displayed_soc
						= last_disp_round * 100 - 75;
					info->soca->soc_delta += 100;
					if (info->soca->soc_delta <= -100)
						delay_flag = 1;
				} else {
					info->soca->displayed_soc -= 25;
					disp_dec = info->soca->displayed_soc % 100;
					if ((25 <= disp_dec) && (disp_dec <= 49))
						info->soca->soc_delta = 0;
				}
				if ((info->soca->displayed_soc + 50)/100
							 <= soc_round) {
					info->soca->displayed_soc
						= info->soca->soc ;
					info->soca->status
						= NXE2000_SOCA_STABLE;
					delay_flag = 1;
				}
			} else if (last_disp_round < soc_round) {
				/* Case 4 : Dis-Charge, Display < SOC */
				if (info->soca->soc_delta <= -300) {
					info->soca->displayed_soc -= 100;
					info->soca->soc_delta += 300;
				}
				if ((info->soca->displayed_soc + 50)/100
							 >= soc_round) {
					info->soca->displayed_soc
						= info->soca->soc ;
					info->soca->status
						= NXE2000_SOCA_STABLE;
					delay_flag = 1;
				}
			}
		}
	} else if (NXE2000_SOCA_UNSTABLE == info->soca->status) {
		/* caluc 95% ocv */
		temp_ocv = get_OCV_voltage(info, 10) -
					(get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))/2;

		if(g_full_flag == 1){	/* for issue 1 solution start*/
			info->soca->status = NXE2000_SOCA_FULL;
			info->soca->last_soc_full = 0;
			err = reset_FG_process(info);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");

			goto end_flow;
		}else if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
			&& (calculated_ocv > temp_ocv)) {
			info->soca->status = NXE2000_SOCA_FULL;
			g_full_flag = 0;
			info->soca->last_soc_full = 0;
			err = reset_FG_process(info);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
			goto end_flow;
		} else if (info->soca->Ibat_ave >= -20) {
			/* for issue1 solution end */
			/* check Full state or not*/
			if ((calculated_ocv > (get_OCV_voltage(info, 9) + (get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))*7/10))
				|| (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
				|| (info->soca->displayed_soc > 9850))
			{
				info->soca->status = NXE2000_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_soc_full = 0;
				err = reset_FG_process(info);
				if (err < 0)
					dev_err(info->dev, "Error in writing the control register\n");
				goto end_flow;
			} else if ((calculated_ocv > (get_OCV_voltage(info, 9)))
				&& (info->soca->Ibat_ave < 300))
			{
				info->soca->status = NXE2000_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_soc_full = 0;
				err = reset_FG_process(info);
				if (err < 0)
					dev_err(info->dev, "Error in writing the control register\n");
				goto end_flow;
			}
		}

		info->soca->soc = info->soca->init_pswr * 100;

		err = calc_capacity_in_period(info, &cc_cap,
						 &is_charging, 0);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

		info->soca->cc_delta
			 = (is_charging == true) ? cc_cap : -cc_cap;

		displayed_soc_temp
			 = info->soca->soc + info->soca->cc_delta;
		if (displayed_soc_temp < 0)
			displayed_soc_temp = 0;
		displayed_soc_temp
			 = MIN(9850, displayed_soc_temp);
		displayed_soc_temp = MAX(0, displayed_soc_temp);

		info->soca->displayed_soc = displayed_soc_temp;

	} else if (NXE2000_SOCA_FG_RESET == info->soca->status) {
		/* No update */
	} else if (NXE2000_SOCA_START == info->soca->status) {

		err = measure_Ibatt_FG(info, &Ibat);
		err = measure_vbatt_FG(info, &Vbat);
		err = measure_vsys_ADC(info, &Vsys);

		PM_LOGOUT("PMU : %s : Ibat(%d mA), Vbat(%d uV), Vsys(%d uV) **********\n",
			__func__, Ibat, Vbat, Vsys);

		info->soca->Ibat_ave = Ibat;
		info->soca->Vbat_ave = Vbat;
		info->soca->Vsys_ave = Vsys;

		err = check_jeita_status(info, &is_jeita_updated);
		is_jeita_updated = false;
		if (err < 0) {
			dev_err(info->dev, "Error in updating JEITA %d\n", err);
		}
		err = nxe2000_read(info->dev->parent, PSWR_REG, &val);
		val &= 0x7f;
		if (info->first_pwon) {
			info->soca->soc = calc_first_soc(info) * 100;

			val = (info->soca->soc + 50)/100;
			val &= 0x7f;
			err = nxe2000_write(info->dev->parent, PSWR_REG, val);
			if (err < 0)
				dev_err(info->dev, "Error in writing PSWR_REG\n");
			info->soca->init_pswr = val;
			g_soc = val;

			PM_LOGOUT(KERN_INFO "PMU: %s soc(%d), init_pswr(%d)\n",
				 __func__, info->soca->soc, info->soca->init_pswr);

			if ((info->soca->soc == 0) && (calculated_ocv
					< get_OCV_voltage(info, 0))) {
				info->soca->displayed_soc = 0;
				info->soca->status = NXE2000_SOCA_ZERO;
			} else {
				if (0 == info->soca->jt_limit) {
					check_charge_status_2(info, info->soca->soc);
				} else {
					info->soca->displayed_soc = info->soca->soc;
				}
				if (Ibat < 0) {
					if (info->soca->displayed_soc < 300) {
						info->soca->target_use_cap = 0;
						info->soca->status = NXE2000_SOCA_LOW_VOL;
					} else {
						if ((info->fg_poff_vbat != 0)
						      && (Vbat < info->fg_poff_vbat * 1000) ){
							  info->soca->target_use_cap = 0;
							  info->soca->status = NXE2000_SOCA_LOW_VOL;
						  } else {
							  info->soca->status = NXE2000_SOCA_UNSTABLE;
						  }
					}
				} else {
					info->soca->status = NXE2000_SOCA_UNSTABLE;
				}
			}
		} else if (g_fg_on_mode && (val == 0x7f)) {
			info->soca->soc = calc_capacity(info) * 100;
			if ((info->soca->soc == 0) && (calculated_ocv
					< get_OCV_voltage(info, 0))) {
				info->soca->displayed_soc = 0;
				info->soca->status = NXE2000_SOCA_ZERO;
			} else {
				if (0 == info->soca->jt_limit) {
					check_charge_status_2(info, info->soca->soc);
				} else {
					info->soca->displayed_soc = info->soca->soc;
				}
				info->soca->last_soc = info->soca->soc;
				info->soca->status = NXE2000_SOCA_STABLE;
			}
		} else {
			info->soca->soc = val * 100;
			if (err < 0) {
				dev_err(info->dev,
					 "Error in reading PSWR_REG %d\n", err);
				info->soca->soc
					 = calc_capacity(info) * 100;
			}

			err = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 2);
			if (err < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");

			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			if (calculated_ocv < get_OCV_voltage(info, 0)) {
				info->soca->displayed_soc = 0;
				info->soca->status = NXE2000_SOCA_ZERO;
			} else {
				cc_poff_term = (info->soca->cc_delta/100)*100;

				soc_now = calc_soc_on_ocv(info, calculated_ocv);

				if(cc_poff_term < 0){ //When discharging
					cc_delta_offset = 100;	//unit is 1/100times
				} else {
					if( (soc_now - cc_poff_term) >= 100*100 ){//remove condition divide by Zero
						cc_delta_offset = 5 * 100; //MAX 5 times
					} else {
						cc_delta_offset = (100 * 100 - info->soca->soc) * 100 / (100 * 100 -  (soc_now - cc_poff_term));

						cc_delta_offset = MIN(5 * 100, cc_delta_offset); //MAX is 5 times
						cc_delta_offset = MAX(100 / 5, cc_delta_offset); //MIN is 1/5 times
					}
				}

				/*unit of cc_delta_offset is 0.01 times*/
				displayed_soc_temp = info->soca->soc + cc_poff_term * cc_delta_offset / 100;

				PM_LOGOUT(KERN_INFO "PMU: %s : cc_poff_term(%d)[0.01%%], cc_delta rate(%d)\n",
					 __func__, cc_poff_term, cc_delta_offset);
				PM_LOGOUT(KERN_INFO "PMU: %s : dsoc temp(%d)[0.01%%], soc(%d)[0.01%%], soc_now(%d)[0.01%%]\n",
					 __func__, displayed_soc_temp, info->soca->soc, soc_now);
				PM_LOGOUT(KERN_INFO "PMU: %s : status(%d), rsoc_ready_flag(%d)\n",
					 __func__, info->soca->status, info->soca->rsoc_ready_flag);

				cc_correct_value = cc_poff_term * cc_delta_offset / 100 - cc_poff_term;

				/* Apply cc_correct to PSWR */
				err = nxe2000_read(info->dev->parent, PSWR_REG, &val);
				if (err < 0)
					dev_err(info->dev, "Error in reading the pswr register\n");
				val &= 0x7f;

				PM_LOGOUT(KERN_INFO "PMU: %s : before PSWR %d per\n", __func__, val);

				val += (info->soca->cc_delta/100);
				val += cc_correct_value / 100;

				val = MIN(99, val);
				val = MAX(1, val);

				val &= 0x7f;

				err = nxe2000_write(info->dev->parent, PSWR_REG, val);
				if (err < 0)
					dev_err(info->dev, "Error in writing PSWR_REG\n");
				info->soca->init_pswr = val;
				g_soc = val;

				displayed_soc_temp = displayed_soc_temp - ( cc_correct_value - (cc_correct_value / 100 * 100));

				PM_LOGOUT(KERN_INFO "PMU: %s : dsoc_temp(%d)[0.01%%], cc offset(%d)[0.01%%]\n",
					 __func__, displayed_soc_temp, cc_correct_value);
				PM_LOGOUT(KERN_INFO "PMU: %s : after PSWR %d per\n", __func__, val);

				if (displayed_soc_temp <= 100) {
					displayed_soc_temp = 100;
					val = 1;
					err = nxe2000_write(info->dev->parent, PSWR_REG, val);
					if (err < 0)
						dev_err(info->dev, "Error in writing PSWR_REG\n");
					info->soca->init_pswr = val;
					g_soc = val;
					err = calc_capacity_in_period(info, &cc_cap,
								 &is_charging, 1);
				} else if (displayed_soc_temp >= 10000) {
					displayed_soc_temp = 10000;
					val = 100;
					err = nxe2000_write(info->dev->parent, PSWR_REG, val);
					if (err < 0)
						dev_err(info->dev, "Error in writing PSWR_REG\n");
					info->soca->init_pswr = val;
					g_soc = val;
					err = calc_capacity_in_period(info, &cc_cap,
								 &is_charging, 1);
				}

				if (0 == info->soca->jt_limit) {
					check_charge_status_2(info, displayed_soc_temp);
				} else {
					info->soca->displayed_soc = displayed_soc_temp;
				}
				info->soca->last_soc = calc_capacity_2(info);

				if(info->soca->rsoc_ready_flag == 0) {
					info->soca->status = NXE2000_SOCA_DISP;
					info->soca->soc_delta = 0;
					PM_LOGOUT("PMU FG_RESET : %s : initial  dsoc  is  %d\n",__func__,info->soca->displayed_soc);
				} else if  (Ibat < 0) {
					if (info->soca->displayed_soc < 300) {
						info->soca->target_use_cap = 0;
						info->soca->status = NXE2000_SOCA_LOW_VOL;
					} else {
						if ((info->fg_poff_vbat != 0)
						      && (Vbat < info->fg_poff_vbat * 1000)){
							  info->soca->target_use_cap = 0;
							  info->soca->status = NXE2000_SOCA_LOW_VOL;
						  } else {
							  info->soca->status = NXE2000_SOCA_UNSTABLE;
						  }
					}
				} else {
					info->soca->status = NXE2000_SOCA_UNSTABLE;
				}
			}
		}
	} else if (NXE2000_SOCA_ZERO == info->soca->status) {
		if (calculated_ocv > get_OCV_voltage(info, 0)) {
			err = calc_capacity_in_period(info, &cc_cap,
								 &is_charging, 1);
			if (err < 0)
				{dev_err(info->dev, "Read cc_sum Error !!-----\n");}
			info->soca->rsoc_ready_flag =0;
			info->soca->init_pswr = 1;
			val = 1;
			val &= 0x7f;
			g_soc = 1;
			err = nxe2000_write(info->dev->parent, PSWR_REG, val);
			if (err < 0)
				{dev_err(info->dev, "Error in writing PSWR_REG\n");}
			info->soca->last_soc = 100;
			info->soca->status = NXE2000_SOCA_STABLE;
		}

		info->soca->displayed_soc = 0;
	}
end_flow:
	/* keep DSOC = 1 when Vbat is over 3.4V*/
	if( info->fg_poff_vbat != 0) {
 		if (info->soca->zero_flg == 1) {
			if ((info->soca->Ibat_ave >= 0)
			|| (info->soca->Vbat_ave >= (info->fg_poff_vbat+100)*1000)) {
 				info->soca->zero_flg = 0;
			} else {
				info->soca->displayed_soc = 0;
 			}
		} else if (info->soca->displayed_soc < 50) {
			if (info->soca->Vbat_ave < 2000*1000) { /* error value */
				info->soca->displayed_soc = 100;
			} else if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
				info->soca->displayed_soc = 0;
				info->soca->zero_flg = 1;
			} else {
				info->soca->displayed_soc = 100;
			}
		}
	}

	if (g_fg_on_mode
		 && (info->soca->status == NXE2000_SOCA_STABLE)) {
		err = nxe2000_write(info->dev->parent, PSWR_REG, 0x7f);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");
		g_soc = 0x7F;
		err = calc_capacity_in_period(info, &cc_cap,
							&is_charging, 1);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

	} else if ((NXE2000_SOCA_UNSTABLE != info->soca->status)
			&& (info->soca->rsoc_ready_flag != 0)) {

		if ((info->soca->displayed_soc + 50)/100 <= 1) {
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}

		err = nxe2000_write(info->dev->parent, PSWR_REG, val);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;

		err = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 1);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");
		PM_LOGOUT(KERN_INFO "PMU: %s Full-Clear CC, PSWR(%d)\n",
			 __func__, val);
	} else {	/* Case of UNSTABLE STATE */
		if ((info->soca->displayed_soc + 50)/100 <= 1) {
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}
		err = nxe2000_write(info->dev->parent, PSWR_REG, val);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;

		err = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 2);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");
		info->soca->cc_delta
			 = (is_charging == true) ? cc_cap : -cc_cap;

		val = info->soca->init_pswr + (info->soca->cc_delta/100);
		val = MIN(100, val);
		val = MAX(1, val);

		info->soca->init_pswr = val;

		PM_LOGOUT(KERN_INFO "PMU: %s Half-Clear CC, init_pswr(%d), cc_delta(%d)\n",
			 __func__, info->soca->init_pswr, info->soca->cc_delta);

	}

	PM_LOGOUT("PMU:STATUS= %d: IBAT= %5d: VSYS= %7d: VBAT= %7d: DSOC= %5d: RSOC= %5d: rsoc_ready= %d: full_rate= %d\n",
		info->soca->status, info->soca->Ibat_ave, info->soca->Vsys_ave, info->soca->Vbat_ave,
		info->soca->displayed_soc, info->soca->soc, info->soca->rsoc_ready_flag, full_rate);

	PM_LOGOUT( "## Rsys:%d, target_ibat:%d, target_vsys:%d, cutoff_ocv:%d, fg_poff_vbat:%d \n",
						info->soca->Rsys, info->soca->target_ibat, info->soca->target_vsys, info->soca->cutoff_ocv, info->fg_poff_vbat);

#ifdef DISABLE_CHARGER_TIMER
	/* clear charger timer */
	if ( info->soca->chg_status == POWER_SUPPLY_STATUS_CHARGING ) {
		err = nxe2000_read(info->dev->parent, TIMSET_REG, &val);
		if (err < 0)
			dev_err(info->dev,
			"Error in read TIMSET_REG%d\n", err);
		/* to check bit 0-1 */
		val2 = val & 0x03;

		if (val2 == 0x02){
			/* set rapid timer 240 -> 300 */
			err = nxe2000_set_bits(info->dev->parent, TIMSET_REG, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
			}
		} else {
			/* set rapid timer 300 -> 240 */
			err = nxe2000_clr_bits(info->dev->parent, TIMSET_REG, 0x01);
			err = nxe2000_set_bits(info->dev->parent, TIMSET_REG, 0x02);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
			}
		}
	}
#endif

	if (0 == info->soca->ready_fg)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 NXE2000_FG_RESET_TIME * HZ);
	else if (delay_flag == 1)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 NXE2000_DELAY_TIME * HZ);
	else if (NXE2000_SOCA_DISP == info->soca->status)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 NXE2000_DISPLAY_UPDATE_TIME * HZ);
	else if ((info->soca->hurry_up_flg == 1) && (NXE2000_SOCA_LOW_VOL == info->soca->status))
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 NXE2000_LOW_VOL_DOWN_TIME * HZ);
	else
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 NXE2000_DISPLAY_UPDATE_TIME * HZ);

	mutex_unlock(&info->lock);

	if((true == is_jeita_updated)
	|| (info->soca->last_displayed_soc/100 != (info->soca->displayed_soc+50)/100))
		power_supply_changed(&info->battery);

	info->soca->last_displayed_soc = info->soca->displayed_soc+50;

	return;
}

static void nxe2000_stable_charge_countdown_work(struct work_struct *work)
{
	int ret;
	int max = 0;
	int min = 100;
	int i;
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, charge_stable_work.work);

	if (info->entry_factory_mode)
		return;

	mutex_lock(&info->lock);
	if (NXE2000_SOCA_FG_RESET == info->soca->status)
		info->soca->ready_fg = 1;

	if (2 <= info->soca->stable_count) {
		if (3 == info->soca->stable_count
			&& NXE2000_SOCA_FG_RESET == info->soca->status) {
			ret = reset_FG_process(info);
			if (ret < 0)
				dev_err(info->dev, "Error in writing the control register\n");
		}
		info->soca->stable_count = info->soca->stable_count - 1;
		queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 NXE2000_FG_STABLE_TIME * HZ / 10);
	} else if (0 >= info->soca->stable_count) {
		/* Finished queue, ignore */
	} else if (1 == info->soca->stable_count) {
		if (NXE2000_SOCA_UNSTABLE == info->soca->status) {
			/* Judge if FG need reset or Not */
			info->soca->soc = calc_capacity(info) * 100;
			if (info->chg_ctr != 0) {
				queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 NXE2000_FG_STABLE_TIME * HZ / 10);
				mutex_unlock(&info->lock);
				return;
			}
			/* Do reset setting */
			ret = reset_FG_process(info);
			if (ret < 0)
				dev_err(info->dev, "Error in writing the control register\n");

			info->soca->status = NXE2000_SOCA_FG_RESET;

			/* Delay for addition Reset Time (6s) */
			queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 NXE2000_FG_RESET_TIME*HZ);
		} else if (NXE2000_SOCA_FG_RESET == info->soca->status) {
			info->soca->reset_soc[2] = info->soca->reset_soc[1];
			info->soca->reset_soc[1] = info->soca->reset_soc[0];
			info->soca->reset_soc[0] = calc_capacity(info) * 100;
			info->soca->reset_count++;

			if (info->soca->reset_count > 10) {
				/* Reset finished; */
				info->soca->soc = info->soca->reset_soc[0];
				info->soca->stable_count = 0;
				goto adjust;
			}

			for (i = 0; i < 3; i++) {
				if (max < info->soca->reset_soc[i]/100)
					max = info->soca->reset_soc[i]/100;
				if (min > info->soca->reset_soc[i]/100)
					min = info->soca->reset_soc[i]/100;
			}

			if ((info->soca->reset_count > 3) && ((max - min)
					< NXE2000_MAX_RESET_SOC_DIFF)) {
				/* Reset finished; */
				info->soca->soc = info->soca->reset_soc[0];
				info->soca->stable_count = 0;
				goto adjust;
			} else {
				/* Do reset setting */
				ret = reset_FG_process(info);
				if (ret < 0)
					dev_err(info->dev, "Error in writing the control register\n");

				/* Delay for addition Reset Time (6s) */
				queue_delayed_work(info->monitor_wqueue,
						 &info->charge_stable_work,
						 NXE2000_FG_RESET_TIME*HZ);
			}
		/* Finished queue From now, select FG as result; */
		} else if (NXE2000_SOCA_START == info->soca->status) {
			/* Normal condition */
		} else { /* other state ZERO/DISP/STABLE */
			info->soca->stable_count = 0;
		}

		mutex_unlock(&info->lock);
		return;

adjust:
		info->soca->last_soc = info->soca->soc;
		info->soca->status = NXE2000_SOCA_DISP;
		info->soca->soc_delta = 0;

	}
	mutex_unlock(&info->lock);
	return;
}

static void nxe2000_charge_monitor_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, charge_monitor_work.work);

	get_power_supply_status(info);

	if (POWER_SUPPLY_STATUS_DISCHARGING == info->soca->chg_status
		|| POWER_SUPPLY_STATUS_NOT_CHARGING == info->soca->chg_status) {
		switch (info->soca->dischg_state) {
		case	0:
			info->soca->dischg_state = 1;
			break;
		case	1:
			info->soca->dischg_state = 2;
			break;

		case	2:
		default:
			break;
		}
	} else {
		info->soca->dischg_state = 0;
	}

	queue_delayed_work(info->monitor_wqueue, &info->charge_monitor_work,
					 NXE2000_CHARGE_MONITOR_TIME * HZ);

	return;
}

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
int nxe2000_decide_charge_byResource(struct nxe2000_battery_info *info)
{
	uint8_t ctrlreg;
	int ret;

	ret = nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &ctrlreg);
	if(isCheck_ChargeStop_byResource() == 0)
	{
		if(!(ctrlreg&0x03))
		{
			pr_debug("....  resume the changing\n");
			nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
		}
	}
	else // Charging Stop
	{
		if(ctrlreg&0x03)
		{
			pr_debug(".... stop the charging: ctrlreg(0x%x)\n", ctrlreg);
			nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
		}

		queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 NXE2000_CHARGE_UPDATE_TIME * HZ);
		return 0;
	}

	return 1;
}
#endif

static void nxe2000_get_charge_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, get_charge_work.work);

	int Vbat_temp, Vsys_temp, Ibat_temp;
	int Vbat_sort[NXE2000_GET_CHARGE_NUM];
	int Vsys_sort[NXE2000_GET_CHARGE_NUM];
	int Ibat_sort[NXE2000_GET_CHARGE_NUM];
	int i, j;
	int ret;
	int capacity = 0;

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	nxe2000_decide_charge_byResource(info);
#endif

	mutex_lock(&info->lock);

	if(!info->factory_mode_complete) {
		queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
			 NXE2000_CHARGE_CALC_TIME * HZ);
		mutex_unlock(&info->lock);
		return;
	}

	for (i = NXE2000_GET_CHARGE_NUM-1; i > 0; i--) {
		if (0 == info->soca->chg_count) {
			info->soca->Vbat[i] = 0;
			info->soca->Vsys[i] = 0;
			info->soca->Ibat[i] = 0;
			info->soca->Ocv[i]  = 0;
		} else {
			info->soca->Vbat[i] = info->soca->Vbat[i-1];
			info->soca->Vsys[i] = info->soca->Vsys[i-1];
			info->soca->Ibat[i] = info->soca->Ibat[i-1];
		}
	}

	ret = measure_vbatt_FG(info, &info->soca->Vbat[0]);
	ret = measure_vsys_ADC(info, &info->soca->Vsys[0]);
	ret = measure_Ibatt_FG(info, &info->soca->Ibat[0]);

	if ((info->first_soc == 1) || (NXE2000_SOCA_START != info->soca->status)) {
		info->soca->Ocv[info->soca->chg_count]
			 = info->soca->Vbat[0] - info->soca->Ibat[0] * info->soca->Rbat;
	}

	if(info->first_soc){
		if (info->soca->ocv_table[0] >= info->soca->Ocv[info->soca->chg_count]) {
			capacity = 1;
		} else if (info->soca->ocv_table[10] <= info->soca->Ocv[info->soca->chg_count]) {
			capacity = 100;
		} else {
			for (i = 1; i < 11; i++) {
				if (info->soca->ocv_table[i] >= info->soca->Ocv[info->soca->chg_count]) {
					/* unit is 1% */
					capacity = Calc_Linear_Interpolation(
						(i-1)*10, info->soca->ocv_table[i-1], i*10,
						 info->soca->ocv_table[i], info->soca->Ocv[info->soca->chg_count]);
					break;
				}
			}
		}

		capacity = MAX(1, capacity);
		capacity = MIN(100, capacity);

		info->soca->displayed_soc = capacity * 100;

		PM_LOGOUT("PMU : %s : Check Parameter ********** Ibat(%d mA), Vbat(%d uV), Ocv(%d, %d uV) **********\n",
			__func__, info->soca->Ibat[0], info->soca->Vbat[0], info->soca->chg_count, info->soca->Ocv[info->soca->chg_count] );
	}

	info->soca->chg_count++;

	if (NXE2000_GET_CHARGE_NUM != info->soca->chg_count) {
		queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 NXE2000_CHARGE_CALC_TIME * HZ);
		mutex_unlock(&info->lock);
		return ;
	}

	if(info->first_soc) {
		info->first_soc = 0;
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work, 0);
		ret = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x08);
		if (ret < 0)
			dev_err(info->dev, "Error in writing the control register\n");
	}

	for (i = 0; i < NXE2000_GET_CHARGE_NUM; i++) {
		Vbat_sort[i] = info->soca->Vbat[i];
		Vsys_sort[i] = info->soca->Vsys[i];
		Ibat_sort[i] = info->soca->Ibat[i];
	}

	Vbat_temp = 0;
	Vsys_temp = 0;
	Ibat_temp = 0;
	for (i = 0; i < NXE2000_GET_CHARGE_NUM - 1; i++) {
		for (j = NXE2000_GET_CHARGE_NUM - 1; j > i; j--) {
			if (Vbat_sort[j - 1] > Vbat_sort[j]) {
				Vbat_temp = Vbat_sort[j];
				Vbat_sort[j] = Vbat_sort[j - 1];
				Vbat_sort[j - 1] = Vbat_temp;
			}
			if (Vsys_sort[j - 1] > Vsys_sort[j]) {
				Vsys_temp = Vsys_sort[j];
				Vsys_sort[j] = Vsys_sort[j - 1];
				Vsys_sort[j - 1] = Vsys_temp;
			}
			if (Ibat_sort[j - 1] > Ibat_sort[j]) {
				Ibat_temp = Ibat_sort[j];
				Ibat_sort[j] = Ibat_sort[j - 1];
				Ibat_sort[j - 1] = Ibat_temp;
			}
		}
	}

	Vbat_temp = 0;
	Vsys_temp = 0;
	Ibat_temp = 0;
	for (i = 3; i < NXE2000_GET_CHARGE_NUM-3; i++) {
		Vbat_temp = Vbat_temp + Vbat_sort[i];
		Vsys_temp = Vsys_temp + Vsys_sort[i];
		Ibat_temp = Ibat_temp + Ibat_sort[i];
	}
	Vbat_temp = Vbat_temp / (NXE2000_GET_CHARGE_NUM - 6);
	Vsys_temp = Vsys_temp / (NXE2000_GET_CHARGE_NUM - 6);
	Ibat_temp = Ibat_temp / (NXE2000_GET_CHARGE_NUM - 6);

	if (0 == info->soca->chg_count) {
		queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
				 NXE2000_CHARGE_UPDATE_TIME * HZ);
		mutex_unlock(&info->lock);
		return;
	} else {
		info->soca->Vbat_ave = Vbat_temp;
		info->soca->Vsys_ave = Vsys_temp;
		info->soca->Ibat_ave = Ibat_temp;
	}

	info->soca->chg_count = 0;
	queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
				 NXE2000_CHARGE_UPDATE_TIME * HZ);
	mutex_unlock(&info->lock);
	return;
}

/* Initial setting of FuelGauge SOCA function */
static int nxe2000_init_fgsoca(struct nxe2000_battery_info *info)
{
	int i;
	int err;
	uint8_t val;
	int cc_cap = 0;
	bool is_charging = true;

	for (i = 0; i <= 10; i = i+1) {
		info->soca->ocv_table[i] = get_OCV_voltage(info, i);
		PM_LOGOUT(KERN_INFO "PMU: %s : * %3d%%  voltage = %d uV (0x%x)\n",
				 __func__, i*10, info->soca->ocv_table[i], get_OCV_init_Data(info, i));
	}

	for (i = 0; i < 3; i = i+1)
		info->soca->reset_soc[i] = 0;
	info->soca->reset_count = 0;

	if (info->first_pwon) {

		err = nxe2000_read(info->dev->parent, CHGISET_REG, &val);
		if (err < 0)
			dev_err(info->dev,
			"Error in read CHGISET_REG%d\n", err);

		err = nxe2000_write(info->dev->parent, CHGISET_REG, 0);
		if (err < 0)
			dev_err(info->dev,
			"Error in writing CHGISET_REG%d\n", err);
		/* msleep(1000); */

		err = calc_capacity_in_period(info, &cc_cap, &is_charging, 1);

		/* msleep(6000); */

		err = nxe2000_write(info->dev->parent, CHGISET_REG, val);
		if (err < 0)
			dev_err(info->dev,
			"Error in writing CHGISET_REG%d\n", err);
	}

	/* Rbat : Transfer */
	info->soca->Rbat = get_OCV_init_Data(info, 12) * 1000 / 512
							 * 5000 / 4095;
	info->soca->n_cap = get_OCV_init_Data(info, 11);


	info->soca->displayed_soc = 0;
	info->soca->last_displayed_soc = 0;
	info->soca->suspend_soc = 0;
	info->soca->ready_fg = 1;
	info->soca->soc_delta = 0;
	info->soca->full_reset_count = 0;
	info->soca->soc_full = 0;
	info->soca->fc_cap = 0;
	info->soca->status = NXE2000_SOCA_START;
	/* stable count down 11->2, 1: reset; 0: Finished; */
	info->soca->stable_count = 11;
	info->soca->chg_cmp_times = 0;
	info->soca->dischg_state = 0;
	info->soca->Vbat_ave = 0;
	info->soca->Vbat_old = 0;
	info->soca->Vsys_ave = 0;
	info->soca->Ibat_ave = 0;
	info->soca->chg_count = 0;
	info->soca->target_use_cap = 0;
	info->soca->hurry_up_flg = 0;
	info->soca->re_cap_old = 0;
	info->soca->jt_limit = 0;
	info->soca->zero_flg = 0;
	info->soca->cc_cap_offset = 0;
	info->soca->last_soc_full = 0;

	for (i = 0; i < 11; i++) {
		info->soca->ocv_table_low[i] = 0;
	}

	for (i = 0; i < NXE2000_GET_CHARGE_NUM; i++) {
		info->soca->Vbat[i] = 0;
		info->soca->Vsys[i] = 0;
		info->soca->Ibat[i] = 0;
	}

	g_full_flag = 0;

#ifdef ENABLE_FG_KEEP_ON_MODE
	g_fg_on_mode = 1;
#else
	g_fg_on_mode = 0;
#endif

	/* Start first Display job */
	if (info->first_soc == 0) {
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 NXE2000_FIRST_DISPLAY_TIME*HZ);
	}

	/* Start first Waiting stable job */
	queue_delayed_work(info->monitor_wqueue, &info->charge_stable_work,
					 NXE2000_FG_STABLE_TIME*HZ/10);

	queue_delayed_work(info->monitor_wqueue, &info->charge_monitor_work,
					 NXE2000_CHARGE_MONITOR_TIME * HZ);

	queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 NXE2000_CHARGE_CALC_TIME * HZ);

	if (info->jt_en) {
		if (info->jt_hw_sw) {
			/* Enable JEITA function supported by H/W */
			err = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x04);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
		} else {
		 	/* Disable JEITA function supported by H/W */
			err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x04);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
			queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
						 	 NXE2000_FG_RESET_TIME * HZ);
		}
	} else {
		/* Disable JEITA function supported by H/W */
		err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x04);
		if (err < 0)
			dev_err(info->dev, "Error in writing the control register\n");
	}

	PM_LOGOUT(KERN_INFO "PMU: %s : * Rbat = %d mOhm   n_cap = %d mAH (0x%04x)\n",
			 __func__, info->soca->Rbat, info->soca->n_cap, get_OCV_init_Data(info, 11));
	return 1;
}
#endif

static void nxe2000_changed_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, changed_work.work);

	power_supply_changed(&info->battery);

	return;
}

#if (CFG_SW_UBC_ENABLE == 1)
static void nxe2000_sw_ubc_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, sw_ubc_work.work);
	uint8_t val = 0;
	uint8_t extif_gchgdet = 0;
	int pmic_vbus;
	int temp, ret;

	//power_supply_changed(&info->battery);

#if (CFG_USB_DET_FROM_PMIC_INT == 1)
	pmic_vbus = 1;
#else
	pmic_vbus = gpio_get_value(info->gpio_pmic_vbus);
#endif

	info->is_sdp_type = dwc_otg_pcd_get_ep0_state();

	if(!info->is_sdp_type)
	{
		nxe2000_read(info->dev->parent, NXE2000_REG_EXTIF_GCHGDET, &extif_gchgdet);

		if(extif_gchgdet == 0x08)
		{
			info->is_sdp_type = 1;
		}

	}

	ret = nxe2000_read(info->dev->parent, NXE2000_REG_REGISET2, &val);
	if (ret < 0) {
		dev_err(info->dev, "Can't read NXE2000_REG_REGISET2 register. : %d\n", ret);
		goto recheck_ubc;
	}

#if 0
	if (val != info->ch_ilim_dcp)
		info->flag_set_ilimit = false;

	if (pmic_vbus && !info->flag_set_ilimit && !info->is_sdp_type) {
		ret = nxe2000_write(info->dev->parent, NXE2000_REG_REGISET2, info->ch_ilim_dcp);
		if (ret < 0) {
			dev_err(info->dev, "Can't write NXE2000_REG_REGISET2 register. : %d\n", ret);
		}

		info->flag_set_ilimit = true;
	}
#else

	if (info->is_sdp_type)
		temp = (val & 0xE0) | info->ch_ilim_usb;
	else
		temp = (val & 0xE0) | info->ch_ilim_dcp;

	if (val != temp)
		info->flag_set_ilimit = false;

	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] is_sdp_type:0x%x, pmic_vbus:0x%x, flag_set_ilimit:0x%x, temp:0x%x, extif_gchgdet:0x%x \n",
						__func__, __LINE__, info->is_sdp_type, pmic_vbus, info->flag_set_ilimit, temp, extif_gchgdet);

	if (pmic_vbus && !info->flag_set_ilimit) {
		ret = nxe2000_write(info->dev->parent, NXE2000_REG_REGISET2, temp);
		if (ret < 0)
			ret = nxe2000_write(info->dev->parent, NXE2000_REG_REGISET2, temp);

		if (ret < 0) {
			dev_err(info->dev, "Can't write NXE2000_REG_REGISET2 register. : %d\n", ret);
		}

		info->flag_set_ilimit = true;
	}
#endif

recheck_ubc:
	if (info->pmic_vbuschk_count) {
		info->pmic_vbuschk_count--;
		queue_delayed_work(info->monitor_wqueue,
					&info->sw_ubc_work, msecs_to_jiffies(1000));
	}
	else
	{
		power_supply_changed(&info->battery);
	}
	return;
}
#endif

#ifdef KOOK_UBC_CHECK
static void nxe2000_get_charger_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, get_charger_work.work);
	uint8_t val = 0;
	uint8_t val2 = 0;
	uint8_t val3 = 0;
	uint8_t pc_vbus_det = 0;
	int msecs_for_recheck = 200;
	int ret;

	mutex_lock(&info->lock);

	if (info->chg_extif == 0) {
		ret = nxe2000_read(info->dev->parent, NXE2000_INT_IR_CHGEXTIF, &val);
		if (ret < 0) {        /* Check GC_DET */
			dev_err(info->dev, "Can't read NXE2000_INT_IR_CHGEXTIF register. : %d\n", ret);
			goto set_recheck;
		}

		info->chg_extif = val & 0x03;
	}

	if (info->chg_extif & 0x01) {	/* for GCDET */
		info->chg_extif &= ~0x01;

		ret = nxe2000_read(info->dev->parent, EXTIF_GCHGDET_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}

		val2 = (val & 0x0c) >> 2;
		if (val2 != 2) {		/* Check GC_DET */
			dev_err(info->dev, "GC_DET is not completed\n");
			goto set_recheck;
		}

		val2 = (val & 0x80) >> 7;
		if (val2 == 0) {		/* Check DCD_TIMEOUT */
			dev_info(info->dev, "GC Detection is completed\n");
//			goto set_recheck;
		}

		/* Check the port type */
		val2 = (val & 0x40) >> 6;
		val3 = (val & 0x30) >> 4;
		if (val2 == 1)
			info->extif_type = EXTIF_TYPE_IRP;
		else if (val3 == 0)
			info->extif_type = EXTIF_TYPE_SDP;
		else if (val3 == 1)
			info->extif_type = EXTIF_TYPE_CDP;
		else if (val3 == 2)
			info->extif_type = EXTIF_TYPE_DCP;
		else
			info->extif_type = EXTIF_TYPE_OTHERS;

		/* Dummy Read & Write to four registers */
		ret = nxe2000_read(info->dev->parent, EXTIF_FD_EN_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}
		ret = nxe2000_write(info->dev->parent, EXTIF_FD_EN_REG, val);
		if (ret < 0) {
			dev_err(info->dev, "Error in writing the extif register\n");
			goto set_recheck;
		}

		ret = nxe2000_read(info->dev->parent, EXTIF_FD_VREF_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}
		ret = nxe2000_write(info->dev->parent, EXTIF_FD_VREF_REG, val);
		if (ret < 0) {
			dev_err(info->dev, "Error in writing the extif register\n");
			goto set_recheck;
		}

		ret = nxe2000_read(info->dev->parent, EXTIF_SD_EN_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}
		ret = nxe2000_write(info->dev->parent, EXTIF_SD_EN_REG, val);
		if (ret < 0) {
			dev_err(info->dev, "Error in writing the extif register\n");
			goto set_recheck;
		}

		ret = nxe2000_read(info->dev->parent, EXTIF_SD_VREF_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}
		ret = nxe2000_write(info->dev->parent, EXTIF_SD_VREF_REG, val);
		if (ret < 0) {
			dev_err(info->dev, "Error in writing the extif register\n");
			goto set_recheck;
		}

		/* Set the ILIM option */
		ret = nxe2000_read(info->dev->parent, REGISET2_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}
		val &= 0x1f;
		val |= 0xa0;	/* [7:5]=101b */
		ret = nxe2000_write(info->dev->parent, REGISET2_REG, val);
		if (ret < 0) {
			dev_err(info->dev, "Error in writing the extif register\n");
			goto set_recheck;
		}

#if 0
		if ( (info->extif_type == EXTIF_TYPE_IRP) || (info->extif_type == EXTIF_TYPE_OTHERS) )
		{
			/* Start PC Detection */
			ret = nxe2000_set_bits(info->dev->parent,
							 EXTIF_PCHGDET_REG, 0x01);
			if (ret < 0) {
				dev_err(info->dev, "Error in writing the extif register\n");
//				goto set_recheck;
			}

			goto set_recheck;
		}
#else

		if ( (info->extif_type == EXTIF_TYPE_IRP) || (info->extif_type == EXTIF_TYPE_OTHERS) )
		{
			goto set_recheck;
		}
#endif

	} else if (info->chg_extif & 0x02) {	/* for PCDET */
		info->chg_extif &= ~0x02;

		nxe2000_read(info->dev->parent, EXTIF_PCHGDET_REG, &val);
		val2 = (val & 0x0c) >> 2;
		if (val2 != 2) {		/* Check PC_DET */
			dev_err(info->dev, "PC_DET is not completed\n");
			goto set_recheck;
		}

		/* Check PC_VBUS_DET */
		pc_vbus_det = (val & 0xf0) >> 4;

		ret = nxe2000_read(info->dev->parent, REGISET2_REG, &val);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the extif register\n");
			goto set_recheck;
		}
		val2 = val;
		val &= 0xe0;

		/* Selecet PC */
		if ((pc_vbus_det == 0x8)
				 && (info->extif_type == EXTIF_TYPE_SDP)) {
			val |= 0x04;
		} else if ((pc_vbus_det == 0x9)
				 && (info->extif_type == EXTIF_TYPE_CDP)) {
			val |= 0x09;
		} else if ((pc_vbus_det == 0xa)
				 && (info->extif_type == EXTIF_TYPE_SDP)) {
			val |= 0x0e;
		} else {
			/* unknown device */
			val = val2;
		}

		ret = nxe2000_write(info->dev->parent, REGISET2_REG, val);
		if (ret < 0) {
			dev_err(info->dev, "Error in writing the extif register\n");
			goto set_recheck;
		}
	}

//out:
	power_supply_changed(&info->battery);

	msleep(840);

	nxe2000_set_bits(info->dev->parent, 0x91, 0x10);	// GPIO4 : High(Hi-Z)

#if defined(CONFIG_USB_DWCOTG)
	otg_phy_init();
	otg_clk_enable();
#endif

	info->chg_extif = 0;
	info->chg_work_recheck_count = 0;

	mutex_unlock(&info->lock);

	return;

set_recheck:

	if (info->chg_work_recheck_count < 3)
	{
		msecs_for_recheck = 200;

		info->chg_work_recheck_count++;
	}
	else
	{
		msecs_for_recheck = 900;

		nxe2000_set_bits(info->dev->parent, 0x91, 0x10);	// GPIO4 : High(Hi-Z)
#if defined(CONFIG_USB_DWCOTG)
		otg_phy_init();
		otg_clk_enable();

		otg_clk_disable();
		otg_phy_suspend();
#endif
		nxe2000_clr_bits(info->dev->parent, 0x91, 0x10);	// GPIO4 : Low

		info->chg_work_recheck_count = 0;
	}

	/* GCHGDET:(0xDA) setting */
	val = 0x01;
	ret = nxe2000_write(info->dev->parent, EXTIF_GCHGDET_REG, val);
	if (ret < 0) {
		dev_err(info->dev,
			"Error in writing EXTIF_GCHGDET_REG %d\n", val);
	}

	mutex_unlock(&info->lock);

	queue_delayed_work(info->monitor_wqueue,
				&info->get_charger_work, msecs_to_jiffies(msecs_for_recheck));

	return;
}
#endif	// #ifdef KOOK_UBC_CHECK

static int check_jeita_status(struct nxe2000_battery_info *info, bool *is_jeita_updated)
/*  JEITA Parameter settings
*
*          VCHG
*            |
* jt_vfchg_h~+~~~~~~~~~~~~~~~~~~~+
*            |                   |
* jt_vfchg_l-| - - - - - - - - - +~~~~~~~~~~+
*            |    Charge area    +          |
*  -------0--+-------------------+----------+--- Temp
*            !                   +
*          ICHG
*            |                   +
*  jt_ichg_h-+ - -+~~~~~~~~~~~~~~+~~~~~~~~~~+
*            +    |              +          |
*  jt_ichg_l-+~~~~+   Charge area           |
*            |    +              +          |
*         0--+----+--------------+----------+--- Temp
*            0   jt_temp_l      jt_temp_h   55
*/
{
	int temp;
	int err = 0;
	int vfchg;
	uint8_t chgiset_org;
	uint8_t batset2_org;
	uint8_t set_vchg_h, set_vchg_l;
	uint8_t set_ichg_h, set_ichg_l;
	uint8_t charge_backup;

	*is_jeita_updated = false;
	/* No execute if JEITA disabled */
	if (!info->jt_en || info->jt_hw_sw)
		return 0;

	/* Check FG Reset */
	if (info->soca->ready_fg) {
		temp = get_battery_temp_2(info) / 10;
	} else {
		goto out;
	}

	/* Read BATSET2 */
	err = nxe2000_read(info->dev->parent, BATSET2_REG, &batset2_org);
	if (err < 0) {
		dev_err(info->dev, "Error in readng the battery setting register\n");
		goto out;
	}
	vfchg = (batset2_org & 0x70) >> 4;
	batset2_org &= 0x8F;

	/* Read CHGISET */
	err = nxe2000_read(info->dev->parent, CHGISET_REG, &chgiset_org);
	if (err < 0) {
		dev_err(info->dev, "Error in readng the chrage setting register\n");
		goto out;
	}
	chgiset_org &= 0xC0;

	set_ichg_h = (uint8_t)(chgiset_org | info->jt_ichg_h);
	set_ichg_l = (uint8_t)(chgiset_org | info->jt_ichg_l);

	set_vchg_h = (uint8_t)((info->jt_vfchg_h << 4) | batset2_org);
	set_vchg_l = (uint8_t)((info->jt_vfchg_l << 4) | batset2_org);

	if (temp <= 0 || 55 <= temp) {
		/* 1st and 5th temperature ranges (~0, 55~) */
		nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &charge_backup);
		err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
		info->soca->jt_limit = 0;
		*is_jeita_updated = true;
	} else if (temp < info->jt_temp_l) {
		/* 2nd temperature range (0~12) */
		if (vfchg != info->jt_vfchg_h) {
			nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &charge_backup);
			err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
				goto out;
			}

			/* set VFCHG/VRCHG */
			err = nxe2000_write(info->dev->parent,
							 BATSET2_REG, set_vchg_h);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the battery setting register\n");
				goto out;
			}
			info->soca->jt_limit = 0;
			*is_jeita_updated = true;
		} else
		/* set ICHG */
		err = nxe2000_write(info->dev->parent, CHGISET_REG, set_ichg_l);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the battery setting register\n");
			goto out;
		}
		err = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, charge_backup);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
	} else if (temp < info->jt_temp_h) {
		/* 3rd temperature range (12~50) */
		if (vfchg != info->jt_vfchg_h) {
			nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &charge_backup);
			err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
				goto out;
			}
			/* set VFCHG/VRCHG */
			err = nxe2000_write(info->dev->parent,
							 BATSET2_REG, set_vchg_h);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the battery setting register\n");
				goto out;
			}
			info->soca->jt_limit = 0;
			*is_jeita_updated = true;
		}

		/* set ICHG */
		err = nxe2000_write(info->dev->parent, CHGISET_REG, set_ichg_h);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the battery setting register\n");
			goto out;
		}
		err = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, charge_backup);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
	} else if (temp < 55) {
		/* 4th temperature range (50~55) */
		if (vfchg != info->jt_vfchg_l) {
			nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &charge_backup);
			err = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
				goto out;
			}
			/* set VFCHG/VRCHG */
			err = nxe2000_write(info->dev->parent,
							 BATSET2_REG, set_vchg_l);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the battery setting register\n");
				goto out;
			}
			info->soca->jt_limit = 1;
			*is_jeita_updated = true;
		} else
		/* set ICHG */
		err = nxe2000_write(info->dev->parent, CHGISET_REG, set_ichg_h);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the battery setting register\n");
			goto out;
		}
		err = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, charge_backup);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
	}

	get_power_supply_status(info);

	return 0;

out:
	return err;
}

static void nxe2000_jeita_work(struct work_struct *work)
{
	int ret;
	bool is_jeita_updated = false;
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, jeita_work.work);

	mutex_lock(&info->lock);

	ret = check_jeita_status(info, &is_jeita_updated);
	if (0 == ret) {
		queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
					 NXE2000_JEITA_UPDATE_TIME * HZ);
	} else {
		queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
					 NXE2000_FG_RESET_TIME * HZ);
	}

	mutex_unlock(&info->lock);

	if(true == is_jeita_updated)
		power_supply_changed(&info->battery);

	return;
}

#ifdef ENABLE_FACTORY_MODE
/*------------------------------------------------------*/
/* Factory Mode						*/
/*    Check Battery exist or not			*/
/*    If not, disabled Rapid to Complete State change	*/
/*------------------------------------------------------*/
static int nxe2000_factory_mode(struct nxe2000_battery_info *info)
{
	int ret = 0;
	uint8_t val = 0;

	ret = nxe2000_read(info->dev->parent, NXE2000_INT_MON_CHGCTR, &val);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return ret;
	}

	if (!(val & 0x01)) { /* No Adapter connected */
		info->factory_mode_complete = true;

		if(info->first_soc) {
			suspend_charge4first_soc(info);
		}

		return ret;
	}

	/* Rapid to Complete State change disable */
	ret = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x40);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return ret;
	}

	/* Wait 1s for checking Charging State */
	queue_delayed_work(info->factory_mode_wqueue, &info->factory_mode_work,
			 1*HZ);

	return ret;
}

static void check_charging_state_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		struct nxe2000_battery_info, factory_mode_work.work);

	int ret = 0;
	uint8_t val = 0;
	int chargeCurrent = 0;

	ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &val);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return;
	}


	chargeCurrent = get_check_fuel_gauge_reg(info, CC_AVERAGE1_REG,
						 CC_AVERAGE0_REG, 0x3fff);
	if (chargeCurrent < 0) {
		dev_err(info->dev, "Error in reading the FG register\n");
		return;
	}

	/* Repid State && Charge Current about 0mA */
	if (((chargeCurrent >= 0x3ffc && chargeCurrent <= 0x3fff)
		|| chargeCurrent < 0x05) && val == 0x43) {
		info->entry_factory_mode = true;
		/* clear FG_ACC bit */
		ret = nxe2000_clr_bits(info->dev->parent, NXE2000_FG_CTRL, 0x10);
		if (ret < 0)
			dev_err(info->dev, "Error in writing FG_CTRL\n");
		info->factory_mode_complete = true;

		return;	/* Factory Mode */
	}

	/* Return Normal Mode --> Rapid to Complete State change enable */
	/* disable the status change from Rapid Charge to Charge Complete */
	ret = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x40);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return;
	}

	info->factory_mode_complete = true;

	if(info->first_soc){
		suspend_charge4first_soc(info);
	}

	return;
}
#endif /* ENABLE_FACTORY_MODE */

static int Calc_Linear_Interpolation(int x0, int y0, int x1, int y1, int y)
{
	int	alpha;
	int x;

	alpha = (y - y0)*100 / (y1 - y0);

	x = ((100 - alpha) * x0 + alpha * x1) / 100;

	return x;
}

static void nxe2000_scaling_OCV_table(struct nxe2000_battery_info *info, int cutoff_vol, int full_vol, int *start_per, int *end_per)
{
	int		i, j;
	int		temp;
	int		percent_step;
	int		OCV_percent_new[11];

	/* Check Start % */
	if (info->soca->ocv_table_def[0] > cutoff_vol * 1000) {
		*start_per = 0;
		PM_LOGOUT("PMU: %s : setting value of cuttoff_vol(%d) is out of range(%d) \n",__func__, cutoff_vol, info->soca->ocv_table_def[0]);
	} else {
		for (i = 1; i < 11; i++) {
			if (info->soca->ocv_table_def[i] >= cutoff_vol * 1000) {
				/* unit is 0.001% */
				*start_per = Calc_Linear_Interpolation(
					(i-1)*1000, info->soca->ocv_table_def[i-1], i*1000,
					info->soca->ocv_table_def[i], (cutoff_vol * 1000));
				break;
			}
		}
	}

	/* Check End % */
	for (i = 1; i < 11; i++) {
		if (info->soca->ocv_table_def[i] >= full_vol * 1000) {
			/* unit is 0.001% */
			*end_per = Calc_Linear_Interpolation(
				(i-1)*1000, info->soca->ocv_table_def[i-1], i*1000,
				 info->soca->ocv_table_def[i], (full_vol * 1000));
			break;
		}
	}

	/* calc new ocv percent */
	percent_step = ( *end_per - *start_per) / 10;

	for (i = 0; i < 11; i++) {
		OCV_percent_new[i]
			 = *start_per + percent_step*(i - 0);
	}

	/* calc new ocv voltage */
	for (i = 0; i < 11; i++) {
		for (j = 1; j < 11; j++) {
			if (1000*j >= OCV_percent_new[i]) {
				temp = Calc_Linear_Interpolation(
					info->soca->ocv_table_def[j-1], (j-1)*1000,
					info->soca->ocv_table_def[j] , j*1000,
					 OCV_percent_new[i]);

				temp = ( (temp/1000) * 4095 ) / 5000;

				battery_init_para[info->num][i*2 + 1] = temp;
				battery_init_para[info->num][i*2] = temp >> 8;

				break;
			}
		}
	}
	PM_LOGOUT("PMU: %s : new table\n",__func__);
	for (i = 0; i <= 10; i = i+1) {
		temp = (battery_init_para[info->num][i*2]<<8)
			 | (battery_init_para[info->num][i*2+1]);
		/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
		temp = ((temp * 50000 * 10 / 4095) + 5);
		PM_LOGOUT("PMU: %s : ocv_table %3d%% vol = %d uV (0x%x)\n",__func__, i*10, temp, get_OCV_init_Data(info, i));
	}

}

static int nxe2000_set_OCV_table(struct nxe2000_battery_info *info)
{
	int		ret = 0;
	int		i;
	int		full_ocv;
	int		available_cap;
	int		available_cap_ori;
	int		temp;
	int		temp1;
	int		start_per = 0;
	int		end_per = 0;
	int		Rbat;
	int		Ibat_min;
	uint8_t val;
	uint8_t val2;
	uint8_t val_temp;


	//get ocv table
	for (i = 0; i <= 10; i = i+1) {
		info->soca->ocv_table_def[i] = get_OCV_voltage(info, i);
		PM_LOGOUT(KERN_INFO "PMU: %s : %3d%% vol = %d uV (0x%x)\n",
			 __func__, i*10, info->soca->ocv_table_def[i], get_OCV_init_Data(info, i));
	}

	temp =  (battery_init_para[info->num][24]<<8) | (battery_init_para[info->num][25]);
	Rbat = temp * 1000 / 512 * 5000 / 4095;
	info->soca->Rsys = Rbat + 55;

	if ((info->fg_target_ibat == 0) || (info->fg_target_vsys == 0)) {	/* normal version */

		temp =  (battery_init_para[info->num][22]<<8) | (battery_init_para[info->num][23]);
		//fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
		//				0x7fff);

		info->soca->target_ibat = temp*2/10; /* calc 0.2C*/
		temp1 =  (battery_init_para[info->num][0]<<8) | (battery_init_para[info->num][1]);
//		temp = get_OCV_voltage(info, 0) / 1000; /* unit is 1mv*/
//		info->soca->cutoff_ocv = info->soca->target_vsys - Ibat_min * info->soca->Rsys / 1000;

		info->soca->target_vsys = temp1 + ( info->soca->target_ibat * info->soca->Rsys ) / 1000;


	} else {
		info->soca->target_ibat = info->fg_target_ibat;
		/* calc min vsys value */
		temp1 =  (battery_init_para[info->num][0]<<8) | (battery_init_para[info->num][1]);
		temp = temp1 + ( info->soca->target_ibat * info->soca->Rsys ) / 1000;
		if( temp < info->fg_target_vsys) {
			info->soca->target_vsys = info->fg_target_vsys;
		} else {
			info->soca->target_vsys = temp;
		}
	}

	if ((info->soca->target_ibat == 0) || (info->soca->target_vsys == 0)) {	/* normal version */
	} else {	/*Slice cutoff voltage version. */

		Ibat_min = -1 * info->soca->target_ibat;
		info->soca->cutoff_ocv = info->soca->target_vsys - Ibat_min * info->soca->Rsys / 1000;

		full_ocv = (battery_init_para[info->num][20]<<8) | (battery_init_para[info->num][21]);
		full_ocv = full_ocv * 5000 / 4095;

		nxe2000_scaling_OCV_table(info, info->soca->cutoff_ocv, full_ocv, &start_per, &end_per);

		/* calc available capacity */
		/* get avilable capacity */
		/* battery_init_para23-24 is designe capacity */
		available_cap = (battery_init_para[info->num][22]<<8)
					 | (battery_init_para[info->num][23]);

		available_cap = available_cap
			 * ((10000 - start_per) / 100) / 100 ;


		battery_init_para[info->num][23] =  available_cap;
		battery_init_para[info->num][22] =  available_cap >> 8;

	}

	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] Rsys:%d, target_ibat:%d, target_vsys:%d, cutoff_ocv:%d, fg_poff_vbat:%d \n", __func__, __LINE__,
						info->soca->Rsys, info->soca->target_ibat, info->soca->target_vsys, info->soca->cutoff_ocv, info->fg_poff_vbat);

	ret = nxe2000_clr_bits(info->dev->parent, FG_CTRL_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev, "error in FG_En off\n");
		goto err;
	}
	/////////////////////////////////
	ret = nxe2000_read_bank1(info->dev->parent, 0xDC, &val);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	val_temp = val;
	val	&= 0x0F; //clear bit 4-7
	val	|= 0x10; //set bit 4

	ret = nxe2000_write_bank1(info->dev->parent, 0xDC, val);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	ret = nxe2000_read_bank1(info->dev->parent, 0xDC, &val2);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	ret = nxe2000_write_bank1(info->dev->parent, 0xDC, val_temp);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	if (val != val2) {
		ret = nxe2000_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 30, battery_init_para[info->num]);
		if (ret < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			goto err;
		}
	} else {
		ret = nxe2000_read_bank1(info->dev->parent, 0xD2, &val);
		if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
		}

		ret = nxe2000_read_bank1(info->dev->parent, 0xD3, &val2);
		if (ret < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			goto err;
		}

		available_cap_ori = val2 + (val << 8);
		available_cap = battery_init_para[info->num][23]
						+ (battery_init_para[info->num][22] << 8);

		if (available_cap_ori == available_cap) {
			ret = nxe2000_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 22, battery_init_para[info->num]);
			if (ret < 0) {
				dev_err(info->dev, "batterry initialize error\n");
				return ret;
			}

			for (i = 0; i < 6; i++) {
				ret = nxe2000_write_bank1(info->dev->parent, 0xD4+i, battery_init_para[info->num][24+i]);
				if (ret < 0) {
					dev_err(info->dev, "batterry initialize error\n");
					return ret;
				}
			}
		} else {
			ret = nxe2000_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 30, battery_init_para[info->num]);
			if (ret < 0) {
				dev_err(info->dev, "batterry initialize error\n");
				goto err;
			}
		}
	}

	////////////////////////////////

	return 0;
err:
	return ret;
}

/* Initial setting of battery */
static int nxe2000_init_battery(struct nxe2000_battery_info *info)
{
	int ret = 0;
	uint8_t val;
	uint8_t val2;
	/* Need to implement initial setting of batery and error */
	/* -------------------------- */
#ifdef ENABLE_FUEL_GAUGE_FUNCTION

	/* set relaxation state */
	if (NXE2000_REL1_SEL_VALUE > 240)
		val = 0x0F;
	else
		val = NXE2000_REL1_SEL_VALUE / 16 ;

	/* set relaxation state */
	if (NXE2000_REL2_SEL_VALUE > 120)
		val2 = 0x0F;
	else
		val2 = NXE2000_REL2_SEL_VALUE / 8 ;

	val =  val + (val2 << 4);

	ret = nxe2000_write_bank1(info->dev->parent, BAT_REL_SEL_REG, val);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing BAT_REL_SEL_REG\n");
		return ret;
	}

	ret = nxe2000_read_bank1(info->dev->parent, BAT_REL_SEL_REG, &val);

	ret = nxe2000_write_bank1(info->dev->parent, BAT_TA_SEL_REG, 0x00);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing BAT_TA_SEL_REG\n");
		return ret;
	}

//	ret = nxe2000_read(info->dev->parent, FG_CTRL_REG, &val);
//	if (ret < 0) {
//		dev_err(info->dev, "Error in reading the control register\n");
//		return ret;
//	}

//	val = (val & 0x10) >> 4;
//	info->first_pwon = (val == 0) ? 1 : 0;
	ret = nxe2000_read(info->dev->parent, PSWR_REG, &val);
	if (ret < 0) {
		dev_err(info->dev,"Error in reading PSWR_REG %d\n", ret);
		return ret;
	}
	info->first_pwon = (val == 0) ? 1 : 0;

	info->first_soc = info->first_pwon;

	g_soc = val & 0x7f;

	info->soca->init_pswr = val & 0x7f;
	PM_LOGOUT("PMU FG_RESET : %s : initial pswr = %d\n",__func__,info->soca->init_pswr);

	info->soca->rsoc_ready_flag = 0;

	ret = nxe2000_set_OCV_table(info);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the OCV Tabler\n");
		return ret;
	}

	ret = nxe2000_write(info->dev->parent, FG_CTRL_REG, 0x11);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return ret;
	}

#endif

	ret = nxe2000_write(info->dev->parent, VINDAC_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return ret;
	}

	return ret;
}

/* Initial setting of charger */
static int nxe2000_init_charger(struct nxe2000_battery_info *info)
{
	int err = 0;
	uint8_t val;
	uint8_t val2;
	uint8_t val3;
	uint8_t low_det_bits = 0;
	uint8_t charge_backup;
	int charge_status;
	int	vfchg_val;
	int	icchg_val;
	int	rbat;
	int	temp;

	info->chg_ctr = 0;
	info->chg_stat1 = 0;
	info->chg_extif = 0;
	info->extif_type = 0;

	err = nxe2000_set_bits(info->dev->parent, NXE2000_PWR_FUNC, 0x20);
	if (err < 0) {
		dev_err(info->dev, "Error in writing the PWR FUNC register\n");
		goto free_device;
	}

	charge_status = get_power_supply_status(info);

	if (charge_status != POWER_SUPPLY_STATUS_FULL)
	{
		/* Disable charging */
		nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &charge_backup);
		err = nxe2000_clr_bits(info->dev->parent,NXE2000_REG_CHGCTL1, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto free_device;
		}
	}

	/* REGISET1:(0xB6) setting */
	if ((info->ch_ilim_adp != 0xFF) || (info->ch_ilim_adp <= 0x1D)) {
		val = info->ch_ilim_adp;

		err = nxe2000_write(info->dev->parent, REGISET1_REG, val);
		if (err < 0) {
			dev_err(info->dev,
				"Error in writing REGISET1_REG %d\n", err);
			goto free_device;
		}
	}

	/* REGISET2:(0xB7) setting */
	err = nxe2000_read(info->dev->parent, REGISET2_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 		"Error in read REGISET2_REG %d\n", err);
		goto free_device;
	}

#if 0
	if ((info->ch_ilim_usb != 0xFF) || (info->ch_ilim_usb <= 0x1D)) {
		val2 = info->ch_ilim_usb;
	} else {/* Keep OTP value */
		val2 = (val & 0x1F);
	}

	/* keep bit 5-7 */
	val &= 0xE0;

	val = val + val2;

	err = nxe2000_write(info->dev->parent, REGISET2_REG,val);
	if (err < 0) {
		dev_err(info->dev,
			"Error in writing REGISET2_REG %d\n", err);
		goto free_device;
	}
#endif

	/* CHGISET:(0xB8) setting */
	err = nxe2000_read(info->dev->parent, CHGISET_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read CHGISET_REG %d\n", err);
		goto free_device;
	}

	/* Define Current settings value for charging (bit 4~0)*/
	if ((info->ch_ichg != 0xFF) || (info->ch_ichg <= 0x1D)) {
		val2 = info->ch_ichg;
	} else { /* Keep OTP value */
		val2 = (val & 0x1F);
	}

	/* Define Current settings at the charge completion (bit 7~6)*/
	if ((info->ch_icchg != 0xFF) || (info->ch_icchg <= 0x03)) {
		val3 = (info->ch_icchg << NXE2000_POS_CHGISET_ICCHG);
	} else { /* Keep OTP value */
		val3 = (val & 0xC0);
	}
	val = val2 + val3;
	err = nxe2000_write(info->dev->parent, CHGISET_REG, val);
	if (err < 0) {
		dev_err(info->dev,
			"Error in writing CHGISET_REG %d\n", err);
		goto free_device;
	}

#ifdef ENABLE_DEBUG
	//debug messeage
	err = nxe2000_read(info->dev->parent, CHGISET_REG,&val);
	PM_LOGOUT("PMU : %s : after CHGISET_REG (0x%x) is 0x%x info->ch_ichg is 0x%x info->ch_icchg is 0x%x\n",__func__,CHGISET_REG,val,info->ch_ichg,info->ch_icchg);
	err = nxe2000_read(info->dev->parent, BATSET1_REG,&val);
	PM_LOGOUT("PMU : %s : before BATSET1_REG (0x%x) is 0x%x info->ch_vbatovset is 0x%x\n",__func__,BATSET1_REG,val,info->ch_vbatovset);
#endif

	/* BATSET1_REG(0xBA) setting */
	err = nxe2000_read(info->dev->parent, BATSET1_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read BATSET1 register %d\n", err);
		goto free_device;
	}

	/* Define Battery overvoltage  (bit 4)*/
	if ((info->ch_vbatovset != 0xFF) || (info->ch_vbatovset <= 0x1)) {
		val2 = info->ch_vbatovset;
		val2 = val2 << 4;
	} else { /* Keep OTP value */
		val2 = (val & 0x10);
	}

	/* keep bit 0-3 and bit 5-7 */
	val = (val & 0xEF);

	val = val + val2;

	err = nxe2000_write(info->dev->parent, BATSET1_REG, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing BAT1_REG %d\n",
									 err);
		goto free_device;
	}

#ifdef ENABLE_DEBUG
	//debug messeage
	err = nxe2000_read(info->dev->parent, BATSET1_REG,&val);
	PM_LOGOUT("PMU : %s : after BATSET1_REG (0x%x) is 0x%x info->ch_vbatovset is 0x%x\n",__func__,BATSET1_REG,val,info->ch_vbatovset);
	err = nxe2000_read(info->dev->parent, BATSET2_REG,&val);
	PM_LOGOUT("PMU : %s : before BATSET2_REG (0x%x) is 0x%x info->ch_vrchg is 0x%x info->ch_vfchg is 0x%x \n",__func__,BATSET2_REG,val,info->ch_vrchg,info->ch_vfchg);
#endif

	/* BATSET2_REG(0xBB) setting */
	err = nxe2000_read(info->dev->parent, BATSET2_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read BATSET2 register %d\n", err);
		goto free_device;
	}

		/* Define Re-charging voltage (bit 2~0)*/
	if ((info->ch_vrchg != 0xFF) || (info->ch_vrchg <= 0x04)) {
		val2 = info->ch_vrchg;
	} else { /* Keep OTP value */
		val2 = (val & 0x07);
	}

		/* Define FULL charging voltage (bit 6~4)*/
	if ((info->ch_vfchg != 0xFF) || (info->ch_vfchg <= 0x04)) {
		val3 = info->ch_vfchg;
		val3 = val3 << 4;
	} else {	/* Keep OTP value */
		val3 = (val & 0x70);
	}

		/* keep bit 3 and bit 7 */
	val = (val & 0x88);

	val = val + val2 + val3;

	err = nxe2000_write(info->dev->parent, BATSET2_REG, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing NXE2000_RE_CHARGE_VOLTAGE %d\n",
								 err);
		goto free_device;
	}

	/* Set rising edge setting ([1:0]=01b)for INT in charging */
	/*  and rising edge setting ([3:2]=01b)for charge completion */
	err = nxe2000_read(info->dev->parent, NXE2000_CHG_STAT_DETMOD1, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading CHG_STAT_DETMOD1 %d\n",
								 err);
		goto free_device;
	}
	val &= 0xf0;
	val |= 0x05;
	err = nxe2000_write(info->dev->parent, NXE2000_CHG_STAT_DETMOD1, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing CHG_STAT_DETMOD1 %d\n",
								 err);
		goto free_device;
	}

	/* Unmask In charging/charge completion */
	err = nxe2000_write(info->dev->parent, NXE2000_INT_MSK_CHGSTS1, 0xfc);
	if (err < 0) {
		dev_err(info->dev, "Error in writing INT_MSK_CHGSTS1 %d\n",
								 err);
		goto free_device;
	}

	/* Set both edge for VUSB([3:2]=11b)/VADP([1:0]=11b) detect */
	err = nxe2000_read(info->dev->parent, NXE2000_CHG_CTRL_DETMOD1, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading CHG_CTRL_DETMOD1 %d\n",
								 err);
		goto free_device;
	}
	val &= 0xf0;
	val |= 0x0f;
	err = nxe2000_write(info->dev->parent, NXE2000_CHG_CTRL_DETMOD1, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing CHG_CTRL_DETMOD1 %d\n",
								 err);
		goto free_device;
	}

	/* Unmask In VUSB/VADP completion */
	err = nxe2000_write(info->dev->parent, NXE2000_INT_MSK_CHGCTR, 0xfc);
	if (err < 0) {
		dev_err(info->dev, "Error in writing INT_MSK_CHGSTS1 %d\n",
								 err);
		goto free_device;
	}

	if (charge_status != POWER_SUPPLY_STATUS_FULL)
	{
		/* Enable charging */
		err = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, charge_backup);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto free_device;
		}
	}
	/* get OCV100_min, OCV100_min*/
	temp = (battery_init_para[info->num][24]<<8) | (battery_init_para[info->num][25]);
	rbat = temp * 1000 / 512 * 5000 / 4095;

	/* get vfchg value */
	err = nxe2000_read(info->dev->parent, BATSET2_REG, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading the batset2reg\n");
		goto free_device;
	}
	val &= 0x70;
	val2 = val >> 4;
	if (val2 <= 3) {
		vfchg_val = 4050 + val2 * 50;
	} else {
		vfchg_val = 4350;
	}

	/* get  value */
	err = nxe2000_read(info->dev->parent, CHGISET_REG, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading the chgisetreg\n");
		goto free_device;
	}
	val &= 0xC0;
	val2 = val >> 6;
	icchg_val = 50 + val2 * 50;

	info->soca->OCV100_min = ( vfchg_val * 99 / 100 - (icchg_val * (rbat +20))/1000 - 20 ) * 1000;
	info->soca->OCV100_max = ( vfchg_val * 101 / 100 - (icchg_val * (rbat +20))/1000 + 20 ) * 1000;

	PM_LOGOUT("PMU : %s : 100 min %d, 100 max %d vfchg %d icchg %d rbat %d\n",__func__,
		info->soca->OCV100_min,info->soca->OCV100_max,vfchg_val,icchg_val,rbat);

	/* Set ADRQ=00 to stop ADC */
	nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x00);

	/* Set ADC auto conversion interval 250ms */
	nxe2000_write(info->dev->parent, NXE2000_ADC_CNT2, 0x00);

	/*  VBAT threshold low voltage value = (voltage(V)*255)/(2*2.5) */
	info->low_vbat_thl = ((info->low_vbat_vol_mv * 255) / 5000)+1;
	nxe2000_write(info->dev->parent, NXE2000_ADC_VBAT_THL, 0x00);
	low_det_bits |= 0x02;

	/*  VSYS threshold low voltage value = (voltage(V)*255)/(3*2.5) */
	info->low_vsys_thl = ((info->low_vsys_vol_mv * 255) / 7500)+1;
	nxe2000_write(info->dev->parent, NXE2000_ADC_VSYS_THL, 0x00);
	low_det_bits |= 0x10;

	/* Enable VBAT/VSYS pin conversion in auto-ADC */
	nxe2000_write(info->dev->parent, NXE2000_ADC_CNT1, 0x12);

	/* Disable VBAT/VSYS threshold Low interrupt */
	//nxe2000_write(info->dev->parent, NXE2000_INT_EN_ADC1, low_det_bits);
	nxe2000_write(info->dev->parent, NXE2000_INT_EN_ADC1, 0x00);

 	/* Cleaning ADC interrupt */
	nxe2000_write(info->dev->parent, NXE2000_INT_IR_ADCL, 0);

	/* Start auto-mode & average 4-time conversion mode for ADC */
	// set from suspend
	nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x28);

free_device:
	return err;
}


static int set_otg_power_control(struct nxe2000_battery_info *info, int otg_id)
{
	int ret = 0;
	uint8_t set_val = 0;

	ret = nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &set_val);
	if (ret < 0)
		ret = nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &set_val);

	if(info->input_power_type == INPUT_POWER_TYPE_UBC)
	{
		if (otg_id)
		{
			/* OTG POWER OFF */
			if (info->gpio_otg_vbus > -1)
				gpio_set_value(info->gpio_otg_vbus, 0);

			set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

			ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
			if (ret < 0)
				ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
		}
		else
		{
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);

			ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
			if (ret < 0)
				ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

			/* OTG POWER ON */
			if (info->gpio_otg_vbus > -1)
				gpio_set_value(info->gpio_otg_vbus, 1);
		}
	}
	else if(info->input_power_type == INPUT_POWER_TYPE_ADP_UBC)
	{
		if (otg_id)
		{
			/* OTG POWER OFF */
			if (info->gpio_otg_vbus > -1)
				gpio_set_value(info->gpio_otg_vbus, 0);

			//set_val |= (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM);
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);

			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

			ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
			if (ret < 0)
				ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
		}
		else
		{
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);

			ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
			if (ret < 0)
				ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

			/* OTG POWER ON */
			if (info->gpio_otg_vbus > -1)
				gpio_set_value(info->gpio_otg_vbus, 1);
		}
	}
	else if(info->input_power_type == INPUT_POWER_TYPE_ADP_NOBATTERY)
	{
		if (otg_id)
		{
			/* OTG POWER OFF */
			if (info->gpio_otg_vbus > -1)
				gpio_set_value(info->gpio_otg_vbus, 0);

			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

			ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
			if (ret < 0)
				ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
		}
		else
		{
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);

			ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
			if (ret < 0)
				ret = nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

			/* OTG POWER ON */
			if (info->gpio_otg_vbus > -1)
				gpio_set_value(info->gpio_otg_vbus, 1);
		}
	}
	return ret;
}


static int get_power_supply_status(struct nxe2000_battery_info *info)
{
	uint8_t status;
	uint8_t supply_state;
	uint8_t charge_state;
	int ret = 0;

	/* get  power supply status */
	ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &status);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return ret;
	}

	info->present = 1;

	charge_state = (status & 0x1F);
	supply_state = ((status & 0xC0) >> 6);

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	pr_debug("chgState(%s) supply[%d](%s) battery(%d%%) VBAT(%d)\n",
		strChargingState[charge_state],supply_state, strSupply[supply_state],
		0, 0);
	if(charge_state == CHG_STATE_DIE_ERR)
	{
		uint8_t dieTempReg;
		extern int NXP_Get_BoardTemperature(void);
		ret = nxe2000_read(info->dev->parent, NXE2000_REG_CHGISET, &dieTempReg);// (info->dev->parent, NXE2000_REG_CHGISET, CHARGE_CURRENT_100MA);
		ret = nxe2000_read(info->dev->parent, 0xB3, &dieTempReg);
		printk("________die Error. dieTempReg:0x%x\n", dieTempReg);
		pmic_occur_dieError=1;
	}
	else
	{
		pmic_occur_dieError=0;
	}
#endif

	if (info->entry_factory_mode)
			return POWER_SUPPLY_STATUS_NOT_CHARGING;

	if (supply_state == SUPPLY_STATE_BAT) {
		info->soca->chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
	} else {
		switch (charge_state) {
		case	CHG_STATE_CHG_OFF:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_CHG_READY_VADP:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_CHG_TRICKLE:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_CHARGING;
				break;
		case	CHG_STATE_CHG_RAPID:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_CHARGING;
				break;
		case	CHG_STATE_CHG_COMPLETE:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_FULL;
				break;
		case	CHG_STATE_SUSPEND:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_VCHG_OVER_VOL:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_BAT_ERROR:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_NO_BAT:
				info->present = 0;
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_BAT_OVER_VOL:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_BAT_TEMP_ERR:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_DIE_ERR:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_DIE_SHUTDOWN:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_NO_BAT2:
				info->present = 0;
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_CHG_READY_VUSB:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		default:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_UNKNOWN;
				break;
		}
	}

	return info->soca->chg_status;
}

static int get_power_supply_Android_status(struct nxe2000_battery_info *info)
{
	int ret = POWER_SUPPLY_STATUS_UNKNOWN;

	get_power_supply_status(info);

	/* get  power supply status */
	if (info->entry_factory_mode)
		return POWER_SUPPLY_STATUS_NOT_CHARGING;

	switch (info->soca->chg_status) {
		case	POWER_SUPPLY_STATUS_UNKNOWN:
		case	POWER_SUPPLY_STATUS_NOT_CHARGING:
		case	POWER_SUPPLY_STATUS_DISCHARGING:
		case	POWER_SUPPLY_STATUS_CHARGING:
				ret = info->soca->chg_status;
				break;

		case	POWER_SUPPLY_STATUS_FULL:
				if(info->soca->displayed_soc == 100 * 100) {
					ret = POWER_SUPPLY_STATUS_FULL;
				} else {
					ret = POWER_SUPPLY_STATUS_CHARGING;
				}
				break;
		default:
				ret = POWER_SUPPLY_STATUS_UNKNOWN;
				break;
	}

	return ret;
}

static void charger_irq_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info
		 = container_of(work, struct nxe2000_battery_info, irq_work);
	int otg_id = 1;
	int pmic_vbus = 0;
	int ret = 0;
	uint8_t val = 0, set_val = 0;

	power_supply_changed(&info->battery);

	//if (info->gpio_otg_usbid > -1)
	//	otg_id = gpio_get_value(info->gpio_otg_usbid);
	otg_id = info->gpio_otg_usbid_status;

	if (info->gpio_pmic_vbus > -1)
		pmic_vbus	 = gpio_get_value(info->gpio_pmic_vbus);

	info->chg_ctr = 0;
	info->chg_stat1 = 0;

	/* Enable Interrupt for VADP/VUSB */
	ret = nxe2000_write(info->dev->parent, NXE2000_INT_MSK_CHGCTR, 0xfc);
	if (ret < 0)
		dev_err(info->dev, "%s(): Error in enable charger mask INT %d\n", __func__, ret);

	/* Enable Interrupt for Charging & complete */
	ret = nxe2000_write(info->dev->parent, NXE2000_INT_MSK_CHGSTS1, 0xfc);
	if (ret < 0)
		dev_err(info->dev, "%s(): Error in enable charger mask INT %d\n", __func__, ret);

	ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &val);
	if (ret < 0)
	{
		ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &val);
		if (ret < 0)
		{
			dev_err(info->dev, "Error in reading the control register\n");
			return;
		}
	}

	ret = nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &set_val);
	if (ret < 0)
		ret = nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &set_val);

	val = (val & 0xC0) >> 6;

	switch (val)
	{
		case SUPPLY_STATE_BAT: // plug out USB/ADP
			if (info->input_power_type == INPUT_POWER_TYPE_ADP_UBC)
			{
				if (otg_id == 0)
				{
					set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

					/* OTG POWER ON */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 1);

				}
				else
				{
					/* OTG POWER OFF */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 0);

					//set_val |= (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM);
					set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
					set_val |= (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);

					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
				}
			}
			break;

		case SUPPLY_STATE_ADP: // plug in ADP
			if (info->input_power_type == INPUT_POWER_TYPE_UBC)
			{
				info->ubc_check_count = 1;
			}
			else if (info->input_power_type == INPUT_POWER_TYPE_ADP)
			{
				//set_val = (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM);
				set_val |= (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);

				if (otg_id == 0)
				{
					set_val |=	(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

					/* OTG POWER ON */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 1);
				}
				else
				{
					/* OTG POWER OFF */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 0);

					//set_val &=	~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
				}

				info->ubc_check_count = 0;
			}
			else if (info->input_power_type == INPUT_POWER_TYPE_ADP_UBC)
			{
				//set_val |= (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM);
				set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
				set_val |= (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);
				set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);

				if (otg_id == 0)
				{
					set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

					/* OTG POWER ON */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 1);
				}
				else
				{
					/* OTG POWER OFF */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 0);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
				}

				if ((info->ch_ilim_adp != 0xFF) || (info->ch_ilim_adp <= 0x1D))
					nxe2000_write(info->dev->parent, REGISET1_REG, info->ch_ilim_adp);

				info->ubc_check_count = 0;
			}
			break;

		case SUPPLY_STATE_USB:// plug in USB
			info->ubc_check_count = 1;

			if(info->input_power_type == INPUT_POWER_TYPE_UBC)
			{
				if (otg_id == 0)
				{
					set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

					/* OTG POWER ON */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 1);
				}
				else
				{
					/* OTG POWER OFF */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 0);

					//set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
					//set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
				}
			}
			else if (info->input_power_type == INPUT_POWER_TYPE_ADP_UBC)
			{
				set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_CHGP);

				if (otg_id == 0)
				{
					set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);

					/* OTG POWER ON */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 1);
				}
				else
				{
					/* OTG POWER OFF */
					//if (info->gpio_otg_vbus > -1)
					//	gpio_set_value(info->gpio_otg_vbus, 0);

					set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
					set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);

					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
				}
			}
			break;

		case 3:// plug in USB/ADP
			break;

		default:
			break;
	}

#ifdef ENABLE_DEBUG
	PM_LOGOUT( "\n###########################################################\n");
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] val:0x%x, otg_id:0x%x, power_type:0x%x \n", __func__, __LINE__, val, otg_id, info->input_power_type);

	ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] CHGSTATE_REG :0x%02x, val:0x%02x \n", __func__, __LINE__, CHGSTATE_REG, val);

	nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] NXE2000_REG_CHGCTL1 :0x%02x, val:0x%02x \n", __func__, __LINE__, NXE2000_REG_CHGCTL1, val);

	nxe2000_read(info->dev->parent, REGISET1_REG, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] REGISET1_REG:0x%02x, val:0x%02x \n", __func__, __LINE__, REGISET1_REG, val);

	nxe2000_read(info->dev->parent, REGISET2_REG, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] REGISET2_REG:0x%02x, val:0x%02x \n", __func__, __LINE__, REGISET2_REG, val);

	nxe2000_read(info->dev->parent, CHGISET_REG, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] CHGISET_REG:0x%02x, val:0x%02x \n", __func__, __LINE__, CHGISET_REG, val);

	nxe2000_read(info->dev->parent, NXE2000_INT_IR_CHGEXTIF, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] INT_IR_CHGEXTIF:0x%02x, val:0x%02x \n", __func__, __LINE__, NXE2000_INT_IR_CHGEXTIF, val);

	nxe2000_read(info->dev->parent, NXE2000_REG_EXTIF_GCHGDET, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] EXTIF_GCHGDET:0x%02x, val:0x%02x \n", __func__, __LINE__, NXE2000_REG_EXTIF_GCHGDET, val);

	nxe2000_read(info->dev->parent, NXE2000_REG_EXTIF_PCHGDET, &val);
	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] EXTIF_PCHGDET:0x%02x, val:0x%02x \n", __func__, __LINE__, NXE2000_REG_EXTIF_PCHGDET, val);

	PM_LOGOUT( "###########################################################\n");
#endif

/* ======================================================= */

#if (CFG_SW_UBC_ENABLE == 1)
	return;
#endif

	if ( pmic_vbus && otg_id )
	{
		/* OTG DEVICE MODE */
		if (info->gpio_otg_vbus > -1)
			gpio_set_value(info->gpio_otg_vbus, 0);

#ifdef KOOK_ADP_ONLY_MODE
		val     = (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM)
				| (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN)
				| (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
		nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, val);
#endif

/* ======================================================= */

#ifdef KOOK_UBC_CHECK
		if (info->ubc_check_count)
		{
			if (info->input_power_type == INPUT_POWER_TYPE_ADP_UBC)
			{
				uint8_t val2;

				nxe2000_read(info->dev->parent, CHGSTATE_REG, &val2);

				if (val2 & 0x40)
				{
					val = (0x1 << NXE2000_POS_CHGCTL1_CHGP)
						| (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
					nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);
				}
				else if (val2 & 0x80)
				{
					val = (0x1 << NXE2000_POS_CHGCTL1_CHGP)
						| (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM)
						| (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN)
						| (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);
					nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, val);
				}
			}

#if defined(CONFIG_USB_DWCOTG)
			otg_clk_disable();
//			otg_phy_off();
			otg_phy_suspend();
#endif

			nxe2000_clr_bits(info->dev->parent, 0x91, 0x10);    // GPIO4 : Low

			/* GCHGDET:(0xDA) setting */
			val = 0x01;
			ret = nxe2000_write(info->dev->parent, EXTIF_GCHGDET_REG, val);
			if (ret < 0) {
				dev_err(info->dev,
					"Error in writing EXTIF_GCHGDET_REG %d\n", val);
			}

			info->ubc_check_count = 0;
		}
#endif  //#ifdef KOOK_UBC_CHECK

		queue_delayed_work(info->monitor_wqueue,
					&info->get_charger_work, msecs_to_jiffies(900));
	}
	else if (otg_id == 0)
	{
		val = (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
		nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);

		//val = (0x1 << NXE2000_POS_CHGCTL1_SUSPEND);
		//nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);

		/* OTG HOST MODE */
		if (info->gpio_otg_vbus > -1)
			gpio_set_value(info->gpio_otg_vbus, 1);
	}
#ifdef KOOK_UBC_CHECK
	else
	{
		while (delayed_work_pending(&info->get_charger_work))
			cancel_delayed_work(&info->get_charger_work);
	}
#endif

/* ======================================================= */

	if(info->first_soc && info->factory_mode_complete){
		suspend_charge4first_soc(info);
	}
}

#if 0
static void otgid_detect_irq_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		 struct nxe2000_battery_info, otgid_detect_work.work);
	int otg_id = 1;

	if (info->gpio_otg_usbid > -1)
		otg_id = gpio_get_value(info->gpio_otg_usbid);

	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] otg_id:0x%x \n", __func__, __LINE__, otg_id);

	if (otg_id)
		set_otg_power_control(info, 1);
	else
		set_otg_power_control(info, 0);

	msleep(10);
}
#endif

int otgid_power_control_by_dwc(int enable)
{
	if(info_by_dwc == NULL)
		return -ENODEV;

#ifdef ENABLE_DEBUG
	PM_DBGOUT(KERN_ERR "## [\e[31m%s\e[0m():%d] enable:%d\n", __func__, __LINE__, enable);
#endif

	//if (info_by_dwc->gpio_otg_usbid > -1) 
	//{
		if (enable){
			info_by_dwc->gpio_otg_usbid_status = 0;
			set_otg_power_control(info_by_dwc, 0);
		}
		else{
			set_otg_power_control(info_by_dwc, 1);
			info_by_dwc->gpio_otg_usbid_status = 1;
		}
	//}

	return 0;
}
EXPORT_SYMBOL(otgid_power_control_by_dwc);

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
static void low_battery_irq_work(struct work_struct *work)
{
	struct nxe2000_battery_info *info = container_of(work,
		 struct nxe2000_battery_info, low_battery_work.work);

	PM_LOGOUT( "## [\e[31m%s\e[0m():%d] \n", __func__, __LINE__);

	info->low_bat_power_off = true;
	power_supply_changed(&info->battery);
}

static irqreturn_t low_adc_detect_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;

#ifdef ENABLE_DEBUG
	PM_DBGOUT(KERN_ERR "## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
#endif

	queue_delayed_work(info->monitor_wqueue, &info->low_battery_work, LOW_BATTERY_DETECTION_TIME*HZ);
	return IRQ_HANDLED;
}
#endif

static irqreturn_t charger_in_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;

#ifdef ENABLE_DEBUG
	PM_DBGOUT( "## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
#endif

	if(info->first_soc && info->factory_mode_complete){
		suspend_charge4first_soc(info);
	}

	info->chg_stat1 |= 0x01;
	queue_work(info->workqueue, &info->irq_work);
	return IRQ_HANDLED;
}

static irqreturn_t charger_complete_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;

#ifdef ENABLE_DEBUG
	PM_DBGOUT( "## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
#endif

	info->chg_stat1 |= 0x02;
	queue_work(info->workqueue, &info->irq_work);

	return IRQ_HANDLED;
}

static irqreturn_t charger_usb_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;

#ifdef ENABLE_DEBUG
	PM_DBGOUT( "## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
#endif

	info->chg_ctr |= 0x02;
	queue_work(info->workqueue, &info->irq_work);

	info->soca->dischg_state = 0;
	if (info->first_soc == 0) {
		info->soca->chg_count = 0;
	}

	if (NXE2000_SOCA_UNSTABLE == info->soca->status
		|| NXE2000_SOCA_FG_RESET == info->soca->status)
		info->soca->stable_count = 11;

#if (CFG_SW_UBC_ENABLE == 1) && (CFG_USB_DET_FROM_PMIC_INT == 1)
	dwc_otg_pcd_clear_ep0_state();
	info->pmic_vbuschk_count = PMIC_VBUSCHK_COUNT;
	info->flag_set_ilimit = false;
	queue_delayed_work(info->monitor_wqueue, &info->sw_ubc_work, msecs_to_jiffies(1000));
#endif

	return IRQ_HANDLED;
}

static irqreturn_t charger_adp_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;

#ifdef ENABLE_DEBUG
	PM_DBGOUT( "## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
#endif

	info->chg_ctr |= 0x01;
	queue_work(info->workqueue, &info->irq_work);

	info->soca->dischg_state = 0;
	if (info->first_soc == 0) {
		info->soca->chg_count = 0;
	}

	if (NXE2000_SOCA_UNSTABLE == info->soca->status
		|| NXE2000_SOCA_FG_RESET == info->soca->status)
		info->soca->stable_count = 11;

	return IRQ_HANDLED;
}

#if (CFG_SW_UBC_ENABLE == 1) && (CFG_USB_DET_FROM_PMIC_INT == 0)
static irqreturn_t sw_ubc_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;

#ifdef ENABLE_DEBUG
	PM_DBGOUT( "## [\e[31m%s\e[0m():%d]\n", __func__, __LINE__);
#endif

	dwc_otg_pcd_clear_ep0_state();
	info->pmic_vbuschk_count = PMIC_VBUSCHK_COUNT;
	info->flag_set_ilimit = false;
	queue_delayed_work(info->monitor_wqueue, &info->sw_ubc_work, msecs_to_jiffies(1000));
	return IRQ_HANDLED;
}
#endif
#if 0 // by hsjung
static irqreturn_t otgid_det_isr(int irq, void *battery_info)
{
	struct nxe2000_battery_info *info = battery_info;
	queue_delayed_work(info->monitor_wqueue, &info->otgid_detect_work, msecs_to_jiffies(20));
	return IRQ_HANDLED;
}
#endif

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
static int get_check_fuel_gauge_reg(struct nxe2000_battery_info *info,
					 int Reg_h, int Reg_l, int enable_bit)
{
	uint8_t get_data_h, get_data_l;
	int old_data, current_data;
	int i;
	int ret = 0;

	old_data = 0;

	for (i = 0; i < 5 ; i++) {
		ret = nxe2000_read(info->dev->parent, Reg_h, &get_data_h);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			return ret;
		}

		ret = nxe2000_read(info->dev->parent, Reg_l, &get_data_l);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			return ret;
		}

		current_data = ((get_data_h & 0xff) << 8) | (get_data_l & 0xff);
		current_data = (current_data & enable_bit);

		if (current_data == old_data)
			return current_data;
		else
			old_data = current_data;
	}

	return current_data;
}

static int calc_capacity(struct nxe2000_battery_info *info)
{
	uint8_t capacity;
	long    capacity_l;
	int temp;
	int ret = 0;
	int nt;
	int temperature;
	int cc_cap = 0;
	int cc_delta;
	bool is_charging = true;

	if (info->soca->rsoc_ready_flag  != 0) {
		/* get remaining battery capacity from fuel gauge */
		ret = nxe2000_read(info->dev->parent, SOC_REG, &capacity);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			return ret;
		}
		capacity_l = (long)capacity;
	} else {
		ret = calc_capacity_in_period(info, &cc_cap, &is_charging, 0);
		cc_delta = (is_charging == true) ? cc_cap : -cc_cap;
		capacity_l = (info->soca->init_pswr * 100 + cc_delta) / 100;
		PM_LOGOUT("PMU FG_RESET : %s : capacity %ld init_pswr %d cc_delta %d\n",__func__, capacity_l, info->soca->init_pswr, cc_delta);
	}

	temperature = get_battery_temp_2(info) / 10; /* unit 0.1 degree -> 1 degree */

	if (temperature >= 25) {
		nt = 0;
	} else if (temperature >= 5) {
		nt = (25 - temperature) * NXE2000_TAH_SEL2 * 625 / 100;
	} else {
		nt = (625  + (5 - temperature) * NXE2000_TAL_SEL2 * 625 / 100);
	}

	temp = capacity_l * 100 * 100 / (10000 - nt);

	temp = MIN(100, temp);
	temp = MAX(0, temp);

	return temp;		/* Unit is 1% */
}

static int calc_capacity_2(struct nxe2000_battery_info *info)
{
	uint8_t val;
	long capacity;
	int re_cap, fa_cap;
	int temp;
	int ret = 0;
	int nt;
	int temperature;
	int cc_cap = 0;
	int cc_delta;
	bool is_charging = true;


	if (info->soca->rsoc_ready_flag  != 0) {
		re_cap = get_check_fuel_gauge_reg(info, RE_CAP_H_REG, RE_CAP_L_REG,
							0x7fff);
		fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
							0x7fff);

		if (fa_cap != 0) {
			capacity = ((long)re_cap * 100 * 100 / fa_cap);
			capacity = (long)(MIN(10000, (int)capacity));
			capacity = (long)(MAX(0, (int)capacity));
		} else {
			ret = nxe2000_read(info->dev->parent, SOC_REG, &val);
			if (ret < 0) {
				dev_err(info->dev, "Error in reading the control register\n");
				return ret;
			}
			capacity = (long)val * 100;
		}
	} else {
		ret = calc_capacity_in_period(info, &cc_cap, &is_charging, 0);
		cc_delta = (is_charging == true) ? cc_cap : -cc_cap;
		capacity = info->soca->init_pswr * 100 + cc_delta;
		PM_LOGOUT("PMU FG_RESET : %s : capacity %d init_pswr %d cc_delta %d\n",__func__,	(int)capacity, info->soca->init_pswr, cc_delta);
	}

	temperature = get_battery_temp_2(info) / 10; /* unit 0.1 degree -> 1 degree */

	if (temperature >= 25) {
		nt = 0;
	} else if (temperature >= 5) {
		nt = (25 - temperature) * NXE2000_TAH_SEL2 * 625 / 100;
	} else {
		nt = (625  + (5 - temperature) * NXE2000_TAL_SEL2 * 625 / 100);
	}

	temp = (int)(capacity * 100 * 100 / (10000 - nt));

	temp = MIN(10000, temp);
	temp = MAX(0, temp);

	return temp;		/* Unit is 0.01% */
}

static int calc_first_soc(struct nxe2000_battery_info *info)
{
	int Ocv_temp, Ocv_ave, Ocv_gap_min, Ocv_gap_max;
	int i, j;
	int capacity;

	Ocv_temp = 0;

	for (i = 0; i < NXE2000_GET_CHARGE_NUM - 1; i++) {
		for (j = NXE2000_GET_CHARGE_NUM - 1; j > i; j--) {
			if (info->soca->Ocv[j - 1] > info->soca->Ocv[j]) {
				Ocv_temp = info->soca->Ocv[j];
				info->soca->Ocv[j] = info->soca->Ocv[j - 1];
				info->soca->Ocv[j - 1] = Ocv_temp;
			}
		}
	}

	for (i = 0; i < NXE2000_GET_CHARGE_NUM; i++) {
		PM_LOGOUT(KERN_INFO "PMU: %s (%d) Ocv(%d)\n",
			 __func__, i, info->soca->Ocv[i]);
	}

	Ocv_temp = 0;

	for (i = 3; i < NXE2000_GET_CHARGE_NUM-3; i++) {
		Ocv_temp = Ocv_temp + info->soca->Ocv[i];
	}
	Ocv_ave = Ocv_temp / (NXE2000_GET_CHARGE_NUM - 6);
	Ocv_gap_min = info->soca->Ocv[6] - info->soca->Ocv[3];
	Ocv_gap_max = info->soca->Ocv[NXE2000_GET_CHARGE_NUM-1] - info->soca->Ocv[0];
	capacity = calc_soc_on_ocv(info, Ocv_ave) / 100;

	PM_LOGOUT(KERN_INFO "PMU: %s Ocv_ave(%d), Ocv_gap4(%d), Ocv_gap10(%d)\n", __func__, Ocv_ave, Ocv_gap_min, Ocv_gap_max);
	PM_LOGOUT(KERN_INFO "PMU: %s first soc(%d)\n", __func__, capacity);

	return capacity;
}

/**
* Suspend Charging to calculate first soc.
**/
static void suspend_charge4first_soc(struct nxe2000_battery_info *info)
{
	int charge_status;
	int err;

	charge_status = get_power_supply_status(info);

	if ((charge_status == POWER_SUPPLY_STATUS_FULL) || (info->input_power_type == INPUT_POWER_TYPE_ADP_NOBATTERY))
	{
		return;
	}
	else
	{
		err = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, 0x08);
		if (err < 0)
			dev_err(info->dev, "Error in writing the control register\n");
	}

	return;
}

#ifndef CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE
static int get_battery_temp(struct nxe2000_battery_info *info)
{
	int ret = 0;
	int sign_bit;

	ret = get_check_fuel_gauge_reg(info, TEMP_1_REG, TEMP_2_REG, 0x0fff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	/* bit3 of 0xED(TEMP_1) is sign_bit */
	sign_bit = ((ret & 0x0800) >> 11);

	ret = (ret & 0x07ff);

	if (sign_bit == 0)	/* positive value part */
		/* conversion unit */
		/* 1 unit is 0.0625 degree and retun unit
		 * should be 0.1 degree,
		 */
		ret = ret * 625  / 1000;
	else {	/*negative value part */
		ret = (~ret + 1) & 0x7ff;
		ret = -1 * ret * 625 / 1000;
	}

	return ret;
}
#endif

static int get_battery_temp_2(struct nxe2000_battery_info *info)
{
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	if(NXP_Get_BoardTemperature() == 0)
		return 270;

	return (10*(NXP_Get_BoardTemperature()-20));
#else
	uint8_t reg_buff[2];
	long temp, temp_off, temp_gain;
	bool temp_sign, temp_off_sign, temp_gain_sign;
	int Vsns = 0;
	int Iout = 0;
	int Vthm, Rthm;
	int reg_val = 0;
	int new_temp;
	long R_ln1, R_ln2;
	int ret = 0;

	/* Calculate TEMP */
	ret = get_check_fuel_gauge_reg(info, TEMP_1_REG, TEMP_2_REG, 0x0fff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge register\n");
		goto out;
	}

	reg_val = ret;
	temp_sign = (reg_val & 0x0800) >> 11;
	reg_val = (reg_val & 0x07ff);

	if (temp_sign == 0)	/* positive value part */
		/* the unit is 0.0001 degree */
		temp = (long)reg_val * 625;
	else {	/*negative value part */
		reg_val = (~reg_val + 1) & 0x7ff;
		temp = -1 * (long)reg_val * 625;
	}

	/* Calculate TEMP_OFF */
	ret = nxe2000_bulk_reads_bank1(info->dev->parent,
					TEMP_OFF_H_REG, 2, reg_buff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge register\n");
		goto out;
	}

	reg_val = reg_buff[0] << 8 | reg_buff[1];
	temp_off_sign = (reg_val & 0x0800) >> 11;
	reg_val = (reg_val & 0x07ff);

	if (temp_off_sign == 0)	/* positive value part */
		/* the unit is 0.0001 degree */
		temp_off = (long)reg_val * 625;
	else {	/*negative value part */
		reg_val = (~reg_val + 1) & 0x7ff;
		temp_off = -1 * (long)reg_val * 625;
	}

	/* Calculate TEMP_GAIN */
	ret = nxe2000_bulk_reads_bank1(info->dev->parent,
					TEMP_GAIN_H_REG, 2, reg_buff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge register\n");
		goto out;
	}

	reg_val = reg_buff[0] << 8 | reg_buff[1];
	temp_gain_sign = (reg_val & 0x0800) >> 11;
	reg_val = (reg_val & 0x07ff);

	if (temp_gain_sign == 0)	/* positive value part */
		/* 1 unit is 0.000488281. the result is 0.01 */
		temp_gain = (long)reg_val * 488281 / 100000;
	else {	/*negative value part */
		reg_val = (~reg_val + 1) & 0x7ff;
		temp_gain = -1 * (long)reg_val * 488281 / 100000;
	}

	/* Calculate VTHM */
	if (0 != temp_gain)
		Vthm = (int)((temp - temp_off) / 4095 * 2500 / temp_gain);
	else {
		goto out;
	}

	ret = measure_Ibatt_FG(info, &Iout);
	Vsns = Iout * 2 / 100;

	if (temp < -120000) {
		/* Low Temperature */
		if (0 != (2500 - Vthm)) {
			Rthm = 10 * 10 * (Vthm - Vsns) / (2500 - Vthm);
		} else {
			goto out;
		}

		R_ln1 = Rthm / 10;
		R_ln2 =  (R_ln1 * R_ln1 * R_ln1 * R_ln1 * R_ln1 / 100000
			- R_ln1 * R_ln1 * R_ln1 * R_ln1 * 2 / 100
			+ R_ln1 * R_ln1 * R_ln1 * 11
			- R_ln1 * R_ln1 * 2980
			+ R_ln1 * 449800
			- 784000) / 10000;

		/* the unit of new_temp is 0.1 degree */
		new_temp = (int)((100 * 1000 * B_VALUE / (R_ln2 + B_VALUE * 100 * 1000 / 29815) - 27315) / 10);
	} else if (temp > 520000) {
		/* High Temperature */
		if (0 != (2500 - Vthm)) {
			Rthm = 100 * 10 * (Vthm - Vsns) / (2500 - Vthm);
		} else {
			goto out;
		}

		R_ln1 = Rthm / 10;
		R_ln2 =  (R_ln1 * R_ln1 * R_ln1 * R_ln1 * R_ln1 / 100000 * 15652 / 100
			- R_ln1 * R_ln1 * R_ln1 * R_ln1 / 1000 * 23103 / 100
			+ R_ln1 * R_ln1 * R_ln1 * 1298 / 100
			- R_ln1 * R_ln1 * 35089 / 100
			+ R_ln1 * 50334 / 10
			- 48569) / 100;
		/* the unit of new_temp is 0.1 degree */
		new_temp = (int)((100 * 100 * B_VALUE / (R_ln2 + B_VALUE * 100 * 100 / 29815) - 27315) / 10);
	} else {
		/* the unit of new_temp is 0.1 degree */
		new_temp = temp / 1000;
	}

	return new_temp;

out:
	new_temp = get_battery_temp(info);
	return new_temp;
#endif
}

static int get_time_to_empty(struct nxe2000_battery_info *info)
{
	int ret = 0;

	ret = get_check_fuel_gauge_reg(info, TT_EMPTY_H_REG, TT_EMPTY_L_REG,
								0xffff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	/* conversion unit */
	/* 1unit is 1miniute and return nnit should be 1 second */
	ret = ret * 60;

	return ret;
}

static int get_time_to_full(struct nxe2000_battery_info *info)
{
	int ret = 0;

	ret = get_check_fuel_gauge_reg(info, TT_FULL_H_REG, TT_FULL_L_REG,
								0xffff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	ret = ret * 60;

	return  ret;
}

/* battery voltage is get from Fuel gauge */
static int measure_vbatt_FG(struct nxe2000_battery_info *info, int *data)
{
	int uv_per_unit = 0;
	int ret = 0;

	if(info->soca->ready_fg == 1) {
		ret = get_check_fuel_gauge_reg(info, VOLTAGE_1_REG, VOLTAGE_2_REG,
									0x0fff);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the fuel gauge control register\n");
			return ret;
		}

		*data = ret;
#if 0
		/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
		*data = *data * 50000 / 4095;
		/* return unit should be 1uV */
		*data = *data * 100;
#else
		/* conversion unit 1 Unit is 1221uv (5000/4095 mv) */
		uv_per_unit = 5000000 / 4095;
		*data = *data * uv_per_unit;
#endif
		info->soca->Vbat_old = *data;
	} else {
		*data = info->soca->Vbat_old;
	}

	return ret;
}

static int measure_Ibatt_FG(struct nxe2000_battery_info *info, int *data)
{
	int ret = 0;

	ret =  get_check_fuel_gauge_reg(info, CC_AVERAGE1_REG,
						 CC_AVERAGE0_REG, 0x3fff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	*data = (ret > 0x1fff) ? (ret - 0x4000) : ret;
	return ret;
}

static int get_OCV_init_Data(struct nxe2000_battery_info *info, int index)
{
	int ret = 0;
	ret =  (battery_init_para[info->num][index*2]<<8) | (battery_init_para[info->num][index*2+1]);
	return ret;
}

static int get_OCV_voltage(struct nxe2000_battery_info *info, int index)
{
	int uv_per_unit = 0;
	int ret = 0;

	ret =  get_OCV_init_Data(info, index);
#if 0
	/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
	ret = ret * 50000 / 4095;
	/* return unit should be 1uV */
	ret = ret * 100;
#else
	/* conversion unit 1 Unit is 1221uv (5000/4095 mv) */
	uv_per_unit = 5000000 / 4095;
	ret = ret * uv_per_unit;
#endif
	return ret;
}

#else
/* battery voltage is get from ADC */
static int measure_vbatt_ADC(struct nxe2000_battery_info *info, int *data)
{
	int	i;
	uint8_t data_l = 0, data_h = 0;
	int uv_per_unit = 0;
	int ret;

	/* ADC interrupt enable */
	ret = nxe2000_set_bits(info->dev->parent, INTEN_REG, 0x08);
	if (ret < 0) {
		dev_err(info->dev, "Error in setting the control register bit\n");
		goto err;
	}

	/* enable interrupt request of single mode */
	ret = nxe2000_set_bits(info->dev->parent, EN_ADCIR3_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev, "Error in setting the control register bit\n");
		goto err;
	}

	/* single request */
	ret = nxe2000_write(info->dev->parent, ADCCNT3_REG, 0x10);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		goto err;
	}

	for (i = 0; i < 5; i++) {
		usleep(1000);
		DBGOUT(info->dev, "ADC conversion times: %d\n", i);
		/* read completed flag of ADC */
		ret = nxe2000_read(info->dev->parent, EN_ADCIR3_REG, &data_h);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			goto err;
		}

		if (data_h & 0x01)
			goto	done;
	}

	dev_err(info->dev, "ADC conversion too long!\n");
	goto err;

done:
	ret = nxe2000_read(info->dev->parent, VBATDATAH_REG, &data_h);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		goto err;
	}

	ret = nxe2000_read(info->dev->parent, VBATDATAL_REG, &data_l);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		goto err;
	}

	*data = ((data_h & 0xff) << 4) | (data_l & 0x0f);
#if 0
	/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
	*data = *data * 50000 / 4095;
	/* return unit should be 1uV */
	*data = *data * 100;
#else
	/* conversion unit 1 Unit is 1221uv (5000/4095 mv) */
	uv_per_unit = 5000000 / 4095;
	*data = *data * uv_per_unit;
#endif
	return 0;

err:
	return -1;
}
#endif

static int measure_vsys_ADC(struct nxe2000_battery_info *info, int *data)
{
	uint8_t data_l = 0, data_h = 0;
	int uv_per_unit = 0;
	int ret;

	ret = nxe2000_read(info->dev->parent, VSYSDATAH_REG, &data_h);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
	}

	ret = nxe2000_read(info->dev->parent, VSYSDATAL_REG, &data_l);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
	}

	*data = ((data_h & 0xff) << 4) | (data_l & 0x0f);
#if 0
	/* conversion unit 1 Unit is 1.83mv (7500/4095 mv) */
	*data = *data * 1000 * 3 * 5 / 2 / 4095;
	/* return unit should be 1uV */
	*data = *data * 1000;
#else
	/* conversion unit 1 Unit is 1831uv (7500/4095 mv) */
	uv_per_unit = 7500000 / 4095;
	*data = *data * uv_per_unit;
#endif

	return 0;
}

static void nxe2000_external_power_changed(struct power_supply *psy)
{
	struct nxe2000_battery_info *info;

	info = container_of(psy, struct nxe2000_battery_info, battery);
	queue_delayed_work(info->monitor_wqueue,
				&info->changed_work, HZ / 2);
	return;
}

static int set_low_bat_dsoc(struct nxe2000_battery_info *info, int val)
{
	int ret;

	info->soca->displayed_soc = val * 100;

	ret = nxe2000_write(info->dev->parent, PSWR_REG, (uint8_t)val);
	if (ret < 0)
		dev_err(info->dev, "Error in writing PSWR_REG\n");
	g_soc = val;

	info->soca->target_use_cap = 0;
	info->soca->status = NXE2000_SOCA_LOW_VOL;

	PM_LOGOUT(KERN_INFO "PMU: %s displayed_soc(%d)\n",
		 __func__, info->soca->displayed_soc);

	return ret;
}

static int nxe2000_batt_get_prop(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct nxe2000_battery_info *info = dev_get_drvdata(psy->dev->parent);
	int data = 0;
	int ret = 0;

	mutex_lock(&info->lock);

	val->intval = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		{
			uint8_t status;

			ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &status);
			if (ret < 0) {
				dev_err(info->dev, "Error in reading the control register\n");
				mutex_unlock(&info->lock);
				return ret;
			}

#ifdef KOOK_UBC_CHECK
			if (status & 0xC0)
			{
				if ((info->input_power_type == INPUT_POWER_TYPE_ADP)
					|| ((info->input_power_type == INPUT_POWER_TYPE_ADP_UBC) && (status & 0x40))
					|| ((info->input_power_type == INPUT_POWER_TYPE_UBC) && (status & 0x40)))
				{
					if (psy->type == POWER_SUPPLY_TYPE_MAINS)
						val->intval = (status & 0x40 ? 1 : 0);
					else if (psy->type == POWER_SUPPLY_TYPE_USB)
						val->intval = (status & 0x80 ? 1 : 0);
				}
				else
				{
#if (CFG_SW_UBC_ENABLE == 1)
					if (psy->type == POWER_SUPPLY_TYPE_MAINS)
						val->intval = (!info->is_sdp_type ? 1 : 0);
					else if (psy->type == POWER_SUPPLY_TYPE_USB)
						val->intval = (info->is_sdp_type ? 1 : 0);
#else
					if (psy->type == POWER_SUPPLY_TYPE_MAINS)
						val->intval = (info->extif_type == EXTIF_TYPE_DCP ? 1 : 0);
					else if (psy->type == POWER_SUPPLY_TYPE_USB)
						val->intval = (info->extif_type == EXTIF_TYPE_SDP ? 1 : 0);
#endif
				}
			}
#else	// KOOK_UBC_CHECK

			if (status & 0xC0)
			{
				if (psy->type == POWER_SUPPLY_TYPE_MAINS)
					val->intval = (status & 0x40 ? 1 : 0);
				else if (psy->type == POWER_SUPPLY_TYPE_USB)
					val->intval = (status & 0x80 ? 1 : 0);
			}
#endif

#ifdef KOOK_FAKE_POWER_ON_EVENT
			if (nxe2000_power_resume_status)
			{
				val->intval = 1;
				nxe2000_power_resume_status = 0;
			}
			else
#endif
			{
				if ((info->capacity == 0) && (info->soca->Ibat_ave < 0))
					val->intval = 0;
			}

			info->online_state = val->intval;

			if(psy->type == POWER_SUPPLY_TYPE_MAINS)
				DBGOUT(info->dev, "battery online statue(AC) : %d\n", info->online_state);
			else if(psy->type == POWER_SUPPLY_TYPE_USB)
				DBGOUT(info->dev, "battery online statue(USB) : %d\n", info->online_state);
			else
				DBGOUT(info->dev, "battery online statue(unknown) : %d\n", info->online_state);
		}
		break;

	/* this setting is same as battery driver of 584 */
	case POWER_SUPPLY_PROP_STATUS:
		info->status = get_power_supply_Android_status(info);

		if ((info->capacity == 0) && (info->soca->Ibat_ave < 0)) {
			info->status = POWER_SUPPLY_STATUS_DISCHARGING;
		}

		val->intval = info->status;
		DBGOUT(info->dev, "Power Supply Status is %d\n",
							info->status);
		break;

	/* this setting is same as battery driver of 584 */
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = info->present;
		break;

	/* current voltage is got from fuel gauge */
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* return real vbatt Voltage */
#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
		if (info->soca->ready_fg)
			ret = measure_vbatt_FG(info, &data);
		else {
			//val->intval = -EINVAL;
			data = info->cur_voltage * 1000;
		}
#else
		ret = measure_vbatt_ADC(info, &data);
#endif
		val->intval = data;
		/* convert unit uV -> mV */
		info->cur_voltage = data / 1000;
		DBGOUT(info->dev, "battery voltage is %d mV\n",
						info->cur_voltage);
		break;

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
	/* current battery capacity is get from fuel gauge */
	case POWER_SUPPLY_PROP_CAPACITY:
#ifdef KOOK_FAKE_POWER_ON_EVENT
		if (nxe2000_power_lowbat)
		{
			val->intval = (5 * nxe2000_power_lowbat);       /* unit is % */
			nxe2000_power_lowbat--;
			break;
		}
#endif

		if (info->entry_factory_mode) {
			info->capacity = 100;
		} else if (info->soca->displayed_soc <= 0) {
			info->capacity = 0;
		} else {
			info->capacity = (info->soca->displayed_soc + 50)/100;
		}

		if (info->input_power_type == INPUT_POWER_TYPE_ADP_NOBATTERY)
		{
			val->intval = info->capacity = 50;
		}

		val->intval = info->capacity;

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
		if(info->low_bat_power_off == true)
		{
			static int sys_low_vol_cnt = 2;
			if(sys_low_vol_cnt)
			{
				val->intval = info->capacity = sys_low_vol_cnt;
				sys_low_vol_cnt--;
			}
			else
			{
				val->intval = info->capacity = 0;
			}

			set_low_bat_dsoc(info, val->intval);
		}
#endif

		DBGOUT(info->dev, "battery capacity is %d%%\n",
							info->capacity);
		break;

	/* current temperature of battery */
	case POWER_SUPPLY_PROP_TEMP:
		if (info->soca->ready_fg) {
			val->intval = get_battery_temp_2(info);
			info->battery_temp = val->intval/10;
			DBGOUT(info->dev, "battery temperature is %d degree\n",
							 info->battery_temp);
		} else {
			val->intval = info->battery_temp * 10;
			DBGOUT(info->dev, "battery temperature is %d degree\n",
							 info->battery_temp);
		}
		break;

	case POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW:
		if (info->soca->ready_fg) {
			val->intval = get_time_to_empty(info);
			info->time_to_empty = val->intval/60;
			DBGOUT(info->dev, "time of empty battery is %d minutes\n",
							 info->time_to_empty);
		} else {
			//val->intval = -EINVAL;
			val->intval = info->time_to_empty * 60;
			DBGOUT(info->dev, "time of empty battery is %d minutes\n",
							 info->time_to_empty);
		}
		break;

	case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
		if (info->soca->ready_fg) {
			val->intval = get_time_to_full(info);
			info->time_to_full = val->intval/60;
			DBGOUT(info->dev, "time of full battery is %d minutes\n",
							 info->time_to_full);
		} else {
			//val->intval = -EINVAL;
			val->intval = info->time_to_full * 60;
			DBGOUT(info->dev, "time of full battery is %d minutes\n",
							 info->time_to_full);
		}
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = info->full_capacity;
		break;

#endif  /* ENABLE_FUEL_GAUGE_FUNCTION */
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;

	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = "NXE2000";
		break;

	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = "www.nexell.co.kr";
		break;

	default:
		ret = -ENODEV;
	}

	mutex_unlock(&info->lock);

	return ret;
}

static enum power_supply_property nxe2000_batt_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
	POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
#endif
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
};

static enum power_supply_property nxe2000_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

struct power_supply	powerac = {
		.name = "acpwr",
		.type = POWER_SUPPLY_TYPE_MAINS,
		.properties = nxe2000_power_props,
		.num_properties = ARRAY_SIZE(nxe2000_power_props),
		.get_property = nxe2000_batt_get_prop,
};

struct power_supply	powerusb = {
		.name = "usbpwr",
		.type = POWER_SUPPLY_TYPE_USB,
		.properties = nxe2000_power_props,
		.num_properties = ARRAY_SIZE(nxe2000_power_props),
		.get_property = nxe2000_batt_get_prop,
};

static void set_gpio_config(struct nxe2000_battery_info *info)
{
#if defined(CONFIG_USB_DWCOTG)
	//if ( (info->gpio_otg_usbid > -1)
	//	&& (nxp_soc_gpio_get_io_func(info->gpio_otg_usbid) != nxp_soc_gpio_get_altnum(info->gpio_otg_usbid)) )
	//{
	//	nxp_soc_gpio_set_io_func(info->gpio_otg_usbid, nxp_soc_gpio_get_altnum(info->gpio_otg_usbid));
	//	nxp_soc_gpio_set_io_dir(info->gpio_otg_usbid, 0);      // input mode
	//}

	if ( (info->gpio_otg_vbus > -1)
		&& (nxp_soc_gpio_get_io_func(info->gpio_otg_vbus) != nxp_soc_gpio_get_altnum(info->gpio_otg_vbus)) )
	{
		nxp_soc_gpio_set_io_func(info->gpio_otg_vbus, nxp_soc_gpio_get_altnum(info->gpio_otg_vbus));
		nxp_soc_gpio_set_io_dir(info->gpio_otg_vbus, 1);       // output mode

//		nxp_soc_gpio_set_out_value(info->gpio_otg_vbus, 0);
		gpio_set_value(info->gpio_otg_vbus, 0);
	}
#endif

	if ( (info->gpio_pmic_vbus > -1)
		&& (nxp_soc_gpio_get_io_func(info->gpio_pmic_vbus) != nxp_soc_gpio_get_altnum(info->gpio_pmic_vbus)) )
	{
		nxp_soc_gpio_set_io_func(info->gpio_pmic_vbus, nxp_soc_gpio_get_altnum(info->gpio_pmic_vbus));
		nxp_soc_gpio_set_io_dir(info->gpio_pmic_vbus, 0);      // input mode
	}

	if ( (info->gpio_pmic_lowbat > -1)
		&& (nxp_soc_gpio_get_io_func(info->gpio_pmic_lowbat) != nxp_soc_gpio_get_altnum(info->gpio_pmic_lowbat)) )
	{
		nxp_soc_gpio_set_io_func(info->gpio_pmic_lowbat, nxp_soc_gpio_get_altnum(info->gpio_pmic_lowbat));
		nxp_soc_gpio_set_io_dir(info->gpio_pmic_lowbat, 0);    // input mode
	}

#if 0
	if ( (info->gpio_eint > -1)
		&& (nxp_soc_gpio_get_io_func(info->gpio_eint) != nxp_soc_gpio_get_altnum(info->gpio_eint)) )
	{
		nxp_soc_gpio_set_io_func(info->gpio_eint, nxp_soc_gpio_get_altnum(info->gpio_eint));
		nxp_soc_gpio_set_io_dir(info->gpio_eint, 0);           // input mode
	}
#endif
}

#ifdef BAT_RESUME_WORK_QUEUE
static void nxe2000_battery_resume_work(struct work_struct *work);
#endif

static __devinit int nxe2000_battery_probe(struct platform_device *pdev)
{
	struct nxe2000_battery_info *info;
	struct nxe2000_battery_platform_data *pdata;
	int type_n;
	int ret, temp;
	int otg_id = 1;
	uint8_t fa_cap_reg[2];
	uint8_t set_val = 0;

	info = kzalloc(sizeof(struct nxe2000_battery_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->soca = kzalloc(sizeof(struct nxe2000_soca_info), GFP_KERNEL);
	if (!info->soca)
		return -ENOMEM;

	dev_info(&pdev->dev, "%s\n", NXE2000_BATTERY_VERSION);

	info->dev = &pdev->dev;
	info->status = POWER_SUPPLY_STATUS_CHARGING;
	pdata = pdev->dev.platform_data;
	info->monitor_time = pdata->monitor_time;
	info->input_power_type  = pdata->input_power_type;
	//info->gpio_otg_usbid    = pdata->gpio_otg_usbid;
	info->gpio_otg_usbid_status = 1;
	info->gpio_otg_vbus     = pdata->gpio_otg_vbus;
	info->gpio_pmic_vbus    = pdata->gpio_pmic_vbus;
	info->gpio_pmic_lowbat  = pdata->gpio_pmic_lowbat;

	/* check rage of b,.attery type */
	type_n = Battery_Type();
	temp = sizeof(pdata->type)/(sizeof(struct nxe2000_battery_type_data));
	if(type_n  >= temp)
	{
		type_n = 0;
	}

	/* check rage of battery num */
	info->num = Battery_Table();
	temp = sizeof(battery_init_para)/(sizeof(uint8_t)*32);
	if(info->num >= (sizeof(battery_init_para)/(sizeof(uint8_t)*32)))
	{
		info->num = 0;
	}

	/* these valuse are set in platform */
	info->ch_vfchg		= pdata->type[type_n].ch_vfchg;
	info->ch_vrchg		= pdata->type[type_n].ch_vrchg;
	info->ch_vbatovset	= pdata->type[type_n].ch_vbatovset;
	info->ch_ichg		= pdata->type[type_n].ch_ichg;
	info->ch_ichg_slp	= pdata->type[type_n].ch_ichg_slp;
	info->ch_ilim_adp	= pdata->type[type_n].ch_ilim_adp;
	info->ch_ilim_usb	= pdata->type[type_n].ch_ilim_usb;
	info->ch_ilim_dcp	= 0xE0 | ((info->ch_ilim_adp < 15) ? (info->ch_ilim_adp) : 14);
	info->ch_icchg		= pdata->type[type_n].ch_icchg;
	info->fg_target_vsys= pdata->type[type_n].fg_target_vsys;
	info->fg_target_ibat= pdata->type[type_n].fg_target_ibat;
	info->fg_poff_vbat	= pdata->type[type_n].fg_poff_vbat;
	info->jt_en			= pdata->type[type_n].jt_en;
	info->jt_hw_sw		= pdata->type[type_n].jt_hw_sw;
	info->jt_temp_h		= pdata->type[type_n].jt_temp_h;
	info->jt_temp_l		= pdata->type[type_n].jt_temp_l;
	info->jt_vfchg_h	= pdata->type[type_n].jt_vfchg_h;
	info->jt_vfchg_l	= pdata->type[type_n].jt_vfchg_l;
	info->jt_ichg_h		= pdata->type[type_n].jt_ichg_h;
	info->jt_ichg_l		= pdata->type[type_n].jt_ichg_l;
	info->adc_vdd_mv	= ADC_VDD_MV;		/* 2800; */
	info->min_voltage	= MIN_VOLTAGE;	/* 3100; */
	info->max_voltage	= MAX_VOLTAGE;	/* 4200; */
	info->delay			= 500;
	info->entry_factory_mode = false;

	info->low_vbat_vol_mv = pdata->low_vbat_vol_mv;
	info->low_vsys_vol_mv = pdata->low_vsys_vol_mv;

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	info->low_bat_power_off = false;
#endif

	info->online_state		= 0;
	info->ubc_check_count	= 0;

	mutex_init(&info->lock);

	platform_set_drvdata(pdev, info);

	info->battery.name = "battery";
	info->battery.type = POWER_SUPPLY_TYPE_BATTERY;
	info->battery.properties = nxe2000_batt_props;
	info->battery.num_properties = ARRAY_SIZE(nxe2000_batt_props);
	info->battery.get_property = nxe2000_batt_get_prop;
	info->battery.set_property = NULL;
	info->battery.external_power_changed = nxe2000_external_power_changed;

#if 0	//(CFG_SW_UBC_ENABLE == 1)
	nxe2000_clr_bits(info->dev->parent, 0x90, 0x10);	// GPIO4 : Input
#else
	nxe2000_set_bits(info->dev->parent, 0x90, 0x10);	// GPIO4 : Output
#endif
	nxe2000_set_bits(info->dev->parent, 0x91, 0x10);	// GPIO4 : High(Hi-Z)

	set_gpio_config(info);

	//if (info->gpio_otg_usbid > -1)
	//	otg_id = gpio_get_value(info->gpio_otg_usbid);
	otg_id = info->gpio_otg_usbid_status;

#if 1
	if (otg_id == 0)
	{
		nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &set_val);
		set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
		set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
		nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
	}
#else
	set_val = (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM);
	set_val |= (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
	set_val |= (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);
	set_val |= (0x0 << NXE2000_POS_CHGCTL1_CHGP);

	if(info->input_power_type == INPUT_POWER_TYPE_ADP_UBC)
	{
		if (otg_id == 0)
		{
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
		}
	}
	else if(info->input_power_type == INPUT_POWER_TYPE_UBC)
	{
		set_val	|= (0x1 << NXE2000_POS_CHGCTL1_CHGP);

		if (otg_id == 0)
		{
			set_val |= (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
			set_val &= ~(0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
		}
	}
	nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
#endif

	/* Disable Charger/ADC interrupt */
	ret = nxe2000_clr_bits(info->dev->parent, NXE2000_INTC_INTEN, CHG_INT | ADC_INT);
	if (ret)
		goto out;

	ret = nxe2000_init_battery(info);

	info_by_dwc = info;
	if (ret)
		goto out;

#ifdef ENABLE_FACTORY_MODE
	info->factory_mode_complete = false;
	info->factory_mode_wqueue = create_singlethread_workqueue("nxe2000_factory_mode");

	INIT_DELAYED_WORK_DEFERRABLE(&info->factory_mode_work, check_charging_state_work);

	ret = nxe2000_factory_mode(info);
	if (ret)
		goto out;
#else
	info->factory_mode_complete = true;
#endif

	ret = power_supply_register(&pdev->dev, &info->battery);
	if (ret)
		info->battery.dev->parent = &pdev->dev;
	ret = power_supply_register(&pdev->dev, &powerac);
	ret = power_supply_register(&pdev->dev, &powerusb);

	info->monitor_wqueue = create_singlethread_workqueue("nxe2000_battery_monitor");
	info->workqueue = create_singlethread_workqueue("nxe2000_charger_in");

	INIT_WORK(&info->irq_work, charger_irq_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->monitor_work, nxe2000_battery_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->displayed_work, nxe2000_displayed_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->charge_stable_work, nxe2000_stable_charge_countdown_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->charge_monitor_work, nxe2000_charge_monitor_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->get_charge_work, nxe2000_get_charge_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->jeita_work, nxe2000_jeita_work);
	INIT_DELAYED_WORK(&info->changed_work, nxe2000_changed_work);
#ifdef KOOK_UBC_CHECK
	INIT_DELAYED_WORK_DEFERRABLE(&info->get_charger_work, nxe2000_get_charger_work);
#endif

#ifdef BAT_RESUME_WORK_QUEUE
	INIT_WORK(&info->resume_work, nxe2000_battery_resume_work);
#else
	/* add by jhkim */
	device_enable_async_suspend(&pdev->dev);
#endif

	/* for SW UBC. */
#if (CFG_SW_UBC_ENABLE == 1)
#if (CFG_USB_DET_FROM_PMIC_INT == 1)
	INIT_DELAYED_WORK_DEFERRABLE(&info->sw_ubc_work, nxe2000_sw_ubc_work);
#else

	if (info->gpio_pmic_vbus > -1) {
		nxp_soc_gpio_set_int_enable(info->gpio_pmic_vbus, 0);

		ret = request_irq(gpio_to_irq(info->gpio_pmic_vbus), sw_ubc_isr,
						IRQ_TYPE_EDGE_BOTH, "sw_ubc_monitor", info);
		if (ret < 0) {
			dev_err(&pdev->dev,
				"Can't get CHG_INT IRQ for chrager: %d\n", ret);
			goto out;
		}
		INIT_DELAYED_WORK_DEFERRABLE(&info->sw_ubc_work, nxe2000_sw_ubc_work);
	}
#endif
#endif

#if 0
	/* Supported for OTG VBUS. */
	if (info->gpio_otg_usbid > -1) {
		nxp_soc_gpio_set_int_enable(info->gpio_otg_usbid, 0);

		ret = request_irq(gpio_to_irq(info->gpio_otg_usbid), otgid_det_isr,
						IRQ_TYPE_EDGE_BOTH, "usb_id", info);
		if (ret < 0) {
			dev_err(&pdev->dev,
				"Can't get CHG_INT IRQ for chrager: %d\n", ret);
			goto out;
		}
		INIT_DELAYED_WORK_DEFERRABLE(&info->otgid_detect_work, otgid_detect_irq_work);
	}
#endif

	/* Charger IRQ workqueue settings */
	charger_irq = pdata->irq;

	ret = request_threaded_irq(charger_irq + NXE2000_IRQ_FONCHGINT,
					NULL, charger_in_isr, IRQF_ONESHOT, "nxe2000_charger_in", info);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"Can't get CHG_INT IRQ for chrager: %d\n", ret);
		goto out;
	}

	if (info->input_power_type != INPUT_POWER_TYPE_ADP_NOBATTERY)
	{
		ret = request_threaded_irq(charger_irq + NXE2000_IRQ_FCHGCMPINT,
						NULL, charger_complete_isr, IRQF_ONESHOT, "nxe2000_charger_comp", info);
		if (ret < 0) {
			dev_err(&pdev->dev,
				"Can't get CHG_COMP IRQ for chrager: %d\n", ret);
			goto out;
		}
	}

	ret = request_threaded_irq(charger_irq + NXE2000_IRQ_FVUSBDETSINT,
					NULL, charger_usb_isr, IRQF_ONESHOT, "nxe2000_usb_det", info);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"Can't get USB_DET IRQ for chrager: %d\n", ret);
		goto out;
	}

	ret = request_threaded_irq(charger_irq + NXE2000_IRQ_FVADPDETSINT,
					NULL, charger_adp_isr, IRQF_ONESHOT, "nxe2000_adp_det", info);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"Can't get ADP_DET IRQ for chrager: %d\n", ret);
		goto out;
	}

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	ret = request_threaded_irq(
				#if defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
					charger_irq + NXE2000_IRQ_VBATLIR,
				#elif defined(ENABLE_LOW_BATTERY_VSYS_DETECTION)
					charger_irq + NXE2000_IRQ_VSYSLIR,
				#endif
					NULL, low_adc_detect_isr, IRQF_ONESHOT, "nxe2000_adc_det", info);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"Can't get ADC IRQ for chrager: %d\n", ret);
		goto out;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&info->low_battery_work, low_battery_irq_work);
#endif

	ret = nxe2000_bulk_reads(info->dev->parent, FA_CAP_H_REG, 2, fa_cap_reg);
	if (ret < 0)
		goto out;

	info->full_capacity = (fa_cap_reg[0] << 8 | fa_cap_reg[1]) & 0x7fff;

	/* Charger init and IRQ setting */
	ret = nxe2000_init_charger(info);
	if (ret < 0)
		goto out;

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
	ret = nxe2000_init_fgsoca(info);
#endif
	queue_delayed_work(info->monitor_wqueue, &info->monitor_work, NXE2000_MONITOR_START_TIME*HZ);

	/* Enable Charger/ADC interrupt */
	nxe2000_set_bits(info->dev->parent, NXE2000_INTC_INTEN, CHG_INT | ADC_INT);

//	nxe2000_power_suspend_status	= 0;
#ifdef KOOK_FAKE_POWER_ON_EVENT
	nxe2000_power_resume_status		= 0;
	nxe2000_power_lowbat			= 0;
#endif

#if 1
	info->ubc_check_count	= 1;
	info->chg_ctr			= 0x03;
	queue_work(info->workqueue, &info->irq_work);
#endif

#if 0 // by hsjung
	if (info->gpio_otg_usbid > -1) {
		queue_delayed_work(info->monitor_wqueue, &info->otgid_detect_work, msecs_to_jiffies(20));
	}
#endif

#if (CFG_SW_UBC_ENABLE == 1)
#if (CFG_USB_DET_FROM_PMIC_INT == 1)
	dwc_otg_pcd_clear_ep0_state();
	info->pmic_vbuschk_count = PMIC_VBUSCHK_COUNT;
	info->flag_set_ilimit = false;
	queue_delayed_work(info->monitor_wqueue, &info->sw_ubc_work, msecs_to_jiffies(1000));
#else

	if (info->gpio_pmic_vbus > -1) {
		if (gpio_get_value(info->gpio_pmic_vbus)) {
			dwc_otg_pcd_clear_ep0_state();
			info->pmic_vbuschk_count = PMIC_VBUSCHK_COUNT;
			info->flag_set_ilimit = false;
			queue_delayed_work(info->monitor_wqueue, &info->sw_ubc_work, msecs_to_jiffies(1000));
		}
	}
#endif
#endif

	if(info->first_soc && info->factory_mode_complete) {
		suspend_charge4first_soc(info);
	}

	return 0;

out:
	kfree(info);
	return ret;
}

static int __devexit nxe2000_battery_remove(struct platform_device *pdev)
{
	struct nxe2000_battery_info *info = platform_get_drvdata(pdev);
	uint8_t val;
	int ret;
	int err;
	int cc_cap = 0;
	bool is_charging = true;

	if (g_fg_on_mode
		 && (info->soca->status == NXE2000_SOCA_STABLE)) {
		err = nxe2000_write(info->dev->parent, PSWR_REG, 0x7f);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");
		g_soc = 0x7f;
	} else if (info->soca->status != NXE2000_SOCA_START
		&& info->soca->status != NXE2000_SOCA_UNSTABLE
		&& info->soca->rsoc_ready_flag != 0)
	{
		if (info->soca->displayed_soc < 50)
		{
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}
		ret = nxe2000_write(info->dev->parent, PSWR_REG, val);
		if (ret < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;

		ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 1);
		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");
	}

	if (g_fg_on_mode == 0) {
		ret = nxe2000_clr_bits(info->dev->parent,
					 FG_CTRL_REG, 0x01);
		if (ret < 0)
			dev_err(info->dev, "Error in clr FG EN\n");
	}

	/* set rapid timer 300 min */
	err = nxe2000_set_bits(info->dev->parent, TIMSET_REG, 0x03);
	if (err < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
	}

	free_irq(charger_irq + NXE2000_IRQ_FONCHGINT, &info);
	if (info->input_power_type != INPUT_POWER_TYPE_ADP_NOBATTERY)
		free_irq(charger_irq + NXE2000_IRQ_FCHGCMPINT, &info);
	free_irq(charger_irq + NXE2000_IRQ_FVUSBDETSINT, &info);
	free_irq(charger_irq + NXE2000_IRQ_FVADPDETSINT, &info);
#if defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	free_irq(charger_irq + NXE2000_IRQ_VBATLIR, &info);
#endif
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION)
	free_irq(charger_irq + NXE2000_IRQ_VSYSLIR, &info);
#endif

	cancel_delayed_work(&info->monitor_work);
	cancel_delayed_work(&info->charge_stable_work);
	cancel_delayed_work(&info->charge_monitor_work);
	cancel_delayed_work(&info->get_charge_work);
	cancel_delayed_work(&info->displayed_work);
	cancel_delayed_work(&info->changed_work);
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	cancel_delayed_work(&info->low_battery_work);
#endif
#ifdef ENABLE_FACTORY_MODE
	cancel_delayed_work(&info->factory_mode_work);
#endif
	cancel_delayed_work(&info->jeita_work);
	cancel_work_sync(&info->irq_work);

	flush_workqueue(info->monitor_wqueue);
	flush_workqueue(info->workqueue);
#ifdef ENABLE_FACTORY_MODE
	flush_workqueue(info->factory_mode_wqueue);
#endif

	destroy_workqueue(info->monitor_wqueue);
	destroy_workqueue(info->workqueue);
#ifdef ENABLE_FACTORY_MODE
	destroy_workqueue(info->factory_mode_wqueue);
#endif

	power_supply_unregister(&info->battery);
	kfree(info);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int nxe2000_battery_suspend(struct device *dev)
{
	struct nxe2000_battery_info *info = dev_get_drvdata(dev);
	uint8_t set_val = 0, val = 0;
	int ret;
	int err;
	int cc_cap = 0;
	bool is_charging = true;
	int displayed_soc_temp;
	int otg_id = 1;

	PM_DBGOUT("PMU: ++ %s\n", __func__);

	if((0 < info->ch_ichg_slp) && (info->ch_ichg_slp <= 0x0E))
	{
		set_val = (info->ch_icchg << NXE2000_POS_CHGISET_ICCHG) + info->ch_ichg_slp;
		nxe2000_write(info->dev->parent, CHGISET_REG, set_val);
	}

#ifdef KOOK_LOW_VOL_DET_TEST    // Detect for critical low voltage.
	if (info->gpio_pmic_lowbat > -1) {
		if (gpio_get_value(info->gpio_pmic_lowbat) == 0) {
			power_supply_changed(&info->battery);
			return -EINVAL;
		}
	}
#endif

//	nxe2000_power_suspend_status    = 1;
#ifdef KOOK_FAKE_POWER_ON_EVENT
	nxe2000_power_resume_status     = 0;
#endif

#if (CFG_SW_UBC_ENABLE == 1) && (CFG_USB_DET_FROM_PMIC_INT == 0)
	if (info->gpio_pmic_vbus > -1) {
		nxp_soc_gpio_set_int_enable(info->gpio_pmic_vbus, 0);
		nxp_soc_gpio_clr_int_pend(info->gpio_pmic_vbus);
	}
#endif

	//if (info->gpio_otg_usbid > -1) 
	//{
	//	otg_id = gpio_get_value(info->gpio_otg_usbid);
		otg_id = info->gpio_otg_usbid_status;
		if ((vbus_irq_disabled == 0) && (otg_id == 0))
		{
			disable_irq(charger_irq + NXE2000_IRQ_FVUSBDETSINT);
			vbus_irq_disabled = 1;
		}
	//}

#if 1
#if 0
	/* OTG POWER OFF */
	if (info->gpio_otg_vbus > -1)
		gpio_set_value(info->gpio_otg_vbus, 0);

	if (otg_id == 0)
	{
		nxe2000_read(info->dev->parent, NXE2000_REG_CHGCTL1, &set_val);
		set_val	&= ~(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
		nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, set_val);
	}
#endif
#else
	if (info->input_power_type == INPUT_POWER_TYPE_ADP)
	{
		val = (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM)
			| (0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN)
			| (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);

		if (otg_id == 0)
			val |= (0x1 << NXE2000_POS_CHGCTL1_SUSPEND);

		nxe2000_write(info->dev->parent, NXE2000_REG_CHGCTL1, val);
	}
	else if (info->input_power_type == INPUT_POWER_TYPE_UBC)
	{
		uint8_t pwr_path;

		val = (0x1 << NXE2000_POS_CHGCTL1_NOBATOVLIM)
			| (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN)
			| (0x1 << NXE2000_POS_CHGCTL1_VADPCHGEN);
		if (otg_id == 0) {
			val |=	(0x1 << NXE2000_POS_CHGCTL1_SUSPEND);
			val |=	(0x1 << NXE2000_POS_CHGCTL1_OTG_BOOST_EN);
		}

		ret = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);
		if (ret < 0)
			ret = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);

		ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &pwr_path);
		if (ret < 0)
			ret = nxe2000_read(info->dev->parent, CHGSTATE_REG, &pwr_path);

		if (pwr_path & 0x80)
		{
			val = (0x1 << NXE2000_POS_CHGCTL1_CHGP)
				| (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
			ret = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);
			if (ret < 0)
				ret = nxe2000_set_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);
		}
		else if (pwr_path & 0x40)
		{
			val = (0x1 << NXE2000_POS_CHGCTL1_CHGP)
				| (0x1 << NXE2000_POS_CHGCTL1_VUSBCHGEN);
			ret = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);
			if (ret < 0)
				ret = nxe2000_clr_bits(info->dev->parent, NXE2000_REG_CHGCTL1, val);
		}
	}
#endif

#ifdef ENABLE_MASKING_INTERRUPT_IN_SLEEP
	nxe2000_clr_bits(dev->parent, NXE2000_INTC_INTEN, CHG_INT);
#endif

	if (g_fg_on_mode
		 && (info->soca->status == NXE2000_SOCA_STABLE)) {
		err = nxe2000_write(info->dev->parent, PSWR_REG, 0x7f);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");
		 g_soc = 0x7F;
		info->soca->suspend_soc = info->soca->displayed_soc;
		ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 1);
		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

		info->soca->suspend_cc = 0;

	} else if (info->soca->status != NXE2000_SOCA_START
		&& info->soca->status != NXE2000_SOCA_UNSTABLE)
	{
		if (info->soca->rsoc_ready_flag == 0) {
			ret = calc_capacity_in_period(info, &cc_cap,
								 &is_charging, 2);
			if (ret < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");
			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			if (info->soca->status != NXE2000_SOCA_STABLE) {
				displayed_soc_temp
					 = info->soca->displayed_soc + (info->soca->cc_delta/100) *100;
			} else {
				displayed_soc_temp
					 = info->soca->init_pswr * 100 + (info->soca->cc_delta/100) *100;
			}

			if ((info->soca->displayed_soc + 50)/100 >= 100) {
				displayed_soc_temp = min(10000, displayed_soc_temp);
			} else {
				displayed_soc_temp = min(9949, displayed_soc_temp);
			}			displayed_soc_temp = MAX(0, displayed_soc_temp);

			info->soca->displayed_soc = displayed_soc_temp;

			info->soca->suspend_soc = info->soca->displayed_soc;
			info->soca->suspend_cc = info->soca->cc_delta % 100;

			val = info->soca->init_pswr + (info->soca->cc_delta/100);
			val = MIN(100, val);
			val = MAX(1, val);

			info->soca->init_pswr = val;

			PM_LOGOUT(KERN_INFO "PMU: %s status(%d), rrf(%d), suspend_soc(%d), suspend_cc(%d)\n",
				 __func__, info->soca->status, info->soca->rsoc_ready_flag, info->soca->suspend_soc, info->soca->suspend_cc);
			PM_LOGOUT(KERN_INFO "PMU: %s DSOC(%d), init_pswr(%d), cc_delta(%d)\n",
				__func__, info->soca->displayed_soc, info->soca->init_pswr, info->soca->cc_delta);
		} else {
			ret = calc_capacity_in_period(info, &cc_cap,
								 &is_charging, 1);
			if (ret < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");
			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			if (info->soca->status != NXE2000_SOCA_STABLE) {
				displayed_soc_temp
					 = info->soca->displayed_soc + info->soca->cc_delta;

				if ((info->soca->displayed_soc + 50)/100 >= 100) {
					displayed_soc_temp = min(10000, displayed_soc_temp);
				} else {
					displayed_soc_temp = min(9949, displayed_soc_temp);
				}

				displayed_soc_temp = MAX(0, displayed_soc_temp);
				info->soca->displayed_soc = displayed_soc_temp;
			}

			info->soca->suspend_soc = info->soca->displayed_soc;
			info->soca->suspend_cc = 0;
			PM_LOGOUT(KERN_INFO "PMU: %s status(%d), rrf(%d), suspend_soc(%d), suspend_cc(%d)\n",
				 __func__, info->soca->status, info->soca->rsoc_ready_flag, info->soca->suspend_soc, info->soca->suspend_cc);
			PM_LOGOUT(KERN_INFO "PMU: %s DSOC(%d), init_pswr(%d), cc_delta(%d)\n",
				__func__, info->soca->displayed_soc, info->soca->init_pswr, info->soca->cc_delta);
		}

		if (info->soca->displayed_soc < 50)
		{
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}
		ret = nxe2000_write(info->dev->parent, PSWR_REG, val);
		if (ret < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;
	} else if ((info->soca->status == NXE2000_SOCA_START)
		|| (info->soca->status == NXE2000_SOCA_UNSTABLE))
	{
		ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 2);
		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");
		info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

		displayed_soc_temp
			 = info->soca->init_pswr *100 + (info->soca->cc_delta/100) * 100;

		if ((info->soca->displayed_soc + 50)/100 >= 100) {
			displayed_soc_temp = min(10000, displayed_soc_temp);
		} else {
			displayed_soc_temp = min(9949, displayed_soc_temp);
		}

		displayed_soc_temp = MAX(0, displayed_soc_temp);
		info->soca->displayed_soc = displayed_soc_temp;

		info->soca->suspend_soc = info->soca->displayed_soc;
		info->soca->suspend_cc = info->soca->cc_delta % 100;

		val = info->soca->init_pswr + (info->soca->cc_delta/100);
		val = MIN(100, val);
		val = MAX(1, val);
		info->soca->init_pswr = val;

		PM_LOGOUT(KERN_INFO "PMU: %s status(%d), rrf(%d), suspend_soc(%d), suspend_cc(%d)\n",
			 __func__, info->soca->status, info->soca->rsoc_ready_flag, info->soca->suspend_soc, info->soca->suspend_cc);
		PM_LOGOUT(KERN_INFO "PMU: %s DSOC(%d), init_pswr(%d), cc_delta(%d)\n",
			__func__, info->soca->displayed_soc, info->soca->init_pswr, info->soca->cc_delta);
	}

	PM_LOGOUT(KERN_INFO "PMU : %s : Status is %d, ready flag is %d, Suspend CC is %d\n",
		__func__, info->soca->status, info->soca->rsoc_ready_flag, info->soca->suspend_cc);


	if (info->soca->status == NXE2000_SOCA_DISP
		|| info->soca->status == NXE2000_SOCA_STABLE
		|| info->soca->status == NXE2000_SOCA_FULL) {
		info->soca->soc = calc_capacity_2(info);
		info->soca->soc_delta =
			info->soca->soc_delta + (info->soca->soc - info->soca->last_soc);

	} else {
		info->soca->soc_delta = 0;
	}

	if (info->soca->status == NXE2000_SOCA_STABLE
		|| info->soca->status == NXE2000_SOCA_FULL)
		info->soca->status = NXE2000_SOCA_DISP;
	/* set rapid timer 300 min */
	err = nxe2000_set_bits(info->dev->parent, TIMSET_REG, 0x03);
	if (err < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
	}

#if 0
	flush_delayed_work(&info->monitor_work);
	flush_delayed_work(&info->displayed_work);
	flush_delayed_work(&info->charge_stable_work);
	flush_delayed_work(&info->charge_monitor_work);
	flush_delayed_work(&info->get_charge_work);
	flush_delayed_work(&info->changed_work);
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	flush_delayed_work(&info->low_battery_work);
#endif
#ifdef ENABLE_FACTORY_MODE
	flush_delayed_work(&info->factory_mode_work);
#endif
	flush_delayed_work(&info->jeita_work);
/*	flush_work(&info->irq_work); */
#if (CFG_SW_UBC_ENABLE == 1)
#if (CFG_USB_DET_FROM_PMIC_INT == 0)
	if (info->gpio_pmic_vbus > -1)
#endif
		flush_delayed_work(&info->sw_ubc_work);
#endif
#else

	cancel_delayed_work(&info->monitor_work);
	cancel_delayed_work(&info->displayed_work);
	cancel_delayed_work(&info->charge_stable_work);
	cancel_delayed_work(&info->charge_monitor_work);
	cancel_delayed_work(&info->get_charge_work);
	cancel_delayed_work(&info->changed_work);
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	cancel_delayed_work(&info->low_battery_work);
#endif
#ifdef ENABLE_FACTORY_MODE
	cancel_delayed_work(&info->factory_mode_work);
#endif
	cancel_delayed_work(&info->jeita_work);
/*	flush_work(&info->irq_work); */
#if (CFG_SW_UBC_ENABLE == 1)
#if (CFG_USB_DET_FROM_PMIC_INT == 0)
	if (info->gpio_pmic_vbus > -1)
#endif
		cancel_delayed_work(&info->sw_ubc_work);
#endif
#endif

#if 0	//(CFG_SW_UBC_ENABLE == 0)
	#ifdef CONFIG_SUSPEND_IDLE
	ret = nxe2000_clr_bits(info->dev->parent, 0x91, 0x10);    // GPIO4 : Low
	if (ret < 0)
		ret = nxe2000_clr_bits(info->dev->parent, 0x91, 0x10);
	#else
	ret = nxe2000_clr_bits(info->dev->parent, 0x90, 0x10);    // GPIO4 : Input
	if (ret < 0)
		ret = nxe2000_clr_bits(info->dev->parent, 0x90, 0x10);
	#endif
#endif


#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)

	if(info->low_bat_power_off == true)
		return -EBUSY;

	val = 0x0;
#if defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	/* Enable VBAT threshold Low interrupt */
	val |= 0x02;
#endif
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION)
	/* Enable VSYS threshold Low interrupt */
	val |= 0x10;
#endif

	/*  VBAT/VSYS threshold low voltage value */
	nxe2000_write(info->dev->parent, NXE2000_ADC_VBAT_THL, 0x00);
	nxe2000_write(info->dev->parent, NXE2000_ADC_VSYS_THL, 0x00);

	/* Set ADRQ=00 to stop ADC */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x00);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x00);

	/* Set ADC auto conversion interval 16s */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT2, 0x6);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT2, 0x6);

	/* Enable VBAT/VSYS pin conversion in auto-ADC */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT1, 0x12);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT1, 0x12);

	/* Enable VBAT/VSYS threshold Low interrupt */
	ret = nxe2000_write(info->dev->parent, NXE2000_INT_EN_ADC1, val);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_INT_EN_ADC1, val);

	/* Cleaning ADC interrupt */
	ret = nxe2000_write(info->dev->parent, NXE2000_INT_IR_ADCL, 0x00);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_INT_IR_ADCL, 0x00);

	/* Start auto-mode & average 4-time conversion mode for ADC */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x28);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x28);

#if defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)
	mdelay(10);
	nxe2000_write(info->dev->parent, NXE2000_ADC_VBAT_THL, info->low_vbat_thl);
#endif
#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION)
	mdelay(10);
	nxe2000_write(info->dev->parent, NXE2000_ADC_VSYS_THL, info->low_vsys_thl);
#endif
#endif

	PM_DBGOUT("PMU: -- %s\n", __func__);

#ifdef NXE2000_REG_DUMP
	nxe2000_register_dump(info->dev->parent);
#endif

	return 0;
}

#ifdef BAT_RESUME_WORK_QUEUE
static int nxe2000_battery_resume(struct device *dev)
{
	struct nxe2000_battery_info *info = dev_get_drvdata(dev);
	int ret;

#if 0	//(CFG_SW_UBC_ENABLE == 0)
	#ifdef CONFIG_SUSPEND_IDLE
	ret = nxe2000_set_bits(info->dev->parent, 0x91, 0x10);    // GPIO4 : High
	if (ret < 0)
		ret = nxe2000_set_bits(info->dev->parent, 0x91, 0x10);
	#else
	ret = nxe2000_set_bits(info->dev->parent, 0x90, 0x10);    // GPIO4 : Output
	if (ret < 0)
		ret = nxe2000_set_bits(info->dev->parent, 0x90, 0x10);
	#endif
#endif

	schedule_work(&info->resume_work);
	return ret;
}

static void nxe2000_battery_resume_work(struct work_struct *work) {
	struct nxe2000_battery_info *info = container_of(work,
				struct nxe2000_battery_info, resume_work);
#else
static int nxe2000_battery_resume(struct device *dev) {
	struct nxe2000_battery_info *info = dev_get_drvdata(dev);
#endif
	uint8_t val;
	int ret;
	int displayed_soc_temp;
	int cc_cap = 0;
	bool is_charging = true;
	bool is_jeita_updated;
	int i;
	int OCV_now;
	int soc_now;
	int cc_now;
	int cc_suspend_term;
	int cc_delta_offset;
	int cc_correct_value;

	PM_DBGOUT("PMU: ++ %s\n", __func__);

#if defined(ENABLE_LOW_BATTERY_VSYS_DETECTION) || defined(ENABLE_LOW_BATTERY_VBAT_DETECTION)

	/*  VBAT/VSYS threshold low voltage value */
	nxe2000_write(info->dev->parent, NXE2000_ADC_VBAT_THL, 0x00);
	nxe2000_write(info->dev->parent, NXE2000_ADC_VSYS_THL, 0x00);

	/* Set ADRQ=00 to stop ADC */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x0);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x0);

	/* Set ADC auto conversion interval 250ms */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT2, 0x0);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT2, 0x0);

	/* Enable VBAT/VSYS pin conversion in auto-ADC */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT1, 0x12);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT1, 0x12);

	/* Disable VBAT/VSYS threshold Low interrupt */
	ret = nxe2000_write(info->dev->parent, NXE2000_INT_EN_ADC1, 0x0);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_INT_EN_ADC1, 0x0);

	/* Cleaning ADC interrupt */
	ret = nxe2000_write(info->dev->parent, NXE2000_INT_IR_ADCL, 0);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_INT_IR_ADCL, 0);

	/* Start auto-mode & average 4-time conversion mode for ADC */
	ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x28);
	if (ret < 0)
		ret = nxe2000_write(info->dev->parent, NXE2000_ADC_CNT3, 0x28);
#endif

#ifdef KOOK_FAKE_POWER_ON_EVENT
	if (info->gpio_pmic_vbus > -1) {
		if (gpio_get_value(info->gpio_pmic_vbus)) {
			nxe2000_power_resume_status	= 1;
			nxe2000_power_lowbat		= 2;
		}
	}
#endif

	if((0 < info->ch_ichg_slp) && (info->ch_ichg_slp <= 0x0E))
	{
		val = (info->ch_icchg << NXE2000_POS_CHGISET_ICCHG) + info->ch_ichg;
		nxe2000_write(info->dev->parent, CHGISET_REG, val);
	}

#ifdef ENABLE_MASKING_INTERRUPT_IN_SLEEP
	nxe2000_set_bits(dev->parent, NXE2000_INTC_INTEN, CHG_INT);
#endif
	ret = check_jeita_status(info, &is_jeita_updated);
	if (ret < 0) {
		dev_err(info->dev, "Error in updating JEITA %d\n", ret);
	}

	//calc RSOC_now from OCV table
	//unit of RSOC is 0.01%
	OCV_now = calc_ocv(info);
	soc_now = calc_soc_on_ocv(info, OCV_now);

	if (info->entry_factory_mode) {
		info->soca->displayed_soc = -EINVAL;
	} else if (NXE2000_SOCA_ZERO == info->soca->status) {
		if (calc_ocv(info) > get_OCV_voltage(info, 0)) {
			info->soca->soc = info->soca->init_pswr * 100;
			if (ret < 0) {
				dev_err(info->dev,
					 "Error in reading PSWR_REG %d\n", ret);
				info->soca->soc
					 = calc_capacity(info) * 100;
			}

			ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 1);
			if (ret < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");

			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			displayed_soc_temp
				 = info->soca->soc + info->soca->cc_delta;

			if (info->soca->zero_flg == 1) {
				if((info->soca->Ibat_ave >= 0) 
				|| (displayed_soc_temp >= 100)){
					info->soca->zero_flg = 0;
				} else {
					displayed_soc_temp = 0;
				}
			} else if (displayed_soc_temp < 100) {
				/* keep DSOC = 1 when Vbat is over 3.4V*/
				if(info->fg_poff_vbat != 0) {
					if (info->soca->Vbat_ave < 2000*1000) { /* error value */
						displayed_soc_temp = 100;
					} else if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
						displayed_soc_temp = 0;
						info->soca->zero_flg = 1;
					} else {
						displayed_soc_temp = 100;
					}
				}
			}

			displayed_soc_temp = MIN(10000, displayed_soc_temp);
			displayed_soc_temp = MAX(0, displayed_soc_temp);
			info->soca->displayed_soc = displayed_soc_temp;

			ret = reset_FG_process(info);

			if (ret < 0)
				dev_err(info->dev, "Error in writing the control register\n");
			info->soca->status = NXE2000_SOCA_FG_RESET;

		} else
			info->soca->displayed_soc = 0;
	} else {
		info->soca->soc = info->soca->suspend_soc + info->soca->suspend_cc;

		if (NXE2000_SOCA_START == info->soca->status
			|| NXE2000_SOCA_UNSTABLE == info->soca->status
			|| info->soca->rsoc_ready_flag == 0)
		{
			ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 2);
			if (ret < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");
			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			if (NXE2000_SOCA_START == info->soca->status
				|| NXE2000_SOCA_UNSTABLE == info->soca->status
				|| NXE2000_SOCA_STABLE == info->soca->status)
				cc_now = info->soca->cc_delta;
			else
				cc_now = (info->soca->cc_delta/100) * 100;
		} else {
			ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, 1);
			if (ret < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");
			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			cc_now = info->soca->cc_delta;
		}

		//calc cc_offset
		cc_suspend_term	= cc_now - info->soca->suspend_cc;

		if(cc_suspend_term < 0){ //When discharging
			cc_delta_offset = 100;	//unit is 1/100times
		} else {
			if( (soc_now - cc_suspend_term) >= 100*100 ){//remove condition divide by Zero
				cc_delta_offset = 5 * 100; //max 5 times
			} else {
				cc_delta_offset = (100 * 100 - info->soca->soc) * 100 / (100 * 100 -  (soc_now - cc_suspend_term));

				cc_delta_offset = MIN(5 * 100, cc_delta_offset); //MAX is 5 times
				cc_delta_offset = MAX(100 / 5, cc_delta_offset); //MIN is 1/5 times
			}
		}

		/*unit of cc_delta_offset is 0.01 times*/
		displayed_soc_temp = info->soca->soc + cc_suspend_term * cc_delta_offset / 100;

		PM_LOGOUT(KERN_INFO "PMU: %s : cc_now is %d per, suspend CC is %d, CC_suspend_term is %d per, cc_delta offset is %d times\n", __func__, cc_now, info->soca->suspend_cc, cc_suspend_term, cc_delta_offset);
		PM_LOGOUT(KERN_INFO "PMU: %s : soc temp is %d per, sus_soc+sus_cc is %d per soc_now is %d\n\n", __func__, displayed_soc_temp, info->soca->soc, soc_now);
		PM_LOGOUT(KERN_INFO "PMU: %s : status is %d rsoc_ready_flag is %d \n\n", __func__, info->soca->status, info->soca->rsoc_ready_flag);

		if (NXE2000_SOCA_START == info->soca->status
			|| NXE2000_SOCA_UNSTABLE == info->soca->status
			|| info->soca->rsoc_ready_flag == 0)
		{
			cc_correct_value = cc_suspend_term * cc_delta_offset / 100 - cc_suspend_term;

			val = info->soca->init_pswr;
			val &= 0x7f;

			PM_LOGOUT(KERN_INFO "PMU: %s : before init_PSWR %d per\n", __func__, val);

			val += (info->soca->cc_delta/100);
			val += cc_correct_value / 100;

			val = MIN(99, val);
			val = MAX(1, val);

			info->soca->init_pswr = val;

			displayed_soc_temp = displayed_soc_temp - ( cc_correct_value - (cc_correct_value / 100 * 100));

			PM_LOGOUT(KERN_INFO "PMU: %s : soc temp is %d per, cc_correct is %d\n",
				 __func__, displayed_soc_temp, cc_correct_value);
			PM_LOGOUT(KERN_INFO "PMU: %s : after PSWR %d per\n", __func__, val);
		}

		if (info->soca->zero_flg == 1) {
			if((info->soca->Ibat_ave >= 0)
			|| (displayed_soc_temp >= 100)){
				info->soca->zero_flg = 0;
			} else {
				displayed_soc_temp = 0;
			}
		} else if (displayed_soc_temp < 100) {
			/* keep DSOC = 1 when Vbat is over 3.4V*/
			if( info->fg_poff_vbat != 0) {
				if (info->soca->Vbat_ave < 2000*1000) { /* error value */
					displayed_soc_temp = 100;
				} else if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
					displayed_soc_temp = 0;
					info->soca->zero_flg = 1;
				} else {
					displayed_soc_temp = 100;
				}
			}
		}
		displayed_soc_temp = MIN(10000, displayed_soc_temp);
		displayed_soc_temp = MAX(0, displayed_soc_temp);

		if (0 == info->soca->jt_limit) {
			check_charge_status_2(info, displayed_soc_temp);
		} else {
			info->soca->displayed_soc = displayed_soc_temp;
		}

		if (NXE2000_SOCA_DISP == info->soca->status) {
			info->soca->last_soc = calc_capacity_2(info);
		}
	}

	if ((info->soca->displayed_soc + 50)/100 <= 1) {
		val = 1;
	} else {
		val = (info->soca->displayed_soc + 50)/100;
		val &= 0x7f;
	}
	ret = nxe2000_write(info->dev->parent, PSWR_REG, val);
	if (ret < 0)
		dev_err(info->dev, "Error in writing PSWR_REG\n");

	g_soc = val;

	ret = measure_vbatt_FG(info, &info->soca->Vbat_ave);
	ret = measure_vsys_ADC(info, &info->soca->Vsys_ave);
	ret = measure_Ibatt_FG(info, &info->soca->Ibat_ave);

	power_supply_changed(&info->battery);

	queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
						 NXE2000_FG_RESET_TIME * HZ);

	if (NXE2000_SOCA_UNSTABLE == info->soca->status) {
		info->soca->stable_count = 10;
		queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 NXE2000_FG_STABLE_TIME*HZ/10);
	} else if (NXE2000_SOCA_FG_RESET == info->soca->status) {
		info->soca->stable_count = 1;

		for (i = 0; i < 3; i = i+1)
			info->soca->reset_soc[i] = 0;
		info->soca->reset_count = 0;

		queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 NXE2000_FG_RESET_TIME*HZ);
	}

	queue_delayed_work(info->monitor_wqueue, &info->monitor_work,
					 info->monitor_time * HZ);

	queue_delayed_work(info->monitor_wqueue, &info->charge_monitor_work,
					 NXE2000_CHARGE_RESUME_TIME * HZ);

	if (info->first_soc == 0) {
		info->soca->chg_count = 0;
	}

	queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 NXE2000_CHARGE_RESUME_TIME * HZ);

	if (info->jt_en) {
		if (!info->jt_hw_sw) {
			queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
					 NXE2000_JEITA_UPDATE_TIME * HZ);
		}
	}

	//if (info->gpio_otg_usbid > -1) 
	{
		if (vbus_irq_disabled == 1)
		{
			enable_irq(charger_irq + NXE2000_IRQ_FVUSBDETSINT);
			vbus_irq_disabled = 0;
		}
	}

//	nxe2000_power_suspend_status    = 0;

	info->ubc_check_count	= 1;
	info->chg_ctr			= 0x03;
	queue_work(info->workqueue, &info->irq_work);
#if 0 // by hsjung
	if (info->gpio_otg_usbid > -1) {
		queue_delayed_work(info->monitor_wqueue, &info->otgid_detect_work, msecs_to_jiffies(20));
	}
#endif
#if (CFG_SW_UBC_ENABLE == 1)
#if (CFG_USB_DET_FROM_PMIC_INT == 1)
	dwc_otg_pcd_clear_ep0_state();
	info->pmic_vbuschk_count = PMIC_VBUSCHK_COUNT;
	info->flag_set_ilimit = false;
	queue_delayed_work(info->monitor_wqueue, &info->sw_ubc_work, msecs_to_jiffies(1000));
#else
	if (info->gpio_pmic_vbus > -1) {
		dwc_otg_pcd_clear_ep0_state();
		info->pmic_vbuschk_count = PMIC_VBUSCHK_COUNT;
		info->flag_set_ilimit = false;
		queue_delayed_work(info->monitor_wqueue, &info->sw_ubc_work, msecs_to_jiffies(1000));

		nxp_soc_gpio_set_int_enable(info->gpio_pmic_vbus, 1);
	}
#endif
#endif

	PM_DBGOUT("PMU: -- %s\n", __func__);

#ifndef BAT_RESUME_WORK_QUEUE
	return 0;
#endif
}

static const struct dev_pm_ops nxe2000_battery_pm_ops = {
	.suspend	= nxe2000_battery_suspend,
	.resume		= nxe2000_battery_resume,
};
#endif

static struct platform_driver nxe2000_battery_driver = {
	.driver	= {
				.name	= "nxe2000-battery",
				.owner	= THIS_MODULE,
#ifdef CONFIG_PM
				.pm	= &nxe2000_battery_pm_ops,
#endif
	},
	.probe	= nxe2000_battery_probe,
	.remove	= __devexit_p(nxe2000_battery_remove),
};

static int __init nxe2000_battery_init(void)
{
	return platform_driver_register(&nxe2000_battery_driver);
}
subsys_initcall(nxe2000_battery_init);

static void __exit nxe2000_battery_exit(void)
{
	platform_driver_unregister(&nxe2000_battery_driver);
}
module_exit(nxe2000_battery_exit);

MODULE_DESCRIPTION("NEXELL NXE2000 Battery driver");
MODULE_ALIAS("platform:nxe2000-battery");
MODULE_LICENSE("GPL");
