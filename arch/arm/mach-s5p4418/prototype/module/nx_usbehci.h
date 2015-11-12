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
//	File		: nx_USB20HOST.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_USB20HOST_H__
#define __NX_USB20HOST_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	USB20HOST
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
    volatile U32 HCCAPBASE;			// 0
    volatile U32 HCSPARAMS;			// 4
    volatile U32 HCCPARAMS;			// 8
    volatile U32 RESERVED00;		// c

    volatile U32 USBCMD;			// 10
    volatile U32 USBSTS;			// 14
    volatile U32 USBINTR;			// 18
    volatile U32 FRINDEX;			// 1c
    volatile U32 CTRLDSSEGMENT;		// 20
    volatile U32 PERIODICLISTBASE;	// 24
    volatile U32 ASYNCLISTADDR;		// 28
    volatile U32 RESERVED01[9];		// 2c~4c

    volatile U32 CONFIGFLAG;		// 50
    volatile U32 PORTSC;			// 54
    volatile U32 PORTSC1;			// 58
    volatile U32 PORTSC2;			// 5C
    volatile U32 RESERVED02[12];	// 58~8c

    volatile U32 INSNREG00;			// 90
    volatile U32 INSNREG01;			// 94
    volatile U32 INSNREG02;			// 98
    volatile U32 INSNREG03;			// 9c
    volatile U32 INSNREG04;			// a0
    volatile U32 INSNREG05;			// a4	164	1010_0100
    volatile U32 INSNREG06;			// a8	//ohci
    volatile U32 INSNREG07;			// ac	//ohci
    volatile U32 INSNREG08;			// b0	//hsic
} NX_USB20HOST_RegisterSet;

typedef struct
{
    volatile U32 HcRevision;            // 0	//0		//d0	h00
    volatile U32 HcControl;             // 4    //2		//d1	h01
    volatile U32 HcCommandStatus;       // 8    //4 	//d2 	h02
    volatile U32 HcInterruptStatus;		// c    //6 	//d3 	h03
    volatile U32 HcInterruptEnable;     // 10   //8 	//d4 	h04
    volatile U32 HcInterruptDisable;    // 14   //a 	//d5 	h05
    volatile U32 HcHCCA;                // 18   //c		//d6 	h06
    volatile U32 HcPeriodCurrentED;     // 1c   //e 	//d7 	h07
    volatile U32 HcControlHeadED;       // 20   //10	//d8 	h08
    volatile U32 HcControlCurrentED;    // 24   //12	//d9 	h09
    volatile U32 HcBulkHeadED;          // 28   //14	//d10	h0a
    volatile U32 HcBulkCurrentED;       // 2c   //16	//d11	h0b
    volatile U32 HcDoneHead;            // 30   //18	//d12	h0c
    volatile U32 HcFmInterval;          // 34   //1a	//d13	h0d
    volatile U32 HcFmRemaining;         // 38   //1c	//d14	h0e
    volatile U32 HcFmNumber;            // 3c   //1e	//d15	h0f
    volatile U32 HcPeriodicStart;       // 40   //20	//d16	h10
    volatile U32 HcLSThreshold;         // 44   //22	//d17	h11
    volatile U32 HcRhDescriptorA;       // 48   //24	//d18	h12
    volatile U32 HcRhDescriptorB;       // 4c   //26	//d19	h13
    volatile U32 HcRhStatus;            // 50   //28	//d20	h14
    volatile U32 HcRhPortStatus;        // 54	//2a	//d21	h15
    volatile U32 HcReserve[10];         // 58~7c//2c	//d22	h16
} NX_USB20HOST_OHCI_RegisterSet;

typedef struct
{
    volatile U32 APB00[56];
} NX_USB20HOST_APB_RegisterSet;

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_USB20HOST_Initialize( void );
U32   NX_USB20HOST_GetNumberOfModule( void );

U32   NX_USB20HOST_GetSizeOfRegisterSet( void );
void  NX_USB20HOST_SetBaseAddress( U32 ModuleIndex, U32* BaseAddress );
U32   NX_USB20HOST_GetBaseAddress( U32 ModuleIndex );
U32   NX_USB20HOST_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_USB20HOST_OpenModule( U32 ModuleIndex );
CBOOL NX_USB20HOST_CloseModule( U32 ModuleIndex );
CBOOL NX_USB20HOST_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_USB20HOST_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_USB20HOST_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
    NX_USB20HOST_INT_TEST0 = 0, ///< test0 interrupt
    NX_USB20HOST_INT_TEST1 = 1, ///< test1 interrupt
    NX_USB20HOST_INT_TEST2 = 2, ///< test2 interrupt
} NX_USB20HOST_INT;
//U32   NX_USB20HOST_GetInterruptNumber ( U32 ModuleIndex );
//void  NX_USB20HOST_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
//CBOOL NX_USB20HOST_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
//CBOOL NX_USB20HOST_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
//void  NX_USB20HOST_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
//void  NX_USB20HOST_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
//CBOOL NX_USB20HOST_GetInterruptEnableAll( U32 ModuleIndex );
//CBOOL NX_USB20HOST_GetInterruptPendingAll( U32 ModuleIndex );
//void  NX_USB20HOST_ClearInterruptPendingAll( U32 ModuleIndex );
//S32   NX_USB20HOST_GetInterruptPendingNumber( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//------------------------------------------------------------------------------
//@{
/// @brief	DMA index for DMAChannelIndex
typedef enum
{
    NX_USB20HOST_DMA_TXDMA = 0, ///< TX channel
    NX_USB20HOST_DMA_RXDMA = 1, ///< RX channel
} NX_USB20HOST_DMA;
U32 NX_USB20HOST_GetDMANumber ( U32 ModuleIndex, U32 DMAChannelIndex );
//@}


//@}

//------------------------------------------------------------------------------
///	ehci
//------------------------------------------------------------------------------
	void SetHCCAPBASE		(U32 tHCCAPBASE			);
	void SetHCSPARAMS       (U32 tHCSPARAMS       	);
	void SetHCCPARAMS       (U32 tHCCPARAMS       	);
	void SetUSBCMD          (U32 tUSBCMD          	);
	void SetUSBSTS          (U32 tUSBSTS          	);
	void SetUSBINTR         (U32 tUSBINTR         	);
	void SetFRINDEX         (U32 tFRINDEX         	);
	void SetCTRLDSSEGMENT   (U32 tCTRLDSSEGMENT   	);
	void SetPERIODICLISTBASE(U32 tPERIODICLISTBASE	);
	void SetASYNCLISTADDR   (U32 tASYNCLISTADDR   	);
	void SetCONFIGFLAG      (U32 tCONFIGFLAG      	);
	void SetPORTSC          (U32 tPORTSC          	);
	void SetINSNREG00       (U32 tINSNREG00       	);
	void SetINSNREG01       (U32 tINSNREG01       	);
	void SetINSNREG02       (U32 tINSNREG02       	);
	void SetINSNREG03       (U32 tINSNREG03       	);
	void SetINSNREG04       (U32 tINSNREG04       	);
	void SetINSNREG05       (U32 tINSNREG05       	);
	void SetINSNREG06       (U32 tINSNREG06       	);
	void SetINSNREG07       (U32 tINSNREG07       	);
	void SetINSNREG08       (U32 tINSNREG08       	);
	void SetTESTPARM0 		(U32 tTESTPARM0			);
	void SetTESTPARM1 		(U32 tTESTPARM1			);
	void SetTESTPARM2 		(U32 tTESTPARM2			);
	void SetTESTPARM3 		(U32 tTESTPARM3			);
	void SetTESTPARM4 		(U32 tTESTPARM4			);
	void SetTESTPARM5 		(U32 tTESTPARM5			);
	void SetTESTPARM6 		(U32 tTESTPARM6			);

	U32 GetHCCAPBASE        (void);
	U32 GetHCSPARAMS        (void);
	U32 GetHCCPARAMS        (void);
	U32 GetUSBCMD           (void);
	U32 GetUSBSTS           (void);
	U32 GetUSBINTR          (void);
	U32 GetFRINDEX          (void);
	U32 GetCTRLDSSEGMENT    (void);
	U32 GetPERIODICLISTBASE (void);
	U32 GetASYNCLISTADDR    (void);
	U32 GetCONFIGFLAG       (void);
	U32 GetPORTSC           (void);
	U32 GetINSNREG00        (void);
	U32 GetINSNREG01        (void);
	U32 GetINSNREG02        (void);
	U32 GetINSNREG03        (void);
	U32 GetINSNREG04        (void);
	U32 GetINSNREG05        (void);
	U32 GetINSNREG06        (void);
	U32 GetINSNREG07        (void);
	U32 GetINSNREG08        (void);

    void SetHcRevision				(U32 value);
    void SetHcControl               (U32 value);
    void SetHcCommandStatus         (U32 value);
    void SetHcInterruptStatus       (U32 value);
    void SetHcInterruptEnable       (U32 value);
    void SetHcInterruptDisable      (U32 value);
    void SetHcHCCA                  (U32 value);
    void SetHcPeriodCurrentED       (U32 value);
    void SetHcControlHeadED         (U32 value);
    void SetHcControlCurrentED      (U32 value);
    void SetHcBulkHeadED            (U32 value);
    void SetHcBulkCurrentED         (U32 value);
    void SetHcDoneHead              (U32 value);
    void SetHcFmInterval            (U32 value);
    void SetHcFmRemaining           (U32 value);
    void SetHcFmNumber              (U32 value);
    void SetHcPeriodicStart         (U32 value);
    void SetHcLSThreshold           (U32 value);
    void SetHcRhDescriptorA         (U32 value);
    void SetHcRhDescriptorB         (U32 value);
    void SetHcRhStatus              (U32 value);
    void SetHcRhPortStatus          (U32 value);

    U32 GetHcRevision				(void);
    U32 GetHcControl               	(void);
    U32 GetHcCommandStatus         	(void);
    U32 GetHcInterruptStatus       	(void);
    U32 GetHcInterruptEnable       	(void);
    U32 GetHcInterruptDisable      	(void);
    U32 GetHcHCCA                  	(void);
    U32 GetHcPeriodCurrentED       	(void);
    U32 GetHcControlHeadED         	(void);
    U32 GetHcControlCurrentED      	(void);
    U32 GetHcBulkHeadED            	(void);
    U32 GetHcBulkCurrentED         	(void);
    U32 GetHcDoneHead              	(void);
    U32 GetHcFmInterval            	(void);
    U32 GetHcFmRemaining           	(void);
    U32 GetHcFmNumber              	(void);
    U32 GetHcPeriodicStart         	(void);
    U32 GetHcLSThreshold           	(void);
    U32 GetHcRhDescriptorA         	(void);
    U32 GetHcRhDescriptorB         	(void);
    U32 GetHcRhStatus              	(void);
    U32 GetHcRhPortStatus          	(void);

    void SetAPB00(U32 no, U32 value);
    U32 GetAPB00(U32 no);
#ifdef	__cplusplus
}
#endif


#endif // __NX_USB20HOST_H__
