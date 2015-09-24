/*
 *
 * Copyright (C) 2012 Samsung Electronics Co., Ltd.
 * Copyright (C) 2012 Google, Inc.
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
#include <linux/module.h>
#include <linux/slab.h>

#include <sound/soc.h>

#include "spdif.h"

struct manta_spdif {
	struct clk *clk_parent;
	struct clk *clk_spdif;
};

static int manta_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct manta_spdif *machine =
				snd_soc_card_get_drvdata(rtd->codec->card);

	unsigned long pll_out, rclk_rate;
	int ret;

	switch (params_rate(params)) {
	case 44100:
		pll_out = 45158400;
		break;
	case 32000:
	case 48000:
	case 96000:
		pll_out = 49152000;
		break;
	default:
		return -EINVAL;
	}

	/* Setting ratio to 512fs helps to use SPDIF with HDMI without
	 * modify SPDIF ASoC machine driver.
	 */
	rclk_rate = params_rate(params) * 512;

	clk_set_rate(machine->clk_parent, pll_out);
	clk_set_rate(machine->clk_spdif, rclk_rate);

	/* Set SPDIF uses internal source clock */
	ret = snd_soc_dai_set_sysclk(cpu_dai, SND_SOC_SPDIF_INT_MCLK,
					rclk_rate, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	return ret;
}

static struct snd_soc_ops manta_spdif_ops = {
	.hw_params = manta_hw_params,
};

static struct snd_soc_dai_link manta_dai = {
	.name = "SPDIF",
	.stream_name = "SPDIF PCM Playback",
	.platform_name = "samsung-audio",
	.cpu_dai_name = "samsung-spdif",
	.codec_dai_name = "dit-hifi",
	.codec_name = "spdif-dit",
	.ops = &manta_spdif_ops,
};

static struct snd_soc_card manta = {
	.name = "Manta-SPDIF",
	.owner = THIS_MODULE,
	.dai_link = &manta_dai,
	.num_links = 1,
};

static int __devinit snd_manta_probe(struct platform_device *pdev)
{
	struct manta_spdif *machine;
	int ret;

	machine = kzalloc(sizeof(*machine), GFP_KERNEL);
	if (!machine) {
		pr_err("%s: Failed to allocate memory\n", __func__);
		ret = -ENOMEM;
		goto err_kzalloc;
	}

	machine->clk_parent = clk_get(NULL, "fout_epll");
	if (IS_ERR(machine->clk_parent)) {
		pr_err("%s: failed to get fout_epll\n", __func__);
		ret = PTR_ERR(machine->clk_parent);
		goto err_clk_parent_get;
	}

	machine->clk_spdif = clk_get(NULL, "sclk_spdif");
	if (IS_ERR(machine->clk_spdif)) {
		pr_err("%s: failed to get sclk_spdif\n", __func__);
		ret = PTR_ERR(machine->clk_spdif);
		goto err_clk_spdif_get;
	}

	snd_soc_card_set_drvdata(&manta, machine);

	manta.dev = &pdev->dev;
	ret = snd_soc_register_card(&manta);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
		goto err_register_card;
	}

	return 0;

err_register_card:
	clk_put(machine->clk_spdif);
err_clk_spdif_get:
	clk_put(machine->clk_parent);
err_clk_parent_get:
	kfree(machine);
err_kzalloc:
	return ret;
}

static int __devexit snd_manta_remove(struct platform_device *pdev)
{
	struct manta_spdif *machine = snd_soc_card_get_drvdata(&manta);

	snd_soc_unregister_card(&manta);
	clk_put(machine->clk_parent);
	clk_put(machine->clk_spdif);
	kfree(machine);

	return 0;
}

static struct platform_driver snd_manta_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "manta-spdif",
		.pm = &snd_soc_pm_ops,
	},
	.probe = snd_manta_probe,
	.remove = __devexit_p(snd_manta_remove),
};

module_platform_driver(snd_manta_driver);

MODULE_DESCRIPTION("ALSA SoC Manta SPDIF");
MODULE_LICENSE("GPL");
