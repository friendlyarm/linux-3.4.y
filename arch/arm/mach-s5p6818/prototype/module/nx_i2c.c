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
//  Module     	: I2C
//  File		  	: nx_i2c.c
//  Description	:
//  Author     	: Firmware Team
//  History    	: 2014.10.01	Revision Comment & Function Define. (Deoks)
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
 *  @return  CTRUE  indicates that Initialize is succeeded.
 *  			CFALSE indicates that Initialize is failed.
 */
CBOOL   NX_I2C_Initialize( void )
{
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
 *  @brief     	 Get number of modules in the chip.
 *  @return     	 Module's number.
 */
U32     NX_I2C_GetNumberOfModule( void )
{
    return NUMBER_OF_I2C_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *  @brief      	Get module's physical address.
 *  @param[in] 	ModuleIndex     A index of module.
 *  @return     	Module's physical address
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
 *  @brief	Get a size, in byte, of register set.
 *  @return	Size of module's register set.
 */
U32     NX_I2C_GetSizeOfRegisterSet( void )
{
    return sizeof( struct NX_I2C_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *  @brief		Set a base address of register set.
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	BaseAddress Module's base address
 *  @return		None.
 */

void    NX_I2C_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
    NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    __g_ModuleVariables[ModuleIndex].pRegister = (struct NX_I2C_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get a base address of register set
 *  @param[in]   ModuleIndex     A index of module.
 *  @return		Module's base address.
 */

void*    NX_I2C_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );


    return (void*)__g_ModuleVariables[ModuleIndex].pRegister;

}

//------------------------------------------------------------------------------
/**
 *  @brief       	Initialize selected modules with default value.
 *  @param[in]   ModuleIndex     A index of module.
 *  @return		CTRUE  indicates that Initialize is succeeded. 
 *              		CFALSE indicates that Initialize is failed.
 */
CBOOL   NX_I2C_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Deinitialize selected module to the proper stage.
 *  @param[in]   ModuleIndex     A index of module.
 *  @return     	CTRUE  indicates that Deinitialize is succeeded. 
 *              		CFALSE indicates that Deinitialize is failed.
 */
CBOOL   NX_I2C_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Indicates whether the selected modules is busy or not.
 *  @param[in]   ModuleIndex     A index of module.
 *  @return     	CTRUE  indicates that Module is Busy. 
 *              		CFALSE indicates that Module is NOT Busy.
 */
CBOOL   NX_I2C_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );

    return NX_I2C_IsBusy( ModuleIndex );
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Indicaes whether the selected modules is ready to enter power-down stage
 *  @param[in]   ModuleIndex     A index of module.
 *  @return     	CTRUE  indicates that Ready to enter power-down stage. 
 *              		CFALSE indicates that This module can't enter to power-down stage.
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
 *  @brief      	Get a interrupt number for interrupt controller.
 *  @param[in]   ModuleIndex     A index of module.
 *  @return    	Interrupt number
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
 *   					    CFALSE indicates that Interrupt Disable.
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
    //NX_ASSERT( 0 == IntNum );
    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    pRegister   =   __g_ModuleVariables[ModuleIndex].pRegister;

    ReadValue   =   ReadIO32(&pRegister->ICCR);

    ReadValue   &=  ~IRQ_ENB_MASK;
    ReadValue   |=  (U32)Enable << IRQ_ENB_POS;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	IntNum  Interrupt Number ( 0 ).
 *  @return		CTRUE  indicates that Interrupt is enabled. 
 *              		CFALSE indicates that Interrupt is disabled.
 *  @remarks    I2C Module have one interrupt. So always IntNum set to 0.
 */
CBOOL   NX_I2C_GetInterruptEnable( U32 ModuleIndex, S32 IntNum )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

	register struct NX_I2C_RegisterSet* pRegister;

    IntNum = IntNum;
	NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	//NX_ASSERT( 0 == IntNum );

    return  (CBOOL)( (ReadIO32(&pRegister->ICCR) & IRQ_ENB_MASK) >> IRQ_ENB_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  EnableFlag  Specify interrupt bit for enable of disable. Each bit's meaning is like below 
 *                   - EnableFlag[0] : Set TX/RX interrupt enable or disable. 
 *  @return     None.
 */
void    NX_I2C_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    ReadValue   =   ReadIO32(&pRegister->ICCR);

    ReadValue   &=  ~IRQ_ENB_MASK;
    ReadValue   |=  (U32)(EnableFlag & 0x01) << IRQ_ENB_POS;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief     	Indicates current setting value of interrupt enable bit.
 *  @param[in]	ModuleIndex     A index of module.
 *  @return		Current setting value of interrupt. 
 *              		"1" means interrupt is enabled. 
 *              		"0" means interrupt is disabled. 
 *              		- Return Value[0] : TX/RX interrupt's setting value. 
 */
U32     NX_I2C_GetInterruptEnable32( U32 ModuleIndex )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );


    return  (U32)( (ReadIO32(&pRegister->ICCR) & IRQ_ENB_MASK) >> IRQ_ENB_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	IntNum  Interrupt Number ( 0 ).
 *  @return   	CTRUE  indicates that Pending is seted.
 *             		CFALSE indicates that Pending is Not Seted.
 *  @remarks    I2C Module have one interrupt. So always IntNum set to 0.

 */
CBOOL   NX_I2C_GetInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    const U32 PEND_POS   = 4;
    const U32 PEND_MASK  = 1UL << PEND_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    IntNum = IntNum;
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    //NX_ASSERT( 0 == IntNum );

    return  (CBOOL)( (ReadIO32(&pRegister->ICCR) & PEND_MASK) >> PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Indicates current setting value of interrupt pending bit.
 *  @param[in]	ModuleIndex     A index of module.
 *  @return     	Current setting value of pending bit. 
 *              		"1" means pend bit is occured. 
 *              		"0" means pend bit is NOT occured. 
 *              		- Return Value[0] : TX/RX pending state. 
 */
U32     NX_I2C_GetInterruptPending32( U32 ModuleIndex )
{
    const U32 PEND_POS   = 4;
    const U32 PEND_MASK  = 1UL << PEND_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );


    return  (U32)( (ReadIO32(&pRegister->ICCR) & PEND_MASK) >> PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Clear a pending state of specified interrupt.
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	IntNum          Interrupt number ( 0 ).
 *  @return     	None.
 *  @remarks    I2C Module have one interrupt. So always IntNum set to 0.
 */
void    NX_I2C_ClearInterruptPending( U32 ModuleIndex, S32 IntNum )
{
    const U32 INTC_POS   = 4;
    const U32 INTC_MASK  = 1UL << INTC_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    IntNum = IntNum;
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    //NX_ASSERT( 0 == IntNum );

    ReadValue   =   ReadIO32(&pRegister->ICCR);
    ReadValue   &=  ~INTC_MASK;
    ReadValue   |=  INTC_MASK;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  ModuleIndex     A index of module.
 *  @param[in]  PendingFlag     Specify pend bit to clear. Each bit's meaning is like below 
 *                      - PendingFlag[0] : TX/RX pending bit. \n
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
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    ReadValue   =   ReadIO32(&pRegister->ICCR);
    ReadValue   &=  (~PEND_MASK | (~PendingFlag << PEND_POS ));
    ReadValue   |=  INTC_MASK;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief		Set all interrupts to be enables or disables.
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	Enable	CTRUE  indicates that Set to all interrupt enable.
 *					 	CFALSE indicates that Set to all interrupt disable.
 *  @return     None.
 */
void    NX_I2C_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );


    ReadValue   =   ReadIO32(&pRegister->ICCR);

    ReadValue   &=  ~IRQ_ENB_MASK;
    ReadValue   |=  (U32)Enable << IRQ_ENB_POS;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enable or not.
 *  @param[in]	ModuleIndex     A index of module.
 *  @return		CTRUE  indicates that At least one( or more ) interrupt is enabled.
 *            		CFALSE indicates that All interrupt is disabled.
 */
CBOOL   NX_I2C_GetInterruptEnableAll( U32 ModuleIndex )
{
    const U32 IRQ_ENB_POS   = 5;
    const U32 IRQ_ENB_MASK  = 1UL << IRQ_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return  (CBOOL)( (ReadIO32(&pRegister->ICCR) & IRQ_ENB_MASK) >> IRQ_ENB_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are pended or not.
 *  @param[in]	ModuleIndex     A index of module.
 *  @return		CTRUE  indicates that At least one( or more ) pending is seted.
 *   				CFALSE indicates that All pending is NOT seted.
 */
CBOOL   NX_I2C_GetInterruptPendingAll( U32 ModuleIndex )
{
    const U32 PEND_POS   = 4;
    const U32 PEND_MASK  = 1UL << PEND_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return  (CBOOL)( (ReadIO32(&pRegister->ICCR) & PEND_MASK) >> PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Clear pending state of all interrupts.
 *  @param[in]	ModuleIndex     A index of module.
 *  @return     	None.
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
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    ReadValue   =   ReadIO32(&pRegister->ICCR);
    ReadValue   &=  ~PEND_MASK;
    ReadValue   |=  INTC_MASK;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get a interrupt number which has the most prority of pended interrupts
 *  @param[in]	ModuleIndex     A index of module.
 *  @return     	Pending Number( If all pending is not set then return -1 ).
 */
S32     NX_I2C_GetInterruptPendingNumber( U32 ModuleIndex )  // -1 if None
{
    const U32 IRQ_ENB_POS = 5;
    const U32 PEND_POS  = 4;
    //const U32 PEND_MASK = 1UL << PEND_POS;

    const U32   I2CInterruptNumber[NUMBER_OF_I2C_MODULE] = { INTNUM_LIST( I2C ) };

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );


    ReadValue = ((ReadIO32(&pRegister->ICCR) >> IRQ_ENB_POS) 
					& (ReadIO32(&pRegister->ICCR) >> PEND_POS));

    if( ReadValue & 0x01 )
    {
        return  I2CInterruptNumber[ModuleIndex];
    }

    return -1;
}


//------------------------------------------------------------------------------
/**
 *  @brief      	Get current reset number
 *  @param[in]	ModuleIndex     A index of module.
 *  @return		Current reset number
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
 *  @brief      	Get current reset number
 *  @param[in]   ModuleIndex     A index of module.
 *  @return     	Current reset number
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
 *  @brief      	Set PCLK divider and clock prescaler.
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	PclkDivider	Set PCLK divider ( 16, 256 )
 *  @param[in]	Prescaler      Set prescaler. 
 *                       2 ~ 16 ( when PCLK divider is seted 16 ). 
 *                       1 ~ 16 ( when PCLK divider is seted 256 ).
 *  @return     None.
 */
void    NX_I2C_SetClockPrescaler( U32 ModuleIndex, U32 PclkDivider, U32 Prescaler )
{
    const U32   CLKSRC_POS  	= 6;
    const U32   CLKSRC_MASK 	= 1UL << CLKSRC_POS;
    const U32   CLK_SCALER_MASK = 0x0F;

    register U32 SetPclkDivider	= 0;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( (16==PclkDivider) || (512==PclkDivider) );
    NX_ASSERT( 1 <= Prescaler && Prescaler <= 16 );
    NX_ASSERT( (16!=PclkDivider) || ( 2 <= Prescaler && Prescaler <= 16) );

    if( 16 == PclkDivider )
    {
        SetPclkDivider = 0;
    }
    else if( 512 == PclkDivider )
    {
        SetPclkDivider = 1;
    }

    ReadValue   =   ReadIO32(&pRegister->ICCR);

    ReadValue   &=  ~( CLKSRC_MASK | CLK_SCALER_MASK );
    ReadValue   |=  ((SetPclkDivider << CLKSRC_POS) | (Prescaler-1));

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set PCLK divider and clock prescaler.
 *  @param[in]    ModuleIndex	A index of module.
 *  @param[out]	 pPclkDivider     	Get PCLK divider ( 16, 256 )
 *  @param[out] pPrescaler       	Get prescaler.
 *  @return     None.
 */
void    NX_I2C_GetClockPrescaler( U32 ModuleIndex, U32* pPclkDivider, U32* pPrescaler )
{
    const U32   CLKSRC_POS  = 6;
    const U32   CLKSRC_MASK = 1UL << CLKSRC_POS;
    const U32   CLK_SCALER_MASK = 0x0F;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( CNULL != pPclkDivider );
    NX_ASSERT( CNULL != pPrescaler );

    ReadValue   =   ReadIO32(&pRegister->ICCR);

    if( ReadValue & CLKSRC_MASK )
    {
        if(CNULL != pPclkDivider) { *pPclkDivider = 512; }
    }
    else
    {
        if(CNULL != pPclkDivider) { *pPclkDivider = 16; }
    }

    if(CNULL != pPrescaler) { *pPrescaler = (ReadValue & CLK_SCALER_MASK)+1; }
}

//------------------------------------------------------------------------------
/**
 * @brief       	Set Salve Address
 * @param[in]    ModuleIndex		A index of module.
 * @param[in]    Address    		Value of Salve Address ( 0x02 ~ 0xFE )
 * @return      	None.
 * @remarks     	LSB[0] bit must set to 0. Slave Address can change ONLY ICSR[TXRX_ENB] bit is 0.
 */
void    NX_I2C_SetSlaveAddress( U32 ModuleIndex, U8 Address )
{
    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( 0 == (Address & 0x01) );

    WriteIO32(&pRegister->IAR, (U32)Address);
}

//------------------------------------------------------------------------------
/**
 * @brief       	Get Salve Address
 * @param[in]    ModuleIndex		A index of module.
 * @param[in]    Address			Value of Salve Address ( 0x02 ~ 0xFE )
 * @return      	None.
 * @remarks     	LSB[0] bit must set to 0. Slave Address can change ONLY ICSR[TXRX_ENB] bit is 0.
 */
U32    NX_I2C_GetSlaveAddress( U32 ModuleIndex )
{
    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (U32)(ReadIO32(&pRegister->IAR) & 0xFF);

}

//------------------------------------------------------------------------------
// Operation Function
//------------------------------------------------------------------------------
/**
 * @brief      	Set Ack Generation Enable or Diable
 * @param[in] 	ModuleIndex      A index of module.
 * @param[in] 	bAckGen CTRUE indicates that Ack Generate.
 *                       		 CFALSE indicates that Ack Not Generation.
 * @return     	None.
 * @remarks 	Use only for receiver mode.
 */
void        NX_I2C_SetAckGenerationEnable( U32 ModuleIndex, CBOOL bAckGen )
{
    const U32 ACK_GEN_POS   = 7;
    const U32 ACK_GEN_MASK  = 1UL << ACK_GEN_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( (0==bAckGen) ||(1==bAckGen) );

    ReadValue    =  ReadIO32(&pRegister->ICCR);

    ReadValue   &=  ~ACK_GEN_MASK;
    ReadValue   |=  (U32)bAckGen << ACK_GEN_POS;

    WriteIO32(&pRegister->ICCR, ReadValue);
}

//------------------------------------------------------------------------------
/**
 * @brief      Get Setting Value of Ack Generation
 * @param[in]	ModuleIndex      A index of module.
 * @return		CTRUE Indicate that Ack Generation Enabled. 
 *             		CFALSE Indicate that Ack Generation Disabled.
 */
CBOOL       NX_I2C_GetAckGenerationEnable( U32 ModuleIndex )
{
    const U32 ACK_GEN_POS   = 7;
    const U32 ACK_GEN_MASK  = 1UL << ACK_GEN_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return  (CBOOL)( (ReadIO32(&pRegister->ICCR) & ACK_GEN_MASK) >> ACK_GEN_POS );
}


//------------------------------------------------------------------------------
/**
 * @brief      	Set I2C Control Mode
 * @param[in] 	ModuleIndex  A index of module.
 * @param[in] 	TxRxMode	Value of I2C's Mode ( Master tx/rx or Slave tx/rx )
 * @param[in] 	Signal       	Select Start/Stop signal ( NX_I2C_SIGNAL_START, NX_I2C_SIGNAL_STOP )
 * @return     	None.
 * @remarks    	This function make start/stop signal of I2C mode ( master tx/rx, slave tx/rx ).
 */
void        NX_I2C_ControlMode ( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode, NX_I2C_SIGNAL Signal )
{
    const U32   TX_RX_POS       =   6;
    const U32   ST_BUSY_POS     =   5;
    const U32   TXRX_ENB_MASK   =   1UL << 4;
    //const U32   ST_ENB_MASK     =   1UL << 12;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( 4 > TxRxMode );
    NX_ASSERT( 2 > Signal );

    ReadValue = ReadIO32(&pRegister->ICSR) & 0x0F;

    WriteIO32(&pRegister->ICSR , ( ReadValue | (TxRxMode<<TX_RX_POS) | (Signal<<ST_BUSY_POS) | TXRX_ENB_MASK ));
}

void        NX_I2C_SetTXRXEnable( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode )
{
    const U32   TX_RX_POS       =   6;
    const U32   TXRX_ENB_MASK   =   1UL << 4;
    register struct NX_I2C_RegisterSet*    pRegister;
    register U32    ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( 4 > TxRxMode );
    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    NX_ASSERT( CNULL != pRegister );

    ReadValue = ReadIO32(&pRegister->ICSR);
    ReadValue &= ~TXRX_ENB_MASK;
    ReadValue |= (U32)TxRxMode << TX_RX_POS;
    
    WriteIO32(&pRegister->ICSR , ReadValue);

}


//------------------------------------------------------------------------------
/**
 * @brief      	Set Filter Enable or Disable
 * @param[in] 	ModuleIndex  A index of module.
 * @param[in]	Enable		 CTRUE 	 Indicate that Filter Enable
 *             					 CFALSE Indicate that Filter Disable
 * @return		None.
 */
void		NX_I2C_SetFilterEnable( U32 ModuleIndex, CBOOL bEnable )
{	
    const U32 FILTER_ENB_POS   = 2;
    const U32 FILTER_ENB_MASK  = 1UL << FILTER_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;
    NX_ASSERT( (0==bEnable) ||(1==bEnable) );

    ReadValue   =   ReadIO32(&pRegister->ICLC);

    ReadValue   &=  ~FILTER_ENB_MASK;
    ReadValue   |=  (U32)bEnable << FILTER_ENB_POS;

    WriteIO32(&pRegister->ICLC, ReadValue);
}

//------------------------------------------------------------------------------
/**
 * @brief      	Get Filter Enable or Disable
 * @param[in]	ModuleIndex  A index of module.
 * @return     	CTRUE 	 Indicate that Filter Enable
 *             		CFALSE Indicate that Filter Disable
 */
CBOOL       NX_I2C_GetFilterEnable( U32 ModuleIndex )
{
    const U32 FILTER_ENB_POS   = 2;
    const U32 FILTER_ENB_MASK  = 1UL << FILTER_ENB_POS;

    register struct NX_I2C_RegisterSet* pRegister;
	
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    return (CBOOL)( (ReadIO32(&pRegister->ICLC) & FILTER_ENB_MASK) ? CTRUE : CFALSE );
}

//------------------------------------------------------------------------------
/**
 * @brief      	Set SDA Output Delay
 * @param[in] 	ModuleIndex  A index of module.
 * @param[in]	Delay	(0: 0CLK, 1:5CLK, 2:10CLK, 3:15CLK)
 * @return		None.
 */
void		NX_I2C_SetSDAOutputDelay( U32 ModuleIndex, NX_I2C_SDA_DELAY Delay )
{	
    const U32 SDA_DELAY_POS   = 0;
    const U32 SDA_DELAY_MASK  = 1UL << SDA_DELAY_POS;

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    NX_ASSERT( ( 3 >= Delay) );

    ReadValue   =   ReadIO32(&pRegister->ICLC);

    ReadValue   &=  ~SDA_DELAY_MASK;
    ReadValue   |=  (U32)Delay << SDA_DELAY_POS;

    WriteIO32(&pRegister->ICLC, ReadValue);
}

//------------------------------------------------------------------------------
/**
 * @brief      	Set SDA Output Delay
 * @param[in] 	ModuleIndex  A index of module.
 * @return		Delay	(0: 0CLK, 1:5CLK, 2:10CLK, 3:15CLK)
 */
NX_I2C_SDA_DELAY	NX_I2C_GetSDAOutputDelay( U32 ModuleIndex )
{
    const U32 SDA_DELAY_POS   = 0;
    const U32 SDA_DELAY_MASK  = 1UL << SDA_DELAY_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    return (NX_I2C_SDA_DELAY)( (ReadIO32(&pRegister->ICLC) & SDA_DELAY_MASK) ? CTRUE : CFALSE );
}

//------------------------------------------------------------------------------
/**
 * @brief      	Check I2C's status
 * @param[in]  	ModuleIndex  A index of module.
 * @return		CTRUE Indicate that I2C is Busy.
 *             		CFALSE Indicate that I2C is Not Busy.
 */
CBOOL       NX_I2C_IsBusy( U32 ModuleIndex )
{
    const U32   ST_BUSY_POS     =   5;
    const U32   ST_BUSY_MASK    = 1UL << ST_BUSY_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister )


    return  (CBOOL)( (ReadIO32(&pRegister->ICSR) & ST_BUSY_MASK) >> ST_BUSY_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief      	Set Send Data
 * @param[in] 	ModuleIndex  A index of module.
 * @param[in]	WriteData     Value of Write Data ( 0x0 ~ 0xFF )
 * @return		None.
 */
void        NX_I2C_WriteByte ( U32 ModuleIndex, U8 WriteData )
{
    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    NX_ASSERT( 0xFF >= WriteData );

    WriteIO32(&pRegister->IDSR, (U8)WriteData);
}

//------------------------------------------------------------------------------
/**
 * @brief      	Get Received Data
 * @param[in]	ModuleIndex  A index of module.
 * @return		Received Data ( 0x0 ~ 0xFF )
 */
U8          NX_I2C_ReadByte ( U32 ModuleIndex )
{
    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );


    return (U8)(ReadIO32(&pRegister->IDSR));
}

//------------------------------------------------------------------------------
/**
 * @brief       	I2C's Bus Disable
 * @param[in]	ModuleIndex A index of module.
 * @return      	None.
 * @remarks	Only use for Clear Arbitration fail status. 
 *              		Arbitration status means that conflicting two I2C master device when
 *              		data send. 
 *              		This case, master device ( high prority ) send data, but master
 *              		device(low prority) become arbitraion fail status.
 *
 */
void        NX_I2C_BusDisable( U32 ModuleIndex )
{
    const U32 TXRX_ENB_MASK = ( 0x01 << 4 );

    register struct NX_I2C_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    ReadValue   =   pRegister->ICSR;
    ReadValue   &=  ~TXRX_ENB_MASK;

    WriteIO32(&pRegister->ICSR, ReadValue);
}


//------------------------------------------------------------------------------
// Checking Function of external Interrupt's source when interrupt is occurred.
//------------------------------------------------------------------------------
/**
 * @brief       	Check state of slave address is matched or NOT.
 * @param[in]	ModuleIndex A index of module.
 * @return      	CTRUE Indicate that Slave address is matched. 
 *              		CFALSE Indicate that Slave address is NOT matched.
 * @remarks 	Interrupt is occurred when slave address is matched. 
 */
CBOOL       NX_I2C_IsSlaveAddressMatch( U32 ModuleIndex )
{
    const U32   SLAVE_MATCH_OCCUR_POS     =   2;
    const U32   SLAVE_MATCH_OCCUR_MASK    =   1UL << SLAVE_MATCH_OCCUR_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return  (CBOOL)( (ReadIO32(&pRegister->ICSR) & SLAVE_MATCH_OCCUR_MASK) >> SLAVE_MATCH_OCCUR_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief      	Check Bus Arbitration status
 * @param[in]	ModuleIndex  A index of module.
 * @return     	CTRUE Indicate that Bus Arbitration Failed. 
 *             		CFALSE Indicate that Bus Arbitration is Not Failed.
 * @remarks   	 Interrupt is Occured when Extend IRQ Enable and Bus arbitration is failed.
 */
CBOOL       NX_I2C_IsBusArbitFail( U32 ModuleIndex )
{
    const U32   ARBIT_FAIL_POS     = 3;
    const U32   ARBIT_FAIL_MASK   = 1UL << ARBIT_FAIL_POS;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    return  (CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ICSR) & ARBIT_FAIL_MASK) >> ARBIT_FAIL_POS );
}

//------------------------------------------------------------------------------
/**
 * @brief      	Check ACK Status
 * @param[in]	ModuleIndex  A index of module.
 * @return     	CTRUE Indicate that ACK Received.
 *             		CFALSE Indicate that ACK NOT received.
 * @remarks    	Interrupt is Occured when Extend IRQ Enable and NAck Received.
 */
CBOOL       NX_I2C_IsACKReceived( U32 ModuleIndex )
{
    const U32   ACK_STATUS_POS    = 0;
    const U32   ACK_STATUS_MASK   = 1UL << ACK_STATUS_POS;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    return  (CBOOL)(!((ReadIO32(&pRegister->ICSR) & ACK_STATUS_MASK) >> ACK_STATUS_POS ));  // 0 : CTRUE, 1 : CFALSE
}


//------------------------------------------------------------------------------
/** 
 * @brief		Check I2C's Mode ( Rx or Tx )
 * @param[in]	ModuleIndex  A index of module. 
 * @return		CTRUE Indicate that I2C's mode is Tx 
 *             		CFALSE Indicate that I2C's mode is Rx.
 */
CBOOL       NX_I2C_IsTxMode( U32 ModuleIndex )
{
    const U32 TX_RX_MASK = 1UL << 6;

    register struct NX_I2C_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    return ( (ReadIO32(&pRegister->ICSR) & TX_RX_MASK) ? CTRUE : CFALSE );
}

