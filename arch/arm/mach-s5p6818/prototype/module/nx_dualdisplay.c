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
//	File		: nx_DualDisplay.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_dualdisplay.h"


// DualDisplay Prototype Must be 1 MODULE
NX_CASSERT( NUMBER_OF_DUALDISPLAY_MODULE == 1 );


//------------------------------------------------------------------------------
//
//	DUALDISPLAY Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_DUALDISPLAY?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_DUALDISPLAY_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( DUALDISPLAY, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_DUALDISPLAY_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_DUALDISPLAY_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}

