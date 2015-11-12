//------------------------------------------------------------------------------
//
//	Copyright (C) 2012 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: AC97
//	File		: nx_ac97.h
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120802 first implementation
//------------------------------------------------------------------------------
#ifndef _NX_AC97_H
#define _NX_AC97_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @brief  AC97 register set structure
//------------------------------------------------------------------------------
struct NX_AC97_RegisterSet
{
	volatile U32 GLBCTRL;		// 0x00	// AC97 global control register
	volatile U32 GLBSTAT;		// 0x04 // AC97 global status register
	volatile U32 CODEC_CMD;	    // 0x08 // AC97 codec command register
	volatile U32 CODEC_STAT;	// 0x0C // AC97 codec status register
	volatile U32 PCMADDR;		// 0x10 // AC97 PCM out/in channel FIFO address register
	volatile U32 MICADDR;		// 0x14 // AC97 MIC In channel FIFO address register
	volatile U32 PCMDATA;		// 0x18 // AC97 PCM out/in channel FIFO data register
	volatile U32 MICDATA;		// 0x1C // AC97 MIC in channel FIFO data register
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------
enum
{
	NX_AC97_CODEC_RDY			= 6,	///< Codec ready interrupt
	NX_AC97_PCMOUT_OVR_INTR		= 5,	///< PCM out channel underrun interrupt
	NX_AC97_PCMIN_OVR_INTR		= 4,	///< PCM in channel overrun interrupt
	NX_AC97_MICIN_OVR_INTR		= 3,	///< MIC in channel overrun interrupt
	NX_AC97_PCMOUT_THRS_INTR	= 2,	///< PCM out channel threshold interrupt
	NX_AC97_PCMIN_THRS_INTR		= 1,	///< PCM in channel threshold interrupt
	NX_AC97_MICIN_THRS_INTR		= 0		///< MIC in channel threshold interrupt
};

enum
{
	NX_AC97_USE_AC_LINK		= 1UL<<3,	///< Transfer data enable using AC-link
	NX_AC97_AC_LINK_ON		= 1UL<<2,	///< AC-Link on
	NX_AC97_WARM_RESET		= 1UL<<1,	///< Warm reset
	NX_AC97_COLD_RESET		= 1UL<<0	///< Cold reset
};

enum
{
	NX_AC97_MICIN_MODE	= 0,	///< mode - mic in
	NX_AC97_PCMIN_MODE	= 1,	///< mode - pcm in
	NX_AC97_PCMOUT_MODE	= 2		///< mode - pcm out
};

enum
{
	NX_AC97_CTRL_MICIN_OFFSET	= 8,	///< offset - mic in
	NX_AC97_CTRL_PCMIN_OFFSET	= 10,	///< offset - pcm in
	NX_AC97_CTRL_PCMOUT_OFFSET	= 12	///< offset - pcm out
};

enum
{
	NX_AC97_CH_TR_MODE_OFF	= 0,	///< channel transfer mode - off
	NX_AC97_CH_TR_MODE_PIO	= 1,	///< channel transfer mode - pio
	NX_AC97_CH_TR_MODE_DMA	= 2		///< channel transfer mode - dma
};

enum
{
	NX_AC97_CTRL_STATE_IDLE	= 0,	///< controller main state - idle
	NX_AC97_CTRL_STATE_INIT	= 1,	///< controller main state - init
	NX_AC97_CTRL_STATE_RDY	= 2,	///< controller main state - ready
	NX_AC97_CTRL_STATE_ACT	= 3,	///< controller main state - active
	NX_AC97_CTRL_STATE_LP	= 4,	///< controller main state - low power
	NX_AC97_CTRL_STATE_WRM	= 5,	///< controller main state - warm reset
	NX_AC97_CTRL_STATE_MSK	= 7		///< controller main state mask
};

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_AC97_Initialize( void );
U32		NX_AC97_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_AC97_GetPhysicalAddress( U32 ModuleIndex );
//U32		NX_AC97_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
U32		NX_AC97_GetResetNumber( U32 ModuleIndex );
U32		NX_AC97_GetNumberOfReset( void );
U32		NX_AC97_GetSizeOfRegisterSet( void );
void	NX_AC97_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_AC97_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_AC97_OpenModule( U32 ModuleIndex );
CBOOL	NX_AC97_CloseModule( U32 ModuleIndex );
CBOOL	NX_AC97_CheckBusy( U32 ModuleIndex );
CBOOL	NX_AC97_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_AC97_GetInterruptNumber( U32 ModuleIndex );

void	NX_AC97_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_AC97_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_AC97_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_AC97_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

void	NX_AC97_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_AC97_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_AC97_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_AC97_ClearInterruptPendingAll( U32 ModuleIndex );

U32		NX_AC97_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_AC97_GetDMAIndex_PCMout( U32 ModuleIndex );
U32		NX_AC97_GetDMAIndex_PCMin( U32 ModuleIndex );
U32		NX_AC97_GetDMAIndex_MICin( U32 ModuleIndex );
U32		NX_AC97_GetDMABusWidth( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_AC97_SetGLBCTRL( U32 ModuleIndex, U32 value );
U32		NX_AC97_GetGLBCTRL( U32 ModuleIndex );

U32		NX_AC97_GetGLBSTAT( U32 ModuleIndex );

void	NX_AC97_SetCODEC_CMD( U32 ModuleIndex, U32 value );
U32		NX_AC97_GetCODEC_CMD( U32 ModuleIndex );

U32		NX_AC97_GetCODEC_STAT( U32 ModuleIndex );

U32		NX_AC97_GetPCMADDR( U32 ModuleIndex );

U32		NX_AC97_GetMICADDR( U32 ModuleIndex );

void	NX_AC97_SetPCMDATA( U32 ModuleIndex, U32 value );
U32		NX_AC97_GetPCMDATA( U32 ModuleIndex );

void	NX_AC97_SetMICDATA( U32 ModuleIndex, U32 value );
U32		NX_AC97_GetMICDATA( U32 ModuleIndex );
//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_AC97_H
