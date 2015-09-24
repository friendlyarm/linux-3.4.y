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
//	File		: nx_lvds.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_lvds.h"

//------------------
// DisplayTop
//-----------------
#ifdef NUMBER_OF_DISPLAYTOP_MODULE
#include "nx_displaytop.h"
#endif

#ifndef NX_CONSOLE_Printf
#define	NX_CONSOLE_Printf printf
#endif
#ifndef pow
static inline unsigned int pow(int a, int b) // a^b
{
	if (b == 0)
		return 1;
	else
		return a * pow(a, b - 1);
}
#endif

static	NX_LVDS_RegisterSet *__g_pRegister[NUMBER_OF_LVDS_MODULE];


//------------------------------------------------------------------------------
//
//	LVDS Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.
 *	@see	NX_LVDS_GetNumberOfModule
 */
CBOOL	NX_LVDS_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

    if( CFALSE == bInit )
    {
        for( i=0; i < NUMBER_OF_LVDS_MODULE; i++ )
        {
            __g_pRegister[i] = CNULL;
        }

        bInit = CTRUE;
    }

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. \n
 *				It is equal to NUMBER_OF_LVDS_MODULE in <nx_chip.h>.
 *	@see		NX_LVDS_Initialize
 */
U32		NX_LVDS_GetNumberOfModule( void )
{
	return NUMBER_OF_LVDS_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_LVDS_GetPhysicalAddress,
 *				NX_LVDS_SetBaseAddress,			NX_LVDS_GetBaseAddress,
 *				NX_LVDS_OpenModule,				NX_LVDS_CloseModule,
 *				NX_LVDS_CheckBusy,
 */
U32		NX_LVDS_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_LVDS_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_LVDS_GetPhysicalAddress,		NX_LVDS_GetSizeOfRegisterSet,
 *				NX_LVDS_GetBaseAddress,
 *				NX_LVDS_OpenModule,				NX_LVDS_CloseModule,
 *				NX_LVDS_CheckBusy,
 */
void	NX_LVDS_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_LVDS_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_LVDS_GetPhysicalAddress,		NX_LVDS_GetSizeOfRegisterSet,
 *				NX_LVDS_SetBaseAddress,
 *				NX_LVDS_OpenModule,				NX_LVDS_CloseModule,
 *				NX_LVDS_CheckBusy,
 */
U32		NX_LVDS_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	return (U32)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. \n
 *				It is equal to PHY_BASEADDR_LVDS?_MODULE in <nx_chip.h>.
 *	@see		NX_LVDS_GetSizeOfRegisterSet,
 *				NX_LVDS_SetBaseAddress,			NX_LVDS_GetBaseAddress,
 *				NX_LVDS_OpenModule,				NX_LVDS_CloseModule,
 *				NX_LVDS_CheckBusy,
 */
U32		NX_LVDS_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_LVDS_MODULE] =
	{
		PHY_BASEADDR_LIST( LVDS )
	};
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 *	@see		NX_LVDS_GetPhysicalAddress,		NX_LVDS_GetSizeOfRegisterSet,
 *				NX_LVDS_SetBaseAddress,			NX_LVDS_GetBaseAddress,
 *				NX_LVDS_CloseModule,
 *				NX_LVDS_CheckBusy,
 */
CBOOL	NX_LVDS_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_LVDS_GetPhysicalAddress,		NX_LVDS_GetSizeOfRegisterSet,
 *				NX_LVDS_SetBaseAddress,			NX_LVDS_GetBaseAddress,
 *				NX_LVDS_OpenModule,
 *				NX_LVDS_CheckBusy,
 */
CBOOL	NX_LVDS_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_LVDS_GetPhysicalAddress,		NX_LVDS_GetSizeOfRegisterSet,
 *				NX_LVDS_SetBaseAddress,			NX_LVDS_GetBaseAddress,
 *				NX_LVDS_OpenModule,				NX_LVDS_CloseModule,
 */
CBOOL	NX_LVDS_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_LVDS?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_LVDS_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( LVDS, I_RESETN )
	};
	NX_CASSERT( NUMBER_OF_LVDS_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
///	@name	Initial Register Test
//------------------------------------------------------------------------------
//@{
#if defined(LVDS_REISTER_TEST)
CBOOL NX_LVDS_InitRegTest( U32 ModuleIndex )
{
	register NX_LVDS_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	U32 fail = 0;

	WriteIODW(&pRegister->LVDSCTRL0, 0x07fffE);
	regvalue = ReadIODW(&pRegister->LVDSCTRL0);
	if( regvalue != 0x07fffE )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSCTRL0 regvalue = %x, Golden ( 0x07fffE )", regvalue);
	}

	WriteIODW(&pRegister->LVDSCTRL1, 0x03ffff);
	regvalue = ReadIODW(&pRegister->LVDSCTRL1);
	if( regvalue != 0x03ffff )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSCTRL1 regvalue = %x, Golden ( 0x03ffff )", regvalue);
	}

	WriteIODW(&pRegister->LVDSCTRL2, 0x0ffff);
	regvalue = ReadIODW(&pRegister->LVDSCTRL2);
	if( regvalue != 0x0ffff )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSCTRL2 regvalue = %x, Golden ( 0x0ffff )", regvalue);
	}


	WriteIODW(&pRegister->LVDSCTRL3, 0x03ff);
	regvalue = ReadIODW(&pRegister->LVDSCTRL3);
	if( regvalue != 0x03ff )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSCTRL3 regvalue = %x, Golden ( 0x03ff )", regvalue);
	}


	WriteIODW(&pRegister->LVDSCTRL4, 0x1fffffff);
	regvalue = ReadIODW(&pRegister->LVDSCTRL4);
	if( regvalue != 0x1fffffff )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSCTRL4 regvalue = %x, Golden ( 0x1fffffff )", regvalue);
	}

	//@added choiyk 2012-12-17 오전 11:21:54
	WriteIODW(&pRegister->LVDSLOC0, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC0);
	if( regvalue != 0x3fffffff )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC0 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}
	WriteIODW(&pRegister->LVDSLOC1, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC1);
	if( regvalue != 0x3fffffff  )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC1 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}
	WriteIODW(&pRegister->LVDSLOC2, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC2);
	if( regvalue != 0x3fffffff  )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC2 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}

	WriteIODW(&pRegister->LVDSLOC3, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC3);
	if( regvalue != 0x3fffffff  )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC3 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}
	WriteIODW(&pRegister->LVDSLOC4, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC4);
	if( regvalue != 0x3fffffff  )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC4 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}
	WriteIODW(&pRegister->LVDSLOC5, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC5);
	if( regvalue != 0x3fffffff  )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC5 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}
	WriteIODW(&pRegister->LVDSLOC6, (pow(2,30)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOC6);
	if( regvalue != 0x3fffffff )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOC6 regvalue = %x, Golden ( %x )", regvalue, 0x3fffffff );
	}

	WriteIODW(&pRegister->LVDSLOCMASK0, (pow(2,32)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOCMASK0);
	if( regvalue != (pow(2,32)-1) )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOCMASK0 regvalue = %x, Golden ( %x )", regvalue, (pow(2,32)-1));
	}
	WriteIODW(&pRegister->LVDSLOCMASK1, 7 );
	regvalue = ReadIODW(&pRegister->LVDSLOCMASK1);
	if( regvalue != 7 )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] regvalue = %x, Golden ( %x )", regvalue, 7 );
	}

	WriteIODW(&pRegister->LVDSLOCPOL0, (pow(2,32)-1)  );
	regvalue = ReadIODW(&pRegister->LVDSLOCPOL0);
	if( regvalue != (pow(2,32)-1) )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOCPOL0 regvalue = %x, Golden ( %x )", regvalue, (pow(2,32)-1));
	}
	WriteIODW(&pRegister->LVDSLOCPOL1, 7 );
	regvalue = ReadIODW(&pRegister->LVDSLOCPOL1);
	if( regvalue != 7 )
	{
		fail = 1; //return CFALSE;
		NX_CONSOLE_Printf("\n[DEBUG] LVDSLOCPOL1 regvalue = %x, Golden ( %x )", regvalue, 7 );
	}







	if( fail == 1 )
	{
		return CFALSE;
	}

	return CTRUE; // CTRUE
}
#endif

void NX_LVDS_SetLVDSCTRL0( U32 ModuleIndex, U32 regvalue )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSCTRL0, regvalue);
}

void NX_LVDS_SetLVDSCTRL1( U32 ModuleIndex, U32 regvalue )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSCTRL1, regvalue);
}

void NX_LVDS_SetLVDSCTRL2( U32 ModuleIndex, U32 regvalue )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSCTRL2, regvalue);
}
void NX_LVDS_SetLVDSCTRL3( U32 ModuleIndex, U32 regvalue )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSCTRL3, regvalue);
}
void NX_LVDS_SetLVDSCTRL4( U32 ModuleIndex, U32 regvalue )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSCTRL4, regvalue);
}


void NX_LVDS_SetLVDSTMODE0( U32 ModuleIndex, U32 regvalue )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSTMODE0, regvalue);
}


void NX_LVDS_SetLVDSLOC0		 ( U32 ModuleIndex, U32 regvalue ) //'h20
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC0, regvalue);
}
void NX_LVDS_SetLVDSLOC1		 ( U32 ModuleIndex, U32 regvalue ) //'h24
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC1, regvalue);
}

void NX_LVDS_SetLVDSLOC2		 ( U32 ModuleIndex, U32 regvalue ) //'h28
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC2, regvalue);
}

void NX_LVDS_SetLVDSLOC3		 ( U32 ModuleIndex, U32 regvalue ) //'h2C
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC3, regvalue);
}

void NX_LVDS_SetLVDSLOC4		 ( U32 ModuleIndex, U32 regvalue ) //'h30
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC4, regvalue);
}

void NX_LVDS_SetLVDSLOC5		 ( U32 ModuleIndex, U32 regvalue ) //'h34
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC5, regvalue);
}

void NX_LVDS_SetLVDSLOC6		 ( U32 ModuleIndex, U32 regvalue ) //'h38
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOC6, regvalue);
}

void NX_LVDS_SetLVDSLOCMASK0 ( U32 ModuleIndex, U32 regvalue ) //'h40
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOCMASK0, regvalue);
}

void NX_LVDS_SetLVDSLOCMASK1 ( U32 ModuleIndex, U32 regvalue ) //'h44
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOCMASK1, regvalue);
}

void NX_LVDS_SetLVDSLOCPOL0  ( U32 ModuleIndex, U32 regvalue ) //'h48
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOCPOL0, regvalue);
}

void NX_LVDS_SetLVDSLOCPOL1  ( U32 ModuleIndex, U32 regvalue ) //'h4c
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	WriteIODW(&pRegister->LVDSLOCPOL1, regvalue);
}




//========================================
// ECO for LVDS self test.
// Never Use i_PCLK in ECO Script!!!!!!
//@modified choiyk 2013-01-29 오후 3:55:56
//========================================
void NX_LVDS_SetLVDSDUMMY  ( U32 ModuleIndex, U32 regvalue ) //'h4c
{
	register NX_LVDS_RegisterSet* pRegister;
	U32 oldvalue;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	oldvalue = ReadIODW(&pRegister->LVDSCTRL1) & 0x00FFFFFF;
	WriteIODW(&pRegister->LVDSCTRL1, oldvalue | ((regvalue&0xFF) << 24 ) );
}

U32 NX_LVDS_GetLVDSDUMMY  ( U32 ModuleIndex ) //'h4c
{
	register NX_LVDS_RegisterSet* pRegister;
	U32 oldvalue;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	oldvalue = ReadIODW(&pRegister->LVDSCTRL1) ;
	oldvalue = oldvalue >> 24 ;
	return oldvalue;
}




U32 NX_LVDS_GetLVDSCTRL0( U32 ModuleIndex )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (U32) ReadIODW(&pRegister->LVDSCTRL0);
}


U32 NX_LVDS_GetLVDSCTRL1( U32 ModuleIndex )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (U32) ReadIODW(&pRegister->LVDSCTRL1);
}

U32 NX_LVDS_GetLVDSCTRL2( U32 ModuleIndex )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (U32) ReadIODW(&pRegister->LVDSCTRL2);
}

U32 NX_LVDS_GetLVDSCTRL3( U32 ModuleIndex )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (U32) ReadIODW(&pRegister->LVDSCTRL3);
}

U32 NX_LVDS_GetLVDSCTRL4( U32 ModuleIndex )
{
	register NX_LVDS_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_LVDS_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	return (U32) ReadIODW(&pRegister->LVDSCTRL4);
}
