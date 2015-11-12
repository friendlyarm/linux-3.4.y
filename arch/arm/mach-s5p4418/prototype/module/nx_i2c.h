//------------------------------------------------------------------------------
//
//  Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//  Module      : I2C
//  File        : nx_i2c.h
//  Description :
//  Author      : Firmware Team
//  History     :
//------------------------------------------------------------------------------
#ifndef __NX_I2C_H__
#define __NX_I2C_H__

#include "../base/nx_prototype.h"

#ifdef  __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup   I2C I2C
//------------------------------------------------------------------------------
//@{

    /// @brief  I2C Module's Register List
    struct  NX_I2C_RegisterSet
    {
        volatile U32 ICCR;                  ///< 0x00 : I2C Control Register
        volatile U32 ICSR;                  ///< 0x04 : I2C Status Register
        volatile U32 IAR;                   ///< 0x08 : I2C Address Register
        volatile U32 IDSR;                  ///< 0x0C : I2C Data Register
        volatile U32 STOPCON;               ///< 0x10 : I2C Stop Control Register
    };

    /// @brief Select I2C Mode
    typedef enum
    {
        NX_I2C_TXRXMODE_SLAVE_RX    = 0,    ///< Slave Receive Mode
        NX_I2C_TXRXMODE_SLAVE_TX    = 1,    ///< Slave Transmit Mode
        NX_I2C_TXRXMODE_MASTER_RX   = 2,    ///< Master Receive Mode
        NX_I2C_TXRXMODE_MASTER_TX   = 3     ///< Master Transmit Mode

    } NX_I2C_TXRXMODE ;

    /// @brief  Start/Stop signal
    typedef enum
    {
        NX_I2C_SIGNAL_STOP  = 0,            ///< Stop signal generation
        NX_I2C_SIGNAL_START = 1             ///< Start signal generation

    } NX_I2C_SIGNAL ;

//------------------------------------------------------------------------------
/// @name   Module Interface
//@{
CBOOL   NX_I2C_Initialize( void );
U32     NX_I2C_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
/// @name   Basic Interface
//@{
U32     NX_I2C_GetPhysicalAddress( U32 ModuleIndex );
U32     NX_I2C_GetSizeOfRegisterSet( void );
void    NX_I2C_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*    NX_I2C_GetBaseAddress( U32 ModuleIndex );
CBOOL   NX_I2C_OpenModule( U32 ModuleIndex );
CBOOL   NX_I2C_CloseModule( U32 ModuleIndex );
CBOOL   NX_I2C_CheckBusy( U32 ModuleIndex );
CBOOL   NX_I2C_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
/// @name   Interrupt Interface
//@{
S32     NX_I2C_GetInterruptNumber( U32 ModuleIndex );

void    NX_I2C_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable );
CBOOL   NX_I2C_GetInterruptEnable( U32 ModuleIndex, S32 IntNum );
CBOOL   NX_I2C_GetInterruptPending( U32 ModuleIndex, S32 IntNum );
void    NX_I2C_ClearInterruptPending( U32 ModuleIndex, S32 IntNum );

void    NX_I2C_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_I2C_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL   NX_I2C_GetInterruptPendingAll( U32 ModuleIndex );
void    NX_I2C_ClearInterruptPendingAll( U32 ModuleIndex );

void    NX_I2C_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32     NX_I2C_GetInterruptEnable32( U32 ModuleIndex );
U32     NX_I2C_GetInterruptPending32( U32 ModuleIndex );
void    NX_I2C_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );

S32     NX_I2C_GetInterruptPendingNumber( U32 ModuleIndex );    // -1 if None
//@}

//------------------------------------------------------------------------------
/// @name   Clock Control Interface
//@{
U32     NX_I2C_GetClockNumber( U32 ModuleIndex );
U32     NX_I2C_GetResetNumber( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name Configuration Function
//--------------------------------------------------------------------------
//@{
void    NX_I2C_SetClockPrescaler( U32 ModuleIndex, U32 PclkDivider, U32 Prescaler );
void    NX_I2C_GetClockPrescaler( U32 ModuleIndex, U32* pPclkDivider, U32* pPrescaler );
void    NX_I2C_SetSlaveAddress( U32 ModuleIndex, U8 Address);
 U32    NX_I2C_GetSlaveAddress( U32 ModuleIndex );

//void    NX_I2C_SetDataDelay( U32 ModuleIndex, U32 delay );
//U32     NX_I2C_GetDataDelay (U32 ModuleIndex);
//@}

//------------------------------------------------------------------------------
/// @name Operation Function
//------------------------------------------------------------------------------
//@{
void    NX_I2C_SetAckGenerationEnable( U32 ModuleIndex, CBOOL bAckGen );
CBOOL   NX_I2C_GetAckGenerationEnable( U32 ModuleIndex );
//void  NX_I2C_ClearOperationHold   ( U32 ModuleIndex );
void    NX_I2C_ControlMode ( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode, NX_I2C_SIGNAL Signal );
CBOOL   NX_I2C_IsBusy( U32 ModuleIndex );
void    NX_I2C_WriteByte (U32 ModuleIndex, U8 WriteData);
U8      NX_I2C_ReadByte ( U32 ModuleIndex );
void    NX_I2C_BusDisable( U32 ModuleIndex );
void    NX_I2C_NotAckGen( U32 ModuleIndex );
void    NX_I2C_DataLineRelease( U32 ModuleIndex );
void    NX_I2C_ClockLineRelease( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
/// @name Checking Function ( extra Interrupt source )
//------------------------------------------------------------------------------
//@{
CBOOL   NX_I2C_IsSlaveAddressMatch( U32 ModuleIndex );
//void  NX_I2C_ClearSlaveAddressMatch( U32 ModuleIndex );
//CBOOL NX_I2C_IsGeneralCall( U32 ModuleIndex );
//void  NX_I2C_ClearGeneralCall( U32 ModuleIndex );
//CBOOL NX_I2C_IsSlaveRxStop( U32 ModuleIndex );
//void  NX_I2C_ClearSlaveRxStop( U32 ModuleIndex );
CBOOL   NX_I2C_IsBusArbitFail( U32 ModuleIndex );
CBOOL   NX_I2C_IsACKReceived( U32 ModuleIndex );
CBOOL   NX_I2C_IsTxMode( U32 ModuleIndex );
//@}

//@}

#ifdef  __cplusplus
}
#endif

#endif // __NX_I2C_H__
