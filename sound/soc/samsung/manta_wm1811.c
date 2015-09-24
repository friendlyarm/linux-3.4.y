/*
 * Copyright (C) 2012 Google, Inc.
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 * Copyright (C) 2012 Wolfson Microelectronics
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/mfd/wm8994/registers.h>

#include <plat/adc.h>

#include <sound/jack.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "../codecs/wm8994.h"
#include "../../../arch/arm/mach-exynos/board-manta.h"

#define MCLK1_FREQ	24000000
#define MCLK2_FREQ	32768

#define EAR_ADC_CHANNEL		7
#define ADC_HEADPHONE_3POLE	0x4BA
#define ADC_HEADSET_4POLE	0xED8
/* HEADPHONE: [2] - <3 ohm, [1] - Valid, [0] - Mic Accessory is present */
#define STATUS_HEADPHONE_3POLE	0x7
/* HEADSET: [10] - >475 ohm, [1] - Valid, [0] - Mic Accessory is present */
#define STATUS_HEADSET_4POLE	0x403
#define ADC_MIC_TEST_NUM	10
#define ADC_MIC_WAIT_US		10000

struct manta_wm1811 {
	struct clk *clk;
	unsigned int pll1_out;
	unsigned int prev_pll1_out;
	unsigned int pll2_out;
	unsigned int prev_pll2_out;
	struct snd_soc_jack jack;
	struct s3c_adc_client *adc_client;
};

static const struct snd_kcontrol_new manta_controls[] = {
	SOC_DAPM_PIN_SWITCH("HP"),
	SOC_DAPM_PIN_SWITCH("SPK"),
};

const struct snd_soc_dapm_widget manta_widgets_lunchbox[] = {
	SND_SOC_DAPM_HP("HP", NULL),
	SND_SOC_DAPM_SPK("SPK", NULL),

	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_MIC("Main Mic", NULL),
	SND_SOC_DAPM_MIC("Sub Mic", NULL),

	SND_SOC_DAPM_INPUT("S5P RP"),
};

const struct snd_soc_dapm_widget manta_widgets[] = {
	SND_SOC_DAPM_HP("HP", NULL),
	SND_SOC_DAPM_SPK("SPK", NULL),

	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_MIC("Main Mic", NULL),
	SND_SOC_DAPM_MIC("2nd Mic", NULL),
	SND_SOC_DAPM_MIC("3rd Mic", NULL),

	SND_SOC_DAPM_INPUT("S5P RP"),
};

const struct snd_soc_dapm_route manta_paths_lunchbox[] = {
	{ "HP", NULL, "HPOUT1L" },
	{ "HP", NULL, "HPOUT1R" },

	{ "SPK", NULL, "SPKOUTLN" },
	{ "SPK", NULL, "SPKOUTLP" },
	{ "SPK", NULL, "SPKOUTRN" },
	{ "SPK", NULL, "SPKOUTRP" },

	{ "IN1LP", NULL, "MICBIAS1" },
	{ "IN1LN", NULL, "MICBIAS1" },
	{ "MICBIAS1", NULL, "Main Mic" },

	{ "IN1RP", NULL, "MICBIAS1" },
	{ "IN1RN", NULL, "MICBIAS1" },
	{ "MICBIAS1", NULL, "Sub Mic" },

	{ "IN2RP:VXRP", NULL, "MICBIAS2" },
	{ "MICBIAS2", NULL, "Headset Mic" },

	{ "AIF1DAC1L", NULL, "S5P RP" },
	{ "AIF1DAC1R", NULL, "S5P RP" },
};

const struct snd_soc_dapm_route manta_paths[] = {
	{ "HP", NULL, "HPOUT1L" },
	{ "HP", NULL, "HPOUT1R" },

	{ "SPK", NULL, "SPKOUTLN" },
	{ "SPK", NULL, "SPKOUTLP" },
	{ "SPK", NULL, "SPKOUTRN" },
	{ "SPK", NULL, "SPKOUTRP" },

	{ "IN1LP", NULL, "MICBIAS1" },
	{ "IN1LN", NULL, "MICBIAS1" },
	{ "MICBIAS1", NULL, "3rd Mic" },

	{ "IN1RP", NULL, "MICBIAS2" },
	{ "IN1RN", NULL, "MICBIAS2" },
	{ "MICBIAS1", NULL, "Headset Mic" },

	{ "IN2LP:VXRN", NULL, "MICBIAS1" },
	{ "MICBIAS2", NULL, "2nd Mic" },

	{ "IN2RP:VXRP", NULL, "MICBIAS1" },
	{ "MICBIAS2", NULL, "Main Mic" },

	{ "AIF1DAC1L", NULL, "S5P RP" },
	{ "AIF1DAC1R", NULL, "S5P RP" },
};

static int manta_start_fll1(struct snd_soc_dai *codec_dai,
						struct manta_wm1811 *machine)
{
	int ret;

	if (machine->pll1_out != machine->prev_pll1_out) {
		/*
		 * FLL1's frequency needs to be changed. Make sure that we
		 * have a system clock not derived from the FLL, since we
		 * cannot change the FLL when the system clock is derived
		 * from it.
		 * Set FFL clock to maximum during transition in case AIF2
		 * is active to ensure SYSCLK > 256 x fs
		 */
		ret = snd_soc_dai_set_sysclk(codec_dai,
		                        WM8994_SYSCLK_MCLK1,
					MCLK1_FREQ / 2, SND_SOC_CLOCK_IN);
		if (ret < 0) {
			dev_err(codec_dai->dev,
				"Failed to switch away from FLL1: %d\n", ret);
			return ret;
		}

		machine->prev_pll1_out = machine->pll1_out;
	}

	/* Switch the FLL */
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL1,
				WM8994_FLL_SRC_MCLK1, MCLK1_FREQ,
				machine->pll1_out);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to start FLL1\n");
		return ret;
	}

	/* Then switch AIF1CLK to it */
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_FLL1,
				machine->pll1_out, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to switch to FLL1\n");
		return ret;
	}

	return 0;
}

static int manta_stop_flls(struct snd_soc_dai *codec_dai,
						struct manta_wm1811 *machine)
{
	int ret;

	/*
	 * Playback/capture has stopped, so switch to the slower
	 * MCLK2 for reduced power consumption. hw_params handles
	 * turning the FLL back on when needed.
	 * Turn FLL2 off as AIF2 is never used if AIF1 is idle. This is
	 * necessary so that SYSCLK can switch to 32kHz clock.
	 */
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL2, 0, 0, 0);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Failed to stop FLL2: %d\n", ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK2,
					MCLK2_FREQ, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Failed to switch away from FLL: %d\n",
									ret);
		return ret;
	}

	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL1,
					0, 0, 0);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Failed to stop FLL1: %d\n", ret);
		return ret;
	}

	return 0;
}

static int manta_set_bias_level(struct snd_soc_card *card,
					struct snd_soc_dapm_context *dapm,
					enum snd_soc_bias_level level)
{
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(card);
	int ret = 0;

	if (dapm->dev != codec_dai->dev)
		return 0;

	if ((level == SND_SOC_BIAS_PREPARE) &&
			(dapm->bias_level == SND_SOC_BIAS_STANDBY))
		ret = manta_start_fll1(codec_dai, machine);

	return ret;
}

static int manta_set_bias_level_post(struct snd_soc_card *card,
					struct snd_soc_dapm_context *dapm,
					enum snd_soc_bias_level level)
{
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(card);
	int ret = 0;

	if (dapm->dev != codec_dai->dev)
		return 0;

	if (level == SND_SOC_BIAS_STANDBY)
		ret = manta_stop_flls(codec_dai, machine);

	dapm->bias_level = level;

	return ret;
}

static int manta_wm1811_aif1_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(rtd->codec->card);
	int ret;

	machine->pll1_out = params_rate(params) * 512;

	ret = manta_start_fll1(codec_dai, machine);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to start FLL1\n");
		return ret;
	}

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
					SND_SOC_DAIFMT_NB_NF |
					SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to set codec DAIFMT\n");
		return ret;
	}

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
					SND_SOC_DAIFMT_NB_NF |
					SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to set CPU DAIFMT\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops manta_wm1811_aif1_ops = {
	.hw_params = manta_wm1811_aif1_hw_params,
};

static int manta_wm1811_aif2_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct manta_wm1811 *machine =
	        snd_soc_card_get_drvdata(rtd->codec->card);
	int ret;
	int prate;

	prate = params_rate(params);

	switch (prate) {
	case 8000:
	case 16000:
		break;
	default:
		return -EINVAL;
	}

	/* Use 512 multiplier to make sure that SYSCLK > 4096kHz
	 * when fs is 8kHz */
	machine->pll2_out = prate * 512;

	if (machine->pll2_out != machine->prev_pll2_out) {
		/*
		 * FLL2's frequency needs to be changed. Make sure that we
		 * have a system clock not derived from the FLL, since we
		 * cannot change the FLL when the system clock is derived
		 * from it.
		 * Set FFL clock to maximum during transition in case AIF1
		 * is active to ensure SYSCLK > 256 x fs
		 */
		ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1,
		                             MCLK1_FREQ / 2, SND_SOC_CLOCK_IN);
		if (ret < 0) {
			dev_err(codec_dai->dev,
			        "Failed to switch away from FLL2: %d\n", ret);
			return ret;
		}

		machine->prev_pll2_out = machine->pll2_out;
	}

	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL2, WM8994_FLL_SRC_MCLK1,
						MCLK1_FREQ, machine->pll2_out);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to configure FLL2: %d\n", ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_FLL2,
	                             machine->pll2_out, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Unable to switch to FLL2: %d\n", ret);
		return ret;
	}

	/* Set the codec DAI configuration */
	ret = snd_soc_dai_set_fmt(
	                codec_dai,
	                SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
	                                | SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev, "%s snd_soc_dai_set_fmt error %d\n",
		        __func__, ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops manta_wm1811_aif2_ops = {
	.hw_params = manta_wm1811_aif2_hw_params,
};

static struct snd_soc_dai_link manta_dai[] = {
	{
		.name = "media-pri",
		.stream_name = "Media primary",
		.cpu_dai_name = "samsung-i2s.0",
		.codec_dai_name = "wm8994-aif1",
		.platform_name = "samsung-audio",
		.codec_name = "wm8994-codec",
		.ops = &manta_wm1811_aif1_ops,
	},
	{
		.name = "media-sec",
		.stream_name = "Media secondary",
		.cpu_dai_name = "samsung-i2s.4",
		.codec_dai_name = "wm8994-aif1",
#ifdef CONFIG_SND_SAMSUNG_USE_IDMA
		.platform_name = "samsung-idma",
#else
		.platform_name = "samsung-audio",
#endif
		.codec_name = "wm8994-codec",
		.ops = &manta_wm1811_aif1_ops,
	},
	{
		.name = "voice",
		.stream_name = "Voice",
		.cpu_dai_name = "manta-voice",
		.codec_dai_name = "wm8994-aif2",
		.platform_name = "snd-soc-dummy",
		.codec_name = "wm8994-codec",
		.ops = &manta_wm1811_aif2_ops,
		.ignore_suspend = 1,
	},
	{
		.name = "bt",
		.stream_name = "Bluetooth",
		.cpu_dai_name = "manta-bt",
		.codec_dai_name = "wm8994-aif3",
		.platform_name = "snd-soc-dummy",
		.codec_name = "wm8994-codec",
		.ignore_suspend = 1,
	},
};

static struct snd_soc_dai_driver manta_ext_dai[] = {
	{
		.name = "manta-voice",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
	{
		.name = "manta-bt",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
};

static struct platform_device android_device_ear = {
	.name = "android-ear",
	.id = -1,
};

static void manta_mic_id(void *data, u16 status)
{
	struct snd_soc_codec *codec = data;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(codec->card);
	int sum = -1;
	int count = 0;
	int ret;
	int i;

	status = STATUS_HEADPHONE_3POLE;
	if (machine->adc_client) {
		for (i = 0; i < ADC_MIC_TEST_NUM; i++) {
			usleep_range(ADC_MIC_WAIT_US, ADC_MIC_WAIT_US);
			ret = s3c_adc_read(machine->adc_client,
					   EAR_ADC_CHANNEL);
			if (ret >= 0) {
				sum += ret;
				count++;
			}
		}
		if (count > 0)
			sum = (sum + 1) / count;
	}
	if (sum < 0)
		pr_err("Error reading ADC line\n");
	else if (sum > ADC_HEADPHONE_3POLE && sum <= ADC_HEADSET_4POLE)
		status = STATUS_HEADSET_4POLE;

	wm8958_mic_id(data, status);
}

static int manta_late_probe(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = card->rtd[0].codec;
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct snd_soc_dai *cpu_dai = card->rtd[0].cpu_dai;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(codec->card);
	struct wm8994_priv *wm8994 = snd_soc_codec_get_drvdata(codec);
	int ret;

	/*
	 * Hack: permit the codec to open streams with the same number
	 * of channels that the CPU DAI (samsung-i2s) supports, since
	 * the HDMI block takes its audio from the i2s0 channel shared
	 * with the codec.
	 */
	codec_dai->driver->playback.channels_max =
			cpu_dai->driver->playback.channels_max;

	/*
	 * Hack: For using DCS cache from wm1811
	 * because current wm1811 driver does not use cached value
	 * and it increases audio warmup time for headphone routing.
	 * it can help decreasing warmup time
	 */
	wm8994->hubs.no_cache_dac_hp_direct = false;

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK2,
				MCLK2_FREQ, SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(codec->dev, "Unable to switch to MCLK2\n");

	/* Force AIF1CLK on as it will be master for jack detection */
	ret = snd_soc_dapm_force_enable_pin(&codec->dapm, "AIF1CLK");
	if (ret < 0)
		dev_err(codec->dev, "Failed to enable AIF1CLK\n");

	ret = snd_soc_dapm_disable_pin(&codec->dapm, "S5P RP");
	if (ret < 0)
		dev_err(codec->dev, "Failed to disable S5P RP\n");

	ret = snd_soc_jack_new(codec, "Headset",
				SND_JACK_HEADSET | SND_JACK_MECHANICAL |
				SND_JACK_BTN_0 | SND_JACK_BTN_1 |
				SND_JACK_BTN_2, &machine->jack);
	if (ret) {
		dev_err(codec->dev, "Failed to create jack: %d\n", ret);
		return ret;
	}

	/*
	 * Settings provided by Wolfson for Samsung-specific customization
	 * of MICBIAS levels
	 */
	snd_soc_write(codec, 0x102, 0x3);
	snd_soc_write(codec, 0xcb, 0x5151);
	snd_soc_write(codec, 0xd3, 0x3f3f);
	snd_soc_write(codec, 0xd4, 0x3f3f);
	snd_soc_write(codec, 0xd5, 0x3f3f);
	snd_soc_write(codec, 0xd6, 0x3226);
	snd_soc_write(codec, 0x102, 0x0);
	snd_soc_write(codec, 0xd1, 0x87);
	snd_soc_write(codec, 0x3b, 0x9);
	snd_soc_write(codec, 0x3c, 0x2);

	ret = snd_jack_set_key(machine->jack.jack, SND_JACK_BTN_0,
							KEY_MEDIA);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set KEY_MEDIA: %d\n", ret);

	ret = snd_jack_set_key(machine->jack.jack, SND_JACK_BTN_1,
							KEY_VOLUMEUP);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set KEY_VOLUMEUP: %d\n", ret);

	ret = snd_jack_set_key(machine->jack.jack, SND_JACK_BTN_2,
							KEY_VOLUMEDOWN);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set KEY_VOLUMEDOWN: %d\n", ret);

	/* certain manta revisions must use SoC ADC mic detection */
	if (exynos5_manta_get_revision() >= MANTA_REV_DOGFOOD05) {
		machine->adc_client =
			s3c_adc_register(&android_device_ear, NULL, NULL, 0);
		if (IS_ERR(machine->adc_client)) {
			dev_err(codec->dev, "Failed to set ADC client: %ld\n",
				PTR_ERR(machine->adc_client));
			machine->adc_client = NULL;
		}
		wm8958_mic_detect(codec, &machine->jack,
					NULL, NULL, manta_mic_id, codec);
	} else {
		wm8958_mic_detect(codec, &machine->jack,
					NULL, NULL, NULL, NULL);
	}

	return 0;
}

static int manta_card_suspend_post(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = card->rtd->codec;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(codec->card);

	snd_soc_update_bits(codec, WM8994_AIF1_MASTER_SLAVE,
					WM8994_AIF1_TRI_MASK, WM8994_AIF1_TRI);

	clk_disable(machine->clk);

	return 0;
}

static int manta_card_resume_pre(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = card->rtd->codec;
	struct manta_wm1811 *machine =
				snd_soc_card_get_drvdata(codec->card);

	clk_enable(machine->clk);

	snd_soc_update_bits(codec, WM8994_AIF1_MASTER_SLAVE,
					WM8994_AIF1_TRI_MASK, 0);

	return 0;
}

static struct snd_soc_card manta = {
	.name = "Manta-I2S",
	.owner = THIS_MODULE,
	.dai_link = manta_dai,
	.num_links = ARRAY_SIZE(manta_dai),

	.set_bias_level = manta_set_bias_level,
	.set_bias_level_post = manta_set_bias_level_post,

	.controls = manta_controls,
	.num_controls = ARRAY_SIZE(manta_controls),
	.dapm_widgets = manta_widgets,
	.num_dapm_widgets = ARRAY_SIZE(manta_widgets),
	.dapm_routes = manta_paths,
	.num_dapm_routes = ARRAY_SIZE(manta_paths),

	.late_probe = manta_late_probe,

	.suspend_post = manta_card_suspend_post,
	.resume_pre = manta_card_resume_pre,
};

static int __devinit snd_manta_probe(struct platform_device *pdev)
{
	struct manta_wm1811 *machine;
	int ret;
	int hwrev = exynos5_manta_get_revision();

	machine = kzalloc(sizeof(*machine), GFP_KERNEL);
	if (!machine) {
		pr_err("Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_kzalloc;
	}

	machine->clk = clk_get(&pdev->dev, "system_clk");
	if (IS_ERR(machine->clk)) {
		pr_err("failed to get system_clk\n");
		ret = PTR_ERR(machine->clk);
		goto err_clk_get;
	}

	/* Start the reference clock for the codec's FLL */
	clk_enable(machine->clk);

	machine->pll1_out = 44100 * 512; /* default sample rate */
	machine->pll2_out = 0;

	ret = snd_soc_register_dais(&pdev->dev, manta_ext_dai,
						ARRAY_SIZE(manta_ext_dai));
	if (ret != 0)
		pr_err("Failed to register external DAIs: %d\n", ret);

	snd_soc_card_set_drvdata(&manta, machine);

	if (hwrev < MANTA_REV_PRE_ALPHA) {
		manta.dapm_widgets = manta_widgets_lunchbox,
		manta.num_dapm_widgets = ARRAY_SIZE(manta_widgets_lunchbox),
		manta.dapm_routes = manta_paths_lunchbox;
		manta.num_dapm_routes = ARRAY_SIZE(manta_paths_lunchbox);
	}

	manta.dev = &pdev->dev;
	ret = snd_soc_register_card(&manta);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
		goto err_register_card;
	}

	return 0;

err_register_card:
	clk_put(machine->clk);
err_clk_get:
	kfree(machine);
err_kzalloc:
	return ret;
}

static int __devexit snd_manta_remove(struct platform_device *pdev)
{
	struct manta_wm1811 *machine = snd_soc_card_get_drvdata(&manta);

	if (machine->adc_client)
		s3c_adc_release(machine->adc_client);
	snd_soc_unregister_card(&manta);
	clk_disable(machine->clk);
	clk_put(machine->clk);
	kfree(machine);

	return 0;
}

static struct platform_driver snd_manta_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "manta-i2s",
		.pm = &snd_soc_pm_ops,
	},
	.probe = snd_manta_probe,
	.remove = __devexit_p(snd_manta_remove),
};

module_platform_driver(snd_manta_driver);

MODULE_DESCRIPTION("ALSA SoC Manta WM1811");
MODULE_LICENSE("GPL");
