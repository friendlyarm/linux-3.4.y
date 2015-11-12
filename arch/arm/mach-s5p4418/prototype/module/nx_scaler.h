//------------------------------------------------------------------------------
//  Copyright (C) 2009 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : scaler
//  File        : nx_scaler.h
//  Description :
//  Author      : Firmware Team
//  History     :
//------------------------------------------------------------------------------
#ifndef __NX_SCALER_H__
#define __NX_SCALER_H__

#include "../base/nx_prototype.h"

#ifdef  __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup   SCALER  SCALER
//------------------------------------------------------------------------------
//@{

    /// @brief  SCALER Module's Register List
    struct NX_SCALER_RegisterSet
    {
        volatile U32    SCRUNREG;           ///< 0x00           : Scaler Configuration Register0
        volatile U32    SCCFGREG;           ///< 0x04           : Scaler Configuration Register1
        volatile U32    SCINTREG;           ///< 0x08           : Scaler Interrupt Register
        volatile U32    SCSRCADDREG;        ///< 0x0C           : Scaler Source Address Register
        volatile U32    SCSRCSTRIDE;        ///< 0x10           : Scaler Source Stride Register
        volatile U32    SCSRCSIZEREG;       ///< 0x14           : Scaler Source Size Register
        volatile U32    SCDESTADDREG0;      ///< 0x18           : Scaler Destination Address Register
        volatile U32    SCDESTSTRIDE0;      ///< 0x1C           : Scaler Destination Stride Register
        volatile U32    SCDESTADDREG1;      ///< 0x20           : Scaler Destination Address Register
        volatile U32    SCDESTSTRIDE1;      ///< 0x24           : Scaler Destination Stride Register
        volatile U32    SCDESTSIZEREG;      ///< 0x28           : Scaler Destination Size Register
        volatile U32    DELTAXREG;          ///< 0x2C           : Scaler Delta X Register
        volatile U32    DELTAYREG;          ///< 0x30           : Scaler Delta Y Register
        volatile U32    HVSOFTREG;          ///< 0x34           : Horizontal and Vertical Filter Ratio Register
        volatile U32    CMDBUFADDR;         ///< 0x3C           : Scaler Command Buffer Address Register 
        volatile U32    CMDBUFCON;          ///< 0x38           : Scaler Command Buffer Control Register
        volatile U32    YVFILTER[3][8];     ///< 0x40 ~ 0x5C
        volatile U32    __Reserved00[24];
        volatile S32    YHFILTER[5][32];    ///< 0xA0 ~ 0x11C
    };

    /// @brief  SCALER interrupt for interrupt interface
    enum
    {
        NX_SCALER_INT_DONE = 0,      ///< Scaler Done interrupt.
        NX_SCALER_INT_CMD_PROC = 1,  ///< Scaler Command Processor Interrupt
    };

    /// @brief  Rotate modes
    /*
    typedef enum
    {
        NX_SCALER_ROTATE_0          = 0x00UL,       ///< Rotate 0
        NX_SCALER_ROTATE_0_HFLIP    = 0x01UL,       ///< Rotate 0 and Horizontal Inversion
        NX_SCALER_ROTATE_180_HFLIP  = 0x02UL,       ///< Rotate 180 and Horizontal Inversion
        NX_SCALER_ROTATE_180        = 0x03UL        ///< Rotate 180
    }   NX_SCALER_ROTATE;
    */

    typedef enum
    {
        NX_SCALER_MODE_NORMAL   = 0x00UL,   //  8 bpp normal mode
        NX_SCALER_MODE_CBCR     = 0x01UL,   //  CbCr Interleaved mode
    }   NX_SCALER_MODE;

//------------------------------------------------------------------------------
/// @name   Module Interface
//@{
CBOOL   NX_SCALER_Initialize( void );
U32     NX_SCALER_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
/// @name   Basic Interface
//@{
U32     NX_SCALER_GetPhysicalAddress( U32 ModuleIndex );
U32     NX_SCALER_GetSizeOfRegisterSet( void );
void    NX_SCALER_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*    NX_SCALER_GetBaseAddress( U32 ModuleIndex );
CBOOL   NX_SCALER_OpenModule( U32 ModuleIndex );
CBOOL   NX_SCALER_CloseModule( U32 ModuleIndex );
CBOOL   NX_SCALER_CheckBusy( U32 ModuleIndex );
CBOOL   NX_SCALER_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
/// @name   Interrupt Interface
//@{
S32     NX_SCALER_GetInterruptNumber( U32 ModuleIndex );

void    NX_SCALER_SetInterruptEnable( U32 ModuelIndex, S32 IntNum, CBOOL Enable );
CBOOL   NX_SCALER_GetInterruptEnable( U32 ModuleIndex, S32 IntNum );
CBOOL   NX_SCALER_GetInterruptPending( U32 ModuleIndex, S32 IntNum );
void    NX_SCALER_ClearInterruptPending( U32 ModuleIndex, S32 IntNum );

void    NX_SCALER_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SCALER_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL   NX_SCALER_GetInterruptPendingAll( U32 ModuleIndex );
void    NX_SCALER_ClearInterruptPendingAll( U32 ModuleIndex );

void    NX_SCALER_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32     NX_SCALER_GetInterruptEnable32( U32 ModuleIndex );
U32     NX_SCALER_GetInterruptPending32( U32 ModuleIndex );
void    NX_SCALER_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );

S32     NX_SCALER_GetInterruptPendingNumber( U32 ModuleIndex ); // -1 if None
//@}

//------------------------------------------------------------------------------
/// @name   Clock Control Interface - 여기는 추후에 빼버릴 예정
//@{
//void        NX_SCALER_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode );
//NX_PCLKMODE NX_SCALER_GetClockPClkMode( U32 ModuleIndex );
//void        NX_SCALER_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode );
//NX_BCLKMODE NX_SCALER_GetClockBClkMode( U32 ModuleIndex );
U32         NX_SCALER_GetClockNumber( U32 ModuleIndex );
U32         NX_SCALER_GetResetNumber( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name Operation Function of Scaler
//@{
void    NX_SCALER_Run( U32 ModuleIndex );
void    NX_SCALER_Stop( U32 ModuleIndex );
CBOOL   NX_SCALER_IsBusy( U32 ModuleIndex );

void    NX_SCALER_SetFilterEnable( U32 ModuleIndex, CBOOL enable );
CBOOL   NX_SCALER_GetFilterEnable( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name Configuration Function of Scaler
//@{
void    NX_SCALER_SetFilterRatio( U32 ModuleIndex, U32 RatioH, U32 RatioV );
void    NX_SCALER_GetFilterRatio( U32 ModuleIndex, U32* RatioH, U32* RatioV );

void    NX_SCALER_SetYVFilter( U32 ModuleIndex, U32 FilterSel, U32 FilterIndex, U32 FilterVal );
void    NX_SCALER_SetYHFilter( U32 ModuleIndex, U32 FilterSel, U32 FilterIndex, U32 FilterVal );

//void                NX_SCALER_SetRotateMode( U32 ModuleIndex, NX_SCALER_ROTATE rotate );
//NX_SCALER_ROTATE    NX_SCALER_GetRotateMode( U32 ModuleIndex );
void            NX_SCALER_SetMode( U32 ModuleIndex, NX_SCALER_MODE mode );
NX_SCALER_MODE  NX_SCALER_GetMode( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name Configuration Function of Image Size and Address
//@{
void    NX_SCALER_SetSrcAddr( U32 ModuleIndex, U32 Addr );
U32     NX_SCALER_GetSrcAddr( U32 ModuleIndex );
void    NX_SCALER_SetSrcStride( U32 ModuleIndex, U32 Stride );
U32     NX_SCALER_GetSrcStride( U32 ModuleIndex );

void    NX_SCALER_SetDestAddr( U32 ModuleIndex, U32 DstIndex, U32 Addr );
U32     NX_SCALER_GetDestAddr( U32 ModuleIndex, U32 DstIndex );
void    NX_SCALER_SetDestStride( U32 ModuleIndex, U32 DstIndex, U32 Stride );
U32     NX_SCALER_GetDestStride( U32 ModuleIndex, U32 DstIndex );

void    NX_SCALER_SetImageSize( U32 ModuleIndex, U32 dwSrcWidth, U32 dwSrcHeight, U32 dwDestWidth, U32 dwDestHeight );
void    NX_SCALER_GetImageSize( U32 ModuleIndex, U32 *pdwSrcWidth, U32   *pdwSrcHeight, U32  *pdwDestWidth, U32  *pdwDestHeight );
//@}

//--------------------------------------------------------------------------
/// @name Configuration Function of Command Buffer
//@{
void    NX_SCALER_SetCmdBufAddr( U32 ModuleIndex, U32 Addr );
U32     NX_SCALER_GetCmdBufAddr( U32 ModuleIndex );
void    NX_SCALER_RunCmdBuf( U32 ModuleIndex );
void    NX_SCALER_StopCmdBuf( U32 ModuleIndex );
//@}

U32 NX_SCALER_CLKGEN_GetPhysicalAddress(U32 ModuleIndex);
void NX_SCALER_CLKGEN_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress);
void NX_SCALER_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode );
//@}

#ifdef  __cplusplus
}
#endif

#endif // __NX_SCALER_H__

