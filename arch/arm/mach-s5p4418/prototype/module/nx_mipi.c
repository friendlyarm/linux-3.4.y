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
//	Module		:
//	File		: nx_mipi.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_mipi.h"

volatile U32 MIPI_CSIS_PKTDATA  [0x2000/4] ;
static	NX_MIPI_RegisterSet *__g_pRegister[NUMBER_OF_MIPI_MODULE];

NX_CASSERT( (&(((NX_MIPI_RegisterSet*)0)->DSIM_STATUS)) == (void*)0x0100 );
NX_CASSERT( (&(((NX_MIPI_RegisterSet*)0)->MIPI_CSIS_PKTDATA[0])) == (void*)0x2000 );

//------------------------------------------------------------------------------
//
//	MIPI Interface
//
//------------------------------------------------------------------------------
CBOOL  NX_MIPI_SmokeTest ( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	// reset value reading
	if( 0x000000FC != pRegister->CSIS_CONFIG_CH0 ){ return CFALSE; }
	if( 0xB337FFFF != pRegister->DSIM_INTMSK     ){ return CFALSE; }

	// write data
	WriteIO32(&pRegister->CSIS_DPHYCTRL, 0xDEADC0DE );
	WriteIO32(&pRegister->CSIS_CTRL2   , 0xFFFFFFFF );
	WriteIO32(&pRegister->DSIM_MSYNC   , 0xDEADC0DE );

	// read data, check that reserved bits are reserved.
	if( 0xDE80001E != pRegister->CSIS_DPHYCTRL   ){ return CFALSE; }
	//if( 0xEEE00011 != pRegister->CSIS_CTRL2      ){ return CFALSE; } // protection된 RTL
	//if( 0xEEE00010 != pRegister->CSIS_CTRL2      ){ return CFALSE; } // gate
	if( 0xEEE00010 != (pRegister->CSIS_CTRL2 &(~1))){ return CFALSE; }
	if( 0xDE80C0DE != pRegister->DSIM_MSYNC      ){ return CFALSE; }

	return CTRUE;
}

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
 *	@see	NX_MIPI_GetNumberOfModule
 */
CBOOL	NX_MIPI_Initialize( void )
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
 *				It is equal to NUMBER_OF_MIPI_MODULE in <nx_chip.h>.
 *	@see		NX_MIPI_Initialize
 */
U32		NX_MIPI_GetNumberOfModule( void )
{
	return NUMBER_OF_MIPI_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.

 */
U32		NX_MIPI_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_MIPI_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_MIPI_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_MIPI_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_MIPI_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_MIPI?_MODULE in <nx_chip.h>.
 */
U32		NX_MIPI_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_MIPI_MODULE] =
	{
		PHY_BASEADDR_LIST( MIPI )
	};
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. 
 *				CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_MIPI_OpenModule( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
    //{
    //	int i;
    //	//for( i=0; i<32; i++ ) pRegister->DSIM_PLLCTRL    = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PLLCTRL1   = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PLLCTRL2   = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PLLTMR     = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PHYCTRL    = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PHYCTRL1   = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PHYTIMING  = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PHYTIMING1 = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->DSIM_PHYTIMING2 = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->CSIS_DPHYCTRL   = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->CSIS_DPHYCTRL_0 = 1<<i;
	//	for( i=0; i<32; i++ ) pRegister->CSIS_DPHYCTRL_1 = 1<<i;
	//}
	//volatile U32 CSIS_DPHYCTRL  ; // 0x0004 R/W Specifies the D-PHY control register. 0x0000_0000 ==> {3'b0,S_HSSETTLECTL[4:0],S_CLKSETTLECTL[1:0],17'b0,S_ENABLE3,S_ENABLE2,S_ENABLE1,S_ENABLE0,S_ENABLECLK}
	//volatile U32 CSIS_DPHYCTRL_1; // 0x0024 R/W DPHY Analog Control register1         0x0000_0000 ==> {16'b0,S_DPHY_CTL[15:0]}
	//volatile U32 DSIM_PLLCTRL   ; // 0x014C R/W Specifies the PLL control register.   0x0000_0000 ==> { M_HSZEROCTL[3:0], M_BANDCTL[3:0], M_PLLEN, M_PRPRCTL[2:0], M_PLLCTL[18:0],1'b0 }
	//volatile U32 DSIM_PLLCTRL2  ; // 0x0154 R/W Specifies the PLL control register 2. 0x0000_0000 ==> M_DPHYCTL[27:0]
	//volatile U32 DSIM_PLLTMR    ; // 0x0158 R/W Specifies the PLL timer register.     0xFFFF_FFFF ==> {B_DPHYDTL[22:0], 2'b0, M_PRPRCTLCLK[2:0], 2'b0,M_DPDN_SWAP_CLK, M_DPDN_SWAP_DAT}

	pRegister->CSIS_DPHYCTRL_1 = 0; // S_DPHYCTL[15:0] D-PHY Slave Analog block characteristics control register. Default is 16'b0

	// 750Mhz (LN28LPP_MipiDphyCore1Gbps_Supplement.xls)
	//pRegister->DSIM_PLLCTRL2= (3<<5); // AFC_CODE(011) ???
	pRegister->CSIS_DPHYCTRL= (22 <<24);
	//pRegister->DSIM_PLLCTRL = (0xC<<24); // set M_BANDCTL 4'b1100 // @todo 삼성 검토요청


	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_MIPI_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
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
CBOOL	NX_MIPI_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_MIPI?_MODULE in <nx_chip.h>.
 */
U32 NX_MIPI_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( MIPI )
	};
	NX_CASSERT( NUMBER_OF_MIPI_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_MIPI?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_MIPI_GetResetNumber ( U32 ModuleIndex, U32 ChannelIndex )
{
	const U32 ResetNumber[][NUMBER_OF_MIPI_MODULE] =
	{
		{ RESETINDEX_LIST( MIPI, i_nRST ) },
		{ RESETINDEX_LIST( MIPI, i_DSI_I_PRESETn ) },
		{ RESETINDEX_LIST( MIPI, i_CSI_I_PRESETn ) },
		{ RESETINDEX_LIST( MIPI, i_PHY_S_RESETN ) },
		{ RESETINDEX_LIST( MIPI, i_PHY_M_RESETN ) }
	};
	NX_CASSERT( NUMBER_OF_MIPI_MODULE == (sizeof(ResetNumber[0])/sizeof(ResetNumber[0][0])) );
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	return	ResetNumber[ChannelIndex][ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
#define __NX_MIPI_VALID_CSI_INTMASK__ (~((1<<3)))
#define __NX_MIPI_VALID_DSI_INTMASK__ (~((1<<26)|(1<<23)|(1<<22)|(1<<19)))

/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_MIPI?_MODULE in <nx_chip.h>.
 */
U32 	NX_MIPI_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( MIPI ) };
    NX_CASSERT( NUMBER_OF_MIPI_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_MIPI0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_MIPI_INTCH_xxx in <nx_MIPI.h>
 *	@param[in]	Enable	 Set as CTRUE to enable a specified interrupt. 
 *						 Set as CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_MIPI_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( IntNum < 32 ) // CSI
	{
		NX_ASSERT( __NX_MIPI_VALID_CSI_INTMASK__ & ( 1UL << IntNum ) );
		regvalue  = pRegister->CSIS_INTMSK;
		regvalue &=	~( 1UL << IntNum );
		regvalue |= (U32)Enable << IntNum;
		WriteIO32(&pRegister->CSIS_INTMSK, regvalue);
	}
	else // DSI
	{
		NX_ASSERT( __NX_MIPI_VALID_DSI_INTMASK__ & ( 1UL << (IntNum-32) ) );
		regvalue  = pRegister->DSIM_INTMSK;
		regvalue &=	~( 1UL << (IntNum-32) );
		regvalue |= (U32)Enable << (IntNum-32);
		WriteIO32(&pRegister->DSIM_INTMSK, regvalue);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_MIPI_INTCH_xxx in <nx_MIPI.h>
 *	@return		 CTRUE	indicates that a specified interrupt is enabled. 
 *				 CFALSE	indicates that a specified interrupt is disabled.

 */
CBOOL	NX_MIPI_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	if( IntNum < 32 ) // CSI
	{
		NX_ASSERT( __NX_MIPI_VALID_CSI_INTMASK__ & ( 1UL << IntNum ) );
		return (CBOOL)( ( __g_pRegister[ModuleIndex]->CSIS_INTMSK >> IntNum ) & 0x01 );
	}
	else // DSI
	{
		NX_ASSERT( __NX_MIPI_VALID_DSI_INTMASK__ & ( 1UL << (IntNum-32) ) );
		return (CBOOL)( ( __g_pRegister[ModuleIndex]->DSIM_INTMSK >> (IntNum-32) ) & 0x01 );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_MIPI_INTCH_xxx in <nx_MIPI.h>
 *	@return		 CTRUE	indicates that a specified interrupt is pended. 
 *				 CFALSE	indicates that a specified interrupt is not pended.

 */
CBOOL	NX_MIPI_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	if( IntNum < 32 ) // CSI
	{
		NX_ASSERT( __NX_MIPI_VALID_CSI_INTMASK__ & ( 1UL << IntNum ) );
		regvalue  = pRegister->CSIS_INTMSK;
		regvalue &= pRegister->CSIS_INTSRC;
		return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
	}
	else // DSI
	{
		NX_ASSERT( __NX_MIPI_VALID_DSI_INTMASK__ & ( 1UL << (IntNum-32) ) );
		regvalue  = pRegister->DSIM_INTMSK;
		regvalue &= pRegister->DSIM_INTSRC;
		return (CBOOL)( ( regvalue >> (IntNum-32) ) & 0x01 );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.
 *						refer to NX_MIPI_INTCH_xxx in <nx_MIPI.h>
 *	@return		None.

 */
void	NX_MIPI_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	if( IntNum < 32 ) // CSI
	{
		NX_ASSERT( __NX_MIPI_VALID_CSI_INTMASK__ & ( 1UL << IntNum ) );
		WriteIO32(&pRegister->CSIS_INTSRC, 1UL << IntNum);
	}
	else // DSI
	{
		NX_ASSERT( __NX_MIPI_VALID_DSI_INTMASK__ & ( 1UL << (IntNum-32) ) );
		WriteIO32(&pRegister->DSIM_INTSRC, 1UL << (IntNum-32));
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
void	NX_MIPI_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	if( Enable )
	{
		WriteIO32(&pRegister->CSIS_INTMSK, __NX_MIPI_VALID_CSI_INTMASK__);
		WriteIO32(&pRegister->DSIM_INTMSK, __NX_MIPI_VALID_DSI_INTMASK__);
	}
	else
	{
		WriteIO32(&pRegister->CSIS_INTMSK, 0);
		WriteIO32(&pRegister->DSIM_INTMSK, 0);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are enabled. 
 *				 CFALSE	indicates that all interrupts are disabled.

 */
CBOOL	NX_MIPI_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	if( __g_pRegister[ModuleIndex]->CSIS_INTMSK ){ return CTRUE; }
	if( __g_pRegister[ModuleIndex]->DSIM_INTMSK ){ return CTRUE; }
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are pended. 
 *				 CFALSE	indicates that no interrupt is pended.

 */
CBOOL	NX_MIPI_GetInterruptPendingAll( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = pRegister->CSIS_INTMSK;
	regvalue &= pRegister->CSIS_INTSRC;
	if( regvalue ){ return CTRUE; }

	regvalue  = pRegister->DSIM_INTMSK;
	regvalue &= pRegister->DSIM_INTSRC;
	if( regvalue ){ return CTRUE; }

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.r

 */
void	NX_MIPI_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->CSIS_INTSRC, __NX_MIPI_VALID_CSI_INTMASK__);
	WriteIO32(&pRegister->DSIM_INTSRC, __NX_MIPI_VALID_DSI_INTMASK__);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_MIPI_INTCH_xxx in <nx_MIPI.h>

 */
S32		NX_MIPI_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	int i;
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = pRegister->CSIS_INTMSK;
	regvalue &= pRegister->CSIS_INTSRC;
	if( 0!=regvalue )
	{
		for( i=0; i<32; i++ )
		{
			if( regvalue & 1UL )
			{
				return i;
			}
			regvalue>>=1;
		}
	}

	regvalue  = pRegister->DSIM_INTMSK;
	regvalue &= pRegister->DSIM_INTSRC;
	if( 0!=regvalue )
	{
		for( i=0; i<32; i++ )
		{
			if( regvalue & 1UL )
			{
				return i+32;
			}
			regvalue>>=1;
		}
	}

	return -1;
}


#define WRITEREG( regname, mask, value )  regvalue = pRegister->regname; regvalue = (regvalue&(~(mask)))|(value); WriteIO32(&pRegister->regname, regvalue)

//------------------------------------------------------------------------------
///	@name	MIPI-CSI Interface
//------------------------------------------------------------------------------
void  NX_MIPI_CSI_SetSize  ( U32 ModuleIndex, int Channel, U32 Width, U32 Height )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	NX_ASSERT( 1 <= Width && 0xFFFF >= Width );
	NX_ASSERT( 1 <= Height && 0xFFFF >= Height );
	switch( Channel )
	{
	case 0: WriteIO32(&pRegister->CSIS_RESOL_CH0, (Width<<16)|Height); break;
	case 1: WriteIO32(&pRegister->CSIS_RESOL_CH1, (Width<<16)|Height); break;
	case 2: WriteIO32(&pRegister->CSIS_RESOL_CH2, (Width<<16)|Height); break;
	case 3: WriteIO32(&pRegister->CSIS_RESOL_CH3, (Width<<16)|Height); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SetFormat( U32 ModuleIndex, int Channel, NX_MIPI_CSI_FORMAT Format )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	const U32 regmask = 0xFC;
	U32	newvalue = Format<<2;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONFIG_CH0, regmask, newvalue); break;
	case 1: WRITEREG( CSIS_CONFIG_CH1, regmask, newvalue); break;
	case 2: WRITEREG( CSIS_CONFIG_CH2, regmask, newvalue); break;
	case 3: WRITEREG( CSIS_CONFIG_CH3, regmask, newvalue); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SetInterleaveMode( U32 ModuleIndex, NX_MIPI_CSI_INTERLEAVE InterleaveMode )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WRITEREG( CSIS_CONTROL, (3<<22), (InterleaveMode<<22));
}

//	Set timing of Output Protocol of ISP (unit: VCLK)
//	T1 : Interval between rising of VVALID and first rising of HVALID  : 1~64
//	T2 : Interval between last falling of DVALID and falling of HVALID : 2~66
//	T3 : Interval between falling of HVALID and rising of next HVALID  : 1
//	T4 : Interval between rising of HVALID and first rising of DVALID  : 0
//	T5 : Interval between last falling of HVALID and falling of VVALID : 0~4095
//	T6 : Interval between falling of VVALID and rising of next VVALID  : 1
void  NX_MIPI_CSI_SetTimingControl( U32 ModuleIndex, int Channel, int T1, int T2, int T5 )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	const U32 regmask = 0xFFFFFF00;
	U32	newvalue = ((T2-2)<<26)|((T1-1)<<20)|((T5)<<8);
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	NX_ASSERT( 1 <= T1 && 64 >= T1 );
	NX_ASSERT( 2 <= T2 && 66 >= T2 );
	NX_ASSERT( 0 <= T5 && 4095 >= T5 );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONFIG_CH0, regmask, newvalue); break;
	case 1: WRITEREG( CSIS_CONFIG_CH1, regmask, newvalue); break;
	case 2: WRITEREG( CSIS_CONFIG_CH2, regmask, newvalue); break;
	case 3: WRITEREG( CSIS_CONFIG_CH3, regmask, newvalue); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

// Set virtual channel for data interleave
void  NX_MIPI_CSI_SetInterleaveChannel( U32 ModuleIndex, int Channel, int InterleaveChannel )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	const U32 regmask = 0x3;
	U32	newvalue = InterleaveChannel;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > InterleaveChannel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONFIG_CH0, regmask, newvalue); break;
	case 1: WRITEREG( CSIS_CONFIG_CH1, regmask, newvalue); break;
	case 2: WRITEREG( CSIS_CONFIG_CH2, regmask, newvalue); break;
	case 3: WRITEREG( CSIS_CONFIG_CH3, regmask, newvalue); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}


void  NX_MIPI_CSI_EnableDecompress ( U32 ModuleIndex, CBOOL Enable )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( 0==Enable || 1==Enable );
	WRITEREG( CSIS_CONTROL, (1<<24), (Enable<<24));
}

void  NX_MIPI_CSI_SetPrediction ( U32 ModuleIndex, int Channel, NX_MIPI_CSI_PREDICTION Prediction )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONTROL, 1<<25, Prediction<<25); break;
	case 1: WRITEREG( CSIS_CTRL2  , 1<<29, Prediction<<29); break;
	case 2: WRITEREG( CSIS_CTRL2  , 1<<30, Prediction<<30); break;
	case 3: WRITEREG( CSIS_CTRL2  , 1<<31, Prediction<<31); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SetYUV422Layout ( U32 ModuleIndex, int Channel, NX_MIPI_CSI_YUV422LAYOUT YUV422_Layout )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONTROL, 1<<21, YUV422_Layout<<21); break;
	case 1: WRITEREG( CSIS_CTRL2  , 1<<25, YUV422_Layout<<25); break;
	case 2: WRITEREG( CSIS_CTRL2  , 1<<26, YUV422_Layout<<26); break;
	case 3: WRITEREG( CSIS_CTRL2  , 1<<27, YUV422_Layout<<27); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SetParallelDataAlignment32 ( U32 ModuleIndex, int Channel, CBOOL EnableAlign32 )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	NX_ASSERT( 0==EnableAlign32 || 1==EnableAlign32 );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONTROL, 1<<20, EnableAlign32<<20); break;
	case 1: WRITEREG( CSIS_CTRL2  , 1<<21, EnableAlign32<<21); break;
	case 2: WRITEREG( CSIS_CTRL2  , 1<<22, EnableAlign32<<22); break;
	case 3: WRITEREG( CSIS_CTRL2  , 1<<23, EnableAlign32<<23); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SetRGBLayout ( U32 ModuleIndex, int Channel, NX_MIPI_CSI_RGBLAYOUT RGB_Layout )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONTROL, 1<<12, RGB_Layout<<12); break;
	case 1: WRITEREG( CSIS_CONTROL, 1<<13, RGB_Layout<<13); break;
	case 2: WRITEREG( CSIS_CONTROL, 1<<14, RGB_Layout<<14); break;
	case 3: WRITEREG( CSIS_CONTROL, 1<<15, RGB_Layout<<15); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SetVCLK( U32 ModuleIndex, int Channel, NX_MIPI_CSI_VCLKSRC ClockSource )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONTROL, 1<< 8, ClockSource<< 8); break;
	case 1: WRITEREG( CSIS_CONTROL, 1<< 9, ClockSource<< 9); break;
	case 2: WRITEREG( CSIS_CONTROL, 1<<10, ClockSource<<10); break;
	case 3: WRITEREG( CSIS_CONTROL, 1<<11, ClockSource<<11); break;
	default: NX_ASSERT( !"Never get here" ); break;
	}
}

void  NX_MIPI_CSI_SoftwareReset ( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WRITEREG( CSIS_CONTROL, (1<<4), (1<<4));
}

void  NX_MIPI_CSI_SetEnable ( U32 ModuleIndex, CBOOL Enable )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( 0==Enable || 1==Enable );
	//WRITEREG( CSIS_CONTROL, (1<<0), (Enable<<0));
	WRITEREG( CSIS_CONTROL, (1<<0)|(15<<16)|(15<<12), (Enable<<0)|(15<<16)|(15<<12));
}

void  NX_MIPI_CSI_SetPhy( U32 ModuleIndex,
						  U32   NumberOfDataLanes              , // 0~3
                          CBOOL EnableClockLane                ,
                          CBOOL EnableDataLane0                ,
                          CBOOL EnableDataLane1                ,
                          CBOOL EnableDataLane2                ,
                          CBOOL EnableDataLane3                ,
                          CBOOL SwapClockLane                  ,
                          CBOOL SwapDataLane                   )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue ;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	NX_ASSERT( 0==EnableDataLane0 || 1==EnableDataLane0 );
	NX_ASSERT( 0==EnableDataLane1 || 1==EnableDataLane1 );
	NX_ASSERT( 0==EnableDataLane2 || 1==EnableDataLane2 );
	NX_ASSERT( 0==EnableDataLane3 || 1==EnableDataLane3 );
	NX_ASSERT( 0==EnableClockLane || 1==EnableClockLane );
	newvalue = (EnableDataLane3<<4)|(EnableDataLane2<<3)|(EnableDataLane1<<2)|(EnableDataLane0<<1)|(EnableClockLane<<0);
	WRITEREG( CSIS_DPHYCTRL, (0x1F<<0), newvalue);

	NX_ASSERT( 0==SwapClockLane || 1==SwapClockLane );
	NX_ASSERT( 0==SwapDataLane  || 1==SwapDataLane  );
	newvalue = (SwapClockLane<<31)|(SwapDataLane<<30)|(NumberOfDataLanes<<2);
	WRITEREG( CSIS_CONTROL, (0x3<<30)|(3<<2), newvalue);
}


U32   NX_MIPI_CSI_GetVersion ( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	return pRegister->CSIS_VERSION;
}

//	Read current state through shadow register
void  NX_MIPI_CSI_GetCurrentState( U32 ModuleIndex, int Channel, int* pWidth, int* pHeight, int* pT1, int* pT2, int* pT5, NX_MIPI_CSI_FORMAT* pFormat, int* pInterleaveChannel )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	register U32	regvalue2;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NX_MIPI_NUMBEROF_CSI_CHANNELS > Channel );
	switch( Channel )
	{
	case 0: WRITEREG( CSIS_CONTROL, 1<<16, 1<<16); while( pRegister->CSIS_CONTROL & (1<<16) ){} regvalue = pRegister->CSIS_CONFIG_CH0; regvalue2 = pRegister->CSIS_RESOL_CH0; break;
	case 1: WRITEREG( CSIS_CONTROL, 1<<17, 1<<17); while( pRegister->CSIS_CONTROL & (1<<17) ){} regvalue = pRegister->CSIS_CONFIG_CH1; regvalue2 = pRegister->CSIS_RESOL_CH1; break;
	case 2: WRITEREG( CSIS_CONTROL, 1<<18, 1<<18); while( pRegister->CSIS_CONTROL & (1<<18) ){} regvalue = pRegister->CSIS_CONFIG_CH2; regvalue2 = pRegister->CSIS_RESOL_CH2; break;
	case 3: WRITEREG( CSIS_CONTROL, 1<<19, 1<<19); while( pRegister->CSIS_CONTROL & (1<<19) ){} regvalue = pRegister->CSIS_CONFIG_CH3; regvalue2 = pRegister->CSIS_RESOL_CH3; break;
	default: NX_ASSERT( !"Never get here" ); break;
	}

	if( pWidth             ){ *pWidth             = (((regvalue2)>>16) & 0xFFFF); }
	if( pHeight            ){ *pHeight            = (((regvalue2)>> 0) & 0xFFFF); }
	if( pT1                ){ *pT1                = (((regvalue )>>20) & 0x3F  )+1; }
	if( pT2                ){ *pT2                = (((regvalue )>>26) & 0x3F  )+2; }
	if( pT5                ){ *pT5                = (((regvalue )>> 8) & 0xFFF ); }
	if( pFormat            ){ *pFormat            = (NX_MIPI_CSI_FORMAT)(((regvalue )>> 2) & 0x3F  ); }
	if( pInterleaveChannel ){ *pInterleaveChannel = (((regvalue )>> 0) & 0x3   ); }
}

U32   NX_MIPI_CSI_GetNonImageData( U32 ModuleIndex, U32 Address32 )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( 0x2000/4 >= Address32 );
	return pRegister->MIPI_CSIS_PKTDATA[Address32];
}

void  NX_MIPI_CSI_GetStatus( U32 ModuleIndex, U32* pULPS, U32* pStop )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue = pRegister->CSIS_DPHYSTS;
	if( pULPS )
	{
		*pULPS = 0;
		if( regvalue & (1<< 8) ) *pULPS |= (1<<0);
		if( regvalue & (1<< 9) ) *pULPS |= (1<<1);
		if( regvalue & (1<<10) ) *pULPS |= (1<<2);
		if( regvalue & (1<<11) ) *pULPS |= (1<<3);
		if( regvalue & (1<< 1) ) *pULPS |= (1<<4);
	}
	if( pStop )
	{
		*pStop = 0;
		if( regvalue & (1<< 4) ) *pStop |= (1<<0);
		if( regvalue & (1<< 5) ) *pStop |= (1<<1);
		if( regvalue & (1<< 6) ) *pStop |= (1<<2);
		if( regvalue & (1<< 7) ) *pStop |= (1<<3);
		if( regvalue & (1<< 0) ) *pStop |= (1<<4);
	}
}

//------------------------------------------------------------------------------
///	@name	MIPI-DSI Interface
//------------------------------------------------------------------------------
//  bit 0 of ULPS : '1' means data lane 0 is ULPS.
//  bit 1 of ULPS : '1' means data lane 1 is ULPS.
//  bit 2 of ULPS : '1' means data lane 2 is ULPS.
//  bit 3 of ULPS : '1' means data lane 3 is ULPS.
//  bit 4 of ULPS : '1' means clock lane is ULPS.
//  bit 0 of Stop : '1' means data lane 0 is stop state.
//  bit 1 of Stop : '1' means data lane 1 is stop state.
//  bit 2 of Stop : '1' means data lane 2 is stop state.
//  bit 3 of Stop : '1' means data lane 3 is stop state.
//  bit 4 of Stop : '1' means clock lane is stop state.
void  NX_MIPI_DSI_GetStatus( U32 ModuleIndex, U32* pULPS, U32* pStop, U32* pIsPLLStable, U32* pIsInReset, U32* pIsBackward, U32* pIsHSClockReady )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue = pRegister->DSIM_STATUS;
	if( pULPS )
	{
		*pULPS = 0;
		if( regvalue & (1<< 4) ) *pULPS |= (1<<0);
		if( regvalue & (1<< 5) ) *pULPS |= (1<<1);
		if( regvalue & (1<< 6) ) *pULPS |= (1<<2);
		if( regvalue & (1<< 7) ) *pULPS |= (1<<3);
		if( regvalue & (1<< 9) ) *pULPS |= (1<<4);
	}
	if( pStop )
	{
		*pStop = 0;
		if( regvalue & (1<< 0) ) *pStop |= (1<<0);
		if( regvalue & (1<< 1) ) *pStop |= (1<<1);
		if( regvalue & (1<< 2) ) *pStop |= (1<<2);
		if( regvalue & (1<< 3) ) *pStop |= (1<<3);
		if( regvalue & (1<< 8) ) *pStop |= (1<<4);
	}

	if( pIsPLLStable    ){ *pIsPLLStable    = (regvalue >> 31)  & 1; }
	if( pIsInReset      ){ *pIsInReset      = ((regvalue >> 20)  & 1) ? 0 : 1; }
	if( pIsBackward     ){ *pIsBackward     = (regvalue >> 16)  & 1; }
	if( pIsHSClockReady ){ *pIsHSClockReady = (regvalue >> 10)  & 1; }
}

void  NX_MIPI_DSI_SoftwareReset ( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	pRegister->DSIM_SWRST = 0x00010001;
	while( 0 != (pRegister->DSIM_STATUS & (1<<20)) ){}

	// @reset

	pRegister->DSIM_SWRST = 0x00000000;
	//while( 0 == (pRegister->DSIM_STATUS & (1<<20)) ){}

}

void  NX_MIPI_DSI_SetClock ( U32 ModuleIndex,
                             CBOOL EnableTXHSClock    ,
                             CBOOL UseExternalClock   , // CFALSE: PLL clock CTRUE: External clock
                             CBOOL EnableByteClock    , // ByteClock means (D-PHY PLL clock / 8)
                             CBOOL EnableESCClock_ClockLane,
                             CBOOL EnableESCClock_DataLane0,
                             CBOOL EnableESCClock_DataLane1,
                             CBOOL EnableESCClock_DataLane2,
                             CBOOL EnableESCClock_DataLane3,
                             CBOOL EnableESCPrescaler , // ESCClock = ByteClock / ESCPrescalerValue
                             U32   ESCPrescalerValue  )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( ESCPrescalerValue <= 0xFFFF );
	regvalue =0;
    regvalue |= (EnableTXHSClock         <<31);
    regvalue |= (UseExternalClock        <<27);
    regvalue |= (EnableByteClock         <<24);
    regvalue |= (EnableESCClock_ClockLane<<19);
    regvalue |= (EnableESCClock_DataLane0<<20);
    regvalue |= (EnableESCClock_DataLane1<<21);
    regvalue |= (EnableESCClock_DataLane2<<22);
    regvalue |= (EnableESCClock_DataLane3<<23);
    regvalue |= (EnableESCPrescaler      <<28);
	regvalue |= ESCPrescalerValue;
	pRegister->DSIM_CLKCTRL = regvalue;
}

void  NX_MIPI_DSI_SetTimeout( U32 ModuleIndex, U32 BtaTout, U32 LPdrTout )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( BtaTout  <= 0xFF );
	NX_ASSERT( LPdrTout <= 0xFFFF );
	regvalue =0;
    regvalue |= (BtaTout  <<16);
    regvalue |= (LPdrTout << 0);
	pRegister->DSIM_TIMEOUT = regvalue;
}

void  NX_MIPI_DSI_SetConfigVideoMode  ( U32 ModuleIndex                      ,
										CBOOL EnableAutoFlushMainDisplayFIFO ,
										CBOOL EnableAutoVerticalCount        ,
										CBOOL EnableBurst                    ,
										NX_MIPI_DSI_SYNCMODE SyncMode        ,
										CBOOL EnableEoTPacket                ,
										CBOOL EnableHsyncEndPacket           , // Set HSEMode=1
										CBOOL EnableHFP                      , // Set HFPMode=0
										CBOOL EnableHBP                      , // Set HBPMode=0
										CBOOL EnableHSA                      , // Set HSAMode=0
										U32   NumberOfVirtualChannel         , // 0~3
										NX_MIPI_DSI_FORMAT Format            ,
			                            U32   NumberOfWordsInHFP             , // ~65535
			                            U32   NumberOfWordsInHBP             , // ~65535
			                            U32   NumberOfWordsInHSYNC           , // ~65535
			                            U32   NumberOfLinesInVFP             , // ~2047
			                            U32   NumberOfLinesInVBP             , // ~2047
			                            U32   NumberOfLinesInVSYNC           , // ~1023
			                            U32   NumberOfLinesInCommandAllow
			                            )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NumberOfVirtualChannel <= 3      );
	NX_ASSERT( NumberOfWordsInHFP     <= 0xFFFF );
	NX_ASSERT( NumberOfWordsInHBP     <= 0xFFFF );
	NX_ASSERT( NumberOfWordsInHSYNC   <= 0xFFFF );
	NX_ASSERT( NumberOfLinesInVFP     <= 0x7FF  );
	NX_ASSERT( NumberOfLinesInVBP     <= 0x7FF  );
	NX_ASSERT( NumberOfLinesInVSYNC   <= 0x3FF  );
	NX_ASSERT( NumberOfLinesInCommandAllow <= 15);
	NX_ASSERT( Format >= NX_MIPI_DSI_FORMAT_RGB565 );
	NX_ASSERT( NX_MIPI_DSI_SYNCMODE_PULSE != SyncMode || CFALSE == EnableBurst );
	newvalue = (1<<25);
    newvalue |= ((1-EnableAutoFlushMainDisplayFIFO) <<29);
    newvalue |= (EnableAutoVerticalCount        <<24);
    newvalue |= (EnableBurst                    <<26);
    newvalue |= (SyncMode                       <<27);
    newvalue |= ((1-EnableEoTPacket)            <<28);
    newvalue |= (EnableHsyncEndPacket           <<23);
    newvalue |= ((1-EnableHFP)                  <<22);
    newvalue |= ((1-EnableHBP)                  <<21);
    newvalue |= ((1-EnableHSA)                  <<20);
    newvalue |= (NumberOfVirtualChannel         <<18);
    newvalue |= (Format                         <<12);
    WRITEREG( DSIM_CONFIG, 0xFFFFFF00, newvalue );

    newvalue  = (NumberOfLinesInCommandAllow <<28);
    newvalue |= (NumberOfLinesInVFP << 16 );
    newvalue |= (NumberOfLinesInVBP <<  0 );
    pRegister->DSIM_MVPORCH = newvalue;

    newvalue  = (NumberOfWordsInHFP <<16);
    newvalue |= (NumberOfWordsInHBP << 0);
    pRegister->DSIM_MHPORCH = newvalue;

    newvalue  = (NumberOfWordsInHSYNC<< 0);
    newvalue |= (NumberOfLinesInVSYNC<<22);
    pRegister->DSIM_MSYNC = newvalue;
}


void  NX_MIPI_DSI_SetConfigCommandMode( U32 ModuleIndex                      ,
										CBOOL EnableAutoFlushMainDisplayFIFO ,
										CBOOL EnableEoTPacket                ,
										U32   NumberOfVirtualChannel         , // 0~3
										NX_MIPI_DSI_FORMAT Format
			                            )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NumberOfVirtualChannel <= 3      );
	NX_ASSERT( Format < NX_MIPI_DSI_FORMAT_RGB565 || Format > NX_MIPI_DSI_FORMAT_RGB666_PACKED );
	newvalue = (0<<25);
    newvalue |= (EnableAutoFlushMainDisplayFIFO <<29);
    newvalue |= (EnableEoTPacket                <<28);
    newvalue |= (NumberOfVirtualChannel         <<18);
    newvalue |= (Format                         <<12);
    WRITEREG( DSIM_CONFIG, 0xFFFFFF00, newvalue );
}

void  NX_MIPI_DSI_SetEscapeMode( U32 ModuleIndex, U32 StopStateCount, CBOOL ForceStopState, CBOOL ForceBTA, NX_MIPI_DSI_LPMODE CmdinLP, NX_MIPI_DSI_LPMODE TXinLP )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( 0x3FF >= StopStateCount );
	newvalue  = (StopStateCount <<21);
    newvalue |= (ForceStopState <<20);
    newvalue |= (ForceBTA       <<16);
    newvalue |= (CmdinLP        << 7);
    newvalue |= (TXinLP         << 6);
    WRITEREG( DSIM_ESCMODE, 0xFFFFFFC0, newvalue );
}

void  NX_MIPI_DSI_RemoteResetTrigger( U32 ModuleIndex )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	newvalue  = (1<<4);
    WRITEREG( DSIM_ESCMODE, (1<<4), newvalue );
    while( pRegister->DSIM_ESCMODE & (1<<4) ){}
}

void  NX_MIPI_DSI_SetULPS( U32 ModuleIndex, CBOOL ULPSClockLane, CBOOL ULPSDataLane )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = pRegister->DSIM_ESCMODE;

	if( ULPSClockLane )
	{
		regvalue &= ~(1<<0);
		regvalue |=  (1<<1);
	}
	else
	{
		regvalue |=  (1<<0);
	}

	if( ULPSDataLane )
	{
		regvalue &= ~(1<<2);
		regvalue |=  (1<<3);
	}
	else
	{
		regvalue |=  (1<<2);
	}

	pRegister->DSIM_ESCMODE = regvalue ;

	if( ULPSClockLane )
	{
		while( (1<<9) == (pRegister->DSIM_STATUS & (1<<9)) ){}
	}
	else
	{
		while( 0 != (pRegister->DSIM_STATUS & (1<<9)) ){}
	}

	if( ULPSDataLane )
	{
		while( (15<<4) == (pRegister->DSIM_STATUS & (15<<4)) ){}
	}
	else
	{
		while( 0 != (pRegister->DSIM_STATUS & (15<<4)) ){}
	}

	if( ! ULPSClockLane )
	{
		regvalue &= (3<<0);
	}

	if( ! ULPSDataLane )
	{
		regvalue |=  (3<<2);
	}

	pRegister->DSIM_ESCMODE = regvalue ;
}

void  NX_MIPI_DSI_SetSize  ( U32 ModuleIndex, U32 Width, U32 Height )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( 0xFFF >= Width  );
	NX_ASSERT( 0xFFF >= Height );
	newvalue  = (Height <<16);
	newvalue |= (Width  << 0);
    WRITEREG( DSIM_MDRESOL, 0x0FFFFFFF, newvalue );
}

void  NX_MIPI_DSI_SetEnable( U32 ModuleIndex, CBOOL Enable )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
    WRITEREG( DSIM_MDRESOL, (1<<31), (Enable<<31) );
}

void  NX_MIPI_DSI_SetPhy( U32 ModuleIndex,
						  U32   NumberOfDataLanes , // 0~3
                          CBOOL EnableClockLane   ,
                          CBOOL EnableDataLane0   ,
                          CBOOL EnableDataLane1   ,
                          CBOOL EnableDataLane2   ,
                          CBOOL EnableDataLane3   ,
                          CBOOL SwapClockLane     ,
                          CBOOL SwapDataLane      )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( NumberOfDataLanes <= 3 );
	newvalue = (NumberOfDataLanes<<5);
    newvalue |= (EnableClockLane<<0);
    newvalue |= (EnableDataLane0<<1);
    newvalue |= (EnableDataLane1<<2);
    newvalue |= (EnableDataLane2<<3);
    newvalue |= (EnableDataLane3<<4);
    WRITEREG( DSIM_CONFIG, 0xFF, newvalue );
    newvalue  = (SwapClockLane<<1);
    newvalue |= (SwapDataLane <<0);
    WRITEREG( DSIM_PHYACCHR1, 0x3, newvalue );
}

void  NX_MIPI_DSI_SetPLL( U32 ModuleIndex   ,
                          CBOOL Enable      ,
					      U32 PLLStableTimer,
						  U32 M_PLLPMS      , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
						  U32 M_BANDCTL     , // [3:0] Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
						  U32 M_DPHYCTL     , //
						  U32 B_DPHYCTL       // Refer to 10.2.3 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf or NX_MIPI_PHY_B_DPHYCTL enum or LN28LPP_MipiDphyCore1p5Gbps_Supplement.
						  )
{
	register NX_MIPI_RegisterSet* pRegister;
	register U32	regvalue;
	U32 newvalue;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	if( ! Enable )
	{
		newvalue  = (Enable  <<23);
		newvalue |= (M_PLLPMS<< 1);
		newvalue |= (M_BANDCTL<<24);
    	WRITEREG( DSIM_PLLCTRL, 0x0FFFFFFF, newvalue );
    }
    pRegister->DSIM_PHYACCHR  = M_DPHYCTL           ;
    pRegister->DSIM_PLLTMR    = PLLStableTimer      ;
    pRegister->DSIM_PHYACCHR1 = (B_DPHYCTL<<9)      ; // {B_DPHYDTL[22:0], 2'b0, M_PRPRCTLCLK[2:0], 2'b0,M_DPDN_SWAP_CLK, M_DPDN_SWAP_DAT}
	if( Enable )
	{
		newvalue  = (Enable  <<23);
		newvalue |= (M_PLLPMS<< 1);
		newvalue |= (M_BANDCTL<<24);
    	WRITEREG( DSIM_PLLCTRL, 0x0FFFFFFF, newvalue );
    }
}

//U32   NX_MIPI_DSI_GetVersion ( U32 ModuleIndex )
//{
//	register NX_MIPI_RegisterSet* pRegister;
//	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//	pRegister = __g_pRegister[ModuleIndex];
//	return pRegister->DSIM_VERSION;
//}

void  NX_MIPI_DSI_WritePacket      ( U32 ModuleIndex, U32 DataCount32, const U32* pData32 )
{
	register NX_MIPI_RegisterSet* pRegister;
	int i;
	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
	NX_ASSERT( 512 >= DataCount32 );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

    for( i=0; i<DataCount32; i++ )
    {
    	pRegister->DSIM_PAYLOAD = *pData32++;
    }
    pRegister->DSIM_PKTHDR  = 0x29 | (DataCount32<<(2+8));
}
//void  NX_MIPI_DSI_ReadPacketHeader ( U32 ModuleIndex, U32* pDataID, U32* pDataCount32 );
//void  NX_MIPI_DSI_ReadPacketData   ( U32 ModuleIndex, U32 DataCount32, const U32* pData32 );











//------------------------------------------------------------------------------
///	@name	MIPI-PHY Interface
//------------------------------------------------------------------------------
//	volatile U32 CSIS_DPHYCTRL_0; // 0x0020 R/W DPHY Analog Control register0                       0x0000_0000     O_B_DphyCtl
//	volatile U32 CSIS_DPHYCTRL_1; // 0x0024 R/W DPHY Analog Control register1                       0x0000_0000     O_S_DphyCtl

// void  NX_MIPI_PHY_SetControl( U32 ModuleIndex   ,
// 							  int M_TCLKPRPRCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TCLKZEROCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TCLKPOSTCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TCLKTRAILCTL, // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSPRPRCTL  , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSZEROCTL  , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSTRAILCTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_TLPXCTL     , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_THSEXITCTL  , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int S_HSSETTLECTL , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. (4.3 of UG_MOCO_MIPI_CSIS_V3.0.pdf)
// 							  int S_CLKSETTLECTL, // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement. (4.3 of UG_MOCO_MIPI_CSIS_V3.0.pdf)
// 							  int M_PLLPMS      , // Use LN28LPP_MipiDphyCore1p5Gbps_Supplement.
// 							  int M_PLLCTL      , // Refer to 10.2.2 M_PLLCTL of MIPI_D_PHY_USER_GUIDE.pdf
// 							  int B_DPHYCTL     ) // Use NX_MIPI_PHY_B_DPHYCTL enum or refer to LN28LPP_MipiDphyCore1p5Gbps_Supplement. B_DPHYCTL[9:0]
// {
// 	register NX_MIPI_RegisterSet* pRegister;
// 	register U32	regvalue;
// 	register U32	newvalue;
// 	NX_ASSERT( NUMBER_OF_MIPI_MODULE > ModuleIndex );
// 	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
// 	pRegister = __g_pRegister[ModuleIndex];
//
// 	// @todo
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_TCLKPRPRCTL );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_TCLKZEROCTL );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_TCLKPOSTCTL );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_TCLKTRAILCTL);
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_THSPRPRCTL  );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_THSZEROCTL  );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_THSTRAILCTL );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_TLPXCTL     );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_THSEXITCTL  );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_PLLPMS      );
// 	NX_DISABLE_UNUSED_VAR_WARNING( M_PLLCTL      );
// 	NX_DISABLE_UNUSED_VAR_WARNING( B_DPHYCTL     );
//
// 	NX_ASSERT( 0<=S_HSSETTLECTL  && 0x1F>=S_HSSETTLECTL  );
// 	NX_ASSERT( 0<=S_CLKSETTLECTL && 0x3 >=S_CLKSETTLECTL )
// 	newvalue = (S_HSSETTLECTL<<24) |(S_CLKSETTLECTL<<22);
// 	WRITEREG( CSIS_DPHYCTRL, 0xFFC00000, newvalue);
// }
