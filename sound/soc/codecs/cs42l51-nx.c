/*
 * cs42l51.c
 *
 * ASoC Driver for Cirrus Logic CS42L51 codecs
 *
 * Copyright (c) 2010 Arnaud Patard <apatard@mandriva.com>
 *
 * Based on cs4270.c - Copyright (c) Freescale Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * For now:
 *  - Only I2C is support. Not SPI
 *  - master mode *NOT* supported
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <mach/platform.h>
#include <mach/soc.h>

#include "cs42l51.h"

//#define FEATURE_ORG_SOURCE

enum master_slave_mode {
	MODE_SLAVE,
	MODE_SLAVE_AUTO,
	MODE_MASTER,
};

struct cs42l51_private {
	enum snd_soc_control_type control_type;
	void *control_data;
	unsigned int mclk;
	unsigned int audio_mode;	/* The mode (I2S or left-justified) */
	enum master_slave_mode func;
};

struct cs42l51_init_reg{
        u8 reg;
        u8 val;
};

#define CS42L51_FORMATS ( \
		SNDRV_PCM_FMTBIT_S16_LE  | SNDRV_PCM_FMTBIT_S16_BE  | \
		SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_S18_3BE | \
		SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S20_3BE | \
		SNDRV_PCM_FMTBIT_S24_LE  | SNDRV_PCM_FMTBIT_S24_BE)


#define I2C_RETRY_CNT	3

int cs42l51_i2c_write_byte(const struct i2c_client *client, u8 reg, uint8_t val)
{
	int i = 0;
	int ret = 0;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_smbus_write_byte_data(client, reg, val);

		if (ret < 0)
		{
			dev_err(&client->dev, "\e[31mfailed writing 0x%02x to 0x%02x\e[0m\n", reg, val);
			msleep(1);
		}
		else
			break;
	}
	return ret;
}

int cs42l51_i2c_write_word(const struct i2c_client *client, u8 reg, uint16_t val)
{
	int i = 0;
	int ret = 0;
	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_smbus_write_word_data(client, reg, val);
		if (ret < 0)
		{
			dev_err(&client->dev, "\e[31mfailed writing 0x%02x to 0x%04x\e[0m\n", reg, val);
			msleep(1);
		}
		else
			break;
	}

	return ret;
}

int cs42l51_i2c_write_block(const struct i2c_client *client, u8 reg, u8 len, uint8_t *val)
{
	int ret = 0;
	ret = i2c_smbus_write_i2c_block_data(client, reg, len, val);
	if (ret < 0)
		dev_err(&client->dev, "\e[31mfailed writings to 0x%02x\e[0m\n", reg);

	return ret;
}

int cs42l51_i2c_read_byte(const struct i2c_client *client, u8 reg, uint8_t *val)
{
	int ret = 0;

	ret = i2c_smbus_write_byte_data(client, 0xFF, 0);
	if (ret < 0)
		dev_err(&client->dev, "\e[31mfailed writing 0x%02x to 0x%02x\e[0m\n", 0xFF, 0);

	if (!ret)
		ret = i2c_smbus_read_byte_data(client, reg);
		if (ret < 0)
			dev_err(&client->dev, "\e[31mfailed reading at 0x%02x\e[0m\n", reg);
		else
			*val = (uint8_t)ret;

	return ret;
}

#ifdef FEATURE_ORG_SOURCE
int cs42l51_i2c_read_word(const struct i2c_client *client, u8 reg, uint16_t *val)
{
	int ret = 0;

	ret = i2c_smbus_write_byte_data(client, 0xFF, 0);
	if (ret < 0)
		dev_err(&client->dev, "\e[31mfailed writing 0x%02x to 0x%02x\e[0m\n", 0xFF, 0);

	if (!ret)
		ret = i2c_smbus_read_word_data(client, reg);
		if (ret < 0)
			dev_err(&client->dev, "\e[31mfailed reading at 0x%02x\e[0m\n", reg);
		else
			*val = (uint16_t)ret;

	return ret;
}

int cs42l51_i2c_read_block(const struct i2c_client *client, u8 reg, u8 len, uint8_t *val)
{
	int ret = 0;

	ret = i2c_smbus_write_byte_data(client, 0xFF, 0);
	if (ret < 0)
		dev_err(&client->dev, "\e[31mfailed writing 0x%02x to 0x%02x\e[0m\n", 0xFF, 0);

	if (!ret)
		ret = i2c_smbus_read_i2c_block_data(client, reg, len, val);
		if (ret < 0)
			dev_err(&client->dev, "\e[31mfailed reading from 0x%02x\e[0m\n", reg);

	return ret;
}

static int soc_cs42l51_write(struct snd_soc_codec *codec, unsigned reg, u_int val)
{
	u8 datas[2];
	int ret = 0;

	datas[0] = reg & 0xff; /*reg addr*/
	datas[1] = val & 0xff; /*reg val*/

	dev_info(codec->dev, "%s: addr:0x%x, data:0x%x", __func__, reg, val);

	if(codec->hw_write(codec->control_data, datas, 2) != 2)
		ret = -EIO;

	return ret;
}

static int soc_cs42l51_read(struct snd_soc_codec *codec, u_int reg)
{
	u8 data;
	u8 addr;

	addr = reg & 0xff;
	data = codec->hw_read(codec, addr);
	dev_info(codec->dev, "%s: addr:0x%x, data:0x%x", __func__, addr, data);

	return data;
}

static int cs42l51_fill_cache(struct snd_soc_codec *codec)
{
	u8 *cache = codec->reg_cache + 1;
	struct i2c_client *i2c_client = to_i2c_client(codec->dev);
	s32 length;

	length = cs42l51_i2c_read_block(i2c_client, CS42L51_FIRSTREG | 0x80, CS42L51_NUMREGS, cache);
	//length = i2c_smbus_read_i2c_block_data(i2c_client, CS42L51_FIRSTREG | 0x80, CS42L51_NUMREGS, cache);

	if (length != CS42L51_NUMREGS) {
		dev_err(&i2c_client->dev,
				"I2C read failure, addr=0x%x (ret=%d vs %d)\n",
				i2c_client->addr, length, CS42L51_NUMREGS);
		return -EIO;
	}

	return 0;
}

static int cs42l51_get_chan_mix(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned long value = snd_soc_read(codec, CS42L51_PCM_MIXER)&3;

	switch (value) {
	default:
	case 0:
		ucontrol->value.integer.value[0] = 0;
		break;
	/* same value : (L+R)/2 and (R+L)/2 */
	case 1:
	case 2:
		ucontrol->value.integer.value[0] = 1;
		break;
	case 3:
		ucontrol->value.integer.value[0] = 2;
		break;
	}

	return 0;
}

#define CHAN_MIX_NORMAL	0x00
#define CHAN_MIX_BOTH	0x55
#define CHAN_MIX_SWAP	0xFF

static int cs42l51_set_chan_mix(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned char val;

	switch (ucontrol->value.integer.value[0]) {
	default:
	case 0:
		val = CHAN_MIX_NORMAL;
		break;
	case 1:
		val = CHAN_MIX_BOTH;
		break;
	case 2:
		val = CHAN_MIX_SWAP;
		break;
	}

	snd_soc_write(codec, CS42L51_PCM_MIXER, val);

	return 1;
}

static const DECLARE_TLV_DB_SCALE(adc_pcm_tlv, -5150, 50, 0);
static const DECLARE_TLV_DB_SCALE(tone_tlv, -1050, 150, 0);
/* This is a lie. after -102 db, it stays at -102 */
/* maybe a range would be better */
static const DECLARE_TLV_DB_SCALE(aout_tlv, -11550, 50, 0);

static const DECLARE_TLV_DB_SCALE(boost_tlv, 1600, 1600, 0);
static const char *chan_mix[] = {
	"L R",
	"L+R",
	"R L",
};

static const struct soc_enum cs42l51_chan_mix =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(chan_mix), chan_mix);
#else
/**
 * snd_soc_info_volsw_2r - double mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a double mixer control that
 * spans 2 codec registers.
 *
 * Returns 0 for success.
 */
int snd_soc_cs42l5x_info_volsw_2r(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_info *uinfo)
{
    //struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

    int max = (kcontrol->private_value >> 8) & 0xff;
    int min = (kcontrol->private_value >> 16) & 0xff;

	//dev_info(codec->dev, "%s: min:%d, max:%d", __func__, min, max);

    if (max == 1)
        uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
    else
        uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;

    uinfo->count = 2;
    uinfo->value.integer.min = min;
    uinfo->value.integer.max = max;
    return 0;
}
/**
 * snd_soc_get_volsw_2r - double mixer get callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to get the value of a double mixer control that spans 2 registers.
 *
 * Returns 0 for success.
 */

int spk_vol = 0;
int mic_vol = 0;
//unsigned char spk_vol_tbl[] = {0x01, 0xDF, 0xE1, 0xE3, 0xE5, 0xE7, 0xE9, 0xEB, 0xED, 0xEF, 0xF1};  //cs42l52
//unsigned char mic_vol_tbl[] = {0x80, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};  //cs42l52
//unsigned char spk_vol_tbl[] = {0x01, 0x5F, 0x61, 0x63, 0x65, 0x67, 0x69, 0x6B, 0x6D, 0x6F, 0x71};
unsigned char spk_vol_tbl[] = {0x01, 0x65, 0x68, 0x6B, 0x6E, 0x71, 0x74, 0x77, 0x7A, 0x7D, 0x00};
unsigned char mic_vol_tbl[] = {0x80, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0};

int snd_soc_cs42l5x_get_volsw_2r(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
    int reg = kcontrol->private_value & 0xff;
    int reg2 = (kcontrol->private_value >> 24) & 0xff;
    //int max = (kcontrol->private_value >> 8) & 0xff;
    //int min = (kcontrol->private_value >> 16) & 0xff;
    //int mask = (1<<fls(max))-1;
    int val, val2;

    val = snd_soc_read(codec, reg);
    val2 = snd_soc_read(codec, reg2);
    //ucontrol->value.integer.value[0] = (val - min) & mask;
    //ucontrol->value.integer.value[1] = (val2 - min) & mask;

	//dev_info(codec->dev, "%s: val:%d, val2:%d", __func__, val, val2);

#if 0
    if(reg == CODEC_CS42L52_ADCA_MIXER_VOL)
    {
        if(val & 0x80)
            ucontrol->value.integer.value[0] = 1;
        else
            ucontrol->value.integer.value[0] = 0;

        if(val2 & 0x80)
            ucontrol->value.integer.value[1] = 1;
        else
            ucontrol->value.integer.value[1] = 0;
    }
    else 
#endif
    if(reg == CS42L51_ADCA_ATT)
    {
        ucontrol->value.integer.value[0] = mic_vol;
        ucontrol->value.integer.value[1] = mic_vol;
    }
    else
    {
        ucontrol->value.integer.value[0] = spk_vol;
        ucontrol->value.integer.value[1] = spk_vol;
    }
/*    
    SOCDBG("reg[%02x:%02x] = %02x:%02x ucontrol[%02x:%02x], min = %02x, max = %02x, mask %02x\n",
            reg, reg2, val,val2, 
            ucontrol->value.integer.value[0], ucontrol->value.integer.value[1], 
            min, max, mask);
*/
        return 0;
}

/**
 * snd_soc_put_volsw_2r - double mixer set callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to set the value of a double mixer control that spans 2 registers.
 *
 * Returns 0 for success.
 */
int snd_soc_cs42l5x_put_volsw_2r(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
    int reg = kcontrol->private_value & 0xff;
    int reg2 = (kcontrol->private_value >> 24) & 0xff;
    //int max = (kcontrol->private_value >> 8) & 0xff;
    //int min = (kcontrol->private_value >> 16) & 0xff;
    //int mask = (1 << fls(max)) - 1;
    int mask = 0xff;
    int err = 0;
    unsigned short val, val2;

    //val = (ucontrol->value.integer.value[0] + min) & mask;
    //val2 = (ucontrol->value.integer.value[1] + min) & mask;
    val = (ucontrol->value.integer.value[0]);
    val2 = (ucontrol->value.integer.value[1]);
	//dev_info(codec->dev, "%s: val:%d, val2:%d", __func__, val, val2);
#if 0
    if(reg == CODEC_CS42L52_ADCA_MIXER_VOL)
    {
        if(val)
        {
            val = 0x80;
            gpio_set_value(TCC_GPG(11), 0);
        }
        else
        {
            val = 0;
            gpio_set_value(TCC_GPG(11), 1);
        }

        if(val2)
            val2 = 0x80;
        else
            val2 = 0;
    }
    else 
#endif
    if(reg == CS42L51_ADCA_ATT)
    {
    //printk("** cs42l51 set mic\n");
        mic_vol = val;
        val = mic_vol_tbl[val];
        val2 = mic_vol_tbl[val2];
    }
    else
    {
    //printk("** cs42l51 set vol\n");
        spk_vol = val;
        val = spk_vol_tbl[val];
        val2 = spk_vol_tbl[val2];
    }
        return err;

    err = snd_soc_update_bits(codec, reg2, mask, val2);
    return err;
}

/* No shifts required */
#define SOC_DOUBLE_R_CS42L51(xname, reg_left, reg_right, max, min) \
{ \
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
	.name = (xname), \
	.info = snd_soc_cs42l5x_info_volsw_2r, \
	.get = snd_soc_cs42l5x_get_volsw_2r, \
	.put = snd_soc_cs42l5x_put_volsw_2r, \
	.private_value = (reg_left) | ((max) << 8) | ((min) << 16) | ((reg_right) << 24) \
}

// SOC_SINGLE(xname, reg, shift, max, invert)
#endif

static const struct snd_kcontrol_new cs42l51_snd_controls[] = {
#ifdef FEATURE_ORG_SOURCE
	SOC_DOUBLE_R_SX_TLV("PCM Playback Volume",
			CS42L51_PCMA_VOL, CS42L51_PCMB_VOL,
			7, 0xffffff99, 0x18, adc_pcm_tlv),
	SOC_DOUBLE_R("PCM Playback Switch",
			CS42L51_PCMA_VOL, CS42L51_PCMB_VOL, 7, 1, 1),
	SOC_DOUBLE_R_SX_TLV("Analog Playback Volume",
			CS42L51_AOUTA_VOL, CS42L51_AOUTB_VOL,
			8, 0xffffff19, 0x18, aout_tlv),
	SOC_DOUBLE_R_SX_TLV("ADC Mixer Volume",
			CS42L51_ADCA_VOL, CS42L51_ADCB_VOL,
			7, 0xffffff99, 0x18, adc_pcm_tlv),
	SOC_DOUBLE_R("ADC Mixer Switch",
			CS42L51_ADCA_VOL, CS42L51_ADCB_VOL, 7, 1, 1),
	SOC_SINGLE("Playback Deemphasis Switch", CS42L51_DAC_CTL, 3, 1, 0),
	SOC_SINGLE("Auto-Mute Switch", CS42L51_DAC_CTL, 2, 1, 0),
	SOC_SINGLE("Soft Ramp Switch", CS42L51_DAC_CTL, 1, 1, 0),
	SOC_SINGLE("Zero Cross Switch", CS42L51_DAC_CTL, 0, 0, 0),
	SOC_DOUBLE_TLV("Mic Boost Volume",
			CS42L51_MIC_CTL, 0, 1, 1, 0, boost_tlv),
	SOC_SINGLE_TLV("Bass Volume", CS42L51_TONE_CTL, 0, 0xf, 1, tone_tlv),
	SOC_SINGLE_TLV("Treble Volume", CS42L51_TONE_CTL, 4, 0xf, 1, tone_tlv),
	SOC_ENUM_EXT("PCM channel mixer",
			cs42l51_chan_mix,
			cs42l51_get_chan_mix, cs42l51_set_chan_mix),
#else
	SOC_SINGLE("Capture ADC Mute", CS42L51_ADC_INPUT, 0, 1, 1),  //ADCAMUTE
	SOC_SINGLE("Capture ADC PWR", CS42L51_POWER_CTL1, 1, 1, 1),  //ADCAPDN
	SOC_DOUBLE_R_CS42L51("Capture Mic Gain", CS42L51_ADCA_ATT, CS42L51_ADCB_ATT, 10, 0),

	//SOC_DOUBLE_R_CS42L51("Playback HP Volume", CS42L51_AOUTA_VOL, CS42L51_AOUTB_VOL, 10, 0), //-5dB ~ -32dB
	SOC_DOUBLE_R_CS42L51("Playback HP Volume", CS42L51_PCMA_VOL, CS42L51_PCMB_VOL, 10, 0), //-5dB ~ -32dB
	//SOC_DOUBLE_R_CS42L51("Playback Spk Volume", CODEC_CS42L52_SPKA_VOL, CODEC_CS42L52_SPKB_VOL, 10, 0), //-5dB ~ -32dB
	//SOC_DOUBLE_R_CS42L51("ADC Mixer", CODEC_CS42L52_ADCA_MIXER_VOL, CODEC_CS42L52_ADCB_MIXER_VOL, 1, 0),
#endif
};

#ifdef FEATURE_ORG_SOURCE
/*
 * to power down, one must:
 * 1.) Enable the PDN bit
 * 2.) enable power-down for the select channels
 * 3.) disable the PDN bit.
 */
static int cs42l51_pdn_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(w->codec, CS42L51_POWER_CTL1,
				    CS42L51_POWER_CTL1_PDN,
				    CS42L51_POWER_CTL1_PDN);
		break;
	default:
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_update_bits(w->codec, CS42L51_POWER_CTL1,
				    CS42L51_POWER_CTL1_PDN, 0);
		break;
	}

	return 0;
}

static const char *cs42l51_dac_names[] = {"Direct PCM",
	"DSP PCM", "ADC"};
static const struct soc_enum cs42l51_dac_mux_enum =
	SOC_ENUM_SINGLE(CS42L51_DAC_CTL, 6, 3, cs42l51_dac_names);
static const struct snd_kcontrol_new cs42l51_dac_mux_controls =
	SOC_DAPM_ENUM("Route", cs42l51_dac_mux_enum);

static const char *cs42l51_adcl_names[] = {"AIN1 Left", "AIN2 Left",
	"MIC Left", "MIC+preamp Left"};
static const struct soc_enum cs42l51_adcl_mux_enum =
	SOC_ENUM_SINGLE(CS42L51_ADC_INPUT, 4, 4, cs42l51_adcl_names);
static const struct snd_kcontrol_new cs42l51_adcl_mux_controls =
	SOC_DAPM_ENUM("Route", cs42l51_adcl_mux_enum);

static const char *cs42l51_adcr_names[] = {"AIN1 Right", "AIN2 Right",
	"MIC Right", "MIC+preamp Right"};
static const struct soc_enum cs42l51_adcr_mux_enum =
	SOC_ENUM_SINGLE(CS42L51_ADC_INPUT, 6, 4, cs42l51_adcr_names);
static const struct snd_kcontrol_new cs42l51_adcr_mux_controls =
	SOC_DAPM_ENUM("Route", cs42l51_adcr_mux_enum);
#endif

static const struct snd_soc_dapm_widget cs42l51_dapm_widgets[] = {
#ifdef FEATURE_ORG_SOURCE
	SND_SOC_DAPM_MICBIAS("Mic Bias", CS42L51_MIC_POWER_CTL, 1, 1),
	SND_SOC_DAPM_PGA_E("Left PGA", CS42L51_POWER_CTL1, 3, 1, NULL, 0,
		cs42l51_pdn_event, SND_SOC_DAPM_PRE_POST_PMD),
	SND_SOC_DAPM_PGA_E("Right PGA", CS42L51_POWER_CTL1, 4, 1, NULL, 0,
		cs42l51_pdn_event, SND_SOC_DAPM_PRE_POST_PMD),
	SND_SOC_DAPM_ADC_E("Left ADC", "Left HiFi Capture",
		CS42L51_POWER_CTL1, 1, 1,
		cs42l51_pdn_event, SND_SOC_DAPM_PRE_POST_PMD),
	SND_SOC_DAPM_ADC_E("Right ADC", "Right HiFi Capture",
		CS42L51_POWER_CTL1, 2, 1,
		cs42l51_pdn_event, SND_SOC_DAPM_PRE_POST_PMD),
	SND_SOC_DAPM_DAC_E("Left DAC", "Left HiFi Playback",
		CS42L51_POWER_CTL1, 5, 1,
		cs42l51_pdn_event, SND_SOC_DAPM_PRE_POST_PMD),
	SND_SOC_DAPM_DAC_E("Right DAC", "Right HiFi Playback",
		CS42L51_POWER_CTL1, 6, 1,
		cs42l51_pdn_event, SND_SOC_DAPM_PRE_POST_PMD),

	/* analog/mic */
	SND_SOC_DAPM_INPUT("AIN1L"),
	SND_SOC_DAPM_INPUT("AIN1R"),
	SND_SOC_DAPM_INPUT("AIN2L"),
	SND_SOC_DAPM_INPUT("AIN2R"),
	SND_SOC_DAPM_INPUT("MICL"),
	SND_SOC_DAPM_INPUT("MICR"),

	SND_SOC_DAPM_MIXER("Mic Preamp Left",
		CS42L51_MIC_POWER_CTL, 2, 1, NULL, 0),
	SND_SOC_DAPM_MIXER("Mic Preamp Right",
		CS42L51_MIC_POWER_CTL, 3, 1, NULL, 0),

	/* HP */
	SND_SOC_DAPM_OUTPUT("HPL"),
	SND_SOC_DAPM_OUTPUT("HPR"),

	/* mux */
	SND_SOC_DAPM_MUX("DAC Mux", SND_SOC_NOPM, 0, 0,
		&cs42l51_dac_mux_controls),
	SND_SOC_DAPM_MUX("PGA-ADC Mux Left", SND_SOC_NOPM, 0, 0,
		&cs42l51_adcl_mux_controls),
	SND_SOC_DAPM_MUX("PGA-ADC Mux Right", SND_SOC_NOPM, 0, 0,
		&cs42l51_adcr_mux_controls),
#else
	SND_SOC_DAPM_ADC("Left ADC", "Capture", SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_INPUT("MICA"),

	/*output path*/
	SND_SOC_DAPM_DAC("Left SPK DAC", "Playback", SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_OUTPUT("SPKA"),
#endif
};

static const struct snd_soc_dapm_route cs42l51_routes[] = {
#ifdef FEATURE_ORG_SOURCE
	{"HPL", NULL, "Left DAC"},
	{"HPR", NULL, "Right DAC"},

	{"Left ADC", NULL, "Left PGA"},
	{"Right ADC", NULL, "Right PGA"},

	{"Mic Preamp Left",  NULL,  "MICL"},
	{"Mic Preamp Right", NULL,  "MICR"},

	{"PGA-ADC Mux Left",  "AIN1 Left",        "AIN1L" },
	{"PGA-ADC Mux Left",  "AIN2 Left",        "AIN2L" },
	{"PGA-ADC Mux Left",  "MIC Left",         "MICL"  },
	{"PGA-ADC Mux Left",  "MIC+preamp Left",  "Mic Preamp Left" },
	{"PGA-ADC Mux Right", "AIN1 Right",       "AIN1R" },
	{"PGA-ADC Mux Right", "AIN2 Right",       "AIN2R" },
	{"PGA-ADC Mux Right", "MIC Right",        "MICR" },
	{"PGA-ADC Mux Right", "MIC+preamp Right", "Mic Preamp Right" },

	{"Left PGA", NULL, "PGA-ADC Mux Left"},
	{"Right PGA", NULL, "PGA-ADC Mux Right"},
#else
	//{ sink, control, source}
	{"Left ADC", NULL, "MICA"},

	/*output map*/
	{"SPKA", NULL, "Left SPK DAC"},
#endif
};

static int cs42l51_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int format)
{
	int ret = 0;
#ifdef FEATURE_ORG_SOURCE
	struct snd_soc_codec *codec = codec_dai->codec;
	struct cs42l51_private *cs42l51 = snd_soc_codec_get_drvdata(codec);

	dev_info(codec->dev, "%s: format:%d", __func__, format);

	switch (format & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
	case SND_SOC_DAIFMT_RIGHT_J:
		cs42l51->audio_mode = format & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	default:
		dev_err(codec->dev, "\e[31minvalid DAI format\e[0m\n");
		ret = -EINVAL;
	}

	switch (format & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		cs42l51->func = MODE_MASTER;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		cs42l51->func = MODE_SLAVE_AUTO;
		break;
	default:
		ret = -EINVAL;
		break;
	}
#endif

	return ret;
}

#ifdef FEATURE_ORG_SOURCE
struct cs42l51_ratios {
	unsigned int ratio;
	unsigned char speed_mode;
	unsigned char mclk;
};

static struct cs42l51_ratios slave_ratios[] = {
	{  512, CS42L51_QSM_MODE, 0 }, {  768, CS42L51_QSM_MODE, 0 },
	{ 1024, CS42L51_QSM_MODE, 0 }, { 1536, CS42L51_QSM_MODE, 0 },
	{ 2048, CS42L51_QSM_MODE, 0 }, { 3072, CS42L51_QSM_MODE, 0 },
	{  256, CS42L51_HSM_MODE, 0 }, {  384, CS42L51_HSM_MODE, 0 },
	{  512, CS42L51_HSM_MODE, 0 }, {  768, CS42L51_HSM_MODE, 0 },
	{ 1024, CS42L51_HSM_MODE, 0 }, { 1536, CS42L51_HSM_MODE, 0 },
	{  128, CS42L51_SSM_MODE, 0 }, {  192, CS42L51_SSM_MODE, 0 },
	{  256, CS42L51_SSM_MODE, 0 }, {  384, CS42L51_SSM_MODE, 0 },
	{  512, CS42L51_SSM_MODE, 0 }, {  768, CS42L51_SSM_MODE, 0 },
	{  128, CS42L51_DSM_MODE, 0 }, {  192, CS42L51_DSM_MODE, 0 },
	{  256, CS42L51_DSM_MODE, 0 }, {  384, CS42L51_DSM_MODE, 0 },
};

static struct cs42l51_ratios slave_auto_ratios[] = {
	{ 1024, CS42L51_QSM_MODE, 0 }, { 1536, CS42L51_QSM_MODE, 0 },
	{ 2048, CS42L51_QSM_MODE, 1 }, { 3072, CS42L51_QSM_MODE, 1 },
	{  512, CS42L51_HSM_MODE, 0 }, {  768, CS42L51_HSM_MODE, 0 },
	{ 1024, CS42L51_HSM_MODE, 1 }, { 1536, CS42L51_HSM_MODE, 1 },
	{  256, CS42L51_SSM_MODE, 0 }, {  384, CS42L51_SSM_MODE, 0 },
	{  512, CS42L51_SSM_MODE, 1 }, {  768, CS42L51_SSM_MODE, 1 },
	{  128, CS42L51_DSM_MODE, 0 }, {  192, CS42L51_DSM_MODE, 0 },
	{  256, CS42L51_DSM_MODE, 1 }, {  384, CS42L51_DSM_MODE, 1 },
};
#endif

static int cs42l51_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct cs42l51_private *cs42l51 = snd_soc_codec_get_drvdata(codec);

	dev_info(codec->dev, "%s: freq:%dHz", __func__, freq);

	cs42l51->mclk = freq;
	return 0;
}

static int cs42l51_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai)
{
#ifdef FEATURE_ORG_SOURCE
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct cs42l51_private *cs42l51 = snd_soc_codec_get_drvdata(codec);

	int ret;
	unsigned int i;
	unsigned int rate;
	unsigned int ratio;
	struct cs42l51_ratios *ratios = NULL;
	int nr_ratios = 0;
	int intf_ctl, power_ctl, fmt;

	dev_info(codec->dev, "%s: func:%d", __func__, cs42l51->func);

	switch (cs42l51->func) {
	case MODE_MASTER:
		return -EINVAL;
	case MODE_SLAVE:
		ratios = slave_ratios;
		nr_ratios = ARRAY_SIZE(slave_ratios);
		break;
	case MODE_SLAVE_AUTO:
		ratios = slave_auto_ratios;
		nr_ratios = ARRAY_SIZE(slave_auto_ratios);
		break;
	}

	/* Figure out which MCLK/LRCK ratio to use */
	rate = params_rate(params);     /* Sampling rate, in Hz */
	ratio = cs42l51->mclk / rate;    /* MCLK/LRCK ratio */
	for (i = 0; i < nr_ratios; i++) {
		if (ratios[i].ratio == ratio)
			break;
	}

	if (i == nr_ratios) {
		/* We did not find a matching ratio */
		dev_err(codec->dev, "\e[31mcould not find matching ratio\e[0m\n");
		return -EINVAL;
	}

	intf_ctl = snd_soc_read(codec, CS42L51_INTF_CTL);
	power_ctl = snd_soc_read(codec, CS42L51_MIC_POWER_CTL);

	intf_ctl &= ~(CS42L51_INTF_CTL_MASTER | CS42L51_INTF_CTL_ADC_I2S
			| CS42L51_INTF_CTL_DAC_FORMAT(7));
	power_ctl &= ~(CS42L51_MIC_POWER_CTL_SPEED(3)
			| CS42L51_MIC_POWER_CTL_MCLK_DIV2);

	switch (cs42l51->func) {
	case MODE_MASTER:
		intf_ctl |= CS42L51_INTF_CTL_MASTER;
		power_ctl |= CS42L51_MIC_POWER_CTL_SPEED(ratios[i].speed_mode);
		break;
	case MODE_SLAVE:
		power_ctl |= CS42L51_MIC_POWER_CTL_SPEED(ratios[i].speed_mode);
		break;
	case MODE_SLAVE_AUTO:
		power_ctl |= CS42L51_MIC_POWER_CTL_AUTO;
		break;
	}

	switch (cs42l51->audio_mode) {
	case SND_SOC_DAIFMT_I2S:
		intf_ctl |= CS42L51_INTF_CTL_ADC_I2S;
		intf_ctl |= CS42L51_INTF_CTL_DAC_FORMAT(CS42L51_DAC_DIF_I2S);
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		intf_ctl |= CS42L51_INTF_CTL_DAC_FORMAT(CS42L51_DAC_DIF_LJ24);
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S16_LE:
		case SNDRV_PCM_FORMAT_S16_BE:
			fmt = CS42L51_DAC_DIF_RJ16;
			break;
		case SNDRV_PCM_FORMAT_S18_3LE:
		case SNDRV_PCM_FORMAT_S18_3BE:
			fmt = CS42L51_DAC_DIF_RJ18;
			break;
		case SNDRV_PCM_FORMAT_S20_3LE:
		case SNDRV_PCM_FORMAT_S20_3BE:
			fmt = CS42L51_DAC_DIF_RJ20;
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S24_BE:
			fmt = CS42L51_DAC_DIF_RJ24;
			break;
		default:
			dev_err(codec->dev, "\e[31munknown format\e[0m\n");
			return -EINVAL;
		}
		intf_ctl |= CS42L51_INTF_CTL_DAC_FORMAT(fmt);
		break;
	default:
		dev_err(codec->dev, "\e[31munknown format\e[0m\n");
		return -EINVAL;
	}

	if (ratios[i].mclk)
		power_ctl |= CS42L51_MIC_POWER_CTL_MCLK_DIV2;

	ret = snd_soc_write(codec, CS42L51_INTF_CTL, intf_ctl);
	if (ret < 0)
		return ret;

	ret = snd_soc_write(codec, CS42L51_MIC_POWER_CTL, power_ctl);
	if (ret < 0)
		return ret;

#endif
	return 0;
}

static int cs42l51_dai_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	int reg;
	int mask = CS42L51_DAC_OUT_CTL_DACA_MUTE|CS42L51_DAC_OUT_CTL_DACB_MUTE;

	reg = snd_soc_read(codec, CS42L51_DAC_OUT_CTL);

	if (mute)
		reg |= mask;
	else
		reg &= ~mask;

	return snd_soc_write(codec, CS42L51_DAC_OUT_CTL, reg);
}

static int cs42l51_set_bias_level(struct snd_soc_codec *codec, enum snd_soc_bias_level level)
{
	struct i2c_client *i2c_client = to_i2c_client(codec->dev);

#if 1
	switch (level) {
	case SND_SOC_BIAS_ON:
		//dev_info(&i2c_client->dev, "%s: SND_SOC_BIAS_ON\n", __func__);
		nxp_soc_gpio_set_out_value(CFG_IO_HP_MUTE, 0);
		break;

	case SND_SOC_BIAS_PREPARE:
		//dev_info(&i2c_client->dev, "%s: SND_SOC_BIAS_PREPARE\n", __func__);
		break;

	case SND_SOC_BIAS_STANDBY:
		//dev_info(&i2c_client->dev, "%s: SND_SOC_BIAS_STANDBY\n", __func__);
		nxp_soc_gpio_set_out_value(CFG_IO_HP_MUTE, 1);
		break;

	case SND_SOC_BIAS_OFF:
		//dev_info(&i2c_client->dev, "%s: SND_SOC_BIAS_OFF\n", __func__);
		break;

	default:
		break;
	}
#else
	switch (level) {
	case SND_SOC_BIAS_ON:
#ifdef	VMID_ADD_WIDGET
		rt5631_setup(codec);
#endif
		break;

	case SND_SOC_BIAS_PREPARE:
		snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE,
			RT5631_L_MUTE | RT5631_R_MUTE);
		snd_soc_update_bits(codec, RT5631_HP_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE,
			RT5631_L_MUTE | RT5631_R_MUTE);
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MICBIAS1_VOL | RT5631_PWR_MICBIAS2_VOL,
			RT5631_PWR_MICBIAS1_VOL | RT5631_PWR_MICBIAS2_VOL);
		break;

	case SND_SOC_BIAS_STANDBY:
		if (codec->dapm.bias_level == SND_SOC_BIAS_OFF) {
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
				RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS,
				RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
			msleep(80);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
				RT5631_PWR_FAST_VREF_CTRL,
				RT5631_PWR_FAST_VREF_CTRL);
			codec->cache_only = false;
			codec->cache_sync = 1;
			snd_soc_cache_sync(codec);
			rt5631_index_sync(codec);
		}
		break;

	case SND_SOC_BIAS_OFF:
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD1, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD2, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD3, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD4, 0x0000);
		break;

	default:
		break;
	}
#endif
	codec->dapm.bias_level = level;

	return 0;
}

static const struct snd_soc_dai_ops cs42l51_dai_ops = {
	.hw_params      = cs42l51_hw_params,
	.set_sysclk     = cs42l51_set_dai_sysclk,
	.set_fmt        = cs42l51_set_dai_fmt,
	.digital_mute   = cs42l51_dai_mute,
};

static struct snd_soc_dai_driver cs42l51_dai = {
	.name = "cs42l51-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = CS42L51_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = CS42L51_FORMATS,
	},
	.ops = &cs42l51_dai_ops,
};

static struct cs42l51_init_reg init_list[] = {
	/* initialize CS42L51 */
	{CS42L51_POWER_CTL1, 0x7F},			// 0x02      // power down
	{CS42L51_POWER_CTL1, 0x15},			// 0x02

	// Power down
	{CS42L51_MIC_POWER_CTL, 0xA8},		// 0x03                // Mic. Power Down

	// Interface set
	{CS42L51_INTF_CTL, 0x0C},  			// 0x04

	// AIN/MIC
	{CS42L51_MIC_CTL, 	CS42L51_MIC_CTL_ADC_SNGVOL|
						CS42L51_MIC_CTL_ADCA_DBOOST |
						CS42L51_MIC_CTL_MICA_BOOST },		// 0x05 : 0xa1
	{CS42L51_ADC_INPUT, CS42L51_ADC_INPUT_AINA_MUX(3) |
						CS42L51_ADC_INPUT_ADCB_MUTE},		// 0x07 : 0x32
	{CS42L51_ALC_PGA_CTL, CS42L51_ALC_PGX_PGX_VOL(0x10)},	// 0x0A : 0x10
	{CS42L51_ADCA_VOL, CS42L51_MIX_VOLUME(0xA)},			// 0x0E : 0x0a
	{CS42L51_ALC_EN, 0xC0},									// 0x1C : 0xc0

	// AOUT
	{CS42L51_DAC_OUT_CTL, CS42L51_DAC_OUT_CTL_HP_GAIN(6)|
						CS42L51_DAC_OUT_CTL_DACB_MUTE },	// 0x08              // HP_GAIN 1.0, default mute 
	{CS42L51_DAC_CTL,	CS42L51_DAC_CTL_DATA_SEL(1)|
						CS42L51_DAC_CTL_AMUTE|CS42L51_DAC_CTL_DACSZ(2)}, 	// 0x09
	{CS42L51_PCMA_VOL, 0x00},	// 0x10
	{CS42L51_PCMB_VOL, CS42L51_MIX_VOLUME(0xA)},	// 0x11
	{CS42L51_AOUTA_VOL, 0x00},	// 0x16
	{CS42L51_AOUTB_VOL, CS42L51_MIX_VOLUME(0xA)},	// 0x17

	// POWER
	{CS42L51_POWER_CTL1, CS42L51_POWER_CTL1_PDN_PGAB |
						CS42L51_POWER_CTL1_PDN_ADCB },	// 0x02
};

#define CS42L51_INIT_REG_LEN     ARRAY_SIZE(init_list)

static int cs42l51_reg_init(struct snd_soc_codec *codec)
{
	struct i2c_client *i2c_client = to_i2c_client(codec->dev);
	int i = 0;

	for (i = 0; i < CS42L51_INIT_REG_LEN; i ++) {
		cs42l51_i2c_write_byte(i2c_client, init_list[i].reg, init_list[i].val);
	}

	return 0;
}

static inline int soc_cs42l51_get_revison(struct snd_soc_codec *codec)
{
        u8 data;
        u8 addr;
        int ret = 0;

        addr = CS42L51_CHIP_REV_ID;
        data = codec->hw_read(codec, addr);
        if((data & CHIP_ID_MASK) != CS42L51_CHIP_ID)
        {
                printk("\e[31mdata is %x\e[0m\n",  data);
                ret = -ENODEV;
        }

        return ret < 0 ? ret : data;
}

static int cs42l51_probe(struct snd_soc_codec *codec)
{
	int ret;
	struct cs42l51_private *cs42l51 = snd_soc_codec_get_drvdata(codec);
#ifdef FEATURE_ORG_SOURCE
	struct i2c_client *i2c_client = to_i2c_client(codec->dev);
	int reg;

	ret = cs42l51_fill_cache(codec);
	if (ret < 0) {
		dev_err(codec->dev, "failed to fill register cache\n");
		return ret;
	}
#endif

	ret = snd_soc_codec_set_cache_io(codec, 8, 8, cs42l51->control_type);
	if (ret < 0) {
		dev_err(codec->dev, "\e[31mFailed to set cache I/O: %d\e[0m\n", ret);
		return ret;
	}


       //spk off
       // tcc_gpio_config(TCC_GPG(11), GPIO_FN(0));
       // gpio_request(TCC_GPG(11), "CS42L51_MUTE");
       // gpio_direction_output(TCC_GPG(11), 1);        // Speaker 1 : PWR disable, 0: PWR enable


#ifdef FEATURE_ORG_SOURCE
	/*
	 * DAC configuration
	 * - Use signal processor
	 * - auto mute
	 * - vol changes immediate
	 * - no de-emphasize
	 */
	reg = CS42L51_DAC_CTL_DATA_SEL(1)
		| CS42L51_DAC_CTL_AMUTE | CS42L51_DAC_CTL_DACSZ(0);
	ret = snd_soc_write(codec, CS42L51_DAC_CTL, reg);
	if (ret < 0)
	{
		dev_err(codec->dev, "snd_soc_write failure, ret=%d \n", ret);
		return ret;
	}
#else

	//snd_soc_add_controls(codec, cs42l51_snd_controls, ARRAY_SIZE(cs42l51_snd_controls));
	cs42l51_reg_init(codec);

#endif

	return 0;
}

#if 0//def CONFIG_PM
static int cs42l51_suspend(struct snd_soc_codec *codec)
{
	cs42l51_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int cs42l51_resume(struct snd_soc_codec *codec)
{
#if (1)
	u16 *value = codec->reg_cache;
	int i = 0;

	cs42l51_reset(codec);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
		RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS,
		RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
	msleep(80);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
		RT5631_PWR_FAST_VREF_CTRL, RT5631_PWR_FAST_VREF_CTRL);

	for (i = 0; RT5631_VENDOR_ID1 + 1 > i; i++) {
		if (i == RT5631_RESET ||
			i == RT5631_PWR_MANAG_ADD3)
			continue;
		snd_soc_write(codec, i, value[i]);
	}
#endif

	cs42l51_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	return 0;
}
#else
#define cs42l51_suspend NULL
#define cs42l51_resume NULL
#endif


static struct snd_soc_codec_driver soc_codec_device_cs42l51 = {
#ifdef FEATURE_ORG_SOURCE
	.probe = cs42l51_probe,
	.reg_cache_size = CS42L51_NUMREGS + 1,
	.reg_word_size = sizeof(u8),
#else
	.probe = cs42l51_probe,
	//.remove = cs42l51_remove,
	.suspend = cs42l51_suspend,
	.resume = cs42l51_resume,
	.set_bias_level = cs42l51_set_bias_level,

	.reg_cache_size = CS42L51_NUMREGS + 1,
	.reg_word_size = sizeof(u16),
#endif

	.controls = cs42l51_snd_controls,
	.num_controls = ARRAY_SIZE(cs42l51_snd_controls),
	.dapm_widgets = cs42l51_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(cs42l51_dapm_widgets),
	.dapm_routes = cs42l51_routes,
	.num_dapm_routes = ARRAY_SIZE(cs42l51_routes),
};

static int cs42l51_i2c_probe(struct i2c_client *i2c_client,
	const struct i2c_device_id *id)
{
	struct cs42l51_private *cs42l51;
	int ret;
	uint8_t val;

	/* Verify that we have a CS42L51 */
	ret = cs42l51_i2c_read_byte(i2c_client, CS42L51_CHIP_REV_ID, &val);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "%s() : \e[31mfailed to read I2C\e[0m\n", __func__);
		goto error;
	}

	if ((val != CS42L51_MK_CHIP_REV(CS42L51_CHIP_ID, CS42L51_CHIP_REV_A)) &&
	    (val != CS42L51_MK_CHIP_REV(CS42L51_CHIP_ID, CS42L51_CHIP_REV_B))) {
		dev_err(&i2c_client->dev, "\e[31mInvalid chip id\e[0m\n");
		ret = -ENODEV;
		goto error;
	}

	dev_info(&i2c_client->dev, "found device cs42l51 CHIP_ID:0x%x, rev:0x%x\n", val>>3, val&7);

	cs42l51 = devm_kzalloc(&i2c_client->dev, sizeof(struct cs42l51_private), GFP_KERNEL);
	if (!cs42l51) {
		dev_err(&i2c_client->dev, "\e[31mcould not allocate codec\e[0m\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c_client, cs42l51);
	cs42l51->control_type = SND_SOC_I2C;

	ret =  snd_soc_register_codec(&i2c_client->dev, &soc_codec_device_cs42l51, &cs42l51_dai, 1);

	nxp_soc_gpio_set_io_dir(CFG_IO_HP_MUTE, 1);
	nxp_soc_gpio_set_io_func(CFG_IO_HP_MUTE, nxp_soc_gpio_get_altnum(CFG_IO_HP_MUTE));
	nxp_soc_gpio_set_out_value(CFG_IO_HP_MUTE, 1);
	
error:
	return ret;
}

static int cs42l51_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	return 0;
}

#if 0
static void cs42l51_i2c_shutdown(struct i2c_client *client)
{
	struct cs42l51_private *cs42l51 = i2c_get_clientdata(client);
	struct snd_soc_codec *codec = cs42l51->codec;

	if (codec != NULL)
		cs42l51_set_bias_level(codec, SND_SOC_BIAS_OFF);
}
#endif

static const struct i2c_device_id cs42l51_id[] = {
	{"cs42l51", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, cs42l51_id);

static struct i2c_driver cs42l51_i2c_driver = {
	.driver = {
		.name = "cs42l51",
		.owner = THIS_MODULE,
	},
	.id_table = cs42l51_id,
	.probe = cs42l51_i2c_probe,
	.remove = cs42l51_i2c_remove,
	//.shutdown = cs42l51_i2c_shutdown,
};

static int __init cs42l51_init(void)
{
	int ret;

	ret = i2c_add_driver(&cs42l51_i2c_driver);
	if (ret != 0) {
		printk(KERN_ERR "%s: \e[31mcan't add i2c driver\e[0m\n", __func__);
		return ret;
	}
	return 0;
}

static void __exit cs42l51_exit(void)
{
	i2c_del_driver(&cs42l51_i2c_driver);
}

module_init(cs42l51_init);
module_exit(cs42l51_exit);

MODULE_AUTHOR("Arnaud Patard <arnaud.patard@rtp-net.org>");
MODULE_DESCRIPTION("Cirrus Logic CS42L51 ALSA SoC Codec Driver");
MODULE_LICENSE("GPL");
