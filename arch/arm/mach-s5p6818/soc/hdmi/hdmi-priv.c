#include <linux/delay.h>
#include <mach/hdmi/regs-hdmi.h>
#include <mach/hdmi/hdmi-priv.h>
#include <mach/hdmi/hdmi-preset.h>

void __iomem* hdmi_priv_io_base = NULL;

static void hdmi_reg_init(int color_range)
{
    /* enable HPD interrupts */
    hdmi_write_mask(HDMI_INTC_CON_0, ~0, HDMI_INTC_EN_GLOBAL | HDMI_INTC_EN_HPD_PLUG | HDMI_INTC_EN_HPD_UNPLUG);
    /* choose HDMI mode */
    hdmi_write_mask(HDMI_MODE_SEL, HDMI_MODE_HDMI_EN, HDMI_MODE_MASK);
    /* disable bluescreen */
    hdmi_write_mask(HDMI_CON_0, 0, HDMI_BLUE_SCR_EN);
    /* enc option on */
    hdmi_write_mask(HDMI_CON_0, HDMI_ENC_OPTION, HDMI_ENC_OPTION);
    /* enable AVI packet every vsync, fixes purple line problem */
    hdmi_writeb(HDMI_AVI_CON, HDMI_AVI_CON_EVERY_VSYNC);
    /* RGB888 is default output format of HDMI,
     * look to CEA-861-D, table 7 for more detail */
    hdmi_writeb(HDMI_AVI_BYTE(1), 0 << 5);
    if (color_range == 0 || color_range == 2)
        hdmi_write_mask(HDMI_CON_1, 0, 3 << 5);
    else
        hdmi_write_mask(HDMI_CON_1, 1 << 5, 3 << 5);
}

void hdmi_set_dvi_mode(bool is_dvi)
{
    u32 val;

    hdmi_write_mask(HDMI_MODE_SEL, is_dvi ? HDMI_MODE_DVI_EN :
            HDMI_MODE_HDMI_EN, HDMI_MODE_MASK);

    if (is_dvi)
        val = HDMI_VID_PREAMBLE_DIS | HDMI_GUARD_BAND_DIS;
    else
        val = HDMI_VID_PREAMBLE_EN | HDMI_GUARD_BAND_EN;
    hdmi_write(HDMI_CON_2, val);
}

/** must be called after set phy preset */
void hdmi_conf_apply(int color_range, const struct hdmi_preset_conf *conf)
{
    const struct hdmi_core_regs *core = &conf->core;
    /* const struct hdmi_tg_regs *tg = &conf->tg; */

    hdmi_reg_init(color_range);

    /* setting core registers */
    hdmi_writeb(HDMI_H_BLANK_0, core->h_blank[0]);
    hdmi_writeb(HDMI_H_BLANK_1, core->h_blank[1]);
    hdmi_writeb(HDMI_V2_BLANK_0, core->v2_blank[0]);
    hdmi_writeb(HDMI_V2_BLANK_1, core->v2_blank[1]);
    hdmi_writeb(HDMI_V1_BLANK_0, core->v1_blank[0]);
    hdmi_writeb(HDMI_V1_BLANK_1, core->v1_blank[1]);
    hdmi_writeb(HDMI_V_LINE_0, core->v_line[0]);
    hdmi_writeb(HDMI_V_LINE_1, core->v_line[1]);
    hdmi_writeb(HDMI_H_LINE_0, core->h_line[0]);
    hdmi_writeb(HDMI_H_LINE_1, core->h_line[1]);
    hdmi_writeb(HDMI_HSYNC_POL, core->hsync_pol[0]);
    hdmi_writeb(HDMI_VSYNC_POL, core->vsync_pol[0]);
    hdmi_writeb(HDMI_INT_PRO_MODE, core->int_pro_mode[0]);
    hdmi_writeb(HDMI_V_BLANK_F0_0, core->v_blank_f0[0]);
    hdmi_writeb(HDMI_V_BLANK_F0_1, core->v_blank_f0[1]);
    hdmi_writeb(HDMI_V_BLANK_F1_0, core->v_blank_f1[0]);
    hdmi_writeb(HDMI_V_BLANK_F1_1, core->v_blank_f1[1]);
    hdmi_writeb(HDMI_H_SYNC_START_0, core->h_sync_start[0]);
    hdmi_writeb(HDMI_H_SYNC_START_1, core->h_sync_start[1]);
    hdmi_writeb(HDMI_H_SYNC_END_0, core->h_sync_end[0]);
    hdmi_writeb(HDMI_H_SYNC_END_1, core->h_sync_end[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_BEF_2_0, core->v_sync_line_bef_2[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_BEF_2_1, core->v_sync_line_bef_2[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_BEF_1_0, core->v_sync_line_bef_1[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_BEF_1_1, core->v_sync_line_bef_1[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_2_0, core->v_sync_line_aft_2[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_2_1, core->v_sync_line_aft_2[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_1_0, core->v_sync_line_aft_1[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_1_1, core->v_sync_line_aft_1[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_2_0, core->v_sync_line_aft_pxl_2[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_2_1, core->v_sync_line_aft_pxl_2[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_1_0, core->v_sync_line_aft_pxl_1[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_1_1, core->v_sync_line_aft_pxl_1[1]);
    hdmi_writeb(HDMI_V_BLANK_F2_0, core->v_blank_f2[0]);
    hdmi_writeb(HDMI_V_BLANK_F2_1, core->v_blank_f2[1]);
    hdmi_writeb(HDMI_V_BLANK_F3_0, core->v_blank_f3[0]);
    hdmi_writeb(HDMI_V_BLANK_F3_1, core->v_blank_f3[1]);
    hdmi_writeb(HDMI_V_BLANK_F4_0, core->v_blank_f4[0]);
    hdmi_writeb(HDMI_V_BLANK_F4_1, core->v_blank_f4[1]);
    hdmi_writeb(HDMI_V_BLANK_F5_0, core->v_blank_f5[0]);
    hdmi_writeb(HDMI_V_BLANK_F5_1, core->v_blank_f5[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_3_0, core->v_sync_line_aft_3[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_3_1, core->v_sync_line_aft_3[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_4_0, core->v_sync_line_aft_4[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_4_1, core->v_sync_line_aft_4[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_5_0, core->v_sync_line_aft_5[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_5_1, core->v_sync_line_aft_5[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_6_0, core->v_sync_line_aft_6[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_6_1, core->v_sync_line_aft_6[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_3_0, core->v_sync_line_aft_pxl_3[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_3_1, core->v_sync_line_aft_pxl_3[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_4_0, core->v_sync_line_aft_pxl_4[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_4_1, core->v_sync_line_aft_pxl_4[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_5_0, core->v_sync_line_aft_pxl_5[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_5_1, core->v_sync_line_aft_pxl_5[1]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_6_0, core->v_sync_line_aft_pxl_6[0]);
    hdmi_writeb(HDMI_V_SYNC_LINE_AFT_PXL_6_1, core->v_sync_line_aft_pxl_6[1]);
    hdmi_writeb(HDMI_VACT_SPACE_1_0, core->vact_space_1[0]);
    hdmi_writeb(HDMI_VACT_SPACE_1_1, core->vact_space_1[1]);
    hdmi_writeb(HDMI_VACT_SPACE_2_0, core->vact_space_2[0]);
    hdmi_writeb(HDMI_VACT_SPACE_2_1, core->vact_space_2[1]);
    hdmi_writeb(HDMI_VACT_SPACE_3_0, core->vact_space_3[0]);
    hdmi_writeb(HDMI_VACT_SPACE_3_1, core->vact_space_3[1]);
    hdmi_writeb(HDMI_VACT_SPACE_4_0, core->vact_space_4[0]);
    hdmi_writeb(HDMI_VACT_SPACE_4_1, core->vact_space_4[1]);
    hdmi_writeb(HDMI_VACT_SPACE_5_0, core->vact_space_5[0]);
    hdmi_writeb(HDMI_VACT_SPACE_5_1, core->vact_space_5[1]);
    hdmi_writeb(HDMI_VACT_SPACE_6_0, core->vact_space_6[0]);
    hdmi_writeb(HDMI_VACT_SPACE_6_1, core->vact_space_6[1]);

    /* Timing generator registers */
#if 0
    hdmi_writeb(HDMI_TG_H_FSZ_L, tg->h_fsz_l);
    hdmi_writeb(HDMI_TG_H_FSZ_H, tg->h_fsz_h);
    hdmi_writeb(HDMI_TG_HACT_ST_L, tg->hact_st_l);
    hdmi_writeb(HDMI_TG_HACT_ST_H, tg->hact_st_h);
    hdmi_writeb(HDMI_TG_HACT_SZ_L, tg->hact_sz_l);
    hdmi_writeb(HDMI_TG_HACT_SZ_H, tg->hact_sz_h);
    hdmi_writeb(HDMI_TG_V_FSZ_L, tg->v_fsz_l);
    hdmi_writeb(HDMI_TG_V_FSZ_H, tg->v_fsz_h);
    hdmi_writeb(HDMI_TG_VSYNC_L, tg->vsync_l);
    hdmi_writeb(HDMI_TG_VSYNC_H, tg->vsync_h);
    hdmi_writeb(HDMI_TG_VSYNC2_L, tg->vsync2_l);
    hdmi_writeb(HDMI_TG_VSYNC2_H, tg->vsync2_h);
    hdmi_writeb(HDMI_TG_VACT_ST_L, tg->vact_st_l);
    hdmi_writeb(HDMI_TG_VACT_ST_H, tg->vact_st_h);
    hdmi_writeb(HDMI_TG_VACT_SZ_L, tg->vact_sz_l);
    hdmi_writeb(HDMI_TG_VACT_SZ_H, tg->vact_sz_h);
    hdmi_writeb(HDMI_TG_FIELD_CHG_L, tg->field_chg_l);
    hdmi_writeb(HDMI_TG_FIELD_CHG_H, tg->field_chg_h);
    hdmi_writeb(HDMI_TG_VACT_ST2_L, tg->vact_st2_l);
    hdmi_writeb(HDMI_TG_VACT_ST2_H, tg->vact_st2_h);
    hdmi_writeb(HDMI_TG_VACT_ST3_L, tg->vact_st3_l);
    hdmi_writeb(HDMI_TG_VACT_ST3_H, tg->vact_st3_h);
    hdmi_writeb(HDMI_TG_VACT_ST4_L, tg->vact_st4_l);
    hdmi_writeb(HDMI_TG_VACT_ST4_H, tg->vact_st4_h);
    hdmi_writeb(HDMI_TG_VSYNC_TOP_HDMI_L, tg->vsync_top_hdmi_l);
    hdmi_writeb(HDMI_TG_VSYNC_TOP_HDMI_H, tg->vsync_top_hdmi_h);
    hdmi_writeb(HDMI_TG_VSYNC_BOT_HDMI_L, tg->vsync_bot_hdmi_l);
    hdmi_writeb(HDMI_TG_VSYNC_BOT_HDMI_H, tg->vsync_bot_hdmi_h);
    hdmi_writeb(HDMI_TG_FIELD_TOP_HDMI_L, tg->field_top_hdmi_l);
    hdmi_writeb(HDMI_TG_FIELD_TOP_HDMI_H, tg->field_top_hdmi_h);
    hdmi_writeb(HDMI_TG_FIELD_BOT_HDMI_L, tg->field_bot_hdmi_l);
    hdmi_writeb(HDMI_TG_FIELD_BOT_HDMI_H, tg->field_bot_hdmi_h);
    hdmi_writeb(HDMI_TG_3D, tg->tg_3d);
#endif
}

bool hdmi_is_phy_ready(void)
{
    u32 val = hdmi_read(HDMI_PHY_STATUS_0);
    if (val & HDMI_PHY_STATUS_READY)
        return true;

    return false;
}

void hdmi_enable(bool on)
{
    if (on)
        hdmi_write_mask(HDMI_CON_0, ~0, HDMI_EN);
    else
        hdmi_write_mask(HDMI_CON_0, 0, HDMI_EN);
}

void hdmi_hpd_enable(bool on)
{
    /* enable HPD interrupts */
    if (on)
        hdmi_write_mask(HDMI_INTC_CON_0, ~0, HDMI_INTC_EN_GLOBAL |
                HDMI_INTC_EN_HPD_PLUG | HDMI_INTC_EN_HPD_UNPLUG);
}

void hdmi_tg_enable(bool on, bool is_interlace)
{
    u32 mask;

    mask = is_interlace ? HDMI_TG_EN | HDMI_FIELD_EN : HDMI_TG_EN;

#if 0
    if (on)
        hdmi_write_mask(HDMI_TG_CMD, ~0, mask);
    else
        hdmi_write_mask(HDMI_TG_CMD, 0, mask);
#endif
}

void hdmi_stop_vsi(void)
{
    hdmi_writeb(HDMI_VSI_CON, HDMI_VSI_CON_DO_NOT_TRANSMIT);
}

u8 hdmi_chksum(u32 start, u8 len, u32 hdr_sum)
{
    int i;

    /* hdr_sum : header0 + header1 + header2
     * start : start address of packet byte1
     * len : packet bytes - 1 */
    for (i = 0; i < len; ++i)
        hdr_sum += hdmi_read(start + i * 4);

    return (u8)(0x100 - (hdr_sum & 0xff));
}

void hdmi_set_acr(int sample_rate, bool is_dvi)
{
    u32 n, cts;

    if (is_dvi) {
        hdmi_write(HDMI_ACR_CON, HDMI_ACR_CON_TX_MODE_NO_TX);
        return;
    }

    if (sample_rate == 32000) {
        n = 4096;
        cts = 27000;
    } else if (sample_rate == 44100) {
        n = 6272;
        cts = 30000;
    } else if (sample_rate == 48000) {
        n = 6144;
        cts = 27000;
    } else if (sample_rate == 88200) {
        n = 12544;
        cts = 30000;
    } else if (sample_rate == 96000) {
        n = 12288;
        cts = 27000;
    } else if (sample_rate == 176400) {
        n = 25088;
        cts = 30000;
    } else if (sample_rate == 192000) {
        n = 24576;
        cts = 27000;
    } else {
        n = 0;
        cts = 0;
    }

    hdmi_write(HDMI_ACR_N0, HDMI_ACR_N0_VAL(n));
    hdmi_write(HDMI_ACR_N1, HDMI_ACR_N1_VAL(n));
    hdmi_write(HDMI_ACR_N2, HDMI_ACR_N2_VAL(n));

    /* transfer ACR packet */
    hdmi_write(HDMI_ACR_CON, HDMI_ACR_CON_TX_MODE_MESURED_CTS);
}

void hdmi_audio_spdif_init(int audio_codec, int bits_per_sample)
{
    u32 val;
    int bps, rep_time;

    hdmi_write(HDMI_I2S_CLK_CON, HDMI_I2S_CLK_ENABLE);

    val = HDMI_SPDIFIN_CFG_NOISE_FILTER_2_SAMPLE |
        HDMI_SPDIFIN_CFG_PCPD_MANUAL |
        HDMI_SPDIFIN_CFG_WORD_LENGTH_MANUAL |
        HDMI_SPDIFIN_CFG_UVCP_REPORT |
        HDMI_SPDIFIN_CFG_HDMI_2_BURST |
        HDMI_SPDIFIN_CFG_DATA_ALIGN_32;
    hdmi_write(HDMI_SPDIFIN_CONFIG_1, val);
    hdmi_write(HDMI_SPDIFIN_CONFIG_2, 0);

    bps = audio_codec == HDMI_AUDIO_PCM ? bits_per_sample : 16;
    rep_time = audio_codec == HDMI_AUDIO_AC3 ? 1536 * 2 - 1 : 0;
    val = HDMI_SPDIFIN_USER_VAL_REPETITION_TIME_LOW(rep_time) |
        HDMI_SPDIFIN_USER_VAL_WORD_LENGTH_24;
    hdmi_write(HDMI_SPDIFIN_USER_VALUE_1, val);
    val = HDMI_SPDIFIN_USER_VAL_REPETITION_TIME_HIGH(rep_time);
    hdmi_write(HDMI_SPDIFIN_USER_VALUE_2, val);
    hdmi_write(HDMI_SPDIFIN_USER_VALUE_3, 0);
    hdmi_write(HDMI_SPDIFIN_USER_VALUE_4, 0);

    val = HDMI_I2S_IN_ENABLE | HDMI_I2S_AUD_SPDIF | HDMI_I2S_MUX_ENABLE;
    hdmi_write(HDMI_I2S_IN_MUX_CON, val);

    hdmi_write(HDMI_I2S_MUX_CH, HDMI_I2S_CH_ALL_EN);
    hdmi_write(HDMI_I2S_MUX_CUV, HDMI_I2S_CUV_RL_EN);

    hdmi_write_mask(HDMI_SPDIFIN_CLK_CTRL, 0, HDMI_SPDIFIN_CLK_ON);
    hdmi_write_mask(HDMI_SPDIFIN_CLK_CTRL, ~0, HDMI_SPDIFIN_CLK_ON);

    hdmi_write(HDMI_SPDIFIN_OP_CTRL, HDMI_SPDIFIN_STATUS_CHECK_MODE);
    hdmi_write(HDMI_SPDIFIN_OP_CTRL, HDMI_SPDIFIN_STATUS_CHECK_MODE_HDMI);
}

void hdmi_audio_i2s_init(int sample_rate, int bps)
{
    u32 data_num, bit_ch, sample_frq, val;

    if (bps == 16) {
        data_num = 1;
        bit_ch = 0;
    } else if (bps == 20) {
        data_num = 2;
        bit_ch  = 1;
    } else if (bps == 24) {
        data_num = 3;
        bit_ch  = 1;
    } else if (bps == 32) {
        data_num = 1;
        bit_ch  = 2;
    } else {
        data_num = 1;
        bit_ch = 0;
    }

    /* reset I2S */
    hdmi_write(HDMI_I2S_CLK_CON, HDMI_I2S_CLK_DISABLE);
    hdmi_write(HDMI_I2S_CLK_CON, HDMI_I2S_CLK_ENABLE);

    hdmi_write_mask(HDMI_I2S_DSD_CON, 0, HDMI_I2S_DSD_ENABLE);

    /* Configuration I2S input ports. Configure I2S_PIN_SEL_0~4 */
    val = HDMI_I2S_SEL_SCLK(5) | HDMI_I2S_SEL_LRCK(6);
    hdmi_write(HDMI_I2S_PIN_SEL_0, val);
    val = HDMI_I2S_SEL_SDATA1(3) | HDMI_I2S_SEL_SDATA0(4);
    hdmi_write(HDMI_I2S_PIN_SEL_1, val);
    val = HDMI_I2S_SEL_SDATA3(1) | HDMI_I2S_SEL_SDATA2(2);
    hdmi_write(HDMI_I2S_PIN_SEL_2, val);
    hdmi_write(HDMI_I2S_PIN_SEL_3, HDMI_I2S_SEL_DSD(0));

    /* I2S_CON_1 & 2 */
    val = HDMI_I2S_SCLK_FALLING_EDGE | HDMI_I2S_L_CH_LOW_POL;
    hdmi_write(HDMI_I2S_CON_1, val);
    val = HDMI_I2S_MSB_FIRST_MODE | HDMI_I2S_SET_BIT_CH(bit_ch) |
        HDMI_I2S_SET_SDATA_BIT(data_num) | HDMI_I2S_BASIC_FORMAT;
    hdmi_write(HDMI_I2S_CON_2, val);

    if (sample_rate == 32000)
        sample_frq = 0x3;
    else if (sample_rate == 44100)
        sample_frq = 0x0;
    else if (sample_rate == 48000)
        sample_frq = 0x2;
    else if (sample_rate == 96000)
        sample_frq = 0xa;
    else
        sample_frq = 0;

    /* Configure register related to CUV information */
    val = HDMI_I2S_CH_STATUS_MODE_0 | HDMI_I2S_2AUD_CH_WITHOUT_PREEMPH |
        HDMI_I2S_COPYRIGHT | HDMI_I2S_LINEAR_PCM |
        HDMI_I2S_CONSUMER_FORMAT;
    hdmi_write(HDMI_I2S_CH_ST_0, val);
    hdmi_write(HDMI_I2S_CH_ST_1, HDMI_I2S_CD_PLAYER);
    hdmi_writeb(HDMI_I2S_CH_ST_2, HDMI_I2S_SET_SOURCE_NUM(0) |
            HDMI_I2S_SET_CHANNEL_NUM(0x6));
    hdmi_writeb(HDMI_ASP_CON,
            HDMI_AUD_MODE_MULTI_CH | HDMI_AUD_SP_AUD2_EN |
            HDMI_AUD_SP_AUD1_EN | HDMI_AUD_SP_AUD0_EN);
    hdmi_writeb(HDMI_ASP_CHCFG0,
            HDMI_SPK0R_SEL_I_PCM0R | HDMI_SPK0L_SEL_I_PCM0L);
    hdmi_writeb(HDMI_ASP_CHCFG1,
            HDMI_SPK0R_SEL_I_PCM1L | HDMI_SPK0L_SEL_I_PCM1R);
    hdmi_writeb(HDMI_ASP_CHCFG2,
            HDMI_SPK0R_SEL_I_PCM2R | HDMI_SPK0L_SEL_I_PCM2L);
    hdmi_writeb(HDMI_ASP_CHCFG3,
            HDMI_SPK0R_SEL_I_PCM3R | HDMI_SPK0L_SEL_I_PCM3L);

    val = HDMI_I2S_CLK_ACCUR_LEVEL_1 |
        HDMI_I2S_SET_SAMPLING_FREQ(sample_frq);
    hdmi_write(HDMI_I2S_CH_ST_3, val);
    val = HDMI_I2S_ORG_SAMPLING_FREQ_44_1 |
        HDMI_I2S_WORD_LENGTH_MAX24_20BITS |
        HDMI_I2S_WORD_LENGTH_MAX_20BITS;
    hdmi_write(HDMI_I2S_CH_ST_4, val);

    hdmi_write(HDMI_I2S_CH_ST_CON, HDMI_I2S_CH_STATUS_RELOAD);

    val = HDMI_I2S_IN_ENABLE | HDMI_I2S_AUD_I2S | HDMI_I2S_CUV_I2S_ENABLE
        | HDMI_I2S_MUX_ENABLE;
    hdmi_write(HDMI_I2S_IN_MUX_CON, val);

    val = HDMI_I2S_CH0_L_EN | HDMI_I2S_CH0_R_EN | HDMI_I2S_CH1_L_EN |
        HDMI_I2S_CH1_R_EN | HDMI_I2S_CH2_L_EN | HDMI_I2S_CH2_R_EN |
        HDMI_I2S_CH3_L_EN | HDMI_I2S_CH3_R_EN;
    hdmi_write(HDMI_I2S_MUX_CH, val);

    val = HDMI_I2S_CUV_L_EN | HDMI_I2S_CUV_R_EN;
    hdmi_write(HDMI_I2S_MUX_CUV, val);
}

void hdmi_audio_enable(bool on)
{
    if (on)
        hdmi_write_mask(HDMI_CON_0, ~0, HDMI_ASP_ENABLE);
    else
        hdmi_write_mask(HDMI_CON_0, 0, HDMI_ASP_ENABLE);
}

void hdmi_bluescreen_enable(bool on)
{
    if (on)
        hdmi_write_mask(HDMI_CON_0, ~0, HDMI_BLUE_SCR_EN);
    else
        hdmi_write_mask(HDMI_CON_0, 0, HDMI_BLUE_SCR_EN);
}

void hdmi_mute(bool on)
{
    hdmi_bluescreen_enable(on);
    hdmi_audio_enable(!on);
}

int hdmi_hpd_status(void)
{
    return hdmi_read(HDMI_HPD_STATUS);
}

u8  hdmi_get_int_mask(void)
{
    return hdmi_readb(HDMI_INTC_CON_0);
}

void hdmi_set_int_mask(u8 mask, bool on)
{
    if (on) {
        mask |= HDMI_INTC_EN_GLOBAL;
        hdmi_write_mask(HDMI_INTC_CON_0, ~0, mask);
    } else
        hdmi_write_mask(HDMI_INTC_CON_0, 0,
                HDMI_INTC_EN_GLOBAL);

}

void hdmi_sw_hpd_enable(bool on)
{
    if (on)
        hdmi_write_mask(HDMI_HPD, ~0, HDMI_HPD_SEL_I_HPD);
    else
        hdmi_write_mask(HDMI_HPD, 0, HDMI_HPD_SEL_I_HPD);
}

void hdmi_sw_hpd_plug(bool on)
{
    if (on)
        hdmi_write_mask(HDMI_HPD, ~0, HDMI_SW_HPD_PLUGGED);
    else
        hdmi_write_mask(HDMI_HPD, 0, HDMI_SW_HPD_PLUGGED);
}

void hdmi_phy_sw_reset(void)
{
#if 0
    hdmi_write_mask(HDMI_PHY_RSTOUT, ~0, HDMI_PHY_SW_RSTOUT);
    mdelay(10);
    hdmi_write_mask(HDMI_PHY_RSTOUT,  0, HDMI_PHY_SW_RSTOUT);
#endif
}

void hdmi_sw_reset(void)
{
#if 0
    hdmi_write_mask(HDMI_CORE_RSTOUT, 0, HDMI_CORE_SW_RSTOUT);
    mdelay(10);
    hdmi_write_mask(HDMI_CORE_RSTOUT, ~0, HDMI_CORE_SW_RSTOUT);
#endif
}

void hdmi_dump_regs(const char *prefix)
{
#define DUMPREG(reg_id) \
    printk("%s:" #reg_id " = %08x\n", prefix, \
            readl(hdmi_priv_io_base + reg_id))

    int i;

    printk("%s: ---- CONTROL REGISTERS ----\n", prefix);
    DUMPREG(HDMI_INTC_CON_0);
    DUMPREG(HDMI_INTC_FLAG_0);
    DUMPREG(HDMI_HPD_STATUS);
    DUMPREG(HDMI_INTC_CON_1);
    DUMPREG(HDMI_INTC_FLAG_1);
    DUMPREG(HDMI_PHY_STATUS_0);
    DUMPREG(HDMI_PHY_STATUS_PLL);
    DUMPREG(HDMI_PHY_CON_0);
    /* DUMPREG(HDMI_PHY_RSTOUT); */
    /* DUMPREG(HDMI_PHY_VPLL); */
    /* DUMPREG(HDMI_PHY_CMU); */
    /* DUMPREG(HDMI_CORE_RSTOUT); */

    printk("%s: ---- CORE REGISTERS ----\n", prefix);
    DUMPREG(HDMI_CON_0);
    DUMPREG(HDMI_CON_1);
    DUMPREG(HDMI_CON_2);
    DUMPREG(HDMI_STATUS);
    /* DUMPREG(HDMI_PHY_STATUS); */
    DUMPREG(HDMI_STATUS_EN);
    DUMPREG(HDMI_HPD);
    DUMPREG(HDMI_MODE_SEL);
    DUMPREG(HDMI_ENC_EN);
    DUMPREG(HDMI_DC_CONTROL);
    DUMPREG(HDMI_VIDEO_PATTERN_GEN);

    printk("%s: ---- CORE SYNC REGISTERS ----\n", prefix);
    DUMPREG(HDMI_H_BLANK_0);
    DUMPREG(HDMI_H_BLANK_1);
    DUMPREG(HDMI_V2_BLANK_0);
    DUMPREG(HDMI_V2_BLANK_1);
    DUMPREG(HDMI_V1_BLANK_0);
    DUMPREG(HDMI_V1_BLANK_1);
    DUMPREG(HDMI_V_LINE_0);
    DUMPREG(HDMI_V_LINE_1);
    DUMPREG(HDMI_H_LINE_0);
    DUMPREG(HDMI_H_LINE_1);
    DUMPREG(HDMI_HSYNC_POL);

    DUMPREG(HDMI_VSYNC_POL);
    DUMPREG(HDMI_INT_PRO_MODE);
    DUMPREG(HDMI_V_BLANK_F0_0);
    DUMPREG(HDMI_V_BLANK_F0_1);
    DUMPREG(HDMI_V_BLANK_F1_0);
    DUMPREG(HDMI_V_BLANK_F1_1);

    DUMPREG(HDMI_H_SYNC_START_0);
    DUMPREG(HDMI_H_SYNC_START_1);
    DUMPREG(HDMI_H_SYNC_END_0);
    DUMPREG(HDMI_H_SYNC_END_1);

    DUMPREG(HDMI_V_SYNC_LINE_BEF_2_0);
    DUMPREG(HDMI_V_SYNC_LINE_BEF_2_1);
    DUMPREG(HDMI_V_SYNC_LINE_BEF_1_0);
    DUMPREG(HDMI_V_SYNC_LINE_BEF_1_1);

    DUMPREG(HDMI_V_SYNC_LINE_AFT_2_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_2_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_1_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_1_1);

    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_2_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_2_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_1_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_1_1);

    DUMPREG(HDMI_V_BLANK_F2_0);
    DUMPREG(HDMI_V_BLANK_F2_1);
    DUMPREG(HDMI_V_BLANK_F3_0);
    DUMPREG(HDMI_V_BLANK_F3_1);
    DUMPREG(HDMI_V_BLANK_F4_0);
    DUMPREG(HDMI_V_BLANK_F4_1);
    DUMPREG(HDMI_V_BLANK_F5_0);
    DUMPREG(HDMI_V_BLANK_F5_1);

    DUMPREG(HDMI_V_SYNC_LINE_AFT_3_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_3_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_4_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_4_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_5_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_5_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_6_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_6_1);

    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_3_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_3_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_4_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_4_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_5_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_5_1);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_6_0);
    DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_6_1);

    DUMPREG(HDMI_VACT_SPACE_1_0);
    DUMPREG(HDMI_VACT_SPACE_1_1);
    DUMPREG(HDMI_VACT_SPACE_2_0);
    DUMPREG(HDMI_VACT_SPACE_2_1);
    DUMPREG(HDMI_VACT_SPACE_3_0);
    DUMPREG(HDMI_VACT_SPACE_3_1);
    DUMPREG(HDMI_VACT_SPACE_4_0);
    DUMPREG(HDMI_VACT_SPACE_4_1);
    DUMPREG(HDMI_VACT_SPACE_5_0);
    DUMPREG(HDMI_VACT_SPACE_5_1);
    DUMPREG(HDMI_VACT_SPACE_6_0);
    DUMPREG(HDMI_VACT_SPACE_6_1);

    printk("%s: ---- TG REGISTERS ----\n", prefix);
#if 0
    DUMPREG(HDMI_TG_CMD);
    DUMPREG(HDMI_TG_H_FSZ_L);
    DUMPREG(HDMI_TG_H_FSZ_H);
    DUMPREG(HDMI_TG_HACT_ST_L);
    DUMPREG(HDMI_TG_HACT_ST_H);
    DUMPREG(HDMI_TG_HACT_SZ_L);
    DUMPREG(HDMI_TG_HACT_SZ_H);
    DUMPREG(HDMI_TG_V_FSZ_L);
    DUMPREG(HDMI_TG_V_FSZ_H);
    DUMPREG(HDMI_TG_VSYNC_L);
    DUMPREG(HDMI_TG_VSYNC_H);
    DUMPREG(HDMI_TG_VSYNC2_L);
    DUMPREG(HDMI_TG_VSYNC2_H);
    DUMPREG(HDMI_TG_VACT_ST_L);
    DUMPREG(HDMI_TG_VACT_ST_H);
    DUMPREG(HDMI_TG_VACT_SZ_L);
    DUMPREG(HDMI_TG_VACT_SZ_H);
    DUMPREG(HDMI_TG_FIELD_CHG_L);
    DUMPREG(HDMI_TG_FIELD_CHG_H);
    DUMPREG(HDMI_TG_VACT_ST2_L);
    DUMPREG(HDMI_TG_VACT_ST2_H);
    DUMPREG(HDMI_TG_VACT_ST3_L);
    DUMPREG(HDMI_TG_VACT_ST3_H);
    DUMPREG(HDMI_TG_VACT_ST4_L);
    DUMPREG(HDMI_TG_VACT_ST4_H);
    DUMPREG(HDMI_TG_VSYNC_TOP_HDMI_L);
    DUMPREG(HDMI_TG_VSYNC_TOP_HDMI_H);
    DUMPREG(HDMI_TG_VSYNC_BOT_HDMI_L);
    DUMPREG(HDMI_TG_VSYNC_BOT_HDMI_H);
    DUMPREG(HDMI_TG_FIELD_TOP_HDMI_L);
    DUMPREG(HDMI_TG_FIELD_TOP_HDMI_H);
    DUMPREG(HDMI_TG_FIELD_BOT_HDMI_L);
    DUMPREG(HDMI_TG_FIELD_BOT_HDMI_H);
    DUMPREG(HDMI_TG_3D);
#endif

    printk("%s: ---- PACKET REGISTERS ----\n", prefix);
    DUMPREG(HDMI_AVI_CON);
    DUMPREG(HDMI_AVI_HEADER0);
    DUMPREG(HDMI_AVI_HEADER1);
    DUMPREG(HDMI_AVI_HEADER2);
    DUMPREG(HDMI_AVI_CHECK_SUM);
    for (i = 1; i < 6; ++i)
        DUMPREG(HDMI_AVI_BYTE(i));

    DUMPREG(HDMI_VSI_CON);
    DUMPREG(HDMI_VSI_HEADER0);
    DUMPREG(HDMI_VSI_HEADER1);
    DUMPREG(HDMI_VSI_HEADER2);
    for (i = 0; i < 7; ++i)
        DUMPREG(HDMI_VSI_DATA(i));
    DUMPREG(HDMI_AUI_CON);
    DUMPREG(HDMI_ACR_CON);

#undef DUMPREG
}
