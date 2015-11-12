#ifndef _NX_UART_H
#define _NX_UART_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @brief  UART register set structure
//------------------------------------------------------------------------------
struct NX_UART_RegisterSet
{

	volatile U32	LCON	;	// 0x00	// Line Control
	volatile U32	UCON	;	// 0x04	// Control
	volatile U32	FCON	;	// 0x08	// FIFO Control
	volatile U32	MCON	;	// 0x0C	// Modem Control
	volatile U32	USTATUS	;	// 0x10	// Tx/Rx Status
	volatile U32	ESTATUS	;	// 0x14	// Rx Error Status
	volatile U32	FSTATUS	;	// 0x18	// FIFO Status
	volatile U32	MSTATUS	;	// 0x1C	// Modem Status
	volatile U32	THR		;	// 0x20	// Transmit Buffer
	volatile U32	RBR		;	// 0x24	// Recieve Buffer
	volatile U32	BRDR	;	// 0x28	// Baud Rate Driver
	volatile U32	FRACVAL	;	// 0x2C	// Driver Fractional Value
	volatile U32	INTP	;	// 0x30	// Instrrupt Pending
	volatile U32	INTSP	;	// 0x34	// Instrrupt Source
	volatile U32	INTM	;	// 0x38	// Instrrupt Mask
};

//---------------- LCON enum ----------------//
typedef enum
{
	NX_UART_SIGNAL_MODE_NORMAL		= 0UL,
	NX_UART_SIGNAL_MODE_INFRARED	= 1UL
}NX_UART_SIGNAL_MODE;

typedef enum
{
	NX_UART_NO_PARITY			= 0UL,
	NX_UART_PARITY_ODD			= 4UL,
	NX_UART_PARITY_EVEN			= 5UL,
	NX_UART_PARITY_FORCED_1		= 6UL,
	NX_UART_PARITY_FORCED_0		= 7UL
}NX_UART_PARITY;

typedef enum
{
	NX_UART_DATA_BIT_5	= 0UL,
	NX_UART_DATA_BIT_6	= 1UL,
	NX_UART_DATA_BIT_7	= 2UL,
	NX_UART_DATA_BIT_8	= 3UL
}NX_UART_DATA_BIT;

typedef enum
{
	NX_UART_STOP_ONEBIT	= 0UL,
	NX_UART_STOP_TWOBIT	= 1UL
} NX_UART_STOP_BIT;

//---------------- UCON enum ----------------//
typedef enum
{
	NX_UART_DMA_BURST_SIZE_1	= 0UL,
	NX_UART_DMA_BURST_SIZE_4	= 1UL,
	NX_UART_DMA_BURST_SIZE_8	= 2UL,
	NX_UART_DMA_BURST_SIZE_16	= 3UL
}NX_UART_DMA_BURST_SIZE;

typedef enum
{
	NX_UART_INT_TYPE_PULSE		= 0UL,
	NX_UART_INT_TYPE_LEVEL		= 1UL
}NX_UART_INT_TYPE;

typedef enum
{
	NX_UART_OPMODE_DISABLE	= 0UL,
	NX_UART_OPMODE_POLLING	= 1UL,
	NX_UART_OPMODE_DMA		= 2UL
}NX_UART_OPMODE;



//---------------- FCON enum ----------------//
typedef enum
{
	NX_UART_FIFOLEVEL_8BYTE  = 0,
	NX_UART_FIFOLEVEL_16BYTE = 1,
	NX_UART_FIFOLEVEL_24BYTE = 2,
	NX_UART_FIFOLEVEL_32BYTE = 3,
	NX_UART_FIFOLEVEL_40BYTE = 4,
	NX_UART_FIFOLEVEL_48BYTE = 5,
	NX_UART_FIFOLEVEL_56BYTE = 6,
	NX_UART_FIFOLEVEL_64BYTE = 7,
} NX_UART_RX_FIFOLEVEL;

typedef enum
{
	NX_UART_FIFOLEVEL_0BYTE  = 0,
	NX_UART_FIFOLEVEL_9BYTE  = 1,
	NX_UART_FIFOLEVEL_17BYTE = 2,
	NX_UART_FIFOLEVEL_25BYTE = 3,
	NX_UART_FIFOLEVEL_33BYTE = 4,
	NX_UART_FIFOLEVEL_41BYTE = 5,
	NX_UART_FIFOLEVEL_49BYTE = 6,
	NX_UART_FIFOLEVEL_57BYTE = 7,
} NX_UART_TX_FIFOLEVEL;;

//---------------- MCON enum ----------------//
typedef enum
{
	NX_UART_AFC_INACTIVATE	= 0UL, 	// High Level
	NX_UART_AFC_ACTIVATE	= 1UL	// Low Level
}NX_UART_AFC;

typedef enum
{
	NX_UART_RTS_TL_1,
} NX_UART_RTS_FIFOLEVEL;


//---------------- TRSTATE enum ----------------//

typedef enum
{
	NX_UART_DMA_FSM_IDLE		= 0UL,
	NX_UART_DMA_FSM_BURST_REQ	= 1UL,
	NX_UART_DMA_FSM_BURST_ACK	= 2UL,
	NX_UART_DMA_FSM_BURST_NEXT	= 3UL,
	NX_UART_DMA_FSM_SINGLE_REQ	= 4UL,
	NX_UART_DMA_FSM_SINGLE_ACK	= 5UL,
	NX_UART_DMA_FSM_SINGLE_NEXT	= 6UL,
	NX_UART_DMA_FSM_LBURST_REQ	= 7UL,
	NX_UART_DMA_FSM_LBURST_ACK	= 8UL,
	NX_UART_DMA_FSM_LBURST_NEXT	= 9UL,
	NX_UART_DMA_FSM_LSINGLE_ACK	= 10UL
}NX_UART_DMA_FSM;

//---------------- TRSTATE enum ----------------//


//---------------- UINTPn enum ----------------//
typedef enum
{
	NX_UART_INT_RXD		= 0UL,
	NX_UART_INT_ERROR	= 1UL,
	NX_UART_INT_TXD		= 2UL,
	NX_UART_INT_MODEM	= 3UL
}NX_UART_INT;


//---------------- FSTATUS enum ----------------//

//-------------------DEFINE--------------------//
#define NUMBER_OF_UART_CHANNEL	6

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_UART_Initialize( void );
U32		NX_UART_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_UART_GetPhysicalAddress( U32 ModuleIndex );

//U32		NX_UART_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
U32		NX_UART_GetNumberOfReset( void );
U32		NX_UART_GetSizeOfRegisterSet( void );
void	NX_UART_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_UART_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_UART_OpenModule( U32 ModuleIndex );
CBOOL	NX_UART_CloseModule( U32 ModuleIndex );
CBOOL	NX_UART_CheckBusy( U32 ModuleIndex );
CBOOL	NX_UART_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock ,reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_UART_GetClockNumber ( U32 ModuleIndex );
U32 NX_UART_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_UART_GetInterruptNumber( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
// Interrupt Polling Register & Interrupt Source Register & Interrupt Mask Register
//-------------------------------------------------------------
void	NX_UART_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_UART_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_UART_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_UART_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

void	NX_UART_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_UART_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_UART_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_UART_ClearInterruptPendingAll( U32 ModuleIndex );

void	NX_UART_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32		NX_UART_GetInterruptEnable32( U32 ModuleIndex );
U32		NX_UART_GetInterruptPending32( U32 ModuleIndex );
void	NX_UART_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );
//-------------------------------------------------------------
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_UART_GetDMAIndex_Tx( U32 ModuleIndex );
U32		NX_UART_GetDMAIndex_Rx( U32 ModuleIndex );
U32		NX_UART_GetDMABusWidth( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	UART Pad Enable Function
//@{
void 	NX_UART_EnablePAD( U32 ModuleIndex, U32 ModeIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{

// Line Control Register
//-------------------------------------------------------------
void				NX_UART_SetInfraredMode( U32 ModuleIndex, NX_UART_SIGNAL_MODE IMode );
NX_UART_SIGNAL_MODE	NX_UART_GetInfraredMode( U32 ModuleIndex );

void				NX_UART_SetParityMode( U32 ModuleIndex, NX_UART_PARITY ParityMode );
NX_UART_PARITY		NX_UART_GetParityMode( U32 ModuleIndex );
void				NX_UART_SetStopBit( U32 ModuleIndex, NX_UART_STOP_BIT StopBit );
NX_UART_STOP_BIT	NX_UART_GetStopBit( U32 ModuleIndex );
void				NX_UART_SetDataWidth( U32 ModuleIndex, NX_UART_DATA_BIT DataWidth );
NX_UART_DATA_BIT	NX_UART_GetDataWidth( U32 ModuleIndex );

void	NX_UART_SetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITY Parity,	NX_UART_DATA_BIT DataWidth,	NX_UART_STOP_BIT StopBit );
void	NX_UART_GetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITY* pParity, NX_UART_DATA_BIT* pDataWidth, NX_UART_STOP_BIT* pStopBit );
//-------------------------------------------------------------

//Control Register
//-------------------------------------------------------------
void					NX_UART_SetTxDMABurstSize( U32 ModuleIndex, NX_UART_DMA_BURST_SIZE BrustSize );
NX_UART_DMA_BURST_SIZE	NX_UART_GetTxDMABurstSize( U32 ModuleIndex );
void					NX_UART_SetRxDMABurstSize( U32 ModuleIndex, NX_UART_DMA_BURST_SIZE BrustSize );
NX_UART_DMA_BURST_SIZE	NX_UART_GetRxDMABurstSize( U32 ModuleIndex );

void	NX_UART_SetRxDMASuspendEnable( U32 ModuleIndex, U32 Enable );
CBOOL	NX_UART_GetRxDMASuspendEnable( U32 ModuleIndex );

//----------------------------------------------------------------------------------------
void	NX_UART_SetRxTimeOutEnable( U32 ModuleIndex, CBOOL Enable ); 
CBOOL	NX_UART_GetRxTimeOutEnable( U32 ModuleIndex );

void	NX_UART_SetRxTimeOutInterval( U32 ModuleIndex, U32 TimeOutInterval ); // Interval-4bit
U8		NX_UART_GetRxTimeOutInterval( U32 ModuleIndex );
void	NX_UART_SetRxFIFOEmptyTimeOutEnable( U32 ModuleIndex, U32 Enable );
CBOOL	NX_UART_GetRxFIFOEmptyTimeOutEnable( U32 ModuleIndex );
//----------------------------------------------------------------------------------------

void	NX_UART_SetTxInterruptType( U32 ModuleIndex, CBOOL Type ); 
CBOOL	NX_UART_GetTxInterruptType( U32 ModuleIndex );
void	NX_UART_SetRxInterruptType( U32 ModuleIndex, CBOOL Type ); 
CBOOL	NX_UART_GetRxInterruptType( U32 ModuleIndex );

void	NX_UART_SetRxErrorStatusInterruptEnable( U32 ModuleIndex, CBOOL Enable ); 
CBOOL	NX_UART_GetRxErrorStatusInterruptEnable( U32 ModuleIndex );
//----------------------------------------------------------------------------------------

void	NX_UART_SetLoopBackMode( U32 ModuleIndex, CBOOL Enable ); 
CBOOL	NX_UART_GetLoopBackMode( U32 ModuleIndex );
void	NX_UART_SetSendBreakSignal( U32 ModuleIndex, CBOOL Enable ); 
CBOOL	NX_UART_GetSendBreakSignal( U32 ModuleIndex );
//----------------------------------------------------------------------------------------
void	NX_UART_SetTransmitMode( U32 ModuleIndex, NX_UART_OPMODE mode );
NX_UART_OPMODE		NX_UART_GetTransmitMode( U32 ModuleIndex );

void	NX_UART_SetRecieveMode( U32 ModuleIndex, NX_UART_OPMODE mode );
NX_UART_OPMODE		NX_UART_GetRecieveMode( U32 ModuleIndex );
//----------------------------------------------------------------------------------------

// FIFO Control Register
//-------------------------------------------------------------
void					NX_UART_SetTxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_TX_FIFOLEVEL Level ); 
NX_UART_TX_FIFOLEVEL	NX_UART_GetTxFIFOTriggerLevel( U32 ModuleIndex );
void					NX_UART_SetRxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_RX_FIFOLEVEL Level ); 
NX_UART_RX_FIFOLEVEL	NX_UART_GetRxFIFOTriggerLevel( U32 ModuleIndex );

void	NX_UART_SetTxFIFOReset( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_UART_GetTxFIFOReset( U32 ModuleIndex );
void	NX_UART_SetRxFIFOReset( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_UART_GetRxFIFOReset( U32 ModuleIndex );
void	NX_UART_SetFIFOEnb( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_UART_GetFIFOEnb( U32 ModuleIndex );

void	NX_UART_SetFIFOConfig( U32 ModuleIndex, CBOOL fifoEnb, U32 txLevel, U32 rxLevel );
void	NX_UART_GetFIFOConfig( U32 ModuleIndex, CBOOL* fifoEnb, U32* txLevel, U32* rxLevel );
//-------------------------------------------------------------


//Modem Control Register
//-------------------------------------------------------------
void	NX_UART_SetRTSTriggerLevel( U32 ModuleIndex, NX_UART_RTS_FIFOLEVEL Level );
NX_UART_RTS_FIFOLEVEL	NX_UART_GetRTSTriggerLevel( U32 ModuleIndex );

void	NX_UART_SetAutoFlowControl( U32 ModuleIndex, NX_UART_AFC Enable );
CBOOL	NX_UART_GetAutoFlowControl( U32 ModuleIndex );

void	NX_UART_SetModemInterruptEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_UART_GetModemInterruptEnable( U32 ModuleIndex );

void	NX_UART_SetRTSLevel( U32 ModuleIndex, CBOOL Active );
CBOOL	NX_UART_GetRTSLevel( U32 ModuleIndex );
//-------------------------------------------------------------

// Modem Status Register
//-------------------------------------------------------------
CBOOL	NX_UART_GetCTSStatus( U32 ModuleIndex );		// 0 : Not Change, 1:Change
CBOOL	NX_UART_GetCTSLevel( U32 ModuleIndex );		// 0 : high, 1: low
//-------------------------------------------------------------


// TX/RX Status Register
//-------------------------------------------------------------
U8		NX_UART_GetTimeOutStatus( U32 ModuleIndex );
NX_UART_DMA_FSM		NX_UART_GetTxDMAFSMState( U32 ModuleIndex );
NX_UART_DMA_FSM		NX_UART_GetRxDMAFSMState( U32 ModuleIndex );


void	NX_UART_SetRxTimeOutState( U32 ModuleIndex, CBOOL Status );	//Rx-TImeOut Clear
CBOOL	NX_UART_GetRxTimeOutState( U32 ModuleIndex );
//------------------------------------------------------------------
CBOOL	NX_UART_GetTransmitterEmpty( U32 ModuleIndex );
CBOOL	NX_UART_GetTxBufferEmpty( U32 ModuleIndex );
CBOOL	NX_UART_GetRxBufferEmpty( U32 ModuleInde );

//------------------------------------------------------------------


// Rx Error Status Register
//-------------------------------------------------------------
U32		NX_UART_GetErrorStatus( U32 ModuleIndex );
//-------------------------------------------------------------


// FIFO Status Register
//-------------------------------------------------------------
CBOOL	NX_UART_GetTxFIFOFull( U32 ModuleIndex );
U8		NX_UART_GetTxFIFOCount( U32 ModuleIndex );

CBOOL	NX_UART_GetRxFIFOError( U32 ModuleIndex );

CBOOL	NX_UART_GetRxFIFOFull( U32 ModuleIndex );
U8		NX_UART_GetRxFIFOCount( U32 ModuleIndex );
//-------------------------------------------------------------


// Transmit Buffer Register & Receive Buffer Register
//-------------------------------------------------------------
void	NX_UART_SendByte( U32 ModuleIndex, U8 Data );
U8		NX_UART_GetByte( U32 ModuleIndex );
//-------------------------------------------------------------

// Baud Rate Divisor Register
//-------------------------------------------------------------
void	NX_UART_SetBRD( U32 ModuleIndex, U16 BRD );
U16		NX_UART_GetBRD( U32 ModuleIndex );
//-------------------------------------------------------------

// Divisor Fractional Value Register
//-------------------------------------------------------------
void	NX_UART_SetFBRD( U32 ModuleIndex, U8 FBRD );
U8		NX_UART_GetFBRD( U32 ModuleIndex );
//-------------------------------------------------------------

//Baud Rate (Interger & Fractional) Making Function.
//-------------------------------------------------------------
U16		NX_UART_MakeBRD( U32 BaudRate, U32 CLKinHz );
U8		NX_UART_MakeFBRD( U32 BaudRate, U32 CLKinHz );
//-------------------------------------------------------------


//@}



//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_UART_H
