#define DEBUG 1

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

#include <mach/nxp-v4l2-platformdata.h>

#include "nxp-v4l2.h"

#include "regs-hdmi.h"
#include "hdmi-priv.h"
#include "hdmi-preset.h"
#include "nxp-hdmiphy.h"
#include "nxp-edid.h"
#include "nxp-hdcp.h"

#include "nxp-hdmi.h"

#include <mach/platform.h>


#define DEFAULT_SAMPLE_RATE	    44100
#define DEFAULT_BITS_PER_SAMPLE	16
#define DEFAULT_AUDIO_CODEC	    HDMI_AUDIO_PCM

#define subdev_to_nxp_hdmi(sd) \
    container_of(sd, struct nxp_hdmi, sd);

/**
 * tv out
 */
#include <linux/delay.h>
/* #include <mach/nxp3200.h> */
#include <mach/soc.h>

struct nx_display {
    u32     dwOutputMode;
    bool   bInterlace;
    bool   bInvertField;

    u32     dwClockSource;
    u32     dwClockDivider;
    u32     dwClockDelay;
    bool   bClockRisingEdge;
    bool   bDualEdge;

    bool   bDualView;

    u32     dwHorActive;
    u32     dwHorFrontPorch;
    u32     dwHorSyncWidth;
    u32     dwHorBackPorch;
    bool   bHorSyncHighActive;

    u32     dwVerActive;
    u32     dwVerFrontPorch;
    u32     dwVerSyncWidth;
    u32     dwVerBackPorch;
    bool   bVerSyncHighActive;

    u32     dwEvenVerActive;
    u32     dwEvenVerFrontPorch;
    u32     dwEvenVerSyncWidth;
    u32     dwEvenVerBackPorch;

    u32     dwVerSyncStartOffset;
    u32     dwVerSyncEndOffset;
    u32     dwEvenVerSyncStartOffset;
    u32     dwEvenVerSyncEndOffset;
};

struct nx_encoder_mode {
    u32     dwBroadcast;
    bool   bPedestal;
    u32     dwYBandWidth;
    u32     dwCBandWidth;

    u32     dwHorSyncStart;
    u32     dwHorSyncEnd;
    u32     dwVerSyncStart;
    u32     dwVerSyncEnd;
};

struct nx_display_mode {
    struct nx_display *lcd;
    struct nx_encoder_mode *mode;
};

struct nx_display ntsc_lcd = {
    .dwOutputMode = (u32)NX_DPC_FORMAT_CCIR601B,
    .bInterlace = true,
    .bInvertField = false,

    .dwClockSource = 5,
    .dwClockDivider = 1,
    .dwClockDelay = 0,
    .bClockRisingEdge = false,
    .bDualEdge = false,

    .bDualView = false,

    .dwHorActive = 720,
    .dwHorFrontPorch = 16,
    .dwHorSyncWidth = 33,
    .dwHorBackPorch = 90,
    .bHorSyncHighActive = false,

    .dwVerActive = 240,
    .dwVerFrontPorch = 4,
    .dwVerSyncWidth = 3,
    .dwVerBackPorch = 15,
    .bVerSyncHighActive = false,

    .dwEvenVerActive = 240,
    .dwEvenVerFrontPorch = 4,
    .dwEvenVerSyncWidth = 3,
    .dwEvenVerBackPorch = 15,

    .dwVerSyncStartOffset = 0,
    .dwVerSyncEndOffset = 0,
    .dwEvenVerSyncStartOffset = 0,
    .dwEvenVerSyncEndOffset = 0
};

struct nx_encoder_mode ntsc_mode = {
    .dwBroadcast = (u32)NX_DPC_VBS_NTSC_M,
    .bPedestal = true,
    .dwYBandWidth = (u32)NX_DPC_BANDWIDTH_LOW,
    .dwCBandWidth = (u32)NX_DPC_BANDWIDTH_LOW,

    .dwHorSyncStart = 63,
    .dwHorSyncEnd = 1715,
    .dwVerSyncStart = 0,
    .dwVerSyncEnd = 3
};

struct nx_display_mode tv_out_ntsc = {
    .lcd = &ntsc_lcd,
    .mode = &ntsc_mode
};

struct nx_display pal_lcd = {
    .dwOutputMode = (u32)NX_DPC_FORMAT_CCIR601B,
    .bInterlace = true,
    .bInvertField = false,

    .dwClockSource = 5,
    .dwClockDivider = 1,
    .dwClockDelay = 0,
    .bClockRisingEdge = false,
    .bDualEdge = false,

    .bDualView = false,

    .dwHorActive = 720,
    .dwHorFrontPorch = 12,
    .dwHorSyncWidth = 42,
    .dwHorBackPorch = 90,
    .bHorSyncHighActive = false,

    .dwVerActive = 288,
    .dwVerFrontPorch = 4,
    .dwVerSyncWidth = 2,
    .dwVerBackPorch = 21,
    .bVerSyncHighActive = false,

    .dwEvenVerActive = 288,
    .dwEvenVerFrontPorch = 4,
    .dwEvenVerSyncWidth = 2,
    .dwEvenVerBackPorch = 21,

    .dwVerSyncStartOffset = 0,
    .dwVerSyncEndOffset = 0,
    .dwEvenVerSyncStartOffset = 0,
    .dwEvenVerSyncEndOffset = 0
};

struct nx_encoder_mode pal_mode = {
    .dwBroadcast = (u32)NX_DPC_VBS_PAL_BGHI,
    .bPedestal = false,
    .dwYBandWidth = (u32)NX_DPC_BANDWIDTH_LOW,
    .dwCBandWidth = (u32)NX_DPC_BANDWIDTH_LOW,

    .dwHorSyncStart = 83,
    .dwHorSyncEnd = 1727,
    .dwVerSyncStart = 0,
    .dwVerSyncEnd = 2
};

struct nx_display_mode tv_out_pal = {
    .lcd = &pal_lcd,
    .mode = &pal_mode
};

static int _hw_set_tvout(struct nx_display_mode *disp_mode)
{
#if 0
    struct nx_display *disp = disp_mode->lcd;
    struct nx_encoder_mode *mode = disp_mode->mode;

    u32 vclk_divider = 0, sync_delay = 0;
    NX_DPC_DITHER rdither, gdither, bdither;
    bool embedded_sync, rgb_mode;
    NX_DPC_PADCLK clock;

    rdither = gdither = bdither = NX_DPC_DITHER_BYPASS;
    embedded_sync = rgb_mode = false;

    NX_DPC_SetHorizontalUpScaler(1, false, 720, 720);

    NX_DPC_SetDPCEnable(1, false);
    NX_DPC_SetClockDivisorEnable(1, false);

    vclk_divider = 2;
    embedded_sync = false;
    sync_delay = 6 * vclk_divider;

    NX_DPC_SetClockSource(1, 0, disp->dwClockSource);
    NX_DPC_SetClockDivisor(1, 0, disp->dwClockDivider);
    NX_DPC_SetClockOutDelay(1, 0, disp->dwClockDelay);

    NX_DPC_SetClockSource(1, 1, 7);
    NX_DPC_SetClockDivisor(1, 1, vclk_divider);
    NX_DPC_SetClockOutDelay(1, 1, disp->dwClockDelay);

    clock = NX_DPC_PADCLK_VCLK2;

    NX_DPC_SetMode(1,
            (NX_DPC_FORMAT)disp->dwOutputMode,
            disp->bInterlace,
            disp->bInvertField,
            rgb_mode,
            false,
            NX_DPC_YCORDER_CbYCrY,
            embedded_sync ? true : false,
            embedded_sync,
            clock,
            disp->bClockRisingEdge,
            disp->bDualView);
    NX_DPC_SetHSync(1,
            disp->dwHorActive,
            disp->dwHorSyncWidth,
            disp->dwHorFrontPorch,
            disp->dwHorBackPorch,
            disp->bHorSyncHighActive);
    NX_DPC_SetVSync(1,
            disp->dwVerActive,
            disp->dwVerSyncWidth,
            disp->dwVerFrontPorch,
            disp->dwVerBackPorch,
            disp->bVerSyncHighActive,
            disp->dwEvenVerActive,
            disp->dwEvenVerSyncWidth,
            disp->dwEvenVerFrontPorch,
            disp->dwEvenVerBackPorch);
    NX_DPC_SetVSyncOffset(1,
            disp->dwVerSyncStartOffset,
            disp->dwVerSyncEndOffset,
            disp->dwEvenVerSyncStartOffset,
            disp->dwEvenVerSyncEndOffset);
    NX_DPC_SetDelay(1, 0, sync_delay, sync_delay, sync_delay);
    NX_DPC_SetDither(1, NX_DPC_DITHER_BYPASS, NX_DPC_DITHER_BYPASS, NX_DPC_DITHER_BYPASS);

    NX_DPC_SetENCEnable(1, true);
    mdelay(1);
    NX_DPC_SetClockDivisorEnable(1, true);
    mdelay(1);
    NX_DPC_SetENCEnable(1, false);
    mdelay(1);
    NX_DPC_SetClockDivisorEnable(1, false);
    mdelay(1);

    NX_DPC_SetENCEnable(1, true);

    NX_DPC_SetVideoEncoderPowerDown(1, true);
    NX_DPC_SetVideoEncoderMode(1, (NX_DPC_VBS)mode->dwBroadcast, mode->bPedestal);
    NX_DPC_SetVideoEncoderFSCAdjust(1, 0);
    NX_DPC_SetVideoEncoderBandwidth(1, (NX_DPC_BANDWIDTH)mode->dwYBandWidth,
            (NX_DPC_BANDWIDTH)mode->dwCBandWidth);
    NX_DPC_SetVideoEncoderColorControl(1, 0, 0, 0, 0, 0);
    NX_DPC_SetVideoEncoderTiming(1,
            mode->dwHorSyncStart,
            mode->dwHorSyncEnd,
            mode->dwVerSyncStart,
            mode->dwVerSyncEnd);
    NX_DPC_SetVideoEncoderPowerDown(1, false);
    NX_DPC_SetENCEnable(1, true);

    NX_DPC_SetClockDivisorEnable(1, true);
    NX_DPC_SetDPCEnable(1, true);

    //psw0523 debugging
    //nxp_soc_disp_rgb_set_enable(1, 0, 1);
    //nxp_soc_disp_video_set_priority(1, 0);
    printk("%s: mlc enabled(%d), video layer enabled(%d), rgb layer enabled(%d), powermode(%d), interlace(%d)\n",
            __func__, NX_MLC_GetMLCEnable(1),
            NX_MLC_GetLayerEnable(1, MLC_LAYER_VIDEO),
            NX_MLC_GetLayerEnable(1, 0),
            NX_MLC_GetTopPowerMode(1), NX_MLC_GetFieldEnable(1));
    {
        int xresol, yresol;
        NX_MLC_GetScreenSize(1, &xresol, &yresol);
        printk("screen size(%dx%d)\n", xresol, yresol);
    }
#endif
    return 0;
}

static void _hw_disable_tvout(void)
{
#if 0
    NX_DPC_SetDPCEnable(1, false);
    NX_DPC_SetENCEnable(1, false);
    NX_DPC_SetVideoEncoderPowerDown(1, true);
    NX_DPC_SetClockDivisorEnable(1, false);
#endif
}

/**
 * internal function
 */
static const struct hdmi_preset_conf *_hdmi_preset2conf(u32 preset)
{
    return NULL;
}

static const struct hdmi_3d_info *_hdmi_preset2info(u32 preset)
{
    return NULL;
}

static void _hdmi_set_packets(struct nxp_hdmi *me)
{
}

/* TODO : define type */
static void _hdmi_clk_enable(struct nxp_hdmi *me, u32 type, bool enable)
{
    /* TODO */
}

static void _hdmi_reg_infoframe(struct nxp_hdmi *me,
        struct hdmi_infoframe *infoframe,
        const struct hdmi_3d_info *info_3d)
{
}

static int _hdmi_set_infoframe(struct nxp_hdmi *me)
{
    return 0;
}

static int _hdmi_streamon(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    _hw_set_tvout(&tv_out_ntsc);
    //_hw_set_tvout(&tv_out_pal);
    me->streaming = true;
    return 0;
}

static int _hdmi_streamoff(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    _hw_disable_tvout();
    me->streaming = false;
    return 0;
}

static int _hdmi_runtime_resume(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    return 0;
}

static int _hdmi_runtime_suspend(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    return 0;
}

/**
 * member function
 */
static bool _hdmi_is_streaming(struct nxp_hdmi *me)
{
    return me->streaming;
}

/**
 * subdev core ops
 */
static int nxp_hdmi_s_power(struct v4l2_subdev *sd, int on)
{
    int ret = 0;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    printk("%s\n", __func__);

    if (!me) {
        pr_err("%s: can't get me!!!\n", __func__);
        return -ENODEV;
    }

    if (on) {
        ret = _hdmi_runtime_resume(me);
        if (ret < 0) {
            pr_err("%s: failed to _hdmi_runtime_resume()\n", __func__);
            return ret;
        }

    } else {
        _hdmi_runtime_suspend(me);
    }

    return 0;
}

static int nxp_hdmi_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    printk("%s\n", __func__);

    switch (ctrl->id) {
    case V4L2_CID_HDMI_SET_DVI_MODE:
        mutex_lock(&me->mutex);
        me->is_dvi = !!ctrl->value;
        mutex_unlock(&me->mutex);
        break;

    case V4L2_CID_HDMI_SET_ASPECT_RATIO:
        mutex_lock(&me->mutex);
        me->aspect = ctrl->value;
        mutex_unlock(&me->mutex);
        break;

    case V4L2_CID_HDMI_ENABLE_HDMI_AUDIO:
        mutex_lock(&me->mutex);
        me->audio_enable = !!ctrl->value;
        mutex_unlock(&me->mutex);
        break;

    case V4L2_CID_HDMI_SET_NUM_CHANNELS:
        mutex_lock(&me->mutex);
        if ((ctrl->value == 2) || (ctrl->value == 6) ||
            (ctrl->value == 8)) {
            me->audio_channel_count = ctrl->value;
        } else {
            pr_err("%s: invalid channel count(%d)\n", __func__, ctrl->value);
        }
        mutex_unlock(&me->mutex);
        break;

    case V4L2_CID_HDMI_SET_COLOR_RANGE:
        mutex_lock(&me->mutex);
        me->color_range = ctrl->value;
        mutex_unlock(&me->mutex);
        break;

    default:
        pr_err("%s: invalid control id(0x%x)\n", __func__, ctrl->id);
        return -EINVAL;
    }

    return 0;
}

static int nxp_hdmi_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    switch (ctrl->id) {
    case V4L2_CID_HDMI_HPD_STATUS:
        ctrl->value = switch_get_state(&me->hpd_switch);
        break;

    case V4L2_CID_HDMI_GET_DVI_MODE:
        ctrl->value = me->is_dvi;
        break;

    case V4L2_CID_HDMI_MAX_AUDIO_CHANNELS:
        ctrl->value = me->edid.max_audio_channels(&me->edid);
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
    const struct hdmi_preset_conf *conf = _hdmi_preset2conf(preset->preset);
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    if (!conf) {
        pr_err("%s: preset(%u) not supported\n", __func__, preset->preset);
        return -EINVAL;
    }
    me->cur_conf = (struct hdmi_preset_conf *)conf;
    me->cur_preset = preset->preset;
    return 0;
}

static int nxp_hdmi_g_dv_preset(struct v4l2_subdev *sd,
        struct v4l2_dv_preset *preset)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);
    memset(preset, 0, sizeof(*preset));
    preset->preset = me->cur_preset;
    return 0;
}

static int nxp_hdmi_g_mbus_fmt(struct v4l2_subdev *sd,
        struct v4l2_mbus_framefmt *fmt)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    if (!me->cur_conf) {
        pr_err("%s: No current conf!!!\n", __func__);
        return -EINVAL;
    }

    *fmt = me->cur_conf->mbus_fmt;
    return 0;
}

static int nxp_hdmi_s_mbus_fmt(struct v4l2_subdev *sd,
        struct v4l2_mbus_framefmt *fmt)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    if (fmt->code == V4L2_MBUS_FMT_YUV8_1X24)
        me->output_fmt = HDMI_OUTPUT_YUV444; /* TODO : use this ? */
    else
        me->output_fmt = HDMI_OUTPUT_RGB888;

    return 0;
}

static int nxp_hdmi_enum_dv_presets(struct v4l2_subdev *sd,
        struct v4l2_dv_enum_preset *enum_preset)
{
    return 0;
}

static int nxp_hdmi_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    printk("%s: %d\n", __func__, enable);
    if (enable)
        return _hdmi_streamon(me);
    return _hdmi_streamoff(me);
}

static const struct v4l2_subdev_video_ops nxp_hdmi_video_ops = {
    .s_dv_preset = nxp_hdmi_s_dv_preset,
    .g_dv_preset = nxp_hdmi_g_dv_preset,
    .enum_dv_presets = nxp_hdmi_enum_dv_presets,
    .g_mbus_fmt  = nxp_hdmi_g_mbus_fmt,
    .s_mbus_fmt  = nxp_hdmi_s_mbus_fmt,
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
    printk("%s\n", __func__);
    return 0;
}

static const struct media_entity_operations nxp_hdmi_media_ops = {
    .link_setup = nxp_hdmi_link_setup,
};

/**
 * irq handler
 */
/* external irq */
static irqreturn_t _hdmi_irq_handler_ext(int irq, void *dev_data)
{
    struct nxp_hdmi *me = dev_data;

    int irqmode = nxp_soc_gpio_get_int_mode(me->external_irq);
    if (irqmode == 1) {
        nxp_soc_gpio_set_int_mode(me->external_irq, 0);
    } else {
        nxp_soc_gpio_set_int_mode(me->external_irq, 1);
    }

    printk("%s: irqmode(%d)\n", __func__, irqmode);

    queue_delayed_work(system_nrt_wq, &me->hpd_work_ext, 0);

    return IRQ_HANDLED;
}

/**
 * hpd work functions
 */
static void _hdmi_hpd_changed(struct nxp_hdmi *me, int state)
{
#if 0
    u32 preset;

    if (state == switch_get_state(&me->hpd_switch))
        return;

    if (state) {
        /* connected */
        preset = HDMI_DEFAULT_PRESET;

        me->is_dvi = false;
        me->cur_preset = preset;
        me->cur_conf = (struct hdmi_preset_conf *)_hdmi_preset2conf(preset);
    }

    switch_set_state(&me->hpd_switch, state);

    printk("%s: state(%d)\n", __func__, state);
#else
    return;
#endif
}

static void _hdmi_hpd_work_ext(struct work_struct *work)
{
#if 0
    int state;
    struct nxp_hdmi *me = container_of(work, struct nxp_hdmi,
            hpd_work_ext.work);
    state = me->read_hpd_gpio(me->external_irq);
    printk("%s: hpd state(%d)\n", __func__, state);
    _hdmi_hpd_changed(me, state);
#endif
}

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
    /* TODO */
    /* sd->grp_id = NXP_OUT_SUBDEV_GROUP_ID; */
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

    printk("%s\n", __func__);

    me = kzalloc(sizeof(*me), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to alloc me!!!\n", __func__);
        return NULL;
    }

    /* init media entity */
    ret = _init_entities(me);
    if (ret < 0) {
        pr_err("%s: failed to _init_entities()\n", __func__);
        goto error_init_entities;
    }

    /* set plat data */
    me->internal_irq = pdata->internal_irq;
    me->external_irq = pdata->external_irq;
    me->set_int_external = pdata->set_int_external;
    me->set_int_internal = pdata->set_int_internal;
    me->read_hpd_gpio    = pdata->read_hpd_gpio;

    /* member function */
    me->is_streaming = _hdmi_is_streaming;

    /* work */
    INIT_DELAYED_WORK(&me->hpd_work_ext, _hdmi_hpd_work_ext);

    /* etc */
    mutex_init(&me->mutex);

    /* set default value */
    me->audio_enable = false;
    me->audio_channel_count = 2;
    me->sample_rate = DEFAULT_SAMPLE_RATE;
    me->color_range = 3;
    me->bits_per_sample = DEFAULT_BITS_PER_SAMPLE;
    me->audio_codec = DEFAULT_AUDIO_CODEC;
    me->aspect = HDMI_ASPECT_RATIO_16_9;

    return me;

error_init_entities:
    kfree(me);
    return NULL;
}

void release_nxp_hdmi(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);

    media_entity_cleanup(&me->sd.entity);
    kfree(me);
}

/*
 * register irq
 * register switch dev
 */
int register_nxp_hdmi(struct nxp_hdmi *me)
{
    int ret;

    printk("%s\n", __func__);

    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(),
            &me->sd);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    /* HPD */
    me->hpd_switch.name = "hdmi";
    switch_dev_register(&me->hpd_switch);

#if 0
    nxp_soc_gpio_set_int_mode(me->external_irq, 1); /* high level */
    nxp_soc_gpio_set_int_enable(me->external_irq, 0);
    nxp_soc_gpio_clr_int_pend(me->external_irq);
    //nxp_soc_gpio_set_int_enable(me->external_irq, 1);

    ret = request_irq(PB_PIO_IRQ(me->external_irq), _hdmi_irq_handler_ext, 0,
            "hdmi-ext", me);
    if (ret < 0) {
        pr_err("%s: failed to request_irq(%d)\n", __func__, me->external_irq);
        goto error_req_irq_ext;
    }

    nxp_soc_gpio_set_int_enable(me->external_irq, 1);
#endif

    /* start HPD work */
    /* TODO: this is necessary ? */
    queue_delayed_work(system_nrt_wq, &me->hpd_work_ext, msecs_to_jiffies(1000));

    return 0;

error_req_irq_ext:
    switch_dev_unregister(&me->hpd_switch);

    pr_err("%s failed(ret=%d)\n", __func__, ret);
    return ret;
}

void unregister_nxp_hdmi(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    /* stop HPD work */
    cancel_delayed_work_sync(&me->hpd_work_ext);
    switch_dev_unregister(&me->hpd_switch);
    free_irq(me->external_irq, me);

    v4l2_device_unregister_subdev(&me->sd);
}

int suspend_nxp_hdmi(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    return 0;
}

int resume_nxp_hdmi(struct nxp_hdmi *me)
{
    printk("%s\n", __func__);
    return 0;
}
