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
//	File			: nx_uart.c
//	Description	: 
//	Author		: Deoks
//	History		: 2014.09:28 First implementation
//				  2014.10.15  Last implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_uart.h>

static	struct
{
	struct NX_UART_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_UART_CHANNEL] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicates that Initialize is successed.
 *			CFALSE indicates that Initialize is failed.
 */
CBOOL	NX_UART_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_UART_CHANNEL; i++ )
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
 */
U32		NX_UART_GetNumberOfModule( void )
{
	return NUMBER_OF_UART_CHANNEL;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_UART_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[NUMBER_OF_UART_CHANNEL] = {
//		PHY_BASEADDR_LIST( UART )
		PHY_BASEADDR_UART0_MODULE,
		PHY_BASEADDR_pl01115_Uart_modem_MODULE,
		PHY_BASEADDR_UART1_MODULE,
		PHY_BASEADDR_LIST(pl01115_Uart_nodma)
	}; 

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}


/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
U32 NX_UART_GetResetNumber (U32 ModuleIndex)
{
	const U32 ResetPinNumber[NUMBER_OF_UART_CHANNEL] =
	{
		RESETINDEX_OF_UART0_MODULE_nUARTRST,
		RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST,
		RESETINDEX_OF_UART1_MODULE_nUARTRST,
		RESETINDEX_LIST( pl01115_Uart_nodma, nUARTRST )
	};

	return (U32)ResetPinNumber[ModuleIndex];
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
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

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
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Initialize is successed.
 *				CFALSE			indicates that Initialize is failed.
 */
CBOOL	NX_UART_OpenModule( U32 ModuleIndex )
{

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Deinitialize is successed. 
 *				CFALSE			indicates that Deinitialize is failed.
 */
CBOOL	NX_UART_CloseModule( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Module is Busy. 
 *				CFALSE			indicates that Module is NOT Busy.
 */
CBOOL	NX_UART_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Ready to enter power-down stage. 
 *				CFALSE			indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_UART_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.\n
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
	NX_CASSERT( NUMBER_OF_UART_CHANNEL == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return	ClockNumber[ModuleIndex];
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
	const U32	UartInterruptNumber[NUMBER_OF_UART_CHANNEL] =
				{
//					INTNUM_LIST(UART)
					INTNUM_OF_UART0_MODULE,
					INTNUM_OF_pl01115_Uart_modem_MODULE,
					INTNUM_OF_UART1_MODULE,
					INTNUM_LIST(pl01115_Uart_nodma)
				};

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return	UartInterruptNumber[ModuleIndex];
}

// Interrupt Polling Register & Interrupt Source Register & Interrupt Mask Register

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum			Interrupt Number ( 0:RXD, 1:ERROR, 2:TXD, 3:MODEM ).
 *	@param[in]	Enable			CTRUE	indicates that Interrupt Enable. 
 *								CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_UART_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0xF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( 3 >= IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIO32(&pRegister->INTM) & PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum)));
	ReadValue	|=	(U32)((!Enable) << (IntNum)) ;

	WriteIO32(&pRegister->INTM, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum			Interrupt Number ( 0:RXD, 1:ERROR, 2:TXD, 3:MODEM ).
 *	@return		CTRUE			indicates that Interrupt is enabled
 *				CFALSE			indicates that Interrupt is disabled.
 */
CBOOL	NX_UART_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( 4 >= IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( !(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTM) >> IntNum) & 0x01 );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum			Interrupt Number ( 0:RXD, 1:ERROR, 2:TXD, 3:MODEM ).
 *	@return		CTRUE			indicates that Pending is seted. \r\n
 *				CFALSE			indicates that Pending is Not Seted.
 */
CBOOL NX_UART_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTP) >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Current setting value of pending bit. 
 *				"1" means pend bit is occured.
 *				"0" means pend bitis NOT occured. 
 *				- Return Value[0] : Rx pending state. 
 *				- Return Value[1] : Error pending state. 
 *				- Return Value[2] : Tx pending state. 
 *				- Return Value[3] : Modem pending state. 
 */
U32		NX_UART_GetInterruptPending32( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(U32)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTP) )  );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum			Interrupt number( 0:Rx, 1:Error, 2:Tx 3:Modem ).
 *	@return		None.
 */
void	NX_UART_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	1 << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( 4 >= IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->INTP, (1 << IntNum) & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below
 *								- PendingFlag[0] : RX pending bit. 
 *								- PendingFlag[1] : Error pending bit. 
 *								- PendingFlag[2] : TX pending bit. 
 *								- PendingFlag[3] : Modem pending bit. 
 *	@return		None.
 */
void	NX_UART_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0xF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( 0x10 > PendingFlag );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->INTP, PendingFlag & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	CTRUE	indicates that Set to all interrupt enable. 
 *						CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_UART_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32	INT_MASK	=	0xF;
	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
		SetValue	|=	INT_MASK;
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->INTM, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that At least one( or more ) interrupt is enabled. 
 *				CFALSE			indicates that All interrupt is disabled.
 */
void	NX_UART_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0xF << PEND_POS;

	register struct NX_UART_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->INTP, PEND_MASK);
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index of UART's Tx
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DMA peripheral index of UART's Tx
 */
U32		NX_UART_GetDMAIndex_Tx( U32 ModuleIndex )
{
	const U32 UartDmaIndexTx[NUMBER_OF_UART_CHANNEL] =
			{
//				DMAINDEX_WITH_CHANNEL_LIST(UART,UARTTXDMA)
				DMAINDEX_OF_UART0_MODULE_UARTTXDMA,
				DMAINDEX_OF_pl01115_Uart_modem_MODULE_UARTTXDMA,
				DMAINDEX_OF_UART1_MODULE_UARTTXDMA
			};

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return UartDmaIndexTx[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index of UART's Rx
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DMA peripheral index of UART's Rx
 */
U32		NX_UART_GetDMAIndex_Rx( U32 ModuleIndex )
{
	const U32 UartDmaIndexRx[NUMBER_OF_UART_CHANNEL] =
			{
//				DMAINDEX_WITH_CHANNEL_LIST(UART,UARTRXDMA)
				DMAINDEX_OF_UART0_MODULE_UARTRXDMA,
				DMAINDEX_OF_pl01115_Uart_modem_MODULE_UARTRXDMA,
				DMAINDEX_OF_UART1_MODULE_UARTRXDMA
			};

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return UartDmaIndexRx[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA bus width of UART
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DMA bus width of UART
 */
U32		NX_UART_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	return 8;
}

#if 0
//------------------------------------------------------------------------------
/**	@name		NX_UART_EnablePAD
 *	@brief		UART Pad Enable Function.
 *				[kor] SwitchDevice를 통해 해당 모듈의 PAD를 enable 해줍니다. SOC RTL 테스트환경을
 				기준으로 작성되었습니다.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		an interrupt number which has the most priority of pended interrupts. \n
 *				This value is one of @ref NX_UART_INT enum.
 *				If there's no interrupt which is pended and unmasked, it returns -1.
 */
void NX_UART_EnablePAD( U32 ModuleIndex, U32 ModeIndex )
{
	U32 i;
	const U32 PADNumber[3][NUMBER_OF_UART_CHANNEL] =	{
	     { PADINDEX_WITH_CHANNEL_LIST( UART, UARTRXD ) },
	     { PADINDEX_WITH_CHANNEL_LIST( UART, UARTTXD ) },
	     { PADINDEX_WITH_CHANNEL_LIST( UART, SMCAYEN ) }
	};
	NX_CASSERT( NUMBER_OF_UART_CHANNEL == (sizeof(PADNumber[0])/sizeof(PADNumber[0][0])) );
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );

	for(i=0; i<sizeof( PADNumber)/sizeof(PADNumber[0]); i++)
	{
		//NX_SWITCHDEVICE_Set_Switch_Enable ( PADNumber[i][ModuleIndex] );
		//NX_PAD_SetPadFunctionEnable       ( PADNumber[i][ModuleIndex], ModeIndex );
	}
}
#endif

//-------------------------------------------------------------------
/**
 *	@brief		Set (Enable/Disable) Infrared Mode 
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	InfraredMode	(0:Nomal Mode, 1:Infrared Mode) 
 *	@return		None
 */

void	NX_UART_SetInfraredMode( U32 ModuleIndex, NX_UART_SIGNAL_MODE IMode )
{
	const 	 U32 IMMODE_BITPOS		= 6;
	const 	 U32 IMMODE_MASK		= (1 << IMMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == IMode) || (1 == IMode) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);
	ReadValue &= ~IMMODE_MASK;
	ReadValue |= (IMode << IMMODE_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON, ReadValue);
}
//-------------------------------------------------------------------
/**
 *	@brief		Get Infrared Mode 
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		InfraredMode	(0:Nomal Mode, 1:Infrared Mode) 
 */
NX_UART_SIGNAL_MODE	NX_UART_GetInfraredMode( U32 ModuleIndex )
{
	const	 U32 IMMODE_BITPOS		= 6;
	const 	 U32 IMMODE_MASK		= (1 << IMMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);

	return (NX_UART_SIGNAL_MODE)(( ReadValue & IMMODE_MASK ) >> IMMODE_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Parity Mode 
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 * 	@param[in] ParityMode
 				(0xx: No Parity, 100:Odd Parity, 101:Even Parity, 110:Checked as 1, 111: Checked as 0 )
 *	@return		None
 */
void	NX_UART_SetParityMode( U32 ModuleIndex, NX_UART_PARITY Parity )
{
	const 	 U32 PARITYMODE_BITPOS		= 3;
	const 	 U32 PARITYMODE_MASK		= (7 << PARITYMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (7 >= Parity) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);
	ReadValue &= ~PARITYMODE_MASK;
	ReadValue |= (Parity << PARITYMODE_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Parity Mode 
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Parity Mode 
 				(0xx: No Parity, 100:Odd Parity, 101:Even Parity, 110:Checked as 1, 111: Checked as 0 )
 				
 */
NX_UART_PARITY		NX_UART_GetParityMode( U32 ModuleIndex )
{
	const 	 U32 PARITYMODE_BITPOS		= 3;
	const 	 U32 PARITYMODE_MASK		= (7 << PARITYMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);

	return (NX_UART_PARITY)(( ReadValue & PARITYMODE_MASK ) >> PARITYMODE_BITPOS );
}


//-------------------------------------------------------------------
/**
 *	@brief		Set  Stop Bit
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 * 	@param[in] Stop Bit (0:One Bit, 1:Two Bit)
 *	@return		Nonen
 */
void	NX_UART_SetStopBit( U32 ModuleIndex, NX_UART_STOP_BIT StopBit )
{
	const 	 U32 STOPBIT_BITPOS		= 2;
	const 	 U32 STOPBIT_MASK		= (1 << STOPBIT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0==StopBit) || (1==StopBit) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);
	ReadValue &= ~STOPBIT_MASK;
	ReadValue |= (StopBit << STOPBIT_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Stop Bit 
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Stop Bit (0:One Bit, 1:Two Bit)
 */
NX_UART_STOP_BIT		NX_UART_GetStopBit( U32 ModuleIndex )
{
	const 	 U32 STOPBIT_BITPOS		= 2;
	const 	 U32 STOPBIT_MASK			= (1 << STOPBIT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);

	return (NX_UART_STOP_BIT)(( ReadValue & STOPBIT_MASK ) >> STOPBIT_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Word Length
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 * 	@param[in] DataWidth ( 00:5bit, 01:6bit, 10:7bit, 11:8bit )
 *	@return		None
 */
void	NX_UART_SetDataWidth( U32 ModuleIndex, NX_UART_DATA_BIT DataWidth )

{
	const 	 U32 WORDL_BITPOS		= 0;
	const 	 U32 WORDL_MASK			= (3 << WORDL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (3 >= DataWidth) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);
	ReadValue &= ~WORDL_MASK;
	ReadValue |= (DataWidth << WORDL_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Stop Bit 
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		DataWidth ( 00:5bit, 01:6bit, 10:7bit, 11:8bit )
 */
NX_UART_DATA_BIT	NX_UART_GetDataWidth( U32 ModuleIndex )
{
	const 	 U32 WORDL_BITPOS			= 0;
	const 	 U32 WORDL_MASK			= (3 << WORDL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);

	return (NX_UART_DATA_BIT)(( ReadValue & WORDL_MASK ) >> WORDL_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Parity, Stop Bit, Word Length
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Parity Mode 
 				(0xx: No Parity, 100:Odd Parity, 101:Even Parity, 110:Checked as 1, 111: Checked as 0 )
 *   @param[in] Stop Bit (0:One Bit, 1:Two Bit)
 * 	@param[in] DataWidth ( 00:5bit, 01:6bit, 10:7bit, 11:8bit )
 *	@return		None
 */
void	NX_UART_SetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITY Parity,	NX_UART_DATA_BIT DataWidth,	NX_UART_STOP_BIT StopBit )
{
	const U32 PARITYMODE_BITPOS		= 3;
	const U32 STOPBIT_BITPOS		= 2;
	const U32 WORDL_BITPOS			= 0;
	
	const U32 PARITYMODE_MASK		= (7 << PARITYMODE_BITPOS);
	const U32 STOPBIT_MASK			= (1 << STOPBIT_BITPOS);
	const U32 WORDL_MASK			= (3 << WORDL_BITPOS);	

	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	NX_ASSERT( (7 >= Parity) );
	NX_ASSERT( (0 == StopBit) || (1 == StopBit) );
	NX_ASSERT( (3 >= DataWidth) );

	ReadValue =  ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);
	ReadValue &= (~PARITYMODE_MASK) | (~STOPBIT_MASK) | (~WORDL_MASK) ;
	ReadValue |=  (U8)( (Parity << PARITYMODE_BITPOS) | (DataWidth << WORDL_BITPOS) | (StopBit << STOPBIT_BITPOS) ); 
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Parity, Stop Bit, Word Length
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Parity Mode 
 				(0xx: No Parity, 100:Odd Parity, 101:Even Parity, 110:Checked as 1, 111: Checked as 0 )
 *   @param[in] Stop Bit (0:One Bit, 1:Two Bit)
 * 	@param[in] Word Length ( 00:5bit, 01:6bit, 10:7bit, 11:8bit )
 *	@return		None
 */
void	NX_UART_GetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITY* pParity, NX_UART_DATA_BIT* pDataWidth, NX_UART_STOP_BIT* pStopBit )
{
	const U32 PARITYMODE_BITPOS 	= 3;
	const U32 STOPBIT_BITPOS		= 2;
	const U32 WORDL_BITPOS			= 0;
	
	const U32 PARITYMODE_MASK		= (7 << PARITYMODE_BITPOS);
	const U32 STOPBIT_MASK			= (1 << STOPBIT_BITPOS);
	const U32 WORDL_MASK			= (3 << WORDL_BITPOS);	

	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( CNULL != pParity );
	NX_ASSERT( CNULL != pDataWidth );
	NX_ASSERT( CNULL != pStopBit );

	ReadValue =  ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->LCON);

	*pParity	=	(( ReadValue & PARITYMODE_MASK ) >> PARITYMODE_BITPOS);
	*pDataWidth =	(( ReadValue & WORDL_MASK  	   ) >> WORDL_BITPOS) + 5;
	*pStopBit	=	(( ReadValue & STOPBIT_MASK    ) >> STOPBIT_BITPOS) + 1;	

}


//-------------------------------------------------------------------
/**
 *	@brief		Set  Trasmit DMA Burst Size
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	BrustSize 
 				(000: 1byte, 001:4bytes, 010:8bytes, 011~:Reserved )
 *	@return		None
 */
void	NX_UART_SetTxDMABurstSize( U32 ModuleIndex, NX_UART_DMA_BURST_SIZE BurstSize )

{
	const 	 U32 TXDBS_BITPOS		= 20;
	const 	 U32 TXDBS_MASK			= (7 << TXDBS_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (3 > BurstSize) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~TXDBS_MASK;
	ReadValue |= (BurstSize << TXDBS_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Trasmit DMA Burst Size
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		BrustSize 
 				(000: 1byte, 001:4bytes, 010:8bytes, 011~:Reserved )
 */
NX_UART_DMA_BURST_SIZE	NX_UART_GetTxDMABurstSize( U32 ModuleIndex )
{
	const 	 U32 TXDBS_BITPOS		= 20;
	const 	 U32 TXDBS_MASK			= (7 << TXDBS_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (NX_UART_DMA_BURST_SIZE)(( ReadValue & TXDBS_MASK ) >> TXDBS_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive DMA Burst Size
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	BrustSize 
 				(000: 1byte, 001:4bytes, 010:8bytes, 011~:Reserved )
 *	@return		None
 */
void	NX_UART_SetRxDMABurstSize( U32 ModuleIndex, NX_UART_DMA_BURST_SIZE BurstSize )

{
	const 	 U32 TXDBS_BITPOS		= 16;
	const 	 U32 TXDBS_MASK			= (7 << TXDBS_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (3 >= BurstSize) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~TXDBS_MASK;
	ReadValue |= (BurstSize << TXDBS_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Trasmit DMA Burst Size
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		BrustSize 
 				(000: 1byte, 001:4bytes, 010:8bytes, 011~:Reserved )
 */
 NX_UART_DMA_BURST_SIZE	NX_UART_GetRxDMABurstSize( U32 ModuleIndex )
{
	const 	 U32 TXDBS_BITPOS		= 16;
	const 	 U32 TXDBS_MASK			= (7 << TXDBS_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (NX_UART_DMA_BURST_SIZE)(( ReadValue & TXDBS_MASK ) >> TXDBS_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive DMA Suspend Enable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@parma[in]	Receive DMA Suspend (0: Disable, 1: Enable )
 */
void	NX_UART_SetRxDMASuspendEnable( U32 ModuleIndex, U32 Enable )
{
	const 	 U32 RXDMASENB_BITPOS		= 10;
	const 	 U32 RXDMASENB_MASK			= (1 << RXDMASENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXDMASENB_MASK;
	ReadValue |= (Enable << RXDMASENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive DMA Suspend Enable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Receive DMA Suspend (0: Disable, 1: Enable )
 */

CBOOL	NX_UART_GetRxDMASuspendEnable( U32 ModuleIndex )
{
	const 	 U32 RXDMASENB_BITPOS		= 10;
	const 	 U32 RXDMASENB_MASK			= (1 << RXDMASENB_BITPOS);
	register U32 ReadValue;
	
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (U8)(( ReadValue & RXDMASENB_MASK ) >> RXDMASENB_BITPOS );

}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive TimeOut Interal
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	TimeOutInterval (0 ~ 16 )
 *	@return		None
 */
void	NX_UART_SetRxTimeOutInterval( U32 ModuleIndex, U32 TimeOutInterval )
{
	const 	 U32 RXTOI_BITPOS		= 12;
	const 	 U32 RXTOI_MASK			= (0xF << RXTOI_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (16 >= TimeOutInterval) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXTOI_MASK;
	ReadValue |= (TimeOutInterval << RXTOI_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive TimeOut Interal
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	TimeOutInterval (0 ~ 16 )
 */
U8	NX_UART_GetRxTimeOutInterval( U32 ModuleIndex )
{
	const 	 U32 RXTOI_BITPOS		= 12;
	const 	 U32 RXTOI_MASK			= (0xF << RXTOI_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (U8)(( ReadValue & RXTOI_MASK ) >> RXTOI_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive Empty Time Out Enable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *   @param[in] Receive FIFO Empty TimeOut Enable (0: Disable, 1: Enable )
 *	@return		None
 */
void	NX_UART_SetRxFIFOEmptyTimeOutEnable( U32 ModuleIndex, U32 Enable )
{
	const 	 U32 RXFFTOENB_BITPOS		= 11;
	const 	 U32 RXFFTOENB_MASK			= (1 << RXFFTOENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXFFTOENB_MASK;
	ReadValue |= (Enable << RXFFTOENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);

}
//
//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive Empty Time Out Enable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return 	CTRUE  (1:Enable)
				CFALSE (0:Disable)
 */
CBOOL	NX_UART_GetRxFIFOEmptyTimeOutEnable( U32 ModuleIndex )
{
	const 	 U32 RXFFTOENB_BITPOS		= 11;
	const 	 U32 RXFFTOENB_MASK			= (1 << RXFFTOENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (CBOOL)(( ReadValue & RXFFTOENB_MASK ) >> RXFFTOENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Transmit Interrupt Type
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Type (0:Pulse, 1:Level)
 *	@return		None
 */
void	NX_UART_SetTxInterruptType( U32 ModuleIndex, CBOOL Type )
{
	const 	 U32 TXIT_BITPOS		= 9;
	const 	 U32 TXIT_MASK			= (1 << TXIT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Type) || (1 == Type) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~TXIT_MASK;
	ReadValue |= (Type << TXIT_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Transmit Interrupt Type
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return 	Type (0:Pulse, 1:Level)
 */
CBOOL	NX_UART_GetTxInterruptType( U32 ModuleIndex )
{
	const 	 U32 TXIT_BITPOS		= 9;
	const 	 U32 TXIT_MASK			= (1 << TXIT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (U8)(( ReadValue & TXIT_MASK ) >> TXIT_BITPOS );
}


//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive Interrupt Type
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Type (0:Pulse, 1:Level)
 *	@return		None
 */
void	NX_UART_SetRxInterruptType( U32 ModuleIndex, CBOOL Type )
{
	const 	 U32 RXIT_BITPOS		= 8;
	const 	 U32 RXIT_MASK			= (1 << RXIT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Type) || (1 == Type) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXIT_MASK;
	ReadValue |= (Type << RXIT_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive Interrupt Type
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return 	Type (0:Pulse, 1:Level)
 */
CBOOL	NX_UART_GetRxInterruptType( U32 ModuleIndex )
{
	const 	 U32 RXIT_BITPOS		= 8;
	const 	 U32 RXIT_MASK			= (1 << RXIT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (U8)(( ReadValue & RXIT_MASK ) >> RXIT_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive TimeOut Enable/Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable)
 *	@return		None
 */
void	NX_UART_SetRxTimeOutEnable( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 RXTOENB_BITPOS		= 7;
	const 	 U32 RXTOENB_MASK		= (1 << RXTOENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXTOENB_MASK;
	ReadValue |= (Enable << RXTOENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive TimeOut Enable/Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return 	CTRUE  (1:Enable)
 				CFALSE (0:Disable)
 */
CBOOL	NX_UART_GetRxTimeOutEnable( U32 ModuleIndex )
{
	const 	 U32 RXTOENB_BITPOS		= 7;
	const 	 U32 RXTOENB_MASK		= (1 << RXTOENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (CBOOL)(( ReadValue & RXTOENB_MASK ) >> RXTOENB_BITPOS );
}


//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive Error Status Interrupt Enable or Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable)
 *	@return		None
 */
void	NX_UART_SetRxErrorStatusInterruptEnable( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 RXTOSTAINTENB_BITPOS	= 6;
	const 	 U32 RXTOSTAINTENB_MASK		= (1 << RXTOSTAINTENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXTOSTAINTENB_MASK;
	ReadValue |= (Enable << RXTOSTAINTENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive Error Status Interrupt Enable or Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return 	CTRUE  (1:Enable)
 				CFALSE (0:Disable)
 */

CBOOL	NX_UART_GetRxErrorStatusInterruptEnable( U32 ModuleIndex )
{
	const 	 U32 RXTOSTAINTENB_BITPOS	= 6;
	const 	 U32 RXTOSTAINTENB_MASK		= (1 << RXTOSTAINTENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (CBOOL)(( ReadValue & RXTOSTAINTENB_MASK ) >> RXTOSTAINTENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive Loop Back Mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable)
 *	@return		None
 */
void	NX_UART_SetLoopBackMode( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 RXLMENB_BITPOS		= 5;
	const 	 U32 RXLMENB_MASK		= (1 << RXLMENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RXLMENB_MASK;
	ReadValue |= (Enable << RXLMENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive Loop Back Mode
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return 	CTRUE  (1:Enable)
 				CFALSE (0:Disable)
 */
CBOOL	NX_UART_GetLoopBackMode( U32 ModuleIndex )
{
	const 	 U32 RXLMENB_BITPOS			= 5;
	const 	 U32 RXLMENB_MASK			= (1 << RXLMENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (CBOOL)(( ReadValue & RXLMENB_MASK ) >> RXLMENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Send Break Signal
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Nomal Tranmit, 1:Sends the break Signal)
 *	@return		Nonee  
 */
void	NX_UART_SetSendBreakSignal( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 SBSENB_BITPOS			= 4;
	const 	 U32 SBSENB_MASK			= (1 << SBSENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) && (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~SBSENB_MASK;
	ReadValue |= (Enable << SBSENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}
	
//-------------------------------------------------------------------
/**
 *	@brief		Get  Send Break Signal
 *	@param[in]	ModuleIndex 	An index of module ( 0 ~ x ).
 *	@return		CFLASE (0:Nomal Tranmit)
 				CTRUE	(1:Sends the break Signal)
 */
CBOOL	NX_UART_GetSendBreakSignal( U32 ModuleIndex )
{
	const 	 U32 SBSENB_BITPOS			= 4;
	const 	 U32 SBSENB_MASK			= (1 << SBSENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (CBOOL)(( ReadValue & SBSENB_MASK ) >> SBSENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set Transmit Mode
 *	@param[in]	ModuleIndex 	An index of module ( 0 ~ x ).
 *   @param[in] 	Mode (0: Disable, 1: UART, 2: DMA)
 *	@return		None
 */
void	NX_UART_SetTransmitMode( U32 ModuleIndex, NX_UART_OPMODE mode )
{
	const 	 U32 TRMODE_BITPOS	= 2;
	const 	 U32 TRMODE_MASK	= (3<<TRMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( ( 3 >= mode ) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~TRMODE_MASK;
	ReadValue |= (mode<<TRMODE_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Transmit Mode
 *	@param[in]	ModuleIndex 	An index of module ( 0 ~ x ).
 *	@return		Mode (0: Disable, 1: UART, 2: DMA)
 */
NX_UART_OPMODE	NX_UART_GetTransmitMode( U32 ModuleIndex )
{
	const 	 U32 TRMODE_BITPOS	= 2;
	const 	 U32 TRMODE_MASK	= (3<<TRMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (NX_UART_OPMODE)(( ReadValue & TRMODE_MASK ) >> TRMODE_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set Receive Mode
 *	@param[in]	ModuleIndex 	An index of module ( 0 ~ x ).
 *   @param[in] 	Mode (0: Disable, 1: UART, 2: DMA)
 *	@return		None
 */
void	NX_UART_SetRecieveMode( U32 ModuleIndex, NX_UART_OPMODE mode )
{
	const 	 U32 RCMODE_BITPOS	= 0;
	const 	 U32 RCMODE_MASK	= (3<<RCMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( ( 3 >= mode ) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
	ReadValue &= ~RCMODE_MASK;
	ReadValue |= (mode<<RCMODE_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Receive Mode
 *	@param[in]	ModuleIndex 	An index of module ( 0 ~ x ).
 *	@return		Mode (0: Disable, 1: UART, 2: DMA)
 */
NX_UART_OPMODE		NX_UART_GetRecieveMode( U32 ModuleIndex )
{
	const	 U32 RCMODE_BITPOS	= 0;
	const	 U32 RCMODE_MASK	= (3<<RCMODE_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);

	return (U8)(( ReadValue & RCMODE_MASK ) >> RCMODE_BITPOS );
}


// FIFO Control Register
//-------------------------------------------------------------------
/**
 *	@brief		Set  Transmit FIFO Trigger Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Level
 				
 *	@return		None
 */
void	NX_UART_SetTxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_TX_FIFOLEVEL Level )
{
	const 	 U32 TXFFTL_BITPOS		= 8;
	const 	 U32 TXFFTL_MASK		= (7 << TXFFTL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (7 >= Level) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);
	ReadValue &= ~TXFFTL_MASK;
	ReadValue |= (Level << TXFFTL_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Transmit FIFO Trigger Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		None
 */
NX_UART_TX_FIFOLEVEL	NX_UART_GetTxFIFOTriggerLevel( U32 ModuleIndex )
{
	const 	 U32 TXFFTL_BITPOS		= 8;
	const 	 U32 TXFFTL_MASK		= (7 << TXFFTL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);

	return (NX_UART_TX_FIFOLEVEL)(( ReadValue & TXFFTL_MASK ) >> TXFFTL_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive FIFO Trigger Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Level
 				
 *	@return		None
 */
void	NX_UART_SetRxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_RX_FIFOLEVEL Level )
{
	const 	 U32 TXFFTL_BITPOS		= 4;
	const 	 U32 TXFFTL_MASK		= (7 << TXFFTL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (7 >= Level) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);
	ReadValue &= ~TXFFTL_MASK;
	ReadValue |= (Level << TXFFTL_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Receive FIFO Trigger Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		None
 */
NX_UART_RX_FIFOLEVEL	NX_UART_GetRxFIFOTriggerLevel( U32 ModuleIndex )
{
	const 	 U32 TXFFTL_BITPOS		= 4;
	const 	 U32 TXFFTL_MASK		= (7 << TXFFTL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);

	return (NX_UART_RX_FIFOLEVEL)(( ReadValue & TXFFTL_MASK ) >> TXFFTL_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Transmit FIFO Reset
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Nomal, 1:Tx FIFO Reset ) 				
 *	@return		None
 */
void	NX_UART_SetTxFIFOReset( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 TXFRST_BITPOS		= 2;
	const 	 U32 TXFRST_MASK		= (1 << TXFRST_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);
	ReadValue &= ~TXFRST_MASK;
	ReadValue |= (Enable << TXFRST_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Transmit FIFO Reset
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Enable (0:Nomal, 1:Tx FIFO Reset ) 
 */
CBOOL	NX_UART_GetTxFIFOReset( U32 ModuleIndex )
{
	const 	 U32 TXFRST_BITPOS		= 2;
	const 	 U32 TXFRST_MASK		= (1 << TXFRST_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);

	return (CBOOL)(( ReadValue & TXFRST_MASK ) >> TXFRST_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Transmit FIFO Reset
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Nomal, 1:Tx FIFO Reset ) 				
 *	@return		None
 */
void	NX_UART_SetRxFIFOReset( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 RXFRST_BITPOS		= 1;
	const 	 U32 RXFRST_MASK		= (1 << RXFRST_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 <= Enable) || (1 >= Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);
	ReadValue &= ~RXFRST_MASK;
	ReadValue |= (Enable << RXFRST_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Transmit FIFO Reset
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Enable (0:Nomal, 1:Tx FIFO Reset ) 
 */
CBOOL	NX_UART_GetRxFIFOReset( U32 ModuleIndex )
{
	const U32 RXFRST_BITPOS			= 1;
	const U32 RXFRST_MASK			= (1 << RXFRST_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);

	return (CBOOL)(( ReadValue & RXFRST_MASK ) >> RXFRST_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  FIFO Enable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable ) 				
 *	@return		Nonet
 *				NX_UART_SetFIFOEnable				NX_UART_SetFIFOEnable
 */
void	NX_UART_SetFIFOEnb( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 FFENB_BITPOS		= 0;
	const 	 U32 FFENBT_MASK		= (1 << FFENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);
	ReadValue &= ~FFENBT_MASK;
	ReadValue |= (Enable << FFENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  FIFO Enable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Enable (0:Disable, 1:Enable ) 
 */
CBOOL	NX_UART_GetFIFOEnb( U32 ModuleIndex )
{
	const 	 U32 FFENB_BITPOS		= 0;
	const 	 U32 FFENBT_MASK		= (1 << FFENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FCON);

	return (CBOOL)(( ReadValue & FFENBT_MASK ) >> FFENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set FIFO Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] FIFO Enb		CTRUE	FIFO Enable
 								CFALSE	FIFO Disable
 *	@param[in]	Tx Trigger Level ( Range 0 ~ 7 )
  *	@param[in]	Rx Trigger Level ( Range 0 ~ 7 )
 */
void	NX_UART_SetFIFOConfig( U32 ModuleIndex, CBOOL fifoEnb, U32 txLevel, U32 rxLevel )
{
	NX_UART_SetFIFOEnb( ModuleIndex, fifoEnb );
	NX_UART_SetTxFIFOTriggerLevel( ModuleIndex, txLevel );
	NX_UART_SetRxFIFOTriggerLevel( ModuleIndex, rxLevel );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get FIFO Configuration
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] FIFO Enb		CTRUE	FIFO Enable
 								CFALSE	FIFO Disable
 *	@param[in]	Tx Trigger Level ( Range 0 ~ 7 )
  *	@param[in]	Rx Trigger Level ( Range 0 ~ 7 )
 */
void	NX_UART_GetFIFOConfig( U32 ModuleIndex, CBOOL* fifoEnb, U32* txLevel, U32* rxLevel )
{
	*fifoEnb = NX_UART_GetFIFOEnb( ModuleIndex );
	*txLevel = NX_UART_GetTxFIFOTriggerLevel( ModuleIndex );
	*rxLevel = NX_UART_GetRxFIFOTriggerLevel( ModuleIndex );
}


//-------------------------------------------------------------------
/**
 *	@brief		Set  RTS Trigger Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable ) 				
 *	@return		None
 */
void	NX_UART_SetRTSTriggerLevel( U32 ModuleIndex, NX_UART_RTS_FIFOLEVEL Level )
{
	const 	 U32 RTSTL_BITPOS		= 5;
	const 	 U32 RTSTL_MASK			= (7 << RTSTL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (7 >= Level) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);
	ReadValue &= ~RTSTL_MASK;
	ReadValue |= (Level << RTSTL_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON, ReadValue);
}


//-------------------------------------------------------------------
/**
 *	@brief		Get  RTS Trigger Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).		
 *	@return		CFALSE (0:Disable)
 				CTRUE	(1:Enable ) 			
 */
NX_UART_RTS_FIFOLEVEL	NX_UART_GetRTSTriggerLevel( U32 ModuleIndex )
{
	const U32 RTSTL_BITPOS			= 5;
	const U32 RTSTL_MASK			= (7 << RTSTL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);

	return (NX_UART_RTS_FIFOLEVEL)(( ReadValue & RTSTL_MASK ) >> RTSTL_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set Auto FLow Control Enable/Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable ) 				
 *	@return		None
 */
void	NX_UART_SetAutoFlowControl( U32 ModuleIndex, NX_UART_AFC Enable )
{
	const 	 U32 AFC_BITPOS			= 4;
	const 	 U32 AFC_MASK			= (1 << AFC_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);
	ReadValue &= ~AFC_MASK;
	ReadValue |= (Enable << AFC_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Auto FLow Control Enable/Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CFALSE (0:Disable)
 				CTRUE  (1:Enable ) 
 */
CBOOL	NX_UART_GetAutoFlowControl( U32 ModuleIndex )
{
	const 	 U32 AFC_BITPOS			= 4;
	const 	 U32 AFC_MASK			= (1 << AFC_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);

	return (CBOOL)(( ReadValue & AFC_MASK ) >> AFC_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set Modem Interrupt Enable/Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Enable (0:Disable, 1:Enable ) 				
 *	@return		None
 */
void	NX_UART_SetModemInterruptEnable( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 MIENB_BITPOS		= 3;
	const 	 U32 MIENB_MASK			= (1 << MIENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Enable) || (1 == Enable) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);
	ReadValue &= ~MIENB_MASK;
	ReadValue |= (Enable << MIENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Modem Interrupt Enable/Disable
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).) 				
 *	@return		CFALSE (0:Disable)
 				CTRUE  (1:Enable ) 	
 */
 
CBOOL	NX_UART_GetModemInterruptEnable( U32 ModuleIndex )
{
	const 	 U32 MIENB_BITPOS		= 3;
	const 	 U32 MIENB_MASK			= (1 << MIENB_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);

	return (CBOOL)(( ReadValue & MIENB_MASK ) >> MIENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set RTS	Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in] 	Active (0:Inactive, 1:Active ) 				
 *	@return		None
 */
void	NX_UART_SetRTSLevel( U32 ModuleIndex, CBOOL Active )
{
	const 	 U32 RTSL_BITPOS		= 0;
	const 	 U32 RTSL_MASK			= (1 << RTSL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Active) || (1 == Active) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);
	ReadValue &= ~RTSL_MASK;
	ReadValue |= (Active << RTSL_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON, ReadValue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  RTS Level
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).) 				
 *	@return		CFALSE (0:Disable)
 				CTRUE  (1:Enable ) 	
 */
CBOOL	NX_UART_GetRTSLevel( U32 ModuleIndex )
{
	const 	 U32 RTSL_BITPOS		= 0;
	const 	 U32 RTSL_MASK			= (1 << RTSL_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MCON);

	return (CBOOL)(( ReadValue & RTSL_MASK ) >> RTSL_BITPOS );
}

// Modem Status
//-------------------------------------------------------------------
/**
 *	@brief		Get CTS Changed State
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE   (1:Changed)
 				CFLASE (0: Not Changed) 
 */
CBOOL	NX_UART_GetCTSStatus( U32 ModuleIndex )
{
	const 	 U32 CTSSTA_BITPOS		= 0;
	const 	 U32 CTSSTA_MASK		= (0xff<<CTSSTA_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MSTATUS);

	return (CBOOL)(( ReadValue & CTSSTA_MASK ) >> CTSSTA_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get CTS Changed State
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE   (1:HIGH)
 				CFLASE (0: LOW) 
 */
CBOOL	NX_UART_GetCTSLevel( U32 ModuleIndex )
{
	const 	 U32 CTS_BITPOS		= 0;
	const 	 U32 CTS_MASK		= (0xff<<CTS_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MSTATUS);

	return (CBOOL)(( ReadValue & CTS_MASK ) >> CTS_BITPOS );
}

// TX/RX Status Register
//-------------------------------------------------------------------
/**
 *	@brief		Get Transmit DMA FSM State
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ). 				
 *	@return		(0x1:IDLE, 0x2:Burst Request, 0x3:Burst next, 0x4:Single Request  )
 				(0x5:Single Acknowledgement, 0x6:Single Next, 0x7: Last Burst Request)
 				(0x9:Last Single Request, 0x10: Last Single Acknowledgemnet)
 */
NX_UART_DMA_FSM 	NX_UART_GetTxDMAFSMState( U32 ModuleIndex )
{
	const 	 U32 TXDMASTA_BITPOS		= 12;
	const 	 U32 TXDMASTA_MASK			= (0xF << TXDMASTA_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS);

	return (NX_UART_DMA_FSM)(( ReadValue & TXDMASTA_MASK ) >> TXDMASTA_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Receive DMA FSM State
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		(0x1:IDLE, 0x2:Burst Request, 0x3:Burst next, 0x4:Single Request  )
 				(0x5:Single Acknowledgement, 0x6:Single Next, 0x7: Last Burst Request)
 				(0x9:Last Single Request, 0x10: Last Single Acknowledgemnet)
 */
NX_UART_DMA_FSM 	NX_UART_GetRxDMAFSMState( U32 ModuleIndex )
{
	const 	 U32 TXDMASTA_BITPOS		= 8;
	const 	 U32 TXDMASTA_MASK			= (0xF << TXDMASTA_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS);

	return (NX_UART_DMA_FSM)(( ReadValue & TXDMASTA_MASK ) >> TXDMASTA_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set Receive TimeOut Status
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 
 *	@return		None
 */

void	NX_UART_SetRxTimeOutStatus( U32 ModuleIndex, CBOOL Status )		//Clear
{
	const U32 TOSTA_BITPOS			= 3;
	const U32 TOSTA_MASK			= (1 << TOSTA_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (0 == Status) && (1 == Status) );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS);
	ReadValue &= ~TOSTA_MASK;
	ReadValue |= (Status << TOSTA_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS, ReadValue);

}

//-------------------------------------------------------------------
/**
 *	@brief		Get Receive TimeOut Status
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		
 */

CBOOL	NX_UART_GetRxTimeOutStatus( U32 ModuleIndex )
{
	const 	 U32 TOSTA_BITPOS		= 3;
	const 	 U32 TOSTA_MASK			= (1 << TOSTA_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS);

	return (CBOOL)(( ReadValue & TOSTA_MASK ) >> TOSTA_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Transmit Empry
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Empty)
 				CFALSE(0: Not Empty)
 */

CBOOL	NX_UART_GetTransmitterEmpty( U32 ModuleIndex )
{
	const U32 TREMPTY_BITPOS		= 2;
	const U32 TREMPTY_MASK			= (1<<TREMPTY_BITPOS);

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)(( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS) & TREMPTY_MASK ) >> TREMPTY_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Transmit Buffer Empry
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Empty)
 				CFALSE(0: Not Empty)
 */
CBOOL	NX_UART_GetTxBufferEmpty( U32 ModuleIndex )
{
	const 	 U32 TRBEMPTY_BITPOS	= 1;
	const 	 U32 TRBEMPTY_MASK		= (1<<TRBEMPTY_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue =	ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS);

	return (CBOOL)(( ReadValue & TRBEMPTY_MASK ) >> TRBEMPTY_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Receive Buffer Empry
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Empty)
 				CFALSE(0: Not Empty)	
 */
CBOOL	NX_UART_GetRxBufferEmpty( U32 ModuleIndex )
{ 
	const 	 U32 TRBEMPTY_BITPOS	= 0;
	const 	 U32 TRBEMPTY_MASK		= (1<<TRBEMPTY_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue =	ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USTATUS);

	return (CBOOL)( !(( ReadValue & TRBEMPTY_MASK ) >> TRBEMPTY_BITPOS ) );
}

// Rx Error Status Register
//-------------------------------------------------------------------
/**
 *	@brief		Get Error Status
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Error Generate)
 				CFALSE(0: No Error)
 */
U32	NX_UART_GetErrorStatus( U32 ModuleIndex )
{
	const 	 U32 ERR_MASK		= ( 0xF << 0 );
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->ESTATUS);

	return (U32)( ReadValue & ERR_MASK );
}


//-------------------------------------------------------------------
/**
 *	@brief		Get Transmit FIFO Full
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Tx FIFO Full)
 				CFALSE(0: Tx FIFO Not Full)	
 */
CBOOL	NX_UART_GetTxFIFOFull( U32 ModuleIndex )
{
	const 	 U32 TXFF_BITPOS	= 24;
	const 	 U32 TXFF_MASK		= (1<<TXFF_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FSTATUS);

	return (CBOOL)(( ReadValue & TXFF_MASK ) >> TXFF_BITPOS );
}
//-------------------------------------------------------------------
/**
 *	@brief		Get Transmit FIFO Count
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		Transmit FIFO Count ( 0 ~ 256 Bytes)	
 */
U8		NX_UART_GetTxFIFOCount( U32 ModuleIndex )
{
	const 	 U32 TXFIFOCNT_BITPOS	= 16;
	const 	 U32 TXFIFOCNT_MASK		= (0xff<<TXFIFOCNT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FSTATUS);

	return (U8)(( ReadValue& TXFIFOCNT_MASK ) >> TXFIFOCNT_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Receive FIFO Error
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Rx FIFO Error) -> ( Frame Error, Parity Error, Break Signal )
 				CFALSE(0: Rx FIFO Not Full)	
 */
CBOOL	NX_UART_GetRxFIFOError( U32 ModuleIndex )
{
	const 	 U32 RXFIFOERR_BITPOS	= 9;
	const 	 U32 RXFIFOERR_MASK		= (1<<RXFIFOERR_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FSTATUS);

	return (CBOOL)(( ReadValue & RXFIFOERR_MASK ) >> RXFIFOERR_BITPOS );
}


//-------------------------------------------------------------------
/**
 *	@brief		Get Receive FIFO Full
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		CTRUE (1: Rx FIFO Full)
 				CFALSE(0: Rx FIFO Not Full)		
 */
CBOOL	NX_UART_GetRxFIFOFull( U32 ModuleIndex )
{
	const 	 U32 RXFF_BITPOS	= 8;
	const 	 U32 RXFF_MASK		= (1<<RXFF_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FSTATUS);

	return (CBOOL)(( ReadValue & RXFF_MASK ) >> RXFF_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Receive FIFO Count
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).			
 *	@return		Receive FIFO Count ( 0 ~ 256 Bytes)	
 */
U8		NX_UART_GetRxFIFOCount( U32 ModuleIndex )
{
	const 	 U32 RXFIFOCNT_BITPOS	= 0;
	const 	 U32 RXFIFOCNT_MASK		= (0xff<<RXFIFOCNT_BITPOS);
	register U32 ReadValue;

	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	ReadValue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FSTATUS);

	return (U8)(( ReadValue & RXFIFOCNT_MASK ) >> RXFIFOCNT_BITPOS );
}

// Tx & Rx Buffer Register
//-------------------------------------------------------------------
/**
 *	@brief		Send a data to be transmitted.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Data	a data to be transmitted.
 *	@return		None
 */
void	NX_UART_SendByte( U32 ModuleIndex, U8 Data )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THR, (U8)Data);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get a received data
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Value of Received Data
 */
U8		NX_UART_GetByte( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U8)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->RBR) & 0xFF);
}

//-------------------------------------------------------------------
/**
*  @brief	    Set a BRD(Baud Rate Division) value.
*  @param[in]  ModuleIndex	   An index of module ( 0 ~ x ).
*  @param[in]  BRD Baud rate division value, It must be greater than 0.
*  @return	     None.
*  @remarks    The following fomula can determine the BRD\r\n
*/
void	NX_UART_SetBRD( U32 ModuleIndex, U16 BRD )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0xFFFF > BRD );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BRDR, BRD);
}

//-------------------------------------------------------------------
/**
 *  @brief 	 	Get BRD(Baud Rate Divison) Value
 *  @param[in]  ModuleIndex	 An index of module ( 0 ~ x ).
 *  @return	 	Baud Rate Divison Value ( 0x0 ~ 0xFFFF )
 */
U16		NX_UART_GetBRD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U16)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->BRDR));
}

//-------------------------------------------------------------------
/**
 *  @brief 	 	Set FBRD (Divisor Factional Value)
 *  @param[in]  ModuleIndex	 An index of module ( 0 ~ x ).
 *  @return	 	None
 */
void	NX_UART_SetFBRD( U32 ModuleIndex, U8 FBRD )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0xF >= FBRD );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FRACVAL, FBRD);
}

//-------------------------------------------------------------------
/**
  *  @brief 	 	Get FBRD (Divisor Factional Value)
  *  @param[in]  ModuleIndex	 An index of module ( 0 ~ x ).
  *  @return	 	Divisor Factional Value ( 0x0 ~ 0xF )
*/
U8		NX_UART_GetFBRD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_UART_CHANNEL > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U8)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->FRACVAL));
}
//@}

//-------------------------------------------------------------------
/**
  *  @brief 	 	Make FBRD (Making Divisor Value)
  *  @param[in]  Baud Rate 
  *  @param[in]  Input Clock (0 ~ x ).
  *  @return	 	Divisor Value ( 0x0 ~ 0xF )
*/
U16		NX_UART_MakeBRD( U32 BaudRate, U32 CLKinHz )
{
	NX_ASSERT( 0 < BaudRate );
	NX_ASSERT( 0 < CLKinHz );

	CLKinHz = (CLKinHz / (BaudRate * 16)) - 1;

	NX_ASSERT( (0x0000FFFF >= CLKinHz) && (0 < CLKinHz) );
	return (U16)CLKinHz;
}


//-------------------------------------------------------------------
/**
  *  @brief 	 	Make FBRD (Making Divisor Factional Value)
  *  @param[in]  Baud Rate 
  *  @param[in]  Input Clock (0 ~ x ).
  *  @return	 	Divisor Value ( 0x0 ~ 0xF )
*/
U8		NX_UART_MakeFBRD( U32 BaudRate, U32 CLKinHz )
{
	NX_ASSERT( 0 < BaudRate );
	NX_ASSERT( 0 < CLKinHz );

	CLKinHz %= (BaudRate * 16 + 16/2);	// 0.xxxxxx
	CLKinHz <<= 4;	// mul by 64 (0.xxxxxx * 64 = xx.xxxxxx)
	CLKinHz += 32;	// + 0.5 (32/64) ( half add )
	CLKinHz /= (BaudRate * 16 + 16/2); // xx.000000

	NX_ASSERT( (0x000000F >= CLKinHz) );
	
	return (U8)CLKinHz;
}

