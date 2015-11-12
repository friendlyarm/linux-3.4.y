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

#include <mach/platform.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>

/* for prototype */
#include <nx_hdmi.h>
#include <nx_rstcon.h>
#include <nx_displaytop.h>
#include <nx_disptop_clkgen.h>
#include <nx_ecid.h>
#include <nx_tieoff.h>

#include "nxp-v4l2.h"

#include "regs-hdmi.h"
#include "hdmi-priv.h"
#include "hdmi-preset.h"
#include "nxp-hdmiphy.h"
#include "nxp-edid.h"
#include "nxp-hdcp.h"

#include "nxp-hdmi.h"

extern int test_hdmi(void);

#define DEFAULT_SAMPLE_RATE	    44100
#define DEFAULT_BITS_PER_SAMPLE	16
#define DEFAULT_AUDIO_CODEC	    HDMI_AUDIO_PCM

#define subdev_to_nxp_hdmi(sd) \
    container_of(sd, struct nxp_hdmi, sd);

/**
 * Initialize Sequence
 * 1.  release the reset of DisplayTop.i_Top_nRST (0xc0012000[9] in Table1‑4).
 * 2.  set the HDMI CLKGEN's PCLKMODE (0xC0108000) to 1 (always enabled).
 * 3.  set the 0xC001100C[0] to 1 (users must maintain the value for all bits except the specific bit)
 * 4.  release the resets of HDMI.i_PHY_nRST and HDMI.i_nRST
 * 5.  set up the HDMI PHY to specific video clock.
 * 6.  I2S (or SPDIFTX) configuration for the source audio data
 *     Users must note the sampling frequency of the audio data. (for example, assume that the sampling frequency is 44 kHz, and using I2S)
 * 7.  Wait for ECID ready
 * 8.  release the resets of HDMI.i_VIDEO_nRST and HDMI.i_SPDIF_nRST and HDMI.i_TMDS_nRST
 *     Set the MIPI CLKGEN 0’s clock frequency to 22.5 MHz ( 44kHz * 64 * 8 )
 * 9.  Wait for HDMI PHY ready (wait until 0xC0200020.[0], 1)
 * 10. MLC and DPC configuration for HDMI Video. Following is a example for HDMI specification.
 *     Set the DPC CLKGEN’s Source Clock to HDMI_CLK.
 * 11. Set up the HDMI Converter parameters. Following table is shown the example values for calculating parameters.
 * 12. Set up the HDMI Link registers. (Using Table1‑8)
 */

/**
 * low level function
 */
static inline bool _hdmi_initialize(struct nxp_hdmi *me)
{
    if (!me->initialized) {
        /**
         * [SEQ 1] release the reset of DisplayTop.i_Top_nRst)
         */
#if 0
        NX_RSTCON_SetnRST(NX_DISPLAYTOP_GetResetNumber(), RSTCON_nDISABLE);
        NX_RSTCON_SetnRST(NX_DISPLAYTOP_GetResetNumber(), RSTCON_nENABLE);
#endif

        /**
         * [SEQ 2] set the HDMI CLKGEN's PCLKMODE to always enabled
         */
        NX_DISPTOP_CLKGEN_SetBaseAddress(HDMI_CLKGEN,
                (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(HDMI_CLKGEN)));
        NX_DISPTOP_CLKGEN_SetClockPClkMode(HDMI_CLKGEN, NX_PCLKMODE_ALWAYS);

        NX_HDMI_SetBaseAddress(0, (void*)IO_ADDRESS(NX_HDMI_GetPhysicalAddress(0)));
        NX_HDMI_Initialize();

        /**
         * [SEQ 3] set the 0xC001100C[0] to 1
         */
        /* NX_TIEOFF_Initialize(); */
        NX_TIEOFF_SetBaseAddress((void*)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress()));
        NX_TIEOFF_Set(TIEOFFINDEX_OF_DISPLAYTOP0_i_HDMI_PHY_REFCLK_SEL, 1);

        /**
         * [SEQ 4] release the resets of HDMI.i_PHY_nRST and HDMI.i_nRST
         */
#if 1
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nDISABLE);
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nDISABLE);

        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE);
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nENABLE);
#else
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE);
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nENABLE);

        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE);
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nENABLE);
#endif

        me->initialized = true;

        /**
         * Next sequence start in _hdmi_streamon()
         */
    }

    return true;
}

static inline bool _wait_for_ecid_ready(void)
{
    int retry_count = 100;
    bool is_key_ready = false;

    NX_ECID_SetBaseAddress((void*)IO_ADDRESS(NX_ECID_GetPhysicalAddress()));

    do {
        is_key_ready = NX_ECID_GetKeyReady();
        if (is_key_ready) break;
        msleep(1);
        retry_count--;
    } while (retry_count);

    return is_key_ready;
}

static inline void _hdmi_reset(struct nxp_hdmi *me)
{
#if 1
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nDISABLE);

    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nENABLE);
#else
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nENABLE);

    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nENABLE);
#endif
}

static inline bool _wait_for_hdmiphy_ready(void)
{
    // TODO test
    int retry_count = 500;
    do {
        u32 regval = NX_HDMI_GetReg(0, HDMI_LINK_PHY_STATUS_0);
        /* printk("====> PHY: 0x%x\n", regval); */
        if (regval & 0x01) {
            printk("HDMI PHY Ready!!!\n");
            return true;
        }
        mdelay(10);
        retry_count--;
    } while (retry_count);

    return false;
}

static struct v4l2_subdev *
_get_remote_source_subdev(struct nxp_hdmi *me)
{
    struct media_pad *pad =
        media_entity_remote_source(&me->pad);
    if (!pad)
        return NULL;
    return media_entity_to_v4l2_subdev(pad->entity);
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

static int _get_vsync_info(int preset, int device,
				struct disp_vsync_info *vsync, struct disp_syncgen_par *par)
{
	nxp_soc_disp_device_get_sync_param(device, (void*)par);

    /**
     * reference : CEA-861D.pdf
     */
    switch (preset) {
    case V4L2_DV_480P59_94:
    case V4L2_DV_480P60:
        /* 480p: 720x480 */
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

    case V4L2_DV_576P50:
        /* 576p: 720x576 */
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

    case V4L2_DV_720P50:
    case V4L2_DV_720P59_94:
    case V4L2_DV_720P60:
        /* 720p: 1280x720 */
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

    case V4L2_DV_1080P50:
    case V4L2_DV_1080P59_94:
    case V4L2_DV_1080P60:
        /* 1080p: 1920x1080 */
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

    default:
        pr_err("%s: invalid preset value 0x%x\n", __func__, preset);
        return -EINVAL;
    }

    vsync->pixel_clock_hz = _get_clk_hz(preset);
    vsync->clk_src_lv0 = 4;
    vsync->clk_div_lv0 = 1;
    vsync->clk_src_lv1 = 7;
    vsync->clk_div_lv1 = 1;

	/*
	 * EX. 1280 * 720
	 */
	par->out_format	= OUTPUTFORMAT_RGB888;
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

static int _set_remote_sync(struct nxp_hdmi *me)
{
    int ret;
    struct disp_vsync_info vsync;
    struct disp_syncgen_par param;
    int source_device;
    struct v4l2_subdev *remote;

    remote = _get_remote_source_subdev(me);
    if (!remote) {
        pr_err("%s: can't get remote subdev\n", __func__);
        return -EIO;
    }

    if (!strncmp(remote->name, "NXP MLC", 7)) {
        /* connect DPC -> HDMI */
        char *numchar = remote->name + 7;
        int module = *numchar - '0';
        if (module == 0) {
            source_device = DISP_DEVICE_SYNCGEN0;
            NX_DISPLAYTOP_SetHDMIMUX(CTRUE, PrimaryMLC);
            me->source_dpc_module_num = 0;
        } else {
            source_device = DISP_DEVICE_SYNCGEN1;
            NX_DISPLAYTOP_SetHDMIMUX(CTRUE, SecondaryMLC);
            me->source_dpc_module_num = 1;
        }

        ret = _get_vsync_info(me->cur_preset, source_device, &vsync, &param);
        if (ret) {
            pr_err("%s: failed to _get_vsync_info()\n", __func__);
            return ret;
        }

        vsync.interlace = me->cur_conf->mbus_fmt.field == V4L2_FIELD_INTERLACED;

        pr_debug("%s: HDMI Source Device 0x%x\n", __func__, source_device);
        ret = nxp_soc_disp_device_set_sync_param(source_device, (void*)&param);
        if (ret) {
            pr_err("%s: failed to display parameter....\n", __func__);
            return ret;
        }

        ret = nxp_soc_disp_device_connect_to(DISP_DEVICE_HDMI, source_device, &vsync);
        if (ret) {
            pr_err("%s: failed to connect to source\n", __func__);
            return ret;
        }

        me->source_device = source_device;

        return nxp_soc_disp_device_enable(source_device, 1);
    } else {
        /* connect DPC -> RESC -> HDMI */
        source_device = DISP_DEVICE_RESCONV;
        NX_DISPLAYTOP_SetHDMIMUX(CTRUE, ResolutionConv);
        me->source_device = source_device;
        return 0;
    }
}

static int _set_hdmi_clkgen(struct nxp_hdmi *me)
{
    // TODO : use get_clk(), set_clk() interface
    NX_DISPTOP_CLKGEN_SetBaseAddress(ToMIPI_CLKGEN,
            (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(ToMIPI_CLKGEN)));
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ToMIPI_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode(ToMIPI_CLKGEN, NX_PCLKMODE_ALWAYS);
    // TODO select source & divider
    NX_DISPTOP_CLKGEN_SetClockSource(ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, 2); // pll2
    NX_DISPTOP_CLKGEN_SetClockDivisor(ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, 2);
    NX_DISPTOP_CLKGEN_SetClockSource(ToMIPI_CLKGEN, 1, 7);
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ToMIPI_CLKGEN, CTRUE);
    return 0;
}

static int _set_audio_clkgen(struct nxp_hdmi *me)
{
    // TODO : use get_clk(), set_clk() interface
    NX_DISPTOP_CLKGEN_SetBaseAddress(ToMIPI_CLKGEN,
            (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(ToMIPI_CLKGEN)));
    NX_DISPTOP_CLKGEN_SetClockPClkMode(ToMIPI_CLKGEN, NX_PCLKMODE_ALWAYS);
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ToMIPI_CLKGEN, CTRUE);
    return 0;
}

static int _hdmi_sync_init(struct nxp_hdmi *me)
{
    NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd(0, 0);
    NX_DISPLAYTOP_HDMI_SetVSyncStart(0);
    NX_DISPLAYTOP_HDMI_SetHActiveStart(0);
    NX_DISPLAYTOP_HDMI_SetHActiveEnd(0);
    return 0;
}

struct hdmi_config {
    u32 hfp;
    u32 hsw;
    u32 hbp;
    u32 vfp;
    u32 vsw;
    u32 vbp;
#if 0
    u32 vact;
    u32 v2_blank;
    u32 v_blank;
    u32 v_actline;
    u32 v_sync_line_bef_1;
    u32 v_sync_line_bef_2;
    u32 hact;
    u32 h_line;
    u32 h_blank;
    u32 h_sync_start;
    u32 h_sync_end;
    u32 v_sync_line_aft_1;
    u32 v_sync_line_aft_2;
    u32 v_sync_line_aft_3;
    u32 v_sync_line_aft_4;
    u32 v_sync_line_aft_5;
    u32 v_sync_line_aft_6;
    u32 v_sync_line_aft_pxl_1;
    u32 v_sync_line_aft_pxl_2;
    u32 v_sync_line_aft_pxl_3;
    u32 v_sync_line_aft_pxl_4;
    u32 v_sync_line_aft_pxl_5;
    u32 v_sync_line_aft_pxl_6;
    u32 v_act_space_1;
    u32 v_act_space_2;
    u32 v_act_space_3;
    u32 v_act_space_4;
    u32 v_act_space_5;
    u32 v_act_space_6;
#endif
};

static struct hdmi_config config_720p __attribute__((unused)) = {
    .hfp                = 110,
    .hsw                = 40,
    .hbp                = 220,
    .vfp                = 5,
    .vsw                = 5,
    .vbp                = 20,
#if 0
    .vact               = 720,
    .v2_blank           = 750,
    .v_blank            = 30,
    .v_actline          = 720,
    .v_sync_line_bef_1  = 5,
    .v_sync_line_bef_2  = 10,
    .hact               = 1280,
    .h_line             = 1650,
    .h_blank            = 370,
    .h_sync_start       = 110,
    .h_sync_end         = 150,
    .v_sync_line_aft_1  = 0xffff,
    .v_sync_line_aft_2  = 0xffff,
    .v_sync_line_aft_3  = 0xffff,
    .v_sync_line_aft_4  = 0xffff,
    .v_sync_line_aft_5  = 0xffff,
    .v_sync_line_aft_6  = 0xffff,
    .v_sync_line_aft_pxl_1  = 0xffff,
    .v_sync_line_aft_pxl_2  = 0xffff,
    .v_sync_line_aft_pxl_3  = 0xffff,
    .v_sync_line_aft_pxl_4  = 0xffff,
    .v_sync_line_aft_pxl_5  = 0xffff,
    .v_sync_line_aft_pxl_6  = 0xffff,
    .v_act_space_1      = 0xffff,
    .v_act_space_2      = 0xffff,
    .v_act_space_3      = 0xffff,
    .v_act_space_4      = 0xffff,
    .v_act_space_5      = 0xffff,
    .v_act_space_6      = 0xffff,
#endif
};

static struct hdmi_config config_1080p __attribute__((unused)) = {
    .hfp                = 88,
    .hsw                = 44,
    .hbp                = 148,
    .vfp                = 4,
    .vsw                = 5,
    .vbp                = 36,
#if 0
    .vact               = 1080,
    .v2_blank           = 1125,
    .v_blank            = 45,
    .v_actline          = 1080,
    .v_sync_line_bef_1  = 4,
    .v_sync_line_bef_2  = 9,
    .hact               = 1920,
    .h_line             = 2200,
    .h_blank            = 280,
    .h_sync_start       = 88,
    .h_sync_end         = 132,
    .v_sync_line_aft_1  = 0xffff,
    .v_sync_line_aft_2  = 0xffff,
    .v_sync_line_aft_3  = 0xffff,
    .v_sync_line_aft_4  = 0xffff,
    .v_sync_line_aft_5  = 0xffff,
    .v_sync_line_aft_6  = 0xffff,
    .v_sync_line_aft_pxl_1  = 0xffff,
    .v_sync_line_aft_pxl_2  = 0xffff,
    .v_sync_line_aft_pxl_3  = 0xffff,
    .v_sync_line_aft_pxl_4  = 0xffff,
    .v_sync_line_aft_pxl_5  = 0xffff,
    .v_sync_line_aft_pxl_6  = 0xffff,
    .v_act_space_1      = 0xffff,
    .v_act_space_2      = 0xffff,
    .v_act_space_3      = 0xffff,
    .v_act_space_4      = 0xffff,
    .v_act_space_5      = 0xffff,
    .v_act_space_6      = 0xffff,
#endif
};

/* TODO */
static int _config_hdmi(struct nxp_hdmi *me)
{
    /* struct hdmi_config *config; */

    u32 width;
    u32 height;
    u32 hfp;
    u32 hsw;
    u32 hbp;
    u32 vfp;
    u32 vsw;
    u32 vbp;

    u32 h_blank;
    u32 v_blank;
    u32 v_actline;
    u32 v2_blank;
    u32 v_line;
    u32 h_line;
    u32 h_sync_start;
    u32 h_sync_end;
    u32 v_sync_line_bef_1;
    u32 v_sync_line_bef_2;

    u32 fixed_ffff = 0xffff;

    switch (me->cur_conf->mbus_fmt.height) {
    case 480:
        width = 720;
        height = 480;
        hfp = 16;
        hsw = 62;
        hbp = 60;
        vfp = 9;
        vsw = 6;
        vbp = 30;
        break;
    case 576:
        width = 720;
        height = 576;
        hfp = 12;
        hsw = 64;
        hbp = 68;
        vfp = 5;
        vsw = 5;
        vbp = 39;
        break;
    case 720:
        /* config = &config_720p; */
        printk("%s: 720p\n", __func__);
        width = 1280;
        height = 720;
        hfp = 110;
        hsw = 40;
        hbp = 220;
        vfp = 5;
        vsw = 5;
        vbp = 20;
        break;
    case 1080:
        /* config = &config_1080p; */
        printk("%s: 1080p\n", __func__);
        width = 1920;
        height = 1080;
        hfp = 88;
        hsw = 44;
        hbp = 148;
        vfp = 4;
        vsw = 5;
        vbp = 36;
        break;
    default:
        pr_err("%s: not supported height %d\n", __func__, me->cur_conf->mbus_fmt.height);
        return -EINVAL;
    }

    me->width = width;
    me->height = height;

    /**
     * calculate sync variables
     */
    h_blank = hfp + hsw + hbp;
    v_blank = vfp + vsw + vbp;
    v_actline = height;
    v2_blank = height + vfp + vsw + vbp;
    v_line = height + vfp + vsw + vbp; /* total v */
    h_line = width + hfp + hsw + hbp;  /* total h */
    h_sync_start = hfp;
    h_sync_end = hfp + hsw;
    v_sync_line_bef_1 = vfp;
    v_sync_line_bef_2 = vfp + vsw;

    /* for debugging */
    /* printk("h_blank %d, v_blank %d, v_actline %d, v2_blank %d, v_line %d, h_line %d, h_sync_start %d, h_sync_end %d, v_sync_line_bef_1 %d, v_sync_line_bef_2 %d\n", */
    /*         h_blank, v_blank, v_actline, v2_blank, v_line, h_line, h_sync_start, h_sync_end, v_sync_line_bef_1, v_sync_line_bef_2); */

    /* no blue screen mode, encoding order as it is */
    NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_0, (0<<5)|(1<<4));

    /* set HDMI_LINK_BLUE_SCREEN_* to 0x0 */
    NX_HDMI_SetReg(0, HDMI_LINK_BLUE_SCREEN_R_0, 0x5555);
    NX_HDMI_SetReg(0, HDMI_LINK_BLUE_SCREEN_R_1, 0x5555);
    NX_HDMI_SetReg(0, HDMI_LINK_BLUE_SCREEN_G_0, 0x5555);
    NX_HDMI_SetReg(0, HDMI_LINK_BLUE_SCREEN_G_1, 0x5555);
    NX_HDMI_SetReg(0, HDMI_LINK_BLUE_SCREEN_B_0, 0x5555);
    NX_HDMI_SetReg(0, HDMI_LINK_BLUE_SCREEN_B_1, 0x5555);

    /* set HDMI_CON_1 to 0x0 */
    NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_1, 0x0);
    NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_2, 0x0);

    /* set interrupt : enable hpd_plug, hpd_unplug */
    NX_HDMI_SetReg(0, HDMI_LINK_INTC_CON_0, (1<<6)|(1<<3)|(1<<2));

    /* set STATUS_EN to 0x17 */
    NX_HDMI_SetReg(0, HDMI_LINK_STATUS_EN, 0x17);

    /* TODO set HDP to 0x0 : later check hpd */
    NX_HDMI_SetReg(0, HDMI_LINK_HPD, 0x0);

    /* set MODE_SEL to 0x02 */
    NX_HDMI_SetReg(0, HDMI_LINK_MODE_SEL, 0x2);

    /* set H_BLANK_*, V1_BLANK_*, V2_BLANK_*, V_LINE_*, H_LINE_*, H_SYNC_START_*, H_SYNC_END_ *
     * V_SYNC_LINE_BEF_1_*, V_SYNC_LINE_BEF_2_*
     */
    // psw0523 test
    NX_HDMI_SetReg(0, HDMI_LINK_H_BLANK_0, h_blank%256);
    NX_HDMI_SetReg(0, HDMI_LINK_H_BLANK_1, h_blank>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V1_BLANK_0, v_blank%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V1_BLANK_1, v_blank>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V2_BLANK_0, v2_blank%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V2_BLANK_1, v2_blank>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_LINE_0, v_line%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_LINE_1, v_line>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_H_LINE_0, h_line%256);
    NX_HDMI_SetReg(0, HDMI_LINK_H_LINE_1, h_line>>8);

    if (width == 1280) {
        NX_HDMI_SetReg(0, HDMI_LINK_HSYNC_POL, 0x1);
        NX_HDMI_SetReg(0, HDMI_LINK_VSYNC_POL, 0x1);
    } else {
        NX_HDMI_SetReg(0, HDMI_LINK_HSYNC_POL, 0x0);
        NX_HDMI_SetReg(0, HDMI_LINK_VSYNC_POL, 0x0);
    }

    NX_HDMI_SetReg(0, HDMI_LINK_INT_PRO_MODE, 0x0);

    NX_HDMI_SetReg(0, HDMI_LINK_H_SYNC_START_0, (h_sync_start%256)-2);
    NX_HDMI_SetReg(0, HDMI_LINK_H_SYNC_START_1, h_sync_start>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_H_SYNC_END_0, (h_sync_end%256)-2);
    NX_HDMI_SetReg(0, HDMI_LINK_H_SYNC_END_1, h_sync_end>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_BEF_1_0, v_sync_line_bef_1%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_BEF_1_1, v_sync_line_bef_1>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_BEF_2_0, v_sync_line_bef_2%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_BEF_2_1, v_sync_line_bef_2>>8);


    /* Set V_SYNC_LINE_AFT*, V_SYNC_LINE_AFT_PXL*, VACT_SPACE* */
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_1_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_1_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_2_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_2_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_3_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_3_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_4_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_4_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_5_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_5_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_6_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_6_1, fixed_ffff>>8);

    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_1, fixed_ffff>>8);

    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE1_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE1_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE2_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE2_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE3_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE3_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE4_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE4_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE5_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE5_1, fixed_ffff>>8);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE6_0, fixed_ffff%256);
    NX_HDMI_SetReg(0, HDMI_LINK_VACT_SPACE6_1, fixed_ffff>>8);


    NX_HDMI_SetReg(0, HDMI_LINK_CSC_MUX, 0x0);
    NX_HDMI_SetReg(0, HDMI_LINK_SYNC_GEN_MUX, 0x0);

    NX_HDMI_SetReg(0, HDMI_LINK_SEND_START_0, 0xfd);
    NX_HDMI_SetReg(0, HDMI_LINK_SEND_START_1, 0x01);
    NX_HDMI_SetReg(0, HDMI_LINK_SEND_END_0, 0x0d);
    NX_HDMI_SetReg(0, HDMI_LINK_SEND_END_1, 0x3a);
    NX_HDMI_SetReg(0, HDMI_LINK_SEND_END_2, 0x08);

    /* Set DC_CONTROL to 0x00 */
    NX_HDMI_SetReg(0, HDMI_LINK_DC_CONTROL, 0x0);

    /* Set VIDEO_PATTERN_GEN to 0x00 */
    NX_HDMI_SetReg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x0);
    /* NX_HDMI_SetReg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x1); */

    NX_HDMI_SetReg(0, HDMI_LINK_GCP_CON, 0x0a);

    /* Set HDMI Sync Control parameters */
    me->VSyncStart = vsw + vbp + height - 1;
    me->HActiveStart = hsw + hbp;
    me->HActiveEnd = width + hsw + hbp;
    me->VSyncHSStartEnd0 = hsw + hbp + 1;
    me->VSyncHSStartEnd1 = hsw + hbp + 2;

    /* printk("VSyncStart %d, HActiveStart %d, HActiveEnd %d, VSyncHSStartEnd0 %d, VSyncHSStartEnd1 %d\n", */
    /*         me->VSyncStart, me->HActiveStart, me->HActiveEnd, me->VSyncHSStartEnd0, me->VSyncHSStartEnd1); */

    return 0;
}

/**
 * internal function
 */
static const struct hdmi_preset_conf *_hdmi_preset2conf(u32 preset)
{
    int i;
    for (i = 0; i < hdmi_pre_cnt; ++i) {
        if (hdmi_conf[i].preset == preset)
            return hdmi_conf[i].conf;
    }
    return NULL;
}

static const struct hdmi_3d_info *_hdmi_preset2info(u32 preset)
{
    int i;
    for (i = 0; i < hdmi_pre_cnt; ++i) {
        if (hdmi_conf[i].preset == preset)
            return hdmi_conf[i].info;
    }
    return NULL;
}

static void _hdmi_set_packets(struct nxp_hdmi *me)
{
    hdmi_set_acr(me->sample_rate, me->is_dvi);
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
    u32 hdr_sum;
    u8 chksum;
    u32 aspect_ratio;
    u32 vic;

    pr_debug("%s: infoframe type = 0x%x\n", __func__, infoframe->type);

    if (me->is_dvi) {
        hdmi_writeb(HDMI_VSI_CON, HDMI_VSI_CON_DO_NOT_TRANSMIT);
        hdmi_writeb(HDMI_AVI_CON, HDMI_AVI_CON_DO_NOT_TRANSMIT);
        hdmi_write(HDMI_AUI_CON, HDMI_AUI_CON_NO_TRAN);
        return;
    }

    switch (infoframe->type) {
    case HDMI_PACKET_TYPE_VSI:
        hdmi_writeb(HDMI_VSI_CON, HDMI_VSI_CON_EVERY_VSYNC);
        hdmi_writeb(HDMI_VSI_HEADER0, infoframe->type);
        hdmi_writeb(HDMI_VSI_HEADER1, infoframe->ver);
        /* 0x000C03 : 24bit IEEE Registration Identifier */
        hdmi_writeb(HDMI_VSI_DATA(1), 0x03);
        hdmi_writeb(HDMI_VSI_DATA(2), 0x0c);
        hdmi_writeb(HDMI_VSI_DATA(3), 0x00);
        hdmi_writeb(HDMI_VSI_DATA(4),
                HDMI_VSI_DATA04_VIDEO_FORMAT(info_3d->is_3d));
        hdmi_writeb(HDMI_VSI_DATA(5),
                HDMI_VSI_DATA05_3D_STRUCTURE(info_3d->fmt_3d));
        if (info_3d->fmt_3d == HDMI_3D_FORMAT_SB_HALF) {
            infoframe->len += 1;
            hdmi_writeb(HDMI_VSI_DATA(6),
                    (u8)HDMI_VSI_DATA06_3D_EXT_DATA(HDMI_H_SUB_SAMPLE));
        }
        hdmi_writeb(HDMI_VSI_HEADER2, infoframe->len);
        hdr_sum = infoframe->type + infoframe->ver + infoframe->len;
        chksum = hdmi_chksum(HDMI_VSI_DATA(1), infoframe->len, hdr_sum);
        pr_debug("%s: VSI checksum = 0x%x\n", __func__, chksum);
        hdmi_writeb(HDMI_VSI_DATA(0), chksum);
        break;

    case HDMI_PACKET_TYPE_AVI:
        hdmi_writeb(HDMI_AVI_CON, HDMI_AVI_CON_EVERY_VSYNC);
        hdmi_writeb(HDMI_AVI_HEADER0, infoframe->type);
        hdmi_writeb(HDMI_AVI_HEADER1, infoframe->ver);
        hdmi_writeb(HDMI_AVI_HEADER2, infoframe->len);
        hdr_sum = infoframe->type + infoframe->ver + infoframe->len;
        hdmi_writeb(HDMI_AVI_BYTE(1), me->output_fmt << 5 |
                AVI_ACTIVE_FORMAT_VALID | AVI_UNDERSCAN);
        if (me->aspect == HDMI_ASPECT_RATIO_4_3 &&
                (me->cur_preset == V4L2_DV_480P59_94 ||
                 me->cur_preset == V4L2_DV_480P60)) {
            aspect_ratio = AVI_PIC_ASPECT_RATIO_4_3;
            /* 17 : 576P50Hz 4:3 aspect ratio */
            vic = 17;
        } else {
            aspect_ratio = AVI_PIC_ASPECT_RATIO_16_9;
            vic = me->cur_conf->vic;
        }

        hdmi_writeb(HDMI_AVI_BYTE(2), aspect_ratio |
                AVI_SAME_AS_PIC_ASPECT_RATIO | AVI_ITU709);
        if (me->color_range == 0 || me->color_range == 2)
            hdmi_writeb(HDMI_AVI_BYTE(3), AVI_FULL_RANGE);
        else
            hdmi_writeb(HDMI_AVI_BYTE(3), AVI_LIMITED_RANGE);
        pr_debug("%s: VIC code = %d\n", __func__, vic);
        hdmi_writeb(HDMI_AVI_BYTE(4), vic);
        chksum = hdmi_chksum(HDMI_AVI_BYTE(1), infoframe->len, hdr_sum);
        pr_debug("%s: AVI checksum = 0x%x\n", __func__, chksum);
        hdmi_writeb(HDMI_AVI_CHECK_SUM, chksum);
        break;

    case HDMI_PACKET_TYPE_AUI:
        hdmi_write(HDMI_AUI_CON, HDMI_AUI_CON_TRANS_EVERY_VSYNC);
        hdmi_writeb(HDMI_AUI_HEADER0, infoframe->type);
        hdmi_writeb(HDMI_AUI_HEADER1, infoframe->ver);
        hdmi_writeb(HDMI_AUI_HEADER2, infoframe->len);
        hdr_sum = infoframe->type + infoframe->ver + infoframe->len;
        /* speaker placement */
        if (me->audio_channel_count == 6)
            hdmi_writeb(HDMI_AUI_BYTE(4), 0x0b);
        else if (me->audio_channel_count == 8)
            hdmi_writeb(HDMI_AUI_BYTE(4), 0x13);
        else
            hdmi_writeb(HDMI_AUI_BYTE(4), 0x00);
        chksum = hdmi_chksum(HDMI_AUI_BYTE(1), infoframe->len, hdr_sum);
        pr_debug("%s: AUI checksum = 0x%x\n", __func__, chksum);
        hdmi_writeb(HDMI_AUI_CHECK_SUM, chksum);
        break;

    default:
        pr_err("%s: unknown type(0x%x)\n", __func__, infoframe->type);
        break;
    }
}

static int _hdmi_set_infoframe(struct nxp_hdmi *me)
{
    struct hdmi_infoframe infoframe;
    const struct hdmi_3d_info *info_3d;

    info_3d = _hdmi_preset2info(me->cur_preset);

    pr_debug("%s: is_3d %d\n", __func__, info_3d->is_3d);
    if (info_3d->is_3d == HDMI_VIDEO_FORMAT_3D) {
        infoframe.type = HDMI_PACKET_TYPE_VSI;
        infoframe.ver  = HDMI_VSI_VERSION;
        infoframe.len  = HDMI_VSI_LENGTH;
        _hdmi_reg_infoframe(me, &infoframe, info_3d);
    } else {
        hdmi_stop_vsi();
    }

    infoframe.type = HDMI_PACKET_TYPE_AVI;
    infoframe.ver  = HDMI_AVI_VERSION;
    infoframe.len  = HDMI_AVI_LENGTH;
    _hdmi_reg_infoframe(me, &infoframe, info_3d);

    if (me->audio_enable) {
        infoframe.type = HDMI_PACKET_TYPE_AUI;
        infoframe.ver  = HDMI_AUI_VERSION;
        infoframe.len  = HDMI_AUI_LENGTH;
        _hdmi_reg_infoframe(me, &infoframe, info_3d);
    }

    return 0;
}

static inline void _hdmi_enable(struct nxp_hdmi *me)
{
    u32 regval = NX_HDMI_GetReg(0, HDMI_LINK_HDMI_CON_0);
    regval |= 0x01;
    NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_0, regval);

    NX_DISPLAYTOP_HDMI_SetVSyncStart(me->VSyncStart);
    NX_DISPLAYTOP_HDMI_SetHActiveStart(me->HActiveStart);
    NX_DISPLAYTOP_HDMI_SetHActiveEnd(me->HActiveEnd);
    NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd(me->VSyncHSStartEnd0, me->VSyncHSStartEnd1);
}

static int _hdmi_streamon(struct nxp_hdmi *me)
{
    int ret;

    pr_debug("%s\n", __func__);

    /**
     * [SEQ 5] set up the HDMI PHY to specific video clock.
     */
    /* me->phy.s_dv_preset(&me->phy, me->preset); */
    ret = me->phy.s_stream(&me->phy, 1);
    if (ret < 0) {
        pr_err("%s: phy->s_stream() failed\n", __func__);
        return ret;
    }

    /**
     * [SEQ 6] I2S (or SPDIFTX) configuration for the source audio data
     * this is done in another user app  - ex> Android Audio HAL
     */

    /**
     * [SEQ 7] Wait for ECID ready
     */
    if(false == _wait_for_ecid_ready()) {
        pr_err("%s: failed to wait for ecid ready\n", __func__);
        me->phy.s_stream(&me->phy, 0);
        return -EIO;
    }

    /**
     * [SEQ 8] release the resets of HDMI.i_VIDEO_nRST and HDMI.i_SPDIF_nRST and HDMI.i_TMDS_nRST
     */
    _hdmi_reset(me);

    /**
     * [SEQ 9] Wait for HDMI PHY ready (wait until 0xC0200020.[0], 1)
     */
    if (false == _wait_for_hdmiphy_ready()) {
        pr_err("%s: failed to wait for hdmiphy ready\n", __func__);
        /* me->phy.s_stream(&me->phy, 0); */
        /* return -EIO; */
    }

    /**
     * [SEC 10] Set the DPC CLKGEN’s Source Clock to HDMI_CLK & Set Sync Parameter
     */
    _set_hdmi_clkgen(me); /* set hdmi link clk to clkgen  vs default is hdmi phy clk */
 	 _set_audio_clkgen(me);
    _hdmi_sync_init(me);

    /**
     * [SEQ 11] Set up the HDMI Converter parameters
     */
    _config_hdmi(me);
    _set_remote_sync(me);

    _hdmi_set_infoframe(me);
    _hdmi_set_packets(me);

#if defined(CONFIG_NXP_HDMI_AUDIO_I2S)
    hdmi_audio_i2s_init(me->sample_rate, me->bits_per_sample);
#elif defined(CONFIG_NXP_HDMI_AUDIO_SPDIF)
    hdmi_audio_spdif_init(me->audio_codec, me->bits_per_sample);
#endif

    if (me->audio_enable)
        hdmi_audio_enable(true);

    /* hdmi_set_dvi_mode(me->is_dvi); */

    _hdmi_enable(me);

    mdelay(5);

#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    /* start HDCP */
    ret = me->hdcp.start(&me->hdcp);
    if (ret < 0) {
        pr_err("%s: hdcp start error(%d)\n", __func__, ret);
        return ret;
    }
#endif

    me->streaming = true;
    return 0;
}

static int _hdmi_streamoff(struct nxp_hdmi *me)
{
    pr_debug("%s\n", __func__);

    me->streaming = false;

#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    me->hdcp.stop(&me->hdcp);
#endif

    hdmi_audio_enable(false);
    hdmi_enable(false);
    hdmi_tg_enable(false,
            me->cur_conf->mbus_fmt.field == V4L2_FIELD_INTERLACED);

    _hdmi_clk_enable(me, 0, false);

    me->phy.s_stream(&me->phy, 0);

    if (DISP_DEVICE_RESCONV != me->source_device) {
        nxp_soc_disp_device_enable(me->source_device, 0);
        nxp_soc_disp_device_disconnect(DISP_DEVICE_HDMI, me->source_device);
    }

    return 0;
}

static int _hdmi_runtime_resume(struct nxp_hdmi *me) __attribute__((unused));
static int _hdmi_runtime_resume(struct nxp_hdmi *me)
{
#if 0
    int ret;

    pr_debug("%s\n", __func__);

    _hdmi_clk_enable(me, 0, true);
    hdmi_sw_reset();
    hdmi_phy_sw_reset();

    ret = me->phy.s_power(&me->phy, 1);
    if (ret < 0) {
        pr_err("%s: failed to phy.s_power()\n", __func__);
        _hdmi_clk_enable(me, 0, false);
        return ret;
    }

    hdmi_conf_apply(me->color_range, me->cur_conf);
#endif

    return 0;
}

static int _hdmi_runtime_suspend(struct nxp_hdmi *me) __attribute__((unused));
static int _hdmi_runtime_suspend(struct nxp_hdmi *me)
{
    int ret;

    pr_debug("%s\n", __func__);

    _hdmi_clk_enable(me, 0, false);

    ret = me->phy.s_power(&me->phy, 0);
    if (ret < 0) {
        pr_err("%s: failed to phy.s_power()\n", __func__);
        return ret;
    }

    return 0;
}

/**
 * member function
 */
static bool _hdmi_is_streaming(struct nxp_hdmi *me)
{
    if (me->streaming && hdmi_hpd_status())
        return true;
    return false;
}

/**
 * subdev core ops
 */
static int nxp_hdmi_s_power(struct v4l2_subdev *sd, int on)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    pr_debug("%s %d\n", __func__, on);

    if (!me) {
        pr_err("%s: can't get me!!!\n", __func__);
        return -ENODEV;
    }

    if (on) {
        _hdmi_initialize(me);

#if 0
        _hdmi_clk_enable(me, 0, true);

        ret = _hdmi_runtime_resume(me);
        if (ret < 0) {
            pr_err("%s: failed to _hdmi_runtime_resume()\n", __func__);
            return ret;
        }

        /* TODO : hpd interrupt ? */
        disable_irq(me->external_irq);
        cancel_delayed_work_sync(&me->hpd_work_ext);

        me->set_int_internal(me->internal_irq);
        hdmi_hpd_enable(true);
        enable_irq(me->internal_irq);

        pr_debug("%s: HDMI interrupt changed to internal\n", __func__);
#else
        /* hdmi_hpd_enable(true); */
        /* enable_irq(me->internal_irq); */
#endif
    } else {
#if defined(CONFIG_NXP_HDMI_USE_HDCP)
        /* stop HDCP */
        me->hdcp.stop(&me->hdcp);
#endif

#if 0
        hdmi_hpd_enable(false);
        disable_irq(me->internal_irq);
        cancel_work_sync(&me->hpd_work);

        me->set_int_external(me->external_irq);
        enable_irq(me->external_irq);
        pr_debug("%s: HDMI interupt changed to external\n", __func__);

        _hdmi_runtime_suspend(me);

        _hdmi_clk_enable(me, 0, false);
#endif
    }

    return 0;
}

static int nxp_hdmi_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    pr_debug("%s\n", __func__);

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
        if (_hdmi_is_streaming(me)) {
            _hdmi_set_infoframe(me);
            hdmi_audio_enable(me->audio_enable);
        }
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
        if (_hdmi_is_streaming(me))
            _hdmi_set_infoframe(me);
        mutex_unlock(&me->mutex);
        break;

    case V4L2_CID_HDMI_SET_COLOR_RANGE:
        mutex_lock(&me->mutex);
        me->color_range = ctrl->value;
        mutex_unlock(&me->mutex);
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

    printk("%s: %d\n", __func__, preset->preset);
    if (!conf) {
        pr_err("%s: preset(%u) not supported\n", __func__, preset->preset);
        return -EINVAL;
    }
    me->cur_conf = (struct hdmi_preset_conf *)conf;
    me->cur_preset = preset->preset;

    return me->phy.s_dv_preset(&me->phy, preset);
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
    u32 preset;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    preset = me->edid.enum_presets(&me->edid, enum_preset->index);

    if (preset == V4L2_DV_INVALID)
        return -EINVAL;

    return v4l_fill_dv_preset_info(preset, enum_preset);
}

static int nxp_hdmi_s_stream(struct v4l2_subdev *sd, int enable)
{
    int ret = 0;
    struct nxp_hdmi *me = subdev_to_nxp_hdmi(sd);

    /* pr_debug("%s: %d\n", __func__, enable); */
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
    struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
    if (flags & MEDIA_LNK_FL_ENABLED) {
        pr_debug("%s: s_power() on\n", __func__);
        /* on */
        printk("%s: connected to me!!!\n", __func__);
        return v4l2_subdev_call(sd, core, s_power, 1);
    } else {
        pr_debug("%s: s_power() off\n", __func__);
        /* off */
        printk("%s: disconnected to me!!!\n", __func__);
        return v4l2_subdev_call(sd, core, s_power, 0);
    }
}

static const struct media_entity_operations nxp_hdmi_media_ops = {
    .link_setup = nxp_hdmi_link_setup,
};

/**
 * irq handler
 */
/* internal irq */
static irqreturn_t _hdmi_irq_handler(int irq, void *dev_data)
{
    struct nxp_hdmi *me = dev_data;
    u32 flag;

    /* flag = NX_HDMI_GetReg(0, HDMI_LINK_INTC_FLAG_0); */
    flag = hdmi_read(HDMI_LINK_INTC_FLAG_0);
    printk("%s: flag 0x%x\n", __func__, flag);
    if (flag & HDMI_INTC_FLAG_HPD_UNPLUG) {
#if defined(CONFIG_NXP_HDMI_USE_HDCP)
        /* stop HDCP */
        me->hdcp.stop(&me->hdcp);
#endif
        hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, HDMI_INTC_FLAG_HPD_UNPLUG);
        /* NX_HDMI_SetReg(0, HDMI_LINK_INTC_FLAG_0, HDMI_INTC_FLAG_HPD_UNPLUG); */
    }
    if (flag & HDMI_INTC_FLAG_HPD_PLUG) {
        /* ignore plug in interrupt */
        hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, HDMI_INTC_FLAG_HPD_PLUG);
        /* NX_HDMI_SetReg(0, HDMI_LINK_INTC_FLAG_0, HDMI_INTC_FLAG_HPD_PLUG); */
    }
#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    if (flag & HDMI_INTC_FLAG_HDCP) {
        pr_debug("%s: hdcp interrupt occur\n", __func__);
        me->hdcp.irq_handler(&me->hdcp);
        hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, HDMI_INTC_FLAG_HDCP);
        /* NX_HDMI_SetReg(0, HDMI_LINK_INTC_FLAG_0, HDMI_INTC_FLAG_HDCP); */
    }
#endif

    /* queue_work(system_nrt_wq, &me->hpd_work); */
    if (flag & (HDMI_INTC_FLAG_HPD_UNPLUG | HDMI_INTC_FLAG_HPD_PLUG))
        queue_delayed_work(system_nrt_wq, &me->hpd_work, msecs_to_jiffies(1000));

    if (me->callback && (me->irq_num_of_callback & flag)) {
        hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, me->irq_num_of_callback);
        me->callback(me->callback_data);
    }

    return IRQ_HANDLED;
}

/* external irq */
static irqreturn_t _hdmi_irq_handler_ext(int irq, void *dev_data) __attribute__((unused));
static irqreturn_t _hdmi_irq_handler_ext(int irq, void *dev_data)
{
    struct nxp_hdmi *me = dev_data;
    queue_delayed_work(system_nrt_wq, &me->hpd_work_ext, 0);

    return IRQ_HANDLED;
}

/**
 * hpd work functions
 */
static void _hdmi_hpd_changed(struct nxp_hdmi *me, int state)
{
    int ret;
    u32 preset;

    printk("%s: state 0x%x\n", __func__, state);
    if (state == switch_get_state(&me->hpd_switch))
        return;

    if (state) {
        /* connected */
        ret = me->edid.update(&me->edid);
        if (ret < 0) {
            pr_err("%s: failed to edid update(), use static information\n", __func__);
            me->is_dvi = false;
            me->cur_preset = HDMI_DEFAULT_PRESET;
            me->cur_conf = (struct hdmi_preset_conf *)_hdmi_preset2conf(HDMI_DEFAULT_PRESET);
        } else {
            preset = me->edid.preferred_preset(&me->edid);
            if (preset == V4L2_DV_INVALID)
                preset = HDMI_DEFAULT_PRESET;

            // psw0523 fix : edid error?
            /*me->is_dvi = !me->edid.supports_hdmi(&me->edid);*/
            me->is_dvi = false;
            me->cur_preset = preset;
            me->cur_conf = (struct hdmi_preset_conf *)_hdmi_preset2conf(preset);
        }
    }

    switch_set_state(&me->hpd_switch, state);

    pr_debug("%s: state(%d)\n", __func__, state);
}

static void _hdmi_hpd_work_ext(struct work_struct *work)
{
    int state;
    struct nxp_hdmi *me = container_of(work, struct nxp_hdmi,
            hpd_work_ext.work);
    state = me->read_hpd_gpio(me->external_irq);
    _hdmi_hpd_changed(me, state);
}

static void _hdmi_hpd_work(struct work_struct *work)
{
    int state;
    struct nxp_hdmi *me = container_of(work, struct nxp_hdmi, hpd_work.work);
    state = hdmi_hpd_status();
    if (!nxp_cpu_version()) {
        /* no revision */
        /* state is invert!!! */
        state = !state;
    }
    _hdmi_hpd_changed(me, state);
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
        goto error_init_entities;
    }

    /* set plat data */
    /* me->internal_irq = pdata->internal_irq; */
    me->internal_irq = NX_HDMI_GetInterruptNumber(0);
    me->external_irq = pdata->external_irq;
    me->set_int_external = pdata->set_int_external;
    me->set_int_internal = pdata->set_int_internal;
    me->read_hpd_gpio    = pdata->read_hpd_gpio;

    /* member function */
    me->is_streaming = _hdmi_is_streaming;

    /* set priv base addr : TODO */
    /* hdmi_set_base(pdata->base); */
    hdmi_set_base((void *)IO_ADDRESS(NX_HDMI_GetPhysicalAddress(0)));

#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    /* init HDCP */
    ret = nxp_hdcp_init(&me->hdcp, pdata->hdcp);
    if (ret < 0) {
        pr_err("%s: failed to nxp_hdcp_init()\n", __func__);
        goto error_hdcp;
    }
#endif

    /* init EDID */
    ret = nxp_edid_init(&me->edid, pdata->edid);
    if (ret < 0) {
        pr_err("%s: failed to nxp_edid_init()\n", __func__);
        goto error_edid;
    }

    /* init PHY */
    ret = nxp_hdmiphy_init(&me->phy);
    if (ret < 0) {
        pr_err("%s: failed to nxp_phy_init()\n", __func__);
        goto error_phy;
    }

    /* work */
    /* INIT_WORK(&me->hpd_work, _hdmi_hpd_work); */
    INIT_DELAYED_WORK(&me->hpd_work, _hdmi_hpd_work);
    INIT_DELAYED_WORK(&me->hpd_work_ext, _hdmi_hpd_work_ext);

    /* etc */
    mutex_init(&me->mutex);

#ifdef CONFIG_ANDROID
    wake_lock_init(&me->wake_lock, WAKE_LOCK_SUSPEND, "nxp-hdmi");
#endif

    /* set default value */
    /* me->audio_enable = false; */
    me->audio_enable = true;
    me->audio_channel_count = 2;
    me->sample_rate = DEFAULT_SAMPLE_RATE;
    me->color_range = 3;
    me->bits_per_sample = DEFAULT_BITS_PER_SAMPLE;
    me->audio_codec = DEFAULT_AUDIO_CODEC;
    me->aspect = HDMI_ASPECT_RATIO_16_9;

    return me;

error_phy:
    nxp_edid_cleanup(&me->edid);
error_edid:
#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    nxp_hdcp_cleanup(&me->hdcp);
error_hdcp:
#endif
    media_entity_cleanup(&me->sd.entity);
error_init_entities:
    kfree(me);
    return NULL;
}

void release_nxp_hdmi(struct nxp_hdmi *me)
{
    pr_debug("%s\n", __func__);

#ifdef CONFIG_ANDROID
    wake_lock_destroy(&me->wake_lock);
#endif
    media_entity_cleanup(&me->sd.entity);
    _hdmi_clk_enable(me, 0, false);
#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    nxp_hdcp_cleanup(&me->hdcp);
#endif
    nxp_edid_cleanup(&me->edid);
    nxp_hdmiphy_cleanup(&me->phy);
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

    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(),
            &me->sd);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    /* HPD */
    me->hpd_switch.name = "hdmi";
    switch_dev_register(&me->hpd_switch);

    /* irq */
    ret = request_irq(me->internal_irq, _hdmi_irq_handler, 0,
            "hdmi-int", me);
    if (ret < 0) {
        pr_err("%s: failed to request_irq(%d)\n", __func__, me->internal_irq);
        goto error_req_irq_int;
    }
    disable_irq(me->internal_irq);

#if 1
    me->initialized = false;
    _hdmi_initialize(me);
    printk("%s: end _hdmi_initialize()\n", __func__);
    /* hdmi_hpd_enable(true); */
    /* no blue screen mode, encoding order as it is */

    /* set interrupt : enable hpd_plug, hpd_unplug */
    NX_HDMI_SetReg(0, HDMI_LINK_INTC_CON_0, (1<<6)|(1<<3)|(1<<2));
    enable_irq(me->internal_irq);
    printk("%s: end enable_irq()\n", __func__);
#else
    /* this is test code !!! */
    test_hdmi();
#endif

#if defined(CONFIG_NXP_HDMI_USE_HDCP)
    /* prepare HDCP */
    ret = me->hdcp.prepare(&me->hdcp);
    if (ret < 0) {
        pr_err("%s: failed to hdcp prepare()\n", __func__);
        goto error_hdcp;
    }
#endif

    /* start HPD work */
    /* TODO: this is necessary ? */
    /* queue_delayed_work(system_nrt_wq, &me->hpd_work_ext, msecs_to_jiffies(1000)); */

    me->streaming = false;

    return 0;

#if defined(CONFIG_NXP_HDMI_USE_HDCP)
error_hdcp:
/*     free_irq(me->external_irq, me); */
/* error_req_irq_ext: */
    free_irq(me->internal_irq, me);
#endif
error_req_irq_int:
    switch_dev_unregister(&me->hpd_switch);

    pr_debug("%s failed(ret=%d)\n", __func__, ret);
    return ret;
}

void unregister_nxp_hdmi(struct nxp_hdmi *me)
{
    pr_debug("%s\n", __func__);
    /* stop HPD work */
    cancel_delayed_work_sync(&me->hpd_work_ext);
    switch_dev_unregister(&me->hpd_switch);
    free_irq(me->external_irq, me);
    free_irq(me->internal_irq, me);

    v4l2_device_unregister_subdev(&me->sd);
}

#ifdef CONFIG_PM
int suspend_nxp_hdmi(struct nxp_hdmi *me)
{
    PM_DBGOUT("+%s\n", __func__);
    me->initialized = false;
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

int resume_nxp_hdmi(struct nxp_hdmi *me)
{
    PM_DBGOUT("+%s\n", __func__);
    _hdmi_initialize(me);
    NX_HDMI_SetReg(0, HDMI_LINK_INTC_CON_0, (1<<6)|(1<<3)|(1<<2));
    if (me->streaming)
        _hdmi_streamon(me);
    else if (hdmi_hpd_status())
        _hdmi_hpd_changed(me, 1);
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif

extern struct nxp_hdmi *get_nxp_hdmi(void);
int register_hdmi_irq_callback(int int_num, int (*callback)(void *), void *data)
{
    struct nxp_hdmi *hdmi = get_nxp_hdmi();
    if (hdmi) {
        hdmi->callback = callback;
        hdmi->callback_data = data;
        hdmi->irq_num_of_callback = int_num;
        printk("%s: int_num 0x%x, callback %p\n", __func__, int_num, callback);
        return 0;
    }
    return -ENODEV;
}

int unregister_hdmi_irq_callback(int int_num)
{
    struct nxp_hdmi *hdmi = get_nxp_hdmi();
    if (hdmi && int_num == hdmi->irq_num_of_callback) {
        hdmi->callback = NULL;
        hdmi->callback_data = NULL;
        hdmi->irq_num_of_callback = -1;
        return 0;
    }
    return -ENODEV;
}
