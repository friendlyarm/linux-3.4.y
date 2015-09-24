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
//	File		: nx_PDM.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------

#include <nx_chip.h>
#include "nx_PDM.h"
#include <string.h> // for memset

volatile static	NX_PDM_RegisterSet *__g_pRegister[NUMBER_OF_PDM_MODULE];


//------------------------------------------------------------------------------
//
//	PDM Interface
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//---------- CLKGEN 을 위한 prototype
U32 NX_PDM_GetClockNumber (U32 ModuleIndex)
{
    static const U32 CLKGEN_PDMLIST[] = { CLOCKINDEX_LIST( PDM ) };
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
    return (U32)CLKGEN_PDMLIST[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.
 *	@see	NX_PDM_GetNumberOfModule
 */
CBOOL	NX_PDM_Initialize( void )
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
 *				It is equal to NUMBER_OF_PDM_MODULE in <nx_chip.h>.
 *	@see		NX_PDM_Initialize
 */
U32		NX_PDM_GetNumberOfModule( void )
{
	return NUMBER_OF_PDM_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_PDM_GetPhysicalAddress,
 *				NX_PDM_SetBaseAddress,			NX_PDM_GetBaseAddress,
 *				NX_PDM_OpenModule,				NX_PDM_CloseModule,
 *				NX_PDM_CheckBusy,
 */
U32		NX_PDM_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_PDM_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_PDM_GetPhysicalAddress,		NX_PDM_GetSizeOfRegisterSet,
 *				NX_PDM_GetBaseAddress,
 *				NX_PDM_OpenModule,				NX_PDM_CloseModule,
 *				NX_PDM_CheckBusy,
 */
void	NX_PDM_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_PDM_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_PDM_GetPhysicalAddress,		NX_PDM_GetSizeOfRegisterSet,
 *				NX_PDM_SetBaseAddress,
 *				NX_PDM_OpenModule,				NX_PDM_CloseModule,
 *				NX_PDM_CheckBusy,
 */
U32		NX_PDM_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	return (U32)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. \n
 *				It is equal to PHY_BASEADDR_PDM?_MODULE in <nx_chip.h>.
 *	@see		NX_PDM_GetSizeOfRegisterSet,
 *				NX_PDM_SetBaseAddress,			NX_PDM_GetBaseAddress,
 *				NX_PDM_OpenModule,				NX_PDM_CloseModule,
 *				NX_PDM_CheckBusy,
 */
U32		NX_PDM_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_PDM_MODULE] =
	{
		PHY_BASEADDR_LIST( PDM )
	};
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 *	@see		NX_PDM_GetPhysicalAddress,		NX_PDM_GetSizeOfRegisterSet,
 *				NX_PDM_SetBaseAddress,			NX_PDM_GetBaseAddress,
 *				NX_PDM_CloseModule,
 *				NX_PDM_CheckBusy,
 */
CBOOL	NX_PDM_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_PDM_GetPhysicalAddress,		NX_PDM_GetSizeOfRegisterSet,
 *				NX_PDM_SetBaseAddress,			NX_PDM_GetBaseAddress,
 *				NX_PDM_OpenModule,
 *				NX_PDM_CheckBusy,
 */
CBOOL	NX_PDM_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_PDM_GetPhysicalAddress,		NX_PDM_GetSizeOfRegisterSet,
 *				NX_PDM_SetBaseAddress,			NX_PDM_GetBaseAddress,
 *				NX_PDM_OpenModule,				NX_PDM_CloseModule,
 */
// No CheckBusy
//CBOOL	NX_PDM_CheckBusy( U32 ModuleIndex )
//{
//    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
//    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
//    // @todo Implement
//	return CFALSE;
//}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_PDM?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_PDM_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( PDM, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_PDM_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.\n
 *				It is equal to INTNUM_OF_PDM?_MODULE in <nx_chip.h>.
 *	@see		NX_PDM_SetInterruptEnable,
 *				NX_PDM_GetInterruptEnable,
 *				NX_PDM_GetInterruptPending,
 *				NX_PDM_ClearInterruptPending,
 *				NX_PDM_SetInterruptEnableAll,
 *				NX_PDM_GetInterruptEnableAll,
 *				NX_PDM_GetInterruptPendingAll,
 *				NX_PDM_ClearInterruptPendingAll,
 *				NX_PDM_GetInterruptPendingNumber
 */
U32 	NX_PDM_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( PDM ) };
    NX_CASSERT( NUMBER_OF_PDM_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );

    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt Mode
 *              Mode means IRQ Count Number.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Mode	\b Set as 8~1  to enable a interrupt. \r\n
 *						\b Set as 0    to disable a interrupt.
 *								  9~   dont's work
 *	@return		None.
 *	@see		NX_PDM_GetInterruptNumber,
 *				NX_PDM_GetInterruptPendingAll,
 *				NX_PDM_GetInterruptPendingNumber
 */
void	NX_PDM_SetInterruptMode( U32 ModuleIndex, U32 Mode )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	NX_ASSERT( Mode <= 8 && Mode >= 0 );

	regvalue = Mode;
	WriteIODW(&pRegister->PDM_IRQCTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		\b CTRUE	indicates that one or more interrupts are pended. \r\n
 *				\b CFALSE	indicates that no interrupt is pended.
 *	@see		NX_PDM_GetInterruptNumber,
 *				NX_PDM_SetInterruptEnable,
 *				NX_PDM_GetInterruptEnable,
 *				NX_PDM_GetInterruptPending,
 *				NX_PDM_ClearInterruptPending,
 *				NX_PDM_SetInterruptEnableAll,
 *				NX_PDM_GetInterruptEnableAll,
 *				NX_PDM_ClearInterruptPendingAll,
 *				NX_PDM_GetInterruptPendingNumber

 */
CBOOL	NX_PDM_GetInterruptPendingAll( U32 ModuleIndex )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );


	regvalue  = ReadIODW(&pRegister->PDM_IRQCTRL);
	if( (regvalue >> 6) && 0x01 )   return CTRUE;
	else 							return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 *	@see		NX_PDM_GetInterruptNumber,
 *				NX_PDM_SetInterruptEnable,
 *				NX_PDM_GetInterruptEnable,
 *				NX_PDM_GetInterruptPending,
 *				NX_PDM_ClearInterruptPending,
 *				NX_PDM_SetInterruptEnableAll,
 *				NX_PDM_GetInterruptEnableAll,
 *				NX_PDM_GetInterruptPendingAll,
 *				NX_PDM_GetInterruptPendingNumber

 */
void	NX_PDM_ClearInterruptPendingAll( U32 ModuleIndex )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue  = ReadIODW(&pRegister->PDM_IRQCTRL);
	regvalue  = regvalue | (1<<5);
	WriteIODW(&pRegister->PDM_IRQCTRL, regvalue);	// just write operation make pending clear
}



//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index
 *	@param[in]	ModuleIndex	an index of module.
 *	@return		DMA peripheral index.
 *	@see		NX_DMA_TransferMemToIO(DestinationPeriID),
 *				NX_DMA_TransferIOToMem(SourcePeriID)
 */
U32 NX_PDM_GetDMANumber ( U32 ModuleIndex )
{
	const U32 DMANumber[NUMBER_OF_PDM_MODULE] =
	{
	    { DMAINDEX_LIST( PDM ) }, // DMAINDEX_OF_PDM?_MODULE
	};
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );

	return DMANumber[ModuleIndex];
}

U32 NX_PDM_GetDMABusWidth( U32 ModuleIndex )
{
	return 32; // 32 bit
}

//------------------------------------------------------------------------------
///	@name	Basic Use Functions
//------------------------------------------------------------------------------
//@{
void NX_PDM_InitSet( U32 ModuleIndex, U32 Enb )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );

	regvalue  = ReadIODW(&pRegister->PDM_CTRL);

	if( Enb == CTRUE )	regvalue = regvalue | 0x01;
	else				regvalue = regvalue & (~0x01);

	WriteIODW(&pRegister->PDM_CTRL, regvalue);	// just write operation make pending clear
}

// S16으로 처리할때는 모든 연산에 S16을 처리해주도록 하자
// 2012-11-08 GainxMinux2에서 32bit로 처리되어서 or한 결과가 이상한 경우도 발생.
// 따라서 아래와 같이 Masking을 해주도록 한다.
void NX_PDM_SetGain0( U32 ModuleIndex, S16 Gainx4, S16 Gainx2 )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	regvalue  = ((0xFFFF & (S16)Gainx4) << 16) | ((0xFFFF & (S16)Gainx2) << 0);

	WriteIODW(&pRegister->PDM_GAIN0, regvalue);	// just write operation make pending clear
}


void NX_PDM_SetGain1( U32 ModuleIndex, S16 GainxMinus4, S16 GainxMinus2 )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	regvalue  = ((0xFFFF & (S16)GainxMinus4) << 16) | ((0xFFFF & (S16)GainxMinus2) << 0);

	WriteIODW(&pRegister->PDM_GAIN1, regvalue);	// just write operation make pending clear
}

void NX_PDM_SetCoeff( U32 ModuleIndex, S16 Coeff1, S16 Coeff0 )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	regvalue  = ((0xFFFF & (S16)Coeff1) << 16) | ((0xFFFF & (S16)Coeff0) << 0);

	WriteIODW(&pRegister->PDM_COEFF, regvalue);	// just write operation make pending clear
}

void NX_PDM_SetOverSample( U32 ModuleIndex, U32 OverSample )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( OverSample < 128 ); // 7 bit : 128

	regvalue  = ReadIODW(&pRegister->PDM_CTRL);
	regvalue  = regvalue & (~( 0x7f<<16 ));
	regvalue  = regvalue | (OverSample << 16);

	WriteIODW(&pRegister->PDM_CTRL, regvalue);	// just write operation make pending clear
}

void NX_PDM_SetStrobeShift( U32 ModuleIndex, U32 StrobeShift )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( StrobeShift < 32 ); // 5 bit : 32

	regvalue  = ReadIODW(&pRegister->PDM_CTRL);
	regvalue  = regvalue & (~( 0x1f<<8 ));
	regvalue  = regvalue | (StrobeShift << 8);

	WriteIODW(&pRegister->PDM_CTRL, regvalue);	// just write operation make pending clear
}

void NX_PDM_DMAMode( U32 ModuleIndex, U32 DMAMode )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( DMAMode < 2 ); // 1 bit : 2

	regvalue  = ReadIODW(&pRegister->PDM_CTRL);
	regvalue  = regvalue & (~( 0x1<<2 ));
	regvalue  = regvalue | (DMAMode << 2);

	WriteIODW(&pRegister->PDM_CTRL, regvalue);	// just write operation make pending clear
}


void NX_PDM_StartEnable( U32 ModuleIndex, CBOOL Enb )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );

	regvalue  = ReadIODW(&pRegister->PDM_CTRL);


	if( Enb == CTRUE )	regvalue  = regvalue | (0x1<<1);
	else				regvalue  = regvalue & (~( 0x1<<1 ));

	WriteIODW(&pRegister->PDM_CTRL, regvalue);	// just write operation make pending clear
}



void NX_PDM_SetShiftPerPixel( U32 ModuleIndex, U32 NUM )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue  = ReadIODW(&pRegister->PDM_CTRL1);
	regvalue = regvalue & (~(0x7 << 16));
	regvalue = regvalue | ((NUM&0x7)<<16);

	WriteIODW(&pRegister->PDM_CTRL1, regvalue);	// just write operation make pending clear
}

void NX_PDM_SetNumOfClock( U32 ModuleIndex, U32 NUM )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue  = ReadIODW(&pRegister->PDM_CTRL1);
	regvalue = regvalue & (~(0xff << 8));
	regvalue = regvalue | ((NUM&0xff)<<8);

	WriteIODW(&pRegister->PDM_CTRL1, regvalue);	// just write operation make pending clear
}

void NX_PDM_SetSamplePosition( U32 ModuleIndex, U32 NUM )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue  = ReadIODW(&pRegister->PDM_CTRL1);
	regvalue = regvalue & (~(0xff << 0));
	regvalue = regvalue | ((NUM&0xff)<<0);

	WriteIODW(&pRegister->PDM_CTRL1, regvalue);	// just write operation make pending clear
}


//@}
void NX_PDM_SetCTRL1( U32 ModuleIndex, U32 Shift, U32 Numof, U32 Sample )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue = ((Shift&0x7)<<16) | ((Numof&0xff)<<8) | ((Sample&0xff)<<0);
	WriteIODW(&pRegister->PDM_CTRL1, regvalue);	// just write operation make pending clear

}


//====================
// PDM RegTest Function
// 자체적으로 R/W를 하면서 확인한다.
//====================
#include <nx_simio.h>

CBOOL NX_PDM_CheckReg( U32 Addr, U32 initvalue, U32 writevalue, char *RegName);

// Macro 만들면 좋을텐데.. 모르겠다. 2013-01-22 오후 5:12:14
//	Addr = (U32) &pRegister->PDM_CTRL;
//	initvalue = 0x00; writevalue = (0x7F<<16) | (0xF<<8) | (1<<2) | (1<<1) | (1<<0);
//	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_CTRL") )
//	{
//		Result = CFALSE;
//	}


CBOOL NX_PDM_RegTest( U32 ModuleIndex )
{
	volatile register NX_PDM_RegisterSet* pRegister = 0;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_PDM_MODULE > ModuleIndex );
	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );


	//---------------
	// Init Value Check.
	//---------------
	CBOOL Result = CTRUE ;
	U32 Addr, initvalue, writevalue;

//    volatile U32 PDM_CTRL	    ;
//    volatile U32 PDM_GAIN0	    ;
//    volatile U32 PDM_GAIN1	    ;
//    volatile U32 PDM_COEFF	    ;
//    volatile U32 PDM_DATA	    ;
//    volatile U32 PDM_CTRL1      ;
//    volatile U32 PDM_IRQCTRL	;

	// PDM_CTRL
	Addr = (U32) &pRegister->PDM_CTRL;
	initvalue = 0x00; writevalue = (0x7F<<16) | (0xF<<8) | (1<<2) | (1<<1) | (1<<0);
	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_CTRL") )
	{
		Result = CFALSE;
	}

	// PDM_GAIN0
	Addr = (U32) &pRegister->PDM_GAIN0;
	initvalue = 0x00; writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_GAIN0") )
	{
		Result = CFALSE;
	}

	// PDM_GAIN1
	Addr = (U32) &pRegister->PDM_GAIN1;
	initvalue = 0x00; writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_GAIN1") )
	{
		Result = CFALSE;
	}

	// PDM_DATA : FIFO Data이므로 읽기 불가능.

	// PDM_CTRL1
	Addr = (U32) &pRegister->PDM_CTRL1;
	initvalue = 0x00; writevalue = (0x7<<16) | (0xFF<<8) | (0xFF<<0);
	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_CTRL1") )
	{
		Result = CFALSE;
	}

	// PDM_IRQCTRL
	Addr = (U32) &pRegister->PDM_IRQCTRL;
	initvalue = 0x00; writevalue =  (0x1F<<0);
	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_IRQCTRL") )
	{
		Result = CFALSE;
	}

	return Result;

}

CBOOL NX_PDM_CheckReg( U32 Addr, U32 initvalue, U32 writevalue, char *RegName)
{
	CBOOL Result = CTRUE;

	//------
	// Init Value Check.
	//------
	U32 regvalue = ReadIODW( Addr );
	if( regvalue != initvalue )
	{
		NX_CONSOLE_Printf("\n[ERROR] %s Register's initial value Error ( read = %x, golden = %x )",
			RegName, regvalue, initvalue ) ;
		Result = CFALSE;
	}

	//------
	// write value Check.
	//------
	WriteIODW( Addr, writevalue );
	regvalue = ReadIODW( Addr );
	if( regvalue != writevalue )
	{
		NX_CONSOLE_Printf("\n[ERROR] %s Register write Error ( read = %x, golden = %x )",
			RegName, regvalue, writevalue ) ;
		Result = CFALSE;
	}


	return Result;
}


