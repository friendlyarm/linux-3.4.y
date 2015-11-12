#define DEBUG 1

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2_nxp_media.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>

#include "nxp-v4l2.h"
#include "sync-preset.h"
#include "nxp-resc.h"

#include <mach/soc.h>
#include <mach/platform.h>

#define FIXED_POINT     (65536)
#define SOFT_V          (8)
#define SOFT_H          (16)
/**
 * hw functions
 */
static void _resc_intr_callback(void *data)
{
    struct nxp_resc *me = data;
    uint32_t vsw;
    struct dpc_sync_param *dpc_sync = &me->preset->dpc_sync_param;
    me->dynamic_change_count++;
    me->dynamic_change_count %= me->preset->dpc_sync_param.dynamic_change_num;
    vsw = me->preset->dpc_sync_param.dynamic_tbl[me->dynamic_change_count];
    /* printk("%u, ", vsw); */
    udelay(100);
    NX_DPC_SetVSync(me->dpc_module_num,
            dpc_sync->vact,
            vsw,
            dpc_sync->vfp,
            dpc_sync->vbp,
            0,
            dpc_sync->vact,
            vsw,
            dpc_sync->vfp,
            dpc_sync->vbp);
}

static inline void _get_hdmi_width_height(uint32_t preset, uint32_t *width, uint32_t *height)
{
    switch (preset) {
    case V4L2_DV_480P59_94:
    case V4L2_DV_480P60:
        *width = 720;
        *height = 480;
        break;
    case V4L2_DV_576P50:
        *width = 720;
        *height = 576;
        break;
    case V4L2_DV_720P24:
    case V4L2_DV_720P25:
    case V4L2_DV_720P30:
    case V4L2_DV_720P50:
    case V4L2_DV_720P59_94:
    case V4L2_DV_720P60:
    case V4L2_DV_720P60_FP:
    case V4L2_DV_720P60_SB_HALF:
    case V4L2_DV_720P60_TB:
    case V4L2_DV_720P59_94_FP:
    case V4L2_DV_720P59_94_SB_HALF:
    case V4L2_DV_720P59_94_TB:
    case V4L2_DV_720P50_FP:
    case V4L2_DV_720P50_SB_HALF:
    case V4L2_DV_720P50_TB:
        *width = 1280;
        *height = 720;
        break;
    case V4L2_DV_1080I29_97:
    case V4L2_DV_1080I30:
    case V4L2_DV_1080I25:
    case V4L2_DV_1080I50:
    case V4L2_DV_1080I60:
    case V4L2_DV_1080P24:
    case V4L2_DV_1080P25:
    case V4L2_DV_1080P30:
    case V4L2_DV_1080P50:
    case V4L2_DV_1080P60:
    case V4L2_DV_1080I59_94:
    case V4L2_DV_1080P59_94:
    case V4L2_DV_1080P24_FP:
    case V4L2_DV_1080P24_SB_HALF:
    case V4L2_DV_1080P24_TB:
    case V4L2_DV_1080P23_98_FP:
    case V4L2_DV_1080P23_98_SB_HALF:
    case V4L2_DV_1080P23_98_TB:
    case V4L2_DV_1080I60_SB_HALF:
    case V4L2_DV_1080I59_94_SB_HALF:
    case V4L2_DV_1080I50_SB_HALF:
    case V4L2_DV_1080P60_SB_HALF:
    case V4L2_DV_1080P60_TB:
    case V4L2_DV_1080P30_FP:
    case V4L2_DV_1080P30_SB_HALF:
    case V4L2_DV_1080P30_TB:
        *width = 1920;
        *height = 1080;
        break;
    }
}

static int _get_preset(struct nxp_resc *me)
{
    uint32_t mlc_width = me->format[NXP_RESC_PAD_SINK].width;
    uint32_t mlc_height = me->format[NXP_RESC_PAD_SINK].height;
    uint32_t resc_width = me->format[NXP_RESC_PAD_SOURCE].width;
    uint32_t resc_height = me->format[NXP_RESC_PAD_SOURCE].height;
    uint32_t hdmi_width = 0;
    uint32_t hdmi_height = 0;
    _get_hdmi_width_height(me->remote_preset, &hdmi_width, &hdmi_height);

    pr_debug("%s: mlc %dx%d, resc %dx%d, hdmi %dx%d\n",
            __func__, mlc_width, mlc_height, resc_width, resc_height, hdmi_width, hdmi_height);
    me->preset = NULL;
    me->preset = nxp_out_find_sync_preset(mlc_width, mlc_height,
            resc_width, resc_height, hdmi_width, hdmi_height);
    if (me->preset)
        return 0;
    else {
        printk(KERN_ERR "%s: can't find preset!!!\n", __func__);
        return -EINVAL;
    }
}

static int _hw_set_with_preset(struct nxp_resc *me)
{
    u32 sg_vfp, sg_vs, sg_vbp, sg_hfp, sg_hbp, sg_hs;
    u32 src_hsize, src_vsize, dst_hsize, dst_vsize, sg_hav, sg_vav;
    u32 src_crop_l, src_crop_r, src_crop_t, src_crop_b;
    u32 sg_hdelay, sg_hoffset, sg_s2in_vs;
    u32 delta_x, delta_y;

    struct sync_preset *preset = me->preset;

    pr_debug("%s: entered\n", __func__);

    src_hsize  = preset->mlc_resol.width;
    src_vsize  = preset->mlc_resol.height;
    dst_hsize  = preset->resc_sync_param.hact;
    dst_vsize  = preset->resc_sync_param.vact;
    sg_hav = dst_hsize;
    sg_vav = dst_vsize;

    sg_vfp  = preset->resc_sync_param.vfp;
    sg_vs   = preset->resc_sync_param.vsw;
    sg_vbp  = preset->resc_sync_param.vbp;

    sg_hfp  = preset->resc_sync_param.hfp;
    sg_hs   = preset->resc_sync_param.hsw;
    sg_hbp  = preset->resc_sync_param.hbp;

    src_crop_l = 0;
    src_crop_r = 0;
    src_crop_t = 0;
    src_crop_b = 0;
    if (src_hsize >= dst_hsize) {
        // scale down
        pr_debug("%s: scale down\n", __func__);
        delta_x = (src_hsize*FIXED_POINT+dst_hsize/2)/(dst_hsize);
        delta_y = (src_vsize*FIXED_POINT+dst_vsize/2)/(dst_vsize);
    } else {
        // scale up
        pr_debug("%s: scale up\n", __func__);
        delta_x = (src_hsize*FIXED_POINT+dst_hsize/2)/(dst_hsize-1);
        delta_y = (src_vsize*FIXED_POINT+dst_vsize/2)/(dst_vsize-1);
    }

    sg_hdelay  = preset->resc_sync_param.hdelay;
    sg_hoffset = preset->resc_sync_param.hoffset;
    sg_s2in_vs = 0;

    pr_debug("%s: sg_vfp %d, sg_vs %d, sg_vbp %d, sg_hfp %d, sg_hbp %d, sg_hs %d\n",
            __func__, sg_vfp, sg_vs, sg_vbp, sg_hfp, sg_hbp, sg_hs);
    pr_debug("%s: src_hsize %d, src_vsize %d, st_hsize %d, dst_vsize %d, sg_hav %d, sg_vav %d\n",
            __func__, src_hsize, src_vsize, dst_hsize, dst_vsize, sg_hav, sg_vav);
    pr_debug("%s: sg_hdelay %d, sg_hoffset %d, sg_s2in_vs %d\n",
            __func__, sg_hdelay, sg_hoffset, sg_s2in_vs);

#if defined(CONFIG_ARCH_S5P4418)
    NX_RSTCON_SetnRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_nENABLE);
#elif defined(CONFIG_ARCH_S5P6818)
    NX_RSTCON_SetRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_ASSERT);
    NX_RSTCON_SetRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_NEGATE);
#endif

    NX_RESCONV_Initialize();
    NX_RESCONV_SetBaseAddress(me->id,
            (void*)IO_ADDRESS(NX_RESCONV_GetPhysicalAddress(0)));

    NX_RESCONV_FIFO_Init(me->id, CTRUE);
    NX_RESCONV_FIFO_Init(me->id, CFALSE);

    NX_RESCONV_SetS2IN_VS(me->id, CFALSE);

    NX_RESCONV_FINIT(me->id);

    NX_RESCONV_INIT(me->id,
            src_hsize,
            src_vsize,
            src_crop_l,
            src_crop_r,
            src_crop_t,
            src_crop_b,
            dst_hsize,
            dst_vsize,
            FIXED_POINT,
            delta_x,
            delta_y,
            SOFT_V,
            SOFT_H,
            sg_vfp,
            sg_vbp,
            sg_vs,
            sg_vav,
            sg_hfp,
            sg_hbp,
            sg_hs,
            sg_hav,
            sg_s2in_vs,
            sg_hoffset,
            sg_hdelay);

    pr_debug("%s: exit\n", __func__);
    return 0;

}

static int _hw_set(struct nxp_resc *me)
{
    u32 sg_vfp, sg_vs, sg_vbp, sg_hfp, sg_hbp, sg_hs;
    u32 src_hsize, src_vsize, dst_hsize, dst_vsize, sg_hav, sg_vav;
    u32 src_crop_l, src_crop_r, src_crop_t, src_crop_b;
    u32 sg_hdelay, sg_hoffset, sg_s2in_vs;
    u32 delta_x, delta_y;
    u32 v_diff, h_diff;

    src_hsize  = me->format[NXP_RESC_PAD_SINK].width;
    src_vsize  = me->format[NXP_RESC_PAD_SINK].height;
    dst_hsize  = me->format[NXP_RESC_PAD_SOURCE].width;
    dst_vsize = me->format[NXP_RESC_PAD_SOURCE].height;
    sg_hav = dst_hsize;
    sg_vav = dst_vsize;

    v_diff = src_vsize - dst_vsize;
    h_diff = src_hsize - dst_hsize;

    sg_vfp  = 4 + v_diff/2;;
    sg_vs   = 5;
    sg_vbp  = 36 + v_diff/2;

    sg_hfp  = 88 + h_diff/2;
    sg_hs   = 44;
    sg_hbp  = 148 + h_diff/2;

    src_crop_l = 0;
    src_crop_r = 0;
    src_crop_t = 0;
    src_crop_b = 0;
    delta_x = (src_hsize*FIXED_POINT+dst_hsize/2)/(dst_hsize);
    delta_y = (src_vsize*FIXED_POINT+dst_vsize/2)/(dst_vsize);

    sg_hdelay  = 1;
    sg_hoffset = 0;
    sg_s2in_vs = 0;

#if defined(CONFIG_ARCH_S5P4418)
    NX_RSTCON_SetnRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_nENABLE);
#elif defined(CONFIG_ARCH_S5P6818)
    NX_RSTCON_SetRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_ASSERT);
    NX_RSTCON_SetRST(RESETINDEX_OF_RESCONV_MODULE, RSTCON_NEGATE);
#endif

    NX_RESCONV_Initialize();
    NX_RESCONV_SetBaseAddress(me->id,
            (void*)IO_ADDRESS(NX_RESCONV_GetPhysicalAddress(0)));

    NX_RESCONV_FIFO_Init(me->id, CTRUE);
    NX_RESCONV_FIFO_Init(me->id, CFALSE);

    NX_RESCONV_SetS2IN_VS(me->id, CFALSE);

    NX_RESCONV_FINIT(me->id);

    NX_RESCONV_INIT(me->id,
            src_hsize,
            src_vsize,
            src_crop_l,
            src_crop_r,
            src_crop_t,
            src_crop_b,
            dst_hsize,
            dst_vsize,
            FIXED_POINT,
            delta_x,
            delta_y,
            SOFT_V,
            SOFT_H,
            sg_vfp,
            sg_vbp,
            sg_vs,
            sg_vav,
            sg_hfp,
            sg_hbp,
            sg_hs,
            sg_hav,
            sg_s2in_vs,
            sg_hoffset,
            sg_hdelay);

    return 0;
}

static void _hw_run(struct nxp_resc *me)
{
    NX_RESCONV_RUN(me->id);
}

static void _hw_stop(struct nxp_resc *me)
{
    NX_RESCONV_STOP(me->id);
}

/**
 * util functions
 */
static struct v4l2_subdev *
_get_remote_source_subdev(struct nxp_resc *me)
{
    struct media_pad *pad =
        media_entity_remote_source(&me->pads[NXP_RESC_PAD_SINK]);
    if (!pad) {
        pr_err("%s: can't find remote source!!!\n", __func__);
        return NULL;
    }
    return media_entity_to_v4l2_subdev(pad->entity);
}

static struct v4l2_subdev *
_get_remote_sink_subdev(struct nxp_resc *me)
{
    struct media_pad *pad =
        media_entity_remote_source(&me->pads[NXP_RESC_PAD_SOURCE]);
    if (!pad) {
        pr_err("%s: can't find remote sink!!!\n", __func__);
        return NULL;
    }
    return media_entity_to_v4l2_subdev(pad->entity);
}

static struct v4l2_mbus_framefmt *
_get_pad_format(struct nxp_resc *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    if (pad >= NXP_RESC_PAD_MAX)
        return NULL;

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
_get_pad_crop(struct nxp_resc *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    if (pad >= NXP_RESC_PAD_MAX)
        return NULL;

    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_crop(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        return &me->crop[pad];
    default:
        return NULL;
    }
}

static inline unsigned long _get_clk_hz(int preset)
{
    switch (preset) {
    case V4L2_DV_480P59_94:
        return 27000000;
    case V4L2_DV_480P60:
        return 27027000;
    case V4L2_DV_576P50:
        return 27000000;
    case V4L2_DV_720P50:
        return 74250000;
    case V4L2_DV_720P59_94:
        return 74175000;
    case V4L2_DV_720P60:
        return 74250000;
    case V4L2_DV_1080P50:
        return 148500000;
    case V4L2_DV_1080P59_94:
        return 148352000;
    case V4L2_DV_1080P60:
        return 148500000;
    default:
        pr_err("%s: invalid preset 0x%x\n", __func__, preset);
        return 0;
    }
}

// same to hdmi setting part
static int _get_vsync_info(struct nxp_resc *me, int device,
        struct disp_vsync_info *vsync, struct disp_syncgen_par *par)
{
    int width = me->format[NXP_RESC_PAD_SINK].width;
    int height = me->format[NXP_RESC_PAD_SINK].height;

    nxp_soc_disp_device_get_sync_param(device, (void *)par);

    vsync->h_active_len = width;
    vsync->v_active_len = height;

    switch (height) {
    case 1080:
        vsync->h_active_len = 1920;
        vsync->h_sync_width = 44;
        vsync->h_back_porch = 148;
        vsync->h_front_porch = 88;
        vsync->h_sync_invert = 0;
        vsync->v_active_len = 1080;
        vsync->v_sync_width = 5;
        vsync->v_back_porch = 36;
        vsync->v_front_porch = 4;
        vsync->v_sync_invert = 0;
        break;

    case 720:
        vsync->h_active_len = 1280;
        vsync->h_sync_width = 40;
        vsync->h_back_porch = 220;
        vsync->h_front_porch = 110;
        vsync->h_sync_invert = 0;
        vsync->v_active_len = 720;
        vsync->v_sync_width = 5;
        vsync->v_back_porch = 20;
        vsync->v_front_porch = 5;
        vsync->v_sync_invert = 0;
        break;

    case 576:
        vsync->h_active_len = 720;
        vsync->h_sync_width = 64;
        vsync->h_back_porch = 68;
        vsync->h_front_porch = 12;
        vsync->h_sync_invert = 0;
        vsync->v_active_len = 576;
        vsync->v_sync_width = 5;
        vsync->v_back_porch = 39;
        vsync->v_front_porch = 5;
        vsync->v_sync_invert = 0;
        break;

    case 480:
        vsync->h_active_len = 720;
        vsync->h_sync_width = 62;
        vsync->h_back_porch = 60;
        vsync->h_front_porch = 16;
        vsync->h_sync_invert = 0;
        vsync->v_active_len = 480;
        vsync->v_sync_width = 6;
        vsync->v_back_porch = 30;
        vsync->v_front_porch = 9;
        vsync->v_sync_invert = 0;
        break;

    default:
        pr_err("%s: invalid wxh(%dx%d)\n", __func__, width, height);
        return -EINVAL;
    }

    vsync->h_sync_invert = 0;
    vsync->v_sync_invert = 0;

    vsync->interlace = 0;

    vsync->pixel_clock_hz = _get_clk_hz(me->remote_preset);
    vsync->clk_src_lv0 = 4; /* hdmi */
    vsync->clk_div_lv0 = 1;
    vsync->clk_src_lv1 = 4;
    /*vsync->clk_src_lv1 = 7;*/
    vsync->clk_div_lv1 = 1;

    par->out_format = OUTPUTFORMAT_RGB888;
    par->delay_mask = (DISP_SYNCGEN_DELAY_RGB_PVD | DISP_SYNCGEN_DELAY_HSYNC_CP1 |
            DISP_SYNCGEN_DELAY_VSYNC_FRAM | DISP_SYNCGEN_DELAY_DE_CP);
    par->d_rgb_pvd = 0;
    par->d_hsync_cp1 = 0;
    par->d_vsync_fram = 0;
    par->d_de_cp2 = 7;

    //	HFP + HSW + HBP + AVWidth-VSCLRPIXEL- 1;
    par->vs_start_offset = (vsync->h_front_porch + vsync->h_sync_width +
            vsync->h_back_porch + vsync->h_active_len - 1);
    par->vs_end_offset = 0;
    // HFP + HSW + HBP + AVWidth-EVENVSCLRPIXEL- 1
    par->ev_start_offset = (vsync->h_front_porch + vsync->h_sync_width +
            vsync->h_back_porch + vsync->h_active_len - 1);
    par->ev_end_offset = 0;

    return 0;
}

static int _get_vsync_info_with_preset(struct nxp_resc *me, int device,
        struct disp_vsync_info *vsync, struct disp_syncgen_par *par)
{
    struct dpc_sync_param *preset = &me->preset->dpc_sync_param;

    nxp_soc_disp_device_get_sync_param(device, (void *)par);

    vsync->h_active_len      = preset->hact;
    vsync->v_active_len      = preset->vact;
    vsync->h_front_porch     = preset->hfp;
    vsync->h_sync_width      = preset->hsw;
    vsync->h_back_porch      = preset->hbp;
    vsync->v_front_porch     = preset->vfp;
    vsync->v_sync_width      = preset->vsw;
    vsync->v_back_porch      = preset->vbp;

    vsync->h_sync_invert = 0;
    vsync->v_sync_invert = 0;

    vsync->interlace = 0;

    vsync->pixel_clock_hz = _get_clk_hz(me->remote_preset);
    vsync->clk_src_lv0 = preset->clk_src_lv0;
    vsync->clk_div_lv0 = preset->clk_div_lv0;
    vsync->clk_src_lv1 = preset->clk_src_lv1;
    vsync->clk_div_lv1 = preset->clk_div_lv1;

    par->out_format = OUTPUTFORMAT_RGB888;
    par->delay_mask = (DISP_SYNCGEN_DELAY_RGB_PVD | DISP_SYNCGEN_DELAY_HSYNC_CP1 |
            DISP_SYNCGEN_DELAY_VSYNC_FRAM | DISP_SYNCGEN_DELAY_DE_CP);
    par->d_rgb_pvd = 0;
    par->d_hsync_cp1 = 0;
    par->d_vsync_fram = 0;
    par->d_de_cp2 = 7;

    par->vs_start_offset = 0;
    par->vs_end_offset = 0;
    par->ev_start_offset = 0;
    par->ev_end_offset = 0;

    return 0;
}

static int _set_remote_sync(struct nxp_resc *me)
{
    int ret;
    struct disp_vsync_info vsync;
    struct disp_syncgen_par param;
    int source_device;
    struct v4l2_subdev *remote_source;

    remote_source = _get_remote_source_subdev(me);
    if (!remote_source) {
        pr_err("%s: can't get remote source\n", __func__);
        return -EIO;
    }

    if (!strncmp(remote_source->name, "NXP MLC", 7)) {
        char *numchar = remote_source->name + 7;
        int module = *numchar - '0';
        if (module == 0) {
            source_device = DISP_DEVICE_SYNCGEN0;
            NX_DISPLAYTOP_SetRESCONVMUX(CTRUE, PrimaryMLC);
        } else {
            source_device = DISP_DEVICE_SYNCGEN1;
            NX_DISPLAYTOP_SetRESCONVMUX(CTRUE, SecondaryMLC);
        }
        me->dpc_module_num = module;
        pr_debug("%s: dpc module num %d\n", __func__, module);

        if (me->preset) {
            ret = _get_vsync_info_with_preset(me, source_device, &vsync, &param);
            if (ret) {
                pr_err("%s: failed to _get_vsync_info_with_preset()\n", __func__);
                return ret;
            }
        } else {
            ret = _get_vsync_info(me, source_device, &vsync, &param);
            if (ret) {
                pr_err("%s: failed to _get_vsync_info()\n", __func__);
                return ret;
            }
        }

        pr_debug("%s: RESC Source Device 0x%x\n", __func__, source_device);
        ret = nxp_soc_disp_device_set_sync_param(source_device, (void *)&param);
        if (ret) {
            pr_err("%s: failed to nxp_soc_disp_device_set_sync_param()\n", __func__);
            return ret;
        }

        me->dynamic_change_count = 0;
        if (me->preset && me->preset->dpc_sync_param.use_dynamic)
            me->callback = nxp_soc_disp_register_irq_callback(me->dpc_module_num, _resc_intr_callback, me);

        ret = nxp_soc_disp_device_connect_to(DISP_DEVICE_RESCONV, source_device, &vsync);
        if (ret) {
            pr_err("%s: failed to connect to source\n", __func__);
            return ret;
        }

        me->source_device = source_device;
        return nxp_soc_disp_device_enable(source_device, 1);
    } else {
        pr_err("%s: invalid source %s\n", __func__, remote_source->name);
        return -EINVAL;
    }
}

static void _set_resc_clkgen_pre(struct nxp_resc *me, bool enable, bool sink_is_hdmi)
{
    NX_DISPTOP_CLKGEN_SetBaseAddress(ResConv_CLKGEN,
            (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(ResConv_CLKGEN)));
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ResConv_CLKGEN, CFALSE);
    if (enable) {
        NX_DISPTOP_CLKGEN_SetClockPClkMode(ResConv_CLKGEN, NX_PCLKMODE_ALWAYS);
        if (sink_is_hdmi) {
            NX_DISPTOP_CLKGEN_SetClockSource(ResConv_CLKGEN, ResConv_ICLK, 4); // HDMI CLK
            NX_DISPTOP_CLKGEN_SetClockDivisor(ResConv_CLKGEN, ResConv_ICLK, 1);
        } else {
            /* TODO : select iclk by sink */
            pr_err("%s: TODO ==> select iclk for not hdmi\n", __func__);
        }
        NX_DISPTOP_CLKGEN_SetClockSource(ResConv_CLKGEN, ResConv_CCLK, 2); // PLL2
        NX_DISPTOP_CLKGEN_SetClockDivisor(ResConv_CLKGEN, ResConv_CCLK, 1);
        NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ResConv_CLKGEN, CTRUE);

        NX_DISPTOP_CLKGEN_SetBaseAddress(LCDIF_CLKGEN,
                (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(LCDIF_CLKGEN)));
        NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CFALSE);
        NX_DISPTOP_CLKGEN_SetClockPClkMode      ( LCDIF_CLKGEN, NX_PCLKMODE_ALWAYS );
        NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CFALSE );
        NX_DISPTOP_CLKGEN_SetClockSource        ( LCDIF_CLKGEN, 0, 4 );
        NX_DISPTOP_CLKGEN_SetClockDivisor       ( LCDIF_CLKGEN, 0, 1 );
        NX_DISPTOP_CLKGEN_SetClockSource        ( LCDIF_CLKGEN, 1, 7 );
        NX_DISPTOP_CLKGEN_SetClockDivisor       ( LCDIF_CLKGEN, 1, 1 );
        NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CTRUE);
    } else {
        NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CFALSE);
        NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ResConv_CLKGEN, CFALSE);
    }
}

/* default : clk_src 0, clk_divide 4 */
static void _set_resc_clkgen_post(struct nxp_resc *me, int clk_src, int clk_divide)
{
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ResConv_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockSource(ResConv_CLKGEN, ResConv_CCLK, clk_src);
    NX_DISPTOP_CLKGEN_SetClockDivisor(ResConv_CLKGEN, ResConv_CCLK, clk_divide);
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ResConv_CLKGEN, CTRUE);
}

/**
 * v4l2_subdev_core_ops
 */
static int nxp_resc_s_power(struct v4l2_subdev *sd, int on)
{
    pr_debug("%s: %d\n", __func__, on);
    return 0;
}

static const struct v4l2_subdev_core_ops nxp_resc_core_ops = {
    .s_power = nxp_resc_s_power,
};

/**
 * v4l2_subdev_video_ops
 */
static int nxp_resc_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct nxp_resc *me;
    struct v4l2_subdev *remote_sink;

    me = v4l2_get_subdevdata(sd);
    remote_sink = _get_remote_sink_subdev(me);

    printk("%s: enable %d\n", __func__, enable);

    if (enable) {
        if (!me->streaming) {
            _set_resc_clkgen_pre(me, true, true);

            if (remote_sink) {
                struct v4l2_dv_preset preset;
                // test
#if 0
                {
                    extern void hdmi_set_hsync_offset(int offset);
                    hdmi_set_hsync_offset(-100);
                }
#endif
                v4l2_subdev_call(remote_sink, video, s_stream, 1);
                v4l2_subdev_call(remote_sink, video, g_dv_preset, &preset);
                me->remote_preset = preset.preset;
                pr_debug("%s: remote_preset 0x%x\n", __func__, me->remote_preset);
            }

            if (_get_preset(me) < 0) {
                printk(KERN_ERR "%s: failed to _get_preset()\n", __func__);
                _hw_set(me);
                // if downscale, use hdmi clk
                _set_resc_clkgen_post(me, 4, 1);
            } else {
                _set_remote_sync(me);
                _hw_set_with_preset(me);
                // if upscale, use max 250MHz clk
                if (me->preset->mlc_resol.width > me->preset->resc_resol.width) {
                    // scale down
                    pr_debug("scale down clk set\n");
                    _set_resc_clkgen_post(me, 4, 1);
                } else {
                    // scale up
                    // TODO: find source clock
                    pr_debug("scale up clk set\n");
                    _set_resc_clkgen_post(me, 0, 4);
                }
            }

            _hw_run(me);
            me->streaming = true;

            {
                int count = 10;
                do {
                    mdelay(5);
                    printk("%s: over %d, under %d\n",
                            __func__,
                            NX_RESCONV_IsOverFlow(0),
                            NX_RESCONV_IsUnderFlow(0));
                    count--;
                } while (count > 0);
            }
        }
    } else {
        if (me->streaming) {
            if (remote_sink)
                v4l2_subdev_call(remote_sink, video, s_stream, 0);

            _hw_stop(me);
            NX_RESCONV_SetS2IN_VS(me->id, CTRUE);
            _set_resc_clkgen_pre(me, false, true);

            nxp_soc_disp_device_enable(me->source_device, 0);
            nxp_soc_disp_device_disconnect(DISP_DEVICE_RESCONV, me->source_device);
            if (me->preset && me->preset->dpc_sync_param.use_dynamic) {
                nxp_soc_disp_unregister_irq_callback(me->dpc_module_num, me->callback);
                me->callback = NULL;
            }

            me->streaming = false;
        }
    }

    return 0;
}

static const struct v4l2_subdev_video_ops nxp_resc_video_ops = {
    .s_stream = nxp_resc_s_stream,
};

/**
 * v4l2_subdev_pad_ops
 */
static int nxp_resc_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_resc *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *_fmt = _get_pad_format(me, fh, format->pad, format->which);
    if (!_fmt)
        return -EINVAL;
    format->format = *_fmt;
    return 0;
}

static int nxp_resc_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *format)
{
    struct nxp_resc *me = v4l2_get_subdevdata(sd);
    struct v4l2_mbus_framefmt *_fmt = _get_pad_format(me, fh, format->pad, format->which);
    if (!_fmt)
        return -EINVAL;
    printk("%s\n", __func__);
    *_fmt = format->format;
    return 0;
}

static int nxp_resc_get_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_resc *me = v4l2_get_subdevdata(sd);
    struct v4l2_rect *_crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    if (!_crop)
        return -EINVAL;
    crop->rect = *_crop;
    return 0;
}

static int nxp_resc_set_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct nxp_resc *me = v4l2_get_subdevdata(sd);
    struct v4l2_rect *_crop = _get_pad_crop(me, fh, crop->pad, crop->which);
    if (!_crop)
        return -EINVAL;
    *_crop = crop->rect;
    return 0;
}

static const struct v4l2_subdev_pad_ops nxp_resc_pad_ops = {
    .get_fmt    = nxp_resc_get_fmt,
    .set_fmt    = nxp_resc_set_fmt,
    .get_crop   = nxp_resc_get_crop,
    .set_crop   = nxp_resc_set_crop,
};

/**
 * v4l2_subdev_ops
 */
static const struct v4l2_subdev_ops nxp_resc_subdev_ops = {
    .core   = &nxp_resc_core_ops,
    .video  = &nxp_resc_video_ops,
    .pad    = &nxp_resc_pad_ops,
};

/**
 * media_entity_ops
 */
static int nxp_resc_link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    switch (local->index | media_entity_type(remote->entity)) {
        case NXP_RESC_PAD_SINK | MEDIA_ENT_T_V4L2_SUBDEV:
            if (flags & MEDIA_LNK_FL_ENABLED)
                printk("%s: mlc connect to me\n", __func__);
            else
                printk("%s: mlc disconnect to me\n", __func__);
            break;

        case NXP_RESC_PAD_SOURCE | MEDIA_ENT_T_V4L2_SUBDEV:
            if (flags & MEDIA_LNK_FL_ENABLED)
                printk("%s: I connect to HDMI\n", __func__);
            else
                printk("%s: I disconnect to HDMI\n", __func__);
            break;

        default:
            pr_err("%s: invalid link\n", __func__);
            return -EINVAL;
    }

    return 0;
}

static const struct media_entity_operations nxp_resc_media_ops = {
    .link_setup = nxp_resc_link_setup,
};

/**
 * init entity
 */
static int _init_entities(struct nxp_resc *me)
{
    int ret;
    struct v4l2_subdev *sd = &me->subdev;
    struct media_pad *pads = me->pads;
    struct media_entity *entity = &sd->entity;

    v4l2_subdev_init(sd, &nxp_resc_subdev_ops);

    snprintf(sd->name, sizeof(sd->name), "NXP RESC%d", me->id);
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pads[NXP_RESC_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
    pads[NXP_RESC_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

    entity->ops = &nxp_resc_media_ops;
    ret = media_entity_init(entity, NXP_RESC_PAD_MAX, pads, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init(()\n", __func__);
        return ret;
    }

    return 0;
}

/**
 * public api
 */
struct nxp_resc *create_nxp_resc(int id)
{
    int ret;
    struct nxp_resc *me;

    pr_debug("%s\n", __func__);

    me = kzalloc(sizeof(struct nxp_resc), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to alloc me!!!\n", __func__);
        return NULL;
    }

    me->id = id;
    ret = _init_entities(me);

    if (unlikely(ret)) {
        pr_err("%s: failed to _init_entities()\n", __func__);
        kfree(me);
        return NULL;
    }

    me->streaming = false;

    return me;
}

void release_nxp_resc(struct nxp_resc *me)
{
    pr_debug("%s\n", __func__);
    kfree(me);
}

int  register_nxp_resc(struct nxp_resc *me)
{
    int ret;
    pr_debug("%s\n", __func__);
    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(), &me->subdev);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    return 0;
}

void unregister_nxp_resc(struct nxp_resc *me)
{
    pr_debug("%s\n", __func__);
    v4l2_device_unregister_subdev(&me->subdev);
}

#ifdef CONFIG_PM
int  suspend_nxp_resc(struct nxp_resc *me)
{
    PM_DBGOUT("%s\n", __func__);
    return 0;
}

int  resume_nxp_resc(struct nxp_resc *me)
{
    PM_DBGOUT("%s\n", __func__);
    return 0;
}
#endif
