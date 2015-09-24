//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: MCUS
//	File		: nx_mcus.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//		2011.03.10	Hans	add 24bit error correction and elp, etc...
//------------------------------------------------------------------------------
#ifndef __NX_MCUS_H__
#define __NX_MCUS_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	MCUS	Static Memory Control Unit
//------------------------------------------------------------------------------
//@{
#define CS_SIZE (1<<26)
/// @brief	MCUS Module's Register List
struct	NX_MCUS_RegisterSet
{
	volatile U32 MEMBW			;	///< 00h			: Memory Bus Width Register
	volatile U32 MEMTIMEACS[2]	;	///< 04h, 08h		: Memory Timing for tACS Register
	volatile U32 MEMTIMECOS[2]	;	///< 0Ch, 10h		: Memory Timing for tCOS Register
	volatile U32 MEMTIMEACC[4]	;	///< 14h, 18h, 1Ch	: Memory Timing for tACC Register
	volatile U32 MEMTIMESACC[4]	;	///< 20h, 24h, 28h	: Memory Timing for tSACC Register
	volatile U32 MEMTIMEWACC[4]	;	///< 2Ch, 30h, 34h	: Memory Timing for tWACC Register
	volatile U32 MEMTIMECOH[2]	;	///< 38h, 3Ch		: Memory Timing for tCOH Register
	volatile U32 MEMTIMECAH[2]	;	///< 40h, 44h		: Memory Timing for tCAH Register
	volatile U32 MEMBURST		;	///< 48h			: Memory Burst Control Register
	volatile U32 __Reserved1[1]	;	///< 4Ch			: Reserved for future use
	volatile U32 MEMWAIT		;	///< 50h			: Memory Wait Control Register
	volatile U32 IDEDMATIMEOUT	;	///< 54h			: DMA Time-out Register
	volatile U32 IDEDMACTRL		;	///< 58h			: DMA Control Register
	volatile U32 IDEDMAPOL		;	///< 5Ch			: DMA Polarity Register
	volatile U32 IDEDMATIME0	;	///< 60h			: DMA Timing 0 Register
	volatile U32 IDEDMATIME1	;	///< 64h			: DMA Timing 1 Register
	volatile U32 IDEDMATIME2	;	///< 68h			: DMA Timing 2 Register
	volatile U32 IDEDMATIME3	;	///< 6Ch			: DMA Timing 3 Register
	volatile U32 IDEDMARST		;	///< 70h			: DMA Reset Register
	volatile U32 IDEDMATIME4	;	///< 74h			: DMA Timing 4 Register
	volatile U32 __Reserved2[1]	;	///< 78h			: Reserved for future use.
	volatile U32 NFCONTROL		;	///< 7Ch			: Nand Flash Control Register
	volatile U32 NFECCCTRL		;	///<104h			: Nand ECC Control Register
	volatile U32 NFCNT			;	///< B8h			: Nand Flash Data Count Register
	volatile U32 NFECCSTATUS	;	///< BCh			: Nand Flash ECC Status Register
	volatile U32 NFTACS			;	///< ECh			: Nand Timing for tACS Register
	volatile U32 NFTCOS			;	///< F0h			: Nand Timing for tCOS Register
	volatile U32 NFTACC			;	///< F4h			: Nand Timing for tACC Register
	volatile U32 __Reserved4	;	///< F8h			: Reserved for future use.
	volatile U32 NFTOCH			;	///< FCh			: Nand Timing for tOCH Register
	volatile U32 NFTCAH			;	///<100h			: Nand Timing for tCAH Register
	volatile U32 NFECC[27]		;	///< 80h ~ 98h		: Nand Flash ECC 0 ~ 6 Register
	volatile U32 NFORGECC[27]	;	///< 9Ch ~ B4h		: Nand Flash Origin ECC 0 ~ 6 Register
	volatile U32 NFSYNDROME[30]	;	///< C0h ~ DCh		: Nand Flash ECC Syndrome Value 0 ~ 7 Register
	volatile U32 NFELP[30]		;	///<138h ~ 164h		: Nand Flash ELP Value 0 ~ 11 Register
	volatile U32 NFERRLOCATION[30];	///<168h ~ 194h		: Nand Flash Error Location 0 ~ 11 Register
	volatile U32 NFECCAUTOMODE	;	///< BCh			: Nand Flash ECC Status Register
	volatile U32 WRNFSYNDROME[30];	///<168h ~ 194h		: Nand Flash Error Location 0 ~ 11 Register
};

struct	NX_MCUS_NAND_RegisterSet
{
	volatile U32 NAND_DATA	;	///< 00h			: Memory Bus Width Register
	volatile U32 NAND_ADDR	;	///< 00h			: Memory Bus Width Register
	volatile U32 NAND_CMD	;	///< 00h			: Memory Bus Width Register
};

//------------------------------------------------------------------------------
/// @name	enumerators
//@{

///	@brief	Static BUS ID
typedef enum
{
	NX_MCUS_SBUSID_STATIC0		=	0,	///< Static BUS 0
	NX_MCUS_SBUSID_STATIC1		=	1,	///< Static BUS 1
	NX_MCUS_SBUSID_STATIC2		=	2,	///< Static BUS 2
	NX_MCUS_SBUSID_STATIC3		=	3,	///< Static BUS 3
	NX_MCUS_SBUSID_STATIC4		=	4,	///< Static BUS 4
	NX_MCUS_SBUSID_STATIC5		=	5,	///< Static BUS 5
	NX_MCUS_SBUSID_STATIC6		=	6,	///< Static BUS 6
	NX_MCUS_SBUSID_STATIC7		=	7,	///< Static BUS 7
	NX_MCUS_SBUSID_STATIC8		=	8,	///< Static BUS 8
	NX_MCUS_SBUSID_STATIC9		=	9,	///< Static BUS 9
	NX_MCUS_SBUSID_STATIC10		=	10,	///< Static BUS 10
	NX_MCUS_SBUSID_STATIC11		=	11,	///< Static BUS 11
	NX_MCUS_SBUSID_STATIC12		=	12,	///< Static BUS 12
	NX_MCUS_SBUSID_STATIC13		=	13,	///< Static BUS 13
	NX_MCUS_SBUSID_IDE			= 14,	///< IDE
	NX_MCUS_SBUSID_NAND			= 15	///< NAND Flash
} NX_MCUS_SBUSID ;

///	@brief	wait control
typedef enum
{
	NX_MCUS_WAITMODE_DISABLE		= 1,	///< wait control is disabled
	NX_MCUS_WAITMODE_ACTIVEHIGH		= 2,	///< nSWAIT is active high
	NX_MCUS_WAITMODE_ACTIVELOW		= 3		///< nSWAIT is active low
} NX_MCUS_WAITMODE ;

/// @brief	Burst Mode control
typedef enum
{
	NX_MCUS_BURSTMODE_DISABLE		= 0,	///< Disable burst access
	NX_MCUS_BURSTMODE_4BYTEBURST	= 1,	///< 4 byte burst access
	NX_MCUS_BURSTMODE_8BYTEBURST	= 2,	///< 8 byte burst access
	NX_MCUS_BURSTMODE_16BYTEBURST	= 3		///< 16 byte burst access
} NX_MCUS_BURSTMODE ;

///	@brief	signal polarity
typedef enum
{
	NX_MCUS_POLARITY_ACTIVELOW		= 0,	///< active low signal
	NX_MCUS_POLARITY_ACTIVEHIGH		= 1		///< active high signal
} NX_MCUS_POLARITY ;

 /// @brief	Nand Flash address type.
typedef enum
{
	NX_MCUS_NFTYPE_SBADDR3	= 0,	///< Small block 3 address type
	NX_MCUS_NFTYPE_SBADDR4	= 1,	///< Small block 4 address type
	NX_MCUS_NFTYPE_LBADDR4	= 2,	///< Large block 4 address type
	NX_MCUS_NFTYPE_LBADDR5	= 3		///< Large block 5 address type
} NX_MCUS_NFTYPE ;

/// @brief	MCUS ECC encoder decoder mode setting
typedef enum
{
	NX_MCUS_DECMODE_ENCODER	= 0,		///< NAND Flash Encoder mode
	NX_MCUS_DECMODE_DECODER	= 1			///< NAND Flash Decoder mode
} NX_MCUS_DECMODE;

/// @brief	MCUS ECC mode
typedef enum
{
	NX_MCUS_4BITECC		= 0,		///< NAND Flash Encoder mode
	NX_MCUS_8BITECC		= 1,		///< NAND Flash Encoder mode
	NX_MCUS_12BITECC	= 2,		///< NAND Flash Encoder mode
	NX_MCUS_16BITECC	= 3,		///< NAND Flash Encoder mode
	NX_MCUS_24BITECC	= 4,		///< NAND Flash Encoder mode
	NX_MCUS_24BITECC_512= 5,		///< NAND Flash Encoder mode
	NX_MCUS_40BITECC	= 6,		///< NAND Flash Encoder mode
	NX_MCUS_60BITECC	= 7		///< NAND Flash Encoder mode
} NX_MCUS_ECCMODE;

typedef enum
{
	NX_MCUS_8BIT	= 0,		///< NAND Flash Encoder mode
	NX_MCUS_16BIT	= 1		///< NAND Flash Encoder mode
} NX_MCUS_BUSBITS;

/// @brief	MCUS Interrupts for interrupt interfaces
enum	NX_MCUS_INT
{
	NX_MCUS_INT_NAND	= 0		///< NAND Flash Controller RnB
};

//}@

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_MCUS_Initialize( void );
U32		NX_MCUS_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_MCUS_GetPhysicalAddress( void );
U32		NX_MCUS_GetSizeOfRegisterSet( void );
void	NX_MCUS_SetBaseAddress( U32 BaseAddress );
U32		NX_MCUS_GetBaseAddress( void );
CBOOL	NX_MCUS_OpenModule( void );
CBOOL	NX_MCUS_CloseModule( void );
CBOOL	NX_MCUS_CheckBusy( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
S32		NX_MCUS_GetInterruptNumber( void );

void	NX_MCUS_SetInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_MCUS_GetInterruptEnable( S32 IntNum );
CBOOL	NX_MCUS_GetInterruptPending( S32 IntNum );
void	NX_MCUS_ClearInterruptPending( S32 IntNum );

void	NX_MCUS_SetECCInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_MCUS_GetECCInterruptEnable( S32 IntNum );
CBOOL	NX_MCUS_GetECCInterruptPending( S32 IntNum );
void	NX_MCUS_ClearECCInterruptPending( S32 IntNum );

void	NX_MCUS_SetInterruptEnableAll( CBOOL Enable );
#if 0
CBOOL	NX_MCUS_GetInterruptEnableAll( void );
CBOOL	NX_MCUS_GetInterruptPendingAll( void );
#endif
void	NX_MCUS_ClearInterruptPendingAll( void );

void	NX_MCUS_SetInterruptEnable32( U32 EnableFlag );
U32		NX_MCUS_GetInterruptEnable32( void );
U32		NX_MCUS_GetInterruptPending32( void );
void	NX_MCUS_ClearInterruptPending32( U32 PendingFlag );

void	NX_MCUS_SetECCInterruptEnable32 ( U32 EnableFlag );
U32		NX_MCUS_GetECCInterruptEnable32 ( void );
U32		NX_MCUS_GetECCInterruptPending32( void );
void	NX_MCUS_ClearECCInterruptPending32( U32 PendingFlag );

S32		NX_MCUS_GetInterruptPendingNumber( void );	// -1 if None
S32		NX_MCUS_GetECCInterruptPendingNumber( void );
//@}

//------------------------------------------------------------------------------
///	@name	Static BUS Configuration.
//------------------------------------------------------------------------------
//@{
NX_MCUS_SBUSID		NX_MCUS_GetISRAMCS( void );
NX_MCUS_SBUSID		NX_MCUS_GetIROMCS( void );

void	NX_MCUS_SetStaticBUSConfig
	(
		NX_MCUS_SBUSID Id, U32 BitWidth,
		U32 tACS, U32 tCAH, U32 tCOS, U32 tCOH, U32 tACC, U32 tSACC,
		NX_MCUS_WAITMODE WaitMode,	NX_MCUS_BURSTMODE BurstRead, NX_MCUS_BURSTMODE BurstWrite
	);
void	NX_MCUS_GetStaticBUSConfig
	(
		NX_MCUS_SBUSID Id, U32* pBitWidth,
		U32* ptACS, U32* ptCAH, U32* ptCOS, U32* ptCOH, U32* ptACC, U32* ptSACC,
		NX_MCUS_WAITMODE* pWaitMode, NX_MCUS_BURSTMODE* pBurstRead,NX_MCUS_BURSTMODE* pBurstWrite
	);
//@}

//--------------------------------------------------------------------------
///	@name	NAND Flash Memory operations
//--------------------------------------------------------------------------
//@{
void	NX_MCUS_SetNANDBUSConfig
	(
		U32 nNSCSx,
		U32 tACS, U32 tCAH, U32 tCOS, U32 tOCH, U32 tACC
	);
void	NX_MCUS_GetNANDBUSConfig
	(
		U32 nNSCSx,
		U32* ptACS, U32* ptCAH, U32* ptCOS, U32* ptOCH, U32* ptACC
	);

void			NX_MCUS_SetNFType( NX_MCUS_NFTYPE type );
NX_MCUS_NFTYPE	NX_MCUS_GetNFType( void );
// @modified martin nand HW boot function delete
//void	NX_MCUS_SetNFBootEnable( CBOOL bEnb );
//CBOOL	NX_MCUS_GetNFBootEnable( void );
void	NX_MCUS_SetNFEXChannelEnable( CBOOL bEnb );
CBOOL	NX_MCUS_GetNFEXChannelEnable( void );

void	NX_MCUS_SetNFBank( U32 Bank );
U32		NX_MCUS_GetNFBank( void );

void	NX_MCUS_SetNFCSEnable( CBOOL bEnb );
CBOOL	NX_MCUS_GetNFCSEnable( void );

CBOOL	NX_MCUS_IsNFReady ( void );

void	NX_MCUS_SetAutoResetEnable( CBOOL bEnb );
CBOOL	NX_MCUS_GetAutoResetEnable( void );

void	NX_MCUS_ResetNFECCBlock( void );

void	NX_MCUS_SetECCMode( NX_MCUS_ECCMODE EccMode );
U32		NX_MCUS_GetECCMode( void );

U32		NX_MCUS_GetNFWriteDataCount( void );
U32		NX_MCUS_GetNFReadDataCount ( void );

CBOOL	NX_MCUS_IsNFError	( void );
CBOOL	NX_MCUS_IsNFReadEnd ( void );
CBOOL	NX_MCUS_IsNFWriteEnd( void );
CBOOL	NX_MCUS_IsDecodeError(void );
CBOOL	NX_MCUS_IsNFECCBusy(void);

U32		NX_MCUS_GetNumOfError	( void);

void			NX_MCUS_SetNFDecMode( NX_MCUS_DECMODE dMode );
NX_MCUS_DECMODE	NX_MCUS_GetNFDecMode( void );

void	NX_MCUS_LoadELP(void);

void	NX_MCUS_RunECCEncDec(void);

void	NX_MCUS_GetNFECC4 ( U32* pECC );
void	NX_MCUS_GetNFECC8 ( U32* pECC );
void	NX_MCUS_GetNFECC12( U32* pECC );
void	NX_MCUS_GetNFECC16( U32* pECC );
void	NX_MCUS_GetNFECC24( U32* pECC );
void	NX_MCUS_GetNFECC40( U32* pECC );
void	NX_MCUS_GetNFECC60( U32* pECC );

void	NX_MCUS_SetNFOriginECC4 ( U32 *pECC );
void	NX_MCUS_SetNFOriginECC8 ( U32 *pECC );
void	NX_MCUS_SetNFOriginECC12( U32 *pECC );
void	NX_MCUS_SetNFOriginECC16( U32 *pECC );
void	NX_MCUS_SetNFOriginECC24( U32 *pECC );
void	NX_MCUS_SetNFOriginECC40( U32 *pECC );
void	NX_MCUS_SetNFOriginECC60( U32 *pECC );

void	NX_MCUS_GetNFECCOddSyndrome4 ( U32* pSyndrome );
void	NX_MCUS_GetNFECCOddSyndrome8 ( U32* pSyndrome );
void	NX_MCUS_GetNFECCOddSyndrome12( U32* pSyndrome );
void	NX_MCUS_GetNFECCOddSyndrome16( U32* pSyndrome );
void	NX_MCUS_GetNFECCOddSyndrome24( U32* pSyndrome );
void	NX_MCUS_GetNFECCOddSyndrome40( U32* pSyndrome );
void	NX_MCUS_GetNFECCOddSyndrome60( U32* pSyndrome );
void    NX_MCUS_GetNFECCOddSyndrome( U32* pSyndrome, U32 eccbits );

void	NX_MCUS_SetNumOfELP( U32 ELPNum );
U32		NX_MCUS_GetNumOfELP( void );
void	NX_MCUS_SetParityCount( U32 ParityByte );
U32		NX_MCUS_GetParityCount( void );
void	NX_MCUS_SetNANDRWDataNum( U32 DataNum );
U32		NX_MCUS_GetNANDRWDataNum( void );
void	NX_MCUS_SetNANDECCAutoMode(CBOOL UseCPUELP, CBOOL UseCPUSYNDROM);

void	NX_MCUS_SetELP4( U16 *pELP );
void	NX_MCUS_SetELP8( U16 *pELP );
void	NX_MCUS_SetELP12( U16 *pELP );
void	NX_MCUS_SetELP16( U16 *pELP );
void	NX_MCUS_SetELP24( U16 *pELP );
void	NX_MCUS_SetELP40( U16 *pELP );
void	NX_MCUS_SetELP60( U16 *pELP );
void    NX_MCUS_SetELP( U16 *pELP, unsigned int elpbits );

void	NX_MCUS_GetErrLoc4( U16 *pELoc );
void	NX_MCUS_GetErrLoc8( U16 *pELoc );
void	NX_MCUS_GetErrLoc12( U16 *pELoc );
void	NX_MCUS_GetErrLoc16( U16 *pELoc );
void	NX_MCUS_GetErrLoc24( U16 *pELoc );
void	NX_MCUS_GetErrLoc40( U16 *pELoc );
void	NX_MCUS_GetErrLoc60( U16 *pELoc );

void	NX_MCUS_SetPad ( CBOOL Is16bit, CBOOL IsNand, CBOOL IsSDEX, U32 NumberOfChipSelect);

void 	NX_MCUS_WriteNANDAddress	( U8 Address );
void 	NX_MCUS_WriteNANDCommand	( U8 Command );
void 	NX_MCUS_WriteNANDData		( U32 Data );
U32  	NX_MCUS_ReadNANDData		( void );
void 	NX_MCUS_WriteNANDDataBlock	( const U32* pSrcData, U32 Size32 );
void 	NX_MCUS_ReadNANDDataBlock	( U32* pDstData, U32 Size32 );
//void 	NX_MCUS_GetNANDECC(U32& ECC0, U32& ECC1, U32& ECC2, U32& ECC3, U32& ECC4, U32& ECC5, U32& ECC6, U32& ECC7, U32& ECC8, U32& ECC9, U32& ECC10);
//void 	NX_MCUS_GetNANDECC(U32 ECC0, U32 ECC1, U32 ECC2, U32 ECC3, U32 ECC4, U32 ECC5, U32 ECC6, U32 ECC7, U32 ECC8, U32 ECC9, U32 ECC10);
void    NX_MCUS_SetNANDECCCONFIG(CBOOL DECMODE, U32 DATASIZE, U32 PARITYSIZE, U32 ELPNUMBER);
U32 NX_MCUS_GetErrorPos( U32 number );
//@}

//@}

#ifdef	__cplusplus
}
#endif

#endif // __NX_MCUS_H__

