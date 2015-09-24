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
//	Module		: PWM
//	File		: nx_pwm.c
//	Description	:
//	Author		: Jonghyuk Park(charles@nexell.co.kr)
//	History		: 20120830 first implementation
//------------------------------------------------------------------------------
#include "nx_pwm.h"

static	struct
{
	struct NX_PWM_RegisterSet *pRegister;

} __g_ModuleVariables[NUMBER_OF_PWM_MODULE] = { {CNULL, }, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicates that Initialize is successed.\r\n
 *			\b CFALSE indicates that Initialize is failed.\r\n
 *	@see	NX_PWM_GetNumberOfModule
 */
CBOOL	NX_PWM_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for( i=0; i < NUMBER_OF_PWM_MODULE; i++ )
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
 *	@see		NX_PWM_Initialize
 */
U32		NX_PWM_GetNumberOfModule( void )
{
	return NUMBER_OF_PWM_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of submodules in the pwmtimer.
 *	@return		SubModule's number.
 *	@see		NX_PWM_Initialize
 */
U32		NX_PWM_GetNumberOfSubModule( void )
{
	return NUMBER_PWMTIMER_SUBMODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's physical address
 *	@see		NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_SetBaseAddress,		NX_PWM_GetBaseAddress,
 *				NX_PWM_OpenModule,			NX_PWM_CloseModule,
 *				NX_PWM_CheckBusy,			NX_PWM_CanPowerDown
 */
U32		NX_PWM_GetPhysicalAddress( U32 ModuleIndex )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( PWM ) };

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );

	return (U32)PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_PWM_GetPhysicalAddress,
 *				NX_PWM_SetBaseAddress,		NX_PWM_GetBaseAddress,
 *				NX_PWM_OpenModule,			NX_PWM_CloseModule,
 *				NX_PWM_CheckBusy,			NX_PWM_CanPowerDown
 */
U32		NX_PWM_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_PWM_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_PWM_GetPhysicalAddress,	NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_GetBaseAddress,
 *				NX_PWM_OpenModule,			NX_PWM_CloseModule,
 *				NX_PWM_CheckBusy,			NX_PWM_CanPowerDown
 */
void	NX_PWM_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );

	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_PWM_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Module's base address.
 *	@see		NX_PWM_GetPhysicalAddress,	NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_SetBaseAddress,
 *				NX_PWM_OpenModule,			NX_PWM_CloseModule,
 *				NX_PWM_CheckBusy,			NX_PWM_CanPowerDown
 */
U32		NX_PWM_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );

	return (U32)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Initialize is successed. \r\n
 *				\b CFALSE	indicates that Initialize is failed.
 *	@see		NX_PWM_GetPhysicalAddress,	NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_SetBaseAddress,		NX_PWM_GetBaseAddress,
 *				NX_PWM_CloseModule,
 *				NX_PWM_CheckBusy,			NX_PWM_CanPowerDown
 */
CBOOL	NX_PWM_OpenModule( U32 ModuleIndex )
{
	register struct NX_PWM_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// check reset value
	NX_ASSERT( ReadIODW(&pRegister->TCFG0) 		== 0x00000101 );
	NX_ASSERT( ReadIODW(&pRegister->TCFG1) 		== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCON) 		== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTB0) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCMPB0) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTO0) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTB1) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCMPB1) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTO1) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTB2) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCMPB2) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTO2) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTB3) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCMPB3) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTO3) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTB4) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TCNTO4) 	== 0x00000000 );
	NX_ASSERT( ReadIODW(&pRegister->TINT_CSTAT)	== 0x00000000 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Deinitialize is successed. \r\n
 *				\b CFALSE	indicates that Deinitialize is failed.
 *	@see		NX_PWM_GetPhysicalAddress,	NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_SetBaseAddress,		NX_PWM_GetBaseAddress,
 *				NX_PWM_OpenModule,
 *				NX_PWM_CheckBusy,			NX_PWM_CanPowerDown
 */
CBOOL	NX_PWM_CloseModule( U32 ModuleIndex )
{
	register struct NX_PWM_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	// set up reset value
	WriteIODW(&pRegister->TCFG0, 	0x00000101 );
	WriteIODW(&pRegister->TCFG1, 	0x00000000 );
	WriteIODW(&pRegister->TCON,	 	0x00000000 );
	WriteIODW(&pRegister->TCNTB0,	0x00000000 );
	WriteIODW(&pRegister->TCMPB0,	0x00000000 );
	WriteIODW(&pRegister->TCNTO0,	0x00000000 );
	WriteIODW(&pRegister->TCNTB1,	0x00000000 );
	WriteIODW(&pRegister->TCMPB1,	0x00000000 );
	WriteIODW(&pRegister->TCNTO1,	0x00000000 );
	WriteIODW(&pRegister->TCNTB2,	0x00000000 );
	WriteIODW(&pRegister->TCMPB2,	0x00000000 );
	WriteIODW(&pRegister->TCNTO2,	0x00000000 );
	WriteIODW(&pRegister->TCNTB3,	0x00000000 );
	WriteIODW(&pRegister->TCMPB3,	0x00000000 );
	WriteIODW(&pRegister->TCNTO3,	0x00000000 );
	WriteIODW(&pRegister->TCNTB4,	0x00000000 );
	WriteIODW(&pRegister->TCNTO4,	0x00000000 );
	WriteIODW(&pRegister->TINT_CSTAT, 0x00000000 );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Module is Busy. \r\n
 *				\b CFALSE	indicates that Module is NOT Busy.
 *	@see		NX_PWM_GetPhysicalAddress,	NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_SetBaseAddress,		NX_PWM_GetBaseAddress,
 *				NX_PWM_OpenModule,			NX_PWM_CloseModule,
 *				NX_PWM_CanPowerDown
 */
CBOOL	NX_PWM_CheckBusy( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that Ready to enter power-down stage. \r\n
 *				\b CFALSE	indicates that This module can't enter to power-down stage.
 *	@see		NX_PWM_GetPhysicalAddress,	NX_PWM_GetSizeOfRegisterSet,
 *				NX_PWM_SetBaseAddress,		NX_PWM_GetBaseAddress,
 *				NX_PWM_OpenModule,			NX_PWM_CloseModule,
 *				NX_PWM_CheckBusy
 */
CBOOL	NX_PWM_CanPowerDown( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );

	return CTRUE;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.\n
 *				It is equal to CLOCKINDEX_OF_PWM?_MODULE in <nx_chip.h>.
 *	@see		NX_CLKGEN_SetClockDivisorEnable,
 *				NX_CLKGEN_GetClockDivisorEnable,
 *				NX_CLKGEN_SetClockSource,
 *				NX_CLKGEN_GetClockSource,
 *				NX_CLKGEN_SetClockDivisor,
 *				NX_CLKGEN_GetClockDivisor
 */
//U32 NX_PWM_GetClockNumber ( U32 ModuleIndex )
//{
//	const U32 ClockNumber[] =
//	{
//		CLOCKINDEX_LIST( PWM )
//	};
//	NX_CASSERT( NUMBER_OF_PWM_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
//    NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
//	return	ClockNumber[ModuleIndex];
//}
U32 NX_PWM_GetClockNumber ( U32 ModuleIndex, U32 SubModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( PWM ),
		CLOCKINDEX_OF_Inst_PWM01_MODULE,
		CLOCKINDEX_OF_Inst_PWM02_MODULE,
		CLOCKINDEX_OF_Inst_PWM03_MODULE
	};
	//NX_CASSERT( NUMBER_OF_PWM_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
    NX_ASSERT( NUMBER_PWMTIMER_SUBMODULE > SubModuleIndex );

	return	ClockNumber[SubModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_PWM?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_PWM_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( PWM, PRESETn )
	};
	NX_CASSERT( NUMBER_OF_PWM_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Interrupt number
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
U32		NX_PWM_GetInterruptNumber( U32 ModuleIndex, U32 SubModuleIndex )
{
	const U32	PWMInterruptNumber[][4] =
				{
					{ INTNUM_WITH_CHANNEL_LIST(PWM,INT0) },
					{ INTNUM_WITH_CHANNEL_LIST(PWM,INT1) },
					{ INTNUM_WITH_CHANNEL_LIST(PWM,INT2) },
					{ INTNUM_WITH_CHANNEL_LIST(PWM,INT3) },
				};

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_PWMTIMER_SUBMODULE > SubModuleIndex );

	return	PWMInterruptNumber[ModuleIndex][SubModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@param[in]	Enable	\b CTRUE	indicates that Interrupt Enable. \r\n
 *						\b CFALSE	indicates that Interrupt Disable.
 *	@return		None.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
void	NX_PWM_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	PEND_MASK	=	0x1f << PEND_POS;

	register struct NX_PWM_RegisterSet*	pRegister;
	register U32	ReadValue;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( 5 > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	ReadValue	=	ReadIODW(&pRegister->TINT_CSTAT) & ~PEND_MASK;

	ReadValue	&=	(U32)(~(1UL << (IntNum+PEND_POS)));
	ReadValue	|=	(U32)Enable << (IntNum+PEND_POS) ;

	WriteIODW(&pRegister->TINT_CSTAT, ReadValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Interrupt is enabled. \r\n
 *				\b CFALSE	indicates that Interrupt is disabled.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
CBOOL	NX_PWM_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	0;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( 5 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicates that Pending is seted. \r\n
 *				\b CFALSE	indicates that Pending is Not Seted.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
CBOOL	NX_PWM_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	5;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( 5 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return	(CBOOL)( (ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT) >> (IntNum+PEND_POS)) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ 5 ).
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
void	NX_PWM_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f;
	register U32 PendEnb;

	register struct NX_PWM_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( 5 > IntNum );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	PendEnb	=	ReadIODW(&pRegister->TINT_CSTAT) & PEND_MASK;

	WriteIODW(&pRegister->TINT_CSTAT, (((1 << IntNum) & PEND_MASK) << PEND_POS ) | PendEnb );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@param[in]	Enable	\b CTRUE	indicates that Set to all interrupt enable. \r\n
 *						\b CFALSE	indicates that Set to all interrupt disable.
 *	@return		None.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
void	NX_PWM_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0x1f;

	register U32	SetValue;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	SetValue	=	0;

	if( Enable )
	{
		SetValue	|=	INT_MASK << PEND_POS;
	}

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT, SetValue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that At least one( or more ) interrupt is enabled. \r\n
 *				\b CFALSE	indicates that All interrupt is disabled.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
CBOOL	NX_PWM_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	0;
	const U32	INT_MASK	=	0x1f;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT) & (INT_MASK << PEND_POS) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		\b CTRUE	indicates that At least one( or more ) pending is seted. \r\n
 *				\b CFALSE	indicates that All pending is NOT seted.
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
CBOOL	NX_PWM_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	if( ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT) & (PEND_MASK << PEND_POS) )
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
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
void	NX_PWM_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f;

	register struct NX_PWM_RegisterSet*	pRegister;
	register U32 PendEnb;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister	=	__g_ModuleVariables[ModuleIndex].pRegister;

	PendEnb	=	ReadIODW(&pRegister->TINT_CSTAT) & PEND_MASK;

	WriteIODW(&pRegister->TINT_CSTAT, ((PEND_MASK<<PEND_POS) | PendEnb) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@param[in]	ModuleIndex		An index of module ( 0 ~ x ).
 *	@return		Pending Number( If all pending is not set then return -1 ).
 *	@see		NX_PWM_GetInterruptNumber,
 *				NX_PWM_GetInterruptEnable,			NX_PWM_GetInterruptPending,
 *				NX_PWM_ClearInterruptPending,		NX_PWM_SetInterruptEnableAll,
 *				NX_PWM_GetInterruptEnableAll,		NX_PWM_GetInterruptPendingAll,
 *				NX_PWM_ClearInterruptPendingAll,	NX_PWM_GetInterruptPendingNumber
 */
U32		NX_PWM_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const U32	PEND_POS	=	5;
	const U32	PEND_MASK	=	0x1f << PEND_POS;
	register U32 PendingIndex = 0;

	register struct NX_PWM_RegisterSet	*pRegister;
	register U32 Pend;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;

	Pend	=	(ReadIODW(&pRegister->TINT_CSTAT)>>PEND_POS) & PEND_MASK;

	for( PendingIndex=0 ; PendingIndex<=20 ; PendingIndex++)
		if(Pend & ((U32)0x1)<<PendingIndex)
			break;

	if(PendingIndex > 20)
		return -1;
	else
		return PendingIndex;
}


//--------------------------------------------------------------------------
// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_PWM_SetTCFG0( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCFG0, value);
}
U32		NX_PWM_GetTCFG0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCFG0));
}

void	NX_PWM_SetTCFG1( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCFG1, value);
}
U32		NX_PWM_GetTCFG1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCFG1));
}

//void	NX_PWM_SetTCON( U32 ModuleIndex, U32 value )
//{
//	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );
//
//	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCON, value);
//}
void	NX_PWM_SetTCON( U32 ModuleIndex, U32 SubModuleIndex, U32 value )
{
	register U32 regvalue = 0;

	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( NUMBER_PWMTIMER_SUBMODULE > SubModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	regvalue = ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCON);
	if(SubModuleIndex==0){
		NX_ASSERT( (0x1<<5) > value );
		regvalue = regvalue & ~(0x1f<<PWM_TCON_TIMER0);
		regvalue = regvalue	| (value << PWM_TCON_TIMER0);
	}
	else if(SubModuleIndex==1){
		NX_ASSERT( (0x1<<4) > value );
		regvalue = regvalue & ~(0xf<<PWM_TCON_TIMER1);
		regvalue = regvalue	| (value << PWM_TCON_TIMER1);
	}
	else if(SubModuleIndex==2){
		NX_ASSERT( (0x1<<4) > value );
		regvalue = regvalue & ~(0xf<<PWM_TCON_TIMER2);
		regvalue = regvalue	| (value << PWM_TCON_TIMER2);
	}
	else if(SubModuleIndex==3){
		NX_ASSERT( (0x1<<4) > value );
		regvalue = regvalue & ~(0xf<<PWM_TCON_TIMER3);
		regvalue = regvalue	| (value << PWM_TCON_TIMER3);
	}

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCON, regvalue);
}

U32		NX_PWM_GetTCON( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCON));
}

////////////////////////////////////////////////////////////////
void	NX_PWM_SetTCNTB0( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB0, value);
}
U32		NX_PWM_GetTCNTB0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB0));
}

void	NX_PWM_SetTCMPB0( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB0, value);
}
U32		NX_PWM_GetTCMPB0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB0));
}

U32		NX_PWM_GetTCNTO0( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTO0));
}

////////////////////////////////////////////////////////////////
void	NX_PWM_SetTCNTB1( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB1, value);
}
U32		NX_PWM_GetTCNTB1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB1));
}

void	NX_PWM_SetTCMPB1( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB1, value);
}
U32		NX_PWM_GetTCMPB1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB1));
}

U32		NX_PWM_GetTCNTO1( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTO1));
}

////////////////////////////////////////////////////////////////
void	NX_PWM_SetTCNTB2( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB2, value);
}
U32		NX_PWM_GetTCNTB2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB2));
}

void	NX_PWM_SetTCMPB2( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB2, value);
}
U32		NX_PWM_GetTCMPB2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB2));
}

U32		NX_PWM_GetTCNTO2( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTO2));
}

////////////////////////////////////////////////////////////////
void	NX_PWM_SetTCNTB3( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB3, value);
}
U32		NX_PWM_GetTCNTB3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB3));
}

void	NX_PWM_SetTCMPB3( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB3, value);
}
U32		NX_PWM_GetTCMPB3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCMPB3));
}

U32		NX_PWM_GetTCNTO3( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTO3));
}

////////////////////////////////////////////////////////////////
void	NX_PWM_SetTCNTB4( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB4, value);
}
U32		NX_PWM_GetTCNTB4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTB4));
}

U32		NX_PWM_GetTCNTO4( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TCNTO4));
}

////////////////////////////////////////////////////////////////
void	NX_PWM_SetTINT_CSTAT( U32 ModuleIndex, U32 value )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	WriteIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT, value);
}
U32		NX_PWM_GetTINT_CSTAT( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_PWM_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables[ModuleIndex].pRegister );

	return (U32)(ReadIODW(&__g_ModuleVariables[ModuleIndex].pRegister->TINT_CSTAT));
}
//@}