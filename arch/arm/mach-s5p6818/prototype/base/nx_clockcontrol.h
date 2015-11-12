//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: base
//	File		: nx_clockcontrol.h
//	Description: support clock control type.
//	Author		: Goofy
//	Export		:
//	History	:
//		2010.04.27	Hans
//		2007.04.04	Goofy first draft.
//------------------------------------------------------------------------------
#ifndef __NX_CLOCKCONTROL_H__
#define __NX_CLOCKCONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
///	@name	Clock Control types
//------------------------------------------------------------------------------
//@{

///	@brief	type for PCLK control mode
typedef enum
{
	NX_PCLKMODE_DYNAMIC = 0UL,		///< PCLK is provided only when CPU has access to registers of this module.
	NX_PCLKMODE_ALWAYS	= 1UL		///< PCLK is always provided for this module.
} NX_PCLKMODE ;

///	@brief type for BCLK control mode
typedef enum
{
	NX_BCLKMODE_DISABLE	= 0UL,	///< BCLK is disabled.
	NX_BCLKMODE_DYNAMIC	= 2UL,	///< BCLK is provided only when this module requests it.
	NX_BCLKMODE_ALWAYS	= 3UL		///< BCLK is always provided for this module.
} NX_BCLKMODE ;

//@}

#ifdef __cplusplus
}
#endif

#endif // __NX_CLOCKCONTROL_H__

