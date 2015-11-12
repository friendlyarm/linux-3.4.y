/*
 * alc5633.c  --  ALC5633 ALSA Soc Audio driver
 *
 * Copyright 2011 Realtek Microelectronics
 *
 * Author: flove <flove@realtek.com>
 *
 * Based on WM8753.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include "alc5633.h"

/*
#define	VMID_ADD_WIDGET
*/

#define VERSION "0.1.0 alsa 1.0.25"

struct alc5633_init_reg {
	u8 reg;
	u16 val;
};

struct alc5633_priv {
        struct snd_soc_codec *codec;
        int codec_version;
        int master;
        int sysclk;
        int rate;
        int rx_rate;
        int bclk_rate;
        int dmic_used_flag;
        struct i2c_client *client;
};

static const u16 alc5633_reg[ALC5633_VENDOR_ID1 + 1] = {
#if 0
        [ALC5633_SPK_OUT_VOL] = 0x8888,
        [ALC5633_HP_OUT_VOL] = 0x8080,
        [ALC5633_MONO_AXO_1_2_VOL] = 0xa080,
        [ALC5633_AUX_IN_VOL] = 0x0808,
        [ALC5633_ADC_REC_MIXER] = 0xf0f0,
        //[ALC5633_VDAC_DIG_VOL] = 0x0010,  //for ALC5631V
        [ALC5633_OUTMIXER_L_CTRL] = 0xffc0,
        [ALC5633_OUTMIXER_R_CTRL] = 0xffc0,
        [ALC5633_AXO1MIXER_CTRL] = 0x88c0,
        [ALC5633_AXO2MIXER_CTRL] = 0x88c0,
        [ALC5633_DIG_MIC_CTRL] = 0x3000,
        [ALC5633_MONO_INPUT_VOL] = 0x8808,
        [ALC5633_SPK_MIXER_CTRL] = 0xf8f8,
        [ALC5633_SPK_MONO_OUT_CTRL] = 0xfc00,
        [ALC5633_SPK_MONO_HP_OUT_CTRL] = 0x4440,
        [ALC5633_SDP_CTRL] = 0x8000,
        //[ALC5633_MONO_SDP_CTRL] = 0x8000,     //for ALC5631V
        [ALC5633_STEREO_AD_DA_CLK_CTRL] = 0x2010,
        [ALC5633_GEN_PUR_CTRL_REG] = 0x0e00,
        [ALC5633_INT_ST_IRQ_CTRL_2] = 0x0710,
        [ALC5633_MISC_CTRL] = 0x2040,
        [ALC5633_DEPOP_FUN_CTRL_2] = 0x8000,
        [ALC5633_SOFT_VOL_CTRL] = 0x07e0,
        [ALC5633_ALC_CTRL_1] = 0x0206,
        [ALC5633_ALC_CTRL_3] = 0x2000,
        [ALC5633_PSEUDO_SPATL_CTRL] = 0x0553,
#endif
};


static struct alc5633_init_reg init_list[] = {
        {ALC5633_SPK_OUT_VOL            , 0xe000},//speaker output volume is 0db by default,
        {ALC5633_SPK_HP_MIXER_CTRL      , 0x0020},//HP from HP_VOL
        {ALC5633_HP_OUT_VOL             , 0xc0c0},//HP output volume is 0 db by default
        {ALC5633_AUXOUT_VOL             , 0x0010},//Auxout volume is 0db by default
        {ALC5633_REC_MIXER_CTRL         , 0x7d7d},//ADC Record Mixer Control
        {ALC5633_ADC_CTRL               , 0x000a},
        {ALC5633_MIC_CTRL_2             , 0x7700},//boost 40db
        {ALC5633_HPMIXER_CTRL           , 0x3e3e},//"HP Mixer Control"
//      {ALC5633_AUXMIXER_CTRL          , 0x3e3e},//"AUX Mixer Control"
        {ALC5633_SPKMIXER_CTRL          , 0x08fc},//"SPK Mixer Control"
        {ALC5633_SPK_AMP_CTRL           , 0x0000},
//      {ALC5633_GEN_PUR_CTRL_1         , 0x8C00}, //set spkratio to auto
        {ALC5633_ZC_SM_CTRL_1           , 0x0001},      //Disable Zero Cross
        {ALC5633_ZC_SM_CTRL_2           , 0x3000},      //Disable Zero cross
        {ALC5633_MIC_CTRL_1             , 0x8808}, //set mic1 to differnetial mode
        {ALC5633_DEPOP_CTRL_2           , 0xB000},
        {ALC5633_PRI_REG_ADD                , 0x0056},
        {ALC5633_PRI_REG_DATA           , 0x303f},
        {ALC5633_DIG_BEEP_IRQ_CTRL      , 0x01E0},
        {ALC5633_ALC_CTRL_1                 , 0x0808},
        {ALC5633_ALC_CTRL_2                 , 0x0003},
        {ALC5633_ALC_CTRL_3                 , 0xe081},
};

#define ALC5633_INIT_REG_LEN ARRAY_SIZE(init_list)

int alc5633_write_mask(struct snd_soc_codec *codec, unsigned short reg,
                         unsigned int value, unsigned int mask)
{
      printk("%s reg=0x%x, mask=0x%x, val=0x%x\n", __func__, reg, mask, value);
    return snd_soc_update_bits(codec, reg, mask, value);
}

//static struct snd_soc_device *alc5633_socdev;
//static struct snd_soc_codec *alc5633_codec;
/*
 * read alc5633 register cache
 */
static inline unsigned int alc5633_read_reg_cache(struct snd_soc_codec *codec,
        unsigned int reg)
{
        u16 *cache = codec->reg_cache;
        if (reg < 1 || reg > (ARRAY_SIZE(alc5633_reg) + 1))
                return -1;
        return cache[reg];
}


/*
 * write alc5633 register cache
 */

static inline void alc5633_write_reg_cache(struct snd_soc_codec *codec,
        unsigned int reg, unsigned int value)
{
        u16 *cache = codec->reg_cache;
        if (reg < 0 || reg > 0x7e)
                return;
        cache[reg] = value;
}


static int alc5633_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int val)
{
      printk("%s reg=0x%x, val=0x%x\n", __func__, reg, val);
        snd_soc_write(codec, reg, val);
        alc5633_write_reg_cache(codec, reg, val);

	return 0;
}

static unsigned int alc5633_read(struct snd_soc_codec *codec, unsigned int reg)
{
        return(snd_soc_read(codec, reg));
}

static int alc5633_reg_init(struct snd_soc_codec *codec)
{
	int i;

	for (i = 0; i < ALC5633_INIT_REG_LEN; i++)
		alc5633_write(codec, init_list[i].reg, init_list[i].val);

	return 0;
}

static int alc5633_write_index(struct snd_soc_codec *codec, unsigned int index,unsigned int value)
{
    unsigned char RetVal = 0;

    RetVal = alc5633_write(codec,ALC5633_PRI_REG_ADD,index);

    if(RetVal != 0)
      return RetVal;

    RetVal = alc5633_write(codec,ALC5633_PRI_REG_DATA,value);
    return RetVal;
}

unsigned int alc5633_read_index(struct snd_soc_codec *codec, unsigned int reg)
{
        unsigned int value = 0x0;
        alc5633_write(codec,ALC5633_PRI_REG_ADD,reg);
        value=alc5633_read(codec,ALC5633_PRI_REG_DATA);

        return value;
}

void alc5633_write_index_mask(struct snd_soc_codec *codec, unsigned int reg,unsigned int value,unsigned int mask)
{
        unsigned  int CodecData;

        if(!mask)
                return;

        if(mask!=0xffff)
         {
                CodecData=alc5633_read_index(codec,reg);
                CodecData&=~mask;
                CodecData|=(value&mask);
                alc5633_write_index(codec,reg,CodecData);
         }
        else
        {
                alc5633_write_index(codec,reg,value);
        }
}

static int alc5633_reset(struct snd_soc_codec *codec)
{
	return alc5633_write(codec, ALC5633_RESET, 0);
}

#if 0
static int alc5633_volatile_register(struct snd_soc_codec *codec,
				    unsigned int reg)
{
	switch (reg) {
	case ALC5633_RESET:
	case ALC5633_INT_ST_IRQ_CTRL_2:
	case ALC5633_INDEX_ADD:
	case ALC5633_INDEX_DATA:
	case ALC5633_EQ_CTRL:
	//case ALC5633_VENDOR_ID:		//for ALC5631V
	case ALC5633_VENDOR_ID1:
	//case ALC5633_VENDOR_ID2:	//for ALC5631V
		return 1;
	default:
		return 0;
	}
}

static int alc5633_readable_register(struct snd_soc_codec *codec,
				    unsigned int reg)
{
	switch (reg) {
	case ALC5633_RESET:
	case ALC5633_SPK_OUT_VOL:
	case ALC5633_HP_OUT_VOL:
	case ALC5633_MONO_AXO_1_2_VOL:
	case ALC5633_AUX_IN_VOL:
	case ALC5633_STEREO_DAC_VOL_1:
	case ALC5633_MIC_CTRL_1:
	case ALC5633_STEREO_DAC_VOL_2:
	case ALC5633_ADC_CTRL_1:
	case ALC5633_ADC_REC_MIXER:
	case ALC5633_ADC_CTRL_2:
	//case ALC5633_VDAC_DIG_VOL:   //for ALC5631V
	case ALC5633_OUTMIXER_L_CTRL:
	case ALC5633_OUTMIXER_R_CTRL:
	case ALC5633_AXO1MIXER_CTRL:
	case ALC5633_AXO2MIXER_CTRL:
	case ALC5633_MIC_CTRL_2:
	case ALC5633_DIG_MIC_CTRL:
	case ALC5633_MONO_INPUT_VOL:
	case ALC5633_SPK_MIXER_CTRL:
	case ALC5633_SPK_MONO_OUT_CTRL:
	case ALC5633_SPK_MONO_HP_OUT_CTRL:
	case ALC5633_SDP_CTRL:
	//case ALC5633_MONO_SDP_CTRL:	//for ALC5631V
	case ALC5633_STEREO_AD_DA_CLK_CTRL:
	case ALC5633_PWR_MANAG_ADD1:
	case ALC5633_PWR_MANAG_ADD2:
	case ALC5633_PWR_MANAG_ADD3:
	case ALC5633_PWR_MANAG_ADD4:
	case ALC5633_GEN_PUR_CTRL_REG:
	case ALC5633_GLOBAL_CLK_CTRL:
	case ALC5633_PLL_CTRL:
	case ALC5633_INT_ST_IRQ_CTRL_1:
	case ALC5633_INT_ST_IRQ_CTRL_2:
	case ALC5633_GPIO_CTRL:
	case ALC5633_MISC_CTRL:
	case ALC5633_DEPOP_FUN_CTRL_1:
	case ALC5633_DEPOP_FUN_CTRL_2:
	case ALC5633_JACK_DET_CTRL:
	case ALC5633_SOFT_VOL_CTRL:
	case ALC5633_ALC_CTRL_1:
	case ALC5633_ALC_CTRL_2:
	case ALC5633_ALC_CTRL_3:
	case ALC5633_PSEUDO_SPATL_CTRL:
	case ALC5633_INDEX_ADD:
	case ALC5633_INDEX_DATA:
	case ALC5633_EQ_CTRL:
	//case ALC5633_VENDOR_ID:		//for ALC5631V
	case ALC5633_VENDOR_ID1:
	//case ALC5633_VENDOR_ID2:	//for ALC5631V
		return 1;
	default:
		return 0;
	}
}

/**
 * alc5633_headset_detect - Detect headset.
 * @codec: SoC audio codec device.
 * @jack_insert: Jack insert or not.
 *
 * Detect whether is headset or not when jack inserted.
 *
 * Returns detect status.
 */
int alc5633_headset_detect(struct snd_soc_codec *codec, int jack_insert)
{
	int jack_type;

	if(jack_insert) {
		snd_soc_update_bits(codec, ALC5633_PWR_MANAG_ADD2,
			ALC5633_PWR_MICBIAS1_VOL, ALC5633_PWR_MICBIAS1_VOL);
		snd_soc_update_bits(codec, ALC5633_MIC_CTRL_2,
			ALC5633_MICBIAS1_S_C_DET_MASK |
			ALC5633_MICBIAS1_SHORT_CURR_DET_MASK,
			ALC5633_MICBIAS1_S_C_DET_ENA |
			ALC5633_MICBIAS1_SHORT_CURR_DET_600UA);
		msleep(50);
		if (alc5633_index_read(codec, 0x4a) & 0x2)
			jack_type = ALC5633_HEADPHO_DET;
		else
			jack_type = ALC5633_HEADSET_DET;
	} else {
		snd_soc_update_bits(codec, ALC5633_MIC_CTRL_2,
			ALC5633_MICBIAS1_S_C_DET_MASK,
			ALC5633_MICBIAS1_S_C_DET_DIS);
		jack_type = ALC5633_NO_JACK;
	}

	return jack_type;
}
EXPORT_SYMBOL(alc5633_headset_detect);

static const DECLARE_TLV_DB_SCALE(out_vol_tlv, -4650, 150, 0);
static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -95625, 375, 0);
static const DECLARE_TLV_DB_SCALE(in_vol_tlv, -3450, 150, 0);
/* {0, +20, +24, +30, +35, +40, +44, +50, +52} dB */
static unsigned int mic_bst_tlv[] = {
	TLV_DB_RANGE_HEAD(7),
	0, 0, TLV_DB_SCALE_ITEM(0, 0, 0),
	1, 1, TLV_DB_SCALE_ITEM(2000, 0, 0),
	2, 2, TLV_DB_SCALE_ITEM(2400, 0, 0),
	3, 5, TLV_DB_SCALE_ITEM(3000, 500, 0),
	6, 6, TLV_DB_SCALE_ITEM(4400, 0, 0),
	7, 7, TLV_DB_SCALE_ITEM(5000, 0, 0),
	8, 8, TLV_DB_SCALE_ITEM(5200, 0, 0),
};

static int alc5633_dmic_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);

	ucontrol->value.integer.value[0] = alc5633->dmic_used_flag;

	return 0;
}

static int alc5633_dmic_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);

	alc5633->dmic_used_flag = ucontrol->value.integer.value[0];
	return 0;
}

/* MIC Input Type */
static const char *alc5633_input_mode[] = {
	"Single ended", "Differential"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_mic1_mode_enum, ALC5633_MIC_CTRL_1,
	ALC5633_MIC1_DIFF_INPUT_SHIFT, alc5633_input_mode);

static const SOC_ENUM_SINGLE_DECL(
	alc5633_mic2_mode_enum, ALC5633_MIC_CTRL_1,
	ALC5633_MIC2_DIFF_INPUT_SHIFT, alc5633_input_mode);

/* MONO Input Type */
static const SOC_ENUM_SINGLE_DECL(
	alc5633_monoin_mode_enum, ALC5633_MONO_INPUT_VOL,
	ALC5633_MONO_DIFF_INPUT_SHIFT, alc5633_input_mode);

/* SPK Ratio Gain Control */
static const char *alc5633_spk_ratio[] = {"1.00x", "1.09x", "1.27x", "1.44x",
			"1.56x", "1.68x", "1.99x", "2.34x"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_spk_ratio_enum, ALC5633_GEN_PUR_CTRL_REG,
	ALC5633_SPK_AMP_RATIO_CTRL_SHIFT, alc5633_spk_ratio);

/* ADC Data Select Control */
static const char *alc5633_adc_data_sel[] = {"Stereo", "Left ADC", "Right ADC"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_adc_data_enum, ALC5633_INT_ST_IRQ_CTRL_2,
	ALC5633_ADC_DATA_SEL_SHIFT, alc5633_adc_data_sel);

/* ADCR Function Select Control */
static const char *alc5633_adcr_fun_sel[] = {"Stereo ADC", "Voice ADC"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_adcr_fun_enum, ALC5633_GLOBAL_CLK_CTRL,
	ALC5633_ADCR_FUN_SFT, alc5633_adcr_fun_sel);

/* VDAC Clock Select Control */
static const char *alc5633_vdac_clk_sel[] = {"SYSCLK1", "SYSCLK2"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_vdac_clk_enum, ALC5633_GLOBAL_CLK_CTRL,
	ALC5633_VDAC_CLK_SOUR_SFT, alc5633_vdac_clk_sel);

static const struct snd_kcontrol_new alc5633_snd_controls[] = {
	/* MIC */
	SOC_ENUM("MIC1 Mode Control",  alc5633_mic1_mode_enum),
	SOC_SINGLE_TLV("MIC1 Boost", ALC5633_MIC_CTRL_2,
		ALC5633_MIC1_BOOST_SHIFT, 8, 0, mic_bst_tlv),
	SOC_ENUM("MIC2 Mode Control", alc5633_mic2_mode_enum),
	SOC_SINGLE_TLV("MIC2 Boost", ALC5633_MIC_CTRL_2,
		ALC5633_MIC2_BOOST_SHIFT, 8, 0, mic_bst_tlv),
	/* MONO IN */
	SOC_ENUM("MONOIN Mode Control", alc5633_monoin_mode_enum),
	SOC_DOUBLE_TLV("MONOIN_RX Capture Volume", ALC5633_MONO_INPUT_VOL,
			ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT,
			ALC5633_VOL_MASK, 1, in_vol_tlv),
	/* AXI */
	SOC_DOUBLE_TLV("AXI Capture Volume", ALC5633_AUX_IN_VOL,
			ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT,
			ALC5633_VOL_MASK, 1, in_vol_tlv),
	/* ADC */
	SOC_DOUBLE_TLV("PCM Record Volume", ALC5633_ADC_CTRL_2,
			ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT,
			ALC5633_DAC_VOL_MASK, 1, dac_vol_tlv),
	SOC_DOUBLE("PCM Record Switch", ALC5633_ADC_CTRL_1,
			ALC5633_L_MUTE_SHIFT, ALC5633_R_MUTE_SHIFT, 1, 1),
	/* DAC */
	SOC_DOUBLE_TLV("PCM Playback Volume", ALC5633_STEREO_DAC_VOL_2,
			ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT,
			ALC5633_DAC_VOL_MASK, 1, dac_vol_tlv),
	SOC_DOUBLE("PCM Playback Switch", ALC5633_STEREO_DAC_VOL_1,
			ALC5633_L_MUTE_SHIFT, ALC5633_R_MUTE_SHIFT, 1, 1),
	/* AXO */
	SOC_SINGLE("AXO1 Playback Switch", ALC5633_MONO_AXO_1_2_VOL,
				ALC5633_L_MUTE_SHIFT, 1, 1),
	SOC_SINGLE("AXO2 Playback Switch", ALC5633_MONO_AXO_1_2_VOL,
				ALC5633_R_VOL_SHIFT, 1, 1),
	/* OUTVOL */
	SOC_DOUBLE("OUTVOL Channel Switch", ALC5633_SPK_OUT_VOL,
		ALC5633_L_EN_SHIFT, ALC5633_R_EN_SHIFT, 1, 0),

	/* SPK */
	SOC_DOUBLE("Speaker Playback Switch", ALC5633_SPK_OUT_VOL,
		ALC5633_L_MUTE_SHIFT, ALC5633_R_MUTE_SHIFT, 1, 1),
	SOC_DOUBLE_TLV("Speaker Playback Volume", ALC5633_SPK_OUT_VOL,
		ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT, 39, 1, out_vol_tlv),
#if 0
	/* MONO OUT */
	SOC_SINGLE("MONO Playback Switch", ALC5633_MONO_AXO_1_2_VOL,
				ALC5633_MUTE_MONO_SHIFT, 1, 1),
	SOC_DOUBLE_TLV("MONO Playback Volume", ALC5633_MONO_AXO_1_2_VOL,
		ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT,
		ALC5633_VOL_MASK, 1, out_vol_tlv),
#endif
	/* HP */
	SOC_DOUBLE("HP Playback Switch", ALC5633_HP_OUT_VOL,
		ALC5633_L_MUTE_SHIFT, ALC5633_R_MUTE_SHIFT, 1, 1),
	SOC_DOUBLE_TLV("HP Playback Volume", ALC5633_HP_OUT_VOL,
		ALC5633_L_VOL_SHIFT, ALC5633_R_VOL_SHIFT,
		ALC5633_VOL_MASK, 1, out_vol_tlv),
	/* DMIC */
	SOC_SINGLE_EXT("DMIC Switch", 0, 0, 1, 0,
		alc5633_dmic_get, alc5633_dmic_put),
	SOC_DOUBLE("DMIC Capture Switch", ALC5633_DIG_MIC_CTRL,
		ALC5633_DMIC_L_CH_MUTE_SHIFT,
		ALC5633_DMIC_R_CH_MUTE_SHIFT, 1, 1),
	/* SPK Ratio Gain Control */
	SOC_ENUM("SPK Ratio Control", alc5633_spk_ratio_enum),
	/* ADC Data Select Control */
	SOC_ENUM("ADC Data Select", alc5633_adc_data_enum),
	/* ADCR Function Select Control */
	SOC_ENUM("ADCR Function Select", alc5633_adcr_fun_enum),
	/* VDAC Function Select Control */
	SOC_ENUM("Voice DAC Clock Select", alc5633_vdac_clk_enum),
};

static int check_sysclk1_source(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);
	return (reg & ALC5633_SYSCLK_SOUR_SEL_PLL);
}

static int check_adcr_pll1(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);

	if (reg & ALC5633_ADCR_FUN_VADC) {
		if (reg & ALC5633_VDAC_CLK_SOUR_SCLK2)
			return 0;
		else
			return (reg & ALC5633_SYSCLK_SOUR_SEL_PLL);
	} else
		return (reg & ALC5633_SYSCLK_SOUR_SEL_PLL);
}

static int check_adcr_stereo(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);
	return (ALC5633_ADCR_FUN_ADC == (reg & ALC5633_ADCR_FUN_MASK));
}

static int check_adcr_voice(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);
	return (ALC5633_ADCR_FUN_VADC == (reg & ALC5633_ADCR_FUN_MASK));
}

static int check_adcr_pll2(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);

	if (reg & ALC5633_ADCR_FUN_VADC) {
		if (reg & ALC5633_VDAC_CLK_SOUR_SCLK2)
			return (reg & ALC5633_SYSCLK2_SOUR_SEL_PLL2);
		else
			return 0;
	} else
		return 0;
}

static int check_vdac_pll1(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);

	if (reg & ALC5633_VDAC_CLK_SOUR_SCLK2)
		return 0;
	else
		return (reg & ALC5633_SYSCLK_SOUR_SEL_PLL);
}

static int check_vdac_pll2(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg = snd_soc_read(source->codec, ALC5633_GLOBAL_CLK_CTRL);

	if (reg & ALC5633_VDAC_CLK_SOUR_SCLK2)
		return (reg & ALC5633_SYSCLK2_SOUR_SEL_PLL2);
	else
		return 0;
}

static int check_dmic_used(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(source->codec);
	return alc5633->dmic_used_flag;
}
#if (0)
static int check_dacl_to_outmixl(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, ALC5633_OUTMIXER_L_CTRL);
	return !(reg & ALC5633_M_DAC_L_TO_OUTMIXER_L);
}

static int check_dacr_to_outmixr(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, ALC5633_OUTMIXER_R_CTRL);
	return !(reg & ALC5633_M_DAC_R_TO_OUTMIXER_R);
}

static int check_dacl_to_spkmixl(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, ALC5633_SPK_MIXER_CTRL);
	return !(reg & ALC5633_M_DAC_L_TO_SPKMIXER_L);
}

static int check_dacr_to_spkmixr(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, ALC5633_SPK_MIXER_CTRL);
	return !(reg & ALC5633_M_DAC_R_TO_SPKMIXER_R);
}

static int check_vdac_to_outmix(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg, ret = 1;

	reg = snd_soc_read(source->codec, ALC5633_OUTMIXER_L_CTRL);
	if (reg & ALC5633_M_VDAC_TO_OUTMIXER_L) {
		reg = snd_soc_read(source->codec, ALC5633_OUTMIXER_R_CTRL);
		if (reg & ALC5633_M_VDAC_TO_OUTMIXER_R)
			ret = 0;
	}
	return ret;
}
#endif

static int check_adcl_select(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg, ret = 0;

	reg = snd_soc_read(source->codec, ALC5633_ADC_REC_MIXER);
	if (reg & ALC5633_M_MIC2_TO_RECMIXER_R)
		if (!(reg & ALC5633_M_MIC1_TO_RECMIXER_L))
			ret = 1;
	return ret;
}

static int check_adcr_select(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg, ret = 0;

	reg = snd_soc_read(source->codec, ALC5633_ADC_REC_MIXER);
	if (reg & ALC5633_M_MIC1_TO_RECMIXER_L)
		if (!(reg & ALC5633_M_MIC2_TO_RECMIXER_R))
			ret = 1;
	return ret;
}

/**
 * onebit_depop_power_stage - auto depop in power stage.
 * @enable: power on/off
 *
 * When power on/off headphone, the depop sequence is done by hardware.
 */
static void onebit_depop_power_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* enable one-bit depop function */
	snd_soc_update_bits(codec, ALC5633_DEPOP_FUN_CTRL_2,
				ALC5633_EN_ONE_BIT_DEPOP, 0);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, ALC5633_SOFT_VOL_CTRL);
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, ALC5633_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		/* config one-bit depop parameter */
		alc5633_index_write(codec, ALC5633_TEST_MODE_CTRL, 0x84c0);
		alc5633_index_write(codec, ALC5633_SPK_INTL_CTRL, 0x309f);
		alc5633_index_write(codec, ALC5633_CP_INTL_REG2, 0x6530);
		/* power on capless block */
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_2,
				ALC5633_EN_CAP_FREE_DEPOP);
	} else {
		/* power off capless block */
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_2, 0);
		msleep(100);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc);
}

/**
 * onebit_depop_mute_stage - auto depop in mute stage.
 * @enable: mute/unmute
 *
 * When mute/unmute headphone, the depop sequence is done by hardware.
 */
static void onebit_depop_mute_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* enable one-bit depop function */
	snd_soc_update_bits(codec, ALC5633_DEPOP_FUN_CTRL_2,
				ALC5633_EN_ONE_BIT_DEPOP, 0);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, ALC5633_SOFT_VOL_CTRL);
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, ALC5633_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		schedule_timeout_uninterruptible(msecs_to_jiffies(10));
		/* config one-bit depop parameter */
		alc5633_index_write(codec, ALC5633_SPK_INTL_CTRL, 0x307f);
		snd_soc_update_bits(codec, ALC5633_HP_OUT_VOL,
				ALC5633_L_MUTE | ALC5633_R_MUTE, 0);
		msleep(300);
	} else {
		snd_soc_update_bits(codec, ALC5633_HP_OUT_VOL,
			ALC5633_L_MUTE | ALC5633_R_MUTE,
			ALC5633_L_MUTE | ALC5633_R_MUTE);
		msleep(100);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc);
}

/**
 * onebit_depop_power_stage - step by step depop sequence in power stage.
 * @enable: power on/off
 *
 * When power on/off headphone, the depop sequence is done in step by step.
 */
static void depop_seq_power_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* depop control by register */
	snd_soc_update_bits(codec, ALC5633_DEPOP_FUN_CTRL_2,
		ALC5633_EN_ONE_BIT_DEPOP, ALC5633_EN_ONE_BIT_DEPOP);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, ALC5633_SOFT_VOL_CTRL);
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, ALC5633_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		/* config depop sequence parameter */
		alc5633_index_write(codec, ALC5633_SPK_INTL_CTRL, 0x303e);

		/* power on headphone and charge pump */
		snd_soc_update_bits(codec, ALC5633_PWR_MANAG_ADD3,
			ALC5633_PWR_CHARGE_PUMP | ALC5633_PWR_HP_L_AMP |
			ALC5633_PWR_HP_R_AMP,
			ALC5633_PWR_CHARGE_PUMP | ALC5633_PWR_HP_L_AMP |
			ALC5633_PWR_HP_R_AMP);

		/* power on soft generator and depop mode2 */
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN | ALC5633_EN_DEPOP2_FOR_HP);
		msleep(100);

		/* stop depop mode */
		snd_soc_update_bits(codec, ALC5633_PWR_MANAG_ADD3,
			ALC5633_PWR_HP_DEPOP_DIS, ALC5633_PWR_HP_DEPOP_DIS);
	} else {
		/* config depop sequence parameter */
		alc5633_index_write(codec, ALC5633_SPK_INTL_CTRL, 0x303F);
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN | ALC5633_EN_MUTE_UNMUTE_DEPOP |
			ALC5633_PD_HPAMP_L_ST_UP | ALC5633_PD_HPAMP_R_ST_UP);
		msleep(75);
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN | ALC5633_PD_HPAMP_L_ST_UP |
			ALC5633_PD_HPAMP_R_ST_UP);

		/* start depop mode */
		snd_soc_update_bits(codec, ALC5633_PWR_MANAG_ADD3,
				ALC5633_PWR_HP_DEPOP_DIS, 0);

		/* config depop sequence parameter */
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN | ALC5633_EN_DEPOP2_FOR_HP |
			ALC5633_PD_HPAMP_L_ST_UP | ALC5633_PD_HPAMP_R_ST_UP);
		msleep(80);
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN);

		/* power down headphone and charge pump */
		snd_soc_update_bits(codec, ALC5633_PWR_MANAG_ADD3,
			ALC5633_PWR_CHARGE_PUMP | ALC5633_PWR_HP_L_AMP |
			ALC5633_PWR_HP_R_AMP, 0);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc);
}

/**
 * depop_seq_mute_stage - step by step depop sequence in mute stage.
 * @enable: mute/unmute
 *
 * When mute/unmute headphone, the depop sequence is done in step by step.
 */
static void depop_seq_mute_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* depop control by register */
	snd_soc_update_bits(codec, ALC5633_DEPOP_FUN_CTRL_2,
		ALC5633_EN_ONE_BIT_DEPOP, ALC5633_EN_ONE_BIT_DEPOP);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, ALC5633_SOFT_VOL_CTRL);
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, ALC5633_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		schedule_timeout_uninterruptible(msecs_to_jiffies(10));

		/* config depop sequence parameter */
		alc5633_index_write(codec, ALC5633_SPK_INTL_CTRL, 0x302f);
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN | ALC5633_EN_MUTE_UNMUTE_DEPOP |
			ALC5633_EN_HP_R_M_UN_MUTE_DEPOP |
			ALC5633_EN_HP_L_M_UN_MUTE_DEPOP);

		snd_soc_update_bits(codec, ALC5633_HP_OUT_VOL,
				ALC5633_L_MUTE | ALC5633_R_MUTE, 0);
		msleep(160);
	} else {
		/* config depop sequence parameter */
		alc5633_index_write(codec, ALC5633_SPK_INTL_CTRL, 0x302f);
		snd_soc_write(codec, ALC5633_DEPOP_FUN_CTRL_1,
			ALC5633_POW_ON_SOFT_GEN | ALC5633_EN_MUTE_UNMUTE_DEPOP |
			ALC5633_EN_HP_R_M_UN_MUTE_DEPOP |
			ALC5633_EN_HP_L_M_UN_MUTE_DEPOP);

		snd_soc_update_bits(codec, ALC5633_HP_OUT_VOL,
			ALC5633_L_MUTE | ALC5633_R_MUTE,
			ALC5633_L_MUTE | ALC5633_R_MUTE);
		msleep(150);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, ALC5633_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, ALC5633_INT_ST_IRQ_CTRL_2, hp_zc);
}

static int set_dmic_params(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);

	switch (alc5633->rx_rate) {
	case 44100:
	case 48000:
		snd_soc_update_bits(codec, ALC5633_DIG_MIC_CTRL,
			ALC5633_DMIC_CLK_CTRL_MASK,
			ALC5633_DMIC_CLK_CTRL_TO_32FS);
		break;

	case 32000:
	case 22050:
		snd_soc_update_bits(codec, ALC5633_DIG_MIC_CTRL,
			ALC5633_DMIC_CLK_CTRL_MASK,
			ALC5633_DMIC_CLK_CTRL_TO_64FS);
		break;

	case 16000:
	case 11025:
	case 8000:
		snd_soc_update_bits(codec, ALC5633_DIG_MIC_CTRL,
			ALC5633_DMIC_CLK_CTRL_MASK,
			ALC5633_DMIC_CLK_CTRL_TO_128FS);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static const struct snd_kcontrol_new alc5633_recmixl_mixer_controls[] = {
	SOC_DAPM_SINGLE("OUTMIXL Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_OUTMIXL_RECMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_BST1 Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_MIC1_RECMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXILVOL Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_AXIL_RECMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MONOIN_RX Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_MONO_IN_RECMIXL_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_recmixr_mixer_controls[] = {
	SOC_DAPM_SINGLE("MONOIN_RX Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_MONO_IN_RECMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXIRVOL Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_AXIR_RECMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_MIC2_RECMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTMIXR Capture Switch", ALC5633_ADC_REC_MIXER,
			ALC5633_M_OUTMIXR_RECMIXR_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_spkmixl_mixer_controls[] = {
	SOC_DAPM_SINGLE("RECMIXL Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_RECMIXL_SPKMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_P Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_MIC1P_SPKMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACL Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_DACL_SPKMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTMIXL Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_OUTMIXL_SPKMIXL_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_spkmixr_mixer_controls[] = {
	SOC_DAPM_SINGLE("OUTMIXR Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_OUTMIXR_SPKMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACR Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_DACR_SPKMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_P Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_MIC2P_SPKMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXR Playback Switch", ALC5633_SPK_MIXER_CTRL,
			ALC5633_M_RECMIXR_SPKMIXR_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_outmixl_mixer_controls[] = {
	SOC_DAPM_SINGLE("RECMIXL Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_RECMIXL_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXR Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_RECMIXR_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACL Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_DACL_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_MIC1_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_MIC2_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MONOIN_RXP Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_MONO_INP_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXILVOL Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_AXIL_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXIRVOL Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_AXIR_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("VDAC Playback Switch", ALC5633_OUTMIXER_L_CTRL,
				ALC5633_M_VDAC_OUTMIXL_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_outmixr_mixer_controls[] = {
	SOC_DAPM_SINGLE("VDAC Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_VDAC_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXIRVOL Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_AXIR_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXILVOL Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_AXIL_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MONOIN_RXN Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_MONO_INN_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_MIC2_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_MIC1_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACR Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_DACR_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXR Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_RECMIXR_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXL Playback Switch", ALC5633_OUTMIXER_R_CTRL,
				ALC5633_M_RECMIXL_OUTMIXR_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_AXO1MIX_mixer_controls[] = {
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", ALC5633_AXO1MIXER_CTRL,
				ALC5633_M_MIC1_AXO1MIX_BIT , 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", ALC5633_AXO1MIXER_CTRL,
				ALC5633_M_MIC2_AXO1MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLL Playback Switch", ALC5633_AXO1MIXER_CTRL,
				ALC5633_M_OUTMIXL_AXO1MIX_BIT , 1 , 1),
	SOC_DAPM_SINGLE("OUTVOLR Playback Switch", ALC5633_AXO1MIXER_CTRL,
				ALC5633_M_OUTMIXR_AXO1MIX_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_AXO2MIX_mixer_controls[] = {
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", ALC5633_AXO2MIXER_CTRL,
				ALC5633_M_MIC1_AXO2MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", ALC5633_AXO2MIXER_CTRL,
				ALC5633_M_MIC2_AXO2MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLL Playback Switch", ALC5633_AXO2MIXER_CTRL,
				ALC5633_M_OUTMIXL_AXO2MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLR Playback Switch", ALC5633_AXO2MIXER_CTRL,
				ALC5633_M_OUTMIXR_AXO2MIX_BIT, 1 , 1),
};

static const struct snd_kcontrol_new alc5633_spolmix_mixer_controls[] = {
	SOC_DAPM_SINGLE("SPKVOLL Playback Switch", ALC5633_SPK_MONO_OUT_CTRL,
				ALC5633_M_SPKVOLL_SPOLMIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOLR Playback Switch", ALC5633_SPK_MONO_OUT_CTRL,
				ALC5633_M_SPKVOLR_SPOLMIX_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_spormix_mixer_controls[] = {
	SOC_DAPM_SINGLE("SPKVOLL Playback Switch", ALC5633_SPK_MONO_OUT_CTRL,
				ALC5633_M_SPKVOLL_SPORMIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOLR Playback Switch", ALC5633_SPK_MONO_OUT_CTRL,
				ALC5633_M_SPKVOLR_SPORMIX_BIT, 1, 1),
};

static const struct snd_kcontrol_new alc5633_monomix_mixer_controls[] = {
	SOC_DAPM_SINGLE("OUTVOLL Playback Switch", ALC5633_SPK_MONO_OUT_CTRL,
				ALC5633_M_OUTVOLL_MONOMIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLR Playback Switch", ALC5633_SPK_MONO_OUT_CTRL,
				ALC5633_M_OUTVOLR_MONOMIX_BIT, 1, 1),
};

/* Left SPK Volume Input */
static const char *alc5633_spkvoll_sel[] = {"Vmid", "SPKMIXL"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_spkvoll_enum, ALC5633_SPK_OUT_VOL,
	ALC5633_L_EN_SHIFT, alc5633_spkvoll_sel);

static const struct snd_kcontrol_new alc5633_spkvoll_mux_control =
	SOC_DAPM_ENUM("Left SPKVOL SRC", alc5633_spkvoll_enum);

/* Left HP Volume Input */
static const char *alc5633_hpvoll_sel[] = {"Vmid", "OUTMIXL"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_hpvoll_enum, ALC5633_HP_OUT_VOL,
	ALC5633_L_EN_SHIFT, alc5633_hpvoll_sel);

static const struct snd_kcontrol_new alc5633_hpvoll_mux_control =
	SOC_DAPM_ENUM("Left HPVOL SRC", alc5633_hpvoll_enum);

/* Left Out Volume Input */
static const char *alc5633_outvoll_sel[] = {"Vmid", "OUTMIXL"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_outvoll_enum, ALC5633_MONO_AXO_1_2_VOL,
	ALC5633_L_EN_SHIFT, alc5633_outvoll_sel);

static const struct snd_kcontrol_new alc5633_outvoll_mux_control =
	SOC_DAPM_ENUM("Left OUTVOL SRC", alc5633_outvoll_enum);

/* Right Out Volume Input */
static const char *alc5633_outvolr_sel[] = {"Vmid", "OUTMIXR"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_outvolr_enum, ALC5633_MONO_AXO_1_2_VOL,
	ALC5633_R_EN_SHIFT, alc5633_outvolr_sel);

static const struct snd_kcontrol_new alc5633_outvolr_mux_control =
	SOC_DAPM_ENUM("Right OUTVOL SRC", alc5633_outvolr_enum);

/* Right HP Volume Input */
static const char *alc5633_hpvolr_sel[] = {"Vmid", "OUTMIXR"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_hpvolr_enum, ALC5633_HP_OUT_VOL,
	ALC5633_R_EN_SHIFT, alc5633_hpvolr_sel);

static const struct snd_kcontrol_new alc5633_hpvolr_mux_control =
	SOC_DAPM_ENUM("Right HPVOL SRC", alc5633_hpvolr_enum);

/* Right SPK Volume Input */
static const char *alc5633_spkvolr_sel[] = {"Vmid", "SPKMIXR"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_spkvolr_enum, ALC5633_SPK_OUT_VOL,
	ALC5633_R_EN_SHIFT, alc5633_spkvolr_sel);

static const struct snd_kcontrol_new alc5633_spkvolr_mux_control =
	SOC_DAPM_ENUM("Right SPKVOL SRC", alc5633_spkvolr_enum);

/* SPO Left Channel Input */
static const char *alc5633_spol_src_sel[] = {
	"SPOLMIX", "MONOIN_RX", "VDAC", "DACL"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_spol_src_enum, ALC5633_SPK_MONO_HP_OUT_CTRL,
	ALC5633_SPK_L_MUX_SEL_SHIFT, alc5633_spol_src_sel);

static const struct snd_kcontrol_new alc5633_spol_mux_control =
	SOC_DAPM_ENUM("SPOL SRC", alc5633_spol_src_enum);

/* SPO Right Channel Input */
static const char *alc5633_spor_src_sel[] = {
	"SPORMIX", "MONOIN_RX", "VDAC", "DACR"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_spor_src_enum, ALC5633_SPK_MONO_HP_OUT_CTRL,
	ALC5633_SPK_R_MUX_SEL_SHIFT, alc5633_spor_src_sel);

static const struct snd_kcontrol_new alc5633_spor_mux_control =
	SOC_DAPM_ENUM("SPOR SRC", alc5633_spor_src_enum);

/* MONO Input */
static const char *alc5633_mono_src_sel[] = {"MONOMIX", "MONOIN_RX", "VDAC"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_mono_src_enum, ALC5633_SPK_MONO_HP_OUT_CTRL,
	ALC5633_MONO_MUX_SEL_SHIFT, alc5633_mono_src_sel);

static const struct snd_kcontrol_new alc5633_mono_mux_control =
	SOC_DAPM_ENUM("MONO SRC", alc5633_mono_src_enum);

/* Left HPO Input */
static const char *alc5633_hpl_src_sel[] = {"Left HPVOL", "Left DAC"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_hpl_src_enum, ALC5633_SPK_MONO_HP_OUT_CTRL,
	ALC5633_HP_L_MUX_SEL_SHIFT, alc5633_hpl_src_sel);

static const struct snd_kcontrol_new alc5633_hpl_mux_control =
	SOC_DAPM_ENUM("HPL SRC", alc5633_hpl_src_enum);

/* Right HPO Input */
static const char *alc5633_hpr_src_sel[] = {"Right HPVOL", "Right DAC"};

static const SOC_ENUM_SINGLE_DECL(
	alc5633_hpr_src_enum, ALC5633_SPK_MONO_HP_OUT_CTRL,
	ALC5633_HP_R_MUX_SEL_SHIFT, alc5633_hpr_src_sel);

static const struct snd_kcontrol_new alc5633_hpr_mux_control =
	SOC_DAPM_ENUM("HPR SRC", alc5633_hpr_src_enum);


#endif



static int spk_event(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
        struct snd_soc_codec *codec = w->codec;
        unsigned int l, r;

        l = (alc5633_read(codec, ALC5633_PWR_MANAG_ADD4) & (0x01 << 15)) >> 15;
        r = (alc5633_read(codec, ALC5633_PWR_MANAG_ADD4) & (0x01 << 14)) >> 14;

        switch (event) {
        case SND_SOC_DAPM_PRE_PMD:
                if (l && r)
                {
                        alc5633_write_mask(codec, ALC5633_SPK_OUT_VOL, 0x8000, 0x8000);

                        alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD1, 0x0000, 0x2020);
			printk("spk_event 0\n");
                }

                break;
        case SND_SOC_DAPM_POST_PMU:
                if (l && r)
                {
                        alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD1, 0x2020, 0x2020);
                        alc5633_write_mask(codec, ALC5633_SPK_OUT_VOL, 0x0000, 0x8000);
                        alc5633_write(codec, ALC5633_DAC_DIG_VOL, 0x1010);
                        alc5633_write_index(codec, 0X45, 0X4100);
			printk("spk_event 1\n");
                }
                break;
        default:
                return -EINVAL;
        }

	return 0;
}

static int auxout_event(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
        struct snd_soc_codec *codec = w->codec;
        unsigned int l, r;
	static unsigned int aux_out_enable=0;

        l = (alc5633_read(codec, ALC5633_PWR_MANAG_ADD4) & (0x01 << 9)) >> 9;
        r = (alc5633_read(codec, ALC5633_PWR_MANAG_ADD4) & (0x01 << 8)) >> 8;

        switch (event) {
        case SND_SOC_DAPM_PRE_PMD:
                if ((l && r)&&(aux_out_enable))
                {
                        alc5633_write_mask(codec, ALC5633_AUXOUT_VOL, 0x8080, 0x8080);
			aux_out_enable=0;
			printk("auxout_event 0\n");
                }

                break;
        case SND_SOC_DAPM_POST_PMU:
                if ((l && r)&&(!aux_out_enable))
                {
                        alc5633_write_mask(codec, ALC5633_AUXOUT_VOL, 0x0000, 0x8080);
			aux_out_enable=1;
			printk("auxout_event 1\n");
                }
                break;
        default:
                return -EINVAL;
        }

        return 0;
}

//HP mute/unmute depop

static void hp_mute_unmute_depop(struct snd_soc_codec *codec,unsigned int Mute)
{
        if(Mute)
        {
                alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP
                                  ,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
                alc5633_write_mask(codec, ALC5633_HP_OUT_VOL, 0x8080, 0x8080);
                msleep(80);
                alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,0,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
        }
        else
        {
                alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP
                                  ,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
                alc5633_write_mask(codec, ALC5633_HP_OUT_VOL, 0x0000, 0x8080);
                msleep(80);
                alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,0,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
        }
}

//HP power on depop

static void hp_depop_mode2(struct snd_soc_codec *codec)
{
        alc5633_write_mask(codec,ALC5633_PWR_MANAG_ADD3,PWR_MAIN_BIAS|PWR_VREF,PWR_VREF|PWR_MAIN_BIAS);
        alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,PW_SOFT_GEN,PW_SOFT_GEN);
        alc5633_write_mask(codec,ALC5633_PWR_MANAG_ADD3,PWR_HP_AMP,PWR_HP_AMP);
        alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,EN_DEPOP_2,EN_DEPOP_2);
       // schedule_timeout_uninterruptible(msecs_to_jiffies(300));
        alc5633_write_mask(codec,ALC5633_PWR_MANAG_ADD3,PWR_HP_DIS_DEPOP|PWR_HP_AMP_DRI,PWR_HP_DIS_DEPOP|PWR_HP_AMP_DRI);
        alc5633_write_mask(codec,ALC5633_PWR_MANAG_ADD4,PWR_HP_L_VOL|PWR_HP_R_VOL,PWR_HP_L_VOL|PWR_HP_R_VOL);
        alc5633_write_mask(codec,ALC5633_DEPOP_CTRL_1,0,EN_DEPOP_2);
}

static int open_hp_end_widgets(struct snd_soc_codec *codec)
{
	hp_mute_unmute_depop(codec, 0);

	return 0;
}

static int close_hp_end_widgets(struct snd_soc_codec *codec)
{
	hp_mute_unmute_depop(codec, 1);
	
	alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3, 0x0000, 0x000F);
	return 0;
}

static int hp_event(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
        struct snd_soc_codec *codec = w->codec;
        unsigned int l, r;
	static unsigned int hp_out_enable=0;

        l = (alc5633_read(codec, ALC5633_PWR_MANAG_ADD4) & (0x01 << 11)) >> 11;
        r = (alc5633_read(codec, ALC5633_PWR_MANAG_ADD4) & (0x01 << 10)) >> 10;

        switch (event) {
        case SND_SOC_DAPM_PRE_PMD:
                if ((l && r)&&(hp_out_enable))
                {
			close_hp_end_widgets(codec);
			hp_out_enable = 0;
			printk("hp_event 0\n");
                }

                break;
        case SND_SOC_DAPM_POST_PMU:
                if ((l && r)&&(!hp_out_enable))
                {
			hp_depop_mode2(codec);
			open_hp_end_widgets(codec);
			hp_out_enable = 1;
                        alc5633_write(codec, ALC5633_DAC_DIG_VOL, 0x0000);
			printk("hp_event 1\n");
                }
                break;
        default:
                return -EINVAL;
        }

        return 0;
}

static int dac_event(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
	printk("dac_event\n");
        struct snd_soc_codec *codec = w->codec;
        static unsigned int dac_enable=0;

        switch (event) {

        case SND_SOC_DAPM_PRE_PMD:

                //printk("dac_event --SND_SOC_DAPM_PRE_PMD\n");
                if (dac_enable)
                {

#if ALC5633_EQ_FUNC_ENA

        #if (ALC5633_EQ_FUNC_SEL==ALC5633_EQ_FOR_DAC)

                alc5633_update_eqmode(codec,NORMAL);    //disable EQ

        #endif

#endif

#if ALC5633_ALC_FUNC_ENA

        #if (ALC5633_ALC_FUNC_SEL==ALC5633_ALC_FOR_DAC)

                alc5633_alc_enable(codec,0);            //disable ALC

        #endif

#endif

                        dac_enable=0;
                }
                break;

        case SND_SOC_DAPM_POST_PMU:

                //printk("dac_event --SND_SOC_DAPM_POST_PMU\n");
                if(!dac_enable)
                {

#if ALC5633_EQ_FUNC_ENA

        #if (ALC5633_EQ_FUNC_SEL==ALC5633_EQ_FOR_DAC)

                alc5633_update_eqmode(codec,EQ_DEFAULT_PRESET); //enable EQ preset

        #endif

#endif

#if ALC5633_ALC_FUNC_ENA

        #if (ALC5633_ALC_FUNC_SEL==ALC5633_ALC_FOR_DAC)

                alc5633_alc_enable(codec,1);            //enable ALC

        #endif

#endif
                        dac_enable=1;
                }
                break;
        default:
                return 0;
        }

	return 0;
}

static int adc_event(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
	printk("adc_event- not yet\n");
	return 0;
}


static const char *alc5633_spo_source_sel[] = {"VMID", "HPMIX", "SPKMIX", "AUXMIX"};
static const char *alc5633_input_mode_source_sel[] = {"Single-end", "Differential"};
static const char *alc5633_auxout_mode_source_sel[] = {"Differential", "Stereo"};
static const char *alc5633_mic_boost[] = {"Bypass", "+20db", "+24db", "+30db",
                        "+35db", "+40db", "+44db", "+50db", "+52db"};
static const char *alc5633_spor_source_sel[] = {"RN", "RP", "LN", "VMID"};

static const struct soc_enum alc5633_enum[] = {
SOC_ENUM_SINGLE(ALC5633_SPKMIXER_CTRL, 10, 4, alc5633_spo_source_sel),   /*0*/
SOC_ENUM_SINGLE(ALC5633_MIC_CTRL_1, 15, 2,  alc5633_input_mode_source_sel),     /*1*/
SOC_ENUM_SINGLE(ALC5633_MIC_CTRL_1, 7, 2,  alc5633_input_mode_source_sel),      /*2*/
SOC_ENUM_SINGLE(ALC5633_SPK_OUT_VOL, 12, 2, alc5633_input_mode_source_sel),     /*3*/
SOC_ENUM_SINGLE(ALC5633_MIC_CTRL_2, 12, 8, alc5633_mic_boost),                  /*4*/
SOC_ENUM_SINGLE(ALC5633_MIC_CTRL_2, 8, 8, alc5633_mic_boost),                   /*5*/
SOC_ENUM_SINGLE(ALC5633_SPK_OUT_VOL, 13, 4, alc5633_spor_source_sel), /*6*/
SOC_ENUM_SINGLE(ALC5633_AUXOUT_VOL, 14, 2, alc5633_auxout_mode_source_sel), /*7*/
};


static const struct snd_kcontrol_new alc5633_snd_controls[] = {
SOC_ENUM("MIC1 Mode Control",  alc5633_enum[1]),
SOC_ENUM("MIC1 Boost", alc5633_enum[4]),

SOC_ENUM("MIC2 Mode Control", alc5633_enum[2]),
SOC_ENUM("MIC2 Boost", alc5633_enum[5]),
SOC_ENUM("Classab Mode Control", alc5633_enum[3]),
SOC_ENUM("SPKR Out Control", alc5633_enum[6]),
SOC_ENUM("AUXOUT Control", alc5633_enum[7]),
SOC_DOUBLE("Line1 Capture Volume", ALC5633_LINE_IN_1_VOL, 8, 0, 31, 1),
SOC_DOUBLE("Line2 Capture Volume", ALC5633_LINE_IN_2_VOL, 8, 0, 31, 1),

SOC_SINGLE("MIC1 Playback Volume", ALC5633_MIC_CTRL_1, 8, 31, 1),
SOC_SINGLE("MIC2 Playback Volume", ALC5633_MIC_CTRL_1, 0, 31, 1),

SOC_SINGLE("AXOL Playback Switch", ALC5633_AUXOUT_VOL, 15, 1, 1),
SOC_SINGLE("AXOR Playback Switch", ALC5633_AUXOUT_VOL, 7, 1, 1),
SOC_DOUBLE("AUX Playback Volume", ALC5633_AUXOUT_VOL, 8, 0, 31, 1),
SOC_SINGLE("SPK Playback Switch", ALC5633_SPK_OUT_VOL, 15, 1, 1),
SOC_DOUBLE("SPK Playback Volume", ALC5633_SPK_OUT_VOL, 5, 0, 31, 1),
SOC_SINGLE("HPL Playback Switch", ALC5633_HP_OUT_VOL, 15, 1, 1),
SOC_SINGLE("HPR Playback Switch", ALC5633_HP_OUT_VOL, 7, 1, 1),
SOC_DOUBLE("HP Playback Volume", ALC5633_HP_OUT_VOL, 8, 0, 31, 1),
};



static const struct snd_kcontrol_new alc5633_recmixl_mixer_controls[] = {
SOC_DAPM_SINGLE("HPMIXL Capture Switch", ALC5633_REC_MIXER_CTRL, 14, 1, 1),
SOC_DAPM_SINGLE("AUXMIXL Capture Switch", ALC5633_REC_MIXER_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("SPKMIX Capture Switch", ALC5633_REC_MIXER_CTRL, 12, 1, 1),
SOC_DAPM_SINGLE("LINE1L Capture Switch", ALC5633_REC_MIXER_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("LINE2L Capture Switch", ALC5633_REC_MIXER_CTRL, 10, 1, 1),
SOC_DAPM_SINGLE("MIC1 Capture Switch", ALC5633_REC_MIXER_CTRL, 9, 1, 1),
SOC_DAPM_SINGLE("MIC2 Capture Switch", ALC5633_REC_MIXER_CTRL, 8, 1, 1),
};

static const struct snd_kcontrol_new alc5633_recmixr_mixer_controls[] = {
SOC_DAPM_SINGLE("HPMIXR Capture Switch", ALC5633_REC_MIXER_CTRL, 6, 1, 1),
SOC_DAPM_SINGLE("AUXMIXR Capture Switch", ALC5633_REC_MIXER_CTRL, 5, 1, 1),
SOC_DAPM_SINGLE("SPKMIX Capture Switch", ALC5633_REC_MIXER_CTRL, 4, 1, 1),
SOC_DAPM_SINGLE("LINE1R Capture Switch", ALC5633_REC_MIXER_CTRL, 3, 1, 1),
SOC_DAPM_SINGLE("LINE2R Capture Switch", ALC5633_REC_MIXER_CTRL, 2, 1, 1),
SOC_DAPM_SINGLE("MIC1 Capture Switch", ALC5633_REC_MIXER_CTRL, 1, 1, 1),
SOC_DAPM_SINGLE("MIC2 Capture Switch", ALC5633_REC_MIXER_CTRL, 0, 1, 1),
};


static const struct snd_kcontrol_new alc5633_hp_mixl_mixer_controls[] = {
SOC_DAPM_SINGLE("RECMIXL Playback Switch", ALC5633_HPMIXER_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("MIC1 Playback Switch", ALC5633_HPMIXER_CTRL, 12, 1, 1),
SOC_DAPM_SINGLE("MIC2 Playback Switch", ALC5633_HPMIXER_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("LINE1 Playback Switch", ALC5633_HPMIXER_CTRL, 10, 1, 1),
SOC_DAPM_SINGLE("LINE2 Playback Switch", ALC5633_HPMIXER_CTRL, 9, 1, 1),
SOC_DAPM_SINGLE("DAC Playback Switch", ALC5633_HPMIXER_CTRL, 8, 1, 1),


};

static const struct snd_kcontrol_new alc5633_hp_mixr_mixer_controls[] = {
SOC_DAPM_SINGLE("RECMIXR Playback Switch", ALC5633_HPMIXER_CTRL, 5, 1, 1),
SOC_DAPM_SINGLE("MIC1 Playback Switch", ALC5633_HPMIXER_CTRL, 4, 1, 1),
SOC_DAPM_SINGLE("MIC2 Playback Switch", ALC5633_HPMIXER_CTRL, 3, 1, 1),
SOC_DAPM_SINGLE("LINE1 Playback Switch", ALC5633_HPMIXER_CTRL, 2, 1, 1),
SOC_DAPM_SINGLE("LINE2 Playback Switch", ALC5633_HPMIXER_CTRL, 1, 1, 1),
SOC_DAPM_SINGLE("DAC Playback Switch", ALC5633_HPMIXER_CTRL, 0, 1, 1),
};

static const struct snd_kcontrol_new alc5633_auxmixl_mixer_controls[] = {
SOC_DAPM_SINGLE("RECMIXL Playback Switch", ALC5633_AUXMIXER_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("MIC1 Playback Switch", ALC5633_AUXMIXER_CTRL, 12, 1, 1),
SOC_DAPM_SINGLE("MIC2 Playback Switch", ALC5633_AUXMIXER_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("LINE1 Playback Switch", ALC5633_AUXMIXER_CTRL, 10, 1, 1),
SOC_DAPM_SINGLE("LINE2 Playback Switch", ALC5633_AUXMIXER_CTRL, 9, 1, 1),
SOC_DAPM_SINGLE("DAC Playback Switch", ALC5633_AUXMIXER_CTRL, 8, 1, 1),

};

static const struct snd_kcontrol_new alc5633_auxmixr_mixer_controls[] = {
SOC_DAPM_SINGLE("RECMIXR Playback Switch", ALC5633_AUXMIXER_CTRL, 5, 1, 1),
SOC_DAPM_SINGLE("MIC1 Playback Switch", ALC5633_AUXMIXER_CTRL, 4, 1, 1),
SOC_DAPM_SINGLE("MIC2 Playback Switch", ALC5633_AUXMIXER_CTRL, 3, 1, 1),
SOC_DAPM_SINGLE("LINE1 Playback Switch", ALC5633_AUXMIXER_CTRL, 2, 1, 1),
SOC_DAPM_SINGLE("LINE2 Playback Switch", ALC5633_AUXMIXER_CTRL, 1, 1, 1),
SOC_DAPM_SINGLE("DAC Playback Switch", ALC5633_AUXMIXER_CTRL, 0, 1, 1),
};

static const struct snd_kcontrol_new alc5633_spkmixr_mixer_controls[]  = {
SOC_DAPM_SINGLE("MIC1 Playback Switch", ALC5633_SPKMIXER_CTRL, 7, 1, 1),
SOC_DAPM_SINGLE("MIC2 Playback Switch", ALC5633_SPKMIXER_CTRL, 6, 1, 1),
SOC_DAPM_SINGLE("LINE1L Playback Switch", ALC5633_SPKMIXER_CTRL, 5, 1, 1),
SOC_DAPM_SINGLE("LINE1R Playback Switch", ALC5633_SPKMIXER_CTRL, 4, 1, 1),
SOC_DAPM_SINGLE("LINE2L Playback Switch", ALC5633_SPKMIXER_CTRL, 3, 1, 1),
SOC_DAPM_SINGLE("LINE2R Playback Switch", ALC5633_SPKMIXER_CTRL, 2, 1, 1),
SOC_DAPM_SINGLE("DACL Playback Switch", ALC5633_SPKMIXER_CTRL, 1, 1, 1),
SOC_DAPM_SINGLE("DACR Playback Switch", ALC5633_SPKMIXER_CTRL, 0, 1, 1),
};

static const struct snd_kcontrol_new alc5633_spo_mux_control =
SOC_DAPM_ENUM("Route", alc5633_enum[0]);

static const struct snd_soc_dapm_widget alc5633_dapm_widgets[] = {

SND_SOC_DAPM_INPUT("MIC1"),
SND_SOC_DAPM_INPUT("MIC2"),
SND_SOC_DAPM_INPUT("LINE1L"),
SND_SOC_DAPM_INPUT("LINE2L"),
SND_SOC_DAPM_INPUT("LINE1R"),
SND_SOC_DAPM_INPUT("LINE2R"),


SND_SOC_DAPM_PGA("Mic1 Boost", ALC5633_PWR_MANAG_ADD2, 5, 0, NULL, 0),
SND_SOC_DAPM_PGA("Mic2 Boost", ALC5633_PWR_MANAG_ADD2, 4, 0, NULL, 0),

SND_SOC_DAPM_PGA("LINE1L Inp Vol", ALC5633_PWR_MANAG_ADD2, 9, 0, NULL, 0),
SND_SOC_DAPM_PGA("LINE1R Inp Vol", ALC5633_PWR_MANAG_ADD2, 8, 0, NULL, 0),
SND_SOC_DAPM_PGA("LINE2L Inp Vol", ALC5633_PWR_MANAG_ADD2, 7, 0, NULL, 0),
SND_SOC_DAPM_PGA("LINE2R Inp Vol", ALC5633_PWR_MANAG_ADD2, 6, 0, NULL, 0),



SND_SOC_DAPM_MIXER("RECMIXL Mixer", ALC5633_PWR_MANAG_ADD2, 11, 0,
        &alc5633_recmixl_mixer_controls[0], ARRAY_SIZE(alc5633_recmixl_mixer_controls)),
SND_SOC_DAPM_MIXER("RECMIXR Mixer", ALC5633_PWR_MANAG_ADD2, 10, 0,
        &alc5633_recmixr_mixer_controls[0], ARRAY_SIZE(alc5633_recmixr_mixer_controls)),


SND_SOC_DAPM_ADC_E("Left ADC","Left ADC HIFI Capture", ALC5633_PWR_MANAG_ADD1,12, 0,
                adc_event, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
SND_SOC_DAPM_ADC_E("Right ADC","Right ADC HIFI Capture", ALC5633_PWR_MANAG_ADD1,11, 0,
                adc_event, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),

SND_SOC_DAPM_DAC_E("Left DAC", "Left DAC HIFI Playback", ALC5633_PWR_MANAG_ADD1, 10, 0,
                dac_event, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
SND_SOC_DAPM_DAC_E("Right DAC", "Right DAC HIFI Playback", ALC5633_PWR_MANAG_ADD1, 9, 0,
                dac_event, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),

SND_SOC_DAPM_MIXER("HPMIXL Mixer", ALC5633_PWR_MANAG_ADD2, 15, 0,
        &alc5633_hp_mixl_mixer_controls[0], ARRAY_SIZE(alc5633_hp_mixl_mixer_controls)),
SND_SOC_DAPM_MIXER("HPMIXR Mixer", ALC5633_PWR_MANAG_ADD2, 14, 0,
        &alc5633_hp_mixr_mixer_controls[0], ARRAY_SIZE(alc5633_hp_mixr_mixer_controls)),
SND_SOC_DAPM_MIXER("AUXMIXL Mixer", ALC5633_PWR_MANAG_ADD2, 13, 0,
        &alc5633_auxmixl_mixer_controls[0], ARRAY_SIZE(alc5633_auxmixl_mixer_controls)),
SND_SOC_DAPM_MIXER("AUXMIXR Mixer", ALC5633_PWR_MANAG_ADD2, 12, 0,
        &alc5633_auxmixr_mixer_controls[0], ARRAY_SIZE(alc5633_auxmixr_mixer_controls)),
SND_SOC_DAPM_MIXER("SPXMIX Mixer", ALC5633_PWR_MANAG_ADD2, 0, 0,
        &alc5633_spkmixr_mixer_controls[0], ARRAY_SIZE(alc5633_spkmixr_mixer_controls)),

SND_SOC_DAPM_PGA_E("Left SPK Vol", ALC5633_PWR_MANAG_ADD4, 15, 0, NULL, 0,
                spk_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Right SPK Vol", ALC5633_PWR_MANAG_ADD4, 14, 0, NULL, 0,
                spk_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

SND_SOC_DAPM_PGA_E("Left HP Vol", ALC5633_PWR_MANAG_ADD4, 11, 0, NULL, 0,
                hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Right HP Vol", ALC5633_PWR_MANAG_ADD4, 10, 0, NULL, 0,
                hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Left AUX Out Vol", ALC5633_PWR_MANAG_ADD4, 9, 0, NULL, 0,
                auxout_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Right AUX Out Vol", ALC5633_PWR_MANAG_ADD4, 8, 0, NULL, 0,
                auxout_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

SND_SOC_DAPM_MUX("SPKO Mux", SND_SOC_NOPM, 0, 0, &alc5633_spo_mux_control),

SND_SOC_DAPM_MICBIAS("Mic Bias1", ALC5633_PWR_MANAG_ADD2, 3, 0),
SND_SOC_DAPM_MICBIAS("Mic Bias2", ALC5633_PWR_MANAG_ADD2, 2, 0),

SND_SOC_DAPM_OUTPUT("AUXOUTL"),
SND_SOC_DAPM_OUTPUT("AUXOUTR"),
SND_SOC_DAPM_OUTPUT("SPOL"),
SND_SOC_DAPM_OUTPUT("SPOR"),
SND_SOC_DAPM_OUTPUT("HPOL"),
SND_SOC_DAPM_OUTPUT("HPOR"),

};

static const struct snd_soc_dapm_route alc5633_dapm_routes[] = {
        {"Mic1 Boost", NULL, "MIC1"},
        {"Mic2 Boost", NULL, "MIC2"},


        {"LINE1L Inp Vol", NULL, "LINE1L"},
        {"LINE1R Inp Vol", NULL, "LINE1R"},

        {"LINE2L Inp Vol", NULL, "LINE2L"},
        {"LINE2R Inp Vol", NULL, "LINE2R"},


        {"RECMIXL Mixer", "HPMIXL Capture Switch", "HPMIXL Mixer"},
        {"RECMIXL Mixer", "AUXMIXL Capture Switch", "AUXMIXL Mixer"},
        {"RECMIXL Mixer", "SPKMIX Capture Switch", "SPXMIX Mixer"},
        {"RECMIXL Mixer", "LINE1L Capture Switch", "LINE1L Inp Vol"},
        {"RECMIXL Mixer", "LINE2L Capture Switch", "LINE2L Inp Vol"},
        {"RECMIXL Mixer", "MIC1 Capture Switch", "Mic1 Boost"},
        {"RECMIXL Mixer", "MIC2 Capture Switch", "Mic2 Boost"},

        {"RECMIXR Mixer", "HPMIXR Capture Switch", "HPMIXR Mixer"},
        {"RECMIXR Mixer", "AUXMIXR Capture Switch", "AUXMIXR Mixer"},
        {"RECMIXR Mixer", "SPKMIX Capture Switch", "SPXMIX Mixer"},
        {"RECMIXR Mixer", "LINE1R Capture Switch", "LINE1R Inp Vol"},
        {"RECMIXR Mixer", "LINE2R Capture Switch", "LINE2R Inp Vol"},
        {"RECMIXR Mixer", "MIC1 Capture Switch", "Mic1 Boost"},
        {"RECMIXR Mixer", "MIC2 Capture Switch", "Mic2 Boost"},

        {"Left ADC", NULL, "RECMIXL Mixer"},
        {"Right ADC", NULL, "RECMIXR Mixer"},

        {"HPMIXL Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
        {"HPMIXL Mixer", "MIC1 Playback Switch", "Mic1 Boost"},
        {"HPMIXL Mixer", "MIC2 Playback Switch", "Mic2 Boost"},
        {"HPMIXL Mixer", "LINE1 Playback Switch", "LINE1L Inp Vol"},
        {"HPMIXL Mixer", "LINE2 Playback Switch", "LINE2L Inp Vol"},
        {"HPMIXL Mixer", "DAC Playback Switch", "Left DAC"},

        {"HPMIXR Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},
        {"HPMIXR Mixer", "MIC1 Playback Switch", "Mic1 Boost"},
        {"HPMIXR Mixer", "MIC2 Playback Switch", "Mic2 Boost"},
        {"HPMIXR Mixer", "LINE1 Playback Switch", "LINE1R Inp Vol"},
        {"HPMIXR Mixer", "LINE2 Playback Switch", "LINE2R Inp Vol"},
        {"HPMIXR Mixer", "DAC Playback Switch", "Right DAC"},

        {"AUXMIXL Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
        {"AUXMIXL Mixer", "MIC1 Playback Switch", "Mic1 Boost"},
        {"AUXMIXL Mixer", "MIC2 Playback Switch", "Mic2 Boost"},
        {"AUXMIXL Mixer", "LINE1 Playback Switch", "LINE1L Inp Vol"},
        {"AUXMIXL Mixer", "LINE2 Playback Switch", "LINE2L Inp Vol"},
        {"AUXMIXL Mixer", "DAC Playback Switch", "Left DAC"},

        {"AUXMIXR Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},
        {"AUXMIXR Mixer", "MIC1 Playback Switch", "Mic1 Boost"},
        {"AUXMIXR Mixer", "MIC2 Playback Switch", "Mic2 Boost"},
        {"AUXMIXR Mixer", "LINE1 Playback Switch", "LINE1R Inp Vol"},
        {"AUXMIXR Mixer", "LINE2 Playback Switch", "LINE2R Inp Vol"},
        {"AUXMIXR Mixer", "DAC Playback Switch", "Right DAC"},

        {"SPXMIX Mixer", "MIC1 Playback Switch", "Mic1 Boost"},
        {"SPXMIX Mixer", "MIC2 Playback Switch", "Mic2 Boost"},
        {"SPXMIX Mixer", "DACL Playback Switch", "Left DAC"},
        {"SPXMIX Mixer", "DACR Playback Switch", "Right DAC"},
        {"SPXMIX Mixer", "LINE1L Playback Switch", "LINE1L Inp Vol"},
        {"SPXMIX Mixer", "LINE1R Playback Switch", "LINE1R Inp Vol"},
        {"SPXMIX Mixer", "LINE2L Playback Switch", "LINE2L Inp Vol"},
        {"SPXMIX Mixer", "LINE2R Playback Switch", "LINE2R Inp Vol"},

        {"SPKO Mux", "HPMIX", "HPMIXL Mixer"},
        {"SPKO Mux", "SPKMIX", "SPXMIX Mixer"},
        {"SPKO Mux", "AUXMIX", "AUXMIXL Mixer"},

        {"Left SPK Vol",  NULL, "SPKO Mux"},
        {"Right SPK Vol",  NULL, "SPKO Mux"},

        {"Right HP Vol",  NULL, "HPMIXR Mixer"},
        {"Left HP Vol",  NULL, "HPMIXL Mixer"},

        {"Left AUX Out Vol",  NULL, "AUXMIXL Mixer"},
        {"Right AUX Out Vol",  NULL, "AUXMIXR Mixer"},

        {"AUXOUTL", NULL, "Left AUX Out Vol"},
        {"AUXOUTR", NULL, "Right AUX Out Vol"},
        {"SPOL", NULL, "Left SPK Vol"},
        {"SPOR", NULL, "Right SPK Vol"},
        {"HPOL", NULL, "Left HP Vol"},
        {"HPOR", NULL, "Right HP Vol"},
};

struct _coeff_div{
        unsigned int mclk;       //pllout or MCLK
        unsigned int bclk;       //master mode
        unsigned int rate;
        unsigned int reg_val;
};

/* PLL divisors */
struct _pll_div {
	u32 pll_in;
	u32 pll_out;
	u16 regvalue;
};

static const struct _pll_div codec_master_pll_div[] = {

        {  2048000,  8192000,   0x0ea0},
        {  3686400,  8192000,   0x4e27},
        { 12000000,  8192000,   0x456b},
        { 13000000,  8192000,   0x495f},
        { 13100000,      8192000,       0x0320},
        {  2048000,  11289600,  0xf637},
        {  3686400,  11289600,  0x2f22},
        { 12000000,  11289600,  0x3e2f},
        { 13000000,  11289600,  0x4d5b},
        { 13100000,      11289600,      0x363b},
        {  2048000,  16384000,  0x1ea0},
        {  3686400,  16384000,  0x9e27},
        { 12000000,  16384000,  0x452b},
        { 13000000,  16384000,  0x542f},
        { 13100000,      16384000,      0x03a0},
        {  2048000,  16934400,  0xe625},
        {  3686400,  16934400,  0x9126},
        { 12000000,  16934400,  0x4d2c},
        { 13000000,  16934400,  0x742f},
        { 13100000,      16934400,      0x3c27},
        {  2048000,  22579200,  0x2aa0},
        {  3686400,  22579200,  0x2f20},
        { 12000000,  22579200,  0x7e2f},
        { 13000000,  22579200,  0x742f},
        { 13100000,      22579200,      0x3c27},
        {  2048000,  24576000,  0x2ea0},
        {  3686400,  24576000,  0xee27},
        { 12000000,  24576000,  0x2915},
        { 13000000,  24576000,  0x772e},
        { 13100000,      24576000,      0x0d20},
        { 26000000,  24576000,  0x2027},
        { 26000000,  22579200,  0x392f},
        { 24576000,  22579200,  0x0921},
        { 24576000,  24576000,  0x02a0},
};

static const struct _pll_div codec_slave_pll_div[] = {

        {  1024000,  16384000,  0x3ea0},
        {  1411200,  22579200,  0x3ea0},
        {  1536000,      24576000,      0x3ea0},
        {  2048000,  16384000,  0x1ea0},
        {  2822400,  22579200,  0x1ea0},
        {  3072000,      24576000,      0x1ea0},
        {       705600,  11289600,      0x3ea0},
        {       705600,   8467200,      0x3ab0},

};

struct _coeff_div coeff_div[] = {

        //sysclk is 256fs
        { 2048000,  8000 * 32,  8000, 0x1000},
        { 2048000,  8000 * 64,  8000, 0x0000},
        { 2822400, 11025 * 32, 11025, 0x1000},
        { 2822400, 11025 * 64, 11025, 0x0000},
        { 4096000, 16000 * 32, 16000, 0x1000},
        { 4096000, 16000 * 64, 16000, 0x0000},
        { 5644800, 22050 * 32, 22050, 0x1000},
        { 5644800, 22050 * 64, 22050, 0x0000},
        { 8192000, 32000 * 32, 32000, 0x1000},
        { 8192000, 32000 * 64, 32000, 0x0000},
        {11289600, 44100 * 32, 44100, 0x1000},
        {11289600, 44100 * 64, 44100, 0x0000},
        {12288000, 48000 * 32, 48000, 0x1000},
        {12288000, 48000 * 64, 48000, 0x0000},
        //sysclk is 512fs
        { 4096000,  8000 * 32,  8000, 0x3000},
        { 4096000,  8000 * 64,  8000, 0x2000},
        { 5644800, 11025 * 32, 11025, 0x3000},
        { 5644800, 11025 * 64, 11025, 0x2000},
        { 8192000, 16000 * 32, 16000, 0x3000},
        { 8192000, 16000 * 64, 16000, 0x2000},
        {11289600, 22050 * 32, 22050, 0x3000},
        {11289600, 22050 * 64, 22050, 0x2000},
        {16384000, 32000 * 32, 32000, 0x3000},
        {16384000, 32000 * 64, 32000, 0x2000},
        {22579200, 44100 * 32, 44100, 0x3000},
        {22579200, 44100 * 64, 44100, 0x2000},
        {24576000, 48000 * 32, 48000, 0x3000},
        {24576000, 48000 * 64, 48000, 0x2000},
        //SYSCLK is 22.5792Mhz or 24.576Mhz(8k to 48k)
        {24576000, 48000 * 32, 48000, 0x3000},
        {24576000, 48000 * 64, 48000, 0x2000},
        {22579200, 44100 * 32, 44100, 0x3000},
        {22579200, 44100 * 64, 44100, 0x2000},
        {24576000, 32000 * 32, 32000, 0x1080},
        {24576000, 32000 * 64, 32000, 0x0080},
        {22579200, 22050 * 32, 22050, 0x5000},
        {22579200, 22050 * 64, 22050, 0x4000},
        {24576000, 16000 * 32, 16000, 0x3080},
        {24576000, 16000 * 64, 16000, 0x2080},
        {22579200, 11025 * 32, 11025, 0x7000},
        {22579200, 11025 * 64, 11025, 0x6000},
        {24576000,      8000 * 32,      8000, 0x7080},
        {24576000,      8000 * 64,      8000, 0x6080},

};

static int get_coeff(int mclk, int rate, int timesofbclk)
{
        int i;

        for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
                if ((coeff_div[i].mclk == mclk)
                                && (coeff_div[i].rate == rate)
                                && ((coeff_div[i].bclk / coeff_div[i].rate) == timesofbclk))
                                return i;
        }

                return -1;
}

static unsigned int BLCK_FREQ=32; 
static int get_coeff_in_slave_mode(int mclk, int rate)
{
        return get_coeff(mclk, rate, BLCK_FREQ);
}

static int get_coeff_in_master_mode(int mclk, int rate)
{
        return get_coeff(mclk, rate ,BLCK_FREQ);
}

static int alc5633_hifi_pcm_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
        struct snd_soc_pcm_runtime *rtd = substream->private_data;
        struct snd_soc_codec *codec = rtd->codec;
        struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);
        unsigned int iface = 0;
        int rate = params_rate(params);
        int coeff = 0;

        printk(KERN_DEBUG "enter %s\n", __func__);
        if (!alc5633->master)
                coeff = get_coeff_in_slave_mode(alc5633->sysclk, rate);
        else
                coeff = get_coeff_in_master_mode(alc5633->sysclk, rate);
        if (coeff < 0) {
                printk(KERN_ERR "%s get_coeff err!\n", __func__);
        //      return -EINVAL;
        }
        switch (params_format(params))
        {
                case SNDRV_PCM_FORMAT_S16_LE:
                        break;
                case SNDRV_PCM_FORMAT_S20_3LE:
                        iface |= 0x0004;
                        break;
                case SNDRV_PCM_FORMAT_S24_LE:
                        iface |= 0x0008;
                        break;
                case SNDRV_PCM_FORMAT_S8:
                        iface |= 0x000c;
                        break;
                default:
                        return -EINVAL;
        }

        alc5633_write_mask(codec, ALC5633_SDP_CTRL, iface, SDP_I2S_DL_MASK);
        alc5633_write(codec, ALC5633_STEREO_AD_DA_CLK_CTRL, coeff_div[coeff].reg_val);
        alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD1, 0x81C0, 0x81C0);


//    alc5633_reg_dump(codec);

        return 0;
}

static int alc5633_hifi_codec_set_dai_fmt(
	struct snd_soc_dai *codec_dai, unsigned int fmt)
{
        struct snd_soc_codec *codec = codec_dai->codec;
        struct alc5633_priv *alc5633 =  snd_soc_codec_get_drvdata(codec);
        u16 iface = 0;

        printk(KERN_DEBUG "enter %s\n", __func__);
        switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
        case SND_SOC_DAIFMT_CBM_CFM:
                alc5633->master = 1;
                break;
        case SND_SOC_DAIFMT_CBS_CFS:
                iface |= (0x0001 << 15);
                alc5633->master = 0;
                break;
        default:
                return -EINVAL;
        }

        switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
        case SND_SOC_DAIFMT_I2S:
                break;
        case SND_SOC_DAIFMT_LEFT_J:
                iface |= (0x0001);
                break;
        case SND_SOC_DAIFMT_DSP_A:
                iface |= (0x0002);
                break;
        case SND_SOC_DAIFMT_DSP_B:
                iface  |= (0x0003);
                break;
        default:
                return -EINVAL;
        }

        switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_NF:
                break;
        case SND_SOC_DAIFMT_IB_NF:
                iface |= (0x0001 << 7);
                break;
        default:
                return -EINVAL;
        }

        alc5633_write(codec, ALC5633_SDP_CTRL, iface);
        return 0;

}

static int alc5633_hifi_codec_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				int clk_id, unsigned int freq, int dir)
{
        struct snd_soc_codec *codec = codec_dai->codec;
        struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);

        printk(KERN_DEBUG "enter %s\n", __func__);
        if ((freq >= (256 * 8000)) && (freq <= (512 * 96000))) {
                alc5633->sysclk = freq;
                return 0;
        }

        printk(KERN_ERR "unsupported sysclk freq %u for audio i2s\n", freq);

	return -EINVAL;
}

static int alc5633_codec_set_dai_pll(struct snd_soc_dai *codec_dai, int pll_id,
		int source, unsigned int freq_in, unsigned int freq_out)
{
        struct snd_soc_codec *codec = codec_dai->codec;
        struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);
        int i;
        int ret = -EINVAL;


        printk(KERN_DEBUG "enter %s:   freq_in[%d]  freq_out[%d]\n", __func__, freq_in, freq_out);
        alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD2, 0, PWR_PLL);

        if (!freq_in || !freq_out)
                return 0;

        if (alc5633->master) {
                for (i = 0; i < ARRAY_SIZE(codec_master_pll_div); i ++) {
                        if ((freq_in == codec_master_pll_div[i].pll_in) && (freq_out == codec_master_pll_div[i].pll_out)) {
                                alc5633_write(codec, ALC5633_PLL_CTRL, codec_master_pll_div[i].regvalue);
                                alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD2, PWR_PLL, PWR_PLL);
                                msleep(20);
                                alc5633_write(codec, ALC5633_GBL_CLK_CTRL, 0x0000);
                                ret = 0;
                        }
                }
        } else {
                for (i = 0; i < ARRAY_SIZE(codec_slave_pll_div); i ++) {
                        if ((freq_in == codec_slave_pll_div[i].pll_in) && (freq_out == codec_slave_pll_div[i].pll_out))  {
                                alc5633_write(codec, ALC5633_PLL_CTRL, codec_slave_pll_div[i].regvalue);
                                alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD2, PWR_PLL, PWR_PLL);
                                msleep(20);
                                alc5633_write(codec, ALC5633_GBL_CLK_CTRL, 0x6000);
                                ret = 0;
                        }
                }
        }

        return 0;
}

#if 0
#define ALC5633_SPK_VOL_MASK		0x3f
#define ALC5633_HP_VOL_MASK		ALC5633_VOL_MASK

/*
 * Speaker Volume control
 * 	amixer -c 0 cget numid=16
 *
 * Headphone Volume control
 * 	amixer -c 0 cget numid=20
 *
 * Mic boost control (MIC1=2, MIC2=4)
 * 	amixer -c 0 cget numid=2
 * 	amixer -c 0 cget numid=4
 */
static void alc5633_setup(struct snd_soc_codec *codec)
{
	int spk_vol_up = 4; 		// 0 ~ 8 : 0 ~ 12db
	int hp_vol_up  = 0x10; 		// 0 ~ 1f : -46.5 ~ 0 db (step 1.5)
	int mic_boost  = 1; 		// 0=bypass, 1=20, 2=24, 3=30, 4=35, 5=40, 6=44, 7=50, 8=+52 db

	snd_soc_write(codec, ALC5633_SPK_OUT_VOL, 0xc8c8);			// 02h: 0xc8c8
	snd_soc_write(codec, ALC5633_HP_OUT_VOL, 0xc0c0);			// 04h: 0x4848
	snd_soc_write(codec, ALC5633_MONO_AXO_1_2_VOL, 0xa080);
	snd_soc_write(codec, ALC5633_ADC_REC_MIXER, 0xb0b0);
	snd_soc_write(codec, ALC5633_MIC_CTRL_2, 0x1100);			// 22h: 5500, no boost
	snd_soc_write(codec, ALC5633_OUTMIXER_L_CTRL, 0xdfC0);
	snd_soc_write(codec, ALC5633_OUTMIXER_R_CTRL, 0xdfC0);
	snd_soc_write(codec, ALC5633_SPK_MIXER_CTRL, 0xe8e8);		// 28h: 0xd8d8
	snd_soc_write(codec, ALC5633_SPK_MONO_OUT_CTRL, 0x6c00);
	snd_soc_write(codec, ALC5633_GEN_PUR_CTRL_REG, 0x4e00);		// 40h: HP volume
	snd_soc_write(codec, ALC5633_SPK_MONO_HP_OUT_CTRL, 0x0000);

	/* 02h : speaker volume */
	snd_soc_update_bits(codec, ALC5633_SPK_OUT_VOL,
		ALC5633_SPK_VOL_MASK << ALC5633_L_VOL_SHIFT | ALC5633_SPK_VOL_MASK,
		abs(spk_vol_up - 0x8) << ALC5633_L_VOL_SHIFT | abs(spk_vol_up - 0x8));

	/* 04h : headphone volume */
	snd_soc_update_bits(codec, ALC5633_HP_OUT_VOL,
		ALC5633_VOL_MASK << ALC5633_L_VOL_SHIFT | ALC5633_VOL_MASK,
		abs(hp_vol_up - 0x1f) << ALC5633_L_VOL_SHIFT | abs(hp_vol_up - 0x1f));

	/* 22h : mic boost */
	snd_soc_update_bits(codec, ALC5633_MIC_CTRL_2,
		ALC5633_MIC1_BOOST_CTRL_MASK | ALC5633_MIC2_BOOST_CTRL_MASK,
		mic_boost << ALC5633_MIC1_BOOST_SHIFT |
		mic_boost << ALC5633_MIC2_BOOST_SHIFT);
}
#endif

static int alc5633_set_bias_level(struct snd_soc_codec *codec,
			enum snd_soc_bias_level level)
{
        printk(KERN_DEBUG "enter %s\n", __func__);

        switch (level) {
        case SND_SOC_BIAS_ON:
        case SND_SOC_BIAS_PREPARE:
                alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3,PWR_VREF|PWR_MAIN_BIAS, PWR_VREF|PWR_MAIN_BIAS);
                alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD2,0x0008, 0x0008);
                break;
        case SND_SOC_BIAS_STANDBY:
                if (codec->dapm.bias_level == SND_SOC_BIAS_OFF) {
                        alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3,PWR_VREF|PWR_MAIN_BIAS, PWR_VREF|PWR_MAIN_BIAS);
                        msleep(80);
                        alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3,PWR_DIS_FAST_VREF,PWR_DIS_FAST_VREF);

                        codec->cache_only = false;
                        snd_soc_cache_sync(codec);
                }
                break;
        case SND_SOC_BIAS_OFF:
                alc5633_write_mask(codec, ALC5633_SPK_OUT_VOL, 0x8000, 0x8000); //mute speaker volume
                alc5633_write_mask(codec, ALC5633_HP_OUT_VOL, 0x8080, 0x8080);  //mute hp volume
                alc5633_write(codec, ALC5633_PWR_MANAG_ADD1, 0x0000);
                alc5633_write(codec, ALC5633_PWR_MANAG_ADD2, 0x0000);
                alc5633_write(codec, ALC5633_PWR_MANAG_ADD3, 0x0000);
                alc5633_write(codec, ALC5633_PWR_MANAG_ADD4, 0x0000);
                break;
        }

	codec->dapm.bias_level = level;

	return 0;
}

/**
 * alc5633_index_show - Dump private registers.
 * @dev: codec device.
 * @attr: device attribute.
 * @buf: buffer for display.
 *
 * To show non-zero values of all private registers.
 *
 * Returns buffer length.
 */
static ssize_t alc5633_index_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct alc5633_priv *alc5633 = i2c_get_clientdata(client);
	struct snd_soc_codec *codec = alc5633->codec;
	unsigned int val;
	int cnt = 0, i;

	cnt += sprintf(buf, "ALC5633 index register\n");
#if 0
	for (i = 0; i <= 0x23; i++) {
		if (cnt + 9 >= PAGE_SIZE - 1)
			break;
		val = alc5633_index_read(codec, i);
		if (!val)
			continue;
		cnt += snprintf(buf + cnt, 10, "%02x: %04x\n", i, val);
	}

	if (cnt >= PAGE_SIZE)
		cnt = PAGE_SIZE - 1;

#endif
	return cnt;
}
static DEVICE_ATTR(index_reg, 0444, alc5633_index_show, NULL);

static int alc5633_probe(struct snd_soc_codec *codec)
{
	struct alc5633_priv *alc5633 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;
	int ret;

	pr_info("Codec driver version %s\n", VERSION);
      printk("%s\n", __func__);

	ret = snd_soc_codec_set_cache_io(codec, 8, 16, SND_SOC_I2C);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);
		return ret;
	}

	alc5633_reset(codec);
	alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3,
		PWR_VREF | PWR_MAIN_BIAS,
		PWR_VREF | PWR_MAIN_BIAS);
	msleep(110);
	alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3,
		PWR_DIS_FAST_VREF,PWR_DIS_FAST_VREF);	

      printk("call alc5633_reg_init\n");
	alc5633_reg_init(codec);

#if 0
#ifndef	VMID_ADD_WIDGET
	alc5633_setup(codec);
#endif
#endif

	codec->dapm.bias_level = SND_SOC_BIAS_STANDBY;
	alc5633->codec = codec;

	ret = device_create_file(codec->dev, &dev_attr_index_reg);
 	if (ret < 0) {
 		dev_err(codec->dev,
			"Failed to create index_reg sysfs files: %d\n", ret);
		return ret;
	}

	return 0;
}

static int alc5633_remove(struct snd_soc_codec *codec)
{
	alc5633_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

#ifdef CONFIG_PM
static int alc5633_suspend(struct snd_soc_codec *codec)
{
	alc5633_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int alc5633_resume(struct snd_soc_codec *codec)
{
	alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD3,
		PWR_VREF | PWR_MAIN_BIAS,
		PWR_VREF | PWR_MAIN_BIAS);
	msleep(110);
	alc5633_reg_init(codec);
	alc5633_write_mask(codec, ALC5633_PWR_MANAG_ADD1, 0x81C0, 0x81C0);

	alc5633_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	return 0;
}
#else
#define alc5633_suspend NULL
#define alc5633_resume NULL
#endif

#define ALC5633_STEREO_RATES SNDRV_PCM_RATE_8000_96000
#define ALC5633_VOICE_RATES (SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_8000)
#define ALC5633_FORMAT	(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S8)

struct snd_soc_dai_ops alc5633_ops = {
	.hw_params = alc5633_hifi_pcm_params,
	.set_fmt = alc5633_hifi_codec_set_dai_fmt,
	.set_sysclk = alc5633_hifi_codec_set_dai_sysclk,
	.set_pll = alc5633_codec_set_dai_pll,
};

struct snd_soc_dai_driver alc5633_dai[] = {
	{
		.name = "alc5633-hifi",
		.id = ALC5633_AIF1,
		.playback = {
			.stream_name = "HIFI Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = ALC5633_STEREO_RATES,
			.formats = ALC5633_FORMAT,
		},
		.capture = {
			.stream_name = "HIFI Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = ALC5633_STEREO_RATES,
			.formats = ALC5633_FORMAT,
		},
		.ops = &alc5633_ops,
	},
#if 0
	{
		.name = "alc5633-reserve",
		.id = ALC5633_AIF2,
	}
#endif
#if 1
	{
		.name = "alc5633-voice",
		.id = ALC5633_AIF2,
		.playback = {
			.stream_name = "Mono Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = ALC5633_VOICE_RATES,
			.formats = ALC5633_FORMAT,
		},
		.capture = {
			.stream_name = "Voice HIFI Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = ALC5633_VOICE_RATES,
			.formats = ALC5633_FORMAT,
		},
		.ops = &alc5633_ops,
	},
#endif
};

static struct snd_soc_codec_driver soc_codec_dev_alc5633 = {
	.probe = alc5633_probe,
	.remove = alc5633_remove,
	.suspend = alc5633_suspend,
	.resume = alc5633_resume,

	.set_bias_level = alc5633_set_bias_level,
	.reg_cache_size = ARRAY_SIZE(alc5633_reg),
	.reg_word_size = sizeof(u16),
	.reg_cache_default = alc5633_reg,

	//.volatile_register = alc5633_volatile_register,
	//.readable_register = alc5633_readable_register,
	//.reg_cache_step = 1,

	.controls = alc5633_snd_controls,
	.num_controls = ARRAY_SIZE(alc5633_snd_controls),
	.dapm_widgets = alc5633_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(alc5633_dapm_widgets),
	.dapm_routes = alc5633_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(alc5633_dapm_routes),
};

static const struct i2c_device_id alc5633_i2c_id[] = {
	{ "alc5633", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, alc5633_i2c_id);

static int alc5633_i2c_probe(struct i2c_client *i2c,
		    const struct i2c_device_id *id)
{
	struct alc5633_priv *alc5633;
	int ret;

	alc5633 = kzalloc(sizeof(struct alc5633_priv), GFP_KERNEL);
	if (NULL == alc5633)
		return -ENOMEM;

	alc5633->client = i2c;
	i2c_set_clientdata(i2c, alc5633);

	ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_alc5633,
			alc5633_dai, ARRAY_SIZE(alc5633_dai));

        if (ret < 0) {
                kfree(alc5633);
                printk("failed to initialise alc5633!\n");
        }

	return ret;
}

static __devexit int alc5633_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	kfree(i2c_get_clientdata(client));
	return 0;
}

static void alc5633_i2c_shutdown(struct i2c_client *client)
{
	struct alc5633_priv *alc5633 = i2c_get_clientdata(client);
	struct snd_soc_codec *codec = alc5633->codec;

	if (codec != NULL)
		alc5633_set_bias_level(codec, SND_SOC_BIAS_OFF);
}

struct i2c_driver alc5633_i2c_driver = {
	.driver = {
		.name = "alc5633",
		.owner = THIS_MODULE,
	},
	.probe = alc5633_i2c_probe,
	.remove = __devexit_p(alc5633_i2c_remove),
	.shutdown = alc5633_i2c_shutdown,
	.id_table = alc5633_i2c_id,
};

static int __init alc5633_modinit(void)
{
        int ret;

        printk(KERN_DEBUG "enter %s\n", __func__);
        printk("enter %s\n", __func__);
        ret = i2c_add_driver(&alc5633_i2c_driver);
        if (ret != 0) {
                printk(KERN_ERR "Failed to register ALC5633 I2C driver: %d\n", ret);
        }

        return ret;

}
module_init(alc5633_modinit);

static void __exit alc5633_modexit(void)
{
	i2c_del_driver(&alc5633_i2c_driver);
}
module_exit(alc5633_modexit);

MODULE_DESCRIPTION("ASoC ALC5633 driver");
MODULE_AUTHOR("flove <flove@realtek.com>");
MODULE_LICENSE("GPL");
