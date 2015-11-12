#include "../base/nx_prototype.h"

//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		: nx_ppm.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_PPM_H__
#define __NX_PPM_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	PPM
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
	volatile U32	PPM_CTRL;					///< 0x00 : Control Register
	volatile U32	__Reserved00;				///< 0x02 : Reserved Regopm
	volatile U32	PPM_STAT;					///< 0x04 : Status Register
	volatile U32	PPM_LOWPERIOD;				///< 0x06 : Low Period Register
	volatile U32	PPM_HIGHPERIOD;				///< 0x08 : High Period Register
} NX_PPM_RegisterSet;

//------------------------------------------------------------------------------
///	@name	PPM Interface
//------------------------------------------------------------------------------
//@{

/// @brief	Set Polarity of Input Signal
typedef enum
{
	NX_PPM_INPUTPOL_INVERT		= 0UL,	///< Input Signal Invert
	NX_PPM_INPUTPOL_BYPASS		= 1UL	///< Input Signal Bypass
}	NX_PPM_INPUTPOL;

void		NX_PPM_SetPPMEnable ( U32 ModuleIndex, CBOOL enable );
CBOOL		NX_PPM_GetPPMEnable ( U32 ModuleIndex );

void			NX_PPM_SetInputSignalPolarity( U32 ModuleIndex, NX_PPM_INPUTPOL polarity );
NX_PPM_INPUTPOL	NX_PPM_GetInputSignalPolarity( U32 ModuleIndex );

CBOOL		NX_PPM_IsHighOverflow( U32 ModuleIndex );
CBOOL		NX_PPM_IsLowOverflow( U32 ModuleIndex );

U32			NX_PPM_GetPPMLowPeriodValue ( U32 ModuleIndex );
U32			NX_PPM_GetPPMHighPeriodValue ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_PPM_Initialize( void );
U32   NX_PPM_GetNumberOfModule( void );

U32   NX_PPM_GetSizeOfRegisterSet( void );
void  NX_PPM_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void* NX_PPM_GetBaseAddress( U32 ModuleIndex );
U32   NX_PPM_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_PPM_OpenModule( U32 ModuleIndex );
CBOOL NX_PPM_CloseModule( U32 ModuleIndex );
CBOOL NX_PPM_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_PPM_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_PPM_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
	NX_PPM_INT_RISEEDGE = 0,	///< Rising edge detect interrupt.
	NX_PPM_INT_FALLEDGE = 1,	///< Falling edge detect interrupt.
	NX_PPM_INT_OVERFLOW = 2		///< Overflow interrupt.
} NX_PPM_INT;
U32   NX_PPM_GetInterruptNumber ( U32 ModuleIndex );
void  NX_PPM_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL NX_PPM_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL NX_PPM_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_PPM_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_PPM_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL NX_PPM_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL NX_PPM_GetInterruptPendingAll( U32 ModuleIndex );
void  NX_PPM_ClearInterruptPendingAll( U32 ModuleIndex );
S32   NX_PPM_GetInterruptPendingNumber( U32 ModuleIndex );
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_PPM_H__
