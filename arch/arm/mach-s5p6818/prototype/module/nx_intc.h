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
//	Module		: Interrupt Controller
//	File		: nx_intc.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//
// @choyk 2012/07/03 : Vic 용 prototype 32,
// Vic의 familyName은 똑같이 INTC를 사용하도록 한다.
//
//------------------------------------------------------------------------------
#ifndef __NX_INTC_H__
#define __NX_INTC_H__

#include "../base/nx_prototype.h"

//#define NX_INTC_NUM_OF_INT  (32 * NUMBER_OF_INTC_MODULE) // NUMBER_OF_INTC_MODULE is in nx_chip.h
//#define NX_INTC_NUM_OF_INT  64 // @todo 필요한 개수를 nx_chip.h 에 정의할것.

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	INTC INTERRUPT
//------------------------------------------------------------------------------
//@{

	//--------------------------------------------------------------------------
	/// @brief	INTC register set structure.
	struct NX_INTC_RegisterSet
	{
		volatile U32 IRQSTATUS;					// 0x000
		volatile U32 FIQSTATUS;					// 0x004
		volatile U32 RAWINTR;					// 0x008
		volatile U32 INTMODE;					// 0x00C
		volatile U32 INTENABLE;					// 0x010
		volatile U32 INTDISABLE;				// 0x014
		volatile U32 SWINT;						// 0x018
		volatile U32 SWINTCLEAR;				// 0x01C
		volatile U32 PROENBALE;					// 0x020 protection enable bit
		volatile U32 SWPRIMASK;					// 0x024 software priority mask
		volatile U32 PRIORITY;					// 0x028 protection priority bit
		volatile U8  _Reserved0[0x100-0x2C];	// 0x028~0x0FF;
		volatile U32 VECTORADDR[32];			// 0x100~0x17C
		volatile U8  _Reserved1[0x200-0x180];	// 0x180~0x1FF
		volatile U32 VECTORPRI[32];				// 0x200~0x27C
		volatile U8  _Reserved2[0xF00-0x280];	// 0x280~0xEFF
		volatile U32 CURRENTVECTOR;				// 0xF00
		volatile U8  _Reserved3[0x1000-0xF04];	// 0xF04~0xFFF
	};

	/// @brief	interrupt mode.
	typedef enum
	{
		NX_INTC_INTMODE_IRQ = 0UL,		///< IRQ mode
		NX_INTC_INTMODE_FIQ = 1UL		///< FIQ mode

	}NX_INTC_INTMODE ;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{

CBOOL	NX_INTC_Initialize( void );
U32		NX_INTC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{

U32		NX_INTC_GetPhysicalAddress( void );
U32		NX_INTC_GetSizeOfRegisterSet( void );
void	NX_INTC_SetBaseAddress( void* BaseAddress );
void*	NX_INTC_GetBaseAddress( void );
CBOOL	NX_INTC_OpenModule( void );
CBOOL	NX_INTC_CloseModule( void );
CBOOL	NX_INTC_CheckBusy( void );
CBOOL	NX_INTC_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
void	NX_INTC_SetInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_INTC_GetInterruptEnable( S32 IntNum );
CBOOL	NX_INTC_GetInterruptPending( S32 IntNum );
void	NX_INTC_ClearInterruptPending( S32 IntNum );

void	NX_INTC_SetInterruptEnableAll( CBOOL Enable );
CBOOL	NX_INTC_GetInterruptEnableAll( void );
CBOOL	NX_INTC_GetInterruptPendingAll( void );
void	NX_INTC_ClearInterruptPendingAll( void );

// 2012/07/03
//void	NX_INTC_SetInterruptEnable64( U32 EnableHigh, U32 EnableLow );
//void	NX_INTC_GetInterruptEnable64( U32 *pEnableHigh, U32 *pEnableLow );
//void	NX_INTC_GetInterruptPending64( U32 *pPendingHigh, U32 *pPendingLow );
//void	NX_INTC_ClearInterruptPending64( U32 PendingHigh, U32 PendingLow );

S32		NX_INTC_GetInterruptPendingNumber( void );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Controller Operation.
//------------------------------------------------------------------------------
//@{
void	NX_INTC_SetInterruptMode ( U32 IntNum, NX_INTC_INTMODE IntMode	);
void	NX_INTC_SetPriorityMode( U32 ArbiterNum, U32 OrderSel );
//@}

void NX_INTC_SetIntHandler(U32 IntNum, void *Handler);
void *NX_INTC_GetIntHandler(U32 IntNum);
void *NX_INTC_GetCurrentIntHandler(void);

//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_INTC_H__
