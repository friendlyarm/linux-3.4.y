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
//	File		: nx_resconv.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_RESCONV_H__
#define __NX_RESCONV_H__




#include "../base/nx_prototype.h"


// #define	RESCONV_REISTER_TEST

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	RESCONV
//------------------------------------------------------------------------------
//@{

#define 	NUMBER_OF_RESCONV_MODULE 		1
//#define 	PHY_BASEADDR_DUALDISPLAY_MODULE     (PHY_BASEADDR_DISPLAYTOP_MODULE + 0x1000*1)
//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
    volatile U32 ADDR_RC_REG00;         // 0x00
    volatile U32 ADDR_RC_REG01;         // 0x04
    volatile U32 ADDR_RC_REG02;         // 0x08
    volatile U32 ADDR_RC_REG03;         // 0x0C
    volatile U32 ADDR_RC_REG04;         // 0x10
    volatile U32 ADDR_RC_REG05;         // 0x14
    volatile U32 ADDR_RC_REG06;         // 0x18
    volatile U32 ADDR_RC_REG07;         // 0x1C
    volatile U32 ADDR_RC_REG08;         // 0x20
    volatile U32 ADDR_RC_REG09;         // 0x24
    volatile U32 ADDR_RC_REG10;         // 0x28
    volatile U32 ADDR_RC_REG11;         // 0x2C
    volatile U32 ADDR_RC_REG12;         // 0x30
	volatile U32 _Reserved0[19]; 		// 21 ~ 31
    volatile U32 ADDR_RC_YVFILTER[24];	// 32 ~ 55
	volatile U32 _Reserved1[8]; 		// 56 ~ 63
    volatile U32 ADDR_RC_YHFILTER[160];
} NX_RESCONV_RegisterSet;

//------------------------------------------------------------------------------
///	@name	RESCONV Interface
//------------------------------------------------------------------------------
//@{
U32   NX_RESCONV_GetTEMP( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_RESCONV_Initialize( void );
U32   NX_RESCONV_GetNumberOfModule( void );

U32   NX_RESCONV_GetSizeOfRegisterSet( void );
void  NX_RESCONV_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void* NX_RESCONV_GetBaseAddress( U32 ModuleIndex );
U32   NX_RESCONV_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_RESCONV_OpenModule( U32 ModuleIndex );
CBOOL NX_RESCONV_CloseModule( U32 ModuleIndex );
CBOOL NX_RESCONV_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_RESCONV_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_RESCONV_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
    NX_RESCONV_INT_TEST0 = 0, ///< test0 interrupt
    NX_RESCONV_INT_TEST1 = 1, ///< test1 interrupt
    NX_RESCONV_INT_TEST2 = 2, ///< test2 interrupt
} NX_RESCONV_INT;
U32   NX_RESCONV_GetInterruptNumber ( U32 ModuleIndex );
void  NX_RESCONV_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL NX_RESCONV_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL NX_RESCONV_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_RESCONV_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_RESCONV_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL NX_RESCONV_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL NX_RESCONV_GetInterruptPendingAll( U32 ModuleIndex );
void  NX_RESCONV_ClearInterruptPendingAll( U32 ModuleIndex );
S32   NX_RESCONV_GetInterruptPendingNumber( U32 ModuleIndex );
//@}
void	NX_RESCONV_DOWN_INIT ( U32 ModuleIndex );
void    NX_RESCONV_FINIT( U32 ModuleIndex );
void    NX_RESCONV_INIT (   U32 ModuleIndex,
                            int SRC_HSIZE,
                            int SRC_VSIZE,
                            int SRC_CROP_L,
                            int SRC_CROP_R,
                            int SRC_CROP_T,
                            int SRC_CROP_B,
                            int DST_HSIZE,
                            int DST_VSIZE,
                            int FIXED_POINT,
                            int DELTA_X,
                            int DELTA_Y,
                            int SoftV,
                            int SoftH,
                            int SG_VFP, // ResConv SyncGen V Front Porch
                            int SG_VBP, // ResConv SyncGen V Back  Porch
                            int SG_VS , // ResConv SyncGen V Sync
                            int SG_VAV, // ResConv SyncGen V Active
                            int SG_HFP, // ResConv SyncGen H Front Porch
                            int SG_HBP, // ResConv SyncGen H Back  Porch
                            int SG_HS , // ResConv SyncGen H Sync
                            int SG_HAV, // ResConv SyncGen H Active
                            int SG_S2IN_VS,
                            int SG_HOFFSET,
                            int SG_HDELAY	);
void	NX_RESCONV_RUN ( U32 ModuleIndex );
void	NX_RESCONV_STOP ( U32 ModuleIndex );
void	NX_RESCONV_INTCLEAR ( U32 ModuleIndex );

#if defined(RESCONV_REISTER_TEST)
void	NX_RESCONV_REG_RD_TEST ( U32 ModuleIndex );
#endif

//------------------------------------------------------------------------------
///	@name	DMA Interface
//------------------------------------------------------------------------------
//@{
/// @brief	DMA index for DMAChannelIndex
typedef enum
{
    NX_RESCONV_DMA_TXDMA = 0, ///< TX channel
    NX_RESCONV_DMA_RXDMA = 1, ///< RX channel
} NX_RESCONV_DMA;
U32 NX_RESCONV_GetDMANumber ( U32 ModuleIndex, U32 DMAChannelIndex );
//@}


//@}
//================
#define NX_RESCONV_IsOverFlow(ModuleIndex )		CFALSE
#define NX_RESCONV_IsUnderFlow(ModuleIndex )	CFALSE
#define NX_RESCONV_SetS2IN_VS(ModuleIndex,	SG_S2IN_VS)
#define NX_RESCONV_FIFO_Init(ModuleIndex, Enb )

#ifdef	__cplusplus
}
#endif


#endif // __NX_RESCONV_H__
