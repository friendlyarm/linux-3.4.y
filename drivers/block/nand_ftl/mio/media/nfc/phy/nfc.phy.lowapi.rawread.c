/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.lowapi.rawread.c
 * Date         : 2014.09.04
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.25, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/

#if 0 // obsolute 2015.01.24

#include "nfc.phy.lowapi.h"

#include "nfc.phy.register.h"


/******************************************************************************
 * to use: printf()
 ******************************************************************************/
#include "../../exchange.config.h"

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
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
#include <linux/math64.h>

//#define DBG_PHY_LOWAPI_RAW(fmt, args...) printk(fmt, ##args)
#define DBG_PHY_LOWAPI_RAW(fmt, args...)

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <div64.h>
#include <linux/math64.h>
#include <common.h>
#include <malloc.h>

//#define DBG_PHY_LOWAPI_RAW(fmt, args...) printf(fmt, ##args)
#define DBG_PHY_LOWAPI_RAW(fmt, args...)

#else
#error "nfc.phy.lowapi.rawread.c: error! not defined build mode!"
#endif

/******************************************************************************
 *
 ******************************************************************************/
#include "../../exchange.config.h"

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
static volatile unsigned int *pNFCONTROL = (unsigned int *)(0xF0051000 + 0x88);
static volatile unsigned char *pNFDATA = (unsigned char *)(0xF0500000 + 0x00);
static volatile unsigned char *pNFCMD  = (unsigned char *)(0xF0500000 + 0x10);
static volatile unsigned char *pNFADDR = (unsigned char *)(0xF0500000 + 0x18);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
static volatile unsigned int *pNFCONTROL = (unsigned int *)(0xC0051000 + 0x88);
static volatile unsigned char *pNFDATA = (unsigned char *)(0x2C000000 + 0x00);
static volatile unsigned char *pNFCMD  = (unsigned char *)(0x2C000000 + 0x10);
static volatile unsigned char *pNFADDR = (unsigned char *)(0x2C000000 + 0x18);
#else
#error "nfc.phy.lowapi.rawread.c: error! not defined build mode!"
#endif

/******************************************************************************
 * local functions
 ******************************************************************************/
static void NFC_PHY_LOW_API_RAW_ChipSelect(unsigned int channel, unsigned int way, unsigned int select);
static int NFC_PHY_LOW_API_RAW_read(const MIO_NAND_RAW_INFO *info, unsigned int block_ofs, unsigned int page_ofs, unsigned int byte_ofs, unsigned int bytes_to_read, void *buf);
static int NFC_PHY_LOW_API_RAW_write(const MIO_NAND_RAW_INFO *info, unsigned int block_ofs, unsigned int bytes_to_write, void *buf);
static int NFC_PHY_LOW_API_RAW_erase(const MIO_NAND_RAW_INFO *info, unsigned int block_ofs, unsigned int block_cnt);

/******************************************************************************
 * extern functions
 ******************************************************************************/
// No ECC + No Randomize + No ReadRetry
int NFC_PHY_LOWAPI_nand_raw_read(const MIO_NAND_RAW_INFO *info, loff_t ofs, size_t *len, u_char *buf)
{
    unsigned int block_ofs=0, page_ofs=0, byte_ofs=0;
    unsigned int bytes_to_read = (unsigned int)*len;
    int curr_blockindex = -1;

    byte_ofs  = ofs & (info->bytes_per_page - 1);  ofs = div_u64(ofs, info->bytes_per_page);  //ofs /= info->bytes_per_page;
    page_ofs  = ofs & (info->pages_per_block - 1); ofs = div_u64(ofs, info->pages_per_block); //ofs /= info->pages_per_block;
    block_ofs = ofs;

    curr_blockindex = NFC_PHY_LOW_API_RAW_read(info, block_ofs, page_ofs, byte_ofs, bytes_to_read, buf);
    if (curr_blockindex < 0)
    {
        *len = 0;
        return -1;
    }

    *len = (size_t)bytes_to_read;

    return curr_blockindex;
}

int NFC_PHY_LOWAPI_nand_raw_write(const MIO_NAND_RAW_INFO *info, loff_t ofs, size_t *len, u_char *buf)
{
    unsigned int block_ofs=0, page_ofs=0, byte_ofs=0;
    unsigned int bytes_to_write = (unsigned int)*len;
    int curr_blockindex = -1;

    byte_ofs  = ofs & (info->bytes_per_page - 1);  ofs = div_u64(ofs, info->bytes_per_page);  //ofs /= info->bytes_per_page;
    page_ofs  = ofs & (info->pages_per_block - 1); ofs = div_u64(ofs, info->pages_per_block); //ofs /= info->pages_per_block;
    block_ofs = ofs;

    if (byte_ofs || page_ofs || !(*len))
    {
        DBG_PHY_LOWAPI_RAW("NFC_PHY_LOWAPI_nand_raw_write: error! byte_ofs:%d, page_ofs%d, *len:%d\n", byte_ofs, page_ofs, *len);
        // 'ofs' must be block aligned
        *len = 0;
        return -1;
    }

    curr_blockindex = NFC_PHY_LOW_API_RAW_write(info, block_ofs, bytes_to_write, buf);
    if (curr_blockindex < 0)
    {
        *len = 0;
        return -1;
    }

    *len = (size_t)bytes_to_write;

    return curr_blockindex;
}

int NFC_PHY_LOWAPI_nand_raw_erase(const MIO_NAND_RAW_INFO *info, loff_t ofs, size_t size)
{
    loff_t ofs_org = ofs;
    unsigned int block_ofs=0, page_ofs=0, byte_ofs=0;
    unsigned int end_block_ofs=0, block_cnt=0;

    byte_ofs  = ofs & (info->bytes_per_page - 1);  ofs = div_u64(ofs, info->bytes_per_page);  //ofs /= info->bytes_per_page;
    page_ofs  = ofs & (info->pages_per_block - 1); ofs = div_u64(ofs, info->pages_per_block); //ofs /= info->pages_per_block;
    block_ofs = ofs;

    ofs = ofs_org + size - 1;
    byte_ofs  = ofs & (info->bytes_per_page - 1);  ofs = div_u64(ofs, info->bytes_per_page);  //ofs /= info->bytes_per_page;
    page_ofs  = ofs & (info->pages_per_block - 1); ofs = div_u64(ofs, info->pages_per_block); //ofs /= info->pages_per_block;
    end_block_ofs = ofs;
    block_cnt = end_block_ofs - block_ofs + 1;

    return NFC_PHY_LOW_API_RAW_erase(info, block_ofs, block_cnt);
}

#if 0 // No Need
int NFC_PHY_LOWAPI_raw_init(void)
{
    volatile unsigned int regval = 0;
    volatile unsigned int temp_reval = 0;

    NFC_PHY_LOW_API_RAW_ChipSelect(0, 0, 0);
    NFC_PHY_LOW_API_RAW_ChipSelect(0, 1, 0);

    // Disable AutoReset
    regval = *pNFCONTROL;
    regval &= ~(1 << NFCONTROL_IRQPEND);
    regval &= ~(1 << NFCONTROL_AUTORESET);

    temp_reval = regval;
    regval &= ~((1 << NFCONTROL_HWBOOT_W) | (1 << NFCONTROL_EXSEL_W));
    regval |= (((temp_reval >> NFCONTROL_EXSEL_R) & 0x1) << NFCONTROL_EXSEL_W);

    *pNFCONTROL = regval;

    // Clear InterruptPending
    regval = *pNFCONTROL;
    regval &= ~((1 << NFCONTROL_IRQPEND) | (1 << NFCONTROL_ECCRST));
    regval |= (1 << NFCONTROL_IRQPEND);

    temp_reval = regval;
    regval &= ~((1 << NFCONTROL_HWBOOT_W) | (1 << NFCONTROL_EXSEL_W));
    regval |= (((temp_reval >> NFCONTROL_EXSEL_R) & 0x1) << NFCONTROL_EXSEL_W);

    *pNFCONTROL = regval;

    // Disable Interrupt
    regval = *pNFCONTROL;
    regval &= ~((1 << NFCONTROL_IRQPEND) | (1 << NFCONTROL_IRQENB) | (1 << NFCONTROL_ECCIRQPEND) | (1 << NFCONTROL_ECCIRQENB) | (1 << NFCONTROL_ECCRST));
    regval |= (0 << NFCONTROL_IRQENB) | (0 << NFCONTROL_ECCIRQENB);

    temp_reval = regval;
    regval &= ~((1 << NFCONTROL_HWBOOT_W) | (1 << NFCONTROL_EXSEL_W));
    regval |= (((temp_reval >> NFCONTROL_EXSEL_R) & 0x1) << NFCONTROL_EXSEL_W);

    *pNFCONTROL = regval;

    DBG_PHY_LOWAPI_RAW("nfc.phy.raw.api : NFC_PHY_LOWAPI_raw_init\n");
}
#endif

/******************************************************************************
 * local functions
 ******************************************************************************/
void NFC_PHY_LOW_API_RAW_tDelay(unsigned int tDelay)
{
#if 0
    ndelay(tDelay);
#else
    while (tDelay--)
    {
        asm ("nop");
    }
#endif
}

void NFC_PHY_LOW_API_RAW_ChipSelect(unsigned int channel, unsigned int way, unsigned int select)
{
    volatile unsigned int regval = 0;
    volatile unsigned int temp_reval = 0;

    // SetNFBank
    regval = *pNFCONTROL;
    regval &= ~((1 << NFCONTROL_IRQPEND) | (3 << NFCONTROL_BANK));
    regval |= (way << NFCONTROL_BANK);

    temp_reval = regval;
    regval &= ~((1 << NFCONTROL_HWBOOT_W) | (1 << NFCONTROL_EXSEL_W));
    regval |= (((temp_reval >> NFCONTROL_EXSEL_R) & 0x1) << NFCONTROL_EXSEL_W);

    *pNFCONTROL = regval;

    // SetNFCS
    regval = *pNFCONTROL;
    regval &= ~(1 << NFCONTROL_IRQPEND);
    if (select) { regval |= (1 << NFCONTROL_NCSENB); }
    else        { regval &= ~(1 << NFCONTROL_NCSENB); }

    temp_reval = regval;
    regval &= ~((1 << NFCONTROL_HWBOOT_W) | (1 << NFCONTROL_EXSEL_W));
    regval |= (((temp_reval >> NFCONTROL_EXSEL_R) & 0x1) << NFCONTROL_EXSEL_W);

    *pNFCONTROL = regval;
}

int NFC_PHY_LOW_API_RAW_read(const MIO_NAND_RAW_INFO *info, unsigned int block_ofs, unsigned int page_ofs, unsigned int byte_ofs, unsigned int bytes_to_read, void *buf)
{
    unsigned char channel = info->channel;
    unsigned char phyway = info->phyway;
    unsigned int pages_per_block = info->pages_per_block;
    unsigned int bytes_per_page = info->bytes_per_page;
    unsigned int remain_bytes = bytes_to_read;
    unsigned char status, failed = 0;
    unsigned int row=0, col=0;

    unsigned int curr_byte_ofs = byte_ofs;
    unsigned int curr_bytes = 0, read_loop = 0;
    unsigned char *curr_buff = (unsigned char *)buf;
    unsigned char *data = 0;
    unsigned int curr_blockindex = block_ofs;
    unsigned int curr_pageindex = page_ofs;

    NFC_PHY_LOW_API_RAW_ChipSelect(channel, phyway, 1);
    {
        while (remain_bytes > 0)
        {
            if (curr_blockindex >= info->blocks_per_lun)
            {
                curr_blockindex = -1;
                break;
            }

            failed = 0;
            row = (curr_blockindex * pages_per_block) + curr_pageindex;
            col = curr_byte_ofs;

            /******************************************************************
             * 1st : set read
             ******************************************************************/
            *pNFCMD = 0x00; // READ
            *pNFADDR = ((col&0x000000FF) >> 0);
            *pNFADDR = ((col&0x0000FF00) >> 8);
            *pNFADDR = ((row&0x000000FF) >> 0);
            *pNFADDR = ((row&0x0000FF00) >> 8);
            *pNFADDR = ((row&0x00FF0000) >> 16);
            *pNFCMD = 0x30;
            NFC_PHY_LOW_API_RAW_tDelay(500);    // tWB

            // wait
            *pNFCMD = 0x70; // STATUS
            NFC_PHY_LOW_API_RAW_tDelay(500);   // tWHR
            do
            {
                status = *pNFDATA;
            }while (!(status & (1 << 6)));

            // if failed, go next block.
            if (status & (1 << 0))
            {
                failed = 1;
            }
            else
            {
                curr_bytes = bytes_per_page - curr_byte_ofs;
                if (curr_bytes > remain_bytes)
                    curr_bytes = remain_bytes;

                DBG_PHY_LOWAPI_RAW("NFC_PHY_LOWAPI_RAW_read: blk:%d, pg:%d, byteofs:%d, bytes:%d, remain_bytes:%d\n", curr_blockindex, curr_pageindex, curr_byte_ofs, curr_bytes, remain_bytes);

                /**************************************************************
                 * 2nd : read data
                 **************************************************************/
                *pNFCMD = 0x00;
                NFC_PHY_LOW_API_RAW_tDelay(500);  // tWHR
                data = curr_buff;
                for (read_loop = 0; read_loop < curr_bytes; read_loop++)
                {
                    *data++ = *pNFDATA;
                }
            }

            if (failed)
            {
                curr_blockindex += 1;
            }
            else
            {
                remain_bytes -= curr_bytes;
                curr_buff += curr_bytes;

                if (remain_bytes)
                {
                    curr_byte_ofs = 0;
                    curr_pageindex += 1;
                    if (curr_pageindex >= pages_per_block)
                    {
                        curr_blockindex += 1;
                        curr_pageindex = 0;
                    }
                }
            }
        }
    }
    NFC_PHY_LOW_API_RAW_ChipSelect(channel, phyway, 0);

    return curr_blockindex;
}

int NFC_PHY_LOW_API_RAW_write(const MIO_NAND_RAW_INFO *info, unsigned int block_ofs, unsigned int bytes_to_write, void *buf)
{
    unsigned char channel = info->channel;
    unsigned char phyway = info->phyway;
    unsigned int pages_per_block = info->pages_per_block;
    unsigned int bytes_per_page = info->bytes_per_page;
    unsigned int remain_bytes = bytes_to_write;
    unsigned char status, failed = 0;
    unsigned int row=0, col=0;

    unsigned int curr_bytes = 0, write_loop = 0;
    unsigned char *curr_buff = (unsigned char *)buf;
    unsigned char *data = 0;
    unsigned int curr_blockindex = block_ofs;
    unsigned int curr_pageindex = 0;

    NFC_PHY_LOW_API_RAW_ChipSelect(channel, phyway, 1);
    {
        while (remain_bytes > 0)
        {
            if (curr_blockindex >= info->blocks_per_lun)
            {
                curr_blockindex = -1;
                break;
            }

            failed = 0;
            row = (curr_blockindex * pages_per_block) + curr_pageindex;
            col = 0;

            /******************************************************************
             * 1st : set program
             ******************************************************************/
            *pNFCMD = 0x80; // PROG 1ST
            *pNFADDR = ((col&0x000000FF) >> 0);
            *pNFADDR = ((col&0x0000FF00) >> 8);
            *pNFADDR = ((row&0x000000FF) >> 0);
            *pNFADDR = ((row&0x0000FF00) >> 8);
            *pNFADDR = ((row&0x00FF0000) >> 16);
            NFC_PHY_LOW_API_RAW_tDelay(500);    // tADL

            /******************************************************************
             * 2nd : write data
             ******************************************************************/
            curr_bytes = bytes_per_page;
            if (curr_bytes > remain_bytes)
                curr_bytes = remain_bytes;

            data = curr_buff;
            for (write_loop = 0; write_loop < curr_bytes; write_loop++)
            {
                *pNFDATA = *data;
                data += 1;
            }
            DBG_PHY_LOWAPI_RAW("NFC_PHY_LOWAPI_RAW_write: blk:%d, pg:%d, bytes:%d\n", curr_blockindex, curr_pageindex, curr_bytes);

            /******************************************************************
             * 3th : program
             ******************************************************************/
             *pNFCMD = 0x10; // PROG 2ND
            NFC_PHY_LOW_API_RAW_tDelay(500);    // tWB

            /******************************************************************
             * 4th : wait
             ******************************************************************/
            *pNFCMD = 0x70; // STATUS
            NFC_PHY_LOW_API_RAW_tDelay(500);   // tWHR
            do
            {
                status = *pNFDATA;
            }while (!(status & (1 << 6)));

            // if failed, go next block.
            if (status & (1 << 0))
            {
                failed = 1;
            }

            if (failed)
            {
                remain_bytes += curr_pageindex * bytes_per_page;
                curr_buff -= curr_pageindex * bytes_per_page;
                curr_pageindex = 0;
                curr_blockindex += 1;
            }
            else
            {
                remain_bytes -= curr_bytes;
                curr_buff += curr_bytes;

                if (remain_bytes)
                {
                    curr_pageindex += 1;
                    if (curr_pageindex >= info->pages_per_block)
                    {
                        curr_blockindex += 1;
                        curr_pageindex = 0;
                    }
                }
            }
        }
    }
    NFC_PHY_LOW_API_RAW_ChipSelect(channel, phyway, 0);

    return curr_blockindex;
}

int NFC_PHY_LOW_API_RAW_erase(const MIO_NAND_RAW_INFO *info, unsigned int block_ofs, unsigned int block_cnt)
{
    unsigned char channel = info->channel;
    unsigned char phyway = info->phyway;
    unsigned int pages_per_block = info->pages_per_block;
    unsigned int row=0;
    unsigned char status;
    unsigned int curr_blockindex = block_ofs;

    if (!block_cnt)
    {
        return -1;
    }

    NFC_PHY_LOW_API_RAW_ChipSelect(channel, phyway, 1);
    {
        while (block_cnt)
        {
            if (curr_blockindex >= info->blocks_per_lun)
            {
                curr_blockindex = -1;
                break;
            }
        
            row = curr_blockindex * pages_per_block;

            DBG_PHY_LOWAPI_RAW("NFC_PHY_LOW_API_RAW_erase (blk:%4d) ", curr_blockindex);

            /******************************************************************
             * 1st : erase
             ******************************************************************/
            *pNFCMD = 0x60; // ERASE 1ST
            *pNFADDR = ((row&0x000000FF) >> 0);
            *pNFADDR = ((row&0x0000FF00) >> 8);
            *pNFADDR = ((row&0x00FF0000) >> 16);
            *pNFCMD = 0xD0; // ERASE 2ST
            NFC_PHY_LOW_API_RAW_tDelay(500);    // tWB

            /******************************************************************
             * 2nd : wait
             ******************************************************************/
            *pNFCMD = 0x70; // STATUS
            NFC_PHY_LOW_API_RAW_tDelay(500);   // tWHR
            do
            {
                status = *pNFDATA;
            }while (!(status & (1 << 6)));

            // if failed, go next block.
            if (status & (1 << 0))
            {
                DBG_PHY_LOWAPI_RAW("failed!\n");
                curr_blockindex += 1;
                continue;
            }

            DBG_PHY_LOWAPI_RAW("done\n");

            block_cnt -= 1;
            if (block_cnt)
            {
                curr_blockindex += 1;
            }
        }
    }
    NFC_PHY_LOW_API_RAW_ChipSelect(channel, phyway, 0);

    return curr_blockindex;
}

#endif
