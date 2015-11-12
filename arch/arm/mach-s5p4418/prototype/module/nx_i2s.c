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
//	Module     : I2C
//	File       : nx_i2c.c
//	Description:
//	Author     : Firmware Team
//	History    : 2012.08.23 parkjh - xl00300_I2S proto type
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
 *	@see	NX_I2S_GetNumberOfModule
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
 *	@see		NX_I2S_Initialize
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
 *	@return		CTRUE	indicates that Ready to enter power-down stage. 
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
U32		NX_I2S_GetClockNumber( U32 ModuleIndex )
{
    // PHY_BASEADDR_CLKGEN_MODULE
    static const U32 CLKGEN_I2SLIST[] =
    {
        CLOCKINDEX_LIST( I2S )
    };

	NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

	return (U32)CLKGEN_I2SLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get current reset number
 *  @param[in]  ModuleIndex		A index of module.
 *  @return     Current reset number
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
/// @name   I2C Interface
//@{

// i2s con
CBOOL   NX_I2S_GetChannelClockIndication( U32 ModuleIndex )
{
    const U32 LRI_POS   = 11;
    const U32 LRI_MASK  = 1UL<<LRI_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->CON & LRI_MASK)>>LRI_POS);
}

CBOOL   NX_I2S_GetTxFifoEmpty( U32 ModuleIndex )
{
    const U32 TFE_POS   = 10;
    const U32 TFE_MASK  = 1UL<<TFE_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->CON & TFE_MASK)>>TFE_POS);
}

CBOOL   NX_I2S_GetRxFifoEmpty( U32 ModuleIndex )
{
    const U32 RFE_POS   = 9;
    const U32 RFE_MASK  = 1UL<<RFE_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->CON & RFE_MASK)>>RFE_POS);
}

CBOOL   NX_I2S_GetTxFifoFull( U32 ModuleIndex )
{
    const U32 TFF_POS   = 8;
    const U32 TFF_MASK  = 1UL<<TFF_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->CON & TFF_MASK)>>TFF_POS);
}

CBOOL   NX_I2S_GetRxFifoFull( U32 ModuleIndex )
{
    const U32 RFF_POS   = 7;
    const U32 RFF_MASK  = 1UL<<RFF_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (CBOOL)((__g_ModuleVariables[ModuleIndex].pRegister->CON & RFF_MASK)>>RFF_POS);
}

void    NX_I2S_TxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TDP_POS   = 6;
    const U32 TDP_MASK  = 1UL<<TDP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~TDP_MASK;
    RegVal     |= Enable<<TDP_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

void    NX_I2S_RxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RDP_POS   = 5;
    const U32 RDP_MASK  = 1UL<<RDP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~RDP_MASK;
    RegVal     |= Enable<<RDP_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

void    NX_I2S_TxChPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TCP_POS   = 4;
    const U32 TCP_MASK  = 1UL<<TCP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~TCP_MASK;
    RegVal     |= Enable<<TCP_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

void    NX_I2S_RxChPauseEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RCP_POS   = 3;
    const U32 RCP_MASK  = 1UL<<RCP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~RCP_MASK;
    RegVal     |= Enable<<RCP_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

void    NX_I2S_TxDmaEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TXD_POS   = 2;
    const U32 TXD_MASK  = 1UL<<TXD_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~TXD_MASK;
    RegVal     |= Enable<<TXD_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

void    NX_I2S_RxDmaEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RXD_POS   = 1;
    const U32 RXD_MASK  = 1UL<<RXD_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~RXD_MASK;
    RegVal     |= Enable<<RXD_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

void    NX_I2S_I2SEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 ACT_POS   = 0;
    const U32 ACT_MASK  = 1UL<<ACT_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->CON;
    RegVal     &= ~ACT_MASK;
    RegVal     |= Enable<<ACT_POS;

    WriteIO32(&pRegister->CON, RegVal);
}

U32    NX_I2S_GetI2SEnable( U32 ModuleIndex )
{
    const U32 ACT_POS   = 0;
    const U32 ACT_MASK  = 1UL<<ACT_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (pRegister->CON >>ACT_POS) & ACT_MASK;
}


// i2s mod
void    NX_I2S_SetBitLengthControl( U32 ModuleIndex, NX_I2S_BLC BitLength )
{
    const U32 BLC_POS   = 13;
    const U32 BLC_MASK  = 3UL<<BLC_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~BLC_MASK;
    RegVal     |= BitLength<<BLC_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

 U32   NX_I2S_GetBitLengthControl( U32 ModuleIndex )
{
    const U32 BLC_POS   = 13;
    const U32 BLC_MASK  = 3UL<<BLC_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (pRegister->MOD & BLC_MASK) >> BLC_POS;
}


void    NX_I2S_CodecClockDisable( U32 ModuleIndex, CBOOL Disable )
{
    const U32 CCE_POS   = 12;
    const U32 CCE_MASK  = 1UL<<CCE_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~CCE_MASK;
    RegVal     |= Disable<<CCE_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

U32    NX_I2S_GetCodecClockDisable( U32 ModuleIndex )
{
    const U32 CCE_POS   = 12;
    const U32 CCE_MASK  = 1UL<<CCE_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

	return (pRegister->MOD & CCE_MASK) >> CCE_POS;
}


void    NX_I2S_SetMasterSlaveMode( U32 ModuleIndex, NX_I2S_IMS Mode )
{
    const U32 IMS_POS   = 10;
    const U32 IMS_MASK  = 3UL<<IMS_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~IMS_MASK;
    RegVal     |= Mode<<IMS_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

U32    NX_I2S_GetMasterSlaveMode( U32 ModuleIndex )
{
    const U32 IMS_POS   = 10;
    const U32 IMS_MASK  = 3UL<<IMS_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (pRegister->MOD & IMS_MASK) >> IMS_POS;
}
void    NX_I2S_SetTxRxMode( U32 ModuleIndex, NX_I2S_TXR Mode )
{
    const U32 TXR_POS   = 8;
    const U32 TXR_MASK  = 3UL<<TXR_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~TXR_MASK;
    RegVal     |= Mode<<TXR_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

 U32   NX_I2S_GetTxRxMode( U32 ModuleIndex )
{
    const U32 TXR_POS   = 8;
    const U32 TXR_MASK  = 3UL<<TXR_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (pRegister->MOD & TXR_MASK) >> TXR_POS;
}


void    NX_I2S_SetLRClockPolarity( U32 ModuleIndex, NX_I2S_LRP Polarity )
{
    const U32 LRP_POS   = 7;
    const U32 LRP_MASK  = 1UL<<LRP_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~LRP_MASK;
    RegVal     |= Polarity<<LRP_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

 U32   NX_I2S_GetLRClockPolarity( U32 ModuleIndex )
{
    const U32 LRP_POS   = 7;
    const U32 LRP_MASK  = 1UL<<LRP_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (pRegister->MOD & LRP_MASK) >> LRP_POS;
}


void    NX_I2S_SetSerialDataFormat( U32 ModuleIndex, NX_I2S_SDF DataFormat )
{
    const U32 SDF_POS   = 5;
    const U32 SDF_MASK  = 3UL<<SDF_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~SDF_MASK;
    RegVal     |= DataFormat<<SDF_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

 U32   NX_I2S_GetSerialDataFormat( U32 ModuleIndex )
{
    const U32 SDF_POS   = 5;
    const U32 SDF_MASK  = 3UL<<SDF_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    return (pRegister->MOD & SDF_MASK) >> SDF_POS;
}

void    NX_I2S_SetRootClockFrequency( U32 ModuleIndex, NX_I2S_ROOTCLOCK RootClock )
{
    const U32 RFS_POS   = 3;
    const U32 RFS_MASK  = 3UL<<RFS_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~RFS_MASK;
    RegVal     |= RootClock<<RFS_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

 U32   NX_I2S_GetRootClockFrequency( U32 ModuleIndex )
{
    const U32 RFS_POS   = 3;
    const U32 RFS_MASK  = 3UL<<RFS_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	RegVal		= (pRegister->MOD & RFS_MASK) >> RFS_POS;

    return RegVal;
}


void    NX_I2S_SetBitClockFrequency( U32 ModuleIndex, NX_I2S_BITCLOCK BitClock )
{
    const U32 BFS_POS   = 1;
    const U32 BFS_MASK  = 3UL<<BFS_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->MOD;
    RegVal     &= ~BFS_MASK;
    RegVal     |= BitClock<<BFS_POS;

    WriteIO32(&pRegister->MOD, RegVal);
}

 U32   NX_I2S_GetBitClockFrequency( U32 ModuleIndex )
{
    const U32 BFS_POS   = 1;
    const U32 BFS_MASK  = 3UL<<BFS_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;
	RegVal		= (pRegister->MOD & BFS_MASK) >> BFS_POS;

    return RegVal;

}

// fifo control
void    NX_I2S_TxFifoFlushEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 TFL_POS   = 15;
    const U32 TFL_MASK  = 1UL<<TFL_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->FIC;
    RegVal     &= ~TFL_MASK;
    RegVal     |= Enable<<TFL_POS;

    WriteIO32(&pRegister->FIC, RegVal);
}

U32     NX_I2S_GetTxFifoDataCount( U32 ModuleIndex )
{
    const U32 TFC_POS   = 8;
    const U32 TFC_MASK  = 0x1F<<TFC_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (U32)( (__g_ModuleVariables[ModuleIndex].pRegister->FIC & TFC_MASK)>>TFC_POS );
}

void    NX_I2S_RxFifoFlushEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 RFL_POS   = 7;
    const U32 RFL_MASK  = 1UL<<RFL_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->FIC;
    RegVal     &= ~RFL_MASK;
    RegVal     |= Enable<<RFL_POS;

    WriteIO32(&pRegister->FIC, RegVal);
}

U32     NX_I2S_GetRxFifoDataCount( U32 ModuleIndex )
{
    const U32 RFC_POS   = 0;
    const U32 RFC_MASK  = 0x1F<<RFC_POS;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    return (U32)( (__g_ModuleVariables[ModuleIndex].pRegister->FIC & RFC_MASK)>>RFC_POS );
}

// prescaler
void    NX_I2S_PrescalerEnable( U32 ModuleIndex, CBOOL Enable )
{
    const U32 PSE_POS   = 15;
    const U32 PSE_MASK  = 1UL<<PSE_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->PSR;
    RegVal     &= ~PSE_MASK;
    RegVal     |= Enable<<PSE_POS;

    WriteIO32(&pRegister->PSR, RegVal);
}

U32    	NX_I2S_GetPrescalerEnable( U32 ModuleIndex )
{
    const U32 PSE_POS   = 15;
    const U32 PSE_MASK  = 1UL<<PSE_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

	return (pRegister->PSR & PSE_MASK) >> PSE_POS;
}

void    NX_I2S_SetPrescalerValue( U32 ModuleIndex, U32 PsVal )
{
    const U32 PSV_POS  = 8;
	const U32 PSV_MASK = 0x3F << PSV_POS;

    register struct NX_I2S_RegisterSet* pRegister;
    register U32 RegVal;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );
    NX_ASSERT( PsVal < 256 );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    RegVal      = pRegister->PSR;
    RegVal     &= ~PSV_MASK;
    RegVal     |= (PsVal-1) << PSV_POS;

    WriteIO32(&pRegister->PSR, RegVal);
}

U32    NX_I2S_GetPrescalerValue( U32 ModuleIndex )
{
    const U32 PSV_POS  = 8;
	const U32 PSV_MASK = 0x3F << PSV_POS;

    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

	return (U32)((pRegister->PSR & PSV_MASK) >> PSV_POS) + 1;
}


// txd register
void    NX_I2S_SetTxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength, U32 TxData )
{
    register struct NX_I2S_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );
    NX_ASSERT( BitLength==NX_I2S_BITLENGTH_24 || BitLength==NX_I2S_BITLENGTH_16 || BitLength==NX_I2S_BITLENGTH_8 );

    pRegister   = __g_ModuleVariables[ModuleIndex].pRegister;

    ChannelIndex = ChannelIndex;
/*
	U32 tData	= 0;

	if( BitLength == NX_I2S_BITLENGTH_8 )
	{
		tData  = (TxData & 0xFF);
		tData |= (TxData & 0xFF00 ) << 8; 
		WriteIO32(&pRegister->TXD, tData);
	}
	else
*/
		 WriteIO32(&pRegister->TXD, TxData);

}


// rxd register
U32     NX_I2S_GetRxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength )
{
    const U32 RXD_POS[]   = {0, 16};
    const U32 RXD_MASK[]  = {0xFFFF, 0xFF};

    NX_ASSERT( NUMBER_OF_I2S_MODULE > ModuleIndex );
    NX_ASSERT( BitLength == NX_I2S_BITLENGTH_24 || BitLength==NX_I2S_BITLENGTH_16 || BitLength==NX_I2S_BITLENGTH_8 );

    U32 RxDataPos   = RXD_POS[ChannelIndex];
    U32 RxDataMask  = RXD_MASK[BitLength];

    return (U32)( (__g_ModuleVariables[ModuleIndex].pRegister->RXD & (RxDataMask<<RxDataPos))>>RxDataPos );
}




#if 0
//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number ( 0 : PCM Output Buffer Underfow, 1 : PCM Input Buffer Overflow ).
 *	@param[in]	Enable	CTRUE	indicates that Interrupt Enable. 
 *						CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_I2S_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
	register U32 ReadValue;

	NX_ASSERT( (0==IntNum) || (1==IntNum) );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = __g_pRegister->I2S_IRQ_ENA;

	ReadValue &= ~( 1UL << IntNum );
	ReadValue |= ( (U32)Enable << (IntNum) );

	__g_pRegister->I2S_IRQ_ENA = ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number( 0 : PCM Output Buffer Underfow, 1 : PCM Input Buffer Overflow ).
 *	@return		CTRUE	indicates that Interrupt is enabled. 
 *				CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_I2S_GetInterruptEnable( S32 IntNum )
{
	NX_ASSERT( (0==IntNum) || (1==IntNum) );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)( (__g_pRegister->I2S_IRQ_ENA) >> (IntNum)) & 0x01;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							- EnableFlag[0] : Set PCM Output Buffer Underfow interrupt enable or disable. 
 *							- EnableFlag[1] : Set PCM Input Buffer Overflow	interrupt enable or disable. 
 *	@return		None.
 */
void	NX_I2S_SetInterruptEnable32( U32 EnableFlag )
{
	const U32	ENB_MASK = (1<<2)-1;

	NX_ASSERT( CNULL != __g_pRegister );

	__g_pRegister->I2S_IRQ_ENA = (EnableFlag & ENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@return		Current setting value of interrupt. 
 *				"1" means interrupt is enabled. 
 *				"0" means interrupt is disabled. 
 *				- Return Value[0] : PCM Output Buffer Underfow interrupt's setting value. 
 *				- Return Value[1] : PCM Input Buffer Overflow	interrupt's setting value. 
 */
U32		NX_I2S_GetInterruptEnable32( void )
{
	const U32	ENB_MASK = (1<<2)-1;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)(__g_pRegister->I2S_IRQ_ENA & ENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number( 0 : PCM Output Buffer Underfow, 1 : PCM Input Buffer Overflow ).
 *	@return		CTRUE	indicates that Pending is seted. 
 *				CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_I2S_GetInterruptPending( S32 IntNum )
{
	NX_ASSERT( (0==IntNum) || (1==IntNum) );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)( (__g_pRegister->I2S_IRQ_PEND) >> (IntNum)) & 0x01;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@return		Current setting value of pending bit. 
 *				"1" means pend bit is occured. 
 *				"0" means pend bit is NOT occured. 
 *				- Return Value[0] : PCM Output Buffer Underfow pending state. 
 *				- Return Value[1] : PCM Input Buffer Overflow	pending state. 
 */
U32		NX_I2S_GetInterruptPending32( void )
{
	const U32 PEND_MASK = (1<<2)-1;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)(__g_pRegister->I2S_IRQ_PEND & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt number( 0 : PCM Output Buffer Underfow, 1 : PCM Input Buffer Overflow ).
 *	@return		None.
 */
void	NX_I2S_ClearInterruptPending( S32 IntNum )
{
	NX_ASSERT( (0==IntNum) || (1==IntNum) );
	NX_ASSERT( CNULL != __g_pRegister );

	__g_pRegister->I2S_IRQ_PEND = (u16)(1UL << (IntNum)) ;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	 
 *								- PendingFlag[0] : PCM Output Buffer Underfow pending bit. 
 *								- PendingFlag[1] : PCM Input Buffer Overflow	pending bit. 
 *	@return		None.
 */
void	NX_I2S_ClearInterruptPending32( U32 PendingFlag )
{
	const U32	PEND_MASK	= (1<<2)-1;

	NX_ASSERT( CNULL != __g_pRegister );

	__g_pRegister->I2S_IRQ_PEND = (PendingFlag & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	Enable	CTRUE	indicates that Set to all interrupt enable. 
 *						CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_I2S_SetInterruptEnableAll( CBOOL Enable )
{
	NX_ASSERT( CNULL != __g_pRegister );

	if( CTRUE == Enable ){ __g_pRegister->I2S_IRQ_ENA = 0x03;		}
	else					{ __g_pRegister->I2S_IRQ_ENA = 0;		}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_I2S_GetInterruptEnableAll( void )
{
	const U32 IRQENA_MASK = 0x03;
	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->I2S_IRQ_ENA & IRQENA_MASK )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@return		CTRUE	indicates that At least one( or more ) pending is seted. 
 *				CFALSE	indicates that All pending is NOT seted.
 */
CBOOL	NX_I2S_GetInterruptPendingAll( void )
{
	const U32 IRQPEND_MASK = 0x03;
	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->I2S_IRQ_PEND & IRQPEND_MASK )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@return		None.
 */
void	NX_I2S_ClearInterruptPendingAll( void )
{
	const U32 IRQPEND_MASK = 0x03;
	NX_ASSERT( CNULL != __g_pRegister );

	__g_pRegister->I2S_IRQ_PEND = IRQPEND_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@return		Pending Number( If all pending is not set then return -1 ).
 *				0 (PCM Out), 1 (PCM In)
 */
S32		NX_I2S_GetInterruptPendingNumber( void )	// -1 if None
{
	const U32	POUND_PEND	=	1<<0;
	const U32	PIOVR_PEND	=	1<<1;

	register struct NX_I2S_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	Pend = ( pRegister->I2S_IRQ_ENA & pRegister->I2S_IRQ_PEND );

	if( Pend & POUND_PEND )
	{
		return 0;
	}
	else if( Pend & PIOVR_PEND )
	{
		return 1;
	}

	return -1;
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Get DMA peripheral index of I2S controller's pcm input
 *	@return DMA peripheral index of I2S controller's pcm input
 */
U32		NX_I2S_GetDMAIndex_PCMIn( void )
{
	return DMAINDEX_OF_I2S_MODULE_PCMIN;
}

/**
 *	@brief	Get DMA peripheral index of pcm output
 *	@return DMA peripheral index of I2S controller's pcm output
 */
U32		NX_I2S_GetDMAIndex_PCMOut( void )
{
	return DMAINDEX_OF_I2S_MODULE_PCMOUT;
}

/**
 *	@brief		Get bus width of I2S controller
 *	@return		DMA bus width of I2S controller.
 */
U32		NX_I2S_GetDMABusWidth( void )
{
	return 16;
}


//------------------------------------------------------------------------------
// Clock Control Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Set a PCLK mode
 *	@param[in]	mode	PCLK mode
 *	@return		None.
 *	@remarks	I2S controller only support NX_PCLKMODE_ALWAYS.
 *				If user set to NX_PCLKMODE_DYNAMIC, then I2S controller NOT operate.
 */
void			NX_I2S_SetClockPClkMode( NX_PCLKMODE mode )
{
	const U32 PCLKMODE_POS	=	3;

	register U32 regvalue;
	register struct NX_I2S_RegisterSet* pRegister;

	U32 clkmode=0;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	switch(mode)
	{
		case NX_PCLKMODE_DYNAMIC:	clkmode = 0;		break;
		case NX_PCLKMODE_ALWAYS:	clkmode = 1;		break;
		default: NX_ASSERT( CFALSE );
	}

	regvalue = pRegister->CLKENB;

	regvalue &= ~(1UL<<PCLKMODE_POS);
	regvalue |= ( clkmode & 0x01 ) << PCLKMODE_POS;

	pRegister->CLKENB = regvalue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current PCLK mode
 *	@return		Current PCLK mode
 */
NX_PCLKMODE	NX_I2S_GetClockPClkMode( void )
{
	const U32 PCLKMODE_POS	= 3;

	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->CLKENB & ( 1UL << PCLKMODE_POS ) )
	{
		return NX_PCLKMODE_ALWAYS;
	}

	return	NX_PCLKMODE_DYNAMIC;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock source of clock generator
 *	@param[in]	Index	Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	ClkSrc	Select clock source of clock generator.
 *						0:PLL0, 1:PLL1, 2:None, 3:Bit Clock, 4: Invert Bit Clock
 *						5:AV Clock, 6:Invert AV Clock, 7: CLKGEN0's output clock ( Only clock generator1 use ).
 *	@remarks	I2S controller have two clock generator. so \e Index must set to 0 or 1.
 *	@return		None.
 */
void	NX_I2S_SetClockSource( U32 Index, U32 ClkSrc )
{
	const U32 CLKSRCSEL_POS		= 2;
	const U32 CLKSRCSEL_MASK	= 0x07 << CLKSRCSEL_POS;

	register U32 ReadValue;

	NX_ASSERT( 2 > Index );
	NX_ASSERT( (0!=Index) || ( (2!=ClkSrc) && (ClkSrc<=6) ) );
	NX_ASSERT( (1!=Index) || ( (2!=ClkSrc) && (ClkSrc<=7) ) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = __g_pRegister->CLKGEN[Index][0];

	ReadValue &= ~CLKSRCSEL_MASK;
	ReadValue |= ClkSrc << CLKSRCSEL_POS;

	__g_pRegister->CLKGEN[Index][0] = ReadValue;

}

//------------------------------------------------------------------------------
/**
 *	@brief		Get clock source of specified clock generator.
 *	@param[in]	Index	Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@return		Clock source of clock generator 
 *				0:PLL0, 1:PLL1, 2:None, 3:Bit Clock, 4: Invert Bit Clock
 *				5:AV Clock, 6:Invert AV Clock, 7: CLKGEN0's output clock
 *	@remarks	I2S controller have two clock generator. so \e Index must set to 0 or 1.
 */
U32				NX_I2S_GetClockSource( U32 Index )
{
	const U32 CLKSRCSEL_POS		= 2;
	const U32 CLKSRCSEL_MASK	= 0x07 << CLKSRCSEL_POS;

	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_pRegister );

	return ( __g_pRegister->CLKGEN[Index][0] & CLKSRCSEL_MASK ) >> CLKSRCSEL_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock divisor of specified clock generator.
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	Divisor		Clock divisor ( 1 ~ 256 ).
 *	@return		None.
 *	@remarks	I2S controller have two clock generator. so \e Index must set to 0 or 1.
 */
void			NX_I2S_SetClockDivisor( U32 Index, U32 Divisor )
{
	const U32 CLKDIV_POS	=	5;
	const U32 CLKDIV_MASK	=	0xFF << CLKDIV_POS;

	register U32 ReadValue;

	NX_ASSERT( 2 > Index );
	NX_ASSERT( 1 <= Divisor && Divisor <= 256 );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue	=	__g_pRegister->CLKGEN[Index][0];

	ReadValue	&= ~CLKDIV_MASK;
	ReadValue	|= (Divisor-1) << CLKDIV_POS;

	__g_pRegister->CLKGEN[Index][0] = ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get clock divisor of specified clock generator.
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@return		Clock divisor ( 1 ~ 256 ).
 *	@remarks	I2S controller have two clock generator. so \e Index must set to 0 or 1.
 */
U32				NX_I2S_GetClockDivisor( U32 Index )
{
	const U32 CLKDIV_POS	=	5;
	const U32 CLKDIV_MASK	=	0xFF << CLKDIV_POS;

	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_pRegister );

	return ((__g_pRegister->CLKGEN[Index][0] & CLKDIV_MASK) >> CLKDIV_POS) + 1;

}

//------------------------------------------------------------------------------
/**
 *	@brief		Set inverting of output clock
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 );
 *	@param[in]	OutClkInv	CTRUE	indicates that Output clock Invert.
 *							CFALSE	indicates that Output clock Normal.
 *	@return		None.
 *	@remarks	I2S controller have two clock generator. so \e Index must set to 0 or 1.
 */
void			NX_I2S_SetClockOutInv( U32 Index, CBOOL OutClkInv )
{
	const U32 OUTCLKINV_POS		=	1;
	const U32 OUTCLKINV_MASK	=	1UL << OUTCLKINV_POS;

	register U32 ReadValue;

	NX_ASSERT( 2 > Index );
	NX_ASSERT( (0==OutClkInv) ||(1==OutClkInv) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue	=	__g_pRegister->CLKGEN[Index][0];

	ReadValue	&=	~OUTCLKINV_MASK;
	ReadValue	|=	OutClkInv << OUTCLKINV_POS;

	__g_pRegister->CLKGEN[Index][0]	=	ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get invert status of output clock.
 *	@param[in]	Index		Select clock generator( 0 : clock generator 0, 1: clock generator1 )
 *	@return		CTRUE	indicates that Output clock is Inverted. 
 *				CFALSE	indicates that Output clock is Normal.
 *	@remarks	I2S controller have two clock generator. so \e Index must set to 0 or 1.
 */
CBOOL			NX_I2S_GetClockOutInv( U32 Index )
{
	const U32 OUTCLKINV_POS		=	1;
	const U32 OUTCLKINV_MASK	=	1UL << OUTCLKINV_POS;

	NX_ASSERT( 2 > Index );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->CLKGEN[Index][0] & OUTCLKINV_MASK ) >> OUTCLKINV_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock output pin's direction.
 *	@param[in]	Index		Select clock generator( 1: clock generator1 );
 *	@param[in]	OutClkEnb	CTRUE	indicates that Pin's direction is Output.
 *							CFALSE	indicates that Pin's direction is Iutput.
 *	@return		None.
 *	@remarks	Decides the I/O direction when the output clock is connected to a bidirectional PAD.
 *				Only clock generator 1 can set the output pin's direction.
 */
void			NX_I2S_SetClockOutEnb( U32 Index, CBOOL OutClkEnb )
{
	const U32	OUTCLKENB1_POS	=	15;
	const U32	OUTCLKENB1_MASK = 1UL << OUTCLKENB1_POS;

	register U32 ReadValue;

	NX_ASSERT( 1 == Index );
	NX_ASSERT( (0==OutClkEnb) ||(1==OutClkEnb) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue	=	__g_pRegister->CLKGEN[Index][0];

	ReadValue	&=	~OUTCLKENB1_MASK;

	if( ! OutClkEnb )
	{
			ReadValue	|=	1UL << OUTCLKENB1_POS;
	}

	__g_pRegister->CLKGEN[Index][0] = ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get clock output pin's direction.
 *	@param[in]	Index		Select clock generator( 1: clock generator1 );
 *	@return		CTRUE	indicates that Pin's direction is Output.
 *				CFALSE	indicates that Pin's direction is Input.
 *	@remarks	Only clock generator 1 can set the output pin's direction. so \e Index must set to 1.
 */
CBOOL			NX_I2S_GetClockOutEnb( U32 Index )
{
	const U32	OUTCLKENB1_POS	=	15;
	const U32	OUTCLKENB1_MASK = 1UL << OUTCLKENB1_POS;

	NX_ASSERT( 1 == Index );
	NX_ASSERT( CNULL != __g_pRegister );

	if( __g_pRegister->CLKGEN[Index][0] & OUTCLKENB1_MASK )
	{
		return CFALSE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock generator's operation
 *	@param[in]	Enable	CTRUE	indicates that Enable of clock generator. 
 *						CFALSE	indicates that Disable of clock generator.
 *	@return		None.
 */
void			NX_I2S_SetClockDivisorEnable( CBOOL Enable )
{
	const U32	CLKGENENB_POS	=	2;
	const U32	CLKGENENB_MASK	=	1UL << CLKGENENB_POS;

	register U32 ReadValue;

	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue	=	__g_pRegister->CLKENB;

	ReadValue	&=	~CLKGENENB_MASK;
	ReadValue	|= (U32)Enable << CLKGENENB_POS;

	__g_pRegister->CLKENB	=	ReadValue;

}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of clock generator's operation
 *	@return		CTRUE	indicates that Clock generator is enabled.
 *				CFALSE	indicates that Clock generator is disabled.
 */
CBOOL			NX_I2S_GetClockDivisorEnable( void )
{
	const U32	CLKGENENB_POS	=	2;
	const U32	CLKGENENB_MASK	=	1UL << CLKGENENB_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (__g_pRegister->CLKENB & CLKGENENB_MASK) >> CLKGENENB_POS );
}


//------------------------------------------------------------------------------
// I2S Configuration Function
//------------------------------------------------------------------------------
/**
 *	@brief		Set I2S's Controller Mode ( Master or Slave )
 *	@param[in]	Enable	CTRUE	indicates that Master mode. 
 *						CFALSE	indicates that Slave mode.
 *	@return		None.
 */
void		NX_I2S_SetMasterMode( CBOOL Enable )
{
	const U32	MST_SLV_POS		=	0;
	const U32	MST_SLV_MASK	=	1UL << MST_SLV_POS;
	register U32 ReadValue;

	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue	=	__g_pRegister->I2S_CONFIG;

	ReadValue	&=	~MST_SLV_MASK;

	if( CTRUE != Enable )
	{
		ReadValue	|=	1UL << MST_SLV_POS;
	}

	__g_pRegister->I2S_CONFIG	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get I2S's Controller Mode ( Master or Slave )
 *	@return		CTRUE	indicates that Master mode.
 *				CFALSE	indicates that Slave mode.
 */
CBOOL		NX_I2S_GetMasterMode( void )
{
	const U32	MST_SLV_POS		=	0;
	const U32	MST_SLV_MASK	=	1UL << MST_SLV_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	if(__g_pRegister->I2S_CONFIG & MST_SLV_MASK)
	{
		return CFALSE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set I2S interface mode.
 *	@param[in]	mode	Interface mode ( 0 : I2S, 1 : None, 2 : Left-Justified, 3 : Right-Justified )
 *	@return		None.
 */
void			NX_I2S_SetInterfaceMode( NX_I2S_IF mode )
{
	const U32	IF_MODE_POS		=	6;
	const U32	IF_MODE_MASK	=	0x03UL << IF_MODE_POS;
	register U32 ReadValue;

	NX_ASSERT( (0==mode) || (2==mode) || (3==mode) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue	=	__g_pRegister->I2S_CONFIG;

	ReadValue	&=	~IF_MODE_MASK;
	ReadValue	|=	(U32)mode << IF_MODE_POS;

	__g_pRegister->I2S_CONFIG	=	(u16)ReadValue;

}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of I2S interface mode.
 *	@return		Interface mode ( 0 : I2S, 1 : None, 2 : Left-Justified, 3 : Right-Justified	)
 */
NX_I2S_IF	NX_I2S_GetInterfaceMode( void )
{
	const U32	IF_MODE_POS		=	6;
	const U32	IF_MODE_MASK	=	0x03UL << IF_MODE_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(NX_I2S_IF)( (__g_pRegister->I2S_CONFIG & IF_MODE_MASK) >> IF_MODE_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Sync Period of I2S ( 32fs, 48fs, 64fs )
 *	@param[in]	period		Sync Period ( 32fs, 48fs, 64fs )
 *	@return		None.
 *	@remarks	The setting is required in mater mode also.
 */
void	NX_I2S_SetSyncPeriod( U32 period )
{
	const U32	SYNC_PERIOD_POS		=	4;
	const U32	SYNC_PERIOD_MASK	=	0x03UL << SYNC_PERIOD_POS;

	register U32 ReadValue;
	register U32 SetValue=0;

	NX_ASSERT( (32==period) ||(48==period) || (64==period) );
	NX_ASSERT( CNULL != __g_pRegister );

	switch( period )
	{
		case	32: SetValue = 0;	break;
		case	48: SetValue = 1;	break;
		case	64: SetValue = 2;	break;
		default:	NX_ASSERT( CFALSE );
	}

	ReadValue	=	__g_pRegister->I2S_CONFIG;

	ReadValue	&=	~SYNC_PERIOD_MASK;
	ReadValue	|=	(U32)SetValue << SYNC_PERIOD_POS;

	__g_pRegister->I2S_CONFIG	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Sync Period of I2S ( 32fs, 48fs, 64fs )
 *	@return		Sync Period ( 32fs, 48fs, 64fs )
 */
U32	NX_I2S_GetSyncPeriod( void )
{
	const U32	SYNC_PERIOD_POS		=	4;
	const U32	SYNC_PERIOD_MASK	=	0x03UL << SYNC_PERIOD_POS;

	register U32 ReadValue;
	register U32 RetValue=0;

	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ((__g_pRegister->I2S_CONFIG & SYNC_PERIOD_MASK) >> SYNC_PERIOD_POS );

	switch( ReadValue )
	{
		case	0: RetValue = 32;	break;
		case	1: RetValue = 48;	break;
		case	2: RetValue = 64;	break;
		default:	NX_ASSERT( CFALSE );
	}

	return RetValue;
}


//--------------------------------------------------------------------------
// I2S Control Function
//--------------------------------------------------------------------------
/**
 *	@brief		Set I2S controller Link On
 *	@return		None.
 *	@remarks	If user want I2S link off, reset the I2S controller
 */
void	NX_I2S_SetLinkOn( void )
{
	const U32 I2SLINK_RUN_MASK	= 1UL << 1;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	=	__g_pRegister;

	ReadValue	=	pRegister->I2S_CTRL;

	ReadValue	|=	I2SLINK_RUN_MASK;

	pRegister->I2S_CTRL =	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get link status of I2S controller
 *	@return		CTRUE	indicates that Link On. 
 *				CFALSE	indicates that None.
 */
CBOOL NX_I2S_GetI2SLinkOn( void )
{
	const U32 I2SLINK_RUN_POS	= 1;
	const U32 I2SLINK_RUN_MASK	= 1UL << I2SLINK_RUN_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->I2S_CTRL & I2SLINK_RUN_MASK) >> I2SLINK_RUN_POS);
}

 //------------------------------------------------------------------------------
/**
 *	@brief		I2S Controller Reset
 *	@param[in]	Enable		CTRUE	indicates that Contoller Reset.
 *							CFALSE	indicates that Nomal Operation.
 *	@return		None.
 *	@remarks	After Reset You should set normal operation
 *	@code
 *				NX_I2S_SetControllerReset( CTRUE );		// I2S Controller Reset
 *				NX_I2S_SetControllerReset( CFALSE );	// Normal Operation
 *	@endcode
 */
void	NX_I2S_SetControllerReset( CBOOL Enable )
{
	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	if( Enable )
	{
		__g_pRegister->I2S_CTRL = 0x00;
	}
	else
	{
		__g_pRegister->I2S_CTRL = 0x01;
	}
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get status of I2S Controller
 *	@return		CTRUE	indicates that Contoller Reset.
 *				CFALSE	indicates that Nomal Operation.
 */
CBOOL NX_I2S_GetControllerReset( void )
{
	const U32 I2S_EN_MASK	= 1UL << 0;

	NX_ASSERT( CNULL != __g_pRegister );

	if(__g_pRegister->I2S_CTRL & I2S_EN_MASK)
	{
		return CFALSE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set I2S controller's output operation
 *	@param[in]	Enable	CTRUE	indicates that Output Enable. 
 *						CFALSE	indicates that Output Disable.
 *	@return		None.
 */
void	NX_I2S_SetOutputEnable( CBOOL Enable )
{
	const U32 I2SO_EN_POS	= 1;
	const U32 I2SO_EN_MASK	= 1UL << I2SO_EN_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	=	__g_pRegister;

	ReadValue	=	pRegister->I2S_CONFIG;

	ReadValue	&=	~I2SO_EN_MASK;
	ReadValue	|=	(U32)Enable << I2SO_EN_POS;

	pRegister->I2S_CONFIG	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get output operation status of I2S controller
 *	@return		CTRUE	indicates that Output is Enabled. 
 *				CFALSE	indicates that Output is Disabled.
 */
CBOOL NX_I2S_GetI2SOutputEnable( void )
{
	const U32 I2SO_EN_POS	= 1;
	const U32 I2SO_EN_MASK	= 1UL << I2SO_EN_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (__g_pRegister->I2S_CONFIG & I2SO_EN_MASK) >> I2SO_EN_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set I2S controller's input operation
 *	@param[in]	Enable	CTRUE	indicates that Input Enable. 
 *						CFALSE	indicates that Input Disable.
 */
void	NX_I2S_SetInputEnable( CBOOL Enable )
{
	const U32 I2SI_EN_POS	= 2;
	const U32 I2SI_EN_MASK	= 1UL << I2SI_EN_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	=	__g_pRegister;

	ReadValue	=	pRegister->I2S_CONFIG;

	ReadValue	&=	~I2SI_EN_MASK;
	ReadValue	|=	(U32)Enable << I2SI_EN_POS;

	pRegister->I2S_CONFIG	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Input operation status of I2S controller
 *	@return		CTRUE	indicates that Input is Enabled. 
 *				CFALSE	indicates that Input is Disabled.
 */
CBOOL NX_I2S_GetInputEnable( void )
{
	const U32 I2SI_EN_POS	= 2;
	const U32 I2SI_EN_MASK	= 1UL << I2SI_EN_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (__g_pRegister->I2S_CONFIG & I2SI_EN_MASK) >> I2SI_EN_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Loop Back operation
 *	@param[in]	Enable	CTRUE	indicates that Loop Back mode Enable. 
 *						CFALSE	indicates that Loop Back mode Disable.
 *	@return		None.
 *	@remarks	User need to set NX_I2S_SetI2SInputEnable( CTRUE ) for look back operation.
 */
void	NX_I2S_SetLoopBackEnable( CBOOL Enable )
{
	const U32 LOOP_BACK_POS		= 3;
	const U32 LOOP_BACK_MASK	= 1UL << LOOP_BACK_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	=	__g_pRegister;

	ReadValue	=	pRegister->I2S_CONFIG;

	ReadValue	&=	~LOOP_BACK_MASK;
	ReadValue	|=	(U32)Enable << LOOP_BACK_POS;

	pRegister->I2S_CONFIG	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of Loop Back operation
 *	@return		CTRUE	indicates that Loop Back mode is Enabled.
 *				CFALSE	indicates that Loop Back mode is Disabled.
 */
CBOOL NX_I2S_GetLoopBackEnable( void )
{
	const U32 LOOP_BACK_POS		= 3;
	const U32 LOOP_BACK_MASK	= 1UL << LOOP_BACK_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (__g_pRegister->I2S_CONFIG & LOOP_BACK_MASK) >> LOOP_BACK_POS );
}

//--------------------------------------------------------------------------
// I2S Buffer Function
//--------------------------------------------------------------------------
/**
 *	@brief		Set I2S's output buffer operation.
 *	@param[in]	Enable	CTRUE	indicates that PCM output buffer Enable. 
 *						CFALSE	indicates that PCM output buffer Disable.
 *	@return		None.
 *	@remarks	I2S output buffer's enable and disable means that setting data transfer to the
 *				I2S buffer throught DMA.
 *				Enable( DMA can transfer data to I2S's output buffer )
 *				Disable( DMA can't transfer data to I2S's output buffer )
 */
void	NX_I2S_SetBufferPCMOUTEnable( CBOOL Enable )
{
	const U32 PCMOBUF_EN_POS	= 0;
	const U32 PCMOBUF_EN_MASK	= 1UL << PCMOBUF_EN_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	=	__g_pRegister;

	ReadValue	=	pRegister->I2S_BUFF_CTRL;

	ReadValue	&=	~PCMOBUF_EN_MASK;
	ReadValue	|=	(U32)Enable << PCMOBUF_EN_POS;

	pRegister->I2S_BUFF_CTRL	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get operation status of I2S output buffer
 *	@return		CTRUE	indicates that PCM output buffer is Enabled. 
 *				CFALSE	indicates that PCM output buffer is Disabled.
 *	@remarks	I2S output buffer's enable and disable means that setting data transfer to the
 *				I2S buffer throught DMA.
 *				Enable( DMA can transfer data to I2S's output buffer )
 *				Disable( DMA can't transfer data to I2S's output buffer )
 */
CBOOL NX_I2S_GetBufferPCMOUTEnable( void )
{
	const U32 PCMOBUF_EN_POS	= 0;
	const U32 PCMOBUF_EN_MASK	= 1UL << PCMOBUF_EN_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (__g_pRegister->I2S_BUFF_CTRL & PCMOBUF_EN_MASK) >> PCMOBUF_EN_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set I2S's input buffer operation.
 *	@param[in]	Enable	CTRUE	indicates that PCM input buffer Enable. 
 *						CFALSE	indicates that PCM input buffer Disable.
 *	@return		None.
 *	@remarks	I2S input buffer's enable and disable means that setting data transfer to the
 *				I2S buffer throught DMA.
 *				Enable( DMA can receive data from	I2S's input buffer )
 *				Disable( DMA can't receive data from I2S's input buffer )
 */
void	NX_I2S_SetBufferPCMINEnable( CBOOL Enable )
{
	const U32 PCMIBUF_EN_POS	= 2;
	const U32 PCMIBUF_EN_MASK	= 1UL << PCMIBUF_EN_POS;

	register struct NX_I2S_RegisterSet* pRegister;
	register U32 ReadValue;

	NX_ASSERT( (0==Enable) ||(1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister	=	__g_pRegister;

	ReadValue	=	pRegister->I2S_BUFF_CTRL;

	ReadValue	&=	~PCMIBUF_EN_MASK;
	ReadValue	|=	(U32)Enable << PCMIBUF_EN_POS;

	pRegister->I2S_BUFF_CTRL	=	(u16)ReadValue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get operation status of I2S input buffer
 *	@return		CTRUE	indicates that PCM input buffer is Enabled. 
 *				CFALSE	indicates that PCM input buffer is Disabled.
 *	@remarks	I2S input buffer's enable and disable means that setting data transfer to the
 *				I2S buffer throught DMA.
 *				Enable( DMA can receive data from	I2S's input buffer )
 *				Disable( DMA can't receive data from I2S's input buffer )
 */
CBOOL NX_I2S_GetBufferPCMINEnable( void )
{
	const U32 PCMIBUF_EN_POS	= 2;
	const U32 PCMIBUF_EN_MASK	= 1UL << PCMIBUF_EN_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (__g_pRegister->I2S_BUFF_CTRL & PCMIBUF_EN_MASK) >> PCMIBUF_EN_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check PCM input buffer's status
 *	@return		CTRUE	indicates that Input buffer is ready. 
 *				CFALSE	indicates that None.
 *	@remarks	Input buffer's ready means that Input buffer have some space to receive data.
 */
CBOOL NX_I2S_IsPCMInBufferReady( void )
{
	const U32	PIBUF_RDY_POS	= 1;
	const U32	PIBUF_RDY_MASK	= 1UL << PIBUF_RDY_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((__g_pRegister->I2S_STATUS1 & PIBUF_RDY_MASK) >> PIBUF_RDY_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check PCM output buffer's status
 *	@return		CTRUE	indicates that output buffer is ready. 
 *				CFALSE	indicates that None.
 *	@remarks	Output buffer's ready means that Output buffer have some data to send.
 */
CBOOL NX_I2S_IsPCMOutBufferReady( void )
{
	const U32	POBUF_RDY_POS	= 0;
	const U32	POBUF_RDY_MASK	= 1UL << POBUF_RDY_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)(__g_pRegister->I2S_STATUS1 & POBUF_RDY_MASK);
}
#endif
