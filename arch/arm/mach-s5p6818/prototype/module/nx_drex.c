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
//	Module		: DDR SDRAM Controller
//	File		: nx_drex.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.08.20 Hans create
//------------------------------------------------------------------------------

#include "nx_drex.h"


static	struct NX_DREXSDRAM_RegisterSet *__g_pSDRAMRegister = (struct NX_DREXSDRAM_RegisterSet *)CNULL;
static	struct NX_DREXTZ_RegisterSet *__g_pTZRegister = (struct NX_DREXTZ_RegisterSet *)CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		\b CTRUE	indicates that Initialize is succeeded.\n
 *				\b CFALSE	indicates that Initialize is failed.\n
 */
CBOOL	NX_DREXSDRAM_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pSDRAMRegister = (struct NX_DREXSDRAM_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_DREXSDRAM_GetNumberOfModule( void )
{
	return NUMBER_OF_DREX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_DDRSDRAM_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_DREX_MODULE_CH0_APB );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_DREXSDRAM_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DREXSDRAM_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */

void	NX_DREXSDRAM_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pSDRAMRegister = (struct NX_DREXSDRAM_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */

void*	NX_DREXSDRAM_GetBaseAddress( void )
{

	return (void*)__g_pSDRAMRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicates that Initialize is succeeded. \n
 *				\b CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_DREXSDRAM_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicates that Deinitialize is succeeded. \n
 *				\b CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_DREXSDRAM_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicates that Module is Busy. \n
 *				\b CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_DREXSDRAM_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_DREXSDRAM_CanPowerDown( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
// there is no interrupt interface


//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		\b CTRUE	indicates that Initialize is succeeded.\n
 *				\b CFALSE	indicates that Initialize is failed.\n
 */
CBOOL	NX_DREXTZ_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pTZRegister = (struct NX_DREXTZ_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_DREXTZ_GetNumberOfModule( void )
{
	return NUMBER_OF_DREX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_DREXTZ_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_DREX_TZ_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_DREXTZ_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DREXTZ_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_DREXTZ_SetBaseAddress( U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pTZRegister = (struct NX_DREXTZ_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
U32		NX_DREXTZ_GetBaseAddress( void )
{
	return (U32)__g_pTZRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicates that Initialize is succeeded. \n
 *				\b CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_DREXTZ_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicates that Deinitialize is succeeded. \n
 *				\b CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_DREXTZ_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicates that Module is Busy. \n
 *				\b CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_DREXTZ_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_DREXTZ_CanPowerDown( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
// there is no interrupt interface


