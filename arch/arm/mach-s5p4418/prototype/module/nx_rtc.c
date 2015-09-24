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
//	Module		: RTC
//	File		: nx_rtc.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------


#include "nx_chip.h"
#ifdef NUMBER_OF_CLKPWR_MODULE
	#include "nx_clkpwr.h"
#endif
#include "nx_rtc.h"

static	struct NX_RTC_RegisterSet *__g_pRegister = CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.\n
 *	@see	NX_RTC_GetNumberOfModule
 */
CBOOL	NX_RTC_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = CNULL;
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@see		NX_RTC_Initialize
 */
U32		NX_RTC_GetNumberOfModule( void )
{
	return NUMBER_OF_RTC_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 *	@see										NX_RTC_GetSizeOfRegisterSet,
 *				NX_RTC_SetBaseAddress,			NX_RTC_GetBaseAddress,
 *				NX_RTC_OpenModule,				NX_RTC_CloseModule,
 *				NX_RTC_CheckBusy,				NX_RTC_CanPowerDown
 */
U32		NX_RTC_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_RTC_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_RTC_GetPhysicalAddress,
 *				NX_RTC_SetBaseAddress,			NX_RTC_GetBaseAddress,
 *				NX_RTC_OpenModule,				NX_RTC_CloseModule,
 *				NX_RTC_CheckBusy,				NX_RTC_CanPowerDown
 */
U32		NX_RTC_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_RTC_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_RTC_GetPhysicalAddress,		NX_RTC_GetSizeOfRegisterSet,
 *												NX_RTC_GetBaseAddress,
 *				NX_RTC_OpenModule,				NX_RTC_CloseModule,
 *				NX_RTC_CheckBusy,				NX_RTC_CanPowerDown
 */
void	NX_RTC_SetBaseAddress( U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	__g_pRegister = ( struct NX_RTC_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_RTC_GetPhysicalAddress,		NX_RTC_GetSizeOfRegisterSet,
 *				NX_RTC_SetBaseAddress,
 *				NX_RTC_OpenModule,				NX_RTC_CloseModule,
 *				NX_RTC_CheckBusy,				NX_RTC_CanPowerDown
 */
U32		NX_RTC_GetBaseAddress( void )
{
	return (U32)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 *	@see		NX_RTC_GetPhysicalAddress,		NX_RTC_GetSizeOfRegisterSet,
 *				NX_RTC_SetBaseAddress,			NX_RTC_GetBaseAddress,
 *												NX_RTC_CloseModule,
 *				NX_RTC_CheckBusy,				NX_RTC_CanPowerDown
 */
CBOOL	NX_RTC_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_RTC_GetPhysicalAddress,		NX_RTC_GetSizeOfRegisterSet,
 *				NX_RTC_SetBaseAddress,			NX_RTC_GetBaseAddress,
 *				NX_RTC_OpenModule,
 *				NX_RTC_CheckBusy,				NX_RTC_CanPowerDown
 */
CBOOL	NX_RTC_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_RTC_GetPhysicalAddress,		NX_RTC_GetSizeOfRegisterSet,
 *				NX_RTC_SetBaseAddress,			NX_RTC_GetBaseAddress,
 *				NX_RTC_OpenModule,				NX_RTC_CloseModule,
 *												NX_RTC_CanPowerDown
 */
CBOOL	NX_RTC_CheckBusy( void )
{
	return ( NX_RTC_IsBusyAlarmCounter() || NX_RTC_IsBusyRTCCounter() );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		\b CTRUE	indicate that Ready to enter power-down stage. \n
 *				\b CFALSE	indicate that This module can't enter to power-down stage.
 *	@see		NX_RTC_GetPhysicalAddress,		NX_RTC_GetSizeOfRegisterSet,
 *				NX_RTC_SetBaseAddress,			NX_RTC_GetBaseAddress,
 *				NX_RTC_OpenModule,				NX_RTC_CloseModule,
 *				NX_RTC_CheckBusy
 */
CBOOL	NX_RTC_CanPowerDown( void )
{
	if( CTRUE == NX_RTC_CheckBusy() )
	{
		return CFALSE;
	}

	return CTRUE;
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@return		Interrupt number
 *	@see											NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
S32		NX_RTC_GetInterruptNumber( void )
{
	return	INTNUM_OF_RTC_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number ( 0 (RTC Counter), 1 (RTC Alarm) ) .
 *	@param[in]	Enable	\b CTRUE	indicate that Interrupt Enable. \n
 *						\b CFALSE	indicate that Interrupt Disable.
 *	@return		None.
 *	@remarks	RTC module have two interrupt source that is RTC counter interrupt and RTC alarm interrupt.
 *				So. \e IntNum must have 0(Counter interrupt) or 1(Alarm interrupt).
 *	@see		NX_RTC_GetInterruptNumber,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
void	NX_RTC_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
	register struct NX_RTC_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( 2 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	= __g_pRegister;

	regvalue = pRegister->RTCINTENB;

	regvalue &= ~( 1UL		<< IntNum);
	regvalue |= (U32)Enable << IntNum;

//	pRegister->RTCINTENB = regvalue;
	WriteIODW(&pRegister->RTCINTENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number ( 0 (RTC Counter), 1 (RTC Alarm) ).
 *	@return		\b CTRUE	indicate that Interrupt is enabled. \n
 *				\b CFALSE	indicate that Interrupt is disabled.
 *	@remarks	RTC module have two interrupt source that is RTC counter interrupt and RTC alarm interrupt.
 *				So. \e IntNum must have 0(Counter interrupt) or 1(Alarm interrupt).
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *													NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
CBOOL	NX_RTC_GetInterruptEnable( S32 IntNum )
{
	NX_ASSERT( 2 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL) (( __g_pRegister->RTCINTENB >> IntNum ) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	\n
 *							- EnableFlag[0] : Set RTC Counter interrupt enable or disable. \n
 *							- EnableFlag[1] : Set RTC Alarm interrupt enable or disable. \n
 *	@return		None.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
void	NX_RTC_SetInterruptEnable32( U32 EnableFlag )
{
	const U32 ENB_MASK = 0x03;

	register struct NX_RTC_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	= __g_pRegister;

//	pRegister->RTCINTENB = EnableFlag & ENB_MASK;
	WriteIODW(&pRegister->RTCINTENB, EnableFlag & ENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@return		Current setting value of interrupt. \n
 *				"1" means interrupt is enabled. \n
 *				"0" means interrupt is disabled. \n
 *				- Return Value[0] : RTC Counter interrupt's setting value. \n
 *				- Return Value[1] : RTC Alarm interrupt's setting value. \n
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *													NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
U32		NX_RTC_GetInterruptEnable32( void )
{
	const U32	ENB_MASK = 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)(__g_pRegister->RTCINTENB & ENB_MASK);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number ( 0 (RTC Counter), 1 (RTC Alarm) ).
 *	@return		\b CTRUE	indicate that Pending is seted. \n
 *				\b CFALSE	indicate that Pending is Not Seted.
 *	@remarks	RTC module have two interrupt source that is RTC counter interrupt and RTC alarm interrupt.
 *				So. \e IntNum must have 0(Counter interrupt) or 1(Alarm interrupt).
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
CBOOL	NX_RTC_GetInterruptPending( S32 IntNum )
{
	NX_ASSERT( 2 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL) (( __g_pRegister->RTCINTPND >> IntNum ) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@return		Current setting value of pending bit. \n
 *				"1" means pend bit is occured. \n
 *				"0" means pend bit is NOT occured. \n
 *				- Return Value[0] : RTC Counter pending state. \n
 *				- Return Value[1] : RTC Alarm pending state. \n
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *													NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
U32		NX_RTC_GetInterruptPending32( void )
{
	const U32 PEND_MASK = 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)(__g_pRegister->RTCINTPND & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt Number ( 0 (RTC Counter), 1 (RTC Alarm) ).
 *	@return		None.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
void	NX_RTC_ClearInterruptPending( S32 IntNum )
{
	register struct NX_RTC_RegisterSet*	pRegister;

	NX_ASSERT( 2 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	= __g_pRegister;

//	pRegister->RTCINTPND = (U32)(1 << IntNum);
	WriteIODW(&pRegister->RTCINTPND, (U32)(1 << IntNum));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	\n \n
 *								- PendingFlag[0] : RTC Counter pending bit. \n
 *								- PendingFlag[1] : RTC Alarm pending bit. \n
 *	@return		None.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *													NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
void	NX_RTC_ClearInterruptPending32( U32 PendingFlag )
{
	const U32 PEND_MASK = 0x03;

	register struct NX_RTC_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	= __g_pRegister;

//	pRegister->RTCINTPND = (U32)(PendingFlag & PEND_MASK);
	WriteIODW(&pRegister->RTCINTPND, (U32)(PendingFlag & PEND_MASK));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	Enable	\b CTRUE	indicate that Set to all interrupt enable. \n
 *						\b CFALSE	indicate that Set to all interrupt disable.
 *	@return		None.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
void	NX_RTC_SetInterruptEnableAll( CBOOL Enable )
{
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	__g_pRegister->RTCINTENB = Enable<<1 | Enable;

	if( Enable )
	{
	//	__g_pRegister->RTCINTENB = 0x03;
		WriteIODW(&__g_pRegister->RTCINTENB, 0x03);
	}
	else
	{
	//	__g_pRegister->RTCINTENB = 0x00;
		WriteIODW(&__g_pRegister->RTCINTENB, 0x00);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		\b CTRUE	indicate that At least one( or more ) interrupt is enabled. \n
 *				\b CFALSE	indicate that All interrupt is disabled.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *													NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
CBOOL	NX_RTC_GetInterruptEnableAll( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->RTCINTENB )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@return		\b CTRUE	indicate that At least one( or more ) pending is seted. \n
 *				\b CFALSE	indicate that All pending is NOT seted.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,
 *				NX_RTC_ClearInterruptPendingAll,	NX_RTC_GetInterruptPendingNumber
 */
CBOOL	NX_RTC_GetInterruptPendingAll( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->RTCINTPND )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@return		None.
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *													NX_RTC_GetInterruptPendingNumber
 */
void	NX_RTC_ClearInterruptPendingAll( void )
{
	register struct NX_RTC_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

//	pRegister->RTCINTPND = 0x03;
	WriteIODW(&pRegister->RTCINTPND, 0x03);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@return		Pending Number( If all pending is not set then return -1 ).
 *	@see		NX_RTC_GetInterruptNumber,			NX_RTC_SetInterruptEnable,
 *				NX_RTC_GetInterruptEnable,			NX_RTC_SetInterruptEnable32,
 *				NX_RTC_GetInterruptEnable32,		NX_RTC_GetInterruptPending,
 *				NX_RTC_GetInterruptPending32,		NX_RTC_ClearInterruptPending,
 *				NX_RTC_ClearInterruptPending32,		NX_RTC_SetInterruptEnableAll,
 *				NX_RTC_GetInterruptEnableAll,		NX_RTC_GetInterruptPendingAll,
 *				NX_RTC_ClearInterruptPendingAll
 */
S32		NX_RTC_GetInterruptPendingNumber( void )	// -1 if None
{
	register		U32 intpend;
	static const	S32 IntNum[4] = { -1, 0, 1, 0 };

	NX_ASSERT( CNULL != __g_pRegister );

	intpend = __g_pRegister->RTCINTPND & 0x03;

	return IntNum[ intpend ];
}

//------------------------------------------------------------------------------
// RTC Operation.
//------------------------------------------------------------------------------
/**
 *	@brief		Set new alarm value.
 *	@param[in]	AlarmCounter	new alarm value.
 *	@return		None.
 *	@remark		Before you call this function, you have to check a return value of
 *				NX_RTC_IsBusyAlarmCounter function to ensure the alarm counter is ready
 *				to write a new value.\n
 *				Therefore the sequence for changing a alarm value is as follows.
 *	@code
 *		while( NX_RTC_IsBusyAlarmCounter() );	// wait until a last writing operation is completed.
 *		NX_RTC_SetAlarmCounter( newvalue );	// update a alarm value.
 *	@endcode
 *	@see											NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
void		NX_RTC_SetAlarmCounter( U32 AlarmCounter )
{
	NX_ASSERT( CFALSE == NX_RTC_IsBusyAlarmCounter() );
	NX_ASSERT( CNULL != __g_pRegister );

//	__g_pRegister->RTCALARM = AlarmCounter;
	WriteIODW(&__g_pRegister->RTCALARM, AlarmCounter);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a alarm value.
 *	@return		current alarm value.
 *	@see		NX_RTC_SetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
U32		NX_RTC_GetAlarmCounter( void )
{
	NX_ASSERT( CNULL != __g_pRegister );
	return __g_pRegister->RTCALARM;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether a last writing operation on alarm register is completed or not.
 *	@return		\b CTRUE	indicates a writing operation on alarm counter is not completed.\n
 *				\b CFALSE	indicates a writing operation on alarm counter is completed.
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *													NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
CBOOL	NX_RTC_IsBusyAlarmCounter( void )
{
	const U32 ALARMCNTWAIT	= (1UL<<3);
	NX_ASSERT( CNULL != __g_pRegister );

	return (__g_pRegister->RTCCTRL & ALARMCNTWAIT) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set new RTC counter value.
 *	@param[in]	RTCCounter	new RTC counter value.
 *	@return		None.
 *	@remark		Before you call this function, you have to ensure CPU can access a
 *				RTC counter block. After you call this function, you have to wait
 *				until a writing operation is completed before you isolate a RTC
 *				counter block.\n
 *				Therefore the sequence for changing a alarm value is as follows.
 *	@code
 *		NX_RTC_SetRTCCounterWriteEnable( CTRUE );	// Deisolate a RTC counter block.
 *		NX_RTC_SetRTCCounter( newvalue );			// update a RTC counter value.
 *		while( NX_RTC_IsBusyRTCCounter() );			// wait until a writing operation is completed.
 *		NX_RTC_SetRTCCounterWriteEnable( CFALSE );	// Isolate a RTC counter block.
 *	@endcode
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
void	NX_RTC_SetRTCCounter( U32 RTCCounter )
{
	NX_ASSERT( CFALSE == NX_RTC_IsBusyRTCCounter() );
	NX_ASSERT( CNULL != __g_pRegister );

//	__g_pRegister->RTCCNTWRITE = RTCCounter;
	WriteIODW(&__g_pRegister->RTCCNTWRITE, RTCCounter);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current RTC counter value.
 *	@return		current RTC counter value.
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *													NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
U32		NX_RTC_GetRTCCounter( void )
{
	NX_ASSERT( CNULL != __g_pRegister );
	return __g_pRegister->RTCCNTREAD;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether a last writing operation on RTC counter register is completed or not.
 *	@return		\b CTRUE	indicates a writing operation on RTC counter is not completed.\n
 *				\b CFALSE	indicates a writing operation on RTC counter is completed.
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
CBOOL	NX_RTC_IsBusyRTCCounter( void )
{
	const U32 RTCCNTWAIT	= (1UL<<4);
	NX_ASSERT( CNULL != __g_pRegister );

	return (__g_pRegister->RTCCTRL & RTCCNTWAIT) ? CTRUE : CFALSE;
}

void NX_RTC_SetOscSel(NX_RTC_OSCSEL mode)
{
	const U32 RTCOSCSEL_BIT	= 1;
	NX_ASSERT( CNULL != __g_pRegister );
	__g_pRegister->RTCCTRL = NX_BIT_SetBit32( __g_pRegister->RTCCTRL, (U32)mode, RTCOSCSEL_BIT);

}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable to write data on RTC registers.
 *	@param[in]	enable	Set it to CTRUE to enable a writing on RTC counter register.
 *	@return		None.
 *	@remark		RTC counter module is always powered on regardless the system power
 *				modes. RTC has a isolation function to prevent unpredictable writing
 *				on RTC counter register	by a power-off sequence of other modules.
 *				A calling this function with CFALSE isolates RTC counter block and
 *				prevents a writing on RTC counter by CPU. A calling this function
 *				with CTURE deisolates RTC counter block and allows a writing on RTC
 *				counter by CPU. Therefore you have to call this function with CFALSE
 *				before entering	a power-down sequence. A reading on RTC counter has
 *				no affect by this function.
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *													NX_RTC_SetRestDelay,
 *				NX_RTC_GetRestDelay
 */
void	NX_RTC_SetRTCCounterWriteEnable( CBOOL enable )
{
	const U32 WRITEENB_POS	= 0;
	const U32 WRITEENB		= 1UL << 0;
	register struct NX_RTC_RegisterSet	*pRegister;
	register U32 regvalue;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( (0==enable) || (1==enable) );

	pRegister = __g_pRegister;
	regvalue = pRegister->RTCCTRL;

	regvalue &= ~(WRITEENB);
	regvalue |= (enable) << WRITEENB_POS;

//	pRegister->RTCCTRL = regvalue;
	WriteIODW(&pRegister->RTCCTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set reset delay time
 *	@param[in]	delay	holding time for Power Manager reset.
 *	@return		None.
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,
 *				NX_RTC_GetRestDelay
 */
void				NX_RTC_SetRestDelay( NX_RTC_RESETDELAY delay )
{
	NX_ASSERT( NX_RTC_RESETDELAY_186MS >= delay );
	NX_ASSERT( CNULL != __g_pRegister );

//	__g_pRegister->RTCCORERSTIMESEL = (U32)delay;
	WriteIODW(&__g_pRegister->RTCCORERSTIMESEL, (U32)delay);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of delay time.
 *	@return		delay time for Power Manager reset.
 *	@see		NX_RTC_SetAlarmCounter,				NX_RTC_GetAlarmCounter,
 *				NX_RTC_IsBusyAlarmCounter,			NX_RTC_SetRTCCounter,
 *				NX_RTC_GetRTCCounter,				NX_RTC_IsBusyRTCCounter,
 *				NX_RTC_SetRTCCounterWriteEnable,	NX_RTC_SetRestDelay
 */
NX_RTC_RESETDELAY	NX_RTC_GetRestDelay( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return (NX_RTC_RESETDELAY)(__g_pRegister->RTCCORERSTIMESEL);
}


void  NX_RTC_SetScratch( U32 Scratch )
{
	NX_ASSERT( CNULL != __g_pRegister );

	__g_pRegister->RTCSCRATCH = Scratch;
}
U32   NX_RTC_GetScratch( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return __g_pRegister->RTCSCRATCH;
}