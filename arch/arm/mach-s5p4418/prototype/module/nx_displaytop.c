//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		:
//	File		: nx_DisplayTop.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------


#include "nx_displaytop.h"

//-----------
// DisplayTop은 무조건 1개만 존재해야 한다.
// Display를 더 넣을 생각이라면, 내부를 수정하는 쪽으로 진행한다.!
//-----------
NX_CASSERT( NUMBER_OF_DISPLAYTOP_MODULE == 1 );



//------------------------------------------------------------------------------
//
//	DISPLAYTOP Interface
//
//------------------------------------------------------------------------------
static	struct
{
	struct NX_DISPLAYTOP_RegisterSet *pRegister;
} __g_ModuleVariables = { CNULL, };





//------------------------------------------------------------------------------
//	Module Interface
//------------------------------------------------------------------------------

/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.
 *	@see	NX_DISPLAYTOP_GetNumberOfModule
 */
CBOOL	NX_DISPLAYTOP_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_DISPLAYTOP_MODULE; i++)
		{
			__g_ModuleVariables.pRegister = CNULL;
		}
		bInit = CTRUE;
	}
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_DISPLAYTOP_Initialize
 */
U32		NX_DISPLAYTOP_GetNumberOfModule( void )
{
	return NUMBER_OF_DISPLAYTOP_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 *	@see										NX_DISPLAYTOP_GetSizeOfRegisterSet,
 *				NX_DISPLAYTOP_SetBaseAddress,			NX_DISPLAYTOP_GetBaseAddress,
 *				NX_DISPLAYTOP_OpenModule,				NX_DISPLAYTOP_CloseModule,
 *				NX_DISPLAYTOP_CheckBusy,				NX_DISPLAYTOP_CanPowerDown
 */
U32		NX_DISPLAYTOP_GetPhysicalAddress( void )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( DISPLAYTOP ) }; // PHY_BASEADDR_UART?_MODULE
	NX_CASSERT( NUMBER_OF_DISPLAYTOP_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	return (U32)(PhysicalAddr[0] + PHY_BASEADDR_DISPLAYTOP_MODULE_OFFSET);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_DISPLAYTOP_GetPhysicalAddress,
 *				NX_DISPLAYTOP_SetBaseAddress,			NX_DISPLAYTOP_GetBaseAddress,
 *				NX_DISPLAYTOP_OpenModule,				NX_DISPLAYTOP_CloseModule,
 *				NX_DISPLAYTOP_CheckBusy,				NX_DISPLAYTOP_CanPowerDown
 */
U32		NX_DISPLAYTOP_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DISPLAYTOP_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_DISPLAYTOP_GetPhysicalAddress,		NX_DISPLAYTOP_GetSizeOfRegisterSet,
 *												NX_DISPLAYTOP_GetBaseAddress,
 *				NX_DISPLAYTOP_OpenModule,				NX_DISPLAYTOP_CloseModule,
 *				NX_DISPLAYTOP_CheckBusy,				NX_DISPLAYTOP_CanPowerDown
 */
void	NX_DISPLAYTOP_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	__g_ModuleVariables.pRegister = (struct NX_DISPLAYTOP_RegisterSet *)BaseAddress;

}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_DISPLAYTOP_GetPhysicalAddress,		NX_DISPLAYTOP_GetSizeOfRegisterSet,
 *				NX_DISPLAYTOP_SetBaseAddress,
 *				NX_DISPLAYTOP_OpenModule,				NX_DISPLAYTOP_CloseModule,
 *				NX_DISPLAYTOP_CheckBusy,				NX_DISPLAYTOP_CanPowerDown
 */
void*	NX_DISPLAYTOP_GetBaseAddress( void )
{

	return (void*)__g_ModuleVariables.pRegister;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 *	@see		NX_DISPLAYTOP_GetPhysicalAddress,		NX_DISPLAYTOP_GetSizeOfRegisterSet,
 *				NX_DISPLAYTOP_SetBaseAddress,			NX_DISPLAYTOP_GetBaseAddress,
 *												NX_DISPLAYTOP_CloseModule,
 *				NX_DISPLAYTOP_CheckBusy,				NX_DISPLAYTOP_CanPowerDown
 */
CBOOL	NX_DISPLAYTOP_OpenModule( void )
{

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_DISPLAYTOP_GetPhysicalAddress,		NX_DISPLAYTOP_GetSizeOfRegisterSet,
 *				NX_DISPLAYTOP_SetBaseAddress,			NX_DISPLAYTOP_GetBaseAddress,
 *				NX_DISPLAYTOP_OpenModule,
 *				NX_DISPLAYTOP_CheckBusy,				NX_DISPLAYTOP_CanPowerDown
 */
CBOOL	NX_DISPLAYTOP_CloseModule(  )
{

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@name		NX_DISPLAYTOP_CheckBusy
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_DISPLAYTOP_GetPhysicalAddress,		NX_DISPLAYTOP_GetSizeOfRegisterSet,
 *				NX_DISPLAYTOP_SetBaseAddress,			NX_DISPLAYTOP_GetBaseAddress,
 *				NX_DISPLAYTOP_OpenModule,				NX_DISPLAYTOP_CloseModule,
 *												NX_DISPLAYTOP_CanPowerDown
 *	@see also	Status register, SSPSR
 */
CBOOL	NX_DISPLAYTOP_CheckBusy(  )
{

	return CFALSE;
}

//------------------------------------------------------------------------------
///	@name	Basic MUX SEL Function
//@{
void	NX_DISPLAYTOP_SetRESCONVMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );

	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->RESCONV_MUX_CTRL, (U32)regvalue);
}

void	NX_DISPLAYTOP_SetHDMIMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (bEnb == CTRUE) || (bEnb == CFALSE) );


	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->INTERCONV_MUX_CTRL, (U32)regvalue);
}

void	NX_DISPLAYTOP_SetMIPIMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (bEnb == CTRUE) || (bEnb == CFALSE) );


	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->MIPI_MUX_CTRL, (U32)regvalue);
}

void	NX_DISPLAYTOP_SetLVDSMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (bEnb == CTRUE) || (bEnb == CFALSE) );


	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->LVDS_MUX_CTRL, (U32)regvalue);
}

//---------- RSTCON 을 위한 prototype
U32	NX_DISPLAYTOP_GetResetNumber ( void )
{
	// todo
    const U32 ResetPinNumber[NUMBER_OF_DISPLAYTOP_MODULE] =
    {
        RESETINDEX_LIST( DISPLAYTOP, i_Top_nRST ),
    };
    return (U32)ResetPinNumber[0];
}

// Primary MUX Control ! -
// 0 : Primary MLC, 1 : Primary MPU,
// 2 : Secondary MLC, 3 : ResConv(LCDIF)

void	NX_DISPLAYTOP_SetPrimaryMUX( U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->TFTMPU_MUX, (U32)SEL);
}


//@modified choiyk 2012-12-24 오전 11:03:36
//HDMI Sync Set.
void	NX_DISPLAYTOP_HDMI_SetVSyncStart( U32 SEL ) // from posedge VSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL0, (U32)SEL);
}

void	NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd( U32 Start, U32 End ) // from posedge HSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL3, (U32)(End<<16) | (Start<<0) );
}


void	NX_DISPLAYTOP_HDMI_SetHActiveStart( U32 SEL ) // from posedge HSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL1, (U32)SEL);
}

void	NX_DISPLAYTOP_HDMI_SetHActiveEnd( U32 SEL ) // from posedge HSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL2, (U32)SEL);
}


