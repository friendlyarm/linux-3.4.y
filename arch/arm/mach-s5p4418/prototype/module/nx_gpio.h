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
//	Module		: GPIO
//	File		: nx_gpio.h
//	Description:
//	Author		: Firmware Team
//	History	:
//
//------------------------------------------------------------------------------
#ifndef __NX_GPIO_H__
#define __NX_GPIO_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	GPIO GPIO
//------------------------------------------------------------------------------
//@{

	/// @brief	GPIO Module's Register List
	struct	NX_GPIO_RegisterSet
	{
		volatile U32 GPIOxOUT;			///< 0x00	: Output Register
		volatile U32 GPIOxOUTENB;		///< 0x04	: Output Enable Register
		volatile U32 GPIOxDETMODE[2];	///< 0x08	: Event Detect Mode Register
		volatile U32 GPIOxINTENB;		///< 0x10	: Interrupt Enable Register
		volatile U32 GPIOxDET;			///< 0x14	: Event Detect Register
		volatile U32 GPIOxPAD;			///< 0x18	: PAD Status Register
		volatile U32 GPIOxPUENB;		///< 0x1C	: Pull Up Enable Register
		volatile U32 GPIOxALTFN[2];		///< 0x20	: Alternate Function Select Register
		volatile U32 GPIOxDETMODEEX;    ///< 0x28   : Event Detect Mode extended Register

		volatile U32 __Reserved[4];		///< 0x2B	:
		volatile U32 GPIOxDETENB;		///< 0x3C	: IntPend Detect Enable Register

		volatile U32 GPIOx_SLEW;					///< 0x40	: Slew Register
		volatile U32 GPIOx_SLEW_DISABLE_DEFAULT;    ///< 0x44	: Slew set On/Off Register
		volatile U32 GPIOx_DRV1;                    ///< 0x48	: drive strength LSB Register
		volatile U32 GPIOx_DRV1_DISABLE_DEFAULT;    ///< 0x4C	: drive strength LSB set On/Off Register
		volatile U32 GPIOx_DRV0;                    ///< 0x50	: drive strength MSB Register
		volatile U32 GPIOx_DRV0_DISABLE_DEFAULT;    ///< 0x54	: drive strength MSB set On/Off Register
		volatile U32 GPIOx_PULLSEL;                 ///< 0x58	: Pull UP/DOWN Selection Register
		volatile U32 GPIOx_PULLSEL_DISABLE_DEFAULT; ///< 0x5C	: Pull UP/DOWN Selection On/Off Register
		volatile U32 GPIOx_PULLENB;                 ///< 0x60	: Pull Enable/Disable Register
		volatile U32 GPIOx_PULLENB_DISABLE_DEFAULT; ///< 0x64	: Pull Enable/Disable selection On/Off Register
		volatile U32 GPIOx_InputMuxSelect0;			///< 0x68
		volatile U32 GPIOx_InputMuxSelect1;			///< 0x6C
		U8 __Reserved1[0x1000-0x70];
	};

	///@brief	GPIO Interrupts for interrupt interface
	enum
	{
		NX_GPIO_INT_GPIO0	= 0,	///<	GPIO 0 Interrupt
		NX_GPIO_INT_GPIO1	= 1,	///<	GPIO 1 Interrupt
		NX_GPIO_INT_GPIO2	= 2,	///<	GPIO 2 Interrupt
		NX_GPIO_INT_GPIO3	= 3,	///<	GPIO 3 Interrupt
		NX_GPIO_INT_GPIO4	= 4,	///<	GPIO 4 Interrupt
		NX_GPIO_INT_GPIO5	= 5,	///<	GPIO 5 Interrupt
		NX_GPIO_INT_GPIO6	= 6,	///<	GPIO 6 Interrupt
		NX_GPIO_INT_GPIO7	= 7,	///<	GPIO 7 Interrupt
		NX_GPIO_INT_GPIO8	= 8,	///<	GPIO 8 Interrupt
		NX_GPIO_INT_GPIO9	= 9,	///<	GPIO 9 Interrupt
		NX_GPIO_INT_GPIO10	= 10,	///<	GPIO 10 Interrupt
		NX_GPIO_INT_GPIO11	= 11,	///<	GPIO 11 Interrupt
		NX_GPIO_INT_GPIO12	= 12,	///<	GPIO 12 Interrupt
		NX_GPIO_INT_GPIO13	= 13,	///<	GPIO 13 Interrupt
		NX_GPIO_INT_GPIO14	= 14,	///<	GPIO 14 Interrupt
		NX_GPIO_INT_GPIO15	= 15,	///<	GPIO 15 Interrupt
		NX_GPIO_INT_GPIO16	= 16,	///<	GPIO 16 Interrupt
		NX_GPIO_INT_GPIO17	= 17,	///<	GPIO 17 Interrupt
		NX_GPIO_INT_GPIO18	= 18,	///<	GPIO 18 Interrupt
		NX_GPIO_INT_GPIO19	= 19,	///<	GPIO 19 Interrupt
		NX_GPIO_INT_GPIO20	= 20,	///<	GPIO 20 Interrupt
		NX_GPIO_INT_GPIO21	= 21,	///<	GPIO 21 Interrupt
		NX_GPIO_INT_GPIO22	= 22,	///<	GPIO 22 Interrupt
		NX_GPIO_INT_GPIO23	= 23,	///<	GPIO 23 Interrupt
		NX_GPIO_INT_GPIO24	= 24,	///<	GPIO 24 Interrupt
		NX_GPIO_INT_GPIO25	= 25,	///<	GPIO 25 Interrupt
		NX_GPIO_INT_GPIO26	= 26,	///<	GPIO 26 Interrupt
		NX_GPIO_INT_GPIO27	= 27,	///<	GPIO 27 Interrupt
		NX_GPIO_INT_GPIO28	= 28,	///<	GPIO 28 Interrupt
		NX_GPIO_INT_GPIO29	= 29,	///<	GPIO 29 Interrupt
		NX_GPIO_INT_GPIO30	= 30,	///<	GPIO 30 Interrupt
		NX_GPIO_INT_GPIO31	= 31	///<	GPIO 31 Interrupt
	};

	/// @brief	GPIO interrupt mode
	typedef enum
	{
		NX_GPIO_INTMODE_LOWLEVEL	= 0UL,		///< Low level detect
		NX_GPIO_INTMODE_HIGHLEVEL	= 1UL,		///< High level detect
		NX_GPIO_INTMODE_FALLINGEDGE = 2UL,		///< Falling edge detect
		NX_GPIO_INTMODE_RISINGEDGE	= 3UL,		///< Rising edge detect
		NX_GPIO_INTMODE_BOTHEDGE    = 4UL  		///< both (rise and falling) edge detect

	}NX_GPIO_INTMODE;

	/// @brief	I/O mode
	typedef enum
	{
		NX_GPIO_PADFUNC_0			= 0UL,			///< Alternate function 0
		NX_GPIO_PADFUNC_1			= 1UL,			///< Alternate function 1
		NX_GPIO_PADFUNC_2			= 2UL,			///< Alternate function 2
		NX_GPIO_PADFUNC_3			= 3UL			///< Alternate function 3

	}NX_GPIO_PADFUNC;

	/// @brief	Pull I/O mode
	typedef enum
	{
		NX_GPIO_DRVSTRENGTH_0		= 0UL,
		NX_GPIO_DRVSTRENGTH_1		= 1UL,
		NX_GPIO_DRVSTRENGTH_2		= 2UL,
		NX_GPIO_DRVSTRENGTH_3		= 3UL

	} NX_GPIO_DRVSTRENGTH;

	typedef enum
	{
		NX_GPIO_PULL_DOWN			= 0UL,
		NX_GPIO_PULL_UP			    = 1UL,
		NX_GPIO_PULL_OFF			= 2UL
	} NX_GPIO_PULL;
    
//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_GPIO_Initialize( void );
U32		NX_GPIO_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_GPIO_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_GPIO_GetSizeOfRegisterSet( void );
void	NX_GPIO_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_GPIO_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_GPIO_OpenModule( U32 ModuleIndex );
CBOOL	NX_GPIO_CloseModule( U32 ModuleIndex );
CBOOL	NX_GPIO_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_GPIO_GetInterruptNumber( U32 ModuleIndex );

void	NX_GPIO_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable );
CBOOL	NX_GPIO_GetInterruptEnable( U32 ModuleIndex, S32 IntNum );
CBOOL	NX_GPIO_GetInterruptPending( U32 ModuleIndex, S32 IntNum );
void	NX_GPIO_ClearInterruptPending( U32 ModuleIndex, S32 IntNum );

void	NX_GPIO_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_GPIO_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_GPIO_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_GPIO_ClearInterruptPendingAll( U32 ModuleIndex );

void	NX_GPIO_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32		NX_GPIO_GetInterruptEnable32( U32 ModuleIndex );
U32		NX_GPIO_GetInterruptPending32( U32 ModuleIndex );
void	NX_GPIO_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );

S32		NX_GPIO_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	GPIO Operation.
//@{
void			NX_GPIO_SetInterruptMode( U32 ModuleIndex, U32 BitNumber, NX_GPIO_INTMODE IntMode );
NX_GPIO_INTMODE	NX_GPIO_GetInterruptMode( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetDetectEnable ( U32 ModuleIndex, U32 BitNumber, CBOOL DetectEnb );
CBOOL			NX_GPIO_GetDetectEnable ( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetDetectEnable32 ( U32 ModuleIndex, U32 EnableFlag );
U32				NX_GPIO_GetDetectEnable32 ( U32 ModuleIndex );
void			NX_GPIO_SetOutputEnable ( U32 ModuleIndex, U32 BitNumber, CBOOL OutputEnb );
CBOOL			NX_GPIO_GetOutputEnable ( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetOutputEnable32 ( U32 ModuleIndex, CBOOL OutputEnb );
U32				NX_GPIO_GetOutputEnable32 ( U32 ModuleIndex );
void			NX_GPIO_SetOutputValue	( U32 ModuleIndex, U32 BitNumber, CBOOL Value );
CBOOL			NX_GPIO_GetOutputValue	( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetOutputValue32	( U32 ModuleIndex, U32 Value );
U32				NX_GPIO_GetOutputValue32	( U32 ModuleIndex );
CBOOL			NX_GPIO_GetInputValue		( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetPullMode ( U32 ModuleIndex, U32 BitNumber, NX_GPIO_PULL mode);
void			NX_GPIO_SetPullSelect ( U32 ModuleIndex, U32 BitNumber, CBOOL enable);
CBOOL			NX_GPIO_GetPullSelect ( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetPullSelect32 ( U32 ModuleIndex, U32 Value );
U32				NX_GPIO_GetPullSelect32 ( U32 ModuleIndex );
void			NX_GPIO_SetPullEnable ( U32 ModuleIndex, U32 BitNumber, CBOOL enable);
CBOOL			NX_GPIO_GetPullEnable ( U32 ModuleIndex, U32 BitNumber );
void			NX_GPIO_SetPullEnable32 ( U32 ModuleIndex, U32 Value );
U32				NX_GPIO_GetPullEnable32 ( U32 ModuleIndex );

void			NX_GPIO_SetPadFunction( U32 ModuleIndex, U32 BitNumber, NX_GPIO_PADFUNC padfunc );
void	NX_GPIO_SetPadFunction32_0( U32 ModuleIndex, U32 Value );
void	NX_GPIO_SetPadFunction32_1( U32 ModuleIndex, U32 Value );
NX_GPIO_PADFUNC	NX_GPIO_GetPadFunction( U32 ModuleIndex, U32 BitNumber );

//------------------------------------------------------------------------------
///	@name	GPIO Operation.
//@{
void	NX_GPIO_SetSLEW	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetSLEW	( U32 ModuleIndex );
void	NX_GPIO_SetSLEW_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetSLEW_DISABLE_DEFAULT	( U32 ModuleIndex );
void	NX_GPIO_SetDRV1	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetDRV1	( U32 ModuleIndex );
void	NX_GPIO_SetDRV1_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetDRV1_DISABLE_DEFAULT	( U32 ModuleIndex );
void	NX_GPIO_SetDRV0	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetDRV0	( U32 ModuleIndex );
void	NX_GPIO_SetDRV0_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetDRV0_DISABLE_DEFAULT	( U32 ModuleIndex );

void	NX_GPIO_SetSlew	( U32 ModuleIndex, U32 BitNumber, CBOOL Enable );
CBOOL	NX_GPIO_GetSlew	( U32 ModuleIndex, U32 BitNumber );
void	NX_GPIO_SetSlewDisableDefault	( U32 ModuleIndex, U32 BitNumber, CBOOL Enable );

void	NX_GPIO_SetSlew32	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetSlew32	( U32 ModuleIndex );
void	NX_GPIO_SetDriveStrength(U32 ModuleIndex, U32 BitNumber, NX_GPIO_DRVSTRENGTH drvstrength);
NX_GPIO_DRVSTRENGTH		NX_GPIO_GetDriveStrength(U32 ModuleIndex, U32 BitNumber);
void	NX_GPIO_SetDriveStrengthDisableDefault	( U32 ModuleIndex, U32 BitNumber, CBOOL Enable );


void	NX_GPIO_SetPULLSEL	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetPULLSEL	( U32 ModuleIndex );
void	NX_GPIO_SetPULLSEL_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetPULLSEL_DISABLE_DEFAULT	( U32 ModuleIndex );
void	NX_GPIO_SetPULLENB	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetPULLENB	( U32 ModuleIndex );
void	NX_GPIO_SetPULLENB_DISABLE_DEFAULT	( U32 ModuleIndex, U32 Value );
U32		NX_GPIO_GetPULLENB_DISABLE_DEFAULT	( U32 ModuleIndex );
void	NX_GPIO_SetInputMuxSelect0	( U32 ModuleIndex, U32 Value );
void	NX_GPIO_SetInputMuxSelect1	( U32 ModuleIndex, U32 Value );

U32				NX_GPIO_GetValidBit( U32 ModuleIndex );

void NX_GPIO_SetPadFunctionEnable (  U32 padInfo  );


//@}

//@}

#ifdef	__cplusplus
}
#endif

#endif //__NX_GPIO_H__
