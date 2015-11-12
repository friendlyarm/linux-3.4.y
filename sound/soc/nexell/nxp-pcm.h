/*
 *
 */

#ifndef __NXP_PCM_H__
#define __NXP_PCM_H__

#include <mach/devices.h>
#include <linux/amba/pl08x.h>
#include "nxp-i2s.h"

#define SND_SOC_PCM_FORMATS	SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S8	|	\
							SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_U16_LE |	\
			    			SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_U24_LE 

struct nxp_pcm_dma_param {
	bool		 active;
	bool  	  (*dma_filter)(struct dma_chan *chan, void *filter_param);
	char 	   *dma_ch_name;
	dma_addr_t	peri_addr;
	int	 		bus_width_byte;
	int	 		max_burst_byte;
	unsigned int real_clock;
};

struct nxp_pcm_dma_area {
	dma_addr_t		physical;		/* dma virtual addr */
	unsigned char * virtual;		/* dma physical addr */
};

struct nxp_pcm_runtime_data {
	/* hw params */
	int period_bytes;
	int periods;
	int buffer_bytes;
	unsigned int dma_area;	/* virtual addr */
	unsigned int offset;
	/* DMA param */
	struct dma_chan  *dma_chan;
	struct nxp_pcm_dma_param *dma_param;
	/* dbg dma */
	unsigned int mem_area;
	long mem_len;
	unsigned int mem_offs;
	long long period_time_us;
	long long time_stamp_us;
};

#define	STREAM_STR(dir)	(SNDRV_PCM_STREAM_PLAYBACK == dir ? "playback" : "capture ")

#define	SNDDEV_STATUS_CLEAR		(0)
#define	SNDDEV_STATUS_SETUP		(1<<0)
#define	SNDDEV_STATUS_POWER		(1<<1)
#define	SNDDEV_STATUS_PLAY		(1<<2)
#define	SNDDEV_STATUS_CAPT		(1<<3)
#define	SNDDEV_STATUS_RUNNING	(SNDDEV_STATUS_PLAY | SNDDEV_STATUS_CAPT)

#endif
