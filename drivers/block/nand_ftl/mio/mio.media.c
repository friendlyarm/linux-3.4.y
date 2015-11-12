/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.media.c
 * Date         : 2014.06.30
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.06.30 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#define __MIO_MEDIA_GLOBAL__
#include "mio.media.h"

#include "mio.definition.h"
#include "media/nfc/phy/nfc.phy.lowapi.h"

#if defined (__BUILD_MODE_X86_LINUX_DEVICE_DRIVER__)
#define __MEDIA_ON_RAM__
#elif defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#define __MEDIA_ON_NAND__
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#define __MEDIA_ON_NAND__
#endif

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__MEDIA_ON_RAM__)
#define MEDIA_ON_RAM_SIZE __MB(100)
#include "mio.block.h"

static u8 * media_on_ram;

#elif defined (__MEDIA_ON_NAND__)
#include "media/exchange.h"
#include "mio.block.h"
#include "mio.smart.h"
#endif

/******************************************************************************
 * Optimize Option
 ******************************************************************************/
#if defined (__COMPILE_MODE_BEST_DEBUGGING__)
//#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

/******************************************************************************
 *
 ******************************************************************************/
static unsigned char * __trim_buffer = 0;

/******************************************************************************
 *
 ******************************************************************************/
void media_gpio_init(void);
void media_gpio_c00_high(void);
void media_gpio_c00_low(void);
void media_gpio_c01_high(void);
void media_gpio_c01_low(void);

static ktime_t media_elapse_begin(void);
static void media_elapse_end(ELAPSED_MEDIA_IO * elapsed_io, ktime_t begin_tm, unsigned int lba, unsigned int seccnt);

/******************************************************************************
 *
 ******************************************************************************/
int media_open(void)
{
    int capacity = -1;

#if defined (__MEDIA_ON_RAM__)
    {
        media_on_ram = (u8 *)vmalloc(MEDIA_ON_RAM_SIZE);

        if (media_on_ram == NULL)
        {
            return -1;
        }

        capacity =  __SECTOR_OF_BYTE(MEDIA_ON_RAM_SIZE);
    }

#elif defined (__MEDIA_ON_NAND__)
    printk(KERN_INFO "MIO.MEDIA: Open Begin\n");

    /**************************************************************************
     * MIO Debug Options
     **************************************************************************/
  //Exchange.debug.misc.block_thread = 1;
  //Exchange.debug.misc.block_transaction = 1;
  //Exchange.debug.misc.block_background = 1;
  //Exchange.debug.misc.media_open = 1;
  //Exchange.debug.misc.media_format = 1;
  //Exchange.debug.misc.media_close = 1;
  //Exchange.debug.misc.media_rw_memcpy = 1;
  //Exchange.debug.misc.smart_store = 1;
  //Exchange.debug.misc.uboot_format = 1;
  //Exchange.debug.misc.uboot_init = 1;

    Exchange.debug.ftl.format = 1;
    Exchange.debug.ftl.format_progress = 1;
    Exchange.debug.ftl.configurations = 1;
    Exchange.debug.ftl.open = 1;
    Exchange.debug.ftl.memory_usage = 1;
    Exchange.debug.ftl.boot = 1;
    Exchange.debug.ftl.block_summary = 1;
    Exchange.debug.ftl.license_detail = 0;
    Exchange.debug.ftl.warn = 1;
    Exchange.debug.ftl.error = 1;

  //Exchange.debug.nfc.sche.operation = 1;

  //Exchange.debug.nfc.phy.operation = 1;
  //Exchange.debug.nfc.phy.info_feature = 1;
  //Exchange.debug.nfc.phy.info_ecc = 1;
  //Exchange.debug.nfc.phy.info_ecc_correction = 1;
  //Exchange.debug.nfc.phy.info_ecc_corrected = 1;
  //Exchange.debug.nfc.phy.info_randomizer = 1;
  //Exchange.debug.nfc.phy.info_readretry = 1;
  //Exchange.debug.nfc.phy.info_readretry_table = 1;
  //Exchange.debug.nfc.phy.info_readretry_otp_table = 1;
  //Exchange.debug.nfc.phy.info_lowapi = 1;
    Exchange.debug.nfc.phy.warn_prohibited_block_access = 1;
  //Exchange.debug.nfc.phy.warn_ecc_uncorrectable = 1;
  //Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show = 1;
    Exchange.debug.nfc.phy.err_ecc_uncorrectable = 1;

    /**************************************************************************
     * MIO Exchange Init
     **************************************************************************/
    if (Exchange.debug.misc.media_open) { printk(KERN_INFO "MIO.MEDIA: EXCHANGE_init()\n"); }
    EXCHANGE_init();

    /**************************************************************************
     * FTL Need Leaner Buffer, Do Not Use kmalloc(...)
     **************************************************************************/
    Exchange.buffer.mpool_size  = 0;
    Exchange.buffer.mpool_size += 1 * 2 * (4<<20); // 1CH x 2WAY x 4MB (Page Map Table per Lun)
    Exchange.buffer.mpool_size += 1 * 2 * (1<<20); // 1CH x 2WAY x 1MB (Update Map Table per Lun)
    Exchange.buffer.mpool_size += (1<<20);         // 1MB (Misc)
    Exchange.buffer.mpool = (unsigned char *)vmalloc(Exchange.buffer.mpool_size);
    Exchange.sys.fn._memset((void *)Exchange.buffer.mpool, 0, Exchange.buffer.mpool_size);

    if (!Exchange.buffer.mpool)
    {
        printk(KERN_ERR "MIO.MEDIA: Memory Pool Pre-Allocation Fail\n");
        return -1;
    }

    /**************************************************************************
     * Media Need Extra Buffer
     **************************************************************************/
    __trim_buffer = (unsigned char *)Exchange.buffer.mpool;
    Exchange.buffer.mpool += 1024;
    Exchange.buffer.mpool_size -= 1024;

    /**************************************************************************
     * MIO Sys Options
     **************************************************************************/
    Exchange.sys.gpio.c_00 = 32*2+0;  // Asign GPIOC.00
    Exchange.sys.gpio.c_01 = 32*2+1;  // Asign GPIOC.01
    Exchange.sys.gpio.c_27 = 32*2+27; // Asign GPIOC.27

    Exchange.sys.support_list.spor = 1;
    Exchange.sys.support_list.led_indicator = 0;
    Exchange.sys.support_list.gpio_debug = 1;

    if ((Exchange.sys.support_list.led_indicator && Exchange.sys.support_list.gpio_debug) || (!Exchange.sys.support_list.led_indicator && !Exchange.sys.support_list.gpio_debug))
    {
        // Conflict or Disabled
    }
    else
    {
        if (Exchange.sys.support_list.led_indicator)
        {
            Exchange.sys.fn.LedReqBusy = media_gpio_c00_high;
            Exchange.sys.fn.LedReqIdle = media_gpio_c00_low;
            Exchange.sys.fn.LedNfcBusy = media_gpio_c01_high;
            Exchange.sys.fn.LedNfcIdle = media_gpio_c01_low;
        }
        else if (Exchange.sys.support_list.gpio_debug)
        {
            Exchange.sys.fn.GpioC00High = media_gpio_c00_high;
            Exchange.sys.fn.GpioC00Low  = media_gpio_c00_low;
            Exchange.sys.fn.GpioC01High = media_gpio_c01_high;
            Exchange.sys.fn.GpioC01Low  = media_gpio_c01_low;
        }

        media_gpio_init();
    }

    /**************************************************************************
     * Intial EWS FTL
     **************************************************************************/
#if defined (__COMPILE_MODE_FORMAT__)
    if (Exchange.debug.misc.media_format) { Exchange.sys.fn.print("MIO.MEDIA: Exchange.ftl.fnFormat()\n"); }
    if (Exchange.ftl.fnFormat(CHIP_NAME, CHIP_ID_BASE, 0) < 0)
    {
        printk(KERN_ERR "MIO.MEDIA: Exchange.ftl.fnFormat() Fail\n");
        return -1;
    }
#endif

    if (Exchange.debug.misc.media_open) { Exchange.sys.fn.print("MIO.MEDIA: Exchange.ftl.fnOpen()\n"); }
    if (Exchange.ftl.fnOpen(CHIP_NAME, CHIP_ID_BASE, 0) < 0)
    {
        printk(KERN_ERR "MIO.MEDIA: Exchange.ftl.fnOpen() Fail\n");
        return -1;
    }

    if (Exchange.debug.misc.media_open) { Exchange.sys.fn.print("MIO.MEDIA: Exchange.ftl.fnBoot()\n"); }
    if (Exchange.ftl.fnBoot(0) < 0)
    {
        printk(KERN_ERR "MIO.MEDIA: Exchange.ftl.fnBoot() Fail\n");
        return -1;
    }

    capacity = *Exchange.ftl.Capacity;

    NFC_PHY_LOWAPI_init();

    printk(KERN_INFO "MIO.MEDIA: Open End\n");

#endif

    return capacity;
}

/******************************************************************************
 *
 ******************************************************************************/
void media_close(void)
{
#if defined (__MEDIA_ON_RAM__)
    vfree(media_on_ram);
#elif defined (__MEDIA_ON_NAND__)
    printk(KERN_INFO "MIO.MEDIA: Close Begin\n");

    if (Exchange.debug.misc.media_close) { Exchange.sys.fn.print("MIO.MEDIA: Exchange.ftl.fnClose()\n"); }
    Exchange.ftl.fnClose();
    vfree(Exchange.buffer.mpool);

    printk(KERN_INFO "MIO.MEDIA: Close End\n");
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
unsigned int media_suspend(void)
{
#if defined (__MEDIA_ON_RAM__)
    return 0;

#elif defined (__MEDIA_ON_NAND__)
    unsigned int looper = 0;
    unsigned int * mpool = (unsigned int *)Exchange.buffer.mpool;
    unsigned int mpool_size = Exchange.buffer.mpool_size;
    unsigned int sum = 0;

    /**************************************************************************
     * FTL Suspend
     **************************************************************************/
    media_powerdown(NULL);
    while (!media_is_idle(NULL));
    Exchange.nfc.fnSuspend();

    /**************************************************************************
     * Memory Pool CheckSum After Suspend
     **************************************************************************/
    for (looper = 0; looper < (mpool_size/4); looper++)
    {
        sum += *mpool; mpool++;
    }

    return sum;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
unsigned int media_resume(void)
{
#if defined (__MEDIA_ON_RAM__)
    return 0;

#elif defined (__MEDIA_ON_NAND__)
    unsigned int looper = 0;
    unsigned int * mpool = (unsigned int *)Exchange.buffer.mpool;
    unsigned int mpool_size = Exchange.buffer.mpool_size;
    unsigned int sum = 0;

    /**************************************************************************
     * Memory Pool CheckSum Before Resume
     **************************************************************************/
    for (looper = 0; looper < (mpool_size/4); looper++)
    {
        sum += *mpool; mpool++;
    }

    /**************************************************************************
     * Resume
     **************************************************************************/
    Exchange.nfc.fnResume();

    return sum;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void media_write(sector_t _lba, unsigned int _seccnt, u8 * _buffer, void * _io_state)
{
    sector_t lba = _lba;
    unsigned int seccnt = _seccnt;
    u8 * buffer = _buffer;
  //struct mio_state * io_state = _io_state;

#if defined (__MEDIA_ON_RAM__)
    memcpy(media_on_ram + lba * __SECTOR_SIZEOF(1), buffer, seccnt * __SECTOR_SIZEOF(1));
#elif defined (__MEDIA_ON_NAND__)
    int wcidxfar = 0;
    int wcidx = 0;
    ktime_t begin_tm;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    Exchange.debug.elapse_t.io.write = 1;
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_MEDIA_RW, ELAPSE_T_IO_MEDIA_R, ELAPSE_T_IO_MEDIA_W); }
#endif

    begin_tm = media_elapse_begin();

    // Test Put Command To FTL
    while (1)
    {
        wcidxfar = Exchange.ftl.fnPrePutCommand(IO_CMD_WRITE, 0, lba, seccnt);
        wcidx = wcidxfar & (*Exchange.buffer.SectorsOfWriteCache - 1);

        if (-1 == wcidxfar)
        {
            media_super();
        }
        else
        {
            break;
        }
    }

    // Adjust Write Index
    *Exchange.buffer.WriteBlkIdx = (unsigned int)wcidxfar;

    // Copy buffer to WCache
    {
        unsigned long dest = 0;
        unsigned long src  = 0;
        unsigned int size = 0;

        // Write Cache Roll Over
        if ((wcidx + seccnt) > *Exchange.buffer.SectorsOfWriteCache)
        {
            dest = *Exchange.buffer.BaseOfWriteCache + (wcidx << 9);
            src  = (unsigned long)buffer;
            size = (*Exchange.buffer.SectorsOfWriteCache - wcidx) << 9;
            if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
            else                         {                  memcpy((void *)dest, (const void *)src, size); }
            if (Exchange.debug.misc.media_rw_memcpy)
            {
                if (sizeof(unsigned long) == 8) { printk(KERN_INFO "w %08x, %8x, mcpy 0 %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                else                            { printk(KERN_INFO "w %08x, %8x, mcpy 0 %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
            }

            dest = *Exchange.buffer.BaseOfWriteCache;
            src  = (unsigned long)buffer + size;
            size = (seccnt << 9) - size;
            if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
            else                         {                  memcpy((void *)dest, (const void *)src, size); }
            if (Exchange.debug.misc.media_rw_memcpy)
            {
                if (sizeof(unsigned long) == 8) { printk(KERN_INFO "w %08x, %8x, mcpy 1 %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                else                            { printk(KERN_INFO "w %08x, %8x, mcpy 1 %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
            }
        }
        else
        {
            dest = *Exchange.buffer.BaseOfWriteCache + (wcidx << 9);
            src  = (unsigned long)buffer;
            size = seccnt << 9;
            if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
            else                         {                  memcpy((void *)dest, (const void *)src, size); }
            if (Exchange.debug.misc.media_rw_memcpy)
            {
                if (sizeof(unsigned long) == 8) { printk(KERN_INFO "w %08x, %8x, mcpy - %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                else                            { printk(KERN_INFO "w %08x, %8x, mcpy - %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
            }
        }
    }

    // Adjust Write Index
    *Exchange.buffer.WriteBlkIdx += (unsigned int)seccnt;

    // Put Command to FTL
    Exchange.ftl.fnPutCommand(IO_CMD_WRITE, 0, lba, seccnt);

    media_super();

    media_elapse_end(&(elapsed_mio_media_info.max_write), begin_tm, _lba, _seccnt);

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_MEDIA_RW, ELAPSE_T_IO_MEDIA_R, ELAPSE_T_IO_MEDIA_W); }
    Exchange.debug.elapse_t.io.write = 0;
#endif
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void media_read(sector_t _lba, unsigned int _seccnt, u8 * _buffer, void * _io_state)
{
    sector_t lba = _lba;
    unsigned int seccnt = _seccnt;
    unsigned int req_seccnt = _seccnt;
    unsigned int req_trseccnt = 0;
    u8 * buffer = _buffer;
  //struct mio_state * io_state = _io_state;

#if defined (__MEDIA_ON_RAM__)
    memcpy(buffer, media_on_ram + lba * __SECTOR_SIZEOF(1), seccnt * __SECTOR_SIZEOF(1));
#elif defined (__MEDIA_ON_NAND__)
    int rbidxfar = 0;
    ktime_t begin_tm;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    Exchange.debug.elapse_t.io.read = 1;
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_MEDIA_RW, ELAPSE_T_IO_MEDIA_R, ELAPSE_T_IO_MEDIA_W); }
#endif

    begin_tm = media_elapse_begin();

    // Test Put Command to FTL
    while (1)
    {
        rbidxfar = Exchange.ftl.fnPrePutCommand(IO_CMD_READ, 0, lba, seccnt);

        if (-1 == rbidxfar)
        {
            media_super();
        }
        else
        {
            break;
        }
    }

    // Adjust Read Index
    *Exchange.buffer.ReadBlkIdx = (unsigned int)rbidxfar;

    // Put Command to FTL
    Exchange.ftl.fnPutCommand(IO_CMD_READ, 0, lba, seccnt);

    // Copy DATA From "FTL Read Buffer" to "BIO Read Buffer"
    while (1)
    {
        unsigned int rbidx  = 0;
        unsigned int readed = 0;

        rbidx  = *Exchange.buffer.ReadBlkIdx & (*Exchange.buffer.SectorsOfReadBuffer - 1);
        readed = Exchange.buffer.fnGetRequestReadSeccnt();

        if (readed)
        {
            /******************************************************************
             * Case By Case
             *
             *   Case 1) "FTL Readed Buffer" is Linear
             *
             *     a) "FTL Readed Buffer" > "BIO Read Buffer" : Cause "Read-Look-Ahead" or "Another BIO Requested" or ...
             *                                             readed
             *       FTL Read Buffer :   base +---------+----------+------------------------------------------+ end
             *                                |         | ++++++++ |                                          |
             *                                +---------+----------+------------------------------------------+
             *       BIO Read Buffer :                b +------+
             *                                          | ++++ |
             *                                          +------+
             *
             *     b) "FTL Readed Buffer" < "BIO Read Buffer"
             *                                                        readed
             *       FTL Read Buffer :   base +--------------------+----------+--------------------------------+ end
             *                                |                    | ++++++++ |                                |
             *                                +--------------------+----------+--------------------------------+
             *       BIO Read Buffer :                           b +----------------+
             *                                                     | ++++++++       |
             *                                                     +----------------+
             *
             *     c) "FTL Readed Buffer" == "BIO Read Buffer"
             *                                                                   readed
             *       FTL Read Buffer :   base +-------------------------------+----------+---------------------+ end
             *                                |                               | ++++++++ |                     |
             *                                +-------------------------------+----------+---------------------+
             *       BIO Read Buffer :                                      b +----------+
             *                                                                | ++++++++ |
             *                                                                +----------+
             *
             *
             *   Case 2) "FTL Readed Buffer" is Roll Over : refer a), b), c)
             *
             *       FTL Read Buffer :   base +-----+-----------------------------------------------------+----+ end
             *                                | +++ |                                                     | ++ |
             *                                +-----+-----------------------------------------------------+----+
             *
             ******************************************************************/
            unsigned int IsLinear = 1;

            unsigned long dest = 0;
            unsigned long src  = 0;
            unsigned int size = 0;
            unsigned int trseccnt = 0;

            // Is Roll Over ?
            if ((rbidx + readed) > *Exchange.buffer.SectorsOfReadBuffer)
            {
                IsLinear = 0;
            }

            // "FTL Readed Buffer" > "BIO Buffer"
            if (readed > seccnt)
            {
                // "FTL Readed Buffer" is Linear
                if (IsLinear)
                {
                    dest = (unsigned long)buffer;
                    src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                    size = seccnt << 9;
                    trseccnt += size >> 9;
                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                    else                         {                  memcpy((void *)dest, (const void *)src, size); }
                    if (Exchange.debug.misc.media_rw_memcpy)
                    {
                        if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                    }
                }
                // "FTL Readed Buffer" is Roll Over
                else
                {
                    // But 'seccnt' is Linear
                    if ((*Exchange.buffer.SectorsOfReadBuffer - rbidx) >= seccnt)
                    {
                        dest = (unsigned long)buffer;
                        src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                        size = seccnt << 9;
                        trseccnt += size >> 9;
                        if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                        else                         {                  memcpy((void *)dest, (const void *)src, size); }
                        if (Exchange.debug.misc.media_rw_memcpy)
                        {
                            if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                            else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        }
                    }
                    else
                    {
                        dest = (unsigned long)buffer;
                        src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                        size = (*Exchange.buffer.SectorsOfReadBuffer - rbidx) << 9;
                        trseccnt += size >> 9;
                        if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                        else                         {                  memcpy((void *)dest, (const void *)src, size); }
                        if (Exchange.debug.misc.media_rw_memcpy)
                        {
                            if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                            else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        }

                        dest = (unsigned long)buffer + size;
                        src  = *Exchange.buffer.BaseOfReadBuffer;
                        size = (seccnt << 9) - size;
                        trseccnt += size >> 9;
                        if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                        else                         {                  memcpy((void *)dest, (const void *)src, size); }
                        if (Exchange.debug.misc.media_rw_memcpy)
                        {
                            if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                            else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        }
                    }
                }
            }
            // "FTL Readed Buffer" <= "BIO Buffer"
            else if (readed <= seccnt)
            {
                // "FTL Readed Buffer" is Linear
                if (IsLinear)
                {
                    dest = (unsigned long)buffer;
                    src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                    size = readed << 9;
                    trseccnt += size >> 9;
                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                    else                         {                  memcpy((void *)dest, (const void *)src, size); }
                    if (Exchange.debug.misc.media_rw_memcpy)
                    {
                        if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                    }
                }
                // "FTL Readed Buffer" is Roll Over
                else
                {
                    dest = (unsigned long)buffer;
                    src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                    size = (*Exchange.buffer.SectorsOfReadBuffer - rbidx) << 9;
                    trseccnt += size >> 9;
                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                    else                         {                  memcpy((void *)dest, (const void *)src, size); }
                    if (Exchange.debug.misc.media_rw_memcpy)
                    {
                        if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                    }

                    dest = (unsigned long)buffer + size;
                    src  = *Exchange.buffer.BaseOfReadBuffer;
                    size = (readed << 9) - size;
                    trseccnt += size >> 9;
                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dest, (const void *)src, size); }
                    else                         {                  memcpy((void *)dest, (const void *)src, size); }
                    if (Exchange.debug.misc.media_rw_memcpy)
                    {
                        if (sizeof(unsigned long) == 8) { printk(KERN_INFO "r %08x, %8x, %8x mcpy %016lx %016lx %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, dest, src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                        else                            { printk(KERN_INFO "r %08x, %8x, %8x mcpy %08x %08x %8x %08x--%08x\n", (unsigned int)_lba, req_trseccnt, _seccnt, (unsigned int)dest, (unsigned int)src, size, *((unsigned int *)src), *((unsigned int *)(src + size - 4))); }
                    }
                }
            }

            seccnt -= trseccnt;
            buffer += trseccnt << 9;
            req_trseccnt += trseccnt;

            // Adjust Read Index
            *Exchange.buffer.ReadBlkIdx += trseccnt;

            // Current Request Completed
            if (req_trseccnt == req_seccnt)
            {
                break;
            }
        }
        else
        {
            media_super();
        }
    }

    media_super();

    media_elapse_end(&(elapsed_mio_media_info.max_read), begin_tm, _lba, _seccnt);

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_MEDIA_RW, ELAPSE_T_IO_MEDIA_R, ELAPSE_T_IO_MEDIA_W); }
    Exchange.debug.elapse_t.io.read = 0;
#endif
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
int media_super(void)
{
    int ret = -1;

#if defined (__MEDIA_ON_RAM__)
#elif defined (__MEDIA_ON_NAND__)
    if (Exchange.ftl.fnMain)
    {
        Exchange.ftl.fnMain();
        ret = 0;
    }
#endif

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
void media_flush(void * _io_state)
{
    ktime_t begin_tm;

    begin_tm = media_elapse_begin();

    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_FLUSH, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_FLUSH, 0, 0, 0);
                break;
            }
        }
    }

    media_elapse_end(&(elapsed_mio_media_info.max_flush), begin_tm, 0, 0);
}

void media_trim(void * _io_state, int _lba, int _seccnt)
{
    ktime_t begin_tm;

    // Make Trim Buffer
    unsigned int (*entry)[2] = (unsigned int (*)[2])__trim_buffer;

    begin_tm = media_elapse_begin();

    memset((void *)__trim_buffer, 0, 512);
    entry[0][0] = _lba;
    entry[0][1] = (_seccnt<<16);

    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_DATA_SET_MANAGEMENT, IO_CMD_DATA_SET_MANAGEMENT_FEATURE_TRIM, (unsigned long)__trim_buffer, 1) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_DATA_SET_MANAGEMENT, IO_CMD_DATA_SET_MANAGEMENT_FEATURE_TRIM, (unsigned long)__trim_buffer, 1);
                break;
            }
        }
    }

    media_elapse_end(&(elapsed_mio_media_info.max_trim), begin_tm, 0, 0);
}

void media_background(void * _io_state)
{
    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_BACKGROUND, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_BACKGROUND, 0, 0, 0);
                break;
            }
        }
    }
}

void media_standby(void * _io_state)
{
    ktime_t begin_tm;

    begin_tm = media_elapse_begin();

    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_STANDBY, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_STANDBY, 0, 0, 0);
                break;
            }
        }
    }

    media_elapse_end(&(elapsed_mio_media_info.max_standby), begin_tm, 0, 0);
}

void media_powerdown(void * _io_state)
{
    ktime_t begin_tm;

    begin_tm = media_elapse_begin();

    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_POWER_DOWN, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_POWER_DOWN, 0, 0, 0);
                break;
            }
        }
    }

    media_elapse_end(&(elapsed_mio_media_info.max_powerdown), begin_tm, 0, 0);
}

int media_is_idle(void * _io_state)
{
    if (!Exchange.ftl.fnIsIdle())
    {
        media_super();
        return 0;
    }

    return 1;
}

/******************************************************************************
 *
 ******************************************************************************/
static ktime_t media_elapse_begin(void)
{
    return ktime_get();
}

static void media_elapse_end(ELAPSED_MEDIA_IO * elapsed_io, ktime_t begin_tm, unsigned int lba, unsigned int seccnt)
{
    ktime_t end_tm = ktime_get();
    s64 elapsed_ns = ktime_to_ns(ktime_sub(end_tm, begin_tm));

    if (elapsed_io->elapsed_ns < elapsed_ns)
    {
        elapsed_io->elapsed_ns = elapsed_ns;
        elapsed_io->lba = lba;
        elapsed_io->seccnt = seccnt;
    }
}

/******************************************************************************
 * LED Indicator
 ******************************************************************************/
extern int /* -1 = invalid gpio, 0 = gpio's input mode, 1 = gpio's output mode. */ nxp_soc_gpio_get_io_dir(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/);
extern void nxp_soc_gpio_set_io_dir(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/, int /* '1' is output mode, '0' is input mode */);
extern void nxp_soc_gpio_set_out_value(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/, int /* '1' is high level, '0' is low level */);

void media_gpio_init(void)
{
    nxp_soc_gpio_set_io_dir(Exchange.sys.gpio.c_00, 1);
    nxp_soc_gpio_set_io_dir(Exchange.sys.gpio.c_01, 1);
}

void media_gpio_c00_high(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_00, 1);
}

void media_gpio_c00_low(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_00, 0);
}

void media_gpio_c01_high(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_01, 1);
}

void media_gpio_c01_low(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_01, 0);
}

/******************************************************************************
 * Optimize Restore
 ******************************************************************************/
#if defined (__COMPILE_MODE_BEST_DEBUGGING__)
//#pragma GCC pop_options
#endif
