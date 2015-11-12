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
#include <linux/delay.h>
#include <mach/platform.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>

#include <mach/devices.h>
#include <mach/soc.h>
#include <mach/nxp-v4l2-platformdata.h>

#include "display_4418.h"

#include <nx_hdmi.h>
#include <nx_rstcon.h>
#include <nx_displaytop.h>
#include <nx_disptop_clkgen.h>
#include <nx_ecid.h>
#include <nx_tieoff.h>

#include <mach/hdmi/nxp-hdmi-context.h>

#if (0)
#define DBGOUT(msg...)		{ printk(KERN_INFO msg); }
#else
#define DBGOUT(msg...)		do {} while (0)
#endif

struct nxp_hdmi_display {
    struct nxp_hdmi_context ctx;
    struct nxp_lcd_plat_data *plat_data;
    enum disp_dev_type input;
};

#define context_to_nxp_hdmi_display(ctx) \
    container_of(ctx, struct nxp_hdmi_display, ctx);

static struct nxp_hdmi_display *__me = NULL;

static int hdmi_disp_get_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
    struct nxp_hdmi_display *me = (struct nxp_hdmi_display *)pdev->priv;
    pr_debug("%s entered\n", __func__);
    hdmi_get_vsync(&me->ctx);
    memcpy(psync, &me->ctx.dpc_sync_info, sizeof(struct disp_vsync_info));
    pr_debug("%s exit\n", __func__);
    return 0;
}

static int hdmi_disp_prepare(struct disp_process_dev *pdev)
{
    return 0;
}

static int hdmi_disp_enable(struct disp_process_dev *pdev, int enable)
{
    struct nxp_hdmi_display *me = (struct nxp_hdmi_display *)pdev->priv;
    pr_debug("%s entered: %d, %p\n", __func__, enable, me);
    if (enable) {
        if (hdmi_is_connected()) {
            hdmi_run(&me->ctx, false);
        }
    } else {
        hdmi_stop(&me->ctx);
    }
    return 0;
}

static int hdmi_disp_stat_enable(struct disp_process_dev *pdev)
{
    return pdev->status & PROC_STATUS_ENABLE ? 1 : 0;
}

static int hdmi_disp_suspend(struct disp_process_dev *pdev)
{
    struct nxp_hdmi_display *me = (struct nxp_hdmi_display *)pdev->priv;
    return hdmi_suspend(&me->ctx);
}

static void hdmi_disp_pre_resume(struct disp_process_dev *pdev)
{
}

static void hdmi_disp_resume(struct disp_process_dev *pdev)
{
    struct nxp_hdmi_display *me = (struct nxp_hdmi_display *)pdev->priv;
    hdmi_resume(&me->ctx);
}

static struct disp_process_ops hdmi_ops = {
    .get_vsync 		= hdmi_disp_get_vsync,
    .prepare 		= hdmi_disp_prepare,
    .enable 		= hdmi_disp_enable,
    .stat_enable 	= hdmi_disp_stat_enable,
    .suspend 		= hdmi_disp_suspend,
    .pre_resume		= hdmi_disp_pre_resume,
    .resume 		= hdmi_disp_resume,
};

static struct i2c_board_info hdmi_edid_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_edid", 0xA0>>1),
};

static struct nxp_v4l2_i2c_board_info edid = {
    .board_info = &hdmi_edid_i2c_boardinfo,
    .i2c_adapter_id = 0,
};

static struct i2c_board_info hdmi_hdcp_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_hdcp", 0x74>>1),
};

static struct nxp_v4l2_i2c_board_info hdcp = {
    .board_info = &hdmi_hdcp_i2c_boardinfo,
    .i2c_adapter_id = 0,
};

static void _set_hdmi_mux(struct nxp_hdmi_context *ctx)
{
    struct nxp_hdmi_display *me = context_to_nxp_hdmi_display(ctx);

    pr_debug("%s entered\n", __func__);
    nxp_soc_disp_device_set_vsync_info(ctx->source_device, &ctx->dpc_sync_info);
    nxp_soc_disp_device_set_sync_param(ctx->source_device, &ctx->dpc_sync_param);
    ctx->source_device = me->plat_data->display_in;
    if (ctx->source_device == DISP_DEVICE_SYNCGEN0) {
        NX_DISPLAYTOP_SetHDMIMUX(CTRUE, PrimaryMLC);
        ctx->source_dpc_module_num = 0;
        pr_debug("source device : SYNCGEN0\n");
    } else if (ctx->source_device == DISP_DEVICE_SYNCGEN1) {
        NX_DISPLAYTOP_SetHDMIMUX(CTRUE, SecondaryMLC);
        ctx->source_dpc_module_num = 1;
        pr_debug("source device : SYNCGEN1\n");
    } else {
        NX_DISPLAYTOP_SetHDMIMUX(CTRUE, ResolutionConv);
        ctx->source_dpc_module_num = 0;
        pr_debug("source device : RESC\n");
    }
    pr_debug("%s exit\n", __func__);
}

static void notify_hpd_changed(void *data, int state)
{
    struct nxp_hdmi_display *me = (struct nxp_hdmi_display *)data;

    printk("%s: state %d\n", __func__, state);

    if (state) {
        hdmi_run(&me->ctx, true);
    } else {
        hdmi_stop(&me->ctx);
    }
}

static int hdmi_probe(struct platform_device *pdev)
{
    struct nxp_lcd_plat_data *plat = pdev->dev.platform_data;
    struct nxp_hdmi_display *me = NULL;
    u32 preferred_preset;
    int ret = 0;

    RET_ASSERT_VAL(plat, -EINVAL);
    RET_ASSERT_VAL(plat->display_in == DISP_DEVICE_SYNCGEN0 ||
            plat->display_in == DISP_DEVICE_SYNCGEN1 ||
            plat->display_dev == DISP_DEVICE_HDMI ||
            plat->display_in == DISP_DEVICE_RESCONV, -EINVAL);

    me = (struct nxp_hdmi_display *)kzalloc(sizeof(struct nxp_hdmi_display), GFP_KERNEL);
    if (!me) {
        printk(KERN_ERR "failed to allocation nxp_hdmi_display\n");
        return -ENOMEM;
    }

    ret = hdmi_init_context(&me->ctx, &edid, &hdcp);
    if (ret) {
        printk(KERN_ERR "failed to hdmi_init_context()\n");
        kfree(me);
        return ret;
    }

#if defined(CONFIG_NXP_DISPLAY_HDMI_1280_720P)
    preferred_preset = V4L2_DV_720P60;
#elif defined(CONFIG_NXP_DISPLAY_HDMI_1920_1080P)
    preferred_preset = V4L2_DV_1080P60;
#else
#error "***** NOT SPECIFIED HDMI RESOLUTION !!! *****"
#endif

    if (hdmi_is_connected())
        preferred_preset = hdmi_get_edid_preset(&me->ctx, preferred_preset);

    hdmi_set_preset(&me->ctx, preferred_preset);

    me->plat_data = plat;

    /* sync param initialize */
    me->ctx.source_device = me->plat_data->display_in;
    hdmi_get_vsync(&me->ctx);
    hdmi_hook_set_mux(&me->ctx, _set_hdmi_mux);
    hdmi_register_notifier_hpd_changed(&me->ctx, notify_hpd_changed, me);

    nxp_soc_disp_register_priv(DISP_DEVICE_HDMI, (void *)me);
    nxp_soc_disp_register_proc_ops(DISP_DEVICE_HDMI, &hdmi_ops);
    nxp_soc_disp_device_connect_to(DISP_DEVICE_HDMI, me->ctx.source_device, &me->ctx.dpc_sync_info);

    __me = me;

    pr_debug("%s exit\n", __func__);
    return 0;
}

static struct platform_driver hdmi_driver = {
    .driver	= {
        .name	= DEV_NAME_HDMI,
        .owner	= THIS_MODULE,
    },
    .probe	= hdmi_probe,
};
module_platform_driver(hdmi_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Display HDMI driver for the Nexell");
MODULE_LICENSE("GPL");
