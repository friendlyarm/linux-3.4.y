/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.smart.h
 * Date         : 2014.10.23
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.10.23)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#ifdef __MIO_SMART_GLOBAL__
#define MIO_SMART_EXT
#else
#define MIO_SMART_EXT extern
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
#pragma pack(1)
typedef struct __MIO_SMART_CE_DATA__
{
	unsigned int  this_size;
	unsigned char this_version[4];

    struct
    {
		unsigned int corrected;
		unsigned int leveldetected;
		unsigned int uncorrectable;

    } ecc_sector;

    unsigned int writefail_count;
    unsigned int erasefail_count;
	unsigned int readretry_count;

	unsigned int reserved[4];

	unsigned int crc32;

} MIO_SMART_CE_DATA;
#pragma pack()

#pragma pack(1)
typedef struct __MIO_SMART_COMMON_DATA__
{
	unsigned int  this_size;
	unsigned char this_version[4];

	unsigned long long read_bytes;
	unsigned long long read_sectors;
	unsigned long long write_bytes;
	unsigned long long write_sectors;

	unsigned int reserved[10];

	unsigned int crc32;

} MIO_SMART_COMMON_DATA;
#pragma pack()

typedef struct __MIO_SMART_INFO__
{
	MIO_SMART_COMMON_DATA io_accumulate;
	MIO_SMART_COMMON_DATA io_current;

	unsigned char max_channels;
	unsigned char max_ways;

	MIO_SMART_CE_DATA **nand_accumulate;  // [ways][channels]
	MIO_SMART_CE_DATA **nand_current;     // [ways][channels]

	unsigned int volatile_writesectors;

  //unsigned int *wearlevel_data;

} MIO_SMART_INFO;

/******************************************************************************
 *
 ******************************************************************************/
MIO_SMART_EXT MIO_SMART_INFO MioSmartInfo;

/******************************************************************************
 *
 ******************************************************************************/
MIO_SMART_EXT int miosmart_init(unsigned int _max_channels, unsigned int _max_ways);
MIO_SMART_EXT int miosmart_is_init(void);
MIO_SMART_EXT void miosmart_deinit(void);

MIO_SMART_EXT int miosmart_update_eccstatus(void);
MIO_SMART_EXT int miosmart_load(void);
MIO_SMART_EXT int miosmart_save(void);

MIO_SMART_EXT void miosmart_get_erasecount(unsigned int *min, unsigned int *max, unsigned int *sum, unsigned int average[]);
MIO_SMART_EXT unsigned int miosmart_get_total_usableblocks(void);

