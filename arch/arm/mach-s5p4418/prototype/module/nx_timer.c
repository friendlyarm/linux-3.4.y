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
//	Module		: TIMER
//	File		: nx_timer.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 2012.08.30 first implementation
//				  2014.08.05 hans modify to functionality
//				  2014.10.15 deoks modify to functionality
//				  2015.05.20 dahye modify to functionality
//------------------------------------------------------------------------------
#include "nx_timer.h"

static struct NX_TIMER_RegisterSet *__g_pRegister[5];


//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE		indicates that Initialize is successed.
 *			CFALSE		indicates that Initialize is failed.
 */
CBOOL	NX_TIMER_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister[0] = (struct NX_TIMER_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return bInit;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32		NX_TIMER_GetNumberOfModule( void )
{
	return NUMBER_OF_TIMER_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of submodules in the timertimer.
 *	@return		SubModule's number.
 */
U32		NX_TIMER_GetNumberOfChannel( void )
{
	return NUMBER_OF_TIMER_CHANNEL;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 */
U32		NX_TIMER_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( TIMER ) };

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_TIMER_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_TIMER_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress		Module's base address
 *	@return		None.
 */
void	NX_TIMER_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister[ModuleIndex] = (struct NX_TIMER_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 */
U32		NX_TIMER_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	return (U32)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Initialize is successed.
 *				CFALSE			indicates that Initialize is failed.
 */
CBOOL	NX_TIMER_OpenModule( U32 ModuleIndex )
{
	register struct NX_TIMER_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister	=	__g_pRegister[ModuleIndex];

	// check reset value
	NX_ASSERT( ReadIO32(&pRegister->TCFG0) 		== 0x00000101 );
	NX_ASSERT( ReadIO32(&pRegister->TCFG1) 		== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCON) 		== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTB0) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCMPB0) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTO0) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTB1) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCMPB1) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTO1) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTB2) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCMPB2) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTO2) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTB3) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCMPB3) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTO3) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTB4) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TCNTO4) 	== 0x00000000 );
	NX_ASSERT( ReadIO32(&pRegister->TINT_CSTAT)	== 0x00000000 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Deinitialize is successed. 
 *				CFALSE			indicates that Deinitialize is failed.
 */
CBOOL	NX_TIMER_CloseModule( U32 ModuleIndex )
{
	register struct NX_TIMER_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister	=	__g_pRegister[ModuleIndex];

	// set up reset value
	WriteIO32(&pRegister->TCFG0, 	0x00000101 );
	WriteIO32(&pRegister->TCFG1, 	0x00000000 );
	WriteIO32(&pRegister->TCON,	 	0x00000000 );
	WriteIO32(&pRegister->TCNTB0,	0x00000000 );
	WriteIO32(&pRegister->TCMPB0,	0x00000000 );
	WriteIO32(&pRegister->TCNTO0,	0x00000000 );
	WriteIO32(&pRegister->TCNTB1,	0x00000000 );
	WriteIO32(&pRegister->TCMPB1,	0x00000000 );
	WriteIO32(&pRegister->TCNTO1,	0x00000000 );
	WriteIO32(&pRegister->TCNTB2,	0x00000000 );
	WriteIO32(&pRegister->TCMPB2,	0x00000000 );
	WriteIO32(&pRegister->TCNTO2,	0x00000000 );
	WriteIO32(&pRegister->TCNTB3,	0x00000000 );
	WriteIO32(&pRegister->TCMPB3,	0x00000000 );
	WriteIO32(&pRegister->TCNTO3,	0x00000000 );
	WriteIO32(&pRegister->TCNTB4,	0x00000000 );
	WriteIO32(&pRegister->TCNTO4,	0x00000000 );
	WriteIO32(&pRegister->TINT_CSTAT, 0x00000000 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Module is Busy. 
 *				CFALSE			indicates that Module is NOT Busy.
 */
CBOOL	NX_TIMER_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that Ready to enter power-down stage. 
 *				CFALSE			indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_TIMER_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_TIMER?_MODULE in <nx_chip.h>.
 */
U32 NX_TIMER_GetClockNumber ( U32 ModuleIndex, U32 Channel )
{
	const U32 ClockNumber[] =
	{
//		CLOCKINDEX_LIST( TIMER ),
		CLOCKINDEX_OF_TIMER_MODULE,
		CLOCKINDEX_OF_Inst_TIMER01_MODULE,
		CLOCKINDEX_OF_Inst_TIMER02_MODULE,
		CLOCKINDEX_OF_Inst_TIMER03_MODULE,
		CLOCKINDEX_OF_TIMER_MODULE
	};
	
    NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
    NX_ASSERT( NUMBER_OF_TIMER_CHANNEL > Channel );

	return	ClockNumber[Channel];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_TIMER?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_TIMER_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( TIMER, PRESETn )
	};
	NX_CASSERT( NUMBER_OF_TIMER_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

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
U32		NX_TIMER_GetInterruptNumber( U32 ModuleIndex, U32 Channel )
{
	const U32	TIMERInterruptNumber[][4] =
				{
					{ INTNUM_WITH_CHANNEL_LIST(TIMER,INT0) },
					{ INTNUM_WITH_CHANNEL_LIST(TIMER,INT1) },
					{ INTNUM_WITH_CHANNEL_LIST(TIMER,INT2) },
					{ INTNUM_WITH_CHANNEL_LIST(TIMER,INT3) },
				};

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_OF_TIMER_CHANNEL > Channel );

	return	TIMERInterruptNumber[ModuleIndex][Channel];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum			Interrupt Number.
 *	@param[in]	Enable	CTRUE	indicates that Interrupt Enable.
 *						CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 */
void	NX_TIMER_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	register U32	ReadValue;
	const U32		TIMER_ENB_MASK	= 0x1F;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_OF_TIMER_CHANNEL > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	ReadValue	=	ReadIO32(&pRegister->TINT_CSTAT) & TIMER_ENB_MASK;

	ReadValue	&=	(U32)(~(1UL << IntNum));
	ReadValue	|=	(U32)Enable << IntNum ;

	WriteIO32(&pRegister->TINT_CSTAT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum			Interrupt Number.
 *	@return		CTRUE			indicates that Interrupt is enabled. 
 *				CFALSE			indicates that Interrupt is disabled.
 */
CBOOL	NX_TIMER_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_TIMER_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_OF_TIMER_CHANNEL > IntNum );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	return	(CBOOL)( (ReadIO32(&pRegister->TINT_CSTAT) >> IntNum) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum			Interrupt Number.
 *	@return		CTRUE			indicates that Pending is seted. 
 *				CFALSE			indicates that Pending is Not Seted.
 */
CBOOL	NX_TIMER_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	const U32	PEND_POS	=	5;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_OF_TIMER_CHANNEL > IntNum );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	return	(CBOOL)( (ReadIO32(&pRegister->TINT_CSTAT) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum			Interrupt number.
 *	@return		None.
 */
void	NX_TIMER_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f;
	register U32 PendEnb;


	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_OF_TIMER_CHANNEL > IntNum );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	PendEnb	=	ReadIO32(&pRegister->TINT_CSTAT) & PEND_MASK;
	PendEnb |=	1UL<<(IntNum+PEND_POS);

	WriteIO32(&pRegister->TINT_CSTAT, PendEnb );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	CTRUE	indicates that Set to all interrupt enable. 
 *						CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_TIMER_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );
	NX_ASSERT( (CFALSE==Enable) || (CTRUE==Enable) );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );


	if( Enable )
		SetValue	=	0x1F;
	else
		SetValue	=	0;

	WriteIO32(&pRegister->TINT_CSTAT, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		CTRUE			indicates that At least one( or more ) interrupt is enabled. 
 *				CFALSE			indicates that All interrupt is disabled.
 */
CBOOL	NX_TIMER_GetInterruptEnableAll( U32 ModuleIndex )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	const U32	ENABLE_POS	=	0;
	const U32	INT_MASK	=	0x1f;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	if( ReadIO32(&pRegister->TINT_CSTAT) & (INT_MASK << ENABLE_POS) )
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
CBOOL	NX_TIMER_GetInterruptPendingAll( U32 ModuleIndex )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	if( ReadIO32(&pRegister->TINT_CSTAT) & (PEND_MASK << PEND_POS) )
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
void	NX_TIMER_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register struct NX_TIMER_RegisterSet*	pRegister;
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f;

	register U32 PendEnb;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	PendEnb	=	ReadIO32(&pRegister->TINT_CSTAT) & PEND_MASK;
	PendEnb |=	PEND_MASK<<PEND_POS;

	WriteIO32(&pRegister->TINT_CSTAT, PendEnb );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
U32		NX_TIMER_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f << PEND_POS;
	register U32 PendingIndex, Pend;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > ModuleIndex );

	pRegister	=	__g_pRegister[ModuleIndex];

	NX_ASSERT( CNULL != pRegister );

	Pend	=	(ReadIO32(&pRegister->TINT_CSTAT) & PEND_MASK)>>PEND_POS;

	for( PendingIndex = 0; PendingIndex<NX_TIMER_INT; PendingIndex++)
		if(Pend & (1UL<<PendingIndex))
			break;

	if(PendingIndex > NX_TIMER_INT)
		return -1;
	else
		return PendingIndex;
}


//--------------------------------------------------------------------------
//	Configuration operations
//--------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Set prescaler ( register value 1 is bypass )
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	value		divide value (1 ~ 0xFF).
 *	@return		none.
 */
void	NX_TIMER_SetPrescaler(U32 Channel, U32 value)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 regvalue;
	const U32 PRSC_MASK	= 0xFF;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	NX_ASSERT( (0x100 > value) && (0 < value) );

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCFG0);

	if(Channel & 0x6)
	{
		regvalue &= ~(1UL<<8);
		regvalue |= ((value-1) & PRSC_MASK)<<8;		// timer channel 2, 3, 4
	}
	else
	{
		regvalue &= ~(1UL<<0);
		regvalue |= ((value-1) & PRSC_MASK)<<0;		// timer channel 0, 1
	}

	WriteIO32(&pRegister->TCFG0, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get prescaler ( register value 1 is bypass )
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		divide value (1 ~ 0xFF).
 */
U32		NX_TIMER_GetPrescaler(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 regvalue;
	const U32 PRSC_MASK	= 0xFF;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCFG0);
	
	if(Channel & 0x6)
		return ((regvalue>>8) & PRSC_MASK)+1;		// timer channel 2, 3, 4
	else
		return ((regvalue>>0) & PRSC_MASK)+1;		// timer channel 0, 1
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Dead Zone width
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	Length		value (0 ~ 0xFE).
 *	@return		none.
 */
void	NX_TIMER_SetDeadZoneLength(U32 Channel, U32 Length)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 regvalue;
	const U32	DEADZ_MASK	=	0xff;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCFG0);
	regvalue &= ~(DEADZ_MASK<<16);
	regvalue |= (Length & DEADZ_MASK)<<16;
	WriteIO32(&pRegister->TCFG0, regvalue);
}
//------------------------------------------------------------------------------
/**
 *	@brief		Get Dead Zone width
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		width value (0 ~ 0xFE).
 */
U32		NX_TIMER_GetDeadZoneLength(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	const U32	DEADZ_MASK	=	0xff;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	return (U32)(ReadIO32(&pRegister->TCFG0)>>16)&DEADZ_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Dead Zone run
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	Enable		CTRUE: enable, CFALSE: disable.
 *	@return		none.
 */
CBOOL	NX_TIMER_SetDeadZoneEnable(U32 Channel, CBOOL Enable)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 regvalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	NX_ASSERT( (CFALSE==Enable) || (CTRUE == Enable) );

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(Channel != 0)
		return CFALSE;

	regvalue = ReadIO32(&pRegister->TCON);
	if(Enable)
		regvalue |= (1UL<<4);
	else
		regvalue &= ~(1UL<<4);

	WriteIO32(&pRegister->TCON, regvalue);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Dead Zone run status
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		CTRUE 		Dead zone is working
 				CFALSE 		Not working.
 */
CBOOL	NX_TIMER_GetDeadZoneEnable(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(Channel != 0)
		return CFALSE;

	if(ReadIO32(&pRegister->TCON) & (1UL<<4))
		return CTRUE;

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set divider path
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	divider		divider select.
 							(0: DIV1, 1: DIV2, 2: DIV4, 3: DIV8, 4: DIV16)
 *	@return		CTRUE : this channel can select which selecting.
 *				CFALSE: channel cannot select selecting path divider channel.
 */
CBOOL	NX_TIMER_SetDividerPath(U32 Channel, NX_TIMER_DIVIDSELECT divider)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue, updatevalue;
	const U32	DIV_MASK	=	0xF;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;
	NX_ASSERT( NX_TIMER_DIVIDSELECT_TCLK >= divider );

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 4 && NX_TIMER_DIVIDSELECT_TCLK == divider)
		return CFALSE;

	updatevalue = divider<<(4*modulechannel);

	regvalue = ReadIO32(&pRegister->TCFG1);
	regvalue &= ~(DIV_MASK<<modulechannel);
	regvalue |= updatevalue;
	WriteIO32(&pRegister->TCFG1, regvalue);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get divider path
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		current Divider path number
 */
NX_TIMER_DIVIDSELECT NX_TIMER_GetDividerPath(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue;
	const U32	DIV_MASK	=	0xF;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	regvalue = ReadIO32(&pRegister->TCFG1);
	regvalue >>= (4*modulechannel);
	regvalue &= DIV_MASK;

	return (NX_TIMER_DIVIDSELECT)regvalue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set Output Invert
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	Enable		CTRUE: enable, CFALSE: disable.
 *	@return		CTRUE		this channel can invert output signal.
 *				CFALSE 		this channel has no output port, so cannot invert signal.
 */
CBOOL	NX_TIMER_SetOutInvert(U32 Channel, CBOOL Enable)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue, updatevalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	NX_ASSERT( (CFALSE==Enable) || (CTRUE == Enable) );

	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;
	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 4)
		return CFALSE;

	if(modulechannel == 0)
		updatevalue = 1UL<<(4*(modulechannel+0)+2);
	else
		updatevalue = 1UL<<(4*(modulechannel+1)+2);

	regvalue = ReadIO32(&pRegister->TCON);
	if(Enable)
		regvalue |= updatevalue;
	else
		regvalue &= ~updatevalue;

	WriteIO32(&pRegister->TCON, regvalue);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get Output Invert status
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		CTRUE	this channel is inverted output signal.
 *				CFALSE	channel has no output port or not inverted.
 */
CBOOL	NX_TIMER_GetOutInvert(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, comparevalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );

	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;
	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 4)
		return CFALSE;

	if(modulechannel == 0)
		comparevalue = 1UL<<(4*(modulechannel+0)+2);
	else
		comparevalue = 1UL<<(4*(modulechannel+1)+2);

	if(comparevalue & ReadIO32(&pRegister->TCON))
		return CTRUE;

	return CFALSE;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set LoadMode is AutoReload or OneShot
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	ShotMode	LoadMode (0:OneShot, 1:AutoReload)
 *	@return		None.
 */
void	NX_TIMER_SetShotMode(U32 Channel, NX_TIMER_LOADMODE ShotMode)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;
	NX_ASSERT( (NX_TIMER_LOADMODE_ONESHOT == ShotMode) || (NX_TIMER_LOADMODE_AUTORELOAD == ShotMode));

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCON);

	if(modulechannel == 0)
	{
		regvalue &= ~(1UL<<3);
		regvalue |= ShotMode<<3;
	}
	else if(modulechannel & 0x3 )
	{
		regvalue &= ~(1UL<<(4*(modulechannel+1)+3));
		regvalue |= ShotMode<<(4*(modulechannel+1)+3);
	}
	else
	{
		regvalue &= ~(1UL<<22);
		regvalue |= ShotMode<<22;
	}

	WriteIO32(&pRegister->TCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set LoadMode is AutoReload or OneShot
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return 	ShotMode	LoadMode (0:OneShot, 1:AutoReload)
 */

NX_TIMER_LOADMODE	NX_TIMER_GetShotMode(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCON);

	if(modulechannel == 0)
	{
		regvalue >>= 3;
	}
	
	if(modulechannel == 4)
	{
		regvalue >>= 22;
	}else
	{
		regvalue >>= (4*(modulechannel+1)+3);
	}
	regvalue &= 0x1;

	return (NX_TIMER_LOADMODE)regvalue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set TCNTx, TCMPx Manual Update
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	Enable	CTRUE	Indicates that Manual Update is enabled.
 						CFALSE	Indicates that Manual Update is disabled.
 *	@return		None.
 */
void	NX_TIMER_UpdateCounter(U32 Channel, CBOOL Enable )
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue, updatedonevalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCON);
	if(modulechannel == 0)
	{
		regvalue |= Enable<<1;
		updatedonevalue &= ~(1UL<<1);
	}
	else
	{
		regvalue |= 1<<(4*(modulechannel+1)+1);
		updatedonevalue &= ~(1<<(4*(modulechannel+1)+1));
	}

	WriteIO32(&pRegister->TCON, regvalue);
	WriteIO32(&pRegister->TCON, updatedonevalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set TIMER Start(Run)
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		None.
 */
void	NX_TIMER_Run(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel % NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCON);

	if(modulechannel == 0)
	{
		regvalue |= 1UL<<0;
	}
	else
	{
		regvalue |= 1<<(4*(modulechannel+1));
	}

	WriteIO32(&pRegister->TCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set TIMER Stop
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		None.
 */
void	NX_TIMER_Stop(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCON);

	if(modulechannel == 0)
	{
		regvalue &= ~(1UL<<0);
	}
	else
	{
		regvalue &= ~(1<<(4*(modulechannel+1)));
	}

	WriteIO32(&pRegister->TCON, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get TIMER Running state.
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		None.
 */
CBOOL	NX_TIMER_IsRun(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel, regvalue;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->TCON);

	if(modulechannel == 0)
	{
		regvalue >>= 0;
	}
	else
	{
		regvalue >>= (4*(modulechannel+1));
	}
	regvalue &= 0x1;

	return (CBOOL)regvalue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set TIMER Period (Count).
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	Period		TIMER Period ( range : 0 ~ 0xFFFFFFFF )
 *	@return		None.
 */
void	NX_TIMER_SetPeriod(U32 Channel, U32 Period)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel % NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 0)
	{
		WriteIO32(&pRegister->TCNTB0, Period);
	}
	else if(modulechannel == 1)
	{
		WriteIO32(&pRegister->TCNTB1, Period);
	}
	else if(modulechannel == 2)
	{
		WriteIO32(&pRegister->TCNTB2, Period);
	}
	else if(modulechannel == 3)
	{
		WriteIO32(&pRegister->TCNTB3, Period);
	}	
	else if(modulechannel == 4)
	{
		WriteIO32(&pRegister->TCNTB4, Period);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set TIMER Period (Count).
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		Period		TIMER Period ( range : 0 ~ 0xFFFFFFFF )
 */
U32		NX_TIMER_GetPeriod(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];
	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 0)
	{
		return ReadIO32(&pRegister->TCNTB0);
	}
	else if(modulechannel == 1)
	{
		return ReadIO32(&pRegister->TCNTB1);;
	}
	else if(modulechannel == 2)
	{
		return ReadIO32(&pRegister->TCNTB2);
	}
	else if(modulechannel == 3)
	{
		return ReadIO32(&pRegister->TCNTB3);
	}	
	else if(modulechannel == 4)
	{
		return ReadIO32(&pRegister->TCNTB4);
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set TIMER Duty (Compare).
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@param[in]	Duty		TIMER Duty ( range : 0 ~ 0xFFFFFFFF )
 *	@return		None.
 */
CBOOL	NX_TIMER_SetDuty(U32 Channel, U32 Duty)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );


	if(modulechannel == 0)
	{
		WriteIO32(&pRegister->TCMPB0, Duty);;
	}
	else if(modulechannel == 1)
	{
		WriteIO32(&pRegister->TCMPB1, Duty);
	}
	else if(modulechannel == 2)
	{
		WriteIO32(&pRegister->TCMPB2, Duty);
	}
	else if(modulechannel == 3)
	{
		WriteIO32(&pRegister->TCMPB3, Duty);
	}	
	else if(modulechannel == 4)
	{
		return CFALSE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get TIMER Duty (Compare).
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		Duty		TIMER Duty. ( range : 0 ~ 0xFFFFFFFF )
 */
U32		NX_TIMER_GetDuty(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel%NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 0)
	{
		return ReadIO32(&pRegister->TCMPB0);
	}
	else if(modulechannel == 1)
	{
		return ReadIO32(&pRegister->TCMPB1);;
	}
	else if(modulechannel == 2)
	{
		return ReadIO32(&pRegister->TCMPB2);
	}
	else if(modulechannel == 3)
	{
		return ReadIO32(&pRegister->TCMPB3);
	}	
	else if(modulechannel == 4)
	{
		return CFALSE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get TIMER Current Count.
 *	@param[in]	Channel		An index of timer channel ( 0 ~ x ).
 *	@return		Count		TIMER Count. ( range : 0 ~ x )
 */
U32		NX_TIMER_GetCurrentCount(U32 Channel)
{
	register struct NX_TIMER_RegisterSet	*pRegister;
	register U32 modulechannel;

	NX_ASSERT( NUMBER_OF_TIMER_MODULE > Channel/NUMBER_OF_TIMER_CHANNEL );
	modulechannel = Channel % NUMBER_OF_TIMER_CHANNEL;

	pRegister	=	__g_pRegister[Channel/NUMBER_OF_TIMER_CHANNEL];

	NX_ASSERT( CNULL != pRegister );

	if(modulechannel == 0)
	{
		return ReadIO32(&pRegister->TCNTO0);
	}
	else if(modulechannel == 1)
	{
		return ReadIO32(&pRegister->TCNTO1);;
	}
	else if(modulechannel == 2)
	{
		return ReadIO32(&pRegister->TCNTO2);
	}
	else if(modulechannel == 3)
	{
		return ReadIO32(&pRegister->TCNTO3);
	}	
	else if(modulechannel == 4)
	{
		return ReadIO32(&pRegister->TCNTO4);
	}

	return CTRUE;
}
//@}

