//------------------------------------------------------------------------------
//
//  Copyright (C) 2009 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : SCALER
//  File        : nx_scaler.c
//  Description :
//  Author      : Firmware Team
//  History     :
//------------------------------------------------------------------------------

#include "nx_chip.h"
#include "nx_scaler.h"

static  struct
{
    struct NX_SCALER_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_SCALER_MODULE] = { {CNULL,}, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *  @brief  Initialize of prototype enviroment & local variables.
 *  @return  CTRUE    indicates that Initialize is successed.
 *           CFALSE   indicates that Initialize is failed.
 *  @see    NX_SCALER_GetNumberOfModule
 */
CBOOL   NX_SCALER_Initialize( void )
{
    static CBOOL bInit = CFALSE;
    U32 i;

    if( CFALSE == bInit )
    {
        for( i=0; i<NUMBER_OF_SCALER_MODULE; i++ )
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
 *  @return     Module's number.
 *  @see        NX_SCALER_Initialize
 */
U32     NX_SCALER_GetNumberOfModule( void )
{
    return NUMBER_OF_SCALER_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get module's physical address.
 *  @return     Module's physical address
 */
U32     NX_SCALER_GetPhysicalAddress( U32 ModuleIndex )
{
    static const U32 ScalerPhysicalAddr[NUMBER_OF_SCALER_MODULE] =
    {
        PHY_BASEADDR_LIST( SCALER )
    };

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return  (U32)ScalerPhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a size, in byte, of register set.
 *  @return     Size of module's register set.
 */
U32     NX_SCALER_GetSizeOfRegisterSet( void )
{
    return sizeof( struct NX_SCALER_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a base address of register set.
 *  @param[in]  BaseAddress Module's base address
 *  @return     None.
 */
void    NX_SCALER_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
    NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    __g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SCALER_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a base address of register set
 *  @return     Module's base address.
 */
void*    NX_SCALER_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Initialize selected modules with default value.
 *  @return      CTRUE    indicates that Initialize is successed. 
 *               CFALSE   indicates that Initialize is failed.
 */
CBOOL   NX_SCALER_OpenModule( U32 ModuleIndex )
{
    register struct NX_SCALER_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;


    WriteIO32(&pRegister->SCRUNREG,     0x00000000);
    WriteIO32(&pRegister->SCCFGREG,     0x00000000);
    WriteIO32(&pRegister->SCINTREG,     0x00000100);
    WriteIO32(&pRegister->SCSRCADDREG,  0x00000000);
    WriteIO32(&pRegister->SCSRCSTRIDE,  0x00000000);
    WriteIO32(&pRegister->SCSRCSIZEREG, 0x00000000);
    WriteIO32(&pRegister->SCDESTADDREG0,0x00000000);
    WriteIO32(&pRegister->SCDESTSTRIDE0,0x00000000);
    WriteIO32(&pRegister->SCDESTADDREG1,0x00000000);
    WriteIO32(&pRegister->SCDESTSTRIDE1,0x00000000);
    WriteIO32(&pRegister->SCDESTSIZEREG,0x00000000);
    WriteIO32(&pRegister->DELTAXREG,    0x00000000);
    WriteIO32(&pRegister->DELTAYREG,    0x00000000);
    WriteIO32(&pRegister->HVSOFTREG,    0x00000000);
    WriteIO32(&pRegister->CMDBUFCON,    0x00000000);
    WriteIO32(&pRegister->CMDBUFADDR,   0x00000000);

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinitialize selected module to the proper stage.
 *  @return      CTRUE    indicates that Deinitialize is successed. 
 *               CFALSE   indicates that Deinitialize is failed.
 */
CBOOL   NX_SCALER_CloseModule( U32 ModuleIndex )
{
    register struct NX_SCALER_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    WriteIO32(&pRegister->SCRUNREG,     0x00000000);
    WriteIO32(&pRegister->SCCFGREG,     0x00000000);
    WriteIO32(&pRegister->SCINTREG,     0x00000100);
    WriteIO32(&pRegister->SCSRCADDREG,  0x00000000);
    WriteIO32(&pRegister->SCSRCSTRIDE,  0x00000000);
    WriteIO32(&pRegister->SCSRCSIZEREG, 0x00000000);
    WriteIO32(&pRegister->SCDESTADDREG0,0x00000000);
    WriteIO32(&pRegister->SCDESTSTRIDE0,0x00000000);
    WriteIO32(&pRegister->SCDESTADDREG1,0x00000000);
    WriteIO32(&pRegister->SCDESTSTRIDE1,0x00000000);
    WriteIO32(&pRegister->SCDESTSIZEREG,0x00000000);
    WriteIO32(&pRegister->DELTAXREG,    0x00000000);
    WriteIO32(&pRegister->DELTAYREG,    0x00000000);
    WriteIO32(&pRegister->HVSOFTREG,    0x00000000);
    WriteIO32(&pRegister->CMDBUFCON,    0x00000000);
    WriteIO32(&pRegister->CMDBUFADDR,   0x00000000);

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the selected modules is busy or not.
 *  @return      CTRUE    indicates that Module is Busy. 
 *               CFALSE   indicates that Module is NOT Busy.
 */
CBOOL   NX_SCALER_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return NX_SCALER_IsBusy( ModuleIndex );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicaes whether the selected modules is ready to enter power-down stage
 *  @return      CTRUE    indicates that Ready to enter power-down stage. 
 *               CFALSE   indicates that This module can't enter to power-down stage.
 */
CBOOL   NX_SCALER_CanPowerDown( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number for interrupt controller.
 *  @return     Interrupt number
 */
S32     NX_SCALER_GetInterruptNumber( U32 ModuleIndex )
{
    const U32   ScalerInterruptNumber[NUMBER_OF_SCALER_MODULE] = { INTNUM_LIST( SCALER ) };

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return ScalerInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  IntNum  Interrupt Number ( 0: Scaler End ).
 *  @param[in]  Enable   CTRUE    indicates that Interrupt Enable. 
 *                       CFALSE   indicates that Interrupt Disable.
 *  @return     None.
 */
void    NX_SCALER_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable )
{
    const U32 SC_INT_ENB_BITPOS = 16;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( (1 >= IntNum) && (0 <= IntNum) );
    NX_ASSERT( (0 == Enable) || (1 == Enable) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG = ((U32)Enable << SC_INT_ENB_BITPOS);
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, ((U32)Enable << (SC_INT_ENB_BITPOS+IntNum)));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]  IntNum  Interrupt Number( 0: Scaler End ).
 *  @return      CTRUE    indicates that Interrupt is enabled. 
 *               CFALSE   indicates that Interrupt is disabled.
 */
CBOOL   NX_SCALER_GetInterruptEnable( U32 ModuleIndex, S32 IntNum )
{
    const U32 SC_INT_ENB_BITPOS = 16;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( (1 >= IntNum) && (0 <= IntNum) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG >> (SC_INT_ENB_BITPOS+IntNum)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  EnableFlag  Specify interrupt bit for enable of disable. Each bit's meaning is like below   
 *                          - EnableFlag[0] : Set Scaler Done interrupt enable or disable. 
 *  @return     None.
 */
void    NX_SCALER_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag )
{
    const U32   SC_INT_ENB_BITPOS = 16;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG = ((EnableFlag & 0x01) << SC_INT_ENB_BITPOS);
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, ((EnableFlag & 0x03) << SC_INT_ENB_BITPOS));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates current setting value of interrupt enable bit.
 *  @return     Current setting value of interrupt. 
 *              "1" means interrupt is enabled. 
 *              "0" means interrupt is disabled. 
 *              - Return Value[0] : Scaler Done interrupt's setting value. 
 */
U32     NX_SCALER_GetInterruptEnable32( U32 ModuleIndex )
{
    const U32   SC_INT_ENB_BITPOS   = 16;
    const U32   SC_INT_ENB_MASK     = 3<<SC_INT_ENB_BITPOS;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (U32)((__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG & SC_INT_ENB_MASK)>>SC_INT_ENB_BITPOS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]  IntNum  Interrupt Number( 0 : Scaler End ).
 *  @return      CTRUE    indicates that Pending is seted. 
 *               CFALSE   indicates that Pending is Not Seted.
 */
CBOOL   NX_SCALER_GetInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( (1 >= IntNum) && (0 <= IntNum) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    IntNum = IntNum;

    return (CBOOL)( ((__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG)>>IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates current setting value of interrupt pending bit.
 *  @return     Current setting value of pending bit. 
 *              "1" means pend bit is occured. 
 *              "0" means pend bit is NOT occured. 
 *              - Return Value[0] : Scaler Done pending state. 
 */
U32     NX_SCALER_GetInterruptPending32( U32 ModuleIndex )
{
    const U32 PEND_MASK = 0x03;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (U32)(__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG & PEND_MASK);
}


//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  IntNum  Interrupt number( 0 : Scaler End ).
 *  @return     None.
 */
void    NX_SCALER_ClearInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    const U32 SC_INT_CLR_BITPOS = 8;
    register U32 regval;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( (1 >= IntNum) && (0 <= IntNum) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    IntNum = IntNum;

    //__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG |= (0x01 << SC_INT_CLR_BITPOS);
    regval = __g_ModuleVariables[ModuleIndex].pRegister->SCINTREG | (0x01 << (SC_INT_CLR_BITPOS+IntNum));
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  PendingFlag     Specify pend bit to clear. Each bit's meaning is like below  
 *                              - PendingFlag[0] : Scaler Done pending bit. 
 *  @return     None.
 */
void    NX_SCALER_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
    const U32   PEND_BITPOS = 8;
    register U32 regval;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG |=  (PendingFlag&0x01) << PEND_BITPOS;
    regval = __g_ModuleVariables[ModuleIndex].pRegister->SCINTREG | (PendingFlag&0x03) << PEND_BITPOS;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set all interrupts to be enables or disables.
 *  @param[in]  Enable   CTRUE    indicates that Set to all interrupt enable. 
 *                       CFALSE   indicates that Set to all interrupt disable.
 *  @return     None.
 */
void    NX_SCALER_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
    const U32 SC_INT_ENB_MASK   = 0x03;
    const U32 SC_INT_ENB_BITPOS = 16;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( (0 == Enable) || (1 == Enable) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( CTRUE == Enable)
        WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, (SC_INT_ENB_MASK << SC_INT_ENB_BITPOS));
    else
        WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, 0x00);

}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enable or not.
 *  @return      CTRUE    indicates that At least one( or more ) interrupt is enabled. 
 *               CFALSE   indicates that All interrupt is disabled.
 */
CBOOL   NX_SCALER_GetInterruptEnableAll( U32 ModuleIndex )
{
    const U32 SC_INT_ENB_BITPOS = 16;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG >> SC_INT_ENB_BITPOS) & 0x03 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are pended or not.
 *  @return      CTRUE    indicates that At least one( or more ) pending is seted. 
 *               CFALSE   indicates that All pending is NOT seted.
 */
CBOOL   NX_SCALER_GetInterruptPendingAll( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (CBOOL)(__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG & 0x03 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear pending state of all interrupts.
 *  @return     None.
 */
void    NX_SCALER_ClearInterruptPendingAll( U32 ModuleIndex )
{
    const U32 SC_INT_CLR_BITPOS = 8;
    register U32 regval;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG |= (0x01 << SC_INT_CLR_BITPOS);
    regval = __g_ModuleVariables[ModuleIndex].pRegister->SCINTREG | (0x03 << SC_INT_CLR_BITPOS);
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCINTREG, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number which has the most prority of pended interrupts
 *  @return     Pending Number( If all pending is not set then return -1 ).
 *              0 : Scaler End
 */
S32     NX_SCALER_GetInterruptPendingNumber( U32 ModuleIndex ) // -1 if None
{
    const U32 SC_INT_ENB_BITPOS = 16;
    const U32 SC_INT_PEND_MASK  = 0x01;

    register struct NX_SCALER_RegisterSet   *pRegister;
    register U32 Pend;
    U32 IntNum;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    Pend = (pRegister->SCINTREG >> SC_INT_ENB_BITPOS) & pRegister->SCINTREG;

    for( IntNum=0; IntNum<2; IntNum++ )
    {
        if( Pend & (SC_INT_PEND_MASK<<IntNum) )
        {
            return IntNum;
        }
    }

    return -1;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a PCLK mode
 *  @param[in]  mode    PCLK mode
 *  @return     None.
 */
/*
void            NX_SCALER_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode )
{
    const U32 PCLKMODE_POS  =   3;

    register U32 regvalue;
    register struct NX_SCALER_RegisterSet* pRegister;

    U32 clkmode=0;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    switch(mode)
    {
        case NX_PCLKMODE_DYNAMIC:   clkmode = 0;        break;
        case NX_PCLKMODE_ALWAYS:    clkmode = 1;        break;
        default: NX_ASSERT( CFALSE );
    }

    regvalue = pRegister->CLKENB;
    regvalue &= ~(1UL<<PCLKMODE_POS);
    regvalue |= ( clkmode & 0x01 ) << PCLKMODE_POS;
    //pRegister->CLKENB = regvalue;
    WriteIO32(&pRegister->CLKENB, regvalue);
}
*/

//------------------------------------------------------------------------------
/**
 *  @brief      Get current PCLK mode
 *  @return     Current PCLK mode
 */
/*
NX_PCLKMODE NX_SCALER_GetClockPClkMode( U32 ModuleIndex )
{
    const U32 PCLKMODE_POS  = 3;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( __g_ModuleVariables[ModuleIndex].pRegister->CLKENB & ( 1UL << PCLKMODE_POS ) )
    {
        return NX_PCLKMODE_ALWAYS;
    }

    return  NX_PCLKMODE_DYNAMIC;
}
*/

//------------------------------------------------------------------------------
/**
 *  @brief      Set System Bus Clock's operation Mode
 *  @param[in]  mode        BCLK Mode
 *  @return     None.
 */
/*
void NX_SCALER_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode )
{
    register U32 regvalue;
    register struct NX_SCALER_RegisterSet* pRegister;
    U32 clkmode=0;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    switch(mode)
    {
        case NX_BCLKMODE_DISABLE:   clkmode = 0;        break;
        case NX_BCLKMODE_DYNAMIC:   clkmode = 2;        break;
        case NX_BCLKMODE_ALWAYS:    clkmode = 3;        break;
        default: NX_ASSERT( CFALSE );
    }

    regvalue = pRegister->CLKENB;
    regvalue &= ~(0x03);
    regvalue |= clkmode & 0x03;
    //pRegister->CLKENB = regvalue;
    WriteIO32(&pRegister->CLKENB, regvalue);
}
*/

static U32 *__g_clkBaseAddress = CNULL;

void NX_SCALER_CLKGEN_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress)
{
    // Prevent Warrning
    ModuleIndex = ModuleIndex;
    __g_clkBaseAddress = (U32 *)BaseAddress;
}

U32 NX_SCALER_CLKGEN_GetPhysicalAddress(U32 ModuleIndex)
{
    // Prevent Warrning
    ModuleIndex = ModuleIndex;    
    return 0xC00B6000;
}

void NX_SCALER_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode )
{
    U32 clkmode=0;

    // Prevent Warrning
    ModuleIndex = ModuleIndex;

    NX_ASSERT( CNULL != __g_clkBaseAddress);

    switch(mode)
    {
        case NX_BCLKMODE_DISABLE:   clkmode = 0;        break;
        case NX_BCLKMODE_DYNAMIC:   clkmode = 2;        break;
        case NX_BCLKMODE_ALWAYS:    clkmode = 3;        break;
        default: NX_ASSERT( CFALSE );
    }
    WriteIO32(__g_clkBaseAddress, clkmode);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get System Bus Clock's operation Mode
 *  @return     BCLK Mode
 *  @see        NX_SCALER_SetClockPClkMode,     NX_SCALER_GetClockPClkMode,
 *              NX_SCALER_SetClockBClkMode
 */
/*
NX_BCLKMODE NX_SCALER_GetClockBClkMode( U32 ModuleIndex )
{
    const U32 BCLKMODE  = 3UL<<0;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    switch( __g_ModuleVariables[ModuleIndex].pRegister->CLKENB & BCLKMODE )
    {
        case 0 :    return NX_BCLKMODE_DISABLE;
        case 2 :    return NX_BCLKMODE_DYNAMIC;
        case 3 :    return NX_BCLKMODE_ALWAYS;
    }

    NX_ASSERT( CFALSE );
    return NX_BCLKMODE_DISABLE;
}
*/

//------------------------------------------------------------------------------
/**
 *  @brief      Get System Clock's Number
 *  @return     BCLK Mode
 */
U32 NX_SCALER_GetClockNumber( U32 ModuleIndex )
{
    // PHY_BASEADDR_CLKGEN_MODULE
    const U32 CLKGEN_SCALERLIST[] =
    {
        CLOCKINDEX_LIST( SCALER )
    };

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return (U32)CLKGEN_SCALERLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get System Reset Number
 *  @return     BCLK Mode
 */
U32 NX_SCALER_GetResetNumber( U32 ModuleIndex )
{
    const U32 RSTCON_SCALERLIST[] =
    {
        RESETINDEX_LIST( SCALER, i_nRST )
    };

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return (U32)RSTCON_SCALERLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
// Scaler Hardware Interface Implementation
//------------------------------------------------------------------------------
/**
 *  @brief      Set Scaler Run
 *  @return     None.
 */
void    NX_SCALER_Run( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCRUNREG = 0x01;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCRUNREG, 0x01);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Scaler Stop
 *  @return     None.
 *  @see        NX_SCALER_Run,      NX_SCALER_IsBusy
 */
void    NX_SCALER_Stop( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCRUNREG = 0x00;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCRUNREG, 0x00);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Check scaler's state ( busy or idle )
 *  @return     CTRUE indicates that scaler is running.
 *              CFALSE indicates that scaler is idle.
 */
CBOOL   NX_SCALER_IsBusy( U32 ModuleIndex )
{
    const U32   BUSY_MASK = ( 1 << 24 );

    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( __g_ModuleVariables[ModuleIndex].pRegister->SCINTREG & BUSY_MASK ) {
        return CTRUE;
    } else {
        return CFALSE;
    }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Scaler's Filter Operation
 *  @param[in]  enable      CTRUE( Filter Enable ), CFALSE( Filter Disable )
 *  @return     None.
 */
void    NX_SCALER_SetFilterEnable( U32 ModuleIndex, CBOOL enable )
{
    const U32   FENB_MASK   = ( 0x03 << 0 );
    register U32    temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG ;

    if( CTRUE == enable )
    {
        temp |= FENB_MASK;
    }
    else
    {
        temp &= ~FENB_MASK;
    }

    //__g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG = temp;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Check Scaler's Filter Operation
 *  @return     CTRUE   indicates that Filter is Enabled.
 *              CFALSE  indicates that Filter is Disabled.
 */
CBOOL   NX_SCALER_GetFilterEnable( U32 ModuleIndex )
{
    const U32   FENB_MASK   = ( 0x03 << 0 );
    register U32    temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG ;

    if( temp & FENB_MASK )
    {
        return CTRUE;
    }
    else
    {
        return CFALSE;
    }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Horizontal and Vertical Ration Value
 *  @param[in]  RatioH      Value of Horizontal Ratio( 0x0 ~ 0x3F )
 *  @param[in]  RatioV      Value of Vertical Ratio( 0x0 ~ 0x1F )
 *  @return     None.
 */
void    NX_SCALER_SetFilterRatio( U32 ModuleIndex, U32 RatioH, U32 RatioV )
{
    const U32   H_RATIO_BITPOS =    0;
    const U32   V_RATIO_BITPOS = 16;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( (1<<6) > RatioH );
    NX_ASSERT( (1<<5) > RatioV );

    //__g_ModuleVariables[ModuleIndex].pRegister->HVSOFTREG = ( (RatioH << H_RATIO_BITPOS ) | ( RatioV << V_RATIO_BITPOS ) );
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->HVSOFTREG, ( (RatioH << H_RATIO_BITPOS ) | ( RatioV << V_RATIO_BITPOS ) ));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Horizontal and Vertical Ration Value
 *  @param[out] RatioH      Value of Horizontal Ratio( 0x0 ~ 0x3F )
 *  @param[out] RatioV      Value of Vertical Ratio( 0x0 ~ 0x1F )
 *  @return     None.
 *  @remarks    Parameter(RatioH, RatioV) can set to CNULL, when the information is not need.
 */
void    NX_SCALER_GetFilterRatio( U32 ModuleIndex, U32* RatioH, U32* RatioV )
{
    const U32   H_RATIO_BITPOS  =   0;
    const U32   H_RATIO_MASK    = ( 0x3F << H_RATIO_BITPOS );
    const U32   V_RATIO_BITPOS  = 16;
    const U32   V_RATIO_MASK    = ( 0x1F << V_RATIO_BITPOS );
    register U32 temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->HVSOFTREG ;

    if( CNULL != RatioH )
    {
        *RatioH = ( temp & H_RATIO_MASK ) >> H_RATIO_BITPOS;
    }

    if( CNULL != RatioV )
    {
        *RatioV = ( temp & V_RATIO_MASK ) >> V_RATIO_BITPOS;
    }
}


void    NX_SCALER_SetYVFilter( U32 ModuleIndex, U32 FilterSel, U32 FilterIndex, U32 FilterVal )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( FilterSel < 3 );
    NX_ASSERT( FilterIndex < 8 );

    //__g_ModuleVariables[ModuleIndex].pRegister->YVFILTER[FilterSel][FilterIndex] = (S8)FilterVal;
    WriteIO32( &__g_ModuleVariables[ModuleIndex].pRegister->YVFILTER[FilterSel][FilterIndex], (U32)FilterVal );
}

void    NX_SCALER_SetYHFilter( U32 ModuleIndex, U32 FilterSel, U32 FilterIndex, U32 FilterVal )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( FilterSel < 5 );
    NX_ASSERT( FilterIndex < 64 );

    //__g_ModuleVariables[ModuleIndex].pRegister->YHFILTER[FilterSel][FilterIndex] = (S16)FilterVal;
    WriteIO32( &__g_ModuleVariables[ModuleIndex].pRegister->YHFILTER[FilterSel][FilterIndex], (U32)FilterVal );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Scaler's Rotate Mode
 *  @param[in]  rotate      Value of Rotate Mode
 *  @return     None.
 */
/*
void    NX_SCALER_SetRotateMode( U32 ModuleIndex, NX_SCALER_ROTATE rotate )
{
    const U32   ROT_MASK    = ( 0x07 << 24 );
    const U32   ROT_BITPOS  = 24;
    register U32    temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( NX_SCALER_ROTATE_180 >= rotate );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG ;

    temp = ( temp & ~ROT_MASK ) | ( (U32)rotate << ROT_BITPOS );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG = temp ;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG, temp);
}
*/

//------------------------------------------------------------------------------
/**
 *  @brief      Get Scaler's Rotate Mode
 *  @return     Value of Rotate Mode
 */
/*
NX_SCALER_ROTATE    NX_SCALER_GetRotateMode( U32 ModuleIndex )
{
    const U32   ROT_MASK    = ( 0x07 << 24 );
    const U32   ROT_BITPOS  = 24;
    register U32    temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG ;

    return(NX_SCALER_ROTATE)( ( temp & ROT_MASK ) >> ROT_BITPOS );
}
*/

//------------------------------------------------------------------------------
/**
 *  @brief      Set Scaler's Rotate Mode
 *  @param[in]  rotate      Value of Rotate Mode
 *  @return     None.
 */
void    NX_SCALER_SetMode( U32 ModuleIndex, NX_SCALER_MODE mode )
{
    const U32   MOD_MASK    = ( 0x07 << 24 );
    const U32   MOD_BITPOS  = 24;
    register U32    temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( NX_SCALER_MODE_CBCR >= mode );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG ;

    temp = ( temp & ~MOD_MASK ) | ( (U32)mode << MOD_BITPOS );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG = temp ;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Scaler's Rotate Mode
 *  @return     Value of Rotate Mode
 */
NX_SCALER_MODE    NX_SCALER_GetMode( U32 ModuleIndex )
{
    const U32   MOD_MASK    = ( 0x07 << 24 );
    const U32   MOD_BITPOS  = 24;
    register U32    temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCCFGREG ;

    return(NX_SCALER_MODE)( ( temp & MOD_MASK ) >> MOD_BITPOS );
}

//--------------------------------------------------------------------------
// Configuration Function of Image Size and Address
//--------------------------------------------------------------------------
/**
 *  @brief      Set Source Image Address
 *  @param[in]  Addr        Value of Source Image Address
 *  @return     None.
 *  @remarks    Address's Format is 2D Address. so each bit have meaning.
 *              Addr[27:24] : Segment Address.
 *              Addr[23:12] : Base Address Y. Should be arranged in 8-byte. 
 *              Addr[11:0]  : Base Address X. Should be arranged in 8-byte. 
 */
void    NX_SCALER_SetSrcAddr( U32 ModuleIndex, U32 Addr )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    //NX_ASSERT( 0 == (0x50007007 & Addr) );
    //NX_ASSERT( 0 == (0x50007007 & Addr) );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCSRCADDREG  = Addr;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCSRCADDREG, Addr);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Source Image Address
 *  @return     Value of Source Image 2D Address.
 *              SrcAddr[27:24] : Segment Address.
 *              SrcAddr[23:12] : Base Address Y.
 *              SrcAddr[11:0]   : Base Address X.
 */
U32     NX_SCALER_GetSrcAddr( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return __g_ModuleVariables[ModuleIndex].pRegister->SCSRCADDREG;
}

//--------------------------------------------------------------------------
/**
 *  @brief      Set Source Image Stride
 *  @param[in]  Addr        Value of Source Image Address
 *  @return     None.
 *  @remarks    Address's Format is 2D Address. so each bit have meaning.
 *              Addr[27:24] : Segment Address.
 *              Addr[23:12] : Base Address Y. Should be arranged in 8-byte. 
 *              Addr[11:0]  : Base Address X. Should be arranged in 8-byte. 
 *  @see                                NX_SCALER_GetSrcAddr,
 *              NX_SCALER_SetDestAddr,  NX_SCALER_GetDestAddr
 */
void    NX_SCALER_SetSrcStride( U32 ModuleIndex, U32 Stride )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->SCSRCADDREG  = Addr;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCSRCSTRIDE, Stride);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Source Image Stride
 *  @return     Value of Source Image 2D Address.
 *              SrcAddr[27:24] : Segment Address.
 *              SrcAddr[23:12] : Base Address Y.
 *              SrcAddr[11:0]   : Base Address X.
 */
U32     NX_SCALER_GetSrcStride( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    return __g_ModuleVariables[ModuleIndex].pRegister->SCSRCSTRIDE;
}


//--------------------------------------------------------------------------
/**
 *  @brief      Set Destination Image Address
 *  @param[in]  Addr        Value of Destination Image Address
 *  @return     None.
 *  @remarks    Address's Format is 2D Address. so each bit have meaning.
 *              Addr[27:24] : Segment Address.
 *              Addr[23:12] : Base Address Y. Should be arranged in 8-byte. 
 *              Addr[11:0]  : Base Address X. Should be arranged in 8-byte. 
 *  @see        NX_SCALER_SetSrcAddr,   NX_SCALER_GetSrcAddr,
 *                                      NX_SCALER_GetDestAddr
 */
void    NX_SCALER_SetDestAddr( U32 ModuleIndex, U32 DstIndex, U32 Addr )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( DstIndex < 2 );
    //NX_ASSERT( 0 == (0x50007007 & Addr) );
    //NX_ASSERT( 0 == (0x50007007 & Addr) );

    switch( DstIndex )
    {
        case 0: WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG0, Addr);    break;
        case 1: WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG1, Addr);    break;
    }

    //__g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG = Addr;
    //WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG, Addr);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Image Address
 *  @return     Value of Destination Image 2D Address.
 *              SrcAddr[27:24]  : Segment Address.
 *              SrcAddr[23:12]  : Base Address Y.
 *              SrcAddr[11:0]   : Base Address X.
 */
U32     NX_SCALER_GetDestAddr( U32 ModuleIndex, U32 DstIndex )
{
    U32 DestAddr;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( DstIndex < 2 );

    switch( DstIndex )
    {
        case 0: DestAddr = __g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG0;   break;
        case 1: DestAddr = __g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG1;   break;
    }

    //return __g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG;
    return DestAddr;
}

//--------------------------------------------------------------------------
/**
 *  @brief      Set Destination Image Address
 *  @param[in]  Addr        Value of Destination Image Address
 *  @return     None.
 *  @remarks    Address's Format is 2D Address. so each bit have meaning.
 *              Addr[27:24] : Segment Address.
 *              Addr[23:12] : Base Address Y. Should be arranged in 8-byte. 
 *              Addr[11:0]  : Base Address X. Should be arranged in 8-byte. 
 */
void    NX_SCALER_SetDestStride( U32 ModuleIndex, U32 DstIndex, U32 Stride )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( DstIndex < 2 );

    switch( DstIndex )
    {
        case 0: WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTSTRIDE0, Stride);  break;
        case 1: WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTSTRIDE1, Stride);  break;
    }

    //__g_ModuleVariables[ModuleIndex].pRegister->SCDESTADDREG = Addr;
    //WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTSTRIDE, Stride);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Image Address
 *  @return     Value of Destination Image 2D Address.
 *              SrcAddr[27:24]  : Segment Address.
 *              SrcAddr[23:12]  : Base Address Y.
 *              SrcAddr[11:0]   : Base Address X.
 */
U32     NX_SCALER_GetDestStride( U32 ModuleIndex, U32 DstIndex )
{
    U32 DestStride;
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );

    switch( DstIndex )
    {
        case 0: DestStride = __g_ModuleVariables[ModuleIndex].pRegister->SCDESTSTRIDE0; break;
        case 1: DestStride = __g_ModuleVariables[ModuleIndex].pRegister->SCDESTSTRIDE1; break;
    }

    //return __g_ModuleVariables[ModuleIndex].pRegister->SCDESTSTRIDE;
    return DestStride;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Source and Destination Image Size
 *  @param[in]  dwSrcWidth      Value of width ( 8 ~ 4096 )
 *  @param[in]  dwSrcHeight     Value of height( 1 ~ 4096 )
 *  @param[in]  dwDestWidth     Value of width ( 8 ~ 4096 )
 *  @param[in]  dwDestHeight    Value of height( 1 ~ 4096 )
 *  @return     None.
 *  @remarks    Width must align to 8.
 */
void    NX_SCALER_SetImageSize
(
    U32 ModuleIndex, U32 dwSrcWidth, U32 dwSrcHeight, U32 dwDestWidth, U32 dwDestHeight
)
{
    const U32   WIDTH_BITPOS    = 0 ;
    const U32   HEIGHT_BITPOS   = 16 ;
    register U32 temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    NX_ASSERT( ( 0x1000 > dwSrcWidth ) && ( 0 == ( dwSrcWidth & 0x07 )) );
    NX_ASSERT( ( 0x1000 > dwSrcHeight) && ( 0 < dwSrcHeight ) );
    NX_ASSERT( ( 0x1000 > dwDestWidth ) && ( 0 == ( dwDestWidth & 0x07 )) );
    NX_ASSERT( ( 0x1000 > dwDestHeight) && ( 0 < dwDestHeight ) );

    temp = ( ( dwSrcHeight - 1 ) << HEIGHT_BITPOS ) | ( ( dwSrcWidth - 1 ) << WIDTH_BITPOS );
    //__g_ModuleVariables[ModuleIndex].pRegister->SCSRCSIZEREG = temp;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCSRCSIZEREG, temp);

    temp = ( ( dwDestHeight - 1 ) << HEIGHT_BITPOS ) | ( ( dwDestWidth - 1 ) << WIDTH_BITPOS );
    //__g_ModuleVariables[ModuleIndex].pRegister->SCDESTSIZEREG = temp;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SCDESTSIZEREG, temp);

    // Setting DeltaX, DeltaY
    //__g_ModuleVariables[ModuleIndex].pRegister->DELTAXREG = ( dwSrcWidth * 0x10000 ) / ( dwDestWidth-1 );
    //__g_ModuleVariables[ModuleIndex].pRegister->DELTAYREG = ( dwSrcHeight * 0x10000 ) / ( dwDestHeight-1 );
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DELTAXREG, ( dwSrcWidth  * 0x10000 ) / ( dwDestWidth -1 ));
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DELTAYREG, ( dwSrcHeight * 0x10000 ) / ( dwDestHeight-1 ));
    //--------------------------------------------------------------------------
    // 2009.11.10, Goofy : Fix to appear abnormal gray vertical pattern
    //                     in the region is filled with a single gray color.
    //--------------------------------------------------------------------------
    //__g_ModuleVariables[ModuleIndex].pRegister->DELTAXREG = ( dwSrcWidth  * 0x10000 ) / ( dwDestWidth  );
    //__g_ModuleVariables[ModuleIndex].pRegister->DELTAYREG = ( dwSrcHeight * 0x10000 ) / ( dwDestHeight );
    //WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DELTAXREG, ( dwSrcWidth  * 0x10000 ) / ( dwDestWidth  ));
    //WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DELTAYREG, ( dwSrcHeight * 0x10000 ) / ( dwDestHeight ));

    /* NX_CONSOLE_Printf( "[NXLOG] SCALER:: SRC WIDTH  : 0x%d\n", dwSrcWidth-1 ); */
    /* NX_CONSOLE_Printf( "[NXLOG] SCALER:: SRC HEIGHT : 0x%d\n", dwSrcHeight-1 ); */
    /* NX_CONSOLE_Printf( "[NXLOG] SCALER:: DST WIDTH  : 0x%d\n", dwDestWidth-1 ); */
    /* NX_CONSOLE_Printf( "[NXLOG] SCALER:: DST HEIGHT : 0x%d\n", dwDestHeight-1 ); */
    /* NX_CONSOLE_Printf( "[NXLOG] SCALER:: DELTA      : 0x%x\n", ( dwSrcWidth  * 0x10000 ) / ( dwDestWidth -1 )); */
    /* NX_CONSOLE_Printf( "[NXLOG] SCALER:: DELTA      : 0x%x\n", ( dwSrcHeight * 0x10000 ) / ( dwDestHeight-1 )); */
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Source Image's width and height
 *  @param[out] pdwSrcWidth     Value of width
 *  @param[out] pdwSrcHeight    Value of height
 *  @param[out] pdwDestWidth    Value of width
 *  @param[out] pdwDestHeight   Value of height
 *  @return     None.
 *  @remarks    Parameter can set to CNULL, when the information is not need.
 *  @see        NX_SCALER_SetImageSize
 */
void        NX_SCALER_GetImageSize
(
    U32 ModuleIndex, U32 *pdwSrcWidth, U32   *pdwSrcHeight, U32  *pdwDestWidth, U32  *pdwDestHeight
)
{
    const U32   WIDTH_BITPOS    = 0 ;
    const U32   WIDTH_MASK      = ( 0x0FFF << 0 );
    const U32   HEIGHT_BITPOS   = 16 ;
    const U32   HEIGHT_MASK     = ( 0x0FFF <<16 );
    register U32 temp;

    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCSRCSIZEREG;

    if( CNULL != pdwSrcWidth ) { *pdwSrcWidth   = ( ( temp & WIDTH_MASK  ) >> WIDTH_BITPOS  ) + 1;  }
    if( CNULL != pdwSrcHeight) { *pdwSrcHeight  = ( ( temp & HEIGHT_MASK ) >> HEIGHT_BITPOS ) + 1;  }

    temp = __g_ModuleVariables[ModuleIndex].pRegister->SCDESTSIZEREG;

    if( CNULL != pdwDestWidth ) { *pdwDestWidth     = ( ( temp & WIDTH_MASK  ) >> WIDTH_BITPOS  ) + 1;  }
    if( CNULL != pdwDestHeight) { *pdwDestHeight    = ( ( temp & HEIGHT_MASK ) >> HEIGHT_BITPOS ) + 1;  }
}





void    NX_SCALER_SetCmdBufAddr( U32 ModuleIndex, U32 Addr )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CMDBUFADDR, Addr );
}

U32     NX_SCALER_GetCmdBufAddr( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return __g_ModuleVariables[ModuleIndex].pRegister->CMDBUFADDR;
}

void    NX_SCALER_RunCmdBuf( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CMDBUFCON, 0x01 );
}

void    NX_SCALER_StopCmdBuf( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_SCALER_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CMDBUFCON, 0x02 );
}

