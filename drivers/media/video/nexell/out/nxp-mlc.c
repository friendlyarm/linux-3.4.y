/* #define DEBUG 1 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/v4l2-mediabus.h>

/* for nexell specific control */
#include <linux/videodev2_nxp_media.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>

#include "nxp-video.h"
#include "nxp-v4l2.h"

#include "nxp-mlc.h"

/* for nexell specific prototype */
#include <mach/soc.h>
#include <mach/platform.h>
#include <mach/fourcc.h>

#define	RGB_0_LAYER				0	/* 0: RGB0 layer number */
#define	RGB_1_LAYER				1	/* 1: RGB1 layer number */
#define	RGB_2_LAYER				2	/* 2: RGB2 layer number */
#define	VIDEO_LAYER				3	/* 3: video layer number */

#define VID_BRIGHTNESS_MIN  -128
#define VID_SATURATION_MIN  -100
/*
 * supported formats
 */
static const unsigned int supported_vid_formats[] = {
    /* YUYV422 packed */
    V4L2_MBUS_FMT_YUYV8_2X8,
    /* YUYV420 3-plane */
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    /* YUYV422 3-plane */
    V4L2_MBUS_FMT_YUYV8_1X16,
    /* YUYV444 3-plane */
    V4L2_MBUS_FMT_YUV8_1X24,
};

static inline u32 _convert_nxp_to_v4l2_format(u32 nxp_format)
{
    switch (nxp_format) {
        /* yuv420 plane */
        case FOURCC_MVS0:
        case FOURCC_YV12:
            return V4L2_MBUS_FMT_YUYV8_1_5X8;
        /* yuv 422 plane */
        case FOURCC_MVS2:
        case FOURCC_YUY2:
            return V4L2_MBUS_FMT_YUYV8_1X16;
        /* yuv 444 plane */
        case FOURCC_MVS4:
            return V4L2_MBUS_FMT_YUV8_1X24;
        /* yuv 422 packed */
        case FOURCC_YUYV:
            return V4L2_MBUS_FMT_YUYV8_2X8;
        default:
            pr_err("%s: invalid nx format(0x%x)\n", __func__, nxp_format);
            return 0;
    }
}

static inline u32 _convert_v4l2_to_nxp_format(u32 v4l2_format)
{
    switch (v4l2_format) {
        case V4L2_MBUS_FMT_YUYV8_1_5X8:
            return FOURCC_MVS0;
        case V4L2_MBUS_FMT_YUYV8_1X16:
            return FOURCC_MVS2;
        case V4L2_MBUS_FMT_YUV8_1X24:
            return FOURCC_MVS4;
        case V4L2_MBUS_FMT_YUYV8_2X8:
            return FOURCC_YUYV;
        default:
            pr_err("%s: invalid v4l2 format(0x%x)\n", __func__, v4l2_format);
            return 0;
    }
}

/* TODO: duplicate */
static inline bool
_find_vid_format(const unsigned int array[], int array_size, unsigned int code)
{
    int i;
    for (i = 0; i < array_size; i++) {
        if (code == array[i])
            return true;
    }
    return false;
}

static inline u32 _get_pixel_byte(u32 nxp_rgb_format)
{
    switch (nxp_rgb_format) {
        case NX_MLC_RGBFMT_R5G6B5:
        case NX_MLC_RGBFMT_B5G6R5:
        case NX_MLC_RGBFMT_X1R5G5B5:
        case NX_MLC_RGBFMT_X1B5G5R5:
        case NX_MLC_RGBFMT_X4R4G4B4:
        case NX_MLC_RGBFMT_X4B4G4R4:
        case NX_MLC_RGBFMT_X8R3G3B2:
        case NX_MLC_RGBFMT_X8B3G3R2:
        case NX_MLC_RGBFMT_A1R5G5B5:
        case NX_MLC_RGBFMT_A1B5G5R5:
        case NX_MLC_RGBFMT_A4R4G4B4:
        case NX_MLC_RGBFMT_A4B4G4R4:
        case NX_MLC_RGBFMT_A8R3G3B2:
        case NX_MLC_RGBFMT_A8B3G3R2:
            return 2;

        case NX_MLC_RGBFMT_R8G8B8:
        case NX_MLC_RGBFMT_B8G8R8:
            return 3;

        case NX_MLC_RGBFMT_A8R8G8B8:
        case NX_MLC_RGBFMT_A8B8G8R8:
            return 4;

        default:
            pr_err("%s: invalid nxp_rgb_format(0x%x)\n", __func__, nxp_rgb_format);
            return 0;
    }
}

/**
 * hw specific
 */
static bool _hw_get_enabled(int id)
{
    return NX_DPC_GetDPCEnable(id);
}

/* return enabled */
static bool _hw_get_rgb_attr(int id, struct nxp_mlc_rgb_attr *attr)
{
    /* disable all attr */
    attr->alpha = -1;
    attr->transcolor = -1;
    attr->invertcolor = -1;
    attr->backcolor = -1;
    return false;
}

/* return enabled */
static bool _hw_get_vid_attr(int id, struct nxp_mlc_vid_attr *attr)
{
    /* disable all attr */
    attr->colorkey = -1;
    attr->alpha = -1;
    attr->brightness = -1;
    attr->hue = -1;
    attr->contrast = -1;
    attr->saturation = -1;
    attr->hfilter_enable = -1;
    attr->vfilter_enable = -1;

    return false;
}

static void _hw_set_rgb_addr(struct nxp_mlc *me, struct nxp_video_buffer *buf)
{
    int layer = RGB_0_LAYER;
    // TODO, pixelbyte, stride
    pr_debug("%s: %d, addr(0x%x), pixelbyte(%d), stride(%d)\n", __func__, me->id, buf->dma_addr[0],
            me->rgb_attr.pixelbyte, buf->stride[0]);
    nxp_soc_disp_rgb_set_address(me->id, layer, /* module, layer */
            buf->dma_addr[0],  /* addr */
            me->rgb_attr.pixelbyte, buf->stride[0], /* pixelbyte, stride */
            false); /* waitsync */
}

static void _hw_set_video_addr(struct nxp_mlc *me, struct nxp_video_buffer *buf)
{
    pr_debug("%s: buf(0x%x)\n", __func__, buf->dma_addr[0]);
    /* printk("%s: bufs 0x%x, 0x%x, 0x%x, stride %d, %d, %d\n", */
    /*         __func__, buf->dma_addr[0], buf->dma_addr[1], buf->dma_addr[2], */
    /*         buf->stride[0], buf->stride[1], buf->stride[2]); */
    nxp_soc_disp_video_set_address(me->id, /* module */
            buf->dma_addr[0], buf->stride[0], /* lu_a, lu_s */
            buf->dma_addr[1], buf->stride[1], /* cb_a, cb_s */
            buf->dma_addr[2], buf->stride[2], /* cr_a, cr_s */
            false);
}

static void _hw_rgb_enable(struct nxp_mlc *me, bool enable)
{
    printk("%s: module %d, enable %d\n", __func__, me->id, enable);
    nxp_soc_disp_rgb_set_enable(me->id, RGB_0_LAYER, enable);
}

static void _hw_video_enable(struct nxp_mlc *me, bool enable)
{
    // psw0523 debugging
    printk("%s: module %d, enable %d\n", __func__, me->id, enable);
    nxp_soc_disp_video_set_enable(me->id, enable);
}

static void _hw_configure_rgb(struct nxp_mlc *me)
{
    int id = me->id;
    struct nxp_mlc_rgb_attr *attr = &me->rgb_attr;
    int layer = RGB_0_LAYER;

    printk("%s %d: code(0x%x), w(%d), h(%d), pixelbyte(%d)\n", __func__, id,
            attr->format.code, attr->format.width, attr->format.height, _get_pixel_byte(attr->format.code));
    nxp_soc_disp_rgb_set_format(id, layer,
            attr->format.code,
            attr->format.width, attr->format.height,
            _get_pixel_byte(attr->format.code));

    nxp_soc_disp_set_bg_color(id, 0x00000000);
    nxp_soc_disp_rgb_set_position(id, layer, 0, 0, false);

    // psw0523 debugging
    printk("%s %d crop(%d:%d:%d:%d)\n",
            __func__, id, attr->crop.left, attr->crop.top, attr->crop.width, attr->crop.height);
    nxp_soc_disp_rgb_set_clipping(id, layer, attr->crop.left, attr->crop.top, attr->crop.width, attr->crop.height);

    /* layer enable */
    _hw_rgb_enable(me, true);
}

static void _hw_configure_video(struct nxp_mlc *me)
{
    int id = me->id;
    struct nxp_mlc_vid_attr *attr = &me->vid_attr;

    nxp_soc_disp_video_set_format(id,
            _convert_v4l2_to_nxp_format(attr->format.code),
            attr->format.width, attr->format.height);

    // psw0523 debugging
    printk("%s %d: fw(%d), fh(%d), crop(%d:%d:%d:%d), priority(%d), format(0x%x)\n",
            __func__, id, attr->format.width, attr->format.height,
            attr->crop.left, attr->crop.top, attr->crop.width, attr->crop.height,
            attr->priority, _convert_v4l2_to_nxp_format(attr->format.code));
    if (attr->crop.width > 0 && attr->crop.height > 0)
        nxp_soc_disp_video_set_position(id,  /* module */
                attr->crop.left, attr->crop.top, /* left, top */
                attr->crop.left + attr->crop.width, /* right */
                attr->crop.top + attr->crop.height, /* bottom */
                false); /* waitsync */

    if (attr->source_crop.width > 0 && attr->source_crop.height > 0)
        nxp_soc_disp_video_set_crop(me->id, true,
                attr->source_crop.left,
                attr->source_crop.top,
                attr->source_crop.width,
                attr->source_crop.height, true);
    else
        nxp_soc_disp_video_set_crop(me->id, false, 0, 0, 0, 0, true);

    nxp_soc_disp_video_set_priority(id, attr->priority);

    /* layer enable */
    _hw_video_enable(me, true);
}

static void _hw_enable(struct nxp_mlc *me, bool enable)
{
    pr_debug("%s: enable %d\n", __func__, enable);
    nxp_soc_disp_device_enable_all(me->id, enable);
}

/**
 * buffer operations
 */
static void _done_buf(struct nxp_video_buffer *buf, bool updated)
{
    if (updated)
        buf->consumer_index++;
    buf->cb_buf_done(buf);
}

static void _irq_callback(void *data)
{
    struct nxp_mlc *me = (struct nxp_mlc *)data;
    spin_lock(&me->vlock);
    if (me->old_vid_buf)
        _done_buf(me->old_vid_buf, true);
    me->old_vid_buf = NULL;
    spin_unlock(&me->vlock);
}

static void _clear_all_rgb_buf(struct nxp_mlc *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf = NULL;

    spin_lock_irqsave(&me->rlock, flags);
    while (!list_empty(&me->rgb_buffer_list)) {
        buf = list_entry(me->rgb_buffer_list.next,
                struct nxp_video_buffer, list);
        list_del_init(&buf->list);
    }
    INIT_LIST_HEAD(&me->rgb_buffer_list);
    spin_unlock_irqrestore(&me->rlock, flags);
}

static void _update_rgb_buffer(struct nxp_mlc *me)
{
    struct nxp_video_buffer *buf;
    unsigned long flags;

    pr_debug("%s\n", __func__);

    spin_lock_irqsave(&me->rlock, flags);
    if (list_empty(&me->rgb_buffer_list)) {
        spin_unlock_irqrestore(&me->rlock, flags);
        pr_err("%s: buf not ready!!!\n", __func__);
        return;
    }
    buf = list_first_entry(&me->rgb_buffer_list,
            struct nxp_video_buffer, list);
    list_del_init(&buf->list);
    spin_unlock_irqrestore(&me->rlock, flags);

    _hw_set_rgb_addr(me, buf);

    if (!me->rgb_enabled) {
        _hw_configure_rgb(me);
        me->rgb_enabled = true;
    }

    if (!me->enabled) {
        _hw_enable(me, true);
        me->enabled = true;
    }

    _done_buf(buf, true);
}

static int mlc_rgb_buffer_queue(struct nxp_video_buffer *buf, void *data)
{
    unsigned long flags;
    struct nxp_mlc *me = data;

    spin_lock_irqsave(&me->rlock, flags);
    list_add_tail(&buf->list, &me->rgb_buffer_list);
    spin_unlock_irqrestore(&me->rlock, flags);

    if (me->rgb_streaming) {
        _update_rgb_buffer(me);
    }

    return 0;
}

static void _clear_all_vid_buf(struct nxp_mlc *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf = NULL;

    spin_lock_irqsave(&me->vlock, flags);
    while (!list_empty(&me->vid_buffer_list)) {
        buf = list_entry(me->vid_buffer_list.next,
                struct nxp_video_buffer, list);
        _done_buf(buf, true);
        list_del_init(&buf->list);
    }
    INIT_LIST_HEAD(&me->vid_buffer_list);
    spin_unlock_irqrestore(&me->vlock, flags);
}

static void _update_vid_buffer(struct nxp_mlc *me)
{
    struct nxp_video_buffer *buf;
    unsigned long flags;

    pr_debug("%s\n", __func__);

    spin_lock_irqsave(&me->vlock, flags);
    if (list_empty(&me->vid_buffer_list)) {
        spin_unlock_irqrestore(&me->vlock, flags);
        pr_err("%s: buf not ready!!!\n", __func__);
        return;
    }
    buf = list_first_entry(&me->vid_buffer_list,
            struct nxp_video_buffer, list);
    list_del_init(&buf->list);
    if (me->cur_vid_buf)
        me->old_vid_buf = me->cur_vid_buf;
    else
        me->old_vid_buf = NULL;
    me->cur_vid_buf = buf;
    spin_unlock_irqrestore(&me->vlock, flags);

    if (!me->vid_enabled) {
        pr_debug("%s: register irq callback\n", __func__);
        me->callback = nxp_soc_disp_register_irq_callback(me->id, _irq_callback, me);
        _hw_configure_video(me);
        me->vid_enabled = true;
    }

    _hw_set_video_addr(me, buf);

    if (!me->enabled) {
        _hw_enable(me, true);
        me->enabled = true;
    }
}

static int mlc_vid_buffer_queue(struct nxp_video_buffer *buf, void *data)
{
    unsigned long flags;
    struct nxp_mlc *me = data;

    spin_lock_irqsave(&me->vlock, flags);
    list_add_tail(&buf->list, &me->vid_buffer_list);
    spin_unlock_irqrestore(&me->vlock, flags);

    pr_debug("%s: %d, %p\n", __func__, me->vid_streaming, buf);
    if (me->vid_streaming) {
        _update_vid_buffer(me);
    }

    return 0;
}

static int _register_rgb_buffer_consumer(struct nxp_mlc *me,
        struct nxp_video *video, u32 type)
{
    int ret;

    if (!me->rgb_consumer) {
        me->rgb_consumer = kzalloc(sizeof(struct nxp_buffer_consumer), GFP_KERNEL);
        if (!me->rgb_consumer) {
            pr_err("%s: failed to alloc rgb consumer\n", __func__);
            return -ENOMEM;
        }
    }

    me->rgb_consumer->priv  = me;
    me->rgb_consumer->queue = mlc_rgb_buffer_queue;
    ret = video->register_buffer_consumer(video, me->rgb_consumer, type);
    if (ret < 0) {
        pr_err("%s: failed to video->register_buffer_consumer()\n", __func__);
        return ret;
    }

    me->rgb_consumer->usage_count++;
    return 0;
}

static void  _unregister_rgb_buffer_consumer(struct nxp_mlc *me,
        struct nxp_video *video, u32 type)
{
    if (me->rgb_consumer) {
        if (me->rgb_consumer->usage_count > 0) {
            video->unregister_buffer_consumer(video, me->rgb_consumer, type);
            me->rgb_consumer->usage_count--;
        }
        if (me->rgb_consumer->usage_count <= 0) {
            kfree(me->rgb_consumer);
            me->rgb_consumer = NULL;
        }
    }
}

static int _register_vid_buffer_consumer(struct nxp_mlc *me,
        struct nxp_video *video, u32 type)
{
    int ret;

    if (!me->vid_consumer) {
        me->vid_consumer = kzalloc(sizeof(struct nxp_buffer_consumer), GFP_KERNEL);
        if (!me->vid_consumer) {
            pr_err("%s: failed to alloc video consumer\n", __func__);
            return -ENOMEM;
        }
    }

    me->vid_consumer->priv  = me;
    me->vid_consumer->queue = mlc_vid_buffer_queue;
    ret = video->register_buffer_consumer(video, me->vid_consumer, type);
    if (ret < 0) {
        pr_err("%s: failed to video->register_buffer_consumer()\n", __func__);
        return ret;
    }

    me->vid_consumer->usage_count++;

    return 0;
}

static void _unregister_vid_buffer_consumer(struct nxp_mlc *me,
        struct nxp_video *video, u32 type)
{
    if (me->vid_consumer) {
        if (me->vid_consumer->usage_count > 0) {
            video->unregister_buffer_consumer(video, me->vid_consumer, type);
            me->vid_consumer->usage_count--;
        }
        if (me->vid_consumer->usage_count <= 0) {
            kfree(me->vid_consumer);
            me->vid_consumer = NULL;
        }
    }
}

/**
 * util funtions
 */
static struct v4l2_subdev *
_get_remote_sink_subdev(struct nxp_mlc *me)
{
    struct media_pad *pad =
        media_entity_remote_source(&me->pads[NXP_MLC_PAD_SOURCE]);
    if (!pad) {
        pr_debug("%s: id(%d) can't find remote sink pad!!!\n", __func__, me->id);
        return NULL;
    }
    return media_entity_to_v4l2_subdev(pad->entity);
}

static int _handle_input_connection(struct nxp_mlc *me,
        struct nxp_video *remote, bool connected)
{
    u32 type;
    bool is_video;

    if (remote->type == NXP_VIDEO_TYPE_CAPTURE ||
            remote->type == NXP_VIDEO_TYPE_M2M) {
        /* video device is sink */
        type = NXP_BUFFER_CONSUMER_SINK;
        is_video = true;
    } else {
        /* video device is source */
        /* from MLC video device */
        type = NXP_BUFFER_CONSUMER_SOURCE;
        if (!strncmp(remote->name, "VIDEO MLC RGB", strlen("VIDEO MLC RGB"))) {
            is_video = false;
        } else if (!strncmp(remote->name, "VIDEO MLC VID", strlen("VIDEO MLC VID"))) {
            is_video = true;
        } else {
            pr_err("%s: invalid video device(%s)\n", __func__, remote->name);
            return -EINVAL;
        }
    }

    if (connected) {
        if (is_video)
            return _register_vid_buffer_consumer(me, remote, type);
        else
            return _register_rgb_buffer_consumer(me, remote, type);
    } else {
        if (is_video)
            _unregister_vid_buffer_consumer(me, remote, type);
        else
            _unregister_rgb_buffer_consumer(me, remote, type);
    }
    return 0;
}

static int _handle_output_connection(struct nxp_mlc *me,
        struct v4l2_subdev *sd, bool connected)
{
    pr_debug("%s:%s %s\n", __func__, sd->name,
            connected ? "connected" : "disconnected");
    return 0;
}

static struct v4l2_mbus_framefmt *
_get_pad_format(struct nxp_mlc *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    if (pad >= NXP_MLC_PAD_SOURCE)
        return NULL;

    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_format(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        if (pad == NXP_MLC_PAD_SINK_RGB)
            return &me->rgb_attr.format;
        else
            return &me->vid_attr.format;
    default:
        return NULL;
    }
}

static struct v4l2_rect *
_get_pad_crop(struct nxp_mlc *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    if (pad > NXP_MLC_PAD_SOURCE)
        return NULL;

    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_crop(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        if (pad == NXP_MLC_PAD_SINK_RGB)
            return &me->rgb_attr.crop;
        else if (pad == NXP_MLC_PAD_SINK_VIDEO)
            return &me->vid_attr.crop;
        else
            return &me->vid_attr.source_crop;
    default:
        return NULL;
    }
}

/**
 * v4l2_subdev_internal_ops
 */
static int nxp_mlc_open(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    pr_debug("%s\n", __func__);
    return v4l2_subdev_call(sd, core, s_power, 1);
}

static int nxp_mlc_close(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    pr_debug("%s\n", __func__);
    return v4l2_subdev_call(sd, core, s_power, 0);
}

static int nxp_mlc_registered(struct v4l2_subdev *sd)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static void nxp_mlc_unregistered(struct v4l2_subdev *sd)
{
    pr_debug("%s\n", __func__);
}

static const struct v4l2_subdev_internal_ops nxp_mlc_internal_ops = {
    .open         = nxp_mlc_open,
    .close        = nxp_mlc_close,
    .registered   = nxp_mlc_registered,
    .unregistered = nxp_mlc_unregistered,
};

/**
 * v4l2_subdev_core_ops
 */
static int nxp_mlc_s_power(struct v4l2_subdev *sd, int on)
{
    struct nxp_mlc *me;
    struct v4l2_subdev *remote_sink;

    pr_debug("%s: %d\n", __func__, on);

    me = v4l2_get_subdevdata(sd);
    remote_sink = _get_remote_sink_subdev(me);

    if (on) {
        me->user_count++;
        if (me->user_count == 1 && remote_sink)
            return v4l2_subdev_call(remote_sink, core, s_power, 1);
    } else {
        me->user_count--;
        if (me->user_count == 0) {
             if (remote_sink)
                 v4l2_subdev_call(remote_sink, core, s_power, 0);
             if (me->vid_streaming) {
                 unsigned long flags;
                 me->vid_streaming = false;
                 _hw_video_enable(me, false);
                 pr_debug("%s: unregister irq callback\n", __func__);
                 nxp_soc_disp_unregister_irq_callback(me->id, me->callback);
                 me->callback = NULL;
                 me->vid_enabled = false;
                 spin_lock_irqsave(&me->vlock, flags);
                 if (me->old_vid_buf) {
                     pr_debug("%s: video done buf late for old buf!", __func__);
                     _done_buf(me->old_vid_buf, true);
                     me->old_vid_buf = NULL;
                 }
                 if (me->cur_vid_buf) {
                     pr_debug("%s: video done buf late for cur buf!", __func__);
                     _done_buf(me->cur_vid_buf, true);
                     me->cur_vid_buf = NULL;
                 }
                 spin_unlock_irqrestore(&me->vlock, flags);
                 _clear_all_vid_buf(me);
             }
        }
    }

    return 0;
}

static int nxp_mlc_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);
    int id = me->id;
    __s32 value = ctrl->value;
    bool is_on = false;
    int layer;

    pr_debug("%s\n", __func__);

    switch (ctrl->id) {
    case V4L2_CID_MLC_VID_PRIORITY:
        if (value >= 0 && value <= 3) {
           pr_debug("%s: video priority(%d), streaming(%d)\n", __func__, value, me->vid_streaming);
            me->vid_attr.priority = value;
            //if (me->vid_streaming)
                nxp_soc_disp_video_set_priority(id, value);
        }
        break;

    case V4L2_CID_MLC_VID_COLORKEY:
        if (value >= 0)
            is_on = true;
        else
            is_on = false;

        me->vid_attr.colorkey = value;
        //if (me->vid_streaming)
        pr_debug("%s: id %d, colorkey 0x%x, is_on %d\n", __func__, id, value, is_on);
        nxp_soc_disp_set_bg_color(id, 0x0);
        nxp_soc_disp_rgb_set_fblayer(id, 0);
        nxp_soc_disp_video_set_colorkey(id, value, is_on);
        //nxp_soc_disp_video_set_colorkey(id, value, false);
        break;

    case V4L2_CID_MLC_VID_ALPHA:
        if (value >= 0 && value <= 15)
            is_on = true;
        else
            is_on = false;

        me->vid_attr.alpha = value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_color(id, VIDEO_COLOR_ALPHA, value, is_on);
        break;

    case V4L2_CID_BRIGHTNESS:
        if (value >= -128 && value <= 128)
            is_on = true;
        else
            is_on = false;

        me->vid_attr.brightness = value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_color(id, VIDEO_COLOR_BRIGHT, value, is_on);
        break;

    case V4L2_CID_HUE:
        if (value >= 0 && value <= 360)
            is_on = true;
        else
            is_on = false;

        me->vid_attr.hue = value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_color(id, VIDEO_COLOR_HUE, value, is_on);
        break;

    case V4L2_CID_CONTRAST:
        if (value >= 0 && value <= 8)
            is_on = true;
        else
            is_on = false;

        me->vid_attr.contrast = value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_color(id, VIDEO_COLOR_CONTRAST, value, is_on);
        break;

    case V4L2_CID_SATURATION:
        if (value >= -100 && value <= 100)
            is_on = true;
        else
            is_on = false;

        me->vid_attr.saturation = value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_color(id, VIDEO_COLOR_SATURATION, value, is_on);
        break;

    case V4L2_CID_MLC_VID_HFILTER:
        me->vid_attr.hfilter_enable = !!value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_hfilter(id, !!value);
        break;

    case V4L2_CID_MLC_VID_VFILTER:
        me->vid_attr.vfilter_enable = !!value;
        //if (me->vid_streaming)
            nxp_soc_disp_video_set_vfilter(id, !!value);
        break;

    case V4L2_CID_MLC_RGB_ALPHA:
        if (value >= 0 && value <= 15)
            is_on = true;
        else
            is_on = false;

        me->rgb_attr.alpha = value;
        layer = RGB_0_LAYER;
        //if (me->rgb_streaming)
            nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_ALPHA, value, is_on);
        break;

    case V4L2_CID_MLC_RGB_TRANSCOLOR:
        if (value >= 0)
            is_on = true;
        else
            is_on = false;

        me->rgb_attr.transcolor = value;
        layer = RGB_0_LAYER;
        //if (me->rgb_streaming)
            /* nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_TRANSP, value, is_on); */
            nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_TRANSP, value, false);
        break;

    case V4L2_CID_MLC_RGB_INVERTCOLOR:
        if (value >= 0)
            is_on = true;
        else
            is_on = false;

        me->rgb_attr.invertcolor = value;
        layer = RGB_0_LAYER;
        //if (me->rgb_streaming)
            nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_INVERT, value, is_on);
        break;

    case V4L2_CID_BG_COLOR:
        if (value >= 0)
            is_on = true;
        else
            is_on = false;

        me->rgb_attr.backcolor = value;
        layer = RGB_0_LAYER;
        //if (me->rgb_streaming && is_on)
            nxp_soc_disp_set_bg_color(id, value);
        break;
    }

    return 0;
}

static int nxp_mlc_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);

    pr_debug("%s\n", __func__);

    switch (ctrl->id) {
    case V4L2_CID_MLC_VID_PRIORITY:
        ctrl->value = me->vid_attr.priority;
        break;

    case V4L2_CID_MLC_VID_COLORKEY:
        ctrl->value = me->vid_attr.colorkey;
        break;

    case V4L2_CID_MLC_VID_ALPHA:
        ctrl->value = me->vid_attr.alpha;
        break;

    case V4L2_CID_BRIGHTNESS:
        ctrl->value = me->vid_attr.brightness;
        break;

    case V4L2_CID_HUE:
        ctrl->value = me->vid_attr.hue;
        break;

    case V4L2_CID_CONTRAST:
        ctrl->value = me->vid_attr.contrast;
        break;

    case V4L2_CID_SATURATION:
        ctrl->value = me->vid_attr.saturation;
        break;

    case V4L2_CID_MLC_VID_HFILTER:
        ctrl->value = me->vid_attr.hfilter_enable;
        break;

    case V4L2_CID_MLC_VID_VFILTER:
        ctrl->value = me->vid_attr.vfilter_enable;
        break;

    case V4L2_CID_MLC_RGB_ALPHA:
        ctrl->value = me->rgb_attr.alpha;
        break;

    case V4L2_CID_MLC_RGB_TRANSCOLOR:
        ctrl->value = me->rgb_attr.transcolor;
        break;

    case V4L2_CID_MLC_RGB_INVERTCOLOR:
        ctrl->value = me->rgb_attr.invertcolor;
        break;

    case V4L2_CID_BG_COLOR:
        ctrl->value = me->rgb_attr.backcolor;
        break;
    }

    return 0;
}

static const struct v4l2_subdev_core_ops nxp_mlc_core_ops = {
    .s_power = nxp_mlc_s_power,
    .s_ctrl  = nxp_mlc_s_ctrl,
    .g_ctrl  = nxp_mlc_g_ctrl,
};

/**
 * v4l2_subdev_video_ops
 */
static int nxp_mlc_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct nxp_mlc *me;
    bool is_video = true;
    char *client;
    struct v4l2_subdev *remote_sink;

    me = v4l2_get_subdevdata(sd);
    client = v4l2_get_subdev_hostdata(sd);

    if (!strncmp(client, "VIDEO MLC RGB", strlen("VIDEO MLC RGB")))
        is_video = false;

    /* pr_debug("%s: enable %d, client %s, is_video %d\n", __func__, */
    /*         enable, client, is_video); */
    printk("%s: enable %d, client %s, is_video %d\n", __func__,
            enable, client, is_video);

    if (enable) {
        if (is_video) {
            me->vid_streaming = true;
            _update_vid_buffer(me);
        } else {
            me->rgb_streaming = true;
            _update_rgb_buffer(me);
        }
        remote_sink = _get_remote_sink_subdev(me);
        if (remote_sink)
            v4l2_subdev_call(remote_sink, video, s_stream, 1);
    } else {
        if (is_video) {
            unsigned long flags;
            me->vid_streaming = false;
            _hw_video_enable(me, false);
            pr_debug("%s: unregister irq callback\n", __func__);
            nxp_soc_disp_unregister_irq_callback(me->id, me->callback);
            me->callback = NULL;
            me->vid_enabled = false;
            spin_lock_irqsave(&me->vlock, flags);
            if (me->old_vid_buf) {
                pr_debug("%s: video done buf late for old buf!", __func__);
                _done_buf(me->old_vid_buf, true);
                me->old_vid_buf = NULL;
            }
            if (me->cur_vid_buf) {
                pr_debug("%s: video done buf late for cur buf!", __func__);
                _done_buf(me->cur_vid_buf, true);
                me->cur_vid_buf = NULL;
            }
            spin_unlock_irqrestore(&me->vlock, flags);
            _clear_all_vid_buf(me);
        } else {
            me->rgb_streaming = false;
            _hw_rgb_enable(me, false);
            me->rgb_enabled = false;
            _clear_all_rgb_buf(me);
        }

        if (me->id > 0 && me->rgb_enabled == false && me->vid_enabled == false) {
            _hw_enable(me, false);
            me->enabled = false;

            remote_sink = _get_remote_sink_subdev(me);
            if (remote_sink)
                v4l2_subdev_call(remote_sink, video, s_stream, 0);

        }
    }

    return 0;
}

static const struct v4l2_subdev_video_ops nxp_mlc_video_ops = {
    .s_stream = nxp_mlc_s_stream,
};

/**
 * v4l2_subdev_pad_ops
 */
static int nxp_mlc_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *_fmt =
        _get_pad_format(me, fh, format->pad, format->which);
    if (_fmt) {
        format->format = *_fmt;
        return 0;
    } else
        return -EINVAL;
}

static int nxp_mlc_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *_fmt =
        _get_pad_format(me, fh, format->pad, format->which);

    if (!_fmt) {
        pr_err("%s: can't get pad format\n", __func__);
        return -EINVAL;
    }

    switch (format->pad) {
    case NXP_MLC_PAD_SINK_RGB:
        *_fmt = format->format;
        pr_debug("%s: rgb format code 0x%x\n", __func__, format->format.code);
        switch (format->format.code) {
        case V4L2_MBUS_FMT_RGB565_2X8_LE:
            _fmt->code = NX_MLC_RGBFMT_R5G6B5;
            me->rgb_attr.pixelbyte = 2;
            break;
        case V4L2_MBUS_FMT_XRGB8888_4X8_LE:
            /* android uses A8R8G8B8 */
            // fix for kitkat
            /*_fmt->code = NX_MLC_RGBFMT_A8R8G8B8;*/
            _fmt->code = NX_MLC_RGBFMT_A8B8G8R8;
            me->rgb_attr.pixelbyte = 4;
            break;
        }
        break;

    case NXP_MLC_PAD_SINK_VIDEO:
        if (!_find_vid_format(supported_vid_formats,
                    ARRAY_SIZE(supported_vid_formats), format->format.code)) {
            pr_err("%s: not supported video format(0x%x)\n", __func__,
                    format->format.code);
            return -EINVAL;
        }
        *_fmt = format->format;
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static int nxp_mlc_get_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);
    struct v4l2_rect *_crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    if (_crop) {
        crop->rect = *_crop;
        return 0;
    } else
        return -EINVAL;
}

static int nxp_mlc_set_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);
    struct v4l2_rect *_crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    if (!_crop) {
        pr_err("%s: can't get pad crop\n", __func__);
        return -EINVAL;
    }

    printk("%s: pad(%d), crop(%d,%d,%d,%d)\n", __func__, crop->pad,
            crop->rect.left, crop->rect.top, crop->rect.width, crop->rect.height);
    *_crop = crop->rect;

    /* psw0523 add for runtime video layer config */
    if (crop->pad == NXP_MLC_PAD_SINK_VIDEO && me->vid_enabled) {
        if (_crop->width > 0 && _crop->height > 0)
            nxp_soc_disp_video_set_position(me->id,  /* module */
                    _crop->left, _crop->top, /* left, top */
                    _crop->left + _crop->width, /* right */
                    _crop->top + _crop->height, /* bottom */
                    true); /* waitsync */
    } else if (crop->pad == NXP_MLC_PAD_SOURCE && me->vid_enabled) {
        if (_crop->width > 0 && _crop->height > 0) {
            printk("%s: source crop(%d:%d-%d:%d)\n", __func__, _crop->left, _crop->top, _crop->width, _crop->height);
            nxp_soc_disp_video_set_crop(me->id, true, _crop->left, _crop->top, _crop->width, _crop->height, true);
        } else {
            printk("%s: source crop off\n", __func__);
            nxp_soc_disp_video_set_crop(me->id, false, 0, 0, 0, 0, true);
        }
    }
    return 0;
}

static const struct v4l2_subdev_pad_ops nxp_mlc_pad_ops = {
    .get_fmt        = nxp_mlc_get_fmt,
    .set_fmt        = nxp_mlc_set_fmt,
    .get_crop       = nxp_mlc_get_crop,
    .set_crop       = nxp_mlc_set_crop,
};

/**
 * v4l2_subdev_ops
 */
static const struct v4l2_subdev_ops nxp_mlc_subdev_ops = {
    .core  = &nxp_mlc_core_ops,
    .video = &nxp_mlc_video_ops,
    .pad   = &nxp_mlc_pad_ops,
};

/**
 * media entity ops
 */
static int nxp_mlc_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    int ret = 0;
    struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    struct nxp_mlc *me = v4l2_get_subdevdata(sd);
    struct video_device *remote_vdev;
    struct v4l2_subdev *remote_sd;
    struct nxp_video *remote_nxp_video;

    pr_debug("%s\n", __func__);

    switch (local->index | media_entity_type(remote->entity)) {
        case NXP_MLC_PAD_SINK_RGB | MEDIA_ENT_T_DEVNODE:
        case NXP_MLC_PAD_SINK_VIDEO | MEDIA_ENT_T_DEVNODE:
            /* link video device to my rgb sink */
            remote_vdev = media_entity_to_video_device(remote->entity);
            remote_nxp_video = video_get_drvdata(remote_vdev);
            ret = _handle_input_connection(me, remote_nxp_video,
                    (flags & MEDIA_LNK_FL_ENABLED) ? true : false);
            break;

        case NXP_MLC_PAD_SOURCE | MEDIA_ENT_T_V4L2_SUBDEV:
            /* link me to subdev (resolution converter, hdmi) */
            remote_sd = media_entity_to_v4l2_subdev(remote->entity);
            ret = _handle_output_connection(me, remote_sd,
                    (flags & MEDIA_LNK_FL_ENABLED) ? true : false);
            break;

        default:
            pr_err("%s: invalid link\n", __func__);
            return -EINVAL;
    }

    return 0;
}

static const struct media_entity_operations nxp_mlc_media_ops = {
    .link_setup = nxp_mlc_link_setup,
};

/*
 * _init_entities
 */
static int _init_entities(struct nxp_mlc *me)
{
    int ret;
    struct v4l2_subdev *sd = &me->subdev;
    struct media_pad *pads = me->pads;
    struct media_entity *entity = &sd->entity;
    char dev_name[NXP_VIDEO_MAX_NAME_SIZE] = {0, };

    me->vid_input_type = NXP_MLC_INPUT_TYPE_NONE;

    v4l2_subdev_init(sd, &nxp_mlc_subdev_ops);
    sd->internal_ops = &nxp_mlc_internal_ops;
    /* entity name : used app media link setup */
    //strlcpy(sd->name, "NXP MLC", sizeof(sd->name));
    snprintf(sd->name, sizeof(sd->name), "NXP MLC%d", me->id);
    /* TODO */
    /* sd->grp_id = NXP_OUT_SUBDEV_GROUP_ID */
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pads[NXP_MLC_PAD_SINK_RGB].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_MLC_PAD_SINK_VIDEO].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_MLC_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

    entity->ops = &nxp_mlc_media_ops;
    ret = media_entity_init(entity, NXP_MLC_PAD_MAX, pads, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

    snprintf(dev_name, sizeof(dev_name), "VIDEO MLC RGB%d", me->id);
    me->rgb_video = create_nxp_video(dev_name, NXP_VIDEO_TYPE_OUT,
            nxp_v4l2_get_v4l2_device(), nxp_v4l2_get_alloc_ctx());
    if (!me->rgb_video) {
        pr_err("%s: failed to create_nxp_video() for rgb\n", __func__);
        ret = -EINVAL;
        goto error_video_rgb;
    }

    snprintf(dev_name, sizeof(dev_name), "VIDEO MLC VID%d", me->id);
    me->vid_video = create_nxp_video(dev_name, NXP_VIDEO_TYPE_OUT,
            nxp_v4l2_get_v4l2_device(), nxp_v4l2_get_alloc_ctx());
    if (!me->vid_video) {
        pr_err("%s: failed to create_nxp_video() for video\n", __func__);
        ret = -EINVAL;
        goto error_video_vid;
    }

    /* create link rgb video to subdev rgb sink pad */
    ret = media_entity_create_link(&me->rgb_video->vdev.entity, 1,
            entity, NXP_MLC_PAD_SINK_RGB, 0);
    if (ret < 0) {
        pr_err("%s: failed to link rgb video to my sink rgb\n", __func__);
        goto error_link;
    }

    /* create link vid video to subdev vid sink pad */
    ret = media_entity_create_link(&me->vid_video->vdev.entity, 1,
            entity, NXP_MLC_PAD_SINK_VIDEO, 0);
    if (ret < 0) {
        pr_err("%s: failed to link vid video to my sink video\n", __func__);
        goto error_link;
    }

    /* link from me to others are set by parent */

    return 0;

error_link:
    release_nxp_video(me->vid_video);
    me->vid_video = NULL;
error_video_vid:
    release_nxp_video(me->rgb_video);
    me->rgb_video = NULL;
error_video_rgb:
    media_entity_cleanup(entity);
    return ret;
}

static void _init_context(struct nxp_mlc *me)
{
    struct nxp_mlc_rgb_attr *rgb = &me->rgb_attr;
    struct nxp_mlc_vid_attr *vid = &me->vid_attr;

    me->enabled = _hw_get_enabled(me->id);

    if (me->enabled) {
        me->rgb_enabled = _hw_get_rgb_attr(me->id, rgb);
        me->vid_enabled = _hw_get_vid_attr(me->id, vid);
    } else {
        me->rgb_enabled = false;
        me->vid_enabled = false;
    }

    pr_debug("%s: enabled(%d), rgb_enabled(%d), vid_enabled(%d)\n",
            __func__, me->enabled, me->rgb_enabled, me->vid_enabled);
}

/**
 * public api
 */
struct nxp_mlc *create_nxp_mlc(int id)
{
    int ret;
    struct nxp_mlc *me;

    pr_debug("%s\n", __func__);

    me = kzalloc(sizeof(struct nxp_mlc), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to alloc me!!!\n", __func__);
        return NULL;
    }

    me->id = id;
    ret = _init_entities(me);

    if (likely(!ret)) {
        _init_context(me);
        spin_lock_init(&me->rlock);
        spin_lock_init(&me->vlock);
        INIT_LIST_HEAD(&me->rgb_buffer_list);
        INIT_LIST_HEAD(&me->vid_buffer_list);
        return me;
    }

    return NULL;
}

void release_nxp_mlc(struct nxp_mlc *me)
{
    pr_debug("%s\n", __func__);
    release_nxp_video(me->rgb_video);
    release_nxp_video(me->vid_video);
    kfree(me);
}

int register_nxp_mlc(struct nxp_mlc *me)
{
    int ret;

    pr_debug("%s: %d\n", __func__, me->id);

    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(),
            &me->subdev);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    ret = register_nxp_video(me->rgb_video);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_video() for rgb\n", __func__);
        goto error_reg_rgb_video;
    }

    ret = register_nxp_video(me->vid_video);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_video() for video\n", __func__);
        goto error_reg_vid_video;
    }

    return 0;

error_reg_vid_video:
    unregister_nxp_video(me->rgb_video);
error_reg_rgb_video:
    v4l2_device_unregister_subdev(&me->subdev);
    return ret;
}

void unregister_nxp_mlc(struct nxp_mlc *me)
{
    pr_debug("%s: %d\n", __func__, me->id);
    unregister_nxp_video(me->rgb_video);
    unregister_nxp_video(me->vid_video);
    v4l2_device_unregister_subdev(&me->subdev);
}

#ifdef CONFIG_PM
int suspend_nxp_mlc(struct nxp_mlc *me)
{
    PM_DBGOUT("%s\n", __func__);

    if (me->enabled) {
        if (me->vid_enabled) {
            nxp_soc_disp_video_get_address(me->id,
                    &me->vid_attr.lu_a, &me->vid_attr.lu_s,
                    &me->vid_attr.cb_a, &me->vid_attr.cb_s,
                    &me->vid_attr.cr_a, &me->vid_attr.cr_s);
        }

        if (me->rgb_enabled) {
            nxp_soc_disp_rgb_get_address(me->id, RGB_0_LAYER,
                    &me->rgb_attr.addr, &me->rgb_attr.pixelbyte, &me->rgb_attr.stride);
        }
    }
    return 0;
}

int resume_nxp_mlc(struct nxp_mlc *me)
{
    int id = me->id;

    PM_DBGOUT("+%s: %d\n", __func__, id);

    /* if (me->enabled) { */
    /*     if (me->vid_enabled) { */
    /*         PM_DBGOUT("%s: resume mlc %d video\n", __func__, id); */
    /*         nxp_soc_disp_video_set_priority(id, me->vid_attr.priority); */
    /*         nxp_soc_disp_set_bg_color(id, 0x0); */
    /*         nxp_soc_disp_rgb_set_fblayer(id, 0); */
    /*         nxp_soc_disp_video_set_colorkey(id, me->vid_attr.colorkey, false); */
    /*         nxp_soc_disp_video_set_color(id, VIDEO_COLOR_ALPHA, me->vid_attr.alpha, */
    /*                 me->vid_attr.alpha >= 0 && me->vid_attr.alpha <= 15); */
    /*         nxp_soc_disp_video_set_color(id, VIDEO_COLOR_BRIGHT, me->vid_attr.brightness, */
    /*                 me->vid_attr.brightness >= -128 && me->vid_attr.brightness <= 128); */
    /*         nxp_soc_disp_video_set_color(id, VIDEO_COLOR_HUE, me->vid_attr.hue, */
    /*                 me->vid_attr.hue >= 0 && me->vid_attr.hue <= 360); */
    /*         nxp_soc_disp_video_set_color(id, VIDEO_COLOR_CONTRAST, me->vid_attr.contrast, */
    /*                 me->vid_attr.contrast >= 0 && me->vid_attr.contrast <= 8); */
    /*         nxp_soc_disp_video_set_color(id, VIDEO_COLOR_SATURATION, me->vid_attr.saturation, */
    /*                 me->vid_attr.saturation >= -100 && me->vid_attr.saturation <= 100); */
    /*         nxp_soc_disp_video_set_hfilter(id, me->vid_attr.hfilter_enable); */
    /*         nxp_soc_disp_video_set_vfilter(id, me->vid_attr.vfilter_enable); */

    /*         _hw_configure_video(me); */

    /*         nxp_soc_disp_video_set_address(id, #<{(| module |)}># */
    /*                 me->vid_attr.lu_a, me->vid_attr.lu_s, #<{(| lu_a, lu_s |)}># */
    /*                 me->vid_attr.cb_a, me->vid_attr.cb_s, #<{(| cb_a, cb_s |)}># */
    /*                 me->vid_attr.cr_a, me->vid_attr.cr_s, #<{(| cr_a, cr_s |)}># */
    /*                 true); */
    /*     } */

    /*     if (me->rgb_enabled) { */
    /*         int layer = RGB_0_LAYER; */
    /*         PM_DBGOUT("%s: resume mlc %d rgb\n", __func__, id); */
    /*         nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_ALPHA, me->rgb_attr.alpha, */
    /*                 me->rgb_attr.alpha >= 0 && me->rgb_attr.alpha <= 15); */
    /*         nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_TRANSP, me->rgb_attr.transcolor, false); */
    /*         nxp_soc_disp_rgb_set_color(id, layer, RGB_COLOR_INVERT, me->rgb_attr.invertcolor, me->rgb_attr.invertcolor >= 0); */
    /*         nxp_soc_disp_set_bg_color(id, me->rgb_attr.backcolor); */

    /*         PM_DBGOUT("call _hw_configure_rgb()\n"); */
    /*         _hw_configure_rgb(me); */

    /*         PM_DBGOUT("call nxp_soc_disp_rgb_set_address(): addr 0x%x, pixelbyte 0x%x, stride %d\n", */
    /*                 me->rgb_attr.addr, me->rgb_attr.pixelbyte, me->rgb_attr.stride); */
    /*         nxp_soc_disp_rgb_set_address(id, layer, #<{(| module, layer |)}># */
    /*                 me->rgb_attr.addr,  #<{(| addr |)}># */
    /*                 me->rgb_attr.pixelbyte, me->rgb_attr.stride, */
    /*                 true); */
    /*     } */

    /*     PM_DBGOUT("call _hw_enable()\n"); */
    /*     _hw_enable(me, true); */
    /* } */

    if (me->enabled)
        nxp_soc_disp_device_resume(id);

    PM_DBGOUT("-%s: %d\n", __func__, id);
    return 0;
}
#endif
