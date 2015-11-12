//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: tieoff Controller
//	File		: nx_tieoff.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_tieoff.h"

static struct NX_TIEOFF_RegisterSet *__g_pRegister;


CBOOL	NX_TIEOFF_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister =(struct NX_TIEOFF_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}


void    NX_TIEOFF_SetBaseAddress( void* BaseAddress )
{
    __g_pRegister = (struct NX_TIEOFF_RegisterSet *)BaseAddress;
}

U32     NX_TIEOFF_GetPhysicalAddress( void )
{
    const U32 PhysicalAddr[] =  {   PHY_BASEADDR_LIST( TIEOFF )  }; // PHY_BASEADDR_TIEOFF_MODULE

    NX_CASSERT( NUMBER_OF_TIEOFF_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );

    return (U32)PhysicalAddr[0]; 
}

//void		NX_TIEOFF_Set(TIEOFFREG_POS Position, TIEOFFREG_BITS BitWidth, U32 tieoff_value)
//{
//	U32 regindex, mask;
//	U32 LSB, MSB;
//	U32 regval;
//
//	regindex	= Position/32;
//	mask		= (0xffffffff<<MSB) | (0xffffffff<<(32-LSB));
//	regval		= (U32)__g_pRegister->TIEOFFREG[regindex] & mask;
//	regval		= regval & (tieoff_value & ~(0xffffffff<<BitWidth));
//	WriteIO32(&__g_pRegister->TIEOFFREG[regNum], regval);
//	
//}
//
//U32		NX_TIEOFF_Get(TIEOFFREG_POS Position, TIEOFFREG_BITS BitWidth)
//{
//	U32 regindex, bitPos, mask;
//	U32 LSB, MSB;
//	U32 regval;
//
//	regindex	= Position/32;
//	mask		= (0xffffffff<<MSB) | (0xffffffff<<(32-LSB));
//	regval		= (U32)__g_pRegister->TIEOFFREG[regindex];
//	regval		= (regval>>LSB) | ~(0xffffffff << BitWidth);
//	
//	return regval;
//}

void		NX_TIEOFF_Set(U32 tieoff_index, U32 tieoff_value)
{
	U32 regindex, mask;
	U32 LSB, MSB;
	U32 regval;

	U32 Position;
	U32 BitWidth;
	Position = tieoff_index & 0xffff;
	BitWidth = (tieoff_index>>16) & 0xffff;

	regindex	= Position/32;

	//LSB = Position%(32*regindex);
	LSB = Position%32;
	MSB = LSB+BitWidth;

	mask		= (0xffffffff<<MSB) | (~(0xffffffff<<LSB));
	regval		= (U32)__g_pRegister->TIEOFFREG[regindex] & mask;
	regval		= regval | ((tieoff_value & ~(0xffffffff<<BitWidth))<<LSB);
	WriteIO32(&__g_pRegister->TIEOFFREG[regindex], regval);
}

U32			NX_TIEOFF_Get(U32 tieoff_index)
{
	U32 regindex, mask;
	U32 LSB, MSB;
	U32 regval;

	U32 Position;
	U32 BitWidth;
	Position = tieoff_index & 0xffff;
	BitWidth = (tieoff_index>>16) & 0xffff;

	regindex	= Position/32;
	LSB = Position%32;
	MSB = LSB+BitWidth;

	mask		= (0xffffffff<<MSB) | ~(0xffffffff<<LSB);
	regval		= (U32)__g_pRegister->TIEOFFREG[regindex] & ~mask;
	regval		= (regval>>LSB);
	
	return regval;
}

