//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: Interrupt Controller
//	File		: nx_rstcon.c
//	Description	:
//	Author		:
//	History		:
//					2007/04/03	first
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

void		NX_RSTCON_SetnRST(U32 RSTIndex, RSTCON_nRST STATUS)
{
	U32 regNum, bitPos, curStat;
	regNum 		= RSTIndex >> 5;
	curStat		= (U32)ReadIO32(&__g_pRegister->REGRST[regNum]);
	bitPos		= RSTIndex & 0x1f;
	curStat		&= ~(1UL << bitPos);
	curStat		|= (STATUS & 0x01) << bitPos;
	WriteIO32(&__g_pRegister->REGRST[regNum], curStat);
}

void		NX_RSTCON_SetRST(U32 RSTIndex, RSTCON_RST STATUS)
{
	U32 regNum, bitPos, curStat;
	regNum 		= RSTIndex >> 5;
	curStat		= (U32)ReadIO32(&__g_pRegister->REGRST[regNum]);
	bitPos		= RSTIndex & 0x1f;
	curStat		&= ~(1UL << bitPos);
	curStat		|= (STATUS & 0x01) << bitPos;
	WriteIO32(&__g_pRegister->REGRST[regNum], curStat);
}

RSTCON_nRST		NX_RSTCON_GetnRST(U32 RSTIndex)
{
	U32 regNum, bitPos, curStat;
	regNum 		= RSTIndex >> 5;
	curStat		= (U32)ReadIO32(&__g_pRegister->REGRST[regNum]);
	bitPos	= RSTIndex & 0x1f;
	curStat		= 0x01 & (curStat >> bitPos);
	return (RSTCON_nRST) curStat;
}

RSTCON_RST		NX_RSTCON_GetRST(U32 RSTIndex)
{
	U32 regNum, bitPos, curStat;
	regNum 		= RSTIndex >> 5;
	curStat		= (U32)ReadIO32(&__g_pRegister->REGRST[regNum]);
	bitPos	= RSTIndex & 0x1f;
	curStat		= 0x01 & (curStat >> bitPos);

	return (RSTCON_RST) curStat;
}

