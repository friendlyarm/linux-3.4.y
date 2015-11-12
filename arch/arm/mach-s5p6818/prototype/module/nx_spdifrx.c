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
//	Module		: SPDIFRX
//	File		: nx_spdifrx.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120830 first implementation
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_spdifrx.h>

static	struct
{
	struct NX_SPDIFRX_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_SPDIFRX_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicates that Initialize is successed.
 *			CFALSE indicates that Initialize is failed.
 *	@see	NX_SPDIFRX_GetNumberOfModule
 */
CBOOL	NX_SPDIFRX_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_SPDIFRX_MODULE; i++ )
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
 *	@see		NX_SPDIFRX_Initialize
 */
U32		NX_SPDIFRX_GetNumberOfModule( void )
{
	return NUMBER_OF_SPDIFRX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_SPDIFRX_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( SPDIFRX ) };

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
U32 NX_SPDIFRX_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
{
	const U32 ResetPinNumber[2][NUMBER_OF_SPDIFRX_MODULE] =
	{
		{ RESETINDEX_LIST( SPDIFRX, PRESETn )}
	};

	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_SPDIFRX_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_SPDIFRX_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_SPDIFRX_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_SPDIFRX_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_SPDIFRX_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 */
void*	NX_SPDIFRX_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Initialize is successed. 
 *				CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_SPDIFRX_OpenModule( U32 ModuleIndex )
{
	register struct NX_SPDIFRX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_CTRL) == 0x0 );
	NX_ASSERT( ReadIO32(&pRegister->SPDIF_ENBIRQ) == 0x0 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Deinitialize is successed. 
 *				CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_SPDIFRX_CloseModule( U32 ModuleIndex )
{
	register struct NX_SPDIFRX_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIO32(&pRegister->SPDIF_CTRL, 0x0);
	WriteIO32(&pRegister->SPDIF_ENBIRQ, 0x0);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Module is Busy.
 *				CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_SPDIFRX_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that Ready to enter power-down stage. 
 *				CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_SPDIFRX_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

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
U32		NX_SPDIFRX_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	SPDIFRXInterruptNumber[NUMBER_OF_SPDIFRX_MODULE] =
				{
					INTNUM_LIST(SPDIFRX)
				};

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return	SPDIFRXInterruptNumber[ModuleIndex];
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
void	NX_SPDIFRX_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0xf << PEND_POS;

	register struct NX_SPDIFRX_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIO32(&pRegister->SPDIF_ENBIRQ) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIO32(&pRegister->SPDIF_ENBIRQ, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that Interrupt is enabled. 
 *				CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_SPDIFRX_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that Pending is seted. 
 *				CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_SPDIFRX_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	4;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) >> (IntNum + PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 */
void	NX_SPDIFRX_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf << PEND_POS;

	register struct NX_SPDIFRX_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( 4 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	WriteIO32(&pRegister->SPDIF_ENBIRQ, ((1 << (IntNum + PEND_POS)) & PEND_MASK) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	CTRUE	indicates that Set to all interrupt enable. 
 *						CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_SPDIFRX_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0xf;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK << PEND_POS;
	}

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_SPDIFRX_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0xf;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) & (INT_MASK << PEND_POS) )
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
CBOOL	NX_SPDIFRX_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ) & (PEND_MASK << PEND_POS) )
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
void	NX_SPDIFRX_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf << PEND_POS;
	const U32	ENB_MASK	=	0xf;

	register struct NX_SPDIFRX_RegisterSet*	pRegister;
	register U32 IRQEnb;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	IRQEnb	=	ReadIO32(&pRegister->SPDIF_ENBIRQ) & ENB_MASK;

	WriteIO32(&pRegister->SPDIF_ENBIRQ, (PEND_MASK|IRQEnb) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
U32		NX_SPDIFRX_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	4;
	const U32	PEND_MASK	=	0xf << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_SPDIFRX_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	(ReadIO32(&pRegister->SPDIF_ENBIRQ)>>PEND_POS) & PEND_MASK;

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
U32		NX_SPDIFRX_GetDMAIndex( U32 ModuleIndex )
{
	const U32 SPDIFRXDmaIndex[NUMBER_OF_SPDIFRX_MODULE] = { DMAINDEX_LIST(SPDIFRX)  };

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return SPDIFRXDmaIndex[ModuleIndex];
}

U32		NX_SPDIFRX_GetDMABusWidth( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );

	return 32;
}
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/**
 *	@brief		Set CPU Header
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in] Enable			Setting Value [0] : Disable
 								Setting Value [1] : Enable
 *	@return		None
 *	@remark	
 */
void	NX_SPDIFRX_SetCPUHeader( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 CPU_HEADER_BITPOS	= 16;
	const 	 U32 CPU_HEADER_MASK	= (1UL << CPU_HEADER_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~CPU_HEADER_MASK;
	regvalue |= (Enable << CPU_HEADER_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get CPU Header
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Disable
 				Return Value [1] : Enable
 */
CBOOL	NX_SPDIFRX_GetCPUHeader( U32 ModuleIndex )
{
	const 	 U32 CPU_HEADER_BITPOS	= 16;
	const 	 U32 CPU_HEADER_MASK	= (1UL << CPU_HEADER_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & CPU_HEADER_MASK ) >> CPU_HEADER_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set Decrease Rate
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in] Enable			Setting Value [0] : Disable
 								Setting Value [1] : Enable
 *	@return		None
 *	@remark	
 */
void	NX_SPDIFRX_SetDecreseRate( U32 ModuleIndex, U8 Rate )
{
	const 	 U32 DEC_BITPOS		= 12;
	const 	 U32 DEC_MASK		= (0xF << DEC_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0xF >= Rate );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~DEC_MASK;
	regvalue |= (Rate << DEC_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Decrease Rate
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Disable
 				Return Value [1] : Enable
 */
U8		NX_SPDIFRX_GetDecreseRate( U32 ModuleIndex )
{
	const 	 U32 DEC_BITPOS		= 12;
	const 	 U32 DEC_MASK		= (0xF << DEC_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (U8)(( regvalue & DEC_MASK ) >> DEC_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  User Data Fill
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in] Enable			Setting Value [0] : Disable
 								Setting Value [1] : Enable
 *	@return		None
 */
void	NX_SPDIFRX_SetUserDataFill( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 UDATA_BITPOS	= 11;
	const 	 U32 UDATA_MASK		= (1UL << UDATA_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~UDATA_MASK;
	regvalue |= (Enable << UDATA_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  User Data Fill
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Disable
 				Return Value [1] : Clear
 */
CBOOL		NX_SPDIFRX_GetUserDataFill( U32 ModuleIndex )
{
	const 	 U32 UDATA_BITPOS	= 11;
	const 	 U32 UDATA_MASK		= (1UL << UDATA_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & UDATA_MASK ) >> UDATA_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Capture User Data.
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in]	Setting Value [0] : Disable
 				Setting Value [1] : Enable
 *	@return		None
 *	@remark	When  start of block  pulse  is  TRUE,  RegUserA,  RegUserB, RegStatA, RegStatB 
 				Detected Rx-Data can be captured.
 */
void	NX_SPDIFRX_SetCaptureUserData( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 CAP_UD_BITPOS	= 3;
	const 	 U32 CAP_UD_MASK	= (1UL << CAP_UD_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~CAP_UD_MASK;
	regvalue |= (Enable << CAP_UD_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Capture User Data.
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Disable
 				Return Value [1] : Enable
 */
CBOOL	NX_SPDIFRX_GetCaptureUserData( U32 ModuleIndex )
{
	const 	 U32 CAP_UD_BITPOS	= 3;
	const 	 U32 CAP_UD_MASK	= (1UL << CAP_UD_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & CAP_UD_MASK ) >> CAP_UD_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Phase Detect
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in]	Enable			Setting Value [0] : Disable
 								Setting Value [1] : Enable
 *	@return		None
 */
void	NX_SPDIFRX_SetPhaseDetect( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 PHASE_BITPOS	= 8;
	const 	 U32 PHASE_MASK		= (1UL << PHASE_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~PHASE_MASK;
	regvalue |= (Enable << PHASE_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Phase Detect
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Disable
 				Return Value [1] : Enable
 */
CBOOL	NX_SPDIFRX_GetPhaseDetect( U32 ModuleIndex )
{
	const 	 U32 PHASE_BITPOS	= 8;
	const 	 U32 PHASE_MASK		= (1UL << PHASE_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & PHASE_MASK ) >> PHASE_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Decode	Enable.
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in]	Setting Value [0] : Disable
 				Setting Value [1] : Enable
 *	@return		None
 */
void	NX_SPDIFRX_SetDecodeEnable( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 DEC_ENB_BITPOS		= 0;
	const 	 U32 DEC_ENB_MASK		= (1UL << DEC_ENB_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~DEC_ENB_MASK;
	regvalue |= (Enable << DEC_ENB_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get Decode Enable.
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Disable
 				Return Value [1] : Enable				
 */
CBOOL	NX_SPDIFRX_GetDecodeEnable( U32 ModuleIndex )
{
	const 	 U32 DEC_ENB_BITPOS		= 0;
	const 	 U32 DEC_ENB_MASK		= (1UL << DEC_ENB_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & DEC_ENB_MASK ) >> DEC_ENB_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Lock
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : UnLock
 				Return Value [1] : Lock
 */
CBOOL	NX_SPDIFRX_GetLock( U32 ModuleIndex )
{
	const U32 LOCK_BITPOS	= 10;
	const U32 LOCK_MASK 	= 1UL << LOCK_BITPOS;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & LOCK_MASK ) >> LOCK_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Write/Read Count Clear.
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in] Enable			Setting Value [0] : Disable
 								Setting Value [1] : Clear
 *	@return		None
 */
void	NX_SPDIFRX_ResetFIFO( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 RST_BITPOS		= 9;
	const 	 U32 RST_MASK		= (1UL << RST_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~RST_MASK;
	regvalue |= (Enable << RST_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  Sample Offset
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in]	Setting Value [0] : 8 Bit,	Setting Value [1] : 7 Bit
 				Setting Value [2] : 6 Bit,	Setting Value [3] : 5 Bit
 				Setting Value [4] : 4 Bit,	Setting Value [5] : 3 Bit
 				Setting Value [6] : 2 Bit,	Setting Value [7] : 1 Bit
 				Setting Value [8] : 0 Bit
 *	@return		None
 *	@remark	The vaild sampling bit.
 */
void	NX_SPDIFRX_SetSampleOffset( U32 ModuleIndex, NX_SPDIFRX_SAMPLEOFFSET Offset )
{
	const 	 U32 OFFSET_BITPOS	= 4;
	const 	 U32 OFFSET_MASK	= (0xF << OFFSET_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( 0xF >= Offset );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~OFFSET_MASK;
	regvalue |= (Offset << OFFSET_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  Sample Offset
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : 8 Bit,	Return Value [1] : 7 Bit
 				Return Value [2] : 6 Bit,	Return Value [3] : 5 Bit
 				Return Value [4] : 4 Bit,	Return Value [5] : 3 Bit
 				Return Value [6] : 2 Bit,	Return Value [7] : 1 Bit
 				Return Value [8] : 0 Bit
 */
NX_SPDIFRX_SAMPLEOFFSET		NX_SPDIFRX_GetSampleOffset( U32 ModuleIndex )
{
	const 	 U32 OFFSET_BITPOS	= 4;
	const 	 U32 OFFSET_MASK	= (0xF << OFFSET_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (NX_SPDIFRX_SAMPLEOFFSET)(( regvalue & OFFSET_MASK ) >> OFFSET_BITPOS );
}

//-------------------------------------------------------------------
/**
 *	@brief		Set  DMA Data Only
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in]	Setting Value [0] : Disable
 				Setting Value [1] : Enable
 *	@return		None
 */
void	NX_SPDIFRX_SetDMADataOnly( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 DMA_DO_BITPOS	= 2;
	const 	 U32 DMA_DO_MASK	= (1UL << DMA_DO_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~DMA_DO_MASK;
	regvalue |= (Enable << DMA_DO_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  DMA Data Only
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Data & Status
 				Return Value [1] : Data Only
 */
CBOOL	NX_SPDIFRX_GetDMADataOnly( U32 ModuleIndex )
{
	const 	 U32 DMA_DO_BITPOS	= 2;
	const 	 U32 DMA_DO_MASK	= (1UL << DMA_DO_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & DMA_DO_MASK ) >> DMA_DO_BITPOS );
}


//-------------------------------------------------------------------
/**
 *	@brief		Set  DMA Data Swap
 *	@param[in]	ModuleIndex		An index of module.
 * 	@param[in]	Setting Value [0] : Channel A First
 				Setting Value [1] : Channel B First
 *	@return		None
 *	@remark	The swap data of the channel A and channel B.
 */
void	NX_SPDIFRX_SetDMADataSwap( U32 ModuleIndex, CBOOL Enable )
{
	const 	 U32 DMA_SWAP_BITPOS	= 1;
	const 	 U32 DMA_SWAP_MASK		= (1UL << DMA_SWAP_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	NX_ASSERT( (CTRUE == Enable) || (CFALSE == Enable) );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);
	regvalue &= ~DMA_SWAP_MASK;
	regvalue |= (Enable << DMA_SWAP_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, regvalue);
}

//-------------------------------------------------------------------
/**
 *	@brief		Get  DMA Data Swap
 *	@param[in]	ModuleIndex		An index of module.
 * 	@return 	Return Value [0] : Channel A First
 				Return Value [1] : Channel B First
 *	@remark	The swap data of the channel A and channel B. 				
 */
CBOOL	NX_SPDIFRX_GetDMADataSwap( U32 ModuleIndex )
{
	const 	 U32 DMA_SWAP_BITPOS	= 1;
	const 	 U32 DMA_SWAP_MASK		= (1UL << DMA_SWAP_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL);

	return (CBOOL)(( regvalue & DMA_SWAP_MASK ) >> DMA_SWAP_BITPOS );
}

//--------------------------------------------------------------------------
// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_SPDIFRX_Set_SPDCTRL( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL, value);
}
U32		NX_SPDIFRX_Get_SPDCTRL( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_CTRL));
}

void	NX_SPDIFRX_Set_SPDENBIRQ( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ, value);
}
U32		NX_SPDIFRX_Get_SPDENBIRQ( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SPDIF_ENBIRQ));
}

U32		NX_SPDIFRX_Get_REGUSERA0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA0));
}
U32		NX_SPDIFRX_Get_REGUSERA1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA1));
}
U32		NX_SPDIFRX_Get_REGUSERA2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA2));
}
U32		NX_SPDIFRX_Get_REGUSERA3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA3));
}
U32		NX_SPDIFRX_Get_REGUSERA4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA4));
}
U32		NX_SPDIFRX_Get_REGUSERA5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERA5));
}
U32		NX_SPDIFRX_Get_REGUSERB0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB0));
}
U32		NX_SPDIFRX_Get_REGUSERB1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB1));
}
U32		NX_SPDIFRX_Get_REGUSERB2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB2));
}
U32		NX_SPDIFRX_Get_REGUSERB3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB3));
}
U32		NX_SPDIFRX_Get_REGUSERB4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB4));
}
U32		NX_SPDIFRX_Get_REGUSERB5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGUSERB5));
}
U32		NX_SPDIFRX_Get_REGSTATA0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA0));
}
U32		NX_SPDIFRX_Get_REGSTATA1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA1));
}
U32		NX_SPDIFRX_Get_REGSTATA2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA2));
}
U32		NX_SPDIFRX_Get_REGSTATA3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA3));
}
U32		NX_SPDIFRX_Get_REGSTATA4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA4));
}
U32		NX_SPDIFRX_Get_REGSTATA5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATA5));
}
U32		NX_SPDIFRX_Get_REGSTATB0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB0));
}
U32		NX_SPDIFRX_Get_REGSTATB1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB1));
}
U32		NX_SPDIFRX_Get_REGSTATB2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB2));
}
U32		NX_SPDIFRX_Get_REGSTATB3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB3));
}
U32		NX_SPDIFRX_Get_REGSTATB4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB4));
}
U32		NX_SPDIFRX_Get_REGSTATB5( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_SPDIFRX_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->REGSTATB5));
}
//@}
