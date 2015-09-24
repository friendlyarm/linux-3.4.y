/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.sys.h
 * Date         : 2014.07.02
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.02 SD.LEE)
 *
 * Description  : APIs
 *
 ******************************************************************************/
#pragma once

#ifdef __MIO_BLOCK_SYSFS_GLOBAL__
#define MIO_BLOCK_SYSFS_EXT
#else
#define MIO_BLOCK_SYSFS_EXT extern
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
#include <linux/mutex.h>

/******************************************************************************
 * ioctl of miosys 
 ******************************************************************************/
struct miosys_media_io
{
    unsigned int blknr;
    unsigned int blkcnt;
    unsigned char *buf;
};

struct miosys_nand_io
{
    loff_t ofs;
    size_t len;
    u_char *buf;
};

/******************************************************************************
 *
 ******************************************************************************/
struct miosys_device
{
    struct mutex * ioctl_mutex;
    struct miscdevice * miscdev;
};

MIO_BLOCK_SYSFS_EXT struct miosys_device miosys_dev;

/******************************************************************************
 *
 ******************************************************************************/
MIO_BLOCK_SYSFS_EXT int miosys_init(void);


#define MIOSYS_CONTROL       _IO('M', 1)
//#define MIOSYS_MEDIA_READ    _IOR('M', 2, struct miosys_media_io)
//#define MIOSYS_MEDIA_WRITE   _IOW('M', 3, struct miosys_media_io)
#define MIOSYS_NAND_READ     _IOR('M', 4, struct miosys_nand_io)
#define MIOSYS_NAND_WRITE    _IOW('M', 5, struct miosys_nand_io)
#define MIOSYS_NAND_ERASE    _IOW('M', 6, struct miosys_nand_io)
#define MIOSYS_NANDRAW_READ  _IOR('M', 7, struct miosys_nand_io)
#define MIOSYS_NANDRAW_WRITE _IOW('M', 8, struct miosys_nand_io)
#define MIOSYS_NANDRAW_ERASE _IOW('M', 9, struct miosys_nand_io)

