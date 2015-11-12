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
#include "nxp-i2s.h"


static char str_dai_name[16] = DEV_NAME_I2S;
static int (*cpu_resume_fn)(struct snd_soc_dai *dai) = NULL;

static int cs42l51_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int freq = params_rate(params) * 256;	/* 48K * 256 = 12.288 Mhz */
	unsigned int fmt  = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
						SND_SOC_DAIFMT_CBS_CFS;
	int ret = 0;

	printk("%s: freq:%d, fmt:0x%x\n", __func__, freq, fmt);

	ret = snd_soc_dai_set_sysclk(codec_dai, 0, freq, SND_SOC_CLOCK_IN);
	if (0 > ret)
		return ret;

	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (0 > ret)
		return ret;
	return ret;
}

static int cs42l51_resume_pre(struct snd_soc_card *card)
{
	struct snd_soc_dai *cpu_dai = card->rtd->cpu_dai;
	int ret = 0;
	PM_DBGOUT("+%s\n", __func__);

	/*
	 * first execute cpu(i2s) resume and execute codec resume.
	 */
	if (cpu_dai->driver->resume && ! cpu_resume_fn) {
		cpu_resume_fn  = cpu_dai->driver->resume;
		cpu_dai->driver->resume = NULL;
	}

	if (cpu_resume_fn)
		ret = cpu_resume_fn(cpu_dai);

	PM_DBGOUT("-%s\n", __func__);
	return ret;
}

static struct snd_soc_ops cs42l51_ops = {
	.hw_params 		= cs42l51_hw_params,
};

static int cs42l51_spk_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *k, int event)
{
	pr_debug("%s:%d (event:%d)\n", __func__, __LINE__, event);
	return 0;
}

/* cs42l51 machine dapm widgets */
static const struct snd_soc_dapm_widget cs42l51_dapm_widgets[] = {
	SND_SOC_DAPM_SPK("Ext Spk", cs42l51_spk_event),
	//SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

/* Corgi machine audio map (connections to the codec pins) */
static const struct snd_soc_dapm_route cs42l51_audio_map[] = {
	/* headphone connected to HPOL, HPOR */
	//{"Headphone Jack", NULL, "HPOL"},
	//{"Headphone Jack", NULL, "HPOR"},

	/* speaker connected to SPOL, SPOR */
	{"Ext Spk", NULL, "SPOR"},
	{"Ext Spk", NULL, "SPOL"},
};

/* Headphones jack detection DAPM pin */
static struct snd_soc_jack_pin jack_pins[] = {
	//{
	//	.pin	= "Headphone Jack",
	//	.mask	= SND_JACK_HEADPHONE,
	//},
	{
		.pin	= "Ext Spk",
		.mask   = SND_JACK_HEADPHONE,
		.invert	= 1,				// when insert disalbe
	},
};

/* Headphones jack detection GPIO */
static struct snd_soc_jack_gpio jack_gpio = {
	.invert		= false,			// High detect : invert = false
	.name		= "hp-gpio",
	.report		= SND_JACK_HEADPHONE,
	.debounce_time	= 200,
};

static struct snd_soc_jack hp_jack;

static int cs42l51_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	struct snd_soc_jack_gpio *jack = &jack_gpio;
	int ret;

	/* set endpoints to not connected */
	//snd_soc_dapm_nc_pin(dapm, "AUXO1");
	//snd_soc_dapm_nc_pin(dapm, "AUXO2");
	snd_soc_dapm_nc_pin(dapm, "MONO");

	if (NULL == jack->name)
		return 0;

	/* Headset jack detection */
	ret = snd_soc_jack_new(codec, "Headphone Jack",
				SND_JACK_HEADPHONE, &hp_jack);
	if (ret)
		return ret;

	ret = snd_soc_jack_add_pins(&hp_jack, ARRAY_SIZE(jack_pins), jack_pins);
	if (ret)
		return ret;

	/* to power up cs42l51 (HP Depop: hp_event) */
	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
	snd_soc_dapm_sync(dapm);

	ret = snd_soc_jack_add_gpios(&hp_jack, 1, jack);
	if (ret)
		printk("Fail, register audio jack detect, io [%d]...\n", jack->gpio);

	return 0;
}

static struct snd_soc_dai_link cs42l51_dai_link = {
	.name 			= "ASOC-CS42L51",
	.stream_name 	= "cs42l51 HiFi",
	.cpu_dai_name 	= str_dai_name,			/* nxp_snd_i2s_driver name */
	.platform_name  = DEV_NAME_PCM,			/* nxp_snd_pcm_driver name */
	.codec_dai_name = "cs42l51-hifi",		/* cs42l51_dai's name */
	.codec_name 	= "cs42l51.0-004a",		/* cs42l51_i2c_driver name + '.' + bus + '-' + address(7bit) */
	.ops 			= &cs42l51_ops,
	.symmetric_rates = 1,
	.init		= cs42l51_dai_init,
};

static struct snd_soc_card cs42l51_card = {
	.name 				= "I2S-CS42L51",		/* proc/asound/cards */
	.owner 				= THIS_MODULE,
	.dai_link 			= &cs42l51_dai_link,
	.num_links 			= 1,
	.resume_pre			= &cs42l51_resume_pre,
	.dapm_widgets 		= cs42l51_dapm_widgets,
	.num_dapm_widgets 	= ARRAY_SIZE(cs42l51_dapm_widgets),
	.dapm_routes 		= cs42l51_audio_map,
	.num_dapm_routes 	= ARRAY_SIZE(cs42l51_audio_map),
};

/*
 * codec driver
 */
static int cs42l51_probe(struct platform_device *pdev)
{
	struct nxp_snd_dai_plat_data *plat = pdev->dev.platform_data;
	struct snd_soc_card *card = &cs42l51_card;
	struct snd_soc_jack_gpio *jack = &jack_gpio;
	struct snd_soc_dai_driver *i2s_dai = NULL;
	struct nxp_snd_jack_pin *hpin = NULL;
	unsigned int rates = 0, format = 0;
	int ret;

	/* set I2S name */
	if (plat)
		sprintf(str_dai_name, "%s.%d", DEV_NAME_I2S, plat->i2s_ch);

	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
		return ret;
	}

	if (plat) {
		rates = plat->sample_rate;
		format = plat->pcm_format;
		hpin = &plat->hp_jack;
		if (hpin->support) {
			jack->gpio = hpin->detect_io;
			jack->invert = hpin->detect_level ?  false : true;
			jack->debounce_time = hpin->debounce_time ?
					hpin->debounce_time : 200;
		} else {
			jack->name = NULL;
		}
	}

	if (card->rtd) {
		struct snd_soc_dai *cpu_dai = card->rtd->cpu_dai;
		if (cpu_dai)
			i2s_dai = cpu_dai->driver;
	}
	pr_debug("cs42l51-dai: register card %s -> %s\n",
		card->dai_link->codec_dai_name, card->dai_link->cpu_dai_name);

	if (NULL == i2s_dai)
		return 0;

	/*
	 * Reset i2s sample rates
	 */
	if (rates) {
		rates = snd_pcm_rate_to_rate_bit(rates);
		if (SNDRV_PCM_RATE_KNOT == rates)
			printk("%s, invalid sample rates=%d\n", __func__, plat->sample_rate);
		else {
			i2s_dai->playback.rates = rates;
			i2s_dai->capture.rates = rates;
		}
	}

	/*
	 * Reset i2s format
	 */
	if (format) {
		i2s_dai->playback.formats = format;
		i2s_dai->capture.formats = format;
	}

	return ret;
}

static int cs42l51_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver cs42l51_driver = {
	.driver		= {
		.name	= "cs42l51-audio",
		.owner	= THIS_MODULE,
		.pm 	= &snd_soc_pm_ops,	/* for suspend */
	},
	.probe		= cs42l51_probe,
	.remove		= __devexit_p(cs42l51_remove),
};
module_platform_driver(cs42l51_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound codec-cs42l51 driver for the SLSI");
MODULE_LICENSE("GPL");
