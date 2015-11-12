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
//	Module		: Clock & Power Manager
//	File		: nx_clkpwr.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------


#include "nx_chip.h"
#include "nx_clkpwr.h"
#include "nx_pmu.h"


static	struct NX_PMU_RegisterSet *__g_pRegister = CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		\b CTRUE	indicates that Initialize is succeeded.\n
 *				\b CFALSE	indicates that Initialize is failed.\n
 *	@see									NX_PMU_GetNumberOfModule
 */
CBOOL	NX_PMU_Initialize( void )
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
 *	@see		NX_PMU_Initialize
 */
U32		NX_PMU_GetNumberOfModule( void )
{
	return NUMBER_OF_PMU_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 *	@see											NX_PMU_GetSizeOfRegisterSet,
 *				NX_PMU_SetBaseAddress,			NX_PMU_GetBaseAddress,
 *				NX_PMU_OpenModule,				NX_PMU_CloseModule,
 *				NX_PMU_CheckBusy,				NX_PMU_CanPowerDown
 */
U32		NX_PMU_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_PMU_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_PMU_GetPhysicalAddress,
 *				NX_PMU_SetBaseAddress,			NX_PMU_GetBaseAddress,
 *				NX_PMU_OpenModule,				NX_PMU_CloseModule,
 *				NX_PMU_CheckBusy,				NX_PMU_CanPowerDown
 */
U32		NX_PMU_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_PMU_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_PMU_GetPhysicalAddress,		NX_PMU_GetSizeOfRegisterSet,
 *													NX_PMU_GetBaseAddress,
 *				NX_PMU_OpenModule,				NX_PMU_CloseModule,
 *				NX_PMU_CheckBusy,				NX_PMU_CanPowerDown
 */
void	NX_PMU_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_PMU_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_PMU_GetPhysicalAddress,		NX_PMU_GetSizeOfRegisterSet,
 *				NX_PMU_SetBaseAddress,
 *				NX_PMU_OpenModule,				NX_PMU_CloseModule,
 *				NX_PMU_CheckBusy,				NX_PMU_CanPowerDown
 */
void*	NX_PMU_GetBaseAddress( void )
{
	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicates that Initialize is succeeded. \n
 *				\b CFALSE	indicates that Initialize is failed.
 *	@see		NX_PMU_GetPhysicalAddress,		NX_PMU_GetSizeOfRegisterSet,
 *				NX_PMU_SetBaseAddress,			NX_PMU_GetBaseAddress,
 *													NX_PMU_CloseModule,
 *				NX_PMU_CheckBusy,				NX_PMU_CanPowerDown
 */
CBOOL	NX_PMU_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicates that Deinitialize is succeeded. \n
 *				\b CFALSE	indicates that Deinitialize is failed.
 *	@see		NX_PMU_GetPhysicalAddress,		NX_PMU_GetSizeOfRegisterSet,
 *				NX_PMU_SetBaseAddress,			NX_PMU_GetBaseAddress,
 *				NX_PMU_OpenModule,
 *				NX_PMU_CheckBusy,				NX_PMU_CanPowerDown
 */
CBOOL	NX_PMU_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicates that Module is Busy. \n
 *				\b CFALSE	indicates that Module is NOT Busy.
 *	@see		NX_PMU_GetPhysicalAddress,		NX_PMU_GetSizeOfRegisterSet,
 *				NX_PMU_SetBaseAddress,			NX_PMU_GetBaseAddress,
 *				NX_PMU_OpenModule,				NX_PMU_CloseModule,
 *													NX_PMU_CanPowerDown
 */
CBOOL	NX_PMU_CheckBusy( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 *	@see		NX_PMU_GetPhysicalAddress,		NX_PMU_GetSizeOfRegisterSet,
 *				NX_PMU_SetBaseAddress,			NX_PMU_GetBaseAddress,
 *				NX_PMU_OpenModule,				NX_PMU_CloseModule,
 *				NX_PMU_CheckBusy
 */
CBOOL	NX_PMU_CanPowerDown( void )
{
	return CFALSE;
}


void NX_PMU_SetPowerDown (  U32 domain, NX_PMU_POWER_SWITCH  powerswitch)
{
	const U32 POWER_OFF_nISOLATE = 0;
	const U32 POWER_OFF_SC = 1;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( domain < NUMBER_OF_PMU_POWER_DOMAIN );


	switch ( powerswitch )
	{
	case NX_PMU_POWER_SWITCH_ISOLATE:
		__g_pRegister->nISOLATE = NX_BIT_SetBit32(__g_pRegister->nISOLATE, POWER_OFF_nISOLATE, domain  );
		//__g_pRegister->nISOLATE = NX_BIT_SetBit32(__g_pRegister->nISOLATE, domain , POWER_OFF_nISOLATE  );
		break;
	case NX_PMU_POWER_SWITCH_SCPRE  :
		__g_pRegister->SCPRE = NX_BIT_SetBit32(__g_pRegister->SCPRE, POWER_OFF_SC, domain  );
		//__g_pRegister->SCPRE = NX_BIT_SetBit32(__g_pRegister->SCPRE, domain, POWER_OFF_SC  );
		break;
	case NX_PMU_POWER_SWITCH_SCALL  :
		__g_pRegister->SCALL = NX_BIT_SetBit32(__g_pRegister->SCALL, POWER_OFF_SC, domain  );
		//__g_pRegister->SCALL = NX_BIT_SetBit32(__g_pRegister->SCALL, POWER_OFF_SC, domain  );
		break;
	default: NX_ASSERT (0 && "invalid NX_PMU_POWER_SWITCH value");
	}
}



void NX_PMU_SetPowerUp (  U32 domain, NX_PMU_POWER_SWITCH  powerswitch)
{
	const U32 POWER_ON_nISOLATE = 1;
	const U32 POWER_ON_SC = 0;
	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( domain < NUMBER_OF_PMU_POWER_DOMAIN );

	switch ( powerswitch )
	{
	case NX_PMU_POWER_SWITCH_ISOLATE:
		__g_pRegister->nISOLATE = NX_BIT_SetBit32(__g_pRegister->nISOLATE, POWER_ON_nISOLATE, domain  );
		//__g_pRegister->nISOLATE = NX_BIT_SetBit32(__g_pRegister->nISOLATE, domain, POWER_ON_nISOLATE  );
		break;
	case NX_PMU_POWER_SWITCH_SCPRE  :
		__g_pRegister->SCPRE = NX_BIT_SetBit32(__g_pRegister->SCPRE, POWER_ON_SC,  domain  );
		//__g_pRegister->SCPRE = NX_BIT_SetBit32(__g_pRegister->SCPRE,domain, POWER_ON_SC   );
		break;
	case NX_PMU_POWER_SWITCH_SCALL  :
		__g_pRegister->SCALL = NX_BIT_SetBit32(__g_pRegister->SCALL, POWER_ON_SC, domain  );
		//__g_pRegister->SCALL = NX_BIT_SetBit32(__g_pRegister->SCALL, domain, POWER_ON_SC  );
		break;
	default: NX_ASSERT (0 && "invalid NX_PMU_POWER_SWITCH value");
	}
}


CBOOL NX_PMU_GetPowerDownState ( U32 domain )
{
	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( domain < NUMBER_OF_PMU_POWER_DOMAIN );
	return (CBOOL)NX_BIT_GetBit32( __g_pRegister->SCALLACK, domain ) ;
}



U32 NX_PMU_GetNumberOfPowerDomain ( void )
{
	return NUMBER_OF_PMU_POWER_DOMAIN;
}
