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
//	Module		: TZC 380 TrustZone Address Space Controller
//	File		: nx_tzc380.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.09.11	Hans create
//------------------------------------------------------------------------------

#include "nx_tzc380.h"


static	struct NX_TZC380_RegisterSet (*__g_pRegister)[1] = (struct NX_TZC380_RegisterSet (*)[])CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		\b CTRUE	indicates that Initialize is succeeded.\n
 *				\b CFALSE	indicates that Initialize is failed.\n
 */
CBOOL	NX_TZC380_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = (struct NX_TZC380_RegisterSet (*)[])CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_TZC380_GetNumberOfModule( void )
{
	return 3;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_TZC380_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_XIUA_DREX0_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_TZC380_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_TZC380_RegisterSet) * 3;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */

void	NX_TZC380_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_TZC380_RegisterSet (*)[])BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */

void*	NX_TZC380_GetBaseAddress( void )
{
	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicates that Initialize is succeeded. \n
 *				\b CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_TZC380_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicates that Deinitialize is succeeded. \n
 *				\b CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_TZC380_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicates that Module is Busy. \n
 *				\b CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_TZC380_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_TZC380_CanPowerDown( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
// there is no interrupt interface

//------------------------------------------------------------------------------
//	Clock Management
//--------------------------------------------------------------------------
// there in no clock management feature

//--------------------------------------------------------------------------
// Power Management
//--------------------------------------------------------------------------


