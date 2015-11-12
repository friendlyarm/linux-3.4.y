/*
 * (C) Copyright 2010
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
#ifndef __NAND_ECC_H__
#define __NAND_ECC_H__

#if defined(CONFIG_MTD_NAND_ECC_HW) && defined(CONFIG_MTD_NAND_ECC_BCH)
#error "=============================================="
#error "Select one nand hw ecc or bch ecc !!!"
#error "=============================================="
#endif

#define	ECC_PAGE_SIZE	CFG_NAND_ECC_BYTES
#define	ECC_HW_BITS		CFG_NAND_ECC_BITS
#define	ECC_BCH_BITS	CFG_NAND_ECC_BITS

//------------------------------------------------------------------------------
// Register Bits
#define NX_NFCTRL_NCSENB			(1U<<31)
#define NX_NFCTRL_AUTORESET			(1U<<30)
#define NX_NFCTRL_ECCMODE			(7U<<27)
#define NX_NFCTRL_ECCMODE_S24		(5U<<27)
#define NX_NFCTRL_ECCMODE_60		(7U<<27)
#define NX_NFCTRL_IRQPEND			(1U<<15)
#define NX_NFCTRL_ECCIRQPEND		(1U<<14)
#define NX_NFCTRL_ECCRST			(1U<<11)
#define NX_NFCTRL_RNB				(1U<< 9)
#define NX_NFCTRL_IRQENB			(1U<< 8)
#define NX_NFCTRL_ECCIRQENB			(1U<< 7)
#define NX_NFCTRL_HWBOOT_W			(1U<< 6)
#define NX_NFCTRL_EXSEL_R			(1U<< 6)
#define NX_NFCTRL_EXSEL_W			(1U<< 5)
#define NX_NFCTRL_BANK				(3U<< 0)

#define NX_NFACTRL_SYN				(1U<< 1)		// 0: auto mode 1: cpu mode
#define NX_NFACTRL_ELP				(1U<< 0)		// 0: auto mode 1: cpu mode

#define NX_NFECCSTATUS_ELPERR		(0x7FU<< 16)	// 7bit (16, 17, 18, 19, 20, 21, 22)
#define NX_NFECCSTATUS_NCORRECTABLE	(1U<<11)
#define NX_NFECCSTATUS_NUMERR		(0x7FU<< 4)		// 7bit (4, 5, 6, 7, 8, 9, 10)
#define NX_NFECCSTATUS_ERROR		(1U<< 3)
#define NX_NFECCSTATUS_BUSY			(1U<< 2)
#define NX_NFECCSTATUS_DECDONE		(1U<< 1)
#define NX_NFECCSTATUS_ENCDONE		(1U<< 0)

#define NX_NFECCCTRL_RUNECC_W		28	// ecc start
#define NX_NFECCCTRL_DECMODE_R		28
#define NX_NFECCCTRL_DECMODE_W		26	// 0: encoder 1: decoder
#define NX_NFECCCTRL_ELPLOAD		27	// set after elp registers
#define NX_NFECCCTRL_ERRNUM			25
#define NX_NF_ENCODE			0
#define NX_NF_DECODE			1
#define NX_NFECCCTRL_ZEROPAD		25
#define NX_NFECCCTRL_ELPNUM			18	// number of elp (0x7F)
#define NX_NFECCCTRL_PDATACNT		10	// number of parity bit (0xFF)
#define NX_NFECCCTRL_DATACNT		0	// nand data count value(write) (0x3FF)

//#define NXP_NAND_PROFILE

struct nxp_nand {
	struct mtd_info mtd;
	struct nand_chip chip;

	struct platform_device *pdev;

	unsigned int irq;
	int irqcond;
	int eccmode;
	spinlock_t cmdlock;
#ifdef CONFIG_NAND_RANDOMIZER
	uint8_t *randomize_buf;
	uint32_t nowpage;
	uint32_t pages_per_block_mask;
#endif
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	uint8_t *verify_page;
#endif
};

#define mtd_to_nxp(m)			container_of(m, struct nxp_nand, mtd)

#ifdef CFG_NAND_ECCIRQ_MODE
uint32_t wait_for_location_done(struct mtd_info *mtd);
#endif

#if defined(CONFIG_MTD_NAND_ECC_HW)
int nand_hw_ecc_init_device (struct mtd_info *nand);
int nand_hw_ecc_fini_device (struct mtd_info *nand);
int nand_ecc_layout_hwecc(struct mtd_info *mtd);
#endif

#endif /* __NAND_ECC_H__ */
