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
//	File		: nx_CAN.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_CAN_H__
#define __NX_CAN_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	CAN
//------------------------------------------------------------------------------
//@{

//#define 	NUMBER_OF_CAN_MODULE 		1
//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
	volatile U16 CAN_REG00; // 0x00 : can control
    volatile U16 CAN_REG01; // 0x02 : status
    volatile U16 CAN_REG02; // 0x04 : Error counter
    volatile U16 CAN_REG03; // 0x06 : Bit Timing
    volatile U16 INTCTRL;   // 0x08 : Interrupt
    volatile U16 CAN_REG05; // 0x0a : test register
    volatile U16 CAN_REG06; // 0x0c : BRP Extension
    volatile U16 CAN_REG07; // 0x0e : Reserved
    volatile U16 CAN_REG08; // 0x10 : IF1 Command Request
    volatile U16 CAN_REG09; // 0x12 : IF1 Command Mask
    volatile U16 CAN_REG10; // 0x14 : IF1 Mask 1
    volatile U16 CAN_REG11; // 0x16 : IF1 Mask 2
    volatile U16 CAN_REG12; // 0x18 : IF1 Arbitration 1
    volatile U16 CAN_REG13; // 0x1a : IF1 Arbitration 2
    volatile U16 CAN_REG14; // 0x1c : IF1 Message Control
    volatile U16 CAN_REG15; // 0x1e : IF1 Data A 1
    volatile U16 CAN_REG16; // 0x20 : IF1 Data A 2
    volatile U16 CAN_REG17; // 0x22 : IF1 Data B 1
    volatile U16 CAN_REG18; // 0x24 : IF1 Data B 2
    volatile U16 CAN_REG19; // 0x26 : reserved
    volatile U16 CAN_REG20; // 0x28 : reserved
    volatile U16 CAN_REG21; // 0x2a : reserved
    volatile U16 CAN_REG22; // 0x2c : reserved
    volatile U16 CAN_REG23; // 0x2e : reserved
    volatile U16 CAN_REG24; // 0x30 : reserved
    volatile U16 CAN_REG25; // 0x32 : reserved
    volatile U16 CAN_REG26; // 0x34 : reserved
    volatile U16 CAN_REG27; // 0x36 : reserved
    volatile U16 CAN_REG28; // 0x38 : reserved
    volatile U16 CAN_REG29; // 0x3a : reserved
    volatile U16 CAN_REG30; // 0x3c : reserved
    volatile U16 CAN_REG31; // 0x3e : reserved
    volatile U16 CAN_REG32; // 0x40 : IF2 registers
    volatile U16 CAN_REG33; // 0x42 : IF2 registers
    volatile U16 CAN_REG34; // 0x44 : IF2 registers
    volatile U16 CAN_REG35; // 0x46 : IF2 registers
    volatile U16 CAN_REG36; // 0x48 : IF2 registers
    volatile U16 CAN_REG37; // 0x4a : IF2 registers
    volatile U16 CAN_REG38; // 0x4c : IF2 registers
    volatile U16 CAN_REG39; // 0x4e : IF2 registers
    volatile U16 CAN_REG40; // 0x50 : IF2 registers
    volatile U16 CAN_REG41; // 0x52 : IF2 registers
    volatile U16 CAN_REG42; // 0x54 : IF2 registers
    volatile U16 CAN_REG43; // 0x56 : reserved
    volatile U16 CAN_REG44; // 0x58 : reserved
    volatile U16 CAN_REG45; // 0x5a : reserved
    volatile U16 CAN_REG46; // 0x5c : reserved
    volatile U16 CAN_REG47; // 0x5e : reserved
    volatile U16 CAN_REG48; // 0x60 : reserved
    volatile U16 CAN_REG49; // 0x62 : reserved
    volatile U16 CAN_REG50; // 0x64 : reserved
    volatile U16 CAN_REG51; // 0x66 : reserved
    volatile U16 CAN_REG52; // 0x68 : reserved
    volatile U16 CAN_REG53; // 0x6a : reserved
    volatile U16 CAN_REG54; // 0x6c : reserved
    volatile U16 CAN_REG55; // 0x6e : reserved
    volatile U16 CAN_REG56; // 0x70 : reserved
    volatile U16 CAN_REG57; // 0x72 : reserved
    volatile U16 CAN_REG58; // 0x74 : reserved
    volatile U16 CAN_REG59; // 0x76 : reserved
    volatile U16 CAN_REG60; // 0x78 : reserved
    volatile U16 CAN_REG61; // 0x7a : reserved
    volatile U16 CAN_REG62; // 0x7c : reserved
    volatile U16 CAN_REG63; // 0x7e : reserved
    volatile U16 CAN_REG64; // 0x80 : Transmission Request 1
    volatile U16 CAN_REG65; // 0x82 : Transmission Request 2
    volatile U16 CAN_REG66; // 0x84 : reserved
    volatile U16 CAN_REG67; // 0x86 : reserved
    volatile U16 CAN_REG68; // 0x88 : reserved
    volatile U16 CAN_REG69; // 0x8a : reserved
    volatile U16 CAN_REG70; // 0x8c : reserved
    volatile U16 CAN_REG71; // 0x8e : reserved
    volatile U16 CAN_REG72; // 0x90 : New Data 1
    volatile U16 CAN_REG73; // 0x92 : New Data 2
    volatile U16 CAN_REG74; // 0x94 : reserved
    volatile U16 CAN_REG75; // 0x96 : reserved
    volatile U16 CAN_REG76; // 0x98 : reserved
    volatile U16 CAN_REG77; // 0x9a : reserved
    volatile U16 CAN_REG78; // 0x9c : reserved
    volatile U16 CAN_REG79; // 0x9e : reserved
    volatile U16 INTPEND  ; // 0xa0 : Interrupt Pending 1
    volatile U16 CAN_REG81; // 0xa2 : Interrupt Pending 2
    volatile U16 CAN_REG82; // 0xa4 : reserved
    volatile U16 CAN_REG83; // 0xa6 : reserved
    volatile U16 CAN_REG84; // 0xa8 : reserved
    volatile U16 CAN_REG85; // 0xaa : reserved
    volatile U16 CAN_REG86; // 0xac : reserved
    volatile U16 CAN_REG87; // 0xae : reserved
    volatile U16 CAN_REG88; // 0xb0 : Message Valid 1
    volatile U16 CAN_REG89; // 0xb2 : Message Valid 2
    volatile U16 CAN_REG90; // 0xb4 : reserved
    volatile U16 CAN_REG91; // 0xb6 : reserved
    volatile U16 CAN_REG92; // 0xb8 : reserved
    volatile U16 CAN_REG93; // 0xba : reserved
    volatile U16 CAN_REG94; // 0xbc : reserved
    volatile U16 CAN_REG95; // 0xbe : reserved
} NX_CAN_RegisterSet;

//------------------------------------------------------------------------------
///	@name	CAN Interface
//------------------------------------------------------------------------------
//@{
U32   NX_CAN_GetTEMP( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_CAN_Initialize( void );
U32   NX_CAN_GetNumberOfModule( void );

U32   NX_CAN_GetSizeOfRegisterSet( void );
void  NX_CAN_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*  NX_CAN_GetBaseAddress( U32 ModuleIndex );
U32   NX_CAN_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_CAN_OpenModule( U32 ModuleIndex );
CBOOL NX_CAN_CloseModule( U32 ModuleIndex );
CBOOL NX_CAN_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_CAN_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_CAN_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
    NX_CAN_INT_TEST0 = 0, ///< test0 interrupt
    NX_CAN_INT_TEST1 = 1, ///< test1 interrupt
    NX_CAN_INT_TEST2 = 2, ///< test2 interrupt
} NX_CAN_INT;
U32   NX_CAN_GetInterruptNumber ( U32 ModuleIndex );
void  NX_CAN_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL NX_CAN_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL NX_CAN_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_CAN_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_CAN_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL NX_CAN_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL NX_CAN_GetInterruptPendingAll( U32 ModuleIndex );
void  NX_CAN_ClearInterruptPendingAll( U32 ModuleIndex );
S32   NX_CAN_GetInterruptPendingNumber( U32 ModuleIndex );
void    NX_CAN_REG_RD_TEST ( U32 ModuleIndex );
void    NX_CAN_RDWR_RAM_TEST ( U32 ModuleIndex );
void    NX_CAN_TRANSMIT_TEST ( U32 ModuleIndex );
void    NX_CAN_RD_STATUS_REG ( U32 ModuleIndex );

//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//------------------------------------------------------------------------------
//@{
/// @brief	DMA index for DMAChannelIndex
typedef enum
{
    NX_CAN_DMA_TXDMA = 0, ///< TX channel
    NX_CAN_DMA_RXDMA = 1, ///< RX channel
} NX_CAN_DMA;
U32 NX_CAN_GetDMANumber ( U32 ModuleIndex, U32 DMAChannelIndex );
//@}

//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_CAN_H__
