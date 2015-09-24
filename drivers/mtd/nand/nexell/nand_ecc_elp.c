/*
 * (C) Copyright 2009
 * KOO Bon-Gyu, Nexell Co, <freestyle@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#define	U_BOOT_NAND		(0)

#if (U_BOOT_NAND)
#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <platform.h>
#else
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include <mach/platform.h>
#include <mach/soc.h>
#endif

#if	(0)
#define DBGOUT(msg...)		printk(msg)
#else
#define DBGOUT(msg...)		do {} while (0)
#endif
#if	(0)
#define ECCERR(msg...)		printk(msg)
#else
#define ECCERR(msg...)		do {} while (0)
#endif

#define ERROUT(msg...)		{ 					\
		printk(KERN_ERR "ERROR: %s, %s line %d: \n",		\
			__FILE__, __FUNCTION__, __LINE__),	\
		printk(KERN_ERR msg); }


#if defined (CONFIG_MTD_NAND_ECC_HW) || defined (CONFIG_SYS_NAND_HW_ECC)

#define	NAND_READ_RETRY		(1)

#include "nand_ecc.h"

static void NX_NAND_CreateLookupTable(void);
static int NX_NAND_GetErrorLocation(int *pOddSyn, int *pLocation, int *ErrCnt);
static void NX_NAND_SetECCCtrl(void);
static void NX_NAND_SetResetECC(int EccMode);
static void NX_NAND_SetDecMode(int mode);
static void NX_NAND_SetOriECC(unsigned int *pECC, int EccMode);
static void NX_NAND_GetOddSyndrome(int *pSyndrome, int EccMode);
static void NX_NAND_GetErrLoc(U16 *pELoc, int EccMode);

/*
 * u-boot nand hw ecc
 */
static int iNX_BCH_VAR_K	 = 1024*8;			/* (512 * 8) or (1024 *8)*/
static int iNX_BCH_VAR_M	 = 14;			/* 13 or 14 */
static int iNX_BCH_VAR_T	 = ECC_HW_BITS;	/* 4, 8, 12, 16, 24, 40, 60 ... */
static int iNX_BCH_VAR_TMAX  = 24;			/* eccsize == 512 ? 24 : 60 */

static struct NX_MCUS_RegisterSet * const _pNCTRL =
	(struct NX_MCUS_RegisterSet *)IO_ADDRESS(PHY_BASEADDR_MCUSTOP_MODULE);

static void __ecc_reset_decoder(void)
{
	_pNCTRL->NFCONTROL |= NX_NFCTRL_ECCRST;
	// disconnect syndrome path
	_pNCTRL->NFECCAUTOMODE = (_pNCTRL->NFECCAUTOMODE & ~(NX_NFACTRL_ELP)) | NX_NFACTRL_SYN;
}

static void __ecc_decode_enable(int eccsize)	/* 512 or 1024 */
{
	int iNX_BCH_VAR_R = (((iNX_BCH_VAR_M * iNX_BCH_VAR_T)/8) - 1);

	// connect syndrome path
	_pNCTRL->NFECCAUTOMODE = (_pNCTRL->NFECCAUTOMODE & ~(NX_NFACTRL_ELP | NX_NFACTRL_SYN));

	// run ecc
	_pNCTRL->NFECCCTRL =
		(1 << NX_NFECCCTRL_RUNECC_W)   |	   // run ecc
		(0 << NX_NFECCCTRL_ELPLOAD)   |
		(NX_NF_DECODE << NX_NFECCCTRL_DECMODE_W)	|
		(0 << NX_NFECCCTRL_ZEROPAD)	|
		((iNX_BCH_VAR_T & 0x7F) << NX_NFECCCTRL_ELPNUM)		|
		((iNX_BCH_VAR_R & 0xFF) << NX_NFECCCTRL_PDATACNT)	|
		(((eccsize-1) & 0x3FF)  << NX_NFECCCTRL_DATACNT);
}

static void __ecc_write_ecc_decode(unsigned int *ecc, int eccbyte)
{
	volatile U32 *pNFORGECC = _pNCTRL->NFORGECC;
	volatile int i, len = eccbyte>>2;

	/* align 4byte */
	if (eccbyte & ~(4-1))
		len += 1;

	for(i = 0; len > i; i++)
		*pNFORGECC++ = *ecc++;
}

static inline void __ecc_wait_for_decode(void)
{
	while (0 ==(_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_DECDONE));
	{ ; }
}

static inline unsigned int __ecc_decode_error(void)
{
	return (int)(_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_ERROR);
}

static inline void __ecc_start_correct(int eccsize)
{
	int iNX_BCH_VAR_R = (((iNX_BCH_VAR_M * iNX_BCH_VAR_T)/8) - 1);

	// load elp
	_pNCTRL->NFECCCTRL =
		(0 << NX_NFECCCTRL_RUNECC_W)   |
		(1 << NX_NFECCCTRL_ELPLOAD)    |	   // load elp
		(NX_NF_DECODE << NX_NFECCCTRL_DECMODE_W)	|
		(0 << NX_NFECCCTRL_ZEROPAD)	|
 		((iNX_BCH_VAR_T & 0x07F) << NX_NFECCCTRL_ELPNUM )	|
		((iNX_BCH_VAR_R & 0x0FF) << NX_NFECCCTRL_PDATACNT)	|
	 	(((eccsize - 1) & 0x3FF) << NX_NFECCCTRL_DATACNT);
}

#ifdef CFG_NAND_ECCIRQ_MODE
#else
static inline void __ecc_wait_for_correct(void)
{
	while (_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_BUSY)
	{ ; }
}
#endif

static inline int __ecc_get_err_location(unsigned int *pLocation)
{
	volatile U32 *pELoc = _pNCTRL->NFERRLOCATION;
	volatile int len = ECC_HW_BITS/2;
	volatile int err, i;

	// it's not error correctable
	if (((_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_NUMERR) >>  4) !=
		((_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_ELPERR) >> 16))
		return -1;

	for (i = 0; len > i; i++) {
		register U32 regvalue = *pELoc++;
		*pLocation++ = (regvalue>>0  & 0x3FFF)^0x7;
		*pLocation++ = (regvalue>>14 & 0x3FFF)^0x7;
	}

	err = (_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_NUMERR) >> 4;
	return err;
}

static inline void __ecc_setup_encoder(void)
{
	int iNX_BCH_VAR_R = ((((iNX_BCH_VAR_M * iNX_BCH_VAR_T)/8) - 1) & 0xFF);

    NX_MCUS_SetNANDRWDataNum(iNX_BCH_VAR_K / 8);
    NX_MCUS_SetParityCount(((iNX_BCH_VAR_R + 7) / 8) - 1);
    NX_MCUS_SetNumOfELP(iNX_BCH_VAR_T);
}

static inline void __ecc_encode_enable(void)
{
	NX_MCUS_SetNFDecMode(NX_MCUS_DECMODE_ENCODER);
	NX_MCUS_RunECCEncDec();
}

static inline void __ecc_read_ecc_encode(unsigned int *ecc, int eccbyte)
{
	volatile U32 *pNFECC = _pNCTRL->NFECC;
	volatile int i, len = eccbyte>>2;

	/* align 4byte */
	if (eccbyte & ~(4-1))
		len += 1;

	for(i = 0; len > i; i++)
		*ecc++ = *pNFECC++;
}

static inline void __ecc_wait_for_encode(void)
{
	while ( 0==(_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_ENCDONE) )
	{ ; }
}

/*
 * u-boot nand hw ecc interface
 */

/* ECC related defines */
#define	ECC_HW_MAX_BYTES		((106/32)*32 + 32) 	/* 128 */

static struct nand_ecclayout nand_ecc_oob = {
	.eccbytes 	=   0,
	.eccpos 	= { 0, },
	.oobfree 	= { {.offset = 0, .length = 0} }
};


static int nand_sw_ecc_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; len > i; i++)
		if (buf[i] != readb(chip->IO_ADDR_R))
			return -EFAULT;
	return 0;
}


#ifdef _TIME_ELAPSE_

typedef char (*__msg_t)[64];

#define P_TIME_START(A) \
	do { \
			iter = 0; \
			memset(nxp->_page, 0, PAGE_SIZE); \
			A = ktime_get(); \
	} while (0);

#define P_TIME(LOGIC, A, B)	\
	do { \
			B = ktime_get(); \
			sprintf(((__msg_t)(nxp->_page))[iter++], "%20s [%lldus]", LOGIC, ktime_to_us(ktime_sub(B, A))); \
			A = B; \
	} while (0)

#define P_TIME_DONE() \
	do { \
			int i; \
			if ((__p_once++ % 1000)) break; \
			for (i = 0; i < iter; ) { \
				printk("%20s", ((__msg_t)(nxp->_page))[i++]); \
				printk("%20s", ((__msg_t)(nxp->_page))[i++]); \
				printk("%20s", ((__msg_t)(nxp->_page))[i++]); \
				printk("%20s", ((__msg_t)(nxp->_page))[i++]); \
				printk("\n"); \
			} \
	} while (0)
#else

#define P_TIME_START(A)			do {} while (0)
#define P_TIME(LOGIC, A, B)		do {} while (0)
#define P_TIME_DONE()			do {} while (0)

#endif

static uint32_t  eccbuff[ECC_HW_MAX_BYTES/4];
static int errpos[ECC_HW_BITS];

static int nand_hw_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int i, k, n, ret = 0, retry = 0;

	int eccsteps = chip->ecc.steps;
	int eccbytes = chip->ecc.bytes;
	int eccsize  = chip->ecc.size;
	int eccrange = eccsteps * eccsize;

	uint8_t  *ecccode = (uint8_t*)eccbuff;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t  *p = buf;

	int o_syn[ECC_HW_BITS];
	uint32_t *errdat;
	int err = 0, errcnt = 0;
#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
	uint32_t corrected = 0, failed = 0;
	uint32_t max_bitflips = 0;
#endif

#ifdef _TIME_ELAPSE_
	struct nxp_nand *nxp = mtd_to_nxp(mtd);
	ktime_t time_start, time_end;
	int iter;
	static int __p_once = 1;
#endif

	DBGOUT("%s, page=%d, ecc mode=%d, bytes=%d, page=%d, step=%d\n",
		__func__, page, ECC_HW_BITS, eccbytes, mtd->writesize, eccsteps);
	do {
		/* reset value */
		eccsteps = chip->ecc.steps;
		p = buf;
#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
		corrected = failed = 0;
#endif

		P_TIME_START(time_start);

		if (512 >= mtd->writesize) {
			chip->ecc.read_oob(mtd, chip, page);
			chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);
		} else {
			chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
			chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
			chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		}


		P_TIME("Read OOB", time_start, time_end);

		for (n = 0; eccsteps; eccsteps--, p += eccsize) {

			P_TIME("OOB COPY", time_start, time_end);

			for (i = 0; i < eccbytes; i++, n++)
				ecccode[i] = chip->oob_poi[eccpos[n]];

			/* set hw ecc */
			/* write ecc data to orignal ecc register */
			NX_MCUS_SetNANDECCAutoMode(CFALSE, CTRUE);

 			NX_NAND_SetDecMode(NX_MCUS_DECMODE_DECODER);
			NX_NAND_SetECCCtrl();
  			NX_NAND_SetOriECC((uint32_t*)ecccode, ECC_HW_BITS);

	    	NX_MCUS_RunECCEncDec();

			P_TIME("before read_buf", time_start, time_end);
			/* read data */
			chip->read_buf(mtd, p, eccsize);

			P_TIME("after read_buf", time_start, time_end);
			__ecc_wait_for_decode();
			P_TIME("dec done", time_start, time_end);
			err = __ecc_decode_error();

			if (err) {
				/* check erase status */
				for (i = 0 ; eccbytes > i; i++)
					if (0xFF != ecccode[i]) break;
				if (i == eccbytes)
					continue;

				P_TIME("before correct", time_start, time_end);

				/* correct Error */
				NX_NAND_GetOddSyndrome(&o_syn[0], ECC_HW_BITS);
				ret = NX_NAND_GetErrorLocation(&o_syn[0], &errpos[0], &errcnt);
				if (0 > ret) {
					ERROUT("page %d step %2d ecc error, can't %s ...\n",
						page, (chip->ecc.steps-eccsteps), 0==errcnt?"detect":"correct");
#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
					failed++;
#else
					mtd->ecc_stats.failed++;
#endif
					ret = -EBADMSG;
					printk("read retry page %d, retry: %d \n", page, retry);
					goto retry_rd;	/* EXIT */
				} else {
					ECCERR("page %d step %2d, ecc error %2d\n", page, (chip->ecc.steps-eccsteps), errcnt);
					for (k = 0; errcnt > k; k++) {
						errdat = (uint32_t*)p;
						ECCERR("offs = 0x%04x: 0x%4x -> ",
							((chip->ecc.steps-eccsteps)*eccsize)+((errpos[k]/32)*4), errdat[errpos[k]/32]);
						/* Error correct */
						if (errpos[k] >= eccrange) 		/* skip ecc error in oob */
							continue;
						errdat[errpos[k] / 32] ^= 1U<<(errpos[k] % 32);
						ECCERR("0x%4x\n", errdat[errpos[k]/32]);
					}

#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
					#if !(U_BOOT_NAND)
					corrected += errcnt;
					#endif
					max_bitflips = max_t(unsigned int, max_bitflips, errcnt);
#else
					#if !(U_BOOT_NAND)
					mtd->ecc_stats.corrected += errcnt;
					#endif
#endif
				}
			}
		}
		P_TIME("ALL DONE", time_start, time_end);
		P_TIME_DONE();

#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
		mtd->ecc_stats.corrected += corrected;
		if (failed > 0)
			mtd->ecc_stats.failed++;

		DBGOUT("DONE %s, ret=%d\n", __func__, ret);
		return max_bitflips;
#else
		ret = 0;	/* no error */
		DBGOUT("DONE %s, ret=%d\n", __func__, ret);
		return ret;
#endif

retry_rd:
		retry++;
	} while (NAND_READ_RETRY > retry);

#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
	mtd->ecc_stats.corrected += corrected;
	if (failed > 0)
		mtd->ecc_stats.failed++;
#endif

	DBGOUT("FAIL %s, ret=%d, retry=%d\n", __func__, ret, retry);
	return ret;
}

static void nand_hw_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	int i, n;
	int eccsteps = chip->ecc.steps;
	int eccbytes = chip->ecc.bytes;
	int eccsize  = chip->ecc.size;

	uint8_t  *ecccode = (uint8_t*)eccbuff;
	uint32_t *eccpos   = chip->ecc.layout->eccpos;
	uint8_t  *p = (uint8_t *)buf;

	DBGOUT("%s\n", __func__);

    __ecc_setup_encoder();

	/* write data and get ecc */
	for (n = 0; eccsteps; eccsteps--, p += eccsize) {
		__ecc_encode_enable();

		chip->write_buf(mtd, p, eccsize);

		/* get ecc code from ecc register */
		__ecc_wait_for_encode();
		__ecc_read_ecc_encode((uint32_t *)ecccode, eccbytes);

		/* set oob with ecc */
		for (i = 0; i < eccbytes; i++, n++)
			chip->oob_poi[eccpos[n]] = ecccode[i];
	}

	/* write oob */
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

static int nand_hw_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			   const uint8_t *buf, int oob_required, int page, int cached, int raw)
{
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	struct mtd_ecc_stats stats;
	int ret = 0;
#endif
	int status;

	DBGOUT("%s page %d, raw=%d\n", __func__, page, raw);
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	/* not verify */
	if (raw)
		chip->ecc.write_page_raw(mtd, chip, buf, oob_required);
	else
		chip->ecc.write_page(mtd, chip, buf, oob_required);

	/*
	 * Cached progamming disabled for now, Not sure if its worth the
	 * trouble. The speed gain is not very impressive. (2.3->2.6Mib/s)
	 */
	cached = 0;

	if (!cached || !(chip->options & NAND_CACHEPRG)) {

		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
		/*
		 * See if operation failed and additional status checks are
		 * available
		 */
		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
			status = chip->errstat(mtd, chip, FL_WRITING, status,
					       page);

		if (status & NAND_STATUS_FAIL)
			return -EIO;
	} else {
		chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	if (raw)
		return 0;

	stats = mtd->ecc_stats;
	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
	ret = nand_hw_ecc_read_page(mtd, chip, (uint8_t *)buf, oob_required, page);
	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;	// EBADMSG
#endif
	return 0; // mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0
}

int nand_ecc_layout_hwecc(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_ecclayout *layout = chip->ecc.layout;
	struct nand_oobfree *oobfree  = chip->ecc.layout->oobfree;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	int ecctotal = chip->ecc.total;
	int oobsize	 = mtd->oobsize;
	int i = 0, n = 0;
	int ret = 0;

	if (512 > mtd->writesize) {
		printk(KERN_INFO "NAND ecc: page size %d not support hw ecc\n",
			mtd->writesize);
		chip->ecc.mode 			= NAND_ECC_SOFT;
		chip->ecc.read_page 	= NULL;
		chip->ecc.read_subpage 	= NULL;
		chip->ecc.write_page 	= NULL;
		chip->ecc.layout		= NULL;
		chip->verify_buf		= nand_sw_ecc_verify_buf;

		if ( chip->buffers &&
			!(chip->options & NAND_OWN_BUFFERS)) {
			kfree(chip->buffers);
			chip->buffers = NULL;
		}
		ret = nand_scan_tail(mtd);
		printk(KERN_INFO "NAND ecc: Software \n");
		return ret;
	}

	if (ecctotal > oobsize)  {
		printk(KERN_INFO "\n");
		printk(KERN_INFO "==================================================\n");
		printk(KERN_INFO "error: %d bit hw ecc mode requires ecc %d byte	\n", ECC_HW_BITS, ecctotal);
		printk(KERN_INFO "       it's over the oob %d byte for page %d byte	\n", oobsize, mtd->writesize);
		printk(KERN_INFO "==================================================\n");
		printk(KERN_INFO "\n");
		return -EINVAL;
	}

	/*
	 * set ecc layout
	 */
	if (16 >= mtd->oobsize) {
		for (i = 0, n = 0; ecctotal>i; i++, n++) {
			if (5 == n) n += 1;	// Bad marker
			eccpos[i] = n;
		}
		oobfree->offset  = n;
		oobfree->length  = mtd->oobsize - ecctotal - 1;
		layout->oobavail = oobfree->length;

    	mtd->oobavail = oobfree->length;
		printk("hw ecc %2d bit, oob %3d, bad '5', ecc 0~4,6~%d (%d), free %d~%d (%d) ",
			ECC_HW_BITS, oobsize, ecctotal+1-1, ecctotal, oobfree->offset,
			oobfree->offset+oobfree->length-1, oobfree->length);
	} else {

		oobfree->offset  = 2;
		oobfree->length  = mtd->oobsize - ecctotal - 2;
		layout->oobavail = oobfree->length;

		n = oobfree->offset + oobfree->length;
		for (i = 0; i < ecctotal; i++, n++)
			eccpos[i] = n;

    	mtd->oobavail = oobfree->length;
		printk("hw ecc %2d bit, oob %3d, bad '0,1', ecc %d~%d (%d), free 2~%d (%d) ",
			ECC_HW_BITS, oobsize, oobfree->offset+oobfree->length, n-1,
			ecctotal, oobfree->length+2-1, oobfree->length);
	}

	/* must reset mtd */
	mtd->ecclayout = chip->ecc.layout;
	mtd->oobavail  = chip->ecc.layout->oobavail;
	return ret;
}

int nand_hw_ecc_init_device(struct mtd_info *mtd)
{
#ifdef _TIME_ELAPSE_
	struct nxp_nand *nxp = mtd_to_nxp(mtd);
#endif
	struct nand_chip *chip = mtd->priv;
	int eccbyte = 0, eccsize = ECC_PAGE_SIZE, eccidx;
	NX_MCUS_ECCMODE eccmode;

	/*
	 * HW ECC bytes:
	 *
	 *  4 bit ecc need " 4 * 13 =  52" bit (  6.5B) ecc code per  512 Byte
	 *  8 bit ecc need " 8 * 13 = 104" bit ( 13.0B) ecc code per  512 Byte
     * 12 bit ecc need "12 * 13 = 156" bit ( 19.5B) ecc code per  512 Byte
	 * 16 bit ecc need "16 * 13 = 208" bit ( 26.0B) ecc code per  512 Byte
	 * 24 bit ecc need "24 * 13 = 312" bit ( 39.0B) ecc code per  512 Byte
	 * 24 bit ecc need "24 * 14 = 336" bit ( 42.0B) ecc code per 1024 Byte
	 * 40 bit ecc need "40 * 14 = 560" bit ( 70.0B) ecc code per 1024 Byte
	 * 60 bit ecc need "60 * 14 = 840" bit (105.0B) ecc code per 1024 Byte
	 *
	 *  Page  512 Byte +  16 Byte
	 *  Page 2048 Byte +  64 Byte
	 *  Page 4096 Byte + 128 Byte
     *
     *  Page 8192 Byte + 436 Byte (MLC)
	 */
	switch (ECC_HW_BITS) {
	case  4: eccbyte =   7, eccidx = 13, eccmode = NX_MCUS_4BITECC;
			if (512 != eccsize) goto _ecc_fail;
			break;
	case  8: eccbyte =  13, eccidx = 13,  eccmode = NX_MCUS_8BITECC;
			if (512 != eccsize) goto _ecc_fail;
			break;
    case 12: eccbyte =  20, eccidx = 13,  eccmode = NX_MCUS_12BITECC;
    		if (512 != eccsize) goto _ecc_fail;
    		break;
	case 16: eccbyte =  26, eccidx = 13,  eccmode = NX_MCUS_16BITECC;
			if (512 != eccsize) goto _ecc_fail;
			break;
	case 24: 
			if (eccsize == 512)
				eccbyte = 39, eccidx = 13, eccmode = NX_MCUS_24BITECC_512;
			else
				eccbyte = 42, eccidx = 14, eccmode = NX_MCUS_24BITECC;
			break;
	case 40: eccbyte =  70, eccidx = 14,  eccmode = NX_MCUS_40BITECC;
			if (1024 != eccsize) goto _ecc_fail;
			break;
	case 60: eccbyte = 105, eccidx = 14,  eccmode = NX_MCUS_60BITECC;
			if (1024 != eccsize) goto _ecc_fail;
			break;
	default:
		goto _ecc_fail;
		break;
	}

#ifdef _TIME_ELAPSE_
	nxp->_page = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!nxp->_page) {
		printk("page alloc failed\n");
		goto _ecc_fail;
	}
#endif

	iNX_BCH_VAR_K	 = eccsize * 8;	/* fix */
	iNX_BCH_VAR_M	 = eccidx;			/* 13 or 14 */
	iNX_BCH_VAR_T	 = ECC_HW_BITS;	/* 4, 8, 12, 16, 24, 40, 60 ... */
	iNX_BCH_VAR_TMAX = (eccsize == 512 ? 24 : 60);
	DBGOUT("%s ecc %d bit, eccsize=%d, eccbyte=%d, eccindex=%d\n",
		__func__, ECC_HW_BITS, eccsize, eccbyte, eccidx);

	chip->ecc.mode 			= NAND_ECC_HW;
	chip->ecc.size 			= eccsize;			/* per 512 or 1024 bytes */
	chip->ecc.bytes 		= eccbyte;
	chip->ecc.layout		= &nand_ecc_oob;
	chip->ecc.read_page 	= nand_hw_ecc_read_page;
	chip->ecc.write_page 	= nand_hw_ecc_write_page;
	chip->write_page		= nand_hw_write_page;
#ifndef NO_ISSUE_MTD_BITFLIP_PATCH	/* freestyle@2013.09.26 */
	chip->ecc.strength		= eccbyte * 8 / fls (8*eccsize);
#endif

	NX_MCUS_ResetNFECCBlock();
	NX_MCUS_SetECCMode(eccmode);
	NX_NAND_SetECCCtrl();
	NX_NAND_CreateLookupTable();

	return 0;

_ecc_fail:
	printk("Fail: not support ecc %d bits for pagesize %d !!!\n", ECC_HW_BITS, eccsize);
	return -EINVAL;
}
#endif /* CONFIG_MTD_NAND_ECC_HW */


/******************************************************************************
 * BCH variables:
 *-----------------------------------------------------------------------------
 *	k : number of information
 *	m : dimension of Galois field.
 *	t : number of error that can be corrected.
 *	n : length of codeword = 2^m - 1
 *	r : number of parity bit = m * t
 *****************************************************************************/
#if (ECC_HW_BITS >= 24 && ECC_PAGE_SIZE == 1024)
#define NX_BCH_VAR_K		(1024 * 8)
#define NX_BCH_VAR_M		(14)
#define NX_BCH_VAR_T		(ECC_HW_BITS)		/* 24 */

#define NX_BCH_VAR_N		(((1<<NX_BCH_VAR_M)-1))
#define NX_BCH_VAR_R		(NX_BCH_VAR_M * NX_BCH_VAR_T)

#define NX_BCH_VAR_TMAX		(NX_BCH_VAR_T)
#define NX_BCH_VAR_RMAX		(NX_BCH_VAR_M * NX_BCH_VAR_TMAX)

#define NX_BCH_VAR_R32		((NX_BCH_VAR_R   +31)/32)
#define NX_BCH_VAR_RMAX32	((NX_BCH_VAR_RMAX+31)/32)

#define nn_max			    32768	/* Length of codeword, n = 2**m - 1 */

#else
#define NX_BCH_VAR_K		(512 * 8)
#define NX_BCH_VAR_M		(13)
#define NX_BCH_VAR_T		(ECC_HW_BITS)		/* 4, 8, 12, 16 */

#define NX_BCH_VAR_N		(((1<<NX_BCH_VAR_M)-1))
#define NX_BCH_VAR_R		(NX_BCH_VAR_M * NX_BCH_VAR_T)

#define NX_BCH_VAR_TMAX		(NX_BCH_VAR_T)
#define NX_BCH_VAR_RMAX		(NX_BCH_VAR_M * NX_BCH_VAR_TMAX)

#define NX_BCH_VAR_R32		((NX_BCH_VAR_R   +31)/32)
#define NX_BCH_VAR_RMAX32	((NX_BCH_VAR_RMAX+31)/32)

#define nn_max			    8192	/* Length of codeword, n = 2**m - 1 */
#endif

static uint32_t BCH_AlphaToTable[nn_max];
static uint32_t BCH_IndexOfTable[nn_max];
static int elp[(NX_BCH_VAR_TMAX*2)+4][(NX_BCH_VAR_TMAX*2)*4]; 	/* Error locator polynomial (ELP) */
//static int B  [(NX_BCH_VAR_TMAX*2)+1][(NX_BCH_VAR_TMAX*2)*2];	/* Scratch polynomial */

static int L[(NX_BCH_VAR_TMAX*2)+3];			// Degree of ELP
static int u_L[(NX_BCH_VAR_TMAX*2)+3];		// Difference between step number and the degree of ELP
static U16 reg[NX_BCH_VAR_TMAX+3];			// Register state

static int desc[(NX_BCH_VAR_TMAX*2)+4];		// Discrepancy 'mu'th discrepancy

#define NX_BCH_AlphaTo(i)       (BCH_AlphaToTable[i])
#define NX_BCH_IndexOf(i)       (BCH_IndexOfTable[i])

/******************************************************************************
 * Generate GF(2**NX_BCH_VAR_M) from the primitive polynomial p(X) in p[0]..p[NX_BCH_VAR_M]
 * The lookup table looks like:
 * index -> polynomial form:   pAlphaTo[ ] contains j = alpha**i;
 * polynomial form -> index form:  pIndexOf[j = alpha**i] = i
 * pAlphaTo[1] = 2 is the primitive element of GF(2**NX_BCH_VAR_M)
 *****************************************************************************/
static void NX_NAND_CreateLookupTable(void)
{
	int i;
	int mask;	    /* Register states */
	unsigned int p; /* Primitive polynomials */

	unsigned int *   pAlphaTo = &BCH_AlphaToTable[0];
	unsigned int *   pIndexOf = &BCH_IndexOfTable[0];

    if (NX_BCH_VAR_M == 13)	p = 0x25AF;
	if (NX_BCH_VAR_M == 14)	p = 0x41D5;

	/*
     * Galois field implementation with shift registers
	 * Ref: L&C, Chapter 6.7, pp. 217
     */
	mask = 1;
	pAlphaTo[NX_BCH_VAR_M] = 0;
	for (i = 0; i < NX_BCH_VAR_M; i++)	{
		pAlphaTo[i] = mask;
		pIndexOf[pAlphaTo[i]] = i;

		if (p & (1U << i))
			pAlphaTo[NX_BCH_VAR_M] ^= mask;

		mask <<= 1 ;
	}

	pIndexOf[pAlphaTo[NX_BCH_VAR_M]] = NX_BCH_VAR_M;
	mask >>= 1;

	for (i = (NX_BCH_VAR_M + 1); i < NX_BCH_VAR_N; i++) {
		if (pAlphaTo[i - 1] >= mask)
			pAlphaTo[i] = pAlphaTo[NX_BCH_VAR_M]^((pAlphaTo[i - 1]^mask) << 1);
		else
			pAlphaTo[i] = pAlphaTo[i - 1] << 1;

		pIndexOf[pAlphaTo[i]] = i;
	}
	pIndexOf[0] = -1;
}

#define	iNX_BCH_VAR_N	NX_BCH_VAR_N
#define	iNX_BCH_VAR_T	NX_BCH_VAR_T

static int	NX_NAND_GetErrorLocation(int *pOddSyn, int *pLocation, int *ErrCnt)
{
	register int i, j;
#if (1 == NX_BCH_SW_EMUL)
	register int elp_sum ;
#endif
	int count;

	int u;				// u = 'mu' + 1 and u ranges from -1 to 2*t (see L&C)
	int q;				//


	int	s[(NX_BCH_VAR_TMAX*2)+2] = {0 ,};


	for( i=0 ; i<iNX_BCH_VAR_T ; i++ )	s[i*2+1] = pOddSyn[i];

	// Even syndrome = (Odd syndrome) ** 2
	for( i=2; i<=(iNX_BCH_VAR_T*2) ; i+=2)
	{   j=i/2;
		if( s[j] == 0 )		s[i] = 0;
		else	    		s[i] = NX_BCH_AlphaTo( ( 2 * NX_BCH_IndexOf(s[j]) ) % iNX_BCH_VAR_N );
	}

	// initialise table entries
	for (i = 1; i <= (iNX_BCH_VAR_T*2); i++) 	s[i] = NX_BCH_IndexOf(s[i]);

	desc[0] = 0;				/* index form */
	desc[1] = s[1];				/* index form */
	elp[0][0] = 1;				/* polynomial form */
	elp[1][0] = 1;				/* polynomial form */
	for (i = 1; i < (iNX_BCH_VAR_T*2); i++) {
		elp[0][i] = 0;			/* polynomial form */
		elp[1][i] = 0;			/* polynomial form */
	}
	L[0] = 0;
	L[1] = 0;
	u_L[0] = -1;
	u_L[1] = 0;
	u = -1;

	do {
		// even loops always produce no discrepany so they can be skipped
		u = u + 2;
		if (desc[u] == -1) {
			L[u + 2] = L[u];
			for (i = 0; i <= L[u]; i++)
				elp[u + 2][i] = elp[u][i];
		}
		else {
			// search for words with greatest u_L[q] for which desc[q]!=0
			q = u - 2;
			if (q<0) q=0;
			// Look for first non-zero desc[q]
			while ((desc[q] == -1) && (q > 0))
				q=q-2;
			if (q < 0) q = 0;

			// Find q such that desc[u]!=0 and u_L[q] is maximum
			if (q > 0) {
				j = q;
			  	do {
			    		j=j-2;
					if (j < 0) j = 0;
			    		if ((desc[j] != -1) && (u_L[q] < u_L[j]))
			      			q = j;
			  	} while (j > 0);
			}

			// store degree of new elp polynomial
			if (L[u] > L[q] + u - q)
				L[u + 2] = L[u];
			else
				L[u + 2] = L[q] + u - q;

			// Form new elp(x)
			for (i = 0; i < (iNX_BCH_VAR_T*2); i++)
				elp[u + 2][i] = 0;
			for (i = 0; i <= L[q]; i++)
				if (elp[q][i] != 0)
					elp[u + 2][i + u - q] = NX_BCH_AlphaTo((desc[u] + iNX_BCH_VAR_N - desc[q] + NX_BCH_IndexOf(elp[q][i])) % iNX_BCH_VAR_N);
			for (i = 0; i <= L[u]; i++)
				elp[u + 2][i] ^= elp[u][i];

		}
		u_L[u + 2] = u+1 - L[u + 2];

		// Form (u+2)th discrepancy.  No discrepancy computed on last iteration
		if (u < (iNX_BCH_VAR_T*2)) {
			if (s[u + 2] != -1)
				desc[u + 2] = NX_BCH_AlphaTo(s[u + 2]);
			else
				desc[u + 2] = 0;

			for (i = 1; i <= L[u + 2]; i++)
				if ((s[u + 2 - i] != -1) && (elp[u + 2][i] != 0))
		        		desc[u + 2] ^= NX_BCH_AlphaTo((s[u + 2 - i] + NX_BCH_IndexOf(elp[u + 2][i])) % iNX_BCH_VAR_N);
		 	// put desc[u+2] into index form
			desc[u + 2] = NX_BCH_IndexOf(desc[u + 2]);

		}

	} while ((u < ((iNX_BCH_VAR_T*2)-1)) && (L[u + 2] <= iNX_BCH_VAR_T));
	u=u+2;
	L[(iNX_BCH_VAR_T*2)-1] = L[u];

	for( i=0 ; i<=iNX_BCH_VAR_T ; i++ )
	{
		reg[i] = 0 ;
	}
	// Chien's search to find roots of the error location polynomial
	// Ref: L&C pp.216, Fig.6.1
//	NX_TRACE(("ELP:%d\n", L[(iNX_BCH_VAR_T*2)-1]));
	for( i=1 ; i<=L[(iNX_BCH_VAR_T*2)-1] ; i++ )
	{
		reg[i] = (U16)elp[u][i];
//		NX_TRACE(("%04X ", reg[i]));
	}


#if (0 == NX_BCH_SW_EMUL)
		NX_MCUS_SetNFDecMode( NX_MCUS_DECMODE_ENCODER );
		NX_MCUS_SetNumOfELP(NX_BCH_VAR_T);
		NX_MCUS_SetParityCount((NX_BCH_VAR_R+7)/8-1);
		NX_MCUS_SetNANDRWDataNum(NX_BCH_VAR_K/8);

		if(4 == NX_BCH_VAR_T)
			NX_MCUS_SetELP4((U16 *)&reg[1]);
		else if(8 == NX_BCH_VAR_T)
			NX_MCUS_SetELP8((U16 *)&reg[1]);
		else if(12 == NX_BCH_VAR_T)
			NX_MCUS_SetELP12((U16 *)&reg[1]);
		else if(16 == NX_BCH_VAR_T)
			NX_MCUS_SetELP16((U16 *)&reg[1]);
		else if(24 == NX_BCH_VAR_T)
			NX_MCUS_SetELP24((U16 *)&reg[1]);
		else if(40 == NX_BCH_VAR_T)
			NX_MCUS_SetELP((U16 *)&reg[1], NX_BCH_VAR_T);
		else if(60 == NX_BCH_VAR_T)
			NX_MCUS_SetELP((U16 *)&reg[1], NX_BCH_VAR_T);

		NX_MCUS_SetNANDECCAutoMode(CFALSE, CFALSE); /* no UseCPUELP, no UseCPUSYNDROM */

		NX_MCUS_LoadELP();

        /* run and check find error location done */
		while( CTRUE == NX_MCUS_IsNFECCBusy() );

		count = NX_MCUS_GetNumOfError();    /* get error count */

        /* Number of roots = degree of elp hence <= NX_BCH_VAR_T errors */
		if( count == L[(iNX_BCH_VAR_T*2) - 1] ) {
			U16 pELoc[NX_BCH_VAR_T];
			NX_NAND_GetErrLoc(pELoc, NX_BCH_VAR_T);

			for( i=0; i<count; i++ ) {
				pLocation[i] = pELoc[i]^0x7;
			}

		    if (ErrCnt)
			    *ErrCnt = count;

            return 0;
		}
        /* Number of roots != degree of ELP => > NX_BCH_VAR_T errors and cannot solve */
		else {
		    if (ErrCnt)
			    *ErrCnt = L[(iNX_BCH_VAR_T*2) - 1];

			return -1;
        }

#else /* #if (NX_BCH_SW_EMUL == 1) */
		count = 0;
		for (i = 1; i <= NX_BCH_VAR_N; i++)	{

			elp_sum = 1;
			for (j = 1; j <= L[(iNX_BCH_VAR_T*2) - 1]; j++) {
				if (reg[j] != 0) {
					reg[j] = NX_BCH_AlphaTo((NX_BCH_IndexOf(reg[j]) + j) % NX_BCH_VAR_N);
					elp_sum ^= reg[j] ;
				}
			}

            /* store root and error location number indices */
			if (!elp_sum) {

				/* Convert error location from systematic form to storage form */
				pLocation[count] = NX_BCH_VAR_N - i;

				if (pLocation[count] >= NX_BCH_VAR_R) {
					/* Data Bit Error */
					pLocation[count] = pLocation[count] - NX_BCH_VAR_R;
					pLocation[count] = (NX_BCH_VAR_K-1) - pLocation[count];
				}
				else {
					/* ECC Error */
					pLocation[count] = pLocation[count] + NX_BCH_VAR_K;
				}

				if (pLocation[count] < 0) {
					if(ErrCnt)
						*ErrCnt = L[(NX_BCH_VAR_T*2)-1];
					return -1;
				}

				pLocation[count] ^= 0x7;
				if( pLocation[count] < 0 ) {
					//printk(("\nL[(NX_BCH_VAR_T*2)-1] = %d\n", L[(iNX_BCH_VAR_T*2)-1] ));
					//printk(("pLocation[%d] = %d\n", count, pLocation[count] ));
					//return -1;
				}
				else {
					count++;
				}
			}
		}

        if(ErrCnt)
            *ErrCnt = L[(iNX_BCH_VAR_T*2) - 1];

        /* Number of roots = degree of pPoly->elp hence <= NX_BCH_VAR_T errors */
        if (count == L[(iNX_BCH_VAR_T*2) - 1])	{
			return 0;
		}
        /* Number of roots != degree of ELP => >NX_BCH_VAR_T errors and cannot solve */
		else {
			return -1;
		}

#endif /* #if (NX_BCH_SW_EMUL == 1) */

}

static void NX_NAND_SetECCCtrl(void)
{
    NX_MCUS_SetNANDRWDataNum(NX_BCH_VAR_K / 8);
    NX_MCUS_SetParityCount(((NX_BCH_VAR_R + 7) / 8) - 1);
    NX_MCUS_SetNumOfELP(NX_BCH_VAR_T);
}

static void NX_NAND_SetResetECC(int EccMode)
{
    NX_MCUS_SetECCMode(EccMode);
}

static void NX_NAND_SetDecMode(int mode)
{
    NX_MCUS_SetNFDecMode(mode);
}

static void NX_NAND_SetOriECC(unsigned int *pECC, int EccMode)
{
    switch (EccMode) {
    case  4: NX_MCUS_SetNFOriginECC4(pECC);  break;
    case  8: NX_MCUS_SetNFOriginECC8(pECC);  break;
    case 12: NX_MCUS_SetNFOriginECC12(pECC); break;
    case 16: NX_MCUS_SetNFOriginECC16(pECC); break;
    case 24: NX_MCUS_SetNFOriginECC24(pECC); break;
    case 40: NX_MCUS_SetNFOriginECC40(pECC); break;
    case 60: NX_MCUS_SetNFOriginECC60(pECC); break;
    default:
        break;
    }
}

static void NX_NAND_GetOddSyndrome(int *pSyndrome, int EccMode)
{
    switch (EccMode) {
    case  4: NX_MCUS_GetNFECCOddSyndrome4((U32*)pSyndrome);  break;
    case  8: NX_MCUS_GetNFECCOddSyndrome8((U32*)pSyndrome);  break;
    case 12: NX_MCUS_GetNFECCOddSyndrome12((U32*)pSyndrome); break;
    case 16: NX_MCUS_GetNFECCOddSyndrome16((U32*)pSyndrome); break;
    case 24: NX_MCUS_GetNFECCOddSyndrome24((U32*)pSyndrome); break;
	case 40:
	case 60:
			 NX_MCUS_GetNFECCOddSyndrome((U32*)pSyndrome, EccMode); break;
    default:
        break;
    }
}

static void NX_NAND_GetErrLoc(U16 *pELoc, int EccMode)
{
	switch (EccMode) {
	case  4: NX_MCUS_GetErrLoc4(pELoc); break;
	case  8: NX_MCUS_GetErrLoc8(pELoc); break;
	case 12: NX_MCUS_GetErrLoc12(pELoc); break;
	case 16: NX_MCUS_GetErrLoc16(pELoc); break;
	case 24: NX_MCUS_GetErrLoc24(pELoc); break;
	case 40: NX_MCUS_GetErrLoc40(pELoc); break;
	case 60: NX_MCUS_GetErrLoc60(pELoc); break;
	default:
		break;
	}
}
