//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: TZC380 TrustZone Address Space Controller
//	File		: nx_tzc380.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.09.11 Hans create
//------------------------------------------------------------------------------
#ifndef __NX_TZC380_H__
#define __NX_TZC380_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	TZC-380 Base
//------------------------------------------------------------------------------
//@{

	struct NX_TZC_RegionSetup
	{
		volatile U32 REGION_SETUP_LOW;		// 0x1x0 Region Setup Low
		volatile U32 REGION_SETUP_HIGH;		// 0x1x4 Region Setup High
		volatile U32 REGION_ATTRIBUTES;		// 0x1x8 Region Attributes
		volatile U32 _Reserved;				// 0x1xC
	};

	struct	NX_TZC380_RegisterSet
	{
		volatile U32 CONFIGURATION;			// 0x000 Configuration
		volatile U32 ACTION;				// 0x004 Action
		volatile U32 LOCKDOWN_RANGE;		// 0x008 Lockdown Range
		volatile U32 LOCKDOWN_SELECT;		// 0x00C Lockdown Select
		volatile U32 INT_STATUS;			// 0x010 Interrupt Status
		volatile U32 INT_CLEAR;				// 0x014 Interrupt Clear
		volatile U32 _Reserved0[2];			// 0x018~0x01C
		volatile U32 FAIL_ADDR_LOW;			// 0x020 Fail Address Low
		volatile U32 FAIL_ADDR_HIGH;		// 0x024 Fail Address High
		volatile U32 FAIL_CONTROL;			// 0x028 Fail Control
		volatile U32 FAIL_ID;				// 0x02C Fail ID
		volatile U32 SPECULATION_CONTROL;	// 0x030 Speculation Control
		volatile U32 SECURITY_INVERSION_EN;	// 0x034 Security Inversion Enable
		volatile U32 _Reserved1[50];		// 0x038~0x0FC
		struct NX_TZC_RegionSetup RS[16];	// 0x100~0x1FC
		volatile U32 _Reserved2[0x300];		// 0x200~0xDFC
		volatile U32 ITCRG;					// 0xE00
		volatile U32 ITIP;					// 0xE04
		volatile U32 ITOP;					// 0xE08
		volatile U32 _Reserved3[0x71];		// 0xE0C~0xFCC
		volatile U32 PERIPH_ID4;			// 0xFD0
		volatile U32 _Reserved4[3];			// 0xFD4~0xFDC
		volatile U32 PERIPH_ID[4];			// 0xFE0~0xFEC
		volatile U32 COMPONENT_ID[4];		// 0xFF0~0xFFC
	};



//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TZC380_Initialize( void );
U32		NX_TZC380_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TZC380_GetPhysicalAddress( void );
U32		NX_TZC380_GetSizeOfRegisterSet( void );

void	NX_TZC380_SetBaseAddress( void* BaseAddress );
void*	NX_TZC380_GetBaseAddress( void );

CBOOL	NX_TZC380_OpenModule( void );
CBOOL	NX_TZC380_CloseModule( void );
CBOOL	NX_TZC380_CheckBusy( void );
CBOOL	NX_TZC380_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
// there is no interrupt interface
//@}

//------------------------------------------------------------------------------
/// @name	Clock Management
//@{
// there is no clock interface
//@}

//------------------------------------------------------------------------------
/// @name	Power Management
//@{
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


#endif // __NX_TZC380_H__

