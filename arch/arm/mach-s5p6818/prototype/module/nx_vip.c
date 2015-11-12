//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		:
//	File		: nx_vip.c
//	Description	:
//	Author		: Junseo Kim (junseo@nexell.co.kr)
//	History		: 2012-11-12 First Realease
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_vip.h"

static	NX_VIP_RegisterSet *__g_pRegister[NUMBER_OF_VIP_MODULE];


//------------------------------------------------------------------------------
//
//	VIP Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicate that Initialize is successed.
 *			 CFALSE	indicate that Initialize is failed.
 *	@see	NX_VIP_GetNumberOfModule
 */
CBOOL	NX_VIP_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		/* memset( __g_pRegister, 0, sizeof(__g_pRegister) ); */
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. 
 *				It is equal to NUMBER_OF_VIP_MODULE in <nx_chip.h>.
 */
U32		NX_VIP_GetNumberOfModule( void )
{
	return NUMBER_OF_VIP_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_VIP_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_VIP_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_VIP_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_VIP_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_VIP_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_VIP?_MODULE in <nx_chip.h>.
 */
U32		NX_VIP_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_VIP_MODULE] =
	{
		PHY_BASEADDR_LIST( VIP )
	};
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_VIP_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_VIP_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_VIP_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_VIP?_MODULE in <nx_chip.h>.
 */
U32 NX_VIP_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( VIP )
	};
	NX_CASSERT( NUMBER_OF_VIP_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_VIP?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_VIP_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( VIP, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_VIP_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_VIP?_MODULE in <nx_chip.h>.
 */
U32 	NX_VIP_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( VIP ) };
    NX_CASSERT( NUMBER_OF_VIP_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_VIP0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_VIP_INTCH_xxx in <nx_vip.h>
 *	@param[in]	Enable	 Set as CTRUE to enable a specified interrupt. 
 *						 Set as CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_VIP_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	const U16 ODINTENB_BITPOS = 8;

	register U16 regvalue;
	register NX_VIP_RegisterSet*	pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( 3>IntNum );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( 2 > IntNum )
	{
		regvalue = pRegister->VIP_HVINT & ~( 1 << (8+IntNum) );

		if( Enable ){	regvalue |=	( 1 << (8+IntNum));	}
		else		{	regvalue &= ~( 1 << (8+IntNum));	}

	//	pRegister->VIP_HVINT = regvalue;
		WriteIO32(&pRegister->VIP_HVINT, regvalue);
	}
	else
	{
		if( Enable )
		{
		//	pRegister->VIP_ODINT = 1 << ODINTENB_BITPOS;
			WriteIO32(&pRegister->VIP_ODINT, (1 << ODINTENB_BITPOS));
		}
		else
		{
		//	pRegister->VIP_ODINT = 0;
			WriteIO32(&pRegister->VIP_ODINT, 0);
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_VIP_INTCH_xxx in <nx_vip.h>
 *	@return		 CTRUE	indicates that a specified interrupt is enabled. 
 *				 CFALSE	indicates that a specified interrupt is disabled.

 */
CBOOL	NX_VIP_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	const U16 ODINTENB_BITPOS	= 8;
	const U16 ODINTENB_MASK		= 1 << ODINTENB_BITPOS;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( 3>IntNum );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( 2 > IntNum )
	{
		return (__g_pRegister[ModuleIndex]->VIP_HVINT & ( 1 << (8+IntNum)) ) ? CTRUE : CFALSE;
	}

	return (__g_pRegister[ModuleIndex]->VIP_ODINT & ODINTENB_MASK) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_VIP_INTCH_xxx in <nx_vip.h>
 *	@return		 CTRUE	indicates that a specified interrupt is pended. 
 *				 CFALSE	indicates that a specified interrupt is not pended.

 */
CBOOL	NX_VIP_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U16 ODINTPEND_BITPOS	 = 0;
	const U16 ODINTPEND_MASK	 = 1 << ODINTPEND_BITPOS;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( 3 > IntNum );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	if( 2 > IntNum )
	{
		return (__g_pRegister[ModuleIndex]->VIP_HVINT & (1 << IntNum) ) ? CTRUE : CFALSE;
	}

	return (__g_pRegister[ModuleIndex]->VIP_ODINT & ODINTPEND_MASK) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.
 *						refer to NX_VIP_INTCH_xxx in <nx_vip.h>
 *	@return		None.

 */
void	NX_VIP_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	const U32 VIPINTENB_MASK = 0x03 << 8;

	register NX_VIP_RegisterSet	*pRegister;
	register U16 regvalue;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( 3>IntNum );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( 2 > IntNum )
	{
		regvalue = pRegister->VIP_HVINT & VIPINTENB_MASK;

		regvalue |= (0x01 << IntNum);

	//	pRegister->VIP_HVINT = regvalue;
		WriteIO32(&pRegister->VIP_HVINT, regvalue);
	}
	else
	{
		regvalue  = pRegister->VIP_ODINT;
		regvalue |= 1;

	//	pRegister->VIP_ODINT = regvalue;
		WriteIO32(&pRegister->VIP_ODINT, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 Set as CTRUE to enable all interrupts. 
 *						 Set as CFALSE to disable all interrupts.
 *	@return		None.

 */
void	NX_VIP_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( Enable )
	{
	//	pRegister->VIP_HVINT	= (U16)(0x03<<8);
		WriteIO32(&pRegister->VIP_HVINT, (U16)(0x03<<8));

	//	pRegister->VIP_ODINT	= (U16)(0x01<<8);
		WriteIO32(&pRegister->VIP_ODINT, (U16)(0x01<<8));
	}
	else
	{
	//	pRegister->VIP_HVINT	= (U16)0x00;
		WriteIO32(&pRegister->VIP_HVINT, (U16)0x00);

	//	pRegister->VIP_ODINT	= (U16)0x00;
		WriteIO32(&pRegister->VIP_ODINT, (U16)0x00);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are enabled. 
 *				 CFALSE	indicates that all interrupts are disabled.

 */
CBOOL	NX_VIP_GetInterruptEnableAll( U32 ModuleIndex )
{
	const U16 VHSINTENB_MASK	= 0x03 << 8;
	const U16 ODINTENB_MASK		= 0x01 << 8;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( (pRegister->VIP_HVINT & VHSINTENB_MASK) || (pRegister->VIP_ODINT & ODINTENB_MASK) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are pended. 
 *				 CFALSE	indicates that no interrupt is pended.

 */
CBOOL	NX_VIP_GetInterruptPendingAll( U32 ModuleIndex )
{
	const U16 VHSINTPEND_MASK	= 0x03;
	const U16 ODINTPEND_MASK	= 0x01;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	if( (pRegister->VIP_HVINT & VHSINTPEND_MASK) || (pRegister->VIP_ODINT & ODINTPEND_MASK) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.

 */
void	NX_VIP_ClearInterruptPendingAll( U32 ModuleIndex )
{
	const U16 VHSINTPEND_MASK	= 0x03;
	const U16 ODINTPEND_MASK	= 0x01;

	register NX_VIP_RegisterSet	*pRegister;
	register U16 	regvalue;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = pRegister->VIP_HVINT;
	regvalue |= VHSINTPEND_MASK;

//	pRegister->VIP_HVINT = regvalue;
	WriteIO32(&pRegister->VIP_HVINT, regvalue);

	regvalue  = pRegister->VIP_ODINT;
	regvalue |= ODINTPEND_MASK;

//	pRegister->VIP_ODINT = regvalue;
	WriteIO32(&pRegister->VIP_ODINT, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_VIP_INTCH_xxx in <nx_vip.h>

 */
S32		NX_VIP_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	const S16	VIP_PEND[8] = { -1, 0, 1, 0, 2, 0, 1, 0 };

	register NX_VIP_RegisterSet	*pRegister;
	register U16 Pend;
	register U16 Pend0;
	register U16 Pend1;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	Pend0 = pRegister->VIP_HVINT;
	Pend0 = Pend0 & (Pend0>>8);

	Pend1 = pRegister->VIP_ODINT;
	Pend1 = Pend1 & (Pend1>>8);

	Pend = ( Pend1<<2 | Pend0 );

	NX_ASSERT( 8 > Pend );

	return VIP_PEND[Pend];
}

//------------------------------------------------------------------------------
//	PAD Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get number of PAD mode
 *	@param[in]	ModuleIndex	an index of module.
 *	@return		number of PAD mode.
 *	@see		NX_VIP_EnablePAD
 */
U32 NX_VIP_GetNumberOfPADMode ( U32 ModuleIndex )
{
	// NXP4330 only
	const U32 number_of_pad_mode[] = {1,2};
	return number_of_pad_mode[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Enable PAD for a module.
 *	@param[in]	ModuleIndex	an index of module.
 *	@see		NX_VIP_GetNumberOfPADMode
 */
#if 0
void NX_VIP_EnablePAD ( U32 ModuleIndex, U32 ModeIndex )
{
	// NXP4330 only
	if( ModuleIndex==1 && ModeIndex==1 )
	{
		U32 i;
		const U32 PADNumber[] =	{
		    PADINDEX_OF_VIP1_i_ExtCLK2   ,
		    PADINDEX_OF_VIP1_i_ExtHSYNC2 ,
		    PADINDEX_OF_VIP1_i_ExtVSYNC2 ,
		    PADINDEX_OF_VIP1_i_VD2_0_    ,
		    PADINDEX_OF_VIP1_i_VD2_1_    ,
		    PADINDEX_OF_VIP1_i_VD2_2_    ,
		    PADINDEX_OF_VIP1_i_VD2_3_    ,
		    PADINDEX_OF_VIP1_i_VD2_4_    ,
		    PADINDEX_OF_VIP1_i_VD2_5_    ,
		    PADINDEX_OF_VIP1_i_VD2_6_    ,
		    PADINDEX_OF_VIP1_i_VD2_7_    ,
		};

		for(i=0; i<sizeof( PADNumber)/sizeof(PADNumber[0]); i++)
		{
            NX_SWITCHDEVICE_Set_Switch_Enable ( PADNumber[i] );
			NX_GPIO_SetPadFunctionEnable       ( PADNumber[i], ModeIndex );
		}
	}
	else
	{
		U32 i;
		const U32 PADNumber[][NUMBER_OF_VIP_MODULE] =	{
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_ExtCLK   ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_ExtHSYNC ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_ExtVSYNC ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_DValid   ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_0_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_1_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_2_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_3_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_4_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_5_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_6_    ) },
		     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_7_    ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_8_    ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_9_    ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_10_   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_11_   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_12_   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_13_   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_14_   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VD_15_   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, i_VCLK27   ) },
		//     { PADINDEX_WITH_CHANNEL_LIST( VIP, o_VCLK27    ) },
		};
		NX_CASSERT( NUMBER_OF_VIP_MODULE == (sizeof(PADNumber[0])/sizeof(PADNumber[0][0])) );
		NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );

		for(i=0; i<sizeof( PADNumber)/sizeof(PADNumber[0]); i++)
		{
			NX_SWITCHDEVICE_Set_Switch_Enable ( PADNumber[i][ModuleIndex] );
			NX_PAD_SetPadFunctionEnable       ( PADNumber[i][ModuleIndex], ModeIndex );
		}
	}
}
#endif

//------------------------------------------------------------------------------
//	Module customized operations
//------------------------------------------------------------------------------
/**
 *	@brief		Enable/Disable Video Input Port
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	bVIPEnb		Set it to CTURE to enable VIP.
 *	@param[in]	bSepEnb		Set it to CTURE to enable Separator.
 *	@param[in]	bClipEnb	Set it to CTRUE to enable Clipper.
 *	@param[in]	bDeciEnb	Set it to CTRUE to enable Decimator.
 *	@return		None
 */
void
NX_VIP_SetVIPEnable
(
	U32   ModuleIndex,
	CBOOL bVIPEnb,
	CBOOL bSepEnb,
	CBOOL bClipEnb,
	CBOOL bDeciEnb
)
{
	const U16 VIPENB	= 1U<<0;
	const U16 SEPENB	= 1U<<8;
	const U16 CLIPENB	= 1U<<1;
	const U16 DECIENB	= 1U<<0;

	register U16 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	temp = pRegister->VIP_CONFIG;
	if( bVIPEnb )	temp |= (U16) VIPENB;
	else			temp &= (U16)~VIPENB;

//	pRegister->VIP_CONFIG = temp;
	WriteIO32(&pRegister->VIP_CONFIG, temp);

	temp = 0;
	if( bSepEnb )	temp |= (U16)SEPENB;
	if( bClipEnb )	temp |= (U16)CLIPENB;
	if( bDeciEnb )	temp |= (U16)DECIENB;

//	pRegister->VIP_CDENB = temp;
	WriteIO32(&pRegister->VIP_CDENB, temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Informs whether VIP controllers are enabled or disabled.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pbVIPEnb	Determines whether VIP is enabled or not.
 *							CTRUE = Enable, CFALSE = Disable.
 *	@param[out] pbSepEnb	Determines whether Separator is enabled or not.
 *							CTRUE = Enable, CFALSE = Disable.
 *	@param[out] pbClipEnb	Determines whether Clipper is enabled or not.
 *							CTRUE = Enable, CFALSE = Disable.
 *	@param[out] pbDeciEnb	Determines whether Decimator is enabled or not.
 *							CTRUE = Enable, CFALSE = Disable.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 */
void
NX_VIP_GetVIPEnable
(
	U32    ModuleIndex,
	CBOOL *pbVIPEnb,
	CBOOL *pbSepEnb,
	CBOOL *pbClipEnb,
	CBOOL *pbDeciEnb
)
{
	const U16 VIPENB	= 1U<<0;
	const U16 SEPENB	= 1U<<8;
	const U16 CLIPENB	= 1U<<1;
	const U16 DECIENB	= 1U<<0;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( CNULL != pbVIPEnb )		*pbVIPEnb	= (pRegister->VIP_CONFIG & VIPENB ) ? CTRUE : CFALSE;
	if( CNULL != pbSepEnb )		*pbSepEnb	= (pRegister->VIP_CDENB	& SEPENB ) ? CTRUE : CFALSE;
	if( CNULL != pbClipEnb )	*pbClipEnb	= (pRegister->VIP_CDENB	& CLIPENB) ? CTRUE : CFALSE;
	if( CNULL != pbDeciEnb )	*pbDeciEnb	= (pRegister->VIP_CDENB	& DECIENB) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Select VIP module's input port.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	InputPort	Select input port .
 *							- Port A : Use VIDCLK0, VIDHSYNC0, VIDVSYNC0, FIELD/DVALID, VID0 pin for connection to Video decoder. 
 *							- Port B : Use VIDCLK1, VIDHSYNC1, VIDVSYNC1, SIPFIELD/DVALID, VID1 pin for connection to Video decoder.
 *	@return		None.
 *	@remark		NXP2120 only has one VIP module. but VIP module have 2 set GPIO for connection to video decoder. 
 *				so, Should select which pin use.
 */
void	NX_VIP_SetInputPort( U32 ModuleIndex, NX_VIP_INPUTPORT InputPort )
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NX_VIP_INPUTPORT_B >= InputPort );
	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

//	pRegister->VIP_VIP1 = (U32)InputPort;
	WriteIO32(&pRegister->VIP_VIP1, (U32)InputPort);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get selected input port of VIP modules.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return		NX_VIP_INPUTPORT_A	: Port A connected to VIP module(VIDCLK0, VIDHSYNC0, VIDVSYNC0, FIELD/DVALID, VID0). 
 *				NX_VIP_INPUTPORT_B	: Port B connected to VIP module(VIDCLK1, VIDHSYNC1, VIDVSYNC1, SIPFIELD/DVALID, VID1). 
 */
NX_VIP_INPUTPORT	NX_VIP_GetInputPort( U32 ModuleIndex )
{
	const U16 SIPENB_MASK = 1 << 0;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (NX_VIP_INPUTPORT)(__g_pRegister[ModuleIndex]->VIP_VIP1 & SIPENB_MASK);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set settings for the data mode.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	DataOrder	Specifies the input video data order.
 *	@param[in]	DataWidth	Specifies the input data width. 8 or 16 are only valid.
 *	@return		None
 */
void
NX_VIP_SetDataMode
(
	U32 				ModuleIndex,
	NX_VIP_DATAORDER	DataOrder,
	U32					DataWidth
)
{
	const U32 DORDER_POS	= 2;
	const U32 DORDER_MASK	= 3UL<<DORDER_POS;
	const U32 DWIDTH_POS	= 1;
	const U32 DWIDTH_MASK	= 1UL<<DWIDTH_POS;
	register U32 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 4 > DataOrder );
	NX_ASSERT( 8 == DataWidth || 16 == DataWidth );

	pRegister = __g_pRegister[ModuleIndex];

	temp = (U32)pRegister->VIP_CONFIG;
	temp &= ~(DORDER_MASK | DWIDTH_MASK);
	temp |= ((U32)DataOrder<<DORDER_POS);
	temp |= ((8 == DataWidth) ? DWIDTH_MASK : 0);

//	pRegister->VIP_CONFIG = (U16)temp;
	WriteIO32(&pRegister->VIP_CONFIG, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current settings for the data mode.
 *  @param[in]  ModuleIndex		An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pDataOrder		Indicates the input video data order.
 *	@param[out] pDataWidth		Indicates the input data width.
 *	@return		None
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_GetDataConfig
 */
void
NX_VIP_GetDataMode
(
	U32 				 ModuleIndex,
	NX_VIP_DATAORDER	*pDataOrder,
	U32					*pDataWidth
)
{
	const U32 DORDER_POS	= 2;
	const U32 DORDER_MASK	= 3UL<<DORDER_POS;
	const U32 DWIDTH_POS	= 1;
	const U32 DWIDTH_MASK	= 1UL<<DWIDTH_POS;
	register U32 temp;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	temp = (U32)__g_pRegister[ModuleIndex]->VIP_CONFIG;

	if( CNULL != pDataOrder )	*pDataOrder = (NX_VIP_DATAORDER)((temp & DORDER_MASK)>>DORDER_POS);
	if( CNULL != pDataWidth )	*pDataWidth = (temp & DWIDTH_MASK) ? 8 : 16;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set configuration for sync generation.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	bExtSync	Specifies whether external H/V Sync signals are used or not.
 *							CTRUE = External H/V Sync, CFALSE = H/V Sync in Embedded Sync.

 *	@param[in]	AVW		Specifies the active video width in clocks, 0 ~ 8190.
 *							This value must be a multiple of 2.
 *	@param[in]	AVH		Specifies the active video height in lines. 0 ~ 2047.

 *	@param[in]	HSW		Specifies the horizontal sync width in clocks.
 *							This argument is only valid when bExtSync is CFALSE.
 *	@param[in]	HFP		Specifies the horizontal sync front porch in clocks.
 *							This value must be greater than 7.
 *							This argument is only valid when bExtSync is CFALSE.
 *	@param[in]	HBP		Specifies the horizontal sync back porch in clocks.
 *							This value must be greater than 0.
 *							This argument is only valid when bExtSync is CTRUE.

 *	@param[in]	VSW		Specifies the vertical sync width in lines.
 *							This argument is only valid when bExtSync is CFALSE.
 *	@param[in]	VFP		Specifies the vertical sync front porch in lines.
 *							This argument is only valid when bExtSync is CFALSE.
 *	@param[in]	VBP		Specifies the vertical sync back porch in lines.
 *							This argument is only valid when bExtSync is CTRUE.

 *	@return		None.
 *	@remark		You have to set sync parameters even if Embedded sync is used.
 *				A sum of arguments(HSW + HFP + HBP or VSW + VFP + VBP) has to be less than 65536.
 *				See follwing figure for more details.
 *	@code
 *
 *						<----------------TOTAL----------------->
 *						<-SW->
 *						+-------+								+-------+
 *						|		|								|		|
 *		Sync	--------+		+--------------/-/--------------+		+---
 *				<-FP->		<-BP-> <--ACTIVE VIDEO-->
 *		Active	--+					+-------/-/--------+
 *		Video	|		(BLANK)		|	(ACTIVE DATA)	|		(BLANK)
 *				+--------------------+					+-----------------
 *	@endcode
 *	@see		NX_VIP_GetHVSync
 */
void
NX_VIP_SetSync
(
	U32				ModuleIndex,
	CBOOL			bExtSync,
	NX_VIP_VD_BITS	SourceBits,
	U32				AVW,
	U32				AVH,
	U32				HSW,
	U32				HFP,
	U32				HBP,
	U32				VSW,
	U32				VFP,
	U32				VBP
)
{
	const U32 DRANGE		= 1UL<<9;
	const U32 EXTSYNCENB	= 1UL<<8;

	register U32 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	// SyncGen의 신호들이 제멋대로 움직이는 것을 막기 위해
	// begin/end값들을 일단 최대값으로 채워둔다.
	WriteIO32(&pRegister->VIP_VBEGIN, 0xFFFFFFFF);
	WriteIO32(&pRegister->VIP_VEND  , 0xFFFFFFFF);
	WriteIO32(&pRegister->VIP_HBEGIN, 0xFFFFFFFF);
	WriteIO32(&pRegister->VIP_HEND  , 0xFFFFFFFF);

	temp = (U32)pRegister->VIP_CONFIG;
	temp &= ~DRANGE;	// Reserved 0.
	if( bExtSync )
	{
		temp |=	EXTSYNCENB;
	}
	else
	{
		temp &= ~EXTSYNCENB;
	}
	WriteIO32(&pRegister->VIP_CONFIG, (U16)temp);

	NX_ASSERT( 32768 > AVW && 0 == (AVW & 1) );
	NX_ASSERT( 32768 > AVH );

	switch( SourceBits )
	{
	case NX_VIP_VD_8BIT	:
		WriteIO32(&pRegister->VIP_IMGWIDTH, (U16)(AVW>>1));
		WriteIO32(&pRegister->VIP_IMGHEIGHT, (U16)AVH);
		break;
	case NX_VIP_VD_16BIT:
		WriteIO32(&pRegister->VIP_IMGWIDTH , (U16)AVW);
		WriteIO32(&pRegister->VIP_IMGHEIGHT, (U16)AVH);
		break;
	default:
		break;
	}

	if( bExtSync )
	{
		NX_ASSERT( 0 <= VBP );
		NX_ASSERT( 65536 >= (VBP + AVH));
		NX_ASSERT( 0 <= HBP );
		NX_ASSERT( 65536 >= (HBP + AVW));
		if( 0!=VBP )
		{
			temp = (U32)pRegister->VIP_SYNCCTRL;
			temp &= ~(3<<11); // { VSYNCGENSOURCE(0:H-Sync 1:Video-clock),ExtVSyncMode (0:Sync 1:Blank)}
			WriteIO32(&pRegister->VIP_SYNCCTRL, (U16)temp);

			WriteIO32(&pRegister->VIP_VBEGIN, (U16)(VBP - 1));
			WriteIO32(&pRegister->VIP_VEND, (U16)(VBP + AVH - 1));
		}
		else
		{
			temp = (U32)pRegister->VIP_SYNCCTRL;
			temp |= (3<<11); // { VSYNCGENSOURCE(0:H-Sync 1:Video-clock),ExtVSyncMode (0:Sync 1:Blank)}
			WriteIO32(&pRegister->VIP_SYNCCTRL, (U16)temp);

			/*WriteIO32(&pRegister->VIP_VBEGIN, (U16)(HFP + HSW + HBP + 1));*/
			/*WriteIO32(&pRegister->VIP_VEND, (U16)((HFP + HSW + HBP)*2 + 1));*/
            WriteIO32(&pRegister->VIP_VBEGIN, (U16)(VFP + 1));
            WriteIO32(&pRegister->VIP_VEND, (U16)(VFP + VSW + 1));
		}
		if( 0!=HBP )
		{
			temp = (U32)pRegister->VIP_SYNCCTRL;
			temp &= ~(1<<10);// ExtHSyncMode (0:Sync 1:Blank)
			WriteIO32(&pRegister->VIP_SYNCCTRL, (U16)temp);

			WriteIO32(&pRegister->VIP_HBEGIN, (U16)(HBP - 1));
			WriteIO32(&pRegister->VIP_HEND, (U16)(HBP + AVW - 1));
		}
		else
		{
			temp = (U32)pRegister->VIP_SYNCCTRL;
			temp |= (1<<10);// ExtHSyncMode (0:Sync 1:Blank)
			WriteIO32(&pRegister->VIP_SYNCCTRL, (U16)temp);

			/*WriteIO32(&pRegister->VIP_HBEGIN, (U16)(HFP - 7));*/
			/*WriteIO32(&pRegister->VIP_HEND, (U16)(HFP + HSW - 7));*/
			WriteIO32(&pRegister->VIP_HBEGIN, (U16)(HFP));
			WriteIO32(&pRegister->VIP_HEND, (U16)(HFP + HSW));
		}
	}
	else
	{
		NX_ASSERT( 65536 > (VFP + VSW) );
		//NX_ASSERT( 7 <= HFP );
		//NX_ASSERT( 65536 > (HFP + HSW - 7) );
		WriteIO32(&pRegister->VIP_VBEGIN, (U16)(VFP + 1));
		WriteIO32(&pRegister->VIP_VEND, (U16)(VFP + VSW + 1));
		WriteIO32(&pRegister->VIP_HBEGIN, (U16)(HFP - 7));
		WriteIO32(&pRegister->VIP_HEND, (U16)(HFP + HSW - 7));
	}
}
void
NX_VIP_SetHVSync
(
	U32		ModuleIndex,
	CBOOL		bExtSync,
	U32			AVW,
	U32			AVH,
	U32			HSW,
	U32			HFP,
	U32			HBP,
	U32			VSW,
	U32			VFP,
	U32			VBP
)
{
	NX_VIP_SetSync( ModuleIndex, bExtSync,NX_VIP_VD_8BIT,AVW,AVH,HSW,HFP,HBP,VSW,VFP,VBP );
}

void	NX_VIP_SetHVSyncForMIPI( U32 ModuleIndex, U32 AVW, U32 AVH,
								U32 HSW, U32 HFP, U32 HBP,
								U32 VSW, U32 VFP, U32 VBP )
{
	CBOOL bExtSync			= CTRUE ;
	CBOOL bExtDValid		= CTRUE ;
	CBOOL Bypass_ExtDValid	= CTRUE ;
	CBOOL Bypass_ExtSync	= CFALSE;

	NX_VIP_SetSync( ModuleIndex, bExtSync,NX_VIP_VD_16BIT,AVW>>1,AVH,HSW,HFP,0,VSW,VFP,0 );
	NX_VIP_SetDValidMode( ModuleIndex, bExtDValid, Bypass_ExtDValid, Bypass_ExtSync );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current configuration for sync generation.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pbExtSync	Indicates whether the external H/V Sync signals are used or not.\n
 *							CTRUE = External H/V Sync, CFALSE = H/V Sync in Embedded Sync.

 *	@param[out] pAVW		Indicates the active video width in clocks.
 *	@param[out] pAVH		Indicates the active video height in lines.

 *	@param[out] pHBEGIN	When *pbExtSync is CTRUE, this value is used to generate a internal horizontal blank.
 *							This value specifies the number of clocks from the end of horizontal sync to the end of horizontal blank. \n
 *							-> HBEGIN = tHBP - 1 \n
 *							When *pbExtSync is CFALSE, this value is used to generate a internal horizontal sync.
 *							This value specifies the number of clocks from the begin of horizontal blank to the begin of horizontal sync.\n
 *							-> HBEGIN = tHFP - 7
 *	@param[out] pHEND		When EXTSYNCENB is set, this value is used to generate a internal horizontal blank.
 *							This value specifies the number of clocks from the end of horizontal sync to the begin of horizontal blank.\n
 *							-> HEND = tHBP + tAVW - 1 \n
 *							When EXTSYNCENB is clear, this value is used to generate a internal horizontal sync.
 *							This value specifies the number of clocks from the begin of horizontal blank to the end of horizontal sync.\n
 *							-> HEND = tHFP + tHSW - 7
 *	@param[out] pVBEGIN	When *pbExtSync is CTRUE, this value is used to generate a internal vertical blank.
 *							This value specifies the number of lines from the end of vertical sync to the end of vertical blank.\n
 *							-> VBEGIN = tVBP - 1 \n
 *								When *pbExtSync is CFALSE, this value is used to generate a internal vertical sync.
 *							This value specifies the number of lines from the begin of vertical blank to the begin of vertical sync.\n
 *							-> VBEGIN = tVFP + 1
 *	@param[out] pVEND		When *pbExtSync is CTRUE, this value is used to generate a internal vertical blank.
 *							This value specifies the number of lines from the end of vertical sync to the begin of vertical blank.\n
 *							-> VEND = tVBP + tAVH - 1 \n
 *								When *pbExtSync is CFALSE, this value is used to generate a internal vertical sync.
 *							This value specifies the number of lines from the begin of vertical blank to the end of vertical sync.\n
 *							-> VEND = tVFP + tVSW + 1
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetHVSync
 */
void
NX_VIP_GetHVSync
(
	U32 	 ModuleIndex,
	CBOOL	*pbExtSync,
	U32		*pAVW,
	U32		*pAVH,
	U32		*pHBEGIN,
	U32		*pHEND,
	U32		*pVBEGIN,
	U32		*pVEND
)
{
	const U16 EXTSYNCENB = 1U<<8;
	CBOOL	bExtSync;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	bExtSync = (pRegister->VIP_CONFIG & EXTSYNCENB) ? CTRUE : CFALSE;
	if( CNULL != pbExtSync )	*pbExtSync = bExtSync;

	if( CNULL != pAVW )		*pAVW	 = ((U32)(pRegister->VIP_IMGWIDTH)<<1) - ((bExtSync) ? 0 : 4);
	if( CNULL != pAVH )		*pAVH	 = (U32)pRegister->VIP_IMGHEIGHT;

	if( CNULL != pVBEGIN )	*pVBEGIN = (U32)pRegister->VIP_VBEGIN;
	if( CNULL != pVEND )	*pVEND	 = (U32)pRegister->VIP_VEND;
	if( CNULL != pHBEGIN )	*pHBEGIN = (U32)pRegister->VIP_HBEGIN;
	if( CNULL != pHEND )	*pHEND	 = (U32)pRegister->VIP_HEND;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set configuration for data valid signal.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	bExtDValid	Specifies whether external DValid signal is used or not.\n
 *							CTRUE = External DVaild & Internal generation, CFALSE = Internal generation only.
 *	@param[in]	bDValidPol	Specifies the external DValid polarity.\n
 *							This argument is only valid when bExtDValid is set to CTRUE.\n
 *							CTRUE = Active High, CFALSE = Active Low.
 *	@return		None.
 *	@see		NX_VIP_GetDValidMode
 */
void
NX_VIP_SetDValidMode
(
	U32 	ModuleIndex,
	CBOOL	bExtDValid,
	CBOOL	bDValidPol,
	CBOOL bSyncPol
)
{
	const U32 DVALIDPOL	= 1UL<<4;
	const U32 EXTDVENB	= 1UL<<2;

	register U32 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	temp = (U32)pRegister->VIP_SYNCCTRL;

	if( bExtDValid )	temp |=	EXTDVENB;
	else				temp &= ~EXTDVENB;
	if( bDValidPol )	temp |=	DVALIDPOL;
	else				temp &= ~DVALIDPOL;

	temp &= ~((1<<8) | (1<<9));
	temp |= (( (bSyncPol    & 0x1)<<8 ) |
           ( (bSyncPol    & 0x1)<<9 ) );

//	pRegister->VIP_SYNCCTRL = (U16)temp;
	WriteIO32(&pRegister->VIP_SYNCCTRL, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current configuration for data valid signal.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pbExtDValid	Indicates whether external DValid signal is used or not.\n
 *								CTRUE = External DVaild & Internal generation, CFALSE = Internal generation only.
 *	@param[out] pbDValidPol		Indicates the external DValid polarity.\n
 *								This argument is only valid when *pbExtDValid is CTRUE.\n
 *								CTRUE = Active High, CFALSE = Active Low.
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetDValidMode
 */
void
NX_VIP_GetDValidMode
(
	U32 	 ModuleIndex,
	CBOOL	*pbExtDValid,
	CBOOL	*pbDValidPol
)
{
	const U32 DVALIDPOL	= 1UL<<4;
	const U32 EXTDVENB	= 1UL<<2;

	register U32 temp;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	temp = (U32)__g_pRegister[ModuleIndex]->VIP_SYNCCTRL;

	if( CNULL != pbExtDValid )	*pbExtDValid = (temp & EXTDVENB	) ? CTRUE : CFALSE;
	if( CNULL != pbDValidPol )	*pbDValidPol = (temp & DVALIDPOL ) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set configuration for field signal processing.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	bExtField	Specifies whether the external Field signal is used or not.\n
 *							CTRUE = External field, CFALSE = Field signal in Embedded Sync.
 *	@param[in]	FieldSel	Specifies the field selection.

 *	@param[in]	bInterlace	Specifies whether the scan mode for the Clipper & Decimator is interlace or not.\n
 *							CTRUE = Interlace mode, CFALSE = Progressive mode.
 *	@param[in]	bInvField	Specifies the Field polarity which is used for the Clipper & Decimator.\n
 *							CTRUE = Bypass (Low is odd field), CFALSE = Invert (Low is even field).
 *	@return		None.
 *	@see		NX_VIP_GetFieldMode, NX_VIP_GetFieldStatus
 */
void
NX_VIP_SetFieldMode
(
	U32 	 	    ModuleIndex,
	CBOOL			bExtField,
	NX_VIP_FIELDSEL	FieldSel,
	CBOOL			bInterlace,
	CBOOL			bInvField
)
{
	const U32 EXTFIELDENB			= 1UL<<3;
	const U32 NX_VIP_FIELDSEL_MASK	= 3UL<<0;
	const U32 INTERLACEENB			= 1UL<<1;
	const U32 FIELDINV				= 1UL<<0;
	register U32 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( 4 > FieldSel );
	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	temp = (U32)pRegister->VIP_SYNCCTRL;

	if( bExtField )		temp |=	EXTFIELDENB;
	else				temp &= ~EXTFIELDENB;

	temp &= ~NX_VIP_FIELDSEL_MASK;
	temp |= (U32)(FieldSel);

//	pRegister->VIP_SYNCCTRL = (U16)temp;
	WriteIO32(&pRegister->VIP_SYNCCTRL, (U16)temp);

//	pRegister->VIP_SCANMODE = (U16)(((bInterlace) ? INTERLACEENB : 0)
//									| ((bInvField ) ? FIELDINV		: 0));

	WriteIO32(&pRegister->VIP_SCANMODE, (U16)(((bInterlace) ? INTERLACEENB : 0) | ((bInvField ) ? FIELDINV : 0)));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get current configuration for field signal processing.
 *  @param[in]  ModuleIndex		An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pbExtField		Indicates whether the external Field signal is used or not.\n
 *								CTRUE = External field, CFALSE = Field signal in Embedded Sync.
 *	@param[out] pFieldSel		Indicates the field selection.

 *	@param[out] pbInterlace	Indicates whether the scan mode for the Clipper & Decimator is interlace or not.\n
 *								CTRUE = Interlace mode, CFALSE = Progressive mode.
 *	@param[out] pbInvField		Indicates the Field polarity which is used for the Clipper & Decimator.\n
 *								CTRUE = Bypass (Low is odd field), CFALSE = Invert (Low is even field).
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetFieldMode, NX_VIP_GetFieldStatus
 */
void
NX_VIP_GetFieldMode
(
	U32 	 	     ModuleIndex,
	CBOOL			*pbExtField,
	NX_VIP_FIELDSEL	*pFieldSel,
	CBOOL			*pbInterlace,
	CBOOL			*pbInvField
)
{
	const U32 EXTFIELDENB			= 1UL<<3;
	const U32 NX_VIP_FIELDSEL_MASK	= 3UL<<0;
	const U32 INTERLACEENB			= 1UL<<1;
	const U32 FIELDINV				= 1UL<<0;
	register U32 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	temp = (U32)pRegister->VIP_SYNCCTRL;
	if( CNULL != pbExtField )	*pbExtField = (temp & EXTFIELDENB) ? CTRUE : CFALSE;
	if( CNULL != pFieldSel )	*pFieldSel	= (NX_VIP_FIELDSEL)(temp & NX_VIP_FIELDSEL_MASK);

	temp = (U32)pRegister->VIP_SCANMODE;
	if( CNULL != pbInterlace )	*pbInterlace = (temp & INTERLACEENB) ? CTRUE : CFALSE;
	if( CNULL != pbInvField )	*pbInvField	 = (temp & FIELDINV	) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get the last field status.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	CTRUE	indicates the last field is even field.\n
 *			CFALSE	indicate the last field is odd field.
 *	@remark	This field signal is generated by VIP block and transferred to the
 *			Clipper & Decimator block. Therefore this field status does not
 *			represent the result of the field signal polarity control in the
 *			Clipper & Decimator block.\n
 *			This field signal is update at every FrameStart(the begin of the
 *			vertical active video). Therefore If you call this function after
 *			the VSYNC interrupt, It returns the last field status which is
 *			already processed for the input data.\n
 *			Because the field signal is captured by PCLK, you have to call the
 *			function SetClockPClkMode() with PCLKMODE_ALWAYS value to get
 *			correct field status.
 *	@see	NX_VIP_SetFieldMode, NX_VIP_GetFieldMode
 */
CBOOL
NX_VIP_GetFieldStatus( U32 ModuleIndex )
{
	const U16 LASTFIELD = 1U<<5;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (__g_pRegister[ModuleIndex]->VIP_SYNCCTRL & LASTFIELD) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get current status of the internal HSYNC signal.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	CTRUE	indicates the internal HSYNC is in activate status.\n
 *			CFALSE	indicate the internal HSYNC is in inactivate status.
 *	@remark	The internal HSYNC signal is generated by internal sync generator
 *			for embedded sync. Therefore In case of external sync mode, the
 *			return value of this function is meaningless.
 *	@see	NX_VIP_GetVSyncStatus
 */
CBOOL
NX_VIP_GetHSyncStatus( U32 ModuleIndex )
{
	const U16 CURHSYNC = 1U<<1;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (__g_pRegister[ModuleIndex]->VIP_SYNCMON & CURHSYNC) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get current status of the internal VSYNC signal.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	CTRUE	indicates the internal VSYNC is in activate status.\n
 *			CFALSE	indicate the internal VSYNC is in inactivate status.
 *	@remark	The internal VSYNC signal is generated by internal sync generator
 *			for embedded sync. Therefore In case of external sync mode, the
 *			return value of this function is meaningless.
 *	@see	NX_VIP_GetHSyncStatus
 */
CBOOL
NX_VIP_GetVSyncStatus( U32 ModuleIndex )
{
	const U16 CURVSYNC = 1U<<0;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (__g_pRegister[ModuleIndex]->VIP_SYNCMON & CURVSYNC) ? CTRUE : CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Specifies the FIFO reset mode.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	FIFOReset	Specifies the FIFO reset mode.
 *	@return		None.
 *	@see		NX_VIP_GetFIFOResetMode, NX_VIP_GetFIFOStatus, NX_VIP_ResetFIFO
 */
void
NX_VIP_SetFIFOResetMode
(
	U32 				ModuleIndex,
	NX_VIP_FIFORESET	FIFOReset
)
{
	const U32 RESETFIFOSEL_POS = 1UL;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 4 > FIFOReset );

	pRegister = __g_pRegister[ModuleIndex];

//	pRegister->VIP_FIFOCTRL = (U16)(FIFOReset<<RESETFIFOSEL_POS);
	WriteIO32(&pRegister->VIP_FIFOCTRL, (U16)(FIFOReset<<RESETFIFOSEL_POS));
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get the FIFO reset mode.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	the current FIFO reset mode..
 *	@see	NX_VIP_SetFIFOResetMode, NX_VIP_GetFIFOStatus, NX_VIP_ResetFIFO
 */
NX_VIP_FIFORESET
NX_VIP_GetFIFOResetMode( U32 ModuleIndex )
{
	const U16 RESETFIFOSEL_POS	= 1;
	const U16 RESETFIFOSEL_MASK = 3U<<RESETFIFOSEL_POS;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return (NX_VIP_FIFORESET)((pRegister->VIP_FIFOCTRL & RESETFIFOSEL_MASK)>>RESETFIFOSEL_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get current FIFO status.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return FIFO status which consists of ORed NX_VIP_FSTATUS combination.
 *	@see	NX_VIP_SetFIFOResetMode, NX_VIP_GetFIFOResetMode, NX_VIP_ResetFIFO
 */
U32
NX_VIP_GetFIFOStatus( U32 ModuleIndex )
{
	const U32 FIFOSTATUS_POS = 8UL;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (U32)(__g_pRegister[ModuleIndex]->VIP_FIFOCTRL >> FIFOSTATUS_POS);
}

//------------------------------------------------------------------------------
/**
 *	@brief	Reset the internal FIFO.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	None.
 *	@remark	This function has only affect when the FIFO reset mode is NX_VIP_FIFORESET_CPU or NX_VIP_FIFORESET_ALL.
 *	@see	NX_VIP_SetFIFOResetMode, NX_VIP_GetFIFOResetMode, NX_VIP_GetFIFOStatus
 */
void
NX_VIP_ResetFIFO( U32 ModuleIndex )
{
	const U16 RESETFIFO = 1U<<0;
	register U16 temp;

	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	temp 	  = pRegister->VIP_FIFOCTRL;

//	pRegister->VIP_FIFOCTRL = (U16)(temp | RESETFIFO);
	WriteIO32(&pRegister->VIP_FIFOCTRL, (U16)(temp | RESETFIFO));

//	pRegister->VIP_FIFOCTRL = (U16)temp;	// clear it manually.
	WriteIO32(&pRegister->VIP_FIFOCTRL, (U16)temp);
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get the number of total clocks in a horizontal active video.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	the number of total clocks in a horizontal active video.
 *	@see	NX_VIP_GetVerCount
 */
U32
NX_VIP_GetHorCount( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (U32)__g_pRegister[ModuleIndex]->VIP_HCOUNT;
}

//------------------------------------------------------------------------------
/**
 *	@brief	Get the number of total lines in a vertical active video.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@return	the number of total lines in a vertical active video.
 *	@see	NX_VIP_GetHorCount
 */
U32
NX_VIP_GetVerCount( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	return (U32)__g_pRegister[ModuleIndex]->VIP_VCOUNT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a region to crop the input image by the Clipper.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	Left	Specifies the x-coordinate, in pixels, of the upper-left corner of the clip region in the input image, 0 ~ 4095.
 *	@param[in]	Top		Specifies the y-coordinate, in lines, of the upper-left corner of the clip region in the input image, 0 ~ 2047.
 *	@param[in]	Right	Specifies the x-coordinate, in pixels, of the lower-right corner of the clip region in the input image, 0 ~ 4095.
 *	@param[in]	Bottom	Specifies the y-coordinate, in lines, of the lower-right corner of the clip region in the input image, 0 ~ 2047.
 *	@return		None.
 *	@remark		In case of interlace scan mode, The input image is a field data not
 *				a frame data. Therefore you should set the y-coordinate carefully.
 *				The clipping width (Right - Left) must be a multiple of 16 and
 *				the clipping height (Bottom - Top) must be a even number.
 *	@see		NX_VIP_GetClipRegion
 */
void
NX_VIP_SetClipRegion
(
	U32 ModuleIndex,
	U32	Left,
	U32	Top,
	U32	Right,
	U32	Bottom
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 32768 > Left );
	NX_ASSERT( 32768 > Top );
	NX_ASSERT( 32768 > Right );
	NX_ASSERT( 32768 > Bottom );
	NX_ASSERT( Left < Right );
	NX_ASSERT( Top < Bottom );
	NX_ASSERT( 0 == ((Right-Left)%32) );
	NX_ASSERT( 0 == ((Bottom-Top)&1) );

	pRegister = __g_pRegister[ModuleIndex];

//	pRegister->CLIP_LEFT	= (U16)Left;
	WriteIO32(&pRegister->CLIP_LEFT, (U16)Left);

//	pRegister->CLIP_RIGHT	= (U16)Right;
	WriteIO32(&pRegister->CLIP_RIGHT, (U16)Right);

//	pRegister->CLIP_TOP		= (U16)Top;
	WriteIO32(&pRegister->CLIP_TOP, (U16)Top);

//	pRegister->CLIP_BOTTOM	= (U16)Bottom;
	WriteIO32(&pRegister->CLIP_BOTTOM, (U16)Bottom);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a region to crop the input image by the Clipper.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pLeft		Indicates the x-coordinate, in pixels, of the upper-left corner of the clip region in the input image.
 *	@param[out] pTop		Indicates the y-coordinate, in lines, of the upper-left corner of the clip region in the input image.
 *	@param[out] pRight		Indicates the x-coordinate, in pixels, of the lower-right corner of the clip region in the input image.
 *	@param[out] pBottom		Indicates the y-coordinate, in lines, of the lower-right corner of the clip region in the input image.
 *	@return		None.
 *	@remark		In case of interlace scan mode, The input image is a field data not
 *				a frame data.\n
 *				Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetClipRegion
 */
void
NX_VIP_GetClipRegion
(
	U32  ModuleIndex,
	U32	*pLeft,
	U32	*pTop,
	U32	*pRight,
	U32	*pBottom
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( CNULL != pLeft )	*pLeft		= (U32)pRegister->CLIP_LEFT;
	if( CNULL != pTop )		*pTop		= (U32)pRegister->CLIP_TOP;
	if( CNULL != pRight )	*pRight		= (U32)pRegister->CLIP_RIGHT;
	if( CNULL != pBottom )	*pBottom	= (U32)pRegister->CLIP_BOTTOM;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the image size to scale-down the clipped image by the Decimator.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	SrcWidth	Specifies the width of the clip region, 0 ~ 4095.
 *	@param[in]	SrcHeight	Specifies the height of the clip region, 0 ~ 2047.
 *	@param[in]	DstWidth	Specifies the width of the output image of the decimator.
 *							This argument must be less than or equal to SrcWidth,
 *							and a multiple of 16.
 *	@param[in]	DstHeight	Specifies the height of the output image of the decimator.
 *							This argument must be less than or equal to SrcHeight,
 *							and a even number.
 *	@return		None.
 *	@remark		The input image data transferred to the Decimator via the Clipper.
 *				Thererfore the source image of the Decimator is the result image of
 *				the Clipper. and the Decimator can only handle the even field data
 *				in the interlace scan mode.
 *	@see		NX_VIP_GetDecimation
 */

/* psw0523 add for GetDecimationSource() */
static U32 DeciSrcWidth[2];
static U32 DeciSrcHeight[2];
void
NX_VIP_SetDecimation
(
	U32 ModuleIndex,
	U32	SrcWidth,
	U32	SrcHeight,
	U32	DstWidth,
	U32	DstHeight
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 32768 > SrcWidth );
	NX_ASSERT( 32768 > SrcHeight );
	NX_ASSERT( DstWidth <= SrcWidth );
	NX_ASSERT( DstHeight <= SrcHeight );
	NX_ASSERT( 0 == (DstWidth%32) );
	NX_ASSERT( 0 == (DstHeight&1) );

	pRegister = __g_pRegister[ModuleIndex];

//	pRegister->DECI_TARGETW	= (U16)DstWidth;
	WriteIO32(&pRegister->DECI_TARGETW, (U16)DstWidth);

//	pRegister->DECI_TARGETH	= (U16)DstHeight;
	WriteIO32(&pRegister->DECI_TARGETH, (U16)DstHeight);

//	pRegister->DECI_DELTAW	= (U16)(SrcWidth - DstWidth);
	WriteIO32(&pRegister->DECI_DELTAW, (U16)(SrcWidth - DstWidth));

//	pRegister->DECI_DELTAH	= (U16)(SrcHeight - DstHeight);
	WriteIO32(&pRegister->DECI_DELTAH, (U16)(SrcHeight - DstHeight));

//	pRegister->DECI_CLEARW	= (S16)((DstWidth<<1) - SrcWidth);
	WriteIO32((volatile U16 *)&pRegister->DECI_CLEARW, (S16)((DstWidth<<1) - SrcWidth));

//	pRegister->DECI_CLEARH	= (S16)((DstHeight<<1) - SrcHeight);
	WriteIO32((volatile U16 *)&pRegister->DECI_CLEARH, (S16)((DstHeight<<1) - SrcHeight));

	DeciSrcWidth[ModuleIndex] = SrcWidth;
	DeciSrcHeight[ModuleIndex] = SrcHeight;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the paramters for the decimation.
 *  @param[in]  ModuleIndex		An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pDstWidth		Indicates the width of the output image of the decimator.
 *	@param[out] pDstHeight		Indicates the height of the output image of the decimator.
 *	@param[out] pDeltaWidth		Indicates the difference between clipped image width and destination image width of decimator.\n
 *								-> DeltaWidth = the clipped image width - DstWidth
 *	@param[out] pDeltaHeight	Indicates the difference between clipped image height and destination image height of decimator.\n
 *								-> DeltaHeight = the clipped image height - DstHeight
 *	@param[out] pClearWidth		Indicates the difference between destination image width and DeltaWidth of decimator.\n
 *								-> ClearWidth = DstWidth - DeltaWidth
 *	@param[out] pClearHeight	Indicates the difference between destination image height and DeltaHeight of decimator.\n
 *								-> ClearHeight = DstHeight - DeltaHeight
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetDecimation
 */
void
NX_VIP_GetDecimation
(
	U32  ModuleIndex,
	U32	*pDstWidth,
	U32	*pDstHeight,
	U32	*pDeltaWidth,
	U32	*pDeltaHeight,
	S32	*pClearWidth,
	S32 *pClearHeight
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( CNULL != pDstWidth )	*pDstWidth		= (U32)pRegister->DECI_TARGETW;
	if( CNULL != pDstHeight )	*pDstHeight		= (U32)pRegister->DECI_TARGETH;
	if( CNULL != pDeltaWidth )	*pDeltaWidth	= (U32)pRegister->DECI_DELTAW;
	if( CNULL != pDeltaHeight )	*pDeltaHeight	= (U32)pRegister->DECI_DELTAH;
	if( CNULL != pClearWidth )	*pClearWidth	= (S32)((U32)pRegister->DECI_CLEARW | ((pRegister->DECI_CLEARW & (1<<12)) ? 0xFFFFE000 : 0));
	if( CNULL != pClearHeight )	*pClearHeight	= (S32)((U32)pRegister->DECI_CLEARH | ((pRegister->DECI_CLEARH & (1<<11)) ? 0xFFFFF000 : 0));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the data Format, rotation and flip for the output image of the Clipper.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	Format		Specifies the data Format of the Clipper.
 *	@param[in]	bRotation	Set it to CTRUE to rotate the image clockwise by 90 degree.
 *							This argument must be CFALSE when Format is not NX_VIP_FORMAT_YUYV.
 *	@param[in]	bHFlip		Set it to CTRUE to flip the image horizontally.
 *							This argument is only valid when Format is not NX_VIP_FORMAT_YUYV.
 *	@param[in]	bVFlip		Set it to CTRUE to flip the image vertically.
 *							This argument is only valid when Format is not NX_VIP_FORMAT_YUYV.
 *	@return		None.
 *	@see		NX_VIP_GetClipperFormat
 */
void
NX_VIP_SetClipperFormat
(
	U32  			ModuleIndex,
	NX_VIP_FORMAT	Format
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 4 > Format );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->CLIP_FORMAT, (U16)Format);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the data Format, rotation and flip for the output image of the Clipper.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pFormat		Indicates the data Format of the Clipper.
 *	@param[out] pbRotation		Indicates whether the image is rotated clockwise by 90 degree.
 *								CTRUE = 90' Rotation, CFALSE = No Rotation
 *	@param[out] pbHFlip			Indicates whether to flip an image horizontally or not.
 *								This argument is only valid when Format is not Format_YUYV.\n
 *								CTRUE = Horizontal Flip, CFALSE = No Horizontal Flip
 *	@param[out] pbVFlip			Indicates whether to flip an image vertically or not.
 *								This argument is only valid when Format is not Format_YUYV.\n
 *								CTRUE = Vertical Flip, CFALSE = No Vertical Flip
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetClipperFormat
 */
void
NX_VIP_GetClipperFormat
(
	U32				 ModuleIndex,
	NX_VIP_FORMAT	*pFormat
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( CNULL != pFormat )
	{
		*pFormat = (NX_VIP_FORMAT)pRegister->CLIP_FORMAT;
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the data Format, rotation and flip for the output image of the Decimator.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	Format		Specifies the data Format of the Decimator. This argument must not be Format_YUYV;
 *	@param[in]	bRotation	Set it to CTRUE to rotate the image clockwise by 90 degree.
 *	@param[in]	bHFlip		Set it to CTRUE to flip the image horizontally.
 *	@param[in]	bVFlip		Set it to CTRUE to flip the image vertically.
 *	@return		None.
 *	@see		NX_VIP_GetDecimatorFormat
 */
void
NX_VIP_SetDecimatorFormat
(
	U32				ModuleIndex,
	NX_VIP_FORMAT	Format
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 3 > Format );

	pRegister = __g_pRegister[ModuleIndex];

//	pRegister->DECI_FORMAT	= (U16)Format;
	WriteIO32(&pRegister->DECI_FORMAT, (U16)Format);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get the data Format, rotation and flip for the output image of the Decimator.
 *  @param[in]  ModuleIndex		An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[out] pFormat			Indicates the data Format of the Decimator.
 *	@param[out] pbRotation		Indicates whether the image is rotated clockwise by 90 degree.\n
 *								CTRUE = 90' Rotation, CFALSE = No Rotation
 *	@param[out] pbHFlip			Indicates whether to flip an image horizontally or not.\n
 *								CTRUE = Horizontal Flip, CFALSE = No Horizontal Flip
 *	@param[out] pbVFlip			Indicates whether to flip an image vertically or not.\n
 *								CTRUE = Vertical Flip, CFALSE = No Vertical Flip
 *	@return		None.
 *	@remark		Arguments which does not required can be CNULL.
 *	@see		NX_VIP_SetDecimatorFormat
 */
void
NX_VIP_GetDecimatorFormat
(
	U32				 ModuleIndex,
	NX_VIP_FORMAT	*pFormat
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( CNULL != pFormat )		*pFormat = (NX_VIP_FORMAT)pRegister->DECI_FORMAT;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set the base address to store the output image of the Clipper.
 *  @param[in]  ModuleIndex	An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	Format		Specifies the data Format of the Clipper.\n
 *							It must not be Format_YUYV.
 *	@param[in]	Width		Specifies the width of the clip region.
 *	@param[in]	Height		Specifies the height of the clip region.
 *							It must be a height of the frame image even if the scan mode is interlace.
 *	@param[in]	LuAddr		Specifies the base address of the upper-left corner of Y data in a segment.
 *	@param[in]	CbAddr		Specifies the base address of the upper-left corner of Cb data in a segment.
 *	@param[in]	CrAddr		Specifies the base address of the upper-left corner of Cr data in a segment.
 *	@return		None.
 *	@remark		The argument Format, Width and Height, are only used to calculate
 *				image address. so you have to call the function NX_VIP_SetClipperFormat()
 *				and NX_VIP_SetClipRegion() to set these values.\n
 *				This function is for YUV 420, 422, 444 not YUYV. You have to call
 *				the function NX_VIP_SetClipperAddrYUYV for YUYV Format.\n
 *				The Format of LuAddr, CbAddr, CrAddr is as follows,
 *				- [31:24] : Specifies the index of the segment.
 *				- [23:12] : Sepcifies the y-coordinate in the segment.
 *				- [11: 0] : Specifies the x-coordinate in the segment.
 *				.
 *				The Clipper stores a frame image even if the scan mode is interlace
 *				scan mode. Therefore you have to allocate and set memory pool which
 *				can store a frame image.\n
 *				Memory address must align by 8.\n
 */
void
NX_VIP_SetClipperAddr
(
	U32				ModuleIndex,
	NX_VIP_FORMAT	Format,
	U32				Width,
	U32				Height,
	U32				LuAddr,
	U32				CbAddr,
	U32				CrAddr,
	U32				StrideY,
	U32				StrideCbCr
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 3 > Format );
	NX_ASSERT( 0 == (LuAddr%16) );
	NX_ASSERT( 0 == (CrAddr%16) );
	NX_ASSERT( 0 == (CbAddr%16) );
	NX_ASSERT( 0 == (StrideY%16) );
	NX_ASSERT( 0 == (StrideCbCr%16) );
	NX_ASSERT( 0 == (Width%32) );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->CLIP_LUADDR  , LuAddr     );
	WriteIO32(&pRegister->CLIP_LUSTRIDE, StrideY    );
	WriteIO32(&pRegister->CLIP_CRADDR  , CrAddr     );
	WriteIO32(&pRegister->CLIP_CRSTRIDE, StrideCbCr );
	WriteIO32(&pRegister->CLIP_CBADDR  , CbAddr     );
	WriteIO32(&pRegister->CLIP_CBSTRIDE, StrideCbCr );

}



//------------------------------------------------------------------------------
/**
 *	@brief		Set the base address to store the output image of the Decimator.
 *  @param[in]  ModuleIndex		An index of module ( 0 : VIP 0, 1 : VIP 1 ).
 *	@param[in]	NX_VIP_FORMAT	Specifies the data NX_VIP_FORMAT of the Decimator.\n
 *								It must not be NX_VIP_FORMAT_YUYV.
 *	@param[in]	Width		Specifies the width of the target image of the Decimator.
 *	@param[in]	Height		Specifies the height of the target image of the Decimator.
 *	@param[in]	LuAddr		Specifies the base address of the upper-left corner of Y data in a segment.
 *	@param[in]	CbAddr		Specifies the base address of the upper-left corner of Cb data in a segment.
 *	@param[in]	CrAddr		Specifies the base address of the upper-left corner of Cr data in a segment.
 *	@return		None.
 *	@remark		The argument NX_VIP_FORMAT, Width and Height, are only used to calculate
 *				image address. so you have to call the function SetDecimatorNX_VIP_FORMAT()
 *				and SetDecimation() to set these values.\n
 *				The NX_VIP_FORMAT of LuAddr, CbAddr, CrAddr is as follows,
 *				- [31:24] : Specifies the index of the segment.
 *				- [23:12] : Sepcifies the y-coordinate in the segment.
 *				- [11: 0] : Specifies the x-coordinate in the segment.
 *				.
 *				The Decimator can only store the even field data when the scan mode
 *				is interlace.
 */
void
NX_VIP_SetDecimatorAddr
(
	U32  			ModuleIndex,
	NX_VIP_FORMAT	NX_VIP_FORMAT,
	U32				Width,
	U32				Height,
	U32				LuAddr,
	U32				CbAddr,
	U32				CrAddr,
	U32				YStride,
	U32				CBCRStride
)
{
	register NX_VIP_RegisterSet	*pRegister;

	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	NX_ASSERT( 3 > NX_VIP_FORMAT );

	NX_ASSERT( 0 == (LuAddr%16) );
	NX_ASSERT( 0 == (CbAddr%16) );
	NX_ASSERT( 0 == (CrAddr%16) );
	NX_ASSERT( 0 == (Width%32) );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->DECI_LUADDR  , LuAddr     );
//	WriteIO32(&pRegister->DECI_LUSTRIDE, Width      );
	WriteIO32(&pRegister->DECI_LUSTRIDE, YStride      );

	if( NX_VIP_FORMAT_420 == NX_VIP_FORMAT )
	{
		NX_ASSERT( 0 == (Width&1) );
		NX_ASSERT( 0 == (Height&1) );
		Width	>>= 1;
		Height	>>= 1;
	}
	else if( NX_VIP_FORMAT_422 == NX_VIP_FORMAT )
	{
		NX_ASSERT( 0 == (Width&1) );
		Width	>>= 1;
	}

	WriteIO32(&pRegister->DECI_CRADDR  , CrAddr     );
	/*WriteIO32(&pRegister->DECI_CRSTRIDE, Width      );*/
	WriteIO32(&pRegister->DECI_CRSTRIDE, CBCRStride      );
	WriteIO32(&pRegister->DECI_CBADDR  , CbAddr     );
	/*WriteIO32(&pRegister->DECI_CBSTRIDE, Width      );*/
	WriteIO32(&pRegister->DECI_CBSTRIDE, CBCRStride      );
}


CBOOL  NX_VIP_SmokeTest ( U32 ModuleIndex )
{
	register NX_VIP_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_VIP_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	// reset value reading
	if( 0x00000200 != pRegister->VIP_FIFOCTRL ){ return CFALSE; }
	if( 0x00000003 != pRegister->VIP_SYNCMON  ){ return CFALSE; }

	// write data
	WriteIO32(&pRegister->VIP_IMGWIDTH, 0xC0DEBEEF );

	// read data, check that reserved bits are reserved.
	if( 0x00003EEF != pRegister->VIP_IMGWIDTH ){ return CFALSE; }

	return CTRUE;
}

/* psw0523 add */
void NX_VIP_GetDeciSource(U32 ModuleIndex, U32 *pSrcWidth, U32 *pSrcHeight)
{
	if (pSrcWidth)
		*pSrcWidth = DeciSrcWidth[ModuleIndex];
	if (pSrcHeight)
		*pSrcHeight = DeciSrcHeight[ModuleIndex];
}

