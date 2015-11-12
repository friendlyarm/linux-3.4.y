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
//	Module		: CCI 400 Base Bus Control
//	File		: nx_cci400.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.09.04	Hans create
//------------------------------------------------------------------------------

#include "nx_cci400.h"


static	struct NX_CCI400_RegisterSet *__g_pRegister = CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		 CTRUE	indicates that Initialize is succeeded.
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_CCI400_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = (struct NX_CCI400_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_CCI400_GetNumberOfModule( void )
{
	return NUMBER_OF_CCI400_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_CCI400_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_CCI400_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_CCI400_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_CCI400_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_CCI400_SetBaseAddress( void* BaseAddress )

{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_CCI400_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */

void*	NX_CCI400_GetBaseAddress( void )
{
	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicates that Initialize is succeeded. 
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_CCI400_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicates that Deinitialize is succeeded. 
 *				 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_CCI400_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicates that Module is Busy. 
 *				 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_CCI400_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		 CTRUE	indicates that Ready to enter power-down stage. 
 *				 CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_CCI400_CanPowerDown( void )
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


