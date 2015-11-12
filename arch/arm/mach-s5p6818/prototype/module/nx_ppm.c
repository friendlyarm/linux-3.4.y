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
//	Module		:
//	File               : nx_ppm.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------

#include <nx_chip.h>
#include "nx_ppm.h"

static	NX_PPM_RegisterSet *__g_pRegister[NUMBER_OF_PPM_MODULE];

//------------------------------------------------------------------------------
//
//	PPM Interface
//
//------------------------------------------------------------------------------

/**
 *	@brief		Set PPM Enable or Disable
 *	@param[in]	enable		CTRUE( PPM Enable )CFALSE( PPM Disable )
 *	@return		None.
 */
void	NX_PPM_SetPPMEnable
(
	U32 ModuleIndex,
	CBOOL enable
)
{
	const U32 ENB_MASK	= ( 0x01 << 15 );
	register U32 temp;

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	temp = __g_pRegister[ModuleIndex]->PPM_CTRL;

	if( CTRUE == enable )
	{
		temp |= ENB_MASK;
	}
	else
	{
		temp &= ~ENB_MASK;
	}

	WriteIO32(&__g_pRegister[ModuleIndex]->PPM_CTRL, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get State of PPM is Enable or Disable
 *	@return		CTRUE	indicate that PPM is Enabled.
 *				CFALSE	indicate that PPM is Disabled.
 */

CBOOL	NX_PPM_GetPPMEnable ( U32 ModuleIndex )
{
	const U32 ENB_MASK	= ( 0x01 << 15 );

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( __g_pRegister[ModuleIndex]->PPM_CTRL & ENB_MASK )
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
 *	@brief		Set Polarity of PPM Input Signal
 *	@param[in]	polarity		Set Polarity ( Invert or Bypass )
 *	@return		None.
 */
void	NX_PPM_SetInputSignalPolarity
(
	U32 ModuleIndex,
	NX_PPM_INPUTPOL polarity
)
{
	const U32 POL_BITPOS	=	14;
	const U32 POL_MASK		=	( 0x01 << POL_BITPOS );

	U32 temp;

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( NX_PPM_INPUTPOL_BYPASS >= polarity );

	temp = __g_pRegister[ModuleIndex]->PPM_CTRL;

	if( NX_PPM_INPUTPOL_INVERT == polarity )
	{
		temp &= ~POL_MASK;
	}
	else if( NX_PPM_INPUTPOL_BYPASS == polarity )
	{
		temp |= POL_MASK;
	}

	WriteIO32(&__g_pRegister[ModuleIndex]->PPM_CTRL, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Polarity of PPM Input Signal
 *	@return		Value of input signal's polarity ( Invert or Bypass )
 */

NX_PPM_INPUTPOL	NX_PPM_GetInputSignalPolarity( U32 ModuleIndex )
{
	const U32 POL_MASK	= ( 0x01 << 14 );

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( __g_pRegister[ModuleIndex]->PPM_CTRL & POL_MASK )
	{
		return NX_PPM_INPUTPOL_BYPASS;
	}
	else
	{
		return NX_PPM_INPUTPOL_INVERT;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Overflow State In High Period
 *	@return		CTRUE indicate that High period value is overflowed.
 *				CFALSE indicate that High period value is NOT overflowed.
 *	@remarks	Check overflow state ( high period's value ) when Overflow interrupt is occurred.
 */
CBOOL	NX_PPM_IsHighOverflow( U32 ModuleIndex )
{
	const U32	HOVER_MASK = ( 0x01 << 3 );

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( __g_pRegister[ModuleIndex]->PPM_STAT & HOVER_MASK )
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
 *	@brief		Get Overflow State In Low Period
 *	@return		CTRUE indicate that Low period value is overflowed.
 *				CFALSE indicate that Low period value is NOT overflowed.
 *	@remarks	Check overflow state ( low period's value ) when Overflow interrupt is occured.
 */

CBOOL	NX_PPM_IsLowOverflow( U32 ModuleIndex )
{
	const U32	LOVER_MASK = ( 0x01 << 4 );

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( __g_pRegister[ModuleIndex]->PPM_STAT & LOVER_MASK )
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
 *	@brief		Read Count Value of Low Period
 *	@return		Value of Low period ( 0 ~ 0xFFFF )
 */
U32		NX_PPM_GetPPMLowPeriodValue ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return(U32)( __g_pRegister[ModuleIndex]->PPM_LOWPERIOD );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Read Count Value of High Period
 *	@return		Value of High period ( 0 ~ 0xFFFF )
 */
U32		NX_PPM_GetPPMHighPeriodValue ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return(U32)( __g_pRegister[ModuleIndex]->PPM_HIGHPERIOD );
}

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicate that Initialize is successed.
 *			 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_PPM_Initialize( void )
{
	static CBOOL bInit = CFALSE;
    int i = 0;

	if( CFALSE == bInit )
	{
	    for(i = 0; i < NUMBER_OF_PPM_MODULE; i++)
            __g_pRegister[i] = 0;
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. 
 *				It is equal to NUMBER_OF_PPM_MODULE in <nx_chip.h>.
 *	@see		NX_PPM_Initialize
 */
U32		NX_PPM_GetNumberOfModule( void )
{
	return NUMBER_OF_PPM_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_PPM_GetPhysicalAddress,
 *				NX_PPM_SetBaseAddress,			NX_PPM_GetBaseAddress,
 *				NX_PPM_OpenModule,				NX_PPM_CloseModule,
 *				NX_PPM_CheckBusy,
 */
U32		NX_PPM_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_PPM_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_PPM_GetPhysicalAddress,		NX_PPM_GetSizeOfRegisterSet,
 *				NX_PPM_GetBaseAddress,
 *				NX_PPM_OpenModule,				NX_PPM_CloseModule,
 *				NX_PPM_CheckBusy,
 */
void	NX_PPM_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_PPM_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_PPM_GetPhysicalAddress,		NX_PPM_GetSizeOfRegisterSet,
 *				NX_PPM_SetBaseAddress,
 *				NX_PPM_OpenModule,				NX_PPM_CloseModule,
 *				NX_PPM_CheckBusy,
 */
void*	NX_PPM_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_PPM?_MODULE in <nx_chip.h>.
 *	@see		NX_PPM_GetSizeOfRegisterSet,
 *				NX_PPM_SetBaseAddress,			NX_PPM_GetBaseAddress,
 *				NX_PPM_OpenModule,				NX_PPM_CloseModule,
 *				NX_PPM_CheckBusy,
 */
U32		NX_PPM_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_PPM_MODULE] =
	{
		PHY_BASEADDR_LIST( PPM )
	};
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 *	@see		NX_PPM_GetPhysicalAddress,		NX_PPM_GetSizeOfRegisterSet,
 *				NX_PPM_SetBaseAddress,			NX_PPM_GetBaseAddress,
 *				NX_PPM_CloseModule,
 *				NX_PPM_CheckBusy,
 */
CBOOL	NX_PPM_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    
	WriteIO32(&__g_pRegister[ModuleIndex]->PPM_CTRL, 0x4000);
	WriteIO32(&__g_pRegister[ModuleIndex]->PPM_STAT, 0x0007);
    
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_PPM_GetPhysicalAddress,		NX_PPM_GetSizeOfRegisterSet,
 *				NX_PPM_SetBaseAddress,			NX_PPM_GetBaseAddress,
 *				NX_PPM_OpenModule,
 *				NX_PPM_CheckBusy,
 */
CBOOL	NX_PPM_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    
	WriteIO32(&__g_pRegister[ModuleIndex]->PPM_CTRL, 0x4000);
	WriteIO32(&__g_pRegister[ModuleIndex]->PPM_STAT, 0x0007);
    
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_PPM_GetPhysicalAddress,		NX_PPM_GetSizeOfRegisterSet,
 *				NX_PPM_SetBaseAddress,			NX_PPM_GetBaseAddress,
 *				NX_PPM_OpenModule,				NX_PPM_CloseModule,
 */
CBOOL	NX_PPM_CheckBusy( U32 ModuleIndex )
{
	const U16 PPMENB_BITPOS = 15;

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( __g_pRegister[ModuleIndex]->PPM_CTRL & ( 1 << PPMENB_BITPOS ) )
	{
		return CFALSE;
	}
	else
	{
		return CFALSE;
	}
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_PPM?_MODULE in <nx_chip.h>.
 */
U32 NX_PPM_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( PPM )
	};
	NX_CASSERT( NUMBER_OF_PPM_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_PPM?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_PPM_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( PPM, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_PPM_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_PPM?_MODULE in <nx_chip.h>.
 */
U32 	NX_PPM_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( PPM ) };
    NX_CASSERT( NUMBER_OF_PPM_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_PPM0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_PPM_INTCH_xxx in <nx_ppm.h>
 *	@param[in]	Enable	 Set as CTRUE to enable a specified interrupt. 
 *						 Set as CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_PPM_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register NX_PPM_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->PPM_CTRL;

	regvalue &=	~( 1UL << IntNum );
	regvalue |= (U32)Enable << IntNum;

	WriteIO32(&pRegister->PPM_CTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_PPM_INTCH_xxx in <nx_ppm.h>
 *	@return		 CTRUE	indicates that a specified interrupt is enabled. 
 *				 CFALSE	indicates that a specified interrupt is disabled.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_ClearInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
CBOOL	NX_PPM_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)( ( __g_pRegister[ModuleIndex]->PPM_CTRL >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_PPM_INTCH_xxx in <nx_ppm.h>
 *	@return		 CTRUE	indicates that a specified interrupt is pended. 
 *				 CFALSE	indicates that a specified interrupt is not pended.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_ClearInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
CBOOL	NX_PPM_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_PPM_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->PPM_CTRL;
	regvalue &= pRegister->PPM_STAT;
	return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.
 *						refer to NX_PPM_INTCH_xxx in <nx_ppm.h>
 *	@return		None.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_ClearInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
void	NX_PPM_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_PPM_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->PPM_STAT, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 Set as CTRUE to enable all interrupts. 
 *						 Set as CFALSE to disable all interrupts.
 *	@return		None.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_ClearInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
void	NX_PPM_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register NX_PPM_RegisterSet* pRegister;
	register U32	regvalue;
	const U32 ENB_MASK	= 0x07;

	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->PPM_CTRL;
	regvalue &=	~ENB_MASK;
	regvalue  = Enable ? (regvalue | ENB_MASK) : regvalue ;

	WriteIO32(&pRegister->PPM_CTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are enabled. 
 *				 CFALSE	indicates that all interrupts are disabled.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_ClearInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
CBOOL	NX_PPM_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->PPM_CTRL & 7 ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are pended. 
 *				 CFALSE	indicates that no interrupt is pended.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_ClearInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
CBOOL	NX_PPM_GetInterruptPendingAll( U32 ModuleIndex )
{
	register NX_PPM_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->PPM_CTRL;
	regvalue &= pRegister->PPM_STAT;
	return (CBOOL)( 0 != ( regvalue & 7 ) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_GetInterruptPendingNumber

 */
void	NX_PPM_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register NX_PPM_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->PPM_STAT, 7);	// just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_PPM_INTCH_xxx in <nx_ppm.h>
 *	@see		NX_PPM_GetInterruptNumber,
 *				NX_PPM_SetInterruptEnable,
 *				NX_PPM_GetInterruptEnable,
 *				NX_PPM_GetInterruptPending,
 *				NX_PPM_ClearInterruptPending,
 *				NX_PPM_SetInterruptEnableAll,
 *				NX_PPM_GetInterruptEnableAll,
 *				NX_PPM_GetInterruptPendingAll,
 *				NX_PPM_ClearInterruptPendingAll

 */
S32		NX_PPM_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	int i;
	register NX_PPM_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PPM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->PPM_CTRL;
	regvalue &= pRegister->PPM_STAT;
	if( 0!=regvalue )
	{
		//for( i=0; i<3; i++ )	==> overflow pending 먼저 check한다.
		//{
		//	if( regvalue & 1UL )
		//	{
		//		return i;
		//	}
		//	regvalue>>=1;
		//}
		for( i=2; i>=0; i-- )
		{
			if( (regvalue>>i) & 1UL )
			{
				return i;
			}
		}
	}
	return -1;
}

