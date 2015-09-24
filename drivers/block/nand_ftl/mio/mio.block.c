/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.block.c
 * Date         : 2014.06.30
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.06.30 SD.LEE)
 *
 * Description  : MIO means "Media I/O"
 *                Media means any raw storage like ram, nand, nor, etc...
 *
 ******************************************************************************/

#define __MIO_BLOCK_GLOBAL__
#include "mio.block.h"
#include "mio.media.h"
#include "mio.sys.h"
#include "mio.definition.h"
#include "mio.smart.h"

#include "media/exchange.h"

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

/******************************************************************************
 * Optimize Option
 ******************************************************************************/
#if defined (__COMPILE_MODE_BEST_DEBUGGING__)
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

/******************************************************************************
 *
 *
 *
 ******************************************************************************/
static u_int mio_major = 0;

/******************************************************************************
 * Block Device Operation
 ******************************************************************************/
static int mio_bdev_open(struct block_device * _bdev, fmode_t _mode);
static int mio_bdev_close(struct gendisk * _disk, fmode_t _mode);
static int mio_bdev_ioctl(struct block_device * _bdev, fmode_t _mode, unsigned int _cmd, unsigned long _arg);

static struct block_device_operations mio_bdev_fops =
{
    .open = mio_bdev_open,
    .release = mio_bdev_close,
    .ioctl = mio_bdev_ioctl,
    .owner = THIS_MODULE,
};

/******************************************************************************
 *
 ******************************************************************************/
DEFINE_SEMAPHORE(mio_mutex);

static struct mio_state io_state;
static struct mio_device mio_dev;

unsigned long nxp_ftl_start_block = CFG_NAND_FTL_START_BLOCK;

/******************************************************************************
 *
 * Block Device Operations
 *
 ******************************************************************************/
static int mio_bdev_open(struct block_device * _bdev, fmode_t _mode)
{
    if (iminor(_bdev->bd_inode) > MIO_MINOR_CNT)
    {
        return -ENODEV;
    }

    return 0;
}

static int mio_bdev_close(struct gendisk * disk, fmode_t _mode)
{
    return 0;
}

static int mio_bdev_ioctl(struct block_device * _bdev, fmode_t _mode, unsigned int _cmd, unsigned long _arg)
{
    return 0;
}

/******************************************************************************
 *
 * MIO KThreads
 *
 ******************************************************************************/

/******************************************************************************
 *
 ******************************************************************************/
static int mio_background_thread(void * _arg)
{
    struct mio_state * io_state = mio_dev.io_state;

    if (Exchange.debug.misc.block_thread) { Exchange.sys.fn.print("MIO.BLOCK: mio_background_thread() Start\n"); }

    while (!kthread_should_stop())
    {
        Exchange.sys.fnSpor();

        io_state->background.status = MIO_BG_SLEEP;
        wait_event_timeout(io_state->background.wake.q, io_state->background.wake.cnt, HZ/10); // Wake-Up Every 100 ms
        io_state->background.status = MIO_BG_IDLE;

        if (io_state->power.suspending)
        {
            io_state->background.status = MIO_BG_SCHEDULED;
            schedule();
            io_state->background.status = MIO_BG_IDLE;
        }
        else
        {
            u64 cur_jiffies = get_jiffies_64();

            // Save Smart
            if ((mio_dev.capacity && (MioSmartInfo.volatile_writesectors > (mio_dev.capacity >> 6))) || (cur_jiffies > io_state->background.t.save_smart))
            {
                // Clear Trigger
                MioSmartInfo.volatile_writesectors = 0;

                // Set Background Jobs
                io_state->background.t.save_smart = cur_jiffies + MIO_TIME_SEC(1*60);
                io_state->background.e.save_smart = 1;
            }

            // Stanby Job
            if (io_state->transaction.trigger.e.written_standby && (cur_jiffies > io_state->background.t.standby))
            {
                // Clear Trigger
                io_state->transaction.trigger.e.written_standby = 0;
   
                // Set Background Jobs
                io_state->background.t.standby = MIO_TIME_DIFF_MAX(cur_jiffies);
                io_state->background.e.standby = 1;
            }
   
            // Background Job
            if (io_state->transaction.trigger.e.written_bgjobs && (cur_jiffies > io_state->background.t.bgjobs))
            {
                // Clear Trigger
                io_state->transaction.trigger.e.written_bgjobs = 0;
   
                // Set Background Jobs
                io_state->background.t.bgjobs = MIO_TIME_DIFF_MAX(cur_jiffies);
                io_state->background.e.bgjobs = 1;
            }
   
            // Wake-Up Transaction Thread
            if (io_state->background.e.save_smart || io_state->background.e.standby || io_state->background.e.bgjobs)
            {
                if (!io_state->power.suspending)
                {
                    wake_up_process(io_state->transaction.thread);
                }
            }
        }
    }

    if (Exchange.debug.misc.block_thread) { Exchange.sys.fn.print("MIO.BLOCK: mio_background_thread() Stop\n"); }

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
void mio_background(struct mio_state * _io_state)
{
    struct mio_state * io_state = _io_state;

    /**************************************************************************
     * MIO Background : Statistics
     **************************************************************************/
    if (io_state->background.e.save_smart)
    {
        miosmart_update_eccstatus();
        miosmart_save();
    
        // Clear Event
        io_state->background.e.save_smart = 0;
        io_state->background.e.standby = 0;
        io_state->background.e.bgjobs = 0;
    }

    /**************************************************************************
     * MIO Background : Background Operations (Migration, Garbage Collection, ...)
     **************************************************************************/
    if (io_state->background.e.standby)
    {
        media_standby(io_state);
        while (!media_is_idle(io_state));

        // Clear Event
        io_state->background.e.standby = 0;
        if (Exchange.debug.misc.block_background) { Exchange.sys.fn.print("MIO.BLOCK: Background standby\n"); }
    }

    if (io_state->background.e.bgjobs)
    {
        media_background(io_state);
        while (!media_is_idle(io_state));

        // Clear Event
        io_state->background.e.bgjobs = 0;
        if (Exchange.debug.misc.block_background) { Exchange.sys.fn.print("MIO.BLOCK: Background bgjobs\n"); }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
static int mio_transaction(struct request * _req, struct mio_state * _io_state)
{
    struct request * req = _req;
    struct mio_state * io_state = _io_state;

    unsigned int total_seccnt = get_capacity(req->rq_disk);
    unsigned int req_dir = rq_data_dir(req);
    unsigned int req_lba = blk_rq_pos(req);
    unsigned int req_seccnt = 0; //blk_rq_cur_sectors(req);
    unsigned int req_bytes = blk_rq_cur_bytes(req);
    char * req_buffer = req->buffer;

    unsigned int cmd_flags = req->cmd_flags;
    enum rq_cmd_type_bits cmd_type = req->cmd_type;

    req_seccnt = req_bytes >> 9;

    /**************************************************************************
     * Request Validation
     **************************************************************************/
    if (cmd_type != REQ_TYPE_FS)
    {
        return -EIO;
    }

    /**************************************************************************
     *
     **************************************************************************/
    if (cmd_flags & REQ_DISCARD)
    {
#if defined (__COMPILE_MODE_ELAPSE_T__)
        if (Exchange.sys.fn.elapse_t_start) { Exchange.sys.fn.elapse_t_start(ELAPSE_T_IO_MEDIA_DISCARD); }
#endif
        media_flush(io_state);
        media_trim(io_state, req_lba, req_seccnt);
        while (!media_is_idle(io_state));

#if defined (__COMPILE_MODE_ELAPSE_T__)
        if (Exchange.sys.fn.elapse_t_end) { Exchange.sys.fn.elapse_t_end(ELAPSE_T_IO_MEDIA_DISCARD); }
#endif
        return 0;
    }
    else if (cmd_flags & REQ_FLUSH)
    {
#if defined (__COMPILE_MODE_ELAPSE_T__)
        if (Exchange.sys.fn.elapse_t_start) { Exchange.sys.fn.elapse_t_start(ELAPSE_T_IO_MEDIA_FLUSH); }
#endif
        media_flush(io_state);
        while (!media_is_idle(io_state));

#if defined (__COMPILE_MODE_ELAPSE_T__)
        if (Exchange.sys.fn.elapse_t_end) { Exchange.sys.fn.elapse_t_end(ELAPSE_T_IO_MEDIA_FLUSH); }
#endif
        // Strange ??
        if ((req_lba + req_seccnt) <= total_seccnt)
        {
            return -EIO;
        }

        io_state->background.t.standby = get_jiffies_64() + MIO_TIME_MSEC(500);
        io_state->background.e.standby = 0;
        io_state->transaction.trigger.e.written_standby = 1;

        io_state->background.t.bgjobs = get_jiffies_64() + MIO_TIME_SEC(1);
        io_state->background.e.bgjobs = 0;
        io_state->transaction.trigger.e.written_bgjobs = 1;

        return 0;
    }
    else
    {
        /**********************************************************************
         * Request Validation
         **********************************************************************/
        if ((req_lba + req_seccnt) > total_seccnt)
        {
            return -EIO;
        }
    }

    /**************************************************************************
     * Transaction
     **************************************************************************/
    switch (req_dir)
    {
        case READ:
        {
            media_read(req_lba, req_seccnt, req_buffer, io_state);

            MioSmartInfo.io_current.read_bytes += req_bytes;
            MioSmartInfo.io_current.read_sectors += req_seccnt;
            MioSmartInfo.io_accumulate.read_bytes += req_bytes;
            MioSmartInfo.io_accumulate.read_sectors += req_seccnt;

        } break;

        case WRITE:
        {
            media_write(req_lba, req_seccnt, req_buffer, io_state);

            MioSmartInfo.io_current.write_bytes += req_bytes;
            MioSmartInfo.io_current.write_sectors += req_seccnt;
            MioSmartInfo.io_accumulate.write_bytes += req_bytes;
            MioSmartInfo.io_accumulate.write_sectors += req_seccnt;

            MioSmartInfo.volatile_writesectors += req_seccnt;

        } break;

        default:
        {
            return -EIO;
        }
    }

    /**************************************************************************
     * !! Do Not Modify !!
     **************************************************************************/
    io_state->transaction.trigger.t.ioed = get_jiffies_64();
    io_state->background.t.standby = get_jiffies_64() + MIO_TIME_MSEC(400);
    io_state->background.e.standby = 0;
    io_state->background.t.bgjobs = get_jiffies_64() + MIO_TIME_SEC(5);
    io_state->background.e.bgjobs = 0;

    if (WRITE == req_dir)
    {
        io_state->transaction.trigger.e.written_standby = 1;
        io_state->transaction.trigger.e.written_bgjobs = 1;
    }

    /**************************************************************************
     * Debug Transaction
     **************************************************************************/
    if (Exchange.debug.misc.block_transaction)
    {
        unsigned int _req_bcnt = req_bytes;
        unsigned int _i = 0;
        unsigned int _j = 0;

        if (WRITE == req_dir)
        {
            for (_i = 0; _i < _req_bcnt; _i += 512)
            {
                Exchange.sys.fn.print("MIO.BLOCK: Write(%xh,1) .. ", (unsigned int)(req_lba+(_i>>9)));

                for (_j = 0; _j < 16; _j++)
                {
                    Exchange.sys.fn.print("%02x ", req_buffer[_i+_j]);
                }   Exchange.sys.fn.print(" ... \n");
            }
        }
        else
        {
            for (_i = 0; _i < _req_bcnt; _i += 512)
            {
                Exchange.sys.fn.print("MIO.BLOCK:  Read(%xh,1) .. ", (unsigned int)(req_lba+(_i>>9)));

                for (_j = 0; _j < 16; _j++)
                {
                    Exchange.sys.fn.print("%02x ", req_buffer[_i+_j]);
                }   Exchange.sys.fn.print(" ... \n");
            }
        }
    }
    
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static int mio_transaction_thread(void * _arg)
{
    struct mio_state * io_state = mio_dev.io_state;
    struct request_queue * rq = io_state->transaction.rq;
    struct request * req = NULL;

    if (Exchange.debug.misc.block_thread) { Exchange.sys.fn.print("MIO.BLOCK: mio_transaction_thread() Start\n"); }

    spin_lock_irq(rq->queue_lock);

    while (!kthread_should_stop())
    {
        int res = 0;

        Exchange.sys.fnSpor();

        if (!req && !(req = blk_fetch_request(rq)))
        {
            // Background Jobs
            if (!io_state->power.suspending)
            {
                io_state->transaction.status = MIO_BACKGROUND;
                spin_unlock_irq(rq->queue_lock);
                {
                    mutex_lock(miosys_dev.ioctl_mutex);

#if defined (__COMPILE_MODE_ELAPSE_T__)
                    if (Exchange.sys.fn.elapse_t_start) { Exchange.sys.fn.elapse_t_start(ELAPSE_T_TRANSACTION_THREAD_BACKGROUND); }
#endif
                    mio_background(io_state);

#if defined (__COMPILE_MODE_ELAPSE_T__)
                    if (Exchange.sys.fn.elapse_t_end) { Exchange.sys.fn.elapse_t_end(ELAPSE_T_TRANSACTION_THREAD_BACKGROUND); }
#endif
                    mutex_unlock(miosys_dev.ioctl_mutex);
                }
                spin_lock_irq(rq->queue_lock);
                io_state->transaction.status = MIO_IDLE;
            }

            io_state->transaction.trigger.t.ioed = MIO_TIME_DIFF_MAX(get_jiffies_64());

            set_current_state(TASK_INTERRUPTIBLE);

            if (kthread_should_stop())
            {
                set_current_state(TASK_RUNNING);
            }

            spin_unlock_irq(rq->queue_lock);
            {
                io_state->transaction.status = MIO_SCHEDULED;

#if defined (__COMPILE_MODE_ELAPSE_T__)
                if (Exchange.sys.fn.elapse_t_start) { Exchange.sys.fn.elapse_t_start(ELAPSE_T_TRANSACTION_THREAD_SCHEDULED); }
#endif
                wait_event_timeout(io_state->transaction.wake.q, io_state->transaction.wake.cnt, HZ/10);

#if defined (__COMPILE_MODE_ELAPSE_T__)
                if (Exchange.sys.fn.elapse_t_end) { Exchange.sys.fn.elapse_t_end(ELAPSE_T_TRANSACTION_THREAD_SCHEDULED); }
#endif
                io_state->transaction.status = MIO_IDLE;
            }
            spin_lock_irq(rq->queue_lock);

            continue;
        }

        // Request IO
        io_state->transaction.status = MIO_REQ_BUSY;
        spin_unlock_irq(rq->queue_lock);
        {
            mutex_lock(miosys_dev.ioctl_mutex);

            media_super();

#if defined (__COMPILE_MODE_ELAPSE_T__)
            if (Exchange.sys.fn.elapse_t_start) { Exchange.sys.fn.elapse_t_start(ELAPSE_T_TRANSACTION_THREAD_IO); }
#endif
            res = mio_transaction(req, io_state);

#if defined (__COMPILE_MODE_ELAPSE_T__)
            if (Exchange.sys.fn.elapse_t_end) { Exchange.sys.fn.elapse_t_end(ELAPSE_T_TRANSACTION_THREAD_IO); }
#endif
            mutex_unlock(miosys_dev.ioctl_mutex);

            if (io_state->transaction.wake.cnt) { io_state->transaction.wake.cnt -= 1; }

            if (Exchange.sys.fn.LedReqIdle) { Exchange.sys.fn.LedReqIdle(); }
        }
        spin_lock_irq(rq->queue_lock);
        io_state->transaction.status = MIO_IDLE;

        if (!__blk_end_request_cur(req, res))
        {
            req = NULL;
        }
    }

    if (req)
    {
        __blk_end_request_all(req, -EIO);
    }

    spin_unlock_irq(rq->queue_lock);

    if (Exchange.debug.misc.block_thread) { Exchange.sys.fn.print("MIO.BLOCK: mio_transaction_thread() Stop\n"); }

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static void mio_request_fetch(struct request_queue * _q)
{
    struct mio_state * io_state = mio_dev.io_state;
    struct request_queue * rq = io_state->transaction.rq;
    struct request * req = NULL;

    if (Exchange.sys.fn.LedReqBusy) { Exchange.sys.fn.LedReqBusy(); }

    // If Suspend/Resume Memory Pool CheckSum Verify Fault
    if (io_state->power.pm_verify_fault)
    {
        if (!req && !(req = blk_fetch_request(rq)))
        {
            __blk_end_request_cur(req, -1);
        }
    }
    // If Suspend Mode
    else if (io_state->power.suspending)
    {
        if (!req && !(req = blk_fetch_request(rq)))
        {
            __blk_end_request_cur(req, -1);
        }
    }
    else
    {
        // Wake Up Background Thread for Suspend Resume
        if (!io_state->power.suspending && (MIO_BG_SCHEDULED == io_state->background.status))
        {
            wake_up_process(io_state->background.thread);
        }

        // Wake Up Transaction Thread
        io_state->transaction.wake.cnt += 1;
        wake_up_process(io_state->transaction.thread);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
static void mio_spin_lock(void)
{
    spin_lock_irq(&mio_dev.io_state->transaction.lock.s);
}

static void mio_spin_unlock(void)
{
    spin_unlock_irq(&mio_dev.io_state->transaction.lock.s);
}

static void mio_mutex_lock(void)
{
    mutex_lock(&mio_dev.io_state->transaction.lock.m);
}

static void mio_mutext_unlock(void)
{
    mutex_unlock(&mio_dev.io_state->transaction.lock.m);
}

static void mio_elapse_t_init(void)
{
    memset(Exchange.debug.elapse_t.sum, 0x00, sizeof(unsigned long long) * ELAPSE_T_MAX);
    memset(Exchange.debug.elapse_t.avg, 0x00, sizeof(unsigned long long) * ELAPSE_T_MAX);
    memset(Exchange.debug.elapse_t.cnt, 0x00, sizeof(unsigned long long) * ELAPSE_T_MAX);
    memset(Exchange.debug.elapse_t.min, 0xFF, sizeof(unsigned long long) * ELAPSE_T_MAX);
    memset(Exchange.debug.elapse_t.max, 0x00, sizeof(unsigned long long) * ELAPSE_T_MAX);
}

/******************************************************************************
 * Elapse Time
 ******************************************************************************/
#if defined (__COMPILE_MODE_ELAPSE_T__)

static struct
{
    ktime_t t1[ELAPSE_T_MAX];
    ktime_t t2[ELAPSE_T_MAX];
    ktime_t dt[ELAPSE_T_MAX];

} measure_t;

static unsigned char mio_elapse_t_condition(int _i)
{
    unsigned char measure_condition = 0;

    switch (_i)
    {
        case ELAPSE_T_TRANSACTION_THREAD_00:
        case ELAPSE_T_TRANSACTION_THREAD_01:
        case ELAPSE_T_TRANSACTION_THREAD_02:
        case ELAPSE_T_TRANSACTION_THREAD_03:
        case ELAPSE_T_TRANSACTION_THREAD_04:
        case ELAPSE_T_TRANSACTION_THREAD_05:
        case ELAPSE_T_TRANSACTION_THREAD_06:
        case ELAPSE_T_TRANSACTION_THREAD_07:
        {
            measure_condition = 1;

        } break;

        case ELAPSE_T_IO_MEDIA_DISCARD:
        case ELAPSE_T_IO_MEDIA_FLUSH:
        case ELAPSE_T_IO_MEDIA_RW:
        case ELAPSE_T_IO_MEDIA_R:
        case ELAPSE_T_IO_MEDIA_W:
        {
            measure_condition = 1;

        } break;

        case ELAPSE_T_IO_NFC_RW:
        case ELAPSE_T_IO_NFC_RANDOMIZER_RW:
        case ELAPSE_T_IO_NFC_DELAY_RW:
        case ELAPSE_T_IO_MEMIO_RW:
        case ELAPSE_T_IO_FTL_MAP_SEARCH_RW:
        {
            if (Exchange.debug.elapse_t.io.read || Exchange.debug.elapse_t.io.write)
            {
                measure_condition = 1;
            }

        } break;

        case ELAPSE_T_IO_NFC_R:
        case ELAPSE_T_IO_NFC_RANDOMIZER_R:
        case ELAPSE_T_IO_NFC_DELAY_R:
        case ELAPSE_T_IO_MEMIO_R:
        case ELAPSE_T_IO_FTL_MAP_SEARCH_R:
        {
            if (Exchange.debug.elapse_t.io.read)
            {
                measure_condition = 1;
            }

        } break;

        case ELAPSE_T_IO_NFC_W:
        case ELAPSE_T_IO_NFC_RANDOMIZER_W:
        case ELAPSE_T_IO_NFC_DELAY_W:
        case ELAPSE_T_IO_MEMIO_W:
        case ELAPSE_T_IO_FTL_MAP_SEARCH_W:
        {
            if (Exchange.debug.elapse_t.io.write)
            {
                measure_condition = 1;
            }

        } break;
    }

    return measure_condition;
}

static void mio_elapse_t_start(int _i)
{
    if (mio_elapse_t_condition(_i))
    {
        measure_t.t1[_i] = ktime_get();
    }
}

static void mio_elapse_t_end(int _i)
{
    if (mio_elapse_t_condition(_i))
    {
        measure_t.t2[_i] = ktime_get();
        measure_t.dt[_i].tv64 = ktime_to_ns(ktime_sub(measure_t.t2[_i], measure_t.t1[_i]));

        Exchange.debug.elapse_t.cnt[_i] += 1;
        Exchange.debug.elapse_t.sum[_i] += measure_t.dt[_i].tv64;
        Exchange.debug.elapse_t.avg[_i] = Exchange.sys.fn.div64(Exchange.debug.elapse_t.sum[_i], Exchange.debug.elapse_t.cnt[_i]);
        if (measure_t.dt[_i].tv64 < Exchange.debug.elapse_t.min[_i]) { Exchange.debug.elapse_t.min[_i] = measure_t.dt[_i].tv64; }
        if (measure_t.dt[_i].tv64 > Exchange.debug.elapse_t.max[_i]) { Exchange.debug.elapse_t.max[_i] = measure_t.dt[_i].tv64; }
    }
}

static void mio_elapse_t_io_measure_start(int _rw, int _r, int _w)
{
    if (Exchange.debug.elapse_t.io.read || Exchange.debug.elapse_t.io.write)
    {
        mio_elapse_t_start(_rw);

             if (Exchange.debug.elapse_t.io.read)  { mio_elapse_t_start(_r); }
        else if (Exchange.debug.elapse_t.io.write) { mio_elapse_t_start(_w); }
    }
}

static void mio_elapse_t_io_measure_end(int _rw, int _r, int _w)
{
    if (Exchange.debug.elapse_t.io.read || Exchange.debug.elapse_t.io.write)
    {
             if (Exchange.debug.elapse_t.io.read)  { mio_elapse_t_end(_r); }
        else if (Exchange.debug.elapse_t.io.write) { mio_elapse_t_end(_w); }

        mio_elapse_t_end(_rw);
    }
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
static int __init mio_init(void)
{
    int capacity = 0;

    printk(KERN_INFO "MIO.BLOCK:\n");
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");
#if !defined (__COMPILE_MODE_BEST_DEBUGGING__)
    printk(KERN_INFO "MIO.BLOCK:  Init Begin\n");
#else
    printk(KERN_INFO "MIO.BLOCK:  Init Begin (!!! WARNING : Driver Compiled Best Debug Mode !!!\n");
#endif
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");

    mio_dev.mutex = &mio_mutex;
    mio_dev.io_state = &io_state;

    /**************************************************************************
     * Open Media
     **************************************************************************/
    spin_lock_init(&mio_dev.io_state->transaction.lock.s);
    mutex_init(&mio_dev.io_state->transaction.lock.m);

    Exchange.sys.fn.slock   = mio_spin_lock;
    Exchange.sys.fn.sunlock = mio_spin_unlock;
    Exchange.sys.fn.mlock   = mio_mutex_lock;
    Exchange.sys.fn.munlock = mio_mutext_unlock;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    Exchange.sys.fn.elapse_t_init             = mio_elapse_t_init;
    Exchange.sys.fn.elapse_t_start            = mio_elapse_t_start;
    Exchange.sys.fn.elapse_t_end              = mio_elapse_t_end;
    Exchange.sys.fn.elapse_t_io_measure_start = mio_elapse_t_io_measure_start;
    Exchange.sys.fn.elapse_t_io_measure_end   = mio_elapse_t_io_measure_end;
    Exchange.sys.fn.elapse_t_init();
#endif

    if ((capacity = media_open()) < 0)
    {
        printk(KERN_ERR "MIO.BLOCK: media_open() Fail\n");
        return -1;
    }

    mio_dev.capacity = capacity;

    /**************************************************************************
     * Open Smart of Media
     **************************************************************************/
    if (miosmart_init(*Exchange.ftl.Channel, *Exchange.ftl.Way) < 0)
    {
        printk(KERN_ERR "MIO.BLOCK: miosmart_init() Fail\n");
        return -1;
    }
    miosmart_load();

    /**************************************************************************
     * Register Block Device Driver
     **************************************************************************/
    down(mio_dev.mutex);
    {
        /**********************************************************************
         * Register "mio" Block Device Driver
         **********************************************************************/
        if ((mio_major = register_blkdev(mio_major, "mio")) <= 0)
        {
            printk(KERN_ERR "MIO.BLOCK: register_blkdev() Fail\n");
            media_close();

            return -ENODEV;
        }

        /**********************************************************************
         * Initial Background Thread
         **********************************************************************/
        mio_dev.io_state->background.thread = NULL;

        mio_dev.io_state->background.status = MIO_BG_IDLE;

        init_waitqueue_head(&mio_dev.io_state->background.wake.q);
        mio_dev.io_state->background.wake.cnt = 0;

        mio_dev.io_state->background.t.standby = MIO_TIME_DIFF_MAX(get_jiffies_64());
        mio_dev.io_state->background.t.bgjobs = MIO_TIME_DIFF_MAX(get_jiffies_64());
        mio_dev.io_state->background.t.save_smart = MIO_TIME_DIFF_MAX(get_jiffies_64());
        mio_dev.io_state->background.e.standby = 0;
        mio_dev.io_state->background.e.bgjobs = 0;
        mio_dev.io_state->background.e.save_smart = 0;

        /**********************************************************************
         * Initial Transaction Thread
         **********************************************************************/
        mio_dev.io_state->transaction.thread = NULL;
        mio_dev.io_state->transaction.rq = NULL;
        spin_lock_init(&mio_dev.io_state->transaction.queue_lock);

        mio_dev.io_state->transaction.status = MIO_IDLE;

        init_waitqueue_head(&mio_dev.io_state->transaction.wake.q);
        mio_dev.io_state->transaction.wake.cnt = 0;

        mio_dev.io_state->transaction.trigger.t.ioed = MIO_TIME_DIFF_MAX(get_jiffies_64());
        mio_dev.io_state->transaction.trigger.e.written_standby = 0;
        mio_dev.io_state->transaction.trigger.e.written_bgjobs = 0;

        /**********************************************************************
         * Request Queue Create
         **********************************************************************/
        if (NULL == (mio_dev.io_state->transaction.rq = blk_init_queue(mio_request_fetch, &mio_dev.io_state->transaction.queue_lock)))
        {
            printk(KERN_ERR "MIO.BLOCK: blk_init_queue() Fail\n");
            unregister_blkdev(mio_major, "mio");
            media_close();

            return -ENODEV;
        }

        if (elevator_change(mio_dev.io_state->transaction.rq, "noop"))
        {
            blk_cleanup_queue(mio_dev.io_state->transaction.rq);
            return -ENODEV;
        }

        mio_dev.io_state->transaction.rq->queuedata = mio_dev.io_state;

        /**********************************************************************
         * KThreads
         **********************************************************************/
//#define THREAD_BIND_TO_CORE

        if (NULL == mio_dev.io_state->transaction.thread)
        {
#if !defined (THREAD_BIND_TO_CORE)
            mio_dev.io_state->transaction.thread = (struct task_struct *)kthread_run(mio_transaction_thread, mio_dev.io_state, "mio_transaction_thread");
#else
            mio_dev.io_state->transaction.thread = (struct task_struct *)kthread_create(mio_transaction_thread, mio_dev.io_state, "mio_transaction_thread");
#endif
            if (IS_ERR(mio_dev.io_state->transaction.thread))
            {
                mio_dev.io_state->transaction.thread = NULL;

                blk_cleanup_queue(mio_dev.io_state->transaction.rq);
                return -ENODEV;
            }
#if defined (THREAD_BIND_TO_CORE)
            // get_cpu() ÈÄ¿¡ put_cpu()
            kthread_bind(mio_dev.io_state->transaction.thread, 0);
            wake_up_process(mio_dev.io_state->transaction.thread);
#endif
        }

        if (NULL == mio_dev.io_state->background.thread)
        {
            mio_dev.io_state->background.thread = (struct task_struct *)kthread_run(mio_background_thread, mio_dev.io_state, "mio_background_thread");

            if (IS_ERR(mio_dev.io_state->background.thread))
            {
                mio_dev.io_state->background.thread = NULL;
                if (mio_dev.io_state->transaction.thread) { kthread_stop(mio_dev.io_state->transaction.thread); mio_dev.io_state->transaction.thread = NULL; }

                blk_cleanup_queue(mio_dev.io_state->transaction.rq);
                return -ENODEV;
            }
        }

        /**********************************************************************
         * The gendisk structure
         *
         *  - int major;
         *
         *  - int first_minor;
         *
         *  - int minors;
         *     Fields that describe the device number(s) used by the disk.
         *     At a minimum, a drive must use at least one minor number.
         *     If your drive is to be partitionable, however (and most should be),
         *     you want to allocate one minor number for each possible partition as well.
         *     A common value for minors is 16, which allows for the "full disk" device
         *     and 15 partitions. Some disk drivers use 64 minor numbers for each device.
         *
         *  - char disk_name[32];
         *     Field that should be set to the name of the disk device.
         *     It shows up in /proc/partitions and sysfs.
         *
         *  - struct block_device_operations *fops;
         *     Set of device operations from the previous section.
         *
         *  - struct request_queue *queue;
         *     Structure used by the kernel to manage I/O requests for this device
         *
         *  - int flags;
         *     A (little-used) set of flags describing the state of the drive.
         *     If your device has removable media, you should set GENHD_FL_REMOVABLE.
         *     CD-ROM drives can set GENHD_FL_CD.
         *     If, for some reason, you do not want partition information to show up in /proc/partitions, set GENHD_FL_SUPPRESS_PARTITION_INFO.
         *
         *  - sector_t capacity;
         *     The capacity of this drive, in 512-byte sectors.
         *     The sector_t type can be 64 bits wide. Drivers should not set this field directly.
         *     instead, pass the number of sectors to set_capacity.
         *
         *  - void * private_data;
         *     Block drivers may use this field for a pointer to their own internal data.
         *
         **********************************************************************/
        if (!(mio_dev.disk = alloc_disk(MIO_MINOR_CNT)))
        {
            printk(KERN_ERR "MIO.BLOCK: alloc_disk() Fail\n");
            blk_cleanup_queue(mio_dev.io_state->transaction.rq);
            unregister_blkdev(mio_major, "mio");
            media_close();
            return -ENODEV;
        }
        else
        {
            mio_dev.disk->major = mio_major;
            mio_dev.disk->first_minor = MIO_FIRST_MINOR;
            sprintf(mio_dev.disk->disk_name, "mio");
            mio_dev.disk->fops = &mio_bdev_fops;
            mio_dev.disk->queue = mio_dev.io_state->transaction.rq;
            mio_dev.disk->private_data = &mio_dev;
          //mio_dev.disk->flags = GENHD_FL_SUPPRESS_PARTITION_INFO;
            set_capacity(mio_dev.disk, mio_dev.capacity);

            /******************************************************************
             * Block Device Driver Support Flush Feature
             ******************************************************************/
            blk_queue_flush(mio_dev.io_state->transaction.rq, REQ_FLUSH);
          //mio_dev.io_state->transaction.rq->flush_flags = REQ_FLUSH & (REQ_FLUSH | REQ_FUA);

            /******************************************************************
             * Block Device Is Solid Disk
             ******************************************************************/
            queue_flag_set_unlocked(QUEUE_FLAG_NONROT, mio_dev.io_state->transaction.rq);

            /******************************************************************
             * Block Device Driver Support Discard Feature
             ******************************************************************/
            queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, mio_dev.io_state->transaction.rq);
            mio_dev.io_state->transaction.rq->limits.discard_granularity = 1<<9;
            mio_dev.io_state->transaction.rq->limits.max_discard_sectors = mio_dev.capacity;
            mio_dev.io_state->transaction.rq->limits.discard_zeroes_data = 1;

            add_disk(mio_dev.disk);
        }
    }
    up(mio_dev.mutex);

    /**************************************************************************
     * Register : /sys/class/misc/miosys
     **************************************************************************/
    if (misc_register(miosys_dev.miscdev))
    {
        printk(KERN_ERR "MIO.BLOCK: misc_register() Fail\n");
        return -ENODEV;
    }

    /**************************************************************************
     * Now the disk is "live"
     **************************************************************************/
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");
    printk(KERN_INFO "MIO.BLOCK:  Init End: Capacity %xh(%d) Sectors = %d MB\n", mio_dev.capacity, mio_dev.capacity, ((mio_dev.capacity>>10)<<9)>>10);
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");
    printk(KERN_INFO "MIO.BLOCK:\n");

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static void __exit mio_exit(void)
{
    printk(KERN_INFO "MIO.BLOCK:\n");
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");
    printk(KERN_INFO "MIO.BLOCK:  Exit Begin\n");
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");

    /**************************************************************************
     * Un-Register : /sys/class/misc/miosys
     **************************************************************************/
    misc_deregister(miosys_dev.miscdev);

    /**************************************************************************
     * Un-Register Block Device Driver
     **************************************************************************/
    down(mio_dev.mutex);
    {
        del_gendisk(mio_dev.disk);
        put_disk(mio_dev.disk);

        if (mio_dev.io_state->background.thread) { kthread_stop(mio_dev.io_state->background.thread); mio_dev.io_state->background.thread = NULL; }
        if (mio_dev.io_state->transaction.thread) { kthread_stop(mio_dev.io_state->transaction.thread); mio_dev.io_state->transaction.thread = NULL; }

        blk_cleanup_queue(mio_dev.io_state->transaction.rq);
        unregister_blkdev(mio_major, "mio");
    }
    up(mio_dev.mutex);

    /**************************************************************************
     * Close Media
     **************************************************************************/
    media_close();
    miosmart_deinit();

    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");
    printk(KERN_INFO "MIO.BLOCK:  Exit End\n");
    printk(KERN_INFO "MIO.BLOCK: --------------------------------------------------------------------------\n");
    printk(KERN_INFO "MIO.BLOCK:\n");
}

/******************************************************************************
 * Suspend/Resume
 ******************************************************************************/
static unsigned int nand_pm_verify = 0; // If Set, Can't Revert Power-On Time

static int nand_suspend(struct device * dev)
{
    mio_dev.io_state->power.suspending = 1;

    while (1)
    {
        if ((MIO_SCHEDULED == mio_dev.io_state->transaction.status) && (MIO_BG_SCHEDULED == mio_dev.io_state->background.status))
        {
            break;
        }

        usleep_range(1,1);
    }

    nand_pm_verify = media_suspend();

    return 0;
}

static int nand_resume(struct device * dev)
{
    if (media_resume() != nand_pm_verify)
    {
        mio_dev.io_state->power.pm_verify_fault = 1;
        return -1;
    }

    mio_dev.io_state->power.suspending = 0;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static SIMPLE_DEV_PM_OPS(nand_pmops, nand_suspend, nand_resume);

static struct platform_driver nand_driver =
{
    .driver = {
        .name  = DEV_NAME_NAND,
        .pm    = &nand_pmops,
        .owner = THIS_MODULE,
    },
};

/******************************************************************************
 *
 ******************************************************************************/
static int __init nand_init(void)
{
    memset((void *)&Exchange, 0, sizeof(EXCHANGES));

    platform_driver_register(&nand_driver);
    miosys_init();
    mio_init();

    return 0;
}

static void __exit nand_exit(void)
{
    mio_exit();
    platform_driver_unregister(&nand_driver);
}

/******************************************************************************
 *
 ******************************************************************************/
module_init(nand_init);
module_exit(nand_exit);
MODULE_LICENSE("EWS");
MODULE_AUTHOR("SD.LEE (mcdu1214@eastwho.com)");
MODULE_DESCRIPTION("Media I/O Block Driver");
MODULE_ALIAS_BLOCKDEV_MAJOR(mio_major);

/******************************************************************************
 * Optimize Restore
 ******************************************************************************/
#if defined (__COMPILE_MODE_BEST_DEBUGGING__)
#pragma GCC pop_options
#endif
