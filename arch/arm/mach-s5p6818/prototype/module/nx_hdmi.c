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
//	File		: nx_hdmi.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_hdmi.h"

#ifdef NUMBER_OF_DISPLAYTOP_MODULE
#include "nx_displaytop.h"
#endif

// register를 만들지 않고 define을 통해 처리한다.
//static	NX_HDMI_RegisterSet *__g_pRegister[NUMBER_OF_HDMI_MODULE];

#ifdef	CONFIG_ARM64 
	U32*	HDMI_BaseAddr;
#else
	U32	HDMI_BaseAddr;
#endif

//U32  HDMI_TXLINK_BaseAddr;
//U32  HDMI_CEC_BaseAddr;
//U32  HDMI_PHY_BaseAddr;

//#define NX_HDMI_PHY_INIT_REG_TEST


//------------------------------------------------------------------------------
//
//	HDMI Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *  @brief  It is a sample function
 */
U32  NX_HDMI_GetReg( U32 ModuleIndex, U32 Offset )
{
#ifdef CONFIG_ARM64
  U32* RegAddr;
#else
  U32 RegAddr;
#endif

  U32 regvalue;

  NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );

#ifdef CONFIG_ARM64
  RegAddr = HDMI_BaseAddr + (Offset/sizeof(U32));
  regvalue = ReadIO32( (U32*)RegAddr );
#else
  RegAddr = HDMI_BaseAddr + Offset;
  regvalue = ReadIO32( (U32*)RegAddr );
#endif

  return regvalue;
}

void NX_HDMI_SetReg( U32 ModuleIndex, U32 Offset, U32 regvalue )
{
#ifdef CONFIG_ARM64
  S64 OffsetNew = (S64)((S32)Offset);
  U32* RegAddr;
#else
  U32 RegAddr;
#endif

  NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );

#ifdef CONFIG_ARM64 
  RegAddr = HDMI_BaseAddr + (OffsetNew/sizeof(U32));
#else
  RegAddr = HDMI_BaseAddr + Offset;
#endif

  WriteIO32((U32*)RegAddr, regvalue);
}

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicate that Initialize is successed.
 *			CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_HDMI_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. 
 *				It is equal to NUMBER_OF_HDMI_MODULE in <nx_chip.h>.
 */
U32		NX_HDMI_GetNumberOfModule( void )
{
	return NUMBER_OF_HDMI_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_HDMI_GetSizeOfRegisterSet( void )
{
	return -1; // sizeof( NX_HDMI_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */

void	NX_HDMI_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );

#ifdef CONFIG_ARM64
	HDMI_BaseAddr = (U32*)BaseAddress;
#else
	HDMI_BaseAddr = (U32)BaseAddress;
#endif
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */

void*	NX_HDMI_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );

	return (U32*)HDMI_BaseAddr;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_HDMI?_MODULE in <nx_chip.h>.
 */
U32		NX_HDMI_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_HDMI_MODULE] =
	{
		PHY_BASEADDR_LIST( HDMI )
	};
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. 
 *				CFALSE	indicate that Initialize is failed.,
 */
CBOOL	NX_HDMI_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicate that Deinitialize is successed. 
 *				CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_HDMI_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicate that Module is Busy. 
 *				CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_HDMI_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}



//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_HDMI?_MODULE_i_nRST in <nx_chip.h>.
 */
U32 NX_HDMI_GetResetNumber ( U32 ModuleIndex, U32 ChannelNumber )
{
	const U32 ResetNumber[5][NUMBER_OF_HDMI_MODULE] =
	{
		{ RESETINDEX_LIST( HDMI, i_nRST ) },
		{ RESETINDEX_LIST( HDMI, i_nRST_VIDEO ) },
		{ RESETINDEX_LIST( HDMI, i_nRST_SPDIF ) },
		{ RESETINDEX_LIST( HDMI, i_nRST_TMDS ) },
		{ RESETINDEX_LIST( HDMI, i_nRST_PHY ) }
	};
	NX_CASSERT( NUMBER_OF_HDMI_MODULE == (sizeof(ResetNumber[0])/sizeof(ResetNumber[0][0])) );
	NX_ASSERT( (sizeof(ResetNumber)/sizeof(ResetNumber[0])) == ChannelNumber );
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
	return	ResetNumber[ChannelNumber][ModuleIndex];
}


////------------------------------------------------------------------------------
//// Interrupt Interface
////------------------------------------------------------------------------------
///**
// *	@brief		Get a interrupt number for the interrupt controller.
// *	@param[in]	ModuleIndex		an index of module.
// *	@return		A interrupt number.
// *				It is equal to INTNUM_OF_HDMI?_MODULE in <nx_chip.h>.
// */
U32 	NX_HDMI_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( HDMI ) };
    NX_CASSERT( NUMBER_OF_HDMI_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_HDMI0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

////------------------------------------------------------------------------------
///**
// *	@brief		Set a specified interrupt to be enabled or disabled.
// *	@param[in]	ModuleIndex		an index of module.
// *	@param[in]	IntNum	a interrupt Number .
// *						refer to NX_HDMI_INTCH_xxx in <nx_hdmi.h>
// *	@param[in]	Enable	Set as CTRUE to enable a specified interrupt. 
// *						Set as CFALSE to disable a specified interrupt.
// *	@return		None.
// */
//void	NX_HDMI_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
//{
//	register NX_HDMI_RegisterSet* pRegister;
//	register U32	regvalue;
//
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( (0==Enable) || (1==Enable) );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	regvalue  = pRegister->INTCTRL;
//
//	regvalue &=	~( 1UL << IntNum );
//	regvalue |= (U32)Enable << IntNum;
//
//	WriteIO32(&pRegister->INTCTRL, regvalue);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Indicates whether a specified interrupt is enabled or disabled.
// *	@param[in]	ModuleIndex		an index of module.
// *	@param[in]	IntNum	a interrupt Number.
// *						refer to NX_HDMI_INTCH_xxx in <nx_hdmi.h>
// *	@return		CTRUE	indicates that a specified interrupt is enabled. 
// *				CFALSE	indicates that a specified interrupt is disabled.
//
// */
//CBOOL	NX_HDMI_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
//{
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	return (CBOOL)( ( __g_pRegister[ModuleIndex]->INTCTRL >> IntNum ) & 0x01 );
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Indicates whether a specified interrupt is pended or not
// *	@param[in]	ModuleIndex		an index of module.
// *	@param[in]	IntNum	a interrupt Number.
// *						refer to NX_HDMI_INTCH_xxx in <nx_hdmi.h>
// *	@return		CTRUE	indicates that a specified interrupt is pended. 
// *				CFALSE	indicates that a specified interrupt is not pended.
//
// */
//CBOOL	NX_HDMI_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
//{
//	register NX_HDMI_RegisterSet* pRegister;
//	register U32	regvalue;
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	pRegister = __g_pRegister[ModuleIndex];
//	regvalue  = pRegister->INTCTRL;
//	regvalue &= pRegister->INTPEND;
//	return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Clear a pending state of specified interrupt.
// *	@param[in]	ModuleIndex		an index of module.
// *	@param[in]	IntNum	a interrupt number.
// *						refer to NX_HDMI_INTCH_xxx in <nx_hdmi.h>
// *	@return		None.
//
// */
//void	NX_HDMI_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
//{
//	register NX_HDMI_RegisterSet* pRegister;
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	pRegister = __g_pRegister[ModuleIndex];
//	WriteIO32(&pRegister->INTPEND, 1UL << IntNum);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Set all interrupts to be enabled or disabled.
// *	@param[in]	ModuleIndex		an index of module.
// *	@param[in]	Enable	Set as CTRUE to enable all interrupts. 
// *						Set as CFALSE to disable all interrupts.
// *	@return		None.
//
// */
//void	NX_HDMI_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
//{
//	register NX_HDMI_RegisterSet* pRegister;
//	register U32	regvalue;
//
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( (0==Enable) || (1==Enable) );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	regvalue  = Enable ? 0xFFFFFFFF : 0 ;
//
//	WriteIO32(&pRegister->INTCTRL, regvalue);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Indicates whether some of interrupts are enabled or not.
// *	@param[in]	ModuleIndex		an index of module.
// *	@return		CTRUE	indicates that one or more interrupts are enabled. 
// *				CFALSE	indicates that all interrupts are disabled.
//
// */
//CBOOL	NX_HDMI_GetInterruptEnableAll( U32 ModuleIndex )
//{
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->INTCTRL ));
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Indicates whether some of interrupts are pended or not.
// *	@param[in]	ModuleIndex		an index of module.
// *	@return		CTRUE	indicates that one or more interrupts are pended. 
// *				CFALSE	indicates that no interrupt is pended.
//
// */
//CBOOL	NX_HDMI_GetInterruptPendingAll( U32 ModuleIndex )
//{
//	register NX_HDMI_RegisterSet* pRegister;
//	register U32	regvalue;
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	pRegister = __g_pRegister[ModuleIndex];
//	regvalue  = pRegister->INTCTRL;
//	regvalue &= pRegister->INTPEND;
//	return (CBOOL)( 0 != ( regvalue ) );
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Clear pending state of all interrupts.
// *	@param[in]	ModuleIndex		an index of module.
// *	@return		None.
//
// */
//void	NX_HDMI_ClearInterruptPendingAll( U32 ModuleIndex )
//{
//	register NX_HDMI_RegisterSet* pRegister;
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	pRegister = __g_pRegister[ModuleIndex];
//	WriteIO32(&pRegister->INTPEND, 0xFFFFFFFF);	// just write operation make pending clear
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Get a interrupt number which has the most prority of pended interrupts.
// *	@param[in]	ModuleIndex		an index of module.
// *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
// *				refer to NX_HDMI_INTCH_xxx in <nx_hdmi.h>
//
// */
//S32		NX_HDMI_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
//{
//	int i;
//	register NX_HDMI_RegisterSet* pRegister;
//	register U32	regvalue;
//	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	pRegister = __g_pRegister[ModuleIndex];
//	regvalue  = pRegister->INTCTRL;
//	regvalue &= pRegister->INTPEND;
//	if( 0!=regvalue )
//	{
//		for( i=0; i<32; i++ )
//		{
//			if( regvalue & 1UL )
//			{
//				return i;
//			}
//			regvalue>>=1;
//		}
//	}
//	return -1;
//}


//-------------
// Init Reg Test
//-------------
#ifdef NX_HDMI_PHY_INIT_REG_TEST
#define NX_HDMI_CheckInitReg_MACRO( OFFSET_DEF, INIT_VALUE ) if( INIT_VALUE != (regvalue = NX_HDMI_GetReg( ModuleIndex, OFFSET_DEF                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value ( %x ) = %x(Golden : %x)", OFFSET_DEF, regvalue, INIT_VALUE); }TotalCount+=1;

CBOOL NX_HDMI_PHY_InitRegTest( U32 ModuleIndex )
{
	U32 IsFail = 0;
	U32 TotalCount = 0;
	U32 regvalue;
	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );

	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_REG00                              , 0x01 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg04                              , 0x91 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg08                              , 0x1f ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg0C                              , 0x10 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg10                              , 0x40 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg14                              , 0x5b ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg18                              , 0xef ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg1C                              , 0x08 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg20                              , 0x81 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg24                              , 0x28 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg28                              , 0xb9 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg2C                              , 0xd8 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg30                              , 0x45 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg34                              , 0xa0 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg38                              , 0xac ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg3C                              , 0x90 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg40                              , 0x08 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg44                              , 0x80 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg48                              , 0x01 ) ; // ! 0x0a
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg4C                              , 0x84 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg50                              , 0x05 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg54                              , 0x21 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg58                              , 0x24 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg5C                              , 0x86 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg60                              , 0x54 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg64                              , 0xa6 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg68                              , 0x24 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg6C                              , 0x01 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg70                              , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg74                              , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg78                              , 0x01 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg7C                              , 0x80 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg80                              , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg84                              , 0x00 ) ;
	//NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg88                              , 0x11 ) ; // ! 0x12, 통합에선 0x13..
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg8C                              , 0x10 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_PHY_Reg90                              , 0x00 ) ;

	if( IsFail > 0 )
		{
			NX_CONSOLE_Printf("\n[DEBUG] HDMI PHY Reset Value Check. IsFail Count = %d, Total Count = %d\n", IsFail, TotalCount );
			return CFALSE;
		}
	else
		{
			return CTRUE;
		}
}




CBOOL NX_HDMI_InitRegTest( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_HDMI_MODULE > ModuleIndex );
	U32 IsFail = 0;
	U32 TotalCount = 0;

	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AESKEY_VALID               ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HPD                        ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_INTC_FLAG_1                ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_PHY_STATUS_0               ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_PHY_STATUS_CMU             ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_PHY_STATUS_PLL             ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_PHY_CON_0                  ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HPD_CTRL                   ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HPD_STATUS                 ) ) {IsFail+=1;}TotalCount+=1;
	//if( 0x00 != NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HPD_TH_x                   ) ) {IsFail+=1;}TotalCount+=1;

	U32 regvalue;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_INTC_CON_0                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_INTC_CON_0                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_INTC_FLAG_0                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_INTC_FLAG_0               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_INTC_CON_1                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_INTC_CON_1                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_CON_0                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_CON_0                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_CON_1                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_CON_1                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_CON_2                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_CON_2                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_STATUS                     ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_STATUS                    ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_STATUS_EN                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_STATUS_EN                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_MODE_SEL                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_MODE_SEL                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ENC_EN                     ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ENC_EN                    ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0xEB != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_YMAX                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_YMAX                 ) = %x(Golden : 0xEB)", regvalue); }TotalCount+=1;
	if( 0x10 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_YMIN                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_YMIN                 ) = %x(Golden : 0x10)", regvalue); }TotalCount+=1;
	if( 0xF0 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_CMAX                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_CMAX                 ) = %x(Golden : 0xF0)", regvalue); }TotalCount+=1;
	if( 0x10 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDMI_CMIN                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDMI_CMIN                 ) = %x(Golden : 0x10)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_BLANK_0                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_BLANK_0                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_BLANK_1                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_BLANK_1                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V2_BLANK_0                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V2_BLANK_0                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V2_BLANK_1                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V2_BLANK_1                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V1_BLANK_0                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V1_BLANK_0                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V1_BLANK_1                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V1_BLANK_1                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_LINE_0                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_LINE_0                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_LINE_1                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_LINE_1                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_LINE_0                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_LINE_0                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_LINE_1                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_LINE_1                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HSYNC_POL                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HSYNC_POL                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VSYNC_POL                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VSYNC_POL                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_INT_PRO_MODE               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_INT_PRO_MODE              ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F0_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F0_0              ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F0_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F0_1              ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F1_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F1_0              ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F1_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F1_1              ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_SYNC_START_0             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_SYNC_START_0            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_SYNC_START_1             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_SYNC_START_1            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_SYNC_END_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_SYNC_END_0              ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_H_SYNC_END_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_H_SYNC_END_1              ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_BEF_2_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_BEF_2_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_BEF_2_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_BEF_2_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_BEF_1_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_BEF_1_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_BEF_1_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_BEF_1_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_2_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_2_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_2_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_2_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_1_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_1_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_1_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_1_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_0    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_0   ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_1    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_1   ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_0    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_0   ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_1    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_1   ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F2_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F2_0              ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F2_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F2_1              ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F3_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F3_0              ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F3_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F3_1              ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F4_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F4_0              ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F4_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F4_1              ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F5_0               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F5_0              ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_BLANK_F5_1               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_BLANK_F5_1              ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_3_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_3_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_3_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_3_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_4_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_4_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_4_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_4_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_5_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_5_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_5_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_5_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_6_0        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_6_0       ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_6_1        ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_6_1       ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_0    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_0   ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_1    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_1   ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_0    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_0   ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_1    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_1   ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_0    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_0   ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_1    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_1   ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_0    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_0   ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_1    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_1   ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE1_0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE1_0             ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE1_1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE1_1             ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE2_0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE2_0             ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE2_1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE2_1             ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE3_0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE3_0             ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE3_1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE3_1             ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE4_0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE4_0             ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE4_1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE4_1             ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE5_0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE5_0             ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE5_1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE5_1             ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0xff != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE6_0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE6_0             ) = %x(Golden : 0xff)", regvalue); }TotalCount+=1;
	if( 0x1f != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE6_1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE6_1             ) = %x(Golden : 0x1f)", regvalue); }TotalCount+=1;
	if( 0x04 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GCP_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GCP_CON                   ) = %x(Golden : 0x04)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GCP_BYTE1                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GCP_BYTE1                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GCP_BYTE2                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GCP_BYTE2                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GCP_BYTE3                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GCP_BYTE3                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ASP_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ASP_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ASP_SP_FLAT                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ASP_SP_FLAT               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x08 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ASP_CHCFG0                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ASP_CHCFG0                ) = %x(Golden : 0x08)", regvalue); }TotalCount+=1;
	if( 0x1A != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ASP_CHCFG1                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ASP_CHCFG1                ) = %x(Golden : 0x1A)", regvalue); }TotalCount+=1;
	if( 0x2C != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ASP_CHCFG2                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ASP_CHCFG2                ) = %x(Golden : 0x2C)", regvalue); }TotalCount+=1;
	if( 0x3E != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ASP_CHCFG3                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ASP_CHCFG3                ) = %x(Golden : 0x3E)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x01 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_MCTS0                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_MCTS0                 ) = %x(Golden : 0x01)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_MCTS1                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_MCTS1                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_MCTS2                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_MCTS2                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0xE8 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_N0                     ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_N0                    ) = %x(Golden : 0xE8)", regvalue); }TotalCount+=1;
	if( 0x03 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_N1                     ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_N1                    ) = %x(Golden : 0x03)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACR_N2                     ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACR_N2                    ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACP_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACP_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACP_TYPE                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACP_TYPE                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ACP_DATAx                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ACP_DATAx                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ISRC_CON                   ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ISRC_CON                  ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ISRC1_HEADER1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ISRC1_HEADER1             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ISRC1_DATAx                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ISRC1_DATAx               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_ISRC2_DATAx                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_ISRC2_DATAx               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AVI_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AVI_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AVI_HEADER0                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AVI_HEADER0               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AVI_HEADER1                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AVI_HEADER1               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AVI_HEADER2                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AVI_HEADER2               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AVI_CHECK_SUM              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AVI_CHECK_SUM             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AVI_BYTEx                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AVI_BYTEx                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AUI_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AUI_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AUI_HEADER0                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AUI_HEADER0               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AUI_HEADER1                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AUI_HEADER1               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AUI_HEADER2                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AUI_HEADER2               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AUI_CHECK_SUM              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AUI_CHECK_SUM             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_AUI_BYTEx                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_AUI_BYTEx                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_MPG_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_MPG_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_MPG_CHECK_SUM              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_MPG_CHECK_SUM             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_MPG_DATAx                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_MPG_DATAx                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_SPD_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_SPD_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_SPD_HEADER0                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_SPD_HEADER0               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_SPD_HEADER1                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_SPD_HEADER1               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_SPD_HEADER2                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_SPD_HEADER2               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_SPD_DATAx                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_SPD_DATAx                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GAMUT_CON                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GAMUT_CON                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GAMUT_HEADER0              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GAMUT_HEADER0             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GAMUT_HEADER1              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GAMUT_HEADER1             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GAMUT_HEADER2              ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GAMUT_HEADER2             ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_GAMUT_METADATAx            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_GAMUT_METADATAx           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VSI_CON                    ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VSI_CON                   ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VSI_HEADER0                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VSI_HEADER0               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VSI_HEADER1                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VSI_HEADER1               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VSI_HEADER2                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VSI_HEADER2               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VSI_DATAx                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VSI_DATAx                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_DC_CONTROL                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_DC_CONTROL                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VIDEO_PATTERN_GEN          ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VIDEO_PATTERN_GEN         ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0xFF != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_An_Seed_Sel                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_An_Seed_Sel               ) = %x(Golden : 0xFF)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_An_Seed_0                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_An_Seed_0                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_An_Seed_1                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_An_Seed_1                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_An_Seed_2                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_An_Seed_2                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_An_Seed_3                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_An_Seed_3                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_SHA1_x                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_SHA1_x               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_KSV_LIST_x            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_KSV_LIST_x           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x01 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_KSV_LIST_CON          ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_KSV_LIST_CON         ) = %x(Golden : 0x01)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_SHA_RESULT            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_SHA_RESULT           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_CTRL1                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_CTRL1                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_CTRL2                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_CTRL2                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_CHECK_RESULT          ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_CHECK_RESULT         ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_BKSV_x                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_BKSV_x               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_AKSV_x                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_AKSV_x               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_An_x                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_An_x                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_BCAPS                 ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_BCAPS                ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_BSTATUS_0             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_BSTATUS_0            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_BSTATUS_1             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_BSTATUS_1            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_Ri_0                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_Ri_0                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_Ri_1                  ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_Ri_1                 ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_I2C_INT               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_I2C_INT              ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_AN_INT                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_AN_INT               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_WATCHDOG_INT          ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_WATCGDOG_INT         ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_Ri_INT                ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_Ri_INT               ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x80 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_Ri_Compare_0          ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_Ri_Compare_0         ) = %x(Golden : 0x80)", regvalue); }TotalCount+=1;
	if( 0x7F != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_Ri_Compare_1          ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_Ri_Compare_1         ) = %x(Golden : 0x7F)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_HDCP_Frame_Count           ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_HDCP_Frame_Count          ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_RGB_ROUND_EN               ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_RGB_ROUND_EN              ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE_R_0             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE_R_0            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE_R_1             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE_R_1            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE_G_0             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE_G_0            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE_G_1             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE_G_1            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE_B_0             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE_B_0            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_VACT_SPACE_B_1             ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_VACT_SPACE_B_1            ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_BLUE_SCREEN_R_0            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_BLUE_SCREEN_R_0           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_BLUE_SCREEN_R_1            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_BLUE_SCREEN_R_1           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_BLUE_SCREEN_G_0            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_BLUE_SCREEN_G_0           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_BLUE_SCREEN_G_1            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_BLUE_SCREEN_G_1           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_BLUE_SCREEN_B_0            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_BLUE_SCREEN_B_0           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;
	if( 0x00 != (regvalue = NX_HDMI_GetReg( ModuleIndex, HDMI_LINK_BLUE_SCREEN_B_1            ) )) {IsFail+=1;NX_CONSOLE_Printf("\n[DEBUG] Wrong Init Value (HDMI_LINK_BLUE_SCREEN_B_1           ) = %x(Golden : 0x00)", regvalue); }TotalCount+=1;

	// Check 하고 확인. - define macro로 정의 .
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_AES_START                     , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_AES_DATA_SIZE_L               , 0x20 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_AES_DATA_SIZE_H               , 0x01 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_AES_DATA                      , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CLK_CTRL              , 0x02 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_OP_CTRL               , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_IRQ_MASK              , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_IRQ_STATUS            , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CONFIG_1              , 0x02 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CONFIG_2              , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_USER_VALUE_1          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_USER_VALUE_2          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_USER_VALUE_3          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_USER_VALUE_4          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CH_STATUS_0_1         , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CH_STATUS_0_2         , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CH_STATUS_0_3         , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CH_STATUS_0_4         , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_CH_STATUS_1           , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_FRAME_PERIOD_1        , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_FRAME_PERIOD_2        , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_Pc_INFO_1             , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_Pc_INFO_2             , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_Pd_INFO_1             , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_Pd_INFO_2             , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_DATA_BUF_0_1          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_DATA_BUF_0_2          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_DATA_BUF_0_3          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_USER_BUF_0            , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_DATA_BUF_1_1          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_DATA_BUF_1_2          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_DATA_BUF_1_3          , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_SPDIFIN_USER_BUF_1            , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CLK_CON                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CON_1                     , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CON_2                     , 0x16 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_PIN_SEL_0                 , 0x77 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_PIN_SEL_1                 , 0x77 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_PIN_SEL_2                 , 0x77 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_PIN_SEL_3                 , 0x07 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_DSD_CON                   , 0x02 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_MUX_CON                   , 0x60 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_CON                 , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_3                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_4                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_SH_0                , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_SH_1                , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_SH_2                , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_SH_3                , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH_ST_SH_4                , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_VD_DATA                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_MUX_CH                    , 0x03 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_MUX_CUV                   , 0x03 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_L_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_L_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_L_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_R_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_R_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_R_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH0_R_3                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_L_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_L_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_L_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_L_3                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_R_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_R_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_R_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH1_R_3                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_L_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_L_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_L_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_L_3                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_R_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_R_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH2_R_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_Ch2_R_3                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH3_L_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH3_L_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH3_L_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH3_R_0                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH3_R_1                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CH3_R_2                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_LINK_I2S_CUV_L_R                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_STATUS_0                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_STATUS_1                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_RX_STATUS_0                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_RX_STATUS_1                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_INTR_MASK                    , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_INTR_CLEAR                   , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_LOGIC_ADDR                   , 0x0F ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_DIVISOR_0                    , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_DIVISOR_1                    , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_DIVISOR_2                    , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_DIVISOR_3                    , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_CTRL                      , 0x50 ) ; // ! 0x10
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_BYTE_NUM                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_STATUS_2                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_STATUS_3                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_TX_BUFFER_x                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_RX_CTRL                      , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_RX_STATUS_2                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_RX_STATUS_3                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_RX_BUFFER_x                  , 0x00 ) ;
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_FILTER_CTRL                  , 0x01 ) ; // ! 0x01
	NX_HDMI_CheckInitReg_MACRO( HDMI_CEC_FILTER_TH                    , 0x03 ) ;



	if( IsFail > 0 )
		{
			NX_CONSOLE_Printf("\n[DEBUG] HDMI Reset Value Check. IsFail Count = %d, Total Count = %d\n", IsFail, TotalCount );
			return CFALSE;
		}
	else
		{
			return CTRUE;
		}
}

#endif

