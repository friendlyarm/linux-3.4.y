//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: PHY for DDR SDRAM
//	File		: nx_ddrphy.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.08.20 Hans create
//------------------------------------------------------------------------------
#ifndef __NX_DDRPHY_H__
#define __NX_DDRPHY_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	DDRPHY DDR PHY controller
//------------------------------------------------------------------------------
//@{

	struct	NX_DDRPHY_RegisterSet
	{
		volatile U32 PHY_CON[6];				///< 0x000 ~ 0x014 : PHY Control
		volatile U32 LP_CON;					///< 0x018 : Low Power Control
		volatile U32 RODT_CON;					///< 0x01C : Read ODT Control
		volatile U32 OFFSETR_CON[3];			///< 0x020 ~ 0x028 : READ Code Control
		volatile U32 _Reserved0;				///< 0x02C
		volatile U32 OFFSETW_CON[3];			///< 0x030 ~ 0x038 : Write Code Control
		volatile U32 _Reserved1;				///< 0x03C
		volatile U32 OFFSETC_CON[3];			///< 0x040 ~ 0x048 : GATE Code Control
		volatile U32 SHIFTC_CON;				///< 0x04C : GATE Code Shift Control
		volatile U32 OFFSETD_CON;				///< 0x050 : CMD Code Control
		volatile U32 _Reserved2;				///< 0x054
		volatile U32 LP_DDR_CON[5];				///< 0x058 ~ 0x068 : LPDDR Control
		volatile U32 WR_LVL_CON[4];				///< 0x06C ~ 0x078 : Write Leveling Control
		volatile U32 CA_DSKEW_CON[4];			///< 0x07C ~ 0x088 : CA Deskew Control
		volatile U32 _Reserved3[2];				///< 0x08C ~ 0x090
		volatile U32 CA_DSKEW_CON4;				///< 0x094 : CA Deskew Control
		volatile U32 _Reserved4;				///< 0x098
		volatile U32 DRVDS_CON[2];				///< 0x09C ~ 0x0A0 : Driver Strength Control
		volatile U32 _Reserved5[3];				///< 0x0A4 ~ 0x0AC
		volatile U32 MDLL_CON[2];				///< 0x0B0 ~ 0x0B4 : MDLL Control
		volatile U32 _Reserved6[2];				///< 0x0B8 ~ 0x0BC
		volatile U32 ZQ_CON;					///< 0x0C0 : ZQ Control
		volatile U32 ZQ_STATUS;					///< 0x0C4 : ZQ Status
		volatile U32 ZQ_DIVIDER;				///< 0x0C8 : ZQ Divider Control
		volatile U32 ZQ_TIMER;					///< 0x0CC : ZQ Timer Control
		volatile U32 T_RDDATA_CON[3];			///< 0x0D0 ~ 0x0D8 : Read Data Enable Status
		volatile U32 CAL_WL_STAT;				///< 0x0DC : WL Calibration Fail Status
		volatile U32 CAL_FAIL_STAT[4];			///< 0x0E0 ~ 0x0EC : Calibration Fail Status
		volatile U32 CAL_GT_VWMC[3];			///< 0x0F0 ~ 0x0F8 : Calibration Gate Training Centering Code
		volatile U32 CAL_GT_CYC;				///< 0x0FC : Calibration Gate Training Cycle
		volatile U32 CAL_RD_VWMC[3];			///< 0x100 ~ 0x108 : Calibration Read Center Code
		volatile U32 _Reserved7;				///< 0x10C
		volatile U32 CAL_RD_VWML[3];			///< 0x110 ~ 0x118 : Calibration Read Left Code
		volatile U32 _Reserved8;				///< 0x11C
		volatile U32 CAL_RD_VWMR[3];			///< 0x120 ~ 0x128 : Calibration Read Right Code
		volatile U32 _Reserved9;				///< 0x12C
		volatile U32 CAL_WR_VWMC[3];			///< 0x130 ~ 0x138 : Calibration Write Center Code
		volatile U32 _Reserved10;				///< 0x13C
		volatile U32 CAL_WR_VWML[3];			///< 0x140 ~ 0x148 : Calibration Write Left Code
		volatile U32 _Reserved11;				///< 0x14C
		volatile U32 CAL_WR_VWMR[3];			///< 0x150 ~ 0x158 : Calibration Write Right Code
		volatile U32 _Reserved12;				///< 0x15C
		volatile U32 CAL_DM_VWMC[3];			///< 0x160 ~ 0x168 : Calibration DM Center Code
		volatile U32 _Reserved13;				///< 0x16C
		volatile U32 CAL_DM_VWML[3];			///< 0x170 ~ 0x178 : Calibration DM Left Code
		volatile U32 _Reserved14;				///< 0x17C
		volatile U32 CAL_DM_VWMR[3];			///< 0x180 ~ 0x188 : Calibration DM Right Code
		volatile U32 _Reserved15;				///< 0x18C
		volatile U32 RD_DESKEW_CON[24];			///< 0x190 ~ 0x1EC : READ DeSkew Control
		volatile U32 WR_DESKEW_CON[24];			///< 0x1F0 ~ 0x24C : WRITE DeSkew Control
		volatile U32 DM_DESKEW_CON[3];			///< 0x250 ~ 0x258 : DM DeSkew Control
		volatile U32 VWMC_STAT[24];				///< 0x25C ~ 0x2B8 : VWMC Status
		volatile U32 DM_VWMC_STAT[3];			///< 0x2BC ~ 0x2C4 : DM VWMC
		volatile U32 VWML_STAT[24];				///< 0x2C8 ~ 0x324 : VWML Status
		volatile U32 DM_VWML_STAT[3];			///< 0x328 ~ 0x330 : DM VWML
		volatile U32 VWMR_STAT[24];				///< 0x334 ~ 0x390 : VWMR Status
		volatile U32 DM_VWMR_STAT[3];			///< 0x394 ~ 0x39C : DM VWMR
		volatile U32 DQ_IO_RDATA[3];			///< 0x3A0 ~ 0x3A8 : DQ I/O Read Data Status
		volatile U32 VERSION_INFO;				///< 0x3AC : Version Information
	};

enum
{
	DDRMODE_DDR2	= 0,
	DDRMODE_LPDDR1	= 0,
	DDRMODE_DDR3	= 1,
	DDRMODE_LPDDR2	= 2,
	DDRMODE_LPDDR3	= 3
};

enum
{
	SHIFT_DEGREE_0		= 0,	// 0
	SHIFT_DEGREE_365	= 1,	// T
	SHIFT_DEGREE_180	= 2,	// T/2
	SHIFT_DEGREE_90		= 3,	// T/4
	SHIFT_DEGREE_45		= 4,	// T/8
	SHIFT_DEGREE_22_5	= 5		// T/16
};

enum
{
	PHY_DRV_STRENGTH_240OHM	= 0,	// Div1, 240ohm
	PHY_DRV_STRENGTH_120OHM	= 1,	// Div2, 120ohm
	PHY_DRV_STRENGTH_80OHM	= 2,	// Div3, 80ohm
	PHY_DRV_STRENGTH_60OHM	= 3,	// Div4, 60ohm
	PHY_DRV_STRENGTH_48OHM	= 4,	// Div5, 48ohm
	PHY_DRV_STRENGTH_40OHM	= 5,	// Div6, 40ohm
	PHY_DRV_STRENGTH_34OHM	= 6,	// Div7, 34ohm
	PHY_DRV_STRENGTH_30OHM	= 7		// Div8, 30ohm
};

enum
{
	PHY_ODT_120OHM	= 1,	// 120ohm receiver termination
	PHY_ODT_60OHM	= 2,	// 60ohm receiver termination
	PHY_ODT_40OHM	= 3,	// 40ohm receiver termination
	PHY_ODT_30OHM	= 4		// 30ohm receiver termination
};

enum
{
	CALIBRATION_FORCE	= 0,
	CALIBRATION_LONG	= 1,
	CALIBRATION_SHORT	= 2
};



//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_DDRPHY_Initialize( void );
U32		NX_DDRPHY_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_DDRPHY_GetPhysicalAddress( void );
U32		NX_DDRPHY_GetSizeOfRegisterSet( void );
void	NX_DDRPHY_SetBaseAddress( void* BaseAddress );
void*	NX_DDRPHY_GetBaseAddress( void );
CBOOL	NX_DDRPHY_OpenModule( void );
CBOOL	NX_DDRPHY_CloseModule( void );
CBOOL	NX_DDRPHY_CheckBusy( void );
CBOOL	NX_DDRPHY_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
// there is no interrupt interface in DDR PHY controller
//@}

//------------------------------------------------------------------------------
/// @name	Clock Management
//@{
// there is no clock interface in DDR PHY controller
//@}

//------------------------------------------------------------------------------
/// @name	Power Management
//@{
void	NX_DDRPHY_SetLowPowerModeEnable ( CBOOL Enable );
CBOOL	NX_DDRPHY_GetLOwPowerModeEnable ( void );
void	NX_DDRPHY_WakeupFromLowPowerMode ( void );
//@}

//------------------------------------------------------------------------------
/// @name	Reset Management
//@{
// will find reset feature
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_DDRPHY_H__
