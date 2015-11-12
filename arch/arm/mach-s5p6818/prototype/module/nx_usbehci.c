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
//	File		: nx_USB20HOST.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include "nx_usb20host.h"
#include <string.h> // for memset

static	NX_USB20HOST_RegisterSet *__g_pRegister[0];
static	NX_USB20HOST_OHCI_RegisterSet *__g_pOhciRegister[0];
static	NX_USB20HOST_APB_RegisterSet *__g_pApbRegister[0];

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
 *	@see	NX_USB20HOST_GetNumberOfModule
 */
CBOOL	NX_USB20HOST_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		memset( __g_pRegister, 0, sizeof(__g_pRegister) );
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. \n
 *				It is equal to NUMBER_OF_USB20HOST_MODULE in <nx_chip.h>.
 *	@see		NX_USB20HOST_Initialize
 */
U32		NX_USB20HOST_GetNumberOfModule( void )
{
	return NUMBER_OF_USB20HOST_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_USB20HOST_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_USB20HOST_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_USB20HOST_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
    if( ModuleIndex == 0 ) { __g_pRegister[0] = (NX_USB20HOST_RegisterSet *)BaseAddress; }
    if( ModuleIndex == 1 ) { __g_pOhciRegister[0] = (NX_USB20HOST_RegisterSet *)BaseAddress; }
    if( ModuleIndex == 2 ) { __g_pApbRegister[0] = (NX_USB20HOST_RegisterSet *)BaseAddress; }
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_USB20HOST_GetBaseAddress( U32 ModuleIndex )
{
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. \n
 *				It is equal to PHY_BASEADDR_USB20HOST?_MODULE in <nx_chip.h>.
 */
//@TODO : ehci, apb
U32		NX_USB20HOST_GetPhysicalAddress( U32 ModuleIndex )
{
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
    if( ModuleIndex == 0 ) { return PHY_BASEADDR_WITH_CHANNEL_LIST( USB20HOST, EHCI_S_ABH ); }
    if( ModuleIndex == 1 ) { return PHY_BASEADDR_WITH_CHANNEL_LIST( USB20HOST, OHCI_S_ABH ); }
    if( ModuleIndex == 2 ) { return PHY_BASEADDR_WITH_CHANNEL_LIST( USB20HOST, APB ); }

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_USB20HOST_OpenModule( U32 ModuleIndex )
{
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_USB20HOST_CloseModule( U32 ModuleIndex )
{
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_USB20HOST_CheckBusy( U32 ModuleIndex )
{
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
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
 *	@return		Module's clock index.\n
 *				It is equal to CLOCKINDEX_OF_USB20HOST?_MODULE in <nx_chip.h>.
 *	@see		NX_CLKGEN_SetClockDivisorEnable,
 *				NX_CLKGEN_GetClockDivisorEnable,
 *				NX_CLKGEN_SetClockSource,
 *				NX_CLKGEN_GetClockSource,
 *				NX_CLKGEN_SetClockDivisor,
 *				NX_CLKGEN_GetClockDivisor
 */
U32 NX_USB20HOST_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( USB20HOST )
	};
	NX_CASSERT( NUMBER_OF_USB20HOST_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_USB20HOST?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_USB20HOST_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( USB20HOST, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_USB20HOST_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    //NX_ASSERT( NUMBER_OF_USB20HOST_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
//	Set Member Functions
//------------------------------------------------------------------------------
void SetHCCAPBASE (U32 tHCCAPBASE)
{
    __g_pRegister[0]->HCCAPBASE = tHCCAPBASE;
}

void SetHCSPARAMS (U32 tHCSPARAMS)
{
    __g_pRegister[0]->HCSPARAMS = tHCSPARAMS;
}

void SetHCCPARAMS (U32 tHCCPARAMS)
{
    __g_pRegister[0]->HCCPARAMS = tHCCPARAMS;
}

void SetUSBCMD (U32 tUSBCMD)
{
    __g_pRegister[0]->USBCMD = tUSBCMD;
}

void SetUSBSTS (U32 tUSBSTS)
{
    __g_pRegister[0]->USBSTS = tUSBSTS;
}

void SetUSBINTR (U32 tUSBINTR)
{
    __g_pRegister[0]->USBINTR = tUSBINTR;
}

void SetFRINDEX (U32 tFRINDEX)
{
    __g_pRegister[0]->FRINDEX = tFRINDEX;
}

void SetCTRLDSSEGMENT (U32 tCTRLDSSEGMENT)
{
    __g_pRegister[0]->CTRLDSSEGMENT = tCTRLDSSEGMENT;
}

void SetPERIODICLISTBASE(U32 tPERIODICLISTBASE)
{
    __g_pRegister[0]->PERIODICLISTBASE = tPERIODICLISTBASE;
}

void SetASYNCLISTADDR (U32 tASYNCLISTADDR)
{
    __g_pRegister[0]->ASYNCLISTADDR = tASYNCLISTADDR;
}

void SetCONFIGFLAG (U32 tCONFIGFLAG)
{
    __g_pRegister[0]->CONFIGFLAG = tCONFIGFLAG;
}

void SetPORTSC (U32 tPORTSC)
{
    __g_pRegister[0]->PORTSC = tPORTSC;
}

void SetINSNREG00 (U32 tINSNREG00)
{
    __g_pRegister[0]->INSNREG00 = tINSNREG00;
}

void SetINSNREG01 (U32 tINSNREG01)
{
    __g_pRegister[0]->INSNREG01 = tINSNREG01;
}

void SetINSNREG02 (U32 tINSNREG02)
{
    __g_pRegister[0]->INSNREG02 = tINSNREG02;
}

void SetINSNREG03 (U32 tINSNREG03)
{
    __g_pRegister[0]->INSNREG03 = tINSNREG03;
}

void SetINSNREG04 (U32 tINSNREG04)
{
    __g_pRegister[0]->INSNREG04 = tINSNREG04;
}

void SetINSNREG05 (U32 tINSNREG05)
{
    __g_pRegister[0]->INSNREG05 = tINSNREG05;
}
void SetINSNREG06 (U32 value)
{
    __g_pRegister[0]->INSNREG06 = value;
}
void SetINSNREG07 (U32 value)
{
    __g_pRegister[0]->INSNREG07 = value;
}
void SetINSNREG08 (U32 value)
{
    __g_pRegister[0]->INSNREG08 = value;
}

//------------------------------------------------------------------------------
//	Get Member Functions
//------------------------------------------------------------------------------
U32 GetHCCAPBASE (void)
{
    return __g_pRegister[0]->HCCAPBASE;
}

U32 GetHCSPARAMS (void)
{
    return __g_pRegister[0]->HCSPARAMS;
}

U32 GetHCCPARAMS (void)
{
    return __g_pRegister[0]->HCCPARAMS;
}

U32 GetUSBCMD (void)
{
    return __g_pRegister[0]->USBCMD;
}

U32 GetUSBSTS (void)
{
    return __g_pRegister[0]->USBSTS;
}

U32 GetUSBINTR (void)
{
    return __g_pRegister[0]->USBINTR;
}

U32 GetFRINDEX (void)
{
    return __g_pRegister[0]->FRINDEX;
}

U32 GetCTRLDSSEGMENT (void)
{
    return __g_pRegister[0]->CTRLDSSEGMENT;
}

U32 GetPERIODICLISTBASE	(void)
{
    return __g_pRegister[0]->PERIODICLISTBASE;
}

U32 GetASYNCLISTADDR (void)
{
    return __g_pRegister[0]->ASYNCLISTADDR;
}

U32 GetCONFIGFLAG(void)
{
    return __g_pRegister[0]->CONFIGFLAG;
}

U32 GetPORTSC (void)
{
    return __g_pRegister[0]->PORTSC;
}

U32 GetINSNREG00 (void)
{
    return __g_pRegister[0]->INSNREG00;
}

U32 GetINSNREG01 (void)
{
    return __g_pRegister[0]->INSNREG01;
}

U32 GetINSNREG02 (void)
{
    return __g_pRegister[0]->INSNREG02;
}

U32 GetINSNREG03 (void)
{
    return __g_pRegister[0]->INSNREG03;
}

U32 GetINSNREG04 (void)
{
    return __g_pRegister[0]->INSNREG04;
}

U32 GetINSNREG05 (void)
{
    return __g_pRegister[0]->INSNREG05;
}
U32 GetINSNREG06 (void)
{
    return __g_pRegister[0]->INSNREG06;
}
U32 GetINSNREG07 (void)
{
    return __g_pRegister[0]->INSNREG07;
}
U32 GetINSNREG08 (void)
{
    return __g_pRegister[0]->INSNREG08;
}



    void SetHcRevision				(U32 value) { __g_pOhciRegister[0]->HcRevision			= value; }
    void SetHcControl               (U32 value) { __g_pOhciRegister[0]->HcControl           = value; }
    void SetHcCommandStatus         (U32 value) { __g_pOhciRegister[0]->HcCommandStatus     = value; }
    void SetHcInterruptStatus       (U32 value) { __g_pOhciRegister[0]->HcInterruptStatus   = value; }
    void SetHcInterruptEnable       (U32 value) { __g_pOhciRegister[0]->HcInterruptEnable   = value; }
    void SetHcInterruptDisable      (U32 value) { __g_pOhciRegister[0]->HcInterruptDisable  = value; }
    void SetHcHCCA                  (U32 value) { __g_pOhciRegister[0]->HcHCCA              = value; }
    void SetHcPeriodCurrentED       (U32 value) { __g_pOhciRegister[0]->HcPeriodCurrentED   = value; }
    void SetHcControlHeadED         (U32 value) { __g_pOhciRegister[0]->HcControlHeadED     = value; }
    void SetHcControlCurrentED      (U32 value) { __g_pOhciRegister[0]->HcControlCurrentED  = value; }
    void SetHcBulkHeadED            (U32 value) { __g_pOhciRegister[0]->HcBulkHeadED        = value; }
    void SetHcBulkCurrentED         (U32 value) { __g_pOhciRegister[0]->HcBulkCurrentED     = value; }
    void SetHcDoneHead              (U32 value) { __g_pOhciRegister[0]->HcDoneHead          = value; }
    void SetHcFmInterval            (U32 value) { __g_pOhciRegister[0]->HcFmInterval        = value; }
    void SetHcFmRemaining           (U32 value) { __g_pOhciRegister[0]->HcFmRemaining       = value; }
    void SetHcFmNumber              (U32 value) { __g_pOhciRegister[0]->HcFmNumber          = value; }
    void SetHcPeriodicStart         (U32 value) { __g_pOhciRegister[0]->HcPeriodicStart     = value; }
    void SetHcLSThreshold           (U32 value) { __g_pOhciRegister[0]->HcLSThreshold       = value; }
    void SetHcRhDescriptorA         (U32 value) { __g_pOhciRegister[0]->HcRhDescriptorA     = value; }
    void SetHcRhDescriptorB         (U32 value) { __g_pOhciRegister[0]->HcRhDescriptorB     = value; }
    void SetHcRhStatus              (U32 value) { __g_pOhciRegister[0]->HcRhStatus          = value; }
    void SetHcRhPortStatus          (U32 value) { __g_pOhciRegister[0]->HcRhPortStatus      = value; }


    U32 GetHcRevision				(void) { return __g_pOhciRegister[0]->HcRevision			; }
    U32 GetHcControl               	(void) { return __g_pOhciRegister[0]->HcControl             ; }
    U32 GetHcCommandStatus         	(void) { return __g_pOhciRegister[0]->HcCommandStatus       ; }
    U32 GetHcInterruptStatus       	(void) { return __g_pOhciRegister[0]->HcInterruptStatus     ; }
    U32 GetHcInterruptEnable       	(void) { return __g_pOhciRegister[0]->HcInterruptEnable     ; }
    U32 GetHcInterruptDisable      	(void) { return __g_pOhciRegister[0]->HcInterruptDisable    ; }
    U32 GetHcHCCA                  	(void) { return __g_pOhciRegister[0]->HcHCCA                ; }
    U32 GetHcPeriodCurrentED       	(void) { return __g_pOhciRegister[0]->HcPeriodCurrentED     ; }
    U32 GetHcControlHeadED         	(void) { return __g_pOhciRegister[0]->HcControlHeadED       ; }
    U32 GetHcControlCurrentED      	(void) { return __g_pOhciRegister[0]->HcControlCurrentED    ; }
    U32 GetHcBulkHeadED            	(void) { return __g_pOhciRegister[0]->HcBulkHeadED          ; }
    U32 GetHcBulkCurrentED         	(void) { return __g_pOhciRegister[0]->HcBulkCurrentED       ; }
    U32 GetHcDoneHead              	(void) { return __g_pOhciRegister[0]->HcDoneHead            ; }
    U32 GetHcFmInterval            	(void) { return __g_pOhciRegister[0]->HcFmInterval          ; }
    U32 GetHcFmRemaining           	(void) { return __g_pOhciRegister[0]->HcFmRemaining         ; }
    U32 GetHcFmNumber              	(void) { return __g_pOhciRegister[0]->HcFmNumber            ; }
    U32 GetHcPeriodicStart         	(void) { return __g_pOhciRegister[0]->HcPeriodicStart       ; }
    U32 GetHcLSThreshold           	(void) { return __g_pOhciRegister[0]->HcLSThreshold         ; }
    U32 GetHcRhDescriptorA         	(void) { return __g_pOhciRegister[0]->HcRhDescriptorA       ; }
    U32 GetHcRhDescriptorB         	(void) { return __g_pOhciRegister[0]->HcRhDescriptorB       ; }
    U32 GetHcRhStatus              	(void) { return __g_pOhciRegister[0]->HcRhStatus            ; }
    U32 GetHcRhPortStatus          	(void) { return __g_pOhciRegister[0]->HcRhPortStatus        ; }

    void SetAPB00(U32 no,U32 value) { __g_pApbRegister[0]->APB00[no]= value; }
    U32 GetAPB00 (U32 no) { return __g_pApbRegister[0]->APB00[no]; }
