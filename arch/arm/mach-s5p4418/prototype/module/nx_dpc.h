//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: DPC
//	File		: nx_dpc.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------



#ifndef __NX_DPC_H__
#define __NX_DPC_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	DPC DPC
//------------------------------------------------------------------------------
//@{

	/// @brief	DPC Module's Register List
	struct	NX_DPC_RegisterSet
	{
		volatile U32 __Reserved[(0x07C-0x00)/2];///< 0x00-0x078*2 Reserved Regions
		volatile U32 DPCHTOTAL		;			///< 0x07C*2 : DPC Horizontal Total Length Register
		volatile U32 DPCHSWIDTH		;			///< 0x07E*2 : DPC Horizontal Sync Width Register
		volatile U32 DPCHASTART		;			///< 0x080*2 : DPC Horizontal Active Video Start Register
		volatile U32 DPCHAEND		;			///< 0x082*2 : DPC Horizontal Active Video End Register
		volatile U32 DPCVTOTAL		;			///< 0x084*2 : DPC Vertical Total Length Register
		volatile U32 DPCVSWIDTH		;			///< 0x086*2 : DPC Vertical Sync Width Register
		volatile U32 DPCVASTART		;			///< 0x088*2 : DPC Vertical Active Video Start Register
		volatile U32 DPCVAEND		;			///< 0x08A*2 : DPC Vertical Active Video End Register
		volatile U32 DPCCTRL0		;			///< 0x08C*2 : DPC Control 0 Register
		volatile U32 DPCCTRL1		;			///< 0x08E*2 : DPC Control 1 Register
		volatile U32 DPCEVTOTAL		;			///< 0x090*2 : DPC Even Field Vertical Total Length Register
		volatile U32 DPCEVSWIDTH	;			///< 0x092*2 : DPC Even Field Vertical Sync Width Register
		volatile U32 DPCEVASTART	;			///< 0x094*2 : DPC Even Field Vertical Active Video Start Register
		volatile U32 DPCEVAEND		;			///< 0x096*2 : DPC Even Field Vertical Active Video End Register
		volatile U32 DPCCTRL2		;			///< 0x098*2 : DPC Control 2 Register
		volatile U32 DPCVSEOFFSET	;			///< 0x09A*2 : DPC Vertical Sync End Offset Register
		volatile U32 DPCVSSOFFSET	;			///< 0x09C*2 : DPC Vertical Sync Start Offset Register
		volatile U32 DPCEVSEOFFSET	;			///< 0x09E*2 : DPC Even Field Vertical Sync End Offset Register
		volatile U32 DPCEVSSOFFSET	;			///< 0x0A0*2 : DPC Even Field Vertical Sync Start Offset Register
		volatile U32 DPCDELAY0		;			///< 0x0A2*2 : DPC Delay 0 Register
		volatile U32 DPCUPSCALECON0 ;			///< 0x0A4*2 : DPC Sync Upscale Control Register 0
		volatile U32 DPCUPSCALECON1 ;			///< 0x0A6*2 : DPC Sync Upscale Control Register 1
		volatile U32 DPCUPSCALECON2 ;			///< 0x0A8*2 : DPC Sync Upscale Control Register 2
		//@modified choiyk 2012-10-09 오전 11:10:34 : ( reference - NXP3200, LF2000, mes_disp03.h )
		volatile U32 DPCRNUMGENCON0 ;           ///< 0x0AA*2 : DPC Sync Random number generator control register 0
		volatile U32 DPCRNUMGENCON1 ;           ///< 0x0AC*2 : DPC Sync Random number generator control register 1
		volatile U32 DPCRNUMGENCON2 ;           ///< 0x0AE*2 : DPC Sync Random number generator control register 2
	    volatile U32 DPCRNDCONFORMULA_L;        ///< 0x0B0*2
	    volatile U32 DPCRNDCONFORMULA_H;        ///< 0x0B2*2
		volatile U32 DPCFDTAddr     ;           ///< 0x0B4*2 : DPC Sync frame dither table address register
		volatile U32 DPCFRDITHERVALUE;          ///< 0x0B6*2 : DPC Sync frame red dither table value register
		volatile U32 DPCFGDITHERVALUE;          ///< 0x0B8*2 : DPC Sync frame green dither table value register
		volatile U32 DPCFBDITHERVALUE;          ///< 0x0BA*2 : DPC Sync frame blue table value register
        volatile U32 DPCDELAY1       ;          ///< 0x0BC*2 : DPC Delay 1 Register
        volatile U32 DPCMPUTIME0     ;          ///< 0x0BE*2   @todo Find Databook of these registers..
        volatile U32 DPCMPUTIME1     ;          ///< 0x0C0*2
        volatile U32 DPCMPUWRDATAL   ;          ///< 0x0C2*2
        volatile U32 DPCMPUINDEX     ;          ///< 0x0C4*2
        volatile U32 DPCMPUSTATUS    ;          ///< 0x0C6*2
        volatile U32 DPCMPUDATAH     ;          ///< 0x0C8*2
        volatile U32 DPCMPURDATAL    ;          ///< 0x0CA*2
        volatile U32 DPCDummy12      ;	        ///< 0x0CC*2
        volatile U32 DPCCMDBUFFERDATAL;         ///< 0x0CE*2
        volatile U32 DPCCMDBUFFERDATAH;         ///< 0x0D0*2
        volatile U32 DPCPOLCTRL      ;          ///< 0x0D2*2
        volatile U32 DPCPADPOSITION[8];         ///< 0x0D4*2~0x0E2*2(4,6,8,A,C,E,0,2)
        volatile U32 DPCRGBMASK[2]   ;          ///< 0x0E4*2~0x0E6*2(4,6)
        volatile U32 DPCRGBSHIFT     ;          ///< 0x0E8*2
        volatile U32 DPCDATAFLUSH    ;		    ///< 0x0EA*2
		volatile U32 __Reserved06[( (0x3C0) - (2*0x0EC) )/4];		///< 0x0EC*2 ~ (0x3C0)
		// CLKGEN
		volatile U32 DPCCLKENB		;			///< 0x3C0 : DPC Clock Generation Enable Register
		volatile U32 DPCCLKGEN[2][2];			///< 0x3C4 : DPC Clock Generation Control 0 Low Register
												///< 0x3C8 : DPC Clock Generation Control 0 High Register
												///< 0x3CC : DPC Clock Generation Control 1 Low Register
												///< 0x3D0 : DPC Clock Generation Control 1 High Register
		// odd index is Reserved Region.
	};

	/// @brief	DPC interrupt for interrupt interface
	enum
	{
		NX_DPC_INT_VSYNC = 0		///< DPC VSYNC interrupt
	};

	/// @brief	the data output format.
	typedef	enum
	{
		NX_DPC_FORMAT_RGB555		= 0UL,	///< RGB555 Format
		NX_DPC_FORMAT_RGB565		= 1UL,	///< RGB565 Format
		NX_DPC_FORMAT_RGB666		= 2UL,	///< RGB666 Format
		NX_DPC_FORMAT_RGB666B		= 18UL,	///< RGB666 Format

		NX_DPC_FORMAT_RGB888		= 3UL,	///< RGB888 Format
		NX_DPC_FORMAT_MRGB555A		= 4UL,	///< MRGB555A Format
		NX_DPC_FORMAT_MRGB555B		= 5UL,	///< MRGB555B Format
		NX_DPC_FORMAT_MRGB565		= 6UL,	///< MRGB565 Format
		NX_DPC_FORMAT_MRGB666		= 7UL,	///< MRGB666 Format
		NX_DPC_FORMAT_MRGB888A		= 8UL,	///< MRGB888A Format
		NX_DPC_FORMAT_MRGB888B		= 9UL,	///< MRGB888B Format
		NX_DPC_FORMAT_CCIR656		= 10UL,	///< ITU-R BT.656 / 601(8-bit)
		NX_DPC_FORMAT_CCIR601A		= 12UL,	///< ITU-R BT.601A
		NX_DPC_FORMAT_CCIR601B		= 13UL,	///< ITU-R BT.601B
		NX_DPC_FORMAT_SRGB888		= 14UL,	///< SRGB888 Format
		NX_DPC_FORMAT_SRGBD8888		= 15UL,	///< SRGBD8888 Format
		NX_DPC_FORMAT_4096COLOR		= 1UL,	///< 4096 Color Format
		NX_DPC_FORMAT_16GRAY		= 3UL	///< 16 Level Gray Format

	}	NX_DPC_FORMAT;

	/// @brief	the data output order in case of ITU-R BT.656 / 601.
	typedef enum
	{
		NX_DPC_YCORDER_CbYCrY		= 0UL,	///< Cb, Y, Cr, Y
		NX_DPC_YCORDER_CrYCbY		= 1UL,	///< Cr, Y, Cb, Y
		NX_DPC_YCORDER_YCbYCr		= 2UL,	///< Y, Cb, Y, Cr
		NX_DPC_YCORDER_YCrYCb		= 3UL	///< Y, Cr, Y, Cb

	}	NX_DPC_YCORDER;

	///	@brief the PAD output clock.
	typedef enum
	{
		NX_DPC_PADCLK_VCLK		= 0UL,	///< VCLK
		NX_DPC_PADCLK_VCLK2		= 1UL,	///< VCLK2
		NX_DPC_PADCLK_VCLK3		= 2UL	///< VCLK3

	}	NX_DPC_PADCLK;

		/// @brief	RGB dithering mode.
	typedef	enum
	{
		NX_DPC_DITHER_BYPASS	= 0UL,	///< Bypass mode.
		NX_DPC_DITHER_4BIT		= 1UL,	///< 8 bit -> 4 bit mode.
		NX_DPC_DITHER_5BIT		= 2UL,	///< 8 bit -> 5 bit mode.
		NX_DPC_DITHER_6BIT		= 3UL	///< 8 bit -> 6 bit mode.

	}	NX_DPC_DITHER;

	///	@brief	Video Broadcast Standards.
	typedef enum
	{
		NX_DPC_VBS_NTSC_M		= 0UL,	///< NTSC-M				59.94 Hz(525)
		NX_DPC_VBS_NTSC_N		= 1UL,	///< NTSC-N				50 Hz(625)
		NX_DPC_VBS_NTSC_443		= 2UL,	///< NTSC-4.43			60 Hz(525)
		NX_DPC_VBS_PAL_M		= 3UL,	///< PAL-M				59.952 Hz(525)
		NX_DPC_VBS_PAL_N		= 4UL,	///< PAL-combination N	50 Hz(625)
		NX_DPC_VBS_PAL_BGHI		= 5UL,	///< PAL-B,D,G,H,I,N	50 Hz(625)
		NX_DPC_VBS_PSEUDO_PAL	= 6UL,	///< Pseudo PAL			60 Hz(525)
		NX_DPC_VBS_PSEUDO_NTSC	= 7UL	///< Pseudo NTSC		50 Hz (625)

	}	NX_DPC_VBS;

	///	@brief Luma/Chroma bandwidth control
	typedef enum
	{
		NX_DPC_BANDWIDTH_LOW		= 0UL,	///< Low bandwidth
		NX_DPC_BANDWIDTH_MEDIUM		= 1UL,	///< Medium bandwidth
		NX_DPC_BANDWIDTH_HIGH		= 2UL	///< High bandwidth

	}	NX_DPC_BANDWIDTH;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_DPC_Initialize( void );
U32		NX_DPC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_DPC_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_DPC_GetSizeOfRegisterSet( void );
void	NX_DPC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_DPC_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_DPC_OpenModule( U32 ModuleIndex );
CBOOL	NX_DPC_CloseModule( U32 ModuleIndex );
CBOOL	NX_DPC_CheckBusy( U32 ModuleIndex );
CBOOL	NX_DPC_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
S32		NX_DPC_GetInterruptNumber( U32 ModuleIndex );

void	NX_DPC_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable );
CBOOL	NX_DPC_GetInterruptEnable( U32 ModuleIndex, S32 IntNum );
CBOOL	NX_DPC_GetInterruptPending( U32 ModuleIndex, S32 IntNum );
void	NX_DPC_ClearInterruptPending( U32 ModuleIndex, S32 IntNum );

void	NX_DPC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_DPC_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_DPC_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_DPC_ClearInterruptPendingAll( U32 ModuleIndex );

void	NX_DPC_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32		NX_DPC_GetInterruptEnable32( U32 ModuleIndex );
U32		NX_DPC_GetInterruptPending32( U32 ModuleIndex );
void	NX_DPC_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );

S32		NX_DPC_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	Clock Control Interface
//@{
void		NX_DPC_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode );
NX_PCLKMODE	NX_DPC_GetClockPClkMode( U32 ModuleIndex );
void		NX_DPC_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc );
U32			NX_DPC_GetClockSource( U32 ModuleIndex, U32 Index );
void		NX_DPC_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor );
U32			NX_DPC_GetClockDivisor( U32 ModuleIndex, U32 Index );
void		NX_DPC_SetClockOutInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
CBOOL		NX_DPC_GetClockOutInv( U32 ModuleIndex, U32 Index );
void		NX_DPC_SetClockOutSelect( U32 ModuleIndex, U32 Index, CBOOL bBypass );
CBOOL		NX_DPC_GetClockOutSelect( U32 ModuleIndex, U32 Index );
void		NX_DPC_SetClockPolarity( U32 ModuleIndex, CBOOL bPolarity );
CBOOL		NX_DPC_GetClockPolarity( U32 ModuleIndex );
void		NX_DPC_SetClockOutEnb( U32 ModuleIndex, U32 Index, CBOOL OutClkEnb );
CBOOL		NX_DPC_GetClockOutEnb( U32 ModuleIndex, U32 Index );
void		NX_DPC_SetClockOutDelay( U32 ModuleIndex, U32 Index, U32 delay );
U32			NX_DPC_GetClockOutDelay( U32 ModuleIndex, U32 Index );
void		NX_DPC_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL		NX_DPC_GetClockDivisorEnable( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
///	@name	Display Controller(TFT) Operations
//--------------------------------------------------------------------------
//@{
void	NX_DPC_SetDPCEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_DPC_GetDPCEnable( U32 ModuleIndex );

void	NX_DPC_SetDelay( U32 ModuleIndex, U32 DelayRGB_PVD, U32 DelayHS_CP1, U32 DelayVS_FRAM, U32 DelayDE_CP2 );
void	NX_DPC_GetDelay( U32 ModuleIndex, U32 *pDelayRGB_PVD, U32 *pDelayHS_CP1, U32 *pDelayVS_FRAM, U32 *pDelayDE_CP2 );

void	NX_DPC_SetDither( U32 ModuleIndex, NX_DPC_DITHER DitherR, NX_DPC_DITHER DitherG, NX_DPC_DITHER DitherB );
void	NX_DPC_GetDither( U32 ModuleIndex, NX_DPC_DITHER *pDitherR, NX_DPC_DITHER *pDitherG, NX_DPC_DITHER *pDitherB );

void	NX_DPC_SetHorizontalUpScaler( U32 ModuleIndex, CBOOL bEnb, U32 sourceWidth, U32 destWidth );
void	NX_DPC_GetHorizontalUpScaler( U32 ModuleIndex, CBOOL* pbEnb, U32* psourceWidth, U32* pdestWidth );
//@}

//--------------------------------------------------------------------------
/// @name TFT LCD Specific Control Function
//--------------------------------------------------------------------------
//@{
void	NX_DPC_SetMode( U32 ModuleIndex,
							NX_DPC_FORMAT format,
							CBOOL bInterlace, CBOOL bInvertField,
							CBOOL bRGBMode, CBOOL bSwapRB,
							NX_DPC_YCORDER ycorder, CBOOL bClipYC, CBOOL bEmbeddedSync,
							NX_DPC_PADCLK clock, CBOOL bInvertClock, CBOOL bDualView );

void	NX_DPC_GetMode( U32 ModuleIndex,
							NX_DPC_FORMAT *pFormat,
							CBOOL *pbInterlace, CBOOL *pbInvertField,
							CBOOL *pbRGBMode, CBOOL *pbSwapRB,
							NX_DPC_YCORDER *pYCorder, CBOOL *pbClipYC, CBOOL *pbEmbeddedSync,
							NX_DPC_PADCLK *pClock, CBOOL *pbInvertClock, CBOOL *pbDualView );

void	NX_DPC_SetHSync( U32 ModuleIndex, U32 AVWidth, U32 HSW, U32 HFP, U32 HBP, CBOOL bInvHSYNC );
void	NX_DPC_GetHSync( U32 ModuleIndex, U32 *pAVWidth, U32 *pHSW, U32 *pHFP, U32 *pHBP, CBOOL *pbInvHSYNC );

void	NX_DPC_SetVSync( U32 ModuleIndex, U32 AVHeight, U32 VSW, U32 VFP, U32 VBP, CBOOL bInvVSYNC,
							U32 EAVHeight, U32 EVSW, U32 EVFP, U32 EVBP );

void	NX_DPC_GetVSync( U32 ModuleIndex, U32 *pAVHeight, U32 *pVSW, U32 *pVFP, U32 *pVBP, CBOOL *pbInvVSYNC,
							U32 *pEAVHeight, U32 *pEVSW, U32 *pEVFP, U32 *pEVBP );

void	NX_DPC_SetVSyncOffset( U32 ModuleIndex, U32 VSSOffset, U32 VSEOffset, U32 EVSSOffset, U32 EVSEOffset );
void	NX_DPC_GetVSyncOffset( U32 ModuleIndex, U32 *pVSSOffset, U32 *pVSEOffset, U32 *pEVSSOffset, U32 *pEVSEOffset );
//@}


//------------------------------------------------------------------------------
/// @brief	Prototype 이식.
//------------------------------------------------------------------------------
typedef enum
{
    PROGRESSIVE         = 0,
    INTERLACE           = 1
}SYNCGENMODE;

typedef enum
{
	POLARITY_ACTIVEHIGH  = 0,	///< active HIGH
	POLARITY_ACTIVELOW   = 1    ///< active LOW
}POLARITY;

typedef enum
{
	OUTPUTFORMAT_RGB555     =  0,	///<	256 color for stn-lcd
	OUTPUTFORMAT_RGB565     =  1,	///<	4096 color for stn-lcd
	OUTPUTFORMAT_RGB666     =  2,	///<	2 Gray Level for stn-lcd
	OUTPUTFORMAT_RGB888     =  3,	///<	16 Gray Level for stn-lcd
	OUTPUTFORMAT_MRGB555A   =  4,	///<
	OUTPUTFORMAT_MRGB555B   =  5,	///<
	OUTPUTFORMAT_MRGB565    =  6,	///<
	OUTPUTFORMAT_MRGB666    =  7,	///<
	OUTPUTFORMAT_MRGB888A   =  8,	///<
	OUTPUTFORMAT_MRGB888B   =  9,	///<
	OUTPUTFORMAT_BGR555     = 10,	///<
	OUTPUTFORMAT_BGR565     = 11,	///<
	OUTPUTFORMAT_BGR666     = 12,	///<
	OUTPUTFORMAT_BGR888     = 13,	///<
	OUTPUTFORMAT_MBGR555A   = 14,	///<
	OUTPUTFORMAT_MBGR555B   = 15,	///<
	OUTPUTFORMAT_MBGR565    = 16,	///<
	OUTPUTFORMAT_MBGR666    = 17,	///<
	OUTPUTFORMAT_MBGR888A   = 18,	///<
	OUTPUTFORMAT_MBGR888B   = 19,	///<
	OUTPUTFORMAT_CCIR656    = 20,	///<
	OUTPUTFORMAT_CCIR601_8  = 21,	///<
	OUTPUTFORMAT_CCIR601_16A= 22,	///<
	OUTPUTFORMAT_CCIR601_16B= 23,	///<
	OUTPUTFORMAT_SRGB888	= 24,	///<
	OUTPUTFORMAT_SRGBD8888	= 25	///<
}OUTPUTFORMAT;

typedef enum
{
    PADVCLK     = 0,    ///< Output pad clock vclk ( pixel clock )
    PADVCLK2    = 1,     ///< Output pad clock vclk*2 ( pixel clock*2 )
    PADVCLK3    = 2     ///< Output pad clock vclk*3 ( pixel clock*3 )
}OUTPADCLKSEL;

typedef enum
{
	QMODE_220 = 0,	///< 220 quantization mode
	QMODE_256 = 1	///< 256 quantization mode
}QMODE;







void NX_DPC_SetSync
(
    U32 ModuleIndex,
    SYNCGENMODE SyncGenMode,    ///< [in] interlace and progressive mode select
	U32 AVWidth,			    ///< [in] Width of Active video (unit:Clock)
	U32 AVHeight,			    ///< [in] Height of Active video(unit:Line)
	U32 HSW,				    ///< [in] Horizontal sync width (unit:Clock)
	U32 HFP,				    ///< [in] Horizontal front porch width (unit:Clock)
	U32 HBP,				    ///< [in] Horizontal back porch width (unit:Clock)
    U32 VSW,                    ///< [in] Vertical sync width (unit:Line)
    U32 VFP,                    ///< [in] Vertical front porch width (unit:Line)
    U32 VBP,                    ///< [in] Vertical back porch width (unit:Line)
	POLARITY FieldPolarity,	    ///< [in] Field polarity
	POLARITY HSyncPolarity,	    ///< [in] Horizontal sync polarity
	POLARITY VSyncPolarity,	    ///< [in] Vertical sync polarity
	U32 EvenVSW,                ///< [in] Vertical sync width (unit:Line)
	U32 EvenVFP,                ///< [in] Vertical front porch width (unit:Line)
	U32 EvenVBP,                ///< [in] Vertical back porch width (unit:Line)
    U32 VSETPIXEL,              ///< [in] Vertical sync Set number of pixel (unit:Clock)
    U32 VSCLRPIXEL,             ///< [in] Vertical sync clear number of pixel (unit:Clock)
    U32 EVENVSETPIXEL,          ///< [in] Vertical sync Set number of pixel (unit:Clock)
    U32 EVENVSCLRPIXEL          ///< [in] Vertical sync clear number of pixel (unit:Clock)
);
void NX_DPC_SetOutputFormat
(
    U32 ModuleIndex,
	OUTPUTFORMAT OutputFormat,	///< [in] video/color output format
	U8 OutputVideoConfig        ///< [in] video output config
	                            ///< [in] 0 : Cb Y Cr Y  |  1 : Cb Y Cr Y
	                            ///< [in] 2 : Cb Y Cr Y  |  3 : Cb Y Cr Y
);
void NX_DPC_SetQuantizationMode
(
    U32 ModuleIndex,
	QMODE RGB2YC,
	QMODE YC2RGB
);
void NX_DPC_SetEnable
(
    U32 ModuleIndex,
	CBOOL Enable,			///< [in] display controller enable
	CBOOL RGBMode,          ///< [in] output format reb & ycbcr enable
	CBOOL UseNTSCSync,		///< [in] use NTSC encoder sync
	CBOOL UseAnalogOutput,	///< [in] use analog output(use DAC)
	CBOOL SEAVEnable        ///< [in] Start of active and End of active Enable
);

void NX_DPC_SetOutVideoClkSelect
(
    U32 ModuleIndex,
    OUTPADCLKSEL OutPadVClkSel   ///< [in] Out Video clk selection
);

void NX_DPC_SetRegFlush( U32 ModuleIndex );

void NX_DPC_SetSRAMOn ( U32 ModuleIndex );

void NX_DPC_SetSyncLCDType
(
	U32 ModuleIndex,
	CBOOL STNLCD,
	CBOOL DualViewEnb,
	CBOOL BitWidh,
	U8 CPCycle
);



void NX_DPC_SetUpScaleControl
(
	U32 ModuleIndex,
	CBOOL UpScaleEnb, 		// Sync Generator Source image Scale Enable
	CBOOL FilterEnb,
	U32 HScale, 			// Scale
	U16 SourceWidth			// Sync Generator Source image Width
);


void NX_DPC_SetMPUTime(U32 ModuleIndex, U8 Setup, U8 Hold, U8 Acc);
void NX_DPC_SetIndex(U32 ModuleIndex, U32 Index)                  ;
void NX_DPC_SetData(U32 ModuleIndex, U32 Data)                    ;
void NX_DPC_SetCmdBufferFlush(U32 ModuleIndex )                   ;
void NX_DPC_SetCmdBufferClear( U32 ModuleIndex )                  ;
void NX_DPC_SetCmdBufferWrite(U32 ModuleIndex, U32 CmdData )      ;
void NX_DPC_SetMPUCS1( U32 ModuleIndex )                          ;
U32 NX_DPC_GetData( U32 ModuleIndex )                             ;
U32 NX_DPC_GetStatus( U32 ModuleIndex )                           ;

void NX_DPC_RGBMASK( U32 ModuleIndex, U32 RGBMask );

void NX_DPC_SetPadLocation( U32 ModuleIndex, U32 Index, U32 regvalue );

U32 NX_DPC_GetFieldFlag( U32 ModuleIndex );

#ifdef	__cplusplus
}
#endif


#endif // __NX_DPC_H__
