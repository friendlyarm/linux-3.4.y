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
//  Module     : I2C
//  File       : nx_i2c.c
//  Description:
//  Author     : Firmware Team
//  History    : 2012.08.01 parkjh - sec02034_I2S proto type
//------------------------------------------------------------------------------

#include "nx_chip.h"
#include "nx_i2c.h"


static  struct
{
    struct NX_I2C_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_I2C_MODULE] = { {CNULL,}, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *  @brief  Initialize of prototype enviroment & local variables.
 *  @return CTRUE  indicates that Initialize is succeeded.
 *          CFALSE indicates that Initialize is failed.

 */
CBOOL   NX_I2C_Initialize( void )
{
    //  @modified Gamza static variable(__g_ModuleVariables) is automatically filled by '0'
    //                  만약 초기화 과정에 전역변수를 0으로 초기화 하는 작업 이외의 일을
    //                  해야한다면 bInit 값을 CFALSE로 수정해야한다.
    static CBOOL bInit = CTRUE;
    U32 i;

    if( CFALSE == bInit )
    {
        for( i=0; i < NUMBER_OF_I2C_MODULE; i++ )
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
 */
U32     NX_I2C_GetNumberOfModule( void )
{
    return NUMBER_OF_I2C_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get module's physical address.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Module's physical address
 */
U32     NX_I2C_GetPhysicalAddress( U32 ModuleIndex )
{
    static const U32 I2CPhysicalAddr[NUMBER_OF_I2C_MODULE] =
    {
        PHY_BASEADDR_LIST( I2C )
    };

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return  (U32)I2CPhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a size, in byte, of register set.
 *  @return     Size of module's register set.
 */
U32     NX_I2C_GetSizeOfRegisterSet( void )
{
    return sizeof( struct NX_I2C_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a base address of register set.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  BaseAddress Module's base address
 *  @return     None.
 */
void    NX_I2C_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
    NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    __g_ModuleVariables[ModuleIndex].pRegister = (struct NX_I2C_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a base address of register set
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Module's base address.
 */
void*    NX_I2C_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Initialize selected modules with default value.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     CTRUE  indicates that Initialize is succeeded. 
 *              CFALSE indicates that Initialize is failed.

 */
CBOOL   NX_I2C_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinitialize selected module to the proper stage.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     CTRUE  indicates that Deinitialize is succeeded. 
 *              CFALSE indicates that Deinitialize is failed.
 */
CBOOL   NX_I2C_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the selected modules is busy or not.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     CTRUE  indicates that Module is Busy. 
 *              CFALSE indicates that Module is NOT Busy.

 */
CBOOL   NX_I2C_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return NX_I2C_IsBusy( ModuleIndex );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicaes whether the selected modules is ready to enter power-down stage
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     CTRUE  indicates that Ready to enter power-down stage. 
 *              CFALSE indicates that This module can't enter to power-down stage.
 */
CBOOL   NX_I2C_CanPowerDown( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number for interrupt controller.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Interrupt number

 */
S32     NX_I2C_GetInterruptNumber( U32 ModuleIndex )
{
    const U32   I2CInterruptNumber[NUMBER_OF_I2C_MODULE] = { INTNUM_LIST( I2C ) };

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return  I2CInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  IntNum  Interrupt Number ( 0 ).
 *  @param[in]  Enable  CTRUE  indicates that Interrupt Enable. 
 *                      CFALSE indicates that Interrupt Disable.
 *  @return     None.
 *  @remarks    I2C Module have one interrupt. So always  IntNum set to 0.
 */
void    NX_I2C_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;
    IntNum = IntNum;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =   pRegister->ICCR;

    ReadValue   &=  ~IRQ_ENB_MASK;
    ReadValue   |=  (U32)Enable << IRQ_ENB_POS;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  IntNum  Interrupt Number ( 0 ).
 *  @return     CTRUE  indicates that Interrupt is enabled. 
 *              CFALSE indicates that Interrupt is disabled.
 *  @remarks    I2C Module have one interrupt. So always  IntNum set to 0.
 */
CBOOL   NX_I2C_GetInterruptEnable( U32 ModuleIndex, S32 IntNum )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;
    IntNum = IntNum;
    
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & IRQ_ENB_MASK) >> IRQ_ENB_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  EnableFlag  Specify interrupt bit for enable of disable. Each bit's meaning is like below  
 *                          - EnableFlag[0] : Set TX/RX interrupt enable or disable. 
 *  @return     None.
 */
void    NX_I2C_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =   pRegister->ICCR;

    ReadValue   &=  ~IRQ_ENB_MASK;
    ReadValue   |=  (U32)(EnableFlag & 0x01) << IRQ_ENB_POS;

    //pRegister->ICCR  =   ReadValue;
    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates current setting value of interrupt enable bit.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current setting value of interrupt. 
 *              "1" means interrupt is enabled. 
 *              "0" means interrupt is disabled. 
 *              - Return Value[0] : TX/RX interrupt's setting value. 
 */
U32     NX_I2C_GetInterruptEnable32( U32 ModuleIndex )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (U32)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & IRQ_ENB_MASK) >> IRQ_ENB_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  IntNum  Interrupt Number ( 0 ).
 *  @return     CTRUE  indicates that Pending is seted. 
 *              CFALSE indicates that Pending is Not Seted.
 *  @remarks    I2C Module have one interrupt. So always  IntNum set to 0.
 */
CBOOL   NX_I2C_GetInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    const U32 PEND_POS   = 4;
    const U32 PEND_MASK  = 1UL << PEND_POS;
    IntNum = IntNum;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & PEND_MASK) >> PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates current setting value of interrupt pending bit.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current setting value of pending bit. 
 *              "1" means pend bit is occured. 
 *              "0" means pend bit is NOT occured. 
 *              - Return Value[0] : TX/RX pending state. 
 */
U32     NX_I2C_GetInterruptPending32( U32 ModuleIndex )
{
    const U32 PEND_POS   = 4;
    const U32 PEND_MASK  = 1UL << PEND_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (U32)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & PEND_MASK) >> PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  IntNum          Interrupt number ( 0 ).
 *  @return     None.
 *  @remarks    I2C Module have one interrupt. So always  IntNum set to 0.
 */
void	NX_I2C_ClearInterruptPending( U32 ModuleIndex, S32 IntNum )
{
	const U32 PEND_POS	 = 4;
	const U32 INTC_POS	 = 8;
	const U32 PEND_MASK  = 1UL << PEND_POS;
	const U32 INTC_MASK  = 1UL << INTC_POS;

	register struct NX_I2C_RegisterSet* pRegister;
	register U32 ReadValue;

    IntNum = IntNum;
	NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	//NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	ReadValue	=	pRegister->ICCR;
	ReadValue	&=	~PEND_MASK;
	ReadValue	|=	INTC_MASK;

	//__g_ModuleVariables[ModuleIndex].pRegister->ICCR = ReadValue;
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ICCR, ReadValue);
}


//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  PendingFlag     Specify pend bit to clear. Each bit's meaning is like below   
 *                              - PendingFlag[0] : TX/RX pending bit. 
 *  @return     None.
 */
void    NX_I2C_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
    const U32 PEND_POS   = 4;
    const U32 INTC_POS   = 8;
    const U32 PEND_MASK  = 1UL << PEND_POS;
    const U32 INTC_MASK  = 1UL << INTC_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;
    ReadValue   =   pRegister->ICCR;
    ReadValue   &=  (~PEND_MASK | (~PendingFlag << PEND_POS ));
    ReadValue   |=  INTC_MASK;

    //__g_ModuleVariables[ModuleIndex].pRegister->ICCR = ReadValue;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set all interrupts to be enables or disables.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  Enable  CTRUE  indicates that Set to all interrupt enable. 
 *                      CFALSE indicates that Set to all interrupt disable.
 *  @return     None.
 */
void    NX_I2C_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( (0==Enable) ||(1==Enable) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =   pRegister->ICCR;

    ReadValue   &=  ~IRQ_ENB_MASK;
    ReadValue   |=  (U32)Enable << IRQ_ENB_POS;

    //pRegister->ICCR  =   ReadValue;
    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enable or not.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     CTRUE  indicates that At least one( or more ) interrupt is enabled. 
 *              CFALSE indicates that All interrupt is disabled.
 */
CBOOL   NX_I2C_GetInterruptEnableAll( U32 ModuleIndex )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & IRQ_ENB_MASK) >> IRQ_ENB_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are pended or not.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     CTRUE  indicates that At least one( or more ) pending is seted. 
 *              CFALSE indicates that All pending is NOT seted.
 */
CBOOL   NX_I2C_GetInterruptPendingAll( U32 ModuleIndex )
{
    const U32 PEND_POS   = 4;
    const U32 PEND_MASK  = 1UL << PEND_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & PEND_MASK) >> PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear pending state of all interrupts.
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     None.
 */
void    NX_I2C_ClearInterruptPendingAll( U32 ModuleIndex )
{
    const U32 PEND_POS   = 4;
    const U32 INTC_POS   = 8;
    const U32 PEND_MASK  = 1UL << PEND_POS;
    const U32 INTC_MASK  = 1UL << INTC_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;
    ReadValue   =   pRegister->ICCR;
    ReadValue   &=  ~PEND_MASK;
    ReadValue   |=  INTC_MASK;

    //__g_ModuleVariables[ModuleIndex].pRegister->ICCR = ReadValue;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number which has the most prority of pended interrupts
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Pending Number( If all pending is not set then return -1 ).
 */
S32     NX_I2C_GetInterruptPendingNumber( U32 ModuleIndex )  // -1 if None
{
    const U32 IRQ_ENB_POS = 5;
    const U32 PEND_POS  = 4;
    //const U32 PEND_MASK = 1UL << PEND_POS;

    const U32   I2CInterruptNumber[NUMBER_OF_I2C_MODULE] = { INTNUM_LIST( I2C ) };

    register struct NX_I2C_RegisterSet *pRegister;
    register U32 Pend;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    Pend = ((pRegister->ICCR >> IRQ_ENB_POS) & (pRegister->ICCR >> PEND_POS));

    if( Pend & 0x01 )
    {
        return  I2CInterruptNumber[ModuleIndex];
    }

    return -1;
}

//------------------------------------------------------------------------------
// Clock Control Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Set a PCLK mode
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  mode    PCLK mode
 *  @return     None.
 *  @remarks    I2C controller only support NX_PCLKMODE_ALWAYS.
 *              If user set to NX_PCLKMODE_DYNAMIC, then I2C controller NOT operate.
 */
/*
void            NX_I2C_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode )
{
    const U32 PCLKMODE_POS  =   3;

    register U32 regvalue;
    register struct NX_I2C_RegisterSet* pRegister;

    U32 clkmode=0;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    switch(mode)
    {
        case NX_PCLKMODE_DYNAMIC:  clkmode = 0;     break;
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
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current PCLK mode
 *  @remarks    I2C controller only support NX_PCLKMODE_ALWAYS.
 *              If user set to NX_PCLKMODE_DYNAMIC, then I2C controller NOT operate.
 */
/*
NX_PCLKMODE    NX_I2C_GetClockPClkMode( U32 ModuleIndex )
{
    const U32 PCLKMODE_POS  = 3;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
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
 *  @brief      Get current reset number
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current reset number
 *  @remarks
 */
U32     NX_I2C_GetClockNumber( U32 ModuleIndex )
{
    // PHY_BASEADDR_CLKGEN_MODULE
    static const U32 CLKGEN_I2CLIST[] =
    {
        CLOCKINDEX_LIST( I2C )
    };

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return (U32)CLKGEN_I2CLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get current reset number
 *  @param[in]  ModuleIndex     A index of module.
 *  @return     Current reset number
 *  @remarks
 */
U32     NX_I2C_GetResetNumber( U32 ModuleIndex )
{
    const U32 ResetPortNumber[NUMBER_OF_I2C_MODULE] =
    {
        RESETINDEX_LIST( I2C, PRESETn )
    };

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return (U32)ResetPortNumber[ModuleIndex];
}


//--------------------------------------------------------------------------
// Configuration Function
//--------------------------------------------------------------------------
/**
 *  @brief      Set PCLK divider and clock prescaler.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  PclkDivider     Set PCLK divider ( 16, 256 )
 *  @param[in]  Prescaler       Set prescaler. 
 *                              2 ~ 16 ( when PCLK divider is seted 16 ). 
 *                              1 ~ 16 ( when PCLK divider is seted 256 ).
 *  @return     None.
 */
void    NX_I2C_SetClockPrescaler( U32 ModuleIndex, U32 PclkDivider, U32 Prescaler )
{
    const U32   CLKSRC_POS  = 6;
    const U32   CLKSRC_MASK = 1UL << CLKSRC_POS;
    const U32   CLK_SCALER_MASK = 0x0F;

    register U32 SetPclkDivider=0;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( (16==PclkDivider) || (256==PclkDivider) );
    NX_ASSERT( 1 <= Prescaler && Prescaler <= 16 );
    NX_ASSERT( (16!=PclkDivider) || ( 2 <= Prescaler && Prescaler <= 16) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    if( 16 == PclkDivider )
    {
        SetPclkDivider = 0;
    }
    else if( 256 == PclkDivider )
    {
        SetPclkDivider = 1;
    }

    ReadValue   =   __g_ModuleVariables[ModuleIndex].pRegister->ICCR;

    ReadValue   &=  ~( CLKSRC_MASK | CLK_SCALER_MASK );
    ReadValue   |=  ((SetPclkDivider << CLKSRC_POS) | (Prescaler-1));

    //__g_ModuleVariables[ModuleIndex].pRegister->ICCR = ReadValue;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set PCLK divider and clock prescaler.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[out] pPclkDivider     Get PCLK divider ( 16, 256 )
 *  @param[out] pPrescaler       Get prescaler. 
 *  @return     None.
 */
void    NX_I2C_GetClockPrescaler( U32 ModuleIndex, U32* pPclkDivider, U32* pPrescaler )
{
    const U32   CLKSRC_POS  = 6;
    const U32   CLKSRC_MASK = 1UL << CLKSRC_POS;
    const U32   CLK_SCALER_MASK = 0x0F;

    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != pPclkDivider );
    NX_ASSERT( CNULL != pPrescaler );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    ReadValue   =   __g_ModuleVariables[ModuleIndex].pRegister->ICCR;

    if( ReadValue & CLKSRC_MASK )
    {
        if(CNULL != pPclkDivider) { *pPclkDivider = 256; }
    }
    else
    {
        if(CNULL != pPclkDivider) { *pPclkDivider = 16; }
    }

    if(CNULL != pPrescaler) { *pPrescaler = (ReadValue & CLK_SCALER_MASK)+1; }
}

//------------------------------------------------------------------------------
/**
 * @brief       Set Salve Address
 * @param[in]   ModuleIndex     A index of module.
 * @param[in]   Address    Value of Salve Address ( 0x02 ~ 0xFE )
 * @return      None.
 * @remarks     LSB[0] bit must set to 0. Slave Address can change ONLY ICSR[TXRX_ENB] bit is 0.
 */
void    NX_I2C_SetSlaveAddress( U32 ModuleIndex, U8 Address )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( 0 == (Address & 0x01) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->IAR  =   (U32)Address;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IAR, (U32)Address);
}

U32    NX_I2C_GetSlaveAddress( U32 ModuleIndex )
{
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    ReadValue   =   (__g_ModuleVariables[ModuleIndex].pRegister->IAR) & 0x7F;

    return ReadValue;
}


//------------------------------------------------------------------------------
/**
 * @brief       Set delay, in PCLKs, between SCL and SDA
 * @param[in]   ModuleIndex     A index of module.
 * @param[in]   delay    number of PCLK ( 1 ~ 32 ) for delay
 * @return      None.
 * @remarks     SDA must be changed at center of low state of SCL from I2C spec.
 *              For this, you have to set delay for SDA change position from falling edge of SCL when TX.
 *              This delay is also used for SDA fetch postiion from rising edge of SCL when RX.
 *              Usually this dealy is 1/4 of SCL period in PCLKs.
 */
/*
void    NX_I2C_SetDataDelay( U32 ModuleIndex, U32 delay )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( 1 <= delay && delay <= 32 );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->QCNT_MAX = delay-1;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->QCNT_MAX, delay-1);
}
*/

//------------------------------------------------------------------------------
/**
 * @brief       Get PCLK number from rising or falling edge to data.
 * @param[in]   ModuleIndex     A index of module.
 * @return      Number of PCLK ( 1 ~ 32 )
 * @remarks     SDA must be changed at center of low state of SCL from I2C spec.
 *              For this, you have to set delay for SDA change position from falling edge of SCL when TX.
 *              This delay is also used for SDA fetch postiion from rising edge of SCL when RX.
 *              Usually this dealy is 1/4 of SCL period in PCLKs.
 */
/*
U32     NX_I2C_GetDataDelay ( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (__g_ModuleVariables[ModuleIndex].pRegister->QCNT_MAX + 1);
}
*/

//------------------------------------------------------------------------------
// Operation Function
//------------------------------------------------------------------------------
/**
 * @brief      Set Ack Generation Enable or Diable
 * @param[in]  ModuleIndex      A index of module.
 * @param[in]  bAckGen   CTRUE indicates that Ack Generate. 
 *                       CFALSE indicates that Ack Not Generation.
 * @return     None.
 * @remarks    Use only for receiver mode.
 */
void        NX_I2C_SetAckGenerationEnable( U32 ModuleIndex, CBOOL bAckGen )
{
    const U32 ACK_GEN_POS   = 7;
    const U32 ACK_GEN_MASK  = 1UL << ACK_GEN_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( (0==bAckGen) ||(1==bAckGen) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =   pRegister->ICCR;

    ReadValue   &=  ~ACK_GEN_MASK;
    ReadValue   |=  (U32)bAckGen << ACK_GEN_POS;

    //pRegister->ICCR  =   ReadValue;
    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 * @brief      Get Setting Value of Ack Generation
 * @param[in]  ModuleIndex      A index of module.
 * @return     CTRUE Indicate that Ack Generation Enabled. 
 *             CFALSE Indicate that Ack Generation Disabled.
 */
CBOOL       NX_I2C_GetAckGenerationEnable( U32 ModuleIndex )
{
    const U32 ACK_GEN_POS   = 7;
    const U32 ACK_GEN_MASK  = 1UL << ACK_GEN_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICCR & ACK_GEN_MASK) >> ACK_GEN_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief      Clear Operation Hold.
 * @return     None.
 * @remarks    I2C module's operation will be hold after transmiting or
 *             receiving a byte. Therefore you have to clear hold status to continue
 *             next progress.
 *             Also, user must clear hold status after module's start/stop setting.
 */
/*
void        NX_I2C_ClearOperationHold  ( U32 ModuleIndex )
{
    const U32    OP_HOLD_MASK = ( 0x01 << 1 ) ;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //__g_ModuleVariables[ModuleIndex].pRegister->PEND = (U16)OP_HOLD_MASK;
    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->PEND, (U16)OP_HOLD_MASK);
}
*/

//------------------------------------------------------------------------------
/**
 * @brief      Set I2C Control Mode
 * @param[in]  ModuleIndex  A index of module.
 * @param[in]  TxRxMode     Value of I2C's Mode ( Master tx/rx or Slave tx/rx )
 * @param[in]  Signal       Select Start/Stop signal ( NX_I2C_SIGNAL_START, NX_I2C_SIGNAL_STOP )
 * @return     None.
 * @remarks    This function make start/stop signal of I2C mode ( master tx/rx, slave tx/rx ).
 */
void        NX_I2C_ControlMode ( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode, NX_I2C_SIGNAL Signal )
{
    const U32   TX_RX_POS       =   6;
    const U32   ST_BUSY_POS     =   5;
    const U32   TXRX_ENB_MASK   =   1UL << 4;
    //const U32   ST_ENB_MASK     =   1UL << 12;

    register struct NX_I2C_RegisterSet*    pRegister;
    register U32    temp;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( 4 > TxRxMode );
    NX_ASSERT( 2 > Signal );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

    //temp = pRegister->ICSR & 0x1F0F;
    temp = pRegister->ICSR & 0x0F;

    // State enable 이 뭘까??
    //pRegister->ICSR =   ( temp | (TxRxMode<<TX_RX_POS) | (Signal<<ST_BUSY_POS) | TXRX_ENB_MASK | ST_ENB_MASK );
    //WriteIO32(&pRegister->ICSR , ( temp | (TxRxMode<<TX_RX_POS) | (Signal<<ST_BUSY_POS) | TXRX_ENB_MASK | ST_ENB_MASK ));
    WriteIO32(&pRegister->ICSR , ( temp | (TxRxMode<<TX_RX_POS) | (Signal<<ST_BUSY_POS) | TXRX_ENB_MASK ));
}

void        NX_I2C_SetTXRXEnable( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode )
{
    const U32   TX_RX_POS       =   6;
    register struct NX_I2C_RegisterSet*    pRegister;
    register U32    RegVal;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( 4 > TxRxMode );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
    
    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    RegVal = pRegister->ICSR & 0xFF;

    WriteIO32(&pRegister->ICSR , ( RegVal | (TxRxMode<<TX_RX_POS) ));
}

//------------------------------------------------------------------------------
/**
 * @brief      Check I2C's status
 * @param[in]  ModuleIndex  A index of module.
 * @return     CTRUE Indicate that I2C is Busy.
 *             CFALSE Indicate that I2C is Not Busy.
 */
CBOOL       NX_I2C_IsBusy( U32 ModuleIndex )
{
    const U32   ST_BUSY_POS     =   5;
    const U32   ST_BUSY_MASK    = 1UL << ST_BUSY_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICSR & ST_BUSY_MASK) >> ST_BUSY_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief      Set Send Data
 * @param[in]  ModuleIndex  A index of module.
 * @param[in]  WriteData     Value of Write Data ( 0x0 ~ 0xFF )
 * @return     None.
 */
void        NX_I2C_WriteByte ( U32 ModuleIndex, U8 WriteData )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( 0x100 > WriteData );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IDSR, (U8)( WriteData ));
}

//------------------------------------------------------------------------------
/**
 * @brief      Get Received Data
 * @param[in]  ModuleIndex  A index of module.
 * @return     Received Data ( 0x0 ~ 0xFF )
 */
U8          NX_I2C_ReadByte ( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return (U8)ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IDSR);
}

//------------------------------------------------------------------------------
/**
 * @brief       I2C's Bus Disable
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 * @remarks     Only use for Clear Arbitration fail status. 
 *              Arbitration status means that conflicting two I2C master device when
 *              data send. 
 *              This case, master device ( high prority ) send data, but master
 *              device(low prority) become arbitraion fail status.
 */
void        NX_I2C_BusDisable( U32 ModuleIndex )
{
    const U32 TXRX_ENB_MASK = ( 0x01 << 4 );

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =   pRegister->ICSR;
    ReadValue   &=  ~TXRX_ENB_MASK;

    //pRegister->ICSR = ReadValue;
    WriteIO32(&pRegister->ICSR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 * @brief       I2C's do not generate ack for the last transfer data
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 * @remarks     Only use for Clear Arbitration fail status. 
 *              Arbitration status means that conflicting two I2C master device when
 *              data send. 
 *              This case, master device ( high prority ) send data, but master
 *              device(low prority) become arbitraion fail status.
 *
 * @see         NX_I2C_DataLineRelease,            NX_I2C_ClockLineRelease
 */
void    NX_I2C_NotAckGen( U32 ModuleIndex )
{
    const U32 NOT_ACK_MASK  = ( 0x01 << 2 );

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    WriteIO32(&pRegister->STOPCON, NOT_ACK_MASK);
}

//------------------------------------------------------------------------------
/**
 * @brief       I2C's data bus release
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 * @remarks     Only use for Clear Arbitration fail status. 
 *              Arbitration status means that conflicting two I2C master device when
 *              data send. 
 *              This case, master device ( high prority ) send data, but master
 *              device(low prority) become arbitraion fail status.
 */
void    NX_I2C_DataLineRelease( U32 ModuleIndex )
{
    const U32 DAT_REL_MASK  = ( 0x01 << 1 );

    register struct NX_I2C_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    WriteIO32(&pRegister->STOPCON, DAT_REL_MASK);
}

//------------------------------------------------------------------------------
/**
 * @brief       I2C's Bus clock bus release
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 * @remarks     Only use for Clear Arbitration fail status. 
 *              Arbitration status means that conflicting two I2C master device when
 *              data send. 
 *              This case, master device ( high prority ) send data, but master
 */
void    NX_I2C_ClockLineRelease( U32 ModuleIndex )
{
    const U32 CLK_REL_MASK  = ( 0x01 << 0 );

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    WriteIO32(&pRegister->STOPCON, CLK_REL_MASK);
}


//------------------------------------------------------------------------------
// Checking Function of external Interrupt's source when interrupt is occurred.
//------------------------------------------------------------------------------
/**
 * @brief       Check state of slave address is matched or NOT.
 * @param[in]   ModuleIndex A index of module.
 * @return      CTRUE Indicate that Slave address is matched. 
 *              CFALSE Indicate that Slave address is NOT matched.
 * @remarks     Interrupt is occurred when slave address is matched. 
 */
CBOOL       NX_I2C_IsSlaveAddressMatch( U32 ModuleIndex )
{
    const U32   SLAVE_MATCH_OCCUR_POS     =   2;
    const U32   SLAVE_MATCH_OCCUR_MASK    =   1UL << SLAVE_MATCH_OCCUR_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICSR & SLAVE_MATCH_OCCUR_MASK) >> SLAVE_MATCH_OCCUR_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief       Clear state of slave address is matched.
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 */
/*
void        NX_I2C_ClearSlaveAddressMatch( U32 ModuleIndex )
{
    const U32   ST_ENB_MASK               = 1UL << 12;
    const U32   SLAVE_MATCH_OCCUR_POS     = 10;
    const U32   SLAVE_MATCH_OCCUR_MASK    = 1UL << SLAVE_MATCH_OCCUR_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =    pRegister->ICSR;
    ReadValue   &= ~( ST_ENB_MASK | SLAVE_MATCH_OCCUR_MASK );

    //pRegister->ICSR = ReadValue;
    WriteIO32(&pRegister->ICSR, ReadValue);
}
*/

//------------------------------------------------------------------------------
/**
 * @brief       Check state of General call is occurred or NOT.
 * @param[in]   ModuleIndex A index of module.
 * @return      CTRUE Indicate that General call is occurred. 
 *              CFALSE Indicate that General call is NOT occurred.
 * @remarks     Interrupt is occurred when general call is occurred.
 *              General call means that master device send a command to all slave device( broadcasting ).
 */
/*
CBOOL       NX_I2C_IsGeneralCall( U32 ModuleIndex )
{
    const U32   GENERAL_CALL_OCCUR_POS     =   9;
    const U32   GENERAL_CALL_OCCUR_MASK    =   1UL << GENERAL_CALL_OCCUR_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICSR & GENERAL_CALL_OCCUR_MASK) >> GENERAL_CALL_OCCUR_POS );
}
*/

//------------------------------------------------------------------------------
/**
 * @brief       Clear state of general call is occurred.
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 */
/*
void        NX_I2C_ClearGeneralCall( U32 ModuleIndex )
{
    const U32   ST_ENB_MASK                = 1UL << 12;
    const U32   GENERAL_CALL_OCCUR_POS     = 9;
    const U32   GENERAL_CALL_OCCUR_MASK    = 1UL << GENERAL_CALL_OCCUR_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =    pRegister->ICSR;
    ReadValue   &= ~( ST_ENB_MASK | GENERAL_CALL_OCCUR_MASK );

    //pRegister->ICSR = ReadValue;
    WriteIO32(&pRegister->ICSR, ReadValue);
}
*/

//------------------------------------------------------------------------------
/**
 * @brief       Check state of slave RX is stopped or NOT.
 * @param[in]   ModuleIndex A index of module.
 * @return      CTRUE Indicate that Slave RX is stopped. 
 *              CFALSE Indicate that Slave RX is NOT stopped.
 * @remarks     Interrupt is occurred when slave RX is stopped.
 */
/*
CBOOL       NX_I2C_IsSlaveRxStop( U32 ModuleIndex )
{
    const U32   SLV_RX_STOP_POS     = 0;
    const U32   SLV_RX_STOP_MASK    = 1UL << SLV_RX_STOP_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICSR & SLV_RX_STOP_MASK) >> SLV_RX_STOP_POS );
}
*/

//------------------------------------------------------------------------------
/**
 * @brief       Clear state of Slave RX is stopped.
 * @param[in]   ModuleIndex A index of module.
 * @return      None.
 */
/*
void        NX_I2C_ClearSlaveRxStop( U32 ModuleIndex )
{
    const U32   ST_ENB_MASK         = 1UL << 12;
    const U32   SLV_RX_STOP_POS     = 8;
    const U32   SLV_RX_STOP_MASK    = 1UL << SLV_RX_STOP_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =    pRegister->ICSR;
    ReadValue   &= ~( ST_ENB_MASK | SLV_RX_STOP_MASK );

//  pRegister->ICSR = ReadValue;
    WriteIO32(&pRegister->ICSR, ReadValue);
}
*/

//------------------------------------------------------------------------------
/**
 * @brief      Check Bus Arbitration status
 * @param[in]  ModuleIndex  A index of module.
 * @return     CTRUE Indicate that Bus Arbitration Failed. 
 *             CFALSE Indicate that Bus Arbitration is Not Failed.
 * @remarks    Interrupt is Occured when Extend IRQ Enable and Bus arbitration is failed.
 */
CBOOL       NX_I2C_IsBusArbitFail( U32 ModuleIndex )
{
    const U32   ARBIT_FAIL_POS     = 3;
    const U32   ARBIT_FAIL_MASK   = 1UL << ARBIT_FAIL_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (__g_ModuleVariables[ModuleIndex].pRegister->ICSR & ARBIT_FAIL_MASK) >> ARBIT_FAIL_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief      Check ACK Status
 * @param[in]  ModuleIndex  A index of module.
 * @return     CTRUE Indicate that ACK Received.
 *             CFALSE Indicate that ACK NOT received.
 * @remarks    Interrupt is Occured when Extend IRQ Enable and NAck Received.
 */
CBOOL       NX_I2C_IsACKReceived( U32 ModuleIndex )
{
    const U32   ACK_STATUS_POS    = 0;
    const U32   ACK_STATUS_MASK   = 1UL << ACK_STATUS_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    //return  (CBOOL)(((__g_ModuleVariables[ModuleIndex].pRegister->ICSR & ACK_STATUS_MASK) >> ACK_STATUS_POS ) & 0x01 );  // 0 : CTRUE, 1 : CFALSE
    return  (CBOOL)(!((__g_ModuleVariables[ModuleIndex].pRegister->ICSR & ACK_STATUS_MASK) >> ACK_STATUS_POS ));  // 0 : CTRUE, 1 : CFALSE
}


//------------------------------------------------------------------------------
/**
 * @brief      Check I2C's Mode ( Rx or Tx )
 * @return     CTRUE Indicate that I2C's mode is Tx 
 *             CFALSE Indicate that I2C's mode is Rx.
 */
CBOOL       NX_I2C_IsTxMode( U32 ModuleIndex )
{
    const U32 TX_RX_MASK = 1UL << 6;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return ( (__g_ModuleVariables[ModuleIndex].pRegister->ICSR & TX_RX_MASK) ? CTRUE : CFALSE );
}

