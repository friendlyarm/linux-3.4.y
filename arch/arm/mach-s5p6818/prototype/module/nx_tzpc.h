//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: TZPC TrustZone Protection Controller
//	File		: nx_tzpc.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.09.11 Hans create
//------------------------------------------------------------------------------
#ifndef __NX_TZPC_H__
#define __NX_TZPC_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	TZPC Base
//------------------------------------------------------------------------------
//@{
	struct NX_TZPC_DETPROT
	{
		volatile U32 STATUS;			// Decode Protection Status
		volatile U32 SET;				// Decode Protect Set
		volatile U32 CLEAR;				// Decode Protect Clear
	};

	struct	NX_TZPC_RegisterSet
	{
		volatile U32 R0SIZE;				// 0x000 RAM Region Size
		volatile U32 _Reserved0[0x1FF];		// 0x004~0x7FC
		struct NX_TZPC_DETPROT DETPROT[3];	// 0x800~0x820
		volatile U32 _Reserved1[0x1EF];		// 0x824~0xFDC
		volatile U32 PERIPHID[4];			// 0xFE0~0xFEC
		volatile U32 PCELLID[4];			// 0xFF0~0xFFC
	};



//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TZPC_Initialize( void );
U32		NX_TZPC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TZPC_GetPhysicalAddress( void );
U32		NX_TZPC_GetSizeOfRegisterSet( void );

void	NX_TZPC_SetBaseAddress( void* BaseAddress );
void*	NX_TZPC_GetBaseAddress( void );
=======
void	NX_TZPC_SetBaseAddress( U32 BaseAddress );
U32*	NX_TZPC_GetBaseAddress( void );

CBOOL	NX_TZPC_OpenModule( void );
CBOOL	NX_TZPC_CloseModule( void );
CBOOL	NX_TZPC_CheckBusy( void );
CBOOL	NX_TZPC_CanPowerDown( void );
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


#endif // __NX_TZPC_H__

