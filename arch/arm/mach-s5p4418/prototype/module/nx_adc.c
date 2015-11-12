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
//	File		: nx_adc.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_adc.h"

static	struct NX_ADC_RegisterSet *__g_pRegister[NUMBER_OF_ADC_MODULE];

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicate that Initialize is successed.
 *			CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_ADC_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_ADC_MODULE; i++)
			__g_pRegister[i]= (struct NX_ADC_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. 
 *				It is equal to NUMBER_OF_ADC_MODULE in <nx_chip.h>.
 */
U32		NX_ADC_GetNumberOfModule( void )
{
	return NUMBER_OF_ADC_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_ADC_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_ADC_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_ADC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	__g_pRegister[ModuleIndex] = (struct NX_ADC_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_ADC_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_ADC?_MODULE in <nx_chip.h>.
 */
U32		NX_ADC_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_ADC_MODULE] =
	{
		PHY_BASEADDR_LIST( ADC )
	};
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	return	PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. 
 *				CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_ADC_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicate that Deinitialize is successed. 
 *				CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_ADC_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicate that Module is Busy. 
 *				CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_ADC_CheckBusy( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet* pRegister;
	const U32	ADEN_BITPOS	=	0;
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (CBOOL)((ReadIO32(&pRegister->ADCCON) >> ADEN_BITPOS) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_ADC?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_ADC_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( ADC, nRST )
	};
	NX_CASSERT( NUMBER_OF_ADC_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_ADC?_MODULE in <nx_chip.h>.
 */
U32 	NX_ADC_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( ADC ) };
    NX_CASSERT( NUMBER_OF_ADC_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_ADC0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 *	@param[in]	Enable	Set as CTRUE to enable a specified interrupt. \r
 *						Set as CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_ADC_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = ReadIO32(&pRegister->ADCINTENB);

	regvalue &=	~( 1UL << IntNum );
	regvalue |= (U32)Enable << IntNum;

	WriteIO32(&pRegister->ADCINTENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 *	@return		CTRUE	indicates that a specified interrupt is enabled. \r
 *				CFALSE	indicates that a specified interrupt is disabled.
 */
CBOOL	NX_ADC_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_ADC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (CBOOL)( ( ReadIO32(&pRegister->ADCINTENB) >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 *	@return		CTRUE	indicates that a specified interrupt is pended. \r
 *				CFALSE	indicates that a specified interrupt is not pended.
 */
CBOOL	NX_ADC_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->ADCINTENB);
	regvalue &= ReadIO32(&pRegister->ADCINTCLR);

	return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 *	@return		None.
 */
void	NX_ADC_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_ADC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->ADCINTCLR, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	Set as CTRUE to enable all interrupts. \r
 *						Set as CFALSE to disable all interrupts.
 *	@return		None.
 */
void	NX_ADC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = Enable ? 0xFFFFFFFF : 0 ;

	WriteIO32(&pRegister->ADCINTENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	indicates that one or more interrupts are enabled. \r
 *				CFALSE	indicates that all interrupts are disabled.
 */
CBOOL	NX_ADC_GetInterruptEnableAll( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (CBOOL)(0!=( ReadIO32(&pRegister->ADCINTENB) ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	indicates that one or more interrupts are pended. \r
 *				CFALSE	indicates that no interrupt is pended.
 */
CBOOL	NX_ADC_GetInterruptPendingAll( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->ADCINTENB);
	regvalue &= ReadIO32(&pRegister->ADCINTCLR);

	return (CBOOL)( 0 != ( regvalue ) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 */
void	NX_ADC_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->ADCINTCLR, 0xFFFFFFFF);	// just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 */
S32		NX_ADC_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->ADCINTENB);
	regvalue &= ReadIO32(&pRegister->ADCINTCLR);

	if( 0!=regvalue )
	{
		return 0; // ADC has only one interrupt source
	}
	return -1;
}

//------------------------------------------------------------------------------
//
//	ADC Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ADC Operation.
//------------------------------------------------------------------------------
/**
 *	@brief		Set Clock Prescaler Value of A/D Converter
 *	@param[in]	value		Value of Prescaler ( Range 20 ~ 256 )
 *	@return		None.
 *	@remarks	This Function must be set before SetPrescalerEnable( ) Function.
 *				Max ADC Clock is 2.5Mhz(400ns) when Prescaler Value is 256.
 *	@code
 *				NX_ADC_SetPrescalerValue(0x256);
 *				NX_ADC_SetPrescalerEnable(CTRUE);
 *	@endcode
 */
void	NX_ADC_SetPrescalerValue( U32 ModuleIndex, U32 value )
{
	const U32	APSV_MASK	= ( 0xFFUL << 6 );
	const U32	APSV_BITPOS = 6;

	register	struct NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (256 >= value) && (20 <= value) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue	=	ReadIO32(&pRegister->ADCCON);

	regvalue = ( regvalue & ~APSV_MASK ) | ( (value-1) << APSV_BITPOS ) ;

	WriteIO16(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Prescaler Value of A/D Converter
 *	@return		Value of Prescaler
 */
U32		NX_ADC_GetPrescalerValue( U32 ModuleIndex )
{
	register	struct NX_ADC_RegisterSet*	pRegister;
	const	U32	APSV_MASK	= ( 0xFF << 6 );
	const	U32	APSV_BITPOS = 6;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (U32)((( ReadIO32(&pRegister->ADCCON) & APSV_MASK ) >> APSV_BITPOS ) + 1 ) ;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Prescaler Enable
 *	@param[in]	enable	CTRUE indicate that Prescaler Enable. 
 *						CFALSE indicate that Prescaler Disable.
 *	@return		None.
 *	@remarks	This function is set after SetPrescalerValue() function
 *	@code
 *				NX_ADC_SetPrescalerValue(256);
 *				NX_ADC_SetPrescalerEnable(CTRUE);
 *	@endcode
 */
void	NX_ADC_SetPrescalerEnable( U32 ModuleIndex, CBOOL enable )
{
	const	U32	APEN_MASK	=	( 0x01UL << 14 );
	const	U32	APEN_POS	=	14;
	register U32	regvalue;
	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (0==enable) || (1==enable) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->ADCCON);

	regvalue &= ~APEN_MASK;
	regvalue |= (U32)enable << APEN_POS;

	WriteIO16(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check Prescaler is enabled or disabled
 *	@return		CTRUE indicate that Prescaler is Enabled.
 *				CFALSE indicate that Prescaler is Disabled.
 */
CBOOL	NX_ADC_GetPrescalerEnable( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet*	pRegister;
	const	U32	APEN_POS	=	14;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (CBOOL)( ( ReadIO32(&pRegister->ADCCON) >> APEN_POS ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Input Channel
 *	@param[in]	channel		Value of Input Channel ( 0 ~ 7 )
 *	@return		None.
 */
void	NX_ADC_SetInputChannel( U32 ModuleIndex, U32 channel )
{
	const	U32	ASEL_MASK	=	( 0x07 << 3 );
	const	U32	ASEL_BITPOS	=	3;
	register struct NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( NUMBER_OF_ADC_CHANNEL > channel );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue	=	ReadIO32(&pRegister->ADCCON);
	regvalue	=	( regvalue & ~ASEL_MASK ) | ( channel << ASEL_BITPOS );

	WriteIO16(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Input Channel
 *	@return		Value of Input Channel ( 0 ~ 7 )
 */
U32		NX_ADC_GetInputChannel( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet*	pRegister;
	const	U32	ASEL_MASK	=	( 0x07 << 3 );
	const	U32	ASEL_BITPOS	=	3;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (U32)( ( ReadIO32(&pRegister->ADCCON) & ASEL_MASK ) >> ASEL_BITPOS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Standby Mode
 *	@param[in]	enable	CTRUE indicate that Standby Mode ON. 
 *						CFALSE indicate that Standby Mode OFF.
 *	@return		None.
 *	@remark		If Standby Mode is enabled then ADC power is cut offed.
 *				You have to disable the standby mode to use ADC.
 */
void	NX_ADC_SetStandbyMode( U32 ModuleIndex, CBOOL enable )
{
	const	U32 STBY_MASK	= ( 0x01UL << 2 );
	const	U32 STBY_POS	= 2;
	register U32 regvalue;
	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (0==enable) || (1==enable) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->ADCCON);

	regvalue &= ~STBY_MASK;
	regvalue |= (U32)enable << STBY_POS;

	WriteIO16(&pRegister->ADCCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ADC's Standby Mode
 *	@return		CTRUE indicate that Standby Mode is Enabled.
 *				CFALSE indicate that Standby Mode is Disabled.
 */
CBOOL	NX_ADC_GetStandbyMode( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet*	pRegister;
	const	U32 STBY_MASK	= ( 0x01UL << 2 );
	const	U32 STBY_POS	= 2;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (CBOOL)( ( ReadIO32(&pRegister->ADCCON) & STBY_MASK ) >> STBY_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		ADC Start
 *	@return		None.
 *	@remarks	Sequence of ADC operation
 *	@code
 *		NX_ADC_SetStandbyMode( CFALSE );		// Standby mode disable
 *		...
 *		NX_ADC_SetPrescalerValue( 256 );		// Set prescaler value ( 20 ~ 256 )
 *		NX_ADC_SetPrescalerEnable( CTRUE );		// Prescaler enable
 *		NX_ADC_SetInputChannel( 0 );			// Set input channel
 *		...
 *		NX_ADC_Start();							// Start ADC converting
 *		...
 *		while( NX_ADC_IsBusy() ){ }				// Wait during ADC converting
 *		...
 *		Value = NX_ADC_GetConvertedData();		// Read Converted ADC data
 *
 *	@endcode
 */
void	NX_ADC_Start( U32 ModuleIndex )
{
	const	U32	ADEN_MASK = ( 0x01 << 0 );
	register U32	regvalue;
	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->ADCCON);

	regvalue |= ADEN_MASK;

	WriteIO16(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check ADC's operation
 *	@return		CTRUE indicate that ADC is Busy. 
 *				CFALSE indicate that ADC Conversion is ended.
 */
CBOOL	NX_ADC_IsBusy( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet*	pRegister;
	const	U32	ADEN_MASK = (0x01 << 0);

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (CBOOL)( ReadIO32(&pRegister->ADCCON) & ADEN_MASK );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Conversioned Data of ADC
 *	@return		10bit Data of ADC
 */
U32		NX_ADC_GetConvertedData( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (U32)( ReadIO32(&pRegister->ADCDAT) );
}

