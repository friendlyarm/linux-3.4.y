//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: SSP/SPI 
//	File			: nx_ssp.c
//	Description	: 
//	Author		: Deoks
//	History		: 2014.09.xx 	First implementation
//				  2014.10.15	Last implementation
//------------------------------------------------------------------------------


#include <nx_chip.h>
#include <nx_ssp.h>

static	struct
{
	struct NX_SSP_RegisterSet *pRegister;
} __g_ModuleVariables[NUMBER_OF_SSP_MODULE] = { {CNULL,}, };

static const U32 NX_SSP_TX_FIFO_FULL_LVL[10] = { 256,256,256 };
static const U32 NX_SSP_RX_FIFO_FULL_LVL[10] = { 256,256,256 };

//------------------------------------------------------------------------------
//	Module Interface
//------------------------------------------------------------------------------

/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE		Indicate that Initialize is successed.
 *			CFALSE		Indicate that Initialize is failed.
 */
CBOOL	NX_SSP_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_SSP_MODULE; i++)
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
U32		NX_SSP_GetNumberOfModule( void )
{
	return NUMBER_OF_SSP_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_SSP_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SSP ) };

	NX_CASSERT( NUMBER_OF_SSP_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_SSP_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_SSP_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ). 
 *	@param[in]	BaseAddress 	Module's base address
 *	@return		None.
 */
void	NX_SSP_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{

	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SSP_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_SSP_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ). 
 *	@return		CTRUE			Indicate that Initialize is successed.
 *				CFALSE			Indicate that Initialize is failed.
 */
CBOOL	NX_SSP_OpenModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			Indicate that Deinitialize is successed.
 *				CFALSE			Indicate that Deinitialize is failed.
 */
CBOOL	NX_SSP_CloseModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ). 
 *	@return		Module's Reset number
 */
U32		NX_SSP_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
    const U32 ResetPinNumber[2][NUMBER_OF_SSP_MODULE] =
    {
        { RESETINDEX_LIST( SSP, PRESETn )},
        { RESETINDEX_LIST( SSP, nSSPRST )},
    };

    return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ). 
 *	@return		Module's Clock number
 */
U32 	NX_SSP_GetClockNumber (U32 ModuleIndex)
{
    static const U32 CLKGEN_SSPLIST[] = { CLOCKINDEX_LIST( SSP ) };
    NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

    return (U32)CLKGEN_SSPLIST[ModuleIndex];
}


//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *   				this function only can use after transaction .
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).  
 *	@return		CTRUE			Indicate that Module is Busy.
 *				CFALSE			Indicate that Module is NOT Busy.
 */
CBOOL	NX_SSP_CheckBusy( U32 ModuleIndex )
{
	register U32 regvalue;
	U32 TX_DONE;

	register struct NX_SSP_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->SPI_STATUS);
	TX_DONE = (regvalue >> 25) & 0x1 ;

	if( (1 == TX_DONE) )
	{
		return CFALSE;
	}
	return CTRUE;
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module. ( 0 ~ x ). 
 *	@return		An interrupt number.
 *	@remark	Return value can be used for the interrupt controller module's
 *				functions.
 */
U32		NX_SSP_GetInterruptNumber( U32 ModuleIndex )
{
	const U32 INTNumber[NUMBER_OF_SSP_MODULE] =
	{
		INTNUM_LIST(SSP)
	};

	NX_CASSERT( NUMBER_OF_SSP_MODULE == (sizeof(INTNumber)/sizeof(INTNumber[0])) );
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	return INTNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		an index of module..
 *	@param[in]	IntNum			interrupt index number .
 *				[0] : TXFIFORDY,  [1] : RXFIFORDY,  [2] : TX_UNDERRUN,  [3] : TX_OVERRUN
 *				[4] : RX_UNDERRUN,  [5] : RX_OVERRUN,  [6] : TRAILING
 *	@param[in]	Enable	CTRUE	Indicate that Interrupt Enable.
 *						CFALSE	Indicate that Interrupt Disable.
 *	@return		None.
 *	@see also	Interrupt mask set or clear register, SSPIMSC
 */
void	NX_SSP_SetInterruptEnable		( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER  > IntNum );
	NX_ASSERT( (1 == Enable) || (0 == Enable) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->SPI_INT_EN);

	if( CTRUE == Enable )
	{
		regvalue |= ( 1UL << IntNum );
	}
	else
	{
		regvalue &= ~( 1UL << IntNum );
	}

	WriteIO32(&pRegister->SPI_INT_EN, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module. ( 0 ~ x ). 
 *	@param[in]	IntNum			interrupt index number .
 *				[0] : TXFIFORDY,  [1] : RXFIFORDY,  [2] : TX_UNDERRUN,  [3] : TX_OVERRUN
 *				[4] : RX_UNDERRUN,  [5] : RX_OVERRUN,  [6] : TRAILING
 *
 *	@return		CTRUE			Indicates an interrupt specified by @a IntNum is enabled.\n
 *				CFALSE			Indicates an interrupt specified by @a IntNum is disabled.
 *	@see also	Interrupt mask set or clear register, SSPIMSC
 */

CBOOL	NX_SSP_GetInterruptEnable	( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER > IntNum );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->SPI_INT_EN);

	if( 0 != (( 1UL << IntNum ) & regvalue) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	ChannelIndex	Interrupt Index Number .
 *				[0] : TXFIFORDY,  [1] : RXFIFORDY,  [2] : TX_UNDERRUN,  [3] : TX_OVERRUN
 *				[4] : RX_UNDERRUN,  [5] : RX_OVERRUN,  [6] : TRAILING
 *
 *	@return		CTRUE			Indicates an interrupt specified by @a IntNum is pended.\n
 *				CFALSE			Indicates an interrupt specified by @a IntNum is not pended.
 *	@remark	The interrupt pending status are logged regardless of interrupt
 *				enable status. Therefore the return value can be CTRUE even
 *				though the specified interrupt has been disabled.
 */
CBOOL	NX_SSP_GetInterruptPending( U32 ModuleIndex , NX_SSP_INTR_MASK IntNum )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_status;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER > IntNum );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	irq_status = ReadIO32(&pRegister->SPI_STATUS);
 
	if( 0 != ( ( 1UL << IntNum ) & irq_status) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module. ( 0 ~ x ). 
 *	@param[in]	InterruptNum	Interrupt Index Number .
 *				[0] : TXFIFORDY,  [1] : RXFIFORDY,  [2] : TX_UNDERRUN,  [3] : TX_OVERRUN
 *				[4] : RX_UNDERRUN,  [5] : RX_OVERRUN,  [6] : TRAILING
 *				only [0] and [1] can be pended..
 *	@return		None.
 */
void	NX_SSP_ClearInterruptPending	( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_pending ;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER > IntNum );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	if( IntNum == TRAILING   ) irq_pending = 1<<0;
	if( IntNum == RX_OVERRUN ) irq_pending = 1<<1;
	if( IntNum == TX_UNDERRUN) irq_pending = 1<<2;
	if( IntNum == TX_OVERRUN ) irq_pending = 1<<3;
	if( IntNum == RX_UNDERRUN) irq_pending = 1<<4;

	WriteIO32(&pRegister->PENDING_CLR_REG, irq_pending);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			Set it as CTRUE to enable all interrupts.
 *								Set it as CFALSE to disable all interrupts.
 *	@return		None.
 */
void	NX_SSP_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			new_irq_mask;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (1 == Enable) || (0 == Enable) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	if( CTRUE == Enable )
	{
		new_irq_mask = (0x7F); // All
	}
	else
	{
		new_irq_mask = (0x00);
	}

	WriteIO32(&pRegister->SPI_INT_EN, new_irq_mask);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE			Indicates there's interrupts which are enabled.\n
 *				CFALSE			Indicates there's no interrupt which are enabled.
 */
CBOOL	NX_SSP_GetInterruptEnableAll	( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_mask;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	irq_mask = ReadIO32(&pRegister->SPI_INT_EN);

	if( 0 != ((0x7f) & irq_mask) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE			Indicates there's interrupts which are pended.\n
 *				CFALSE			Indicates there's no interrupt which are pended.
 */
CBOOL	NX_SSP_GetInterruptPendingAll	( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			masked_irq_status;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	masked_irq_status = ReadIO32(&pRegister->SPI_STATUS);

	if( 0 != ( 0x3F & masked_irq_status) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 */
void	NX_SSP_ClearInterruptPendingAll	( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_pending = 0x1F;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->PENDING_CLR_REG, irq_pending);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		an interrupt number which has the most priority of pended interrupts.
 *				This value is one of @ref NX_SSP_INT enum.
 *				If there's no interrupt which is pended and unmasked, it returns -1.
 */

NX_SSP_INTR_MASK	NX_SSP_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_status;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	irq_status = ReadIO32(&pRegister->SPI_STATUS);
	if( (irq_status >> 0) & 0x01 ) return TXFIFO_RDY;
	if( (irq_status >> 1) & 0x01 ) return RXFIFO_RDY;
	if( (irq_status >> 2) & 0x01 ) return TX_UNDERRUN;
	if( (irq_status >> 3) & 0x01 ) return TX_OVERRUN;
	if( (irq_status >> 4) & 0x01 ) return RX_UNDERRUN;
	if( (irq_status >> 5) & 0x01 ) return RX_OVERRUN;
	if( (irq_status >> 6) & 0x01 ) return TRAILING;
	return -1; // no interrupt pending now
}

#if 0
//------------------------------------------------------------------------------
/**	@name		NX_SSP_EnablePAD
 *	@brief		SSP Pad Enable Function.
 *				for GPIO Alt & SwitchDevice Enable pin Config
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		an interrupt number which has the most priority of pended interrupts. \n
 *				This value is one of @ref NX_SSP_INT enum.
 *				If there's no interrupt which is pended and unmasked, it returns -1.
 */
void NX_SSP_EnablePAD( U32 ModuleIndex )
{
	U32 i;
	const U32 PADNumber[4][NUMBER_OF_SSP_MODULE] =	{
		 { PADINDEX_WITH_CHANNEL_LIST( SSP, SSPCLK_IO ) },
	     { PADINDEX_WITH_CHANNEL_LIST( SSP, SSPFSS ) },
	     { PADINDEX_WITH_CHANNEL_LIST( SSP, SSPRXD ) },
	     { PADINDEX_WITH_CHANNEL_LIST( SSP, SSPTXD ) }
	};
	NX_CASSERT( NUMBER_OF_SSP_MODULE == (sizeof(PADNumber[0])/sizeof(PADNumber[0][0])) );
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	for(i=0; i<sizeof( PADNumber)/sizeof(PADNumber[0]); i++)
	{
		NX_SWITCHDEVICE_Set_Switch_Enable ( PADNumber[i][ModuleIndex] );
		//@todo : GPIO Alt Select 부분 추가.
		#ifdef NUMBER_OF_GPIO_MODULE
		NX_GPIO_SetPadFunctionEnable ( PADNumber[i][ModuleIndex] );
		// SSP의 PAD를 빠르게..
		#endif
	}
}
#endif

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index for SSP/SPI Tx.
 *	@param[in]	ModuleIndex		an index of module. ( 0 ~ x ). 
 *	@return		DMA peripheral index for SSP/SPI Tx.
 */
U32		NX_SSP_GetDMAIndex_Tx( U32 ModuleIndex )
{
	const U32	SspDmaIndexTx[NUMBER_OF_SSP_MODULE] =
		{ DMAINDEX_WITH_CHANNEL_LIST(SSP,SSPTXDMA)  };

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	return SspDmaIndexTx[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index for SSP/SPI Rx.
 *	@param[in]	ModuleIndex		an index of module. ( 0 ~ x ). 
 *	@return		DMA peripheral index for SSP/SPI Rx.
 */
U32		NX_SSP_GetDMAIndex_Rx( U32 ModuleIndex )
{
	const U32	SspDmaIndexRx[NUMBER_OF_SSP_MODULE] =
		{ DMAINDEX_WITH_CHANNEL_LIST(SSP,SSPRXDMA)  };

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	return SspDmaIndexRx[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA bus width of SSP/SPI.
 *	@param[in]	ModuleIndex		an index of module. ( 0 ~ x ). 
 *	@return		DMA bus width of SSP/SPI.
 *				It returns 8 in case of the data bit width is between 1 and 8,
 *				16 in case of the data bit width is between 9 and 16.
 *	@remark	You have to call NX_SSP_SetBitWidth() to set the data bit
 *				width properly before using	this function.
 */
U32		NX_SSP_GetDMABusWidth( U32 ModuleIndex )
{
	U32	DSS;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	DSS = (U32)((ReadIO32(&pRegister->MODE_CFG)>>17) & 0x03);

	if( DSS == 0 )   	{ return  8; 	}
	if( DSS == 1 )   	{ return 16; 	}
	if( DSS == 2 )   	{ return 32; 	}

	return CFALSE;
}

//------------------------------------------------------------------------------
// @name	Clock Control Interface
// NK3 SPI has only 2x clock divider.
// SCLK_SPI must be divided in System Controller for operation .
//------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/// @name Configuration Function
//------------------------------------------------------------------------------
/**	
 *	@brief		Set Transmit DMA Mode
 *	@param[in]	ModuleIndex		An index of module. ( 0 ~ x ). 
 *	@param[in]	Enable	CTRUE  	Indicate that Transmit/Receive DMA Enable. 
 *						CFALSE 	Indicate that Transmit/Receive DMA Disable. 
 *	@return		None.
**/
void	NX_SSP_SetDMATransferMode( U32 ModuleIndex, CBOOL Enable )
{
	register U32 regvalue;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Enable || 0 == Enable);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);
	if( CTRUE == Enable )
	{
		regvalue |= (3UL<<1);
	}
	else
	{
		regvalue &= (~(3UL<<1));
	}
	WriteIO32(&pRegister->MODE_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**	
 *	@brief		Get All DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE 			Indicate that All DMA Enabled
 				CFALSE			Indicate that All DMA Disabled
**/
CBOOL	NX_SSP_GetDMATransferMode( U32 ModuleIndex )
{
	register U32 regvalue;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);
	if( 0 != ( (3UL<<1) & regvalue ) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**	
 *	@brief		Set Transmit DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	CTRUE 	Indicate that Transmit DMA Enable.
 *						CFALSE 	Indicate that Transmit DMA Disable. 
 *	@return		None.
**/
void	NX_SSP_SetDMATransmitMode( U32 ModuleIndex, CBOOL Enable )
{
	register U32 regvalue;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Enable || 0 == Enable);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);
	if( CTRUE == Enable )
	{
		regvalue |= (1UL<<1);
	}
	else
	{
		regvalue &= (~(1UL<<1));
	}
	WriteIO32(&pRegister->MODE_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Transmit DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE 		 	Indicate that Transmit DMA Enabled
 				CFALSE 			Indicate that Transmit DMA Disabled
**/
CBOOL	NX_SSP_GetDMATransmitMode( U32 ModuleIndex )
{
	register U32 regvalue;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);
	if( 0 != ( (1UL<<1) & regvalue ) )
	{
		return CTRUE;
	}
	return CFALSE;
}


//------------------------------------------------------------------------------
/**	
 *	@brief		Set Receive DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	CTRUE   Indicate that Receive DMA Enable.
 *						CFALSE  Indicate that Receive DMA Disable.
 *	@return		None.
**/
void	NX_SSP_SetDMAReceiveMode( U32 ModuleIndex, CBOOL Enable )
{
	register U32 regvalue;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Enable || 0 == Enable);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);
	if( CTRUE == Enable )
	{
		regvalue |= (1UL<<2);
	}
	else
	{
		regvalue &= (~(1UL<<2));
	}
	WriteIO32(&pRegister->MODE_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**	
 *	@brief		Get Receive DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE  			Indicate that Receive DMA Enabled
 				CFALSE 			Indicate that Receive DMA Disabled
**/
CBOOL	NX_SSP_GetDMAReceiveMode( U32 ModuleIndex )
{
	register U32 regvalue;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);
	if( 0 != ( (1UL<<2) & regvalue ) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set DMA Burst Size
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Mode			DMA Burstsize ( 0: Single, 1: Burst4 )
 *	@return		None.
 */
void	NX_SSP_SetDMABurstSize( U32 ModuleIndex , NX_SSP_DMA_BURSTSIZE Mode )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ( ReadIO32(&pRegister->MODE_CFG) ) & (~(1<<0));

	regval = regval | ( (Mode&1) <<0);
	WriteIO32( &pRegister->MODE_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA Burst Size
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		DMA Burstsize ( 0: Single, 1: Burst4 )
 */
NX_SSP_DMA_BURSTSIZE	NX_SSP_GetDMABurstSize( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (NX_SSP_DMA_BURSTSIZE)(ReadIO32(&pRegister->MODE_CFG) ) & (~(1<<0));

}


//------------------------------------------------------------------------------
/**
 *	@brief		Set Data bit width
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bitWidth	Value of Data Bit Width( 8, 16, 32 )
 *	@return		None.
 *	@remarks	NX_SSP_SetBitWidth() must invoke before NX_SSP_GetDMABusWidth()
 *	@code
 *				NX_SSP_SetBitWidth();
 *				NX_SSP_GetDMABusWidth();
 *	@endcode
 */
void	NX_SSP_SetBitWidth( U32 ModuleIndex, U32 bitWidth )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	U32		new_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (bitWidth == 32) || (bitWidth == 16) || (bitWidth == 8) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->MODE_CFG);
	// BUS_WIDTH
	if( bitWidth ==  8 ) new_sspcr0 = ( old_sspcr0 & (~(0x03<<17)) ) | (0<<17) ;
	if( bitWidth == 16 ) new_sspcr0 = ( old_sspcr0 & (~(0x03<<17)) ) | (1<<17) ;
	if( bitWidth == 32 ) new_sspcr0 = ( old_sspcr0 & (~(0x03<<17)) ) | (2<<17) ;

	// CH_WIDTH
	if( bitWidth ==  8 ) new_sspcr0 = ( new_sspcr0 & (~(0x03<<29)) ) | (0<<29) ;
	if( bitWidth == 16 ) new_sspcr0 = ( new_sspcr0 & (~(0x03<<29)) ) | (1<<29) ;
	if( bitWidth == 32 ) new_sspcr0 = ( new_sspcr0 & (~(0x03<<29)) ) | (2<<29) ;

	WriteIO32(&pRegister->MODE_CFG, new_sspcr0);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Data Bit Width
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Data Bit Width( 8, 16, 32 )
 *	@see
 */
U32		NX_SSP_GetBitWidth( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->MODE_CFG);

	if( 0 == ((old_sspcr0 >> 17)&0x03) ) return  8;
	if( 1 == ((old_sspcr0 >> 17)&0x03) ) return 16;
	if( 2 == ((old_sspcr0 >> 17)&0x03) ) return 32;

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Operation Mode ( Master or Slave )
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bSlave			CTRIE Indicate that Slave mode enable.
 *								CFALSE Indicate that Master mode enable.
 *	@return		None.
 */
void	NX_SSP_SetSlaveMode( U32 ModuleIndex, NX_SSP_MODE bSlave )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 1 == bSlave || 0 == bSlave);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);
	if( CTRUE == bSlave )
	{
		regvalue |= (1UL<<4); 	  // SLAVE
	}
	else
	{
		regvalue &= (~(1UL<<4)); // MASTER
	}

	WriteIO32(&pRegister->CH_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Operation Mode ( Master or Slave )
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRIE 			Indicate that Slave mode enable.
 *				CFALSE 			Indicate that Master mode enable.
 */
CBOOL	NX_SSP_GetSlaveMode( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);
	if( 0 != ((1UL<<4) & regvalue) )
	{
		return CFALSE;
	}
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Polarity of SSPCLK
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Invert	CTRUE	Indicate that SSPCLK Invert enable.
 *						CFALSE	Indicate that SSPCLK Invert disable.
 *	@return		None.
 *				This Function can work Motorola SPI frame format only. otherwise,
 *				ASSERT ERROR will be called
 */
void	NX_SSP_SetClockPolarityInvert( U32 ModuleIndex, CBOOL Invert )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 1 == Invert || 0 == Invert);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);

	if( CTRUE == Invert )
	{
		regvalue |= (1UL<<3);
	}
	else
	{
		regvalue &= (~(1UL<<3));
	}

	WriteIO32(&pRegister->CH_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of SSPCLK's polarity is invert or normal.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE			Indicate that SSPCLK's polarity is invert.
 *				CFALSE			Indicate that SSPCLK's polarity is normal.
 *	@remark	This Function can work Motorola SPI frame format only. otherwise,
 *				ASSERT ERROR will be called
 */
CBOOL	NX_SSP_GetClockPolarityInvert( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);

	if( 0 != ((1UL<<3) &  regvalue) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SPI format
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Format			SPI format ( Fomrt A [0] or Format B[1]  )
 *	@return		None.
 */
void	NX_SSP_SetSPIFormat( U32 ModuleIndex, NX_SSP_FORMAT Format)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (NX_SSP_FORMAT_A == Format) || (NX_SSP_FORMAT_B == Format) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);

	switch(Format)
	{
		case NX_SSP_FORMAT_B:
			regvalue |= (1UL<<2);
			break;
		case NX_SSP_FORMAT_A:
			regvalue &= (~(1UL<<2));
			break;
		default:
			NX_ASSERT( CFALSE ); //"Never Get Here" );
	}

	WriteIO32(&pRegister->CH_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get SPI format
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		SPI format ( Fomrt A [0] or Format B[1]  )
 */
NX_SSP_FORMAT	NX_SSP_GetSPIFormat( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);

	return  (NX_SSP_FORMAT)(0x01 &  (regvalue>>2) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set NCS Time Count
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	TimeCount		NCS Time ( range : 0 ~ 0x3F )
 *	@return		None
 *	@remarks	Controls the spacing between the frames.
 */
void	NX_SSP_SetNCSTIMECount( U32 ModuleIndex, U32 TimeCount)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3F >= TimeCount );

	regval = ( ReadIO32(&pRegister->CS_REG) ) & (~(0x3F<<4));
	regval = regval | ( (TimeCount & 0x3F) <<4);
	
	WriteIO32( &pRegister->CS_REG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get NCS Time Count
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		TimeCount		NCS Time ( range : 0 ~ 0x3F )
 *	@remarks	Controls the spacing between the frames.
 */
U32		NX_SSP_GetNCSTIMECount( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (ReadIO32(&pRegister->CS_REG)>>6) & 0xFF;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set Chip Select Low or High
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	NSSOUT			CS ( 0 : Active(Low) 1: InActive(High) )
 *	@return		None
 *	@remarks	Is used at the time of control, manually, the CS.
 */
void	NX_SSP_SetNSSOUT( U32 ModuleIndex, CBOOL NSSOUT )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0 == NSSOUT || 1 == NSSOUT );

	regval = ( ReadIO32(&pRegister->CS_REG) ) & (~(1<<0));

	regval = regval | ( (NSSOUT&1) <<0);
	WriteIO32( &pRegister->CS_REG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Chip Select Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Mode			CS Mode ( 0: Auto, 1: Manual )
 *	@return		None
 */
void	NX_SSP_SetCSMode( U32 ModuleIndex, NX_SSP_CS_MODE Mode )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ( ReadIO32(&pRegister->CS_REG) ) & (~(1<<1));
	regval = regval | ((Mode & 0x01)<<1);
	WriteIO32( &pRegister->CS_REG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Chip Select Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None
 */
NX_SSP_CS_MODE	NX_SSP_GetCSMode( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (NX_SSP_CS_MODE)(ReadIO32(&pRegister->CS_REG)>>1) & 1;
}


//--------------------------------------------------------------------------
/// @name Operation Function
//--------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI enable or disable. (RX & TX Channel On)
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bEnable			CTRUE Indicate that SSP/SPI Enable.
 *								CFALSE Indicate that SSP/SPI Disable.
 *	@return		None.
 */
void	NX_SSP_SetEnable( U32 ModuleIndex, CBOOL bEnable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 1 == bEnable || 0 == bEnable );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);

	if( bEnable )
	{
		regvalue |= (3UL<<0);
	}
	else
	{
		regvalue &= (~(3UL<<0));
	}

	WriteIO32(&pRegister->CH_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of SSP/SPI is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE			Indicate that SSP/SPI is enabled.
 *				CFALSE			Indicate that SSP/SPI is disabled.
 */
CBOOL	NX_SSP_GetEnable( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->CH_CFG);

	if( 3 == ((3UL<<0) & regvalue) )
	{
		return CTRUE;
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI is Trailing Count.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	TrailingCnt		trailing count value
 */
void	NX_SSP_SetTrailingCount( U32 ModuleIndex, U16 TrailingCnt )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( 0x3FF >= TrailingCnt );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->MODE_CFG);

	if( TrailingCnt )
	{
		regvalue |= (0x3FF<<19);
	}
	else
	{
		regvalue &= (~(0x3FF<<19));
	}

	WriteIO32(&pRegister->CH_CFG, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get SSP/SPI is Trailing Count.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Trailing Count	trailing count value
 */
U16		NX_SSP_GetTralingCount( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U16)((ReadIO32(&pRegister->MODE_CFG) >> 19) & 0x3FF );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get state of SSP/SPI is Trailing Byte.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Trailing Byte		trailing byte value
 *	@remark	Indicates the trailing bytes in the pipe now.
 */
U32		NX_SSP_GetTrailingByte( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U32)((ReadIO32(&pRegister->SPI_STATUS) >> 24) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Read Byte Data.
 *				GetData( U32 ModuleIndex, U32 DataWidth ) 를 사용하도록 권장.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Byte Data
 */
U8		NX_SSP_GetByte(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U8)((ReadIO32(&pRegister->SPI_RX_DATA)) & 0xFF);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Read Half Word Data( 2byte ).
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Half Word Data( 2byte )
 */
U16		NX_SSP_GetHalfWord(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U16)((ReadIO32(&pRegister->SPI_RX_DATA)) & 0xffff);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Read Word Data( 4byte ).
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Word Data( 4byte )
 */
U32		NX_SSP_GetWord(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U32)((ReadIO32(&pRegister->SPI_RX_DATA)) );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Send Byte.
 *				GetData( U32 ModuleIndex, U32 DataWidth ) 를 사용하도록 권장.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	ByteData		Value of Data ( 0x0 ~ 0xFF )
 *	@return		None.
 */
void	NX_SSP_PutByte(U32 ModuleIndex, U8 ByteData)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT(8 >= NX_SSP_GetBitWidth(ModuleIndex) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );


	WriteIO32(&pRegister->SPI_TX_DATA, (U8)ByteData);	//put the byte data
}

//------------------------------------------------------------------------------
/**
 *	@brief		Send Half Word( 2byte ).
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	HalfWordData	Value of Data ( 0x0 ~ 0xFFFF )
 *	@return		None.
 */
void	NX_SSP_PutHalfWord(U32 ModuleIndex, U16 HalfWordData)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	NX_ASSERT(9 <= NX_SSP_GetBitWidth(ModuleIndex) );

	WriteIO32(&pRegister->SPI_TX_DATA, (U16)HalfWordData);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Send Word( 4byte ).
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	WordData		Value of Data ( 0x0 ~ 0xFFFFFFFF )
 *	@return		None.
 */
void	NX_SSP_PutWord(U32 ModuleIndex, U32 WordData)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	NX_ASSERT(32 <= NX_SSP_GetBitWidth(ModuleIndex) );

	WriteIO32(&pRegister->SPI_TX_DATA, (U32)WordData);	
}


//--------------------------------------------------------------------------
/// @name FIFO State Check Function
/// @see also	Status Register, SSPSR
//--------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI enable or disable the packet count.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bEnable			CTRUE  Indicate that Packet Count Enable.
 *								CFALSE Indicate that Packet Count Disable.	
 *	@return		None.
 */
void	NX_SSP_SetPacketCountEnb( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 1 == Enable || 0 == Enable );

	regval = ( ReadIO32(&pRegister->PACKET_CNT_REG) ) & (~(Enable<<16));

	regval = regval | ( (Enable & 0x1) << 16);
	WriteIO32( &pRegister->PACKET_CNT_REG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI enable or disable the packet count.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		bEnable			CTRUE  Indicate that Packet Count Enable.
 *								CFALSE Indicate that Packet Count Disable.	
 */
CBOOL	NX_SSP_GetPacketCountEnb( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U32)((ReadIO32(&pRegister->PACKET_CNT_REG) >> 16) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI enable or disable the packet count.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	PacketCount		packet count ( range : 0 ~ 0xFFFF )
 *	@return		None
 */
void	NX_SSP_SetPacketCount( U32 ModuleIndex, U16 PacketCount )
{
	register struct NX_SSP_RegisterSet *pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0xFFFF > PacketCount );

	regvalue = ( ReadIO32(&pRegister->PACKET_CNT_REG) ) & (~(PacketCount<<0));

	regvalue = regvalue | ( (PacketCount & 0xFFFF) << 0);
	WriteIO32( &pRegister->PACKET_CNT_REG, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI enable or disable the packet count.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		PacketCount		packet count ( range : 0 ~ 0xFFFF )
 */
U16		NX_SSP_GetPacketCount( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U16)((ReadIO32(&pRegister->PACKET_CNT_REG) >> 0) & 0xFFFF);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Check Tx FIFO is Empty or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE 			Indicate that FIFO Empty.
 *				CFALSE 			Indicate that FIFO NOT Empty
 */
CBOOL	NX_SSP_IsTxFIFOEmpty(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	TX_FIFO_LVL;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	TX_FIFO_LVL = (ReadIO32(&pRegister->SPI_STATUS)>>6) & 0x1FF;

	if( 0 == (TX_FIFO_LVL) )
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
 *	@brief		Check Tx FIFO is Full or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE			Indicate that FIFO Full.
 *				CFALSE			Indicate that FIFO NOT Full.
 */
CBOOL	NX_SSP_IsTxFIFOFull(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	TX_FIFO_LVL;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	TX_FIFO_LVL = (ReadIO32(&pRegister->SPI_STATUS)>>6) & 0x1FF;

	if( NX_SSP_TX_FIFO_FULL_LVL[ModuleIndex] <= TX_FIFO_LVL )
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
 *	@brief		Check Rx FIFO is Empty or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE			Indicate that FIFO Empty.
 *				CFALSE			Indicate that FIFO NOT Empty.
 */
CBOOL	NX_SSP_IsRxFIFOEmpty(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	RX_FIFO_LVL;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	RX_FIFO_LVL = (ReadIO32(&pRegister->SPI_STATUS)>>15) & 0x1FF;

	if( 0 == RX_FIFO_LVL )
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
 *	@brief		Check Rx FIFO is Full or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE	Indicate that FIFO Full.
 *				CFALSE	Indicate that FIFO NOT Full.
 */
CBOOL	NX_SSP_IsRxFIFOFull(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	RX_FIFO_LVL;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	RX_FIFO_LVL = (ReadIO32(&pRegister->SPI_STATUS)>>15) & 0x1FF;

	if( NX_SSP_RX_FIFO_FULL_LVL[ModuleIndex] <= RX_FIFO_LVL )
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
 *	@brief		Check Tx FIFO is Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Tx FIFO Level	currnet FIFO level
 */
U32		NX_SSP_GetTxFIFOLVL(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (ReadIO32(&pRegister->SPI_STATUS)>>6) & 0x1FF;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check Rx FIFO is Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Rx FIFO Level	Currnet FIFO level
 */
U32		NX_SSP_GetRxFIFOLVL(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (ReadIO32(&pRegister->SPI_STATUS)>>15) & 0x1FF;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Rx FIFO Trigger Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	RX_RDY_LVEL	Set Trigger Level x4  
 *	@return		None
 *	@remark	
 */
void	NX_SSP_SetRXRDYLVL( U32 ModuleIndex, U32 RX_RDY_LVL )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3F >= RX_RDY_LVL );

	regval = ReadIO32(&pRegister->MODE_CFG);
	regval = regval & (~(0x3F << 11));
	regval = regval | (RX_RDY_LVL & 0x3F)<<11;
	WriteIO32( &pRegister->MODE_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Rx FIFO Trigger Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Get Rx Trgger Level
 */
U8		NX_SSP_GetRXRDYLVL( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U8)(ReadIO32(&pRegister->MODE_CFG) >> 11) & 0x3F;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx FIFO Trigger Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	TX_RDY_LVL		Set Trigger Level x4  
 *	@return		None
 *	@remark	
 */
void  	NX_SSP_SetTXRDYLVL( U32 ModuleIndex, U32 TX_RDY_LVL )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3F >= TX_RDY_LVL );

	regval = ReadIO32(&pRegister->MODE_CFG);
	regval = regval & (~(0x3F << 5));
	regval = regval | (TX_RDY_LVL & 0x3F)<<5;
	WriteIO32( &pRegister->MODE_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx FIFO Trigger Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Get Tx Trgger Level
 */
U8  	NX_SSP_GetTXRDYLVL( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (U8)(ReadIO32(&pRegister->MODE_CFG) >> 5) & 0x3F;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Check Tx/Rx is End or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE			Indicate that Tx/Rx is End.
 *				CFALSE			Indicate that Tx/Rx is NOT End.
 *	@remarks	This function is same to NX_SSP_GetInterruptPending(2)
 */
CBOOL	NX_SSP_IsTxRxEnd( U32 ModuleIndex )
{
	return NX_SSP_CheckBusy(ModuleIndex);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Reset Tx/Rx FIFO
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None
 *	@remarks	Transmiter / Receiver FIFO Reset.
 */
void	NX_SSP_ResetFIFO( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;
	U32 sw_rst_regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ( ReadIO32(&pRegister->CH_CFG) ) & (~(1<<5));

	sw_rst_regval = regval | (1<<5);
	WriteIO32( &pRegister->CH_CFG, sw_rst_regval ); // reset FIFO & state
	WriteIO32( &pRegister->CH_CFG, regval ); // reset leave
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set High Speed is enable or disable.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that High Speed Enable.
 *								CFALSE Indicate that High Speed Disable.	
 *	@return		None
 *	@remarks	When the slave mode, and adjusts the timing of the data capture.
 */
void  NX_SSP_SetHIGHSPEEDMode( U32 ModuleIndex, CBOOL Enable)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->CH_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<6))	;
	}
	else
	{
		regval = regval | (1<<6) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get High Speed is enable or disable.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that High Speed Enable.
 *				CFALSE Indicate that High Speed Disable.
 *	@remarks	When the slave mode, and adjusts the timing of the data capture.
 */
CBOOL  NX_SSP_GetHIGHSPEEDMode( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->CH_CFG)>>6) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Receive  Swap Enable.
 *								CFALSE Indicate that Receive Swap Disable.	
 *	@return		None
 *	@remarks	
 */
void  NX_SSP_SetTxSwapEnable( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<0))	;
	}
	else
	{
		regval = regval | (1<<0) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Enable			CTRUE  Indicate that Receive Swap Enable.
 *								CFALSE Indicate that Receive Swap Disable.	
 *	@remarks	
 */
CBOOL  NX_SSP_GetTxSwapEnable( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>0) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Rx Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Receive Swap Enable.
 *								CFALSE Indicate that Receive Swap Disable.	
 *	@return		None
 *	@remarks	
 */
void  NX_SSP_SetRxSwapEnable( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<4))	;
	}
	else
	{
		regval = regval | (1<<4) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Enable			CTRUE  Indicate Receive that Swap Enable.
 *								CFALSE Indicate  Receive that Swap Disable.	
 *	@remarks	
 */
CBOOL  NX_SSP_GetRxSwapEnable( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>4) & 1;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx HalfWord Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Swap HalfWord Enable.
 *								CFALSE Indicate that Swap HalfWord Disable.	
 *	@return		None
 *	@remarks	The role swap halfword of data to transfer.
 */
void  NX_SSP_SetTxHalfWordSwap( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<3))	;
	}
	else
	{
		regval = regval | (1<<3) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx HalfWord Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that HalfWord Swap Enable.
 *				CFALSE Indicate that HalfWord Swap Disable.
 */
CBOOL  NX_SSP_GetTxHalfWordSwap( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>3) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Rx HalfWord Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Swap HalfWord Enable.
 *								CFALSE Indicate that Swap HalfWord Disable.	
 *	@return		None
 *	@remarks	The role swap halfword of data to receive.
 */
void  NX_SSP_SetRxHalfWordSwap( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<7))	;
	}
	else
	{
		regval = regval | (1<<7) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Rx HalfWord Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that HalfWord Swap Enable.
 *				CFALSE Indicate that HalfWord Swap Disable..
 */
CBOOL  NX_SSP_GetRxHalfWordSwap( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>7) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx Byte Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Swap Byte Enable.
 *								CFALSE Indicate that Swap Byte Disable.	
 *	@return		None
 *	@remarks	The role swap bytes of data to transfer.
 */
void  NX_SSP_SetTxByteSwap( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<2))	;
	}
	else
	{
		regval = regval | (1<<2) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx Byte Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that Byte Swap Enable.
 *				CFALSE Indicate that Byte Swap Disable.
 */
CBOOL  NX_SSP_GetTxByteSwap( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>2) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Rx Byte Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Swap Byte Enable.
 *								CFALSE Indicate that Swap Byte Disable.	
 *	@return		None
 *	@remarks	The role swap bytes of data to receive.
 */
void  NX_SSP_SetRxByteSwap( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<6))	;
	}
	else
	{
		regval = regval | (1<<6) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Rx Byte Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that Byte Swap Enable.
 *				CFALSE Indicate that Byte Swap Disable.
 */
CBOOL  NX_SSP_GetRxByteSwap( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>6) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx Bit Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Swap Bit Enable.
 *								CFALSE Indicate that Swap Bit Disable.	
 *	@return		None
 *	@remarks	The role swap bits of data to transfer.
 */
void  NX_SSP_SetTxBitSwap( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<1))	;
	}
	else
	{
		regval = regval | (1<<1) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx Bit Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that Bit Swap Enable.
 *				CFALSE Indicate that Bit Swap Disable.
 */
CBOOL  NX_SSP_GetTxBitSwap( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>1) & 1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Rx Bit Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			CTRUE  Indicate that Swap Bit Enable.
 *								CFALSE Indicate that Swap Bit Disable.	
 *	@return		None
 *	@remarks	The role swap bits of data to receive.
 */
void  NX_SSP_SetRxBitSwap( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	regval;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	regval = ReadIO32(&pRegister->SWAP_CFG);
	if( Enable == CFALSE )
	{
		regval = regval & (~(1<<5))	;
	}
	else
	{
		regval = regval | (1<<5) ;
	}
	WriteIO32( &pRegister->CH_CFG, regval );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Rx Bit Swap is enable or disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE  Indicate that Bit Swap Enable.
 *				CFALSE Indicate that Bit Swap Disable.
 */
CBOOL  NX_SSP_GetRxBitSwap( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)(ReadIO32(&pRegister->SWAP_CFG)>>5) & 1;
}

