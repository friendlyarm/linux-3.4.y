//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: Display Control by neo
//	File		: nx_displaytop.h
//	Description	:
//	Author		: Prototype by choiyk
//	History		:
//------------------------------------------------------------------------------

#ifndef __NX_LCDINTERFACE_H__
#define __NX_LCDINTERFACE_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	DISPLAYTOP
//------------------------------------------------------------------------------
//@{




//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
	volatile U32 DISPCNTL0;    // 0x00
	volatile U32 DISPCNTL1;    // 0x04
	volatile U32 DISPDELYCNT0; // 0x08
	volatile U32 _Reserved0;   // 0x0C
	volatile U32 DISPPADLOC0;  // 0x10
	volatile U32 DISPPADLOC1;  // 0x14
	volatile U32 DISPPADLOC2;  // 0x18
	volatile U32 DISPPADLOC3;  // 0x1C
	volatile U32 DISPRGBMASK;  // 0x20
	volatile U32 _Reserved1[3]; // 0x24, 0x28, 0x2C
	volatile U32 I80DATARW;    // 0x30
	volatile U32 I80REGRW;     // 0x34
	volatile U32 I80TIMING;    // 0x38
	volatile U32 I80POLCTRL;
	volatile U32 DISPCMDBUFCTRL0;
	volatile U32 DISPCMDBUFDATA;
	volatile U32 DISPSOURCESEL; // 0x48
//@fixed choiyk 2012-10-30 오후 2:27:35 LCDIF내에 CLKGEN이 같이 포함되지 않고
//별도로 존재한다.
//	volatile U32 _Reserved2[(0xfc0-0x04c)/4] ;
//	volatile U32 CLKENB                     ;   // 4096 : FC0, CLKGEN
//	volatile U32 CLKGEN[4]                  ;
} NX_LCDINTERFACE_RegisterSet;


//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_LCDINTERFACE_Initialize( void );
U32   NX_LCDINTERFACE_GetNumberOfModule( void );
U32   NX_LCDINTERFACE_GetSizeOfRegisterSet( void );
void  NX_LCDINTERFACE_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void* NX_LCDINTERFACE_GetBaseAddress( U32 ModuleIndex );
U32   NX_LCDINTERFACE_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_LCDINTERFACE_OpenModule( U32 ModuleIndex );
CBOOL NX_LCDINTERFACE_CloseModule( U32 ModuleIndex );
CBOOL NX_LCDINTERFACE_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
// CLKGEN 을 포함하고 있다.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{

#ifndef __DEF__NX_DISPLAYTOP_RESET__
#define __DEF__NX_DISPLAYTOP_RESET__
typedef enum
{
    NX_DISPLAYTOP_RESET_nRST 			= 0		,///< CLKGEN Reset(All)
    NX_DISPLAYTOP_RESET_Disp0RESETN 	= 1		,///< test0 interrupt
    NX_DISPLAYTOP_RESET_Disp1RESETN 	= 2		,///< test0 interrupt
    NX_DISPLAYTOP_RESET_ResConvRESETN 	= 3		,///< test0 interrupt
    NX_DISPLAYTOP_RESET_LCDIFRESETN 	= 4		,///< test0 interrupt
    NX_DISPLAYTOP_RESET_InterConvRESETN	= 5		,///< test0 interrupt
    NX_DISPLAYTOP_RESET_HDMIRESETN 		= 6 	,///< test0 interrupt
} NX_DISPLAYTOP_RESET;
#endif

U32 NX_LCDINTERFACE_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
// Interrupt Not Use.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
///	@name	DMA Interface
// DMA Not Use.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
///	@name	PAD Interface
// PAD Not Use.
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Default Use Function
//------------------------------------------------------------------------------


#ifndef __DEF__NX_LCDINTERFACE_PADCLK
#define __DEF__NX_LCDINTERFACE_PADCLK
typedef enum
{
	NX_LCDINTERFACE_PADCLK_VCLK			= 0UL,	///< VCLK
	NX_LCDINTERFACE_PADCLK_VCLK2		= 1UL,	///< VCLK2
	NX_LCDINTERFACE_PADCLK_VCLK3		= 2UL	///< VCLK3

}	NX_LCDINTERFACE_PADCLK;
#endif

#ifndef __DEF__NX_LCDINTERFACE_FORMAT
#define __DEF__NX_LCDINTERFACE_FORMAT
// 기본적으로 제공하는 PAD Loc 설정값.
// 그외에는 PADLOC를 직접 설정하여야 한다.
typedef	enum
{
	NX_LCDINTERFACE_FORMAT_RGB555		= 0UL,	///< RGB555 Format
	NX_LCDINTERFACE_FORMAT_RGB565		= 1UL,	///< RGB565 Format
	NX_LCDINTERFACE_FORMAT_RGB666		= 2UL,	///< RGB666 Format
	NX_LCDINTERFACE_FORMAT_RGB888		= 3UL,	///< RGB888 Format
	NX_LCDINTERFACE_FORMAT_MRGB565		= 4UL,	///< MRGB565 Format
	NX_LCDINTERFACE_FORMAT_SRGB888		= 5UL,	///< SRGB888 Format
	NX_LCDINTERFACE_FORMAT_SRGBD8888	= 6UL,	///< SRGBD8888 Format
	NX_LCDINTERFACE_FORMAT_RGB565_I80   = 7UL,  ///< RGB565 Format For I80

}	NX_LCDINTERFACE_FORMAT;
#endif

#ifndef __DEF__NX_LCDINTERFACE_SERIAL_FORMAT
#define __DEF__NX_LCDINTERFACE_SERIAL_FORMAT
// 기본적으로 제공하는 PAD Loc 설정값.
// 그외에는 PADLOC를 직접 설정하여야 한다.
typedef	enum
{
	NX_LCDINTERFACE_SERIAL_FORMAT_RGB	= 0UL,	///< ex) RGB555   Format
	NX_LCDINTERFACE_SERIAL_FORMAT_MRGB	= 1UL,	///< ex) MRGB565  Format
	NX_LCDINTERFACE_SERIAL_FORMAT_SRGB3	= 2UL,	///< ex) SRGB888  Format
	NX_LCDINTERFACE_SERIAL_FORMAT_SRGB4	= 3UL,	///< ex) SRGB8888 Format
}	NX_LCDINTERFACE_SERIAL_FORMAT;
#endif


#ifndef __DEF__NX_LCDINTERFACE_DITHER
#define __DEF__NX_LCDINTERFACE_DITHER
typedef	enum
{
	NX_LCDINTERFACE_DITHER_BYPASS	= 0UL,	///< Bypass mode.
	NX_LCDINTERFACE_DITHER_4BIT		= 1UL,	///< 8 bit -> 4 bit mode.
	NX_LCDINTERFACE_DITHER_5BIT		= 2UL,	///< 8 bit -> 5 bit mode.
	NX_LCDINTERFACE_DITHER_6BIT		= 3UL	///< 8 bit -> 6 bit mode.

}	NX_LCDINTERFACE_DITHER;
#endif



void	NX_LCDINTERFACE_SetLCDIFEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_LCDINTERFACE_GetLCDIFEnable( U32 ModuleIndex );

// RGB Delay는 없음.
void	NX_LCDINTERFACE_SetDelay( U32 ModuleIndex, U32 DelayHS, U32 DelayVS, U32 DelayDE );
void	NX_LCDINTERFACE_GetDelay( U32 ModuleIndex, U32 *DelayHS, U32 *DelayVS, U32 *DelayDE );


void	NX_LCDINTERFACE_SetDither( U32 ModuleIndex, NX_LCDINTERFACE_DITHER DitherR, NX_LCDINTERFACE_DITHER DitherG, NX_LCDINTERFACE_DITHER DitherB );
void	NX_LCDINTERFACE_GetDither( U32 ModuleIndex, NX_LCDINTERFACE_DITHER *pDitherR, NX_LCDINTERFACE_DITHER *pDitherG, NX_LCDINTERFACE_DITHER *pDitherB );



/*
  Input RGB n
-------------------------------------------------------------------------
|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|
|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
-------------------------------------------------------------------------
*/

void	NX_LCDINTERFACE_SetRGBMode( U32 ModuleIndex,
							NX_LCDINTERFACE_FORMAT format );


void	NX_LCDINTERFACE_SetRGBShift( U32 ModuleIndex, U32 RGBSHIFT );


U32		NX_LCDINTERFACE_GetRGBShift( U32 ModuleIndex);

void	NX_LCDINTERFACE_SetSerialFormat( U32 ModuleIndex, NX_LCDINTERFACE_SERIAL_FORMAT SerialFormat );
NX_LCDINTERFACE_SERIAL_FORMAT	NX_LCDINTERFACE_GetSerialFormat( U32 ModuleIndex);


// new functions
void	NX_LCDINTERFACE_SetTFTPolarity( U32 ModuleIndex, CBOOL PHS, CBOOL PVS, CBOOL PDE);
void	NX_LCDINTERFACE_GetTFTPolarity( U32 ModuleIndex, CBOOL *PHS, CBOOL *PVS, CBOOL *PDE);

void	NX_LCDINTERFACE_SetDirtyFlag( U32 ModuleIndex, CBOOL Enable);
CBOOL	NX_LCDINTERFACE_GetDirtyFlagClear( U32 ModuleIndex);


// ---- i80 Interface -----
void	NX_LCDINTERFACE_Seti80Mode( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_LCDINTERFACE_Geti80Mode( U32 ModuleIndex);


void	NX_LCDINTERFACE_SetReg( U32 ModuleIndex, U32 Data ); // RS==1
U32		NX_LCDINTERFACE_GetStatus( U32 ModuleIndex ); // RS==1

void	NX_LCDINTERFACE_SetData( U32 ModuleIndex, U32 Data ); // RS==0
U32		NX_LCDINTERFACE_GetData( U32 ModuleIndex ); // RS==0

void	NX_LCDINTERFACE_Set_i80Timing( U32 ModuleIndex, U32 tSetup, U32 tHold, U32 tAcc);
void	NX_LCDINTERFACE_Get_i80Timing( U32 ModuleIndex, U32 *tSetup, U32 *tHold, U32 *tAcc);

//@added choiyk 2012-10-30 오후 3:41:26
void NX_LCDINTERFACE_SetCmdBufferClear( U32 ModuleIndex);
void NX_LCDINTERFACE_SetCmdBufferWrite( U32 ModuleIndex, U32 regvalue);
void NX_LCDINTERFACE_SetCmdBufferFlush( U32 ModuleIndex, CBOOL Enb );


CBOOL NX_LCDINTERFACE_GetInterruptPendingAll( U32 ModuleIndex );
void NX_LCDINTERFACE_ClearInterruptPendingAll( U32 ModuleIndex );

//--------
//Output Video Clock Selection Register
//0 : PADVCLK	1 : PADPixelCLK
//2 : VCLK/2
//3-7: Reserved
//--------

typedef enum {
 NX_LCDIF_PADPixelCLKxN = 0,
 NX_LCDIF_PADPixelCLK = 1,
 NX_LCDIF_PixelCLKxN_DIV2 = 2, // SRGB888의 경우 이 클럭을 선택해야 한다.
}PADCLK_SELECT;

void NX_LCDINTERFACE_SetOutputClock( U32 ModuleIndex, U32 SourceSel );

//------------------------------------------------------------------------------
// Default Register Function
//------------------------------------------------------------------------------
void NX_LCDINTERFACE_Set_DISPCNTL0		( U32 ModuleIndex, U32 regvalue );
U32  NX_LCDINTERFACE_Get_DISPCNTL0		( U32 ModuleIndex  );
void NX_LCDINTERFACE_Set_DISPCNTL1      ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPDELYCNT0   ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPPADLOC0    ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPPADLOC1    ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPPADLOC2    ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPPADLOC3    ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPRGBMASK    ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_I80DATARW      ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_I80REGRW       ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_I80TIMING      ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_I80POLCTRL     ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPCMDBUFCTRL0( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPCMDBUFDATA ( U32 ModuleIndex, U32 regvalue );
void NX_LCDINTERFACE_Set_DISPSOURCESEL  ( U32 ModuleIndex, U32 regvalue );




//*********************************************************************************************
// CLKGEN
////*********************************************************************************************
//void		NX_LCDINTERFACE_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode );
//NX_PCLKMODE	NX_LCDINTERFACE_GetClockPClkMode( U32 ModuleIndex );
//void		NX_LCDINTERFACE_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc );
//U32			NX_LCDINTERFACE_GetClockSource( U32 ModuleIndex, U32 Index );
//void		NX_LCDINTERFACE_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor );
//U32			NX_LCDINTERFACE_GetClockDivisor( U32 ModuleIndex, U32 Index );
//void		NX_LCDINTERFACE_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable );
//CBOOL		NX_LCDINTERFACE_GetClockDivisorEnable( U32 ModuleIndex );
//
//void		NX_LCDINTERFACE_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode );
//NX_BCLKMODE	NX_LCDINTERFACE_GetClockBClkMode( U32 ModuleIndex );
//
////void		NX_LCDINTERFACE_SetClockInInv( U32 ModuleIndex, CBOOL OutClkInv );
////CBOOL		NX_LCDINTERFACE_GetClockInInv( U32 ModuleIndex );
//
//void		NX_LCDINTERFACE_SetClockOutInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
//CBOOL		NX_LCDINTERFACE_GetClockOutInv( U32 ModuleIndex, U32 Index );
//
//CBOOL		NX_LCDINTERFACE_SetInputInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
//CBOOL		NX_LCDINTERFACE_GetInputInv( U32 ModuleIndex, U32 Index );
////*********************************************************************************************




#ifdef	__cplusplus
}
#endif


#endif // __NX_LCDINTERFACE_H__










