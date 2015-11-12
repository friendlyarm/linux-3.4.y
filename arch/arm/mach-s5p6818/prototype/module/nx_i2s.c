//------------------------------------------------------------------------------
//
//  Copyright (C) 2009 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module     : I2S
//	File       : nx_i2s.c
//	Description:
//	Author      : Firmware Team
//	History     : 2012.08.23 parkjh - xl00300_I2S proto type
//------------------------------------------------------------------------------

#include "nx_chip.h"
#include "nx_dma.h"
#include "nx_i2s.h"

static	struct
{
    struct NX_I2S_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_I2S_MODULE] = { {CNULL,}, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicates that Initialize is successed.
 *			CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_I2S_Initialize( void )
{
	static CBOOL bInit = CFALSE;
    U32 i;

	if( CFALSE == bInit )
	{
        for( i=0; i<NUMBER_OF_I2S_MODULE; i++ )
        {
            __g_ModuleVariables[i].pRegister = CNULL;
        }

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_I2S_GetNumberOfModule( void )
{
	return NUMBER_OF_I2S_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_I2S_GetPhysicalAddress( U32 ModuleIndex )
{
    static const U32 I2SPhysicalAddr[NUMBER_OF_I2S_MODULE] =
    {
        PHY_BASEADDR_LIST( I2S )
    };

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return  (U32)I2SPhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_I2S_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_I2S_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */

void	NX_I2S_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_I2S_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */

void*	NX_I2S_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );


	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicates that Initialize is successed. 
 *				CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_I2S_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicates that Deinitialize is successed.
 *				CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_I2S_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicates that Module is Busy. 
 *				CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_I2S_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		CTRUE	indicates that Ready to enter power-down stage. \r\n
 *				CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_I2S_CanPowerDown( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Get DMA peripheral index of I2S controller's DMA number
 *	@return DMA peripheral index of I2S controller's DMA number
 */
U32		NX_I2S_GetDMAIndex_PCMIn( U32 ModuleIndex )
{
    const U32 I2sDMAIndex[NUMBER_OF_I2S_MODULE] =
    {
        DMAINDEX_WITH_CHANNEL_LIST(I2S, I2SRXDMA)    
    };

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return  I2sDMAIndex[ModuleIndex];
}

/**
 *	@brief	Get DMA peripheral index of I2S controller's DMA number
 *	@return DMA peripheral index of I2S controller's DMA number
 */
U32		NX_I2S_GetDMAIndex_PCMOut( U32 ModuleIndex )
{
    const U32 I2sDMAIndex[NUMBER_OF_I2S_MODULE] =
    {
        DMAINDEX_WITH_CHANNEL_LIST(I2S, I2STXDMA)
    };

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return  I2sDMAIndex[ModuleIndex];
}

/**
 *	@brief		Get bus width of I2S controller
 *	@return		DMA bus width of I2S controller.
 */
U32		NX_I2S_GetDMABusWidth( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );
	return 32;
}

//------------------------------------------------------------------------------
// Clock Control Interface
//------------------------------------------------------------------------------

/**
 *	@brief		Get current clock number
 *	@return		Current Clock Index
 */
U32		NX_I2S_GetClockNumber( U32 ModuleIndex )
{
    static const U32 CLKGEN_I2SLIST[] =
    {
        CLOCKINDEX_LIST( I2S )
    };

	NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return (U32)CLKGEN_I2SLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get current reset number
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		Current reset number
 *  @remarks
 */
U32     NX_I2S_GetResetNumber( U32 ModuleIndex )
{
    const U32 ResetPortNumber[NUMBER_OF_I2S_MODULE] =
    {
        RESETINDEX_LIST( I2S, PRESETn )
    };

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (U32)ResetPortNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
/// @  name   I2S Interface
/// @{

// The DMA-Related functions. 
//------------------------------------------------------------------------------
/**
 *  @brief      	Set DMA Tx Pause Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Enable	CTRUE	Indicate that DMA Tx Pause Enable
 *  						CFALSE	Indicate that DMA Tx Pause Disable
 *  @return		None.
 */
void    NX_I2S_TxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TDP_POS   = 6;
    const U32 TDP_MASK  = 1UL<<TDP_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~TDP_MASK;
    regvalue     |= Enable<<TDP_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set DMA Rx Pause Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Enable	CTRUE	Indicate that DMA Rx Pause Enable
 *  						CFALSE	Indicate that DMA Rx Pause Disable
 */
void    NX_I2S_RxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RDP_POS   = 5;
    const U32 RDP_MASK  = 1UL<<RDP_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~RDP_MASK;
    regvalue     |= Enable<<RDP_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set I2S Tx Pause Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Enable	CTRUE	Indicate that I2S Tx Pause Enable
 *  						CFALSE	Indicate that I2S Tx Pause Disable
 *  @return		None.
 */
void    NX_I2S_TxChPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TCP_POS   = 4;
    const U32 TCP_MASK  = 1UL<<TCP_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~TCP_MASK;
    regvalue     |= Enable<<TCP_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set I2S Rx Pause Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @parma[in]	Enable	CTRUE	Indicate that I2S Rx Pause Enable
 *  						CFALSE	Indicate that I2S Rx Pause Disable
 *  @reutn		None.
 */
void    NX_I2S_RxChPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RCP_POS   = 3;
    const U32 RCP_MASK  = 1UL<<RCP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~RCP_MASK;
    regvalue     |= Enable<<RCP_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set DMA Tx Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Enable	CTRUE	Indicate that DMA Tx Enable
 *  						CFALSE	Indicate that DMA Tx Disable
 *  @return		None.
 */
void    NX_I2S_TxDmaEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TXD_POS   = 2;
    const U32 TXD_MASK  = 1UL<<TXD_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~TXD_MASK;
    regvalue     |= Enable<<TXD_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set DMA Rx Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Enable	CTRUE	Indicate that DMA Rx Enable
 *  						CFALSE	Indicate that DMA Rx Disable
 *  @return		None.
 */
void    NX_I2S_RxDmaEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RXD_POS   = 1;
    const U32 RXD_MASK  = 1UL<<RXD_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~RXD_MASK;
    regvalue     |= Enable<<RXD_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

// The FIFO-Related functions. 
//------------------------------------------------------------------------------
/**
 *  @brief      	Get c
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		Current reset number
 */
CBOOL   NX_I2S_GetChannelClockIndication( U32 ModuleIndex )
{
    const U32 LRI_POS   = 11;
    const U32 LRI_MASK  = 1UL << LRI_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (CBOOL)((ReadIO32(&pRegister->CON) & LRI_MASK)>>LRI_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get Tx FIFO is Empty or Not
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		CTRUE			Indicate that FIFO Empty
 *  				CFALSE			Indicate that FIFO Not Empty
 */
CBOOL   NX_I2S_GetTxFifoEmpty( U32 ModuleIndex )
{
    const U32 TFE_POS   = 10;
    const U32 TFE_MASK  = 1UL<<TFE_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (CBOOL)((ReadIO32(&pRegister->CON) & TFE_MASK)>>TFE_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get Rx FIFO is Empty or Not
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		CTRUE			Indicate that FIFO Empty
 *  				CFALSE			Indicate that FIFO Not Empty
 */

CBOOL   NX_I2S_GetRxFifoEmpty( U32 ModuleIndex )
{
    const U32 RFE_POS   = 9;
    const U32 RFE_MASK  = 1UL<<RFE_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (CBOOL)((ReadIO32(&pRegister->CON) & RFE_MASK)>>RFE_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get Tx FIFO is Full or Not
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		CTRUE			Indicate that FIFO Full
 *  				CFALSE			Indicate that FIFO Not Full
 */
CBOOL   NX_I2S_GetTxFifoFull( U32 ModuleIndex )
{
    const U32 TFF_POS   = 8;
    const U32 TFF_MASK  = 1UL<<TFF_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );


    return (CBOOL)((ReadIO32(&pRegister->CON) & TFF_MASK)>>TFF_POS);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get Rx FIFO is Full or Not
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		CTRUE			Indicate that FIFO Full
 *  				CFALSE			Indicate that FIFO Not Full
 */
CBOOL   NX_I2S_GetRxFifoFull( U32 ModuleIndex )
{
    const U32 RFF_POS   = 7;
    const U32 RFF_MASK  = 1UL<<RFF_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (CBOOL)((ReadIO32(&pRegister->CON) & RFF_MASK)>>RFF_POS);
}

//------------------------------------------------------------------------------
/**
  *  @brief		Set Transmit FIFO Flush.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	Enable	CTRUE	Indicate that Tx FIFO Flush.
  						CFALSE Indicate that Tx FIFO No Flush.
  *	@return		None.
  */
void    NX_I2S_TxFIFOFlushEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TFL_POS   = 15;
    const U32 TFL_MASK  = 1UL<<TFL_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
    regvalue      = ReadIO32(&pRegister->FIC);
    regvalue     &= ~TFL_MASK;
    regvalue     |= Enable<<TFL_POS;

    WriteIO32(&pRegister->FIC, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief		Get Transmit FIFO Data Count.
  *  @param[in]  ModuleIndex		A index of module.
  *  @return	   	Tx FIFO Count	( range : 0 ~ 64 depth )
  */
U32     NX_I2S_GetTxFIFODataCount( U32 ModuleIndex )
{
    const U32 TFC_POS   = 8;
    const U32 TFC_MASK  = 0x7F<<TFC_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (U32)( (ReadIO32(&pRegister->FIC) & TFC_MASK)>>TFC_POS );
}

//------------------------------------------------------------------------------
/**
  *  @brief		Set Receive FIFO Flush.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	Enable	CTRUE	Indicate that Rx FIFO Flush.
  						CFALSE Indicate that Rx FIFO No Flush.
  *	@return		None.
  */
void    NX_I2S_RxFIFOFlushEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RFL_POS   = 7;
    const U32 RFL_MASK  = 1UL<<RFL_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->FIC);
    regvalue     &= ~RFL_MASK;
    regvalue     |= Enable<<RFL_POS;

    WriteIO32(&pRegister->FIC, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief		Get Receive FIFO Data Count.
  *  @param[in]  ModuleIndex		A index of module.
  *  @return	   	Rx FIFO Count	( range : 0 ~ 64 depth )
  */
U32     NX_I2S_GetRxFifoDataCount( U32 ModuleIndex )
{
    const U32 RFC_POS   = 0;
    const U32 RFC_MASK  = 0x7F<<RFC_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (U32)( (ReadIO32(&pRegister->FIC) & RFC_MASK)>>RFC_POS );
}

// The Mode-Related functions. 
//------------------------------------------------------------------------------
/**
 *  @brief      	Set I2S Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Enable	CTRUE	Indicate that I2S Enable
 *  						CFALSE	Indicate that I2S Disable
 *  @return		None.
 */
void    NX_I2S_SetI2SEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 ACT_POS   = 0;
    const U32 ACT_MASK  = 1UL<<ACT_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->CON);
    regvalue     &= ~ACT_MASK;
    regvalue     |= Enable<<ACT_POS;

    WriteIO32(&pRegister->CON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get I2S Enable or Disable
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		CTRUE			Indicate that I2S Enable
 *  				CFALSE			Indicate that I2S Disable
 */
CBOOL    NX_I2S_GetI2SEnable( U32 ModuleIndex )
{
    const U32 ACT_POS   = 0;
    const U32 ACT_MASK  = 1UL<<ACT_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (CBOOL)(ReadIO32(&pRegister->CON) & ACT_MASK) >> ACT_POS ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set Codec Clock source select.
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Disable			(0: Internal Codec Clock(PCLK), 1: CDCLK )
 *  @return		None.
 */
void    NX_I2S_SetCodecClockDisable( U32 ModuleIndex, CBOOL Disable )
{
    const U32 CCE_POS   = 12;
    const U32 CCE_MASK  = 1UL<<CCE_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~CCE_MASK;
    regvalue     |= Disable<<CCE_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get Codec Clock source select.
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		BitLength		Channel Bit Length (0:16bit, 1:8bit, 2:24bit)
 */
CBOOL    NX_I2S_GetCodecClockDisable( U32 ModuleIndex )
{
    const U32 CCE_POS   = 12;
    const U32 CCE_MASK  = 1UL<<CCE_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->MOD) & CCE_MASK) >> CCE_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set IIS Master or Slave mode select.
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Mode			IIS MS Mode (0: Master Mode, 1:Slave Mode)			
 *  @return		None.
 */
void    NX_I2S_SetMasterSlaveMode( U32 ModuleIndex, NX_I2S_IMS Mode )
{
    const U32 IMS_POS   = 11;
    const U32 IMS_MASK  = 1UL<<IMS_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~IMS_MASK;
    regvalue     |= Mode<<IMS_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get IIS Master or Slave mode select.
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		Mode			IIS MS Mode (0: Master Mode, 1:Slave Mode)			
 */
NX_I2S_IMS	NX_I2S_GetMasterSlaveMode( U32 ModuleIndex )
{
    const U32 IMS_POS   = 11;
    const U32 IMS_MASK  = 1UL<<IMS_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (NX_I2S_IMS)(ReadIO32(&pRegister->MOD) & IMS_MASK) >> IMS_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set Transmit or Receive mode select.
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	Mode			IIS TXR Mode (0: Tx Only 1:Rx Only, 2: Tx & Rx)			
 *  @return		None.
 */
void    NX_I2S_SetTxRxMode( U32 ModuleIndex, NX_I2S_TXR Mode )
{
    const U32 TXR_POS   = 8;
    const U32 TXR_MASK  = 3UL<<TXR_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = pRegister->MOD;
    regvalue     &= ~TXR_MASK;
    regvalue     |= Mode<<TXR_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief 		 Get Transmit or Receive mode select.
  *  @param[in]  ModuleIndex		A index of module.
  *  @return	 	Mode		 	IIS TXR Mode (0: Tx Only 1:Rx Only, 2: Tx & Rx)			 
  */
 NX_I2S_TXR   	NX_I2S_GetTxRxMode( U32 ModuleIndex )
{
    const U32 TXR_POS   = 8;
    const U32 TXR_MASK  = 3UL<<TXR_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );;

    return (NX_I2S_TXR)(ReadIO32(&pRegister->MOD) & TXR_MASK) >> TXR_POS;
}

//------------------------------------------------------------------------------
/**
  *  @brief		Set LR Clock Polarity select.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	Polarity			LR Clock Polarity (0:Left Low/Right High, 1:Left High/Right Low)
  *	@return		None.
  */
void    NX_I2S_SetLRClockPolarity( U32 ModuleIndex, NX_I2S_LRP Polarity )
{
    const U32 LRP_POS   = 7;
    const U32 LRP_MASK  = 1UL<<LRP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~LRP_MASK;
    regvalue     |= Polarity<<LRP_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

 //------------------------------------------------------------------------------
 /**
   *  @brief	 	Set LR Clock Polarity select.
   *  @param[in]	ModuleIndex		 A index of module.
   *  @return		Polarity			 LR Clock Polarity (0:Left Low/Right High, 1:Left High/Right Low)
   */
NX_I2S_LRP	NX_I2S_GetLRClockPolarity( U32 ModuleIndex )
{
    const U32 LRP_POS   = 7;
    const U32 LRP_MASK  = 1UL<<LRP_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
    return (NX_I2S_LRP)(ReadIO32(&pRegister->MOD) & LRP_MASK) >> LRP_POS;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Set Transmission 8/16/24 bits per audio channel. 
 *  @param[in]	ModuleIndex		A index of module.
 *  @param[in]	BitLength		Channel Bit Length (0:16bit, 1:8bit, 2:24bit)
 *  @return		None.
 */
void    NX_I2S_SetBitLengthControl( U32 ModuleIndex, NX_I2S_BLC BitLength )
{
    const U32 BLC_POS   = 13;
    const U32 BLC_MASK  = 3UL<<BLC_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~BLC_MASK;
    regvalue     |= BitLength<<BLC_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	Get Transmission 8/16/24 bits per audio channel. 
 *  @param[in]	ModuleIndex		A index of module.
 *  @return		BitLength		Channel Bit Length (0:16bit, 1:8bit, 2:24bit)
 */
NX_I2S_BLC	NX_I2S_GetBitLengthControl( U32 ModuleIndex )
{
    const U32 BLC_POS   = 13;
    const U32 BLC_MASK  = 3UL<<BLC_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return (NX_I2S_BLC)(ReadIO32(&pRegister->MOD) & BLC_MASK) >> BLC_POS;
}


//------------------------------------------------------------------------------
/**
  *  @brief		Set Serial Data Format select.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	DataFormat		Serial Data Format (0:I2S, 1:MSB, 2:LSB)
  *	@return		None.
  */
void    NX_I2S_SetSerialDataFormat( U32 ModuleIndex, NX_I2S_SDF DataFormat )
{
    const U32 SDF_POS   = 5;
    const U32 SDF_MASK  = 3UL<<SDF_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~SDF_MASK;
    regvalue     |= DataFormat<<SDF_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief	 	Get Serial Data Format select.
  *  @param[in]  ModuleIndex		A index of module.
  *  @return		DataFormat 	 	Serial Data Format (0:I2S, 1:MSB, 2:LSB)
  */
NX_I2S_SDF	NX_I2S_GetSerialDataFormat( U32 ModuleIndex )
{
    const U32 SDF_POS   = 5;
    const U32 SDF_MASK  = 3UL<<SDF_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
    return (NX_I2S_SDF)(ReadIO32(&pRegister->MOD) & SDF_MASK) >> SDF_POS;
}

// The Sampling Clock-Related functions. 
//------------------------------------------------------------------------------
/**
  *  @brief		Set Root Clock Frequency select.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	RootClock		Root Clock Sampling (0:256FS, 1:512FS, 2:394FS, 3:768FS)
  *	@return		None.
  */
void    NX_I2S_SetRootClockFrequency( U32 ModuleIndex, NX_I2S_ROOTCLOCK RootClock )
{
    const U32 RFS_POS   = 3;
    const U32 RFS_MASK  = 3UL<<RFS_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~RFS_MASK;
    regvalue     |= RootClock<<RFS_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

 //------------------------------------------------------------------------------
 /**
   *  @brief	 	Get Root Clock Frequency select.
   *  @param[in]	ModuleIndex		 A index of module.
   *  @return	 	RootClock		 Root Clock Sampling (0:256FS, 1:512FS, 2:394FS, 3:768FS)
   */
NX_I2S_ROOTCLOCK   NX_I2S_GetRootClockFrequency( U32 ModuleIndex )
{
    const U32 RFS_POS   = 3;
    const U32 RFS_MASK  = 3UL<<RFS_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (NX_I2S_ROOTCLOCK)(ReadIO32(&pRegister->MOD) & RFS_MASK) >> RFS_POS;
}

//------------------------------------------------------------------------------
/**
  *  @brief		Set Bit Clock Frequency select.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	BitClock			Bit Clock Sampling (0:32FS, 1:48FS, 2:16FS, 3:24FS)
  *	@return		None.
  */
void    NX_I2S_SetBitClockFrequency( U32 ModuleIndex, NX_I2S_BITCLOCK BitClock )
{
    const U32 BFS_POS   = 1;
    const U32 BFS_MASK  = 3UL<<BFS_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~BFS_MASK;
    regvalue     |= BitClock<<BFS_POS;

    WriteIO32(&pRegister->MOD, regvalue);
}

 //------------------------------------------------------------------------------
 /**
   *  @brief	 	Get Bit Clock Frequency select.
   *  @param[in]	ModuleIndex		 A index of module.
   *  @return	 	BitClock			Bit Clock Sampling (0:32FS, 1:48FS, 2:16FS, 3:24FS)
   */
 NX_I2S_BITCLOCK   NX_I2S_GetBitClockFrequency( U32 ModuleIndex )
{
    const U32 BFS_POS   = 1;
    const U32 BFS_MASK  = 3UL<<BFS_POS;

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return	(NX_I2S_BITCLOCK)(ReadIO32(&pRegister->MOD) & BFS_MASK) >> BFS_POS;
}

// The Tramit/Receive-Related functions. 
//------------------------------------------------------------------------------
/**
  *  @brief		Set Transmit Data.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	ChannelIndex	Left / Right ( 0:Left, 1:Right )
  *  @param[in]	BitLength		Channel Bit Length ( 0:16Bit, 1:8Bit, 2:24Bit)  
  *  @param[in]	TxData			Set Transmission Data.  
  *	@return		None.
  */
void    NX_I2S_SetTxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength, U32 TxData )
{
	const U32 BLC_POS 	= 13;
	const U32 BLC_MASK	= 1UL << BLC_POS;
	
	register struct NX_I2S_RegisterSet* pRegister;
	register U32 regvalue;

    ChannelIndex = ChannelIndex;
    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	
    NX_ASSERT( BitLength==NX_I2S_BITLENGTH_24 || BitLength==NX_I2S_BITLENGTH_16 
		    || BitLength==NX_I2S_BITLENGTH_8 );

    regvalue      = ReadIO32(&pRegister->MOD);
    regvalue     &= ~BLC_MASK;
    regvalue     |= BitLength << BLC_POS;

	WriteIO32(&pRegister->MOD, regvalue);
#if 0
   	U32 tData			= 0;
	if( BitLength == NX_I2S_BITLENGTH_8 )
		tData	= (TxData & 0xFF);
	else if( BitLength == NX_I2S_BITLENGTH_16 )
		tData	= (TxData & 0xFFFF);

	if( ChannelIndex == NX_I2S_CH_RIGHT )
		tData <<= 0;
	if( ChannelIndex == NX_I2S_CH_LEFT )
		tData <<= 16; 

	WriteIO32(&pRegister->TXD, tData);
#else
	WriteIO32(&pRegister->TXD, TxData);
#endif
}

//------------------------------------------------------------------------------
/**
  *  @brief		Get Receive Data.
  *  @param[in]  ModuleIndex		A index of module.
  *  @param[in]	ChannelIndex	Left / Right ( 0:Left, 1:Right )
  *  @param[in]	BitLength		Channel Bit Length ( 0:16Bit, 1:8Bit, 2:24Bit)  
  *	@return		Get Rx FIFO Data.
  */
U32     NX_I2S_GetRxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength )
{
    const U32 RXD_POS[]   = {0, 16};
    const U32 RXD_MASK[]  = {0xFFFF, 0xFF};

    U32 RxDataPos   = RXD_POS[ChannelIndex];
    U32 RxDataMask  = RXD_MASK[BitLength];

	register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    ChannelIndex = ChannelIndex;
    BitLength = BitLength;    
    RxDataMask = RxDataMask;
    RxDataPos  = RxDataPos;
    
    return (U32)(ReadIO32(&pRegister->RXD));
	//return ((ReadIO32(&pRegister->RXD)) & RxDataMask);
}

