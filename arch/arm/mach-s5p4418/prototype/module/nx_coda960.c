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
//	File		: nx_coda960.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_coda960.h"
#include <string.h> // for memset

static	NX_CODA960_RegisterSet *__g_pRegister[NUMBER_OF_CODA960_MODULE];


//------------------------------------------------------------------------------
//
//	CODA960 Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	It is a sample function
 *	@return a register value
 */
U32   NX_CODA960_GetTEMP( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return __g_pRegister[ModuleIndex]->TEMP;
}

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
 *	@see	NX_CODA960_GetNumberOfModule
 */
CBOOL	NX_CODA960_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		memset( __g_pRegister, 0, sizeof(__g_pRegister) );
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. 
 *				It is equal to NUMBER_OF_CODA960_MODULE in <nx_chip.h>.
 *	@see		NX_CODA960_Initialize
 */
U32		NX_CODA960_GetNumberOfModule( void )
{
	return NUMBER_OF_CODA960_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_CODA960_GetSizeOfRegisterSet( void )
{
return sizeof( *__g_pRegister );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_CODA960_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_CODA960_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.

 */
void*	NX_CODA960_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_CODA960?_MODULE in <nx_chip.h>.

 */
U32		NX_CODA960_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_CODA960_MODULE] =
	{
		//PHY_BASEADDR_LIST( CODA960 )
		PHY_BASEADDR_WITH_CHANNEL_LIST( CODA960, APB1 )
	};
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. 
 *				CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_CODA960_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicate that Deinitialize is successed. 
 *				CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_CODA960_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicate that Module is Busy. 
 */
CBOOL	NX_CODA960_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_CODA960?_MODULE in <nx_chip.h>.
 */
U32 NX_CODA960_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( CODA960 )
	};
	NX_CASSERT( NUMBER_OF_CODA960_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_CODA960?_MODULE_i_nRST in <nx_chip.h>.
 */
 /*
U32 NX_CODA960_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( CODA960, IPnRST0 )
	};
	NX_CASSERT( NUMBER_OF_CODA960_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}
*/

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_CODA960?_MODULE in <nx_chip.h>.
 */
 /*
U32 	NX_CODA960_GetInterruptNumber( U32 ModuleIndex )
{
    //const U32 InterruptNumber[] = { INTNUM_LIST( CODA960 ) };
    const U32 InterruptNumber[] = { INTNUM_WITH_CHANNEL_LIST( CODA960, INTR0 ) };
    NX_CASSERT( NUMBER_OF_CODA960_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_CODA9600_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}
*/
//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_CODA960_INTCH_xxx in <nx_coda960.h>
 *	@param[in]	Enable	Set as CTRUE to enable a specified interrupt. 
 *						Set as CFALSE to disable a specified interrupt.
 *	@return		None.

 */
void	NX_CODA960_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register NX_CODA960_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->INTCTRL;

	regvalue &=	~( 1UL << IntNum );
	regvalue |= (U32)Enable << IntNum;

	WriteIO32(&pRegister->INTCTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_CODA960_INTCH_xxx in <nx_coda960.h>
 *	@return		CTRUE	indicates that a specified interrupt is enabled. 
 *				CFALSE	indicates that a specified interrupt is disabled.

 */
CBOOL	NX_CODA960_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)( ( __g_pRegister[ModuleIndex]->INTCTRL >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_CODA960_INTCH_xxx in <nx_coda960.h>
 *	@return		CTRUE	indicates that a specified interrupt is pended. 
 *				CFALSE	indicates that a specified interrupt is not pended.

 */
CBOOL	NX_CODA960_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_CODA960_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->INTCTRL;
	regvalue &= pRegister->INTPEND;
	return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.
 *						refer to NX_CODA960_INTCH_xxx in <nx_coda960.h>
 *	@return		None.

 */
void	NX_CODA960_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_CODA960_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->INTPEND, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	Set as CTRUE to enable all interrupts. 
 *						Set as CFALSE to disable all interrupts.
 *	@return		None.

 */
void	NX_CODA960_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register NX_CODA960_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = Enable ? 0xFFFFFFFF : 0 ;

	WriteIO32(&pRegister->INTCTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	indicates that one or more interrupts are enabled. 
 *				CFALSE	indicates that all interrupts are disabled.

 */
CBOOL	NX_CODA960_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->INTCTRL ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	indicates that one or more interrupts are pended. 
 *				CFALSE	indicates that no interrupt is pended.

 */
CBOOL	NX_CODA960_GetInterruptPendingAll( U32 ModuleIndex )
{
	register NX_CODA960_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->INTCTRL;
	regvalue &= pRegister->INTPEND;
	return (CBOOL)( 0 != ( regvalue ) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.

 */
void	NX_CODA960_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register NX_CODA960_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->INTPEND, 0xFFFFFFFF);	// just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_CODA960_INTCH_xxx in <nx_coda960.h>

 */
S32		NX_CODA960_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	int i;
	register NX_CODA960_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->INTCTRL;
	regvalue &= pRegister->INTPEND;
	if( 0!=regvalue )
	{
		for( i=0; i<32; i++ )
		{
			if( regvalue & 1UL )
			{
				return i;
			}
			regvalue>>=1;
		}
	}
	return -1;
}


//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index
 *	@param[in]	ModuleIndex	an index of module.
 *	@param[in]	ChannelIndex refer to NX_CODA960_DMACH_xxx in <nx_coda960.h>
 *	@return		DMA peripheral index.
 */
 /*
U32 NX_CODA960_GetDMANumber ( U32 ModuleIndex , U32 ChannelIndex )
{
	const U32 DMANumber[][NUMBER_OF_CODA960_MODULE] =
	{
	    { DMAINDEX_WITH_CHANNEL_LIST( CODA960, TXDMA ) }, // DMAINDEX_OF_CODA960?_MODULE_TEST0
	    { DMAINDEX_WITH_CHANNEL_LIST( CODA960, RXDMA ) }, // DMAINDEX_OF_CODA960?_MODULE_TEST1
	};
	NX_CASSERT( NUMBER_OF_CODA960_MODULE == (sizeof(DMANumber[0])/sizeof(DMANumber[0][0])) );
	NX_ASSERT( NUMBER_OF_CODA960_MODULE > ModuleIndex );
	NX_ASSERT( (sizeof(DMANumber)/sizeof(DMANumber[0])) > ChannelIndex );
	// NX_ASSERT( DMAINDEX_OF_CODA9600_MODULE_TEST0 == DMANumber[0][0] );
	// NX_ASSERT( DMAINDEX_OF_CODA9601_MODULE_TEST0 == DMANumber[1][0] );
	// ...
	return DMANumber[ChannelIndex][ModuleIndex];
}
*/

