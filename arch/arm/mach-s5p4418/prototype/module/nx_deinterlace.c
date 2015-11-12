//------------------------------------------------------------------------------
//  Copyright (C) 2009 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : Deinterlace
//  File        : nx_deinterlace.c
//  Description :
//  Author      :
//  History     :
//      2012/08/08 parkjh - 1st release
//------------------------------------------------------------------------------

#include "nx_chip.h"
#include "nx_deinterlace.h"

static  struct  NX_DEINTERLACE_RegisterSet *__g_pRegister = CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Initialize of prototype enviroment & local variables.
 *  @return     CTRUE    indicates that Initialize is successed.
 *              CFALSE   indicates that Initialize is failed.
 *  @see        NX_DEINTERLACE_GetNumberOfModule
 */
CBOOL   NX_DEINTERLACE_Initialize( void )
{
    static CBOOL bInit = CFALSE;

    if( CFALSE == bInit )
    {
        __g_pRegister = CNULL;

        bInit = CTRUE;
    }

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get number of modules in the chip.
 *  @return     Module's number.
 *  @see        NX_DEINTERLACE_Initialize
 */
U32     NX_DEINTERLACE_GetNumberOfModule( void )
{
    return NUMBER_OF_DEINTERLACE_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get module's physical address.
 *  @return     Module's physical address
 */
U32     NX_DEINTERLACE_GetPhysicalAddress( void )
{
    return  (U32)( PHY_BASEADDR_DEINTERLACE_MODULE );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a size, in byte, of register set.
 *  @return     Size of module's register set.
 */
U32     NX_DEINTERLACE_GetSizeOfRegisterSet( void )
{
    return sizeof( struct NX_DEINTERLACE_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a base address of register set.
 *  @param[in]  BaseAddress Module's base address
 *  @return     None.
 */
void    NX_DEINTERLACE_SetBaseAddress( void* BaseAddress )
{
    NX_ASSERT( CNULL != BaseAddress );

    __g_pRegister = (struct NX_DEINTERLACE_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a base address of register set
 *  @return     Module's base address.
 */
void*    NX_DEINTERLACE_GetBaseAddress( void )
{
    return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Initialize selected modules with default value.
 *  @return     CTRUE    indicates that Initialize is successed. 
 *              CFALSE   indicates that Initialize is failed.
 */
CBOOL   NX_DEINTERLACE_OpenModule( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    //  __g_pRegister->MODE         = 0;
    //  __g_pRegister->INTENB       = 0;
    //  __g_pRegister->INTPEND      = 0x0F;

    //  __g_pRegister->SRAMPORSLEEP = 0;
    //  __g_pRegister->TSPARA       = (( 18<<16) |  10);
    //  __g_pRegister->TMPARA       = (( 16<<16) |  8);
    //  __g_pRegister->TIPARA       = ((306<<16) |  50);
    //  __g_pRegister->TPARA        = ((466<<16) | 434);
    //  __g_pRegister->BLENDPARA    = 3;

    //  __g_pRegister->SRCSIZE_Y    = 0;
    //  __g_pRegister->SRCADDRP_Y   = 0;
    //  __g_pRegister->SRCADDRC_Y   = 0;
    //  __g_pRegister->SRCADDRN_Y   = 0;
    //  __g_pRegister->SRCSTRD_Y    = 0;
    //  __g_pRegister->SRCCONT_Y    = 0x00040000;
    //  __g_pRegister->DESTADDRD_Y  = 0;
    //  __g_pRegister->DESTADDRF_Y  = 0;
    //  __g_pRegister->DESTSTRD_Y   = 0;

    //  __g_pRegister->SRCSIZE_CB   = 0;
    //  __g_pRegister->SRCADDRC_CB  = 0;
    //  __g_pRegister->SRCSTRD_CB   = 0;
    //  __g_pRegister->SRCCONT_CB   = 0x00040000;
    //  __g_pRegister->DESTADDRD_CB = 0;
    //  __g_pRegister->DESTADDRF_CB = 0;
    //  __g_pRegister->DESTSTRD_CB  = 0;

    //  __g_pRegister->SRCSIZE_CR   = 0;
    //  __g_pRegister->SRCADDRC_CR  = 0;
    //  __g_pRegister->SRCSTRD_CR   = 0;
    //  __g_pRegister->SRCCONT_CR   = 0x00040000;
    //  __g_pRegister->DESTADDRD_CR = 0;
    //  __g_pRegister->DESTADDRF_CR = 0;
    //  __g_pRegister->DESTSTRD_CR  = 0;

    WriteIO32(&__g_pRegister->MODE, 0);
    WriteIO32(&__g_pRegister->INTENB, 0);
    WriteIO32(&__g_pRegister->INTPEND, 0x0F);

    WriteIO32(&__g_pRegister->SRAMPORSLEEP, 0);
    WriteIO32(&__g_pRegister->TSPARA, (( 18<<16) |  10));
    WriteIO32(&__g_pRegister->TMPARA, (( 16<<16) |  8));
    WriteIO32(&__g_pRegister->TIPARA, ((306<<16) |  50));
    WriteIO32(&__g_pRegister->TPARA, ((466<<16) | 434));
    WriteIO32(&__g_pRegister->BLENDPARA, 3);
    WriteIO32(&__g_pRegister->LRMASK, 0);

    WriteIO32(&__g_pRegister->CMDSTATE, 0);
    WriteIO32(&__g_pRegister->CMDBUFADDR, 0);

    WriteIO32(&__g_pRegister->SRCSIZE_Y, 0);
    WriteIO32(&__g_pRegister->SRCADDRP_Y, 0);
    WriteIO32(&__g_pRegister->SRCADDRC_Y, 0);
    WriteIO32(&__g_pRegister->SRCADDRN_Y, 0);
    WriteIO32(&__g_pRegister->SRCSTRD_Y, 0);
    WriteIO32(&__g_pRegister->SRCCONT_Y, 0x00040000);
    WriteIO32(&__g_pRegister->DESTADDRD_Y, 0);
    WriteIO32(&__g_pRegister->DESTADDRF_Y, 0);
    WriteIO32(&__g_pRegister->DESTSTRD_Y, 0);

    WriteIO32(&__g_pRegister->SRCSIZE_CB, 0);
    WriteIO32(&__g_pRegister->SRCADDRC_CB, 0);
    WriteIO32(&__g_pRegister->SRCSTRD_CB, 0);
    WriteIO32(&__g_pRegister->SRCCONT_CB, 0x00040000);
    WriteIO32(&__g_pRegister->DESTADDRD_CB, 0);
    WriteIO32(&__g_pRegister->DESTADDRF_CB, 0);
    WriteIO32(&__g_pRegister->DESTSTRD_CB, 0);

    WriteIO32(&__g_pRegister->SRCSIZE_CR, 0);
    WriteIO32(&__g_pRegister->SRCADDRC_CR, 0);
    WriteIO32(&__g_pRegister->SRCSTRD_CR, 0);
    WriteIO32(&__g_pRegister->SRCCONT_CR, 0x00040000);
    WriteIO32(&__g_pRegister->DESTADDRD_CR, 0);
    WriteIO32(&__g_pRegister->DESTADDRF_CR, 0);
    WriteIO32(&__g_pRegister->DESTSTRD_CR, 0);

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinitialize selected module to the proper stage.
 *  @return     CTRUE    indicates that Deinitialize is successed. 
 *              CFALSE   indicates that Deinitialize is failed.
 */
CBOOL   NX_DEINTERLACE_CloseModule( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    //  __g_pRegister->MODE         = 0;
    //  __g_pRegister->INTENB       = 0;
    //  __g_pRegister->INTPEND      = 0x0F;

    //  __g_pRegister->SRAMPORSLEEP = 0;
    //  __g_pRegister->TSPARA       = (( 18<<16) |  10);
    //  __g_pRegister->TMPARA       = (( 16<<16) |  8);
    //  __g_pRegister->TIPARA       = ((306<<16) |  50);
    //  __g_pRegister->TPARA        = ((466<<16) | 434);
    //  __g_pRegister->BLENDPARA    = 3;

    //  __g_pRegister->SRCSIZE_Y    = 0;
    //  __g_pRegister->SRCADDRP_Y   = 0;
    //  __g_pRegister->SRCADDRC_Y   = 0;
    //  __g_pRegister->SRCADDRN_Y   = 0;
    //  __g_pRegister->SRCSTRD_Y    = 0;
    //  __g_pRegister->SRCCONT_Y    = 0x00040000;
    //  __g_pRegister->DESTADDRD_Y  = 0;
    //  __g_pRegister->DESTADDRF_Y  = 0;
    //  __g_pRegister->DESTSTRD_Y   = 0;

    //  __g_pRegister->SRCSIZE_CB   = 0;
    //  __g_pRegister->SRCADDRC_CB  = 0;
    //  __g_pRegister->SRCSTRD_CB   = 0;
    //  __g_pRegister->SRCCONT_CB   = 0x00040000;
    //  __g_pRegister->DESTADDRD_CB = 0;
    //  __g_pRegister->DESTADDRF_CB = 0;
    //  __g_pRegister->DESTSTRD_CB  = 0;

    //  __g_pRegister->SRCSIZE_CR   = 0;
    //  __g_pRegister->SRCADDRC_CR  = 0;
    //  __g_pRegister->SRCSTRD_CR   = 0;
    //  __g_pRegister->SRCCONT_CR   = 0x00040000;
    //  __g_pRegister->DESTADDRD_CR = 0;
    //  __g_pRegister->DESTADDRF_CR = 0;
    //  __g_pRegister->DESTSTRD_CR  = 0;

    WriteIO32(&__g_pRegister->MODE, 0);
    WriteIO32(&__g_pRegister->INTENB, 0);
    WriteIO32(&__g_pRegister->INTPEND, 0x0F);

    WriteIO32(&__g_pRegister->SRAMPORSLEEP, 0);
    WriteIO32(&__g_pRegister->TSPARA, (( 18<<16) |  10));
    WriteIO32(&__g_pRegister->TMPARA, (( 16<<16) |  8));
    WriteIO32(&__g_pRegister->TIPARA, ((306<<16) |  50));
    WriteIO32(&__g_pRegister->TPARA, ((466<<16) | 434));
    WriteIO32(&__g_pRegister->BLENDPARA, 3);
    WriteIO32(&__g_pRegister->LRMASK, 0);

    WriteIO32(&__g_pRegister->CMDSTATE, 0);
    WriteIO32(&__g_pRegister->CMDBUFADDR, 0);

    WriteIO32(&__g_pRegister->SRCSIZE_Y, 0);
    WriteIO32(&__g_pRegister->SRCADDRP_Y, 0);
    WriteIO32(&__g_pRegister->SRCADDRC_Y, 0);
    WriteIO32(&__g_pRegister->SRCADDRN_Y, 0);
    WriteIO32(&__g_pRegister->SRCSTRD_Y, 0);
    WriteIO32(&__g_pRegister->SRCCONT_Y, 0x00040000);
    WriteIO32(&__g_pRegister->DESTADDRD_Y, 0);
    WriteIO32(&__g_pRegister->DESTADDRF_Y, 0);
    WriteIO32(&__g_pRegister->DESTSTRD_Y, 0);

    WriteIO32(&__g_pRegister->SRCSIZE_CB, 0);
    WriteIO32(&__g_pRegister->SRCADDRC_CB, 0);
    WriteIO32(&__g_pRegister->SRCSTRD_CB, 0);
    WriteIO32(&__g_pRegister->SRCCONT_CB, 0x00040000);
    WriteIO32(&__g_pRegister->DESTADDRD_CB, 0);
    WriteIO32(&__g_pRegister->DESTADDRF_CB, 0);
    WriteIO32(&__g_pRegister->DESTSTRD_CB, 0);

    WriteIO32(&__g_pRegister->SRCSIZE_CR, 0);
    WriteIO32(&__g_pRegister->SRCADDRC_CR, 0);
    WriteIO32(&__g_pRegister->SRCSTRD_CR, 0);
    WriteIO32(&__g_pRegister->SRCCONT_CR, 0x00040000);
    WriteIO32(&__g_pRegister->DESTADDRD_CR, 0);
    WriteIO32(&__g_pRegister->DESTADDRF_CR, 0);
    WriteIO32(&__g_pRegister->DESTSTRD_CR, 0);

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the selected modules is busy or not.
 *  @return     CTRUE    indicates that Module is Busy. 
 *              CFALSE   indicates that Module is NOT Busy.
 */
CBOOL   NX_DEINTERLACE_CheckBusy( void )
{
    const U32   cfgIDLE_MASK    = ( 0x01 << 1 );
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & cfgIDLE_MASK ) ? CFALSE : CTRUE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicaes whether the selected modules is ready to enter power-down stage
 *  @return     CTRUE    indicates that Ready to enter power-down stage. 
 *              CFALSE   indicates that This module can't enter to power-down stage.

 */
CBOOL   NX_DEINTERLACE_CanPowerDown( void )
{
    return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number for interrupt controller.
 *  @return     Interrupt number
 */
S32     NX_DEINTERLACE_GetInterruptNumber( void )
{
    return  INTNUM_OF_DEINTERLACE_MODULE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  IntNum  Interrupt Number ( 0:Y, 1:Cb, 2:Cr, 3:YCbCr ).
 *  @param[in]  Enable  CTRUE    indicates that Interrupt Enable. 
 *                      CFALSE   indicates that Interrupt Disable.
 *  @return     None.
 */
void    NX_DEINTERLACE_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
    struct NX_DEINTERLACE_RegisterSet *pRegister;
    register U32    ReadValue;

    NX_ASSERT( (5 > IntNum) && (0 <= IntNum) );
    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    ReadValue = pRegister->INTENB;

    ReadValue &= ~(1<<IntNum);
    ReadValue |= ((U32)Enable << IntNum );

    //pRegister->INTENB = ReadValue;
    WriteIO32(&pRegister->INTENB, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]  IntNum  Interrupt Number( 0:Y, 1:Cb, 2:Cr, 3:YCbCr ).
 *  @return     CTRUE    indicates that Interrupt is enabled. 
 *              CFALSE   indicates that Interrupt is disabled.
 */
CBOOL   NX_DEINTERLACE_GetInterruptEnable( S32 IntNum )
{
    NX_ASSERT( (5 > IntNum) && (0 <= IntNum) );
    NX_ASSERT( CNULL != __g_pRegister );

    return (CBOOL)((__g_pRegister->INTENB >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  EnableFlag  Specify interrupt bit for enable of disable. Each bit's meaning is like below   
 *                          - EnableFlag[0] : Set Y interrupt enable or disable. 
 *                          - EnableFlag[1] : Set Cb interrupt enable or disable. 
 *                          - EnableFlag[2] : Set Cr interrupt enable or disable. 
 *                          - EnableFlag[3] : Set YCbCr interrupt enable or disable. 
 *  @return     None.
 */
void    NX_DEINTERLACE_SetInterruptEnable32( U32 EnableFlag )
{
    const U32   ENB_MASK = 0x1F ;

    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->INTENB = EnableFlag & ENB_MASK;
    WriteIO32(&__g_pRegister->INTENB, EnableFlag & ENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates current setting value of interrupt enable bit.
 *  @return     Current setting value of interrupt. 
 *              "1" means interrupt is enabled. 
 *              "0" means interrupt is disabled. 
 *              - Return Value[0] : Y interrupt's setting value. 
 *              - Return Value[1] : Cb interrupt's setting value. 
 *              - Return Value[2] : Cr interrupt's setting value. 
 *              - Return Value[3] : YCbCr interrupt's setting value. 
 */
U32     NX_DEINTERLACE_GetInterruptEnable32( void )
{
    const U32   ENB_MASK = 0x1F ;

    NX_ASSERT( CNULL != __g_pRegister );

    return (U32)(__g_pRegister->INTENB & ENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]  IntNum  Interrupt Number( 0:Y, 1:Cb, 2:Cr, 3:YCbCr ).
 *  @return     CTRUE    indicates that Pending is seted. 
 *              CFALSE   indicates that Pending is Not Seted.
 */
CBOOL   NX_DEINTERLACE_GetInterruptPending( S32 IntNum )
{
    NX_ASSERT( (5 > IntNum) && (0 <= IntNum) );
    NX_ASSERT( CNULL != __g_pRegister );

    return (CBOOL)((__g_pRegister->INTPEND >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates current setting value of interrupt pending bit.
 *  @return     Current setting value of pending bit. 
 *              "1" means pend bit is occured. 
 *              "0" means pend bit is NOT occured. 
 *              - Return Value[0] : Y pending state. 
 *              - Return Value[0] : Cb pending state. 
 *              - Return Value[0] : Cr pending state. 
 *              - Return Value[0] : YCbCr pending state. 
 */
U32     NX_DEINTERLACE_GetInterruptPending32( void )
{
    const U32 PEND_MASK = 0x1F;

    NX_ASSERT( CNULL != __g_pRegister );

    return (U32)(__g_pRegister->INTPEND & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  IntNum  Interrupt number( 0:Y, 1:Cb, 2:Cr, 3:YCbCr ).
 *  @return     None.
 */
void    NX_DEINTERLACE_ClearInterruptPending( S32 IntNum )
{
    NX_ASSERT( (5 > IntNum) && (0 <= IntNum) );
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->INTPEND = (0x01 << IntNum);
    WriteIO32(&__g_pRegister->INTPEND, (0x01 << IntNum));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  PendingFlag     Specify pend bit to clear. Each bit's meaning is like below  
 *                              - PendingFlag[0] : Y pending bit. 
 *                              - PendingFlag[1] : Cb pending bit. 
 *                              - PendingFlag[2] : Cr pending bit. 
 *                              - PendingFlag[3] : YCbCr pending bit. 
 *  @return     None.
 */
void    NX_DEINTERLACE_ClearInterruptPending32( U32 PendingFlag )
{
    const U32 PEND_MASK = 0x1F;
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->INTPEND = (PendingFlag & PEND_MASK);
    WriteIO32(&__g_pRegister->INTPEND, (PendingFlag & PEND_MASK));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set all interrupts to be enables or disables.
 *  @param[in]  Enable  CTRUE    indicates that Set to all interrupt enable. 
 *                      CFALSE   indicates that Set to all interrupt disable.
 *  @return     None.
 */
void    NX_DEINTERLACE_SetInterruptEnableAll( CBOOL Enable )
{
    const U32 INTENB_MASK = 0x1F;

    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( CNULL != __g_pRegister );

    if( CTRUE == Enable )
    {
        //__g_pRegister->INTENB = INTENB_MASK;
        WriteIO32(&__g_pRegister->INTENB, INTENB_MASK);
    }
    else
    {
        //__g_pRegister->INTENB = 0x00;
        WriteIO32(&__g_pRegister->INTENB, 0x00);
    }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enable or not.
 *  @return     CTRUE    indicates that At least one( or more ) interrupt is enabled. 
 *              CFALSE   indicates that All interrupt is disabled.
 */
CBOOL   NX_DEINTERLACE_GetInterruptEnableAll( void )
{
    const U32 INTENB_MASK = 0x1F;

    NX_ASSERT( CNULL != __g_pRegister );

    if( INTENB_MASK & __g_pRegister->INTENB )
    {
        return CTRUE;
    }

    return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are pended or not.
 *  @return     CTRUE    indicates that At least one( or more ) pending is seted. 
 *              CFALSE   indicates that All pending is NOT seted.
 */
CBOOL   NX_DEINTERLACE_GetInterruptPendingAll( void )
{
    const U32 INTPEND_MASK = 0x1F;

    NX_ASSERT( CNULL != __g_pRegister );

    if( INTPEND_MASK & __g_pRegister->INTPEND )
    {
        return CTRUE;
    }

    return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear pending state of all interrupts.
 *  @return     None.
 */
void    NX_DEINTERLACE_ClearInterruptPendingAll( void )
{
    const U32 INTPEND_MASK = 0x1F;

    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->INTPEND = INTPEND_MASK;
    WriteIO32(&__g_pRegister->INTPEND, INTPEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number which has the most prority of pended interrupts
 *  @return     Pending Number( If all pending is not set then return -1 ).
 *              0:Y, 1:Cb, 2:Cr, 3:YCbCr.
 */
S32     NX_DEINTERLACE_GetInterruptPendingNumber( void )    // -1 if None
{
    register struct NX_DEINTERLACE_RegisterSet  *pRegister;
    register U32 dwPend;
    U32 dwIntNum;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    dwPend = ( pRegister->INTENB & pRegister->INTPEND);

    for( dwIntNum=0; dwIntNum < 5; dwIntNum++ )
    {
        if( dwPend & (1<<dwIntNum) )
        {
            return dwIntNum;
        }
    }

    return -1;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a PCLK mode
 *  @param[in]  mode    PCLK mode
 *  @return     None.
 *  @remark     Deinterlace doesn't support NX_PCLKMODE_DYNAMIC. If you call this
 *              function with NX_PCLKMODE_DYNAMIC, it makes that PCLK doesn't
 *              provide to the Deinterlace module and Deinterlace module doesn't operate
 *                  correctly. You have to set a PCLK mode as NX_PCLKMODE_ALWAYS
 *                  to operate and control the Deinterlace module. But you can set a PCLK
 *              mode as NX_PCLKMODE_DYNAMIC to slightly reduce a power
 *              cunsumption if you don't want to use the Deinterlace module.
 */
void    NX_DEINTERLACE_SetClockPClkMode( NX_PCLKMODE mode )
{
    const U32 PCLKMODE_POS  =   3;

    register U32 regvalue;
    register struct NX_DEINTERLACE_RegisterSet* pRegister;

    U32 clkmode=0;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

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

//------------------------------------------------------------------------------
/**
 *  @brief      Get current PCLK mode
 *  @return     Current PCLK mode
 */
NX_PCLKMODE NX_DEINTERLACE_GetClockPClkMode( void )
{
    const U32 PCLKMODE_POS  = 3;

    NX_ASSERT( CNULL != __g_pRegister );

    if( __g_pRegister->CLKENB & ( 1UL << PCLKMODE_POS ) )
    {
        return NX_PCLKMODE_ALWAYS;
    }

    return  NX_PCLKMODE_DYNAMIC;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set System Bus Clock's operation Mode
 *  @param[in]  mode        BCLK Mode
 *  @return     None.
 *  @remark     Deinterlace doesn't support NX_BCLKMODE_DYNAMIC. If you call this
 *              function with NX_BCLKMODE_DYNAMIC, it makes that BCLK doesn't
 *              provide to the Deinterlace module and Deinterlace module doesn't operate
 *                  correctly. You have to set a PCLK mode as NX_BCLKMODE_ALWAYS
 *                  to operate and control the Deinterlace module. But you can set a BCLK
 *              mode as NX_BCLKMODE_DYNAMIC to slightly reduce a power
 *              cunsumption if you don't want to use the Deinterlace module.
 */
void NX_DEINTERLACE_SetClockBClkMode( NX_BCLKMODE mode )
{
    register U32 regvalue;
    register struct NX_DEINTERLACE_RegisterSet* pRegister;
    U32 clkmode=0;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

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

//------------------------------------------------------------------------------
/**
 *  @brief      Get System Bus Clock's operation Mode
 *  @return     BCLK Mode
 */
NX_BCLKMODE NX_DEINTERLACE_GetClockBClkMode( void )
{
    const U32 BCLKMODE  = 3UL<<0;

    NX_ASSERT( CNULL != __g_pRegister );

    switch( __g_pRegister->CLKENB & BCLKMODE )
    {
        case 0 :    return NX_BCLKMODE_DISABLE;
        case 2 :    return NX_BCLKMODE_DYNAMIC;
        case 3 :    return NX_BCLKMODE_ALWAYS;
    }

    NX_ASSERT( CFALSE );
    return NX_BCLKMODE_DISABLE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get System Bus Reset Number
 *  @return     Reset Number
 */
U32         NX_DEINTERLACE_GetClockNumber( void )
{
    // PHY_BASEADDR_CLKGEN_MODULE
    static const U32 CLKGEN_DEINTERLACELIST[] =
    {
        CLOCKINDEX_LIST( DEINTERLACE )
    };

    return (U32)CLKGEN_DEINTERLACELIST[0];
}

U32         NX_DEINTERLACE_GetResetNumber( void )
{
    const U32 ResetPortNumber[NUMBER_OF_DEINTERLACE_MODULE] =
    {
        RESETINDEX_LIST( DEINTERLACE, i_nRST )
    };

    return (U32)ResetPortNumber[0];
}

//------------------------------------------------------------------------------
// Deinterlace Configuration Function
//------------------------------------------------------------------------------
/**
 *  @brief      Set AS module parameter.
 *  @param[in]  dwTS1   AS module parameter(TS1 Value : 0 ~ 255).
 *  @param[in]  dwTS2   AS module parameter(TS2 Value : 0 ~ 255).
 *  @return     None.
 */
void    NX_DEINTERLACE_SetASParameter( U32 dwTS1, U32 dwTS2 )
{
    const U32 TS2_BITPOS = 16;

    NX_ASSERT((1<<8) > dwTS1);
    NX_ASSERT((1<<8) > dwTS2);
    NX_ASSERT( CNULL != __g_pRegister )

    //__g_pRegister->TSPARA = ((dwTS2<<TS2_BITPOS) | dwTS1);
    WriteIO32(&__g_pRegister->TSPARA, ((dwTS2<<TS2_BITPOS) | dwTS1));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get AS module parameter.
 *  @param[out] pdwTS1  AS module parameter(TS1 Value : 0 ~ 255).
 *  @param[out] pdwTS2  AS module parameter(TS2 Value : 0 ~ 255).
 *  @return     None.
 */
void    NX_DEINTERLACE_GetASParameter( U32 *pdwTS1, U32 *pdwTS2 )
{
    const U32 TS1_MASK      = 0xFF;
    const U32 TS2_BITPOS    = 16;
    const U32 TS2_MASK      = 0xFF << TS2_BITPOS;

    U32 ReadValue;

    NX_ASSERT( CNULL != __g_pRegister )

    ReadValue = __g_pRegister->TSPARA;

    if( CNULL != pdwTS1 ){ *pdwTS1 = ReadValue & TS1_MASK;  }
    if( CNULL != pdwTS2 ){ *pdwTS2 = (ReadValue & TS2_MASK) >> TS2_BITPOS;  }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set MDSAD module parameter.
 *  @param[in]  dwTM1   MDSAD module parameter(TM1 Value : 0 ~ 255).
 *  @param[in]  dwTM2   MDSAD module parameter(TM2 Value : 0 ~ 255).
 *  @return     None.
 */
void    NX_DEINTERLACE_SetMDSADParameter( U32 dwTM1, U32 dwTM2 )
{
    const U32 TM2_BITPOS = 16;

    NX_ASSERT((1<<8) > dwTM1);
    NX_ASSERT((1<<8) > dwTM2);
    NX_ASSERT( CNULL != __g_pRegister )

    //__g_pRegister->TMPARA = ((dwTM2<<TM2_BITPOS) | dwTM1);
    WriteIO32(&__g_pRegister->TMPARA, ((dwTM2<<TM2_BITPOS) | dwTM1));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get MDSAD module parameter.
 *  @param[out] pdwTM1  MDSAD module parameter(TM1 Value : 0 ~ 255).
 *  @param[out] pdwTM2  MDSAD module parameter(TM2 Value : 0 ~ 255).
 *  @return     None.
 */
void    NX_DEINTERLACE_GetMDSADParameter( U32 *pdwTM1, U32 *pdwTM2 )
{
    const U32 TM1_MASK      = 0xFF;
    const U32 TM2_BITPOS    = 16;
    const U32 TM2_MASK      = 0xFF << TM2_BITPOS;

    U32 ReadValue;

    NX_ASSERT( CNULL != __g_pRegister )

    ReadValue = __g_pRegister->TMPARA;

    if( CNULL != pdwTM1 ){ *pdwTM1 = ReadValue & TM1_MASK;  }
    if( CNULL != pdwTM2 ){ *pdwTM2 = (ReadValue & TM2_MASK) >> TM2_BITPOS;  }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set MI module parameter.
 *  @param[in]  dwTI1   MI module parameter(TI1 Value : 0 ~ 65535).
 *  @param[in]  dwTI2   MI module parameter(TI2 Value : 0 ~ 65535).
 *  @return     None.
 */
void    NX_DEINTERLACE_SetMIParameter( U32 dwTI1, U32 dwTI2 )
{
    const U32 TI2_BITPOS = 16;

    NX_ASSERT((1<<16) > dwTI1);
    NX_ASSERT((1<<16) > dwTI2);
    NX_ASSERT( CNULL != __g_pRegister )

    //__g_pRegister->TIPARA = ((dwTI2<<TI2_BITPOS) | dwTI1);
    WriteIO32(&__g_pRegister->TIPARA, ((dwTI2<<TI2_BITPOS) | dwTI1));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get MI module parameter.
 *  @param[out] pdwTI1  MI module parameter(TI1 Value : 0 ~ 65535).
 *  @param[out] pdwTI2  MI module parameter(TI2 Value : 0 ~ 65535).
 *  @return     None.
 */
void    NX_DEINTERLACE_GetMIParameter( U32 *pdwTI1, U32 *pdwTI2 )
{
    const U32 TI1_MASK      = 0xFFFF;
    const U32 TI2_BITPOS    = 16;
    const U32 TI2_MASK      = 0xFFFF << TI2_BITPOS;

    U32 ReadValue;

    NX_ASSERT( CNULL != __g_pRegister )

    ReadValue = __g_pRegister->TIPARA;

    if( CNULL != pdwTI1 ){ *pdwTI1 = ReadValue & TI1_MASK;  }
    if( CNULL != pdwTI2 ){ *pdwTI2 = (ReadValue & TI2_MASK) >> TI2_BITPOS;  }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set YS module parameter.
 *  @param[in]  dwT1    YS module parameter(T1 Value : 0 ~ 511).
 *  @param[in]  dwT2    YS module parameter(T2 Value : 0 ~ 511).
 *  @return     None.
 */
void    NX_DEINTERLACE_SetYSParameter( U32 dwT1, U32 dwT2 )
{
    const U32 T2_BITPOS = 16;

    NX_ASSERT((1<<9) > dwT1);
    NX_ASSERT((1<<9) > dwT2);
    NX_ASSERT( CNULL != __g_pRegister )

    //__g_pRegister->TPARA = ((dwT2<<T2_BITPOS) | dwT1);
    WriteIO32(&__g_pRegister->TPARA, ((dwT2<<T2_BITPOS) | dwT1));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get YS module parameter.
 *  @param[out] pdwT1   YS module parameter(T1 Value : 0 ~ 511).
 *  @param[out] pdwT2   YS module parameter(T2 Value : 0 ~ 511).
 *  @return     None.
 */
void    NX_DEINTERLACE_GetYSParameter( U32 *pdwT1, U32 *pdwT2 )
{
    const U32 T1_MASK   = 0x1FF;
    const U32 T2_BITPOS = 16;
    const U32 T2_MASK   = 0x1FF << T2_BITPOS;

    U32 ReadValue;

    NX_ASSERT( CNULL != __g_pRegister )

    ReadValue = __g_pRegister->TPARA;

    if( CNULL != pdwT1 ){ *pdwT1 = ReadValue & T1_MASK; }
    if( CNULL != pdwT2 ){ *pdwT2 = (ReadValue & T2_MASK) >> T2_BITPOS;  }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Blend module parameter.
 *  @param[in]  dwShift Blend module parameter(Shift Value : 0 ~ 15).
 *  @return     None.
 */
void    NX_DEINTERLACE_SetBLENDParameter( U32 dwShift )
{
    NX_ASSERT((1<<4) > dwShift);
    NX_ASSERT( CNULL != __g_pRegister )

    //__g_pRegister->BLENDPARA = dwShift;
    WriteIO32(&__g_pRegister->BLENDPARA, dwShift);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get BLEND module parameter.
 *  @param[out] pdwShift    BLEND module parameter(Shift Value).
 *  @return     None.
 */
void    NX_DEINTERLACE_GetBLENDParameter( U32 *pdwShift )
{
    const U32 SHIFTVALUE_MASK = 0xF;

    U32 ReadValue;

    NX_ASSERT( CNULL != __g_pRegister )

    ReadValue = __g_pRegister->BLENDPARA;

    if( CNULL != pdwShift ){ *pdwShift = ReadValue & SHIFTVALUE_MASK;   }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Burst Control
 *  @param[in]  burst       Value of Burst Control Mode ( BURST_4 or BURST_8 )
 *  @return     None.
 *  @remarks    If Set Burst_4 mode then deitnerlace write data to memory 4 count.
 *              If Set Burst_8 mode then deitnerlace write data to memory 8 count.
 */
/*
void    NX_DEINTERLACE_SetBurstControl
(
    NX_DEINTERLACE_BURST burst
)
{
    const U32   START_MASK      = ( 1 << 0 );
    const U32   IDLE_MASK       = ( 1 << 1 );
    const U32   BURST_BITPOS    = 2 ;
    const U32   BURST_MASK      = ( 1 << BURST_BITPOS );

    register U32    temp;

    NX_ASSERT( NX_DEINTERLACE_BURST_4 >= burst );
    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    temp &= ~( START_MASK | IDLE_MASK | BURST_MASK );
    temp |= (U32)burst << BURST_BITPOS;

    //__g_pRegister->MODE = temp;
    WriteIO32(&__g_pRegister->MODE, temp);
}
*/

//------------------------------------------------------------------------------
/*
 *  @brief      Get Burst Control
 *  @return     Value of Burst Control Mode ( BURST_4 or BURST_8 )
 */
/*
NX_DEINTERLACE_BURST    NX_DEINTERLACE_GetBurstControl( void )
{
    const U32   BURST_BITPOS    = 2;
    const U32   BURST_MASK      = ( 0x01 << BURST_BITPOS );
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    temp = ( temp & BURST_MASK ) >> BURST_BITPOS ;

    return ( NX_DEINTERLACE_BURST )temp;
}
*/

//--------------------------------------------------------------------------
/**
 *  @brief      Set CR Field Information
 *  @param[in]  field       Value of CR Field ( Odd or Even )
 *  @return     None.
 */
void    NX_DEINTERLACE_SetCRFieldInfo
(
    NX_DEINTERLACE_FIELD field
)
{
    const   U32 START_MASK  = ( 1 << 0 );
    const   U32 IDLE_MASK   = ( 1 << 1 );
    const   U32 CR_BITPOS   = 14 ;
    const   U32 CR_MASK     = ( 1 << CR_BITPOS );

    struct NX_DEINTERLACE_RegisterSet *pRegister;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( NX_DEINTERLACE_FIELD_ODD >= field );

    pRegister = __g_pRegister;

    temp = pRegister->MODE ;

    temp &= ~( CR_MASK | START_MASK | IDLE_MASK );
    temp |= (U32)field << CR_BITPOS;

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CR Field Information
 *  @return     Value of CR Field ( Odd or Even )
 */
NX_DEINTERLACE_FIELD    NX_DEINTERLACE_GetCRFieldInfo( void )
{
    const U32   CR_BITPOS   = 14;
    const U32   CR_MASK     = ( 0x01 << CR_BITPOS );

    NX_ASSERT( CNULL != __g_pRegister );

    return (NX_DEINTERLACE_FIELD)((__g_pRegister->MODE & CR_MASK) >> CR_BITPOS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CB Field Information
 *  @param[in]  field       Value of CB Field ( Odd or Even )
 *  @return     None.
 */

void    NX_DEINTERLACE_SetCBFieldInfo
(
    NX_DEINTERLACE_FIELD field
)
{
    const   U32 START_MASK  = ( 1 << 0 );
    const   U32 IDLE_MASK   = ( 1 << 1 );
    const   U32 CB_BITPOS   =   13;
    const   U32 CB_MASK     = ( 0x01 << CB_BITPOS );

    struct NX_DEINTERLACE_RegisterSet *pRegister;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( NX_DEINTERLACE_FIELD_ODD >= field );

    pRegister = __g_pRegister;

    temp = pRegister->MODE ;

    temp &= ~( CB_MASK | START_MASK | IDLE_MASK );
    temp |= (U32)field << CB_BITPOS;

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CB Field Information
 *  @return     Value of CB Field ( Odd or Even )
 */
NX_DEINTERLACE_FIELD    NX_DEINTERLACE_GetCBFieldInfo( void )
{
    const U32   CB_BITPOS   = 13;
    const U32   CB_MASK     = ( 0x01 << CB_BITPOS );

    NX_ASSERT( CNULL != __g_pRegister );

    return (NX_DEINTERLACE_FIELD)((__g_pRegister->MODE & CB_MASK) >> CB_BITPOS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Field Information
 *  @param[in]  field       Value of Y Field ( Odd or Even )
 *  @return     None.
 */

void    NX_DEINTERLACE_SetYFieldInfo
(
    NX_DEINTERLACE_FIELD field
)
{
    const U32 START_MASK    = ( 1 << 0 );
    const U32 IDLE_MASK     = ( 1 << 1 );
    const U32 Y_BITPOS      = 12 ;
    const U32 Y_MASK        = ( 0x01 << Y_BITPOS );

    struct NX_DEINTERLACE_RegisterSet *pRegister;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( NX_DEINTERLACE_FIELD_ODD >= field );

    pRegister = __g_pRegister;

    temp = pRegister->MODE ;

    temp &= ~( START_MASK | IDLE_MASK | Y_MASK );
    temp |= (U32)field << Y_BITPOS;

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field Information
 *  @return     Value of Y Field ( Odd or Even )
 */

NX_DEINTERLACE_FIELD    NX_DEINTERLACE_GetYFieldInfo( void )
{
    const U32   Y_BITPOS    = 12;
    const U32   Y_MASK      = ( 0x01 << Y_BITPOS );

    NX_ASSERT( CNULL != __g_pRegister );

    return (NX_DEINTERLACE_FIELD)((__g_pRegister->MODE & Y_MASK) >> Y_BITPOS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y, CB, CR Field Information
 *  @param[in]  Yfield      Value of Y Field ( Odd or Even )
 *  @param[in]  CBfield     Value of CB Field ( Odd or Even )
 *  @param[in]  CRfield     Value of CR Field ( Odd or Even )
 *  @return     None.
 */
void    NX_DEINTERLACE_SetYCBCRField
(
    NX_DEINTERLACE_FIELD    Yfield,
    NX_DEINTERLACE_FIELD    CBfield,
    NX_DEINTERLACE_FIELD    CRfield
)
{
    const   U32 START_MASK  = ( 1 << 0 );
    const   U32 IDLE_MASK   = ( 1 << 1 );
    const   U32 FIELD_MASK  = ( 0x7 << 12 );
    const   U32 Y_BITPOS    = 12;
    const   U32 CB_BITPOS   = 13;
    const   U32 CR_BITPOS   = 14;
    register U32    temp;
    struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( NX_DEINTERLACE_FIELD_ODD >= Yfield );
    NX_ASSERT( NX_DEINTERLACE_FIELD_ODD >= CBfield );
    NX_ASSERT( NX_DEINTERLACE_FIELD_ODD >= CRfield );

    pRegister = __g_pRegister;

    temp = pRegister->MODE;
    temp &= ~(FIELD_MASK | START_MASK | IDLE_MASK);
    temp |= ( (U32)Yfield << Y_BITPOS ) | ( (U32)CBfield << CB_BITPOS ) | ( (U32)CRfield << CR_BITPOS );

    //pRegister->MODE =   temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set left right masking enable
 *  @param[in]  LMaskEnb    Left Masking Enable
 *  @param[in]  RMaskEnb    Right Masking Enable
 */
void    NX_DEINTERLACE_SetLRMask( U32 LMaskEnb, U32 RMaskEnb )
{
    const   U32 RIGHT_BITPOS    = 0;
    const   U32 LEFT_BITPOS     = 1;

    register U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = ( ((U32)LMaskEnb << LEFT_BITPOS) | ((U32)RMaskEnb << RIGHT_BITPOS ));
    //__g_pRegister->LRMASK = temp;
    WriteIO32(&__g_pRegister->LRMASK, temp);
}

//--------------------------------------------------------------------------
// Deinterlace Operation Function
//------------------------------------------------------------------------------
/**
 *  @brief      Set Deinterlace buffer power
 *  @param[in]  enable  CTRUE    Deinterlace buffer is power on.
 *                      CFALSE   Deinterlace buffer is power off.
 *  @return     None.
 *  @remarks    Deinterlace buffer should on before deinterlace to enable.
 */
void    NX_DEINTERLACE_SetBufferPowerOn( CBOOL enable )
{
    // Warnning prevention.
    enable = enable;
    return;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get deinterlace's buffer status
 *  @return     CTRUE    Indicate that deinterlace buffer is power on. 
 *              CFALSE   Indicate that deinterlace buffer is power off.

 */
CBOOL   NX_DEINTERLACE_GetBufferPowerOn( void )
{
    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set deinterlace's buffer sleep mode
 *  @param[in]  enable  CTRUE   Sleep mode enable. 
 *                      CFALSE  Sleep mode disable. 
 *  @return     None.
 *  @remarks    Sleep mode only avaliable when buffer is power on.

 */
void    NX_DEINTERLACE_SetBufferPowerSleep( CBOOL enable )
{
    const U32 POWER_MASK = (0x1F<<16);
    const U32 SLEEP_MASK = (0x1F<<0);
    struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( (0==enable) || (1==enable) );
    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    if( enable )
    {
        //pRegister->SRAMPORSLEEP = 0;
        WriteIO32(&pRegister->SRAMPORSLEEP, 0);
    }
    else
    {
        //pRegister->SRAMPORSLEEP = SLEEP_MASK;
        WriteIO32(&pRegister->SRAMPORSLEEP, POWER_MASK|SLEEP_MASK);
    }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get deinterlace's buffer sleep mode status
 *  @return     CTRUE    Indicate that deinterlace buffer is sleep mode on. 
 *              CFALSE   Indicate that deinterlace buffer is sleep mode off.
 */
CBOOL   NX_DEINTERLACE_GetBufferPowerSleep( void )
{
    const U32 SLEEP_MASK = 0x1F;

    NX_ASSERT( CNULL != __g_pRegister )

    if( SLEEP_MASK & __g_pRegister->SRAMPORSLEEP )
    {
        return CFALSE;
    }

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CR Operation
 *  @param[in]  enable      CTRUE( CR Enable ), CFALSE( CR Disable )
 *  @return     None.
 */
void    NX_DEINTERLACE_SetCREnable
(
    CBOOL enable
)
{
    const U32 START_MASK    = ( 1 << 0 );
    const U32 IDLE_MASK     = ( 1 << 1 );
    const U32 CRENB_MASK    = ( 0x01 << 10 );
    register U32 temp;
    struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    temp = pRegister->MODE ;

    temp &= ~( CRENB_MASK | START_MASK | IDLE_MASK );

    if( CTRUE == enable )
    {
        temp |= CRENB_MASK;
    }

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CR Operation
 *  @return     CTRUE indicates that CR Deinterlace is enabled.
 *              CFALSE indicates that CR Deinterlace is disable.
 */
CBOOL   NX_DEINTERLACE_GetCREnable( void )
{
    const       U32 CRENB_MASK  = ( 0x01 << 10 );
    register    U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & CRENB_MASK ) ? CTRUE : CFALSE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CB Operation
 *  @param[in]  enable      CTRUE( CB Enable ), CFALSE( CB Disable )
 *  @return     None.
 */
void    NX_DEINTERLACE_SetCBEnable
(
    CBOOL enable
)
{
    const U32 START_MASK    = ( 1 << 0 );
    const U32 IDLE_MASK     = ( 1 << 1 );
    const U32 CBENB_MASK    = ( 0x01 << 9 );
    register struct NX_DEINTERLACE_RegisterSet *pRegister;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    temp = pRegister->MODE ;

    temp &= ~( CBENB_MASK | START_MASK | IDLE_MASK );

    if( CTRUE == enable )
    {
        temp |= CBENB_MASK;
    }

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CB Operation
 *  @return     CTRUE indicates that CB Deinterlace is enabled.
 *              CFALSE indicates that CB Deinterlace is disable.
 */
CBOOL   NX_DEINTERLACE_GetCBEnable( void )
{
    const       U32 CBENB_MASK  = ( 0x01 << 9 );
    register    U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & CBENB_MASK ) ? CTRUE : CFALSE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Operation
 *  @param[in]  enable      CTRUE( Y Enable ), CFALSE( Y Disable )
 *  @return     None.
 */
void    NX_DEINTERLACE_SetYEnable
(
    CBOOL enable
)
{
    const U32 START_MASK    = ( 1 << 0 );
    const U32 IDLE_MASK     = ( 1 << 1 );
    const U32 YENB_MASK     = ( 0x01 << 8 );
    register struct NX_DEINTERLACE_RegisterSet *pRegister;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    temp = pRegister->MODE ;

    temp &= ~( START_MASK | IDLE_MASK | YENB_MASK );

    if( CTRUE ==    enable )
    {
        temp |= YENB_MASK;
    }

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Operation
 *  @return     CTRUE indicates that Y Deinterlace is enabled.
 *              CFALSE indicates that Y Deinterlace is disable.
 */
CBOOL   NX_DEINTERLACE_GetYEnable( void )
{
    const       U32 YENB_MASK   = ( 0x01 << 8 );
    register    U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & YENB_MASK ) ? CTRUE : CFALSE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y, CB, CR Operation
 *  @param[in]  Yenable     CTRUE( Y Enable ), CFALSE( Y Disable )
 *  @param[in]  CBenable        CTRUE( CB Enable ), CFALSE( CB Disable )
 *  @param[in]  CRenable        CTRUE( CR Enable ), CFALSE( CR Disable )
 *  @return     None.
 */
void    NX_DEINTERLACE_SetYCBCREnable
(
    CBOOL Yenable,
    CBOOL CBenable,
    CBOOL CRenable
)
{
    const U32 START_MASK    = ( 1 << 0 );
    const U32 IDLE_MASK     = ( 1 << 1 );
    const U32 ENB_MASK      = ( 0x7UL << 8 );
    const U32 YENB_MASK     = ( 0x01UL << 8 );
    const U32 CBENB_MASK    = ( 0x01UL << 9 );
    const U32 CRENB_MASK    = ( 0x01UL << 10 );

    register U32 temp;
    struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    temp = pRegister->MODE;

    temp &= ~(ENB_MASK | START_MASK | IDLE_MASK);

    if( CTRUE == Yenable )  temp |= YENB_MASK;
    if( CTRUE == CBenable ) temp |= CBENB_MASK;
    if( CTRUE == CRenable ) temp |= CRENB_MASK;

    //pRegister->MODE =   temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Status of CR
 *  @return     CTRUE indicates that CR is Busy.
 *              CFALSE indicates that CR is Idle.
 */
CBOOL   NX_DEINTERLACE_IsCRBusy( void )
{
    const U32   CRBUSY_MASK = ( 0x01 << 6 );
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & CRBUSY_MASK ) ? CTRUE : CFALSE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Status of CB
 *  @return     CTRUE indicates that CB is Busy.
 *              CFALSE indicates that CB is Idle.
 */
CBOOL   NX_DEINTERLACE_IsCBBusy( void )
{
    const U32   CBBUSY_MASK = ( 0x01 << 5 );
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & CBBUSY_MASK ) ? CTRUE : CFALSE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Status of Y
 *  @return     CTRUE indicates that Y is Busy.
 *              CFALSE indicates that Y is Idle.
 */
CBOOL   NX_DEINTERLACE_IsYBusy( void )
{
    const U32   YBUSY_MASK  = ( 0x01 << 4 );
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = __g_pRegister->MODE ;

    return ( temp & YBUSY_MASK ) ? CTRUE : CFALSE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Check Deinterlace is running or idle
 *  @return     CTRUE indicates that Deinterlace is running.
 *              CFALSE indicates that Deinterlace is idle.
 */
CBOOL   NX_DEINTERLACE_IsDeinterlaceBusy( void )
{
    const   U32 BUSY_MASK = ( 0x01 << 1 );

    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->MODE & BUSY_MASK ) ? CFALSE : CTRUE ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Check Deinterlace is running or idle
 *  @return     CTRUE indicates that Deinterlace is running.
 *              CFALSE indicates that Deinterlace is idle.
 */
void    NX_DEINTERLACE_IsDeinterlaceSwReset( void )
{
    const U32 RESET_MASK = ( 0x01 << 1 );
    register struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    WriteIO32(&pRegister->MODE, RESET_MASK);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinterlace Start
 *  @return     None.
 */
void    NX_DEINTERLACE_DeinterlaceStart( void )
{
    const   U32 START_MASK  = ( 0x01 << 0 );
    const   U32 IDLE_MASK   = ( 0x01 << 1 );
    register U32    temp;
    register struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    temp = pRegister->MODE;

    temp =  ( temp & ~IDLE_MASK ) | START_MASK;

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinterlace Stop
 *  @return     None.
 *  @remarks    Deinterlace is stopped mandatorily by this function
 */

void    NX_DEINTERLACE_DeinterlaceStop( void )
{
    const   U32 STOP_MASK = ( 0x01 << 1 );
    register U32    temp;
    struct NX_DEINTERLACE_RegisterSet *pRegister;

    NX_ASSERT( CNULL != __g_pRegister );

    pRegister = __g_pRegister;

    temp = pRegister->MODE;

    temp |= STOP_MASK;

    //pRegister->MODE = temp;
    WriteIO32(&pRegister->MODE, temp);
}

//------------------------------------------------------------------------------
// Y Field Configuration Function
//------------------------------------------------------------------------------
/**
 *  @brief      Set Size of Source Y Field Image
 *  @param[in]  YSrcHeight      Height  ( 0 ~ 1023 )
 *  @param[in]  YSrcWidth       Width   ( 0 ~ 1023 )
 *  @return     None
 *  @remarks    Each Height and Width can set maxium 10bit value.
 */

void    NX_DEINTERLACE_SetYSrcImageSize
(
    U32 YSrcHeight,
    U32 YSrcWidth
)
{
    const U32   YHEIGHT_BITPOS  = 16 ;
    const U32   YWIDTH_BITPOS   = 0 ;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( 1024 > YSrcHeight );
    NX_ASSERT( 1024 > YSrcWidth );

    temp = ( YSrcHeight << YHEIGHT_BITPOS ) | ( YSrcWidth << YWIDTH_BITPOS ) ;

    //__g_pRegister->SRCSIZE_Y    = temp;
    WriteIO32(&__g_pRegister->SRCSIZE_Y, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Field's Previous Address of Source Image
 *  @param[in]  YSrcAddrPrev        Value of Y Field's Previous Address of Source Image ( 0 ~ 0xFFFFFFFF )
 *  @return     None
 */

void    NX_DEINTERLACE_SetYSrcAddrPrev
(
    U32 YSrcAddrPrev
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->SRCADDRP_Y = YSrcAddrPrev;
    WriteIO32(&__g_pRegister->SRCADDRP_Y, YSrcAddrPrev);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Field's Current Address of Source Image
 *  @param[in]  YSrcAddrCurr        Value of Y Field's Currnet Address of Source Image ( 0 ~ 0xFFFFFFFF )
 *  @return     None
 */
void    NX_DEINTERLACE_SetYSrcAddrCurr
(
    U32 YSrcAddrCurr
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->SRCADDRC_Y   = YSrcAddrCurr;
    WriteIO32(&__g_pRegister->SRCADDRC_Y, YSrcAddrCurr);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Field's Next Address of Source Image
 *  @param[in]  YSrcAddrNext        Value of Y Field's Next Address of Source Image ( 0 ~ 0xFFFFFFFF )
 *  @return     None
 */
void    NX_DEINTERLACE_SetYSrcAddrNext
(
    U32 YSrcAddrNext
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->SRCADDRN_Y   = YSrcAddrNext;
    WriteIO32(&__g_pRegister->SRCADDRN_Y, YSrcAddrNext);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Field's Horizontal and Vertical Stride of Source Image
 *  @param[in]  YSrcVStride     Vertical Stride ( 0 ~ 0xFFFF )
 *  @param[in]  YSrcHStride     Horizontal Stride ( 0 ~ 0xFFFF )
 *  @return     None
 *  @remarks    Each Horizontal and Vertical stride can set maxium 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetYSrcStride
(
    U32 YSrcVStride,
    U32 YSrcHStride
)
{
    const   U32 VSTRIDE_BITPOS = 16 ;
    const   U32 HSTRIDE_BITPOS  = 0 ;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( 0x10000 > YSrcVStride );
    NX_ASSERT( 0x10000 > YSrcHStride );

    temp = ( YSrcVStride << VSTRIDE_BITPOS  ) | ( YSrcHStride << HSTRIDE_BITPOS );

    //__g_pRegister->SRCSTRD_Y    = temp;
    WriteIO32(&__g_pRegister->SRCSTRD_Y, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Read Control
 *  @param[in]  DirEnb      Direction of read data from cache
 *  @param[in]  ByteSize    Read Data Byte Size
 *  @param[in]  VOffset     Vertical Position Offset
 *  @param[in]  HOffset     Horizontal Position Offset
 *  @return     None
 *  @remarks    Each Horizontal and Vertical stride can set maxium 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetYSrcControl
(
    NX_DEINTERLACE_DIR      DirEnb,
    NX_DEINTERLACE_BITWIDTH ByteSize,
    U8                      VOffset,
    U8                      HOffset
)
{
    const   U32 DIRECT_ENB_BITPOS   = 18;
    const   U32 BYTE_SIZE_BITPOS    = 16;
    const   U32 VOFFSET_BITPOS      =  8;
    const   U32 HOFFSET_BITPOS      =  0;
    register U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = ( DirEnb     << DIRECT_ENB_BITPOS ) | ( ByteSize   << BYTE_SIZE_BITPOS  )
         | ( VOffset    << VOFFSET_BITPOS    ) | ( HOffset    << HOFFSET_BITPOS    );

    //__g_pRegister->SRCSTRD_Y    = temp;
    WriteIO32(&__g_pRegister->SRCCONT_Y, temp);

}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Destination Address of Deinterlaced Y
 *  @param[in]  YDestAddrDIT        Dest Addres of Deinterlaced Y ( 0 ~ 0xFFFFFFFF )
 *  @return     None.
 *  @remarks    Deinterlaced Y Data is saved this address.
 */

void    NX_DEINTERLACE_SetYDestAddrDIT
(
    U32 YDestAddrDIT
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTADDRD_Y  =   YDestAddrDIT;
    WriteIO32(&__g_pRegister->DESTADDRD_Y,  YDestAddrDIT);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Destination Address of Field Y
 *  @param[in]  YDestAddrFil        Destination Addres of Field Y ( 0 ~ 0xFFFFFFFF )
 *  @return     None.
 *  @remarks    Current Y Field's Data is coppied to this address.
 */

void    NX_DEINTERLACE_SetYDestAddrFil
(
    U32 YDestAddrFil
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTADDRF_Y  =   YDestAddrFil;
    WriteIO32(&__g_pRegister->DESTADDRF_Y,  YDestAddrFil);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Y Field's Vertical Stride for Destination Image
 *  @param[in]  YDestStride Vertical Stride ( 0 ~ 0xFFFF )
 *  @return     None.
 *  @remarks    Vertical stride can set maximum 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetYDestStride
(
    U32 YDestStride
)
{
    NX_ASSERT( 0x10000 > YDestStride );
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTSTRD_Y = YDestStride;
    WriteIO32(&__g_pRegister->DESTSTRD_Y,   YDestStride);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field Image Size
 *  @return     Value of Image Size
 *  @remarks    Height is saved to return value's [25:16]bit.
 *              Width is saved to return values's [9:0]bit.
 *              Each Height and Width's range ( 0 ~ 1023 )
 */
U32     NX_DEINTERLACE_GetYSrcImageSize( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCSIZE_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field's Previous Address
 *  @return     Value of Y Field's Previous Address ( 0 ~ 0xFFFFFFFF )
 */

U32     NX_DEINTERLACE_GetYSrcAddrPrev ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCADDRP_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field's Current Address
 *  @return     Value of Y Field's Current Address ( 0 ~ 0xFFFFFFFF )
 */
U32     NX_DEINTERLACE_GetYSrcAddrCurr ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCADDRC_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field's Next Address
 *  @return     Value of Y Field's Next Address ( 0 ~ 0xFFFFFFFF )
 */
U32     NX_DEINTERLACE_GetYSrcAddrNext ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCADDRN_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field's Stride of Source Image
 *  @return     Value of Y Field's Horizontal and Vertical Stride
 *  @remarks    Horizontal Stride is saved to return value's [15:0]bit.
 *              Vertical Stride is saved to return values's [31:16]bit.
 */

U32     NX_DEINTERLACE_GetYSrcStride    ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCSTRD_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Address of Deinterlaced Y
 *  @return     Value of Destination Address ( Deinterlaced Y Data is stored )
 */

U32     NX_DEINTERLACE_GetYDestAddrDIT ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTADDRD_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Address of Field Y
 *  @return     Value of Destination Address ( Current Y Field Data is stored )
 */

U32     NX_DEINTERLACE_GetYDestAddrFil ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTADDRF_Y );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Y Field's Vertical Stride of Destination Image
 *  @return     Value of Vertical Stride ( 0 ~ 0xFFFF )
 */

U32     NX_DEINTERLACE_GetYDestStride   ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTSTRD_Y );
}


//------------------------------------------------------------------------------
// CB Field Configuration Function
//------------------------------------------------------------------------------
/**
 *  @brief      Set Size of Source CB Field Image
 *  @param[in]  CBSrcHeight         Height ( 0 ~ 1023 )
 *  @param[in]  CBSrcWidth          Width   ( 0 ~ 1023 )
 *  @return     None
 *  @remarks    Each Height and Width can set maximum 10bit value.
 */

void    NX_DEINTERLACE_SetCBSrcImageSize
(
    U32 CBSrcHeight,
    U32 CBSrcWidth
)
{
    const U32   CBHEIGHT_BITPOS = 16 ;
    const U32   CBWIDTH_BITPOS  = 0 ;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( 1024 > CBSrcHeight );
    NX_ASSERT( 1024 > CBSrcWidth );

    temp = ( CBSrcHeight << CBHEIGHT_BITPOS ) | ( CBSrcWidth << CBWIDTH_BITPOS ) ;

    //__g_pRegister->SRCSIZE_CB   = temp;
    WriteIO32(&__g_pRegister->SRCSIZE_CB, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CB Field's Current Address of Source Image
 *  @param[in]  CBSrcAddrCurr       Value of CB Field's Currnet Address of Source Image ( 0 ~ 0xFFFFFFFF )
 *  @return     None
 */

void    NX_DEINTERLACE_SetCBSrcAddrCurr
(
    U32 CBSrcAddrCurr
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->SRCADDRC_CB  = CBSrcAddrCurr;
    WriteIO32(&__g_pRegister->SRCADDRC_CB, CBSrcAddrCurr);
}


//------------------------------------------------------------------------------
/**
 *  @brief      Set CB Field's Horizontal and Vertical Stride of Source Image
 *  @param[in]  CBSrcVStride        Vertical Stride ( 0 ~ 0xFFFF )
 *  @param[in]  CBSrcHStride        Horizontal Stride ( 0 ~ 0xFFFF )
 *  @return     None
 *  @remarks    Each Horizontal and Vertical stride can set maximum 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetCBSrcStride
(
    U32 CBSrcVStride,
    U32 CBSrcHStride
)
{
    const   U32 VSTRIDE_BITPOS  = 16 ;
    const   U32 HSTRIDE_BITPOS  = 0 ;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( 0x10000 > CBSrcVStride );
    NX_ASSERT( 0x10000 > CBSrcHStride );

    temp = ( CBSrcVStride << VSTRIDE_BITPOS ) | ( CBSrcHStride << HSTRIDE_BITPOS );

    //__g_pRegister->SRCSTRD_CB   = temp;
    WriteIO32(&__g_pRegister->SRCSTRD_CB, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CB Read Control
 *  @param[in]  DirEnb      Direction of read data from cache
 *  @param[in]  ByteSize    Read Data Byte Size
 *  @param[in]  VOffset     Vertical Position Offset
 *  @param[in]  HOffset     Horizontal Position Offset
 *  @return     None
 *  @remarks
 */
void    NX_DEINTERLACE_SetCBSrcControl
(
    NX_DEINTERLACE_DIR      DirEnb,
    NX_DEINTERLACE_BITWIDTH ByteSize,
    U8                      VOffset,
    U8                      HOffset
)
{
    const   U32 DIRECT_ENB_BITPOS   = 18;
    const   U32 BYTE_SIZE_BITPOS    = 16;
    const   U32 VOFFSET_BITPOS      =  8;
    const   U32 HOFFSET_BITPOS      =  0;
    register U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = ( DirEnb     << DIRECT_ENB_BITPOS ) | ( ByteSize   << BYTE_SIZE_BITPOS  )
         | ( VOffset    << VOFFSET_BITPOS    ) | ( HOffset    << HOFFSET_BITPOS    );

    //__g_pRegister->SRCSTRD_Y    = temp;
    WriteIO32(&__g_pRegister->SRCCONT_CB, temp);

}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Destination Address of Deinterlaced CB
 *  @param[in]  CBDestAddrDIT       Dest Addres of Deinterlaced CB ( 0 ~ 0xFFFFFFFF)
 *  @return     None.
 *  @remarks    Deinterlaced CB Data is saved this address.
 */

void    NX_DEINTERLACE_SetCBDestAddrDIT
(
    U32 CBDestAddrDIT
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTADDRD_CB =   CBDestAddrDIT;
    WriteIO32(&__g_pRegister->DESTADDRD_CB, CBDestAddrDIT);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Destination Address of Field CB
 *  @param[in]  CBDestAddrFil       Destination Addres of Field CB ( 0 ~ 0xFFFFFFFF )
 *  @return     None.
 *  @remarks    Current CB Field's Data is coppied to this address.
 *  @see        NX_DEINTERLACE_GetCBDestAddrFil
 */

void    NX_DEINTERLACE_SetCBDestAddrFil
(
    U32 CBDestAddrFil
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTADDRF_CB =   CBDestAddrFil;
    WriteIO32(&__g_pRegister->DESTADDRF_CB, CBDestAddrFil);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CB Field's Stride for Destination Image
 *  @param[in]  CBDestStride    Vertical Stride ( 0 ~ 0xFFFF )
 *  @return     None.
 *  @remarks    Vertical stride can set maximum 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetCBDestStride
(
    U32 CBDestStride
)
{
    NX_ASSERT( 0x10000 > CBDestStride );
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTSTRD_CB =    CBDestStride;
    WriteIO32(&__g_pRegister->DESTSTRD_CB,  CBDestStride);
}


//------------------------------------------------------------------------------
/**
 *  @brief      Get CB Field Image Size
 *  @return     Value of Image Size
 *  @remarks    Height is saved to return value's [25:16]bit.
 *              Width is saved to return values's [9:0]bit.
 *              Each Height and Width's range ( 0 ~ 1023 )
 */
U32     NX_DEINTERLACE_GetCBSrcImageSize( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCSIZE_CB );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CB Field's Current Address
 *  @return     Value of CB Field's Current Address ( 0 ~ 0xFFFFFFFF )
 */
U32     NX_DEINTERLACE_GetCBSrcAddrCurr ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCADDRC_CB );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CB Field's Stride of Source Image
 *  @return     Value of CB Field's Horizontal and Vertical Stride
 *  @remarks    Horizontal Stride is saved to return value's [15:0]bit.
 *              Vertical Stride is saved to return values's [31:16]bit.
 */
U32     NX_DEINTERLACE_GetCBSrcStride   ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCSTRD_CB );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Address of Deinterlaced CB
 *  @return     Value of Destination Address ( Deinterlaced CB  Data is stored )
 */

U32     NX_DEINTERLACE_GetCBDestAddrDIT ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTADDRD_CB );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Address of Field CB
 *  @return     Value of Destination Address ( Current CB Field Data is stored )
 */

U32     NX_DEINTERLACE_GetCBDestAddrFil ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTADDRF_CB );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CB Field's Vertical Stride of Destination Image
 *  @return     Value of Vertical Stride ( 0 ~ 0xFFFF )
 */

U32     NX_DEINTERLACE_GetCBDestStride  ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTSTRD_CB );
}


//------------------------------------------------------------------------------
// CR Field Configuration Function
//------------------------------------------------------------------------------
/**
 *  @brief      Set Size of Source CR Field Image
 *  @param[in]  CRSrcHeight     Height ( 0 ~ 1023 )
 *  @param[in]  CRSrcWidth      Width   ( 0 ~ 1023 )
 *  @return     None
 *  @remarks    Each Height and Width can set maximum 10bit value.
 */

void    NX_DEINTERLACE_SetCRSrcImageSize
(
    U32 CRSrcHeight,
    U32 CRSrcWidth
)
{
    const U32   CRHEIGHT_BITPOS = 16 ;
    const U32   CRWIDTH_BITPOS  = 0 ;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( 1024 > CRSrcHeight );
    NX_ASSERT( 1024 > CRSrcWidth );

    temp = ( CRSrcHeight << CRHEIGHT_BITPOS ) | ( CRSrcWidth << CRWIDTH_BITPOS ) ;

    //__g_pRegister->SRCSIZE_CR   = temp;
    WriteIO32(&__g_pRegister->SRCSIZE_CR, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CR Field's Current Address of Source Image
 *  @param[in]  CRSrcAddrCurr   Value of CR Field's Currnet Address of Source Image ( 0xFFFFFFFF )
 *  @return     None
 */

void    NX_DEINTERLACE_SetCRSrcAddrCurr
(
    U32 CRSrcAddrCurr
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->SRCADDRC_CR  = CRSrcAddrCurr;
    WriteIO32(&__g_pRegister->SRCADDRC_CR, CRSrcAddrCurr);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CR Field's Horizontal and Vertical Stride of Source Image
 *  @param[in]  CRSrcVStride        Vertical Stride     ( 0 ~ 0xFFFF )
 *  @param[in]  CRSrcHStride        Horizontal Stride   ( 0 ~ 0xFFFF )
 *  @return     None
 *  @remarks    Each Horizontal and Vertical stride can set maximum 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetCRSrcStride
(
    U32 CRSrcVStride,
    U32 CRSrcHStride
)
{
    const   U32 VSTRIDE_BITPOS = 16 ;
    const   U32 HSTRIDE_BITPOS  = 0 ;
    register U32    temp;

    NX_ASSERT( CNULL != __g_pRegister );
    NX_ASSERT( 0x10000 > CRSrcVStride );
    NX_ASSERT( 0x10000 > CRSrcHStride );

    temp = ( CRSrcVStride << VSTRIDE_BITPOS ) | ( CRSrcHStride << HSTRIDE_BITPOS );

    //__g_pRegister->SRCSTRD_CR   = temp;
    WriteIO32(&__g_pRegister->SRCSTRD_CR, temp);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CR Read Control
 *  @param[in]  DirEnb      Direction of read data from cache
 *  @param[in]  ByteSize    Read Data Byte Size
 *  @param[in]  VOffset     Vertical Position Offset
 *  @param[in]  HOffset     Horizontal Position Offset
 *  @return     None
 *  @remarks
 */
void    NX_DEINTERLACE_SetCRSrcControl
(
    NX_DEINTERLACE_DIR      DirEnb,
    NX_DEINTERLACE_BITWIDTH ByteSize,
    U8                      VOffset,
    U8                      HOffset
)
{
    const   U32 DIRECT_ENB_BITPOS   = 18;
    const   U32 BYTE_SIZE_BITPOS    = 16;
    const   U32 VOFFSET_BITPOS      =  8;
    const   U32 HOFFSET_BITPOS      =  0;
    register U32 temp;

    NX_ASSERT( CNULL != __g_pRegister );

    temp = ( DirEnb     << DIRECT_ENB_BITPOS ) | ( ByteSize   << BYTE_SIZE_BITPOS  )
         | ( VOffset    << VOFFSET_BITPOS    ) | ( HOffset    << HOFFSET_BITPOS    );

    //__g_pRegister->SRCSTRD_Y    = temp;
    WriteIO32(&__g_pRegister->SRCCONT_CR, temp);

}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Destination Address of Deinterlaced CR
 *  @param[in]  CRDestAddrDIT       Dest Addres of Deinterlaced CR ( 0 ~ 0xFFFFFFFF )
 *  @return     None.
 *  @remarks    Deinterlaced CR Data is saved this address.
 */

void    NX_DEINTERLACE_SetCRDestAddrDIT
(
    U32 CRDestAddrDIT
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTADDRD_CR =   CRDestAddrDIT;
    WriteIO32(&__g_pRegister->DESTADDRD_CR, CRDestAddrDIT);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set Destination Address of Field CR
 *  @param[in]  CRDestAddrFil       Destination Addres of Field CR ( 0 ~ 0xFFFFFFFF )
 *  @return     None.
 *  @remarks    Current CR Field's Data is coppied to this address.
 */

void    NX_DEINTERLACE_SetCRDestAddrFil
(
    U32 CRDestAddrFil
)
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTADDRF_CR =   CRDestAddrFil;
    WriteIO32(&__g_pRegister->DESTADDRF_CR, CRDestAddrFil);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set CR Field's Stride for Destination Image
 *  @param[in]  CRDestStride    Vertical Stride ( 0 ~ 0xFFFF )
 *  @return     None.
 *  @remarks    Vertical stride can set maximum 16bit value.
 *              Stride range ( 0 ~ 0xFFFF ).
 */
void    NX_DEINTERLACE_SetCRDestStride
(
    U32 CRDestStride
)
{
    NX_ASSERT( 0x10000 > CRDestStride );
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->DESTSTRD_CR =    CRDestStride;
    WriteIO32(&__g_pRegister->DESTSTRD_CR,  CRDestStride);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CR Field Image Size
 *  @return     Value of Image Size
 *  @remarks    Height is saved to return value's [25:16]bit.
 *              Width is saved to return values's [9:0]bit.
 *              Each Height and Width's range ( 0 ~ 1023 )
 */
U32     NX_DEINTERLACE_GetCRSrcImageSize( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCSIZE_CR );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CR Field's Current Address
 *  @return     Value of CR Field's Current Address ( 0 ~ 0xFFFFFFFF )
 *  @see        NX_DEINTERLACE_SetCRSrcAddrCurr
 */
U32     NX_DEINTERLACE_GetCRSrcAddrCurr ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCADDRC_CR );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CR Field's Stride of Source Image
 *  @return     Value of CR Field's Horizontal and Vertical Stride
 *  @remarks    Horizontal Stride is saved to return value's [15:0]bit.
 *              Vertical Stride is saved to return values's [31:16]bit.
 */
U32     NX_DEINTERLACE_GetCRSrcStride   ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->SRCSTRD_CR );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Address of Deinterlaced CR
 *  @return     Value of Destination Address ( Deinterlaced CR  Data is stored )
 *  @see        NX_DEINTERLACE_SetCRDestAddrDIT
 */

U32     NX_DEINTERLACE_GetCRDestAddrDIT ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTADDRD_CR );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get Destination Address of Field CR
 *  @return     Value of Destination Address ( Current CR Field Data is stored )
 */

U32     NX_DEINTERLACE_GetCRDestAddrFil ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTADDRF_CR );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get CR Field's Vertical Stride of Destination Image
 *  @return     Value of Vertical Stride ( 0 ~ 0xFFFF )
 *  @see        NX_DEINTERLACE_SetCRDestStride
 */

U32     NX_DEINTERLACE_GetCRDestStride  ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->DESTSTRD_CR );
}

//------------------------------------------------------------------------------
//  Command buffer configuration function
//------------------------------------------------------------------------------
void    NX_DEINTERLACE_CmdBufStart      ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->CMDSTATE =    0x01;
    WriteIO32( &__g_pRegister->CMDSTATE, 0x01 );
}

void    NX_DEINTERLACE_CmdBufStop       ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->CMDSTATE =    0x02;
    WriteIO32( &__g_pRegister->CMDSTATE, 0x02 );
}

void    NX_DEINTERLACE_SetCmdBufAddr    ( U32 CmdBufAddr )
{
    NX_ASSERT( CNULL != __g_pRegister );

    //__g_pRegister->CMDBUFADDR =    CmdBufAddr;
    WriteIO32( &__g_pRegister->CMDBUFADDR, CmdBufAddr );
}

U32     NX_DEINTERLACE_GetCmdBufAddr    ( void )
{
    NX_ASSERT( CNULL != __g_pRegister );

    return ( __g_pRegister->CMDBUFADDR );
}

