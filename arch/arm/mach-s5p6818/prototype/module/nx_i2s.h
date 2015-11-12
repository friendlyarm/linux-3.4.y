#ifndef __NX_I2S_H__
#define __NX_I2S_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	I2S I2S
//------------------------------------------------------------------------------
//@{

	/// @brief	I2S Module's Register List
	struct	NX_I2S_RegisterSet
	{
		volatile U32 CON;		///< 0x00 :
		volatile U32 MOD;		///< 0x04 :
		volatile U32 FIC;		///< 0x08 :
		volatile U32 PSR;		///< 0x0C :
		volatile U32 TXD;		///< 0x10 :
		volatile U32 RXD;		///< 0x14 :
	};

	/// @brief	I2S interrupt for interrupt interface
	enum
	{
		NX_I2S_INT_PCMOUTUNDER	= 0,	///< PCM OUT Buffer Underflow interrupt
		NX_I2S_INT_PCMINOVER	= 1		///< PCM IN Buffer Overflow interrupt
	};

	/// @brief	I2S Operation Mode
    typedef enum
    {
        NX_I2S_CH_LEFT      =   0,
        NX_I2S_CH_RIGHT     =   1
    }   NX_I2S_CH;

    typedef enum
    {
        NX_I2S_BLC_16BIT    = 0,    // 16 bit per channel
        NX_I2S_BLC_8BIT     = 1,    // 8  bit per channel
        NX_I2S_BLC_24BIT    = 2     // 24 bit per channel
    }   NX_I2S_BLC;

    typedef enum
    {
        NX_I2S_IMS_MASTER 	= 0,   // master mode
        NX_I2S_IMS_SLAVE   	= 1,   // slave mode

    }   NX_I2S_IMS;

    typedef enum
    {
        NX_I2S_TXR_TXONLY   = 0,
        NX_I2S_TXR_RXONLY   = 1,
        NX_I2S_TXR_TXRX     = 2
    }   NX_I2S_TXR;

    typedef enum
    {
        NX_I2S_LRP_LEFT     = 0,    //  low  for left channel and high for right channel
        NX_I2S_LRP_RIGHT    = 1     //  high for left channel and low  for rignt channel
    }   NX_I2S_LRP;

	typedef enum
	{
		NX_I2S_SDF_I2S		=	0,	///< I2S Mode
		NX_I2S_SDF_MSB		=	1,	///< Left-Justified Mode.
		NX_I2S_SDF_LSB		=	2	///< Right-Justified Mode.
	}	NX_I2S_SDF ;

	typedef enum
	{
		NX_I2S_MS_MASTER	=	0,	///< master mode
		NX_I2S_MS_SLAVE		=	1	///< slave mode
	}	NX_I2S_MS;

	typedef enum
	{
		NX_I2S_CLOCK_INTERNAL		=	0,	///< internal clock source selected(pclk)
		NX_I2S_CLOCK_EXTERNAL		=	1	///< external clock source selected
	}	NX_I2S_CLOCK;

	typedef enum
	{
		NX_I2S_ROOTCLOCK_256FS		=	0,	///<
		NX_I2S_ROOTCLOCK_512FS		=	1,	///<
		NX_I2S_ROOTCLOCK_384FS		=	2,
		NX_I2S_ROOTCLOCK_768FS		=	3
	}	NX_I2S_ROOTCLOCK;

	typedef enum
	{
		NX_I2S_BITCLOCK_32FS		=	0,	///<
		NX_I2S_BITCLOCK_48FS		=	1,
		NX_I2S_BITCLOCK_16FS		=	2,
		NX_I2S_BITCLOCK_24FS		=	3
	}	NX_I2S_BITCLOCK;

	typedef enum
	{
		NX_I2S_BITLENGTH_16		=	0,	///<
		NX_I2S_BITLENGTH_8		=	1,
		NX_I2S_BITLENGTH_24		=	2
	}	NX_I2S_BITLENGTH;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_I2S_Initialize( void );
U32		NX_I2S_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_I2S_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_I2S_GetSizeOfRegisterSet( void );

void	NX_I2S_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_I2S_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_I2S_OpenModule( U32 ModuleIndex );
CBOOL	NX_I2S_CloseModule( U32 ModuleIndex );
CBOOL	NX_I2S_CheckBusy( U32 ModuleIndex );
CBOOL	NX_I2S_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_I2S_GetDMAIndex_PCMIn( U32 ModuleIndex );
U32		NX_I2S_GetDMAIndex_PCMOut( U32 ModuleIndex );
U32     NX_I2S_GetDMANumber( U32 ChannelIndex );
U32		NX_I2S_GetDMABusWidth( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Clock Control Interface
//@{
U32		NX_I2S_GetClockNumber( U32 ModuleIndex );
U32     NX_I2S_GetResetNumber( U32 ModuleIndex );
//@}

void    NX_I2S_TxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_TxChPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxChPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_TxDmaEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxDmaEnable( U32 ModuleIndex, CBOOL Enable );


//------------------------------------------------------------------------------
/// @name   I2C Interface
//@{

// The DMA-Related functions. 
void    NX_I2S_TxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_TxChPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxChPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_TxDmaEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxDmaEnable( U32 ModuleIndex, CBOOL Enable );

// The FIFO-Related functions. 
CBOOL   NX_I2S_GetChannelClockIndication( U32 ModuleIndex );
CBOOL   NX_I2S_GetTxFifoEmpty( U32 ModuleIndex );
CBOOL   NX_I2S_GetRxFifoEmpty( U32 ModuleIndex );
CBOOL   NX_I2S_GetTxFifoFull( U32 ModuleIndex );
CBOOL   NX_I2S_GetRxFifoFull( U32 ModuleIndex );
void    NX_I2S_TxFIFOFlushEnable( U32 ModuleIndex, CBOOL Enable );

U32     NX_I2S_GetRxFifoDataCount( U32 ModuleIndex );
U32     NX_I2S_GetTxFIFODataCount( U32 ModuleIndex );
void    NX_I2S_RxFIFOFlushEnable( U32 ModuleIndex, CBOOL Enable );
U32     NX_I2S_GetRxFIFODataCount( U32 ModuleIndex );

// The Mode-Related functions. 
void    NX_I2S_SetI2SEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_I2S_GetI2SEnable( U32 ModuleIndex );
void    NX_I2S_SetCodecClockDisable( U32 ModuleIndex, CBOOL Disable );
CBOOL   NX_I2S_GetCodecClockDisable( U32 ModuleIndex );

void    	NX_I2S_SetMasterSlaveMode( U32 ModuleIndex, NX_I2S_IMS Mode );
NX_I2S_IMS  NX_I2S_GetMasterSlaveMode( U32 ModuleIndex );
void    	NX_I2S_SetTxRxMode( U32 ModuleIndex, NX_I2S_TXR Mode );
NX_I2S_TXR	NX_I2S_GetTxRxMode( U32 ModuleIndex );
void    	NX_I2S_SetLRClockPolarity( U32 ModuleIndex, NX_I2S_LRP Polarity );
NX_I2S_LRP  NX_I2S_GetLRClockPolarity( U32 ModuleIndex );
void    	NX_I2S_SetBitLengthControl( U32 ModuleIndex, NX_I2S_BLC BitLength );
NX_I2S_BLC 	NX_I2S_GetBitLengthControl( U32 ModuleIndex );
void    	NX_I2S_SetSerialDataFormat( U32 ModuleIndex, NX_I2S_SDF DataFormat );
NX_I2S_SDF  NX_I2S_GetSerialDataFormat( U32 ModuleIndex );

// The Sampling Clock-Related functions. 
void    			NX_I2S_SetRootClockFrequency( U32 ModuleIndex, NX_I2S_ROOTCLOCK RootClock );
NX_I2S_ROOTCLOCK   	NX_I2S_GetRootClockFrequency( U32 ModuleIndex );
void    		NX_I2S_SetBitClockFrequency( U32 ModuleIndex, NX_I2S_BITCLOCK BitClock );
NX_I2S_BITCLOCK	NX_I2S_GetBitClockFrequency( U32 ModuleIndex );

// The Tramit/Receive-Related functions. 
void    NX_I2S_SetTxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength, U32 TxData );
U32     NX_I2S_GetRxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength );
//@}
#endif
