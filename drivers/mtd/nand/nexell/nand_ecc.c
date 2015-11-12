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
#ifdef CONFIG_NAND_RANDOMIZER
#include "nx_randomizer.h"
#endif


/*
 * u-boot nand hw ecc
 */
static int iNX_BCH_VAR_K	 = ECC_PAGE_SIZE;	/* 512 or 1024 */
static int iNX_BCH_VAR_M	 = 14;				/* 13 or 14 */
static int iNX_BCH_VAR_T	 = 60;				/* 4, 8, 12, 16, 24, 40, 60 ... */
static int iNX_BCH_VAR_R	 = 104;				/* (iNX_BCH_VAR_K * iNX_BCH_VAR_M) / 8 - 1 */
static int iNX_BCH_VAR_TMAX  = 60;				/* eccsize == 512 ? 24 : 60 */

static struct NX_MCUS_RegisterSet * const _pNCTRL =
	(struct NX_MCUS_RegisterSet *)IO_ADDRESS(PHY_BASEADDR_MCUSTOP_MODULE);

static void inline __ecc_reset_decoder(void)
{
	_pNCTRL->NFCONTROL |= NX_NFCTRL_ECCRST;
	// disconnect syndrome path
	_pNCTRL->NFECCAUTOMODE = (_pNCTRL->NFECCAUTOMODE & ~(NX_NFACTRL_ELP)) | NX_NFACTRL_SYN;

	dmb();
}

static void __ecc_decode_enable(int eccsize)	/* 512 or 1024 */
{
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

	dmb();
}

static void __ecc_write_ecc_decode(unsigned int *ecc, int eccbyte)
{
	volatile U32 *pNFORGECC = _pNCTRL->NFORGECC;
	volatile int i, len;

	/* align 4byte */
	len = DIV_ROUND_UP(eccbyte, sizeof(U32));

	for(i = 0; len > i; i++)
		*pNFORGECC++ = *ecc++;

	dmb();
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
	// load elp
	_pNCTRL->NFECCCTRL =
		(0 << NX_NFECCCTRL_RUNECC_W)   |
		(1 << NX_NFECCCTRL_ELPLOAD)    |	   // load elp
		(NX_NF_DECODE << NX_NFECCCTRL_DECMODE_W)	|
		(0 << NX_NFECCCTRL_ZEROPAD)	|
 		((iNX_BCH_VAR_T & 0x07F) << NX_NFECCCTRL_ELPNUM )	|
		((iNX_BCH_VAR_R & 0x0FF) << NX_NFECCCTRL_PDATACNT)	|
	 	(((eccsize - 1) & 0x3FF) << NX_NFECCCTRL_DATACNT);

	dmb();
}

#ifdef CFG_NAND_ECCIRQ_MODE
#else
static inline void __ecc_wait_for_correct(void)
{
	while (_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_BUSY)
	{ ; }
}
#endif

static int __ecc_get_err_location(unsigned int *pLocation)
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
    NX_MCUS_SetNANDRWDataNum(iNX_BCH_VAR_K);
    NX_MCUS_SetParityCount(iNX_BCH_VAR_R);
    NX_MCUS_SetNumOfELP(iNX_BCH_VAR_T);

	dmb();
}

static inline void __ecc_encode_enable(void)
{
	NX_MCUS_SetNFDecMode(NX_MCUS_DECMODE_ENCODER);
	NX_MCUS_RunECCEncDec();

	dmb();
}

static inline void __ecc_read_ecc_encode(unsigned int *ecc, int eccbyte)
{
	volatile U32 *pNFECC = _pNCTRL->NFECC;
	volatile int i, len;

	/* align 4byte */
	len = DIV_ROUND_UP(eccbyte, sizeof(U32));

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


#ifdef NXP_NAND_PROFILE

enum prof_type { 
	/* FEATUREs */
	N_READPAGE, N_RANDOMIZE, MAX_FEATURE,
	/* LOGICs */
	N_READOOB, N_READSECT, N_DECWAIT, N_LOCATION, N_CORRECTION, N_LOADELP, MAX_LOGIC
};

const char *nand_logic_str[] = {
	[N_READPAGE]		= "READ PAGE",
	[N_RANDOMIZE]		= "DERANDOMIZE",
	[N_READOOB]			= "READ OOB",
	[N_READSECT]		= "READ SECT",
	[N_DECWAIT]			= "WAIT DECODE",
	[N_LOADELP]			= "LOAD ELP",
	[N_LOCATION]		= "LOCATION",
	[N_CORRECTION]		= "CORRECTION",
};

enum nand_prof_level { L_FEATURE, L_LOGIC };

typedef struct logic_prof {
	int					logic;
	ktime_t				ts;
	ktime_t				te;
	int					open;
} logic_prof_t;

typedef struct nand_prof {
	const char				*prof_name;
	int						nr_prof;
	logic_prof_t			profs[256];
	enum nand_prof_level	prof_level;
} nand_prof_t;

#define __RAW_NAND_PROFILE_INITIALIZER(name)	\
	{											\
		.prof_name = __stringify(name),				\
		.nr_prof = 0								\
	}

#define DEFINE_NAND_PROFILE(x)		nand_prof_t x = (nand_prof_t) __RAW_NAND_PROFILE_INITIALIZER(x)


#define NAND_PROF_INIT(p)		\
	do {							\
		(p)->nr_prof = 0; \
	} while (0)

/* ktime_to_ms, ktime_to_us, ktime_to_ns selection? */
#define NAND_PROF_START(p, l)	\
	do { \
		int nr_prof = (p)->nr_prof++; \
		logic_prof_t *lp = &((p)->profs)[nr_prof]; \
		lp->logic = l; \
		lp->ts = ktime_get(); \
		lp->open = 1; \
	} while (0)

#define NAND_PROF_END(p, l)	\
	do { \
		int i; \
		logic_prof_t *lp; \
		for (i = (p)->nr_prof - 1; i >= 0; i--) { \
			lp = &((p)->profs[i]); \
			if (lp->logic == l && lp->open) {\
				lp->te = ktime_get();  \
				lp->open = 0; \
				break; \
			} \
		}\
	} while (0)

#define NAND_PROF_REPORT(p, lvl) \
	do { \
		int i; \
		logic_prof_t *lp; \
		\
		printk ("\t[%s]\n", (p)->prof_name); \
		printk ("---------------------------------------------\n"); \
		for (i = 0; i < (p)->nr_prof; i++) { \
			lp = &((p)->profs[i]); \
			\
			if (lvl == L_FEATURE) { \
				if (lp->logic > MAX_FEATURE ) continue;\
			} \
			if (!lp->open) {\
				printk ("\t\t%20s\t[%lldus]\n", nand_logic_str[lp->logic], \
						ktime_to_us(ktime_sub(lp->te, lp->ts))); \
			} \
		} \
		(p)->nr_prof = 0; \
	} while (0)
#else

#define NAND_PROF_INIT(p)					do {} while (0)
#define NAND_PROF_START(p, l)				do {} while (0)
#define NAND_PROF_END(p, l)					do {} while (0)
#define NAND_PROF_REPORT(p, lvl)			do {} while (0)

#endif /* NXP_NAND_PROFILE */

static uint32_t  eccbuff[ECC_HW_MAX_BYTES/4];
static int errpos[ECC_HW_BITS];

#ifdef NXP_NAND_PROFILE
static DEFINE_NAND_PROFILE(nand_read);
static int profile_once = 1;
#endif

static int nand_hw_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int i, k, n, ret = 0, retry = 0;

	int eccsteps = chip->ecc.steps;
	int eccbytes = chip->ecc.bytes;
	int eccsize  = chip->ecc.size;
	int eccrange = 8 * eccsize;

	uint8_t  *ecccode = (uint8_t*)eccbuff;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t  *p = buf;

	uint32_t *errdat;
	int err = 0, errcnt = 0;
	uint32_t corrected = 0, failed = 0;
	uint32_t max_bitflips = 0;
	int is_erasedpage = 0;
	struct nxp_nand *nxp = mtd_to_nxp(mtd);

	NAND_PROF_INIT(&nand_read);

	DBGOUT("%s, page=%d, ecc mode=%d, bytes=%d, page size=%d, step=%d\n",
		__func__, page, ECC_HW_BITS, eccbytes, mtd->writesize, eccsteps);
	do {
		/* reset value */
		eccsteps = chip->ecc.steps;
		p = buf;
		corrected = failed = 0;
		is_erasedpage = 0;

		NAND_PROF_START(&nand_read, N_READPAGE);

		NAND_PROF_START(&nand_read, N_READOOB);
		if (512 >= mtd->writesize) {
			chip->ecc.read_oob(mtd, chip, page);
			chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);
		} else {
			chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
			chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
			chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		}
		NAND_PROF_END(&nand_read, N_READOOB);

		for (n = 0; eccsteps; eccsteps--, p += eccsize) {

			//memset (eccbuff, 0x00, sizeof eccbuff);

			for (i = 0; i < eccbytes; i++, n++)
				ecccode[i] = chip->oob_poi[eccpos[n]];

			/* set hw ecc */
			__ecc_reset_decoder();	/* discon syndrome */
			__ecc_write_ecc_decode((unsigned int*)ecccode, eccbytes);
			__ecc_decode_enable(eccsize);

			/* read data */
			NAND_PROF_START(&nand_read, N_READSECT);
			chip->read_buf(mtd, p, eccsize);
			NAND_PROF_END(&nand_read, N_READSECT);

			NAND_PROF_START(&nand_read, N_DECWAIT);
			__ecc_wait_for_decode();
			NAND_PROF_END(&nand_read, N_DECWAIT);

			err = __ecc_decode_error();
			if (err) {
				/* check erase status */
				for (i = 0 ; eccbytes > i; i++)
					if (0xFF != ecccode[i]) break;
				if (i == eccbytes) {
					is_erasedpage = 1;
					continue;
				}

				__ecc_start_correct(eccsize);

				NAND_PROF_START(&nand_read, N_LOADELP);
#ifdef CFG_NAND_ECCIRQ_MODE
				wait_for_location_done(mtd);
#else
				__ecc_wait_for_correct();
#endif
				NAND_PROF_END(&nand_read, N_LOADELP);

#if (0)
				if (((_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_ELPERR) >>  16) >= chip->ecc.strength) {
					printk ("  over ecc.strength %d, page: %d, step:%d, numerr: %d, elperr: %d\n",
							chip->ecc.strength, page, 
							(chip->ecc.steps-eccsteps),
							((_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_NUMERR) >>  4),
							((_pNCTRL->NFECCSTATUS & NX_NFECCSTATUS_ELPERR) >> 16));
				}
#endif

				/* correct Error */
				NAND_PROF_START(&nand_read, N_LOCATION);
				errcnt = __ecc_get_err_location((unsigned int *)errpos);
				NAND_PROF_END(&nand_read, N_LOCATION);
				if (0 >= errcnt) {
//					ERROUT("page %d step %2d ecc error, can't %s ...\n",
//						page, (chip->ecc.steps-eccsteps), 0==errcnt?"detect":"correct");
					failed++;
					/* The driver should never return -EBADMSG if it failed to read all the requested data - freestyle */
					//ret = -EBADMSG;
					goto retry_rd;	/* EXIT */
				} else {
					NAND_PROF_START(&nand_read, N_CORRECTION);

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

					#if !(U_BOOT_NAND)
					corrected += errcnt;
					#endif
					max_bitflips = max_t(unsigned int, max_bitflips, errcnt);

					NAND_PROF_END(&nand_read, N_CORRECTION);
				}
			}
		}

#ifdef CONFIG_NAND_RANDOMIZER
		if (!is_erasedpage)
		{
			NAND_PROF_START(&nand_read, N_RANDOMIZE);
			randomizer_page (page & nxp->pages_per_block_mask, buf, mtd->writesize);
			NAND_PROF_END(&nand_read, N_RANDOMIZE);
		}
#endif
		/* for managing erased-page bitflip. what about power-cut off fail? */
		if (is_erasedpage)
			memset (buf, 0xff, mtd->writesize);

		NAND_PROF_END(&nand_read, N_READPAGE);

		NAND_PROF_REPORT(&nand_read, L_LOGIC);

		mtd->ecc_stats.corrected += corrected;
		if (failed > 0)
			mtd->ecc_stats.failed++;

		DBGOUT("DONE %s, ret=%d\n", __func__, ret);
		return max_bitflips;

retry_rd:
		retry++;
	} while (NAND_READ_RETRY > retry);

	mtd->ecc_stats.corrected += corrected;
	if (failed > 0)
		mtd->ecc_stats.failed++;

	DBGOUT("FAIL %s, ret=%d, retry=%d\n", __func__, ret, retry);
	return ret;
}

static int nand_hw_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	int i;
	int eccsteps = chip->ecc.steps;
	int eccbytes = chip->ecc.bytes;
	int eccsize  = chip->ecc.size;

	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint32_t *eccpos   = chip->ecc.layout->eccpos;
	uint8_t  *p = (uint8_t *)buf;

	DBGOUT("%s\n", __func__);

    __ecc_setup_encoder();

	/* write data and get ecc */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		__ecc_encode_enable();

		chip->write_buf(mtd, p, eccsize);

		/* get ecc code from ecc register */
		__ecc_wait_for_encode();
		__ecc_read_ecc_encode((uint32_t *)(&ecc_calc[i]), eccbytes);
	}
	/* set oob with ecc */
	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	/* write oob */
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

static int nand_hw_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			   const uint8_t *buf, int oob_required, int page, int cached, int raw)
{
	struct nxp_nand *nxp = mtd_to_nxp(mtd);
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	int ret = 0;
#endif
	int status;
	uint8_t *p = (uint8_t *)buf;

#ifdef CONFIG_NAND_RANDOMIZER
	p = nxp->randomize_buf;
	memcpy (p, buf, mtd->writesize);

	nxp->nowpage = page & nxp->pages_per_block_mask;
	randomizer_page (nxp->nowpage, p, mtd->writesize);
#endif

	DBGOUT("%s page %d, raw=%d\n", __func__, page, raw);
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	/* for hynix H27UBG8T2BTR */
	//ndelay(200);

	/* not verify */
	if (raw)
		chip->ecc.write_page_raw(mtd, chip, buf, oob_required);
	else
		chip->ecc.write_page(mtd, chip, p, oob_required);

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

	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
	ret = chip->ecc.read_page(mtd, chip, (uint8_t *)nxp->verify_page, oob_required, page);
	if (ret < 0)
	{
		ERROUT ("  read page (%d) for write-verify failed!\n", page);
		return -EIO; //		return ret;
	}

	if (memcmp (nxp->verify_page, buf, mtd->writesize))
	{
		ERROUT ("%s fail verify %d page\n", __func__, page);
		return -EIO;
	}

	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
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
	struct nxp_nand *nxp = mtd_to_nxp(mtd);
	struct nand_chip *chip = mtd->priv;
	int eccbyte = 0, eccsize = ECC_PAGE_SIZE, eccidx;
	NX_MCUS_ECCMODE eccmode;


	/* must be resume */
	if (nxp->eccmode)
	{
		NX_MCUS_ResetNFECCBlock();
		NX_MCUS_SetECCMode(nxp->eccmode);

		return 0;
	}
	spin_lock_init (&nxp->cmdlock);

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

	nxp->eccmode = eccmode;

	iNX_BCH_VAR_M			= eccidx;			/* 13 or 14 */
	iNX_BCH_VAR_T			= ECC_HW_BITS;		/* 4, 8, 12, 16, 24, 40, 60 ... */
	iNX_BCH_VAR_R			= DIV_ROUND_UP(iNX_BCH_VAR_M * iNX_BCH_VAR_T, 8) - 1;
	iNX_BCH_VAR_TMAX		= (eccsize == 512 ? 24 : 60);
	DBGOUT("%s ecc %d bit, eccsize=%d, parity=%d, eccbyte=%d, eccindex=%d\n",
		__func__, ECC_HW_BITS, eccsize, iNX_BCH_VAR_R, eccbyte, eccidx);

	chip->ecc.mode 			= NAND_ECC_HW;
	chip->ecc.size 			= eccsize;			/* per 512 or 1024 bytes */
	chip->ecc.bytes 		= eccbyte;
	chip->ecc.layout		= &nand_ecc_oob;
	chip->ecc.read_page 	= nand_hw_ecc_read_page;
	chip->ecc.write_page 	= nand_hw_ecc_write_page;
	chip->write_page		= nand_hw_write_page;
	chip->ecc.strength		= ((eccbyte * 8 / fls (8*eccsize)) * 80 / 100);

	NX_MCUS_ResetNFECCBlock();
	NX_MCUS_SetECCMode(eccmode);

	return 0;

_ecc_fail:
	printk("Fail: not support ecc %d bits for pagesize %d !!!\n", ECC_HW_BITS, eccsize);
	return -EINVAL;
}

int nand_hw_ecc_fini_device(struct mtd_info *mtd)
{
	return 0;
}
#endif /* CONFIG_MTD_NAND_ECC_HW */

