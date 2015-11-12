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
//	Module		: WDT
//	File			: nx_wdt.h
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120814 first implementation
//------------------------------------------------------------------------------
#ifndef _NX_WDT_H
#define _NX_WDT_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @brief  WDT register set structure
//------------------------------------------------------------------------------
struct NX_WDT_RegisterSet
{
	volatile U32 WTCON;		// 0x0000	// R/W	// control register
	volatile U32 WTDAT;		// 0x0004	// R/W	// data register
	volatile U32 WTCNT;		// 0x0008	// R/W	// count register
	volatile U32 WTCLRINT;	// 0x000C	// W	// interrupt clear register
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------
typedef enum
{
	WDT_CLOCK_DIV16	 = 0,
	WDT_CLOCK_DIV32  = 1,
	WDT_CLOCK_DIV64  = 2,
	WDT_CLOCK_DIV128 = 3
	
} NX_WDT_CLOCK_DIV;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_WDT_Initialize( void );
U32		NX_WDT_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_WDT_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_WDT_GetSizeOfRegisterSet( void );
void	NX_WDT_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_WDT_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_WDT_OpenModule( U32 ModuleIndex );
CBOOL	NX_WDT_CloseModule( U32 ModuleIndex );
CBOOL	NX_WDT_CheckBusy( U32 ModuleIndex );
CBOOL	NX_WDT_CanPowerDown( U32 ModuleIndex );
U32		NX_WDT_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
U32		NX_WDT_GetNumberOfReset( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_WDT_GetInterruptNumber( U32 ModuleIndex );

void	NX_WDT_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_WDT_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
//CBOOL	NX_WDT_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_WDT_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

//void	NX_WDT_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
//CBOOL	NX_WDT_GetInterruptEnableAll( U32 ModuleIndex );
//CBOOL	NX_WDT_GetInterruptPendingAll( U32 ModuleIndex );
//void	NX_WDT_ClearInterruptPendingAll( U32 ModuleIndex );

//U32		NX_WDT_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
//U32		NX_WDT_GetDMAIndex( U32 ModuleIndex );
//U32		NX_WDT_GetDMABusWidth( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_WDT_SetPrescaler( U32 ModuleIndex, U8 Prescaler );
U8		NX_WDT_GetPrescaler( U32 ModuleIndex );
void				NX_WDT_SetClockDivide( U32 ModuleIndex, NX_WDT_CLOCK_DIV ClkSel );
NX_WDT_CLOCK_DIV	NX_WDT_GetClockDivide( U32 ModuleIndex );

void	NX_WDT_SetEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_WDT_GetEnable( U32 ModuleIndex );
void	NX_WDT_SetResetEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_WDT_GetResetEnable( U32 ModuleIndex );

void	NX_WDT_SetReloadCount( U32 ModuleIndex, U16 ReloadData );
U16		NX_WDT_GetReloadCount( U32 ModuleIndex );
void	NX_WDT_SetCurrentCount( U32 ModuleIndex, U16 CurData );
U16		NX_WDT_GetCurrentCount( U32 ModuleIndex );
	
//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_WDT_H
