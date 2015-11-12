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
	volatile U32	VIP_PADCLK_SEL;		// 0x02C : VIP PAD clock sel 0: clkgen 1:PAD clk
	volatile U32	VIP_INFIFOCLR;		// 0x030 : VIP input FIFO clear
	volatile U8		__Reserved00[0x200-0x34];	// 0x02C ~ 0x1FF
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
	volatile U32	__Reserved01;		// 0x238 :
	volatile U32	__Reserved02;		// 0x23C :
	volatile U32	__Reserved03;		// 0x240 :
	volatile U32	DECI_FORMAT;		// 0x244 : VIP Decimator NX_VIP_FORMAT Register
	volatile U32	DECI_LUADDR;		// 0x248 :
	volatile U32	DECI_LUSTRIDE;		// 0x24C :
	volatile U32	DECI_CRADDR;		// 0x250 :
	volatile U32	DECI_CRSTRIDE;		// 0x254 :
	volatile U32	DECI_CBADDR ;		// 0x258 :
	volatile U32	DECI_CBSTRIDE;		// 0x25C :
	volatile U32	__Reserved04;		// 0x260 :
	volatile U32	__Reserved05;		// 0x264 :
	volatile U32	__Reserved06;		// 0x268 :
	volatile U32	__Reserved07;		// 0x26C :
	volatile U32	__Reserved08;		// 0x270 :
	volatile U32	__Reserved09;		// 0x274 :
	volatile U32	__Reserved10;		// 0x278 :
	volatile U32	__Reserved11;		// 0x27C :
	volatile U32	__Reserved12;		// 0x280 :
	volatile U32	__Reserved13;		// 0x284 :
	volatile U32	CLIP_FORMAT;		// 0x288 : VIP Clipper Format Register
	volatile U32	CLIP_LUADDR;		// 0x28C :
	volatile U32	CLIP_LUSTRIDE;		// 0x290 :
	volatile U32	CLIP_CRADDR;		// 0x294 :
	volatile U32	CLIP_CRSTRIDE;		// 0x298 :
	volatile U32	CLIP_CBADDR;		// 0x29C :
	volatile U32	CLIP_CBSTRIDE;		// 0x2A0 :
	volatile U32	__Reserved14;		// 0x2A4 :
	volatile U32	__Reserved15;		// 0x2A8 :
	volatile U32	__Reserved16;		// 0x2AC :
	volatile U32	__Reserved17;		// 0x2B0 :
	volatile U32	__Reserved18;		// 0x2B4 :
	volatile U32	__Reserved19;		// 0x2B8 :
	volatile U32	__Reserved20;		// 0x2BC :
	volatile U32	VIP_SCANMODE;		// 0x2C0 : VIP Clipper & Decimator Scan Mode Register
	volatile U32	__Reserved21;		// 0x2C4 :
	volatile U32	__Reserved22;		// 0x2C8 :
	volatile U32	__Reserved23;		// 0x2CC :
	volatile U32	__Reserved24;		// 0x2D0 :
	volatile U32	__Reserved25;		// 0x2D4 :
	volatile U32	VIP_VIP1;			// 0x2D8 :
} NX_VIP_RegisterSet;

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
		NX_VIP_FORMAT_420				= 0UL,	/// Separated YUV 420
		NX_VIP_FORMAT_422				= 1UL,	/// Separated YUV 422
		NX_VIP_FORMAT_444				= 2UL,	/// Separated YUV 444
		NX_VIP_FORMAT_L422				= 3UL,	/// Non-Separated YUYV 422
		NX_VIP_FORMAT_YUYV				= 3UL,	/// Non-Separated YUYV 422

		NX_VIP_FORMAT_420_CBCR			= 4UL,	/// Separated YUV 420 CBCR packed
		NX_VIP_FORMAT_422_CBCR			= 5UL,	/// Separated YUV 422 CBCR packed
		NX_VIP_FORMAT_444_CBCR			= 6UL,	/// Separated YUV 444 CBCR packed
		NX_VIP_FORMAT_RESERVED00		= 7UL,	/// Reserved

		NX_VIP_FORMAT_420_CRCB			= 8UL,	/// Separated YUV 420 CRCB packed
		NX_VIP_FORMAT_422_CRCB			= 9UL,	/// Separated YUV 422 CRCB packed
		NX_VIP_FORMAT_444_CRCB			= 10UL,	/// Separated YUV 444 CRCB packed
		NX_VIP_FORMAT_RESERVED01		= 11UL	/// Reserved
	} NX_VIP_FORMAT;

	/// @brief	input port select
	typedef enum
	{
		NX_VIP_INPUTPORT_A				= 0UL,	///	Input port A(VIDCLK0, VIDHSYNC0, VIDVSYNC0, FIELD/DVALID, VID0)
		NX_VIP_INPUTPORT_B				= 1UL	///	Input port B(VIDCLK1, VIDHSYNC1, VIDVSYNC1, SIPFIELD/DVALID, VID1)
	}	NX_VIP_INPUTPORT;

	///	@brief	data channel width
	typedef enum
	{
		NX_VIP_VD_8BIT	= 0,	///< 8bit VD
		NX_VIP_VD_16BIT	= 1,	///< 16bit VD
	} NX_VIP_VD_BITS;

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
///	@name	PAD Interface
//------------------------------------------------------------------------------
//@{
U32 NX_VIP_GetNumberOfPADMode ( U32 ModuleIndex );
void NX_VIP_EnablePAD ( U32 ModuleIndex, U32 ModeIndex );
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

void	NX_VIP_SetSync( U32 ModuleIndex, CBOOL bExtSync, NX_VIP_VD_BITS SourceBits,
						U32 AVW, U32 AVH, U32 HSW, U32 HFP, U32 HBP, U32 VSW, U32 VFP, U32 VBP );
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

void	NX_VIP_SetClipperFormat( U32 ModuleIndex, NX_VIP_FORMAT Format );
void	NX_VIP_GetClipperFormat( U32 ModuleIndex, NX_VIP_FORMAT *pFormat );
void	NX_VIP_SetClipperAddr( U32 ModuleIndex, NX_VIP_FORMAT Format, U32 Width, U32 Height, U32 LuAddr, U32 CbAddr, U32 CrAddr, U32 StrideY, U32 StrideCbCr );
//@}

//------------------------------------------------------------------------------
/// @name Decimator Function
//------------------------------------------------------------------------------
//@{
void	NX_VIP_SetDecimation( U32 ModuleIndex, U32 SrcWidth, U32 SrcHeight, U32 DstWidth, U32 DstHeight );
void	NX_VIP_GetDecimation( U32 ModuleIndex, U32 *pDstWidth, U32 *pDstHeight,
							U32 *pDeltaWidth, U32 *pDeltaHeight, S32 *pClearWidth, S32 *pClearHeight );

void	NX_VIP_SetDecimatorFormat( U32 ModuleIndex, NX_VIP_FORMAT Format );
void	NX_VIP_GetDecimatorFormat( U32 ModuleIndex, NX_VIP_FORMAT *pFormat );
void	NX_VIP_SetDecimatorAddr( U32 ModuleIndex, NX_VIP_FORMAT Format, U32 Width, U32 Height, U32 LuAddr, U32 CbAddr, U32 CrAddr, U32 YStride, U32 CBCRStride );
void    NX_VIP_GetDeciSource(U32 ModuleIndex, U32 *pSrcWidth, U32 *pSrcHeight);

//@}

CBOOL  NX_VIP_SmokeTest ( U32 ModuleIndex );
//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_VIP_H__
