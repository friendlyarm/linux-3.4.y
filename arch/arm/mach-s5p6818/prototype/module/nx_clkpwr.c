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
//	Module		: Clock & Power Manager
//	File		: nx_clkpwr.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------

#include "nx_clkpwr.h"


#define NX_CLKPWR_NUMBER_OF_CLOCK 5
#define NX_CLKPWR_NUMBER_OF_PLL 4
#define NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER 15
#define NX_CLKPWR_INT_NUMBER 3

static	struct NX_CLKPWR_RegisterSet *__g_pRegister = CNULL;
static	U32 __g_OSC_KHz;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		 CTRUE	indicates that Initialize is succeeded.
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_CLKPWR_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_CLKPWR_Initialize
 */
U32		NX_CLKPWR_GetNumberOfModule( void )
{
	return NUMBER_OF_CLKPWR_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_CLKPWR_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_CLKPWR_MODULE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_CLKPWR_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_CLKPWR_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.

 */
void	NX_CLKPWR_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_CLKPWR_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_CLKPWR_GetBaseAddress( void )
{
	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicates that Initialize is succeeded. 
 *				 CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_CLKPWR_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicates that Deinitialize is succeeded. 
 *				 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_CLKPWR_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicates that Module is Busy. 
 *				 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_CLKPWR_CheckBusy( void )
{
	if( NX_CLKPWR_IsPLLStable() )
	{
		return CFALSE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		 CTRUE	indicates that Ready to enter power-down stage. 
 *				 CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_CLKPWR_CanPowerDown( void )
{
	if( NX_CLKPWR_IsPLLStable() )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@return		Interrupt numberer
 */
S32		NX_CLKPWR_GetInterruptNumber( void )
{
	return	INTNUM_OF_CLKPWR_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number(0 ~ 10). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3, 4:ALIVEGPIO4, 5:ALIVEGPIO5, 
 *						6:ALIVEGPIO6, 7:ALIVEGPIO7, 8:VDDTOGGLE, 9:RTC, 10:BATF
 *	@param[in]	Enable	 CTRUE	indicates that Interrupt Enable. 
 *						 CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_CLKPWR_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
	U32 ReadValue;

	NX_ASSERT( NX_CLKPWR_INT_MAX > IntNum );
	NX_ASSERT( (0==Enable) | (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	if( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > IntNum )
	{
		ReadValue = ReadIO32(&__g_pRegister->GPIOINTENB);

		ReadValue &= ~( 0x01 << IntNum );
		ReadValue |= (U32)Enable << IntNum;

		WriteIO32(&__g_pRegister->GPIOINTENB, ReadValue);
	}
	else	// RTC, BATF
	{
		ReadValue = ReadIO32(&__g_pRegister->INTENABLE);

		ReadValue &= ~( 0x01 << (IntNum-NX_CLKPWR_INT_NUMBER) );
		ReadValue |= (U32)Enable << (IntNum-NX_CLKPWR_INT_NUMBER);

		WriteIO32(&__g_pRegister->INTENABLE, ReadValue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number(0 ~ 10). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3, 4:ALIVEGPIO4, 5:ALIVEGPIO5, 
 *						6:ALIVEGPIO6, 7:ALIVEGPIO7, 8:VDDTOGGLE, 9:RTC, 10:BATF
 *	@return		 CTRUE	indicates that Interrupt is enabled. 
 *				 CFALSE	indicates that Interrupt is disabled.
 */
CBOOL	NX_CLKPWR_GetInterruptEnable( S32 IntNum )
{
	NX_ASSERT( (NX_CLKPWR_INT_NUMBER + NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER ) > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	if( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > IntNum )
	{
		return (CBOOL)( (ReadIO32(&__g_pRegister->GPIOINTENB) >> IntNum) & 0x01 );
	}
	else
	{
		return (CBOOL)( (ReadIO32(&__g_pRegister->INTENABLE) >> (IntNum-NX_CLKPWR_INT_NUMBER)) & 0x01 );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	EnableFlag	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							- EnableFlag[0] : ALIVE GPIO0 interrupt enable or disable. 
 *							...
 *							- EnableFlag[7] : ALIVE GPIO7 interrupt enable or disable. 
 *							- EnableFlag[8] : VDD power toggle interrupt enable or disable. 
 *							- EnableFlag[9] : RTC interrupt enable or disable. 
 *							- EnableFlag[10]: Battery Fault interrupt enable or disable. 
 *	@return		None.
 */
void	NX_CLKPWR_SetInterruptEnable32( U32 EnableFlag )
{
	const U32 GPIOENB_MASK	= 0x1FF;
	const U32 ENB_MASK		= 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->GPIOINTENB, EnableFlag & GPIOENB_MASK);
	WriteIO32(&__g_pRegister->INTENABLE, (EnableFlag >> 9) & ENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt enable bit.
 *	@return		Current setting value of interrupt. 
 *				"1" means interrupt is enabled. 
 *				"0" means interrupt is disabled. 
 *				- Return Value[0] : ALIVE GPIO0 interrupt's setting value. 
 *				...
 *				- Return Value[7] : ALIVE GPIO7 interrupt's setting value. 
 *				- Return Value[8] : VDD power toggle interrupt's setting value. 
 *				- Return Value[9] : RTC interrupt's setting value. 
 *				- Return Value[10]: Battery Fault interrupt's setting value. 
 */
U32		NX_CLKPWR_GetInterruptEnable32( void )
{
	const U32 GPIOENB_MASK	= 0x1FF;
	const U32 ENB_MASK		= 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)((ReadIO32(&__g_pRegister->GPIOINTENB) & GPIOENB_MASK) | ((ReadIO32(&__g_pRegister->INTENABLE) & ENB_MASK)<<9));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number(0 ~ 10). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3, 4:ALIVEGPIO4, 5:ALIVEGPIO5, 
 *						6:ALIVEGPIO6, 7:ALIVEGPIO7, 8:VDDTOGGLE, 9:RTC, 10:BATF
 *	@return		 CTRUE	indicates that Pending is seted. 
 *				 CFALSE	indicates that Pending is Not Seted.
 */
CBOOL	NX_CLKPWR_GetInterruptPending( S32 IntNum )
{
	NX_ASSERT( NX_CLKPWR_INT_MAX  > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	if( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > IntNum )
	{
		return ( (ReadIO32(&__g_pRegister->GPIOINTPEND) >> IntNum) & 0x01 );
	}
	else
	{
		return ( (ReadIO32(&__g_pRegister->INTPEND) >> (IntNum-NX_CLKPWR_INT_NUMBER)) & 0x01 );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates current setting value of interrupt pending bit.
 *	@return		Current setting value of pending bit. 
 *				"1" means pend bit is occured. 
 *				"0" means pend bit is NOT occured. 
 *				- Return Value[0] : ALIVE GPIO0 pending state. 
 *				...
 *				- Return Value[7] : ALIVE GPIO7 pending state. 
 *				- Return Value[8] : VDD power toggle pending state. 
 *				- Return Value[9] : RTC pending state. 
 *				- Return Value[10]: Battery Fault pending state. 
 */
U32		NX_CLKPWR_GetInterruptPending32( void )
{
	const U32 GPIOPEND_MASK	= 0x1FF;
	const U32 PEND_MASK		= 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)((ReadIO32(&__g_pRegister->GPIOINTPEND) & GPIOPEND_MASK) | ((ReadIO32(&__g_pRegister->INTPEND) & PEND_MASK)<<9));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt Number(0 ~ 10). 
 *						0:ALIVEGPIO0, 1:ALIVEGPIO1, 2:ALIVEGPIO2, 3:ALIVEGPIO3, 4:ALIVEGPIO4, 5:ALIVEGPIO5, 
 *						6:ALIVEGPIO6, 7:ALIVEGPIO7, 8:VDDTOGGLE, 9:RTC, 10:BATF
 *	@return		None.
 */
void	NX_CLKPWR_ClearInterruptPending( S32 IntNum )
{
	NX_ASSERT( NX_CLKPWR_INT_MAX > IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	if( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > IntNum )
	{
		WriteIO32(&__g_pRegister->GPIOINTPEND, 1 << IntNum);
	}
	else
	{
		WriteIO32(&__g_pRegister->INTPEND, 1 << (IntNum-NX_CLKPWR_INT_NUMBER));
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	PendingFlag		Specify pend bit to clear. Each bit's meaning is like below	 
 *								- PendingFlag[0] : ALIVE GPIO0 pending bit. 
 *								...
 *								- PendingFlag[7] : ALIVE GPIO7 pending bit. 
 *								- PendingFlag[8] : VDD power toggle pending bit. 
 *								- PendingFlag[9] : RTC pending bit. 
 *								- PendingFlag[10]: Battery Fault pending bit. 
 *	@return		None.
 */
void	NX_CLKPWR_ClearInterruptPending32( U32 PendingFlag )
{
	const U32 GPIOPEND_MASK = 0x1FF;
	const U32 PEND_MASK = 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->GPIOINTPEND, (PendingFlag & GPIOPEND_MASK));
	WriteIO32(&__g_pRegister->INTPEND, (PendingFlag >> 9 ) & PEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	Enable	 CTRUE	indicates that Set to all interrupt enable. 
 *						 CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_CLKPWR_SetInterruptEnableAll( CBOOL Enable )
{
	NX_ASSERT( CNULL != __g_pRegister );

	if( Enable )
	{
		WriteIO32(&__g_pRegister->GPIOINTENB, 0x1FF);
		WriteIO32(&__g_pRegister->INTENABLE, 0x03);
	}
	else
	{
		WriteIO32(&__g_pRegister->GPIOINTENB, 0x00);
		WriteIO32(&__g_pRegister->INTENABLE, 0x00);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		 CTRUE	indicates that At least one( or more ) interrupt is enabled. 
 *				 CFALSE	indicates that All interrupt is disabled.
 */
CBOOL	NX_CLKPWR_GetInterruptEnableAll( void )
{
	const U32 GPIOINTENB_MASK	= 0x1FF;
	const U32 INTENB_MASK		= 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	if( (ReadIO32(&__g_pRegister->GPIOINTENB) & GPIOINTENB_MASK) || (ReadIO32(&__g_pRegister->INTENABLE) & INTENB_MASK) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@return		 CTRUE	indicates that At least one( or more ) pending is seted. 
 *				 CFALSE	indicates that All pending is NOT seted.
 */
CBOOL	NX_CLKPWR_GetInterruptPendingAll( void )
{
	const U32 GPIOINTPEND_MASK	= 0x1FF;
	const U32 INTPEND_MASK		= 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	if( (ReadIO32(&__g_pRegister->GPIOINTPEND) & GPIOINTPEND_MASK) ||
		(ReadIO32(&__g_pRegister->INTPEND) & INTPEND_MASK) )
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
void	NX_CLKPWR_ClearInterruptPendingAll( void )
{
	const U32 GPIOINTPEND_MASK	= 0x1FF;
	const U32 INTPEND_MASK		= 0x03;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->GPIOINTPEND, GPIOINTPEND_MASK);
	WriteIO32(&__g_pRegister->INTPEND, INTPEND_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
S32		NX_CLKPWR_GetInterruptPendingNumber( void )	// -1 if None
{
	register U32 Pend;
	U32 dwIntNum;

	NX_ASSERT( CNULL != __g_pRegister );

	Pend = (ReadIO32(&__g_pRegister->GPIOINTPEND) &
			ReadIO32(&__g_pRegister->GPIOINTENB));

	Pend |= ((ReadIO32(&__g_pRegister->INTPEND) &
			  ReadIO32(&__g_pRegister->INTENABLE))<<9 );

	for( dwIntNum = 0; dwIntNum < 11; dwIntNum++ )
	{
		if( Pend & ( 1 << dwIntNum) )
		{
			return dwIntNum;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
//	Clock Management
//--------------------------------------------------------------------------
/**
 *	@brief		Set PLL by P, M, S.
 *	@param[in]	pllnumber	PLL to be changed ( 0 : PLL0, 1 : PLL1, 2 : PLL2, 3 : PLL3  )
 *	@param[in]	PDIV		Input frequency divider : 1 ~ 63
 *	@param[in]	MDIV		VCO frequency divider	: PLL0, 1(64~1023), PLL2, 3(13~255)
 *	@param[in]	SDIV		Output frequency scaler : 0 ~ 3
 *	@return		None.
 *	@remark		PLL can be calculated by following fomula. 
 *				(m * Fin)/(p * 2^s ).
 *				- PLL0, 1 : 40 ~ 2500 Mhz 
 *				- PLL2, 3 : 40 ~ 2200 Mhz 
 *				- IMPORTANT : 
 *					You should set a PMS value carefully. 
 *					Please refer PMS table we are recommend or Contact Nexell or
 *						Samsung(manufacture of PLL core) if you wish to change it.
 *
 *	@remark		PLL is not changed until NX_CLKPWR_DoPLLChange() function is called.
 */
void	NX_CLKPWR_SetPLLPMS ( U32 pllnumber, U32 PDIV, U32 MDIV, U32 SDIV )
{
	const U32 PLL_PDIV_BIT_POS =	18;
	const U32 PLL_MDIV_BIT_POS =	8;
	const U32 PLL_SDIV_BIT_POS =	0;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL	> pllnumber );
	NX_ASSERT( 1	<= PDIV && PDIV <= 63 );
	NX_ASSERT( (2 > pllnumber) && (64	<= MDIV && MDIV <= 1023) );
//	NX_ASSERT( (2 > pllnumber) && (3	>= SDIV) );
	NX_ASSERT( (2 <= pllnumber) && (13	<= MDIV && MDIV <= 255) );
//	NX_ASSERT( (2 <= pllnumber) && (3	>= SDIV) );
//	NX_ASSERT( 5 >= SDIV && 0 <= SDIV );
    NX_ASSERT( 5 >= SDIV );

	WriteIO32(&__g_pRegister->PLLSETREG[pllnumber],
		NX_BIT_SetBitRange32(ReadIO32(&__g_pRegister->PLLSETREG[pllnumber]),
				(PDIV<<PLL_PDIV_BIT_POS) | (MDIV<<PLL_MDIV_BIT_POS) | (SDIV<<PLL_SDIV_BIT_POS),
				23, 0));
}

U32		NX_CLKPWR_GetPLLFreq( U32 pllnumber )
{
	const U32 PLL_PDIV_BIT_POS =	18;
	const U32 PLL_MDIV_BIT_POS =	8;
	const U32 PLL_SDIV_BIT_POS =	0;
	const U32 PLL_KDIV_BIT_POS =	16;
	register U32 RegValue, RegValue1, nP, nM, nS, nK;
	U32 temp = 0;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL	> pllnumber );

	RegValue = ReadIO32(&__g_pRegister->PLLSETREG[pllnumber]);
	nP = (RegValue >> PLL_PDIV_BIT_POS) & 0x3F;
	nM = (RegValue >> PLL_MDIV_BIT_POS) & 0x3FF;
	nS = (RegValue >> PLL_SDIV_BIT_POS) & 0xFF;
	RegValue1 = ReadIO32(&__g_pRegister->PLLSETREG_SSCG[pllnumber]);
	nK = (RegValue1 >> PLL_KDIV_BIT_POS) & 0xFFFF;

	if ((pllnumber > 1) && nK) {
		temp = (U32)(((((nK * 1000) / 65536) * __g_OSC_KHz) / nP) >> nS);
	}

	temp = (U32)((((( nM * __g_OSC_KHz)/nP)>>nS)*1000) + temp);
	return temp;
}

void	NX_CLKPWR_SetPLLDither ( U32 pllnumber, S32 K, U32 MFR, U32 MRR, U32 SEL_PF, CBOOL SSCG_EN  )
{
	const U32 PLL_SSCG_EN_BIT_POS =	30;
	const U32 PLL_K_BIT_POS =	16;
	const U32 PLL_MFR_BIT_POS =	8;
	const U32 PLL_MRR_BIT_POS =	2;
	const U32 PLL_SEL_PF_BIT_POS =	0;
	U32 K32 = (U32)(U16)(K);

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL	> pllnumber );


	NX_ASSERT( (-32768	<= K && K <= 32767) );
	NX_ASSERT( (MFR <= 0x2f) );
	NX_ASSERT( (MRR <= 63) );
	NX_ASSERT( (SEL_PF <= 2) );

//	__g_pRegister->PLLSETREG[pllnumber] = (PDIV<<PLL_PDIV_BIT_POS) | (MDIV<<PLL_MDIV_BIT_POS) | (SDIV<<PLL_SDIV_BIT_POS);
	//WriteIO32(&__g_pRegister->PLLSETREG[pllnumber], (PDIV<<PLL_PDIV_BIT_POS) | (MDIV<<PLL_MDIV_BIT_POS) | (SDIV<<PLL_SDIV_BIT_POS));
	WriteIO32(&__g_pRegister->PLLSETREG_SSCG[pllnumber],
		NX_BIT_SetBitRange32(ReadIO32(&__g_pRegister->PLLSETREG_SSCG[pllnumber]),
				      ( K32<<PLL_K_BIT_POS  )
				 	| (MFR<<PLL_MFR_BIT_POS)
					| (MRR<<PLL_MRR_BIT_POS)
					| (SEL_PF<<PLL_SEL_PF_BIT_POS)
					, 31, 0));

	WriteIO32(&__g_pRegister->PLLSETREG[pllnumber],
		NX_BIT_SetBitRange32(ReadIO32(&__g_pRegister->PLLSETREG[pllnumber]),
				((U32)SSCG_EN),
				PLL_SSCG_EN_BIT_POS, PLL_SSCG_EN_BIT_POS));
}
#if 0
/*
 *	@brief		Set PLL1 power On/Off.
 *	@param[in]	On	 CTRUE	indicates that Normal mode . 
 *					 CFALSE	indicates that Power Down.
 *	@return		None.
 *	@remark		PLL1 is only available.

 */
void	NX_CLKPWR_SetPLLPowerOn ( CBOOL On )
{
	register U32 PLLPWDN1 = (U32)(1UL << 30);
	register U32 SetValue;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( 0 && "NX_CLKPWR_SetPLLPowerOn is removed, please use NX_CLKPWR_SetPLLPower" );

//	if( On )	__g_pRegister->CLKMODEREG0 &=	~PLLPWDN1;
//	else		__g_pRegister->CLKMODEREG0 |=	PLLPWDN1;

	SetValue = __g_pRegister->CLKMODEREG0;
	if( On )	SetValue &=	~PLLPWDN1;
	else		SetValue |=	PLLPWDN1;
	WriteIO32(&__g_pRegister->CLKMODEREG0, SetValue);
}
#endif
//------------------------------------------------------------------------------
/**
 *	@brief		Change PLL with P, M, S value on PLLSETREG.
 *	@return		None.
 *	@remark		If you call this function, you must check by NX_CLKPWR_IsPLLStable(),
 *				because PLL change need stable time.
 *				Therefore the sequence for changing PLL is as follows.
 *	@code
 *		NX_CLKPWR_SetPLLPMS( PLLn, P, M, S );	// Change P, M, S values
 *		NX_CLKPWR_DoPLLChange();				// Change PLL
 *		while( !NX_CLKPWR_IsPLLStable() );		// wait until PLL is stable.
 *	@endcode
 */
void	NX_CLKPWR_DoPLLChange ( void )
{
	const U32 CHGPLL = (1<<15);
	register U32 SetValue;

	NX_ASSERT( CNULL != __g_pRegister );

	SetValue = ReadIO32(&__g_pRegister->PWRMODE) | CHGPLL;
	WriteIO32(&__g_pRegister->PWRMODE, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check whether PLL are stable or not.
 *	@return		If PLL is stable, return  CTRUE. Otherwise, return  CFALSE.
 */
CBOOL	NX_CLKPWR_IsPLLStable ( void )
{
	const U32 CHGPLL = (1<<15);
	NX_ASSERT( CNULL != __g_pRegister );

	return ((ReadIO32(&__g_pRegister->PWRMODE) & CHGPLL ) ? CFALSE : CTRUE );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock for CPU and CPU Bus.
 *	@param[in]	ClkSrc		Source clock for CPU ( 0 : PLL0, 1 : PLL1 ).
 *	@param[in]	CoreDivider	Divider for CPU core clock, 1 ~ 16.
 *	@param[in]	BusDivider	Divider for CPU Bus clock, 2 ~ 16.
 *	@return		None.
 *	@remark		
 *				- CPU core clock = Source clock / CoreDivider, CoreDivider = 1 ~ 16
 *				- CPU bus clock = CPU core clock / BusDivider, BusDivider = 2 ~ 16
 */
void	NX_CLKPWR_SetClockCPU	( U32 ClkSrc, U32 CoreDivider, U32 BusDivider )
{
	const U32	CLKDIV1CPU0_POS	= 0;
	const U32	CLKSELCPU0_POS	= 4;
	const U32	CLKDIV2CPU0_POS	= 8;
	register U32 temp;

	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL > ClkSrc );
	NX_ASSERT( 1 <= CoreDivider && CoreDivider <= 16 );
	NX_ASSERT( 2 <= BusDivider && BusDivider <= 16 );
	NX_ASSERT( CNULL != __g_pRegister );

	temp	= (U32)( ((CoreDivider-1) << CLKDIV1CPU0_POS)
						| ((U32)ClkSrc << CLKSELCPU0_POS)
						| ((BusDivider-1)	<< CLKDIV2CPU0_POS) );

	WriteIO32(&__g_pRegister->CLKMODEREG0, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set clock divider of two stage
 *	@param[in]	ClkSrc		Source clock for CPU ( 0 : PLL0, 1 : PLL1 ).
 *	@param[in]	Divider1		Divider for 1st clock, 1 ~ 16.
 *	@param[in]	Divider2		Divider for 2nd clock, 2 ~ 16.
 *	@return		None.
 *	@remark		
 *				- Divider = 1 ~ 16
 */
void	NX_CLKPWR_SetClockDivider2	( NX_CLKPWR_CLOCK clock_number, U32 ClkSrc, U32 Divider1, U32 Divider2 )
{
	const U32	CLKDIV2_POS	= 9;
	const U32	CLKDIV1_POS	= 3;
	const U32	CLKSEL_POS	= 0;
	register U32 temp;

	NX_ASSERT( NX_CLKPWR_NUMBER_OF_CLOCK > clock_number );
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL > ClkSrc );
	NX_ASSERT( 1 <= Divider1 && Divider1 <= 16 );
	NX_ASSERT( 2 <= Divider2 && Divider2 <= 16 );
	NX_ASSERT( CNULL != __g_pRegister );

	temp	= (U32)(  ((Divider2-1) << CLKDIV2_POS)
					| ((Divider1-1)	<< CLKDIV1_POS)
					| ((U32)ClkSrc << CLKSEL_POS)
				    );
	WriteIO32(&__g_pRegister->DVOREG[clock_number], temp);
}

void	NX_CLKPWR_SetClockDivider3	( NX_CLKPWR_CLOCK clock_number, U32 ClkSrc, U32 Divider1, U32 Divider2, U32 Divider3 )
{
	const U32	CLKDIV3_POS	= 15;
	const U32	CLKDIV2_POS	= 9;
	const U32	CLKDIV1_POS	= 3;
	const U32	CLKSEL_POS	= 0;
	register U32 temp;

	// @todo tony
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_CLOCK > clock_number );
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL > ClkSrc );
	NX_ASSERT( 1 <= Divider1 && Divider1 <= 16 );
	NX_ASSERT( 2 <= Divider2 && Divider2 <= 16 );
	NX_ASSERT( 2 <= Divider3 && Divider3 <= 16 );
	NX_ASSERT( CNULL != __g_pRegister );

	temp	= (U32)(  ((Divider3-1) << CLKDIV3_POS)
		            | ((Divider2-1) << CLKDIV2_POS)
					| ((Divider1-1)	<< CLKDIV1_POS)
					| ((U32)ClkSrc << CLKSEL_POS)
				    );
	WriteIO32(&__g_pRegister->DVOREG[clock_number], temp);
}

void	NX_CLKPWR_SetClockDivider4	( NX_CLKPWR_CLOCK clock_number, U32 ClkSrc, U32 Divider1, U32 Divider2, U32 Divider3, U32 Divider4 )
{
	const U32	CLKDIV4_POS	= 21;
	const U32	CLKDIV3_POS	= 15;
	const U32	CLKDIV2_POS	= 9;
	const U32	CLKDIV1_POS	= 3;
	const U32	CLKSEL_POS	= 0;
	register U32 temp;

	// @todo tony
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_CLOCK > clock_number );
	NX_ASSERT( NX_CLKPWR_NUMBER_OF_PLL > ClkSrc );
	NX_ASSERT( 1 <= Divider1 && Divider1 <= 16 );
	NX_ASSERT( 1 <= Divider2 && Divider2 <= 16 );
	NX_ASSERT( 2 <= Divider3 && Divider3 <= 16 );
	NX_ASSERT( 2 <= Divider4 && Divider4 <= 16 );
	NX_ASSERT( CNULL != __g_pRegister );

	temp	= (U32)(  ((Divider4-1) << CLKDIV4_POS)
	                | ((Divider3-1) << CLKDIV3_POS)
		            | ((Divider2-1) << CLKDIV2_POS)
					| ((Divider1-1)	<< CLKDIV1_POS)
					| ((U32)ClkSrc  << CLKSEL_POS )
				   );
	WriteIO32(&__g_pRegister->DVOREG[clock_number], temp);
}
#if 0
//------------------------------------------------------------------------------
/**
 *	@brief		Set clock for MCLK, BCLK and PCKL.
 *	@param[in]	ClkSrc			Source clock for MCLK ( 0 : PLL0, 1 : PLL1, 3:FCLK )
 *	@param[in]	MCLKDivider		Divide value for MCLK, 1 ~ 16.
 *	@param[in]	BCLKDivider		Divide value for BCLK, 1 or 2.
 *	@param[in]	PCLKDivider		Divide value for PCLK, 2.
 *	@return		None.
 *	@remark
 *				MCLK is memory bus clock. It must be lesser than 400MHz. 
 *				- MCLK = PLL0 or PLL1 / MCLKDivider, MCLKDivider = 1 ~ 16.
 *
 *				BCLK is system bus clock. BCLK must set same clock of MCLK or half clock. 
 *				- BCLK = MCLK / BCLKDivider, BCLKDivider = 1 or 2.
 *
 *				PCLK is registe access clock.	PCLKDivide commonly set 2. 
 *				- PCLK = BCLK / PCLKDivider, PCLKDivider = 2.
 *
 */
void	NX_CLKPWR_SetClockMCLK( U32 ClkSrc, U32 MCLKDivider, U32 BCLKDivider, U32 PCLKDivider )
{
	const U32	CLKDIVMCLK_BITPOS	= 0;
	const U32	CLKSELMCLK_BITPOS	= 4;
	const U32	CLKDIVBCLK_BITPOS	= 8;
	const U32	CLKDIVPCLK_BITPOS	= 12;
	register U32 temp=0;

	NX_ASSERT( (3>=ClkSrc) && (2!=ClkSrc) );
	NX_ASSERT( 1 <= MCLKDivider && MCLKDivider <= 16 );
	NX_ASSERT( (1==BCLKDivider) || (2==BCLKDivider) );
	NX_ASSERT( 2==PCLKDivider );

	temp = (U32)( (ClkSrc << CLKSELMCLK_BITPOS)
						| ((MCLKDivider-1) << CLKDIVMCLK_BITPOS)
						| ((BCLKDivider-1) << CLKDIVBCLK_BITPOS)
						| ((PCLKDivider-1) << CLKDIVPCLK_BITPOS)
						);

	WriteIO32(&__g_pRegister->CLKMODEREG1, temp);
}

void NX_CLKPWR_SetCPUBUSSyncMode(CBOOL Enable)
{
	const U32 SYNCMODE = (1<<28);
	register U32 SetValue;

	NX_ASSERT( CNULL != __g_pRegister );

	SetValue = ReadIO32(&__g_pRegister->PWRMODE);
	if(Enable)
		SetValue |= SYNCMODE;
	else
		SetValue &= ~SYNCMODE;
	WriteIO32(&__g_pRegister->PWRMODE, SetValue);
}
#endif
//--------------------------------------------------------------------------
// Wakeup Management
//--------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable RTC Wakeup.
 *	@param[in]	Enable	Set to  CTRUE to enable RTC wakeup. Otherwise, set to  CFALSE.
 *	@return		None.
 *	@remark		Wakeup control for RTC.
 */
void	NX_CLKPWR_SetRTCWakeUpEnable ( CBOOL Enable )
{
	const U32 RTCWKENB_BITPOS	= 1;
	const U32 RTCWKENB_MASK		= 1 << RTCWKENB_BITPOS;

	register U32 ReadValue;

	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ReadIO32(&__g_pRegister->PWRCONT);

	ReadValue &= ~RTCWKENB_MASK;
	ReadValue |= (U32)Enable << RTCWKENB_BITPOS;

	WriteIO32(&__g_pRegister->PWRCONT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a status of RTC Wakeup Enable.
 *	@return	If RTC Wakeup is enabled, return  CTRUE. Otherwise, return  CFALSE.
 *	@remark		Wakeup control for RTC.
 */
CBOOL	NX_CLKPWR_GetRTCWakeUpEnable ( void )
{
	const U32 RTCWKENB_BITPOS = 1;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->PWRCONT) >> RTCWKENB_BITPOS ) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set wakeup source of ALIVEGPIO
 *	@param[in]	dwBitNumber		Select wakeup source (0 ~ 8). AliveGPIO(0~7), VDD Power Toggle Pin(8)
 *	@param[in]	bEnable			 CTRUE indicates that ALIVEGPIO is use wakeup source. 
 *								 CFALSE indicates that ALIVEGPIO is NOT use wakeup source. 
 *	@return		None.
 *	@remark		ALIVEGPIO only chose one function(reset or wakeup source). 
 */
void	NX_CLKPWR_SetALIVEGPIOWakeupEnable( U32 dwBitNumber, CBOOL bEnable )
{
	register U32 ReadValue;

	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ReadIO32(&__g_pRegister->GPIOWAKEUPENB);

	ReadValue &= ~(1 << dwBitNumber);
	ReadValue |= ((U32)bEnable << dwBitNumber);

	WriteIO32(&__g_pRegister->GPIOWAKEUPENB, ReadValue);
}

void	NX_CLKPWR_SetALIVEGPIOWakeupEnableAll(  CBOOL bEnable )
{
	U32 dwBitNumber;
	NX_ASSERT( CNULL != __g_pRegister );
	for (dwBitNumber=0; dwBitNumber < NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER; dwBitNumber++)
	{
		NX_CLKPWR_SetALIVEGPIOWakeupEnable(  dwBitNumber, bEnable );
	}
}


void	NX_CLKPWR_SetALIVEGPIOResetEnableAll(  CBOOL bEnable )
{
	U32 dwBitNumber;
	NX_ASSERT( CNULL != __g_pRegister );
	for (dwBitNumber=0; dwBitNumber < NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER; dwBitNumber++)
	{
		NX_CLKPWR_SetALIVEGPIOResetEnable(  dwBitNumber, bEnable );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of wakeup source
 *	@param[in]	dwBitNumber		Select wakeup source (0 ~ 8). AliveGPIO(0~7), VDD Power Toggle Pin(8)
 *	@return		bEnable			 CTRUE indicates that ALIVEGPIO is use wakeup source. 
 *								 CFALSE indicates that ALIVEGPIO is NOT use wakeup source. 
 */
CBOOL	NX_CLKPWR_GetALIVEGPIOWakeupEnable( U32 dwBitNumber )
{
	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->GPIOWAKEUPENB) >> dwBitNumber) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set enable or disable of ALIVEGPIO's rising edge detection.
 *	@param[in]	dwBitNumber		Select wakeup source (0 ~ 8). AliveGPIO(0~7), VDD Power Toggle Pin(8)
 *	@param[in]	bEnable			 CTRUE indicates that rising edge detection enable.
 *								 CFALSE indicates that rising edge detection disable.
 *	@return		None.
 */
void	NX_CLKPWR_SetALIVEGPIOWakeUpRiseEdgeDetectEnable( U32 dwBitNumber, CBOOL bEnable )
{
	register U32 ReadValue;

	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ReadIO32(&__g_pRegister->GPIOWAKEUPRISEENB);

	ReadValue &= ~(1 << dwBitNumber);
	ReadValue |= ((U32)bEnable << dwBitNumber);

	WriteIO32(&__g_pRegister->GPIOWAKEUPRISEENB, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of rising edge detection is enabled or disabled.
 *	@param[in]	dwBitNumber		Select wakeup source (0 ~ 8). AliveGPIO(0~7), VDD Power Toggle Pin(8)
 *	@return		bEnable			 CTRUE indicates that rising edge detection is enabled. 
 *								 CFALSE indicates that rising edge detection is disabled. 
 */
CBOOL	NX_CLKPWR_GetALIVEGPIOWakeUpRiseEdgeDetectEnable( U32 dwBitNumber )
{
	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->GPIOWAKEUPRISEENB) >> dwBitNumber) & 0x01);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set enable or disable of ALIVEGPIO's faling edge detection.
 *	@param[in]	dwBitNumber		Select wakeup source (0 ~ 8). AliveGPIO(0~7), VDD Power Toggle Pin(8)
 *	@param[in]	bEnable			 CTRUE indicates that falling edge detection enable.
 *								 CFALSE indicates that falling edge detection disable.
 *	@return		None.
 */
void	NX_CLKPWR_SetALIVEGPIOWakeUpFallEdgeDetectEnable( U32 dwBitNumber, CBOOL bEnable )
{
	register U32 ReadValue;

	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ReadIO32(&__g_pRegister->GPIOWAKEUPFALLENB);

	ReadValue &= ~(1 << dwBitNumber);
	ReadValue |= ((U32)bEnable << dwBitNumber);

	WriteIO32(&__g_pRegister->GPIOWAKEUPFALLENB, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get setting value of falling edge detection is enabled or disabled.
 *	@param[in]	dwBitNumber		Select wakeup source (0 ~ 8). AliveGPIO(0~7), VDD Power Toggle Pin(8)
 *	@return		 CTRUE	indicates that falling edge detection is enabled. 
 *				 CFALSE	indicates that falling edge detection is disabled. 
 */
CBOOL	NX_CLKPWR_GetALIVEGPIOWakeUpFallEdgeDetectEnable( U32 dwBitNumber )
{
	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->GPIOWAKEUPFALLENB) >> dwBitNumber) & 0x01);
}

//--------------------------------------------------------------------------
// Reset Management
//--------------------------------------------------------------------------
/** @brief		Set enable or disable of software reset.
 *	@param[in]	bEnable		 CTRUE indicates that software reset enable.
 *							 CFALSEindicates that software reset disable.
 *	@return		None.
 */
void	NX_CLKPWR_SetSoftwareResetEnable( CBOOL bEnable )
{
	const U32 SWRSTENB_BITPOS	= 3;
	const U32 SWRSTENB_MASK		= 1 << SWRSTENB_BITPOS;

	register U32 ReadValue;

	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ReadIO32(&__g_pRegister->PWRCONT);

	ReadValue &= ~SWRSTENB_MASK;
	ReadValue |= (U32)bEnable << SWRSTENB_BITPOS;

	WriteIO32(&__g_pRegister->PWRCONT, ReadValue);
}

//--------------------------------------------------------------------------
/** @brief		Get setting value of software reset is enabled or disabled.
 *	@return		 CTRUE	indicates that software reset is enabled.
 *				 CFALSE	indicates that software reset is disabled.
 */
CBOOL	NX_CLKPWR_GetSoftwareResetEnable( void )
{
	const U32 SWRSTENB_BITPOS	= 3;
	const U32 SWRSTENB_MASK		= 1 << SWRSTENB_BITPOS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->PWRCONT) & SWRSTENB_MASK) ? CTRUE : CFALSE ;
}

//--------------------------------------------------------------------------
/** @brief		Generate a software reset.
 *	@return		None.
 */
void	NX_CLKPWR_DoSoftwareReset( void )
{
	const U32 SWREST_BITPOS	= 12;
	const U32 SWREST_MASK	= 1 << SWREST_BITPOS;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->PWRMODE, SWREST_MASK);
}

//------------------------------------------------------------------------------
/** @brief		Set enable or disable of ALIVEGPIO's reset source.
 *	@param[in]	dwBitNumber		Select ALIVEGPIO (0 ~7).
 *	@param[in]	bEnable			 CTRUE indicates that ALIVEGPIO is use to reset source.
 *								 CFALSE indicates that ALIVEGPIO is NOT use to reset source.
 *	@return		None.
 *	@remark		ALIVEGPIO only chose one function(reset or wakeup source). 
 */
void	NX_CLKPWR_SetALIVEGPIOResetEnable( U32 dwBitNumber, CBOOL bEnable )
{
	register U32 ReadValue;

	NX_ASSERT( NX_CLKPWR_ALIVEGPIOWAKEUP_NUMBER > dwBitNumber );
	NX_ASSERT( (0==bEnable) || (1==bEnable) );
	NX_ASSERT( CNULL != __g_pRegister );

	ReadValue = ReadIO32(&__g_pRegister->GPIORSTENB);

	ReadValue &= ~(1 << dwBitNumber);
	ReadValue |= ((U32)bEnable << dwBitNumber);

	WriteIO32(&__g_pRegister->GPIORSTENB, ReadValue);
}

//------------------------------------------------------------------------------
/** @brief		Get setting value that ALIVEGPIO's reset source is enabled or disabled.
 *	@param[in]	dwBitNumber		Select ALIVEGPIO (0 ~7).
 *	@return		 CTRUE indicates that ALIVEGPIO's reset source is enabled.
 *				 CFALSEindicates that ALIVEGPIO's reset source is disabled.
 */
CBOOL	NX_CLKPWR_GetALIVEGPIOResetEnable( U32 dwBitNumber )
{
	NX_ASSERT( 8 > dwBitNumber );
	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->GPIORSTENB) >> dwBitNumber) & 0x01);
}

//------------------------------------------------------------------------------
// Check Last Reset Status
//------------------------------------------------------------------------------
/** @brief		Get last reset status
 *	@return		Reset status
 *	@remarks	Reset priority is 
 *				POR > GPIO > Watchdog > Software > DSleep > Sleep.
 *	@code
 *				NX_CLKPWR_RESETSTATUS ResetStatus;
 *
 *				ResetStatus = NX_CLKPWR_GetResetStatus();
 *
 *				if( NX_CLKPWR_RESETSTATUS_POWERON = ResetStatus )		// Last Reset status is "Power on Reset".
 *				{
 *					...
 *				}
 *				else if( NX_CLKPWR_RESETSTATUS_GPIO = ResetStatus )	// Last Reset status is "GPIO Reset".
 *				{
 *					...
 *				}
 *	@endcode
 */
NX_CLKPWR_RESETSTATUS NX_CLKPWR_GetResetStatus( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return (NX_CLKPWR_RESETSTATUS)(ReadIO32(&__g_pRegister->RESETSTATUS) & 0x1F);
}

/*
void NX_CLKPWR ClearResetStatus( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	//return (NX_CLKPWR_RESETSTATUS)(__g_pRegister->RESETSTATUS & 0x1F);

	const U32  CLEARMASK = (U32)(0x1f << 11);
	__g_pRegister->SCRATCHPAD |=  CLEARMASK;
	__g_pRegister->SCRATCHPAD &= ~CLEARMASK;
}
*/

//------------------------------------------------------------------------------
// Power Management
//------------------------------------------------------------------------------
/** @brief		Change operation mode ( Normal => Stop )
 *	@return		None.
 *	@see										NX_CLKPWR_GoIdleMode,
 *				NX_CLKPWR_GetLastPowerMode
 */
void	NX_CLKPWR_GoStopMode( void )
{
	const U32 STOP_MASK = 1<<1;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->PWRMODE, STOP_MASK);
}

void	NX_CLKPWR_SetImmediateSleepEnable( CBOOL enable  )
{

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->PWRCONT,
	NX_BIT_SetBit32(ReadIO32(&__g_pRegister->PWRCONT), enable, 0 ));
}


//------------------------------------------------------------------------------
/** @brief		Change operation mode ( Normal => Idle )
 *	@return		None.
 *	@see		NX_CLKPWR_GoStopMode,
 *				NX_CLKPWR_GetLastPowerMode
 */
void	NX_CLKPWR_GoIdleMode( void )
{
	const U32 IDLE_MASK = 1<<0;

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->PWRMODE, IDLE_MASK);
}

//--------------------------------------------------------------------------
/** @brief		Get Last Power Mode.
 *	@return		Last power mode.
 *	@code
 *				NX_CLKPWR_LASTPOWERMODE PowerMode = NX_CLKPWR_GetLastPowerMode();
 *
 *				if( NX_CLKPWR_POWERMODE_STOP == PowerMode )			// Last power mode is Stop.
 *				{
 *					...
 *				}
 *				else if( NX_CLKPWR_POWERMODE_IDLE == PowerMode )	// Last power mode is Idle.
 *				{
 *					...
 *				}
 *				...
 *	@endcode
 *	@remarks	Only one power mode is return.
 *				This bit(LASTPWRSTOP, LASTPWRIDLE) is cleared. when reset ocure.

 *	@see		NX_CLKPWR_GoStopMode,			NX_CLKPWR_GoIdleMode
 */
NX_CLKPWR_POWERMODE	NX_CLKPWR_GetLastPowerMode ( void )
{
	const U32 LASTPWRIDLE_BITPOS = 4;
	const U32 LASTPWR_MASK = 0x03 << LASTPWRIDLE_BITPOS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (NX_CLKPWR_POWERMODE)(ReadIO32(&__g_pRegister->PWRMODE) & LASTPWR_MASK );
}

//------------------------------------------------------------------------------
// Scratch
//------------------------------------------------------------------------------
/** @brief		Set data value to scratch register.
 *	@param[in]	dwIndex		Index of scratch register(0~2).
 *	@param[in]	dwValue		save value.
 *	@return		none.
 *	@remarks	scratch regiser is cleared when coure power reset.
 *	@see		NX_CLKPWR_GetScratch
 */
void	NX_CLKPWR_SetScratchPad( U32 dwIndex, U32 dwValue )
{
	NX_ASSERT( 3 > dwIndex );
	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->SCRATCH[dwIndex], dwValue);
}

//------------------------------------------------------------------------------
/** @brief		Get saved value of scratch register
 *	@param[in]	dwIndex		Index of scratch register(0~2).
 *	@return		data value
 *	@remarks	scratch regiser is cleared when core power reset.
 *	@see		NX_CLKPWR_SetScratch
 */
U32		NX_CLKPWR_GetScratchPad( U32 dwIndex )
{
	NX_ASSERT( 3 > dwIndex );
	NX_ASSERT( CNULL != __g_pRegister );

	return ReadIO32(&__g_pRegister->SCRATCH[dwIndex]);
}

//------------------------------------------------------------------------------
/** @brief		Get system configuration value.
 *	@return		None.
 */
U32		NX_CLKPWR_GetSystemResetConfiguration( void )
{
	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)ReadIO32(&__g_pRegister->SYSRSTCONFIG);
}

//------------------------------------------------------------------------------
// CPU Power Management
//------------------------------------------------------------------------------
void	NX_CLKPWR_SetCPUPowerOff( U32 nCPU )
{
	register U32 regvalue;

	NX_ASSERT( nCPU	< 8 );

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = 1 << nCPU;

	WriteIO32(&__g_pRegister->CPUPOWERDOWNREQ, regvalue);
}

void	NX_CLKPWR_SetCPUPowerOff32( U32 CPUbits )
{
	NX_ASSERT( CPUbits	< 0x100 );

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->CPUPOWERDOWNREQ, CPUbits);
}

void	NX_CLKPWR_SetCPUPowerOn( U32 nCPU )
{
	register U32 regvalue;

	NX_ASSERT( nCPU	< 8 );

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = 1 << nCPU;

	WriteIO32(&__g_pRegister->CPUPOWERONREQ, regvalue);
}

void	NX_CLKPWR_SetCPUPowerOn32( U32 CPUbits )
{
	NX_ASSERT( CPUbits	< 0x100 );

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->CPUPOWERONREQ, CPUbits);
}

void	NX_CLKPWR_SetCPUResetMode( NX_CLKPWR_CPU_RESETMODE mode )
{
	NX_ASSERT(	(NX_CLKPWR_CPU_RESETMODE_SAFE == mode) ||
				(NX_CLKPWR_CPU_RESETMODE_DIRECT == mode) );

	NX_ASSERT( CNULL != __g_pRegister );

	WriteIO32(&__g_pRegister->CPUWARMRESETREQ, mode);
}

void	NX_CLKPWR_SetCPUWarmReset( U32 nCPU )
{
	register U32 regvalue;

	NX_ASSERT( nCPU	< 8 );

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = 1 << nCPU;

	WriteIO32(&__g_pRegister->CPUWARMRESETREQ, regvalue);
}

CBOOL	NX_CLKPWR_GetCPUPowerOnStatus( U32 nCPU )
{
	register U32 regvalue;

	NX_ASSERT( nCPU	< 8 );

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = ReadIO32(&__g_pRegister->CPUSTATUS);

	return ((regvalue >> 8) & (1 << nCPU) ? CFALSE : CTRUE);
}

U32		NX_CLKPWR_GetCPUPowerOnStatus32( void )
{
	register U32 regvalue;

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = ReadIO32(&__g_pRegister->CPUSTATUS);

	return ((~regvalue >> 8) & 0xFF);
}

CBOOL	NX_CLKPWR_GetCPUPowerOffStatus( U32 nCPU )
{
	register U32 regvalue;

	NX_ASSERT( nCPU	< 8 );

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = ReadIO32(&__g_pRegister->CPUSTATUS);

	return ((regvalue >> 8) & (1 << nCPU) ? CTRUE : CFALSE);
}

U32		NX_CLKPWR_GetCPUPowerOffStatus32( void )
{
	register U32 regvalue;

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = ReadIO32(&__g_pRegister->CPUSTATUS);

	return ((regvalue >> 8) & 0xFF);
}

U32		NX_CLKPWR_GetCPUClockOnStatus( void )
{
	register U32 regvalue;

//	NX_ASSERT( nCPU	< 8 );

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = ReadIO32(&__g_pRegister->CPUSTATUS);

	return (regvalue & 0xFF);
}

//------------------------------------------------------------------------------
void	NX_CLKPWR_SetPllOutGMux(U32 pllnumber, NX_CLKPWR_GMUX gmux)
{
	NX_ASSERT( CNULL != __g_pRegister );
	WriteIO32(&__g_pRegister->PLLSETREG[pllnumber],
		NX_BIT_SetBit32(ReadIO32(&__g_pRegister->PLLSETREG[pllnumber]), gmux, 28 ));
}



void	NX_CLKPWR_UpdatePllSetReg(U32 pllnumber)
{
	const U32 UPDATE_PLL		= 0x01;
	NX_ASSERT( CNULL != __g_pRegister );
	WriteIO32(&__g_pRegister->CLKMODEREG0,
		NX_BIT_SetBit32(ReadIO32(&__g_pRegister->CLKMODEREG0), UPDATE_PLL, pllnumber));
}

CBOOL	NX_CLKPWR_IsPLLStableUpdate(void)
{
	const U32 UPDATEPLL = (1<<31);
	NX_ASSERT( CNULL != __g_pRegister );
	return ((ReadIO32(&__g_pRegister->CLKMODEREG0) & UPDATEPLL ) ? CFALSE : CTRUE );
}


void NX_CLKPWR_SetPLLPower( U32 pllnumber, NX_CLKPWR_PLL_POWER powermode )
{
	NX_ASSERT( CNULL != __g_pRegister );
	WriteIO32(&__g_pRegister->PLLSETREG[pllnumber],
		NX_BIT_SetBit32(ReadIO32(&__g_pRegister->PLLSETREG[pllnumber]), powermode, 29 ));
}

