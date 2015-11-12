/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.sys.c
 * Date         : 2014.07.02
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.02 SD.LEE)
 *
 * Description  : API
 *
 ******************************************************************************/
#define __MIO_BLOCK_SYSFS_GLOBAL__
#include "mio.sys.h"
#include "mio.block.h"
#include "media/exchange.h"
#include "mio.definition.h"
#include "mio.smart.h"
#include "mio.media.h"

#include "media/nfc/phy/nfc.phy.lowapi.h"

/******************************************************************************
 *
 ******************************************************************************/
DEFINE_MUTEX(miosys_ioctl_mutex);

/******************************************************************************
 * 
 ******************************************************************************/
static ssize_t miosys_read(struct file * file, char * buf, size_t count, loff_t * ppos);
static ssize_t miosys_write(struct file * file, const char * buf, size_t count, loff_t * ppos);
static long miosys_ioctl(struct file *file, u_int cmd, unsigned long arg);
static long miosys_ioctl_get_user_argument(unsigned int cmd, unsigned long arg, void * arg_buf, unsigned int arg_buf_size);
static long miosys_ioctl_read(unsigned int cmd, unsigned long arg);
static long miosys_ioctl_write(unsigned int cmd, unsigned long arg);
static long miosys_ioctl_erase(unsigned int cmd, unsigned long arg);
static long miosys_ioctl_control(unsigned int cmd, unsigned long arg);
static int miosys_print_wearleveldata(void);
static int miosys_print_smart(void);
static int miosys_print_readretrytable(void);
static int miosys_print_elapsed_t(void);
static int miosys_print_elapsed_media(void);
static int miosys_print_threadstatus(void);
static int miosys_print_ftlinfo(const char * text, unsigned int len);
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
static int miosys_run_randomizer(int);
static int miosys_prt_randomizer(void);
static int miosys_clr_randomizer(void);
#endif

struct file_operations miosys_fops =
{
    .owner = THIS_MODULE,
    .read = miosys_read,
    .write = miosys_write,
    .unlocked_ioctl = miosys_ioctl,
};

struct miscdevice miosys_miscdev =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "miosys",
    .fops = &miosys_fops,
};

/******************************************************************************
 * 
 ******************************************************************************/
int miosys_init(void)
{
    miosys_dev.ioctl_mutex = &miosys_ioctl_mutex;
    miosys_dev.miscdev = &miosys_miscdev;

    return 0;
}

/******************************************************************************
 * 
 ******************************************************************************/
static ssize_t miosys_read(struct file * file, char * buf, size_t count, loff_t * ppos)
{
    char kbuf[16];
  //DBG_MIOSYS(KERN_ERR "miosys_read(file:0x%08x buf:0x%08x count:%d ppos:0x%08x)", (unsigned int)file, (unsigned int)buf, count, (unsigned int)ppos);

    if (count < 256) { return -EINVAL; }
    if (*ppos != 0) { return 0; }
    memset((void *)buf, 0, count);
    if (copy_to_user(buf, kbuf, 16)) { return -EINVAL; }
    *ppos = count;

    return count;
}

char * strtok(register char *s, register const char *delim);

static ssize_t miosys_write(struct file * file, const char * buf, size_t count, loff_t * ppos)
{
    char * cmd_buf = (char *)vmalloc(count+1);
    memset(cmd_buf, 0, count+1);
    if (copy_from_user(cmd_buf, buf, count)) { return -EINVAL; }

  //DBG_MIOSYS(KERN_INFO "miosys_write(file:0x%08x buf:0x%08x count:%d ppos:0x%08x)", (unsigned int)file, (unsigned int)buf, count, (unsigned int)ppos);
               
    // Command Parse
    {
        enum
        {
            MIOSYS_NONE = 0x00000000,

            MIOSYS_REQUEST_SMART = 0x10000000,
            MIOSYS_REQUEST_WEARLEVEL = 0x20000000,
            MIOSYS_REQUEST_READRETRYTABLE = 0x30000000,
            MIOSYS_REQUEST_ELAPSED_T = 0x40000000,
            MIOSYS_REQUEST_ELAPSED_MEDIA = 0x41000000,
            MIOSYS_REQUEST_THREAD_STATUS = 0x50000000,
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
			MIOSYS_REQUEST_RUN_RANDOMIZER = 0x60000000,			// MIO_UNIT_TEST_RANDOMIZER
			MIOSYS_REQUEST_PRT_RANDOMIZER = 0x61000000,
			MIOSYS_REQUEST_CLR_RANDOMIZER = 0x62000000,
#endif
            MIOSYS_MAX = 0xFFFFFFFF

        } state = MIOSYS_NONE;

        char delim[] = " \n";
        char * token = strtok(cmd_buf, delim);
        int breaker = 0;
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
		int run_randomizer_cnt = run_randomizer_cnt;
#endif

        while ((token != NULL) && !breaker)
        {
            switch (state)
            {
                case MIOSYS_NONE:
                {
                    if (!memcmp((const void *)token, (const void *)"smart", strlen("smart")))
                    {
                        state = MIOSYS_REQUEST_SMART;
                    }
                    else if (!memcmp((const void *)token, (const void *)"wearlevel", strlen("wearlevel")))
                    {
                        state = MIOSYS_REQUEST_WEARLEVEL;
                    }
                    else if (!memcmp((const void *)token, (const void *)"readretrytable", strlen("readretrytable")))
                    {
                        state = MIOSYS_REQUEST_READRETRYTABLE;
                    }
                    else if (!memcmp((const void *)token, (const void *)"elapse_t", strlen("elapse_t")))
                    {
                        state = MIOSYS_REQUEST_ELAPSED_T;
                    }
                    else if (!memcmp((const void *)token, (const void *)"elapse_media", strlen("elapse_media")))
                    {
                        state = MIOSYS_REQUEST_ELAPSED_MEDIA;
                    }
                    else if (!memcmp((const void *)token, (const void *)"thread", strlen("thread")))
                    {
                        state = MIOSYS_REQUEST_THREAD_STATUS;
                    }
                    else if (!memcmp((const void *)token, (const void *)"ftl_", strlen("ftl_")))
                    {
                        miosys_print_ftlinfo((const char *)token, strlen(token));
                    }
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
					/***********************************************************************
					 * MIO_UNIT_TEST_RANDOMIZER
					 ***********************************************************************/
					else if (!memcmp((const void *)token, (const void *)"randtest", strlen("randtest")))
					{
						state = MIOSYS_REQUEST_RUN_RANDOMIZER;
					}
					else if (!memcmp((const void *)token, (const void *)"randprint", strlen("randprint")))
					{
						state = MIOSYS_REQUEST_PRT_RANDOMIZER;
					}
					else if (!memcmp((const void *)token, (const void *)"randclear", strlen("randclear")))
					{
						state = MIOSYS_REQUEST_CLR_RANDOMIZER;
					}
#endif
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case MIOSYS_REQUEST_SMART: { breaker = 1; } break;
                case MIOSYS_REQUEST_WEARLEVEL: { breaker = 1; } break;
                case MIOSYS_REQUEST_THREAD_STATUS: { breaker = 1; } break;
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
				case MIOSYS_REQUEST_RUN_RANDOMIZER:
				{
					run_randomizer_cnt = simple_strtoul(token, NULL, 10);
					breaker = 1;
				} break;
				case MIOSYS_REQUEST_PRT_RANDOMIZER: { breaker = 1; } break;
				case MIOSYS_REQUEST_CLR_RANDOMIZER: { breaker = 1; } break;
#endif

                default: { breaker = 1; } break;

            }

            token = strtok(NULL, delim);
        }

        // execute
        switch (state)
        {
            case MIOSYS_REQUEST_SMART:          { miosys_print_smart(); } break;
            case MIOSYS_REQUEST_WEARLEVEL:      { miosys_print_wearleveldata(); } break;
            case MIOSYS_REQUEST_READRETRYTABLE: { miosys_print_readretrytable(); } break;
            case MIOSYS_REQUEST_ELAPSED_T:      { miosys_print_elapsed_t(); } break;
            case MIOSYS_REQUEST_ELAPSED_MEDIA:  { miosys_print_elapsed_media(); } break;
            case MIOSYS_REQUEST_THREAD_STATUS:  { miosys_print_threadstatus(); } break;
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
			case MIOSYS_REQUEST_RUN_RANDOMIZER:
			{
				if (run_randomizer_cnt < 1 || run_randomizer_cnt > 1000000)
					run_randomizer_cnt = 1;
				//Exchange.sys.fn.print("  > run randomize %d times.\n", run_randomizer_cnt);

				miosys_run_randomizer(run_randomizer_cnt);
			} break;
			case MIOSYS_REQUEST_PRT_RANDOMIZER: { miosys_prt_randomizer(); } break;
			case MIOSYS_REQUEST_CLR_RANDOMIZER: { miosys_clr_randomizer(); } break;
#endif

            default: {} break;
        }
    }

    if (cmd_buf)
    {
        vfree(cmd_buf);
    }

    *ppos = count;
    return count;
}


char * strtok(register char *s, register const char *delim)
{
    register char *spanp;
    register int c, sc;
    char *tok;
    static char *last;

    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /* Skip (span) leading delimiters (s += strspn(s, delim), sort of). */
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;)
    {
        if (c == sc)
            goto cont;
    }

    if (c == 0)
    {
        /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /* Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too. */
    for (;;)
    {
        c = *s++;
        spanp = (char *)delim;

        do
        {
            if ((sc = *spanp++) == c)
            {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;

                last = s;

                return (tok);
            }

        } while (sc != 0);
    }
}

int miosys_print_smart(void)
{
    MIO_SMART_CE_DATA *pnand_accumulate = 0;
    MIO_SMART_CE_DATA *pnand_current = 0;
    unsigned int channel = 0, way = 0;
    unsigned int sum_erasecount = 0, sum_usableblocks = 0;
    unsigned int min_erasecount = 0xFFFFFFFF, max_erasecount = 0;
    unsigned int accumulated_sum_readretrycount = 0;
    unsigned int average_erasecount[2] = {123,456};

    if (!miosmart_is_init())
    {
        __PRINT(KERN_ERR "miosys_print_smart: miosmart is not inited!");
        return -1;
    }

    miosmart_update_eccstatus();

    miosmart_get_erasecount(&min_erasecount, &max_erasecount, &sum_erasecount, average_erasecount);
    sum_usableblocks = miosmart_get_total_usableblocks();

    for (way = 0; way < *Exchange.ftl.Way; way++)
    {
        for (channel = 0; channel < *Exchange.ftl.Channel; channel++)
        {
            pnand_accumulate = &(MioSmartInfo.nand_accumulate[way][channel]);

            if (pnand_accumulate)
            {
                accumulated_sum_readretrycount += pnand_accumulate->readretry_count;
            }
        }
    }

    __PRINT("SMART Information\n");

    __PRINT("\n Life Cycle I/O\n");
    __PRINT(" - Read (%lld MB, %lld Sectors) and Retried (%u Times)\n", MioSmartInfo.io_accumulate.read_bytes >> 20, MioSmartInfo.io_accumulate.read_sectors, accumulated_sum_readretrycount);
    __PRINT(" - Write (%lld MB, %lld Sectors)\n", MioSmartInfo.io_accumulate.write_bytes >> 20, MioSmartInfo.io_accumulate.write_sectors);

    __PRINT("\n Power Cycle I/O\n");
    __PRINT(" - Read (%lld MB, %lld Sectors) and Retried (%u Times)\n", MioSmartInfo.io_current.read_bytes >> 20, MioSmartInfo.io_current.read_sectors, *Exchange.ftl.ReadRetryCount);
    __PRINT(" - Write (%lld MB, %lld Sectors)\n", MioSmartInfo.io_current.write_bytes >> 20, MioSmartInfo.io_current.write_sectors);

    __PRINT("\n Erase Status\n");
    __PRINT(" - Total Erase Count:    %u\n", sum_erasecount);
    __PRINT(" - Max Erase Count:      %u\n", max_erasecount);
    __PRINT(" - Min Erase Count:      %u\n", min_erasecount);
    __PRINT(" - Average Erase Count:  %u.%02u\n", average_erasecount[0], average_erasecount[1]);

    __PRINT("\n Block Status\n");
    __PRINT(" - Total Usable Blocks: %u\n", sum_usableblocks);

    for (way = 0; way < *Exchange.ftl.Way; way++)
    {
        for (channel = 0; channel < *Exchange.ftl.Channel; channel++)
        {
            unsigned int max_channel = 0;
            unsigned int max_way = 0;
            NAND nand;

            unsigned int total_block = 0;
            unsigned int total_usable_block = 0;
            unsigned int total_bad_block = 0;
            unsigned int retired_block = 0;
            unsigned int free_block = 0;
            unsigned int used_block = 0;

            Exchange.nfc.fnGetFeatures(&max_channel, &max_way, (void *)&nand);
            total_block = nand._f.mainblocks_per_lun;

            pnand_accumulate = &(MioSmartInfo.nand_accumulate[way][channel]);
            pnand_current = &(MioSmartInfo.nand_current[way][channel]);

            if (!pnand_accumulate || !pnand_current)
            {
                return -1;
            }

            total_usable_block = total_block - Exchange.ftl.fnGetBlocksCount(channel, way, BLOCK_TYPE_DATA_HOT_BAD, BLOCK_TYPE_DATA_COLD_BAD, BLOCK_TYPE_FBAD, BLOCK_TYPE_IBAD, BLOCK_TYPE_RBAD, 0xFF);
            total_bad_block    = Exchange.ftl.fnGetBlocksCount(channel, way, BLOCK_TYPE_DATA_HOT_BAD, BLOCK_TYPE_DATA_COLD_BAD, BLOCK_TYPE_FBAD, BLOCK_TYPE_IBAD, BLOCK_TYPE_RBAD, 0xFF);
            retired_block      = Exchange.ftl.fnGetBlocksCount(channel, way, BLOCK_TYPE_DATA_HOT_BAD, BLOCK_TYPE_DATA_COLD_BAD, BLOCK_TYPE_RBAD, 0xFF);
            free_block         = Exchange.ftl.fnGetBlocksCount(channel, way, BLOCK_TYPE_FREE, 0xFF);
            used_block         = total_usable_block - free_block;

            __PRINT("\n NAND CHANNEL:%d WAY:%d Information\n", channel, way);

            /******************************************************************
             * ECC Information
             ******************************************************************/

            __PRINT("\n  ECC Status                    Power Cycle          Life Cycle\n");
            __PRINT("  - Corrected Sectors:           %10u %19u\n", pnand_current->ecc_sector.corrected, pnand_accumulate->ecc_sector.corrected);
            __PRINT("  - Level Detected Sectors:      %10u %19u\n", pnand_current->ecc_sector.leveldetected, pnand_accumulate->ecc_sector.leveldetected);
            __PRINT("  - Uncorrectable Sectors:       %10u %19u\n", pnand_current->ecc_sector.uncorrectable, pnand_accumulate->ecc_sector.uncorrectable);

            __PRINT("\n  Fail Status                   Power Cycle          Life Cycle\n");
            __PRINT("  - Write Fail Count:            %10u %19u\n", pnand_current->writefail_count, pnand_accumulate->writefail_count);
            __PRINT("  - Erase Fail Count:            %10u %19u\n", pnand_current->erasefail_count, pnand_accumulate->erasefail_count);

            __PRINT("\n  Retry Status                  Power Cycle          Life Cycle\n");
            __PRINT("  - Read Retry Count:            %10u %19u\n", pnand_current->readretry_count, pnand_accumulate->readretry_count);

            __PRINT("\n  Block Status\n");
            __PRINT("  - Total Block:        %d\n", total_block);
            __PRINT("  - Total Usable Block: %d\n", total_usable_block);
            __PRINT("  - Total Bad Block:    %d\n", total_bad_block);
            __PRINT("  - Retired Block:      %d\n", retired_block);
            __PRINT("  - Free Block:         %d\n", free_block);
            __PRINT("  - Used Block:         %d\n", used_block);

        }
    }
    __PRINT(" \n");

    return 0;
}

int miosys_print_wearleveldata(void)
{
    unsigned char channel = 0;
    unsigned char way = 0;
    unsigned int *buff = 0;
    unsigned int buff_size = 0;
    NAND nand;
    int blockindex = 0;
    unsigned int entrydata = 0;
    unsigned int attribute = 0, sub_attribute = 0;
    unsigned int erasecount = 0, partition = 0;
    unsigned int badblock_count = 0;
    unsigned int min_erasecount = 0xFFFFFFFF, max_erasecount = 0;
    unsigned int average_erasecount[2] = {0,0};
    unsigned char isvalid_erasecount = 0;
    unsigned int validnum_erasecount = 0, sum_erasecount = 0;


    if (!miosmart_is_init())
    {
        __PRINT(KERN_ERR "miosys_print_smart: miosmart is not inited!");
        return -1;
    }

    // create buffer
    Exchange.ftl.fnGetNandInfo(&nand);
    buff_size = nand._f.luns_per_ce * nand._f.mainblocks_per_lun * sizeof(unsigned int);

  //DBG_MIOSYS(KERN_INFO "buffer size:%d", buff_size);

    buff = (unsigned int *)vmalloc(buff_size);
    if (!buff)
    {
        __PRINT(KERN_ERR "mio.sys: wearlevel: memory alloc: fail");
        return -1;
    }

    for (channel=0; channel < *Exchange.ftl.Channel; channel++)
    {
        for (way=0; way < *Exchange.ftl.Way; way++)
        {
            __PRINT("NAND CH%02d-WAY%02d\n", channel, way);

            // Get Wearlevel data
            Exchange.ftl.fnGetWearLevelData(channel, way, buff, buff_size);

            // print wearlevel data
            for (blockindex = 0; blockindex < nand._f.mainblocks_per_lun; blockindex++)
            {
                isvalid_erasecount = 0;
                entrydata = *(buff + blockindex);
                attribute = (entrydata & 0xF0000000) >> 28;
                partition = (entrydata & 0x0F000000) >> 24;
                sub_attribute = (entrydata & 0x00F00000) >> 20;
                erasecount = (entrydata & 0x000FFFFF);

                switch (attribute)
                {
                    case BLOCK_TYPE_UPDATE_SEQUENT:
                    case BLOCK_TYPE_UPDATE_RANDOM:
                    case BLOCK_TYPE_DATA_HOT:
                    case BLOCK_TYPE_DATA_HOT_BAD:
                    case BLOCK_TYPE_DATA_COLD:
                    case BLOCK_TYPE_DATA_COLD_BAD:
                    {
                        if (!partition) { __PRINT(" %08x BLOCK:%5d EraseCount:%5d DATA\n", entrydata, blockindex, erasecount); isvalid_erasecount = 1; }
                        else            { __PRINT(" %08x BLOCK:%5d EraseCount:%5d DATA (ADMIN)\n", entrydata, blockindex, erasecount); }
                    } break;

                    case BLOCK_TYPE_MAPLOG: { __PRINT(" %08x BLOCK:%5d EraseCount:%5d SYSTEM (M)\n", entrydata, blockindex, erasecount); isvalid_erasecount = 1; } break;
                    case BLOCK_TYPE_FREE:   { __PRINT(" %08x BLOCK:%5d EraseCount:%5d FREE\n", entrydata, blockindex, erasecount); isvalid_erasecount = 1; } break;
                    case 0xA:
                    {
                        switch (sub_attribute)
                        {
                            case 0x7:             { __PRINT(" %08x BLOCK:%5d PROHIBIT\n", entrydata, blockindex); } break;
                            case 0x8:             { __PRINT(" %08x BLOCK:%5d CONFIG\n", entrydata, blockindex); } break;
                            case 0x9:             { __PRINT(" %08x BLOCK:%5d SYSTEM (RETRY)\n", entrydata, blockindex); } break;
                            case BLOCK_TYPE_IBAD: { __PRINT(" %08x BLOCK:%5d BAD (Initial)\n", entrydata, blockindex); badblock_count++; } break;
                            case BLOCK_TYPE_FBAD: { __PRINT(" %08x BLOCK:%5d BAD (Factory)\n", entrydata, blockindex); badblock_count++; } break;
                            case BLOCK_TYPE_RBAD: { __PRINT(" %08x BLOCK:%5d BAD (Runtime)\n", entrydata, blockindex); badblock_count++; } break;
                            case BLOCK_TYPE_ROOT: { __PRINT(" %08x BLOCK:%5d SYSTEM (ROOT)\n", entrydata, blockindex); } break;
                            case BLOCK_TYPE_ENED: { __PRINT(" %08x BLOCK:%5d SYSTEM (ENED)\n", entrydata, blockindex); } break;
                            case BLOCK_TYPE_FIRM: { __PRINT(" %08x BLOCK:%5d SYSTEM (FW)\n", entrydata, blockindex); } break;
                            default:              { __PRINT(" %08x BLOCK %5d SYSTEM (0x%x)\n", entrydata, blockindex, sub_attribute); } break;
                        }
                    } break;
                    default:  { __PRINT( " %08x BLOCK %5d, unknown (0x%02x)\n", entrydata, blockindex, attribute); } break;
                }

                // erasecount: max/min/average
                if (isvalid_erasecount)
                {
                    if (erasecount < min_erasecount) { min_erasecount = erasecount; }
                    if (erasecount > max_erasecount) { max_erasecount = erasecount; }

                    sum_erasecount += erasecount;
                    validnum_erasecount += 1;
                }
            }

            if (validnum_erasecount)
            {
                average_erasecount[0] = sum_erasecount / validnum_erasecount;
                average_erasecount[1] = ((sum_erasecount % validnum_erasecount) * 100) / validnum_erasecount;
            }

            __PRINT( "bad blocks %d\n", badblock_count);
            __PRINT( "max erasecount %5d\n", max_erasecount);
            __PRINT( "min erasecount %5d\n", min_erasecount);
            __PRINT( "average erasecount %d.%02d\n", average_erasecount[0], average_erasecount[1]);
        }
    }
    __PRINT( " \n");

    // destory buffer
    vfree(buff);

    return 0;
}

int miosys_print_readretrytable(void)
{
    if (Exchange.nfc.fnReadRetry_PrintTable)
    {
        Exchange.nfc.fnReadRetry_PrintTable();
    }
    return 0;
}

#ifdef __MIO_UNIT_TEST_RANDOMIZER__
extern void NFC_PHY_RAND_Test(int cnt);
int miosys_run_randomizer(int cnt)
{
	NFC_PHY_RAND_Test(cnt);

    return 0;
}

extern void NFC_PHY_RAND_Print(void);
int miosys_prt_randomizer(void)
{
	NFC_PHY_RAND_Print();

    return 0;
}

void NFC_PHY_RAND_Clear(void);
int miosys_clr_randomizer(void)
{
	NFC_PHY_RAND_Clear();

	return 0;
}
#endif /* __MIO_UNIT_TEST_RANDOMIZER__ */


int miosys_print_elapsed_t(void)
{
#if defined (__COMPILE_MODE_ELAPSE_T__)
    unsigned char sz[32];

    unsigned int sum_roundup = Exchange.debug.elapse_t.sum_roundup;
    unsigned long long * sum = Exchange.debug.elapse_t.sum;
    unsigned long long * avg = Exchange.debug.elapse_t.avg;
    unsigned long long * min = Exchange.debug.elapse_t.min;
    unsigned long long * max = Exchange.debug.elapse_t.max;

    unsigned long long gt_sum     = sum[ELAPSE_T_TRANSACTION_THREAD_BACKGROUND] + sum[ELAPSE_T_TRANSACTION_THREAD_SCHEDULED] + sum[ELAPSE_T_TRANSACTION_THREAD_IO];
    unsigned long long sum_nfc_rw = sum[ELAPSE_T_IO_NFC_RW] - (sum[ELAPSE_T_IO_NFC_RANDOMIZER_RW] + sum[ELAPSE_T_IO_NFC_DELAY_RW]);
    unsigned long long avg_nfc_rw = avg[ELAPSE_T_IO_NFC_RW] - (avg[ELAPSE_T_IO_NFC_RANDOMIZER_RW] + avg[ELAPSE_T_IO_NFC_DELAY_RW]);
    unsigned long long min_nfc_rw = min[ELAPSE_T_IO_NFC_RW] - (min[ELAPSE_T_IO_NFC_RANDOMIZER_RW] + min[ELAPSE_T_IO_NFC_DELAY_RW]);
    unsigned long long max_nfc_rw = max[ELAPSE_T_IO_NFC_RW] - (max[ELAPSE_T_IO_NFC_RANDOMIZER_RW] + max[ELAPSE_T_IO_NFC_DELAY_RW]);
    unsigned long long sum_nfc_r  = sum[ELAPSE_T_IO_NFC_R]  - (sum[ELAPSE_T_IO_NFC_RANDOMIZER_R]  + sum[ELAPSE_T_IO_NFC_DELAY_R]);
    unsigned long long avg_nfc_r  = avg[ELAPSE_T_IO_NFC_R]  - (avg[ELAPSE_T_IO_NFC_RANDOMIZER_R]  + avg[ELAPSE_T_IO_NFC_DELAY_R]);
    unsigned long long min_nfc_r  = min[ELAPSE_T_IO_NFC_R]  - (min[ELAPSE_T_IO_NFC_RANDOMIZER_R]  + min[ELAPSE_T_IO_NFC_DELAY_R]);
    unsigned long long max_nfc_r  = max[ELAPSE_T_IO_NFC_R]  - (max[ELAPSE_T_IO_NFC_RANDOMIZER_R]  + max[ELAPSE_T_IO_NFC_DELAY_R]);
    unsigned long long sum_nfc_w  = sum[ELAPSE_T_IO_NFC_W]  - (sum[ELAPSE_T_IO_NFC_RANDOMIZER_W]  + sum[ELAPSE_T_IO_NFC_DELAY_W]);
    unsigned long long avg_nfc_w  = avg[ELAPSE_T_IO_NFC_W]  - (avg[ELAPSE_T_IO_NFC_RANDOMIZER_W]  + avg[ELAPSE_T_IO_NFC_DELAY_W]);
    unsigned long long min_nfc_w  = min[ELAPSE_T_IO_NFC_W]  - (min[ELAPSE_T_IO_NFC_RANDOMIZER_W]  + min[ELAPSE_T_IO_NFC_DELAY_W]);
    unsigned long long max_nfc_w  = max[ELAPSE_T_IO_NFC_W]  - (max[ELAPSE_T_IO_NFC_RANDOMIZER_W]  + max[ELAPSE_T_IO_NFC_DELAY_W]);

    sum[ELAPSE_T_IO_FTL_RW] = sum[ELAPSE_T_IO_MEDIA_RW] - sum_nfc_rw;
    avg[ELAPSE_T_IO_FTL_RW] = avg[ELAPSE_T_IO_MEDIA_RW] - avg_nfc_rw;
    min[ELAPSE_T_IO_FTL_RW] = min[ELAPSE_T_IO_MEDIA_RW] - min_nfc_rw;
    max[ELAPSE_T_IO_FTL_RW] = max[ELAPSE_T_IO_MEDIA_RW] - max_nfc_rw;
    sum[ELAPSE_T_IO_FTL_R]  = sum[ELAPSE_T_IO_MEDIA_R]  - sum_nfc_r;
    avg[ELAPSE_T_IO_FTL_R]  = avg[ELAPSE_T_IO_MEDIA_R]  - avg_nfc_r;
    min[ELAPSE_T_IO_FTL_R]  = min[ELAPSE_T_IO_MEDIA_R]  - min_nfc_r;
    max[ELAPSE_T_IO_FTL_R]  = max[ELAPSE_T_IO_MEDIA_R]  - max_nfc_r;
    sum[ELAPSE_T_IO_FTL_W]  = sum[ELAPSE_T_IO_MEDIA_W]  - sum_nfc_w;
    avg[ELAPSE_T_IO_FTL_W]  = avg[ELAPSE_T_IO_MEDIA_W]  - avg_nfc_w;
    min[ELAPSE_T_IO_FTL_W]  = min[ELAPSE_T_IO_MEDIA_W]  - min_nfc_w;
    max[ELAPSE_T_IO_FTL_W]  = max[ELAPSE_T_IO_MEDIA_W]  - max_nfc_w;
                                                                                                            __PRINT("\n");
                                                                                                            __PRINT(" Measured On Transaction Thread (Measured Unit is nano second)\n");
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
                                                                                                            __PRINT(" |                       |     %10d x Sum |             Avg |             Min |             Max |     Ratio |\n", sum_roundup);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
    Exchange.sys.fn.ratio(sz, gt_sum, sum[ELAPSE_T_TRANSACTION_THREAD_BACKGROUND]);                         __PRINT(" | BackGround            | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_TRANSACTION_THREAD_BACKGROUND], avg[ELAPSE_T_TRANSACTION_THREAD_BACKGROUND], min[ELAPSE_T_TRANSACTION_THREAD_BACKGROUND], max[ELAPSE_T_TRANSACTION_THREAD_BACKGROUND], sz);
    Exchange.sys.fn.ratio(sz, gt_sum, sum[ELAPSE_T_TRANSACTION_THREAD_SCHEDULED ]);                         __PRINT(" | Scheduled             | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_TRANSACTION_THREAD_SCHEDULED ], avg[ELAPSE_T_TRANSACTION_THREAD_SCHEDULED ], min[ELAPSE_T_TRANSACTION_THREAD_SCHEDULED ], max[ELAPSE_T_TRANSACTION_THREAD_SCHEDULED ], sz);
    Exchange.sys.fn.ratio(sz, gt_sum, sum[ELAPSE_T_TRANSACTION_THREAD_IO        ]);                         __PRINT(" | Transaction           | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_TRANSACTION_THREAD_IO        ], avg[ELAPSE_T_TRANSACTION_THREAD_IO        ], min[ELAPSE_T_TRANSACTION_THREAD_IO        ], max[ELAPSE_T_TRANSACTION_THREAD_IO        ], sz);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
                                                                                                            __PRINT("\n");
                                                                                                            __PRINT(" Measured On Transaction (Measured Unit is nano second)\n");
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
                                                                                                            __PRINT(" |                       |     %10d x Sum |             Avg |             Min |             Max |     Ratio |\n", sum_roundup);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_TRANSACTION_THREAD_IO], sum[ELAPSE_T_TRANSACTION_THREAD_IO]);    __PRINT(" | Transaction           | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_TRANSACTION_THREAD_IO], avg[ELAPSE_T_TRANSACTION_THREAD_IO], min[ELAPSE_T_TRANSACTION_THREAD_IO], max[ELAPSE_T_TRANSACTION_THREAD_IO], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_TRANSACTION_THREAD_IO], sum[ELAPSE_T_IO_MEDIA_DISCARD     ]);    __PRINT(" | - Media Discard       | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEDIA_DISCARD     ], avg[ELAPSE_T_IO_MEDIA_DISCARD     ], min[ELAPSE_T_IO_MEDIA_DISCARD     ], max[ELAPSE_T_IO_MEDIA_DISCARD     ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_TRANSACTION_THREAD_IO], sum[ELAPSE_T_IO_MEDIA_FLUSH       ]);    __PRINT(" | - Media Flush         | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEDIA_FLUSH       ], avg[ELAPSE_T_IO_MEDIA_FLUSH       ], min[ELAPSE_T_IO_MEDIA_FLUSH       ], max[ELAPSE_T_IO_MEDIA_FLUSH       ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_TRANSACTION_THREAD_IO], sum[ELAPSE_T_IO_MEDIA_RW          ]);    __PRINT(" | - Media I/O           | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEDIA_RW          ], avg[ELAPSE_T_IO_MEDIA_RW          ], min[ELAPSE_T_IO_MEDIA_RW          ], max[ELAPSE_T_IO_MEDIA_RW          ], sz);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
                                                                                                            __PRINT("\n\n");
                                                                                                            __PRINT(" Measured On Media I/O (Measured Unit is nano second)\n");
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
                                                                                                            __PRINT(" |                       |     %10d x Sum |             Avg |             Min |             Max |     Ratio |\n", sum_roundup);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_MEDIA_RW          ]);    __PRINT(" | Media I/O         R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEDIA_RW          ], avg[ELAPSE_T_IO_MEDIA_RW          ], min[ELAPSE_T_IO_MEDIA_RW          ], max[ELAPSE_T_IO_MEDIA_RW          ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_FTL_RW            ]);    __PRINT(" | - FTL             R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_FTL_RW            ], avg[ELAPSE_T_IO_FTL_RW            ], min[ELAPSE_T_IO_FTL_RW            ], max[ELAPSE_T_IO_FTL_RW            ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum_nfc_rw                         );    __PRINT(" | - NFC             R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum_nfc_rw                         , avg_nfc_rw                         , min_nfc_rw                         , max_nfc_rw                         , sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_NFC_RANDOMIZER_RW ]);    __PRINT(" | - NFC Randomizer  R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_NFC_RANDOMIZER_RW ], avg[ELAPSE_T_IO_NFC_RANDOMIZER_RW ], min[ELAPSE_T_IO_NFC_RANDOMIZER_RW ], max[ELAPSE_T_IO_NFC_RANDOMIZER_RW ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_NFC_DELAY_RW      ]);    __PRINT(" | - NFC Delay       R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_NFC_DELAY_RW      ], avg[ELAPSE_T_IO_NFC_DELAY_RW      ], min[ELAPSE_T_IO_NFC_DELAY_RW      ], max[ELAPSE_T_IO_NFC_DELAY_RW      ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_MEMIO_RW          ]);    __PRINT(" | - Memory I/O      R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEMIO_RW          ], avg[ELAPSE_T_IO_MEMIO_RW          ], min[ELAPSE_T_IO_MEMIO_RW          ], max[ELAPSE_T_IO_MEMIO_RW          ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_FTL_MAP_SEARCH_RW ]);    __PRINT(" | - Ftl Map Search  R/W | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_FTL_MAP_SEARCH_RW ], avg[ELAPSE_T_IO_FTL_MAP_SEARCH_RW ], min[ELAPSE_T_IO_FTL_MAP_SEARCH_RW ], max[ELAPSE_T_IO_FTL_MAP_SEARCH_RW ], sz);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_MEDIA_R           ]);    __PRINT(" | Media I/O         RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEDIA_R           ], avg[ELAPSE_T_IO_MEDIA_R           ], min[ELAPSE_T_IO_MEDIA_R           ], max[ELAPSE_T_IO_MEDIA_R           ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_FTL_R             ]);    __PRINT(" | - FTL             RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_FTL_R             ], avg[ELAPSE_T_IO_FTL_R             ], min[ELAPSE_T_IO_FTL_R             ], max[ELAPSE_T_IO_FTL_R             ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum_nfc_r                          );    __PRINT(" | - NFC             RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum_nfc_r                          , avg_nfc_r                          , min_nfc_r                          , max_nfc_r                          , sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_NFC_RANDOMIZER_R  ]);    __PRINT(" | - NFC Randomizer  RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_NFC_RANDOMIZER_R  ], avg[ELAPSE_T_IO_NFC_RANDOMIZER_R  ], min[ELAPSE_T_IO_NFC_RANDOMIZER_R  ], max[ELAPSE_T_IO_NFC_RANDOMIZER_R  ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_NFC_DELAY_R       ]);    __PRINT(" | - NFC Delay       RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_NFC_DELAY_R       ], avg[ELAPSE_T_IO_NFC_DELAY_R       ], min[ELAPSE_T_IO_NFC_DELAY_R       ], max[ELAPSE_T_IO_NFC_DELAY_R       ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_MEMIO_R           ]);    __PRINT(" | - Memory I/O      RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEMIO_R           ], avg[ELAPSE_T_IO_MEMIO_R           ], min[ELAPSE_T_IO_MEMIO_R           ], max[ELAPSE_T_IO_MEMIO_R           ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_FTL_MAP_SEARCH_R  ]);    __PRINT(" | - Ftl Map Search  RD  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_FTL_MAP_SEARCH_R  ], avg[ELAPSE_T_IO_FTL_MAP_SEARCH_R  ], min[ELAPSE_T_IO_FTL_MAP_SEARCH_R  ], max[ELAPSE_T_IO_FTL_MAP_SEARCH_R  ], sz);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_MEDIA_W           ]);    __PRINT(" | Media I/O         WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEDIA_W           ], avg[ELAPSE_T_IO_MEDIA_W           ], min[ELAPSE_T_IO_MEDIA_W           ], max[ELAPSE_T_IO_MEDIA_W           ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_FTL_W             ]);    __PRINT(" | - FTL             WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_FTL_W             ], avg[ELAPSE_T_IO_FTL_W             ], min[ELAPSE_T_IO_FTL_W             ], max[ELAPSE_T_IO_FTL_W             ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum_nfc_w                          );    __PRINT(" | - NFC             WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum_nfc_w                          , avg_nfc_w                          , min_nfc_w                          , max_nfc_w                          , sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_NFC_RANDOMIZER_W  ]);    __PRINT(" | - NFC Randomizer  WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_NFC_RANDOMIZER_W  ], avg[ELAPSE_T_IO_NFC_RANDOMIZER_W  ], min[ELAPSE_T_IO_NFC_RANDOMIZER_W  ], max[ELAPSE_T_IO_NFC_RANDOMIZER_W  ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_NFC_DELAY_W       ]);    __PRINT(" | - NFC Delay       WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_NFC_DELAY_W       ], avg[ELAPSE_T_IO_NFC_DELAY_W       ], min[ELAPSE_T_IO_NFC_DELAY_W       ], max[ELAPSE_T_IO_NFC_DELAY_W       ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_MEMIO_W           ]);    __PRINT(" | - Memory I/O      WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_MEMIO_W           ], avg[ELAPSE_T_IO_MEMIO_W           ], min[ELAPSE_T_IO_MEMIO_W           ], max[ELAPSE_T_IO_MEMIO_W           ], sz);
    Exchange.sys.fn.ratio(sz, sum[ELAPSE_T_IO_MEDIA_RW          ], sum[ELAPSE_T_IO_FTL_MAP_SEARCH_W  ]);    __PRINT(" | - Ftl Map Search  WR  | %20lld | %15lld | %15lld | %15lld | %s |\n", sum[ELAPSE_T_IO_FTL_MAP_SEARCH_W  ], avg[ELAPSE_T_IO_FTL_MAP_SEARCH_W  ], min[ELAPSE_T_IO_FTL_MAP_SEARCH_W  ], max[ELAPSE_T_IO_FTL_MAP_SEARCH_W  ], sz);
                                                                                                            __PRINT(" +-----------------------+----------------------+-----------------+-----------------+-----------------+-----------+\n");
                                                                                                            __PRINT("\n");
#endif

    return 0;
}

/******************************************************************************
 * 
 ******************************************************************************/
int miosys_print_elapsed_media(void)
{
    ELAPSED_MEDIA_IO *media_io = 0;

    __PRINT("Media Read/Write Elapsed time\n");

    media_io = &(elapsed_mio_media_info.max_read);
    __PRINT("\n Max Elapsed Time\n");
    __PRINT(" - READ     : %5lld ms (%11lldns LBA:0x%08x Seccnt:0x%04x)\n", div_u64(media_io->elapsed_ns,1000000), media_io->elapsed_ns, media_io->lba, media_io->seccnt);
    media_io = &(elapsed_mio_media_info.max_write);
    __PRINT(" - WRITE    : %5lld ms (%11lldns LBA:0x%08x Seccnt:0x%04x)\n", div_u64(media_io->elapsed_ns,1000000), media_io->elapsed_ns, media_io->lba, media_io->seccnt);
    media_io = &(elapsed_mio_media_info.max_flush);
    __PRINT(" - FLUSH    : %5lld ms (%11lldns)\n", div_u64(media_io->elapsed_ns,1000000), media_io->elapsed_ns);
    media_io = &(elapsed_mio_media_info.max_trim);
    __PRINT(" - TRIM     : %5lld ms (%11lldns)\n", div_u64(media_io->elapsed_ns,1000000), media_io->elapsed_ns);
    media_io = &(elapsed_mio_media_info.max_standby);
    __PRINT(" - STANDBY  : %5lld ms (%11lldns)\n", div_u64(media_io->elapsed_ns,1000000), media_io->elapsed_ns);
    media_io = &(elapsed_mio_media_info.max_powerdown);
    __PRINT(" - POWERDOWN: %5lld ms (%11lldns)\n", div_u64(media_io->elapsed_ns,1000000), media_io->elapsed_ns);
    __PRINT("\n");

    return 0;
}

/******************************************************************************
 * 
 ******************************************************************************/
static int miosys_print_threadstatus(void)
{
    struct mio_state * io_state = mio_dev.io_state;

    __PRINT("MIO THREAD Information\n\n");
    __PRINT(" - miosys_dev.ioctl_mutex lock: %d\n", mutex_is_locked(miosys_dev.ioctl_mutex));
    __PRINT(" - mio_dev.io_state->transaction.status(%d): ", mio_dev.io_state->transaction.status);
    switch (io_state->transaction.status)
    {
        case MIO_IDLE:       { __PRINT("MIO_IDLE"); } break;
        case MIO_SCHEDULED:  { __PRINT("MIO_SCHEDULED"); } break;
        case MIO_BACKGROUND: { __PRINT("MIO_BACKGROUND"); } break;
        case MIO_SUPER:      { __PRINT("MIO_SUPER"); } break;
        case MIO_REQ_BUSY:   { __PRINT("MIO_REQ_BUSY"); } break;
        default: {} break;
    }
    __PRINT("\n\n");

    return 0;
}

/******************************************************************************
 * 
 ******************************************************************************/
static int miosys_print_ftlinfo(const char * text, unsigned int len)
{
    if (Exchange.ftl.fnPrintInfo)
    {
        unsigned int headlen = strlen("ftl_");
        
        if (len > headlen)
        {
            Exchange.ftl.fnPrintInfo(text+headlen, len-headlen);
        }
    }

    return 0;
}

/******************************************************************************
 * 
 ******************************************************************************/
long miosys_ioctl(struct file *file, u_int cmd, unsigned long arg)
{
    long ret = 0;

#if 0
    __PRINT("miosys_ioctl:CMD:%08x", cmd);
    switch (cmd)
    {
        case  MIOSYS_MEDIA_READ   : { __PRINT("(MIOSYS_MEDIA_READ   )"); } break;
        case  MIOSYS_MEDIA_WRITE  : { __PRINT("(MIOSYS_MEDIA_WRITE  )"); } break;
        case  MIOSYS_NAND_READ    : { __PRINT("(MIOSYS_NAND_READ    )"); } break;
        case  MIOSYS_NAND_WRITE   : { __PRINT("(MIOSYS_NAND_WRITE   )"); } break;
        case  MIOSYS_NAND_ERASE   : { __PRINT("(MIOSYS_NAND_ERASE   )"); } break;
        case  MIOSYS_NANDRAW_READ : { __PRINT("(MIOSYS_NANDRAW_READ )"); } break;
        case  MIOSYS_NANDRAW_WRITE: { __PRINT("(MIOSYS_NANDRAW_WRITE)"); } break;
        case  MIOSYS_NANDRAW_ERASE: { __PRINT("(MIOSYS_NANDRAW_ERASE)"); } break;
        default: {} break;
    }
    __PRINT(" arg:%08lx\n", arg);
#endif

    mutex_lock(miosys_dev.ioctl_mutex);
    {
        // prerequisite - standby command
        while (1)
        {
            if (Exchange.ftl.fnMain)
            {
                Exchange.ftl.fnMain();
            }

            if (Exchange.ftl.fnIsReady())
            {
                if (Exchange.ftl.fnPrePutCommand(IO_CMD_STANDBY, 0, 0, 0) >= 0)
                {
                    Exchange.ftl.fnPutCommand(IO_CMD_STANDBY, 0, 0, 0);
                    break;
                }
            }
        }

        while (Exchange.ftl.fnIsBusy())
        {
            if (Exchange.ftl.fnMain)
            {
                Exchange.ftl.fnMain();
            }
        }

        switch (cmd)
        {
        case MIOSYS_CONTROL:
        {
            ret = miosys_ioctl_control(cmd, arg);
        } break;

      //case MIOSYS_MEDIA_READ:
        case MIOSYS_NAND_READ:
        case MIOSYS_NANDRAW_READ:
        {
            ret = miosys_ioctl_read(cmd, arg);
        } break;

      //case MIOSYS_MEDIA_WRITE:
        case MIOSYS_NAND_WRITE:
        case MIOSYS_NANDRAW_WRITE:
        {
            ret = miosys_ioctl_write(cmd, arg);
        } break;

        case MIOSYS_NAND_ERASE:
        case MIOSYS_NANDRAW_ERASE:
        {
            ret = miosys_ioctl_erase(cmd, arg);
        } break;

        default: { ret = -EINVAL; } break;
        }
    }
    mutex_unlock(miosys_dev.ioctl_mutex);

    return ret;
}

static long miosys_ioctl_get_user_argument(unsigned int cmd, unsigned long arg, void * arg_buf, unsigned int arg_buf_size)
{
    long ret = 0;
    void __user *argp = (void __user *)arg;
    int arg_size = 0;
  //struct miosys_media_io *arg_media = (struct miosys_media_io *)arg_buf;
    struct miosys_nand_io  *arg_nand  = (struct miosys_nand_io *)arg_buf;

    if ((arg_buf_size < sizeof(struct miosys_media_io)) ||
        (arg_buf_size < sizeof(struct miosys_nand_io)))
    {
        return -EFAULT;
    }

    arg_size = _IOC_SIZE(cmd);
    if (!arg_size)
    {
        return -EFAULT;
    }

    if (copy_from_user(arg_buf, argp, arg_size))
    {
        return -EFAULT;
    }

    // check arguments
    switch (cmd)
    {
  //case MIOSYS_MEDIA_READ:
  //case MIOSYS_MEDIA_WRITE:
  //{
  //    if (!arg_media->buf)    { return -EINVAL; }
  //    if (!arg_media->blkcnt) { return -EINVAL; }
  //    if (arg_media->blknr + arg_media->blkcnt > *Exchange.ftl.Capacity) { return -ENXIO; }
  //} break;

    case MIOSYS_NAND_READ:
    case MIOSYS_NAND_WRITE:
    case MIOSYS_NANDRAW_READ:
    case MIOSYS_NANDRAW_WRITE:
    {
        if (!arg_nand->buf) { return -EINVAL; }
        if (!arg_nand->len) { return -EINVAL; }
        if (((arg_nand->ofs >> 9) + (arg_nand->len >> 9)) > *Exchange.ftl.Capacity) { return -ENXIO; }
    } break;

    case MIOSYS_NAND_ERASE:
    case MIOSYS_NANDRAW_ERASE:
    {
        if (!arg_nand->len) { return -EINVAL; }
        if (((arg_nand->ofs >> 9) + (arg_nand->len >> 9)) > *Exchange.ftl.Capacity) { return -ENXIO; }
    } break;

    default: { return -ENOEXEC; } break;
    }

    return ret;
}

static long miosys_ioctl_read(unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    size_t len = 0;
    unsigned char * kbuf = 0;
    unsigned char *buf = 0;
    unsigned int arg_buf_size = 40;
    char __user arg_buf[40] = {0,};
    unsigned int buf_size = 0;
    unsigned char enable_ecc = 0;
  //struct miosys_media_io *arg_media = (struct miosys_media_io *)arg_buf;
    struct miosys_nand_io  *arg_nand  = (struct miosys_nand_io *)arg_buf;

    ret = miosys_ioctl_get_user_argument(cmd, arg, (void *)arg_buf, arg_buf_size);
    if (ret < 0)
    {
        __PRINT("miosys_ioctl_read: miosys_ioctl_get_user_argument() return %ld!\n", ret);
        return ret;
    }

    switch (cmd)
    {
  //case MIOSYS_MEDIA_READ:   { buf = (unsigned char *)arg_media->buf; buf_size = arg_media->blkcnt << 9; } break;
    case MIOSYS_NAND_READ:    { buf = (unsigned char *)arg_nand->buf;  buf_size = arg_nand->len;          } break;
    case MIOSYS_NANDRAW_READ: { buf = (unsigned char *)arg_nand->buf;  buf_size = arg_nand->len;          } break;
    }

    kbuf = (unsigned char *)vmalloc(buf_size);
    if (!kbuf)  { return -ENOMEM; }

    switch (cmd)
    {
  //case MIOSYS_MEDIA_READ:
  //{
  //    int sectors = 0;
  //
  //    sectors = Exchange.ftl.fnRead(arg_media->blknr, arg_media->blkcnt, (void *)kbuf);
  //    len = sectors << 9;
  //    ret = len;
  //} break;

    case MIOSYS_NAND_READ:
    case MIOSYS_NANDRAW_READ:
    {
        if (!NFC_PHY_LOWAPI_is_init())
        {
            ret = -ENODEV;
            __PRINT("miosys_ioctl_read: error! NFC_PHY_LOWAPI is not initialized!\n");
            break;
        }

        len = arg_nand->len;
        enable_ecc = (cmd == MIOSYS_NAND_READ)? 1: 0;

        if (NFC_PHY_LOWAPI_nand_read(arg_nand->ofs, &len, (u_char *)kbuf, enable_ecc) < 0)
        {
            ret = -EIO;
        }

    } break;

    default: { len = 0; } break;

    }

    if (copy_to_user(buf, kbuf, len))
    {
        vfree(kbuf);
        return -EFAULT;
    }

#if 0 // print read data 
if (len <= 32 * 1024)
{
    int data_idx=0;
    unsigned int *data = (unsigned int *)buf;

    DBG_MIOSYS("buf(%08x) len:%d", (unsigned int)buf, len);
    for (data_idx=0; data_idx < (len/sizeof(unsigned int)); data_idx++)
    {
        if (!(data_idx % 16))
        {
            DBG_MIOSYS("\n");
            DBG_MIOSYS("[%04x] ", data_idx);
        }

        DBG_MIOSYS("%08x ", data[data_idx]);
    }
    DBG_MIOSYS("\n");
}
#endif

    if (kbuf)
    {
        vfree(kbuf);
    }

    return ret;
}

static long miosys_ioctl_write(unsigned int cmd, unsigned long arg)
{
    size_t len = 0;
    long ret = 0;
    unsigned char * kbuf = 0;
    unsigned char *buf = 0;
    unsigned int arg_buf_size = 40;
    char __user arg_buf[40] = {0,};
    unsigned int buf_size = 0;
    unsigned char enable_ecc = 0;
  //struct miosys_media_io *arg_media = (struct miosys_media_io *)arg_buf;
    struct miosys_nand_io  *arg_nand  = (struct miosys_nand_io *)arg_buf;

    ret = miosys_ioctl_get_user_argument(cmd, arg, (void *)arg_buf, arg_buf_size);
    if (ret < 0)
    {
        return ret;
    }

    switch (cmd)
    {
  //case MIOSYS_MEDIA_WRITE:   { buf = (unsigned char *)arg_media->buf; buf_size = arg_media->blkcnt << 9; } break;
    case MIOSYS_NAND_WRITE:    { buf = (unsigned char *)arg_nand->buf;  buf_size = arg_nand->len;          } break;
    case MIOSYS_NANDRAW_WRITE: { buf = (unsigned char *)arg_nand->buf;  buf_size = arg_nand->len;          } break;
    }

    kbuf = (unsigned char *)vmalloc(buf_size);
    if (!kbuf)  { return -ENOMEM; }

    if (copy_from_user(kbuf, buf, buf_size))
    {
        vfree(kbuf);
        return -EFAULT;
    }

    switch (cmd)
    {
  //case MIOSYS_MEDIA_WRITE:
  //{
  //    int sectors = 0;
  //
  //    sectors = Exchange.ftl.fnWrite(arg_media->blknr, arg_media->blkcnt, (const void *)kbuf);
  //    len = sectors << 9;
  //    ret = len;
  //} break;

    case MIOSYS_NAND_WRITE:
    case MIOSYS_NANDRAW_WRITE:
    {
        if (!NFC_PHY_LOWAPI_is_init())
        {
            ret = -ENODEV;
            __PRINT("miosys_ioctl_write: error! NFC_PHY_LOWAPI is not initialized!\n");
            break;
        }

        len = arg_nand->len;
        enable_ecc = (cmd == MIOSYS_NAND_WRITE)? 1: 0;

        if (NFC_PHY_LOWAPI_nand_write(arg_nand->ofs, &len, (u_char *)kbuf, enable_ecc) < 0)
        {
            ret = -EIO;
        }

    } break;

    default: { len = 0; } break;
    }

#if 0 // print write data 
if (len <= 32 * 1024)
{
    int data_idx=0;
    unsigned int *data = (unsigned int *)kbuf;

    DBG_MIOSYS("buf(%08x) len:%d", (unsigned int)kbuf, buf_size);
    for (data_idx=0; data_idx < (buf_size/sizeof(unsigned int)); data_idx++)
    {
        if (!(data_idx % 16))
        {
            DBG_MIOSYS("\n");
            DBG_MIOSYS("[%04x] ", data_idx);
        }

        DBG_MIOSYS("%08x ", data[data_idx]);
    }
    DBG_MIOSYS("\n");
}
#endif

    if (kbuf)
    {
        vfree(kbuf);
    }

    return ret;
}

static long miosys_ioctl_erase(unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    unsigned int arg_buf_size = 40;
    char __user arg_buf[40] = {0,};
    struct miosys_nand_io     *arg_nand    = (struct miosys_nand_io *)arg_buf;

    ret = miosys_ioctl_get_user_argument(cmd, arg, (void *)arg_buf, arg_buf_size);
    if (ret < 0)
    {
        return ret;
    }

    switch (cmd)
    {
    case MIOSYS_NAND_ERASE:
    case MIOSYS_NANDRAW_ERASE:
    {
        if (!NFC_PHY_LOWAPI_is_init())
        {
            ret = -ENODEV;
            __PRINT("miosys_ioctl_erase(): error! NFC_PHY_LOWAPI is not initialized!\n");
            break;
        }

        if (NFC_PHY_LOWAPI_nand_erase(arg_nand->ofs, arg_nand->len) < 0)
        {
            ret = -EIO;
        }
    } break;

    default: {} break;
    }

    return ret;
}

static long miosys_ioctl_control(unsigned int cmd, unsigned long arg)
{
    long ret = 0;

    // add here
    // ..

    return ret;
}
