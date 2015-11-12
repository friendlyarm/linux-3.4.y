//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2009
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: reset controller
//	File		: nx_rstcon.c
//	Description	:
//	Author		:
//	History		:
//					2014/08/05	first
//------------------------------------------------------------------------------
#include "nx_rstcon.h"

static struct NX_RSTCON_RegisterSet *__g_pRegister;


CBOOL	NX_RSTCON_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = CNULL;
		bInit = CTRUE;
	}
	return CTRUE;
}

U32  NX_RSTCON_GetPhysicalAddress( void )
{
    const U32 PhysicalAddr[] =  {   PHY_BASEADDR_LIST( RSTCON )  }; // PHY_BASEADDR_RSTCON_MODULE
    NX_CASSERT( NUMBER_OF_RSTCON_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( PHY_BASEADDR_RSTCON_MODULE == PhysicalAddr[0] );

	return (U32)PhysicalAddr[0];
}

U32	 NX_RSTCON_GetSizeOfRegisterSet( void )
{
	return sizeof(struct NX_RSTCON_RegisterSet);
}


void NX_RSTCON_SetBaseAddress( void* BaseAddress )

{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_RSTCON_RegisterSet *)BaseAddress;
}


void* NX_RSTCON_GetBaseAddress( void )

{

	return (void*)__g_pRegister;

}

void	NX_RSTCON_SetRST(U32 RSTIndex, RSTCON STATUS)
{
	U32 regvalue;

	NX_ASSERT(NUMBER_OF_RESET_MODULE_PIN > RSTIndex);
	NX_ASSERT((RSTCON_ASSERT == STATUS) || (RSTCON_NEGATE == STATUS));

	regvalue	= ReadIO32(&__g_pRegister->REGRST[RSTIndex >> 5]);

	regvalue	&= ~(1UL << (RSTIndex & 0x1f));
	regvalue	|= (STATUS & 0x01) << (RSTIndex & 0x1f);

	WriteIO32(&__g_pRegister->REGRST[RSTIndex >> 5], regvalue);
}

RSTCON	NX_RSTCON_GetRST(U32 RSTIndex)
{
	NX_ASSERT(NUMBER_OF_RESET_MODULE_PIN > RSTIndex);

	return	(RSTCON)((ReadIO32(&__g_pRegister->REGRST[RSTIndex >> 5])>> (RSTIndex & 0x1f)) & 0x1);
}

