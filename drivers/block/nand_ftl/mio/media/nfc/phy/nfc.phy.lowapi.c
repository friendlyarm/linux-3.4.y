/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.lowapi.c
 * Date         : 2014.08.25
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.25, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#define __NFC_PHY_LOWAPI_GLOBAL__
#include "nfc.phy.lowapi.h"

#include "../../exchange.h"
#include "../../../mio.definition.h"

#include "nfc.phy.h"
#include "nfc.phy.scan.h"
#include "nfc.phy.readretry.h"
#include "nfc.phy.rand.h"

/******************************************************************************
 * to use functions: printf()
 ******************************************************************************/
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

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <div64.h>
#include <linux/math64.h>
#include <common.h>
#include <malloc.h>

#else
#error "nfc.phy.lowapi.c: error! not defined build mode!"
#endif

/******************************************************************************
 * local
 ******************************************************************************/
static struct
{
    unsigned char temp_buf[512];

    unsigned char is_init;
    unsigned char go_nextblock;
    unsigned char rsvd[2];

    MIO_NAND_INFO nandinfo;

} low_api;

static int NFC_PHY_LOWAPI_write(unsigned int block_ofs, unsigned int page_ofs, unsigned int sectors, void *buf, unsigned char enable_ecc);
static int NFC_PHY_LOWAPI_read(unsigned int block_ofs, unsigned int page_ofs, unsigned int sec_ofs, unsigned int sectors, void *buf, unsigned char enable_ecc);
static int NFC_PHY_LOWAPI_erase(unsigned int block_ofs, unsigned int block_cnt);
static int NFC_PHY_LOWAPI_ofs_write(loff_t ofs, size_t *len, u_char *buf, unsigned char enable_ecc);
static int NFC_PHY_LOWAPI_ofs_read(loff_t ofs, size_t *len, u_char *buf, unsigned char enable_ecc);
static int NFC_PHY_LOWAPI_ofs_erase(loff_t ofs, size_t size);

/******************************************************************************
 * extern functions
 ******************************************************************************/
int NFC_PHY_LOWAPI_init(void)
{
    int ret = 0;
    NAND * nand_config = (NAND *)&phy_features.nand_config;
    unsigned int channels = 1, ways = 1;

    if (Exchange.ftl.fnIsBooted)
    {
        if (Exchange.ftl.fnIsBooted())
        {
            low_api.nandinfo.channel = 0;
            low_api.nandinfo.phyway = 0;
            low_api.nandinfo.pages_per_block = nand_config->_f.pages_per_block;
            low_api.nandinfo.bytes_per_page = nand_config->_f.databytes_per_page;
            low_api.nandinfo.blocks_per_lun = nand_config->_f.mainblocks_per_lun;
            low_api.nandinfo.ecc_bits = nand_config->_f.number_of_bits_ecc_correctability;
            low_api.nandinfo.bytes_per_ecc = nand_config->_f.maindatabytes_per_eccunit;
            low_api.nandinfo.bytes_per_parity = (14 * low_api.nandinfo.ecc_bits + 7) / 8;
            low_api.nandinfo.bytes_per_parity = (low_api.nandinfo.bytes_per_parity + (4-1)) & ~(4-1);
            low_api.nandinfo.readretry_type = nand_config->_f.support_type.read_retry;

            low_api.is_init = 1;

            if (Exchange.debug.nfc.phy.info_lowapi)
            {
                Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: OK (by FTL Boot)\n");
            }
            return 0;
        }
    }

    if (low_api.is_init)
    {
        return 0;
    }

    ret = NFC_PHY_Init(1);
    if (ret < 0)
    {
        Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: error! Init:%d\n", ret);
        return -1;
    }

    ret = NFC_PHY_EccInfoInit(channels, ways, 0);
    if (ret < 0)
    {
        Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: error! EccInfo Init:%d\n", ret);
        return -1;
    }
    NFC_PHY_SetFeatures(channels, ways, (void *)nand_config);

    ret = 0;

    if (nand_config->_f.support_type.read_retry)
    {
        ret = NFC_PHY_READRETRY_Init(channels, ways, 0, nand_config->_f.support_type.read_retry);
        if (ret < 0)
        {
            Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: error! ReadRetry!\n");
        }
    }

    switch(nand_config->_f.support_type.read_retry)
    {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            ret = NFC_PHY_HYNIX_READRETRY_MakeRegAll();
            if (ret >= 0)
            {
                Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: error! Hynix ReadRetry - Make!\n");
            }
        } break;
        
        default: {} break;
    }

    if (nand_config->_f.support_list.randomize)
    {
        ret = NFC_PHY_RAND_Init(nand_config->_f.maindatabytes_per_eccunit);
        if (ret < 0)
        {
            Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: error! Randomize\n");
            return -1;
        }

        NFC_PHY_RAND_Enable(1);
    }

    if (ret >= 0)
    {
        low_api.nandinfo.channel = 0;
        low_api.nandinfo.phyway = 0;
        low_api.nandinfo.pages_per_block = nand_config->_f.pages_per_block;
        low_api.nandinfo.bytes_per_page = nand_config->_f.databytes_per_page;
        low_api.nandinfo.blocks_per_lun = nand_config->_f.mainblocks_per_lun;
        low_api.nandinfo.ecc_bits = nand_config->_f.number_of_bits_ecc_correctability;
        low_api.nandinfo.bytes_per_ecc = nand_config->_f.maindatabytes_per_eccunit;
        low_api.nandinfo.bytes_per_parity = (14 * low_api.nandinfo.ecc_bits + 7) / 8;
        low_api.nandinfo.bytes_per_parity = (low_api.nandinfo.bytes_per_parity + (4-1)) & ~(4-1);
        low_api.nandinfo.readretry_type = nand_config->_f.support_type.read_retry;

        low_api.is_init = 1;
        low_api.go_nextblock = 0;

        if (Exchange.debug.nfc.phy.info_lowapi)
        {
            Exchange.sys.fn.print("NFC_PHY_LOWAPI_init: OK\n");
        }
    }

    return ret;
}

void NFC_PHY_LOWAPI_deinit(void)
{
    NAND * nand_config = (NAND *)&phy_features.nand_config;

    if (Exchange.ftl.fnIsBooted)
    {
        if (Exchange.ftl.fnIsBooted())
        {
            low_api.is_init = 0;
            return;
        }
    }

    if (low_api.is_init)
    {
        if (nand_config && nand_config->_f.support_list.randomize)
        {
            NFC_PHY_RAND_DeInit();
        }

        if (low_api.nandinfo.readretry_type)
        {
            NFC_PHY_READRETRY_DeInit();
        }

        NFC_PHY_DeInit();
        NFC_PHY_EccInfoDeInit();

        low_api.is_init = 0;
    }
}

int NFC_PHY_LOWAPI_is_init(void)
{
    return (low_api.is_init);
}

int NFC_PHY_LOWAPI_nand_write(loff_t ofs, size_t *len, u_char *buf, unsigned char enable_ecc)
{
    int ret = -1;

    {
        unsigned char warn_prohibited_block_access = Exchange.debug.nfc.phy.warn_prohibited_block_access;
    
        Exchange.debug.nfc.phy.warn_prohibited_block_access = 0;
        ret = NFC_PHY_LOWAPI_ofs_write(ofs, len, buf, enable_ecc);
        Exchange.debug.nfc.phy.warn_prohibited_block_access = warn_prohibited_block_access;
    }

    return ret;
}

int NFC_PHY_LOWAPI_nand_read(loff_t ofs, size_t *len, u_char *buf, unsigned char enable_ecc)
{
    int ret = -1;

    {
        unsigned char warn_prohibited_block_access = Exchange.debug.nfc.phy.warn_prohibited_block_access;
    
        Exchange.debug.nfc.phy.warn_prohibited_block_access = 0;
        ret = NFC_PHY_LOWAPI_ofs_read(ofs, len, buf, enable_ecc);
        Exchange.debug.nfc.phy.warn_prohibited_block_access = warn_prohibited_block_access;
    }

    return ret;
}

int NFC_PHY_LOWAPI_nand_erase(loff_t ofs, size_t size)
{
    int ret = -1;

    {
        unsigned char warn_prohibited_block_access = Exchange.debug.nfc.phy.warn_prohibited_block_access;
    
        Exchange.debug.nfc.phy.warn_prohibited_block_access = 0;
        ret = NFC_PHY_LOWAPI_ofs_erase(ofs, size);
        Exchange.debug.nfc.phy.warn_prohibited_block_access = warn_prohibited_block_access;
    }

    return ret;
}

/******************************************************************************
 * local functions
 ******************************************************************************/
int NFC_PHY_LOWAPI_ofs_write(loff_t ofs, size_t *len, u_char *buf, unsigned char enable_ecc)
{
    MIO_NAND_INFO *info = &low_api.nandinfo;
    unsigned int block_ofs=0, page_ofs=0, sector_ofs=0, byte_ofs=0;
    unsigned int sectors= (*len) / 512;
    unsigned int curr_blockindex = 0;

    byte_ofs   = ofs & (512 - 1);                        ofs = div_u64(ofs, 512);                      //ofs /= 512;
    sector_ofs = ofs & ((info->bytes_per_page/512) - 1); ofs = div_u64(ofs, info->bytes_per_page/512); //ofs /= (info->bytes_per_page/512);
    page_ofs   = ofs & (info->pages_per_block - 1);      ofs = div_u64(ofs, info->pages_per_block);    //ofs /= info->pages_per_block;
    block_ofs  = ofs;

    if (byte_ofs || sector_ofs || (*len & (512-1)) || (sectors & 1))
    {
        Exchange.sys.fn.print("NFC_PHY_LOWAPI_ofs_write: error! byte_ofs:%d, sector_ofs:%d, *len:%d, sectors:%d\n", byte_ofs, sector_ofs, *len, sectors);
        // 'ofs' must be block aligned and '*len' must be 1024 bytes aligned
        *len = 0;
        return -1;
    }

    curr_blockindex = NFC_PHY_LOWAPI_write(block_ofs, page_ofs, sectors, buf, enable_ecc);
    if (curr_blockindex < 0)
    {
        *len = 0;
        return -1;
    }

    *len = (sectors << 9);
    return curr_blockindex;
}

int NFC_PHY_LOWAPI_ofs_read(loff_t ofs, size_t *len, u_char *buf, unsigned char enable_ecc)
{
    MIO_NAND_INFO *info = &low_api.nandinfo;
    unsigned int block_ofs=0, page_ofs=0, byte_ofs=0, sector_ofs=0;
    unsigned int sectors=0;
    unsigned char *destbuf = (unsigned char *)buf;
    unsigned int remain_bytes = (unsigned int)*len;
    unsigned int copybytes = 0;
    int curr_blockindex = -1;

    byte_ofs   = ofs & (512 - 1);                        ofs = div_u64(ofs, 512);                      //ofs /= 512;
    sector_ofs = ofs & ((info->bytes_per_page/512) - 1); ofs = div_u64(ofs, info->bytes_per_page/512); //ofs /= (info->bytes_per_page/512);
    page_ofs   = ofs & (info->pages_per_block - 1);      ofs = div_u64(ofs, info->pages_per_block);    //ofs /= info->pages_per_block;
    block_ofs = ofs;

    do
    {
        // for first not sector aligned data
        if (byte_ofs)
        {
            copybytes = 512 - byte_ofs;
            curr_blockindex = NFC_PHY_LOWAPI_read(block_ofs, page_ofs, sector_ofs, 1, low_api.temp_buf, enable_ecc);
            if (curr_blockindex < 0)
            {
                break;
            }

            block_ofs += (curr_blockindex - block_ofs);

            if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy(destbuf, low_api.temp_buf+byte_ofs, copybytes); }
            else                         {                  memcpy(destbuf, low_api.temp_buf+byte_ofs, copybytes); }

            destbuf += copybytes;
            remain_bytes -= copybytes;
            sector_ofs += 1;
            if (sector_ofs >= (info->bytes_per_page/512))
            {
                page_ofs += 1;
                sector_ofs = 0;
                if (page_ofs >= info->pages_per_block)
                {
                    block_ofs += 1;
                    page_ofs = 0;
                }
            }
        }

        // for sector aligned data
        sectors = remain_bytes / 512;
        if (sectors)
        {
            curr_blockindex = NFC_PHY_LOWAPI_read(block_ofs, page_ofs, sector_ofs, sectors, destbuf, enable_ecc);
            if (curr_blockindex < 0)
            {
                break;
            }

            block_ofs += (curr_blockindex - block_ofs);

            destbuf += sectors * 512;
            remain_bytes -= sectors * 512;

            sector_ofs += sectors;
            if (sector_ofs >= (info->bytes_per_page/512))
            {
                page_ofs += sector_ofs / (info->bytes_per_page/512);
                sector_ofs &= ((info->bytes_per_page/512) - 1);
                if (page_ofs >= info->pages_per_block)
                {
                    block_ofs += page_ofs / (info->bytes_per_page/512);
                    page_ofs &= (info->pages_per_block - 1);
                }
            }
        }

        // for last not sector aligned data
        if (remain_bytes)
        {
            curr_blockindex = NFC_PHY_LOWAPI_read(block_ofs, page_ofs, sector_ofs, 1, low_api.temp_buf, enable_ecc);
            if (curr_blockindex < 0)
            {
                break;
            }

            if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy(destbuf, low_api.temp_buf, remain_bytes); }
            else                         {                  memcpy(destbuf, low_api.temp_buf, remain_bytes); }

            remain_bytes -= remain_bytes;
        }

    } while(0);

    *len -= remain_bytes;

    return curr_blockindex;
}

int NFC_PHY_LOWAPI_ofs_erase(loff_t ofs, size_t size)
{
    MIO_NAND_INFO *info = &low_api.nandinfo;
    loff_t ofs_org = ofs;
    unsigned int block_ofs=0, page_ofs=0, sector_ofs=0, byte_ofs=0;
    unsigned int end_block_ofs=0, block_cnt=0;

    byte_ofs = ofs & (512 - 1);                          ofs = div_u64(ofs, 512);                      //ofs /= 512;
    sector_ofs = ofs & ((info->bytes_per_page/512) - 1); ofs = div_u64(ofs, info->bytes_per_page/512); //ofs /= (info->bytes_per_page/512);
    page_ofs = ofs & (info->pages_per_block - 1);        ofs = div_u64(ofs, info->pages_per_block);    //ofs /= info->pages_per_block;
    block_ofs = ofs;

    ofs = ofs_org + size - 1;
    byte_ofs = ofs & (512 - 1);                          ofs = div_u64(ofs, 512);                      //ofs /= 512;
    sector_ofs = ofs & ((info->bytes_per_page/512) - 1); ofs = div_u64(ofs, info->bytes_per_page/512); //ofs /= (info->bytes_per_page/512);
    page_ofs = ofs & (info->pages_per_block - 1);        ofs = div_u64(ofs, info->pages_per_block);    //ofs /= info->pages_per_block;
    end_block_ofs = ofs;
    block_cnt = end_block_ofs - block_ofs + 1;

    return NFC_PHY_LOWAPI_erase(block_ofs, block_cnt);
}

int NFC_PHY_LOWAPI_write(unsigned int block_ofs, unsigned int page_ofs, unsigned int sectors, void *buf, unsigned char enable_ecc)
{
    MIO_NAND_INFO *info = &low_api.nandinfo;
    unsigned char channel = info->channel;
    unsigned char phyway = info->phyway;
    unsigned int sectors_per_page = (info->bytes_per_page >> 9);
    unsigned int eccunits_per_page = (info->bytes_per_ecc >> 9);
    unsigned int remain_sectors = sectors;
    unsigned char status, failed = 0;
    unsigned int loop_count = 0;
    unsigned int row=0, col=0;

    unsigned int curr_sectors = 0;
    unsigned char *curr_buff = (unsigned char *)buf;
    unsigned int curr_blockindex = block_ofs;
    unsigned int curr_pageindex = page_ofs;

    while (remain_sectors > 0)
    {
        if (curr_blockindex >= info->blocks_per_lun)
        {
            return -1;
        }

        failed = 0;
        row = (curr_blockindex * info->pages_per_block) + curr_pageindex;

        // write data to the specific block
        curr_sectors = (remain_sectors > sectors_per_page)? sectors_per_page: remain_sectors;
        loop_count = (curr_sectors + eccunits_per_page - 1) / eccunits_per_page;
        loop_count -= 1;

        if (Exchange.debug.nfc.phy.info_lowapi)
        {
            Exchange.sys.fn.print("NFC_PHY_LOWAPI_write: blk:%d, pg:%d, loop:%d enable_ecc:%d\n", curr_blockindex, curr_pageindex, loop_count, enable_ecc);
        }

        if (enable_ecc)
        {
            NFC_PHY_1stWriteData(channel, phyway, row, -1, col,
                                 /* mode */
                                 0, 0,
                                 /* DATA */
                                 loop_count, 0,
                                 info->bytes_per_ecc,
                                 info->bytes_per_parity,
                                 info->ecc_bits,
                                 (void *)curr_buff, 0,
                                 /* SPARE */
                                 0, 0, 0, 0);
        }
        else
        {
            NFC_PHY_1stWriteDataNoEcc(channel, phyway, row, col,
                                      /* DATA */
                                      loop_count,
                                      1024,
                                      (void *)curr_buff,
                                      /* SPARE */
                                      0, 0);
        }

        NFC_PHY_2ndWrite(channel, phyway);
        do
        {
            status = NFC_PHY_3rdWrite(channel, phyway);
        } while (!NFC_PHY_StatusIsRDY(status));

        // if failed, go next block.
        if (NFC_PHY_StatusIsFAIL(status))
        {
            failed = 1;
        }

        if (failed)
        {
            if (low_api.go_nextblock)
            {
                remain_sectors += curr_pageindex * sectors_per_page;
                curr_buff -= curr_pageindex * info->bytes_per_page;
                curr_pageindex = 0;
                curr_blockindex += 1;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            remain_sectors -= curr_sectors;
            curr_buff += info->bytes_per_page;

            if (remain_sectors)
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

    return curr_blockindex;
}

int NFC_PHY_LOWAPI_read(unsigned int block_ofs, unsigned int page_ofs, unsigned int sec_ofs, unsigned int sectors, void *buf, unsigned char enable_ecc)
{
    MIO_NAND_INFO *info = &low_api.nandinfo;
    unsigned char channel = info->channel;
    unsigned char phyway = info->phyway;
    unsigned char way = phyway;
    unsigned int sectors_per_page = (info->bytes_per_page >> 9);
    unsigned int remain_sectors = sectors;
    unsigned char status, failed = 0;
    unsigned int loop_count = 0;
    unsigned int row=0, col=0;

    unsigned int curr_sec_ofs = sec_ofs;
    unsigned int curr_stage = 0;
    unsigned int curr_sectors = 0;
    unsigned char *curr_buff = (unsigned char *)buf;
    unsigned int curr_blockindex = block_ofs;
    unsigned int curr_pageindex = page_ofs;

    unsigned char curr_retry_cnt = 0, max_retry_cnt = 0;
    unsigned int retryable = 0;

    max_retry_cnt = NFC_PHY_READRETRY_GetTotalReadRetryCount(channel, way);

    NfcEccStatus.level_error[way][channel] = 0;
    NfcEccStatus.error[way][channel] = 0;
    NfcEccStatus.correct_sector[way][channel] = 0;
    NfcEccStatus.correct_bit[way][channel] = 0;
    NfcEccStatus.max_correct_bit[way][channel] = 0;

    while (remain_sectors > 0)
    {
        if (curr_blockindex >= info->blocks_per_lun)
        {
            return -1;
        }

        failed = 0;
        row = (curr_blockindex * info->pages_per_block) + curr_pageindex;

        // cmd
        NFC_PHY_1stRead(channel, phyway, row, 0);
        do
        {
            status = NFC_PHY_StatusRead(channel, phyway);
        }while (!NFC_PHY_StatusIsRDY(status));
        // if failed, go next block.
        if (NFC_PHY_StatusIsFAIL(status))
        {
            failed = 1;
        }
        else
        {
            curr_sectors = sectors_per_page - curr_sec_ofs;
            if (curr_sectors > remain_sectors)
                curr_sectors = remain_sectors;

            loop_count = curr_sectors;
            loop_count -= (curr_sec_ofs & 0x1);
            loop_count /= (info->bytes_per_ecc >> 9);
            if (loop_count)
                loop_count -= 1;

            curr_stage = (curr_sec_ofs & 0x1);
            curr_stage |= ((curr_sectors - curr_stage) & 0x1)? 0x4: 0x0;
            curr_stage |= (curr_sectors > (info->bytes_per_ecc >> 9))? 0x2: 0x0;

            if (enable_ecc)
            {
                NfcEccStatus.error[way][channel] = 0;

                col = (curr_sec_ofs / (info->bytes_per_ecc/512)) * (info->bytes_per_ecc + info->bytes_per_parity);
                retryable = ((curr_retry_cnt + 1) < max_retry_cnt)? 1: 0;

                if (Exchange.debug.nfc.phy.info_lowapi)
                {
                    Exchange.sys.fn.print("NFC_PHY_LOWAPI_read: blk:%d, pg:%d, secofs:%d stage:%d, loop:%d \n", curr_blockindex, curr_pageindex, curr_sec_ofs, curr_stage, loop_count);
                }

                NFC_PHY_2ndReadData(curr_stage,
                                    channel, phyway, row, col,
                                    /* fake spare */
                                    0, 0,
                                    /* DATA */
                                    loop_count, info->bytes_per_ecc, info->bytes_per_parity, info->ecc_bits, curr_buff,
                                    /* SPARE */
                                    0, 0, 0, 0,
                                    /* etc */
                                    retryable);

                // check ecc
                if(NfcEccStatus.error[way][channel])
                {
                    curr_retry_cnt += 1;

                    if (curr_retry_cnt < max_retry_cnt)
                    {
                        NfcEccStatus.level_error[way][channel] = 0;
                        NfcEccStatus.error[way][channel] = 0;
                        NfcEccStatus.correct_sector[way][channel] = 0;
                        NfcEccStatus.correct_bit[way][channel] = 0;
                        NfcEccStatus.max_correct_bit[way][channel] = 0;

                        NFC_PHY_READRETRY_SetParameter(channel, phyway);

                        continue;
                    }
                    else
                    {
                        failed = 1;
                    }
                }

                // restore read retry option
                if (curr_retry_cnt)
                {
                    NFC_PHY_READRETRY_Post(channel, phyway);
                }
                
            }
            else
            {
                if (Exchange.debug.nfc.phy.info_lowapi)
                {
                    Exchange.sys.fn.print("NFC_PHY_LOWAPI_read: NoECC blk:%d, pg:%d, secofs:%d, sectors:%d\n", curr_blockindex, curr_pageindex, curr_sec_ofs, curr_sectors);
                }

                col = curr_sec_ofs * 512;
                NFC_PHY_2ndRandomReadDataNoEcc(channel, phyway, col,
                                               /* DATA */
                                               curr_sectors*512, curr_buff);
            }
        }

        curr_retry_cnt = 0;

        if (failed)
        {
            if (low_api.go_nextblock)
            {
                curr_blockindex += 1;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            remain_sectors -= curr_sectors;
            curr_buff += (curr_sectors << 9);

            if (remain_sectors)
            {
                curr_sec_ofs = 0;
                curr_pageindex += 1;
                if (curr_pageindex >= info->pages_per_block)
                {
                    curr_blockindex += 1;
                    curr_pageindex = 0;
                }
            }
        }
    }

    NfcEccStatus.level_error[way][channel] = 0;
    NfcEccStatus.error[way][channel] = 0;
    NfcEccStatus.correct_sector[way][channel] = 0;
    NfcEccStatus.correct_bit[way][channel] = 0;
    NfcEccStatus.max_correct_bit[way][channel] = 0;

    return curr_blockindex;
}

int NFC_PHY_LOWAPI_erase(unsigned int block_ofs, unsigned int block_cnt)
{
    MIO_NAND_INFO *info = &low_api.nandinfo;
    unsigned char channel = info->channel;
    unsigned char phyway = info->phyway;
    unsigned int pages_per_block = info->pages_per_block;
    unsigned int row0=0;
    unsigned char status;
    unsigned int curr_blockindex = block_ofs;

    if (!block_cnt)
    {
        return -1;
    }

    while (block_cnt)
    {
        if (curr_blockindex >= info->blocks_per_lun)
        {
            return -1;
        }

        row0 = curr_blockindex * pages_per_block;

        if (Exchange.debug.nfc.phy.info_lowapi)
        {
            Exchange.sys.fn.print("erase (blk:%4d) ", curr_blockindex);
        }

        // erase block
        NFC_PHY_1stErase(channel, phyway, row0, -1, 0);
        NFC_PHY_2ndErase(channel, phyway);
        do
        {
            status = NFC_PHY_3rdErase(channel, phyway);
        }while (!NFC_PHY_StatusIsRDY(status));

        // if failed, go next block.
        if (NFC_PHY_StatusIsFAIL(status))
        {
            if (Exchange.debug.nfc.phy.info_lowapi)
            {
                Exchange.sys.fn.print("failed!\n");
            }

            if (low_api.go_nextblock)
            {
                curr_blockindex += 1;
                continue;
            }
            else
            {
                return -1;
            }
        }

        if (Exchange.debug.nfc.phy.info_lowapi)
        {
            Exchange.sys.fn.print("done\n");
        }

        block_cnt -= 1;
        if (block_cnt)
        {
            curr_blockindex += 1;
        }
    }

    return curr_blockindex;
}
