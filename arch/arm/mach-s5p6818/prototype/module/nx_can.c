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
//	File		: nx_CAN.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_can.h"

static	NX_CAN_RegisterSet *__g_pRegister[NUMBER_OF_CAN_MODULE];


//------------------------------------------------------------------------------
//
//	CAN Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	It is a sample function
 *	@return a register value
 */
/*
U32   NX_CAN_GetTEMP( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return __g_pRegister[ModuleIndex]->TEMP;
}
*/

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
 *	@see	NX_CAN_GetNumberOfModule
 */
CBOOL	NX_CAN_Initialize( void )
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
 *				It is equal to NUMBER_OF_CAN_MODULE in <nx_chip.h>.
 */
U32		NX_CAN_GetNumberOfModule( void )
{
	return NUMBER_OF_CAN_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_CAN_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_CAN_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_CAN_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_CAN_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_CAN_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_CAN?_MODULE in <nx_chip.h>.
 */
U32		NX_CAN_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_CAN_MODULE] =
	{
		PHY_BASEADDR_LIST( CAN )
		//PHY_BASEADDR_WITH_CHANNEL_LIST( CAN, APB1 )
	};
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_CAN_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_CAN_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_CAN_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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
 *				It is equal to CLOCKINDEX_OF_CAN?_MODULE in <nx_chip.h>.
 */
/*
U32 NX_CAN_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( CAN )
	};
	NX_CASSERT( NUMBER_OF_CAN_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}
*/

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_CAN?_MODULE_i_nRST in <nx_chip.h>.
 */
/*
U32 NX_CAN_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( CAN, IPnRST0 )
	};
	NX_CASSERT( NUMBER_OF_CAN_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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
 *				It is equal to INTNUM_OF_CAN?_MODULE in <nx_chip.h>.
 */
U32 	NX_CAN_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( CAN ) };
    //const U32 InterruptNumber[] = { INTNUM_WITH_CHANNEL_LIST( CAN, INTR0 ) };
    NX_CASSERT( NUMBER_OF_CAN_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_CAN0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_CAN_INTCH_xxx in <nx_CAN.h>
 *	@param[in]	Enable	 Set as CTRUE to enable a specified interrupt. 
 *						 Set as CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_CAN_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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
 *						refer to NX_CAN_INTCH_xxx in <nx_CAN.h>
 *	@return		 CTRUE	indicates that a specified interrupt is enabled. 
 *				 CFALSE	indicates that a specified interrupt is disabled.

 */
CBOOL	NX_CAN_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)( ( __g_pRegister[ModuleIndex]->INTCTRL >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_CAN_INTCH_xxx in <nx_CAN.h>
 *	@return		 CTRUE	indicates that a specified interrupt is pended. 
 *				 CFALSE	indicates that a specified interrupt is not pended.

 */
CBOOL	NX_CAN_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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
 *						refer to NX_CAN_INTCH_xxx in <nx_CAN.h>
 *	@return		None.

 */
void	NX_CAN_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_CAN_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->INTPEND, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 Set as CTRUE to enable all interrupts. 
 *						 Set as CFALSE to disable all interrupts.
 *	@return		None.

 */
void	NX_CAN_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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
 *	@return		 CTRUE	indicates that one or more interrupts are enabled. 
 *				 CFALSE	indicates that all interrupts are disabled.


 */
CBOOL	NX_CAN_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->INTCTRL ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are pended. 
 *				 CFALSE	indicates that no interrupt is pended.


 */
CBOOL	NX_CAN_GetInterruptPendingAll( U32 ModuleIndex )
{
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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
void	NX_CAN_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register NX_CAN_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->INTPEND, 0xFFFFFFFF);	// just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_CAN_INTCH_xxx in <nx_CAN.h>

 */
S32		NX_CAN_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	int i;
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
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


#ifdef SOC_TEST
void    NX_CAN_REG_RD_TEST ( U32 ModuleIndex )
{
    int i;
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

        regvalue = pRegister->CAN_REG00;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 0, regvalue );
        regvalue = pRegister->CAN_REG01;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 1, regvalue );
        regvalue = pRegister->CAN_REG02;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 2, regvalue );
        regvalue = pRegister->CAN_REG03;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 3, regvalue );
        regvalue = pRegister->INTCTRL;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 4, regvalue );
        regvalue = pRegister->CAN_REG05;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 5, regvalue );
        regvalue = pRegister->CAN_REG06;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 6, regvalue );
        regvalue = pRegister->CAN_REG07;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 7, regvalue );
        regvalue = pRegister->CAN_REG08;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 8, regvalue );
        regvalue = pRegister->CAN_REG09;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 9, regvalue );
        regvalue = pRegister->CAN_REG10;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 10, regvalue );
        regvalue = pRegister->CAN_REG11;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 11, regvalue );
        regvalue = pRegister->CAN_REG12;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 12, regvalue );
        regvalue = pRegister->CAN_REG13;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 13, regvalue );
        regvalue = pRegister->CAN_REG14;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 14, regvalue );
        regvalue = pRegister->CAN_REG15;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 15, regvalue );
        regvalue = pRegister->CAN_REG16;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 16, regvalue );
        regvalue = pRegister->CAN_REG17;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 17, regvalue );
        regvalue = pRegister->CAN_REG18;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 18, regvalue );
        regvalue = pRegister->CAN_REG19;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 19, regvalue );
        regvalue = pRegister->CAN_REG20;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 20, regvalue );
        regvalue = pRegister->CAN_REG21;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 21, regvalue );
        regvalue = pRegister->CAN_REG22;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 22, regvalue );
        regvalue = pRegister->CAN_REG23;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 23, regvalue );
        regvalue = pRegister->CAN_REG24;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 24, regvalue );
        regvalue = pRegister->CAN_REG25;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 25, regvalue );
        regvalue = pRegister->CAN_REG26;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 26, regvalue );
        regvalue = pRegister->CAN_REG27;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 27, regvalue );
        regvalue = pRegister->CAN_REG28;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 28, regvalue );
        regvalue = pRegister->CAN_REG29;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 29, regvalue );
        regvalue = pRegister->CAN_REG30;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 30, regvalue );
        regvalue = pRegister->CAN_REG31;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 31, regvalue );
        regvalue = pRegister->CAN_REG32;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 32, regvalue );
        regvalue = pRegister->CAN_REG33;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 33, regvalue );
        regvalue = pRegister->CAN_REG34;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 34, regvalue );
        regvalue = pRegister->CAN_REG35;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 35, regvalue );
        regvalue = pRegister->CAN_REG36;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 36, regvalue );
        regvalue = pRegister->CAN_REG37;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 37, regvalue );
        regvalue = pRegister->CAN_REG38;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 38, regvalue );
        regvalue = pRegister->CAN_REG39;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 39, regvalue );
        regvalue = pRegister->CAN_REG40;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 40, regvalue );
        regvalue = pRegister->CAN_REG41;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 41, regvalue );
        regvalue = pRegister->CAN_REG42;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 42, regvalue );
        regvalue = pRegister->CAN_REG43;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 43, regvalue );
        regvalue = pRegister->CAN_REG44;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 44, regvalue );
        regvalue = pRegister->CAN_REG45;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 45, regvalue );
        regvalue = pRegister->CAN_REG46;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 46, regvalue );
        regvalue = pRegister->CAN_REG47;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 47, regvalue );
        regvalue = pRegister->CAN_REG48;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 48, regvalue );
        regvalue = pRegister->CAN_REG49;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 49, regvalue );
        regvalue = pRegister->CAN_REG50;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 50, regvalue );
        regvalue = pRegister->CAN_REG51;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 51, regvalue );
        regvalue = pRegister->CAN_REG52;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 52, regvalue );
        regvalue = pRegister->CAN_REG53;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 53, regvalue );
        regvalue = pRegister->CAN_REG54;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 54, regvalue );
        regvalue = pRegister->CAN_REG55;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 55, regvalue );
        regvalue = pRegister->CAN_REG56;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 56, regvalue );
        regvalue = pRegister->CAN_REG57;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 57, regvalue );
        regvalue = pRegister->CAN_REG58;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 58, regvalue );
        regvalue = pRegister->CAN_REG59;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 59, regvalue );
        regvalue = pRegister->CAN_REG60;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 60, regvalue );
        regvalue = pRegister->CAN_REG61;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 61, regvalue );
        regvalue = pRegister->CAN_REG62;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 62, regvalue );
        regvalue = pRegister->CAN_REG63;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 63, regvalue );
        regvalue = pRegister->CAN_REG64;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 64, regvalue );
        regvalue = pRegister->CAN_REG65;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 65, regvalue );
        regvalue = pRegister->CAN_REG66;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 66, regvalue );
        regvalue = pRegister->CAN_REG67;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 67, regvalue );
        regvalue = pRegister->CAN_REG68;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 68, regvalue );
        regvalue = pRegister->CAN_REG69;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 69, regvalue );
        regvalue = pRegister->CAN_REG70;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 70, regvalue );
        regvalue = pRegister->CAN_REG71;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 71, regvalue );
        regvalue = pRegister->CAN_REG72;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 72, regvalue );
        regvalue = pRegister->CAN_REG73;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 73, regvalue );
        regvalue = pRegister->CAN_REG74;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 74, regvalue );
        regvalue = pRegister->CAN_REG75;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 75, regvalue );
        regvalue = pRegister->CAN_REG77;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 77, regvalue );
        regvalue = pRegister->CAN_REG78;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 78, regvalue );
        regvalue = pRegister->CAN_REG79;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 79, regvalue );
        regvalue = pRegister->INTPEND  ;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 80, regvalue );
        regvalue = pRegister->CAN_REG81;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 81, regvalue );
        regvalue = pRegister->CAN_REG82;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 82, regvalue );
        regvalue = pRegister->CAN_REG83;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 83, regvalue );
        regvalue = pRegister->CAN_REG84;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 84, regvalue );
        regvalue = pRegister->CAN_REG85;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 85, regvalue );
        regvalue = pRegister->CAN_REG86;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 86, regvalue );
        regvalue = pRegister->CAN_REG87;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 87, regvalue );
        regvalue = pRegister->CAN_REG88;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 88, regvalue );
        regvalue = pRegister->CAN_REG89;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 89, regvalue );
        regvalue = pRegister->CAN_REG90;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 90, regvalue );
        regvalue = pRegister->CAN_REG91;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 91, regvalue );
        regvalue = pRegister->CAN_REG92;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 92, regvalue );
        regvalue = pRegister->CAN_REG93;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 93, regvalue );
        regvalue = pRegister->CAN_REG94;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 94, regvalue );
        regvalue = pRegister->CAN_REG95;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 95, regvalue );

		pRegister->CAN_REG00 = 0x5555;
        regvalue = pRegister->CAN_REG00;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 0, regvalue );
		pRegister->CAN_REG01 = 0x5555;
        regvalue = pRegister->CAN_REG01;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 1, regvalue );
        pRegister->CAN_REG02 = 0x5555;
        regvalue = pRegister->CAN_REG02;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 2, regvalue );
        pRegister->CAN_REG03 = 0x5555;
        regvalue = pRegister->CAN_REG03;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 3, regvalue );
        pRegister->CAN_REG06 = 0x5555;
        regvalue = pRegister->CAN_REG06;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 6, regvalue );
        pRegister->CAN_REG08 = 0x5555;
        regvalue = pRegister->CAN_REG08;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 8, regvalue );
        pRegister->CAN_REG09 = 0x5555;
        regvalue = pRegister->CAN_REG09;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 9, regvalue );
        pRegister->CAN_REG10 = 0x5555;
        regvalue = pRegister->CAN_REG10;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 10, regvalue );
        pRegister->CAN_REG11 = 0x5555;
        regvalue = pRegister->CAN_REG11;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 11, regvalue );
        pRegister->CAN_REG12 = 0x5555;
        regvalue = pRegister->CAN_REG12;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 12, regvalue );
        pRegister->CAN_REG13 = 0x5555;
        regvalue = pRegister->CAN_REG13;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 13, regvalue );
        pRegister->CAN_REG14 = 0x5555;
        regvalue = pRegister->CAN_REG14;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 14, regvalue );
        pRegister->CAN_REG15 = 0x5555;
        regvalue = pRegister->CAN_REG15;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 15, regvalue );
        pRegister->CAN_REG16 = 0x5555;
        regvalue = pRegister->CAN_REG16;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 16, regvalue );
        pRegister->CAN_REG17 = 0x5555;
        regvalue = pRegister->CAN_REG17;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 17, regvalue );
        pRegister->CAN_REG18 = 0x5555;
        regvalue = pRegister->CAN_REG18;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 18, regvalue );
        pRegister->CAN_REG28 = 0x5555;
        regvalue = pRegister->CAN_REG28;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 28, regvalue );
        pRegister->CAN_REG29 = 0x5555;
        regvalue = pRegister->CAN_REG29;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 29, regvalue );
        pRegister->CAN_REG30 = 0x5555;
        regvalue = pRegister->CAN_REG30;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 30, regvalue );
        pRegister->CAN_REG31 = 0x5555;
        regvalue = pRegister->CAN_REG31;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 31, regvalue );
        pRegister->CAN_REG32 = 0x5555;
        regvalue = pRegister->CAN_REG32;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 32, regvalue );
        pRegister->CAN_REG33 = 0x5555;
        regvalue = pRegister->CAN_REG33;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 33, regvalue );
        pRegister->CAN_REG34 = 0x5555;
        regvalue = pRegister->CAN_REG34;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 34, regvalue );
        pRegister->CAN_REG35 = 0x5555;
        regvalue = pRegister->CAN_REG35;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 35, regvalue );
        pRegister->CAN_REG36 = 0x5555;
        regvalue = pRegister->CAN_REG36;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 36, regvalue );
        pRegister->CAN_REG37 = 0x5555;
        regvalue = pRegister->CAN_REG37;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 37, regvalue );
        pRegister->CAN_REG38 = 0x5555;
        regvalue = pRegister->CAN_REG38;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 38, regvalue );
        pRegister->CAN_REG60 = 0x5555;
        regvalue = pRegister->CAN_REG60;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 60, regvalue );
        pRegister->CAN_REG61 = 0x5555;
        regvalue = pRegister->CAN_REG61;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 61, regvalue );
        pRegister->CAN_REG68 = 0x5555;
        regvalue = pRegister->CAN_REG68;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 68, regvalue );
        pRegister->CAN_REG69 = 0x5555;
        regvalue = pRegister->CAN_REG69;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 69, regvalue );
        pRegister->INTPEND =   0x5555;
        regvalue = pRegister->INTPEND  ;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 76, regvalue );
        pRegister->CAN_REG77 = 0x5555;
        regvalue = pRegister->CAN_REG77;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 77, regvalue );
        pRegister->CAN_REG84 = 0x5555;
        regvalue = pRegister->CAN_REG84;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 84, regvalue );
        pRegister->CAN_REG85 = 0x5555;
        regvalue = pRegister->CAN_REG85;
        NX_CONSOLE_Printf("REG_RD : addr = %d : data = %08x\n", 85, regvalue );

}

void    NX_CAN_RDWR_RAM_TEST ( U32 ModuleIndex )
{
    int i;
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

    pRegister->CAN_REG09 = 0x00F3;
    pRegister->CAN_REG33 = 0x007F;
    pRegister->CAN_REG10 = 0x5555;
    pRegister->CAN_REG11 = 0x5555;
    pRegister->CAN_REG12 = 0x5555;
    pRegister->CAN_REG13 = 0x5555;
    pRegister->CAN_REG14 = 0x5555;
    pRegister->CAN_REG15 = 0x5555;
    pRegister->CAN_REG16 = 0x5555;
    pRegister->CAN_REG17 = 0x5555;
    pRegister->CAN_REG18 = 0x5555;
    pRegister->CAN_REG08 = 0x0001;

    pRegister->CAN_REG10 = 0xAAAA;
    pRegister->CAN_REG11 = 0xAAAA;
    pRegister->CAN_REG12 = 0xAAAA;
    pRegister->CAN_REG13 = 0xAAAA;
    pRegister->CAN_REG14 = 0xAAAA;
    pRegister->CAN_REG15 = 0xAAAA;
    pRegister->CAN_REG16 = 0xAAAA;
    pRegister->CAN_REG17 = 0xAAAA;
    pRegister->CAN_REG18 = 0xAAAA;
    pRegister->CAN_REG08 = 0x0020;

    pRegister->CAN_REG88 = 0x0000;
    pRegister->CAN_REG89 = 0x8000;
    pRegister->CAN_REG64 = 0x0001;
    pRegister->CAN_REG65 = 0x0000;
    pRegister->CAN_REG72 = 0x0000;
    pRegister->CAN_REG73 = 0x8000;
    pRegister->INTPEND   = 0x0000;
    pRegister->CAN_REG81 = 0x8000;
    pRegister->CAN_REG32 = 0x0001;

}

void    NX_CAN_TRANSMIT_TEST ( U32 ModuleIndex )
{
    int i;
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

    pRegister->CAN_REG09 = 0x00F3;
    pRegister->CAN_REG08 = 0x0001;
    pRegister->CAN_REG08 = 0x0002;
    pRegister->CAN_REG08 = 0x0003;
    pRegister->CAN_REG08 = 0x0004;
    pRegister->CAN_REG08 = 0x0005;
    pRegister->CAN_REG08 = 0x0006;
    pRegister->CAN_REG08 = 0x0007;
    pRegister->CAN_REG08 = 0x0008;
    pRegister->CAN_REG08 = 0x0009;
    pRegister->CAN_REG08 = 0x000A;
    pRegister->CAN_REG08 = 0x000B;
    pRegister->CAN_REG08 = 0x000C;
    pRegister->CAN_REG08 = 0x000D;
    pRegister->CAN_REG08 = 0x000E;
    pRegister->CAN_REG08 = 0x000F;
    pRegister->CAN_REG08 = 0x0010;
    pRegister->CAN_REG08 = 0x0011;
    pRegister->CAN_REG08 = 0x0012;
    pRegister->CAN_REG08 = 0x0013;
    pRegister->CAN_REG08 = 0x0014;
    pRegister->CAN_REG08 = 0x0015;
    pRegister->CAN_REG08 = 0x0016;
    pRegister->CAN_REG08 = 0x0017;
    pRegister->CAN_REG08 = 0x0018;
    pRegister->CAN_REG08 = 0x0019;
    pRegister->CAN_REG08 = 0x001A;
    pRegister->CAN_REG08 = 0x001B;
    pRegister->CAN_REG08 = 0x001C;
    pRegister->CAN_REG08 = 0x001D;
    pRegister->CAN_REG08 = 0x001E;
    pRegister->CAN_REG08 = 0x001F;
    pRegister->CAN_REG08 = 0x0020;
    pRegister->CAN_REG00 = 0x0041;
    pRegister->CAN_REG03 = 0x2381;
    pRegister->CAN_REG00 = 0x000E;

    //for (i=0; i<10000; i++)
    //{
    //}

    pRegister->CAN_REG13 = 0xA444;
    pRegister->CAN_REG12 = 0x0000;
    pRegister->CAN_REG11 = 0xDFFF;
    pRegister->CAN_REG10 = 0xFFFF;
    pRegister->CAN_REG14 = 0x8188;
    pRegister->CAN_REG15 = 0x1514;
    pRegister->CAN_REG16 = 0x1716;
    pRegister->CAN_REG17 = 0x1918;
    pRegister->CAN_REG18 = 0x1B1A;
    pRegister->CAN_REG09 = 0x00F3;
    pRegister->CAN_REG08 = 0x8001;

    //for (i=0; i<1000000; i++)
    //{
    //}

}

void    NX_CAN_RD_STATUS_REG ( U32 ModuleIndex )
{
    int i;
	register NX_CAN_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

    regvalue = pRegister->CAN_REG01;
    pRegister->CAN_REG01 = 0x0007;

}


//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index
 *	@param[in]	ModuleIndex	an index of module.
 *	@param[in]	ChannelIndex refer to NX_CAN_DMACH_xxx in <nx_CAN.h>
 *	@return		DMA peripheral index.
 *	@see		NX_DMA_TransferMemToIO(DestinationPeriID),
 *				NX_DMA_TransferIOToMem(SourcePeriID)
 */
U32 NX_CAN_GetDMANumber ( U32 ModuleIndex , U32 ChannelIndex )
{
	//const U32 DMANumber[][NUMBER_OF_CAN_MODULE] =
	//{
	//    { DMAINDEX_WITH_CHANNEL_LIST( CAN, TXDMA ) }, // DMAINDEX_OF_CAN?_MODULE_TEST0
	//    { DMAINDEX_WITH_CHANNEL_LIST( CAN, RXDMA ) }, // DMAINDEX_OF_CAN?_MODULE_TEST1
	//};
	//NX_CASSERT( NUMBER_OF_CAN_MODULE == (sizeof(DMANumber[0])/sizeof(DMANumber[0][0])) );
	//NX_ASSERT( NUMBER_OF_CAN_MODULE > ModuleIndex );
	//NX_ASSERT( (sizeof(DMANumber)/sizeof(DMANumber[0])) > ChannelIndex );
	//// NX_ASSERT( DMAINDEX_OF_CAN0_MODULE_TEST0 == DMANumber[0][0] );
	//// NX_ASSERT( DMAINDEX_OF_CAN1_MODULE_TEST0 == DMANumber[1][0] );
	//// ...
	return 1;
}
#endif
