//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: Dma Controller
//	File		: nx_dma.c
//	Description	:
//	Author		:
//	History		:
//					2007/04/03	first
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_dma.h"

#define CHANNELBUFFERSIZE 4096

//static	struct NX_DMA_RegisterSet *__g_pRegister = CNULL;
static	struct
{
	struct NX_DMA_RegisterSet *pRegister;
} __g_ModuleVariables[NUMBER_OF_DMA_MODULE] = { {CNULL,}, };

static NX_Memory1D g_DMA_COMMANDBuffer[NUMBER_OF_DMA_MODULE];

//------------------------------------------------------------------------------
//	Module Interface
//------------------------------------------------------------------------------

/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return \b CTRUE	indicate that Initialize is successed.\n
 *			\b CFALSE	indicate that Initialize is failed.
 *	@see	NX_DMA_GetNumberOfModule
 */
CBOOL	NX_DMA_Initialize( void )
{
	//	@modified Gamza static variable(__g_ModuleVariables) is automatically filled by '0'
	//					만약 초기화 과정에 전역변수를 0으로 초기화 하는 작업 이외의 일을
	//					해야한다면 bInit 값을 CFALSE로 수정해야한다.
	static CBOOL bInit = CTRUE;
	//register struct NX_DMA_RegisterSet *pRegister;
	U32 i, j;

	//NX_CONSOLE_Init();

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_DMA_MODULE;i++)
		{
			__g_ModuleVariables[i].pRegister = CNULL;
		}
		bInit = CTRUE;
	}
	for(j=0; j<NUMBER_OF_DMA_MODULE;j++)
	{
		//for(i=0; i<NUMBER_OF_DMA_CHANNEL ; i++)
		//{
		//	NX_Malloc1D( 0, 4096, &g_DMA_COMMANDBuffer[j][i] );
		//}
		NX_Malloc1D( 0, CHANNELBUFFERSIZE * NUMBER_OF_DMA_CHANNEL, &g_DMA_COMMANDBuffer[j] );
	}
	//NX_CONSOLE_Printf("g_DMA_COMMANDBuffer[0] : 0x%x\n", g_DMA_COMMANDBuffer[0].Address );

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_DMA_Initialize
 */
U32		NX_DMA_GetNumberOfModule( void )
{
	return NUMBER_OF_DMA_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 *	@see										NX_DMA_GetSizeOfRegisterSet,
 *				NX_DMA_SetBaseAddress,			NX_DMA_GetBaseAddress,
 *				NX_DMA_OpenModule,				NX_DMA_CloseModule,
 *				NX_DMA_CheckBusy,				NX_DMA_CanPowerDown
 */
U32		NX_DMA_GetPhysicalAddress( U32 ModuleIndex )
{
	const U32 PhysicalAddr[] =
	{
		PHY_BASEADDR_LIST( DMA )
	};
	NX_CASSERT( NUMBER_OF_DMA_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );

	return	PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 *	@see		NX_DMA_GetPhysicalAddress,
 *				NX_DMA_SetBaseAddress,			NX_DMA_GetBaseAddress,
 *				NX_DMA_OpenModule,				NX_DMA_CloseModule,
 *				NX_DMA_CheckBusy,				NX_DMA_CanPowerDown
 */
U32		NX_DMA_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DMA_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 *	@see		NX_DMA_GetPhysicalAddress,		NX_DMA_GetSizeOfRegisterSet,
 *												NX_DMA_GetBaseAddress,
 *				NX_DMA_OpenModule,				NX_DMA_CloseModule,
 *				NX_DMA_CheckBusy,				NX_DMA_CanPowerDown
 */
void	NX_DMA_SetBaseAddress( U32 ModuleIndex, U32 BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
	__g_ModuleVariables[ModuleIndex].pRegister = (struct NX_DMA_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 *	@see		NX_DMA_GetPhysicalAddress,		NX_DMA_GetSizeOfRegisterSet,
 *				NX_DMA_SetBaseAddress,
 *				NX_DMA_OpenModule,				NX_DMA_CloseModule,
 *				NX_DMA_CheckBusy,				NX_DMA_CanPowerDown
 */
U32		NX_DMA_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );

	return (U32)__g_ModuleVariables[ModuleIndex].pRegister;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		\b CTRUE	indicate that Initialize is successed. \n
 *				\b CFALSE	indicate that Initialize is failed.
 *	@see		NX_DMA_GetPhysicalAddress,		NX_DMA_GetSizeOfRegisterSet,
 *				NX_DMA_SetBaseAddress,			NX_DMA_GetBaseAddress,
 *												NX_DMA_CloseModule,
 *				NX_DMA_CheckBusy,				NX_DMA_CanPowerDown
 */
CBOOL	NX_DMA_OpenModule(U32 ModuleIndex)
{
    register struct NX_DMA_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	WriteIODW(&pRegister->Configuration, 0x1);
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		\b CTRUE	indicate that Deinitialize is successed. \n
 *				\b CFALSE	indicate that Deinitialize is failed.
 *	@see		NX_DMA_GetPhysicalAddress,		NX_DMA_GetSizeOfRegisterSet,
 *				NX_DMA_SetBaseAddress,			NX_DMA_GetBaseAddress,
 *				NX_DMA_OpenModule,
 *				NX_DMA_CheckBusy,				NX_DMA_CanPowerDown
 */
CBOOL	NX_DMA_CloseModule( U32 ModuleIndex )
{
	U32 i;
    register struct NX_DMA_RegisterSet *pRegister;
	pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
	WriteIODW(&pRegister->Configuration, 0x0);
	NX_DMA_ClearInterruptPendingAll();

	for (i=0; i<NUMBER_OF_DMA_CHANNEL ; i++)	NX_DMA_Stop(i+(ModuleIndex*8), CTRUE);

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		\b CTRUE	indicate that Module is Busy. \n
 *				\b CFALSE	indicate that Module is NOT Busy.
 *	@see		NX_DMA_GetPhysicalAddress,		NX_DMA_GetSizeOfRegisterSet,
 *				NX_DMA_SetBaseAddress,			NX_DMA_GetBaseAddress,
 *				NX_DMA_OpenModule,				NX_DMA_CloseModule,
 *												NX_DMA_CanPowerDown
 */
CBOOL	NX_DMA_CheckBusy( void )
{
	U32 i;
	U32 CheckValue;

    CheckValue = 0;
    for (i=0; i<NUMBER_OF_DMA_CHANNEL*NUMBER_OF_DMA_MODULE ; i++)	CheckValue |= NX_DMA_CheckRunning(i);

    if (0 != CheckValue)	return CFALSE;
	return CTRUE;
}

U32		NX_DMA_GetInterruptNumber( U32 nChannel )
{
	const U32 INTNumber[NUMBER_OF_DMA_MODULE] =
	{
		INTNUM_LIST(DMA)
	};
	U32 		DMA_ModuleIndex = nChannel/8;

	NX_CASSERT( NUMBER_OF_DMA_MODULE == (sizeof(INTNumber)/sizeof(INTNumber[0])) );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );

	return INTNumber[DMA_ModuleIndex];
}
//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enable or disable.
 *	@param[in]	IntNum	Interrupt Number .
 *	@param[in]	Enable	\b CTRUE	indicate that Interrupt Enable. \n
 *						\b CFALSE	indicate that Interrupt Disable.
 *	@return		None.
 *	@see											NX_DMA_GetInterruptEnable,
 *				NX_DMA_GetInterruptPending,
 *				NX_DMA_ClearInterruptPending,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
void	NX_DMA_SetInterruptEnable( U32 nChannel, U32 IntNum, CBOOL Enable )
{
	const U32 INTENB_M		= 31; // module
	const U32 INTENB_C		= 14; // channel
	const U32 INTENB_C_MASK	= (3UL<<INTENB_C);
	const U32 INTENB_M_MASK	= (1UL<<INTENB_M);

	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	register struct NX_DMA_RegisterSet *pRegister;
	register	U32 regval;

	NX_ASSERT( NX_DMA_NUM_OF_INT > IntNum );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );
	NX_ASSERT( CNULL != __g_ModuleVariables );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

	regval = ReadIODW(&pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & ~INTENB_M_MASK;
	regval |= (Enable << INTENB_M);
	WriteIODW(&pRegister->Channel[DMA_ChannelIndex].SGLLI.Control, regval);
	regval = ReadIODW(&pRegister->Channel[DMA_ChannelIndex].Configuration) & ~INTENB_C_MASK;
	regval |= (Enable << (INTENB_C+IntNum));
	WriteIODW(&pRegister->Channel[DMA_ChannelIndex].Configuration, regval);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicate that Interrupt is enabled. \n
 *				\b CFALSE	indicate that Interrupt is disabled.
 *	@see		NX_DMA_SetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
CBOOL	NX_DMA_GetInterruptEnable( U32 nChannel, U32 IntNum )
{
	register struct NX_DMA_RegisterSet *pRegister;
	register U32 regvalue;
	const U32 INTENB_POS	= 14;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( NX_DMA_NUM_OF_INT > IntNum );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	//pRegister = __g_pRegister;
	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

	regvalue = ReadIODW(&pRegister->Channel[DMA_ChannelIndex].Configuration);

	if( regvalue & ( 1UL << (IntNum+INTENB_POS)) )
	{
		return CFALSE;
	}

	return CTRUE;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	IntNum	Interrupt Number.
 *	@return		\b CTRUE	indicate that Pending is seted. \n
 *				\b CFALSE	indicate that Pending is Not Seted.
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *													NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
CBOOL	NX_DMA_GetInterruptPending( U32 nChannel , U32 IntNum )
{
	register struct NX_DMA_RegisterSet *pRegister;
	//register U32 regvalue;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

	if( ReadIODW(&pRegister->IntStatus) & 1UL<<DMA_ChannelIndex )
	{
		return CTRUE;
	}

	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	IntNum	Interrupt number.
 *	@return		None.
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *													NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
void	NX_DMA_ClearInterruptPending( U32 nChannel, U32 IntNum )
{
	register struct NX_DMA_RegisterSet *pRegister;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;
	//register U32 regvalue;

	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

	if (0 != IntNum)	WriteIODW(&pRegister->IntTCClear, 1UL<<DMA_ChannelIndex);
	else				WriteIODW(&pRegister->IntErrClr, 1UL<<DMA_ChannelIndex);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enables or disables.
 *	@param[in]	Enable	\b CTRUE	indicate that Set to all interrupt enable. \n
 *						\b CFALSE	indicate that Set to all interrupt disable.
 *	@return		None.
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *													NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
void	NX_DMA_SetInterruptEnableAll( CBOOL Enable )
{
	register struct NX_DMA_RegisterSet *pRegister;
	register U32 regvalue;
	const U32 INTENB_M	= 31;
	const U32 INTENB_C	= 14;
	const U32 INTENB_M_MASK	= (1UL<<INTENB_M);
	const U32 INTENB_C_MASK	= (3UL<<INTENB_C);
	int i, j;

	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( (0==Enable) || (1==Enable) );

	//NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	for (j=0 ; j<NUMBER_OF_DMA_MODULE; j++)
	{
		pRegister = __g_ModuleVariables[j].pRegister;

		for (i=0; i<NUMBER_OF_DMA_CHANNEL;i++)
		{
			regvalue = ReadIODW(&pRegister->Channel[i].SGLLI.Control) & ~INTENB_M_MASK;
			regvalue |= (Enable << INTENB_M);
			WriteIODW(&pRegister->Channel[i].SGLLI.Control, regvalue);
			regvalue = ReadIODW(&pRegister->Channel[i].Configuration) & ~INTENB_C_MASK;
			regvalue |= (Enable << INTENB_C) | (Enable << (INTENB_C+1));
			WriteIODW(&pRegister->Channel[i].Configuration, regvalue);
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enable or not.
 *	@return		\b CTRUE	indicate that At least one( or more ) interrupt is enabled. \n
 *				\b CFALSE	indicate that All interrupt is disabled.
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
CBOOL	NX_DMA_GetInterruptEnableAll( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@return		\b CTRUE	indicate that At least one( or more ) pending is seted. \n
 *				\b CFALSE	indicate that All pending is NOT seted.
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *													NX_DMA_ClearInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
CBOOL	NX_DMA_GetInterruptPendingAll( void )
{
	register struct NX_DMA_RegisterSet *pRegister;
	NX_ASSERT( CNULL != __g_ModuleVariables );
	//NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	int j;
	for (j=0; j<NUMBER_OF_DMA_MODULE; j++)
	{
		pRegister = __g_ModuleVariables[j].pRegister;

		if(ReadIODW(&pRegister->IntStatus))
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
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,
 *				NX_DMA_GetInterruptPendingNumber
 */
void	NX_DMA_ClearInterruptPendingAll( void )
{
	register struct NX_DMA_RegisterSet *pRegister;
	int j;
	NX_ASSERT( CNULL != __g_ModuleVariables );
	//NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	for (j=0; j<NUMBER_OF_DMA_MODULE; j++)
	{
		pRegister = __g_ModuleVariables[j].pRegister;
		WriteIODW(&pRegister->IntTCClear, 0xFF );
		WriteIODW(&pRegister->IntErrClr, 0xFF );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts
 *	@return		Pending Number( If all pending is not set then return -1 ).
 *	@see		NX_DMA_SetInterruptEnable,			NX_DMA_GetInterruptEnable,
 *				NX_DMA_SetInterruptEnable64,		NX_DMA_GetInterruptEnable64,
 *				NX_DMA_GetInterruptPending,		NX_DMA_GetInterruptPending64,
 *				NX_DMA_ClearInterruptPending,		NX_DMA_ClearInterruptPending64,
 *				NX_DMA_SetInterruptEnableAll,		NX_DMA_GetInterruptEnableAll,
 *				NX_DMA_GetInterruptPendingAll,		NX_DMA_ClearInterruptPendingAll
 */
U32		NX_DMA_GetInterruptPendingNumber( U32 nChannel )	// -1 if None
{
	register struct NX_DMA_RegisterSet *pRegister;
	register U32 regvalue;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	NX_ASSERT( CNULL != __g_ModuleVariables );

	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

	regvalue = ReadIODW(&pRegister->IntTCStatus);
	regvalue &= (1UL << DMA_ChannelIndex);
	if (0 != regvalue)	return 1;
	regvalue = ReadIODW(&pRegister->IntErrorStatus);
	regvalue &= (1UL << DMA_ChannelIndex);
	if (0 != regvalue)	return 0;
	else				return -1;
}


//------------------------------------------------------------------------------
// DMA Configuration Function
//------------------------------------------------------------------------------
/**
 *	@brief		Transfer memory to memory.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	pSource			source buffer address.\n this value must be aligned by 64 bits(8 bytes).
 *	@param[in]	pDestination	destination buffer address.\n this value must be aligned by 64 bits(8 bytes).
 *	@param[in]	TransferSize	transfer size in bytes.
 *	@return		None.
 *	@see										NX_DMA_TransferMemToIO,
 *				NX_DMA_TransferIOToMem,			NX_DMA_GetMaxTransferSize,
 *				NX_DMA_SetSourceAddress,		NX_DMA_GetSourceAddress,
 *				NX_DMA_SetDestinationAddress,	NX_DMA_GetDestinationAddress,
 *				NX_DMA_SetDataSize,				NX_DMA_GetDataSize,
 *				NX_DMA_SetIOIndex,				NX_DMA_GetIOIndex,
 *				NX_DMA_SetAttribute
 */
void	NX_DMA_TransferMemToMem( U32 nChannel, const void* pSource, void* pDestination, U32 TransferSize )
{
	U32			INTENB;
	//register	struct tag_ModuleVariables*		pVariables;
	U32			CmdBufferAddr;
	U32 		regvalue;
	U32 		Number_of_LLI;
	U32 		LLI_Count;
	U32 		CurTransferSize;

	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;
	register struct NX_DMA_RegisterSet *pRegister;
	//register	U32	regvalue;

	NX_ASSERT( 0 == (((U32)pSource) % 8) );
	NX_ASSERT( 0 == (((U32)pDestination) % 8) );

	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	NX_ASSERT( CNULL != __g_ModuleVariables);
	NX_ASSERT( CNULL != g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	CmdBufferAddr = g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address+(CHANNELBUFFERSIZE*DMA_ChannelIndex);

	//MAXTransferSize = (16*1024)-4;
	U32 MAXTransferSize;
	MAXTransferSize = (16*1024)-4;
	Number_of_LLI = TransferSize/(MAXTransferSize+4);
	#ifdef NX_DEBUGLOG
	NX_CONSOLE_Printf(" Req TransferSize : %d\n", TransferSize );
	NX_CONSOLE_Printf(" Number_of_LLI : %d\n", Number_of_LLI );
	#endif
	if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/4;
	else					CurTransferSize = TransferSize/4;

	TransferSize = TransferSize-MAXTransferSize;
	#ifdef NX_DEBUGLOG
	NX_CONSOLE_Printf(" CurTransferSize : 0x%x\n", CurTransferSize );
	NX_CONSOLE_Printf(" Nexet TransferSize : 0x%x\n", TransferSize );
	#endif

	//INTENB = NX_DMA_GetInterruptEnable(ModuleIndex, nChannel, 0);
	INTENB = NX_DMA_GetInterruptEnable(nChannel, 0);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.SRCADDR = (U32)pSource;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.DSTADDR = (U32)pDestination;

	regvalue = (pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & 0x80000000;

	INTENB = (regvalue>>31) & 1;

	regvalue |= (U32)( CurTransferSize		// Transfer size
					| (3<<26) // SBSize (source burst size)
					| (3<<12) // SBSize (source burst size)
					| (3<<15) // DBSize (destination burst size)
					| (2<<18) // SWidth (source transfer width)
					| (2<<21) // DWidth (destination transfer width)
					| (0UL<<24) // source master bus 0: AHB1, 1; AHB:2
					| (0UL<<25) // destination master bus
					);
	// tranfer size 가 16Kbyte-4byte 보다 클경우 LLI 의 마지막 에서 interrupt enable 한다.
	if (0 != Number_of_LLI)	regvalue = regvalue & ~(1UL<<31);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.Control = regvalue;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = 0x0;
	if (0 != Number_of_LLI)
	{
		LLI_Count = 1;
		while(1)
		{
			Number_of_LLI = TransferSize/(MAXTransferSize+4);
			if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/4;
			else					CurTransferSize = TransferSize/4;

			TransferSize = TransferSize-MAXTransferSize;
			// last link list 일때 interrupt enable 한다.
			if(0 == Number_of_LLI) regvalue |= (INTENB<<31);

			regvalue = (regvalue&~0xfff) | CurTransferSize;

			NX_DMA_Build_LLI( (U32)pSource+(LLI_Count*MAXTransferSize),
								(U32)pDestination+(LLI_Count*MAXTransferSize),
								(U32)regvalue,
								(U32)CmdBufferAddr+((LLI_Count-1)*4),
								(U32)Number_of_LLI);
		#ifdef NX_DEBUGLOG
			NX_CONSOLE_Printf(" Number_of_LLI[%d]\n", LLI_Count );
			NX_CONSOLE_Printf(" Number_of_LLI[%d] NexeLLI : %d\n", LLI_Count,  Number_of_LLI);
			NX_CONSOLE_Printf(" Number_of_LLI[%d] SrcAddr : 0x%x\n", LLI_Count,  pSource+(LLI_Count*MAXTransferSize));
			NX_CONSOLE_Printf(" Number_of_LLI[%d] DstAddr : 0x%x\n", LLI_Count,  pDestination+(LLI_Count*MAXTransferSize));
			NX_CONSOLE_Printf(" Number_of_LLI[%d] LLIADDR : 0x%x\n", LLI_Count,  CmdBufferAddr+(LLI_Count*4));
			NX_CONSOLE_Printf(" Number_of_LLI[%d] CurTransferSize : 0x%x\n", LLI_Count,  CurTransferSize);
			NX_CONSOLE_Printf(" Number_of_LLI[%d] Next TrSize : 0x%x\n", LLI_Count,  TransferSize);
		#endif
			if(0 == Number_of_LLI)
			{
				break;
			}
			LLI_Count++;
		}
		pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = CmdBufferAddr&~0x3;
	}
	regvalue = pRegister->Channel[DMA_ChannelIndex].Configuration & (0x3<<14);
	regvalue |= (U32)(1);
	pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;
}

void	NX_DMA_TransferMemToIO( U32 nChannel, const void* pSource, void* pDestination, U32 DestinationPeriID, U32 DestinationBitWidth, U32 TransferSize )
{
	//register	struct tag_ModuleVariables*		pVariables;
	U32			CmdBufferAddr;
	U32 		regvalue;
	U32 		Number_of_LLI;
	U32 		LLI_Count;
	U32 		CurTransferSize;
	U32 		INTENB;

	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;
	register struct NX_DMA_RegisterSet *pRegister;
	//register	U32	regvalue;

	NX_ASSERT( 0 == (((U32)pSource) % 8) );
	NX_ASSERT( 0 == (((U32)pDestination) % 2) );

	NX_ASSERT( CNULL != __g_ModuleVariables);
	NX_ASSERT( CNULL != g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	CmdBufferAddr = g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address + (CHANNELBUFFERSIZE*DMA_ChannelIndex);

	//MAXTransferSize = (16*1024)-4;
	U32 MAXTransferSize;
	MAXTransferSize = (16*1024)-4;
	Number_of_LLI = TransferSize/(MAXTransferSize+4);
	if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/4;
	else					CurTransferSize = TransferSize/4;

	TransferSize = TransferSize-MAXTransferSize;

	INTENB = NX_DMA_GetInterruptEnable(nChannel, 0);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.SRCADDR = (U32)pSource;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.DSTADDR = (U32)pDestination;

	regvalue = (pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & 0x80000000;

	INTENB = (regvalue>>31) & 1;

	regvalue |= (U32)( CurTransferSize		// Transfer size
					| (0<<12) // SBSize (source burst size)
					| (0<<15) // DBSize (destination burst size)
					| (2<<18) // SWidth (source transfer width)
					| ((DestinationBitWidth>>4)<<21) // DWidth (destination transfer width)
					| (0UL<<24) // source master bus 0: AHB1, 1; AHB:2
					| (1UL<<25) // destination master bus
					| (1UL<<26) // SI (source increment)
					| (0UL<<27)	// DI (destination increment)
					);
	// tranfer size 가 16Kbyte-4byte 보다 클경우 LLI 의 마지막 에서 interrupt enable 한다.
	if (0 != Number_of_LLI)	regvalue = regvalue & ~(1UL<<31);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.Control = regvalue;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = 0x0;
	if (0 != Number_of_LLI)
	{
		LLI_Count = 1;
		while(1)
		{
			Number_of_LLI = TransferSize/(MAXTransferSize+4);
			if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/4;
			else					CurTransferSize = TransferSize/4;

			TransferSize = TransferSize-MAXTransferSize;
			// last link list 일때 interrupt enable 한다.
			if(0 == Number_of_LLI) regvalue |= (INTENB<<31);

			regvalue = (regvalue&~0xfff) | CurTransferSize;

			NX_DMA_Build_LLI( (U32)pSource+(LLI_Count*MAXTransferSize),
								(U32)pDestination,
								(U32)regvalue,
								(U32)CmdBufferAddr+((LLI_Count-1)*4),
								(U32)Number_of_LLI);
			if(0 == Number_of_LLI)
			{
				break;
			}
			LLI_Count++;
		}
		pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = CmdBufferAddr&~0x3;
	}
	regvalue = pRegister->Channel[DMA_ChannelIndex].Configuration & (0x3<<14);
	regvalue |= (U32)(1
					| (DestinationPeriID<<6) // destination id
					| (1<<11) // mode
					);
	pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;

}
void	NX_DMA_TransferIOToMem( U32 nChannel, const void* pSource, U32 SourcePeriID, U32 SourceBitWidth, void* pDestination, U32 TransferSize )
{
	//register	struct tag_ModuleVariables*		pVariables;
	U32			CmdBufferAddr;
	U32 		regvalue;
	U32 		Number_of_LLI;
	U32 		LLI_Count;
	U32 		CurTransferSize;
	U32			INTENB;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;
	register struct NX_DMA_RegisterSet *pRegister;
	//register	U32	regvalue;

	NX_ASSERT( 0 == (((U32)pSource) % 2) );
	NX_ASSERT( 0 == (((U32)pDestination) % 8) );
    //MES_REQUIRE ( 64 > SourcePeriID );
    //MES_REQUIRE ( 8 == SourceBitWidth || 16 == SourceBitWidth || 32 == SourceBitWidth );

	NX_ASSERT( CNULL != __g_ModuleVariables);
	NX_ASSERT( CNULL != g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	CmdBufferAddr = g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address + (CHANNELBUFFERSIZE*DMA_ChannelIndex);

	//MAXTransferSize = (16*1024)-4;
	U32 Bytes = SourceBitWidth/8;
	U32 MAXTransferSize;
    if (8 == SourceBitWidth)		MAXTransferSize = (4*1024)-Bytes;
    else if (16 == SourceBitWidth)	MAXTransferSize = (8*1024)-Bytes;
    else							MAXTransferSize = (16*1024)-Bytes;
	//MAXTransferSize = (16*1024)-4;
	Number_of_LLI = TransferSize/(MAXTransferSize+Bytes);
	if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/Bytes;
	else					CurTransferSize = TransferSize/Bytes;

	TransferSize = TransferSize-MAXTransferSize;

	INTENB = NX_DMA_GetInterruptEnable(nChannel, 0);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.SRCADDR = (U32)pSource;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.DSTADDR = (U32)pDestination;

	regvalue = (pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & 0x80000000;

	INTENB = (regvalue>>31) & 1;

	regvalue |= (U32)( (CurTransferSize&0xFFF)		// Transfer size
					| (0<<12) // SBSize (source burst size)
					| (0<<15) // DBSize (destination burst size)
					| ((SourceBitWidth>>4)<<18) // SWidth (source transfer width)
					| (2<<21) // DWidth (destination transfer width)
					| (1UL<<24) // source master bus 0: AHB1, 1; AHB:2
					| (0UL<<25) // destination master bus
					| (0UL<<26) // SI (source increment)
					| (1UL<<27)	// DI (destination increment)
					);
	if (0 != Number_of_LLI)	regvalue = regvalue & ~(1UL<<31);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.Control = regvalue;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = 0x0;
	if (0 != Number_of_LLI)
	{
		LLI_Count = 1;
		while(1)
		{
			Number_of_LLI = TransferSize/(MAXTransferSize+Bytes);
			if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/Bytes;
			else					CurTransferSize = TransferSize/Bytes;

			TransferSize = TransferSize-MAXTransferSize;
			// last link list 일때 interrupt enable 한다.
			if(0 == Number_of_LLI) regvalue |= (INTENB<<31);

			regvalue = (regvalue&~0xfff) | CurTransferSize;

			NX_DMA_Build_LLI( (U32)pSource,
								(U32)pDestination+(LLI_Count*MAXTransferSize),
								(U32)regvalue,
								(U32)CmdBufferAddr+((LLI_Count-1)*Bytes),//(U32)CmdBufferAddr+((LLI_Count-1)*4),
								(U32)Number_of_LLI);
			if(0 == Number_of_LLI)
			{
				break;
			}
			LLI_Count++;
		}
		pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = CmdBufferAddr&~0x3;
	}
	//m_pRegister->Channel[nChannel].SGLLI.LLI = CmdBufferAddr&~0x3;
	regvalue = pRegister->Channel[DMA_ChannelIndex].Configuration & (0x3<<14);
	regvalue |= (U32)(1
					| (SourcePeriID<<1) // source id
					| (2<<11) // mode
					);
	pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;

}

void	NX_DMA_TransferMemToIO_Burst( U32 nChannel, const void* pSource, void* pDestination, U32 DestinationPeriID, U32 DestinationBitWidth, NX_DMA_BURST_SIZE DestinationBurstSize, U32 TransferSize )
{
	//register	struct tag_ModuleVariables*		pVariables;
	U32			CmdBufferAddr;
	U32 		regvalue;
	U32 		Number_of_LLI;
	U32 		LLI_Count;
	U32 		CurTransferSize;
	U32 		INTENB;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;
	register struct NX_DMA_RegisterSet *pRegister;
	//register	U32	regvalue;

	NX_ASSERT( 0 == (((U32)pSource) % 8) );
	NX_ASSERT( 0 == (((U32)pDestination) % 2) );

	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	NX_ASSERT( CNULL != __g_ModuleVariables);
	NX_ASSERT( CNULL != g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	CmdBufferAddr = g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address + (CHANNELBUFFERSIZE*DMA_ChannelIndex);

	//MAXTransferSize = (16*1024)-4;
	U32 MAXTransferSize;
	MAXTransferSize = (16*1024)-4;
	Number_of_LLI = TransferSize/(MAXTransferSize+4);
	if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/4;
	else					CurTransferSize = TransferSize/4;

	TransferSize = TransferSize-MAXTransferSize;

	INTENB = NX_DMA_GetInterruptEnable(nChannel, 0);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.SRCADDR = (U32)pSource;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.DSTADDR = (U32)pDestination;

	regvalue = (pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & 0x80000000;

	INTENB = (regvalue>>31) & 1;

	regvalue |= (U32)( CurTransferSize		// Transfer size
					| (0<<12) // SBSize (source burst size)
					| (DestinationBurstSize<<15) // DBSize (destination burst size)
					| (2<<18) // SWidth (source transfer width)
					| ((DestinationBitWidth>>4)<<21) // DWidth (destination transfer width)
					| (0UL<<24) // source master bus 0: AHB1, 1; AHB:2
					| (1UL<<25) // destination master bus
					| (1UL<<26) // SI (source increment)
					| (0UL<<27)	// DI (destination increment)
					);
	// tranfer size 가 16Kbyte-4byte 보다 클경우 LLI 의 마지막 에서 interrupt enable 한다.
	if (0 != Number_of_LLI)	regvalue = regvalue & ~(1UL<<31);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.Control = regvalue;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = 0x0;
	if (0 != Number_of_LLI)
	{
		LLI_Count = 1;
		while(1)
		{
			Number_of_LLI = TransferSize/(MAXTransferSize+4);
			if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/4;
			else					CurTransferSize = TransferSize/4;

			TransferSize = TransferSize-MAXTransferSize;
			// last link list 일때 interrupt enable 한다.
			if(0 == Number_of_LLI) regvalue |= (INTENB<<31);

			regvalue = (regvalue&~0xfff) | CurTransferSize;

			NX_DMA_Build_LLI( (U32)pSource+(LLI_Count*MAXTransferSize),
								(U32)pDestination,
								(U32)regvalue,
								(U32)CmdBufferAddr+((LLI_Count-1)*4),
								(U32)Number_of_LLI);
			if(0 == Number_of_LLI)
			{
				break;
			}
			LLI_Count++;
		}
		pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = CmdBufferAddr&~0x3;
	}
	regvalue = pRegister->Channel[DMA_ChannelIndex].Configuration & (0x3<<14);
	regvalue |= (U32)(1
					| (DestinationPeriID<<6) // destination id
					| (1<<11) // mode
					);
	pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;

}
void	NX_DMA_TransferIOToMem_Burst( U32 nChannel, const void* pSource, U32 SourcePeriID, U32 SourceBitWidth, NX_DMA_BURST_SIZE SourceBurstSize, void* pDestination, U32 TransferSize )
{
	//register	struct tag_ModuleVariables*		pVariables;
	U32			CmdBufferAddr;
	U32 		regvalue;
	U32 		Number_of_LLI;
	U32 		LLI_Count;
	U32 		CurTransferSize;
	U32			INTENB;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;
	register struct NX_DMA_RegisterSet *pRegister;
	//register	U32	regvalue;

	NX_ASSERT( 0 == (((U32)pSource) % 2) );
	NX_ASSERT( 0 == (((U32)pDestination) % 8) );
    //MES_REQUIRE ( 64 > SourcePeriID );
    //MES_REQUIRE ( 8 == SourceBitWidth || 16 == SourceBitWidth || 32 == SourceBitWidth );

	NX_ASSERT( CNULL != __g_ModuleVariables);
	NX_ASSERT( CNULL != g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	CmdBufferAddr = g_DMA_COMMANDBuffer[DMA_ModuleIndex].Address + (CHANNELBUFFERSIZE*DMA_ChannelIndex);

	//MAXTransferSize = (16*1024)-4;
	U32 Bytes = SourceBitWidth/8;
	U32 MAXTransferSize;
    if (8 == SourceBitWidth)		MAXTransferSize = (4*1024)-Bytes;
    else if (16 == SourceBitWidth)	MAXTransferSize = (8*1024)-Bytes;
    else							MAXTransferSize = (16*1024)-Bytes;
	//MAXTransferSize = (16*1024)-4;
	Number_of_LLI = TransferSize/(MAXTransferSize+Bytes);
	if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/Bytes;
	else					CurTransferSize = TransferSize/Bytes;

	TransferSize = TransferSize-MAXTransferSize;

	INTENB = NX_DMA_GetInterruptEnable(nChannel, 0);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.SRCADDR = (U32)pSource;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.DSTADDR = (U32)pDestination;

	regvalue = (pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & 0x80000000;

	INTENB = (regvalue>>31) & 1;

	regvalue |= (U32)( (CurTransferSize&0xFFF)		// Transfer size
					| (SourceBurstSize<<12) // SBSize (source burst size)
					| (0<<15) // DBSize (destination burst size)
					| ((SourceBitWidth>>4)<<18) // SWidth (source transfer width)
					| (2<<21) // DWidth (destination transfer width)
					| (1UL<<24) // source master bus 0: AHB1, 1; AHB:2
					| (0UL<<25) // destination master bus
					| (0UL<<26) // SI (source increment)
					| (1UL<<27)	// DI (destination increment)
					);
	if (0 != Number_of_LLI)	regvalue = regvalue & ~(1UL<<31);

	pRegister->Channel[DMA_ChannelIndex].SGLLI.Control = regvalue;
	pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = 0x0;
	if (0 != Number_of_LLI)
	{
		LLI_Count = 1;
		while(1)
		{
			Number_of_LLI = TransferSize/(MAXTransferSize+Bytes);
			if (0 != Number_of_LLI)	CurTransferSize = MAXTransferSize/Bytes;
			else					CurTransferSize = TransferSize/Bytes;

			TransferSize = TransferSize-MAXTransferSize;
			// last link list 일때 interrupt enable 한다.
			if(0 == Number_of_LLI) regvalue |= (INTENB<<31);

			regvalue = (regvalue&~0xfff) | CurTransferSize;

			NX_DMA_Build_LLI( (U32)pSource,
								(U32)pDestination+(LLI_Count*MAXTransferSize),
								(U32)regvalue,
								(U32)CmdBufferAddr+((LLI_Count-1)*Bytes),//(U32)CmdBufferAddr+((LLI_Count-1)*4),
								(U32)Number_of_LLI);
			if(0 == Number_of_LLI)
			{
				break;
			}
			LLI_Count++;
		}
		pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = CmdBufferAddr&~0x3;
	}
	//m_pRegister->Channel[nChannel].SGLLI.LLI = CmdBufferAddr&~0x3;
	regvalue = pRegister->Channel[DMA_ChannelIndex].Configuration & (0x3<<14);
	regvalue |= (U32)(1
					| (SourcePeriID<<1) // source id
					| (2<<11) // mode
					);
	pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;

}

CBOOL	NX_DMA_Build_LLI( U32 pSource, U32 pDestination, U32 ControlReg, U32 LLI_ADDR, U32 NextLLI)
{
	if(0 != NextLLI)
	{
		*(volatile U32*)(LLI_ADDR+0x0) = (U32)(pSource );
		*(volatile U32*)(LLI_ADDR+0x4) = (U32)(pDestination );
		*(volatile U32*)(LLI_ADDR+0x8) = (U32)(LLI_ADDR+4 );
		*(volatile U32*)(LLI_ADDR+0xC) = (U32)(ControlReg );
	}
	else
	{
		*(volatile U32*)(LLI_ADDR+0x0) =(U32)(pSource );
		*(volatile U32*)(LLI_ADDR+0x4) =(U32)(pDestination );
		*(volatile U32*)(LLI_ADDR+0x8) =(U32)(0x0 );
		*(volatile U32*)(LLI_ADDR+0xC) =(U32)(ControlReg );
	}
}

//------------------------------------------------------------------------------
/**
 *	@brief		Run DMA transfer.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.
 *	@remarks	DMA have different sequence by DMA operation mode ( Wirte Back or Write Through ). \n
 *	@code
 *				//--------------------------------------------------------------
 *				// Write Back Mode Operation
 *				//--------------------------------------------------------------
 *
 *				NX_DMA_SetCommandBufferMode( CTRUE );				// DMA set Write Back Mode
 *				...													// DMA configuration
 *
 *				while( CFALSE == NX_DMA_IsCommandBufferReady() )	// Check command buffer
 *				{
 *					CNULL;
 *				}
 *				NX_DMA_Run();										// DMA run
 *
 *				//--------------------------------------------------------------
 *				// Write Through Mode Operation
 *				//--------------------------------------------------------------
 *
 *				NX_DMA_SetCommandBufferMode( CFALSE );				// DMA set Write Through Mode
 *				...
 *
 *				NX_DMA_Run();										// DMA run

 *	@endcode
 *	@see		NX_DMA_SetCommandBufferMode,	NX_DMA_GetCommandBufferMode,
 *												NX_DMA_CheckRunning,
 *				NX_DMA_Stop,					NX_DMA_CommandBufferFlush,
 *				NX_DMA_IsCommandBufferFlush,	NX_DMA_IsCommandBufferReady,
 *				NX_DMA_IsCommandBufferEmpty,	NX_DMA_GetCommandBufferFreeCount
 */
void	NX_DMA_Run( U32 nChannel )
{
	register	struct	NX_DMA_RegisterSet		*pRegister;
	U32	regvalue;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	regvalue = pRegister->Channel[DMA_ChannelIndex].Configuration;
	regvalue |= 1;
	pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Check whether DMA transfer is running or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		\b CTURE	indicates DMA transfer is running.\n
 *				\b CFALSE	indicates DMA transfer is idle.
 *	@see	NX_DMA_SetCommandBufferMode,	NX_DMA_GetCommandBufferMode,
 *			NX_DMA_Run,
 *			NX_DMA_Stop,					NX_DMA_CommandBufferFlush,
 *			NX_DMA_IsCommandBufferFlush,	NX_DMA_IsCommandBufferReady,
 *			NX_DMA_IsCommandBufferEmpty,	NX_DMA_GetCommandBufferFreeCount
 */
U32	NX_DMA_CheckRunning ( U32 nChannel )
{
	register	struct	NX_DMA_RegisterSet		*pRegister;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	return (U32)((ReadIODW(&pRegister->EnbldChns) & 0xFF) & 1UL<<DMA_ChannelIndex);
	//return (CBOOL)((pRegister->CSR_CPC[ModuleIndex*2] & 0xF) != 0x0);
}


//------------------------------------------------------------------------------
/**
 *	@brief		Stop/Cancel DMA Transfer.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param		Enable		\b CTRUE	indicate that DMA Stop is Enable. \n
 *							\b CFALSE	indicate that DMA Stop is Disable.
 *	@return		None.
 *	@remark		If DMA running write back mode, then user have to clear command buffer.\n
 *				Also, user should check command buffer flush is completed or not.\n \n
 *				After NX_DMA_Stop( CTRUE ), user should check DMA running status. \n
 *				If DMA is really stopped, user should setting NX_DMA_Stop( CFALSE ).
 *	@code
 *				if( NX_DMA_GetCommandBufferMode() )
 *				{
 *					NX_DMA_CommandBufferFlush();						// Command Buffer Flush
 *					while(	CTRUE != NX_DMA_IsCommandBufferFlush() )	// wait during command buffer flushing
 *					{ CNULL; }
 *				}
 *
 *				NX_DMA_Stop(CTRUE);
 *				while( NX_DMA_CheckRunning() )
 *				{
 *					CNULL;												// wait during DMA stop
 *				}
 *				NX_DMA_Stop(CFALSE);
 *	@endcode
 *	@see		NX_DMA_SetCommandBufferMode,	NX_DMA_GetCommandBufferMode,
 *				NX_DMA_Run,						NX_DMA_CheckRunning,
 *												NX_DMA_CommandBufferFlush,
 *				NX_DMA_IsCommandBufferFlush,	NX_DMA_IsCommandBufferReady,
 *				NX_DMA_IsCommandBufferEmpty,	NX_DMA_GetCommandBufferFreeCount
 */
void	NX_DMA_Stop ( U32 nChannel, CBOOL Enable )
{
	const U32 Enb	= (1UL<<0);
	const U32 Holt	= (1UL<<18);
	register	struct	NX_DMA_RegisterSet		*pRegister;
	U32	regvalue;
	U32 		DMA_ModuleIndex = nChannel/8;
	U32 		DMA_ChannelIndex = nChannel%8;

	//NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
	NX_ASSERT( CNULL != __g_ModuleVariables );
	NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
	NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

	pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
	regvalue = ReadIODW(&pRegister->Channel[DMA_ChannelIndex].Configuration);

	if( Enable )
	{
		regvalue  &= ~Enb;
	}
	else
	{
		regvalue  |= Holt;
	}

	WriteIODW(&pRegister->Channel[DMA_ChannelIndex].Configuration, regvalue);
}

