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
//	Module		: SPDIFRX
//	File		: nx_spdifrx.h
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120830 first implementation
//------------------------------------------------------------------------------
#ifndef _NX_SPDIFRX_H
#define _NX_SPDIFRX_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @brief  SPDIFRX register set structure
//------------------------------------------------------------------------------
struct NX_SPDIFRX_RegisterSet
{
	volatile U32 SPDIF_CTRL		;	// {16'h0, lock, Clr_FIFO, EnbPhaseDet, conf_mode, EnbCapUserStat, DMA_DataOnly, DMA_Swap, conf_sample};
    volatile U32 SPDIF_ENBIRQ	;	// {16'h0, 7'h0, PendLock, PendErr, PendParity, PendBlock, EnbIRQLock, EnbIRQErr, EnbIRQParity, EnbIRQBlock};
    volatile U32 REGUSERA0		;	// RegUserA [ 31:  0]
    volatile U32 REGUSERA1		;	// RegUserA [ 63: 32]
    volatile U32 REGUSERA2		;	// RegUserA [ 95: 64]
    volatile U32 REGUSERA3		;	// RegUserA [127: 96]
    volatile U32 REGUSERA4		;	// RegUserA [159:128]
    volatile U32 REGUSERA5		;	// RegUserA [191:160]
    volatile U32 REGUSERB0		;	// RegUserB [ 31:  0]
    volatile U32 REGUSERB1		;	// RegUserB [ 63: 32]
    volatile U32 REGUSERB2		;	// RegUserB [ 95: 64]
    volatile U32 REGUSERB3		;	// RegUserB [127: 96]
    volatile U32 REGUSERB4		;	// RegUserB [159:128]
    volatile U32 REGUSERB5		;	// RegUserB [191:160]
    volatile U32 REGSTATA0		;	// RegStatA [ 31:  0]
    volatile U32 REGSTATA1		;	// RegStatA [ 63: 32]
    volatile U32 REGSTATA2		;	// RegStatA [ 95: 64]
    volatile U32 REGSTATA3		;	// RegStatA [127: 96]
    volatile U32 REGSTATA4		;	// RegStatA [159:128]
    volatile U32 REGSTATA5		;	// RegStatA [191:160]
    volatile U32 REGSTATB0		;	// RegStatB [ 31:  0]
    volatile U32 REGSTATB1		;	// RegStatB [ 63: 32]
    volatile U32 REGSTATB2		;	// RegStatB [ 95: 64]
    volatile U32 REGSTATB3		;	// RegStatB [127: 96]
    volatile U32 REGSTATB4		;	// RegStatB [159:128]
    volatile U32 REGSTATB5		;	// RegStatB [191:160]
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------
enum
{
	SPDIFRX_CTRL_DECODE_ENB		= 0,
	SPDIFRX_CTRL_DMA_Swap		= 1,
	SPDIFRX_CTRL_DMA_DataOnly	= 2,
	SPDIFRX_CTRL_EnbCapUserStat	= 3,
	SPDIFRX_CTRL_Sample_OFFSET	= 4,	// 4bit
	SPDIFRX_CTRL_EnbPhaseDet	= 8,
	SPDIFRX_CTRL_Clr_FIFO		= 9,
	SPDIFRX_CTRL_lock			= 10,
	FillRegUserInv				= 11
};

enum
{
	SPDIFRX_INTREN_BLOCK	= 1UL<<0,
	SPDIFRX_INTREN_PARITY	= 1UL<<1,
	SPDIFRX_INTREN_ERROR	= 1UL<<2,
	SPDIFRX_INTREN_LOCK		= 1UL<<3,
	SPDIFRX_PEND_BLOCK		= 1UL<<4,
	SPDIFRX_PEND_PARITY		= 1UL<<5,
	SPDIFRX_PEND_ERROR		= 1UL<<6,
	SPDIFRX_PEND_LOCK		= 1UL<<7
};

//18'h0, RdCnt, WrCnt, FIFOFull, FIFOEmpty, DMAACK, DMAREQ
enum
{
	SPDIFRX_STAT_DMAREQ		= 0,
	SPDIFRX_STAT_DMAACK		= 1,
	SPDIFRX_STAT_FIFOEmpty	= 2,
	SPDIFRX_STAT_FIFOFull	= 3,
	SPDIFRX_STAT_WrCnt		= 4,
	SPDIFRX_STAT_RdCnt		= 9
};

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_SPDIFRX_Initialize( void );
U32		NX_SPDIFRX_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_SPDIFRX_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_SPDIFRX_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
U32		NX_SPDIFRX_GetNumberOfReset( void );
U32		NX_SPDIFRX_GetSizeOfRegisterSet( void );
void	NX_SPDIFRX_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress );
U32		NX_SPDIFRX_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_SPDIFRX_OpenModule( U32 ModuleIndex );
CBOOL	NX_SPDIFRX_CloseModule( U32 ModuleIndex );
CBOOL	NX_SPDIFRX_CheckBusy( U32 ModuleIndex );
CBOOL	NX_SPDIFRX_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_SPDIFRX_GetInterruptNumber( U32 ModuleIndex );

void	NX_SPDIFRX_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_SPDIFRX_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_SPDIFRX_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_SPDIFRX_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

void	NX_SPDIFRX_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SPDIFRX_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_SPDIFRX_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_SPDIFRX_ClearInterruptPendingAll( U32 ModuleIndex );

U32		NX_SPDIFRX_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_SPDIFRX_GetDMAIndex( U32 ModuleIndex );
U32		NX_SPDIFRX_GetDMABusWidth( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_SPDIFRX_Set_SPDCTRL( U32 ModuleIndex, U32 value );
U32		NX_SPDIFRX_Get_SPDCTRL( U32 ModuleIndex );

void	NX_SPDIFRX_Set_SPDENBIRQ( U32 ModuleIndex, U32 value );
U32		NX_SPDIFRX_Get_SPDENBIRQ( U32 ModuleIndex );

U32		NX_SPDIFRX_Get_REGUSERA0( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERA1( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERA2( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERA3( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERA4( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERA5( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERB0( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERB1( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERB2( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERB3( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERB4( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGUSERB5( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATA0( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATA1( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATA2( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATA3( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATA4( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATA5( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATB0( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATB1( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATB2( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATB3( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATB4( U32 ModuleIndex );
U32		NX_SPDIFRX_Get_REGSTATB5( U32 ModuleIndex );
//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_SPDIFRX_H
