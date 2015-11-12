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

#ifdef CONFIG_NXP_DFS_BCLK
#include <mach/nxp-dfs-bclk.h>
#endif

#include "nxp-i2s.h"

/*
#define pr_debug				printk
*/

#if defined(CONFIG_SND_NXP_DFS)
#define SND_NXP_DFS_PLLNO		5 // virtual no
#endif

#define	DEF_SAMPLE_RATE			48000
#define	DEF_FRAME_BIT			32	// 32, 48 	 (BFS)

#define	I2S_BASEADDR			PHY_BASEADDR_I2S0
#define	I2S_CH_OFFSET			0x1000
#define	I2S_BUS_WIDTH			4	// Byte
#define	I2S_PERI_BURST			4	// Byte
#define	I2S_MAX_CLOCK			166000000

#define	I2S0_PRESETn 			(23)
#define	I2S1_PRESETn 			(24)
#define	I2S2_PRESETn 			(25)

/*
 * I2S register
 */
struct i2s_register {
	unsigned int CON;		///< 0x00 :
	unsigned int CSR;		///< 0x04 :
	unsigned int FIC;		///< 0x08 :
#if defined(CONFIG_ARCH_S5P4418)
	unsigned int PSR;		///< 0x0C :
#endif
};

#define	I2S_CON_OFFSET			(0x00)
#define	I2S_CSR_OFFSET			(0x04)
#define	I2S_FIC_OFFSET			(0x08)
#if defined(CONFIG_ARCH_S5P4418)
#define	I2S_PSR_OFFSET			(0x0C)
#endif
#define	I2S_TXD_OFFSET			(0x10)
#define	I2S_RXD_OFFSET			(0x14)

#define	CON_TXDMAPAUSE_POS		6		// [6]
#define	CON_RXDMAPAUSE_POS		5		// [5]
#define	CON_TXCHPAUSE_POS		4		// [4]
#define	CON_RXCHPAUSE_POS		3		// [3]
#define	CON_TXDMACTIVE_POS		2		// [2]
#define	CON_RXDMACTIVE_POS		1		// [1]
#define	CON_I2SACTIVE_POS		0		// [0]

#define	CSR_BLC_POS				13		// [13:14]
#define	CSR_CDCLKCON_POS		12		// [12]
#if defined(CONFIG_ARCH_S5P4418)
#define	CSR_IMS_POS				10		// [10:11]
#elif defined(CONFIG_ARCH_S5P6818)
#define	CSR_IMS_POS				11		// [11]
#endif
#define	CSR_TXR_POS			 	8		// [08:09]
#define	CSR_LRP_POS			 	7 		// [7]
#define	CSR_SDF_POS			 	5		// [06:06]
#define	CSR_RFS_POS			 	3		// [03:04]
#define	CSR_BFS_POS			 	1		// [01:02]

#if defined(CONFIG_ARCH_S5P4418)
#define	PSR_PSRAEN_POS	   	   	15		// [15]
#define	PSR_PSVALA_POS		 	8		// [08:13]
#endif

#if defined(CONFIG_ARCH_S5P4418)
#define	IMS_BIT_PCLK			(0<<0)
#define	IMS_BIT_EXTCLK			(1<<0)
#define	IMS_BIT_SLAVE_PCLK		(2<<0)
#define	IMS_BIT_SLAVE			(3<<0)
#elif defined(CONFIG_ARCH_S5P6818)
#define	IMS_BIT_EXTCLK			(0<<0)
#define	IMS_BIT_SLAVE			(1<<0)
#endif

#define BLC_8BIT				1
#define BLC_16BIT				0
#define BLC_24BIT				2

#define BFS_16BIT				2
#define BFS_24BIT				3
#define BFS_32BIT				0
#define BFS_48BIT				1

#define RATIO_256				0
#define RATIO_384				2

struct clock_ratio {
	unsigned int sample_rate;
	unsigned int ratio_256;
	unsigned int ratio_384;
};

static struct clock_ratio clk_ratio [] = {
	{   8000,  2048000,  3072000 },
	{  11025,  2822400,  4233600 },
	{  16000,  4096000,  6144000 },
	{  22050,  5644800,  8467200 },
	{  32000,  8192000, 12288000 },
	{  44100, 11289600, 16934400 },
	{  48000, 12288000, 18432000 },
	{  64000, 16384000, 24576000 },
	{  88200, 22579200, 33868800 },
	{  96000, 24576000, 36864000 },
	{ 176400, 45158400, 67737600 },
	{ 192000, 49152000, 73728000 },
};

/*
 * parameters
 */
struct nxp_i2s_snd_param {
	int channel;
    int master_mode;   	/* 0 = master_mode, 1 = slave */
    int mclk_in;
    int trans_mode; 	/* 0 = I2S, 2 = Left-Justified, 3 = Right-Justified  */
    int sample_rate;
    int	frame_bit;		/* 16, 24, 32, 48 */
	int LR_pol_inv;
	int in_clkgen;
	int pre_supply_mclk;
	bool ext_is_en;
	unsigned long (*set_ext_mclk)(unsigned long clk, int ch);	
	int	status;
	spinlock_t	lock;
	unsigned long flags;
	/* clock control */
	struct clk *clk;
	long clk_rate;
	/* DMA channel */
	struct nxp_pcm_dma_param play;
	struct nxp_pcm_dma_param capt;
	/* Register */
	unsigned int base_addr;
	struct i2s_register i2s;
};

#define	SND_I2S_LOCK_INIT(x)		spin_lock_init(x)
#define	SND_I2S_LOCK(x, f)		spin_lock_irqsave(x, f);
#define	SND_I2S_UNLOCK(x, f)		spin_unlock_irqrestore(x, f);

#if !defined(CONFIG_SND_NXP_DFS)
/*
 * return 0 = clkgen, 1 = peripheral clock
 */
static int set_sample_rate_clock(struct clk *clk, unsigned long request,
				unsigned long *rate_hz, int *prescale)
{
	struct clk *pclk = clk_get(NULL, "pclk");	/* get peripheral clock */
	unsigned long peri_hz = clk_get_rate(pclk);
	unsigned long find, rate = 0, clock = request;
	int dio = 0, din = 0, div = 1;
	int ret = 1, i = 0;	/* peripheral clock */

	/* form peripheral clock */
	div  = peri_hz/clock;
	rate = peri_hz/div;
	dio  = abs(rate - clock);
	if (0 == dio)
		goto done;

	/* form clock generator */
	for (i = 1; 65 > i; i++, clock = i*request) {
		find = clk_set_rate(clk, clock);
		if (find > I2S_MAX_CLOCK)
			break;
		din = abs((clock/i) - (find/i));
		if (dio >= din) {
			dio = din, rate = find/i;
			div = i, ret = 0;
			if (0 == din)
				break;
		}
	}

done:
	pr_debug("%s: req=%ld, acq=%ld, div=%2d, %s\n",
		__func__, request, rate, div, ret==1?"periclock":"clkgen");

	if (rate_hz)
		*rate_hz = rate;
	if (prescale)
		*prescale = div;

	/* release peripheral clock */
	clk_put(pclk);
	return ret;
}
#endif // #if !defined(CONFIG_SND_NXP_DFS)

static void supply_master_clock(struct nxp_i2s_snd_param *par)
{
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;

	pr_debug("%s: %s (status=0x%x)\n",
		__func__, par->master_mode?"mater":"slave", par->status);

	if (!par->master_mode ||
		(SNDDEV_STATUS_POWER & par->status))
		return;

	if (par->in_clkgen)
		clk_enable(par->clk);

	i2s->CSR &= ~(1 << CSR_CDCLKCON_POS);
	writel(i2s->CSR, (base+I2S_CSR_OFFSET));

	par->status |= SNDDEV_STATUS_POWER;
}

static void cutoff_master_clock(struct nxp_i2s_snd_param *par)
{
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;

	pr_debug("%s: %s (status=0x%x)\n",
		__func__, par->master_mode?"mater":"slave", par->status);

	if (!par->master_mode ||
		!(SNDDEV_STATUS_POWER & par->status))
		return;

	if (par->in_clkgen)
		clk_disable(par->clk);

	i2s->CSR |= (1 << CSR_CDCLKCON_POS);	/* when high is MCLK OUT enable */
	writel(i2s->CSR, (base+I2S_CSR_OFFSET));

	par->status &= ~SNDDEV_STATUS_POWER;
}

static void inline i2s_reset(struct nxp_i2s_snd_param *par)
{
	nxp_soc_peri_reset_set(RESET_ID_I2S0 + par->channel);
}

static int i2s_start(struct nxp_i2s_snd_param *par, int stream)
{
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;
	unsigned int FIC = 0;

	pr_debug("%s %d\n", __func__, par->channel);

	SND_I2S_LOCK(&par->lock, par->flags);

	if (!par->pre_supply_mclk) {
		if (par->ext_is_en)
		    par->set_ext_mclk(CTRUE, par->channel);
		supply_master_clock(par);
	}

	if (SNDRV_PCM_STREAM_PLAYBACK == stream) {
		FIC |= (1 << 15);	/* flush fifo */
		i2s->CON |=  ((1 << CON_TXDMACTIVE_POS) | (1 << CON_I2SACTIVE_POS));
		i2s->CSR &= ~(3 << CSR_TXR_POS);
		i2s->CSR |=  (0 << CSR_TXR_POS);		/* Transmit only */
		par->status |= SNDDEV_STATUS_PLAY;
	} else {
		FIC |= (1 << 7);	/* flush fifo */
		i2s->CON |=  ((1 << CON_RXDMACTIVE_POS) | (1 << CON_I2SACTIVE_POS));
		i2s->CSR &= ~(3 << CSR_TXR_POS);
		i2s->CSR |=  (1 << CSR_TXR_POS);		/* Receive only */
		par->status |= SNDDEV_STATUS_CAPT;
	}

	if ((par->status & SNDDEV_STATUS_PLAY) &&
		(par->status & SNDDEV_STATUS_CAPT)) {
		i2s->CSR &= ~(3 << CSR_TXR_POS);
		i2s->CSR |=  (2 << CSR_TXR_POS);	/* Transmit and Receive simultaneous mode */
	}

	writel(FIC, (base+I2S_FIC_OFFSET));	/* Flush the current FIFO */
	writel(0x0, (base+I2S_FIC_OFFSET));	/* Clear the Flush bit */

	writel(i2s->CSR, (base+I2S_CSR_OFFSET));
	writel(i2s->CON, (base+I2S_CON_OFFSET));

	SND_I2S_UNLOCK(&par->lock, par->flags);

	return 0;
}

static void i2s_stop(struct nxp_i2s_snd_param *par, int stream)
{
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;

	pr_debug("%s %d\n", __func__, par->channel);

	SND_I2S_LOCK(&par->lock, par->flags);

	if (SNDRV_PCM_STREAM_PLAYBACK == stream) {
		par->status &= ~SNDDEV_STATUS_PLAY;
		i2s->CON &= ~(1 << CON_TXDMACTIVE_POS);
		i2s->CSR &= ~(3 << CSR_TXR_POS);
		i2s->CSR |=  (1 << CSR_TXR_POS);		/* Receive only */
	} else {
		par->status &= ~SNDDEV_STATUS_CAPT;
		i2s->CON &= ~(1 << CON_RXDMACTIVE_POS);
		i2s->CSR &= ~(3 << CSR_TXR_POS);
		i2s->CSR |=  (0 << CSR_TXR_POS);		/* Transmit only */
	}

	if (!(par->status & SNDDEV_STATUS_RUNNING)) {
		if (!par->pre_supply_mclk)
			i2s->CON &= ~(1 << CON_I2SACTIVE_POS);
		i2s->CSR |=  (3 << CSR_TXR_POS);		/* no tx/rx */
	}

	writel(i2s->CON, (base+I2S_CON_OFFSET));
	writel(i2s->CSR, (base+I2S_CSR_OFFSET));

	/* I2S Inactive */
	if (!(par->status & SNDDEV_STATUS_RUNNING) &&
		!par->pre_supply_mclk) {
		cutoff_master_clock(par);
		if (par->ext_is_en)
		    par->set_ext_mclk(CFALSE, par->channel);
	}

	SND_I2S_UNLOCK(&par->lock, par->flags);
}

static struct snd_soc_dai_driver i2s_dai_driver;

static int nxp_i2s_check_param(struct nxp_i2s_snd_param *par)
{
#if !defined(CONFIG_SND_NXP_DFS)
	static struct snd_soc_dai_driver *dai = &i2s_dai_driver;
	int divide = 0;
#endif
	struct i2s_register *i2s = &par->i2s;
	struct nxp_pcm_dma_param *dmap_play = &par->play;
	struct nxp_pcm_dma_param *dmap_capt = &par->capt;
	unsigned int base = par->base_addr;
	unsigned long request = 0, rate_hz = 0;
	int i = 0;
#if defined(CONFIG_ARCH_S5P4418) && !defined(CONFIG_SND_NXP_DFS)
	int prescale = 0, en_pclk = 0, PSRAEN = 0;
#endif

	int LRP, IMS, BLC = BLC_16BIT, BFS = 0, RFS = RATIO_256;
	int SDF = 0, OEN = 0;

	IMS = par->master_mode ? 0 : IMS_BIT_SLAVE;
	SDF = par->trans_mode & 0x03;	/* 0:I2S, 1:Left 2:Right justfied */
	LRP = par->LR_pol_inv ? 1 : 0;
	OEN = !par->master_mode ? 1 : par->mclk_in; /* Active low : MLCK out enable */

	switch (par->frame_bit) {
	case 16: BFS = BFS_16BIT;BLC = BLC_8BIT;  break;
	case 32: BFS = BFS_32BIT;BLC = BLC_16BIT; break;
	case 48: BFS = BFS_48BIT;BLC = BLC_24BIT; break;
	default:
		printk(KERN_ERR "Fail, not support i2s frame bits %d (32, 48)\n",
			par->frame_bit);
		return -EINVAL;
	}

	for (i = 0; ARRAY_SIZE(clk_ratio) > i; i++) {
		if (par->sample_rate == clk_ratio[i].sample_rate)
			break;
	}

	if (i >= ARRAY_SIZE(clk_ratio)) {
		printk(KERN_ERR "Fail, not support i2s sample rate %d \n",
			par->sample_rate);
		return -EINVAL;
	}

	if (!par->master_mode){ 
	 	/* 384 RATIO */
		RFS = RATIO_384, request = clk_ratio[i].ratio_384;
		/* 256 RATIO */
		if (BFS_32BIT == BFS)
			RFS = RATIO_256, request = clk_ratio[i].ratio_256;
		goto done;
	}

	if (par->ext_is_en) {
		if (BFS_48BIT == BFS)
			request = clk_ratio[i].ratio_384;
		else	
			request = clk_ratio[i].ratio_256;
	    par->set_ext_mclk(request, par->channel);
	} 
#if defined(CONFIG_SND_NXP_DFS)
	else {
		if (BFS_48BIT == BFS) {
			request = par->sample_rate * 384;RFS = RATIO_384;
		} else {
			request = par->sample_rate * 256;RFS = RATIO_256;
		}
		rate_hz = request;
	
		cutoff_master_clock(par);
		clk_disable(par->clk);
		nxp_cpu_pll_change_frequency(SND_NXP_DFS_PLLNO, request);
#if defined(CONFIG_ARCH_S5P4418)
		nxp_cpu_clock_update_rate(CONFIG_SND_NXP_PLLDEV);
#elif defined(CONFIG_ARCH_S5P6818)
		nxp_cpu_clock_update_pll(CONFIG_SND_NXP_PLLDEV);
#endif
		clk_set_rate(par->clk, request);
		clk_enable(par->clk);
		supply_master_clock(par);
	}
	par->in_clkgen = 1;
	IMS |= IMS_BIT_EXTCLK;
#else
 	/* 384 RATIO */
	RFS = RATIO_384, request = clk_ratio[i].ratio_384;
#if defined(CONFIG_ARCH_S5P4418)
	en_pclk = set_sample_rate_clock(par->clk, request, &rate_hz, &divide);
#elif defined(CONFIG_ARCH_S5P6818)
	set_sample_rate_clock(par->clk, request, &rate_hz, &divide);
#endif

	/* 256 RATIO */
	if (rate_hz != request && BFS_32BIT == BFS) {
		unsigned int rate = rate_hz, div = divide;
#if defined(CONFIG_ARCH_S5P4418)
		unsigned int pclk = en_pclk;
#endif
		RFS = RATIO_256, request = clk_ratio[i].ratio_256;
#if defined(CONFIG_ARCH_S5P4418)
		en_pclk = set_sample_rate_clock(par->clk, request, &rate_hz, &divide);
#elif defined(CONFIG_ARCH_S5P6818)
		set_sample_rate_clock(par->clk, request, &rate_hz, &divide);
#endif
		if (abs(request - rate_hz) >
			abs(request - rate)) {
			rate_hz = rate, divide = div, RFS = RATIO_384;
#if defined(CONFIG_ARCH_S5P4418)
			en_pclk = pclk;
#endif
		}
	}

	/* input clock */
#if defined(CONFIG_ARCH_S5P4418)
	if (!en_pclk) {
		par->clk_rate = clk_set_rate(par->clk, rate_hz);
		par->in_clkgen = 1;
		IMS |= IMS_BIT_EXTCLK;
	} else {
		if (par->mclk_in == 1) {
			par->in_clkgen = 1;
			IMS |= IMS_BIT_EXTCLK;
		} else {
			struct clk *pclk = clk_get(NULL, "pclk");
			clk_enable(pclk);
			par->clk_rate = 0;
			par->in_clkgen = 0;
			IMS = IMS_BIT_PCLK;
		}
	}

	if (en_pclk) {
		PSRAEN = 1;
		prescale = divide - 1;
	}
#elif defined(CONFIG_ARCH_S5P6818)
	par->clk_rate = clk_set_rate(par->clk, rate_hz);
	par->in_clkgen = 1;
#endif
#endif // #if defined(CONFIG_SND_NXP_DFS)

done:
	i2s->CSR = 	(BLC << CSR_BLC_POS) |
				(OEN << CSR_CDCLKCON_POS) |
				(IMS << CSR_IMS_POS) |
				(LRP << CSR_LRP_POS) |
				(SDF << CSR_SDF_POS) |
				(RFS << CSR_RFS_POS) |
				(BFS << CSR_BFS_POS);
#if defined(CONFIG_ARCH_S5P4418) && !defined(CONFIG_SND_NXP_DFS)
	i2s->PSR = 	((PSRAEN &0x1) << PSR_PSRAEN_POS) | ((prescale & 0x3f) << PSR_PSVALA_POS);
#endif
	i2s_reset(par);

	if (par->pre_supply_mclk) {
		if (par->ext_is_en)
	    	rate_hz = par->set_ext_mclk(CTRUE, par->channel);
		supply_master_clock(par);
		i2s->CON |=  1 << CON_I2SACTIVE_POS;
		writel(i2s->CON, (base+I2S_CON_OFFSET));
	} else {
		if (par->ext_is_en) {
	    	rate_hz = par->set_ext_mclk(CTRUE, par->channel);
	    	par->set_ext_mclk(CFALSE, par->channel);
		}
	}

	dmap_play->real_clock = rate_hz/(RATIO_256==RFS?256:384);
	dmap_capt->real_clock = rate_hz/(RATIO_256==RFS?256:384);
	printk("snd i2s: ch %d, %s, %s mode, %d(%ld)hz, %d FBITs, MCLK=%ldhz, RFS=%d\n",
		par->channel, par->master_mode?"master":"slave",
		par->trans_mode==0?"iis":(par->trans_mode==1?"left justified":"right justified"),
		par->sample_rate, rate_hz/(RATIO_256==RFS?256:384),
		par->frame_bit, rate_hz, (RATIO_256==RFS?256:384));
	pr_debug("snd i2s: BLC=%d, IMS=%d, LRP=%d, SDF=%d, RFS=%d, BFS=%d\n", BLC, IMS, LRP, SDF, RFS, BFS);
#if defined(CONFIG_ARCH_S5P4418) && !defined(CONFIG_SND_NXP_DFS)
	pr_debug("snd i2s: PSRAEN=%d, PSVALA=%d \n", PSRAEN, prescale);
#endif
#if !defined(CONFIG_SND_NXP_DFS)
	/* i2s support format */
	if (RFS == RATIO_256 || BFS != BFS_48BIT) {
		dai->playback.formats &= ~(SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_U24_LE);
		dai->capture.formats  &= ~(SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_U24_LE);
	}
#endif
	return 0;
}

static int nxp_i2s_set_plat_param(struct nxp_i2s_snd_param *par, void *data)
{
	struct platform_device *pdev = data;
	struct nxp_i2s_plat_data *plat = pdev->dev.platform_data;
	struct nxp_pcm_dma_param *dma = &par->play;
	unsigned int phy_base = I2S_BASEADDR + (pdev->id * I2S_CH_OFFSET);
	int i = 0, ret = 0;

	par->channel = pdev->id;
    par->master_mode = plat->master_mode;
    par->mclk_in = plat->master_clock_in;
    par->trans_mode = plat->trans_mode;
    par->sample_rate = plat->sample_rate ? plat->sample_rate : DEF_SAMPLE_RATE;
    par->frame_bit = plat->frame_bit ? plat->frame_bit : DEF_FRAME_BIT;
    par->LR_pol_inv = plat->LR_pol_inv;
    par->pre_supply_mclk = plat->pre_supply_mclk;
	if (plat->ext_is_en) {
		par->ext_is_en = plat->ext_is_en();
		par->mclk_in = par->ext_is_en ? 1 : plat->master_clock_in;
	} else {
		par->ext_is_en = 0;
	}
	if (plat->set_ext_mclk)
		par->set_ext_mclk = plat->set_ext_mclk;
	par->base_addr = IO_ADDRESS(phy_base);
	SND_I2S_LOCK_INIT(&par->lock);

    for (i = 0; 2 > i; i++, dma = &par->capt) {
		if (! plat->dma_play_ch) {
			dma->active = false;
			continue;
		}

		dma->active = true;
		dma->dma_filter = plat->dma_filter;
		dma->dma_ch_name = (char*)(i == 0 ? plat->dma_play_ch :	plat->dma_capt_ch);
		dma->peri_addr = phy_base + (i == 0 ? I2S_TXD_OFFSET : I2S_RXD_OFFSET);	/* I2S TXD/RXD */
		dma->bus_width_byte = I2S_BUS_WIDTH;
		dma->max_burst_byte = I2S_PERI_BURST;
		pr_debug("snd i2s: %s dma (%s, peri 0x%x, bus %dbits)\n",
			STREAM_STR(i), dma->dma_ch_name, dma->peri_addr, dma->bus_width_byte*8);
	}

	par->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(par->clk)) {
		ret = PTR_ERR(par->clk);
		return ret;
	}

	return nxp_i2s_check_param(par);
}

static int nxp_i2s_setup(struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;

	SND_I2S_LOCK(&par->lock, par->flags);

	if (SNDDEV_STATUS_SETUP & par->status) {
		SND_I2S_UNLOCK(&par->lock, par->flags);
		return 0;
	}

	writel(i2s->CSR, (base+I2S_CSR_OFFSET));
#if defined(CONFIG_ARCH_S5P4418)
	writel(i2s->PSR, (base+I2S_PSR_OFFSET));
#endif

	par->status |= SNDDEV_STATUS_SETUP;

	SND_I2S_UNLOCK(&par->lock, par->flags);

	return 0;
}

static void nxp_i2s_release(struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;

	SND_I2S_LOCK(&par->lock, par->flags);

	i2s->CON =~((1 << CON_TXDMAPAUSE_POS) | (1 << CON_RXDMAPAUSE_POS) |
				(1 << CON_TXCHPAUSE_POS) | (1 << CON_RXCHPAUSE_POS) |
				(1 << CON_TXDMACTIVE_POS) | (1 << CON_RXDMACTIVE_POS) |
				(1 << CON_I2SACTIVE_POS) );

	writel(i2s->CON, (base+I2S_CON_OFFSET));

	cutoff_master_clock(par);
	clk_put(par->clk);

	par->status = SNDDEV_STATUS_CLEAR;

	SND_I2S_UNLOCK(&par->lock, par->flags);
}

/*
 * snd_soc_dai_ops
 */
static int nxp_i2s_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct nxp_pcm_dma_param *dmap =
		SNDRV_PCM_STREAM_PLAYBACK == substream->stream ? &par->play : &par->capt;

	snd_soc_dai_set_dma_data(dai, substream, dmap);
	return 0;
}

static void nxp_i2s_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	i2s_stop(par, substream->stream);
}

static int nxp_i2s_trigger(struct snd_pcm_substream *substream,
				int cmd, struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	int stream = substream->stream;
	pr_debug("%s: %s cmd=%d\n", __func__, STREAM_STR(stream), cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	case SNDRV_PCM_TRIGGER_START:
#ifdef CONFIG_NXP_DFS_BCLK
        bclk_get(BCLK_USER_DMA);
#endif
		i2s_start(par, stream);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
    case SNDRV_PCM_TRIGGER_STOP:
		i2s_stop(par, stream);
#ifdef CONFIG_NXP_DFS_BCLK
        bclk_put(BCLK_USER_DMA);
#endif
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

#if defined(CONFIG_SND_NXP_DFS)
static int nxp_i2s_set_dai_sysclk(struct snd_soc_dai *cpu_dai,
                                      int clk_id, unsigned int freq, int dir)
{
	struct nxp_i2s_snd_param 	*par = snd_soc_dai_get_drvdata(cpu_dai);
	struct i2s_register *i2s = &par->i2s;
	int LRP, IMS, BFS = 0;
	int SDF = 0;
	BFS = (i2s->CSR >> CSR_BFS_POS) & 0x3;

	IMS = par->master_mode ? 0 : IMS_BIT_SLAVE;
	SDF = par->trans_mode & 0x03;	/* 0:I2S, 1:Left 2:Right justfied */
	LRP = par->LR_pol_inv ? 1 : 0;

    pr_debug(" %s, clk_id %d, sample_rate = %u, dir = %d\n", __func__, clk_id, freq, dir);

	par->sample_rate = freq;

    return 0;
}

static int nxp_i2s_set_dfs_sysclk(struct nxp_i2s_snd_param *par)
{
	struct i2s_register *i2s = &par->i2s;
	struct nxp_pcm_dma_param *dmap_play = &par->play;
	struct nxp_pcm_dma_param *dmap_capt = &par->capt;
	unsigned int base = par->base_addr;
	unsigned long request = 0, rate_hz = 0;
	int i = 0;
	int LRP, IMS, BFS = 0, RFS = RATIO_256;
	int SDF = 0, OEN = 0;
	int BLC = (i2s->CSR >> CSR_BLC_POS) & 0x3;

	BFS = (i2s->CSR >> CSR_BFS_POS) & 0x3;
	IMS = par->master_mode ? 0 : IMS_BIT_SLAVE;
	SDF = par->trans_mode & 0x03;	/* 0:I2S, 1:Left 2:Right justfied */
	LRP = par->LR_pol_inv ? 1 : 0;

	switch (BFS) {
		case BFS_16BIT: par->frame_bit = 16;pr_debug("==BFS 16BIT==\n"); break;
		case BFS_32BIT: par->frame_bit = 32;pr_debug("==BFS 32BIT==\n"); break;
		case BFS_48BIT: par->frame_bit = 48;pr_debug("==BFS 48BIT==\n"); break;
		default:
			printk(KERN_ERR "Fail, not support i2s frame bits %d (32, 48)\n",
				BFS);
		return -EINVAL;
	}

	for (i = 0; ARRAY_SIZE(clk_ratio) > i; i++) {
		if (par->sample_rate == clk_ratio[i].sample_rate)
			break;
	}

	if (i >= ARRAY_SIZE(clk_ratio)) {
		printk(KERN_ERR "Fail, not support i2s sample rate %d \n",
			par->sample_rate);
		return -EINVAL;
	}

	if (!par->master_mode){ 
	 	/* 384 RATIO */
		RFS = RATIO_384, request = clk_ratio[i].ratio_384;
		/* 256 RATIO */
		if (BFS_32BIT == BFS)
			RFS = RATIO_256, request = clk_ratio[i].ratio_256;
		goto done;
	}

	if (par->ext_is_en) {
		if (BFS_48BIT == BFS)
			request = clk_ratio[i].ratio_384;
		else	
			request = clk_ratio[i].ratio_256;
	    par->set_ext_mclk(request, par->channel);
	} else {
		if (BFS_48BIT == BFS) {
			request = par->sample_rate * 384;RFS = RATIO_384;
		} else {
			request = par->sample_rate * 256;RFS = RATIO_256;
		}
		rate_hz = request;
	
		cutoff_master_clock(par);
		clk_disable(par->clk);
		nxp_cpu_pll_change_frequency(SND_NXP_DFS_PLLNO, request);
#if defined(CONFIG_ARCH_S5P4418)
		nxp_cpu_clock_update_rate(CONFIG_SND_NXP_PLLDEV);
#elif defined(CONFIG_ARCH_S5P6818)
		nxp_cpu_clock_update_pll(CONFIG_SND_NXP_PLLDEV);
#endif
		clk_set_rate(par->clk, request);
		clk_enable(par->clk);
		supply_master_clock(par);
	}
	par->in_clkgen = 1;
	IMS |= IMS_BIT_EXTCLK;

done:
	i2s->CSR =  (BLC << CSR_BLC_POS) |
				(OEN << CSR_CDCLKCON_POS) |
				(IMS << CSR_IMS_POS) |
				(LRP << CSR_LRP_POS) |
				(SDF << CSR_SDF_POS) |
				(RFS << CSR_RFS_POS) |
				(BFS << CSR_BFS_POS);

	i2s_reset(par);

	if (par->pre_supply_mclk) {
		if (par->ext_is_en)
	    	rate_hz = par->set_ext_mclk(CTRUE, par->channel);
		supply_master_clock(par);
		i2s->CON |=  1 << CON_I2SACTIVE_POS;
		writel(i2s->CON, (base+I2S_CON_OFFSET));
	} else {
		if (par->ext_is_en) {
	    	rate_hz = par->set_ext_mclk(CTRUE, par->channel);
	    	par->set_ext_mclk(CFALSE, par->channel);
		}
	}

	dmap_play->real_clock = rate_hz/(RATIO_256==RFS?256:384);
	dmap_capt->real_clock = rate_hz/(RATIO_256==RFS?256:384);
	pr_debug("snd i2s: ch %d, %s, %s mode, %d(%ld)hz, %d FBITs, MCLK=%ldhz, RFS=%d\n",
		par->channel, par->master_mode?"master":"slave",
		par->trans_mode==0?"iis":(par->trans_mode==1?"left justified":"right justified"),
		par->sample_rate, rate_hz/(RATIO_256==RFS?256:384),
		par->frame_bit, rate_hz, (RATIO_256==RFS?256:384));
	pr_debug("snd i2s: BLC=%d, IMS=%d, LRP=%d, SDF=%d, RFS=%d, BFS=%d\n", BLC, IMS, LRP, SDF, RFS, BFS);

    return 0;
}

static int nxp_i2s_set_dai_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
    pr_debug(" %s \n", __func__);

    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
		case SND_SOC_DAIFMT_CBS_CFS:
			pr_debug("SND_SOC_DAIFMT_CBS_CFS\n");
			break;
		case SND_SOC_DAIFMT_CBM_CFS:
			pr_debug("SND_SOC_DAIFMT_CBM_CFS\n");
			break;
		default:
			pr_debug("SND_SOC_DAIFMT_MASTER_MASK Error\n");
			break;
    }

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:
			pr_debug("SND_SOC_DAIFMT_I2S\n");
			break;
		case SND_SOC_DAIFMT_LEFT_J:
			pr_debug("SND_SOC_DAIFMT_LEFT_J\n");
			break;
		case SND_SOC_DAIFMT_DSP_A:
			pr_debug("SND_SOC_DAIFMT_DSP_A\n");
			break;
		case SND_SOC_DAIFMT_DSP_B:
			pr_debug("SND_SOC_DAIFMT_DSP_B\n");
			break;
		default:
			pr_debug("SND_SOC_DAIFMT_FORMAT_MASK Error\n");
			return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			pr_debug("SND_SOC_DAIFMT_NB_NF\n");
			break;
		case SND_SOC_DAIFMT_IB_NF:
			pr_debug("SND_SOC_DAIFMT_IB_NF\n");
			break;
		default:
			pr_debug("SND_SOC_DAIFMT_INV_MASK Error\n");
			return -EINVAL;
	}

    return 0;
}
#endif

static int nxp_i2s_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct i2s_register *i2s = &par->i2s;
	unsigned int format = params_format(params);
#if !defined(CONFIG_SND_NXP_DFS)
	int RFS = (i2s->CSR >> CSR_RFS_POS) & 0x3;
	int BFS = (i2s->CSR >> CSR_BFS_POS) & 0x3;
#endif
	int BLC = (i2s->CSR >> CSR_BLC_POS) & 0x3;
	int ret = 0;

	pr_debug("nxp_i2s_hw_params, %d\n", format);

	switch (format) {
	case SNDRV_PCM_FORMAT_S8:
	case SNDRV_PCM_FORMAT_U8:
		pr_debug("i2s: change sample bits S08\n");
		if (BLC != BLC_8BIT) {
			i2s->CSR &= ~(0x3 << CSR_BLC_POS);
			i2s->CSR |=  (BLC_8BIT << CSR_BLC_POS);
		}
#if defined(CONFIG_SND_NXP_DFS)
		i2s->CSR &= ~(0x3 << CSR_BFS_POS);
		i2s->CSR |=  (BFS_16BIT << CSR_BFS_POS);
#endif
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		pr_debug("i2s: change i2s sample bits %s -> S16\n", BLC==BLC_16BIT?"S16":"S24");
		if (BLC != BLC_16BIT) {
			i2s->CSR &= ~(0x3 << CSR_BLC_POS);
			i2s->CSR |=  (BLC_16BIT << CSR_BLC_POS);
		}
#if defined(CONFIG_SND_NXP_DFS)
		i2s->CSR &= ~(0x3 << CSR_BFS_POS);
		i2s->CSR |=  (BFS_32BIT << CSR_BFS_POS);
#endif
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		pr_debug("i2s: change i2s sample bits %s -> S24\n", BLC==BLC_16BIT?"S16":"S24");
#if !defined(CONFIG_SND_NXP_DFS)
		if (RFS == RATIO_256 || BFS != BFS_48BIT) {
			printk(KERN_ERR "Fail, i2s RFS 256/BFS 32 not support 24 sample bits\n");
			return -EINVAL;
		}
#endif
		if (BLC != BLC_24BIT) {
			pr_debug("i2s: BLC != BLC_24BIT\n");
			i2s->CSR &= ~(0x3 << CSR_BLC_POS);
			i2s->CSR |=  (BLC_24BIT << CSR_BLC_POS);
		}
		else
			pr_debug("i2s: BLC == BLC_24BIT\n");
#if defined(CONFIG_SND_NXP_DFS)
		i2s->CSR &= ~(0x3 << CSR_BFS_POS);
		i2s->CSR |=  (BFS_48BIT << CSR_BFS_POS);
#endif
		break;
	default:
		pr_debug("i2s: default %d\n", format);
		return -EINVAL;
	}
#if defined(CONFIG_SND_NXP_DFS)
	nxp_i2s_set_dfs_sysclk(par);
#endif

	return ret;
}

static struct snd_soc_dai_ops nxp_i2s_ops = {
#if defined(CONFIG_SND_NXP_DFS)
    .set_sysclk = nxp_i2s_set_dai_sysclk,
    .set_fmt    = nxp_i2s_set_dai_fmt,
#endif
	.startup	= nxp_i2s_startup,
	.shutdown	= nxp_i2s_shutdown,
	.trigger	= nxp_i2s_trigger,
	.hw_params	= nxp_i2s_hw_params,
};

/*
 * snd_soc_dai_driver
 */
static int nxp_i2s_dai_suspend(struct snd_soc_dai *dai)
{
	PM_DBGOUT("%s\n", __func__);
	return 0;
}

static int nxp_i2s_dai_resume(struct snd_soc_dai *dai)
{
	struct nxp_i2s_snd_param *par = snd_soc_dai_get_drvdata(dai);
	struct i2s_register *i2s = &par->i2s;
	unsigned int base = par->base_addr;
	unsigned int FIC = 0;

	PM_DBGOUT("%s\n", __func__);

	i2s_reset(par);

	if (par->pre_supply_mclk && par->ext_is_en)
	    par->set_ext_mclk(CTRUE, par->channel);

	if (par->master_mode && par->in_clkgen) {
		clk_set_rate(par->clk, par->clk_rate);
		if(SNDDEV_STATUS_POWER & par->status)
			clk_enable(par->clk);
	}

	/* flush fifo */
	FIC |= (par->status & SNDDEV_STATUS_PLAY) ? (1<<15) : 0;
	FIC |= (par->status & SNDDEV_STATUS_CAPT) ? (1<< 7) : 0;

	writel(FIC, (base+I2S_FIC_OFFSET));	/* Flush the current FIFO */
	writel(0x0, (base+I2S_FIC_OFFSET));	/* Clear the Flush bit */
	writel(i2s->CSR, (base+I2S_CSR_OFFSET));
#if defined(CONFIG_ARCH_S5P4418)
	writel(i2s->PSR, (base+I2S_PSR_OFFSET));
#endif
	writel(i2s->CON, (base+I2S_CON_OFFSET));

	return 0;
}

static int nxp_i2s_dai_probe(struct snd_soc_dai *dai)
{
	return nxp_i2s_setup(dai);
}

static int nxp_i2s_dai_remove(struct snd_soc_dai *dai)
{
	nxp_i2s_release(dai);
	return 0;
}

static struct snd_soc_dai_driver i2s_dai_driver = {
	.probe 		= nxp_i2s_dai_probe,
	.remove		= nxp_i2s_dai_remove,
	.suspend	= nxp_i2s_dai_suspend,
	.resume 	= nxp_i2s_dai_resume,
	.playback	= {
		.channels_min 	= 2,
		.channels_max 	= 2,
		.formats		= SND_SOC_I2S_FORMATS,
		.rates			= SND_SOC_I2S_RATES,
		.rate_min 		= 0,
		.rate_max 		= 1562500,
		},
	.capture 	= {
		.channels_min 	= 2,
		.channels_max 	= 2,
		.formats		= SND_SOC_I2S_FORMATS,
		.rates			= SND_SOC_I2S_RATES,
		.rate_min 		= 0,
		.rate_max 		= 1562500,
		},
	.ops = &nxp_i2s_ops,
	.symmetric_rates = 1,
};

static __devinit int nxp_i2s_probe(struct platform_device *pdev)
{
	struct nxp_i2s_snd_param *par;
	int ret = 0;

    /*  allocate i2c_port data */
    par = kzalloc(sizeof(struct nxp_i2s_snd_param), GFP_KERNEL);
    if (! par) {
        printk(KERN_ERR "fail, %s allocate driver info ...\n", pdev->name);
        return -ENOMEM;
    }

	ret = nxp_i2s_set_plat_param(par, pdev);
	if (ret)
		goto err_out;

	ret = snd_soc_register_dai(&pdev->dev, &i2s_dai_driver);
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

static __devexit int nxp_i2s_remove(struct platform_device *pdev)
{
	struct nxp_i2s_snd_param *par = dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_dai(&pdev->dev);
	if (par)
		kfree(par);
	return 0;
}

static struct platform_driver i2s_driver = {
	.probe  = nxp_i2s_probe,
	.remove = nxp_i2s_remove,
	.driver = {
	.name 	= DEV_NAME_I2S,
	.owner 	= THIS_MODULE,
	},
};

static int __init nxp_i2s_init(void)
{
	return platform_driver_register(&i2s_driver);
}

static void __exit nxp_i2s_exit(void)
{
	platform_driver_unregister(&i2s_driver);
}

module_init(nxp_i2s_init);
module_exit(nxp_i2s_exit);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound I2S driver for the SLSI");
MODULE_LICENSE("GPL");




