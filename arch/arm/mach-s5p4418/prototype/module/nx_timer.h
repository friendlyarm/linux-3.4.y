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
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
//  defines
//------------------------------------------------------------------------------
#define NUMBER_OF_TIMER_CHANNEL	5
#define NX_TIMER_INT		    4

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
	volatile U8 _Reserved0[0x1000-0x48];	// 0x48~0x1000
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------
typedef enum {
	NX_TIMER_DIVIDSELECT_1	= 0UL,
	NX_TIMER_DIVIDSELECT_2	= 1UL,
	NX_TIMER_DIVIDSELECT_4	= 2UL,
	NX_TIMER_DIVIDSELECT_8	= 3UL,
	NX_TIMER_DIVIDSELECT_16	= 4UL,
	NX_TIMER_DIVIDSELECT_TCLK	= 5

}NX_TIMER_DIVIDSELECT;

typedef enum {
	NX_TIMER_LOADMODE_ONESHOT		= 0UL,
	NX_TIMER_LOADMODE_AUTORELOAD	= 1UL
}NX_TIMER_LOADMODE;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TIMER_Initialize( void );
U32		NX_TIMER_GetNumberOfModule( void );
U32		NX_TIMER_GetNumberOfChannel( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TIMER_GetPhysicalAddress( U32 ModuleIndex );

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

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_TIMER_SetPrescaler(U32 Channel, U32 value);
U32		NX_TIMER_GetPrescaler(U32 Channel);
CBOOL	NX_TIMER_SetDividerPath(U32 Channel, NX_TIMER_DIVIDSELECT divider);
NX_TIMER_DIVIDSELECT NX_TIMER_GetDividerPath(U32 Channel);


void	NX_TIMER_SetDeadZoneLength(U32 Channel, U32 Length);
U32		NX_TIMER_GetDeadZoneLength(U32 Channel);

CBOOL	NX_TIMER_SetDeadZoneEnable(U32 Channel, CBOOL Enable);
CBOOL	NX_TIMER_GetDeadZoneEnable(U32 Channel);

CBOOL	NX_TIMER_SetOutInvert(U32 Channel, CBOOL Enable);
CBOOL	NX_TIMER_GetOutInvert(U32 Channel);

void	NX_TIMER_SetShotMode(U32 Channel, NX_TIMER_LOADMODE ShotMode);
NX_TIMER_LOADMODE	NX_TIMER_GetShotMode(U32 Channel);

void	NX_TIMER_UpdateCounter(U32 Channel, CBOOL Enable );

void	NX_TIMER_Run(U32 Channel);
void	NX_TIMER_Stop(U32 Channel);
CBOOL	NX_TIMER_IsRun(U32 Channel);

void	NX_TIMER_SetPeriod(U32 Channel, U32 Period);
U32		NX_TIMER_GetPeriod(U32 Channel);

CBOOL	NX_TIMER_SetDuty(U32 Channel, U32 Duty);
U32		NX_TIMER_GetDuty(U32 Channel);

U32		NX_TIMER_GetCurrentCount(U32 Channel);

//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_TIMER_H
