//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		:
//	File		: nx_MPEGTSI.h
//	Description	:
//	Author		: Sei (parkjh@nexell.co.kr)
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_MPEGTSI_H__
#define __NX_MPEGTSI_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	MPEGTSI
//------------------------------------------------------------------------------
//@{
    /// @brief  MPEGTSI Module's Register List
    struct  NX_MPEGTSI_RegisterSet
    {
        volatile U32 CAP_CTRL[2];   // 0x00 - 0x04  : Capture 0 control register
        //volatile U32 CAP1_CTRL;     // 0x04         : Capture 1 control register
        volatile U32 CPU_WRDATA;    // 0x08         : CPU Write data register
        volatile U32 CPU_WRADDR;    // 0x0C         : CPU Write address register
        volatile U32 CAP_DATA[2];   // 0x10 - 0x14  : Capture 0 data register
        //volatile U32 CAP1_DATA;     // 0x14         : Capture 1 data register
        volatile U32 TSP_INDATA;    // 0x18         : MPEGTSI input data register
        volatile U32 TSP_OUTDATA;   // 0x1C         : MPEGTSI output data regsiter
        volatile U32 CTRL0;         // 0x20         : MPEGTSI control register
        volatile U32 IDMAEN;        // 0x24         : MPEGTSI internal dma enable register
        volatile U32 IDMARUN;       // 0x28         : MPEGTSI internal dma run/stop register
        volatile U32 IDMAINT;       // 0x2C         : MPEGTSI internal dma interrupt register
        volatile U32 IDMAADDR[4];   // 0x30 - 0x3C  : MPEGTSI internal dma address register
        volatile U32 IDMALEN[4];    // 0x40 - 0x4C  : MPEGTSI internal dma length register
    };
//@}

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_MPEGTSI_Initialize( void );
U32		NX_MPEGTSI_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
/// @name   Basic Interface
//@{
U32     NX_MPEGTSI_GetPhysicalAddress( void );
U32     NX_MPEGTSI_GetSizeOfRegisterSet( void );

void    NX_MPEGTSI_SetBaseAddress( void* BaseAddress );
void*   NX_MPEGTSI_GetBaseAddress( void );

CBOOL   NX_MPEGTSI_OpenModule( void );
CBOOL   NX_MPEGTSI_CloseModule( void );
CBOOL   NX_MPEGTSI_CheckBusy( void );
CBOOL   NX_MPEGTSI_CanPowerDown( void );
void    NX_MPEGTSI_EnableChannelPAD( U32 ChannelIndex );
//@}

//------------------------------------------------------------------------------
/// @name   Clock Control Interface
//@{
U32     NX_MPEGTSI_GetClockNumber( void );
U32     NX_MPEGTSI_GetResetNumber( void );
//@}

//------------------------------------------------------------------------------
/// @name   Interrupt Interface
//@{
S32     NX_MPEGTSI_GetInterruptNumber( void );
//@}

//------------------------------------------------------------------------------
/// @name   DMA Interface
//@{
U32     NX_MPEGTSI_GetDMAIndex( U32 ChannelIndex );
U32     NX_MPEGTSI_GetDMABusWidth( void );
//@}

//------------------------------------------------------------------------------
/// @name   Internal DMA Interface
//@{
void    NX_MPEGTSI_SetIDMAEnable( U32 ChannelIndex, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetIDMAEnable( U32 ChannelIndex );
CBOOL   NX_MPEGTSI_GetIDMABusyStatus( U32 ChannelIndex );
void    NX_MPEGTSI_RunIDMA( U32 ChannelIndex );
void    NX_MPEGTSI_StopIDMA( U32 ChannelIndex );
void    NX_MPEGTSI_SetIDMABaseAddr( U32 ChannelIndex, U32 BaseAddr );
U32     NX_MPEGTSI_GetIDMABaseAddr( U32 ChannelIndex );
void    NX_MPEGTSI_SetIDMALength( U32 ChannelIndex, U32 Length );
U32     NX_MPEGTSI_GetIDMALength( U32 ChannelIndex );
void    NX_MPEGTSI_SetIDMAIntEnable( U32 ChannelIndex, CBOOL Enable );
U32     NX_MPEGTSI_GetIDMAIntEnable( void );
void    NX_MPEGTSI_SetIDMAIntMaskClear( U32 ChannelIndex, CBOOL Unmask );
U32     NX_MPEGTSI_GetIDMAIntMaskClear( void );
CBOOL   NX_MPEGTSI_GetIDMAIntStatus( U32 ChannelIndex );
U32     NX_MPEGTSI_GetIDMAIntRawStatus( void );
void    NX_MPEGTSI_SetIDMAIntClear( U32 ChannelIndex );

//@}

//------------------------------------------------------------------------------
/// @name   Module Function Interface
//@{
void    NX_MPEGTSI_SetCapEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetCapEnable( U32 CapIdx );
void    NX_MPEGTSI_SetBypassEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetBypassEnable( U32 CapIdx );
void    NX_MPEGTSI_SetSerialEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetSerialEnable( U32 CapIdx );
void    NX_MPEGTSI_SetTCLKPolarityEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTCLKPolarityEnable( U32 CapIdx );
void    NX_MPEGTSI_SetTDPPolarityEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTDPPolarityEnable( U32 CapIdx );
void    NX_MPEGTSI_SetTSYNCPolarityEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTSYNCPolarityEnable( U32 CapIdx );
void    NX_MPEGTSI_SetTERRPolarityEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTERRPolarityEnable( U32 CapIdx );
void    NX_MPEGTSI_SetCapIntLockEnable( U32 CapIdx, CBOOL Enable );
void    NX_MPEGTSI_SetCapSramWakeUp( U32 CapIdx, CBOOL WakeOn );
CBOOL   NX_MPEGTSI_GetCapSramWakeUp( U32 CapIdx );
void    NX_MPEGTSI_SetCapSramPowerEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetCapSramPowerEnable( U32 CapIdx );

void    NX_MPEGTSI_SetCap1OutputEnable( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetCap1OutputEnable( void );
void    NX_MPEGTSI_SetCap1OutTCLKPolarityEnable( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetCap1OutTCLKPolarityEnable( void );
CBOOL   NX_MPEGTSI_GetCap1OutPolarityEnable( void );

void    NX_MPEGTSI_SetCapIntLockEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetCapIntLockEnable( U32 CapIdx );
void    NX_MPEGTSI_SetCapIntEnable( U32 CapIdx, CBOOL Enable );
CBOOL   NX_MPEGTSI_GetCapIntEnable( U32 CapIdx );
void    NX_MPEGTSI_SetCapIntMaskClear( U32 CapIdx, CBOOL Unmask );
CBOOL   NX_MPEGTSI_GetCapIntMaskClear( U32 CapIdx );
void    NX_MPEGTSI_SetCapIntClear( U32 CapIdx );
CBOOL   NX_MPEGTSI_GetCapIntStatus( U32 CapIdx );
U32     NX_MPEGTSI_GetCapFifoData( U32 CapIdx  );
void    NX_MPEGTSI_SetCPUWrData( U32 WrData );
U32     NX_MPEGTSI_GetCPUWrData( void );
void    NX_MPEGTSI_SetCPUWrAddr( U32 WrAddr );
void    NX_MPEGTSI_SetTsiEnable( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTsiEnable( void );
void    NX_MPEGTSI_SetTsiEncrypt( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTsiEncrypt( void );
void    NX_MPEGTSI_SetTsiSramWakeUp( CBOOL WakeUp );
CBOOL   NX_MPEGTSI_GetTsiSramWakeUp( void );
void    NX_MPEGTSI_SetTsiSramPowerEnable( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTsiSramPowerEnable( void );
void    NX_MPEGTSI_SetTsiIntEnable( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTsiIntEnable( void );
void    NX_MPEGTSI_SetTsiIntMaskClear( CBOOL Enable );
CBOOL   NX_MPEGTSI_GetTsiIntMaskClear( void );
void    NX_MPEGTSI_SetTsiIntClear( void );
CBOOL   NX_MPEGTSI_GetTsiIntStatus( void );
U32     NX_MPEGTSI_GetCapData( U32 CapIdx );
U32     NX_MPEGTSI_GetTsiOutData( void );

U32     NX_MPEGTSI_ByteSwap( U32 Data );
void    NX_MPEGTSI_WritePID( U32 Bank, U32 Type, U32 PidAddr, U32 PidData );
void    NX_MPEGTSI_WriteAESKEYIV( U32 CwAddr, U32 Cw0, U32 Cw1, U32 Cw2, U32 Cw3,
                                U32 Iv0, U32 Iv1, U32 Iv2, U32 Iv3 );
void    NX_MPEGTSI_WriteCASCW( U32 CwAddr, U32 Cw0, U32 Cw1, U32 Cw2, U32 Cw3 );
//@}


#ifdef	__cplusplus
}
#endif


#endif // __NX_MPEGTSI_H__

