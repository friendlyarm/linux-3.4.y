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
//	Module		: pl02212 Ssp
//	File		: nx_ssp.c
//	Description	:
//	Author		:
//	History		:
// @choyk 2012/07/18 : pl02212 Ssp prototype
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_ssp.h"

static	struct
{
	struct NX_SSP_RegisterSet *pRegister;
} __g_ModuleVariables[NUMBER_OF_SSP_MODULE] = { {CNULL,}, };


//------------------------------------------------------------------------------
//	Module Interface
//------------------------------------------------------------------------------

/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicate that Initialize is successed.
 *			 CFALSE	indicate that Initialize is failed.
 *	@see	NX_SSP_GetNumberOfModule
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
 *	@see		NX_SSP_Initialize
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
 *	@return		Module's physical address
 */
U32		NX_SSP_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SSP ) }; // PHY_BASEADDR_UART?_MODULE

	NX_CASSERT( NUMBER_OF_SSP_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( PHY_BASEADDR_SSP0_MODULE == PhysicalAddr[0] );
	NX_ASSERT( PHY_BASEADDR_SSP1_MODULE == PhysicalAddr[1] );

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
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_SSP_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SSP ) }; // PHY_BASEADDR_UART?_MODULE

	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SSP_RegisterSet *)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_SSP_GetPhysicalAddress,		NX_SSP_GetSizeOfRegisterSet,
 *				NX_SSP_SetBaseAddress,
 *				NX_SSP_OpenModule,				NX_SSP_CloseModule,
 *				NX_SSP_CheckBusy,				NX_SSP_CanPowerDown
 */
void*	NX_SSP_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_SSP_OpenModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_SSP_CloseModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@name		NX_SSP_CheckBusy
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 *	@see also	Status register, SSPSR
 */
CBOOL	NX_SSP_CheckBusy( U32 ModuleIndex )
{
	U32	status_register_value;
	register struct NX_SSP_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	status_register_value = ReadIO32(&pRegister->SSPSR);

	if( 0 != (status_register_value & 0x10) ) // BSY : busy flag Seted..
	{
		return CTRUE;
	}
	else
	{
		return CFALSE;
	}

}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		An interrupt number.
 *	@remark		Return value can be used for the interrupt controller module's
 *				functions.
 */
U32		NX_SSP_GetInterruptNumber		( U32 ModuleIndex )
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
 *	@param[in]	ModuleIndex		Module Index Number .
 *	@param[in]	IntNum			Interrupt Index Number .
 *				[0] : RORIM,  [1] : RTIM,  [2] : RXIM,  [3] : TXIM
 *	@param[in]	Enable	 CTRUE	indicate that Interrupt Enable. 
 *						 CFALSE	indicate that Interrupt Disable.
 *	@return		None.
 *	@see also	Interrupt mask set or clear register, SSPIMSC
 */

//------------------------------------------------------------------------------
void	NX_SSP_SetInterruptEnable		( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			old_irq_mask, new_irq_mask;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER  > IntNum );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_irq_mask = ReadIO32(&pRegister->SSPIMSC);

	if( CTRUE == Enable )
	{
		new_irq_mask = ( 1 << IntNum );
		new_irq_mask = old_irq_mask | new_irq_mask;
	}
	else
	{
		new_irq_mask = ~( 1 << IntNum );
		new_irq_mask = old_irq_mask & new_irq_mask;
	}

	WriteIO32(&pRegister->SSPIMSC, new_irq_mask);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum			Interrupt Index Number .
 *				[0] : RORIM,  [1] : RTIM,  [2] : RXIM,  [3] : TXIM
 *	@return		CTRUE	indicates an interrupt specified by @a IntNum is enabled.
 *				CFALSE	indicates an interrupt specified by @a IntNum is disabled.
 *	@see also	Interrupt mask set or clear register, SSPIMSC
 */
CBOOL	NX_SSP_GetInterruptEnable		( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			old_irq_mask;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER > IntNum );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_irq_mask = ReadIO32(&pRegister->SSPIMSC);

	if( 0 != (( 1 << IntNum ) & old_irq_mask) )
	{
		return CFALSE;
	}
	else
	{
		return CTRUE;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	ChannelIndex	Interrupt Index Number .
 *				[0] : RORIM,  [1] : RTIM,  [2] : RXIM,  [3] : TXIM
 *	@return		CTRUE	indicates an interrupt specified by @a IntNum is pended.
 *				CFALSE	indicates an interrupt specified by @a IntNum is not pended.
 *	@remark		The interrupt pending status are logged regardless of interrupt
 *				enable status. Therefore the return value can be CTRUE even
 *				though the specified interrupt has been disabled.
 */
CBOOL	NX_SSP_GetInterruptPending		( U32 ModuleIndex , U32 IntNum )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			masked_irq_status;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER > IntNum );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	masked_irq_status = ReadIO32(&pRegister->SSPMIS);

	if( 0 != ( ( 1 << IntNum ) & masked_irq_status) )
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
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	ChannelIndex	Interrupt Index Number .
 *				[0] : RORIM,  [1] : RTIM,  [2] : RXIM,  [3] : TXIM
 *				only [0] and [1] can be pended..
 *	@return		None.
 *	@see also	Interrupt clear register, SSPICR
 */
void	NX_SSP_ClearInterruptPending	( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_pending = ( 1 << IntNum );


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_INTR_MAXNUMBER > IntNum );
	NX_ASSERT( (SSPRORINTR == IntNum) || (SSPRTINTR == IntNum) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->SSPICR, irq_pending);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bEnb			Set it as CTRUE to enable all interrupts. 
 *								Set it as CFALSE to disable all interrupts.
 *	@return		None.
 */
void	NX_SSP_SetInterruptEnableAll	( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			new_irq_mask;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	if( CTRUE == Enable )
	{
		new_irq_mask = (0x0f); // All
	}
	else
	{
		new_irq_mask = (0x00);
	}

	WriteIO32(&pRegister->SSPIMSC, new_irq_mask);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE	indicates there's interrupts which are enabled.
 *				CFALSE	indicates there's no interrupt which are enabled.
 */
CBOOL	NX_SSP_GetInterruptEnableAll	( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			old_irq_mask;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_irq_mask = ReadIO32(&pRegister->SSPIMSC);

	if( 0x0f == ((0x0f) & old_irq_mask) )
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
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE	indicates there's interrupts which are pended.
 *				CFALSE	indicates there's no interrupt which are pended.
 */
CBOOL	NX_SSP_GetInterruptPendingAll	( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			masked_irq_status;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	masked_irq_status = ReadIO32(&pRegister->SSPMIS);

	if( 0 != ( 0x0f & masked_irq_status) )
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
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 */
void	NX_SSP_ClearInterruptPendingAll	( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			irq_pending = (0x01 | 0x02);


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->SSPICR, ~irq_pending);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		an interrupt number which has the most priority of pended interrupts. 
 *				This value is one of @ref NX_SSP_INT enum.
 *				If there's no interrupt which is pended and unmasked, it returns -1.
 */
U32		NX_SSP_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32			masked_irq_status;


	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	if( CFALSE == NX_SSP_GetInterruptPendingAll( ModuleIndex ) )
	{
		return -1;
	}
	else
	{
		masked_irq_status = ReadIO32(&pRegister->SSPMIS);

		if ( 0 != ((1<<SSPRORINTR) & masked_irq_status))
		{
			return	SSPRORINTR;
		}
		else if ( 0 != ((1<<SSPRTINTR) & masked_irq_status))
		{
			return	SSPRTINTR;
		}
		else if ( 0 != ((1<<SSPRXINTR) & masked_irq_status))
		{
			return	SSPRXINTR;
		}
		else if ( 0 != ((1<<SSPTXINTR) & masked_irq_status))
		{
			return	SSPTXINTR;
		}
		else
		{
			// Never Get Here
			NX_ASSERT( CFALSE );
			return -1;
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a priority control.
 *	@param[in]	ArbiterNum	An arbiter number to be changed, 0 ~ 11 are valid.
 *	@param[in]	OrderSel	Determines the order of priority manually.
 *	@return		None.
 *	@remark		Arbiter0	: for interrupt	0 ~	5 
 *				Arbiter1	: for interrupt	6 ~ 11 
 *				Arbiter2	: for interrupt 12 ~ 17 
 *				Arbiter3	: for interrupt 18 ~ 23 
 *				Arbiter4	: for interrupt 24 ~ 29 
 *				Arbiter5	: for interrupt 30 ~ 35 
 *				Arbiter6	: for interrupt 36 ~ 41 
 *				Arbiter7	: for interrupt 42 ~ 47 
 *				Arbiter8	: for interrupt 48 ~ 53 
 *				Arbiter9	: for interrupt 54 ~ 59 
 *				Arbiter10 : for interrupt 60 ~ 63 
 *				Arbiter11 : arbiter 0 ~ 10
 */
void	NX_SSP_SetPriorityMode( U32 ArbiterNum, U32 OrderSel )
{
    NX_ASSERT( CFALSE ); // TODO
    ArbiterNum = ArbiterNum;
    OrderSel = OrderSel;
	return;
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index for SSP/SPI Tx.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		DMA peripheral index for SSP/SPI Tx.
 *	@see		NX_SSP_GetDMAIndex_Rx, NX_SSP_GetDMABusWidth
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
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		DMA peripheral index for SSP/SPI Rx.
 *	@see		NX_SSP_GetDMAIndex_Tx, NX_SSP_GetDMABusWidth
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
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		DMA bus width of SSP/SPI..
 *				It returns 8 in case of the data bit width is between 1 and 8,
 *				16 in case of the data bit width is between 9 and 16..
 *	@remark		You have to call NX_SSP_SetBitWidth() to set the data bit
 *				width properly before using	this function.
 *	@see		NX_SSP_GetDMAIndex_Tx, NX_SSP_GetDMAIndex_Rx
 *	@see also	Control register 0, SSPCR0, DSS
 */
U32		NX_SSP_GetDMABusWidth( U32 ModuleIndex )
{
	U32	DSS;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT(NX_SSP_GetBitWidth(ModuleIndex)<=16);


	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	DSS = (U32)(ReadIO32(&pRegister->SSPCR0) & 0x0f);
	// 0111 : 8-bit data -> 0x07
	if( DSS <= 0x07 )	{ return 8; 	}
	else				{ return 16;	}
}

//------------------------------------------------------------------------------
///	@name	Clock Control Interface
//------------------------------------------------------------------------------
/**	@name		NX_SSP_SetClockPrescaler
 *	@brief		Set Serial Clock Prescaler. CLOCK = Fsspclk / ( CPSDVR * ( 1+SCR) )
 *	@param[in]	ModuleIndex		an index of module
 	@param[in]	CPSDVR			Clock prescale register ( 2~254, only even )
 	@param[in]	SCR				Serial Clock rate, ( 0~255 )
 	@see also	Control register 0, SSPCR0
 				Clock prescale register, SSPCPSR
 */
void		NX_SSP_SetClockPrescaler( U32 ModuleIndex, U32 CPSDVR, U32 SCR )
{
	U32		new_sspcr0;
	U32		old_sspcr0;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (CPSDVR % 2 == 0) && (CPSDVR <= 254 && CPSDVR >= 2))
//	NX_ASSERT( SCR <= 255 && SCR >= 0 )
   	NX_ASSERT( SCR <= 255 )

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	new_sspcr0 = old_sspcr0 & 0xFFFF00FF;
	new_sspcr0 = new_sspcr0 | (SCR << 8);

	WriteIO32(&pRegister->SSPCR0, new_sspcr0);
	WriteIO32(&pRegister->SSPCPSR, CPSDVR);
}
//------------------------------------------------------------------------------
/**	@name		NX_SSP_GetClockPrescaler
 *	@brief		Get Serial Clock Prescaler.
 	@param[in]	ModuleIndex		an index of module
 	@return		( CPSDVR * ( 1+SCR) )
 	@see also	Control register 0, SSPCR0
 				Clock prescale register, SSPCPSR
**/
U32			NX_SSP_GetClockPrescaler( U32 ModuleIndex )
{
	U32		old_cpsdvr;
	U32		old_scr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );


	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_scr = (ReadIO32(&pRegister->SSPCR0) & 0x0000FF00);
	old_scr = old_scr >> 8;

	old_cpsdvr = ReadIO32(&pRegister->SSPCPSR) & 0xFF;

	return ( old_cpsdvr * ( 1 + old_scr ) );
}


//--------------------------------------------------------------------------
/// @name Configuration Function
//------------------------------------------------------------------------------
/**	@name		NX_SSP_SetAllDMAMode
 *	@brief		Set Transmit DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 CTRUE indicate that Transmit/Receive DMA Enable. 
 *						 CFALSE indicate that Transmit/Receive DMA Disable. .
 *	@return		None.
 *	@see
**/
void	NX_SSP_SetDMATransferMode( U32 ModuleIndex, CBOOL Enable )
{
	U32 old_sspdmacr;
	U32 new_sspdmacr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == Enable || CFALSE == Enable);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspdmacr = ReadIO32(&pRegister->SSPDMACR);
	if( CTRUE == Enable )
	{
		new_sspdmacr = old_sspdmacr | (0x03);
	}
	else
	{
		new_sspdmacr = old_sspdmacr & (~0x03);
	}
	WriteIO32(&pRegister->SSPDMACR, new_sspdmacr);
}

//------------------------------------------------------------------------------
/**	@name		NX_SSP_GetDMATransferMode
 *	@brief		Get All DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE  indicate that All DMA Enabled
 				CFALSE indicate that All DMA Disabled
 *	@see
**/
CBOOL	NX_SSP_GetDMATransferMode( U32 ModuleIndex )
{
	U32 old_sspdmacr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspdmacr = ReadIO32(&pRegister->SSPDMACR);
	if( 0 != ( 0x03 & old_sspdmacr ) )
	{
		return CTRUE;
	}
	else
	{
		return CFALSE;
	}
}


/**	@name		NX_SSP_SetTransmitDMAMode
 *	@brief		Set Transmit DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 CTRUE indicate that Transmit DMA Enable. 
 *						 CFALSE indicate that Transmit DMA Disable. .
 *	@return		None.
 *	@see
**/
void	NX_SSP_SetDMATransmitMode( U32 ModuleIndex, CBOOL Enable )
{
	U32 old_sspdmacr;
	U32 new_sspdmacr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == Enable || CFALSE == Enable);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspdmacr = ReadIO32(&pRegister->SSPDMACR);
	if( CTRUE == Enable )
	{
		new_sspdmacr = old_sspdmacr | (0x02);
	}
	else
	{
		new_sspdmacr = old_sspdmacr & (~0x02);
	}
	WriteIO32(&pRegister->SSPDMACR, new_sspdmacr);
}

//------------------------------------------------------------------------------
/**	@name		NX_SSP_GetTransmitDMAMode
 *	@brief		Get Transmit DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE  indicate that Transmit DMA Enabled
 				CFALSE indicate that Transmit DMA Disabled
 *	@see
**/
CBOOL	NX_SSP_GetDMATransmitMode( U32 ModuleIndex )
{
	U32 old_sspdmacr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspdmacr = ReadIO32(&pRegister->SSPDMACR);
	if( 0 != ( 0x02 & old_sspdmacr ) )
	{
		return CTRUE;
	}
	else
	{
		return CFALSE;
	}
}

//------------------------------------------------------------------------------
/**	@name		NX_SSP_SetReceiveDMAMode
 *	@brief		Set Receive DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 CTRUE  indicate that Receive DMA Enable. 
 *						 CFALSE indicate that Receive DMA Disable. .
 *	@return		None.
 *	@see
**/
void	NX_SSP_SetDMAReceiveMode( U32 ModuleIndex, CBOOL Enable )
{
	U32 old_sspdmacr;
	U32 new_sspdmacr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == Enable || CFALSE == Enable);

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspdmacr = ReadIO32(&pRegister->SSPDMACR);
	if( CTRUE == Enable )
	{
		new_sspdmacr = old_sspdmacr | (0x01);
	}
	else
	{
		new_sspdmacr = old_sspdmacr & (~0x01);
	}
	WriteIO32(&pRegister->SSPDMACR, new_sspdmacr);
}

//------------------------------------------------------------------------------
/**	@name		NX_SSP_GetReceiveDMAMode
 *	@brief		Get Receive DMA Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTURE  indicate that Receive DMA Enabled
 				CFALSE indicate that Receive DMA Disabled
 *	@see
**/
CBOOL	NX_SSP_GetDMAReceiveMode( U32 ModuleIndex )
{
	U32 old_sspdmacr;
	register struct NX_SSP_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspdmacr = ReadIO32(&pRegister->SSPDMACR);
	if( 0 != ( 0x01 & old_sspdmacr ) )
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
 *	@brief		Set Data bit width
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bitWidth	Value of Data Bit Width( 4 ~ 16 )
 *	@return		None.
 *	@remarks	NX_SSP_SetBitWidth() must invoke before NX_SSP_GetDMABusWidth()
 *	@code
 *				NX_SSP_SetBitWidth();
 *				NX_SSP_GetDMABusWidth();
 *	@endcode
 *	@see
 */
void	NX_SSP_SetBitWidth( U32 ModuleIndex, U32 bitWidth )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	U32		new_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( bitWidth <= 16 && bitWidth >= 4);
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	new_sspcr0 = old_sspcr0 & (~0x0F);
	new_sspcr0 = new_sspcr0 | (bitWidth-1);

	WriteIO32(&pRegister->SSPCR0, new_sspcr0);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Data Bit Width
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Data Bit Width( 4 ~ 16 )
 *	@see
 */
U32		NX_SSP_GetBitWidth( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);

	return (U32)(old_sspcr0 & 0x0F);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Operation Mode ( Master or Slave )
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bSlave	 CTRIE indicate that Slave mode enable. 
 *						 CFALSE indicate that Master mode enable.
 *	@return		None.
 *	@see		Control register 1, SSPCR1
 */
void	NX_SSP_SetSlaveMode( U32 ModuleIndex, NX_SSP_MODE bSlave )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;
	U32		new_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == bSlave || CFALSE == bSlave);
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);
	if( CTRUE == bSlave )
	{
		new_sspcr1 = old_sspcr1 | (0x04); // SLAVE
	}
	else
	{
		new_sspcr1 = old_sspcr1 & (~0x04); // MASTER
	}

	WriteIO32(&pRegister->SSPCR1, new_sspcr1);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Operation Mode ( Master or Slave )
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRIE indicate that Slave mode enable. 
 *				 CFALSE indicate that Master mode enable.
 *	@see		Control register 1, SSPCR1
 */
NX_SSP_MODE	NX_SSP_GetSlaveMode( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);
	if( 0 != (0x04 & old_sspcr1) )
	{
		return NX_SSP_MODE_SLAVE;
	}
	else
	{
		return NX_SSP_MODE_MASTER;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Polarity of SSPCLK
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bInvert	 CTRUE indicate that SSPCLK Invert enable. 
 *						 CFALSE indicate that SSPCLK Invert disable. 
 *	@return		None.
 *	@see		Control register 0, SSPCR0
 *				This Function can work Motorola SPI frame format only. otherwise,
 *				ASSERT ERROR will be called
 */
void	NX_SSP_SetClockPolarityInvert( U32 ModuleIndex, CBOOL bInvert )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	U32		new_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == bInvert || CFALSE == bInvert);
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	NX_ASSERT( 0x0000 == (old_sspcr0 & 0x0030)); // only for Motorola SPI frame format

	if( CTRUE == bInvert )
	{
		new_sspcr0 = old_sspcr0 | (0x0040);
	}
	else
	{
		new_sspcr0 = old_sspcr0 & (~0x0040);
	}

	WriteIO32(&pRegister->SSPCR0, new_sspcr0);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of SSPCLK's polarity is invert or normal.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicate that SSPCLK's polarity is invert. 
 *				 CFALSE	indicate that SSPCLK's polarity is normal.
 *	@see		Control register 0, SSPCR0
 *				This Function can work Motorola SPI frame format only. otherwise,
 *				ASSERT ERROR will be called
 */
CBOOL	NX_SSP_GetClockPolarityInvert( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	NX_ASSERT( 0x0000 == (old_sspcr0 & 0x0030)); // only for Motorola SPI frame format

	if( 0 != (0x0040 &  old_sspcr0) )
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
 *	@brief		Set status of Slave Output Enable/Disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable			Enable/Disable
 */
void	NX_SSP_SetSlaveOutputEnable( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;
	U32		new_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == Enable || CFALSE == Enable);
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);

	if( CFALSE == Enable )
	{
		new_sspcr1 = old_sspcr1 | (0x0008);
	}
	else
	{
		new_sspcr1 = old_sspcr1 & (~0x0008);
	}

	WriteIO32(&pRegister->SSPCR1, new_sspcr1);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of Slave Output Enable/Disable
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Enable/Disable Status
 *	@see		Control register 0, SSPCR1
 */
CBOOL	NX_SSP_GetSlaveOutputEnable( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;
	//U32		new_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);

	if( 0 == (old_sspcr1 & 0x04) )
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
 *	@brief		Set SPI format
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	format		SPI format ( NX_SSP_FORMAT_A[0] or NX_SSP_FORMAT_B[1] )
 *	@return		None.
 */
void	NX_SSP_SetSPIFormat( U32 ModuleIndex, NX_SSP_FORMAT format)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	U32		new_sspcr0 = 0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( NX_SSP_FORMAT_D >= format );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	NX_ASSERT( 0x0000 == (old_sspcr0 & 0x0030)); // only for Motorola SPI frame format

	new_sspcr0 = old_sspcr0 | (format  << 6);

	WriteIO32(&pRegister->SSPCR0, new_sspcr0);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get SPI format
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		SPI format ( NX_SSP_FORMAT_A or NX_SSP_FORMAT_B )
 */
NX_SSP_FORMAT	NX_SSP_GetSPIFormat( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	NX_ASSERT( 0x0000 == (old_sspcr0 & 0x0030)); // only for Motorola SPI frame format

	old_sspcr0 =  (0x00C0 &  old_sspcr0);
	old_sspcr0 = old_sspcr0 >> 7;

	if( NX_SSP_FORMAT_A == old_sspcr0 )
	{
		return NX_SSP_FORMAT_A;
	}
	else if( NX_SSP_FORMAT_B == old_sspcr0 )
	{
		return NX_SSP_FORMAT_B;
	}
	else
	{
		NX_ASSERT( CFALSE ); // "Never Get Here" );
	}
	return 0;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set SPI Clock Phase Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Phase			1 or 0
 				SPH = 1 or 0
 				SPH = 0 : LF2000의 Format A
 				SPH = 1 : LF2000의 Format B
 *	@return		None.
 *	@see		SSPCR0, 2-12
 */
void			NX_SSP_SetClockPhase( U32 ModuleIndex, CBOOL	Phase )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	U32		new_sspcr0 = 0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (1 == Phase) || (0 == Phase) );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	NX_ASSERT( 0x0000 == (old_sspcr0 & 0x0030)); // only for Motorola SPI frame format

	if(1 == Phase)
	{
		new_sspcr0 = old_sspcr0 | (0x0080);
	}
	else if(0 == Phase)
	{
		new_sspcr0 = old_sspcr0 & (~0x0080);
	}
	else
	{
		NX_ASSERT( CFALSE ); //"Never Get Here" );
	}

	WriteIO32(&pRegister->SSPCR0, new_sspcr0);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get SPI Clock Phase Mode
 *	@param[in]	ModuleIndex		an index of module.

 *	@return		SPH = 1 or 0
 				SPH = 0 : LF2000의 Format A
 				SPH = 1 : LF2000의 Format B
 *	@see		SSPCR0, 2-12
 */
U32				NX_SSP_GetClockPhase( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	NX_ASSERT( 0x0000 == (old_sspcr0 & 0x0030)); // only for Motorola SPI frame format

	old_sspcr0 =  (0x0080 &  old_sspcr0);
	old_sspcr0 = old_sspcr0 >> 7;

	if( 0 == old_sspcr0 )
	{
		return 0;
	}
	else if( 1 == old_sspcr0 )
	{
		return 1;
	}
	else
	{
		NX_ASSERT( CFALSE ); // "Never Get Here" );
	}
	return 0;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI/NM Mode
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	protocol		Mode
 				NX_SSP_PROTOCOL_SPI = 0x00, TI SPI frame format*
 				NX_SSP_PROTOCOL_SSP = 0x01, Motorola SSP frame format
 				NX_SSP_PROTOCOL_NM  = 0x02, National Microwire frame format
 				( NM은 지원은 한다고 하는데 테스트는 어떻게 해야할지.. )
 *	@return		None.
 *	@see		SSPCR0
 */
void	NX_SSP_SetProtocol( U32 ModuleIndex, NX_SSP_PROTOCOL protocol )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;
	U32		new_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( (NX_SSP_PROTOCOL_SSP == protocol) || (NX_SSP_PROTOCOL_SPI == protocol) || (NX_SSP_PROTOCOL_NM == protocol) );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0);
	new_sspcr0 = old_sspcr0 & (~0x0030);

	new_sspcr0 = new_sspcr0 | (protocol<<4);

	WriteIO32(&pRegister->SSPCR0, new_sspcr0);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get SSP/SPI Mode
 *	@param[in]	ModuleIndex		an index of module.
 	@return		protocol		Mode
 *				NX_SSP_PROTOCOL_SSP = 0x00, Motorola SSP frame format
 				NX_SSP_PROTOCOL_SPI = 0x01, TI SPI frame format
 				NX_SSP_PROTOCOL_NM  = 0x02, National Microwire frame format
 				( NM은 지원은 한다고 하는데 테스트는 어떻게 해야할지.. )
 *	@see		SSPCR0
 */
NX_SSP_PROTOCOL	NX_SSP_GetProtocol( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr0;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr0 = ReadIO32(&pRegister->SSPCR0) & (0x0030);;
	old_sspcr0 = old_sspcr0 >> 4;

	return (NX_SSP_PROTOCOL) old_sspcr0;
}


//--------------------------------------------------------------------------
/// @name Operation Function
//--------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Set SSP/SPI enable or disable.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	bEnable		 CTRUE indicate that SSP/SPI Enable. 
 *							 CFALSE indicate that SSP/SPI Disable.
 *	@return		None.
 *	@see		SSPCR1
 */
void	NX_SSP_SetEnable( U32 ModuleIndex, CBOOL bEnable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;
	U32		new_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == bEnable || CFALSE == bEnable );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);

	if( CTRUE == bEnable )
	{
		new_sspcr1 = old_sspcr1 | 0x02;
	}
	else
	{
		new_sspcr1 = old_sspcr1 & (~0x02);
	}

	WriteIO32(&pRegister->SSPCR1, new_sspcr1);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get status of SSP/SPI is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicate that SSP/SPI is enabled. 
 *				 CFALSE	indicate that SSP/SPI is disabled.
 *	@see		SSPCR1
 */
CBOOL	NX_SSP_GetEnable( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);

	if( 0 != (0x02 & old_sspcr1) )
	{
		return CTRUE;
	}
	else
	{
		return CFALSE;
	}
}

void NX_SSP_SetLoopBackMode( U32 ModuleIndex, CBOOL bEnable )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;
	U32		new_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	NX_ASSERT( CTRUE == bEnable || CFALSE == bEnable );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);

	if( CTRUE == bEnable )
	{
		new_sspcr1 = old_sspcr1 | 0x01;
	}
	else
	{
		new_sspcr1 = old_sspcr1 & (~0x01);
	}

	WriteIO32(&pRegister->SSPCR1, new_sspcr1);
}

CBOOL NX_SSP_GetLoopBackMode( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32		old_sspcr1;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspcr1 = ReadIO32(&pRegister->SSPCR1);

	if( 0 != (0x01 & old_sspcr1) )
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
 *	@brief		Read Byte Data.
 *				pl02212는 4~16bit가 가능한데.. 무조건 8 bit를 읽어오는것이 정당할지..
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

	return (U8)((ReadIO32(&pRegister->SSPDR)) & 0xff);
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

	return (U16)((ReadIO32(&pRegister->SSPDR)) & 0xffff);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Send Byte.
 *				pl02212는 4~16bit가 가능한데.. 무조건 8 bit를 읽어오는것이 정당할지..
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


	WriteIO32(&pRegister->SSPDR, (U8)ByteData);	//put the byte data
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

	WriteIO32(&pRegister->SSPDR, (U16)HalfWordData);	//put the byte data
}


//--------------------------------------------------------------------------
/// @name FIFO State Check Function
/// @see also	Status Register, SSPSR
//--------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Check Tx FIFO is Empty or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE Indicate that FIFO Empty.
 *				 CFALSE Indicate that FIFO NOT Empty
 */
CBOOL	NX_SSP_IsTxFIFOEmpty(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	old_sspsr;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspsr = ReadIO32(&pRegister->SSPSR);

	if( 0 == (old_sspsr & 0x01) )
	{
		return CFALSE;
	}
	else
	{
		return CTRUE;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check Tx FIFO is Full or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	Indicate that FIFO Full.
 *				 CFALSE	Indicate that FIFO NOT Full.
 */
CBOOL	NX_SSP_IsTxFIFOFull(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	old_sspsr;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspsr = ReadIO32(&pRegister->SSPSR);

	if( 0 == (old_sspsr & 0x02) )
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
 *	@return		 CTRUE	Indicate that FIFO Empty.
 *				 CFALSE	Indicate that FIFO NOT Empty.
 */
CBOOL	NX_SSP_IsRxFIFOEmpty(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	old_sspsr;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspsr = ReadIO32(&pRegister->SSPSR);

	if( 0 == (old_sspsr & 0x04) )
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
 *	@return		 CTRUE	Indicate that FIFO Full.
 *				 CFALSE	Indicate that FIFO NOT Full.
 */
CBOOL	NX_SSP_IsRxFIFOFull(U32 ModuleIndex)
{
	register struct NX_SSP_RegisterSet *pRegister;
	U32	old_sspsr;

	NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	old_sspsr = ReadIO32(&pRegister->SSPSR);

	if( 0 == (old_sspsr & 0x08) )
	{
		return CFALSE;
	}
	else
	{
		return CTRUE;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check Tx/Rx is End or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	Indicate that Tx/Rx is End.
 *				 CFALSE	Indicate that Tx/Rx is NOT End.
 *	@remarks	This function is same to NX_SSP_GetInterruptPending(2)
 */
CBOOL	NX_SSP_IsTxRxEnd( U32 ModuleIndex )
{
	return NX_SSP_CheckBusy(ModuleIndex);
}

//------------------------------------------------------------------------------
///	@name	Register Backup & Store Function [주의] 내부에서 ASSERT 제공 안함..

void	NX_SSP_Set_SSPCR0				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPCR0, (U32)RegValue);
}

U32		NX_SSP_Get_SSPCR0				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCR0));
}

void	NX_SSP_Set_SSPCR1				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPCR1, (U32)RegValue);
}

U32		NX_SSP_Get_SSPCR1				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCR1));
}

void	NX_SSP_Set_SSPDR				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPDR, (U32)RegValue);
}

U32		NX_SSP_Get_SSPDR				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPDR));
}


U32		NX_SSP_Get_SSPSR				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPSR));
}

void	NX_SSP_Set_SSPCPSR				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPCPSR, (U32)RegValue);
}

U32		NX_SSP_Get_SSPCPSR				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCPSR));
}

void	NX_SSP_Set_SSPIMSC				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPIMSC, (U32)RegValue);
}

U32		NX_SSP_Get_SSPIMSC				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPIMSC));
}


U32		NX_SSP_Get_SSPRIS				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPRIS));
}


U32		NX_SSP_Get_SSPMIS				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPMIS));
}

void	NX_SSP_Set_SSPICR				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPICR, (U32)RegValue);
}


void	NX_SSP_Set_SSPDMACR				( U32 ModuleIndex, U32 RegValue )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	WriteIO32(&pRegister->SSPDMACR, (U32)RegValue);
}

U32		NX_SSP_Get_SSPDMACR				( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPDMACR));
}


U32		NX_SSP_Get_SSPPeriphID0			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPPeriphID0));
}


U32		NX_SSP_Get_SSPPeriphID1			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPPeriphID1));
}


U32		NX_SSP_Get_SSPPeriphID2			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPPeriphID2));
}


U32		NX_SSP_Get_SSPPeriphID3			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPPeriphID3));
}


U32		NX_SSP_Get_SSPCellID0			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCellID0));
}


U32		NX_SSP_Get_SSPCellID1			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCellID1));
}


U32		NX_SSP_Get_SSPCellID2			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCellID2));
}


U32		NX_SSP_Get_SSPCellID3			( U32 ModuleIndex )
{
	register struct NX_SSP_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	return (U32)(ReadIO32(&pRegister->SSPCellID3));
}


//---------- CLKGEN 을 위한 prototype
U32 NX_SSP_GetClockNumber (U32 ModuleIndex)
{
    static const U32 CLKGEN_SSPLIST[] = { CLOCKINDEX_LIST( SSP ) };
    //NX_ASSERT( NUMBER_OF_SSP_MODULE > (U32)CLKGEN_SSPLIST[ModuleIndex] );
    NX_ASSERT( NUMBER_OF_SSP_MODULE > ModuleIndex );
    return (U32)CLKGEN_SSPLIST[ModuleIndex];
}

//---------- RSTCON 을 위한 prototype
U32	NX_SSP_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
    const U32 ResetPinNumber[2][NUMBER_OF_SSP_MODULE] =
    {
        { RESETINDEX_LIST( SSP, PRESETn )},
        { RESETINDEX_LIST( SSP, nSSPRST )},
    };

    return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}




//------------------------------------------------------------------------------

