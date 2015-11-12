/*#define DEBUG 0*/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>
#include <mach/platform.h>

#include <asm/io.h>

/* prototype */
#include <nx_tieoff.h>
#include <nx_mipi.h>

#include "nxp-v4l2-common.h"
#include "nxp-video.h"
#include "nxp-v4l2.h"

#include "nxp-capture.h"
#include "nxp-csi.h"


#define CSI0_MAX_LANES		4
#define CSI1_MAX_LANES		2
/* Register map definition */

/* CSI global control */
#define NXP_CSI_CTRL			    0x00
#define NXP_CSI_CTRL_DPDN_DEFAULT	(0 << 31)
#define NXP_CSI_CTRL_DPDN_SWAP		(1 << 31)
#define NXP_CSI_CTRL_ALIGN_32BIT	(1 << 20)
#define NXP_CSI_CTRL_UPDATE_SHADOW	(1 << 16)
#define NXP_CSI_CTRL_WCLK_EXTCLK	(1 << 8)
#define NXP_CSI_CTRL_RESET		    (1 << 4)
#define NXP_CSI_CTRL_ENABLE		    (1 << 0)

/* D-PHY control */
#define NXP_CSI_DPHYCTRL		    0x04
#define NXP_CSI_DPHYCTRL_HSS_MASK	(0x1f << 27)
#define NXP_CSI_DPHYCTRL_ENABLE		(0x1f << 0)

#define NXP_CSI_CONFIG			    0x08
#define NXP_CSI_CFG_FMT_YCBCR422_8BIT	(0x1e << 2)
#define NXP_CSI_CFG_FMT_RAW8		(0x2a << 2)
#define NXP_CSI_CFG_FMT_RAW10		(0x2b << 2)
#define NXP_CSI_CFG_FMT_RAW12		(0x2c << 2)
/* User defined formats, x = 1...4 */
#define NXP_CSI_CFG_FMT_USER(x)		((0x30 + x - 1) << 2)
#define NXP_CSI_CFG_FMT_MASK		(0x3f << 2)
#define NXP_CSI_CFG_NR_LANE_MASK	3

/* Interrupt mask. */
#define NXP_CSI_INTMSK			    0x10
#define NXP_CSI_INTMSK_EN_ALL		0xf000103f
#define NXP_CSI_INTSRC			    0x14

/* Pixel resolution */
#define NXP_CSI_RESOL			    0x2c
#define CSI_MAX_PIX_WIDTH		    0xffff
#define CSI_MAX_PIX_HEIGHT		    0xffff
#define CSI_SRC_CLK			        "mout_mpll_user"

/**
 * pixel format description
 * @pix_width_alignment: width will be multiple of 2^pix_width_alignment
 * @code: media bus code
 * @fmt_reg: NXP_CSI_CONFIG register value
 */
struct nxp_csi_pix_format {
    unsigned int pix_width_alignement;
    enum v4l2_mbus_pixelcode code;
    u32 fmt_reg;
};

/**
 * supported formats: only support in/out YUYV422 packed
 */
static const struct nxp_csi_pix_format supported_formats[] = {
    /* YUYV422 packed */
    {
        .code = V4L2_MBUS_FMT_YUYV8_2X8,
        .fmt_reg = NXP_CSI_CFG_FMT_YCBCR422_8BIT,
    },
};

/**
 * hw specific functions
 */
#define _csi_write(me, r, v) writel(v, me->regs + r)
#define _csi_read(me, r)     readl(me->regs + r)

/* one shot setting */
static void _hw_run(struct nxp_csi *me)
{
    NX_MIPI_Initialize();
    NX_TIEOFF_Set(TIEOFFINDEX_OF_MIPI0_NX_DPSRAM_1R1W_EMAA, 3);
    NX_TIEOFF_Set(TIEOFFINDEX_OF_MIPI0_NX_DPSRAM_1R1W_EMAA, 3);

    NX_MIPI_SetBaseAddress(me->module, (void*)IO_ADDRESS(NX_MIPI_GetPhysicalAddress(me->module)));
    NX_CLKGEN_SetBaseAddress(NX_MIPI_GetClockNumber(me->module),
            (void*)IO_ADDRESS(NX_CLKGEN_GetPhysicalAddress(NX_MIPI_GetClockNumber(me->module))));
    NX_CLKGEN_SetClockDivisorEnable(NX_MIPI_GetClockNumber(me->module), CTRUE);

    nxp_soc_peri_reset_enter(RESET_ID_MIPI);
    nxp_soc_peri_reset_enter(RESET_ID_MIPI_CSI);
    nxp_soc_peri_reset_enter(RESET_ID_MIPI_PHY_S);
    nxp_soc_peri_reset_exit(RESET_ID_MIPI);
    nxp_soc_peri_reset_exit(RESET_ID_MIPI_CSI);

    NX_MIPI_OpenModule(me->module);
    NX_MIPI_SetInterruptEnableAll(me->module, CFALSE);
    NX_MIPI_ClearInterruptPendingAll(me->module);

    NX_CLKGEN_SetClockDivisorEnable(NX_MIPI_GetClockNumber(me->module), CFALSE);
    /* TODO : use clk_get(), clk_get_rate() for dynamic clk binding */
    NX_CLKGEN_SetClockSource(NX_MIPI_GetClockNumber(me->module), 0, 2); // use pll2 -> current 295MHz
    NX_CLKGEN_SetClockDivisor(NX_MIPI_GetClockNumber(me->module), 0, 2);
    /* NX_CLKGEN_SetClockDivisor(NX_MIPI_GetClockNumber(me->module), 0, 6); */
    NX_CLKGEN_SetClockDivisorEnable(NX_MIPI_GetClockNumber(me->module), CTRUE);

    NX_MIPI_CSI_SetParallelDataAlignment32(me->module, 1, CFALSE);
    NX_MIPI_CSI_SetYUV422Layout(me->module, 1, NX_MIPI_CSI_YUV422LAYOUT_FULL);
    NX_MIPI_CSI_SetFormat(me->module, 1, NX_MIPI_CSI_FORMAT_YUV422_8);
    NX_MIPI_CSI_EnableDecompress(me->module, CFALSE);
    NX_MIPI_CSI_SetInterleaveMode(me->module, NX_MIPI_CSI_INTERLEAVE_VC);
    NX_MIPI_CSI_SetTimingControl(me->module, 1, 32, 16, 368);
    NX_MIPI_CSI_SetInterleaveChannel(me->module, 0, 1);
    NX_MIPI_CSI_SetInterleaveChannel(me->module, 1, 0);
    vmsg("%s: width %d, height %d\n", __func__, me->format.width, me->format.height);
    NX_MIPI_CSI_SetSize(me->module, 1, me->format.width, me->format.height);
    NX_MIPI_CSI_SetVCLK(me->module, 1, NX_MIPI_CSI_VCLKSRC_EXTCLK);
    /* HACK!!! -> this is variation : confirm to camera sensor */
    NX_MIPI_CSI_SetPhy(me->module,
            1,  // U32   NumberOfDataLanes (0~3)
            1,  // CBOOL EnableClockLane
            1,  // CBOOL EnableDataLane0
            1,  // CBOOL EnableDataLane1
            0,  // CBOOL EnableDataLane2
            0,  // CBOOL EnableDataLane3
            0,  // CBOOL SwapClockLane
            0   // CBOOL SwapDataLane
            );
    NX_MIPI_CSI_SetEnable(me->module, CTRUE);

    nxp_soc_peri_reset_exit(RESET_ID_MIPI_PHY_S);

    /* NX_MIPI_DSI_SetPLL( me->module, */
    /*                     CTRUE,       // CBOOL Enable      , */
    /*                     0xFFFFFFFF,  // U32 PLLStableTimer, */
    /*                     0x33E8,      // 19'h033E8: 1Ghz  19'h043E8: 750Mhz // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. */
    /*                     0xF,         // 4'hF: 1Ghz  4'hC: 750Mhz           // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. */
    /*                     0,           // U32 M_PLLCTL      , */
    /*                                  // Refer to 10.2.2 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf  Default value is all "0". */
    /*                                  // If you want to change register values, it need to confirm from IP Design Team */
    /*                     0            // U32 B_DPHYCTL */
    /*                                  // Refer to 10.2.3 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf */
    /*                                  // or NX_MIPI_PHY_B_DPHYCTL enum or LN28LPP_MipiDphyCore1p5Gbps_Supplement. */
    /*                                  // default value is all "0". */
    /*                                  // If you want to change register values, it need to confirm from IP Design Team */
    /*                    ); */
    NX_MIPI_DSI_SetPLL( me->module,
                        CTRUE,       // CBOOL Enable      ,
                        0xFFFFFFFF,  // U32 PLLStableTimer,
                        0x43E8,      // 19'h033E8: 1Ghz  19'h043E8: 750Mhz // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
                        0xC,         // 4'hF: 1Ghz  4'hC: 750Mhz           // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
                        0,           // U32 M_PLLCTL      ,
                                     // Refer to 10.2.2 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf  Default value is all "0".
                                     // If you want to change register values, it need to confirm from IP Design Team
                        0            // U32 B_DPHYCTL
                                     // Refer to 10.2.3 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf
                                     // or NX_MIPI_PHY_B_DPHYCTL enum or LN28LPP_MipiDphyCore1p5Gbps_Supplement.
                                     // default value is all "0".
                                     // If you want to change register values, it need to confirm from IP Design Team
                       );

    NX_MIPI_SetInterruptEnable(me->module, NX_MIPI_INT_CSI_FrameEnd_CH1, CTRUE);
}

static void _hw_enable_interrupts(struct nxp_csi *me, bool on)
{
    u32 val = _csi_read(me, NXP_CSI_INTMSK);
    val = on ? val | NXP_CSI_INTMSK_EN_ALL :
            val & ~NXP_CSI_INTMSK_EN_ALL;
    _csi_write(me, NXP_CSI_INTMSK, val);
}

/* static void _hw_reset(struct nxp_csi *me) */
/* { */
/*     u32 val = _csi_read(me, NXP_CSI_CTRL); */
/*     _csi_write(me, NXP_CSI_CTRL, val | NXP_CSI_CTRL_RESET); */
/*     udelay(10); */
/* } */

static void _hw_system_enable(struct nxp_csi *me, bool on)
{
    u32 val = _csi_read(me, NXP_CSI_CTRL);
    if (on)
        val |= NXP_CSI_CTRL_ENABLE;
    else
        val &= ~NXP_CSI_CTRL_ENABLE;
    _csi_write(me, NXP_CSI_CTRL, val);

    val = _csi_read(me, NXP_CSI_DPHYCTRL);
    if (on)
        val |= NXP_CSI_DPHYCTRL_ENABLE;
    else
        val &= ~NXP_CSI_DPHYCTRL_ENABLE;
    _csi_write(me, NXP_CSI_DPHYCTRL, val);
}

/* static void _hw_set_format(struct nxp_csi *me) */
/* { */
/*     struct v4l2_mbus_framefmt *fmt = &me->format; */
/*  */
/*     #<{(| pixel format |)}># */
/*     u32 val = _csi_read(me, NXP_CSI_CONFIG); */
/*     val = (val & ~NXP_CSI_CFG_FMT_MASK) | me->csi_format->fmt_reg; */
/*     _csi_write(me, NXP_CSI_CONFIG, val); */
/*  */
/*     #<{(| resolution |)}># */
/*     val = (fmt->width << 16) | fmt->height; */
/*     _csi_write(me, NXP_CSI_RESOL, val); */
/* } */

/* static void _hw_set_hsync_settle(struct nxp_csi *me, int settle) */
/* { */
/*     u32 val = _csi_read(me, NXP_CSI_DPHYCTRL); */
/*     val = (val & ~NXP_CSI_DPHYCTRL_HSS_MASK) | (settle << 27); */
/*     _csi_write(me, NXP_CSI_DPHYCTRL, val); */
/* } */

/* static void _hw_set_params(struct nxp_csi *me) */
/* { */
/*     struct nxp_mipi_csi_platformdata *pdata = me->platdata; */
/*     u32 val; */
/*  */
/*     val = _csi_read(me, NXP_CSI_CONFIG); */
/*     val = (val & ~NXP_CSI_CFG_NR_LANE_MASK) | (pdata->lanes - 1); */
/*     _csi_write(me, NXP_CSI_CONFIG, val); */
/*  */
/*     _hw_set_format(me); */
/*     _hw_set_hsync_settle(me, pdata->hs_settle); */
/*  */
/*     val = _csi_read(me, NXP_CSI_CTRL); */
/*     if (pdata->alignment == 32) */
/*         val |= NXP_CSI_CTRL_ALIGN_32BIT; */
/*     else */
/*         val &= ~NXP_CSI_CTRL_ALIGN_32BIT; */
/*     val |= NXP_CSI_CTRL_WCLK_EXTCLK; */
/*     _csi_write(me, NXP_CSI_CTRL, val); */
/*  */
/*     val = _csi_read(me, NXP_CSI_CTRL); */
/*     _csi_write(me, NXP_CSI_CTRL, val | NXP_CSI_CTRL_UPDATE_SHADOW); */
/* } */

static void _hw_set_clock(struct nxp_csi *me, bool on)
{
}

/* static void _hw_start_stream(struct nxp_csi *me) */
/* { */
/*     _hw_reset(me); */
/*     _hw_set_params(me); */
/*     _hw_system_enable(me, true); */
/*     _hw_enable_interrupts(me, true); */
/* } */

static void _hw_stop_stream(struct nxp_csi *me)
{
    _hw_enable_interrupts(me, false);
    _hw_system_enable(me, false);
}

/**
 * irq handler
 */
static irqreturn_t nxp_csi_irq_handler(int irq, void *dev_id)
{
    /* only report error */
    struct nxp_csi *me = dev_id;
    u32 val;

    val = _csi_read(me, NXP_CSI_INTSRC);
    _csi_write(me, NXP_CSI_INTSRC, val);

    pr_err("%s: irq error(0x%x)\n", __func__, val);
    return IRQ_HANDLED;
}

/*
 * internal ops
 */
static inline struct v4l2_subdev *
_get_remote_source_subdev(struct nxp_csi *me)
{
    struct media_pad *pad;
    struct v4l2_subdev *remote = NULL;

    pad = media_entity_remote_source(&me->pads[NXP_CSI_PAD_SINK]);
    if (pad)
        remote = media_entity_to_v4l2_subdev(pad->entity);
    return remote;
}

static struct v4l2_mbus_framefmt *
_get_pad_format(struct nxp_csi *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_format(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        return &me->format;
    default:
        return NULL;
    }
}

/*
 * v4l2_subdev_internal_ops
 */
static int nxp_csi_registered(struct v4l2_subdev *sd)
{
    vmsg("%s\n", __func__);
    return 0;
}

static void nxp_csi_unregistered(struct v4l2_subdev *sd)
{
    vmsg("%s\n", __func__);
}

static int nxp_csi_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    struct nxp_csi *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *format;

    vmsg("%s\n", __func__);

    format = v4l2_subdev_get_try_format(fh, NXP_CSI_PAD_SINK);
    if (format) {
        format->code  = supported_formats[0].code;
        format->width = me->platdata->width;
        format->height = me->platdata->height;
        format->field = V4L2_FIELD_NONE;
        format->colorspace = V4L2_COLORSPACE_SRGB;
    }

    return 0;
}

static int nxp_csi_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    vmsg("%s\n", __func__);
    return 0;
}

static const struct v4l2_subdev_internal_ops nxp_csi_internal_ops = {
    .open         = nxp_csi_open,
    .close        = nxp_csi_close,
    .registered   = nxp_csi_registered,
    .unregistered = nxp_csi_unregistered,
};

/*
 * v4l2_subdev_core_ops
 */
static int nxp_csi_s_power(struct v4l2_subdev *sd, int on)
{
    /**
     * on/off phy
     */
    struct nxp_csi *me = v4l2_get_subdevdata(sd);
    struct v4l2_subdev *remote_source = _get_remote_source_subdev(me);
    int ret = 0;

    vmsg("%s: %d\n", __func__, on);

    if (on) {
        _hw_set_clock(me, true);
        me->platdata->phy_enable(true);
        ret = v4l2_subdev_call(remote_source, core, s_power, 1);
    } else {
        ret = v4l2_subdev_call(remote_source, core, s_power, 0);
        me->platdata->phy_enable(false);
        _hw_set_clock(me, false);
    }

    return ret;
}

static struct v4l2_subdev_core_ops nxp_csi_core_ops = {
    .s_power = nxp_csi_s_power,
};

/*
 * v4l2_subdev_video_ops
 */
static int nxp_csi_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct nxp_csi *me = v4l2_get_subdevdata(sd);
    struct v4l2_subdev *remote_source = _get_remote_source_subdev(me);
    int ret;

    vmsg("%s: %d\n", __func__, enable);
    if (enable) {
        ret = v4l2_subdev_call(remote_source, video, s_stream, enable);
        /* _hw_start_stream(me); */
        _hw_run(me);
        NXP_ATOMIC_SET(&me->state, NXP_CSI_STATE_RUNNING);
    } else {
        _hw_stop_stream(me);
        ret = v4l2_subdev_call(remote_source, video, s_stream, enable);
        NXP_ATOMIC_CLEAR_MASK(NXP_CSI_STATE_RUNNING, &me->state);
    }

    return ret;
}

static struct v4l2_subdev_video_ops nxp_csi_video_ops = {
    .s_stream = nxp_csi_s_stream,
};

/*
 * v4l2_subdev_pad_ops
 */
static int nxp_csi_enum_mbus_code(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_mbus_code_enum *code)
{
    if (code->index >= ARRAY_SIZE(supported_formats))
        return -EINVAL;

    code->code = supported_formats[code->index].code;
    return 0;
}

static int nxp_csi_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_csi *me = v4l2_get_subdevdata(sd);
    format->format = *_get_pad_format(me, fh, format->pad, format->which);
    return 0;
}

static int nxp_csi_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_csi *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *__format =
        _get_pad_format(me, fh, format->pad, format->which);

    if (format->format.code != supported_formats[0].code) {
        pr_err("%s: not supported format(0x%x)\n", __func__, format->format.code);
        return -EINVAL;
    }

    *__format = format->format;

    if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE)
        me->csi_format = (struct nxp_csi_pix_format *)&supported_formats[0];

    return 0;
}

static struct v4l2_subdev_pad_ops nxp_csi_pad_ops = {
    .enum_mbus_code  = nxp_csi_enum_mbus_code,
    .get_fmt         = nxp_csi_get_fmt,
    .set_fmt         = nxp_csi_set_fmt,
};

/*
 * v4l2_subdev_ops
 */
static struct v4l2_subdev_ops nxp_csi_subdev_ops = {
    .core   = &nxp_csi_core_ops,
    .pad    = &nxp_csi_pad_ops,
    .video  = &nxp_csi_video_ops,
};

/*
 * media entity ops
 */
static int nxp_csi_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    /**
     * current do nothing
     * only debugging
     * TODO
     */
    //struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    //struct nxp_csi *me = v4l2_get_subdevdata(sd);

    switch (local->index | media_entity_type(remote->entity)) {
    case NXP_CSI_PAD_SINK | MEDIA_ENT_T_V4L2_SUBDEV:
        if (flags & MEDIA_LNK_FL_ENABLED) {
            vmsg("%s: input sensor connected\n", __func__);
        } else {
            vmsg("%s: input sensor disconnected\n", __func__);
        }
        break;

    case NXP_CSI_PAD_SOURCE | MEDIA_ENT_T_V4L2_SUBDEV:
        if (flags & MEDIA_LNK_FL_ENABLED) {
            vmsg("%s: output connected\n", __func__);
        } else {
            vmsg("%s: output disconnected\n", __func__);
        }
        break;

    default:
        pr_err("%s: invalid link\n", __func__);
        return -EINVAL;
    }

    return 0;
}

static const struct media_entity_operations nxp_csi_media_ops = {
    .link_setup = nxp_csi_link_setup,
};

/**
 * internal ops
 */
static int _init_entities(struct nxp_csi *me)
{
    struct v4l2_subdev *sd = &me->subdev;
    struct media_pad *pads = me->pads;
    struct media_entity *entity = &sd->entity;
    //struct nxp_capture *parent = nxp_csi_to_parent(me);
    int ret;

    v4l2_subdev_init(sd, &nxp_csi_subdev_ops);
    sd->internal_ops = &nxp_csi_internal_ops;
    strlcpy(sd->name, "NXP MIPI CSI", sizeof(sd->name));
    sd->grp_id = NXP_CAPTURE_SUBDEV_GROUP_ID;
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pads[NXP_CSI_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_CSI_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

    entity->ops = &nxp_csi_media_ops;
    ret = media_entity_init(entity, NXP_CSI_PAD_MAX, pads, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

    return 0;
}

/*
 * public api
 */
int nxp_csi_init(struct nxp_csi *me, struct nxp_mipi_csi_platformdata *platdata)
{
    int ret = _init_entities(me);
    if (!ret) {
        me->platdata = platdata;
        me->module = platdata->module;
        /* me->irq = platdata->irq; */
        /* me->regs = platdata->base; */
        me->irq = NX_MIPI_GetInterruptNumber(me->module);
#ifdef CONFIG_ARCH_S5P6818
        me->irq += 32;
#endif
        me->regs = (void __iomem *)IO_ADDRESS(NX_MIPI_GetPhysicalAddress(me->module));
    }

    return ret;
}

void nxp_csi_cleanup(struct nxp_csi *me)
{
}

int nxp_csi_register(struct nxp_csi *me)
{
    int ret;
    struct nxp_capture *parent = nxp_csi_to_parent(me);

#if 0
    ret = request_irq(me->irq, nxp_csi_irq_handler, 0, "nxp-csi", me);
    if (ret) {
        pr_err("%s: failed to request_irq()\n", __func__);
        return ret;
    }
#endif

    ret = v4l2_device_register_subdev(parent->get_v4l2_device(parent),
            &me->subdev);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_register_subdev()\n", __func__);
        return ret;
    }

    vmsg("%s: success!!!\n", __func__);

    return 0;
}

void nxp_csi_unregister(struct nxp_csi *me)
{
    free_irq(me->irq, me);
    v4l2_device_unregister_subdev(&me->subdev);
}

#ifdef CONFIG_PM
int nxp_csi_suspend(struct nxp_csi *me)
{
    struct v4l2_subdev *remote_source;
    PM_DBGOUT("+%s\n", __func__);
    remote_source = _get_remote_source_subdev(me);
    if (remote_source) {
        v4l2_subdev_call(remote_source, core, s_power, 0);
    }
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

int nxp_csi_resume(struct nxp_csi *me)
{
    PM_DBGOUT("+%s\n", __func__);
    if (NXP_ATOMIC_READ(&me->state) & NXP_CSI_STATE_RUNNING) {
        struct v4l2_subdev *remote_source = _get_remote_source_subdev(me);
        PM_DBGOUT("call subdev s_stream 1\n");
        if (remote_source)
            v4l2_subdev_call(remote_source, video, s_stream, 1);
        PM_DBGOUT("call _hw_run()\n");
        _hw_run(me);
    }
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif
