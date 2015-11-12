/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.c
 * Date         : 2014.07.07
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.07, SD.LEE)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#define __NFC_PHY_REGISTER_GLOBAL__
#define __NFC_PHY_GLOBAL__
#include "nfc.phy.h"
#include "nfc.phy.scan.h"
#include "nfc.phy.register.h"
#include "nfc.phy.rand.h"

/******************************************************************************
 *
 ******************************************************************************/
#include "../../exchange.h"
#include "../../../mio.definition.h"

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#include <linux/vmalloc.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/sizes.h>
//#include <asm/mach-types.h>
#include <linux/bitops.h>
#include <linux/gpio.h>

#include <linux/sched.h>
#include <asm/stacktrace.h>
#include <asm/traps.h>
//#include <asm/unwind.h>

#if defined (__COMPILE_MODE_X64__)
    /* nexell soc headers */
    #include <nexell/platform.h>
    #include <nexell/soc-s5pxx18.h>
    #include <nexell/nxp-ftl-nand.h>
#else
    #include <mach/platform.h>
    #include <mach/devices.h>
    #include <mach/soc.h>
#endif

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <common.h>
#include <malloc.h>

#include <asm/io.h>
#include <platform.h>
#include <mach-api.h>
#include <asm/gpio.h>

#endif


/******************************************************************************
 *
 * NFC IO Width
 *
 ******************************************************************************/
#define IO_BURST_X4		(0x4)
#define IO_BURST_X1		(0x2)
#define IO_BURST_X0		(0x1)

#if defined(CONFIG_ARM64)			/* FIXME: u-boot cannot recognize__COMPILE_MODE_X64__ */
#define IOR_WIDTH		IO_BURST_X4
#define IOW_WIDTH		IO_BURST_X0
#else
#define IOR_WIDTH		IO_BURST_X4
#define IOW_WIDTH		IO_BURST_X0
#endif



/******************************************************************************
 * IO Read/Write Burst
 ******************************************************************************/
#ifdef CONFIG_ARM64
void __nfc_read_burst(const void __iomem *addr, void *data, int len);
void __nfc_write_burst(const void __iomem *addr, void *data, int len);

static void ior_burst(char *data, void __iomem *nfdata, int len)
{
	__nfc_read_burst(nfdata, data, len);
}

static void iow_burst(char *data, void __iomem *nfdata, int len)
{
	__nfc_write_burst(nfdata, data, len);
}
#else
void __nfc_readsb(const void __iomem *addr, void *data, int bytelen);
void __nfc_readsl(const void __iomem *addr, void *data, int longlen);
void __nfc_writesb(void __iomem *addr, const void *data, int bytelen);
void __nfc_writesl(void __iomem *addr, const void *data, int longlen);

static void ior_burst(char *data, void __iomem *nfdata, int len)
{
	int burst_len = len & ~(4-1);

	//printk("nfcShadowI: %p, burst_len: %d, len: %d\n", nfcShadowI, burst_len, len);
	__nfc_readsl(nfdata, data, burst_len/4);
	__nfc_readsb(nfdata, data + burst_len, len-burst_len);
}

static void iow_burst(char *data, void __iomem *nfdata, int len)
{
	int burst_len = len & ~(4-1);

	//printk("nfcShadowI: %p, burst_len: %d, len: %d\n", nfcShadowI, burst_len, len);
	__nfc_writesl(nfdata, data, burst_len/4);
	__nfc_writesb(nfdata, data + burst_len, len-burst_len);
}
#endif

static char *nfc_io_read(char *data, void __iomem *nfdata, int len)
{
	unsigned int read_loop = read_loop;

#if (IOR_WIDTH == IO_BURST_X4)
	ior_burst(data, (void __iomem *)&nfcShadowI->nfdata, len);
	data += len;
#elif (IOR_WIDTH == IO_BURST_X1)
	for (read_loop = 0; read_loop < len/4; read_loop++)
	{
		*((unsigned int *)data) = nfcShadowI32->nfdata; data+=4;
	}
	for (read_loop = 0; read_loop < len%4; read_loop++)
	{
		*data++ = nfcShadowI->nfdata;
	}
#else
	for (read_loop = 0; read_loop < len; read_loop++)
	{
		*data++ = nfcShadowI->nfdata;
	}
#endif

	return data;
}

static char *nfc_io_write(char *data, void __iomem *nfdata, int len)
{
	unsigned int write_loop = write_loop;

#if (IOW_WIDTH == IO_BURST_X4)
	iow_burst(data, (void __iomem *)&nfcShadowI->nfdata, len);
	data += len;
#elif (IOW_WIDTH == IO_BURST_X1)
	for (write_loop = 0; write_loop < len/4; write_loop++)
	{
		nfcShadowI32->nfdata = *((unsigned int *)data); data+=4;
	}
	for (write_loop = 0; write_loop < len%4; write_loop++)
	{
		nfcShadowI->nfdata = *data; data += 1;
	}
#else
	for (write_loop = 0; write_loop < len; write_loop++)
	{
		nfcShadowI->nfdata = *data; data += 1;
	}
#endif

	return data;
}



#ifdef DEBUG_TRIGGER_GPIO

#define CFG_IO_NFC_DEBUG		(PAD_GPIO_C + 3)	/* GPIO */

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
extern int /* -1 = invalid gpio, 0 = gpio's input mode, 1 = gpio's output mode. */ nxp_soc_gpio_get_io_dir(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/);
extern void nxp_soc_gpio_set_io_dir(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/, int /* '1' is output mode, '0' is input mode */);
extern void nxp_soc_gpio_set_out_value(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/, int /* '1' is high level, '0' is low level */);
extern int /* -1 = invalid gpio, 0 = gpio's input value is low level, alive input is low, 1 = gpio's input value is high level, alive input is high */ nxp_soc_gpio_get_in_value(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/);

#define NFC_DEBUG_PIN_ON()		do { nxp_soc_gpio_set_out_value(CFG_IO_NFC_DEBUG, 1); } while (0)
#define NFC_DEBUG_PIN_OFF()		do { nxp_soc_gpio_set_out_value(CFG_IO_NFC_DEBUG, 0); } while (0)
#else
#define NFC_DEBUG_PIN_ON()		do { gpio_set_value(CFG_IO_NFC_DEBUG, 1); } while (0)
#define NFC_DEBUG_PIN_OFF()		do { gpio_set_value(CFG_IO_NFC_DEBUG, 0); } while (0)
#endif

#else

#define NFC_DEBUG_PIN_ON()		do { } while (0)
#define NFC_DEBUG_PIN_OFF()		do { } while (0)

#endif	/* DEBUG_TRIGGER_GPIO */







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
static char * __parity_buffer0 = 0;         // Max 100 bit ecc
static char * __parity_buffer1 = 0;         // Max 100 bit ecc
static char * __fake_buffer0 = 0;           // 512/1024
static char * __fake_buffer1 = 0;           // 512/1024
static char * __dummy_buffer = 0;           // 1024
static char * __error_location_buffer0 = 0; // Max 100 bit ecc
static char * __error_location_buffer1 = 0; // Max 100 bit ecc

/******************************************************************************
 * Do Not Optimize
 ******************************************************************************/
void NFC_PHY_tDelay(unsigned int _tDelay)
{

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_NFC_DELAY_RW, ELAPSE_T_IO_NFC_DELAY_R, ELAPSE_T_IO_NFC_DELAY_W); }
#endif

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    {
        ktime_t t1 = ktime_get();
        ktime_t t2 = ktime_get();

        t1.tv64 += _tDelay;

        do
        {
            if (t2.tv64 >= t1.tv64)
            {
                break;
            }

            t2 = ktime_get();

        } while (1);
    }

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    {
        volatile unsigned int tDelay = _tDelay;
        volatile unsigned int dummy = 0;

        while (tDelay)
        {
            dummy = nfcI->nftacs;
            //nfcI->nftacs = dummy;
            tDelay -= 1;
        }
    }
#endif

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_NFC_DELAY_RW, ELAPSE_T_IO_NFC_DELAY_R, ELAPSE_T_IO_NFC_DELAY_W); }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
volatile unsigned int NFC_PHY_NfcontrolResetBit(volatile unsigned int _regval)
{
    volatile unsigned int regval = _regval;
    volatile unsigned int tmp = _regval;

    regval &= ~(__POW(1,NFCONTROL_HWBOOT_W) | __POW(1,NFCONTROL_EXSEL_W));
    regval |= __POW(__NROOT(tmp,NFCONTROL_EXSEL_R) & 0x1,NFCONTROL_EXSEL_W);

    return regval;
}

void NFC_PHY_SetAutoResetEnable(int _enable)
{
    volatile unsigned int regval = nfcI->nfcontrol;

    regval &= ~__POW(1,NFCONTROL_IRQPEND);
    if (_enable) { regval |= __POW(1,NFCONTROL_AUTORESET); }
    else         { regval &= ~__POW(1,NFCONTROL_AUTORESET); }
    regval = NFC_PHY_NfcontrolResetBit(regval);

    nfcI->nfcontrol = regval;
}

void NFC_PHY_ClearInterruptPending(int _intnum)
{
    volatile unsigned int regval = nfcI->nfcontrol;

    regval &= ~(__POW(1,NFCONTROL_IRQPEND) | __POW(1,NFCONTROL_ECCRST));
    regval |= __POW(1,NFCONTROL_IRQPEND);
    regval = NFC_PHY_NfcontrolResetBit(regval);

    nfcI->nfcontrol = regval;
}

void NFC_PHY_SetInterruptEnableAll(int _enable)
{
    volatile unsigned int regval = nfcI->nfcontrol;

    regval &= ~(__POW(1,NFCONTROL_IRQPEND) | __POW(1,NFCONTROL_IRQENB) | __POW(1,NFCONTROL_ECCIRQPEND) | __POW(1,NFCONTROL_ECCIRQENB) | __POW(1,NFCONTROL_ECCRST));
    regval |= __POW(_enable,NFCONTROL_IRQENB) | __POW(_enable,NFCONTROL_ECCIRQENB);
    regval = NFC_PHY_NfcontrolResetBit(regval);

    nfcI->nfcontrol = regval;
}

void NFC_PHY_SetNFBank(unsigned int _bank)
{
    volatile unsigned int regval = nfcI->nfcontrol;

    regval &= ~(__POW(1,NFCONTROL_IRQPEND) | __POW(3,NFCONTROL_BANK));
    regval |= __POW(_bank,NFCONTROL_BANK);
    regval = NFC_PHY_NfcontrolResetBit(regval);

    nfcI->nfcontrol = regval;
}

void NFC_PHY_SetNFCSEnable(unsigned int _select)
{
    volatile unsigned int regval = nfcI->nfcontrol;

    regval &= ~__POW(1,NFCONTROL_IRQPEND);
    if (_select) { regval |= __POW(1,NFCONTROL_NCSENB);  if (Exchange.sys.fn.LedNfcBusy) { Exchange.sys.fn.LedNfcBusy(); } }
    else         { regval &= ~__POW(1,NFCONTROL_NCSENB); if (Exchange.sys.fn.LedNfcIdle) { Exchange.sys.fn.LedNfcIdle(); } else { NFC_PHY_tDelay(1); } }
    regval = NFC_PHY_NfcontrolResetBit(regval);

    nfcI->nfcontrol = regval;
}

void NFC_PHY_SetEccBitMode(unsigned int _bytes_per_ecc, unsigned int _ecc_bits)
{
    volatile unsigned int regval = 0;
    unsigned int bytes_per_ecc = _bytes_per_ecc;
    unsigned int ecc_bits = _ecc_bits;
    unsigned int ecc_mode = 0;

    if (512 == bytes_per_ecc)
    {
        switch (ecc_bits)
        {
            case 4:  { ecc_mode = 0; } break;
            case 8:  { ecc_mode = 1; } break;
            case 12: { ecc_mode = 2; } break;
            case 16: { ecc_mode = 3; } break;
            case 24: { ecc_mode = 5; } break;
            default: { ecc_mode = 0xFFFFFFFF; /* error */ } break;
        }
    }
    else if (1024 == bytes_per_ecc)
    {
        switch (ecc_bits)
        {
            case 24: { ecc_mode = 4; } break;
            case 40: { ecc_mode = 6; } break;
            case 60: { ecc_mode = 7; } break;
            default: { ecc_mode = 0xFFFFFFFF; /* error */ } break;
        }
    }

    if (0xFFFFFFFF != ecc_mode)
    {
        regval = nfcI->nfcontrol;
        regval &= ~__POW(0x7,NFCONTROL_ECCMODE);
        regval |= __POW(ecc_mode,NFCONTROL_ECCMODE);
        regval = NFC_PHY_NfcontrolResetBit(regval);
        nfcI->nfcontrol = regval;
    }
}

void NFC_PHY_SetBytesPerEcc(unsigned int _bytes_per_ecc)
{
    volatile unsigned int regval = nfcI->nfeccctrl;
    unsigned int decmode = __NROOT(regval & __POW(1,NFECCCTRL_DECMODE_R),NFECCCTRL_DECMODE_R);
    unsigned int bytes_per_ecc = _bytes_per_ecc;

    regval = (regval & 0xaffffff) | __POW(decmode,NFECCCTRL_DECMODE_W);
    regval &= ~__POW(0x3FF,NFECCCTRL_DATACNT);
    regval |= __POW(bytes_per_ecc-1,NFECCCTRL_DATACNT) & __POW(0x3FF,NFECCCTRL_DATACNT);

    nfcI->nfeccctrl = regval;
}

void NFC_PHY_SetBytesPerParity(unsigned int _bytes_per_parity)
{
    volatile unsigned int regval = nfcI->nfeccctrl;
    unsigned int decmode = __NROOT(regval & __POW(1,NFECCCTRL_DECMODE_R),NFECCCTRL_DECMODE_R);
    unsigned int bytes_per_parity = _bytes_per_parity;

    regval = (regval & 0xaffffff) | __POW(decmode,NFECCCTRL_DECMODE_W);
    regval &= ~__POW(0xFF,NFECCCTRL_PDATACNT);
    regval |= __POW(bytes_per_parity,NFECCCTRL_PDATACNT) & __POW(0xFF,NFECCCTRL_PDATACNT);

    nfcI->nfeccctrl = regval;
}

void NFC_PHY_SetElpNum(unsigned int _elpnum)
{
    volatile unsigned int regval = nfcI->nfeccctrl;
    unsigned int decmode = __NROOT(regval & __POW(1,NFECCCTRL_DECMODE_R),NFECCCTRL_DECMODE_R);
    unsigned int elpnum = _elpnum;

    regval = (regval & 0xaffffff) | __POW(decmode,NFECCCTRL_DECMODE_W);
    regval &= ~__POW(0x7F,NFECCCTRL_ELPNUM);
    regval |= __POW(elpnum,NFECCCTRL_ELPNUM) & __POW(0x7F,NFECCCTRL_ELPNUM);

    nfcI->nfeccctrl = regval;
}

void NFC_PHY_SetEccMode(unsigned int _mode)
{
    volatile unsigned int regval = nfcI->nfeccctrl;
    unsigned int decmode = __NROOT(regval & __POW(1,NFECCCTRL_DECMODE_R),NFECCCTRL_DECMODE_R);
    unsigned int mode = _mode;

    regval = (regval & 0xaffffff) | __POW(decmode,NFECCCTRL_DECMODE_W);
    regval &= ~__POW(0x1,NFECCCTRL_DECMODE_W);
    regval |= __POW(mode,NFECCCTRL_DECMODE_W);

    nfcI->nfeccctrl = regval;

	dmb();
}

void NFC_PHY_RunEcc(void)
{
    volatile unsigned int regval = nfcI->nfeccctrl;
    unsigned int decmode = __NROOT(regval & __POW(1,NFECCCTRL_DECMODE_R),NFECCCTRL_DECMODE_R);

    regval = (regval & 0xaffffff) | __POW(decmode,NFECCCTRL_DECMODE_W);
    regval |= __POW(0x1,NFECCCTRL_RUNECC_W);

    nfcI->nfeccctrl = regval;

	dmb();
}

/******************************************************************************
 * ECC Encoder
 ******************************************************************************/
void NFC_PHY_EccEncoderSetup(unsigned int _bytes_per_ecc, unsigned int _ecc_bits)
{
    unsigned int r = 0;
    unsigned int k = _bytes_per_ecc;
    unsigned int m = (_bytes_per_ecc == 512)? 13: 14;
    unsigned int t = _ecc_bits;
  //unsigned int tmax = 24; // case _bytes_per_ecc == 512 ? 24 : 60

    r = (((m * t + 7) / 8) - 1) & 0xFF;

    NFC_PHY_SetEccBitMode(_bytes_per_ecc, _ecc_bits);

    NFC_PHY_SetBytesPerEcc(k);
    NFC_PHY_SetBytesPerParity(r);
    NFC_PHY_SetElpNum(t);

	dmb();
}

void NFC_PHY_EccEncoderEnable(void)
{
    nfcI->nfcontrol |= __POW(1,NFCONTROL_ECCRST);

    NFC_PHY_SetEccMode(0);
    NFC_PHY_RunEcc();

	dmb();
}

void NFC_PHY_EccEncoderReadParity(unsigned int * _parity, unsigned int _ecc_bits)
{
    unsigned int ecc_bits = _ecc_bits;
    unsigned int orgecc = 0;
    unsigned int loop = 0;
    unsigned int * parity = _parity;

    switch (ecc_bits)
    {
        case 4:  { orgecc = 2; } break;
        case 8:  { orgecc = 4; } break;
        case 12: { orgecc = 5; } break;
        case 16: { orgecc = 7; } break;
        case 24: { orgecc = 11; } break;
        case 40: { orgecc = 18; } break;
        case 60: { orgecc = 27; } break;
    }

    for (loop = 0; loop < orgecc; loop++)
    {
        parity[loop] = nfcI->nfecc[loop];
    }
}

void NFC_PHY_EccEncoderWaitDone(void)
{
    while (!(nfcI->nfeccstatus & __POW(1,NFECCSTATUS_ENCDONE)));
}

/******************************************************************************
 * ECC Decoder
 ******************************************************************************/
void NFC_PHY_EccDecoderReset(unsigned int _bytes_per_ecc, unsigned int _ecc_bits)
{
    NFC_PHY_SetEccBitMode(_bytes_per_ecc, _ecc_bits);

    nfcI->nfcontrol |= __POW(1,NFCONTROL_ECCRST);
    nfcI->nfeccautomode = (nfcI->nfeccautomode & ~__POW(1,NFECCAUTOMODE_CPUELP)) | __POW(1,NFECCAUTOMODE_CPUSYND);

	dmb();
}

void NFC_PHY_EccDecoderSetOrg(unsigned int * _parity, unsigned int _ecc_bits)
{
    unsigned int ecc_bits = _ecc_bits;
    unsigned int orgecc = 0;
    unsigned int loop = 0;
    unsigned int * parity = _parity;

    switch (ecc_bits)
    {
        case 4:  { orgecc = 2; } break;
        case 8:  { orgecc = 4; } break;
        case 12: { orgecc = 5; } break;
        case 16: { orgecc = 7; } break;
        case 24: { orgecc = 11; } break;
        case 40: { orgecc = 18; } break;
        case 60: { orgecc = 27; } break;
    }

    for (loop = 0; loop < orgecc; loop++)
    {
        nfcI->nforgecc[loop] = parity[loop];
    }

	dmb();
}

void NFC_PHY_EccDecoderEnable(unsigned int _bytes_per_ecc, unsigned int _ecc_bits)
{
    unsigned int r = 0;
    unsigned int k = _bytes_per_ecc;
    unsigned int m = (_bytes_per_ecc == 512)? 13: 14;
    unsigned int t = _ecc_bits;
  //unsigned int tmax = 24; // case _bytes_per_ecc == 512 ? 24 : 60

    r = ((m * t+ 7) / 8) - 1;

    // connect syndrome path
  //nfcI->nfeccautomode = nfcI->nfeccautomode & ~(__POW(1,NFECCAUTOMODE_CPUELP) | __POW(1,NFECCAUTOMODE_CPUSYND));

    // ecc run
    nfcI->nfeccctrl = __POW(1,NFECCCTRL_RUNECC_W) |
                      __POW(0,NFECCCTRL_ELPLOAD) |
                      __POW(NF_DECODE,NFECCCTRL_DECMODE_W) |
                      __POW(t & 0x7F,NFECCCTRL_ELPNUM) |
                      __POW(r & 0xFF,NFECCCTRL_PDATACNT) |
                      __POW((k-1) & 0x3FF,NFECCCTRL_DATACNT);

	dmb();
}

void NFC_PHY_EccDecoderWaitDone(void)
{
    while (!(nfcI->nfeccstatus & __POW(1,NFECCSTATUS_DECDONE)));
}

unsigned int NFC_PHY_EccDecoderHasError(void)
{
    return nfcI->nfeccstatus & __POW(1,NFECCSTATUS_ERROR);
}

void NFC_PHY_EccDecoderCorrection(unsigned int _bytes_per_ecc, unsigned int _ecc_bits)
{
    unsigned int r = 0;
    unsigned int k = _bytes_per_ecc;
    unsigned int m = (_bytes_per_ecc == 512)? 13: 14;
    unsigned int t = _ecc_bits;
  //unsigned int tmax = 24; // case _bytes_per_ecc == 512 ? 24 : 60

    // connect syndrome path
    nfcI->nfeccautomode = nfcI->nfeccautomode & ~(__POW(1,NFECCAUTOMODE_CPUELP) | __POW(1,NFECCAUTOMODE_CPUSYND));

    r = ((m * t+ 7) / 8) - 1;

    // load elp
    nfcI->nfeccctrl = __POW(0,NFECCCTRL_RUNECC_W) |
                      __POW(1,NFECCCTRL_ELPLOAD) |
                      __POW(NF_DECODE,NFECCCTRL_DECMODE_W) |
                      __POW(t & 0x07F,NFECCCTRL_ELPNUM) |
                      __POW(r & 0x0FF,NFECCCTRL_PDATACNT) |
                      __POW((k-1) & 0x3FF,NFECCCTRL_DATACNT);
}

void NFC_PHY_EccDecoderWaitCorrectionDone(void)
{
    while (nfcI->nfeccstatus & __POW(1,NFECCSTATUS_BUSY));
}

int NFC_PHY_EccDecoderGetErrorLocation(unsigned int * _errorlocationbuffer, unsigned int _ecc_bits)
{
    unsigned int * location = _errorlocationbuffer;
    unsigned int loop = 0;
    unsigned int length = __NROOT(_ecc_bits,1);

    if (__NROOT(nfcI->nfeccstatus & __POW(0x7F,NFECCSTATUS_NUMERR),NFECCSTATUS_NUMERR) !=
        __NROOT(nfcI->nfeccstatus & __POW(0x7F,NFECCSTATUS_ELPERR),NFECCSTATUS_ELPERR))
    {
        return -1;
    }

    for (loop = 0; loop < length; loop++)
    {
        volatile unsigned int regval = nfcI->nferrlocation[loop];
        *location++ = (__NROOT(regval,0) & 0x3FFF) ^ 0x7;
        *location++ = (__NROOT(regval,14) & 0x3FFF) ^ 0x7;
    }

    return __NROOT(nfcI->nfeccstatus & __POW(0x7F,NFECCSTATUS_NUMERR),NFECCSTATUS_NUMERR);
}

void NFC_PHY_ShowUncorrectedData(unsigned int _tried,
                                 unsigned int _row, unsigned int _col,
                                 char * _read_buffer,   unsigned int _bytes_per_ecc, unsigned int _ecc_bits,
                                 char * _parity_buffer, unsigned int _bytes_per_parity)
{
    int i = 0;
    int j = 0;

    unsigned int   tried = _tried;
    unsigned int   row = _row;
    unsigned int   col = _col;
    char         * read_buffer = _read_buffer;
    unsigned int   bytes_per_ecc = _bytes_per_ecc;
    unsigned int   ecc_bits = _ecc_bits;
    char         * parity_buffer = _parity_buffer;
    unsigned int   bytes_per_parity = _bytes_per_parity;

    Exchange.sys.fn.print("\n Data/Parity Address: row(%04d), col(%04d), paritycol(%04d) : Try #%d\n", row, col, col+bytes_per_ecc, tried);
    Exchange.sys.fn.print("  ------------------------------------------------------------------------------\n");
    Exchange.sys.fn.print("  Data : bytes_per_ecc(%d) ecc_bits(%d)\n", bytes_per_ecc, ecc_bits);

    for (i = 0; i < bytes_per_ecc/16; i++)
    {
        Exchange.sys.fn.print("  ");
        for (j = 0; j < 16; j++)
        {
            Exchange.sys.fn.print("%02x ", read_buffer[i * 16 + j]);
        }   Exchange.sys.fn.print("\n");
    }

    Exchange.sys.fn.print("\n");
    Exchange.sys.fn.print("  Parity : bytes_per_parity(%d)\n", bytes_per_parity);

    Exchange.sys.fn.print("  ");
    for (i = 0; i < bytes_per_parity; i++)
    {
        Exchange.sys.fn.print("%02x ", parity_buffer[i]);
    }   Exchange.sys.fn.print("\n");
    Exchange.sys.fn.print("  ------------------------------------------------------------------------------\n");
}

unsigned int NFC_PHY_EccCorrection(char         * _error_at,
                                   unsigned int   _row,
                                   unsigned int   _col,
                                   char         * _read_buffer,
                                   unsigned int   _bytes_per_ecc,
                                   unsigned int   _ecc_bits,
                                   char         * _parity_buffer,
                                   unsigned int   _bytes_per_parity,
                                   char         * _error_location_buffer,
                                   unsigned int * _report_ecc_correct_bit,
                                   unsigned int * _report_ecc_correct_sector,
                                   unsigned int * _report_ecc_max_correct_bit,
                                   unsigned int * _report_ecc_level,
                                   unsigned int * _report_ecc_level_error,
                                   unsigned int * _report_ecc_error)
{
    unsigned int   uncorrected = 0;

    char         * error_at = _error_at;
    unsigned int   row = _row;
    unsigned int   col = _col;
    char         * read_buffer = _read_buffer;
    unsigned int   bytes_per_ecc = _bytes_per_ecc;
    unsigned int   ecc_bits = _ecc_bits;
    char         * parity_buffer = _parity_buffer;
    unsigned int   bytes_per_parity = _bytes_per_parity;
    char         * error_location_buffer = _error_location_buffer;
    unsigned int * report_ecc_correct_bit = _report_ecc_correct_bit;
    unsigned int * report_ecc_correct_sector = _report_ecc_correct_sector;
    unsigned int * report_ecc_max_correct_bit = _report_ecc_max_correct_bit;
    unsigned int * report_ecc_level = _report_ecc_level;
    unsigned int * report_ecc_level_error = _report_ecc_level_error;
  //unsigned int * report_ecc_error = _report_ecc_error;

    unsigned int read_loop = 0;
    unsigned int * location = __NULL;
    int ecc_error_count = 0;

    for (read_loop = 0; read_loop < bytes_per_parity; read_loop++)
    {
        if (0xFF != parity_buffer[read_loop])
        {
            break;
        }
    }

#if 0 // for DEBUG: this page may have been erased.
    {
        unsigned int *puiMyData = (unsigned int *)read_buffer;
        unsigned int uiMyDataLen = bytes_per_ecc;
        unsigned int uiMyParityLen = bytes_per_parity;

        if (read_loop > ((uiMyParityLen * 95) / 100))    // 95% are 0xFF
        {
            Exchange.sys.fn.print("nfc.phy: NFC_PHY_2ndReadLog: this page may have been erased. read_loop:%d / %d \n"read_loop, uiMyParityLen);
            {
                int i=0;

                for (i=0; i < uiMyDataLen; i+=sizeof(U32))
                {
                    if (!(i%64))
                    {
                        Exchange.sys.fn.print("\n [0x%04X]", i);
                    }
                    Exchange.sys.fn.print("%02x ", puiMyData[i/sizeof(U32)]);
                }
                Exchange.sys.fn.print("\n");
            }
        }
    }
#endif

    if (read_loop != bytes_per_parity)
    {
        *report_ecc_correct_sector += __NROOT((512 != bytes_per_ecc) ? 1024 : bytes_per_ecc, 9);

        NFC_PHY_EccDecoderCorrection((512 != bytes_per_ecc) ? 1024 : bytes_per_ecc, ecc_bits);
        NFC_PHY_EccDecoderWaitCorrectionDone();

        location = (unsigned int *)error_location_buffer;
        ecc_error_count = NFC_PHY_EccDecoderGetErrorLocation(location, ecc_bits);

        if (ecc_error_count > 0)
        {
            unsigned int k = 0;
            unsigned int * err_read = (unsigned int *)read_buffer;

            *report_ecc_correct_bit += ecc_error_count;

            if (0 == *report_ecc_max_correct_bit)
            {
                *report_ecc_max_correct_bit = ecc_error_count;
            }
            else
            {
                if (ecc_error_count > *report_ecc_max_correct_bit)
                {
                    *report_ecc_max_correct_bit = ecc_error_count;
                }
            }

            for (k = 0; k < ecc_error_count; k++)
            {
                if (location[k] >= 8 * bytes_per_ecc)
                {
                    continue;
                }

                if (Exchange.debug.nfc.phy.info_ecc_correction)
                {
                    if (NULL != error_at) { Exchange.sys.fn.print("%s\n", error_at); error_at = NULL; }
                    Exchange.sys.fn.print("Correction: location[%d] = %08x\n", 0, location[k]);
                    Exchange.sys.fn.print("Correction: err_read[location[%d]/32]   %08x\n", k, err_read[location[k]/32]);
                    Exchange.sys.fn.print("Correction: __POW(1,location[%d]%%32]  ^ %08x\n", k, __POW(1,location[k]%32));
                    err_read[location[k]/32] ^= __POW(1,location[k]%32);
                    Exchange.sys.fn.print("Correction: err_read[location[%d]/32] = %08x\n", k, err_read[location[k]/32]);
                }
                else
                {
                    err_read[location[k]/32] ^= __POW(1,location[k]%32);
                }
            }

            if (ecc_error_count >= *report_ecc_level)
            {
                *report_ecc_level_error += 1;
            }

            if (Exchange.debug.nfc.phy.info_ecc_corrected)
            {
                if (NULL != error_at) { Exchange.sys.fn.print("%s\n", error_at); error_at = NULL; }
                Exchange.sys.fn.print("Corrected: row(%04d),col(%04d): count(%d)\n", row, col, ecc_error_count);
            }
        }
        // Uncorrectable
        else
        {
            uncorrected = 1;

#if 0 // print uncorrectable data 
            {
                int data_idx=0;
                unsigned int *data = (unsigned int *)read_buffer;

                Exchange.sys.fn.print("%s uncorrected data: row:%d col:%d", _error_at, row, col);
                for (data_idx=0; data_idx < (bytes_per_ecc/sizeof(unsigned int)); data_idx++)
                {
                    if (!(data_idx % 16))
                    {
                        Exchange.sys.fn.print("\n");
                        Exchange.sys.fn.print("[%04x] ", data_idx);
                    }
                    
                    Exchange.sys.fn.print("%08x ", data[data_idx]);
                }
                Exchange.sys.fn.print("\n");
            }
            
          //{
          //    int i = 0;
          //    char *parity = parity_buffer;
          //    Exchange.sys.fn.print("ParityBuff:0x%016lx", (unsigned long)parity);
          //    for (i=0; i < bytes_per_parity; i++)
          //    {
          //        Exchange.sys.fn.print(" %02x", parity[i]);
          //    }
          //    Exchange.sys.fn.print("\n");
          //}
#endif

        }
    }
    else
    {
        if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)read_buffer, 0xff, bytes_per_ecc); }
        else                         {                  memset((void *)read_buffer, 0xff, bytes_per_ecc); }
    }

    return uncorrected;
}

/******************************************************************************
 *
 ******************************************************************************/
unsigned int NFC_PHY_GetEccParitySize(unsigned int _ecc_codeword_size, unsigned int _eccbits)
{
    unsigned int i = 0;
    unsigned int uiBytesPerParity = 0;
    unsigned int m = (_ecc_codeword_size == 512)? 13: 14;

#if defined (__BUILD_MODE_X86_WINDOWS_SIMULATOR__)
    unsigned char pucValidEccList[] = {0};
#elif defined (__BUILD_MODE_X86_LINUX_DEVICE_DRIVER__)
    unsigned char pucValidEccList[] = {0};
#elif defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__) || defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    unsigned char pucValidEccList[] = {4,8,12,16,24,40,60};
#endif

    for (i = 0; i < sizeof (pucValidEccList); ++i)
    {
        if (_eccbits == pucValidEccList[i])
        {
            uiBytesPerParity = (m * _eccbits + 7) / 8;
            uiBytesPerParity = (uiBytesPerParity + (4-1)) & ~(4-1); // 4Byte Align
            break;
        }
    }

    return uiBytesPerParity;
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_SporInit(void)
{
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    if (Exchange.sys.support_list.spor)
    {
    }
#endif
}

void NFC_PHY_Spor(void)
{
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    if (Exchange.sys.support_list.spor)
    {
        // LVD Monitor
        if (!Exchange.sys.lvd_detected)
        {
            if (0 /* LVD Detect Logic */)
            {
                Exchange.sys.lvd_detected = 1;
            }
        }

        // Write Protecting
        if (Exchange.sys.lvd_detected)
        {
            NFC_PHY_WriteProtect(1);
        }
    }
#endif
}

void NFC_PHY_WriteProtect(unsigned int _wp_enable)
{
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    // NAND WP direction check
    {
        int io_dir = 0;

        io_dir = nxp_soc_gpio_get_io_dir(Exchange.sys.gpio.c_27);

        if (io_dir < 0)
        {
            Exchange.sys.fn.print("NFC_PHY_WriteProtect(Warning) : Invalid Write Protect Port Number !\n");
            return;
        }
        else if (1 != io_dir)
        {
            nxp_soc_gpio_set_io_dir(Exchange.sys.gpio.c_27, 1); // set output mode
        }
    }

    // NAND WP is Active Low
    switch (_wp_enable)
    {
        // Issue Writable
        case 0:
        {
            if (Exchange.sys.support_list.spor)
            {
                if (Exchange.sys.lvd_detected)
                {
                    Exchange.sys.fn.print("NFC_PHY_WriteProtect(Warning) : NAND Write Protected Forcibly !!! (Detected RSTN)\n");
                    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_27, 0);
                }
                else
                {
                    // Writable
                    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_27, 1);
                }
            }
            else
            {
                // Writable
                nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_27, 1);
            }

        } break;

        // Issue Write Protecting
        case 1:
        {
            // Write Protected
            nxp_soc_gpio_set_out_value(Exchange.sys.gpio.c_27, 0);

        } break;

        default:
        {
            Exchange.sys.fn.print("NFC_PHY_WriteProtect(Warning) : Invaild Function Parameter - 0 : disable, 1 : enable\n");

        } break;
    }
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    // NAND WP direction Out
    gpio_direction_output(CFG_IO_NAND_nWP, 1);

    // NAND WP is Active Low
    switch (_wp_enable)
    {
        case 0:  { gpio_set_value(CFG_IO_NAND_nWP, 1); } break; // Writable
        case 1:  { gpio_set_value(CFG_IO_NAND_nWP, 0); } break; // Write Protected
        default: { Exchange.sys.fn.print("NFC_PHY_WriteProtect(Warning) : Invaild Function Parameter - 0 : disable, 1 : enable\n"); } break;
    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_ChipSelect(unsigned int _channel, unsigned int _way, unsigned int _select)
{
  //unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int select = _select;

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (__TRUE == select) { if (Exchange.sys.fn.elapse_t_io_measure_start) { Exchange.sys.fn.elapse_t_io_measure_start(ELAPSE_T_IO_NFC_RW, ELAPSE_T_IO_NFC_R, ELAPSE_T_IO_NFC_W); } }
#endif

	dmb();

    NFC_PHY_SetNFBank(way);
    NFC_PHY_SetNFCSEnable(select);

	dmb();

#if defined (__COMPILE_MODE_ELAPSE_T__)
    if (__FALSE == select) { if (Exchange.sys.fn.elapse_t_io_measure_end) { Exchange.sys.fn.elapse_t_io_measure_end(ELAPSE_T_IO_NFC_RW, ELAPSE_T_IO_NFC_R, ELAPSE_T_IO_NFC_W); } }
#endif
}

void NFC_PHY_Cmd(unsigned char _cmd)
{
    NFC_PHY_Spor();
    nfcShadowI->nfcmd = _cmd;
}

void NFC_PHY_Addr(unsigned char _addr)
{
    nfcShadowI->nfaddr = _addr;
}

void NFC_PHY_WData(unsigned char _data)
{
    nfcShadowI->nfdata = _data;
}

void NFC_PHY_WData16(unsigned short _data)
{
    nfcShadowI16->nfdata = _data;
}

void NFC_PHY_WData32(unsigned int _data)
{
    nfcShadowI32->nfdata = _data;
}

unsigned char NFC_PHY_RData(void)
{
    return nfcShadowI->nfdata;
}

unsigned short NFC_PHY_RData16(void)
{
    return nfcShadowI16->nfdata;
}

unsigned int NFC_PHY_RData32(void)
{
    return nfcShadowI32->nfdata;
}

/******************************************************************************
 * BoostOn / BoostOff
 ******************************************************************************/

NF_TIME_REGS BoostOn_OriginTime;
NF_TIME_REGS BoostOn_BoostTime;

void NFC_PHY_Boost_time_regval(NF_TIME_REGS _t)
{
	BoostOn_BoostTime.nftacs = _t.nftacs;
	BoostOn_BoostTime.nftcos = _t.nftcos;
	BoostOn_BoostTime.nftacc = _t.nftacc;
	BoostOn_BoostTime.nftoch = _t.nftoch;
	BoostOn_BoostTime.nftcah = _t.nftcah;
}

void NFC_PHY_Origin_time_regval(NF_TIME_REGS _t)
{
	BoostOn_OriginTime.nftacs = _t.nftacs;
	BoostOn_OriginTime.nftcos = _t.nftcos;
	BoostOn_OriginTime.nftacc = _t.nftacc;
	BoostOn_OriginTime.nftoch = _t.nftoch;
	BoostOn_OriginTime.nftcah = _t.nftcah;
}

void NFC_PHY_BoostOn(void)
{
	NFC_PHY_ForceSet_Nftime(BoostOn_BoostTime);
}

void NFC_PHY_BoostOff(void)
{
	/* restore */
	NFC_PHY_ForceSet_Nftime(BoostOn_OriginTime);
}

/******************************************************************************
 *
 ******************************************************************************/
unsigned int NFC_PHY_Init(unsigned int _scan_format)
{
    unsigned int scan_format = _scan_format;

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    nfcI = (MCUS_I *)IO_ADDRESS(PHY_BASEADDR_MCUSTOP_MODULE);
#if defined (__COMPILE_MODE_X64__)
    nfcShadowI = (NFC_SHADOW_I *)nxp_nand.io_intf;
    nfcShadowI16 = (NFC_SHADOW_I16 *)nxp_nand.io_intf;
    nfcShadowI32 = (NFC_SHADOW_I32 *)nxp_nand.io_intf;
#else
    nfcShadowI = (NFC_SHADOW_I *)__PB_IO_MAP_NAND_VIRT;
    nfcShadowI16 = (NFC_SHADOW_I16 *)__PB_IO_MAP_NAND_VIRT;
    nfcShadowI32 = (NFC_SHADOW_I32 *)__PB_IO_MAP_NAND_VIRT;
#endif
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    nfcI = (MCUS_I *)IO_ADDRESS(PHY_BASEADDR_MCUSTOP_MODULE);
    nfcShadowI = (NFC_SHADOW_I *)CONFIG_SYS_NAND_BASE;
    nfcShadowI16 = (NFC_SHADOW_I16 *)CONFIG_SYS_NAND_BASE;
    nfcShadowI32 = (NFC_SHADOW_I32 *)CONFIG_SYS_NAND_BASE;
#endif

    NFC_PHY_DeInit();

#ifdef DEBUG_TRIGGER_GPIO
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
	nxp_soc_gpio_set_out_value(CFG_IO_NFC_DEBUG, 0);
	nxp_soc_gpio_set_io_dir(CFG_IO_NFC_DEBUG, 1);
	nxp_soc_gpio_set_io_func(CFG_IO_NFC_DEBUG, nxp_soc_gpio_get_altnum(CFG_IO_NFC_DEBUG));
#else
	gpio_request(CFG_IO_NFC_DEBUG, "NFC Debug pin");
	gpio_direction_output(CFG_IO_NFC_DEBUG, 1);
#endif
#endif

    NFC_PHY_ChipSelect(0, 0, __FALSE);
    NFC_PHY_ChipSelect(0, 1, __FALSE);

    Exchange.nfc.fnSuspend = NFC_PHY_Suspend;
    Exchange.nfc.fnResume = NFC_PHY_Resume;
    Exchange.nfc.fnGetFeatures = NFC_PHY_GetFeatures;
    Exchange.nfc.fnAdjustFeatures = NFC_PHY_AdjustFeatures;
    Exchange.nfc.fnSetFeatures = NFC_PHY_SetFeatures;
    Exchange.nfc.fnDelay = NFC_PHY_tDelay;
    Exchange.nfc.fnReadId = NFC_PHY_ReadId;
    Exchange.nfc.fnEccInfoInit = NFC_PHY_EccInfoInit;
    Exchange.nfc.fnEccInfoDeInit = NFC_PHY_EccInfoDeInit;
    Exchange.nfc.fnSetOnfiFeature = NFC_PHY_SetOnfiFeature;
    Exchange.nfc.fnGetOnfiFeature = NFC_PHY_GetOnfiFeature;
    Exchange.nfc.fnStatusIsFAIL = NFC_PHY_StatusIsFAIL;
    Exchange.nfc.fnStatusIsFAILC = NFC_PHY_StatusIsFAILC;
    Exchange.nfc.fnStatusIsARDY = NFC_PHY_StatusIsARDY;
    Exchange.nfc.fnStatusIsRDY = NFC_PHY_StatusIsRDY;
    Exchange.nfc.fnStatusIsWP = NFC_PHY_StatusIsWP;
    Exchange.nfc.fnStatusRead = NFC_PHY_StatusRead;
    Exchange.nfc.fnStatusData = NFC_PHY_StatusData;
    Exchange.nfc.fn1stRead = NFC_PHY_1stRead;
    Exchange.nfc.fn2ndReadDataNoEcc = NFC_PHY_2ndReadDataNoEcc;
    Exchange.nfc.fn2ndReadLog = NFC_PHY_2ndReadLog;
    Exchange.nfc.fn2ndReadData = NFC_PHY_2ndReadData;
    Exchange.nfc.fn3rdRead = NFC_PHY_3rdRead;
    Exchange.nfc.fn1stWriteLog = NFC_PHY_1stWriteLog;
    Exchange.nfc.fn1stWriteRoot = NFC_PHY_1stWriteRoot;
    Exchange.nfc.fn1stWriteData = NFC_PHY_1stWriteData;
    Exchange.nfc.fn2ndWrite = NFC_PHY_2ndWrite;
    Exchange.nfc.fn3rdWrite = NFC_PHY_3rdWrite;
    Exchange.nfc.fn1stErase = NFC_PHY_1stErase;
    Exchange.nfc.fn2ndErase = NFC_PHY_2ndErase;
    Exchange.nfc.fn3rdErase = NFC_PHY_3rdErase;
    Exchange.sys.fnSpor = NFC_PHY_Spor;
    //Exchange.nfc.fnBoostOn = NFC_PHY_BoostOn;
    //Exchange.nfc.fnBoostOff = NFC_PHY_BoostOff;

    /**************************************************************************
     * Set Default Delay Timing
     **************************************************************************/
    NfcTime.tRWC  = 100;    // ns
    NfcTime.tR    = 200000; // ns
    NfcTime.tWB   = 200;    // ns
    NfcTime.tCCS  = 400;    // ns
    NfcTime.tCCS2 = 400;    // ns
    NfcTime.tADL  = 400;    // ns
    NfcTime.tRHW  = 200;    // ns
    NfcTime.tWHR  = 120;    // ns
    NfcTime.tWW   = 100;    // ns
    NfcTime.tRR   = 40;     // ns
    NfcTime.tFEAT = 1000;   // ns

    NfcTime.tParity = 1000; // ns

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_SporInit();

    NFC_PHY_SetAutoResetEnable(__FALSE);
    NFC_PHY_ClearInterruptPending(0);
    NFC_PHY_SetInterruptEnableAll(__FALSE);

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    __parity_buffer0 = (char *)vmalloc(256 * sizeof(char));
    __parity_buffer1 = (char *)vmalloc(256 * sizeof(char));
    __fake_buffer0 = (char *)vmalloc(1024 * sizeof(char));
    __fake_buffer1 = (char *)vmalloc(1024 * sizeof(char));
    __dummy_buffer = (char *)vmalloc(1024 * sizeof(char));
    __error_location_buffer0 = (char *)vmalloc(512 * sizeof(char));
    __error_location_buffer1 = (char *)vmalloc(512 * sizeof(char));
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    __parity_buffer0 = (char *)malloc(256 * sizeof(char));
    __parity_buffer1 = (char *)malloc(256 * sizeof(char));
    __fake_buffer0 = (char *)malloc(1024 * sizeof(char));
    __fake_buffer1 = (char *)malloc(1024 * sizeof(char));
    __dummy_buffer = (char *)malloc(1024 * sizeof(char));
    __error_location_buffer0 = (char *)malloc(512 * sizeof(char));
    __error_location_buffer1 = (char *)malloc(512 * sizeof(char));
#endif

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print("EWS.NFC.PHY: Init\n"); }

    return NFC_PHY_ScanFeature(scan_format);
}

void NFC_PHY_DeInit(void)
{
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    if (__parity_buffer0)         { vfree(__parity_buffer0);         __parity_buffer0 = 0; }
    if (__parity_buffer1)         { vfree(__parity_buffer1);         __parity_buffer1 = 0; }
    if (__fake_buffer0)           { vfree(__fake_buffer0);           __fake_buffer0 = 0; }
    if (__fake_buffer1)           { vfree(__fake_buffer1);           __fake_buffer1 = 0; }
    if (__error_location_buffer0) { vfree(__error_location_buffer0); __error_location_buffer0 = 0; }
    if (__error_location_buffer1) { vfree(__error_location_buffer1); __error_location_buffer1 = 0; }
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    if (__parity_buffer0)         { free(__parity_buffer0);         __parity_buffer0 = 0; }
    if (__parity_buffer1)         { free(__parity_buffer1);         __parity_buffer1 = 0; }
    if (__fake_buffer0)           { free(__fake_buffer0);           __fake_buffer0 = 0; }
    if (__fake_buffer1)           { free(__fake_buffer1);           __fake_buffer1 = 0; }
    if (__error_location_buffer0) { free(__error_location_buffer0); __error_location_buffer0 = 0; }
    if (__error_location_buffer1) { free(__error_location_buffer1); __error_location_buffer1 = 0; }
#endif

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print("EWS.NFC.PHY: DeInit\n"); }
}

void NFC_PHY_Suspend(void)
{
    /* Nothing To Do */
}

void NFC_PHY_Resume(void)
{
    int way = 0;

    NFC_PHY_SporInit();
    NFC_PHY_SetAutoResetEnable(__FALSE);
    NFC_PHY_ClearInterruptPending(0);
    NFC_PHY_SetInterruptEnableAll(__FALSE);
    NFC_PHY_AdjustFeatures();

    for (way = 0; way < *Exchange.ftl.Way; way++)
    {
        NFC_PHY_NandReset(0, way);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
int NFC_PHY_EccInfoInit(unsigned int _max_channel, unsigned int _max_way, const unsigned char *_way_map)
{
    int resp = 0;
    unsigned int max_channels = _max_channel;
    unsigned int max_ways = _max_way;
    unsigned int way=0;
    unsigned int size=0;
    unsigned int i=0;

    NFC_PHY_EccInfoDeInit();

    for (i=0; i < max_ways; i++)
    {
        NfcEccStatus.phyway_map[i] = (_way_map)? _way_map[i]: i;
    }

    size = max_ways * sizeof(unsigned int *);

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    NfcEccStatus.level_error = (unsigned int **)vmalloc(size);
    NfcEccStatus.error = (unsigned int **)vmalloc(size);
    NfcEccStatus.correct_sector = (unsigned int **)vmalloc(size);
    NfcEccStatus.correct_bit = (unsigned int **)vmalloc(size);
    NfcEccStatus.max_correct_bit = (unsigned int **)vmalloc(size);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    NfcEccStatus.level_error = (unsigned int **)malloc(size);
    NfcEccStatus.error = (unsigned int **)malloc(size);
    NfcEccStatus.correct_sector = (unsigned int **)malloc(size);
    NfcEccStatus.correct_bit = (unsigned int **)malloc(size);
    NfcEccStatus.max_correct_bit = (unsigned int **)malloc(size);
#endif

    if (NfcEccStatus.level_error && NfcEccStatus.error && NfcEccStatus.correct_sector && NfcEccStatus.correct_bit && NfcEccStatus.max_correct_bit)
    {
        NfcEccStatus.max_ways = max_ways;

        size = max_channels * sizeof(unsigned int);

        for (way = 0; way < max_ways; way++)
        {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
            NfcEccStatus.level_error[way] = (unsigned int *)vmalloc(size);
            NfcEccStatus.error[way] = (unsigned int *)vmalloc(size);
            NfcEccStatus.correct_sector[way] = (unsigned int *)vmalloc(size);
            NfcEccStatus.correct_bit[way] = (unsigned int *)vmalloc(size);
            NfcEccStatus.max_correct_bit[way] = (unsigned int *)vmalloc(size);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
            NfcEccStatus.level_error[way] = (unsigned int *)malloc(size);
            NfcEccStatus.error[way] = (unsigned int *)malloc(size);
            NfcEccStatus.correct_sector[way] = (unsigned int *)malloc(size);
            NfcEccStatus.correct_bit[way] = (unsigned int *)malloc(size);
            NfcEccStatus.max_correct_bit[way] = (unsigned int *)malloc(size);
#endif

            if (NfcEccStatus.level_error[way] && NfcEccStatus.error[way] && NfcEccStatus.correct_sector[way] && NfcEccStatus.correct_bit[way] && NfcEccStatus.max_correct_bit[way])
            {
                NfcEccStatus.max_channels = max_channels;

                if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)(NfcEccStatus.level_error[way]), 0x00, size);
                                               Exchange.sys.fn._memset((void *)(NfcEccStatus.error[way]), 0x00, size);
                                               Exchange.sys.fn._memset((void *)(NfcEccStatus.correct_sector[way]), 0x00, size);
                                               Exchange.sys.fn._memset((void *)(NfcEccStatus.correct_bit[way]), 0x00, size);
                                               Exchange.sys.fn._memset((void *)(NfcEccStatus.max_correct_bit[way]), 0x00, size); }
                else                         {                  memset((void *)(NfcEccStatus.level_error[way]), 0x00, size);
                                                                memset((void *)(NfcEccStatus.error[way]), 0x00, size);
                                                                memset((void *)(NfcEccStatus.correct_sector[way]), 0x00, size);
                                                                memset((void *)(NfcEccStatus.correct_bit[way]), 0x00, size);
                                                                memset((void *)(NfcEccStatus.max_correct_bit[way]), 0x00, size); }
            }
            else
            {
                Exchange.sys.fn.print("NFC_PHY_EccInfoInit: error! memory allocation failed\n");
                resp = -1;
            }
        }

    }
    else
    {
        Exchange.sys.fn.print("NFC_PHY_EccInfoInit: error! memory allocation failed\n");
        resp = -1;
    }

    if (resp >= 0)
    {
        Exchange.nfc.ecc.level_error = (unsigned int **)NfcEccStatus.level_error;
        Exchange.nfc.ecc.error = (unsigned int **)NfcEccStatus.error;
        Exchange.nfc.ecc.correct_sector = (unsigned int **)NfcEccStatus.correct_sector;
        Exchange.nfc.ecc.correct_bit = (unsigned int **)NfcEccStatus.correct_bit;
        Exchange.nfc.ecc.max_correct_bit = (unsigned int **)NfcEccStatus.max_correct_bit;
    }

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print("EWS.NFC.PHY: Ecc Info Init:%d\n", resp); }

    return resp;
}

void NFC_PHY_EccInfoDeInit(void)
{
    unsigned int max_ways = NfcEccStatus.max_ways;
    unsigned int way=0;

    // levelover
    if (NfcEccStatus.level_error)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (NfcEccStatus.level_error[way])
            {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                vfree(NfcEccStatus.level_error[way]);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                free(NfcEccStatus.level_error[way]);
#endif
                NfcEccStatus.level_error[way] = 0;
            }
            else
            {
                break;
            }
        }
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(NfcEccStatus.level_error);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(NfcEccStatus.level_error);
#endif
        NfcEccStatus.level_error = 0;
    }

    // error
    if (NfcEccStatus.error)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (NfcEccStatus.error[way])
            {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                vfree(NfcEccStatus.error[way]);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                free(NfcEccStatus.error[way]);
#endif
                NfcEccStatus.error[way] = 0;
            }
            else
            {
                break;
            }
        }
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(NfcEccStatus.error);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(NfcEccStatus.error);
#endif
        NfcEccStatus.error = 0;
    }

    // correct_sector
    if (NfcEccStatus.correct_sector)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (NfcEccStatus.correct_sector[way])
            {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                vfree(NfcEccStatus.correct_sector[way]);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                free(NfcEccStatus.correct_sector[way]);
#endif
                NfcEccStatus.correct_sector[way] = 0;
            }
            else
            {
                break;
            }
        }
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(NfcEccStatus.correct_sector);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(NfcEccStatus.correct_sector);
#endif
        NfcEccStatus.correct_sector = 0;
    }

    // correct_bit
    if (NfcEccStatus.correct_bit)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (NfcEccStatus.correct_bit[way])
            {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                vfree(NfcEccStatus.correct_bit[way]);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                free(NfcEccStatus.correct_bit[way]);
#endif
                NfcEccStatus.correct_bit[way] = 0;
            }
            else
            {
                break;
            }
        }
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(NfcEccStatus.correct_bit);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(NfcEccStatus.correct_bit);
#endif
        NfcEccStatus.correct_bit = 0;
    }

    // max_correct_bit
    if (NfcEccStatus.max_correct_bit)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (NfcEccStatus.max_correct_bit[way])
            {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                vfree(NfcEccStatus.max_correct_bit[way]);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                free(NfcEccStatus.max_correct_bit[way]);
#endif
                NfcEccStatus.max_correct_bit[way] = 0;
            }
            else
            {
                break;
            }
        }
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(NfcEccStatus.max_correct_bit);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(NfcEccStatus.max_correct_bit);
#endif
        NfcEccStatus.max_correct_bit = 0;
    }

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print("EWS.NFC.PHY: Ecc Info DeInit\n"); }
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_GetFeatures(unsigned int * _max_channel, unsigned int * _max_way, void * _nand_config)
{
    *_max_channel = phy_features.max_channel;
    *_max_way = phy_features.max_way;

    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)_nand_config, (const void *)&phy_features.nand_config, sizeof(NAND)); }
    else                         {                  memcpy((void *)_nand_config, (const void *)&phy_features.nand_config, sizeof(NAND)); }
}

// When BCLK Changes, Must Be Reference.
void NFC_PHY_AdjustFeatures(void)
{
    NFC_PHY_SetFeatures(phy_features.max_channel, phy_features.max_way, (void *)&phy_features.nand_config);
}

int NFC_PHY_ForceSet_Nftime(NF_TIME_REGS t)
{
	unsigned int max_channel = phy_features.max_channel;
	unsigned int max_way = phy_features.max_way;
    unsigned int channel = 0;
    unsigned int way = 0;

	for (way = 0; way < max_way; way++)
	{
		for (channel = 0; channel < max_channel; channel++)
		{
			volatile unsigned int regval = 0;

			regval = nfcI->nftacs;
			regval &= ~__POW(0xF,way*4);
			regval |= __POW(t.nftacs,way*4);
			nfcI->nftacs = regval;

			regval = nfcI->nftcos;
			regval &= ~__POW(0xF,way*4);
			regval |= __POW(t.nftcos,way*4);
			nfcI->nftcos = regval;

			regval = nfcI->nftacc;
			regval &= ~__POW(0xFF,way*8);
			regval |= __POW(t.nftacc,way*8);
			nfcI->nftacc = regval;

			regval = nfcI->nftoch;
			regval &= ~__POW(0xF,way*4);
			regval |= __POW(t.nftoch,way*4);
			nfcI->nftoch = regval;

			regval = nfcI->nftcah;
			regval &= ~__POW(0xF,way*4);
			regval |= __POW(t.nftcah,way*4);
			nfcI->nftcah = regval;
		}
	}

	dmb();

	// debugging - Exchange.sys.fn.print
	//printf(" force =>\n tACS: %u, tCOS: %u, tACC: %u, tOCH: %u, tCAH: %u\n",
	//		nfcI->nftacs, nfcI->nftcos, nfcI->nftacc, nfcI->nftoch, nfcI->nftcah);

	return 0;
}

void NFC_PHY_SetFeatures(unsigned int _max_channel, unsigned int _max_way, void * _nand_config)
{
    unsigned int max_channel = _max_channel;
    unsigned int max_way = _max_way;
    unsigned int channel = 0;
    unsigned int way = 0;

    unsigned int tACS = 0;
    unsigned int tCOS = 0;
    unsigned int tACC = 0;
    unsigned int tOCH = 0;
    unsigned int tCAH = 0;

    phy_features.max_channel = _max_channel;
    phy_features.max_way = _max_way;

    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)&phy_features.nand_config, (const void *)_nand_config, sizeof(NAND)); }
    else                         {                  memcpy((void *)&phy_features.nand_config, (const void *)_nand_config, sizeof(NAND)); }

    NfcTime.tRWC    = phy_features.nand_config._f.timing.async.tRWC;
    NfcTime.tR      = phy_features.nand_config._f.timing.async.tR;
    NfcTime.tWB     = phy_features.nand_config._f.timing.async.tWB;
    NfcTime.tCCS    = phy_features.nand_config._f.timing.async.tCCS;
    NfcTime.tCCS2   = NfcTime.tCCS*2;
    NfcTime.tADL    = phy_features.nand_config._f.timing.async.tADL;
    NfcTime.tRHW    = phy_features.nand_config._f.timing.async.tRHW;
    NfcTime.tWHR    = phy_features.nand_config._f.timing.async.tWHR;
    NfcTime.tWW     = phy_features.nand_config._f.timing.async.tWW;
    NfcTime.tRR     = phy_features.nand_config._f.timing.async.tRR;
    NfcTime.tFEAT   = phy_features.nand_config._f.timing.async.tFEAT;
    NfcTime.tParity = 1000; // 1 us

    if (Exchange.debug.nfc.phy.info_feature)
    {
        Exchange.sys.fn.print("Input Feature:\n");
        Exchange.sys.fn.print(" max_channel: %u\n", phy_features.max_channel);
        Exchange.sys.fn.print(" max_way: %u\n", phy_features.max_way);
        Exchange.sys.fn.print(" interface_type: %u\n", phy_features.nand_config._f.interfacetype);
        Exchange.sys.fn.print(" onfi_timing_mode: %u\n", phy_features.nand_config._f.onfi_timing_mode);

        Exchange.sys.fn.print(" tClk: %u\n",  phy_features.nand_config._f.timing.async.tClk);
        Exchange.sys.fn.print(" tRWC: %u\n",  phy_features.nand_config._f.timing.async.tRWC);
        Exchange.sys.fn.print(" tR: %u\n",    phy_features.nand_config._f.timing.async.tR);
        Exchange.sys.fn.print(" tWB: %u\n",   phy_features.nand_config._f.timing.async.tWB);
        Exchange.sys.fn.print(" tCCS: %u\n",  phy_features.nand_config._f.timing.async.tCCS);
        Exchange.sys.fn.print(" tADL: %u\n",  phy_features.nand_config._f.timing.async.tADL);
        Exchange.sys.fn.print(" tRHW: %u\n",  phy_features.nand_config._f.timing.async.tRHW);
        Exchange.sys.fn.print(" tWHR: %u\n",  phy_features.nand_config._f.timing.async.tWHR);
        Exchange.sys.fn.print(" tWW: %u\n",   phy_features.nand_config._f.timing.async.tWW);
        Exchange.sys.fn.print(" tRR: %u\n",   phy_features.nand_config._f.timing.async.tRR);
        Exchange.sys.fn.print(" tFEAT: %u\n", phy_features.nand_config._f.timing.async.tFEAT);

        Exchange.sys.fn.print(" tCS: %u\n",   phy_features.nand_config._f.timing.async.tCS);
        Exchange.sys.fn.print(" tCH: %u\n",   phy_features.nand_config._f.timing.async.tCH);
        Exchange.sys.fn.print(" tCLS: %u\n",  phy_features.nand_config._f.timing.async.tCLS);
        Exchange.sys.fn.print(" tALS: %u\n",  phy_features.nand_config._f.timing.async.tALS);
        Exchange.sys.fn.print(" tCLH: %u\n",  phy_features.nand_config._f.timing.async.tCLH);
        Exchange.sys.fn.print(" tALH: %u\n",  phy_features.nand_config._f.timing.async.tALH);
        Exchange.sys.fn.print(" tWP: %u\n",   phy_features.nand_config._f.timing.async.tWP);
        Exchange.sys.fn.print(" tWH: %u\n",   phy_features.nand_config._f.timing.async.tWH);
        Exchange.sys.fn.print(" tWC: %u\n",   phy_features.nand_config._f.timing.async.tWC);
        Exchange.sys.fn.print(" tDS: %u\n",   phy_features.nand_config._f.timing.async.tDS);
        Exchange.sys.fn.print(" tDH: %u\n",   phy_features.nand_config._f.timing.async.tDH);

        Exchange.sys.fn.print(" tCEA: %u\n",  phy_features.nand_config._f.timing.async.tCEA);
        Exchange.sys.fn.print(" tREA: %u\n",  phy_features.nand_config._f.timing.async.tREA);
        Exchange.sys.fn.print(" tRP: %u\n",   phy_features.nand_config._f.timing.async.tRP);
        Exchange.sys.fn.print(" tREH: %u\n",  phy_features.nand_config._f.timing.async.tREH);
        Exchange.sys.fn.print(" tRC: %u\n",   phy_features.nand_config._f.timing.async.tRC);
        Exchange.sys.fn.print(" tCOH: %u\n",  phy_features.nand_config._f.timing.async.tCOH);
    }

    {
        unsigned int tR      = NfcTime.tR;
        unsigned int tWB     = NfcTime.tWB;
        unsigned int tCCS    = NfcTime.tCCS;
        unsigned int tCCS2   = NfcTime.tCCS2;
        unsigned int tADL    = NfcTime.tADL;
        unsigned int tRHW    = NfcTime.tRHW;
        unsigned int tWHR    = NfcTime.tWHR;
        unsigned int tWW     = NfcTime.tWW;
        unsigned int tRR     = NfcTime.tRR;
        unsigned int tFEAT   = NfcTime.tFEAT;
        unsigned int tParity = NfcTime.tParity;

        unsigned int tCS  = phy_features.nand_config._f.timing.async.tCS * 1000000;
        unsigned int tCH  = phy_features.nand_config._f.timing.async.tCH * 1000000;
        unsigned int tCLS = phy_features.nand_config._f.timing.async.tCLS * 1000000;
      //unsigned int tALS = phy_features.nand_config._f.timing.async.tALS * 1000000;
        unsigned int tCLH = phy_features.nand_config._f.timing.async.tCLH * 1000000;
      //unsigned int tALH = phy_features.nand_config._f.timing.async.tALH * 1000000;
        unsigned int tWP  = phy_features.nand_config._f.timing.async.tWP * 1000000;
        unsigned int tWH  = phy_features.nand_config._f.timing.async.tWH * 1000000;
        unsigned int tWC  = phy_features.nand_config._f.timing.async.tWC * 1000000;
        unsigned int tDS  = phy_features.nand_config._f.timing.async.tDS * 1000000;
        unsigned int tDH  = phy_features.nand_config._f.timing.async.tDH * 1000000;
        unsigned int tCEA = phy_features.nand_config._f.timing.async.tCEA * 1000000;
        unsigned int tREA = phy_features.nand_config._f.timing.async.tREA * 1000000;
        unsigned int tRP  = phy_features.nand_config._f.timing.async.tRP * 1000000;
        unsigned int tREH = phy_features.nand_config._f.timing.async.tREH * 1000000;
        unsigned int tRC  = phy_features.nand_config._f.timing.async.tRC * 1000000;
        unsigned int tCOH = phy_features.nand_config._f.timing.async.tCOH * 1000000;

        unsigned int tRCS = 0;
        unsigned int tWCS0 = 0;
        unsigned int tWCS1 = 0;
        unsigned int tRACC = 0;
        unsigned int tWACC = 0;
        unsigned int tRCH = 0;
        unsigned int tWCH0 = 0;
        unsigned int tWCH1 = 0;

        struct clk * clk;
        unsigned int clkhz;
        unsigned int nclk = 0;

        clk = clk_get(NULL, "bclk");
        clkhz = clk_get_rate(clk);
        clk_put(clk);

#if defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        // Adjust Nfc Timing
        NfcTime.tWB   *= 2;

        NfcTime.tR      /= 50;
        NfcTime.tWB     /= 50;
        NfcTime.tCCS    /= 50;
        NfcTime.tCCS2   /= 50;
        NfcTime.tADL    /= 50;
        NfcTime.tRHW    /= 50;
        NfcTime.tWHR    /= 50;
        NfcTime.tWW     /= 50;
        NfcTime.tRR     /= 50;
        NfcTime.tFEAT   /= 50;
        NfcTime.tParity /= 50;
#endif

        if (Exchange.debug.nfc.phy.info_feature)
        {
            Exchange.sys.fn.print("Change Feature:\n");
            Exchange.sys.fn.print(" tR:      %u -> %u\n", tR, NfcTime.tR);
            Exchange.sys.fn.print(" tWB:     %u -> %u\n", tWB, NfcTime.tWB);
            Exchange.sys.fn.print(" tCCS:    %u -> %u\n", tCCS, NfcTime.tCCS);
            Exchange.sys.fn.print(" tCCS2:   %u -> %u\n", tCCS2, NfcTime.tCCS2);
            Exchange.sys.fn.print(" tADL:    %u -> %u\n", tADL, NfcTime.tADL);
            Exchange.sys.fn.print(" tRHW:    %u -> %u\n", tRHW, NfcTime.tRHW);
            Exchange.sys.fn.print(" tWHR:    %u -> %u\n", tWHR, NfcTime.tWHR);
            Exchange.sys.fn.print(" tWW:     %u -> %u\n", tWW, NfcTime.tWW);
            Exchange.sys.fn.print(" tRR:     %u -> %u\n", tRR, NfcTime.tRR);
            Exchange.sys.fn.print(" tFEAT:   %u -> %u\n", tFEAT, NfcTime.tFEAT);
            Exchange.sys.fn.print(" tParity: %u -> %u\n", tParity, NfcTime.tParity);
        }

        // timing calculation
        nclk = 1000000000/(clkhz/1000); // convert to pico second

        tRCS = (tCEA - tREA) / nclk;                                                                // (tCEA-tREA)/nclk
        tWCS0 = (tCS - tWP) / nclk;                                                                 // (tCS-tWP)/nclk
        tWCS1 = (tCLS - tWP) / nclk;                                                                // (tCLS-tWP)/nclk
        tRACC = ((tREA + nclk * 2000) > tRP ? (tREA + nclk * 2000) : tRP) / nclk;                   // MAX(tREA+nclk*2, tRP)/nclk
        tWACC = ((tWP > tDS) ? tWP : tDS) / nclk;                                                   // MAX(tWP,tDS)/nclk
        tRCH = ((tRC - tRP) > tREH ? (tRC - tRP) : tREH) / nclk - tRCS;                             // MAX(tRC-tRP,tREH)/nclk-tRCS
        tWCH0 = ((tWC - tWP) > tWH ? (tWC - tWP) : tWH) / nclk - (tWCS0 > tWCS1 ? tWCS0 : tWCS1);   // MAX(tWC-tWP, tWH)/nclk - MAX(tWCS0, tWCS1)
        tWCH1 = ((tCH > tCLH ? tCH : tCLH) > tDH ? (tCH > tCLH ? tCH : tCLH) : tDH) / nclk;         // MAX(tCH,tCLH,tDH)/nclk

        // convertion to clock base asynchronous nand controller state machine
        tCOS = (tRCS > tWCS0 ? tRCS : tWCS0) > tWCS1 ? (tRCS > tWCS0 ? tRCS : tWCS0) : tWCS1;       // MAX(tRCS, tWCS0, tWCS1);
        tACC = tRACC > tWACC ? tRACC : tWACC;                                                       // MAX(tRACC, tWACC);
        tOCH = (tRCH > tWCH0 ? tRCH : tWCH0) > tWCH1 ? (tRCH > tWCH0 ? tRCH : tWCH0) : tWCH1;       // MAX(tRCH, tWCH0, tWCH1);

        // convert to register value
        tCOS += 999; tCOS /= 1000;  // round up tCOS
        tACC += 999; tACC /= 1000;  // round up tACC
        tOCH += 999; tOCH /= 1000;  // round up tOCH

        if (Exchange.debug.nfc.phy.info_feature)
        {
            Exchange.sys.fn.print("Timing Calculation\n");
            Exchange.sys.fn.print(" BCLK: %u HZ\n", clkhz);
            Exchange.sys.fn.print(" tCS: %u, tCH: %u tCLS: %u, tCLH: %u, tWP: %u, tWH: %u, tWC: %u\n", tCS, tCH, tCLS, tCLH, tWP, tWH, tWC);
            Exchange.sys.fn.print(" tDS: %u, tDH: %u, tCEA: %u, tREA: %u, tRP: %u, tREH: %u, tRC: %u, tCOH: %u\n", tDS, tDH, tCEA, tREA, tRP, tREH, tRC, tCOH);
            Exchange.sys.fn.print(" tRCS: %u, tWCS0: %u, tWCS1: %u, tRACC: %u, tWACC: %u, tRCH: %u, tWCH0: %u, tWCH1: %u\n", tRCS, tWCS0, tWCS1, tRACC, tWACC, tRCH, tWCH0, tWCH1);
            Exchange.sys.fn.print(" tACS: %u, tCOS: %u, tACC: %u, tOCH: %u, tCAH: %u\n", tACS, tCOS, tACC, tOCH, tCAH);
        }
    }

    switch (phy_features.nand_config._f.interfacetype)
    {
        case NAND_INTERFACE_ASYNC:
        case NAND_INTERFACE_ONFI_ASYNC:
        {
            volatile unsigned int regval = 0;

            tACS = (tACS + 15) & 0xF;
            tCOS = (tCOS + 15) & 0xF;
            tACC -= 1;
            tOCH = (tOCH + 15) & 0xF;
            tCAH = (tCAH + 15) & 0xF;

            if (Exchange.debug.nfc.phy.info_feature)
            {
                Exchange.sys.fn.print(" tACS: %u, tCOS: %u, tACC: %u, tOCH: %u, tCAH: %u\n", tACS, tCOS, tACC, tOCH, tCAH);
            }

            for (way = 0; way < max_way; way++)
            {
                for (channel = 0; channel < max_channel; channel++)
                {
                    regval = nfcI->nftacs;
                    regval &= ~__POW(0xF,way*4);
                    regval |= __POW(tACS,way*4);
                    nfcI->nftacs = regval;

                    regval = nfcI->nftcos;
                    regval &= ~__POW(0xF,way*4);
                    regval |= __POW(tCOS,way*4);
                    nfcI->nftcos = regval;

                    regval = nfcI->nftacc;
                    regval &= ~__POW(0xFF,way*8);
                    regval |= __POW(tACC,way*8);
                    nfcI->nftacc = regval;

                    regval = nfcI->nftoch;
                    regval &= ~__POW(0xF,way*4);
                    regval |= __POW(tOCH,way*4);
                    nfcI->nftoch = regval;

                    regval = nfcI->nftcah;
                    regval &= ~__POW(0xF,way*4);
                    regval |= __POW(tCAH,way*4);
                    nfcI->nftcah = regval;

                    if (NAND_INTERFACE_ONFI_ASYNC == phy_features.nand_config._f.interfacetype)
                    {
                        NFC_PHY_SetOnfiFeature(channel, way, Exchange.nfc.onfi_feature_address.timing_mode, phy_features.nand_config._f.onfi_timing_mode);
                    }
                }
            }

            {
                NF_TIME_REGS _t1 = {  0xf,  0x0,  0x7,  0x0,  0xf };
                NF_TIME_REGS _t2 = { tACS, tCOS, tACC, tOCH, tCAH };

                NFC_PHY_Boost_time_regval(_t1);
                NFC_PHY_Origin_time_regval(_t2);
			}

        } break;

        case NAND_INTERFACE_ONFI_SYNC: {} break;    /* Not Support NXP4330 */
        case NAND_INTERFACE_TOGGLE_ASYNC: {} break;
        case NAND_INTERFACE_TOGGLE_DDR: {} break;   /* Not Support NXP4330 */
    }
}

/******************************************************************************
 *
 ******************************************************************************/
int NFC_PHY_ReadId(unsigned int _channel, unsigned int _way, char * _id, char * _onfi_id, char * _jedec_id)
{
    int res = 0;
    unsigned int channel = _channel;
    unsigned int way = _way;
    char * id = _id;
    char * onfi_id = _onfi_id;
    char * jedec_id = _jedec_id;
    char status = 0;

    unsigned int expire = 10000;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_WriteProtect(1);
        NFC_PHY_tDelay(NfcTime.tWW);

        NFC_PHY_Cmd(NF_CMD_RESET);
        NFC_PHY_tDelay(NfcTime.tWB);

        NFC_PHY_WriteProtect(0);
        NFC_PHY_tDelay(NfcTime.tWW);

        // Wait tRST
        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);

        while (!NFC_PHY_StatusIsRDY(nfcShadowI->nfdata) && expire)
        {
            expire -= 1;
        }

        // If id buffer is NULL, this function issue NAND Reset Only.

        if (0 == expire)
        {
            if (id)
            {
                *id = 0; id++;
                *id = 0; id++;
                *id = 0; id++;
                *id = 0; id++;
                *id = 0; id++;
                *id = 0; id++;
                *id = 0; id++;
                *id = 0; id++;
            }

            if (onfi_id)
            {
                *onfi_id = 0; onfi_id++;
                *onfi_id = 0; onfi_id++;
                *onfi_id = 0; onfi_id++;
                *onfi_id = 0; onfi_id++;
            }

            if (jedec_id)
            {
                *jedec_id = 0; jedec_id++;
                *jedec_id = 0; jedec_id++;
                *jedec_id = 0; jedec_id++;
                *jedec_id = 0; jedec_id++;
                *jedec_id = 0; jedec_id++;
                *jedec_id = 0; jedec_id++;
            }

            res = -1;
        }
        else
        {
            if (id)
            {
                status = nfcShadowI->nfdata;

                NFC_PHY_Cmd(NF_CMD_READ_ID);
                NFC_PHY_Addr(0x00);
                NFC_PHY_tDelay(NfcTime.tWHR);

                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata;               id++;
                *id = nfcShadowI->nfdata; *id = status; id++;
            }

            if (onfi_id)
            {
                NFC_PHY_Cmd(NF_CMD_READ_ID);
                NFC_PHY_Addr(0x20);
                NFC_PHY_tDelay(NfcTime.tWHR);

                *onfi_id++ = nfcShadowI->nfdata;
                *onfi_id++ = nfcShadowI->nfdata;
                *onfi_id++ = nfcShadowI->nfdata;
                *onfi_id++ = nfcShadowI->nfdata;
            }

            if (jedec_id)
            {
                NFC_PHY_Cmd(NF_CMD_READ_ID);
                NFC_PHY_Addr(0x40);
                NFC_PHY_tDelay(NfcTime.tWHR);

                *jedec_id++ = nfcShadowI->nfdata;
                *jedec_id++ = nfcShadowI->nfdata;
                *jedec_id++ = nfcShadowI->nfdata;
                *jedec_id++ = nfcShadowI->nfdata;
                *jedec_id++ = nfcShadowI->nfdata;
                *jedec_id++ = nfcShadowI->nfdata;
            }
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return res;
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_NandReset(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_RESET);
        NFC_PHY_tDelay(NfcTime.tWB);

        // Wait tRST
        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
        while (!NFC_PHY_StatusIsRDY(nfcShadowI->nfdata));
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);
}

/******************************************************************************
 * ONFI Feature
 ******************************************************************************/
void NFC_PHY_SetOnfiFeature(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned int _parameter)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned char feature_address = _feature_address;
    unsigned int parameter = _parameter;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_SET_FEATURE);
        NFC_PHY_Addr(feature_address);
        NFC_PHY_tDelay(NfcTime.tADL);

        nfcShadowI->nfdata = __NROOT(parameter&0x000000FF,0);
        nfcShadowI->nfdata = __NROOT(parameter&0x0000FF00,8);
        nfcShadowI->nfdata = __NROOT(parameter&0x00FF0000,16);
        nfcShadowI->nfdata = __NROOT(parameter&0xFF000000,24);

        NFC_PHY_tDelay(NfcTime.tFEAT);
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);
}

void NFC_PHY_GetOnfiFeature(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned int * _parameter)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned char feature_address = _feature_address;
    unsigned int * parameter = _parameter;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_GET_FEATURE);
        NFC_PHY_Addr(feature_address);
        NFC_PHY_tDelay(NfcTime.tADL);
        NFC_PHY_tDelay(NfcTime.tFEAT);

        *parameter  = __POW(nfcShadowI->nfdata,0);
        *parameter |= __POW(nfcShadowI->nfdata,8);
        *parameter |= __POW(nfcShadowI->nfdata,16);
        *parameter |= __POW(nfcShadowI->nfdata,24);
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);
}

void NFC_PHY_GetStandardParameter(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned char * _parameter, unsigned char * _ext_parameter)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned char feature_address = _feature_address;

    unsigned char * read_parameter = (unsigned char *)0;
    unsigned char * read_ext_parameter = (unsigned char *)0;
    unsigned int parameter_page_loop = 0;
    unsigned int ext_parameter_section_loop = 0;
    unsigned int read_loop = 0;
    unsigned int read_offset = 0;
    unsigned int read_done = 0;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_READ_PARAM);
        NFC_PHY_Addr(feature_address);
        NFC_PHY_tDelay(NfcTime.tWB);
        NFC_PHY_tDelay(NfcTime.tR);
        NFC_PHY_tDelay(NfcTime.tRR);

        /**********************************************************************
         * ONFI Parameter
         **********************************************************************/
        if (0x00 == feature_address)
        {
            ONFI_PARAMETER * onfi_param = (ONFI_PARAMETER *)_parameter;
            ONFI_EXT_PARAMETER * onfi_ext_param = (ONFI_EXT_PARAMETER *)_ext_parameter;

            /******************************************************************
             * Read ONFI Parameter
             ******************************************************************/
            read_offset = 0;
            read_done = 0;

            // Read Mandatory ONFI Parameter Pages
            for (parameter_page_loop = 0; parameter_page_loop < 3; parameter_page_loop++)
            {
                read_parameter = (unsigned char *)onfi_param->_c;

                for (read_loop = 0; read_loop < sizeof(onfi_param->_c); read_loop++)
                {
                    if (!read_done) { *read_parameter++ = nfcShadowI->nfdata; }
                    else            { nfcShadowI->nfdata; }

                    read_offset += 1;
                }

                if (!read_done)
                {
                    // Check Integrity CRC
                    if (onfi_param->_f.integrity_crc == Exchange.sys.fn.get_crc16(0x4F4E, (void *)onfi_param->_c, sizeof(onfi_param->_c) - 2))
                    {
                        read_done = 1;
                    }
                }

                if (read_done)
                {
                    unsigned int col = 3*256 - read_offset;

                    // If Breaking Loop, Must Be Column Change
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                    NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                    NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                    NFC_PHY_tDelay(NfcTime.tCCS);

                    read_offset += col;

                    break;
                }
            }

            if (read_done)
            {
                unsigned int col = (onfi_param->_f.number_of_parameter_pages-3)*256 + read_offset;

                // If Skip Loop, Must Be Column Change
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                NFC_PHY_tDelay(NfcTime.tCCS);

                read_offset += col;
            }
            else
            {
                // Read Additional ONFI Parameter Pages
                for (parameter_page_loop = 3; parameter_page_loop < onfi_param->_f.number_of_parameter_pages; parameter_page_loop++)
                {
                    read_parameter = (unsigned char *)onfi_param->_c;

                    for (read_loop = 0; read_loop < sizeof(onfi_param->_c); read_loop++)
                    {
                        if (!read_done) { *read_parameter++ = nfcShadowI->nfdata; }
                        else            { nfcShadowI->nfdata; }

                        read_offset += 1;
                    }

                    if (!read_done)
                    {
                        // Check Integrity CRC
                        if (onfi_param->_f.integrity_crc == Exchange.sys.fn.get_crc16(0x4F4E, (void *)onfi_param->_c, sizeof(onfi_param->_c) - 2))
                        {
                            read_done = 1;
                        }
                    }
                }
            }

            /******************************************************************
             * Read ONFI Extended Parameter
             ******************************************************************/
            read_offset = 0;
            read_done = 0;

            if (onfi_param->_f.features_supported.extended_parameter_page)
            {
                for (parameter_page_loop = 0; parameter_page_loop < onfi_param->_f.number_of_parameter_pages; parameter_page_loop++)
                {
                    unsigned int entire_section_length = 0;

                    /**********************************************************
                     * Read Section Type & Length
                     **********************************************************/
                    read_parameter = (unsigned char *)onfi_ext_param->_c;

                    for (read_loop = 0; read_loop < 32; read_loop++)
                    {
                        if (!read_done) { *read_parameter++ = nfcShadowI->nfdata; }
                        else            { nfcShadowI->nfdata; }
                    }

                    /**********************************************************
                     * Sections
                     **********************************************************/
                    entire_section_length = 32;
                    entire_section_length += onfi_ext_param->_f.section_0_type ? onfi_ext_param->_f.section_0_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_1_type ? onfi_ext_param->_f.section_1_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_2_type ? onfi_ext_param->_f.section_2_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_3_type ? onfi_ext_param->_f.section_3_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_4_type ? onfi_ext_param->_f.section_4_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_5_type ? onfi_ext_param->_f.section_5_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_6_type ? onfi_ext_param->_f.section_6_length * 16 : 0;
                    entire_section_length += onfi_ext_param->_f.section_7_type ? onfi_ext_param->_f.section_7_length * 16 : 0;

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                    read_ext_parameter = vmalloc(entire_section_length);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                    read_ext_parameter = malloc(entire_section_length);
#endif
                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)read_ext_parameter, (const void *)onfi_ext_param->_c, 32); }
                    else                         {                  memcpy((void *)read_ext_parameter, (const void *)onfi_ext_param->_c, 32); }

                    read_parameter = read_ext_parameter + 32;

                    for (ext_parameter_section_loop = 0; ext_parameter_section_loop < 8; ext_parameter_section_loop++)
                    {
                        unsigned int section_type = 0;
                        unsigned int section_length = 0;

                        switch (ext_parameter_section_loop)
                        {
                            case 0: { section_type = onfi_ext_param->_f.section_0_type; section_length = onfi_ext_param->_f.section_0_length; } break;
                            case 1: { section_type = onfi_ext_param->_f.section_1_type; section_length = onfi_ext_param->_f.section_1_length; } break;
                            case 2: { section_type = onfi_ext_param->_f.section_2_type; section_length = onfi_ext_param->_f.section_2_length; } break;
                            case 3: { section_type = onfi_ext_param->_f.section_3_type; section_length = onfi_ext_param->_f.section_3_length; } break;
                            case 4: { section_type = onfi_ext_param->_f.section_4_type; section_length = onfi_ext_param->_f.section_4_length; } break;
                            case 5: { section_type = onfi_ext_param->_f.section_5_type; section_length = onfi_ext_param->_f.section_5_length; } break;
                            case 6: { section_type = onfi_ext_param->_f.section_6_type; section_length = onfi_ext_param->_f.section_6_length; } break;
                            case 7: { section_type = onfi_ext_param->_f.section_7_type; section_length = onfi_ext_param->_f.section_7_length; } break;
                        }

                        switch (section_type)
                        {
                            case 0: { /* Nothing To Do */ } break;

                            case 1:
                            {
                                for (read_loop = 0; read_loop < (section_length*16); read_loop++)
                                {
                                    *read_parameter++ = nfcShadowI->nfdata;
                                }

                            } break;

                            case 2:
                            {
                                unsigned char * it_buffer = read_parameter;

                                /* Extended ECC Information */
                                for (read_loop = 0; read_loop < (section_length*16); read_loop++)
                                {
                                    if (!read_done) { *read_parameter++ = nfcShadowI->nfdata; }
                                    else            { nfcShadowI->nfdata; }
                                }

                                if (!read_done)
                                {
                                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)(onfi_ext_param->_c + 32), (const void *)it_buffer, sizeof(unsigned char) * section_length * 16); }
                                    else                         {                  memcpy((void *)(onfi_ext_param->_c + 32), (const void *)it_buffer, sizeof(unsigned char) * section_length * 16); }
                                }

                            } break;
                        }
                    }

                    if (!read_done)
                    {
                        // Check Integrity CRC
                        if (onfi_ext_param->_f.extended_parameter_page_integrity_crc == Exchange.sys.fn.get_crc16(0x4F4E, (void *)(read_ext_parameter+2), entire_section_length-2))
                        {
                            read_done = 1;
                        }
                    }

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                    vfree(read_ext_parameter);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                    free(read_ext_parameter);
#endif
                    if (read_done)
                    {
                        break;
                    }
                }
            }
        }
        /**********************************************************************
         * JEDEC Parameter
         **********************************************************************/
        else if (0x40 == feature_address)
        {
            JEDEC_PARAMETER * jedec_param = (JEDEC_PARAMETER *)_parameter;

            /******************************************************************
             * Read JEDEC standard formatted Parameter
             ******************************************************************/
            read_offset = 0;
            read_done = 0;

            for (parameter_page_loop = 0; parameter_page_loop < 16; parameter_page_loop++)
            {
                read_parameter = (unsigned char *)jedec_param->_c;

                for (read_loop = 0; read_loop < sizeof(jedec_param->_c); read_loop++)
                {
                    if (!read_done) { *read_parameter++ = nfcShadowI->nfdata; }
                    else            { nfcShadowI->nfdata; }

                    read_offset += 1;
                }

                if (!read_done)
                {
                    // Check Integrity CRC
                    if (jedec_param->_f.integrity_crc == Exchange.sys.fn.get_crc16(0x4F4E, (void *)jedec_param->_c, sizeof(jedec_param->_c) - 2))
                    {
                        read_done = 1;
                        break;
                    }
                }
            }

#if 0 // view the parameters
            Exchange.sys.fn.print("parameter_page_loop:%d \n", parameter_page_loop);
            {
                unsigned int i=0;

                for (i=0; i<512; i++)
                {
                    if (i%32 == 0)
                  //if (i%10 == 0)
                    {
                        Exchange.sys.fn.print("\n[%04x(%04d)] ", i, i);
                    }

                    Exchange.sys.fn.print("%02x ", jedec_param->_c[i]);
                }
            }
            Exchange.sys.fn.print("\n");
#endif

        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);
}


/******************************************************************************
 *
 ******************************************************************************/
int NFC_PHY_StatusIsFAIL(unsigned char _status)
{
    // Pass/Fail (N) : 0=Pass, 1=Fail
    return (_status & __POW(1,0)) >> 0;
}

int NFC_PHY_StatusIsFAILC(unsigned char _status)
{
    // Pass/Fail (N-1) : 0=Pass, 1=Fail
    return (_status & __POW(1,1)) >> 1;
}

int NFC_PHY_StatusIsARDY(unsigned char _status)
{
    // Ready/Busy Array : 0=Busy, 1=Ready
    return (_status & __POW(1,5)) >> 5;
}

int NFC_PHY_StatusIsRDY(unsigned char _status)
{
    // Ready/Busy I/O : 0=Busy, 1=Ready
    return (_status & __POW(1,6)) >> 6;
}

int NFC_PHY_StatusIsWP(unsigned char _status)
{
    // Write Protect : 0=Protected, 1=Not Protected
    return (_status & __POW(1,7)) ? 0 : 1;
}

unsigned char NFC_PHY_StatusRead(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned char status = 0;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
        status = nfcShadowI->nfdata;
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return status;
}

unsigned char NFC_PHY_StatusData(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned char data = 0;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        data = nfcShadowI->nfdata;
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return data;
}

/******************************************************************************
 *
 ******************************************************************************/
int NFC_PHY_1stRead(unsigned int _channel,
                    unsigned int _way,
                    unsigned int _row,
                    unsigned int _col)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row = _row;
    unsigned int col = _col;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_READ_1ST);
        NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
        NFC_PHY_Addr(__NROOT(row&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(row&0x0000FF00,8));
        NFC_PHY_Addr(__NROOT(row&0x00FF0000,16));
        NFC_PHY_Cmd(NF_CMD_READ_2ND);
        NFC_PHY_tDelay(NfcTime.tWB);

        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    NFC_PHY_RAND_SetPageSeed(row&0x000000FF);

    /**************************************************************************
     * FTL Side : Violate Prohibited Block Access, But Allowed
     **************************************************************************/
    {
        unsigned int block = __NROOT(_row&0x000F0000,8) + __NROOT(_row&0x0000FF00,8);

        if ((0 <= block) && (block < Exchange.ewsftl_start_block))
        {
            if (Exchange.debug.nfc.phy.warn_prohibited_block_access)
            {
                Exchange.sys.fn.print("EWS.NFC: Warnning: Read Requested \"Block %d\" Prohibited, But Allowed !!!\n", block);
            }
        }
    }

    return 0;
}

int NFC_PHY_2ndReadDataNoEcc(unsigned int _channel,
                             unsigned int _way,
                             /* DATA */
                             unsigned int _data_loop_count,
                             unsigned int _bytes_per_data_ecc,
                             void * _data_buffer,
                             /* SPARE */
                             unsigned int _bytes_spare,
                             void * _spare_buffer)
{
    unsigned int channel = _channel;
    unsigned int way = _way;

    /* DATA */
    unsigned int data_loop = 0;
    unsigned int data_loop_count = _data_loop_count;
    unsigned int bytes_per_data_ecc = _bytes_per_data_ecc;
    char * data_buffer = _data_buffer;
    char * data = data_buffer;

    /* SPARE */
    unsigned int bytes_spare = _bytes_spare;
    char * spare_buffer = _spare_buffer;
    char * spare = spare_buffer;

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        /**********************************************************************
         * 1st : Send Read Command, Because Previous Status Read Command Issued
         **********************************************************************/
        NFC_PHY_Cmd(NF_CMD_READ_1ST);
        NFC_PHY_tDelay(NfcTime.tWHR);

        /**********************************************************************
         * 2nd : Read Data
         **********************************************************************/
        if (__NULL != data_buffer)
        {
            for (data_loop = 0; data_loop <= data_loop_count; data_loop++)
            {
                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_read(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }
            }
        }

        /**********************************************************************
         * 3rd : Read Spare
         **********************************************************************/
        if (__NULL != spare_buffer)
        {
            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
				spare = nfc_io_read(spare, (void *)&nfcShadowI->nfdata, bytes_spare);
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return 0;
}

int NFC_PHY_2ndRandomReadDataNoEcc(unsigned int _channel,
                                   unsigned int _way,
                                   unsigned int _col,
                                   /* DATA */
                                   unsigned int _bytes_to_read,
                                   void * _data_buffer)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int col = _col;

    unsigned int bytes_to_read = _bytes_to_read;
    char * data_buffer = _data_buffer;
    char * data = data_buffer;

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        // Send Read Command : Because Previous Status Read Command Issued
        NFC_PHY_Cmd(NF_CMD_READ_1ST);
        NFC_PHY_tDelay(NfcTime.tRHW);

        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
        NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
        NFC_PHY_tDelay(NfcTime.tCCS);

        // Read Data
        if (__NULL != data_buffer)
        {
            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
				data = nfc_io_read(data, (void *)&nfcShadowI->nfdata, bytes_to_read);
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return 0;
}

int NFC_PHY_2ndReadLog(unsigned int _channel,
                       unsigned int _way,
                       unsigned int _row,
                       unsigned int _col,
                       /* LOG */
                       unsigned int _log_loop_count,
                       unsigned int _bytes_per_log_ecc,
                       unsigned int _bytes_per_log_parity,
                       unsigned int _log_ecc_bits,
                       void * _log_buffer,
                       /* MAP DATA */
                       unsigned int _data_loop_count,
                       unsigned int _bytes_per_data_ecc,
                       unsigned int _bytes_per_data_parity,
                       unsigned int _data_ecc_bits,
                       void * _data_buffer,
                       /* etc */
                       unsigned int _retryable)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row = _row;
    unsigned int col = _col;
    unsigned int paritycol = 0;
    unsigned int retryable = _retryable;

    /* LOG */
    unsigned int log_loop = 0;
    unsigned int log_loop_count = _log_loop_count;
    unsigned int bytes_per_log_ecc = _bytes_per_log_ecc;
    unsigned int bytes_per_log_parity = _bytes_per_log_parity;
    unsigned int log_ecc_bits = _log_ecc_bits;
    char * log_buffer = (char *)_log_buffer;
    char * log = log_buffer;
    char * log_parity_buffer = (char *)__parity_buffer0;
  //char * log_fake_buffer = (char *)__fake_buffer0;
    char * log_error_location_buffer = (char *)__error_location_buffer0;

    /* MAP DATA */
    unsigned int data_loop = 0;
    unsigned int data_loop_count = _data_loop_count;
    unsigned int bytes_per_data_ecc = _bytes_per_data_ecc;
    unsigned int bytes_per_data_parity = _bytes_per_data_parity;
    unsigned int data_ecc_bits = _data_ecc_bits;
    char * data_buffer = (char *)_data_buffer;
    char * data = data_buffer;
    char * data_parity_buffer = (char *)__parity_buffer1;
  //char * data_fake_buffer = (char *)__fake_buffer1;
    char * data_error_location_buffer = (char *)__error_location_buffer1;

    /* MISC */
    char * parity = __NULL;
  //char * fake = __NULL;

    char * cur_read = __NULL;

    unsigned int ecc_error_detected = 0;
    unsigned int ecc_uncorrected = 0;
    unsigned int ecc_uncorrected_retry = 0;

    unsigned int report_ecc[6] = {0,};

    unsigned int * report_ecc_correct_bit = &(NfcEccStatus.correct_bit[way][channel]);
    unsigned int * report_ecc_correct_sector = &(NfcEccStatus.correct_sector[way][channel]);
    unsigned int * report_ecc_max_correct_bit = &(NfcEccStatus.max_correct_bit[way][channel]);
    unsigned int * report_ecc_level = Exchange.nfc.ecc.level0;
    unsigned int * report_ecc_level_error = &(NfcEccStatus.level_error[way][channel]);
    unsigned int * report_ecc_error = &(NfcEccStatus.error[way][channel]);

    if (!report_ecc_correct_bit) { report_ecc_correct_bit = &report_ecc[0]; }
    if (!report_ecc_correct_sector) { report_ecc_correct_sector = &report_ecc[1]; }
    if (!report_ecc_max_correct_bit) { report_ecc_max_correct_bit = &report_ecc[2]; }
    if (!report_ecc_level) { report_ecc_level = &report_ecc[3]; }
    if (!report_ecc_level_error) { report_ecc_level_error = &report_ecc[4]; }
    if (!report_ecc_error) { report_ecc_error = &report_ecc[5]; }

    /**************************************************************************
     *
     **************************************************************************/
    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row&0x000000FF,0);
        unsigned int _block  = __NROOT(_row&0x000FFF00,8);
        Exchange.sys.fn.print("Read.Log     : block(%04d), page(%04d) => row(%08d), column(%04d) ... Issue\n", _block, _page, _row, _column);
    }

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        // Send Read Command : Because Previous Status Read Command Issued
        NFC_PHY_Cmd(NF_CMD_READ_1ST);

        /**********************************************************************
         * LOG
         **********************************************************************/
        log = log_buffer;

        for (log_loop = 0; log_loop <= log_loop_count; log_loop++)
        {
            cur_read = log;
            ecc_uncorrected_retry = 0;

            do
            {
                ecc_error_detected = 0;
                ecc_uncorrected = 0;

                /**************************************************************
                 * Zero Initial
                 **************************************************************/
              //if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)log_parity_buffer, 0, bytes_per_log_parity);
              //                               Exchange.sys.fn._memset((void *)log_fake_buffer, 0, bytes_per_log_ecc);
              //                               Exchange.sys.fn._memset((void *)log_error_location_buffer, 0, log_ecc_bits*sizeof(unsigned int)); }
              //else                         {                  memset((void *)log_parity_buffer, 0, bytes_per_log_parity);
              //                                                memset((void *)log_fake_buffer, 0, bytes_per_log_ecc);
              //                                                memset((void *)log_error_location_buffer, 0, log_ecc_bits*sizeof(unsigned int)); }

                /**************************************************************
                 * Read Parity
                 **************************************************************/
                parity = log_parity_buffer;
                paritycol = col + bytes_per_log_ecc;

                NFC_PHY_tDelay(NfcTime.tRHW);
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                NFC_PHY_Addr(__NROOT(paritycol&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(paritycol&0x0000FF00,8));
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                NFC_PHY_tDelay(NfcTime.tCCS2);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_read(parity, (void *)&nfcShadowI->nfdata, bytes_per_log_parity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_RAND_Randomize((void*)log_parity_buffer, bytes_per_log_parity, 0);

                /**************************************************************
                 * Read Data
                 **************************************************************/
                NFC_PHY_tDelay(NfcTime.tRHW);
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                NFC_PHY_tDelay(NfcTime.tCCS);

                NFC_PHY_EccDecoderReset(bytes_per_log_ecc, log_ecc_bits);
                NFC_PHY_EccDecoderSetOrg((unsigned int *)log_parity_buffer, log_ecc_bits);
                NFC_PHY_EccDecoderEnable(bytes_per_log_ecc, log_ecc_bits);

                // Read Data From NAND Flash
                log = cur_read;

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					log = nfc_io_read(log, (void *)&nfcShadowI->nfdata, bytes_per_log_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                /**************************************************************
                 * ECC Correction
                 **************************************************************/
                NFC_PHY_EccDecoderWaitDone();
                ecc_error_detected = NFC_PHY_EccDecoderHasError();

                if (ecc_error_detected)
                {
                    ecc_uncorrected = NFC_PHY_EccCorrection("@ 2ndReadLog:Log",
                                                            row,
                                                            col,
                                                            cur_read,
                                                            bytes_per_log_ecc,
                                                            log_ecc_bits,
                                                            log_parity_buffer,
                                                            bytes_per_log_parity,
                                                            log_error_location_buffer,
                                                            report_ecc_correct_bit,
                                                            report_ecc_correct_sector,
                                                            report_ecc_max_correct_bit,
                                                            report_ecc_level,
                                                            report_ecc_level_error,
                                                            report_ecc_error);
                }

                if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show)
                {
                    if ((ecc_error_detected && (ecc_uncorrected || ecc_uncorrected_retry)) || (!ecc_error_detected && ecc_uncorrected_retry))
                    {
                        NFC_PHY_ShowUncorrectedData(ecc_uncorrected_retry, row, col, cur_read, bytes_per_log_ecc, log_ecc_bits, log_parity_buffer, bytes_per_log_parity);
                    }
                }

                ecc_uncorrected_retry += ecc_uncorrected;

            } while (ecc_error_detected && ecc_uncorrected && (ecc_uncorrected_retry < 2));

            if (ecc_uncorrected)
            {
                *report_ecc_error += 1;

                if (!retryable && Exchange.debug.nfc.phy.err_ecc_uncorrectable)
                {
                    Exchange.sys.fn.print("@ 2ndReadLog:Log:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Un-Corrected\n", row, col, ecc_uncorrected_retry+1);
                }
            }
            else
            {
                if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable && ecc_uncorrected_retry)
                {
                    Exchange.sys.fn.print("@ 2ndReadLog:Log:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Corrected\n", row, col, ecc_uncorrected_retry+1);
                }
            }

            NFC_PHY_RAND_Randomize((void*)cur_read, bytes_per_log_ecc, 0);

            col += bytes_per_log_ecc + bytes_per_log_parity;
        }

        /**********************************************************************
         * MAP DATA
         **********************************************************************/
        data = data_buffer;

        for (data_loop = 0; data_loop <= data_loop_count; data_loop++)
        {
            cur_read = data;
            ecc_uncorrected_retry = 0;

            do
            {
                ecc_error_detected = 0;
                ecc_uncorrected = 0;

                /**************************************************************
                 * Zero Initial
                 **************************************************************/
              //if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
              //                               Exchange.sys.fn._memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
              //                               Exchange.sys.fn._memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }
              //else                         {                  memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
              //                                                memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
              //                                                memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }

                /**************************************************************
                 * Read Parity
                 **************************************************************/
                parity = data_parity_buffer;
                paritycol = col + bytes_per_data_ecc;

                NFC_PHY_tDelay(NfcTime.tRHW);
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                NFC_PHY_Addr(__NROOT(paritycol&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(paritycol&0x0000FF00,8));
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                NFC_PHY_tDelay(NfcTime.tCCS2);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_read(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_RAND_Randomize((void*)data_parity_buffer, bytes_per_data_parity, 0);

                /**************************************************************
                 * Read Data
                 **************************************************************/
                NFC_PHY_tDelay(NfcTime.tRHW);
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                NFC_PHY_tDelay(NfcTime.tCCS);

                NFC_PHY_EccDecoderReset(bytes_per_data_ecc, data_ecc_bits);
                NFC_PHY_EccDecoderSetOrg((unsigned int *)data_parity_buffer, data_ecc_bits);
                NFC_PHY_EccDecoderEnable(bytes_per_data_ecc, data_ecc_bits);

                // Read Data From NAND Flash
                data = cur_read;

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_read(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                /**************************************************************
                 * ECC Correction
                 **************************************************************/
                NFC_PHY_EccDecoderWaitDone();
                ecc_error_detected = NFC_PHY_EccDecoderHasError();

                if (ecc_error_detected)
                {
                    ecc_uncorrected = NFC_PHY_EccCorrection("@ 2ndReadLog:Map",
                                                            row,
                                                            col,
                                                            cur_read,
                                                            bytes_per_data_ecc,
                                                            data_ecc_bits,
                                                            data_parity_buffer,
                                                            bytes_per_data_parity,
                                                            data_error_location_buffer,
                                                            report_ecc_correct_bit,
                                                            report_ecc_correct_sector,
                                                            report_ecc_max_correct_bit,
                                                            report_ecc_level,
                                                            report_ecc_level_error,
                                                            report_ecc_error);
                }

                if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show)
                {
                    if ((ecc_error_detected && (ecc_uncorrected || ecc_uncorrected_retry)) || (!ecc_error_detected && ecc_uncorrected_retry))
                    {
                        NFC_PHY_ShowUncorrectedData(ecc_uncorrected_retry, row, col, cur_read, bytes_per_data_ecc, data_ecc_bits, data_parity_buffer, bytes_per_data_parity);
                    }
                }

                ecc_uncorrected_retry += ecc_uncorrected;

            } while (ecc_error_detected && ecc_uncorrected && (ecc_uncorrected_retry < 2));

            if (ecc_uncorrected)
            {
                *report_ecc_error += 1;

                if (!retryable && Exchange.debug.nfc.phy.err_ecc_uncorrectable)
                {
                    Exchange.sys.fn.print("@ 2ndReadLog:Map:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Un-Corrected\n", row, col, ecc_uncorrected_retry+1);
                }
            }
            else
            {
                if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable && ecc_uncorrected_retry)
                {
                    Exchange.sys.fn.print("@ 2ndReadLog:Map:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Corrected\n", row, col, ecc_uncorrected_retry+1);
                }
            }

            NFC_PHY_RAND_Randomize((void*)cur_read, bytes_per_data_ecc, 0);

            col += bytes_per_data_ecc + bytes_per_data_parity;
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row&0x000000FF,0);
        unsigned int _block  = __NROOT(_row&0x000FFF00,8);
        Exchange.sys.fn.print("Read.Log     : block(%04d), page(%04d) => row(%08d), column(%04d) ... Done\n", _block, _page, _row, _column);
    }

    return 0;
}

int NFC_PHY_2ndReadData(unsigned int _stage,
                        unsigned int _channel,
                        unsigned int _way,
                        unsigned int _row,
                        unsigned int _col,
                        unsigned int _fake_spare_row,
                        unsigned int _fake_spare_col,
                        /* DATA */
                        unsigned int _data_loop_count,
                        unsigned int _bytes_per_data_ecc,
                        unsigned int _bytes_per_data_parity,
                        unsigned int _data_ecc_bits,
                        void * _data_buffer,
                        /* SPARE */
                        unsigned int _bytes_per_spare_ecc,
                        unsigned int _bytes_per_spare_parity,
                        unsigned int _spare_ecc_bits,
                        void * _spare_buffer,
                        unsigned int _retryable)
{
    unsigned int stage = _stage & 0xF;
    unsigned int indexed = __NROOT(_stage,31);
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row = _row;
    unsigned int col = _col;
    unsigned int retryable = _retryable;

  //unsigned int fake_spare_row = _fake_spare_row;
    unsigned int fake_spare_col = _fake_spare_col;
    unsigned int paritycol = 0;

    /* DATA */
    unsigned int data_loop = 0;
    unsigned int data_loop_count = _data_loop_count;
    unsigned int bytes_per_data_ecc = _bytes_per_data_ecc;
    unsigned int bytes_per_data_parity = _bytes_per_data_parity;
    unsigned int data_ecc_bits = _data_ecc_bits;
    char * data_buffer = (char *)_data_buffer;
    char * data = data_buffer;
    char * data_parity_buffer = (char *)__parity_buffer0;
    char * data_fake_buffer = (char *)__fake_buffer0;
    char * data_error_location_buffer = (char *)__error_location_buffer0;

    /* SPARE */
    unsigned int bytes_per_spare_eccunit = (_spare_ecc_bits <= 16)? 512: 1024;
    unsigned int bytes_per_spare_ecc = _bytes_per_spare_ecc;
    unsigned int bytes_per_spare_parity = _bytes_per_spare_parity;
    unsigned int spare_ecc_bits = _spare_ecc_bits;
    char * spare_buffer = (char *)_spare_buffer;
    char * spare = spare_buffer;
    char * spare_parity_buffer = (char *)__parity_buffer1;
    char * spare_fake_buffer = (char *)__fake_buffer1;
    char * spare_error_location_buffer = (char *)__error_location_buffer1;

    /* MISC */
    char * parity = __NULL;
    char * fake = __NULL;

    unsigned int read_loop = 0;
    char * cur_read = __NULL;

    unsigned int ecc_error_detected = 0;
    unsigned int ecc_uncorrected = 0;
    unsigned int ecc_uncorrected_retry = 0;

    unsigned int report_ecc[6] = {0, };

    unsigned int * report_ecc_correct_bit = &(NfcEccStatus.correct_bit[way][channel]);
    unsigned int * report_ecc_correct_sector = &(NfcEccStatus.correct_sector[way][channel]);
    unsigned int * report_ecc_max_correct_bit = &(NfcEccStatus.max_correct_bit[way][channel]);
    unsigned int * report_ecc_level = Exchange.nfc.ecc.level1;
    unsigned int * report_ecc_level_error = &(NfcEccStatus.level_error[way][channel]);
    unsigned int * report_ecc_error = &(NfcEccStatus.error[way][channel]);

    if (!report_ecc_correct_bit) { report_ecc_correct_bit = &report_ecc[0]; }
    if (!report_ecc_correct_sector) { report_ecc_correct_sector = &report_ecc[1]; }
    if (!report_ecc_max_correct_bit) { report_ecc_max_correct_bit = &report_ecc[2]; }
    if (!report_ecc_level) { report_ecc_level = &report_ecc[3]; }
    if (!report_ecc_level_error) { report_ecc_level_error = &report_ecc[4]; }
    if (!report_ecc_error) { report_ecc_error = &report_ecc[5]; }

    /**************************************************************************
     *
     **************************************************************************/
    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row&0x000000FF,0);
        unsigned int _block  = __NROOT(_row&0x000FFF00,8);

        Exchange.sys.fn.print("Read.Data.?  : bytes_per_data_ecc(%d), bytes_per_data_parity(%d), data_ecc_bits(%d)\n", bytes_per_data_ecc, bytes_per_data_parity, data_ecc_bits);
        Exchange.sys.fn.print("Read.Data%s  : block(%04d), page(%04d) => row(%08d), column(%04d) ... Issue\n", (8 == stage) ? ".s" : (indexed ? ".i" : "  "), _block, _page, _row, _column);
    }

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        // Send Read Command : Because Previous Status Read Command Issued
        NFC_PHY_Cmd(NF_CMD_READ_1ST);

        /**********************************************************************
         * 1st : read half last sector
         **********************************************************************/
        switch (stage)
        {
          //case 0: {} break;
          //case 2: {} break;
          //case 4: {} break;
          //case 6: {} break;
          //case 8: {} break; // read spare

            case 1:
            case 3:
            case 5:
            case 7:
            {
                ecc_uncorrected_retry = 0;

                do
                {
                    ecc_error_detected = 0;
                    ecc_uncorrected = 0;

                    /**********************************************************
                     * Zero Initial
                     **********************************************************/
                  //if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
                  //                               Exchange.sys.fn._memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
                  //                               Exchange.sys.fn._memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }
                  //else                         {                  memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
                  //                                                memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
                  //                                                memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }

                    /**********************************************************
                     * Read Parity
                     **********************************************************/
                    parity = data_parity_buffer;
                    paritycol = col + bytes_per_data_ecc;

                    NFC_PHY_tDelay(NfcTime.tRHW);
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                    NFC_PHY_Addr(__NROOT(paritycol&0x000000FF,0));
                    NFC_PHY_Addr(__NROOT(paritycol&0x0000FF00,8));
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                    NFC_PHY_tDelay(NfcTime.tCCS2);

                    if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                    {
						parity = nfc_io_read(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    }
                    if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                    NFC_PHY_RAND_Randomize((void*)data_parity_buffer, bytes_per_data_parity, 0);

                    /**********************************************************
                     * Read Data
                     **********************************************************/
                    NFC_PHY_tDelay(NfcTime.tRHW);
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                    NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                    NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                    NFC_PHY_tDelay(NfcTime.tCCS);

                    NFC_PHY_EccDecoderReset(bytes_per_data_ecc, data_ecc_bits);
                    NFC_PHY_EccDecoderSetOrg((unsigned int *)data_parity_buffer, data_ecc_bits);
                    NFC_PHY_EccDecoderEnable(bytes_per_data_ecc, data_ecc_bits);

                    // 1st 512 fake / 2nd 512 data
                    fake = data_fake_buffer;

                    if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                    {
						fake = nfc_io_read(fake, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                    }
                    if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                    /**********************************************************
                     * ECC Correction
                     **********************************************************/
                    NFC_PHY_EccDecoderWaitDone();
                    ecc_error_detected = NFC_PHY_EccDecoderHasError();

                    if (ecc_error_detected)
                    {
                        ecc_uncorrected = NFC_PHY_EccCorrection("@ 2ndReadData:Data.1st",
                                                                row,
                                                                col,
                                                                data_fake_buffer,
                                                                bytes_per_data_ecc,
                                                                data_ecc_bits,
                                                                data_parity_buffer,
                                                                bytes_per_data_parity,
                                                                data_error_location_buffer,
                                                                report_ecc_correct_bit,
                                                                report_ecc_correct_sector,
                                                                report_ecc_max_correct_bit,
                                                                report_ecc_level,
                                                                report_ecc_level_error,
                                                                report_ecc_error);
                    }

                    if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show)
                    {
                        if ((ecc_error_detected && (ecc_uncorrected || ecc_uncorrected_retry)) || (!ecc_error_detected && ecc_uncorrected_retry))
                        {
                            NFC_PHY_ShowUncorrectedData(ecc_uncorrected_retry, row, col, data_fake_buffer, bytes_per_data_ecc, data_ecc_bits, data_parity_buffer, bytes_per_data_parity);
                        }
                    }

                    ecc_uncorrected_retry += ecc_uncorrected;

                } while (ecc_error_detected && ecc_uncorrected && (ecc_uncorrected_retry < 2));

                if (ecc_uncorrected)
                {
                    *report_ecc_error += 1;

                    if (!retryable && Exchange.debug.nfc.phy.err_ecc_uncorrectable)
                    {
                        Exchange.sys.fn.print("@ 2ndReadData:Data.1st:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Un-Corrected\n", row, col, ecc_uncorrected_retry+1);
                    }
                }
                else
                {
                    if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable && ecc_uncorrected_retry)
                    {
                        Exchange.sys.fn.print("@ 2ndReadData:Data.1st:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Corrected\n", row, col, ecc_uncorrected_retry+1);
                    }
                }

                NFC_PHY_RAND_Randomize((void*)data_fake_buffer, bytes_per_data_ecc, 0);

                if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)data, (const void *)(data_fake_buffer + __NROOT(bytes_per_data_ecc,1)), __NROOT(bytes_per_data_ecc,1)); }
                else                         {                  memcpy((void *)data, (const void *)(data_fake_buffer + __NROOT(bytes_per_data_ecc,1)), __NROOT(bytes_per_data_ecc,1)); }
                data += __NROOT(bytes_per_data_ecc,1);
                col += bytes_per_data_ecc + bytes_per_data_parity;

            } break;
        }

        /**********************************************************************
         * 2nd : read loop
         **********************************************************************/
        for (data_loop = 0; data_loop <= data_loop_count; data_loop++)
        {
            // read single/dual sector (depends on bytes_per_data_ecc)
            switch (stage)
            {
              //case 1: {} break;
              //case 4: {} break;
              //case 5: {} break;
              //case 8: {} break; // read spare

                case 0:
                case 2:
                case 3:
                case 6:
                case 7:
                {
                    cur_read = data;
                    ecc_uncorrected_retry = 0;

                    do
                    {
                        ecc_error_detected = 0;
                        ecc_uncorrected = 0;

                        /******************************************************
                         * Zero Initial
                         ******************************************************/
                      //if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
                      //                               Exchange.sys.fn._memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
                      //                               Exchange.sys.fn._memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }
                      //else                         {                  memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
                      //                                                memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
                      //                                                memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }

                        /******************************************************
                         * Read Parity
                         ******************************************************/
                        parity = data_parity_buffer;
                        paritycol = col + bytes_per_data_ecc;

                        NFC_PHY_tDelay(NfcTime.tRHW);
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                        NFC_PHY_Addr(__NROOT(paritycol&0x000000FF,0));
                        NFC_PHY_Addr(__NROOT(paritycol&0x0000FF00,8));
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                        NFC_PHY_tDelay(NfcTime.tCCS2);

                        if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                        {
							parity = nfc_io_read(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                        }
                        if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                        NFC_PHY_RAND_Randomize((void*)data_parity_buffer, bytes_per_data_parity, 0);

                        /******************************************************
                         * Read Data
                         ******************************************************/
                        NFC_PHY_tDelay(NfcTime.tRHW);
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                        NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                        NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                        NFC_PHY_tDelay(NfcTime.tCCS);

                        NFC_PHY_EccDecoderReset(bytes_per_data_ecc, data_ecc_bits);
                        NFC_PHY_EccDecoderSetOrg((unsigned int *)data_parity_buffer, data_ecc_bits);
                        NFC_PHY_EccDecoderEnable(bytes_per_data_ecc, data_ecc_bits);

                        // Read Data From NAND Flash
                        data = cur_read;

                        if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
						{
							data = nfc_io_read(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                        }
                        if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                        /******************************************************
                         * ECC Correction
                         ******************************************************/
                        NFC_PHY_EccDecoderWaitDone();
                        ecc_error_detected = NFC_PHY_EccDecoderHasError();

                        if (ecc_error_detected)
                        {
                            ecc_uncorrected = NFC_PHY_EccCorrection("@ 2ndReadData:Data.2nd",
                                                                    row,
                                                                    col,
                                                                    cur_read,
                                                                    bytes_per_data_ecc,
                                                                    data_ecc_bits,
                                                                    data_parity_buffer,
                                                                    bytes_per_data_parity,
                                                                    data_error_location_buffer,
                                                                    report_ecc_correct_bit,
                                                                    report_ecc_correct_sector,
                                                                    report_ecc_max_correct_bit,
                                                                    report_ecc_level,
                                                                    report_ecc_level_error,
                                                                    report_ecc_error);
                        }

                        if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show)
                        {
                            if ((ecc_error_detected && (ecc_uncorrected || ecc_uncorrected_retry)) || (!ecc_error_detected && ecc_uncorrected_retry))
                            {
                                NFC_PHY_ShowUncorrectedData(ecc_uncorrected_retry, row, col, cur_read, bytes_per_data_ecc, data_ecc_bits, data_parity_buffer, bytes_per_data_parity);
                            }
                        }

                        ecc_uncorrected_retry += ecc_uncorrected;

                    } while (ecc_error_detected && ecc_uncorrected && (ecc_uncorrected_retry < 2));

                    if (ecc_uncorrected)
                    {
                        *report_ecc_error += 1;

                        if (!retryable && Exchange.debug.nfc.phy.err_ecc_uncorrectable)
                        {
                            Exchange.sys.fn.print("@ 2ndReadData:Data.2nd:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Un-Corrected\n", row, col, ecc_uncorrected_retry+1);
                        }
                    }
                    else
                    {
                        if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable && ecc_uncorrected_retry)
                        {
                            Exchange.sys.fn.print("@ 2ndReadData:Data.2nd:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Corrected\n", row, col, ecc_uncorrected_retry+1);
                        }
                    }

                    NFC_PHY_RAND_Randomize((void*)cur_read, bytes_per_data_ecc, 0);

                    col += bytes_per_data_ecc + bytes_per_data_parity;

                } break;
            }
        }

        /**********************************************************************
         * 3rd : read half first sector
         **********************************************************************/
        switch (stage)
        {
          //case 0: {} break;
          //case 1: {} break;
          //case 2: {} break;
          //case 3: {} break;
          //case 8: {} break; // read spare

            case 4:
            case 5:
            case 6:
            case 7:
            {
                ecc_uncorrected_retry = 0;

                do
                {
                    ecc_error_detected = 0;
                    ecc_uncorrected = 0;

                    /**********************************************************
                     * Zero Initial
                     **********************************************************/
                  //if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
                  //                               Exchange.sys.fn._memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
                  //                               Exchange.sys.fn._memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }
                  //else                         {                  memset((void *)data_parity_buffer, 0, bytes_per_data_parity);
                  //                                                memset((void *)data_fake_buffer, 0, bytes_per_data_ecc);
                  //                                                memset((void *)data_error_location_buffer, 0, data_ecc_bits*sizeof(unsigned int)); }

                    /**********************************************************
                     * Read Parity
                     **********************************************************/
                    parity = data_parity_buffer;
                    paritycol = col + bytes_per_data_ecc;

                    NFC_PHY_tDelay(NfcTime.tRHW);
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                    NFC_PHY_Addr(__NROOT(paritycol&0x000000FF,0));
                    NFC_PHY_Addr(__NROOT(paritycol&0x0000FF00,8));
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                    NFC_PHY_tDelay(NfcTime.tCCS2);

                    if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                    {
						parity = nfc_io_read(parity , (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    }
                    if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                    NFC_PHY_RAND_Randomize((void*)data_parity_buffer, bytes_per_data_parity, 0);

                    /**********************************************************
                     * Read Data
                     **********************************************************/
                    NFC_PHY_tDelay(NfcTime.tRHW);
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                    NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                    NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                    NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                    NFC_PHY_tDelay(NfcTime.tCCS);

                    NFC_PHY_EccDecoderReset(bytes_per_data_ecc, data_ecc_bits);
                    NFC_PHY_EccDecoderSetOrg((unsigned int *)data_parity_buffer, data_ecc_bits);
                    NFC_PHY_EccDecoderEnable(bytes_per_data_ecc, data_ecc_bits);

                    // 1st 512 data / 2nd 512 fake
                    fake = data_fake_buffer;

                    if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                    {
						fake = nfc_io_read(fake, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                    }
                    if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                    /**********************************************************
                     * ECC Correction
                     **********************************************************/
                    NFC_PHY_EccDecoderWaitDone();
                    ecc_error_detected = NFC_PHY_EccDecoderHasError();

                    if (ecc_error_detected)
                    {
                        ecc_uncorrected = NFC_PHY_EccCorrection("@ 2ndReadData:Data.3rd",
                                                                row,
                                                                col,
                                                                data_fake_buffer,
                                                                bytes_per_data_ecc,
                                                                data_ecc_bits,
                                                                data_parity_buffer,
                                                                bytes_per_data_parity,
                                                                data_error_location_buffer,
                                                                report_ecc_correct_bit,
                                                                report_ecc_correct_sector,
                                                                report_ecc_max_correct_bit,
                                                                report_ecc_level,
                                                                report_ecc_level_error,
                                                                report_ecc_error);
                    }

                    if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show)
                    {
                        if ((ecc_error_detected && (ecc_uncorrected || ecc_uncorrected_retry)) || (!ecc_error_detected && ecc_uncorrected_retry))
                        {
                            NFC_PHY_ShowUncorrectedData(ecc_uncorrected_retry, row, col, data_fake_buffer, bytes_per_data_ecc, data_ecc_bits, data_parity_buffer, bytes_per_data_parity);
                        }
                    }

                    ecc_uncorrected_retry += ecc_uncorrected;

                } while (ecc_error_detected && ecc_uncorrected && (ecc_uncorrected_retry < 2));

                if (ecc_uncorrected)
                {
                    *report_ecc_error += 1;

                    if (!retryable && Exchange.debug.nfc.phy.err_ecc_uncorrectable)
                    {
                        Exchange.sys.fn.print("@ 2ndReadData:Data.3rd:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Un-Corrected\n", row, col, ecc_uncorrected_retry+1);
                    }
                }
                else
                {
                    if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable && ecc_uncorrected_retry)
                    {
                        Exchange.sys.fn.print("@ 2ndReadData:Data.3rd:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Corrected\n", row, col, ecc_uncorrected_retry+1);
                    }
                }

                NFC_PHY_RAND_Randomize((void*)data_fake_buffer, bytes_per_data_ecc, 0);

                if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)data, (const void *)data_fake_buffer, __NROOT(bytes_per_data_ecc,1)); }
                else                         {                  memcpy((void *)data, (const void *)data_fake_buffer, __NROOT(bytes_per_data_ecc,1)); }
                data += __NROOT(bytes_per_data_ecc,1);
                col += bytes_per_data_ecc + bytes_per_data_parity;

            } break;
        }

        /**********************************************************************
         * 4th : read spare
         **********************************************************************/
        if (spare_buffer)
        {
            switch (stage)
            {
              //case 1: {} break;
              //case 2: {} break;
              //case 3: {} break;
              //case 4: {} break;
              //case 5: {} break;
              //case 6: {} break;
              //case 7: {} break;

                case 0:
                case 8: /* read spare */
                {
                    ecc_uncorrected_retry = 0;

                    do
                    {
                        ecc_error_detected = 0;
                        ecc_uncorrected = 0;

                        /******************************************************
                         * Zero Initial
                         ******************************************************/
                      //if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)spare_parity_buffer, 0, bytes_per_spare_parity);
                      //                               Exchange.sys.fn._memset((void *)spare_fake_buffer, 0, bytes_per_spare_ecc);
                      //                               Exchange.sys.fn._memset((void *)spare_error_location_buffer, 0, spare_ecc_bits*sizeof(unsigned int)); }
                      //else                         {                  memset((void *)spare_parity_buffer, 0, bytes_per_spare_parity);
                      //                                                memset((void *)spare_fake_buffer, 0, bytes_per_spare_ecc);
                      //                                                memset((void *)spare_error_location_buffer, 0, spare_ecc_bits*sizeof(unsigned int)); }

                        /******************************************************
                         * Read Parity
                         ******************************************************/
                        parity = spare_parity_buffer;
                        paritycol = col + bytes_per_spare_ecc;

                        NFC_PHY_tDelay(NfcTime.tRHW);
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                        NFC_PHY_Addr(__NROOT(paritycol&0x000000FF,0));
                        NFC_PHY_Addr(__NROOT(paritycol&0x0000FF00,8));
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                        NFC_PHY_tDelay(NfcTime.tCCS2);

                        if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                        {
							parity = nfc_io_read(parity , (void *)&nfcShadowI->nfdata, bytes_per_spare_parity);
                        }
                        if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

#if defined (__KTW_SUPPORT_SPARE_RAMDOMIZE__)
                        NFC_PHY_RAND_Randomize((void*)spare_parity_buffer, bytes_per_spare_parity, 0);
#endif
                        /******************************************************
                         * Read Data
                         ******************************************************/
                        NFC_PHY_tDelay(NfcTime.tRHW);
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                        NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
                        NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
                        NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                        NFC_PHY_tDelay(NfcTime.tCCS2);

                        NFC_PHY_EccDecoderReset(bytes_per_spare_eccunit, spare_ecc_bits);
                        NFC_PHY_EccDecoderSetOrg((unsigned int *)spare_parity_buffer, spare_ecc_bits);
                        NFC_PHY_EccDecoderEnable(bytes_per_spare_eccunit, spare_ecc_bits);

                        // 1st 512 data / 2nd 512 fake
                        fake = spare_fake_buffer;

                        if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                        {
							fake = nfc_io_read(fake, (void *)&nfcShadowI->nfdata, bytes_per_spare_ecc);
                        }
                        if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                        for (read_loop = 0; read_loop < (bytes_per_spare_eccunit - bytes_per_spare_ecc) / 4; read_loop++)
                        {
                            NFC_PHY_tDelay(NfcTime.tRHW);
                            NFC_PHY_Cmd(NF_CMD_READ_RANDOM_1ST);
                            NFC_PHY_Addr(__NROOT(fake_spare_col&0x000000FF,0));
                            NFC_PHY_Addr(__NROOT(fake_spare_col&0x0000FF00,8));
                            NFC_PHY_Cmd(NF_CMD_READ_RANDOM_2ND);
                            NFC_PHY_tDelay(NfcTime.tCCS2);

#if (IOR_WIDTH == IO_BURST_X4 || IOR_WIDTH == IO_BURST_X1)
                            *((unsigned int *)fake) = nfcShadowI32->nfdata; fake+=4;
#else /* (IOR_WIDTH == IO_BURST_X0) */
                            *fake++ = nfcShadowI->nfdata;
                            *fake++ = nfcShadowI->nfdata;
                            *fake++ = nfcShadowI->nfdata;
                            *fake++ = nfcShadowI->nfdata;
#endif
                        }

                        /******************************************************
                         * ECC Correction
                         ******************************************************/
                        NFC_PHY_EccDecoderWaitDone();
                        ecc_error_detected = NFC_PHY_EccDecoderHasError();

                        if (ecc_error_detected)
                        {
                            ecc_uncorrected = NFC_PHY_EccCorrection("@ 2ndReadData:Spare",
                                                                    row,
                                                                    col,
                                                                    spare_fake_buffer,
                                                                    bytes_per_spare_ecc,
                                                                    spare_ecc_bits,
                                                                    spare_parity_buffer,
                                                                    bytes_per_spare_parity,
                                                                    spare_error_location_buffer,
                                                                    report_ecc_correct_bit,
                                                                    report_ecc_correct_sector,
                                                                    report_ecc_max_correct_bit,
                                                                    report_ecc_level,
                                                                    report_ecc_level_error,
                                                                    report_ecc_error);
                        }

                        if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show)
                        {
                            if ((ecc_error_detected && (ecc_uncorrected || ecc_uncorrected_retry)) || (!ecc_error_detected && ecc_uncorrected_retry))
                            {
                                NFC_PHY_ShowUncorrectedData(ecc_uncorrected_retry, row, col, spare_fake_buffer, bytes_per_spare_ecc, spare_ecc_bits, spare_parity_buffer, bytes_per_spare_parity);
                            }
                        }

                        ecc_uncorrected_retry += ecc_uncorrected;

                    } while (ecc_error_detected && ecc_uncorrected && (ecc_uncorrected_retry < 2));

                    if (ecc_uncorrected)
                    {
                        *report_ecc_error += 1;

                        if (!retryable && Exchange.debug.nfc.phy.err_ecc_uncorrectable)
                        {
                            Exchange.sys.fn.print("@ 2ndReadData:Spare:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Un-Corrected\n", row, col, ecc_uncorrected_retry+1);
                        }
                    }
                    else
                    {
                        if (!retryable && Exchange.debug.nfc.phy.warn_ecc_uncorrectable && ecc_uncorrected_retry)
                        {
                            Exchange.sys.fn.print("@ 2ndReadData:Spare:row(%04d),col(%04d) - Un-Corrected Retry #%d Times => Finally Corrected\n", row, col, ecc_uncorrected_retry+1);
                        }
                    }

                    if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)spare, (const void *)spare_fake_buffer, bytes_per_spare_ecc); }
                    else                         {                  memcpy((void *)spare, (const void *)spare_fake_buffer, bytes_per_spare_ecc); }
                    spare += bytes_per_spare_ecc;
                    col += bytes_per_spare_ecc + bytes_per_spare_parity;

                } break;
            }
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row&0x000000FF,0);
        unsigned int _block  = __NROOT(_row&0x000FFF00,8);
        Exchange.sys.fn.print("Read.Data%s  : block(%04d), page(%04d) => row(%08d), column(%04d) ... Done\n", (8 == stage) ? ".s" : (indexed ? ".i" : "  "), _block, _page, _row, _column);
    }

    return 0;
}

int NFC_PHY_3rdRead(unsigned int _channel, unsigned int _way) { return 0; }

/******************************************************************************
 *
 ******************************************************************************/
int NFC_PHY_1stWriteLog(unsigned int _channel,
                        unsigned int _way,
                        unsigned int _row0,
                        unsigned int _row1,
                        unsigned int _col,
                        unsigned int _multi_plane_write_cmd,
                        unsigned int _cache_write_cmd,
                        /* LOG */
                        unsigned int _log_loop_count,
                        unsigned int _bytes_per_log_ecc,
                        unsigned int _bytes_per_log_parity,
                        unsigned int _log_ecc_bits,
                        void * _log_buffer,
                        /* MAP DATA */
                        unsigned int _data_loop_count0,
                        unsigned int _data_loop_count1,
                        unsigned int _bytes_per_data_ecc,
                        unsigned int _bytes_per_data_parity,
                        unsigned int _data_ecc_bits,
                        void * _data_buffer0,
                        void * _data_buffer1)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row0 = _row0;
    unsigned int row1 = _row1;
    unsigned int col0 = _col;
    unsigned int col1 = _col;
    unsigned int block0 = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);
    unsigned int block1 = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

  //unsigned int multi_plane_write_support = __NROOT(_multi_plane_write_cmd&0xFFFF0000,16);
    unsigned int multi_plane_write_type = __NROOT(_multi_plane_write_cmd&0x0000FFFF,0);
  //unsigned int cache_write_support = __NROOT(_cache_write_cmd&0xFFFF0000,16);
  //unsigned int cache_write_type = __NROOT(_cache_write_cmd&0x0000FFFF,0);

    /* LOG */
    unsigned int log_loop = 0;
    unsigned int log_loop_count = _log_loop_count;
    unsigned int bytes_per_log_ecc = _bytes_per_log_ecc;
    unsigned int bytes_per_log_parity = _bytes_per_log_parity;
    unsigned int log_ecc_bits = _log_ecc_bits;
    char * log_buffer = _log_buffer;
    char * log = log_buffer;
    char * log_parity_buffer = (char *)__parity_buffer0;

    /* MAP DATA */
    unsigned int data_loop = 0;
    unsigned int data_loop_count0 = _data_loop_count0;
    unsigned int data_loop_count1 = _data_loop_count1;
    unsigned int bytes_per_data_ecc = _bytes_per_data_ecc;
    unsigned int bytes_per_data_parity = _bytes_per_data_parity;
    unsigned int data_ecc_bits = _data_ecc_bits;
    char * data_buffer0 = _data_buffer0;
    char * data_buffer1 = _data_buffer1;
    char * data = data_buffer0;
    char * data_parity_buffer = (char *)__parity_buffer1;

    /* MISC */
    char * parity = __NULL;

#if defined (__COMPILE_MODE_READONLY__)
    Exchange.sys.fn.print("EWS.NFC: WriteSkip: Log\n");
    return 0;
#endif

    /**************************************************************************
     *
     **************************************************************************/
    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row0&0x000000FF,0);
        unsigned int _block  = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);

        Exchange.sys.fn.print("Write.Log.0  : block(%04d), page(%04d) => row(%08d), column(%04d) .", _block, _page, _row0, _column);

        if ((int)_row1 >= 0)
        {
            _column = _col;
            _page   = __NROOT(_row1&0x000000FF,0);
            _block  = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

            Exchange.sys.fn.print("\n");
            Exchange.sys.fn.print(".. Write.Log.1  : block(%04d), page(%04d) => row(%08d), column(%04d) .", _block, _page, _row1, _column);
        }
    }

    /**************************************************************************
     * FTL Side : Violate Prohibited Block Access, But Allowed
     **************************************************************************/
    if (((0 <= block0) && (block0 < Exchange.ewsftl_start_block)) || ((0 <= block1) && (block1 < Exchange.ewsftl_start_block)))
    {
        if (Exchange.debug.nfc.phy.warn_prohibited_block_access)
        {
            Exchange.sys.fn.print("EWS.NFC: Warnning: Write Requested \"Block %d\" or \"Block %d\" Prohibited, But Allowed !!!\n", block0, block1);
        }
    }

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        /**********************************************************************
         * Single Plane Write
         *
         *  CMD80 - ADDR - DATA LOADING - CMD10
         *
         **********************************************************************/
        if (0xFFFFFFFF == row1)
        {
            /******************************************************************
             * 1st : set program
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_1ST);
            NFC_PHY_Addr(__NROOT(col0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(col0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
            NFC_PHY_tDelay(NfcTime.tADL);

            /******************************************************************
             * 2nd : write log
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_log_ecc, log_ecc_bits);

            for (log_loop = 0; log_loop <= log_loop_count; log_loop++)
            {
                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                log = log_buffer + (log_loop * bytes_per_log_ecc);
                log = (char *)NFC_PHY_RAND_Randomize((void *)log, bytes_per_log_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					log = nfc_io_write(log, (void *)&nfcShadowI->nfdata, bytes_per_log_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)log_parity_buffer, log_ecc_bits);

                // Program Parity
                parity = log_parity_buffer;
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_log_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_log_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                col0 += bytes_per_log_ecc + bytes_per_log_parity;
            }

            /******************************************************************
             * 3rd : write data (map)
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_data_ecc, data_ecc_bits);

            data = data_buffer0;

            for (data_loop = 0; data_loop <= data_loop_count0; data_loop++)
            {
                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                data = data_buffer0 + (data_loop * bytes_per_data_ecc);
                data = (char *)NFC_PHY_RAND_Randomize((void *)data, bytes_per_data_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)data_parity_buffer, data_ecc_bits);

                // Program Parity
                parity = data_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_data_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                col0 += bytes_per_data_ecc + bytes_per_data_parity;
            }

            /******************************************************************
             * 4th : program
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_2ND);
            NFC_PHY_tDelay(NfcTime.tWB);
        }
        /**********************************************************************
         * Multi Plane Write
         *
         *  CMD80 - ADDR - DATA LOADING - CMD11 - CMD80 - ADDR - DATA LOADING - CMD10 or
         *  CMD80 - ADDR - DATA LOADING - CMD11 - CMD81 - ADDR - DATA LOADING - CMD10
         *
         **********************************************************************/
        else
        {
            /******************************************************************
             * Plane 0 : 1st : set program
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_80);
            NFC_PHY_Addr(__NROOT(col0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(col0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
            NFC_PHY_tDelay(NfcTime.tADL);

            /******************************************************************
             * Plane 0 : 2nd : write log
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_log_ecc, log_ecc_bits);

            for (log_loop = 0; log_loop <= log_loop_count; log_loop++)
            {
                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                log = log_buffer + (log_loop * bytes_per_log_ecc);
                log = (char *)NFC_PHY_RAND_Randomize((void *)log, bytes_per_log_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					log = nfc_io_write(log, (void *)&nfcShadowI->nfdata, bytes_per_log_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)log_parity_buffer, log_ecc_bits);

                // Program Parity
                parity = log_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_log_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_log_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                col0 += bytes_per_log_ecc + bytes_per_log_parity;
            }

            /******************************************************************
             * Plane 0 : 3rd : write data (map)
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_data_ecc, data_ecc_bits);

            data = data_buffer0;

            for (data_loop = 0; data_loop <= data_loop_count0; data_loop++)
            {
                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                data = data_buffer0 + (data_loop * bytes_per_data_ecc);
                data = (char *)NFC_PHY_RAND_Randomize((void *)data, bytes_per_data_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)data_parity_buffer, data_ecc_bits);

                // Program Parity
                parity = data_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_data_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                col0 += bytes_per_data_ecc + bytes_per_data_parity;
            }

            /******************************************************************
             * Plane 0 : 4th : program & wait ready
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_PLANE_2ND);
            NFC_PHY_tDelay(NfcTime.tWB);

            // Check BUSY : Status Read
            NFC_PHY_Cmd(NF_CMD_READ_STATUS);
            NFC_PHY_tDelay(NfcTime.tWHR);
            while (!NFC_PHY_StatusIsRDY(nfcShadowI->nfdata));

            /******************************************************************
             * Plane 1 : 1st : set program
             ******************************************************************/
            switch (multi_plane_write_type)
            {
                case 1:  { NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_81); } break;
                case 2:  { NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_80); } break;
                default: { NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_81); } break;
            }
            NFC_PHY_Addr(__NROOT(col1&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(col1&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row1&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row1&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row1&0x00FF0000,16));
            NFC_PHY_tDelay(NfcTime.tADL);

            /******************************************************************
             * Plane 1 : 2nd : write data (map)
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_data_ecc, data_ecc_bits);

            data = data_buffer1;

            for (data_loop = 0; data_loop <= data_loop_count1; data_loop++)
            {
                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row1&0x000000FF);
                data = data_buffer1 + (data_loop * bytes_per_data_ecc);
                data = (char *)NFC_PHY_RAND_Randomize((void *)data, bytes_per_data_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)data_parity_buffer, data_ecc_bits);

                // Program Parity
                parity = data_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row1&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_data_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                col1 += bytes_per_data_ecc + bytes_per_data_parity;
            }

            /******************************************************************
             * Plane 1 : 3rd : program
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_2ND);
            NFC_PHY_tDelay(NfcTime.tWB);
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print(".. Issue ."); }

    return 0;
}

int NFC_PHY_1stWriteRoot(unsigned int _channel,
                         unsigned int _way,
                         unsigned int _row0,
                         unsigned int _row1,
                         unsigned int _col,
                         /* ROOT */
                         unsigned int _root_loop_unit,
                         unsigned int _bytes_per_root_ecc,
                         unsigned int _bytes_per_root_parity,
                         unsigned int _root_ecc_bits,
                         void * _root_buffer)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row0 = _row0;
  //unsigned int row1 = _row1;
    unsigned int col = _col;
    unsigned int block0 = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);

    /* ROOT */
    unsigned int root_loop = 0;
    unsigned int root_loop_unit = _root_loop_unit;
    unsigned int bytes_per_root_ecc = _bytes_per_root_ecc;
    unsigned int bytes_per_root_parity = _bytes_per_root_parity;
    unsigned int root_ecc_bits = _root_ecc_bits;
    char * root_buffer = (char *)_root_buffer;
    char * root = root_buffer;
    char * root_parity_buffer = (char *)__parity_buffer0;

    /* MISC */
    char * parity = __NULL;

#if defined (__COMPILE_MODE_READONLY__)
    Exchange.sys.fn.print("EWS.NFC: WriteSkip: Root\n");
    return 0;
#endif

    /**************************************************************************
     *
     **************************************************************************/
    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row0&0x000000FF,0);
        unsigned int _block  = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);

        Exchange.sys.fn.print("Write.Root.? : bytes_per_data_ecc(%d), bytes_per_data_parity(%d), data_ecc_bits(%d)\n", bytes_per_root_ecc, _bytes_per_root_parity, root_ecc_bits);
        Exchange.sys.fn.print("Write.Root.0 : block(%04d), page(%04d) => row(%08d), column(%04d) .", _block, _page, _row0, _column);

        if ((int)_row1 >= 0)
        {
            _column = _col;
            _page   = __NROOT(_row1&0x000000FF,0);
            _block  = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

            Exchange.sys.fn.print("\n");
            Exchange.sys.fn.print(".. Write.Root.1 : block(%04d), page(%04d) => row(%08d), column(%04d) .", _block, _page, _row1, _column);
        }
    }

    /**************************************************************************
     * FTL Side : Violate Prohibited Block Access, But Allowed
     **************************************************************************/
    if ((0 <= block0) && (block0 < Exchange.ewsftl_start_block))
    {
        if (Exchange.debug.nfc.phy.warn_prohibited_block_access)
        {
            Exchange.sys.fn.print("EWS.NFC: Warnning: Write Requested \"Block %d\" Prohibited, But Allowed !!!\n", block0);
        }
    }

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        /**********************************************************************
         * 1st : set program
         **********************************************************************/
        NFC_PHY_Cmd(NF_CMD_PROG_1ST);
        NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
        NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
        NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
        NFC_PHY_tDelay(NfcTime.tADL);

        /**********************************************************************
         * 2nd : write root
         **********************************************************************/
        NFC_PHY_EccEncoderSetup(bytes_per_root_ecc, root_ecc_bits);

        for (root_loop = 0; root_loop < 2; root_loop += root_loop_unit)
        {
            NFC_PHY_EccEncoderEnable();

            // Program Data
            NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
            root = root_buffer + (root_loop * bytes_per_root_ecc);
            root = (char *)NFC_PHY_RAND_Randomize((void *)root, bytes_per_root_ecc, 1);

            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
				root = nfc_io_write(root, (void *)&nfcShadowI->nfdata, bytes_per_root_ecc);
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

            NFC_PHY_EccEncoderWaitDone();
            NFC_PHY_EccEncoderReadParity((unsigned int *)root_parity_buffer, root_ecc_bits);

            // Program Parity
            parity = root_parity_buffer;

            NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
            parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_root_parity, 1);

            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
				parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_root_parity);
                NFC_PHY_tDelay(NfcTime.tParity);
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

            col += bytes_per_root_ecc + bytes_per_root_parity;
        }

        /**********************************************************************
         * 3rd : program
         **********************************************************************/
        NFC_PHY_Cmd(NF_CMD_PROG_2ND); // CMD80 - CMD10
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print(".. Issue ."); }

    return 0;
}

int NFC_PHY_1stWriteData(unsigned int _channel,
                         unsigned int _way,
                         unsigned int _row0,
                         unsigned int _row1,
                         unsigned int _col,
                         unsigned int _multi_plane_write_cmd,
                         unsigned int _cache_write_cmd,
                         /* DATA */
                         unsigned int _data_loop_count0,
                         unsigned int _data_loop_count1,
                         unsigned int _bytes_per_data_ecc,
                         unsigned int _bytes_per_data_parity,
                         unsigned int _data_ecc_bits,
                         void * _data_buffer0,
                         void * _data_buffer1,
                         /* SPARE */
                         unsigned int _bytes_per_spare_ecc,
                         unsigned int _bytes_per_spare_parity,
                         unsigned int _spare_ecc_bits,
                         void * _spare_buffer)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row0 = _row0;
    unsigned int row1 = _row1;
    unsigned int col0 = _col;
    unsigned int col1 = _col;
    unsigned int block0 = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);
    unsigned int block1 = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

  //unsigned int multi_plane_write_support = __NROOT(_multi_plane_write_cmd&0xFFFF0000,16);
    unsigned int multi_plane_write_type = __NROOT(_multi_plane_write_cmd&0x0000FFFF,0);
    unsigned int cache_write_support = __NROOT(_cache_write_cmd&0xFFFF0000,16);
    unsigned int cache_write_type = __NROOT(_cache_write_cmd&0x0000FFFF,0);

    /* DATA */
    unsigned int data_loop = 0;
    unsigned int data_loop_count0 = _data_loop_count0;
    unsigned int data_loop_count1 = _data_loop_count1;
    unsigned int bytes_per_data_ecc = _bytes_per_data_ecc;
    unsigned int bytes_per_data_parity = _bytes_per_data_parity;
    unsigned int data_ecc_bits = _data_ecc_bits;
    char * data_buffer0 = (char *)_data_buffer0;
    char * data_buffer1 = (char *)_data_buffer1;
    char * data = data_buffer0;
    char * data_parity_buffer = (char *)__parity_buffer0;

    /* SPARE */
    unsigned int bytes_per_spare_eccunit = (_spare_ecc_bits <= 16)? 512: 1024;
    unsigned int bytes_per_spare_ecc = _bytes_per_spare_ecc;
    unsigned int bytes_per_spare_parity = _bytes_per_spare_parity;
    unsigned int spare_ecc_bits = _spare_ecc_bits;
    char * spare_buffer = (char *)_spare_buffer;
    char * spare = spare_buffer;
    char * spare_parity_buffer = (char *)__parity_buffer1;
    char * dummy_buffer = (char *)__dummy_buffer;

    /* MISC */
    char * parity = __NULL;

    char * cur_write = __NULL;

#if defined (__COMPILE_MODE_READONLY__)
    Exchange.sys.fn.print("EWS.NFC: WriteSkip: Data\n");
    return 0;
#endif
 
    /**************************************************************************
     *
     **************************************************************************/
    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _column = _col;
        unsigned int _page   = __NROOT(_row0&0x000000FF,0);
        unsigned int _block  = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);

        Exchange.sys.fn.print("Write.Data.? : bytes_per_data_ecc(%d), bytes_per_data_parity(%d), data_ecc_bits(%d)\n", bytes_per_data_ecc, bytes_per_data_parity, data_ecc_bits);
        Exchange.sys.fn.print("Write.Data.0 : block(%04d), page(%04d) => row(%08d), column(%04d) .", _block, _page, _row0, _column);

        if (0xFFFFFFFF != _row1)
        {
            _column = _col;
            _page   = __NROOT(_row1&0x000000FF,0);
            _block  = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

            Exchange.sys.fn.print("\n");
            Exchange.sys.fn.print(".. Write.Data.1 : block(%04d), page(%04d) => row(%08d), column(%04d) .", _block, _page, _row1, _column);
        }
    }

    /**************************************************************************
     * FTL Side : Violate Prohibited Block Access, But Allowed
     **************************************************************************/
    if (((0 <= block0) && (block0 < Exchange.ewsftl_start_block)) || ((0 <= block1) && (block1 < Exchange.ewsftl_start_block)))
    {
        if (Exchange.debug.nfc.phy.warn_prohibited_block_access)
        {
            Exchange.sys.fn.print("EWS.NFC: Warnning: Write Requested \"Block %d\" or \"Block %d\" Prohibited, But Allowed !!!\n", block0, block1);
        }
    }

    /**************************************************************************
     * Calc Parity With Dummy Write For Spare Information
     **************************************************************************/
    if (spare_buffer)
    {
        NFC_PHY_EccEncoderSetup(bytes_per_spare_eccunit, spare_ecc_bits);

        spare = spare_buffer;
        cur_write = spare;

        NFC_PHY_EccEncoderEnable();

        // Program Spare
        if (Exchange.sys.fn._memset) { Exchange.sys.fn._memset((void *)dummy_buffer, 0xff, bytes_per_spare_eccunit); }
        else                         {                  memset((void *)dummy_buffer, 0xff, bytes_per_spare_eccunit); }
        if (Exchange.sys.fn._memcpy) { Exchange.sys.fn._memcpy((void *)dummy_buffer, (const void *)spare, bytes_per_spare_ecc); }
        else                         {                  memcpy((void *)dummy_buffer, (const void *)spare, bytes_per_spare_ecc); }

        if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
        {
			dummy_buffer = nfc_io_write(dummy_buffer, (void *)&nfcShadowI->nfdata, bytes_per_spare_eccunit);
        }
        if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

        NFC_PHY_EccEncoderWaitDone();
        NFC_PHY_EccEncoderReadParity((unsigned int *)spare_parity_buffer, spare_ecc_bits);

        NFC_PHY_tDelay(NfcTime.tParity);

        if (Exchange.debug.nfc.phy.info_ecc)
        {
            unsigned int line = 0;
            unsigned int print_size = 0;

            Exchange.sys.fn.print("NFC_PHY_1stWriteData: calc spare parity\n");

            do
            {
                for (line = 0; line < 16; line++)
                {
                    Exchange.sys.fn.print("%02x ", spare_parity_buffer[print_size++]);
                    if (print_size >= bytes_per_spare_parity) { break; }
                }

                Exchange.sys.fn.print("\n");
                if (print_size >= bytes_per_spare_parity) { break; }

            } while (1);
        }
    }

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        /**********************************************************************
         * Single Plane Write
         *
         *  CMD80 - ADDR - DATA LOADING - CMD10
         *
         **********************************************************************/
        if (0xFFFFFFFF == row1)
        {
            /******************************************************************
             * 1st : set program
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_1ST);
            NFC_PHY_Addr(__NROOT(col0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(col0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
            NFC_PHY_tDelay(NfcTime.tADL);

            /******************************************************************
             * 2nd : write data
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_data_ecc, data_ecc_bits);

            data = data_buffer0;

            for (data_loop = 0; data_loop <= data_loop_count0; data_loop++)
            {
                cur_write = data;

                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);

                data = data_buffer0 + (data_loop * bytes_per_data_ecc);
                data = (char *)NFC_PHY_RAND_Randomize((void *)data, bytes_per_data_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
#if (IOR_WIDTH == IO_BURST_X4)
					iow_burst(data, (void __iomem *)&nfcShadowI->nfdata, bytes_per_data_ecc);
					data += bytes_per_data_ecc;
#else
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
#endif
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                if (Exchange.debug.nfc.phy.info_ecc)
                {
                    unsigned int i = 0;

                    Exchange.sys.fn.print("NFC_PHY_1stWriteData: Single Plane: Data    : row(%05d), col(%05d)\n", row0, col0);

                    for (i = 0; i < 32; i++)
                    {
                        Exchange.sys.fn.print("%02x ", cur_write[i]);
                    }   Exchange.sys.fn.print("\n");
                }

                col0 += bytes_per_data_ecc;

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)data_parity_buffer, data_ecc_bits);

                // Program Parity
                parity = data_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_data_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                if (Exchange.debug.nfc.phy.info_ecc)
                {
                    unsigned int i = 0;

                    Exchange.sys.fn.print("NFC_PHY_1stWriteData: Single Plane: Parity  : row(%05d), col(%05d): ", row0, col0);

                    for (i = 0; i < bytes_per_data_parity; i++)
                    {
                        Exchange.sys.fn.print("%02x ", data_parity_buffer[i]);
                    }   Exchange.sys.fn.print("\n");
                }

                col0 += bytes_per_data_parity;
            }

            /******************************************************************
             * 3rd : write spare
             ******************************************************************/
            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
                if (spare_buffer)
                {
                    spare = spare_buffer;
                    cur_write = spare;

                    // Program Spare
					spare = nfc_io_write(spare, (void *)&nfcShadowI->nfdata, bytes_per_spare_ecc);
                    col0 += bytes_per_spare_ecc;

                    // Program Parity
                    parity = spare_parity_buffer;
#if defined (__KTW_SUPPORT_SPARE_RAMDOMIZE__)
                    NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                    parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_spare_parity, 1);
#endif
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_spare_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);

                    col0 += bytes_per_spare_parity;
                }
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

            /******************************************************************
             * 4th : program
             ******************************************************************/
            if ((1 == cache_write_support) && (1 == cache_write_type))
            {
                NFC_PHY_Cmd(NF_CMD_PROG_CACHE_2ND); // CMD80 - CMD15 - CMD80 - CMD10
                NFC_PHY_tDelay(NfcTime.tWB);
            }
            else
            {
                NFC_PHY_Cmd(NF_CMD_PROG_2ND); // CMD80 - CMD10
                NFC_PHY_tDelay(NfcTime.tWB);
            }
        }
        /**********************************************************************
         * Multi Plane Write
         *
         *  CMD80 - ADDR - DATA LOADING - CMD11 - CMD80 - ADDR - DATA LOADING - CMD10 or
         *  CMD80 - ADDR - DATA LOADING - CMD11 - CMD80 - ADDR - DATA LOADING - CMD15 - CMD80 - ADDR - DATA LOADING - CMD11 - CMD80 - ADDR - DATA LOADING - CMD10 or
         *  CMD80 - ADDR - DATA LOADING - CMD11 - CMD81 - ADDR - DATA LOADING - CMD10 or
         *  CMD80 - ADDR - DATA LOADING - CMD11 - CMD81 - ADDR - DATA LOADING - CMD15 - CMD80 - ADDR - DATA LOADING - CMD11 - CMD81 - ADDR - DATA LOADING - CMD10
         *
         **********************************************************************/
        else
        {
            /******************************************************************
             * Plane 0 : 1st : set program
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_80);
            NFC_PHY_Addr(__NROOT(col0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(col0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
            NFC_PHY_tDelay(NfcTime.tADL);

            /******************************************************************
             * Plane 0 : 2nd : write data
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_data_ecc, data_ecc_bits);

            data = data_buffer0;

            for (data_loop = 0; data_loop <= data_loop_count0; data_loop++)
            {
                cur_write = data;

                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                data = data_buffer0 + (data_loop * bytes_per_data_ecc);
                data = (char *)NFC_PHY_RAND_Randomize((void *)data, bytes_per_data_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                if (Exchange.debug.nfc.phy.info_ecc)
                {
                    unsigned int i = 0;

                    Exchange.sys.fn.print("NFC_PHY_1stWriteData: Plane0: Data    : row(%05d), col(%05d)\n", row0, col0);

                    for (i = 0; i < 32; i++)
                    {
                        Exchange.sys.fn.print("%02x ", cur_write[i]);
                    }   Exchange.sys.fn.print("\n");
                }

                col0 += bytes_per_data_ecc;

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)data_parity_buffer, data_ecc_bits);

                // Program Parity
                parity = data_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_data_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                if (Exchange.debug.nfc.phy.info_ecc)
                {
                    unsigned int i = 0;

                    Exchange.sys.fn.print("NFC_PHY_1stWriteData: Plane0: Parity  : row(%05d), col(%05d): ", row0, col0);

                    for (i = 0; i < bytes_per_data_parity; i++)
                    {
                        Exchange.sys.fn.print("%02x ", data_parity_buffer[i]);
                    }   Exchange.sys.fn.print("\n");
                }

                col0 += bytes_per_data_parity;
            }

            /******************************************************************
             * Plane 0 : 3rd : write spare
             ******************************************************************/
            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
                if (spare_buffer)
                {
                    spare = spare_buffer;
                    cur_write = spare;

                    // Program Spare
					spare = nfc_io_write(spare, (void *)&nfcShadowI->nfdata, bytes_per_spare_ecc);
                    col0 += bytes_per_spare_ecc;

                    // Program Parity
                    parity = spare_parity_buffer;
#if defined (__KTW_SUPPORT_SPARE_RAMDOMIZE__)
                    NFC_PHY_RAND_SetPageSeed(row0&0x000000FF);
                    parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_spare_parity, 1);
#endif
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_spare_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);

                    col0 += bytes_per_spare_parity;
                }
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

            /******************************************************************
             * Plane 0 : 4th : program & wait ready
             ******************************************************************/
            NFC_PHY_Cmd(NF_CMD_PROG_PLANE_2ND);
            NFC_PHY_tDelay(NfcTime.tWB);

            // Check BUSY : Status Read
            NFC_PHY_Cmd(NF_CMD_READ_STATUS);
            NFC_PHY_tDelay(NfcTime.tWHR);
            while (!NFC_PHY_StatusIsRDY(nfcShadowI->nfdata));

            /******************************************************************
             * Plane 1 : 1st : set program
             ******************************************************************/
            switch (multi_plane_write_type)
            {
                case 1:  { NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_81); } break;
                case 2:  { NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_80); } break;
                default: { NFC_PHY_Cmd(NF_CMD_PROG_PLANE_1ST_81); } break;
            }
            NFC_PHY_Addr(__NROOT(col1&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(col1&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row1&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row1&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row1&0x00FF0000,16));
            NFC_PHY_tDelay(NfcTime.tADL);

            /******************************************************************
             * Plane 1 : 2nd : write data
             ******************************************************************/
            NFC_PHY_EccEncoderSetup(bytes_per_data_ecc, data_ecc_bits);

            data = data_buffer1;

            for (data_loop = 0; data_loop <= data_loop_count1; data_loop++)
            {
                cur_write = data;

                NFC_PHY_EccEncoderEnable();

                // Program Data
                NFC_PHY_RAND_SetPageSeed(row1&0x000000FF);
                data = data_buffer1 + (data_loop * bytes_per_data_ecc);
                data = (char *)NFC_PHY_RAND_Randomize((void *)data, bytes_per_data_ecc, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                if (Exchange.debug.nfc.phy.info_ecc)
                {
                    unsigned int i = 0;

                    Exchange.sys.fn.print("NFC_PHY_1stWriteData: Plane1: Data    : row(%05d), col(%05d)\n", row1, col1);

                    for (i = 0; i < 32; i++)
                    {
                        Exchange.sys.fn.print("%02x ", cur_write[i]);
                    }   Exchange.sys.fn.print("\n");
                }

                col1 += bytes_per_data_ecc;

                NFC_PHY_EccEncoderWaitDone();
                NFC_PHY_EccEncoderReadParity((unsigned int *)data_parity_buffer, data_ecc_bits);

                // Program Parity
                parity = data_parity_buffer;

                NFC_PHY_RAND_SetPageSeed(row1&0x000000FF);
                parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_data_parity, 1);

                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_data_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

                if (Exchange.debug.nfc.phy.info_ecc)
                {
                    unsigned int i = 0;

                    Exchange.sys.fn.print("NFC_PHY_1stWriteData: Plane1: Parity  : row(%05d), col(%05d): ", row1, col1);

                    for (i = 0; i < bytes_per_data_parity; i++)
                    {
                        Exchange.sys.fn.print("%02x ", data_parity_buffer[i]);
                    }   Exchange.sys.fn.print("\n");
                }

                col1 += bytes_per_data_parity;
            }

            /******************************************************************
             * Plane 1 : 3rd : write spare
             ******************************************************************/
            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
                if (spare_buffer)
                {
                    spare = spare_buffer;
                    cur_write = spare;

                    // Program Spare
					spare = nfc_io_write(spare, (void *)&nfcShadowI->nfdata, bytes_per_spare_ecc);
                    col1 += bytes_per_spare_ecc;

                    // Program Parity
                    parity = spare_parity_buffer;
#if defined (__KTW_SUPPORT_SPARE_RAMDOMIZE__)
                    NFC_PHY_RAND_SetPageSeed(row1&0x000000FF);
                    parity = (char *)NFC_PHY_RAND_Randomize((void *)parity, bytes_per_spare_parity, 1);
#endif
					parity = nfc_io_write(parity, (void *)&nfcShadowI->nfdata, bytes_per_spare_parity);
                    NFC_PHY_tDelay(NfcTime.tParity);

                    col1 += bytes_per_spare_parity;
                }
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }

            /******************************************************************
             * Plane 1 : 4th : program
             ******************************************************************/
            if ((1 == cache_write_support) && (1 == cache_write_type))
            {
                NFC_PHY_Cmd(NF_CMD_PROG_CACHE_2ND); // CMD80 - CMD15 - CMD80 - CMD10
                NFC_PHY_tDelay(NfcTime.tWB);
            }
            else
            {
                NFC_PHY_Cmd(NF_CMD_PROG_2ND); // CMD80 - CMD10
                NFC_PHY_tDelay(NfcTime.tWB);
            }
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print(".. Issue ."); }

    return 0;
}

int NFC_PHY_1stWriteDataNoEcc(unsigned int _channel,
                              unsigned int _way,
                              unsigned int _row,
                              unsigned int _col,
                              /* DATA */
                              unsigned int _data_loop_count,
                              unsigned int _bytes_per_data_ecc,
                              void * _data_buffer,
                              /* SPARE */
                              unsigned int _bytes_spare,
                              void * _spare_buffer)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row = _row;
    unsigned int col = _col;

    /* DATA */
    unsigned int data_loop = 0;
    unsigned int data_loop_count = _data_loop_count;
    unsigned int bytes_per_data_ecc = _bytes_per_data_ecc;
    char * data_buffer = _data_buffer;
    char * data = data_buffer;

    /* SPARE */
    unsigned int bytes_spare = _bytes_spare;
    char * spare_buffer = _spare_buffer;
    char * spare = spare_buffer;

#if defined (__COMPILE_MODE_READONLY__)
    Exchange.sys.fn.print("EWS.NFC: WriteSkip: DataNoEcc\n");
    return 0;
#endif

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        /**********************************************************************
         * 1st : Set Program
         **********************************************************************/
        NFC_PHY_Cmd(NF_CMD_PROG_1ST);
        NFC_PHY_Addr(__NROOT(col&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(col&0x0000FF00,8));
        NFC_PHY_Addr(__NROOT(row&0x000000FF,0));
        NFC_PHY_Addr(__NROOT(row&0x0000FF00,8));
        NFC_PHY_Addr(__NROOT(row&0x00FF0000,16));
        NFC_PHY_tDelay(NfcTime.tADL);

        /**********************************************************************
         * 2nd : Write Data
         **********************************************************************/
        if (__NULL != data_buffer)
        {
            for (data_loop = 0; data_loop <= data_loop_count; data_loop++)
            {
                if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
                {
					data = nfc_io_write(data, (void *)&nfcShadowI->nfdata, bytes_per_data_ecc);
                }
                if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }
            }
        }

        /**********************************************************************
         * 3rd : Write Spare
         **********************************************************************/
        if (__NULL != spare_buffer)
        {
            if (Exchange.nfc.fnBoostOn) { Exchange.nfc.fnBoostOn(); }
            {
				spare = nfc_io_write(spare, (void *)&nfcShadowI->nfdata, bytes_spare);
            }
            if (Exchange.nfc.fnBoostOff) { Exchange.nfc.fnBoostOff(); }
        }

        /**********************************************************************
         * 4th : program
         **********************************************************************/
        NFC_PHY_Cmd(NF_CMD_PROG_2ND); // CMD80 - CMD10
        NFC_PHY_tDelay(NfcTime.tWB);
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return 0;
}

int NFC_PHY_2ndWrite(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return 0;
}

unsigned char NFC_PHY_3rdWrite(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;

    unsigned char status = 0;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        status = nfcShadowI32->nfdata;
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation)
    {
        if (NFC_PHY_StatusIsRDY(status))
        {
            Exchange.sys.fn.print(".. Done\n");
        }
    }

    return status;
}

/******************************************************************************
 *
 ******************************************************************************/
int NFC_PHY_1stErase(unsigned int _channel,
                     unsigned int _way,
                     unsigned int _row0,
                     unsigned int _row1,
                     unsigned int _multi_plane_erase_cmd)
{
    unsigned int channel = _channel;
    unsigned int way = _way;
    unsigned int row0 = _row0;
    unsigned int row1 = _row1;
    unsigned int block0 = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);
    unsigned int block1 = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

    unsigned int multi_plane_erase_cmd = _multi_plane_erase_cmd;

#if defined (__COMPILE_MODE_READONLY__)
    Exchange.sys.fn.print("EWS.NFC: EraseSkip\n");
    return 0;
#endif

    /**************************************************************************
     *
     **************************************************************************/
    if (Exchange.debug.nfc.phy.operation)
    {
        unsigned int _page  = __NROOT(_row0&0x000000FF,0);
        unsigned int _block = __NROOT(_row0&0x000F0000,8) + __NROOT(_row0&0x0000FF00,8);

        Exchange.sys.fn.print("Erase.0      : block(%04d), page(%04d) => row(%08d)               .", _block, _page, _row0);

        if ((int)_row1 >= 0)
        {
            _page   = __NROOT(_row1&0x000000FF,0);
            _block  = __NROOT(_row1&0x000F0000,8) + __NROOT(_row1&0x0000FF00,8);

            Exchange.sys.fn.print("\n");
            Exchange.sys.fn.print("Erase.1      : block(%04d), page(%04d) => row(%08d)               .", _block, _page, _row1);
        }
    }

    /**************************************************************************
     * FTL Side : Violate Prohibited Block Access, But Allowed
     **************************************************************************/
    if (((0 <= block0) && (block0 < Exchange.ewsftl_start_block)) || ((0 <= block1) && (block1 < Exchange.ewsftl_start_block)))
    {
        if (Exchange.debug.nfc.phy.warn_prohibited_block_access)
        {
            Exchange.sys.fn.print("EWS.NFC: Warnning: Erase Requested \"Block %d\" or \"Block %d\" Prohibited, But Allowed !!!\n", block0, block1);
        }
    }

    /**************************************************************************
     *
     **************************************************************************/
    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        // Single Plane Erase
        if (0xFFFFFFFF == row1)
        {
            NFC_PHY_Cmd(NF_CMD_ERASE_1ST);
            NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
            NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
            NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
            NFC_PHY_Cmd(NF_CMD_ERASE_2ND);
            NFC_PHY_tDelay(NfcTime.tWB);

            // May Be After tBERS Block Erase Finish.
        }
        // Multi Plane Erase
        else
        {
            if (2 == multi_plane_erase_cmd)
            {
                // First Plane Erase
                NFC_PHY_Cmd(NF_CMD_ERASE_1ST);
                NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
                NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));
                NFC_PHY_Cmd(NF_CMD_ERASE_PLANE_2ND);
                NFC_PHY_tDelay(NfcTime.tWB);

                // Execute Erase, Check BUSY : Status Read
                NFC_PHY_Cmd(NF_CMD_READ_STATUS);
                NFC_PHY_tDelay(NfcTime.tWHR);
                while (!NFC_PHY_StatusIsRDY(nfcShadowI->nfdata));

                // Second Plane Erase
                NFC_PHY_Cmd(NF_CMD_ERASE_1ST);
                NFC_PHY_Addr(__NROOT(row1&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(row1&0x0000FF00,8));
                NFC_PHY_Addr(__NROOT(row1&0x00FF0000,16));

                // Execute Erase
                NFC_PHY_Cmd(NF_CMD_ERASE_2ND);
                NFC_PHY_tDelay(NfcTime.tWB);
            }
            else
            {
                // First Plane Erase
                NFC_PHY_Cmd(NF_CMD_ERASE_1ST);
                NFC_PHY_Addr(__NROOT(row0&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(row0&0x0000FF00,8));
                NFC_PHY_Addr(__NROOT(row0&0x00FF0000,16));

                // Second Plane Erase
                NFC_PHY_Cmd(NF_CMD_ERASE_1ST);
                NFC_PHY_Addr(__NROOT(row1&0x000000FF,0));
                NFC_PHY_Addr(__NROOT(row1&0x0000FF00,8));
                NFC_PHY_Addr(__NROOT(row1&0x00FF0000,16));

                // Execute Erase
                NFC_PHY_Cmd(NF_CMD_ERASE_2ND);
                NFC_PHY_tDelay(NfcTime.tWB);
            }

            // May Be After tBERS, Block Erase Finish.
        }
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation) { Exchange.sys.fn.print(".. Issue ."); }

    return 0;
}

int NFC_PHY_2ndErase(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    return 0;
}

unsigned char NFC_PHY_3rdErase(unsigned int _channel, unsigned int _way)
{
    unsigned int channel = _channel;
    unsigned int way = _way;

    unsigned char status = 0;

    NFC_PHY_ChipSelect(channel, way, __TRUE);
    {
        status = nfcShadowI32->nfdata;
    }
    NFC_PHY_ChipSelect(channel, way, __FALSE);

    if (Exchange.debug.nfc.phy.operation)
    {
        if (NFC_PHY_StatusIsRDY(status))
        {
            Exchange.sys.fn.print(".. Done (%x)\n", status);
        }
    }

    return status;
}

/******************************************************************************
 * Optimize Restore
 ******************************************************************************/
#if defined (__COMPILE_MODE_BEST_DEBUGGING__)
//#pragma GCC pop_options
#endif

