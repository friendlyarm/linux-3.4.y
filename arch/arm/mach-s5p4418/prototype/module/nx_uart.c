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
//	Module		: pl01115 Uart
//	File		: nx_uart.c
//	Description	:
//	Author		:
//	History		: 2014
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_uart.h"

static	struct
{
	struct NX_UART_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_UART_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicates that Initialize is successed.
 *			 CFALSE indicates that Initialize is failed.
 *	@see	NX_UART_GetNumberOfModule
 */
CBOOL	NX_UART_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_UART_MODULE; i++ )
		{
			__g_ModuleVariables[i].pRegister = CNULL;
		}

		bInit = CTRUE;
	}

	return bInit;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_UART_Initialize
 */
U32		NX_UART_GetNumberOfModule( void )
{
	return NUMBER_OF_UART_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
#ifdef	NUMBER_OF_UART_MODULE
U32		NX_UART_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = {
//		PHY_BASEADDR_LIST( UART )
		PHY_BASEADDR_UART0_MODULE,
		PHY_BASEADDR_pl01115_Uart_modem_MODULE,
		PHY_BASEADDR_UART1_MODULE,
		PHY_BASEADDR_LIST(pl01115_Uart_nodma)
		}; // PHY_BASEADDR_UART?_MODULE

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}
#endif

#ifdef	NUMBER_OF_pl01115_Uart_modem_MODULE
//U32		NX_UART_GetPhysicalAddressModem( U32 ModuleIndex )
//{
//	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( pl01115_Uart_modem ) }; // PHY_BASEADDR_pl01115_Uart_modem_MODULE
//
//	NX_ASSERT( NUMBER_OF_pl01115_Uart_modem_MODULE > ModuleIndex );
//
//	return (U32)PhysicalAddr[ModuleIndex];
//}
#endif

#ifdef	NUMBER_OF_pl01115_Uart_nodma_MODULE
//U32		NX_UART_GetPhysicalAddressNodma( U32 ModuleIndex )
//{
//	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( pl01115_Uart_nodma ) }; // PHY_BASEADDR_pl01115_Uart_nodma_MODULE
//
//	NX_ASSERT( NUMBER_OF_pl01115_Uart_nodma_MODULE > ModuleIndex );
//
//	return (U32)PhysicalAddr[ModuleIndex];
//}
#endif

#ifdef	NUMBER_OF_pl01115_Uart_modemnodma_MODULE
//U32		NX_UART_GetPhysicalAddressModemNodma( U32 ModuleIndex )
//{
//	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( pl01115_Uart_modemnodma ) }; // PHY_BASEADDR_pl01115_Uart_modemnodma_MODULE
//
//	NX_ASSERT( NUMBER_OF_pl01115_Uart_modemnodma_MODULE > ModuleIndex );
//
//	return (U32)PhysicalAddr[ModuleIndex];
//}
#endif
/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
//U32 NX_UART_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
//{
//	const U32 ResetPinNumber[2][NUMBER_OF_UART_MODULE] =
//	{
//		{ RESETINDEX_LIST( UART, nUARTRST )}
//	};
//
//	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
//}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_UART_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_UART_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_UART_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_UART_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_UART_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 */
void*	NX_UART_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Initialize is successed. 
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_UART_OpenModule( U32 ModuleIndex )
{
	register struct NX_UART_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;


	WriteIO32(&pRegister->RSR_ECR, 0);	// status clear
	WriteIO32(&pRegister->LCR_H, 0x60);	// 8 bits, fifo disable, one stop, no parity, break disable
	WriteIO32(&pRegister->CR, 0);		// no AFC, RX TX disable, Loopback disable, SIR disable, UART disable
	WriteIO32(&pRegister->DMACR, 0);	// tx rx DMA disable
	WriteIO32(&pRegister->IMSC, 0);	// all interrupt disable

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Deinitialize is successed. 
 *				 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_UART_CloseModule( U32 ModuleIndex )
{
	register struct NX_UART_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->RSR_ECR, 0);	// status clear
	WriteIO32(&pRegister->LCR_H, 0x60);	// 8 bits, fifo disable, one stop, no parity, break disable
	WriteIO32(&pRegister->CR, 0);		// no AFC, RX TX disable, Loopback disable, SIR disable, UART disable
	WriteIO32(&pRegister->DMACR, 0);	// tx rx DMA disable
	WriteIO32(&pRegister->IMSC, 0);	// all interrupt disable
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Module is Busy. 
 *				 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_UART_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Ready to enter power-down stage. 
 *				 CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_UART_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_UART_MODULE in <nx_chip.h>.
 */
U32 NX_UART_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
//		CLOCKINDEX_LIST( UART )
		CLOCKINDEX_OF_UART0_MODULE,
		CLOCKINDEX_OF_pl01115_Uart_modem_MODULE,
		CLOCKINDEX_OF_UART1_MODULE,
		CLOCKINDEX_LIST(pl01115_Uart_nodma)
	};
	NX_CASSERT( NUMBER_OF_UART_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_UART_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_UART_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
//		RESETINDEX_LIST( UART, nUARTRST )
		RESETINDEX_OF_UART0_MODULE_nUARTRST,
		RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST,
		RESETINDEX_OF_UART1_MODULE_nUARTRST,
		RESETINDEX_LIST( pl01115_Uart_nodma, nUARTRST )
	};
	NX_CASSERT( NUMBER_OF_UART_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 */
U32		NX_UART_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	UartInterruptNumber[NUMBER_OF_UART_MODULE] =
				{
//					INTNUM_LIST(UART)
					INTNUM_OF_UART0_MODULE,
					INTNUM_OF_pl01115_Uart_modem_MODULE,
					INTNUM_OF_UART1_MODULE,
					INTNUM_LIST(pl01115_Uart_nodma)
				};

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return	UartInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number ( 0:RI, 1:CTS, 2:DCD, 3:DSR, 4:RX, 5:TX, 6:RT, 7:FE, 8:PE, 9:BE, 10:OE ).
 *	@param[in]	Enable	 CTRUE	indicates that Interrupt Enable. 
 *						 CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_UART_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x7FF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( 11 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIO32(&pRegister->IMSC) & PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum)));
	ReadValue	|=	(U32)Enable << (IntNum) ;

	WriteIO32(&pRegister->IMSC, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number ( 0:RI, 1:CTS, 2:DCD, 3:DSR, 4:RX, 5:TX, 6:RT, 7:FE, 8:PE, 9:BE, 10:OE ).
 *	@return		 CTRUE	indicates that Interrupt is enabled. 
 *				 CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_UART_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( 11 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC) >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							- EnableFlag[0] : Set TX interrupt enable or disable. 
 *							- EnableFlag[1] : Set RX interrupt enable or disable. 
 *							- EnableFlag[2] : Set Error interrupt enable or disable. 
 *							- EnableFlag[3] : Set Modem interrupt enable or disable. 
 *	@return		None.
 */
void	NX_UART_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag )
{
	const U32	ENB_POS = 0;

	NX_ASSERT( 0x800 > EnableFlag );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	// @modified charles 110630 의도가 맞는지 확인해볼 것
	//WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTCON, (U16)(EnableFlag << ENB_POS));
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC, (U16)(EnableFlag << ENB_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Current setting value of interrupt. 
 *				"1" means interrupt is enabled. 
 *				"0" means interrupt is disabled. 
 *				- Return Value[0] : TX interrupt's setting value. 
 *				- Return Value[1] : RX interrupt's setting value. 
 *				- Return Value[2] : Error interrupt's setting value. 
 *				- Return Value[3] : Modem interrupt's setting value. 
 */
U32		NX_UART_GetInterruptEnable32( U32 ModuleIndex )
{
	const U32	ENB_POS		= 0;
	const U32	ENB_MASK	= 0x7FF << ENB_POS;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)((ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC) & ENB_MASK) >> ENB_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number( 0:RI, 1:CTS, 2:DCD, 3:DSR, 4:RX, 5:TX, 6:RT, 7:FE, 8:PE, 9:BE, 10:OE ).
 *	@return		 CTRUE	indicates that Pending is seted. 
 *				 CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_UART_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( 11 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MIS) >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Current setting value of pending bit. 
 *				"1" means pend bit is occured. 
 *				"0" means pend bitis NOT occured. 
 *				- Return Value[0] : TX pending state. 
 *				- Return Value[1] : RX pending state. 
 *				- Return Value[2] : Error pending state. 
 *				- Return Value[3] : Modem pending state. r
 */
U32		NX_UART_GetInterruptPending32( U32 ModuleIndex )
{
	const U32 PEND_MASK = 0x7FF;

	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MIS) & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number( 0:Tx, 1:Rx, 2:Error, 3:Modem ).
 *	@return		None.
 */
void	NX_UART_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x7FF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( 11 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->ICR, (1 << IntNum) & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	 
 *								- PendingFlag[0] : TX pending bit. "
 *								- PendingFlag[1] : RX pending bit. "
 *								- PendingFlag[2] : Error pending bit. "
 *								- PendingFlag[3] : Modem pending bit. "
 *	@return		None.
 */
void	NX_UART_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x7FF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( 0x800 > PendingFlag );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->ICR, PendingFlag & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	 CTRUE	indicates that Set to all interrupt enable. 
 *						 CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_UART_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	INT_MASK	=	0x7FF;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK;
	}

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				 CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_UART_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	INT_MASK	=	0x7FF;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC) & INT_MASK )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that At least one( or more ) pending is seted. 
 *				 CFALSE	indicates that All pending is NOT seted.
 */
CBOOL	NX_UART_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_MASK	=	0x7FF;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC) & PEND_MASK )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		None.
 */
void	NX_UART_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x7FF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->ICR, PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
U32		NX_UART_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x7FF << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_UART_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	ReadIO32(&pRegister->MIS) & PEND_MASK;

	// @modified charles 110630 ADS에서 문법을 지원하지 않는가보오
	//for( ; PendingIndex<=10 PendingIndex++)
	//	if(Pend & 1<<PendingIndex)
	//		break;
	for( PendingIndex=0 ; PendingIndex<=10 ; PendingIndex++)
		if(Pend & ((U32)0x1)<<PendingIndex)
			break;

	if(PendingIndex > 10)
		return -1;
	else
		return PendingIndex;
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index of UART's Tx
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DMA peripheral index of UART's Tx
 *	@see		NX_UART_GetDMAIndex_Rx, NX_UART_GetDMABusWidth
 */
U32		NX_UART_GetDMAIndex_Tx( U32 ModuleIndex )
{
	const U32 UartDmaIndexTx[NUMBER_OF_UART_MODULE-NUMBER_OF_pl01115_Uart_nodma_MODULE] =
			{
//				DMAINDEX_WITH_CHANNEL_LIST(UART,UARTTXDMA)
				DMAINDEX_OF_UART0_MODULE_UARTTXDMA,
				DMAINDEX_OF_pl01115_Uart_modem_MODULE_UARTTXDMA,
				DMAINDEX_OF_UART1_MODULE_UARTTXDMA
			};

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return UartDmaIndexTx[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index of UART's Rx
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DMA peripheral index of UART's Rx
 *	@see		NX_UART_GetDMAIndex_Tx, NX_UART_GetDMABusWidth
 */
U32		NX_UART_GetDMAIndex_Rx( U32 ModuleIndex )
{
	const U32 UartDmaIndexRx[NUMBER_OF_UART_MODULE-NUMBER_OF_pl01115_Uart_nodma_MODULE] =
			{
//				DMAINDEX_WITH_CHANNEL_LIST(UART,UARTRXDMA)

				DMAINDEX_OF_UART0_MODULE_UARTRXDMA,
				DMAINDEX_OF_pl01115_Uart_modem_MODULE_UARTRXDMA,
				DMAINDEX_OF_UART1_MODULE_UARTRXDMA
			};
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return UartDmaIndexRx[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA bus width of UART
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DMA bus width of UART
 *	@see		NX_UART_GetDMAIndex_Tx, NX_UART_GetDMAIndex_Rx
 */
U32		NX_UART_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );

	return 8;
}

//--------------------------------------------------------------------------
// Configuration operations
//--------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable SIR mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bEnb	Set as CTRUE to use SIR Mode
 *	@return		None.
 *	@remarks	determine whether or not to use the infra-red Tx/Rx mode
 */
void	NX_UART_SetSIRMode( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 SIR_MODE_MASK = (1<<1);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if ( CTRUE == bEnb )SetValue =	(U16)( SetValue | SIR_MODE_MASK );
	else				SetValue &= ~SIR_MODE_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get SIR mode's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that SIR Mode is Enabled.
 *				 CFALSE	indicates that SIR Mode is Disabled.
 */
CBOOL	NX_UART_GetSIRMode( U32 ModuleIndex )
{
	const U32 SIR_MODE_MASK = (1<<1);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & SIR_MODE_MASK )	{ return CTRUE; }
	else																			{ return CFALSE; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable Loop-back mode.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bEnb	Set as CTRUE to use Loop-back Mode
 *	@return		None.
 *	@remarks	determine whether or not to use the Loop-back mode.
 */
void	NX_UART_SetLoopBackMode( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 LOOPBACK_MODE_MASK = (1<<7);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if ( CTRUE == bEnb )SetValue = (U16)( SetValue | LOOPBACK_MODE_MASK );
	else				SetValue &= ~LOOPBACK_MODE_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, SetValue);
}

//-------------------------------------------------------------------------------
/**
 *	@brief		Get Loop-Back mode's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	Indicate that Loop-back mode is Enabled.
 *				 CFALSE	Indicate that Loop-back mode is Disabled.
 */
CBOOL	NX_UART_GetLoopBackMode( U32 ModuleIndex )
{
	const U32 LOOPBACK_MODE_MASK = (1<<7);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & LOOPBACK_MODE_MASK )	{ return CTRUE; }
	else																				{ return CFALSE; }
}

void	NX_UART_SetIntEnbWhenExceptionOccur( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 ExceptionMask = (0xFUL<<7);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( (CFALSE==bEnb) || (CTRUE==bEnb) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC);

	if ( CTRUE == bEnb )SetValue |= ExceptionMask;
	else				SetValue &= ExceptionMask;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC, SetValue);
}
CBOOL	NX_UART_GetIntEnbWhenExceptionOccur( U32 ModuleIndex )
{
	const U32 ExceptionMask = (0xFUL<<7);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC) & ExceptionMask )	{ return CTRUE; }
	else																				{ return CFALSE; }
}

void	NX_UART_SetTXEnable( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 TXEnb = (1<<8);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if ( CTRUE == bEnb )SetValue = (U16)( SetValue | TXEnb );
	else				SetValue &= ~TXEnb;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, SetValue);
}

CBOOL	NX_UART_GetTXEnable( U32 ModuleIndex )
{
	const U32 TXEnb = (1<<8);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & TXEnb )	{ return CTRUE; }
	else																	{ return CFALSE; }
}

void	NX_UART_SetRXEnable( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 TXEnb = (1<<9);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if ( CTRUE == bEnb )SetValue = (U16)( SetValue | TXEnb );
	else				SetValue &= ~TXEnb;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, SetValue);
}

CBOOL	NX_UART_GetRXEnable( U32 ModuleIndex )
{
	const U32 TXEnb = (1<<9);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & TXEnb )	{ return CTRUE; }
	else																	{ return CFALSE; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set frame configuration during UART trasmit and receive operation.
 *	@param[in]	ModuleIndex	An index of module ( 0 ~ x ).
 *	@param[in]	Parity		Parity generation/check type
 *	@param[in]	DataWidth	The number of data bit( 5, 6, 7, 8 bit )
 *	@param[in]	StopBit		The number of stop bit( 1, 2 )
 *	@return		None.
 *	@remarks	determine the number of data/stop bit and parity mode.
 */
void	NX_UART_SetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITYMODE Parity,	U32 DataWidth,	U32 StopBit )
{
	register U16 temp;

	const U32 PARITY_BITPOS	= 1;
	const U32 EVENP_BITPOS	= 2;
	const U32 STICKP_BITPOS	= 7;
	const U32 STOP_BITPOS	= 3;
	const U32 DATA_BITPOS	= 5;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0==Parity ||	( 1 <= Parity && Parity <= 4 ) );
	NX_ASSERT( 5 <= DataWidth && DataWidth <=8 );
	NX_ASSERT( (1==StopBit) || (2==StopBit) );

	temp = (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H));

	temp = (U16)( (U16)( (StopBit-1)	<<STOP_BITPOS)
				| (U16)( (DataWidth-5)	<<DATA_BITPOS) );

	if(Parity != 0)
	{
		temp |= (U16) 1<< PARITY_BITPOS;	// parity enable, odd parity
		if(Parity == 2)	// even parity
			temp |= (U16) 1<< EVENP_BITPOS;
		else if(Parity == 3)	// force 1
			temp |= (U16) 1<< STICKP_BITPOS;
		else if(Parity == 4)	// force 0
			temp |= ((U16) 1<< EVENP_BITPOS | (U16) 1<< STICKP_BITPOS);
	}


	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H, temp);
}

//-------------------------------------------------------------------------------
/**
 *	@brief		Get Frame Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[out]	pParity	Parity generation/check type
 *	@param[out]	pDataWidth	The number of data bit ( 5, 6, 7, 8 )
 *	@param[out]	pStopBit	The number of stop bit ( 1, 2 )
 *	@return		None.
 */
void	NX_UART_GetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITYMODE* pParity, U32* pDataWidth, U32* pStopBit )
{
	const U32	PARITY_BITPOS	= 1;
	const U32	EVENP_BITPOS	= 2;
	const U32	STICKP_BITPOS	= 7;
	const U32	WLEN_BITPOS = 5;
	const U32	WLEN_MASK	= ( 0x03 << WLEN_BITPOS );
	const U32	STP2_BITPOS = 3;
	const U32	STP2_MASK	= ( 0x01 << STP2_BITPOS );

	register U32 temp;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CNULL != pParity );
	NX_ASSERT( CNULL != pDataWidth );
	NX_ASSERT( CNULL != pStopBit );

	temp	=	ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H);

	*pParity	=	(NX_UART_PARITYMODE)0;
	if(temp & 1<<PARITY_BITPOS)
	{
		*pParity	=	(NX_UART_PARITYMODE)1;
		if(temp & 1<< STICKP_BITPOS)
		{
			*pParity	=	(NX_UART_PARITYMODE)((int)(*pParity) + 2);
		}
		if(temp & 1<< EVENP_BITPOS)
		{
			*pParity	=	(NX_UART_PARITYMODE)((int)(*pParity) + 1);
		}
	}

	*pDataWidth =	(( temp & WLEN_MASK ) >> WLEN_BITPOS)+5;
	*pStopBit	=	(( temp & STP2_MASK ) >> STP2_BITPOS)+1;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a BRD(Baud Rate Division) value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BRD	Baud rate division value, It must be greater than 0.
 *	@return		None.
 *	@remarks	The following fomula can determine the BRD
 *				BRD = (UART_CLK / (bps x 16))
 */
void	NX_UART_SetBRD( U32 ModuleIndex, U16 BRD )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CNULL != BRD );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IBRD, BRD);
}
U16		NX_UART_GetBRD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IBRD));
}
//-------------------------------------------------------------------------------
/**
 *	@brief		Get Baud Rate Divison Value
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Baud Rate Divison Value ( 0x0 ~ 0xFFFF )
 */
void	NX_UART_SetFBRD( U32 ModuleIndex, U16 FBRD )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
//	NX_ASSERT( CNULL == FBRD || ((__g_ModuleVariables[ModuleIndex].pRegister->FBRD & 0xFFFF)!=0xFFFF) );
	NX_ASSERT( 0x3F >= FBRD );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FBRD, FBRD);
}
U16		NX_UART_GetFBRD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FBRD));
}
//------------------------------------------------------------------------------
/**
 *	@brief		Make a BRD with a baud rate and clock in Hz.
 *	@param[in]	BaudRate	a desired baud rate.
 *	@param[in]	CLKinHz		a uart clock in Hz.
 *	@return		a calculated BRD value.
 *	@remarks	The following fomula can determine the BRD
 *				BRD = (UART_CLK / (bps x 16))
 */
U16		NX_UART_MakeBRD( U32 BaudRate, U32 CLKinHz )
{
	NX_ASSERT( 0 < BaudRate );
	NX_ASSERT( 0 < CLKinHz );

	CLKinHz /= BaudRate;
	CLKinHz /= 16;

	NX_ASSERT( (0x0000FFFF >= CLKinHz) && (0 < CLKinHz) );
	return (U16)CLKinHz;
}

U16		NX_UART_MakeFBRD( U32 BaudRate, U32 CLKinHz )
{
	NX_ASSERT( 0 < BaudRate );
	NX_ASSERT( 0 < CLKinHz );

	CLKinHz %= (BaudRate * 16 + 16/2);	// 0.xxxxxx
	CLKinHz <<= 6;	// mul by 64 (0.xxxxxx * 64 = xx.xxxxxx)
	CLKinHz += 32;	// + 0.5 (32/64) ( half add )
	CLKinHz /= (BaudRate * 16 + 16/2); // xx.000000

	NX_ASSERT( (0x0000003F >= CLKinHz) );
	return (U16)CLKinHz;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Status of RX Time Out interrupt is Enabled or Disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	Indicate that RX Time Out Interrupt is Enabled.
 *				 CFALSE	Indicate that RX Time Out Interrupt is Disabled.
 */
CBOOL	NX_UART_GetRxTimeOutEnb( U32 ModuleIndex )
{
	const U32	RXTIME_MASK	=	( 0x01 << 6 );

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC) & RXTIME_MASK ){ return CTRUE;	}
	else																		{ return CFALSE; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Send a break signal during 1 fream time.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		None.
 *	@remarks	Auto cleared after sending the break signal
 */
void	NX_UART_SendBreak( U32 ModuleIndex )
{
	const U32 SENDBREAK_MASK = (1<<0);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H) | SENDBREAK_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx DMA mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bEnb		DMA mode ( Disable or Enable Tx DMA )
 *	@return		None.
 */
void	NX_UART_SetTxDMAMode( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 TXOPERMODE_BITPOS	= 1;
	const U32 TXOPERMODE_MASK	= (1<<TXOPERMODE_BITPOS);

	register U32 temp;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR);
	if(bEnb)
		temp |= ((U32)1UL<<TXOPERMODE_BITPOS);
	else
		temp &= ~TXOPERMODE_MASK;
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx DMA mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	DMA Op mode Enabled
 *				 CFALSE	DMA Op mode Disabled
 */
CBOOL	NX_UART_GetTxDMAMode( U32 ModuleIndex )
{
	const U32 TXOPERMODE_BITPOS	= 1;
	const U32 TXOPERMODE_MASK	= (1<<TXOPERMODE_BITPOS);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(( __g_ModuleVariables[ModuleIndex].pRegister->DMACR & TXOPERMODE_MASK ) >> TXOPERMODE_BITPOS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Rx DMA mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bEnb		DMA mode ( Disable or Enable )
 *	@return		None.
 */
void	NX_UART_SetRxDMAMode( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 RXOPERMODE_MASK	= (1<<0);

	register U32 temp;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR);
	if(bEnb)
		temp |= RXOPERMODE_MASK;
	else
		temp &= ~RXOPERMODE_MASK;
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Rx DMA mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Value of operation mode ( Disable or Normal or DMA )
 */
CBOOL	NX_UART_GetRxDMAMode( U32 ModuleIndex )
{
	const U32 RXOPERMODE_MASK	= (1<<0);
	const U32 RXOPERMODE_BITPOS	= 0;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR) & RXOPERMODE_MASK ) >> RXOPERMODE_BITPOS );
}

void	NX_UART_SetUARTEnable( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 UARTEN_BITPOS	= 0;
	const U32 UARTEN_MASK	= (1<<UARTEN_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if(bEnb)
		temp |= UARTEN_MASK;
	else
		temp &= ~UARTEN_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, temp);
}
CBOOL	NX_UART_GetUARTEnable( U32 ModuleIndex )
{
	const U32 UARTEN_BITPOS	= 0;
	const U32 UARTEN_MASK	= (1<<UARTEN_BITPOS);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & UARTEN_MASK ) >> UARTEN_BITPOS );
}

//--------------------------------------------------------------------------
//	FIFO control operations
//--------------------------------------------------------------------------
/**
 *	@brief		Set the trigger level of Tx FIFO
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	level	FIFO trigger level ( 1/8 byte, 1/4byte, 1/2byte, 3/4byte, 7/8byte )
 *	@return		None.
 */
void	NX_UART_SetTxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_FIFOLEVEL level )
{
	const U32 TXFIFOTL_BITPOS	= 0;
	const U32 TXFIFOTL_MASK		= (7<<TXFIFOTL_BITPOS);

	register U32 temp;
	register U32 SetValue = 0;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT(  (NX_UART_FIFOLEVEL1_8==level) ||
				(NX_UART_FIFOLEVEL2_8==level) ||
				(NX_UART_FIFOLEVEL4_8==level) ||
				(NX_UART_FIFOLEVEL6_8==level) ||
				(NX_UART_FIFOLEVEL7_8==level) );

	switch( level )
	{
    	case NX_UART_FIFOLEVEL1_8:		SetValue = 0;	break;
    	case NX_UART_FIFOLEVEL2_8:		SetValue = 1;	break;
    	case NX_UART_FIFOLEVEL4_8:		SetValue = 2;	break;
    	case NX_UART_FIFOLEVEL6_8:		SetValue = 3;	break;
    	case NX_UART_FIFOLEVEL7_8:		SetValue = 4;	break;
	}

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS);
	temp &= ~TXFIFOTL_MASK;
	temp |= ((U32)SetValue<<TXFIFOTL_BITPOS);
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the trigger level of Tx FIFO
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Value of FIFO trigger level ( 1/8 byte, 1/4byte, 1/2byte, 3/4byte, 7/8byte )
 */
NX_UART_FIFOLEVEL		NX_UART_GetTxFIFOTriggerLevel( U32 ModuleIndex )
{
	const U32 TXFIFOTL_BITPOS	= 0;
	const U32 TXFIFOTL_MASK		= (7<<TXFIFOTL_BITPOS);

	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS) & TXFIFOTL_MASK ) >> TXFIFOTL_BITPOS;

	switch( ReadValue )
	{
	case 0:		return NX_UART_FIFOLEVEL1_8;
	case 1:		return NX_UART_FIFOLEVEL2_8;
	case 2:		return NX_UART_FIFOLEVEL4_8;
	case 3:		return NX_UART_FIFOLEVEL6_8;
	case 4:		return NX_UART_FIFOLEVEL7_8;
	}
	return NX_UART_FIFOLEVEL_ERR;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the trigger level of Rx FIFO
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	level	FIFO trigger level ( 1/8 byte, 1/4byte, 1/2byte, 3/4byte, 7/8byte )
 *	@return		None.
 */
void	NX_UART_SetRxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_FIFOLEVEL level )
{
	const U32 RXFIFOTL_BITPOS	= 3;
	const U32 RXFIFOTL_MASK		= (7<<RXFIFOTL_BITPOS);

	register U32 temp;
	register U32 SetValue=0;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (NX_UART_FIFOLEVEL1_8==level) ||
				(NX_UART_FIFOLEVEL2_8==level) ||
				(NX_UART_FIFOLEVEL4_8==level) ||
				(NX_UART_FIFOLEVEL6_8==level) ||
				(NX_UART_FIFOLEVEL7_8==level) );

	switch( level )
	{
	case NX_UART_FIFOLEVEL1_8:		SetValue = 0;	break;
	case NX_UART_FIFOLEVEL2_8:		SetValue = 1;	break;
	case NX_UART_FIFOLEVEL4_8:		SetValue = 2;	break;
	case NX_UART_FIFOLEVEL6_8:		SetValue = 3;	break;
	case NX_UART_FIFOLEVEL7_8:		SetValue = 4;	break;
	}

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS);
	temp &= ~RXFIFOTL_MASK;
	temp |= ((U32)SetValue<<RXFIFOTL_BITPOS);
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the trigger level of Rx FIFO
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Value of FIFO trigger level ( 1/8 byte, 1/4byte, 1/2byte, 3/4byte, 7/8byte )
 */
NX_UART_FIFOLEVEL		NX_UART_GetRxFIFOTriggerLevel( U32 ModuleIndex )
{
	const U32 RXFIFOTL_BITPOS	= 3;
	const U32 RXFIFOTL_MASK		= (7<<RXFIFOTL_BITPOS);

	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = (( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS) & RXFIFOTL_MASK ) >> RXFIFOTL_BITPOS );

	switch( ReadValue )
	{
	case 0:		return NX_UART_FIFOLEVEL1_8;
	case 1:		return NX_UART_FIFOLEVEL2_8;
	case 2:		return NX_UART_FIFOLEVEL4_8;
	case 3:		return NX_UART_FIFOLEVEL6_8;
	case 4:		return NX_UART_FIFOLEVEL7_8;
	}
	return NX_UART_FIFOLEVEL_ERR;
}
//------------------------------------------------------------------------------
/**
 *	@brief		Enable/disable FIFO mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bEnb	Set as CTRUE to use FIFO
 *	@return		None.
 */
void	NX_UART_SetFIFOEnb( U32 ModuleIndex, CBOOL bEnb )
{
	const U32 FIFOEN_MASK	= (1<<4);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H);

	if ( CTRUE == bEnb )SetValue = (U16)( SetValue | FIFOEN_MASK );
	else				SetValue &= ~FIFOEN_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get FIFO's mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	Indicate that FIFO is Enabled.
 *				 CFALSE	Indicate that FIFO is Disabled.
 */
CBOOL	NX_UART_GetFIFOEnb( U32 ModuleIndex )
{
	const U32 FIFOEN_MASK	= (1<<4);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( __g_ModuleVariables[ModuleIndex].pRegister->LCR_H & FIFOEN_MASK ){ return CTRUE; }
	else																{ return CFALSE; }
}

//--------------------------------------------------------------------------
//	Status operations
//--------------------------------------------------------------------------
/**
 *	@brief		Get Tx/Rx status
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		return status of Transmitter, Tx Buffer and Rx Buffer
 *	@remarks	Check state Using TXRXSTATUS.
 *	@code
 *		// mi : module index
 *		U32 state;
 *		state = NX_UART_GetTxRxStatus( mi );
 *		if( NX_UART_RX_BUFFER_READY & state ){ ... }
 *		if( NX_UART_TX_BUFFER_EMPTY & state ){ ... }
 *		...
 *	@endcode
 */
U32		NX_UART_GetTxRxStatus( U32 ModuleIndex )
{
	const U32	TXRXST_BITPOS	= 4;
	const U32	TXRXST_MASK		= ( 0x0F << TXRXST_BITPOS );

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FR) & TXRXST_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Error status
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		return status of Error which occurs during UART operation.
 *	@remarks		Check state Using ERRSTAT.
 *	@code
 *		// mi = module index
 *		U32 state;
 *		state = NX_UART_GetErrorStatus( mi );
 *		if( NX_UART_ERRSTAT_OVERRUN & state ){ ... }
 *		if( NX_UART_ERRSTAT_PARITY & state ) { ... }
 *		...
 *	@endcode
 */
U32		NX_UART_GetErrorStatus( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RSR_ECR) & 0xFUL);
//	return ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RSR_ECR);
}
void	NX_UART_ClearErrorStatus( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RSR_ECR, 0xFF);
}
//--------------------------------------------------------------------------
// Modem operations
//--------------------------------------------------------------------------
/**
 *	@brief		Activate/Deactivate nDTR signal.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bActive	Set as CTURE to activate nDTR signal.
 *	@return		None.
 *	@remarks	Output low level in active state.
 */
void	NX_UART_SetDTR( U32 ModuleIndex, CBOOL bActive )
{
	const U32 DTR_MASK	= (1<<10);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if( CTRUE == bActive )	SetValue = (U16)( SetValue | DTR_MASK );
	else					SetValue &= ~DTR_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DTR signal's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that DTR is Actived.
 *				 CFALSE	indicates that DTR is Inactived.
 *	@remarks	Output low level in active state.
 */
CBOOL	NX_UART_GetDTR( U32 ModuleIndex )
{
	const U32 DTR_MASK	= (1<<10);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & DTR_MASK )	{ return CTRUE; }
	else																		{ return CFALSE; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Activate/Deactivate nRTS signal.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	bActive	Set as CTURE to activate nRTS signal.
 *	@return		None.
 *	@remarks	output low level in active state.
 *				In AFC mode, nRTS signal is controled automatically by UART module .
 */
void	NX_UART_SetRTS( U32 ModuleIndex, CBOOL bActive )
{
	const U32 RTS_MASK	= (1<<11);
	register U32 SetValue;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if( CTRUE == bActive )	SetValue =	(U16)( SetValue | RTS_MASK );
	else					SetValue &= ~RTS_MASK;

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get RTS signal's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@return		 CTRUE	Indicate that RTS is Actived.
 *				 CFALSE	Indicate that RTS is Inactived.
 *	@remarks	Output low level in active state.
 *				In AFC mode, nRTS signal is controled automatically by UART module .
 */
CBOOL	NX_UART_GetRTS( U32 ModuleIndex )
{
	const U32 RTS_MASK	= (1<<11);

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & RTS_MASK )	{ return CTRUE; }
	else																		{ return CFALSE; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Auto Flow Control	Operation
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	enable	 CTRUE	indicates that Auto Flow Control Enable. 
 *						 CFALSE	indicates that Auto Flow Control Disable.
 *	@return		None.
 */
void	NX_UART_SetAutoFlowControl( U32 ModuleIndex, CBOOL enable )
{
	const U32	AFC_MASK = ( 0x03 << 14 );
	register U32	temp;

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);

	if( enable == CTRUE )	{ temp |=	AFC_MASK; }
	else					{ temp &=	~AFC_MASK; }

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Auto Flow Control	Operation's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	Indicate that Auto Flow Control is Enabled.
 *				 CFALSE	Indicate that Auto Flow Control is Disabled.
 */
CBOOL	NX_UART_GetAutoFlowControl( U32 ModuleIndex )
{
	const U32	AFC_MASK = ( 0x03 << 14 );

	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR) & AFC_MASK ){ return CTRUE; }
	else																	{ return CFALSE; }
}
//------------------------------------------------------------------------------
/**
 *	@brief		Get Modem Status.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		return status of Modem Pin ( DCD, RI, DSR, CTS )
 *	@remarks	Modem's status is cleared after status read.
 *	@code
 *		// mi = module index
 *		U32 state;
 *		state = NX_UART_GetModemStatus( mi );
 *		if( NX_UART_MODEMSTAT_DCD_CHANGED & state ){ ... }
 *		if( NX_UART_MODEMSTAT_RI_CHANGED	& state ){ ... }
 *		...
 *	@endcode

 */
U32	NX_UART_GetModemStatus( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FR) & 0x1FF );
}

//--------------------------------------------------------------------------
//	Basic operations
//--------------------------------------------------------------------------
/**
 *	@brief		Send a data to be transmitted.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Data	a data to be transmitted.
 *	@return		If the result is successful, then return CTRUE.
 */
void	NX_UART_SendByte( U32 ModuleIndex, U8 Data )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DR, Data);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a received data
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Value of Received Data
 */
U8		NX_UART_GetByte( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U8)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DR) & 0xFF);
}

//--------------------------------------------------------------------------
// UART Configuration Function
//--------------------------------------------------------------------------
/**
 *	@brief		Set UART Line Configuration
 *	@param[in]	ModuleIndex	An index of module ( 0 ~ x ).
 *	@param[in]	sirMode		Set as CTRUE to use SIR Mode
 *	@param[in]	Parity		Parity generation/check type
 *	@param[in]	DataWidth	The number of data bit ( 5, 6, 7, 8 )
 *	@param[in]	StopBit		The number of stop bit ( 1 ,2 )
 *	@return		None.
 */
void	NX_UART_SetLineConfig( U32 ModuleIndex, CBOOL sirMode, NX_UART_PARITYMODE Parity, U32 DataWidth, U32 StopBit )
{
	NX_UART_SetSIRMode( ModuleIndex, sirMode );
	NX_UART_SetFrameConfiguration( ModuleIndex, Parity, DataWidth, StopBit );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set UART Control Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	txOper			Operation mode ( Disable or Normal or DMA )
 *	@param[in]	rxOper			Operation mode ( Disable or Normal or DMA )
 *	@return		None
 */
void	NX_UART_SetControlConfig( U32 ModuleIndex, CBOOL txOper, CBOOL rxOper )
{
	NX_UART_SetTxDMAMode( ModuleIndex, txOper );
	NX_UART_SetRxDMAMode( ModuleIndex, rxOper );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set FIFO Control Configuration
 *	@param[in]	ModuleIndex			An index of module ( 0 ~ x ).
 *	@param[in]	fifoEnb			Set as CTRUE to use FIFO
 *	@param[in]	txFIFOReset		CTRUE( Reset Tx FIFO ), CFALSE( Nothing )
 *	@param[in]	rxFIFOReset		CTRUE( Reset Rx FIFO ), CFALSE( Nothing )
 *	@param[in]	txLevel			FIFO trigger level ( 2byte, 4byte, 8byte, 12byte, 14byte )
 *	@param[in]	rxLevel			FIFO trigger level ( 2byte, 4byte, 8byte, 12byte, 14byte )
 *	@return		None
 */
void	NX_UART_SetFIFOConfig( U32 ModuleIndex, CBOOL fifoEnb, U32 txLevel, U32 rxLevel )
{
	NX_UART_SetFIFOEnb( ModuleIndex, fifoEnb );
	NX_UART_SetTxFIFOTriggerLevel( ModuleIndex, txLevel );
	NX_UART_SetRxFIFOTriggerLevel( ModuleIndex, rxLevel );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Baud Rate & Rx Time Out Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	baudRate		Baud rate division value, It must be greater than 0.
 *	@return		None
 */
void	NX_UART_SetBaudRateConfig( U32 ModuleIndex, U16 baudRate )
{
	NX_UART_SetBRD( ModuleIndex, baudRate);
//	NX_UART_SetFBRD( ModuleIndex, baudRate);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set UART Modem Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	autoflow		CTRUE( Auto Flow Control Enable ), CFALSE( Auto Flow Control Disable )
 *	@param[in]	DTR				CTRUE( DTR Active), CFALSE( DTR InActive )
 *	@param[in]	RTS				CTRUE( RTS Active), CFALSE( RTS InActive )
 *	@return		None
 */
void	NX_UART_SetModemConfig( U32 ModuleIndex, CBOOL autoflow, CBOOL DTR, CBOOL RTS )
{
	NX_UART_SetAutoFlowControl( ModuleIndex, autoflow );
	NX_UART_SetDTR( ModuleIndex, DTR );
	NX_UART_SetRTS( ModuleIndex, RTS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get UART Line Configuration
 *	@param[in]	ModuleIndex	An index of module ( 0 ~ x ).
 *	@param[out]	pSirMode	1 : SIR Mode, 0 : UART Mode
 *	@param[out]	pParity		Parity generation/check type
 *	@param[out]	pDataWidth	The number of data bit ( 5, 6, 7, 8 )
 *	@param[out]	pStopBit	The number of stop bit ( 1, 2 )
 *	@return		None.
 */
void	NX_UART_GetLineConfig( U32 ModuleIndex, U32* pSirMode, NX_UART_PARITYMODE* pParity, U32* pDataWidth, U32* pStopBit )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != pSirMode );
	NX_ASSERT( CNULL != pParity );
	NX_ASSERT( CNULL != pDataWidth );
	NX_ASSERT( CNULL != pStopBit );

	if( CTRUE == NX_UART_GetSIRMode( ModuleIndex ) )	{ *pSirMode = 1;	}
	else												{ *pSirMode = 0;	}

	NX_UART_GetFrameConfiguration( ModuleIndex, pParity, pDataWidth, pStopBit );

	NX_ASSERT( 0==(*pParity) ||	( 4 <= (*pParity) && (*pParity) <= 7 ) );
	NX_ASSERT( 5 <= (*pDataWidth) && (*pDataWidth) <=8 );
	NX_ASSERT( (1==(*pStopBit)) || (2==(*pStopBit)) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get UART Control Configuration
 *	@param[in]	ModuleIndex	An index of module ( 0 ~ x ).
 *	@param[out]	pTxOper		Operation mode ( Disable or Normal or DMA )
 *	@param[out]	pRxOper		Operation mode ( Disable or Normal or DMA )
 *	@return		None
 */
void	NX_UART_GetControlConfig( U32 ModuleIndex, CBOOL* pTxOper, CBOOL* pRxOper )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != pTxOper );
	NX_ASSERT( CNULL != pRxOper );

	*pTxOper = NX_UART_GetTxDMAMode( ModuleIndex );
	*pRxOper = NX_UART_GetRxDMAMode( ModuleIndex );

	NX_ASSERT( 3 > *pTxOper );
	NX_ASSERT( 3 > *pRxOper );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get FIFO Control Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[out]	pFIFOEnb		1 : FIFO Enable			0 : FIFO Disable
 *	@param[out]	pTxLevel		FIFO trigger level ( 2byte, 4byte, 8byte, 12byte, 14byte )
 *	@param[out]	pRxLevel		FIFO trigger level ( 2byte, 4byte, 8byte, 12byte, 14byte )
 *	@return		None
 */
void	NX_UART_GetFIFOConfig( U32 ModuleIndex, U32* pFIFOEnb, U32* pTxLevel, U32* pRxLevel )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != pFIFOEnb );
	NX_ASSERT( CNULL != pTxLevel );
	NX_ASSERT( CNULL != pRxLevel );

	if( CTRUE == NX_UART_GetFIFOEnb( ModuleIndex ) ){ *pFIFOEnb = 1; }
	else											{ *pFIFOEnb = 0; }

	*pTxLevel = NX_UART_GetTxFIFOTriggerLevel( ModuleIndex );
	*pRxLevel = NX_UART_GetRxFIFOTriggerLevel( ModuleIndex );

	//NX_ASSERT( (2==(*pTxLevel)) || (4==(*pTxLevel)) || (8==(*pTxLevel)) || (12==(*pTxLevel)) || (14==(*pTxLevel)) );
	//NX_ASSERT( (2==(*pRxLevel)) || (4==(*pRxLevel)) || (8==(*pRxLevel)) || (12==(*pRxLevel)) || (14==(*pRxLevel)) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Baud Rate & Rx Time Out Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[out]	pBaudRate		Baud rate division value, It must be greater than 0.
 *	@return		None
 */
void	NX_UART_GetBaudRateConfig( U32 ModuleIndex, U16* pBaudRate )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != pBaudRate );

	*pBaudRate	= NX_UART_GetBRD( ModuleIndex );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get UART Modem Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[out]	pAutoflow		1:( Auto Flow Control Enable ),	0:( Auto Flow Control Disable )
 *	@param[out]	pDTR			1:( DTR Active),				0:( DTR InActive )
 *	@param[out]	pRTS			1:( RTS Active),				0:( RTS InActive )
 *	@return		None
 */
void	NX_UART_GetModemConfig( U32 ModuleIndex, U32* pAutoflow, U32* pDTR, U32* pRTS )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != pAutoflow );
	NX_ASSERT( CNULL != pDTR );
	NX_ASSERT( CNULL != pRTS );

	if( CTRUE == NX_UART_GetAutoFlowControl( ModuleIndex ) ){ *pAutoflow = 1; }
	else													{ *pAutoflow = 0; }

	if( CTRUE == NX_UART_GetDTR( ModuleIndex ) ){ *pDTR= 1; }
	else										{ *pDTR= 0; }

	if( CTRUE == NX_UART_GetRTS( ModuleIndex ) ){ *pRTS = 1; }
	else										{ *pRTS = 0; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Receive Status/Error Clear Register
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	value			Line Control Register Value.
 *	@return		None.
 */
void	NX_UART_SetRSRECR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RSR_ECR, value);
}
U16	NX_UART_GetRSRECR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RSR_ECR));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Flag Register
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Flag.
 */
U16	NX_UART_GetFR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FR));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set IrDA Low Power counter Register
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	value			FIFO Control Register Value.
 *	@return		None.
 */
void	NX_UART_SetILPR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ILPR, value);
}
U16	NX_UART_GetILPR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ILPR));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Line Control Register
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	value			Modem Control Register Value.
 *	@return		None.
 */
void	NX_UART_SetLCR_H( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H, value);
}

U16	NX_UART_GetLCR_H( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCR_H));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Control Register
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	value			control.
 *	@return		None.
 */
void	NX_UART_SetCR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR, value);
}

U16	NX_UART_GetCR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CR);
}


void	NX_UART_SetIFLS( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS, value);
}
U16	NX_UART_GetIFLS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IFLS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set interrupt Mask Register
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	value			Interrupt mask	Value.
 *	@return		None.
 */
void	NX_UART_SetIMSC( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC, value);
}
U16		NX_UART_GetIMSC( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->IMSC));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Raw Interrupt Status Register's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Raw Interrupt Status Register's value
 */
U16		NX_UART_GetRIS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RIS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Masked Interrupt Status Register's state
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Control Register's value
 */
U16		NX_UART_GetMIS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MIS));
}

void	NX_UART_SetICR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ICR, value);
}

void	NX_UART_SetDMACR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0x8 > value );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR, value);
}

U16		NX_UART_GetDMACR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->DMACR));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Test Registers
 */
void	NX_UART_SetTCR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TCR, value);
}
U16		NX_UART_GetTCR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TCR));
}

void	NX_UART_SetITIP( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ITIP, value);
}
U16		NX_UART_GetITIP( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ITIP));
}

void	NX_UART_SetITOP( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ITOP, value);
}
U16		NX_UART_GetITOP( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ITOP));
}

void	NX_UART_SetTDR( U32 ModuleIndex, U16 value )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TDR, value);
}
U16		NX_UART_GetTDR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TDR));
}
