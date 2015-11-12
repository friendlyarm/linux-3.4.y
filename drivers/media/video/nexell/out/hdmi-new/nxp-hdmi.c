#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/switch.h>
/* for private s_ctrl, g_ctrl */
#include <linux/videodev2_nxp_media.h>

#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#include <mach/platform.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>
#include <mach/hdmi/nxp-hdmi-context.h>

#include "nxp-v4l2.h"
#include "nxp-hdmi.h"

#define subdev_to_nxp_hdmi(sd) \
    container_of(sd, struct nxp_hdmi, sd);

#define context_to_nxp_hdmi(ctx) \
    container_of(ctx, struct nxp_hdmi, ctx);

static struct v4l2_subdev *_get_remote_source_subdev(struct nxp_hdmi *me)
{
    struct media_pad *pad =
        media_entity_remote_source(&me->pad);
    if (!pad)
        return NULL;
    return media_entity_to_v4l2_subdev(pad->entity);
}

static void _set_hdmi_mux(struct nxp_hdmi_context *ctx)
{
    struct v4l2_subdev *remote;
    struct nxp_hdmi *me;

    me = context_to_nxp_hdmi(ctx);
    remote = _get_remote_source_subdev(me);

    printk("%s: entered: me %p, remote %p\n", __func__, me, remote);
#if 1
    if (!strncmp(remote->name, "NXP MLC", 7)) {
        /* connect DPC -> HDMI */
        char *numchar = remote->name + 7;
        int module = *numchar - '0';
        if (module == 0) {
            ctx->source_device = DISP_DEVICE_SYNCGEN0;
            NX_DISPLAYTOP_SetHDMIMUX(CTRUE, PrimaryMLC);
            ctx->source_dpc_module_num = 0;
            printk("source device : SYNCGEN0\n");
        } else {
            ctx->source_device = DISP_DEVICE_SYNCGEN1;
            NX_DISPLAYTOP_SetHDMIMUX(CTRUE, SecondaryMLC);
            ctx->source_dpc_module_num = 1;
            printk("source device : SYNCGEN1\n");
        }
    } else {
        ctx->source_device = DISP_DEVICE_RESCONV;
        NX_DISPLAYTOP_SetHDMIMUX(CTRUE, ResolutionConv);
        // not meaningful, source dpc module checked by resolution converter
        ctx->source_dpc_module_num = 0;
        printk("source device : RESCONV\n");
    }
#else
    // test
    ctx->source_device = DISP_DEVICE_RESCONV;
    NX_DISPLAYTOP_SetHDMIMUX(CTRUE, ResolutionConv);
    // not meaningful, source dpc module checked by resolution converter
    ctx->source_dpc_module_num = 0;
    printk("source device : RESCONV\n");
#endif
    printk("%s: exit\n", __func__);
}

static int _hdmi_streamon(struct nxp_hdmi *me)
{
    int ret = hdmi_run(&me->ctx, true);
    if (ret)
        return ret;
    me->streaming = true;
    return 0;
}

static int _hdmi_streamoff(struct nxp_hdmi *me)
{
     me->streaming = false;
     hdmi_stop(&me->ctx);
     return 0;
}

static int nxp_hdmi_s_power(struct v4l2_subdev *sd, int on)
{
    return 0;
}

static int nxp_hdmi_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_hdmi_context *ctx;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    ctx = &me->ctx;

    pr_debug("%s\n", __func__);

    switch (ctrl->id) {
    case V4L2_CID_HDMI_SET_DVI_MODE:
        mutex_lock(&ctx->mutex);
        ctx->is_dvi = !!ctrl->value;
        mutex_unlock(&ctx->mutex);
        break;

    case V4L2_CID_HDMI_SET_ASPECT_RATIO:
        mutex_lock(&ctx->mutex);
        ctx->aspect = ctrl->value;
        mutex_unlock(&ctx->mutex);
        break;

    case V4L2_CID_HDMI_ENABLE_HDMI_AUDIO:
        mutex_lock(&ctx->mutex);
        hdmi_enable_audio(ctx, !!ctrl->value);
        mutex_unlock(&ctx->mutex);
        break;

    case V4L2_CID_HDMI_SET_NUM_CHANNELS:
        mutex_lock(&ctx->mutex);
        if ((ctrl->value == 2) || (ctrl->value == 6) ||
            (ctrl->value == 8)) {
            ctx->audio_channel_count = ctrl->value;
        } else {
            pr_err("%s: invalid channel count(%d)\n", __func__, ctrl->value);
        }
        hdmi_set_infoframe(ctx);
        mutex_unlock(&ctx->mutex);
        break;

    case V4L2_CID_HDMI_SET_COLOR_RANGE:
        mutex_lock(&ctx->mutex);
        ctx->color_range = ctrl->value;
        mutex_unlock(&ctx->mutex);
        break;

    case V4L2_CID_HDMI_ON_OFF:
        printk("%s: V4L2_CID_HDMI_ON_OFF val %d\n", __func__, ctrl->value);
        if (ctrl->value > 0) {
            u32 regval = NX_HDMI_GetReg(0, HDMI_LINK_HDMI_CON_0);
            regval |= 0x01;
            NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_0, regval);
        } else {
            u32 regval = NX_HDMI_GetReg(0, HDMI_LINK_HDMI_CON_0);
            regval &= ~0x01;
            NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_0, regval);
        }
        break;

    default:
        pr_err("%s: invalid control id(0x%x)\n", __func__, ctrl->id);
        return -EINVAL;
    }

    return 0;
}

static int nxp_hdmi_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_hdmi_context *ctx;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    ctx = &me->ctx;

    switch (ctrl->id) {
    case V4L2_CID_HDMI_HPD_STATUS:
        ctrl->value = switch_get_state(&ctx->hpd_switch);
        break;

    case V4L2_CID_HDMI_GET_DVI_MODE:
        ctrl->value = ctx->is_dvi;
        break;

    case V4L2_CID_HDMI_MAX_AUDIO_CHANNELS:
        ctrl->value = ctx->edid.max_audio_channels(&ctx->edid);
        break;

    default:
        pr_err("%s: invalid control id(0x%x)\n", __func__, ctrl->id);
        return -EINVAL;
    }

    return 0;
}

static const struct v4l2_subdev_core_ops nxp_hdmi_core_ops = {
    .s_power = nxp_hdmi_s_power,
    .s_ctrl  = nxp_hdmi_s_ctrl,
    .g_ctrl  = nxp_hdmi_g_ctrl,
};

/**
 * subdev video ops
 */
static int nxp_hdmi_s_dv_preset(struct v4l2_subdev *sd,
        struct v4l2_dv_preset *preset)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    return hdmi_set_dv_preset(&me->ctx, preset);
}

static int nxp_hdmi_g_dv_preset(struct v4l2_subdev *sd,
        struct v4l2_dv_preset *preset)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    memset(preset, 0, sizeof(*preset));
    preset->preset = me->ctx.cur_preset;
    return 0;
}

static int nxp_hdmi_g_mbus_fmt(struct v4l2_subdev *sd,
        struct v4l2_mbus_framefmt *fmt)
{
    struct nxp_hdmi_context *ctx;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    ctx = &me->ctx;

    if (!ctx->cur_conf) {
        pr_err("%s: No current conf!!!\n", __func__);
        return -EINVAL;
    }

    *fmt = ctx->cur_conf->mbus_fmt;
    return 0;
}

static int nxp_hdmi_enum_dv_presets(struct v4l2_subdev *sd,
        struct v4l2_dv_enum_preset *enum_preset)
{
    u32 preset;
    struct nxp_hdmi_context *ctx;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    ctx = &me->ctx;

    preset = ctx->edid.enum_presets(&ctx->edid, enum_preset->index);

    if (preset == V4L2_DV_INVALID) {
        printk("error invalid\n");
        return -EINVAL;
    }

    pr_debug("%s: supported %d\n", __func__, preset);
    return v4l_fill_dv_preset_info(preset, enum_preset);
}

static int nxp_hdmi_s_stream(struct v4l2_subdev *sd, int enable)
{
    int ret = 0;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    printk("%s: enable %d\n", __func__, enable);
    if (enable) {
        if (!me->streaming) {
#ifdef CONFIG_ANDROID
            wake_lock(&me->wake_lock);
#endif
            ret = _hdmi_streamon(me);
        }
    } else {
        if (me->streaming) {
            ret = _hdmi_streamoff(me);
#ifdef CONFIG_ANDROID
            wake_unlock(&me->wake_lock);
#endif
        }
    }

    return ret;
}

static const struct v4l2_subdev_video_ops nxp_hdmi_video_ops = {
    .s_dv_preset = nxp_hdmi_s_dv_preset,
    .g_dv_preset = nxp_hdmi_g_dv_preset,
    .enum_dv_presets = nxp_hdmi_enum_dv_presets,
    .g_mbus_fmt  = nxp_hdmi_g_mbus_fmt,
    .s_stream    = nxp_hdmi_s_stream,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops nxp_hdmi_subdev_ops = {
    .core = &nxp_hdmi_core_ops,
    .video = &nxp_hdmi_video_ops,
};

/**
 * entity ops
 */
static int nxp_hdmi_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    if (flags & MEDIA_LNK_FL_ENABLED) {
        /* on */
        printk("%s: connected to me!!!\n", __func__);
        return v4l2_subdev_call(sd, core, s_power, 1);
    } else {
        /* off */
        printk("%s: disconnected to me!!!\n", __func__);
        return v4l2_subdev_call(sd, core, s_power, 0);
    }
}

static const struct media_entity_operations nxp_hdmi_media_ops = {
    .link_setup = nxp_hdmi_link_setup,
};

/*
 * _init_entities
 */
static int _init_entities(struct nxp_hdmi *me)
{
    int ret;
    struct v4l2_subdev *sd = &me->sd;
    struct media_pad *pad  = &me->pad;
    struct media_entity *entity = &sd->entity;

    v4l2_subdev_init(sd, &nxp_hdmi_subdev_ops);

    strlcpy(sd->name, "NXP HDMI", sizeof(sd->name));
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pad->flags = MEDIA_PAD_FL_SINK;

    entity->ops = &nxp_hdmi_media_ops;
    ret = media_entity_init(entity, 1, pad, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

    return 0;
}

/**
 * public api
 */
struct nxp_hdmi *create_nxp_hdmi(struct nxp_hdmi_platformdata *pdata)
{
    int ret;
    struct nxp_hdmi *me;

    pr_debug("%s\n", __func__);

    me = kzalloc(sizeof(*me), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to alloc me!!!\n", __func__);
        return NULL;
    }

    /* init media entity */
    ret = _init_entities(me);
    if (ret < 0) {
        pr_err("%s: failed to _init_entities()\n", __func__);
        kfree(me);
        return NULL;
    }

#ifdef CONFIG_ANDROID
    wake_lock_init(&me->wake_lock, WAKE_LOCK_SUSPEND, "nxp-hdmi");
#endif

    /*me->plat_data = pdata;*/
    me->plat_data = (struct nxp_hdmi_platformdata *)kmalloc(sizeof(struct nxp_hdmi_platformdata), GFP_KERNEL);
    memcpy(me->plat_data, pdata, sizeof(struct nxp_hdmi_platformdata));

    return me;
}

void release_nxp_hdmi(struct nxp_hdmi *me)
{
    pr_debug("%s\n", __func__);

    hdmi_deinit_context(&me->ctx);
#ifdef CONFIG_ANDROID
    wake_lock_destroy(&me->wake_lock);
#endif
    media_entity_cleanup(&me->sd.entity);
    kfree(me->plat_data);
    kfree(me);
}

/*
 * register irq
 * register switch dev
 */
int register_nxp_hdmi(struct nxp_hdmi *me)
{
    int ret;
    pr_debug("%s\n", __func__);

    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(), &me->sd);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    ret = hdmi_init_context(&me->ctx, me->plat_data->edid, me->plat_data->hdcp);
    if (ret) {
        pr_err("%s: failed to hdmi_init_context()\n", __func__);
        return ret;
    }
    hdmi_hook_set_mux(&me->ctx, _set_hdmi_mux);

    return 0;
}

void unregister_nxp_hdmi(struct nxp_hdmi *me)
{
    pr_debug("%s\n", __func__);
    hdmi_deinit_context(&me->ctx);
    v4l2_device_unregister_subdev(&me->sd);
}

#ifdef CONFIG_PM
int suspend_nxp_hdmi(struct nxp_hdmi *me)
{
    return hdmi_suspend(&me->ctx);
}

int resume_nxp_hdmi(struct nxp_hdmi *me)
{
    return hdmi_resume(&me->ctx);
}
#endif
