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
//	File		: nx_mali400.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_mali400.h"
#include <string.h> // for memset

static	NX_MALI400_RegisterSet *__g_pRegister[NUMBER_OF_MALI400_MODULE];

#define NUMBEROF_PP 2	

//------------------------------------------------------------------------------
//
//	MALI400 Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	It is a sample function
 *	@return a register value
 */
U32   NX_MALI400_GetTEMP( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return __g_pRegister[ModuleIndex]->REG_CONTROL[0x002C/4];
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
 *	@see	NX_MALI400_GetNumberOfModule
 */
CBOOL	NX_MALI400_Initialize( void )
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
 *				It is equal to NUMBER_OF_MALI400_MODULE in <nx_chip.h>.
 *	@see		NX_MALI400_Initialize
 */
U32		NX_MALI400_GetNumberOfModule( void )
{
	return NUMBER_OF_MALI400_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.			
 */
U32		NX_MALI400_GetSizeOfRegisterSet( void )
{
return sizeof( *__g_pRegister );	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.				
 */

void	NX_MALI400_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_MALI400_RegisterSet *)BaseAddress;	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.				
 */

void*	NX_MALI400_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );

	return (void*)__g_pRegister[ModuleIndex];	

}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_MALI400?_MODULE in <nx_chip.h>.				
 */
U32		NX_MALI400_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_MALI400_MODULE] =
	{
		PHY_BASEADDR_LIST( MALI400 )
	};
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];
		
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.				
 */
CBOOL	NX_MALI400_OpenModule( U32 ModuleIndex )
{
	int i;
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    
	//for( i=0; i<15; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x0080/4] |= (0xF<<i); }
	//for( i=0; i< 3; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x0088/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x0090/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x0098/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x00A0/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x00A8/4] |= (0xF<<i); }
    //
	//for( i=0; i<15; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x0084/4] |= (0xF<<i); }
	//for( i=0; i< 3; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x008C/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x0094/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x009C/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x00A4/4] |= (0xF<<i); }
	//for( i=0; i<32; i+=4 ) { __g_pRegister[ModuleIndex]->REG_PMU[0x00AC/4] |= (0xF<<i); }	    
    //
    
    __g_pRegister[ModuleIndex]->REG_PMU[0x0018/4] = 1; // clear power up interrupt
    __g_pRegister[ModuleIndex]->REG_PMU[0x000C/4] = 0; // mask interrupt
    __g_pRegister[ModuleIndex]->REG_PMU[0x0000/4] = 0xFF; // power up
    while( 0 == (__g_pRegister[ModuleIndex]->REG_PMU[0x0010/4]&1) ){ } // check power up interrupt
    __g_pRegister[ModuleIndex]->REG_PMU[0x0018/4] = 1; // clear power up interrupt
    
    __g_pRegister[ModuleIndex]->REG_L2 [0x0018/4] = 4; // outstanding-transaction-cap

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.				
 */
CBOOL	NX_MALI400_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
   
    __g_pRegister[ModuleIndex]->REG_PMU[0x0018/4] = 1; // clear power up interrupt
    __g_pRegister[ModuleIndex]->REG_PMU[0x000C/4] = 0; // mask interrupt
    __g_pRegister[ModuleIndex]->REG_PMU[0x0004/4] = 0xFF; // power down
    while( ((1<<(NUMBEROF_PP+2))-1) != __g_pRegister[ModuleIndex]->REG_PMU[0x0008/4] ){ } // check sleep status
    __g_pRegister[ModuleIndex]->REG_PMU[0x0018/4] = 1; // clear power up interrupt
    //
	//__g_pRegister[ModuleIndex]->REG_PMU[0x0084/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x008C/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x0094/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x009C/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x00A4/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x00AC/4] = 0;
    //
	//__g_pRegister[ModuleIndex]->REG_PMU[0x0080/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x0088/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x0090/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x0098/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x00A0/4] = 0;
	//__g_pRegister[ModuleIndex]->REG_PMU[0x00A8/4] = 0;

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_MALI400_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
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
 *				It is equal to CLOCKINDEX_OF_MALI400?_MODULE in <nx_chip.h>.
 */
U32 NX_MALI400_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( MALI400 )
	};
	NX_CASSERT( NUMBER_OF_MALI400_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );  
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_MALI400?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_MALI400_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( MALI400, nRST )
	};
	NX_CASSERT( NUMBER_OF_MALI400_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );  
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_MALI400?_MODULE in <nx_chip.h>.
 */
U32 	NX_MALI400_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( MALI400 ) };
    NX_CASSERT( NUMBER_OF_MALI400_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );  
    NX_ASSERT( NUMBER_OF_MALI400_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_MALI4000_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}
