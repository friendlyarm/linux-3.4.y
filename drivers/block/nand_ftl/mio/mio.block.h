/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.block.h
 * Date         : 2014.06.30
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.06.30 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#ifdef __MIO_BLOCK_GLOBAL__
#define MIO_BLOCK_EXT
#else
#define MIO_BLOCK_EXT extern
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
#include <linux/mm.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/of.h>

/******************************************************************************
 *
 ******************************************************************************/
//WQ : handling block request with WorkQueue. Just for test.
//#define TRANS_USING_WQ
//#define USING_WQ_THREAD

/******************************************************************************
 *
 ******************************************************************************/
#define MIO_FIRST_MINOR     (0)
#define MIO_MINOR_CNT       (16)

/******************************************************************************
 *
 ******************************************************************************/
#define MIO_TIME_DIFF_MAX(J64)  (0xFFFFFFFFFFFFFFFF - (J64))
#define MIO_TIME_MSEC(MS)       msecs_to_jiffies(MS)
#define MIO_TIME_SEC(S)         msecs_to_jiffies(S * 1000)


/******************************************************************************
 *
 ******************************************************************************/
struct mio_state
{
    struct
    {
        unsigned int suspending;
        unsigned int pm_verify_fault;

    } power;

    // MIO Background Jobs
    struct
    {
        struct task_struct * thread;

#define MIO_BG_IDLE           (0)
#define MIO_BG_SCHEDULED      (1)
#define MIO_BG_SLEEP          (2)
        unsigned short status;

        struct
        {
            wait_queue_head_t q;
            int cnt;

        } wake;

        // time
        struct
        {
            u64 standby;
            u64 bgjobs;
            u64 save_smart;

        } t;

        // event
        struct
        {
            unsigned int standby;
            unsigned int bgjobs;
            unsigned int save_smart;

        } e;

    } background;

    // MIO Data Transaction Jobs
    struct
    {
        struct task_struct * thread;
        struct request_queue * rq;
        spinlock_t queue_lock;

#define MIO_IDLE        (0)
#define MIO_SCHEDULED   (1)
#define MIO_BACKGROUND  (2)
#define MIO_SUPER       (3)
#define MIO_REQ_BUSY    (4)
        unsigned short status;

        struct
        {
            wait_queue_head_t q;
            int cnt;

        } wake;

        struct
        {
            struct
            {
                u64 ioed;

            } t;

            struct
            {
                unsigned int written_standby;
                unsigned int written_bgjobs;

            } e;

        } trigger;

        struct
        {
            spinlock_t s;
            struct mutex m;

        } lock;

    } transaction;

#ifdef TRANS_USING_WQ
#ifdef USING_WQ_THREAD
	struct kthread_worker	io_worker;
	struct task_struct	*io_thread;
	struct kthread_work	io_work;

	struct workqueue_struct *wq;
	struct work_struct work;
	bool bg_stop;
#endif
#endif
};

struct mio_device
{
    struct semaphore * mutex;

    /**************************************************************************
     * Device Information
     **************************************************************************/
    unsigned int capacity;
    struct gendisk * disk;
    struct mio_state * io_state;
};

MIO_BLOCK_EXT struct mio_device mio_dev;

