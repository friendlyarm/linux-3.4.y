//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: GPIO
//	File		: nx_gpio.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------

#include "nx_gpio.h"

//U32 __g_NX_GPIO_VALID_BIT[NUMBER_OF_GPIO_MODULE] = {
//			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF	// A, B, C
//		};
//
//U32	__g_NX_GPIO_VALID_NUM_OF_INT[NUMBER_OF_GPIO_MODULE] = {
//			32, 32, 32, 32, 32, 32		// A, B, C
//		};
//
//U32	__g_NX_GPIO_VALID_INT_MASK[NUMBER_OF_GPIO_MODULE] = {
//			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF		// A, B, C
//		};
U32 __g_NX_GPIO_VALID_BIT[NUMBER_OF_GPIO_MODULE];
U32	__g_NX_GPIO_VALID_NUM_OF_INT[NUMBER_OF_GPIO_MODULE];
U32	__g_NX_GPIO_VALID_INT_MASK[NUMBER_OF_GPIO_MODULE];

static const U32 __g_GPIO_PAD_INFO[32][NUMBER_OF_GPIO_MODULE] =
{
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_0_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_1_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_2_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_3_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_4_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_5_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_6_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_7_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_8_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_9_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_10_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_11_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_12_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_13_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_14_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_15_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_16_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_17_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_18_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_19_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_20_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_21_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_22_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_23_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_24_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_25_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_26_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_27_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_28_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_29_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_30_) },
    { PADINDEX_WITH_CHANNEL_LIST_ALPHA(GPIO, GPIO_31_) }
};

static	struct
{
	struct NX_GPIO_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_GPIO_MODULE] = { {CNULL,}, };

enum { NX_GPIO_MAX_BIT = 32 };

//------------------------------------------------------------------------------
// inline Function
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set 1bit value
 *	@param[in]	Value	This Value is changed when SetBit() executed.
 *	@param[in]	Bit		Bit number
 *	@param[in]	Enable	 CTRUE( Set ).
 *						 CFALSE( Clear )
 *	@return		None.
 *	@see		NX_GPIO_GetBit
 */


__inline void NX_GPIO_SetBit
(
	volatile U32* Value,
	U32 Bit,
	CBOOL Enable
)
{
	register U32 newvalue;

	NX_ASSERT( CNULL != Value );
	NX_ASSERT( NX_GPIO_MAX_BIT > Bit );

	newvalue = *Value;

	newvalue &=		~(1UL		<< Bit );
	newvalue |=		(U32)Enable << Bit ;

//	*Value = newvalue;
	WriteIO32(Value, newvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get bit value
 *	@param[in]	Value	Check this value of 1bit state
 *	@param[in]	Bit		Bit number
 *	@return		 CTRUE	indicate that bit is Seted.
 *				 CFALSE	indicate that bit is cleared.
 *	@see		NX_GPIO_SetBit
 */
__inline CBOOL NX_GPIO_GetBit
(
	U32 Value,
	U32 Bit
)
{
	NX_ASSERT( NX_GPIO_MAX_BIT > Bit );

	return (CBOOL)(( Value >> Bit ) & ( 1UL ) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set 2bit value
 *	@param[in]	Value		This Value is changed when SetBit2() executed.
 *	@param[in]	Bit			Bit number
 *	@param[in]	BitValue	Set value of bit
 *	@return		None.
 */
__inline void NX_GPIO_SetBit2
(
	volatile U32* Value,
	U32 Bit,
	U32 BitValue
)
{
	register U32 newvalue = *Value;

	NX_ASSERT( CNULL != Value );
	NX_ASSERT( NX_GPIO_MAX_BIT > Bit );

	newvalue = (U32)(newvalue & ~(3UL<<(Bit*2)));
	newvalue = (U32)(newvalue | (BitValue<<(Bit*2)));
//	*Value = newvalue;
	WriteIO32(Value, newvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get 2bit value
 *	@param[in]	Value	Check this value of 2bit state
 *	@param[in]	Bit		Bit number
 *	@return		2bit value
 */

__inline U32 NX_GPIO_GetBit2
(
	U32 Value,
	U32 Bit
)
{
	NX_ASSERT( NX_GPIO_MAX_BIT > Bit );

	return (U32)((U32)(Value>>(Bit*2)) & 3UL);
}

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicate that Initialize is successed.
 *			 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_GPIO_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_GPIO_MODULE; i++ )
		{
			__g_ModuleVariables[i].pRegister = CNULL;
		}

		bInit = CTRUE;
	}

	for( i = 0 ; i < NUMBER_OF_GPIO_MODULE; i++) {
		 __g_NX_GPIO_VALID_BIT[i] = 0xFFFFFFFF;
		__g_NX_GPIO_VALID_NUM_OF_INT[i] = 32;
		__g_NX_GPIO_VALID_INT_MASK[i] = 0xFFFFFFFF;
	};

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_GPIO_GetNumberOfModule( void )
{
	return NUMBER_OF_GPIO_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		A index of module.
 *	@return		Module's physical address
 */
U32		NX_GPIO_GetPhysicalAddress( U32 ModuleIndex )
{
#if 	NUMBER_OF_GPIO_MODULE>1
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST_ALPHA( GPIO ) }; // PHY_BASEADDR_UART?_MODULE
#else
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( GPIO ) }; // PHY_BASEADDR_UART?_MODULE
#endif
	NX_CASSERT( NUMBER_OF_GPIO_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set
 */
U32		NX_GPIO_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_GPIO_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		A index of module.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_GPIO_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_GPIO_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		A index of module.
 *	@return		Module's base address.
 */
void*	NX_GPIO_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		A index of module.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_GPIO_OpenModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		A index of module.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_GPIO_CloseModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		A index of module.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 *
 */
CBOOL	NX_GPIO_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	return CFALSE;
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@return		Interrupt number
 */
 U32		NX_GPIO_GetInterruptNumber( U32 ModuleIndex )
{
	const U32 INTNumber[NUMBER_OF_GPIO_MODULE] =
	{
#if NUMBER_OF_GPIO_MODULE>1
		INTNUM_LIST_ALPHA(GPIO)
#else
		INTNUM_LIST(GPIO)
#endif
	};

	NX_CASSERT( NUMBER_OF_GPIO_MODULE == (sizeof(INTNumber)/sizeof(INTNumber[0])) );
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	return INTNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	IntNum	Interrupt Number. ( 0 ~ 31 ), if ModuleIndex is GPIOC then IntNum is only 0 ~ 20.
 *	@param[in]	Enable	 CTRUE	indicate that Interrupt Enable. 
 *						 CFALSE	indicate that Interrupt Disable.
 *	@return		None
 */
void	NX_GPIO_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable )
{
	register	struct NX_GPIO_RegisterSet *pRegister;
	register	U32 ReadValue ;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( __g_NX_GPIO_VALID_NUM_OF_INT[ModuleIndex] > (U32)IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue = pRegister->GPIOxINTENB;

	ReadValue &= ~( (U32)1		<< IntNum);
	ReadValue |=	( (U32)Enable	<< IntNum );

//	pRegister->GPIOxINTENB = ReadValue;
	WriteIO32(&pRegister->GPIOxINTENB, ReadValue);

	NX_GPIO_SetDetectEnable(ModuleIndex, IntNum, Enable);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		 CTRUE	indicate that Interrupt is enabled. 
 *				 CFALSE	indicate that Interrupt is disabled.
 */
CBOOL	NX_GPIO_GetInterruptEnable( U32 ModuleIndex, S32 IntNum )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( __g_NX_GPIO_VALID_NUM_OF_INT[ModuleIndex] > (U32)IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB & (1UL << IntNum ) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							- EnableFlag[0] : Set GPIO0 interrupt enable or disable. 
 *							...
 *							- EnableFlag[31] : Set GPIO31 interrupt enable or disable. 
 *	@return		None.
 */
void	NX_GPIO_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

//	__g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB = (EnableFlag & __g_NX_GPIO_VALID_INT_MASK[ModuleIndex] );
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB, (EnableFlag & __g_NX_GPIO_VALID_INT_MASK[ModuleIndex] ));

	NX_GPIO_SetDetectEnable32(ModuleIndex, EnableFlag);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		Current setting value of interrupt. 
 *				"1" means interrupt is enabled. 
 *				"0" means interrupt is disabled. 
 *				- Return Value[0] : GPIO0 interrupt's setting value. 
 *				...
 *				- Return Value[31] : GPIO31 interrupt's setting value. 
 */
U32		NX_GPIO_GetInterruptEnable32( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB & __g_NX_GPIO_VALID_INT_MASK[ModuleIndex] );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		 CTRUE	indicate that Pending is seted. 
 *				 CFALSE	indicate that Pending is Not Seted.
 */
CBOOL	NX_GPIO_GetInterruptPending( U32 ModuleIndex, S32 IntNum )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( __g_NX_GPIO_VALID_NUM_OF_INT[ModuleIndex] > (U32)IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET & (1UL << IntNum ) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		Current setting value of pending bit. 
 *				"1" means pend bit is occured. 
 *				"0" means pend bit is NOT occured. 
 *				- Return Value[0] : GPIO0 pending state. 
 *				...
 *				- Return Value[31] : GPIO31 pending state. 
 */
U32		NX_GPIO_GetInterruptPending32( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return(U32)( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET & __g_NX_GPIO_VALID_INT_MASK[ModuleIndex] );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 */
void	NX_GPIO_ClearInterruptPending( U32 ModuleIndex, S32 IntNum )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( __g_NX_GPIO_VALID_NUM_OF_INT[ModuleIndex] > (U32)IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

//	__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET = 1UL << IntNum;
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	 
 *								- PendingFlag[0] : GPIO0 pending bit. 
 *									...
 *								- PendingFlag[31] : GPIO31 pending bit. 
 *	@return		None.
 */
void	NX_GPIO_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

//	__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET = (PendingFlag & __g_NX_GPIO_VALID_INT_MASK[ModuleIndex]);
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET, (PendingFlag & __g_NX_GPIO_VALID_INT_MASK[ModuleIndex]));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Enable	 CTRUE	indicate that Set to all interrupt enable. 
 *						 CFALSE	indicate that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_GPIO_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register		U32	setvalue;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( Enable )
	{	setvalue = ~(0UL);	}
	else
	{	setvalue = 0UL;	}

//	__g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB	= setvalue;
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB, setvalue);

	NX_GPIO_SetInterruptEnable32(ModuleIndex, setvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		 CTRUE	indicate that At least one( or more ) interrupt is enabled. 
 *				 CFALSE	indicate that All interrupt is disabled.
 */
CBOOL	NX_GPIO_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxINTENB )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		 CTRUE	indicate that At least one( or more ) pending is seted. 
 *				 CFALSE	indicate that All pending is NOT seted.
 */
CBOOL	NX_GPIO_GetInterruptPendingAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		None.
 */
void	NX_GPIO_ClearInterruptPendingAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDET, (U32)(0xFFFFFFFF));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
S32		NX_GPIO_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	register struct NX_GPIO_RegisterSet	*pRegister;
	register	U32 intnum;
	register	U32 intpend;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	intpend = (pRegister->GPIOxINTENB & pRegister->GPIOxDET);

	for( intnum=0 ; intnum<32 ; intnum++ )
	{
		if( 0 != (intpend & (1UL<<intnum)) )
		{
			return intnum;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
// GPIO Operation.
//------------------------------------------------------------------------------

/**
 *	@brief		Set interrupt mode
 *	@param[in]	ModuleIndex	A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	IntMode		GPIO interrupt detect mode
 *	@return		None.
 */
void				NX_GPIO_SetInterruptMode( U32 ModuleIndex, U32 BitNumber, NX_GPIO_INTMODE IntMode )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( NX_GPIO_INTMODE_BOTHEDGE >= IntMode );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit2( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETMODE[BitNumber/16], BitNumber%16, (U32)IntMode & 0x03 );
	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETMODEEX, BitNumber, (U32)(IntMode>>2) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get interrupt mode
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		GPIO interrupt detect mode
 */
NX_GPIO_INTMODE	NX_GPIO_GetInterruptMode( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_GPIO_INTMODE)NX_GPIO_GetBit2( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETMODE[BitNumber/16], BitNumber%16 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio output enable
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	OutputEnb	 CTRUE	indicate that Output Mode. 
 *							 CFALSE	indicate that Input Mode.
 *	@return		None.
 */
void	NX_GPIO_SetOutputEnable ( U32 ModuleIndex, U32 BitNumber, CBOOL OutputEnb )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( (0==OutputEnb) || (1==OutputEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUTENB, BitNumber, OutputEnb );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio Event Detect
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	OutputEnb	 CTRUE	indicate that Event Detect Enabled. 
 *							 CFALSE	indicate that Event Detect Disabled.
 *	@return		None.
 */
CBOOL			NX_GPIO_GetDetectEnable ( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return NX_GPIO_GetBit( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETENB, BitNumber );
}

U32			NX_GPIO_GetDetectEnable32 ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETENB;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Event Detect Enable
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		GPIO interrupt detect mode
 */
void			NX_GPIO_SetDetectEnable ( U32 ModuleIndex, U32 BitNumber, CBOOL DetectEnb )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( (0==DetectEnb) || (1==DetectEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETENB, BitNumber, DetectEnb );
}

void			NX_GPIO_SetDetectEnable32 ( U32 ModuleIndex, U32 EnableFlag )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxDETENB, EnableFlag );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio output enable
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		 CTRUE	indicate that GPIO's current Setting is Output Mode.
 *				 CFALSE	indicate that GPIO's current Setting is Input Mode.
 */
CBOOL	NX_GPIO_GetOutputEnable ( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return NX_GPIO_GetBit( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUTENB, BitNumber );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio output enable
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	OutputEnb	 CTRUE	indicate that Output Mode. 
 *							 CFALSE	indicate that Input Mode.
 *	@return		None.
 */
void	NX_GPIO_SetOutputEnable32 ( U32 ModuleIndex, CBOOL OutputEnb )
{
	U32	EnbMask = 0xffffffff;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( (0==OutputEnb) || (1==OutputEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if(OutputEnb)
		WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUTENB, EnbMask );
	else
		WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUTENB, 0x0 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio output enable
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		 CTRUE	indicate that GPIO's current Setting is Output Mode.
 *				 CFALSE	indicate that GPIO's current Setting is Input Mode.
 */
U32		NX_GPIO_GetOutputEnable32 ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUTENB;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set gpio output value
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	Value		 CTRUE	indicate that High Level. 
 *							 CFALSE	indicate that Low Level.
 *	@return		None.
 */
void	NX_GPIO_SetOutputValue	( U32 ModuleIndex, U32 BitNumber, CBOOL Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( (0==Value) || (1==Value) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUT, BitNumber, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio output value
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		 CTRUE	indicate that GPIO's output value is High Level.
 *				 CFALSE	indicate that GPIO's output value is Low Level.
 */
CBOOL	NX_GPIO_GetOutputValue	( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return NX_GPIO_GetBit( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUT, BitNumber );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set gpio output value
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	Value		 CTRUE	indicate that High Level. 
 *							 CFALSE	indicate that Low Level.
 *	@return		None.

 */
void	NX_GPIO_SetOutputValue32	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUT, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio output value
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		 CTRUE	indicate that GPIO's output value is High Level.
 *				 CFALSE	indicate that GPIO's output value is Low Level.

 */
U32		NX_GPIO_GetOutputValue32	( U32 ModuleIndex )
{
//	U32 EnbMask = 0xffffffff;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOxOUT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get gpio input value
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		 CTRUE	indicate that GPIO's input value is High Level.
 *				 CFALSE	indicate that GPIO's input value is Low Level.
 */
CBOOL	NX_GPIO_GetInputValue		( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );

	return NX_GPIO_GetBit( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxPAD, BitNumber );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Pull sel of GPIO Pin
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	enable		 CTRUE	indicate that Pull Up. 
 *							 CFALSE	indicate that NOT Pull Up.
 *	@return		None.
 */
void	NX_GPIO_SetPullSelect   ( U32 ModuleIndex, U32 BitNumber, CBOOL enable)
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( (0==enable) || (1==enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL_DISABLE_DEFAULT, BitNumber, CTRUE );
//	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL_DISABLE_DEFAULT, BitNumber, enable );
	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL, BitNumber, enable );
}

void	NX_GPIO_SetPullSelect32 ( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get pull sel value of GPIO Pin
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		GPIO pull up value
 */
CBOOL	NX_GPIO_GetPullSelect   ( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return NX_GPIO_GetBit( __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL, BitNumber );
}

U32		NX_GPIO_GetPullSelect32 ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL;
}

void	NX_GPIO_SetPullMode ( U32 ModuleIndex, U32 BitNumber, NX_GPIO_PULL mode)
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( (0==mode) || (1==mode) || (2==mode) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL_DISABLE_DEFAULT, BitNumber, CTRUE );
	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT, BitNumber, CTRUE );

	if (mode == NX_GPIO_PULL_OFF)
	{
//NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT, BitNumber, CFALSE );

		NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB, BitNumber, CFALSE );
		NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL, BitNumber, CFALSE );
	}
	else
	{
//NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT, BitNumber, CTRUE );

		NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL, BitNumber, (mode & 1 ? CTRUE : CFALSE) );
		NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB, BitNumber, CTRUE );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Pull up of GPIO Pin
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	enable		 CTRUE	indicate that Pull Up. 
 *							 CFALSE	indicate that NOT Pull Up.
 *	@return		None.
 */
void	NX_GPIO_SetPullEnable ( U32 ModuleIndex, U32 BitNumber, CBOOL enable)
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( (0==enable) || (1==enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT, BitNumber, enable );
//	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT, BitNumber, CTRUE );
	NX_GPIO_SetBit( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB, BitNumber, enable );
}

void	NX_GPIO_SetPullEnable32	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxPUENB, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get pull up value of GPIO Pin
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		GPIO pull up value

 */
CBOOL	NX_GPIO_GetPullEnable ( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return NX_GPIO_GetBit( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxPUENB, BitNumber );
}

U32		NX_GPIO_GetPullEnable32	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOxPUENB;
}

//--------------------------------------------------------------------------
// Pin Configuration
//--------------------------------------------------------------------------
/**
 *	@brief		Set PAD Fuction
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@param[in]	padfunc		Pad Function
 *	@return		None
 *	@remarks		Each gpio pins can use to GPIO Pin or Alternate Function 0 or Alternate Function 1 or 
 *				Alternate Function 2. So This function Sets gpio pin's function.
 */
void	NX_GPIO_SetPadFunction( U32 ModuleIndex, U32 BitNumber, NX_GPIO_PADFUNC padfunc )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( NX_GPIO_PADFUNC_3 >= padfunc );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_GPIO_SetBit2( &__g_ModuleVariables[ModuleIndex].pRegister->GPIOxALTFN[BitNumber/16], BitNumber%16, (U32)padfunc );
}

void	NX_GPIO_SetPadFunction32_0( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxALTFN[0], Value );
}

void	NX_GPIO_SetPadFunction32_1( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOxALTFN[1], Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get PAD Fuction
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 ), if ModuleIndex is GPIOC then Bit number is only 0 ~ 20.
 *	@return		None
 */
NX_GPIO_PADFUNC	NX_GPIO_GetPadFunction( U32 ModuleIndex, U32 BitNumber )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( 0 != (__g_NX_GPIO_VALID_BIT[ModuleIndex] & (1 << BitNumber)) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_GPIO_PADFUNC)NX_GPIO_GetBit2( __g_ModuleVariables[ModuleIndex].pRegister->GPIOxALTFN[BitNumber/16], BitNumber%16 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO's valid bit
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO's valid bit
 *	@remarks	Each bit's indicate corresponding GPIO pin. If retun value is 0x0FF, then
 *				This indicate that GPIO have eight pins ( GPIO0 ~ GPIO7 ).
 */
U32		NX_GPIO_GetValidBit( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	return __g_NX_GPIO_VALID_BIT[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO Slew
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetSLEW	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_SLEW, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO Slew
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO slew value
 */
U32		NX_GPIO_GetSLEW	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_SLEW;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO Slew Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetSLEW_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_SLEW_DISABLE_DEFAULT, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO Slew Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO GPIO Slew Disable Default value
 */
U32		NX_GPIO_GetSLEW_DISABLE_DEFAULT	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_SLEW_DISABLE_DEFAULT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO DRV1
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetDRV1	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV1, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO DRV1
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO GPIO DRV1 value
 */
U32		NX_GPIO_GetDRV1	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO DRV1 Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetDRV1_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV1_DISABLE_DEFAULT, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DRV1 Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO DRV1 Disable Default value
 */
U32		NX_GPIO_GetDRV1_DISABLE_DEFAULT	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV1_DISABLE_DEFAULT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO DRV0
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetDRV0	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV0, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO DRV0
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO DRV0 value
 */
U32		NX_GPIO_GetDRV0	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV0;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set DRV0 Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetDRV0_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV0_DISABLE_DEFAULT, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DRV0 Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO DRV0 Disable Default value
 */
U32		NX_GPIO_GetDRV0_DISABLE_DEFAULT	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_DRV0_DISABLE_DEFAULT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO Slew
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC, 3:GPIOD, 4:GPIOE )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 )
 *	@param[in]	Enable		Slew On/Off
 *	@return		None
 */
void		NX_GPIO_SetSlew	( U32 ModuleIndex, U32 BitNumber, CBOOL Enable )
{
	register struct NX_GPIO_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	NX_GPIO_SetBit(&pRegister->GPIOx_SLEW, BitNumber, Enable);
}

void		NX_GPIO_SetSlewDisableDefault	( U32 ModuleIndex, U32 BitNumber, CBOOL Enable )
{
	register struct NX_GPIO_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	NX_GPIO_SetBit(&pRegister->GPIOx_SLEW_DISABLE_DEFAULT, BitNumber, Enable);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO Slew
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC, 3:GPIOD, 4:GPIOE )
 *	@return		CTRUE	indicate that GPIO's slew is turned on.
 *				CFALSE	indicate that GPIO's slew is turned off.
 */
CBOOL		NX_GPIO_GetSlew	( U32 ModuleIndex, U32 BitNumber )
{
	register struct NX_GPIO_RegisterSet	*pRegister;
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)NX_GPIO_GetBit(ReadIO32(&pRegister->GPIOx_SLEW), BitNumber);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO Slew
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC, 3:GPIOD, 4:GPIOE )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetSlew32	( U32 ModuleIndex, U32 Value )
{
	register struct NX_GPIO_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->GPIOx_SLEW, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO Slew
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC, 3:GPIOD, 4:GPIOE )
 *	@return		GPIO slew value
 */
U32		NX_GPIO_GetSlew32	( U32 ModuleIndex )
{
	register struct NX_GPIO_RegisterSet	*pRegister;
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	return ReadIO32(&pRegister->GPIOx_SLEW);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO Drive Strength
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC, 3:GPIOD, 4:GPIOE )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 )
 *	@param[in]	DriveStrength
 *	@return		None
 */
void		NX_GPIO_SetDriveStrength	( U32 ModuleIndex, U32 BitNumber, NX_GPIO_DRVSTRENGTH drvstrength)
{
	register struct NX_GPIO_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	NX_GPIO_SetBit(&pRegister->GPIOx_DRV1, BitNumber, (CBOOL)(((U32)drvstrength>>0) & 0x1));
	NX_GPIO_SetBit(&pRegister->GPIOx_DRV0, BitNumber, (CBOOL)(((U32)drvstrength>>1) & 0x1));
}

void		NX_GPIO_SetDriveStrengthDisableDefault	( U32 ModuleIndex, U32 BitNumber, CBOOL Enable )
{
	register struct NX_GPIO_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	NX_GPIO_SetBit(&pRegister->GPIOx_DRV1_DISABLE_DEFAULT, BitNumber, (CBOOL)(Enable) );
	NX_GPIO_SetBit(&pRegister->GPIOx_DRV0_DISABLE_DEFAULT, BitNumber, (CBOOL)(Enable) );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO Drive Strength
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC, 3:GPIOD, 4:GPIOE )
 *	@param[in]	BitNumber	Bit number ( 0 ~ 31 )
 *	@return		GPIO GPIO Drive Strength
 */
NX_GPIO_DRVSTRENGTH		NX_GPIO_GetDriveStrength	( U32 ModuleIndex, U32 BitNumber )
{
	register struct NX_GPIO_RegisterSet	*pRegister;
	register U32 retvalue;

	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

	retvalue  = NX_GPIO_GetBit(ReadIO32(&pRegister->GPIOx_DRV0), BitNumber)<<1;
	retvalue |= NX_GPIO_GetBit(ReadIO32(&pRegister->GPIOx_DRV1), BitNumber)<<0;

	return (NX_GPIO_DRVSTRENGTH)retvalue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO PullSel
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetPULLSEL	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO PullSel
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO PullSel value
 */
U32		NX_GPIO_GetPULLSEL	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO PullSel Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetPULLSEL_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL_DISABLE_DEFAULT, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get PullSel Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO PullSel Disable Default value
 */
U32		NX_GPIO_GetPULLSEL_DISABLE_DEFAULT	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLSEL_DISABLE_DEFAULT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO PullEnb
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetPULLENB	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO PullEnb
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO PullEnb value
 */
U32		NX_GPIO_GetPULLENB	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set GPIO PullEnb Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void		NX_GPIO_SetPULLENB_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get GPIO PullEnb Disable Default
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@return		GPIO  value
 */
U32		NX_GPIO_GetPULLENB_DISABLE_DEFAULT	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return __g_ModuleVariables[ModuleIndex].pRegister->GPIOx_PULLENB_DISABLE_DEFAULT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Input Mux Select0
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void	NX_GPIO_SetInputMuxSelect0	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_InputMuxSelect0, Value );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set  Input Mux Select1
 *	@param[in]	ModuleIndex		A index of module. (0:GPIOA, 1:GPIOB, 2:GPIOC )
 *	@param[in]	Value			32bit input data
 *	@return		None
 */
void	NX_GPIO_SetInputMuxSelect1	( U32 ModuleIndex, U32 Value )
{
	NX_ASSERT( NUMBER_OF_GPIO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GPIOx_InputMuxSelect1, Value );
}

//---------------------------------------------------------
/*
	pad info
	http://nxconfluence.co.kr:8090/pages/editpage.action?pageId=2786185
*/
//---------------------------------------------------------

/*
static CBOOL  NX_GPIO_PadInfo_GetGpioUsed( U32 padinfo )
{
	return NX_BIT_GETBIT_RANGE32 ( padinfo, 16, 16 );
}
*/
static U32 NX_GPIO_PadInfo_GetModuleIndex( U32 padinfo )
{
	return NX_BIT_GETBIT_RANGE32 ( padinfo, 15, 8 );
}
static U32 NX_GPIO_PadInfo_GetBitNumber( U32 padinfo )
{
	return NX_BIT_GETBIT_RANGE32 ( padinfo, 7, 3 );
}
static NX_GPIO_PADFUNC  NX_GPIO_PadInfo_GetPadFunction( U32 padinfo )
{
	return (NX_GPIO_PADFUNC)NX_BIT_GETBIT_RANGE32 ( padinfo, 2, 0 );
}

void NX_GPIO_SetPadFunctionEnable (  U32 padinfo  )
{
	U32 moduleIndex = NX_GPIO_PadInfo_GetModuleIndex( padinfo );
	U32 bitNumber = NX_GPIO_PadInfo_GetBitNumber( padinfo );
	NX_GPIO_PADFUNC  padFunction = NX_GPIO_PadInfo_GetPadFunction( padinfo );
	NX_GPIO_SetPadFunction ( moduleIndex, bitNumber, padFunction );
}
