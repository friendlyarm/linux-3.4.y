//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: Clock & Power Manager
//	File		: nx_clkpwr.h
//	Description	:
//	Author		: Firmware
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_CLKPWR_H__
#define __NX_CLKPWR_H__

#include "../base/nx_prototype.h"

//------------------------
// implentation of ALIVE
//------------------------
#define NUMBER_OF_ALIVE_MODULE 1
#define PHY_BASEADDR_ALIVE_MODULE     (PHY_BASEADDR_CLKPWR_MODULE + 0x800)  // 2048

#define NUMBER_OF_RTC_MODULE 1
#define PHY_BASEADDR_RTC_MODULE     (PHY_BASEADDR_CLKPWR_MODULE + 0xc00) // 2048+1024

#define NUMBER_OF_PMU_MODULE 1
#define PHY_BASEADDR_PMU_MODULE     (PHY_BASEADDR_CLKPWR_MODULE + 0xc00 + 0x100)  // 2048+256


#define INTNUM_OF_RTC_MODULE          INTNUM_OF_CLKPWR_MODULE_RTCIRQ
#define INTNUM_OF_ALIVE_MODULE        INTNUM_OF_CLKPWR_MODULE_ALIVEIRQ
#define INTNUM_OF_CLKPWR_MODULE       INTNUM_OF_CLKPWR_MODULE_INTREQPWR

#define NUMBER_OF_PMU_POWER_DOMAIN 2


//#define PADINDEX_OF_DUALDISPLAY_o_nCS                 PADINDEX_OF_DISPLAYTOP_DualDisplay_PADPrimVCLK
//#define PADINDEX_OF_DUALDISPLAY_o_nRD                 PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADnHSync
//#define PADINDEX_OF_DUALDISPLAY_o_RS                  PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADnVSync
//#define PADINDEX_OF_DUALDISPLAY_o_nWR                 PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADDE
//#define PADINDEX_OF_DUALDISPLAY_PADPrimVCLK           PADINDEX_OF_DISPLAYTOP_DualDisplay_PADPrimVCLK
//#define PADINDEX_OF_DUALDISPLAY_o_PrimPADnHSync       PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADnHSync
//#define PADINDEX_OF_DUALDISPLAY_o_PrimPADnVSync       PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADnVSync
//#define PADINDEX_OF_DUALDISPLAY_o_PrimPADDE           PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADDE
//


#define PLL_PMS_400MHZ             6, 400, 2
#define PLL_PMS_333MHZ             6, 333, 2

#define PLL_PMS_1600MHZ           		 6, 400, 0
#define PLL_PMS_1000MHZ           		 6, 500, 0
#define PLL_PMS_800MHZ            		 6, 400, 1
#define PLL_PMS_550MHZ            		 6, 550, 2    // 528.00 Mhz
#define PLL_PMS_528MHZ            		 6, 528, 2    // 528.00 Mhz
#define PLL_PMS_147P456MHZ        		 6, 590, 4    // 147.4615Mhz
#define PLL_PMS_200MHZ            		 6, 400, 3
#define PLL_PMS_96MHZ             		 6, 384, 4
#define PLL_PMS_125MHZ            		 6, 500, 4

#define PLL_PMS_1600MHZ_DITHER           3, 200, 0
#define PLL_PMS_800MHZ_DITHER            3, 200, 1
#define PLL_PMS_550MHZ_DITHER            3, 275, 2    // 528.00 Mhz
#define PLL_PMS_528MHZ_DITHER            3, 264, 2    // 528.00 Mhz
#define PLL_PMS_147P456MHZ_DITHER        3, 147, 3    // 147.4615Mhz
#define PLL_PMS_200MHZ_DITHER            3, 200, 3
#define PLL_PMS_96MHZ_DITHER             3, 192, 4
#define PLL_PMS_125MHZ_DITHER            3, 250, 4

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	CLKPWR Clock & Power Manager
//------------------------------------------------------------------------------
//@{


	/// @brief	Clock & Power Manager Module's Register List
	// assign	PSEL[0]	= i_PSEL &amp; ( i_PADDR[10:9] == 0 ); //	CLKCTR :0
	// assign	PSEL[1]	= i_PSEL &amp; ( i_PADDR[10:9] == 1 ); //	ALIVE  : 512
	struct	NX_CLKPWR_RegisterSet
	{
		volatile U32 CLKMODEREG0;				///< 0x000 : Clock Mode Register 0
		volatile U32 __Reserved0;				///< 0x004
		volatile U32 PLLSETREG[4];				///< 0x008 ~ 0x014 : PLL Setting Register
		volatile U32 __Reserved1[2];			///< 0x018 ~ 0x01C
		volatile U32 DVOREG[5];					///< 0x020 ~ 0x030 : Divider Setting Register
		volatile U32 __Reserved2[5];			///< 0x034 ~ 0x044
		volatile U32 PLLSETREG_SSCG[6];			///< 0x048 ~ 0x05C
		volatile U32 __reserved3[8];			///< 0x060 ~ 0x07C
		volatile U32 __Reserved4[(512-128)/4];	// padding (0x200-..)/4
		volatile U32 GPIOWAKEUPRISEENB;			///< 0x200 : GPIO Rising Edge Detect Enable Register
		volatile U32 GPIOWAKEUPFALLENB;			///< 0x204 : GPIO Falling Edge Detect Enable Register
		volatile U32 GPIORSTENB;				///< 0x208 : GPIO Reset Enable Register
		volatile U32 GPIOWAKEUPENB;				///< 0x20C : GPIO Wakeup Source Enable
		volatile U32 GPIOINTENB;				///< 0x210 : Interrupt Enable Register
		volatile U32 GPIOINTPEND;				///< 0x214 : Interrupt Pend Register
		volatile U32 RESETSTATUS;				///< 0x218 : Reset Status Register
		volatile U32 INTENABLE;					///< 0x21C : Interrupt Enable Register
		volatile U32 INTPEND;					///< 0x220 : Interrupt Pend Register
		volatile U32 PWRCONT;					///< 0x224 : Power Control Register
		volatile U32 PWRMODE;					///< 0x228 : Power Mode Register
		volatile U32 __Reserved5;				///< 0x22C : Reserved Region
		volatile U32 SCRATCH[3];				///< 0x230 ~ 0x238	: Scratch Register
		volatile U32 SYSRSTCONFIG;				///< 0x23C : System Reset Configuration Register.
		volatile U8  __Reserved6[0x100-0x80];	///< 0x80 ~ 0xFC	: Reserved Region
		volatile U32 PADSTRENGTHGPIO[5][2];		///< 0x100, 0x104 : GPIOA Pad Strength Register
												///< 0x108, 0x10C : GPIOB Pad Strength Register
												///< 0x110, 0x114 : GPIOC Pad Strength Register
												///< 0x118, 0x11C : GPIOD Pad Strength Register
												///< 0x120, 0x124 : GPIOE Pad Strength Register
		volatile U32 __Reserved7[2];			///< 0x128 ~ 0x12C: Reserved Region
		volatile U32 PADSTRENGTHBUS;			///< 0x130		: Bus Pad Strength Register
	};

	///@brief	CLKPWR Interrupts for interrupt interfaces
	enum
	{
		NX_CLKPWR_INT_ALIVEGPIO0		= 0,	///< ALIVE GPIO 0 interrupt
		NX_CLKPWR_INT_ALIVEGPIO1		= 1,	///< ALIVE GPIO 1 interrupt
		NX_CLKPWR_INT_ALIVEGPIO2		= 2,	///< ALIVE GPIO 2 interrupt
		NX_CLKPWR_INT_ALIVEGPIO3		= 3,	///< ALIVE GPIO 3 interrupt
		NX_CLKPWR_INT_ALIVEGPIO4		= 4,	///< ALIVE GPIO 4 interrupt
		NX_CLKPWR_INT_ALIVEGPIO5		= 5,	///< ALIVE GPIO 5 interrupt
		NX_CLKPWR_INT_VDDTOGGLE			= 6,	///< VDD Power Toggle interrupt
		NX_CLKPWR_INT_UART0				= 7,	///< UART RX 0 interrupt
		NX_CLKPWR_INT_UART1				= 8,	///< UART RX 1 interrupt
		NX_CLKPWR_INT_UART2				= 9,	///< UART RX 2 interrupt
		NX_CLKPWR_INT_UART3				=10,	///< UART RX 3 interrupt
		NX_CLKPWR_INT_UART4				=11,	///< UART RX 4 interrupt
		NX_CLKPWR_INT_UART5				=12,	///< UART RX 5 interrupt
		NX_CLKPWR_INT_USB0				=13,	///< USB  0 interrupt
		NX_CLKPWR_INT_USB1				=14,	///< USB  1 interrupt
		NX_CLKPWR_INT_RTC				=15,	///< RTC interrupt
		NX_CLKPWR_INT_BATF				=16,	///< Battery fault event interrupt
		NX_CLKPWR_INT_MAX				=17
	};

	/// @brief	Reset status
	typedef enum
	{
		NX_CLKPWR_RESETSTATUS_POWERON		= 1 << 0,	///< Power on reset
		NX_CLKPWR_RESETSTATUS_GPIO			= 1 << 1,	///< GPIO reset
		NX_CLKPWR_RESETSTATUS_WATCHDOG		= 1 << 2,	///< Watchdog Reset
		NX_CLKPWR_RESETSTATUS_SOFTWARE		= 1 << 3,	///< Software reset
		NX_CLKPWR_RESETSTATUS_DSLEEPRESET	= 1 << 4,	///< Deep Sleep reset
		NX_CLKPWR_RESETSTATUS_SLEEPRESET	= 1 << 5,	///< Sleep reset

	}	NX_CLKPWR_RESETSTATUS;

	/// @brief	Power mode
	typedef enum
	{
		NX_CLKPWR_POWERMODE_STOP		= 1 << 5,	///< Stop mode
		NX_CLKPWR_POWERMODE_IDLE		= 1 << 4	///< Idle mode

	}	NX_CLKPWR_POWERMODE;

	/// @brief	Bus type
	typedef enum
	{
		NX_CLKPWR_BUSPAD_STATIC_CNTL	= 22UL,		///< Static Control Pad
		NX_CLKPWR_BUSPAD_STATIC_ADDR	= 20UL,		///< Static Address Pad
		NX_CLKPWR_BUSPAD_STATIC_DATA	= 18UL,		///< Static Data Pad
		NX_CLKPWR_BUSPAD_VSYNC			= 6UL,		///< Vertical Sync Pad
		NX_CLKPWR_BUSPAD_HSYNC			= 4UL,		///< Horizontal Sync Pad
		NX_CLKPWR_BUSPAD_DE				= 2UL,		///< Data Enable Pad

	}	NX_CLKPWR_BUSPAD;

	/// @brief	pll number
	typedef enum
	{
		NX_CLKPWR_PLL_0	= 0UL,		///< pll number 0
		NX_CLKPWR_PLL_1	= 1UL,		///< pll number 1
		NX_CLKPWR_PLL_2	= 2UL,		///< pll number 2
		NX_CLKPWR_PLL_3 = 3UL,		///< pll number 3
	}	NX_CLKPWR_PLL;

	/// @brief	clock divider select
	typedef enum
	{
		NX_CLKPWR_CLOCK_FCLK		= 0UL,		///< clock divider FCLK
		NX_CLKPWR_CLOCK_BCLK		= 1UL,		///< clock divider BCLK
		NX_CLKPWR_CLOCK_MCLK		= 2UL,		///< clock divider MCLK
		NX_CLKPWR_CLOCK_GR3DBCLK	= 3UL,		///< clock divider GR3DBCLK
		NX_CLKPWR_CLOCK_MPEGBCLK	= 4UL,		///< clock divider MPEGBCLK
	}	NX_CLKPWR_CLOCK;


	/// @brief	Glitchless mux select
	typedef enum
	{
		NX_CLKPWR_GMUX_XTAL	= 0UL,		///< PLL's out-mux to Xrystal
		NX_CLKPWR_GMUX_PLL	= 1UL,		///< PLL's out-mux to PLL
	}	NX_CLKPWR_GMUX;

	/// @brief	power switch number
	typedef enum
	{
		NX_CLKPWR_POWER_SWITCH_PRECHARGE	= 0UL,	///< pre discharge power switch
		NX_CLKPWR_POWER_SWITCH_ALL			= 1UL,	///< all power switch
	}	NX_CLKPWR_POWER_SWITCH;

typedef enum
	{
		NX_CLKPWR_POWER_DOMAIN_0	= 0UL,	///< power domain 0
		NX_CLKPWR_POWER_DOMAIN_1	= 1UL,	///< power domain 1
	}	NX_CLKPWR_POWER_DOMAIN;


	typedef enum
	{
		NX_CLKPWR_PLL_POWER_UP  	= 0UL,	///< power domain 0
		NX_CLKPWR_PLL_POWER_DOWN	= 1UL,	///< power domain 1
	}	NX_CLKPWR_PLL_POWER;




//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_CLKPWR_Initialize( void );
U32		NX_CLKPWR_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_CLKPWR_GetPhysicalAddress( void );
U32		NX_CLKPWR_GetSizeOfRegisterSet( void );
void	NX_CLKPWR_SetBaseAddress( void* BaseAddress );
void*	NX_CLKPWR_GetBaseAddress( void );
CBOOL	NX_CLKPWR_OpenModule( void );
CBOOL	NX_CLKPWR_CloseModule( void );
CBOOL	NX_CLKPWR_CheckBusy( void );
CBOOL	NX_CLKPWR_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
S32		NX_CLKPWR_GetInterruptNumber( void );

void	NX_CLKPWR_SetInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_CLKPWR_GetInterruptEnable( S32 IntNum );
CBOOL	NX_CLKPWR_GetInterruptPending( S32 IntNum );
void	NX_CLKPWR_ClearInterruptPending( S32 IntNum );

void	NX_CLKPWR_SetInterruptEnableAll( CBOOL Enable );
CBOOL	NX_CLKPWR_GetInterruptEnableAll( void );
CBOOL	NX_CLKPWR_GetInterruptPendingAll( void );
void	NX_CLKPWR_ClearInterruptPendingAll( void );

void	NX_CLKPWR_SetInterruptEnable32( U32 EnableFlag );
U32		NX_CLKPWR_GetInterruptEnable32( void );
U32		NX_CLKPWR_GetInterruptPending32( void );
void	NX_CLKPWR_ClearInterruptPending32( U32 PendingFlag );

S32		NX_CLKPWR_GetInterruptPendingNumber( void );	// -1 if None
//@}

//------------------------------------------------------------------------------
/// @name	Clock Management
//@{
void	NX_CLKPWR_SetPLLPMS    ( U32 pllnumber, U32 PDIV, U32 MDIV, U32 SDIV );
U32		NX_CLKPWR_GetPLLFreq	(U32 pllnumber, U32 XTalFreqKHz);
void	NX_CLKPWR_SetPLLDither ( U32 pllnumber, S32 K, U32 MFR, U32 MRR, U32 SEL_PF, CBOOL SSCG_EN  );
void	NX_CLKPWR_SetPLLPowerOn ( CBOOL On );
void	NX_CLKPWR_DoPLLChange( void );
CBOOL	NX_CLKPWR_IsPLLStable( void );
void	NX_CLKPWR_SetClockCPU( U32 ClkSrc, U32 CoreDivider, U32 BusDivider );
//void	NX_CLKPWR_SetClockMCLK( U32 ClkSrc, U32 MCLKDivider, U32 BCLKDivider, U32 PCLKDivider );
void	NX_CLKPWR_SetClockDivider2	( NX_CLKPWR_CLOCK clock_number, U32 ClkSrc, U32 Divider1, U32 Divider2 );
void	NX_CLKPWR_SetClockDivider3	( NX_CLKPWR_CLOCK clock_number, U32 ClkSrc, U32 Divider1, U32 Divider2, U32 Divider3 );
void	NX_CLKPWR_SetClockDivider4	( NX_CLKPWR_CLOCK clock_number, U32 ClkSrc, U32 Divider1, U32 Divider2, U32 Divider3, U32 Divider4 );
//void	NX_CLKPWR_SetCPUBUSSyncMode(CBOOL Enable);
//@}

//------------------------------------------------------------------------------
/// @name	Wakeup Management
//@{
void	NX_CLKPWR_SetRTCWakeUpEnable ( CBOOL Enable );
CBOOL	NX_CLKPWR_GetRTCWakeUpEnable ( void );
void	NX_CLKPWR_SetALIVEGPIOWakeupEnable( U32 dwBitNumber, CBOOL bEnable );
void	NX_CLKPWR_SetALIVEGPIOWakeupEnableAll(  CBOOL bEnable );
CBOOL	NX_CLKPWR_GetALIVEGPIOWakeupEnable( U32 dwBitNumber );
void	NX_CLKPWR_SetALIVEGPIOWakeUpRiseEdgeDetectEnable( U32 dwBitNumber, CBOOL bEnable );
CBOOL	NX_CLKPWR_GetALIVEGPIOWakeUpRiseEdgeDetectEnable( U32 dwBitNumber );
void	NX_CLKPWR_SetALIVEGPIOWakeUpFallEdgeDetectEnable( U32 dwBitNumber, CBOOL bEnable );
CBOOL	NX_CLKPWR_GetALIVEGPIOWakeUpFallEdgeDetectEnable( U32 dwBitNumber );
//@}

//------------------------------------------------------------------------------
/// @name	Reset Management
//@{
void	NX_CLKPWR_SetSoftwareResetEnable( CBOOL bEnable );
CBOOL	NX_CLKPWR_GetSoftwareResetEnable( void );
void	NX_CLKPWR_DoSoftwareReset( void );
void	NX_CLKPWR_SetALIVEGPIOResetEnable( U32 dwBitNumber, CBOOL bEnable );
CBOOL	NX_CLKPWR_GetALIVEGPIOResetEnable( U32 dwBitNumber );
void	NX_CLKPWR_SetALIVEGPIOResetEnableAll( CBOOL bEnable );
//@}

//------------------------------------------------------------------------------
/// @name	Check Last Reset Status
//@{
NX_CLKPWR_RESETSTATUS	NX_CLKPWR_GetResetStatus( void );
//@}

//------------------------------------------------------------------------------
/// @name	Power Management
//@{

void				NX_CLKPWR_SetImmediateSleepEnable( CBOOL enable );
void				NX_CLKPWR_GoStopMode( void );
void				NX_CLKPWR_GoIdleMode( void );
NX_CLKPWR_POWERMODE	NX_CLKPWR_GetLastPowerMode ( void );
//@}

//------------------------------------------------------------------------------
/// @name	Scratch Pad
//@{
void	NX_CLKPWR_SetScratchPad( U32 dwIndex, U32 dwValue );
U32		NX_CLKPWR_GetScratchPad( U32 dwIndex );
//@}

//------------------------------------------------------------------------------
/// @name	System Reset Configuration
//@{
U32		NX_CLKPWR_GetSystemResetConfiguration( void );
//@}

//------------------------------------------------------------------------------
/// @name	PAD Strength Management
//@{
void	NX_CLKPWR_SetGPIOPadStrength( U32 Group, U32 BitNumber, U32 mA );
U32		NX_CLKPWR_GetGPIOPadStrength( U32 Group, U32 BitNumber );
void	NX_CLKPWR_SetBusPadStrength( NX_CLKPWR_BUSPAD Bus, U32 mA );
U32		NX_CLKPWR_GetBusPadStrength( NX_CLKPWR_BUSPAD Bus );
void	NX_CLKPWR_SetPllOutGMux(U32 pll_number, NX_CLKPWR_GMUX gmux);
void	NX_CLKPWR_UpdatePllSetReg(U32 pll_number);
CBOOL	NX_CLKPWR_IsPLLStableUpdate( void );
void NX_CLKPWR_SetPLLPower( U32 pllnumber, NX_CLKPWR_PLL_POWER powermode );
//@}

//------------------------------------------------------------------------------
/// @name Power Domain Switch
// @{
void NX_CLKPWR_SetPowerSwitchEnable ( NX_CLKPWR_POWER_DOMAIN domain, CBOOL on );
void NX_CLKPWR_SetPowerSwitchDelay( NX_CLKPWR_POWER_DOMAIN domain, NX_CLKPWR_POWER_SWITCH power_switch, U32 delay );
void NX_CLKPWR_SetPowerSwitchOverrideEnable( CBOOL on );
void NX_CLKPWR_SetPowerSwitchOverrideIsolation( NX_CLKPWR_POWER_DOMAIN domain, CBOOL on );
void NX_CLKPWR_SetPowerSwitchOverrideSwitch( NX_CLKPWR_POWER_DOMAIN domain, NX_CLKPWR_POWER_SWITCH power_switch, CBOOL on );
// @}

//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_CLKPWR_H__
