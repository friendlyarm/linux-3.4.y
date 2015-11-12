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
//	File		: nx_USB20OTG.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_usb20otg.h"

static	NX_USB20OTG_RegisterSet *__g_pRegister[NUMBER_OF_USB20OTG_MODULE];
static	NX_USB20OTG_APB_RegisterSet *__g_pApbRegister[0];


//------------------------------------------------------------------------------
//
//	USB20OTG Interface
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
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.
 *	@see	NX_USB20OTG_GetNumberOfModule
 */
CBOOL	NX_USB20OTG_Initialize( void )
{
	static CBOOL bInit = CFALSE;
    int i;

	if( CFALSE == bInit )
	{
        for(i = 0; i < NUMBER_OF_USB20HOST_MODULE; i++ )
        {
		    __g_pRegister[i] = CNULL;
        }
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. \n
 *				It is equal to NUMBER_OF_USB20OTG_MODULE in <nx_chip.h>.
 *	@see		NX_USB20OTG_Initialize
 */
U32 		NX_USB20OTG_GetNumberOfModule( void )
{
	return NUMBER_OF_USB20OTG_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32 		NX_USB20OTG_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_USB20OTG_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_USB20OTG_SetBaseAddress(  U32 ModuleIndex, void* BaseAddress )
{
    if( ModuleIndex == 0 ) { __g_pRegister[0] = (NX_USB20OTG_RegisterSet *)BaseAddress; }
    if( ModuleIndex == 1 ) { __g_pApbRegister[0] = (NX_USB20OTG_APB_RegisterSet *)BaseAddress; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*    NX_USB20OTG_GetBaseAddress( U32 ModuleIndex )
{
    if( ModuleIndex == 0 ) { return (void*)__g_pRegister[0];	 }
    if( ModuleIndex == 1 ) { return (void*)__g_pApbRegister[0];	 }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. \n
 *				It is equal to PHY_BASEADDR_USB20OTG?_MODULE in <nx_chip.h>.
 */
U32 		NX_USB20OTG_GetPhysicalAddress( U32 ModuleIndex  )
{
    if( ModuleIndex == 0 ) { return PHY_BASEADDR_WITH_CHANNEL_LIST( USB20OTG, AHBS0 ); }
    if( ModuleIndex == 1 ) { return PHY_BASEADDR_WITH_CHANNEL_LIST( USB20OTG, APB ); }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 */
 //ian
CBOOL	NX_USB20OTG_OpenModule(  )
{
    NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 */
 //ian
CBOOL	NX_USB20OTG_CloseModule(  )
{
    NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_USB20OTG_CheckBusy(  )
{
    NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	return CFALSE;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.\n
 *				It is equal to CLOCKINDEX_OF_USB20OTG?_MODULE in <nx_chip.h>.
 */
U32  NX_USB20OTG_GetClockNumber (  )
{
//	const U32  ClockNumber[] =
//	{
//		CLOCKINDEX_LIST( USB20OTG )
//	};
//	NX_CASSERT( NUMBER_OF_USB20OTG_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
//    NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
//	return	ClockNumber[0];
	return	0;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_USB20OTG?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32  NX_USB20OTG_GetResetNumber (  )
{
	const U32  ResetNumber[] =
	{
		RESETINDEX_LIST( USB20OTG, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_USB20OTG_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	return	ResetNumber[0];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	0		an index of module.
 *	@return		A interrupt number.\n
 *				It is equal to INTNUM_OF_USB20OTG?_MODULE in <nx_chip.h>.
 */
U32  	NX_USB20OTG_GetInterruptNumber(  )
{
    const U32  InterruptNumber[] = { INTNUM_LIST( USB20OTG ) };
    NX_CASSERT( NUMBER_OF_USB20OTG_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
    // NX_ASSERT( INTNUM_OF_USB20OTG0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[0];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	0		an index of module.
 *	@param[in]	Enable	\b Set as CTRUE to enable all interrupts. \r\n
 *						\b Set as CFALSE to disable all interrupts.
 *	@return		None.

 */
void	NX_USB20OTG_SetInterruptEnableAll(  CBOOL Enable )
{
	register NX_USB20OTG_RegisterSet* pRegister;
	register U32 	regvalue;

	NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[0] );

	pRegister = __g_pRegister[0];
	regvalue  = Enable ? 0xF77EFCFE : 0 ;


	WriteIO32(&pRegister->GINTMSK, regvalue);

}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	0		an index of module.
 *	@return		\b CTRUE	indicates that one or more interrupts are enabled. \r\n
 *				\b CFALSE	indicates that all interrupts are disabled.

 */
CBOOL	NX_USB20OTG_GetInterruptEnableAll(  )
{
	NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	NX_ASSERT( CNULL != __g_pRegister[0] );
	return (CBOOL)(0!=( __g_pRegister[0]->GINTMSK & 0xF77EFCFE ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	0		an index of module.
 *	@return		\b CTRUE	indicates that one or more interrupts are pended. \r\n
 *				\b CFALSE	indicates that no interrupt is pended.

 */
CBOOL	NX_USB20OTG_GetInterruptPendingAll(  )
{
	register NX_USB20OTG_RegisterSet* pRegister;
	register U32 	regvalue;
	NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	NX_ASSERT( CNULL != __g_pRegister[0] );
	pRegister = __g_pRegister[0];
	regvalue  = pRegister->GAHBCFG & 1;
	return (CBOOL)( 0 != ( regvalue ) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	0		an index of module.
 *	@return		None.

 */
void	NX_USB20OTG_ClearInterruptPendingAll(  )
{
	register NX_USB20OTG_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	NX_ASSERT( CNULL != __g_pRegister[0] );
	register U32 	regvalue;
	pRegister = __g_pRegister[0];
	regvalue  = pRegister->GAHBCFG;
	WriteIO32(&pRegister->GAHBCFG, regvalue & 0xFFFFFFFE);
	WriteIO32(&pRegister->GINTSTS, 0xFFFFFFFF);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	0		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.\n
 *				refer to NX_USB20OTG_INTCH_xxx in <nx_USB20OTG.h>

 */
S32		NX_USB20OTG_GetInterruptPendingNumber(  )	// -1 if None
{
	int i;
	register NX_USB20OTG_RegisterSet* pRegister;
	register U32 	regvalue;
	NX_ASSERT( NUMBER_OF_USB20OTG_MODULE > 0 );
	NX_ASSERT( CNULL != __g_pRegister[0] );
	pRegister = __g_pRegister[0];
	regvalue  = pRegister->GINTSTS;
	regvalue &= pRegister->GINTMSK;
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
	return -1;
}

//------------------------------------------------------------------------------
// set member function
//------------------------------------------------------------------------------
void OTG_SetGOTGCTL ( U32  tGOTGCTL)
{
	__g_pRegister[0]->GOTGCTL = tGOTGCTL;
}

void OTG_SetGOTGINT ( U32  tGOTGINT)
{
	__g_pRegister[0]->GOTGINT = tGOTGINT;
}

void OTG_SetGAHBCFG ( U32  tGAHBCFG)
{
	__g_pRegister[0]->GAHBCFG = tGAHBCFG;
}

void OTG_SetGUSBCFG ( U32  tGUSBCFG)
{
	__g_pRegister[0]->GUSBCFG = tGUSBCFG;
}

void OTG_SetGRSTCTL ( U32  tGRSTCTL)
{
	__g_pRegister[0]->GRSTCTL = tGRSTCTL;
}

void OTG_SetGINTSTS ( U32  tGINTSTS)
{
	__g_pRegister[0]->GINTSTS = tGINTSTS;
}

void OTG_SetGINTMSK ( U32  tGINTMSK)
{
	__g_pRegister[0]->GINTMSK = tGINTMSK;
}

void OTG_SetGRXSTSR ( U32  tGRXSTSR)
{
	__g_pRegister[0]->GRXSTSR = tGRXSTSR;
}

void OTG_SetGRXSTSP ( U32  tGRXSTSP)
{
	__g_pRegister[0]->GRXSTSP = tGRXSTSP;
}

void OTG_SetGRXFSIZ ( U32  tGRXFSIZ)
{
	__g_pRegister[0]->GRXFSIZ = tGRXFSIZ;
}

void OTG_SetGNPTXFSIZ ( U32  tGNPTXFSIZ)
{
	__g_pRegister[0]->GNPTXFSIZ = tGNPTXFSIZ;
}

void OTG_SetGNPTXSTS ( U32  tGNPTXSTS)
{
	__g_pRegister[0]->GNPTXSTS = tGNPTXSTS;
}

void OTG_SetGI2CCTL ( U32  tGI2CCTL)
{
	__g_pRegister[0]->GI2CCTL = tGI2CCTL;
}

void OTG_SetGPVNDCTL ( U32  tGPVNDCTL)
{
	__g_pRegister[0]->GPVNDCTL = tGPVNDCTL;
}

void OTG_SetGGPIO ( U32  tGGPIO)
{
	__g_pRegister[0]->GGPIO = tGGPIO;
}

void OTG_SetGUID ( U32  tGUID)
{
	__g_pRegister[0]->GUID = tGUID;
}

void OTG_SetGSNPSID ( U32  tGSNPSID)
{
	__g_pRegister[0]->GSNPSID = tGSNPSID;
}

void OTG_SetGHWCFG1 ( U32  tGHWCFG1)
{
	__g_pRegister[0]->GHWCFG1 = tGHWCFG1;
}

void OTG_SetGHWCFG2 ( U32  tGHWCFG2)
{
	__g_pRegister[0]->GHWCFG2 = tGHWCFG2;
}

void OTG_SetGHWCFG3 ( U32  tGHWCFG3)
{
	__g_pRegister[0]->GHWCFG3 = tGHWCFG3;
}

void OTG_SetGHWCFG4 ( U32  tGHWCFG4)
{
	__g_pRegister[0]->GHWCFG4 = tGHWCFG4;
}

void OTG_SetGLPMCFG ( U32  tGLPMCFG)
{
	__g_pRegister[0]->GLPMCFG = tGLPMCFG;
}

void OTG_SetHPTXFSIZ ( U32  tHPTXFSIZ)
{
	__g_pRegister[0]->HPTXFSIZ = tHPTXFSIZ;
}

void OTG_SetHCFG ( U32  tHCFG)
{
	__g_pRegister[0]->HCFG = tHCFG;
}

void OTG_SetHFIR ( U32  tHFIR)
{
	__g_pRegister[0]->HFIR = tHFIR;
}

void OTG_SetHFNUM ( U32  tHFNUM)
{
	__g_pRegister[0]->HFNUM = tHFNUM;
}

void OTG_SetHPTXSTS ( U32  tHPTXSTS)
{
	__g_pRegister[0]->HPTXSTS = tHPTXSTS;
}

void OTG_SetHAINT ( U32  tHAINT)
{
	__g_pRegister[0]->HAINT = tHAINT;
}

void OTG_SetHAINTMSK ( U32  tHAINTMSK)
{
	__g_pRegister[0]->HAINTMSK = tHAINTMSK;
}

void OTG_SetHPRT ( U32  tHPRT)
{
	__g_pRegister[0]->HPRT = tHPRT;
}

void OTG_SetDCFG ( U32  tDCFG)
{
	__g_pRegister[0]->DCFG = tDCFG;
}

void OTG_SetDCTL ( U32  tDCTL)
{
	__g_pRegister[0]->DCTL = tDCTL;
}

void OTG_SetDSTS ( U32  tDSTS)
{
	__g_pRegister[0]->DSTS = tDSTS;
}

void OTG_SetDIEPMSK ( U32  tDIEPMSK)
{
	__g_pRegister[0]->DIEPMSK = tDIEPMSK;
}

void OTG_SetDOEPMSK ( U32  tDOEPMSK)
{
	__g_pRegister[0]->DOEPMSK = tDOEPMSK;
}

void OTG_SetDAINT ( U32  tDAINT)
{
	__g_pRegister[0]->DAINT = tDAINT;
}

void OTG_SetDAINTMSK ( U32  tDAINTMSK)
{
	__g_pRegister[0]->DAINTMSK = tDAINTMSK;
}

void OTG_SetDTKNQR1 ( U32  tDTKNQR1)
{
	__g_pRegister[0]->DTKNQR1 = tDTKNQR1;
}

void OTG_SetDTKNQR2 ( U32  tDTKNQR2)
{
	__g_pRegister[0]->DTKNQR2 = tDTKNQR2;
}

void OTG_SetDVBUSDIS ( U32  tDVBUSDIS)
{
	__g_pRegister[0]->DVBUSDIS = tDVBUSDIS;
}

void OTG_SetDVBUSPULSE ( U32  tDVBUSPULSE)
{
	__g_pRegister[0]->DVBUSPULSE = tDVBUSPULSE;
}

void OTG_SetDTKNQR3 ( U32  tDTKNQR3)
{
	__g_pRegister[0]->DTKNQR3 = tDTKNQR3;
}

void OTG_SetDTKNQR4 ( U32  tDTKNQR4)
{
	__g_pRegister[0]->DTKNQR4 = tDTKNQR4;
}

void OTG_SetPCGCCTL ( U32  tPCGCCTL)
{
	__g_pRegister[0]->PCGCCTL = tPCGCCTL;
}

//void OTG_SetTESTPARM0 ( U32  tTESTPARM0)
//{
//	__g_pRegister[0]->TESTPARM0 = tTESTPARM0;
//}
//
//void OTG_SetTESTPARM1 ( U32  tTESTPARM1)
//{
//	__g_pRegister[0]->TESTPARM1 = tTESTPARM1;
//}
//
//void OTG_SetTESTPARM2 ( U32  tTESTPARM2)
//{
//	__g_pRegister[0]->TESTPARM2 = tTESTPARM2;
//}
//
//void OTG_SetTESTPARM3 ( U32  tTESTPARM3)
//{
//	__g_pRegister[0]->TESTPARM3 = tTESTPARM3;
//}
//
//void OTG_SetTESTPARM4 ( U32  tTESTPARM4)
//{
//	__g_pRegister[0]->TESTPARM4 = tTESTPARM4;
//}
//
//void OTG_SetTESTPARM5 ( U32  tTESTPARM5)
//{
//	__g_pRegister[0]->TESTPARM5 = tTESTPARM5;
//}
//
//void OTG_SetTESTPARM6 ( U32  tTESTPARM6)
//{
//	__g_pRegister[0]->TESTPARM6 = tTESTPARM6;
//}
//
//void OTG_SetTESTPARM7 ( U32  tTESTPARM7)
//{
//	__g_pRegister[0]->TESTPARM7 = tTESTPARM7;
//}
//
//void OTG_SetTESTPARM8 ( U32  tTESTPARM8)
//{
//	__g_pRegister[0]->TESTPARM8 = tTESTPARM8;
//}
//
//void OTG_SetTESTPARM9 ( U32  tTESTPARM9)
//{
//	__g_pRegister[0]->TESTPARM9 = tTESTPARM9;
//}
//
//void OTG_SetTESTPARM10 ( U32  tTESTPARM10)
//{
//	__g_pRegister[0]->TESTPARM10 = tTESTPARM10;
//}

//------------------------------------------------------------------------------
//DPTXFSIZ, HCSR, DIESR, DOESR
void OTG_SetDPTXFSIZ ( U32  tNo, U32  tDPTXFSIZ)
{
	__g_pRegister[0]->DPTXFSIZ[tNo] = tDPTXFSIZ;
}

void OTG_SetHCCHAR( U32  tNo, U32  tHCCHAR)
{
	__g_pRegister[0]->HCSR16[8*tNo+0] = tHCCHAR;
}

void OTG_SetHCSPLT( U32  tNo, U32  tHCSPLT)
{
	__g_pRegister[0]->HCSR16[8*tNo+1] = tHCSPLT;
}

void OTG_SetHCINT( U32  tNo, U32  tHCINT)
{
	__g_pRegister[0]->HCSR16[8*tNo+2] = tHCINT;
}

void OTG_SetHCINTMSK( U32  tNo, U32  tHCINTMSK)
{
	__g_pRegister[0]->HCSR16[8*tNo+3] = tHCINTMSK;
}

void OTG_SetHCTSIZ( U32  tNo, U32  tHCTSIZ)
{
	__g_pRegister[0]->HCSR16[8*tNo+4] = tHCTSIZ;
}

void OTG_SetHCDMA( U32  tNo, U32  tHCDMA)
{
	__g_pRegister[0]->HCSR16[8*tNo+5] = tHCDMA;
}

void OTG_SetDIEPCTL( U32  tNo, U32  tDIEPCTL)
{
	__g_pRegister[0]->DIESR16[8*tNo+0] = tDIEPCTL;
}

void OTG_SetDIEPINT( U32  tNo, U32  tDIEPINT)
{
	__g_pRegister[0]->DIESR16[8*tNo+2] = tDIEPINT;
}

void OTG_SetDIEPTSIZ( U32  tNo, U32  tDIEPTSIZ)
{
	__g_pRegister[0]->DIESR16[8*tNo+4] = tDIEPTSIZ;
}

void OTG_SetDIEPDMA( U32  tNo, U32  tDIEPDMA)
{
	__g_pRegister[0]->DIESR16[8*tNo+5] = tDIEPDMA;
}

void OTG_SetDOEPCTL( U32  tNo, U32  tDOEPCTL)
{
	__g_pRegister[0]->DOESR16[8*tNo+0] = tDOEPCTL;
}

void OTG_SetDOEPINT( U32  tNo, U32  tDOEPINT)
{
	__g_pRegister[0]->DOESR16[8*tNo+2] = tDOEPINT;
}

void OTG_SetDOEPTSIZ( U32  tNo, U32  tDOEPTSIZ)
{
	__g_pRegister[0]->DOESR16[8*tNo+4] = tDOEPTSIZ;
}

void OTG_SetDOEPDMA( U32  tNo, U32  tDOEPDMA)
{
	__g_pRegister[0]->DOESR16[8*tNo+5] = tDOEPDMA;
}

void OTG_SetHCSR ( U32  tNo, U32  tHCDMA, U32  tHCTSIZ, U32  tHCINTMSK,
						 	  U32  tHCINT, U32  tHCSPLT, U32  tHCCHAR)
{
	OTG_SetHCCHAR		(tNo, tHCCHAR	);
	OTG_SetHCSPLT		(tNo, tHCSPLT	);
	OTG_SetHCINT		(tNo, tHCINT		);
	OTG_SetHCINTMSK	    (tNo, tHCINTMSK	);
	OTG_SetHCTSIZ		(tNo, tHCTSIZ	);
	OTG_SetHCDMA		(tNo, tHCDMA		);
}


void OTG_SetDIESR ( U32  tNo, U32  tDIEPDMA, U32  tDIEPTSIZ, U32  tDIEPINT, U32  tDIEPCTL)
{
	OTG_SetDIEPCTL	    (tNo, tDIEPCTL	);
	OTG_SetDIEPINT	    (tNo, tDIEPINT	);
	OTG_SetDIEPTSIZ		(tNo, tDIEPTSIZ	);
	OTG_SetDIEPDMA	    (tNo, tDIEPDMA	);
}

void OTG_SetDOESR ( U32  tNo, U32  tDOEPDMA, U32  tDOEPTSIZ, U32  tDOEPINT, U32  tDOEPCTL)
{
	OTG_SetDOEPCTL	    (tNo, tDOEPCTL	);
	OTG_SetDOEPINT	    (tNo, tDOEPINT	);
	OTG_SetDOEPTSIZ		(tNo, tDOEPTSIZ	);
	OTG_SetDOEPDMA	    (tNo, tDOEPDMA	);
}

//------------------------------------------------------------------------------
//	Get Member Functions
//------------------------------------------------------------------------------
U32  OTG_GetGOTGCTL()
{
	return __g_pRegister[0]->GOTGCTL;
}

U32  OTG_GetGOTGINT()
{
	return __g_pRegister[0]->GOTGINT;
}

U32  OTG_GetGAHBCFG()
{
	return __g_pRegister[0]->GAHBCFG;
}

U32  OTG_GetGUSBCFG()
{
	return __g_pRegister[0]->GUSBCFG;
}

U32  OTG_GetGRSTCTL()
{
	return __g_pRegister[0]->GRSTCTL;
}

U32  OTG_GetGINTSTS()
{
	return __g_pRegister[0]->GINTSTS;
}

U32  OTG_GetGINTMSK()
{
	return __g_pRegister[0]->GINTMSK;
}

U32  OTG_GetGRXSTSR()
{
	return __g_pRegister[0]->GRXSTSR;
}

U32  OTG_GetGRXSTSP()
{
	return __g_pRegister[0]->GRXSTSP;
}

U32  OTG_GetGRXFSIZ()
{
	return __g_pRegister[0]->GRXFSIZ;
}

U32  OTG_GetGNPTXFSIZ()
{
	return __g_pRegister[0]->GNPTXFSIZ;
}

U32  OTG_GetGNPTXSTS()
{
	return __g_pRegister[0]->GNPTXSTS;
}

U32  OTG_GetGI2CCTL()
{
	return __g_pRegister[0]->GI2CCTL;
}

U32  OTG_GetGPVNDCTL()
{
	return __g_pRegister[0]->GPVNDCTL;
}

U32  OTG_GetGGPIO()
{
	return __g_pRegister[0]->GGPIO;
}

U32  OTG_GetGUID()
{
	return __g_pRegister[0]->GUID;
}

U32  OTG_GetGSNPSID()
{
	return __g_pRegister[0]->GSNPSID;
}

U32  OTG_GetGHWCFG1()
{
	return __g_pRegister[0]->GHWCFG1;
}

U32  OTG_GetGHWCFG2()
{
	return __g_pRegister[0]->GHWCFG2;
}

U32  OTG_GetGHWCFG3()
{
	return __g_pRegister[0]->GHWCFG3;
}

U32  OTG_GetGHWCFG4()
{
	return __g_pRegister[0]->GHWCFG4;
}

U32  OTG_GetGLPMCFG()
{
	return __g_pRegister[0]->GLPMCFG;
}

U32  OTG_GetHPTXFSIZ()
{
	return __g_pRegister[0]->HPTXFSIZ;
}

U32  OTG_GetHCFG()
{
	return __g_pRegister[0]->HCFG;
}

U32  OTG_GetHFIR()
{
	return __g_pRegister[0]->HFIR;
}

U32  OTG_GetHFNUM()
{
	return __g_pRegister[0]->HFNUM;
}

U32  OTG_GetHPTXSTS()
{
	return __g_pRegister[0]->HPTXSTS;
}

U32  OTG_GetHAINT()
{
	return __g_pRegister[0]->HAINT;
}

U32  OTG_GetHAINTMSK()
{
	return __g_pRegister[0]->HAINTMSK;
}

U32  OTG_GetHPRT()
{
	return __g_pRegister[0]->HPRT;
}

U32  OTG_GetDCFG()
{
	return __g_pRegister[0]->DCFG;
}

U32  OTG_GetDCTL()
{
	return __g_pRegister[0]->DCTL;
}

U32  OTG_GetDSTS()
{
	return __g_pRegister[0]->DSTS;
}

U32  OTG_GetDIEPMSK()
{
	return __g_pRegister[0]->DIEPMSK;
}

U32  OTG_GetDOEPMSK()
{
	return __g_pRegister[0]->DOEPMSK;
}

U32  OTG_GetDAINT()
{
	return __g_pRegister[0]->DAINT;
}

U32  OTG_GetDAINTMSK()
{
	return __g_pRegister[0]->DAINTMSK;
}

U32  OTG_GetDTKNQR1()
{
	return __g_pRegister[0]->DTKNQR1;
}

U32  OTG_GetDTKNQR2()
{
	return __g_pRegister[0]->DTKNQR2;
}

U32  OTG_GetDVBUSDIS()
{
	return __g_pRegister[0]->DVBUSDIS;
}

U32  OTG_GetDVBUSPULSE()
{
	return __g_pRegister[0]->DVBUSPULSE;
}

U32  OTG_GetDTKNQR3()
{
	return __g_pRegister[0]->DTKNQR3;
}

U32  OTG_GetDTKNQR4()
{
	return __g_pRegister[0]->DTKNQR4;
}

U32  OTG_GetPCGCCTL()
{
	return __g_pRegister[0]->PCGCCTL;
}

//U32  OTG_GetTESTPARM0()
//{
//	return __g_pRegister[0]->TESTPARM0;
//}
//
//U32  OTG_GetTESTPARM1()
//{
//	return __g_pRegister[0]->TESTPARM1;
//}
//
//U32  OTG_GetTESTPARM2()
//{
//	return __g_pRegister[0]->TESTPARM2;
//}
//
//U32  OTG_GetTESTPARM3()
//{
//	return __g_pRegister[0]->TESTPARM3;
//}
//
//U32  OTG_GetTESTPARM4()
//{
//	return __g_pRegister[0]->TESTPARM4;
//}
//
//U32  OTG_GetTESTPARM5()
//{
//	return __g_pRegister[0]->TESTPARM5;
//}
//
//U32  OTG_GetTESTPARM6()
//{
//	return __g_pRegister[0]->TESTPARM6;
//}
//
//U32  OTG_GetTESTPARM7()
//{
//	return __g_pRegister[0]->TESTPARM7;
//}
//
//U32  OTG_GetTESTPARM8()
//{
//	return __g_pRegister[0]->TESTPARM8	;
//}
//
//U32  OTG_GetTESTPARM9()
//{
//	return __g_pRegister[0]->TESTPARM9	;
//}
//
//U32  OTG_GetTESTPARM10()
//{
//	return __g_pRegister[0]->TESTPARM10	;
//}

//------------------------------------------------------------------------------
//DPTXFSIZ, HCSR, DIESR, DOESR
U32  OTG_GetDPTXFSIZ( U32  tNo)
{
	return __g_pRegister[0]->DPTXFSIZ[tNo];
}

U32  OTG_GetHCCHAR( U32  tNo)
{
	return __g_pRegister[0]->HCSR16[8*tNo+0];
}

U32  OTG_GetHCSPLT( U32  tNo)
{
	return __g_pRegister[0]->HCSR16[8*tNo+1];
}

U32  OTG_GetHCINT( U32  tNo)
{
	return __g_pRegister[0]->HCSR16[8*tNo+2];
}

U32  OTG_GetHCINTMSK( U32  tNo)
{
	return __g_pRegister[0]->HCSR16[8*tNo+3];
}

U32  OTG_GetHCTSIZ( U32  tNo)
{
	return __g_pRegister[0]->HCSR16[8*tNo+4];
}

U32  OTG_GetHCDMA( U32  tNo)
{
	return __g_pRegister[0]->HCSR16[8*tNo+5];
}

U32  OTG_GetDIEPCTL( U32  tNo)
{
	return __g_pRegister[0]->DIESR16[8*tNo+0];
}

U32  OTG_GetDIEPINT( U32  tNo)
{
	return __g_pRegister[0]->DIESR16[8*tNo+2];
}

U32  OTG_GetDIEPTSIZ( U32  tNo)
{
	return __g_pRegister[0]->DIESR16[8*tNo+4];
}

U32  OTG_GetDIEPDMA( U32  tNo)
{
	return __g_pRegister[0]->DIESR16[8*tNo+5];
}

U32  OTG_GetDOEPCTL( U32  tNo)
{
	return __g_pRegister[0]->DOESR16[8*tNo+0];
}

U32  OTG_GetDOEPINT( U32  tNo)
{
	return __g_pRegister[0]->DOESR16[8*tNo+2];
}

U32  OTG_GetDOEPTSIZ( U32  tNo)
{
	return __g_pRegister[0]->DOESR16[8*tNo+4];
}

U32  OTG_GetDOEPDMA( U32  tNo)
{
	return __g_pRegister[0]->DOESR16[8*tNo+5];
}

//------------------------------------------------------------------------------
// new member function
//------------------------------------------------------------------------------
void OTG_SetGPWRDN(U32 tGPWRDN){
	__g_pRegister[0]->GPWRDN=tGPWRDN;
}
void OTG_SetGDFIFOCFG(U32 tGDFIFOCFG){
	__g_pRegister[0]->GDFIFOCFG=tGDFIFOCFG;
}
void OTG_SetGADPCTL(U32 tGADPCTL){
	__g_pRegister[0]->GADPCTL=tGADPCTL;
}
void OTG_SetDEACHINT(U32 tDEACHINT){
	__g_pRegister[0]->DEACHINT=tDEACHINT;
}
void OTG_SetDEACHINTMSK(U32 tDEACHINTMSK){
	__g_pRegister[0]->DEACHINTMSK=tDEACHINTMSK;
}
U32  OTG_GetGPWRDN(){
	return __g_pRegister[0]->GPWRDN;
}
U32  OTG_GetGDFIFOCFG(){
	return __g_pRegister[0]->GDFIFOCFG;
}
U32  OTG_GetGADPCTL(){
	return __g_pRegister[0]->GADPCTL;
}
U32  OTG_GetDEACHINT(){
	return __g_pRegister[0]->DEACHINT;
}
U32  OTG_GetDEACHINTMSK(){
	return __g_pRegister[0]->DEACHINTMSK;
}

void OTG_SetDIEPEACHMSK(U32 tNo,U32 tDIEPEACHMSK){
	__g_pRegister[0]->DIEPEACHMSK[tNo]=tDIEPEACHMSK;
}
void OTG_SetDOEPEACHMSK(U32 tNo,U32 tDOEPEACHMSK){
	__g_pRegister[0]->DOEPEACHMSK[tNo]=tDOEPEACHMSK;
}
U32  OTG_GetDIEPEACHMSK(U32 tNo){
	return __g_pRegister[0]->DIEPEACHMSK[tNo];
}
U32  OTG_GetDOEPEACHMSK(U32 tNo){
	return __g_pRegister[0]->DOEPEACHMSK[tNo];
}

void OTG_SetEP00FIFO(U32 tNo,U32 tEP00FIFO){
	__g_pRegister[0]->EP00FIFO[tNo]=tEP00FIFO;
}
void OTG_SetEP01FIFO(U32 tNo,U32 tEP01FIFO){
	__g_pRegister[0]->EP01FIFO[tNo]=tEP01FIFO;
}
void OTG_SetEP02FIFO(U32 tNo,U32 tEP02FIFO){
	__g_pRegister[0]->EP02FIFO[tNo]=tEP02FIFO;
}
void OTG_SetEP03FIFO(U32 tNo,U32 tEP03FIFO){
	__g_pRegister[0]->EP03FIFO[tNo]=tEP03FIFO;
}
void OTG_SetEP04FIFO(U32 tNo,U32 tEP04FIFO){
	__g_pRegister[0]->EP04FIFO[tNo]=tEP04FIFO;
}
void OTG_SetEP05FIFO(U32 tNo,U32 tEP05FIFO){
	__g_pRegister[0]->EP05FIFO[tNo]=tEP05FIFO;
}
void OTG_SetEP06FIFO(U32 tNo,U32 tEP06FIFO){
	__g_pRegister[0]->EP06FIFO[tNo]=tEP06FIFO;
}
void OTG_SetEP07FIFO(U32 tNo,U32 tEP07FIFO){
	__g_pRegister[0]->EP07FIFO[tNo]=tEP07FIFO;
}
void OTG_SetEP08FIFO(U32 tNo,U32 tEP08FIFO){
	__g_pRegister[0]->EP08FIFO[tNo]=tEP08FIFO;
}
void OTG_SetEP09FIFO(U32 tNo,U32 tEP09FIFO){
	__g_pRegister[0]->EP09FIFO[tNo]=tEP09FIFO;
}
void OTG_SetEP10FIFO(U32 tNo,U32 tEP10FIFO){
	__g_pRegister[0]->EP10FIFO[tNo]=tEP10FIFO;
}
void OTG_SetEP11FIFO(U32 tNo,U32 tEP11FIFO){
	__g_pRegister[0]->EP11FIFO[tNo]=tEP11FIFO;
}
void OTG_SetEP12FIFO(U32 tNo,U32 tEP12FIFO){
	__g_pRegister[0]->EP12FIFO[tNo]=tEP12FIFO;
}
void OTG_SetEP13FIFO(U32 tNo,U32 tEP13FIFO){
	__g_pRegister[0]->EP13FIFO[tNo]=tEP13FIFO;
}
void OTG_SetEP14FIFO(U32 tNo,U32 tEP14FIFO){
	__g_pRegister[0]->EP14FIFO[tNo]=tEP14FIFO;
}
void OTG_SetEP15FIFO(U32 tNo,U32 tEP15FIFO){
	__g_pRegister[0]->EP15FIFO[tNo]=tEP15FIFO;
}
U32  OTG_GetEP00FIFO(U32 tNo){
	return __g_pRegister[0]->EP00FIFO[tNo];
}
U32  OTG_GetEP01FIFO(U32 tNo){
	return __g_pRegister[0]->EP01FIFO[tNo];
}
U32  OTG_GetEP02FIFO(U32 tNo){
	return __g_pRegister[0]->EP02FIFO[tNo];
}
U32  OTG_GetEP03FIFO(U32 tNo){
	return __g_pRegister[0]->EP03FIFO[tNo];
}
U32  OTG_GetEP04FIFO(U32 tNo){
	return __g_pRegister[0]->EP04FIFO[tNo];
}
U32  OTG_GetEP05FIFO(U32 tNo){
	return __g_pRegister[0]->EP05FIFO[tNo];
}
U32  OTG_GetEP06FIFO(U32 tNo){
	return __g_pRegister[0]->EP06FIFO[tNo];
}
U32  OTG_GetEP07FIFO(U32 tNo){
	return __g_pRegister[0]->EP07FIFO[tNo];
}
U32  OTG_GetEP08FIFO(U32 tNo){
	return __g_pRegister[0]->EP08FIFO[tNo];
}
U32  OTG_GetEP09FIFO(U32 tNo){
	return __g_pRegister[0]->EP09FIFO[tNo];
}
U32  OTG_GetEP10FIFO(U32 tNo){
	return __g_pRegister[0]->EP10FIFO[tNo];
}
U32  OTG_GetEP11FIFO(U32 tNo){
	return __g_pRegister[0]->EP11FIFO[tNo];
}
U32  OTG_GetEP12FIFO(U32 tNo){
	return __g_pRegister[0]->EP12FIFO[tNo];
}
U32  OTG_GetEP13FIFO(U32 tNo){
	return __g_pRegister[0]->EP13FIFO[tNo];
}
U32  OTG_GetEP14FIFO(U32 tNo){
	return __g_pRegister[0]->EP14FIFO[tNo];
}
U32  OTG_GetEP15FIFO(U32 tNo){
	return __g_pRegister[0]->EP15FIFO[tNo];
}

U32  OTG_GetDEBUGFIFO(U32 tNo){
	return __g_pRegister[0]->DEBUGFIFO[tNo];
}
void OTG_SetDEBUGFIFO(U32 tNo,U32 tDEBUGFIFO){
	__g_pRegister[0]->DEBUGFIFO[tNo]=tDEBUGFIFO;
}
void OTG_SetRESERVED10(U32 tNo,U32 value){
	__g_pRegister[0]->RESERVED10[tNo]=value;
}
U32  OTG_GetRESERVED10(U32 tNo){
	return __g_pRegister[0]->RESERVED10[tNo];
}

U32 xxx (void){
	return __g_pRegister[0]->GPWRDN;
}

    void SetOTG_APB00(U32 no,U32 value) { __g_pApbRegister[0]->OTG_APB00[no]= value; }
    U32 GetOTG_APB00 (U32 no) { return __g_pApbRegister[0]->OTG_APB00[no]; }
