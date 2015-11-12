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
#include <mach/platform.h>
#include "nxp-i2s.h"

/*
#define pr_debug				printk
*/
static char str_dai_name[16] = DEV_NAME_I2S;

#define STUB_RATES		SNDRV_PCM_RATE_8000_192000
#define STUB_FORMATS	(SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_codec_driver soc_codec_snd_null;

static struct snd_soc_dai_driver null_stub_dai = {
	.name		= "snd-null-voice",
	.playback 	= {
		.stream_name	= "Null Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates			= STUB_RATES,
		.formats		= STUB_FORMATS,
	},
	.capture 	= {
		.stream_name	= "Null Capture",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates			= STUB_RATES,
		.formats		= STUB_FORMATS,
	},
};

static int snd_null_probe(struct platform_device *pdev)
{
	int ret = snd_soc_register_codec(&pdev->dev, &soc_codec_snd_null,
			&null_stub_dai, 1);
	if(ret < 0)
		printk("snd null codec driver register fail.(ret=%d)\n", ret);
	return ret;
}

static int snd_null_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

/*
 * SND-NULL Receiver
 */
static struct platform_driver snd_null_driver = {
	.probe		= snd_null_probe,
	.remove		= snd_null_remove,
	.driver		= {
		.name	= "snd-null",
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(snd_null_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("ASoc NULL driver");
MODULE_LICENSE("GPL");

/*
 * SND-NULL Card DAI
 */
#if defined(CONFIG_SND_NXP_DFS)
static int snd_null_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    unsigned int sample_rate = params_rate(params);
//	unsigned int freq = params_rate(params) * 256;	/* 48K * 256 = 12.288 Mhz */
//	unsigned int fmt  = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
//						SND_SOC_DAIFMT_CBS_CFS;
	int ret = 0;

	pr_debug("%s\n", __func__);

    ret = snd_soc_dai_set_sysclk(cpu_dai, 0, sample_rate, SND_SOC_CLOCK_IN);
	if (0 > ret)
		return ret;
/*
    ret = snd_soc_dai_set_fmt(cpu_dai, fmt);
    if (0 > ret)
       return ret;
*/
	return ret;
}

static struct snd_soc_ops snd_null_ops = {
	.hw_params		= snd_null_hw_params,
};
#endif
static struct snd_soc_dai_link snd_null_dai_link = {
	.name 			= "ASoc-NULL",
	.stream_name 	= "Null Voice",
	.cpu_dai_name 	= str_dai_name,
	.platform_name  = DEV_NAME_PCM,			/* nxp_snd_pcm_driver name */
	.codec_dai_name = "snd-null-voice",
	.codec_name 	= "snd-null",
#if defined(CONFIG_SND_NXP_DFS)
	.ops			= &snd_null_ops,
#endif
	.symmetric_rates = 1,
};

static struct snd_soc_card snd_null_card[] = {
	{
	.name 			= "SND-NULL.0",	/* proc/asound/cards */
	.dai_link 		= &snd_null_dai_link,
	.num_links 		= 1,
	},
	{
	.name 			= "SND-NULL.1",	/* proc/asound/cards */
	.dai_link 		= &snd_null_dai_link,
	.num_links 		= 1,
	},
	{
	.name 			= "SND-NULL.2",	/* proc/asound/cards */
	.dai_link 		= &snd_null_dai_link,
	.num_links 		= 1,
	},
};

static int snd_card_probe(struct platform_device *pdev)
{
	struct nxp_snd_dai_plat_data *plat = pdev->dev.platform_data;
	struct snd_soc_card *card = &snd_null_card[0];
	struct snd_soc_dai_driver *cpudrv = NULL;
	unsigned int rates = 0, format = 0;
	int ret;

	/* set I2S name */
	if (plat)
		sprintf(str_dai_name, "%s.%d", DEV_NAME_I2S, plat->i2s_ch);

	if (pdev->id != -1)
		card = &snd_null_card[pdev->id];

	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
		return ret;
	}

	if (plat) {
		rates = plat->sample_rate;
		format = plat->pcm_format;
	}

	if (card->rtd) {
		struct snd_soc_dai *cpu_dai = card->rtd->cpu_dai;
		if (cpu_dai)
			cpudrv = cpu_dai->driver;
	}
	pr_debug("snd-null-dai: register card %s -> %s\n",
		card->dai_link->codec_dai_name, card->dai_link->cpu_dai_name);

	if (NULL == cpudrv)
		return 0;

	/*
	 * Reset i2s sample rates
	 */
	if (rates) {
		rates = snd_pcm_rate_to_rate_bit(rates);
		if (SNDRV_PCM_RATE_KNOT == rates)
			printk("%s, invalid sample rates=%d\n", __func__, plat->sample_rate);
		else {
			cpudrv->playback.rates = rates;
			cpudrv->capture.rates = rates;
		}
	}

	/*
	 * Reset i2s format
	 */
	if (format) {
		cpudrv->playback.formats = format;
		cpudrv->capture.formats = format;
	}

	return ret;
}

static int snd_card_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver snd_card_driver = {
	.driver		= {
		.name	= "snd-null-card",
		.owner	= THIS_MODULE,
		.pm 	= &snd_soc_pm_ops,	/* for suspend */
	},
	.probe		= snd_card_probe,
	.remove		= snd_card_remove,
};
module_platform_driver(snd_card_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound codec-null driver for the SLSI");
MODULE_LICENSE("GPL");

