/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : exchange.c
 * Date         : 2014.07.11
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.11 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#define __EXCHANGE_GLOBAL__
#include "exchange.h"

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
#if defined (__COMPILE_MODE_X64__)
    #include <nexell/nxp-ftl-nand.h>
#endif
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <div64.h>
#include <linux/math64.h>
#include <common.h>
#include <malloc.h>
#endif

/******************************************************************************
 * Import Extern
 ******************************************************************************/
extern void FTL_Configuration(void * _Config);
extern int FTL_Format(unsigned char * _chip_name, unsigned long _chip_id_base, unsigned char _option);
extern int FTL_Open(unsigned char * _chip_name, unsigned long _chip_id_base, unsigned int _format_open);
extern int FTL_Close(void);
extern int FTL_Boot(unsigned char _mode);

extern unsigned int NFC_PHY_Init(unsigned int _scan_format);
extern void NFC_PHY_DeInit(void);

extern int NFC_PHY_READRETRY_Init(unsigned int _max_channels, unsigned int _max_ways, const unsigned char *_way_map, unsigned char _readretry_type);
extern void NFC_PHY_READRETRY_DeInit(void);

extern int NFC_PHY_RAND_Init(int _buf_size);
extern void NFC_PHY_RAND_DeInit(void);

/******************************************************************************
 * Declaration
 ******************************************************************************/
unsigned short EXCHANGE_GetCRC16(unsigned short _initial, void * _buffer, unsigned int _length);
unsigned int EXCHANGE_GetCRC32(unsigned int _initial, void * _buffer, unsigned int _length);

/******************************************************************************
 * Definition
 ******************************************************************************/
void * __memset(void * _p, int _v, unsigned int _n)
{
    void * r = 0;
    
#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_MEMIO_RW, ELAPSE_T_IO_MEMIO_R, ELAPSE_T_IO_MEMIO_W); }
#endif

    r = memset(_p, _v, _n);

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_MEMIO_RW, ELAPSE_T_IO_MEMIO_R, ELAPSE_T_IO_MEMIO_W); }
#endif

    return r;
}

void * __memcpy(void * _d, const void * _s, unsigned int _n)
{
    void * r = 0;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_MEMIO_RW, ELAPSE_T_IO_MEMIO_R, ELAPSE_T_IO_MEMIO_W); }
#endif

    r = memcpy(_d, _s, _n);

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_MEMIO_RW, ELAPSE_T_IO_MEMIO_R, ELAPSE_T_IO_MEMIO_W); }
#endif

    return r;
}

int __memcmp(const void * _p1, const void * _p2, unsigned int _n)
{
    int r = 0;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_MEMIO_RW, ELAPSE_T_IO_MEMIO_R, ELAPSE_T_IO_MEMIO_W); }
#endif

    r = memcmp(_p1, _p2, _n);

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_MEMIO_RW, ELAPSE_T_IO_MEMIO_R, ELAPSE_T_IO_MEMIO_W); }
#endif

    return r;
}

unsigned long long __div64(unsigned long long _dividend, unsigned long long _divisor)
{
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    return div64_u64(_dividend, _divisor);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    return div_u64(_dividend, _divisor);
#endif
}

void __ratio(unsigned char * _sz, unsigned long long _v1, unsigned long long _v2)
{
    unsigned long long v1 = 0;
    unsigned long long v2 = 0;

    unsigned int dividend = 0;
    unsigned int divisor = 0;
    unsigned int quotient = 0;
    unsigned int remainder = 0;
    unsigned int remainder_tmp = 0;
    unsigned int looper = 0;

    /**************************************************************************
     * divisor Must Be Great than dividend
     **************************************************************************/
    if (_v1 > _v2)
    {
        v2 = _v2; // dividend
        v1 = _v1; // divisor
    }
    else
    {
        v2 = _v1; // dividend
        v1 = _v2; // divisor
    }

    if (v1)
    {
        /**********************************************************************
         * Type Cast divisor From u64 To u32
         **********************************************************************/
        if (v1 > 1000000000000000000ull)
        {
            dividend = (unsigned int)__div64(v2, 1000000000000ull*10);
            divisor = (unsigned int)__div64(v1, 1000000000000ull*10);
        }
        else if (v1 > 1000000000000000ull)
        {
            dividend = (unsigned int)__div64(v2, 1000000000ull*10);
            divisor = (unsigned int)__div64(v1, 1000000000ull*10);
        }
        else if (v1 > 1000000000000ull)
        {
            dividend = (unsigned int)__div64(v2, 1000000*10);
            divisor = (unsigned int)__div64(v1, 1000000*10);
        }
        else if (v1 > 1000000000ull)
        {
            dividend = (unsigned int)__div64(v2, 1000*10);
            divisor = (unsigned int)__div64(v1, 1000*10);
        }
        else
        {
            dividend = (unsigned int)v2;
            divisor = (unsigned int)v1;
        }

        /**********************************************************************
         * Make Number
         **********************************************************************/
        quotient = dividend / divisor;
        remainder_tmp = dividend % divisor;
        remainder = 0;

        for (looper = 0; looper < 5; looper++)
        {
            if (remainder_tmp)
            {
                remainder_tmp *= 10;
                remainder *= 10;
                remainder += remainder_tmp / divisor;
                remainder_tmp = remainder_tmp % divisor;
            }
            else
            {
                break;
            }
        }

        /**********************************************************************
         * Make Ratio Number
         **********************************************************************/
        quotient = quotient * 100 + remainder / 1000;
        remainder_tmp = remainder % 1000;
        remainder = 0;

        for (looper = 0; looper < 3; looper++)
        {
            if (remainder_tmp)
            {
                remainder_tmp *= 10;
                remainder *= 10;
                remainder += remainder_tmp / 1000;
                remainder_tmp = remainder_tmp % 1000;
            }
            else
            {
                break;
            }
        }
    }

    /**************************************************************************
     * Make Ratio String
     **************************************************************************/
    if (!remainder) { sprintf((char*)_sz, "%3d.000 %%", quotient); }
    else            { sprintf((char*)_sz, "%3d.%03d %%", quotient, remainder); }
}

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#if !defined (__COMPILE_MODE_X64__)
extern unsigned long nxp_ftl_start_block; /* byte address, Must Be Multiple of 8MB */
#endif
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
extern unsigned long nxp_ftl_start_block; /* byte address, Must Be Multiple of 8MB */
#endif
#define _BLOCK_ALIGN_			(8<<20)

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
static void __usleep_range(unsigned long min, unsigned long max)
{
	#ifdef __MIO_UNIT_TEST_SLEEP__
	ktime_t t1;
	s64 ns = 0;

	t1 = ktime_get();
	#endif

	#ifdef __USING_DELAY_FOR_SHORT_SLEEP__
	if (min < NSEC_PER_USEC)
		udelay(min);
	else
	#endif
		usleep_range(min, max);

	#ifdef __MIO_UNIT_TEST_SLEEP__
	ns = ktime_to_ns(ktime_sub(ktime_get(), t1));
	ns = div64_u64(ns, 1000L);

	if (ns >= min * 20)
		Exchange.sys.fn.print("%s: sleeping too long!!! (req: %lu, elapse: %lld)\n", __func__, min, ns);
	#endif
}

void __msleep(unsigned int msecs)
{
	#ifdef __MIO_UNIT_TEST_SLEEP__
	ktime_t t1;
	s64 ns = 0;

	t1 = ktime_get();
	#endif

	msleep(msecs);

	#ifdef __MIO_UNIT_TEST_SLEEP__
	ns = ktime_to_ns(ktime_sub(ktime_get(), t1));
	ns = div64_u64(ns, 1000L * 1000L);

	if (ns >= msecs * 2)
		Exchange.sys.fn.print("%s: sleeping too long!!! (req: %u, elapse: %lld)\n", __func__, msecs, ns);
	#endif
}
#endif

void EXCHANGE_init(void)
{
	unsigned long ftl_start_block = 0;
    /**************************************************************************
     * FTL Start Offset Must Be Multiple Of 8MB
     **************************************************************************/
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#if defined (__COMPILE_MODE_X64__)
    ftl_start_block = nxp_nand.nxp_ftl_start_block;
#else    
    ftl_start_block = nxp_ftl_start_block;
#endif
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    ftl_start_block = nxp_ftl_start_block;
#endif

	if (ftl_start_block & (_BLOCK_ALIGN_-1))
    {
		ftl_start_block = ALIGN(ftl_start_block, _BLOCK_ALIGN_);
	}

    Exchange.ewsftl_start_offset  = ftl_start_block;
    Exchange.ewsftl_start_page    = 0;
    Exchange.ewsftl_start_block   = 0;

    /**************************************************************************
     * Asign Inital Exchages
     **************************************************************************/
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)

    Exchange.ftl.fnConfig = FTL_Configuration;
#if defined (__COMPILE_MODE_FORMAT__)
    Exchange.ftl.fnFormat = FTL_Format;
#endif
    Exchange.ftl.fnOpen = FTL_Open;
    Exchange.ftl.fnClose = FTL_Close;
    Exchange.ftl.fnBoot = FTL_Boot;

    Exchange.nfc.fnInit = NFC_PHY_Init;
    Exchange.nfc.fnDeInit = NFC_PHY_DeInit;

    Exchange.nfc.fnReadRetry_Init = NFC_PHY_READRETRY_Init;
    Exchange.nfc.fnReadRetry_DeInit = NFC_PHY_READRETRY_DeInit;

    Exchange.nfc.fnRandomize_Init = NFC_PHY_RAND_Init;
    Exchange.nfc.fnRandomize_DeInit = NFC_PHY_RAND_DeInit;

    Exchange.sys.fn.usleep = __usleep_range;
    Exchange.sys.fn.msleep = __msleep;
    Exchange.sys.fn.print = printk;
    Exchange.sys.fn.sprint = sprintf;
    Exchange.sys.fn.strlen = strlen;
    Exchange.sys.fn._memset = __memset;
    Exchange.sys.fn._memcpy = __memcpy;
    Exchange.sys.fn._memcmp = __memcmp;
    Exchange.sys.fn.div64 = __div64;
    Exchange.sys.fn.ratio = __ratio;
    Exchange.sys.fn.get_crc16 = EXCHANGE_GetCRC16;
    Exchange.sys.fn.get_crc32 = EXCHANGE_GetCRC32;

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)

    Exchange.ftl.fnConfig = FTL_Configuration;
    Exchange.ftl.fnFormat = FTL_Format;
    Exchange.ftl.fnOpen = FTL_Open;
    Exchange.ftl.fnClose = FTL_Close;
    Exchange.ftl.fnBoot = FTL_Boot;

    Exchange.nfc.fnInit = NFC_PHY_Init;
    Exchange.nfc.fnDeInit = NFC_PHY_DeInit;
    
    Exchange.nfc.fnReadRetry_Init = NFC_PHY_READRETRY_Init;
    Exchange.nfc.fnReadRetry_DeInit = NFC_PHY_READRETRY_DeInit;

    Exchange.nfc.fnRandomize_Init = NFC_PHY_RAND_Init;
    Exchange.nfc.fnRandomize_DeInit = NFC_PHY_RAND_DeInit;

    Exchange.sys.fn.print = printf;
    Exchange.sys.fn.sprint = sprintf;
    Exchange.sys.fn.strlen = strlen;
    Exchange.sys.fn._memset = __memset;
    Exchange.sys.fn._memcpy = __memcpy;
    Exchange.sys.fn._memcmp = __memcmp;
    Exchange.sys.fn.div64 = __div64;
    Exchange.sys.fn.ratio = __ratio;
    Exchange.sys.fn.get_crc16 = EXCHANGE_GetCRC16;
    Exchange.sys.fn.get_crc32 = EXCHANGE_GetCRC32;

#endif

    Exchange.sys.fn.print("EWS.FTL Start Block is 0x%x\n", ftl_start_block);
}

/******************************************************************************
 * CRC16
 ******************************************************************************/
static const unsigned short CrcTable16[] =
{
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

unsigned short EXCHANGE_GetCRC16(unsigned short _initial, void * _buffer, unsigned int _length)
{
    unsigned short crc = _initial;
    unsigned char * p = (unsigned char *)_buffer;
    unsigned int length = _length;
    unsigned int idx = 0;

    while (length--)
    {
        idx = (((unsigned short)(crc>>8)) ^ *p++) & 0xFF;
        crc = ((crc<<8) ^ CrcTable16[idx]) & 0xFFFF;
    }

    return crc;
}

/******************************************************************************
 * CRC32
 ******************************************************************************/
static const unsigned int CrcTable32[] =
{
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

unsigned int EXCHANGE_GetCRC32(unsigned int _initial, void * _buffer, unsigned int _length)
{
    unsigned int crc = _initial;
    unsigned char * p = (unsigned char *)_buffer;
    unsigned int length = _length;
    unsigned int idx = 0;

    while (length--)
    {
        idx = ((crc>>24) ^ *p++) & 0xff;
        crc = (crc<<8) ^ CrcTable32[idx];
    }

    return crc;
}
