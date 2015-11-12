//------------------------------------------------------------------------------
//  Copyright (C) 2012 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : SDMMC
//  File        : nx_SDMMC.c
//  Description :
//  Author      : sei (parkjh@nexell.com)
//  History     :
//------------------------------------------------------------------------------

#include "nx_chip.h"
#include "nx_sdmmc.h"

//------------------------------------------------------------------------------
// Internal global varialbles
//------------------------------------------------------------------------------
static struct
{
    struct NX_SDMMC_RegisterSet *pRegister;
} __g_ModuleVariables[NUMBER_OF_SDMMC_MODULE] = { {CNULL,}, };


//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Initialize prototype environment and local variables.
 *  @return     CTRUE indicates success.
 *              CFALSE indicates failure.
 *  @remark     You have to call this function before using other functions.
 *  @see        NX_SDMMC_GetNumberOfModule
 */
CBOOL   NX_SDMMC_Initialize( void )
{
    static CBOOL bInit = CFALSE;
    U32 i;

    if( CFALSE == bInit )
    {
        for( i=0; i<NUMBER_OF_SDMMC_MODULE; i++ )
        {
            __g_ModuleVariables[i].pRegister = CNULL;
        }

        bInit = CTRUE;
    }

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get number of modules in the chip.
 *  @return     number of module in the chip.
 *  @see        Nx_SDMMC_Initialize
 */
U32     NX_SDMMC_GetNumberOfModule( void )
{
    return NUMBER_OF_SDMMC_MODULE;
}


//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get a physical address of mudule.
 *  @param[in]  none.
 *  @return     a physical address of module.
 */
U32     NX_SDMMC_GetPhysicalAddress( U32 ModuleIndex )
{
    static const U32 SDMMCPhysicalAddr[NUMBER_OF_SDMMC_MODULE] =
    {
        PHY_BASEADDR_LIST( SDMMC )
    };

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    return  (U32)SDMMCPhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a size, in bytes, of register set.
 *  @return     Byte size of module's register set.
 */
U32     NX_SDMMC_GetSizeOfRegisterSet( void )
{
    return sizeof( struct NX_SDMMC_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a base address of register set.
 *  @param[in]  BaseAddress Module's base address
 *  @return     None.
 *  @remark     You have to call this function before using NX_SDMMC_OpenModule(),
 *              SDMMC_CloseModule(), NX_SDMMC_CheckBusy(), NX_SDMMC_CanPowerDown(),
 *              Interrupt Interfaces, DMA Interfaces, Clock Control Interfaces,
 *              and Module Specific Functions.
 *              You can use this function with:
 *              - virtual address system such as WinCE or Linux.
 *  @code
 *      U32 PhyAddr, VirAddr;
 *      PhyAddr = NX_SDMMC_GetPhysicalAddress( index );
 *      VirAddr = MEMORY_MAP_FUNCTION_TO_VIRTUAL( PhyAddr, ... );
 *      NX_SDMMC_SetBaseAddress( index, VirAddr );
 *  @endcode
 *              - physical address system such as Non-OS.
 *  @code
 *      NX_SDMMC_SetBaseAddress( index, NX_SDMMC_GetPhysicalAddress( index ) );
 *  @endcode
 *
 */
void    NX_SDMMC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != BaseAddress );

    __g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SDMMC_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a base address of register set
 *  @param[in]  none.
 *  @return     Module's base address.

 */
void*    NX_SDMMC_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Initialize selected modules with default value.
 *  @param[in]  none.
 *  @return     CTRUE indicates success.
 *              CFALSE indicates failure.
 *  @remark     This function makes a module is initialized by default value.
 *              Thererfore after calling this function, some registers may be
 *              modified to default state. If you don't want to change any
 *              setting of registers, you can skip to call this function. But
 *              you have to call this function at least once to use these
 *              prototype functions after Power-On-Reset or SDMMC_CloseModule().
 *              IMPORTANT : you have to enable a PCLK before calling this function.
 *  @code
 *      // Initialize the clock generator
 *      NX_SDMMC_SetClockPClkMode( index, NX_PCLKMODE_ALWAYS );
 *      NX_SDMMC_SetClockSource( index, 0, NX_SDMMC_CLKSRC_PCLK );
 *      NX_SDMMC_SetClockDivisor( index, 0, n );    // n = 1 ~ 64
 *      NX_SDMMC_SetClockDivisorEnable( index, CTRUE );
 *
 *      NX_SDMMC_OpenModule( index );
 *
 *      // ......
 *      // Do something
 *      // ......
 *
 *      NX_SDMMC_CloseModule( index );
 *  @endcode

 */
CBOOL   NX_SDMMC_OpenModule( U32 ModuleIndex )
{
    const U32 INT_ENA = 1UL<<4;
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue = ReadIO32(&pRegister->CTRL) | INT_ENA;
    WriteIO32(&pRegister->CTRL, SetValue);      // global interrupt enable bit for SDMMC module.

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      De-initialize selected module to the proper stage.
 *  @param[in]  none.
 *  @return     CTRUE indicates success.
 *              CFALSE indicates failure.
 *  @see        NX_SDMMC_GetPhysicalAddress,    NX_SDMMC_GetSizeOfRegisterSet,
 *              NX_SDMMC_SetBaseAddress,        NX_SDMMC_GetBaseAddress,
 *              NX_SDMMC_OpenModule,
 *              NX_SDMMC_CheckBusy,             NX_SDMMC_CanPowerDown
 */
CBOOL   NX_SDMMC_CloseModule( U32 ModuleIndex )
{
    const U32 INT_ENA = 1UL<<4;
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue  = ReadIO32(&pRegister->CTRL) & ~INT_ENA;
    WriteIO32(&pRegister->CTRL, SetValue);      // global interrupt disable bit for SDMMC module.

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the selected modules is busy or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates the selected module is busy.
 *              CFALSE  indicates the selected module is idle.
 *  @remark     If the command FSM state of SDMMC is idle, it returns CFALSE.
 */
CBOOL   NX_SDMMC_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    if( NX_SDMMC_CMDFSM_IDLE == NX_SDMMC_GetCommandFSM(ModuleIndex) )
        return CFALSE;

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicaes whether the selected modules is ready to enter power-down stage.
 *  @param[in]  none.
 *  @return     CTRUE indicates the selected module is ready to enter power-down stage.
 *              CFALSE indicates the selected module is busy and not ready to enter power-down stage.
 *  @remark     If the command FSM state of SDMMC is idle, it returns CTRUE.
 */
CBOOL   NX_SDMMC_CanPowerDown( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    if( NX_SDMMC_CheckBusy(ModuleIndex) )
        return CFALSE;

    return CTRUE;
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get an interrupt number for the interrupt controller.
 *  @param[in]  none.
 *  @return     An interrupt number.
 *  @remark     Return value can be used for the interrupt controller module's
 *              functions.
 */
S32     NX_SDMMC_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 SDMMCInterruptNumber[NUMBER_OF_SDMMC_MODULE] = { INTNUM_LIST( SDMMC ) };

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    return  (S32)SDMMCInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enabled or disabled.
 *  @param[in]  IntNum          Specifies an interrupt number which is one of @ref NX_SDMMC_INT enum.
 *  @param[in]  Enable          Set it as CTRUE to enable an interrupt specified by @a IntNum. 
 *                              Set it as CFALSE to diable an interrupt specified by @a IntNum.
 *  @return     None.
 */
void    NX_SDMMC_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( (1<=IntNum) && (16>=IntNum) );
    NX_ASSERT( (0==Enable) || ( 1==Enable) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->INTMASK);
    regval &= ~(1UL<<IntNum);
    regval |= ((U32)Enable)<<IntNum;

    WriteIO32(&pRegister->INTMASK, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]  IntNum          Specifies an interrupt number which is one of @ref NX_SDMMC_INT enum.
 *  @return     CTRUE   indicates an interrupt specified by @a IntNum is enabled.
 *              CFALSE  indicates an interrupt specified by @a IntNum is disabled.
 */
CBOOL   NX_SDMMC_GetInterruptEnable( U32 ModuleIndex, S32 IntNum )
{
    NX_ASSERT( (1<=IntNum) && (16>=IntNum) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTMASK) >> IntNum) & 1);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not.
 *  @param[in]  IntNum          Specifies an interrupt number which is one of @ref NX_SDMMC_INT enum.
 *  @return     CTRUE   indicates an interrupt specified by @a IntNum is pended.
 *              CFALSE  indicates an interrupt specified by @a IntNum is not pended.
 *  @remark     The interrupt pending status are logged regardless of interrupt
 *              enable status. Therefore the return value can be CTRUE even
 *              though the specified interrupt has been disabled.
 */
CBOOL   NX_SDMMC_GetInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    NX_ASSERT( (1<=IntNum) && (16>=IntNum) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RINTSTS) >> IntNum) & 1);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  IntNum          Specifies an interrupt number which is one of @ref NX_SDMMC_INT enum.
 *  @return     None.
 */
void    NX_SDMMC_ClearInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    NX_ASSERT( (1<=IntNum) && (16>=IntNum) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RINTSTS, (U32)1<<IntNum);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set all interrupts to be enabled or disabled.
 *  @param[in]  bEnb            Set it as CTRUE to enable all interrupts. 
 *                              Set it as CFALSE to disable all interrupts.
 *  @return     None.
 */
void    NX_SDMMC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
    register U32 SetValue;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( Enable )    SetValue = 0x1FFFE; // 1 to 16
    else            SetValue = 0;

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTMASK, SetValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enabled or not.
 *  @param[in]  none.
 *  @return     CTURE   indicates there's interrupts which are enabled.
 *              CFALSE  indicates there's no interrupt which are enabled.
 */
CBOOL   NX_SDMMC_GetInterruptEnableAll( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTMASK) )
        return CTRUE;

    return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of unmasked interrupts are pended or not.
 *  @param[in]  none.
 *  @return     CTURE   indicates there's unmasked interrupts which are pended.
 *              CFALSE  indicates there's no unmasked interrupt which are pended.
 *  @remark     Since this function doesn't consider about pending status of
 *              interrupts which are disabled, the return value can be CFALSE
 *              even though some interrupts are pended unless a relevant
 *              interrupt is enabled.
 */
CBOOL   NX_SDMMC_GetInterruptPendingAll( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MINTSTS) )
        return CTRUE;

    return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear pending state of all interrupts.
 *  @param[in]  none.
 *  @return     None.
 */
void    NX_SDMMC_ClearInterruptPendingAll( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RINTSTS, 0xFFFFFFFF);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get an interrupt number which has the most prority of pended interrupts.
 *  @param[in]  none.
 *  @return     an interrupt number which has the most priority of pended and
 *              unmasked interrupts. This value is one of @ref NX_SDMMC_INT enum.
 *              If there's no interrupt which is pended and unmasked, it returns -1.
 */
S32     NX_SDMMC_GetInterruptPendingNumber( U32 ModuleIndex )
{
    register S32 i;
    register U32 pend;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pend = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MINTSTS);

    if( 0 == pend )
        return -1;

    for( i=0 ;; i++, pend >>= 1 )
    {
        NX_ASSERT( i < 32 );
        if( pend & 1 )
            break;
    }

    NX_ASSERT( (1<=i) && (16>=i) );
    return i;
}


//------------------------------------------------------------------------------
/**
 *  @brief      Set interrupts to be enabled or disabled.
 *  @param[in]  EnableFlag      Specifies an interrupt enable flag that each bit
 *                              represents an interrupt enable status to be changed -
 *                              Value of 0 masks interrupt and value of 1 enables
 *                              interrupt. EnableFlag[16:1] are only valid.
 *  @return     None.
 */
void    NX_SDMMC_SetInterruptEnable32 ( U32 ModuleIndex, U32 EnableFlag )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTMASK, EnableFlag & 0x1FFFE);   // 1 to 16
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get an interrupt enable status.
 *  @param[in]  none.
 *  @return     An interrupt enable status that each bit represents current
 *              interrupt enable status - Value of 0 indicates relevant interrupt
 *              is masked and value of 1 indicates relevant interrupt is enabled.
 */
U32 NX_SDMMC_GetInterruptEnable32 ( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTMASK);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get an interrupt pending status.
 *  @param[in]  none.
 *  @return     An interrupt pending status that each bit represents current
 *              interrupt pending status - Value of 0 indicates relevant interrupt
 *              is not pended and value of 1 indicates relevant interrupt is pended.
 */
U32 NX_SDMMC_GetInterruptPending32 ( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RINTSTS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear an interrupt pending status.
 *  @param[in]  PendingFlag     Specifies an interrupt pending clear flag. An
 *                              interrupt pending status cleared only if
 *                              corresponding bit in PendingFlag is set.
 *  @return     None.
 */
void    NX_SDMMC_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RINTSTS, PendingFlag);
}


//------------------------------------------------------------------------------
// Clock Control Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *  @brief      Get current module clock number
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current clock number
 *  @remarks
 */
U32     NX_SDMMC_GetClockNumber( U32 ModuleIndex )
{
    // PHY_BASEADDR_CLKGEN_MODULE
    static const U32 CLKGEN_SDMMCLIST[] =
    {
        CLOCKINDEX_LIST( SDMMC )
    };

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    return (U32)CLKGEN_SDMMCLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get current reset number
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current reset number
 *  @remarks
 */
U32     NX_SDMMC_GetResetNumber( U32 ModuleIndex )
{
    const U32 ResetPortNumber[NUMBER_OF_SDMMC_MODULE] =
    {
        RESETINDEX_LIST( SDMMC, i_nRST )
    };

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );

    return (U32)ResetPortNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// SDMMC Specific Interfaces
//------------------------------------------------------------------------------

void    NX_SDMMC_SetPowerEnable( U32 ModuleIndex, U32 PowerIndex, CBOOL Enable )
{
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	NX_ASSERT( CNULL != pRegister );

    SetValue  = ReadIO32(&pRegister->PWREN);
    SetValue &= ~(1<<PowerIndex);
    SetValue |= (Enable<<PowerIndex);

    WriteIO32(&pRegister->PWREN, SetValue);
}

CBOOL   NX_SDMMC_GetPowerEnable( U32 ModuleIndex, U32 PowerIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->PWREN))>>PowerIndex) & 0x1 );
}

/**
 *  @brief      Abort Read Data for SDIO suspend sequence.
 *  @param[in]  none.
 *  @return     None.
 *  @remark     If the read data transfer is suspended by the user, the user
 *              should call NX_SDMMC_AbortReadData() to reset the data state
 *              machine, which is waiting for next block of data.
 */
void    NX_SDMMC_AbortReadData( U32 ModuleIndex )
{
    const U32 ABORT_RDATA = 1UL<<8;
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue = ReadIO32(&pRegister->CTRL) | ABORT_RDATA;
    WriteIO32(&pRegister->CTRL, SetValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Send IRQ response for MMC interrupt mode.
 *  @param[in]  none.
 *  @return     None.
 *  @remark     To wait for MMC card interrupt, the user issues CMD40, and SDMMC
 *              waits for interrupt response from MMC card. In meantime, if the
 *              user wants SDMMC to exit waiting for interrupt state, the user
 *              should call NX_SDMMC_SendIRQResponse() to send CMD40 response
 *              on bus and SDMMC returns to idle state.
 */
void    NX_SDMMC_SendIRQResponse( U32 ModuleIndex )
{
    const U32 SEND_IRQ_RESP = 1UL<<7;
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue = ReadIO32(&pRegister->CTRL) | SEND_IRQ_RESP;
    WriteIO32(&pRegister->CTRL, SetValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Assert or clear Read Wait signal to SDIO card.
 *  @param[in]  none.
 *  @param[in]  bAssert         Set it as CTRUE to assert Read-Wait signal to SD card.
 *                              Set it as CFALSE to clear Read-Wait signal to SD card.
 *  @return     None.
 *  @remark     Read-Wait is used with only the SDIO card and can temporarily
 *              stall the data transfer - either from function or memory - and
 *              allow the user to send commands to any function within the SDIO
 *              device.
 */
void    NX_SDMMC_SetReadWait( U32 ModuleIndex, CBOOL bAssert )
{
    const U32 READ_WAIT_POS = 6;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( (0==bAssert) || (1==bAssert) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(1UL<<READ_WAIT_POS);
    regval |= (U32)bAssert << READ_WAIT_POS;
    WriteIO32(&pRegister->CTRL, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Reset the DMA interface of SDMMC.
 *  @param[in]  none.
 *  @return     None.
 *  @remark     Reset the DMA interface abruptly terminates any DMA transer in
 *              progress. After calling NX_SDMMC_ResetDMA(), you have to wait
 *              until the DMA interface is in normal state.
 *              The example for this is as following.
 *  @code
 *      NX_SDMMC_ResetDMA( index );             // Reset the DMA interface.
 *      while( NX_SDMMC_IsResetDMA( index ) );  // Wait until the DMA reset is completed.
 *  @endcode
 */
void    NX_SDMMC_ResetDMAC( U32 ModuleIndex )
{
    const U32 DMARST    = 1UL<<2;
    const U32 FIFORST   = 1UL<<1;
    const U32 CTRLRST   = 1UL<<0;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(FIFORST | CTRLRST);
    regval |=   DMARST;
    WriteIO32(&pRegister->CTRL, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Enable/Disable DMA transfer mode.
 *  @param[in]  none.
 *  @param[in]  Enable          Set it as CTRUE to enable DMA transfer mode.
 *                              Set it as CFALSE to disable DMA transfer mode.
 *  @return     None.
 *  @see        NX_SDMMC_ResetDMA,  NX_SDMMC_IsResetDMA
 */
void    NX_SDMMC_SetDMAMode( U32 ModuleIndex, CBOOL Enable )
{
    const U32 DMA_ENA_POS = 5;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(1UL<<DMA_ENA_POS);
    regval |= (U32)Enable << DMA_ENA_POS;
    WriteIO32(&pRegister->CTRL, regval);
}

void NX_SDMMC_SetUseInternalDMAC( U32 ModuleIndex, CBOOL Enable )
{
    const U32 INDMA_ENA_POS = 25;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(1UL<<INDMA_ENA_POS);
    regval |= (U32)Enable << INDMA_ENA_POS;
    WriteIO32(&pRegister->CTRL, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a DMA request signal state.
 *  @param[in]  none.
 *  @return     CTRUE   indicates a DMA request signal is asserted.
 *              CFALSE  indicates a DAM request signal is cleared.
 */
CBOOL   NX_SDMMC_IsDMAReq( U32 ModuleIndex )
{
    const U32 DMAREQ_POS    = 31;
    const U32 DMAREQ_MASK   = 1UL << DMAREQ_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & DMAREQ_MASK) >> DMAREQ_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a DMA acknowledge signal state.
 *  @param[in]  none.
 *  @return     CTRUE   indicates a DMA acknowledge signal is asserted.
 *              CFALSE  indicates a DAM acknowledge signal is cleared.
 */
CBOOL   NX_SDMMC_IsDMAAck( U32 ModuleIndex )
{
    const U32 DMAACK_POS    = 30;
    const U32 DMAACK_MASK   = 1UL << DMAACK_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & DMAACK_MASK) >> DMAACK_POS);
}

CBOOL   NX_SDMMC_IsResetDMAC( U32 ModuleIndex )
{
    const U32 DMARST_POS   = 2;
    const U32 DMARST_MASK  = 1UL<<DMARST_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CTRL) & DMARST_MASK)>>DMARST_POS;
}


//------------------------------------------------------------------------------
/**
 *  @brief      Reset the FIFO.
 *  @param[in]  none.
 *  @return     None.
 *  @remark     Resets the FIFO pointers and counters of the FIFO.
 *              If any of the previous data commands do not properly terminate,
 *              the the user should the FIFO reset in order to remove any
 *              residual data, if any, in the FIFO.
 *              After calling NX_SDMMC_ResetFIFO(), you have to wait until the
 *              FIFO reset is completed. The example for this is as following.
 *  @code
 *      NX_SDMMC_ResetFIFO( index );                    // Reest the FIFO.
 *      while( NX_SDMMC_IsResetFIFO( index ) );     // Wait until the FIFO reset is completed.
 *  @endcode
 */
void    NX_SDMMC_ResetFIFO( U32 ModuleIndex )
{
    const U32 DMARST    = 1UL<<2;
    const U32 FIFORST   = 1UL<<1;
    const U32 CTRLRST   = 1UL<<0;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(DMARST | CTRLRST);
    regval |=   FIFORST;
    WriteIO32(&pRegister->CTRL, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the FIFO is in reset or normal state.
 *  @param[in]  none.
 *  @return     CTRUE   indicates reset of the FIFO is in pregress.
 *              CFALSE  indicates the FIFO is in normal state.
 */
CBOOL   NX_SDMMC_IsResetFIFO( U32 ModuleIndex )
{
    const U32 FIFORST_POS   = 1;
    const U32 FIFORST_MASK  = 1UL<<FIFORST_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CTRL) & FIFORST_MASK)>>FIFORST_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Reset the SDMMC internal logic.
 *  @param[in]  none.
 *  @return     None.
 *  @remark     This resets:
 *              - the Card Interface Unit and stete machine.
 *              - AbortReadData, SendIRQResponse and ReadWait control.
 *              - NX_SDMMC_CMD_STARTCMD state.
 *  @remark     After calling NX_SDMMC_ResetController(), you have to wait until
 *              the controller reset is completed. The example for this is as
 *              following.
 *  @code
 *      NX_SDMMC_ResetController( index );              // Reest the controller.
 *      while( NX_SDMMC_IsResetController( index ) );   // Wait until the controller reset is completed.
 *  @endcode
 */
void    NX_SDMMC_ResetController( U32 ModuleIndex )
{
    const U32 DMARST    = 1UL<<2;
    const U32 FIFORST   = 1UL<<1;
    const U32 CTRLRST   = 1UL<<0;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(DMARST | FIFORST);
    regval |=   CTRLRST;
    WriteIO32(&pRegister->CTRL, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the controller is in reset or normal state.
 *  @param[in]  none.
 *  @return     CTRUE   indicates reset of the controller is in pregress.
 *              CFALSE  indicates the controller is in normal state.
 */
CBOOL   NX_SDMMC_IsResetController( U32 ModuleIndex )
{
    const U32 CTRLRST_POS   = 0;
    const U32 CTRLRST_MASK  = 1UL<<CTRLRST_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CTRL) & CTRLRST_MASK)>>CTRLRST_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a divider value for an output clock.
 *  @param[in]  divider         a divider value to generate SDCLK, 1 ~ 510.
 *                              It must be 1 or a even number between 2 and 510.
 *  @return     None.
 *  @remark     a clock divider is used to select a freqeuncy of SDCLK.
 *              A clock source of this divider is an output clock of the clock
 *              generator. For example, if PCLK is 50Mhz, then the SDCLK of
 *              400 KHz can be output by following.
 *  @code
 *      // a source clock = 25 MHz, where PCLK is 50 MHz
 *      NX_SDMMC_SetClockSource( index, 0, NX_SDMMC_CLKSRC_PCLK );
 *      NX_SDMMC_SetClockDivisor( index, 0, 2 );
 *      NX_SDMMC_SetClockDivisorEnable( index, CTRUE );
 *
 *      // 25,000,000 / 400,000 = 62.5 <= 64 (even number)
 *      // actual SDCLK = 25,000,000 / 64 = 390,625 Hz
 *      NX_SDMMC_SetOutputClockDivider( index, 64 );
 *  @endcode
 *  @remark     The result of this function will be applied to the SDMMC module
 *              after calling function NX_SDMMC_SetCommand() with
 *              NX_SDMMC_CMDFLAG_UPDATECLKONLY flag.
 *              The user should not modify clock settings while a command is
 *              being executed.
 *              The following shows how to update clock settings.
 *  @code
 *      // 1. Confirm that no card is engaged in any transaction.
 *      //  If there's a transaction, wait until it finishes.
 *      while( NX_SDMMC_IsDataTransferBusy() );
 *
 *      // 2. Disable the output clock.
 *      NX_SDMMC_SetOutputClockEnable( CFALSE );
 *
 *      // 3. Program the clock divider as required.
 *      NX_SDMMC_SetOutputClockDivider( n );
 *
 *      // 4. Start a command with NX_SDMMC_CMDFLAG_UPDATECLKONLY flag.
 *      repeat_4 :
 *      NX_SDMMC_SetCommand( 0, NX_SDMMC_CMDFLAG_STARTCMD | NX_SDMMC_CMDFLAG_UPDATECLKONLY | NX_SDMMC_CMDFLAG_WAITPRVDAT );
 *
 *      // 5. Wait until a update clock command is taken by the SDMMC module.
 *      //  If a HLE is occurred, repeat 4.
 *      while( NX_SDMMC_IsCommandBusy() );
 *
 *      if( NX_SDMMC_GetInterruptPending( NX_SDMMC_INT_HLE ) )
 *      {
 *          NX_SDMMC_ClearInterruptPending( NX_SDMMC_INT_HLE );
 *          goto repeat_4;
 *      }
 *
 *      // 6. Enable the output clock.
 *      NX_SDMMC_SetOutputClockEnable( CTRUE );
 *
 *      // 7. Start a command with NX_SDMMC_CMDFLAG_UPDATECLKONLY flag.
 *      repeat_7 :
 *      NX_SDMMC_SetCommand( 0, NX_SDMMC_CMDFLAG_STARTCMD | NX_SDMMC_CMDFLAG_UPDATECLKONLY | NX_SDMMC_CMDFLAG_WAITPRVDAT );
 *
 *      // 8. Wait until a update clock command is taken by the SDMMC module.
 *      //  If a HLE is occurred, repeat 7.
 *      while( NX_SDMMC_IsCommandBusy() );
 *
 *      if( NX_SDMMC_GetInterruptPending( NX_SDMMC_INT_HLE ) )
 *      {
 *          NX_SDMMC_ClearInterruptPending( NX_SDMMC_INT_HLE );
 *          goto repeat_7;
 *      }
 *  @endcode
 */

void    NX_SDMMC_SetClockSource( U32 ModuleIndex, NX_SDMMC_CLOCK_SOURCE ClkSrc )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 3 >= ClkSrc );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKSRC, ClkSrc );  
}

NX_SDMMC_CLOCK_SOURCE    NX_SDMMC_GetClockSource( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (NX_SDMMC_CLOCK_SOURCE)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKSRC) >> 0) & 0x3);  
}


void    NX_SDMMC_SetOutputClockDivider( U32 ModuleIndex, U32 divider )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (1==divider) || (0==(divider&1)) ); // 1 or even number
    NX_ASSERT( (1<=divider) && (510>=divider) );   // 1 <= divider <= 510

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKDIV, divider>>1);     // 2*n divider (0 : bypass)
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a divider value for an output clock.
 *  @param[in]  none.
 *  @return     a divider value for an output clock.
 *  @see        NX_SDMMC_SetLowPowerClockMode,  NX_SDMMC_GetLowPowerClockMode,
 *              NX_SDMMC_SetOutputClockDivider,
 *              NX_SDMMC_SetOutputClockDivider, NX_SDMMC_GetOutputClockDivider
 */
U32     NX_SDMMC_GetOutputClockDivider( U32 ModuleIndex )
{
    register U32 divider;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    divider = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKDIV);

    return (divider) ? divider<<1 : 1;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set the Low-Power-Clock mode to be enabled or disabled.
 *  @param[in]  none.
 *  @param[in]  Enable          Set it as CTRUE to enable the Low-Power-Clock mode.
 *                              Set it as CFALSE to disable the Low-Power-Clock mode.
 *  @return     None.
 *  @remark     The Low-Power-Clock mode means it stops the output clock when
 *              card in IDLE (should be normally set to only MMC and SD memory
 *              card; for SDIO cards, if interrupts must be detected, the output
 *              clock should not be stopped).
 *              The result of this function will be applied to the SDMMC module
 *              after calling function NX_SDMMC_SetCommand() with
 *              NX_SDMMC_CMDFLAG_UPDATECLKONLY flag.
 *  @see                                        NX_SDMMC_GetLowPowerClockMode,
 *              NX_SDMMC_SetOutputClockEnable,  NX_SDMMC_GetOutputClockEnable,
 *              NX_SDMMC_SetOutputClockDivider, NX_SDMMC_GetOutputClockDivider
 */
void    NX_SDMMC_SetLowPowerClockMode( U32 ModuleIndex, CBOOL Enable )
{
    const U32 LOWPWR = 1UL<<16;
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue = ReadIO32(&pRegister->CLKENA);
    if( Enable )    SetValue |= LOWPWR;
    else            SetValue &= ~LOWPWR;
    WriteIO32(&pRegister->CLKENA, SetValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the Low-Power-Clock mode is enabled or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates the Low-Power-Clock mode is enabled.
 *              CFALSE  indicates the Low-Power-Clock mode is disabled.
 *  @see        NX_SDMMC_SetLowPowerClockMode,
 *              NX_SDMMC_SetOutputClockEnable,  NX_SDMMC_GetOutputClockEnable,
 *              NX_SDMMC_SetOutputClockDivider, NX_SDMMC_GetOutputClockDivider
 */
CBOOL   NX_SDMMC_GetLowPowerClockMode( U32 ModuleIndex )
{
    const U32 LOWPWR = 1UL<<16;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKENA) & LOWPWR) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set an output clock (SDCLK) to be enabled or disabled.
 *  @param[in]  none.
 *  @param[in]  Enable          Set it as CTRUE to enable an output clock (SDCLK).
 *                              Set it as CFALSE to disable an output clock (SDCLK).
 *  @return     None.
 *  @remark     The result of this function will be applied to the SDMMC module
 *              after calling function NX_SDMMC_SetCommand() with
 *              NX_SDMMC_CMDFLAG_UPDATECLKONLY flag.
 *  @see        NX_SDMMC_SetLowPowerClockMode,  NX_SDMMC_GetLowPowerClockMode,
 *                                              NX_SDMMC_GetOutputClockEnable,
 *              NX_SDMMC_SetOutputClockDivider, NX_SDMMC_GetOutputClockDivider
 */
void    NX_SDMMC_SetOutputClockEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 CLKENA = 1UL<<0;
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue = ReadIO32(&pRegister->CLKENA);
    if( Enable )    SetValue |= CLKENA;
    else            SetValue &= ~CLKENA;
    WriteIO32(&pRegister->CLKENA, SetValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether an output clock (SDCLK) is enabled or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates an output clock is enabled.
 *              CFALSE  indicates an output clock is disabled.
 *  @see        NX_SDMMC_SetLowPowerClockMode,  NX_SDMMC_GetLowPowerClockMode,
 *              NX_SDMMC_SetOutputClockEnable,
 *              NX_SDMMC_SetOutputClockDivider, NX_SDMMC_GetOutputClockDivider
 */
CBOOL   NX_SDMMC_GetOutputClockEnable( U32 ModuleIndex )
{
    const U32 CLKENA = 1UL<<0;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKENA) & CLKENA) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether an output clock (SDCLK) is enabled or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates an output clock is enabled.
 *              CFALSE  indicates an output clock is disabled.
 *  @see        NX_SDMMC_SetLowPowerClockMode,  NX_SDMMC_GetLowPowerClockMode,
 *              NX_SDMMC_SetOutputClockEnable,
 *              NX_SDMMC_SetOutputClockDivider, NX_SDMMC_GetOutputClockDivider
 */
void    NX_SDMMC_SetDriveClockShiftPhase( U32 ModuleIndex, NX_SDMMC_CLKSHIFT ClockShift )
{
    const U32 DCLKPHASE_POS     = 16;
    const U32 DCLKPHASE_MASK    = 3UL<<DCLKPHASE_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (ClockShift == NX_SDMMC_CLOCK_SHIFT_0  ) ||
               (ClockShift == NX_SDMMC_CLOCK_SHIFT_90 ) ||
               (ClockShift == NX_SDMMC_CLOCK_SHIFT_180) ||
               (ClockShift == NX_SDMMC_CLOCK_SHIFT_270) );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval  = ReadIO32(&pRegister->CLKCTRL);
    regval &= ~DCLKPHASE_MASK;
    regval |= ClockShift << DCLKPHASE_POS;
    WriteIO32(&pRegister->CLKCTRL, regval);
}

NX_SDMMC_CLKSHIFT   NX_SDMMC_GetDriveClockShiftPhase( U32 ModuleIndex )
{
    const U32 DCLKPHASE_POS     = 16;
    const U32 DCLKPHASE_MASK    = 3UL<<DCLKPHASE_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (NX_SDMMC_CLKSHIFT)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKCTRL) & DCLKPHASE_MASK)>>DCLKPHASE_POS);
}

void    NX_SDMMC_SetSampleClockShiftPhase( U32 ModuleIndex, NX_SDMMC_CLKSHIFT ClockShift )
{
    const U32 SCLKPHASE_POS     = 24;
    const U32 SCLKPHASE_MASK    = 3UL<<SCLKPHASE_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (ClockShift == NX_SDMMC_CLOCK_SHIFT_0  ) ||
               (ClockShift == NX_SDMMC_CLOCK_SHIFT_90 ) ||
               (ClockShift == NX_SDMMC_CLOCK_SHIFT_180) ||
               (ClockShift == NX_SDMMC_CLOCK_SHIFT_270) );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval  = ReadIO32(&pRegister->CLKCTRL);
    regval &= ~SCLKPHASE_MASK;
    regval |= ClockShift << SCLKPHASE_POS;
    WriteIO32(&pRegister->CLKCTRL, regval);
}

NX_SDMMC_CLKSHIFT   NX_SDMMC_GetSampleClockShiftPhase( U32 ModuleIndex )
{
    const U32 SCLKPHASE_POS     = 24;
    const U32 SCLKPHASE_MASK    = 3UL<<SCLKPHASE_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (NX_SDMMC_CLKSHIFT)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKCTRL) & SCLKPHASE_MASK)>>SCLKPHASE_POS);
}

void    NX_SDMMC_SetDriveClockDelay( U32 ModuleIndex, U32 Delay )
{
    const U32 DCLKDELAY_POS     = 0;
    const U32 DCLKDELAY_MASK    = 0xFFUL<<DCLKDELAY_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( Delay <= 255 );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval  = ReadIO32(&pRegister->CLKCTRL);
    regval &= ~DCLKDELAY_MASK;
    regval |= Delay << DCLKDELAY_POS;
    WriteIO32(&pRegister->CLKCTRL, regval);
}

U32     NX_SDMMC_GetDriveClockDelay( U32 ModuleIndex )
{
    const U32 DCLKDELAY_POS     = 0;
    const U32 DCLKDELAY_MASK    = 0xFFUL<<DCLKDELAY_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKCTRL) & DCLKDELAY_MASK)>>DCLKDELAY_POS;
}

void    NX_SDMMC_SetSampleClockDelay( U32 ModuleIndex, U32 Delay )
{
    const U32 SCLKDELAY_POS     = 8;
    const U32 SCLKDELAY_MASK    = 0xFFUL<<SCLKDELAY_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( Delay <= 255 );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval  = ReadIO32(&pRegister->CLKCTRL);
    regval &= ~SCLKDELAY_MASK;
    regval |= Delay << SCLKDELAY_POS;
    WriteIO32(&pRegister->CLKCTRL, regval);
}

U32     NX_SDMMC_GetSampleClockDelay( U32 ModuleIndex )
{
    const U32 SCLKDELAY_POS     = 8;
    const U32 SCLKDELAY_MASK    = 0xFFUL<<SCLKDELAY_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CLKCTRL) & SCLKDELAY_MASK)>>SCLKDELAY_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether an output clock (SDCLK) is enabled or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates an output clock is enabled.
 *              CFALSE  indicates an output clock is disabled.
 */

//------------------------------------------------------------------------------
/**
 *  @brief      Set a period for data timeout.
 *  @param[in]  none.
 *  @param[in]  dwTimeOut       a timeout period in SDCLKs, 0 ~ 0xFFFFFF.
 *  @return     None.
 *  @remark     This value is used for Data Read Timeout and Data Starvation by
 *              Host Timeout.
 *              - Data Read Timeout:
 *                  During a read-data transfer, if the data start bit is not
 *                  received before the number of data timeout clocks, the SDMMC
 *                  module signals data-timeout error and data transfer done to
 *                  the host and terminates further data transfer.
 *              - Data Starvation by Host Timeout:
 *                  If the FIFO becomes empty during a write data transmission,
 *                  or if the card clock is stopped and the FIFO remains empty for
 *                  data timeout clocks, then a data-starvation error is signaled
 *                  to the host and the SDMMC module continues to wait for data in
 *                  the FIFO.
 *                  During a read data transmission and when the FIFO becomes full,
 *                  the card clock is stopped. If the FIFO remains full for data
 *                  timeout clocks, a data starvation error is signaled to the host
 *                  and the SDMMC module continues to wait for the FIFO to start to
 *                  empty.
 */
void    NX_SDMMC_SetDataTimeOut( U32 ModuleIndex, U32 dwTimeOut )
{
    const U32 DTMOUT_POS    = 8;
    const U32 DTMOUT_MASK   = 0xFFFFFFUL<<DTMOUT_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( 0xFFFFFF >= dwTimeOut );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->TMOUT);
    regval &= ~DTMOUT_MASK;
    regval |= dwTimeOut << DTMOUT_POS;
    WriteIO32(&pRegister->TMOUT, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a period for data timeout.
 *  @param[in]  none.
 *  @return     a number of data timeout clocks in SDCLKs.
 */
U32     NX_SDMMC_GetDataTimeOut( U32 ModuleIndex )
{
    const U32 DTMOUT_POS    = 8;
    const U32 DTMOUT_MASK   = 0xFFFFFFUL<<DTMOUT_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMOUT) & DTMOUT_MASK) >> DTMOUT_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a period for response timeout.
 *  @param[in]  none.
 *  @param[in]  dwTimeOut       a timeout period in SDCLKs, 0 ~ 255.
 *  @return     None.
 *  @remark     This value is used for Response Timeout.
 *              If a response is expected for the command after the command is
 *              sent out, the SDMMC module receives a 48-bit or
 *              136-bit response and sends it to the host. If the start bit of
 *              the card response is not received within the number of response
 *              timeout clocks, then the SDMMC module signals response timeout
 *              error and command done to the host.
 */
void    NX_SDMMC_SetResponseTimeOut( U32 ModuleIndex, U32 dwTimeOut )
{
    const U32 RSPTMOUT_POS  = 0;
    const U32 RSPTMOUT_MASK = 0xFFUL<<RSPTMOUT_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( 0xFF >= dwTimeOut );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->TMOUT);
    regval &= ~RSPTMOUT_MASK;
    regval |= dwTimeOut << RSPTMOUT_POS;
    WriteIO32(&pRegister->TMOUT, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a period for response timeout.
 *  @param[in]  none.
 *  @return     a number of response timeout clocks in SDCLKs.
 */
U32     NX_SDMMC_GetResponseTimeOut( U32 ModuleIndex )
{
    const U32 RSPTMOUT_POS  = 0;
    const U32 RSPTMOUT_MASK = 0xFFUL<<RSPTMOUT_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMOUT) & RSPTMOUT_MASK) >> RSPTMOUT_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a data bus width.
 *  @param[in]  width           a data bus width, 1 or 4.
 *  @return     None.
 *  @remark     1-bit width is standard bus mode and SDMMC uses only SDDAT[0]
 *              for data transfer. 4-bit width is wide bus mode and SDMMC uses
 *              SDDAT[3:0] for data transfer.
 */

void    NX_SDMMC_SetDataBusWidth( U32 ModuleIndex, U32 width )
{
    NX_ASSERT( (1==width) || (4==width) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CTYPE, width >> 2);  // 0 : 1-bit mode, 1 : 4-bit mode
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a data bus width.
 *  @param[in]  none.
 *  @return     a data bus width.
 */

U32     NX_SDMMC_GetDataBusWidth( U32 ModuleIndex )
{
    const U32 WIDTH = 1UL<<0;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CTYPE) & WIDTH) ? 4 : 1;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set the block size for block data transfer.
 *  @param[in]  SizeInByte      Specifies the number of bytes in a block, 0 ~ 65535.
 *  @return     None.
 *  @remark     The Block size is normally set to 512 for MMC/SD module data
 *              transactions. The value is specified in the card's CSD.
 */
void    NX_SDMMC_SetBlockSize( U32 ModuleIndex, U32 SizeInByte )
{
    NX_ASSERT( 65535>=SizeInByte );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BLKSIZ, SizeInByte);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get the block size for block data transfer.
 *  @param[in]  none.
 *  @return     Indicates the number of bytes in a block.
 */
U32     NX_SDMMC_GetBlockSize( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BLKSIZ);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set the data size to be transferred.
 *  @param[in]  SizeInByte      Specifies the data size in bytes.
 *  @return     None.
 *  @remark     The data size should be integer multiple of the block size for
 *              block data transfers. However, for undefined number of byte
 *              transfers, the data size should be set to 0. When the data size
 *              is set to 0, it is responsibility of host to explicitly send
 *              stop/abort command to terminate data transfer.
 */
void    NX_SDMMC_SetByteCount( U32 ModuleIndex, U32 SizeInByte )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BYTCNT, SizeInByte);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get the data size to be transferred.
 *  @param[in]  none.
 *  @return     Indicates the number of bytes to be transferred.

 */
U32     NX_SDMMC_GetByteCount( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BYTCNT);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a command argument to be passed to the card..
 *  @param[in]  argument        Specifies a command argument.
 *  @return     None.
 */
void    NX_SDMMC_SetCommandArgument( U32 ModuleIndex, U32 argument )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CMDARG, argument);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a command.
 *  @param[in]  Cmd             Specifies a command index, 0 ~ 63.
 *  @param[in]  flag            Specifies a command flag. This flag has to consist of ORed @ref NX_SDMMC_CMDFLAG combination.
 *  @return     None.
 *  @remark     This function is for providing user-specific and general command
 *              format of SD/MMC interface.
 *              - If flag has NX_SDMMC_CMDFLAG_STARTCMD, a corresponding command
 *                  will be send to the card immediately.
 *              - If flag does not have NX_SDMMC_CMDFLAG_STARTCMD, you have to
 *                  call NX_SDMMC_StartCommand() function to send a command to the
 *                  card.
 *  @remark     You have to wait unitl NX_SDMMC_IsCommandBusy() returns CFALSE
 *              After calling NX_SDMMC_SetCommand() with NX_SDMMC_CMDFLAG_STARTCMD
 *              flag.
 *  @code
 *      NX_SDMMC_SetCommand( index, cmd, flag | NX_SDMMC_CMDFLAG_STARTCMD );
 *      while( NX_SDMMC_IsCommandBusy( index ) );
 *  @endcode
 */
void    NX_SDMMC_SetCommand( U32 ModuleIndex, U32 Cmd, U32 flag )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( 64 > Cmd );
    NX_ASSERT( (0==flag) || (64 < flag) );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CMD, Cmd | flag);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Start a command which was specified by NX_SDMMC_SetCommand() function.
 *  @param[in]  none.
 *  @return     None.
 *  @remark     You have to wait unitl NX_SDMMC_IsCommandBusy() returns CFALSE
 *              After calling NX_SDMMC_StartCommand().
 *  @code
 *      NX_SDMMC_StartCommand( index );
 *      while( NX_SDMMC_IsCommandBusy( index ) );
 *  @endcode
 *  @see        NX_SDMMC_SetCommand, NX_SDMMC_IsCommandBusy
 */
void    NX_SDMMC_StartCommand( U32 ModuleIndex )
{
    register U32 SetValue;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    SetValue = ReadIO32(&pRegister->CMD) | NX_SDMMC_CMDFLAG_STARTCMD;
    WriteIO32(&pRegister->CMD, SetValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a new command is taken by the SDMMC or not.
 *  @param[in]  none.
 *  @return     CTRUE indicates a command loading is in progress.
 *              CFALSE indicates a command is taken by the SDMMC module.
 *  @remark     While a command loading is in progress, The user should not
 *              modify of SDMMC module's settings. If the user try to modify any
 *              of settings while a command loading is in progress, then the
 *              modification is ignored and the SDMMC module generates a HLE.
 *              The following happens when the command is loaded into the SDMMC:
 *              - The SDMMC module accepts the command for execution, unless one
 *                  command is in progress, at which point the SDMMC module can
 *                  load and keep the second command in the buffer.
 *              - If the SDMMC module is unable to load command - that is, a
 *                  command is already in progress, a second command is in the
 *                  buffer, and a third command is attempted - then it generates
 *                  a HLE(Hardware-locked error).
 *  @see        NX_SDMMC_SetCommand, NX_SDMMC_StartCommand
 */
CBOOL   NX_SDMMC_IsCommandBusy( U32 ModuleIndex )
{
    const U32 STARTCMD_POS = 31;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CMD) >> STARTCMD_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a short response.
 *  @param[in]  none.
 *  @return     A 32-bit short response which represents card status or some argument.
 */
U32     NX_SDMMC_GetShortResponse( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RESP0);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a long response.
 *  @param[in]  none.
 *  @param[out] pLongResponse   Specifies a pointer to 4 x 32bit memory that receives a 128-bit
 *                              long response including CIS/CSD, CRC7 and End bit.
 *  @return     None.
 */
void    NX_SDMMC_GetLongResponse( U32 ModuleIndex, U32 *pLongResponse )
{
    volatile U32 *pSrc;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( CNULL != pLongResponse );
    NX_ASSERT( 0 == ((U32)pLongResponse & 3) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    pSrc = &pRegister->RESP0;

    pLongResponse[0] = pSrc[0];
    pLongResponse[1] = pSrc[1];
    pLongResponse[2] = pSrc[2];
    pLongResponse[3] = pSrc[3];
#if defined(VERBOSE) && (1)
    NX_TRACE(("pCommand->response[0] : 0x%08X\r\n", pSrc[0] ));
    NX_TRACE(("pCommand->response[1] : 0x%08X\r\n", pSrc[1] ));
    NX_TRACE(("pCommand->response[2] : 0x%08X\r\n", pSrc[2] ));
    NX_TRACE(("pCommand->response[3] : 0x%08X\r\n", pSrc[3] ));
#endif
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a short response for auto-stop command.
 *  @param[in]  none.
 *  @return     A 32-bit short response for auto-stop command.
 *  @remark     When the SDMMC module sends auto-stop comamnd(NX_SDMMC_CMDFLAG_SENDAUTOSTOP),
 *              then the user retrieves a response for auto-stop command by this function.
 */
U32     NX_SDMMC_GetAutoStopResponse( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RESP1);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get an index of previous response.
 *  @param[in]  none.
 *  @return     An index of previous response including any auto-stop sent by the SDMMC module.
 */
U32     NX_SDMMC_GetResponseIndex( U32 ModuleIndex )
{
    const U32 RSPINDEX_POS  = 11;
    const U32 RSPINDEX_MASK = 0x3F << RSPINDEX_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & RSPINDEX_MASK) >> RSPINDEX_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a FIFO threshold watermark level when receiving data from card.
 *  @param[in]  Threshold       Specifies a FIFO threshold watermark level when
 *                              receiving data from card. This value represents a
 *                              number of 32-bit data and should be less than or
 *                              equal to 14. Recommend to set it as 7.
 *  @return     None.
 *  @remark     When FIFO data count reaches greater than FIFO RX threshold
 *              watermark level, DMA/FIFO request is raised. During end of
 *              packet, request is generated regardless of threshold programming
 *              in order to complete any remaining data.
 *              In non-DMA mode, when receiver FIFO threshold (RXDR) interrupt
 *              is enabled, then interrupt is generated instead of DMA request.
 *              During end of packet, interrupt is not generated if threshold
 *              programming is larger than any remaining data. It is responsibility
 *              of host to read remaining bytes on seeing Data Transfer Done
 *              interrupt.
 *              In DMA mode, at end of packet, even if remaining bytes are less
 *              than threshold, DMA request does single transfers to flush out
 *              any remaining bytes before Data Transfer Done interrupt is set.
 */
void    NX_SDMMC_SetFIFORxThreshold( U32 ModuleIndex, U32 Threshold )
{
    const U32 RXTH_POS  = 16;
    const U32 RXTH_MASK = 0xFFFUL << RXTH_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( 14 >= Threshold );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->FIFOTH);
    regval &= ~RXTH_MASK;
    regval |= Threshold << RXTH_POS;
    WriteIO32(&pRegister->FIFOTH, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a FIFO threshold watermark level when receiving data from card.
 *  @param[in]  none.
 *  @return     A FIFO threshold watermark level, in 32-bits, when receiving data from card.
 */
U32     NX_SDMMC_GetFIFORxThreshold( U32 ModuleIndex )
{
    const U32 RXTH_POS  = 16;
    const U32 RXTH_MASK = 0xFFFUL << RXTH_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FIFOTH) & RXTH_MASK) >> RXTH_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a FIFO threshold watermark level when transmitting data to card.
 *  @param[in]  Threshold       Specifies a FIFO threshold watermark level when
 *                              transmitting data from card. This value represents a
 *                              number of 32-bit data and should be greater than 1
 *                              and less than or equal to 16.
 *                              Recommend to set it as 8.
 *  @return     None.
 *  @remark     When FIFO data count is less than or equal to FIFO TX threshold
 *              watermark level, DMA/FIFO request is raised. If Interrupt is
 *              enabled, then interrupt occurs. During end of packet, request or
 *              interrupt is generated, regardless of threshold programming.
 *              In non-DMA mode, when transmit FIFO threshold (TXDR) interrupt
 *              is enabled, then interrupt is generated instead of DMA request.
 *              During end of packet, on last interrupt, host is responsible for
 *              filling FIFO with only required remaining bytes (not before FIFO
 *              is full or after SDMMC module completes data transfers, because
 *              FIFO may not be empty).
 */
void    NX_SDMMC_SetFIFOTxThreshold( U32 ModuleIndex, U32 Threshold )
{
    const U32 TXTH_POS  = 0;
    const U32 TXTH_MASK = 0xFFFUL << TXTH_POS;
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( (16 >= Threshold) && (0 < Threshold) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->FIFOTH);
    regval &= ~TXTH_MASK;
    regval |= Threshold << TXTH_POS;
    WriteIO32(&pRegister->FIFOTH, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a FIFO threshold watermark level when transmitting data from card.
 *  @param[in]  none.
 *  @return     A FIFO threshold watermark level, in 32-bits, when transmitting data from card.
 */
U32     NX_SDMMC_GetFIFOTxThreshold( U32 ModuleIndex )
{
    const U32 TXTH_POS  = 0;
    const U32 TXTH_MASK = 0xFFFUL << TXTH_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FIFOTH) & TXTH_MASK) >> TXTH_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a number of filled locations in FIFO.
 *  @param[in]  none.
 *  @return     A number of filled locations, in 32-bits, in FIFO.
 */
U32     NX_SDMMC_GetFIFOCount( U32 ModuleIndex )
{
    const U32 FIFOCOUNT_POS     = 17;
    const U32 FIFOCOUNT_MASK    = 0x1FFFUL << FIFOCOUNT_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & FIFOCOUNT_MASK) >> FIFOCOUNT_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether FIFO is full or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates FIFO is full.
 *              CFALSE  indicates FIFO is not full.
 */
CBOOL   NX_SDMMC_IsFIFOFull( U32 ModuleIndex )
{
    const U32 FIFOFULL_POS  = 3;
    const U32 FIFOFULL_MASK = 1UL << FIFOFULL_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & FIFOFULL_MASK) >> FIFOFULL_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether FIFO is empty or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates FIFO is empty.
 *              CFALSE  indicates FIFO is not empty.
 */
CBOOL   NX_SDMMC_IsFIFOEmpty( U32 ModuleIndex )
{
    const U32 FIFOEMPTY_POS     = 2;
    const U32 FIFOEMPTY_MASK    = 1UL << FIFOEMPTY_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & FIFOEMPTY_MASK) >> FIFOEMPTY_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether FIFO reached transmit watermark level.
 *  @param[in]  none.
 *  @return     CTRUE   indicates FIFO reached trassmit watermark level.
 *              CFALSE  indicates FIFO does not reach transmit watermark level.
 *  @remark     Recommend to use NX_SDMMC_GetInterruptPending( NX_SDMMC_INT_TXDR )
 *              instead of this function.
 */
CBOOL   NX_SDMMC_IsFIFOTxThreshold( U32 ModuleIndex )
{
    const U32 TXWMARK_POS   = 1;
    const U32 TXWMARK_MASK  = 1UL << TXWMARK_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & TXWMARK_MASK) >> TXWMARK_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether FIFO reached receive watermark level.
 *  @param[in]  none.
 *  @return     CTRUE   indicates FIFO reached receive watermark level.
 *              CFALSE  indicates FIFO does not reach receive watermark level.
 *  @remark     Recommend to use NX_SDMMC_GetInterruptPending( NX_SDMMC_INT_RXDR )
 *              instead of this function.
 */
CBOOL   NX_SDMMC_IsFIFORxThreshold( U32 ModuleIndex )
{
    const U32 RXWMARK_POS   = 0;
    const U32 RXWMARK_MASK  = 1UL << RXWMARK_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & RXWMARK_MASK) >> RXWMARK_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a number of bytes transferred by the SDMMC module to card.
 *  @param[in]  none.
 *  @return     A number of bytes transferred by the SDMMC module to card.
 */
U32     NX_SDMMC_GetDataTransferSize( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TCBCNT);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a number of bytes transferred between Host/DMA and FIFO.
 *  @param[in]  none.
 *  @return     A number of bytes transferred   between Host/DMA and FIFO.
 */
U32     NX_SDMMC_GetFIFOTransferSize( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TBBCNT);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a data to be transmitted to the card.
 *  @param[in]  dwData  Specifies a 32-bit data to be transmitted.
 *  @return     None
 *  @remark     This function doesn't check an availablility of FIFO. Therefore
 *              you have to check a FIFO count before using this function.
 *              If you tried to push data when FIFO is full, @ref NX_SDMMC_INT_FRUN
 *              is issued by the SDMMC module.
 */
void    NX_SDMMC_SetData( U32 ModuleIndex, U32 dwData )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DATA, dwData);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a data which was received from the card.
 *  @param[in]  none.
 *  @return     A 32-bit data which was received from the card.
 *  @remark     This function doesn't check an availablility of FIFO. Therefore
 *              you have to check a FIFO count before using this function.
 *              If you tried to read data when FIFO was empty, @ref NX_SDMMC_INT_FRUN
 */
U32     NX_SDMMC_GetData( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DATA);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set 32 bytes of data to be transmitted to the card.
 *  @param[in]  pdwData         a pointer to a buffer include 8 x 32-bit data to be
 *                              transmitted to the card.
 *  @return     None.
 *  @remark     This function doesn't check an availablility of FIFO. Therefore
 *              you have to check a FIFO count before using this function.
 *              If you tried to push data when FIFO is full, @ref NX_SDMMC_INT_FRUN
 *              is issued by the SDMMC module.
 */
void    NX_SDMMC_SetData32( U32 ModuleIndex, const U32 *pdwData )
{
    volatile U32 *pDst;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( CNULL != pdwData );
    NX_ASSERT( 0 == ((U32)pdwData&3) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    pDst = &pRegister->DATA;

    // Loop is unrolled to decrease CPU pipeline broken for a performance.
    WriteIO32(pDst, pdwData[0]);
    WriteIO32(pDst, pdwData[1]);
    WriteIO32(pDst, pdwData[2]);
    WriteIO32(pDst, pdwData[3]);
    WriteIO32(pDst, pdwData[4]);
    WriteIO32(pDst, pdwData[5]);
    WriteIO32(pDst, pdwData[6]);
    WriteIO32(pDst, pdwData[7]);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get 32 bytes of data which were received from the card.
 *  @param[in]  none.
 *  @param[out] pdwData         a pointer to a buffer will be stored 8 x 32-bit data
 *                              which were received from the card.
 *  @return     None.
 *  @remark     This function doesn't check an availablility of FIFO. Therefore
 *              you have to check a FIFO count before using this function.
 *              If you tried to read data when FIFO was empty, @ref NX_SDMMC_INT_FRUN
 *              is issued by the SDMMC module.
 */
void    NX_SDMMC_GetData32( U32 ModuleIndex, U32 *pdwData )
{
    volatile U32 *pSrc;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( CNULL != pdwData );
    NX_ASSERT( 0 == ((U32)pdwData&3) );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    pSrc = &pRegister->DATA;

    // Loop is unrolled to decrease CPU pipeline broken for a performance.
    pdwData[0] = ReadIO32(pSrc);
    pdwData[1] = ReadIO32(pSrc);
    pdwData[2] = ReadIO32(pSrc);
    pdwData[3] = ReadIO32(pSrc);
    pdwData[4] = ReadIO32(pSrc);
    pdwData[5] = ReadIO32(pSrc);
    pdwData[6] = ReadIO32(pSrc);
    pdwData[7] = ReadIO32(pSrc);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a pointer to set or get a 32-bit data on FIFO.
 *  @param[in]  none.
 *  @return     a 32-bit data pointer.
 *  @remark     You have only to aceess this pointer by 32-bit mode and do not
 *              increase or decrease this pointer.
 *              The example for this is as following.
 *  @code
 *      volatile U32 *pData = NX_SDMMC_GetDataPointer( index );
 *      *pData = dwData;    // Push a 32-bit data to FIFO.
 *      dwData = *pData;    // Pop a 32-bit data from FIFO.
 *  @endcode
 */
volatile U32*   NX_SDMMC_GetDataPointer( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return &__g_ModuleVariables[ModuleIndex].pRegister->DATA;
}


//------------------------------------------------------------------------------
// SDMMC Interface ??
//------------------------------------------------------------------------------
CBOOL   NX_SDMMC_MakeBIU( NX_SDMMC_BIUConfig *pBIUC, NX_SDMMC_BIU *pBIU )
{
    S32 i=0;

    NX_ASSERT( CNULL != pBIUC );
    NX_ASSERT( CNULL != pBIU );

    while(pBIUC->DataSize)
    {
        pBIU[i].CTRL = 0;
        if(CTRUE == pBIUC->OWN)
        {
            pBIU[i].CTRL |= (1UL<<31);
        }
        if(CTRUE != pBIUC->IntDisable)
        {
            pBIU[i].CTRL |= (1UL<<1);
        }
        if( 0 == i )
        {
            pBIU[i].CTRL |= (1UL<<3);       // first descriptor
        }

        pBIU[i].BADDR = pBIUC->BuffAddr + (i * NX_SDMMC_MAX_BIU_DATASIZE);

        if(pBIUC->DataSize > NX_SDMMC_MAX_BIU_DATASIZE)
        {
            pBIU[i].BSIZE = (NX_SDMMC_MAX_BIU_DATASIZE<<0)  & ((1UL<<13) -1);
            pBIUC->DataSize -= NX_SDMMC_MAX_BIU_DATASIZE;
            pBIU[i].Next = &pBIU[i+1];
        }
        else
        {
            pBIU[i].CTRL |= (1UL<<2);       // last descriptor
            pBIU[i].BSIZE = ((pBIUC->DataSize<<0) & ((1UL<<13) -1));
            pBIUC->DataSize = 0;
            pBIU[i].Next = CNULL;
        }
        i++;
    }
    return CTRUE;
}

void    NX_SDMMC_GetDataBIU( U32 ModuleIndex, NX_SDMMC_BIU *pBIU )
{
    NX_ASSERT( CNULL != pBIU );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pBIU = (NX_SDMMC_BIU*)ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DBADDR);
}

void    NX_SDMMC_SetDataBIU( U32 ModuleIndex, NX_SDMMC_BIU *pBIU )
{
    NX_ASSERT( CNULL != pBIU );
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DBADDR, pBIU);
}

void NX_SDMMC_PollDemand( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->PLDMND, 1);
}

void NX_SDMMC_SetDMACStatus( U32 ModuleIndex, U32 SetData )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IDSTS, SetData);
}

U32 NX_SDMMC_GetDMACStatus( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IDSTS);
}

void NX_SDMMC_SetDMACIntEnable( U32 ModuleIndex, U32 IntFlag )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IDINTEN, IntFlag);
}

U32 NX_SDMMC_GetDMACIntEnable( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IDINTEN);
}

CBOOL NX_SDMMC_GetDMACResetStatus( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BMOD) & 1UL<<0);
}

void NX_SDMMC_SetDMACBurstLength( U32 ModuleIndex, U32 BurstLength )
{
    register U32 regval, bl = 0;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    NX_ASSERT( 1   == BurstLength || 4   == BurstLength || 8  == BurstLength ||
               16  == BurstLength || 32  == BurstLength || 64 == BurstLength ||
               128 == BurstLength || 256 == BurstLength);

    if(BurstLength == 1)
        bl = 0;
    else
    if(BurstLength == 4)
        bl = 1;
    else
    if(BurstLength == 8)
        bl = 2;
    else
    if(BurstLength == 16)
        bl = 3;
    else
    if(BurstLength == 32)
        bl = 4;
    else
    if(BurstLength == 64)
        bl = 5;
    else
    if(BurstLength == 128)
        bl = 6;
    else
    if(BurstLength == 256)
        bl = 7;

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    //regval = ReadIO32(&pRegister->BMOD);
    //regval &= ~(7UL<<8);
    //regval |= bl<<8;
    //WriteIO32(&pRegister->BMOD, regval);
    regval = ReadIO32(&pRegister->FIFOTH);
    regval &= ~(7UL<<28);
    regval |= bl<<28;
    WriteIO32(&pRegister->FIFOTH, regval);
}

void NX_SDMMC_SetIDMACEnable( U32 ModuleIndex, CBOOL bEnable )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (0 == bEnable) || (1 == bEnable) );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->BMOD);
    regval &= ~(1UL<<7);
    regval |= bEnable<<7;
    WriteIO32(&pRegister->BMOD, regval);
}

CBOOL NX_SDMMC_GetIDMACEnable( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BMOD)>>7) & 0x1);
}

void NX_SDMMC_SetDescSkipLen( U32 ModuleIndex, U32 uLength )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( 0x1FUL >= uLength );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->BMOD);
    regval &= ~(0x1FUL<<2);
    regval |= uLength<<2;
    WriteIO32(&pRegister->BMOD, regval);
}

U32 NX_SDMMC_GetDescSkipLen( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BMOD)>>2) & 0x1F);
}

void NX_SDMMC_ResetIDMAC( U32 ModuleIndex )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->BMOD);
    regval |= (1UL<<0);
    WriteIO32(&pRegister->BMOD, regval);
}

CBOOL NX_SDMMC_IsResetIDMAC( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BMOD) & 0x1);
}

void NX_SDMMC_SetDebounce( U32 ModuleIndex, U32 uDebounce )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( !(0xFF000000 & uDebounce) );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DEBNCE, uDebounce);
}

U32 NX_SDMMC_GetDebounce( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DEBNCE) & 0x00FFFFFF;
}

CBOOL NX_SDMMC_IsIDMACSupported( U32 ModuleIndex )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    WriteIO32(&pRegister->CTRL, regval | 1UL<<25);
    regval = ReadIO32(&pRegister->CTRL);
    if(regval & 1UL<<25)
    {
        WriteIO32(&pRegister->CTRL, regval & ~(1UL<<25));
        return CTRUE;
    }
    return CFALSE;
}


//------------------------------------------------------------------------------
// SDMMC Voltage
//------------------------------------------------------------------------------
void    NX_SDMMC_SetCardVoltage( U32 ModuleIndex, U32 VolBase, U32 VolOffset )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( 0 == VolBase || 1== VolBase);
    NX_ASSERT( 0xF>= VolOffset );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->CTRL);
    regval &= ~(0xFF<<16);
    regval |= VolOffset<<(16+VolBase*4);
    WriteIO32(&pRegister->CTRL, regval);
}

U32     NX_SDMMC_GetCardVoltage( U32 ModuleIndex, U32 VolBase )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( 0 == VolBase || 1== VolBase);

    return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CTRL) >> (16 + VolBase*4)) & 0xF;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a data transfer is busy or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates a data transfer is busy.
 *              CFALSE  indicates a data transfer is not busy.
 *  @see
 */
CBOOL   NX_SDMMC_IsDataTransferBusy( U32 ModuleIndex )
{
    const U32 FSMBUSY_POS   = 10;
    const U32 FSMBUSY_MASK  = 1UL << FSMBUSY_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & FSMBUSY_MASK) >> FSMBUSY_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a card data is busy or not.
 *  @param[in]  none.
 *  @return     CTRUE   indicates a card data is busy.
 *              CFALSE  indicates a card data is not busy.
 *  @remark     The return value is an inverted state of SDDAT[0].
 *  @see        NX_SDMMC_IsCardPresent
 */
CBOOL   NX_SDMMC_IsCardDataBusy( U32 ModuleIndex )
{
    const U32 DATBUSY_POS   = 9;
    const U32 DATBUSY_MASK  = 1UL << DATBUSY_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & DATBUSY_MASK) >> DATBUSY_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a card is present or not.
 *  @param[in]  none.
 *  @return     CTRUE indicates a card is present.
 *              CFALSE indicates a card is not present.
 *  @remark     The return value is a state of SDDAT[3] pin.
 *              SDDAT[3] pin can be used to detect card presence if it is pulled
 *              low by external H/W circuit. When there's no card on the bus,
 *              SDDAT[3] shows a low voltage level.
 *              There's a 50 KOhm pull-up resistor on SDDAT[3] in the card.
 *              Therefore SDDAT[3] pin pulls the bus line high when any card is
 *              inserted on the bus. This pull-up in the card should be
 *              disconnected by the user, during regular data transfer, with
 *              SET_CLR_CARD_DETECT (ACMD42) command.
 *  @see        NX_SDMMC_IsCardDataBusy
 */
CBOOL   NX_SDMMC_IsCardPresent( U32 ModuleIndex )
{
    const U32 CPRESENT_POS  = 8;
    const U32 CPRESENT_MASK = 1UL << CPRESENT_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & CPRESENT_MASK) >> CPRESENT_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a command FSM state.
 *  @param[in]  none.
 *  @return     A command FSM state represents by @ref NX_SDMMC_CMDFSM enum.
 *  @see        NX_SDMMC_CMDFSM
 */
void    NX_SDMMC_SetDDRMode( U32 ModuleIndex, CBOOL bEnable )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (0 == bEnable) || (1 == bEnable) );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->UHS_REG);
    regval &= ~(1UL<<16);
    regval |= bEnable<<16;
    WriteIO32(&pRegister->UHS_REG, regval);
}

CBOOL   NX_SDMMC_GetDDRMode( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UHS_REG)>>16) & 0x1);
}

void    NX_SDMMC_SetVoltageMode( U32 ModuleIndex, CBOOL bEnable )
{
    register U32 regval;
    register struct NX_SDMMC_RegisterSet *pRegister;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (0 == bEnable) || (1 == bEnable) );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->UHS_REG);
    regval &= ~(1UL<<0);
    regval |= bEnable<<0;
    WriteIO32(&pRegister->UHS_REG, regval);
}

CBOOL   NX_SDMMC_GetVoltageMode( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UHS_REG)>>0) & 0x1);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a command FSM state.
 *  @param[in]  none.
 *  @return     A command FSM state represents by @ref NX_SDMMC_CMDFSM enum.
 *  @see        NX_SDMMC_CMDFSM
 */
NX_SDMMC_CMDFSM NX_SDMMC_GetCommandFSM( U32 ModuleIndex )
{
    const U32 CMDFSM_POS    = 4;
    const U32 CMDFSM_MASK   = 0xFUL << CMDFSM_POS;

    NX_ASSERT( NUMBER_OF_SDMMC_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (NX_SDMMC_CMDFSM)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->STATUS) & CMDFSM_MASK) >> CMDFSM_POS);
}

