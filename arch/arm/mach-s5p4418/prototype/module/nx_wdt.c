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
//	File		: nx_wdt.c
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
 *	@return \b CTRUE	indicates that Initialize is successed.\r\n
 *			\b CFALSE indicates that Initialize is failed.\r\n
 *	@see	NX_WDT_GetNumberOfModule
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
 *	@see		NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_SetBaseAddress,		NX_WDT_GetBaseAddress,
 *				NX_WDT_OpenModule,			NX_WDT_CloseModule,
 *				NX_WDT_CheckBusy,			NX_WDT_CanPowerDown
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
 *	@see		NX_WDT_GetPhysicalAddress,
 *				NX_WDT_SetBaseAddress,		NX_WDT_GetBaseAddress,
 *				NX_WDT_OpenModule,			NX_WDT_CloseModule,
 *				NX_WDT_CheckBusy,			NX_WDT_CanPowerDown
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
 *	@see		NX_WDT_GetPhysicalAddress,	NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_GetBaseAddress,
 *				NX_WDT_OpenModule,			NX_WDT_CloseModule,
 *				NX_WDT_CheckBusy,			NX_WDT_CanPowerDown
 */
void	NX_WDT_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
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
 *	@see		NX_WDT_GetPhysicalAddress,	NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_SetBaseAddress,
 *				NX_WDT_OpenModule,			NX_WDT_CloseModule,
 *				NX_WDT_CheckBusy,			NX_WDT_CanPowerDown
 */
U32		NX_WDT_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );

	return (U32)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Initialize is successed. \r\n
 *				\b CFALSE	indicates that Initialize is failed.
 *	@see		NX_WDT_GetPhysicalAddress,	NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_SetBaseAddress,		NX_WDT_GetBaseAddress,
 *				NX_WDT_CloseModule,
 *				NX_WDT_CheckBusy,			NX_WDT_CanPowerDown
 */
CBOOL	NX_WDT_OpenModule( U32 ModuleIndex )
{
	register struct NX_WDT_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIODW(&pRegister->WTCON) == 0x8021 );
	NX_ASSERT( ReadIODW(&pRegister->WTDAT) == 0x8000 );
	NX_ASSERT( ReadIODW(&pRegister->WTCNT) == 0x8000 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Deinitialize is successed. \r\n
 *				\b CFALSE	indicates that Deinitialize is failed.
 *	@see		NX_WDT_GetPhysicalAddress,	NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_SetBaseAddress,		NX_WDT_GetBaseAddress,
 *				NX_WDT_OpenModule,
 *				NX_WDT_CheckBusy,			NX_WDT_CanPowerDown
 */
CBOOL	NX_WDT_CloseModule( U32 ModuleIndex )
{
	register struct NX_WDT_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIODW(&pRegister->WTCON, 0x8021);
	WriteIODW(&pRegister->WTDAT, 0x8000);
	WriteIODW(&pRegister->WTCNT, 0x8000);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Module is Busy. \r\n
 *				\b CFALSE	indicates that Module is NOT Busy.
 *	@see		NX_WDT_GetPhysicalAddress,	NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_SetBaseAddress,		NX_WDT_GetBaseAddress,
 *				NX_WDT_OpenModule,			NX_WDT_CloseModule,
 *				NX_WDT_CanPowerDown
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
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \r\n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 *	@see		NX_WDT_GetPhysicalAddress,	NX_WDT_GetSizeOfRegisterSet,
 *				NX_WDT_SetBaseAddress,		NX_WDT_GetBaseAddress,
 *				NX_WDT_OpenModule,			NX_WDT_CloseModule,
 *				NX_WDT_CheckBusy
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
 *	@see		NX_WDT_GetInterruptNumber,
 *				NX_WDT_GetInterruptEnable,			NX_WDT_GetInterruptPending,
 *				NX_WDT_ClearInterruptPending,		NX_WDT_SetInterruptEnableAll,
 *				NX_WDT_GetInterruptEnableAll,		NX_WDT_GetInterruptPendingAll,
 *				NX_WDT_ClearInterruptPendingAll,	NX_WDT_GetInterruptPendingNumber
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
 *	@param[in]	Enable	\b CTRUE	indicates that Interrupt Enable. \r\n
 *						\b CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 *	@see		NX_WDT_GetInterruptNumber,
 *				NX_WDT_GetInterruptEnable,			NX_WDT_GetInterruptPending,
 *				NX_WDT_ClearInterruptPending,		NX_WDT_SetInterruptEnableAll,
 *				NX_WDT_GetInterruptEnableAll,		NX_WDT_GetInterruptPendingAll,
 *				NX_WDT_ClearInterruptPendingAll,	NX_WDT_GetInterruptPendingNumber
 */
void	NX_WDT_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	2;
	const U32	PEND_MASK	=	0x1 << PEND_POS;

	register struct NX_WDT_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIODW(&pRegister->WTCON) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIODW(&pRegister->WTCON, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Interrupt is enabled. \r\n
 *				\b CFALSE	indicates that Interrupt is disabled.
 *	@see		NX_WDT_GetInterruptNumber,
 *				NX_WDT_GetInterruptEnable,			NX_WDT_GetInterruptPending,
 *				NX_WDT_ClearInterruptPending,		NX_WDT_SetInterruptEnableAll,
 *				NX_WDT_GetInterruptEnableAll,		NX_WDT_GetInterruptPendingAll,
 *				NX_WDT_ClearInterruptPendingAll,	NX_WDT_GetInterruptPendingNumber
 */
CBOOL	NX_WDT_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	2;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@see		NX_WDT_GetInterruptNumber,
 *				NX_WDT_GetInterruptEnable,			NX_WDT_GetInterruptPending,
 *				NX_WDT_ClearInterruptPending,		NX_WDT_SetInterruptEnableAll,
 *				NX_WDT_GetInterruptEnableAll,		NX_WDT_GetInterruptPendingAll,
 *				NX_WDT_ClearInterruptPendingAll,	NX_WDT_GetInterruptPendingNumber
 */
void	NX_WDT_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x0 << PEND_POS;

	register struct NX_WDT_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIODW(&pRegister->WTCLRINT, ((1 << IntNum) & PEND_MASK) << PEND_POS );
}

//--------------------------------------------------------------------------
// Configuration operations
//--------------------------------------------------------------------------
void	NX_WDT_SetWTCON( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON, value);
}
U32		NX_WDT_GetWTCON( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTCON));
}

void NX_WDT_SetResetEnable( U32 ModuleIndex, CBOOL enable )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	
	__g_ModuleVariables[ModuleIndex].pRegister->WTCON 
		= NX_BIT_SetBit32 (	__g_ModuleVariables[ModuleIndex].pRegister->WTCON, (U32)enable, 2  );
}

void	NX_WDT_SetWTDAT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTDAT, value);
}
U32		NX_WDT_GetWTDAT( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTDAT));
}

void	NX_WDT_SetWTCNT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTCNT, value);
}
U32		NX_WDT_GetWTCNT( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTCNT));
}

void	NX_WDT_SetWTCLRINT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_WDT_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->WTCLRINT, value);
}




























