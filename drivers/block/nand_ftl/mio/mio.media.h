/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.media.h
 * Date         : 2014.06.30
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.06.30 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#ifdef __MIO_MEDIA_GLOBAL__
#define MIO_MEDIA_EXT
#else
#define MIO_MEDIA_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/cpufreq.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/gfp.h>

/******************************************************************************
 *
 ******************************************************************************/
MIO_MEDIA_EXT int  media_open(void);
MIO_MEDIA_EXT void media_close(void);
MIO_MEDIA_EXT unsigned int media_suspend(void);
MIO_MEDIA_EXT unsigned int media_resume(void);
MIO_MEDIA_EXT void media_write(sector_t _lba, unsigned int _seccnt, u8 * _buffer, void * _io_state);
MIO_MEDIA_EXT void media_read(sector_t _lba, unsigned int _seccnt, u8 * _buffer, void * _io_state);
MIO_MEDIA_EXT int media_super(void);
MIO_MEDIA_EXT void media_flush(void * _io_state);
MIO_MEDIA_EXT void media_trim(void * _io_state, int _lba, int _seccnt);
MIO_MEDIA_EXT void media_background(void * _io_state);
MIO_MEDIA_EXT void media_standby(void * _io_state);
MIO_MEDIA_EXT void media_powerdown(void * _io_state);
MIO_MEDIA_EXT int media_is_idle(void * _io_state);

/******************************************************************************
 *
 ******************************************************************************/
typedef struct __ELAPSED_MEDIA_IO__
{
	unsigned int lba;
	unsigned int seccnt;
	s64 elapsed_ns;
	
} ELAPSED_MEDIA_IO;

typedef struct __ELAPSED_MIO_MEDIA__
{
    ELAPSED_MEDIA_IO max_read;
    ELAPSED_MEDIA_IO max_write;
    ELAPSED_MEDIA_IO max_flush;
    ELAPSED_MEDIA_IO max_trim;
    ELAPSED_MEDIA_IO max_standby;
    ELAPSED_MEDIA_IO max_powerdown;

} ELAPSED_MIO_MEDIA;

MIO_MEDIA_EXT ELAPSED_MIO_MEDIA elapsed_mio_media_info;

