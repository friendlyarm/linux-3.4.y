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
//	File		: nx_CRYPTO.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------

#include <nx_chip.h>
#include "nx_crypto.h"
#include <string.h> // for memset

static	NX_CRYPTO_RegisterSet *__g_pRegister[NUMBER_OF_CRYPTO_MODULE];
	

//------------------------------------------------------------------------------
//
//	CRYPTO Interface
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------
//---------- CLKGEN 을 위한 prototype
U32 NX_CRYPTO_GetClockNumber (U32 ModuleIndex)
{
    static const U32 CLKGEN_LIST[] = { CLOCKINDEX_LIST( CRYPTO ) };
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
    return (U32)CLKGEN_LIST[ModuleIndex];	
}

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.
 *	@see	NX_CRYPTO_GetNumberOfModule
 */
CBOOL	NX_CRYPTO_Initialize( void )
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
 *				It is equal to NUMBER_OF_CRYPTO_MODULE in <nx_chip.h>.
 *	@see		NX_CRYPTO_Initialize
 */
U32		NX_CRYPTO_GetNumberOfModule( void )
{
	return NUMBER_OF_CRYPTO_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_CRYPTO_GetPhysicalAddress,
 *				NX_CRYPTO_SetBaseAddress,			NX_CRYPTO_GetBaseAddress,
 *				NX_CRYPTO_OpenModule,				NX_CRYPTO_CloseModule,
 *				NX_CRYPTO_CheckBusy,				
 */
U32		NX_CRYPTO_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_CRYPTO_RegisterSet );	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_CRYPTO_GetPhysicalAddress,		NX_CRYPTO_GetSizeOfRegisterSet,
 *				NX_CRYPTO_GetBaseAddress,
 *				NX_CRYPTO_OpenModule,				NX_CRYPTO_CloseModule,
 *				NX_CRYPTO_CheckBusy,				
 */
void	NX_CRYPTO_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_CRYPTO_RegisterSet *)BaseAddress;	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_CRYPTO_GetPhysicalAddress,		NX_CRYPTO_GetSizeOfRegisterSet,
 *				NX_CRYPTO_SetBaseAddress,
 *				NX_CRYPTO_OpenModule,				NX_CRYPTO_CloseModule,
 *				NX_CRYPTO_CheckBusy,				
 */
void*	NX_CRYPTO_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];	
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. \n
 *				It is equal to PHY_BASEADDR_CRYPTO?_MODULE in <nx_chip.h>.
 *	@see		NX_CRYPTO_GetSizeOfRegisterSet,
 *				NX_CRYPTO_SetBaseAddress,			NX_CRYPTO_GetBaseAddress,
 *				NX_CRYPTO_OpenModule,				NX_CRYPTO_CloseModule,
 *				NX_CRYPTO_CheckBusy,				
 */
U32		NX_CRYPTO_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_CRYPTO_MODULE] =
	{
		PHY_BASEADDR_LIST( CRYPTO )
	};
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];
		
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 *	@see		NX_CRYPTO_GetPhysicalAddress,		NX_CRYPTO_GetSizeOfRegisterSet,
 *				NX_CRYPTO_SetBaseAddress,			NX_CRYPTO_GetBaseAddress,
 *				NX_CRYPTO_CloseModule,
 *				NX_CRYPTO_CheckBusy,				
 */
CBOOL	NX_CRYPTO_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_CRYPTO_GetPhysicalAddress,		NX_CRYPTO_GetSizeOfRegisterSet,
 *				NX_CRYPTO_SetBaseAddress,			NX_CRYPTO_GetBaseAddress,
 *				NX_CRYPTO_OpenModule,
 *				NX_CRYPTO_CheckBusy,				
 */
CBOOL	NX_CRYPTO_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_CRYPTO_GetPhysicalAddress,		NX_CRYPTO_GetSizeOfRegisterSet,
 *				NX_CRYPTO_SetBaseAddress,			NX_CRYPTO_GetBaseAddress,
 *				NX_CRYPTO_OpenModule,				NX_CRYPTO_CloseModule,
 */
// No CheckBusy
//CBOOL	NX_CRYPTO_CheckBusy( U32 ModuleIndex )
//{
//    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
//    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
//    // @todo Implement
//	return CFALSE;
//}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_CRYPTO?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_CRYPTO_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( CRYPTO, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_CRYPTO_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );  
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
U32 NX_CRYPTO_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( CRYPTO ) };
    NX_CASSERT( NUMBER_OF_CRYPTO_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );  
    NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
    
    return InterruptNumber[ModuleIndex];
}


void	NX_CRYPTO_SetInterruptEnable( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	
	if( Enb )	regvalue  = regvalue | (1<<10);	
	else		regvalue  = regvalue & (~(1<<10));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetInterruptMask( U32 ModuleIndex, U32 Mask )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mask <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	
	if( Mask )	regvalue  = regvalue | (1<<9);	
	else		regvalue  = regvalue & (~(1<<9));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}






//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		\b CTRUE	indicates that one or more interrupts are pended. \r\n
 *				\b CFALSE	indicates that no interrupt is pended.
 *	@see		NX_CRYPTO_GetInterruptNumber,
 *				NX_CRYPTO_SetInterruptEnable,
 *				NX_CRYPTO_GetInterruptEnable,
 *				NX_CRYPTO_GetInterruptPending,
 *				NX_CRYPTO_ClearInterruptPending,
 *				NX_CRYPTO_SetInterruptEnableAll,
 *				NX_CRYPTO_GetInterruptEnableAll,	
 *				NX_CRYPTO_ClearInterruptPendingAll,
 *				NX_CRYPTO_GetInterruptPendingNumber

 */
CBOOL	NX_CRYPTO_GetInterruptPendingAll( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	if( (regvalue >> 0) && 0x01 )   return CTRUE;
	else 							return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 *	@see		NX_CRYPTO_GetInterruptNumber,
 *				NX_CRYPTO_SetInterruptEnable,
 *				NX_CRYPTO_GetInterruptEnable,
 *				NX_CRYPTO_GetInterruptPending,
 *				NX_CRYPTO_ClearInterruptPending,
 *				NX_CRYPTO_SetInterruptEnableAll,
 *				NX_CRYPTO_GetInterruptEnableAll,	
 *				NX_CRYPTO_GetInterruptPendingAll,
 *				NX_CRYPTO_GetInterruptPendingNumber

 */
void	NX_CRYPTO_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0) & (~0x1f);
	regvalue  = regvalue | (1<<5) ;
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	// just write operation make pending clear
}



//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index
 *	@param[in]	ModuleIndex	an index of module. 
 *  @param[in]  DMAChannel 	// CRYPDMA_BR = 0, CRYPDMA_BW = 1, CRYPDMA_HR = 2,
 *	@return		DMA peripheral index.
 *	@see		NX_DMA_TransferMemToIO(DestinationPeriID),
 *				NX_DMA_TransferIOToMem(SourcePeriID)
 */
U32 NX_CRYPTO_GetDMANumber ( U32 ModuleIndex , NX_CRYPTO_DMACHANNEL Channel)
{
	const U32 DMANumber[][NUMBER_OF_CRYPTO_MODULE] = 
	{
		{ DMAINDEX_WITH_CHANNEL_LIST(CRYPTO,CRYPDMA_BR) },
		{ DMAINDEX_WITH_CHANNEL_LIST(CRYPTO,CRYPDMA_BW) },
		{ DMAINDEX_WITH_CHANNEL_LIST(CRYPTO,CRYPDMA_HR) },
	};
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( Channel < 3 );
	
	return DMANumber[Channel][ModuleIndex]; 
}

U32 NX_CRYPTO_GetDMABusWidth( )
{
	return 32; // 32 bit
}


//------------------------------------------------------------------------------
//	No PAD Interface
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
///	@name	Basic Use Functions
//------------------------------------------------------------------------------
//@{
void	NX_CRYPTO_SetAESTextIn			( U32 ModuleIndex, 
	U32 AES_TIN0, U32 AES_TIN1, U32 AES_TIN2, U32 AES_TIN3 ) // AES_TIN0, AES_TIN1, AES_TIN2, AES_TIN3
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_AES_TIN0, AES_TIN0);
	WriteIO32(&pRegister->CRYPTO_AES_TIN1, AES_TIN1);
	WriteIO32(&pRegister->CRYPTO_AES_TIN2, AES_TIN2);
	WriteIO32(&pRegister->CRYPTO_AES_TIN3, AES_TIN3);
}

void	NX_CRYPTO_SetAESInitVec			( U32 ModuleIndex, 
	U32 AES_IV0,  U32 AES_IV1,  U32 AES_IV2,  U32 AES_IV3 ) // AES_IV0, AES_IV1, AES_IV2, AES_IV3
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_AES_IV0, AES_IV0);
	WriteIO32(&pRegister->CRYPTO_AES_IV1, AES_IV1);
	WriteIO32(&pRegister->CRYPTO_AES_IV2, AES_IV2);
	WriteIO32(&pRegister->CRYPTO_AES_IV3, AES_IV3);
}

void	NX_CRYPTO_SetAESKey				( U32 ModuleIndex, 
	U32 AES_KEY0, U32 AES_KEY1, U32 AES_KEY2, U32 AES_KEY3, 
	U32 AES_KEY4, U32 AES_KEY5, U32 AES_KEY6, U32 AES_KEY7 ) // AES_KEY0, AES_KEY1, AES_KEY2, AES_KEY3, AES_KEY4, AES_KEY5, AES_KEY6, AES_KEY7
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_AES_KEY0, AES_KEY0);
	WriteIO32(&pRegister->CRYPTO_AES_KEY1, AES_KEY1);
	WriteIO32(&pRegister->CRYPTO_AES_KEY2, AES_KEY2);
	WriteIO32(&pRegister->CRYPTO_AES_KEY3, AES_KEY3);
	WriteIO32(&pRegister->CRYPTO_AES_KEY4, AES_KEY4);
	WriteIO32(&pRegister->CRYPTO_AES_KEY5, AES_KEY5);
	WriteIO32(&pRegister->CRYPTO_AES_KEY6, AES_KEY6);
	WriteIO32(&pRegister->CRYPTO_AES_KEY7, AES_KEY7);
}

void	NX_CRYPTO_SetAESEfuseKeyMode		( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<15);	
	else		regvalue  = regvalue & (~(1<<15));	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}
void	NX_CRYPTO_SetAESOutputSwap			( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<9);	
	else		regvalue  = regvalue & (~(1<<9));	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetAESInputSwap			( U32 ModuleIndex, CBOOL Enb ) // 지금은 넣을때 BigEndian으로 넣어주고 있다.
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<8);	
	else		regvalue  = regvalue & (~(1<<8));	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetAESBlockCiphterMode	( U32 ModuleIndex, U32 Mode )  // Block Cipher Mode-> 0:ECB, 1:CBC, 2:CTR
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 2 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
		
	regvalue = regvalue & (~(0x3<<6));	
	regvalue = regvalue | (Mode<<6);
	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetAESMode			( U32 ModuleIndex, U32 Mode ) // AES Mode->0:128, 1:192, 2:256
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 2 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
		
	regvalue = regvalue & (~(0x3<<4));	
	regvalue = regvalue | (Mode<<4);
	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}

void NX_CRYPTO_SetAES64bitCounter( U32 ModuleIndex, U32 Mode ) // 64bit counter
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
		
	regvalue = regvalue & (~(0x1<<3));	
	regvalue = regvalue | (Mode<<3);
	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}
void	NX_CRYPTO_SetAESDMAMode			( U32 ModuleIndex, U32 Mode ) // DMA Mode->0:FIFO mode, 1:DMA mode
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
		
	regvalue = regvalue & (~(0x1<<2));	
	regvalue = regvalue | (Mode<<2);
	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetAESEncoderMode		( U32 ModuleIndex, U32 Mode ) // AES Encoder Mode->0:Decoder, 1: Encoder
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
		
	regvalue = regvalue & (~(0x1<<1));	
	regvalue = regvalue | (Mode<<1);
	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}


void	NX_CRYPTO_SetAESEnable			( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_AES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<0);	
	else		regvalue  = regvalue & (~(1<<0));	
	WriteIO32(&pRegister->CRYPTO_AES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetHASHRun			( U32 ModuleIndex, CBOOL Enb ) // HASH Run : Disable
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];
	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	if( Enb )	regvalue  = regvalue | (1<<2);	
	//else		regvalue  = regvalue & (~(1<<2));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetDESRun				( U32 ModuleIndex, CBOOL Enb ) // DES Run
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];
	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	if( Enb )	regvalue  = regvalue | (1<<1);	
	else		regvalue  = regvalue & (~(1<<1));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetAESRun				( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	if( Enb )	regvalue  = regvalue | (1<<0);	
	else		regvalue  = regvalue & (~(1<<0));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}

void	NX_CRYPTO_GetAESTextOut			( U32 ModuleIndex, 
	U32 *AES_TOUT0, U32* AES_TOUT1, U32 *AES_TOUT2, U32* AES_TOUT3 ) // AES_TOUT0, AES_TOUT1, AES_TOUT2, AES_TOUT3
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	*AES_TOUT0 = ReadIO32(&pRegister->CRYPTO_AES_TOUT0);
	*AES_TOUT1 = ReadIO32(&pRegister->CRYPTO_AES_TOUT1);
	*AES_TOUT2 = ReadIO32(&pRegister->CRYPTO_AES_TOUT2);
	*AES_TOUT3 = ReadIO32(&pRegister->CRYPTO_AES_TOUT3);
}


void	NX_CRYPTO_SetLoadDESIV			( U32 ModuleIndex, CBOOL Enb ) // Load DES IV
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	
	if( Enb )	regvalue  = regvalue | (1<<4);	
	else		regvalue  = regvalue & (~(1<<4));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetLoadAESIV			( U32 ModuleIndex, CBOOL Enb ) // Load AES IV
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	//@modified choiyk 2012-11-15 오후 4:03:19
	// 해당 신호들은 토글용 신호들이다.
	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue  = regvalue & (~(0x00ff));
	
	if( Enb )	regvalue  = regvalue | (1<<3);	
	else		regvalue  = regvalue & (~(1<<3));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}


CBOOL NX_CRYPTO_GetIdleAES( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	
	if( (regvalue & 0x01) == 1 ) return CTRUE;
	else                         return CFALSE; // AES Idel State.
}


CBOOL	NX_CRYPTO_GetIdleHASH( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	
	if( ((regvalue>>2) & 0x01) == 1 ) return CTRUE;
	else                         return CFALSE; // AES Idel State.
}

CBOOL	NX_CRYPTO_GetIdleDES ( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	
	if( ((regvalue>>1) & 0x01) == 1 ) return CTRUE;
	else                         return CFALSE; // AES Idel State.
}


void NX_CRYPTO_SetDESTextIn            ( U32 ModuleIndex, U32 DES_TIN0, U32 DES_TIN1 ) // DES_TIN0, DES_TIN1
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_DES_TIN0, DES_TIN0);
	WriteIO32(&pRegister->CRYPTO_DES_TIN1, DES_TIN1);
}

void NX_CRYPTO_SetDESInitVec           ( U32 ModuleIndex, U32 DES_IV0,  U32 DES_IV1)
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_DES_IV0, DES_IV0);
	WriteIO32(&pRegister->CRYPTO_DES_IV1, DES_IV1);
}

void NX_CRYPTO_SetDESKeyIn0            ( U32 ModuleIndex, U32 DES_KEY0_0, U32 DES_KEY0_1 )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_DES_KEY0_0, DES_KEY0_0);
	WriteIO32(&pRegister->CRYPTO_DES_KEY0_1, DES_KEY0_1);
}

void NX_CRYPTO_SetDESKeyIn1            ( U32 ModuleIndex, U32 DES_KEY1_0, U32 DES_KEY1_1 )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_DES_KEY1_0, DES_KEY1_0);
	WriteIO32(&pRegister->CRYPTO_DES_KEY1_1, DES_KEY1_1);
}

void NX_CRYPTO_SetDESKeyIn2            ( U32 ModuleIndex, U32 DES_KEY2_0, U32 DES_KEY2_1 )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_DES_KEY2_0, DES_KEY2_0);
	WriteIO32(&pRegister->CRYPTO_DES_KEY2_1, DES_KEY2_1);
}


void NX_CRYPTO_SetTDESMode             ( U32 ModuleIndex, U32 TDESMODE ) // TDESMODE
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_DES_CTRL0) & (~(0x7 << 8));
	
	regvalue  = regvalue | ((TDESMODE&0x7)<<8);	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}


void NX_CRYPTO_SetDESOutputSwap        ( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<6);	
	else		regvalue  = regvalue & (~(1<<6));	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetDESInputSwap			( U32 ModuleIndex, CBOOL Enb ) // 지금은 넣을때 BigEndian으로 넣어주고 있다.
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<5);	
	else		regvalue  = regvalue & (~(1<<5));	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetDESBlockCiphterMode	( U32 ModuleIndex, U32 Mode )  // Block Cipher Mode-> 0:ECB, 1:CBC, 2:CTR
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
		
	regvalue = regvalue & (~(0x01<<4));	
	regvalue = regvalue | (Mode<<4);
	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetDESMode			( U32 ModuleIndex, U32 Mode ) // AES Mode->0:128, 1:192, 2:256
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
		
	regvalue = regvalue & (~(0x01<<2));	
	regvalue = regvalue | (Mode<<2);
	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetDESDMAMode			( U32 ModuleIndex, U32 Mode ) // DMA Mode->0:FIFO mode, 1:DMA mode
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
		
	regvalue = regvalue & (~(0x1<<3));	
	regvalue = regvalue | (Mode<<3);
	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}

void	NX_CRYPTO_SetDESEncoderMode		( U32 ModuleIndex, U32 Mode ) // AES Encoder Mode->0:Decoder, 1: Encoder
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
		
	regvalue = regvalue & (~(0x1<<1));	
	regvalue = regvalue | (Mode<<1);
	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}


void	NX_CRYPTO_SetDESEnable			( U32 ModuleIndex, CBOOL Enb )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_DES_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<0);	
	else		regvalue  = regvalue & (~(1<<0));	
	WriteIO32(&pRegister->CRYPTO_DES_CTRL0, regvalue);	
}


void NX_CRYPTO_SetDMAWritePath         ( U32 ModuleIndex, U32 Mode ) // DMA Write Path : 0:AES, 1:DES		
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	regvalue = regvalue & (~(0x00ff));
	
	if( Mode == 1 )	regvalue  = regvalue | (1<<8);	
	else			regvalue  = regvalue & (~(1<<8));	
	WriteIO32(&pRegister->CRYPTO_CRT_CTRL0, regvalue);	
}


void NX_CRYPTO_SetMSZEE0               ( U32 ModuleIndex, U32 HASH_MSZE0, U32 HASH_MSZE1 )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->CRYPTO_HASH_MSZE0, HASH_MSZE0);	
	WriteIO32(&pRegister->CRYPTO_HASH_MSZE1, HASH_MSZE1);	
}


void NX_CRYPTO_SetINITTABLE            ( U32 ModuleIndex, U32 HASH_IV0, 
										U32 HASH_IV1, U32 HASH_IV2, U32 HASH_IV3, U32 HASH_IV4 )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->CRYPTO_HASH_IV0, HASH_IV0);	
	WriteIO32(&pRegister->CRYPTO_HASH_IV1, HASH_IV1);	
	WriteIO32(&pRegister->CRYPTO_HASH_IV2, HASH_IV2);	
	WriteIO32(&pRegister->CRYPTO_HASH_IV3, HASH_IV3);	
	WriteIO32(&pRegister->CRYPTO_HASH_IV4, HASH_IV4);	

}


void NX_CRYPTO_SetHASHDMAPath          ( U32 ModuleIndex, U32 Mode ) // DMA Path
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 3 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_HASH_CTRL0);
		
	regvalue = regvalue & (~(0x3<<5));	
	regvalue = regvalue | (Mode<<5);
	
	WriteIO32(&pRegister->CRYPTO_HASH_CTRL0, regvalue);	
}

void NX_CRYPTO_SetHASHInputSwap        ( U32 ModuleIndex, CBOOL Enb ) // Input Swap 
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Enb == CTRUE || Enb == CFALSE );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_HASH_CTRL0);
	
	if( Enb )	regvalue  = regvalue | (1<<4);	
	else		regvalue  = regvalue & (~(1<<4));	
	WriteIO32(&pRegister->CRYPTO_HASH_CTRL0, regvalue);	
}


void NX_CRYPTO_SetHASHMode             ( U32 ModuleIndex, U32 Mode ) // 0:SHA1, 1:MD5
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_HASH_CTRL0);
		
	regvalue = regvalue & (~(0x1<<3));	
	regvalue = regvalue | (Mode<<3);
	
	WriteIO32(&pRegister->CRYPTO_HASH_CTRL0, regvalue);	
}


void NX_CRYPTO_SetHASHCont             ( U32 ModuleIndex, U32 Mode ) // HASH Cont 
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_HASH_CTRL0);
		
	regvalue = regvalue & (~(0x1<<2));	
	regvalue = regvalue | (Mode<<2);
	
	WriteIO32(&pRegister->CRYPTO_HASH_CTRL0, regvalue);	
}


void NX_CRYPTO_SetDMAMode              ( U32 ModuleIndex, U32 Mode ) // DMA Mode->0:FIFO mode, 1:DMA mode
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_HASH_CTRL0);
		
	regvalue = regvalue & (~(0x1<<1));	
	regvalue = regvalue | (Mode<<1);
	
	WriteIO32(&pRegister->CRYPTO_HASH_CTRL0, regvalue);	
}


void NX_CRYPTO_SetHASHEnable           ( U32 ModuleIndex, U32 Mode )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( Mode <= 1 );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->CRYPTO_HASH_CTRL0);
		
	regvalue = regvalue & (~(0x1<<0));	
	regvalue = regvalue | (Mode<<0);
	
	WriteIO32(&pRegister->CRYPTO_HASH_CTRL0, regvalue);	
}
	

void NX_CRYPTO_GetDESTextOut ( U32 ModuleIndex, U32 *DES_TOUT0, U32 *DES_TOUT1 )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	*DES_TOUT0 = ReadIO32(&pRegister->CRYPTO_DES_TOUT0);
	*DES_TOUT1 = ReadIO32(&pRegister->CRYPTO_DES_TOUT1);
}


void NX_CRYPTO_GetHASHTextOut( U32 ModuleIndex, 
	U32 *HASH_TOUT0, U32 *HASH_TOUT1, U32 *HASH_TOUT2, U32 *HASH_TOUT3, U32 *HASH_TOUT4 )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	*HASH_TOUT0 = ReadIO32(&pRegister->CRYPTO_HASH_TOUT0);
	*HASH_TOUT1 = ReadIO32(&pRegister->CRYPTO_HASH_TOUT1);
	*HASH_TOUT2 = ReadIO32(&pRegister->CRYPTO_HASH_TOUT2);
	*HASH_TOUT3 = ReadIO32(&pRegister->CRYPTO_HASH_TOUT3);
	*HASH_TOUT4 = ReadIO32(&pRegister->CRYPTO_HASH_TOUT4);
}	



void	NX_CRYPTO_SetHASHTextIn			( U32 ModuleIndex, U32 HASH_TIN ) 
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	
	WriteIO32(&pRegister->CRYPTO_HASH_TIN, HASH_TIN);
}



//i_IdleHASHCore
CBOOL NX_CRYPTO_GetIdleHASHCore( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;
	register U32 regvalue;
	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	regvalue  = ReadIO32(&pRegister->CRYPTO_CRT_CTRL0);
	
	if( ((regvalue>>7) & 0x01) == 1 ) return CTRUE;
	else                         return CFALSE; // AES Idel State.
}

#ifdef SOC_TEST
//====================
// CRYPTO RegTest Function 
// 자체적으로 R/W를 하면서 확인한다. 
//====================
#include <nx_simio.h>



// Macro 만들면 좋을텐데.. 모르겠다. 2013-01-22 오후 5:12:14 
//	Addr = (U32) &pRegister->PDM_CTRL;
//	initvalue = 0x00; writevalue = (0x7F<<16) | (0xF<<8) | (1<<2) | (1<<1) | (1<<0);
//	if( CFALSE == NX_PDM_CheckReg( Addr, initvalue, writevalue, "PDM_CTRL") )
//	{
//		Result = CFALSE;
//	}


CBOOL NX_CRYPTO_RegTest( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];


	//---------------
	// Init Value Check.
	//---------------
	CBOOL Result = CTRUE ;
	U32 Addr, initvalue, writevalue;

	
	initvalue = 0x87;
	Addr = (U32) &pRegister->CRYPTO_CRT_CTRL0   ;    writevalue = (1<<10) | (1<<9) | (1<<8) | (0x87<<0);
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_CRT_CTRL0") )
	{
		Result = CFALSE;
	}
	
	initvalue = 0x00;
	
	Addr = (U32) &pRegister->CRYPTO_AES_CTRL0   ;    writevalue = 0x3FF | (1<<15);
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_CTRL0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_IV0     ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_IV0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_IV1     ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_IV1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_IV2     ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_IV2") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_IV3     ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_IV3") )
	{
		Result = CFALSE;
	}
	
	
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY0    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY1    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY2    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY2") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY3    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY3") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY4    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY4") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY5    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY5") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY6    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY6") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_KEY7    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_KEY7") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_TIN0    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_TIN0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_TIN1    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_TIN1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_TIN2    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_TIN2") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_AES_TIN3    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_AES_TIN3") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_CTRL0   ;    writevalue = 0x7F | (0x7<<8);
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_CTRL0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_IV0     ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_IV0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_IV1     ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_IV1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_KEY0_0  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_KEY0_0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_KEY0_1  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_KEY0_1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_KEY1_0  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_KEY1_0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_KEY1_1  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_KEY1_1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_KEY2_0  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_KEY2_0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_KEY2_1  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_KEY2_1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_TIN0    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_TIN0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_DES_TIN1    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_TIN1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_CTRL0  ;    writevalue = 0x7b;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_CTRL0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_IV0    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_IV0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_IV1    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_IV1") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_IV2    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_IV2") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_IV3    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_IV3") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_IV4    ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_IV4") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_MSZE0  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_MSZE0") )
	{
		Result = CFALSE;
	}
	
	Addr = (U32) &pRegister->CRYPTO_HASH_MSZE1  ;    writevalue = 0xFFFFFFFF;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_HASH_MSZE1") )
	{
		Result = CFALSE;
	}
	
	
	//Addr = (U32) &pRegister->CRYPTO_AES_TOUT0   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_AES_TOUT1   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_AES_TOUT2   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_AES_TOUT3   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_DES_TOUT0   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_DES_TOUT1   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_CRT_BDMAR   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_CRT_BDMAW   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_CRT_HDMAR   ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_HASH_TOUT0  ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_HASH_TOUT1  ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_HASH_TOUT2  ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_HASH_TOUT3  ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_HASH_TOUT4  ;  writevalue = 
	//Addr = (U32) &pRegister->CRYPTO_HASH_TIN    ;  writevalue = 
	
	return Result;
	
}

CBOOL NX_CRYPTO_CheckReg( U32 Addr, U32 initvalue, U32 writevalue, char *RegName)
{
	CBOOL Result = CTRUE;
	
	//------
	// Init Value Check.
	//------
	U32 regvalue = ReadIO32( Addr );
	if( regvalue != initvalue )
	{
		NX_CONSOLE_Printf("\n[ERROR] %s Register's initial value Error ( read = %x, golden = %x )", 
			RegName, regvalue, initvalue ) ;
		Result = CFALSE; 
	} 
	
	//------
	// write value Check. 
	//------
	WriteIO32( Addr, writevalue );
	regvalue = ReadIO32( Addr );
	if( regvalue != writevalue )
	{
		NX_CONSOLE_Printf("\n[ERROR] %s Register write Error ( read = %x, golden = %x )", 
			RegName, regvalue, writevalue ) ;
		Result = CFALSE; 
	}
	
	
	return Result;
}



//==================
// @choiyk 2013-01-29 오후 7:31:56 
// ECO Point Check. 
//==================
CBOOL NX_CRYPTO_RegTest_ECO_20130129( U32 ModuleIndex )
{
	register NX_CRYPTO_RegisterSet* pRegister;

	NX_ASSERT( NUMBER_OF_CRYPTO_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

	//---------------
	// Init Value Check.
	//---------------
	CBOOL Result = CTRUE ;
	U32 Addr, initvalue, writevalue;
	
	initvalue = 0x00;

	Addr = (U32) &pRegister->CRYPTO_DES_CTRL0   ;    writevalue = 1<<3 ;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_CTRL0 - DMA MODE ( PWDATA[3] )") )
	{
		Result = CFALSE;
	}
	WriteIO32( Addr, 0 ); // Init 


	Addr = (U32) &pRegister->CRYPTO_DES_CTRL0   ;    writevalue = 1<<5 ;
	if( CFALSE == NX_CRYPTO_CheckReg( Addr, initvalue, writevalue, "CRYPTO_DES_CTRL0 - SWAPIN MODE ( PWDATA[5] )") )
	{
		Result = CFALSE;
	}
	WriteIO32( Addr, 0 ); // Init 
	
	return Result;
	
}
#endif
