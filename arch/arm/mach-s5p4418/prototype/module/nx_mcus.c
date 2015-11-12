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
//	Module		: MCUS
//	File		: nx_mcus.c
//	Description	:
//	Author		: Firmware Team
//	History		:
//------------------------------------------------------------------------------

#include "nx_mcus.h"


//static	struct NX_MCUS_RegisterSet *__g_pRegister[NUMBER_OF_MCUSTOP_MODULE];
static	struct NX_MCUS_RegisterSet *__g_pRegister;

static inline U32	__NX_MCUS_NFCONTROL_RESET_BIT(register U32 regvalue)
{
	const U32 HWBOOT_W = 6;	/* set 0 */
	const U32 EXSEL_R  = 6;
	const U32 EXSEL_W  = 5;

	register U32 tmpvalue;
	tmpvalue = regvalue;

	regvalue &= ~((1<<HWBOOT_W) | (1<<EXSEL_W));
	regvalue |= ((tmpvalue>>EXSEL_R) & 0x1) << EXSEL_W;

	return regvalue;
}

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicate that Initialize is successed.
 *			CFALSE	indicate that Initialize is failed.
 *	@see	NX_MCUS_GetNumberOfModule
 */
CBOOL	NX_MCUS_Initialize( void )
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
 */
U32		NX_MCUS_GetNumberOfModule( void )
{
	return NUMBER_OF_MCUSTOP_MODULE;
}


//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_MCUS_GetPhysicalAddress( void )
{
	//return	(U32)( PHY_BASEADDR_MCUS_MODULE + (OFFSET_OF_MCUS_MODULE * 0) );
    const U32 PhysicalAddr[NUMBER_OF_MCUSTOP_MODULE] =
	{
		PHY_BASEADDR_LIST( MCUSTOP )
	};
	return	PhysicalAddr[0];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_MCUS_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_MCUS_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_MCUS_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	__g_pRegister = (struct NX_MCUS_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_MCUS_GetBaseAddress( void )
{
	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. 
 *				CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_MCUS_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicate that Deinitialize is successed. 
 *				CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_MCUS_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicate that Module is Busy. 
 *				CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_MCUS_CheckBusy( void )
{
	return CFALSE;
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for interrupt controller.
 *	@return		Interrupt number
 */
S32		NX_MCUS_GetInterruptNumber( void )
{
	return	INTNUM_OF_MCUSTOP_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number .
 *	@param[in]	Enable	CTRUE	indicate that Interrupt Enable. 
 *						CFALSE	indicate that Interrupt Disable.
 *	@return		None.
 *	@remarks	MCUS has one interrupt source, therefore you have to set @a IntNum
 *				as '0' only.
 */
void	NX_MCUS_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
	const U32 IRQPEND_POS	= 15;
	const U32 ECCRST_POS	= 11;
	const U32 IRQENB_POS	= 8;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST | IRQENB);
	regvalue |= (U32)Enable<<IRQENB_POS;

	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

void	NX_MCUS_SetECCInterruptEnable( S32 IntNum, CBOOL Enable )
{
	const U32 IRQPEND_POS	= 14;
	const U32 ECCRST_POS	= 11;
	const U32 IRQENB_POS	= 7;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST | IRQENB);
	regvalue |= (U32)Enable<<IRQENB_POS;

	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that specified interrupt is enabled. 
 *				CFALSE indicates that specified interrupt is disabled.
 *	@remarks	MCUS has one interrupt source, therefore you have to set @a IntNum
 *				as '0' only.
 */
CBOOL	NX_MCUS_GetInterruptEnable( S32 IntNum )
{
	const U32 IRQENB_POS	= 8;
	const U32 IRQENB_MASK	= (1UL<<IRQENB_POS);

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (ReadIO32(&__g_pRegister->NFCONTROL) & IRQENB_MASK) >> IRQENB_POS );
}

CBOOL	NX_MCUS_GetECCInterruptEnable( S32 IntNum )
{
	const U32 IRQENB_POS	= 7;
	const U32 IRQENB_MASK	= (1UL<<IRQENB_POS);

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (ReadIO32(&__g_pRegister->NFCONTROL) & IRQENB_MASK) >> IRQENB_POS );
}


//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		CTRUE	indicates that specified interrupr is pended. 
 *				CFALSE indicates that specified interrupt is not pended.
 *	@remarks	MCUS has one interrupt source, therefore you have to set @a IntNum
 *				as '0' only.
 */
CBOOL	NX_MCUS_GetInterruptPending( S32 IntNum )
{
	const U32 IRQPEND_POS	= 15;
	const U32 IRQPEND_MASK	= (1UL<<IRQPEND_POS);

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (ReadIO32(&__g_pRegister->NFCONTROL) & IRQPEND_MASK) >> IRQPEND_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@remarks	MCUS has one interrupt source, therefore you have to set @a IntNum
 *				as '0' only.
 */
void	NX_MCUS_ClearInterruptPending( S32 IntNum )
{
	const U32 IRQPEND_POS	= 15;
	const U32 ECCRST_POS	= 11;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST );
	regvalue |= IRQPEND;
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

void	NX_MCUS_ClearECCInterruptPending( S32 IntNum )
{
	const U32 IRQPEND_POS	= 14;
	const U32 ECCRST_POS	= 11;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( 0 == IntNum );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST );
	regvalue |= IRQPEND;
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	Enable	Set this as CTURE to enable all interrupts. 
 *						Set this as CFALSE to disable all interrupts.
 *	@return		None.
 */
void	NX_MCUS_SetInterruptEnableAll( CBOOL Enable )
{
	const U32 IRQPEND_POS	= 15;
	const U32 ECCRST_POS	= 11;
	const U32 IRQENB_POS	= 8;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST | IRQENB);
	regvalue |= ( (U32)Enable<<IRQENB_POS );
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		CTRUE	indicates that one or more interrupts are enabled. 
 *				CFALSE	indicates that all interrupts are disabled.
 */
CBOOL	NX_MCUS_GetInterruptEnableAll( void )
{
	const U32 IRQENB_POS	= 8;
	const U32 IRQENB_MASK	= (1UL<<IRQENB_POS);

	NX_ASSERT( CNULL != __g_pRegister );

	return	(CBOOL)( (ReadIO32(&__g_pRegister->NFCONTROL) & IRQENB_MASK) >> IRQENB_POS );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of unmasked interrupts are pended or not.
 *	@return		CTURE indicates there's unmasked interrupts which are pended.
 *				CFALSE indicates there's no unmasked interrupt which are pended.
 *	@remark		Since this function doesn't consider about pending status of
 *				interrupts which are disabled, the return value can be CFALSE
 *				even though some interrupts are pended unless a relevant
 *				interrupt is enabled.
 */
CBOOL	NX_MCUS_GetInterruptPendingAll( void )
{
	const U32 IRQPEND_POS	= 15;
	const U32 IRQENB_POS	= 8;

	const U32 IRQPEND_MASK	= (1UL<<IRQPEND_POS);
	const U32 IRQENB_MASK	= (1UL<<IRQENB_POS);

	register U32 regval, intpend, intenb;

	NX_ASSERT( CNULL != __g_pRegister );

	regval	= ReadIO32(&__g_pRegister->NFCONTROL);
	intpend	= (regval & IRQPEND_MASK) >> IRQPEND_POS;
	intenb	= (regval & IRQENB_MASK ) >> IRQENB_POS;

	return	(CBOOL)( intpend & intenb );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@return		None.
 */
void	NX_MCUS_ClearInterruptPendingAll( void )
{
	const U32 IRQPEND_POS	= 15;
	const U32 ECCRST_POS	= 11;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST );
	regvalue |= IRQPEND;
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set interrupts to be enabled or disabled.
 *	@param[in]	EnableFlag		Specifies an interrupt enable flag that each bit
 *								represents an interrupt enable status to be changed -
 *								Value of 0 masks interrupt and value of 1 enables
 *								interrupt. EnableFlag[0] are only valid.
 *	@return		None.
 */
void	NX_MCUS_SetInterruptEnable32 ( U32 EnableFlag )
{
	const U32 IRQPEND_POS	= 15;
	const U32 ECCRST_POS	= 11;
	const U32 IRQENB_POS	= 8;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST | IRQENB);
	regvalue |= ( (U32)(EnableFlag & 1)<<IRQENB_POS );
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

void	NX_MCUS_SetECCInterruptEnable32 ( U32 EnableFlag )
{
	const U32 IRQPEND_POS	= 14;
	const U32 ECCRST_POS	= 11;
	const U32 IRQENB_POS	= 7;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST | IRQENB);
	regvalue |= ( (U32)(EnableFlag & 1)<<IRQENB_POS );
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt enable status.
 *	@return		An interrupt enable status that each bit represents current
 *				interrupt enable status - Value of 0 indicates relevant interrupt
 *				is masked and value of 1 indicates relevant interrupt is enabled.
 */
U32	NX_MCUS_GetInterruptEnable32 ( void )
{
	const U32 IRQENB_POS	= 8;
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFCONTROL) & IRQENB)>>IRQENB_POS;
}

U32	NX_MCUS_GetECCInterruptEnable32 ( void )
{
	const U32 IRQENB_POS	= 7;
	const U32 IRQENB		= (1UL<<IRQENB_POS);

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFCONTROL) & IRQENB)>>IRQENB_POS;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt pending status.
 *	@return		An interrupt pending status that each bit represents current
 *				interrupt pending status - Value of 0 indicates relevant interrupt
 *				is not pended and value of 1 indicates relevant interrupt is pended.
 */
U32	NX_MCUS_GetInterruptPending32 ( void )
{
	const U32 IRQPEND_POS	= 15;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFCONTROL) & IRQPEND)>>IRQPEND_POS;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear an interrupt pending status.
 *	@param[in]	PendingFlag		Specifies an interrupt pending clear flag. An
 *								interrupt pending status cleared only if
 *								corresponding bit in PendingFlag is set.
 *	@return		None.
 */
void	NX_MCUS_ClearInterruptPending32( U32 PendingFlag )
{
	const U32 IRQPEND_POS	= 15;
	const U32 ECCRST_POS	= 11;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST);
	regvalue |= ( (U32)(PendingFlag & 1)<<IRQPEND_POS );
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}

void	NX_MCUS_ClearECCInterruptPending32( U32 PendingFlag )
{
	const U32 IRQPEND_POS	= 14;
	const U32 ECCRST_POS	= 11;
	const U32 IRQPEND		= (1UL<<IRQPEND_POS);
	const U32 ECCRST		= (1UL<<ECCRST_POS);

	register U32 regvalue;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( CNULL != __g_pRegister );

	pRegister = __g_pRegister;

	regvalue = ReadIO32(&pRegister->NFCONTROL);
	regvalue &= ~(IRQPEND | ECCRST);
	regvalue |= ( (U32)(PendingFlag & 1)<<IRQPEND_POS );
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&pRegister->NFCONTROL, regvalue);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get an interrupt number which has the most prority of pended interrupts.
 *	@return		an interrupt number which has the most priority of pended and
 *				unmasked interrupts.
 *				If there's no interrupt which is pended and unmasked, it returns -1.
 */
S32		NX_MCUS_GetInterruptPendingNumber( void )
{
	const U32 IRQPEND_POS	= 15;
	const U32 IRQENB_POS	= 8;

	const U32 IRQPEND_MASK	= (1UL<<IRQPEND_POS);
	const U32 IRQENB_MASK	= (1UL<<IRQENB_POS);

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);

	if( (regval & IRQENB_MASK) && (regval & IRQPEND_MASK) )
	{
		return 0;
	}

	return -1;
}


//------------------------------------------------------------------------------
//	Static BUS Configuration.
//------------------------------------------------------------------------------
/**
 *	@brief		Get the static bus index of internal SRAM.
 *	@return		the static bus index of internal SRAM which is one of @ref NX_MCUS_SBUSID enum.
 *	@remark		The internal SRAM has different static bus index according to
 *				the external configuration - CfgBootMode.
 *				For NANDBOOT or UARTBOOT, the internal SRAM must be at static
 *				bus 0 to be mapped at 0x0 of memory address.
 *				For booting of ROM or NOR flash memory, static bus 0 should be
 *				used for these memory. therefore the internal SRAM must be
 *				other static bus except static bus 0 - static bus 6 is used for
 *				the internal SRAM.
 */
NX_MCUS_SBUSID		NX_MCUS_GetISRAMCS( void )
{
	const U32 ISRAMSHADOW = (1UL<<31);

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->MEMBW) & ISRAMSHADOW) ? NX_MCUS_SBUSID_STATIC12 : NX_MCUS_SBUSID_STATIC0;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the static bus index of internal ROM.
 *	@return		the static bus index of internal ROM which is one of @ref NX_MCUS_SBUSID enum.
 *	@remark		The internal ROM has different static bus index according to
 *				the external configuration - CfgBootMode.
 *				For iROMBOOT, the internal ROM must be at static bus 0 to be
 *				mapped at 0x0 of memory address.
 *				For other boot mode, static bus 0 should be used for other
 *				memory. therefore the internal ROM must be other static bus
 *				except static bus 0 - static bus 7 is used for the internal ROM.
 */
NX_MCUS_SBUSID		NX_MCUS_GetIROMCS( void )
{
	const U32 IROMSHADOW = (1UL<<30);

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->MEMBW) & IROMSHADOW) ? NX_MCUS_SBUSID_STATIC13 : NX_MCUS_SBUSID_STATIC0;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set static BUS configration.
 *	@param[in]	Id			Static BUS ID which is one of @ref NX_MCUS_SBUSID enum.
 *	@param[in]	BitWidth	BUS bit width. ( it must be 8 or 16 ).  This argument is only valid in case of Static #0 ~ #7.
 *	@param[in]	tACS		address setup time before nCS, 0 ~ 15 for Static #0 ~ #7 and NAND, 1 ~ 16 for IDE.
 *	@param[in]	tCAH		address hold time after nCS, 0 ~ 15 for Static #0 ~ #7 and NAND, 1 ~ 16 for IDE.
 *	@param[in]	tCOS		chip select setup time before nOE, 0 ~ 15 for Static #0 ~ #7 and NAND, 1 ~ 16 for IDE.
 *	@param[in]	tCOH		chip select hold time after nOE, 0 ~ 15 for Static #0 ~ #7 and NAND, 1 ~ 16 for IDE.
 *	@param[in]	tACC		data access time, 1 ~ 256 for Static #0 ~ #7, NAND and IDE.
 *	@param[in]	tSACC		data access time in burst access for Static #0 ~ #7, 1 ~ 256.wait access time for IDE, 1 ~ 256.
 *	@param[in]	WaitMode	Wait control  This argument is only valid in case of Static #0 ~ #7 and IDE.
 *	@param[in]	BurstRead	Burst mode control in reading. This argument is only valid in case of Static #0 ~ #7.
 *	@param[in]	BurstWrite	Burst mode control in writing. This argument is only valid in case of Static #0 ~ #7.
 *	@return		None.
 */
void	NX_MCUS_SetStaticBUSConfig
(
	NX_MCUS_SBUSID Id, U32 BitWidth,
	U32 tACS, U32 tCAH, U32 tCOS, U32 tCOH, U32 tACC, U32 tSACC,
	NX_MCUS_WAITMODE WaitMode,	NX_MCUS_BURSTMODE BurstRead, NX_MCUS_BURSTMODE BurstWrite
)
{
	register U32 bitpos2, bitpos4, bitpos8, regindex4, regindex8;
	register struct NX_MCUS_RegisterSet*	pRegister;
	register U32 regval;

	NX_ASSERT( (NX_MCUS_SBUSID_STATIC13>=Id) || (NX_MCUS_SBUSID_IDE==Id) || (NX_MCUS_SBUSID_NAND==Id) );
	NX_ASSERT( 1 <=	tACC &&	tACC <= 256 );
	NX_ASSERT( (1 <= tSACC && tSACC <= 256) || (NX_MCUS_SBUSID_NAND==Id) );
	NX_ASSERT( (8 == BitWidth || 16 == BitWidth) || (NX_MCUS_SBUSID_STATIC13 < Id) );
	NX_ASSERT( (4 > BurstRead) || (NX_MCUS_SBUSID_STATIC13 < Id) );
	NX_ASSERT( (4 > BurstWrite) || (NX_MCUS_SBUSID_STATIC13 < Id) );
	NX_ASSERT( 4 > WaitMode );

	NX_ASSERT( CNULL != __g_pRegister );

	//--------------------------------------------------------------------------
	pRegister	=	__g_pRegister;

	bitpos2		= ((U32)Id) * 2;

	bitpos4		= ((U32)Id) * 4;
	regindex4	= bitpos4 / 32;
	bitpos4		= bitpos4 % 32;

	bitpos8		= ((U32)Id) * 8;
	regindex8	= bitpos8 / 32;
	bitpos8		= bitpos8 % 32;

	if( NX_MCUS_SBUSID_IDE == Id )
	{
		NX_ASSERT( 0 < tACS && 16 >= tACS );
		NX_ASSERT( 0 < tCAH && 16 >= tCAH );
		NX_ASSERT( 0 < tCOS && 16 >= tCOS );
		NX_ASSERT( 0 < tCOH && 16 >= tCOH );

		//--------------------------------------------------------------------------
		// Set timing for tACS, tCOS, tOCH.
		// 0 : 1 cycle, 1 : 2 cycle, 2 : 3 cycle, ......, 15 : 16 cycle
		tACS	-= 1;
		tCOS	-= 1;
		tCOH	-= 1;

		// Set timing for tCAH
		// 0 : 2 cycle, 1 : 3 cycle, 2 : 4 cycle, ......, 15 : 1 cycle
		tCAH = ((tCAH-1) + 15) % 16;
	}
	else
	{
		NX_ASSERT( 16 > tACS );
		NX_ASSERT( 16 > tCAH );
		NX_ASSERT( 16 > tCOS );
		NX_ASSERT( 16 > tCOH );

		//--------------------------------------------------------------------------
		// Set timing for tACS, tCAH, tCOS, tOCH.
		// 0 : 1 cycle, 1 : 2 cycle, 2 : 3 cycle, ......, 15 : 0 cycle
		tACS = (tACS + 15) % 16;
		tCAH = (tCAH + 15) % 16;
		tCOS = (tCOS + 15) % 16;
		tCOH = (tCOH + 15) % 16;
	}

	tACC	-= 1;	// tACC	= n+1 cycle
	tSACC -= 1;	// tSACC = n+1 cycle

	//--------------------------------------------------------------------------
//	pRegister->MEMTIMEACS[regindex4] = (pRegister->MEMTIMEACS[regindex4] & ~(0x0FUL<<bitpos4)) | (tACS<<bitpos4);
//	pRegister->MEMTIMECAH[regindex4] = (pRegister->MEMTIMECAH[regindex4] & ~(0x0FUL<<bitpos4)) | (tCAH<<bitpos4);
//	pRegister->MEMTIMECOS[regindex4] = (pRegister->MEMTIMECOS[regindex4] & ~(0x0FUL<<bitpos4)) | (tCOS<<bitpos4);
//	pRegister->MEMTIMECOH[regindex4] = (pRegister->MEMTIMECOH[regindex4] & ~(0x0FUL<<bitpos4)) | (tCOH<<bitpos4);
//	pRegister->MEMTIMEACC[regindex8] = (pRegister->MEMTIMEACC[regindex8] & ~(0xFFUL<<bitpos8)) | (tACC<<bitpos8);
	regval = ReadIO32(&pRegister->MEMTIMEACS[regindex4]);
	regval &= ~(0x0FUL<<bitpos4);
	regval |= (tACS<<bitpos4);
	WriteIO32(&pRegister->MEMTIMEACS[regindex4], regval);

	regval = ReadIO32(&pRegister->MEMTIMECAH[regindex4]);
	regval &= ~(0x0FUL<<bitpos4);
	regval |= (tCAH<<bitpos4);
	WriteIO32(&pRegister->MEMTIMECAH[regindex4], regval);

	regval = ReadIO32(&pRegister->MEMTIMECOS[regindex4]);
	regval &= ~(0x0FUL<<bitpos4);
	regval |= (tCOS<<bitpos4);
	WriteIO32(&pRegister->MEMTIMECOS[regindex4], regval);

	regval = ReadIO32(&pRegister->MEMTIMECOH[regindex4]);
	regval &= ~(0x0FUL<<bitpos4);
	regval |= (tCOH<<bitpos4);
	WriteIO32(&pRegister->MEMTIMECOH[regindex4], regval);

	regval = ReadIO32(&pRegister->MEMTIMEACC[regindex8]);
	regval &= ~(0xFFUL<<bitpos8);
	regval |= (tACC<<bitpos8);
	WriteIO32(&pRegister->MEMTIMEACC[regindex8], regval);

	//if( NX_MCUS_SBUSID_STATIC13 >= Id	)	// For static bus #0 ~ #13
	{
//		pRegister->MEMTIMESACC[regindex8] = (pRegister->MEMTIMESACC[regindex8] & ~(0xFFUL<<bitpos8)) | (tSACC<<bitpos8);
		regval = ReadIO32(&pRegister->MEMTIMESACC[regindex8]);
		regval &= ~(0xFFUL<<bitpos8);
		regval |= (tSACC<<bitpos8);
		WriteIO32(&pRegister->MEMTIMESACC[regindex8], regval);

		// Bus Width
		if( BitWidth == 16 )	ReadIO32(&pRegister->MEMBW) |=	(1UL<<((U32)Id));	// 1: 16-bit
		else					ReadIO32(&pRegister->MEMBW) &= ~(1UL<<((U32)Id));	// 0:	8-bit

		// Burst Mode
		regval = ReadIO32(&pRegister->MEMBURST);
		regval &= ~(0xFUL<<bitpos4);
		regval |= ((U32)BurstRead )<<(bitpos4+0);
		regval |= ((U32)BurstWrite)<<(bitpos4+2);
//		pRegister->MEMBURST = regval;
		WriteIO32(&pRegister->MEMBURST, regval);

		// Wait control
//		pRegister->MEMWAIT = (pRegister->MEMWAIT & ~(0x3UL<<bitpos2)) | (((U32)WaitMode)<<bitpos2);
		regval = ReadIO32(&pRegister->MEMWAIT);
		regval &= ~(0x3UL<<bitpos2);
		regval |=(((U32)WaitMode)<<bitpos2);
		WriteIO32(&pRegister->MEMWAIT, regval);
	}
	//else if( NX_MCUS_SBUSID_IDE == Id )	// IDE
	//{
//	//	pRegister->MEMTIMEWACC[regindex8] = (pRegister->MEMTIMEWACC[regindex8] & ~(0xFFUL<<bitpos8)) | (tSACC<<bitpos8);
	//	regval = ReadIO32(&pRegister->MEMTIMEWACC[regindex8]);
	//	regval &= ~(0xFFUL<<bitpos8);
	//	regval |= (tSACC<<bitpos8);
	//	WriteIO32(&pRegister->MEMTIMEWACC[regindex8], regval);
    //
	//	// Wait control
//	//	pRegister->MEMWAIT = (pRegister->MEMWAIT & ~(0x3UL<<bitpos2)) | (((U32)WaitMode)<<bitpos2);
	//	regval = ReadIO32(&pRegister->MEMWAIT);
	//	regval &= ~(0x3UL<<bitpos2);
	//	regval |= (((U32)WaitMode)<<bitpos2);
	//	WriteIO32(&pRegister->MEMWAIT, regval);
	//}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get static BUS configration.
 *	@param[in]	Id			Static BUS ID which is one of @ref NX_MCUS_SBUSID enum.
 *	@param[out] pBitWidth	BUS bit width. This argument is only valid for Static #0 ~ #7.
 *	@param[out] ptACS		address setup time before nCS
 *	@param[out] ptCAH		address hold time after nCS
 *	@param[out] ptCOS		chip select setup time before nOE
 *	@param[out] ptCOH		chip select hold time after nOE
 *	@param[out] ptACC		data access time
 *	@param[out] ptSACC		data access time in burst access for Static #0 ~ #7.wait access time for IDE.
 *	@param[out] pWaitMode	wait control.  This argument is only valid for Static #0 ~ #7 and IDE.
 *	@param[out] pBurstRead	Burst mode control in reading. This argument is only valid for Static #0 ~ #7.
 *	@param[out] pBurstWrite Burst mode control in writing. This argument is only valid for Static #0 ~ #7.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 */
void	NX_MCUS_GetStaticBUSConfig
(
	NX_MCUS_SBUSID Id, U32* pBitWidth,
	U32* ptACS, U32* ptCAH, U32* ptCOS, U32* ptCOH, U32* ptACC, U32* ptSACC,
	NX_MCUS_WAITMODE* pWaitMode, NX_MCUS_BURSTMODE* pBurstRead,NX_MCUS_BURSTMODE* pBurstWrite
)
{
	register U32 bitpos2, bitpos4, bitpos8, regindex4, regindex8;
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( (NX_MCUS_SBUSID_STATIC13>=Id) || (NX_MCUS_SBUSID_IDE==Id) || (NX_MCUS_SBUSID_NAND==Id) );
	NX_ASSERT( CNULL != __g_pRegister );

	//--------------------------------------------------------------------------
	pRegister	=	__g_pRegister;

	bitpos2		= ((U32)Id) * 2;

	bitpos4		= ((U32)Id) * 4;
	regindex4	= bitpos4 / 32;
	bitpos4		= bitpos4 % 32;

	bitpos8		= ((U32)Id) * 8;
	regindex8	= bitpos8 / 32;
	bitpos8		= bitpos8 % 32;

	if( NX_MCUS_SBUSID_IDE == Id )
	{
		//--------------------------------------------------------------------------
		// Get timing for tACS, tCOS, tOCH.
		// 0 : 1 cycle, 1 : 2 cycle, 2 : 3 cycle, ......, 15 : 16 cycle
		if( CNULL != ptACS )	*ptACS = ((ReadIO32(&pRegister->MEMTIMEACS[regindex4]) >> bitpos4) & 0x0FUL) + 1;
		if( CNULL != ptCOS )	*ptCOS = ((ReadIO32(&pRegister->MEMTIMECOS[regindex4]) >> bitpos4) & 0x0FUL) + 1;
		if( CNULL != ptCOH )	*ptCOH = ((ReadIO32(&pRegister->MEMTIMECOH[regindex4]) >> bitpos4) & 0x0FUL) + 1;

		//--------------------------------------------------------------------------
		// Get timing for tCAH.
		// 0 : 2 cycle, 1 : 3 cycle, 2 : 4 cycle, ......, 15 : 1 cycle
		if( CNULL != ptCAH )
		{
			*ptCAH = (ReadIO32(&pRegister->MEMTIMECAH[regindex4]) >> bitpos4) & 0x0FUL;
			*ptCAH = ((*ptCAH + 1) % 16) + 1;
		}

		//--------------------------------------------------------------------------
		// Get timing for tACC, tSACC
		// tACC	= n+1 cycle
		if( CNULL != ptACC )	*ptACC = ((ReadIO32(&pRegister->MEMTIMEACC[regindex8]) >> bitpos8) & 0xFFUL) + 1;

		// tSACC = n+1 cycle
		if( CNULL != ptSACC )	*ptSACC = ((ReadIO32(&pRegister->MEMTIMEWACC[regindex8]) >> bitpos8) & 0xFFUL) + 1;

		// Wait control
		if( CNULL != pWaitMode )	{ *pWaitMode	= (NX_MCUS_WAITMODE)((ReadIO32(&pRegister->MEMWAIT) >> bitpos2) & 0x3UL);		}
	}
	else
	{
		//--------------------------------------------------------------------------
		// Get timing for tACS, tCAH, tCOS, tOCH.
		// 0 : 1 cycle, 1 : 2 cycle, 2 : 3 cycle, ......, 15 : 0 cycle
		if( CNULL != ptACS )
		{
			*ptACS = (ReadIO32(&pRegister->MEMTIMEACS[regindex4]) >> bitpos4) & 0x0FUL;
			*ptACS = (*ptACS + 1) % 16;
		}

		if( CNULL != ptCAH )
		{
			*ptCAH = (ReadIO32(&pRegister->MEMTIMECAH[regindex4]) >> bitpos4) & 0x0FUL;
			*ptCAH = (*ptCAH + 1) % 16;
		}

		if( CNULL != ptCOS )
		{
			*ptCOS = (ReadIO32(&pRegister->MEMTIMECOS[regindex4]) >> bitpos4) & 0x0FUL;
			*ptCOS = (*ptCOS + 1) % 16;
		}

		if( CNULL != ptCOH )
		{
			*ptCOH = (ReadIO32(&pRegister->MEMTIMECOH[regindex4]) >> bitpos4) & 0x0FUL;
			*ptCOH = (*ptCOH + 1) % 16;
		}

		//--------------------------------------------------------------------------
		// Get timing for tACC
		// tACC	= n+1 cycle
		if( CNULL != ptACC )
		{
			*ptACC = (ReadIO32(&pRegister->MEMTIMEACC[regindex8]) >> bitpos8) & 0xFFUL;
			*ptACC += 1;
		}

		if( NX_MCUS_SBUSID_STATIC13 >= Id	)	// For static bus #0 ~ #13
		{
			// tSACC = n+1 cycle
			if( CNULL != ptSACC )
			{
				*ptSACC = (ReadIO32(&pRegister->MEMTIMESACC[regindex8]) >> bitpos8) & 0xFFUL;
				*ptSACC += 1;
			}

			if( CNULL != pBitWidth )	{ *pBitWidth		= (ReadIO32(&pRegister->MEMBW) & (1UL<<((U32)Id))) ? 16 : 8;						}
			if( CNULL != pBurstRead )	{ *pBurstRead		= (NX_MCUS_BURSTMODE)((ReadIO32(&pRegister->MEMBURST) >> (bitpos4+0)) & 0x3UL);	}
			if( CNULL != pBurstWrite )	{ *pBurstWrite	= (NX_MCUS_BURSTMODE)((ReadIO32(&pRegister->MEMBURST) >> (bitpos4+2)) & 0x3UL);	}

			// Wait control
			if( CNULL != pWaitMode )	{ *pWaitMode	= (NX_MCUS_WAITMODE)((ReadIO32(&pRegister->MEMWAIT) >> bitpos2) & 0x3UL);		}
		}
	}
}


void	NX_MCUS_SetNANDBUSConfig
(
	U32 NFBank,
	U32 tACS, U32 tCAH, U32 tCOS, U32 tOCH, U32 tACC
)
{
	register struct NX_MCUS_RegisterSet*	pRegister;
	register U32 regval;

	NX_ASSERT( (3>=NFBank) );
	NX_ASSERT( 1 <=	tACC &&	tACC <= 256 );
	NX_ASSERT( 16 > tACS );
	NX_ASSERT( 16 > tCAH );
	NX_ASSERT( 16 > tCOS );
	NX_ASSERT( 16 > tOCH );

	NX_ASSERT( CNULL != __g_pRegister );

	//--------------------------------------------------------------------------
	pRegister	=	__g_pRegister;
	//--------------------------------------------------------------------------
	// Set timing for tACS, tCAH, tCOS, tOCH.
	// 0 : 1 cycle, 1 : 2 cycle, 2 : 3 cycle, ......, 15 : 0 cycle
	tACS = (tACS + 15) & 0xF;
	tCAH = (tCAH + 15) & 0xF;
	tCOS = (tCOS + 15) & 0xF;
	tOCH = (tOCH + 15) & 0xF;

	tACC	-= 1;	// tACC	= n+1 cycle

	//--------------------------------------------------------------------------
	regval = ReadIO32(&pRegister->NFTACS);
	regval &= ~(0x0FUL<<(NFBank*4));
	regval |= (tACS<<(NFBank*4));
	WriteIO32(&pRegister->NFTACS, regval);

	regval = ReadIO32(&pRegister->NFTCAH);
	regval &= ~(0x0FUL<<(NFBank*4));
	regval |= (tCAH<<(NFBank*4));
	WriteIO32(&pRegister->NFTCAH, regval);

	regval = ReadIO32(&pRegister->NFTCOS);
	regval &= ~(0x0FUL<<(NFBank*4));
	regval |= (tCOS<<(NFBank*4));
	WriteIO32(&pRegister->NFTCOS, regval);

	regval = ReadIO32(&pRegister->NFTOCH);
	regval &= ~(0x0FUL<<(NFBank*4));
	regval |= (tOCH<<(NFBank*4));
	WriteIO32(&pRegister->NFTOCH, regval);

	regval = ReadIO32(&pRegister->NFTACC);
	regval &= ~(0xFFUL<<(NFBank*8));
	regval |= (tACC<<(NFBank*8));
	WriteIO32(&pRegister->NFTACC, regval);
}
void	NX_MCUS_GetNANDBUSConfig
(
	U32 NFBank,
	U32* ptACS, U32* ptCAH, U32* ptCOS, U32* ptOCH, U32* ptACC
)
{
	register struct NX_MCUS_RegisterSet*	pRegister;

	NX_ASSERT( (3>=NFBank) );
	NX_ASSERT( CNULL != __g_pRegister );

	//--------------------------------------------------------------------------
	pRegister	=	__g_pRegister;

	//--------------------------------------------------------------------------
	// Get timing for tACS, tCAH, tCOS, tOCH.
	// 0 : 1 cycle, 1 : 2 cycle, 2 : 3 cycle, ......, 15 : 0 cycle
	if( CNULL != ptACS )
	{
		*ptACS = (ReadIO32(&pRegister->NFTACS ) >> (NFBank*4)) & 0x0FUL;
		*ptACS = (*ptACS + 1) & 0xF;
	}

	if( CNULL != ptCAH )
	{
		*ptCAH = (ReadIO32(&pRegister->NFTCAH ) >> (NFBank*4)) & 0x0FUL;
		*ptCAH = (*ptCAH + 1) & 0xF;
	}

	if( CNULL != ptCOS )
	{
		*ptCOS = (ReadIO32(&pRegister->NFTCOS ) >> (NFBank*4)) & 0x0FUL;
		*ptCOS = (*ptCOS + 1) & 0xF;
	}

	if( CNULL != ptOCH )
	{
		*ptOCH = (ReadIO32(&pRegister->NFTOCH ) >> (NFBank*4)) & 0x0FUL;
		*ptOCH = (*ptOCH + 1) & 0xF;
	}

	//--------------------------------------------------------------------------
	// Get timing for tACC
	// tACC	= n+1 cycle
	if( CNULL != ptACC )
	{
		*ptACC = (ReadIO32(&pRegister->NFTACC ) >> (NFBank*8)) & 0xFFUL;
		*ptACC += 1;
	}
}

//------------------------------------------------------------------------------
//	NAND Flash Memory Controller Functions
//------------------------------------------------------------------------------
/**
 *	@brief		Set NAND Flash Address Type.
 *	@param[in]	type	NAND Flash address type which is one of @ref NX_MCUS_NFTYPE enum.
 *	@return		None.
 *	@remark		It can also be set NFTYPE by external pin configuration(CfgNFType)
 *				for NAND Booting.
 */
void	NX_MCUS_SetNFType ( NX_MCUS_NFTYPE type )
{
	const U32 BIT_SIZE	= 2;
	const U32 BIT_POS	= 3;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	const U32 IRQPEND	= (1UL<<15);

	register U32 regval;

	NX_ASSERT( 4 > type );
	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);
	regval &= ~(IRQPEND | BIT_MASK);		// Unmask bits include IRQPEND.
	regval |= (((U32)type) << BIT_POS);
	regval  = __NX_MCUS_NFCONTROL_RESET_BIT(regval);

	WriteIO32(&__g_pRegister->NFCONTROL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current setting for NAND Flash Address Type.
 *	@return		the current setting for NAND Flash Address Type.
 *	@remark		It can also be set NFTYPE by external pin configuration(CfgNFType)
 *				for NAND Booting.
 */
NX_MCUS_NFTYPE	NX_MCUS_GetNFType ( void )
{
	const U32 BIT_SIZE	= 2;
	const U32 BIT_POS	= 3;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (NX_MCUS_NFTYPE)((ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set whether the NAND Flash Boot to enable or disable.
 *	@param[in]	bEnb	Set this as CTRUE to enable the NAND Booting.
 *	@return		None.
 *	@remark		It can also be enabled by external pin configuration(CfgNFBoot).
 */
void	NX_MCUS_SetNFEXChannelEnable( CBOOL bEnb )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 5;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	const U32 IRQPEND	= (1UL<<15);

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);
	regval &= ~IRQPEND;					// Unmask IRQPEND.
	if( bEnb )	regval |=	BIT_MASK;
	else		regval &= ~BIT_MASK;

	regval = __NX_MCUS_NFCONTROL_RESET_BIT(regval);

	WriteIO32(&__g_pRegister->NFCONTROL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the NAND Flash Booting is enabled or not.
 *	@return		CTRUE indicates the NAND Flash Booting is enabled.
 *				CFALSE indicates the NAND Flash Booting is disabled.
 *	@remark		It can also be enabled by external pin configuration(CfgNFBoot).
 */
CBOOL	NX_MCUS_GetNFEXChannelEnable ( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 6;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the NAND Flash bank number to access.
 *	@param[in]	Bank	Specifies the NAND Flash bank number to access, 0 ~ 3.
 *	@return		None.
 */
void	NX_MCUS_SetNFBank( U32 Bank )
{
	const U32 BIT_SIZE	= 2;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	const U32 IRQPEND	= (1UL<<15);

	register U32 regval;

	NX_ASSERT( 4 > Bank );
	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);
	regval &= ~(IRQPEND | BIT_MASK);		// Unmask bits include IRQPEND.
	regval |= (Bank << BIT_POS);
	regval  = __NX_MCUS_NFCONTROL_RESET_BIT(regval);

	WriteIO32(&__g_pRegister->NFCONTROL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the current NAND Flash bank number.
 *	@return		The current NAND Flash bank number.
 */
U32		NX_MCUS_GetNFBank ( void )
{
	const U32 BIT_SIZE	= 2;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return ((ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the external nCS signal of NAND flash to assert or negate.
 *	@param[in]	bEnb	Set this as CTRUE to assert nCS signal for current bank.
 *						Set this as CFALSE to negate nCS signal for current bank.
 *	@return		None.
 *	@remark		You have to set @a bEnb as CTRUE to control the external NAND flash
 *				memory. It makes NAND flash memory controller holds the external
 *				chip select signal of selected bank is asserted. You can also
 *				set @a bEnb as CFALSE to negate the chip select signal of the NAND
 *				flash memory chip to reduce a power cunsumption if it doesn't need
 *				to access anymore.
 */
void	NX_MCUS_SetNFCSEnable( CBOOL bEnb )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 31;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	const U32 IRQPEND	= (1UL<<15);

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);
	regval &= ~IRQPEND;					// Unmask IRQPEND.
	if( bEnb )	regval |=	BIT_MASK;
	else		regval &= ~BIT_MASK;

	regval  = __NX_MCUS_NFCONTROL_RESET_BIT(regval);

	WriteIO32(&__g_pRegister->NFCONTROL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the external nCS signal of NAND flash is asserted or not.
 *	@return		CTRUE indicates the external nCS signal is asserted.
 *				CFALSE indicates the external nCS signal is negated.
 */
CBOOL	NX_MCUS_GetNFCSEnable ( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 31;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether NAND Flash is ready or not.
 *	@return		CTRUE indicates NAND Flash is ready.
 *				CFALSE indicates NAND Flash is busy.
 *	@remark		There's a external pin, RnB, indicates the status of NAND Flash
 *				operation. When low, it indicates that a program, erase or random
 *				read operation is in progress and returns to high state upon
 *				completion. This function returns the state of RnB pin.
 */
CBOOL	NX_MCUS_IsNFReady ( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 9;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Determines whether the auto reset of the H/W ECC block to enable or disable.
 *	@param[in]	bEnb	Set this as CTRUE to reset the H/W ECC block automaically.
 *						Set this as CFALSE to reset the H/W ECC block manually.
 *	@return		None.
 *	@remark		You can all this function with CTRUE argument to reset the H/W
 *				ECC block automatically by CLE or ALE.
 *				If you want to reset the H/W ECC block only by call
 *				NX_MCUS_ResetNFECCBlock(), you should call this function with
 *				CFALSE argument.
 */
void	NX_MCUS_SetAutoResetEnable( CBOOL bEnb )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 30;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	const U32 IRQPEND	= (1UL<<15);

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);
	regval &= ~IRQPEND;					// Unmask IRQPEND.
	if( bEnb )	regval |=	BIT_MASK;
	else		regval &= ~BIT_MASK;

	regval  = __NX_MCUS_NFCONTROL_RESET_BIT(regval);

	WriteIO32(&__g_pRegister->NFCONTROL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the auto reset of the H/W ECC block is enabled or not.
 *	@return		CTRUE indicates the auto reset is enabled.
 *				CFALSE indicates the auto reset is diabled.
 */
CBOOL	NX_MCUS_GetAutoResetEnable( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 30;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Reset the Hardware ECC block.
 *	@return		None.
 */
void	NX_MCUS_ResetNFECCBlock( void )
{
	const U32	ECCRST	= (1UL << 11);
	const U32	IRQPEND = (1UL << 15);

	register U32 regvalue;

	NX_ASSERT( CNULL != __g_pRegister );

	regvalue = ReadIO32(&__g_pRegister->NFCONTROL);
	regvalue &= ~IRQPEND;
	regvalue |= ECCRST;
	regvalue  = __NX_MCUS_NFCONTROL_RESET_BIT(regvalue);

	WriteIO32(&__g_pRegister->NFCONTROL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the H/W ECC mode.
 *	@param[in]	bits	Specifies the number of ECC mode in bits - 4, 8, 16 or 24.
 *	@return		None.
 */
void	NX_MCUS_SetECCMode( NX_MCUS_ECCMODE EccMode )
{
	const U32 BIT_SIZE	= 3;
	const U32 BIT_POS	= 27;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	const U32 IRQPEND	= (1UL<<15);

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFCONTROL);
	regval &= ~(IRQPEND | BIT_MASK);		// Unmask bits include IRQPEND.
	regval |= (EccMode << BIT_POS);
	regval  = __NX_MCUS_NFCONTROL_RESET_BIT(regval);

	WriteIO32(&__g_pRegister->NFCONTROL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the H/W ECC mode.
 *	@return		the number of ECC mode in bits.
 */
U32		NX_MCUS_GetECCMode( void )
{
	const U32 BIT_SIZE	= 3;
	const U32 BIT_POS	= 27;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	// NFECCMODE[1:0] = 0(4), 1(8), 2(16)
	return (4UL << ((ReadIO32(&__g_pRegister->NFCONTROL) & BIT_MASK) >> BIT_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get NAND flash write data size in bytes.
 *	@return		NAND flash write data size in bytes.
 */
U32		NX_MCUS_GetNFWriteDataCount( void )
{
	const U32 BIT_SIZE	= 10;
	const U32 BIT_POS	= 16;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return ((ReadIO32(&__g_pRegister->NFCNT) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get NAND flash read data size in bytes.
 *	@return		NAND flash read data size in bytes.
 */
U32		NX_MCUS_GetNFReadDataCount( void )
{
	const U32 BIT_SIZE	= 10;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return ((ReadIO32(&__g_pRegister->NFCNT) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether there's an error on read data or not.
 *	@return		CTRUE indicates there's an error on read data.
 *				CFALSE indicates there's no error on read data.
 *	@remark		After you read 512 bytes on NAND flash memory, you can check the
 *				result of H/W ECC decoding block whether there's an error on read data.
 */
CBOOL	NX_MCUS_IsNFError( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 2;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->NFECCSTATUS) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether H/W ECC decoding has been finished or not.
 *	@return		CTRUE indicates H/W ECC decoding has been finished.
 *				CFALSE indicates H/W ECC decoding is busy.
 *	@remark		H/W ECC decoding has been finished after 51 BCLKs from reading
 *				512 bytes on NAND flash memory.
 */
CBOOL	NX_MCUS_IsNFReadEnd( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 1;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->NFECCSTATUS) & BIT_MASK) >> BIT_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether H/W ECC encoding has been finished or not.
 *	@return		CTRUE indicates H/W ECC encoding has been finished.
 *				CFALSE indicates H/W ECC encoding is busy.
 *	@remark		H/W ECC encoding has been finished after writing 512 bytes on
 *				NAND flash memory.
 */
CBOOL	NX_MCUS_IsNFWriteEnd( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->NFECCSTATUS) & BIT_MASK) >> BIT_POS);
}

CBOOL	NX_MCUS_IsDecodeError( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 3;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->NFECCSTATUS) & BIT_MASK) >> BIT_POS);
}
CBOOL	NX_MCUS_IsNFECCBusy(void)
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 2;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (CBOOL)((ReadIO32(&__g_pRegister->NFECCSTATUS) & BIT_MASK) >> BIT_POS);
}

U32		NX_MCUS_GetNumOfError( void )
{
	const U32 BIT_SIZE	= 7;
	const U32 BIT_POS	= 4;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (U32)((ReadIO32(&__g_pRegister->NFECCSTATUS) & BIT_MASK) >> BIT_POS);
}

void	NX_MCUS_LoadELP( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 27;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	U32 DecMode;

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL);
	DecMode = (regval&0x10000000)>>2;
	regval = (regval&0xaffffff)|DecMode;
	regval |= BIT_MASK;
	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
}

void NX_MCUS_SetNFDecMode( NX_MCUS_DECMODE dMode )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 26;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	U32 DecMode;

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL);
	DecMode = (regval&0x10000000)>>2;
	regval = (regval&0xaffffff)|DecMode;
	regval &= ~BIT_MASK;
	regval |= (U32)dMode<<BIT_POS;
	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
};

NX_MCUS_DECMODE NX_MCUS_GetNFDecMode( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 28;//26;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (NX_MCUS_DECMODE)((ReadIO32(&__g_pRegister->NFECCCTRL) & BIT_MASK) >> BIT_POS);
}

void	NX_MCUS_RunECCEncDec( void )
{
	const U32 BIT_SIZE	= 1;
	const U32 BIT_POS	= 28;//const U32 BIT_POS	= 21;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;
	U32 DecMode;

	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL);
	DecMode = (regval&0x10000000)>>2;
	regval = (regval&0xaffffff)|DecMode;
	regval |= BIT_MASK;
	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ECC parity code generated by H/W ECC encoding block for 4-bit ECC Mode.
 *	@param[out] pECC	a pointer to store ECC parity code which is 52 bits.
 *	@return		None.
 */
void	NX_MCUS_GetNFECC4( U32* pECC )
{
	const U32 BIT_MASK	= 0x00F0FFFF;

	register volatile U32 *pReg;

	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL != __g_pRegister );

	pReg = &__g_pRegister->NFECC[0];

	pECC[0] = ReadIO32(&pReg[0]);
	pECC[1] = ReadIO32(&pReg[1]) & BIT_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ECC parity code generated by H/W ECC encoding block for 8-bit ECC Mode.
 *	@param[out] pECC	a pointer to store ECC parity code which is 104 bits.
 *	@return		None.
 */
void	NX_MCUS_GetNFECC8( U32* pECC )
{
	const U32 BIT_SIZE	= 8;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	register volatile U32 *pReg;

	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL != __g_pRegister );

	pReg = &__g_pRegister->NFECC[0];

	pECC[0] = ReadIO32(&pReg[0]);
	pECC[1] = ReadIO32(&pReg[1]);
	pECC[2] = ReadIO32(&pReg[2]);
	pECC[3] = ReadIO32(&pReg[3]) & BIT_MASK;
}

void	NX_MCUS_GetNFECC12( U32* pECC )
{
	const U32 BIT_MASK	= 0xF0FFFFFF;
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL != __g_pRegister );

	pReg = &__g_pRegister->NFECC[0];

	pECC[0] = ReadIO32(&pReg[0]);
	pECC[1] = ReadIO32(&pReg[1]);
	pECC[2] = ReadIO32(&pReg[2]);
	pECC[3] = ReadIO32(&pReg[3]);
	pECC[4] = ReadIO32(&pReg[4]) & BIT_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ECC parity code generated by H/W ECC encoding block for 16-bit ECC Mode.
 *	@param[out] pECC	a pointer to store ECC parity code which is 208 bits.
 *	@return		None.
 */
void	NX_MCUS_GetNFECC16( U32* pECC )
{
	const U32 BIT_SIZE	= 16;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	register volatile U32 *pReg;

	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL != __g_pRegister );

	pReg = &__g_pRegister->NFECC[0];

	pECC[0] = ReadIO32(&pReg[0]);
	pECC[1] = ReadIO32(&pReg[1]);
	pECC[2] = ReadIO32(&pReg[2]);
	pECC[3] = ReadIO32(&pReg[3]);
	pECC[4] = ReadIO32(&pReg[4]);
	pECC[5] = ReadIO32(&pReg[5]);
	pECC[6] = ReadIO32(&pReg[6]) & BIT_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get ECC parity code generated by H/W ECC encoding block for 24-bit ECC Mode.
 *	@param[out] pECC	a pointer to store ECC parity code which is 336 bits.
 *	@return		None.
 */
void	NX_MCUS_GetNFECC24( U32* pECC )
{
	const U32 BIT_SIZE	= 16;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1<<BIT_SIZE)-1) << BIT_POS;

	//register volatile U32 *pReg, *pRegH;
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL != __g_pRegister );

	pReg = &__g_pRegister->NFECC[0];
	//pRegH = &__g_pRegister->NFECCH[0];

	pECC[0] = ReadIO32(&pReg[0]);
	pECC[1] = ReadIO32(&pReg[1]);
	pECC[2] = ReadIO32(&pReg[2]);
	pECC[3] = ReadIO32(&pReg[3]);
	pECC[4] = ReadIO32(&pReg[4]);
	pECC[5] = ReadIO32(&pReg[5]);
	pECC[6] = ReadIO32(&pReg[6]);
	pECC[7] = ReadIO32(&pReg[7]);
	pECC[8] = ReadIO32(&pReg[8]);
	pECC[9] = ReadIO32(&pReg[9]);
	pECC[10]= ReadIO32(&pReg[10]) & BIT_MASK;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set ECC parity code for H/W ECC decoding block when 4-bit ECC mode.
 *	@param[in]	pECC	a pointer to load ECC parity code which is 52 bits.
 *	@return		None.
 *	@remark		ECC parity code which is set by this function is used for generating
 *				syndrome data on H/W ECC decoding block when you read 512 bytes on
 *				NAND flash memory.
 */
void	NX_MCUS_SetNFOriginECC4( U32 *pECC )
{
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL == (0xFF0F0000 & pECC[1]) );

	pReg = &__g_pRegister->NFORGECC[0];

	WriteIO32(&pReg[0], pECC[0]);
	WriteIO32(&pReg[1], pECC[1]);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set ECC parity code for H/W ECC decoding block when 8-bit ECC mode.
 *	@param[in]	pECC	a pointer to load ECC parity code which is 104 bits.
 *	@return		None.
 *	@remark		ECC parity code which is set by this function is used for generating
 *				syndrome data on H/W ECC decoding block when you read 512 bytes on
 *				NAND flash memory.
 */
void	NX_MCUS_SetNFOriginECC8( U32 *pECC )
{
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( (1UL<<8) > pECC[3] );

	pReg = &__g_pRegister->NFORGECC[0];

	WriteIO32(&pReg[0], pECC[0]);
	WriteIO32(&pReg[1], pECC[1]);
	WriteIO32(&pReg[2], pECC[2]);
	WriteIO32(&pReg[3], pECC[3]);
}

void	NX_MCUS_SetNFOriginECC12( U32 *pECC )
{
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( CNULL == (0x0F000000 & pECC[4]) );

	pReg = &__g_pRegister->NFORGECC[0];

	WriteIO32(&pReg[0], pECC[0]);
	WriteIO32(&pReg[1], pECC[1]);
	WriteIO32(&pReg[2], pECC[2]);
	WriteIO32(&pReg[3], pECC[3]);
	WriteIO32(&pReg[4], pECC[4]);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set ECC parity code for H/W ECC decoding block when 16-bit ECC mode.
 *	@param[in]	pECC	a pointer to load ECC parity code which is 208 bits.
 *	@return		None.
 *	@remark		ECC parity code which is set by this function is used for generating
 *				syndrome data on H/W ECC decoding block when you read 512 bytes on
 *				NAND flash memory.
 */
void	NX_MCUS_SetNFOriginECC16( U32 *pECC )
{
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( (1UL<<16) > pECC[6] );

	pReg = &__g_pRegister->NFORGECC[0];

	WriteIO32(&pReg[0], pECC[0]);
	WriteIO32(&pReg[1], pECC[1]);
	WriteIO32(&pReg[2], pECC[2]);
	WriteIO32(&pReg[3], pECC[3]);
	WriteIO32(&pReg[4], pECC[4]);
	WriteIO32(&pReg[5], pECC[5]);
	WriteIO32(&pReg[6], pECC[6]);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set ECC parity code for H/W ECC decoding block when 24-bit ECC mode.
 *	@param[in]	pECC	a pointer to load ECC parity code which is 336 bits.
 *	@return		None.
 *	@remark		ECC parity code which is set by this function is used for generating
 *				syndrome data on H/W ECC decoding block when you read 1024 bytes on
 *				NAND flash memory.
 */
void	NX_MCUS_SetNFOriginECC24( U32 *pECC )
{
	//register volatile U32 *pReg, *pRegH;
	register volatile U32 *pReg;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	NX_ASSERT( (1UL<<16) > pECC[10] );

	pReg = &__g_pRegister->NFORGECC[0];
	//pRegH = &__g_pRegister->NFORGECCH[0];

	WriteIO32(&pReg[0], pECC[0]);
	WriteIO32(&pReg[1], pECC[1]);
	WriteIO32(&pReg[2], pECC[2]);
	WriteIO32(&pReg[3], pECC[3]);
	WriteIO32(&pReg[4], pECC[4]);
	WriteIO32(&pReg[5], pECC[5]);
	WriteIO32(&pReg[6], pECC[6]);
	WriteIO32(&pReg[7], pECC[7]);
	WriteIO32(&pReg[8], pECC[8]);
	WriteIO32(&pReg[9], pECC[9]);
	WriteIO32(&pReg[10], pECC[10]);
}

void	NX_MCUS_SetNFOriginECC40( U32 *pECC )
{
	//register volatile U32 *pReg, *pRegH;
	register volatile U32 *pReg;
	U32 i;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	//NX_ASSERT( (1UL<<16) > pECC[10] );

	pReg = &__g_pRegister->NFORGECC[0];
	//pRegH = &__g_pRegister->NFORGECCH[0];
	for (i=0; i<18; i++)
	{
		WriteIO32(&pReg[i], pECC[i]);
	}
}

void	NX_MCUS_SetNFOriginECC60( U32 *pECC )
{
	register volatile U32 *pReg;
	U32 i;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pECC );
	//NX_ASSERT( (1UL<<16) > pECC[10] );

	pReg = &__g_pRegister->NFORGECC[0];
	//pRegH = &__g_pRegister->NFORGECCH[0];
	for (i=0; i<27; i++)
	{
		WriteIO32(&pReg[i], pECC[i]);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get odd syndromes from H/W ECC decoding for 4-bit ECC mode.
 *	@param[out] pSyndrome	a pointer to store 4 odd syndromes.
 *	@return		None.
 */
void	NX_MCUS_GetNFECCOddSyndrome4( U32* pSyndrome )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 16;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);

	register volatile U32 *pReg;
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pSyndrome );

	pReg = &__g_pRegister->NFSYNDROME[0];

	regval = ReadIO32(&pReg[0]);
	pSyndrome[0] = (regval				& BIT_MASK);	// Syndrome	1 - NFSYNDROME[0][13: 0]
	pSyndrome[1] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	3 - NFSYNDROME[0][29:16]

	regval = ReadIO32(&pReg[1]);
	pSyndrome[2] = (regval				& BIT_MASK);	// Syndrome	5 - NFSYNDROME[1][13: 0]
	pSyndrome[3] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	7 - NFSYNDROME[1][29:16]
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get odd syndromes from H/W ECC decoding for 8-bit ECC mode.
 *	@param[out] pSyndrome	a pointer to store 8 odd syndromes.
 *	@return		None.
 */
void	NX_MCUS_GetNFECCOddSyndrome8( U32* pSyndrome )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 16;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);

	register volatile U32 *pReg;
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pSyndrome );

	pReg = &__g_pRegister->NFSYNDROME[0];

	regval = ReadIO32(&pReg[0]);
	pSyndrome[0] = (regval				& BIT_MASK);	// Syndrome	1 - NFSYNDROME[0][13: 0]
	pSyndrome[1] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	3 - NFSYNDROME[0][29:16]

	regval = ReadIO32(&pReg[1]);
	pSyndrome[2] = (regval				& BIT_MASK);	// Syndrome	5 - NFSYNDROME[1][13: 0]
	pSyndrome[3] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	7 - NFSYNDROME[1][29:16]

	regval = ReadIO32(&pReg[2]);
	pSyndrome[4] = (regval				& BIT_MASK);	// Syndrome	9 - NFSYNDROME[2][13: 0]
	pSyndrome[5] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 11 - NFSYNDROME[2][29:16]

	regval = ReadIO32(&pReg[3]);
	pSyndrome[6] = (regval				& BIT_MASK);	// Syndrome 13 - NFSYNDROME[3][13: 0]
	pSyndrome[7] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 15 - NFSYNDROME[3][29:16]
}
//------------------------------------------------------------------------------
/**
 *	@brief		Get odd syndromes from H/W ECC decoding for 12-bit ECC mode.
 *	@param[out] pSyndrome	a pointer to store 12 odd syndromes.
 *	@return		None.
 */
void	NX_MCUS_GetNFECCOddSyndrome12( U32* pSyndrome )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 16;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);

	register volatile U32 *pReg;
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pSyndrome );

	pReg = &__g_pRegister->NFSYNDROME[0];

	regval = ReadIO32(&pReg[0]);
	pSyndrome[0] = (regval				& BIT_MASK);	// Syndrome	1 - NFSYNDROME[0][13: 0]
	pSyndrome[1] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	3 - NFSYNDROME[0][29:16]

	regval = ReadIO32(&pReg[1]);
	pSyndrome[2] = (regval				& BIT_MASK);	// Syndrome	5 - NFSYNDROME[1][13: 0]
	pSyndrome[3] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	7 - NFSYNDROME[1][29:16]

	regval = ReadIO32(&pReg[2]);
	pSyndrome[4] = (regval				& BIT_MASK);	// Syndrome	9 - NFSYNDROME[2][13: 0]
	pSyndrome[5] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 11 - NFSYNDROME[2][29:16]

	regval = ReadIO32(&pReg[3]);
	pSyndrome[6] = (regval				& BIT_MASK);	// Syndrome 13 - NFSYNDROME[3][13: 0]
	pSyndrome[7] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 15 - NFSYNDROME[3][29:16]

	regval = ReadIO32(&pReg[4]);
	pSyndrome[8] = (regval				& BIT_MASK);	// Syndrome 17 - NFSYNDROME[4][13: 0]
	pSyndrome[9] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 19 - NFSYNDROME[4][29:16]

	regval = ReadIO32(&pReg[5]);
	pSyndrome[10] = (regval				& BIT_MASK);	// Syndrome 21 - NFSYNDROME[5][13: 0]
	pSyndrome[11] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 23 - NFSYNDROME[5][29:16]
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get odd syndromes from H/W ECC decoding for 16-bit ECC mode.
 *	@param[out] pSyndrome	a pointer to store 16 odd syndromes.
 *	@return		None.
 */
void	NX_MCUS_GetNFECCOddSyndrome16( U32* pSyndrome )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 16;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);

	register volatile U32 *pReg;
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pSyndrome );

	pReg = &__g_pRegister->NFSYNDROME[0];

	regval = ReadIO32(&pReg[0]);
	pSyndrome[0] = (regval				& BIT_MASK);	// Syndrome	1 - NFSYNDROME[0][13: 0]
	pSyndrome[1] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	3 - NFSYNDROME[0][29:16]

	regval = ReadIO32(&pReg[1]);
	pSyndrome[2] = (regval				& BIT_MASK);	// Syndrome	5 - NFSYNDROME[1][13: 0]
	pSyndrome[3] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	7 - NFSYNDROME[1][29:16]

	regval = ReadIO32(&pReg[2]);
	pSyndrome[4] = (regval				& BIT_MASK);	// Syndrome	9 - NFSYNDROME[2][13: 0]
	pSyndrome[5] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 11 - NFSYNDROME[2][29:16]

	regval = ReadIO32(&pReg[3]);
	pSyndrome[6] = (regval				& BIT_MASK);	// Syndrome 13 - NFSYNDROME[3][13: 0]
	pSyndrome[7] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 15 - NFSYNDROME[3][29:16]

	regval = ReadIO32(&pReg[4]);
	pSyndrome[8] = (regval				& BIT_MASK);	// Syndrome 17 - NFSYNDROME[4][13: 0]
	pSyndrome[9] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 19 - NFSYNDROME[4][29:16]

	regval = ReadIO32(&pReg[5]);
	pSyndrome[10] = (regval				& BIT_MASK);	// Syndrome 21 - NFSYNDROME[5][13: 0]
	pSyndrome[11] = (regval >> BIT_POS) & BIT_MASK;		// Syndrome 23 - NFSYNDROME[5][29:16]

	regval = ReadIO32(&pReg[6]);
	pSyndrome[12] = (regval				& BIT_MASK);	// Syndrome 25 - NFSYNDROME[6][13: 0]
	pSyndrome[13] = (regval >> BIT_POS) & BIT_MASK;		// Syndrome 27 - NFSYNDROME[6][29:16]

	regval = ReadIO32(&pReg[7]);
	pSyndrome[14] = (regval				& BIT_MASK);	// Syndrome 29 - NFSYNDROME[7][13: 0]
	pSyndrome[15] = (regval >> BIT_POS) & BIT_MASK;		// Syndrome 31 - NFSYNDROME[7][29:16]
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get odd syndromes from H/W ECC decoding for 24-bit ECC mode.
 *	@param[out] pSyndrome	a pointer to store 24 odd syndromes.
 *	@return		None.
 */
void	NX_MCUS_GetNFECCOddSyndrome24( U32* pSyndrome )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 16;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);

	register volatile U32 *pReg;
	//register volatile U32 *pReg, *pRegH;
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pSyndrome );

	pReg = &__g_pRegister->NFSYNDROME[0];
	//pRegH = &__g_pRegister->NFSYNDROMEH[0];

	regval = ReadIO32(&pReg[0]);
	pSyndrome[0] = (regval				& BIT_MASK);	// Syndrome	1 - NFSYNDROME[0][13: 0]
	pSyndrome[1] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	3 - NFSYNDROME[0][29:16]

	regval = ReadIO32(&pReg[1]);
	pSyndrome[2] = (regval				& BIT_MASK);	// Syndrome	5 - NFSYNDROME[1][13: 0]
	pSyndrome[3] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome	7 - NFSYNDROME[1][29:16]

	regval = ReadIO32(&pReg[2]);
	pSyndrome[4] = (regval				& BIT_MASK);	// Syndrome	9 - NFSYNDROME[2][13: 0]
	pSyndrome[5] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 11 - NFSYNDROME[2][29:16]

	regval = ReadIO32(&pReg[3]);
	pSyndrome[6] = (regval				& BIT_MASK);	// Syndrome 13 - NFSYNDROME[3][13: 0]
	pSyndrome[7] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 15 - NFSYNDROME[3][29:16]

	regval = ReadIO32(&pReg[4]);
	pSyndrome[8] = (regval				& BIT_MASK);	// Syndrome 17 - NFSYNDROME[4][13: 0]
	pSyndrome[9] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 19 - NFSYNDROME[4][29:16]

	regval = ReadIO32(&pReg[5]);
	pSyndrome[10] = (regval				& BIT_MASK);	// Syndrome 21 - NFSYNDROME[5][13: 0]
	pSyndrome[11] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 23 - NFSYNDROME[5][29:16]

	regval = ReadIO32(&pReg[6]);
	pSyndrome[12] = (regval				& BIT_MASK);	// Syndrome 25 - NFSYNDROME[6][13: 0]
	pSyndrome[13] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 27 - NFSYNDROME[6][29:16]

	regval = ReadIO32(&pReg[7]);
	pSyndrome[14] = (regval				& BIT_MASK);	// Syndrome 29 - NFSYNDROME[7][13: 0]
	pSyndrome[15] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 31 - NFSYNDROME[7][29:16]

	regval = ReadIO32(&pReg[8]);
	pSyndrome[16] = (regval				& BIT_MASK);	// Syndrome 33 - NFSYNDROMEH[0][13: 0]
	pSyndrome[17] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 35 - NFSYNDROMEH[4][29:16]

	regval = ReadIO32(&pReg[9]);
	pSyndrome[18] = (regval				& BIT_MASK);	// Syndrome 37 - NFSYNDROMEH[5][13: 0]
	pSyndrome[19] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 39 - NFSYNDROMEH[5][29:16]

	regval = ReadIO32(&pReg[10]);
	pSyndrome[20] = (regval				& BIT_MASK);	// Syndrome 41 - NFSYNDROMEH[6][13: 0]
	pSyndrome[21] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 43 - NFSYNDROMEH[6][29:16]

	regval = ReadIO32(&pReg[11]);
	pSyndrome[22] = (regval				& BIT_MASK);	// Syndrome 45 - NFSYNDROMEH[7][13: 0]
	pSyndrome[23] = (regval >> BIT_POS)	& BIT_MASK;		// Syndrome 47 - NFSYNDROMEH[7][29:16]
}

void NX_MCUS_SetNumOfELP(U32 ELPNum)
{
	const U32 BIT_SIZE	= 7;
	const U32 BIT_POS	= 18;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1)<<BIT_POS;
	register U32 regval;
	U32 DecMode;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( 4 == ELPNum || 8 == ELPNum || 12 == ELPNum || 16 == ELPNum 
        || 24 == ELPNum || 40 == ELPNum || 60 == ELPNum );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL);
	DecMode = (regval&0x10000000)>>2;
	regval = (regval&0xaffffff)|DecMode;
	regval &= ~BIT_MASK;
	regval |= (ELPNum << BIT_POS) & BIT_MASK;
	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
}

U32 NX_MCUS_GetNumOfELP( void )
{
	const U32 BIT_SIZE	= 7;
	const U32 BIT_POS	= 18;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1)<<BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return ( ReadIO32(&__g_pRegister->NFECCCTRL) & BIT_MASK ) >> BIT_POS;
}

void	NX_MCUS_SetParityCount( U32 ParityByte )
{
	const U32 BIT_SIZE	= 8;
	const U32 BIT_POS	= 10;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1)<<BIT_POS;
	register U32 regval;
	U32 DecMode;

	NX_ASSERT( CNULL != __g_pRegister );
	//NX_ASSERT( 6 == ParityByte || 12 == ParityByte || 19 == ParityByte || 25 == ParityByte || 41 == ParityByte );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL);
	DecMode = (regval&0x10000000)>>2;
	regval = (regval&0xaffffff)|DecMode;
	regval &= ~BIT_MASK;
	regval |= (ParityByte << BIT_POS) & BIT_MASK;
	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
}

U32		NX_MCUS_GetParityCount( void )
{
	const U32 BIT_SIZE	= 8;
	const U32 BIT_POS	= 10;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1)<<BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return (ReadIO32(&__g_pRegister->NFECCCTRL) & BIT_MASK) >> BIT_POS;
}

void	NX_MCUS_SetNANDRWDataNum( U32 DataNum )
{
	const U32 BIT_SIZE	= 10;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1)<<BIT_POS;
	register U32 regval;
	U32 DecMode;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( 512 == DataNum || 1024 == DataNum );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL);
	DecMode = (regval&0x10000000)>>2;
	regval = (regval&0xaffffff)|DecMode;
	regval &= ~BIT_MASK;
	regval |= ((DataNum-1) << BIT_POS) & BIT_MASK;
	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
}

U32		NX_MCUS_GetNANDRWDataNum( void )
{
	const U32 BIT_SIZE	= 10;
	const U32 BIT_POS	= 0;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1)<<BIT_POS;

	NX_ASSERT( CNULL != __g_pRegister );

	return ((ReadIO32(&__g_pRegister->NFECCCTRL) & BIT_MASK) >> BIT_POS)+1;
}

void	NX_MCUS_SetNANDECCAutoMode(CBOOL UseCPUELP, CBOOL UseCPUSYNDROM)
{
//	const U32 ELPBIT_SIZE	= 1;
	const U32 ELPBIT_POS	= 0;
//	const U32 SYNBIT_SIZE	= 1;
	const U32 SYNBIT_POS	= 1;
	//const U32 BIT_MASK	= (((1UL<<ELPBIT_SIZE)-1)<<ELPBIT_POS)|(((1UL<<SYNBIT_SIZE)-1)<<SYNBIT_POS);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );

	regval = ((UseCPUELP << ELPBIT_POS)|(UseCPUSYNDROM<<SYNBIT_POS));

	WriteIO32(&__g_pRegister->NFECCAUTOMODE, regval);
}

void	NX_MCUS_SetELP4( U16 *pELP )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELP );

	regval =  (U32)(pELP[0] & BIT_MASK)
			| (U32)(pELP[1] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[0], regval);
	regval =  (U32)(pELP[2] & BIT_MASK)
			| (U32)(pELP[3] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[1], regval);
}
void	NX_MCUS_SetELP8( U16 *pELP )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELP );

	regval =  (U32)(pELP[0] & BIT_MASK)
			| (U32)(pELP[1] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[0], regval);
	regval =  (U32)(pELP[2] & BIT_MASK)
			| (U32)(pELP[3] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[1], regval);
	regval =  (U32)(pELP[4] & BIT_MASK)
			| (U32)(pELP[5] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[2], regval);
	regval =  (U32)(pELP[6] & BIT_MASK)
			| (U32)(pELP[7] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[3], regval);
}
void	NX_MCUS_SetELP12( U16 *pELP )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELP );

	regval =  (U32)(pELP[0] & BIT_MASK)
			| (U32)(pELP[1] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[0], regval);
	regval =  (U32)(pELP[2] & BIT_MASK)
			| (U32)(pELP[3] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[1], regval);
	regval =  (U32)(pELP[4] & BIT_MASK)
			| (U32)(pELP[5] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[2], regval);
	regval =  (U32)(pELP[6] & BIT_MASK)
			| (U32)(pELP[7] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[3], regval);
	regval =  (U32)(pELP[8] & BIT_MASK)
			| (U32)(pELP[9] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[4], regval);
	regval =  (U32)(pELP[10] & BIT_MASK)
			| (U32)(pELP[11] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[5], regval);
}

void	NX_MCUS_SetELP16( U16 *pELP )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELP );

	regval =  (U32)(pELP[0] & BIT_MASK)
			| (U32)(pELP[1] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[0], regval);
	regval =  (U32)(pELP[2] & BIT_MASK)
			| (U32)(pELP[3] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[1], regval);
	regval =  (U32)(pELP[4] & BIT_MASK)
			| (U32)(pELP[5] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[2], regval);
	regval =  (U32)(pELP[6] & BIT_MASK)
			| (U32)(pELP[7] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[3], regval);
	regval =  (U32)(pELP[8] & BIT_MASK)
			| (U32)(pELP[9] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[4], regval);
	regval =  (U32)(pELP[10] & BIT_MASK)
			| (U32)(pELP[11] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[5], regval);
	regval =  (U32)(pELP[12] & BIT_MASK)
			| (U32)(pELP[13] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[6], regval);
	regval =  (U32)(pELP[14] & BIT_MASK)
			| (U32)(pELP[15] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[7], regval);
}
void	NX_MCUS_SetELP24( U16 *pELP )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELP );

	regval =  (U32)(pELP[0] & BIT_MASK)
			| (U32)(pELP[1] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[0], regval);
	regval =  (U32)(pELP[2] & BIT_MASK)
			| (U32)(pELP[3] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[1], regval);
	regval =  (U32)(pELP[4] & BIT_MASK)
			| (U32)(pELP[5] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[2], regval);
	regval =  (U32)(pELP[6] & BIT_MASK)
			| (U32)(pELP[7] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[3], regval);
	regval =  (U32)(pELP[8] & BIT_MASK)
			| (U32)(pELP[9] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[4], regval);
	regval =  (U32)(pELP[10] & BIT_MASK)
			| (U32)(pELP[11] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[5], regval);
	regval =  (U32)(pELP[12] & BIT_MASK)
			| (U32)(pELP[13] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[6], regval);
	regval =  (U32)(pELP[14] & BIT_MASK)
			| (U32)(pELP[15] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[7], regval);
	regval =  (U32)(pELP[16] & BIT_MASK)
			| (U32)(pELP[17] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[8], regval);
	regval =  (U32)(pELP[18] & BIT_MASK)
			| (U32)(pELP[19] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[9], regval);
	regval =  (U32)(pELP[20] & BIT_MASK)
			| (U32)(pELP[21] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[10], regval);
	regval =  (U32)(pELP[22] & BIT_MASK)
			| (U32)(pELP[23] & BIT_MASK)<<BIT_POS;
	WriteIO32(&__g_pRegister->NFELP[11], regval);
}
void	NX_MCUS_GetErrLoc4( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );

	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[0]);
	pELoc[0] = (U16)(regval & BIT_MASK);
	pELoc[1] = (U16)((regval >> BIT_POS) & BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[1]);
	pELoc[2] = (U16)(regval & BIT_MASK);
	pELoc[3] = (U16)((regval >> BIT_POS) & BIT_MASK);
}
void	NX_MCUS_GetErrLoc8( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );

	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[0]);
	pELoc[0] = (U16)(regval					& BIT_MASK);
	pELoc[1] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[1]);
	pELoc[2] = (U16)(regval					& BIT_MASK);
	pELoc[3] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[2]);
	pELoc[4] = (U16)(regval					& BIT_MASK);
	pELoc[5] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[3]);
	pELoc[6] = (U16)(regval					& BIT_MASK);
	pELoc[7] = (U16)((regval >> BIT_POS)	& BIT_MASK);
}
void	NX_MCUS_GetErrLoc12( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );

	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[0]);
	pELoc[0] = (U16)(regval					& BIT_MASK);
	pELoc[1] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[1]);
	pELoc[2] = (U16)(regval					& BIT_MASK);
	pELoc[3] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[2]);
	pELoc[4] = (U16)(regval					& BIT_MASK);
	pELoc[5] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[3]);
	pELoc[6] = (U16)(regval					& BIT_MASK);
	pELoc[7] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[4]);
	pELoc[8] = (U16)(regval					& BIT_MASK);
	pELoc[9] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[5]);
	pELoc[10] = (U16)(regval				& BIT_MASK);
	pELoc[11] = (U16)((regval >> BIT_POS)	& BIT_MASK);
}
void	NX_MCUS_GetErrLoc16( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );

	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[0]);
	pELoc[0] = (U16)(regval					& BIT_MASK);
	pELoc[1] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[1]);
	pELoc[2] = (U16)(regval					& BIT_MASK);
	pELoc[3] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[2]);
	pELoc[4] = (U16)(regval					& BIT_MASK);
	pELoc[5] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[3]);
	pELoc[6] = (U16)(regval					& BIT_MASK);
	pELoc[7] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[4]);
	pELoc[8] = (U16)(regval					& BIT_MASK);
	pELoc[9] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[5]);
	pELoc[10] = (U16)(regval				& BIT_MASK);
	pELoc[11] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[6]);
	pELoc[12] = (U16)(regval				& BIT_MASK);
	pELoc[13] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	regval = ReadIO32(&__g_pRegister->NFERRLOCATION[7]);
	pELoc[14] = (U16)(regval				& BIT_MASK);
	pELoc[15] = (U16)((regval >> BIT_POS)	& BIT_MASK);
}

void	NX_MCUS_GetErrLoc24( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;
	U32 i;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );
	for (i=0; i<24; i=i+2 )
	{
		regval = ReadIO32(&__g_pRegister->NFERRLOCATION[i/2]);
		pELoc[i] = (U16)(regval					& BIT_MASK);
		pELoc[i+1] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	}
}

void	NX_MCUS_GetErrLoc40( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;
	U32 i;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );
	for (i=0; i<40; i=i+2 )
	{
		regval = ReadIO32(&__g_pRegister->NFERRLOCATION[i/2]);
		pELoc[i] = (U16)(regval					& BIT_MASK);
		pELoc[i+1] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	}
}

void	NX_MCUS_GetErrLoc60( U16 *pELoc )
{
	const U32 BIT_SIZE	= 14;
	const U32 BIT_POS	= 14;
	const U32 BIT_MASK	= ((1UL<<BIT_SIZE)-1);
	register U32 regval;
	U32 i;

	NX_ASSERT( CNULL != __g_pRegister );
	NX_ASSERT( CNULL != pELoc );
	for (i=0; i<60; i=i+2 )
	{
		regval = ReadIO32(&__g_pRegister->NFERRLOCATION[i/2]);
		pELoc[i] = (U16)(regval					& BIT_MASK);
		pELoc[i+1] = (U16)((regval >> BIT_POS)	& BIT_MASK);
	}
}

void 	NX_MCUS_WriteNANDAddress	( U8 Address )
{
	*(volatile U8 *)(CS_SIZE*11+0x18) = Address;
}
void 	NX_MCUS_WriteNANDCommand	( U8 Command )
{
	*(volatile U8 *)(CS_SIZE*11+0x10) = Command;
}
void 	NX_MCUS_WriteNANDData		( U32 Data )
{
	*(volatile U8 *)(CS_SIZE*11) = Data;
}
U32  	NX_MCUS_ReadNANDData		( void )
{
	return *(volatile U8 *)(CS_SIZE*11);
}

//void NX_MCUS_GetNANDECC
//(
//	U32& ECC0, ///< [out] ECC code 0
//	U32& ECC1, ///< [out] ECC code 1
//	U32& ECC2, ///< [out] ECC code 2
//	U32& ECC3, ///< [out] ECC code 3
//	U32& ECC4, ///< [out] ECC code 4
//	U32& ECC5, ///< [out] ECC code 5
//	U32& ECC6,  ///< [out] ECC code 6
//	U32& ECC7, ///< [out] ECC code 3
//	U32& ECC8, ///< [out] ECC code 4
//	U32& ECC9, ///< [out] ECC code 5
//	U32& ECC10  ///< [out] ECC code 6
//)
//{
//	NX_ASSERT( CNULL != __g_pRegister );
//	ECC0 = (U32)ReadIO32(&__g_pRegister->NFECC[0]);
//	ECC1 = (U32)ReadIO32(&__g_pRegister->NFECC[1]);
//	ECC2 = (U32)ReadIO32(&__g_pRegister->NFECC[2]);
//	ECC3 = (U32)ReadIO32(&__g_pRegister->NFECC[3]);
//	ECC4 = (U32)ReadIO32(&__g_pRegister->NFECC[4]);
//	ECC5 = (U32)ReadIO32(&__g_pRegister->NFECC[5]);
//	ECC6 = (U32)ReadIO32(&__g_pRegister->NFECC[6]);
//	ECC7 = (U32)ReadIO32(&__g_pRegister->NFECC[7]);
//	ECC8 = (U32)ReadIO32(&__g_pRegister->NFECC[8]);
//	ECC9 = (U32)ReadIO32(&__g_pRegister->NFECC[9]);
//	ECC10 = (U32)ReadIO32(&__g_pRegister->NFECC[10]);
//}

void NX_MCUS_SetNANDECCCONFIG(CBOOL DECMODE, U32 DATASIZE, U32 PARITYSIZE, U32 ELPNUMBER)
{
	U32 regval;
	NX_ASSERT( CNULL != __g_pRegister );

	regval = ReadIO32(&__g_pRegister->NFECCCTRL)& ~0x011fffff;
	regval = (regval | (DECMODE<<24) | DATASIZE | (PARITYSIZE<<10) | (ELPNUMBER<<16));

	WriteIO32(&__g_pRegister->NFECCCTRL, regval);
}

U32 NX_MCUS_GetErrorPos( U32 number )
{
	U32 Reg;
	NX_ASSERT( CNULL != __g_pRegister );

	Reg = ReadIO32(&__g_pRegister->NFERRLOCATION[number/2]);

	if (number&0x1) Reg = Reg >> 14;

	return (Reg & 0x3fff);
}


