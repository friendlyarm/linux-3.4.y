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
//	Module		: DPC
//	File		: nx_dpc.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------

#include "nx_chip.h"
#include "nx_dpc.h"
#include "nx_dualdisplay.h"


static	struct
{
	struct NX_DPC_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_DPC_MODULE] = { {CNULL,}, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicates that	Initialize is successed.\n
 *			\b CFALSE	Initialize is failed.\n
 *	@see								NX_DPC_GetNumberOfModule
 */
CBOOL	NX_DPC_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_DPC_MODULE; i++ )
		{
			__g_ModuleVariables[i].pRegister = CNULL;
		}

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_DPC_Initialize
 */
U32		NX_DPC_GetNumberOfModule( void )
{
	return NUMBER_OF_DPC_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Module's physical address
 *	@see		NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_SetBaseAddress,		NX_DPC_GetBaseAddress,
 *				NX_DPC_OpenModule,			NX_DPC_CloseModule,
 *				NX_DPC_CheckBusy,			NX_DPC_CanPowerDown
 */
U32		NX_DPC_GetPhysicalAddress( U32 ModuleIndex )
{
	const U32 PhysicalAddr[] =
	{
		PHY_BASEADDR_LIST( DPC )
	};
	NX_CASSERT( NUMBER_OF_DPC_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_DPC_GetPhysicalAddress,
 *				NX_DPC_SetBaseAddress,		NX_DPC_GetBaseAddress,
 *				NX_DPC_OpenModule,			NX_DPC_CloseModule,
 *				NX_DPC_CheckBusy,			NX_DPC_CanPowerDown
 */
U32		NX_DPC_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DPC_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_DPC_GetPhysicalAddress,	NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_GetBaseAddress,
 *				NX_DPC_OpenModule,			NX_DPC_CloseModule,
 *				NX_DPC_CheckBusy,			NX_DPC_CanPowerDown
 */
void	NX_DPC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_DPC_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Module's base address.
 *	@see		NX_DPC_GetPhysicalAddress,	NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_SetBaseAddress,
 *				NX_DPC_OpenModule,			NX_DPC_CloseModule,
 *				NX_DPC_CheckBusy,			NX_DPC_CanPowerDown
 */
void*	NX_DPC_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that	Initialize is successed. \n
 *				\b CFALSE	Initialize is failed.
 *	@see		NX_DPC_GetPhysicalAddress,	NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_SetBaseAddress,		NX_DPC_GetBaseAddress,
 *				NX_DPC_CloseModule,
 *				NX_DPC_CheckBusy,			NX_DPC_CanPowerDown
 */
CBOOL	NX_DPC_OpenModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that	Deinitialize is successed. \n
 *				\b CFALSE	Deinitialize is failed.
 *	@see		NX_DPC_GetPhysicalAddress,	NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_SetBaseAddress,		NX_DPC_GetBaseAddress,
 *				NX_DPC_OpenModule,
 *				NX_DPC_CheckBusy,			NX_DPC_CanPowerDown
 */
CBOOL	NX_DPC_CloseModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that	Module is Busy. \n
 *				\b CFALSE	Module is NOT Busy.
 *	@see		NX_DPC_GetPhysicalAddress,	NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_SetBaseAddress,		NX_DPC_GetBaseAddress,
 *				NX_DPC_OpenModule,			NX_DPC_CloseModule,
 *				NX_DPC_CanPowerDown
 */
CBOOL	NX_DPC_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that	Ready to enter power-down stage. \n
 *				\b CFALSE	This module can't enter to power-down stage.
 *	@see		NX_DPC_GetPhysicalAddress,	NX_DPC_GetSizeOfRegisterSet,
 *				NX_DPC_SetBaseAddress,		NX_DPC_GetBaseAddress,
 *				NX_DPC_OpenModule,			NX_DPC_CloseModule,
 *				NX_DPC_CheckBusy
 */
CBOOL	NX_DPC_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Interrupt number
 *	@see											NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
S32		NX_DPC_GetInterruptNumber( U32 ModuleIndex )
{
	//static const U32 IntNumDPC[NUMBER_OF_DPC_MODULE] = { INTNUM_OF_DPC0_MODULE };

	static const U32 IntNumDPC[NUMBER_OF_DPC_MODULE] =
		{ INTNUM_OF_DUALDISPLAY_MODULE_PRIMIRQ,
		  INTNUM_OF_DUALDISPLAY_MODULE_SECONDIRQ };

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );

	return	IntNumDPC[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	IntNum	Interrupt Number ( 0 : VSYNC ).
 *	@param[in]	Enable	\b CTRUE	indicates that	Interrupt Enable. \n
 *						\b CFALSE	Interrupt Disable.
 *	@return		None.
 *	@remarks	DPC Module have one interrupt. So always \e IntNum set to 0.
 *	@see		NX_DPC_GetInterruptNumber,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
void	NX_DPC_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable )
{
	const U32	INTENB_POS		= 11;
	const U32	INTENB_MASK		= 1UL << INTENB_POS;
	const U32	INTPEND_POS		= 10;
	const U32	INTPEND_MASK	= 1UL << INTPEND_POS;

	register U32 regvalue;
	register struct NX_DPC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->DPCCTRL0;

	regvalue &= ~(INTENB_MASK | INTPEND_MASK);
	regvalue |= (U32)Enable << INTENB_POS;

//	pRegister->DPCCTRL0 = regvalue;
	WriteIO32(&pRegister->DPCCTRL0, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	IntNum	Interrupt Number	( 0 : VSYNC ).
 *	@return		\b CTRUE	indicates that	Interrupt is enabled. \n
 *				\b CFALSE	Interrupt is disabled.
 *	@remarks	DPC Module have one interrupt. So always \e IntNum set to 0.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *													NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
CBOOL	NX_DPC_GetInterruptEnable( U32 ModuleIndex, S32 IntNum )
{
	const U32	INTENB_POS	= 11;
	const U32	INTENB_MASK	= 1UL << INTENB_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & INTENB_MASK) >> INTENB_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	\n
 *							- EnableFlag[0] : Set Vertical Sync interrupt enable or disable. \n
 *	@return		None.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
void	NX_DPC_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag )
{
	const U32	INTENB_POS		= 11;
	const U32	INTENB_MASK		= 1<<INTENB_POS;

	const U32	INTPEND_POS		= 10;
	const U32	INTPEND_MASK	= 1<<INTPEND_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->DPCCTRL0 & ~(INTPEND_MASK | INTENB_MASK);

//	pRegister->DPCCTRL0 = (U32)( ReadValue | (EnableFlag&0x01)<<INTENB_POS );
	WriteIO32(&pRegister->DPCCTRL0, (U32)( ReadValue | (EnableFlag&0x01)<<INTENB_POS ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Current setting value of interrupt. \n
 *				"1" means interrupt is enabled. \n
 *				"0" means interrupt is disabled. \n
 *				- Return Value[0] : Vertical Sync	interrupt's setting value. \n
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *													NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
U32		NX_DPC_GetInterruptEnable32( U32 ModuleIndex )
{
	const U32	INTENB_POS	= 11;
	const U32	INTENB_MASK	= 1<<INTENB_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)((__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & INTENB_MASK) >> INTENB_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	IntNum	Interrupt Number ( 0 : VSYNC ).
 *	@return		\b CTRUE	indicates that	Pending is seted. \n
 *				\b CFALSE	Pending is Not Seted.
 *	@remarks	DPC Module have one interrupt. So always \e IntNum set to 0.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
CBOOL	NX_DPC_GetInterruptPending( U32 ModuleIndex, S32 IntNum )
{
	const U32	INTPEND_POS = 10;
	const U32	INTPEND_MASK = 1UL << INTPEND_POS;

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & INTPEND_MASK) >> INTPEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Current setting value of pending bit. \n
 *				"1" means pend bit is occured. \n
 *				"0" means pend bit is NOT occured. \n
 *				- Return Value[0] : Vertical Sync pending state. \n
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *													NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
U32		NX_DPC_GetInterruptPending32( U32 ModuleIndex )
{
	const U32	INTPEND_POS		= 10;
	const U32	INTPEND_MASK	= 1<<INTPEND_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)((__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & INTPEND_MASK )>>INTPEND_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@remarks	DPC Module have one interrupt. So always \e IntNum set to 0.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
void	NX_DPC_ClearInterruptPending( U32 ModuleIndex, S32 IntNum )
{
	const U32	INTPEND_POS = 10;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	regvalue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue  = pRegister->DPCCTRL0;
	regvalue |= 1UL << INTPEND_POS;

//	pRegister->DPCCTRL0 = regvalue;
	WriteIO32(&pRegister->DPCCTRL0, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	\n \n
 *								- PendingFlag[0] : Vertical Sync pending bit. \n
 *	@return		None.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *													NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
void	NX_DPC_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
	const U32	INTPEND_POS		= 10;
	const U32	INTPEND_MASK	= 1<<INTPEND_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->DPCCTRL0 & ~INTPEND_MASK;

//	pRegister->DPCCTRL0 = (U32)(ReadValue |((PendingFlag & 0x01) << INTPEND_POS));
	WriteIO32(&pRegister->DPCCTRL0, (U32)(ReadValue |((PendingFlag & 0x01) << INTPEND_POS)));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Enable	\b CTRUE	indicates that	Set to all interrupt enable. \n
 *						\b CFALSE	Set to all interrupt disable.
 *	@return		None.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
void	NX_DPC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	INTENB_POS		= 11;
	const U32	INTENB_MASK		= 1UL << INTENB_POS;
	const U32	INTPEND_POS		= 10;
	const U32	INTPEND_MASK	= 1UL << INTPEND_POS;

	register U32 regvalue;
	register struct NX_DPC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->DPCCTRL0;

	regvalue &= ~(INTENB_MASK | INTPEND_MASK);
	regvalue |= (U32)Enable << INTENB_POS;

//	pRegister->DPCCTRL0 = regvalue;
	WriteIO32(&pRegister->DPCCTRL0, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that	At least one( or more ) interrupt is enabled. \n
 *				\b CFALSE	All interrupt is disabled.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,	NX_DPC_SetInterruptEnableAll,
 *													NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
CBOOL	NX_DPC_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	INTENB_POS	= 11;
	const U32	INTENB_MASK	= 1UL << INTENB_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & INTENB_MASK) >> INTENB_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that	At least one( or more ) pending is seted. \n
 *				\b CFALSE	All pending is NOT seted.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,
 *				NX_DPC_ClearInterruptPendingAll,	NX_DPC_GetInterruptPendingNumber
 */
CBOOL	NX_DPC_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	INTPEND_POS		= 10;
	const U32	INTPEND_MASK	= 1UL << INTPEND_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & INTPEND_MASK) >> INTPEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		None.
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *													NX_DPC_GetInterruptPendingNumber
 */
void	NX_DPC_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	INTPEND_POS = 10;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	regvalue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue  = pRegister->DPCCTRL0;
	regvalue |= 1UL << INTPEND_POS;

//	pRegister->DPCCTRL0 = regvalue;
	WriteIO32(&pRegister->DPCCTRL0, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Pending Number( If all pending is not set then return -1 ). \n
 *				0 ( VSYNC )
 *	@see		NX_DPC_GetInterruptNumber,			NX_DPC_SetInterruptEnable,
 *				NX_DPC_GetInterruptEnable,			NX_DPC_SetInterruptEnable32,
 *				NX_DPC_GetInterruptEnable32,		NX_DPC_GetInterruptPending,
 *				NX_DPC_GetInterruptPending32,		NX_DPC_ClearInterruptPending,
 *				NX_DPC_ClearInterruptPending32,		NX_DPC_SetInterruptEnableAll,
 *				NX_DPC_GetInterruptEnableAll,		NX_DPC_GetInterruptPendingAll,
 *				NX_DPC_ClearInterruptPendingAll
 */
S32		NX_DPC_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	INTENB_POS	= 11;
	const U32	INTPEND_POS	= 10;

	register struct NX_DPC_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend = ((pRegister->DPCCTRL0>>INTENB_POS) && (pRegister->DPCCTRL0>>INTPEND_POS));

	if( Pend & 0x01 )
	{
		return 0;
	}

	return -1;
}

//------------------------------------------------------------------------------
// Clock Control Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Set a PCLK mode
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	mode	PCLK mode
 *	@return		None.
 *	@see										NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode )
{
	const U32 PCLKMODE_POS	=	3;

	register U32 regvalue;
	register struct NX_DPC_RegisterSet* pRegister;

	U32 clkmode=0;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	switch(mode)
	{
		case NX_PCLKMODE_DYNAMIC:	clkmode = 0;		break;
		case NX_PCLKMODE_ALWAYS:	clkmode = 1;		break;
		default: NX_ASSERT( CFALSE );
	}

	regvalue = pRegister->DPCCLKENB;

	regvalue &= ~(1UL<<PCLKMODE_POS);
	regvalue |= ( clkmode & 0x01 ) << PCLKMODE_POS;

//	pRegister->DPCCLKENB = regvalue;
	WriteIO32(&pRegister->DPCCLKENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current PCLK mode
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		Current PCLK mode
 *	@see		NX_DPC_SetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
NX_PCLKMODE	NX_DPC_GetClockPClkMode( U32 ModuleIndex )
{
	const U32 PCLKMODE_POS	= 3;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->DPCCLKENB & ( 1UL << PCLKMODE_POS ) )
	{
		return NX_PCLKMODE_ALWAYS;
	}

	return	NX_PCLKMODE_DYNAMIC;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock source of clock generator
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index	Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	ClkSrc	Select clock source of clock generator.\n
 *						0:PLL0, 1:PLL1, 2:SVLCK, 3:P(S)VCLK, 4:~P(S)VCLK, 5:AVCLK \n
 *						6:~SVLCK, 7:ClKGEN0's Output( Only use Clock generator1 )
 *	@return		None.
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.\n
 *				Only Clock generator 1 can set to ClkGEN0's output.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *												NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void	NX_DPC_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc )
{
	const U32 CLKSRCSEL_POS		= 2;
	const U32 CLKSRCSEL_MASK	= 0x07 << CLKSRCSEL_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	//NX_ASSERT( (0!=Index) || ( (2!=ClkSrc) && (ClkSrc<=5) ) );
	NX_ASSERT( (0!=Index) || ( (ClkSrc<=5) ) );
	//NX_ASSERT( (1!=Index) || ( (2!=ClkSrc) && (6!=ClkSrc) && (ClkSrc<=7) ) );
	NX_ASSERT( (1!=Index) || ( (ClkSrc<=7) ) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->DPCCLKGEN[Index][0];

	ReadValue &= ~CLKSRCSEL_MASK;
	ReadValue |= ClkSrc << CLKSRCSEL_POS;

//	pRegister->DPCCLKGEN[Index][0] = ReadValue;
	WriteIO32(&pRegister->DPCCLKGEN[Index][0], ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get clock source of specified clock generator.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index	Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@return		Clock source of clock generator \n
 *				0:PLL0, 1:PLL1, 2:SVLCK, 3:P(S)VCLK, 4:~P(S)VCLK, 5:AVCLK \n
 *				6:~SVLCK, 7:ClKGEN0's Output( Only use Clock generator1 )
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
U32				NX_DPC_GetClockSource( U32 ModuleIndex, U32 Index )
{
	const U32 CLKSRCSEL_POS		= 2;
	const U32 CLKSRCSEL_MASK	= 0x07 << CLKSRCSEL_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return ( __g_ModuleVariables[ModuleIndex].pRegister->DPCCLKGEN[Index][0] & CLKSRCSEL_MASK ) >> CLKSRCSEL_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock divisor of specified clock generator.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	Divisor		Clock divisor ( 1 ~ 256 ).
 *	@return		None.
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *												NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor )
{
	const U32 CLKDIV_POS	=	5;
	const U32 CLKDIV_MASK	=	((1<<8)-1) << CLKDIV_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( 1 <= Divisor && Divisor <= 256 );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	= pRegister->DPCCLKGEN[Index][0];

	ReadValue	&= ~CLKDIV_MASK;
	ReadValue	|= (Divisor-1) << CLKDIV_POS;

//	pRegister->DPCCLKGEN[Index][0] = ReadValue;
	WriteIO32(&pRegister->DPCCLKGEN[Index][0], ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get clock divisor of specified clock generator.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@return		Clock divisor ( 1 ~ 256 ).
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
U32				NX_DPC_GetClockDivisor( U32 ModuleIndex, U32 Index )
{
	const U32 CLKDIV_POS	=	5;
	const U32 CLKDIV_MASK	=	((1<<8)-1) << CLKDIV_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return ((__g_ModuleVariables[ModuleIndex].pRegister->DPCCLKGEN[Index][0] & CLKDIV_MASK) >> CLKDIV_POS) + 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set inverting of output clock
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	OutClkInv	\b CTRUE indicates that Output clock Invert (Rising Edge). \n
 *							\b CFALSE indicates that Output clock Normal (Fallng Edge).
 *	@return		None.
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *												NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockOutInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv )
{
	const U32 OUTCLKINV_POS		=	1;
	const U32 OUTCLKINV_MASK	=	1UL << OUTCLKINV_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( (0==OutClkInv) ||(1==OutClkInv) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	= pRegister->DPCCLKGEN[Index][0];

	ReadValue	&=	~OUTCLKINV_MASK;
	ReadValue	|=	OutClkInv << OUTCLKINV_POS;

//	pRegister->DPCCLKGEN[Index][0]	=	ReadValue;
	WriteIO32(&pRegister->DPCCLKGEN[Index][0], ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get invert status of output clock.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 )
 *	@return		\b CTRUE	indicates that Output clock is Inverted. \n
 *				\b CFALSE	indicates that Output clock is Normal.
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
CBOOL			NX_DPC_GetClockOutInv( U32 ModuleIndex, U32 Index )
{
	const U32 OUTCLKINV_POS		=	1;
	const U32 OUTCLKINV_MASK	=	1UL << OUTCLKINV_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->DPCCLKGEN[Index][0] & OUTCLKINV_MASK ) >> OUTCLKINV_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set output clock's delay( source clock/2 ns )
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 1: clock generator1 );
 *	@param[in]	bBypass		\b CTRUE indicates that source clock directly out.
 *							\b CFALSE indicates that source clock divide by 2 ( source clock /2 ns ).
 *	@return		None.
 *	@remarks	Only even divide can use this function.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *												NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockOutSelect( U32 ModuleIndex, U32 Index, CBOOL bBypass )
{
	const U32 OUTCLKSEL_POS		=	0;
	const U32 OUTCLKSEL_MASK	=	1UL << OUTCLKSEL_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Index );
	NX_ASSERT( (0==bBypass) ||(1==bBypass) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	= pRegister->DPCCLKGEN[Index][0];

	ReadValue	&=	~OUTCLKSEL_MASK;

	if( CFALSE == bBypass )
		ReadValue	|=	OUTCLKSEL_MASK;

//	pRegister->DPCCLKGEN[Index][0]	=	ReadValue;
	WriteIO32(&pRegister->DPCCLKGEN[Index][0], ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of output clock's delay( source clock/2 ns )
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 1: clock generator1 );
 *	@return		\b CTRUE	indicates that source clock directly out.
 *				\b CFALSE	indicates that source clock divide by 2 ( source clock /2 ns ).
 *	@remarks	Only even divide can use this function.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
CBOOL			NX_DPC_GetClockOutSelect( U32 ModuleIndex, U32 Index )
{
	const U32 OUTCLKSEL_POS		=	0;
	const U32 OUTCLKSEL_MASK	=	1UL << OUTCLKSEL_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Index );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->DPCCLKGEN[Index][0] & OUTCLKSEL_MASK )
	{
		return CFALSE;
	}
	else
	{
		return CTRUE;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set output clock polarity
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	bBypass		\b CTRUE indicates that data is latch out when source clock rising edge.
 *							\b CFALSE indicates that data is latch out when source clock falling edge.
 *	@return		None.
 *	@remarks
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *												NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockPolarity( U32 ModuleIndex, CBOOL bPolarity )
{
	const U32 CLKPOL_POS		=	2;
	const U32 CLKPOL_MASK	=	1UL << CLKPOL_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bPolarity) ||(1==bPolarity) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	= pRegister->DPCCTRL1;

	ReadValue	&=	~CLKPOL_MASK;

	if( CTRUE == bPolarity )
		ReadValue	|=	CLKPOL_MASK;

//	pRegister->DPCCTRL1	=	ReadValue;
	WriteIO32(&pRegister->DPCCTRL1, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of output clock polarity
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that data is latch out when source clock rising edge.
 *				\b CFALSE	indicates that data is latch out when source clock falling edge.
 *	@remarks	Only even divide can use this function.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
CBOOL			NX_DPC_GetClockPolarity( U32 ModuleIndex )
{
	const U32 CLKPOL_POS		=	2;
	const U32 CLKPOL_MASK	=	1UL << CLKPOL_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL1 & CLKPOL_MASK )
	{
		return CTRUE;
	}
	else
	{
		return CFALSE;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Select SVCLK Pad's direction.
 *	@param[in]	ModuleIndex	An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 1: clock generator1 );
 *	@param[in]	OutClkEnb	\b CTRUE indicate that SVCLK Pad is output. \n
 *							\b CFALSE indicate that SVCLK Pad is input. \n
 *	@return		None.
 *	@remarks	Only secondary DPC can select the pad direction. \n
 *				Primary DPC's pad direction is changed by GPIO setting( ALT Function ).
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *												NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockOutEnb( U32 ModuleIndex, U32 Index, CBOOL OutClkEnb )
{
	const U32 OUTCLKENB_POS		=	15;
	const U32 OUTCLKENB_MASK	=	1UL << OUTCLKENB_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	//NX_ASSERT( 1 == Index );
	NX_ASSERT( (0==OutClkEnb) ||(1==OutClkEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	= pRegister->DPCCLKGEN[Index][0];

	ReadValue	&=	~OUTCLKENB_MASK;

	//@modified choiyk 2012-10-08 오후 1:48:28
	// OutEnb가 HIGH Active이므로 이 부분을 바꿔준다.
	if( CTRUE == OutClkEnb )	// input
		ReadValue	|=	OUTCLKENB_MASK;

//	pRegister->DPCCLKGEN[Index][0]	=	ReadValue;
	WriteIO32(&pRegister->DPCCLKGEN[Index][0], ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Setting value of SVCLK Pad's direction.
 *	@param[in]	ModuleIndex	An index of module ( 0 : First DPC ).
 *	@param[in]	Index		Select clock generator( 1: clock generator1 );
 *	@return		\b CTRUE	indicate that SVCLK Pad is output. \n
 *				\b CFALSE	indicate that SVCLK Pad is input.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
CBOOL			NX_DPC_GetClockOutEnb( U32 ModuleIndex, U32 Index )
{
	const U32 OUTCLKENB_POS		=	15;
	const U32 OUTCLKENB_MASK	=	1UL << OUTCLKENB_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Index );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	//CLKGEN OutEnb : HIGH Active
	if( __g_ModuleVariables[ModuleIndex].pRegister->DPCCLKGEN[Index][0] & OUTCLKENB_MASK )
	{
		return CTRUE; // Input
	}
	else
	{
		return CFALSE; // Output
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock output delay of specifed clock generator
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index	Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	delay	Select clock output delay of clock generator.\n
 *						0:0ns, 1:0.5ns, 2:1.0ns, 3:1.5ns, 4:2.0ns, 5:2.5ns, \n
 *						6:3.0ns 7:3.5ns
 *	@return		None.
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.\n
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *												NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockOutDelay( U32 ModuleIndex, U32 Index, U32 delay )
{
	const U32 OUTCLKDELAY_POS	= 0;
	const U32 OUTCLKDELAY_MASK	= 0x1F << OUTCLKDELAY_POS ;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( 0x20 > delay );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue = pRegister->DPCCLKGEN[Index][1];

	ReadValue &= ~OUTCLKDELAY_MASK;
	ReadValue |= (U32)delay << OUTCLKDELAY_POS;

//	pRegister->DPCCLKGEN[Index][1] = ReadValue;
	WriteIO32(&pRegister->DPCCLKGEN[Index][1], ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get clock output delay of specifed clock generator
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Index	Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@return		Get clock output delay of specifed clock generator.\n
 *				0:0ns, 1:0.5ns, 2:1.0ns, 3:1.5ns, 4:2.0ns, 5:2.5ns, \n
 *				6:3.0ns 7:3.5ns
 *	@return		None.
 *	@remarks	DPC controller have two clock generator. so \e Index must set to 0 or 1.\n
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable,	NX_DPC_GetClockDivisorEnable
 */
U32				NX_DPC_GetClockOutDelay( U32 ModuleIndex, U32 Index )
{
	const U32 OUTCLKDELAY_POS = 0;
	const U32 OUTCLKDELAY_MASK = 0x1F << OUTCLKDELAY_POS ;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return ((__g_ModuleVariables[ModuleIndex].pRegister->DPCCLKGEN[Index][1] & OUTCLKDELAY_MASK) >> OUTCLKDELAY_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock generator's operation
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	Enable	\b CTRUE	indicates that Enable of clock generator. \n
 *						\b CFALSE	indicates that Disable of clock generator.
 *	@return		None.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *												NX_DPC_GetClockDivisorEnable
 */
void			NX_DPC_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable )
{
	const U32	CLKGENENB_POS	=	2;
	const U32	CLKGENENB_MASK	=	1UL << CLKGENENB_POS;

	register struct NX_DPC_RegisterSet*	pRegister;
	register U32 	ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	= pRegister->DPCCLKENB;

	ReadValue	&=	~CLKGENENB_MASK;
	ReadValue	|= (U32)Enable << CLKGENENB_POS;

//	pRegister->DPCCLKENB	=	ReadValue;
	WriteIO32(&pRegister->DPCCLKENB, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of clock generator's operation
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		\b CTRUE	indicates that Clock generator is enabled. \n
 *				\b CFALSE	indicates that Clock generator is disabled.
 *	@see		NX_DPC_SetClockPClkMode,		NX_DPC_GetClockPClkMode,
 *				NX_DPC_SetClockSource,			NX_DPC_GetClockSource,
 *				NX_DPC_SetClockDivisor,			NX_DPC_GetClockDivisor,
 *				NX_DPC_SetClockOutInv,			NX_DPC_GetClockOutInv,
 *				NX_DPC_SetClockOutSelect,		NX_DPC_GetClockOutSelect,
 *				NX_DPC_SetClockOutEnb,			NX_DPC_GetClockOutEnb,
 *				NX_DPC_SetClockOutDelay,		NX_DPC_GetClockOutDelay,
 *				NX_DPC_SetClockDivisorEnable
 */
CBOOL			NX_DPC_GetClockDivisorEnable( U32 ModuleIndex )
{
	const U32	CLKGENENB_POS	=	2;
	const U32	CLKGENENB_MASK	=	1UL << CLKGENENB_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->DPCCLKENB & CLKGENENB_MASK) >> CLKGENENB_POS );
}


//--------------------------------------------------------------------------
//	Display controller operations
//--------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable Display controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	bEnb	Set it to CTRUE to enable display controller.
 *	@return		None.
 *	@see		NX_DPC_GetDPCEnable
 */
void	NX_DPC_SetDPCEnable( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 INTPEND_POS	= 10;
	const U32 INTPEND_MASK	= 1UL << INTPEND_POS;

	const U32 DPCENB_POS	= 15;
	const U32 DPCENB_MASK	= 1UL << DPCENB_POS;

	register struct NX_DPC_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnb) ||(1==bEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	pRegister->DPCCTRL0;

	ReadValue	&=	~(INTPEND_MASK|DPCENB_MASK);
	ReadValue	|=	(U32)bEnb << DPCENB_POS;

//	pRegister->DPCCTRL0	=	ReadValue;
	WriteIO32(&pRegister->DPCCTRL0, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether DPC is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@return		CTRUE indicates DPC is enabled.\n
 *				CFALSE indicates DPC is disabled.
 *	@see		NX_DPC_SetDPCEnable
 */
CBOOL	NX_DPC_GetDPCEnable( U32 ModuleIndex )
{
	const U32 DPCENB_POS	= 15;
	const U32 DPCENB_MASK	= 1UL << DPCENB_POS;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0 & DPCENB_MASK) >> DPCENB_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set delay values for data and sync signals.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	DelayRGB_PVD	Specifies the delay value for RGB/PVD signal, 0 ~ 16.
 *	@param[in]	DelayHS_CP1		Specifies the delay value for HSYNC/CP1 signal, 0 ~ 63.
 *	@param[in]	DelayVS_FRAM	Specifies the delay value for VSYNC/FRAM signal, 0 ~ 63.
 *	@param[in]	DelayDE_CP2		Specifies the delay value for DE/CP2 signal, 0 ~ 63.
 *	@return		None.
 *	@remarks	Set delay value for TFT LCD's data and sync signal.\n
 *				\b TFT \b LCD \n
 *				The delay valus for data is generally '0' for normal operation.
 *				but the delay values for sync signals depend on the output format.
 *				The unit is VCLK2.\n
 *				The setting values for normal operation is as follows,
 *	@code
 *		+-----------------------+-----------+-------------------------------+
 *		|		FORMAT			| DelayRGB	| DelayHS, VS, DE				|
 *		+-----------------------+-----------+-------------------------------+
 *		| RGB					|	0		|				4				|
 *		+-----------------------+-----------+-------------------------------+
 *		| MRGB					|	0		|				8				|
 *		+-----------------------+-----------+-------------------------------+
 *		| ITU-R BT.601A			|	0		|				6				|
 *		+-----------------------+-----------+-------------------------------+
 *		| ITU-R BT.656 / 601B	|	0		|				12				|
 *		+-----------------------+-----------+-------------------------------+
 *	@endcode
 *	@see		NX_DPC_GetDelay
 */
void	NX_DPC_SetDelay( U32 ModuleIndex, U32 DelayRGB_PVD, U32 DelayHS_CP1, U32 DelayVS_FRAM, U32 DelayDE_CP2 )
{
	const U32 INTPEND_MASK	= 1U<<10;
	const U32 DELAYRGB_POS	= 4;
	const U32 DELAYRGB_MASK	= 0xFU<<DELAYRGB_POS;
	register U32 temp;

	const U32 DELAYDE_POS	= 0;
	const U32 DELAYVS_POS	= 8;
	const U32 DELAYHS_POS	= 0;

	register struct NX_DPC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 16 > DelayRGB_PVD );
	NX_ASSERT( 64 > DelayHS_CP1 );
	NX_ASSERT( 64 > DelayVS_FRAM );
	NX_ASSERT( 64 > DelayDE_CP2 );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	temp  = pRegister->DPCCTRL0;
	temp &= (U32)~(INTPEND_MASK | DELAYRGB_MASK);	// unmask intpend & DELAYRGB bits.
	temp  = (U32)(temp | (DelayRGB_PVD<<DELAYRGB_POS));

//	pRegister->DPCCTRL0 = temp;
	WriteIO32(&pRegister->DPCCTRL0, temp);

//	pRegister->DPCDELAY0 = (U32)((DelayVS_FRAM<<DELAYVS_POS) | (DelayHS_CP1<<DELAYHS_POS));
	WriteIO32(&pRegister->DPCDELAY0, (U32)((DelayVS_FRAM<<DELAYVS_POS) | (DelayHS_CP1<<DELAYHS_POS)));

//	pRegister->DPCDELAY1 = (U32)(DelayDE_CP2<<DELAYDE_POS);
	WriteIO32(&pRegister->DPCDELAY1, (U32)(DelayDE_CP2<<DELAYDE_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current delay values for data and sync signals.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[out] pDelayRGB_PVD		Get current delay value for RGB/PVD signal.
 *	@param[out] pDelayHS_CP1		Get current delay value for HSYNC/CP1 signal.
 *	@param[out] pDelayVS_FRAM		Get current delay value for VSYNC/FRAM signal.
 *	@param[out] pDelayDE_CP2		Get current delay value for DE/CP2 signal.
 *	@return		None.
 *	@remarks	Arguments which does not required can be CNULL.\n
 *				\b TFT \b LCD
 *					- Signal is RGB, HSYNC, VSYNC, DE.
 *					- The unit is VCLK2.
 *
 *	@see		NX_DPC_SetDelay
 */
void	NX_DPC_GetDelay( U32 ModuleIndex, U32 *pDelayRGB_PVD, U32 *pDelayHS_CP1, U32 *pDelayVS_FRAM, U32 *pDelayDE_CP2 )
{
	const U32 DELAYRGB_POS	= 4;
	const U32 DELAYRGB_MASK	= 0xFU<<DELAYRGB_POS;

	const U32 DELAYDE_POS	= 0;
	const U32 DELAYDE_MASK	= 0x3FU<<DELAYDE_POS;
	const U32 DELAYVS_POS	= 8;
	const U32 DELAYVS_MASK	= 0x3FU<<DELAYVS_POS;
	const U32 DELAYHS_POS	= 0;
	const U32 DELAYHS_MASK	= 0x3FU<<DELAYHS_POS;

	register U32 temp;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0;
	if( CNULL != pDelayRGB_PVD )	*pDelayRGB_PVD	= (U32)((temp & DELAYRGB_MASK)>>DELAYRGB_POS);

	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCDELAY0;
	if( CNULL != pDelayHS_CP1	)	*pDelayHS_CP1	= (U32)((temp & DELAYHS_MASK )>>DELAYHS_POS );
	if( CNULL != pDelayVS_FRAM	)	*pDelayVS_FRAM	= (U32)((temp & DELAYVS_MASK )>>DELAYVS_POS );
	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCDELAY1;
	if( CNULL != pDelayDE_CP2	)	*pDelayDE_CP2	= (U32)((temp & DELAYDE_MASK )>>DELAYDE_POS );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set RGB dithering mode.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	DitherR		Specifies the dithering mode for Red component.
 *	@param[in]	DitherG		Specifies the dithering mode for Green component.
 *	@param[in]	DitherB		Specifies the dithering mode for Blue component.
 *	@return		None.
 *	@remark		The dithering is useful method for case which is that the color
 *				depth of destination is less than one of source.
 *	@see		NX_DPC_GetDither
 */
void	NX_DPC_SetDither( U32 ModuleIndex, NX_DPC_DITHER DitherR, NX_DPC_DITHER DitherG, NX_DPC_DITHER DitherB )
{
	const U32 DITHER_MASK	= 0x3FU;
	const U32 RDITHER_POS	= 0;
	const U32 GDITHER_POS	= 2;
	const U32 BDITHER_POS	= 4;
	register U32 temp;

	register struct NX_DPC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 4 > DitherR );
	NX_ASSERT( 4 > DitherG );
	NX_ASSERT( 4 > DitherB );

	pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

	temp = pRegister->DPCCTRL1;
	temp &= (U32)~DITHER_MASK;	// unmask dithering mode.
	temp = (U32)(temp | ((DitherB<<BDITHER_POS) | (DitherG<<GDITHER_POS) | (DitherR<<RDITHER_POS)));

//	pRegister->DPCCTRL1 = temp;
	WriteIO32(&pRegister->DPCCTRL1, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current RGB dithering mode.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC, 1 : Second DPC ).
 *	@param[out] pDitherR		Get current dithering mode for Red component.
 *	@param[out] pDitherG		Get current dithering mode for Green component.
 *	@param[out] pDitherB		Get current dithering mode for Blue component.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_DPC_SetDither
 */
void	NX_DPC_GetDither( U32 ModuleIndex, NX_DPC_DITHER *pDitherR, NX_DPC_DITHER *pDitherG, NX_DPC_DITHER *pDitherB )
{
	const U32 RDITHER_POS	= 0;
	const U32 RDITHER_MASK	= 0x3U<<RDITHER_POS;
	const U32 GDITHER_POS	= 2;
	const U32 GDITHER_MASK	= 0x3U<<GDITHER_POS;
	const U32 BDITHER_POS	= 4;
	const U32 BDITHER_MASK	= 0x3U<<BDITHER_POS;
	register U32 temp;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL1;
	if( CNULL != pDitherR )		*pDitherR	= (NX_DPC_DITHER)((temp & RDITHER_MASK)>>RDITHER_POS);
	if( CNULL != pDitherG )		*pDitherG	= (NX_DPC_DITHER)((temp & GDITHER_MASK)>>GDITHER_POS);
	if( CNULL != pDitherB )		*pDitherB	= (NX_DPC_DITHER)((temp & BDITHER_MASK)>>BDITHER_POS);
}

//--------------------------------------------------------------------------
// TFT LCD specific control function
//--------------------------------------------------------------------------
/**
 *	@brief		Set display mode.
 *	@param[in] ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in] format			Specifies data format.

 *	@param[in] bInterlace		Specifies scan mode.\n
 *									CTRUE = Interface, CFALSE = Progressive.
 *	@param[in] bInvertField		Specifies internal field polarity.\n
 *									CTRUE = Invert Field(Low is even field), CFALSE = Normal Field(Low is odd field).

 *	@param[in] bRGBMode			Specifies pixel format.\n
 *									CTRUE = RGB Mode, CFASE = YCbCr Mode.
 *	@param[in] bSwapRB			Swap Red and Blue component for RGB output.\n
 *									CTRUE = Swap Red and Blue, CFALSE = No swap.

 *	@param[in] ycorder			Specifies output order for YCbCr Output.
 *	@param[in] bClipYC			Specifies output range of RGB2YC.\n
 *									CTRUE = Y(16 ~ 235), Cb/Cr(16 ~ 240), CFALSE = Y/Cb/Cr(0 ~ 255).\n
 *									You have to set to CTRUE for ITU-R BT.656 and internal video encoder.
 *	@param[in] bEmbeddedSync	Specifies embedded sync mode(SAV/EAV).\n
 *								CTRUE = Enable, CFALSE = Disable.\n
 *									You have to set to CTRUE for ITU-R BT.656.

 *	@param[in] clock			Specifies the PAD output clock.
 *	@param[in] bInvertClock		Sepcifies the pixel clock polarity.\n
 *									CTRUE = rising edge, CFALSE = falling edge.
 *
 *	@param[in] bDualView		Specifies the daul view LCD type.\n
 *								CTRUE = dual view LCD, CFALSE = none.
 *	@return		None.
 *	@see		NX_DPC_GetMode
 */
void	NX_DPC_SetMode( U32 ModuleIndex, NX_DPC_FORMAT format,
							CBOOL bInterlace, CBOOL bInvertField,
							CBOOL bRGBMode, CBOOL bSwapRB,
							NX_DPC_YCORDER ycorder, CBOOL bClipYC, CBOOL bEmbeddedSync,
							NX_DPC_PADCLK clock, CBOOL bInvertClock, CBOOL bDualView )
{
	// DPC Control 0 register
	const U32 POLFIELD_POS	= 2;
	const U32 SEAVENB_POS	= 8;
	const U32 SCANMODE_POS	= 9;
	const U32 INTPEND_POS	= 10;
	const U32 RGBMODE_POS	= 12;

	// DPC Control 1 register
	const U32 DITHER_MASK	= 0x3F;
	const U32 YCORDER_POS	= 6;
	const U32 FORMAT_POS	= 8;
	const U32 YCRANGE_POS	= 13;
	const U32 SWAPRB_POS	= 15;

	// DPC Control 2 register
	const U32 PADCLKSEL_POS		= 0;
	const U32 PADCLKSEL_MASK	= 3U<<PADCLKSEL_POS;
	const U32 LCDTYPE_POS		= 7;
	const U32 LCDTYPE_MASK		= 3U<<LCDTYPE_POS;

	register struct NX_DPC_RegisterSet*	pRegister;

	register U32 temp;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( bDualView == 0 );
	NX_ASSERT( 19 > format );
	NX_ASSERT( 4 > ycorder );
	NX_ASSERT( 4 > clock );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	temp  = pRegister->DPCCTRL0;
	temp &= (U32)~(1U<<INTPEND_POS);	// unmask intpend bit.

	if( bInterlace )	temp |= (U32) (1U<<SCANMODE_POS);
	else				temp &= (U32)~(1U<<SCANMODE_POS);
	if( bInvertField )	temp |= (U32) (1U<<POLFIELD_POS);
	else				temp &= (U32)~(1U<<POLFIELD_POS);
	if( bRGBMode )		temp |= (U32) (1U<<RGBMODE_POS);
	else				temp &= (U32)~(1U<<RGBMODE_POS);
	if( bEmbeddedSync )	temp |= (U32) (1U<<SEAVENB_POS);
	else				temp &= (U32)~(1U<<SEAVENB_POS);

//	pRegister->DPCCTRL0 = temp;
	WriteIO32(&pRegister->DPCCTRL0, temp);

	temp  = pRegister->DPCCTRL1;
	temp &= (U32)DITHER_MASK;		// mask other bits.
	temp  = (U32)(temp | (ycorder << YCORDER_POS));
	if (format>=16)
	{
		register U32 temp1;
		temp1= pRegister->DPCCTRL2;
		temp1= temp1 | (1<<4);
		WriteIO32(&pRegister->DPCCTRL2, temp1);
	}
	else
	{
		register U32 temp1;
		temp1= pRegister->DPCCTRL2;
		temp1= temp1 & ~(1<<4);
		WriteIO32(&pRegister->DPCCTRL2, temp1);
	}

	temp  = (U32)(temp | ((format&0xf) << FORMAT_POS));

	if( !bClipYC )	temp |= (U32)(1U<<YCRANGE_POS);
	if( bSwapRB )	temp |= (U32)(1U<<SWAPRB_POS);

//	pRegister->DPCCTRL1 = temp;
	WriteIO32(&pRegister->DPCCTRL1, temp);

	temp  = pRegister->DPCCTRL2;
	temp &= (U32)~(PADCLKSEL_MASK | LCDTYPE_MASK );		// TFT or Video Encoder
	temp  = (U32)(temp | (clock<<PADCLKSEL_POS));

//	pRegister->DPCCTRL2 = temp;
	WriteIO32(&pRegister->DPCCTRL2, temp);

	// Determines whether invert or not the polarity of the pad clock.
	NX_DPC_SetClockOutInv( ModuleIndex, 0, bInvertClock );
	NX_DPC_SetClockOutInv( ModuleIndex, 1, bInvertClock );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current display mode.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[out] pFormat			Get current data format.

 *	@param[out] pbInterlace		Get current scan mode.\n
 *								CTRUE = Interface, CFALSE = Progressive.
 *	@param[out] pbInvertField	Specifies internal field polarity.\n
 *								CTRUE = Invert Field(Low is even field), CFALSE = Normal Field(Low is odd field).

 *	@param[out] pbRGBMode		Get current pixel format.\n
 *								CTRUE = RGB Mode, CFASE = YCbCr Mode.
 *	@param[out] pbSwapRB		Get current setting about Swap Red and Blue component for RGB output.\n
 *								CTRUE = Swap Red and Blue, CFALSE = No swap.

 *	@param[out] pYCorder		Get current output order for YCbCr Output.
 *	@param[out] pbClipYC		Get current output range of RGB2YC.\n
 *								CTRUE = Y(16 ~ 235), Cb/Cr(16 ~ 240), CFALSE = Y/Cb/Cr(0 ~ 255).
 *	@param[out] pbEmbeddedSync	Get current embedded sync mode(SAV/EAV).\n
 *								CTRUE = Enable, CFALSE = Disable.

 *	@param[out] pClock			Get current PAD output clock.
 *	@param[out] pbInvertClock	Get current pixel clock polarity.\n
 *								CTRUE = rising edge, CFALSE = falling edge.

 *	@param[out] pbDualView		Specifies the daul view LCD type.\n
 *								CTRUE = dual view LCD, CFALSE = none.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_DPC_SetMode
 */
void	NX_DPC_GetMode( U32 ModuleIndex, NX_DPC_FORMAT *pFormat,
							CBOOL *pbInterlace, CBOOL *pbInvertField,
							CBOOL *pbRGBMode, CBOOL *pbSwapRB,
							NX_DPC_YCORDER *pYCorder, CBOOL *pbClipYC, CBOOL *pbEmbeddedSync,
							NX_DPC_PADCLK *pClock, CBOOL *pbInvertClock, CBOOL *pbDualView )
{
	// DPC Control 0 register
	const U32 POLFIELD	= 1U<<2;
	const U32 SEAVENB	= 1U<<8;
	const U32 SCANMODE	= 1U<<9;
	const U32 RGBMODE	= 1U<<12;

	// DPC Control 1 register
	const U32 YCORDER_POS	= 6;
	const U32 YCORDER_MASK	= 0x3U<<YCORDER_POS;
	const U32 FORMAT_POS	= 8;
	const U32 FORMAT_MASK	= 0xFU<<FORMAT_POS;
	const U32 YCRANGE		= 1U<<13;
	const U32 SWAPRB		= 1U<<15;

	// DPC Control 2 register
	const U32 PADCLKSEL_POS		= 0;
	const U32 PADCLKSEL_MASK	= 3U<<PADCLKSEL_POS;
	const U32 LCDTYPE_POS		= 7;
	const U32 LCDTYPE_MASK		= 3U<<LCDTYPE_POS;

	register U32 temp;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL0;
	if( CNULL != pbInterlace )		*pbInterlace	= (temp & SCANMODE) ? CTRUE : CFALSE;
	if( CNULL != pbInvertField )	*pbInvertField	= (temp & POLFIELD) ? CTRUE : CFALSE;
	if( CNULL != pbRGBMode )		*pbRGBMode		= (temp & RGBMODE ) ? CTRUE : CFALSE;
	if( CNULL != pbEmbeddedSync )	*pbEmbeddedSync	= (temp & SEAVENB ) ? CTRUE : CFALSE;

	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL1;
	if( CNULL != pYCorder )			*pYCorder	= (NX_DPC_YCORDER) ((temp & YCORDER_MASK)>>YCORDER_POS);
	if( CNULL != pFormat )			*pFormat	= (NX_DPC_FORMAT)((temp & FORMAT_MASK	)>>FORMAT_POS	);
	if( CNULL != pbClipYC )			*pbClipYC	= (temp & YCRANGE) ? CFALSE : CTRUE;
	if( CNULL != pbSwapRB )			*pbSwapRB	= (temp & SWAPRB) ? CTRUE : CFALSE;

	temp = __g_ModuleVariables[ModuleIndex].pRegister->DPCCTRL2;
	if( CNULL != pClock )			*pClock		= (NX_DPC_PADCLK)((temp & PADCLKSEL_MASK)>>PADCLKSEL_POS);
	if( CNULL != pbDualView)		*pbDualView = (2 == ((temp&LCDTYPE_MASK)>>LCDTYPE_POS)) ? CTRUE : CFALSE;

	// Determines whether invert or not the polarity of the pad clock.
	if( CNULL != pbInvertClock )	*pbInvertClock = NX_DPC_GetClockOutInv( ModuleIndex, 1 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set parameters for horizontal sync.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	AVWidth		Specifies the active video width in clocks.
 *	@param[in]	HSW			Specifies the horizontal sync width in clocks
 *	@param[in]	HFP			Specifies the horizontal sync front porch in clocks.
 *	@param[in]	HBP			Specifies the horizontal sync back porch in clocks.
 *	@param[in]	bInvHSYNC Specifies HSYNC polarity. CTRUE = High active, CFALSE = Low active.
 *	@return		None.
 *	@remark		A sum of arguments except bInvHSYNC has to be less than or equal to 65536.
 *				The unit is VCLK( one clock for a pixel).\n
 *				See follwing figure for more details.
 *	@code
 *
 *						<---------------TOTAL------------------->
 *						<--SW--->
 *		Sync	--------+		+--------------/-/--------------+		+---
 *						|		|								|		|
 *						+-------+								+-------+
 *				<--FP->		<--BP--><----ACTIVE VIDEO--->
 *		Active -+					+-------/-/---------+
 *		Video	|		(BLANK)		|	(ACTIVE DATA)	|		(BLANK)
 *				+-------------------+					+-----------------
 *						<---ASTART-->
 *						<-------------AEND------------->
 *	@endcode
 *	@see		NX_DPC_GetHSync
 */
void	NX_DPC_SetHSync( U32 ModuleIndex, U32 AVWidth, U32 HSW, U32 HFP, U32 HBP, CBOOL bInvHSYNC )
{
	const U32 INTPEND	= 1U<<10;
	const U32 POLHSYNC	= 1U<<0;
	register U32 temp;

	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 65536 >= (AVWidth + HFP + HSW + HBP) );
	NX_ASSERT( 0 < HSW );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->DPCHTOTAL	= (U32)(HSW + HBP + AVWidth + HFP - 1);
	WriteIO32(&pRegister->DPCHTOTAL, (U32)(HSW + HBP + AVWidth + HFP - 1));

//	pRegister->DPCHSWIDTH	= (U32)(HSW - 1);
	WriteIO32(&pRegister->DPCHSWIDTH, (U32)(HSW - 1));

//	pRegister->DPCHASTART	= (U32)(HSW + HBP - 1);
	WriteIO32(&pRegister->DPCHASTART, (U32)(HSW + HBP - 1));

//	pRegister->DPCHAEND	= (U32)(HSW + HBP + AVWidth - 1);
	WriteIO32(&pRegister->DPCHAEND, (U32)(HSW + HBP + AVWidth - 1));

	temp  = pRegister->DPCCTRL0;
	temp &= ~INTPEND;	// unmask intpend bit.

	if( bInvHSYNC )		temp |= (U32) POLHSYNC;
	else				temp &= (U32)~POLHSYNC;

//	pRegister->DPCCTRL0 = temp;
	WriteIO32(&pRegister->DPCCTRL0, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current parameters for horizontal sync.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[out] pAVWidth	Get current active video width in clocks.
 *	@param[out] pHSW		Get current horizontal sync width in clocks
 *	@param[out] pHFP		Get current horizontal sync front porch in clocks.
 *	@param[out] pHBP		Get current horizontal sync back porch in clocks.
 *	@param[out] pbInvHSYNC	Get current HSYNC polarity. CTRUE = High active, CFALSE = Low active.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.\n
 *				The unit is VCLK( one clock for a pixel).
 *	@see		NX_DPC_SetHSync
 */
void	NX_DPC_GetHSync( U32 ModuleIndex, U32 *pAVWidth, U32 *pHSW, U32 *pHFP, U32 *pHBP, CBOOL *pbInvHSYNC )
{
	const U32 POLHSYNC	= 1U<<0;

	U32	htotal, hsw, hab, hae;
	U32 avw, hfp, hbp;

	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	htotal	= (U32)pRegister->DPCHTOTAL + 1;
	hsw		= (U32)pRegister->DPCHSWIDTH + 1;
	hab		= (U32)pRegister->DPCHASTART + 1;
	hae		= (U32)pRegister->DPCHAEND + 1;

	hbp		= hab - hsw;
	avw		= hae - hab;
	hfp		= htotal - hae;

	if( CNULL != pAVWidth )		*pAVWidth	= avw;
	if( CNULL != pHSW )			*pHSW		= hsw;
	if( CNULL != pHFP )			*pHFP		= hfp;
	if( CNULL != pHBP )			*pHBP		= hbp;

	if( CNULL != pbInvHSYNC )	*pbInvHSYNC	= (pRegister->DPCCTRL0 & POLHSYNC) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set parameters for vertical sync.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	AVHeight		Specifies the active video height in lines.
 *	@param[in]	VSW			Specifies the vertical sync width in lines.
 *								When interlace mode, this value is used for odd field.
 *	@param[in]	VFP			Specifies the vertical sync front porch in lines.
 *								When interlace mode, this value is used for odd field.
 *	@param[in]	VBP			Specifies the vertical sync back porch in lines.
 *								When interlace mode, this value is used for odd field.
 *	@param[in]	bInvVSYNC		Specifies VSYNC polarity. CTRUE = High active, CFALSE = Low active.
 *	@param[in]	EAVHeight		Specifies the active video height in lines for even field.
 *	@param[in]	EVSW			Specifies the vertical sync width in lines for even field.
 *	@param[in]	EVFP			Specifies the vertical sync front porch in lines for even field.
 *	@param[in]	EVBP			Specifies the vertical sync back porch in lines for even field.
 *	@return		None.
 *	@remark		A sum of arguments(AVHeight + VSW + VFP + VBP or AVHeight + EVSW + EVFP + EVBP)
 *				has to be less than or equal to 65536.\n
 *				See follwing figure for more details.
 *	@code
 *
 *						<----------------TOTAL------------------>
 *						<---SW-->
 *		Sync	--------+		+--------------/-/--------------+		+---
 *						|		|								|		|
 *						+-------+								+-------+
 *				<-FP->	<--BP--> <---ACTIVE VIDEO--->
 *		Active -+					+--------/-/--------+
 *		Video	|		(BLANK)		|	(ACTIVE DATA)	|		(BLANK)
 *				+-------------------+					+-----------------
 *						<---ASTART-->
 *						<-------------AEND------------->
 *	@endcode
 *	@see		NX_DPC_GetVSync
 */
void	NX_DPC_SetVSync( U32 ModuleIndex, U32 AVHeight, U32 VSW, U32 VFP, U32 VBP, CBOOL bInvVSYNC,
							U32 EAVHeight, U32 EVSW, U32 EVFP, U32 EVBP )
{
	const U32 INTPEND	= 1U<<10;
	const U32 POLVSYNC	= 1U<<1;

	register U32 temp;

	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 65536 >= (AVHeight + VFP + VSW + VBP) );
	NX_ASSERT( 65536 >= (AVHeight + EVFP + EVSW + EVBP) );
	NX_ASSERT( 0 < VSW );
	NX_ASSERT( 0 < EVSW );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->DPCVTOTAL	= (U32)(VSW + VBP + AVHeight + VFP - 1);
	WriteIO32(&pRegister->DPCVTOTAL, (U32)(VSW + VBP + AVHeight + VFP - 1));

//	pRegister->DPCVSWIDTH	= (U32)(VSW - 1);
	WriteIO32(&pRegister->DPCVSWIDTH, (U32)(VSW - 1));

//	pRegister->DPCVASTART	= (U32)(VSW + VBP - 1);
	WriteIO32(&pRegister->DPCVASTART, (U32)(VSW + VBP - 1));

//	pRegister->DPCVAEND		= (U32)(VSW + VBP + AVHeight - 1);
	WriteIO32(&pRegister->DPCVAEND, (U32)(VSW + VBP + AVHeight - 1));

//	pRegister->DPCEVTOTAL	= (U32)(EVSW + EVBP + EAVHeight + EVFP - 1);
	WriteIO32(&pRegister->DPCEVTOTAL, (U32)(EVSW + EVBP + EAVHeight + EVFP - 1));

//	pRegister->DPCEVSWIDTH	= (U32)(EVSW - 1);
	WriteIO32(&pRegister->DPCEVSWIDTH, (U32)(EVSW - 1));

//	pRegister->DPCEVASTART	= (U32)(EVSW + EVBP - 1);
	WriteIO32(&pRegister->DPCEVASTART, (U32)(EVSW + EVBP - 1));

//	pRegister->DPCEVAEND	= (U32)(EVSW + EVBP + EAVHeight - 1);
	WriteIO32(&pRegister->DPCEVAEND, (U32)(EVSW + EVBP + EAVHeight - 1));

	temp  = pRegister->DPCCTRL0;
	temp &= ~INTPEND;	// unmask intpend bit.

	if( bInvVSYNC )		temp |= (U32) POLVSYNC;
	else				temp &= (U32)~POLVSYNC;

//	pRegister->DPCCTRL0 = temp;
	WriteIO32(&pRegister->DPCCTRL0, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current parameters for vertical sync.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC, 1 : Second DPC ).
 *	@param[out] pAVHeight		Get current active video height in lines.
 *	@param[out] pVSW			Get current vertical sync width in lines.
 *								When interlace mode, this value is used for odd field.
 *	@param[out] pVFP			Get current vertical sync front porch in lines.
 *								When interlace mode, this value is used for odd field.
 *	@param[out] pVBP			Get current vertical sync back porch in lines.
 *								When interlace mode, this value is used for odd field.
 *	@param[out] pbInvVSYNC	Get current VSYNC polarity. CTRUE = High active, CFALSE = Low active.
 *	@param[out] pEAVHeight	Get current active video height in lines for even field.
 *	@param[out] pEVSW			Get current vertical sync width in lines for even field.
 *	@param[out] pEVFP			Get current vertical sync front porch in lines for even field.
 *	@param[out] pEVBP			Get current vertical sync back porch in lines for even field.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_DPC_SetVSync
 */
void	NX_DPC_GetVSync( U32 ModuleIndex, U32 *pAVHeight, U32 *pVSW, U32 *pVFP, U32 *pVBP, CBOOL *pbInvVSYNC,
							U32 *pEAVHeight, U32 *pEVSW, U32 *pEVFP, U32 *pEVBP )
{
	const U32 POLVSYNC	= 1U<<1;

	U32 vtotal, vsw, vab, vae;
	U32 avh, vfp, vbp;

	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	vtotal	= (U32)pRegister->DPCVTOTAL		+ 1;
	vsw		= (U32)pRegister->DPCVSWIDTH	+ 1;
	vab		= (U32)pRegister->DPCVASTART	+ 1;
	vae		= (U32)pRegister->DPCVAEND		+ 1;

	vbp		= vab - vsw;
	avh		= vae - vab;
	vfp		= vtotal - vae;

	if( CNULL != pAVHeight )	*pAVHeight	= avh;
	if( CNULL != pVSW )			*pVSW		= vsw;
	if( CNULL != pVFP )			*pVFP		= vfp;
	if( CNULL != pVBP )			*pVBP		= vbp;

	vtotal	= (U32)pRegister->DPCEVTOTAL	+ 1;
	vsw		= (U32)pRegister->DPCEVSWIDTH	+ 1;
	vab		= (U32)pRegister->DPCEVASTART	+ 1;
	vae		= (U32)pRegister->DPCEVAEND		+ 1;

	vbp		= vab - vsw;
	avh		= vae - vab;
	vfp		= vtotal - vae;

	if( CNULL != pEAVHeight )	*pEAVHeight	= avh;
	if( CNULL != pEVSW )		*pEVSW		= vsw;
	if( CNULL != pEVFP )		*pEVFP		= vfp;
	if( CNULL != pEVBP )		*pEVBP		= vbp;

	if( CNULL != pbInvVSYNC )	*pbInvVSYNC	= (pRegister->DPCCTRL0 & POLVSYNC) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set offsets for vertical sync.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	VSSOffset Specifies the number of clocks from the start of horizontal sync to the start of vertical sync,
 *								where horizontal sync is the last one in vertical front porch.
 *								If this value is 0 then the start of vertical sync synchronizes with the start of horizontal sync
 *								which is the new one in vertical sync.
 *								This value has to be less than HTOTAL. When interlace mode, this vaule is used for odd field.
 *	@param[in]	VSEOffset Specifies the number of clocks from the start of horizontal sync to the end of vertical sync,
 *								where horizontal sync is the last one in vertical sync.
 *								If this value is 0 then the end of vertical sync synchronizes with the start of horizontal sync
 *								which is the new one in vertical back porch.
 *								This value has to be less than HTOTAL. When interlace mode, this vaule is used for odd field.
 *	@param[in]	EVSSOffset Specifies the number of clocks from the start of horizontal sync to the start of vertical sync,
 *								where horizontal sync is the last one in vertical front porch.
 *								If this value is 0 then the start of vertical sync synchronizes with the start of horizontal sync
 *								which is the new one in vertical sync.
 *								This value has to be less than HTOTAL and is used for even field.
 *	@param[in]	EVSEOffset Specifies the number of clocks from the start of horizontal sync to the end of vertical sync,
 *								where horizontal sync is the last one in vertical sync.
 *								If this value is 0 then the end of vertical sync synchronizes with the start of horizontal sync
 *								which is the new one in vertical back porch.
 *								This value has to be less than HTOTAL and is used for even field.
 *	@return		None.
 *	@remark		All arguments has to be less than HTOTAL or 65536.
 *				The unit is VCLK( one clock for a pixel).\n
 *				See follwing figure for more details.
 *	@code
 *				<---HTOTAL--->
 *		HSYNC	----+ +---------+ +---/-/---+ +---------+ +-------------
 *					| |			| |			| |			| |
 *					+-+			+-+			+-+			+-+
 *
 *		If VSSOffset == 0 and VSEOffset == 0 then
 *
 *		VSYNC	----------------+								+---------------
 *					(VFP)		|			(VSW)				|		(VBP)
 *								+-------------/-/---------------+
 *
 *		,else
 *
 *		VSYNC	--------+								+---------------------
 *						|<---> tVSSO					|<---> tVSEO
 *						+--------------/-/--------------+
 *				<------>						<------>
 *				VSSOffset						VSEOffset
 *				= HTOTAL - tVSSO				= HTOTAL - tVSEO
 *	@endcode
 *	@see		NX_DPC_GetVSyncOffset
 */
void	NX_DPC_SetVSyncOffset( U32 ModuleIndex, U32 VSSOffset, U32 VSEOffset, U32 EVSSOffset, U32 EVSEOffset )
{
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 65536 > VSSOffset );
	NX_ASSERT( 65536 > VSEOffset );
	NX_ASSERT( 65536 > EVSSOffset );
	NX_ASSERT( 65536 > EVSEOffset );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->DPCVSEOFFSET	= (U32)VSEOffset;
	WriteIO32(&pRegister->DPCVSEOFFSET, (U32)VSEOffset);

//	pRegister->DPCVSSOFFSET	= (U32)VSSOffset;
	WriteIO32(&pRegister->DPCVSSOFFSET, (U32)VSSOffset);

//	pRegister->DPCEVSEOFFSET	= (U32)EVSEOffset;
	WriteIO32(&pRegister->DPCEVSEOFFSET, (U32)EVSEOffset);

//	pRegister->DPCEVSSOFFSET	= (U32)EVSSOffset;
	WriteIO32(&pRegister->DPCEVSSOFFSET, (U32)EVSSOffset);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current offsets for vertical sync.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[out] pVSSOffset			Get current number of clocks from the start of horizontal sync to the start of vertical sync,
 *									where horizontal sync is the last one in vertical front porch.
 *									If this value is 0 then the start of vertical sync synchronizes with the start of horizontal sync
 *									which is the new one in vertical sync. When interlace mode, this vaule is used for odd field.
 *	@param[out] pVSEOffset			Get current number of clocks from the start of horizontal sync to the end of vertical sync,
 *									where horizontal sync is the last one in vertical sync.
 *									If this value is 0 then the end of vertical sync synchronizes with the start of horizontal sync
 *									which is the new one in vertical back porch. When interlace mode, this vaule is used for odd field.
 *	@param[out] pEVSSOffset		Get current number of clocks from the start of horizontal sync to the start of vertical sync,
 *									where horizontal sync is the last one in vertical front porch.
 *									If this value is 0 then the start of vertical sync synchronizes with the start of horizontal sync
 *									which is the new one in vertical sync. This value is used for even field.
 *	@param[out] pEVSEOffset			Get current number of clocks from the start of horizontal sync to the end of vertical sync,
 *									where horizontal sync is the last one in vertical sync.
 *									If this value is 0 then the end of vertical sync synchronizes with the start of horizontal sync
 *									which is the new one in vertical back porch. This value is used for even field.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *				The unit is VCLK( one clock for a pixel).
 *	@see		NX_DPC_SetVSyncOffset
 */
void	NX_DPC_GetVSyncOffset( U32 ModuleIndex, U32 *pVSSOffset, U32 *pVSEOffset, U32 *pEVSSOffset, U32 *pEVSEOffset )
{
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( CNULL != *pVSEOffset )	*pVSEOffset	= (U32)pRegister->DPCVSEOFFSET;
	if( CNULL != *pVSSOffset )	*pVSSOffset	= (U32)pRegister->DPCVSSOFFSET;
	if( CNULL != *pEVSEOffset)	*pEVSEOffset = (U32)pRegister->DPCEVSEOFFSET;
	if( CNULL != *pEVSSOffset)	*pEVSSOffset = (U32)pRegister->DPCEVSSOFFSET;
}



//------------------------------------------------------------------------------
/**
 *	@brief		Set horizontal scale up ratio.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	bEnb	\b CTRUE	indicates that scaler enable.\n
 *						\b CFALSE	indicates that scaler disable.\n
 *	@param[in]	sourceWidth Source image width that valuse have to same of MLC source width.
 *	@param[in]	destWidth	Destination image width.
 *	@return		None.
 *	@remarks		Only seconary DPC can scale up of horizontal width.\n
 *				Scale ration calculation is below
 *					- UpScale = (source width - 1 ) * (1<<11) / (destination width - 1)
 *	@see		NX_DPC_GetHorizontalUpScaler
 */
void	NX_DPC_SetHorizontalUpScaler( U32 ModuleIndex, CBOOL bEnb, U32 sourceWidth, U32 destWidth )
{
	const U32 UPSCALEL_POS		= 8;

	const U32 UPSCALEH_POS		= 0;
	const U32 UPSCALEH_MASK		= ((1<<15)-1) << UPSCALEH_POS;

	const U32 UPSCALERENB_POS	= 0;

	register struct NX_DPC_RegisterSet* pRegister;
	register U32 regvalue;
	register U32 UpScale;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnb) ||(1==bEnb) );
	NX_ASSERT( 1 <= sourceWidth && sourceWidth <= 65536 );
	NX_ASSERT( 1 <= destWidth && destWidth <= 65536 );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	UpScale = ((sourceWidth-1) * (1<<11)) / (destWidth-1) ;

	regvalue = 0;
	regvalue |= (((U32)bEnb<<UPSCALERENB_POS) | (UpScale & 0xFF)<<UPSCALEL_POS );

//	pRegister->DPCUPSCALECON0 = regvalue;
	WriteIO32(&pRegister->DPCUPSCALECON0, regvalue);

//	pRegister->DPCUPSCALECON1 = ( UpScale >> 0x08 ) & UPSCALEH_MASK;
	WriteIO32(&pRegister->DPCUPSCALECON1, ( UpScale >> 0x08 ) & UPSCALEH_MASK);

//	pRegister->DPCUPSCALECON2 = sourceWidth - 1;
	WriteIO32(&pRegister->DPCUPSCALECON2, sourceWidth - 1);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get horizontal scale up ratio.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[out]	pbEnb	\b CTRUE	indicates that scaler is enabled.\n
 *						\b CFALSE	indicates that scaler is disabled.\n
 *	@param[out]	psourceWidth Source image width.
 *	@param[out]	pdestWidth	Destination image width.
 *	@return		None.
 *	@remarks		Only seconary DPC can scale up of horizontal width.\n
 *				Scale ration calculation is below
 *					- UpScale = (source width - 1 ) * (1<<11) / (destination width - 1)
 *	@see		NX_DPC_SetHorizontalUpScaler
 */
void	NX_DPC_GetHorizontalUpScaler( U32 ModuleIndex, CBOOL* pbEnb, U32* psourceWidth, U32* pdestWidth )
{
	const U32 UPSCALERENB_POS	= 0;
	const U32 UPSCALERENB_MASK	= 1U << UPSCALERENB_POS;

	register struct NX_DPC_RegisterSet* pRegister;
	U32 UpScale;
	U32 destwidth, srcwidth;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	UpScale = ((U32)(pRegister->DPCUPSCALECON1 & 0x7FFF)<<8) | ((U32)(pRegister->DPCUPSCALECON0>>8) & 0xFF );
	srcwidth	=	pRegister->DPCUPSCALECON2;
	destwidth	=	(srcwidth * (1<<11)) / UpScale;

	if( CNULL != pbEnb )		*pbEnb			= (pRegister->DPCUPSCALECON0 & UPSCALERENB_MASK );
	if( CNULL != psourceWidth ) *psourceWidth	= srcwidth + 1;
	if( CNULL != pdestWidth )	*pdestWidth		= destwidth + 1;
}












//----------------------------------------------------------------
// Prototype 이식 : ( @todo 한꺼번에 header로 옮길것 ( 2012-10-05 오후 4:37:05 )
//----------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// @brief	set parameters of sync generator
//------------------------------------------------------------------------------
void NX_DPC_SetSync
(
    U32 ModuleIndex,
    SYNCGENMODE SyncGenMode,    ///< [in] interlace and progressive mode select
	U32 AVWidth,			    ///< [in] Width of Active video (unit:Clock)
	U32 AVHeight,			    ///< [in] Height of Active video(unit:Line)
	U32 HSW,				    ///< [in] Horizontal sync width (unit:Clock)
	U32 HFP,				    ///< [in] Horizontal front porch width (unit:Clock)
	U32 HBP,				    ///< [in] Horizontal back porch width (unit:Clock)
    U32 VSW,                    ///< [in] Vertical sync width (unit:Line)
    U32 VFP,                    ///< [in] Vertical front porch width (unit:Line)
    U32 VBP,                    ///< [in] Vertical back porch width (unit:Line)
	POLARITY FieldPolarity,	    ///< [in] Field polarity
	POLARITY HSyncPolarity,	    ///< [in] Horizontal sync polarity
	POLARITY VSyncPolarity,	    ///< [in] Vertical sync polarity
	U32 EvenVSW,                ///< [in] Vertical sync width (unit:Line)
	U32 EvenVFP,                ///< [in] Vertical front porch width (unit:Line)
	U32 EvenVBP,                ///< [in] Vertical back porch width (unit:Line)
    U32 VSETPIXEL,              ///< [in] Vertical sync Set number of pixel (unit:Clock)
    U32 VSCLRPIXEL,             ///< [in] Vertical sync clear number of pixel (unit:Clock)
    U32 EVENVSETPIXEL,          ///< [in] Vertical sync Set number of pixel (unit:Clock)
    U32 EVENVSCLRPIXEL          ///< [in] Vertical sync clear number of pixel (unit:Clock)
)
{

	U32 regvalue  = 0;
	U32 regvalue1 = 0;

	register struct NX_DPC_RegisterSet* pRegister;
	//@modified choiyk 2013-01-03 오후 2:04:04
	// HDMI eco지원을 위해서..
	//NX_ASSERT( 0 < AVWidth );
	//NX_ASSERT( 0 < AVHeight );
	//NX_ASSERT( 0 < HSW );
	//NX_ASSERT( 0 < HFP );
	//NX_ASSERT( 0 < HBP );
	//NX_ASSERT( 0 < VSW );
	//NX_ASSERT( 0 < VFP );
	//NX_ASSERT( 0 < VBP );
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;


	//                 |<---------------TOT----------------->|
 	//                 |<-SW->|<------------SBEGIN---------->|      |
	// Sync   ---------+      +------------------------------+      +-
	//                 +------+                              +------+
	//          |<-FP->|      |<-BP->|<------AV------>|
	// Active --+                    +----------------+
	// Video    +--------------------+                +-----------------
	//                        |<---->|
	//                        |<---------AEND-------->|
	WriteIO32(&pRegister->DPCHTOTAL		, (U32)( HFP + HSW + HBP + AVWidth - 1 ));
	WriteIO32(&pRegister->DPCHSWIDTH	, (U32)( HSW - 1 ));
	WriteIO32(&pRegister->DPCHASTART	, (U32)( HSW + HBP - 1 ));
	WriteIO32(&pRegister->DPCHAEND		, (U32)( HSW + HBP + AVWidth - 1 ));
	WriteIO32(&pRegister->DPCVTOTAL		, (U32)( VFP + VSW + VBP + AVHeight - 1 ));
	WriteIO32(&pRegister->DPCVSWIDTH	, (U32)( VSW - 1 ));
	WriteIO32(&pRegister->DPCVASTART	, (U32)( VSW + VBP - 1 ));
	WriteIO32(&pRegister->DPCVAEND		, (U32)( VSW + VBP + AVHeight - 1 ));

	WriteIO32(&pRegister->DPCVSEOFFSET	, (U32) VSETPIXEL);
	WriteIO32(&pRegister->DPCVSSOFFSET	, (U32)( HFP + HSW + HBP + AVWidth-VSCLRPIXEL- 1 ));
	WriteIO32(&pRegister->DPCEVSEOFFSET	, (U32) EVENVSETPIXEL);
	WriteIO32(&pRegister->DPCEVSSOFFSET	, (U32)( HFP + HSW + HBP + AVWidth-EVENVSCLRPIXEL- 1 ));

	if (1==SyncGenMode)
	{
		WriteIO32(&pRegister->DPCEVTOTAL  , (U32)( EvenVFP + EvenVSW + EvenVBP + AVHeight - 1 ));
		WriteIO32(&pRegister->DPCEVSWIDTH , (U32)( EvenVSW - 1 ));
		WriteIO32(&pRegister->DPCEVASTART , (U32)( EvenVSW + EvenVBP - 1 ));
		WriteIO32(&pRegister->DPCEVAEND   , (U32)( EvenVSW + EvenVBP + AVHeight - 1 ));
	}

	regvalue = ReadIO32(&pRegister->DPCCTRL0) & 0xFFF0UL;
	regvalue |= (((U32)FieldPolarity<<2) | ((U32)VSyncPolarity<<1) | ((U32)HSyncPolarity<<0));
	WriteIO32(&pRegister->DPCCTRL0, (U32)regvalue);

	// user never set INTERMODE in this config..
	// INTERMODE must be set with SYNCENB
}


//------------------------------------------------------------------------------
/// @brief	Set color output format
//------------------------------------------------------------------------------
void NX_DPC_SetOutputFormat
(
	U32 ModuleIndex,
	OUTPUTFORMAT OutputFormat,	///< [in] video/color output format
	U8 OutputVideoConfig		///< [in] video output config
								///< [in] 0 : Cb Y Cr Y  |  1 : Cb Y Cr Y
								///< [in] 2 : Cb Y Cr Y  |  3 : Cb Y Cr Y
)
{


	const U32 FORMAT_TABLE[] =
	{
		(0<<0),			///< OUTPUTFORMAT_RGB555
		(1<<0),			///< OUTPUTFORMAT_RGB565
		(2<<0),			///< OUTPUTFORMAT_RGB666
		(3<<0),			///< OUTPUTFORMAT_RGB888
		(4<<0),			///< OUTPUTFORMAT_MRGB555A
		(5<<0),			///< OUTPUTFORMAT_MRGB555B
		(6<<0),			///< OUTPUTFORMAT_MRGB565
		(7<<0),			///< OUTPUTFORMAT_MRGB666
		(8<<0),			///< OUTPUTFORMAT_MRGB888A
		(9<<0),			///< OUTPUTFORMAT_MRGB888B
		(0<<0)|(1<<7),	///< OUTPUTFORMAT_BGR555
		(1<<0)|(1<<7),	///< OUTPUTFORMAT_BGR565
		(2<<0)|(1<<7),	///< OUTPUTFORMAT_BGR666
		(3<<0)|(1<<7),	///< OUTPUTFORMAT_BGR888
		(4<<0)|(1<<7),	///< OUTPUTFORMAT_MBGR555A
		(5<<0)|(1<<7),	///< OUTPUTFORMAT_MBGR555B
		(6<<0)|(1<<7),	///< OUTPUTFORMAT_MBGR565
		(7<<0)|(1<<7),	///< OUTPUTFORMAT_MBGR666
		(8<<0)|(1<<7),	///< OUTPUTFORMAT_MBGR888A
		(9<<0)|(1<<7),	///< OUTPUTFORMAT_MBGR888B
        (10<<0),         ///< OUTPUTFORMAT_CCIR656
        (11<<0),	    	///< OUTPUTFORMAT_CCIR601_8
        (12<<0), 		///< OUTPUTFORMAT_CCIR601_16A
        (13<<0), 		///< OUTPUTFORMAT_CCIR601_16B
        (14<<0), 		///< serial (ODD : R8->G8->B8), (EVEN : R8->G8->B8)
        (15<<0) 		///< serial (ODD,EVEN : R8->G8->B8->dummy)
	};

	U32 regvalue;
	U32 regvalue0;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = ReadIO32(&pRegister->DPCCTRL1) & 0x30FFUL;


	regvalue |= (FORMAT_TABLE[OutputFormat]<<8);
	WriteIO32(&pRegister->DPCCTRL1, (U32)regvalue);

	regvalue0 = (U32)(ReadIO32(&pRegister -> DPCCTRL1) & 0xff3f);
	regvalue0 = (U32)((OutputVideoConfig<<6) | regvalue0);
	WriteIO32(&pRegister -> DPCCTRL1, (U32)regvalue0);
}

//------------------------------------------------------------------------------
/// @brief	Set quantiazation mode
//------------------------------------------------------------------------------
void NX_DPC_SetQuantizationMode
(
    U32 ModuleIndex,
	QMODE RGB2YC,
	QMODE YC2RGB
)
{

	U32 regvalue;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = ReadIO32(&pRegister->DPCCTRL1) & 0x8FFFUL;
	regvalue |= ((U32)RGB2YC<<13) | ((U32)YC2RGB<<12);
	WriteIO32(&pRegister->DPCCTRL1, (U32)regvalue);
}


void NX_DPC_SetEnable
(
    U32 ModuleIndex,
	CBOOL Enable,			///< [in] display controller enable
	CBOOL RGBMode,          ///< [in] output format reb & ycbcr enable
	CBOOL UseNTSCSync,		///< [in] use NTSC encoder sync
	CBOOL UseAnalogOutput,	///< [in] use analog output(use DAC)
	CBOOL SEAVEnable        ///< [in] Start of active and End of active Enable
)
{
	U32 regvalue;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( ~UseAnalogOutput | UseNTSCSync );

	regvalue = ReadIO32(&pRegister->DPCCTRL0) & 0x0EFFUL;
	regvalue |= ((U32)Enable<<15) |
				((U32)UseNTSCSync<<14) | ((U32)SEAVEnable<<8) |((U32)UseAnalogOutput<<13) | ((U32)RGBMode<<12);

	WriteIO32(&pRegister->DPCCTRL0, (U32)regvalue);
}




void NX_DPC_SetOutVideoClkSelect
(
    U32 ModuleIndex,
    OUTPADCLKSEL OutPadVClkSel   ///< [in] Out Video clk selection
)
{

	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;


    WriteIO32(&pRegister -> DPCCTRL2, (U32)((ReadIO32(&pRegister->DPCCTRL2)) | (OutPadVClkSel&0x3)));
}

void NX_DPC_SetRegFlush( U32 ModuleIndex )
{
	U32 Reg;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	Reg = ReadIO32(&pRegister -> DPCDATAFLUSH);
	WriteIO32(&pRegister -> DPCDATAFLUSH, (U32) ( Reg | (1UL<<4)));
}



void NX_DPC_SetSRAMOn ( U32 ModuleIndex )
{

	U32 reg;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	reg = (U32)(ReadIO32(&pRegister -> DPCCTRL2) & 0xf3ff);
	WriteIO32(&pRegister -> DPCCTRL2, (U32)(reg | (1UL<<10)));

	reg = (U32)(ReadIO32(&pRegister -> DPCCTRL2) & 0xf7ff);
	WriteIO32(&pRegister -> DPCCTRL2, (U32)(reg | (1UL<<11)));
}

void NX_DPC_SetSyncLCDType
(
	U32 ModuleIndex,
	CBOOL STNLCD,
	CBOOL DualViewEnb,
	CBOOL BitWidh,
	U8 CPCycle
)
{
	U32 reg;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;


	reg = (U32)(ReadIO32(&pRegister -> DPCCTRL2) & 0xc0f);
	WriteIO32(&pRegister -> DPCCTRL2, (U32)(reg | (CPCycle<<12) | (BitWidh<<9) | (DualViewEnb<<8) | (STNLCD<<7)));
}




//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NX_DPC_SetUpScaleControl
(
	U32 ModuleIndex,
	CBOOL UpScaleEnb, 		// Sync Generator Source image Scale Enable
	CBOOL FilterEnb,
	U32 HScale, 			// Scale
	U16 SourceWidth			// Sync Generator Source image Width
)
{

	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister -> DPCUPSCALECON0, (U32)((HScale << 8) | ((U32)FilterEnb<<1) | (UpScaleEnb)));
    WriteIO32(&pRegister -> DPCUPSCALECON1, (U32)(HScale>>8));
    WriteIO32(&pRegister -> DPCUPSCALECON2, SourceWidth);
}



//----
// i80 Functions
//----
//------------------------------------------------------------------------------
/// @brief	Set/Get MPU parameters
//------------------------------------------------------------------------------
void NX_DPC_SetMPUTime(U32 ModuleIndex, U8 Setup, U8 Hold, U8 Acc)
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32( &pRegister->DPCMPUTIME0, (U32)((Setup<<8)|(Hold&0xff)) );
	WriteIO32( &pRegister->DPCMPUTIME1, (U32)(Acc) );
}
void NX_DPC_SetIndex(U32 ModuleIndex, U32 Index)
{
	U32 regvalue;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32( &pRegister->DPCMPUWRDATAL, (U32)(Index&0xffff) );
	WriteIO32( &pRegister->DPCMPUINDEX, (U32)((Index>>16)&0xff) );

	if (0x22 == Index)
	{
		regvalue = ReadIO32(&pRegister->DPCCTRL2);
		WriteIO32(&pRegister->DPCCTRL2, (regvalue|0x10)  );
	}
}
void NX_DPC_SetData(U32 ModuleIndex, U32 Data)
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32( &pRegister->DPCMPUWRDATAL, (U32)(Data&0xffff) );
	WriteIO32( &pRegister->DPCMPUDATAH, (U32)((Data>>16)&0xff) );
}

void NX_DPC_SetCmdBufferFlush(U32 ModuleIndex )
{
	U32 Reg;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	Reg = ReadIO32(&pRegister -> DPCDATAFLUSH)	;
	WriteIO32( &pRegister -> DPCDATAFLUSH, (U32)(Reg | (1<<1)) );
}
void NX_DPC_SetCmdBufferClear( U32 ModuleIndex )
{
	U32 Reg;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	Reg = ReadIO32( &pRegister -> DPCDATAFLUSH );
	WriteIO32( &pRegister -> DPCDATAFLUSH, (U32)(Reg | (1<<0)) );
}

void NX_DPC_SetCmdBufferWrite(U32 ModuleIndex, U32 CmdData )
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32( &pRegister -> DPCCMDBUFFERDATAL, (U32)(CmdData &0xffff) );
	WriteIO32( &pRegister -> DPCCMDBUFFERDATAH, (U32)(CmdData>>16) );
}

void NX_DPC_SetMPUCS1( U32 ModuleIndex )
{
	U32 Reg;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	Reg = ReadIO32( &pRegister -> DPCPOLCTRL)	;
	WriteIO32( &pRegister -> DPCPOLCTRL, (U32)(Reg|0x1) );
}

U32 NX_DPC_GetData( U32 ModuleIndex )
{
	U32 Reg = 0;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	Reg = ReadIO32( &pRegister->DPCMPUDATAH );
	Reg = (Reg<<16)| ReadIO32( &pRegister->DPCMPURDATAL );

	return Reg;
}
U32 NX_DPC_GetStatus( U32 ModuleIndex )
{
	U32 Reg = 0;
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	Reg = ReadIO32( &pRegister->DPCMPUSTATUS );

	Reg = (Reg<<16)| ReadIO32( &pRegister->DPCMPURDATAL );

	return Reg;
}

void NX_DPC_RGBMASK( U32 ModuleIndex, U32 RGBMask )
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->DPCRGBMASK[0], (RGBMask>>0)&0xffff);
	WriteIO32(&pRegister->DPCRGBMASK[1], (RGBMask>>16)&0x00ff);

}


void NX_DPC_SetPadLocation( U32 ModuleIndex, U32 Index, U32 regvalue )
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->DPCPADPOSITION[Index], regvalue);
}

U32 NX_DPC_GetFieldFlag( U32 ModuleIndex )
{
	register struct NX_DPC_RegisterSet* pRegister;
	U32 regvalue;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = ReadIO32(&pRegister->DPCRGBSHIFT);

	return ((regvalue >> 5) && 0x01 );
}

void NX_DPC_SetEnable_WITH_INTERLACE
(
	U32 ModuleIndex,
	CBOOL Enable,			///< [in] display controller enable
	CBOOL RGBMode,			///< [in] output format reb & ycbcr enable
	CBOOL UseNTSCSync,		///< [in] use NTSC encoder sync
	CBOOL UseAnalogOutput,	///< [in] use analog output(use DAC)
	CBOOL SEAVEnable		///< [in] Start of active and End of active Enable
)
{
	U32 regvalue;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( ~UseAnalogOutput | UseNTSCSync );

	regvalue = ReadIO32(&pRegister->DPCCTRL0) & 0x0EFF;
	regvalue = ReadIO32(&pRegister->DPCCTRL0) & 0x0EFF;
	regvalue |= ((U32)Enable<<15) |
				((U32)UseNTSCSync<<14) | ((U32)SEAVEnable<<8) |((U32)UseAnalogOutput<<13) | ((U32)RGBMode<<12);

	// @added by choiyk 2014/05/08 pm0710
	// INTERLACE MODE
	regvalue |= (1<<9);

	WriteIO32(&pRegister->DPCCTRL0, (U16)regvalue);
}

//===========================================================
// NTSC encoder
//===========================================================
void NX_DPC_SetEncoderControlReg		(U32 ModuleIndex, U32 ParamA, U32 ParamB, U32 ParamC)
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_ECMDA, ParamA);
	WriteIO32(&pRegister->NTSC_ECMDB, ParamB);
	WriteIO32(&pRegister->NTSC_ECMDC, ParamC);
}

void NX_DPC_SetEncoderSHCPhaseControl	(U32 ModuleIndex, U32 ChromaParam)
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_SCH, ChromaParam);
}

void NX_DPC_SetEncoderTimingConfigReg	(U32 ModuleIndex, U32 ICNTL)
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_ICNTL, ICNTL);
}

void NX_DPC_SetEncoderDACOutputSelect	(U32 ModuleIndex,
										 U8 DACSEL0,    U8 DACSEL1,    U8 DACSEL2,
										 U8 DACSEL3,    U8 DACSEL4,    U8 DACSEL5  )
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_DACSEL10, ((DACSEL1&0xf)<<4) | (DACSEL0&0xf) );
	WriteIO32(&pRegister->NTSC_DACSEL32, ((DACSEL3&0xf)<<4) | (DACSEL2&0xf) );
	WriteIO32(&pRegister->NTSC_DACSEL54, ((DACSEL5&0xf)<<4) | (DACSEL4&0xf) );
}

void NX_DPC_SetEncoderSyncLocation		(U32 ModuleIndex,
										 U16 HSOE,    U16 HSOB,    U16 VSOB,
										 U16 VSOE,    U8 VSOST,    CBOOL NOVRST )
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_HSVSO,(U16)((((VSOB&0x100)>>2)|((HSOB&0x700)>>5)|(HSOE&0x700)>>8)) );
	WriteIO32(&pRegister->NTSC_HSOE ,(U16)(HSOE&0xff) );
	WriteIO32(&pRegister->NTSC_HSOB ,(U16)(HSOB&0xff) );
	WriteIO32(&pRegister->NTSC_VSOB ,(U16)(VSOB&0xff) );
	WriteIO32(&pRegister->NTSC_VSOE ,(U16)(((VSOST&0x3)<<6) | (NOVRST<<5) | (VSOE&0x1f)) );
}

void NX_DPC_SetEncoderDACPowerEnable	(U32 ModuleIndex, U8 DACPD)
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_DACPD, DACPD);
}


void	NX_DPC_SetYCOrder ( U32 ModuleIndex, NX_DPC_YCORDER ycorder )
{
	// DPC Control 1 register
	const U16 YCORDER_POS	= 6;
	register struct NX_DPC_RegisterSet*	pRegister;
	U32 temp;

	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 4 > ycorder );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	temp  = pRegister->DPCCTRL1 & (~(0xF<<YCORDER_POS));
	temp  = (U16)(temp | (ycorder << YCORDER_POS));
	WriteIO16(&pRegister->DPCCTRL1, temp);
}

void	NX_DPC_SetLumaGain ( U32 ModuleIndex, U32 LumaGain )
{
	register struct NX_DPC_RegisterSet*	pRegister;
	
	NX_ASSERT( NUMBER_OF_DPC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO16(&pRegister->NTSC_CONT, LumaGain);
}


//------------------------------------------------------------------------------
// Internal video encoder operations
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/**
 *	@brief	    Enable/Disable the internal video encoder.
 *  @param[in]  bEnb    Set it to CTRUE to enable the internal video encoder.
 *	@return	    None.
 *	@remark	    This function also controls the RESET of the internal video encoder and
 *              internal DAC's Enable signal.
 *			    Therefore you have to enable the internal video encoder before using
 *			    funtions relative to the internal video encoder. And also if you
 *			    disable the internal video encoder, all registers of the internal
 *			    video encoder will be reset. Therefore you have to re-initialize the
 *			    internal video encoder again. This function is only valid for secondary
 *			    display controller.
 *	@see	    NX_DPC_GetENCEnable, NX_DPC_SetVideoEncoderPowerDown, NX_DPC_GetVideoEncoderPowerDown,
 *			    NX_DPC_SetVideoEncoderMode, NX_DPC_SetVideoEncoderSCHLockControl, NX_DPC_GetVideoEncoderSCHLockControl,
 *			    NX_DPC_SetVideoEncoderBandwidth, NX_DPC_GetVideoEncoderBandwidth,
 *			    NX_DPC_SetVideoEncoderColorControl, NX_DPC_GetVideoEncoderColorControl,
 *			    NX_DPC_SetVideoEncoderTiming, NX_DPC_GetVideoEncoderTiming
 */
void    NX_DPC_SetENCEnable( U32 ModuleIndex, CBOOL	bEnb )
{
	const U16 ENCMODE	= 1U<<14;
	const U16 ENCRST	= 1U<<13;
	const U16 INTPEND	= 1U<<10;
	register struct NX_DPC_RegisterSet* pRegister;
	register U16 temp;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	temp = ReadIO32(&pRegister->DPCCTRL0);
	temp &= (U16)~INTPEND;
	if( bEnb )	temp |= (U16) ENCRST;
	else		temp &= (U16)~ENCRST;
	WriteIO32(&pRegister->DPCCTRL0, (temp | ENCMODE));

	WriteIO32(&pRegister->NTSC_ICNTL, 7);
}

//------------------------------------------------------------------------------
/**
 *	@brief	Informs whether the internal video encoder is enabled or disabled.
 *	@return \b CTRUE indicates the internal video encoder is enabled.\n
 *			\b CFALSE indicates the internal video encoder is disabled.
 *	@remark This function is only valid for secondary display controller.
 *	@see	NX_DPC_SetENCEnable
 */
CBOOL   NX_DPC_GetENCEnable( U32 ModuleIndex )
{
	const U16 ENCRST	= 1U<<13;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	return (ReadIO32(&pRegister->DPCCTRL0) & ENCRST) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Specifies power down mode of the internal video encoder.
 *  @param[in]  bEnb    \b CTRUE specifies power down mode for the entire digital logic circuits
 *		        of encoder digital core.\n
 *		        \b CFALSE specifies normal operatation.
 *	@return	    None.
 *	@remark	    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_GetVideoEncoderPowerDown
 */
void    NX_DPC_SetVideoEncoderPowerDown( U32 ModuleIndex, CBOOL	bEnb )
{
	const U16 PWDENC = 1U<<7;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	if( bEnb )
	{
		WriteIO32(&pRegister->NTSC_ECMDA, ReadIO32(&pRegister->NTSC_ECMDA) | (U16) PWDENC);
		WriteIO32(&pRegister->NTSC_DACSEL10, 0);
	}
	else
	{
		WriteIO32(&pRegister->NTSC_DACSEL10, 1);
		WriteIO32(&pRegister->NTSC_ECMDA, ReadIO32(&pRegister->NTSC_ECMDA) & (U16)~PWDENC);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief	Determines whether the internal video encoder is power down or not.
 *	@return	\b CTRUE indicates power down mode for the entire digital logic circuits
 *			of encoder digital core.\n
 *			\b CFALSE indicates normal operation.\n
 *	@remark	You have to call this function only when the internal video encoder is enabled.
 *			And this function is only valid for secondary display controller.
 *	@see	NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderPowerDown
 */
CBOOL   NX_DPC_GetVideoEncoderPowerDown( U32 ModuleIndex )
{
	const U16 PWDENC = 1U<<7;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	return (ReadIO32(&pRegister->NTSC_ECMDA) & PWDENC) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Set Video Broadcast Standard mode for the internal video encoder.
 *  @param[in]  vbs     	Specifies video broadcast standard.
 *  @param[in]  bPedestal   Specifies whether the video output has a pedestal or not.
 *	@return	    None.
 *	@remark	    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable
 */
void    NX_DPC_SetVideoEncoderMode( U32 ModuleIndex, NX_DPC_VBS vbs, CBOOL	bPedestal )
{
	register struct NX_DPC_RegisterSet* pRegister;
	// to avoid below compile error of gcc :
	// 		error: initializer element is not constant
	// 			   (near initialization for ‘NTSC_ECMDA_TABLE[0]’)
	#define PHALT 			(1U<<0)
	#define IFMT 			(1U<<1)
	#define PED 			(1U<<3)
	#define FSCSEL_NTSC 	(0U<<4)
	#define FSCSEL_PAL 		(1U<<4)
	#define FSCSEL_PALM 	(2U<<4)
	#define FSCSEL_PALN 	(3U<<4)
	#define FDRST 			(1U<<6)
	#define PWDENC 			(1U<<7)
/*
	const U8 PHALT	= 1U<<0;
	const U8 IFMT	= 1U<<1;
	const U8 PED	= 1U<<3;
	const U8 FSCSEL_NTSC	= 0U<<4;
	const U8 FSCSEL_PAL 	= 1U<<4;
	const U8 FSCSEL_PALM	= 2U<<4;
	const U8 FSCSEL_PALN	= 3U<<4;
	const U8 FDRST	= 1U<<6;
	const U16 PWDENC	= 1U<<7;
*/
	register U16 temp;

	static const U8 NTSC_ECMDA_TABLE[] =
	{
		(U8)(       FSCSEL_NTSC         | FDRST),	// NTSC-M             59.94 Hz(525)
		(U8)(IFMT | FSCSEL_NTSC                ),	// NTSC-N             50 Hz(625)
		(U8)(       FSCSEL_PAL                 ),	// NTSC-4.43          60 Hz(525)
		(U8)(       FSCSEL_PALM | PHALT        ),	// PAL-M              59.952 Hz(525)
		(U8)(IFMT | FSCSEL_PALN | PHALT        ),	// PAL-combination N  50 Hz(625)
		(U8)(IFMT | FSCSEL_PAL  | PHALT | FDRST),	// PAL-B,D,G,H,I,N    50 Hz(625)
		(U8)(       FSCSEL_PAL  | PHALT        ),	// Pseudo PAL         60 Hz(525)
		(U8)(IFMT | FSCSEL_NTSC                )	// Pseudo NTSC        50 Hz (625)
	};

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );
	NX_ASSERT( 8 > vbs );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	temp = ReadIO32(&pRegister->NTSC_ECMDA);
	temp &= (U16)PWDENC;		// mask PWDENB bit.
	temp = (U16)( temp | (U16)NTSC_ECMDA_TABLE[vbs]);
	if( bPedestal )	temp |= (U16)PED;
	WriteIO32(&pRegister->NTSC_ECMDA, temp);

	#undef PHALT
	#undef IFMT
	#undef PED
	#undef FSCSEL_NTSC
	#undef FSCSEL_PAL
	#undef FSCSEL_PALM
	#undef FSCSEL_PALN
	#undef FDRST
	#undef PWDENC
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Set SCH Chroma-Luma locking control for the internal video encoder.
 *  @param[in]  bFreeRun    \b CTRUE specifies Chroma is free running as compared to horizontal sync.\n
 *		        \b CFALSE specifies Constant relationship between color burst and horizontal
 *		        sync maintained for appropriate video standards.
 *	@return	    None.
 *	@remark	    The function SetVideoEncoderMode also affects the result of this function.\n
 *			    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderMode, NX_DPC_GetVideoEncoderSCHLockControl
 */
void    NX_DPC_SetVideoEncoderSCHLockControl( U32 ModuleIndex, CBOOL bFreeRun )
{
	const U16 FDRST = 1U<<6;
	register struct NX_DPC_RegisterSet* pRegister;
	register U16 temp;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	temp = ReadIO32(&pRegister->NTSC_ECMDA);
	if( bFreeRun )	temp |= (U16) FDRST;
	else			temp &= (U16)~FDRST;
	WriteIO32(&pRegister->NTSC_ECMDA, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get SCH Chroma-Luma locking control for the internal video encoder.
 *	@return	\b CTRUE indicates Chroma is free running as compared to horizontal sync.\n
 *			\b CFALSE indicates Constant relationship between color burst and horizontal
 *			sync maintained for appropriate video standards.
 *	@remark	The function SetVideoEncoderMode also affects the result of this function.\n
 *			You have to call this function only when the internal video encoder is enabled.
 *			And this function is only valid for secondary display controller.
 *	@see	NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderMode, NX_DPC_SetVideoEncoderSCHLockControl
 */
CBOOL   NX_DPC_GetVideoEncoderSCHLockControl( U32 ModuleIndex )
{
	const U16 FDRST = 1U<<6;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	return (ReadIO32(&pRegister->NTSC_ECMDA) & FDRST) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Set Chroma and Luma bandwidth control for the internal video encoder.
 *  @param[in]  Luma	Specifies luma bandwidth.
 *  @param[in]  Chroma	Specifies chroma bandwidth.
 *	@return	    None.
 *	@remark	    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_GetVideoEncoderBandwidth
 */
void    NX_DPC_SetVideoEncoderBandwidth( U32 ModuleIndex, NX_DPC_BANDWIDTH	Luma, NX_DPC_BANDWIDTH	Chroma )
{
	const U16 YBW_POS	= 0;
	const U16 CBW_POS	= 2;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );
	NX_ASSERT( 3 > Luma );
	NX_ASSERT( 3 > Chroma );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_ECMDB, (U16)((Chroma<<CBW_POS) | (Luma<<YBW_POS)));
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Get Chroma and Luma bandwidth control for the internal video encoder.
 *  @param[out] pLuma		Indicates current luma bandwidth.
 *  @param[out] pChroma	    Indicates current chroma bandwidth.
 *	@return	    None.
 *	@remark	    Arguments which does not required can be CNULL.\n
 *			    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderBandwidth
 */
void    NX_DPC_GetVideoEncoderBandwidth( U32 ModuleIndex, NX_DPC_BANDWIDTH	*pLuma, NX_DPC_BANDWIDTH	*pChroma )
{
	const U16 YBW_POS	= 0;
	const U16 YBW_MASK	= 3U<<YBW_POS;
	const U16 CBW_POS	= 2;
	const U16 CBW_MASK	= 3U<<CBW_POS;
	register struct NX_DPC_RegisterSet* pRegister;

	register U16 temp;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	temp = ReadIO32(&pRegister->NTSC_ECMDB);
	if( CNULL != pLuma  )	*pLuma		= (NX_DPC_BANDWIDTH)((temp & YBW_MASK)>>YBW_POS);
	if( CNULL != pChroma )	*pChroma	= (NX_DPC_BANDWIDTH)((temp & CBW_MASK)>>CBW_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Set the color burst phase and HUE, Saturation for the internal video encoder.
 *  @param[in]  SCH		Specifies the color burst phase relative to the horizontal sync.
 *		        	    The 8 bit control covers the entire 360 range as a 2's compliment number.
 *		        	    '0' is nominal value.
 *  @param[in]  HUE		Specifies the active video color burst phase relative to color burst.
 *		        	    The 8 bit control covers the entire 360 range as a 2's compliment number.
 *		        	    '0' is nominal value.
 *  @param[in]  SAT		Specifies the active video chroma gain relative to the color burst gain.
 *		        	    This value is 2's compliment with '0' the nominal value.
 *  @param[in]  CRT     Specifies the Luma gain. This value is 2's compliment with '0' the nominal value.
 *  @param[in]  BRT     Specifies the Luma offset. This value is 2's compliment with '0' the nominal value.
 *	@return	    None.
 *	@remark	    This funcion can operate only when SCH Chroma-Luma locking control is constant mode.\n
 *			    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_GetVideoEncoderColorControl
 */
void    NX_DPC_SetVideoEncoderColorControl( U32 ModuleIndex, S8 SCH, S8 HUE, S8 SAT, S8 CRT, S8 BRT )
{
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_SCH, (U16)SCH);
	WriteIO32(&pRegister->NTSC_HUE, (U16)HUE);
	WriteIO32(&pRegister->NTSC_SAT, (U16)SAT);
	WriteIO32(&pRegister->NTSC_CONT, (U16)CRT);
	WriteIO32(&pRegister->NTSC_BRIGHT, (U16)BRT);
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Get the color burst phase and HUE, Saturation for the internal video encoder.
 *  @param[out] pSCH	 Get current color burst phase relative to the horizontal sync.
 *		        		 The 8 bit control covers the entire 360 range as a 2's compliment number.
 *		        		 '0' is nominal value.
 *  @param[out] pHUE	 Get current active video color burst phase relative to color burst.
 *		        		 The 8 bit control covers the entire 360 range as a 2's compliment number.
 *		        		 '0' is nominal value.
 *  @param[out] pSAT	 Get current active video chroma gain relative to the color burst gain.
 *		        		 This value is 2's compliment with '0' the nominal value.
 *  @param[out] pCRT     Get current Luma gain. This value is 2's compliment with '0' the nominal value.
 *  @param[out] pBRT     Get current Luma offset. This value is 2's compliment with '0' the nominal value.
 *	@return	    None.
 *	@remark	    Arguments which does not required can be CNULL.\n
 *			    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderColorControl
 */
void    NX_DPC_GetVideoEncoderColorControl( U32 ModuleIndex, S8 *pSCH, S8 *pHUE, S8 *pSAT, S8 *pCRT, S8 *pBRT )
{
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	if( CNULL != pSCH )    *pSCH = (S8)ReadIO32(&pRegister->NTSC_SCH);
	if( CNULL != pHUE )    *pHUE = (S8)ReadIO32(&pRegister->NTSC_HUE);
	if( CNULL != pSAT )    *pSAT = (S8)ReadIO32(&pRegister->NTSC_SAT);
	if( CNULL != pCRT )    *pCRT = (S8)ReadIO32(&pRegister->NTSC_CONT);
	if( CNULL != pBRT )    *pBRT = (S8)ReadIO32(&pRegister->NTSC_BRIGHT);
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Set the adjust value for the color burst frequency of the internal video encoder.
 *  @param[in]  Adjust	Specifies the color burst frequency adjustment value.\n
 *						Allows the pixel clock to be varied up to +/-200 ppm of
 * 						its nominal value. This allows dot crawl adjustment.
 *						This 16 bit value is multiplied by 4 and added to the
 *						internal chroma frequency constant.
 *	@return	    None.
 *	@remark	    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_GetVideoEncoderFSCAdjust
 */
void    NX_DPC_SetVideoEncoderFSCAdjust( U32 ModuleIndex, S16 Adjust )
{
	register struct NX_DPC_RegisterSet* pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->NTSC_FSC_ADJH, (U16)(Adjust >> 8));
	WriteIO32(&pRegister->NTSC_FSC_ADJL, (U16)(Adjust & 0xFF));
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Get the adjust value for the color burst frequency of the internal video encoder.
 *	@return	    The color burst frequency adjustment value.
 *	@remark	    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderFSCAdjust
 */
U16 NX_DPC_GetVideoEncoderFSCAdjust( U32 ModuleIndex )
{
	register U32 temp;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	temp = (U32)ReadIO32(&pRegister->NTSC_FSC_ADJH);
	temp <<= 8;
	temp |= (((U32)ReadIO32(&pRegister->NTSC_FSC_ADJL)) & 0xFF);
	return (U16)temp;
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Set parameters for sync signals of the internal video encoder.
 *  @param[in]  HSOS	 Specifies the number of clocks for the horizontal sync width, 1 ~ 2048.
 *  @param[in]  HSOE	 Specifies the number of total clocks for the horizontal period, 1 ~ 2048.
 *  @param[in]  VSOS	 Specifies the number of lines for the start of vertical sync, 0 ~ 511.
 *  @param[in]  VSOE	 Specifies the number of lines for the end of vertical sync, 0 ~ 63.
 *	@return	    None.
 *	@remark	    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.\n
 *			    The unit is VCLK2(two clock for a pixel) for HSOS and HSOE.\n
 *			    See follwing figure for more details.
 *	@code
 *
 *	                   <----------------HSOE----------------->
 * 	                   <-HSOS->
 *	  HSync  ---------+        +--------------/-/-------------+        +---
 *	                  |        |                              |        |
 *	                  +--------+                              +--------+
 *
 *    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	                   <------VSOE------>
 * 	                   <-VSOS->
 *	  VSync  ------------------+         +--------------/-/-----------------
 *	                  ^        |         |
 *	                  |        +---------+
 *	        Vertical -+
 *          Start
 *
 *	@endcode
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_GetVideoEncoderTiming
 */
void    NX_DPC_SetVideoEncoderTiming( U32 ModuleIndex, U32	HSOS, U32 HSOE,	U32 VSOS, U32 VSOE )
{
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );
	NX_ASSERT( 2048 > (HSOS-1) );
	NX_ASSERT( 2048 > (HSOE-1) );
	NX_ASSERT( 512 > VSOS );
	NX_ASSERT( 64 > VSOE );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	HSOS -= 1;
	HSOE -= 1;
	WriteIO32(&pRegister->NTSC_HSVSO, (U16)((((VSOS >> 8)&1U)<<6) | (((HSOS >> 8)&7U)<<3) | (((HSOE >> 8)&7U)<<0)));
	WriteIO32(&pRegister->NTSC_HSOB, (U16)(HSOS & 0xFFU));
	WriteIO32(&pRegister->NTSC_HSOE, (U16)(HSOE & 0xFFU));
	WriteIO32(&pRegister->NTSC_VSOB, (U16)(VSOS & 0xFFU));
	WriteIO32(&pRegister->NTSC_VSOE, (U16)(VSOE & 0x1FU));
}

//------------------------------------------------------------------------------
/**
 *	@brief	    Get current parameters for sync signals of the internal video encoder.
 *  @param[out] pHSOS	Get current number of clocks for the horizontal sync width, 1 ~ 2048.
 *  @param[out] pHSOE	Get current number of total clocks for the horizontal period, 1 ~ 2048.
 *  @param[out] pVSOS	Get current number of lines for the start of vertical sync, 0 ~ 511.
 *  @param[out] pVSOE	Get current number of lines for the end of vertical sync, 0 ~ 63.
 *	@return	    None.
 *	@remark	    The unit is VCLK2(two clock for a pixel) for HSOS and HSOE.\n
 *			    Arguments which does not required can be CNULL.\n
 *			    You have to call this function only when the internal video encoder is enabled.
 *			    And this function is only valid for secondary display controller.
 *	@see	    NX_DPC_SetENCEnable, NX_DPC_SetVideoEncoderTiming
 */
void    NX_DPC_GetVideoEncoderTiming
(
	U32 ModuleIndex,
	U32	*pHSOS,
	U32 *pHSOE,
	U32 *pVSOS,
	U32 *pVSOE
)
{
	register U16 HSVSO;
	register struct NX_DPC_RegisterSet* pRegister;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CTRUE == NX_DPC_GetENCEnable( ModuleIndex ) );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	HSVSO = ReadIO32(&pRegister->NTSC_HSVSO);
	if( CNULL != pHSOS )	*pHSOS = (U32)((((HSVSO>>3)&7U)<<8) | (ReadIO32(&pRegister->NTSC_HSOB) & 0xFFU)) + 1;
	if( CNULL != pHSOE )	*pHSOE = (U32)((((HSVSO>>0)&7U)<<8) | (ReadIO32(&pRegister->NTSC_HSOE) & 0xFFU)) + 1;
	if( CNULL != pVSOS )	*pVSOS = (U32)((((HSVSO>>6)&1U)<<8) | (ReadIO32(&pRegister->NTSC_VSOB) & 0xFFU));
	if( CNULL != pVSOE )	*pVSOE = (U32)(ReadIO32(&pRegister->NTSC_VSOE) & 0x1FU);
}



