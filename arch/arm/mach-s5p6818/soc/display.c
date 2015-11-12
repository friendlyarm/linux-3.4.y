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
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>
#include <mach/fourcc.h>

#include "display_proto.h"

#if (0)
#define DBGOUT(msg...)		do { printk(KERN_INFO msg); } while (0)
#else
#define DBGOUT(msg...)		do {} while (0)
#endif

/* 12345'678'[8] -> 12345 [5], 123456'78'[8] -> 123456[6] */
static inline u_short R8G8B8toR5G6B5(unsigned int RGB)
{
	u_char	R = (u_char)((RGB>>16) & 0xff);
	u_char	G = (u_char)((RGB>>8 ) & 0xff);
	u_char	B = (u_char)((RGB>>0 ) & 0xff);
	u_short R5G6B5 = ((R & 0xF8)<<8) | ((G & 0xFC)<<3) | ((B & 0xF8)>>3);
	DBGOUT(" RGB888:0x%08x -> RGB565:0x%08x\n", RGB, R5G6B5);

	return R5G6B5;
}

/* 12345 [5] -> 12345'123'[8], 123456[6] -> 123456'12'[8] */
static inline unsigned int R5G6B5toR8G8B8(u_short RGB)
{
	u_char R5  = (RGB >> 11) & 0x1f;
	u_char G6  = (RGB >> 5 ) & 0x3f;
	u_char B5  = (RGB >> 0 ) & 0x1f;
	u_char R8  = ((R5 << 3) & 0xf8) | ((R5 >> 2) & 0x7);
	u_char G8  = ((G6 << 2) & 0xfc) | ((G6 >> 4) & 0x3);
	u_char B8  = ((B5 << 3) & 0xf8) | ((B5 >> 2) & 0x7);
	unsigned int R8B8G8 = (R8 << 16) | (G8 << 8) | (B8);
	DBGOUT(" RGB565:0x%08x -> RGB888:0x%08x\n", RGB, R8B8G8);

	return R8B8G8;
}

/* 123'45678'[8] -> 123[3], 12'345678'[8] -> 12 [2] */
static inline u_char R8G8B8toR3G3B2(unsigned int RGB)
{
	u_char	R = (u_char)((RGB>>16) & 0xff);
	u_char	G = (u_char)((RGB>>8 ) & 0xff);
	u_char	B = (u_char)((RGB>>0 ) & 0xff);
	u_char R3G3B2 = ( (R & 0xE0) | ((G & 0xE0)>>3) | ((B & 0xC0)>>6) );
	DBGOUT(" RGB888:0x%08x -> RGB332:0x%08x\n", RGB, R3G3B2);

	return R3G3B2;
}

/* 123[3] -> 123'123'12' [8], 12 [2] -> 12'12'12'12'[8] */
static inline unsigned int R3G3B2toR8G8B8(u_char RGB)
{
	u_char R3  = (RGB >> 5) & 0x7;
	u_char G3  = (RGB >> 2) & 0x7;
	u_char B2  = (RGB >> 0) & 0x3;
	u_char R8  = ((R3 << 5) | (R3 << 2) | (R3 >> 1) );
	u_char G8  = ((G3 << 5) | (G3 << 2) | (G3 >> 1) );
	u_char B8  = ((B2 << 6) | (B2 << 4) | (B2 << 2) | B2 );
	unsigned int R8B8G8 = (R8 << 16) | (G8 << 8) | (B8);
	DBGOUT(" RGB332:0x%08x -> RGB888:0x%08x\n", RGB, R8B8G8);

	return R8B8G8;
}

/* For math func */
#define	PI 						3.141592
#define	DEGREE_RADIAN(dg)		(dg*PI/180)

static inline double ksin(double radian)
{
	int    i = 1;
	double d = 0.0;
	double s = radian;
	while (1) {
		d += s;
		s *= -radian * radian/(2 * i * (2 * i + 1));
		i++;
		if (s < 0.000000000000001 && s > -0.000000000000001)
			break;
	}
	return d;
}

static inline double kcos(double radian)
{
	int    i = 1;
	double d = 0.0;
	double s = 1.0;
	while (1) {
		d += s;
		s *= -radian * radian/(2 * i * (2 * i - 1));
		i++;
		if (s < 0.000000000000001 && s > -0.000000000000001)
			break;
	}
	return d;
}

/*
 */

#define DEF_MLC_INTERLACE		CFALSE
#define DEF_OUT_FORMAT			DPC_FORMAT_RGB888
#define DEF_OUT_INVERT_FIELD	CFALSE
#define DEF_OUT_SWAPRB			CFALSE
#define DEF_OUT_YCORDER			DPC_YCORDER_CbYCrY
#define DEF_PADCLKSEL			DPC_PADCLKSEL_VCLK
#define DEF_CLKGEN0_DELAY		0
#define DEF_CLKGEN0_INVERT		0
#define DEF_CLKGEN1_DELAY		0
#define DEF_CLKGEN1_INVERT		0
#define DEF_CLKSEL1_SELECT		0

struct disp_control_info {
	int 	module;							/* 0: primary, 1: secondary */
	int		irqno;
	int 	active_notify;
	int		cond_notify;
	unsigned int 		condition;
	struct kobject   * 	kobj;
	struct work_struct 	work;
	unsigned int 	  	wait_time;
	wait_queue_head_t 	wait_queue;
	ktime_t 	      	time_stamp;
	long	 	      	fps;	/* ms */
	unsigned int	  	status;
	struct list_head 	link;
	struct lcd_operation    *lcd_ops;		/* LCD and Backlight */
	struct disp_multily_dev  multilayer;
	struct disp_process_dev	*proc_dev;
#if 0
	void (*callback)(void *);
	void  *callback_data;
#else
    spinlock_t lock_callback;
    struct list_head callback_list;
#endif
};

static LIST_HEAD(disp_resconv_link);
static struct disp_control_info	*display_info[NUMBER_OF_DPC_MODULE] = { NULL, };
static struct NX_MLC_RegisterSet save_multily[NUMBER_OF_DPC_MODULE];
static struct NX_DPC_RegisterSet save_syncgen[NUMBER_OF_DPC_MODULE];

/*
 * display device array
 */
#define	LIST_INIT(x) 	LIST_HEAD_INIT(device_dev[x].list)
#define	LOCK_INIT(x) 	(__SPIN_LOCK_UNLOCKED(device_dev[x].lock))

static struct disp_process_dev device_dev[] = {
	[0] = { .dev_id = DISP_DEVICE_RESCONV , .name = "RESCONV" , .list = LIST_INIT(0), .lock = LOCK_INIT(0)},
	[1] = { .dev_id = DISP_DEVICE_LCD     ,	.name = "LCD" 	  , .list = LIST_INIT(1), .lock = LOCK_INIT(1)},
	[2] = { .dev_id = DISP_DEVICE_HDMI    ,	.name = "HDMI"    , .list = LIST_INIT(2), .lock = LOCK_INIT(2)},
	[3] = { .dev_id = DISP_DEVICE_MIPI    ,	.name = "MiPi"    , .list = LIST_INIT(3), .lock = LOCK_INIT(3)},
	[4] = { .dev_id = DISP_DEVICE_LVDS    ,	.name = "LVDS"    , .list = LIST_INIT(4), .lock = LOCK_INIT(4)},
	[5] = { .dev_id = DISP_DEVICE_TVOUT   ,	.name = "TVOUT"   , .list = LIST_INIT(5), .lock = LOCK_INIT(5)},
	[6] = { .dev_id = DISP_DEVICE_SYNCGEN0, .name = "SYNCGEN0", .list = LIST_INIT(6), .lock = LOCK_INIT(6)},
	[7] = { .dev_id = DISP_DEVICE_SYNCGEN1, .name = "SYNCGEN1", .list = LIST_INIT(7), .lock = LOCK_INIT(7)},
};
#define DEVICE_SIZE	 ARRAY_SIZE(device_dev)

static struct kobject *kobj_syncgen = NULL;

static inline void *get_display_ptr(enum disp_dev_type device)
{
	return (&device_dev[device]);
}

const char * dev_to_str(enum disp_dev_type device)
{
	struct disp_process_dev *pdev = get_display_ptr(device);
	return pdev->name;
}

#define	get_display_ops(d)		(device_dev[d].disp_ops)
#define	set_display_ops(d, op)	{ if (op) { device_dev[d].disp_ops  = op; op->dev = &device_dev[d]; } }

static inline void *get_module_to_info(int module)
{
	struct disp_control_info *info = display_info[module];
	RET_ASSERT_VAL(module == 0 || module == 1, NULL);
	RET_ASSERT_VAL(info, NULL);
	return info;
}

static inline void *get_device_to_info(struct disp_process_dev *pdev)
{
	return (void*)pdev->dev_info;
}

#define	DISP_CONTROL_INFO(m, in)		\
	struct disp_control_info *in = get_module_to_info(m)

#define	DISP_MULTILY_DEV(m, d)		\
	DISP_CONTROL_INFO(m, info);							\
	struct disp_multily_dev *d = &info->multilayer

#define	DISP_MULTILY_RGB(m, r, l)		\
	DISP_CONTROL_INFO(m, info);							\
	struct disp_multily_dev *pmly = &info->multilayer;	\
	struct mlc_layer_info *r = &pmly->layer[l]

#define	DISP_MULTILY_VID(m, v)		\
	DISP_CONTROL_INFO(m, info);							\
	struct disp_multily_dev *pmly = &info->multilayer;	\
	struct mlc_layer_info *v = &pmly->layer[LAYER_VIDEO_IDX]

#define	DISP_WAIT_POLL_VSYNC(m, w)		\
	do { mdelay(1);	} while (w-- >0 && !NX_DPC_GetInterruptPendingAll(m))

static inline void set_display_kobj(struct kobject * kobj) { kobj_syncgen = kobj; }
static inline struct kobject *get_display_kobj(enum disp_dev_type device) { return kobj_syncgen; }

static inline void disp_topctl_reset(void)
{
	/* RESET: DISPLAYTOP = Low active ___|--- */
	nxp_soc_peri_reset_set(RESET_ID_DISP_TOP);
}

static inline void disp_syncgen_reset(void)
{
	nxp_soc_peri_reset_set(RESET_ID_DISPLAY);
}

static inline void video_alpha_blend(int module, int depth, bool on)
{
	if (depth <= 0 ) depth = 0;
	if (depth >= 15) depth = 15;

	NX_MLC_SetAlphaBlending(module, MLC_LAYER_VIDEO, (on?CTRUE:CFALSE), depth);
	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

static inline void video_lu_color(int module, int bright, int contra)
{
	if (contra >=    7) contra =    7;
	if (contra <=    0)	contra =    0;
	if (bright >=  127) bright =  127;
	if (bright <= -128) bright = -128;

	NX_MLC_SetVideoLayerLumaEnhance(module, (unsigned int)contra, (S32)bright);
	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

static inline void video_cbr_color(int module, int cba, int cbb, int cra, int crb)
{
	int i;
	if (cba > 127) cba=127; if (cba <= -128) cba=-128;
	if (cbb > 127) cbb=127; if (cbb <= -128) cbb=-128;
	if (cra > 127) cra=127; if (cra <= -128) cra=-128;
	if (crb > 127) crb=127; if (crb <= -128) crb=-128;

	for (i=1; 5 > i; i++)
		NX_MLC_SetVideoLayerChromaEnhance(module, i, cba, cbb, cra, crb);

	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

#define	SET_PARAM(s, d, member)	\
	if(s->member && s->member != d->member)	\
		d->member = s->member;

static inline void set_syncgen_param(struct disp_syncgen_par *src, struct disp_syncgen_par *dst)
{
	if (!src || !dst)
		return;

	SET_PARAM(src, dst, interlace);
	SET_PARAM(src, dst, out_format);
	SET_PARAM(src, dst, invert_field);
	SET_PARAM(src, dst, swap_RB);
	SET_PARAM(src, dst, yc_order);
	SET_PARAM(src, dst, delay_mask);
	SET_PARAM(src, dst, d_rgb_pvd);
	SET_PARAM(src, dst, d_hsync_cp1);
	SET_PARAM(src, dst, d_vsync_fram);
	SET_PARAM(src, dst, d_de_cp2);
	SET_PARAM(src, dst, vs_start_offset);
	SET_PARAM(src, dst, vs_end_offset);
	SET_PARAM(src, dst, ev_start_offset);
	SET_PARAM(src, dst, ev_end_offset);
	SET_PARAM(src, dst, vclk_select);
	SET_PARAM(src, dst, clk_inv_lv0);
	SET_PARAM(src, dst, clk_delay_lv0);
	SET_PARAM(src, dst, clk_inv_lv1);
	SET_PARAM(src, dst, clk_delay_lv1);
    SET_PARAM(src, dst, clk_sel_div1);
}

static int display_framerate_jiffies(int module, struct disp_vsync_info *psync)
{
	unsigned long hpix, vpix, pixclk;
	long rate_jiffies;
	long fps, rate;
	struct clk *clk = NULL;
	char name[32] = {0,};

	sprintf(name, "pll%d", psync->clk_src_lv0);
	clk = clk_get(NULL, name);

	/* clock divider align */
	if (1 != psync->clk_div_lv0 && (psync->clk_div_lv0&0x1))
		psync->clk_div_lv0 += 1;

	if (1 != psync->clk_div_lv1 && (psync->clk_div_lv1&0x1))
		psync->clk_div_lv1 += 1;

	/* get pixel clock */
	psync->pixel_clock_hz = ((clk_get_rate(clk)/psync->clk_div_lv0)/psync->clk_div_lv1);

	hpix = psync->h_active_len + psync->h_front_porch +
			psync->h_back_porch + psync->h_sync_width;
	vpix = psync->v_active_len + psync->v_front_porch +
			psync->v_back_porch + psync->v_sync_width;
	pixclk = psync->pixel_clock_hz;

	DBGOUT(" pixel clock   =%ld\n", pixclk);
	DBGOUT(" pixel horizon =%4ld (x=%4d, hfp=%3d, hbp=%3d, hsw=%3d)\n",
		hpix, psync->h_active_len, psync->h_front_porch,
		psync->h_back_porch, psync->h_sync_width);
	DBGOUT(" pixel vertical=%4ld (y=%4d, vfp=%3d, vbp=%3d, vsw=%3d)\n",
		vpix, psync->v_active_len, psync->v_front_porch,
		psync->v_back_porch, psync->v_sync_width);

	fps	 = pixclk ? ((pixclk/hpix)/vpix) : 60;
	fps  = fps ? fps : 60;
	rate = 1000/fps;
	rate_jiffies = msecs_to_jiffies(rate) + 2;	/* +1 jiffies */

	printk("Display.%d fps=%2ld (%ld ms), wait=%2ld jiffies, Pixelclk=%ldhz\n",
		module, fps, rate, rate_jiffies, pixclk);

	return rate_jiffies;
}

static int disp_multily_enable(struct disp_control_info *info, int enable)
{
	struct disp_process_dev *pdev = info->proc_dev;
	struct disp_vsync_info *psync = &pdev->vsync;
	struct disp_multily_dev *pmly = &info->multilayer;
	int module = info->module, i = 0;

	if (enable) {
		struct mlc_layer_info *layer = pmly->layer;
		int xresol, yresol;
		int interlace, lock_size;
		int i = 0;

		pmly->x_resol = psync->h_active_len;
		pmly->y_resol = psync->v_active_len;
		pmly->interlace = psync->interlace;

		xresol = pmly->x_resol;
		yresol = pmly->y_resol;
		interlace = pmly->interlace;
		lock_size = pmly->mem_lock_len;

		NX_MLC_SetFieldEnable(module, interlace);
		NX_MLC_SetScreenSize(module, xresol, yresol);
		NX_MLC_SetRGBLayerGamaTablePowerMode(module, CFALSE, CFALSE, CFALSE);
		NX_MLC_SetRGBLayerGamaTableSleepMode(module, CTRUE, CTRUE, CTRUE);
		NX_MLC_SetRGBLayerGammaEnable(module, CFALSE);
		NX_MLC_SetDitherEnableWhenUsingGamma(module, CFALSE);
		NX_MLC_SetGammaPriority(module, CFALSE);
    	NX_MLC_SetTopPowerMode(module, CTRUE);
    	NX_MLC_SetTopSleepMode(module, CFALSE);
		NX_MLC_SetMLCEnable(module, CTRUE);

		/* restore layer enable status */
		for (i = 0; MULTI_LAYER_NUM > i; i++, layer++) {
			NX_MLC_SetLockSize(module, i, lock_size);
			NX_MLC_SetLayerEnable(module, i, layer->enable ? CTRUE : CFALSE);
			NX_MLC_SetDirtyFlag(module, i);
			DBGOUT("%s: %s, %s\n", __func__, layer->name, layer->enable?"ON":"OFF");
		}
		NX_MLC_SetTopDirtyFlag(module);
		pmly->enable = enable;

	} else {
		for (i = 0; MULTI_LAYER_NUM > i ; i++) {
			NX_MLC_SetLayerEnable(module, i, CFALSE);
			NX_MLC_SetDirtyFlag(module, i);
		}
    	NX_MLC_SetTopPowerMode(module, CFALSE);
    	NX_MLC_SetTopSleepMode(module, CTRUE);
		NX_MLC_SetMLCEnable(module, CFALSE);
		NX_MLC_SetTopDirtyFlag(module);
		pmly->enable = enable;
	}
	return 0;
}

static int disp_syncgen_waitsync(int module, int layer, int waitvsync)
{
	DISP_CONTROL_INFO(module, info);
	int ret = -1;

	if (waitvsync && NX_DPC_GetDPCEnable(module)) {
		info->condition = 0;
		/*
		 * wait_event_timeout may be loss sync evnet, so use
		 * wait_event_interruptible_timeout
		 */
		ret = wait_event_interruptible_timeout(info->wait_queue,
					info->condition, info->wait_time);
		if (0 == info->condition)
			printk(KERN_ERR "Fail, wait vsync %d, time:%s, condition:%d\n",
				module, !ret?"out":"remain", info->condition);
	}

	return ret;	/* 0: success, -1: fail */
}

static inline void disp_syncgen_irqenable(int module, int enable)
{
	DBGOUT("%s: display.%d, %s\n", __func__, module, enable?"ON":"OFF");
	/* enable source irq */
	NX_DPC_ClearInterruptPendingAll(module);
	NX_DPC_SetInterruptEnableAll(module, enable ? CTRUE : CFALSE);
}

static void disp_syncgen_irq_work(struct work_struct *work)
{
	struct disp_control_info *info;
	struct disp_process_dev *pdev;
	int module;
	char path[32]={0,};

	info = container_of(work, struct disp_control_info, work);
	RET_ASSERT(info);

	if (!info->cond_notify)
		return;

	pdev = info->proc_dev;
	module = info->module;
	sprintf(path, "vsync.%d", module);

	/* check select with exceptfds */
	sysfs_notify(info->kobj, NULL, path);

	info->cond_notify = 0;
}

static irqreturn_t	disp_syncgen_irqhandler(int irq, void *desc)
{
	struct disp_control_info *info = desc;
	int module = info->module;
	int version = nxp_cpu_version();
	ktime_t ts;

	info->condition = 1;
	wake_up_interruptible(&info->wait_queue);

	if (info->active_notify) {
		info->cond_notify = 1;
		ts = ktime_get();
		info->fps = ktime_to_us(ts) - ktime_to_us(info->time_stamp);
		info->time_stamp = ts;

		schedule_work(&info->work);
	}
	NX_DPC_ClearInterruptPendingAll(module);

	if (!version)
		NX_MLC_SetTopDirtyFlag(module);

    spin_lock(&info->lock_callback);
    if (!list_empty(&info->callback_list)) {
        struct disp_irq_callback *callback;
        list_for_each_entry(callback, &info->callback_list, list)
            callback->handler(callback->data);
    }
    spin_unlock(&info->lock_callback);

#if (0)
    {
		static long ts[2] = {0, };
		long new = ktime_to_ms(ktime_get());
		/* if (ts) printk("[%dms]\n", new-ts); */
        if ((new - ts[module]) > 18) {
            printk("[DPC %d]INTR OVERRUN %ld ms\n", module, new - ts[module]);
        }
		ts[module] = new;
    }
#endif

	return IRQ_HANDLED;
}

static void disp_syncgen_initialize(void)
{
	int power = nxp_soc_peri_reset_status(RESET_ID_DISPLAY);
	int i = 0;

	printk("Disply Reset Status : %s\n", power?"On":"Off");

	/* reset */
	if (!power) {
		disp_topctl_reset();
		disp_syncgen_reset();
		printk("Disply Reset Top/Syncgen ...\n");
	}

	/* prototype */
	NX_DPC_Initialize();
	NX_MLC_Initialize();

	for (; DISPLAY_SYNCGEN_NUM > i; i++) {
		/* BASE : MLC, PCLK/BCLK */
		NX_MLC_SetBaseAddress(i, (void*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(i)));
		NX_MLC_SetClockPClkMode(i, NX_PCLKMODE_ALWAYS);
		NX_MLC_SetClockBClkMode(i, NX_BCLKMODE_ALWAYS);

		/* BASE : DPC, PCLK */
		NX_DPC_SetBaseAddress(i, (void*)IO_ADDRESS(NX_DPC_GetPhysicalAddress(i)));
		NX_DPC_SetClockPClkMode(i, NX_PCLKMODE_ALWAYS);
	}

	/* Top is one, Top's devices  */
	NX_DISPLAYTOP_Initialize();
	NX_DISPLAYTOP_SetBaseAddress((void*)IO_ADDRESS(NX_DISPLAYTOP_GetPhysicalAddress()));
	NX_DISPLAYTOP_OpenModule();
	NX_DISPTOP_CLKGEN_Initialize();
}

static int  disp_syncgen_set_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	RET_ASSERT_VAL(psync, -EINVAL);

	DBGOUT("%s\n", __func__);

	memcpy(&pdev->vsync, psync, sizeof(struct disp_vsync_info));

	pdev->status |= PROC_STATUS_READY;
	info->wait_time = display_framerate_jiffies(info->module, &pdev->vsync);

	return 0;
}

static int disp_syncgen_get_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	struct list_head *head = &info->link;

	if (list_empty(head)) {
		printk("display:%9s not connected display out ...\n", dev_to_str(pdev->dev_id));
		return -1;
	}

	if (psync)
		memcpy(psync, &pdev->vsync, sizeof(struct disp_vsync_info));

	return 0;
}

static int  disp_syncgen_prepare(struct disp_control_info *info)
{
	struct disp_process_dev *pdev = info->proc_dev;
	struct disp_syncgen_par *psgen = &pdev->sync_gen;
	struct disp_vsync_info *psync = &pdev->vsync;
	int module = info->module;
	unsigned int out_format = psgen->out_format;
	unsigned int delay_mask = psgen->delay_mask;
	int rgb_pvd = 0, hsync_cp1 = 7, vsync_fram = 7, de_cp2 = 7;
	int v_vso = 1, v_veo = 1, e_vso = 1, e_veo = 1;

	int clk_src_lv0 = psync->clk_src_lv0;
	int clk_div_lv0 = psync->clk_div_lv0;
	int clk_src_lv1 = psync->clk_src_lv1;
	int clk_div_lv1 = psync->clk_div_lv1;
	int clk_dly_lv0 = psgen->clk_delay_lv0;
	int clk_dly_lv1 = psgen->clk_delay_lv1;

#if 0
	int 	   invert_field = psgen->invert_field;
	int 		 swap_RB    = psgen->swap_RB;
	unsigned int yc_order   = psgen->yc_order;
	int interlace = psgen->interlace;
	int vclk_select = psgen->vclk_select;
	int vclk_invert = psgen->clk_inv_lv0 | psgen->clk_inv_lv1;
	CBOOL EmbSync = (out_format == DPC_FORMAT_CCIR656 ? CTRUE : CFALSE);
#endif
	CBOOL RGBMode = CFALSE;
	NX_DPC_DITHER RDither, GDither, BDither;

	/* set delay mask */
	if (delay_mask & DISP_SYNCGEN_DELAY_RGB_PVD)
		rgb_pvd = psgen->d_rgb_pvd;
	if (delay_mask & DISP_SYNCGEN_DELAY_HSYNC_CP1)
		hsync_cp1 = psgen->d_hsync_cp1;
	if (delay_mask & DISP_SYNCGEN_DELAY_VSYNC_FRAM)
		vsync_fram = psgen->d_vsync_fram;
	if (delay_mask & DISP_SYNCGEN_DELAY_DE_CP)
		de_cp2 = psgen->d_de_cp2;

	if (psgen->vs_start_offset != 0 ||
		psgen->vs_end_offset 	 != 0 ||
		psgen->ev_start_offset != 0 ||
		psgen->ev_end_offset   != 0) {
		v_vso = psgen->vs_start_offset;
		v_veo = psgen->vs_end_offset;
		e_vso = psgen->ev_start_offset;
		e_veo = psgen->ev_end_offset;
	}

    if (((U32)NX_DPC_FORMAT_RGB555   == out_format) ||
		((U32)NX_DPC_FORMAT_MRGB555A == out_format) ||
		((U32)NX_DPC_FORMAT_MRGB555B == out_format))	{
		RDither = GDither = BDither = NX_DPC_DITHER_5BIT;
		RGBMode = CTRUE;
	}
	else if (((U32)NX_DPC_FORMAT_RGB565  == out_format) ||
			 ((U32)NX_DPC_FORMAT_MRGB565 == out_format))	{
		RDither = BDither = NX_DPC_DITHER_5BIT;
		GDither = NX_DPC_DITHER_6BIT, RGBMode = CTRUE;
	}
	else if (((U32)NX_DPC_FORMAT_RGB666  == out_format) ||
			 ((U32)NX_DPC_FORMAT_MRGB666 == out_format))	{
		RDither = GDither = BDither = NX_DPC_DITHER_6BIT;
		RGBMode = CTRUE;
	}
	else {
		RDither = GDither = BDither = NX_DPC_DITHER_BYPASS;
		RGBMode = CTRUE;
	}

	/* CLKGEN0/1 */
	NX_DPC_SetClockSource  (module, 0, clk_src_lv0);
	NX_DPC_SetClockDivisor (module, 0, clk_div_lv0);
	NX_DPC_SetClockOutDelay(module, 0, clk_dly_lv0);
	NX_DPC_SetClockSource  (module, 1, clk_src_lv1);
	NX_DPC_SetClockDivisor (module, 1, clk_div_lv1);
	NX_DPC_SetClockOutDelay(module, 1, clk_dly_lv1);

	/* LCD out */
#if 0
	NX_DPC_SetMode(module, out_format, interlace, invert_field, RGBMode,
			swap_RB, yc_order, EmbSync, EmbSync, vclk_select, vclk_invert, CFALSE);
	NX_DPC_SetHSync(module,  psync->h_active_len,
			psync->h_sync_width,  psync->h_front_porch,  psync->h_back_porch,  psync->h_sync_invert);
	NX_DPC_SetVSync(module,
			psync->v_active_len, psync->v_sync_width, psync->v_front_porch, psync->v_back_porch,
			psync->v_sync_invert,
			psync->v_active_len, psync->v_sync_width, psync->v_front_porch, psync->v_back_porch);
	NX_DPC_SetVSyncOffset(module, v_vso, v_veo, e_vso, e_veo);
	NX_DPC_SetDelay (module, rgb_pvd, hsync_cp1, vsync_fram, de_cp2);
 	NX_DPC_SetDither(module, RDither, GDither, BDither);
#else
    {
        POLARITY FieldPolarity = POLARITY_ACTIVEHIGH;
        POLARITY HSyncPolarity = POLARITY_ACTIVEHIGH;
        POLARITY VSyncPolarity = POLARITY_ACTIVEHIGH;

        NX_DPC_SetSync ( module,
                PROGRESSIVE,
                psync->h_active_len,
                psync->v_active_len,
                psync->h_sync_width,
                psync->h_front_porch,
                psync->h_back_porch,
                psync->v_sync_width,
                psync->v_front_porch,
                psync->v_back_porch,
                FieldPolarity,
                HSyncPolarity,
                VSyncPolarity,
                0, 0, 0, 0, 0, 0, 0); // EvenVSW, EvenVFP, EvenVBP, VSP, VCP, EvenVSP, EvenVCP

        NX_DPC_SetDelay (module, rgb_pvd, hsync_cp1, vsync_fram, de_cp2);
        NX_DPC_SetOutputFormat(module, out_format, 0 );
        NX_DPC_SetDither(module, RDither, GDither, BDither);
        NX_DPC_SetQuantizationMode(module, QMODE_256, QMODE_256 );
    }
#endif

	DBGOUT("%s: display.%d (x=%4d, hfp=%3d, hbp=%3d, hsw=%3d)\n",
		__func__, module, psync->h_active_len, psync->h_front_porch,
		psync->h_back_porch, psync->h_sync_width);
	DBGOUT("%s: display.%d (y=%4d, vfp=%3d, vbp=%3d, vsw=%3d)\n",
		__func__, module, psync->v_active_len, psync->v_front_porch,
		psync->v_back_porch, psync->v_sync_width);
	DBGOUT("%s: display.%d clk 0[s=%d, d=%3d], 1[s=%d, d=%3d], inv[%d:%d]\n",
		__func__, module, clk_src_lv0, clk_div_lv0, clk_src_lv1, clk_div_lv1,
		psgen->clk_inv_lv0, psgen->clk_inv_lv1);
	DBGOUT("%s: display.%d v_vso=%d, v_veo=%d, e_vso=%d, e_veo=%d\n",
		__func__, module, v_vso, v_veo, e_vso, e_veo);
	DBGOUT("%s: display.%d delay RGB=%d, HS=%d, VS=%d, DE=%d\n",
		__func__, module, rgb_pvd, hsync_cp1, vsync_fram, de_cp2);

	return 0;
}

static int  disp_syncgen_enable(struct disp_process_dev *pdev, int enable)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	int wait = info->wait_time * 10;
	int module = info->module;

	DBGOUT("%s : display.%d, %s, wait=%d, status=0x%x\n",
		__func__, module, enable?"ON":"OFF", wait, pdev->status);

	if (!enable) {
		/* multilayer top */
		disp_multily_enable(info, enable);

		if (CTRUE == NX_DPC_GetDPCEnable(module)) {
			disp_syncgen_irqenable(module, 1);
			DISP_WAIT_POLL_VSYNC(module, wait);
		}

		NX_DPC_SetDPCEnable(module, CFALSE);			/* START: DPC */
		NX_DPC_SetClockDivisorEnable(module, CFALSE);	/* START: CLKGEN */

		disp_syncgen_irqenable(info->module, 0);
		pdev->status &= ~PROC_STATUS_ENABLE;
	} else {
		/* set irq wait time */
		if (!(PROC_STATUS_READY & pdev->status)) {
            if (pdev->dev_id == DISP_DEVICE_TVOUT) {
                disp_multily_enable(info, enable);
                pdev->status |=  PROC_STATUS_ENABLE;
                disp_syncgen_irqenable(info->module, 1);
                return 0;
            }
			printk(KERN_ERR "Fail, %s not set sync ...\n", dev_to_str(pdev->dev_id));
			return -EINVAL;
		}

		disp_multily_enable(info, enable);
		disp_syncgen_prepare(info);
		disp_syncgen_irqenable(info->module, 1);

        if (module == 0)
            NX_DPC_SetRegFlush(module);
		NX_DPC_SetDPCEnable(module, CTRUE);				/* START: DPC */
		NX_DPC_SetClockDivisorEnable(module, CTRUE);	/* START: CLKGEN */

		pdev->status |=  PROC_STATUS_ENABLE;
	}

	return 0;
}

static int  disp_syncgen_stat_enable(struct disp_process_dev *pdev)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	struct disp_process_ops *ops;
	struct list_head *pos, *head;
	int ret = pdev->status & PROC_STATUS_ENABLE ? 1 : 0;

	DBGOUT("%s %s -> ", __func__, dev_to_str(pdev->dev_id));

	head = &info->link;
	if (list_empty(head)) {
		printk("display:%9s not connected display out ...\n", dev_to_str(pdev->dev_id));
		return -1;
	}

	/* from last */
	list_for_each_prev(pos, head) {
		if (list_is_last(pos, head)) {
			pdev = container_of(pos, struct disp_process_dev, list);
			if (pdev) {
				ops = pdev->disp_ops;
				if (ops && ops->stat_enable)
					ret = ops->stat_enable(pdev);
			}
			break;
		}
	}

	DBGOUT("(%s status = %s)\n", dev_to_str(pdev->dev_id),
		pdev->status & PROC_STATUS_ENABLE?"ON":"OFF");

	return ret;
}

static int  disp_multily_suspend(struct disp_process_dev *pdev)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	struct disp_multily_dev *pmly = &info->multilayer;
	int mlc_len = sizeof(struct NX_MLC_RegisterSet);
	int module = info->module;

	PM_DBGOUT("%s display.%d (MLC:%s, DPC:%s)\n",
		__func__, module, pmly->enable?"ON":"OFF", pdev->status & PROC_STATUS_ENABLE?"ON":"OFF");

	NX_MLC_SetMLCEnable(module, CFALSE);
	NX_MLC_SetTopDirtyFlag(module);

	memcpy((void*)pmly->save_addr, (const void*)pmly->base_addr, mlc_len);

	return 0;
}

static void disp_multily_resume(struct disp_process_dev *pdev)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	struct disp_multily_dev *pmly = &info->multilayer;
	int mlc_len = sizeof(struct NX_MLC_RegisterSet);
	int module = info->module;
	int i = 0;

	PM_DBGOUT("%s display.%d (MLC:%s, DPC:%s)\n",
		__func__, module, pmly->enable?"ON":"OFF",
				pdev->status & PROC_STATUS_ENABLE?"ON":"OFF");

	/* restore */
	NX_MLC_SetClockPClkMode(module, NX_PCLKMODE_ALWAYS);
	NX_MLC_SetClockBClkMode(module, NX_BCLKMODE_ALWAYS);
	memcpy((void*)pmly->base_addr, (const void*)pmly->save_addr, mlc_len);

	if (pmly->enable) {
		NX_MLC_SetTopPowerMode(module, CTRUE);
   		NX_MLC_SetTopSleepMode(module, CFALSE);
		NX_MLC_SetMLCEnable(module, CTRUE);

		for (i = 0; LAYER_RGB_NUM > i; i++)
			NX_MLC_SetDirtyFlag(module, i);

		NX_MLC_SetDirtyFlag(module, LAYER_VID_NUM);
		NX_MLC_SetTopDirtyFlag(module);
	}
}

static int  disp_syncgen_suspend(struct disp_process_dev *pdev)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	struct disp_multily_dev *pmly = &info->multilayer;
	int mlc_len = sizeof(struct NX_MLC_RegisterSet);
	int dpc_len = sizeof(struct NX_DPC_RegisterSet);
	int module = info->module;

	PM_DBGOUT("%s display.%d (MLC:%s, DPC:%s)\n",
		__func__, module, pmly->enable?"ON":"OFF", pdev->status & PROC_STATUS_ENABLE?"ON":"OFF");

	NX_MLC_SetMLCEnable(module, CFALSE);
	NX_MLC_SetTopDirtyFlag(module);

	NX_DPC_SetDPCEnable(module, CFALSE);
	NX_DPC_SetClockDivisorEnable(module, CFALSE);

	memcpy((void*)pmly->save_addr, (const void*)pmly->base_addr, mlc_len);
	memcpy((void*)pdev->save_addr, (const void*)pdev->base_addr, dpc_len);

	disp_syncgen_irqenable(module, 0);
	info->condition = 1;

	return 0;
}

static void disp_syncgen_resume(struct disp_process_dev *pdev)
{
	struct disp_control_info *info = get_device_to_info(pdev);
	struct disp_multily_dev *pmly = &info->multilayer;
	int mlc_len = sizeof(struct NX_MLC_RegisterSet);
	int dpc_len = sizeof(struct NX_DPC_RegisterSet);
	int module = info->module;
	int i = 0;

	PM_DBGOUT("%s display.%d (MLC:%s, DPC:%s)\n",
		__func__, module, pmly->enable?"ON":"OFF",
				pdev->status & PROC_STATUS_ENABLE?"ON":"OFF");

	/* restore */
	NX_MLC_SetClockPClkMode(module, NX_PCLKMODE_ALWAYS);
	NX_MLC_SetClockBClkMode(module, NX_BCLKMODE_ALWAYS);
	memcpy((void*)pmly->base_addr, (const void*)pmly->save_addr, mlc_len);

	if (pmly->enable) {
		NX_MLC_SetTopPowerMode(module, CTRUE);
   		NX_MLC_SetTopSleepMode(module, CFALSE);
		NX_MLC_SetMLCEnable(module, CTRUE);

		for (i = 0; LAYER_RGB_NUM > i; i++)
			NX_MLC_SetDirtyFlag(module, i);

		NX_MLC_SetDirtyFlag(module, LAYER_VID_NUM);
		NX_MLC_SetTopDirtyFlag(module);
	}

	/* restore */
	NX_DPC_SetClockPClkMode(module, NX_PCLKMODE_ALWAYS);
	memcpy((void*)pdev->base_addr, (const void*)pdev->save_addr, dpc_len);

	if (pdev->status & PROC_STATUS_ENABLE) {
//		int wait = info->wait_time * 10;
		disp_syncgen_irqenable(module, 0);	/* disable interrupt */

		NX_DPC_SetDPCEnable(module, CTRUE);
		NX_DPC_SetClockDivisorEnable(module, CTRUE);

		/* wait sync */
//		DISP_WAIT_POLL_VSYNC(module, wait);
		disp_syncgen_irqenable(module, 1);	/* enable interrupt */
	}
}

static inline int disp_ops_prepare_devs(struct list_head *head)
{
	struct disp_process_ops *ops;
	struct disp_process_dev *pdev;
	struct list_head *pos;
	int ret = 0;

	list_for_each_prev(pos, head) {
		pdev = container_of(pos, struct disp_process_dev, list);
		ops  = pdev->disp_ops;
		DBGOUT("PRE: %s, ops=0x%p\n", pdev->name, ops);
		if (ops && ops->prepare) {
			ret = ops->prepare(ops->dev);
			if (ret) {
				printk(KERN_ERR "Fail, display prepare [%s]...\n",
					dev_to_str(pdev->dev_id));
				return -EINVAL;
			}
		}
	}
	return 0;
}

static inline void disp_ops_enable_devs(struct list_head *head, int on)
{
	struct disp_process_ops *ops;
	struct disp_process_dev *pdev;
	struct list_head *pos;

	list_for_each(pos, head) {
		pdev = container_of(pos, struct disp_process_dev, list);
		ops  = pdev->disp_ops;
		DBGOUT("%s: %s, ops=0x%p\n", on?"ON ":"OFF", pdev->name, ops);
		if (ops && ops->enable)
			ops->enable(ops->dev, on);
	}
}

static inline void disp_ops_pre_resume_devs(struct list_head *head)
{
	struct disp_process_ops *ops;
	struct disp_process_dev *pdev;
	struct list_head *pos;

	list_for_each_prev(pos, head) {
		pdev = container_of(pos, struct disp_process_dev, list);
		ops  = pdev->disp_ops;
		PM_DBGOUT("PRE RESUME: %s, ops=0x%p\n", pdev->name, ops);
		if (ops && ops->pre_resume)
			ops->pre_resume(ops->dev);
	}
}

static inline int disp_ops_suspend_devs(struct list_head *head, int suspend)
{
	struct disp_process_ops *ops;
	struct disp_process_dev *pdev;
	struct list_head *pos;
	int ret = 0;

	if (suspend) { /* last  -> first */
		list_for_each_prev(pos, head) {
			pdev = container_of(pos, struct disp_process_dev, list);
			ops  = pdev->disp_ops;
			PM_DBGOUT("SUSPEND: %s, ops=0x%p\n", pdev->name, ops);
			if (ops && ops->suspend) {
				ret = ops->suspend(ops->dev);
				if (ret)
					return -EINVAL;
			}
		}
	} else {
		list_for_each(pos, head) {
			pdev = container_of(pos, struct disp_process_dev, list);
			ops  = pdev->disp_ops;
			PM_DBGOUT("RESUME : %s, ops=0x%p\n", pdev->name, ops);
			if (ops && ops->resume)
				ops->resume(ops->dev);
		}
	}

	return 0;
}

static struct disp_process_ops syncgen_ops[DISPLAY_SYNCGEN_NUM] = {
	/* primary */
	{
		.set_vsync 		= disp_syncgen_set_vsync,
		.get_vsync  	= disp_syncgen_get_vsync,
		.enable 		= disp_syncgen_enable,
		.stat_enable	= disp_syncgen_stat_enable,
		.suspend		= disp_syncgen_suspend,
		.resume	  		= disp_syncgen_resume,
	},
#if (DISPLAY_SYNCGEN_NUM > 1)
	/* secondary */
	{
		.set_vsync 		= disp_syncgen_set_vsync,
		.get_vsync  	= disp_syncgen_get_vsync,
		.enable 		= disp_syncgen_enable,
		.stat_enable	= disp_syncgen_stat_enable,
		.suspend		= disp_syncgen_suspend,
		.resume	  		= disp_syncgen_resume,
	},
#endif
};

/*
 * RGB Layer on MultiLayer
 */
int nxp_soc_disp_rgb_set_fblayer(int module, int layer)
{
	DISP_MULTILY_DEV(module, pmly);
	RET_ASSERT_VAL(module > -1 && 2 > module, -EINVAL);
	RET_ASSERT_VAL(layer > -1 && LAYER_RGB_NUM > layer, -EINVAL);
	DBGOUT("%s: framebuffer layer = %d.%d\n", __func__, module, layer);

	pmly->fb_layer = layer;

	return 0;
}

int nxp_soc_disp_rgb_get_fblayer(int module)
{
	DISP_MULTILY_DEV(module, pmly);
	return pmly->fb_layer;
}

int  nxp_soc_disp_rgb_set_format(int module, int layer, unsigned int format,
				int image_w, int image_h, int pixelbyte)
{
	DISP_MULTILY_RGB(module, prgb, layer);
	CBOOL EnAlpha = CFALSE;

	DBGOUT("%s: %s, fmt:0x%x, w=%d, h=%d, bpp/8=%d\n",
		__func__, prgb->name, format, image_w, image_h, pixelbyte);

	prgb->pos_x = 0;
	prgb->pos_y = 0;
	prgb->left = 0;
	prgb->top = 0;
	prgb->right = image_w;
	prgb->bottom = image_h;
	prgb->format = format;
	prgb->width = image_w;
	prgb->height = image_h;
	prgb->pixelbyte = pixelbyte;
	prgb->clipped = 0;

	/* set alphablend */
	if (format == MLC_RGBFMT_A1R5G5B5 ||
		format == MLC_RGBFMT_A1B5G5R5 ||
		format == MLC_RGBFMT_A4R4G4B4 ||
		format == MLC_RGBFMT_A4B4G4R4 ||
		format == MLC_RGBFMT_A8R3G3B2 ||
		format == MLC_RGBFMT_A8B3G3R2 ||
		format == MLC_RGBFMT_A8R8G8B8 ||
		format == MLC_RGBFMT_A8B8G8R8)
		EnAlpha = CTRUE;

    /* psw0523 fix for video -> prgb setting ordering */
	/* NX_MLC_SetTransparency(module, layer, CFALSE, prgb->color.transcolor); */
	NX_MLC_SetColorInversion(module, layer, CFALSE, prgb->color.invertcolor);
	NX_MLC_SetAlphaBlending(module, layer, EnAlpha, prgb->color.alphablend);
	NX_MLC_SetFormatRGB(module, layer, (NX_MLC_RGBFMT)format);
	NX_MLC_SetRGBLayerInvalidPosition(module, layer, 0, 0, 0, 0, 0, CFALSE);
	NX_MLC_SetRGBLayerInvalidPosition(module, layer, 1, 0, 0, 0, 0, CFALSE);

   if (image_w && image_h)
        NX_MLC_SetPosition(module, layer, 0, 0, image_w-1, image_h-1);

	return 0;
}

void  nxp_soc_disp_rgb_get_format(int module, int layer, unsigned int *format,
				int *image_w, int *image_h, int *pixelbyte)
{
	DISP_MULTILY_RGB(module, prgb, layer);

	if (format)  *format = prgb->format;
	if (image_w) *image_w = prgb->width;
	if (image_h) *image_h = prgb->height;
	if (pixelbyte) *pixelbyte = prgb->pixelbyte;
}

int nxp_soc_disp_rgb_set_position(int module, int layer, int x, int y, int waitvsync)
{
	DISP_MULTILY_RGB(module, prgb, layer);
	int left = prgb->pos_x = x;
	int top = prgb->pos_y = y;
	int right = x + prgb->right;
	int bottom = y + prgb->bottom;
	RET_ASSERT_VAL(prgb->format, -EINVAL);

	DBGOUT("%s: %s, wait=%d - L=%d, T=%d, R=%d, B=%d\n",
		__func__, prgb->name, waitvsync, left, top, right, bottom);

	NX_MLC_SetPosition(module, layer, left, top, right-1, bottom-1);
	NX_MLC_SetDirtyFlag(module, layer);
	disp_syncgen_waitsync(module, layer, waitvsync);

	return 0;
}

int nxp_soc_disp_rgb_get_position(int module, int layer, int *x, int *y)
{
	int left, top, right, bottom;
	NX_MLC_GetPosition(module, layer, &left, &top, &right, &bottom);
	if (x) *x = left;
	if (y) *y = top;
	return 0;
}

int  nxp_soc_disp_rgb_set_clipping(int module, int layer,
				int left, int top, int width, int height)
{
	DISP_MULTILY_RGB(module, prgb, layer);
	RET_ASSERT_VAL(left >= 0 && top >= 0, -EINVAL);
	RET_ASSERT_VAL(width > 0 && height > 0, -EINVAL);

	DBGOUT("%s: %s, x=%d, y=%d, w=%d, h=%d\n",
		__func__, prgb->name, left, top, width, height);

	prgb->clipped = 1;
	prgb->left = left;
	prgb->top = top;
	prgb->right = left + width;
	prgb->bottom = top + height;

	return 0;
}

void nxp_soc_disp_rgb_get_clipping(int module, int layer,
			int *left, int *top, int *width, int *height)
{
	DISP_MULTILY_RGB(module, prgb, layer);

	if (left) *left = prgb->left;
	if (top) *top = prgb->top;
	if (width) *width = prgb->right - prgb->left;
	if (height) *height = prgb->bottom - prgb->top;
}

void nxp_soc_disp_rgb_set_address(int module, int layer,
				unsigned int phyaddr, unsigned int pixelbyte, unsigned int stride,
				int waitvsync)
{
	DISP_MULTILY_RGB(module, prgb, layer);

	DBGOUT("%s: %s, pa=0x%x, hs=%d, vs=%d, wait=%d\n",
		__func__, prgb->name, phyaddr, pixelbyte, stride, waitvsync);

	if (prgb->clipped) {
		int xoff = prgb->left * pixelbyte;
		int yoff = prgb->top * (prgb->width * prgb->pixelbyte);
		phyaddr += (xoff + yoff);
		stride = (prgb->width - prgb->left) * prgb->pixelbyte;
		NX_MLC_SetPosition(module, layer,
				prgb->pos_x, prgb->pos_x, prgb->right-1, prgb->bottom-1);
	}

	prgb->address = phyaddr;
	prgb->pixelbyte = pixelbyte;
	prgb->stride = stride;

	NX_MLC_SetRGBLayerStride (module, layer, pixelbyte, stride);
	NX_MLC_SetRGBLayerAddress(module, layer, phyaddr);
	NX_MLC_SetDirtyFlag(module, layer);
	disp_syncgen_waitsync(module, layer, waitvsync);
}

void nxp_soc_disp_rgb_get_address(int module, int layer,
				unsigned int *phyaddr,unsigned int *pixelbyte, unsigned int *stride)
{
	DISP_MULTILY_RGB(module, prgb, layer);

	if (!prgb->pixelbyte || !prgb->stride || !prgb->address){
		unsigned int phy_addr, pixelbyte, stride;

		NX_MLC_GetRGBLayerStride (module, layer, &pixelbyte, &stride);
		NX_MLC_GetRGBLayerAddress(module, layer, &phy_addr);
		prgb->pixelbyte = pixelbyte;
		prgb->stride = stride;
		prgb->address = phy_addr;
	}

	if (phyaddr) *phyaddr = prgb->address;
	if (pixelbyte) *pixelbyte = prgb->pixelbyte;
	if (stride) *stride = prgb->stride;

	DBGOUT("%s: %s, pa=0x%x, hs=%d, vs=%d\n",
		__func__, prgb->name, prgb->address, prgb->pixelbyte, prgb->stride);
}

void  nxp_soc_disp_rgb_set_color(int module, int layer, unsigned int type,
				unsigned int color, int enable)
{
	DISP_MULTILY_RGB(module, prgb, layer);

	switch (type) {
	case RGB_COLOR_ALPHA:
		if (color <= 0 ) color = 0;
		if (color >= 15) color = 15;
		prgb->color.alpha = (enable?color:15);
		NX_MLC_SetAlphaBlending(module, layer, (enable ? CTRUE : CFALSE), color);
		NX_MLC_SetDirtyFlag(module, layer);
		break;
	case RGB_COLOR_TRANSP:
		if (1 == prgb->pixelbyte)
			color = R8G8B8toR3G3B2((unsigned int)color), color = R3G3B2toR8G8B8((u_char)color);
		if (2 == prgb->pixelbyte)
			color = R8G8B8toR5G6B5((unsigned int)color), color = R5G6B5toR8G8B8((u_short)color);
		prgb->color.transcolor = (enable?color:0);
		NX_MLC_SetTransparency(module, layer, (enable ? CTRUE : CFALSE), (color & 0x00FFFFFF));
		NX_MLC_SetDirtyFlag(module, layer);
		break;
	case RGB_COLOR_INVERT:
		if (1 == prgb->pixelbyte)
			color = R8G8B8toR3G3B2((unsigned int)color), color = R3G3B2toR8G8B8((u_char)color);
		if (2 == prgb->pixelbyte)
			color = R8G8B8toR5G6B5((unsigned int)color), color = R5G6B5toR8G8B8((u_short)color);
		prgb->color.invertcolor = (enable?color:0);
		NX_MLC_SetColorInversion(module, layer, (enable ? CTRUE : CFALSE), (color & 0x00FFFFFF));
		NX_MLC_SetDirtyFlag(module, layer);
		break;
	default:
		break;
	}
}

unsigned int  nxp_soc_disp_rgb_get_color(int module, int layer, unsigned int type)
{
	DISP_MULTILY_RGB(module, prgb, layer);

	switch (type) {
	case RGB_COLOR_ALPHA:	 return (unsigned int)prgb->color.alpha;
	case RGB_COLOR_TRANSP: return (unsigned int)prgb->color.transcolor;
	case RGB_COLOR_INVERT: return (unsigned int)prgb->color.invertcolor;
	default: break;
	}

	return 0;
}

void nxp_soc_disp_rgb_set_enable(int module, int layer, int enable)
{
	DISP_MULTILY_RGB(module, prgb, layer);
	DBGOUT("%s: %s, %s\n", __func__, prgb->name, enable?"ON":"OFF");

	prgb->enable = enable;
	NX_MLC_SetLayerEnable(module, layer, (enable ? CTRUE : CFALSE));
	NX_MLC_SetDirtyFlag(module, layer);
}

int nxp_soc_disp_rgb_stat_enable(int module, int layer)
{
	int enable = NX_MLC_GetLayerEnable(module, layer) ? 1 : 0;
	DISP_MULTILY_RGB(module, prgb, layer);
	prgb->enable = enable;
	return enable;
}

/*
 * RGB Layer on MultiLayer
 */
int nxp_soc_disp_video_set_format(int module, unsigned int fourcc, int image_w, int image_h)
{
	DISP_MULTILY_VID(module, pvid);
	unsigned int format;
	RET_ASSERT_VAL(image_w > 0 && image_h > 0, -EINVAL);

	switch (fourcc) {
	case FOURCC_MVS0:
	case FOURCC_YV12: format = NX_MLC_YUVFMT_420;
		break;
	case FOURCC_MVS2:
	case FOURCC_MVN2: format = NX_MLC_YUVFMT_422;
		break;
	case FOURCC_MVS4: format = NX_MLC_YUVFMT_444;
		break;
	case FOURCC_YUY2:
	case FOURCC_YUYV: format = NX_MLC_YUVFMT_YUYV;
		break;
	default:
		printk(KERN_ERR "Fail, not support video fourcc=%c%c%c%c\n",
		(fourcc>>0)&0xFF, (fourcc>>8)&0xFF, (fourcc>>16)&0xFF, (fourcc>>24)&0xFF);
		return -EINVAL;
	}

	pvid->format = fourcc;
	pvid->width  = image_w;
	pvid->height = image_h;

	NX_MLC_SetFormatYUV(module, (NX_MLC_YUVFMT)format);
	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);

	return 0;
}

void nxp_soc_disp_video_get_format(int module, unsigned int *fourcc, int *image_w, int *image_h)
{
	DISP_MULTILY_VID(module, pvid);
	*fourcc  = pvid->format;
	*image_w = pvid->width;
	*image_h = pvid->height;
}

void nxp_soc_disp_video_set_address(int module, unsigned int lu_a, unsigned int lu_s,
				unsigned int cb_a, unsigned int cb_s, unsigned int cr_a, unsigned int cr_s,
				int waitvsync)
{
	DISP_MULTILY_VID(module, pvid);

	DBGOUT("%s: %s, lua=0x%x, cba=0x%x, cra=0x%x (lus=%d, cbs=%d, crs=%d), wait=%d\n",
		__func__, pvid->name, lu_a, cb_a, cr_a, lu_s, cb_s, cr_s, waitvsync);

	if (FOURCC_YUYV == pvid->format) {
		NX_MLC_SetVideoLayerAddressYUYV(module, lu_a, lu_s);
	} else {
		NX_MLC_SetVideoLayerStride (module, lu_s, cb_s, cr_s);
		NX_MLC_SetVideoLayerAddress(module, lu_a, cb_a, cr_a);
	}

	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
	disp_syncgen_waitsync(module, MLC_LAYER_VIDEO, waitvsync);
}

void 	nxp_soc_disp_video_get_address(int module, unsigned int *lu_a, unsigned int *lu_s,
				unsigned int *cb_a, unsigned int *cb_s, unsigned int *cr_a,	unsigned int *cr_s)
{
	DISP_MULTILY_VID(module, pvid);

	if (FOURCC_YUYV == pvid->format) {
		RET_ASSERT(lu_a && lu_s);
		NX_MLC_GetVideoLayerAddressYUYV(module, lu_a, lu_s);
	} else {
		RET_ASSERT(lu_a && cb_a && cr_a);
		RET_ASSERT(lu_s && cb_s && cr_s);
		NX_MLC_GetVideoLayerAddress(module, lu_a, cb_a, cr_a);
		NX_MLC_GetVideoLayerStride (module, lu_s, cb_s, cr_s);
	}
}

int nxp_soc_disp_video_set_position(int module, int left, int top,
				int right, int bottom, int waitvsync)
{
	DISP_MULTILY_VID(module, pvid);
	int srcw = pvid->width;
	int srch = pvid->height;
	int dstw = right - left;
	int dsth = bottom - top;
	int hf = 1, vf = 1;
	int version = nxp_cpu_version();

	RET_ASSERT_VAL(pvid->format, -EINVAL);

	DBGOUT("%s: %s, L=%d, T=%d, R=%d, B=%d, wait=%d\n",
		__func__, pvid->name, left, top, right, bottom, waitvsync);

	if (left >= right ) right  = left+1;
	if (top  >= bottom) bottom = top +1;
	if (0    >= right )	left  -= (right -1),  right  = 1;
	if (0    >= bottom)	top   -= (bottom-1), bottom = 1;
	if (dstw >= 2048)   dstw = 2048;
	if (dsth >= 2048)   dsth = 2048;

	if (srcw == dstw && srch == dsth)
		hf = 0, vf = 0;

	if (!version) {
		hf = 0;
		if (dstw > srcw)
			hf = 1, vf = 1;
		if (srcw > 1024)	/* exceed line buffer */
			hf = 0, vf = 0;
		hf = 0, vf = 0;	/* disable filter */
	}

	pvid->hFilter = hf;
	pvid->vFilter = vf;

	/* set scale */
	NX_MLC_SetVideoLayerScale(module, srcw, srch, dstw, dsth,
					(CBOOL)hf, (CBOOL)hf, (CBOOL)vf, (CBOOL)vf);
	NX_MLC_SetPosition(module, MLC_LAYER_VIDEO, left, top, right-1, bottom-1);
	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
	disp_syncgen_waitsync(module, MLC_LAYER_VIDEO, waitvsync);

	return 0;
}

void nxp_soc_disp_video_set_crop(int module, bool enable, int left, int top, int width, int height, int waitvsync)
{
	DISP_MULTILY_VID(module, pvid);

    pvid->en_source_crop = enable;

    if (enable) {
        int srcw = width;
        int srch = height;
        int dstw = pvid->right - pvid->left;
        int dsth = pvid->bottom - pvid->top;
        int hf = 1, vf = 1;

        if (dstw == 0)
            dstw = pvid->width;
        if (dsth == 0)
            dsth = pvid->height;

        printk("%s: %s, L=%d, T=%d, W=%d, H=%d, dstw=%d, dsth=%d, wait=%d\n",
                __func__, pvid->name, left, top, width, height, dstw, dsth, waitvsync);

        if (srcw == dstw && srch == dsth)
            hf = 0, vf = 0;

        pvid->hFilter = hf;
        pvid->vFilter = vf;

        /* set scale */
        NX_MLC_SetVideoLayerScale(module, srcw, srch, dstw, dsth,
                (CBOOL)hf, (CBOOL)hf, (CBOOL)vf, (CBOOL)vf);
        NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
        disp_syncgen_waitsync(module, MLC_LAYER_VIDEO, waitvsync);
    } else {
        nxp_soc_disp_video_set_position(module, pvid->left, pvid->top,
                pvid->right, pvid->bottom, waitvsync);
    }
}

int nxp_soc_disp_video_get_position(int module, int *left, int *top, int *right, int *bottom)
{
	NX_MLC_GetVideoPosition(module, left, top, right, bottom);
	return 0;
}

void nxp_soc_disp_video_set_enable(int module, int enable)
{
	DISP_MULTILY_VID(module, pvid);
	CBOOL hl, hc, vl, vc;

	if (enable) {
        NX_MLC_SetVideoLayerLineBufferPowerMode(module, CTRUE);
   	    NX_MLC_SetVideoLayerLineBufferSleepMode(module, CFALSE);
		NX_MLC_SetLayerEnable(module, MLC_LAYER_VIDEO, CTRUE);
		NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
	} else {
 		NX_MLC_SetLayerEnable(module, MLC_LAYER_VIDEO, CFALSE);
		NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
		disp_syncgen_waitsync(module, MLC_LAYER_VIDEO, 1);

		NX_MLC_GetVideoLayerScaleFilter(module, &hl, &hc, &vl, &vc);
		if (hl | hc | vl | vc)
			NX_MLC_SetVideoLayerScaleFilter(module, 0, 0, 0, 0);
        NX_MLC_SetVideoLayerLineBufferPowerMode(module, CFALSE);
   	    NX_MLC_SetVideoLayerLineBufferSleepMode(module, CTRUE);
		NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
	}
	pvid->enable = enable;
}

int nxp_soc_disp_video_stat_enable(int module)
{
	int enable = NX_MLC_GetLayerEnable(module, MLC_LAYER_VIDEO) ? 1 : 0;
	DISP_MULTILY_VID(module, pvid);

	pvid->enable = enable;

	return enable;
}

void nxp_soc_disp_video_set_priority(int module, int prior)
{
	DISP_MULTILY_DEV(module, pmly);
	NX_MLC_PRIORITY priority = NX_MLC_PRIORITY_VIDEOFIRST;
	DBGOUT("%s: multilayer.%d, video prior=%d\n", __func__, module, prior);

	switch (prior) {
	case 0: priority = NX_MLC_PRIORITY_VIDEOFIRST;  break;	// PRIORITY-video>0>1>2
	case 1: priority = NX_MLC_PRIORITY_VIDEOSECOND; break; 	// PRIORITY-0>video>1>2
	case 2: priority = NX_MLC_PRIORITY_VIDEOTHIRD;  break; 	// PRIORITY-0>1>video>2
	case 3: priority = NX_MLC_PRIORITY_VIDEOFOURTH; break;  // PRIORITY-0>1>2>video
	default: printk(KERN_ERR "Fail, Not support video priority num(0~3),(%d)\n", prior);
		return;
	}
	NX_MLC_SetLayerPriority(module, priority);
	NX_MLC_SetTopDirtyFlag(module);
	pmly->video_prior = prior;
}

int  nxp_soc_disp_video_get_priority(int module)
{
	DISP_MULTILY_DEV(module, pmly);
	return (int)pmly->video_prior;
}

void  nxp_soc_disp_video_set_colorkey(int module, unsigned int color, int enable)
{
	DISP_MULTILY_DEV(module, pmly);
	nxp_soc_disp_rgb_set_color(module, pmly->fb_layer, RGB_COLOR_TRANSP, color, enable);
}

unsigned int nxp_soc_disp_video_get_colorkey(int module)
{
	DISP_MULTILY_DEV(module, pmly);
	return nxp_soc_disp_rgb_get_color(module, pmly->fb_layer, RGB_COLOR_TRANSP);
}

void nxp_soc_disp_video_set_color(int module, unsigned int type,
			unsigned int color, int enable)
{
	DISP_MULTILY_VID(module, pvid);
	int sv,	cv;
	double ang;

	DBGOUT("%s: %s, type=0x%x, col=0x%x, %s\n",
		__func__, pvid->name, type, color, enable?"ON":"OFF");

	switch (type) {
	case VIDEO_COLOR_ALPHA:
		pvid->color.alpha = (enable ? color : 15);		// Default 15
		video_alpha_blend(module, pvid->color.alpha, enable);
		break;
	case VIDEO_COLOR_BRIGHT:
		pvid->color.bright   = (enable ? color : 0);
		pvid->color.contrast = (enable?pvid->color.contrast:0);
		video_lu_color(module, (int)pvid->color.bright, (int)pvid->color.contrast);
		break;
	case VIDEO_COLOR_CONTRAST:
		pvid->color.contrast = (enable ? color : 0);
		pvid->color.bright   = (enable ? pvid->color.bright : 0);
		video_lu_color(module, (int)pvid->color.bright, (int)pvid->color.contrast);
		break;
	case VIDEO_COLOR_HUE:
		if ((int)color <=   0) color = 0;
		if ((int)color >= 360) color = 360;

		pvid->color.hue = (enable ? (int)color : 0);
		pvid->color.saturation = (enable ? pvid->color.saturation : 1);
		ang = DEGREE_RADIAN(pvid->color.hue);
		sv  = (ksin(ang) * 64 * pvid->color.saturation);
		cv  = (kcos(ang) * 64 * pvid->color.saturation);
		video_cbr_color(module, cv, -sv, sv, cv);
		break;
	case VIDEO_COLOR_SATURATION:
 		if ((int)color <= -100) color = -100;
		if ((int)color >=  100) color =  100;

		pvid->color.hue = (enable ? pvid->color.hue : 0);
		pvid->color.saturation = (enable ? 1 + (0.01 * (int)color) : 1);
		pvid->color.satura 	  = (enable ? (int)color : 0);
		ang = (DEGREE_RADIAN(pvid->color.hue));
		sv  = (ksin(ang) * 64 * pvid->color.saturation);
		cv  = (kcos(ang) * 64 * pvid->color.saturation);
		video_cbr_color(module, cv, -sv, sv, cv);
		break;
	case VIDEO_COLOR_GAMMA:
		pvid->color.gamma = color;
		break;
	default:
		break;
	}
}

unsigned int  nxp_soc_disp_video_get_color(int module, unsigned int type)
{
	DISP_MULTILY_VID(module, pvid);
	unsigned int color;
	switch (type) {
	case VIDEO_COLOR_ALPHA:	color = (unsigned int)pvid->color.alpha; break;
	case VIDEO_COLOR_BRIGHT: color = (unsigned int)pvid->color.bright; break;
	case VIDEO_COLOR_CONTRAST: color = (unsigned int)pvid->color.contrast; break;
	case VIDEO_COLOR_HUE: color = (unsigned int)pvid->color.hue; break;
	case VIDEO_COLOR_SATURATION: color = (unsigned int)pvid->color.satura; break;
	case VIDEO_COLOR_GAMMA:	color = (unsigned int)pvid->color.gamma; break;
	default: return -EINVAL;
	}
	return color;
}

void nxp_soc_disp_video_set_hfilter(int module, int enable)
{
	DISP_MULTILY_VID(module, pvid);
	CBOOL hl, hc, vl, vc;

	hl = hc = pvid->hFilter = enable;
	vl = vc = pvid->vFilter;
	NX_MLC_SetVideoLayerScaleFilter(module, hl, hc, vl, vc);
	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

unsigned int nxp_soc_disp_video_get_hfilter(int module)
{
	CBOOL hl, hc, vl, vc;

	NX_MLC_GetVideoLayerScaleFilter(module, &hl, &hc, &vl, &vc);
	if (hl != hc) {
		printk(KERN_INFO "%s: WARN %d horizontal filter Lu=%s, Ch=%s \r\n",
			__func__, module, hl?"On":"Off", hc?"On":"Off" );
	}

	return (unsigned int)hl;
}

void nxp_soc_disp_video_set_vfilter(int module, int enable)
{
	DISP_MULTILY_VID(module, pvid);
	CBOOL hl, hc, vl, vc;

	hl = hc = pvid->hFilter;
	vl = vc = pvid->vFilter = enable;
	NX_MLC_SetVideoLayerScaleFilter(module, hl, hc, vl, vc);
	NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

unsigned int nxp_soc_disp_video_get_vfilter(int module)
{
	CBOOL hl, hc, vl, vc;

	NX_MLC_GetVideoLayerScaleFilter(module, &hl, &hc, &vl, &vc);
	if (hl != hc) {
		printk(KERN_INFO "%s: WARN %d vertical filter Lu=%s, Ch=%s \r\n",
			__func__, module, vl?"On":"Off", vc?"On":"Off");
	}

	return (unsigned int)vl;
}

/*
 * TOP Layer on MultiLayer
 */
void nxp_soc_disp_get_resolution(int module, int *w, int *h)
{
	DISP_MULTILY_DEV(module, pmly);
	*w = pmly->x_resol;
	*h = pmly->y_resol;
}

void nxp_soc_disp_set_bg_color(int module, unsigned int color)
{
	DISP_MULTILY_DEV(module, pmly);
	pmly->bg_color = color;
	NX_MLC_SetBackground(module, color & 0x00FFFFFF);
	NX_MLC_SetTopDirtyFlag(module);
}

unsigned int nxp_soc_disp_get_bg_color(int module)
{
	DISP_MULTILY_DEV(module, pmly);
	return pmly->bg_color;
}

int nxp_soc_disp_wait_vertical_sync(int module)
{
	return disp_syncgen_waitsync(module, 0, 1);
}

void nxp_soc_disp_layer_set_enable (int module, int layer, int enable)
{
	if (MLC_LAYER_VIDEO == layer)
		nxp_soc_disp_video_set_enable(module, enable);
	else
		nxp_soc_disp_rgb_set_enable(module, layer, enable);
}

int  nxp_soc_disp_layer_stat_enable(int module, int layer)
{
	if (MLC_LAYER_VIDEO == layer)
		return nxp_soc_disp_video_stat_enable(module);
	else
		return nxp_soc_disp_rgb_stat_enable(module, layer);
}

/*
 *	Display Devices
 */
int	nxp_soc_disp_device_connect_to(enum disp_dev_type device,
				enum disp_dev_type to, struct disp_vsync_info *psync)
{
	struct disp_process_dev *pdev, *sdev;
	struct disp_process_ops *ops;
	struct list_head *head, *new, *obj;
	int ret = 0;

	RET_ASSERT_VAL(device != to, -EINVAL);
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	RET_ASSERT_VAL(to == DISP_DEVICE_SYNCGEN0 || to == DISP_DEVICE_SYNCGEN1 ||
				   to == DISP_DEVICE_RESCONV, -EINVAL);
	DBGOUT("%s: %s, in[%s]\n", __func__, dev_to_str(device), dev_to_str(to));

	sdev = get_display_ptr((int)to);
	pdev = get_display_ptr((int)device);
	sdev->dev_out = device;
	pdev->dev_in  = to;

	spin_lock(&sdev->lock);

	/* list add */
	if (DISP_DEVICE_SYNCGEN0 == sdev->dev_id ||
		DISP_DEVICE_SYNCGEN1 == sdev->dev_id) {
		struct disp_control_info *info = get_device_to_info(sdev);
		head = &info->link;
	} else {
		head = &disp_resconv_link;
	}

	/* check connect status */
	list_for_each(obj, head) {
		struct disp_process_dev *dev = container_of(obj,
					struct disp_process_dev, list);
		if (dev == pdev) {
			printk(KERN_ERR "Fail, %s is already connected to %s ...\n",
				dev_to_str(dev->dev_id), dev_to_str(sdev->dev_id));
			ret = -EINVAL;
			goto _exit;
		}
	}

	if (psync) {
		ops = sdev->disp_ops;	/* source sync */
		if (ops && ops->set_vsync) {
			ret = ops->set_vsync(sdev, psync);
			if (0 > ret)
				goto _exit;
		}

		ops = pdev->disp_ops;	/* device operation */
		if (ops && ops->set_vsync) {
			ret = ops->set_vsync(pdev, psync);
			if (0 > ret)
				goto _exit;
		}
	}

	new	= &pdev->list;
	list_add_tail(new, head);

_exit:
	spin_unlock(&sdev->lock);
	return ret;
}

void nxp_soc_disp_device_disconnect(enum disp_dev_type device, enum disp_dev_type to)
{
	struct disp_process_dev *pdev, *sdev;
	struct list_head *entry;

	RET_ASSERT(device != to);
	RET_ASSERT(DEVICE_SIZE > device);
	RET_ASSERT(to == DISP_DEVICE_SYNCGEN0 || to == DISP_DEVICE_SYNCGEN1 ||
			   to == DISP_DEVICE_RESCONV);
	DBGOUT("%s: %s, in[%s]\n", __func__, dev_to_str(device), dev_to_str(to));

	/* list delete */
	sdev = get_display_ptr((int)to);
	pdev = get_display_ptr((int)device);

	spin_lock(&sdev->lock);

	/* list add */
	entry = &pdev->list;
	list_del(entry);

	spin_unlock(&sdev->lock);
}

int nxp_soc_disp_device_set_vsync_info(enum disp_dev_type device, struct disp_vsync_info *psync)
{
	struct disp_process_ops *ops = NULL;
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	DBGOUT("%s: %s\n", __func__, dev_to_str(device));

	ops = get_display_ops(device);
	if (ops && ops->set_vsync)
		return ops->set_vsync(ops->dev, psync);

	return -1;
}

int nxp_soc_disp_device_get_vsync_info(enum disp_dev_type device, struct disp_vsync_info *psync)
{
	struct disp_process_ops *ops = NULL;
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	DBGOUT("%s: %s\n", __func__, dev_to_str(device));

	ops = get_display_ops(device);
	if (ops && ops->get_vsync)
		return ops->get_vsync(ops->dev, psync);

	return -1;
}

int	nxp_soc_disp_device_set_sync_param(enum disp_dev_type device, struct disp_syncgen_par *sync_par)
{
	struct disp_process_dev *pdev = get_display_ptr(device);
	struct disp_syncgen_par *pdst, *psrc;

	RET_ASSERT_VAL(sync_par, -EINVAL);
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	DBGOUT("%s: %s\n", __func__, dev_to_str(device));

	pdst = &pdev->sync_gen;
	psrc = sync_par;
	set_syncgen_param(psrc, pdst);

	return 0;
}

int	nxp_soc_disp_device_get_sync_param(enum disp_dev_type device, struct disp_syncgen_par *sync_par)
{
	struct disp_process_dev *pdev = get_display_ptr(device);
	int size = sizeof(struct disp_syncgen_par);

	RET_ASSERT_VAL(sync_par, -EINVAL);
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	DBGOUT("%s: %s\n", __func__, dev_to_str(device));

	memcpy(sync_par, &pdev->sync_gen, size);
	return 0;
}

int	nxp_soc_disp_device_set_dev_param(enum disp_dev_type device, void *param)
{
	struct disp_process_dev *pdev = get_display_ptr(device);

	RET_ASSERT_VAL(param, -EINVAL);
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	DBGOUT("%s: %s, param = %p \n", __func__, dev_to_str(device), pdev->dev_param);

	pdev->dev_param = param;
	return 0;
}

#define	no_prev(dev) 	(NULL == &dev->list.prev ? 1 : 0)
#define	no_next(dev)  	(NULL == &dev->list.next ? 1 : 0)

int nxp_soc_disp_device_suspend(enum disp_dev_type device)
{
	struct disp_process_ops *ops = NULL;
	struct disp_control_info *info;
	struct lcd_operation *lcd;
	int ret = 0;

	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	PM_DBGOUT("%s: %s\n", __func__, dev_to_str(device));

	ops = get_display_ops(device);

	/* LCD control: first */
	if  (DISP_DEVICE_SYNCGEN0 == device ||
		 DISP_DEVICE_SYNCGEN1 == device) {
	 	RET_ASSERT_VAL(ops, -EINVAL);
	 	info = get_device_to_info(ops->dev);
	 	lcd = info->lcd_ops;

		if (no_prev(ops->dev) && lcd && lcd->backlight_suspend)
			lcd->backlight_suspend(info->module, lcd->data);
	}

	if (ops && ops->suspend)
		ret = ops->suspend(ops->dev);

	/* LCD control: last */
	if  (DISP_DEVICE_SYNCGEN0 == device ||
		 DISP_DEVICE_SYNCGEN1 == device) {
		RET_ASSERT_VAL(ops, -EINVAL);
	 	info = get_device_to_info(ops->dev);
	 	lcd = info->lcd_ops;

		if (no_next(ops->dev) && lcd && lcd->lcd_suspend)
			lcd->lcd_suspend(info->module, lcd->data);
	}
	return ret;
}

#define	DISPLAY_TOP_RESET()	{	\
		if (!nxp_soc_peri_reset_status(RESET_ID_DISPLAY))	{	\
			disp_topctl_reset(), disp_syncgen_reset();	\
		}		\
	}

void nxp_soc_disp_device_resume(enum disp_dev_type device)
{
	struct disp_process_ops *ops = NULL;
	struct disp_control_info *info;
	struct lcd_operation *lcd;

	RET_ASSERT(DEVICE_SIZE > device);
	PM_DBGOUT("%s: %s\n", __func__, dev_to_str(device));

	DISPLAY_TOP_RESET();

	ops = get_display_ops(device);

	/* LCD control: first */
	if  (DISP_DEVICE_SYNCGEN0 == device ||
		 DISP_DEVICE_SYNCGEN1 == device) {
	 	RET_ASSERT(ops);
	 	info = get_device_to_info(ops->dev);
	 	lcd = info->lcd_ops;

		if (no_prev(ops->dev) && lcd && lcd->backlight_resume)
			lcd->backlight_resume(info->module, lcd->data);
	}

	if (ops && ops->resume)
		ops->resume(ops->dev);

	/* LCD control: last */
	if  (DISP_DEVICE_SYNCGEN0 == device ||
		 DISP_DEVICE_SYNCGEN1 == device) {
		RET_ASSERT(ops);
	 	info = get_device_to_info(ops->dev);
	 	lcd = info->lcd_ops;

		if (no_next(ops->dev) && lcd && lcd->lcd_resume)
			lcd->lcd_resume(info->module, lcd->data);
	}
}

int nxp_soc_disp_device_suspend_all(int module)
{
	struct disp_control_info *info = get_module_to_info(module);
	struct lcd_operation *lcd = info->lcd_ops;
	int ret = 0;

	RET_ASSERT_VAL(0 == module || 1 == module, -EINVAL);
	PM_DBGOUT("%s: display.%d\n", __func__, module);

	/* LCD control */
	if (lcd && lcd->backlight_suspend)
		lcd->backlight_suspend(module, lcd->data);

	if (list_empty(&info->link)) {
		PM_DBGOUT("display:%9s not connected display out ...\n",
			dev_to_str(((struct disp_process_dev *)
			get_display_ptr((0 == module ? DISP_DEVICE_SYNCGEN0 : DISP_DEVICE_SYNCGEN1)))->dev_id));
		return 0;
	}

	ret = disp_ops_suspend_devs(&disp_resconv_link, 1);		/* resolution convertor */
	if (ret)
		return ret;

	ret = disp_ops_suspend_devs(&info->link, 1);			/* from last */
	if (ret)
		return ret;

	/* LCD control */
	if (lcd && lcd->lcd_suspend)
		lcd->lcd_suspend(module, lcd->data);

	return 0;
}

void nxp_soc_disp_device_resume_all(int module)
{
	struct disp_control_info *info = get_module_to_info(module);
	struct lcd_operation *lcd = info->lcd_ops;

	RET_ASSERT(0 == module || 1 == module);
	PM_DBGOUT("%s: display.%d\n", __func__, module);

	DISPLAY_TOP_RESET();

	/* LCD control */
	if (lcd && lcd->lcd_resume)
		lcd->lcd_resume(module, lcd->data);

	/* device control */
	if (list_empty(&info->link)) {
		PM_DBGOUT("display:%9s not connected display out ...\n",
			dev_to_str(((struct disp_process_dev *)
			get_display_ptr((0 == module ? DISP_DEVICE_SYNCGEN0 : DISP_DEVICE_SYNCGEN1)))->dev_id));
		return;
	}

	/* pre_resume */
	disp_ops_pre_resume_devs(&disp_resconv_link);
	disp_ops_pre_resume_devs(&info->link);

	/* resume */
	disp_ops_suspend_devs(&info->link, 0);
	disp_ops_suspend_devs(&disp_resconv_link, 0);

	/* LCD control */
	if (lcd && lcd->backlight_resume)
		lcd->backlight_resume(module, lcd->data);
}

int nxp_soc_disp_device_enable(enum disp_dev_type device, int enable)
{
	struct disp_process_ops *ops = NULL;
	struct disp_control_info *info;
	struct lcd_operation *lcd;
	int ret = 0;
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);
	DBGOUT("%s: %s, %s\n", __func__, dev_to_str(device), enable?"ON":"OFF");

	ops = get_display_ops(device);

	/* LCD control: first */
	if  (DISP_DEVICE_SYNCGEN0 == device ||
		 DISP_DEVICE_SYNCGEN1 == device) {
	 	RET_ASSERT_VAL(ops, -EINVAL);
	 	info = get_device_to_info(ops->dev);
	 	lcd = info->lcd_ops;

		if (enable) {
			if (no_prev(ops->dev) && lcd && lcd->lcd_poweron)
				lcd->lcd_poweron(info->module, lcd->data);
		} else {
			if (no_prev(ops->dev) && lcd && lcd->backlight_off)
				lcd->backlight_off(info->module, lcd->data);
		}
	}

	if (ops && ops->enable)
		ret = ops->enable(ops->dev, enable);

	/* LCD control: last */
	if  (DISP_DEVICE_SYNCGEN0 == device ||
		 DISP_DEVICE_SYNCGEN1 == device) {
		RET_ASSERT_VAL(ops, -EINVAL);
	 	info = get_device_to_info(ops->dev);
	 	lcd = info->lcd_ops;

		if (enable) {
			if (no_next(ops->dev) && lcd && lcd->backlight_on)
				lcd->backlight_on(info->module, lcd->data);
		} else {
			if (no_next(ops->dev) && lcd && lcd->lcd_poweroff)
				lcd->lcd_poweroff(info->module, lcd->data);
		}
	}
	return ret;
}

int nxp_soc_disp_device_stat_enable(enum disp_dev_type device)
{
	struct disp_process_ops *ops = NULL;
	RET_ASSERT_VAL(DEVICE_SIZE > device, -EINVAL);

	ops = get_display_ops(device);
	if (ops && ops->stat_enable)
		return ops->stat_enable(ops->dev);

	return 0;
}

int nxp_soc_disp_device_enable_all(int module, int enable)
{
	struct disp_control_info *info = get_module_to_info(module);
	struct lcd_operation *lcd = info->lcd_ops;
	enum disp_dev_type device = (0 == module ? DISP_DEVICE_SYNCGEN0 : DISP_DEVICE_SYNCGEN1);
	int ret = 0;

	RET_ASSERT_VAL(0 == module || 1 == module, -EINVAL);
	DBGOUT("%s: display.%d, %s\n", __func__, module, enable?"ON":"OFF");

	/* LCD control */
	if (enable) {
		if (lcd && lcd->lcd_poweron)
			lcd->lcd_poweron(module, lcd->data);
	} else {
		if (lcd && lcd->backlight_off)
			lcd->backlight_off(module, lcd->data);
	}

	/* device control */
	if (list_empty(&info->link)) {
		device = (0 == module ? DISP_DEVICE_SYNCGEN0 : DISP_DEVICE_SYNCGEN1);
		printk("display:%9s not connected display out ...\n",
			dev_to_str(((struct disp_process_dev *)get_display_ptr(device))->dev_id));
		nxp_soc_disp_device_enable(device, 0);
		return 0;
	}

	if (enable) {
		ret = disp_ops_prepare_devs(&disp_resconv_link);
		if (ret)
			return -EINVAL;

		ret = disp_ops_prepare_devs(&info->link);
		if (ret)
			return -EINVAL;

		disp_ops_enable_devs(&info->link, 1);
		disp_ops_enable_devs(&disp_resconv_link, 1);

	} else {
		disp_ops_enable_devs(&disp_resconv_link, 0);
		disp_ops_enable_devs(&info->link, 0);
	}

	/* LCD control */
	if (enable) {
		if (lcd && lcd->backlight_on)
			lcd->backlight_on(module, lcd->data);
	} else {
		if (lcd && lcd->lcd_poweroff)
			lcd->lcd_poweroff(module, lcd->data);
	}

	return 0;
}

int nxp_soc_disp_device_enable_all_saved(int module, int enable)
{
	struct disp_control_info *info = get_module_to_info(module);
	struct lcd_operation *lcd = info->lcd_ops;
	enum disp_dev_type device = (0 == module ? DISP_DEVICE_SYNCGEN0 : DISP_DEVICE_SYNCGEN1);
	struct disp_process_dev *pdev = get_display_ptr(device);
	int ret = 0;

	RET_ASSERT_VAL(0 == module || 1 == module, -EINVAL);
	DBGOUT("%s: display.%d, %s\n", __func__, module, enable?"ON":"OFF");

	if (pdev->status & PROC_STATUS_ENABLE)
		return 0;

	/* LCD control */
	if (enable) {
		if (lcd && lcd->lcd_poweron)
			lcd->lcd_poweron(module, lcd->data);
	} else {
		if (lcd && lcd->backlight_off)
			lcd->backlight_off(module, lcd->data);
	}

	/* device control */
	if (list_empty(&info->link)) {
		device = (0 == module ? DISP_DEVICE_SYNCGEN0 : DISP_DEVICE_SYNCGEN1);
		printk("display:%9s not connected display out ...\n",
			dev_to_str(((struct disp_process_dev *)get_display_ptr(device))->dev_id));
		nxp_soc_disp_device_enable(device, 0);
		return 0;
	}

	if (enable) {
		ret = disp_ops_prepare_devs(&disp_resconv_link);
		if (ret)
			return -EINVAL;

		ret = disp_ops_prepare_devs(&info->link);
		if (ret)
			return -EINVAL;

		disp_multily_resume(pdev);	/* restore multiple layer */
		disp_ops_enable_devs(&info->link, 1);
		disp_ops_enable_devs(&disp_resconv_link, 1);

	} else {
		disp_multily_suspend(pdev);	/* save multiple layer */
		disp_ops_enable_devs(&disp_resconv_link, 0);
		disp_ops_enable_devs(&info->link, 0);
	}

	/* LCD control */
	if (enable) {
		if (lcd && lcd->backlight_on)
			lcd->backlight_on(module, lcd->data);
	} else {
		if (lcd && lcd->lcd_poweroff)
			lcd->lcd_poweroff(module, lcd->data);
	}

	return 0;
}

void nxp_soc_disp_device_reset_top(void)
{
	char *dev_str = "RESCONV, LCD, HDMI, LVDS, MiPi";
	printk(KERN_INFO "display: Reset top control \n");
	printk(KERN_INFO " - device selector(MUX) for %s\n", dev_str);
	printk(KERN_INFO " - clock gen for %s\n", dev_str);
	disp_topctl_reset();
}

void nxp_soc_disp_register_proc_ops(enum disp_dev_type device, struct disp_process_ops *ops)
{
	struct disp_process_dev *pdev = get_display_ptr(device);
	RET_ASSERT(DEVICE_SIZE > device);
	RET_ASSERT(device == pdev->dev_id);

	if (get_display_ops(device))
		printk(KERN_ERR "Warn , %s operation will be replaced \n", dev_to_str(device));

	spin_lock(&pdev->lock);

	/* set device info */
	set_display_ops (device, ops);

	spin_unlock(&pdev->lock);
	printk(KERN_INFO "Display %s register operation \n", dev_to_str(device));
}

void nxp_soc_disp_register_priv(enum disp_dev_type device, void *priv)
{
    struct disp_process_dev *pdev = get_display_ptr(device);
    RET_ASSERT(DEVICE_SIZE > device);
    RET_ASSERT(device == pdev->dev_id);

    pdev->priv = priv;
    DBGOUT("%s: %p\n", __func__, priv);
}

void nxp_soc_disp_register_lcd_ops(int module, struct lcd_operation *ops)
{
	DISP_CONTROL_INFO(module, info);
	info->lcd_ops = ops;
}

struct disp_irq_callback *nxp_soc_disp_register_irq_callback(int module, void (*callback)(void *), void *data)
{
    unsigned long flags;
    struct disp_irq_callback *entry = NULL;
    struct disp_control_info *info = get_module_to_info(module);
    RET_ASSERT_NULL(0 == module || 1 == module);
    RET_ASSERT_NULL(callback);

    DBGOUT("%s: display.%d\n", __func__, module);

    entry = (struct disp_irq_callback *)kmalloc(sizeof(struct disp_irq_callback), GFP_KERNEL);
    if (!entry) {
        printk("%s: failed to allocate disp_irq_callback entry\n", __func__);
        return NULL;
    }
    entry->handler = callback;
    entry->data = data;
    spin_lock_irqsave(&info->lock_callback, flags);
    list_add_tail(&entry->list, &info->callback_list);
    spin_unlock_irqrestore(&info->lock_callback, flags);
    return entry;
}

void nxp_soc_disp_unregister_irq_callback(int module, struct disp_irq_callback *callback)
{
    unsigned long flags;
    struct disp_control_info *info = get_module_to_info(module);
    RET_ASSERT(0 == module || 1 == module);

    DBGOUT("%s: display.%d\n", __func__, module);

    spin_lock_irqsave(&info->lock_callback, flags);
    list_del(&callback->list);
    spin_unlock_irqrestore(&info->lock_callback, flags);
    kfree(callback);
}

void nxp_soc_disp_device_framebuffer(int module, int fb)
{
	struct disp_control_info *info = get_module_to_info(module);
	struct disp_process_dev *pdev = info->proc_dev;

	RET_ASSERT(0 == module || 1 == module);
	pdev->dev_in = fb;
	printk("display.%d connected to fb.%d  ...\n", module, fb);
}

/* TOP Layer */
EXPORT_SYMBOL(nxp_soc_disp_get_resolution);
EXPORT_SYMBOL(nxp_soc_disp_set_bg_color);
EXPORT_SYMBOL(nxp_soc_disp_get_bg_color);
EXPORT_SYMBOL(nxp_soc_disp_wait_vertical_sync);
EXPORT_SYMBOL(nxp_soc_disp_layer_set_enable);
EXPORT_SYMBOL(nxp_soc_disp_layer_stat_enable);
/* RGB Layer */
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_fblayer);
EXPORT_SYMBOL(nxp_soc_disp_rgb_get_fblayer);
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_format);
EXPORT_SYMBOL(nxp_soc_disp_rgb_get_format);
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_position);
EXPORT_SYMBOL(nxp_soc_disp_rgb_get_position);
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_address);
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_clipping);
EXPORT_SYMBOL(nxp_soc_disp_rgb_get_clipping);
EXPORT_SYMBOL(nxp_soc_disp_rgb_get_address);
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_color);
EXPORT_SYMBOL(nxp_soc_disp_rgb_get_color);
EXPORT_SYMBOL(nxp_soc_disp_rgb_set_enable);
EXPORT_SYMBOL(nxp_soc_disp_rgb_stat_enable);
/* Video Layer */
EXPORT_SYMBOL(nxp_soc_disp_video_set_format);
EXPORT_SYMBOL(nxp_soc_disp_video_get_format);
EXPORT_SYMBOL(nxp_soc_disp_video_set_address);
EXPORT_SYMBOL(nxp_soc_disp_video_get_address);
EXPORT_SYMBOL(nxp_soc_disp_video_set_position);
EXPORT_SYMBOL(nxp_soc_disp_video_get_position);
EXPORT_SYMBOL(nxp_soc_disp_video_set_enable);
EXPORT_SYMBOL(nxp_soc_disp_video_stat_enable);
EXPORT_SYMBOL(nxp_soc_disp_video_set_priority);
EXPORT_SYMBOL(nxp_soc_disp_video_get_priority);
EXPORT_SYMBOL(nxp_soc_disp_video_set_colorkey);
EXPORT_SYMBOL(nxp_soc_disp_video_get_colorkey);
EXPORT_SYMBOL(nxp_soc_disp_video_set_color);
EXPORT_SYMBOL(nxp_soc_disp_video_get_color);
EXPORT_SYMBOL(nxp_soc_disp_video_set_hfilter);
EXPORT_SYMBOL(nxp_soc_disp_video_get_hfilter);
EXPORT_SYMBOL(nxp_soc_disp_video_set_vfilter);
EXPORT_SYMBOL(nxp_soc_disp_video_get_vfilter);
/* Device */
EXPORT_SYMBOL(nxp_soc_disp_device_connect_to);
EXPORT_SYMBOL(nxp_soc_disp_device_disconnect);
EXPORT_SYMBOL(nxp_soc_disp_device_get_vsync_info);
EXPORT_SYMBOL(nxp_soc_disp_device_get_sync_param);
EXPORT_SYMBOL(nxp_soc_disp_device_set_sync_param);
EXPORT_SYMBOL(nxp_soc_disp_device_set_dev_param);
EXPORT_SYMBOL(nxp_soc_disp_device_suspend);
EXPORT_SYMBOL(nxp_soc_disp_device_suspend_all);
EXPORT_SYMBOL(nxp_soc_disp_device_resume);
EXPORT_SYMBOL(nxp_soc_disp_device_resume_all);
EXPORT_SYMBOL(nxp_soc_disp_device_enable);
EXPORT_SYMBOL(nxp_soc_disp_device_stat_enable);
EXPORT_SYMBOL(nxp_soc_disp_device_enable_all);
EXPORT_SYMBOL(nxp_soc_disp_device_enable_all_saved);
EXPORT_SYMBOL(nxp_soc_disp_device_reset_top);
EXPORT_SYMBOL(nxp_soc_disp_register_lcd_ops);
EXPORT_SYMBOL(nxp_soc_disp_register_proc_ops);
EXPORT_SYMBOL(nxp_soc_disp_register_priv);

/*
 * Notify vertical sync en/disable
 *
 * /sys/devices/platform/display/active.N
 */
static ssize_t active_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	struct attribute *at = &attr->attr;
	struct disp_control_info *info;
	struct disp_process_dev *dev;
	const char *c;
	char *s = buf;
	int a, i, d[2], m = 0;

	c = &at->name[strlen("active.")];
	a = simple_strtoul(c, NULL, 10);

	for (i = 0; 2 > i; i++) {
		info = get_module_to_info(i);
		dev  = info->proc_dev;
		d[i] = dev->dev_in;
	}

	/* not fb */
	if (d[0] != DISP_DEVICE_END && d[1] != DISP_DEVICE_END)
		m = a;
	else /* 1 fb */
		m = (d[0] == a) ? 0 : 1;

	info = get_module_to_info(m);

	s += sprintf(s, "%d\n", info->active_notify);
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t active_store(struct device *pdev,
			struct device_attribute *attr, const char *buf, size_t n)
{
	struct attribute *at = &attr->attr;
	struct disp_control_info *info;
	struct disp_process_dev *dev;
	const char *c;
	int a, i, d[2], m = 0;
	int active = 0;

	c = &at->name[strlen("active.")];
	a = simple_strtoul(c, NULL, 10);

	for (i = 0; 2 > i; i++) {
		info = get_module_to_info(i);
		dev  = info->proc_dev;
		d[i] = dev->dev_in;
	}

	/* not fb */
	if (d[0] != DISP_DEVICE_END && d[1] != DISP_DEVICE_END)
		m = a;
	else /* 1 fb */
		m = (d[0] == a) ? 0 : 1;

	info = get_module_to_info(m);

	sscanf(buf,"%d", &active);
	info->active_notify = active ? 1 : 0;

	return n;
}

static struct device_attribute active0_attr =
	__ATTR(active.0, 0664, active_show, active_store);
static struct device_attribute active1_attr =
	__ATTR(active.1, 0664, active_show, active_store);

/*
 * Notify vertical sync timestamp
 *
 * /sys/devices/platform/display/vsync.N
 */
static ssize_t vsync_show(struct device *pdev,
		struct device_attribute *attr, char *buf)
{
	struct attribute *at = &attr->attr;
	struct disp_control_info *info;
	struct disp_process_dev *dev;
	const char *c;
	int a, i, d[2], m = 0;

	c = &at->name[strlen("vsync.")];
	a = simple_strtoul(c, NULL, 10);

	for (i = 0; 2 > i; i++) {
		info = get_module_to_info(i);
		dev  = info->proc_dev;
		d[i] = dev->dev_in;
	}

	/* not fb */
	if (d[0] != DISP_DEVICE_END && d[1] != DISP_DEVICE_END)
		m = a;
	else /* 1 fb */
		m = (d[0] == a) ? 0 : 1;

	info = get_module_to_info(m);

    return scnprintf(buf, PAGE_SIZE, "%llu\n", ktime_to_ns(info->time_stamp));
}

static struct device_attribute vblank0_attr =
	__ATTR(vsync.0, S_IRUGO | S_IWUSR, vsync_show, NULL);
static struct device_attribute vblank1_attr =
	__ATTR(vsync.1, S_IRUGO | S_IWUSR, vsync_show, NULL);

/*
 * Notify vertical sync timestamp
 *
 * /sys/devices/platform/display/fps.N
 */
static ssize_t fps_show(struct device *pdev,
		struct device_attribute *attr, char *buf)
{
	struct attribute *at = &attr->attr;
	struct disp_control_info *info;
	struct disp_process_dev *dev;
	const char *c;
	int a, i, d[2], m = 0;

	c = &at->name[strlen("vsync.")];
	a = simple_strtoul(c, NULL, 10);

	for (i = 0; 2 > i; i++) {
		info = get_module_to_info(i);
		dev  = info->proc_dev;
		d[i] = dev->dev_in;
	}

	/* not fb */
	if (d[0] != DISP_DEVICE_END && d[1] != DISP_DEVICE_END)
		m = a;
	else /* 1 fb */
		m = (d[0] == a) ? 0 : 1;

	info = get_module_to_info(m);

    return scnprintf(buf, PAGE_SIZE, "%ld.%3ld\n", info->fps/1000, info->fps%1000);
}

static struct device_attribute fps0_attr =
	__ATTR(fps.0, S_IRUGO | S_IWUSR, fps_show, NULL);
static struct device_attribute fps1_attr =
	__ATTR(fps.1, S_IRUGO | S_IWUSR, fps_show, NULL);

/* sys attribte group */
static struct attribute *attrs[] = {
	&active0_attr.attr,	&active1_attr.attr,
	&vblank0_attr.attr,	&vblank1_attr.attr,
	&fps0_attr.attr, &fps1_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = (struct attribute **)attrs,
};

static int display_soc_setup(int module, struct disp_process_dev *pdev,
			struct platform_device *pldev)
{
	struct disp_control_info *info = pdev->dev_info;
	struct disp_multily_dev *pmly = &info->multilayer;
	struct mlc_layer_info *layer = pmly->layer;
	int i = 0, ret;
	RET_ASSERT_VAL(info, -EINVAL)

	for (i = 0; MULTI_LAYER_NUM > i; i++, layer++) {
		if (LAYER_RGB_NUM > i) {
			sprintf(layer->name, "rgb.%d.%d", module, i);
		} else {
			/* video layers color */
			sprintf(layer->name, "vid.%d.%d", module, i);
			layer->color.alpha = 15;
			layer->color.bright = 0;
			layer->color.contrast = 0;
			layer->color.satura = 0;
			/* NOTE>
		 	* hue and saturation type is double (float point)
		 	* so set after enable VFP
		 	*/
		}
	}

	info->kobj = get_display_kobj(pdev->dev_id);
	info->module = module;
	info->wait_time = 4;
	info->condition = 0;
	info->irqno = (info->module == 0 ? IRQ_PHY_DPC_P : IRQ_PHY_DPC_S);
	INIT_WORK(&info->work, disp_syncgen_irq_work);

	init_waitqueue_head(&info->wait_queue);
    INIT_LIST_HEAD(&info->callback_list);
    spin_lock_init(&info->lock_callback);
	ret = request_irq(info->irqno, &disp_syncgen_irqhandler,
			IRQF_DISABLED, DEV_NAME_DISP, info);
	if (ret) {
		printk(KERN_ERR "Fail, display.%d request interrupt %d ...\n",
			info->module, info->irqno);
		return ret;
	}

	/* irq enable */
	disp_syncgen_irqenable(info->module, 1);

	/* init status */
	pmly->enable = NX_MLC_GetMLCEnable(module) ? 1 : 0;
	pdev->status = NX_DPC_GetDPCEnable(module) ? PROC_STATUS_ENABLE : 0;

	return ret;
}

static int display_soc_resume(struct platform_device *pldev)
{
	struct disp_process_dev *pdev = platform_get_drvdata(pldev);
	struct disp_control_info *info = pdev->dev_info;
	int module = info->module;

	PM_DBGOUT("%s [%d]\n", __func__, module);

	DISPLAY_TOP_RESET();

	NX_MLC_SetClockPClkMode(module, NX_PCLKMODE_ALWAYS);
	NX_MLC_SetClockBClkMode(module, NX_BCLKMODE_ALWAYS);

	/* BASE : DPC, PCLK */
	NX_DPC_SetBaseAddress(module, (void*)IO_ADDRESS(NX_DPC_GetPhysicalAddress(module)));
	NX_DPC_SetClockPClkMode(module, NX_PCLKMODE_ALWAYS);

	return 0;
}

/* suspend save register */
static int display_soc_probe(struct platform_device *pldev)
{
	struct disp_control_info *info;
	struct disp_process_dev *pdev = NULL;
	struct disp_multily_dev *pmly = NULL;
	struct disp_syncgen_par *psgen = NULL;
	int module = pldev->id;
	int size = sizeof(*info);
	int ret = 0;
	RET_ASSERT_VAL(0 == module || 1 == module, -EINVAL);

	size += sizeof(struct disp_syncgen_par);
	info = kzalloc(size, GFP_KERNEL);
	if (! info) {
		printk(KERN_ERR "Error, allocate memory (%d) for display.%d device \n",
			module, size);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&info->link);

	/* set syncgen device */
	pdev = get_display_ptr(DISP_DEVICE_SYNCGEN0 + module);
	list_add(&pdev->list, &info->link);

	pdev->dev_in  = DISP_DEVICE_END;
	pdev->dev_out = DISP_DEVICE_END;
	pdev->dev_info = (void*)info;
	pdev->dev_id = DISP_DEVICE_SYNCGEN0 + module;
	pdev->disp_ops  = &syncgen_ops[module];
	pdev->base_addr = (unsigned int)NX_DPC_GetBaseAddress(module);
	pdev->save_addr = (unsigned int)&save_syncgen[module];
	pdev->disp_ops->dev = pdev;

	psgen = &pdev->sync_gen;
	psgen->interlace = DEF_MLC_INTERLACE,
	psgen->out_format = DEF_OUT_FORMAT,
	psgen->invert_field = DEF_OUT_INVERT_FIELD,
	psgen->swap_RB = DEF_OUT_SWAPRB,
	psgen->yc_order = DEF_OUT_YCORDER,
	psgen->delay_mask = 0,
	psgen->vclk_select = DEF_PADCLKSEL,
	psgen->clk_delay_lv0 = DEF_CLKGEN0_DELAY,
	psgen->clk_inv_lv0 = DEF_CLKGEN0_INVERT,
	psgen->clk_delay_lv1 = DEF_CLKGEN1_DELAY,
	psgen->clk_inv_lv1 = DEF_CLKGEN1_INVERT,
	psgen->clk_sel_div1 = DEF_CLKSEL1_SELECT,

	/* set multilayer device */
	pmly = &info->multilayer;
	pmly->base_addr = (unsigned int)NX_MLC_GetBaseAddress(module);
	pmly->save_addr = (unsigned int)&save_multily[module];
	pmly->mem_lock_len = 16;	/* fix mem lock size, psgen->mem_lock_size */

	/* set control info */
	ret = display_soc_setup(module, pdev, pldev);
	if (0 > ret){
		kfree(info);
		return ret;
	}

	info->proc_dev = pdev;
	display_info[module] = info;

	/* set operation */
	set_display_ops(pdev->dev_id , pdev->disp_ops);

	platform_set_drvdata(pldev, pdev);
	return 0;
}

static struct platform_driver disp_driver = {
	.driver	= {
	.name	= DEV_NAME_DISP,
	.owner	= THIS_MODULE,
	},
	.probe	= display_soc_probe,
	.resume = display_soc_resume,
};

static int __init display_soc_initcall(void)
{
	struct kobject *kobj = NULL;
	int ret = 0;

	/* prototype and clock */
	disp_syncgen_initialize();

	/* create attribute interface */
	kobj = kobject_create_and_add("display", &platform_bus.kobj);
	if (! kobj) {
		printk(KERN_ERR "Fail, create kobject for display\n");
		return -ret;
	}

	ret = sysfs_create_group(kobj, &attr_group);
	if (ret) {
		printk(KERN_ERR "Fail, create sysfs group for display\n");
		kobject_del(kobj);
		return -ret;
	}
	set_display_kobj(kobj);

	return platform_driver_register(&disp_driver);
}
module_init(display_soc_initcall);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Display core driver for the Nexell");
MODULE_LICENSE("GPL");
