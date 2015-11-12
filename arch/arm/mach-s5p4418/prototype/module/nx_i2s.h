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
        NX_I2S_IMS_DIVIDE   	  = 0,   // internal master mode
        NX_I2S_IMS_BYPASS   	  = 1,   // external master mode
        NX_I2S_IMS_PCLK_SLAVE     = 2,   // slave mode (PCLK)
        NX_I2S_IMS_CODECLKI_SLAVE = 3,	 // slave mode (CODCLKI)
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

//------------------------------------------------------------------------------
/// @name   I2C Interface
//@{
CBOOL   NX_I2S_GetChannelClockIndication( U32 ModuleIndex );
CBOOL   NX_I2S_GetTxFifoEmpty( U32 ModuleIndex );
CBOOL   NX_I2S_GetRxFifoEmpty( U32 ModuleIndex );
CBOOL   NX_I2S_GetTxFifoFull( U32 ModuleIndex );
CBOOL   NX_I2S_GetRxFifoFull( U32 ModuleIndex );
void    NX_I2S_TxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxDmaPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_TxChPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxChPauseEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_TxDmaEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_RxDmaEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_I2SEnable( U32 ModuleIndex, CBOOL Enable );
void    NX_I2S_SetBitLengthControl( U32 ModuleIndex, NX_I2S_BLC BitLength );
U32   	NX_I2S_GetBitLengthControl( U32 ModuleIndex );

void    NX_I2S_CodecClockDisable( U32 ModuleIndex, CBOOL Enable );
 U32    NX_I2S_GetCodecClockDisable( U32 ModuleIndex );
void    NX_I2S_SetMasterSlaveMode( U32 ModuleIndex, NX_I2S_IMS Mode );
 U32   	NX_I2S_GetMasterSlaveMode( U32 ModuleIndex );

void    NX_I2S_SetTxRxMode( U32 ModuleIndex, NX_I2S_TXR Mode );
void    NX_I2S_SetLRClockPolarity( U32 ModuleIndex, NX_I2S_LRP Polarity );
 U32   	NX_I2S_GetLRClockPolarity( U32 ModuleIndex );

void    NX_I2S_SetSerialDataFormat( U32 ModuleIndex, NX_I2S_SDF DataFormat );
U32   	NX_I2S_GetSerialDataFormat( U32 ModuleIndex );

void    NX_I2S_SetRootClockFrequency( U32 ModuleIndex, NX_I2S_ROOTCLOCK RootClock );
U32  	NX_I2S_GetRootClockFrequency( U32 ModuleIndex );
void    NX_I2S_SetBitClockFrequency( U32 ModuleIndex, NX_I2S_BITCLOCK BitClock );
U32		NX_I2S_GetBitClockFrequency( U32 ModuleIndex );

void    NX_I2S_TxFifoFlushEnable( U32 ModuleIndex, CBOOL Enable );
U32     NX_I2S_GetTxFifoDataCount( U32 ModuleIndex );
void    NX_I2S_RxFifoFlushEnable( U32 ModuleIndex, CBOOL Enable );
U32     NX_I2S_GetRxFifoDataCount( U32 ModuleIndex );
void    NX_I2S_PrescalerEnable( U32 ModuleIndex, CBOOL Enable );
U32		NX_I2S_GetPrescalerEnable( U32 ModuleIndex );
void    NX_I2S_SetPrescalerValue( U32 ModuleIndex, U32 PsVal );
 U32    NX_I2S_GetPrescalerValue( U32 ModuleIndex );
void    NX_I2S_SetTxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength, U32 TxData );
U32     NX_I2S_GetRxData( U32 ModuleIndex, NX_I2S_CH ChannelIndex, NX_I2S_BITLENGTH BitLength );
//@}



#if 0
//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
S32		NX_I2S_GetInterruptNumber( void );

void	NX_I2S_SetInterruptEnable( S32 IntNum, CBOOL Enable );
CBOOL	NX_I2S_GetInterruptEnable( S32 IntNum );
CBOOL	NX_I2S_GetInterruptPending( S32 IntNum );
void	NX_I2S_ClearInterruptPending( S32 IntNum );

void	NX_I2S_SetInterruptEnableAll( CBOOL Enable );
CBOOL	NX_I2S_GetInterruptEnableAll( void );
CBOOL	NX_I2S_GetInterruptPendingAll( void );
void	NX_I2S_ClearInterruptPendingAll( void );

void	NX_I2S_SetInterruptEnable32( U32 EnableFlag );
U32		NX_I2S_GetInterruptEnable32( void );
U32		NX_I2S_GetInterruptPending32( void );
void	NX_I2S_ClearInterruptPending32( U32 PendingFlag );

S32		NX_I2S_GetInterruptPendingNumber( void );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//@{
U32		NX_I2S_GetDMAIndex_PCMIn( void );
U32		NX_I2S_GetDMAIndex_PCMOut( void );
U32		NX_I2S_GetDMABusWidth( void );
//@}

//------------------------------------------------------------------------------
///	@name	Clock Control Interface
//@{
void		NX_I2S_SetClockPClkMode( NX_PCLKMODE mode );
NX_PCLKMODE	NX_I2S_GetClockPClkMode( void );
void		NX_I2S_SetClockSource( U32 Index, U32 ClkSrc );
U32			NX_I2S_GetClockSource( U32 Index );
void		NX_I2S_SetClockDivisor( U32 Index, U32 Divisor );
U32			NX_I2S_GetClockDivisor( U32 Index );
void		NX_I2S_SetClockOutInv( U32 Index, CBOOL OutClkInv );
CBOOL		NX_I2S_GetClockOutInv( U32 Index );
void		NX_I2S_SetClockOutEnb( U32 Index, CBOOL OutClkEnb );
CBOOL		NX_I2S_GetClockOutEnb( U32 Index );
void		NX_I2S_SetClockDivisorEnable( CBOOL Enable );
CBOOL		NX_I2S_GetClockDivisorEnable( void );
//@}

//------------------------------------------------------------------------------
/// @name I2S Configuration Function
//------------------------------------------------------------------------------
//@{
void		NX_I2S_SetMasterMode( CBOOL Enable );
CBOOL		NX_I2S_GetMasterMode( void );
void		NX_I2S_SetInterfaceMode( NX_I2S_IF mode );
NX_I2S_IF	NX_I2S_GetInterfaceMode( void );
void		NX_I2S_SetSyncPeriod( U32 period );
U32			NX_I2S_GetSyncPeriod( void );
//@}

//------------------------------------------------------------------------------
/// @name I2S Control Function
//------------------------------------------------------------------------------
//@{
void	NX_I2S_SetLinkOn( void );
CBOOL	NX_I2S_GetLinkOn( void );
void	NX_I2S_SetControllerReset( CBOOL Enable );
CBOOL	NX_I2S_GetControllerReset( void );
void	NX_I2S_SetOutputEnable( CBOOL Enable );
CBOOL	NX_I2S_GetOutputEnable( void );
void	NX_I2S_SetInputEnable( CBOOL Enable );
CBOOL	NX_I2S_GetInputEnable( void );
void	NX_I2S_SetLoopBackEnable( CBOOL Enable );
CBOOL	NX_I2S_GetLoopBackEnable( void );
//@}

//--------------------------------------------------------------------------
/// @name I2S Buffer Function
//--------------------------------------------------------------------------
//@{
CBOOL	NX_I2S_IsPCMInBufferReady( void );
CBOOL	NX_I2S_IsPCMOutBufferReady( void );
//@}
#endif
//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_I2S_H__

