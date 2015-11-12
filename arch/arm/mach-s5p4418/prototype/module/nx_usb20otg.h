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
//	File		: nx_USB20OTG.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_USB20OTG_H__
#define __NX_USB20OTG_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	USB20OTG
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
	volatile U32	GOTGCTL 		;	//000h							//000
	volatile U32	GOTGINT 		;	//004h      					//001
	volatile U32	GAHBCFG 		;	//008h      					//002
	volatile U32	GUSBCFG 		;	//00Ch      					//003
	volatile U32	GRSTCTL 		;	//010h      					//004
	volatile U32	GINTSTS 		;	//014h      					//005
	volatile U32	GINTMSK 		;	//018h      					//006
	volatile U32	GRXSTSR 		;	//01Ch      					//007
	volatile U32	GRXSTSP 		;	//020h      					//008
	volatile U32	GRXFSIZ 		;	//024h      					//009
	volatile U32	GNPTXFSIZ 		;	//028h      					//00A
	volatile U32	GNPTXSTS 		;	//02Ch      					//00B
	volatile U32	GI2CCTL 		;	//030h      					//00C
	volatile U32	GPVNDCTL 		;	//034h      					//00D
	volatile U32	GGPIO 			;	//038h      					//00E
	volatile U32	GUID 			;	//03Ch      					//00F
	volatile U32	GSNPSID 		;	//040h      					//010
	volatile U32	GHWCFG1 		;	//044h      					//011
	volatile U32	GHWCFG2 		;	//048h      					//012
	volatile U32	GHWCFG3 		;	//04Ch      					//013
	volatile U32	GHWCFG4 		;	//050h      					//014
	volatile U32	GLPMCFG 		;	//054h      					//015
	volatile U32	GPWRDN 			;	//058h							//016	//new
	volatile U32	GDFIFOCFG 		;	//05Ch                          //017	//new
	volatile U32	GADPCTL 		;	//060h                          //018	//new

	volatile U32	RESERVED00[39]	;	//064h-0FFh 					//019
	volatile U32	HPTXFSIZ 		;	//100h      					//040
	volatile U32	DPTXFSIZ[15]	;	//104h-13Ch 					//041
	volatile U32	RESERVED01[176]	;	//140h-3FFh 					//050
										//Host Mode CSR Map (400h-7FFh)
	volatile U32	HCFG 			;	//400h							//100
	volatile U32	HFIR 			;	//404h      					//101
	volatile U32	HFNUM 			;	//408h      					//102
	volatile U32	RESERVED02		;	//40Ch      					//103
	volatile U32	HPTXSTS 		;	//410h      					//104
	volatile U32	HAINT 			;	//414h      					//105
	volatile U32	HAINTMSK 		;	//418h      					//106
	volatile U32	RESERVED03[9]	;	//41Ch-43Ch	24h=36
	volatile U32	HPRT 			;	//440h							//110
	volatile U32	RESERVED04[47]	;	//444h-4FCh	BCh=188=47*4
	volatile U32	HCSR16[128]		;	//500h-6FCh						//140
										//16 channel*(6개+2개)=128, 128*4=512=200h,
										//{Reserved[2],HCDMA,HCTSIZ,HCINTMSK,HCINT,HCSPLT,HCCHAR}
	volatile U32	RESERVED05[64]	;	//700h-7FCh
										//Device Mode CSR Map (800h-BFFh), 800h-ACh : Device Logical IN Endpoint-Specific Registers
	volatile U32	DCFG 			;	//800h							//200
	volatile U32	DCTL 			;	//804h                          //201
	volatile U32	DSTS 			;	//808h                          //202
	volatile U32	RESERVED06		;	//80Ch                          //203
	volatile U32	DIEPMSK 		;	//810h                          //204
	volatile U32	DOEPMSK 		;	//814h                          //205
	volatile U32	DAINT 			;	//818h                          //206
	volatile U32	DAINTMSK 		;	//81Ch                          //207
	volatile U32	DTKNQR1 		;	//820h                          //208
	volatile U32	DTKNQR2 		;	//824h                          //209
	volatile U32	DVBUSDIS 		;	//828h                          //20A
	volatile U32	DVBUSPULSE 		;	//82Ch                          //20B
	volatile U32	DTKNQR3 		;	//830h                          //20C
	volatile U32	DTKNQR4 		;	//834h                          //20D
	volatile U32	DEACHINT 		;	//838h							//20E	//new
	volatile U32	DEACHINTMSK 	;	//83Ch                          //20F	//new
	volatile U32	DIEPEACHMSK[16]	;	//840h-87Ch                     //210	//new
	volatile U32	DOEPEACHMSK[16]	;	//880h-8C0h                     //220	//new

	volatile U32	RESERVED07[16]	;	//8C0h-8FCh
	volatile U32	DIESR16[128]	;	//900h-AFCh	                    //240
										//16 channel*(4개+4개)=128, 128*4=512=200h,
										//{Reserved,DTXFSTS,DIEPDMA,DIEPTSIZ,Reserved,DIEPINT,Reserved,DIEPCTL}
	volatile U32	DOESR16[128]	;	//B00h-CFCh						//2C0
										//16 channel*(4개+4개)=128, 128*4=512=200h,
										//{Reserved[2],DOEPDMA,DOEPTSIZ,Reserved,DOEPINT,Reserved,DOEPCTL}
	volatile U32	RESERVED08[64]	;	//D00h-DFCh
										//Power and Clock Gating Register
	volatile U32	PCGCCTL 		;	//E00h							//380
	volatile U32	RESERVED09[127]	;	//E04h-FFCh
										//ep fifo Register
	volatile U32	EP00FIFO[1024]	;	//1000h
	volatile U32	EP01FIFO[1024]	;	//2000h
	volatile U32	EP02FIFO[1024]	;	//3000h
	volatile U32	EP03FIFO[1024]	;	//4000h
	volatile U32	EP04FIFO[1024]	;	//5000h
	volatile U32	EP05FIFO[1024]	;	//6000h
	volatile U32	EP06FIFO[1024]	;	//7000h
	volatile U32	EP07FIFO[1024]	;	//8000h
	volatile U32	EP08FIFO[1024]	;	//9000h
	volatile U32	EP09FIFO[1024]	;	//A000h
	volatile U32	EP10FIFO[1024]	;	//B000h
	volatile U32	EP11FIFO[1024]	;	//C000h
	volatile U32	EP12FIFO[1024]	;	//D000h
	volatile U32	EP13FIFO[1024]	;	//E000h
	volatile U32	EP14FIFO[1024]	;	//F000h
	volatile U32	EP15FIFO[1024]	;	//10000h
	volatile U32	RESERVED10[15360];	//11000h-1FFFCh
	volatile U32	DEBUGFIFO[32768];	//20000h-3FFFCh
										//User Register
//    volatile U32 	TESTPARM0		;	//1840h							//610
//    volatile U32 	TESTPARM1		;	//1844h							//611
//    volatile U32 	TESTPARM2		;	//1848h                         //612
//    volatile U32 	TESTPARM3		;	//184Ch                         //613
//    volatile U32 	TESTPARM4		;	//1850h                         //614
//    volatile U32 	TESTPARM5		;	//1854h                         //615
//    volatile U32 	TESTPARM6		;	//1858h                         //616
//    volatile U32 	TESTPARM7		;	//185Ch                         //617
//    volatile U32 	TESTPARM8		;	//1860h                         //618
//    volatile U32 	TESTPARM9		;	//1864h                         //619
//    volatile U32 	TESTPARM10		;	//1868h                         //620
} NX_USB20OTG_RegisterSet;

typedef struct
{
    volatile U32 OTG_APB00[39];
} NX_USB20OTG_APB_RegisterSet;

//------------------------------------------------------------------------------
///	@name	USB20OTG Interface
//------------------------------------------------------------------------------
//@{
U32   NX_USB20OTG_GetTEMP( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_USB20OTG_Initialize( void );
U32   NX_USB20OTG_GetNumberOfModule( void );

U32   NX_USB20OTG_GetSizeOfRegisterSet( void );
void  NX_USB20OTG_SetBaseAddress(  U32 ModuleIndex, void* BaseAddress );
void*  NX_USB20OTG_GetBaseAddress( U32 ModuleIndex );
U32   NX_USB20OTG_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_USB20OTG_OpenModule(  );
CBOOL NX_USB20OTG_CloseModule(  );
CBOOL NX_USB20OTG_CheckBusy(  );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_USB20OTG_GetClockNumber ( void );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_USB20OTG_GetResetNumber ( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
    NX_USB20OTG_INT_TEST0 = 0, ///< test0 interrupt
    NX_USB20OTG_INT_TEST1 = 1, ///< test1 interrupt
    NX_USB20OTG_INT_TEST2 = 2, ///< test2 interrupt
} NX_USB20OTG_INT;
void  NX_USB20OTG_SetInterruptEnableAll(  CBOOL Enable );
CBOOL NX_USB20OTG_GetInterruptEnableAll( void );
CBOOL NX_USB20OTG_GetInterruptPendingAll( void );
void  NX_USB20OTG_ClearInterruptPendingAll( void );
S32   NX_USB20OTG_GetInterruptPendingNumber( void );
//@}

//------------------------------------------------------------------------------
///	@name	Member function
//------------------------------------------------------------------------------
//@{
void OTG_SetGOTGCTL		( U32 tGOTGCTL     	);
void OTG_SetGOTGINT     ( U32 tGOTGINT     	);
void OTG_SetGAHBCFG     ( U32 tGAHBCFG     	);
void OTG_SetGUSBCFG     ( U32 tGUSBCFG     	);
void OTG_SetGRSTCTL     ( U32 tGRSTCTL     	);
void OTG_SetGINTSTS     ( U32 tGINTSTS     	);
void OTG_SetGINTMSK     ( U32 tGINTMSK     	);
void OTG_SetGRXSTSR     ( U32 tGRXSTSR     	);
void OTG_SetGRXSTSP     ( U32 tGRXSTSP     	);
void OTG_SetGRXFSIZ     ( U32 tGRXFSIZ     	);
void OTG_SetGNPTXFSIZ   ( U32 tGNPTXFSIZ   	);
void OTG_SetGNPTXSTS    ( U32 tGNPTXSTS    	);
void OTG_SetGI2CCTL     ( U32 tGI2CCTL     	);
void OTG_SetGPVNDCTL    ( U32 tGPVNDCTL    	);
void OTG_SetGGPIO       ( U32 tGGPIO       	);
void OTG_SetGUID        ( U32 tGUID        	);
void OTG_SetGSNPSID     ( U32 tGSNPSID     	);
void OTG_SetGHWCFG1     ( U32 tGHWCFG1     	);
void OTG_SetGHWCFG2     ( U32 tGHWCFG2     	);
void OTG_SetGHWCFG3     ( U32 tGHWCFG3     	);
void OTG_SetGHWCFG4     ( U32 tGHWCFG4     	);
void OTG_SetGLPMCFG     ( U32 tGLPMCFG     	);
void OTG_SetHPTXFSIZ    ( U32 tHPTXFSIZ    	);
void OTG_SetHCFG        ( U32 tHCFG        	);
void OTG_SetHFIR        ( U32 tHFIR        	);
void OTG_SetHFNUM       ( U32 tHFNUM       	);
void OTG_SetHPTXSTS     ( U32 tHPTXSTS     	);
void OTG_SetHAINT       ( U32 tHAINT       	);
void OTG_SetHAINTMSK    ( U32 tHAINTMSK    	);
void OTG_SetHPRT        ( U32 tHPRT        	);
void OTG_SetDCFG        ( U32 tDCFG        	);
void OTG_SetDCTL        ( U32 tDCTL        	);
void OTG_SetDSTS        ( U32 tDSTS        	);
void OTG_SetDIEPMSK     ( U32 tDIEPMSK     	);
void OTG_SetDOEPMSK     ( U32 tDOEPMSK     	);
void OTG_SetDAINT       ( U32 tDAINT       	);
void OTG_SetDAINTMSK    ( U32 tDAINTMSK    	);
void OTG_SetDTKNQR1     ( U32 tDTKNQR1     	);
void OTG_SetDTKNQR2     ( U32 tDTKNQR2     	);
void OTG_SetDVBUSDIS    ( U32 tDVBUSDIS    	);
void OTG_SetDVBUSPULSE  ( U32 tDVBUSPULSE  	);
void OTG_SetDTKNQR3     ( U32 tDTKNQR3     	);
void OTG_SetDTKNQR4     ( U32 tDTKNQR4     	);
void OTG_SetPCGCCTL     ( U32 tPCGCCTL     	);
//void OTG_SetTESTPARM0   ( U32 tTESTPARM0   	);
//void OTG_SetTESTPARM1   ( U32 tTESTPARM1   	);
//void OTG_SetTESTPARM2   ( U32 tTESTPARM2   	);
//void OTG_SetTESTPARM3   ( U32 tTESTPARM3   	);
//void OTG_SetTESTPARM4   ( U32 tTESTPARM4   	);
//void OTG_SetTESTPARM5   ( U32 tTESTPARM5   	);
//void OTG_SetTESTPARM6   ( U32 tTESTPARM6   	);
//void OTG_SetTESTPARM7   ( U32 tTESTPARM7   	);
//void OTG_SetTESTPARM8   ( U32 tTESTPARM8		);
//void OTG_SetTESTPARM9   ( U32 tTESTPARM9		);
//void OTG_SetTESTPARM10  ( U32 tTESTPARM10	);

void OTG_SetDPTXFSIZ   	( U32 tNo, U32 tDPTXFSIZ	);

//{Reserved[2],HCDMA,HCTSIZ,HCINTMSK,HCINT,HCSPLT,HCCHAR}
void OTG_SetHCSR		( U32 tNo, U32 tHCDMA, U32 tHCTSIZ, U32 tHCINTMSK,
					 U32 tHCINT, U32 tHCSPLT, U32 tHCCHAR);
void OTG_SetHCCHAR		( U32 tNo, U32 tHCCHAR	);
void OTG_SetHCSPLT		( U32 tNo, U32 tHCSPLT	);
void OTG_SetHCINT		( U32 tNo, U32 tHCINT	);
void OTG_SetHCINTMSK	( U32 tNo, U32 tHCINTMSK	);
void OTG_SetHCTSIZ		( U32 tNo, U32 tHCTSIZ	);
void OTG_SetHCDMA		( U32 tNo, U32 tHCDMA	);

//{Reserved[2],DIEPDMA,DIEPTSIZ,Reserved,DIEPINT,Reserved,DIEPCTL}
void OTG_SetDIESR		( U32 tNo, U32 tDIEPDMA, U32 tDIEPTSIZ, U32 tDIEPINT, U32 tDIEPCTL);
void OTG_SetDIEPCTL	    ( U32 tNo, U32 tDIEPCTL	);
void OTG_SetDIEPINT	    ( U32 tNo, U32 tDIEPINT	);
void OTG_SetDIEPTSIZ	( U32 tNo, U32 tDIEPTSIZ	);
void OTG_SetDIEPDMA	    ( U32 tNo, U32 tDIEPDMA	);

//{Reserved[2],DOEPDMA,DOEPTSIZ,Reserved,DOEPINT,Reserved,DOEPCTL}
void OTG_SetDOESR		( U32 tNo, U32 tDOEPDMA, U32 tDOEPTSIZ, U32 tDOEPINT, U32 tDOEPCTL);
void OTG_SetDOEPCTL	    ( U32 tNo, U32 tDOEPCTL	);
void OTG_SetDOEPINT	    ( U32 tNo, U32 tDOEPINT	);
void OTG_SetDOEPTSIZ	( U32 tNo, U32 tDOEPTSIZ	);
void OTG_SetDOEPDMA	    ( U32 tNo, U32 tDOEPDMA	);

U32 OTG_GetGOTGCTL      ( void );
U32 OTG_GetGOTGINT      ( void );
U32 OTG_GetGAHBCFG      ( void );
U32 OTG_GetGUSBCFG      ( void );
U32 OTG_GetGRSTCTL      ( void );
U32 OTG_GetGINTSTS      ( void );
U32 OTG_GetGINTMSK      ( void );
U32 OTG_GetGRXSTSR      ( void );
U32 OTG_GetGRXSTSP      ( void );
U32 OTG_GetGRXFSIZ      ( void );
U32 OTG_GetGNPTXFSIZ    ( void );
U32 OTG_GetGNPTXSTS     ( void );
U32 OTG_GetGI2CCTL      ( void );
U32 OTG_GetGPVNDCTL     ( void );
U32 OTG_GetGGPIO        ( void );
U32 OTG_GetGUID         ( void );
U32 OTG_GetGSNPSID      ( void );
U32 OTG_GetGHWCFG1      ( void );
U32 OTG_GetGHWCFG2      ( void );
U32 OTG_GetGHWCFG3      ( void );
U32 OTG_GetGHWCFG4      ( void );
U32 OTG_GetGLPMCFG      ( void );
U32 OTG_GetHPTXFSIZ     ( void );
U32 OTG_GetHCFG         ( void );
U32 OTG_GetHFIR         ( void );
U32 OTG_GetHFNUM        ( void );
U32 OTG_GetHPTXSTS      ( void );
U32 OTG_GetHAINT        ( void );
U32 OTG_GetHAINTMSK     ( void );
U32 OTG_GetHPRT         ( void );
U32 OTG_GetDCFG         ( void );
U32 OTG_GetDCTL         ( void );
U32 OTG_GetDSTS         ( void );
U32 OTG_GetDIEPMSK      ( void );
U32 OTG_GetDOEPMSK      ( void );
U32 OTG_GetDAINT        ( void );
U32 OTG_GetDAINTMSK     ( void );
U32 OTG_GetDTKNQR1      ( void );
U32 OTG_GetDTKNQR2      ( void );
U32 OTG_GetDVBUSDIS     ( void );
U32 OTG_GetDVBUSPULSE   ( void );
U32 OTG_GetDTKNQR3      ( void );
U32 OTG_GetDTKNQR4      ( void );
U32 OTG_GetPCGCCTL      ( void );
//U32 OTG_GetTESTPARM0    ();
//U32 OTG_GetTESTPARM1    ();
//U32 OTG_GetTESTPARM2    ();
//U32 OTG_GetTESTPARM3    ();
//U32 OTG_GetTESTPARM4    ();
//U32 OTG_GetTESTPARM5    ();
//U32 OTG_GetTESTPARM6    ();
//U32 OTG_GetTESTPARM7    ();
//U32 OTG_GetTESTPARM8	();
//U32 OTG_GetTESTPARM9	();
//U32 OTG_GetTESTPARM10	();

U32 OTG_GetDPTXFSIZ    	( U32 tNo);

U32 OTG_GetHCCHAR		( U32 tNo);
U32 OTG_GetHCSPLT		( U32 tNo);
U32 OTG_GetHCINT		( U32 tNo);
U32 OTG_GetHCINTMSK	    ( U32 tNo);
U32 OTG_GetHCTSIZ		( U32 tNo);
U32 OTG_GetHCDMA		( U32 tNo);

U32 OTG_GetDIEPCTL	    ( U32 tNo);
U32 OTG_GetDIEPINT	    ( U32 tNo);
U32 OTG_GetDIEPTSIZ		( U32 tNo);
U32 OTG_GetDIEPDMA	    ( U32 tNo);

U32 OTG_GetDOEPCTL	    ( U32 tNo);
U32 OTG_GetDOEPINT	    ( U32 tNo);
U32 OTG_GetDOEPTSIZ		( U32 tNo);
U32 OTG_GetDOEPDMA	    ( U32 tNo);

//new
void OTG_SetGPWRDN 		( U32 tGPWRDN 		);
void OTG_SetGDFIFOCFG 	( U32 tGDFIFOCFG 	);
void OTG_SetGADPCTL 	( U32 tGADPCTL 	    );
void OTG_SetDEACHINT 	( U32 tDEACHINT 	);
void OTG_SetDEACHINTMSK ( U32 tDEACHINTMSK	);
U32 OTG_GetGPWRDN 		( void );
U32 OTG_GetGDFIFOCFG 	( void );
U32 OTG_GetGADPCTL 		( void );
U32 OTG_GetDEACHINT 	( void );
U32 OTG_GetDEACHINTMSK 	( void );

void OTG_SetDIEPEACHMSK	( U32 tNo, U32 tDIEPEACHMSK	);
void OTG_SetDOEPEACHMSK	( U32 tNo, U32 tDOEPEACHMSK	);
U32 OTG_GetDIEPEACHMSK	( U32 tNo);
U32 OTG_GetDOEPEACHMSK	( U32 tNo);

void OTG_SetEP00FIFO	( U32 tNo, U32 tEP00FIFO	);
void OTG_SetEP01FIFO	( U32 tNo, U32 tEP01FIFO	);
void OTG_SetEP02FIFO	( U32 tNo, U32 tEP02FIFO	);
void OTG_SetEP03FIFO	( U32 tNo, U32 tEP03FIFO	);
void OTG_SetEP04FIFO	( U32 tNo, U32 tEP04FIFO	);
void OTG_SetEP05FIFO	( U32 tNo, U32 tEP05FIFO	);
void OTG_SetEP06FIFO	( U32 tNo, U32 tEP06FIFO	);
void OTG_SetEP07FIFO	( U32 tNo, U32 tEP07FIFO	);
void OTG_SetEP08FIFO	( U32 tNo, U32 tEP08FIFO	);
void OTG_SetEP09FIFO	( U32 tNo, U32 tEP09FIFO	);
void OTG_SetEP10FIFO	( U32 tNo, U32 tEP10FIFO	);
void OTG_SetEP11FIFO	( U32 tNo, U32 tEP11FIFO	);
void OTG_SetEP12FIFO	( U32 tNo, U32 tEP12FIFO	);
void OTG_SetEP13FIFO	( U32 tNo, U32 tEP13FIFO	);
void OTG_SetEP14FIFO	( U32 tNo, U32 tEP14FIFO	);
void OTG_SetEP15FIFO	( U32 tNo, U32 tEP15FIFO	);
U32 OTG_GetEP00FIFO		( U32 tNo);
U32 OTG_GetEP01FIFO		( U32 tNo);
U32 OTG_GetEP02FIFO		( U32 tNo);
U32 OTG_GetEP03FIFO		( U32 tNo);
U32 OTG_GetEP04FIFO		( U32 tNo);
U32 OTG_GetEP05FIFO		( U32 tNo);
U32 OTG_GetEP06FIFO		( U32 tNo);
U32 OTG_GetEP07FIFO		( U32 tNo);
U32 OTG_GetEP08FIFO		( U32 tNo);
U32 OTG_GetEP09FIFO		( U32 tNo);
U32 OTG_GetEP10FIFO		( U32 tNo);
U32 OTG_GetEP11FIFO		( U32 tNo);
U32 OTG_GetEP12FIFO		( U32 tNo);
U32 OTG_GetEP13FIFO		( U32 tNo);
U32 OTG_GetEP14FIFO		( U32 tNo);
U32 OTG_GetEP15FIFO		( U32 tNo);

void OTG_SetDEBUGFIFO	( U32 tNo, U32 tDEBUGFIFO	);
U32 OTG_GetDEBUGFIFO	( U32 tNo);

void OTG_SetRESERVED10	( U32 tNo, U32 value	);
U32 OTG_GetRESERVED10	( U32 tNo);

U32 xxx (void);


//@}


//------------------------------------------------------------------------------
///	@name	member structure
//------------------------------------------------------------------------------
typedef struct
{
	void (*SetGOTGCTL		) ( U32 tGOTGCTL     	);
	void (*SetGOTGINT     	) ( U32 tGOTGINT     	);
	void (*SetGAHBCFG     	) ( U32 tGAHBCFG     	);
	void (*SetGUSBCFG     	) ( U32 tGUSBCFG     	);
	void (*SetGRSTCTL     	) ( U32 tGRSTCTL     	);
	void (*SetGINTSTS     	) ( U32 tGINTSTS     	);
	void (*SetGINTMSK     	) ( U32 tGINTMSK     	);
	void (*SetGRXSTSR     	) ( U32 tGRXSTSR     	);
	void (*SetGRXSTSP     	) ( U32 tGRXSTSP     	);
	void (*SetGRXFSIZ     	) ( U32 tGRXFSIZ     	);
	void (*SetGNPTXFSIZ   	) ( U32 tGNPTXFSIZ   	);
	void (*SetGNPTXSTS    	) ( U32 tGNPTXSTS    	);
	void (*SetGI2CCTL     	) ( U32 tGI2CCTL     	);
	void (*SetGPVNDCTL    	) ( U32 tGPVNDCTL    	);
	void (*SetGGPIO       	) ( U32 tGGPIO       	);
	void (*SetGUID        	) ( U32 tGUID        	);
	void (*SetGSNPSID     	) ( U32 tGSNPSID     	);
	void (*SetGHWCFG1     	) ( U32 tGHWCFG1     	);
	void (*SetGHWCFG2     	) ( U32 tGHWCFG2     	);
	void (*SetGHWCFG3     	) ( U32 tGHWCFG3     	);
	void (*SetGHWCFG4     	) ( U32 tGHWCFG4     	);
	void (*SetGLPMCFG     	) ( U32 tGLPMCFG     	);
	void (*SetHPTXFSIZ    	) ( U32 tHPTXFSIZ    	);
	void (*SetHCFG        	) ( U32 tHCFG        	);
	void (*SetHFIR        	) ( U32 tHFIR        	);
	void (*SetHFNUM       	) ( U32 tHFNUM       	);
	void (*SetHPTXSTS     	) ( U32 tHPTXSTS     	);
	void (*SetHAINT       	) ( U32 tHAINT       	);
	void (*SetHAINTMSK    	) ( U32 tHAINTMSK    	);
	void (*SetHPRT        	) ( U32 tHPRT        	);
	void (*SetDCFG        	) ( U32 tDCFG        	);
	void (*SetDCTL        	) ( U32 tDCTL        	);
	void (*SetDSTS        	) ( U32 tDSTS        	);
	void (*SetDIEPMSK     	) ( U32 tDIEPMSK     	);
	void (*SetDOEPMSK     	) ( U32 tDOEPMSK     	);
	void (*SetDAINT       	) ( U32 tDAINT       	);
	void (*SetDAINTMSK    	) ( U32 tDAINTMSK    	);
	void (*SetDTKNQR1     	) ( U32 tDTKNQR1     	);
	void (*SetDTKNQR2     	) ( U32 tDTKNQR2     	);
	void (*SetDVBUSDIS    	) ( U32 tDVBUSDIS    	);
	void (*SetDVBUSPULSE  	) ( U32 tDVBUSPULSE  	);
	void (*SetDTKNQR3     	) ( U32 tDTKNQR3     	);
	void (*SetDTKNQR4     	) ( U32 tDTKNQR4     	);
	void (*SetPCGCCTL     	) ( U32 tPCGCCTL     	);

	void (*SetDPTXFSIZ   	) ( U32 tNo, U32 tDPTXFSIZ	);

	void (*SetHCSR			) ( U32 tNo, U32 tHCDMA, U32 tHCTSIZ, U32 tHCINTMSK, U32 tHCINT, U32 tHCSPLT, U32 tHCCHAR);
	void (*SetHCCHAR		) ( U32 tNo, U32 tHCCHAR	);
	void (*SetHCSPLT		) ( U32 tNo, U32 tHCSPLT	);
	void (*SetHCINT			) ( U32 tNo, U32 tHCINT	);
	void (*SetHCINTMSK		) ( U32 tNo, U32 tHCINTMSK	);
	void (*SetHCTSIZ		) ( U32 tNo, U32 tHCTSIZ	);
	void (*SetHCDMA			) ( U32 tNo, U32 tHCDMA	);

	void (*SetDIESR			) ( U32 tNo, U32 tDIEPDMA, U32 tDIEPTSIZ, U32 tDIEPINT, U32 tDIEPCTL);
	void (*SetDIEPCTL	    ) ( U32 tNo, U32 tDIEPCTL	);
	void (*SetDIEPINT	    ) ( U32 tNo, U32 tDIEPINT	);
	void (*SetDIEPTSIZ		) ( U32 tNo, U32 tDIEPTSIZ	);
	void (*SetDIEPDMA	    ) ( U32 tNo, U32 tDIEPDMA	);

	void (*SetDOESR			) ( U32 tNo, U32 tDOEPDMA, U32 tDOEPTSIZ, U32 tDOEPINT, U32 tDOEPCTL);
	void (*SetDOEPCTL	    ) ( U32 tNo, U32 tDOEPCTL	);
	void (*SetDOEPINT	    ) ( U32 tNo, U32 tDOEPINT	);
	void (*SetDOEPTSIZ		) ( U32 tNo, U32 tDOEPTSIZ	);
	void (*SetDOEPDMA	    ) ( U32 tNo, U32 tDOEPDMA	);

	U32 (*GetGOTGCTL      	) ( void );
	U32 (*GetGOTGINT      	) ( void );
	U32 (*GetGAHBCFG      	) ( void );
	U32 (*GetGUSBCFG      	) ( void );
	U32 (*GetGRSTCTL      	) ( void );
	U32 (*GetGINTSTS      	) ( void );
	U32 (*GetGINTMSK      	) ( void );
	U32 (*GetGRXSTSR      	) ( void );
	U32 (*GetGRXSTSP      	) ( void );
	U32 (*GetGRXFSIZ      	) ( void );
	U32 (*GetGNPTXFSIZ    	) ( void );
	U32 (*GetGNPTXSTS     	) ( void );
	U32 (*GetGI2CCTL      	) ( void );
	U32 (*GetGPVNDCTL     	) ( void );
	U32 (*GetGGPIO        	) ( void );
	U32 (*GetGUID         	) ( void );
	U32 (*GetGSNPSID      	) ( void );
	U32 (*GetGHWCFG1      	) ( void );
	U32 (*GetGHWCFG2      	) ( void );
	U32 (*GetGHWCFG3      	) ( void );
	U32 (*GetGHWCFG4      	) ( void );
	U32 (*GetGLPMCFG      	) ( void );
	U32 (*GetHPTXFSIZ     	) ( void );
	U32 (*GetHCFG         	) ( void );
	U32 (*GetHFIR         	) ( void );
	U32 (*GetHFNUM        	) ( void );
	U32 (*GetHPTXSTS      	) ( void );
	U32 (*GetHAINT        	) ( void );
	U32 (*GetHAINTMSK     	) ( void );
	U32 (*GetHPRT         	) ( void );
	U32 (*GetDCFG         	) ( void );
	U32 (*GetDCTL         	) ( void );
	U32 (*GetDSTS         	) ( void );
	U32 (*GetDIEPMSK      	) ( void );
	U32 (*GetDOEPMSK      	) ( void );
	U32 (*GetDAINT        	) ( void );
	U32 (*GetDAINTMSK     	) ( void );
	U32 (*GetDTKNQR1      	) ( void );
	U32 (*GetDTKNQR2      	) ( void );
	U32 (*GetDVBUSDIS     	) ( void );
	U32 (*GetDVBUSPULSE   	) ( void );
	U32 (*GetDTKNQR3      	) ( void );
	U32 (*GetDTKNQR4      	) ( void );
	U32 (*GetPCGCCTL      	) ( void );

	U32 (*GetDPTXFSIZ    	) ( U32 tNo);

	U32 (*GetHCCHAR			) ( U32 tNo);
	U32 (*GetHCSPLT			) ( U32 tNo);
	U32 (*GetHCINT			) ( U32 tNo);
	U32 (*GetHCINTMSK	    ) ( U32 tNo);
	U32 (*GetHCTSIZ			) ( U32 tNo);
	U32 (*GetHCDMA			) ( U32 tNo);

	U32 (*GetDIEPCTL	    ) ( U32 tNo);
	U32 (*GetDIEPINT	    ) ( U32 tNo);
	U32 (*GetDIEPTSIZ		) ( U32 tNo);
	U32 (*GetDIEPDMA	    ) ( U32 tNo);

	U32 (*GetDOEPCTL	    ) ( U32 tNo);
	U32 (*GetDOEPINT	    ) ( U32 tNo);
	U32 (*GetDOEPTSIZ		) ( U32 tNo);
	U32 (*GetDOEPDMA	    ) ( U32 tNo);
} OTG_FUNC_IF;

    void SetOTG_APB00(U32 no, U32 value);
    U32 GetOTG_APB00(U32 no);

//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_USB20OTG_H__
