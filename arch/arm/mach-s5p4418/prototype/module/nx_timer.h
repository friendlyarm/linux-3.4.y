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
//	Module		: TIMER Timer
//	File		: nx_timer.h
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120911 first implementation
//				  20121130 NUMBER_OF_TIMER(TIMER)_MODULE -> number of timer(timer) instances in SOCP
//					 	   NUMBER_TIMERTIMER_SUBMODULE   -> number of submodules in TIMERTIMER IP
//------------------------------------------------------------------------------
#ifndef _NX_TIMER_H
#define _NX_TIMER_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
//  defines
//------------------------------------------------------------------------------
#define NUMBER_TIMERTIMER_SUBMODULE	4

//------------------------------------------------------------------------------
/// @brief  TIMER register set structure
//------------------------------------------------------------------------------
struct NX_TIMER_RegisterSet
{
	volatile U32 TCFG0		;	// 0x00	// R/W	// Timer Configuration Register 0 that configures the two 8-bit Prescaler and DeadZone Length
	volatile U32 TCFG1		;	// 0x04	// R/W	// Timer Configuration Register 1 that controls 5 MUX and DMA Mode Select Bit
	volatile U32 TCON       ;	// 0x08	// R/W	// Timer Control Register
	volatile U32 TCNTB0     ;	// 0x0C	// R/W	// Timer 0 Count Buffer Register
	volatile U32 TCMPB0     ;	// 0x10	// R/W	// Timer 0 Compare Buffer Register
	volatile U32 TCNTO0     ;	// 0x14	// R	// Timer 0 Count Observation Register
 	volatile U32 TCNTB1     ;	// 0x18	// R/W	// Timer 1 Count Buffer Register
	volatile U32 TCMPB1     ;	// 0x1C	// R/W	// Timer 1 Compare Buffer Register
	volatile U32 TCNTO1     ;	// 0x20	// R	// Timer 1 Count Observation Register
	volatile U32 TCNTB2     ;	// 0x24	// R/W	// Timer 2 Count Buffer Register
	volatile U32 TCMPB2     ;	// 0x28	// R/W	// Timer 2 Compare Buffer Register
	volatile U32 TCNTO2     ;	// 0x2C	// R	// Timer 2 Count Observation Register
	volatile U32 TCNTB3     ;	// 0x30	// R/W	// Timer 3 Count Buffer Register
	volatile U32 TCMPB3     ;	// 0x34	// R/W	// Timer 3 Compare Buffer Register
	volatile U32 TCNTO3     ;	// 0x38	// R	// Timer 3 Count Observation Register
	volatile U32 TCNTB4     ;	// 0x3C	// R/W	// Timer 4 Count Buffer Register
	volatile U32 TCNTO4     ;	// 0x40	// R	// Timer 4 Count Observation Register
	volatile U32 TINT_CSTAT ;	// 0x44	// R/W	// Timer Interrupt Control and Status Register
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------
enum
{
	TIMER_TCFG0_PRESCALE0	= 0,
	TIMER_TCFG0_PRESCALE1	= 8,
	TIMER_TCFG0_DEADZONE	= 16
};

enum
{
	TIMER_TCFG1_DMAMODE		= 20,
	TIMER_TCFG1_MUX4		= 16,
	TIMER_TCFG1_MUX3		= 12,
	TIMER_TCFG1_MUX2		= 8,
	TIMER_TCFG1_MUX1		= 4,
	TIMER_TCFG1_MUX0		= 0
};

enum
{
	TIMER_TCON_DEADZONEENB	= 4,
	TIMER_TCON_AOTORELOAD	= 3,
	TIMER_TCON_OUTINVERTER	= 2,
	TIMER_TCON_MANUALUPDATE	= 1,
	TIMER_TCON_STARTSTOP	= 0
};

enum
{
	TIMER_TCON_TIMER4	= 20,
	TIMER_TCON_TIMER3	= 16,
	TIMER_TCON_TIMER2	= 12,
	TIMER_TCON_TIMER1	= 8,
	TIMER_TCON_TIMER0	= 0
};

enum
{
	PWM_DIV_TCLK		= 5,
	TIMER_DIV_16		= 4,
	TIMER_DIV_8			= 3,
	TIMER_DIV_4			= 2,
	TIMER_DIV_2			= 1,
	TIMER_DIV_1			= 0
};

enum
{
	TIMER_DMA_INT4		= 5,
	TIMER_DMA_INT3		= 4,
	TIMER_DMA_INT2		= 3,
	TIMER_DMA_INT1		= 2,
	TIMER_DMA_INT0		= 1,
	TIMER_DMA_NOSEL		= 0
};



//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TIMER_Initialize( void );
U32		NX_TIMER_GetNumberOfModule( void );
U32		NX_TIMER_GetNumberOfSubModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TIMER_GetPhysicalAddress( U32 ModuleIndex );
//U32		NX_TIMER_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
//U32		NX_TIMER_GetNumberOfReset( void );
U32		NX_TIMER_GetSizeOfRegisterSet( void );
void	NX_TIMER_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress );
U32		NX_TIMER_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_TIMER_OpenModule( U32 ModuleIndex );
CBOOL	NX_TIMER_CloseModule( U32 ModuleIndex );
CBOOL	NX_TIMER_CheckBusy( U32 ModuleIndex );
CBOOL	NX_TIMER_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_TIMER_GetClockNumber ( U32 ModuleIndex, U32 SubModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_TIMER_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_TIMER_GetInterruptNumber( U32 ModuleIndex, U32 SubModuleIndex );

void	NX_TIMER_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_TIMER_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_TIMER_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_TIMER_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

void	NX_TIMER_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_TIMER_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_TIMER_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_TIMER_ClearInterruptPendingAll( U32 ModuleIndex );

U32		NX_TIMER_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
//U32		NX_TIMER_GetDMAIndex( U32 ModuleIndex );
//U32		NX_TIMER_GetDMABusWidth( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_TIMER_SetTCFG0( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCFG0( U32 ModuleIndex );

void	NX_TIMER_SetTCFG1( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCFG1( U32 ModuleIndex );

//void	NX_TIMER_SetTCON( U32 ModuleIndex, U32 value );
void	NX_TIMER_SetTCON( U32 ModuleIndex, U32 SubModuleIndex, U32 value );
U32		NX_TIMER_GetTCON( U32 ModuleIndex );

////////////////////////////////////////////////////////////////
void	NX_TIMER_SetTCNTB0( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCNTB0( U32 ModuleIndex );

void	NX_TIMER_SetTCMPB0( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCMPB0( U32 ModuleIndex );

U32		NX_TIMER_GetTCNTO0( U32 ModuleIndex );

////////////////////////////////////////////////////////////////
void	NX_TIMER_SetTCNTB1( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCNTB1( U32 ModuleIndex );

void	NX_TIMER_SetTCMPB1( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCMPB1( U32 ModuleIndex );

U32		NX_TIMER_GetTCNTO1( U32 ModuleIndex );

////////////////////////////////////////////////////////////////
void	NX_TIMER_SetTCNTB2( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCNTB2( U32 ModuleIndex );

void	NX_TIMER_SetTCMPB2( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCMPB2( U32 ModuleIndex );

U32		NX_TIMER_GetTCNTO2( U32 ModuleIndex );

////////////////////////////////////////////////////////////////
void	NX_TIMER_SetTCNTB3( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCNTB3( U32 ModuleIndex );

void	NX_TIMER_SetTCMPB3( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCMPB3( U32 ModuleIndex );

U32		NX_TIMER_GetTCNTO3( U32 ModuleIndex );

////////////////////////////////////////////////////////////////
void	NX_TIMER_SetTCNTB4( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTCNTB4( U32 ModuleIndex );

U32		NX_TIMER_GetTCNTO4( U32 ModuleIndex );

////////////////////////////////////////////////////////////////
void	NX_TIMER_SetTINT_CSTAT( U32 ModuleIndex, U32 value );
U32		NX_TIMER_GetTINT_CSTAT( U32 ModuleIndex );
//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_TIMER_H
