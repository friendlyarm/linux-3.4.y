//------------------------------------------------------------------------------
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: Alive
//	File		: nx_alive.c
//	Description	:
//	Author		: Firmware
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#ifdef NUMBER_OF_CLKPWR_MODULE
	#include "nx_clkpwr.h"
#endif
#include "nx_alive.h"
                                          
static	struct NX_ALIVE_RegisterSet *__g_pRegister = CNULL;
//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		 CTRUE	indicates that Initializing is succeeded.\n
 *				 CFALSE	indicates that Initializing is failed.\n
 *	@see		NX_ALIVE_GetNumberOfModule
 */
CBOOL	NX_ALIVE_Initialize( void )
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
 *	@return		Module's number.
 */
U32		NX_ALIVE_GetNumberOfModule( void )
{
	return NUMBER_OF_ALIVE_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_ALIVE_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_ALIVE_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_ALIVE_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_ALIVE_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_ALIVE_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_ALIVE_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_ALIVE_GetBaseAddress( void )
{
	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicates that Initializing is succeeded. 
 *				 CFALSE	indicates that Initializing is failed.
 */
CBOOL	NX_ALIVE_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicates that Deinitializing is succeeded. 
 *				 CFALSE	indicates that Deinitializing is failed.
 */
CBOOL	NX_ALIVE_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicates that Module is Busy. 
 *				 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_ALIVE_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		 CTRUE	indicates that Ready to enter power-down stage. 
 *				 CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_ALIVE_CanPowerDown( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@return		Interrupt number
 */
S32		NX_ALIVE_GetInterruptNumber( void )
{
	return	INTNUM_OF_ALIVE_MODULE;
}

/*
static const U32 IntNumDPC[NUMBER_OF_DPC_MODULE] = 
		{ INTNUM_OF_DUALDISPLAY_MODULE_PRIMIRQ,
		  INTNUM_OF_DUALDISPLAY_MODULE_SECONDIRQ };

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );

	return	IntNumDPC[ModuleIndex];
*/
	

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number(0 ~ 3). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3
 *	@param[in]	Enable	 CTRUE	indicates that Interrupt Enable. 
 *						 CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 *	@remarks	Wake up from sleep mode ( Sleep mode is all block's power off except alive block )
 *				- user press nVDDPWRONTOGGLE pin.
 *				- RTC interrupt occuring for RTC block.
 *				- Alive GPIO's event occuring.
 *				This case Alive block turn VDDPWRON pin on so that core power supply to Clock & Power block. 
 *				Alive GPIO pin can not use input pin like normal GPIO input. because Alive GPIO input is occuring
 *				it turn VDDPWRON pin on directly. 
 *				Alive GPIO only use for wake up from sleep mode.
 *	@code
 *	//--------------------------------------------------------------------------
 *	//
 *	//					OR
 *	//(wake up from)<---+-- nVDDPWRTOGGLE (external switch)
 *	//	sleep			|
 *	//					+-- RTC IRQ (from RTC block)
 *	//					|							AND
 *	//					+---<-------------------+---+-- Detect Enable [DETECTENB]
 *	//												|		|					OR
 *	//												|		+-- Detect Mode ----+-- Async Low			[ASYNCDETECTMODEREG0]
 *	//												|							+-- Async High			[ASYNCDETECTMODEREG1]
 *	//					AND							|							+-- Sync Falling Edge	[DETECTMODEREG0]
 *	//	Alive IRQ		<---+-- Pend [DETECTPEND] <-+							+-- Sync Rising Edge	[DETECTMODEREG1]
 *	//						|													+-- Sync Low			[DETECTMODEREG2]
 *	//						+-- Interrupt Enable [INTENB]						+-- Sync High			[DETECTMODEREG3]
 *	//
 *	//														Alive GPIO[3:0]
 *	//--------------------------------------------------------------------------
 *	@endcode
 */
void	NX_ALIVE_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
	register U32 INTENB_MASK;

	//NX_ASSERT( 4 > IntNum );
	NX_ASSERT( (0==Enable) | (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	INTENB_MASK = ( 1UL << IntNum );


	if( Enable )
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOINTENBSETREG, INTENB_MASK);
	}
	else
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOINTENBRSTREG, INTENB_MASK);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number(0 ~ 3). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3
 *	@return		 CTRUE	indicates that Interrupt is enabled. 
 *				 CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_ALIVE_GetInterruptEnable( S32 IntNum )
{
	//NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)( (__g_pRegister->ALIVEGPIOINTENBREADREG >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							- EnableFlag[0] : Set ALIVEGPIO0 interrupt enable or disable. 
 *							...
 *							- EnableFlag[3] : Set ALIVEGPIO3 interrupt enable or disable. 
 *	@return		None.
 */
void	NX_ALIVE_SetInterruptEnable32( U32 EnableFlag )
{
	register U32 Set_Mask;
	register U32 ReSet_Mask;

	NX_ASSERT( CNULL != __g_pRegister );

	Set_Mask = EnableFlag & 0x03F;
	ReSet_Mask = (~EnableFlag & 0x03F);

	WriteIO32(&__g_pRegister->ALIVEGPIOINTENBRSTREG, ReSet_Mask);
	WriteIO32(&__g_pRegister->ALIVEGPIOINTENBSETREG, Set_Mask);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@return		Current setting value of interrupt. 
 *				"1" means interrupt is enabled. 
 *				"0" means interrupt is disabled. 
 *				- Return Value[0] : ALIVEGPIO0 interrupt's setting value. 
 *				...
 *				- Return Value[3] : ALIVEGPIO3 interrupt's setting value. 
 */
U32		NX_ALIVE_GetInterruptEnable32( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)(__g_pRegister->ALIVEGPIOINTENBREADREG & 0x03F );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number(0 ~ 3). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3
 *	@return		 CTRUE	indicates that Pending is seted. 
 *				 CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_ALIVE_GetInterruptPending( S32 IntNum )
{
	NX_ASSERT( 8 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return ( (__g_pRegister->ALIVEGPIODETECTPENDREG >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@return		Current setting value of pending bit. 
 *				"1" means pend bit is occured. 
 *				"0" means pend bit is NOT occured. 
 *				- Return Value[0] : ALIVEGPIO0 pending state. 
 *				...
 *				- Return Value[3] : ALIVEGPIO3 pending state. 
 */
U32		NX_ALIVE_GetInterruptPending32( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)(__g_pRegister->ALIVEGPIODETECTPENDREG & 0x03F );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt Number(0 ~ 3). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3
 *	@return		None.
 */
void	NX_ALIVE_ClearInterruptPending( S32 IntNum )
{
	NX_ASSERT( 8 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->ALIVEGPIODETECTPENDREG, 1 << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	 
 *								- PendingFlag[0] : ALIVEGPIO0 pending bit. 
 *								...
 *								- PendingFlag[3] : ALIVEGPIO3 pending bit. 
 *	@return		None.
 */
void	NX_ALIVE_ClearInterruptPending32( U32 PendingFlag )
{
	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->ALIVEGPIODETECTPENDREG, PendingFlag & 0x03F);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	Enable	 CTRUE	indicates that Set to all interrupt enable. 
 *						 CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_ALIVE_SetInterruptEnableAll( CBOOL Enable )
{
	NX_ASSERT( CNULL != __g_pRegister );

	if( Enable )
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOINTENBSETREG, 0x03F);
	}
	else
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOINTENBRSTREG, 0x03F);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		 CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				 CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_ALIVE_GetInterruptEnableAll( void )
{
	const U32 INTENB_MASK = 0x03F;

	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->ALIVEGPIOINTENBREADREG & INTENB_MASK )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@return		 CTRUE	indicates that At least one( or more ) pending is seted. 
 *				 CFALSE	indicates that All pending is NOT seted.
 */
CBOOL	NX_ALIVE_GetInterruptPendingAll( void )
{
	const U32 ALIVEGPIODETECTPEND_MASK = 0x03F;

	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->ALIVEGPIODETECTPENDREG & ALIVEGPIODETECTPEND_MASK )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@return		None.
 */
void	NX_ALIVE_ClearInterruptPendingAll( void )
{
	const U32 ALIVEGPIODETECTPEND_MASK = 0x03F;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->ALIVEGPIODETECTPENDREG, ALIVEGPIODETECTPEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
S32		NX_ALIVE_GetInterruptPendingNumber( void )	// -1 if None
{
	register struct NX_ALIVE_RegisterSet	*pRegister;
	register U32 Pend;
	U32 dwIntNum;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	Pend = (pRegister->ALIVEGPIODETECTPENDREG & pRegister->ALIVEGPIOINTENBREADREG);

	for( dwIntNum = 0; dwIntNum < 6; dwIntNum++ )
	{
		if( Pend & ( 1 << dwIntNum) )
		{
			return dwIntNum;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
// PAD Configuration
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set Alive GPIO's condition ( Writable or Not )
 *	@param[in]	bEnable		 CTRUE indicate that writing data to Alive GPIO is Enable. 
 *							 CFALSE indicate that writing data to Alive GPIO is Disable.
 *	@return		None.
 *	@remarks	The default setting is NOT writing to Alive Register, in order to keep
 *				the values of Alive Registers when Core Power 1.0V is off. 
 *				User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting Alive GPIO.
 *	@code
 *
 *		NX_ALIVE_SetWriteEnable( CTRUE );			// Can Alive GPIO Setting
 *		...
 *		NX_ALIVE_SetOutputValue( 0, CTRUE );		// Alive GPIO 0 pin set to High value
 *		NX_ALIVE_SetOutputValue( 0, CFALSE );		// Alive GPIO 0 pin set to Low value.
 *		...
 *		NX_ALIVE_SetWriteEnable( CFALSE );			// Now can't setting Alive GPIO pin value.
 *		...
 *		NX_ALIVE_SetOutputValue( 0, CTRUE );		// Alive Gpio 0 pin still have LOW value.
 *	@endcode
 */
void	NX_ALIVE_SetWriteEnable( CBOOL bEnable )
{
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->ALIVEPWRGATEREG, (U32)bEnable);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of Alive GPIO is writable or Not.
 *	@return		 CTRUE indicate that Writing data to Alive GPIO is enabled.
 *				 CFALSE indicate that Writing data to Alive GPIO is disabled.
 */
CBOOL	NX_ALIVE_GetWriteEnable( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)(__g_pRegister->ALIVEPWRGATEREG & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set scratch register
 *	@param[in]	dwData	Data
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 *				The value of Scratch Register maintains in the case of power off of CoreVDD.
 */
void	NX_ALIVE_SetScratchReg( U32 dwData )
{
	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->ALIVESCRATCHSETREG, dwData);
	WriteIO32(&__g_pRegister->ALIVESCRATCHRSTREG, ~dwData);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get data of scratch register.
 *	@return		Data of scratch register.
 */
U32		NX_ALIVE_GetScratchReg( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return	(U32)(__g_pRegister->ALIVESCRATCHREADREG);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set pull-up/down status to Alive GPIO
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@param[in]	bEnable			CTRUE	indicate that PAD pull-up enable. 
 *								CFALSE	indicate that PAD pull-down enable.
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 */
void	NX_ALIVE_SetPullUpEnable( U32 BitNumber, CBOOL bEnable )
{
	register U32 PULLUP_MASK;

	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	PULLUP_MASK = (1UL << BitNumber);

	if( bEnable )
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOPADPULLUPSETREG, PULLUP_MASK);
	}
	else
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOPADPULLUPRSTREG, PULLUP_MASK);
	}
}
void    NX_ALIVE_SetPullUpEnable32( U32 value32 )
{
	NX_ASSERT( CNULL != __g_pRegister );
	
	WriteIO32(&__g_pRegister->ALIVEGPIOPADPULLUPRSTREG, ~value32);
	WriteIO32(&__g_pRegister->ALIVEGPIOPADPULLUPSETREG,  value32);
}
//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of Alive GPIO's pull-up/down status.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@return			CTRUE	indicate that pull-up is enabled. 
 *					CFALSE	indicate that pull-down is enabled.
 */
CBOOL	NX_ALIVE_GetPullUpEnable( U32 BitNumber )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->ALIVEGPIOPADPULLUPREADREG >> BitNumber) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set PAD retention enable or disable.
 *	@param[in]	Group		PAD Group ( 0(RX0 ~ RX4), 1(USB VBUS), 2(GPIO) )
 *	@param[in]	bEnable			CTRUE	indicate that PAD retention enable. .
 *								CFALSE	indicate that PAD retention disable.
 *	@return		None.
 *	@remarks	nPADHoldEnb set to 0 after booting(Power on Reset). 
 *				Use only use nPADHold(bit) for controling PAD protection. 
 *
 *				Setting method into sleep mode .
 *				- 1. Clear wakeup pending (ALIVEGPIODETECTPEND) 
 *				- 2. PAD retention enable (nPADHold set to 0)	
 *				- 3. Core power off ( VDDPWRON set to 0 ) 
 *
 *				Wake up from sleep mode. 
 *				- 1. user press nVDDPWRTOGGLE pin or Alive GPIO pin 
 *				- 2. PAD retention disable (nPADHold set to 1)	
 *				- 3. Core power on ( VDDPWRON set to 1 ) 
 *
 *				User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 *	@code
 *	//--------------------------------------------------------------------------
 *	//
 *	//					OR
 *	// PAD protection --+-- nPADHoldEnb [VDDCTRL register]
 *	//					|
 *	//					|	AND
 *	//					+---+-- Core VDD ( on(1), off(0) )
 *	//						|
 *	//						|	OR
 *	//						+---+-- nPADHold [VDDCTRL register]
 *	//							|
 *	//							|	OR
 *	//							+---+-- nVDDPWRTOGGLE
 *	//								|
 *	//								+-- Alive wakeup event pending
 *	//
 *	//--------------------------------------------------------------------------
 *	@endcode
 *	@see		NX_ALIVE_SetWriteEnable,			NX_ALIVE_GetWriteEnable,
 *				NX_ALIVE_SetScratchReg,				NX_ALIVE_GetScratchReg,
 *				NX_ALIVE_SetPullUpEnable,			NX_ALIVE_GetPullUpEnable,
 *													NX_ALIVE_GetPadRetentionEnable,
 *				NX_ALIVE_SetPadRetentionHold,		NX_ALIVE_GetPadRetentionHold
 */
void	NX_ALIVE_SetPadRetentionEnable( NX_ALIVE_PADGROUP Group, CBOOL bEnable )
{
	const U32 PADHOLDENB_BITPOS = 6;

	NX_ASSERT( NX_ALIVE_PADGROUP_NUMBER > Group );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );


	if( bEnable )
	{
		WriteIO32(&__g_pRegister->VDDCTRLRSTREG, 1U << (Group + PADHOLDENB_BITPOS));
	}
	else
	{
		WriteIO32(&__g_pRegister->VDDCTRLSETREG, 1U << (Group + PADHOLDENB_BITPOS));
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of PAD retention.
 *	@param[in]	Group		PAD Group ( 0(RX0 ~ RX4), 1(USB VBUS), 2(GPIO) )
 *	@return			CTRUE indicate that PAD retention is enabled. .
 *					CFALSE indicate that PAD retention is disabled.
 *	@see		NX_ALIVE_SetWriteEnable,			NX_ALIVE_GetWriteEnable,
 *				NX_ALIVE_SetScratchReg,				NX_ALIVE_GetScratchReg,
 *				NX_ALIVE_SetPullUpEnable,			NX_ALIVE_GetPullUpEnable,
 *				NX_ALIVE_SetPadRetentionEnable,
 *				NX_ALIVE_SetPadRetentionHold,		NX_ALIVE_GetPadRetentionHold
 */
CBOOL	NX_ALIVE_GetPadRetentionEnable( NX_ALIVE_PADGROUP Group )
{
	const U32 PADHOLDENB_BITPOS = 5;

	NX_ASSERT( NX_ALIVE_PADGROUP_NUMBER > Group );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->VDDCTRLREADREG >> (PADHOLDENB_BITPOS + (U32)Group) ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set PAD retention.
 *	@param[in]	Group		PAD Group ( 0(RX0 ~ RX4), 1(USB VBUS), 2(GPIO) )
 *	@param[in]	bEnable			CTRUE	indicate that PAD retention enable. .
 *								CFALSE	indicate that PAD retention disable.
 *	@return		None.
 *	@remarks	nPADHoldEnb set to 0 after booting(Power on Reset). 
 *				Use only use nPADHold(bit) for controling PAD protection. 
 *
 *				Setting method into sleep mode .
 *				- 1. Clear wakeup pending (ALIVEGPIODETECTPEND) 
 *				- 2. PAD retention enable (nPADHold set to 0)	
 *				- 3. Core power off ( VDDPWRON set to 0 ) 
 *
 *				Wake up from sleep mode. 
 *				- 1. user press nVDDPWRTOGGLE pin or Alive GPIO pin 
 *				- 2. PAD retention disable (nPADHold set to 1)	
 *				- 3. Core power on ( VDDPWRON set to 1 ) 
 *
 *				User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 *	@code
 *	//--------------------------------------------------------------------------
 *	//
 *	//					OR
 *	// PAD protection --+-- nPADHoldEnb [VDDCTRL register]
 *	//					|
 *	//					|	AND
 *	//					+---+-- Core VDD ( on(1), off(0) )
 *	//						|
 *	//						|	OR
 *	//						+---+-- nPADHold [VDDCTRL register]
 *	//							|
 *	//							|	OR
 *	//							+---+-- nVDDPWRTOGGLE
 *	//								|
 *	//								+-- Alive wakeup event pending
 *	//
 *	//--------------------------------------------------------------------------
 *	@endcode
 *	@see		NX_ALIVE_SetWriteEnable,			NX_ALIVE_GetWriteEnable,
 *				NX_ALIVE_SetScratchReg,				NX_ALIVE_GetScratchReg,
 *				NX_ALIVE_SetPullUpEnable,			NX_ALIVE_GetPullUpEnable,
 *				NX_ALIVE_SetPadRetentionEnable,		NX_ALIVE_GetPadRetentionEnable,
 *													NX_ALIVE_GetPadRetentionHold
 */
void	NX_ALIVE_SetPadRetentionHold( NX_ALIVE_PADGROUP Group, CBOOL bEnable )
{
	const U32 PADHOLD_BITPOS = 2;

	NX_ASSERT( NX_ALIVE_PADGROUP_NUMBER > Group );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	if( bEnable )	WriteIO32(&__g_pRegister->VDDCTRLRSTREG, 1U << (Group + PADHOLD_BITPOS));
	else			WriteIO32(&__g_pRegister->VDDCTRLSETREG, 1U << (Group + PADHOLD_BITPOS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of PAD retention.
 *	@param[in]	Group		PAD Group ( 0(RX0 ~ RX4), 1(USB VBUS), 2(GPIO) )
 *	@return			CTRUE indicate that PAD retention is enabled. .
 *					CFALSE indicate that PAD retention is disabled.
 *	@see		NX_ALIVE_SetWriteEnable,			NX_ALIVE_GetWriteEnable,
 *				NX_ALIVE_SetScratchReg,				NX_ALIVE_GetScratchReg,
 *				NX_ALIVE_SetPullUpEnable,			NX_ALIVE_GetPullUpEnable,
 *				NX_ALIVE_SetPadRetentionEnable,		NX_ALIVE_GetPadRetentionEnable,
 *				NX_ALIVE_SetPadRetentionHold,
 */
CBOOL	NX_ALIVE_GetPadRetentionHold( NX_ALIVE_PADGROUP Group )
{
	const U32 PADHOLD_BITPOS = 2;

	NX_ASSERT( NX_ALIVE_PADGROUP_NUMBER > Group );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->VDDCTRLREADREG >> (PADHOLD_BITPOS + (U32)Group) ) & 0x01 );
}

//------------------------------------------------------------------------------
// Input Setting Function
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set Alive GPIO's detection
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@param[in]	bEnable			CTRUE	indicate that Alive GPIO detect enable.
 *								CFALSE	indicate that Alive GPIO detect disable.
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().

 */
void	NX_ALIVE_SetDetectEnable( U32 BitNumber, CBOOL bEnable )
{
	register U32 DETECTENB_MASK;

	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	DETECTENB_MASK = ( 1UL << BitNumber );

	if( bEnable )
	{
		WriteIO32(&__g_pRegister->ALIVEGPIODETECTENBSETREG, DETECTENB_MASK);//50
	}
	else
	{
		WriteIO32(&__g_pRegister->ALIVEGPIODETECTENBRSTREG, DETECTENB_MASK);//4c
	}
}

void	NX_ALIVE_SetDetectEnable32( U32 value32 )
{
	NX_ASSERT( CNULL != __g_pRegister );
	WriteIO32(&__g_pRegister->ALIVEGPIODETECTENBSETREG, value32);
	WriteIO32(&__g_pRegister->ALIVEGPIODETECTENBRSTREG, ~value32);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of Alive GPIO's detection.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@return			CTRUE	indicate that Alive GPIO detect is enabled.
 *					CFALSE	indicate that Alive GPIO detect is disabled.

 */
CBOOL	NX_ALIVE_GetDetectEnable( U32 BitNumber )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->ALIVEGPIODETECTENBREADREG >> BitNumber) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Alive GPIO's detection mode.
 *	@param[in]	DetectMode	Select detection mode to chage.
 *	@param[in]	BitNumber	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@param[in]	bEnable			CTRUE	indicate that Alive GPIO's detect mode enable.
 *								CFALSE	indicate that Alive GPIO's detect mode disable.
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 */
void	NX_ALIVE_SetDetectMode( NX_ALIVE_DETECTMODE DetectMode, U32 BitNumber, CBOOL bEnable )
{
	volatile U32	*pSetReg=0;
	volatile U32	*pRstReg=0;

	NX_ASSERT( NX_ALIVE_DETECTMODE_SYNC_HIGHLEVEL >= DetectMode );
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	switch( DetectMode )
	{
		case	NX_ALIVE_DETECTMODE_ASYNC_LOWLEVEL:

				pSetReg = &__g_pRegister->ALIVEGPIOASYNCDETECTMODESETREG0;
				pRstReg = &__g_pRegister->ALIVEGPIOASYNCDETECTMODERSTREG0;

				break;

		case	NX_ALIVE_DETECTMODE_ASYNC_HIGHLEVEL:

				pSetReg = &__g_pRegister->ALIVEGPIOASYNCDETECTMODESETREG1;
				pRstReg = &__g_pRegister->ALIVEGPIOASYNCDETECTMODERSTREG1;

				break;

		case	NX_ALIVE_DETECTMODE_SYNC_FALLINGEDGE:

				pSetReg = &__g_pRegister->ALIVEGPIODETECTMODESETREG0;
				pRstReg = &__g_pRegister->ALIVEGPIODETECTMODERSTREG0;

				break;

		case	NX_ALIVE_DETECTMODE_SYNC_RISINGEDGE:

				pSetReg = &__g_pRegister->ALIVEGPIODETECTMODESETREG1;
				pRstReg = &__g_pRegister->ALIVEGPIODETECTMODERSTREG1;

				break;

		case	NX_ALIVE_DETECTMODE_SYNC_LOWLEVEL:

				pSetReg = &__g_pRegister->ALIVEGPIODETECTMODESETREG2;
				pRstReg = &__g_pRegister->ALIVEGPIODETECTMODERSTREG2;

				break;

		case	NX_ALIVE_DETECTMODE_SYNC_HIGHLEVEL:

				pSetReg = &__g_pRegister->ALIVEGPIODETECTMODESETREG3;
				pRstReg = &__g_pRegister->ALIVEGPIODETECTMODERSTREG3;

				break;

		default:
				NX_ASSERT( CFALSE );
				break;
	}

	if( bEnable )	WriteIO32(pSetReg, (1UL << BitNumber));
	else			WriteIO32(pRstReg, (1UL << BitNumber));
}
void	NX_ALIVE_SetDetectMode32( NX_ALIVE_DETECTMODE DetectMode, U32 value32 )
{
	U32 i;
	for ( i=0;i<NX_ALIVE_NUMBER_OF_GPIO;i++)
	{
		NX_ALIVE_SetDetectMode(  DetectMode, i, (CBOOL)((value32>>i)&1) )	;
	}
}
//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of Alive GPIO's detection mode.
 *	@param[in]	DetectMode	Select detection mode to check.
 *	@param[in]	BitNumber	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@return			CTRUE	indicate that Alive GPIO's detect mode is enabled.
 *					CFALSE	indicate that Alive GPIO's detect mode is disabled.
 */
CBOOL	NX_ALIVE_GetDetectMode( NX_ALIVE_DETECTMODE DetectMode, U32 BitNumber )
{
	NX_ASSERT( NX_ALIVE_DETECTMODE_SYNC_HIGHLEVEL >= DetectMode );
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	switch( DetectMode )
	{
		case	NX_ALIVE_DETECTMODE_ASYNC_LOWLEVEL:

				return (CBOOL)((__g_pRegister->ALIVEGPIOLOWASYNCDETECTMODEREADREG >> BitNumber) & 0x01);
				//break;

		case	NX_ALIVE_DETECTMODE_ASYNC_HIGHLEVEL:

				return (CBOOL)((__g_pRegister->ALIVEGPIOHIGHASYNCDETECTMODEREADREG >> BitNumber) & 0x01);
				//break;

		case	NX_ALIVE_DETECTMODE_SYNC_FALLINGEDGE:

				return (CBOOL)((__g_pRegister->ALIVEGPIOFALLDETECTMODEREADREG >> BitNumber) & 0x01);
				//break;

		case	NX_ALIVE_DETECTMODE_SYNC_RISINGEDGE:

				return (CBOOL)((__g_pRegister->ALIVEGPIORISEDETECTMODEREADREG >> BitNumber) & 0x01);
				//break;

		case	NX_ALIVE_DETECTMODE_SYNC_LOWLEVEL:

				return (CBOOL)((__g_pRegister->ALIVEGPIOLOWDETECTMODEREADREG >> BitNumber) & 0x01);
				//break;

		case	NX_ALIVE_DETECTMODE_SYNC_HIGHLEVEL:

				return (CBOOL)((__g_pRegister->ALIVEGPIOHIGHDETECTMODEREADREG >> BitNumber) & 0x01);
				//break;

		default:
				NX_ASSERT( CFALSE );
				//break;
	}

	NX_ASSERT( CFALSE );
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get state of VDDPWRTOGGLE pin.
 *	@return			CTRUE	indicate that nVDDPWRTOGGLE pin is pushed.
 *					CFALSE	indicate that nVDDPWRTOGGLE pin is NOT pushed.
 */
CBOOL	NX_ALIVE_GetVDDPWRTOGGLE( void )
{
	const U32 VDDPWRTOGGLE_BITPOS = 10;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->VDDCTRLREADREG >> VDDPWRTOGGLE_BITPOS) & 0x01);
}

//------------------------------------------------------------------------------
// Output Setting Function
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set Alive GPIO's output mode.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@param[in]	OutputEnb		CTRUE	indicate that selected Alive GPIO's output mode is  OUTPUT.
 *								CFALSE	indicate that selected Alive GPIO's output mode is  INPUT.
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 */
void	NX_ALIVE_SetOutputEnable( U32 BitNumber, CBOOL OutputEnb )
{
	register U32 PADOUTENB_MASK;

	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( (0==OutputEnb) || (1==OutputEnb) );
	NX_ASSERT( CNULL != __g_pRegister );

	PADOUTENB_MASK = (1UL << BitNumber);

	if( OutputEnb )
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTENBSETREG, PADOUTENB_MASK);
	}
	else
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTENBRSTREG, PADOUTENB_MASK);
	}
}
/*
void	NX_ALIVE_SetOutputEnable32( U32 value )
{
	U32 i;
	for ( i=0 ; i<NX_ALIVE_NUMBER_OF_GPIO ; i++ )
	{
		NX_ALIVE_SetOutputEnable ( i, (value>>i) & 1 );
	}
}
*/
void  NX_ALIVE_SetOutputEnable32( U32 value )
{
	WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTENBRSTREG, ~value );
	WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTENBSETREG, value );
}    

void	NX_ALIVE_SetInputEnable32( U32 value32 )
{
	WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTENBRSTREG, value32 & 0x3F );
}  

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of Alive GPIO's output mode.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@return			CTRUE	indicate that selected Alive GPIO's output mode is  OUTPUT.
 *					CFALSE	indicate that selected Alive GPIO's output mode is  INPUT.
 */
CBOOL	NX_ALIVE_GetOutputEnable( U32 BitNumber )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->ALIVEGPIOPADOUTENBREADREG >> BitNumber) & 0x01);
}

U32		NX_ALIVE_GetOutputEnable32 (void)
{
	NX_ASSERT( CNULL != __g_pRegister );
	return __g_pRegister->ALIVEGPIOPADOUTENBREADREG & 0x3F;
}

CBOOL	NX_ALIVE_GetInputEnable( U32 BitNumber )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->ALIVEGPIOPADOUTENBREADREG >> BitNumber) & 0x01) ? CFALSE : CTRUE;
}

U32		NX_ALIVE_GetInputEnable32 (void)
{
	NX_ASSERT( CNULL != __g_pRegister );
	return (~__g_pRegister->ALIVEGPIOPADOUTENBREADREG) & 0x3F;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Alive GPIO's output value.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@param[in]	Value			CTRUE	indicate that selected Alive GPIO's output value is  HIGH.
 *								CFALSE	indicate that selected Alive GPIO's output value is  LOW.
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting NX_ALIVE_SetScratchReg().
 */
void	NX_ALIVE_SetOutputValue( U32 BitNumber, CBOOL Value )
{
	register U32 PADOUT_MASK;

	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( (0==Value) || (1==Value) );
	NX_ASSERT( CNULL != __g_pRegister );

	PADOUT_MASK = (1UL << BitNumber);

	if( Value )
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTSETREG, PADOUT_MASK);
	}
	else
	{
		WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTRSTREG, PADOUT_MASK);
	}
}

void    NX_ALIVE_SetOutputValue32( U32 value32 )
{
	NX_ASSERT( CNULL != __g_pRegister );
	WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTRSTREG, ~value32);
	WriteIO32(&__g_pRegister->ALIVEGPIOPADOUTSETREG, value32);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of Alive GPIO's output value.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@return			CTRUE	indicate that selected Alive GPIO's output value is  HIGH.
 *					CFALSE	indicate that selected Alive GPIO's output value is  LOW.
 */
CBOOL	NX_ALIVE_GetOutputValue( U32 BitNumber )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->ALIVEGPIOPADOUTREADREG >> BitNumber) & 0x01);
}

U32     NX_ALIVE_GetOutputValue32( void )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return __g_pRegister->ALIVEGPIOPADOUTREADREG;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of Alive GPIO's input value.
 *	@param[in]	BitNumber	Alive GPIO ( 0 ~ 3 ).
 *	@return			CTRUE	indicate that selected Alive GPIO's input value is  HIGH.
 *					CFALSE	indicate that selected Alive GPIO's input value is  LOW.
 */
CBOOL	NX_ALIVE_GetInputValue( U32 BitNumber )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->ALIVEGPIOINPUTVALUE >> BitNumber) & 0x01);
}

U32     NX_ALIVE_GetInputValue32( void )
{
	//NX_ASSERT( 6 > BitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return __g_pRegister->ALIVEGPIOINPUTVALUE;
}




//------------------------------------------------------------------------------
/**
 *	@brief		Set VDDPWRON pin's output value.
 *	@param[in]	bVDDPWRON		Set this as  CTRUE to output VDDPWRON pin as HIGH(on).
 *								Set this as  CFALSE to output VDDPWRON pin as LOW(off).
 *	@param[in]	bVDDPWRON_DDR	Set this as  CTRUE to output VDDPWRON_DDR pin as HIGH(on).
 *								Set this as  CFALSE to output VDDPWRON_DDR pin as LOW(off).
 *	@return		None.
 *	@remarks	User should set  NX_ALIVE_SetWriteEnable( CTRUE ) before Setting this values().
 */
void	NX_ALIVE_SetVDDPWRON( CBOOL bVDDPWRON, CBOOL bVDDPWRON_DDR )
{
	const U32 VDDPWRON		= (1UL<<0);
	const U32 VDDPWRON_DDR	= (1UL<<1);

	register U32 regset = 0;
	register U32 regrst = 0;

	NX_ASSERT( CNULL != __g_pRegister );

	if( bVDDPWRON )			regset |= VDDPWRON;
	else					regrst |= VDDPWRON;
	
	if( bVDDPWRON_DDR )		regset |= VDDPWRON_DDR;
	else					regrst |= VDDPWRON_DDR;
	
	WriteIO32(&__g_pRegister->VDDCTRLSETREG, regset);
	WriteIO32(&__g_pRegister->VDDCTRLRSTREG, regrst);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of VDDPWRON pin's output.
 *	@return			CTRUE	indicate that selected VDDPWRON's output value is  HIGH(on).
 *					CFALSE	indicate that selected VDDPWRON's output value is  LOW(off).
 */
CBOOL	NX_ALIVE_GetVDDPWRON( void )
{
	const U32 VDDPWRON		= (1UL<<0);
	
	NX_ASSERT( CNULL != __g_pRegister );

	return (__g_pRegister->VDDCTRLREADREG & VDDPWRON) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of VDDPWRON_DDR pin's output.
 *	@return			CTRUE	indicate that selected VDDPWRON_DDR's output value is  HIGH(on).
 *					CFALSE	indicate that selected VDDPWRON_DDR's output value is  LOW(off).
 */
CBOOL	NX_ALIVE_GetVDDPWRON_DDR( void )
{
	const U32 VDDPWRON_DDR	= (1UL<<1);

	NX_ASSERT( CNULL != __g_pRegister );

	return (__g_pRegister->VDDCTRLREADREG & VDDPWRON_DDR) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of VDDPWRON_DDR pin's output.
 *	@return			CTRUE	indicate that selected VDDPWRON_DDR's output value is  HIGH(on).
 *					CFALSE	indicate that selected VDDPWRON_DDR's output value is  LOW(off).
 */
U32	NX_ALIVE_GetCorePowerOffDelayTime( void )
{
	if (__g_pRegister->VDDOFFCNTVALUE1 != __g_pRegister->VDDOFFCNTVALUE0)	return 0xffffffff;
	else															     	return __g_pRegister->VDDOFFCNTVALUE1;
}

void  NX_ALIVE_SetCorePowerOffDelayTime( U32 Delay )
{
	__g_pRegister->VDDOFFCNTVALUERST = 0xffffffff;
	__g_pRegister->VDDOFFCNTVALUESET = Delay;
}

U32  NX_ALIVE_GetWakeUpStatus( void )
{
	U32 Status;
  //  volatile U32 CLEARWAKEUPSTATUS;	// 0x70    
   // volatile U32 WAKEUPSTATUS	    ;	// 0x70    
    Status = __g_pRegister->WAKEUPSTATUS;
    return Status;
	
}


void  NX_ALIVE_ClearWakeUpStatus( void )
{
	__g_pRegister->CLEARWAKEUPSTATUS = 1;
}	

