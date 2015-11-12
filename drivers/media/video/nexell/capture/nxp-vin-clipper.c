/*#define DEBUG 0*/

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/v4l2-mediabus.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#include <mach/nxp-v4l2-platformdata.h>

#include "nxp-v4l2-common.h"
#include "nxp-video.h"
#include "nxp-v4l2.h"

#include "nxp-capture.h"
#include "nxp-vin-clipper.h"

/* for nexell specific prototype */
#ifdef CONFIG_ARCH_NXP4330_3200
#include <mach/nxp3200.h>
#include <mach/iomap.h>
#else
#include <mach/platform.h>
#include <mach/soc.h>
#endif

#include <linux/timer.h>

#ifdef DEBUG_SYNC
#define DEBUG_SYNC_TIMEOUT_MS   (1000)
#endif

#define LATE_POWERDOWN_TIMEOUT_MS (10000) /* 10 seconds */

/* input pad is exclusive */
enum nxp_vin_input_type {
    NXP_VIN_INPUT_NONE,
    NXP_VIN_INPUT_PARALLEL,
    NXP_VIN_INPUT_MIPI_CSI
};

/* output pad is orring */
enum nxp_vin_output_type {
    NXP_VIN_OUTPUT_NONE,
    NXP_VIN_OUTPUT_MEM,
    NXP_VIN_OUTPUT_DECIMATOR
};

enum nxp_vin_clipper_state {
    NXP_VIN_STATE_UNINIT            = 0,
    NXP_VIN_STATE_INIT              = (1 << 0),
    NXP_VIN_STATE_REGISTERED        = (1 << 1),
    NXP_VIN_STATE_INPUT_CONNECTED   = (1 << 2),
    NXP_VIN_STATE_OUTPUT_CONNECTED  = (1 << 3),
    NXP_VIN_STATE_READY             = (NXP_VIN_STATE_REGISTERED |
        NXP_VIN_STATE_INPUT_CONNECTED | NXP_VIN_STATE_OUTPUT_CONNECTED),
    NXP_VIN_STATE_RUNNING_CLIPPER   = (1 << 4),
    NXP_VIN_STATE_RUNNING_DECIMATOR = (1 << 5),
    NXP_VIN_STATE_STOPPING          = (1 << 30),
    NXP_VIN_STATE_SUSPEND           = (1 << 31)
};


#define subdev_to_nxp_vin(subdev)   \
    container_of(subdev, struct nxp_vin_clipper, subdev)
#define nxp_vin_to_parent(me)       \
    container_of(me, struct nxp_capture, vin_clipper)

/* DEBUG SYNC */
#ifdef DEBUG_SYNC
static void debug_sync(unsigned long priv)
{
    struct nxp_vin_clipper *me = (struct nxp_vin_clipper *)priv;
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);

    printk("VCOUNT:%d, HCOUNT: %d\n", NX_VIP_GetVerCount(module), NX_VIP_GetHorCount(module));
    mod_timer(&me->timer, jiffies + msecs_to_jiffies(DEBUG_SYNC_TIMEOUT_MS));
}
#endif
/**
 * NOTE
 * input connection()
 * --> core_ops->s_power() :
 * do setup_io(), set_clock(), sensor power_enable()
 * if sink pad has videodevice, call source pad s_power()
 * video device streamon()
 * --> video_ops->s_stream() :
 * do sensor set_clock()
 * set_input_size(), set_output_format(), set_crop()
 */

/**
 * supported formats
 */
static const unsigned int supported_input_formats[] = {
    /* YUYV422 packed */
    V4L2_MBUS_FMT_YUYV8_2X8,
};

static const unsigned int supported_output_formats[] = {
    /* YUYV422 packed */
    V4L2_MBUS_FMT_YUYV8_2X8,
    /* YUYV420 3-plane */
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    /* YUYV422 3-plane */
    V4L2_MBUS_FMT_YUYV8_1X16,
    /* YUYV444 3-plane */
    V4L2_MBUS_FMT_YUV8_1X24,
};

/* TODO: duplicate */
static inline bool
_find_format(const unsigned int array[], int array_size, unsigned int code)
{
    int i;
    for (i = 0; i < array_size; i++) {
        if (code == array[i])
            return true;
    }
    return false;
}

static inline u32 _convert_to_nxp_vip_format(u32 v4l2_code)
{
    switch (v4l2_code) {
    case V4L2_MBUS_FMT_YUYV8_2X8:
        return NX_VIP_FORMAT_YUYV;
    case V4L2_MBUS_FMT_YUYV8_1_5X8:
        return NX_VIP_FORMAT_420;
    case V4L2_MBUS_FMT_YUYV8_1X16:
        return NX_VIP_FORMAT_422;
    case V4L2_MBUS_FMT_YUV8_1X24:
        return NX_VIP_FORMAT_444;
    default:
        pr_err("%s: invalid code 0x%x\n", __func__, v4l2_code);
        return NX_VIP_FORMAT_420;
    }
}

static struct v4l2_subdev *
_get_remote_source_subdev(struct nxp_vin_clipper *me)
{
    struct media_pad *pad
        = media_entity_remote_source(&me->pads[NXP_VIN_PAD_SINK]);
    if (!pad) {
        pr_err("%s: can't find remote source pad!!!\n", __func__);
        return NULL;
    }
    return media_entity_to_v4l2_subdev(pad->entity);
}

/**
 * hw specific functions
 */
static int _hw_set_clock(struct nxp_vin_clipper *me, bool on)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);

    vmsg("%s: module(%d) on(%d)\n", __func__, module, on);

#ifdef CONFIG_ARCH_NXP4330_3200
    if (on) {
        NX_VIP_SetClockPClkMode(module, NX_PCLKMODE_ALWAYS);
        NX_VIP_SetClockBClkMode(module, NX_BCLKMODE_DYNAMIC);
        NX_VIP_SetClockSource(module, 0, me->platdata->clock_invert ?
                NX_VIP_CLKSRC_INVICLKIN : NX_VIP_CLKSRC_ICLKIN);
        NX_VIP_SetClockDivisor(module, 0, 1);
        NX_VIP_SetClockDivisorEnable(module, CTRUE);
    } else {
        NX_VIP_SetClockPClkMode(module, NX_PCLKMODE_DYNAMIC);
        NX_VIP_SetClockBClkMode(module, NX_BCLKMODE_DISABLE);
        NX_VIP_SetClockDivisorEnable(module, CFALSE);
    }
#else
    if (on) {
        volatile u32 *clkgen_base = (volatile u32 *)IO_ADDRESS(NX_CLKGEN_GetPhysicalAddress(NX_VIP_GetClockNumber(module)));
        NX_CLKGEN_SetBaseAddress(NX_VIP_GetClockNumber(module), (void*)clkgen_base);
        NX_CLKGEN_SetClockDivisorEnable(NX_VIP_GetClockNumber(module), CTRUE);
        NX_CLKGEN_SetClockBClkMode(NX_VIP_GetClockNumber(module), NX_BCLKMODE_DYNAMIC);
#if defined(CONFIG_ARCH_S5P4418)
        NX_RSTCON_SetnRST(NX_VIP_GetResetNumber(module), RSTCON_nDISABLE);
        NX_RSTCON_SetnRST(NX_VIP_GetResetNumber(module), RSTCON_nENABLE);
#elif defined(CONFIG_ARCH_S5P6818)
        NX_RSTCON_SetRST(NX_VIP_GetResetNumber(module), RSTCON_ASSERT);
        NX_RSTCON_SetRST(NX_VIP_GetResetNumber(module), RSTCON_NEGATE);
#endif
        if (me->platdata->is_mipi) {
            vmsg("%s: apply mipi csi clock!!!\n", __func__);
            NX_CLKGEN_SetClockSource(NX_VIP_GetClockNumber(module), 0, 2); /* external PCLK */
            NX_CLKGEN_SetClockDivisor(NX_VIP_GetClockNumber(module), 0, 2);
            NX_CLKGEN_SetClockDivisorEnable(NX_VIP_GetClockNumber(module), CTRUE);
        } else {
            NX_CLKGEN_SetClockSource(NX_VIP_GetClockNumber(module), 0, 4 + me->platdata->port); /* external PCLK */
            NX_CLKGEN_SetClockDivisor(NX_VIP_GetClockNumber(module), 0, 1);
            NX_CLKGEN_SetClockDivisorEnable(NX_VIP_GetClockNumber(module), CTRUE);
            if (me->platdata->clock_invert)
                *clkgen_base |= 1 << 4;
        }

        vmsg("VIP CLK GEN VAL: 0x%x\n", *clkgen_base);
        NX_VIP_SetBaseAddress(module, (void*)IO_ADDRESS(NX_VIP_GetPhysicalAddress(module)));
    }
#endif

    return 0;
}

static int _hw_set_sensor_param(struct nxp_vin_clipper *me)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);
    struct nxp_vin_platformdata *info = me->platdata;

    vmsg("%s\n", __func__);
    if (info->is_mipi) {
        vmsg("%s: set mipi param\n", __func__);
        NX_VIP_SetInputPort(module, NX_VIP_INPUTPORT_B);
        NX_VIP_SetDataMode(module, NX_VIP_DATAORDER_CBY0CRY1, 16);
        /* NX_VIP_SetHVSyncForMIPI(module, */
        /*         info->h_active *2, */
        /*         info->v_active, */
        /*         info->h_syncwidth, */
        /*         info->h_frontporch, */
        /*         info->h_backporch, */
        /*         info->v_syncwidth, */
        /*         info->v_frontporch, */
        /*         info->v_backporch); */
        NX_VIP_SetFieldMode(module, CFALSE, NX_VIP_FIELDSEL_BYPASS, CFALSE, CFALSE);
        NX_VIP_SetDValidMode(module, CTRUE, CTRUE, CTRUE);
        NX_VIP_SetFIFOResetMode(module, NX_VIP_FIFORESET_ALL);
    } else {
        /* NX_VIP_SetHVSync(module, */
        /*         CFALSE, */
        /*         info->h_active*2, */
        /*         info->v_active, */
        /*         info->h_syncwidth*2, */
        /*         info->h_frontporch*2, */
        /*         info->h_backporch*2, */
        /*         info->v_syncwidth, */
        /*         info->v_frontporch, */
        /*         info->v_backporch); */
        NX_VIP_SetDataMode(module, info->data_order, 8);
        NX_VIP_SetFieldMode(module,
                CFALSE,
                0,
                info->interlace,
                CFALSE);
        NX_VIP_SetDValidMode(module,
                CFALSE,
                CFALSE,
                CFALSE);
        NX_VIP_SetFIFOResetMode(module, NX_VIP_FIFORESET_ALL);
        /* input port is PAD PORT */
        NX_VIP_SetInputPort(module, (NX_VIP_INPUTPORT)info->port);
    }

    return 0;
}

static int _hw_set_output_format(struct nxp_vin_clipper *me)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);
    struct v4l2_mbus_framefmt *mbus_fmt = &me->format[1];
    u32 nx_format;

    vmsg("%s\n", __func__);

    if (me->platdata->is_mipi) {
        /* BUGGY !!! */
        nx_format = NX_VIP_FORMAT_420;
        vmsg("%s: mipi set clipper format to 0x%x\n", __func__, nx_format);
    } else {
        switch (mbus_fmt->code) {
            case V4L2_MBUS_FMT_YUYV8_2X8:
                nx_format = NX_VIP_FORMAT_YUYV;
                break;
            case V4L2_MBUS_FMT_YUYV8_1_5X8:
                nx_format = NX_VIP_FORMAT_420;
                break;
            case V4L2_MBUS_FMT_YUYV8_1X16:
                nx_format = NX_VIP_FORMAT_422;
                break;
            case V4L2_MBUS_FMT_YUV8_1X24:
                nx_format = NX_VIP_FORMAT_444;
                break;
            default:
                pr_err("%s: invalid format(0x%x)\n", __func__, mbus_fmt->code);
                return -EINVAL;
        }
    }

#if defined(CONFIG_ARCH_S5P4418)
    NX_VIP_SetClipperFormat(module, nx_format, 0, 0, 0);
#elif defined(CONFIG_ARCH_S5P6818)
    NX_VIP_SetClipperFormat(module, nx_format);
#endif

    return 0;
}

static int _hw_set_input_size(struct nxp_vin_clipper *me)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);
    struct v4l2_mbus_framefmt *mbus_fmt = &me->format[1];
    struct nxp_vin_platformdata *info = me->platdata;

    vmsg("%s: w(%d), h(%d)\n", __func__, mbus_fmt->width, mbus_fmt->height);

    if (me->platdata->is_mipi) {
        vmsg("%s: mipi!!!\n", __func__);
        NX_VIP_SetHVSyncForMIPI(module,
                mbus_fmt->width * 2,
                mbus_fmt->height,
                info->h_syncwidth,
                info->h_frontporch,
                info->h_backporch,
                info->v_syncwidth,
                info->v_frontporch,
                info->v_backporch);
    } else{
        NX_VIP_SetHVSync(module,
                info->external_sync,
                mbus_fmt->width*2,
                /*mbus_fmt->height,*/
                info->interlace ? mbus_fmt->height >> 1 : mbus_fmt->height,
                info->h_syncwidth,
                info->h_frontporch,
                info->h_backporch,
                info->v_syncwidth,
                info->v_frontporch,
                info->v_backporch);
    }

    return 0;
}

static int _hw_set_crop(struct nxp_vin_clipper *me)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);
    struct v4l2_rect *c = &me->crop;
    struct nxp_vin_platformdata *info = me->platdata;

    vmsg("%s: l(%d), t(%d), w(%d), h(%d)\n", __func__, c->left, c->top, c->width, c->height);

    /*NX_VIP_SetClipRegion(module, c->left, c->top,*/
            /*c->left + c->width, c->top + c->height);*/
    NX_VIP_SetClipRegion(module, c->left, c->top,
            c->left + c->width, info->interlace ? (c->top + c->height) >> 1 : c->top + c->height);

    return 0;
}

/* psw0523 for debugging */
static u32 _lu_addr = 0;
static u32 _cb_addr = 0;
static u32 _cr_addr = 0;
static int _hw_set_addr(struct nxp_vin_clipper *me, struct nxp_video_buffer *buf)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);

    vmsg("%s: addr(0x%x)\n", __func__, buf->dma_addr[0]);

    /* for debugging */
    _lu_addr = buf->dma_addr[0];
    _cb_addr = buf->dma_addr[1];
    _cr_addr = buf->dma_addr[2];

    if (me->platdata->is_mipi) {
        struct v4l2_rect *c = &me->crop;
        vmsg("%s: set mipi vip format(0x%x), width %d, height %d, buf y 0x%x, buf cb 0x%x, buf cr 0x%x, stride 0 %d, stride 1 %d\n",
                __func__,
                NX_VIP_FORMAT_422,
                c->width - c->left,
                c->height - c->top,
                buf->dma_addr[0], buf->dma_addr[1], buf->dma_addr[2],
                buf->stride[0], buf->stride[1]);

        NX_VIP_SetClipperAddr(module, NX_VIP_FORMAT_420, c->width - c->left, c->height - c->top,
                buf->dma_addr[0], buf->dma_addr[1], buf->dma_addr[2],
                buf->stride[0], buf->stride[1]);
    } else {
        if (me->format[1].code == V4L2_MBUS_FMT_YUYV8_2X8) {
            vmsg("%s: clipper bufs 0x%x, stride %d\n",
                __func__, buf->dma_addr[0], buf->stride[0]);
            /* TODO: how can s5p6818 do this? */
#if defined(CONFIG_ARCH_S5P4418)
            NX_VIP_SetClipperAddrYUYV(module, buf->dma_addr[0], buf->stride[0]>>1);
#endif
        } else {
            u32 nx_format = _convert_to_nxp_vip_format(me->format[1].code);
            struct v4l2_rect *c = &me->crop;
            vmsg("%s: clipper bufs 0x%x, 0x%x, 0x%x, stride %d, %d,%d\n",
                __func__, buf->dma_addr[0], buf->dma_addr[1], buf->dma_addr[2],
                buf->stride[0], buf->stride[1], buf->stride[2]);
            NX_VIP_SetClipperAddr(module, nx_format, c->width, c->height,
                    buf->dma_addr[0], buf->dma_addr[1], buf->dma_addr[2],
                    buf->stride[0], buf->stride[1]);
        }
    }

    return 0;
}

static void _work_power_down(struct work_struct *work)
{
    struct nxp_vin_clipper *me = container_of(work, struct nxp_vin_clipper, work_power_down.work);
    struct v4l2_subdev *remote = _get_remote_source_subdev(me);

    vmsg("%s\n", __func__);
    if (remote && me->platdata->power_enable) {
        if (!(NXP_ATOMIC_READ(&me->state) & (NXP_VIN_STATE_RUNNING_DECIMATOR | NXP_VIN_STATE_RUNNING_CLIPPER))) {
            vmsg("power down!!!\n");
            me->platdata->power_enable(false);
        }
    }
}

static int _configure(struct nxp_vin_clipper *me, int enable)
{
    int ret = 0;
    struct v4l2_subdev *remote = _get_remote_source_subdev(me);

    if (!remote) {
        pr_err("%s: can't find remote source!!!\n", __func__);
        return -EINVAL;
    }

    ret = down_interruptible(&me->s_stream_sem);

    vmsg("%s: enable %d\n", __func__, enable);
    if (enable) {
        if (me->platdata->late_power_down)
            cancel_delayed_work_sync(&me->work_power_down);
        _hw_set_sensor_param(me);
        _hw_set_input_size(me);
        _hw_set_output_format(me);
        _hw_set_crop(me);

        if (me->platdata->power_enable)
            me->platdata->power_enable(true);
        if (me->platdata->set_clock)
            me->platdata->set_clock(me->platdata->clk_rate);

        if (me->platdata->power_state_changed) {
            if (me->platdata->power_state_changed())
                v4l2_subdev_call(remote, core, s_power, 0);
        }
        ret = v4l2_subdev_call(remote, video, s_stream, enable);
    } else {
        struct nxp_capture *parent = nxp_vin_to_parent(me);
        ret = v4l2_subdev_call(remote, video, s_stream, enable);

        /* flush fifo */
        vmsg("FIFO Reset\n");
        NX_VIP_SetFIFOResetMode(parent->get_module_num(parent), NX_VIP_FIFORESET_ALL);
        NX_VIP_ResetFIFO(parent->get_module_num(parent));

        vmsg("FIFO Statue: 0x%x\n", NX_VIP_GetFIFOStatus(parent->get_module_num(parent)));

        if (me->platdata->late_power_down)
            queue_delayed_work(system_nrt_wq, &me->work_power_down, msecs_to_jiffies(LATE_POWERDOWN_TIMEOUT_MS));

        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_STOPPING)
            NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_STOPPING, &me->state);
    }
    vmsg("%s exit\n", __func__);

    up(&me->s_stream_sem);

    return ret;
}

static int _done_buf(struct nxp_vin_clipper *me, bool updated);
static int _update_next_buffer(struct nxp_vin_clipper *me);
static void _unregister_irq_handler(struct nxp_vin_clipper *me);
static void _clear_buf(struct nxp_vin_clipper *me);

/**
 * call back functions
 */
#if 0
static irqreturn_t clipper_irq_handler(void *data)
{
    struct nxp_vin_clipper *me = data;

    if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_CLIPPER) {
        _done_buf(me, true);

        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_STOPPING) {
            struct nxp_capture *parent = nxp_vin_to_parent(me);
            vmsg("%s: real stop...\n", __func__);
            parent->stop(parent, me);
            _unregister_irq_handler(me);
            _clear_buf(me);
            complete(&me->stop_done);
        } else {
            _update_next_buffer(me);
        }
    }

    return IRQ_HANDLED;
}
#else
static uint32_t _irq_count = 0;
static irqreturn_t clipper_irq_handler(void *data)
{
    struct nxp_vin_clipper *me = data;

    if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_CLIPPER) {
        bool interlace = me->platdata->interlace;
        bool do_process = true;
        if (interlace) {
            _irq_count++;

            if (_irq_count == 2) {
                _irq_count = 0;
            } else {
                do_process = false;
            }
        }

				/* printk("%s - do_process : %d, irq_count : %d\n", __func__, do_process, _irq_count); */

        if (do_process) {
            _done_buf(me, true);

            if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_STOPPING) {
                struct nxp_capture *parent = nxp_vin_to_parent(me);
               // printk("%s: real stop...\n", __func__);
                parent->stop(parent, me);
                _unregister_irq_handler(me);
                _clear_buf(me);
                complete(&me->stop_done);
            } else {
                _update_next_buffer(me);
            }
        }
    }

    return IRQ_HANDLED;
}
#endif

static int clipper_buffer_queue(struct nxp_video_buffer *buf, void *me)
{
    unsigned long flags;
    struct nxp_vin_clipper *me2 = me;

    vmsg("%s: %p\n", __func__, buf);
    spin_lock_irqsave(&me2->slock, flags);
    list_add_tail(&buf->list, &me2->buffer_list);
    me2->buffer_count++;
    spin_unlock_irqrestore(&me2->slock, flags);
    return 0;
}

/**
 * internal functions
 */

static int _register_irq_handler(struct nxp_vin_clipper *me)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    if (!me->irq_entry) {
        me->irq_entry = kzalloc(sizeof(struct nxp_v4l2_irq_entry), GFP_KERNEL);
        if (!me->irq_entry) {
            pr_err("%s: failed to allocate irq_entry\n", __func__);
            return -ENOMEM;
        }
    }
    me->irq_entry->irqs = CAPTURE_CLIPPER_INT;
    me->irq_entry->priv = me;
    me->irq_entry->handler = clipper_irq_handler;
    return parent->register_irq_entry(parent, me->irq_entry);
}

static void _unregister_irq_handler(struct nxp_vin_clipper *me)
{
    if (me->irq_entry) {
        struct nxp_capture *parent = nxp_vin_to_parent(me);
        parent->unregister_irq_entry(parent, me->irq_entry);
        kfree(me->irq_entry);
        me->irq_entry = NULL;
    }
}

static int _register_buffer_consumer(struct nxp_vin_clipper *me)
{
    if (!me->consumer) {
        me->consumer = kzalloc(sizeof(struct nxp_buffer_consumer), GFP_KERNEL);
        if (!me->consumer) {
            pr_err("%s: failed to alloc consumer\n", __func__);
            return -ENOMEM;
        }
    }

    me->consumer->priv = me;
    me->consumer->queue = clipper_buffer_queue;
    return me->video->register_buffer_consumer(me->video,
            me->consumer, NXP_BUFFER_CONSUMER_SINK);
}

static void _unregister_buffer_consumer(struct nxp_vin_clipper *me)
{
    if (me->consumer) {
        me->video->unregister_buffer_consumer(me->video,
                me->consumer, NXP_BUFFER_CONSUMER_SINK);
        kfree(me->consumer);
        me->consumer = NULL;
    }
}

static int _handle_input_connection(struct nxp_vin_clipper *me, bool connected)
{
    if (connected) {
        /* init input parameters */
        me->format[NXP_VIN_PAD_SINK].code   = supported_input_formats[0];
        me->format[NXP_VIN_PAD_SINK].width  = me->platdata->h_active;
        me->format[NXP_VIN_PAD_SINK].height = me->platdata->v_active;
    }

    return 0;
}

static int _handle_video_connection(struct nxp_vin_clipper *me, bool connected)
{
    int ret = 0;

    if (connected) {
        /* register buffer consumer */
        ret = _register_buffer_consumer(me);
        if (ret < 0) {
            pr_err("%s: failed to _register_buffer_consumer\n", __func__);
            return ret;
        }
        /* init output parameters */
        if (!me->crop.width) {
            me->format[NXP_VIN_PAD_SOURCE_MEM].code = supported_output_formats[0];
            me->crop.left    = 0;
            me->crop.top     = 0;
            me->crop.width   = me->format[NXP_VIN_PAD_SINK].width;
            me->crop.height  = me->format[NXP_VIN_PAD_SINK].height;
        }
    } else {
        /* unregister buffer consumer */
        _unregister_buffer_consumer(me);
    }

    return ret;
}

static int _handle_decimator_connection(struct nxp_vin_clipper *me, bool connected)
{
    if (connected) {
        /* init output parameters */
        if (!me->crop.width) {
            me->format[NXP_VIN_PAD_SOURCE_DECIMATOR].code = supported_output_formats[0];
            me->crop.left    = 0;
            me->crop.top     = 0;
            me->crop.width   = me->format[NXP_VIN_PAD_SINK].width;
            me->crop.height  = me->format[NXP_VIN_PAD_SINK].height;
        }
    }

    /* what else to do? */
    return 0;
}

/**
 * called when s_power() off
 */
static void _disable_all(struct nxp_vin_clipper *me)
{
    if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_CLIPPER) {
        vmsg("%s: clipper video stopping...\n", __func__);
        NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_STOPPING, &me->state);
        if (!wait_for_completion_timeout(&me->stop_done, 2*HZ)) {
            struct nxp_capture *parent = nxp_vin_to_parent(me);
            vmsg("wait timeout for stopping\n");
            parent->stop(parent, me);
            _unregister_irq_handler(me);
            _clear_buf(me);
            NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_STOPPING, &me->state);
        }
        NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_RUNNING_CLIPPER, &me->state);
    }

    if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_DECIMATOR) {
        NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_RUNNING_DECIMATOR, &me->state);
    }

    _configure(me, 0);
}

/**
 * buffer operations
 */
static int _update_next_buffer(struct nxp_vin_clipper *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf;

    spin_lock_irqsave(&me->slock, flags);
    if (me->buffer_count == 0) {
        me->cur_buf = NULL;
        spin_unlock_irqrestore(&me->slock, flags);
        return 0;
    }
    buf = list_first_entry(&me->buffer_list, struct nxp_video_buffer, list);
    list_del_init(&buf->list);
    me->cur_buf = buf;
    me->buffer_count--;
    spin_unlock_irqrestore(&me->slock, flags);

    vmsg("%s add %p\n", __func__, buf);
    _hw_set_addr(me, buf);

#ifdef DEBUG_SYNC
    mod_timer(&me->timer, jiffies + msecs_to_jiffies(DEBUG_SYNC_TIMEOUT_MS));
#endif

    return 0;
}

#ifdef CONFIG_ARCH_NXP4330_3200
extern void nxp_decimator_simulation_run(struct nxp_video_buffer *src_buf);
#endif

static int _done_buf(struct nxp_vin_clipper *me, bool updated)
{
    unsigned long flags;
    struct nxp_video_buffer *buf;

    spin_lock_irqsave(&me->slock, flags);
    buf = me->cur_buf;
    me->cur_buf = NULL;
    spin_unlock_irqrestore(&me->slock, flags);

    if (!buf) {
        vmsg("%s: no current buf!!!\n", __func__);
        return 0;
    }

    if (updated)
        buf->consumer_index++;
    /* for decimator */
#ifdef CONFIG_ARCH_NXP4330_3200
    nxp_decimator_simulation_run(buf);
#endif
    if (buf->cb_buf_done)
        buf->cb_buf_done(buf);

    return 0;
}

static void _clear_buf(struct nxp_vin_clipper *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf = NULL;

    spin_lock_irqsave(&me->slock, flags);
    if (me->buffer_count > 0) {
        while (!list_empty(&me->buffer_list)) {
            //buf = list_first_entry(&me->buffer_list, struct nxp_video_buffer, list);
            buf = list_entry(me->buffer_list.next, struct nxp_video_buffer, list);
            if (buf) {
                /* buf->cb_buf_done(buf); */
                list_del_init(&buf->list);
            } else {
                break;
            }
        }
        INIT_LIST_HEAD(&me->buffer_list);
    }
    me->buffer_count = 0;
    me->cur_buf = NULL;
    spin_unlock_irqrestore(&me->slock, flags);
}

/**
 * utils used in subdev ops
 */
static struct v4l2_mbus_framefmt *
_get_pad_format(struct nxp_vin_clipper *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    /* use output pad format one */
    if (pad > NXP_VIN_PAD_SOURCE_MEM)
        pad = NXP_VIN_PAD_SOURCE_MEM;

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
_get_pad_crop(struct nxp_vin_clipper *me, struct v4l2_subdev_fh *fh,
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

/**
 * v4l2_subdev_internal_ops
 */
static int nxp_vin_clipper_open(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    /*
     * called when user app open subdev node
     */
    struct v4l2_mbus_framefmt *format;
    struct v4l2_rect *crop;
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    /* int ret; */

    vmsg("%s\n", __func__);

    /* first power on */
#if 0
    ret = v4l2_subdev_call(sd, core, s_power, 1);
    if (ret < 0) {
        pr_err("%s: failed to subdev s_power(), ret %d\n", __func__, ret);
        //return ret;
    }
#endif

    crop = v4l2_subdev_get_try_crop(fh, 0);
    if (crop) {
        crop->left = 0;
        crop->top  = 0;
        crop->width = me->platdata->h_active;
        crop->height = me->platdata->v_active;
    }

    /* input format */
    format = v4l2_subdev_get_try_format(fh, NXP_VIN_PAD_SINK);
    if (format) {
        format->code = supported_input_formats[0];
        format->width = me->platdata->h_active;
        format->height = me->platdata->v_active;
        format->field = V4L2_FIELD_NONE;
        format->colorspace = V4L2_COLORSPACE_SRGB;
    }

    /* output format : use only mem */
    format = v4l2_subdev_get_try_format(fh, NXP_VIN_PAD_SOURCE_MEM);
    if (format) {
        format->code = supported_output_formats[0];
        format->width = me->platdata->h_active;
        format->height = me->platdata->v_active;
        format->field = V4L2_FIELD_NONE;
        format->colorspace = V4L2_COLORSPACE_SRGB;
    }

    return 0;
}

static int nxp_vin_clipper_close(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    /*
     * called when user app close subdev node
     */
    vmsg("%s\n", __func__);

    /* power off */
#if 0
    return v4l2_subdev_call(sd, core, s_power, 0);
#else
    return 0;
#endif
}

static int nxp_vin_clipper_registered(struct v4l2_subdev *sd)
{
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    NXP_ATOMIC_SET(&me->state, NXP_VIN_STATE_REGISTERED);
    vmsg("%s\n", __func__);
    return 0;
}

static void nxp_vin_clipper_unregistered(struct v4l2_subdev *sd)
{
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    NXP_ATOMIC_SET(&me->state, NXP_VIN_STATE_INIT);
    vmsg("%s\n", __func__);
}

static const struct v4l2_subdev_internal_ops nxp_vin_clipper_internal_ops = {
    .open         = nxp_vin_clipper_open,
    .close        = nxp_vin_clipper_close,
    .registered   = nxp_vin_clipper_registered,
    .unregistered = nxp_vin_clipper_unregistered,
};

/**
 * v4l2_subdev_core_ops
 */
static int nxp_vin_clipper_s_power(struct v4l2_subdev *sd, int on)
{
    /*
     * called explicitly from user app or video device
     */
    struct nxp_vin_clipper *me;
    struct v4l2_subdev *remote_source;
    struct nxp_capture *parent;
    int ret = 0;
    u32 state;
    int module;

    vmsg("%s: %d\n", __func__, on);

	me = v4l2_get_subdevdata(sd);
	if (!me) {
        pr_err("%s error: me is NULL\n", __func__);
        return -EINVAL;
	}

    remote_source = _get_remote_source_subdev(me);
    if (!remote_source) {
        pr_err("%s error: remote is NULL\n", __func__);
        return -EINVAL;
    }

    parent = nxp_vin_to_parent(me);
    if (!parent) {
        pr_err("%s error: parent is NULL\n", __func__);
        return -EINVAL;
    }
    module = parent->get_module_num(parent);

    /* check state */
    state = NXP_ATOMIC_READ(&me->state);
    if (state < NXP_VIN_STATE_READY &&
        !(state & NXP_VIN_STATE_INPUT_CONNECTED)) {
        pr_err("%s error: invalid state(0x%x)\n", __func__, state);
        return -EINVAL;
    }

    if (on) {
        if (me->platdata->setup_io)
            me->platdata->setup_io(module, false);
        _hw_set_clock(me, true);
        ret = v4l2_subdev_call(remote_source, core, s_power, 1);
    } else {
        _disable_all(me);
        ret = v4l2_subdev_call(remote_source, core, s_power, 0);
        _hw_set_clock(me, false);
    }

    return ret;
}

static const struct v4l2_subdev_core_ops nxp_vin_clipper_core_ops = {
    .s_power = nxp_vin_clipper_s_power,
};

/**
 * v4l2_subdev_video_ops
 */
#define YUV_STRIDE_ALIGN_FACTOR     128
#define YUV_VSTRIDE_ALIGN_FACTOR    16
#define YUV_YSTRIDE(w)   ALIGN(w, YUV_STRIDE_ALIGN_FACTOR)
static int nxp_vin_clipper_s_stream(struct v4l2_subdev *sd, int enable)
{
    /*
     * called from video_device, when stream on/off
     */
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int ret = 0;
    char *hostname = (char *)v4l2_get_subdev_hostdata(sd);
    bool is_host_video = false;
    int module = parent->get_module_num(parent);

    if (!strncmp(hostname, "VIDEO", 5))
        is_host_video = true;

    vmsg("%s: state:0x%x, hostname: %s, en:%d\n", __func__, NXP_ATOMIC_READ(&me->state), hostname, enable);

    if (enable) {
         vmsg("lu_addr: %p, cb_addr: %p, cr_addr: %p\n", &_lu_addr, &_cb_addr, &_cr_addr);
        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_STOPPING) {
            int timeout = 50; // 5 second
            vmsg("wait clipper stopping...\n");
            while (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_STOPPING) {
                msleep(100);
                timeout--;
                if (timeout == 0) {
                    printk("wait clipper stopping timeout!!!\n");
                    break;
                }
            }
        }
        _configure(me, enable);
        if (is_host_video) {
            ret = _register_irq_handler(me);
            if (ret < 0) {
                pr_err("%s: failed to _register_irq_handler\n", __func__);
                return ret;
            }
            _update_next_buffer(me);
#if 0
            if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_DECIMATOR) {
                struct v4l2_rect *c = &me->crop;
                U32 src_width, src_height, dst_width, dst_height;
                NX_VIP_GetDeciSource(module, &src_width, &src_height);
                if (c->width != src_width || c->height != src_height) {
                    NX_VIP_GetDecimation(module, &dst_width, &dst_height, NULL, NULL, NULL, NULL);
                    if (dst_width > c->width || dst_height > c->height) {
                        pr_err("%s: Decimator wxh(%dx%d) is bigger than clipper(%dx%d)\n", __func__, dst_width, dst_height, c->width, c->height);
                        NX_VIP_SetDecimation(module, c->width, c->height, c->width, c->height);
                    } else {
                        vmsg("%s: SetDecimation: src(%dx%d), dst(%dx%d)\n", __func__, c->width, c->height, dst_width, dst_height);
                        if (dst_width > 0 && dst_height > 0)
                            NX_VIP_SetDecimation(module, c->width, c->height, dst_width, dst_height);
                    }
                }
            }
#else
            CBOOL vip_enable, deci_enable;
            NX_VIP_GetVIPEnable(module, &vip_enable, NULL, NULL, &deci_enable);
            if (vip_enable && deci_enable) {
                struct v4l2_rect *c = &me->crop;
                U32 src_width, src_height, dst_width, dst_height;
                NX_VIP_GetDeciSource(module, &src_width, &src_height);
                if (c->width != src_width || c->height != src_height) {
                    NX_VIP_GetDecimation(module, &dst_width, &dst_height, NULL, NULL, NULL, NULL);
                    if (dst_width > c->width || dst_height > c->height) {
                        int deci_width = dst_width > c->width ? c->width : dst_width;
                        int deci_height = dst_height > c->height ? c->height : dst_height;

                     	/*  printk("%s: Decimator wxh(%dx%d) is bigger than clipper(%dx%d)\n", __func__, dst_width, dst_height, c->width, c->height); */

                        /*while (!NX_VIP_GetInterruptPending(module, 1));*/
                        while (!NX_VIP_GetInterruptPendingAll(module));
                        /*printk("OD INT\n");*/
                        /* printk("ALL INT\n"); */
    #ifdef CONFIG_TURNAROUND_VIP_RESET
                        parent->backup_reset_restore_register(module);
    #endif
                        /*NX_VIP_SetDecimation(module, c->width, c->height, c->width, c->height);*/
                        /* printk("set decimation: %d,%d --> %d,%d\n", c->width, c->height, YUV_YSTRIDE(deci_width-127), deci_height); */
                        NX_VIP_SetDecimation(module, c->width, c->height, YUV_YSTRIDE(deci_width-127), deci_height);
    /*#ifdef CONFIG_TURNAROUND_VIP_RESET*/
                        /*NX_VIP_SetVIPEnable(module, CTRUE, CTRUE, CFALSE, CTRUE);*/
    /*#endif*/
                    } else {
                        vmsg("%s: SetDecimation: src(%dx%d), dst(%dx%d)\n", __func__, c->width, c->height, dst_width, dst_height);
                        if (dst_width > 0 && dst_height > 0)
                            NX_VIP_SetDecimation(module, c->width, c->height, dst_width, dst_height);
                    }
                }
            }
#endif
            parent->run(parent, me);
            NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_RUNNING_CLIPPER, &me->state);
        } else {
            NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_RUNNING_DECIMATOR, &me->state);
        }
        return 0;
    } else {
        if (is_host_video) {
            if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_CLIPPER) {
                vmsg("clipper video stopping...\n");
                NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_STOPPING, &me->state);
                if (!wait_for_completion_timeout(&me->stop_done, 2*HZ)) {
                    printk("wait timeout for clipper stopping\n");
                    parent->stop(parent, me);
                    _unregister_irq_handler(me);
                    _clear_buf(me);
                    NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_STOPPING, &me->state);
                    NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_RUNNING_CLIPPER, &me->state);
                }
                NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_RUNNING_CLIPPER, &me->state);
            }
        } else {
            NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_RUNNING_DECIMATOR, &me->state);
        }

        if (!(NXP_ATOMIC_READ(&me->state) & (NXP_VIN_STATE_RUNNING_CLIPPER | NXP_VIN_STATE_RUNNING_DECIMATOR))) {
            _configure(me, 0);
        } else if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_DECIMATOR) {
            NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_STOPPING, &me->state);
        }

        return 0;
    }

    pr_err("%s: invalid call(en:%d, state:0x%x, out:0x%x)\n", __func__,
            enable, NXP_ATOMIC_READ(&me->state), me->output_type);
    return -EINVAL;
}

static const struct v4l2_subdev_video_ops nxp_vin_clipper_video_ops = {
    .s_stream = nxp_vin_clipper_s_stream,
};

/**
 * v4l2_subdev_pad_ops
 */
static int nxp_vin_clipper_enum_mbus_code(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_mbus_code_enum *code)
{
    //struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);

    switch (code->pad) {
    case NXP_VIN_PAD_SINK:
        if (code->index != 0)
            return -EINVAL;

        code->code = supported_input_formats[code->index];
        break;

    case NXP_VIN_PAD_SOURCE_MEM:
    case NXP_VIN_PAD_SOURCE_DECIMATOR:
        if (code->index >= ARRAY_SIZE(supported_output_formats))
            return -EINVAL;

        code->code = supported_output_formats[code->index];
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static int nxp_vin_clipper_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);

    format->format = *_get_pad_format(me, fh, format->pad, format->which);

    return 0;
}

static int nxp_vin_clipper_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *__format =
        _get_pad_format(me, fh, format->pad, format->which);
    struct nxp_vin_platformdata *info = me->platdata;
    info->h_active = format->format.width;
    info->v_active = format->format.height;
    vmsg("%s: h_active(%d), v_active(%d), code(0x%x)\n", __func__, info->h_active, info->v_active, format->format.code);

    switch (format->pad) {
    case NXP_VIN_PAD_SINK:
        if (!_find_format(supported_input_formats,
                    ARRAY_SIZE(supported_input_formats), format->format.code)) {
            pr_err("%s: not supported input format(0x%x)\n", __func__,
                    format->format.code);
            return -EINVAL;
        }
        *__format = format->format;
        break;

    case NXP_VIN_PAD_SOURCE_MEM:
    case NXP_VIN_PAD_SOURCE_DECIMATOR:
        /**
         * use only code
         * width, height is set by set_crop
         */
        if (!_find_format(supported_output_formats,
                    ARRAY_SIZE(supported_output_formats), format->format.code)) {
            pr_err("%s: not supported output format(0x%x)\n", __func__,
                    format->format.code);
            return -EINVAL;
        }
        // psw0523 TODO
        //__format->code = format->format.code;
        *__format = format->format;
        break;

    default:
        return -EINVAL;
    }

    vmsg("%s: success!!!\n", __func__);
    return 0;
}

static int nxp_vin_clipper_set_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    /* set output w/h */
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    struct v4l2_rect *__crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    struct v4l2_mbus_framefmt *__format =
        _get_pad_format(me, fh, crop->pad, crop->which);

    vmsg("%s: %dx%d -- %dx%d\n",
           __func__, crop->rect.left, crop->rect.top, crop->rect.width, crop->rect.height);
    if ((crop->rect.left + crop->rect.width) > __format->width ||
        (crop->rect.top + crop->rect.height) > __format->height) {
        pr_err("%s: invalid crop rect(left %d, width %d, fwidth %d, top %d, height %d, fheight %d\n", __func__,
                crop->rect.left, crop->rect.width, __format->width,
                crop->rect.top, crop->rect.height, __format->height);
        return -EINVAL;
    }

    *__crop = crop->rect;

    /* add for source crop */
    if (crop->pad == 2)
        _hw_set_crop(me);
    /* end add for source crop */

    return 0;
}

static int nxp_vin_clipper_get_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    struct v4l2_rect *__crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    crop->rect = *__crop;
    return 0;
}

static const struct v4l2_subdev_pad_ops nxp_vin_clipper_pad_ops = {
    .enum_mbus_code = nxp_vin_clipper_enum_mbus_code,
    .get_fmt        = nxp_vin_clipper_get_fmt,
    .set_fmt        = nxp_vin_clipper_set_fmt,
    .set_crop       = nxp_vin_clipper_set_crop,
    .get_crop       = nxp_vin_clipper_get_crop,
};

/**
 * v4l2_subdev_ops
 */
static const struct v4l2_subdev_ops nxp_vin_clipper_subdev_ops = {
    .core  = &nxp_vin_clipper_core_ops,
    .video = &nxp_vin_clipper_video_ops,
    .pad   = &nxp_vin_clipper_pad_ops,
};

/**
 * media_entity_operations
 */
static int nxp_vin_clipper_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    struct nxp_vin_clipper *me = v4l2_get_subdevdata(sd);
    struct nxp_capture *parent = nxp_vin_to_parent(me);

    switch (local->index | media_entity_type(remote->entity)) {
    case NXP_VIN_PAD_SINK | MEDIA_ENT_T_V4L2_SUBDEV:
        vmsg("%s: link sensor to clipper %d, enable %d\n", __func__, local->index, flags & MEDIA_LNK_FL_ENABLED);
        /* link sensor to me */
        if (!(flags & MEDIA_LNK_FL_ENABLED)) {
            /* disconnect */
            if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_INPUT_CONNECTED) {
                me->input_type = NXP_VIN_INPUT_NONE;
                NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_INPUT_CONNECTED, &me->state);
                _handle_input_connection(me, false);
                break;
            } else {
                pr_err("%s: invalid disconnect input\n", __func__);
                return -EINVAL;
            }
        }

        /* connect */
        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_INPUT_CONNECTED) {
            pr_err("%s: already input occupied(%d)\n", __func__, me->input_type);
            return -EBUSY;
        }

        if (remote->entity == &parent->get_csi_subdev(parent)->entity) {
            me->input_type = NXP_VIN_INPUT_MIPI_CSI;
        } else {
            me->input_type = NXP_VIN_INPUT_PARALLEL;
        }
        NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_INPUT_CONNECTED, &me->state);

        vmsg("%s: connect clipper to sensor\n", __func__);
        _handle_input_connection(me, true);
        break;

    case NXP_VIN_PAD_SOURCE_MEM | MEDIA_ENT_T_DEVNODE:
        vmsg("%s: link video to clipper %d, enable %d\n", __func__, local->index, flags & MEDIA_LNK_FL_ENABLED);
        if (!(flags & MEDIA_LNK_FL_ENABLED)) {
            /* disconnect video device */
            if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_OUTPUT_CONNECTED &&
                me->output_type & NXP_VIN_OUTPUT_MEM) {
                me->output_type &= ~NXP_VIN_OUTPUT_MEM;
                if (me->output_type == NXP_VIN_OUTPUT_NONE)
                    NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_OUTPUT_CONNECTED, &me->state);

                _handle_video_connection(me, false);
                me->link_count--;
                break;
            } else {
                pr_err("%s: invalid disconnect output mem\n", __func__);
                return -EINVAL;
            }
        }

        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_OUTPUT_CONNECTED &&
            me->output_type & NXP_VIN_OUTPUT_MEM) {
            pr_err("%s: already video output set\n", __func__);
            return -EBUSY;
        }

        /* connect video device */
        me->output_type |= NXP_VIN_OUTPUT_MEM;
        NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_OUTPUT_CONNECTED, &me->state);

        _handle_video_connection(me, true);
        me->link_count++;
        break;

    case NXP_VIN_PAD_SOURCE_DECIMATOR | MEDIA_ENT_T_V4L2_SUBDEV:
        vmsg("%s: link decimator to clipper %d, enable %d\n", __func__, local->index, flags & MEDIA_LNK_FL_ENABLED);
        if (!(flags & MEDIA_LNK_FL_ENABLED)) {
            /* disconnect decimator device */
            if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_OUTPUT_CONNECTED &&
                me->output_type & NXP_VIN_OUTPUT_DECIMATOR) {
                me->output_type &= ~NXP_VIN_OUTPUT_DECIMATOR;
                if (me->output_type == NXP_VIN_OUTPUT_NONE)
                    NXP_ATOMIC_CLEAR_MASK(NXP_VIN_STATE_OUTPUT_CONNECTED, &me->state);

                _handle_decimator_connection(me, false);
                me->link_count--;
                break;
            } else {
                pr_err("%s: invalid disconnect output decimator\n", __func__);
                return -EINVAL;
            }
        }

        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_OUTPUT_CONNECTED &&
            me->output_type & NXP_VIN_OUTPUT_DECIMATOR) {
            pr_err("%s: already decimator output set\n", __func__);
            return -EBUSY;
        }

        /* connect decimator */
        me->output_type |= NXP_VIN_OUTPUT_DECIMATOR;
        NXP_ATOMIC_SET_MASK(NXP_VIN_STATE_OUTPUT_CONNECTED, &me->state);

        _handle_decimator_connection(me, true);
        me->link_count++;
        break;

    case NXP_VIN_PAD_SOURCE_MEM | MEDIA_ENT_T_V4L2_SUBDEV:
    case NXP_VIN_PAD_SOURCE_DECIMATOR | MEDIA_ENT_T_DEVNODE:
        /* invalid setting */
        pr_err("%s: invalid connection(%d-%d)\n",
                __func__, local->index, media_entity_type(remote->entity));
        return -EINVAL;
    }

    return 0;
}

static const struct media_entity_operations nxp_vin_clipper_media_ops = {
    .link_setup = nxp_vin_clipper_link_setup,
};

/*
 * _init_entities
 * do sw initialize
 * postpone hw initialization until state go to NXP_VIN_STATE_READY
 */
static int _init_entities(struct nxp_vin_clipper *me)
{
    struct v4l2_subdev *sd = &me->subdev;
    struct media_pad *pads = me->pads;
    struct media_entity *entity = &sd->entity;
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    char dev_name[NXP_VIDEO_MAX_NAME_SIZE] = {0, };
    int ret;

    me->input_type = NXP_VIN_INPUT_NONE;
    me->output_type = NXP_VIN_OUTPUT_NONE;

    v4l2_subdev_init(sd, &nxp_vin_clipper_subdev_ops);
    sd->internal_ops = &nxp_vin_clipper_internal_ops;
    /* entity name : used app media link setup */
#if 0
    snprintf(sd->name, sizeof(sd->name), "NXP VIN CLIPPER%d",
            parent->get_module_num(parent));
#else
    snprintf(sd->name, sizeof(sd->name), "NXP VIN CLIPPER%d",
            parent->get_index_num(parent));
#endif
    sd->grp_id = NXP_CAPTURE_SUBDEV_GROUP_ID;
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pads[NXP_VIN_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_VIN_PAD_SOURCE_MEM].flags = MEDIA_PAD_FL_SOURCE;
    pads[NXP_VIN_PAD_SOURCE_DECIMATOR].flags = MEDIA_PAD_FL_SOURCE;

    entity->ops = &nxp_vin_clipper_media_ops;
    ret = media_entity_init(entity, NXP_VIN_PAD_MAX, pads, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

#if 0
    snprintf(dev_name, sizeof(dev_name), "VIDEO CLIPPER%d",
            parent->get_module_num(parent));
#else
    snprintf(dev_name, sizeof(dev_name), "VIDEO CLIPPER%d",
            parent->get_index_num(parent));
#endif
    me->video = create_nxp_video(dev_name, NXP_VIDEO_TYPE_CAPTURE,
            parent->get_v4l2_device(parent), parent->get_alloc_ctx(parent));
    if (!me->video) {
        pr_err("%s: failed to create_nxp_video()\n", __func__);
        ret = -EINVAL;
        goto error_video;
    }

    /* create link subdev to video node */
    ret = media_entity_create_link(entity, NXP_VIN_PAD_SOURCE_MEM,
            &me->video->vdev.entity, 0, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_create_link()\n", __func__);
        goto error_link;
    }

    return 0;

error_link:
    release_nxp_video(me->video);
    me->video = NULL;
error_video:
    media_entity_cleanup(entity);
    return ret;
}

/**
 * public api
 */
int nxp_vin_clipper_init(struct nxp_vin_clipper *me,
        struct nxp_vin_platformdata *platdata)
{
    int ret = _init_entities(me);

    if (likely(!ret)) {
        NXP_ATOMIC_SET(&me->state, NXP_VIN_STATE_INIT);
        INIT_LIST_HEAD(&me->buffer_list);
        me->platdata = platdata;
        spin_lock_init(&me->slock);
        me->buffer_count = 0;
        me->link_count = 0;
        me->enable_count = 0;
#ifdef DEBUG_SYNC
        setup_timer(&me->timer, debug_sync, (long)me);
#endif
        INIT_DELAYED_WORK(&me->work_power_down, _work_power_down);
        init_completion(&me->stop_done);
        sema_init(&me->s_stream_sem, 1);
    }

    vmsg("%s: ret(%d)\n", __func__, ret);
    return ret;
}

/*
 * must be called when NXP_VIN_STATE_INIT
 */
void nxp_vin_clipper_cleanup(struct nxp_vin_clipper *me)
{
    vmsg("%s\n", __func__);
    NXP_ATOMIC_SET(&me->state, NXP_VIN_STATE_UNINIT);
    release_nxp_video(me->video);
    me->video = NULL;
}

int nxp_vin_clipper_register(struct nxp_vin_clipper *me)
{
    int ret;
    struct nxp_capture *parent = nxp_vin_to_parent(me);

    vmsg("%s\n", __func__);

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

    return ret;
}

void nxp_vin_clipper_unregister(struct nxp_vin_clipper *me)
{
    vmsg("%s\n", __func__);
    if (NXP_ATOMIC_READ(&me->state) == NXP_VIN_STATE_REGISTERED) {
        unregister_nxp_video(me->video);
        v4l2_device_unregister_subdev(&me->subdev);
    } else {
        pr_err("%s: invalid state(0x%x)\n", __func__, NXP_ATOMIC_READ(&me->state));
    }
}

#ifdef CONFIG_PM
int nxp_vin_clipper_suspend(struct nxp_vin_clipper *me)
{
    struct v4l2_subdev *remote_source;
    PM_DBGOUT("+%s\n", __func__);
    if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_CLIPPER) {
        unsigned long flags;
        spin_lock_irqsave(&me->slock, flags);
        if (me->cur_buf == NULL) {
            PM_DBGOUT("%s: clipper current buffer is NULL\n", __func__);
            spin_unlock_irqrestore(&me->slock, flags);
            return -ENOENT;
        }
        spin_unlock_irqrestore(&me->slock, flags);

    }

    remote_source = _get_remote_source_subdev(me);
    if (remote_source) {
        v4l2_subdev_call(remote_source, core, s_power, 0);
        if (me->platdata->power_enable)
            me->platdata->power_enable(false);
    }
    PM_DBGOUT("-%s\n", __func__);

    return 0;
}

int nxp_vin_clipper_resume(struct nxp_vin_clipper *me)
{
    struct nxp_capture *parent = nxp_vin_to_parent(me);
    int module = parent->get_module_num(parent);
    /*struct v4l2_subdev *remote_source;*/

    PM_DBGOUT("+%s\n", __func__);

#if 0
    remote_source = _get_remote_source_subdev(me);
    if (remote_source) {
        /* struct v4l2_subdev_format format; */
        if (me->platdata->setup_io)
            me->platdata->setup_io(module, true);
        _hw_set_clock(me, true);
        _hw_set_sensor_param(me);
    }
#else
    if (me->platdata->setup_io)
        me->platdata->setup_io(module, true);
    _hw_set_clock(me, true);
#endif

    if (NXP_ATOMIC_READ(&me->state) & (NXP_VIN_STATE_RUNNING_DECIMATOR | NXP_VIN_STATE_RUNNING_CLIPPER)) {
        PM_DBGOUT("%s: reconfigure\n", __func__);
        _configure(me, 1);
        if (NXP_ATOMIC_READ(&me->state) & NXP_VIN_STATE_RUNNING_CLIPPER) {
            PM_DBGOUT("%s: reset clipper address\n", __func__);
            _hw_set_addr(me, me->cur_buf);
            /* parent->run(parent, me); */
        }
    }
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif
