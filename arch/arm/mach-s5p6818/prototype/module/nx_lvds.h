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
//	File		: nx_lvds.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_LVDS_H__
#define __NX_LVDS_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif


//------------------------------------------------------------------------------
/// @defgroup	LVDS
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map

















typedef struct
{
		volatile U32 LVDSCTRL0;       //'h00
    volatile U32 LVDSCTRL1;       //'h04
    volatile U32 LVDSCTRL2;       //'h08
    volatile U32 LVDSCTRL3;       //'h0c
    volatile U32 LVDSCTRL4;       //'h10
    volatile U32 _Reserved0[3];   //'h14--'h1c
    volatile U32 LVDSLOC0			;   //'h20
    volatile U32 LVDSLOC1		  ;   //'h24
    volatile U32 LVDSLOC2		  ;   //'h28
    volatile U32 LVDSLOC3		  ;   //'h2C
    volatile U32 LVDSLOC4		  ;   //'h30
    volatile U32 LVDSLOC5		  ;   //'h34
    volatile U32 LVDSLOC6		  ;   //'h38
    volatile U32 _Reserved1   ;   //'h3c
    volatile U32 LVDSLOCMASK0 ;   //'h40
    volatile U32 LVDSLOCMASK1 ;   //'h44
    volatile U32 LVDSLOCPOL0  ;   //'h48
    volatile U32 LVDSLOCPOL1  ;   //'h4c
    volatile U32 LVDSTMODE0   ;   //'h50
    volatile U32 LVDSTMODE1   ;   //'h54

    volatile U32 _Reserved2[2];   //'h58--'h5C
	//========================================
	// ECO for LVDS self test.
	// Never Use i_PCLK in ECO Script!!!!!!
	//@modified choiyk 2013-01-29 오후 3:55:56
	//========================================

} NX_LVDS_RegisterSet;

//------------------------------------------------------------------------------
///	@name	LVDS Interface
//------------------------------------------------------------------------------
//@{

//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_LVDS_Initialize( void );
U32   NX_LVDS_GetNumberOfModule( void );

U32   NX_LVDS_GetSizeOfRegisterSet( void );

void  NX_LVDS_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void* NX_LVDS_GetBaseAddress( U32 ModuleIndex );

U32   NX_LVDS_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_LVDS_OpenModule( U32 ModuleIndex );
CBOOL NX_LVDS_CloseModule( U32 ModuleIndex );
CBOOL NX_LVDS_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_LVDS_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Initial Register Test
//------------------------------------------------------------------------------
//@{
//#define LVDS_REISTER_TEST
#if defined(LVDS_REISTER_TEST)
CBOOL NX_LVDS_InitRegTest( U32 ModuleIndex );
#endif
//@}

//------------------------------------------------------------------------------
///	@name Register Set Function
//------------------------------------------------------------------------------
//@{
void NX_LVDS_SetLVDSCTRL0( U32 ModuleIndex, U32 regvalue );
void NX_LVDS_SetLVDSCTRL1( U32 ModuleIndex, U32 regvalue );
void NX_LVDS_SetLVDSCTRL2( U32 ModuleIndex, U32 regvalue );
void NX_LVDS_SetLVDSCTRL3( U32 ModuleIndex, U32 regvalue );
void NX_LVDS_SetLVDSCTRL4( U32 ModuleIndex, U32 regvalue );

U32 NX_LVDS_GetLVDSCTRL0( U32 ModuleIndex );
U32 NX_LVDS_GetLVDSCTRL1( U32 ModuleIndex );
U32 NX_LVDS_GetLVDSCTRL2( U32 ModuleIndex );
U32 NX_LVDS_GetLVDSCTRL3( U32 ModuleIndex );
U32 NX_LVDS_GetLVDSCTRL4( U32 ModuleIndex );


void NX_LVDS_SetLVDSTMODE0( U32 ModuleIndex, U32 regvalue );

void NX_LVDS_SetLVDSLOC0		 ( U32 ModuleIndex, U32 regvalue ); //'h20
void NX_LVDS_SetLVDSLOC1		 ( U32 ModuleIndex, U32 regvalue ); //'h24
void NX_LVDS_SetLVDSLOC2		 ( U32 ModuleIndex, U32 regvalue ); //'h28
void NX_LVDS_SetLVDSLOC3		 ( U32 ModuleIndex, U32 regvalue ); //'h2C
void NX_LVDS_SetLVDSLOC4		 ( U32 ModuleIndex, U32 regvalue ); //'h30
void NX_LVDS_SetLVDSLOC5		 ( U32 ModuleIndex, U32 regvalue ); //'h34
void NX_LVDS_SetLVDSLOC6		 ( U32 ModuleIndex, U32 regvalue ); //'h38
void NX_LVDS_SetLVDSLOCMASK0 ( U32 ModuleIndex, U32 regvalue ); //'h40
void NX_LVDS_SetLVDSLOCMASK1 ( U32 ModuleIndex, U32 regvalue ); //'h44
void NX_LVDS_SetLVDSLOCPOL0  ( U32 ModuleIndex, U32 regvalue ); //'h48
void NX_LVDS_SetLVDSLOCPOL1  ( U32 ModuleIndex, U32 regvalue ); //'h4c


void NX_LVDS_SetLVDSLOCPOL1  ( U32 ModuleIndex, U32 regvalue ); //'h4c



//========================================
// ECO for LVDS self test.
// Never Use i_PCLK in ECO Script!!!!!!
//@modified choiyk 2013-01-29 오후 3:55:56
//========================================
void NX_LVDS_SetLVDSDUMMY( U32 ModuleIndex, U32 regvalue );
U32 NX_LVDS_GetLVDSDUMMY  ( U32 ModuleIndex ); //'h4c


//@}


#ifdef	__cplusplus
}
#endif


#endif // __NX_LVDS_H__
