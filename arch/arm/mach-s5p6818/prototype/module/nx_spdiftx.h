//------------------------------------------------------------------------------
//
//	Copyright (C) 2012 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: SPDIFTX
//	File		: nx_spdiftx.h
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120814 first implementation
//------------------------------------------------------------------------------
#ifndef _NX_SPDIFTX_H
#define _NX_SPDIFTX_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @brief  SPDIFTX register set structure
//------------------------------------------------------------------------------
struct NX_SPDIFTX_RegisterSet
{
	volatile U32 SPDIF_CLKCON;		// 0x0000	// R/W	// clock control register
	volatile U32 SPDIF_CON;			// 0x0004	// R/W	// control register
	volatile U32 SPDIF_BSTAS;		// 0x0008	// R/W	// burst status register
	volatile U32 SPDIF_CSTAS;		// 0x000C	// R/W	// channel status register
	volatile U32 SPDIF_DAT;			// 0x0010	//   W	// SPDIF_IFOUT data buffer
	volatile U32 SPDIF_CNT;			// 0x0014 	//   W	// repetition count register
	volatile U32 SPDIF_BSTAS_SHD;	// 0x0018 	// R	// shadowed burst status register
	volatile U32 SPDIF_CNT_SHD;		// 0x001C 	// R	// shadowed repetition count register
	volatile U32 USERBIT1;			// 0x0020 	// R/W	// sub-code Q1 to Q32
	volatile U32 USERBIT2;			// 0x0024 	// R/W	// sub-code Q33 to Q64
	volatile U32 USERBIT3;			// 0x0028 	// R/W	// sub-code Q65 to Q96
	volatile U32 USERBIT1_SHD;		// 0x002C	// R	// shadowed register userbit1
	volatile U32 USERBIT2_SHD;		// 0x0030	// R	// shadowed register userbit2
	volatile U32 USERBIT3_SHD;		// 0x0034	// R	// shadowed register userbit3
	volatile U32 VERSION_INFO;		// 0x0038	// R	// RTL version information
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------

typedef enum
{
	NX_SPDIFTX_INTERNAL_CLK = 0,
	NX_SPDIFTX_EXTERNAL_CLK = 1
} NX_SPDIFTX_MCLK;;

typedef enum
{
	NX_SPDIFTX_TRIGGER_0DEPTH  = 0,
	NX_SPDIFTX_TRIGGER_1DEPTH  = 1,
	NX_SPDIFTX_TRIGGER_4DEPTH  = 2,
	NX_SPDIFTX_TRIGGER_6DEPTH  = 3,
	NX_SPDIFTX_TRIGGER_10DEPTH = 4,
	NX_SPDIFTX_TRIGGER_12DEPTH = 5,
	NX_SPDIFTX_TRIGGER_14DEPTH = 6,
	NX_SPDIFTX_TRIGGER_15DEPTH	= 7

} NX_SPDIFTX_TRIGGER_LEVEL;

typedef enum
{
	NX_SPDIFTX_MODE_DMA			= 0,
	NX_SPDIFTX_MODE_POLLING		= 1,
	NX_SPDIFTX_MODE_INTERRUPT	= 2
} NX_SPDIFTX_TRANSFER_MODE;

typedef enum
{
	NX_SPDIFTX_SAMPLE_256FS = 0,
	NX_SPDIFTX_SAMPLE_384FS = 1,
	NX_SPDIFTX_SAMPLE_512FS = 2
} NX_SPIDFTX_MCLK_SAMPLE;

typedef enum
{
	NX_SPDIFTX_DATA_BIT_16	= 0,
	NX_SPDIFTX_DATA_BIT_20	= 1,
	NX_SPDIFTX_DATA_BIT_24	= 2
} NX_SPDIFTX_DATA_BIT;

enum
{
	NX_SPDIFTX_FFLV_INTR_EN			= 15,	///< FIFO_level interrupt enable bitnum
	NX_SPDIFTX_USDA_INTR_EN			= 10,	///< User data interrupt enable bitnum
	NX_SPDIFTX_BFEM_INTR_EN			= 8,	///< Buffer enpty interrupt enable bitnum
	NX_SPDIFTX_STED_INTR_EN			= 6		///< Stream end interrupt enable bitnum
};

enum
{
	NX_SPDIFTX_FFLV_INTR_PD			= 16,	///< FIFO_level interrupt pending bitnum
	NX_SPDIFTX_USDA_INTR_PD			= 11,	///< User data interrupt pending bitnum
	NX_SPDIFTX_BFEM_INTR_PD			= 9,	///< Buffer enpty interrupt pending bitnum
	NX_SPDIFTX_STED_INTR_PD			= 7		///< Stream end interrupt pending bitnum
};

typedef enum
{
	NX_SPDIFTX_BIG_ENDIAN			= 0,	///< big_endian
	NX_SPDIFTX_4BYTE_SWAP			= 1,	///< 4byte_swap
	NX_SPDIFTX_3BYTE_SWAP			= 2,	///< 3byte_swap
	NX_SPDIFTX_2BYTE_SWAP			= 3 	///< 2byte_swap
	
} NX_SPDIFTX_SWAP;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_SPDIFTX_Initialize( void );
U32		NX_SPDIFTX_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_SPDIFTX_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_SPDIFTX_GetSizeOfRegisterSet( void );
void	NX_SPDIFTX_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_SPDIFTX_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_SPDIFTX_OpenModule( U32 ModuleIndex );
CBOOL	NX_SPDIFTX_CloseModule( U32 ModuleIndex );
CBOOL	NX_SPDIFTX_CheckBusy( U32 ModuleIndex );
CBOOL	NX_SPDIFTX_CanPowerDown( U32 ModuleIndex );
U32		NX_SPDIFTX_GetNumberOfReset( void );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_SPDIFTX_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32	NX_SPDIFTX_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_SPDIFTX_GetInterruptNumber( U32 ModuleIndex );

void	NX_SPDIFTX_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_SPDIFTX_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_SPDIFTX_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_SPDIFTX_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

void	NX_SPDIFTX_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SPDIFTX_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_SPDIFTX_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_SPDIFTX_ClearInterruptPendingAll( U32 ModuleIndex );

U32		NX_SPDIFTX_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_SPDIFTX_GetDMAIndex( U32 ModuleIndex );
U32		NX_SPDIFTX_GetDMABusWidth( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{

void				NX_SPDIFTX_SetMCLK( U32 ModuleIndex, NX_SPDIFTX_MCLK MainClk );
NX_SPDIFTX_MCLK		NX_SPDIFTX_GetMCLK( U32 ModuleIndex );


CBOOL	NX_SPDIF_GetClockDownReady( U32 ModuleIndex );
void	NX_SPDIFTX_SetPowerOn( U32 ModuleIndex, CBOOL PowerOn );
CBOOL	NX_SPDIFTX_GetPowerOn( U32 ModuleIndex );



CBOOL	NX_SPDIFTX_IsFIFOEmpty(U32 ModuleIndex);
CBOOL	NX_SPDIFTX_IsFIFOFull(U32 ModuleIndex);
U32		NX_SPDIFTX_GetFIFOLVL(U32 ModuleIndex);

void	NX_SPDIFTX_SetTriggerLevel( U32 ModuleIndex, NX_SPDIFTX_TRIGGER_LEVEL TX_RDY_LVL );
NX_SPDIFTX_TRIGGER_LEVEL  	NX_SPDIFTX_GetTriggerLevel( U32 ModuleIndex );

//--------------------------------------------------------------------------
//@{
void	NX_SPDIFTX_SetTransferMode( U32 ModuleIndex, NX_SPDIFTX_TRANSFER_MODE Mode );
NX_SPDIFTX_TRANSFER_MODE  	NX_SPDIFTX_GetTransferMode( U32 ModuleIndex );
void	NX_SPDIFTX_SetEndianFormat( U32 ModuleIndex, NX_SPDIFTX_SWAP Swap );
NX_SPDIFTX_SWAP  NX_SPDIFTX_GetEndianFormat( U32 ModuleIndex );

void	NX_SPDIFTX_SetUserData( U32 ModuleIndex, CBOOL UserData );
CBOOL  NX_SPDIFTX_GetUserData( U32 ModuleIndex );


void	NX_SPDIFTX_SetSoftwareReset( U32 ModuleIndex, CBOOL Enable );
CBOOL  	NX_SPDIFTX_GetSoftwareReset( U32 ModuleIndex );

void					NX_SPDIFTX_SetMCLKSamplingSel( U32 ModuleIndex, NX_SPIDFTX_MCLK_SAMPLE Sample );
NX_SPIDFTX_MCLK_SAMPLE  NX_SPDIFTX_GetMCLKSamplingSel( U32 ModuleIndex );


void	NX_SPDIFTX_SetBitWidth( U32 ModuleIndex, NX_SPDIFTX_DATA_BIT DataBit );
NX_SPDIFTX_DATA_BIT  NX_SPDIFTX_GetBitWidth( U32 ModuleIndex );
void	NX_SPDIFTX_SetTransferType( U32 ModuleIndex, CBOOL Type );
CBOOL  NX_SPDIFTX_GetTransferType( U32 ModuleIndex );
//@}


//--------------------------------------------------------------------------
void	NX_SPDIFTX_SetSPDCLKCON( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetSPDCLKCON( U32 ModuleIndex );

void	NX_SPDIFTX_SetSPDCON( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetSPDCON( U32 ModuleIndex );

void	NX_SPDIFTX_SetSPDBSTAS( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetSPDBSTAS( U32 ModuleIndex );

void	NX_SPDIFTX_SetSPDCSTAS( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetSPDCSTAS( U32 ModuleIndex );

void	NX_SPDIFTX_SetSPDDAT( U32 ModuleIndex, U32 value );

void	NX_SPDIFTX_SetSPDCNT( U32 ModuleIndex, U32 value );

U32		NX_SPDIFTX_GetSPDBSTAS_SHD( U32 ModuleIndex );

U32		NX_SPDIFTX_GetSPDCNT_SHD( U32 ModuleIndex );

void	NX_SPDIFTX_SetUSERBIT1( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetUSERBIT1( U32 ModuleIndex );

void	NX_SPDIFTX_SetUSERBIT2( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetUSERBIT2( U32 ModuleIndex );

void	NX_SPDIFTX_SetUSERBIT3( U32 ModuleIndex, U32 value );
U32		NX_SPDIFTX_GetUSERBIT3( U32 ModuleIndex );

U32		NX_SPDIFTX_GetUSERBIT1_SHD( U32 ModuleIndex );

U32		NX_SPDIFTX_GetUSERBIT2_SHD( U32 ModuleIndex );

U32		NX_SPDIFTX_GetUSERBIT3_SHD( U32 ModuleIndex );

U32		NX_SPDIFTX_GetVERSION_INFO( U32 ModuleIndex );
//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_SPDIFTX_H
