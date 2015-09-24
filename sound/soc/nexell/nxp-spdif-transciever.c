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
#include <mach/platform.h>
#include "nxp-spdif.h"
#include "nxp-pcm.h"

/*
#define pr_debug(msg...)		printk(KERN_INFO msg)
*/

static struct snd_soc_dai_link spdiftx_dai_link = {
	.name 			= "SPDIF Transciever",
	.stream_name 	= "SPDIF PCM Playback",
	.cpu_dai_name 	= DEV_NAME_SPDIF_TX,	/* spdif_driver name */
	.platform_name  = DEV_NAME_PCM,			/* nxp_snd_pcm_driver name */
	.codec_dai_name = "dit-hifi",			/* spidf_transciever.c */
	.codec_name 	= "spdif-dit",			/* spidf_transciever.c */
	.symmetric_rates = 1,
};

static struct snd_soc_card spdiftx_card = {
	.name 			= "SPDIF-Transciever",	/* proc/asound/cards */
	.dai_link 		= &spdiftx_dai_link,
	.num_links 		= 1,
};

/*
 * codec driver
 */
static int spdiftx_probe(struct platform_device *pdev)
{
	struct nxp_snd_dai_plat_data *plat = pdev->dev.platform_data;
	struct snd_soc_card *card = &spdiftx_card;
	struct snd_soc_dai *codec_dai = NULL;
	struct snd_soc_dai_driver *driver = NULL;
	unsigned int sample_rate = 0;
	int ret;

	/* register card */
	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
		return ret;
	}

	if (card->rtd) {
		codec_dai = card->rtd->codec_dai;
		if (codec_dai)
			driver = codec_dai->driver;
	}

	/* Reset spdif sample rates and format */
	if (plat && driver) {
		if (plat->sample_rate) {
			sample_rate = snd_pcm_rate_to_rate_bit(plat->sample_rate);
			if (SNDRV_PCM_RATE_KNOT != sample_rate)
				driver->playback.rates = sample_rate;
			else
				printk("%s, invalid sample rates=%d\n", __func__, plat->sample_rate);
		}

		if(plat->pcm_format)
			driver->playback.formats = plat->pcm_format;
	}

	pr_debug("spdif-rx-dai: register card %s -> %s\n",
		card->dai_link->codec_dai_name, card->dai_link->cpu_dai_name);
	return ret;
}

static int spdiftx_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver spdiftx_driver = {
	.driver		= {
		.name	= "spdif-transciever",
		.owner	= THIS_MODULE,
		.pm 	= &snd_soc_pm_ops,	/* for suspend */
	},
	.probe		= spdiftx_probe,
	.remove		= spdiftx_remove,
};
module_platform_driver(spdiftx_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound SPDIF transciever driver for the SLSI");
MODULE_LICENSE("GPL");
