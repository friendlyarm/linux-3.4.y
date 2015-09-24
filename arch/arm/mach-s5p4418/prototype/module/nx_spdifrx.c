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
//	Module		: SPDIFRX
//	File		: nx_spdifrx.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120830 first implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_spdifrx.h>

static	struct
{
	struct NX_SPDIFRX_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_SPDIFRX_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicates that Initialize is successed.\r\n
 *			\b CFALSE indicates that Initialize is failed.\r\n
 *	@see	NX_SPDIFRX_GetNumberOfModule
 */
CBOOL	NX_SPDIFRX_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_SPDIFRX_MODULE; i++ )
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
 *	@see		NX_SPDIFRX_Initialize
 */
U32		NX_SPDIFRX_GetNumberOfModule( void )
{
	return NUMBER_OF_SPDIFRX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 *	@see		NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_SetBaseAddress,		NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_OpenModule,			NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CheckBusy,			NX_SPDIFRX_CanPowerDown
 */
U32		NX_SPDIFRX_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SPDIFRX ) };

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
U32 NX_SPDIFRX_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
	const U32 ResetPinNumber[2][NUMBER_OF_SPDIFRX_MODULE] =
	{
		{ RESETINDEX_LIST( SPDIFRX, PRESETn )}
	};

	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_SPDIFRX_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,
 *				NX_SPDIFRX_SetBaseAddress,		NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_OpenModule,			NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CheckBusy,			NX_SPDIFRX_CanPowerDown
 */
U32		NX_SPDIFRX_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_SPDIFRX_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,	NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_OpenModule,			NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CheckBusy,			NX_SPDIFRX_CanPowerDown
 */
void	NX_SPDIFRX_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SPDIFRX_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,	NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_SetBaseAddress,
 *				NX_SPDIFRX_OpenModule,			NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CheckBusy,			NX_SPDIFRX_CanPowerDown
 */
U32		NX_SPDIFRX_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return (U32)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Initialize is successed. \r\n
 *				\b CFALSE	indicates that Initialize is failed.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,	NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_SetBaseAddress,		NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CheckBusy,			NX_SPDIFRX_CanPowerDown
 */
CBOOL	NX_SPDIFRX_OpenModule( U32 ModuleIndex )
{
	register struct NX_SPDIFRX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_CTRL) == 0x0 );
	NX_ASSERT( ReadIODW(&pRegister->SPDIF_ENBIRQ) == 0x0 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Deinitialize is successed. \r\n
 *				\b CFALSE	indicates that Deinitialize is failed.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,	NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_SetBaseAddress,		NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_OpenModule,
 *				NX_SPDIFRX_CheckBusy,			NX_SPDIFRX_CanPowerDown
 */
CBOOL	NX_SPDIFRX_CloseModule( U32 ModuleIndex )
{
	register struct NX_SPDIFRX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIODW(&pRegister->SPDIF_CTRL, 0x0);
	WriteIODW(&pRegister->SPDIF_ENBIRQ, 0x0);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Module is Busy. \r\n
 *				\b CFALSE	indicates that Module is NOT Busy.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,	NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_SetBaseAddress,		NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_OpenModule,			NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CanPowerDown
 */
CBOOL	NX_SPDIFRX_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \r\n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 *	@see		NX_SPDIFRX_GetPhysicalAddress,	NX_SPDIFRX_GetSizeOfRegisterSet,
 *				NX_SPDIFRX_SetBaseAddress,		NX_SPDIFRX_GetBaseAddress,
 *				NX_SPDIFRX_OpenModule,			NX_SPDIFRX_CloseModule,
 *				NX_SPDIFRX_CheckBusy
 */
CBOOL	NX_SPDIFRX_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
U32		NX_SPDIFRX_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	SPDIFRXInterruptNumber[NUMBER_OF_SPDIFRX_MODULE] =
				{
					INTNUM_LIST(SPDIFRX)
				};

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return	SPDIFRXInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@param[in]	Enable	\b CTRUE	indicates that Interrupt Enable. \r\n
 *						\b CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
void	NX_SPDIFRX_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0xf << PEND_POS;

	register struct NX_SPDIFRX_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIODW(&pRegister->SPDIF_ENBIRQ) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIODW(&pRegister->SPDIF_ENBIRQ, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Interrupt is enabled. \r\n
 *				\b CFALSE	indicates that Interrupt is disabled.
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFRX_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Pending is seted. \r\n
 *				\b CFALSE	indicates that Pending is Not Seted.
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFRX_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	4;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
void	NX_SPDIFRX_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf << PEND_POS;

	register struct NX_SPDIFRX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIODW(&pRegister->SPDIF_ENBIRQ, ((1 << IntNum) & PEND_MASK) << PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	\b CTRUE	indicates that Set to all interrupt enable. \r\n
 *						\b CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
void	NX_SPDIFRX_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0xf;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK << PEND_POS;
	}

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that At least one( or more ) interrupt is enabled. \r\n
 *				\b CFALSE	indicates that All interrupt is disabled.
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFRX_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0xf;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) & (INT_MASK << PEND_POS) )
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
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
CBOOL	NX_SPDIFRX_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) & (PEND_MASK << PEND_POS) )
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
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
void	NX_SPDIFRX_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf << PEND_POS;
	const U32	ENB_MASK	=	0xf;

	register struct NX_SPDIFRX_RegisterSet*	pRegister;
	register U32 IRQEnb;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	IRQEnb	=	ReadIODW(&pRegister->SPDIF_ENBIRQ) & ENB_MASK;

	WriteIODW(&pRegister->SPDIF_ENBIRQ, (PEND_MASK|IRQEnb) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 *	@see		NX_SPDIFRX_GetInterruptNumber,
 *				NX_SPDIFRX_GetInterruptEnable,			NX_SPDIFRX_GetInterruptPending,
 *				NX_SPDIFRX_ClearInterruptPending,		NX_SPDIFRX_SetInterruptEnableAll,
 *				NX_SPDIFRX_GetInterruptEnableAll,		NX_SPDIFRX_GetInterruptPendingAll,
 *				NX_SPDIFRX_ClearInterruptPendingAll,	NX_SPDIFRX_GetInterruptPendingNumber
 */
U32		NX_SPDIFRX_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_SPDIFRX_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	(ReadIODW(&pRegister->SPDIF_ENBIRQ)>>PEND_POS) & PEND_MASK;

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
U32		NX_SPDIFRX_GetDMAIndex( U32 ModuleIndex )
{
	const U32 SPDIFRXDmaIndex[NUMBER_OF_SPDIFRX_MODULE] =
				{ DMAINDEX_LIST(SPDIFRX)  };

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return SPDIFRXDmaIndex[ModuleIndex];
}

U32		NX_SPDIFRX_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return 32;
}

//--------------------------------------------------------------------------
// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_SPDIFRX_Set_SPDCTRL( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, value);
}
U32		NX_SPDIFRX_Get_SPDCTRL( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL));
}

void	NX_SPDIFRX_Set_SPDENBIRQ( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ, value);
}
U32		NX_SPDIFRX_Get_SPDENBIRQ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ));
}

U32		NX_SPDIFRX_Get_REGUSERA0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA0));
}
U32		NX_SPDIFRX_Get_REGUSERA1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA1));
}
U32		NX_SPDIFRX_Get_REGUSERA2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA2));
}
U32		NX_SPDIFRX_Get_REGUSERA3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA3));
}
U32		NX_SPDIFRX_Get_REGUSERA4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA4));
}
U32		NX_SPDIFRX_Get_REGUSERA5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA5));
}
U32		NX_SPDIFRX_Get_REGUSERB0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB0));
}
U32		NX_SPDIFRX_Get_REGUSERB1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB1));
}
U32		NX_SPDIFRX_Get_REGUSERB2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB2));
}
U32		NX_SPDIFRX_Get_REGUSERB3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB3));
}
U32		NX_SPDIFRX_Get_REGUSERB4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB4));
}
U32		NX_SPDIFRX_Get_REGUSERB5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB5));
}
U32		NX_SPDIFRX_Get_REGSTATA0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA0));
}
U32		NX_SPDIFRX_Get_REGSTATA1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA1));
}
U32		NX_SPDIFRX_Get_REGSTATA2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA2));
}
U32		NX_SPDIFRX_Get_REGSTATA3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA3));
}
U32		NX_SPDIFRX_Get_REGSTATA4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA4));
}
U32		NX_SPDIFRX_Get_REGSTATA5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA5));
}
U32		NX_SPDIFRX_Get_REGSTATB0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB0));
}
U32		NX_SPDIFRX_Get_REGSTATB1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB1));
}
U32		NX_SPDIFRX_Get_REGSTATB2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB2));
}
U32		NX_SPDIFRX_Get_REGSTATB3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB3));
}
U32		NX_SPDIFRX_Get_REGSTATB4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB4));
}
U32		NX_SPDIFRX_Get_REGSTATB5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB5));
}
//@}