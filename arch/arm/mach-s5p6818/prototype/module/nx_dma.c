//------------------------------------------------------------------------------
//
//  Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//  Module      : Dma Controller
//  File        : nx_dma.c
//  Description :
//  Author      :
//  History     :
//                  2007/04/03  first
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_dma.h"

//-----------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//  Module Interface
//------------------------------------------------------------------------------

static	struct 
{
	struct NX_DMA_RegisterSet *pRegister;
} __g_ModuleVariables[NUMBER_OF_DMA_MODULE] = { {CNULL,}, };

/**
 *  @brief  Initialize of prototype enviroment & local variables.
 *  @return CTRUE    indicate that Initialize is successed.
 *          CFALSE   indicate that Initialize is failed.
 *  @see    NX_DMA_GetNumberOfModule
 */
CBOOL   NX_DMA_Initialize( void )
{
    //  @modified Gamza static variable(__g_ModuleVariables) is automatically filled by '0'
    //                  ë§Œì•½ ì´ˆê¸°??ê³¼ì •???„ì—­ë³€?˜ë? 0?¼ë¡œ ì´ˆê¸°???˜ëŠ” ?‘ì—… ?´ì™¸???¼ì„
    //                  ?´ì•¼?œë‹¤ë©?bInit ê°’ì„ CFALSEë¡??˜ì •?´ì•¼?œë‹¤.
    static CBOOL bInit = CTRUE;
    //register struct NX_DMA_RegisterSet *pRegister;
    U32 i;

    if( CFALSE == bInit )
    {
        for(i=0; i<NUMBER_OF_DMA_MODULE; i++)
        {
            __g_ModuleVariables[i].pRegister = CNULL;
        }
        bInit = CTRUE;
    }

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get number of modules in the chip.
 *  @return     Module's number.
 *  @see        NX_DMA_Initialize
 */
U32     NX_DMA_GetNumberOfModule( void )
{
    return NUMBER_OF_DMA_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *  @brief      Get module's physical address.
 *  @return     Module's physical address
 *  @see                                        NX_DMA_GetSizeOfRegisterSet,
 *              NX_DMA_SetBaseAddress,          NX_DMA_GetBaseAddress,
 *              NX_DMA_OpenModule,              NX_DMA_CloseModule,
 *              NX_DMA_CheckBusy,               NX_DMA_CanPowerDown
 */
U32     NX_DMA_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[] =
    {
        PHY_BASEADDR_LIST( DMA )
    };
    NX_CASSERT( NUMBER_OF_DMA_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );

    return  PhysicalAddr[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a size, in byte, of register set.
 *  @return     Size of module's register set.
 *  @see        NX_DMA_GetPhysicalAddress,
 *              NX_DMA_SetBaseAddress,          NX_DMA_GetBaseAddress,
 *              NX_DMA_OpenModule,              NX_DMA_CloseModule,
 *              NX_DMA_CheckBusy,               NX_DMA_CanPowerDown
 */
U32     NX_DMA_GetSizeOfRegisterSet( void )
{
    return sizeof( struct NX_DMA_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a base address of register set.
 *  @param[in]  BaseAddress Module's base address
 *  @return     None.
 *  @see        NX_DMA_GetPhysicalAddress,      NX_DMA_GetSizeOfRegisterSet,
 *                                              NX_DMA_GetBaseAddress,
 *              NX_DMA_OpenModule,              NX_DMA_CloseModule,
 *              NX_DMA_CheckBusy,               NX_DMA_CanPowerDown
 */
void    NX_DMA_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
    NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
    __g_ModuleVariables[ModuleIndex].pRegister = (struct NX_DMA_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a base address of register set
 *  @return     Module's base address.
 *  @see        NX_DMA_GetPhysicalAddress,      NX_DMA_GetSizeOfRegisterSet,
 *              NX_DMA_SetBaseAddress,
 *              NX_DMA_OpenModule,              NX_DMA_CloseModule,
 *              NX_DMA_CheckBusy,               NX_DMA_CanPowerDown
 */
void*    NX_DMA_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );

    return (void*)__g_ModuleVariables[ModuleIndex].pRegister;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Initialize selected modules with default value.
 *  @return     CTRUE    indicate that Initialize is successed. \n
 *              CFALSE   indicate that Initialize is failed.
 *  @see        NX_DMA_GetPhysicalAddress,      NX_DMA_GetSizeOfRegisterSet,
 *              NX_DMA_SetBaseAddress,          NX_DMA_GetBaseAddress,
 *                                              NX_DMA_CloseModule,
 *              NX_DMA_CheckBusy,               NX_DMA_CanPowerDown
 */
CBOOL   NX_DMA_OpenModule(U32 ModuleIndex)
{
    register struct NX_DMA_RegisterSet *pRegister;
    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    WriteIO32(&pRegister->Configuration, 0x1);
	
    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinitialize selected module to the proper stage.
 *  @return     CTRUE    indicate that Deinitialize is successed. \n
 *              CFALSE   indicate that Deinitialize is failed.
 *  @see        NX_DMA_GetPhysicalAddress,      NX_DMA_GetSizeOfRegisterSet,
 *              NX_DMA_SetBaseAddress,          NX_DMA_GetBaseAddress,
 *              NX_DMA_OpenModule,
 *              NX_DMA_CheckBusy,               NX_DMA_CanPowerDown
 */
CBOOL   NX_DMA_CloseModule( U32 ModuleIndex )
{
    U32 i;
    register struct NX_DMA_RegisterSet *pRegister;
    pRegister = __g_ModuleVariables[ModuleIndex].pRegister;
    WriteIO32(&pRegister->Configuration, 0x0);
    NX_DMA_ClearInterruptPendingAll();

    for (i=0; i<NUMBER_OF_DMA_CHANNEL ; i++)    
		NX_DMA_Stop(i+(ModuleIndex*8), CTRUE);

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the selected modules is busy or not.
 *  @return     CTRUE    indicate that Module is Busy. \n
 *              CFALSE   indicate that Module is NOT Busy.
 *  @see        NX_DMA_GetPhysicalAddress,      NX_DMA_GetSizeOfRegisterSet,
 *              NX_DMA_SetBaseAddress,          NX_DMA_GetBaseAddress,
 *              NX_DMA_OpenModule,              NX_DMA_CloseModule,
 *                                              NX_DMA_CanPowerDown
 */
CBOOL   NX_DMA_CheckBusy( void )
{
    U32 i;
    U32 CheckValue;

    CheckValue = 0;
    for (i=0; i<NUMBER_OF_DMA_CHANNEL*NUMBER_OF_DMA_MODULE ; i++)   
		CheckValue |= NX_DMA_CheckRunning(i);

    if (0 != CheckValue)    return CTRUE;
    	return CFALSE;
}

U32     NX_DMA_GetInterruptNumber( U32 nChannel )
{
    const U32 INTNumber[NUMBER_OF_DMA_MODULE] =
    {
        INTNUM_LIST(DMA)
    };
    U32         DMA_ModuleIndex = nChannel/8;

    NX_CASSERT( NUMBER_OF_DMA_MODULE == (sizeof(INTNumber)/sizeof(INTNumber[0])) );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );

    return INTNumber[DMA_ModuleIndex];
}
//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enable or disable.
 *  @param[in]  IntNum  Interrupt Number .
 *  @param[in]  Enable  CTRUE    indicate that Interrupt Enable. \n
 *                      CFALSE   indicate that Interrupt Disable.
 *  @return     None.
 *  @see                                            NX_DMA_GetInterruptEnable,
 *              NX_DMA_GetInterruptPending,
 *              NX_DMA_ClearInterruptPending,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
void    NX_DMA_SetInterruptEnable( U32 nChannel, U32 IntNum, CBOOL Enable )
{
    const U32 INTENB_M      = 31; // module
    const U32 INTENB_C      = 14; // channel
    const U32 INTENB_C_MASK = (3UL<<INTENB_C);
    const U32 INTENB_M_MASK = (1UL<<INTENB_M);

    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    register struct NX_DMA_RegisterSet *pRegister;
    register    U32 regval;

    NX_ASSERT( NX_DMA_NUM_OF_INT > IntNum );
    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );
    NX_ASSERT( CNULL != __g_ModuleVariables );

    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

    regval = ReadIO32(&pRegister->Channel[DMA_ChannelIndex].SGLLI.Control) & ~INTENB_M_MASK;
    regval |= (Enable << INTENB_M);
    WriteIO32(&pRegister->Channel[DMA_ChannelIndex].SGLLI.Control, regval);
    regval = ReadIO32(&pRegister->Channel[DMA_ChannelIndex].Configuration) & ~INTENB_C_MASK;
    regval |= (Enable << (INTENB_C+IntNum));
    WriteIO32(&pRegister->Channel[DMA_ChannelIndex].Configuration, regval);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]  IntNum  Interrupt Number.
 *  @return     CTRUE    indicate that Interrupt is enabled. 
 *              CFALSE   indicate that Interrupt is disabled.
 *  @see        NX_DMA_SetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
CBOOL   NX_DMA_GetInterruptEnable( U32 nChannel, U32 IntNum )
{
    register struct NX_DMA_RegisterSet *pRegister;
    register U32 regvalue;
    const U32 INTENB_POS    = 14;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NX_DMA_NUM_OF_INT > IntNum );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    //pRegister = __g_pRegister;
    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

    regvalue = ReadIO32(&pRegister->Channel[DMA_ChannelIndex].Configuration);

    if( regvalue & ( 1UL << (IntNum+INTENB_POS)) )
    {
        return CFALSE;
    }

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]  IntNum  Interrupt Number.
 *  @return     CTRUE    indicate that Pending is seted. \n
 *              CFALSE   indicate that Pending is Not Seted.
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *                                                  NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
CBOOL   NX_DMA_GetInterruptPending( U32 nChannel , U32 IntNum )
{
    register struct NX_DMA_RegisterSet *pRegister;
    //register U32 regvalue;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    IntNum = IntNum;
    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

    if( ReadIO32(&pRegister->IntStatus) & 1UL<<DMA_ChannelIndex )
    {
        return CTRUE;
    }

    return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  IntNum  Interrupt number.
 *  @return     None.
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     NX_DMA_GetInterruptPending64,
 *                                                  NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
void    NX_DMA_ClearInterruptPending( U32 nChannel, U32 IntNum )
{
    register struct NX_DMA_RegisterSet *pRegister;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    IntNum = IntNum;
    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

    if (0 != IntNum)    WriteIO32(&pRegister->IntTCClear, 1UL<<DMA_ChannelIndex);
    else                WriteIO32(&pRegister->IntErrClr, 1UL<<DMA_ChannelIndex);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set all interrupts to be enables or disables.
 *  @param[in]  Enable  CTRUE    indicate that Set to all interrupt enable. \n
 *                      CFALSE   indicate that Set to all interrupt disable.
 *  @return     None.
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *                                                  NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
void    NX_DMA_SetInterruptEnableAll( CBOOL Enable )
{
    register struct NX_DMA_RegisterSet *pRegister;
    register U32 regvalue;
    const U32 INTENB_M  = 31;
    const U32 INTENB_C  = 14;
    const U32 INTENB_M_MASK = (1UL<<INTENB_M);
    const U32 INTENB_C_MASK = (3UL<<INTENB_C);
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
            regvalue = ReadIO32(&pRegister->Channel[i].SGLLI.Control) & ~INTENB_M_MASK;
            regvalue |= (Enable << INTENB_M);
            WriteIO32(&pRegister->Channel[i].SGLLI.Control, regvalue);
            regvalue = ReadIO32(&pRegister->Channel[i].Configuration) & ~INTENB_C_MASK;
            regvalue |= (Enable << INTENB_C) | (Enable << (INTENB_C+1));
            WriteIO32(&pRegister->Channel[i].Configuration, regvalue);
        }
    }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enable or not.
 *  @return     CTRUE    indicate that At least one( or more ) interrupt is enabled. \n
 *              CFALSE   indicate that All interrupt is disabled.
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
CBOOL   NX_DMA_GetInterruptEnableAll( void )
{
    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are pended or not.
 *  @return     CTRUE    indicate that At least one( or more ) pending is seted. \n
 *              CFALSE   indicate that All pending is NOT seted.
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *                                                  NX_DMA_ClearInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
CBOOL   NX_DMA_GetInterruptPendingAll( void )
{
    register struct NX_DMA_RegisterSet *pRegister;
    NX_ASSERT( CNULL != __g_ModuleVariables );
    //NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
    //NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
    int j;
    for (j=0; j<NUMBER_OF_DMA_MODULE; j++)
    {
        pRegister = __g_ModuleVariables[j].pRegister;

        if(ReadIO32(&pRegister->IntStatus))
        {
            return CTRUE;
        }
    }

    return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear pending state of all interrupts.
 *  @return     None.
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,
 *              NX_DMA_GetInterruptPendingNumber
 */
void    NX_DMA_ClearInterruptPendingAll( void )
{
    register struct NX_DMA_RegisterSet *pRegister;
    int j;
    NX_ASSERT( CNULL != __g_ModuleVariables );
    //NX_ASSERT( NUMBER_OF_DMA_MODULE > ModuleIndex );
    //NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
    for (j=0; j<NUMBER_OF_DMA_MODULE; j++)
    {
        pRegister = __g_ModuleVariables[j].pRegister;
        WriteIO32(&pRegister->IntTCClear, 0xFF );
        WriteIO32(&pRegister->IntErrClr, 0xFF );
    }
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number which has the most prority of pended interrupts
 *  @return     Pending Number( If all pending is not set then return -1 ).
 *  @see        NX_DMA_SetInterruptEnable,          NX_DMA_GetInterruptEnable,
 *              NX_DMA_SetInterruptEnable64,        NX_DMA_GetInterruptEnable64,
 *              NX_DMA_GetInterruptPending,     	NX_DMA_GetInterruptPending64,
 *              NX_DMA_ClearInterruptPending,       NX_DMA_ClearInterruptPending64,
 *              NX_DMA_SetInterruptEnableAll,       NX_DMA_GetInterruptEnableAll,
 *              NX_DMA_GetInterruptPendingAll,      NX_DMA_ClearInterruptPendingAll
 */
U32     NX_DMA_GetInterruptPendingNumber( U32 nChannel )    // -1 if None
{
    register struct NX_DMA_RegisterSet *pRegister;
    register U32 regvalue;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    NX_ASSERT( CNULL != __g_ModuleVariables );

    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

    regvalue = ReadIO32(&pRegister->IntTCStatus);
    regvalue &= (1UL << DMA_ChannelIndex);
    if (0 != regvalue)  return 1;
    regvalue = ReadIO32(&pRegister->IntErrorStatus);
    regvalue &= (1UL << DMA_ChannelIndex);
    if (0 != regvalue)  return 0;
    else                return -1;
}

#define NX_DMA_DEBUG	0


void    NX_DMA_SetControl( U32 nChannel, NX_DMA_INFO* pInfoSet )
{
    register    struct NX_DMA_RegisterSet *pRegister;
    register    U32 regvalue     	= 0;
	U32 		Number_of_LLI;

    U32         CurTransferCount  	= 0;
    U32         TransferSize        = pInfoSet->TxSize;
    U32			MaxTransferSize  	= 0;

    U32 		Byte 			 	= ( (pInfoSet->SrcWidth << 4)/8);

    U32         DMA_ModuleIndex  	= nChannel / 8;
    U32         DMA_ChannelIndex 	= nChannel % 8;

    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE  > DMA_ModuleIndex  );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister  = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

	MaxTransferSize = ( Byte* 1*1024)-Byte;    

	Number_of_LLI = TransferSize/(MaxTransferSize+Byte);    
	if ( Number_of_LLI > 0 )	
    { 
        CurTransferCount = MaxTransferSize/Byte;
        pInfoSet->TxSize -= MaxTransferSize;
	}
	else
	{
        CurTransferCount = TransferSize/Byte;
        pInfoSet->TxSize -= TransferSize;
	}

	pRegister->Channel[DMA_ChannelIndex].SGLLI.SRCADDR = (U32)(pInfoSet->SrcAddr);
	pRegister->Channel[DMA_ChannelIndex].SGLLI.DSTADDR = (U32)(pInfoSet->DstAddr);

    regvalue  = pRegister->Channel[DMA_ChannelIndex].SGLLI.Control;

    regvalue |= (U32)( ( CurTransferCount       << 0  ) // Transfer size
                     | ( pInfoSet->SrcBurstSize << 12 ) // SBSize (source burst size)
                     | ( pInfoSet->DstBurstSize << 15 ) // DBSize (destination burst size)
                     | ( pInfoSet->SrcWidth     << 18 ) // SWidth (source transfer width)
                     | ( pInfoSet->DstWidth     << 21 ) // DWidth (destination transffer width)
                     | ( pInfoSet->SrcAHBSel    << 24 ) // Source AHB Master x Select
                     | ( pInfoSet->DstAHBSel    << 25 ) // Destination AHB Master x Select
                     | ( pInfoSet->SrcAddrInc   << 26 ) // SI (source increment)
                     | ( pInfoSet->DstAddrInc   << 27 ) // DI (destination increment) 
                     | ( 1UL   << 31 )
                     );

    pRegister->Channel[DMA_ChannelIndex].SGLLI.Control 	= regvalue; 	// Control Register Set
    pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI 		= 0x0;
    pInfoSet->Control = regvalue;

#if NX_DMA_DEBUG
    NX_MSG("TxSize:       %8d \r\n", pInfoSet->TxSize );
    NX_MSG("SrcBurstSize: %8d \r\n", pInfoSet->SrcBurstSize );
    NX_MSG("DstBurstSize: %8d \r\n", pInfoSet->DstBurstSize );
    NX_MSG("SrcWidth:     %8d \r\n", pInfoSet->SrcWidth );
    NX_MSG("DstWidth:     %8d \r\n", pInfoSet->DstWidth );
    NX_MSG("SrcAHBSel:    %8d \r\n", pInfoSet->SrcAHBSel );
    NX_MSG("DstAHBSel:    %8d \r\n", pInfoSet->DstAHBSel );
    NX_MSG("SrcAddrInc:   %8d \r\n", pInfoSet->SrcAddrInc );
    NX_MSG("DstAddrInc:   %8d \r\n", pInfoSet->DstAddrInc );
    NX_MSG("Control Register : %8x \r\n", regvalue );
#endif
    
}

void    NX_DMA_Configuration( U32 nChannel, NX_DMA_INFO* pInfoSet )
{
    register    struct NX_DMA_RegisterSet *pRegister;
    register    U32 regvalue     = 0;

    U32         DMA_ModuleIndex  = nChannel / 8;
    U32         DMA_ChannelIndex = nChannel % 8;

    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE  > DMA_ModuleIndex  );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister  = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
    //---------------------------------------------------------------------------------------------
	if( pInfoSet->FlowCtrl != 0 ) {
        if( (pInfoSet->FlowCtrl == 6) || (pInfoSet->FlowCtrl == 2) )
        	regvalue |= (U32)( pInfoSet->SrcPeriID << 1  );    // Source Peripheral.
        else if( (pInfoSet->FlowCtrl == 5) || (pInfoSet->FlowCtrl == 1) )
            regvalue |= (U32)( pInfoSet->DstPeriID << 6  );    // Destination Peripheral.
        else if( (pInfoSet->FlowCtrl == 7) || (pInfoSet->FlowCtrl == 3) || (pInfoSet->FlowCtrl == 4) )
        {
            regvalue |= (U32)( ( pInfoSet->SrcPeriID << 1  )    // Source Peripheral.
                     |       ( pInfoSet->DstPeriID << 6  )    // Destination Peripheral.
                            );
        }           
    }

    regvalue |= (U32)( ( pInfoSet->FlowCtrl     << 11 ) // Flow Control and Transfer type.
                     | ( pInfoSet->IntIE        << 14 ) // Interrupt Error Mask.
                     | ( pInfoSet->IntITC       << 15 ) // Terminal Count interrupt Mask.
                    );
    //---------------------------------------------------------------------------------------------
    pRegister->Channel[DMA_ChannelIndex].Configuration = regvalue;
    pInfoSet->Configuration= regvalue;

#if NX_DMA_DEBUG
    NX_MSG("FlowCtrl:  %8d \r\n", pInfoSet->FlowCtrl );
    NX_MSG("SrcPeriID: %8d \r\n", pInfoSet->SrcPeriID );
    NX_MSG("DstPeriID: %8d \r\n", pInfoSet->DstPeriID );
    NX_MSG("IntIE:     %8d \r\n", pInfoSet->IntIE );
    NX_MSG("IntITC:    %8d \r\n", pInfoSet->IntITC );
    NX_MSG("Configuration Register : %8x \r\n", regvalue );
#endif   

}

void    NX_DMA_SetAttribute( U32 nChannel, NX_DMA_INFO* pInfoSet )
{
    register    struct NX_DMA_RegisterSet *pRegister;

    U32         DMA_ModuleIndex  = nChannel / 8;
    U32         DMA_ChannelIndex = nChannel % 8;

    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE  > DMA_ModuleIndex  );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    // Channel Control
    NX_DMA_SetControl( nChannel, pInfoSet );
    // Channel Configuration.
    NX_DMA_Configuration( nChannel, pInfoSet );  

}

void*     NX_DMA_Build_LLI( U32 nChannel, NX_DMA_INFO* pInfoSet )
{
    register    struct NX_DMA_RegisterSet *pRegister;
    register    U32 regvalue     = 0;

    U32			MaxTransferSize  				= 0;
    U32			TransferSize     				= pInfoSet->TxSize;
    U32			CurTransferCount  				= 0;

    U32			Byte 			 				= ((pInfoSet->SrcWidth << 4)/8);

    void*       SrcAddr          				= (void*)(pInfoSet->SrcAddr);
    void*       DstAddr          				= (void*)(pInfoSet->DstAddr);

    U32			Number_of_LLI    				= 0;

    struct NX_DMALLI_RegisterSet*	pLLI_Addr 	= 0;
    U32			LLI_BUFF 	     				= 0;    

    U32         DMA_ModuleIndex  = nChannel / 8;
    U32         DMA_ChannelIndex = nChannel % 8;

//    NX_ASSERT( 0 == (((U32)pInfoSet->SrcAddr) % 8) );
//    NX_ASSERT( 0 == (((U32)pInfoSet->DstAddr) % 8) );
    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE  > DMA_ModuleIndex  );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister  = __g_ModuleVariables[DMA_ModuleIndex].pRegister;

    pLLI_Addr = (struct NX_DMALLI_RegisterSet*)(pInfoSet->HwLLI);  
    LLI_BUFF  = (U32)(pInfoSet->HwLLI);

	MaxTransferSize = ( Byte* 1*1024)-Byte;

    if( TransferSize > MaxTransferSize )
    {   
        if( pInfoSet->SrcAddrInc == CTRUE )
            SrcAddr += MaxTransferSize;
        if( pInfoSet->DstAddrInc == CTRUE )
            DstAddr += MaxTransferSize;      	
		
        regvalue     = pRegister->Channel[DMA_ChannelIndex].SGLLI.Control;
	#if NX_DMA_DEBUG
		NX_MSG("----------------------------\r\n");
		NX_MSG("pInfoSet+SrcAddr: %8X(%8X) \r\n", (pInfoSet->SrcAddr), &(pInfoSet->SrcAddr) );
		NX_MSG("pInfoSet+DstAddr: %8X(%8X) \r\n", (pInfoSet->DstAddr), &(pInfoSet->DstAddr) );
		NX_MSG("pInfoSet+HwLLI	: %8X(%8X) \r\n", (pInfoSet->HwLLI	), &(pInfoSet->HwLLI  ) );
		NX_MSG("pInfoSet+Control: %8X(%8X) \r\n", (pInfoSet->Control), &(pInfoSet->Control) );
		NX_MSG("----------------------------\r\n");
	#endif
        while(1)
        {
            // LLI Count Calcurate.    
            Number_of_LLI    = (TransferSize)/((MaxTransferSize+Byte));
            // Current Transfer Size Calcurate.
            if( 0 != Number_of_LLI )    CurTransferCount = MaxTransferSize/Byte;
            else                        CurTransferCount = TransferSize/Byte;

            // Sub TransferSize            
            TransferSize =  TransferSize-MaxTransferSize;

            if(0 == Number_of_LLI) regvalue |= (1UL<<31);           
            regvalue = (regvalue & (~0xFFF)) | (CurTransferCount);
            
            if( 0 != Number_of_LLI )
            {
                pLLI_Addr->SRCADDR = ((U32)SrcAddr);
                pLLI_Addr->DSTADDR = ((U32)DstAddr);
                pLLI_Addr->LLI     = (pLLI_Addr + 0x1);
                pLLI_Addr->Control = (regvalue);
            }
            else
            {
                pLLI_Addr->SRCADDR = ((U32)SrcAddr);
                pLLI_Addr->DSTADDR = ((U32)DstAddr);
                pLLI_Addr->LLI     = 0;
                pLLI_Addr->Control = (regvalue);
                break;
            }
        #if NX_DMA_DEBUG
            NX_MSG("Number of LLI : %d    \r\n", Number_of_LLI );
            NX_MSG("----------------------------\r\n");
            NX_MSG("LLIADDR+0x0: %8X(%8X) \r\n", (pLLI_Addr->SRCADDR), &(pLLI_Addr->SRCADDR) );
            NX_MSG("LLIADDR+0x4: %8X(%8X) \r\n", (pLLI_Addr->DSTADDR), &(pLLI_Addr->DSTADDR) );
            NX_MSG("LLIADDR+0x8: %8X(%8X) \r\n", (pLLI_Addr->LLI    ), &(pLLI_Addr->LLI    ) );
            NX_MSG("LLIADDR+0xC: %8X(%8X) \r\n", (pLLI_Addr->Control), &(pLLI_Addr->Control) );
            NX_MSG("----------------------------\r\n");
            NX_MSG("Control Register: %8x \r\n", regvalue );
            NX_MSG("----------------------------\r\n");
        #endif        

            if( pInfoSet->SrcAddrInc == CTRUE )
                SrcAddr += MaxTransferSize;
            if( pInfoSet->DstAddrInc == CTRUE )
                DstAddr += MaxTransferSize;            
            pLLI_Addr += 1;
        }
        pRegister->Channel[DMA_ChannelIndex].SGLLI.LLI = (U32)(LLI_BUFF & (~0x3));
    } 
    
    regvalue = pRegister->Configuration;
    pRegister->Configuration = (regvalue & ~0x1) | 0x1;						// DMA Controller Enable

    return (void*)pLLI_Addr;
}

void    NX_DMA_Transfer( U32 nChannel, NX_DMA_INFO *pInfoSet )
{    
	NX_DMA_SetAttribute( nChannel, pInfoSet );
	NX_DMA_Build_LLI( nChannel, pInfoSet );
	NX_DMA_Run( nChannel );
}


//------------------------------------------------------------------------------
/**
 *  @brief      Run DMA transfer.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     None.
 *  @remarks    DMA have different sequence by DMA operation mode ( Wirte Back or Write Through ). \n
 *  @code
 *              //--------------------------------------------------------------
 *              // Write Back Mode Operation
 *              //--------------------------------------------------------------
 *
 *              NX_DMA_SetCommandBufferMode( CTRUE );               // DMA set Write Back Mode
 *              ...                                                 // DMA configuration
 *
 *              while( CFALSE == NX_DMA_IsCommandBufferReady() )    // Check command buffer
 *              {
 *                  CNULL;
 *              }
 *              NX_DMA_Run();                                       // DMA run
 *
 *              //--------------------------------------------------------------
 *              // Write Through Mode Operation
 *              //--------------------------------------------------------------
 *
 *              NX_DMA_SetCommandBufferMode( CFALSE );              // DMA set Write Through Mode
 *              ...
 *
 *              NX_DMA_Run();                                       // DMA run

 *  @endcode
 *  @see        NX_DMA_SetCommandBufferMode,    NX_DMA_GetCommandBufferMode,
 *                                              NX_DMA_CheckRunning,
 *              NX_DMA_Stop,                    NX_DMA_CommandBufferFlush,
 *              NX_DMA_IsCommandBufferFlush,    NX_DMA_IsCommandBufferReady,
 *              NX_DMA_IsCommandBufferEmpty,    NX_DMA_GetCommandBufferFreeCount
 */
void    NX_DMA_Run( U32 nChannel )
{
    register    struct  NX_DMA_RegisterSet      *pRegister;
    U32 regvalue;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

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
 *  @brief      Check whether DMA transfer is running or not.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     CTURE    indicates DMA transfer is running.\n
 *              CFALSE   indicates DMA transfer is idle.
 *  @see    NX_DMA_SetCommandBufferMode,    NX_DMA_GetCommandBufferMode,
 *          NX_DMA_Run,
 *          NX_DMA_Stop,                    NX_DMA_CommandBufferFlush,
 *          NX_DMA_IsCommandBufferFlush,    NX_DMA_IsCommandBufferReady,
 *          NX_DMA_IsCommandBufferEmpty,    NX_DMA_GetCommandBufferFreeCount
 */
U32 NX_DMA_CheckRunning ( U32 nChannel )
{
    register    struct  NX_DMA_RegisterSet      *pRegister;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    //NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
    return (U32)((ReadIO32(&pRegister->EnbldChns) & 0xFF) & 1UL<<DMA_ChannelIndex);
    //return (CBOOL)((pRegister->CSR_CPC[ModuleIndex*2] & 0xF) != 0x0);
}


//------------------------------------------------------------------------------
/**
 *  @brief      Stop/Cancel DMA Transfer.
 *  @param[in]  ModuleIndex     an index of module.
 *  @param      Enable      CTRUE    indicate that DMA Stop is Enable. \n
 *                          CFALSE   indicate that DMA Stop is Disable.
 *  @return     None.
 *  @remark     If DMA running write back mode, then user have to clear command buffer.\n
 *              Also, user should check command buffer flush is completed or not.\n \n
 *              After NX_DMA_Stop( CTRUE ), user should check DMA running status. \n
 *              If DMA is really stopped, user should setting NX_DMA_Stop( CFALSE ).
 *  @code
 *              if( NX_DMA_GetCommandBufferMode() )
 *              {
 *                  NX_DMA_CommandBufferFlush();                        // Command Buffer Flush
 *                  while(  CTRUE != NX_DMA_IsCommandBufferFlush() )    // wait during command buffer flushing
 *                  { CNULL; }
 *              }
 *
 *              NX_DMA_Stop(CTRUE);
 *              while( NX_DMA_CheckRunning() )
 *              {
 *                  CNULL;                                              // wait during DMA stop
 *              }
 *              NX_DMA_Stop(CFALSE);
 *  @endcode
 *  @see        NX_DMA_SetCommandBufferMode,    NX_DMA_GetCommandBufferMode,
 *              NX_DMA_Run,                     NX_DMA_CheckRunning,
 *                                              NX_DMA_CommandBufferFlush,
 *              NX_DMA_IsCommandBufferFlush,    NX_DMA_IsCommandBufferReady,
 *              NX_DMA_IsCommandBufferEmpty,    NX_DMA_GetCommandBufferFreeCount
 */
void    NX_DMA_Stop ( U32 nChannel, CBOOL Enable )
{
    const U32 Enb   = (1UL<<0);
    const U32 Holt  = (1UL<<18);
    register    struct  NX_DMA_RegisterSet      *pRegister;
    U32         regvalue;
    U32         DMA_ModuleIndex = nChannel/8;
    U32         DMA_ChannelIndex = nChannel%8;

    //NX_ASSERT( NUMBER_OF_DMA_CHANNEL > nChannel );
    NX_ASSERT( CNULL != __g_ModuleVariables );
    NX_ASSERT( NUMBER_OF_DMA_MODULE > DMA_ModuleIndex );
    NX_ASSERT( NUMBER_OF_DMA_CHANNEL > DMA_ChannelIndex );

    pRegister = __g_ModuleVariables[DMA_ModuleIndex].pRegister;
    regvalue = ReadIO32(&pRegister->Channel[DMA_ChannelIndex].Configuration);

    if( Enable )
    {
        regvalue  &= ~Enb;
    }
    else
    {
        regvalue  |= Holt;
    }

    WriteIO32(&pRegister->Channel[DMA_ChannelIndex].Configuration, regvalue);
}

//-----------------------------------------------------------------------------------------------


