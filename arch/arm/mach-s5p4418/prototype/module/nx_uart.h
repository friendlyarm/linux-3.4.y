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

#define NX_UART_FIFO_DEPTH		32

/******************************************************************************/
/******************** MASKS FOR REGISTERS *************************************/
/******************************************************************************/

#define UTRSR_MASK            0x03
#define UTIIR_MASK            0x1f
#define UTFR_MASK             0xff
#define UTDMACR_MASK          0x3f

#define UARTDR_MASK           0xff
#define UARTECR_MASK          0xff
#define UARTRSR_MASK          0x0f
#define UARTLCR_H_MASK        0x7f
#define UARTLCR_M_MASK        0xff
#define UARTLCR_L_MASK        0xff
#define UARTCR_MASK           0x7f
#define UARTCR_newMASK        0xffff
#define UARTFR_MASK           0xf8
#define UARTILPR_MASK         0xff
#define UARTIBRD_MASK         0xffff
#define UARTFBRD_MASK         0x3f
#define UARTLCR_H_new_MASK    0xff
#define UARTIFLS_MASK         0x3f
#define UARTIMSC_MASK         0x7ff
#define UARTRIS_MASK          0xfff
#define UARTMIS_MASK          0xfff
#define UARTICR_MASK          0x7ff
#define UARTDMACR_MASK        0x07
#define MASK_IDREG            0x0ff



#define UARTTCR_MASK          0x07
#define UARTITIP_MASK         0xff
#define UARTITOP_MASK         0xffff
#define UARTTDR_MASK          0xfff

/******************************************************************************/
/******************** REGISTER BIT MASKS **************************************/
/******************************************************************************/

/* For RSTMODE_REG Register */
#define RSTMODE_ENABLE      0x01
#define PCLK_ENABLE         0x02
#define PCLKGEN             0x03
#define REFCLKGEN           0x04
#define REFCLKON            0x05
#define PCLKON              0x06

/* For UTIIR Register */
#define UT_MSINT            0x01
#define UT_RXINT            0x02
#define UT_TXINT            0x04
#define UT_RTISINT          0x08
#define UT_UARTINTR         0x10
#define UT_UARTEINTR        0x20

/* For UT_FORCED_ERROR Register */
#define FORCED_PARITY_ERR   0x01
#define FORCED_FRAMING_ERR  0x02
#define RX_JITTER_BITS      0x0c
#define RX_JITTER_SIGN      0x10
#define TX_JITTER_BITS      0x60
#define TX_JITTER_SIGN      0x80

/* For UTFR Register */
#define UTRXFIFO_EMPTY      0x01
#define UTRXFIFO_HALFFULL   0x02
#define UTRXFIFO_FULL       0x04
#define UTTXFIFO_EMPTY      0x08
#define UTTXFIFO_HALFEMPTY  0x10
#define UTTXFIFO_FULL       0x20
#define UTTXBUSY            0x40
#define UTRXBUSY            0x80

/* For UTRSR Register */
#define UTPE                0x02

/* For UTDMACR Register */
#define TXDMASREQ           0x04
#define RXDMASREQ           0x08
#define TXDMABREQ           0x10
#define RXDMABREQ           0x20

/* For UARTFR Register */
#define UART_CTS            0x01
#define UART_DSR            0x02
#define UART_DCD            0x04
#define UART_UBUSY          0x08
#define UART_RXFE           0x10
#define UART_TXFF           0x20
#define UART_RXFF           0x40
#define UART_TXFE           0x80
#define UART_RI             0x100

/* For UARTIFLS Register */
#define UART_RXFEIGHT       0x00
#define UART_RXFQUART       0x08
#define UART_RXFHALF        0x10
#define UART_RXF3QUART      0x18
#define UART_RXF7EIGHT      0x20
#define UART_TXFEIGHT       0x00
#define UART_TXFQUART       0x01
#define UART_TXFHALF        0x02
#define UART_TXF3QUART      0x03
#define UART_TXF7EIGHT      0x04

/* UARTRIS Register */
#define UART_RIRINT         0x01
#define UART_CTSRINT        0x02
#define UART_DCDRINT        0x04
#define UART_DSRRINT        0x08
#define UART_RXRINT         0x10
#define UART_TXRINT         0x20
#define UART_RTRINT         0x40
#define UART_FERINT         0x80
#define UART_PERINT         0x100
#define UART_BERINT         0x200
#define UART_OERINT         0x400

/* UARTMIS Register */
#define UART_RIMINT         0x01
#define UART_CTSMINT        0x02
#define UART_DCDMINT        0x04
#define UART_DSRMINT        0x08
#define UART_RXMINT         0x10
#define UART_TXMINT         0x20
#define UART_RTMINT         0x40
#define UART_FEMINT         0x80
#define UART_PEMINT         0x100
#define UART_BEMINT         0x200
#define UART_OEMINT         0x400

/* UARTDMACR Register */
#define UART_RXDMAE         0x01
#define UART_TXDMAE         0x02
#define DMAONERR            0x04

//------------------------------------------------------------------------------
/// @brief  UART register set structure
//------------------------------------------------------------------------------
struct NX_UART_RegisterSet
{
	volatile U32 DR;				// 0x00	// Data Register
	volatile U32 RSR_ECR;			// 0x04	// Receive Status Register / Error Clear Register
	volatile U32 __Reserved0[(0x18-0x08)/4];	// 0x08~0x14	// Reserved
	volatile U32 FR;				// 0x18	// Flag Register
	volatile U32 __Reserved1;		// 0x1C	// Reserved
	volatile U32 ILPR;				// 0x20	// IrDA Low Power Counter Register
	volatile U32 IBRD;				// 0x24	// IntegerBaud Rate Register
	volatile U32 FBRD;				// 0x28 // Fractional Baud Rate Register
	volatile U32 LCR_H;				// 0x2C // Line Control Register
	volatile U32 CR;				// 0x30 // Control Register
	volatile U32 IFLS;				// 0x34	// Interrupt FIFO Level Select Register
	volatile U32 IMSC;				// 0x38	// Interrupt Mask Set/Clear Register
	volatile U32 RIS;				// 0x3C // Raw Interrupt Status Register
	volatile U32 MIS;				// 0x40	// Masked Interrupt Status Register
	volatile U32 ICR;				// 0x44	// Interrupt Clear Register
	volatile U32 DMACR;				// 0x48	// DMA Control Register
	volatile U32 __Reserved2[(0x80-0x4C)/4];		// 0x4C~0x7C	// Reserved
	//volatile U32 __Reserved3[(0x90-0x80)/4];		// 0x80~0x8C	// Reserved
	volatile U32 TCR;				// 0x80 // Test Control Register
	volatile U32 ITIP;				// 0x84 // Integration Test Input Register
	volatile U32 ITOP;				// 0x88 // Integration Test Output Register
	volatile U32 TDR;				// 0x8C // Test Data Register
	volatile U32 __Reserved4[(0xFD0-0x90)/4];		// 0x90~0xFCC	// Reserved
	volatile U32 __Reserved5[(0xFE0-0xFD0)/4];		// 0xFD0~0xFDC	// Reserved
	volatile U32 PeriphID0;			// 0xFE0	// PeriphID0 Register
	volatile U32 PeriphID1;			// 0xFE4	// PeriphID1 Register
	volatile U32 PeriphID2;			// 0xFE8	// PeriphID2 Register
	volatile U32 PeriphID3;			// 0xFEC	// PeriphID3 Register
	volatile U32 PCellID0;			// 0xFF0	// PCellID0 Register
	volatile U32 PCellID1;			// 0xFF4	// PCellID1 Register
	volatile U32 PCellID2;			// 0xFF8	// PCellID2 Register
	volatile U32 PCellID3;			// 0xFFC	// PCellID3 Register
};


/// @brief	UART interrupt for interrupt interface
enum
{
	NX_UART_INT_RIM		= 0UL,	/// < RI Modem Interrupt
	NX_UART_INT_CTSM	= 1UL,	/// < CTS Modem Interrupt
	NX_UART_INT_DCDM	= 2UL,	/// < DCD Modem Interrupt
	NX_UART_INT_DSRM	= 3UL,	/// < DSR Modem Interrupt
	NX_UART_INT_RX		= 4UL,	/// < Receive Interrupt
	NX_UART_INT_TX		= 5UL,	/// < Transmit Interrupt
	NX_UART_INT_RT		= 6UL,	/// < Receive Timeout Interrupt
	NX_UART_INT_FE		= 7UL,	/// < Framing Error Interrupt
	NX_UART_INT_PE		= 8UL,	/// < Parity Error Interrupt
	NX_UART_INT_BE		= 9UL,	///	< Break Error Interrupt
	NX_UART_INT_OE		= 10UL	///	< Overrun Error Interrupt
};

enum
{
	NX_UART_CR_UARTEN	=	0UL,
	NX_UART_CR_SIREN	=	1UL,
	NX_UART_CR_IIRLP	=	2UL,
	NX_UART_CR_LPE		=	7UL,
	NX_UART_CR_TXE		=	8UL,
	NX_UART_CR_RXE		=	9UL,
	NX_UART_CR_DTR		=	10UL,
	NX_UART_CR_RTS		=	11UL,
	NX_UART_CR_OUT1	=	12UL,
	NX_UART_CR_OUT2	=	13UL,
	NX_UART_CR_RTSEN	=	14UL,
	NX_UART_CR_CTSEN	=	15UL
};

/// @brief Parity generation/check type
typedef enum
{
	NX_UART_PARITYMODE_NONE	= 0UL,	/// < No Parity
	NX_UART_PARITYMODE_ODD	= 1UL,	/// < Odd Parity
	NX_UART_PARITYMODE_EVEN	= 2UL,	/// < Even Parity
	NX_UART_PARITYMODE_FONE	= 3UL,	/// < Force 1 Parity
	NX_UART_PARITYMODE_FZ	= 4UL	/// < Force 0 Parity
}	NX_UART_PARITYMODE;

typedef enum
{
	NX_UART_ERRSTAT_FRAME	= 1UL<<0,	///< Frame Error
	NX_UART_ERRSTAT_PARITY	= 1UL<<1,	///< Parity Error
	NX_UART_ERRSTAT_BREAK	= 1UL<<2,	///< Break Signal receive
	NX_UART_ERRSTAT_OVERRUN	= 1UL<<3	///< Overrun Error

}	NX_UART_ERRSTAT ;

typedef enum
{
	NX_UART_FLAG_CTS	= 1UL<<0,	///< Clear to send
	NX_UART_FLAG_DSR	= 1UL<<1,	///< Data set ready
	NX_UART_FLAG_DCD	= 1UL<<2,	///< Data carrier detect
	NX_UART_FLAG_BUSY	= 1UL<<3,	///< UART Busy
	NX_UART_FLAG_RXFE	= 1UL<<4,	///< Receive FIFO empty
	NX_UART_FLAG_TXFF	= 1UL<<5,	///< Transmit FIFO full
	NX_UART_FLAG_RXFF	= 1UL<<6,	///< Receive FIFO full
	NX_UART_FLAG_TXFE	= 1UL<<7,	///< Transmit FIFO empty
	NX_UART_FLAG_RI		= 1UL<<8	///< Ring indicator

}NX_UART_FLAG;

typedef enum
{
	NX_UART_RXDMAE			= 1UL<<0,	///< Clear to send
	NX_UART_TXDMAE			= 1UL<<1,	///< Clear to send
	NX_UART_DMAONERR		= 1UL<<2,	///< Clear to send

}NX_UART_DMA;

typedef enum
{
	NX_UART_FIFOLEVEL1_8	= (NX_UART_FIFO_DEPTH*1/8),	///< 1/8n
	NX_UART_FIFOLEVEL2_8	= (NX_UART_FIFO_DEPTH*2/8),	///< 1/4n
	NX_UART_FIFOLEVEL4_8	= (NX_UART_FIFO_DEPTH*4/8),	///< 1/2n
	NX_UART_FIFOLEVEL6_8	= (NX_UART_FIFO_DEPTH*6/8),	///< 3/4n
	NX_UART_FIFOLEVEL7_8	= (NX_UART_FIFO_DEPTH*7/8),	///< 7/8n
	NX_UART_FIFOLEVEL_ERR	= 0xFFFFFFFFUL
}NX_UART_FIFOLEVEL;

typedef enum
{
	NX_UART_DATABIT_5	= 0UL,	///< 5 bits
	NX_UART_DATABIT_6	= 1UL,	///< 6 bits
	NX_UART_DATABIT_7	= 2UL,	///< 7 bits
	NX_UART_DATABIT_8	= 4UL,	///< 8 bits
	NX_UART_DATABIT_ERR	= 0xFFFFFFFFUL
}NX_UART_DATABIT;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_UART_Initialize( void );
U32		NX_UART_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
#ifdef	NUMBER_OF_UART_MODULE
U32		NX_UART_GetPhysicalAddress( U32 ModuleIndex );
#endif
#ifdef	NUMBER_OF_pl01115_Uart_modem_MODULE
U32		NX_UART_GetPhysicalAddressModem( U32 ModuleIndex );
#endif
#ifdef	NUMBER_OF_pl01115_Uart_nodma_MODULE
U32		NX_UART_GetPhysicalAddressNodma( U32 ModuleIndex );
#endif
#ifdef	NUMBER_OF_pl01115_Uart_modemnodma_MODULE
U32		NX_UART_GetPhysicalAddressModemNodma( U32 ModuleIndex );
#endif
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

U32		NX_UART_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_UART_GetDMAIndex_Tx( U32 ModuleIndex );
U32		NX_UART_GetDMAIndex_Rx( U32 ModuleIndex );
U32		NX_UART_GetDMABusWidth( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_UART_SetSIRMode( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetSIRMode( U32 ModuleIndex );
void	NX_UART_SetLoopBackMode( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetLoopBackMode( U32 ModuleIndex );
void	NX_UART_SetIntEnbWhenExceptionOccur( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetIntEnbWhenExceptionOccur( U32 ModuleIndex );
void	NX_UART_SetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITYMODE Parity,	U32 DataWidth,	U32 StopBit );
void	NX_UART_GetFrameConfiguration( U32 ModuleIndex, NX_UART_PARITYMODE* pParity, U32* pDataWidth, U32* pStopBit );
void	NX_UART_SetBRD( U32 ModuleIndex, U16 BRD );
U16		NX_UART_GetBRD( U32 ModuleIndex );
void	NX_UART_SetFBRD( U32 ModuleIndex, U16 FBRD );
U16		NX_UART_GetFBRD( U32 ModuleIndex );
U16		NX_UART_MakeBRD( U32 BaudRate, U32 CLKinHz );
U16		NX_UART_MakeFBRD( U32 BaudRate, U32 CLKinHz );
CBOOL	NX_UART_GetRxTimeOutEnb( U32 ModuleIndex );
void	NX_UART_SendBreak( U32 ModuleIndex );
void	NX_UART_SetTxDMAMode( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetTxDMAMode( U32 ModuleIndex );
void	NX_UART_SetRxDMAMode( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetRxDMAMode( U32 ModuleIndex );
void	NX_UART_SetUARTEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetUARTEnable( U32 ModuleIndex );

//@}

//--------------------------------------------------------------------------
/// @name	FIFO control operations
//--------------------------------------------------------------------------
//@{
void	NX_UART_SetTxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_FIFOLEVEL level );
NX_UART_FIFOLEVEL		NX_UART_GetTxFIFOTriggerLevel( U32 ModuleIndex );
void	NX_UART_SetRxFIFOTriggerLevel( U32 ModuleIndex, NX_UART_FIFOLEVEL level );
NX_UART_FIFOLEVEL		NX_UART_GetRxFIFOTriggerLevel( U32 ModuleIndex );
void	NX_UART_SetTXEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetTXEnable( U32 ModuleIndex );
void	NX_UART_SetRXEnable( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetRXEnable( U32 ModuleIndex );
void	NX_UART_SetFIFOEnb( U32 ModuleIndex, CBOOL bEnb );
CBOOL	NX_UART_GetFIFOEnb( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Status operations
//--------------------------------------------------------------------------
//@{
U32		NX_UART_GetTxRxStatus( U32 ModuleIndex );
U32		NX_UART_GetErrorStatus( U32 ModuleIndex );
void	NX_UART_ClearErrorStatus( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Modem operations
//--------------------------------------------------------------------------
//@{
void	NX_UART_SetDTR( U32 ModuleIndex, CBOOL bActive );
CBOOL	NX_UART_GetDTR( U32 ModuleIndex );
void	NX_UART_SetRTS( U32 ModuleIndex, CBOOL bActive );
CBOOL	NX_UART_GetRTS( U32 ModuleIndex );
void	NX_UART_SetAutoFlowControl( U32 ModuleIndex, CBOOL enable );
CBOOL	NX_UART_GetAutoFlowControl( U32 ModuleIndex );
U32		NX_UART_GetModemStatus( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Basic operations
//--------------------------------------------------------------------------
//@{
void	NX_UART_SendByte( U32 ModuleIndex, U8 Data );
U8		NX_UART_GetByte( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	UART Configuration Function
//--------------------------------------------------------------------------
//@{
void	NX_UART_SetLineConfig( U32 ModuleIndex, CBOOL sirMode, NX_UART_PARITYMODE Parity, U32 DataWidth, U32 StopBit );
void	NX_UART_SetControlConfig( U32 ModuleIndex, CBOOL txOper, CBOOL rxOper );
void	NX_UART_SetFIFOConfig( U32 ModuleIndex, CBOOL fifoEnb, U32 txLevel, U32 rxLevel );
void	NX_UART_SetBaudRateConfig( U32 ModuleIndex, U16 baudRate );
void	NX_UART_SetModemConfig( U32 ModuleIndex, CBOOL autoflow, CBOOL DTR, CBOOL RTS );

void	NX_UART_GetLineConfig( U32 ModuleIndex, U32* pSirMode, NX_UART_PARITYMODE* pParity, U32* pDataWidth, U32* pStopBit );
void	NX_UART_GetControlConfig( U32 ModuleIndex, CBOOL* pTxOper, CBOOL* pRxOper );
void	NX_UART_GetFIFOConfig( U32 ModuleIndex, U32* pFIFOEnb, U32* pTxLevel, U32* pRxLevel );
void	NX_UART_GetBaudRateConfig( U32 ModuleIndex, U16* pBaudRate );
void	NX_UART_GeztModemConfig( U32 ModuleIndex, U32* pAutoflow, U32* pDTR, U32* pRTS );
//@}

//--------------------------------------------------------------------------
/// @name	Register Backup & Store Function
//--------------------------------------------------------------------------
//@{
void	NX_UART_SetRSRECR( U32 ModuleIndex, U16 value );
U16		NX_UART_GetRSRECR( U32 ModuleIndex );

U16		NX_UART_GetFR( U32 ModuleIndex );

void	NX_UART_SetILPR( U32 ModuleIndex, U16 value );
U16		NX_UART_GetILPR( U32 ModuleIndex );

void	NX_UART_SetLCR_H( U32 ModuleIndex, U16 value );
U16		NX_UART_GetLCR_H( U32 ModuleIndex );

void	NX_UART_SetCR( U32 ModuleIndex, U16 value );
U16		NX_UART_GetCR( U32 ModuleIndex );

void	NX_UART_SetIFLS( U32 ModuleIndex, U16 value );
U16		NX_UART_GetIFLS( U32 ModuleIndex );

void	NX_UART_SetIMSC( U32 ModuleIndex, U16 value );
U16		NX_UART_GetIMSC( U32 ModuleIndex );

U16		NX_UART_GetRIS( U32 ModuleIndex );
U16		NX_UART_GetMIS( U32 ModuleIndex );
void	NX_UART_SetICR( U32 ModuleIndex, U16 value );

void	NX_UART_SetDMACR( U32 ModuleIndex, U16 value );
U16		NX_UART_GetDMACR( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Test Registers
//--------------------------------------------------------------------------
//@{
void	NX_UART_SetTCR( U32 ModuleIndex, U16 value );
U16		NX_UART_GetTCR( U32 ModuleIndex );

void	NX_UART_SetITIP( U32 ModuleIndex, U16 value );
U16		NX_UART_GetITIP( U32 ModuleIndex );

void	NX_UART_SetITOP( U32 ModuleIndex, U16 value );
U16		NX_UART_GetITOP( U32 ModuleIndex );

void	NX_UART_SetTDR( U32 ModuleIndex, U16 value );
U16		NX_UART_GetTDR( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	SIMIO
//--------------------------------------------------------------------------
void	Uart_Test_Exit( void );

//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_UART_H
