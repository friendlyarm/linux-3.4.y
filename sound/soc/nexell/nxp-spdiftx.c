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

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include "nxp-spdif.h"

/*
#define pr_debug		printk
*/

#define	DEF_SAMPLE_RATE			48000
#define	DEF_SAMPLE_BIT			16	// 8, 16, 24 (PCM)

#define	SPDIF_BASEADDR			PHY_BASEADDR_SPDIF_TX

#if (DEF_SAMPLE_BIT == 16)
#define	SPDIF_BUS_WIDTH			2	// Byte
#else
#define	SPDIF_BUS_WIDTH			4	// Byte
#endif

#define	SPDIF_MAX_BURST			4	// Byte
#define	SPDIF_MAX_CLOCK			166000000

#define	SPDIFTX_PRESETn 		(43)

/*
 * SPDIF register
 */
struct spdif_register {
	unsigned int clkcon;		// 0x0000	// R/W	// clock control register
	unsigned int con;			// 0x0004	// R/W	// control register
	unsigned int bstas;			// 0x0008	// R/W	// burst status register
	unsigned int cstas;			// 0x000C	// R/W	// channel status register
	unsigned int data;			// 0x0010	//   W	// SPDIF_IFOUT data buffer
	unsigned int count;			// 0x0014 	//   W	// repetition count register
	unsigned int bstas_shd;		// 0x0018 	// R	// shadowed burst status register
	unsigned int cnt_shd;		// 0x001C 	// R	// shadowed repetition count register
	unsigned int userbit1;		// 0x0020 	// R/W	// sub-code Q1 to Q32
	unsigned int userbit2;		// 0x0024 	// R/W	// sub-code Q33 to Q64
	unsigned int userbit3;		// 0x0028 	// R/W	// sub-code Q65 to Q96
	unsigned int userbit1_shd;	// 0x002C	// R	// shadowed register userbit1
	unsigned int userbit2_shd;	// 0x0030	// R	// shadowed register userbit2
	unsigned int userbit3_shd;	// 0x0034	// R	// shadowed register userbit3
	unsigned int version_info;	// 0x0038	// R	// RTL version information
};

#define	SPDIF_CLKCON_OFFSET		(0x00)
#define	SPDIF_CON_OFFSET		(0x04)
#define	SPDIF_BSTAS_OFFSET		(0x08)
#define	SPDIF_CSTAS_OFFSET		(0x0C)
#define	SPDIF_DAT_OFFSET		(0x10)
#define	SPDIF_CNT_OFFSET		(0x14)

#define	CLKCON_MCLK_SEL_POS		2		// [2:3]
#define	CLKCON_POWER_POS		0		// [0]

#define	CON_FIFO_LV_POS			22		// [22:26]
#define	CON_FIFO_TH_POS			19		// [19:21]
#define	CON_FIFO_TR_POS			17		// [17:18]
#define	CON_ENDIAN_POS			13		// [13:14]
#define CON_USERDATA_POS		12		// [12]
#define	CON_SW_RESET_POS		5		// [5]
#define	CON_RATIO_POS			3		// [3:4], main audio clock frequency (256, 384, 512)
#define	CON_PCM_BIT_POS			1		// [1:2], sample bits (16, 20, 24)
#define	CON_PCM_POS				0		// [0]

#define	CSTAS_FREQUENCY_POS		24		// [24:27], sample rate (32, 44.1, 48, 96)
#define	CSTAS_CATEGORY_POS		8		// [8:15]
#define	CSTAS_COPYRIGHT_POS		2		// [2]
#define	CSTAS_PCM_TYPE_POS		1		// [1],  0=Linear PCM, 1 = Non-linear PCM

#define	CON_FIFO_LEVEL_0		0
#define	CON_FIFO_LEVEL_1		1
#define	CON_FIFO_LEVEL_4		2
#define	CON_FIFO_LEVEL_6		3
#define	CON_FIFO_LEVEL_10		4
#define	CON_FIFO_LEVEL_12		5
#define	CON_FIFO_LEVEL_14		6
#define	CON_FIFO_LEVEL_15		7

#define	CON_FIFO_TR_DMA			0
#define	CON_FIFO_TR_POLLING		1
#define	CON_FIFO_TR_INTR		2

#define	CON_ENDIAN_BIG			0
#define	CON_ENDIAN_4B_SWAP		1
#define	CON_ENDIAN_3B_SWAP		2
#define	CON_ENDIAN_2B_SWAP		3

#define PCM_16BIT				0
#define PCM_20BIT				1
#define PCM_24BIT				2

#define RATIO_256				0
#define RATIO_384				1

#define	CATEGORY_CD				1
#define	CATEGORY_DAT			3
#define	CATEGORY_DCC			0x43
#define	CATEGORY_MiNi_DISC		0x49

#define	NO_COPYRIGHT			1
#define	LINEAR_PCM				0
#define	NON_LINEAR_PCM			1

struct clock_ratio {
	unsigned int sample_rate;
	unsigned int ratio_256;
	unsigned int ratio_384;
	int			 reg_val;
};

static struct clock_ratio clk_ratio [] = {
	{  32000,  8192000, 12288000,  3, },
	{  44100, 11289600, 16934400,  0, },
	{  48000, 12288000, 18432000,  2, },
	{  96000, 24576000, 36864000, 10, },
};

/*
 * parameters
 */
struct nxp_spdif_snd_param {
    int sample_rate;
	int	status;
	long master_clock;
	int  master_ratio;
	spinlock_t	lock;
	/* clock control */
	struct clk *clk;
	long clk_rate;
	/* DMA channel */
	struct nxp_pcm_dma_param dma;
	/* Register */
	unsigned int base_addr;
	struct spdif_register spdif;
	int hdmi_out;
	int hdmi_mclk_pll;
	int hdmi_mclk_div;
};

#define	SPDIF_MASTER_CLKGEN_BASE		0xC0105000
#define	SPDIF_IN_CLKS					4				// PLL0, PLL1, PLL2, PLL3
#define	MAX_DIVIDER						((1<<8) - 1)	// 256, align 2
#define	DIVIDER_ALIGN					2

static int calc_hdmi_master_clock(unsigned long mclk, int *pllsel, int *plldiv)
{
	struct clk *clk;
	unsigned long request = mclk * 8;
	unsigned long clk_hz[SPDIF_IN_CLKS], freq_hz = 0, rate_hz = 0, pll_hz;
	int pll_div = 0, divide, maxdiv, alidiv, n, pll_sel = 0;

	clk = clk_get(NULL, "pll0"), clk_hz[0] = clk_get_rate(clk), clk_put(clk);
	clk = clk_get(NULL, "pll1"), clk_hz[1] = clk_get_rate(clk), clk_put(clk);
	clk = clk_get(NULL, "pll2"), clk_hz[2] = clk_get_rate(clk), clk_put(clk);
	clk = clk_get(NULL, "pll3"), clk_hz[3] = clk_get_rate(clk), clk_put(clk);

	for (n = 0; SPDIF_IN_CLKS > n; n++) {
	#ifdef  CONFIG_ARM_NXP_CPUFREQ
		if (n == CONFIG_NXP_CPUFREQ_PLLDEV)
			continue;
	#endif
		pll_hz = clk_hz[n];
		divide = (pll_hz/request);
		maxdiv = MAX_DIVIDER & ~(DIVIDER_ALIGN-1);
		alidiv = (divide & ~(DIVIDER_ALIGN-1)) + DIVIDER_ALIGN;

		if (!divide) {
			divide = 1;
		} else {
			if (1 != divide)
				divide &= ~(DIVIDER_ALIGN-1);

			if (divide != alidiv &&
				abs(request - pll_hz/divide) >
				abs(request - pll_hz/alidiv))
				divide = alidiv;

			divide = (divide > maxdiv ? maxdiv : divide);
		}
		freq_hz = pll_hz / divide;

		if (rate_hz && (abs(freq_hz-request) > abs(rate_hz-request)))
			continue;

		rate_hz = freq_hz;
		pll_div = divide;
		pll_sel = n;
	}

	if (pllsel)
		*pllsel = pll_sel;

	if (plldiv)
		*plldiv = pll_div;

	pr_debug("%s: req=%ld, acq=%ld (pll[%d]=%ld, div=%2d)\n",
		__func__, request, rate_hz, pll_sel, clk_hz[pll_sel], pll_div);

	return 0;
}

static void inline spdif_reset(struct nxp_spdif_snd_param *par)
{
	unsigned int base = par->base_addr;

	nxp_soc_peri_reset_set(RESET_ID_SPDIFTX);
	writel((1 << CON_SW_RESET_POS), (base+SPDIF_CON_OFFSET));
}

static int spdif_start(struct nxp_spdif_snd_param *par, int stream)
{
	struct spdif_register *spdif = &par->spdif;
	unsigned int base = par->base_addr;
	unsigned int hdmi = IO_ADDRESS(SPDIF_MASTER_CLKGEN_BASE);
	volatile u32 value;

	if (par->hdmi_out) {
		writel(readl(hdmi) | (1<<3), hdmi);
		value = readl((hdmi + 0x04));
		value = (value & ~(0x07<<2)) | (par->hdmi_mclk_pll<<2);
		value = (value & ~(0xFF<<5)) | ((par->hdmi_mclk_div-1)<<5);
		writel(value, (hdmi + 0x04));
		writel(readl(hdmi) | (1<<2), hdmi);
	}

	spdif->clkcon |= (1<<CLKCON_POWER_POS);
	writel(spdif->con, (base+SPDIF_CON_OFFSET));
	writel(spdif->clkcon, (base+SPDIF_CLKCON_OFFSET));	/* Power On */

	par->status |= SNDDEV_STATUS_PLAY;
	return 0;
}

static void spdif_stop(struct nxp_spdif_snd_param *par, int stream)
{
	struct spdif_register *spdif = &par->spdif;
	unsigned int base = par->base_addr;
	unsigned int hdmi = IO_ADDRESS(SPDIF_MASTER_CLKGEN_BASE);

	spdif->clkcon &= ~(1 << CLKCON_POWER_POS);
	writel(spdif->clkcon, (base+SPDIF_CLKCON_OFFSET));

	par->status &= ~SNDDEV_STATUS_PLAY;

	if (par->hdmi_out)
		writel(readl(hdmi) & ~(1<<2), hdmi);	/* clk disable */
}

static int nxp_spdif_check_param(struct nxp_spdif_snd_param *par)
{
	struct spdif_register *spdif = &par->spdif;
	struct nxp_pcm_dma_param *dmap = &par->dma;
	unsigned long request = 0, rate_hz = 0;
	int MCLK = 0; /* only support internal */
	int PCM  = (DEF_SAMPLE_BIT == 24 ? PCM_24BIT : PCM_16BIT);
	int RATIO, SAMPLE_HZ, DATA_23RDBIT = 1, PCM_OR_STREAM = 1;
	int i = 0;

	for (i = 0; ARRAY_SIZE(clk_ratio) > i; i++) {
		if (par->sample_rate == clk_ratio[i].sample_rate) {
			SAMPLE_HZ = clk_ratio[i].reg_val;
			break;
		}
	}

	if (i >= ARRAY_SIZE(clk_ratio)) {
		printk(KERN_ERR "Fail, not support spdif sample rate %d \n",
			par->sample_rate);
		return -EINVAL;
	}

 	/* 384 RATIO */
	RATIO = RATIO_384, request = clk_ratio[i].ratio_384;
	rate_hz = clk_round_rate(par->clk, request);

	/* 256 RATIO */
	if (rate_hz != request && PCM == PCM_16BIT) {
		unsigned int o_rate = rate_hz;
		RATIO = RATIO_256, request = clk_ratio[i].ratio_256;
		rate_hz = clk_round_rate(par->clk, request);
		if (abs(request - rate_hz) > abs(request - o_rate)) {
			rate_hz = o_rate, RATIO = RATIO_384;
		}
	}

	par->master_clock = rate_hz;
	par->master_ratio = RATIO;

	dmap->real_clock = rate_hz/(RATIO_256==RATIO?256:384);

	spdif->clkcon 	= 	(MCLK << CLKCON_MCLK_SEL_POS);
	spdif->con 		= 	(CON_FIFO_LEVEL_15 	<< CON_FIFO_TH_POS)	| \
				 	  	(CON_FIFO_TR_DMA 	<< CON_FIFO_TR_POS)	| \
				 	  	(CON_ENDIAN_BIG 	<< CON_ENDIAN_POS)	| \
				 	  	(DATA_23RDBIT 		<< CON_USERDATA_POS)| \
				 		(RATIO 				<< CON_RATIO_POS)	| \
				 		(PCM 				<< CON_PCM_BIT_POS) | \
				 		(PCM_OR_STREAM 		<< CON_PCM_POS);
	spdif->cstas 	= 	(SAMPLE_HZ		<< CSTAS_FREQUENCY_POS)	| \
				   		(CATEGORY_CD 	<< CSTAS_CATEGORY_POS)	| \
				   		(LINEAR_PCM 	<< CSTAS_PCM_TYPE_POS)	| \
				   		(NO_COPYRIGHT 	<< CSTAS_COPYRIGHT_POS);

	return 0;
}

static int nxp_spdif_set_plat_param(struct nxp_spdif_snd_param *par, void *data)
{
	struct platform_device *pdev = data;
	struct nxp_spdif_plat_data *plat = pdev->dev.platform_data;
	struct nxp_pcm_dma_param *dma = &par->dma;
	unsigned int phy_base = SPDIF_BASEADDR;
	int ret = 0;

	par->sample_rate = plat->sample_rate ? plat->sample_rate : DEF_SAMPLE_RATE;
	par->hdmi_out = plat->hdmi_out ? 1 : 0;
	par->base_addr = IO_ADDRESS(phy_base);
	spin_lock_init(&par->lock);

	if (! plat->dma_ch)
		return -EINVAL;

	dma->active = true;
	dma->dma_filter = plat->dma_filter;
	dma->dma_ch_name = (char*)(plat->dma_ch);
	dma->peri_addr = phy_base + SPDIF_DAT_OFFSET;	/* SPDIF DAT */
	dma->bus_width_byte = SPDIF_BUS_WIDTH;
	dma->max_burst_byte = SPDIF_MAX_BURST;
	pr_debug("spdif-tx: %s, %s dma, addr 0x%x, bus %dbyte, burst %dbyte\n",
		STREAM_STR(0), dma->dma_ch_name, dma->peri_addr,
		dma->bus_width_byte, dma->max_burst_byte);

	par->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(par->clk)) {
		ret = PTR_ERR(par->clk);
		return ret;
	}

	return nxp_spdif_check_param(par);
}

static struct snd_soc_dai_driver spdif_dai_driver;

static int nxp_spdif_setup(struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct spdif_register *spdif = &par->spdif;
	unsigned int base = par->base_addr;
	int hdmi_out = par->hdmi_out;
	long rate_hz = par->master_clock;
	int  ratio = par->master_ratio;
	unsigned int cstas = spdif->cstas;

	if (SNDDEV_STATUS_SETUP & par->status)
		return 0;

	printk(KERN_INFO "spdif-tx: %d(%ld)Hz, MCLK=%ldhz %s out\n",
		par->sample_rate, rate_hz/(RATIO_256==ratio?256:384), rate_hz,
		par->hdmi_out?"HDMI":"SPDIF");

	if (hdmi_out)
		calc_hdmi_master_clock(rate_hz, &par->hdmi_mclk_pll, &par->hdmi_mclk_div);

	/* set clock */
	par->clk_rate = clk_set_rate(par->clk, rate_hz);
	clk_enable(par->clk);

	spdif_reset(par);

	cstas |= readl(base+SPDIF_CSTAS_OFFSET) & 0x3fffffff;
	writel(spdif->con, (base+SPDIF_CON_OFFSET));
	writel(cstas, (base+SPDIF_CSTAS_OFFSET));

	par->status |= SNDDEV_STATUS_SETUP;
	return 0;
}

static void nxp_spdif_release(struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct spdif_register *spdif = &par->spdif;
	unsigned int base = par->base_addr;

	spdif->clkcon &= ~(1 << CLKCON_POWER_POS);
	writel(spdif->clkcon, (base+SPDIF_CLKCON_OFFSET));

	clk_disable(par->clk);
	clk_put(par->clk);

	par->status = SNDDEV_STATUS_CLEAR;
}

/*
 * snd_soc_dai_ops
 */
static int  nxp_spdif_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct nxp_pcm_dma_param *dmap = &par->dma;

	snd_soc_dai_set_dma_data(dai, substream, dmap);
	return 0;
}


static void nxp_spdif_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	spdif_stop(par, substream->stream);
}

static int nxp_spdif_trigger(struct snd_pcm_substream *substream,
				int cmd, struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	int stream = substream->stream;
	pr_debug("%s: %s cmd=%d\n", __func__, STREAM_STR(stream), cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_START:
		spdif_start(par, stream);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_STOP:
		spdif_stop(par, stream);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int nxp_spdif_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct spdif_register *spdif = &par->spdif;
	struct nxp_pcm_dma_param *dmap = &par->dma;
	unsigned int format = params_format(params);
	int PCM = (spdif->con >> CON_PCM_BIT_POS) & 0x3;
	int ret = 0;

	switch (format) {
	case SNDRV_PCM_FORMAT_S16_LE:
		pr_debug("spdiftx: change sample bits %s -> S16\n", PCM==PCM_16BIT?"S16":"S24");
		if (PCM != PCM_16BIT) {
			spdif->con &= ~(0x3 << CON_PCM_BIT_POS);
			spdif->con |=  (PCM_16BIT << CON_PCM_BIT_POS);
			dmap->bus_width_byte = 2;	/* change dma bus width */
			dmap->max_burst_byte = SPDIF_MAX_BURST;
		}
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		pr_debug("spdiftx: change sample bits %s -> S24\n", PCM==PCM_16BIT?"S16":"S24");
		if (PCM != PCM_24BIT) {
			spdif->con &= ~(0x3 << CON_PCM_BIT_POS);
			spdif->con |=  (PCM_24BIT << CON_PCM_BIT_POS);
			dmap->bus_width_byte = 4;	/* change dma bus width */
			dmap->max_burst_byte = SPDIF_MAX_BURST;
		}
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

static struct snd_soc_dai_ops nxp_spdif_ops = {
	.startup	= nxp_spdif_startup,
	.shutdown	= nxp_spdif_shutdown,
	.trigger	= nxp_spdif_trigger,
	.hw_params	= nxp_spdif_hw_params,
};

/*
 * snd_soc_dai_driver
 */
static int nxp_spdif_dai_suspend(struct snd_soc_dai *dai)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}

static int nxp_spdif_dai_resume(struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct spdif_register *spdif = &par->spdif;
	unsigned int cstas = spdif->cstas;
	unsigned int base = par->base_addr;

	PM_DBGOUT("%s\n", __func__);

	clk_set_rate(par->clk, par->clk_rate);
	clk_enable(par->clk);

	spdif_reset(par);

	cstas |= readl(base+SPDIF_CSTAS_OFFSET) & 0x3fffffff;
	writel(spdif->con, (base+SPDIF_CON_OFFSET));
	writel(cstas, (base+SPDIF_CSTAS_OFFSET));

	return 0;
}

static int nxp_spdif_dai_probe(struct snd_soc_dai *dai)
{
	return nxp_spdif_setup(dai);
}

static int nxp_spdif_dai_remove(struct snd_soc_dai *dai)
{
	nxp_spdif_release(dai);
	return 0;
}

static struct snd_soc_dai_driver spdif_dai_driver = {
	.playback	= {
		.channels_min 	= 2,
		.channels_max 	= 2,
		.formats		= SND_SOC_SPDIF_TX_FORMATS,
		.rates			= SND_SOC_SPDIF_RATES,
		.rate_min 		= 0,
		.rate_max 		= 1562500,
		},
	.probe 		= nxp_spdif_dai_probe,
	.remove		= nxp_spdif_dai_remove,
	.suspend	= nxp_spdif_dai_suspend,
	.resume 	= nxp_spdif_dai_resume,
	.ops 		= &nxp_spdif_ops,
};

static __devinit int nxp_spdif_probe(struct platform_device *pdev)
{
	struct nxp_spdif_snd_param *par;
	int ret = 0;

    /*  allocate i2c_port data */
    par = kzalloc(sizeof(struct nxp_spdif_snd_param), GFP_KERNEL);
    if (! par) {
        printk(KERN_ERR "fail, %s allocate driver info ...\n", pdev->name);
        return -ENOMEM;
    }

	ret = nxp_spdif_set_plat_param(par, pdev);
	if (ret)
		goto err_out;

	ret = snd_soc_register_dai(&pdev->dev, &spdif_dai_driver);
	if (ret) {
        printk(KERN_ERR "fail, %s snd_soc_register_dai ...\n", pdev->name);
		goto err_out;
	}

	dev_set_drvdata(&pdev->dev, par);
	return ret;

err_out:
	if (par)
		kfree(par);
	return ret;
}

static __devexit int nxp_spdif_remove(struct platform_device *pdev)
{
	struct nxp_spdif_snd_param *par = dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_dai(&pdev->dev);
	if (par)
		kfree(par);
	return 0;
}

static struct platform_driver spdif_driver = {
	.probe  = nxp_spdif_probe,
	.remove = nxp_spdif_remove,
	.driver = {
	.name 	= DEV_NAME_SPDIF_TX,
	.owner 	= THIS_MODULE,
	},
};

static int __init nxp_spdif_init(void)
{
	return platform_driver_register(&spdif_driver);
}

static void __exit nxp_spdif_exit(void)
{
	platform_driver_unregister(&spdif_driver);
}

module_init(nxp_spdif_init);
module_exit(nxp_spdif_exit);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound S/PDIF tx driver for the SLSI");
MODULE_LICENSE("GPL");