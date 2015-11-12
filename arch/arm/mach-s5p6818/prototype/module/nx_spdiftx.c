//------------------------------------------------------------------------------
//
//	Copyright (C) 2012 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: SPDIFTX
//	File			: nx_spdiftx.c
//	Description	:
//	Author		: Deoks
//	History		: 2014.10.16 First implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_spdiftx.h>

static	struct
{
	struct NX_SPDIFTX_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_SPDIFTX_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicates that Initialize is successed.
 *			CFALSE indicates that Initialize is failed.
 *	@see	NX_SPDIFTX_GetNumberOfModule
 */
CBOOL	NX_SPDIFTX_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_SPDIFTX_MODULE; i++ )
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
 *	@see		NX_SPDIFTX_Initialize
 */
U32		NX_SPDIFTX_GetNumberOfModule( void )
{
	return NUMBER_OF_SPDIFTX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_SPDIFTX_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SPDIFTX ) };

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_SPDIFTX_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_SPDIFTX_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_SPDIFTX_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SPDIFTX_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 */
void*	NX_SPDIFTX_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Initialize is successed. 
 *				CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_SPDIFTX_OpenModule( U32 ModuleIndex )
{
	register struct NX_SPDIFTX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_CLKCON) == 0x2 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_CON) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_BSTAS) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_CSTAS) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_DAT) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_CNT) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_BSTAS_SHD) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_CNT_SHD) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->USERBIT1) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->USERBIT2) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->USERBIT3) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->USERBIT1_SHD) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->USERBIT2_SHD) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->USERBIT3_SHD) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->VERSION_INFO) == 0xD );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Deinitialize is successed. 
 *				 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_SPDIFTX_CloseModule( U32 ModuleIndex )
{
	register struct NX_SPDIFTX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIO32(&pRegister->SPDIF_CLKCON, 0x2);
	WriteIO32(&pRegister->SPDIF_CON, 0x0);
	WriteIO32(&pRegister->SPDIF_BSTAS, 0x0);
	WriteIO32(&pRegister->SPDIF_CSTAS, 0x0);
	WriteIO32(&pRegister->SPDIF_DAT, 0x0);
	WriteIO32(&pRegister->SPDIF_CNT, 0x0);
	WriteIO32(&pRegister->SPDIF_BSTAS_SHD, 0x0);
	WriteIO32(&pRegister->SPDIF_CNT_SHD, 0x0);
	WriteIO32(&pRegister->USERBIT1, 0x0);
	WriteIO32(&pRegister->USERBIT2, 0x0);
	WriteIO32(&pRegister->USERBIT3, 0x0);
	WriteIO32(&pRegister->USERBIT1_SHD, 0x0);
	WriteIO32(&pRegister->USERBIT2_SHD, 0x0);
	WriteIO32(&pRegister->USERBIT3_SHD, 0x0);
	WriteIO32(&pRegister->VERSION_INFO, 0xD);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Module is Busy. 
 *				CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_SPDIFTX_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Ready to enter power-down stage. 
 *				CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_SPDIFTX_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return CTRUE;
}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_SPDIFTX_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_SPDIFTX?_MODULE in <nx_chip.h>.
 */
U32 NX_SPDIFTX_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( SPDIFTX )
	};
	NX_CASSERT( NUMBER_OF_SPDIFTX_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_SPDIFTX?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_SPDIFTX_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
	const U32 ResetPinNumber[2][NUMBER_OF_SPDIFTX_MODULE] =
	{
		{ RESETINDEX_LIST( SPDIFTX, PRESETn )}
	};

	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 */
U32		NX_SPDIFTX_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	SPDIFTXInterruptNumber[NUMBER_OF_SPDIFTX_MODULE] =
				{
					INTNUM_LIST(SPDIFTX)
				};

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return	SPDIFTXInterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@param[in]	Enable	CTRUE	indicates that Interrupt Enable.
 *						CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_SPDIFTX_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x8540 << PEND_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIO32(&pRegister->SPDIF_CON) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIO32(&pRegister->SPDIF_CON, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that Interrupt is enabled. 
 *				CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_SPDIFTX_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that Pending is seted. 
 *				CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_SPDIFTX_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	//NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 */
void	NX_SPDIFTX_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80 << PEND_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->SPDIF_CON, ((1 << IntNum) & PEND_MASK) << PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	CTRUE	indicates that Set to all interrupt enable. 
 *						CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_SPDIFTX_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0x8540;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK << PEND_POS;
	}

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_SPDIFTX_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0x8540;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) & (INT_MASK << PEND_POS) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that At least one( or more ) pending is seted. 
 *				CFALSE	indicates that All pending is NOT seted.
 */
CBOOL	NX_SPDIFTX_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON) & (PEND_MASK << PEND_POS) )
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
void	NX_SPDIFTX_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80 << PEND_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->SPDIF_CON, (PEND_MASK<<PEND_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
U32		NX_SPDIFTX_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x10A80 << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_SPDIFTX_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	(ReadIO32(&pRegister->SPDIF_CON)>>PEND_POS) & PEND_MASK;

	for( PendingIndex=0 ; PendingIndex<=20 ; PendingIndex++)
		if(Pend & ((U32)0x1)<<PendingIndex)
			break;

	if(PendingIndex > 20)
		return -1;
	else
		return PendingIndex;
}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
U32		NX_SPDIFTX_GetDMAIndex( U32 ModuleIndex )
{
	const U32 SPDIFTXDmaIndex[NUMBER_OF_SPDIFTX_MODULE] =
				{ DMAINDEX_LIST(SPDIFTX)  };

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return SPDIFTXDmaIndex[ModuleIndex];
}

U32		NX_SPDIFTX_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	return 32;
}


//--------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set Main Audio Clock Select.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	MainClk			Main Audio Clock 
 								( 0 : Internal Clock, 1: External Clock )
 *	@return		None.
 */
void	NX_SPDIFTX_SetMCLK( U32 ModuleIndex, NX_SPDIFTX_MCLK MainClk )
{
	const U32 MCLK_POS	=	2;
	const U32 MCLK_MASK	=	1UL << MCLK_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (NX_SPDIFTX_INTERNAL_CLK == MainClk) || (NX_SPDIFTX_EXTERNAL_CLK == MainClk) );

	regvalue 	 = ReadIO32(&pRegister->SPDIF_CLKCON);
	regvalue	&= ~MCLK_MASK;
	regvalue	|= MainClk << MCLK_POS;

	WriteIO32(&pRegister->SPDIF_CLKCON, regvalue);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get Main Audio Clock Select.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		Presclaer		(0:Internal Clock, 1:External Clock)
 */
NX_SPDIFTX_MCLK		NX_SPDIFTX_GetMCLK( U32 ModuleIndex )
{
	const U32 MCLK_POS	=	2;
	const U32 MCLK_MASK	=	1UL << MCLK_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (NX_SPDIFTX_MCLK)((ReadIO32(&pRegister->SPDIF_CLKCON) & MCLK_MASK) >> MCLK_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Clock Down Ready.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		Clock Down Ready ( 0 : Not Ready, 1: Ready )
 */

CBOOL	NX_SPDIFTX_GetClockDownReady( U32 ModuleIndex )
{
	const U32 RDY_POS	=	1;
	const U32 RDY_MASK	=	1UL << RDY_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)((ReadIO32(&pRegister->SPDIF_CLKCON) & RDY_MASK) >> RDY_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set SPDIFTX Power On or Off.
 *	@param[in]	ModuleIndex		An index of module.
 *	@param[in]	PowerOn		Power On/Off ( 0 : Power Off, 1: Power On )
 *	@return		None.
 */
void	NX_SPDIFTX_SetPowerOn( U32 ModuleIndex, CBOOL PowerOn )
{
	const U32 POWER_POS		=	0;
	const U32 POWER_MASK	=	1UL << POWER_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (CTRUE == PowerOn) || (CFALSE == PowerOn) );

	regvalue 	 = ReadIO32(&pRegister->SPDIF_CLKCON);
	regvalue	&= ~POWER_MASK;
	regvalue	|= PowerOn << POWER_POS;

	WriteIO32(&pRegister->SPDIF_CLKCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Power On.
 *	@param[in]	ModuleIndex		An index of module.
 *	@return		Clock Down Ready ( 0 : Not Ready, 1: Ready )
 */

CBOOL	NX_SPDIFTX_GetPowerOn( U32 ModuleIndex )
{
	const U32 POWER_POS		=	0;
	const U32 POWER_MASK	=	1UL << POWER_POS;

	register struct NX_SPDIFTX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (CBOOL)((ReadIO32(&pRegister->SPDIF_CLKCON) & POWER_MASK) >> POWER_POS);
}


//-----------------------------------------------------------------------------


// FIFO Related Function.
//------------------------------------------------------------------------------
/**
 *	@brief		Check Tx FIFO is Empty or Not
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE 			Indicate that FIFO Empty.
 *				CFALSE 			Indicate that FIFO NOT Empty
 */
CBOOL	NX_SPDIFTX_IsFIFOEmpty(U32 ModuleIndex)
{
	const U32 FLVL_POS	= 22;
	const U32 FLVL_MASK	= 0x1F << FLVL_POS;
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32 FIFO_LVL;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	FIFO_LVL = (ReadIO32(&pRegister->SPDIF_CON) & FLVL_MASK) >> FLVL_POS;

	if( 0 == FIFO_LVL )
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
CBOOL	NX_SPDIFTX_IsFIFOFull(U32 ModuleIndex)
{
	const U32 FLVL_POS	= 22;
	const U32 FLVL_MASK	= 0x1F << FLVL_POS;
	const U32 FLVL_MAX	= 16;
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32 FIFO_LVL;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	FIFO_LVL = (ReadIO32(&pRegister->SPDIF_CON) & FLVL_MASK) >> FLVL_POS;

	if( FLVL_MAX == FIFO_LVL )
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
U32		NX_SPDIFTX_GetFIFOLVL(U32 ModuleIndex)
{
	const U32 FLVL_POS	= 22;
	const U32 FLVL_MASK	= 0x1F << FLVL_POS;
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

	return (ReadIO32(&pRegister->SPDIF_CON) & FLVL_MASK) & FLVL_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Tx FIFO Trigger Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	TX_RDY_LVEL	Set Trigger Level
 				(0 : 0Depth, 1: 1Depth, 2: 4Depth, 3:6Depth, 4:10Depth, 5:12Depth, 6:14Depth, 7:15Depth)
 *	@return		None
 *	@remark	
 */
void	NX_SPDIFTX_SetTriggerLevel( U32 ModuleIndex, NX_SPDIFTX_TRIGGER_LEVEL TX_RDY_LVL )
{
	const U32 FIFO_RDY_POS	= 19;
	const U32 FIFO_RDY_MASK	= 7UL << FIFO_RDY_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x7 >= TX_RDY_LVL );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~FIFO_RDY_MASK;
	regvalue	|= TX_RDY_LVL << FIFO_RDY_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx FIFO Trigger Level
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Get Tx Trgger Level
 				(0 : 0Depth, 1: 1Depth, 2: 4Depth, 3:6Depth, 4:10Depth, 5:12Depth, 6:14Depth, 7:15Depth)
 */
NX_SPDIFTX_TRIGGER_LEVEL  	NX_SPDIFTX_GetTriggerLevel( U32 ModuleIndex )
{
	const U32 FIFO_RDY_POS	= 19;
	const U32 FIFO_RDY_MASK	= 7UL << FIFO_RDY_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_SPDIFTX_TRIGGER_LEVEL)((ReadIO32(&pRegister->SPDIF_CON) & FIFO_RDY_MASK) >> FIFO_RDY_POS);
}


//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Set SPDIFTX Transfer Mode Select.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Mode			SPDIFTX Mode Select.
 								( 0 : DMA, 1: POLLING, 2: INTERRUPT )
 *	@return		None	
 */
void	NX_SPDIFTX_SetTransferMode( U32 ModuleIndex, NX_SPDIFTX_TRANSFER_MODE Mode )
{
	const U32 MODE_POS	= 17;
	const U32 MODE_MASK	= 3UL << MODE_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3 >= Mode );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~MODE_MASK;
	regvalue	|= Mode << MODE_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Tx FIFO Trigger Level.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Get Tx Trgger Level
 				(0 : 0Depth, 1: 1Depth, 2: 4Depth, 3:6Depth, 4:10Depth, 5:12Depth, 6:14Depth, 7:15Depth)
 */
NX_SPDIFTX_TRANSFER_MODE  	NX_SPDIFTX_GetTransferMode( U32 ModuleIndex )
{
	const U32 MODE_POS	= 17;
	const U32 MODE_MASK	= 3UL << MODE_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_SPDIFTX_TRANSFER_MODE)((ReadIO32(&pRegister->SPDIF_CON) & MODE_MASK) >> MODE_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Endian Format Select.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Mode			Endian Format Select. ( 0 : Big Endian, 1: 4Byte, 2:3Byte, 3:2Byte )
 *	@return		None	
 */
void	NX_SPDIFTX_SetEndianFormat( U32 ModuleIndex, NX_SPDIFTX_SWAP Swap )
{
	const U32 ENDIAN_POS	= 13;
	const U32 ENDIAN_MASK	= 3UL << ENDIAN_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3 >= Swap );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~ENDIAN_MASK;
	regvalue	|= Swap << ENDIAN_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get Endian Format Select.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Endian Format Select. ( 0 : Big Endian, 1: 4Byte, 2:3Byte, 3:2Byte )
 */
NX_SPDIFTX_SWAP  NX_SPDIFTX_GetEndianFormat( U32 ModuleIndex )
{
	const U32 ENDIAN_POS	= 13;
	const U32 ENDIAN_MASK	= 3UL << ENDIAN_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_SPDIFTX_SWAP)((ReadIO32(&pRegister->SPDIF_CON) & ENDIAN_MASK) >> ENDIAN_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set User Data Attach.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	UserData		(0:User Data is stored in USERBIT Register )
 								(1:USer Data is stored in 23rd bit of Aduio data.)
 *	@return		None	
 */
void	NX_SPDIFTX_SetUserData( U32 ModuleIndex, CBOOL UserData )
{
	const U32 USERDATA_POS	= 12;
	const U32 USERDATA_MASK	= 1UL << USERDATA_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (CTRUE == UserData) || (CFALSE == UserData) );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~USERDATA_MASK;
	regvalue	|= UserData << USERDATA_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get User Data Attach.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		(0:User Data is stored in USERBIT Register )
 				(1:USer Data is stored in 23rd bit of Aduio data.)
 */
CBOOL  NX_SPDIFTX_GetUserData( U32 ModuleIndex )
{
	const U32 USERDATA_POS	= 12;
	const U32 USERDATA_MASK	= 1UL << USERDATA_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((ReadIO32(&pRegister->SPDIF_CON) & USERDATA_MASK) >> USERDATA_POS);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set Software Reset.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	CTRUE	Indicate that Software Reset.
 						CFALSE Indicate that Software Normal Operation.
 *	@return		None	
 */
void	NX_SPDIFTX_SetSoftwareReset( U32 ModuleIndex, CBOOL Enable )
{
	const U32 RESET_POS		= 5;
	const U32 RESET_MASK	= 1UL << RESET_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~RESET_MASK;
	regvalue	|= Enable << RESET_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Software Reset.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		CTRUE			Indicate that Software Reset.
 				CFALSE 			Indicate that Software Normal Operation.
 */
CBOOL  NX_SPDIFTX_GetSoftwareReset( U32 ModuleIndex )
{
	const U32 RESET_POS		= 5;
	const U32 RESET_MASK	= 1UL << RESET_POS; 

	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((ReadIO32(&pRegister->SPDIF_CON) & RESET_MASK) >> RESET_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Main Audio Clock Sampling Frequency Select.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Sample			Sampling Frequency Select ( 0: 256fs, 1: 384fs, 2:512fs )
 *	@return		None	
 */
void	NX_SPDIFTX_SetMCLKSamplingSel( U32 ModuleIndex, NX_SPIDFTX_MCLK_SAMPLE Sample )
{
	const U32 SAMPLE_POS	= 3;
	const U32 SAMPLE_MASK	= 3UL << SAMPLE_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3 > Sample );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~SAMPLE_MASK;
	regvalue	|= Sample << SAMPLE_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Main Audio Clock Sampling Frequency Select.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		Sampling Frequency Select ( 0: 256fs, 1: 384fs, 2:512fs )
 */
NX_SPIDFTX_MCLK_SAMPLE  NX_SPDIFTX_GetMCLKSamplingSel( U32 ModuleIndex )
{
	const U32 SAMPLE_POS	= 3;
	const U32 SAMPLE_MASK	= 3UL << SAMPLE_POS; 

	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_SPIDFTX_MCLK_SAMPLE)((ReadIO32(&pRegister->SPDIF_CON) & SAMPLE_MASK) >> SAMPLE_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set PCM Data Size.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	DataBit			PCM Data Size ( 0: 16bit, 1: 20bit, 2:24bit )
 *	@return		None	
 */
void	NX_SPDIFTX_SetBitWidth( U32 ModuleIndex, NX_SPDIFTX_DATA_BIT DataBit )
{
	const U32 DATA_BIT_POS	= 1;
	const U32 DATA_BIT_MASK	= 3UL << DATA_BIT_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( 0x3 >= DataBit );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~DATA_BIT_MASK;
	regvalue	|= DataBit << DATA_BIT_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get PCM Data Size.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		PCM Data Size ( 0: 16bit, 1: 20bit, 2:24bit )
 */
NX_SPDIFTX_DATA_BIT  NX_SPDIFTX_GetBitWidth( U32 ModuleIndex )
{
	const U32 DATA_BIT_POS	= 1;
	const U32 DATA_BIT_MASK	= 3UL << DATA_BIT_POS; 

	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (NX_SPDIFTX_DATA_BIT)((ReadIO32(&pRegister->SPDIF_CON) & DATA_BIT_MASK) >> DATA_BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set PCM or Stream
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	DataBit			PCM or Steam ( 0: PCM, 1: Stream )
 *	@return		None	
 */
void	NX_SPDIFTX_SetTransferType( U32 ModuleIndex, CBOOL Type )
{
	const U32 TYPE_POS	= 0;
	const U32 TYPE_MASK	= 1UL << TYPE_POS; 
	
	register struct NX_SPDIFTX_RegisterSet *pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (CTRUE == Type) || (CFALSE == Type) );

	regvalue	 = ReadIO32(&pRegister->SPDIF_CON);
	regvalue	&= ~TYPE_MASK;
	regvalue	|= Type << TYPE_POS;
	
	WriteIO32( &pRegister->SPDIF_CON, regvalue );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get PCM Data Size.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		PCM Data Size ( 0: 16bit, 1: 20bit, 2:24bit )
 */
CBOOL  NX_SPDIFTX_GetTransferType( U32 ModuleIndex )
{
	const U32 TYPE_POS	= 0;
	const U32 TYPE_MASK	= 1UL << TYPE_POS; 

	register struct NX_SPDIFTX_RegisterSet *pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (CBOOL)((ReadIO32(&pRegister->SPDIF_CON) & TYPE_MASK) >> TYPE_POS);
}


//--------------------------------------------------------------------------
// Configuration operations
//--------------------------------------------------------------------------
void	NX_SPDIFTX_SetSPDCLKCON( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CLKCON, value);
}
U32		NX_SPDIFTX_GetSPDCLKCON( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CLKCON));
}

void	NX_SPDIFTX_SetSPDCON( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON, value);
}
U32		NX_SPDIFTX_GetSPDCON( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CON));
}

void	NX_SPDIFTX_SetSPDBSTAS( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_BSTAS, value);
}
U32		NX_SPDIFTX_GetSPDBSTAS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_BSTAS));
}

void	NX_SPDIFTX_SetSPDCSTAS( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CSTAS, value);
}
U32		NX_SPDIFTX_GetSPDCSTAS( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CSTAS));
}

void	NX_SPDIFTX_SetSPDDAT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_DAT, (value & 0xFFFFFF) );
}

void	NX_SPDIFTX_SetSPDCNT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CNT, value );
}

U32		NX_SPDIFTX_GetSPDBSTAS_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_BSTAS_SHD));
}

U32		NX_SPDIFTX_GetSPDCNT_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CNT_SHD));
}

void	NX_SPDIFTX_SetUSERBIT1( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT1, value);
}
U32		NX_SPDIFTX_GetUSERBIT1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT1));
}

void	NX_SPDIFTX_SetUSERBIT2( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT2, value);
}
U32		NX_SPDIFTX_GetUSERBIT2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT2));
}

void	NX_SPDIFTX_SetUSERBIT3( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT3, value);
}
U32		NX_SPDIFTX_GetUSERBIT3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT3));
}

U32		NX_SPDIFTX_GetUSERBIT1_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT1_SHD));
}

U32		NX_SPDIFTX_GetUSERBIT2_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT2_SHD));
}

U32		NX_SPDIFTX_GetUSERBIT3_SHD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->USERBIT3_SHD));
}

U32		NX_SPDIFTX_GetVERSION_INFO( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFTX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->VERSION_INFO));
}































