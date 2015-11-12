//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: RTC
//	File		: nx_rtc.h
//	Description	:
//	Author		: Goofy
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_RTC_H__
#define __NX_RTC_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	RTC RTC
//------------------------------------------------------------------------------
//@{

	/// @brief	RTC Module's Register List
	struct	NX_RTC_RegisterSet
	{
		volatile U32 RTCCNTWRITE;		///< 0x00 : RTC counter register (Write only)
		volatile U32 RTCCNTREAD;		///< 0x04 : RTC counter register (Read only)
		volatile U32 RTCALARM;			///< 0x08 : RTC alarm register
		volatile U32 RTCCTRL;			///< 0x0C : RTC control register
		volatile U32 RTCINTENB;			///< 0x10 : RTC interrupt enable register
		volatile U32 RTCINTPND;			///< 0x14 : RTC interrupt pending register (Read only)
		volatile U32 RTCCORERSTIMESEL;	///< 0x18 : RTC Core POR time select register
		volatile U32 RTCSCRATCH;
		volatile U32 RTCSCRATCHSTATUS;
	};

	/// @brief RTC interrupt for interrupt interface
	enum
	{
		NX_RTC_INT_COUNTER	= 0,	///<	RTC Counter interrupt.
		NX_RTC_INT_ALARM	= 1		///<	RTC Alarm interrupt.
	};

	/// @brief	Delay time for Power Manger's reset
	typedef enum
	{
		NX_RTC_RESETDELAY_BYPASS	= 1UL << 0,	///< No Delay(Bypass). Without delay, reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_31MS		= 1UL << 1,	///< After 31ms , reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_62MS		= 1UL << 2,	///< After 62ms	, reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_93MS		= 1UL << 3,	///< After 93ms	, reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_124MS		= 1UL << 4,	///< After 124ms, reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_155MS		= 1UL << 5,	///< After 155ms, reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_186MS		= 1UL << 6,	///< After 186ms, reset signal is sent to power manager. 
		NX_RTC_RESETDELAY_210MS		= 0			///< After 210ms, reset signal is sent to power manager. 
		
	} NX_RTC_RESETDELAY;
	
	typedef enum
	{
		NX_RTC_OSCSEL_1HZ	= 0,	///< RTC oscillation selection to 1Hz
		NX_RTC_OSCSEL_32HZ	= 1,	///< RTC oscillation selection to 32Hz
	} NX_RTC_OSCSEL;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_RTC_Initialize( void );
U32		NX_RTC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_RTC_GetPhysicalAddress( void );
U32		NX_RTC_GetSizeOfRegisterSet( void );
void	NX_RTC_SetBaseAddress( void* BaseAddress );
void*	NX_RTC_GetBaseAddress( void );
CBOOL	NX_RTC_OpenModule( void );
CBOOL	NX_RTC_CloseModule( void );
CBOOL	NX_RTC_CheckBusy( void );
CBOOL	NX_RTC_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
S32		NX_RTC_GetInterruptNumber( void );

void	NX_RTC_SetInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_RTC_GetInterruptEnable( S32 IntNum );
CBOOL	NX_RTC_GetInterruptPending( S32 IntNum );
void	NX_RTC_ClearInterruptPending( S32 IntNum );

void	NX_RTC_SetInterruptEnableAll( CBOOL Enable );
CBOOL	NX_RTC_GetInterruptEnableAll( void );
CBOOL	NX_RTC_GetInterruptPendingAll( void );
void	NX_RTC_ClearInterruptPendingAll( void );

void	NX_RTC_SetInterruptEnable32( U32 EnableFlag );
U32		NX_RTC_GetInterruptEnable32( void );
U32		NX_RTC_GetInterruptPending32( void );
void	NX_RTC_ClearInterruptPending32( U32 PendingFlag );

S32		NX_RTC_GetInterruptPendingNumber( void );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	RTC Operation.
//@{
void	NX_RTC_SetAlarmCounter( U32 AlarmCounter );
U32		NX_RTC_GetAlarmCounter( void );
CBOOL	NX_RTC_IsBusyAlarmCounter( void );
void	NX_RTC_SetRTCCounter( U32 RTCCounter );
U32		NX_RTC_GetRTCCounter( void );
CBOOL	NX_RTC_IsBusyRTCCounter( void );
void	NX_RTC_SetRTCCounterWriteEnable( CBOOL enable );
void				NX_RTC_SetRestDelay( NX_RTC_RESETDELAY delay );
NX_RTC_RESETDELAY	NX_RTC_GetRestDelay( void );
void  NX_RTC_SetScratch( U32 Scratch );
U32   NX_RTC_GetScratch( void );
void NX_RTC_SetOscSel(NX_RTC_OSCSEL mode);
//@}
	
//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_xxx_H__
