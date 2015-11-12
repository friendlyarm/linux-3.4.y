/*#define DEBUG 0*/

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/v4l2-mediabus.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#include <mach/nxp-v4l2-platformdata.h>
#include <mach/platform.h>

#include "nxp-v4l2-common.h"
#include "nxp-video.h"
#include "nxp-v4l2.h"

#include "nxp-capture.h"
#include "nxp-decimator.h"

/* for nexell specific prototype */
#ifdef CONFIG_ARCH_NXP4330_3200
#include <mach/nxp3200.h>
#include <linux/workqueue.h>
#else
#include <mach/platform.h>
#endif

enum nxp_decimator_state {
    NXP_DECIMATOR_STATE_UNINIT  = 0,
    NXP_DECIMATOR_STATE_INIT    = (1 << 0),
    NXP_DECIMATOR_STATE_STOPPED = (1 << 1),
    NXP_DECIMATOR_STATE_RUNNING = (1 << 2),
    NXP_DECIMATOR_STATE_STOPPING = (1 << 3)
};


#define subdev_to_nxp_decimator(subdev)   \
    container_of(subdev, struct nxp_decimator, subdev)
#define nxp_decimator_to_parent(me)       \
    container_of(me, struct nxp_capture, decimator)

/**
 * hw routines
 */
static inline NX_VIP_FORMAT _convert_to_vip_format(unsigned int format)
{
    switch (format) {
    case V4L2_MBUS_FMT_YUYV8_2X8:
        return NX_VIP_FORMAT_YUYV;
    case V4L2_MBUS_FMT_YUYV8_1_5X8:
        return NX_VIP_FORMAT_420;
    case V4L2_MBUS_FMT_YUYV8_1X16:
        return NX_VIP_FORMAT_422;
    case V4L2_MBUS_FMT_YUV8_1X24:
        return NX_VIP_FORMAT_444;
    default:
        pr_err("%s: invalid format 0x%x, return YUV420\n", __func__, format);
        return NX_VIP_FORMAT_420;
    }
}

#define YUV_STRIDE_ALIGN_FACTOR     128
#define YUV_VSTRIDE_ALIGN_FACTOR    16
#define YUV_YSTRIDE(w)   ALIGN(w, YUV_STRIDE_ALIGN_FACTOR)

static struct v4l2_subdev *_get_remote_source_subdev(struct nxp_decimator *me);
static int _hw_configure(struct nxp_decimator *me)
{
    struct nxp_capture *parent = nxp_decimator_to_parent(me);
    int module = parent->get_module_num(parent);

    vmsg("decimator %s: source(%dx%d), target(%dx%d)\n", __func__,
            me->src_width, me->src_height,
            me->target_width, me->target_height);

#if 0
    NX_VIP_SetClipRegion(module, 0, 0, me->src_width, me->src_height);

    NX_VIP_SetDecimation(module,
            me->src_width, me->src_height,
            me->target_width, me->target_height);

    NX_VIP_SetDecimatorFormat(module,
            _convert_to_vip_format(me->code) , false, false, false);
#else
    // check clipper on & clipper region
    {
    #if defined(CONFIG_ARCH_S5P4418)
        int left, top, right, bottom;
        NX_VIP_GetClipRegion(module, &left, &top, &right, &bottom);
        if ((me->src_width != (right - left)) || (me->src_height != (bottom - top))) {
            pr_err("%s: current clipper setting is differ from me\n", __func__);
            pr_err("clipper(%d,%d--%d,%d)\n", left, top, right, bottom);
            pr_err("me(%d,%d--%d,%d)\n", 0, 0, me->src_width, me->src_height);
            pr_err("setting force me to clipper: %d,%d --> %d,%d\n", right, bottom, YUV_YSTRIDE(right-127), bottom);

            NX_VIP_SetClipRegion(module, 0, 0, right, bottom);
            NX_VIP_SetDecimation(module,
                    right, bottom,
                    YUV_YSTRIDE(right - 127), bottom);

        } else {
            NX_VIP_SetClipRegion(module, 0, 0, me->src_width, me->src_height);
            NX_VIP_SetDecimation(module,
                    me->src_width, me->src_height,
                    me->target_width, me->target_height);
        }
    #else
        NX_VIP_SetClipRegion(module, 0, 0, me->src_width, me->src_height);
        NX_VIP_SetDecimation(module,
                me->src_width, me->src_height,
                me->target_width, me->target_height);
    #endif

    #if defined(CONFIG_ARCH_S5P4418)
        NX_VIP_SetDecimatorFormat(module,
                _convert_to_vip_format(me->code) , false, false, false);
    #elif defined(CONFIG_ARCH_S5P6818)
        NX_VIP_SetDecimatorFormat(module, _convert_to_vip_format(me->code));
    #endif
    }
#endif

    return 0;
}

// psw0523 for debugging
static unsigned int _lu_addr = 0;
static unsigned int _cb_addr = 0;
static unsigned int _cr_addr = 0;

static int _hw_set_addr(struct nxp_decimator *me, struct nxp_video_buffer *buf)
{
    struct nxp_capture *parent = nxp_decimator_to_parent(me);
    int module = parent->get_module_num(parent);

    if (me->code == V4L2_MBUS_FMT_YUYV8_2X8) {
        // YUYV422
         vmsg("%s decimator: addr 0x%x\n", __func__, buf->dma_addr[0]);
        NX_VIP_SetDecimatorAddr(module,
                NX_VIP_FORMAT_YUYV,
                me->target_width,
                me->target_height,
                buf->dma_addr[0],
                0,
#ifdef CONFIG_ARCH_NXP4330_3200
                0);
#else
                0,
                buf->stride[0], 0);
#endif
    } else {
         vmsg("%s decimator: addr LU(0x%x) CB(0x%x) CR(0x%x), stride LU(%d) CB(%d) CR(%d)\n",
                 __func__, buf->dma_addr[0], buf->dma_addr[1], buf->dma_addr[2],
                 buf->stride[0], buf->stride[1], buf->stride[2]);
        _lu_addr = buf->dma_addr[0];
        _cb_addr = buf->dma_addr[1];
        _cr_addr = buf->dma_addr[2];
        NX_VIP_SetDecimatorAddr(module,
                _convert_to_vip_format(me->code),
                me->target_width,
                me->target_height,
                buf->dma_addr[0],
                buf->dma_addr[1],
#ifdef CONFIG_ARCH_NXP4330_3200
                buf->dma_addr[2]);
#else
                buf->dma_addr[2],
                buf->stride[0],
                buf->stride[1]);
#endif
    }

    return 0;
}

/**
 * utils used in subdev ops
 */
#if 0
static struct v4l2_mbus_framefmt *
_get_pad_format(struct nxp_decimator *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    if (pad >= NXP_DECIMATOR_PAD_MAX)
        pad -= 1;

    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_format(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        return &me->format[pad];
    default:
        return NULL;
    }
}

static struct v4l2_rect *
_get_pad_crop(struct nxp_decimator *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_crop(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        return &me->crop;
    default:
        return NULL;
    }
}
#endif

static struct v4l2_subdev *
_get_remote_source_subdev(struct nxp_decimator *me)
{
    struct media_pad *pad =
        media_entity_remote_source(&me->pads[NXP_DECIMATOR_PAD_SINK]);
    if (!pad)
        return NULL;
    return media_entity_to_v4l2_subdev(pad->entity);
}

/**
 * buffer ops
 */
static int decimator_buffer_queue(struct nxp_video_buffer *buf, void *me)
{
    unsigned long flags;
    struct nxp_decimator *_me = me;

    vmsg("%s: %p\n", __func__, buf);
    spin_lock_irqsave(&_me->slock, flags);
    list_add_tail(&buf->list, &_me->buffer_list);
    spin_unlock_irqrestore(&_me->slock, flags);
    return 0;
}

static int _register_buffer_consumer(struct nxp_decimator *me)
{
    if (!me->consumer) {
        me->consumer = kzalloc(sizeof(struct nxp_buffer_consumer), GFP_KERNEL);
        if (!me->consumer) {
            pr_err("%s: failed to alloc consumer\n", __func__);
            return -ENOMEM;
        }
    }

    me->consumer->priv = me;
    me->consumer->queue = decimator_buffer_queue;
    return me->video->register_buffer_consumer(me->video,
            me->consumer, NXP_BUFFER_CONSUMER_SINK);
}

static void _unregister_buffer_consumer(struct nxp_decimator *me)
{
    if (me->consumer) {
        me->video->unregister_buffer_consumer(me->video,
                me->consumer, NXP_BUFFER_CONSUMER_SINK);
        kfree(me->consumer);
        me->consumer = NULL;
    }
}

static int _update_next_buffer(struct nxp_decimator *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf;

    spin_lock_irqsave(&me->slock, flags);
    if (list_empty(&me->buffer_list)) {
        spin_unlock_irqrestore(&me->slock, flags);
        return -ENOENT;
    }

    buf = list_first_entry(&me->buffer_list, struct nxp_video_buffer, list);
    if (buf != NULL) {
        list_del_init(&buf->list);
        me->cur_buf = buf;
        spin_unlock_irqrestore(&me->slock, flags);

        vmsg("%s decimator: %p\n", __func__, buf);
#ifndef CONFIG_ARCH_NXP4330_3200
        _hw_set_addr(me, buf);
#endif
    }
    return 0;
}

static int _done_buf(struct nxp_decimator *me, bool updated)
{
    unsigned long flags;
    struct nxp_video_buffer *buf;

    spin_lock_irqsave(&me->slock, flags);
    buf = me->cur_buf;
    me->cur_buf = NULL;
    spin_unlock_irqrestore(&me->slock, flags);

    vmsg("%s: %p\n", __func__, buf);

    if (!buf)
        return 0;

    if (updated)
        buf->consumer_index++;

    if (buf->cb_buf_done)
        buf->cb_buf_done(buf);

    return 0;
}

static void _clear_buf(struct nxp_decimator *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf;

    spin_lock_irqsave(&me->slock, flags);
    while (!list_empty(&me->buffer_list)) {
        buf = list_entry(me->buffer_list.next, struct nxp_video_buffer, list);
        if (buf) {
            /* buf->cb_buf_done(buf); */
            list_del_init(&buf->list);
        } else {
            break;
        }
    }
    INIT_LIST_HEAD(&me->buffer_list);
    me->cur_buf = NULL;
    spin_unlock_irqrestore(&me->slock, flags);
}

/**
 * irq
 */
static void _unregister_irq_handler(struct nxp_decimator *me);

static irqreturn_t decimator_irq_handler(void *data)
{
    struct nxp_decimator *me = data;

    _done_buf(me, true);
    if (NXP_ATOMIC_READ(&me->state) & NXP_DECIMATOR_STATE_STOPPING) {
        struct nxp_capture *parent = nxp_decimator_to_parent(me);
        vmsg("%s: real stop...\n", __func__);
        parent->stop(parent, me);
        _unregister_irq_handler(me);
        _clear_buf(me);
        complete(&me->stop_done);
    } else {
        _update_next_buffer(me);
    }

    return IRQ_HANDLED;
}

static int _register_irq_handler(struct nxp_decimator *me)
{
    struct nxp_capture *parent = nxp_decimator_to_parent(me);
    if (!me->irq_entry) {
        me->irq_entry = kzalloc(sizeof(struct nxp_v4l2_irq_entry), GFP_KERNEL);
        if (!me->irq_entry) {
            pr_err("%s: failed to allocate irq_entry\n", __func__);
            return -ENOMEM;
        }
    }
    me->irq_entry->irqs = CAPTURE_DECIMATOR_INT;
    me->irq_entry->priv = me;
    me->irq_entry->handler = decimator_irq_handler;
    return parent->register_irq_entry(parent, me->irq_entry);
}

static void _unregister_irq_handler(struct nxp_decimator *me)
{
    if (me->irq_entry) {
        struct nxp_capture *parent = nxp_decimator_to_parent(me);
        parent->unregister_irq_entry(parent, me->irq_entry);
        kfree(me->irq_entry);
        me->irq_entry = NULL;
    }
}

static void _disable_all(struct nxp_decimator *me)
{
    if (NXP_ATOMIC_READ(&me->state) == NXP_DECIMATOR_STATE_RUNNING) {
        vmsg("%s: decimator video stopping...\n", __func__);
        NXP_ATOMIC_SET_MASK(NXP_DECIMATOR_STATE_STOPPING, &me->state);
        if (!wait_for_completion_timeout(&me->stop_done, 5*HZ)) {
            struct nxp_capture *parent = nxp_decimator_to_parent(me);
            printk("timeout wait for decimator stopping\n");
            parent->stop(parent, me);
            _unregister_irq_handler(me);
            _clear_buf(me);
        }
        NXP_ATOMIC_SET(&me->state, NXP_DECIMATOR_STATE_STOPPED);
    }
}

/**
 * handle connections
 */
static int _handle_video_connection(struct nxp_decimator *me, bool connected)
{
    if (connected) {
        int ret = _register_buffer_consumer(me);
        if (ret < 0) {
            pr_err("%s: failed to _register_buffer_consumer\n", __func__);
            return ret;
        }
    } else {
        _unregister_buffer_consumer(me);
    }

    return 0;
}

/**
 * v4l2_subdev_internal_ops
 */
static int nxp_decimator_open(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    vmsg("%s\n", __func__);
    return 0;
}

static int nxp_decimator_close(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    vmsg("%s\n", __func__);
    return 0;
}

static int nxp_decimator_registered(struct v4l2_subdev *sd)
{
    vmsg("%s\n", __func__);
    return 0;
}

static void nxp_decimator_unregistered(struct v4l2_subdev *sd)
{
    vmsg("%s\n", __func__);
}

static const struct v4l2_subdev_internal_ops nxp_decimator_internal_ops = {
    .open         = nxp_decimator_open,
    .close        = nxp_decimator_close,
    .registered   = nxp_decimator_registered,
    .unregistered = nxp_decimator_unregistered,
};

/**
 * v4l2_subdev_core_ops
 */
static int nxp_decimator_s_power(struct v4l2_subdev *sd, int on)
{
    struct nxp_decimator *me;
    struct v4l2_subdev *remote_source;

    vmsg("%s: %d\n", __func__, on);

    me = v4l2_get_subdevdata(sd);
    if (!me) {
        pr_err("%s: me is NULL\n", __func__);
        return -EINVAL;
    }

    remote_source = _get_remote_source_subdev(me);

    if (on) {
        if (me->ref_count == 0) {
            if (remote_source)
                v4l2_subdev_call(remote_source, core, s_power, 1);
            me->ref_count++;
        }
    } else {
        me->ref_count--;
        if (me->ref_count == 0) {
            _disable_all(me);
            if (remote_source)
                v4l2_subdev_call(remote_source, core, s_power, 0);
        }
    }

    return 0;
}

static const struct v4l2_subdev_core_ops nxp_decimator_core_ops = {
    .s_power = nxp_decimator_s_power,
};

/* simulation */
#ifdef CONFIG_ARCH_NXP4330_3200
#include <media/videobuf2-core.h>
static struct nxp_decimator *__me = NULL;
static struct nxp_video_buffer *_src_buf = NULL;

static void simulation_isr(struct work_struct *work)
{
    struct nxp_decimator *me = __me;
    struct vb2_buffer *src_vb, *dst_vb;
    u8 *src_virt, *dst_virt;

    src_vb = _src_buf->priv;
    dst_vb = me->cur_buf->priv;

    src_virt = vb2_plane_vaddr(src_vb, 0);
    dst_virt = vb2_plane_vaddr(dst_vb, 0);

    if (!src_virt) {
        pr_err("%s: failed to get virtual address of src_vb %p\n", __func__, src_vb);
        return;
    }

    if (!dst_virt) {
        pr_err("%s: failed to get virtual address of dst_vb %p\n", __func__, dst_vb);
        return;
    }

    memcpy(dst_virt, src_virt, me->src_width * me->src_height * 2);
    _done_buf(me, true);
    vmsg("%s: end _done_buf\n", __func__);
    _update_next_buffer(me);
}

void nxp_decimator_simulation_run(struct nxp_video_buffer *src_buf)
{
    struct nxp_decimator *me = __me;

    if (!me || !me->cur_buf)
        return;

    _src_buf = src_buf;
    schedule_work(&me->work);
}
#endif

/**
 * v4l2_subdev_video_ops
 */
static int nxp_decimator_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct nxp_decimator *me = v4l2_get_subdevdata(sd);
    struct nxp_capture *parent = nxp_decimator_to_parent(me);
    int ret = 0;
    void *hostdata_back;

    vmsg("%s: %d\n", __func__, enable);

#ifdef CONFIG_ARCH_NXP4330_3200
    if (enable) {
        _update_next_buffer(me);
        __me = me;
    } else {
        cancel_work_sync(&me->work);
        _clear_buf(me);
        __me = NULL;
    }
#else
    if (enable) {
        vmsg("%s: my state 0x%x\n", __func__, NXP_ATOMIC_READ(&me->state));
        if (NXP_ATOMIC_READ(&me->state) != NXP_DECIMATOR_STATE_RUNNING) {
            struct v4l2_subdev *remote = _get_remote_source_subdev(me);
            if (!remote) {
                pr_err("%s: can't find remote source!!!\n", __func__);
                return -EINVAL;
            }

            hostdata_back = v4l2_get_subdev_hostdata(remote);
            v4l2_set_subdev_hostdata(remote, me->name);
            v4l2_subdev_call(remote, video, s_stream, 1);
            v4l2_set_subdev_hostdata(remote, hostdata_back);

            vmsg("%s: lu %p, cb %p, cr %p\n", __func__, &_lu_addr, &_cb_addr, &_cr_addr);

            _hw_configure(me);
            ret = _register_irq_handler(me);
            if (ret < 0) {
                pr_err("%s: failed to _register_irq_handler\n", __func__);
                return ret;
            }
            _update_next_buffer(me);

            if (me->platdata->start_delay_ms)
                msleep(me->platdata->start_delay_ms);

            parent->run(parent, me);
            NXP_ATOMIC_SET(&me->state, NXP_DECIMATOR_STATE_RUNNING);
        } else {
            printk("%s: decimator already running!!!\n", __func__);
            WARN_ON(1);
        }
    } else {
        if (NXP_ATOMIC_READ(&me->state) == NXP_DECIMATOR_STATE_RUNNING) {
            struct v4l2_subdev *remote = _get_remote_source_subdev(me);
            if (!remote) {
                pr_err("%s: can't find remote source!!!\n", __func__);
                return -EINVAL;
            }

            vmsg("decimator video stopping...\n");
            NXP_ATOMIC_SET_MASK(NXP_DECIMATOR_STATE_STOPPING, &me->state);
            if (!wait_for_completion_timeout(&me->stop_done, 5*HZ)) {
                printk("timeout wait for decimator stopping\n");
                parent->stop(parent, me);
                _unregister_irq_handler(me);
                _clear_buf(me);
            }
            if (me->platdata->stop_delay_ms)
                msleep(me->platdata->stop_delay_ms);
            hostdata_back = v4l2_get_subdev_hostdata(remote);
            v4l2_set_subdev_hostdata(remote, me->name);
            v4l2_subdev_call(remote, video, s_stream, 0);
            v4l2_set_subdev_hostdata(remote, hostdata_back);
            NXP_ATOMIC_SET(&me->state, NXP_DECIMATOR_STATE_STOPPED);
        } else {
            printk("%s: decimator not running!!!\n", __func__);
            /* WARN_ON(1); */
        }
    }
#endif

    return 0;
}

static const struct v4l2_subdev_video_ops nxp_decimator_video_ops = {
    .s_stream = nxp_decimator_s_stream,
};

/**
 * v4l2_subdev_pad_ops
 */
static int nxp_decimator_enum_mbus_code(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_mbus_code_enum *code)
{
    vmsg("%s\n", __func__);
    return 0;
}

static int nxp_decimator_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_decimator *me = v4l2_get_subdevdata(sd);
#if 0
    format->format = *_get_pad_format(me, fh, format->pad, format->which);
#else
    format->format.width = me->src_width;
    format->format.height = me->src_height;
    format->format.code = me->code;
#endif
    return 0;
}

static int nxp_decimator_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_decimator *me = v4l2_get_subdevdata(sd);

    vmsg("%s: sw(%d)xsh(%d), code(0x%x)\n", __func__, format->format.width, format->format.height, format->format.code);
    me->src_width = format->format.width;
    me->src_height = format->format.height;
    me->code = format->format.code;

    {
        struct v4l2_subdev *remote_source;
        int ret;

        remote_source = _get_remote_source_subdev(me);
        if (!remote_source) {
            pr_err("%s: can't get remote source subdev\n", __func__);
            return 0;
        }
        /* call to subdev */
        format->pad = 2;
        ret = v4l2_subdev_call(remote_source, pad, set_fmt, NULL, format);
        if (ret < 0) {
            pr_err("%s: failed to subdev set_fmt()\n", __func__);
            return ret;
        }
    }

    return 0;
}

static int nxp_decimator_set_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_decimator *me = v4l2_get_subdevdata(sd);
    if (crop->rect.left != 0 || crop->rect.top != 0) {
        pr_err("%s: crop left, top must be 0\n", __func__);
        return -EINVAL;
    }
    if (crop->rect.width > me->src_width ||
        crop->rect.top  > me->src_height) {
        pr_err("%s: invalid crop rect(l %d, w %d, fw %d, t %d, h %d, fh %d\n)", __func__,
                crop->rect.left, crop->rect.width, me->src_width,
                crop->rect.top, crop->rect.height, me->src_height);
        return -EINVAL;
    }

    if (crop->pad == 1) {
        vmsg("%s: setting decimator scale down(target crop)\n", __func__);
        vmsg("[%d:%d-%d:%d]\n", crop->rect.left, crop->rect.top, crop->rect.width, crop->rect.height);
        me->target_width = crop->rect.width;
        me->target_height = crop->rect.height;
    } else if (crop->pad == 2) {
        struct v4l2_subdev *remote_source;
        int ret;

        vmsg("%s: setting decimator source crop\n", __func__);
        vmsg("[%d:%d-%d:%d]\n", crop->rect.left, crop->rect.top, crop->rect.width, crop->rect.height);

        remote_source = _get_remote_source_subdev(me);
        if (!remote_source) {
            pr_err("%s: can't get remote source subdev\n", __func__);
            return 0;
        }
        /* call to subdev */
        /*crop->pad = 2;*/
        ret = v4l2_subdev_call(remote_source, pad, set_crop, NULL, crop);
        if (ret < 0) {
            pr_err("%s: failed to subdev set_fmt()\n", __func__);
            return ret;
        }
        me->src_width = crop->rect.width;
        me->src_height = crop->rect.height;
    }

    return 0;
}

static int nxp_decimator_get_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_decimator *me = v4l2_get_subdevdata(sd);
#if 0
    struct v4l2_rect *__crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    crop->rect = *__crop;
#else
    crop->rect.left = 0;
    crop->rect.top = 0;
    crop->rect.width = me->target_width;
    crop->rect.height = me->target_height;
#endif
    return 0;
}

static const struct v4l2_subdev_pad_ops nxp_decimator_pad_ops = {
    .enum_mbus_code = nxp_decimator_enum_mbus_code,
    .get_fmt        = nxp_decimator_get_fmt,
    .set_fmt        = nxp_decimator_set_fmt,
    .set_crop       = nxp_decimator_set_crop,
    .get_crop       = nxp_decimator_get_crop,
};

/**
 * v4l2_subdev_ops
 */
static const struct v4l2_subdev_ops nxp_decimator_subdev_ops = {
    .core  = &nxp_decimator_core_ops,
    .video = &nxp_decimator_video_ops,
    .pad   = &nxp_decimator_pad_ops,
};

/**
 * media_entity_operations
 */
static int nxp_decimator_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    struct nxp_decimator *me = v4l2_get_subdevdata(sd);

    vmsg("%s: local index %d, remote type 0x%x\n", __func__,
            local->index, media_entity_type(remote->entity));

    switch (local->index | media_entity_type(remote->entity)) {
    case NXP_DECIMATOR_PAD_SINK | MEDIA_ENT_T_V4L2_SUBDEV:
        /* link clipper to me */
        if (!(flags & MEDIA_LNK_FL_ENABLED)) {
            /* disconnect */
            vmsg("%s: clipper disconnected!!!\n", __func__);
        } else {
            /* connect */
            vmsg("%s: clipper connected!!!\n", __func__);
        }
        break;

    case NXP_DECIMATOR_PAD_SOURCE_MEM | MEDIA_ENT_T_DEVNODE:
        /* link decimator video device to me */
        if (!(flags & MEDIA_LNK_FL_ENABLED)) {
            /* disconnect */
            vmsg("%s: video device disconnected!!!\n", __func__);
            _handle_video_connection(me, false);
        } else {
            /* connect */
            vmsg("%s: video device connected!!!\n", __func__);
            _handle_video_connection(me, true);
        }
        break;

    default:
        pr_err("%s: invalid connection(%d-%d)\n",
                __func__, local->index, media_entity_type(remote->entity));
        return -EINVAL;
    }

    return 0;
}

static const struct media_entity_operations nxp_decimator_media_ops = {
    .link_setup = nxp_decimator_link_setup,
};

/**
 * internal func
 */
static int _init_entities(struct nxp_decimator *me)
{
    struct v4l2_subdev *sd = &me->subdev;
    struct media_pad *pads = me->pads;
    struct media_entity *entity = &sd->entity;
    struct nxp_capture *parent = nxp_decimator_to_parent(me);
    char dev_name[NXP_VIDEO_MAX_NAME_SIZE] = {0, };
    int ret;

    v4l2_subdev_init(sd, &nxp_decimator_subdev_ops);
    sd->internal_ops = &nxp_decimator_internal_ops;
    /* entity name : used app media link setup */
#if 0
    snprintf(sd->name, sizeof(sd->name), "NXP DECIMATOR%d",
            parent->get_module_num(parent));
#else
    snprintf(sd->name, sizeof(sd->name), "NXP DECIMATOR%d",
            parent->get_index_num(parent));
#endif
    sd->grp_id = NXP_CAPTURE_SUBDEV_GROUP_ID;
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pads[NXP_DECIMATOR_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_DECIMATOR_PAD_SOURCE_MEM].flags = MEDIA_PAD_FL_SOURCE;

    entity->ops = &nxp_decimator_media_ops;
    ret = media_entity_init(entity, NXP_DECIMATOR_PAD_MAX, pads, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

#if 0
    snprintf(dev_name, sizeof(dev_name), "VIDEO DECIMATOR%d", parent->get_module_num(parent));
#else
    snprintf(dev_name, sizeof(dev_name), "VIDEO DECIMATOR%d", parent->get_index_num(parent));
#endif
    me->video = create_nxp_video(dev_name, NXP_VIDEO_TYPE_CAPTURE,
            parent->get_v4l2_device(parent), parent->get_alloc_ctx(parent));
    if (!me->video) {
        pr_err("%s: failed to create_nxp_video()\n", __func__);
        goto error_video;
    }

    /* create link subdev to video node */
    ret = media_entity_create_link(entity, NXP_DECIMATOR_PAD_SOURCE_MEM,
            &me->video->vdev.entity, 0, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_create_link()\n", __func__);
        goto error_link;
    }

    return 0;

error_link:
    release_nxp_video(me->video);
error_video:
    media_entity_cleanup(entity);
    return ret;
}

/**
 * public api
 */
int nxp_decimator_init(struct nxp_decimator *me, struct nxp_decimator_platformdata *platdata)
{
    int ret = _init_entities(me);

    if (likely(!ret)) {
        atomic_set(&me->state, NXP_DECIMATOR_STATE_INIT);
        INIT_LIST_HEAD(&me->buffer_list);
        spin_lock_init(&me->slock);
        me->platdata = platdata;
#ifdef CONFIG_ARCH_NXP4330_3200
        INIT_WORK(&me->work, simulation_isr);
#endif
        init_completion(&me->stop_done);
    }

    return ret;
}

void nxp_decimator_cleanup(struct nxp_decimator *me)
{
    atomic_set(&me->state, NXP_DECIMATOR_STATE_UNINIT);
}

int  nxp_decimator_register(struct nxp_decimator *me)
{
    int ret;
    struct nxp_capture *parent = nxp_decimator_to_parent(me);

    snprintf(me->name, sizeof(me->name), "%s%d", "SUBDEV DECIMATOR", parent->get_module_num(parent));

    ret = v4l2_device_register_subdev(parent->get_v4l2_device(parent),
            &me->subdev);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    ret = register_nxp_video(me->video);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_video()\n", __func__);
        v4l2_device_unregister_subdev(&me->subdev);
    }

    vmsg("%s: success!!!\n", __func__);
    return ret;
}

void nxp_decimator_unregister(struct nxp_decimator *me)
{
    unregister_nxp_video(me->video);
    v4l2_device_unregister_subdev(&me->subdev);
}

#ifdef CONFIG_PM
int  nxp_decimator_suspend(struct nxp_decimator *me)
{
	PM_DBGOUT("+%s\n", __func__);
    if (NXP_ATOMIC_READ(&me->state) == NXP_DECIMATOR_STATE_RUNNING) {
        unsigned long flags;
        spin_lock_irqsave(&me->slock, flags);
        if (me->cur_buf == NULL) {
            PM_DBGOUT("%s: decimator current buffer is NULL\n", __func__);
            spin_unlock_irqrestore(&me->slock, flags);
            return -ENOENT;
        }
        spin_unlock_irqrestore(&me->slock, flags);
    }
	PM_DBGOUT("-%s\n", __func__);
    return 0;
}

int  nxp_decimator_resume(struct nxp_decimator *me)
{
    PM_DBGOUT("+%s\n", __func__);
    if (NXP_ATOMIC_READ(&me->state) == NXP_DECIMATOR_STATE_RUNNING) {
        /* struct nxp_capture *parent = nxp_decimator_to_parent(me); */
        PM_DBGOUT("call _hw_configure()\n");
        _hw_configure(me);
        if (me->cur_buf) {
            PM_DBGOUT("call _hw_set_addr(): %p\n", me->cur_buf);
            _hw_set_addr(me, me->cur_buf);
        } else {
            PM_DBGOUT("cur_buf is NULL!!!\n");
        }
        /* parent->run(parent, me); */
    }
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif
