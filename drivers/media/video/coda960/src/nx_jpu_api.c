//
//	Module : Nexell VPU Device Driver for CNM Coda960
//	Description : Device Driver (Firmware) Layer Interface
//	Author : SeongO-Park ( ray@nexell.co.kr )
//

#ifndef UNUSED
#define UNUSED(p) ((void)(p))
#endif

#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <linux/mutex.h>
#include <mach/soc.h>
#if defined (CONFIG_ARCH_NXP6818)
#include <mach/s5p6818.h>
#endif
#if defined (CONFIG_ARCH_NXP4418)
#include <mach/s5p6818.h>
#endif
#include <mach/devices.h>



#include "vpu_hw_interface.h"			//	Register Access
#include "../include/nx_vpu_api.h"
#include "../include/nx_vpu_config.h"

//	system
#include "../include/nx_alloc_mem.h"
#include "../include/vpu_types.h"

#define	NX_DTAG		"[DRV|JPU_API]"
#include "../include/drv_osapi.h"

#define DBG_VBS		0
#define DBG_DBG		0
#define DBG_ERR		1

#include "nx_vpu_gdi.h"

#if ( ( defined(__LINUX__) || defined(linux) || defined(__linux__) )&& defined(__KERNEL__) )
#include <mach/soc.h>
#include <mach/platform.h>
#endif

#define	ENABLE_INTERRUPT_MODE

#define DC_TABLE_INDEX0				0
#define AC_TABLE_INDEX0				1
#define DC_TABLE_INDEX1				2
#define AC_TABLE_INDEX1				3

#define Q_COMPONENT0				0
#define Q_COMPONENT1				0x40
#define Q_COMPONENT2				0x80


//------------------------------------------------------------------------------
// File: VpuJpegTable.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#ifndef JPEG_TABLE_H
#define JPEG_TABLE_H

static unsigned char lumaDcBits[16] = {
	0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static unsigned char lumaDcValue[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x00, 0x00, 0x00, 0x00,
};
static unsigned char lumaAcBits[16] = {
	0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
	0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
};
static unsigned char lumaAcValue[168] = {
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
	0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
	0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
	0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
	0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
	0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4,
	0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
	0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
	0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static unsigned char chromaDcBits[16] = {
	0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static unsigned char chromaDcValue[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x00, 0x00, 0x00, 0x00,
};
static unsigned char chromaAcBits[16] = {
	0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
	0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
};
static unsigned char chromaAcValue[168] = {
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0,
	0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
	0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
	0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
	0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
	0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3,
	0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2,
	0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
	0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
	0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#ifdef USE_PRE_DEF_QTABLE
static unsigned char lumaQ[64] = {
	0x06, 0x04, 0x04, 0x04, 0x05, 0x04, 0x06, 0x05,
	0x05, 0x06, 0x09, 0x06, 0x05, 0x06, 0x09, 0x0B,
	0x08, 0x06, 0x06, 0x08, 0x0B, 0x0C, 0x0A, 0x0A,
	0x0B, 0x0A, 0x0A, 0x0C, 0x10, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x10, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
};
static unsigned char chromaBQ[64] = {
	0x07, 0x07, 0x07, 0x0D, 0x0C, 0x0D, 0x18, 0x10,
	0x10, 0x18, 0x14, 0x0E, 0x0E, 0x0E, 0x14, 0x14,
	0x0E, 0x0E, 0x0E, 0x0E, 0x14, 0x11, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x11, 0x11, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x11, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
};
#endif

static unsigned char cInfoTable[5][24] = {
	{ 00, 02, 02, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //420
	{ 00, 02, 01, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //422H
	{ 00, 01, 02, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //422V
	{ 00, 01, 01, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //444	
	{ 00, 01, 01, 00, 00, 00, 01, 00, 00, 00, 00, 00, 02, 00, 00, 00, 00, 00, 03, 00, 00, 00, 00, 00 }, //400
};

#endif	//	JPEG_TABLE_H

int JpuSetupTables(EncJpegInfo *pJpgInfo, int quality)
{
	int scale_factor;
	int i;
	long temp;
	const int force_baseline = 1;

	/* These are the sample quantization tables given in JPEG spec section K.1.
	* The spec says that the values given produce "good" quality, and
	* when divided by 2, "very good" quality.
	*/
	static const unsigned int std_luminance_quant_tbl[64] = {
		16,  11,  10,  16,  24,  40,  51,  61,
		12,  12,  14,  19,  26,  58,  60,  55,
		14,  13,  16,  24,  40,  57,  69,  56,
		14,  17,  22,  29,  51,  87,  80,  62,
		18,  22,  37,  56,  68, 109, 103,  77,
		24,  35,  55,  64,  81, 104, 113,  92,
		49,  64,  78,  87, 103, 121, 120, 101,
		72,  92,  95,  98, 112, 100, 103,  99
	};
	static const unsigned int std_chrominance_quant_tbl[64] = {
		17,  18,  24,  47,  99,  99,  99,  99,
		18,  21,  26,  66,  99,  99,  99,  99,
		24,  26,  56,  99,  99,  99,  99,  99,
		47,  66,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99
	};

	if (quality <= 0) quality = 1;
	if (quality > 100) quality = 100;

	/* The basic table is used as-is (scaling 100) for a quality of 50.
	* Qualities 50..100 are converted to scaling percentage 200 - 2*Q;
	* note that at Q=100 the scaling is 0, which will cause jpeg_add_quant_table
	* to make all the table entries 1 (hence, minimum quantization loss).
	* Qualities 1..50 are converted to scaling percentage 5000/Q.
	*/
	if (quality < 50)
		scale_factor = 5000 / quality;
	else
		scale_factor = 200 - quality*2;


	for (i = 0; i < 64; i++) {
		temp = ((long) std_luminance_quant_tbl[i] * scale_factor + 50L) / 100L;
		/* limit the values to the valid range */
		if (temp <= 0L) temp = 1L;
		if (temp > 32767L) temp = 32767L; /* max quantizer needed for 12 bits */
		if (force_baseline && temp > 255L)
			temp = 255L;		/* limit to baseline range if requested */

		pJpgInfo->qMatTab[DC_TABLE_INDEX0][i] = (unsigned char)temp;
	}

	for (i = 0; i < 64; i++) {
		temp = ((long) std_chrominance_quant_tbl[i] * scale_factor + 50L) / 100L;
		/* limit the values to the valid range */
		if (temp <= 0L) temp = 1L;
		if (temp > 32767L) temp = 32767L; /* max quantizer needed for 12 bits */
		if (force_baseline && temp > 255L)
			temp = 255L;		/* limit to baseline range if requested */

		pJpgInfo->qMatTab[AC_TABLE_INDEX0][i] = (unsigned char)temp;		
	}

	//setting of qmatrix table information
	//#define USE_CNM_DEFAULT_QMAT_TABLE
#ifdef USE_CNM_DEFAULT_QMAT_TABLE
	memcpy(&pJpgInfo->qMatTab[DC_TABLE_INDEX0], lumaQ, 64);
	memcpy(&pJpgInfo->qMatTab[AC_TABLE_INDEX0], chromaBQ, 64);
#endif

	memcpy(&pJpgInfo->qMatTab[DC_TABLE_INDEX1], &pJpgInfo->qMatTab[DC_TABLE_INDEX0], 64);
	memcpy(&pJpgInfo->qMatTab[AC_TABLE_INDEX1], &pJpgInfo->qMatTab[AC_TABLE_INDEX0], 64);

	//setting of huffman table information
	memcpy(&pJpgInfo->huffBits[DC_TABLE_INDEX0], lumaDcBits, 16);		// Luma DC BitLength
	memcpy(&pJpgInfo->huffVal[DC_TABLE_INDEX0], lumaDcValue, 16);		// Luma DC HuffValue

	memcpy(&pJpgInfo->huffBits[AC_TABLE_INDEX0], lumaAcBits, 16);		// Luma AC BitLength
	memcpy(&pJpgInfo->huffVal[AC_TABLE_INDEX0], lumaAcValue, 162);		// Luma AC HuffValue

	memcpy(&pJpgInfo->huffBits[DC_TABLE_INDEX1], chromaDcBits, 16);		// Chroma DC BitLength
	memcpy(&pJpgInfo->huffVal[DC_TABLE_INDEX1], chromaDcValue, 16);		// Chroma DC HuffValue

	memcpy(&pJpgInfo->huffBits[AC_TABLE_INDEX1], chromaAcBits, 16);		// Chroma AC BitLength
	memcpy(&pJpgInfo->huffVal[AC_TABLE_INDEX1], chromaAcValue, 162);	// Chorma AC HuffValue

	return 1;
}

enum {
    INT_JPU_DONE = 0,
    INT_JPU_ERROR = 1,
    INT_JPU_BIT_BUF_EMPTY = 2,
    INT_JPU_BIT_BUF_FULL = 2,
    INT_JPU_PARIAL_OVERFLOW = 3
};


int JPU_IsBusy(void)
{
	unsigned int val;
	val = VpuReadReg(MJPEG_PIC_STATUS_REG);
	
	if ((val & (1<<INT_JPU_DONE)) ||
		(val & (1<<INT_JPU_ERROR)))
		return 0;
	
	return 1; 
}

void JPU_ClrStatus(unsigned int val)
{
	if (val != 0)
		VpuWriteReg(MJPEG_PIC_STATUS_REG, val);
}

unsigned int JPU_GetStatus(void)
{
	return VpuReadReg(MJPEG_PIC_STATUS_REG);
}


//
//	Initialize Interrupt
//
#ifdef ENABLE_INTERRUPT_MODE
static DECLARE_WAIT_QUEUE_HEAD(gst_JPU_WaitQueue);
static atomic_t gJpuEventPresent =  ATOMIC_INIT(0);
static int gJpuIntrReason = 0;

static irqreturn_t JPU_InterruptHandler( int irq, void *desc )
{
	int ret;
	ret = JPU_GetStatus();
	JPU_ClrStatus(ret);
	gJpuIntrReason = ret;
	atomic_set(&gJpuEventPresent, 1);
	wake_up_interruptible(&gst_JPU_WaitQueue);
	return IRQ_HANDLED;
}
#endif

int JPU_InitInterrupt(void)
{
#ifdef ENABLE_INTERRUPT_MODE

	//	Initialize Interrupt
	if(request_irq( IRQ_PHY_CODA960_JPG, JPU_InterruptHandler, IRQF_DISABLED, "JPU_CODEC_IRQ", NULL) != 0)
	{
		NX_ErrMsg( ("%s:%d Cannot get IRQ %d\n", __func__, __LINE__, IRQ_PHY_CODA960_JPG ) );
		return -1;
	}
	init_waitqueue_head(&gst_JPU_WaitQueue);
	atomic_set(&gJpuEventPresent, 0);
#endif
	return 0;
}

void JPU_DeinitInterrupt(void)
{
#ifdef ENABLE_INTERRUPT_MODE
	free_irq(IRQ_PHY_CODA960_JPG, NULL);
#endif
}

static int JPU_WaitInterrupt(int timeOut)
{
#ifdef ENABLE_INTERRUPT_MODE
	unsigned int reason=0;
	if( 0 == wait_event_interruptible_timeout(gst_JPU_WaitQueue, atomic_read(&gJpuEventPresent), msecs_to_jiffies(timeOut)) )
	{
		reason=JPU_GetStatus();
		NX_ErrMsg(("JPU_WaitBitInterrupt() TimeOut!!!(reason = 0x%.8x)\n", reason));
		VPU_SWReset( SW_RESET_SAFETY );
		return 0;
	}
	else
	{
		atomic_set(&gJpuEventPresent, 0);
		reason = gJpuIntrReason;
		return reason;
	}
#else
	unsigned int reason=0;
	while(timeOut > 0)
	{
		DrvMSleep( 1 );
		if( JPU_IsBusy() )
		{
			reason = JPU_GetStatus();
			if (reason & (1<<INT_JPU_BIT_BUF_FULL))
			{
				NX_ErrMsg(("Stream Buffer Too Small!!!"));
				JPU_ClrStatus((1<<INT_JPU_BIT_BUF_FULL));
				return reason;
			}
		}
		else
		{
			break;
		}
		timeOut --;
		if( timeOut == 0 )
		{
			NX_ErrMsg(("JPU TimeOut!!!"));
			break;
		}
	}
	reason = JPU_GetStatus();
	return reason;
#endif
}


static int JpgEncGenHuffTab(VpuEncInfo * pEncInfo, int tabNum)
{
	int p, i, l, lastp, si, maxsymbol;
	static int huffsize[256];
	static int huffcode[256];
	int code;

	unsigned char *bitleng, *huffval;
	unsigned int *ehufco, *ehufsi;

	EncJpegInfo *pJpgInfo = &pEncInfo->EncCodecParam.jpgEncInfo;

	bitleng	= pJpgInfo->huffBits[tabNum];
	huffval	= pJpgInfo->huffVal[tabNum];
	ehufco	= pJpgInfo->huffCode[tabNum];
	ehufsi	= pJpgInfo->huffSize[tabNum];

	maxsymbol = tabNum & 1 ? 256 : 16;

	/* Figure C.1: make table of Huffman code length for each symbol */

	p = 0;
	for (l=1; l<=16; l++) {
		i = bitleng[l-1];
		if (i < 0 || p + i > maxsymbol)
			return 0;
		while (i--)
			huffsize[p++] = l;
	}
	lastp = p;

	/* Figure C.2: generate the codes themselves */
	/* We also validate that the counts represent a legal Huffman code tree. */

	code = 0;
	si = huffsize[0];
	p = 0;
	while (p < lastp) {
		while (huffsize[p] == si) {
			huffcode[p++] = code;
			code++;
		}
		if (code >= (1 << si))
			return 0;
		code <<= 1;
		si++;
	}

	/* Figure C.3: generate encoding tables */
	/* These are code and size indexed by symbol value */

	for(i=0; i<256; i++)
		ehufsi[i] = 0x00;

	for(i=0; i<256; i++)
		ehufco[i] = 0x00;

	for (p=0; p<lastp; p++) {
		i = huffval[p];
		if (i < 0 || i >= maxsymbol || ehufsi[i])
			return 0;
		ehufco[i] = huffcode[p];
		ehufsi[i] = huffsize[p];
	}

	return 1;
}

static int JpgEncLoadHuffTab(VpuEncInfo * pEncInfo)
{
	int i, j, t;
	int huffData;
	EncJpegInfo *pJpgInfo = &pEncInfo->EncCodecParam.jpgEncInfo;

	for (i=0; i<4; i++)
		JpgEncGenHuffTab(pEncInfo, i);

	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x3);

	for (j=0; j<4; j++) 
	{

		t = (j==0) ? AC_TABLE_INDEX0 : (j==1) ? AC_TABLE_INDEX1 : (j==2) ? DC_TABLE_INDEX0 : DC_TABLE_INDEX1;

		for (i=0; i<256; i++) 
		{
			if ((t==DC_TABLE_INDEX0 || t==DC_TABLE_INDEX1) && (i>15))	// DC
				break;

			if ((pJpgInfo->huffSize[t][i] == 0) && (pJpgInfo->huffCode[t][i] == 0))
				huffData = 0;
			else 
			{
				huffData =                    (pJpgInfo->huffSize[t][i] - 1);	// Code length (1 ~ 16), 4-bit
				huffData = (huffData << 16) | (pJpgInfo->huffCode[t][i]    );	// Code word, 16-bit
			}
			VpuWriteReg(MJPEG_HUFF_DATA_REG, huffData);
		}
	}
	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x0);
	return 1;
}

static int JpgEncLoadQMatTab(VpuEncInfo * pEncInfo)
{
	long dividend = 0x80000;
	long quotient;
	int quantID;
	int divisor;
	int comp;
	int i, t;
	EncJpegInfo *pJpgInfo = &pEncInfo->EncCodecParam.jpgEncInfo;

	for (comp=0; comp<3; comp++)
	{
		quantID = pJpgInfo->cInfoTab[comp][3];
		if (quantID >= 4)
			return 0;
		t = (comp==0)? Q_COMPONENT0 :
			(comp==1)? Q_COMPONENT1 : Q_COMPONENT2;
		VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x3 + t);
		for (i=0; i<64; i++)
		{
			divisor = pJpgInfo->qMatTab[quantID][i];
			quotient= dividend / divisor;
			VpuWriteReg(MJPEG_QMAT_DATA_REG, (int) quotient);
		}
		VpuWriteReg(MJPEG_QMAT_CTRL_REG, t);
	}
	return 1;
}


NX_VPU_RET VPU_EncMjpgDefParam( VpuEncInfo *pInfo )
{
    unsigned char *p;
	EncJpegInfo *pJpgInfo = &pInfo->EncCodecParam.jpgEncInfo;
	int format;
	pJpgInfo->frameIdx = 0;
	pJpgInfo->seqInited = 0;
	format = pJpgInfo->format = IMG_FORMAT_420;
	pJpgInfo->rstIntval = 60;

	if (format == IMG_FORMAT_420) {
		pJpgInfo->busReqNum = 2;
		pJpgInfo->mcuBlockNum = 6;
		pJpgInfo->compNum = 3;
		pJpgInfo->compInfo[0] = 10;
		pJpgInfo->compInfo[1] = 5;
		pJpgInfo->compInfo[2] = 5;
	} else if (format == IMG_FORMAT_422) {
		pJpgInfo->busReqNum = 3;
		pJpgInfo->mcuBlockNum = 4;
		pJpgInfo->compNum = 3;
		pJpgInfo->compInfo[0] = 9;
		pJpgInfo->compInfo[1] = 5;
		pJpgInfo->compInfo[2] = 5;
	} else if (format == IMG_FORMAT_224) {
		pJpgInfo->busReqNum  = 3;
		pJpgInfo->mcuBlockNum = 4;
		pJpgInfo->compNum = 3;
		pJpgInfo->compInfo[0] = 6;
		pJpgInfo->compInfo[1] = 5;
		pJpgInfo->compInfo[2] = 5;
	} else if (format == IMG_FORMAT_444) {
		pJpgInfo->busReqNum = 4;
		pJpgInfo->mcuBlockNum = 3;
		pJpgInfo->compNum = 3;
		pJpgInfo->compInfo[0] = 5;
		pJpgInfo->compInfo[1] = 5;
		pJpgInfo->compInfo[2] = 5;
	} else if (format == IMG_FORMAT_400) {
		pJpgInfo->busReqNum = 4;
		pJpgInfo->mcuBlockNum = 1;
		pJpgInfo->compNum = 1;
		pJpgInfo->compInfo[0] = 5;
		pJpgInfo->compInfo[1] = 0;
		pJpgInfo->compInfo[2] = 0;
	}

	p = &cInfoTable[format][0];
    NX_DrvMemcpy( &pJpgInfo->cInfoTab[0], p, 6 ); p += 6;
	NX_DrvMemcpy( &pJpgInfo->cInfoTab[1], p, 6 ); p += 6;
	NX_DrvMemcpy( &pJpgInfo->cInfoTab[2], p, 6 ); p += 6;
	NX_DrvMemcpy( &pJpgInfo->cInfoTab[3], p, 6 );
	return VPU_RET_OK;
}


#define PUT_BYTE(_p, _b) \
	if (tot++ > len) return VPU_RET_ERROR; \
	*_p++ = (unsigned char)(_b);	

NX_VPU_RET NX_VpuJpegGetHeader( NX_VPU_INST_HANDLE handle, VPU_ENC_GET_HEADER_ARG *pArg )
{
	VpuEncInfo *pInfo = &handle->codecInfo.encInfo;
	EncJpegInfo *pJpgInfo = &pInfo->EncCodecParam.jpgEncInfo;
	unsigned char *p;
	int i, tot, len;
//	int jfifLen = 16, pad;

	tot = 0;
	len = sizeof(pArg->jpgHeader.jpegHeader);
	p = pArg->jpgHeader.jpegHeader;

	// SOI Header
	PUT_BYTE(p, 0xff);
	PUT_BYTE(p, 0xD8);

#if 0
	// JFIF marker Header : Added by Ray Park for Normal Jpeg File
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xE0);
	PUT_BYTE(p, (jfifLen>>8));		//	Legnth
	PUT_BYTE(p, (jfifLen&0xFF));
	PUT_BYTE(p, 'J');				//	Identifier
	PUT_BYTE(p, 'F');
	PUT_BYTE(p, 'I');
	PUT_BYTE(p, 'F');
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x01);				//	Major Version
	PUT_BYTE(p, 0x01);				//	Minor Version
	PUT_BYTE(p, 0x00);				//	Density Units
	PUT_BYTE(p, 0x00);				//	X density
	PUT_BYTE(p, 0x01);
	PUT_BYTE(p, 0x00);				//	Y density
	PUT_BYTE(p, 0x01);
	PUT_BYTE(p, 0x00);				//	Thumbnail Width
	PUT_BYTE(p, 0x00);				//	THumbnail Height
#endif

	// APP9 Header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xE9);

	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x04);

	PUT_BYTE(p, (pJpgInfo->frameIdx >> 8));
	PUT_BYTE(p, (pJpgInfo->frameIdx & 0xFF));


	// DRI header
	if (pJpgInfo->rstIntval) {
		
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xDD);

		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0x04);

		PUT_BYTE(p, (pJpgInfo->rstIntval >> 8));
		PUT_BYTE(p, (pJpgInfo->rstIntval & 0xff));

	}
		
	// DQT Header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xDB);
	
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x43);

	PUT_BYTE(p, 0x00);

	for (i=0; i<64; i++) {
		PUT_BYTE(p, pJpgInfo->qMatTab[0][i]);
	}

	if (pJpgInfo->format != IMG_FORMAT_400) {
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xDB);
		
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0x43);
		
		PUT_BYTE(p, 0x01);

		for (i=0; i<64; i++) {
			PUT_BYTE(p, pJpgInfo->qMatTab[1][i]);
		}
	}
	
	// DHT Header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xC4);
	
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x1F);
	
	PUT_BYTE(p, 0x00);

	for (i=0; i<16; i++) {
		PUT_BYTE(p, pJpgInfo->huffBits[0][i]);
	}
	
	for (i=0; i<12; i++) {
		PUT_BYTE(p, pJpgInfo->huffVal[0][i]);
	}

	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xC4);
	
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0xB5);
	
	PUT_BYTE(p, 0x10);

	for (i=0; i<16; i++) {
		PUT_BYTE(p, pJpgInfo->huffBits[1][i]);
	}
	
	for (i=0; i<162; i++) {
		PUT_BYTE(p, pJpgInfo->huffVal[1][i]);
	}

	
	if (pJpgInfo->format != IMG_FORMAT_400) {

		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xC4);
		
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0x1F);
	
		PUT_BYTE(p, 0x01);

	
		for (i=0; i<16; i++) {
			PUT_BYTE(p, pJpgInfo->huffBits[2][i]);
		}	
		for (i=0; i<12; i++) {
			PUT_BYTE(p, pJpgInfo->huffVal[2][i]);
		}
			 
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xC4);
		
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0xB5);
		
		PUT_BYTE(p, 0x11);

			
		for (i=0; i<16; i++) {
			PUT_BYTE(p, pJpgInfo->huffBits[3][i]);
		}

		for (i=0; i<162; i++) {
			PUT_BYTE(p, pJpgInfo->huffVal[3][i]);			
		}
	}

	// SOF header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xC0);
	
	PUT_BYTE(p, (((8+(pJpgInfo->compNum*3)) >> 8) & 0xFF));
	PUT_BYTE(p, ((8+(pJpgInfo->compNum*3)) & 0xFF));
	
	PUT_BYTE(p, 0x08);

	if (pJpgInfo->rotationEnable && (pJpgInfo->rotationAngle == 90 || pJpgInfo->rotationAngle == 270)) {
		PUT_BYTE(p, (pInfo->srcWidth >> 8));
		PUT_BYTE(p, (pInfo->srcWidth & 0xFF));
		PUT_BYTE(p, (pInfo->srcHeight >> 8));
		PUT_BYTE(p, (pInfo->srcHeight & 0xFF));
	}
	else 
	{
		PUT_BYTE(p, (pInfo->srcHeight >> 8));
		PUT_BYTE(p, (pInfo->srcHeight & 0xFF));
		PUT_BYTE(p, (pInfo->srcWidth >> 8));
		PUT_BYTE(p, (pInfo->srcWidth & 0xFF));
	}

	PUT_BYTE(p, pJpgInfo->compNum);

	for (i=0; i<pJpgInfo->compNum; i++) {
		PUT_BYTE(p, (i+1));
		PUT_BYTE(p, ((pJpgInfo->cInfoTab[i][1]<<4) & 0xF0) + (pJpgInfo->cInfoTab[i][2] & 0x0F));
		PUT_BYTE(p, pJpgInfo->cInfoTab[i][3]);		
	}

	//tot = p - para->pParaSet;
#if 0
	pad = 0;
	if (tot % 8) {
		pad = tot % 8;
		pad = 8-pad;
		for (i=0; i<pad; i++) {
			PUT_BYTE(p, 0x00);
		}
	}
#endif

	pJpgInfo->frameIdx++;
	pArg->jpgHeader.headerSize = tot;

	NX_DbgMsg( DBG_VBS, ("JPEG Header Done\n") );
	return VPU_RET_OK;
}

NX_VPU_RET NX_VpuJpegRunFrame( NX_VPU_INST_HANDLE handle, VPU_ENC_RUN_FRAME_ARG *runArg )
{
	int val, reason;
	unsigned int rotMirMode;
	VpuEncInfo *pInfo = &handle->codecInfo.encInfo;
	EncJpegInfo *pJpgInfo = &pInfo->EncCodecParam.jpgEncInfo;
	int yuv_format = pJpgInfo->format;
	unsigned int mapEnable;
	int stride = runArg->inImgBuffer.luStride;

	NX_DbgMsg( 0, ("Jpeg Encode Info : Rotate = %d, Mirror = %d\n", pJpgInfo->rotationAngle, pJpgInfo->mirrorDirection) );

	pJpgInfo->alignedWidth = ((pInfo->srcWidth+15)/16)*16;
	pJpgInfo->alignedHeight = ((pInfo->srcHeight+15)/16)*16;;

	VpuWriteReg(MJPEG_BBC_BAS_ADDR_REG, pInfo->strmBufPhyAddr);
	VpuWriteReg(MJPEG_BBC_END_ADDR_REG, pInfo->strmBufPhyAddr+pInfo->strmBufSize);
	VpuWriteReg(MJPEG_BBC_WR_PTR_REG, pInfo->strmBufPhyAddr);
	VpuWriteReg(MJPEG_BBC_RD_PTR_REG, pInfo->strmBufPhyAddr);
	VpuWriteReg(MJPEG_BBC_CUR_POS_REG, 0);
	VpuWriteReg(MJPEG_BBC_DATA_CNT_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
	VpuWriteReg(MJPEG_BBC_EXT_ADDR_REG, pInfo->strmBufPhyAddr);
	VpuWriteReg(MJPEG_BBC_INT_ADDR_REG, 0);

	//JpgEncGbuResetReg
	VpuWriteReg(MJPEG_GBU_BT_PTR_REG, 0);
	VpuWriteReg(MJPEG_GBU_WD_PTR_REG, 0);
	VpuWriteReg(MJPEG_GBU_BBSR_REG, 0);

	VpuWriteReg(MJPEG_GBU_BBER_REG, ((256 / 4) * 2) - 1);
	VpuWriteReg(MJPEG_GBU_BBIR_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
	VpuWriteReg(MJPEG_GBU_BBHR_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte

	VpuWriteReg(MJPEG_PIC_CTRL_REG, 0x18);

	VpuWriteReg(MJPEG_PIC_SIZE_REG, pJpgInfo->alignedWidth<<16 | pJpgInfo->alignedHeight);

	rotMirMode = 0;

	if( pJpgInfo->rotationEnable )
	{
		switch( pJpgInfo->rotationAngle )
		{
		case 90:	rotMirMode |= 0x1; break;
		case 180:	rotMirMode |= 0x2; break;
		case 270:	rotMirMode |= 0x3; break;
		default:	rotMirMode |= 0x0; break;
		}
	}
	if (pJpgInfo->mirrorEnable)
	{
		switch( pJpgInfo->mirrorDirection )
		{
		case MIRDIR_VER:		rotMirMode |= 0x4; break;
		case MIRDIR_HOR:		rotMirMode |= 0x8; break;
		case MIRDIR_HOR_VER:	rotMirMode |= 0xC; break;
		default:				rotMirMode |= 0x0; break;
		}
	}

	if( pJpgInfo->rotationEnable || pJpgInfo->rotationEnable )
	{
		rotMirMode |= 0x10;
	}

	VpuWriteReg(MJPEG_ROT_INFO_REG, rotMirMode);

	if (rotMirMode&0x01)
		yuv_format = (pJpgInfo->format==IMG_FORMAT_422) ? IMG_FORMAT_224 : (pJpgInfo->format==IMG_FORMAT_224) ? IMG_FORMAT_422 : pJpgInfo->format;

	if (yuv_format == IMG_FORMAT_422)
	{
		if (rotMirMode & 1)
			pJpgInfo->compInfo[0] = 6;
		else
			pJpgInfo->compInfo[0] = 9;
	} else if (yuv_format == IMG_FORMAT_224) {
		if (rotMirMode & 1)
			pJpgInfo->compInfo[0] = 9;
		else
			pJpgInfo->compInfo[0] = 6;
	}

	VpuWriteReg(MJPEG_MCU_INFO_REG, pJpgInfo->mcuBlockNum << 16 | pJpgInfo->compNum << 12 
		| pJpgInfo->compInfo[0] << 8 | pJpgInfo->compInfo[1] << 4 | pJpgInfo->compInfo[2]);

	VpuWriteReg(MJPEG_SCL_INFO_REG, 0);
	VpuWriteReg(MJPEG_DPB_CONFIG_REG, VPU_FRAME_BUFFER_ENDIAN << 1 | CBCR_INTERLEAVE);
	VpuWriteReg(MJPEG_RST_INTVAL_REG, pJpgInfo->rstIntval);
	VpuWriteReg(MJPEG_BBC_CTRL_REG, ((VPU_STREAM_ENDIAN& 3) << 1) | 1);

	VpuWriteReg(MJPEG_OP_INFO_REG, pJpgInfo->busReqNum);

	if (!JpgEncLoadHuffTab(pInfo)) {
		return VPU_RET_ERR_PARAM;
	}

	if (!JpgEncLoadQMatTab(pInfo)) {
		return VPU_RET_ERR_PARAM;
	}

	val = 0;	//gdi status
	VpuWriteReg(GDI_CONTROL, 1);
	while(!val) 
		val = VpuReadReg(GDI_STATUS);

	mapEnable = 0;

	VpuWriteReg(GDI_INFO_CONTROL, (mapEnable<<20) | ((pJpgInfo->format & 0x07) << 17) | (CBCR_INTERLEAVE << 16) | stride);
	VpuWriteReg(GDI_INFO_PIC_SIZE, (pJpgInfo->alignedWidth << 16) | pJpgInfo->alignedHeight);

	VpuWriteReg(GDI_INFO_BASE_Y,  runArg->inImgBuffer.luPhyAddr);
	VpuWriteReg(GDI_INFO_BASE_CB, runArg->inImgBuffer.cbPhyAddr);
	VpuWriteReg(GDI_INFO_BASE_CR, runArg->inImgBuffer.crPhyAddr);

	VpuWriteReg(MJPEG_DPB_BASE00_REG, 0);

	VpuWriteReg(GDI_CONTROL, 0);
	VpuWriteReg(GDI_PIC_INIT_HOST, 1);

	VpuWriteReg(MJPEG_PIC_START_REG, 1);

	//	Wait Jpeg Interrupt
	reason = JPU_WaitInterrupt( JPU_ENC_TIMEOUT );

	if( reason == 0 )
	{
		return VPU_RET_ERR_TIMEOUT;
	}
	if( 1 != reason )
	{
		NX_ErrMsg(("JPU Encode Error( reason = 0x%08x)\n", reason));
		return VPU_RET_ERROR;
	}

	//	Post Porcessing
	val = VpuReadReg(MJPEG_PIC_STATUS_REG);
		
	if ((val & 0x4) >> 2) {
		return VPU_RET_ERR_WRONG_SEQ;
	}
		
	if (val != 0)
		VpuWriteReg(MJPEG_PIC_STATUS_REG, val);

	runArg->outStreamAddr = (unsigned char*)pInfo->strmBufVirAddr;
	runArg->outStreamSize = VpuReadReg(MJPEG_BBC_WR_PTR_REG) - pInfo->strmBufPhyAddr;
	VpuWriteReg(MJPEG_BBC_FLUSH_CMD_REG, 0);

	return VPU_RET_OK;
}
