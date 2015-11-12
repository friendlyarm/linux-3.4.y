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
//	Module		: TMU
//	File		: nx_tmu.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_tmu.h>

static	struct
{
	struct NX_TMU_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_TMU_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicates that Initialize is successed.\r\n
 *			\b CFALSE indicates that Initialize is failed.\r\n
 *	@see	NX_TMU_GetNumberOfModule
 */
CBOOL	NX_TMU_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_TMU_MODULE; i++ )
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
 *	@see		NX_TMU_Initialize
 */
U32		NX_TMU_GetNumberOfModule( void )
{
	return NUMBER_OF_TMU_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 *	@see		NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_SetBaseAddress,		NX_TMU_GetBaseAddress,
 *				NX_TMU_OpenModule,			NX_TMU_CloseModule,
 *				NX_TMU_CheckBusy,			NX_TMU_CanPowerDown
 */
U32		NX_TMU_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( TMU ) }; // PHY_BASEADDR_TMU?_MODULE
	
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	
	return (U32)PhysicalAddr[ModuleIndex];
}


/**
 *	@brief		Get module's Reset number.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's Reset number
 */
//U32 NX_TMU_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex)
//{	
//	const U32 ResetPinNumber[2][NUMBER_OF_TMU_MODULE] =
//	{
//		{ RESETINDEX_LIST( TMU, nTMURST )}
//	};
//	
//	return (U32)ResetPinNumber[ChannelIndex][ModuleIndex];
//}

/**
 *	@brief		Get module's number of Reset.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's number of Reset
 */
U32 NX_TMU_GetNumberOfReset()
{
	return (U32)NUMBER_OF_RESET_MODULE_PIN;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_TMU_GetPhysicalAddress,
 *				NX_TMU_SetBaseAddress,		NX_TMU_GetBaseAddress,
 *				NX_TMU_OpenModule,			NX_TMU_CloseModule,
 *				NX_TMU_CheckBusy,			NX_TMU_CanPowerDown
 */
U32		NX_TMU_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_TMU_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_TMU_GetPhysicalAddress,	NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_GetBaseAddress,
 *				NX_TMU_OpenModule,			NX_TMU_CloseModule,
 *				NX_TMU_CheckBusy,			NX_TMU_CanPowerDown
 */
void	NX_TMU_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_TMU_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 *	@see		NX_TMU_GetPhysicalAddress,	NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_SetBaseAddress,
 *				NX_TMU_OpenModule,			NX_TMU_CloseModule,
 *				NX_TMU_CheckBusy,			NX_TMU_CanPowerDown
 */
void*	NX_TMU_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );

	return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Initialize is successed. \r\n
 *				\b CFALSE	indicates that Initialize is failed.
 *	@see		NX_TMU_GetPhysicalAddress,	NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_SetBaseAddress,		NX_TMU_GetBaseAddress,
 *				NX_TMU_CloseModule,
 *				NX_TMU_CheckBusy,			NX_TMU_CanPowerDown
 */
CBOOL	NX_TMU_OpenModule( U32 ModuleIndex )
{
	register struct NX_TMU_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

    // Open Code Add..
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Deinitialize is successed. \r\n
 *				\b CFALSE	indicates that Deinitialize is failed.
 *	@see		NX_TMU_GetPhysicalAddress,	NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_SetBaseAddress,		NX_TMU_GetBaseAddress,
 *				NX_TMU_OpenModule,
 *				NX_TMU_CheckBusy,			NX_TMU_CanPowerDown
 */
CBOOL	NX_TMU_CloseModule( U32 ModuleIndex )
{
	register struct NX_TMU_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

    // Open Code Add..
return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Module is Busy. \r\n
 *				\b CFALSE	indicates that Module is NOT Busy.
 *	@see		NX_TMU_GetPhysicalAddress,	NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_SetBaseAddress,		NX_TMU_GetBaseAddress,
 *				NX_TMU_OpenModule,			NX_TMU_CloseModule,
 *				NX_TMU_CanPowerDown
 */
CBOOL	NX_TMU_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \r\n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 *	@see		NX_TMU_GetPhysicalAddress,	NX_TMU_GetSizeOfRegisterSet,
 *				NX_TMU_SetBaseAddress,		NX_TMU_GetBaseAddress,
 *				NX_TMU_OpenModule,			NX_TMU_CloseModule,
 *				NX_TMU_CheckBusy
 */
CBOOL	NX_TMU_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 *	@see											NX_TMU_SetInterruptEnable,
 *				NX_TMU_GetInterruptEnable,			NX_TMU_SetInterruptEnable32,
 *				NX_TMU_GetInterruptEnable32,		NX_TMU_GetInterruptPending,
 *				NX_TMU_GetInterruptPending32,		NX_TMU_ClearInterruptPending,
 *				NX_TMU_ClearInterruptPending32,	NX_TMU_SetInterruptEnableAll,
 *				NX_TMU_GetInterruptEnableAll,		NX_TMU_GetInterruptPendingAll,
 *				NX_TMU_ClearInterruptPendingAll,	NX_TMU_GetInterruptPendingNumber
 */
U32		NX_TMU_GetInterruptNumber( U32 ModuleIndex )
{
	const U32	UartInterruptNumber[NUMBER_OF_TMU_MODULE] =
				{
					INTNUM_LIST(TMU)
				};

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );

	return	UartInterruptNumber[ModuleIndex];
}

void	NX_TMU_SetInterruptEnable( U32 ModuleIndex, NX_TMU_INT_MASK IntNum, CBOOL Enable )
{
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTEN);

	if( CTRUE == Enable )
	{
		regvalue |= ( 1UL << IntNum );
	}
	else
	{
		regvalue &= ~( 1UL << IntNum );
	}
	//WriteIO32(&pRegister->P0_INTEN, regvalue);
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTEN, regvalue);
}


void	NX_TMU_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	//NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue  = Enable ? 0xFFFFFFFF : 0 ;

	//WriteIO32(&pRegister->P0_INTEN, regvalue);
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTEN, regvalue);
}

CBOOL	NX_TMU_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	//NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
	
	//return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->P0_INTEN ));
	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTEN));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]	ModuleIndex     A index of module.
 *  @param[in]	IntNum  Interrupt Number ( 0 ).
 *  @return   	CTRUE  indicates that Pending is seted.
 *             		CFALSE indicates that Pending is Not Seted.
 *  @remarks    TMU Module have one interrupt. So always IntNum set to 0.

 */
CBOOL   NX_TMU_GetInterruptPending( U32 ModuleIndex, NX_TMU_INT_MASK IntNum )
{
    const U32 PEND_MASK  = 1UL << IntNum;

    register struct NX_TMU_RegisterSet* pRegister;

    NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );

    return  (CBOOL)( (ReadIO32(&pRegister->P0_INTSTAT) & PEND_MASK) >> IntNum );
}

CBOOL	NX_TMU_GetInterruptPendingAll( U32 ModuleIndex )
{
	register U32    regvalue;
	
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	//NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTSTAT); 
    
	return (CBOOL)( regvalue  ? 1 : 0  );
}

void    NX_TMU_ClearInterruptPending( U32 ModuleIndex, NX_TMU_INT_MASK IntNum )
{
    const U32 INTC_MASK  = 1UL << IntNum;

    register struct NX_TMU_RegisterSet* pRegister;
    register U32 ReadValue;

    IntNum = IntNum;
    NX_ASSERT( NUMBER_OF_I2C_MODULE > ModuleIndex );
	
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	NX_ASSERT( CNULL != pRegister );
    //NX_ASSERT( 0 == IntNum );

    ReadValue   =   ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTCLEAR);
    ReadValue   &=  ~INTC_MASK;
    ReadValue   |=  INTC_MASK;

    WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTCLEAR, ReadValue);
}


void	NX_TMU_ClearInterruptPendingAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	//NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTCLEAR, 0xFFFFFFFF);
}

//--------------------------------------------------------------------------
// Configuration operations
//--------------------------------------------------------------------------
//@{
//void	NX_TMU_SetTransmitMode( U32 ModuleIndex, U8 mode )
//{
//	const U32 TRMODE_BITPOS	= 2;
//	const U32 TRMODE_MASK	= (3<<TRMODE_BITPOS);
//	register U32 temp;
//
//	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
//
//	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON);
//	temp &= ~TRMODE_MASK;
//	temp |= (mode<<TRMODE_BITPOS);
//	
//	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON, temp);
//}
//
//U8	NX_TMU_GetTransmitMode( U32 ModuleIndex )
//{
//	const U32 TRMODE_BITPOS	= 2;
//	const U32 TRMODE_MASK	= (3<<TRMODE_BITPOS);
//
//	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
//
//	return (U8)(( ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->UCON) & TRMODE_MASK ) >> TRMODE_BITPOS );
//}

U32	    NX_TMU_GetTriminfo85	( U32 ModuleIndex )	// TRIMINFO[15:8]
{
	const U32 TRIM85_BITPOS	= 8;
	const U32 TRIM85_MASK	= (0xFF<<TRIM85_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue  = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TRIMINFO);
    regvalue  = ( regvalue & TRIM85_MASK ) >> TRIM85_BITPOS;
	
	return regvalue;
}

U32     NX_TMU_GetTriminfo25	( U32 ModuleIndex )	// TRIMINFO[8:0]
{
	const U32 TRIM25_BITPOS	= 0;
	const U32 TRIM25_MASK	= (0xFF<<TRIM25_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TRIMINFO);
    regvalue = ( regvalue & TRIM25_MASK ) >> TRIM25_BITPOS;

	return regvalue;
}

void	NX_TMU_SetTmuTripMode	( U32 ModuleIndex, NX_TMU_TRIPMODE TripMode )	// TMU_CONTROL[15:13]
{
	const U32 TRMODE_BITPOS	= 13;
	const U32 TRMODE_MASK	= (7<<TRMODE_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL);
	temp &= ~TRMODE_MASK;
	temp |= (TripMode<<TRMODE_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL, temp);
}

void	NX_TMU_SetTmuTripEn		( U32 ModuleIndex, U32 value )	// TMU_CONTROL[12]
{
	const U32 TREN_BITPOS	= 12;
	const U32 TREN_MASK	= (1<<TREN_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL);
	temp &= ~TREN_MASK;
	temp |= (value<<TREN_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL, temp);
}

void	NX_TMU_SetBufSlopeSel		( U32 ModuleIndex, U32 value )	// TMU_CONTROL[8]
{
	const U32 BUF_SLOPE_SEL_BITPOS	= 8;
	const U32 BUF_SLOPE_SEL_MASK	= (0xF<<BUF_SLOPE_SEL_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL);
	temp &= ~BUF_SLOPE_SEL_MASK;
	temp |= (value<<BUF_SLOPE_SEL_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL, temp);
}

U32     NX_TMU_GetBufSlopeSel	( U32 ModuleIndex )	// TMU_CONTROL[8]
{
	const U32 BUF_SLOPE_SEL_BITPOS	= 8;
	const U32 BUF_SLOPE_SEL_MASK	= (0xF<<BUF_SLOPE_SEL_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TRIMINFO);
    regvalue = ( regvalue & BUF_SLOPE_SEL_MASK ) >> BUF_SLOPE_SEL_BITPOS;

	return regvalue;
}

void	NX_TMU_SetVRefVBESel		( U32 ModuleIndex, U32 value )	// TMU_CONTROL1[0]
{
	const U32 VREF_VBE_SEL_BITPOS	= 0;
	const U32 VREF_VBE_SEL_MASK	= (0xF<<VREF_VBE_SEL_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL);
	temp &= ~VREF_VBE_SEL_MASK;
	temp |= (value<<VREF_VBE_SEL_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL, temp);
}

U32     NX_TMU_GetVRefVBESel	( U32 ModuleIndex )	// TMU_CONTROL1[0]
{
	const U32 VREF_VBE_SEL_BITPOS	= 0;
	const U32 VREF_VBE_SEL_MASK	= (0xF<<VREF_VBE_SEL_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TRIMINFO);
    regvalue = ( regvalue & VREF_VBE_SEL_MASK ) >> VREF_VBE_SEL_BITPOS;

	return regvalue;
}



void	NX_TMU_SetTmuStart		( U32 ModuleIndex, CBOOL value )	// TMU_CONTROL[0]
{
	const U32 TMUSTART_BITPOS	= 0;
	const U32 TMUSTART_MASK	= (1<<TMUSTART_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL);
	temp &= ~TMUSTART_MASK;
	temp |= (value<<TMUSTART_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL, temp);
}

U32     NX_TMU_GetTmuStart	( U32 ModuleIndex )	// TRIMINFO[8:0]
{
	const U32 TMUSTART_BITPOS	= 0;
	const U32 TMUSTART_MASK	    = (1<<TMUSTART_BITPOS);
	register U32 regvalue;


	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TRIMINFO);
    regvalue = ( regvalue & TMUSTART_MASK ) >> TMUSTART_BITPOS;

	return regvalue;
}

U32		NX_TMU_IsBusy	( U32 ModuleIndex )
{
    const U32 TMUBUSY_BITPOS    = 0;
    const U32 TMUBUSY_MASK      = (1<<TMUBUSY_BITPOS);
    register U32 regvalue          = 0;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_STATUS);
    regvalue = (regvalue & TMUBUSY_MASK) >> TMUBUSY_BITPOS;
    
    return (!regvalue);
}

void	NX_TMU_SetThresholdLevelRise0	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[7:0]
{
	const U32 RISELEVEL0_BITPOS	= 0;
	const U32 RISELEVEL0_MASK	= (0xFF<<RISELEVEL0_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
	regvalue &= ~RISELEVEL0_MASK;
	regvalue |= (value << RISELEVEL0_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelRise0	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[7:0]
{
	const U32 RISELEVEL0_BITPOS	= 0;
	const U32 RISELEVEL0_MASK	= (0xFF<<RISELEVEL0_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
    regvalue = (( regvalue & RISELEVEL0_MASK  ) >> RISELEVEL0_BITPOS);

	return regvalue;
}

void	NX_TMU_SetThresholdLevelRise1	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[15:8]
{
	const U32 RISELEVEL1_BITPOS	= 8;
	const U32 RISELEVEL1_MASK	= (0xFF<<RISELEVEL1_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
	regvalue &= ~RISELEVEL1_MASK;
	regvalue |= (value << RISELEVEL1_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelRise1	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[15:8]
{
	const U32 RISELEVEL1_BITPOS	= 8;
	const U32 RISELEVEL1_MASK	= (0xFF<<RISELEVEL1_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
    regvalue = (( regvalue & RISELEVEL1_MASK  ) >> RISELEVEL1_BITPOS);

	return regvalue;
}

void	NX_TMU_SetThresholdLevelRise2	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 RISELEVEL2_BITPOS	= 16;
	const U32 RISELEVEL2_MASK	= (0xFF<<RISELEVEL2_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
	regvalue &= ~RISELEVEL2_MASK;
	regvalue |= (value << RISELEVEL2_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelRise2	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 RISELEVEL2_BITPOS	= 16;
	const U32 RISELEVEL2_MASK	= (0xFF<<RISELEVEL2_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
    regvalue = (( regvalue & RISELEVEL2_MASK  ) >> RISELEVEL2_BITPOS);

	return regvalue;
}

void	NX_TMU_SetThresholdLevelRise3	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 RISELEVEL3_BITPOS	= 24;
	const U32 RISELEVEL3_MASK	= (0xFF<<RISELEVEL3_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
	regvalue &= ~RISELEVEL3_MASK;
	regvalue |= (value << RISELEVEL3_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelRise3	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 RISELEVEL3_BITPOS	= 24;
	const U32 RISELEVEL3_MASK	= (0xFF<<RISELEVEL3_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
    regvalue = (( regvalue & RISELEVEL3_MASK  ) >> RISELEVEL3_BITPOS);

	return regvalue;
}
//------------------

void	NX_TMU_SetThresholdLevelFalling0	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[7:0]
{
	const U32 FALLINGLEVEL0_BITPOS	= 0;
	const U32 FALLINGLEVEL0_MASK	= (0xFF<<FALLINGLEVEL0_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
	regvalue &= ~FALLINGLEVEL0_MASK;
	regvalue |= (value << FALLINGLEVEL0_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelFalling0	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[7:0]
{
	const U32 FALLINGLEVEL0_BITPOS	= 0;
	const U32 FALLINGLEVEL0_MASK	= (0xFF<<FALLINGLEVEL0_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
    regvalue = (( regvalue & FALLINGLEVEL0_MASK  ) >> FALLINGLEVEL0_BITPOS);

	return regvalue;
}

void	NX_TMU_SetThresholdLevelFalling1	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[15:8]
{
	const U32 FALLINGLEVEL1_BITPOS	= 8;
	const U32 FALLINGLEVEL1_MASK	= (0xFF<<FALLINGLEVEL1_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
	regvalue &= ~FALLINGLEVEL1_MASK;
	regvalue |= (value << FALLINGLEVEL1_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelFalling1	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[15:8]
{
	const U32 FALLINGLEVEL1_BITPOS	= 8;
	const U32 FALLINGLEVEL1_MASK	= (0xFF<<FALLINGLEVEL1_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
    regvalue = (( regvalue & FALLINGLEVEL1_MASK  ) >> FALLINGLEVEL1_BITPOS);

	return regvalue;
}

void	NX_TMU_SetThresholdLevelFalling2	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 FALLINGLEVEL2_BITPOS	= 16;
	const U32 FALLINGLEVEL2_MASK	= (0xFF<<FALLINGLEVEL2_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
	regvalue &= ~FALLINGLEVEL2_MASK;
	regvalue |= (value << FALLINGLEVEL2_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelFalling2	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 FALLINGLEVEL2_BITPOS	= 16;
	const U32 FALLINGLEVEL2_MASK	= (0xFF<<FALLINGLEVEL2_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
    regvalue = (( regvalue & FALLINGLEVEL2_MASK  ) >> FALLINGLEVEL2_BITPOS);

	return regvalue;
}

void	NX_TMU_SetThresholdLevelFalling3	( U32 ModuleIndex, U32 value ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 FALLINGLEVEL3_BITPOS	= 24;
	const U32 FALLINGLEVEL3_MASK	= (0xFF<<FALLINGLEVEL3_BITPOS);
	register U32 regvalue;
    
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
	regvalue &= ~FALLINGLEVEL3_MASK;
	regvalue |= (value << FALLINGLEVEL3_BITPOS);

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, regvalue);
}

U32     NX_TMU_GetThresholdLevelFalling3	( U32 ModuleIndex ) //THRESHOLE_TEMP_RISE[23:16]
{
	const U32 FALLINGLEVEL3_BITPOS	= 24;
	const U32 FALLINGLEVEL3_MASK	= (0xFF<<FALLINGLEVEL3_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL);
    regvalue = (( regvalue & FALLINGLEVEL3_MASK  ) >> FALLINGLEVEL3_BITPOS);

	return regvalue;
}
//-------


void	NX_TMU_SetTmuEmulEn		( U32 ModuleIndex, CBOOL value )	// EMUL_CON[0]
{
	const U32 TMUEMULEN_BITPOS	= 0;
	const U32 TMUEMULEN_MASK	= (1<<TMUEMULEN_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON);
	temp &= ~TMUEMULEN_MASK;
	temp |= (value<<TMUEMULEN_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON, temp);
}

U32	    NX_TMU_GetTmuEmulEn		( U32 ModuleIndex )	// EMUL_CON[0]
{
	const U32 TMUEMULEN_BITPOS	= 0;
	const U32 TMUEMULEN_MASK	= (1<<TMUEMULEN_BITPOS);
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

    regvalue = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE);
    regvalue = (( regvalue & TMUEMULEN_MASK  ) >> TMUEMULEN_BITPOS);

    return regvalue;
}


void	NX_TMU_SetTmuNextTime	( U32 ModuleIndex, U16 value )	// EMUL_CON[31:16]
{
	const U32 TMUNEXTTIME_BITPOS	= 16;
	const U32 TMUNEXTTIME_MASK	= (0xffff<<TMUNEXTTIME_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON);
	temp &= ~TMUNEXTTIME_MASK;
	temp |= (value<<TMUNEXTTIME_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON, temp);
}

void	NX_TMU_SetTmuNextData	( U32 ModuleIndex, U32 value )	// EMUL_CON[15:8]
{
	const U32 TMUNEXTDATA_BITPOS	= 8;
	const U32 TMUNEXTDATA_MASK	    = (0xff<<TMUNEXTDATA_BITPOS);
	register U32 temp;

	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	temp = ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON);
	temp &= ~TMUNEXTDATA_MASK;
	temp |= (value<<TMUNEXTDATA_BITPOS);
	
	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON, temp);
}

//@}

//--------------------------------------------------------------------------
/// @name	Register handling
//--------------------------------------------------------------------------
//@{
U32		NX_TMU_GetTriminfo	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TRIMINFO));
}

void	NX_TMU_SetTmuControl	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL, value);
}
U32		NX_TMU_GetTmuControl	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL));
}

void	NX_TMU_SetTmuControl1	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL1, value);
}
U32		NX_TMU_GetTmuControl1	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_CONTROL1));
}

U32		NX_TMU_GetTmuStatus	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TMU_STATUS));
}

void	NX_TMU_SetSamplingInterval	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SAMPLING_INTERVAL, value);
}
U32		NX_TMU_GetSamplingInterval	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->SAMPLING_INTERVAL));
}

void	NX_TMU_SetCounterValue0	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->COUNTER_VALUE0, value);
}
U32		NX_TMU_GetCounterValue0	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->COUNTER_VALUE0));
}

void	NX_TMU_SetCounterValue1	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->COUNTER_VALUE1, value);
}
U32		NX_TMU_GetCounterValue1	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->COUNTER_VALUE1));
}

U32		NX_TMU_GetCurrentTemp0	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CURRENT_TEMP0));
}

U32		NX_TMU_GetCurrentTemp1	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->CURRENT_TEMP1));
}

void	NX_TMU_SetThresholdTempRise	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE, value);
}
U32		NX_TMU_GetThresholdTempRise	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_RISE));
}

void	NX_TMU_SetThresholdTempFall	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL, value);
}
U32		NX_TMU_GetThresholdTempFall	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TEMP_FALL));
}

void	NX_TMU_SetThresholdTq	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TQ, value);
}
U32		NX_TMU_GetThresholdTq	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->THRESHOLD_TQ));
}

U32		NX_TMU_GetTqStatus	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->TQ_STATUS));
}

U32		NX_TMU_GetP0PastTemp3_0	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_PAST_TEMP3_0));
}

U32		NX_TMU_GetP0PastTemp7_4	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_PAST_TEMP7_4));
}

U32		NX_TMU_GetP0PastTemp11_8	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_PAST_TEMP11_8));
}

U32		NX_TMU_GetP0PastTemp15_12	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_PAST_TEMP15_12));
}

void	NX_TMU_SetP0IntEn	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTEN, value);
}
U32		NX_TMU_GetP0IntEn	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTEN));
}

U32		NX_TMU_GetP0IntStat	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTSTAT));
}

void	NX_TMU_SetP0IntClear	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTCLEAR, value);
}
U32		NX_TMU_GetP0IntClear	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->P0_INTCLEAR));
}

void	NX_TMU_SetEmulCon	( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON, value);
}
U32		NX_TMU_GetEmulCon	( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TMU_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIO32(&__g_ModuleVariables[ModuleIndex].pRegister->EMUL_CON));
}
























//@}


