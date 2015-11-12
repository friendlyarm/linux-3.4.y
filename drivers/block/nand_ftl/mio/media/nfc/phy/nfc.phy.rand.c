/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.rand.c
 * Date         : 2014.09.02
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.09.02, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#define __NFC_PHY_RAND_GLOBAL__
#include "nfc.phy.rand.h"

/******************************************************************************
 * to use: printf(), malloc(), memset(), free()
 ******************************************************************************/
#include "../../exchange.h"
#include "../../../mio.definition.h"
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

#include <linux/random.h>
#include <linux/jiffies.h>

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <common.h>
#include <malloc.h>
#else
#error "nfc.phy.rand.c: error! not defined build mode!"
#endif

/******************************************************************************
 * local
 ******************************************************************************/
static struct
{
    unsigned char enable;
    int curr_page;

    unsigned char *buf;
    int buf_size;

} randomizer;

#ifdef __MIO_UNIT_TEST_RANDOMIZER__

#define RAND_TEST_BUFSIZE	8192

static unsigned long long rand_elapse_sum = 0;

static struct
{
	ktime_t s;
	ktime_t e;
	ktime_t ns;
} rand_elapse_t;

char rand_test_buffer[RAND_TEST_BUFSIZE];
#endif


#define NFC_PHY_RAND_PAGESEED_CNT   (128)
static unsigned short page_seed[NFC_PHY_RAND_PAGESEED_CNT] =
{
    0x576A, 0x05E8, 0x629D, 0x45A3, 0x649C, 0x4BF0, 0x2342, 0x272E,
    0x7358, 0x4FF3, 0x73EC, 0x5F70, 0x7A60, 0x1AD8, 0x3472, 0x3612,
    0x224F, 0x0454, 0x030E, 0x70A5, 0x7809, 0x2521, 0x48F4, 0x5A2D,
    0x492A, 0x043D, 0x7F61, 0x3969, 0x517A, 0x3B42, 0x769D, 0x0647,
    0x7E2A, 0x1383, 0x49D9, 0x07B8, 0x2578, 0x4EEC, 0x4423, 0x352F,
    0x5B22, 0x72B9, 0x367B, 0x24B6, 0x7E8E, 0x2318, 0x6BD0, 0x5519,
    0x1783, 0x18A7, 0x7B6E, 0x7602, 0x4B7F, 0x3648, 0x2C53, 0x6B99,
    0x0C23, 0x67CF, 0x7E0E, 0x4D8C, 0x5079, 0x209D, 0x244A, 0x747B, // 64
    0x350B, 0x0E4D, 0x7004, 0x6AC3, 0x7F3E, 0x21F5, 0x7A15, 0x2379,
    0x1517, 0x1ABA, 0x4E77, 0x15A1, 0x04FA, 0x2D61, 0x253A, 0x1302,
    0x1F63, 0x5AB3, 0x049A, 0x5AE8, 0x1CD7, 0x4A00, 0x30C8, 0x3247,
    0x729C, 0x5034, 0x2B0E, 0x57F2, 0x00E4, 0x575B, 0x6192, 0x38F8,
    0x2F6A, 0x0C14, 0x45FC, 0x41DF, 0x38DA, 0x7AE1, 0x7322, 0x62DF,
    0x5E39, 0x0E64, 0x6D85, 0x5951, 0x5937, 0x6281, 0x33A1, 0x6A32,
    0x3A5A, 0x2BAC, 0x743A, 0x5E74, 0x3B2E, 0x7EC7, 0x4FD2, 0x5D28,
    0x751F, 0x3EF8, 0x39B1, 0x4E49, 0x746B, 0x6EF6, 0x44BE, 0x6DB7, // 128
};

/******************************************************************************
 * local functions
 ******************************************************************************/
static int randomizer_page(int page, unsigned char *buffer, int size, unsigned char auto_disable)
{
    unsigned short seed = page_seed[page & (NFC_PHY_RAND_PAGESEED_CNT-1)];
    unsigned char ch;
    int i = 0;
    int ff_cnt = 0;

    for (; size > i; i++)
    {
        unsigned char x0 = (seed & (1<<14)) ? 0xff : 0;
        unsigned char x1 = (seed & (1<<13)) ? 0xff : 0;
        unsigned char ret = x0 ^ x1;
        seed = (seed<<1) | (ret & 0x1);
        ch = buffer[i] ^ ret;

        if (buffer[i] == 0xFF)
        {
            ff_cnt += 1;
        }

        buffer[i] = ch;
    }

    if (auto_disable)
    {
        if (ff_cnt > ((size * 95) / 100))
        {
            if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)buffer, 0xFF, size); }
            else                         {                  memset((void *)buffer, 0xFF, size); }
        }
    }

    return 0;
}

/******************************************************************************
 * extern functions
 ******************************************************************************/
int NFC_PHY_RAND_Init(int _buf_size)
{
#ifdef __MIO_UNIT_TEST_RANDOMIZER__
	srandom32((uint)jiffies);
#endif
    Exchange.nfc.fnRandomize_Enable = NFC_PHY_RAND_Enable;

    NFC_PHY_RAND_DeInit();

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    randomizer.buf = (unsigned char *)vmalloc(_buf_size);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    randomizer.buf = (unsigned char *)malloc(_buf_size);
#endif

    if (randomizer.buf)
    {
        randomizer.buf_size = _buf_size;
        return 0;
    }

    return -1;
}

void NFC_PHY_RAND_DeInit(void)
{
    if (randomizer.buf)
    {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(randomizer.buf);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(randomizer.buf);
#endif

        randomizer.buf = 0;
        randomizer.buf_size = 0;
    }
}

void NFC_PHY_RAND_Enable(unsigned char _enable)
{
    if (Exchange.debug.nfc.phy.info_randomizer) { Exchange.sys.fn.print("NFC_PHY_RAND_Enable: %d \n", _enable); }
    randomizer.enable = (_enable)? 1: 0;
}


#ifdef __MIO_UNIT_TEST_RANDOMIZER__
static void unit_test_rand_start(void)
{
	rand_elapse_t.s = ktime_get();
}

static void unit_test_rand_end(void)
{
	rand_elapse_t.e = ktime_get();
	rand_elapse_t.ns.tv64 = ktime_to_ns(ktime_sub(rand_elapse_t.e, rand_elapse_t.s));

	rand_elapse_sum += rand_elapse_t.ns.tv64;
}


static void * NFC_PHY_RAND_RUN(void *_buf, int _buf_size, unsigned char keep)
{
    unsigned char *rand_buf = (unsigned char *)_buf;
    int size = _buf_size;

	unit_test_rand_start();

    if (keep > 1)
    {
        if (!rand_buf || (size > randomizer.buf_size))
        {
            if (Exchange.debug.nfc.phy.info_randomizer) { Exchange.sys.fn.print("NFC_PHY_RAND_Randomize: error: rand_buf:0x%08x, size:%d\n", (unsigned int)rand_buf, size); }
        }
        else
        {
			randomizer_page(randomizer.curr_page, rand_buf, size, 1);
		}
    }

	unit_test_rand_end();

    return (void *)rand_buf;
}
void NFC_PHY_RAND_Test(int cnt)
{
	u32 page;
    char * data = rand_test_buffer;
	int i;
	
	Exchange.sys.fn.print(" [UNIT TEST] Run Rand Test ............. (count: %d)\n", cnt);

	for (i = 0; i < cnt; i++)
	{
		page = random32();

		NFC_PHY_RAND_SetPageSeed(page & 0xff);
		memset(data, (page & 1) ? 0xAA : 0x55, RAND_TEST_BUFSIZE);

		data = (char *)NFC_PHY_RAND_RUN((void *)data, 1024, 2);
	}
}

void NFC_PHY_RAND_Print(void)
{
	Exchange.sys.fn.print(" [UNIT TEST] randomizer total: %15llu (ns)\n", rand_elapse_sum);
}

void NFC_PHY_RAND_Clear(void)
{
	rand_elapse_sum = 0;
}
#else
#define unit_test_rand_start()		do {} while (0)
#define unit_test_rand_end()		do {} while (0)

void NFC_PHY_RAND_Test(int cnt)		{ }
void NFC_PHY_RAND_Print(void)		{ }
void NFC_PHY_RAND_Clear(void)		{ }
#endif /* __MIO_UNIT_TEST_RANDOMIZER__ */


int NFC_PHY_RAND_IsEnable(void)
{
    return randomizer.enable;
}

void NFC_PHY_RAND_SetPageSeed(int _page)
{
    randomizer.curr_page = _page;
}

void * NFC_PHY_RAND_Randomize(void *_buf, int _buf_size, unsigned char keep)
{
    unsigned char *rand_buf = (unsigned char *)_buf;
    int size = _buf_size;
    int auto_disable = 0;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_NFC_RANDOMIZER_RW, ELAPSE_T_IO_NFC_RANDOMIZER_R, ELAPSE_T_IO_NFC_RANDOMIZER_W); }
#endif

    if (randomizer.enable)
    {
        if (!rand_buf || (size > randomizer.buf_size))
        {
            if (Exchange.debug.nfc.phy.info_randomizer) { Exchange.sys.fn.print("NFC_PHY_RAND_Randomize: error: rand_buf:0x%08x, size:%d\n", (uintptr_t)rand_buf, size); }
        }
        else
        {
            if (keep)
            {
                auto_disable = 0;
                rand_buf = randomizer.buf;

                if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)rand_buf, (const void *)_buf, size); }
                else                         {                  memcpy((void *)rand_buf, (const void *)_buf, size); }
            }
            else
            {
                auto_disable = 1;
            }

            if (Exchange.debug.nfc.phy.info_randomizer) { Exchange.sys.fn.print("NFC_PHY_RAND_Randomize: Pg:%d, size:%5d, Src : %08x, %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", randomizer.curr_page, size, (uintptr_t)rand_buf, rand_buf[0], rand_buf[1], rand_buf[2], rand_buf[3], rand_buf[4], rand_buf[5], rand_buf[6], rand_buf[7], rand_buf[8], rand_buf[9], rand_buf[10], rand_buf[11], rand_buf[12], rand_buf[13], rand_buf[14], rand_buf[15]); }
            randomizer_page(randomizer.curr_page, rand_buf, size, auto_disable);
            if (Exchange.debug.nfc.phy.info_randomizer) { Exchange.sys.fn.print("NFC_PHY_RAND_Randomize: Pg:%d, size:%5d, Dest: %08x, %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", randomizer.curr_page, (uintptr_t)rand_buf, rand_buf[0], rand_buf[1], rand_buf[2], rand_buf[3], rand_buf[4], rand_buf[5], rand_buf[6], rand_buf[7], rand_buf[8], rand_buf[9], rand_buf[10], rand_buf[11], rand_buf[12], rand_buf[13], rand_buf[14], rand_buf[15]); }
        }
    }

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_NFC_RANDOMIZER_RW, ELAPSE_T_IO_NFC_RANDOMIZER_R, ELAPSE_T_IO_NFC_RANDOMIZER_W); }
#endif

    return (void *)rand_buf;
}
