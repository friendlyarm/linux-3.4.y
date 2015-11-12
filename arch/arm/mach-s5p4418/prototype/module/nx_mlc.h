//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: MLC
//	File		: nx_mlc.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------

#ifndef __NX_MLC_H__
#define __NX_MLC_H__

#include "nx_dualdisplay.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	MLC MLC
//------------------------------------------------------------------------------
//@{

/// @brief	MLC Module's Register List
struct	NX_MLC_RegisterSet
{
	volatile U32 MLCCONTROLT;				///< 0x00 : MLC Top Control Register
	volatile U32 MLCSCREENSIZE;				///< 0x04 : MLC Screen Size Register
	volatile U32 MLCBGCOLOR;				///< 0x08 : MLC Background Color Register

	struct {
	volatile U32 MLCLEFTRIGHT;				///< 0x0C, 0x40 : MLC RGB Layer Left Right Register 0/1
	volatile U32 MLCTOPBOTTOM;				///< 0x10, 0x44 : MLC RGB Layer Top Bottom Register 0/1
	volatile U32 MLCINVALIDLEFTRIGHT0;		///< 0x14, 0x48 : MLC RGB Invalid Area0 Left, Right Register 0/1
	volatile U32 MLCINVALIDTOPBOTTOM0;		///< 0x18, 0x4C : MLC RGB Invalid Area0 Top, Bottom Register 0/1
	volatile U32 MLCINVALIDLEFTRIGHT1;		///< 0x1C, 0x50 : MLC RGB Invalid Area1 Left, Right Register 0/1
	volatile U32 MLCINVALIDTOPBOTTOM1;		///< 0x20, 0x54 : MLC RGB Invalid Area1 Top, Bottom Register 0/1
	volatile U32 MLCCONTROL;				///< 0x24, 0x58 : MLC RGB Layer Control Register 0/1
	volatile S32 MLCHSTRIDE;				///< 0x28, 0x5C : MLC RGB Layer Horizontal Stride Register 0/1
	volatile S32 MLCVSTRIDE;				///< 0x2C, 0x60 : MLC RGB Layer Vertical Stride Register 0/1
	volatile U32 MLCTPCOLOR;				///< 0x30, 0x64 : MLC RGB Layer Transparency Color Register 0/1
	volatile U32 MLCINVCOLOR;				///< 0x34, 0x68 : MLC RGB Layer Inversion Color Register 0/1
	volatile U32 MLCADDRESS;				///< 0x38, 0x6C : MLC RGB Layer Base Address Register 0/1
	volatile U32 __Reserved0;				///< 0x3C, 0x70 : Reserved Region

	} MLCRGBLAYER[2];

	struct {
	volatile U32 MLCLEFTRIGHT;				///< 0x74 : MLC Video Layer Left Right Register
	volatile U32 MLCTOPBOTTOM;				///< 0x78 : MLC Video Layer Top Bottom Register
	volatile U32 MLCCONTROL;				///< 0x7C : MLC Video Layer Control Register
	volatile U32 MLCVSTRIDE;				///< 0x80 : MLC Video Layer Y Vertical Stride Register
	volatile U32 MLCTPCOLOR;				///< 0x84 : MLC Video Layer Transparency Color Register
	//volatile U32 __Reserved1[1];			///< 0x88 : Reserved Region
	volatile U32 MLCINVCOLOR;				///< 0x88 : MLCINVCOLOR3 ( )
	volatile U32 MLCADDRESS;				///< 0x8C : MLC Video Layer Y Base Address Register
	volatile U32 MLCADDRESSCB;				///< 0x90 : MLC Video Layer Cb Base Address Register
	volatile U32 MLCADDRESSCR;				///< 0x94 : MLC Video Layer Cr Base Address Register
	volatile S32 MLCVSTRIDECB;				///< 0x98 : MLC Video Layer Cb Vertical Stride Register
	volatile S32 MLCVSTRIDECR;				///< 0x9C : MLC Video Layer Cr Vertical Stride Register
	volatile U32 MLCHSCALE;					///< 0xA0 : MLC Video Layer Horizontal Scale Register
	volatile U32 MLCVSCALE;					///< 0xA4 : MLC Video Layer Vertical Scale Register
	volatile U32 MLCLUENH;					///< 0xA8 : MLC Video Layer Luminance Enhancement Control Register
	volatile U32 MLCCHENH[4];				///< 0xAC, 0xB0, 0xB4, 0xB8 : MLC Video Layer Chrominance Enhancement Control Register 0/1/2/3

	} MLCVIDEOLAYER;

	struct {
	volatile U32 MLCLEFTRIGHT;				///< 0xBC : MLC RGB Layer Left Right Register 2
	volatile U32 MLCTOPBOTTOM;				///< 0xC0 : MLC RGB Layer Top Bottom Register 2
	volatile U32 MLCINVALIDLEFTRIGHT0;		///< 0xC4 : MLC RGB Invalid Area0 Left, Right Register 2
	volatile U32 MLCINVALIDTOPBOTTOM0;		///< 0xC8 : MLC RGB Invalid Area0 Top, Bottom Register 2
	volatile U32 MLCINVALIDLEFTRIGHT1;		///< 0xCC : MLC RGB Invalid Area1 Left, Right Register 2
	volatile U32 MLCINVALIDTOPBOTTOM1;		///< 0xD0 : MLC RGB Invalid Area1 Top, Bottom Register 2
	volatile U32 MLCCONTROL;				///< 0xD4 : MLC RGB Layer Control Register 2
	volatile S32 MLCHSTRIDE;				///< 0xD8 : MLC RGB Layer Horizontal Stride Register 2
	volatile S32 MLCVSTRIDE;				///< 0xDC : MLC RGB Layer Vertical Stride Register 2
	volatile U32 MLCTPCOLOR;				///< 0xE0 : MLC RGB Layer Transparency Color Register 2
	volatile U32 MLCINVCOLOR;				///< 0xE4 : MLC RGB Layer Inversion Color Register 2
	volatile U32 MLCADDRESS;				///< 0xE8 : MLC RGB Layer Base Address Register 2

	} MLCRGBLAYER2;

	volatile U32 MLCPALETETABLE2;               ///< 0xEC
	volatile U32 MLCGAMMACONT;					///< 0xF0 : MLC Gama Control Register
	volatile U32 MLCRGAMMATABLEWRITE;			///< 0xF4 : MLC Red Gamma Table Write Register
	volatile U32 MLCGGAMMATABLEWRITE;			///< 0xF8 : MLC Green Gamma Table Write Register
	volatile U32 MLCBGAMMATABLEWRITE;			///< 0xFC : MLC Blue Gamma Table Write Register
	volatile U32 YUVLAYERGAMMATABLE_RED;        ///< 0x100:
	volatile U32 YUVLAYERGAMMATABLE_GREEN;      ///< 0x104:
	volatile U32 YUVLAYERGAMMATABLE_BLUE;       ///< 0x108:
	// @modified charles 2010/01/28 Dynamic Dimming 추가
	volatile U32 DIMCTRL;                       ///< 0x10C
	volatile U32 DIMLUT0;                       ///< 0x110
	volatile U32 DIMLUT1;                       ///< 0x114
	volatile U32 DIMBUSYFLAG;                   ///< 0x118
	volatile U32 DIMPRDARRR0;                   ///< 0x11C
	volatile U32 DIMPRDARRR1;                   ///< 0x120
	volatile U32 DIMRAM0RDDATA;                 ///< 0x124
	volatile U32 DIMRAM1RDDATA;                 ///< 0x128
	volatile U32 __Reserved2[(0x3C0-0x12C)/4];	///< 0x12C ~ 0x3BC : Reserved Region
	volatile U32 MLCCLKENB;						///< 0x3C0 : MLC Clock Enable Register

};

/// @brief	a priority of layers.
typedef enum
{
	NX_MLC_PRIORITY_VIDEOFIRST	= 0UL,	///< video layer > layer0 > layer1 > layer2
	NX_MLC_PRIORITY_VIDEOSECOND	= 1UL,	///< layer0 > video layer > layer1 > layer2
	NX_MLC_PRIORITY_VIDEOTHIRD	= 2UL,	///< layer0 > layer1 > video layer > layer2
	NX_MLC_PRIORITY_VIDEOFOURTH	= 3UL	///< layer0 > layer1 > layer2 > video layer

} NX_MLC_PRIORITY ;

//--------------------------------------------------------------------------
// To remove following waring on RVDS compiler
//		Warning : #66-D: enumeration value is out of "int" range
//--------------------------------------------------------------------------
#ifdef __arm
#pragma diag_remark 66		// disable #66 warining
#endif

/// @brief	RGB layer pixel format.
typedef enum
{
	NX_MLC_RGBFMT_R5G6B5	= 0x44320000UL,	///< 16bpp { R5, G6, B5 }.
	NX_MLC_RGBFMT_B5G6R5	= 0xC4320000UL,	///< 16bpp { B5, G6, R5 }.

	NX_MLC_RGBFMT_X1R5G5B5	= 0x43420000UL,	///< 16bpp { X1, R5, G5, B5 }.
	NX_MLC_RGBFMT_X1B5G5R5	= 0xC3420000UL,	///< 16bpp { X1, B5, G5, R5 }.
	NX_MLC_RGBFMT_X4R4G4B4	= 0x42110000UL,	///< 16bpp { X4, R4, G4, B4 }.
	NX_MLC_RGBFMT_X4B4G4R4	= 0xC2110000UL,	///< 16bpp { X4, B4, G4, R4 }.
	NX_MLC_RGBFMT_X8R3G3B2	= 0x41200000UL,	///< 16bpp { X8, R3, G3, B2 }.
	NX_MLC_RGBFMT_X8B3G3R2	= 0xC1200000UL,	///< 16bpp { X8, B3, G3, R2 }.

	NX_MLC_RGBFMT_A1R5G5B5	= 0x33420000UL,	///< 16bpp { A1, R5, G5, B5 }.
	NX_MLC_RGBFMT_A1B5G5R5	= 0xB3420000UL,	///< 16bpp { A1, B5, G5, R5 }.
	NX_MLC_RGBFMT_A4R4G4B4	= 0x22110000UL,	///< 16bpp { A4, R4, G4, B4 }.
	NX_MLC_RGBFMT_A4B4G4R4	= 0xA2110000UL,	///< 16bpp { A4, B4, G4, R4 }.
	NX_MLC_RGBFMT_A8R3G3B2	= 0x11200000UL,	///< 16bpp { A8, R3, G3, B2 }.
	NX_MLC_RGBFMT_A8B3G3R2	= 0x91200000UL,	///< 16bpp { A8, B3, G3, R2 }.

	NX_MLC_RGBFMT_R8G8B8	= 0x46530000UL,	///< 24bpp { R8, G8, B8 }.
	NX_MLC_RGBFMT_B8G8R8	= 0xC6530000UL,	///< 24bpp { B8, G8, R8 }.

	NX_MLC_RGBFMT_X8R8G8B8	= 0x46530000UL,	///< 32bpp { X8, R8, G8, B8 }.
	NX_MLC_RGBFMT_X8B8G8R8	= 0xC6530000UL,	///< 32bpp { X8, B8, G8, R8 }.
	NX_MLC_RGBFMT_A8R8G8B8	= 0x06530000UL,	///< 32bpp { A8, R8, G8, B8 }.
	NX_MLC_RGBFMT_A8B8G8R8	= 0x86530000UL	///< 32bpp { A8, B8, G8, R8 }.

}	NX_MLC_RGBFMT ;

/// @brief	Video layer pixel format.
typedef enum
{
	NX_MLC_YUVFMT_420		= 0UL<<16,			///< Block Separated YUV420.
	NX_MLC_YUVFMT_422		= 1UL<<16,			///< Block Separated YUV422.
	NX_MLC_YUVFMT_444		= 3UL<<16,			///< Block Separated YUV444.
	NX_MLC_YUVFMT_YUYV		= 2UL<<16,			///< Linear YUV422(YUYV).
	NX_MLC_YUVFMT_Y4UV		= 4UL<<16			///< Block Separated Y Linear UV
}	NX_MLC_YUVFMT;

#ifdef __arm	// for RVDS
#pragma diag_default 66		// return to default setting for #66 warning
#endif
//--------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_MLC_Initialize( void );
U32		NX_MLC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_MLC_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_MLC_GetSizeOfRegisterSet( void );
void	NX_MLC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_MLC_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_MLC_OpenModule( U32 ModuleIndex );
CBOOL	NX_MLC_CloseModule( U32 ModuleIndex );
CBOOL	NX_MLC_CheckBusy( U32 ModuleIndex );
CBOOL	NX_MLC_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Clock Control Interface
//@{
void		NX_MLC_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode );
NX_PCLKMODE	NX_MLC_GetClockPClkMode( U32 ModuleIndex );
void		NX_MLC_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode );
NX_BCLKMODE	NX_MLC_GetClockBClkMode( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
///	@name	MLC Main Settings
//--------------------------------------------------------------------------
//@{
void	NX_MLC_SetTopPowerMode( U32 ModuleIndex, CBOOL bPower );
CBOOL	NX_MLC_GetTopPowerMode( U32 ModuleIndex );
void	NX_MLC_SetTopSleepMode( U32 ModuleIndex, CBOOL bSleep );
CBOOL	NX_MLC_GetTopSleepMode( U32 ModuleIndex );
void	NX_MLC_SetTopDirtyFlag( U32 ModuleIndex );
CBOOL	NX_MLC_GetTopDirtyFlag( U32 ModuleIndex );
void	NX_MLC_SetMLCEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_MLC_GetMLCEnable( U32 ModuleIndex );
void	NX_MLC_SetFieldEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_MLC_GetFieldEnable( U32 ModuleIndex );
void	NX_MLC_SetLayerPriority( U32 ModuleIndex, NX_MLC_PRIORITY priority );
void	NX_MLC_SetScreenSize( U32 ModuleIndex, U32 width, U32 height );
void	NX_MLC_GetScreenSize( U32 ModuleIndex, U32 *pWidth, U32 *pHeight );
void	NX_MLC_SetBackground( U32 ModuleIndex, U32 color );
//@}

//--------------------------------------------------------------------------
///	@name	Per Layer Operations
//--------------------------------------------------------------------------
//@{
void	NX_MLC_SetDirtyFlag( U32 ModuleIndex, U32 layer );
CBOOL	NX_MLC_GetDirtyFlag( U32 ModuleIndex, U32 layer );
void	NX_MLC_SetLayerEnable( U32 ModuleIndex, U32 layer, CBOOL bEnb );
CBOOL	NX_MLC_GetLayerEnable( U32 ModuleIndex, U32 layer );
void	NX_MLC_SetLockSize( U32 ModuleIndex, U32 layer, U32 locksize );
void	NX_MLC_SetAlphaBlending( U32 ModuleIndex, U32 layer, CBOOL bEnb, U32 alpha );
void	NX_MLC_SetTransparency( U32 ModuleIndex, U32 layer, CBOOL bEnb, U32 color );
void	NX_MLC_SetColorInversion( U32 ModuleIndex, U32 layer, CBOOL bEnb, U32 color );
U32		NX_MLC_GetExtendedColor( U32 ModuleIndex, U32 color, NX_MLC_RGBFMT format );
void	NX_MLC_SetFormatRGB( U32 ModuleIndex, U32 layer, NX_MLC_RGBFMT format );
void	NX_MLC_SetFormatYUV( U32 ModuleIndex, NX_MLC_YUVFMT format );
void	NX_MLC_SetPosition( U32 ModuleIndex, U32 layer, S32 sx, S32 sy, S32 ex, S32 ey );
void	NX_MLC_SetDitherEnableWhenUsingGamma( U32 ModuleIndex, CBOOL bEnable );
CBOOL	NX_MLC_GetDitherEnableWhenUsingGamma( U32 ModuleIndex );
void	NX_MLC_SetGammaPriority( U32 ModuleIndex, CBOOL bVideoLayer );
CBOOL	NX_MLC_GetGammaPriority( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	RGB Layer Specific Operations
//--------------------------------------------------------------------------
//@{
void	NX_MLC_SetRGBLayerInvalidPosition( U32 ModuleIndex, U32 layer, U32 region, S32 sx, S32 sy, S32 ex, S32 ey, CBOOL bEnb );
void	NX_MLC_SetRGBLayerStride( U32 ModuleIndex, U32 layer, S32 hstride, S32 vstride );
void	NX_MLC_SetRGBLayerAddress( U32 ModuleIndex, U32 layer, U32 addr );

void	NX_MLC_SetRGBLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL bRed, CBOOL bGreen, CBOOL bBlue );
void	NX_MLC_GetRGBLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL *pbRed, CBOOL *pbGreen, CBOOL *pbBlue );

void	NX_MLC_SetRGBLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL bRed, CBOOL bGreen, CBOOL bBlue );
void	NX_MLC_GetRGBLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL *pbRed, CBOOL *pbGreen, CBOOL *pbBlue );

void	NX_MLC_SetRGBLayerRGammaTable( U32 ModuleIndex, U32 dwAddress, U32 dwData );
void	NX_MLC_SetRGBLayerGGammaTable( U32 ModuleIndex, U32 dwAddress, U32 dwData );
void	NX_MLC_SetRGBLayerBGammaTable( U32 ModuleIndex, U32 dwAddress, U32 dwData );

void	NX_MLC_SetRGBLayerGammaEnable( U32 ModuleIndex, CBOOL bEnable );
CBOOL	NX_MLC_GetRGBLayerGammaEnable( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Video Layer Specific Operations
//--------------------------------------------------------------------------
//@{
void	NX_MLC_SetVideoLayerStride( U32 ModuleIndex, S32 LuStride, S32 CbStride, S32 CrStride );
void	NX_MLC_SetVideoLayerAddress( U32 ModuleIndex, U32 LuAddr, U32 CbAddr, U32 CrAddr );
void	NX_MLC_SetVideoLayerAddressYUYV( U32 ModuleIndex, U32 Addr, S32 Stride );

void	NX_MLC_SetVideoLayerScaleFactor( U32 ModuleIndex, U32 hscale, U32 vscale, CBOOL bHLumaEnb, CBOOL bHChromaEnb, CBOOL bVLumaEnb, CBOOL bVChromaEnb );
void	NX_MLC_SetVideoLayerScaleFilter( U32 ModuleIndex, CBOOL bHLumaEnb, CBOOL bHChromaEnb, CBOOL bVLumaEnb, CBOOL bVChromaEnb );
void	NX_MLC_GetVideoLayerScaleFilter( U32 ModuleIndex, CBOOL *bHLumaEnb, CBOOL *bHChromaEnb, CBOOL *bVLumaEnb, CBOOL *bVChromaEnb );

void	NX_MLC_SetVideoLayerScale( U32 ModuleIndex, U32 sw, U32 sh, U32 dw, U32 dh, CBOOL bHLumaEnb, CBOOL bHChromaEnb, CBOOL bVLumaEnb, CBOOL bVChromaEnb );

void	NX_MLC_SetVideoLayerLumaEnhance( U32 ModuleIndex, U32 contrast, S32 brightness );
void	NX_MLC_SetVideoLayerChromaEnhance( U32 ModuleIndex, U32 quadrant, S32 CbA, S32 CbB, S32 CrA, S32 CrB );

void	NX_MLC_SetVideoLayerLineBufferPowerMode( U32 ModuleIndex, CBOOL bEnable );
CBOOL	NX_MLC_GetVideoLayerLineBufferPowerMode( U32 ModuleIndex );

void	NX_MLC_SetVideoLayerLineBufferSleepMode( U32 ModuleIndex, CBOOL bEnable );
CBOOL	NX_MLC_GetVideoLayerLineBufferSleepMode( U32 ModuleIndex );

//void	NX_MLC_SetVideoLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL bY, CBOOL bU, CBOOL bV );
//void	NX_MLC_GetVideoLayerGamaTablePowerMode( U32 ModuleIndex, CBOOL *pbY, CBOOL *pbU, CBOOL *pbV );

//void	NX_MLC_SetVideoLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL bY, CBOOL bU, CBOOL bV );
//void	NX_MLC_GetVideoLayerGamaTableSleepMode( U32 ModuleIndex, CBOOL *pbY, CBOOL *pbU, CBOOL *pbV );

void	NX_MLC_SetVideoLayerGammaEnable( U32 ModuleIndex, CBOOL bEnable );
CBOOL	NX_MLC_GetVideoLayerGammaEnable( U32 ModuleIndex );
//@}

//@}





//------------------------------------------------------------------------------
/// @brief	Prototype 이식.
//------------------------------------------------------------------------------

typedef enum
{
    RGBFMT_R5G6B5       = 0,
    RGBFMT_X1R5G5B5     = 1,
    RGBFMT_X4R4G4B4     = 2,
    RGBFMT_X8R3G3B2     = 3,
    RGBFMT_X8L8         = 4,
    RGBFMT_L16          = 5,
    RGBFMT_A1R5G5B5     = 6,
    RGBFMT_A4R4G4B4     = 7,
    RGBFMT_A8R3G3B2     = 8,
    RGBFMT_A8L8         = 9,
    RGBFMT_R8G8B8       = 10,
    RGBFMT_X8R8G8B8     = 11,
    RGBFMT_A8R8G8B8     = 12,
    RGBFMT_G8R8_G8B8    = 13,
    RGBFMT_R8G8_B8G8    = 14,
    RGBFMT_B5G6R5       = 15,
    RGBFMT_X1B5G5R5     = 16,
    RGBFMT_X4B4G4R4     = 17,
    RGBFMT_X8B3G3R2     = 18,
    RGBFMT_A1B5G5R5     = 19,
    RGBFMT_A4B4G4R4     = 20,
    RGBFMT_A8B3G3R2     = 21,
    RGBFMT_B8G8R8       = 22,
    RGBFMT_X8B8G8R8     = 23,
    RGBFMT_A8B8G8R8     = 24,
    RGBFMT_G8B8_G8R8    = 25,
    RGBFMT_B8G8_R8G8    = 26,
	RGBFMT_PATALETB     = 27
}RGBFMT;

typedef enum
{
	YUVFMT_420   = 0,	///< YUV Format YCbCr420
	YUVFMT_422   = 1,	///< YUV Format YCbCr422
	YUVFMT_422NS = 2,	///< YUV Format YCbCr422 no-seperated
	YUVFMT_444   = 3	///< YUV Format YCbCr444
}YUVFMT;

/// @brief  Hue Quadrant
typedef enum
{
    FRISTQUADRANT   = 0,
    SECONDQUADRANT  = 1,
    THIRDQUADRANT   = 2,
    FOURTHQUADRANT  = 3,
    ALLQUADRANT     = 4
}HUEQUADRANT;

typedef enum
{
    TOPMLC      = 0,
    RGB0        = 1,
	RGB1        = 2,
	RGB2        = 3,
    VIDEO       = 4
}LATYERNAME;

typedef enum
{
	POWEROFF	=	0,
	SLEEPMODE	=	2,
	RUN			=	3
}SRAMMODE;

/// @brief  RGB Layer Lock size select
typedef  enum
{
    LOCKSIZE_4  = 0,
    LOCKSIZE_8	= 1,
	LOCKSIZE_16	= 2
}LOCKSIZESEL;

typedef  enum
{
	PRIM	= 0,
	SECON	= 1,
	PRIMORSECON = 2,
	PRIMANDSECON = 3
}G3DADDRCHANGEALLOWED;

void
NX_MLC_SetMLCTopControlParameter
(
    U32 ModuleIndex,
    CBOOL FieldEnable,   ///< [in] display format (interlace and progressive)
    CBOOL MLCEnable,     ///< [in] MLC Operation Enable
    U8 Priority,         ///< [in] Layer preference decide
    G3DADDRCHANGEALLOWED G3DAddrChangeAllowed
);

void
NX_MLC_SetRGB0LayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Lyer Enable
    CBOOL GRP3DEnable,		///< [in] GRP3D Lyer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,          ///< [in] Use Alpha value in no Alpha value format case
    RGBFMT RBGFormat,       ///< [in] RGB Format in normal layer case
    LOCKSIZESEL LockSizeSelect ///< [in] Lock Size select
);

U32 NX_MLC_GetRGBFormat(RGBFMT RBGFormat);

void
NX_MLC_SetRGB1LayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Lyer Enable
    CBOOL GRP3DEnable,		///< [in] GRP3D Lyer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,          ///< [in] Use Alpha value in no Alpha value format case
    RGBFMT RBGFormat,       ///< [in] RGB Format in normal layer case
    LOCKSIZESEL LockSizeSelect ///< [in] Lock Size select
);

void
NX_MLC_SetRGB2LayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Lyer Enable
    CBOOL GRP3DEnable,		///< [in] GRP3D Lyer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,          ///< [in] Use Alpha value in no Alpha value format case
    RGBFMT RBGFormat,       ///< [in] RGB Format in normal layer case
    LOCKSIZESEL LockSizeSelect ///< [in] Lock Size select
);

void
NX_MLC_SetVideoLayerControlParameter
(
    U32 ModuleIndex,
    CBOOL LayerEnable,      ///< [in] Curent Layer Enable
    CBOOL TpEnable,         ///< [in] Transparency enable
    U32 TransparencyColor,  ///< [in] Transparency color
    CBOOL InvEnable,        ///< [in] Inverse enable
    U32 InverseColor,       ///< [in] Inverse color
    CBOOL BlendEnable,      ///< [in] Alpha blending enable
    U8 AlphaValue,			///< [in] Use Alpha value in no Alpha value format case
    YUVFMT	YUVFormat
);

void NX_MLC_SetSRAMMODE(U32 ModuleIndex, LATYERNAME LayerName, SRAMMODE SramMode);

void
NX_MLC_SetLayerRegFinish( U32 ModuleIndex, LATYERNAME LayerName );

void
NX_MLC_SetVideoLayerCoordinate
(
    U32 ModuleIndex,
    CBOOL VFilterEnable,     ///< [in] VScale enable
    CBOOL HFilterEnable,     ///< [in] HScale enable
    CBOOL VFilterEnable_C,     ///< [in] VScale enable
    CBOOL HFilterEnable_C,     ///< [in] HScale enable
    U16 VideoLayerWith,     ///< [in] Video Layer With
    U16 VideoLayerHeight,   ///< [in] Video Layer Height
    S16 Left,               ///< [in] Layer left coordinate
    S16 Right,              ///< [in] Layer right coordinate
    S16 Top,                ///< [in] Layer top coordinate
    S16 Bottom              ///< [in] Layer bottom coordinate
);

void NX_MLCSetGammaSRAMMode
(
	U32 ModuleIndex,
	SRAMMODE SramMode	// gamma sram table mode (power/sleep)
);

void NX_MLC_SetGammaControlParameter
(
	U32 ModuleIndex,
	CBOOL RGBGammaEnb, 		// RGB Layer Gamma table enable
	CBOOL YUVGammaEnb, 		// YUV Layer Gamma table enable
	CBOOL YUVAlphaArray, 	// YUV Layer Alpha Array ( 0:RGB Layer 1:YUV Layer)
	CBOOL DitherEnb			// Gamma value dither(10 -> 8bit)
);


//----------------------------
// 8bit Alpha 설정 함수.
//----------------------------
void NX_MLC_SetLayerAlpha256(U32 ModuleIndex, U32 Layer, U32 Alpha);













#ifdef	__cplusplus
}
#endif

#endif // __NX_MLC_H__
