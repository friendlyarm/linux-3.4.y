#ifndef __NX_SSP_H__
#define __NX_SSP_H__

#include "../base/nx_prototype.h"


#ifdef	__cplusplus
extern "C"
{
#endif

/// @brief	SSP Module's Register List
struct	NX_SSP_RegisterSet
{
	volatile U32	CH_CFG;				///< 0x00 :
	volatile U32	_Reserved0;			///< 0x04 :
	volatile U32	MODE_CFG;			///< 0x08 :
	volatile U32	CS_REG;				///< 0x0C :
	volatile U32	SPI_INT_EN;			///< 0x10 :
	volatile U32	SPI_STATUS;			///< 0x14 :
	volatile U32	SPI_TX_DATA;		///< 0x18 :
	volatile U32	SPI_RX_DATA;		///< 0x1C :
	volatile U32	PACKET_CNT_REG;		///< 0x20 :
	volatile U32	PENDING_CLR_REG;	///< 0x24 :
	volatile U32	SWAP_CFG;			///< 0x28 :
	volatile U32	FB_CLK_SEL;			///< 0x2C :
};

typedef enum {
	TXFIFO_RDY  = 0,
	RXFIFO_RDY  = 1,
	TX_UNDERRUN = 2,
	TX_OVERRUN  = 3,
	RX_UNDERRUN = 4,
	RX_OVERRUN  = 5,
	TRAILING    = 6, // RX FIFO is not empty, but did not read for a while
} NX_SSP_INTR_MASK;

typedef enum {
	SINGLE = 0UL,
	BURST_4 = 1UL,
} NX_SSP_DMA_BURSTSIZE;

typedef enum
{
	NX_SSP_MODE_MASTER	= 0,        // Master Mode
	NX_SSP_MODE_SLAVE   = 1         // Slave  Mode
} NX_SSP_MODE;

typedef enum
{
	NX_SSP_FORMAT_A	= 0UL,	///< Format A, SPH = 0
	NX_SSP_FORMAT_B	= 1UL		///< Format B, SPH = 1
}NX_SSP_FORMAT;

typedef enum
{
	NX_SSP_CS_MODE_AUTO   = 1UL,
	NX_SSP_CS_MODE_MANUAL = 0UL,
} NX_SSP_CS_MODE ;

#define  NX_SSP_INTR_MAXNUMBER   7

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_SSP_Initialize ( void );
U32		NX_SSP_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_SSP_GetPhysicalAddress		( U32 ModuleIndex );
U32		NX_SSP_GetSizeOfRegisterSet		( void );
void	NX_SSP_SetBaseAddress			( U32 ModuleIndex, void* BaseAddress );
void*	NX_SSP_GetBaseAddress			( U32 ModuleIndex );
CBOOL	NX_SSP_OpenModule				( U32 ModuleIndex );
CBOOL	NX_SSP_CloseModule				( U32 ModuleIndex );
CBOOL	NX_SSP_CheckBusy				( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_SSP_GetInterruptNumber	( U32 ModuleIndex );
void	NX_SSP_SetInterruptEnable		( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum, CBOOL Enable );
CBOOL	NX_SSP_GetInterruptEnable		( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum );
CBOOL	NX_SSP_GetInterruptPending	( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum );
void	NX_SSP_ClearInterruptPending	( U32 ModuleIndex, NX_SSP_INTR_MASK IntNum );

void	NX_SSP_SetInterruptEnableAll	( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetInterruptEnableAll	( U32 ModuleIndex );
CBOOL	NX_SSP_GetInterruptPendingAll	( U32 ModuleIndex );
void	NX_SSP_ClearInterruptPendingAll	( U32 ModuleIndex );

NX_SSP_INTR_MASK	NX_SSP_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_SSP_GetDMAIndex_Tx( U32 ModuleIndex );
U32		NX_SSP_GetDMAIndex_Rx( U32 ModuleIndex );
U32		NX_SSP_GetDMABusWidth( U32 ModuleIndex );
//@}

U32 NX_SSP_GetClockNumber (U32 ModuleIndex);
U32	NX_SSP_GetResetNumber (U32 ModuleIndex, U32 ChannelIndex);


//------------------------------------------------------------------------------
///	@name	Register Backup & Store Function
void 	NX_SSP_EnablePAD				( U32 ModuleIndex ); // 해당 모듈에서 사용하는 모든 PAD를 enable 시킨다..
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
///	@name	Clock Control Interface
//@{
//	 	NK3 SPI has only 2x clock divider.
// 		SCLK_SPI must be divided in System Controller for operation .
//@}

//--------------------------------------------------------------------------
/// @name Configuration Function
//--------------------------------------------------------------------------
//@{
void	NX_SSP_SetDMATransferMode( U32 ModuleIndex, CBOOL bDMA );
CBOOL	NX_SSP_GetDMATransferMode( U32 ModuleIndex );

void	NX_SSP_SetDMATransmitMode( U32 ModuleIndex, CBOOL bDMA );
CBOOL	NX_SSP_GetDMATransmitMode( U32 ModuleIndex );
void	NX_SSP_SetDMAReceiveMode( U32 ModuleIndex, CBOOL bDMA );
CBOOL	NX_SSP_GetDMAReceiveMode( U32 ModuleIndex );

void	NX_SSP_SetDMABurstSize( U32 ModuleIndex , NX_SSP_DMA_BURSTSIZE Mode );
NX_SSP_DMA_BURSTSIZE	NX_SSP_GetDMABurstSize( U32 ModuleIndex );

//--------------------------------------------------------------------------
void	NX_SSP_SetBitWidth( U32 ModuleIndex, U32 bitWidth );
U32		NX_SSP_GetBitWidth( U32 ModuleIndex );
void	NX_SSP_SetSlaveMode( U32 ModuleIndex, NX_SSP_MODE bSlave );
CBOOL	NX_SSP_GetSlaveMode( U32 ModuleIndex );

void	NX_SSP_SetSlaveOutputEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetSlaveOutputEnable( U32 ModuleIndex );

void	NX_SSP_SetClockPolarityInvert( U32 ModuleIndex, CBOOL Invert );
CBOOL	NX_SSP_GetClockPolarityInvert( U32 ModuleIndex );
void			NX_SSP_SetSPIFormat( U32 ModuleIndex, NX_SSP_FORMAT Format);
NX_SSP_FORMAT	NX_SSP_GetSPIFormat( U32 ModuleIndex );


void	NX_SSP_SetNSSOUT( U32 ModuleIndex, CBOOL NSSOUT );
void	NX_SSP_SetNCSTIMECount( U32 ModuleIndex, U32 TimeCount);
U32		NX_SSP_GetNCSTIMECount( U32 ModuleIndex );
void			NX_SSP_SetCSMode( U32 ModuleIndex, NX_SSP_CS_MODE Mode );
NX_SSP_CS_MODE	NX_SSP_GetCSMode( U32 ModuleIndex );

//@}

//--------------------------------------------------------------------------
/// @name Operation Function
//--------------------------------------------------------------------------
//@{
void	NX_SSP_ResetFIFO( U32 ModuleIndex );

void	NX_SSP_SetEnable( U32 ModuleIndex, CBOOL bEnable );
CBOOL	NX_SSP_GetEnable( U32 ModuleIndex );

void	NX_SSP_SetTrailingCount( U32 ModuleIndex, U16 TrailingCnt );
U16		NX_SSP_GetTralingCount( U32 ModuleIndex );
U32		NX_SSP_GetTrailingByte( U32 ModuleIndex );


U8		NX_SSP_GetByte(U32 ModuleIndex);
U16		NX_SSP_GetHalfWord(U32 ModuleIndex);
U32		NX_SSP_GetWord(U32 ModuleIndex);

void	NX_SSP_PutByte(U32 ModuleIndex, U8 ByteData);
void	NX_SSP_PutHalfWord(U32 ModuleIndex, U16 HalfWordData);
void	NX_SSP_PutWord(U32 ModuleIndex, U32 HalfWordData);


void	NX_SSP_SetPacketCountEnb( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetPacketCountEnb( U32 ModuleIndex );
void	NX_SSP_SetPacketCount( U32 ModuleIndex, U16 PacketCount );
U16		NX_SSP_GetPacketCount( U32 ModuleIndex );

//@}

//--------------------------------------------------------------------------
/// @name FIFO State Check Function
//--------------------------------------------------------------------------
//@{

CBOOL	NX_SSP_IsTxFIFOEmpty(U32 ModuleIndex);
CBOOL	NX_SSP_IsTxFIFOFull(U32 ModuleIndex);
CBOOL	NX_SSP_IsRxFIFOEmpty(U32 ModuleIndex);
CBOOL	NX_SSP_IsRxFIFOFull(U32 ModuleIndex);

CBOOL	NX_SSP_IsTxRxEnd( U32 ModuleIndex ); // CheckBusy로 대체.

U32		NX_SSP_GetTxFIFOLVL(U32 ModuleIndex);
U32		NX_SSP_GetRxFIFOLVL(U32 ModuleIndex);

void  	NX_SSP_SetRXRDYLVL( U32 ModuleIndex, U32 RX_RDY_LVL );
void  	NX_SSP_SetTXRDYLVL( U32 ModuleIndex, U32 TX_RDY_LVL );

U32		NX_SSP_GetTxFIFOLVL(U32 ModuleIndex);
U32		NX_SSP_GetRxFIFOLVL(U32 ModuleIndex);

void	NX_SSP_SetRXRDYLVL( U32 ModuleIndex, U32 RX_RDY_LVL );
U8		NX_SSP_GetRXRDYLVL( U32 ModuleIndex );
void  	NX_SSP_SetTXRDYLVL( U32 ModuleIndex, U32 TX_RDY_LVL );
U8  	NX_SSP_GetTXRDYLVL( U32 ModuleIndex );

//@}

//--------------------------------------------------------------------------
/// @name Only Slave Mode Function
//--------------------------------------------------------------------------
//@{
void  	NX_SSP_SetHIGHSPEEDMode( U32 ModuleIndex, CBOOL Enable);
CBOOL 	NX_SSP_GetHIGHSPEEDMode( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name Transmiter & Receive Data Swap Function
//--------------------------------------------------------------------------
//@{
void	NX_SSP_SetTxHalfWordSwap( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetTxHalfWordSwap( U32 ModuleIndex );
void	NX_SSP_SetRxHalfWordSwap( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetRxHalfWordSwap( U32 ModuleIndex );

void	NX_SSP_SetTxByteSwap( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetTxByteSwap( U32 ModuleIndex );
void	NX_SSP_SetRxByteSwap( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetRxByteSwap( U32 ModuleIndex );

void	NX_SSP_SetTxBitSwap( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetTxBitSwap( U32 ModuleIndex );
void	NX_SSP_SetRxBitSwap( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_SSP_GetRxBitSwap( U32 ModuleIndex );
//@}



#ifdef	__cplusplus
}
#endif

#endif // __NX_SSP_H__

