/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.register.h
 * Date         : 2014.07.11
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.11, SD.LEE)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_REGISTER_GLOBAL__
#define NFC_PHY_REGISTER_EXT
#else
#define NFC_PHY_REGISTER_EXT extern
#endif

/******************************************************************************
 * NFC : MCU-S Memory Control Base Register
 ******************************************************************************/
#if defined (__IO_PHYSICAL__)
#define REG_MCUS_BASE               (0xC0051000)
#define REG_NFECC_BASE              (REG_MCUS_BASE+0x0B0)
#define REG_NFORGECC_BASE           (REG_MCUS_BASE+0x11C)
#define REG_NFSYNDROME_BASE         (REG_MCUS_BASE+0x188)
#define REG_NFELP_BASE              (REG_MCUS_BASE+0x200)
#define REG_NFERRLOCATION_BASE      (REG_MCUS_BASE+0x278)
#define REG_WRNFSYNDROME_BASE       (REG_MCUS_BASE+0x2F4)
#define REG_NFC_SHADOW_BASE         (0x2C000000)
#else // __IO_VIRTUAL__
#define REG_MCUS_BASE               (0xF0051000)
#define REG_NFECC_BASE              (REG_MCUS_BASE+0x0B0)
#define REG_NFORGECC_BASE           (REG_MCUS_BASE+0x11C)
#define REG_NFSYNDROME_BASE         (REG_MCUS_BASE+0x188)
#define REG_NFELP_BASE              (REG_MCUS_BASE+0x200)
#define REG_NFERRLOCATION_BASE      (REG_MCUS_BASE+0x278)
#define REG_WRNFSYNDROME_BASE       (REG_MCUS_BASE+0x2F4)
#define REG_NFC_SHADOW_BASE         (0xF0500000)
#endif

#define NFECC_SIZE                  (27)
#define NFORGECC_SIZE               (27)
#define NFSYNDROME_SIZE             (30)
#define NFELP_SIZE                  (30)
#define NFERRLOCATION_SIZE          (30)
#define WRNFSYNDROME_SIZE           (30)

/******************************************************************************
 *
 * All of MCUS
 *
 ******************************************************************************/
typedef struct __MCUS_I__
{
    unsigned int membw;             // Memory Bus Width Register
    unsigned int memtimeacs[2];     // Memory Timing for tACS Register
    unsigned int memtimecos[2];     // Memory Timing for tCOS Register
    unsigned int memtimeacc[4];     // Memory Timing for tACC Register
    unsigned int memtimesacc[4];    // Memory Timing for tSACC Register
    unsigned int memtimewacc[4];    // Memory Timing for tWACC Register
    unsigned int memtimecoh[2];     // Memory Timing for tCOH Register
    unsigned int memtimecah[2];     // Memory Timing for tCAH Register
    unsigned int memburst;          // Memory Burst Control Register
    unsigned int _rsvd0;            // Reserved for future use
    unsigned int memwait;           // Memory Wait Control Register
    unsigned int idedmatimeout;     // DMA Time-out Register
    unsigned int idedmactrl;        // DMA Control Register
    unsigned int idedmapol;         // DMA Polarity Register
    unsigned int idedmatime0;       // DMA Timing 0 Register
    unsigned int idedmatime1;       // DMA Timing 1 Register
    unsigned int idedmatime2;       // DMA Timing 2 Register
    unsigned int idedmatime3;       // DMA Timing 3 Register
    unsigned int idedmarst;         // DMA Reset Register
    unsigned int idedmatime4;       // DMA Timing 4 Register
    unsigned int _rsvd1;            // Reserved for future use.
#define NFCONTROL_NCSENB            (31)
#define NFCONTROL_AUTORESET         (30)
#define NFCONTROL_ECCMODE           (27)
#define NFCONTROL_ECCMODE_S24       (27)
#define NFCONTROL_ECCMODE_60        (27)
#define NFCONTROL_IRQPEND           (15)
#define NFCONTROL_ECCIRQPEND        (14)
#define NFCONTROL_ECCRST            (11)
#define NFCONTROL_RNB               (9)
#define NFCONTROL_IRQENB            (8)
#define NFCONTROL_ECCIRQENB         (7)
#define NFCONTROL_HWBOOT_W          (6)
#define NFCONTROL_EXSEL_R           (6)
#define NFCONTROL_EXSEL_W           (5)
#define NFCONTROL_BANK              (0)
    unsigned int nfcontrol;         // Nand Flash Control Register
#define NFECCCTRL_RUNECC_W          (28)
#define NFECCCTRL_DECMODE_R         (28)
#define NFECCCTRL_DECMODE_W         (26)
#define NFECCCTRL_ELPLOAD           (27)
#define NFECCCTRL_ERRNUM            (25)
#define NF_ENCODE                   (0)
#define NF_DECODE                   (1)
#define NFECCCTRL_ZEROPAD           (25)
#define NFECCCTRL_ELPNUM            (18)
#define NFECCCTRL_PDATACNT          (10)
#define NFECCCTRL_DATACNT           (0)
    unsigned int nfeccctrl;         // Nand ECC Control Register
    unsigned int nfcnt;             // Nand Flash Data Count Register
#define NFECCSTATUS_ELPERR          (16)    // 7bit (16, 17, 18, 19, 20, 21, 22)
#define NFECCSTATUS_NCORRECTABLE    (11)
#define NFECCSTATUS_NUMERR          (4)     // 7bit (4, 5, 6, 7, 8, 9, 10)
#define NFECCSTATUS_ERROR           (3)
#define NFECCSTATUS_BUSY            (2)
#define NFECCSTATUS_DECDONE         (1)
#define NFECCSTATUS_ENCDONE         (0)
    unsigned int nfeccstatus;       // Nand Flash ECC Status Register
    unsigned int nftacs;            // Nand Timing for tACS Register
    unsigned int nftcos;            // Nand Timing for tCOS Register
    unsigned int nftacc;            // Nand Timing for tACC Register
    unsigned int _rsvd2;            // Reserved for future use.
    unsigned int nftoch;            // Nand Timing for tOCH Register
    unsigned int nftcah;            // Nand Timing for tCAH Register
    unsigned int nfecc[27];         // Nand Flash ECC 0 ~ 6 Register
    unsigned int nforgecc[27];      // Nand Flash Origin ECC 0 ~ 6 Register
    unsigned int nfsyndrome[30];    // Nand Flash ECC Syndrome Value 0 ~ 7 Register
    unsigned int nfelp[30];         // Nand Flash ELP Value 0 ~ 11 Register
    unsigned int nferrlocation[30]; // Nand Flash Error Location 0 ~ 11 Register
#define NFECCAUTOMODE_CPUELP        (0)
#define NFECCAUTOMODE_CPUSYND       (1)
    unsigned int nfeccautomode;     // Nand Flash ECC Status Register
    unsigned int wrnfsyndrome[30];  // Nand Flash Error Location 0 ~ 11 Register

} MCUS_I;

typedef struct __NFC_SHADOW_I__
{
    unsigned char  nfdata;     // Base + 0x0000
    unsigned char  _rsvd0[15];
    unsigned char  nfcmd;      // Base + 0x0010
    unsigned char  _rsvd1[7];
    unsigned char  nfaddr;     // Base + 0x0018

} NFC_SHADOW_I;

typedef struct __NFC_SHADOW_I16__
{
    unsigned short nfdata;     // Base + 0x0000
    unsigned char  _rsvd0[14];
    unsigned char  nfcmd;      // Base + 0x0010
    unsigned char  _rsvd1[7];
    unsigned char  nfaddr;     // Base + 0x0018

} NFC_SHADOW_I16;

typedef struct __NFC_SHADOW_I32__
{
    unsigned int   nfdata;     // Base + 0x0000
    unsigned char  _rsvd0[12];
    unsigned char  nfcmd;      // Base + 0x0010
    unsigned char  _rsvd1[7];
    unsigned char  nfaddr;     // Base + 0x0018

} NFC_SHADOW_I32;

NFC_PHY_REGISTER_EXT volatile MCUS_I * nfcI;
NFC_PHY_REGISTER_EXT volatile NFC_SHADOW_I * nfcShadowI;
NFC_PHY_REGISTER_EXT volatile NFC_SHADOW_I16 * nfcShadowI16;
NFC_PHY_REGISTER_EXT volatile NFC_SHADOW_I32 * nfcShadowI32;
