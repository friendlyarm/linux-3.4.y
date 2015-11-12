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
//	File		: nx_vip.c
//	Description	:
//	Author		: Junseo Kim (junseo@nexell.co.kr)
//	History		: 2012-11-12 First Realease
//------------------------------------------------------------------------------
#ifndef __NX_VIP_H__
#define __NX_VIP_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	VIP
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
#if 1
typedef struct
{
	volatile U32	VIP_CONFIG;			// 0x000 : VIP Configuration Register
	volatile U32	VIP_HVINT;			// 0x004 : VIP Interrupt Control Register
	volatile U32	VIP_SYNCCTRL;		// 0x008 : VIP Sync Control Register
	volatile U32	VIP_SYNCMON;		// 0x00C : VIP Sync Monitor Register
	volatile U32	VIP_VBEGIN;			// 0x010 : VIP Vertical Sync Start Register
	volatile U32	VIP_VEND;			// 0x014 : VIP Vertical Sync End Register
	volatile U32	VIP_HBEGIN;			// 0x018 : VIP Horizontal Sync Start Register
	volatile U32	VIP_HEND;			// 0x01C : VIP Horizontal Sync End Register
	volatile U32	VIP_FIFOCTRL;		// 0x020 : VIP FIFO Control Register
	volatile U32	VIP_HCOUNT;			// 0x024 : VIP Horizontal Counter Register
	volatile U32	VIP_VCOUNT;			// 0x028 : VIP Vertical Counter Register
	volatile U8		__Reserved00[0x200-0x2C];	// 0x02C ~ 0x1FF
	volatile U32	VIP_CDENB;			// 0x200 : VIP Clipper & Decimator Enable Register
	volatile U32	VIP_ODINT;			// 0x204 : VIP Operation Done Interrupt Register
	volatile U32	VIP_IMGWIDTH;		// 0x208 : VIP Image Width Register
	volatile U32	VIP_IMGHEIGHT;		// 0x20C : VIP Image Height Register
	volatile U32	CLIP_LEFT;			// 0x210 : VIP Clipper Left Register
	volatile U32	CLIP_RIGHT;			// 0x214 : VIP Clipper Right Register
	volatile U32	CLIP_TOP;			// 0x218 : VIP Clipper Top Register
	volatile U32	CLIP_BOTTOM;		// 0x21C : VIP Clipper Bottom Register
	volatile U32	DECI_TARGETW;		// 0x220 : VIP Decimator Target Width Register
	volatile U32	DECI_TARGETH;		// 0x224 : VIP Decimator Target Height Register
	volatile U32	DECI_DELTAW;		// 0x228 : VIP Decimator Delta Width Register
	volatile U32	DECI_DELTAH;		// 0x22C : VIP Decimator Delta Height Register
	volatile S32	DECI_CLEARW;		// 0x230 : VIP Decimator Clear Width Register
	volatile S32	DECI_CLEARH;		// 0x234 : VIP Decimator Clear Height Register
	volatile U32	DECI_LUSEG;			// 0x238 : VIP Decimator Lu Segment Register
	volatile U32	DECI_CRSEG;			// 0x23C : VIP Decimator Cr Segment Register
	volatile U32	DECI_CBSEG;			// 0x240 : VIP Decimator Cb Segment Register
	volatile U32	DECI_FORMAT;		// 0x244 : VIP Decimator NX_VIP_FORMAT Register
	volatile U32	DECI_ROTFLIP;		// 0x248 : VIP Decimator Rotation & Flip Register
	volatile U32	DECI_LULEFT;		// 0x24C : VIP Decimator Lu Left Register
	volatile U32	DECI_CRLEFT;		// 0x250 : VIP Decimator Cr Left Register
	volatile U32	DECI_CBLEFT;		// 0x254 : VIP Decimator Cb Left Register
	volatile U32	DECI_LURIGHT;		// 0x258 : VIP Decimator Lu Right Register
	volatile U32	DECI_CRRIGHT;		// 0x25C : VIP Decimator Cr Right Register
	volatile U32	DECI_CBRIGHT;		// 0x260 : VIP Decimator Cb Right Register
	volatile U32	DECI_LUTOP;			// 0x264 : VIP Decimator Lu Top Register
	volatile U32	DECI_CRTOP;			// 0x268 : VIP Decimator Cr Top Register
	volatile U32	DECI_CBTOP;			// 0x26C : VIP Decimator Cb Top Register
	volatile U32	DECI_LUBOTTOM;		// 0x270 : VIP Decimator Lu Bottom Register
	volatile U32	DECI_CRBOTTOM;		// 0x274 : VIP Decimator Cr Bottom Register
	volatile U32	DECI_CBBOTTOM;		// 0x278 : VIP Decimator Cb Bottom Register
	volatile U32	CLIP_LUSEG;			// 0x27C : VIP Clipper Lu Segment Register
	volatile U32	CLIP_CRSEG;			// 0x280 : VIP Clipper Cr Segment Register
	volatile U32	CLIP_CBSEG;			// 0x284 : VIP Clipper Cb Segment Register
	volatile U32	CLIP_FORMAT;		// 0x288 : VIP Clipper Format Register
	volatile U32	CLIP_ROTFLIP;		// 0x28C : VIP Clipper Rotation & Flip Register
	volatile U32	CLIP_LULEFT;		// 0x290 : VIP Clipper Lu Left Register
	volatile U32	CLIP_CRLEFT;		// 0x294 : VIP Clipper Cr Left Register
	volatile U32	CLIP_CBLEFT;		// 0x298 : VIP Clipper Cb Left Register
	volatile U32	CLIP_LURIGHT;		// 0x29C : VIP Clipper Lu Right Register
	volatile U32	CLIP_CRRIGHT;		// 0x2A0 : VIP Clipper Cr Right Register
	volatile U32	CLIP_CBRIGHT;		// 0x2A4 : VIP Clipper Cb Right Register
	volatile U32	CLIP_LUTOP;			// 0x2A8 : VIP Clipper Lu Top Register
	volatile U32	CLIP_CRTOP;			// 0x2AC : VIP Clipper Cr Top Register
	volatile U32	CLIP_CBTOP;			// 0x2B0 : VIP Clipper Cb Top Register
	volatile U32	CLIP_LUBOTTOM;		// 0x2B4 : VIP Clipper Lu Bottom Register
	volatile U32	CLIP_CRBOTTOM;		// 0x2B8 : VIP Clipper Cr Bottom Register
	volatile U32	CLIP_CBBOTTOM;		// 0x2BC : VIP Clipper Cb Bottom Register
	volatile U32	VIP_SCANMODE;		// 0x2C0 : VIP Clipper & Decimator Scan Mode Register
	volatile U32	CLIP_YUYVENB;		// 0x2C4 : VIP Clipper Linear YUYV Enable Register
	volatile U32	CLIP_BASEADDRH;		// 0x2C8 : VIP Clipper Linear Base Address High Register
	volatile U32	CLIP_BASEADDRL;		// 0x2CC : VIP Clipper Linear Base Address Low Register
	volatile U32	CLIP_STRIDEH;		// 0x2D0 : VIP Clipper Linear Stride High Register
	volatile U32	CLIP_STRIDEL;		// 0x2D4 : VIP Clipper Linear Stride Low Register
	volatile U32	VIP_VIP1;			// 0x2D8 : VIP SIP Enable Register
} NX_VIP_RegisterSet;
#else
	typedef struct
	{
		volatile U16	VIP_CONFIG;			// 0x000 : VIP Configuration Register
		volatile U16	VIP_HVINT;			// 0x002 : VIP Interrupt Control Register
		volatile U16	VIP_SYNCCTRL;		// 0x004 : VIP Sync Control Register
		volatile U16	VIP_SYNCMON;		// 0x006 : VIP Sync Monitor Register
		volatile U16	VIP_VBEGIN;			// 0x008 : VIP Vertical Sync Start Register
		volatile U16	VIP_VEND;			// 0x00A : VIP Vertical Sync End Register
		volatile U16	VIP_HBEGIN;			// 0x00C : VIP Horizontal Sync Start Register
		volatile U16	VIP_HEND;			// 0x00E : VIP Horizontal Sync End Register
		volatile U16	VIP_FIFOCTRL;		// 0x010 : VIP FIFO Control Register
		volatile U16	VIP_HCOUNT;			// 0x012 : VIP Horizontal Counter Register
		volatile U16	VIP_VCOUNT;			// 0x014 : VIP Vertical Counter Register
		volatile U8		__Reserved00[0x200-0x16];	// 0x016 ~ 0x1FF
		volatile U16	VIP_CDENB;			// 0x200 : VIP Clipper & Decimator Enable Register
		volatile U16	VIP_ODINT;			// 0x202 : VIP Operation Done Interrupt Register
		volatile U16	VIP_IMGWIDTH;		// 0x204 : VIP Image Width Register
		volatile U16	VIP_IMGHEIGHT;		// 0x206 : VIP Image Height Register
		volatile U16	CLIP_LEFT;			// 0x208 : VIP Clipper Left Register
		volatile U16	CLIP_RIGHT;			// 0x20A : VIP Clipper Right Register
		volatile U16	CLIP_TOP;			// 0x20C : VIP Clipper Top Register
		volatile U16	CLIP_BOTTOM;		// 0x20E : VIP Clipper Bottom Register
		volatile U16	DECI_TARGETW;		// 0x210 : VIP Decimator Target Width Register
		volatile U16	DECI_TARGETH;		// 0x212 : VIP Decimator Target Height Register
		volatile U16	DECI_DELTAW;		// 0x214 : VIP Decimator Delta Width Register
		volatile U16	DECI_DELTAH;		// 0x216 : VIP Decimator Delta Height Register
		volatile S16	DECI_CLEARW;		// 0x218 : VIP Decimator Clear Width Register
		volatile S16	DECI_CLEARH;		// 0x21A : VIP Decimator Clear Height Register
		volatile U16	DECI_LUSEG;			// 0x21C : VIP Decimator Lu Segment Register
		volatile U16	DECI_CRSEG;			// 0x21E : VIP Decimator Cr Segment Register
		volatile U16	DECI_CBSEG;			// 0x220 : VIP Decimator Cb Segment Register
		volatile U16	DECI_FORMAT;		// 0x222 : VIP Decimator NX_VIP_FORMAT Register
		volatile U16	DECI_ROTFLIP;		// 0x224 : VIP Decimator Rotation & Flip Register
		volatile U16	DECI_LULEFT;		// 0x226 : VIP Decimator Lu Left Register
		volatile U16	DECI_CRLEFT;		// 0x228 : VIP Decimator Cr Left Register
		volatile U16	DECI_CBLEFT;		// 0x22A : VIP Decimator Cb Left Register
		volatile U16	DECI_LURIGHT;		// 0x22C : VIP Decimator Lu Right Register
		volatile U16	DECI_CRRIGHT;		// 0x22E : VIP Decimator Cr Right Register
		volatile U16	DECI_CBRIGHT;		// 0x230 : VIP Decimator Cb Right Register
		volatile U16	DECI_LUTOP;			// 0x232 : VIP Decimator Lu Top Register
		volatile U16	DECI_CRTOP;			// 0x234 : VIP Decimator Cr Top Register
		volatile U16	DECI_CBTOP;			// 0x236 : VIP Decimator Cb Top Register
		volatile U16	DECI_LUBOTTOM;		// 0x238 : VIP Decimator Lu Bottom Register
		volatile U16	DECI_CRBOTTOM;		// 0x23A : VIP Decimator Cr Bottom Register
		volatile U16	DECI_CBBOTTOM;		// 0x23C : VIP Decimator Cb Bottom Register
		volatile U16	CLIP_LUSEG;			// 0x23E : VIP Clipper Lu Segment Register
		volatile U16	CLIP_CRSEG;			// 0x240 : VIP Clipper Cr Segment Register
		volatile U16	CLIP_CBSEG;			// 0x242 : VIP Clipper Cb Segment Register
		volatile U16	CLIP_FORMAT;		// 0x244 : VIP Clipper Format Register
		volatile U16	CLIP_ROTFLIP;		// 0x246 : VIP Clipper Rotation & Flip Register
		volatile U16	CLIP_LULEFT;		// 0x248 : VIP Clipper Lu Left Register
		volatile U16	CLIP_CRLEFT;		// 0x24A : VIP Clipper Cr Left Register
		volatile U16	CLIP_CBLEFT;		// 0x24C : VIP Clipper Cb Left Register
		volatile U16	CLIP_LURIGHT;		// 0x24E : VIP Clipper Lu Right Register
		volatile U16	CLIP_CRRIGHT;		// 0x250 : VIP Clipper Cr Right Register
		volatile U16	CLIP_CBRIGHT;		// 0x252 : VIP Clipper Cb Right Register
		volatile U16	CLIP_LUTOP;			// 0x254 : VIP Clipper Lu Top Register
		volatile U16	CLIP_CRTOP;			// 0x256 : VIP Clipper Cr Top Register
		volatile U16	CLIP_CBTOP;			// 0x258 : VIP Clipper Cb Top Register
		volatile U16	CLIP_LUBOTTOM;		// 0x25A : VIP Clipper Lu Bottom Register
		volatile U16	CLIP_CRBOTTOM;		// 0x25C : VIP Clipper Cr Bottom Register
		volatile U16	CLIP_CBBOTTOM;		// 0x25E : VIP Clipper Cb Bottom Register
		volatile U16	VIP_SCANMODE;		// 0x260 : VIP Clipper & Decimator Scan Mode Register
		volatile U16	CLIP_YUYVENB;		// 0x262 : VIP Clipper Linear YUYV Enable Register
		volatile U16	CLIP_BASEADDRH;		// 0x264 : VIP Clipper Linear Base Address High Register
		volatile U16	CLIP_BASEADDRL;		// 0x266 : VIP Clipper Linear Base Address Low Register
		volatile U16	CLIP_STRIDEH;		// 0x268 : VIP Clipper Linear Stride High Register
		volatile U16	CLIP_STRIDEL;		// 0x26A : VIP Clipper Linear Stride Low Register
		volatile U16	VIP_VIP1;			// 0x26C : VIP SIP Enable Register
	} NX_VIP_RegisterSet;
#endif

	/// @brief VIP Interrupt for interrupt interface
	enum
	{
		NX_VIP_INT_VSYNC				= 0UL,	///< VSYNC(End of VSYNC) interrupt.
		NX_VIP_INT_HSYNC				= 1UL,	///< HSYNC(End of HSYNC) interrupt.
		NX_VIP_INT_DONE					= 2UL	///< Clipper & Decimator Done interrupt.
	};

	///	@brief	the source clock.
	typedef enum
	{
		NX_VIP_CLKSRC_FPLL				= 0UL,	///< PLL0
		NX_VIP_CLKSRC_UPLL				= 1UL,	///< PLL1
		NX_VIP_CLKSRC_ICLKIN			= 3UL,	///< ICLKIN pad
		NX_VIP_CLKSRC_INVICLKIN			= 4UL	///< inversion of ICLKIN pad
	}	NX_VIP_CLKSRC;

	///	@brief	the input data order.
	typedef enum
	{
		NX_VIP_DATAORDER_CBY0CRY1		= 0UL,	///< Cb, Y0, Cr, Y1
		NX_VIP_DATAORDER_CRY1CBY0		= 1UL,	///< Cr, Y1, Cb, Y0
		NX_VIP_DATAORDER_Y0CBY1CR		= 2UL,	///< Y0, Cb, Y1, Cr
		NX_VIP_DATAORDER_Y1CRY0CB		= 3UL	///< Y1, Cr, Y0, Cb
	}	NX_VIP_DATAORDER;

	///	@brief	the field signal selection.
	typedef enum
	{
		NX_VIP_FIELDSEL_BYPASS			= 0UL,	///< Bypass (Low is odd field)
		NX_VIP_FIELDSEL_INVERT			= 1UL,	///< Invert (Low is even field)
		NX_VIP_FIELDSEL_EVEN			= 2UL,	///< Fix 0 (odd field)
		NX_VIP_FIELDSEL_ODD				= 3UL	///< Fix 1 (even field)
	}	NX_VIP_FIELDSEL;

	///	@brief	the FIFO reset mode selection
	typedef enum
	{
		NX_VIP_FIFORESET_FRAMEEND		= 0UL,	///< FrameEnd (the end of the vertical sync).
		NX_VIP_FIFORESET_FRAMESTART		= 1UL,	///< FrameStart (the begin of vertical active video)
		NX_VIP_FIFORESET_CPU			= 2UL,	///< Clear by User
		NX_VIP_FIFORESET_ALL			= 3UL	///< All condition (FrameEnd or FrameStart or CPU)
	}	NX_VIP_FIFORESET;

	///	@brief	the status of FIFO
	typedef enum
	{
		NX_VIP_FSTATUS_FULL				= 1<<0UL,	///< FIFO is full.
		NX_VIP_FSTATUS_EMPTY			= 1<<1UL,	///< FIFO is empty.
		NX_VIP_FSTATUS_READING			= 1<<2UL,	///< FIFO is reading.
		NX_VIP_FSTATUS_WRITING			= 1<<3UL		///< FIFO is writing.
	}	NX_VIP_FSTATUS;

	///	@brief	the data format
	typedef enum
	{
		NX_VIP_FORMAT_420				= 0UL,	/// Block Separated YUV 420
		NX_VIP_FORMAT_422				= 1UL,	/// Block Separated YUV 422
		NX_VIP_FORMAT_444				= 2UL,	/// Block Separated YUV 444
		NX_VIP_FORMAT_YUYV				= 3UL	/// Linear YUV 422 (YUYV)
	}	NX_VIP_FORMAT;

	/// @brief	input port select
	typedef enum
	{
		NX_VIP_INPUTPORT_A				= 0UL,	///	Input port A(VIDCLK0, VIDHSYNC0, VIDVSYNC0, FIELD/DVALID, VID0)
		NX_VIP_INPUTPORT_B				= 1UL	///	Input port B(VIDCLK1, VIDHSYNC1, VIDVSYNC1, SIPFIELD/DVALID, VID1)
	}	NX_VIP_INPUTPORT;

//------------------------------------------------------------------------------
///	@name	VIP Interface
//------------------------------------------------------------------------------
//@{
U32   NX_VIP_GetTEMP( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_VIP_Initialize( void );
U32   NX_VIP_GetNumberOfModule( void );

U32   NX_VIP_GetSizeOfRegisterSet( void );
void  NX_VIP_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void* NX_VIP_GetBaseAddress( U32 ModuleIndex );
U32   NX_VIP_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_VIP_OpenModule( U32 ModuleIndex );
CBOOL NX_VIP_CloseModule( U32 ModuleIndex );
CBOOL NX_VIP_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_VIP_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_VIP_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
U32   NX_VIP_GetInterruptNumber ( U32 ModuleIndex );
void  NX_VIP_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL NX_VIP_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL NX_VIP_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_VIP_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_VIP_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL NX_VIP_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL NX_VIP_GetInterruptPendingAll( U32 ModuleIndex );
void  NX_VIP_ClearInterruptPendingAll( U32 ModuleIndex );
S32   NX_VIP_GetInterruptPendingNumber( U32 ModuleIndex );
//@}


//------------------------------------------------------------------------------
/// @name VIP Configure Function
//------------------------------------------------------------------------------
//@{
void	NX_VIP_SetVIPEnable( U32 ModuleIndex, CBOOL	bVIPEnb, CBOOL	bSepEnb, CBOOL	bClipEnb, CBOOL	bDeciEnb );
void	NX_VIP_GetVIPEnable( U32 ModuleIndex, CBOOL *pbVIPEnb, CBOOL *pbSepEnb, CBOOL *pbClipEnb, CBOOL *pbDeciEnb );

void				NX_VIP_SetInputPort( U32 ModuleIndex, NX_VIP_INPUTPORT InputPort );
NX_VIP_INPUTPORT	NX_VIP_GetInputPort( U32 ModuleIndex );

void	NX_VIP_SetDataMode( U32 ModuleIndex, NX_VIP_DATAORDER DataOrder, U32 DataWidth );
void	NX_VIP_GetDataMode( U32 ModuleIndex, NX_VIP_DATAORDER *DataOrder, U32 *pDataWidth	);

void	NX_VIP_SetHVSync( U32 ModuleIndex, CBOOL bExtSync, U32 AVW, U32 AVH,
								U32 HSW, U32 HFP, U32 HBP,
								U32 VSW, U32 VFP, U32 VBP );
void	NX_VIP_GetHVSync( U32 ModuleIndex, CBOOL *pbExtSync, U32 *pAVW, U32 *pAVH,
								U32 *pHBEGIN, U32 *pHEND, U32 *pVBEGIN, U32 *pVEND );

// It's only for NX4330
void	NX_VIP_SetHVSyncForMIPI( U32 ModuleIndex, U32 AVW, U32 AVH,
								U32 HSW, U32 HFP, U32 HBP,
								U32 VSW, U32 VFP, U32 VBP );

void	NX_VIP_SetDValidMode( U32 ModuleIndex, CBOOL bExtDValid, CBOOL bDValidPol, CBOOL bSyncPol );
void	NX_VIP_GetDValidMode( U32 ModuleIndex, CBOOL *pbExtDValid, CBOOL *pbDValidPol );

void	NX_VIP_SetFieldMode( U32 ModuleIndex, CBOOL bExtField, NX_VIP_FIELDSEL FieldSel, CBOOL bInterlace, CBOOL bInvField );
void	NX_VIP_GetFieldMode( U32 ModuleIndex, CBOOL *pbExtField, NX_VIP_FIELDSEL *pFieldSel, CBOOL *pbInterlace, CBOOL *pbInvField );
CBOOL	NX_VIP_GetFieldStatus( U32 ModuleIndex );

CBOOL	NX_VIP_GetHSyncStatus( U32 ModuleIndex );
CBOOL	NX_VIP_GetVSyncStatus( U32 ModuleIndex );

void				NX_VIP_SetFIFOResetMode( U32 ModuleIndex, NX_VIP_FIFORESET FIFOReset );
NX_VIP_FIFORESET	NX_VIP_GetFIFOResetMode( U32 ModuleIndex );

U32		NX_VIP_GetFIFOStatus( U32 ModuleIndex );
void	NX_VIP_ResetFIFO( U32 ModuleIndex );

U32		NX_VIP_GetHorCount( U32 ModuleIndex );
U32		NX_VIP_GetVerCount( U32 ModuleIndex );

//------------------------------------------------------------------------------
/// @name Clipper Function
//------------------------------------------------------------------------------
//@{
void	NX_VIP_SetClipRegion( U32 ModuleIndex, U32 Left, U32 Top, U32 Right, U32 Bottom );
void	NX_VIP_GetClipRegion( U32 ModuleIndex, U32 *pLeft, U32 *pTop, U32 *pRight, U32 *pBottom );

void	NX_VIP_SetClipperFormat( U32 ModuleIndex, NX_VIP_FORMAT Format, CBOOL bRotation, CBOOL bHFlip, CBOOL bVFlip );
void	NX_VIP_GetClipperFormat( U32 ModuleIndex, NX_VIP_FORMAT *pFormat, CBOOL *pbRotation, CBOOL *pbHFlip, CBOOL *pbVFlip );
void	NX_VIP_SetClipperAddr( U32 ModuleIndex, NX_VIP_FORMAT Format, U32 Width, U32 Height, U32 LuAddr, U32 CbAddr, U32 CrAddr, U32 StrideY, U32 StrideCbCr );
void	NX_VIP_SetClipperAddr2D( U32 ModuleIndex, U32 LuSAddr, U32 LuEAddr, U32 CbSAddr, U32 CbEAddr, U32 CrSAddr, U32 CrEAddr );
void	NX_VIP_GetClipperAddr2D( U32 ModuleIndex, U32 *pLuSAddr, U32 *pLuEAddr, U32 *pCbSAddr, U32 *pCbEAddr,
								U32 *pCrSAddr, U32 *pCrEAddr );
void	NX_VIP_SetClipperAddrYUYV( U32 ModuleIndex, U32 BaseAddr, U32 Stride );
void	NX_VIP_GetClipperAddrYUYV( U32 ModuleIndex, U32 *pBaseAddr, U32 *pStride );
//@}

//------------------------------------------------------------------------------
/// @name Decimator Function
//------------------------------------------------------------------------------
//@{
void	NX_VIP_SetDecimation( U32 ModuleIndex, U32 SrcWidth, U32 SrcHeight, U32 DstWidth, U32 DstHeight );
void	NX_VIP_GetDecimation( U32 ModuleIndex, U32 *pDstWidth, U32 *pDstHeight,
							U32 *pDeltaWidth, U32 *pDeltaHeight, S32 *pClearWidth, S32 *pClearHeight );
void    NX_VIP_GetDeciSource(U32 ModuleIndex, U32 *pSrcWidth, U32 *pSrcHeight);

void	NX_VIP_SetDecimatorFormat( U32 ModuleIndex, NX_VIP_FORMAT Format, CBOOL bRotation, CBOOL bHFlip, CBOOL bVFlip );
void	NX_VIP_GetDecimatorFormat( U32 ModuleIndex, NX_VIP_FORMAT *pFormat, CBOOL *pbRotation, CBOOL *pbHFlip, CBOOL *pbVFlip );
/* psw0523 fix for stride setting */
// void	NX_VIP_SetDecimatorAddr( U32 ModuleIndex, NX_VIP_FORMAT Format, U32 Width, U32 Height, U32 LuAddr, U32 CbAddr, U32 CrAddr, U32 Stride );
void	NX_VIP_SetDecimatorAddr( U32 ModuleIndex, NX_VIP_FORMAT Format, U32 Width, U32 Height, U32 LuAddr, U32 CbAddr, U32 CrAddr, U32 StrideY, U32 StrideCbCr );
void	NX_VIP_SetDecimatorAddr2D( U32 ModuleIndex, U32 LuSAddr, U32 LuEAddr, U32 CbSAddr, U32 CbEAddr, U32 CrSAddr, U32 CrEAddr );
void	NX_VIP_GetDecimatorAddr2D( U32 ModuleIndex, U32 *pLuSAddr, U32 *pLuEAddr, U32 *pCbSAddr, U32 *pCbEAddr,
							U32 *pCrSAddr, U32 *pCrEAddr );
//@}

//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_VIP_H__
