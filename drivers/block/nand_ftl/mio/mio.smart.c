/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.smart.c
 * Date         : 2014.10.23
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.10.23)
 *
 * Description  :
 *
 ******************************************************************************/
#define __MIO_SMART_GLOBAL__
#include "mio.smart.h"

#include "mio.definition.h"
#include "media/exchange.h"
#include "mio.block.h"
#include "media/nfc/phy/nfc.phy.h"
#include "media/nfc/phy/nfc.phy.scan.h"
#include "mio.media.h"

/******************************************************************************
 *
 ******************************************************************************/
#define MIOADMIN_PART_USER      (0)
#define MIOADMIN_PART_ADMIN1    (2)
#define MIOADMIN_PART_ADMIN2    (3)

/******************************************************************************
 * local variable
 ******************************************************************************/
static struct
{
    unsigned char is_init;

    unsigned int addr_adminhigh[2];
    unsigned int addr_adminmiddle[2];
    unsigned int addr_adminlow[4];

    unsigned int adminlow_sectors;
    unsigned int adminmiddle_sectors;
    unsigned int adminhigh_sectors;

    unsigned char * rwbuff;
    unsigned int rwbuff_size;

} mioadmin;

static struct
{
    unsigned char is_init;

    NAND * nand;

} miosmart;

/******************************************************************************
 * local functions
 ******************************************************************************/
static int mioadmin_init(void);
static void mioadmin_deinit(void);
static int mioadmin_cmd_to_ftl(unsigned short usCommand, unsigned char ucFeature, unsigned long uiAddress, unsigned int uiLength);
static int mioadmin_write(unsigned char region, void * buff, unsigned int sectors);
static int mioadmin_read(unsigned char region, void * buff, unsigned int sectors, unsigned char sequent);

/******************************************************************************
 * extern functions
 ******************************************************************************/
int miosmart_init(unsigned int _max_channels, unsigned int _max_ways)
{
    int resp = 0;
    unsigned int max_channels = _max_channels;
    unsigned int max_ways = _max_ways;
    unsigned int way = 0, channel = 0;
    unsigned int size = 0;

    miosmart_deinit();
    miosmart.nand = &phy_features.nand_config;

    // io_current, io_accumulate
    MioSmartInfo.io_accumulate.this_size = sizeof(MIO_SMART_COMMON_DATA);
    MioSmartInfo.io_current.this_size = sizeof(MIO_SMART_COMMON_DATA);

    // nand_current, nand_accumulate
    size = max_ways * sizeof(MIO_SMART_CE_DATA *);
    MioSmartInfo.nand_accumulate = (MIO_SMART_CE_DATA **)vmalloc(size);
    MioSmartInfo.nand_current = (MIO_SMART_CE_DATA **)vmalloc(size);

    if (MioSmartInfo.nand_accumulate && MioSmartInfo.nand_current)
    {
        MioSmartInfo.max_ways = max_ways;

        for (way = 0; way < max_ways; way++)
        {
            size = max_channels * sizeof(MIO_SMART_CE_DATA);

            MioSmartInfo.nand_accumulate[way] = (MIO_SMART_CE_DATA *)vmalloc(size);
            MioSmartInfo.nand_current[way] = (MIO_SMART_CE_DATA *)vmalloc(size);

            if (MioSmartInfo.nand_accumulate[way] && MioSmartInfo.nand_current[way])
            {
                MioSmartInfo.max_channels = max_channels;
                memset((void *)(MioSmartInfo.nand_accumulate[way]), 0x00, size);
                memset((void *)(MioSmartInfo.nand_current[way]), 0x00, size);

                for (channel = 0; channel < max_channels; channel++)
                {
                    MioSmartInfo.nand_accumulate[way][channel].this_size = sizeof(MIO_SMART_CE_DATA);
                    MioSmartInfo.nand_current[way][channel].this_size = sizeof(MIO_SMART_CE_DATA);
                }
            }
            else
            {
                printk(KERN_ERR "MIO.SMART.INIT: Memory Allocation Fail (Used For %d WAY)\n", way);
                resp = -1;
            }
        }
    }
    else
    {
        printk(KERN_ERR "MIO.SMART.INIT: Memory Allocation Fail\n");
        resp = -1;
    }

#if 0
    // wearlevel_data
    size = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun * sizeof(unsigned int);
    MioSmartInfo.wearlevel_data = (unsigned int *)vmalloc(size);
    if (!MioSmartInfo.wearlevel_data)
    {
      //DBG_MIOSMART(KERN_INFO "mio.sys: wearlevel: memory alloc: fail");
        resp = -1;
    }
#endif

    if (mioadmin_init() < 0)
    {
        printk(KERN_ERR "MIO.SMART.INIT: FTL Admin Init Fail\n");
        resp = -1;
    }

    // check validation
    if (resp >= 0)
    {
        unsigned int required_bytes = 0;

        required_bytes = sizeof(MIO_SMART_COMMON_DATA);
        required_bytes += sizeof(MIO_SMART_CE_DATA) * max_channels * max_ways;

        if (required_bytes > __SECTOR_SIZEOF(mioadmin.adminhigh_sectors))
        {
            printk(KERN_ERR "MIO.SMART.INIT: Not Enough SMART Data Region\n");
            resp = -1;
        }
    }

    if (resp >= 0)
    {
        miosmart.is_init = 1;
    }

    return resp;
}

int miosmart_is_init(void)
{
    return (miosmart.is_init);
}

void miosmart_deinit(void)
{
    unsigned int max_ways = MioSmartInfo.max_ways;
    unsigned int way = 0;

    // nand_accumulate
    if (MioSmartInfo.nand_accumulate)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (MioSmartInfo.nand_accumulate[way])
            {
                vfree(MioSmartInfo.nand_accumulate[way]);
                MioSmartInfo.nand_accumulate[way] = 0;
            }
            else
            {
                break;
            }
        }

        vfree(MioSmartInfo.nand_accumulate);
        MioSmartInfo.nand_accumulate = 0;
    }

    // nand_current
    if (MioSmartInfo.nand_current)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (MioSmartInfo.nand_current[way])
            {
                vfree(MioSmartInfo.nand_current[way]);
                MioSmartInfo.nand_current[way] = 0;
            }
            else
            {
                break;
            }
        }

        vfree(MioSmartInfo.nand_current);
        MioSmartInfo.nand_current = 0;
    }

#if 0
    // wearlevel_data
    if (MioSmartInfo.wearlevel_data)
    {
        vfree(MioSmartInfo.wearlevel_data);
        MioSmartInfo.wearlevel_data = 0;
    }
#endif

    mioadmin_deinit();

    miosmart.is_init = 0;
}

int miosmart_update_eccstatus(void)
{
    int diff_cnt = 0;
    unsigned int current_new = 0;
    unsigned int * pcurrent = 0;
    unsigned int * paccumulate = 0;
    MIO_SMART_CE_DATA * pnand_accumulate = 0;
    MIO_SMART_CE_DATA * pnand_current = 0;
    DEVICE_SUMMARY * pdevice_summary = 0;
    unsigned int channel = 0, way = 0;

    if (!miosmart_is_init())
    {
        return -1;
    }

    if (!MioSmartInfo.nand_accumulate || !MioSmartInfo.nand_current || !Exchange.statistics.device_summary)
    {
        return -1;
    }

    // ECC info
    for (way = 0; way < *Exchange.ftl.Way; way++)
    {
        for (channel = 0; channel < *Exchange.ftl.Channel; channel++)
        {
            pnand_accumulate = &(MioSmartInfo.nand_accumulate[way][channel]);
            pnand_current = &(MioSmartInfo.nand_current[way][channel]);
            pdevice_summary = &(Exchange.statistics.device_summary[way][channel]);

            if (!pnand_accumulate || !pnand_current || !pdevice_summary)
            {
                return -1;
            }

            // ECC corrected
            current_new = pdevice_summary->uiAccumEccSector;
            pcurrent    = &(pnand_current->ecc_sector.corrected);
            paccumulate = &(pnand_accumulate->ecc_sector.corrected);
            if (*pcurrent < current_new)
            {
                *paccumulate += current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // ECC leveldetected
            current_new = pdevice_summary->uiAccumEccLevel;
            pcurrent    = &(pnand_current->ecc_sector.leveldetected);
            paccumulate = &(pnand_accumulate->ecc_sector.leveldetected);
            if (*pcurrent < current_new)
            {
                *paccumulate += current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // ECC uncorrectable
            current_new = pdevice_summary->uiAccumEccError;
            pcurrent    = &(pnand_current->ecc_sector.uncorrectable);
            paccumulate = &(pnand_accumulate->ecc_sector.uncorrectable);
            if (*pcurrent < current_new)
            {
                *paccumulate += current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // write failcount
            current_new = pdevice_summary->usAccumWriteFail;
            pcurrent    = &(pnand_current->writefail_count);
            paccumulate = &(pnand_accumulate->writefail_count);
            if (*pcurrent < current_new)
            {
                *paccumulate += current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // erase failcount
            current_new = pdevice_summary->usAccumEraseFail;
            pcurrent    = &(pnand_current->erasefail_count);
            paccumulate = &(pnand_accumulate->erasefail_count);
            if (*pcurrent < current_new)
            {
                *paccumulate += current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // Read retry count
            current_new = pdevice_summary->uiAccumReadRetry;
            pcurrent    = &(pnand_current->readretry_count);
            paccumulate = &(pnand_accumulate->readretry_count);
            if (*pcurrent < current_new)
            {
                *paccumulate += current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }
        }
    }

    return diff_cnt;
}

int miosmart_load(void)
{
    int resp = -1;
    void * src_buff = 0;
    unsigned int region = 1;
    unsigned int region_size = mioadmin.adminhigh_sectors;
    unsigned int crc32 = 0;
    unsigned char try_count = 0;
    unsigned char channel = 0, way = 0;
    MIO_SMART_COMMON_DATA * io_data;
    MIO_SMART_CE_DATA     * nand_data;

    if (!miosmart_is_init())
    {
        return -1;
    }

    for (try_count = 0; try_count < 2; try_count++)
    {
        memset((void *)mioadmin.rwbuff, 0, mioadmin.rwbuff_size);

        if (mioadmin_read(region, (void *)mioadmin.rwbuff, region_size, try_count) < 0)
        {
            continue;
        }
        src_buff = (void *)mioadmin.rwbuff;

        // io_accumulate
        io_data = (MIO_SMART_COMMON_DATA *)src_buff;
        if (!io_data || (io_data->this_size < 4) || (io_data->this_size > sizeof(MIO_SMART_COMMON_DATA)))
        {
            continue;
        }

        crc32 = Exchange.sys.fn.get_crc32(0, (void *)(io_data), (io_data->this_size - 4));
        if (io_data->crc32 != crc32)
        {
            DBG_MIOSMART(KERN_WARNING "MIO.SMART.LOAD.IO.DATA: Not Matched Calculated CRC32 (%08x) With Stored CRC32 (%08x)\n", io_data->crc32, crc32);
            continue;
        }

        memcpy((void *)&MioSmartInfo.io_accumulate, (void *)io_data, io_data->this_size);
        src_buff += io_data->this_size;

        // nand_accumulate
        for (way = 0; way < MioSmartInfo.max_ways; way++)
        {
            for (channel = 0; channel < MioSmartInfo.max_channels; channel++)
            {
                nand_data = (MIO_SMART_CE_DATA *)src_buff;
                if (!nand_data || (nand_data->this_size < 4) || (nand_data->this_size > sizeof(MIO_SMART_CE_DATA)))
                {
                    continue;
                }

                crc32 = Exchange.sys.fn.get_crc32(0, (void *)nand_data, (nand_data->this_size - 4));

                if (nand_data->crc32 != crc32)
                {
                    DBG_MIOSMART(KERN_WARNING "MIO.SMART.LOAD.NAND(%d,%d).DATA: Not Matched Calculated CRC32 (%08x) With Stored CRC32 (%08x)\n", channel, way, nand_data->crc32, crc32);
                    continue;
                }

                memcpy((void *)&MioSmartInfo.nand_accumulate[way][channel], (void *)nand_data, nand_data->this_size);
                src_buff += nand_data->this_size;
            }
        }

        // add here
        // ..

        resp = 0;

        break;
    }

    return resp;
}

int miosmart_save(void)
{
    int resp = -1;
    void * dest_buff = 0;
    unsigned int region = 1;
    unsigned int region_size = mioadmin.adminhigh_sectors;
    unsigned char try_count = 0;
    unsigned char channel = 0, way = 0;
    MIO_SMART_COMMON_DATA * io_data;
    MIO_SMART_CE_DATA     * nand_data;
    unsigned int crc32 = 0;
    static unsigned int prev_iodata_crc32 = 0;
    static unsigned int prev_nanddata0c0w_crc32 = 0;

    if (!miosmart_is_init())
    {
        return -1;
    }

    /**********************************************************************
     * make data
     **********************************************************************/
    memset((void *)mioadmin.rwbuff, 0, mioadmin.rwbuff_size);
    {
        dest_buff = (void *)mioadmin.rwbuff;

        // io_accumulate
        io_data = (MIO_SMART_COMMON_DATA *)&MioSmartInfo.io_accumulate;
        io_data->crc32 = Exchange.sys.fn.get_crc32(0, (void *)io_data, (io_data->this_size - 4));
        memcpy(dest_buff, (void *)io_data, io_data->this_size);
        dest_buff += io_data->this_size;

        // nand_accumulate
        for (way = 0; way < MioSmartInfo.max_ways; way++)
        {
            for (channel = 0; channel < MioSmartInfo.max_channels; channel++)
            {
                nand_data = (MIO_SMART_CE_DATA *)&MioSmartInfo.nand_accumulate[way][channel];
                nand_data->crc32 = Exchange.sys.fn.get_crc32(0, (void *)nand_data, (nand_data->this_size - 4));
                memcpy(dest_buff, (void *)nand_data, nand_data->this_size);
                dest_buff += nand_data->this_size;
            }
        }
    }

    /**********************************************************************
     * is need to save ?
     **********************************************************************/
    if ((prev_iodata_crc32 == io_data->crc32) && (prev_nanddata0c0w_crc32 == MioSmartInfo.nand_accumulate[0][0].crc32))
    {
        return 0;
    }

    /**********************************************************************
     * write & verify
     **********************************************************************/
    if (Exchange.debug.misc.smart_store) { Exchange.sys.fn.print("MIO.SMART.STORE: Start\n"); }

    for (try_count = 0; try_count < 2; try_count++)
    {
        if (mioadmin_write(region, (void *)mioadmin.rwbuff, region_size) < 0)
        {
            continue;
        }

        memset((void *)mioadmin.rwbuff, 0, region_size);
        if (mioadmin_read(region, (void *)mioadmin.rwbuff, region_size, 0) < 0)
        {
            continue;
        }

        // io_accumulate
        dest_buff = (void *)mioadmin.rwbuff;
        io_data = (MIO_SMART_COMMON_DATA *)dest_buff;
        crc32 = Exchange.sys.fn.get_crc32(0, (void *)io_data, (io_data->this_size - 4));

        if (io_data->crc32 != crc32)
        {
            // ??
            DBG_MIOSMART(KERN_WARNING "MIO.SMART.STORE.IO.DATA: Not Matched Calculated CRC32 (%08x) With Storing CRC32 (%08x)\n", io_data->crc32, crc32);
            continue;
        }
        dest_buff += io_data->this_size;

        // nand_accumulate
        for (way = 0; way < MioSmartInfo.max_ways; way++)
        {
            for (channel = 0; channel < MioSmartInfo.max_channels; channel++)
            {
                nand_data = (MIO_SMART_CE_DATA *)dest_buff;
                crc32 = Exchange.sys.fn.get_crc32(0, (void *)nand_data, (nand_data->this_size - 4));
                if (nand_data->crc32 != crc32)
                {
                    // ??
                    DBG_MIOSMART(KERN_WARNING "MIO.SMART.STORE.NAND(%d,%d).DATA: Not Matched Calculated CRC32 (%08x) With Storing CRC32 (%08x)\n", channel, way, nand_data->crc32, crc32);
                    continue;
                }

                dest_buff += nand_data->this_size;
            }
        }

        // add here
        // ...

        /**********************************************************************
         * done
         **********************************************************************/
        resp = 0;
        MioSmartInfo.volatile_writesectors = 0;

        prev_iodata_crc32 = MioSmartInfo.io_accumulate.crc32;
        prev_nanddata0c0w_crc32 = MioSmartInfo.nand_accumulate[0][0].crc32;
    }

    if (Exchange.debug.misc.smart_store) { Exchange.sys.fn.print("MIO.SMART.STORE: Stop\n"); }

    return resp;
}

void miosmart_get_erasecount(unsigned int * min, unsigned int * max, unsigned int * sum, unsigned int average[])
{
    unsigned int sum_erasecount = 0, erasecount = 0, validnum_erasecount = 0;
    unsigned int min_erasecount = 0xFFFFFFFF, max_erasecount = 0;
    unsigned int attribute = 0, sub_attribute = 0, partition = 0;
    unsigned char channel = 0, way = 0;
    unsigned char max_channels = MioSmartInfo.max_channels;
    unsigned char max_ways = MioSmartInfo.max_ways;
    unsigned int block = 0;
  //unsigned char bits_per_cell = miosmart.nand->_f.bits_per_cell;
    unsigned int max_blocks = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun;
    unsigned int buff_size = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun * sizeof(unsigned int);
    unsigned int entrydata = 0;
    unsigned int *wearlevel_data = 0;
    unsigned char isvalid_erasecount = 0;

    wearlevel_data = (unsigned int *)vmalloc(buff_size);
    if (!wearlevel_data)
    {
        DBG_MIOSMART(KERN_WARNING "MIO.SMART.GET.ERASE.COUNT: Memory Allocation Fail\n");
        return;
    }

    for (channel = 0; channel < max_channels; channel++)
    {
        for (way = 0; way < max_ways; way++)
        {
            Exchange.ftl.fnGetWearLevelData(channel, way, wearlevel_data, buff_size);

            for (block = 0; block < max_blocks; block++)
            {
                entrydata = *(wearlevel_data + block);

                attribute = (entrydata & 0xF0000000) >> 28;
                partition = (entrydata & 0x0F000000) >> 24;
                sub_attribute = (entrydata & 0x00F00000) >> 20;
                erasecount = (entrydata & 0x000FFFFF);
              //erasecount = __NROOT(erasecount, (bits_per_cell - 1));

                isvalid_erasecount = 0;

                switch (attribute)
                {
                    case BLOCK_TYPE_UPDATE_SEQUENT:
                    case BLOCK_TYPE_UPDATE_RANDOM:
                    case BLOCK_TYPE_DATA_HOT:
                    case BLOCK_TYPE_DATA_HOT_BAD:
                    case BLOCK_TYPE_DATA_COLD:
                    case BLOCK_TYPE_DATA_COLD_BAD:
                    {
                        if (!partition) { isvalid_erasecount = 1; }
                    } break;

                    case BLOCK_TYPE_MAPLOG:
                    case BLOCK_TYPE_FREE:
                    {
                        isvalid_erasecount = 1;

                    } break;

                    default: {} break;
                }

                if (isvalid_erasecount)
                {
                    if (erasecount < min_erasecount) { min_erasecount = erasecount; }
                    if (erasecount > max_erasecount) { max_erasecount = erasecount; }

                    sum_erasecount += erasecount;
                    validnum_erasecount += 1;
                }
            }
        }
    }

    if (min) { *min = min_erasecount; }
    if (max) { *max = max_erasecount; }
    if (sum) { *sum = sum_erasecount; }

    if (average && validnum_erasecount)
    {
        average[0] = sum_erasecount / validnum_erasecount;
        average[1] = ((sum_erasecount % validnum_erasecount) * 100) / validnum_erasecount;
    }

    if (wearlevel_data)
    {
        vfree(wearlevel_data);
    }

    return;
}

unsigned int miosmart_get_total_usableblocks(void)
{
    unsigned int sum_usableblocks = 0, badblocks = 0;
    unsigned char max_channels = MioSmartInfo.max_channels;
    unsigned char max_ways = MioSmartInfo.max_ways;
    unsigned int max_blocks = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun;
    unsigned char channel = 0, way = 0;

    for (channel = 0; channel < max_channels; channel++)
    {
        for (way = 0; way < max_ways; way++)
        {
            badblocks = Exchange.ftl.fnGetBlocksCount(channel, way, BLOCK_TYPE_FBAD, BLOCK_TYPE_IBAD, BLOCK_TYPE_RBAD, 0xFF);
            sum_usableblocks += max_blocks - badblocks;
        }
    }

    return sum_usableblocks;
}

/******************************************************************************
 * local functions
 ******************************************************************************/
int mioadmin_init(void)
{
    int resp = 0;
    unsigned int page_size = 0;
    unsigned int ofs_adminlow = 0;
    unsigned int ofs_adminhigh = 0;
    unsigned int ofs_adminmiddle = 0;

    page_size = (miosmart.nand->_f.databytes_per_page) ? miosmart.nand->_f.databytes_per_page : 8 * 1024;

    mioadmin.adminlow_sectors = __SECTOR_OF_BYTE(page_size);
    mioadmin.adminmiddle_sectors = __SECTOR_OF_BYTE(page_size);
    mioadmin.adminhigh_sectors = __SECTOR_OF_BYTE(page_size);

    // admin low
    ofs_adminlow = ((mioadmin.adminlow_sectors + page_size - 1) / page_size) * page_size;
    mioadmin.addr_adminlow[0] = *Exchange.buffer.BaseOfAdmin1;
    mioadmin.addr_adminlow[1] = mioadmin.addr_adminlow[0] + __SECTOR_OF_BYTE(ofs_adminlow);
    mioadmin.addr_adminlow[2] = mioadmin.addr_adminlow[1] + __SECTOR_OF_BYTE(ofs_adminlow);
    mioadmin.addr_adminlow[3] = mioadmin.addr_adminlow[2] + __SECTOR_OF_BYTE(ofs_adminlow);

    // admin middle/high
    ofs_adminhigh = ((mioadmin.adminhigh_sectors + page_size - 1) / page_size) * page_size;
    ofs_adminmiddle = ((mioadmin.adminmiddle_sectors + page_size - 1) / page_size) * page_size;
    mioadmin.addr_adminhigh[0] = *Exchange.buffer.BaseOfAdmin2;
    mioadmin.addr_adminhigh[1] = mioadmin.addr_adminhigh[0] + __SECTOR_OF_BYTE(ofs_adminhigh);
    mioadmin.addr_adminmiddle[0] = mioadmin.addr_adminhigh[1] + __SECTOR_OF_BYTE(ofs_adminhigh);
    mioadmin.addr_adminmiddle[1] = mioadmin.addr_adminmiddle[0] + __SECTOR_OF_BYTE(ofs_adminmiddle);

    mioadmin.rwbuff = (unsigned char *)vmalloc(page_size);
    if (mioadmin.rwbuff)
    {
        mioadmin.rwbuff_size = page_size;
    }
    else
    {
        printk(KERN_ERR "MIO.ADMIN.INIT: Memory Allocation Fail\n");
        resp = -1;
    }

    if (resp >= 0)
    {
        mioadmin.is_init = 1;
    }

    return resp;
}

void mioadmin_deinit(void)
{
    mioadmin.is_init = 0;

    // admin
    if (mioadmin.rwbuff)
    {
        vfree(mioadmin.rwbuff);
        mioadmin.rwbuff = 0;
    }
}

static int mioadmin_cmd_to_ftl(unsigned short usCommand, unsigned char ucFeature, unsigned long uiAddress, unsigned int uiLength)
{
    int resp = -1;
    int ext_idx = -1;
    unsigned char is_needretry = 0;

    switch (usCommand)
    {
        case IO_CMD_DATA_SET_MANAGEMENT:
        case IO_CMD_FLUSH:
        case IO_CMD_STANDBY:
        case IO_CMD_SWITCH_PARTITION:
        case IO_CMD_POWER_DOWN:
        case IO_CMD_READ_DIRECT:
        case IO_CMD_WRITE_DIRECT:
        case IO_CMD_READ:
        case IO_CMD_WRITE:
        {
            is_needretry = 1;

        } break;
    }

    do
    {
        resp = Exchange.ftl.fnPrePutCommand(usCommand, ucFeature, uiAddress, uiLength);
        if (resp >= 0)
        {
            ext_idx = resp;

            resp = Exchange.ftl.fnPutCommand(usCommand, ucFeature, uiAddress, uiLength);
            if (resp >= 0)
            {
                is_needretry = 0;
            }
        }

        if (is_needretry)
        {
            media_super();
        }

    } while (is_needretry);

    // wait for done
    if (resp >= 0)
    {
        switch (usCommand)
        {
            case IO_CMD_DATA_SET_MANAGEMENT:
            case IO_CMD_FLUSH:
            case IO_CMD_STANDBY:
            case IO_CMD_SWITCH_PARTITION:
            case IO_CMD_POWER_DOWN:
            {
                do
                {
                    media_super();

                } while (Exchange.ftl.fnIsBusy());

            } break;
        }
    }

    return ext_idx;
}

int mioadmin_write(unsigned char region, void * buff, unsigned int sectors)
{
    unsigned char sequent = 0;
    unsigned int address = 0, feature = 0;
    int ext_idx = 0;
    unsigned char *dest_buff = 0;

    if (!mioadmin.is_init)
    {
        return -1;
    }

    if (region) // admin (update high/middle)
    {
        feature = MIOADMIN_PART_ADMIN2;

        // write to NAND
        for (sequent = 0; sequent < 2; sequent++)
        {
            switch (region)
            {
                case 1: { address = mioadmin.addr_adminhigh[sequent]; } break;
                case 2: { address = mioadmin.addr_adminmiddle[sequent]; } break;
            }

            ext_idx = mioadmin_cmd_to_ftl(IO_CMD_WRITE_DIRECT, __POW(feature, 0), address, sectors);

            if (ext_idx >= 0)
            {
                dest_buff = (unsigned char*)(*Exchange.buffer.BaseOfDirectWriteCache + __SECTOR_SIZEOF(ext_idx));
                memcpy((void *)dest_buff, buff, __SECTOR_SIZEOF(sectors));
            }
            do
            {
                media_super();

            } while (Exchange.ftl.fnIsAdminBusy());
        }
    }
    else  // admin (update low)
    {
        unsigned char *end_of_buff = 0;
        unsigned int diffbytes = 0;
        unsigned int mask_of_max_writeindex = (*Exchange.buffer.SectorsOfWriteCache - 1);

        feature = MIOADMIN_PART_ADMIN1;

        mioadmin_cmd_to_ftl(IO_CMD_SWITCH_PARTITION, feature, 0, 0);

        // write
        for (sequent = 0; sequent < 4; sequent++)
        {
            address = mioadmin.addr_adminlow[sequent];

            ext_idx = mioadmin_cmd_to_ftl(IO_CMD_WRITE, 0, address, sectors);
            if (ext_idx >= 0)
            {
                *Exchange.buffer.WriteBlkIdx = (unsigned int)ext_idx;

                dest_buff = (unsigned char *)(*Exchange.buffer.BaseOfWriteCache);
                dest_buff += __SECTOR_SIZEOF(ext_idx & mask_of_max_writeindex);

                end_of_buff = (unsigned char *)(*Exchange.buffer.BaseOfWriteCache) + __SECTOR_SIZEOF(*Exchange.buffer.SectorsOfWriteCache);
                diffbytes = end_of_buff - dest_buff;
                if (diffbytes < sectors)
                {
                    memcpy((void *)dest_buff, (void *)buff, diffbytes);
                    memcpy((void *)(*Exchange.buffer.BaseOfWriteCache), (void *)((unsigned long)buff + diffbytes), (__SECTOR_SIZEOF(sectors) - diffbytes));
                }
                else
                {
                    memcpy((void *)dest_buff, buff, __SECTOR_SIZEOF(sectors));
                }

                *Exchange.buffer.WriteBlkIdx += sectors;
                *Exchange.buffer.WriteBlkIdx &= mask_of_max_writeindex;

            }
        }

        mioadmin_cmd_to_ftl(IO_CMD_SWITCH_PARTITION, MIOADMIN_PART_USER, 0, 0);
    }

#if 0  // print first 1024 bytes of written data
    {
        int i=0, j=0;
        int max = 1024;
        unsigned char textbuff[1024] = {0,};
        unsigned int textbuff_idx = 0;

        i = 0;
        while(i < max)
        {
            if (i%64 == 0)
            {
                textbuff_idx = 0;
                sprintf((void *)(textbuff + textbuff_idx), "[%04x] ", i);   textbuff_idx += 7;
            }

            for (j = i; j < (i+64); j++)
            {
                sprintf((void *)(textbuff + textbuff_idx), "%02x ", dest_buff[j]); textbuff_idx += 3;
            }

            i += 64;

            DBG_MIOSMART(KERN_WARNING "%s \n", textbuff);
        }
    }
#endif

    return 0;
}

int mioadmin_read(unsigned char region, void * buff, unsigned int sectors, unsigned char sequent)
{
    unsigned char feature = 0;
    unsigned int address = 0;
    int ext_idx = -1;
    unsigned char *src_buff = 0;

    if (!mioadmin.is_init)
    {
        return -1;
    }

    if (region)  // Admin(UpdateHigh/Middle)
    {
        feature = MIOADMIN_PART_ADMIN2;

        switch (region)
        {
            case 1: { address = mioadmin.addr_adminhigh[sequent]; } break;
            case 2: { address = mioadmin.addr_adminmiddle[sequent]; } break;
        }

        ext_idx = mioadmin_cmd_to_ftl(IO_CMD_READ_DIRECT, __POW(feature,0), address, sectors);
        if (ext_idx >= 0)
        {
            do
            {
                media_super();
            }while(Exchange.ftl.fnIsAdminBusy());

            src_buff = (unsigned char *)(*Exchange.buffer.BaseOfDirectReadCache + __SECTOR_SIZEOF(ext_idx));
            memcpy (buff, (void *)src_buff, __SECTOR_SIZEOF(sectors));
        }
    }
    else  // Admin(UpdateLow)
    {
        unsigned char *end_of_buff = 0;
        unsigned int diffbytes = 0;
        unsigned int mask_of_max_readindex = (*Exchange.buffer.SectorsOfReadBuffer-1);

        feature = MIOADMIN_PART_ADMIN1;
        address = mioadmin.addr_adminlow[sequent];

        // switch partition
        mioadmin_cmd_to_ftl(IO_CMD_SWITCH_PARTITION, feature, 0, 0);

        // read
        ext_idx = mioadmin_cmd_to_ftl(IO_CMD_READ, 0, address, sectors);
        if (ext_idx >= 0)
        {
            while (1)
            {
                media_super();

                if (((*Exchange.buffer.ReadNfcIdx - ext_idx) & mask_of_max_readindex) >= sectors)
                {
                    break;
                }
            }
            *Exchange.buffer.ReadBlkIdx = (*Exchange.buffer.ReadNfcIdx & mask_of_max_readindex);

            src_buff = (unsigned char *)*Exchange.buffer.BaseOfReadBuffer;
            src_buff += __SECTOR_SIZEOF(ext_idx & mask_of_max_readindex);

            end_of_buff = (unsigned char *)*Exchange.buffer.BaseOfReadBuffer + __SECTOR_SIZEOF(*Exchange.buffer.SectorsOfReadBuffer);
            diffbytes = end_of_buff - src_buff;
            if (diffbytes < sectors)
            {
                memcpy(buff, (void *)src_buff, diffbytes);
                memcpy(buff, (void *)*Exchange.buffer.BaseOfReadBuffer, (__SECTOR_SIZEOF(sectors) - diffbytes));
            }
            else
            {
                memcpy(buff, (void *)src_buff, __SECTOR_SIZEOF(sectors));
            }
        }

        // switch partition
        mioadmin_cmd_to_ftl(IO_CMD_SWITCH_PARTITION, MIOADMIN_PART_USER, 0, 0);
    }

#if 0 // print first 1024 bytes of read data
    {
        int i=0, j=0;
        int max = 1024; //__SECTOR_SIZEOF(512);
        unsigned char textbuff[1024] = {0,};
        unsigned int textbuff_idx = 0;

        i = 0;
        while(i < max)
        {
            if (i%64 == 0)
            {
                textbuff_idx = 0;
                sprintf((void *)(textbuff + textbuff_idx), "[%04x] ", i);   textbuff_idx += 7;
            }

            for (j = i; j < (i+64); j++)
            {
                sprintf((void *)(textbuff + textbuff_idx), "%02x ", ((unsigned char *)buff)[j]); textbuff_idx += 3;
            }
            
            i += 64;

            DBG_MIOSMART(KERN_WARNING "%s \n", textbuff);
        }
    }
#endif

    return 0;
}

