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

#include "../codecs/wm8978.h"
#include "nxp-i2s.h"

/*
#define	pr_debug	printk
*/

#define	AUDIO_AMP_POWER		CFG_IO_AUDIO_AMP_POWER

static struct snd_soc_jack_gpio jack_gpio;
static struct snd_soc_codec *wm8976 = NULL;
static int codec_bias_level = 0;

static int wm8976_jack_status_check(void)
{
	struct snd_soc_codec *codec = wm8976;
	int jack = jack_gpio.gpio;
	int invert = jack_gpio.invert;
	int level = gpio_get_value_cansleep(jack);

	if (!codec)
		return -1;

	if(invert)
		level = !level;

	pr_debug("%s: hp jack %s\n", __func__, level?"IN":"OUT");

	if (!level) {
		/* HP off/AMP on */
		snd_soc_update_bits(codec, WM8978_LOUT1_HP_CONTROL, 0x40, 0x40);
		snd_soc_update_bits(codec, WM8978_ROUT1_HP_CONTROL, 0x40, 0x40);
		gpio_direction_output(AUDIO_AMP_POWER, 1);
	} else {
		/* HP on/AMP off  */
		snd_soc_update_bits(codec, WM8978_LOUT1_HP_CONTROL, 0x40, 0x0);
		snd_soc_update_bits(codec, WM8978_ROUT1_HP_CONTROL, 0x40, 0x0);
		gpio_direction_output(AUDIO_AMP_POWER, 0);
	}

	return !level;
}

static int wm8976_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
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
}

static int wm8976_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	int stream = substream->stream;
	int jack = jack_gpio.gpio;
	int invert = jack_gpio.invert;
	int level = gpio_get_value_cansleep(jack);

	pr_debug("%s\n", __func__);

	/* set jack detect gpio2 */
	snd_soc_update_bits(codec, WM8978_JACK_DETECT_CONTROL_1, 0x30, 0x10);
	snd_soc_update_bits(codec, WM8978_JACK_DETECT_CONTROL_1, 0x40, 0x00);	// disable jack detection

	if (stream == SNDRV_PCM_STREAM_CAPTURE)
		snd_soc_update_bits(codec, WM8978_POWER_MANAGEMENT_1, 0x10, 0x10);	// MICBIASEN

	if(invert)
		level = !level;

	if (!level) {
		pr_debug("AMP ON\n");
		gpio_direction_output(AUDIO_AMP_POWER, 1);
	}
	return 0;
}

static void wm8976_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	int stream = substream->stream;

	pr_debug("%s\n", __func__);

	if (stream == SNDRV_PCM_STREAM_CAPTURE)
		snd_soc_update_bits(codec, WM8978_POWER_MANAGEMENT_1, 0x10, 0x00);	// MICBIASEN

    if (stream == SNDRV_PCM_STREAM_PLAYBACK)
		gpio_direction_output(AUDIO_AMP_POWER, 0);
}

static int wm8976_suspend_pre(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = wm8976;

	snd_soc_update_bits(codec, WM8978_LOUT1_HP_CONTROL, 0x40, 0x40);
	snd_soc_update_bits(codec, WM8978_ROUT1_HP_CONTROL, 0x40, 0x40);
	gpio_direction_output(AUDIO_AMP_POWER, 0);
	return 0;
}

static int wm8976_resume_pre(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = wm8976;
	PM_DBGOUT("%s BAIAS=%d\n", __func__, codec->dapm.bias_level);
	codec_bias_level = codec->dapm.bias_level;
	return 0;
}

static int wm8976_resume_post(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = wm8976;
	PM_DBGOUT("%s BAIAS=%d\n", __func__, codec->dapm.bias_level);

	if (SND_SOC_BIAS_OFF != codec_bias_level)
		codec->driver->resume(codec);

	wm8976_jack_status_check();
	return 0;
}

static struct snd_soc_jack_gpio jack_gpio = {
	.invert		= true,			// High detect : invert = false
	.name		= "hp-gpio",
	.report		= SND_JACK_HEADPHONE,
	.debounce_time = 200,
	.jack_status_check = wm8976_jack_status_check,
};

static struct snd_soc_jack hp_jack;

static int wm8976_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_jack_gpio *jack = &jack_gpio;
	int ret;

	pr_debug("%s: %s\n", __func__, jack->name);

	wm8976 = codec;

	if (NULL == jack->name)
		return 0;

	/* Headset jack detection */
	ret = snd_soc_jack_new(codec, "Headphone Jack", SND_JACK_HEADPHONE, &hp_jack);
	if (ret)
		return ret;

	ret = snd_soc_jack_add_gpios(&hp_jack, 1, jack);
	pr_debug("%s: %s register audio jack detect, %d\n",
		ret?"Fail":"Done", __func__, jack->gpio);

	return 0;
}

static struct snd_soc_ops wm8976_ops = {
	.hw_params 	= wm8976_hw_params,
	.startup	= wm8976_startup,
	.shutdown	= wm8976_shutdown,
};

static char str_dai_name[16] = DEV_NAME_I2S;

static struct snd_soc_dai_link wm8976_dai_link = {
	.name 			= "ASOC-WM8976",
	.stream_name 	= "wm8976 HiFi",
	.cpu_dai_name 	= str_dai_name,			/* nxp_snd_i2s_driver name */
	.platform_name  = DEV_NAME_PCM,			/* nxp_snd_pcm_driver name */
	.codec_dai_name = "wm8978_codec",		/* wm8976_dai's name */
	.codec_name 	= "wm8978.0-001a",		/* wm8976_i2c_driver name + '.' + bus + '-' + address(7bit) */
	.ops 			= &wm8976_ops,
	.symmetric_rates = 1,
	.init			= wm8976_dai_init,
	.ops 			= &wm8976_ops,
};

static struct snd_soc_card wm8976_card = {
	.name 			= "I2S-WM8976",		/* proc/asound/cards */
	.owner 			= THIS_MODULE,
	.dai_link 		= &wm8976_dai_link,
	.num_links 		= 1,
	.suspend_pre	= &wm8976_suspend_pre,
	.resume_pre		= &wm8976_resume_pre,
	.resume_post	= &wm8976_resume_post,
};

/*
 * codec driver
 */
static int wm8976_probe(struct platform_device *pdev)
{
	struct nxp_snd_dai_plat_data *plat = pdev->dev.platform_data;
	struct snd_soc_card *card = &wm8976_card;
	struct snd_soc_jack_gpio *jack = &jack_gpio;
	struct nxp_snd_jack_pin *hpin = NULL;
	unsigned int rates = 0, format = 0;
	int ret;

	if (plat) {
		rates = plat->sample_rate;
		format = plat->pcm_format;
		hpin = &plat->hp_jack;
		if (hpin->support) {
			jack->gpio = hpin->detect_io;
			jack->invert = hpin->detect_level ?  false : true;
			jack->debounce_time = hpin->debounce_time ? : 200;
		} else {
			jack->name = NULL;
		}
		sprintf(str_dai_name, "%s.%d", DEV_NAME_I2S, plat->i2s_ch);	// set I2S name
	}

	gpio_request(AUDIO_AMP_POWER, "wm8978");

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

	pr_debug("wm8976-dai: register card %s -> %s\n",
		card->dai_link->codec_dai_name, card->dai_link->cpu_dai_name);
	return 0;
}

static int wm8976_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	pr_debug("%s\n", __func__);
	snd_soc_unregister_card(card);
	gpio_free(AUDIO_AMP_POWER);
	return 0;
}

static struct platform_driver wm8976_driver = {
	.driver		= {
		.name	= "wm8976-audio",
		.owner	= THIS_MODULE,
		.pm 	= &snd_soc_pm_ops,	/* for suspend */
	},
	.probe		= wm8976_probe,
	.remove		= __devexit_p(wm8976_remove),
};
module_platform_driver(wm8976_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound codec-wm8976 driver for the SLSI");
MODULE_LICENSE("GPL");
