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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <mach/platform.h>
#include <mach/devices.h>
#include "nxp-pcm.h"

/*
#define pr_debug		printk
*/

//#define DUMP_DMA_ENABLE
#define	DUMP_DMA_PATH_P			"/tmp/pcm_dma_p.raw"
#define	DUMP_DMA_PATH_C			"/tmp/pcm_dma_c.raw"
#define	DUMP_DMA_TIME			20	/* sec */
#define	DUMP_SAMPLE_COMPLETED
//#define	DUMP_DMA_CONTINUOUS

/*
 * PCM INFO
 */
#define	PERIOD_BYTES_MAX		8192

static struct snd_pcm_hardware nxp_pcm_hardware = {
	.info				= 	SNDRV_PCM_INFO_MMAP |
				    		SNDRV_PCM_INFO_MMAP_VALID |
				    		SNDRV_PCM_INFO_INTERLEAVED	|
				    		SNDRV_PCM_INFO_PAUSE |
				    		SNDRV_PCM_INFO_RESUME,	//  | SNDRV_PCM_INFO_BLOCK_TRANSFER
	.formats			= SND_SOC_PCM_FORMATS,
#if defined(CONFIG_SND_NXP_DFS)
   	.rates        = SNDRV_PCM_RATE_8000_192000,
#endif
	.rate_min			= 8000,
	.rate_max			= 192000,
	.channels_min		= 1,
	.channels_max		= 2,
	.buffer_bytes_max	= 64 * 1024 * 2,
	.period_bytes_min	= 32,
	.period_bytes_max	= PERIOD_BYTES_MAX,
	.periods_min		= 2,
	.periods_max		= 64,
	.fifo_size			= 32,
};
#define	substream_to_prtd(s)	(substream->runtime->private_data)

/*
 * DMA DUMP
 */
#ifdef  DUMP_DMA_ENABLE
static void nxp_pcm_file_mem_allocate(char *filename, struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	int sample_bits = snd_pcm_format_physical_width(params_format(params));
	int sample_rate = params->rate_num;
	int sample_ch = params_channels(params);

	if (prtd->mem_area)
		return;

	prtd->mem_len = (sample_rate * sample_ch * (sample_bits/8)) * DUMP_DMA_TIME;
	prtd->mem_area = (uint)vzalloc(prtd->mem_len);

	/* delete previous file */
#ifndef DUMP_DMA_CONTINUOUS
	if (prtd->mem_area) {
		mm_segment_t old_fs = get_fs();
		set_fs(KERNEL_DS);
		sys_unlink(filename);
		set_fs(old_fs);
	}
#endif
	pr_debug("file mem = 0x%08x (%ld = rate:%d, ch:%d, bits:%d, time:%dsec)\n",
		prtd->mem_area, prtd->mem_len, sample_rate, sample_ch, sample_bits, DUMP_DMA_TIME);
}

static void nxp_pcm_file_mem_free(char *filename, struct snd_pcm_substream *substream)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	if (prtd->mem_area) {
		unsigned int mode = O_CREAT|O_RDWR|O_LARGEFILE|O_APPEND;
   		struct file *filp = filp_open(filename, mode, 0777);
   		if (filp) {
    		loff_t pos = 0;
	    	mm_segment_t old_fs = get_fs();
	    	set_fs (KERNEL_DS);
			vfs_write(filp, (void*)prtd->mem_area, prtd->mem_len, &pos);
			set_fs(old_fs);
			filp_close(filp, NULL);
		}
		vfree((const void *)prtd->mem_area);
	}
	prtd->mem_area = 0;
}

static void nxp_pcm_file_mem_write(struct snd_pcm_substream *substream)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);

	if (prtd->mem_area && substream->runtime) {
		struct snd_pcm_runtime *runtime = substream->runtime;
		unsigned offset = prtd->offset;
		int length = snd_pcm_lib_period_bytes(substream);
		void *dst_addr = (void*)(prtd->mem_area + prtd->mem_offs);
		void *src_addr;

	#ifdef DUMP_SAMPLE_COMPLETED
		if (offset == 0)
			offset = snd_pcm_lib_buffer_bytes(substream);
		offset  -= length;
		src_addr = (void*)(runtime->dma_area + offset);
	#else
		src_addr = (void*)(runtime->dma_area + offset);
	#endif
		memcpy(dst_addr, src_addr, length);
		prtd->mem_offs += length;
		if (prtd->mem_offs >= prtd->mem_len)
			prtd->mem_offs = 0;
	}
}

#else
#define	nxp_pcm_file_mem_allocate(f, s, p)
#define	nxp_pcm_file_mem_free(f, s)
#define	nxp_pcm_file_mem_write(s)
#endif

static void nxp_pcm_dma_clear(struct snd_pcm_substream *substream)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned offset = prtd->offset;
	int length = snd_pcm_lib_period_bytes(substream);
	void *src_addr = NULL;

	if (offset == 0)
			offset = snd_pcm_lib_buffer_bytes(substream);
	offset  -= length;
	src_addr = (void*)(runtime->dma_area + offset);

	if ((prtd->dma_chan->chan_id >= DMA_PERIPHERAL_ID_I2S0_TX) 
		&& (prtd->dma_chan->chan_id <= DMA_PERIPHERAL_ID_I2S2_RX)) {
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			memset(src_addr, 0, length);
		}
	}
}

/*
 * PCM INTERFACE
 */
static void nxp_pcm_dma_complete(void *arg)
{
	struct snd_pcm_substream *substream = arg;
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	long long ts  = prtd->time_stamp_us;
	long long new = ktime_to_us(ktime_get());
	long long period_us = prtd->period_time_us;
	int over_samples = div64_s64((new - ts), period_us);
#if !defined (CONFIG_CPU_S5P4418_SMP_ISR)
	int i;
#endif

	/* i2s master mode */
	if(prtd->dma_param->real_clock != 0) {
		if (2 > over_samples){
			over_samples = 1;
			prtd->time_stamp_us = new;
		} else {
			prtd->time_stamp_us += (over_samples*period_us);
		}
	}

	/*
	if (over_samples > 1)
		printk("[pcm overs :%d]\n", over_samples);
	*/
	/*
	 pr_debug("snd pcm: %s complete offset = %8d (preiodbytes=%d) over samples = %d\n",
	 	STREAM_STR(substream->stream), prtd->offset,
	 	snd_pcm_lib_period_bytes(substream), over_samples);
	*/
	if(prtd->dma_param->real_clock != 0) {
		/* i2s master mode */
#if !defined (CONFIG_CPU_S5P4418_SMP_ISR)
		for (i = 0; over_samples > i; i++) {
#endif
			prtd->offset += snd_pcm_lib_period_bytes(substream);
			if (prtd->offset >= snd_pcm_lib_buffer_bytes(substream))
				prtd->offset = 0;

			nxp_pcm_file_mem_write(substream);
			nxp_pcm_dma_clear(substream);
			snd_pcm_period_elapsed(substream);
#if !defined (CONFIG_CPU_S5P4418_SMP_ISR)
		}
#endif
	} else { /* -> i2s slave mode */
		prtd->offset += snd_pcm_lib_period_bytes(substream);
		if (prtd->offset >= snd_pcm_lib_buffer_bytes(substream))
			prtd->offset = 0;

		nxp_pcm_file_mem_write(substream);
		nxp_pcm_dma_clear(substream);
		snd_pcm_period_elapsed(substream);
	}
}

static int nxp_pcm_dma_request_channel(void *runtime_data, int stream)
{
	struct nxp_pcm_runtime_data *prtd = runtime_data;
	dma_filter_fn filter_fn;
	void *filter_data;
	dma_cap_mask_t mask;

	if (NULL == prtd || NULL == prtd->dma_param)
		return -ENXIO;

	filter_fn   = prtd->dma_param->dma_filter;
	filter_data = prtd->dma_param->dma_ch_name;

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE , mask);
	dma_cap_set(DMA_CYCLIC, mask);
	pr_debug("request %s dma '%s'\n", STREAM_STR(stream), (char*)filter_data);

	prtd->dma_chan = dma_request_channel(mask, filter_fn, filter_data);
	if (!prtd->dma_chan) {
		printk(KERN_ERR "Error: %s dma '%s'\n",
			STREAM_STR(stream), (char*)filter_data);
		return -ENXIO;
	}
	return 0;
}

static void nxp_pcm_dma_release_channel(void *runtime_data)
{
	struct nxp_pcm_runtime_data *prtd = runtime_data;
	if (prtd && prtd->dma_chan)
		dma_release_channel(prtd->dma_chan);
	pr_debug("release dma '%s'\n", (char*)prtd->dma_param->dma_ch_name);
}

static int nxp_pcm_dma_slave_config(void *runtime_data, int stream)
{
	struct nxp_pcm_runtime_data *prtd = runtime_data;
	struct nxp_pcm_dma_param *dma_param = prtd->dma_param;
	struct dma_slave_config slave_config = { 0, };
	dma_addr_t	peri_addr = dma_param->peri_addr;
	int	bus_width = dma_param->bus_width_byte;
	int	max_burst = dma_param->max_burst_byte/bus_width; 
	int ret;

	if (SNDRV_PCM_STREAM_PLAYBACK == stream) {
		slave_config.direction 		= DMA_MEM_TO_DEV;
		slave_config.dst_addr 		= peri_addr;
		slave_config.dst_addr_width = bus_width;
		slave_config.dst_maxburst 	= max_burst;
		slave_config.src_addr_width = bus_width;
		slave_config.src_maxburst 	= max_burst;
		slave_config.device_fc 		= false;
	} else {
		slave_config.direction 		= DMA_DEV_TO_MEM;
		slave_config.src_addr 		= peri_addr;
		slave_config.src_addr_width = bus_width;
		slave_config.src_maxburst 	= max_burst;
		slave_config.dst_addr_width = bus_width;
		slave_config.dst_maxburst 	= max_burst;
		slave_config.device_fc 		= false;
	}

	ret = dmaengine_slave_config(prtd->dma_chan, &slave_config);

	pr_debug("%s: %s %s, %s, addr=0x%x, bus=%d byte, burst=%d (%d)\n",
		__func__, ret?"FAIL":"DONE", STREAM_STR(stream),
		dma_param->dma_ch_name,	peri_addr, bus_width, dma_param->max_burst_byte, max_burst);
	return ret;
}

static int nxp_pcm_dma_prepare_and_submit(struct snd_pcm_substream *substream)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_chan *chan = prtd->dma_chan;
	struct dma_async_tx_descriptor *desc;
	enum dma_transfer_direction direction;
	int period_time_us;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        direction = DMA_MEM_TO_DEV;
    else
        direction = DMA_DEV_TO_MEM;

	/* dma offset */
	prtd->offset = 0;

	desc = dmaengine_prep_dma_cyclic(chan,
				runtime->dma_addr,
				snd_pcm_lib_buffer_bytes(substream),
				snd_pcm_lib_period_bytes(substream), direction);

	if (!desc) {
		printk(KERN_ERR "%s: cannot prepare slave %s dma\n",
			__func__, prtd->dma_param->dma_ch_name);
		return -EINVAL;
	}

	desc->callback = nxp_pcm_dma_complete;
	desc->callback_param = substream;
	dmaengine_submit(desc);

	/*
	 * debug msg
	 */
	if(prtd->dma_param->real_clock != 0) /* i2s master mode */
		period_time_us = (1000000*1000)/
			((prtd->dma_param->real_clock*1000)/runtime->period_size);
	else /* i2s slave mode */
		period_time_us = 1000;

	pr_debug("%s: %s\n", __func__, STREAM_STR(substream->stream));
	pr_debug("buffer_bytes=%6d, period_bytes=%6d, periods=%2d, rate=%6d, period_time=%3d ms\n",
		snd_pcm_lib_buffer_bytes(substream), snd_pcm_lib_period_bytes(substream),
		runtime->periods, runtime->rate, period_time_us/1000);

	return 0;
}

static int nxp_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	int ret = 0;
	pr_debug("%s: %s cmd=%d\n", __func__, STREAM_STR(substream->stream), cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		ret = nxp_pcm_dma_prepare_and_submit(substream);
		if (ret)
			return ret;
		dma_async_issue_pending(prtd->dma_chan);
		prtd->time_stamp_us = ktime_to_us(ktime_get());
		break;

	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		dmaengine_resume(prtd->dma_chan);
		break;

	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		dmaengine_pause(prtd->dma_chan);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
		dmaengine_terminate_all(prtd->dma_chan);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static snd_pcm_uframes_t nxp_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	return bytes_to_frames(runtime, prtd->offset);
}

static int nxp_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	static struct snd_pcm_hardware *hw = &nxp_pcm_hardware;
	struct nxp_pcm_runtime_data *prtd;
	char *dma_name;
	int ret = 0;

	pr_debug("%s %s\n", __func__, STREAM_STR(substream->stream));
	prtd = kzalloc(sizeof(struct nxp_pcm_runtime_data), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	runtime->private_data = prtd;

	prtd->dma_param = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	ret = nxp_pcm_dma_request_channel(prtd, substream->stream);
	if (0 > ret)
		return ret;

	ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	if (0 > ret) {
		nxp_pcm_dma_release_channel(prtd);
		return ret;
	}

	/*
	 * change period_bytes_max value for SPDIFTX
	 * SDPIF min bus width is 2 byte for 16bit pcm
	 */
	dma_name = prtd->dma_param->dma_ch_name;
	if (!strcmp(dma_name, DMA_PERIPHERAL_NAME_SPDIFTX))
		hw->period_bytes_max = 4096;
	else
		hw->period_bytes_max = PERIOD_BYTES_MAX;

	return snd_soc_set_runtime_hwparams(substream, &nxp_pcm_hardware);
}

static int nxp_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct nxp_pcm_runtime_data *prtd = runtime->private_data;

	pr_debug("%s %s\n", __func__, STREAM_STR(substream->stream));
	nxp_pcm_dma_release_channel(prtd);
	kfree(prtd);

	return 0;
}

static int nxp_pcm_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct nxp_pcm_runtime_data *prtd = substream_to_prtd(substream);
	int ret;

	ret = nxp_pcm_dma_slave_config(prtd, substream->stream);
	if (0 > ret)
		return ret;

	/* debug info */
	prtd->periods = params_periods(params);
	prtd->period_bytes = params_period_bytes(params);
	prtd->buffer_bytes = params_buffer_bytes(params);

	/* i2s master mode */
	if(prtd->dma_param->real_clock != 0)
		prtd->period_time_us = (1000000*1000)/
			((prtd->dma_param->real_clock*1000)/params_period_size(params));
	else /* i2s slave mode */
		prtd->period_time_us = 1000;

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);

	if(substream->stream == 0)
		nxp_pcm_file_mem_allocate(DUMP_DMA_PATH_P, substream, params);
	else
		nxp_pcm_file_mem_allocate(DUMP_DMA_PATH_C, substream, params);

	/*
	 * debug msg
	 */
	pr_debug("%s: %s\n", __func__, STREAM_STR(substream->stream));
	pr_debug("buffer_size =%6d, period_size =%6d, periods=%2d, rate=%6d\n, real_rate=%6d\n",
		params_buffer_size(params),	params_period_size(params),
		params_periods(params), params_rate(params), prtd->dma_param->real_clock);
	pr_debug("buffer_bytes=%6d, period_bytes=%6d, periods=%2d, period_time=%3lld us\n",
		prtd->buffer_bytes, prtd->period_bytes, prtd->periods, prtd->period_time_us);
	return 0;
}

static int nxp_pcm_hw_free(struct snd_pcm_substream *substream)
{
	if(substream->stream == 0)
		nxp_pcm_file_mem_free(DUMP_DMA_PATH_P, substream);
	else
		nxp_pcm_file_mem_free(DUMP_DMA_PATH_C, substream);

	snd_pcm_set_runtime_buffer(substream, NULL);
	return 0;
}

static int nxp_pcm_mmap(struct snd_pcm_substream *substream,
		struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return dma_mmap_writecombine(substream->pcm->card->dev, vma,
					runtime->dma_area,
					runtime->dma_addr,
					runtime->dma_bytes);
}

static struct snd_pcm_ops nxp_pcm_ops = {
	.open		= nxp_pcm_open,
	.close		= nxp_pcm_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= nxp_pcm_hw_params,
	.hw_free	= nxp_pcm_hw_free,
	.trigger	= nxp_pcm_trigger,
	.pointer	= nxp_pcm_pointer,
	.mmap		= nxp_pcm_mmap,
};

static int nxp_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = nxp_pcm_hardware.buffer_bytes_max;

	pr_debug("%s: %s, dma_alloc_writecombine %d byte\n",
		__func__, STREAM_STR(substream->stream), size);

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->bytes = size;
	buf->area = dma_alloc_writecombine(buf->dev.dev, size, &buf->addr, GFP_KERNEL);
	if (!buf->area) {
		printk(KERN_ERR "Fail, %s dma buffer allocate (%d)\n",
			STREAM_STR(substream->stream), size);
		return -ENOMEM;
	}

	pr_debug("%s: %s, dma_alloc_writecombine %d byte, vir = 0x%x, phy = 0x%x\n",
		__func__, STREAM_STR(substream->stream), size, (unsigned int)buf->area, buf->addr);
	return 0;
}

static void nxp_pcm_release_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;

	substream = pcm->streams[stream].substream;
	if (!substream)
		return;

	buf = &substream->dma_buffer;
	if (!buf->area)
		return;

	dma_free_writecombine(pcm->card->dev, buf->bytes,
				buf->area, buf->addr);
	buf->area = NULL;
}

static u64 nxp_pcm_dmamask = DMA_BIT_MASK(32);

static int nxp_pcm_new(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_card *card = runtime->card->snd_card;
	struct snd_pcm *pcm = runtime->pcm;
	int ret = 0;

	/* dma mask */
	if (!card->dev->dma_mask)
		card->dev->dma_mask = &nxp_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = nxp_pcm_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto err;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = nxp_pcm_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto err_free;
	}
	return 0;

err_free:
	nxp_pcm_release_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
err:
	return ret;
}

static void nxp_pcm_free(struct snd_pcm *pcm)
{
	nxp_pcm_release_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
	nxp_pcm_release_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
}

static struct snd_soc_platform_driver pcm_platform = {
	.ops		= &nxp_pcm_ops,
	.pcm_new	= nxp_pcm_new,
	.pcm_free	= nxp_pcm_free,
};

static int __devinit nxp_pcm_probe(struct platform_device *pdev)
{
	int ret = snd_soc_register_platform(&pdev->dev, &pcm_platform);
	printk(KERN_INFO "snd pcm: %s sound platform '%s'\n", ret?"fail":"register", pdev->name);
	return ret;
}

static int __devexit nxp_pcm_remove(struct platform_device *pdev)
{
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver pcm_driver = {
	.driver = {
		.name  = DEV_NAME_PCM,
		.owner = THIS_MODULE,
	},
	.probe = nxp_pcm_probe,
	.remove = __devexit_p(nxp_pcm_remove),
};

static struct platform_device pcm_device = {
	.name	= DEV_NAME_PCM,
	.id		= -1,
};

static int __init nxp_pcm_init(void)
{
	platform_device_register(&pcm_device);
	return platform_driver_register(&pcm_driver);
}

static void __exit nxp_pcm_exit(void)
{
	platform_driver_unregister(&pcm_driver);
	platform_device_unregister(&pcm_device);
}

module_init(nxp_pcm_init);
module_exit(nxp_pcm_exit);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Sound PCM driver for the SLSI");
MODULE_LICENSE("GPL");

