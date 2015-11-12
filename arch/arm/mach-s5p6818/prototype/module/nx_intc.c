//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: Interrupt Controller
//	File		: nx_intc.c
//	Description	:
//	Author		:
//	History		:
//					2007/04/03	first
// 
//------------------------------------------------------------------------------
#include "nx_intc.h"

static	struct NX_INTC_RegisterSet (*__g_pRegister)[0];

//------------------------------------------------------------------------------
//	Module Interface
//------------------------------------------------------------------------------

/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicate that Initialize is successed.
 *			 CFALSE	indicate that Initialize is failed.
 *	@see	NX_INTC_GetNumberOfModule
 */
CBOOL	NX_INTC_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = CNULL;
		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_INTC_Initialize
 */
U32		NX_INTC_GetNumberOfModule( void )
{
	return NUMBER_OF_INTC_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32		NX_INTC_GetPhysicalAddress( void )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( INTC ) }; // PHY_BASEADDR_INTC?_MODULE
	NX_CASSERT( NUMBER_OF_INTC_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );

	NX_ASSERT( PHY_BASEADDR_INTC_MODULE == PhysicalAddr[0] );

	return (U32)PhysicalAddr[0];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_INTC_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_INTC_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_INTC_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_INTC_RegisterSet (*)[])BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void* 	NX_INTC_GetBaseAddress( void )
{
	return (void* )__g_pRegister;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_INTC_OpenModule( void )
{
	register U32 i;
	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		WriteIO32(&__g_pRegister[i]->INTMODE,  0);
	}
	for(i=0; i<NX_INTC_NUM_OF_INT; i++)
	{
		WriteIO32(&__g_pRegister[i>>5]->VECTORADDR[i], i);
	}
	NX_INTC_SetInterruptEnableAll( CFALSE );
	NX_INTC_ClearInterruptPendingAll();
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_INTC_CloseModule( void )
{
	NX_INTC_SetInterruptEnableAll( CFALSE );
	NX_INTC_ClearInterruptPendingAll();
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.

 */
CBOOL	NX_INTC_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		 CTRUE	indicate that Ready to enter power-down stage. 
 *				 CFALSE	indicate that This module can't enter to power-down stage.
 */
CBOOL	NX_INTC_CanPowerDown( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number .
 *	@param[in]	Enable	 CTRUE	indicate that Interrupt Enable. 
 *						 CFALSE	indicate that Interrupt Disable.
 *	@return		None.
 */
void	NX_INTC_SetInterruptEnable( S32 IntNum, CBOOL Enable )
{
	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	NX_ASSERT( (CFALSE==Enable) || (CTRUE==Enable) );

	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[IntNum >> 5];
	NX_ASSERT( CNULL != pRegister );

	if (Enable)
	{
		WriteIO32(&pRegister->INTENABLE, (1UL << (IntNum & 0x1F)));
	}
	else
	{
		WriteIO32(&pRegister->INTDISABLE, (1UL << (IntNum & 0x1F)));
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		 CTRUE	indicate that Interrupt is enabled. 
 *				 CFALSE	indicate that Interrupt is disabled.
 */
CBOOL	NX_INTC_GetInterruptEnable( S32 IntNum )
{
	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[IntNum >> 5];
	register U32 regvalue;

	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->INTENABLE) & (~ ReadIO32(&pRegister->INTDISABLE));

	if( regvalue & ( 1UL << (IntNum & 0x1F)) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	EnableHigh	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							-EnableHigh[0] : I2C0_______, EnableHigh[10] : _____________, EnableHigh[20] : OHCI_____, EnableHigh[30] : ECTOUCH 
 *							-EnableHigh[1] : I2C1_______, EnableHigh[11] : TIMER4_______, EnableHigh[21] : _________, EnableHigh[31] : - 
 *							-EnableHigh[2] : UART1______, EnableHigh[12] : _____________, EnableHigh[22] : _________
 *							-EnableHigh[3] : ___________, EnableHigh[13] : _____________, EnableHigh[23] : _________
 *							-EnableHigh[4] : ___________, EnableHigh[14] : I2S__________, EnableHigh[24] : _________
 *							-EnableHigh[5] : ___________, EnableHigh[15] : _____________, EnableHigh[25] : _________
 *							-EnableHigh[6] : ___________, EnableHigh[16] : _____________, EnableHigh[26] : _________
 *							-EnableHigh[7] : ___________, EnableHigh[17] : CDROM________, EnableHigh[27] : _________
 *							-EnableHigh[8] : ___________, EnableHigh[18] : ALIVE________, EnableHigh[28] : _________
 *							-EnableHigh[9] : CSC________, EnableHigh[19] : EHCI_________, EnableHigh[29] : _________
 *
 *	@param[in]	EnableLow	Specify interrupt bit for enable of disable. Each bit's meaning is like below	
 *							-EnableLow[0] : PDISPLAY____, EnableLow[10] : UART0_____, EnableLow[20] : UDC___________, EnableLow[30] : SCALER	
 *							-EnableLow[1] : ____________, EnableLow[11] : TIMER1____, EnableLow[21] : TIMER3________, EnableLow[31] : RTC		
 *							-EnableLow[2] : VIP_________, EnableLow[12] : SSPSPI____, EnableLow[22] : DEINTERLACE___
 *							-EnableLow[3] : DMA_________, EnableLow[13] : GPIO______, EnableLow[23] : PPM___________
 *							-EnableLow[4] : TIMER0______, EnableLow[14] : SDMMC0____, EnableLow[24] : AUDIOIF_______
 *							-EnableLow[5] : SYSCTRL_____, EnableLow[15] : TIMER2____, EnableLow[25] : ADC___________
 *							-EnableLow[6] : ____________, EnableLow[16] : H264______, EnableLow[26] : MCUSTATIC_____
 *							-EnableLow[7] : MPEGTSI_____, EnableLow[17] : MPEG______, EnableLow[27] : GRP3D_________
 *							-EnableLow[8] : ____________, EnableLow[18] : __________, EnableLow[28] : UHC___________
 *							-EnableLow[9] : ____________, EnableLow[19] : VLC_______, EnableLow[29] : ROTATOR_______
 *	@return		None.
 *	@see		NX_INTC_SetInterruptEnable,			NX_INTC_GetInterruptEnable,
 *													NX_INTC_GetInterruptEnable64,
 *				NX_INTC_GetInterruptPending,		NX_INTC_GetInterruptPending64,
 *				NX_INTC_ClearInterruptPending,		NX_INTC_ClearInterruptPending64,
 *				NX_INTC_SetInterruptEnableAll,		NX_INTC_GetInterruptEnableAll,
 *				NX_INTC_GetInterruptPendingAll,		NX_INTC_ClearInterruptPendingAll,
 *				NX_INTC_GetInterruptPendingNumber
 */
//void	NX_INTC_SetInterruptEnable64( U32 EnableHigh, U32 EnableLow )
//{
//	const U32 INTMASKL_MASK = ~((1<<6)|(1<<8)|(1<<9)|(1<<18));		// Reserved Region
//	const U32 INTMASKH_MASK = ~((1<<12)|(1UL<<31)|(((1<<10)-1)<<21));	// Reserved Region
//
//	register struct NX_INTC_RegisterSet *pRegister;
//	NX_ASSERT( CNULL != __g_pRegister );
//
//	pRegister = __g_pRegister;
//
////	pRegister->INTMASK[0] = EnableLow  & INTMASKL_MASK;
////	pRegister->INTMASK[1] = EnableHigh & INTMASKH_MASK;
//	WriteIO32(&pRegister->INTMASK[0], EnableLow	 & INTMASKL_MASK);
//	WriteIO32(&pRegister->INTMASK[1], EnableHigh & INTMASKH_MASK);
//}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[out]	pEnableHigh		High part setting value of interrupt. 
 *								-pEnableHigh[0] : I2C0_______, pEnableHigh[10] : _____________, pEnableHigh[20] : OHCI_____, pEnableHigh[30] : ECTOUCH 
 *								-pEnableHigh[1] : I2C1_______, pEnableHigh[11] : TIMER4_______, pEnableHigh[21] : _________, pEnableHigh[31] : - 
 *								-pEnableHigh[2] : UART1______, pEnableHigh[12] : _____________, pEnableHigh[22] : _________
 *								-pEnableHigh[3] : ___________, pEnableHigh[13] : _____________, pEnableHigh[23] : _________
 *								-pEnableHigh[4] : ___________, pEnableHigh[14] : I2S__________, pEnableHigh[24] : _________
 *								-pEnableHigh[5] : ___________, pEnableHigh[15] : _____________, pEnableHigh[25] : _________
 *								-pEnableHigh[6] : ___________, pEnableHigh[16] : _____________, pEnableHigh[26] : _________
 *								-pEnableHigh[7] : ___________, pEnableHigh[17] : CDROM________, pEnableHigh[27] : _________
 *								-pEnableHigh[8] : ___________, pEnableHigh[18] : ALIVE________, pEnableHigh[28] : _________
 *								-pEnableHigh[9] : CSC________, pEnableHigh[19] : EHCI_________, pEnableHigh[29] : _________
 *
 *	@param[out]	pEnableLow		Low part	setting value of interrupt. 
 *								-pEnableLow[0] : PDISPLAY____, pEnableLow[10] : UART0_____, pEnableLow[20] : UDC___________, pEnableLow[30] : SCALER	
 *								-pEnableLow[1] : ____________, pEnableLow[11] : TIMER1____, pEnableLow[21] : TIMER3________, pEnableLow[31] : RTC		
 *								-pEnableLow[2] : VIP_________, pEnableLow[12] : SSPSPI____, pEnableLow[22] : DEINTERLACE___
 *								-pEnableLow[3] : DMA_________, pEnableLow[13] : GPIO______, pEnableLow[23] : PPM___________
 *								-pEnableLow[4] : TIMER0______, pEnableLow[14] : SDMMC0____, pEnableLow[24] : AUDIOIF_______
 *								-pEnableLow[5] : SYSCTRL_____, pEnableLow[15] : TIMER2____, pEnableLow[25] : ADC___________
 *								-pEnableLow[6] : ____________, pEnableLow[16] : H264______, pEnableLow[26] : MCUSTATIC_____
 *								-pEnableLow[7] : MPEGTSI_____, pEnableLow[17] : MPEG______, pEnableLow[27] : GRP3D_________
 *								-pEnableLow[8] : ____________, pEnableLow[18] : __________, pEnableLow[28] : UHC___________
 *								-pEnableLow[9] : ____________, pEnableLow[19] : VLC_______, pEnableLow[29] : ROTATOR_______
 *	@return		None.
 */
//void	NX_INTC_GetInterruptEnable64( U32 *pEnableHigh, U32 *pEnableLow )
//{
//	const U32 INTMASKL_MASK = ~((1<<6)|(1<<8)|(1<<9)|(1<<18));		// Reserved Region
//	const U32 INTMASKH_MASK = ~((1<<12)|(1UL<<31)|(((1<<10)-1)<<21));	// Reserved Region
//
//	register struct NX_INTC_RegisterSet *pRegister;
//	NX_ASSERT( CNULL != __g_pRegister );
//
//	pRegister = __g_pRegister;
//
//	*pEnableLow	= pRegister->INTMASK[0] & INTMASKL_MASK;
//	*pEnableHigh = pRegister->INTMASK[1] & INTMASKH_MASK;
//}


//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		 CTRUE	indicate that Pending is seted. 
 *				 CFALSE	indicate that Pending is Not Seted.
 */
CBOOL	NX_INTC_GetInterruptPending( S32 IntNum )
{
	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[IntNum >> 5];

	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	NX_ASSERT( CNULL != pRegister );

	if( ReadIO32(&pRegister->IRQSTATUS) & ( 1UL << (IntNum & 0x1F)) )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[out]	pPendingHigh	Interrupt controller's pending bit of HIGH part. 
 *								-pPendingHigh[0] : I2C0_______, pPendingHigh[10] : SDMMC1_______, pPendingHigh[20] : OHCI_____, pPendingHigh[30] : ECTOUCH 
 *								-pPendingHigh[1] : I2C1_______, pPendingHigh[11] : TIMER4_______, pPendingHigh[21] : _________, pPendingHigh[31] : - 
 *								-pPendingHigh[2] : UART1______, pPendingHigh[12] : _____________, pPendingHigh[22] : _________
 *								-pPendingHigh[3] : ___________, pPendingHigh[13] : _____________, pPendingHigh[23] : _________
 *								-pPendingHigh[4] : ___________, pPendingHigh[14] : I2S__________, pPendingHigh[24] : _________
 *								-pPendingHigh[5] : ___________, pPendingHigh[15] : _____________, pPendingHigh[25] : _________
 *								-pPendingHigh[6] : ___________, pPendingHigh[16] : _____________, pPendingHigh[26] : _________
 *								-pPendingHigh[7] : ___________, pPendingHigh[17] : CDROM________, pPendingHigh[27] : _________
 *								-pPendingHigh[8] : ___________, pPendingHigh[18] : ALIVE________, pPendingHigh[28] : _________
 *								-pPendingHigh[9] : CSC________, pPendingHigh[19] : EHCI_________, pPendingHigh[29] : _________
 *
 *	@param[out]	pPendingLow		Interrupt controller's pending bit of LOW part.
 *								-pPendingLow[0] : PDISPLAY____, pPendingLow[10] : UART0_____, pPendingLow[20] : UDC___________, pPendingLow[30] : SCALER	
 *								-pPendingLow[1] : ____________, pPendingLow[11] : TIMER1____, pPendingLow[21] : TIMER3________, pPendingLow[31] : RTC		
 *								-pPendingLow[2] : VIP_________, pPendingLow[12] : SSPSPI____, pPendingLow[22] : DEINTERLACE___
 *								-pPendingLow[3] : DMA_________, pPendingLow[13] : GPIO______, pPendingLow[23] : PPM___________
 *								-pPendingLow[4] : TIMER0______, pPendingLow[14] : SDMMC0____, pPendingLow[24] : AUDIOIF_______
 *								-pPendingLow[5] : SYSCTRL_____, pPendingLow[15] : TIMER2____, pPendingLow[25] : ADC___________
 *								-pPendingLow[6] : ____________, pPendingLow[16] : H264______, pPendingLow[26] : MCUSTATIC_____
 *								-pPendingLow[7] : MPEGTSI_____, pPendingLow[17] : MPEG______, pPendingLow[27] : GRP3D_________
 *								-pPendingLow[8] : ____________, pPendingLow[18] : __________, pPendingLow[28] : UHC___________
 *								-pPendingLow[9] : ____________, pPendingLow[19] : VLC_______, pPendingLow[29] : ROTATOR_______
 *	@return		None.
 */
//void	NX_INTC_GetInterruptPending64( U32 *pPendingHigh, U32 *pPendingLow )
//{
//	const U32 INTPENDL_MASK = ~((1<<6)|(1<<8)|(1<<9)|(1<<18));			// Reserved Region
//	const U32 INTPENDH_MASK = ~((1<<12)|(1UL<<31)|(((1<<10)-1)<<21));	// Reserved Region
//
//	register struct NX_INTC_RegisterSet *pRegister;
//	NX_ASSERT( CNULL != __g_pRegister );
//
//	pRegister = __g_pRegister;
//
//	*pPendingLow  = pRegister->INTPEND[0] & INTPENDL_MASK;
//	*pPendingHigh =	pRegister->INTPEND[1] & INTPENDH_MASK;
//}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 */
void	NX_INTC_ClearInterruptPending( S32 IntNum )
{
    // there is no-pending register @Gamza 2012-07-03
	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	register U32 i = IntNum / 32;
	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
	WriteIO32(&pRegister->CURRENTVECTOR, 0xFFFFFFFF);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	PendingHigh		Interrupt controller's HIGH part pending bit to clear.
 *								-PendingHigh[0] : I2C0_______, PendingHigh[10] : _____________, PendingHigh[20] : OHCI_____, PendingHigh[30] : ECTOUCH 
 *								-PendingHigh[1] : I2C1_______, PendingHigh[11] : TIMER4_______, PendingHigh[21] : _________, PendingHigh[31] : - 
 *								-PendingHigh[2] : ___________, PendingHigh[12] : _____________, PendingHigh[22] : _________
 *								-PendingHigh[3] : ___________, PendingHigh[13] : _____________, PendingHigh[23] : _________
 *								-PendingHigh[4] : ___________, PendingHigh[14] : I2S__________, PendingHigh[24] : _________
 *								-PendingHigh[5] : ___________, PendingHigh[15] : _____________, PendingHigh[25] : _________
 *								-PendingHigh[6] : ___________, PendingHigh[16] : _____________, PendingHigh[26] : _________
 *								-PendingHigh[7] : ___________, PendingHigh[17] : CDROM________, PendingHigh[27] : _________
 *								-PendingHigh[8] : ___________, PendingHigh[18] : ALIVE________, PendingHigh[28] : _________
 *								-PendingHigh[9] : CSC________, PendingHigh[19] : EHCI_________, PendingHigh[29] : _________
 *
 *	@param[in]	PendingLow		Interrupt controller's LOW part pending bit to Clear. 
 *								-PendingLow[0] : PDISPLAY____, PendingLow[10] : UART0_____, PendingLow[20] : UDC___________, PendingLow[30] : SCALER	
 *								-PendingLow[1] : ____________, PendingLow[11] : TIMER1____, PendingLow[21] : TIMER3________, PendingLow[31] : RTC		
 *								-PendingLow[2] : VIP_________, PendingLow[12] : SSPSPI____, PendingLow[22] : DEINTERLACE___
 *								-PendingLow[3] : DMA_________, PendingLow[13] : GPIO______, PendingLow[23] : PPM___________
 *								-PendingLow[4] : TIMER0______, PendingLow[14] : SDMMC0____, PendingLow[24] : AUDIOIF_______
 *								-PendingLow[5] : SYSCTRL_____, PendingLow[15] : TIMER2____, PendingLow[25] : ADC___________
 *								-PendingLow[6] : ____________, PendingLow[16] : H264______, PendingLow[26] : MCUSTATIC_____
 *								-PendingLow[7] : MPEGTSI_____, PendingLow[17] : MPEG______, PendingLow[27] : GRP3D_________
 *								-PendingLow[8] : ____________, PendingLow[18] : __________, PendingLow[28] : UHC___________
 *								-PendingLow[9] : ____________, PendingLow[19] : VLC_______, PendingLow[29] : ROTATOR_______
 *	@return		None.
 */
//void	NX_INTC_ClearInterruptPending64( U32 PendingHigh, U32 PendingLow )
//{
//	const U32 INTPENDL_MASK = ~((1<<6)|(1<<8)|(1<<9)|(1<<18));			// Reserved Region
//	const U32 INTPENDH_MASK = ~((1<<12)|(1UL<<31)|(((1<<10)-1)<<21));	// Reserved Region
//
//	register struct NX_INTC_RegisterSet *pRegister;
//	NX_ASSERT( CNULL != __g_pRegister );
//
//	pRegister = __g_pRegister;
//
////	pRegister->INTPEND[0] = PendingLow	& INTPENDL_MASK;
////	pRegister->INTPEND[1] = PendingHigh & INTPENDH_MASK;
//	WriteIO32(&pRegister->INTPEND[0], PendingLow  & INTPENDL_MASK);
//	WriteIO32(&pRegister->INTPEND[1], PendingHigh & INTPENDH_MASK);
//}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	Enable	 CTRUE	indicate that Set to all interrupt enable. 
 *						 CFALSE	indicate that Set to all interrupt disable.
 *	@return		None.
 */
void	NX_INTC_SetInterruptEnableAll( CBOOL Enable )
{
	register U32 i;

	NX_ASSERT( (CFALSE==Enable) || (CTRUE==Enable) );

	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
	    NX_ASSERT( CNULL != pRegister );
		if( Enable )
		{
			WriteIO32(&pRegister->INTENABLE, 0xFFFFFFFF);
		}
		else
		{
			WriteIO32(&pRegister->INTDISABLE, 0xFFFFFFFF);
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		 CTRUE	indicate that At least one( or more ) interrupt is enabled. 
 *				 CFALSE	indicate that All interrupt is disabled.
 */
CBOOL	NX_INTC_GetInterruptEnableAll( void )
{
	register U32 i;
	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
		NX_ASSERT( CNULL != pRegister );

		if( 0 != ( ReadIO32(&pRegister->INTENABLE) & (~ ReadIO32(&pRegister->INTDISABLE)) ) )
        {
            return CTRUE;
		}
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@return		 CTRUE	indicate that At least one( or more ) pending is seted. 
 *				 CFALSE	indicate that All pending is NOT seted.
 */
CBOOL	NX_INTC_GetInterruptPendingAll( void )
{
	register U32 i;
	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
		NX_ASSERT( CNULL != pRegister );

		if( 0 != ( ReadIO32(&pRegister->IRQSTATUS) ) )
        {
            return CTRUE;
		}
	}
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@return		None.
 */
void	NX_INTC_ClearInterruptPendingAll( void )
{
    // there is no-pending register @Gamza 2012-07-03
	register U32 i;
	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
		WriteIO32(&pRegister->CURRENTVECTOR, 0xFFFFFFFF);
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@return		Pending Number( If all pending is not set then return -1 ).
 */
S32		NX_INTC_GetInterruptPendingNumber( void )	// -1 if None
{
	register U32 i;

	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
	    NX_ASSERT( CNULL != pRegister );

		register U32 intpend = ReadIO32(&pRegister->IRQSTATUS);
		register U32 intnum;

        for( intnum = 0; 0!=intpend && intnum<32; intnum++ )
        {
    		if( intpend&0x1 )
    		{
    			return (i<<5) | intnum;
    		}
    		intpend>>=1;
    	}
	}
	return -1;
}


//------------------------------------------------------------------------------
//Interrupt Controller Operation.
//------------------------------------------------------------------------------
/**
 *	@brief		Set an interrupt mode to be generated when the corresponding interrupt is issued.
 *	@param[in]	IntNum		An interrupt number.
 *	@param[in]	IntMode		An interrupt mode, IRQ or FIQ.
 *	@return		None.
 */
void	NX_INTC_SetInterruptMode ( U32 IntNum, NX_INTC_INTMODE IntMode	)
{
	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[IntNum>>5];
	register U32 value;

	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	NX_ASSERT( CNULL != pRegister );

	value = ReadIO32(&pRegister->INTMODE);
	switch( IntMode )
	{
	case NX_INTC_INTMODE_IRQ:
		value &= ~(1<<(IntNum&0x1F));
		break;
	case NX_INTC_INTMODE_FIQ:
		value |= 1<<(IntNum&0x1F);
		break;
	default:
		NX_ASSERT( CFALSE );
	}
	WriteIO32(&pRegister->INTMODE, value);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a priority control.
 *	@param[in]	ArbiterNum	An arbiter number to be changed, 0 ~ 11 are valid.
 *	@param[in]	OrderSel	Determines the order of priority manually.
 *	@return		None.
 *	@remark		Arbiter0	: for interrupt	0 ~	5 
 *				Arbiter1	: for interrupt	6 ~ 11 
 *				Arbiter2	: for interrupt 12 ~ 17 
 *				Arbiter3	: for interrupt 18 ~ 23 
 *				Arbiter4	: for interrupt 24 ~ 29 
 *				Arbiter5	: for interrupt 30 ~ 35 
 *				Arbiter6	: for interrupt 36 ~ 41 
 *				Arbiter7	: for interrupt 42 ~ 47 
 *				Arbiter8	: for interrupt 48 ~ 53 
 *				Arbiter9	: for interrupt 54 ~ 59 
 *				Arbiter10 : for interrupt 60 ~ 63 
 *				Arbiter11 : arbiter 0 ~ 10
 */
void	NX_INTC_SetPriorityMode( U32 ArbiterNum, U32 OrderSel )
{
    NX_ASSERT( CFALSE ); // TODO
    ArbiterNum = ArbiterNum;
    OrderSel = OrderSel;    
	return;

//	register struct NX_INTC_RegisterSet *pRegister;
//	register U32 reg;
//
//	NX_ASSERT( CNULL != __g_pRegister );
//	NX_ASSERT( 12 > ArbiterNum );
//	NX_ASSERT( ((4 > OrderSel) && (11 > ArbiterNum)) || ((8 > OrderSel) && (11 == ArbiterNum)) );
//
//	pRegister = __g_pRegister;
//
//	reg = pRegister->PRIORDER;
//	if( 11 > ArbiterNum )	reg &= ~(3UL<<(ArbiterNum<<1));
//	else					reg &= ~(7UL<<(ArbiterNum<<1));
//	reg |= (OrderSel<<(ArbiterNum<<1));
//
////	pRegister->PRIORDER = reg;
//	WriteIO32(&pRegister->PRIORDER, reg);
}

void NX_INTC_SetIntHandler(U32 IntNum, void *Handler)
{
	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[IntNum >> 5];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->VECTORADDR[IntNum & 0x1F], Handler);
}

void *NX_INTC_GetIntHandler(U32 IntNum)
{
	NX_ASSERT( NX_INTC_NUM_OF_INT > IntNum );
	register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[IntNum >> 5];
	    NX_ASSERT( CNULL != pRegister );

	return (void (*)(void*)) ReadIO32(&pRegister->VECTORADDR[IntNum & 0x1F]);
}

void *NX_INTC_GetCurrentIntHandler(void)
{
	register U32 i;
	for(i=0; i<((NX_INTC_NUM_OF_INT+31)/32); i++)
	{
		register struct NX_INTC_RegisterSet *pRegister = __g_pRegister[i];
		if(ReadIO32(&pRegister->IRQSTATUS))
			return (void*)ReadIO32(&pRegister->CURRENTVECTOR);
	}
	return (void*)-1;
}
