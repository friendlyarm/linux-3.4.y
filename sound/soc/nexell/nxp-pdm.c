/*
 * (C) Copyright 2009
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
#include <mach/pdm.h>

#include "nxp-pdm.h"

/*
#define pr_debug(msg...)		printk(KERN_INFO msg)
*/

#define	DEF_SAMPLE_RATE			48000
#define	DEF_SAMPLE_BIT			16	// 16 (PCM)

#define	PDM_BASEADDR			PHY_BASEADDR_PDM
#define	PDM_BUS_WIDTH			4	// Byte
#define	PDM_MAX_BURST			32	// Byte

//#define PDM_IRQ_COUNT			8

/*
 * PDM register
 */
struct pdm_register {
	unsigned int CTRL;
	unsigned int GAIN0;
	unsigned int GAIN1;
	unsigned int COEFF;
	unsigned int DATA;
	unsigned int CTRL1;
	unsigned int INTC;
};

#define	PDM_CTRL_OFFSET			(0x00)
#define	PDM_GAIN0_OFFSET		(0x04)
#define	PDM_GAIN1_OFFSET		(0x08)
#define	PDM_COEFF_OFFSET		(0x0c)
#define	PDM_DATA_OFFSET			(0x10)
#define	PDM_CTRL1_OFFSET		(0x14)
#define	PDM_INTC_OFFSET			(0x18)

/*
 * parameters
 */
struct nxp_pdm_snd_param {
    int sample_rate;
	int	status;
	spinlock_t	lock;
    /* clock control */
    struct clk *clk;
    long clk_rate;
	/* DMA channel */
	struct nxp_pcm_dma_param dma;
	/* Register */
	unsigned int base_addr;
	struct pdm_register pdm;
};

static void inline pdm_reset(struct nxp_pdm_snd_param *par)
{
	nxp_soc_peri_reset_set(RESET_ID_PDM);
}

static int  pdm_start(struct nxp_pdm_snd_param *par, int stream)
{
	struct pdm_register *pdm = &par->pdm;
	unsigned int base = par->base_addr;

	pr_debug("%s\n", __func__);

	NX_PDM_ClearInterruptPendingAll(0);

	// pdm start
	pdm->CTRL |= 0x1<<1;
	writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));
	
	par->status |= SNDDEV_STATUS_CAPT;
	return 0;
}

static void pdm_stop(struct nxp_pdm_snd_param *par, int stream)
{
	struct pdm_register *pdm = &par->pdm;
	unsigned int base = par->base_addr;
	pdm->CTRL = readl(par->base_addr+PDM_CTRL_OFFSET);
	
	pr_debug("%s\n", __func__);

	// pdm stop
	pdm->CTRL &= ~(0x1<<1);
	writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));

	par->status &= ~SNDDEV_STATUS_CAPT;
}

static int nxp_pdm_check_param(struct nxp_pdm_snd_param *par)
{
	struct pdm_register *pdm = &par->pdm;
    struct nxp_pcm_dma_param *dmap = &par->dma;	
	unsigned long request = 0;
	unsigned int base = par->base_addr;
	struct clk *pclk = NULL;

	pclk = clk_get(NULL, "pclk");
	request = clk_get_rate(pclk);

	pdm->CTRL = readl(par->base_addr+PDM_CTRL_OFFSET);
	pdm->CTRL1 = readl(par->base_addr+PDM_CTRL_OFFSET);

	dmap->real_clock = par->sample_rate;

    par->clk_rate = clk_set_rate(par->clk, request);
	printk(KERN_INFO "pdm-rec: PDM CLK=%ldhz \n", par->clk_rate);

	pdm_reset(par);

	clk_enable(par->clk);

	par->status |= SNDDEV_STATUS_POWER;

	// dma mode default(single mode)
	pdm->CTRL &= ~(0x1 << 2);
	writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));
	
	// init reset
	pdm->CTRL |= 0x1;
	writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));

	// shiftperpixel
	pdm->CTRL1 &= ~(0x7<<16);
	pdm->CTRL1 |= 0 << 16;
	
	// number of clock
	pdm->CTRL1 &= ~(0xff<<8);
	pdm->CTRL1 |= 35 << 8;

	// sample position
	pdm->CTRL1 &= ~(0xff<<0);
	pdm->CTRL1 |= 0 << 0;
	writel(pdm->CTRL1, (base+PDM_CTRL1_OFFSET));

	// gain0
	pdm->GAIN0 = (0x2a6 << 16) | (0x154 << 0);
	writel(pdm->GAIN0, (base+PDM_GAIN0_OFFSET));

	// gain1
	pdm->GAIN1 = (0xfd5a << 16) | (0xfeac << 0);
	writel(pdm->GAIN1, (base+PDM_GAIN1_OFFSET));

	// coeff
	pdm->COEFF = (0x3f2e << 16) | (0xe0cf << 0);
	writel(pdm->COEFF, (base+PDM_COEFF_OFFSET));

	// over sample
	pdm->CTRL &= ~(0x7f<<16);
	pdm->CTRL |= 57 << 16;

	// strobe shift
	pdm->CTRL &= ~(0x1f<<8);
	pdm->CTRL |= 0 << 8;
	writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));

#if defined(PDM_IRQ_COUNT)
	// interrupt mode enable
	pdm->INTC = PDM_IRQ_COUNT;
	writel(pdm->INTC, (base+PDM_INTC_OFFSET));
#endif

	// init reset
	pdm->CTRL &= ~(0x1);
	writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));

	return 0;
}

static int nxp_pdm_set_plat_param(struct nxp_pdm_snd_param *par, void *data)
{
	struct platform_device *pdev = data;
	struct nxp_pdm_plat_data *plat = pdev->dev.platform_data;
	struct nxp_pcm_dma_param *dma = &par->dma;
	unsigned int phy_base = PDM_BASEADDR;
	int ret = 0;

    par->sample_rate = plat->sample_rate ? plat->sample_rate : DEF_SAMPLE_RATE;
	par->base_addr = IO_ADDRESS(phy_base);
	spin_lock_init(&par->lock);

	if (!plat->dma_ch)
		return -EINVAL;

	dma->active = true;
	dma->dma_filter = plat->dma_filter;
	dma->dma_ch_name = (char*)(plat->dma_ch);
	dma->peri_addr = phy_base + PDM_DATA_OFFSET;	/* PDM DAT */
	dma->bus_width_byte = PDM_BUS_WIDTH;
	dma->max_burst_byte = PDM_MAX_BURST;
	pr_debug("pdm-rec: %s, %s dma, addr 0x%x, bus %dbyte, burst %dbyte\n",
		STREAM_STR(1), dma->dma_ch_name, dma->peri_addr,
		dma->bus_width_byte, dma->max_burst_byte);

	par->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(par->clk)) {
		ret = PTR_ERR(par->clk);
		return ret;
	}

	return nxp_pdm_check_param(par);
}

static int nxp_pdm_setup(struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);

	if (SNDDEV_STATUS_SETUP & par->status)
		return 0;

	par->status |= SNDDEV_STATUS_SETUP;

	return 0;
}

static void nxp_pdm_release(struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);

	par->status = SNDDEV_STATUS_CLEAR;
}

/*
 * snd_soc_dai_ops
 */
static int  nxp_pdm_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct nxp_pcm_dma_param *dmap = &par->dma;

	snd_soc_dai_set_dma_data(dai, substream, dmap);
	return 0;
}

static void nxp_pdm_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);
	pdm_stop(par, substream->stream);
}

static int nxp_pdm_trigger(struct snd_pcm_substream *substream,
				int cmd, struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);
	int stream = substream->stream;
	pr_debug("%s: %s cmd=%d\n", __func__, STREAM_STR(stream), cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_START:
		pdm_start(par, stream);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_STOP:
		pdm_stop(par, stream);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int nxp_pdm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct pdm_register *pdm = &par->pdm;
	unsigned int base = par->base_addr;
	unsigned int format = params_format(params);
	unsigned int channels = params_channels(params);

	switch (format) {
		case SNDRV_PCM_FORMAT_S16_LE:
			pr_debug("pdmrec: change sample bits S16\n");
			break;
		default:
			return -EINVAL;
	}

	switch (channels) {
		case 2:
			// dma mode single
			pdm->CTRL &= ~(0x1 << 2);
			writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));
			break;
		case 4:
			// dma mode dual
			pdm->CTRL |= (0x1 << 2);
			writel(pdm->CTRL, (base+PDM_CTRL_OFFSET));
			break;
		default:
			break;
	}

	return 0;
}

static struct snd_soc_dai_ops nxp_pdm_ops = {
	.startup	= nxp_pdm_startup,
	.shutdown	= nxp_pdm_shutdown,
	.trigger	= nxp_pdm_trigger,
	.hw_params	= nxp_pdm_hw_params,
};

/*
 * snd_soc_dai_driver
 */
static int nxp_pdm_dai_suspend(struct snd_soc_dai *dai)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}

static int nxp_pdm_dai_resume(struct snd_soc_dai *dai)
{
	struct nxp_pdm_snd_param *par = snd_soc_dai_get_drvdata(dai);

	PM_DBGOUT("%s\n", __func__);

	pdm_reset(par);

	return 0;
}

static int nxp_pdm_dai_probe(struct snd_soc_dai *dai)
{
	return nxp_pdm_setup(dai);
}

static int nxp_pdm_dai_remove(struct snd_soc_dai *dai)
{
	nxp_pdm_release(dai);
	return 0;
}

static struct snd_soc_dai_driver pdm_dai_driver = {
	.capture	= {
		.channels_min 	= 2,
		.channels_max 	= 4,
		.formats		= SND_SOC_PDM_FORMATS,
		.rates			= SND_SOC_PDM_RATES,
		.rate_min 		= 0,
		.rate_max 		= 48000,
		},
	.probe 		= nxp_pdm_dai_probe,
	.remove		= nxp_pdm_dai_remove,
	.suspend	= nxp_pdm_dai_suspend,
	.resume 	= nxp_pdm_dai_resume,
	.ops 		= &nxp_pdm_ops,
};

#if defined(PDM_IRQ_COUNT)
static irqreturn_t nxp_pdm_irq(int irq, void *dev_id)
{
	NX_PDM_ClearInterruptPendingAll(0);

	return IRQ_HANDLED;
}
#endif

static __devinit int nxp_pdm_probe(struct platform_device *pdev)
{
	struct nxp_pdm_snd_param *par;
	int ret = 0;

    /*  allocate driver data */
    par = kzalloc(sizeof(struct nxp_pdm_snd_param), GFP_KERNEL);
    if (! par) {
        printk(KERN_ERR "fail, %s allocate driver info ...\n", pdev->name);
        return -ENOMEM;
    }

	NX_PDM_Initialize();
	NX_PDM_SetBaseAddress(0, (void*)IO_ADDRESS(NX_PDM_GetPhysicalAddress(0)));

#if defined(PDM_IRQ_COUNT)
	ret = request_irq(IRQ_PHY_PDM,
			nxp_pdm_irq, 0, "nxp-pdm-irq", par);
	if (ret < 0)
		printk("ERROR!!!!!\n");
#endif

	ret = nxp_pdm_set_plat_param(par, pdev);
	if (ret)
		goto err_out;

	ret = snd_soc_register_dai(&pdev->dev, &pdm_dai_driver);
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

static __devexit int nxp_pdm_remove(struct platform_device *pdev)
{
	struct nxp_pdm_snd_param *par = dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_dai(&pdev->dev);
	if (par)
		kfree(par);
	return 0;
}

static struct platform_driver pdm_driver = {
	.probe  = nxp_pdm_probe,
	.remove = nxp_pdm_remove,
	.driver = {
	.name 	= DEV_NAME_PDM,
	.owner 	= THIS_MODULE,
	},
};

static int __init nxp_pdm_init(void)
{
	return platform_driver_register(&pdm_driver);
}

static void __exit nxp_pdm_exit(void)
{
	platform_driver_unregister(&pdm_driver);
}

module_init(nxp_pdm_init);
module_exit(nxp_pdm_exit);

MODULE_AUTHOR("hsjung <hsjung@nexell.co.kr>");
MODULE_DESCRIPTION("Sound PDM recorder driver for the SLSI");
MODULE_LICENSE("GPL");

