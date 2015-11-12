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
//	Module		: AC97
//	File		       : nx_ac97.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120802 first implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_ac97.h>

static	struct
{
	struct NX_AC97_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_AC97_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicates that Initialize is successed.
 *			CFALSE indicates that Initialize is failed.
 *	@see	NX_AC97_GetNumberOfModule
 */
CBOOL	NX_AC97_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_AC97_MODULE; i++ )
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
 *	@see		NX_AC97_Initialize
 */
U32		NX_AC97_GetNumberOfModule( void )
{
	return NUMBER_OF_AC97_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_AC97_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( AC97 ) }; // PHY_BASEADDR_AC97?_MODULE

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
U32 NX_AC97_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( AC97, PRESETn )
	};
	NX_CASSERT( NUMBER_OF_AC97_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_AC97_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_AC97_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_AC97_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_AC97_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_AC97_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 */
void*	NX_AC97_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Initialize is successed. 
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_AC97_OpenModule( U32 ModuleIndex )
{
	register struct NX_AC97_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIO32(&pRegister->GLBCTRL) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->GLBSTAT) == 0x1 );
	NX_ASSERT( ReadIO32(&pRegister->CODEC_CMD) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->CODEC_STAT) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->PCMADDR) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->MICADDR) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->PCMDATA) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->MICDATA) == 0x0 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Deinitialize is successed. 
 *				 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_AC97_CloseModule( U32 ModuleIndex )
{
	register struct NX_AC97_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIO32(&pRegister->GLBCTRL, 0x0);
	WriteIO32(&pRegister->GLBSTAT, 0x1);
	WriteIO32(&pRegister->CODEC_CMD, 0x0);
	WriteIO32(&pRegister->CODEC_STAT, 0x0);
	WriteIO32(&pRegister->PCMADDR, 0x0);
	WriteIO32(&pRegister->MICADDR, 0x0);
	WriteIO32(&pRegister->PCMDATA, 0x0);
	WriteIO32(&pRegister->MICDATA, 0x0);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Module is Busy. 
 *				 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_AC97_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that Ready to enter power-down stage. 
 *				 CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_AC97_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 */
U32		NX_AC97_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	AC97InterruptNumber[NUMBER_OF_AC97_MODULE] =
				{
					INTNUM_LIST(AC97)
				};

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return	AC97InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@param[in]	Enable	 CTRUE	indicates that Interrupt Enable. 
 *						 CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_AC97_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	16;
	const U32	PEND_MASK	=	0x7F << PEND_POS;

	register struct NX_AC97_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIO32(&pRegister->GLBCTRL) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIO32(&pRegister->GLBCTRL, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		 CTRUE	indicates that Interrupt is enabled. 
 *				 CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_AC97_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	16;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBCTRL) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		 CTRUE	indicates that Pending is seted. 
 *				 CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_AC97_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	16;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBSTAT) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 */
void	NX_AC97_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	24;
	const U32	PEND_MASK	=	0x7F << PEND_POS;

	register struct NX_AC97_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( 7 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->GLBCTRL, ((1 << IntNum) & PEND_MASK) << PEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	 CTRUE	indicates that Set to all interrupt enable. 
 *						 CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_AC97_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	PEND_POS	=	16;
	const U32	INT_MASK	=	0x7F;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK << PEND_POS;
	}

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBCTRL, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		 CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				 CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_AC97_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	16;
	const U32	INT_MASK	=	0x7F;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBCTRL) & (INT_MASK << PEND_POS) )
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
CBOOL	NX_AC97_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	16;
	const U32	PEND_MASK	=	0x7F;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBSTAT) & (PEND_MASK << PEND_POS) )
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
void	NX_AC97_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	24;
	const U32	PEND_MASK	=	0x7F << PEND_POS;

	register struct NX_AC97_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->GLBCTRL, (PEND_MASK<<PEND_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
U32		NX_AC97_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	16;
	const U32	PEND_MASK	=	0x7F << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_AC97_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	(ReadIO32(&pRegister->GLBSTAT)>>PEND_POS) & PEND_MASK;

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
U32		NX_AC97_GetDMAIndex_PCMout( U32 ModuleIndex )
{
	const U32 AC97DmaIndexPCMout[NUMBER_OF_AC97_MODULE] =
				{ DMAINDEX_WITH_CHANNEL_LIST(AC97, PCMOUTDMA)  };

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return AC97DmaIndexPCMout[ModuleIndex];
}

U32		NX_AC97_GetDMAIndex_PCMin( U32 ModuleIndex )
{
	const U32 AC97DmaIndexPCMin[NUMBER_OF_AC97_MODULE] =
				{ DMAINDEX_WITH_CHANNEL_LIST(AC97, PCMINDMA)  };

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return AC97DmaIndexPCMin[ModuleIndex];
}

U32		NX_AC97_GetDMAIndex_MICin( U32 ModuleIndex )
{
	const U32 AC97DmaIndexMICin[NUMBER_OF_AC97_MODULE] =
				{ DMAINDEX_WITH_CHANNEL_LIST(AC97, MICINDMA)  };

	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return AC97DmaIndexMICin[ModuleIndex];
}

U32		NX_AC97_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );

	return 32;
}

//--------------------------------------------------------------------------
// Configuration operations
//--------------------------------------------------------------------------
void	NX_AC97_SetGLBCTRL( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBCTRL, value);
}
U32		NX_AC97_GetGLBCTRL( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBCTRL));
}

U32		NX_AC97_GetGLBSTAT( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->GLBSTAT));
}

void	NX_AC97_SetCODEC_CMD( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CODEC_CMD, value);
}
U32		NX_AC97_GetCODEC_CMD( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CODEC_CMD));
}

U32		NX_AC97_GetCODEC_STAT( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CODEC_STAT));
}

U32		NX_AC97_GetPCMADDR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->PCMADDR));
}

U32		NX_AC97_GetMICADDR( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MICADDR));
}

void	NX_AC97_SetPCMDATA( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->PCMDATA, value);
}
U32		NX_AC97_GetPCMDATA( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->PCMDATA));
}

void	NX_AC97_SetMICDATA( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MICDATA, value);
}
U32		NX_AC97_GetMICDATA( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_AC97_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->MICDATA));
}


