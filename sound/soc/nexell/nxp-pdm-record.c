/*
 * (C) Copyright 2015
 * hyun seok jung, Nexell Co, <hsjung@nexell.co.kr>
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
#include "nxp-pdm.h"
#include "nxp-pcm.h"

/*
#define pr_debug(msg...)		printk(KERN_INFO msg)
*/

/*
 * PDM Record
 */
#define STUB_RATES		SNDRV_PCM_RATE_48000
#define STUB_FORMATS	SND_SOC_PDM_FORMATS

static struct snd_soc_codec_driver soc_codec_pdm_rec;

static struct snd_soc_dai_driver rec_dai = {
	.name		= "dit-mulch-recorder",
	.capture 	= {
	.stream_name	= "Capture",
	.channels_min	= 1,
	.channels_max	= 4,
	.rates			= STUB_RATES,
	.formats		= STUB_FORMATS,
	},
};

static int pdm_rec_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_pdm_rec,
			&rec_dai, 1);
}

static int pdm_rec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

/*
 * PDM Record
 */
static struct platform_driver pdm_rec = {
	.probe		= pdm_rec_probe,
	.remove		= pdm_rec_remove,
	.driver		= {
		.name	= "pdm-dit-recorder",
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(pdm_rec);

MODULE_AUTHOR("hsjung <hsjung@nexell.co.kr>");
MODULE_DESCRIPTION("PDM dummy recorder driver");
MODULE_LICENSE("GPL");

/*
 * PDM CARD DAI
 */
static struct snd_soc_dai_link pdmrec_dai_link = {
	.name 			= "PDM Rec",
	.stream_name 	= "PDM PCM Capture",
	.cpu_dai_name 	= DEV_NAME_PDM,			/* pdm_driver name */
	.platform_name  = DEV_NAME_PCM,			/* nxp_snd_pcm_driver name */
	.codec_dai_name = "dit-mulch-recorder",
	.codec_name 	= "pdm-dit-recorder",
	.symmetric_rates = 1,
};

static struct snd_soc_card pdmrec_card = {
	.name 			= "PDM-Recorder",		/* proc/asound/cards */
	.dai_link 		= &pdmrec_dai_link,
	.num_links 		= 1,
};

static int pdmrec_probe(struct platform_device *pdev)
{
	struct nxp_snd_dai_plat_data *plat = pdev->dev.platform_data;
	struct snd_soc_card *card = &pdmrec_card;
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

	/* Reset pdmrec sample rates and format */
	if (plat && driver) {
		if (plat->sample_rate) {
			sample_rate = snd_pcm_rate_to_rate_bit(plat->sample_rate);
			if (SNDRV_PCM_RATE_KNOT != sample_rate)
				driver->capture.rates = sample_rate;
			else
				printk("%s, invalid sample rates=%d\n", __func__, plat->sample_rate);
		}

		if(plat->pcm_format)
			driver->capture.formats = plat->pcm_format;
	}

	pr_debug("pdm-rec-dai: register card %s -> %s\n",
		card->dai_link->codec_dai_name, card->dai_link->cpu_dai_name);
	return ret;
}

static int pdmrec_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver pdmrec_driver = {
	.driver		= {
		.name	= "pdm-recorder",
		.owner	= THIS_MODULE,
		.pm 	= &snd_soc_pm_ops,	/* for suspend */
	},
	.probe		= pdmrec_probe,
	.remove		= pdmrec_remove,
};
module_platform_driver(pdmrec_driver);

MODULE_AUTHOR("hsjung <hsjung@nexell.co.kr>");
MODULE_DESCRIPTION("Sound PDM recorder driver for the SLSI");
MODULE_LICENSE("GPL");

