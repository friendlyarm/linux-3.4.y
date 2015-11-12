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
//	Module		: MLC
//	File		: nx_mlc.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------

#include "nx_mlc.h"
#include "nx_dualdisplay.h"


static	struct
{
	struct NX_MLC_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_MLC_MODULE] = { {CNULL,}, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicates that Initialize is successed.
 *			 CFALSE	indicates that Initialize is failed.
 *	@see	NX_MLC_GetNumberOfModule
 */
CBOOL	NX_MLC_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_MLC_MODULE; i++ )
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
 *	@see		NX_MLC_Initialize
 */
U32		NX_MLC_GetNumberOfModule( void )
{
	return NUMBER_OF_MLC_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		Module's physical address
 */
U32		NX_MLC_GetPhysicalAddress( U32 ModuleIndex )
{
	const U32 PhysicalAddr[] =
	{
		PHY_BASEADDR_LIST( MLC )
	};
	NX_CASSERT( NUMBER_OF_MLC_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_MLC_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_MLC_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_MLC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != BaseAddress );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_MLC_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		Module's base address.
 */
void*	NX_MLC_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that Initialize is successed. 
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_MLC_OpenModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that Deinitialize is successed. 
 *				 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_MLC_CloseModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that Module is Busy. 
 *				 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_MLC_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that Ready to enter power-down stage. 
 *				 CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_MLC_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
// Clock Control Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Set a PCLK mode
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	mode	PCLK mode
 *	@return		None.
 */
void			NX_MLC_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode )
{
	const U32 PCLKMODE_POS	=	3;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	U32 clkmode=0;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	switch(mode)
	{
		case NX_PCLKMODE_DYNAMIC:	clkmode = 0;		break;
		case NX_PCLKMODE_ALWAYS:	clkmode = 1;		break;
		default: NX_ASSERT( CFALSE );
	}

	regvalue = pRegister->MLCCLKENB;

	regvalue &= ~(1UL<<PCLKMODE_POS);
	regvalue |= ( clkmode & 0x01 ) << PCLKMODE_POS;

//	pRegister->MLCCLKENB = regvalue;
	WriteIO32(&pRegister->MLCCLKENB,regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current PCLK mode
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		Current PCLK mode
 *	@see		NX_MLC_SetClockPClkMode,
 *				NX_MLC_SetClockBClkMode,		NX_MLC_GetClockBClkMode,
 */
NX_PCLKMODE	NX_MLC_GetClockPClkMode( U32 ModuleIndex )
{
	const U32 PCLKMODE_POS	= 3;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->MLCCLKENB & ( 1UL << PCLKMODE_POS ) )
	{
		return NX_PCLKMODE_ALWAYS;
	}

	return	NX_PCLKMODE_DYNAMIC;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set System Bus Clock's operation Mode
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	mode		BCLK Mode
 *	@return		None.
 */
void			NX_MLC_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode )
{
	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;
	U32 clkmode=0;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	switch(mode)
	{
	case NX_BCLKMODE_DISABLE:	clkmode = 0;		break;
	case NX_BCLKMODE_DYNAMIC:	clkmode = 2;		break;
	case NX_BCLKMODE_ALWAYS:	clkmode = 3;		break;
	default: NX_ASSERT( CFALSE );
	}

	regvalue = pRegister->MLCCLKENB;

	regvalue &= ~(0x3);
	regvalue |= clkmode & 0x3;

//	pRegister->MLCCLKENB = regvalue;
	WriteIO32(&pRegister->MLCCLKENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get System Bus Clock's operation Mode
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		BCLK Mode
 */
NX_BCLKMODE	NX_MLC_GetClockBClkMode( U32 ModuleIndex )
{
	const U32 BCLKMODE	= 3UL<<0;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	switch( __g_ModuleVariables[ModuleIndex].pRegister->MLCCLKENB & BCLKMODE )
	{
	case 0 :	return NX_BCLKMODE_DISABLE;
	case 2 :	return NX_BCLKMODE_DYNAMIC;
	case 3 :	return NX_BCLKMODE_ALWAYS;
	}

	NX_ASSERT( CFALSE );
	return NX_BCLKMODE_DISABLE;
}

//--------------------------------------------------------------------------
// MLC Main Settings
//--------------------------------------------------------------------------
/**
 *	@brief		Set Power On/Off of MLC's Pixel Buffer Unit
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param		bPower	 CTRUE indicates that Power ON of pixel buffer unit. 
 *						 CFALSE indicates that Power OFF of pixel buffer unit. 
 *	@return		None.
 *	@remark		When MLC ON, first pixel buffer power ON, set to Normal Mode(pixel buffer) and MLC enable.
 *				When MLC Off, first MLC disable, Set pixel buffer to Sleep Mode(pixel buffer) and power OFF.
 *	@code
 *				// MLC ON sequence, mi = 0 ( module index )
 *				NX_MLC_SetTopPowerMode( mi, CTRUE );			// pixel buffer power on
 *				NX_MLC_SetTopSleepMode( mi, CFALSE );			// pixel buffer normal mode
 *				NX_MLC_SetMLCEnable( mi, CTRUE );				// mlc enable
 *				NX_MLC_SetTopDirtyFlag( mi );					// apply setting value
 *				...
 *				// MLC OFF sequence
 *				NX_MLC_SetMLCEnable( mi, CFALSE );				// mlc disable
 *				NX_MLC_SetTopDirtyFlag( mi );					// apply setting value
 *				while( CTRUE == NX_MLC_GetTopDirtyFlag(mi, ))	// wait until mlc is disabled
 *				{ CNULL; }
 *				NX_MLC_SetTopSleepMode( mi, CTRUE );			// pixel buffer sleep mode
 *				NX_MLC_SetTopPowerMode( mi, CFALSE );			// pixel buffer power off
 *	@endcode
 */
void	NX_MLC_SetTopPowerMode( U32 ModuleIndex, CBOOL bPower )
{
	const U32 PIXELBUFFER_PWD_POS	= 11;
	const U32 PIXELBUFFER_PWD_MASK	= 1UL << PIXELBUFFER_PWD_POS;
	const U32 DITTYFLAG_MASK		= 1UL << 3;

	register struct NX_MLC_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bPower) || (1==bPower) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCCONTROLT;

	regvalue &= ~( PIXELBUFFER_PWD_MASK | DITTYFLAG_MASK );
	regvalue |= (bPower << PIXELBUFFER_PWD_POS);

//	pRegister->MLCCONTROLT = regvalue;
	WriteIO32(&pRegister->MLCCONTROLT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get power state of MLC's pixel buffer unit.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that pixel buffer is power ON.
 *				 CFALSE	indicates that pixel buffer is power OFF.
 */
CBOOL	NX_MLC_GetTopPowerMode( U32 ModuleIndex )
{
	const U32 PIXELBUFFER_PWD_POS	= 11;
	const U32 PIXELBUFFER_PWD_MASK	= 1UL << PIXELBUFFER_PWD_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCCONTROLT & PIXELBUFFER_PWD_MASK) >> PIXELBUFFER_PWD_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Sleep Mode Enable/Disalbe of MLC's Pixel Buffer Unit
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param		bSleep	 CTRUE indicates that Sleep Mode Enable of pixel buffer unit. 
 *						 CFALSE indicates that Sleep Mode Disable of pixel buffer unit. 
 *	@return		None.
 *	@remark		When MLC ON, first pixel buffer power ON, set to Normal Mode(pixel buffer) and MLC enable.
 *				When MLC Off, first MLC disable, Set pixel buffer to Sleep Mode(pixel buffer) and power OFF.
 *	@code
 *				// MLC ON sequence,	mi = 0  ( module index )
 *				NX_MLC_SetTopPowerMode( mi, CTRUE );			// pixel buffer power on
 *				NX_MLC_SetTopSleepMode( mi, CFALSE );			// pixel buffer normal mode
 *				NX_MLC_SetMLCEnable( mi, CTRUE );				// mlc enable
 *				NX_MLC_SetTopDirtyFlag( mi );					// apply setting value
 *				...
 *				// MLC OFF sequence
 *				NX_MLC_SetMLCEnable( mi, CFALSE );				// mlc disable
 *				NX_MLC_SetTopDirtyFlag( mi );					// apply setting value
 *				while( CTRUE == NX_MLC_GetTopDirtyFlag( mi ))	// wait until mlc is disabled
 *				{ CNULL; }
 *				NX_MLC_SetTopSleepMode( mi, CTRUE );			// pixel buffer sleep mode
 *				NX_MLC_SetTopPowerMode( mi, CFALSE );			// pixel buffer power off
 *	@endcode
 */
void	NX_MLC_SetTopSleepMode( U32 ModuleIndex, CBOOL bSleep )
{
	const U32 PIXELBUFFER_SLD_POS	= 10;
	const U32 PIXELBUFFER_SLD_MASK	= 1UL << PIXELBUFFER_SLD_POS;
	const U32 DITTYFLAG_MASK		= 1UL << 3;

	register struct NX_MLC_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bSleep) || (1==bSleep) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	bSleep = (CBOOL)((U32)bSleep ^ 1);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCCONTROLT;

	regvalue &= ~( PIXELBUFFER_SLD_MASK | DITTYFLAG_MASK );
	regvalue |= (bSleep << PIXELBUFFER_SLD_POS);

//	pRegister->MLCCONTROLT = regvalue;
	WriteIO32(&pRegister->MLCCONTROLT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Sleep Mode state of MLC's pixel buffer unit.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that pixel buffer is Sleep Mode.
 *				 CFALSE	indicates that pixel buffer is Normal Mode.
 */
CBOOL	NX_MLC_GetTopSleepMode( U32 ModuleIndex )
{
	const U32 PIXELBUFFER_SLD_POS	= 11;
	const U32 PIXELBUFFER_SLD_MASK	= 1UL << PIXELBUFFER_SLD_POS;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(((__g_ModuleVariables[ModuleIndex].pRegister->MLCCONTROLT & PIXELBUFFER_SLD_MASK) >> PIXELBUFFER_SLD_POS) ^ 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Apply modified MLC Top registers to MLC.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 None.
 *	@remark		MLC has dual register set architecture. Therefore you have to set a
 *				dirty flag to apply modified settings to MLC's current settings.
 *				If a dirty flag is set, MLC will update current settings to
 *				register values on a vertical blank. You can also check whether MLC
 *				has been updated by using function NX_MLC_GetTopDirtyFlag().
 */
void	NX_MLC_SetTopDirtyFlag( U32 ModuleIndex )
{
	const U32 DIRTYFLAG = 1UL<<3;

	register struct NX_MLC_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue  =	pRegister->MLCCONTROLT;
	regvalue |= DIRTYFLAG;

//	pRegister->MLCCONTROLT = regvalue;
	WriteIO32(&pRegister->MLCCONTROLT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether modified settings is applied to MLC or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates MLC does not update to modified settings yet.
 *				 CFALSE	indicates MLC has already been updated to modified settings.

 */
CBOOL	NX_MLC_GetTopDirtyFlag( U32 ModuleIndex )
{
	const U32 DIRTYFLAG_POS		= 3;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(
	(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MLCCONTROLT) & DIRTYFLAG_MASK) >> DIRTYFLAG_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable MLC.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bEnb			Set it to CTRUE to enable MLC.
 *	@return		None.
 *	@remark		The result of this function will be applied	to MLC after calling
 *				function NX_MLC_SetTopDirtyFlag().
 */
void	NX_MLC_SetMLCEnable( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 MLCENB_POS	= 1;
	const U32 MLCENB_MASK	= 1UL<<MLCENB_POS;

	const U32 DIRTYFLAG_POS		= 3;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCCONTROLT;

	regvalue &= ~( MLCENB_MASK | DIRTYFLAG_MASK );
	regvalue |= (bEnb<<MLCENB_POS);

//	pRegister->MLCCONTROLT = regvalue;
	WriteIO32(&pRegister->MLCCONTROLT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether MLC is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates MLC is enabled.
 *				 CFALSE	indicates MLC is disabled.
 */
CBOOL	NX_MLC_GetMLCEnable( U32 ModuleIndex )
{
	const U32 MLCENB_POS	= 1;
	const U32 MLCENB_MASK	= 1UL<<MLCENB_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCCONTROLT & MLCENB_MASK) >> MLCENB_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable interlace mode.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bEnb			Set it to CTRUE to enable interlace mode.
 *	@return		None.
 *	@remark		The result of this function will be applied	to MLC after calling
 *				function NX_MLC_SetTopDirtyFlag().
 */
void	NX_MLC_SetFieldEnable( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 FIELDENB_POS	= 0;
	const U32 FIELDENB_MASK = 1UL<<FIELDENB_POS;

	const U32 DIRTYFLAG_POS		= 3;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCCONTROLT;

	regvalue &= ~( FIELDENB_MASK | DIRTYFLAG_MASK );
	regvalue |= (bEnb<<FIELDENB_POS);

//	pRegister->MLCCONTROLT = regvalue;
	WriteIO32(&pRegister->MLCCONTROLT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether MLC is interlace mode or progressive mode.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates MLC is interlace mode.
 *				 CFALSE	indicates MLC is progressive mode.
 */
CBOOL	NX_MLC_GetFieldEnable( U32 ModuleIndex )
{
	const U32 FIELDENB_POS	= 0;
	const U32 FIELDENB_MASK = 1UL<<FIELDENB_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(__g_ModuleVariables[ModuleIndex].pRegister->MLCCONTROLT & FIELDENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the priority of layers.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	priority		the priority of layers
 *	@return		None.
 *	@remark		The result of this function will be applied	to MLC after calling
 *				function NX_MLC_SetTopDirtyFlag().
 */
void	NX_MLC_SetLayerPriority( U32 ModuleIndex, NX_MLC_PRIORITY priority )
{
	const U32 PRIORITY_POS	= 8;
	const U32 PRIORITY_MASK	= 0x03 << PRIORITY_POS;

	const U32 DIRTYFLAG_POS		= 3;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	register struct NX_MLC_RegisterSet* pRegister;
	register U32    regvalue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (NX_MLC_PRIORITY_VIDEOFIRST == priority) || (NX_MLC_PRIORITY_VIDEOSECOND == priority) ||
				(NX_MLC_PRIORITY_VIDEOTHIRD == priority) || (NX_MLC_PRIORITY_VIDEOFOURTH == priority) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCCONTROLT;

	regvalue &= ~( PRIORITY_MASK | DIRTYFLAG_MASK );
	regvalue |= (priority<<PRIORITY_POS);

//	pRegister->MLCCONTROLT = regvalue;
	WriteIO32(&pRegister->MLCCONTROLT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the screen size.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	width		the screen width, 1 ~ 4096.
 *	@param[in]	height		the screen height, 1 ~ 4096.
 *	@return		None.
 *	@remark		The result of this function will be applied	to MLC after calling
 *				function NX_MLC_SetTopDirtyFlag().
 */
void	NX_MLC_SetScreenSize( U32 ModuleIndex, U32 width, U32 height )
{
	register struct NX_MLC_RegisterSet* pRegister;
	register U32    regvalue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4096 > (width - 1) );
	NX_ASSERT( 4096 > (height - 1) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue  = ((height-1)<<16) | (width-1);

//	pRegister->MLCSCREENSIZE = regvalue;
	WriteIO32(&pRegister->MLCSCREENSIZE, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the screen size.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[out] pWidth		the screen width, 1 ~ 4096.
 *	@param[out] pHeight		the screen height, 1 ~ 4096.
 *	@return		None.
 */
void	NX_MLC_GetScreenSize( U32 ModuleIndex, U32 *pWidth, U32 *pHeight )
{
	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( CNULL != pWidth )
	{
		*pWidth = (__g_ModuleVariables[ModuleIndex].pRegister->MLCSCREENSIZE & 0x0FFF) + 1;
	}

	if( CNULL != pHeight )
	{
		*pHeight = ((__g_ModuleVariables[ModuleIndex].pRegister->MLCSCREENSIZE>>16) & 0x0FFF) + 1;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the background color.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	color	24 bit RGB format, 0xXXRRGGBB = { R[7:0], G[7:0], B[7:0] }
 *	@return		None.
 *	@remark		The background color is default color that is shown in regions which
 *				any layer does not include. the result of this function will be
 *				applied to MLC after calling function NX_MLC_SetTopDirtyFlag().
 */
void	NX_MLC_SetBackground( U32 ModuleIndex, U32 color )
{
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCBGCOLOR = color;
	WriteIO32(&pRegister->MLCBGCOLOR, color);
}

//--------------------------------------------------------------------------
//	Per Layer Operations
//------------------------------------------------------------------------------
/**
 *	@brief		Apply modified register values to corresponding layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1, 3: Video ).
 *	@return		None.
 *	@remark		Each layer has dual register set architecture. Therefore you have to
 *				set a dirty flag to apply modified settings to each layer's current
 *				settings. If a dirty flag is set, each layer will update current
 *				settings to register values on a vertical blank. You can also check
 *				whether each layer has been updated by using function NX_MLC_GetDirtyFlag().
 */
void	NX_MLC_SetDirtyFlag( U32 ModuleIndex, U32 layer )
{
	register struct NX_MLC_RegisterSet* pRegister;
	register U32    regvalue;

	const U32 DIRTYFLG_MASK = 1UL << 4;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4 > layer );
	NX_ASSERT( 2 != layer );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if ( 0 == layer || 1 == layer )
	{
		regvalue  = pRegister->MLCRGBLAYER[layer].MLCCONTROL;
		regvalue |= DIRTYFLG_MASK;

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);
	}
	else if( 3 == layer )
	{
		regvalue  = pRegister->MLCVIDEOLAYER.MLCCONTROL;
		regvalue |= DIRTYFLG_MASK;

	//	pRegister->MLCVIDEOLAYER.MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether modified settings is applied to corresponding layer or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1, 3: Video ).
 *	@return		 CTRUE	indicates corresponding layer does not update to modified settings yet.
 *				 CFALSE	indicates corresponding layer has already been updated to modified settings.
 */
CBOOL	NX_MLC_GetDirtyFlag( U32 ModuleIndex, U32 layer )
{
	const U32 DIRTYFLG_POS	= 4;
	const U32 DIRTYFLG_MASK	= 1UL << DIRTYFLG_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4 > layer );
	//NX_ASSERT( 2 != layer );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if ( 0 == layer || 1 == layer )
	{
		return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCRGBLAYER[layer].MLCCONTROL & DIRTYFLG_MASK ) >> DIRTYFLG_POS);
	}
	else if( 2 == layer )
	{
		return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCRGBLAYER2.MLCCONTROL & DIRTYFLG_MASK ) >> DIRTYFLG_POS);
	}
	else if( 3 == layer )
	{
		return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCVIDEOLAYER.MLCCONTROL & DIRTYFLG_MASK ) >> DIRTYFLG_POS);
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable the layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer			the layer number ( 0: RGB0, 1: RGB1, 3: Video ).
 *	@param[in]	bEnb			Set it to CTRUE to enable corresponding layer
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetLayerEnable( U32 ModuleIndex, U32 layer, CBOOL bEnb )
{
	const U32 LAYERENB_POS	= 5;
	const U32 LAYERENB_MASK	= 0x01 << LAYERENB_POS;

	const U32 DIRTYFLAG_POS		= 4;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4 > layer );
	NX_ASSERT( 2 != layer );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
		regvalue = pRegister->MLCRGBLAYER[layer].MLCCONTROL;

		regvalue &= ~( LAYERENB_MASK | DIRTYFLAG_MASK );
		regvalue |= (bEnb<<LAYERENB_POS);

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);
	}
	else if( 3 == layer )
	{
		regvalue = pRegister->MLCVIDEOLAYER.MLCCONTROL;

		regvalue &= ~( LAYERENB_MASK | DIRTYFLAG_MASK );
		regvalue |= (bEnb<<LAYERENB_POS);

	//	pRegister->MLCVIDEOLAYER.MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Determines whether the layer is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1, 3: Video ).
 *	@return		 CTRUE	indicates the layer is enabled.
 *				 CFALSE	indicates the layer is disabled.
 */
CBOOL	NX_MLC_GetLayerEnable( U32 ModuleIndex, U32 layer )
{
	const U32 LAYERENB_POS = 5;
	const U32 LAYERENB_MASK = 0x01 << LAYERENB_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4 > layer );
	NX_ASSERT( 2 != layer );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( 0 == layer || 1 == layer )
	{
		return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCRGBLAYER[layer].MLCCONTROL & LAYERENB_MASK ) >> LAYERENB_POS);
	}
	else if( 3 == layer )
	{
		return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCVIDEOLAYER.MLCCONTROL & LAYERENB_MASK ) >> LAYERENB_POS);
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the lock size for memory access.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer			the layer number ( 0: RGB0, 1: RGB1 ).
 *	@param[in]	locksize		lock size for memory access, 4, 8, 16 are only valid.
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetLockSize( U32 ModuleIndex, U32 layer, U32 locksize )
{
	const U32 LOCKSIZE_MASK		= 3UL<<12;

	const U32 DIRTYFLAG_POS		= 4;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	register struct NX_MLC_RegisterSet* pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > layer );
	NX_ASSERT( 4 == locksize || 8 == locksize || 16 == locksize );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	locksize >>= 3;	// divide by 8

	if( 0 == layer || 1 == layer )
	{
		regvalue = pRegister->MLCRGBLAYER[layer].MLCCONTROL;

		regvalue &= ~(LOCKSIZE_MASK|DIRTYFLAG_MASK);
		regvalue |= (locksize<<12);

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Alpha blending.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number( 0: RGB0, 1: RGB1, 3: Video ).
 *	@param[in]	bEnb	Set it to CTRUE to enable alpha blending.
 *	@param[in]	alpha	alpha blending factor, 0 ~ 15.
 *				- When it is set to 0, this layer color is fully transparent.
 *				- When it is set to 15, this layer becomes fully opaque.
 *	@return		None.
 *	@remark		The argument 'alpha' has only affect when the color format has
 *				no alpha component. The formula for alpha blending is as follows.
 *				- If alpha is 0 then a is 0, else a is ALPHA + 1.
 *					color = this layer color * a / 16 + lower layer color * (16 - a) / 16.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 *				Only one layer must apply to alpha function.
 *
 */
void	NX_MLC_SetAlphaBlending( U32 ModuleIndex, U32 layer, CBOOL bEnb, U32 alpha )
{
	const U32 BLENDENB_POS	= 2;
	const U32 BLENDENB_MASK	= 0x01 << BLENDENB_POS;

	const U32 DIRTYFLAG_POS		= 4;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	const U32 ALPHA_POS		= 28;
	const U32 ALPHA_MASK	= 0xF << ALPHA_POS;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4 > layer );
	NX_ASSERT( 2 != layer );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( 16 > alpha );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
		regvalue = pRegister->MLCRGBLAYER[layer].MLCCONTROL;
			regvalue &= ~( BLENDENB_MASK | DIRTYFLAG_MASK );
		regvalue |= (bEnb<<BLENDENB_POS);

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);

		regvalue = pRegister->MLCRGBLAYER[layer].MLCTPCOLOR;
		regvalue &= ~ALPHA_MASK;
		regvalue |= alpha << ALPHA_POS;

	//	pRegister->MLCRGBLAYER[layer].MLCTPCOLOR = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCTPCOLOR, regvalue);
	}
	else if( 3 == layer )
	{
		regvalue = pRegister->MLCVIDEOLAYER.MLCCONTROL;
		regvalue &= ~( BLENDENB_MASK | DIRTYFLAG_MASK );
		regvalue |= (bEnb<<BLENDENB_POS);

	//	pRegister->MLCVIDEOLAYER.MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, regvalue);

	//	pRegister->MLCVIDEOLAYER.MLCTPCOLOR = alpha << ALPHA_POS;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCTPCOLOR, alpha << ALPHA_POS);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set transparency.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1 ).
 *	@param[in]	bEnb	Set it to CTRUE to enable transparency.
 *	@param[in]	color	Specifies the extended color to be used as the transparency color.
 *						24 bit RGB format, 0xXXRRGGBB = { R[7:0], G[7:0], B[7:0] }
 *						You can get this argument from specific color format
 *						by using the function NX_MLC_GetExtendedColor().
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetTransparency( U32 ModuleIndex, U32 layer, CBOOL bEnb, U32 color )
{
	const U32 TPENB_POS		= 0;
	const U32 TPENB_MASK	= 0x01 << TPENB_POS;

	const U32 DIRTYFLAG_POS		= 4;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	const U32 TPCOLOR_POS	= 0;
	const U32 TPCOLOR_MASK	= ((1<<24)-1) << TPCOLOR_POS;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > layer );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( (1<<24) > color );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
		regvalue = pRegister->MLCRGBLAYER[layer].MLCCONTROL;
			regvalue &= ~( TPENB_MASK | DIRTYFLAG_MASK );
		regvalue |= (bEnb<<TPENB_POS);

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);

		regvalue = pRegister->MLCRGBLAYER[layer].MLCTPCOLOR;
		regvalue &= ~TPCOLOR_MASK;
		regvalue |= (color & TPCOLOR_MASK);

	//	pRegister->MLCRGBLAYER[layer].MLCTPCOLOR = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCTPCOLOR, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set color inversion.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1 ).
 *	@param[in]	bEnb	Set it to CTRUE to enable color inversion.
 *	@param[in]	color	Specifies the extended color to be used for color inversion.
 *						24 bit RGB format, 0xXXRRGGBB = { R[7:0], G[7:0], B[7:0] }
 *						You can get this argument from specific color format
 *						by using the function NX_MLC_GetExtendedColor().
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetColorInversion( U32 ModuleIndex, U32 layer, CBOOL bEnb, U32 color )
{
	const U32 INVENB_POS	= 1;
	const U32 INVENB_MASK	= 0x01 << INVENB_POS;

	const U32 DIRTYFLAG_POS		= 4;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	const U32 INVCOLOR_POS	= 0;
	const U32 INVCOLOR_MASK	= ((1<<24)-1) << INVCOLOR_POS;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > layer );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( (1<<24) > color );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
		regvalue = pRegister->MLCRGBLAYER[layer].MLCCONTROL;
		regvalue &= ~( INVENB_MASK | DIRTYFLAG_MASK );
		regvalue |= (bEnb<<INVENB_POS);

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);

		regvalue = pRegister->MLCRGBLAYER[layer].MLCINVCOLOR;
		regvalue &= ~INVCOLOR_MASK;
		regvalue |= (color & INVCOLOR_MASK);

	//	pRegister->MLCRGBLAYER[layer].MLCINVCOLOR = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCINVCOLOR, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get extended color.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	color	Specifies the color value to be converted.
 *	@param[in]	format	the color format with RGBFMT type.
 *	@return		the color which has 24 bit RGB format
 *				0xXXRRGGBB = { R[7:0], G[7:0], B[7:0] }
 *	@remark		This function is for argument 'color' of the function
 *				NX_MLC_SetTransparency() and NX_MLC_SetColorInversion().
 *				For example,
 *	@code
 *		// mi = 0 ~ 1 ( module index )
 *		NX_MLC_SetTransparency	( mi, layer, CTRUE, NX_MLC_GetExtendedColor( 0x0841, RGBFMT_R5G6B5 ) );
 *		NX_MLC_SetColorInversion( mi, layer, CTRUE, NX_MLC_GetExtendedColor( 0x090909, RGBFMT_R8G8B8 ) );
 *	@endcode
 */
U32		NX_MLC_GetExtendedColor( U32 ModuleIndex, U32 color, NX_MLC_RGBFMT format )
{
	U32 rgb[3];
	int	bw[3], bp[3], blank, fill, i;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 0 == (format & 0x0000FFFFUL) );
	switch( format )
	{
	case NX_MLC_RGBFMT_R5G6B5	:	// 16bpp { R5, G6, B5 }.
		bw[0] =	5;		bw[1] =	6;		bw[2] =	5;
		bp[0] = 11;	bp[1] =	5;	bp[2] =	0;
		break;
	case NX_MLC_RGBFMT_B5G6R5	:	// 16bpp { B5, G6, R5 }.
		bw[0] =	5;		bw[1] =	6;		bw[2] =	5;
		bp[0] =	0;	bp[1] =	5;	bp[2] = 11;
		break;
	case NX_MLC_RGBFMT_X1R5G5B5 :	// 16bpp { X1, R5, G5, B5 }.
	case NX_MLC_RGBFMT_A1R5G5B5 :	// 16bpp { A1, R5, G5, B5 }.
		bw[0] =	5;		bw[1] =	5;		bw[2] =	5;
		bp[0] = 10;	bp[1] =	5;	bp[2] =	0;
		break;
	case NX_MLC_RGBFMT_X1B5G5R5 :	// 16bpp { X1, B5, G5, R5 }.
	case NX_MLC_RGBFMT_A1B5G5R5 :	// 16bpp { A1, B5, G5, R5 }.
		bw[0] =	5;		bw[1] =	5;		bw[2] =	5;
		bp[0] =	0;	bp[1] =	5;	bp[2] = 10;
		break;
	case NX_MLC_RGBFMT_X4R4G4B4 :	// 16bpp { X4, R4, G4, B4 }.
	case NX_MLC_RGBFMT_A4R4G4B4 :	// 16bpp { A4, R4, G4, B4 }.
		bw[0] =	4;		bw[1] =	4;		bw[2] =	4;
		bp[0] =	8;	bp[1] =	4;	bp[2] =	0;
		break;
	case NX_MLC_RGBFMT_X4B4G4R4 :	// 16bpp { X4, B4, G4, R4 }.
	case NX_MLC_RGBFMT_A4B4G4R4 :	// 16bpp { A4, B4, G4, R4 }.
		bw[0] =	4;		bw[1] =	4;		bw[2] =	4;
		bp[0] =	0;	bp[1] =	4;	bp[2] =	8;
		break;
	case NX_MLC_RGBFMT_X8R3G3B2 :	// 16bpp { X8, R3, G3, B2 }.
	case NX_MLC_RGBFMT_A8R3G3B2 :	// 16bpp { A8, R3, G3, B2 }.
		bw[0] =	3;		bw[1] =	3;		bw[2] =	2;
		bp[0] =	5;	bp[1] =	2;	bp[2] =	0;
		break;
	case NX_MLC_RGBFMT_X8B3G3R2 :	// 16bpp { X8, B3, G3, R2 }.
	case NX_MLC_RGBFMT_A8B3G3R2 :	// 16bpp { A8, B3, G3, R2 }.
		bw[0] =	2;		bw[1] =	3;		bw[2] =	3;
		bp[0] =	0;	bp[1] =	2;	bp[2] =	5;
		break;
	case NX_MLC_RGBFMT_R8G8B8	:	// 24bpp { R8, G8, B8 }.
//	case NX_MLC_RGBFMT_X8R8G8B8 :	// 32bpp { X8, R8, G8, B8 }.
	case NX_MLC_RGBFMT_A8R8G8B8 :	// 32bpp { A8, R8, G8, B8 }.
		bw[0] =	8;		bw[1] =	8;		bw[2] =	8;
		bp[0] = 16;	bp[1] =	8;	bp[2] =	0;
		break;
	case NX_MLC_RGBFMT_B8G8R8	:	// 24bpp { B8, G8, R8 }.
//	case NX_MLC_RGBFMT_X8B8G8R8 :	// 32bpp { X8, B8, G8, R8 }.
	case NX_MLC_RGBFMT_A8B8G8R8 :	// 32bpp { A8, B8, G8, R8 }.
		bw[0] =	8;		bw[1] =	8;		bw[2] =	8;
		bp[0] =	0;	bp[1] =	8;	bp[2] = 16;
		break;
	default : NX_ASSERT( CFALSE );
	}

	for( i=0 ; i<3 ; i++ )
	{
		rgb[i] = (color >> bp[i]) & ((U32)(1<<bw[i])-1);

		fill	= bw[i];
		blank	= 8-fill;
		rgb[i] <<= blank;
		while( blank > 0 )
		{
			rgb[i] |= (rgb[i] >> fill);
			blank	-= fill;
			fill	+= fill;
		}
	}

	return (rgb[0]<<16) | (rgb[1]<<8) | (rgb[2]<<0);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the image format of the RGB layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1 ).
 *	@param[in]	format	the RGB format
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetFormatRGB( U32 ModuleIndex, U32 layer, NX_MLC_RGBFMT format )
{
	const U32 DIRTYFLAG_POS		= 4;
	const U32 DIRTYFLAG_MASK	= 1UL<<DIRTYFLAG_POS;

	const U32 FORMAT_MASK		= 0xFFFF0000UL;
	register U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > layer );
	NX_ASSERT( 0 == (format & 0x0000FFFFUL) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
		regvalue = pRegister->MLCRGBLAYER[layer].MLCCONTROL;

		regvalue &= ~(FORMAT_MASK | DIRTYFLAG_MASK);
		regvalue |= (U32)format;

	//	pRegister->MLCRGBLAYER[layer].MLCCONTROL = regvalue;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCCONTROL, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the image format of video layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	format			The YUV format
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetFormatYUV( U32 ModuleIndex, NX_MLC_YUVFMT format )
{
	const U32 FORMAT_MASK	= 0xFFFF0000UL;
	register U32 temp;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 0 == (format & 0xFFF8FFFFUL) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	temp = pRegister->MLCVIDEOLAYER.MLCCONTROL;
	temp &= ~FORMAT_MASK;
	temp |= (U32)format;

//	pRegister->MLCVIDEOLAYER.MLCCONTROL	= temp;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the layer position.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1, 3: Video ).
 *	@param[in]	sx		the x-coordinate of the upper-left corner of the layer, -2048 ~ +2047.
 *	@param[in]	sy		the y-coordinate of the upper-left corner of the layer, -2048 ~ +2047.
 *	@param[in]	ex		the x-coordinate of the lower-right corner of the layer, -2048 or 0 ~ +2047.
 *	@param[in]	ey		the y-coordinate of the lower-right corner of the layer, -2048 or 0 ~ +2047.
 *	@return		None.
 *	@remark		If layer is 2(video layer) then x, y-coordinate of the lower-right
 *				corner of the layer must be a positive value.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetPosition( U32 ModuleIndex, U32 layer, S32 sx, S32 sy, S32 ex, S32 ey )
{
	register struct NX_MLC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 4 > layer );
	NX_ASSERT( -2048 <= sx && sx < 2048 );
	NX_ASSERT( -2048 <= sy && sy < 2048 );
	NX_ASSERT( -2048 <= ex && ex < 2048 );
	NX_ASSERT( -2048 <= ey && ey < 2048 );
	NX_ASSERT( (3!=layer) || ((ex>=0) && (ey>=0)) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
	//	pRegister->MLCRGBLAYER[layer].MLCLEFTRIGHT	= (((U32)sx & 0xFFFUL)<<16) | ((U32)ex & 0xFFFUL);
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCLEFTRIGHT, (((U32)sx & 0xFFFUL)<<16) | ((U32)ex & 0xFFFUL));

	//	pRegister->MLCRGBLAYER[layer].MLCTOPBOTTOM =	(((U32)sy & 0xFFFUL)<<16) | ((U32)ey & 0xFFFUL);
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCTOPBOTTOM,	(((U32)sy & 0xFFFUL)<<16) | ((U32)ey & 0xFFFUL));
	}
	else if( 2 == layer )
	{
//	pRegister->MLCRGBLAYER[layer].MLCLEFTRIGHT	= (((U32)sx & 0xFFFUL)<<16) | ((U32)ex & 0xFFFUL);
		WriteIO32(&pRegister->MLCRGBLAYER2.MLCLEFTRIGHT, (((U32)sx & 0xFFFUL)<<16) | ((U32)ex & 0xFFFUL));

	//	pRegister->MLCRGBLAYER[layer].MLCTOPBOTTOM =	(((U32)sy & 0xFFFUL)<<16) | ((U32)ey & 0xFFFUL);
		WriteIO32(&pRegister->MLCRGBLAYER2.MLCTOPBOTTOM,	(((U32)sy & 0xFFFUL)<<16) | ((U32)ey & 0xFFFUL));
	}
	else if( 3 == layer )
	{
	//	pRegister->MLCVIDEOLAYER.MLCLEFTRIGHT = (((U32)sx & 0xFFFUL)<<16) | ((U32)ex & 0xFFFUL);
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCLEFTRIGHT, (((U32)sx & 0xFFFUL)<<16) | ((U32)ex & 0xFFFUL));

	//	pRegister->MLCVIDEOLAYER.MLCTOPBOTTOM = (((U32)sy & 0xFFFUL)<<16) | ((U32)ey & 0xFFFUL);
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCTOPBOTTOM, (((U32)sy & 0xFFFUL)<<16) | ((U32)ey & 0xFFFUL));
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set dither enable when using gamma.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bEnable			 CTRUE indicates that dither enable. 
 *								 CFALSE indicates that dither disable.
 *	@return		None.
 */
void	NX_MLC_SetDitherEnableWhenUsingGamma( U32 ModuleIndex, CBOOL bEnable )
{
	const U32 DITHERENB_BITPOS	= 0;
	const U32 DITHERENB_MASK	= 1 << DITHERENB_BITPOS;

	register struct NX_MLC_RegisterSet*	pRegister;
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	ReadValue &= ~DITHERENB_MASK;
	ReadValue |= ( (U32)bEnable << DITHERENB_BITPOS );

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of dither is enabled or Not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that dither is enabled when using gamma. 
 *				 CFALSE	indicates that dither is disabled when using gamma.
 */
CBOOL	NX_MLC_GetDitherEnableWhenUsingGamma( U32 ModuleIndex )
{
	const U32 DITHERENB_BITPOS	= 0;
	const U32 DITHERENB_MASK	= 1 << DITHERENB_BITPOS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)( __g_ModuleVariables[ModuleIndex].pRegister->MLCGAMMACONT & DITHERENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Select layer to apply gamma when video layer and RGB layer are overlaped region for alpha blending.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bVideoLayer		 CTRUE indicates that Video layer. 
 *								 CFALSE indicates that RGB layer.
 *	@return		none.
 */
void	NX_MLC_SetGammaPriority( U32 ModuleIndex, CBOOL bVideoLayer )
{
	const U32 ALPHASELECT_BITPOS	= 5;
	const U32 ALPHASELECT_MASK		= 1 << ALPHASELECT_BITPOS;

	register struct NX_MLC_RegisterSet*	pRegister;
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bVideoLayer) || (1==bVideoLayer) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	ReadValue &= ~ALPHASELECT_MASK;
	ReadValue |= ( (U32)bVideoLayer << ALPHASELECT_BITPOS );

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of which region to apply gamma.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that Video layer is selected., 
 *				 CFALSE	indicates that RGB layer is selected.
 */
CBOOL	NX_MLC_GetGammaPriority( U32 ModuleIndex )
{
	const U32 ALPHASELECT_BITPOS	= 5;
	const U32 ALPHASELECT_MASK		= 1 << ALPHASELECT_BITPOS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCGAMMACONT & ALPHASELECT_MASK) >> ALPHASELECT_BITPOS);
}

//------------------------------------------------------------------------------
//	RGB Layer Specific Operations
//------------------------------------------------------------------------------
/**
 *	@brief		Set the invalid area of RGB Layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number ( 0: RGB0, 1: RGB1 ).
 *	@param[in]	region	select region ( 0 or 1 )
 *	@param[in]	sx		the x-coordinate of the upper-left corner of the layer, 0 ~ +2047.
 *	@param[in]	sy		the y-coordinate of the upper-left corner of the layer, 0 ~ +2047.
 *	@param[in]	ex		the x-coordinate of the lower-right corner of the layer, 0 ~ +2047.
 *	@param[in]	ey		the y-coordinate of the lower-right corner of the layer, 0 ~ +2047.
 *	@param[in]	bEnb	 CTRUE indicates that invalid region Enable,
 *						 CFALSE indicates that invalid region Disable.
 *	@return		None.
 *	@remark		Each RGB Layer support two invalid region. so  region argument must set to 0 or 1.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetRGBLayerInvalidPosition( U32 ModuleIndex, U32 layer, U32 region, S32 sx, S32 sy, S32 ex, S32 ey, CBOOL bEnb )
{
	const U32 INVALIDENB_POS = 28;

	register struct NX_MLC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 2 > layer );
	NX_ASSERT( 2 > region );
	NX_ASSERT( 0 <= sx && sx < 2048 );
	NX_ASSERT( 0 <= sy && sy < 2048 );
	NX_ASSERT( 0 <= ex && ex < 2048 );
	NX_ASSERT( 0 <= ey && ey < 2048 );
	NX_ASSERT( (0==bEnb) || (1==bEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
		if( 0 == region )
		{
		//	pRegister->MLCRGBLAYER[layer].MLCINVALIDLEFTRIGHT0 = ((bEnb<<INVALIDENB_POS) | ((sx&0x7FF)<<16) | (ex&0x7FF) );
			WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCINVALIDLEFTRIGHT0, ((bEnb<<INVALIDENB_POS) | ((sx&0x7FF)<<16) | (ex&0x7FF) ));

		//	pRegister->MLCRGBLAYER[layer].MLCINVALIDTOPBOTTOM0 = ( ((sy&0x7FF)<<16) | (ey&0x7FF) );
			WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCINVALIDTOPBOTTOM0, ( ((sy&0x7FF)<<16) | (ey&0x7FF) ));
		}
		else
		{
		//	pRegister->MLCRGBLAYER[layer].MLCINVALIDLEFTRIGHT1 = ((bEnb<<INVALIDENB_POS) | ((sx&0x7FF)<<16) | (ex&0x7FF) );
			WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCINVALIDLEFTRIGHT1, ((bEnb<<INVALIDENB_POS) | ((sx&0x7FF)<<16) | (ex&0x7FF) ));

		//	pRegister->MLCRGBLAYER[layer].MLCINVALIDTOPBOTTOM1 = ( ((sy&0x7FF)<<16) | (ey&0x7FF) );
			WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCINVALIDTOPBOTTOM1, ( ((sy&0x7FF)<<16) | (ey&0x7FF) ));
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set stride for horizontal and vertical.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer		the layer number ( 0: RGB0, 1: RGB1 ).
 *	@param[in]	hstride	the horizontal stride specifying the number of
 *							bytes from one pixel to the next. Generally, this
 *							value has bytes per pixel. You have to set it only
 *							to a positive value.
 *	@param[in]	vstride		the vertical stride specifying the number of
 *							bytes from one scan line of the image buffer to
 *							the next. Generally, this value has bytes per a
 *							line. You can set it to a negative value for
 *							vertical flip.
 *	@return		None.
 *	@remarks	The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetRGBLayerStride( U32 ModuleIndex, U32 layer, S32 hstride, S32 vstride )
{
	register struct NX_MLC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 0 <= hstride );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
	//	pRegister->MLCRGBLAYER[layer].MLCHSTRIDE = hstride;
		WriteIO32((volatile U32 *)&pRegister->MLCRGBLAYER[layer].MLCHSTRIDE, hstride);

	//	pRegister->MLCRGBLAYER[layer].MLCVSTRIDE = vstride;
		WriteIO32((volatile U32 *)&pRegister->MLCRGBLAYER[layer].MLCVSTRIDE, vstride);
	}
	else if( 2 == layer )
	{
	//	pRegister->MLCRGBLAYER[layer].MLCHSTRIDE = hstride;
		WriteIO32((volatile U32 *)&pRegister->MLCRGBLAYER2.MLCHSTRIDE, hstride);

	//	pRegister->MLCRGBLAYER[layer].MLCVSTRIDE = vstride;
		WriteIO32((volatile U32 *)&pRegister->MLCRGBLAYER2.MLCVSTRIDE, vstride);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set an address of the image buffer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	layer	the layer number( 0: RGB0, 1: RGB1 ).
 *	@param[in]	addr	an address of the image buffer.
 *	@return		None.
 *	@remark		Normally, the argument 'addr' specifies an address of upper-left
 *				corner of the image. but you have to set it to an address of
 *				lower-left corner for vertical mirror.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetRGBLayerAddress( U32 ModuleIndex, U32 layer, U32 addr )
{
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );


	//@modified choiyk 2013-01-21 오후 10:11:04
	//아마 이부분은 locksize에 따라 틀릴것으로 생각되는데
	//일단 최대 256 aling이 되어야한다. ( 16length)

	//NX_ASSERT( 0 == (addr % 256) ); // 16bit align이 되어야함


	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( 0 == layer || 1 == layer )
	{
	//	pRegister->MLCRGBLAYER[layer].MLCADDRESS = addr;
		WriteIO32(&pRegister->MLCRGBLAYER[layer].MLCADDRESS, addr);
	}
	else if( 2 == layer )
	{
	//	pRegister->MLCRGBLAYER[layer].MLCADDRESS = addr;
		WriteIO32(&pRegister->MLCRGBLAYER2.MLCADDRESS, addr);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set power of RGB layer's gamma table.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bRed	 CTRUE indicates that Red gamma table is power on. 
 *						 CFALSE indicates that Red gamma table is power off.
 *	@param[in]	bGreen	 CTRUE indicates that Green gamma table is power on. 
 *						 CFALSE indicates that Green gamma table is power off.
 *	@param[in]	bBlue	 CTRUE indicates that Blue gamma table is power on. 
 *						 CFALSE indicates that Blue gamma table is power off.
 *	@return		None.
 *	@remarks	Gamma table must on before MLC gamma enable.
 */
void	NX_MLC_SetRGBLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL bRed, CBOOL bGreen, CBOOL bBlue )
{
	const U32 BGAMMATABLE_PWD_BITPOS = 11;
	const U32 GGAMMATABLE_PWD_BITPOS = 9;
	const U32 RGAMMATABLE_PWD_BITPOS = 3;

	const U32 BGAMMATABLE_PWD_MASK = ( 1 << BGAMMATABLE_PWD_BITPOS );
	const U32 GGAMMATABLE_PWD_MASK = ( 1 << GGAMMATABLE_PWD_BITPOS );
	const U32 RGAMMATABLE_PWD_MASK = ( 1 << RGAMMATABLE_PWD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bRed) || (1==bRed) );
	NX_ASSERT( (0==bGreen) || (1==bGreen) );
	NX_ASSERT( (0==bBlue) || (1==bBlue) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	ReadValue &= ~( BGAMMATABLE_PWD_MASK | GGAMMATABLE_PWD_MASK | RGAMMATABLE_PWD_MASK );

	ReadValue |= ( ((U32)bRed	<< RGAMMATABLE_PWD_BITPOS) |
					((U32)bGreen << GGAMMATABLE_PWD_BITPOS) |
					((U32)bBlue	<< BGAMMATABLE_PWD_BITPOS) );

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of RGB layer's gamma table is power On or NOT.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[out] pbRed	 CTRUE indicates that Red gamma table is power on. 
 *						 CFALSE indicates that Red gamma table is power off.
 *	@param[out] pbGreen	 CTRUE indicates that Green gamma table is power on. 
 *						 CFALSE indicates that Green gamma table is power off.
 *	@param[out] pbBlue	 CTRUE indicates that Blue gamma table is power on. 
 *						 CFALSE indicates that Blue gamma table is power off.
 *	@return		None.
 */
void	NX_MLC_GetRGBLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL *pbRed, CBOOL *pbGreen, CBOOL *pbBlue )
{
	const U32 BGAMMATABLE_PWD_BITPOS = 11;
	const U32 GGAMMATABLE_PWD_BITPOS = 9;
	const U32 RGAMMATABLE_PWD_BITPOS = 3;

	const U32 BGAMMATABLE_PWD_MASK = ( 1 << BGAMMATABLE_PWD_BITPOS );
	const U32 GGAMMATABLE_PWD_MASK = ( 1 << GGAMMATABLE_PWD_BITPOS );
	const U32 RGAMMATABLE_PWD_MASK = ( 1 << RGAMMATABLE_PWD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	if( CNULL != pbRed	)	{ *pbRed	= (ReadValue & RGAMMATABLE_PWD_MASK) ? CTRUE : CFALSE;	}
	if( CNULL != pbGreen)	{ *pbGreen = (ReadValue & GGAMMATABLE_PWD_MASK) ? CTRUE : CFALSE;	}
	if( CNULL != pbBlue )	{ *pbBlue	= (ReadValue & BGAMMATABLE_PWD_MASK) ? CTRUE : CFALSE;	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set sleep mode of RGB layer's gamma table.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bRed	 CTRUE indicates that Red gamma table is sleep on. 
 *						 CFALSE indicates that Red gamma table is sleep off.
 *	@param[in]	bGreen	 CTRUE indicates that Green gamma table is sleep on. 
 *						 CFALSE indicates that Green gamma table is sleep off.
 *	@param[in]	bBlue	 CTRUE indicates that Blue gamma table is sleep on. 
 *						 CFALSE indicates that Blue gamma table is sleep off.
 *	@return		None.
 *	@remarks	Sleep mode is only usable when gamma table is power ON.
 */
void	NX_MLC_SetRGBLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL bRed, CBOOL bGreen, CBOOL bBlue )
{
	const U32 BGAMMATABLE_SLD_BITPOS = 10;
	const U32 GGAMMATABLE_SLD_BITPOS = 8;
	const U32 RGAMMATABLE_SLD_BITPOS = 2;

	const U32 BGAMMATABLE_SLD_MASK = ( 1 << BGAMMATABLE_SLD_BITPOS );
	const U32 GGAMMATABLE_SLD_MASK = ( 1 << GGAMMATABLE_SLD_BITPOS );
	const U32 RGAMMATABLE_SLD_MASK = ( 1 << RGAMMATABLE_SLD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	if( bRed	)	{ ReadValue &= ~RGAMMATABLE_SLD_MASK; }
	else		{ ReadValue |=	RGAMMATABLE_SLD_MASK; }

	if( bGreen)	{ ReadValue &= ~GGAMMATABLE_SLD_MASK; }
	else		{ ReadValue |=	GGAMMATABLE_SLD_MASK; }

	if( bBlue )	{ ReadValue &= ~BGAMMATABLE_SLD_MASK; }
	else		{ ReadValue |=	BGAMMATABLE_SLD_MASK; }

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of RGB layer's gamma table is Sleep mode or NOT.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[out] pbRed	 CTRUE indicates that Red gamma table is sleep on. 
 *						 CFALSE indicates that Red gamma table is sleep off.
 *	@param[out] pbGreen	 CTRUE indicates that Green gamma table is sleep on. 
 *						 CFALSE indicates that Green gamma table is sleep off.
 *	@param[out] pbBlue	 CTRUE indicates that Blue gamma table is sleep on. 
 *						 CFALSE indicates that Blue gamma table is sleep off.
 *	@return		None.
 */
void	NX_MLC_GetRGBLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL *pbRed, CBOOL *pbGreen, CBOOL *pbBlue )
{
	const U32 BGAMMATABLE_SLD_BITPOS = 10;
	const U32 GGAMMATABLE_SLD_BITPOS = 8;
	const U32 RGAMMATABLE_SLD_BITPOS = 2;

	const U32 BGAMMATABLE_SLD_MASK = ( 1 << BGAMMATABLE_SLD_BITPOS );
	const U32 GGAMMATABLE_SLD_MASK = ( 1 << GGAMMATABLE_SLD_BITPOS );
	const U32 RGAMMATABLE_SLD_MASK = ( 1 << RGAMMATABLE_SLD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	// // 0: Sleep Mode 1: Normal
	if( CNULL != pbRed )	{ *pbRed	= (ReadValue & RGAMMATABLE_SLD_MASK) ? CFALSE : CTRUE;	}
	if( CNULL != pbGreen )	{ *pbGreen = (ReadValue & GGAMMATABLE_SLD_MASK) ? CFALSE : CTRUE;	}
	if( CNULL != pbBlue )	{ *pbBlue	= (ReadValue & BGAMMATABLE_SLD_MASK) ? CFALSE : CTRUE;	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Red gamma table value
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	dwAddress		Red gamma table address (0~255).
 *	@param[in]	dwData			Red gamma table value (10bit data).
 *	@return		None.
 */
void	NX_MLC_SetRGBLayerRGammaTable( U32 ModuleIndex, U32 dwAddress, U32 dwData )
{
	register struct NX_MLC_RegisterSet* pRegister;

	const U32 TABLEADDR_BITPOS = 24;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 256 > dwAddress );
	NX_ASSERT( (1<<10) > dwData );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCRGAMMATABLEWRITE = ((dwAddress << TABLEADDR_BITPOS) | dwData);
	WriteIO32(&pRegister->MLCRGAMMATABLEWRITE, ((dwAddress << TABLEADDR_BITPOS) | dwData));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Green gamma table value
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	dwAddress		Green gamma table address (0~255).
 *	@param[in]	dwData			Green gamma table value (10bit data).
 *	@return		None.
 */
void	NX_MLC_SetRGBLayerGGammaTable( U32 ModuleIndex, U32 dwAddress, U32 dwData )
{
	register struct NX_MLC_RegisterSet* pRegister;

	const U32 TABLEADDR_BITPOS = 24;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 256 > dwAddress );
	NX_ASSERT( (1<<10) > dwData );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCGGAMMATABLEWRITE = ((dwAddress << TABLEADDR_BITPOS) | dwData);
	WriteIO32(&pRegister->MLCGGAMMATABLEWRITE, ((dwAddress << TABLEADDR_BITPOS) | dwData));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Blue gamma table value
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	dwAddress		Blue gamma table address (0~255).
 *	@param[in]	dwData			Blue gamma table value (10bit data).
 *	@return		None.
 */
void	NX_MLC_SetRGBLayerBGammaTable( U32 ModuleIndex, U32 dwAddress, U32 dwData )
{
	register struct NX_MLC_RegisterSet* pRegister;

	const U32 TABLEADDR_BITPOS = 24;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 256 > dwAddress );
	NX_ASSERT( (1<<10) > dwData );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCBGAMMATABLEWRITE = ((dwAddress << TABLEADDR_BITPOS) | dwData);
	WriteIO32(&pRegister->MLCBGAMMATABLEWRITE, ((dwAddress << TABLEADDR_BITPOS) | dwData));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set RGB layer gamma enable or Not.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bEnable			 CTRUE indicates that RGB layer's gamma enable. 
 *								 CFALSE indicates that RGB layer's gamma disable.
 *	@return		None.
 */
void	NX_MLC_SetRGBLayerGammaEnable( U32 ModuleIndex, CBOOL bEnable )
{
	const U32 RGBGAMMAEMB_BITPOS	= 1;
	const U32 RGBGAMMAEMB_MASK		= 1 << RGBGAMMAEMB_BITPOS;

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnable) | (1==bEnable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	ReadValue &= ~RGBGAMMAEMB_MASK;
	ReadValue |= (U32)bEnable << RGBGAMMAEMB_BITPOS;

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of RGB layer's gamma is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	Indicates that RGB layer's gamma is enabled. 
 *				 CFALSE	Indicates that RGB layer's gamma is disabled.
 */
CBOOL	NX_MLC_GetRGBLayerGammaEnable( U32 ModuleIndex )
{
	const U32 RGBGAMMAEMB_BITPOS	= 1;
	const U32 RGBGAMMAEMB_MASK		= 1 << RGBGAMMAEMB_BITPOS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return(CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCGAMMACONT & RGBGAMMAEMB_MASK) >> RGBGAMMAEMB_BITPOS);
}

//--------------------------------------------------------------------------
// Video Layer Specific Operations
//------------------------------------------------------------------------------
/**
 *	@brief		Set the vertical stride for video layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	LuStride	the vertical stride for Y component.
 *	@param[in]	CbStride	the vertical stride for Cb component.
 *	@param[in]	CrStride	the vertical stride for Cr component.
 *	@return		None.
 *	@remark		The vertical stride specifies the number of bytes from one scan line
 *				of the image buffer to the next. Generally, it has bytes per a line.
 *				You have to set it only to a positive value.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerStride( U32 ModuleIndex, S32 LuStride, S32 CbStride, S32 CrStride )
{
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 0 < LuStride );
	NX_ASSERT( 0 < CbStride );
	NX_ASSERT( 0 < CrStride );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCVIDEOLAYER.MLCVSTRIDE	= LuStride;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCVSTRIDE	, LuStride);

//	pRegister->MLCVIDEOLAYER.MLCVSTRIDECB = CbStride;
	WriteIO32((volatile U32 *)&pRegister->MLCVIDEOLAYER.MLCVSTRIDECB, CbStride);

//	pRegister->MLCVIDEOLAYER.MLCVSTRIDECR = CrStride;
	WriteIO32((volatile U32 *)&pRegister->MLCVIDEOLAYER.MLCVSTRIDECR, CrStride);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set an address of the image buffer for video layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	LuAddr	an address of the Y component buffer which has 2D block addressing format.
 *	@param[in]	CbAddr	an address of the Cb component buffer which has 2D block addressing format.
 *	@param[in]	CrAddr	an address of the Cr component buffer which has 2D block addressing format.
 *	@return		None.
 *	@remark		This function is valid when the format of video layer must has
 *				separated YUV format. Each color component has the buffer address.
 *				The 2D block addressing format is as follwings.
 *				- Addr[31:24] specifies the index of segment.
 *				- Addr[23:12] specifies the y-coordinate of upper-left corner of the
 *						image in segment.
 *				- Addr[11: 0] specifies the x-coordinate of upper-left corner of the
 *					image in segment.
 *				.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerAddress( U32 ModuleIndex, U32 LuAddr, U32 CbAddr, U32 CrAddr )
{
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	//@modified choiyk 2012-10-29 오후 5:37:02
	// 16 Align이 되어야함!
#if !defined(CONFIG_PLAT_NXP2120_MID) || !defined(CONFIG_PLAT_NXP2120_CREATE)
	NX_ASSERT( 0 == (LuAddr & 15) );
	NX_ASSERT( 0 == (CbAddr & 15) );
	NX_ASSERT( 0 == (CrAddr & 15) );
#endif
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCVIDEOLAYER.MLCADDRESS	= LuAddr;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCADDRESS,   LuAddr);

//	pRegister->MLCVIDEOLAYER.MLCADDRESSCB = CbAddr;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCADDRESSCB, CbAddr);

//	pRegister->MLCVIDEOLAYER.MLCADDRESSCR	= CrAddr;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCADDRESSCR, CrAddr);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set an address of the image buffer for video layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	Addr	An address of the buffer which has YUYV addressing format.
 *	@param[in]	Stride	The vertical stride.
 *	@return		None.
 *	@remark		This function is valid when the format of video layer is YUVV format.
 */
void	NX_MLC_SetVideoLayerAddressYUYV( U32 ModuleIndex, U32 Addr, S32 Stride )
{
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != Addr );
	NX_ASSERT( CNULL != Stride );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCVIDEOLAYER.MLCADDRESS = Addr;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCADDRESS, Addr);

//	pRegister->MLCVIDEOLAYER.MLCVSTRIDE = Stride;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCVSTRIDE, Stride);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Specifies the scale ratio for video layer with the scale factor.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	hscale		the horizontal scale factor, 23 bit unsigned value.
 *	@param[in]	vscale		the vertical scale factor, 23 bit unsigned value.
 *	@param[in]	bHLumaEnb	Set it CTRUE to apply Bi-linear filter for horizontal scale-up(Luminance).
 *	@param[in]	bHChromaEnb Set it CTRUE to apply Bi-linear filter for horizontal scale-up(Chroma).
 *	@param[in]	bVLumaEnb	Set it CTRUE to apply Bi-linear filter for vertical scale-up(Luminance).
 *	@param[in]	bVChromaEnb Set it CTRUE to apply Bi-linear filter for vertical scale-up(Chroma).
 *	@return		None.
 *	@remark		The scale factor can be calculated by following formula.
 *				If Bilinear Filter is used,
 *				- hscale = (source width-1) * (1<<11) / (destination width-1).
 *				- vscale = (source height-1) * (1<<11) / (destinatin height-1).
 *				.
 *				,else
 *				- hscale = source width * (1<<11) / destination width.
 *				- vscale = source height * (1<<11) / destinatin height.
 *				.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerScaleFactor( U32 ModuleIndex, U32 hscale, U32 vscale, CBOOL bHLumaEnb, CBOOL bHChromaEnb, CBOOL bVLumaEnb, CBOOL bVChromaEnb )
{
	const U32 FILTER_LUMA_POS	= 28;
	const U32 FILTER_CHOMA_POS	= 29;
	const U32 SCALE_MASK		= ((1<<23)-1);

	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( (1<<23) > hscale );
	NX_ASSERT( (1<<23) > vscale );
	NX_ASSERT( (0==bHLumaEnb) ||(1==bHLumaEnb) );
	NX_ASSERT( (0==bHChromaEnb) ||(1==bHChromaEnb) );
	NX_ASSERT( (0==bVLumaEnb) ||(1==bVLumaEnb) );
	NX_ASSERT( (0==bVChromaEnb) ||(1==bVChromaEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCVIDEOLAYER.MLCHSCALE = ((bHLumaEnb << FILTER_LUMA_POS) | (bHChromaEnb << FILTER_CHOMA_POS) | (hscale & SCALE_MASK) );
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCHSCALE, ((bHLumaEnb << FILTER_LUMA_POS) | (bHChromaEnb << FILTER_CHOMA_POS) | (hscale & SCALE_MASK) ));

//	pRegister->MLCVIDEOLAYER.MLCVSCALE = ((bVLumaEnb << FILTER_LUMA_POS) | (bVChromaEnb << FILTER_CHOMA_POS) | (vscale & SCALE_MASK) );
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCVSCALE, ((bVLumaEnb << FILTER_LUMA_POS) | (bVChromaEnb << FILTER_CHOMA_POS) | (vscale & SCALE_MASK) ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Specifies the scale filter.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bHLumaEnb	Set it CTRUE to apply Bi-linear filter for horizontal scale-up(Luminance).
 *	@param[in]	bHChromaEnb Set it CTRUE to apply Bi-linear filter for horizontal scale-up(Chroma).
 *	@param[in]	bVLumaEnb	Set it CTRUE to apply Bi-linear filter for vertical scale-up(Luminance).
 *	@param[in]	bVChromaEnb Set it CTRUE to apply Bi-linear filter for vertical scale-up(Chroma).
 *	@return		None.
 *	@remark		The scale factor can be calculated by following formula.
 *				If Bilinear Filter is used,
 *				- hscale = (source width-1) * (1<<11) / (destination width-1).
 *				- vscale = (source height-1) * (1<<11) / (destinatin height-1).
 *				.
 *				,else
 *				- hscale = source width * (1<<11) / destination width.
 *				- vscale = source height * (1<<11) / destinatin height.
 *				.
 *				The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerScaleFilter( U32 ModuleIndex, CBOOL bHLumaEnb, CBOOL bHChromaEnb, CBOOL bVLumaEnb, CBOOL bVChromaEnb )
{
	const U32 FILTER_LUMA_POS	= 28;
	const U32 FILTER_CHOMA_POS	= 29;
	const U32 SCALE_MASK		= ((1<<23)-1);

	register struct NX_MLC_RegisterSet* pRegister;
	register U32 	ReadValue;

	NX_ASSERT( (0==bHLumaEnb) ||(1==bHLumaEnb) );
	NX_ASSERT( (0==bHChromaEnb) ||(1==bHChromaEnb) );
	NX_ASSERT( (0==bVLumaEnb) ||(1==bVLumaEnb) );
	NX_ASSERT( (0==bVChromaEnb) ||(1==bVChromaEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue  = pRegister->MLCVIDEOLAYER.MLCHSCALE;
	ReadValue &= SCALE_MASK;
	ReadValue |= (bHLumaEnb << FILTER_LUMA_POS) | (bHChromaEnb << FILTER_CHOMA_POS);

//	pRegister->MLCVIDEOLAYER.MLCHSCALE = ReadValue;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCHSCALE, ReadValue);

	ReadValue  = pRegister->MLCVIDEOLAYER.MLCVSCALE;
	ReadValue &= SCALE_MASK;
	ReadValue |= (bVLumaEnb << FILTER_LUMA_POS) | (bVChromaEnb << FILTER_CHOMA_POS);

//	pRegister->MLCVIDEOLAYER.MLCVSCALE = ReadValue;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCVSCALE, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the scale filter status.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bHLumaEnb	Get Bi-linear filter status for horizontal scale-up(Luminance).
 *	@param[in]	bHChromaEnb Get Bi-linear filter status for horizontal scale-up(Chroma).
 *	@param[in]	bVLumaEnb	Get Bi-linear filter status for vertical scale-up(Luminance).
 *	@param[in]	bVChromaEnb Get Bi-linear filter status for vertical scale-up(Chroma).
 *	@return		None.
 */
void	NX_MLC_GetVideoLayerScaleFilter( U32 ModuleIndex, CBOOL *bHLumaEnb, CBOOL *bHChromaEnb, CBOOL *bVLumaEnb, CBOOL *bVChromaEnb )
{
	const U32 FILTER_LUMA_POS	= 28;
	const U32 FILTER_CHOMA_POS	= 29;
	const U32 FILTER_MASK		= 1UL;

	register struct NX_MLC_RegisterSet* pRegister;
	register U32 	ReadValue;

	NX_ASSERT( bHLumaEnb );
	NX_ASSERT( bHChromaEnb );
	NX_ASSERT( bVLumaEnb );
	NX_ASSERT( bVChromaEnb );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue    = pRegister->MLCVIDEOLAYER.MLCHSCALE;
	*bHLumaEnb 	 = (ReadValue >> FILTER_LUMA_POS)  & FILTER_MASK;
	*bHChromaEnb = (ReadValue >> FILTER_CHOMA_POS) & FILTER_MASK;

	ReadValue    = pRegister->MLCVIDEOLAYER.MLCVSCALE;
	*bVLumaEnb 	 = (ReadValue >> FILTER_LUMA_POS)  & FILTER_MASK;
	*bVChromaEnb = (ReadValue >> FILTER_CHOMA_POS) & FILTER_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Specifies the scale ratio for video layer with the width and height.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	sw		the width of the source image.
 *	@param[in]	sh		the height of the source image.
 *	@param[in]	dw		the width of the destination image.
 *	@param[in]	dh		the height of the destination image.
 *	@param[in]	bHLumaEnb	Set it CTRUE to apply Bi-linear filter for horizontal scale-up(Luminance).
 *	@param[in]	bHChromaEnb Set it CTRUE to apply Bi-linear filter for horizontal scale-up(Chroma).
 *	@param[in]	bVLumaEnb	Set it CTRUE to apply Bi-linear filter for vertical scale-up(Luminance).
 *	@param[in]	bVChromaEnb Set it CTRUE to apply Bi-linear filter for vertical scale-up(Chroma).
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerScale( U32 ModuleIndex, U32 sw, U32 sh, U32 dw, U32 dh, CBOOL bHLumaEnb, CBOOL bHChromaEnb, CBOOL bVLumaEnb, CBOOL bVChromaEnb )
{
	const U32 FILTER_LUMA_POS	= 28;
	const U32 FILTER_CHOMA_POS	= 29;
	const U32 SCALE_MASK		= ((1<<23)-1);
	register U32 hscale, vscale, cal_sh;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( 4096 > sw );
	NX_ASSERT( 4096 > sh );
	NX_ASSERT( 4096 > dw );
	NX_ASSERT( 4096 > dh );
	NX_ASSERT( (0==bHLumaEnb)	|| (1==bHLumaEnb) );
	NX_ASSERT( (0==bHChromaEnb) || (1==bHChromaEnb) );
	NX_ASSERT( (0==bVLumaEnb)	|| (1==bVLumaEnb) );
	NX_ASSERT( (0==bVChromaEnb) || (1==bVChromaEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	// HScale
	if( (bHLumaEnb || bHChromaEnb) && (dw > sw) )
	{
		sw--;
		dw--;
	}

	hscale = (sw << 11) / dw;

	// VScale
	if( (bVLumaEnb || bVChromaEnb) && ( dh > sh ) )
	{
		sh--;
		dh--;
		vscale = (sh << 11) / dh;

		// To remove a noisy vertical line at end of image in case of vertical filter on and scale-up.
		cal_sh = ((vscale * dh) >> 11);
		if( sh <= cal_sh )
		{
			vscale--;
		}
	}
	else
	{
		vscale = (sh << 11) / dh;
	}

//	pRegister->MLCVIDEOLAYER.MLCHSCALE = ((bHLumaEnb << FILTER_LUMA_POS) | (bHChromaEnb << FILTER_CHOMA_POS) | (hscale & SCALE_MASK) );
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCHSCALE, ((bHLumaEnb << FILTER_LUMA_POS) | (bHChromaEnb << FILTER_CHOMA_POS) | (hscale & SCALE_MASK) ));

//	pRegister->MLCVIDEOLAYER.MLCVSCALE = ((bVLumaEnb << FILTER_LUMA_POS) | (bVChromaEnb << FILTER_CHOMA_POS) | (vscale & SCALE_MASK) );
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCVSCALE, ((bVLumaEnb << FILTER_LUMA_POS) | (bVChromaEnb << FILTER_CHOMA_POS) | (vscale & SCALE_MASK) ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set brightness and contrast for video layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	contrast	the contrast value, 0 ~ 7.
 *							0 : 1.0, 1 : 1.125, 2 : 1.25, 3 : 1.375,
 *							4 : 1.5, 5 : 1.625, 6 : 1.75, 7 : 1.875
 *	@param[in]	brightness	the brightness value, -128 ~ +127.
 *	@return		None.
 *	@remark		The result of this function will be applied to corresponding layer
 *				after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerLumaEnhance( U32 ModuleIndex, U32 contrast, S32 brightness )
{
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 8 > contrast );
	NX_ASSERT( -128 <= brightness && brightness < 128 );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

//	pRegister->MLCVIDEOLAYER.MLCLUENH = (((U32)brightness & 0xFFUL)<<8) | contrast;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCLUENH, (((U32)brightness & 0xFFUL)<<8) | contrast);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set factors to control hue and satuation for video layer.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	quadrant	a quadrant to apply hue and saturation, 0 ~ 4.
 *							Set it to 0 to apply hue and saturation on all quadrant.
 *	@param[in]	CbA		cosine value for B-Y axis, -128 ~ +127.
 *	@param[in]	CbB		sine value for R-Y axis, -128 ~ +127.
 *	@param[in]	CrA		sine value for B-Y axis, -128 ~ +127.
 *	@param[in]	CrB			cosine value for R-Y axis, -128 ~ +127.
 *	@return		None.
 *	@remark		Each quadrant has factors to control hue and satuation.
 *				At each coordinate, HUE and saturation is applied by following formula.
 *					(B-Y)' = (B-Y)*CbA + (R-Y)*CbB 
 *					(R-Y)' = (B-Y)*CrA + (R-Y)*CrB 
 *				, where 
 *					CbA = cos(θ) * 64 * gain, CbB = -sin(θ) * 64 * gain 
 *					CrA = sin(θ) * 64 * gain, CrB =	cos(θ) * 64 * gain 
 *					gain is 0 to 2. 
 *				The example for this equation is as follows.
 *	@code
 *		// mi = 0 ( module index )
 *		// HUE : phase relationship of the chrominance components, -180 ~ 180 degree.
 *		// Sat : color intensity, 0 ~ 127.
 *		void SetChromaEnhance( mi, int Hue, int Sat )
 *		{
 *			S32 sv, cv;
 *
 *			if( Sat < 0 )			Sat = 0;
 *			else if( Sat > 127 )	Sat = 127;
 *
 *			sv = (S32)(sin( (3.141592654f * Hue) / 180 ) * Sat);
 *			cv = (S32)(cos( (3.141592654f * Hue) / 180 ) * Sat);
 *
 *			NX_MLC_SetVideoLayerChromaEnhance( mi, 0, cv, -sv, sv, cv );
 *		};
 *	@endcode
 *			The result of this function will be applied to corresponding layer
 *			after calling function NX_MLC_SetDirtyFlag() with corresponding layer.
 */
void	NX_MLC_SetVideoLayerChromaEnhance( U32 ModuleIndex, U32 quadrant, S32 CbA, S32 CbB, S32 CrA, S32 CrB )
{
	register struct NX_MLC_RegisterSet* pRegister;
	register U32 temp;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( 5 > quadrant );
	NX_ASSERT( -128 <= CbA && CbA < 128 );
	NX_ASSERT( -128 <= CbB && CbB < 128 );
	NX_ASSERT( -128 <= CrA && CrA < 128 );
	NX_ASSERT( -128 <= CrB && CrB < 128 );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	temp	= (((U32)CrB & 0xFFUL)<<24)
			| (((U32)CrA & 0xFFUL)<<16)
			| (((U32)CbB & 0xFFUL)<< 8)
			| (((U32)CbA & 0xFFUL)<< 0);

	if( 0 < quadrant )
	{
	//	pRegister->MLCVIDEOLAYER.MLCCHENH[quadrant-1] = temp;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCHENH[quadrant-1], temp);
	}
	else
	{
	//	pRegister->MLCVIDEOLAYER.MLCCHENH[0] = temp;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCHENH[0], temp);

	//	pRegister->MLCVIDEOLAYER.MLCCHENH[1] = temp;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCHENH[1], temp);

	//	pRegister->MLCVIDEOLAYER.MLCCHENH[2] = temp;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCHENH[2], temp);

	//	pRegister->MLCVIDEOLAYER.MLCCHENH[3] = temp;
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCHENH[3], temp);
	}
}

//-------------------------------------------------------------------------------
/**
 *	@brief		Set power control of video layer's line buffer unit.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bEnable	 CTRUE indicates that Power ON of video layer's line buffer unit. 
 *						 CFALSE indicates that Power OFF of video layer's line buffer unit. 
 *	@return		None.
 *	@remark		- Line buffer can power on/off during Video layer operation.
 *				- Line buffer only use when Video layer's Scale Up/Down operation with biliner filter On .
 *				- Line buffer ON sequence
 *					- Power On
 *					- Normal Mode ( Sleep Mode disable )
 *				- Line buffer OFF sequence
 *					- Sleep Mode
 *					- Power Off
 *	@code
 *				// Line Buffer On sequence, mi = 0 ( module index ), l = 3 (video layer )
 *				NX_MLC_SetVideoLayerLineBufferPowerMode( mi, CTRUE );			// power on
 *				NX_MLC_SetVideoLayerLineBufferSleepMode( mi, CFALSE );			// normal mode
 *				NX_MLC_SetLayerEnable( mi, l, CTRUE );						// layer enable
 *				NX_MLC_SetDirtyFlag( mi, l );									// apply now
 *				...
 *				// Line Buffer Off sequence, mi = 0 ( module index ), l = 3 (video layer )
 *				NX_MLC_SetLayerEnable( mi, l, CFALSE );						// layer disable
 *				NX_MLC_SetDirtyFlag( mi, l );									// apply now
 *				while( CTRUE == NX_MLC_GetDirtyFlag( mi, l ))					// wait until layer is disabled
 *				{ CNULL; }
 *				NX_MLC_SetVideoLayerLineBufferSleepMode( mi, CTRUE );			// sleep mode
 *				NX_MLC_SetVideoLayerLineBufferPowerMode( mi, CFALSE );			// layer off
 *	@endcode
 */
void	NX_MLC_SetVideoLayerLineBufferPowerMode( U32 ModuleIndex, CBOOL bEnable )
{
	const U32 LINEBUFF_PWD_POS	= 15;
	const U32 LINEBUFF_PWD_MASK	= 1UL << LINEBUFF_PWD_POS;
	const U32 DIRTYFLAG_MASK	= 1UL << 4;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCVIDEOLAYER.MLCCONTROL;

	regvalue &= ~( LINEBUFF_PWD_MASK | DIRTYFLAG_MASK );
	regvalue |= ((U32)bEnable << LINEBUFF_PWD_POS);

//	pRegister->MLCVIDEOLAYER.MLCCONTROL = regvalue;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get power state of video layer's line buffer unit
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that video layer's power control unit is power ON.
 *				 CFALSE	indicates that video layer's power control unit is power OFF.
 */
CBOOL	NX_MLC_GetVideoLayerLineBufferPowerMode( U32 ModuleIndex )
{
	const U32 LINEBUFF_PWD_POS	= 15;
	const U32 LINEBUFF_PWD_MASK	= 1UL << LINEBUFF_PWD_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCVIDEOLAYER.MLCCONTROL & LINEBUFF_PWD_MASK ) >> LINEBUFF_PWD_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Sleep mode of video layer's power control unit.
 *	@param[in]	ModuleIndex	An index of module ( 0 : First MLC ).
 *	@param[in]	bEnable	 CTRUE indicates that  SLEEP Mode of video layer's power control unit. 
 *						 CFALSE indicates that  NORMAL Mode of video layer's power control unit. 
 *	@return		None.
 *	@remark		- Power control unit is Line buffer.
 *				- Line buffer can power on/off during Video layer operation.
 *				- Line buffer only use when Video layer's Scale Up/Down operation with biliner filter On .
 *				- Line buffer ON sequence
 *					- Power On
 *					- Normal Mode ( Sleep Mode disable )
 *				- Line buffer OFF sequence
 *					- Sleep Mode
 *					- Power Off
 *	@code
 *				// Line Buffer On sequence, mi = 0 ( module index ), l = 3 (video layer )
 *				NX_MLC_SetVideoLayerLineBufferPowerMode( mi, CTRUE );				// power on
 *				NX_MLC_SetVideoLayerLineBufferSleepMode( mi, CFALSE );			// normal mode
 *				NX_MLC_SetLayerEnable( mi, l, CTRUE );							// layer enable
 *				NX_MLC_SetDirtyFlag( mi, l );										// apply now
 *				...
 *				// Line Buffer Off sequence, mi = 0 ( module index ), l = 3 (video layer )
 *				NX_MLC_SetLayerEnable( mi, l, CFALSE );							// layer disable
 *				NX_MLC_SetDirtyFlag( mi, l );										// apply now
 *				while( CTRUE == NX_MLC_GetDirtyFlag( mi, l ))						// wait until layer is disabled
 *				{ CNULL; }
 *				NX_MLC_SetVideoLayerLineBufferSleepMode( mi, CTRUE );				// sleep mode
 *				NX_MLC_SetVideoLayerLineBufferPowerMode( mi, CFALSE );			// layer off
 *	@endcode
 */
void	NX_MLC_SetVideoLayerLineBufferSleepMode( U32 ModuleIndex, CBOOL bEnable )
{
	const U32 LINEBUFF_SLMD_POS		= 14;
	const U32 LINEBUFF_SLMD_MASK	= 1UL << LINEBUFF_SLMD_POS;
	const U32 DIRTYFLAG_MASK		= 1UL << 4;

	register U32 regvalue;
	register struct NX_MLC_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	bEnable = (CBOOL)((U32)bEnable ^ 1);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	regvalue = pRegister->MLCVIDEOLAYER.MLCCONTROL;

	regvalue &= ~( LINEBUFF_SLMD_MASK | DIRTYFLAG_MASK );
	regvalue |= (bEnable << LINEBUFF_SLMD_POS);				// 0:sleep enable, 1:sleep disable

//	pRegister->MLCVIDEOLAYER.MLCCONTROL = regvalue;
	WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get sleep mode state of video layer's power control unit.
 *	@param[in]	ModuleIndex	An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that video layer's power control unit is sleep on.
 *				 CFALSE	indicates that video layer's power control unit is sleep off.
 */
CBOOL	NX_MLC_GetVideoLayerLineBufferSleepMode( U32 ModuleIndex )
{
	const U32 LINEBUFF_SLMD_POS		= 14;
	const U32 LINEBUFF_SLMD_MASK	= 1UL << LINEBUFF_SLMD_POS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( LINEBUFF_SLMD_MASK & __g_ModuleVariables[ModuleIndex].pRegister->MLCVIDEOLAYER.MLCCONTROL )
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
 *	@brief		Set power of YUV layer's gamma table.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bY	 CTRUE indicates that Y gamma table is power on. 
 *					 CFALSE indicates that Y gamma table is power off.
 *	@param[in]	bU	 CTRUE indicates that U gamma table is power on. 
 *					 CFALSE indicates that U gamma table is power off.
 *	@param[in]	bV	 CTRUE indicates that V gamma table is power on. 
 *					 CFALSE indicates that V gamma table is power off.
 *	@return		None.
 *	@remarks	Gamma table must on before MLC gamma enable.
 */
void	NX_MLC_SetVideoLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL bY, CBOOL bU, CBOOL bV )
{
	const U32 VGAMMATABLE_PWD_BITPOS = 17;
	const U32 UGAMMATABLE_PWD_BITPOS = 15;
	const U32 YGAMMATABLE_PWD_BITPOS = 13;

	const U32 VGAMMATABLE_PWD_MASK = ( 1 << VGAMMATABLE_PWD_BITPOS );
	const U32 UGAMMATABLE_PWD_MASK = ( 1 << UGAMMATABLE_PWD_BITPOS );
	const U32 YGAMMATABLE_PWD_MASK = ( 1 << YGAMMATABLE_PWD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bY) || (1==bY) );
	NX_ASSERT( (0==bU) || (1==bU) );
	NX_ASSERT( (0==bV) || (1==bV) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	ReadValue &= ~( YGAMMATABLE_PWD_MASK | UGAMMATABLE_PWD_MASK | VGAMMATABLE_PWD_MASK );

	ReadValue |= ( ((U32)bY << YGAMMATABLE_PWD_BITPOS) |
					((U32)bU << UGAMMATABLE_PWD_BITPOS) |
					((U32)bV << VGAMMATABLE_PWD_BITPOS) );

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of video layer's gamma table is powered or NOT.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[out] pbY		 CTRUE	indicates that Y gamma table is powered. 
 *						 CFALSE indicates that Y gamma table is NOT powered.
 *	@param[out] pbU		 CTRUE	indicates that U gamma table is powered. 
 *						 CFALSE indicates that U gamma table is NOT powered.
 *	@param[out] pbV		 CTRUE	indicates that V gamma table is powered. 
 *						 CFALSE indicates that V gamma table is NOT powered.
 *	@return		None.
 */
void	NX_MLC_GetVideoLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL *pbY, CBOOL *pbU, CBOOL *pbV )
{
	const U32 VGAMMATABLE_PWD_BITPOS = 17;
	const U32 UGAMMATABLE_PWD_BITPOS = 15;
	const U32 YGAMMATABLE_PWD_BITPOS = 13;

	const U32 VGAMMATABLE_PWD_MASK = ( 1 << VGAMMATABLE_PWD_BITPOS );
	const U32 UGAMMATABLE_PWD_MASK = ( 1 << UGAMMATABLE_PWD_BITPOS );
	const U32 YGAMMATABLE_PWD_MASK = ( 1 << YGAMMATABLE_PWD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	if( CNULL != pbY )	{ *pbY = ( ReadValue & YGAMMATABLE_PWD_MASK) ? CTRUE : CFALSE ;	}
	if( CNULL != pbU )	{ *pbU = ( ReadValue & UGAMMATABLE_PWD_MASK) ? CTRUE : CFALSE ;	}
	if( CNULL != pbV )	{ *pbV = ( ReadValue & VGAMMATABLE_PWD_MASK) ? CTRUE : CFALSE ;	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set sleep mode of video layer's gamma table.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bY	 CTRUE indicates that Y gamma table is sleep on. 
 *					 CFALSE indicates that Y gamma table is sleep off.
 *	@param[in]	bU	 CTRUE indicates that U gamma table is sleep on. 
 *					 CFALSE indicates that U gamma table is sleep off.
 *	@param[in]	bV	 CTRUE indicates that V gamma table is sleep on. 
 *					 CFALSE indicates that V gamma table is sleep off.
 *	@return		None.
 *	@remarks	Sleep mode is only usable when gamma table is power ON.
 */
void	NX_MLC_SetVideoLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL bY, CBOOL bU, CBOOL bV )
{
	const U32 VGAMMATABLE_SLD_BITPOS = 16;
	const U32 UGAMMATABLE_SLD_BITPOS = 14;
	const U32 YGAMMATABLE_SLD_BITPOS = 12;

	const U32 VGAMMATABLE_SLD_MASK = ( 1 << VGAMMATABLE_SLD_BITPOS );
	const U32 UGAMMATABLE_SLD_MASK = ( 1 << UGAMMATABLE_SLD_BITPOS );
	const U32 YGAMMATABLE_SLD_MASK = ( 1 << YGAMMATABLE_SLD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	if( bY ){ ReadValue &= ~YGAMMATABLE_SLD_MASK;	}
	else	{ ReadValue |= YGAMMATABLE_SLD_MASK;	}

	if( bU ){ ReadValue &= ~UGAMMATABLE_SLD_MASK;	}
	else	{ ReadValue |= UGAMMATABLE_SLD_MASK;	}

	if( bV ){ ReadValue &= ~VGAMMATABLE_SLD_MASK;	}
	else	{ ReadValue |= VGAMMATABLE_SLD_MASK;	}

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of video layer's gamma table is Sleep mode or NOT.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[out] pbY		 CTRUE indicates that Y gamma table is sleep on. 
 *						 CFALSE indicates that Y gamma table is sleep off.
 *	@param[out] pbU		 CTRUE indicates that U gamma table is sleep on. 
 *						 CFALSE indicates that U gamma table is sleep off.
 *	@param[out] pbV		 CTRUE indicates that V gamma table is sleep on. 
 *						 CFALSE indicates that V gamma table is sleep off.
 *	@return		None.
 */
void	NX_MLC_GetVideoLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL *pbY, CBOOL *pbU, CBOOL *pbV )
{
	const U32 VGAMMATABLE_SLD_BITPOS = 16;
	const U32 UGAMMATABLE_SLD_BITPOS = 14;
	const U32 YGAMMATABLE_SLD_BITPOS = 12;

	const U32 VGAMMATABLE_SLD_MASK = ( 1 << VGAMMATABLE_SLD_BITPOS );
	const U32 UGAMMATABLE_SLD_MASK = ( 1 << UGAMMATABLE_SLD_BITPOS );
	const U32 YGAMMATABLE_SLD_MASK = ( 1 << YGAMMATABLE_SLD_BITPOS );

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	// // 0: Sleep Mode 1: Normal
	if( CNULL != pbY )	{ *pbY = (ReadValue & VGAMMATABLE_SLD_MASK) ? CFALSE : CTRUE;	}
	if( CNULL != pbU )	{ *pbU = (ReadValue & UGAMMATABLE_SLD_MASK) ? CFALSE : CTRUE;	}
	if( CNULL != pbV )	{ *pbV = (ReadValue & YGAMMATABLE_SLD_MASK) ? CFALSE : CTRUE;	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set video layer's gamma enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@param[in]	bEnable			 CTRUE indicates that Video layer's gamma enable. 
 *								 CFALSE indicates that Video layer's gamma disable.
 *	@return		None.
 */
void	NX_MLC_SetVideoLayerGammaEnable( U32 ModuleIndex, CBOOL bEnable )
{
	const U32 YUVGAMMAEMB_BITPOS	= 4;
	const U32 YUVGAMMAEMB_MASK		= 1 << YUVGAMMAEMB_BITPOS;

	register U32 ReadValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( (0==bEnable) | (1==bEnable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->MLCGAMMACONT;

	ReadValue &= ~YUVGAMMAEMB_MASK;
	ReadValue |= (U32)bEnable << YUVGAMMAEMB_BITPOS;

//	pRegister->MLCGAMMACONT = ReadValue;
	WriteIO32(&pRegister->MLCGAMMACONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of video layer's gamma is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First MLC ).
 *	@return		 CTRUE	indicates that video layer's gamma is enabled. 
 *				 CFALSE	indicates that video layer's gamma is disabled.
 */
CBOOL	NX_MLC_GetVideoLayerGammaEnable( U32 ModuleIndex )
{
	const U32 YUVGAMMAEMB_BITPOS	= 4;
	const U32 YUVGAMMAEMB_MASK		= 1 << YUVGAMMAEMB_BITPOS;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return(CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->MLCGAMMACONT & YUVGAMMAEMB_MASK) >> YUVGAMMAEMB_BITPOS);
}




//----------------------------------------------------------------
// Prototype 이식
//----------------------------------------------------------------
//------------------------------------------------------------------------------
/// @brief	set MLC Controller parameter
//------------------------------------------------------------------------------
// Priority Value |  First Layer | Second Layer | Third Layer | Fourth Layer |
//----------------|--------------|--------------|-------------|--------------|
// Priority = 0   |  Video Layer | Layer0       | Layer1      | Layer2       |
void
NX_MLC_SetMLCTopControlParameter
(
    U32 ModuleIndex,
    CBOOL FieldEnable,   ///< [in] display format (interlace and progressive)
    CBOOL MLCEnable,     ///< [in] MLC Operation Enable
    U8 Priority,         ///< [in] Layer preference decide
    G3DADDRCHANGEALLOWED G3DAddrChangeAllowed
)
{

	register U32 MLCTOPControlReg;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	MLCTOPControlReg = ( ReadIO32(&pRegister->MLCCONTROLT) ) & 0xfffffcfc;

	MLCTOPControlReg = (U32)(MLCTOPControlReg | ((Priority<<8) | ((1==MLCEnable)<<1) | (1==FieldEnable)) | (G3DAddrChangeAllowed<<12));
	WriteIO32(&pRegister->MLCCONTROLT, MLCTOPControlReg);
}


//------------------------------------------------------------------------------
/// @brief	set RGB layer control parameter
//------------------------------------------------------------------------------
// 이 함수는 예전의 4bit Alpha를 가정하고 작성한 버전이다.
// 8bit Alpha를 설정하는 함수는 따로 만들도록 한다.
void
NX_MLC_SetRGB0LayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Lyer Enable
    CBOOL GRP3DEnable,		///< [in] GRP3D Lyer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,          ///< [in] Use Alpha value in no Alpha value format case
    RGBFMT RBGFormat,       ///< [in] RGB Format in normal layer case
    LOCKSIZESEL LockSizeSelect ///< [in] Lock Size select
)
{

//	register U32 MLCTOPControlReg;
    U32 LayerFormat;
    U32 ControlEnb;
    U32 AlphaArgument;
    U32 LockSize = (U32)(LockSizeSelect & 0x3);
    U32 RGB0ControlReg;
    U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

    LayerFormat = NX_MLC_GetRGBFormat(RBGFormat);

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    ControlEnb = (U32)((GRP3DEnable<<8) | (LayerEnable << 5) | (BlendEnable<<2) | (InvEnable<<1) | TpEnable) & 0x127;
    AlphaArgument = (U32)(AlphaValue&0xF);

	// Masking 추가.
    RGB0ControlReg = ReadIO32(&pRegister->MLCRGBLAYER[0].MLCCONTROL) & 0x10;
    regvalue = (U32)(((LayerFormat<<16) | ControlEnb | (LockSize<<12)) | RGB0ControlReg);
    WriteIO32(&pRegister->MLCRGBLAYER[0].MLCCONTROL, regvalue );

    //@todo choiyk 2012-10-15 오전 10:45:23
    // Alpha채널이 8bit로 늘어나게 된다. (이 함수자체는 4bit Alpha를 사용)
    regvalue  = (U32)((AlphaArgument<<28) | TransparencyColor);
    WriteIO32(&pRegister->MLCRGBLAYER[0].MLCTPCOLOR, regvalue );

    regvalue = InverseColor;
    WriteIO32(&pRegister->MLCRGBLAYER[0].MLCINVCOLOR, regvalue );

}


U32 NX_MLC_GetRGBFormat(RGBFMT RBGFormat)
{

	U32 RGBFormatValue;

	const U32 FORMAT_TABLE[] =
	{
        0x4432UL,   ///< RGBFMT_R5G6B5
        0x4342UL,   ///< RGBFMT_X1R5G5B5
        0x4211UL,   ///< RGBFMT_X4R4G4B4
        0x4120UL,   ///< RGBFMT_X8R3G3B2
        0x4003UL,   ///< RGBFMT_X8L8
        0x4554UL,   ///< RGBFMT_L16
        0x3342UL,   ///< RGBFMT_A1R5G5B5
        0x2211UL,   ///< RGBFMT_A4R4G4B4
        0x1120UL,   ///< RGBFMT_A8R3G3B2
        0x1003UL,   ///< RGBFMT_A8L8
        0x4653UL,   ///< RGBFMT_R8G8B8
        0x4653UL,   ///< RGBFMT_X8R8G8B8
        0x0653UL,   ///< RGBFMT_A8R8G8B8
        0x4ED3UL,   ///< RGBFMT_G8R8_G8B8
        0x4F84UL,   ///< RGBFMT_R8G8_B8G8
        0xC432UL,   ///< RGBFMT_B5G6R5
        0xC342UL,   ///< RGBFMT_X1B5G5R5
        0xC211UL,   ///< RGBFMT_X4B4G4R4
        0xC120UL,   ///< RGBFMT_X8B3G3R2
        0xB342UL,   ///< RGBFMT_A1B5G5R5
        0xA211UL,   ///< RGBFMT_A4B4G4R4
        0x9120UL,   ///< RGBFMT_A8B3G3R2
        0xC653UL,   ///< RGBFMT_B8G8R8
        0xC653UL,   ///< RGBFMT_X8B8G8R8
        0x8653UL,   ///< RGBFMT_A8B8G8R8
        0xCED3UL,   ///< RGBFMT_G8B8_G8R8
        0xCF84UL,   ///< RGBFMT_B8G8_R8G8
        0x443AUL    ///< RGBFMT_R8G8B8(PalateTable)
	};
    return RGBFormatValue = FORMAT_TABLE[RBGFormat];
}


//------------------------------------------------------------------------------
/// @brief	set RGB layer control parameter
//------------------------------------------------------------------------------
void
NX_MLC_SetRGB1LayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Lyer Enable
    CBOOL GRP3DEnable,		///< [in] GRP3D Lyer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,          ///< [in] Use Alpha value in no Alpha value format case
    RGBFMT RBGFormat,       ///< [in] RGB Format in normal layer case
    LOCKSIZESEL LockSizeSelect ///< [in] Lock Size select
)
{

//	register U32 MLCTOPControlReg;
    U32 LayerFormat;
    U32 ControlEnb;
    U32 AlphaArgument;
    U32 LockSize = (U32)(LockSizeSelect & 0x3);
    U32 RGB0ControlReg;
    U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

    LayerFormat = NX_MLC_GetRGBFormat(RBGFormat);

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;


    RGB0ControlReg = ReadIO32(&pRegister->MLCRGBLAYER[1].MLCCONTROL) & 0x10;
    ControlEnb = (U32)((GRP3DEnable<<8) | (LayerEnable << 5) | (BlendEnable<<2) | (InvEnable<<1) | TpEnable) & 0x127;
    AlphaArgument = (U32)(AlphaValue&0xF);

    regvalue = (U32)(((LayerFormat<<16) | ControlEnb | (LockSize<<12)) | RGB0ControlReg);
    WriteIO32(&pRegister->MLCRGBLAYER[1].MLCCONTROL, regvalue );

    regvalue  = (U32)((AlphaArgument<<28) | TransparencyColor);
    WriteIO32(&pRegister->MLCRGBLAYER[1].MLCTPCOLOR, regvalue );

    regvalue = InverseColor;
    WriteIO32(&pRegister->MLCRGBLAYER[1].MLCINVCOLOR, regvalue );

}

//------------------------------------------------------------------------------
/// @brief	set RGB layer control parameter
//------------------------------------------------------------------------------
void
NX_MLC_SetRGB2LayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Lyer Enable
    CBOOL GRP3DEnable,		///< [in] GRP3D Lyer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,          ///< [in] Use Alpha value in no Alpha value format case
    RGBFMT RBGFormat,       ///< [in] RGB Format in normal layer case
    LOCKSIZESEL LockSizeSelect ///< [in] Lock Size select
)
{

//	register U32 MLCTOPControlReg;
    U32 LayerFormat;
    U32 ControlEnb;
    U32 AlphaArgument;
    U32 LockSize = (U32)(LockSizeSelect & 0x3);
    U32 RGB0ControlReg;
    U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;

    LayerFormat = NX_MLC_GetRGBFormat(RBGFormat);

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;


    RGB0ControlReg = ReadIO32(&pRegister->MLCRGBLAYER2.MLCCONTROL) & 0x10;
    ControlEnb = (U32)((GRP3DEnable<<8) | (LayerEnable << 5) | (BlendEnable<<2) | (InvEnable<<1) | TpEnable) & 0x127;
    AlphaArgument = (U32)(AlphaValue&0xF);

    regvalue = (U32)(((LayerFormat<<16) | ControlEnb | (LockSize<<12)) | RGB0ControlReg);
    WriteIO32(&pRegister->MLCRGBLAYER2.MLCCONTROL, regvalue );

    regvalue  = (U32)((AlphaArgument<<28) | TransparencyColor);
    WriteIO32(&pRegister->MLCRGBLAYER2.MLCTPCOLOR, regvalue );

    regvalue = InverseColor;
    WriteIO32(&pRegister->MLCRGBLAYER2.MLCINVCOLOR, regvalue );

}

//------------------------------------------------------------------------------
/// @brief	set Fourth layer control parameter
//------------------------------------------------------------------------------
void
NX_MLC_SetVideoLayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Curent Layer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,			///< [in] Use Alpha value in no Alpha value format case
    YUVFMT	YUVFormat
)
{
    U32 ControlEnb;
    U32 AlphaArgument;
    U32 regvalue;
	register struct NX_MLC_RegisterSet* pRegister;
    U32 VideoControlReg;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;


	VideoControlReg = ReadIO32(&pRegister ->MLCVIDEOLAYER. MLCCONTROL);

    ControlEnb = (U32)((YUVFormat<<16) | (LayerEnable<<5) | (BlendEnable<<2) | (InvEnable<<1) | TpEnable) & 0x30027;
    AlphaArgument = (U32)(AlphaValue&0xF);

    regvalue = (U32)(ControlEnb | VideoControlReg);
    WriteIO32(&pRegister->MLCVIDEOLAYER.MLCCONTROL, regvalue );

	regvalue = (U32)((AlphaArgument<<28) | TransparencyColor);
    WriteIO32(&pRegister->MLCVIDEOLAYER.MLCTPCOLOR, regvalue );

    regvalue = (U32)((AlphaArgument<<28) | TransparencyColor);
    WriteIO32(&pRegister->MLCVIDEOLAYER.MLCINVCOLOR, regvalue );

}



void NX_MLC_SetSRAMMODE(U32 ModuleIndex, LATYERNAME LayerName, SRAMMODE SramMode)
{

    U32 ControlRegValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    switch (LayerName)
    {
        case TOPMLC :
            ControlRegValue = ReadIO32(&pRegister -> MLCCONTROLT);
            WriteIO32(&pRegister -> MLCCONTROLT, (U32)(ControlRegValue | (SramMode << 10)));
            ControlRegValue = 0;
            break;
        case RGB0 :
            ControlRegValue = ReadIO32(&pRegister -> MLCRGBLAYER[0].MLCCONTROL);
            WriteIO32(&pRegister -> MLCRGBLAYER[0].MLCCONTROL, (U32)(ControlRegValue | (SramMode << 14)));
            ControlRegValue = 0;
            break;
        case RGB1 :
            ControlRegValue = ReadIO32(&pRegister -> MLCRGBLAYER[1].MLCCONTROL);
            WriteIO32(&pRegister -> MLCRGBLAYER[1].MLCCONTROL, (U32)(ControlRegValue | (SramMode << 14)));
            ControlRegValue = 0;
            break;
        case RGB2 :
            ControlRegValue = ReadIO32(&pRegister -> MLCRGBLAYER2.MLCCONTROL);
            WriteIO32(&pRegister -> MLCRGBLAYER2.MLCCONTROL, (U32)(ControlRegValue | (SramMode << 14)));
            ControlRegValue = 0;
            break;
        case VIDEO :
            ControlRegValue = ReadIO32(&pRegister -> MLCVIDEOLAYER.MLCCONTROL);
            WriteIO32(&pRegister -> MLCVIDEOLAYER.MLCCONTROL, (U32)(ControlRegValue | (SramMode << 14)));
            ControlRegValue = 0;
            break;
        default             :   NX_ASSERT(CFALSE); //MES_NEVER_GET_HERE();
    }
}


void
NX_MLC_SetLayerRegFinish( U32 ModuleIndex, LATYERNAME LayerName )
{
    U32 ControlRegValue;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;


    switch (LayerName)
    {
        case TOPMLC :
            ControlRegValue = ReadIO32(&pRegister -> MLCCONTROLT);
            WriteIO32(&pRegister -> MLCCONTROLT, (U32)(ControlRegValue | (1UL << 3)));
            ControlRegValue = 0;
            break;
        case RGB0 :
            ControlRegValue = ReadIO32(&pRegister -> MLCRGBLAYER[0].MLCCONTROL);
            WriteIO32(&pRegister -> MLCRGBLAYER[0].MLCCONTROL, (U32)(ControlRegValue | (1UL << 4)));
            ControlRegValue = 0;
            break;
        case RGB1 :
            ControlRegValue = ReadIO32(&pRegister -> MLCRGBLAYER[1].MLCCONTROL);
            WriteIO32(&pRegister -> MLCRGBLAYER[1].MLCCONTROL, (U32)(ControlRegValue | (1UL << 4)));
            ControlRegValue = 0;
            break;
        case RGB2 :
            ControlRegValue = ReadIO32(&pRegister -> MLCRGBLAYER2.MLCCONTROL);
            WriteIO32(&pRegister -> MLCRGBLAYER2.MLCCONTROL, (U32)(ControlRegValue | (1UL << 4)));
            ControlRegValue = 0;
            break;
		case VIDEO :
            ControlRegValue = ReadIO32(&pRegister -> MLCVIDEOLAYER.MLCCONTROL);
            WriteIO32(&pRegister -> MLCVIDEOLAYER.MLCCONTROL, (U32)(ControlRegValue | (1UL << 4)));
            ControlRegValue = 0;
            break;
        default             :   NX_ASSERT(CFALSE); //MES_NEVER_GET_HERE();
    }
}


// Fourth Layer
//------------------------------------------------------------------------------
/// @brief	set video layer coordinate and scale size in Screen
//------------------------------------------------------------------------------
// HScale = (source_with+(1<<11)) / (right-left)
// VScale = (source_height+(1<<11)) / (bottom-top)
void
NX_MLC_SetVideoLayerCoordinate
(
    U32 ModuleIndex,
    CBOOL VFilterEnable,       ///< [in] VScale enable
    CBOOL HFilterEnable,       ///< [in] HScale enable
    CBOOL VFilterEnable_C,     ///< [in] VScale enable
    CBOOL HFilterEnable_C,     ///< [in] HScale enable
    U16 VideoLayerWith,     ///< [in] Video Layer With
    U16 VideoLayerHeight,   ///< [in] Video Layer Height
    S16 Left,               ///< [in] Layer left coordinate
    S16 Right,              ///< [in] Layer right coordinate
    S16 Top,                ///< [in] Layer top coordinate
    S16 Bottom              ///< [in] Layer bottom coordinate
)
{

    S32 Source_Width, Source_Height;
    S32 Destination_Width;
    S32 Destination_Height;
    S32 HScale, VScale;
    S32 HFilterEnb, VFilterEnb;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    WriteIO32(&pRegister -> MLCVIDEOLAYER.MLCLEFTRIGHT ,(S32)(((Left&0x0FFF)<<16) | (Right&0x0FFF)) );
    WriteIO32(&pRegister -> MLCVIDEOLAYER.MLCTOPBOTTOM, (S32)(((Top&0x0FFF)<<16) | (Bottom&0x0FFF)));

    Source_Width  = (S32)(VideoLayerWith - 1);
    Source_Height = (S32)(VideoLayerHeight - 1);

    Destination_Width   = (S32)(Right - Left);
    Destination_Height  = (S32)(Bottom - Top);


	//===========================================
	//@modified choiyk 2013-01-25 오후 12:38:34
	// 반올림을 추가해본다.
	//===========================================
	//HScale  =   (S32)((Source_Width*(1UL<<11))/Destination_Width);
    //VScale  =   (S32)((Source_Height*(1UL<<11))/Destination_Height);

	HScale  =   (S32)((Source_Width*(1UL<<11)+(Destination_Width/2) )   / Destination_Width  );
    VScale  =   (S32)((Source_Height*(1UL<<11)+(Destination_Height/2) ) / Destination_Height );




	//HFilterEnb = (U32)(((HFilterEnable)<<28) & 0x30000000);
	HFilterEnb = (U32)(((HFilterEnable_C<<29) | (HFilterEnable)<<28)) & 0x30000000;
	VFilterEnb = (U32)(((VFilterEnable_C<<29) | (VFilterEnable)<<28)) & 0x30000000;
	WriteIO32(&pRegister ->  MLCVIDEOLAYER.MLCHSCALE,(U32)(HFilterEnb | (HScale&0x00FFFFFF)));
	WriteIO32(&pRegister ->  MLCVIDEOLAYER.MLCVSCALE,(U32)(VFilterEnb | (VScale&0x00FFFFFF)));
}






void NX_MLC_SetGammaControlParameter
(
	U32 ModuleIndex,
	CBOOL RGBGammaEnb, 		// RGB Layer Gamma table enable
	CBOOL YUVGammaEnb, 		// YUV Layer Gamma table enable
	CBOOL YUVAlphaArray, 	// YUV Layer Alpha Array ( 0:RGB Layer 1:YUV Layer)
	CBOOL DitherEnb			// Gamma value dither(10 -> 8bit)
)
{
	U32 RegisterData;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	RegisterData	= ReadIO32(&pRegister->MLCGAMMACONT);
	RegisterData = (RegisterData & 0xf0c) | ((YUVAlphaArray<<5) | (YUVGammaEnb<<4) | (RGBGammaEnb<<1) | (DitherEnb<<0));
	WriteIO32(&pRegister->MLCGAMMACONT, RegisterData);
}



//---------------
// 8bit Alpha 설정 함수(다른 함수들은 다 4bit를 기준으로 생각한다.)
// Alpha값만 설정한다.
//---------------

void NX_MLC_SetLayerAlpha256(U32 ModuleIndex, U32 Layer, U32 Alpha)
{
	U32 RegisterData;
	register struct NX_MLC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_MLC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0 <= Layer && Layer <= 3 );
	//NX_ASSERT( 0 <= Alpha && Alpha <= 255 );

	if( Alpha < 0 ) Alpha = 0;
	if( Alpha > 255 ) Alpha = 255;


	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	if( Layer == 0 ) // RGB Layer 0
	{
		RegisterData = ReadIO32(&pRegister->MLCRGBLAYER[0].MLCTPCOLOR) & 0x00ffffff;
		RegisterData = RegisterData | (Alpha<<24);
		WriteIO32(&pRegister->MLCRGBLAYER[0].MLCTPCOLOR, RegisterData);

	}
	else if ( Layer == 1 ) // RGB Layer 1
	{
		RegisterData = ReadIO32(&pRegister->MLCRGBLAYER[1].MLCTPCOLOR) & 0x00ffffff;
		RegisterData = RegisterData | (Alpha<<24);
		WriteIO32(&pRegister->MLCRGBLAYER[1].MLCTPCOLOR, RegisterData);
	}
	else if ( Layer == 2 ) // RGB Layer 2
	{
		RegisterData = ReadIO32(&pRegister->MLCRGBLAYER[1].MLCTPCOLOR) & 0x00ffffff;
		RegisterData = RegisterData | (Alpha<<24);
		WriteIO32(&pRegister->MLCRGBLAYER2.MLCTPCOLOR, RegisterData);
	}
	else // Video Layer
	{
		RegisterData = ReadIO32(&pRegister->MLCVIDEOLAYER.MLCTPCOLOR) & 0x00ffffff;
		RegisterData = RegisterData | (Alpha<<24);
		WriteIO32(&pRegister->MLCVIDEOLAYER.MLCTPCOLOR, RegisterData);
	}

}


