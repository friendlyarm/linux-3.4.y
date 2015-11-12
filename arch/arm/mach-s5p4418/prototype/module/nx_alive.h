//------------------------------------------------------------------------------
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: Alive
//	File		: nx_alive.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_ALIVE_H__
#define	__NX_ALIVE_H__


//#include "../base/nx_prototype.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NX_ALIVE_NUMBER_OF_GPIO 6

//------------------------------------------------------------------------------
/// @defgroup	ALIVE
//------------------------------------------------------------------------------
//@{

	/// @brief ALIVE Module's Register List
	struct NX_ALIVE_RegisterSet
	{
		volatile U32 ALIVEPWRGATEREG;						///< 0x00 : Alive Power Gating Register
		volatile U32 ALIVEGPIOASYNCDETECTMODERSTREG0;		///< 0x04 : Alive GPIO ASync Detect Mode Reset Register0
		volatile U32 ALIVEGPIOASYNCDETECTMODESETREG0;		///< 0x08 : Alive GPIO ASync Detect Mode Set Register0
		volatile U32 ALIVEGPIOLOWASYNCDETECTMODEREADREG;	///< 0x0C : Alive GPIO Low ASync Detect Mode Read Register

		volatile U32 ALIVEGPIOASYNCDETECTMODERSTREG1;		///< 0x10 : Alive GPIO ASync Detect Mode Reset Register1
		volatile U32 ALIVEGPIOASYNCDETECTMODESETREG1;		///< 0x14 : Alive GPIO ASync Detect Mode Set Register1
		volatile U32 ALIVEGPIOHIGHASYNCDETECTMODEREADREG;	///< 0x18 : Alive GPIO High ASync Detect Mode Read Register

		volatile U32 ALIVEGPIODETECTMODERSTREG0;			///< 0x1C : Alive GPIO Detect Mode Reset Register0
		volatile U32 ALIVEGPIODETECTMODESETREG0;			///< 0x20 : Alive GPIO Detect Mode Reset Register0
		volatile U32 ALIVEGPIOFALLDETECTMODEREADREG;		///< 0x24 : Alive GPIO Falling Edge Detect Mode Read Register

		volatile U32 ALIVEGPIODETECTMODERSTREG1;			///< 0x28 : Alive GPIO Detect Mode Reset Register1
		volatile U32 ALIVEGPIODETECTMODESETREG1;			///< 0x2C : Alive GPIO Detect Mode Reset Register1
		volatile U32 ALIVEGPIORISEDETECTMODEREADREG;		///< 0x30 : Alive GPIO Rising Edge Detect Mode Read Register

		volatile U32 ALIVEGPIODETECTMODERSTREG2;			///< 0x34 : Alive GPIO Detect Mode Reset Register2
		volatile U32 ALIVEGPIODETECTMODESETREG2;			///< 0x38 : Alive GPIO Detect Mode Reset Register2
		volatile U32 ALIVEGPIOLOWDETECTMODEREADREG;			///< 0x3C : Alive GPIO Low Level Detect Mode Read Register

		volatile U32 ALIVEGPIODETECTMODERSTREG3;			///< 0x40 : Alive GPIO Detect Mode Reset Register3
		volatile U32 ALIVEGPIODETECTMODESETREG3;			///< 0x44 : Alive GPIO Detect Mode Reset Register3
		volatile U32 ALIVEGPIOHIGHDETECTMODEREADREG;		///< 0x48 : Alive GPIO High Level Detect Mode Read Register

		volatile U32 ALIVEGPIODETECTENBRSTREG;				///< 0x4C : Alive GPIO Detect Enable Reset Register
		volatile U32 ALIVEGPIODETECTENBSETREG;				///< 0x50 : Alive GPIO Detect Enable Set Register
		volatile U32 ALIVEGPIODETECTENBREADREG;				///< 0x54 : Alive GPIO Detect Enable Read Register

		volatile U32 ALIVEGPIOINTENBRSTREG;					///< 0x58 : Alive GPIO Interrupt Enable Reset Register
		volatile U32 ALIVEGPIOINTENBSETREG;					///< 0x5C : Alive GPIO Interrupt Enable Set Register
		volatile U32 ALIVEGPIOINTENBREADREG;				///< 0x60 : Alive GPIO Interrupt Enable Read Register

		volatile U32 ALIVEGPIODETECTPENDREG;				///< 0x64 : Alive GPIO Detect Pending Register

		volatile U32 ALIVESCRATCHRSTREG;					///< 0x68 : Alive Scratch Reset Register
		volatile U32 ALIVESCRATCHSETREG;					///< 0x6C : Alive Scratch Set Register
		volatile U32 ALIVESCRATCHREADREG;					///< 0x70 : Alive Scratch Read Register

		volatile U32 ALIVEGPIOPADOUTENBRSTREG;				///< 0x74 : Alive GPIO PAD Out Enable Reset Register
		volatile U32 ALIVEGPIOPADOUTENBSETREG;				///< 0x78 : Alive GPIO PAD Out Enable Set Register
		volatile U32 ALIVEGPIOPADOUTENBREADREG;				///< 0x7C : Alive GPIO PAD Out Enable Read Register

		volatile U32 ALIVEGPIOPADPULLUPRSTREG;				///< 0x80 : Alive GPIO PAD Pullup Reset Register
		volatile U32 ALIVEGPIOPADPULLUPSETREG;				///< 0x84 : Alive GPIO PAD Pullup Set Register
		volatile U32 ALIVEGPIOPADPULLUPREADREG;				///< 0x88 : Alive GPIO PAD Pullup Read Register

		volatile U32 ALIVEGPIOPADOUTRSTREG;					///< 0x8C : Alive GPIO PAD Out Reset Register
		volatile U32 ALIVEGPIOPADOUTSETREG;					///< 0x90 : Alive GPIO PAD Out Set Register
		volatile U32 ALIVEGPIOPADOUTREADREG;				///< 0x94 :	Alive GPIO PAD Out Read Register

		volatile U32 VDDCTRLRSTREG;							///< 0x98 : VDD Control Reset Register
		volatile U32 VDDCTRLSETREG;							///< 0x9C : VDD Control Set Register
		volatile U32 VDDCTRLREADREG;						///< 0xA0 : VDD Control Read Register

	 	volatile U32 CLEARWAKEUPSTATUS	    ;	//  wCS[41]
	    volatile U32 WAKEUPSTATUS	        ;	//  wCS[42]

	    volatile U32 ALIVESCRATCHRST1       ;	//  wCS[43]
	    volatile U32 ALIVESCRATCHSET1       ;	//  wCS[44]
	    volatile U32 ALIVESCRATCHVALUE1     ;	//  wCS[45]

	    volatile U32 ALIVESCRATCHRST2       ;	//  wCS[46]
	    volatile U32 ALIVESCRATCHSET2       ;	//  wCS[47]
	    volatile U32 ALIVESCRATCHVALUE2     ;	//  wCS[48]

	    volatile U32 ALIVESCRATCHRST3       ;	//  wCS[49]
	    volatile U32 ALIVESCRATCHSET3       ;	//  wCS[50]
	    volatile U32 ALIVESCRATCHVALUE3     ;	//  wCS[51]

	    volatile U32 ALIVESCRATCHRST4       ;	//  wCS[52]
	    volatile U32 ALIVESCRATCHSET4       ;	//  wCS[53]
	    volatile U32 ALIVESCRATCHVALUE4     ;	//  wCS[54]

	    volatile U32 ALIVESCRATCHRST5       ;	//  wCS[55]
	    volatile U32 ALIVESCRATCHSET5       ;	//  wCS[56]
	    volatile U32 ALIVESCRATCHVALUE5     ;	//  wCS[57]

	    volatile U32 ALIVESCRATCHRST6       ;	//  wCS[58]
	    volatile U32 ALIVESCRATCHSET6       ;	//  wCS[59]
	    volatile U32 ALIVESCRATCHVALUE6     ;	//  wCS[60]

	    volatile U32 ALIVESCRATCHRST7       ;	//  wCS[61]
	    volatile U32 ALIVESCRATCHSET7       ;	//  wCS[62]
	    volatile U32 ALIVESCRATCHVALUE7     ;	//  wCS[63]

	    volatile U32 ALIVESCRATCHRST8       ;	//  wCS[64]
	    volatile U32 ALIVESCRATCHSET8       ;	//  wCS[65]
	    volatile U32 ALIVESCRATCHVALUE8     ;	//  wCS[66]

	    volatile U32 VDDOFFCNTVALUERST      ;	//  wCS[67]
	    volatile U32 VDDOFFCNTVALUESET      ;	//  wCS[68]

	    volatile U32 VDDOFFCNTVALUE0	        ;	//  wCS[69]
	    volatile U32 VDDOFFCNTVALUE1	        ;	//  wCS[70]		
	    
	    volatile U32 ALIVEGPIOINPUTVALUE	;	//  wCS[71]		

	};

	///@brief	ALIVE Interrupts for interrupt interface
	enum
	{
		NX_ALIVE_INT_ALIVEGPIO0	= 0,	///<	ALIVE GPIO 0 Interrupt
		NX_ALIVE_INT_ALIVEGPIO1	= 1,	///<	ALIVE GPIO 1 Interrupt
		NX_ALIVE_INT_ALIVEGPIO2	= 2,	///<	ALIVE GPIO 2 Interrupt
		NX_ALIVE_INT_ALIVEGPIO3	= 3,	///<	ALIVE GPIO 3 Interrupt
		NX_ALIVE_INT_ALIVEGPIO4	= 4,	///<	ALIVE GPIO 4 Interrupt
		NX_ALIVE_INT_ALIVEGPIO5	= 5,	///<	ALIVE GPIO 5 Interrupt
		NX_ALIVE_INT_ALIVEGPIO6	= 6,	///<	ALIVE GPIO 6 Interrupt
		NX_ALIVE_INT_ALIVEGPIO7	= 7		///<	ALIVE GPIO 7 Interrupt
	};

	///@brief	ALIVE GPIO Detect Mode
	typedef enum
	{
		NX_ALIVE_DETECTMODE_ASYNC_LOWLEVEL		= 0,	///< Async Low Level Detect Mode
		NX_ALIVE_DETECTMODE_ASYNC_HIGHLEVEL		= 1,	///< Async High Level Detect Mode
		NX_ALIVE_DETECTMODE_SYNC_FALLINGEDGE	= 2,	///< Sync Falling Edge Detect Mode
		NX_ALIVE_DETECTMODE_SYNC_RISINGEDGE		= 3,	///< Sync Rising Edge Detect Mode
		NX_ALIVE_DETECTMODE_SYNC_LOWLEVEL		= 4,	///< Sync Low Level Detect Mode
		NX_ALIVE_DETECTMODE_SYNC_HIGHLEVEL		= 5		///< Sync High Level Detect Mode

	} NX_ALIVE_DETECTMODE;

	/// @brief	Retention PAD Group
	typedef enum
	{
		NX_ALIVE_PADGROUP0 = 0,		///< IO Power Group 0 ( RX0 ~ RX4 )
		NX_ALIVE_PADGROUP1 = 1,		///< IO Power Group 1 ( USB VBUS )
		NX_ALIVE_PADGROUP2 = 2,		///< IO Power Group 2 ( GPIO )
		NX_ALIVE_PADGROUP3 = 3,		///< IO Power Group 2 ( GPIO )
		NX_ALIVE_PADGROUP_NUMBER = 4

	} NX_ALIVE_PADGROUP;


//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_ALIVE_Initialize( void );
U32		NX_ALIVE_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_ALIVE_GetPhysicalAddress( void );
U32		NX_ALIVE_GetSizeOfRegisterSet( void );
void	NX_ALIVE_SetBaseAddress( void* BaseAddress );
void*	NX_ALIVE_GetBaseAddress( void );
CBOOL	NX_ALIVE_OpenModule( void );
CBOOL	NX_ALIVE_CloseModule( void );
CBOOL	NX_ALIVE_CheckBusy( void );
CBOOL	NX_ALIVE_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
S32		NX_ALIVE_GetInterruptNumber( void );

void	NX_ALIVE_SetInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_ALIVE_GetInterruptEnable( S32 IntNum );
CBOOL	NX_ALIVE_GetInterruptPending( S32 IntNum );
void	NX_ALIVE_ClearInterruptPending( S32 IntNum );

void	NX_ALIVE_SetInterruptEnableAll( CBOOL Enable );
CBOOL	NX_ALIVE_GetInterruptEnableAll( void );
CBOOL	NX_ALIVE_GetInterruptPendingAll( void );
void	NX_ALIVE_ClearInterruptPendingAll( void );

void	NX_ALIVE_SetInterruptEnable32( U32 EnableFlag );
U32		NX_ALIVE_GetInterruptEnable32( void );
U32		NX_ALIVE_GetInterruptPending32( void );
void	NX_ALIVE_ClearInterruptPending32( U32 PendingFlag );

S32		NX_ALIVE_GetInterruptPendingNumber( void );	// -1 if None
//@}

//------------------------------------------------------------------------------
/// @name	PAD Configuration
//@{
void	NX_ALIVE_SetWriteEnable( CBOOL bEnable );
CBOOL	NX_ALIVE_GetWriteEnable( void );
void	NX_ALIVE_SetScratchReg( U32 dwData );
U32		NX_ALIVE_GetScratchReg( void );
void	NX_ALIVE_SetPullUpEnable( U32 BitNumber, CBOOL bEnable );
CBOOL	NX_ALIVE_GetPullUpEnable( U32 BitNumber );
void	NX_ALIVE_SetPadRetentionEnable( NX_ALIVE_PADGROUP Group, CBOOL bEnable );
CBOOL	NX_ALIVE_GetPadRetentionEnable( NX_ALIVE_PADGROUP Group );
void	NX_ALIVE_SetPadRetentionHold( NX_ALIVE_PADGROUP Group, CBOOL bEnable );
CBOOL	NX_ALIVE_GetPadRetentionHold( NX_ALIVE_PADGROUP Group );
//@}

//------------------------------------------------------------------------------
/// @name	Input Setting Function
//@{
void	NX_ALIVE_SetDetectEnable( U32 BitNumber, CBOOL bEnable );
CBOOL	NX_ALIVE_GetDetectEnable( U32 BitNumber );
void	NX_ALIVE_SetDetectMode( NX_ALIVE_DETECTMODE DetectMode, U32 BitNumber, CBOOL bEnable );
CBOOL	NX_ALIVE_GetDetectMode( NX_ALIVE_DETECTMODE DetectMode, U32 BitNumber );
CBOOL	NX_ALIVE_GetVDDPWRTOGGLE( void );
//@}

//------------------------------------------------------------------------------
/// @name	Output Setting Function
//@{
void	NX_ALIVE_SetOutputEnable( U32 BitNumber, CBOOL OutputEnb );
CBOOL	NX_ALIVE_GetOutputEnable( U32 BitNumber );
CBOOL	NX_ALIVE_GetInputEnable( U32 BitNumber );
void	NX_ALIVE_SetOutputValue( U32 BitNumber, CBOOL Value );
CBOOL	NX_ALIVE_GetOutputValue( U32 BitNumber );
CBOOL	NX_ALIVE_GetInputValue( U32 BitNumber );
void	NX_ALIVE_SetVDDPWRON( CBOOL bVDDPWRON, CBOOL bVDDPWRON_DDR );
CBOOL	NX_ALIVE_GetVDDPWRON( void );
CBOOL	NX_ALIVE_GetVDDPWRON_DDR( void );
U32     NX_ALIVE_GetCorePowerOffDelayTime(void);
void 	NX_ALIVE_SetCorePowerOffDelayTime( U32 Delay );
void    NX_ALIVE_CheckWakeUpStatus( void );
U32     NX_ALIVE_GetWakeUpStatus( void );
void    NX_ALIVE_ClearWakeUpStatus( void );

void	NX_ALIVE_SetOutputEnable32( U32 value32 );
void	NX_ALIVE_SetInputEnable32( U32 value32 );
U32		NX_ALIVE_GetOutputEnable32 (void);
U32		NX_ALIVE_GetInputEnable32 (void);
void    NX_ALIVE_SetPullUpEnable32( U32 value32 );

U32     NX_ALIVE_GetInputValue32( void );
void	NX_ALIVE_SetDetectEnable32( U32 value32 );
void	NX_ALIVE_SetDetectMode32( NX_ALIVE_DETECTMODE DetectMode, U32 value32 );

//@}

//@}


#ifdef __cplusplus
}
#endif


#endif	// __NX_ALIVE_H__
