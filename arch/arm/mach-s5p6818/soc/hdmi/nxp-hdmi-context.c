#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/switch.h>

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

#include <mach/hdmi/nxp-hdmi-context.h>

extern int test_hdmi(void);

/*#define DEFAULT_SAMPLE_RATE	    44100*/
#define DEFAULT_SAMPLE_RATE	    48000
#define DEFAULT_BITS_PER_SAMPLE	16
#define DEFAULT_AUDIO_CODEC	    HDMI_AUDIO_PCM

/**
 * static var : nxp_hdmi_context;
 */
static struct nxp_hdmi_context *__me = NULL;

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
 * internal functions
 */
static void _hdmi_initialize(struct nxp_hdmi_context *me)
{
    if (!me->initialized) {
        /**
         * [SEQ 1] release the reset of DisplayTop.i_Top_nRst) : skip
         */

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
        NX_TIEOFF_SetBaseAddress((void*)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress()));
        NX_TIEOFF_Set(TIEOFFINDEX_OF_DISPLAYTOP0_i_HDMI_PHY_REFCLK_SEL, 1);

        /**
         * [SEQ 4] release the resets of HDMI.i_PHY_nRST and HDMI.i_nRST
         */
#if defined (CONFIG_ARCH_NXP4330)
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nDISABLE);
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nDISABLE);

        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE);
        NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nENABLE);
#elif defined (CONFIG_ARCH_S5P6818)
        NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_ASSERT);
        NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_ASSERT);

        NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_NEGATE);
        NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_NEGATE);
#endif

        /* initialize intr : hpd */
        NX_HDMI_SetReg(0, HDMI_LINK_INTC_CON_0, (1<<6)|(1<<3)|(1<<2));

        me->initialized = true;

        __me = me;

        /**
         * Next sequence start in hdmi_run()
         */
    }
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

static inline void _hdmi_reset(struct nxp_hdmi_context *me)
{
#if defined (CONFIG_ARCH_NXP4330)
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nDISABLE);

    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nENABLE);
#elif defined (CONFIG_ARCH_S5P6818)
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_ASSERT);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_ASSERT);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_ASSERT);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_ASSERT);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_ASSERT);

    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_NEGATE);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_NEGATE);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_NEGATE);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_NEGATE);
    NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_NEGATE);
#endif
}

static inline bool _wait_for_hdmiphy_ready(void)
{
    int retry_count = 500;
    do {
        u32 regval = NX_HDMI_GetReg(0, HDMI_LINK_PHY_STATUS_0);
        if (regval & 0x01) {
            printk("HDMI PHY Ready!!!\n");
            return true;
        }
        mdelay(10);
        retry_count--;
    } while (retry_count);

    return false;
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

static int _get_vsync_info(struct nxp_hdmi_context *me)
{
    struct disp_vsync_info *vsync = &me->dpc_sync_info;
    struct disp_syncgen_par *par = &me->dpc_sync_param;
    nxp_soc_disp_device_get_sync_param(me->source_device, (void*)par);

    /**
     * reference : CEA-861D.pdf
     */
    switch (me->cur_preset) {
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
        vsync->h_active_len = 1280;
        vsync->h_sync_width = 40;
        vsync->h_back_porch = 220;
        vsync->h_front_porch = 440;
        vsync->h_sync_invert = 0;
        vsync->v_active_len = 720;
        vsync->v_sync_width = 5;
        vsync->v_back_porch = 20;
        vsync->v_front_porch = 5;
        vsync->v_sync_invert = 0;
        break;

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
        pr_err("%s: invalid preset value 0x%x\n", __func__, me->cur_preset);
        return -EINVAL;
    }

    vsync->pixel_clock_hz = _get_clk_hz(me->cur_preset);
    vsync->clk_src_lv0 = 4;
    vsync->clk_div_lv0 = 1;
    vsync->clk_src_lv1 = 7;
    vsync->clk_div_lv1 = 1;

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

static int _set_remote_sync(struct nxp_hdmi_context *me)
{
    int ret;

    ret = _get_vsync_info(me);
    if (ret) {
        pr_err("%s: failed to _get_vsync_info()\n", __func__);
        return ret;
    }

    if (me->source_device != DISP_DEVICE_RESCONV) {
        /*me->dpc_sync_info.interlace = me->cur_conf->mbus_fmt.field == V4L2_FIELD_INTERLACED;*/
        me->dpc_sync_info.interlace = false; // nxp4330 don't support interlace

        ret = nxp_soc_disp_device_set_sync_param(me->source_device, (void*)&me->dpc_sync_param);
        if (ret) {
            pr_err("%s: failed to display parameter....\n", __func__);
            return ret;
        }

        ret = nxp_soc_disp_device_connect_to(DISP_DEVICE_HDMI, me->source_device, &me->dpc_sync_info);
        if (ret) {
            pr_err("%s: failed to connect to source\n", __func__);
            return ret;
        }

        return nxp_soc_disp_device_enable(me->source_device, 1);
    }

    return 0;
}

static inline void _set_hdmi_clkgen(struct nxp_hdmi_context *me)
{
    NX_DISPTOP_CLKGEN_SetBaseAddress(ToMIPI_CLKGEN,
            (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(ToMIPI_CLKGEN)));
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ToMIPI_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode(ToMIPI_CLKGEN, NX_PCLKMODE_ALWAYS);
    NX_DISPTOP_CLKGEN_SetClockSource(ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, 2); // pll2
    NX_DISPTOP_CLKGEN_SetClockDivisor(ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, 2);
    NX_DISPTOP_CLKGEN_SetClockSource(ToMIPI_CLKGEN, 1, 7);
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ToMIPI_CLKGEN, CTRUE);
}

static inline void _set_audio_clkgen(struct nxp_hdmi_context *me)
{
    NX_DISPTOP_CLKGEN_SetBaseAddress(ToMIPI_CLKGEN,
            (void*)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(ToMIPI_CLKGEN)));
    NX_DISPTOP_CLKGEN_SetClockPClkMode(ToMIPI_CLKGEN, NX_PCLKMODE_ALWAYS);
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable(ToMIPI_CLKGEN, CTRUE);
}

static inline void _hdmi_sync_init(struct nxp_hdmi_context *me)
{
    NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd(0, 0);
    NX_DISPLAYTOP_HDMI_SetVSyncStart(0);
    NX_DISPLAYTOP_HDMI_SetHActiveStart(0);
    NX_DISPLAYTOP_HDMI_SetHActiveEnd(0);
}

static int _h_sync_offset = 0;

static inline void _calc_hdmi_sync_param(struct nxp_hdmi_context *me)
{
    struct hdmi_sync_param *param = &me->hdmi_sync_param;
    struct disp_vsync_info *vsync = &me->dpc_sync_info;

#if 0
    param->v_sync_start = vsync->v_sync_width + vsync->v_back_porch + vsync->v_active_len - 1;
    param->h_active_start = vsync->h_sync_width + vsync->h_back_porch;
    param->h_active_end = vsync->h_active_len + vsync->h_sync_width + vsync->h_back_porch;
    param->v_sync_hs_start_end0 = vsync->h_sync_width + vsync->h_back_porch + 1;
    param->v_sync_hs_start_end1 = param->v_sync_hs_start_end0 + 1;
#else
    // this is test
    printk("%s: _h_sync_offset %d\n", __func__, _h_sync_offset);
    param->v_sync_start = vsync->v_sync_width + vsync->v_back_porch + vsync->v_active_len - 1;
    param->h_active_start = vsync->h_sync_width + vsync->h_back_porch + _h_sync_offset;
    param->h_active_end = vsync->h_active_len + vsync->h_sync_width + vsync->h_back_porch + _h_sync_offset;
    param->v_sync_hs_start_end0 = vsync->h_sync_width + vsync->h_back_porch + 1 + _h_sync_offset;
    param->v_sync_hs_start_end1 = param->v_sync_hs_start_end0 + 1;
#endif
}

void hdmi_set_hsync_offset(int offset)
{
    _h_sync_offset = offset;
}

static void _config_hdmi(struct nxp_hdmi_context *me)
{
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

    struct disp_vsync_info *vsync = &me->dpc_sync_info;

    /**
     * calculate sync variables
     */
    h_blank = vsync->h_front_porch + vsync->h_sync_width + vsync->h_back_porch;
    v_blank = vsync->v_front_porch + vsync->v_sync_width + vsync->v_back_porch;
    v_actline = vsync->v_active_len;
    v2_blank = v_line = v_actline + v_blank; /* total v */
    h_line = vsync->h_active_len + vsync->h_front_porch + vsync->h_sync_width + vsync->h_back_porch;  /* total h */
    h_sync_start = vsync->h_front_porch;
    h_sync_end = vsync->h_front_porch + vsync->h_sync_width;
    v_sync_line_bef_1 = vsync->v_front_porch;
    v_sync_line_bef_2 = vsync->v_front_porch + vsync->v_sync_width;

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

    /* set HPD to 0x0 : later check hpd */
    NX_HDMI_SetReg(0, HDMI_LINK_HPD, 0x0);

    /* set MODE_SEL to 0x02 */
    NX_HDMI_SetReg(0, HDMI_LINK_MODE_SEL, 0x2);

    /* set H_BLANK_*, V1_BLANK_*, V2_BLANK_*, V_LINE_*, H_LINE_*, H_SYNC_START_*, H_SYNC_END_*
     * V_SYNC_LINE_BEF_1_*, V_SYNC_LINE_BEF_2_*
     */
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

    // TODO : why need this code ?
#if 0
    if (vsync->h_active_len == 1280) {
        NX_HDMI_SetReg(0, HDMI_LINK_HSYNC_POL, 0x1);
        NX_HDMI_SetReg(0, HDMI_LINK_VSYNC_POL, 0x1);
    } else {
        NX_HDMI_SetReg(0, HDMI_LINK_HSYNC_POL, 0x0);
        NX_HDMI_SetReg(0, HDMI_LINK_VSYNC_POL, 0x0);
    }
#else
    NX_HDMI_SetReg(0, HDMI_LINK_HSYNC_POL, 0x0);
    NX_HDMI_SetReg(0, HDMI_LINK_VSYNC_POL, 0x0);
#endif
    // end TODO

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
    /*NX_HDMI_SetReg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x1);*/
    /*NX_HDMI_SetReg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x3); // internal*/

    NX_HDMI_SetReg(0, HDMI_LINK_GCP_CON, 0x0a);

    /* Set HDMI Sync Control parameters */
    _calc_hdmi_sync_param(me);
}

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

static void _hdmi_set_packets(struct nxp_hdmi_context *me)
{
    hdmi_set_acr(me->sample_rate, me->is_dvi);
}

static void _hdmi_reg_infoframe(struct nxp_hdmi_context *me,
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
        hdmi_writeb(HDMI_AVI_BYTE(1), HDMI_OUTPUT_RGB888 << 5 |
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

static inline void _hdmi_enable(struct nxp_hdmi_context *me)
{
    u32 regval = NX_HDMI_GetReg(0, HDMI_LINK_HDMI_CON_0);
    regval |= 0x01;
    NX_HDMI_SetReg(0, HDMI_LINK_HDMI_CON_0, regval);

    msleep(20);

    pr_debug("%s: v_sync_start %d, h_active_start %d, h_active_end %d, v_sync_hs_start_end0 %d, v_sync_hs_start_end1 %d\n", __func__,
            me->hdmi_sync_param.v_sync_start, me->hdmi_sync_param.h_active_start, me->hdmi_sync_param.h_active_end,
            me->hdmi_sync_param.v_sync_hs_start_end0, me->hdmi_sync_param.v_sync_hs_start_end1);
    NX_DISPLAYTOP_HDMI_SetVSyncStart(me->hdmi_sync_param.v_sync_start);
    NX_DISPLAYTOP_HDMI_SetHActiveStart(me->hdmi_sync_param.h_active_start);
    NX_DISPLAYTOP_HDMI_SetHActiveEnd(me->hdmi_sync_param.h_active_end);
    NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd(me->hdmi_sync_param.v_sync_hs_start_end0, me->hdmi_sync_param.v_sync_hs_start_end1);
}

/**
 * irq handler
 */
static irqreturn_t _hdmi_irq_handler(int irq, void *dev_data)
{
    struct nxp_hdmi_context *me = dev_data;
    u32 flag;

    flag = hdmi_read(HDMI_LINK_INTC_FLAG_0);
    pr_debug("%s: flag 0x%x\n", __func__, flag);
    if (flag & HDMI_INTC_FLAG_HPD_UNPLUG) {
#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
        /* stop HDCP */
        /*me->hdcp.stop(&me->hdcp);*/
#endif
        hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, HDMI_INTC_FLAG_HPD_UNPLUG);
    }
    if (flag & HDMI_INTC_FLAG_HPD_PLUG) {
        hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, HDMI_INTC_FLAG_HPD_PLUG);
    }
#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
    if (flag & HDMI_INTC_FLAG_HDCP) {
        pr_debug("%s: hdcp interrupt occur\n", __func__);
        me->hdcp.irq_handler(&me->hdcp);
        /*hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, HDMI_INTC_FLAG_HDCP);*/
    }
#endif

    if (flag & (HDMI_INTC_FLAG_HPD_UNPLUG | HDMI_INTC_FLAG_HPD_PLUG))
        queue_delayed_work(system_nrt_wq, &me->hpd_work, msecs_to_jiffies(1000));

    spin_lock(&me->lock_callback);
    if (!list_empty(&me->callback_list)) {
        struct hdmi_irq_callback *entry;
        list_for_each_entry(entry, &me->callback_list, list)
            if (entry->irq_num & flag) {
                hdmi_write_mask(HDMI_LINK_INTC_FLAG_0, ~0, entry->irq_num);
                entry->callback(entry->data);
            }
    }
    spin_unlock(&me->lock_callback);

    return IRQ_HANDLED;
}

u32 hdmi_get_edid_cec_phy_address(void)
{
    return __me->edid.cec_phy_address;
}


/**
 * hpd work functions
 */
static void _hdmi_hpd_changed(struct nxp_hdmi_context *me, int state)
{
    int ret;
    u32 preset;

    printk("HDMI HPD State 0x%x\n", state);
    if (state == switch_get_state(&me->hpd_switch))
        return;

    if (state) {
#ifndef CONFIG_PLAT_S5P6818_DRONEL
        /* connected */
        ret = me->edid.update(&me->edid);
        if (ret < 0) {
            pr_err("%s: failed to edid update(), use static information\n", __func__);
            me->is_dvi = false;
            if (me->cur_preset == V4L2_DV_INVALID)
                me->cur_preset = HDMI_DEFAULT_PRESET;
        } else {
            preset = me->edid.preferred_preset(&me->edid);
            printk("%s: edid update success, preset %d\n", __func__, preset);
            if (preset == V4L2_DV_INVALID)
                preset = HDMI_DEFAULT_PRESET;

            me->is_dvi = !me->edid.supports_hdmi(&me->edid);
            if (me->cur_preset == V4L2_DV_INVALID)
                me->cur_preset = preset;
        }
#else
        me->is_dvi = false;
        if (me->cur_preset == V4L2_DV_INVALID)
            me->cur_preset = HDMI_DEFAULT_PRESET;
#endif
        me->cur_conf = (struct hdmi_preset_conf *)_hdmi_preset2conf(me->cur_preset);
    }

    switch_set_state(&me->hpd_switch, state);

    if (me->notify_hpd_changed)
        me->notify_hpd_changed(me->notify_data, state);
}

static void _hdmi_hpd_work(struct work_struct *work)
{
    int state;
    struct nxp_hdmi_context *me = container_of(work, struct nxp_hdmi_context, hpd_work.work);
    state = hdmi_hpd_status();
    if (!nxp_cpu_version()) {
        /* no revision */
        /* state is invert!!! */
        state = !state;
    }
    _hdmi_hpd_changed(me, state);
}

/**
 * public functions
 */
int hdmi_init_context(struct nxp_hdmi_context *me,
        struct nxp_v4l2_i2c_board_info *edid, struct nxp_v4l2_i2c_board_info *hdcp)
{
    int ret = 0;

    hdmi_set_base((void *)IO_ADDRESS(NX_HDMI_GetPhysicalAddress(0)));

#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
    /* init HDCP */
    ret = nxp_hdcp_init(&me->hdcp, hdcp);
    if (ret < 0) {
        pr_err("%s: failed to nxp_hdcp_init()\n", __func__);
        goto error_hdcp_init;
    }
#endif

    /* init EDID */
    ret = nxp_edid_init(&me->edid, edid);
    if (ret < 0) {
        pr_err("%s: failed to nxp_edid_init()\n", __func__);
        goto error_edid_init;
    }

    /* init PHY */
    ret = nxp_hdmiphy_init(&me->phy);
    if (ret < 0) {
        pr_err("%s: failed to nxp_phy_init()\n", __func__);
        goto error_hdmiphy_init;
    }

    INIT_DELAYED_WORK(&me->hpd_work, _hdmi_hpd_work);

    mutex_init(&me->mutex);
    spin_lock_init(&me->lock_callback);
    INIT_LIST_HEAD(&me->callback_list);

    me->initialized = false;
    me->irq = NX_HDMI_GetInterruptNumber(0);
    // psw0523 add for s5p6818
    me->irq += 32;
    atomic_set(&me->state, HDMI_UNPLUGGED);

    me->audio_enable = true;
    me->audio_channel_count = 2;
    me->sample_rate = DEFAULT_SAMPLE_RATE;
    me->color_range = 3;
    me->bits_per_sample = DEFAULT_BITS_PER_SAMPLE;
    me->audio_codec = DEFAULT_AUDIO_CODEC;
    me->aspect = HDMI_ASPECT_RATIO_16_9;
    me->cur_preset = V4L2_DV_INVALID;

    me->hpd_switch.name = "hdmi";
    switch_dev_register(&me->hpd_switch);

    ret = request_irq(me->irq, _hdmi_irq_handler, IRQF_DISABLED, "hdmi-int", me);
    if (ret < 0) {
        pr_err("%s: failed to request_irq(%d)\n", __func__, me->irq);
        goto error_req_irq;
    }
    disable_irq(me->irq);

    _hdmi_initialize(me);
    enable_irq(me->irq);

#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
    /* prepare HDCP */
    ret = me->hdcp.prepare(&me->hdcp);
    if (ret < 0) {
        pr_err("%s: failed to hdcp prepare()\n", __func__);
        goto error_hdcp_prepare;
    }
#endif

    return 0;

#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
error_hdcp_prepare:
    free_irq(me->irq, me);
#endif
error_req_irq:
    switch_dev_unregister(&me->hpd_switch);

error_hdmiphy_init:
    nxp_hdmiphy_cleanup(&me->phy);
error_edid_init:
    nxp_edid_cleanup(&me->edid);
#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
error_hdcp_init:
    nxp_hdcp_cleanup(&me->hdcp);
#endif
    return ret;
}

void hdmi_deinit_context(struct nxp_hdmi_context *me)
{
    pr_debug("%s\n", __func__);
    cancel_delayed_work_sync(&me->hpd_work);
    free_irq(me->irq, me);
    switch_dev_unregister(&me->hpd_switch);
    nxp_hdmiphy_cleanup(&me->phy);
    nxp_edid_cleanup(&me->edid);
#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
    nxp_hdcp_cleanup(&me->hdcp);
#endif
}

void hdmi_get_vsync(struct nxp_hdmi_context *me)
{
    _get_vsync_info(me);
}

void hdmi_hook_set_mux(struct nxp_hdmi_context *me, void (*hook)(struct nxp_hdmi_context *))
{
     me->set_hdmi_mux = hook;
}

int hdmi_run(struct nxp_hdmi_context *me, bool set_remote_sync)
{
    int ret;

    printk("%s entered\n", __func__);

    if (atomic_read(&me->state) == HDMI_RUNNING) {
        pr_warning("%s: hdmi already running!\n", __func__);
        return 0;
    }

    _hdmi_reset(me);

    /**
     * [SEQ 5] set up the HDMI PHY to specific video clock.
     */
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
    /*_hdmi_reset(me);*/

    /**
     * [SEQ 9] Wait for HDMI PHY ready (wait until 0xC0200020.[0], 1)
     */
    if (false == _wait_for_hdmiphy_ready()) {
        pr_err("%s: failed to wait for hdmiphy ready\n", __func__);
        me->phy.s_stream(&me->phy, 0);
        return -EIO;
    }

    if (me->set_hdmi_mux) {
        me->set_hdmi_mux(me);
    } else {
        pr_warn("%s: set_hdmi_mux hook callback not set, use RESC\n", __func__);
        me->source_dpc_module_num = 1;
        me->source_device = DISP_DEVICE_SYNCGEN0;
        NX_DISPLAYTOP_SetHDMIMUX(CTRUE, ResolutionConv);
    }

    /**
     * [SEC 10] Set the DPC CLKGEN’s Source Clock to HDMI_CLK & Set Sync Parameter
     */
    _set_hdmi_clkgen(me); /* set hdmi link clk to clkgen : default is hdmi phy clk */
    _set_audio_clkgen(me);

    _hdmi_sync_init(me);

    /**
     * [SEQ 11] Set up the HDMI Converter parameters
     */

    if (set_remote_sync)
        _set_remote_sync(me);

    _config_hdmi(me);

    hdmi_set_infoframe(me);
    _hdmi_set_packets(me);

#if defined(CONFIG_NXP_HDMI_AUDIO_I2S) || defined(CONFIG_NXP_DISPLAY_HDMI_AUDIO_I2S)
    hdmi_audio_i2s_init(me->sample_rate, me->bits_per_sample);
#else
    /* default is SPDIF */
    hdmi_audio_spdif_init(me->audio_codec, me->bits_per_sample);
#endif

    if (me->audio_enable)
        hdmi_audio_enable(true);

    /* hdmi_set_dvi_mode(me->is_dvi); */

    _hdmi_enable(me);

    /*mdelay(5);*/

#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
    /* start HDCP */
    ret = me->hdcp.start(&me->hdcp);
    if (ret < 0) {
        pr_err("%s: hdcp start error(%d)\n", __func__, ret);
        return ret;
    }
#endif

    atomic_set(&me->state, HDMI_RUNNING);
    printk("%s exit\n", __func__);
    return 0;
}

void hdmi_stop(struct nxp_hdmi_context *me)
{
    pr_debug("%s\n", __func__);
    printk("%s entered\n", __func__);

    if (atomic_read(&me->state) != HDMI_RUNNING)
        return;

#if defined(CONFIG_NXP_HDMI_USE_HDCP) || defined(CONFIG_NXP_DISPLAY_HDMI_USE_HDCP)
    me->hdcp.stop(&me->hdcp);
#endif

    _hdmi_sync_init(me);
    mdelay(100);

    hdmi_audio_enable(false);
    hdmi_enable(false);
    hdmi_tg_enable(false, me->cur_conf->mbus_fmt.field == V4L2_FIELD_INTERLACED);

    me->phy.s_stream(&me->phy, 0);

    if (DISP_DEVICE_RESCONV != me->source_device) {
        nxp_soc_disp_device_enable(me->source_device, 0);
        nxp_soc_disp_device_disconnect(DISP_DEVICE_HDMI, me->source_device);
    }

    _h_sync_offset = 0;

    atomic_set(&me->state, HDMI_STOPPED);
    printk("%s exit\n", __func__);
}

#if 0
void hdmi_stop_dummy(void)
{
    struct nxp_hdmi_context *me = __me;

    _hdmi_sync_init(me);
    mdelay(100);

    hdmi_enable(false);

    /*atomic_set(&me->state, HDMI_STOPPED);*/
}

void hdmi_start_dummy(void)
{
    struct nxp_hdmi_context *me = __me;
    _hdmi_enable(me);
}
#endif

void hdmi_enable_audio(struct nxp_hdmi_context *me, bool enable)
{
    me->audio_enable = enable;
    hdmi_audio_enable(me->audio_enable);
}

void hdmi_enable_pattern_gen(struct nxp_hdmi_context *me, bool enable)
{
    if (enable)
        NX_HDMI_SetReg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x1);
    else
        NX_HDMI_SetReg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x0);
}

void hdmi_set_infoframe(struct nxp_hdmi_context *me)
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
}

/*int hdmi_set_dv_preset(struct nxp_hdmi_context *me, u32 preset)*/
int hdmi_set_dv_preset(struct nxp_hdmi_context *me, struct v4l2_dv_preset *preset)
{
    me->cur_preset = preset->preset;
    me->cur_conf = (struct hdmi_preset_conf *)_hdmi_preset2conf(me->cur_preset);
    return me->phy.s_dv_preset(&me->phy, preset);
}

int  hdmi_set_preset(struct nxp_hdmi_context *me, uint32_t preset_val)
{
    struct v4l2_dv_preset preset;
    me->cur_preset = preset_val;
    memset(&preset, 0, sizeof(struct v4l2_dv_preset));
    preset.preset = preset_val;
    me->cur_conf = (struct hdmi_preset_conf *)_hdmi_preset2conf(me->cur_preset);
    return hdmi_set_dv_preset(me, &preset);
}

bool hdmi_is_connected(void)
{
    int state = hdmi_hpd_status();
    if (!nxp_cpu_version()) {
        /* no revision */
        /* state is invert!!! */
        state = !state;
    }
    if (state) return true;
    return false;
}

uint32_t hdmi_get_edid_preset(struct nxp_hdmi_context *me, uint32_t preferred_preset)
{
    if (me->edid.update(&me->edid) < 0) {
        pr_err("%s: failed to edid update\n", __func__);
        return preferred_preset;
    }

    if(me->edid.supports_preset(&me->edid, preferred_preset)) {
        pr_err("%s: preferred_preset is supported\n", __func__);
        return preferred_preset;
    }

    printk("%s: preferred_preset %d is not supported, use edid preferred preset\n", __func__, preferred_preset);
    return me->edid.preferred_preset(&me->edid);
}

struct hdmi_irq_callback *hdmi_register_irq_callback(int irq_num, int (*callback)(void *), void *data)
{
    if (!__me) {
        pr_err("%s: nxp_hdmi_context not exist!!!\n", __func__);
        return NULL;
    } else {
        unsigned long flags;
        struct nxp_hdmi_context *me = __me;
        struct hdmi_irq_callback *entry = (struct hdmi_irq_callback *)kmalloc(sizeof(struct hdmi_irq_callback), GFP_KERNEL);
        if (!entry) {
            printk("%s: failed to allocate hdmi_irq_callback entry\n", __func__);
            return NULL;
        }
        entry->irq_num = irq_num;
        entry->callback = callback;
        entry->data = data;
        spin_lock_irqsave(&me->lock_callback, flags);
        list_add_tail(&entry->list, &me->callback_list);
        spin_unlock_irqrestore(&me->lock_callback, flags);
        return entry;
    }
}

void hdmi_unregister_irq_callback(struct hdmi_irq_callback *entry)
{
    unsigned long flags;
    struct nxp_hdmi_context *me = __me;
    spin_lock_irqsave(&me->lock_callback, flags);
    list_del(&entry->list);
    spin_unlock_irqrestore(&me->lock_callback, flags);
    kfree(entry);
}

void hdmi_register_notifier_hpd_changed(struct nxp_hdmi_context *me, void (*notify)(void *, int), void *data)
{
    me->notify_hpd_changed = notify;
    me->notify_data = data;
}

int hdmi_suspend(struct nxp_hdmi_context *me)
{
    PM_DBGOUT("+%s\n", __func__);
    me->initialized = false;
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

int hdmi_resume(struct nxp_hdmi_context *me)
{
    PM_DBGOUT("+%s\n", __func__);
    _hdmi_initialize(me);
    if (atomic_read(&me->state) == HDMI_RUNNING)
        hdmi_run(me, true);
    else if (hdmi_hpd_status())
        _hdmi_hpd_changed(me, 1);
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
