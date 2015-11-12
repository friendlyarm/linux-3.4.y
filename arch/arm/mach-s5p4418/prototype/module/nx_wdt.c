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
//	File			: nx_wdt.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120911 first implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_wdt.h>

static	struct
{
	struct NX_WDT_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_WDT_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicates that Initialize is successed.
 *			 CFALSE indicates that Initialize is failed.
 */
CBOOL	NX_WDT_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_WDT_MODULE; i++ )
		{
			__g_ModuleVariables[i].pRegister = CNULL;
		}

		bInit = CTRUE;
	}

	return bInit;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_WDT_Initialize
 */
U32		NX_WDT_GetNumberOfModule( void )
{
	return NUMBER_OF_WDT_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_WDT_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( WDT ) };

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_WDT_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_WDT_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_WDT_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_WDT_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 */
void*	NX_WDT_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (U32*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Initialize is successed. 
 *				CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_WDT_OpenModule( U32 ModuleIndex )
{
	register struct NX_WDT_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIO32(&pRegister->WTCON) == 0x8021 );
	NX_ASSERT( ReadIO32(&pRegister->WTDAT) == 0x8000 );
	NX_ASSERT( ReadIO32(&pRegister->WTCNT) == 0x8000 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Deinitialize is successed.
 *				CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_WDT_CloseModule( U32 ModuleIndex )
{
	register struct NX_WDT_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIO32(&pRegister->WTCON, 0x8021);
	WriteIO32(&pRegister->WTDAT, 0x8000);
	WriteIO32(&pRegister->WTCNT, 0x8000);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Module is Busy. 
 *				CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_WDT_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Ready to enter power-down stage. 
 *				CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_WDT_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return CTRUE;
}

/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
U32 NX_WDT_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
	const U32 ResetPinNumber[2][NUMBER_OF_WDT_MODULE] =
	{
		{ RESETINDEX_LIST( WDT, PRESETn )},
		{ RESETINDEX_LIST( WDT, nPOR )},
	};

	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_WDT_GetNumberOfReset()
{
	//return (U32)NUMBER_OF_RESET_MODULE_PIN;
	return (U32)2;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 */
U32		NX_WDT_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	WDTInterruptNumber[NUMBER_OF_WDT_MODULE] =
	{
		INTNUM_LIST(WDT)
	};

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return	WDTInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@param[in]	Enable	CTRUE	indicates that Interrupt Enable. 
 *						CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_WDT_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	2;
	const U32	PEND_MASK	=	0x1 << PEND_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	NX_ASSERT( ( CFALSE == Enable) || ( CTRUE == Enable) );
  
	ReadValue	=	ReadIO32(&pRegister->WTCON) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (PEND_POS)));
	ReadValue	|=	(U32)Enable << (PEND_POS) ;

	WriteIO32(&pRegister->WTCON, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that Interrupt is enabled. 
 *				CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_WDT_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	2;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 */
void	NX_WDT_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x0 << PEND_POS;

	register struct NX_WDT_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->WTCLRINT, ((1 << IntNum) & PEND_MASK) << PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Watchdog Prescaler value.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	Prescaler		Clock Prescaler ( Range : 0 ~ 0xFF)
 *	@return		None.
 */
void	NX_WDT_SetPrescaler( U32 ModuleIndex, U8 Prescaler )
{
	const U32 PRE_POS	=	8;
	const U32 PRE_MASK	=	0xFF << PRE_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0xFF <= Prescaler );

	regvalue 	 = ReadIO32(&pRegister->WTCON);
	regvalue	&= ~PRE_MASK;
	regvalue	|= Prescaler << PRE_POS;

	WriteIO32(&pRegister->WTCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Watchdog Prescaler value.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		Presclaer		Clock Prescaler ( Range : 0 ~ 0xFF)

 */
U8		NX_WDT_GetPrescaler( U32 ModuleIndex )
{
	const U32 PRE_POS	=	8;
	const U32 PRE_MASK	=	0xFF << PRE_POS;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (U8)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON) & PRE_MASK) >> PRE_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Watchdog Divide Select.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	ClkSel			Clock Divider Select.
 								(0: DIV16, 1:DIV32, 2:DIV64, 3:DIV128 )
 *	@return		None.
 */
void	NX_WDT_SetClockDivide( U32 ModuleIndex, NX_WDT_CLOCK_DIV ClkSel )
{
	const U32 CLKSEL_POS	=	3;
	const U32 CLKSEL_MASK	=	3UL << CLKSEL_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3 >= ClkSel );

	regvalue 	 = ReadIO32(&pRegister->WTCON);
	regvalue	&= ~CLKSEL_MASK;
	regvalue	|= ClkSel << CLKSEL_POS;

	WriteIO32(&pRegister->WTCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Watchdog Divide Select.
 *	@param[in]	ModuleIndex		An index of module .
 *	@param[in]	ClkSel			Clock Divider Select.
 								(0: DIV16, 1:DIV32, 2:DIV64, 3:DIV128 )
 *	@return		None.
 */
NX_WDT_CLOCK_DIV	NX_WDT_GetClockDivide( U32 ModuleIndex )
{
	const U32 CLKSEL_POS	=	3;
	const U32 CLKSEL_MASK	=	3UL << CLKSEL_POS;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (NX_WDT_CLOCK_DIV)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON) & CLKSEL_MASK) >> CLKSEL_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Watchdog Timer Enable.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	Enable			CTRUE 	Indicate that WatchDog Timer Enable.
 								CFALSE	Indicate that WatchDOg Timer Disable.
 *	@return		None.
 */
void	NX_WDT_SetEnable( U32 ModuleIndex, CBOOL Enable )
{
	const U32 ENB_POS	=	5;
	const U32 ENB_MASK	=	1UL << ENB_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue 	 = ReadIO32(&pRegister->WTCON);
	regvalue	&= ~ENB_MASK;
	regvalue	|= Enable << ENB_POS;

	WriteIO32(&pRegister->WTCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Watchdog Timer Enable.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		CTRUE 	Indicate that WatchDog Timer Enable.
 				CFALSE	Indicate that WatchDOg Timer Disable.
 */
CBOOL		NX_WDT_GetEnable( U32 ModuleIndex )
{
	const U32 ENB_POS	=	5;
	const U32 ENB_MASK	=	1UL << ENB_POS;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON) & ENB_MASK) >> ENB_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Watchdog Timer Reset Enable.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	Enable			CTRUE 	Indicate that WatchDog Timer Reset Enable.
 								CFALSE	Indicate that WatchDOg Timer Reset Disable.
 *	@return		None.
 */
void	NX_WDT_SetResetEnable( U32 ModuleIndex, CBOOL Enable )
{
	const U32 ENB_POS	=	0;
	const U32 ENB_MASK	=	1UL << ENB_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue 	 = ReadIO32(&pRegister->WTCON);
	regvalue	&= ~ENB_MASK;
	regvalue	|= Enable << ENB_POS;

	WriteIO32(&pRegister->WTCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Watchdog Timer Reset Enable.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		CTRUE 	Indicate that WatchDog Timer Reset Enable.
 				CFALSE	Indicate that WatchDOg Timer Reset Disable.
 */
CBOOL		NX_WDT_GetResetEnable( U32 ModuleIndex )
{
	const U32 ENB_POS	=	0;
	const U32 ENB_MASK	=	1UL << ENB_POS;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON) & ENB_MASK) >> ENB_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Watchdog Timer Reload Count.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	ReloadData		Reload Data ( Range : 0 ~ 65535 )
 *	@return		None.
 */
void	NX_WDT_SetReloadCount( U32 ModuleIndex, U16 ReloadData )
{
	const U32 RE_DATA_POS	=	0;
	const U32 RE_DATA_MASK	=	0xFFFF << RE_DATA_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0xFFFF >= ReloadData );

	regvalue 	 = ReadIO32(&pRegister->WTDAT);
	regvalue	&= ~RE_DATA_MASK;
	regvalue	|= ReloadData << RE_DATA_POS;

	WriteIO32(&pRegister->WTDAT, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Watchdog Timer Reload Count.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		ReloadData		Reload Data ( Range : 0 ~ 65535 )
 */
U16		NX_WDT_GetReloadCount( U32 ModuleIndex )
{
	const U32 RE_DATA_POS	=	0;
	const U32 RE_DATA_MASK	=	0xFFFF << RE_DATA_POS;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTDAT) & RE_DATA_MASK) >> RE_DATA_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Watchdog Timer Currnet Count.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	CurData			Currnet Data ( Range : 0 ~ 65535 )
 *	@return		None.
 */
void	NX_WDT_SetCurrentCount( U32 ModuleIndex, U16 CurData )
{
	const U32 CUR_DATA_POS	=	0;
	const U32 CUR_DATA_MASK	=	0xFFFF << CUR_DATA_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0xFFFF >= CurData );

	regvalue 	 = ReadIO32(&pRegister->WTCNT);
	regvalue	&= ~CUR_DATA_MASK;
	regvalue	|= CurData << CUR_DATA_POS;

	WriteIO32(&pRegister->WTCNT, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Watchdog Timer Current Count.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		Currnet Data ( Range : 0 ~ 65535 )
 */
U16		NX_WDT_GetCurrentCount( U32 ModuleIndex )
{
	const U32 CUR_DATA_POS	=	0;
	const U32 CUR_DATA_MASK	=	0xFFFF << CUR_DATA_POS;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->WTCNT) & CUR_DATA_MASK) >> CUR_DATA_POS;
}

