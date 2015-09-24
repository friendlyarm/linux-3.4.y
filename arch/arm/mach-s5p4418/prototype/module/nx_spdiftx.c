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
//	Module		: SPDIFTX
//	File		: nx_spdiftx.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120814 first implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_spdiftx.h>

static	struct
{
	struct NX_SPDIFTX_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_SPDIFTX_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicates that Initialize is successed.\r\n
 *			\b CFALSE indicates that Initialize is failed.\r\n
 *	@see	NX_SPDIFTX_GetNumberOfModule
 */
CBOOL	NX_SPDIFTX_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_SPDIFTX_MODULE; i++ )
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
 *	@see		NX_SPDIFTX_Initialize
 */
U32		NX_SPDIFTX_GetNumberOfModule( void )
{
	return NUMBER_OF_SPDIFTX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 *	@see		NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_SetBaseAddress,		NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_OpenModule,			NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CheckBusy,			NX_SPDIFTX_CanPowerDown
 */
U32		NX_SPDIFTX_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SPDIFTX ) };

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,
 *				NX_SPDIFTX_SetBaseAddress,		NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_OpenModule,			NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CheckBusy,			NX_SPDIFTX_CanPowerDown
 */
U32		NX_SPDIFTX_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_SPDIFTX_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,	NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_OpenModule,			NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CheckBusy,			NX_SPDIFTX_CanPowerDown
 */
void	NX_SPDIFTX_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SPDIFTX_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,	NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_SetBaseAddress,
 *				NX_SPDIFTX_OpenModule,			NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CheckBusy,			NX_SPDIFTX_CanPowerDown
 */
U32		NX_SPDIFTX_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return (U32)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Initialize is successed. \r\n
 *				\b CFALSE	indicates that Initialize is failed.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,	NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_SetBaseAddress,		NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CheckBusy,			NX_SPDIFTX_CanPowerDown
 */
CBOOL	NX_SPDIFTX_OpenModule( U32 ModuleIndex )
{
	register struct NX_SPDIFTX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_CLKCON) == 0x2 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_CON) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_BSTAS) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_CSTAS) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_DAT) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_CNT) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_BSTAS_SHD) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_CNT_SHD) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->USERBIT1) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->USERBIT2) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->USERBIT3) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->USERBIT1_SHD) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->USERBIT2_SHD) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->USERBIT3_SHD) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->VERSION_INFO) == 0xD );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Deinitialize is successed. \r\n
 *				\b CFALSE	indicates that Deinitialize is failed.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,	NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_SetBaseAddress,		NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_OpenModule,
 *				NX_SPDIFTX_CheckBusy,			NX_SPDIFTX_CanPowerDown
 */
CBOOL	NX_SPDIFTX_CloseModule( U32 ModuleIndex )
{
	register struct NX_SPDIFTX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIODW(&pRegister->SPDIF_CLKCON, 0x2);
	WriteIODW(&pRegister->SPDIF_CON, 0x0);
	WriteIODW(&pRegister->SPDIF_BSTAS, 0x0);
	WriteIODW(&pRegister->SPDIF_CSTAS, 0x0);
	WriteIODW(&pRegister->SPDIF_DAT, 0x0);
	WriteIODW(&pRegister->SPDIF_CNT, 0x0);
	WriteIODW(&pRegister->SPDIF_BSTAS_SHD, 0x0);
	WriteIODW(&pRegister->SPDIF_CNT_SHD, 0x0);
	WriteIODW(&pRegister->USERBIT1, 0x0);
	WriteIODW(&pRegister->USERBIT2, 0x0);
	WriteIODW(&pRegister->USERBIT3, 0x0);
	WriteIODW(&pRegister->USERBIT1_SHD, 0x0);
	WriteIODW(&pRegister->USERBIT2_SHD, 0x0);
	WriteIODW(&pRegister->USERBIT3_SHD, 0x0);
	WriteIODW(&pRegister->VERSION_INFO, 0xD);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Module is Busy. \r\n
 *				\b CFALSE	indicates that Module is NOT Busy.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,	NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_SetBaseAddress,		NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_OpenModule,			NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CanPowerDown
 */
CBOOL	NX_SPDIFTX_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \r\n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 *	@see		NX_SPDIFTX_GetPhysicalAddress,	NX_SPDIFTX_GetSizeOfRegisterSet,
 *				NX_SPDIFTX_SetBaseAddress,		NX_SPDIFTX_GetBaseAddress,
 *				NX_SPDIFTX_OpenModule,			NX_SPDIFTX_CloseModule,
 *				NX_SPDIFTX_CheckBusy
 */
CBOOL	NX_SPDIFTX_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return CTRUE;
}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_SPDIFTX_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.\n
 *				It is equal to CLOCKINDEX_OF_SPDIFTX?_MODULE in <nx_chip.h>.
 *	@see		NX_CLKGEN_SetClockDivisorEnable,
 *				NX_CLKGEN_GetClockDivisorEnable,
 *				NX_CLKGEN_SetClockSource,
 *				NX_CLKGEN_GetClockSource,
 *				NX_CLKGEN_SetClockDivisor,
 *				NX_CLKGEN_GetClockDivisor
 */
U32 NX_SPDIFTX_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( SPDIFTX )
	};
	NX_CASSERT( NUMBER_OF_SPDIFTX_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_SPDIFTX?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_SPDIFTX_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
	const U32 ResetPinNumber[2][NUMBER_OF_SPDIFTX_MODULE] =
	{
		{ RESETINDEX_LIST( SPDIFTX, PRESETn )}
	};

	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
U32		NX_SPDIFTX_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	SPDIFTXInterruptNumber[NUMBER_OF_SPDIFTX_MODULE] =
				{
					INTNUM_LIST(SPDIFTX)
				};

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return	SPDIFTXInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@param[in]	Enable	\b CTRUE	indicates that Interrupt Enable. \r\n
 *						\b CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
void	NX_SPDIFTX_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x8540 << PEND_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIODW(&pRegister->SPDIF_CON) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIODW(&pRegister->SPDIF_CON, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Interrupt is enabled. \r\n
 *				\b CFALSE	indicates that Interrupt is disabled.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFTX_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Pending is seted. \r\n
 *				\b CFALSE	indicates that Pending is Not Seted.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFTX_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
void	NX_SPDIFTX_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80 << PEND_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIODW(&pRegister->SPDIF_CON, ((1 << IntNum) & PEND_MASK) << PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	\b CTRUE	indicates that Set to all interrupt enable. \r\n
 *						\b CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
void	NX_SPDIFTX_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0x8540;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK << PEND_POS;
	}

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that At least one( or more ) interrupt is enabled. \r\n
 *				\b CFALSE	indicates that All interrupt is disabled.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFTX_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0x8540;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) & (INT_MASK << PEND_POS) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that At least one( or more ) pending is seted. \r\n
 *				\b CFALSE	indicates that All pending is NOT seted.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFTX_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) & (PEND_MASK << PEND_POS) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		None.
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
void	NX_SPDIFTX_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80 << PEND_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIODW(&pRegister->SPDIF_CON, (PEND_MASK<<PEND_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 *	@see		NX_SPDIFTX_GetInterruptNumber,
 *				NX_SPDIFTX_GetInterruptEnable,			NX_SPDIFTX_GetInterruptPending,
 *				NX_SPDIFTX_ClearInterruptPending,		NX_SPDIFTX_SetInterruptEnableAll,
 *				NX_SPDIFTX_GetInterruptEnableAll,		NX_SPDIFTX_GetInterruptPendingAll,
 *				NX_SPDIFTX_ClearInterruptPendingAll,	NX_SPDIFTX_GetInterruptPendingNumber
 */
U32		NX_SPDIFTX_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80 << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_SPDIFTX_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	(ReadIODW(&pRegister->SPDIF_CON)>>PEND_POS) & PEND_MASK;

	for( PendingIndex=0 ; PendingIndex<=20 ; PendingIndex++)
		if(Pend & ((U32)0x1)<<PendingIndex)
			break;

	if(PendingIndex > 20)
		return -1;
	else
		return PendingIndex;
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
U32		NX_SPDIFTX_GetDMAIndex( U32 ModuleIndex )
{
	const U32 SPDIFTXDmaIndex[NUMBER_OF_SPDIFTX_MODULE] =
				{ DMAINDEX_LIST(SPDIFTX)  };

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return SPDIFTXDmaIndex[ModuleIndex];
}

U32		NX_SPDIFTX_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return 32;
}

//--------------------------------------------------------------------------
// Configuration operations
//--------------------------------------------------------------------------
void	NX_SPDIFTX_SetSPDCLKCON( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CLKCON, value);
}
U32		NX_SPDIFTX_GetSPDCLKCON( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CLKCON));
}

void	NX_SPDIFTX_SetSPDCON( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON, value);
}
U32		NX_SPDIFTX_GetSPDCON( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON));
}

void	NX_SPDIFTX_SetSPDBSTAS( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_BSTAS, value);
}
U32		NX_SPDIFTX_GetSPDBSTAS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_BSTAS));
}

void	NX_SPDIFTX_SetSPDCSTAS( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CSTAS, value);
}
U32		NX_SPDIFTX_GetSPDCSTAS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CSTAS));
}

void	NX_SPDIFTX_SetSPDDAT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_DAT, value);
}

void	NX_SPDIFTX_SetSPDCNT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CNT, value);
}

U32		NX_SPDIFTX_GetSPDBSTAS_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_BSTAS_SHD));
}

U32		NX_SPDIFTX_GetSPDCNT_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CNT_SHD));
}

void	NX_SPDIFTX_SetUSERBIT1( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT1, value);
}
U32		NX_SPDIFTX_GetUSERBIT1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT1));
}

void	NX_SPDIFTX_SetUSERBIT2( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT2, value);
}
U32		NX_SPDIFTX_GetUSERBIT2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT2));
}

void	NX_SPDIFTX_SetUSERBIT3( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT3, value);
}
U32		NX_SPDIFTX_GetUSERBIT3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT3));
}

U32		NX_SPDIFTX_GetUSERBIT1_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT1_SHD));
}

U32		NX_SPDIFTX_GetUSERBIT2_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT2_SHD));
}

U32		NX_SPDIFTX_GetUSERBIT3_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT3_SHD));
}

U32		NX_SPDIFTX_GetVERSION_INFO( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->VERSION_INFO));
}































