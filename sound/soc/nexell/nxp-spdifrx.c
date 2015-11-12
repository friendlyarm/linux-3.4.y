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
#define pr_debug(msg...)		printk(KERN_INFO msg)
*/

#define	DEF_SAMPLE_RATE			48000
#define	DEF_SAMPLE_BIT			16	// 8, 16, 24 (PCM)

#define	SPDIF_BASEADDR			PHY_BASEADDR_SPDIF_RX
#define	SPDIF_BUS_WIDTH			4	// Byte
#define	SPDIF_MAX_BURST			4	// Byte

#define	SPDIFRX_PRESETn 		(42)

/*
 * SPDIF register
 */
struct spdif_register {
	unsigned int ctrl;
	unsigned int INTC;
};

#define	SPDIF_CTRL_OFFSET		(0x00)
#define	SPDIF_INTC_OFFSET		(0x04)
#define	SPDIF_DAT_OFFSET		(0x68)	// DMA: 0x68

#define	CTRL_CPUHEADER_POS			16	//	[16]
#define	CTRL_DECRATE_POS			12	//	[12:15]
#define	CTRL_DATA_ORDER_POS			11	//	[11]
#define	CTRL_LOCK_POS				10	//	[10]
#define	CTRL_CLRFIFO_POS            9	//	[9]
#define	CTRL_PHASE_DET_POS			8	//	[8]
#define	CTRL_VALID_SAMPLE_POS      	4	//	[4:7]
#define	CTRL_CAP_USER_DAT_POS       3	//	[3]
#define	CTRL_DMA_DATA_POS          	2	//	[2]
#define	CTRL_DMA_SWAP_POS           1	//	[1]
#define	CTRL_DECODE_ENB_POS        	0	//	[0]

#define	CTRL_ORDER_LSB_MSB			0
#define	CTRL_ORDER_MSB_LSB			1

#define	CTRL_VALID_SAMPLE_8			0
#define	CTRL_VALID_SAMPLE_7			1
#define	CTRL_VALID_SAMPLE_6			2
#define	CTRL_VALID_SAMPLE_5			3
#define	CTRL_VALID_SAMPLE_4			4
#define	CTRL_VALID_SAMPLE_3			5
#define	CTRL_VALID_SAMPLE_2			6
#define	CTRL_VALID_SAMPLE_1			7
#define	CTRL_VALID_SAMPLE_0			8

#define	CTRL_DMA_SWAP_CHA_CHB		0
#define	CTRL_DMA_SWAP_CHB_CHA		1

/*
 * parameters
 */
struct nxp_spdif_snd_param {
    int sample_rate;
	int	status;
	spinlock_t	lock;
	/* DMA channel */
	struct nxp_pcm_dma_param dma;
	/* Register */
	unsigned int base_addr;
	struct spdif_register spdif;
};

static void inline spdif_reset(struct nxp_spdif_snd_param *par)
{
	nxp_soc_peri_reset_set(RESET_ID_SPDIFRX);
}

static int  spdif_start(struct nxp_spdif_snd_param *par, int stream)
{
	struct spdif_register *spdif = &par->spdif;
	unsigned int ctrl = par->base_addr + SPDIF_CTRL_OFFSET;
	unsigned int pend = par->base_addr + SPDIF_INTC_OFFSET;

	/* clear fifo, pend */
	writel(readl(ctrl) | (1<<CTRL_CLRFIFO_POS), ctrl);
	writel(spdif->ctrl, ctrl);
	writel(readl(pend) | 0xF0, pend);

	/* run */
	spdif->ctrl |= (1<<CTRL_DECODE_ENB_POS);
	spdif->ctrl |= (1<<CTRL_PHASE_DET_POS);
	writel(spdif->ctrl, ctrl);

	par->status |= SNDDEV_STATUS_CAPT;
	return 0;
}

static void spdif_stop(struct nxp_spdif_snd_param *par, int stream)
{
	struct spdif_register *spdif = &par->spdif;
	unsigned int ctrl = par->base_addr + SPDIF_CTRL_OFFSET;

	spdif->ctrl &= ~((1<<CTRL_DECODE_ENB_POS) | (1<<CTRL_PHASE_DET_POS));
	writel(spdif->ctrl, ctrl);

	par->status &= ~SNDDEV_STATUS_CAPT;
}

static int nxp_spdif_check_param(struct nxp_spdif_snd_param *par)
{
	struct spdif_register *spdif = &par->spdif;
    struct nxp_pcm_dma_param *dmap = &par->dma;	
	int ORDER = CTRL_ORDER_LSB_MSB;
	int VALID_SAMPLE = CTRL_VALID_SAMPLE_8;
	int DATA_ONLY = 1;
	int DAM_SWAP = 1;	// CHA_CHB = 0, CHB_CHA = 1

	dmap->real_clock = par->sample_rate;

	spdif->ctrl =   (ORDER << CTRL_DATA_ORDER_POS) |			// [11]
					(0 << CTRL_LOCK_POS) |						// [10]
					(0 << CTRL_CLRFIFO_POS) |					// [9]
					(0 << CTRL_PHASE_DET_POS) |					// [8]	-> Enable
					(VALID_SAMPLE << CTRL_VALID_SAMPLE_POS) |	// [4:7]
					(1 << CTRL_CAP_USER_DAT_POS) |				// 3
					(DATA_ONLY << CTRL_DMA_DATA_POS) |			// 2
					(DAM_SWAP << CTRL_DMA_SWAP_POS) |			// 1
					(0	<< CTRL_DECODE_ENB_POS);				// 0

	return 0;
}

static int nxp_spdif_set_plat_param(struct nxp_spdif_snd_param *par, void *data)
{
	struct platform_device *pdev = data;
	struct nxp_spdif_plat_data *plat = pdev->dev.platform_data;
	struct nxp_pcm_dma_param *dma = &par->dma;
	unsigned int phy_base = SPDIF_BASEADDR;

    par->sample_rate = plat->sample_rate ? plat->sample_rate : DEF_SAMPLE_RATE;
	par->base_addr = IO_ADDRESS(phy_base);
	spin_lock_init(&par->lock);

	if (!plat->dma_ch)
		return -EINVAL;

	dma->active = true;
	dma->dma_filter = plat->dma_filter;
	dma->dma_ch_name = (char*)(plat->dma_ch);
	dma->peri_addr = phy_base + SPDIF_DAT_OFFSET;	/* SPDIF DAT */
	dma->bus_width_byte = SPDIF_BUS_WIDTH;
	dma->max_burst_byte = SPDIF_MAX_BURST;
	pr_debug("spdif-rx: %s, %s dma, addr 0x%x, bus %dbyte, burst %dbyte\n",
		STREAM_STR(1), dma->dma_ch_name, dma->peri_addr,
		dma->bus_width_byte, dma->max_burst_byte);

	return nxp_spdif_check_param(par);
}

static int nxp_spdif_setup(struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct spdif_register *spdif = &par->spdif;
	unsigned int ctrl = par->base_addr + SPDIF_CTRL_OFFSET;
	struct clk *clk = NULL;
	unsigned long pclk_hz = 0;


	if (SNDDEV_STATUS_SETUP & par->status)
		return 0;

	clk = clk_get(NULL, "pclk");
	pclk_hz = clk_get_rate(clk);
	clk_put(clk);

	spdif_reset(par);

	printk(KERN_INFO "spdif-rx: PCLK=%ldhz \n", pclk_hz);
	spdif->ctrl &= ~(0xf << CTRL_DECRATE_POS);
	if(pclk_hz > 180000000)
		spdif->ctrl |= 8 << CTRL_DECRATE_POS;
	else if(pclk_hz > 100000000)
		spdif->ctrl |= 6 << CTRL_DECRATE_POS;
	else
		spdif->ctrl |= 3 << CTRL_DECRATE_POS;

	writel(spdif->ctrl, ctrl);
	par->status |= SNDDEV_STATUS_SETUP;

	return 0;
}

static void nxp_spdif_release(struct snd_soc_dai *dai)
{
	struct nxp_spdif_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct spdif_register *spdif = &par->spdif;
	unsigned int ctrl = par->base_addr + SPDIF_CTRL_OFFSET;

	spdif->ctrl &= ~(1 << CTRL_DECODE_ENB_POS);
	writel(spdif->ctrl, ctrl);

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
	unsigned int format = params_format(params);

	switch (format) {
	case SNDRV_PCM_FORMAT_S16_LE:
		pr_debug("spdiftx: change sample bits S16\n");
		spdif->ctrl &= ~((0xF<<CTRL_VALID_SAMPLE_POS) | (1<<CTRL_DMA_DATA_POS));
		spdif->ctrl |= (CTRL_VALID_SAMPLE_8<<CTRL_VALID_SAMPLE_POS) | (1<<CTRL_DMA_DATA_POS);
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		pr_debug("spdiftx: change sample bits S24\n");
		spdif->ctrl &= ~((0xF<<CTRL_VALID_SAMPLE_POS) | (1<<CTRL_DMA_DATA_POS));
		spdif->ctrl |= (CTRL_VALID_SAMPLE_0<<CTRL_VALID_SAMPLE_POS) | (0<<CTRL_DMA_DATA_POS);
		break;
	default:
		return -EINVAL;
	}
	return 0;
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
	unsigned int ctrl = par->base_addr + SPDIF_CTRL_OFFSET;

	PM_DBGOUT("%s\n", __func__);

	spdif_reset(par);
	writel(spdif->ctrl, ctrl);
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
	.capture	= {
		.channels_min 	= 2,
		.channels_max 	= 2,
		.formats		= SND_SOC_SPDIF_RX_FORMATS,
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
	.name 	= DEV_NAME_SPDIF_RX,
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
MODULE_DESCRIPTION("Sound S/PDIF rx driver for the SLSI");
MODULE_LICENSE("GPL");




