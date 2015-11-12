/* #define DEBUG 1 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>

#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-ion-nxp.h>
#include <media/v4l2-subdev.h>

#include "nxp-v4l2.h"
#include "nxp-video.h"

#ifndef ALIGN
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

#define YUV_STRIDE_ALIGN_FACTOR     64
#define YUV_VSTRIDE_ALIGN_FACTOR    16

#define YUV_STRIDE(w)    ALIGN(w, YUV_STRIDE_ALIGN_FACTOR)
#define YUV_YSTRIDE(w)   (ALIGN(w/2, YUV_STRIDE_ALIGN_FACTOR) * 2)
#define YUV_VSTRIDE(h)   ALIGN(h, YUV_VSTRIDE_ALIGN_FACTOR)

/*
 * static variables
 */

/*
 * supported formats
 * V4L2_PIX_FMT_XXX : reference to include/linux/videodev2.h
 */

static struct nxp_video_format supported_formats[] = {
    {
        /* yuyv 422 continuous, 16bit per pixel */
        .name        = "YUV 4:2:2 packed, YCbYCr",
        .pixelformat = V4L2_PIX_FMT_YUYV,
        .mbus_code   = V4L2_MBUS_FMT_YUYV8_2X8,
        .num_planes  = 1,
        .num_sw_planes = 1,
        .is_separated = false,
    }, {
        /* yuv 420 1 plane, 12bit per pixel */
        .name        = "YUV 4:2:0 separated 1-planar, YCbYCr",
        .pixelformat = V4L2_PIX_FMT_YUV420,
        .mbus_code   = V4L2_MBUS_FMT_YUYV8_1_5X8,
        .num_planes  = 1,
        .num_sw_planes = 3,
        .is_separated = false,
    }, {
        /* yuv 422 1 plane, 16bit per pixel */
        .name        = "YUV 4:2:2 separated 1-planar, Y/CbCr",
        .pixelformat = V4L2_PIX_FMT_NV16,
        .mbus_code   = V4L2_MBUS_FMT_YUYV8_1_5X8,
        .num_planes  = 1,
        .num_sw_planes = 3,
        .is_separated = false,
    }, {
        /* yuv 420 1 plane, 12bit per pixel */
        .name        = "YUV 4:2:0 separated 1-planar, Y/CrCb",
        .pixelformat = V4L2_PIX_FMT_NV21,
        .mbus_code   = V4L2_MBUS_FMT_YUYV8_1_5X8,
        .num_planes  = 1,
        .num_sw_planes = 2,
        .is_separated = false,
    }, {
        /* yuv 420 separated 3 plane, 12bit per pixel */
        .name        = "YUV 4:2:0 separated 3-planar, YCbYCr",
        .pixelformat = V4L2_PIX_FMT_YUV420M,
        .mbus_code   = V4L2_MBUS_FMT_YUYV8_1_5X8,
        .num_planes  = 3,
        .num_sw_planes = 3,
        .is_separated = true,

    }, {
        /* yuv 422 separated 3 plane, 16bit per pixel */
        .name        = "YUV 4:2:2 separated 3-planar, YCbYCr",
        .pixelformat = V4L2_PIX_FMT_YUV422P,
        .mbus_code   = V4L2_MBUS_FMT_YUYV8_1X16,
        .num_planes  = 3,
        .num_sw_planes = 3,
        .is_separated = true,
    }, {
        /* yuv 444 separated 3 plane, 24bit per pixel */
        .name        = "YUV 4:4:4 separated 3-planar, YCbYCr",
        .pixelformat = V4L2_PIX_FMT_YUV444,
        .mbus_code   = V4L2_MBUS_FMT_YUV8_1X24,
        .num_planes  = 3,
        .num_sw_planes = 3,
        .is_separated = true,
    }, {
        /* rgb 565 */
        .name        = "RGB 565",
        .pixelformat = V4L2_PIX_FMT_RGB565,
        .mbus_code   = V4L2_MBUS_FMT_RGB565_2X8_LE,
        .num_planes  = 1,
        .num_sw_planes = 1,
        .is_separated = false,
    }, {
        /* rgb 8888 */
        .name        = "RGB 32",
        .pixelformat = V4L2_PIX_FMT_RGB32,
        .mbus_code   = V4L2_MBUS_FMT_XRGB8888_4X8_LE,
        .num_planes  = 1,
        .num_sw_planes = 1,
        .is_separated = false,
    }
};

static struct nxp_video_format *
_find_format(u32 pixelformat, int index)
{
    struct nxp_video_format *fmt, *def_fmt = NULL;
    unsigned int i;

    if (index >= ARRAY_SIZE(supported_formats))
        return NULL;

    for (i = 0; i < ARRAY_SIZE(supported_formats); ++i) {
        fmt = &supported_formats[i];
        if (pixelformat == fmt->pixelformat)
            return fmt;
        if (index == i)
            def_fmt = fmt;
    }
    return def_fmt;
}

#if 1
static int
_set_plane_size(struct nxp_video_frame *frame, unsigned int sizes[])
{
    u32 y_stride = ALIGN(frame->width, 32);
    u32 y_size = y_stride * ALIGN(frame->height, 16);

    switch (frame->format.pixelformat) {
    case V4L2_PIX_FMT_YUYV:
        frame->size[0] = sizes[0] = y_size << 1;
        frame->stride[0] = y_stride << 1;
        break;

    case V4L2_PIX_FMT_YUV420M:
#if 0
        frame->size[0] = sizes[0] = frame->width * frame->height;
        frame->size[1] = sizes[1] =
        frame->size[2] = sizes[2] = frame->size[0] >> 2;

        frame->stride[0] = frame->width;
        frame->stride[1] = frame->stride[2] = frame->width >> 1;
#else
        frame->size[0] = sizes[0] = YUV_YSTRIDE(frame->width) * YUV_VSTRIDE(frame->height);
        frame->size[1] = sizes[1] =
        frame->size[2] = sizes[2] = YUV_STRIDE(frame->width/2) * YUV_VSTRIDE(frame->height/2);

        frame->stride[0] = YUV_YSTRIDE(frame->width);
        frame->stride[1] = YUV_STRIDE(frame->width/2);
        frame->stride[2] = YUV_STRIDE(frame->width/2);
#endif
        break;

    case V4L2_PIX_FMT_YUV420:
        frame->size[0] = sizes[0] = y_size;
        frame->size[1] = sizes[1] =
        frame->size[2] = sizes[2] = ALIGN(y_stride >> 1, 16) * ALIGN(frame->height >> 1, 16);

        frame->stride[0] = y_stride;
        frame->stride[1] = frame->stride[2] = ALIGN(y_stride >> 1, 16);
        break;

    case V4L2_PIX_FMT_NV16:
        frame->size[0] = sizes[0] =
        frame->size[1] = sizes[1] = y_size;

        frame->stride[0] = frame->stride[1] = y_stride;
        break;

    case V4L2_PIX_FMT_NV21:
        frame->size[0] = sizes[0] = y_size;
        frame->size[1] = y_stride * ALIGN(frame->height >> 1, 16);

        frame->stride[0] = y_stride;
        frame->stride[1] = y_stride;
        break;

    case V4L2_PIX_FMT_YUV422P:
        frame->size[0] = sizes[0] = y_size;
        frame->size[1] = frame->size[2] = sizes[1] = sizes[2] = y_size >> 1;

        frame->stride[0] = y_stride;
        frame->stride[1] = frame->stride[2] = ALIGN(y_stride >> 1, 16);
        break;

    case V4L2_PIX_FMT_YUV444:
        frame->size[0] = frame->size[1] = frame->size[2] = sizes[0] =
            sizes[1] = sizes[2] = y_size;
        frame->stride[0] = frame->stride[1] = frame->stride[2] =
            y_stride;
        break;

    case V4L2_PIX_FMT_RGB565:
        frame->size[0] = sizes[0] = (frame->width * frame->height) << 1;
        frame->stride[0] = frame->width << 1;
        break;

    case V4L2_PIX_FMT_RGB32:
        frame->size[0] = sizes[0] = (frame->width * frame->height) << 2;
        frame->stride[0] = (frame->width) << 2;
        break;

    default:
        pr_err("%s: unknown format(%d)\n", __func__, frame->format.pixelformat);
        return -EINVAL;
    }

    return 0;
}
#else
static int
_set_plane_size(struct nxp_video_frame *frame, unsigned int sizes[])
{
    switch (frame->format.pixelformat) {
    case V4L2_PIX_FMT_YUYV:
        frame->size[0] = sizes[0] = YUV_STRIDE(frame->width*2) * YUV_VSTRIDE(frame->height);
        frame->stride[0] = YUV_STRIDE(frame->width*2);
        break;

    case V4L2_PIX_FMT_YUV420M:
        frame->size[0] = sizes[0] = YUV_YSTRIDE(frame->width) * YUV_VSTRIDE(frame->height);
        frame->size[1] = sizes[1] =
        frame->size[2] = sizes[2] = YUV_STRIDE(frame->width/2) * YUV_VSTRIDE(frame->height/2);

        frame->stride[0] = YUV_YSTRIDE(frame->width);
        frame->stride[1] = YUV_STRIDE(frame->width/2);
        frame->stride[2] = YUV_STRIDE(frame->width/2);
        break;

    case V4L2_PIX_FMT_YUV420:
        frame->size[0] = sizes[0] = YUV_YSTRIDE(frame->width) * YUV_VSTRIDE(frame->height);
        frame->size[1] = sizes[1] =
        frame->size[2] = sizes[2] = YUV_STRIDE(frame->width/2) * YUV_VSTRIDE(frame->height/2);

        frame->stride[0] = YUV_YSTRIDE(frame->width);
        frame->stride[1] = YUV_STRIDE(frame->width/2);
        frame->stride[2] = YUV_STRIDE(frame->width/2);
        break;

    case V4L2_PIX_FMT_NV16:
        frame->size[0] = sizes[0] =
        frame->size[1] = sizes[1] = YUV_STRIDE(frame->width) * YUV_VSTRIDE(frame->height);

        frame->stride[0] = frame->stride[1] = YUV_STRIDE(frame->width);
        break;

    case V4L2_PIX_FMT_NV21:
        frame->size[0] = sizes[0] = YUV_STRIDE(frame->width) * YUV_VSTRIDE(frame->height);
        frame->size[1] = YUV_STRIDE(frame->width) * YUV_VSTRIDE(frame->height/2);

        frame->stride[0] = YUV_STRIDE(frame->width);
        frame->stride[1] = YUV_STRIDE(frame->width);
        break;

    case V4L2_PIX_FMT_YUV422P:
        frame->size[0] = sizes[0] = YUV_STRIDE(frame->width) * YUV_VSTRIDE(frame->height);
        frame->size[1] = frame->size[2] = sizes[1] = sizes[2] = YUV_STRIDE(frame->width) * YUV_VSTRIDE(frame->height/2);

        frame->stride[0] = YUV_STRIDE(frame->width);
        frame->stride[1] = frame->stride[2] = YUV_STRIDE(frame->width);
        break;

    case V4L2_PIX_FMT_YUV444:
        frame->size[0] = frame->size[1] = frame->size[2] = sizes[0] =
            sizes[1] = sizes[2] = YUV_STRIDE(frame->width) * YUV_VSTRIDE(frame->height);
        frame->stride[0] = frame->stride[1] = frame->stride[2] =
            YUV_STRIDE(frame->width);
        break;

    case V4L2_PIX_FMT_RGB565:
        frame->size[0] = sizes[0] = (frame->width * frame->height) << 1;
        frame->stride[0] = frame->width << 1;
        break;

    case V4L2_PIX_FMT_RGB32:
        frame->size[0] = sizes[0] = (frame->width * frame->height) << 2;
        frame->stride[0] = (frame->width) << 2;
        break;

    default:
        pr_err("%s: unknown format(%d)\n", __func__, frame->format.pixelformat);
        return -EINVAL;
    }

    return 0;
}
#endif

static unsigned long
plane_addr(struct vb2_buffer *vb, u32 plane_no)
{
    void *cookie = vb2_plane_cookie(vb, plane_no);
    dma_addr_t addr = 0;
    WARN_ON(vb2_ion_dma_address(cookie, &addr) != 0);
    return addr;
}

static struct v4l2_subdev *
_get_remote_subdev(struct nxp_video *me, u32 type, u32 *pad)
{
    struct media_pad *remote;

    if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        remote = media_entity_remote_source(&me->pads[1]); /* output, I'm source */
    else
        remote = media_entity_remote_source(&me->pads[0]); /* capture, I'm sink */

    if (!remote ||
        media_entity_type(remote->entity) != MEDIA_ENT_T_V4L2_SUBDEV)
        return NULL;

    if (pad)
        *pad = remote->index;

    return media_entity_to_v4l2_subdev(remote->entity);
}

static int
_fill_nxp_video_buffer(struct nxp_video_buffer *buf, struct vb2_buffer *vb)
{
    int i;
    u32 type = vb->vb2_queue->type;
    struct nxp_video *me = vb->vb2_queue->drv_priv;
    struct nxp_video_frame *frame;
    bool is_separated;

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        frame = &me->frame[0];
    else
        frame = &me->frame[1];

    is_separated = frame->format.is_separated;
    for (i = 0; i < frame->format.num_sw_planes; i++) {
        if (i == 0 || is_separated)
            buf->dma_addr[i] = plane_addr(vb, i);
        else
            buf->dma_addr[i] = buf->dma_addr[i-1] + frame->size[i-1];
        buf->stride[i] = frame->stride[i];
        pr_debug("[BUF plane %d] addr(0x%x), s(%d)\n",
                i, buf->dma_addr[i], buf->stride[i]);
    }

    buf->consumer_index = 0;
    return 0;
}

static inline struct nxp_buffer_consumer *
_find_consumer(struct nxp_video *me, struct list_head *head, int index)
{
    int i;
    unsigned long flags;
    struct nxp_buffer_consumer *c = NULL;
    struct list_head *cur = head->next;

    spin_lock_irqsave(&me->lock_consumer, flags);
    if (list_empty(head)) {
        spin_unlock_irqrestore(&me->lock_consumer, flags);
        pr_err("%s: consumer list empty!!!\n", __func__);
        return NULL;
    }
    for (i = 0; i <= index; i++) {
        c = list_entry(cur, struct nxp_buffer_consumer, list);
        cur = cur->next;
    }
    spin_unlock_irqrestore(&me->lock_consumer, flags);

    return c;
}

/*
 * consumer call this
 */
static int buffer_done(struct nxp_video_buffer *buf)
{
    struct vb2_buffer *vb = buf->priv;
    struct nxp_video *me = vb->vb2_queue->drv_priv;
    int consumer_count;
    struct list_head *cl;
    u32 ci = buf->consumer_index;
    u32 type = vb->vb2_queue->type;

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        consumer_count = me->sink_consumer_count;
        cl = &me->sink_consumer_list;
    } else {
        consumer_count = me->source_consumer_count;
        cl = &me->source_consumer_list;
    }

    pr_debug("%s: type(0x%x), ci(%d), count(%d)\n",
             __func__, type, ci, consumer_count);

    if (ci >= consumer_count) {
        /* TODO: timestamp */
        do_gettimeofday(&vb->v4l2_buf.timestamp);
        vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
    } else {
        struct nxp_buffer_consumer *nc = _find_consumer(me, cl, ci);
        nc->queue(buf, nc->priv);
    }

    return 0;
}

/*
 * vb2_ops
 */

/*
 * queue_setup() called from vb2_reqbufs()
 * setup plane number, plane size
 */
static int nxp_vb2_queue_setup(struct vb2_queue *q, const struct v4l2_format *fmt,
        unsigned int *num_buffers, unsigned int *num_planes,
        unsigned int sizes[], void *alloc_ctxs[])
{
    int ret;
    int i;
    struct nxp_video *me = q->drv_priv;
    struct nxp_video_frame *frame = NULL;

    pr_debug("%s\n", __func__);

    if (q->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        frame = &me->frame[0];
    else
        frame = &me->frame[1];

    if (!frame) {
        pr_err("%s: can't find frame for type(0x%x)\n", __func__, q->type);
        return -EINVAL;
    }

    ret = _set_plane_size(frame, sizes);
    if (ret < 0) {
        pr_err("%s: failed to _set_plane_size()\n", __func__);
        return ret;
    }

    *num_planes = (unsigned int)(frame->format.num_planes);

    for (i = 0; i < *num_planes; ++i) {
        alloc_ctxs[i] = me->vb2_alloc_ctx;
    }

    return 0;
}

/* TODO : unlock? */
static void nxp_vb2_wait_prepare(struct vb2_queue *q)
{
    pr_debug("%s\n", __func__);
}

/* TODO : lock? */
static void nxp_vb2_wait_finish(struct vb2_queue *q)
{
    pr_debug("%s\n", __func__);
}

/* allocate nxp_video_buffer */
static int nxp_vb2_buf_init(struct vb2_buffer *vb)
{
    struct nxp_video_buffer **bufs;
    struct nxp_video_buffer *buf;
    struct nxp_video *me = vb->vb2_queue->drv_priv;
    int index = vb->v4l2_buf.index;
    u32 type = vb->vb2_queue->type;

    pr_debug("%s: type(0x%x)\n", __func__, type);

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        bufs = me->sink_bufs;
    } else if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        bufs = me->source_bufs;
    } else {
        pr_err("%s: invalid buffer type(0x%x)\n", __func__, type);
        return -EINVAL;
    }

    if (!bufs[index]) {
        buf = kzalloc(sizeof(*buf), GFP_KERNEL);
        if (!buf) {
            pr_err("%s: failed to allocat nxp_video_buffer\n", __func__);
            return -ENOMEM;
        }
        buf->priv        = vb;
        buf->cb_buf_done = buffer_done;
        bufs[index]  = buf;
    }

    return 0;
}

static void nxp_vb2_buf_cleanup(struct vb2_buffer *vb)
{
    struct nxp_video_buffer **bufs;
    struct nxp_video_buffer *buf;
    struct nxp_video *me = vb->vb2_queue->drv_priv;
    int index = vb->v4l2_buf.index;
    u32 type = vb->vb2_queue->type;

    pr_debug("%s\n", __func__);

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        bufs = me->sink_bufs;
    } else if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        bufs = me->source_bufs;
    } else {
        pr_err("%s: invalid buffer type(0x%x)\n", __func__, type);
        return;
    }

    buf = bufs[index];
    if (buf) {
        kfree(buf);
        bufs[index] = NULL;
    }
}


static int nxp_vb2_buf_prepare(struct vb2_buffer *vb)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_vb2_buf_finish(struct vb2_buffer *vb)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_vb2_start_streaming(struct vb2_queue *q, unsigned int count)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_vb2_stop_streaming(struct vb2_queue *q)
{
    pr_debug("%s\n", __func__);
    return 0;
}

/* real queue!!! */
static void nxp_vb2_buf_queue(struct vb2_buffer *vb)
{
    struct nxp_video *me = vb->vb2_queue->drv_priv;
    struct nxp_video_buffer *buf;
    struct nxp_buffer_consumer *c;
    u32 type = vb->vb2_queue->type;
    int ret;

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        buf = me->sink_bufs[vb->v4l2_buf.index];
        c = _find_consumer(me, &me->sink_consumer_list, 0);
    } else if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        buf = me->source_bufs[vb->v4l2_buf.index];
        c = _find_consumer(me, &me->source_consumer_list, 0);
    } else {
        pr_err("%s: invalid buffer type(0x%x)\n", __func__, type);
        return;
    }

    if (!buf || !c) {
        pr_err("%s: No consumer or No buf!!!\n", __func__);
        return;
    }

    ret = _fill_nxp_video_buffer(buf, vb);
    if (ret < 0) {
        pr_err("%s: fatal error!!!\n", __func__);
        vb2_buffer_done(vb, VB2_BUF_STATE_ERROR);
        return;
    }

    pr_debug("%s buf(%p)\n", __func__, buf);
    c->queue(buf, c->priv);

    /* for m2m : TODO */
#if 0
    if (me->type == NXP_VIDEO_TYPE_M2M)
        v4l2_m2m_buf_queue(me->m2m_ctx, vb);
#endif
}

static struct vb2_ops nxp_vb2_ops = {
    .queue_setup    = nxp_vb2_queue_setup,
    .wait_prepare   = nxp_vb2_wait_prepare,
    .wait_finish    = nxp_vb2_wait_finish,
    .buf_init       = nxp_vb2_buf_init,
    .buf_prepare    = nxp_vb2_buf_prepare,
    .buf_finish     = nxp_vb2_buf_finish,
    .buf_cleanup    = nxp_vb2_buf_cleanup,
    .start_streaming = nxp_vb2_start_streaming,
    .stop_streaming = nxp_vb2_stop_streaming,
    .buf_queue      = nxp_vb2_buf_queue,
};

/**
 * m2m specific
 */
static int m2m_queue_init(void *priv, struct vb2_queue *src_vq,
        struct vb2_queue *dst_vq)
{
    struct nxp_video *me = priv;
    int ret;

    memset(src_vq, 0, sizeof(*src_vq));
    src_vq->name = kasprintf(GFP_KERNEL, "%s-src", me->name);
    src_vq->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    src_vq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
    src_vq->drv_priv = me;
    src_vq->ops = &nxp_vb2_ops;
    src_vq->mem_ops = &vb2_ion_memops;
    src_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);

    ret = vb2_queue_init(src_vq);
    if (ret) {
        pr_err("%s: failed to vb2_queue_init(src_vq)\n", __func__);
        return ret;
    }

    memset(dst_vq, 0, sizeof(*dst_vq));
    dst_vq->name = kasprintf(GFP_KERNEL, "%s-dst", me->name);
    dst_vq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    dst_vq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
    dst_vq->drv_priv = me;
    dst_vq->ops = &nxp_vb2_ops;
    dst_vq->mem_ops = &vb2_ion_memops;
    dst_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);

    return vb2_queue_init(dst_vq);
}

/*
 * v4l2_m2m_ops
 */
static void nxp_m2m_device_run(void *priv)
{
    pr_debug("%s\n", __func__);
}

static int nxp_m2m_job_ready(void *priv)
{
    /* TODO */
#if 0
    struct nxp_video *me = priv;

    pr_debug("%s\n", __func__);

    if (v4l2_m2m_num_src_bufs_ready(me->m2m_ctx) < 1 ||
        v4l2_m2m_num_dst_bufs_ready(m2->m2m_ctx) < 1) {
        pr_err("%s: Not enough buffers available\n", __func__);
        return 0;
    }
#endif

    return 1;
}

static void nxp_m2m_job_abort(void *priv)
{
    /* TODO */
    pr_debug("%s\n", __func__);
}

static void nxp_m2m_lock(void *priv)
{
    /* TODO */
    pr_debug("%s\n", __func__);
}

static void nxp_m2m_unlock(void *priv)
{
    /* TODO */
    pr_debug("%s\n", __func__);
}

static struct v4l2_m2m_ops nxp_m2m_ops = {
    .device_run = nxp_m2m_device_run,
    .job_ready  = nxp_m2m_job_ready,
    .job_abort  = nxp_m2m_job_abort,
    .lock       = nxp_m2m_lock,
    .unlock     = nxp_m2m_unlock,
};

/*
 * v4l2_ioctl_ops
 */
/* querycap: check capture, out, m2m */
static int nxp_video_querycap(struct file *file, void *fh,
        struct v4l2_capability *cap)
{
    struct nxp_video *me = file->private_data;

    strlcpy(cap->driver, me->name, sizeof(cap->driver));
    strlcpy(cap->card, me->vdev.name, sizeof(cap->card));
    strlcpy(cap->bus_info, "media", sizeof(cap->bus_info));
    cap->version = KERNEL_VERSION(1, 0, 0);

    pr_debug("%s: devname(%s)\n", __func__, cap->driver);

    switch(me->type) {
    case NXP_VIDEO_TYPE_CAPTURE:
        cap->capabilities = V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_STREAMING;
        break;
    case NXP_VIDEO_TYPE_OUT:
        cap->capabilities = V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_STREAMING;
        break;
    case NXP_VIDEO_TYPE_M2M:
        cap->capabilities = V4L2_CAP_VIDEO_CAPTURE_MPLANE |
            V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_STREAMING;
        break;
    default:
        pr_err("%s: invalid type(%d)\n", __func__, me->type);
        return -EINVAL;
    }

    return 0;
}

static int nxp_video_enum_format(struct file *file, void *fh,
        struct v4l2_fmtdesc *f)
{
    /* TODO */
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_video_get_format(struct file *file, void *fh,
        struct v4l2_format *f)
{
    /* TODO */
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_video_set_format(struct file *file, void *fh,
        struct v4l2_format *f)
{
    struct nxp_video *me = file->private_data;
    struct v4l2_pix_format_mplane *pix;
    struct nxp_video_format *format;
    struct v4l2_subdev_format mbus_fmt;
    struct v4l2_subdev *subdev;
    struct nxp_video_frame *frame;
    u32 pad;
    int ret;
    int i;

    if (me->vbq && (f->type != me->vbq->type)) {
        pr_err("%s: type is different(%d/%d)\n", __func__, f->type, me->vbq->type);
        return -EINVAL;
    }

    if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        frame = &me->frame[0];
    } else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        frame = &me->frame[1];
    } else {
        pr_err("%s: invalid type(0x%x)\n", __func__, f->type);
        return -EINVAL;
    }

    pix = &f->fmt.pix_mp;
    format = _find_format(pix->pixelformat, 0);
    if (!format) {
        pr_err("%s: unsupported format!!!\n", __func__);
        return -EINVAL;
    }

    vmsg("%s: name(%s), width %d, height %d\n", __func__, me->name, pix->width, pix->height);

    subdev = _get_remote_subdev(me, f->type, &pad);
    if (!subdev) {
        pr_err("%s: can't get remote source subdev\n", __func__);
        return -EINVAL;
    }

    memset(&mbus_fmt, 0, sizeof(mbus_fmt));
    mbus_fmt.pad = pad;
    mbus_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    mbus_fmt.format.width  = pix->width;
    mbus_fmt.format.height = pix->height;
    mbus_fmt.format.code   = format->mbus_code;
    mbus_fmt.format.colorspace = pix->colorspace;
    mbus_fmt.format.field  = pix->field;

    /* call to subdev */
    ret = v4l2_subdev_call(subdev, pad, set_fmt, NULL, &mbus_fmt);
    if (ret < 0) {
        pr_err("%s: failed to subdev set_fmt()\n", __func__);
        return ret;
    }

    frame->format.name = format->name;
    frame->format.pixelformat = format->pixelformat;
    frame->format.mbus_code   = format->mbus_code;
    frame->format.num_planes  = format->num_planes;
    frame->format.num_sw_planes  = format->num_sw_planes;
    frame->format.is_separated  = format->is_separated;
    frame->width  = pix->width;
    frame->height = pix->height;

    for (i = 0; i < format->num_planes; ++i) {
        frame->stride[i] = pix->plane_fmt[i].bytesperline;
        frame->size[i] = pix->plane_fmt[i].sizeimage;
    }

    pr_debug("%s: %s success!!!\n", __func__, me->name);
    return 0;
}

static int nxp_video_try_format(struct file *file, void *fh,
        struct v4l2_format *f)
{
    /* TODO */
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_video_reqbufs(struct file *file, void *fh,
        struct v4l2_requestbuffers *b)
{
    struct nxp_video *me = file->private_data;
    pr_debug("%s: %s\n", __func__, me->name);
    if (me->vbq) { /* capture, out */
        return vb2_reqbufs(me->vbq, b); /* call to queue_setup */
    } else { /* m2m */
        struct vb2_queue *vq = v4l2_m2m_get_vq(me->m2m_ctx, b->type);
        return vb2_reqbufs(vq, b);
    }
}

static int nxp_video_querybuf(struct file *file, void *fh,
        struct v4l2_buffer *b)
{
    struct nxp_video *me = file->private_data;
    if (me->vbq) {
        return vb2_querybuf(me->vbq, b);
    } else {
        struct vb2_queue *vq = v4l2_m2m_get_vq(me->m2m_ctx, b->type);
        return vb2_querybuf(vq, b);
    }
}

static int nxp_video_qbuf(struct file *file, void *fh,
        struct v4l2_buffer *b)
{
    struct nxp_video *me = file->private_data;
    pr_debug("%s: %s, index %d\n", __func__, me->name, b->index);
    if (me->vbq) {
        return vb2_qbuf(me->vbq, b);
    } else {
        struct vb2_queue *vq = v4l2_m2m_get_vq(me->m2m_ctx, b->type);
        return vb2_qbuf(vq, b);
    }
}

static int nxp_video_dqbuf(struct file *file, void *fh,
        struct v4l2_buffer *b)
{
    struct nxp_video *me = file->private_data;
    pr_debug("%s\n", __func__);
    if (me->vbq) {
        return vb2_dqbuf(me->vbq, b, file->f_flags & O_NONBLOCK);
    } else {
        struct vb2_queue *vq = v4l2_m2m_get_vq(me->m2m_ctx, b->type);
        return vb2_dqbuf(vq, b, file->f_flags & O_NONBLOCK);
    }
}

static int nxp_video_streamon(struct file *file, void *fh,
        enum v4l2_buf_type i)
{
    int ret;
    u32 pad;
    struct nxp_video *me = file->private_data;
    struct v4l2_subdev *subdev = _get_remote_subdev(me, i, &pad);
    void *hostdata_back;

     vmsg("%s: me %p, %s\n", __func__, me, me->name);

    if (me->vbq) {
        ret = vb2_streamon(me->vbq, i);
        if (ret < 0) {
            pr_err("%s: failed to vb2_streamon()\n", __func__);
            return ret;
        }
    } else {
        struct vb2_queue *vq = v4l2_m2m_get_vq(me->m2m_ctx, i);
        ret = vb2_streamon(vq, i);
        if (ret < 0) {
            pr_err("%s: m2m, failed to vb2_streamon()\n", __func__);
            return ret;
        }
    }

    /* for mlc subdev */
    hostdata_back = v4l2_get_subdev_hostdata(subdev);
    v4l2_set_subdev_hostdata(subdev, me->name);
    ret = v4l2_subdev_call(subdev, video, s_stream, 1);
    v4l2_set_subdev_hostdata(subdev, hostdata_back);
    return ret;
}

static int nxp_video_streamoff(struct file *file, void *fh,
        enum v4l2_buf_type i)
{
    int ret;
    u32 pad;
    struct nxp_video *me;
    struct v4l2_subdev *subdev;
    void *hostdata_back;


    me = file->private_data;
    subdev = _get_remote_subdev(me, i, &pad);

    if (me->vbq) {
        ret = vb2_streamoff(me->vbq, i);
        if (ret < 0) {
            printk(KERN_ERR "%s: failed to vb2_streamoff() %s\n", __func__, me->name);
            return 0;
        }
    } else {
        struct vb2_queue *vq = v4l2_m2m_get_vq(me->m2m_ctx, i);
        ret = vb2_streamoff(vq, i);
        if (ret < 0) {
            pr_err(KERN_ERR "%s: m2m, failed to vb2_streamoff() %s\n", __func__, me->name);
            return 0;
        }
    }

    vmsg("%s %s\n", __func__, me->name);

    hostdata_back = v4l2_get_subdev_hostdata(subdev);
    v4l2_set_subdev_hostdata(subdev, me->name);
    ret = v4l2_subdev_call(subdev, video, s_stream, 0);
    v4l2_set_subdev_hostdata(subdev, hostdata_back);

    vmsg("%s: %s exit\n", __func__, me->name);

    return ret;
}

static int nxp_video_enum_input(struct file *file, void *fh,
        struct v4l2_input *inp)
{
    /* TODO */
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_video_get_input(struct file *file, void *fh,
        unsigned int *i)
{
    pr_debug("%s\n", __func__);
    *i = 0;
    return 0;
}

static int nxp_video_set_input(struct file *file, void *fh,
        unsigned int i)
{
    pr_debug("%s\n", __func__);
    return i == 0 ? 0 : -EINVAL;
}

static int nxp_video_enum_output(struct file *file, void *fh,
        struct v4l2_output *a)
{
    /* TODO */
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_video_get_output(struct file *file, void *fh,
        unsigned int *i)
{
    pr_debug("%s\n", __func__);
    *i = 0;
    return 0;
}

static int nxp_video_set_output(struct file *file, void *fh,
        unsigned int i)
{
    pr_debug("%s\n", __func__);
    return i == 0 ? 0 : -EINVAL;
}

static int nxp_video_cropcap(struct file *file, void *fh,
        struct v4l2_cropcap *a)
{
    int ret;
    u32 pad;
    struct nxp_video *me = file->private_data;
    struct v4l2_subdev *subdev = _get_remote_subdev(me, a->type, &pad);

    pr_debug("%s\n", __func__);

    ret = v4l2_subdev_call(subdev, video, cropcap, a);
    if (ret < 0) {
        pr_err("%s: failed to subdev cropcap\n", __func__);
    }
    return ret == -ENOIOCTLCMD ? -EINVAL : ret;
}

static int nxp_video_get_crop(struct file *file, void *fh,
        struct v4l2_crop *a)
{
    int ret;
    u32 pad;
    struct nxp_video *me = file->private_data;
    struct v4l2_subdev *subdev = _get_remote_subdev(me, a->type, &pad);
    struct v4l2_subdev_crop subdev_crop;

    pr_debug("%s\n", __func__);

    subdev_crop.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    subdev_crop.pad = ((me->type == NXP_VIDEO_TYPE_CAPTURE) ||
        (me->type == NXP_VIDEO_TYPE_M2M)) ? 1 : 0;

    ret = v4l2_subdev_call(subdev, pad, get_crop, NULL, &subdev_crop);
    if (ret < 0) {
        pr_err("%s: failed to subdev get_crop\n", __func__);
        return ret;
    }

    a->type = me->vbq->type;
    a->c    = subdev_crop.rect;
    return 0;
}

static int nxp_video_set_crop(struct file *file, void *fh,
        struct v4l2_crop *a)
{
    int ret;
    u32 pad;
    struct nxp_video *me = file->private_data;
    struct v4l2_subdev *subdev = _get_remote_subdev(me, a->type, &pad);
    struct v4l2_subdev_crop subdev_crop;

    vmsg("%s: name %s, crop pad %d\n", __func__, me->name, a->pad);

    subdev_crop.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    /* TODO */
#if 0
    subdev_crop.pad = ((me->type == NXP_VIDEO_TYPE_CAPTURE) ||
        (me->type == NXP_VIDEO_TYPE_M2M)) ? 1 : 0;
#else
    if (me->type == NXP_VIDEO_TYPE_OUT) {
        if (a->pad == 0)
            subdev_crop.pad = pad;
        else
            subdev_crop.pad = a->pad;
    } else {
        /* add for clipper cropping */
        struct nxp_video_frame *frame;
        frame = &me->frame[0];
        frame->width = a->c.width;
        frame->height = a->c.height;
        /* end clipper cropping */

        if (a->pad == 0)
            subdev_crop.pad = 1;
        else
            subdev_crop.pad = a->pad;
    }
#endif
    subdev_crop.rect = a->c;

    vmsg("%s: call subdev set_crop\n", __func__);
    ret = v4l2_subdev_call(subdev, pad, set_crop, NULL, &subdev_crop);
    if (ret < 0) {
        pr_err("%s: failed to subdev set_crop, ret %d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int nxp_video_s_ctrl(struct file *file, void *fh,
        struct v4l2_control *ctrl)
{
    int ret;
    u32 pad;
    u32 type;
    struct nxp_video *me;
    struct v4l2_subdev *subdev;

    pr_debug("%s\n", __func__);

    me = file->private_data;
    type = (me->type == NXP_VIDEO_TYPE_CAPTURE) ?
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE : V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    subdev = _get_remote_subdev(me, type, &pad);

    ret = v4l2_subdev_call(subdev, core, s_ctrl, ctrl);
    if (ret < 0)
        pr_err("%s: failed to subdev s_ctrl\n", __func__);
    return ret;
}

static struct v4l2_ioctl_ops nxp_video_ioctl_ops = {
    .vidioc_querycap                = nxp_video_querycap,
    .vidioc_enum_fmt_vid_cap_mplane = nxp_video_enum_format,
    .vidioc_enum_fmt_vid_out_mplane = nxp_video_enum_format,
    .vidioc_g_fmt_vid_cap_mplane    = nxp_video_get_format,
    .vidioc_g_fmt_vid_out_mplane    = nxp_video_get_format,
    .vidioc_s_fmt_vid_cap_mplane    = nxp_video_set_format,
    .vidioc_s_fmt_vid_out_mplane    = nxp_video_set_format,
    .vidioc_try_fmt_vid_cap_mplane  = nxp_video_try_format,
    .vidioc_reqbufs                 = nxp_video_reqbufs,
    .vidioc_querybuf                = nxp_video_querybuf,
    .vidioc_qbuf                    = nxp_video_qbuf,
    .vidioc_dqbuf                   = nxp_video_dqbuf,
    .vidioc_streamon                = nxp_video_streamon,
    .vidioc_streamoff               = nxp_video_streamoff,
    .vidioc_enum_input              = nxp_video_enum_input,
    .vidioc_g_input                 = nxp_video_get_input,
    .vidioc_s_input                 = nxp_video_set_input,
    .vidioc_enum_output             = nxp_video_enum_output,
    .vidioc_g_output                = nxp_video_get_output,
    .vidioc_s_output                = nxp_video_set_output,
    .vidioc_cropcap                 = nxp_video_cropcap,
    .vidioc_g_crop                  = nxp_video_get_crop,
    .vidioc_s_crop                  = nxp_video_set_crop,
    .vidioc_s_ctrl                  = nxp_video_s_ctrl,
};

/*
 * v4l2_file_operations
 */
static int nxp_video_open(struct file *file)
{
    struct nxp_video *me = video_drvdata(file);
    u32 pad;
    struct v4l2_subdev *sd;
    int ret = 0;

    pr_debug("%s entered : %s\n", __func__, me->name);

    if (me->open_count == 0) {
        memset(me->frame, 0, sizeof(struct nxp_video_frame)*2);

        sd = _get_remote_subdev(me,
                me->type == NXP_VIDEO_TYPE_OUT ?
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE : V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                &pad);

        if (sd)
            ret = v4l2_subdev_call(sd, core, s_power, 1);

        if (me->m2m_dev) {
            me->m2m_ctx = v4l2_m2m_ctx_init(me->m2m_dev, me, m2m_queue_init);
            if (IS_ERR(me->m2m_ctx)) {
                pr_err("%s: failed to v4l2_m2m_ctx_init()\n", __func__);
                return -EINVAL;
            }
        }
    }
    me->open_count++;
    file->private_data = me;

    pr_debug("%s exit\n", __func__);

    return ret;
}

static int nxp_video_release(struct file *file)
{
    struct nxp_video *me = video_drvdata(file);
    u32 pad;
    struct v4l2_subdev *sd;
    int ret = 0;

    me->open_count--;
    if (me->open_count == 0) {
        sd = _get_remote_subdev(me,
                me->type == NXP_VIDEO_TYPE_OUT ?
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE : V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                &pad);
        if (sd) {
            ret = v4l2_subdev_call(sd, core, s_power, 0);
            if (ret < 0) {
                pr_err("%s: failed to subdev_call s_power(ret: %d)\n", __func__, ret);
                return ret;
            }
        }

        if (me->vbq)
            vb2_queue_release(me->vbq);

        if (me->m2m_ctx) {
            v4l2_m2m_ctx_release(me->m2m_ctx);
            me->m2m_ctx = NULL;
        }

    }
    file->private_data = 0;

    return ret;
}

static int nxp_video_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct nxp_video *me = file->private_data;
    return vb2_mmap(me->vbq, vma);
}

static unsigned int nxp_video_poll(struct file *file, struct poll_table_struct *tbl)
{
    struct nxp_video *me = file->private_data;
    return vb2_poll(me->vbq, file, tbl);
}

static struct v4l2_file_operations nxp_video_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = video_ioctl2,
    .open           = nxp_video_open,
    .release        = nxp_video_release,
    .poll           = nxp_video_poll,
    .mmap           = nxp_video_mmap,
};

/*
 * media_entity_operations
 */
static int nxp_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    struct video_device *vdev = media_entity_to_video_device(entity);
    struct nxp_video *me = video_get_drvdata(vdev);

    pr_debug("%s: me type(%d)\n", __func__, me->type);

    switch (local->index | media_entity_type(remote->entity)) {
    case 0 | MEDIA_ENT_T_V4L2_SUBDEV:
        /* capture, m2m : sink
         * video : source
         */
        pr_debug("local %d, link subdev\n", local->index);
        break;

    case 0 | MEDIA_ENT_T_DEVNODE:
        pr_debug("local %d, link videodev\n", local->index);
        break;

    case 1 | MEDIA_ENT_T_V4L2_SUBDEV:
        pr_debug("local %d, link subdev\n", local->index);
        break;

    case 1 | MEDIA_ENT_T_DEVNODE:
        pr_debug("local %d, link videodev\n", local->index);
        break;
    }

    if (flags & MEDIA_LNK_FL_ENABLED)
        pr_debug("====> linked\n");
    else
        pr_debug("====> unlinked\n");

    return 0;
}

static struct media_entity_operations nxp_media_entity_operations = {
    .link_setup = nxp_link_setup,
};

/**
 * callback functions
 */
static int _register_buffer_consumer(struct nxp_video *me,
        struct nxp_buffer_consumer *consumer,
        enum nxp_buffer_consumer_type type)
{
    unsigned long flags;

    if (type >= NXP_BUFFER_CONSUMER_INVALID) {
        pr_err("%s: invalid type(%d)\n", __func__, type);
        return -EINVAL;
    }

    pr_debug("%s: name(%s) type(0x%x), consumer(%p)\n",
            __func__, me->name, type, consumer);

    spin_lock_irqsave(&me->lock_consumer, flags);
    if (type == NXP_BUFFER_CONSUMER_SINK) {
        list_add_tail(&consumer->list, &me->sink_consumer_list);
        me->sink_consumer_count++;
    } else {
        list_add_tail(&consumer->list, &me->source_consumer_list);
        me->source_consumer_count++;
    }
    spin_unlock_irqrestore(&me->lock_consumer, flags);

    return 0;
}

static void _unregister_buffer_consumer(struct nxp_video *me,
        struct nxp_buffer_consumer *consumer,
        enum nxp_buffer_consumer_type type)
{
    unsigned long flags;
    if (type >= NXP_BUFFER_CONSUMER_INVALID) {
        pr_err("%s: invalid type(%d)\n", __func__, type);
        return;
    }

    pr_debug("%s: type(0x%x), consumer(%p)\n",
            __func__, type, consumer);

    spin_lock_irqsave(&me->lock_consumer, flags);
    list_del(&consumer->list);
    if (type == NXP_BUFFER_CONSUMER_SINK) {
        me->sink_consumer_count--;
    } else {
        me->source_consumer_count--;
    }
    spin_unlock_irqrestore(&me->lock_consumer, flags);
}
/*
 * public functions
 */
struct nxp_video *create_nxp_video(char *name, u32 type,
        struct v4l2_device *v4l2_dev, void *vb2_alloc_ctx)
{
    int ret;
    struct vb2_queue *vbq = NULL;
    struct nxp_video *me = kzalloc(sizeof(struct nxp_video), GFP_KERNEL);
    int pad_num = 0;

    if (!me) {
        pr_err("%s: failed to allocate me!!!\n", __func__);
        return NULL;
    }

    snprintf(me->name, sizeof(me->name), "%s", name);
    snprintf(me->vdev.name, sizeof(me->vdev.name), "%s", name);

    me->type          = type;
    me->v4l2_dev      = v4l2_dev;
    me->vb2_alloc_ctx = vb2_alloc_ctx;

    /* pad init */
    /* TODO */
#if 0
    switch (type) {
    case NXP_VIDEO_TYPE_CAPTURE:
        /**
         * capture subdev -> capture ->
         * m2m subdev or out subdev
         */
        me->pads[0].flags = MEDIA_PAD_FL_SINK;
        me->pads[1].flags = MEDIA_PAD_FL_SOURCE;
        pad_num = 2;
        break;

    case NXP_VIDEO_TYPE_OUT:
        /**
         * out -> out subdev
         */
        me->pads[0].flags = MEDIA_PAD_FL_SOURCE;
        pad_num = 1;
        break;

    case NXP_VIDEO_TYPE_M2M:
        /**
         * capture video -> m2m subdev ->
         * m2m video -> out subdev
         */
        me->pads[0].flags = MEDIA_PAD_FL_SINK;
        me->pads[1].flags = MEDIA_PAD_FL_SOURCE;
        pad_num = 2;
        break;

    default:
        pr_err("%s: invalid type(%d)\n", __func__, type);
        kfree(me);
        return NULL;
    }
#else
    me->pads[0].flags = MEDIA_PAD_FL_SINK;
    me->pads[1].flags = MEDIA_PAD_FL_SOURCE;
    pad_num = 2;
#endif

    me->vdev.entity.ops = &nxp_media_entity_operations;
    ret = media_entity_init(&me->vdev.entity, pad_num, me->pads, 0);
    if (ret < 0) {
        kfree(me);
        return NULL;
    }

    mutex_init(&me->lock);

    me->register_buffer_consumer = _register_buffer_consumer;
    me->unregister_buffer_consumer = _unregister_buffer_consumer;

    me->vdev.fops      = &nxp_video_fops;
    me->vdev.ioctl_ops = &nxp_video_ioctl_ops;
    me->vdev.v4l2_dev  = v4l2_dev;
    me->vdev.minor     = -1;
    me->vdev.vfl_type  = VFL_TYPE_GRABBER;
    me->vdev.release   = video_device_release;
    me->vdev.lock      = &me->lock;

    if (type != NXP_VIDEO_TYPE_M2M) {
        vbq = kzalloc(sizeof(*vbq), GFP_KERNEL);
        if (!vbq) {
            pr_err("%s: failed to allocate vbq\n", __func__);
            ret = -ENOMEM;
            goto error_vbq_alloc;
        }
        vbq->type     = type == NXP_VIDEO_TYPE_CAPTURE ?
            V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE : V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        vbq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
        vbq->drv_priv = me;
        vbq->ops      = &nxp_vb2_ops;
        vbq->mem_ops  = &vb2_ion_memops;
        ret = vb2_queue_init(vbq);
        if (ret < 0) {
            pr_err("%s: failed to vb2_queue_init()\n", __func__);
            goto error_vbq_init;
        }
        me->vbq = vbq;
    } else {
        /* m2m */
        me->m2m_dev = v4l2_m2m_init(&nxp_m2m_ops);
        if (IS_ERR(me->m2m_dev)) {
            pr_err("%s: failed to v4l2_m2m_init()\n", __func__);
            ret = -ENOMEM;
            goto error_vbq_alloc;
        }
    }

    INIT_LIST_HEAD(&me->source_consumer_list);
    INIT_LIST_HEAD(&me->sink_consumer_list);
    spin_lock_init(&me->lock_consumer);

    video_set_drvdata(&me->vdev, me);

    pr_debug("%s: success!!!\n", __func__);
    return me;

error_vbq_init:
    if (vbq)
        kfree(vbq);
error_vbq_alloc:
    kfree(me);
    return NULL;
}

void release_nxp_video(struct nxp_video *me)
{
    mutex_destroy(&me->lock);
    media_entity_cleanup(&me->vdev.entity);
    if (me->m2m_dev)
        v4l2_m2m_release(me->m2m_dev);
    if (me->vbq) {
        vb2_queue_release(me->vbq);
        kfree(me->vbq);
    }
    kfree(me);
}

int register_nxp_video(struct nxp_video *me)
{
    int ret;

    me->vdev.v4l2_dev = me->v4l2_dev;

    pr_debug("%s: entered\n", __func__);
    ret = video_register_device(&me->vdev, VFL_TYPE_GRABBER, -1);
    if (ret < 0) {
        pr_err("%s: failed to video_register_device()\n", __func__);
        return ret;
    }

#if 0
    if (me->m2m_dev) {
        me->m2m_ctx = v4l2_m2m_ctx_init(me->m2m_dev, me, m2m_queue_init);
        if (IS_ERR(me->m2m_ctx)) {
            pr_err("%s: failed to v4l2_m2m_ctx_init()\n", __func__);
            video_unregister_device(&me->vdev);
            return -EINVAL;
        }
    }
#endif

    pr_debug("%s: success!!!\n", __func__);
    return 0;
}

void unregister_nxp_video(struct nxp_video *me)
{
    if (me->m2m_ctx) {
        v4l2_m2m_ctx_release(me->m2m_ctx);
        me->m2m_ctx = NULL;
    }

    video_unregister_device(&me->vdev);
}

