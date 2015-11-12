/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dai.h>
#include <sound/jack.h>
#include <linux/gpio.h>
#include <mach/platform.h>

#include "../codecs/wm8903.h"
#include "nxp-i2s.h"

///*
#define	pr_debug	printk
//*/


static struct snd_soc_codec *wm8903 = NULL;
static int codec_bias_level = 0;

static int wm8903_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
#if 0
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int freq = params_rate(params) * 256;	/* 48K * 256 = 12.288 Mhz */
	unsigned int fmt  = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
						SND_SOC_DAIFMT_CBS_CFS;
	int ret = 0;

	pr_debug("%s\n", __func__);
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, freq, SND_SOC_CLOCK_IN);
	if (0 > ret)
		return ret;

	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (0 > ret)
		return ret;
	return ret;
#endif
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_card *card = codec->card;

	int srate, mclk;
	int err;

	srate = params_rate(params);
	switch (srate) {
	case 64000:
	case 88200:
	case 96000:
		mclk = 128 * srate;
		break;
	default:
		mclk = 256 * srate;
		break;
	}
	/* FIXME: Codec only requires >= 3MHz if OSR==0 */
	while (mclk < 6000000)
		mclk *= 2;

	err = snd_soc_dai_set_fmt(codec_dai,
					SND_SOC_DAIFMT_I2S |
					SND_SOC_DAIFMT_NB_NF |
					SND_SOC_DAIFMT_CBS_CFS);
	if (err < 0) {
		dev_err(card->dev, "codec_dai fmt not set\n");
		return err;
	}
/*
	err = snd_soc_dai_set_fmt(cpu_dai,
					SND_SOC_DAIFMT_I2S |
					SND_SOC_DAIFMT_NB_NF |
					SND_SOC_DAIFMT_CBS_CFS);
	if (err < 0) {
		dev_err(card->dev, "cpu_dai fmt not set\n");
		return err;
	}
*/

	pr_debug("%s mclk %d\n", __func__,mclk);

	err = snd_soc_dai_set_sysclk(codec_dai, 0, mclk,
					SND_SOC_CLOCK_IN);
	if (err < 0) {
		dev_err(card->dev, "codec_dai clock not set\n");
		return err;
	}

	return 0;
}

static int wm8903_suspend_pre(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = wm8903;
	pr_debug("%s\n", __func__);
#if 0
	snd_soc_update_bits(codec, WM8903_LOUT1_HP_CONTROL, 0x40, 0x40);
	snd_soc_update_bits(codec, WM8903_ROUT1_HP_CONTROL, 0x40, 0x40);
#endif
	return 0;
}

static int wm8903_resume_pre(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = wm8903;
	pr_debug("%s\n", __func__);
	PM_DBGOUT("%s BAIAS=%d\n", __func__, codec->dapm.bias_level);
	codec_bias_level = codec->dapm.bias_level;
	return 0;
}

static int wm8903_resume_post(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = wm8903;
	pr_debug("%s\n", __func__);
	PM_DBGOUT("%s BAIAS=%d\n", __func__, codec->dapm.bias_level);

	if (SND_SOC_BIAS_OFF != codec_bias_level)
		codec->driver->resume(codec);

	return 0;
}


static int wm8903_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;

	int ret;

	wm8903 = codec;
	return 0;
}

static struct snd_soc_ops wm8903_ops = {
	.hw_params 	= wm8903_hw_params,
};

static char str_dai_name[16] = DEV_NAME_I2S;

static struct snd_soc_dai_link wm8903_dai_link = {
	.name 			= "ASOC-WM8903",
	.stream_name 	= "wm8903 HiFi",
	.cpu_dai_name 	= str_dai_name,			/* nxp_snd_i2s_driver name */
	.platform_name  = DEV_NAME_PCM,			/* nxp_snd_pcm_driver name */
	.codec_dai_name = "wm8903-hifi",		/* wm8903_dai's name */
	.codec_name 	= "wm8903.1-001a",		/* wm8903_i2c_driver name + '.' + bus + '-' + address(7bit) */
	.ops 			= &wm8903_ops,
	.symmetric_rates = 1,
	.init			= wm8903_dai_init,
	.ops 			= &wm8903_ops,
};

static struct snd_soc_card wm8903_card = {
	.name 			= "I2S-WM8903",		/* proc/asound/cards */
	.owner 			= THIS_MODULE,
	.dai_link 		= &wm8903_dai_link,
	.num_links 		= 1,
	.suspend_pre	= &wm8903_suspend_pre,
	.resume_pre		= &wm8903_resume_pre,
	.resume_post	= &wm8903_resume_post,
};

/*
 * codec driver
 */
static int wm8903_probe(struct platform_device *pdev)
{
	struct nxp_snd_dai_plat_data *plat = pdev->dev.platform_data;
	struct snd_soc_card *card = &wm8903_card;
	unsigned int rates = 0, format = 0;
	int ret;

	if (plat) {
		rates = plat->sample_rate;
		format = plat->pcm_format;
		sprintf(str_dai_name, "%s.%d", DEV_NAME_I2S, plat->i2s_ch);	// set I2S name
	}

	/*
	 * register card
	 */
	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
		return ret;
	}

	if (card->rtd->cpu_dai) {
		struct snd_soc_dai *cpu_dai = card->rtd->cpu_dai;
		struct snd_soc_dai_driver *i2s = cpu_dai->driver;
		if (rates) {
			rates = snd_pcm_rate_to_rate_bit(rates);
			if (SNDRV_PCM_RATE_KNOT == rates)
				printk("%s, invalid sample rates=%d\n", __func__, plat->sample_rate);
			else {
				i2s->playback.rates = rates;
				i2s->capture.rates = rates;
			}
		}
		if (format) {
			i2s->playback.formats = format;
			i2s->capture.formats = format;
		}
	}

	pr_debug("wm8903-dai: register card %s -> %s\n",
		card->dai_link->codec_dai_name, card->dai_link->cpu_dai_name);



	return 0;
}

static int wm8903_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	pr_debug("%s\n", __func__);
	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver wm8903_driver = {
	.driver		= {
		.name	= "wm8903-audio",
		.owner	= THIS_MODULE,
		.pm 	= &snd_soc_pm_ops,	/* for suspend */
	},
	.probe		= wm8903_probe,
	.remove		= __devexit_p(wm8903_remove),
};
module_platform_driver(wm8903_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound codec-wm8903 driver for the SLSI");
MODULE_LICENSE("GPL");
