#include "../base/nx_prototype.h"

//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		: nx_MIPI.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_MIPI_H__
#define __NX_MIPI_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

#define NX_MIPI_NUMBEROF_CSI_CHANNELS 2

//------------------------------------------------------------------------------
/// @defgroup	MIPI
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
	// CSI [0x0000~0x00FF]
	volatile U32 CSIS_CONTROL   ; // 0x0000 R/W Specifies the control register.                     0x0010_0000     {O_DpDnSwap_Clk,O_DpDnSwap_Dat,2'h0,O_Decomp_form,O_Decomp_predict[0],Decomp_en,O_Interleave_mode,O_Doublecmp[0],O_Bus_32[0],Update_Shadow, O_RGB_SWAP, O_WCLK_Src,3'h0,O_SW_RST,O_NumOfDatlane, 1'h0, O_CSIS_EN}
	volatile U32 CSIS_DPHYCTRL  ; // 0x0004 R/W Specifies the D-PHY control register.               0x0000_0000     reserved[31:5] on S5PC1010 {3'b0,S_HSSETTLECTL[4:0],S_CLKSETTLECTL[1:0],17'b0,S_ENABLE3,S_ENABLE2,S_ENABLE1,S_ENABLE0,S_ENABLECLK}
	volatile U32 CSIS_CONFIG_CH0; // 0x0008 R/W Specifies the configuration register of CH0.        0x0000_0000     {Hsync_LIntv[5:0],Vsync_SIntv[5:0],Vsync_EIntv[11:0],DataFormat[5:0],Virtual_CH[1:0]};
	volatile U32 CSIS_DPHYSTS   ; // 0x000C R   Specifies the D-PHY stop state register.            0x0000_00FC
	volatile U32 CSIS_INTMSK    ; // 0x0010 R/W Specifies the interrupt mask register.              0x0000_0000
	volatile U32 CSIS_INTSRC    ; // 0x0014 R/W Specifies the interrupt status register.            0x0000_0000
	volatile U32 CSIS_CTRL2     ; // 0x0018 R/W Control register about ch1~3                        0x00E0_0000     {O_Decomp_predict[3:1], 1'b0, O_Doublecmp[3:1], 1'b0, O_Bus_32[3:1], 16'b0,O_ch_output_else, I_ch_output};
	volatile U32 CSIS_VERSION   ; // 0x001C R   CSIS version register                               0x0300_0000
	volatile U32 CSIS_DPHYCTRL_0; // 0x0020 R/W DPHY Analog Control register0                       0x0000_0000     O_B_DphyCtl, reserved on S5PC1010 (OPEN)
	volatile U32 CSIS_DPHYCTRL_1; // 0x0024 R/W DPHY Analog Control register1                       0x0000_0000     O_S_DphyCtl, reserved on S5PC1010 {16'b0,S_DPHY_CTL[15:0]}
	volatile U32 __Reserved0    ; // 0x0028	 	                                                    0x0000_0000
	volatile U32 CSIS_RESOL_CH0 ; // 0x002C R/W Specifies the image resolution register of CH0.     0x8000_8000     {HResol[15:0],VResol[15:0]};
	volatile U32 __Reserved1    ; // 0x0030                                                         0x0000_0000
	volatile U32 __Reserved2    ; // 0x0034                                                         0x8000_8000
	volatile U32 SDW_CONFIG_CH0 ; // 0x0038 R/W Specifies the shadow register of CH0 configuration. 0x0000_00FC     {O_Hsync_LIntv[5:0],O_Vsync_SIntv[5:0],O_Vsync_EIntv[11:0],O_DataFormat[5:0],O_Virtual_CH[1:0]};
	volatile U32 SDW_RESOL_CH0  ; // 0x003C R   Specifies the shadow register of CH0 resolution.    0x8000_8000	    {O_HResol[15:0],O_VResol[15:0]};
	volatile U32 CSIS_CONFIG_CH1; // 0x0040 R/W Specifies the configuration register of CH1.        0x0000_00FC     {Hsync_LIntv[11:6],Vsync_SIntv[11:6],Vsync_EIntv[23:12],DataFormat[11:6],Virtual_CH[3:2]};
	volatile U32 CSIS_RESOL_CH1 ; // 0x0044 R/W Specifies the image resolution register of CH1.     0x8000_8000     {HResol[31:16],VResol[31:16]};
	volatile U32 SDW_CONFIG_CH1 ; // 0x0048 R/W Specifies the shadow register of CH1 configuration. 0x0000_00FC     {O_Hsync_LIntv[11:6],O_Vsync_SIntv[11:6],O_Vsync_EIntv[23:12],O_DataFormat[11:6],O_Virtual_CH[3:2]};
	volatile U32 SDW_RESOL_CH1  ; // 0x004C R   Specifies the shadow register of CH1 resolution.    0x8000_8000     {O_HResol[31:16],O_VResol[31:16]};
	volatile U32 CSIS_CONFIG_CH2; // 0x0050 R/W Specifies the configuration register of CH2.        0x0000_00FC     {Hsync_LIntv[17:12],Vsync_SIntv[17:12],Vsync_EIntv[35:24],DataFormat[17:12],Virtual_CH[5:4]};
	volatile U32 CSIS_RESOL_CH2 ; // 0x0054 R/W Specifies the image resolution register of CH2.     0x8000_8000     {HResol[47:32],VResol[47:32]};
	volatile U32 SDW_CONFIG_CH2 ; // 0x0058 R/W Specifies the shadow register of CH2 configuration. 0x0000_00FC     {O_Hsync_LIntv[17:12],O_Vsync_SIntv[17:12],O_Vsync_EIntv[35:24],O_DataFormat[17:12],O_Virtual_CH[5:4]};
	volatile U32 SDW_RESOL_CH2  ; // 0x005C R   Specifies the shadow register of CH2 resolution.    0x8000_8000     {O_HResol[47:32],O_VResol[47:32]};
	volatile U32 CSIS_CONFIG_CH3; // 0x0060 R/W Specifies the configuration register of CH3.        0x0000_00FC     {Hsync_LIntv[23:18],Vsync_SIntv[23:18],Vsync_EIntv[47:36],DataFormat[23:18],Virtual_CH[7:6]};
	volatile U32 CSIS_RESOL_CH3 ; // 0x0064 R/W Specifies the image resolution register of CH3.     0x8000_8000     {HResol[63:48],VResol[63:48]};
	volatile U32 SDW_CONFIG_CH3 ; // 0x0068 R/W Specifies the shadow register of CH3 configuration. 0x0000_00FC     {O_Hsync_LIntv[23:18],O_Vsync_SIntv[23:18],O_Vsync_EIntv[47:36],O_DataFormat[23:18],O_Virtual_CH[7:6]};
	volatile U32 SDW_RESOL_3    ; // 0x006C R   Specifies the shadow register of CH3 resolution.    0x8000_8000     {O_HResol[63:48],O_VResol[63:48]};
	volatile U32 __Reserved3[(16+128)/4] ;

	// DISM [0x0100~0x01FF]
	volatile U32 DSIM_STATUS    ; // 0x0100 R   Specifies the status register.                        0x0010_010F
	volatile U32 DSIM_SWRST     ; // 0x0104 R/W Specifies the software reset register.                0x0000_0000
	volatile U32 DSIM_CLKCTRL   ; // 0x0108 R/W Specifies the clock control register.                 0x0000_FFFF
	volatile U32 DSIM_TIMEOUT   ; // 0x010C R/W Specifies the time out register.                      0x00FF_FFFF
	volatile U32 DSIM_CONFIG    ; // 0x0110 R/W Specifies the configuration register.                 0x0200_0000
	volatile U32 DSIM_ESCMODE   ; // 0x0114 R/W Specifies the escape mode register.                   0x0000_0000
	volatile U32 DSIM_MDRESOL   ; // 0x0118 R/W Specifies the main display image resolution register. 0x0300_0400
	volatile U32 DSIM_MVPORCH   ; // 0x011C R/W Specifies the main display Vporch register.           0xF000_0000
	volatile U32 DSIM_MHPORCH   ; // 0x0120 R/W Specifies the main display Hporch register.           0x0000_0000
	volatile U32 DSIM_MSYNC     ; // 0x0124 R/W Specifies the main display Sync Area register.        0x0000_0000
	volatile U32 DSIM_SDRESOL   ; // 0x0128 R/W Specifies the sub display image resolution register.  0x0300_0400
	volatile U32 DSIM_INTSRC    ; // 0x012C R/W Specifies the interrupt source register.              0x0000_0000
	volatile U32 DSIM_INTMSK    ; // 0x0130 R/W Specifies the interrupt mask register.                0xB337_FFFF
	volatile U32 DSIM_PKTHDR    ; // 0x0134 W   Specifies the packet header FIFO register.            0x0000_0000
	volatile U32 DSIM_PAYLOAD   ; // 0x0138 W   Specifies the payload FIFO register.                  0x0000_0000
	volatile U32 DSIM_RXFIFO    ; // 0x013C R   Specifies the read FIFO register.                     0xXXXX_XXXX
	volatile U32 DSIM_FIFOTHLD  ; // 0x0140 R/W Specifies the FIFO threshold level register.          0x0000_01FF
	volatile U32 DSIM_FIFOCTRL  ; // 0x0144 R   Specifies the FIFO status and control register.       0x0155_551F
	volatile U32 DSIM_MEMACCHR  ; // 0x0148 R/W Specifies the FIFO memory AC characteristic register. 0x0000_4040
	volatile U32 DSIM_PLLCTRL   ; // 0x014C R/W Specifies the PLL control register.                   0x0000_0000 ==> { M_HSZEROCTL[3:0], M_BANDCTL[3:0], M_PLLEN, M_PRPRCTL[2:0], M_PLLCTL[18:0],1'b0 }
	volatile U32 DSIM_PLLTMR    ; // 0x0150 R/W Specifies the PLL timer register.                     0xFFFF_FFFF
	volatile U32 DSIM_PHYACCHR  ; // 0x0154 R/W M_DphyCtl
	volatile U32 DSIM_PHYACCHR1 ; // 0x0158 R/W {B_DphyCtl,8'b0,PRPRCTLCLK[2:0],2'b0,DpDnSwap_CLK,DpDnSwap_DAT}  ==> {B_DPHYDTL[22:0], 2'b0, M_PRPRCTLCLK[2:0], 2'b0,M_DPDN_SWAP_CLK, M_DPDN_SWAP_DAT}
	//volatile U32 DSIM_PHYCTRL   ; // 0x015C R/W Specifies the D-PHY AC characteristic register.       0x0000_0000 ==> ???
	//volatile U32 DSIM_PHYCTRL1  ; // 0x0160 R/W Specifies the D-PHY AC characteristic register 1.     0x0000_0000 ==> ???
	//volatile U32 DSIM_PHYTIMING ; // 0x0164 R/W Specifies the D-PHY timing register.                  0x0000_0000 ==> ???
	//volatile U32 DSIM_PHYTIMING1; // 0x0168 R/W Specifies the D-PHY timing register 1.                0x0000_0000 ==> ???
	//volatile U32 DSIM_PHYTIMING2; // 0x016C R/W Specifies the D-PHY timing register 1.                0x0000_0000 ==> ???
	//volatile U32 DSIM_VERSION   ; // 0x0170 R   Specifies the DSIM version register.                  0x8000_0001

	//volatile U32 __Reserved4    [(0x2000-0x0174)/4];
	volatile U32 __Reserved4    [(0x2000-0x015C)/4];

	// CSI DATA [0x2000~0x3FFF]
	volatile U32 MIPI_CSIS_PKTDATA  [0x2000/4] ; // 0x2000 R   Specifies the memory area for storing non-image data.
} NX_MIPI_RegisterSet;

//------------------------------------------------------------------------------
///	@name	MIPI-CSI Interface
//------------------------------------------------------------------------------
//@{
void  NX_MIPI_CSI_SetSize  ( U32 ModuleIndex, int Channel, U32 Width, U32 Height );

typedef enum
{
	NX_MIPI_CSI_FORMAT_YUV420_8   = 0x18, // YUV420 (8bit)
	NX_MIPI_CSI_FORMAT_YUV420_10  = 0x19, // YUV420 (10bit)
	NX_MIPI_CSI_FORMAT_YUV420_8L  = 0x1A, // YUV420 (8bit legacy)
	NX_MIPI_CSI_FORMAT_YUV420_8C  = 0x1C, // YUV420 (8bit CSPS)
	NX_MIPI_CSI_FORMAT_YUV420_10C = 0x1D, // YUV420 (10bit CSPS)
	NX_MIPI_CSI_FORMAT_YUV422_8   = 0x1E, // YUV422 (8bit)
	NX_MIPI_CSI_FORMAT_YUV422_10  = 0x1F, // YUV422 (10bit)
	NX_MIPI_CSI_FORMAT_RGB565     = 0x22, // RGB565
	NX_MIPI_CSI_FORMAT_RGB666     = 0x23, // RGB666
	NX_MIPI_CSI_FORMAT_RGB888     = 0x24, // RGB888
	NX_MIPI_CSI_FORMAT_RAW6       = 0x28, // RAW6
	NX_MIPI_CSI_FORMAT_RAW7       = 0x29, // RAW7
	NX_MIPI_CSI_FORMAT_RAW8       = 0x2A, // RAW8
	NX_MIPI_CSI_FORMAT_RAW10      = 0x2B, // RAW10
	NX_MIPI_CSI_FORMAT_RAW12      = 0x2C, // RAW12
	NX_MIPI_CSI_FORMAT_RAW14      = 0x2D, // RAW14
	NX_MIPI_CSI_FORMAT_User0      = 0x30, // User defined 1
	NX_MIPI_CSI_FORMAT_User1      = 0x31, // User defined 2
	NX_MIPI_CSI_FORMAT_User2      = 0x32, // User defined 3
	NX_MIPI_CSI_FORMAT_User3      = 0x33, // User defined 4
} NX_MIPI_CSI_FORMAT;
void  NX_MIPI_CSI_SetFormat( U32 ModuleIndex, int Channel, NX_MIPI_CSI_FORMAT Format );

typedef enum
{
	NX_MIPI_CSI_INTERLEAVE_CH0  = 0, ///< CH0 only, no data interleave
	NX_MIPI_CSI_INTERLEAVE_DT   = 1, ///< Data type only
	NX_MIPI_CSI_INTERLEAVE_VC   = 2, ///< Virtual channel only
	NX_MIPI_CSI_INTERLEAVE_VCDT = 3  ///< Virtual channel and Data type
} NX_MIPI_CSI_INTERLEAVE; // Select Interleave mode, VC(Virtual channel) and DT(Data type)
void  NX_MIPI_CSI_SetInterleaveMode( U32 ModuleIndex, NX_MIPI_CSI_INTERLEAVE InterleaveMode );

//	Set timing of Output Protocol of ISP (unit: VCLK)
//	T1 : Interval between rising of VVALID and first rising of HVALID  : 1~64
//	T2 : Interval between last falling of DVALID and falling of HVALID : 2~66
//	T3 : Interval between falling of HVALID and rising of next HVALID  : 1
//	T4 : Interval between rising of HVALID and first rising of DVALID  : 0
//	T5 : Interval between last falling of HVALID and falling of VVALID : 0~4095
//	T6 : Interval between falling of VVALID and rising of next VVALID  : 1
void  NX_MIPI_CSI_SetTimingControl( U32 ModuleIndex, int Channel, int T1, int T2, int T5 );

// Set virtual channel for data interleave
void  NX_MIPI_CSI_SetInterleaveChannel( U32 ModuleIndex, int Channel, int InterleaveChannel );

void  NX_MIPI_CSI_EnableDecompress  ( U32 ModuleIndex, CBOOL Enable );

typedef enum
{
	NX_MIPI_CSI_PREDICTION_SIMPLE = 0, ///< Simple decompress prediction mode.
	NX_MIPI_CSI_PREDICTION_NORMAL = 1  ///< Advanced decompress prediction mode.
} NX_MIPI_CSI_PREDICTION; // Decompress prediction mode.
void  NX_MIPI_CSI_SetPrediction ( U32 ModuleIndex, int Channel, NX_MIPI_CSI_PREDICTION Prediction );

typedef enum
{
	NX_MIPI_CSI_YUV422LAYOUT_HALF = 0, ///< half pixel per clock cycle {8'hxx,YUV[7:0],16'hxxxx} or {8'hxx,YUV[9:0],14'hxxxx}
	NX_MIPI_CSI_YUV422LAYOUT_FULL = 1  ///< a pixel per clock cycle    {8'hxx,Y[7:0],UV[7:0],8'hxx} or {8'hxx,Y[9:2],UV[9:2],Y[1:0],UV[1:0],4'hx}
} NX_MIPI_CSI_YUV422LAYOUT;
void  NX_MIPI_CSI_SetYUV422Layout ( U32 ModuleIndex, int Channel, NX_MIPI_CSI_YUV422LAYOUT YUV422_Layout );

// When this is enabled, the outer bus width of MIPI CSIS V3.0 is 32.
// Refers to figures in chapter 12 of MIPI CSI-2 standard spec.
void  NX_MIPI_CSI_SetParallelDataAlignment32 ( U32 ModuleIndex, int Channel, CBOOL EnableAlign32 );

typedef enum
{
	NX_MIPI_CSI_RGBLAYOUT_RGB = 0, ///< MSB is R and LSB is B. {xxx,R[],G[],B[]},
	NX_MIPI_CSI_RGBLAYOUT_BGR = 1  ///< MSB is B and LSB is R. {xxx,B[],G[],R[]},
} NX_MIPI_CSI_RGBLAYOUT;
void  NX_MIPI_CSI_SetRGBLayout ( U32 ModuleIndex, int Channel, NX_MIPI_CSI_RGBLAYOUT RGB_Layout );

typedef enum
{
	NX_MIPI_CSI_VCLKSRC_PCLK   = 0, ///< APB BUS clock used for output VCLK
	NX_MIPI_CSI_VCLKSRC_EXTCLK = 1  ///< CLKGEN clock used for output VCLK (refer to NX_MIPI_GetClockNumber )
} NX_MIPI_CSI_VCLKSRC;
void  NX_MIPI_CSI_SetVCLK( U32 ModuleIndex, int Channel, NX_MIPI_CSI_VCLKSRC ClockSource );

// Software reset. All writable registers in CSI2 go back to initial state.
// Almost MIPI CSI2 block uses ByteClk from D-phy. This ByteClk is not continuous clock.
// User has to assert software reset when Camera module is turned off.
void  NX_MIPI_CSI_SoftwareReset ( U32 ModuleIndex );

// MIPI CSI2 system enable
void  NX_MIPI_CSI_SetEnable ( U32 ModuleIndex, CBOOL Enable );

// D-phy enable
//  Number of data lanes
//  0: 1 data lane
//  1: 2 data lanes
//  2: 3 data lanes
//  3: 4 data lanes.
void  NX_MIPI_CSI_SetPhy( U32 ModuleIndex,
						  U32   NumberOfDataLanes              , // 0~3
                          CBOOL EnableClockLane                ,
                          CBOOL EnableDataLane0                ,
                          CBOOL EnableDataLane1                ,
                          CBOOL EnableDataLane2                ,
                          CBOOL EnableDataLane3                ,
                          CBOOL SwapClockLane                  ,
                          CBOOL SwapDataLane                   );

//	Read current state through shadow register
void  NX_MIPI_CSI_GetCurrentState( U32 ModuleIndex, int Channel, int* pWidth, int* pHeight, int* pT1, int* pT2, int* pT5, NX_MIPI_CSI_FORMAT* pFormat, int* pInterleaveChannel );


//	Address32 : 0x0000/4~0x1000/4 Odd  frame
//	Address32 : 0x1000/4~0x2000/4 Even frame
U32   NX_MIPI_CSI_GetNonImageData( U32 ModuleIndex, U32 Address32 );

//  bit 0 of ULPS : '1' means data lane 0 is ULPS.
//  bit 1 of ULPS : '1' means data lane 1 is ULPS.
//  bit 2 of ULPS : '1' means data lane 2 is ULPS.
//  bit 3 of ULPS : '1' means data lane 3 is ULPS.
//  bit 4 of ULPS : '1' means clock lane is ULPS.
//  bit 0 of Stop : '1' means data lane 0 is stop state.
//  bit 1 of Stop : '1' means data lane 1 is stop state.
//  bit 2 of Stop : '1' means data lane 2 is stop state.
//  bit 3 of Stop : '1' means data lane 3 is stop state.
//  bit 4 of Stop : '1' means clock lane is stop state.
void  NX_MIPI_CSI_GetStatus( U32 ModuleIndex, U32* pULPS, U32* pStop );

U32   NX_MIPI_CSI_GetVersion ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	MIPI-DSI Interface
//------------------------------------------------------------------------------
//@{
//  bit 0 of ULPS : '1' means data lane 0 is ULPS.
//  bit 1 of ULPS : '1' means data lane 1 is ULPS.
//  bit 2 of ULPS : '1' means data lane 2 is ULPS.
//  bit 3 of ULPS : '1' means data lane 3 is ULPS.
//  bit 4 of ULPS : '1' means clock lane is ULPS.
//  bit 0 of Stop : '1' means data lane 0 is stop state.
//  bit 1 of Stop : '1' means data lane 1 is stop state.
//  bit 2 of Stop : '1' means data lane 2 is stop state.
//  bit 3 of Stop : '1' means data lane 3 is stop state.
//  bit 4 of Stop : '1' means clock lane is stop state.
void  NX_MIPI_DSI_GetStatus( U32 ModuleIndex, U32* pULPS, U32* pStop, U32* pIsPLLStable, U32* pIsInReset, U32* pIsBackward, U32* pIsHSClockReady );

void  NX_MIPI_DSI_SoftwareReset ( U32 ModuleIndex );

void  NX_MIPI_DSI_SetClock ( U32 ModuleIndex,
                             CBOOL EnableTXHSClock    ,
                             CBOOL UseExternalClock   , // CFALSE: PLL clock CTRUE: External clock
                             CBOOL EnableByteClock    , // ByteClock means (D-PHY PLL clock / 8)
                             CBOOL EnableESCClock_ClockLane,
                             CBOOL EnableESCClock_DataLane0,
                             CBOOL EnableESCClock_DataLane1,
                             CBOOL EnableESCClock_DataLane2,
                             CBOOL EnableESCClock_DataLane3,
                             CBOOL EnableESCPrescaler , // ESCClock = ByteClock / ESCPrescalerValue
                             U32   ESCPrescalerValue  );// 0~0xFFFF

//	BtaTout
//		~255
//      Specifies the timer for BTA.
//		This register specifies time out from BTA request to change
//		the direction with respect to Tx escape clock.
//	LPdrTout
//		~65535
//		Specifies the timer for LP Rx mode timeout. This register specifies time out
//		on how long RxValid deasserts, after RxLpdt asserts with respect to Tx escape clock.
//		RxValid specifies Rx data valid indicator. RxLpdt specifies an indicator
//		that D-phy is under RxLpdt mode.
//		RxValid and RxLpdt specifies signal from D-phy.
void  NX_MIPI_DSI_SetTimeout( U32 ModuleIndex, U32 BtaTout, U32 LPdrTout );

typedef enum
{
	NX_MIPI_DSI_SYNCMODE_EVENT = 0, //
	NX_MIPI_DSI_SYNCMODE_PULSE = 1, // EnableBurst must be CFALSE
} NX_MIPI_DSI_SYNCMODE;
typedef enum
{
	NX_MIPI_DSI_FORMAT_COMMAND3      = 0, // for NX_MIPI_DSI_SetConfigCommandMode only
	NX_MIPI_DSI_FORMAT_COMMAND8      = 1, // for NX_MIPI_DSI_SetConfigCommandMode only
	NX_MIPI_DSI_FORMAT_COMMAND12     = 2, // for NX_MIPI_DSI_SetConfigCommandMode only
	NX_MIPI_DSI_FORMAT_COMMAND16     = 3, // for NX_MIPI_DSI_SetConfigCommandMode only
	NX_MIPI_DSI_FORMAT_RGB565        = 4, // for NX_MIPI_DSI_SetConfigVideoMode only
	NX_MIPI_DSI_FORMAT_RGB666_PACKED = 5, // for NX_MIPI_DSI_SetConfigVideoMode only
	NX_MIPI_DSI_FORMAT_RGB666        = 6,
	NX_MIPI_DSI_FORMAT_RGB888        = 7
} NX_MIPI_DSI_FORMAT;
void  NX_MIPI_DSI_SetConfigVideoMode  ( U32 ModuleIndex                      ,
										CBOOL EnableAutoFlushMainDisplayFIFO ,
										CBOOL EnableAutoVerticalCount        ,
										CBOOL EnableBurst                    ,
										NX_MIPI_DSI_SYNCMODE SyncMode        ,
										CBOOL EnableEoTPacket                ,
										CBOOL EnableHsyncEndPacket           , // Set HSEMode=1
										CBOOL EnableHFP                      , // Set HFPMode=0
										CBOOL EnableHBP                      , // Set HBPMode=0
										CBOOL EnableHSA                      , // Set HSAMode=0
										U32   NumberOfVirtualChannel         , // 0~3
										NX_MIPI_DSI_FORMAT Format            ,
			                            U32   NumberOfWordsInHFP             , // ~65535
			                            U32   NumberOfWordsInHBP             , // ~65535
			                            U32   NumberOfWordsInHSYNC           , // ~65535
			                            U32   NumberOfLinesInVFP             , // ~2047
			                            U32   NumberOfLinesInVBP             , // ~2047
			                            U32   NumberOfLinesInVSYNC           , // ~1023
			                            U32   NumberOfLinesInCommandAllow      // ~15
			                            );

void  NX_MIPI_DSI_SetConfigCommandMode( U32 ModuleIndex                      ,
										CBOOL EnableAutoFlushMainDisplayFIFO ,
										CBOOL EnableEoTPacket                ,
										U32   NumberOfVirtualChannel         , // 0~3
										NX_MIPI_DSI_FORMAT Format
			                            );
typedef enum
{
	NX_MIPI_DSI_LPMODE_HS = 0,
	NX_MIPI_DSI_LPMODE_LP = 1
} NX_MIPI_DSI_LPMODE;
void  NX_MIPI_DSI_SetEscapeMode( U32 ModuleIndex, U32 StopStateCount, CBOOL ForceStopState, CBOOL ForceBTA, NX_MIPI_DSI_LPMODE CmdinLP, NX_MIPI_DSI_LPMODE TXinLP );

void  NX_MIPI_DSI_RemoteResetTrigger( U32 ModuleIndex );

void  NX_MIPI_DSI_SetULPS( U32 ModuleIndex, CBOOL ULPSClockLane, CBOOL ULPSDataLane );

void  NX_MIPI_DSI_SetSize  ( U32 ModuleIndex, U32 Width, U32 Height );

void  NX_MIPI_DSI_SetEnable( U32 ModuleIndex, CBOOL Enable );

//0x34~0x48

void  NX_MIPI_DSI_SetPhy( U32 ModuleIndex,
						  U32   NumberOfDataLanes , // 0~3
                          CBOOL EnableClockLane   ,
                          CBOOL EnableDataLane0   ,
                          CBOOL EnableDataLane1   ,
                          CBOOL EnableDataLane2   ,
                          CBOOL EnableDataLane3   ,
                          CBOOL SwapClockLane     ,
                          CBOOL SwapDataLane      );

typedef enum
{
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_20_MHZ  = 0x1F4, // 20.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_19_MHZ  = 0x1DB, // 19.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_18_MHZ  = 0x1C2, // 18.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_17_MHZ  = 0x1A9, // 17.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_16_MHZ  = 0x190, // 16.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_15_MHZ  = 0x177, // 15.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_14_MHZ  = 0x15E, // 14.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_13_MHZ  = 0x145, // 13.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_12_MHZ  = 0x12C, // 12.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_11_MHZ  = 0x113, // 11.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_10_MHZ  = 0x0FA, // 10.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_9_MHZ   = 0x0E1, //  9.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_8_MHZ   = 0x0C8, //  8.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_7_MHZ   = 0x0AF, //  7.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_6_MHZ   = 0x096, //  6.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_5_MHZ   = 0x07D, //  5.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_4_MHZ   = 0x064, //  4.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_3_MHZ   = 0x04B, //  3.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_2_MHZ   = 0x032, //  2.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_1_MHZ   = 0x019, //  1.00 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_0_10_MHZ= 0x003, //  0.10 MHz
	NX_MIPI_PHY_B_DPHYCTL_M_TXCLKESC_0_01_MHZ= 0x000  //  0.01 MHz
} NX_MIPI_PHY_B_DPHYCTL;
void  NX_MIPI_DSI_SetPLL( U32 ModuleIndex   ,
                          CBOOL Enable      ,
					      U32 PLLStableTimer,
						  U32 M_PLLPMS      , // 19'h033E8: 1Ghz  19'h043E8: 750Mhz // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
						  U32 M_BANDCTL     , // 4'hF: 1Ghz  4'hC: 750Mhz           // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
						  U32 M_DPHYCTL     , //
						  U32 B_DPHYCTL       // Refer to 10.2.3 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf or NX_MIPI_PHY_B_DPHYCTL enum or LN28LPP_MipiDphyCore1p5Gbps_Supplement.
						  );

//U32   NX_MIPI_DSI_GetVersion ( U32 ModuleIndex );

void  NX_MIPI_DSI_WritePacket      ( U32 ModuleIndex, U32 DataCount32, const U32* pData32 );
//void  NX_MIPI_DSI_ReadPacketHeader ( U32 ModuleIndex, U32* pDataID, U32* pDataCount32 );
//void  NX_MIPI_DSI_ReadPacketData   ( U32 ModuleIndex, U32 DataCount32, const U32* pData32 );


//	P : 1~33
//	M : 41~125
//	S : 0~3
//	PLLin : 6~200Mhz
//	PLLout = ( M * PLLin ) / ( P * (2^S) )
//	ASSERT( 37.5Mhz <= PLLout <= 750Mhz ) ==> DataRate( 80~1500Mbps )
//	VCOout = ( M * PLLin ) / P
//	ASSERT( 500Mhz <= VCOout <= 750Mhz )
//	FinPLL = PLLin / P
//	ASSERT( 6Mhz <= FinPLL <= 12Mhz )
//  PMS[18:1] = {P[5:0],M[8:0],S[2:0]}
//

// typedef enum
// {
// 	NX_MIPI_DSI_AFC_6  = 1, // ByteClock x 8 is in 6~6.99Mhz
// 	NX_MIPI_DSI_AFC_7  = 0, // ByteClock x 8 is in 7~7.99Mhz
// 	NX_MIPI_DSI_AFC_8  = 3, // ByteClock x 8 is in 8~8.99Mhz
// 	NX_MIPI_DSI_AFC_9  = 2, // ByteClock x 8 is in 9~9.99Mhz
// 	NX_MIPI_DSI_AFC_10 = 5, // ByteClock x 8 is in 10~10.99Mhz
// 	NX_MIPI_DSI_AFC_11 = 4, // ByteClock x 8 is in 11~12Mhz
// } NX_MIPI_DSI_AFC; // Adaptive Frequency Calibrator
//  CBOOL EnableAFC, NX_MIPI_DSI_AFC AFCControl );

//@}

//------------------------------------------------------------------------------
///	@name	MIPI-PHY Interface
//------------------------------------------------------------------------------
//@{
//
//	MIPI PHY control registers.
//	Refer to chapter 10 of MIPI_D_PHY_USER_GUIDE.pdf
//	You should call this during power-up sequence.
//
// void  NX_MIPI_PHY_SetControl( U32 ModuleIndex   ,
// 							  int M_TCLKPRPRCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TCLKZEROCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TCLKPOSTCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TCLKTRAILCTL, // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSPRPRCTL  , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSZEROCTL  , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSTRAILCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TLPXCTL     , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSEXITCTL  , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int S_HSSETTLECTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. (4.3 of UG_MOCO_MIPI_CSIS_V3.0.pdf)
// 							  int S_CLKSETTLECTL, // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. (4.3 of UG_MOCO_MIPI_CSIS_V3.0.pdf)
// 							  int M_PLLPMS      , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_PLLCTL      , // Refer to 10.2.2 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf
// 							  int B_DPHYCTL     );// Use NX_MIPI_PHY_B_DPHYCTL enum or refer to LN28LPP_MipiDphyCore1p5Gbps_Supplement. B_DPHYCTL[9:0]
//
//@}

CBOOL  NX_MIPI_SmokeTest ( U32 ModuleIndex );

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_MIPI_Initialize( void );
U32   NX_MIPI_GetNumberOfModule( void );

U32   NX_MIPI_GetSizeOfRegisterSet( void );
void  NX_MIPI_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void* NX_MIPI_GetBaseAddress( U32 ModuleIndex );
U32   NX_MIPI_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_MIPI_OpenModule( U32 ModuleIndex );
CBOOL NX_MIPI_CloseModule( U32 ModuleIndex );
CBOOL NX_MIPI_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_MIPI_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
/// @brief  MIPI reset channel number for NX_MIPI_GetResetNumber
enum
{
	NX_MIPI_RST       = 0,
	NX_MIPI_RST_DSI_I    ,
	NX_MIPI_RST_CSI_I    ,
	NX_MIPI_RST_PHY_S    ,
	NX_MIPI_RST_PHY_M
};
U32 NX_MIPI_GetResetNumber ( U32 ModuleIndex, U32 ChannelIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
	NX_MIPI_INT_CSI_EvenBefore       = 31,    ///< Non Image data are received at Even frame and Before Image
	NX_MIPI_INT_CSI_EvenAfter        = 30,    ///< Non Image data are received at Even frame and After Image
	NX_MIPI_INT_CSI_OddBefore        = 29,    ///< Non Image data are received at Odd frame and Before Image
	NX_MIPI_INT_CSI_OddAfter         = 28,    ///< Non Image data are received at Odd frame and After Image
	NX_MIPI_INT_CSI_FrameStart_CH3   = 27,    ///< FS packet is received, CH3
	NX_MIPI_INT_CSI_FrameStart_CH2   = 26,    ///< FS packet is received, CH2
	NX_MIPI_INT_CSI_FrameStart_CH1   = 25,    ///< FS packet is received, CH1
	NX_MIPI_INT_CSI_FrameStart_CH0   = 24,    ///< FS packet is received, CH0
	NX_MIPI_INT_CSI_FrameEnd_CH3     = 23,    ///< FE packet is received, CH3
	NX_MIPI_INT_CSI_FrameEnd_CH2     = 22,    ///< FE packet is received, CH2
	NX_MIPI_INT_CSI_FrameEnd_CH1     = 21,    ///< FE packet is received, CH1
	NX_MIPI_INT_CSI_FrameEnd_CH0     = 20,    ///< FE packet is received, CH0
	NX_MIPI_INT_CSI_ERR_SOT_HS_CH3   = 19,    ///< Start of transmission error, CH3
	NX_MIPI_INT_CSI_ERR_SOT_HS_CH2   = 18,    ///< Start of transmission error, CH2
	NX_MIPI_INT_CSI_ERR_SOT_HS_CH1   = 17,    ///< Start of transmission error, CH1
	NX_MIPI_INT_CSI_ERR_SOT_HS_CH0   = 16,    ///< Start of transmission error, CH0
	NX_MIPI_INT_CSI_ERR_LOST_FS_CH3  = 15,    ///< Indication of lost of Frame Start packet, CH3
	NX_MIPI_INT_CSI_ERR_LOST_FS_CH2  = 14,    ///< Indication of lost of Frame Start packet, CH2
	NX_MIPI_INT_CSI_ERR_LOST_FS_CH1  = 13,    ///< Indication of lost of Frame Start packet, CH1
	NX_MIPI_INT_CSI_ERR_LOST_FS_CH0  = 12,    ///< Indication of lost of Frame Start packet, CH0
	NX_MIPI_INT_CSI_ERR_LOST_FE_CH3  = 11,    ///< Indication of lost of Frame End packet, CH3
	NX_MIPI_INT_CSI_ERR_LOST_FE_CH2  = 10,    ///< Indication of lost of Frame End packet, CH2
	NX_MIPI_INT_CSI_ERR_LOST_FE_CH1  =  9,    ///< Indication of lost of Frame End packet, CH1
	NX_MIPI_INT_CSI_ERR_LOST_FE_CH0  =  8,    ///< Indication of lost of Frame End packet, CH0
	NX_MIPI_INT_CSI_ERR_OVER_CH3     =  7,    ///< Overflow is caused in image FIFO. CH3
	NX_MIPI_INT_CSI_ERR_OVER_CH2     =  6,    ///< Overflow is caused in image FIFO. CH2
	NX_MIPI_INT_CSI_ERR_OVER_CH1     =  5,    ///< Overflow is caused in image FIFO. CH1
	NX_MIPI_INT_CSI_ERR_OVER_CH0     =  4,    ///< Overflow is caused in image FIFO. CH0
	//NX_MIPI_INT_CSI_Reserved         =  3,    ///< Reserved, read as zero, do not modify
	NX_MIPI_INT_CSI_ERR_ECC          =  2,    ///< ECC error
	NX_MIPI_INT_CSI_ERR_CRC          =  1,    ///< CRC error
	NX_MIPI_INT_CSI_ERR_ID           =  0,    ///< Unknown ID error

	NX_MIPI_INT_DSI_PllStable        = 32+31, ///< Indicates that D-phy PLL is stable. 0
	NX_MIPI_INT_DSI_SwRstRelease     = 32+30, ///< Releases the software reset. 0
	NX_MIPI_INT_DSI_SFRPLFifoEmpty   = 32+29, ///< Specifies the SFR payload FIFO empty. 0
	NX_MIPI_INT_DSI_SFRPHFifoEmpty   = 32+28, ///< Specifies the SFR Packet Header FIFO emtpy 0
	NX_MIPI_INT_DSI_SyncOverride     = 32+27, ///< Indicates that other DSI command transfer have overridden sync timing. 0
	//NX_MIPI_INT_DSI_Reserved         = 32+26,
	NX_MIPI_INT_DSI_BusTurnOver      = 32+25, ///< Indicates when bus grant turns over from DSI slave to DSI master. 0
	NX_MIPI_INT_DSI_FrameDone        = 32+24, ///< Indicates when MIPI DSIM transfers the whole image frame. Note: If Hsync is not received during two line times, internal timer is timed out and this bit is flagged. 0
	//NX_MIPI_INT_DSI_Reserved         = 32+23,
	//NX_MIPI_INT_DSI_Reserved         = 32+22,
	NX_MIPI_INT_DSI_LpdrTout         = 32+21, ///< Specifies the LP Rx timeout. See time out register (0x10). 0
	NX_MIPI_INT_DSI_TaTout           = 32+20, ///< Turns around Acknowledge Timeout. See time out register (0x10). 0
	//NX_MIPI_INT_DSI_Reserved         = 32+19,
	NX_MIPI_INT_DSI_RxDatDone        = 32+18, ///< Completes receiving data. 0
	NX_MIPI_INT_DSI_RxTE             = 32+17, ///< Receives TE Rx trigger. 0
	NX_MIPI_INT_DSI_RxAck            = 32+16, ///< Receives ACK Rx trigger. 0
	NX_MIPI_INT_DSI_ErrRxECC         = 32+15, ///< Specifies the ECC multi bit error in LPDR. 0
	NX_MIPI_INT_DSI_ErrRxCRC         = 32+14, ///< Specifies the CRC error in LPDR. 0
	NX_MIPI_INT_DSI_ErrEsc3          = 32+13, ///< Specifies the escape mode entry error lane 3. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrEsc2          = 32+12, ///< Specifies the escape mode entry error lane 2. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrEsc1          = 32+11, ///< Specifies the escape mode entry error lane 1. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrEsc0          = 32+10, ///< Specifies the escape mode entry error lane 0. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrSync3         = 32+ 9, ///< Specifies the LPDT sync error lane3. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrSync2         = 32+ 8, ///< Specifies the LPDT Sync Error lane2. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrSync1         = 32+ 7, ///< Specifies the LPDT Sync Error lane1. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrSync0         = 32+ 6, ///< Specifies the LPDT Sync Error lane0. For moreinformation, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrControl3      = 32+ 5, ///< Controls Error lane3. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrControl2      = 32+ 4, ///< Controls Error lane2. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrControl1      = 32+ 3, ///< Controls Error lane1. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrControl0      = 32+ 2, ///< Controls Error lane0. For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrContentLP0    = 32+ 1, ///< Specifies the LP0 Contention Error (only lane0, because BTA occurs at lane0 only). For more information, refer to standard D-PHY specification. 0
	NX_MIPI_INT_DSI_ErrContentLP1    = 32+ 0, ///< Specifies the LP1 Contention Error (only lane0, because BTA occurs at lane0 only). For more information, refer to standard D-PHY specification. 0
} NX_MIPI_INT;
U32   NX_MIPI_GetInterruptNumber ( U32 ModuleIndex );
void  NX_MIPI_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL NX_MIPI_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL NX_MIPI_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_MIPI_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_MIPI_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL NX_MIPI_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL NX_MIPI_GetInterruptPendingAll( U32 ModuleIndex );
void  NX_MIPI_ClearInterruptPendingAll( U32 ModuleIndex );
S32   NX_MIPI_GetInterruptPendingNumber( U32 ModuleIndex );
//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_MIPI_H__
