//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2014
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: ADC
//	File			: nx_adc.c
//	Description	: 
//	Author		: Deoks
//	History		: 2014.10.13 First implementation. (Deoks)
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_adc.h"

static	struct
{
	struct NX_ADC_RegisterSet *pRegister;
} __g_ModuleVariables[NUMBER_OF_ADC_MODULE] = { {CNULL,}, };


//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	Indicate that Initialize is successed.
 *			CFALSE	Indicate that Initialize is failed.
 */
CBOOL	NX_ADC_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32	i;

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_ADC_MODULE; i++)
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
 *				It is equal to NUMBER_OF_ADC_MODULE in <nx_chip.h>.
 *	@see		NX_ADC_Initialize
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
	__g_ModuleVariables[ModuleIndex].pRegister = ( struct NX_ADC_RegisterSet *)BaseAddress;	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.				
 */

void*	NX_ADC_GetBaseAddress( U32 ModuleIndex )


{
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. \n
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
 *	@return		CTRUE		Indicate that Initialize is successed. 
 *				CFALSE		Indicate that Initialize is failed.			
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
 *	@return		CTRUE		Indicate that Deinitialize is successed. 
 *				CFALSE		Indicate that Deinitialize is failed.
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
 *	@return		CTRUE		Indicate that Module is Busy. 
 *				CFALSE		Indicate that Module is NOT Busy.
 */
CBOOL	NX_ADC_CheckBusy( U32 ModuleIndex )
{
	const U32	ADEN_BITPOS	=	0;
	
	register struct NX_ADC_RegisterSet* pRegister;
	
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );

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

    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum			a interrupt index number .
 *	@param[in]	Enable			CTRUE  to enable a specified interrupt. 
 *								CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_ADC_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;
	
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (0==Enable) || (1==Enable) );

	regvalue  = ReadIO32(&pRegister->ADCINTENB);

	regvalue &=	~( 1UL << IntNum );
	regvalue |= (U32)Enable << IntNum;

	WriteIO32(&pRegister->ADCINTENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt index number.
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 *	@return		CTRUE	Indicates that a specified interrupt is enabled.
 *				CFALSE	Indicates that a specified interrupt is disabled.
 */
CBOOL	NX_ADC_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_ADC_RegisterSet* pRegister;	

	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)( ( ReadIO32(&pRegister->ADCINTENB) >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.\n
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 *	@return		CTRUE	Indicates that a specified interrupt is pended.
 *				CFALSE	Indicates that a specified interrupt is not pended.
 */
CBOOL	NX_ADC_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;	

	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue  = ReadIO32(&pRegister->ADCINTENB);
	regvalue &= ReadIO32(&pRegister->ADCINTCLR);	
	
	return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.\n
 *						refer to NX_ADC_INTCH_xxx in <nx_adc.h> 
 *	@return		None.
 */
void	NX_ADC_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_ADC_RegisterSet* pRegister;
	
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
	WriteIO32(&pRegister->ADCINTCLR, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	Set as CTRUE to enable all interrupts. 
 *						Set as CFALSE to disable all interrupts.
 *	@return		None.
 */
void	NX_ADC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;
	
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (0==Enable) || (1==Enable) );

	regvalue = ReadIO32(&pRegister->ADCINTENB);
	regvalue = Enable ? 0xFFFFFFFF : 0 ;

	WriteIO32(&pRegister->ADCINTENB, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	Indicates that one or more interrupts are enabled. 
 *				CFALSE	Indicates that all interrupts are disabled.
 */
CBOOL	NX_ADC_GetInterruptEnableAll( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet* pRegister;
	
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (CBOOL)(0!=( ReadIO32(&pRegister->ADCINTENB) ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	Indicates that one or more interrupts are pended. 
 *				CFALSE	Indicates that no interrupt is pended.
 */
CBOOL	NX_ADC_GetInterruptPendingAll( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
	regvalue  = ReadIO32(&pRegister->ADCINTENB);
	regvalue &= pRegister->ADCINTCLR;	
	
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

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
	WriteIO32(&pRegister->ADCINTCLR, 0xFFFFFFFF);	// just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.\n
 *				refer to NX_ADC_INTCH_xxx in <nx_adc.h>
 */
S32		NX_ADC_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	register struct NX_ADC_RegisterSet* pRegister;
	register U32	regvalue;

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != pRegister );
	
	regvalue  = ReadIO32(&pRegister->ADCINTENB);
	regvalue &= ReadIO32(&pRegister->ADCINTCLR);		
	
	if( 0!=regvalue )
	{
		return 0; // ADC has only one interrupt source
	}
	return -1;
}

//------------------------------------------------------------------------------
//	PAD Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get number of PAD mode
 *	@param[in]	ModuleIndex	an index of module.
 *	@return		number of PAD mode.
 *	@see		NX_ADC_EnablePAD
 */
U32 NX_ADC_GetNumberOfPADMode ( U32 ModuleIndex )
{
	NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
	return 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable PAD for a module. 
 *	@param[in]	ModuleIndex	an index of module.
 *	@see		NX_ADC_GetNumberOfPADMode
 */
U32 NX_ADC_EnablePAD ( U32 ModuleIndex, U32 ModeIndex )
{
	U32 i;
	const U32 PADNumber[][NUMBER_OF_ADC_MODULE] =	{
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_0_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_1_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_2_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_3_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_4_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_5_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_6_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, AIN_7_ ) },
	     { PADINDEX_WITH_CHANNEL_LIST( ADC, VREF   ) },
	};
	NX_CASSERT( NUMBER_OF_ADC_MODULE == (sizeof(PADNumber[0])/sizeof(PADNumber[0][0])) ); 
	NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	NX_DISABLE_UNUSED_VAR_WARNING(ModeIndex);
	
	for(i=0; i<sizeof( PADNumber)/sizeof(PADNumber[0]); i++)
	{
		//NX_SWITCHDEVICE_Set_Switch_Enable ( PADNumber[i][ModuleIndex] );
		//NX_PAD_SetPadFunctionEnable       ( PADNumber[i][ModuleIndex] );		
	}
	return 0;
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
 *	@remarks	This Function must be set before SetPrescalerEnable( ) Function.\n
 *				Max ADC Clock is 2.5Mhz(400ns) when Prescaler Value is 256.
 */
void	NX_ADC_SetPrescalerValue( U32 ModuleIndex, U32 Value )
{
	const U32	APSV_POS 	= 0;	
	const U32	APSV_MASK	= ( 0x3FFUL << APSV_POS );

	register struct	NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (256 >= Value) && (20 <= Value) );

	regvalue	 =	ReadIO32(&pRegister->ADCPRESCALE);

	regvalue 	&= ~APSV_MASK;
	regvalue	 = ( (Value-1) << APSV_POS );

	WriteIO32(&pRegister->ADCPRESCALE, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Prescaler Value of A/D Converter
 *	@return		Value of Prescaler ( Range 20 ~ 256 )
 */
U8		NX_ADC_GetPrescalerValue( U32 ModuleIndex )
{
	const	U32	APSV_POS 	= 0;	
	const	U32	APSV_MASK	= ( 0xFF << APSV_POS );

	register struct	NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U8)((( ReadIO32(&pRegister->ADCPRESCALE) & APSV_MASK ) >> APSV_POS ) ) ;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Prescaler Enable
 *	@param[in]	Enable	CTRUE Indicate that Prescaler Enable. \n
 *						CFALSE Indicate that Prescaler Disable.
 *	@return		None.
 *	@remarks	This function is set after SetPrescalerValue() function
 */
void	NX_ADC_SetPrescalerEnable( U32 ModuleIndex, CBOOL Enable )
{
	const	U32	APEN_POS	=	15;	
	const	U32	APEN_MASK	=	( 1UL << APEN_POS );

	register struct	NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (0==Enable) || (1==Enable) );

	regvalue = ReadIO32(&pRegister->ADCPRESCALE);

	regvalue &= ~APEN_MASK;
	regvalue |= Enable << APEN_POS;

	WriteIO32(&pRegister->ADCPRESCALE, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check Prescaler is enabled or disabled
 *	@return		CTRUE	 Indicate that Prescaler is Enabled.
 *				CFALSE	 Indicate that Prescaler is Disabled.
 */
CBOOL	NX_ADC_GetPrescalerEnable( U32 ModuleIndex )
{
	const	U32	APEN_POS	= 15;
	const	U32 APEN_MASK	= 1UL << APEN_POS;

	register struct	NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)( ( ReadIO32(&pRegister->ADCPRESCALE) & APEN_MASK ) >> APEN_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set ADC Data Delay 
 *	@param[in]	Data Delay (0:5CLK, 1: 4CLK, 2:3CLK, 3:2CLK, 4:1CLK, 5:No Delay )
 */
void	NX_ADC_SetADCDataDelay( U32 ModuleIndex, NX_ADC_DATA_DELAY Delay )
{
	const U32	ADC_DATA_DELAY_BITPOS = 10;	
	const U32	ADC_DATA_DELAY_MASK	  = ( 0xF << ADC_DATA_DELAY_BITPOS );

	register struct	NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (5 >= Delay) );

	regvalue   = ReadIO32(&pRegister->ADCCON);

	regvalue  &= ~ADC_DATA_DELAY_MASK;
	regvalue   = Delay << ADC_DATA_DELAY_BITPOS;

	WriteIO32(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ADC Data Delay 
 *	@return		Data Delay (0:5CLK, 1: 4CLK, 2:3CLK, 3:2CLK, 4:1CLK, 5:No Delay )
 */
NX_ADC_DATA_DELAY	NX_ADC_GetADCDataDelay( U32 ModuleIndex )
{
	const U32	ADC_DATA_DELAY_BITPOS = 10;	
	const U32	ADC_DATA_DELAY_MASK	  = ( 0xF << ADC_DATA_DELAY_BITPOS );

	register struct	NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (NX_ADC_DATA_DELAY)(ReadIO32(&pRegister->ADCCON) & ADC_DATA_DELAY_MASK ) >> ADC_DATA_DELAY_BITPOS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Start of Conversion Delay
 *	@paran[in]	Delay value ( Range : 0 ~ 7 ) 
 *	@return		None.
 */
void	NX_ADC_SetSOCDelay( U32 ModuleIndex, U32 Value )
{
	const U32	SOC_DELAY_BITPOS = 6;	
	const U32	SOC_DELAY_MASK	= ( 0x7UL << SOC_DELAY_BITPOS );

	register struct	NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;		
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (7 >= Value) );

	regvalue	 = ReadIO32(&pRegister->ADCCON);

	regvalue 	&= ~SOC_DELAY_MASK;
	regvalue	 = Value << SOC_DELAY_BITPOS;

	WriteIO32(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Start of Conversion Delay
 *	@return		Delay value ( Range : 0 ~ 7 ) 
 */
U8		NX_ADC_GetSOCDelay( U32 ModuleIndex )
{
	const U32	SOC_DELAY_BITPOS = 6;	
	const U32	SOC_DELAY_MASK	= ( 0x7UL << SOC_DELAY_BITPOS );

	register struct	NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );

	return (U8)((( ReadIO32(&pRegister->ADCCON) & SOC_DELAY_MASK ) >> SOC_DELAY_BITPOS ) ) ;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Input Channel
 *	@param[in]	channel		Value of Input Channel ( 0 ~ 4 )
 *	@return		None.
 */
void	NX_ADC_SetInputChannel( U32 ModuleIndex, U32 Channel )
{
	const	U32	ASEL_BITPOS	=	3;	
	const	U32	ASEL_MASK	=	( 0x07 << ASEL_BITPOS );
	
	register struct NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (7 > Channel)  );

	regvalue	 = ReadIO32(&pRegister->ADCCON);

	regvalue	&= ~ASEL_MASK;
	regvalue	|= Channel << ASEL_BITPOS;

	WriteIO32(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Input Channel
 *	@return		Value of Input Channel ( 0 ~ 4 )
 */
U32		NX_ADC_GetInputChannel( U32 ModuleIndex )
{
	const	U32	ASEL_BITPOS	=	3;	
	const	U32	ASEL_MASK	=	( 0x07 << ASEL_BITPOS );

	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );

	return (U32)( ( ReadIO32(&pRegister->ADCCON) & ASEL_MASK ) >> ASEL_BITPOS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Standby Mode
 *	@param[in]	enable	CTRUE  Indicate that Standby Mode ON.
 *						CFALSE Indicate that Standby Mode OFF.
 *	@return		None.
 *	@remark	If Standby Mode is enabled then ADC power is cut offed.
 *				You have to disable the standby mode to use ADC.
 */
void	NX_ADC_SetStandbyMode( U32 ModuleIndex, CBOOL Enable )
{
	const	U32 STBY_POS	= 2;	
	const	U32 STBY_MASK	= ( 0x01UL << STBY_POS );
	
	register U32 regvalue;
	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (0==Enable) || (1==Enable) );

	regvalue  = ReadIO32(&pRegister->ADCCON);

	regvalue &= ~STBY_MASK;
	regvalue |= Enable << STBY_POS;

	WriteIO32(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ADC's Standby Mode
 *	@return		CTRUE  Indicate that Standby Mode ON.
 *				CFALSE Indicate that Standby Mode OFF.
 */
CBOOL	NX_ADC_GetStandbyMode( U32 ModuleIndex )
{
	const	U32 STBY_POS	= 2;	
	const	U32 STBY_MASK	= ( 0x01UL << STBY_POS );

	register struct NX_ADC_RegisterSet*	pRegister;

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	return (CBOOL)( ( ReadIO32(&pRegister->ADCCON) & STBY_MASK ) >> STBY_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		ADC Start
 *	@return		None.
 *	@remarks	Sequence of ADC operation
 */
void	NX_ADC_Start( U32 ModuleIndex )
{
	const	 U32	ADEN_POS  = 0;
	const	 U32	ADEN_MASK = ( 0x01 << ADEN_POS );

	register struct NX_ADC_RegisterSet*	pRegister;
	register U32	regvalue;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );
	
	regvalue = pRegister->ADCCON;

	regvalue |= ADEN_MASK;

	WriteIO32(&pRegister->ADCCON, (U16)regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check ADC's operation
 *	@return		CTRUE Indicate that ADC is Busy.
 *				CFALSE Indicate that ADC Conversion is ended.
 */
CBOOL	NX_ADC_IsBusy( U32 ModuleIndex )
{
	const	U32	ADEN_POS  = 0;
	const	U32	ADEN_MASK = (0x01 << ADEN_POS);

	register struct NX_ADC_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;	
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)( ReadIO32(&pRegister->ADCCON) >> ADEN_POS ) & ADEN_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Conversioned Data of ADC
 *	@return		12bit Data of ADC
 */
U16		NX_ADC_GetConvertedData( U32 ModuleIndex )
{
	register struct NX_ADC_RegisterSet*	pRegister;
	
    NX_ASSERT( NUMBER_OF_ADC_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;		
	NX_ASSERT( CNULL != pRegister );

	return (U16)(ReadIO32(&pRegister->ADCDAT));
}

