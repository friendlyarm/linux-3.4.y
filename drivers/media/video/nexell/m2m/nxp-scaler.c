/*#define DEBUG 1*/

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/v4l2-mediabus.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#ifdef CONFIG_ENABLE_SCALER_MISC_DEVICE
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>

#include <mach/nxp-scaler.h>

extern struct ion_device *get_global_ion_device(void);

/* #define DEBUG_IOCTL_DATA */
#endif

#include "../nxp-v4l2-common.h"
#include "../nxp-video.h"
#include "../nxp-v4l2.h"

/* mach */
#include <mach/platform.h>
#include <mach/soc.h>

/* prototype */
#include <nx_scaler.h>
#include <nx_rstcon.h>
#include <nx_clkgen.h>

#include "scaler-filter-preset.h"
#include "nxp-scaler.h"

#ifdef SIMULATION_SCALER
#include <linux/timer.h>
#define DEFAULT_SIMULATION_ISR_TIMEOUT_MS (30)
#endif

#if 0
#define COMMAND_BUFFER_SIZE (4*14*3)
#else
#define COMMAND_BUFFER_SIZE  PAGE_SIZE
#endif

static struct nxp_scaler *_scaler = NULL;

/**
 * supported formats
 */
static const unsigned int supported_formats[] = {
    /* YUYV420 3-plane */
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    /* YUYV422 3-plane */
    V4L2_MBUS_FMT_YUYV8_1X16,
    /* YUYV444 3-plane */
    V4L2_MBUS_FMT_YUV8_1X24,
};

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

#if 0
static struct video_device *
_get_remote_source_videodev(struct nxp_scaler *me)
{
    struct media_pad *pad;
    struct video_device *remote;

    pad = media_entity_remote_source(&me->pads[NXP_SCALER_PAD_SINK]);
    if (!pad) {
        pr_err("%s: can't find remote source pad!!!\n", __func__);
        return NULL;
    }
    remote = media_entity_to_video_device(pad->entity);
    return remote;
}
#endif

/**
 * hw specific functions
 */
#ifdef SIMULATION_SCALER
static void simulation_isr(unsigned long priv);

static int simulation_process(struct nxp_scaler *me,
        struct vb2_buffer *in_vb,
        struct vb2_buffer *out_vb)
{
    /* src format == dst format */
    /* only yuyv packed type */
    u8 *p_in, *p_out;
    u32 width, height, stride;
    int y;

    vmsg("%s\n", __func__);

    p_in = vb2_plane_vaddr(in_vb, 0);
    p_out = vb2_plane_vaddr(out_vb, 0);

    if (!p_in) {
        pr_err("%s: invalid in_vb\n", __func__);
        return -ENOMEM;
    }

    if (!p_out) {
        pr_err("%s: invalid out_vb\n", __func__);
        return -ENOMEM;
    }

    width = me->format[0].width;
    height = me->format[0].height;
    stride = width * 2;

    for (y = 0; y < height; y++) {
        memcpy(p_out, p_in, stride);
        p_in += stride;
        p_out += stride;
    }

    return 0;
}

#else
#ifndef CONFIG_ENABLE_SCALER_MISC_DEVICE
static irqreturn_t _irq_handler(int irq, void *param);
#endif
#endif

static inline void _hw_set_filter_table(struct nxp_scaler *me, struct filter_table *table)
{
    int i, j;

#ifndef SIMULATION_SCALER
    for (i = 0; i < 3; i++)
        for (j = 0; j < 8; j++)
            NX_SCALER_SetYVFilter(0, i, j,
                    MAKE_YVFILTER(table->yvfilter[i][j][0],
                                  table->yvfilter[i][j][1],
                                  table->yvfilter[i][j][2],
                                  table->yvfilter[i][j][3]));

    for (i = 0; i < 5; i++)
        for (j = 0; j < 32; j++)
            NX_SCALER_SetYHFilter(0, i, j,
                    MAKE_YHFILTER(table->yhfilter[i][j][0],
                                  table->yhfilter[i][j][1]));
#endif
}


static int _hw_init(struct nxp_scaler *me)
{
    int ret = 0;

    vmsg("%s\n", __func__);

#ifdef SIMULATION_SCALER
    setup_timer(&me->timer, simulation_isr, (long)me);
    me->timeout = DEFAULT_SIMULATION_ISR_TIMEOUT_MS;
#else
    NX_SCALER_Initialize();
    vmsg("%s: SCALER REGISTER PHY 0x%x, VIR 0x%x\n", __func__, NX_SCALER_GetPhysicalAddress(0),
            (U32)IO_ADDRESS(NX_SCALER_GetPhysicalAddress(0)));
    NX_SCALER_SetBaseAddress(0, (void*)IO_ADDRESS(NX_SCALER_GetPhysicalAddress(0)));

    /* NX_SCALER_OpenModule(0); */

    me->irq = NX_SCALER_GetInterruptNumber(0);
#ifdef CONFIG_ARCH_S5P6818
    me->irq += 32;
#endif

    NX_CLKGEN_SetBaseAddress(NX_SCALER_GetClockNumber(0), (void*)IO_ADDRESS(NX_CLKGEN_GetPhysicalAddress(NX_SCALER_GetClockNumber(0))));
    NX_CLKGEN_SetClockBClkMode(NX_SCALER_GetClockNumber(0), NX_BCLKMODE_ALWAYS);

    #if defined(CONFIG_ARCH_S5P4418)
    NX_RSTCON_SetnRST(NX_SCALER_GetResetNumber(0), RSTCON_nENABLE);
    #elif defined(CONFIG_ARCH_S5P6818)
    NX_RSTCON_SetRST(NX_SCALER_GetResetNumber(0), RSTCON_NEGATE);
    #endif

    NX_SCALER_SetInterruptEnableAll(0, CFALSE);
    NX_SCALER_ClearInterruptPendingAll(0);

    NX_SCALER_SetFilterEnable(0, CTRUE);
    NX_SCALER_SetMode(0, 0);
#endif

    return ret;
}

static void _hw_cleanup(struct nxp_scaler *me)
{
    vmsg("%s\n", __func__);

#ifdef SIMULATION_SCALER
    del_timer_sync(&me->timer);
#else
    if (me->irq_alive) {
        me->irq_alive = false;
        free_irq(me->irq, me);
    }

    NX_SCALER_Stop(0);
    NX_SCALER_SetInterruptEnableAll(0, CFALSE);
    NX_SCALER_ClearInterruptPendingAll(0);
    NX_CLKGEN_SetClockBClkMode(NX_SCALER_GetClockNumber(0), NX_BCLKMODE_DISABLE);
#endif
}

/*
 * parameters must be checked before called
 */
static int _hw_set_format(struct nxp_scaler *me)
{
    vmsg("%s\n", __func__);

#ifndef SIMULATION_SCALER
    NX_SCALER_SetFilterEnable(0, CTRUE);
    //NX_SCALER_SetRotateMode(NX_SCALER_ROTATE_0);
    /* TODO: format */
#endif

    return 0;
}

#ifndef SIMULATION_SCALER
static inline void _make_cmd(unsigned int *c)
{
    unsigned int command = 0;
    /* copy size */
    command |= (11-1) << 0; // word
    /* start source address register : SCSRCADDRREG */
    command |= 3 << (10+0); // word
    *c = command;
    vmsg("%s: command: 0x%x\n", __func__, command);
}

#if 0
static int _make_command_buffer(struct nxp_scaler *me)
{
    unsigned int *cmd_buffer;
    unsigned int src_width, src_height, src_code, dst_width, dst_height, dst_code;
    unsigned int cb_src_width, cb_src_height, cb_dst_width, cb_dst_height;

    if (!me->command_buffer_vir) {
        me->command_buffer_vir = dma_alloc_coherent(NULL, COMMAND_BUFFER_SIZE, &me->command_buffer_phy, GFP_KERNEL);
        if (!me->command_buffer_vir) {
            pr_err("%s: failed to allocate command buffer!!!\n", __func__);
            return -ENOMEM;
        }
    }

    src_width = me->format[NXP_SCALER_PAD_SINK].width;
    src_height = me->format[NXP_SCALER_PAD_SINK].height;
    src_code = me->format[NXP_SCALER_PAD_SINK].code;
    dst_width = me->format[NXP_SCALER_PAD_SOURCE].width;
    dst_height = me->format[NXP_SCALER_PAD_SOURCE].height;
    dst_code = me->format[NXP_SCALER_PAD_SOURCE].code;

    cmd_buffer = me->command_buffer_vir;

    /* Y command buffer */
    /* header */
    _make_cmd(cmd_buffer); cmd_buffer++;

    /* Source Address Register */
    *cmd_buffer = me->cur_src_buf->dma_addr[0]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = me->cur_src_buf->stride[0]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((src_height - 1) << 16) | (src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = me->cur_dst_buf->dma_addr[0]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = me->cur_dst_buf->stride[0]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((dst_height - 1) << 16) | (dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (src_width << 16) / (dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (src_height << 16) / (dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

#if 0
    /* run command */
    *cmd_buffer = 1 << 29; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
#else
    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;
#endif

    /* CB command buffer */
    if (src_code == V4L2_MBUS_FMT_YUYV8_1_5X8) {
        /* 420 */
        cb_src_width = src_width >> 1;
        cb_src_height = src_height >> 1;
    } else if (src_code == V4L2_MBUS_FMT_YUYV8_1X16) {
        /* 422 */
        cb_src_width = src_width >> 1;
        cb_src_height = src_height;
    } else {
        cb_src_width = src_width;
        cb_src_height = src_height;
    }
    if (dst_code == V4L2_MBUS_FMT_YUYV8_1_5X8) {
        /* 420 */
        cb_dst_width = dst_width >> 1;
        cb_dst_height = dst_height >> 1;
    } else if (dst_code == V4L2_MBUS_FMT_YUYV8_1X16) {
        /* 422 */
        cb_dst_width = dst_width >> 1;
        cb_dst_height = dst_height;
    } else {
        cb_dst_width = dst_width;
        cb_dst_height = dst_height;
    }

    _make_cmd(cmd_buffer); cmd_buffer++;
    /* Source Address Register */
    *cmd_buffer = me->cur_src_buf->dma_addr[1]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = me->cur_src_buf->stride[1]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((cb_src_height - 1) << 16) | (cb_src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = me->cur_dst_buf->dma_addr[1]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = me->cur_dst_buf->stride[1]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((cb_dst_height - 1) << 16) | (cb_dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (cb_src_width << 16) / (cb_dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (cb_src_height << 16) / (cb_dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

#if 0
    /* run command */
    *cmd_buffer = 1 << 29; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
#else
    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;
#endif

    /* CR command buffer */
    _make_cmd(cmd_buffer); cmd_buffer++;
    /* Source Address Register */
    *cmd_buffer = me->cur_src_buf->dma_addr[2]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = me->cur_src_buf->stride[2]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((cb_src_height - 1) << 16) | (cb_src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = me->cur_dst_buf->dma_addr[2]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = me->cur_dst_buf->stride[2]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((cb_dst_height - 1) << 16) | (cb_dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (cb_src_width << 16) / (cb_dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (cb_src_height << 16) / (cb_dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

#if 0
    /* run command : set last */
    *cmd_buffer = (1 << 29) | (1 << 27); cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
#else
    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 27) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;
#endif

    return 0;
}
#else
static int _make_command_buffer(struct nxp_scaler *me, struct nxp_video_buffer *src_buf, struct nxp_video_buffer *dst_buf)
{
    unsigned int *cmd_buffer;
    unsigned int src_width, src_height, src_code, dst_width, dst_height, dst_code;
    unsigned int cb_src_width, cb_src_height, cb_dst_width, cb_dst_height;

    if (!me->command_buffer_vir) {
        me->command_buffer_vir = dma_alloc_coherent(NULL, COMMAND_BUFFER_SIZE, &me->command_buffer_phy, GFP_KERNEL);
        if (!me->command_buffer_vir) {
            pr_err("%s: failed to allocate command buffer!!!\n", __func__);
            return -ENOMEM;
        }
    }

    src_width = me->format[NXP_SCALER_PAD_SINK].width;
    src_height = me->format[NXP_SCALER_PAD_SINK].height;
    src_code = me->format[NXP_SCALER_PAD_SINK].code;
    dst_width = me->format[NXP_SCALER_PAD_SOURCE].width;
    dst_height = me->format[NXP_SCALER_PAD_SOURCE].height;
    dst_code = me->format[NXP_SCALER_PAD_SOURCE].code;

    cmd_buffer = me->command_buffer_vir;

    /* Y command buffer */
    /* header */
    _make_cmd(cmd_buffer); cmd_buffer++;

    /* Source Address Register */
    *cmd_buffer = src_buf->dma_addr[0]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = src_buf->stride[0]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((src_height - 1) << 16) | (src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = dst_buf->dma_addr[0]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = dst_buf->stride[0]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((dst_height - 1) << 16) | (dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (src_width << 16) / (dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (src_height << 16) / (dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;

    /* CB command buffer */
    if (src_code == V4L2_MBUS_FMT_YUYV8_1_5X8) {
        /* 420 */
        cb_src_width = src_width >> 1;
        cb_src_height = src_height >> 1;
    } else if (src_code == V4L2_MBUS_FMT_YUYV8_1X16) {
        /* 422 */
        cb_src_width = src_width >> 1;
        cb_src_height = src_height;
    } else {
        cb_src_width = src_width;
        cb_src_height = src_height;
    }
    if (dst_code == V4L2_MBUS_FMT_YUYV8_1_5X8) {
        /* 420 */
        cb_dst_width = dst_width >> 1;
        cb_dst_height = dst_height >> 1;
    } else if (dst_code == V4L2_MBUS_FMT_YUYV8_1X16) {
        /* 422 */
        cb_dst_width = dst_width >> 1;
        cb_dst_height = dst_height;
    } else {
        cb_dst_width = dst_width;
        cb_dst_height = dst_height;
    }

    _make_cmd(cmd_buffer); cmd_buffer++;
    /* Source Address Register */
    *cmd_buffer = src_buf->dma_addr[1]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = src_buf->stride[1]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((cb_src_height - 1) << 16) | (cb_src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = dst_buf->dma_addr[1]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = dst_buf->stride[1]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((cb_dst_height - 1) << 16) | (cb_dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (cb_src_width << 16) / (cb_dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (cb_src_height << 16) / (cb_dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;

    /* CR command buffer */
    _make_cmd(cmd_buffer); cmd_buffer++;
    /* Source Address Register */
    *cmd_buffer = src_buf->dma_addr[2]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = src_buf->stride[2]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((cb_src_height - 1) << 16) | (cb_src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = dst_buf->dma_addr[2]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = dst_buf->stride[2]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((cb_dst_height - 1) << 16) | (cb_dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (cb_src_width << 16) / (cb_dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (cb_src_height << 16) / (cb_dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 27) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;

    return 0;
}
#endif

#endif

// for debugging
/* static int s_run_count = 0; */
static int _hw_run(struct nxp_scaler *me)
{
    unsigned long flags;
    struct nxp_video_buffer *src_buf, *dst_buf;

    spin_lock_irqsave(&me->slock, flags);
    src_buf = list_first_entry(&me->src_buffer_list, struct nxp_video_buffer, list);
    list_del_init(&src_buf->list);
    spin_unlock_irqrestore(&me->slock, flags);

    spin_lock_irqsave(&me->dlock, flags);
    dst_buf = list_first_entry(&me->dst_buffer_list, struct nxp_video_buffer, list);
    list_del_init(&dst_buf->list);
    spin_unlock_irqrestore(&me->dlock, flags);

    vmsg("%s: src_addr(0x%x), dst_addr(0x%x)\n",
            __func__, src_buf->dma_addr[0], dst_buf->dma_addr[0]);

    me->cur_src_buf = src_buf;
    me->cur_dst_buf = dst_buf;

#ifdef SIMULATION_SCALER
    /* int ret = simulation_process(me, */
    /*         me->cur_src_buf->priv, me->cur_dst_buf->priv); */
    int ret = simulation_process(me,
            src_buf->priv, dst_buf->priv);
    if (ret < 0) {
        pr_err("%s: failed to simulation_process()\n", __func__);
        return ret;
    }
    mod_timer(&me->timer, jiffies + msecs_to_jiffies(me->timeout));
#else
    /* if (s_run_count < 1) { */
    vmsg("SCALER HW RUN!!!\n");
    _make_command_buffer(me, src_buf, dst_buf);
    vmsg("command buffer phys: 0x%x\n", me->command_buffer_phy);
    NX_SCALER_SetCmdBufAddr(0, me->command_buffer_phy);
    NX_SCALER_SetInterruptEnable(0, NX_SCALER_INT_CMD_PROC, CTRUE);
    NX_SCALER_SetMode(0, 0);
    NX_SCALER_RunCmdBuf(0);
    /* s_run_count++; */
    /* } */
#endif

    return 0;
}

/**
 * buffer operations
 */
static int _update_next_buffer(struct nxp_scaler *me)
{
    unsigned long flags;
    spin_lock_irqsave(&me->slock, flags);
    if (list_empty(&me->src_buffer_list)) {
        spin_unlock_irqrestore(&me->slock, flags);
        pr_err("%s: src buf not ready!!!\n", __func__);
        return 0;
    }
    spin_unlock_irqrestore(&me->slock, flags);

    spin_lock_irqsave(&me->dlock, flags);
    if (list_empty(&me->dst_buffer_list)) {
        spin_unlock_irqrestore(&me->dlock, flags);
        pr_err("%s: dst buf not ready!!!\n", __func__);
        return 0;
    }
    spin_unlock_irqrestore(&me->dlock, flags);

    return _hw_run(me);
}

#ifndef CONFIG_ENABLE_SCALER_MISC_DEVICE
static int _done_buf(struct nxp_scaler *me, bool updated)
{
    unsigned long flags;
    struct nxp_video_buffer *buf;

    vmsg("%s\n", __func__);

    /* src buf */
    spin_lock_irqsave(&me->slock, flags);
    buf = me->cur_src_buf;
    me->cur_src_buf = NULL;
    spin_unlock_irqrestore(&me->slock, flags);

    if (updated)
        buf->consumer_index++;
    buf->cb_buf_done(buf);

    /* dst buf */
    spin_lock_irqsave(&me->dlock, flags);
    buf = me->cur_dst_buf;
    me->cur_dst_buf = NULL;
    spin_unlock_irqrestore(&me->dlock, flags);

    if (updated)
        buf->consumer_index++;
    buf->cb_buf_done(buf);

    return 0;
}
#endif

static void _clear_buf(struct nxp_scaler *me)
{
    unsigned long flags;
    struct nxp_video_buffer *buf = NULL;

    /* src buf */
    spin_lock_irqsave(&me->slock, flags);
    while (!list_empty(&me->src_buffer_list)) {
        buf = list_entry(me->src_buffer_list.next,
                struct nxp_video_buffer, list);
        list_del_init(&buf->list);
    }
    INIT_LIST_HEAD(&me->src_buffer_list);
    me->cur_src_buf = NULL;
    spin_unlock_irqrestore(&me->slock, flags);

    /* dst buf */
    spin_lock_irqsave(&me->dlock, flags);
    while (!list_empty(&me->dst_buffer_list)) {
        buf = list_entry(me->dst_buffer_list.next,
                struct nxp_video_buffer, list);
        list_del_init(&buf->list);
    }
    INIT_LIST_HEAD(&me->dst_buffer_list);
    me->cur_dst_buf = NULL;
    spin_unlock_irqrestore(&me->dlock, flags);
}

#ifdef SIMULATION_SCALER
static void simulation_isr(unsigned long priv)
{
    struct nxp_scaler *me = (struct nxp_scaler *)priv;

    vmsg("%s\n", __func__);

    if (me)
        _done_buf(me, true);
    /* TODO ?  */
    //_update_next_buffer(me);
}
#else
#ifndef CONFIG_ENABLE_SCALER_MISC_DEVICE
static irqreturn_t _irq_handler(int irq, void *param)
{
    struct nxp_scaler *me = (struct nxp_scaler *)param;
    vmsg("%s\n", __func__);
    _done_buf(me, true);
    NX_SCALER_ClearInterruptPending(0, NX_SCALER_INT_CMD_PROC);
    _update_next_buffer(me);
    return IRQ_HANDLED;
}
#endif
#endif

static int scaler_buffer_queue(struct nxp_video_buffer *buf, void *me)
{
    unsigned long flags;
    struct nxp_scaler *me2 = me;
    struct list_head *plist;
    spinlock_t *plock;
    struct vb2_buffer *vb;
    u32 type;

    vmsg("%s: %p\n", __func__, buf);
    vb = buf->priv;
    type = vb->vb2_queue->type;

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        plist = &me2->dst_buffer_list;
        plock = &me2->dlock;
    } else if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        plist = &me2->src_buffer_list;
        plock = &me2->slock;
    } else {
        pr_err("%s: invalid type(0x%x)\n", __func__, type);
        return -EINVAL;
    }

    spin_lock_irqsave(plock, flags);
    list_add_tail(&buf->list, plist);
    spin_unlock_irqrestore(plock, flags);

    /* TODO : check state */
    if (me2->streaming) {
#if 0
        struct nxp_video_buffer *src_buf, *dst_buf;
        spin_lock_irqsave(&me2->slock, flags);
        src_buf = me2->cur_src_buf;
        spin_unlock_irqrestore(&me2->slock, flags);
        spin_lock_irqsave(&me2->dlock, flags);
        dst_buf = me2->cur_dst_buf;
        spin_unlock_irqrestore(&me2->dlock, flags);
        /* BUG here ? */
        /* if (!src_buf && !dst_buf) */
        if (!src_buf || !dst_buf)
            _update_next_buffer(me);
#else
        _update_next_buffer(me);
#endif
    }

    return 0;
}

static int
_register_buffer_consumer(struct nxp_scaler *me,
        struct nxp_video *video, u32 type)
{
    int ret;

    if (!me->consumer) {
        me->consumer = kzalloc(sizeof(struct nxp_buffer_consumer), GFP_KERNEL);
        if (!me->consumer) {
            pr_err("%s: failed to alloc consumer\n", __func__);
            return -ENOMEM;
        }
    }

    me->consumer->priv = me;
    me->consumer->queue = scaler_buffer_queue;
    ret = video->register_buffer_consumer(video,
            me->consumer, type);
    if (ret < 0) {
        pr_err("%s: failed to video->register_buffer_consumer()\n", __func__);
        return ret;
    }

    me->consumer->usage_count++;
    return 0;
}

static void
_unregister_buffer_consumer(struct nxp_scaler *me,
        struct nxp_video *video, u32 type)
{
    if (me->consumer) {
        if (me->consumer->usage_count > 0) {
            video->unregister_buffer_consumer(video,
                    me->consumer, type);
            me->consumer->usage_count--;
        }
        if (me->consumer->usage_count <= 0) {
            kfree(me->consumer);
            me->consumer = NULL;
        }
    }
}

static int _handle_input_connection(struct nxp_scaler *me,
        struct nxp_video *remote, bool connected)
{
    u32 type; /* NXP_BUFFER_CONSUMER_SOURCE,
                 NXP_BUFFER_CONSUMER_SINK */

    if (remote->type == NXP_VIDEO_TYPE_CAPTURE) {
        /* connect to capture pad : sink */
        type = NXP_BUFFER_CONSUMER_SINK;
    } else if (remote->type == NXP_VIDEO_TYPE_M2M) {
        /* connect to output pad : source */
        type = NXP_BUFFER_CONSUMER_SOURCE;
    } else {
        pr_err("%s: invalid remote type(0x%x)\n", __func__, remote->type);
        return -EINVAL;
    }

    if (connected) {
        return  _register_buffer_consumer(me, remote, type);
    } else {
        _unregister_buffer_consumer(me, remote, type);
    }

    return 0;
}

static int _handle_output_connection(struct nxp_scaler *me,
        struct nxp_video *remote, bool connected)
{
    u32 type;

    if (remote->type == NXP_VIDEO_TYPE_M2M) {
        /* connect to capture pad : sink */
        type = NXP_BUFFER_CONSUMER_SINK;
    } else if (remote->type == NXP_VIDEO_TYPE_OUT) {
        /* connect to output pad : source */
        type = NXP_BUFFER_CONSUMER_SOURCE;
    } else {
        pr_err("%s: invalid remote type(0x%x)\n", __func__, remote->type);
        return -EINVAL;
    }

    if (connected) {
        return _register_buffer_consumer(me, remote, type);
    } else {
        _unregister_buffer_consumer(me, remote, type);
    }

    return 0;
}

/**
 * utils used in subdev ops
 */
static struct v4l2_mbus_framefmt *
_get_pad_format(struct nxp_scaler *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    vmsg("%s: which(0x%x), pad(%d)\n",
            __func__, which, pad);

    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_format(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        return &me->format[pad];
    default:
        return NULL;
    }
}

/**
 * v4l2_subdev_internal_ops
 */
static int nxp_scaler_open(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    struct v4l2_mbus_framefmt *format;
    int ret;

    vmsg("%s\n", __func__);

    ret = v4l2_subdev_call(sd, core, s_power, 1);
    if (ret < 0) {
        pr_err("%s: failed to subdev s_power(), ret %d\n", __func__, ret);
        return ret;
    }

    format = v4l2_subdev_get_try_format(fh, NXP_SCALER_PAD_SINK);
    if (format) {
        format->code  = supported_formats[0];
        format->width = 0;
        format->height = 0;
        format->field = V4L2_FIELD_NONE;
        format->colorspace = V4L2_COLORSPACE_SRGB;
    }

    format = v4l2_subdev_get_try_format(fh, NXP_SCALER_PAD_SOURCE);
    if (format) {
        format->code  = supported_formats[0];
        format->width = 0;
        format->height = 0;
        format->field = V4L2_FIELD_NONE;
        format->colorspace = V4L2_COLORSPACE_SRGB;
    }

    return 0;
}

static int nxp_scaler_close(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
    vmsg("%s\n", __func__);
    return v4l2_subdev_call(sd, core, s_power, 0);
}

static int nxp_scaler_registered(struct v4l2_subdev *sd)
{
    vmsg("%s\n", __func__);
    return 0;
}

static void nxp_scaler_unregistered(struct v4l2_subdev *sd)
{
    vmsg("%s\n", __func__);
}

static const struct v4l2_subdev_internal_ops nxp_scaler_internal_ops = {
    .open         = nxp_scaler_open,
    .close        = nxp_scaler_close,
    .registered   = nxp_scaler_registered,
    .unregistered = nxp_scaler_unregistered,
};

/**
 * v4l2_subdev_core_ops
 */
static int nxp_scaler_s_power(struct v4l2_subdev *sd, int on)
{
    int ret;
    struct nxp_scaler *me;

    vmsg("%s: %d\n", __func__, on);

    me = v4l2_get_subdevdata(sd);

    if (on) {
        ret = _hw_init(me);
        if (ret < 0) {
            pr_err("%s: failed to _hw_init()\n", __func__);
            return ret;
        }
#ifndef CONFIG_ENABLE_SCALER_MISC_DEVICE
        ret = request_irq(me->irq, _irq_handler, IRQF_DISABLED, "nx-scaler", me);
        if (ret < 0) {
            pr_err("%s: failed to request_irq()\n", __func__);
            return ret;
        }
        me->irq_alive = true;
#endif
        _hw_set_filter_table(me, &_default_filter_table);
    } else {
        if (me->streaming) {
            _clear_buf(me);
            me->streaming = false;
        }
        _hw_cleanup(me);
    }

    return 0;
}

static const struct v4l2_subdev_core_ops nxp_scaler_core_ops = {
    .s_power = nxp_scaler_s_power,
};

/**
 * v4l2_subdev_video_ops
 */
static int nxp_scaler_s_stream(struct v4l2_subdev *sd, int enable)
{
    int ret = 0;
    struct nxp_scaler *me = v4l2_get_subdevdata(sd);

    vmsg("%s: %d\n", __func__, enable);

    if (enable) {
        if (!me->streaming) {
            ret = _hw_set_format(me);
            if (ret < 0) {
                pr_err("%s: failed to _hw_set_format()\n", __func__);
                return ret;
            }
            me->streaming = true;
        }
        /* TODO : start condition */
        ret = _update_next_buffer(me);
    } else {
        if (me->streaming) {
#ifdef SIMULATION_SCALER
            del_timer_sync(&me->timer);
#endif
            _clear_buf(me);
            me->streaming = false;
        }
    }

    return ret;
}

static const struct v4l2_subdev_video_ops nxp_scaler_video_ops = {
    .s_stream = nxp_scaler_s_stream,
};

/**
 * v4l2_subdev_pad_ops
 */
static int nxp_scaler_enum_mbus_code(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_mbus_code_enum *code)
{
    vmsg("%s\n", __func__);

    if (code->index >= ARRAY_SIZE(supported_formats))
        return -EINVAL;

    code->code = supported_formats[code->index];
    return 0;
}

static int nxp_scaler_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_scaler *me = v4l2_get_subdevdata(sd);
    format->format = *_get_pad_format(me, fh, format->pad, format->which);
    vmsg("%s\n", __func__);
    return 0;
}

static int nxp_scaler_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_scaler *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *__format =
        _get_pad_format(me, fh, format->pad, format->which);

    vmsg("%s\n", __func__);

    if (!__format) {
        pr_err("%s: failed to _get_pad_format()\n", __func__);
        return -EINVAL;
    }

    if (!_find_format(supported_formats,
                ARRAY_SIZE(supported_formats), format->format.code)) {
        pr_err("%s: not supported format(0x%x)\n", __func__,
                format->format.code);
        return -EINVAL;
    }

    *__format = format->format;

    return 0;
}

static const struct v4l2_subdev_pad_ops nxp_scaler_pad_ops = {
    .enum_mbus_code = nxp_scaler_enum_mbus_code,
    .get_fmt        = nxp_scaler_get_fmt,
    .set_fmt        = nxp_scaler_set_fmt,
};

/**
 * v4l2_subdev_ops
 */
static const struct v4l2_subdev_ops nxp_scaler_subdev_ops = {
    .core  = &nxp_scaler_core_ops,
    .video = &nxp_scaler_video_ops,
    .pad   = &nxp_scaler_pad_ops,
};

/**
 * media_entity_operations
 */
static int nxp_scaler_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    struct nxp_scaler *me = v4l2_get_subdevdata(sd);
    struct video_device *remote_vdev;
    struct nxp_video *remote_nxp_video;
    int ret = 0;

    vmsg("%s\n", __func__);

    switch (local->index | media_entity_type(remote->entity)) {
    case NXP_SCALER_PAD_SINK | MEDIA_ENT_T_DEVNODE:
        /* link video device to me */
        remote_vdev = media_entity_to_video_device(remote->entity);
        remote_nxp_video = video_get_drvdata(remote_vdev);
        ret = _handle_input_connection(me, remote_nxp_video,
                (flags & MEDIA_LNK_FL_ENABLED) ? true : false);
        break;

    case NXP_SCALER_PAD_SOURCE | MEDIA_ENT_T_DEVNODE:
        /* link me to video device */
        remote_vdev = media_entity_to_video_device(remote->entity);
        remote_nxp_video = video_get_drvdata(remote_vdev);
        ret = _handle_output_connection(me, remote_nxp_video,
                (flags & MEDIA_LNK_FL_ENABLED) ? true : false);
        break;

    default:
        pr_err("%s: invalid connection(%d-%d)\n",
                __func__, local->index, media_entity_type(remote->entity));
        return -EINVAL;
    }

    return ret;
}

static const struct media_entity_operations nxp_scaler_media_ops = {
    .link_setup = nxp_scaler_link_setup,
};

static int _init_entities(struct nxp_scaler *me)
{
    struct v4l2_subdev *sd = &me->subdev;
    struct media_pad *pads = me->pads;
    struct media_entity *entity = &sd->entity;
    int ret;

    me->input_type = NXP_SCALER_INPUT_TYPE_NONE;

    v4l2_subdev_init(sd, &nxp_scaler_subdev_ops);
    sd->internal_ops = &nxp_scaler_internal_ops;
    strlcpy(sd->name, "NXP SCALER", sizeof(sd->name));
    sd->grp_id = NXP_M2M_SUBDEV_GROUP_ID;
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pads[NXP_SCALER_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_SCALER_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

    entity->ops = &nxp_scaler_media_ops;
    ret = media_entity_init(entity, NXP_SCALER_PAD_MAX, pads, 0);
    if (ret < 0) {
        pr_err("%s: failed to create_nxp_video()\n", __func__);
        goto error_video;
    }
    me->video = create_nxp_video("VIDEO SCALER", NXP_VIDEO_TYPE_M2M,
            nxp_v4l2_get_v4l2_device(), nxp_v4l2_get_alloc_ctx());
    if (!me->video) {
        pr_err("%s: failed to create_nxp_video()\n", __func__);
        ret = -EINVAL;
        goto error_video;
    }

    /* create link */
    /* my source-> video sink */
    ret = media_entity_create_link(entity, NXP_SCALER_PAD_SOURCE,
            &me->video->vdev.entity, 0, 0);
    if (ret < 0) {
        pr_err("%s: failed to link my source to video sink\n", __func__);
        goto error_link;
    }
    /* video source -> my sink */
    ret = media_entity_create_link(&me->video->vdev.entity, 1,
            entity, NXP_SCALER_PAD_SINK, 0);
    if (ret < 0) {
        pr_err("%s: failed to link video source to my sink\n", __func__);
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

#ifndef CONFIG_USE_SCALER_COMMAND_BUFFER
static int _run_step(struct nxp_scaler *me, struct nxp_scaler_ioctl_data *data);
#endif

static bool _is_running(struct nxp_scaler *me)
{
#if 0
    unsigned long flags;
    int val;

    spin_lock_irqsave(&me->running_lock, flags);
    printk("%s %p\n", __func__, me);
    val = NXP_ATOMIC_READ(&me->running);
    spin_unlock_irqrestore(&me->running_lock, flags);
    return val == 1;
#else
    int val = NXP_ATOMIC_READ(&me->running);
    return val == 1;
#endif
}

static void _set_running(struct nxp_scaler *me)
{
#if 0
    unsigned long flags;

    spin_lock_irqsave(&me->running_lock, flags);
    printk("%s %p\n", __func__, me);
    NXP_ATOMIC_SET(&me->running, 1);
    spin_unlock_irqrestore(&me->running_lock, flags);
#else
    NXP_ATOMIC_SET(&me->running, 1);
#endif
}

static void _clear_running(struct nxp_scaler *me)
{
#if 0
    unsigned long flags;

    spin_lock_irqsave(&me->running_lock, flags);
    printk("%s %p\n", __func__, me);
    NXP_ATOMIC_SET(&me->running, 0);
    spin_unlock_irqrestore(&me->running_lock, flags);
#else
    NXP_ATOMIC_SET(&me->running, 0);
#endif
}

#ifdef CONFIG_ENABLE_SCALER_MISC_DEVICE
static irqreturn_t _irq_handler_misc(int irq, void *param)
{
    struct nxp_scaler *me = (struct nxp_scaler *)param;
#ifdef CONFIG_USE_SCALER_COMMAND_BUFFER
    NX_SCALER_ClearInterruptPending(0, NX_SCALER_INT_CMD_PROC);
    /*NXP_ATOMIC_SET(&me->running, 0);*/
    _clear_running(me);
    wake_up_interruptible(&me->wq_end);
    wake_up_interruptible(&me->wq_start);
#else
    vmsg("%s: %d\n", __func__, me->run_count);
    NX_SCALER_ClearInterruptPendingAll(0);
    me->run_count++;
    if (me->run_count < 3)
        _run_step(me, me->ioctl_data);
    else
        complete(&me->work_done);
#endif
    return IRQ_HANDLED;
}

#ifdef DEBUG_IOCTL_DATA
static inline void dump_scaler_ioctl_data(struct nxp_scaler_ioctl_data *data)
{
    printk("------dump scaler ioctl data----------->\n");
    printk("src phys:\t0x%x, 0x%x, 0x%x\n", data->src_phys[0], data->src_phys[1], data->src_phys[2]);
    printk("src stride:\t0x%x, 0x%x, 0x%x\n", data->src_stride[0], data->src_stride[1], data->src_stride[2]);
    printk("src width/height:\t%dx%d\n", data->src_width, data->src_height);
    printk("src code:\t0x%x\n", data->src_code);
    printk("dst phys:\t0x%x, 0x%x, 0x%x\n", data->dst_phys[0], data->dst_phys[1], data->dst_phys[2]);
    printk("dst stride:\t0x%x, 0x%x, 0x%x\n", data->dst_stride[0], data->dst_stride[1], data->dst_stride[2]);
    printk("dst width/height:\t%dx%d\n", data->dst_width, data->dst_height);
    printk("dst code:\t0x%x\n", data->dst_code);
    printk("<-----------------\n");
}
#endif

#ifdef CONFIG_USE_SCALER_COMMAND_BUFFER

/* #define DUMP_COMMAND_BUFFER */
#ifdef DUMP_COMMAND_BUFFER
static inline void dump_command_buffer(struct nxp_scaler *me, unsigned int *end)
{
    unsigned int *dump_buffer = me->command_buffer_vir;
    printk("DUMP CMD BUFFER =========>\n");
    while (dump_buffer < end) {
        printk("0x%x\n", *dump_buffer);
        dump_buffer++;
    }
}
#endif

static int _make_command_buffer_misc(struct nxp_scaler *me, struct nxp_scaler_ioctl_data *data)
{
    unsigned int *cmd_buffer;
    unsigned int src_width, src_height, src_code, dst_width, dst_height, dst_code;
    unsigned int cb_src_width, cb_src_height, cb_dst_width, cb_dst_height;

    if (!me->command_buffer_vir) {
        me->command_buffer_vir = dma_alloc_coherent(NULL, COMMAND_BUFFER_SIZE, &me->command_buffer_phy, GFP_KERNEL);
        if (!me->command_buffer_vir) {
            pr_err("%s: failed to allocate command buffer!!!\n", __func__);
            return -ENOMEM;
        }
    }

#ifdef DEBUG_IOCTL_DATA
    dump_scaler_ioctl_data(data);
#endif

    src_width = data->src_width;
    src_height = data->src_height;
    src_code = data->src_code;
    dst_width = data->dst_width;
    dst_height = data->dst_height;
    dst_code = data->dst_code;

    cmd_buffer = me->command_buffer_vir;

    /* Y command buffer */
    /* header */
    _make_cmd(cmd_buffer); cmd_buffer++;

    /* Source Address Register */
    *cmd_buffer = data->src_phys[0]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = data->src_stride[0]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((src_height - 1) << 16) | (src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = data->dst_phys[0]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = data->dst_stride[0]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((dst_height - 1) << 16) | (dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (src_width << 16) / (dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (src_height << 16) / (dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;

    /* CB command buffer */
    if (src_code == V4L2_MBUS_FMT_YUYV8_1_5X8) {
        /* 420 */
        cb_src_width = src_width >> 1;
        cb_src_height = src_height >> 1;
    } else if (src_code == V4L2_MBUS_FMT_YUYV8_1X16) {
        /* 422 */
        cb_src_width = src_width >> 1;
        cb_src_height = src_height;
    } else {
        cb_src_width = src_width;
        cb_src_height = src_height;
    }
    if (dst_code == V4L2_MBUS_FMT_YUYV8_1_5X8) {
        /* 420 */
        cb_dst_width = dst_width >> 1;
        cb_dst_height = dst_height >> 1;
    } else if (dst_code == V4L2_MBUS_FMT_YUYV8_1X16) {
        /* 422 */
        cb_dst_width = dst_width >> 1;
        cb_dst_height = dst_height;
    } else {
        cb_dst_width = dst_width;
        cb_dst_height = dst_height;
    }

    _make_cmd(cmd_buffer); cmd_buffer++;
    /* Source Address Register */
    *cmd_buffer = data->src_phys[1]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = data->src_stride[1]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((cb_src_height - 1) << 16) | (cb_src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = data->dst_phys[1]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = data->dst_stride[1]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((cb_dst_height - 1) << 16) | (cb_dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (cb_src_width << 16) / (cb_dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (cb_src_height << 16) / (cb_dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;

    /* CR command buffer */
    _make_cmd(cmd_buffer); cmd_buffer++;
    /* Source Address Register */
    *cmd_buffer = data->src_phys[2]; cmd_buffer++;
    /* Source Stride Register */
    *cmd_buffer = data->src_stride[2]; cmd_buffer++;
    /* Source Size Register */
    *cmd_buffer = ((cb_src_height - 1) << 16) | (cb_src_width - 1); cmd_buffer++;
    /* Destination Address */
    *cmd_buffer = data->dst_phys[2]; cmd_buffer++;
    /* Destination Stride Register */
    *cmd_buffer = data->dst_stride[2]; cmd_buffer++;
    /* not use Destination Address1, Destination Stride Register1 */
    cmd_buffer++;
    cmd_buffer++;
    /* Destination Size Register */
    *cmd_buffer = ((cb_dst_height - 1) << 16) | (cb_dst_width - 1); cmd_buffer++;
    /* Horizontal Delta Register */
    *cmd_buffer = (cb_src_width << 16) / (cb_dst_width - 1); cmd_buffer++;
    /* Vertical Delta Register */
    *cmd_buffer = (cb_src_height << 16) / (cb_dst_height - 1); cmd_buffer++;
    /* Ratio Reset Value Register : TODO fixed ??? */
    *cmd_buffer = 0x00080010; cmd_buffer++;

    /* workaround */
    *cmd_buffer = 0; cmd_buffer++;
    *cmd_buffer = 0x00000001; cmd_buffer++;
    *cmd_buffer = (1 << 29) | (1 << 27) | (1 << 10); cmd_buffer++;
    *cmd_buffer = 0x00000003; cmd_buffer++;

#ifdef DUMP_COMMAND_BUFFER
    dump_command_buffer(me, cmd_buffer);
#endif
    return 0;
}
#endif

static inline unsigned int _get_width(unsigned int code, unsigned int in_width)
{
    uint32_t width;
    switch (code) {
    case V4L2_MBUS_FMT_YUYV8_1_5X8:
    case V4L2_MBUS_FMT_YUYV8_1X16:
        width = in_width >> 1;
        break;
    default:
        width = in_width;
        break;
    }

    return width;
}

static inline unsigned int _get_height(unsigned int code, unsigned int in_height)
{
    uint32_t height;
    switch (code) {
    case V4L2_MBUS_FMT_YUYV8_1_5X8:
        height = in_height >> 1;
        break;
    default:
        height = in_height;
        break;
    }

    return height;
}

#ifndef CONFIG_USE_SCALER_COMMAND_BUFFER
static int _run_step(struct nxp_scaler *me, struct nxp_scaler_ioctl_data *data)
{
    unsigned int src_phy, src_stride, src_width, src_height;
    unsigned int dst_phy, dst_stride, dst_width, dst_height;

    if (me->run_count == 0) {
        src_phy = data->src_phys[0];
        src_stride = data->src_stride[0];
        src_width = data->src_width;
        src_height = data->src_height;
        dst_phy = data->dst_phys[0];
        dst_stride = data->dst_stride[0];
        dst_width = data->dst_width;
        dst_height = data->dst_height;
    } else if (me->run_count == 1) {
        if (!data->src_phys[1]) {
            complete(&me->work_done);
            return 0;
        }
        src_phy = data->src_phys[1];
        src_stride = data->src_stride[1];
        src_width = _get_width(data->src_code, data->src_width);
        src_height = _get_height(data->src_code, data->src_height);
        dst_phy = data->dst_phys[1];
        dst_stride = data->dst_stride[1];
        dst_width = _get_width(data->dst_code, data->dst_width);
        dst_height = _get_height(data->dst_code, data->dst_height);
    } else if (me->run_count == 2) {
        if (!data->src_phys[2]) {
            complete(&me->work_done);
            return 0;
        }
        src_phy = data->src_phys[2];
        src_stride = data->src_stride[2];
        src_width = _get_width(data->src_code, data->src_width);
        src_height = _get_height(data->src_code, data->src_height);
        dst_phy = data->dst_phys[2];
        dst_stride = data->dst_stride[2];
        dst_width = _get_width(data->dst_code, data->dst_width);
        dst_height = _get_height(data->dst_code, data->dst_height);
    } else {
        printk(KERN_ERR "invalid run_count: %d\n", me->run_count);
        return -EINVAL;
    }

    /* Source Address Register */
    NX_SCALER_SetSrcAddr(0, src_phy);
    /* Source Stride Register */
    NX_SCALER_SetSrcStride(0, src_stride);
    /* Destination Address */
    NX_SCALER_SetDestAddr(0, 0, dst_phy);
    /* Destination Stride Register */
    NX_SCALER_SetDestStride(0, 0, dst_stride);
    /* Image Size : src, dst, delta */
    NX_SCALER_SetImageSize(0, src_width, src_height, dst_width, dst_height);
    /* Ratio Reset Value Register : TODO fixed ??? */
    NX_SCALER_SetFilterRatio(0, 0x10, 0x8);

    NX_SCALER_SetInterruptEnable(0, NX_SCALER_INT_DONE, CTRUE);

    NX_SCALER_Run(0);

    return 0;
}
#endif

#define WAIT_TIMEOUT_HZ     (HZ/5) // 200ms
static int _set_and_run(struct nxp_scaler *me, struct nxp_scaler_ioctl_data *data)
{
#ifdef CONFIG_USE_SCALER_COMMAND_BUFFER
    /*if (NXP_ATOMIC_READ(&me->running) == 1) {*/
    unsigned long flags;

    spin_lock_irqsave(&me->running_lock, flags);
    if (_is_running(me)) {
         vmsg("wait start\n");
        /*if (!wait_event_interruptible_timeout(me->wq_start, NXP_ATOMIC_READ(&me->running) == 0, WAIT_TIMEOUT_HZ)) {*/
         spin_unlock_irqrestore(&me->running_lock, flags);
        if (!wait_event_interruptible_timeout(me->wq_start, _is_running(me) == false, WAIT_TIMEOUT_HZ)) {
            spin_unlock_irqrestore(&me->running_lock, flags);
            printk("wait timeout for starting\n");
            /*return -EBUSY;*/
            return 0;
        }
        spin_lock_irqsave(&me->running_lock, flags);
    }
    _set_running(me);
    spin_unlock_irqrestore(&me->running_lock, flags);

    /*NXP_ATOMIC_SET(&me->running, 1);*/
    _make_command_buffer_misc(me, data);
    NX_SCALER_SetCmdBufAddr(0, me->command_buffer_phy);
    NX_SCALER_SetInterruptEnable(0, NX_SCALER_INT_CMD_PROC, CTRUE);
    NX_SCALER_SetMode(0, 0);
    vmsg("start scaler\n");
    NX_SCALER_RunCmdBuf(0);
#else
    me->ioctl_data = data;
    init_completion(&me->work_done);
    me->run_count = 0;
    _run_step(me, data);
#endif

    if (!wait_event_interruptible_timeout(me->wq_end, NXP_ATOMIC_READ(&me->running) == 0, WAIT_TIMEOUT_HZ)) {
        printk("wait timeout for scaling end\n");
        // psw0523 add for scaler not reworking bug
#if 1
        printk("cleanup scaler!!!\n");
        /*NXP_ATOMIC_SET(&me->running, 0);*/
        _clear_running(me);

        NX_SCALER_Stop(0);
        NX_SCALER_SetInterruptEnableAll(0, CFALSE);
        NX_SCALER_ClearInterruptPendingAll(0);
        NX_CLKGEN_SetClockBClkMode(NX_SCALER_GetClockNumber(0), NX_BCLKMODE_DISABLE);

        _hw_init(me);

        if (me->command_buffer_phy) {
            dma_free_coherent(NULL, COMMAND_BUFFER_SIZE, me->command_buffer_vir, me->command_buffer_phy);
            me->command_buffer_vir = NULL;
            me->command_buffer_phy = 0;
        }
        _hw_set_filter_table(me, &_default_filter_table);
        _hw_set_format(me);

        /*wake_up_interruptible(&me->wq_end);*/
        /*wake_up_interruptible(&me->wq_start);*/
#endif
    }
    vmsg("end scaler\n");
    return 0;
}

static int nxp_scaler_misc_open(struct inode *inode, struct file *file)
{
    int ret;
    struct nxp_scaler *me = _scaler;
    file->private_data = me;

    if (NXP_ATOMIC_READ(&me->open_count) > 0) {
        NXP_ATOMIC_INC(&me->open_count);
        vmsg("%s: open count %d\n", __func__, NXP_ATOMIC_READ(&me->open_count));
        return 0;
    }
    vmsg("%s: entered\n", __func__);
    _hw_init(me);
    _hw_set_filter_table(me, &_default_filter_table);
    _hw_set_format(me);
    ret = request_irq(me->irq, _irq_handler_misc, IRQF_DISABLED, "nxp-scaler", me);
    if (ret < 0) {
        pr_err("%s: failed to request_irq()\n", __func__);
        return ret;
    }
    me->irq_alive = true;
    NXP_ATOMIC_INC(&me->open_count);
    vmsg("%s: exit\n", __func__);
    return 0;
}

static int nxp_scaler_misc_release(struct inode *inode, struct file *file)
{
    struct nxp_scaler *me = (struct nxp_scaler *)file->private_data;
    NXP_ATOMIC_DEC(&me->open_count);
    if (NXP_ATOMIC_READ(&me->open_count) == 0) {
        vmsg("%s: hw cleanup\n", __func__);
        _hw_cleanup(me);
    }
    return 0;
}

static long nxp_scaler_misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {
    case IOCTL_SCALER_SET_AND_RUN:
        {
            struct nxp_scaler *me = (struct nxp_scaler *)file->private_data;
            struct nxp_scaler_ioctl_data data;
            if (copy_from_user(&data, (void __user *)arg, sizeof(struct nxp_scaler_ioctl_data))) {
                printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
                return -EFAULT;
            }
            ret = _set_and_run(me, &data);
        }
        break;

    default:
        break;
    }

    return ret;
}

static const struct file_operations nxp_scaler_file_ops = {
    .owner          = THIS_MODULE,
    .open           = nxp_scaler_misc_open,
    .release        = nxp_scaler_misc_release,
    .unlocked_ioctl = nxp_scaler_misc_ioctl,
};

static struct miscdevice nxp_scaler_misc_device = {
    .minor          = MISC_DYNAMIC_MINOR,
    .name           = "nxp-scaler",
    .fops           = &nxp_scaler_file_ops,
};
#endif /* CONFIG_ENABLE_SCALER_MISC_DEVICE */

/**
 * public api
 */
struct nxp_scaler *create_nxp_scaler(void)
{
    int ret;
    struct nxp_scaler *me = NULL;

    me = (struct nxp_scaler *)kzalloc(sizeof(struct nxp_scaler), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to alloc nxp_scaler\n", __func__);
        return NULL;
    }

    ret = _init_entities(me);

    if (likely(!ret)) {
        INIT_LIST_HEAD(&me->src_buffer_list);
        INIT_LIST_HEAD(&me->dst_buffer_list);
        spin_lock_init(&me->slock);
        spin_lock_init(&me->dlock);
        me->command_buffer_vir = NULL;
        me->command_buffer_phy = 0;
#ifdef CONFIG_ENABLE_SCALER_MISC_DEVICE
        ret = misc_register(&nxp_scaler_misc_device);
        if (ret) {
            printk(KERN_ERR "%s: failed to misc_register()\n", __func__);
            kfree(me);
            return NULL;
        }
        NXP_ATOMIC_SET(&me->open_count, 0);
        spin_lock_init(&me->running_lock);
        NXP_ATOMIC_SET(&me->running, 0);
        init_waitqueue_head(&me->wq_start);
        init_waitqueue_head(&me->wq_end);
#endif
    }

    _scaler = me;
    return me;
}

void release_nxp_scaler(struct nxp_scaler *me)
{
    vmsg("%s\n", __func__);
#ifdef CONFIG_ENABLE_SCALER_MISC_DEVICE
    misc_deregister(&nxp_scaler_misc_device);
#endif
    release_nxp_video(me->video);
    kfree(me);
    _scaler = NULL;
}

int register_nxp_scaler(struct nxp_scaler *me)
{
    int ret;

    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(),
            &me->subdev);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    ret = register_nxp_video(me->video);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_video()\n", __func__);
        v4l2_device_unregister_subdev(&me->subdev);
    } else {
        vmsg("%s: success!!!\n", __func__);
    }

    return ret;
}

void unregister_nxp_scaler(struct nxp_scaler *me)
{
    vmsg("%s\n", __func__);
    if (me->command_buffer_phy) {
        dma_free_coherent(NULL, COMMAND_BUFFER_SIZE, me->command_buffer_vir, me->command_buffer_phy);
        me->command_buffer_vir = NULL;
        me->command_buffer_phy = 0;
    }
    unregister_nxp_video(me->video);
    v4l2_device_unregister_subdev(&me->subdev);
}

#ifdef CONFIG_PM
int suspend_nxp_scaler(struct nxp_scaler *me)
{
    PM_DBGOUT("%s+\n", __func__);
    if (NXP_ATOMIC_READ(&me->open_count) > 0) {
        NX_SCALER_Stop(0);
        NX_SCALER_SetInterruptEnableAll(0, CFALSE);
        NX_SCALER_ClearInterruptPendingAll(0);
        NX_CLKGEN_SetClockBClkMode(NX_SCALER_GetClockNumber(0), NX_BCLKMODE_DISABLE);
    }
    PM_DBGOUT("%s-\n", __func__);
    return 0;
}

int resume_nxp_scaler(struct nxp_scaler *me)
{
    PM_DBGOUT("%s+\n", __func__);
    if (NXP_ATOMIC_READ(&me->open_count) > 0) {
        _hw_init(me);
        _hw_set_filter_table(me, &_default_filter_table);
        _hw_set_format(me);
    }
    PM_DBGOUT("%s-\n", __func__);
    return 0;
}
#endif
