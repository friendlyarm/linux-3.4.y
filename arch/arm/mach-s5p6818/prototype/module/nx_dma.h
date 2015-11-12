#ifndef __NX_DMA_H__
#define __NX_DMA_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	DMA DMA
//------------------------------------------------------------------------------
//@{
	#define NUMBER_OF_DMA_CHANNEL	8
	#define NX_DMA_NUM_OF_INT 2 // @todo ??

	/// @brief	DMA Module's Register List
	struct	NX_DMALLI_RegisterSet
	{
		volatile U32	SRCADDR;		///< 0x00 :
		volatile U32	DSTADDR;		///< 0x04 :
		volatile U32	LLI;			///< 0x08 :
		volatile U32	Control;		///< 0x0C :
	};

	struct	NX_DMAChannel_RegisterSet
	{
		struct NX_DMALLI_RegisterSet SGLLI;		///< 0x00 ~ 0x0C
		volatile U32	Configuration;				///< 0x10 :
		volatile U32	_Reserved[(0x20-0x14)/4];
	};

	struct NX_DMA_RegisterSet
	{
		volatile U32	IntStatus;			// 0x00
		volatile U32	IntTCStatus;		// 0x04
		volatile U32	IntTCClear;			// 0x08
		volatile U32	IntErrorStatus;		// 0x0C
		volatile U32	IntErrClr;			// 0x10
		volatile U32	RawIntTCStatus;		// 0x14
		volatile U32	RawIntErrorStatus;	// 0x18
		volatile U32	EnbldChns;			// 0x1C
		volatile U32	SoftBReq;			// 0x20
		volatile U32	SoftSReq;			// 0x24
		volatile U32	SoftLBReq;			// 0x28
		volatile U32	SoftLSReq;			// 0x2C
		volatile U32	Configuration;		// 0x30
		volatile U32	Sync;				// 0x34
		volatile U32	_Reserved0[(0x100-0x38)/4];
		struct NX_DMAChannel_RegisterSet Channel[NUMBER_OF_DMA_CHANNEL];
	};
   //-------------------------------------------------------------------
	typedef enum{
		NX_DMA_DATAWIDTH_8_BIT	= 0,
		NX_DMA_DATAWIDTH_16_BIT	= 1,
		NX_DMA_DATAWIDTH_32_BIT	= 2
	}NX_DMA_DATAWIDTH;

	typedef enum{
		NX_DMA_BURST_SIZE_1		= 0,
		NX_DMA_BURST_SIZE_4		= 1,
		NX_DMA_BURST_SIZE_8		= 2,
		NX_DMA_BURST_SIZE_16	= 3,
		NX_DMA_BURST_SIZE_32	= 4,
		NX_DMA_BURST_SIZE_64	= 5,
		NX_DMA_BURST_SIZE_128	= 6,
		NX_DMA_BURST_SIZE_256	= 7
	}NX_DMA_BURST_SIZE;


	/// @brief DMA interrupt for interrupt interface
	enum
	{
		NX_DMA_INT_TRANSFERDONE = 0		///< DMA Transfer Complete interrupt.
	};

	enum
    {
    	NX_DMA_INTCH_ERR  = 0, ///< Abort channel
    	NX_DMA_INTCH_DONE   = 1 ///< Done channel
	};

	/// @brief Indicate DMA module's maximum transfer size
	enum NX_DMA_TS
	{
		NX_DMA_MAX_TRANSFER_SIZE		= 0x1000UL	///< Max Transfer Size of DMA Module 4KB
	};


	/// @brief Use for DMA's Operation Mode
	typedef enum
	{
		NX_DMA_OPMODE_MEM_TO_MEM	= 0UL,			///< Memory to Memory operation
		NX_DMA_OPMODE_MEM_TO_IO	= 1UL,			///< Memory to Peripheral operation
		NX_DMA_OPMODE_IO_TO_MEM	= 2UL,			///< Peripheral to Memory operation
		NX_DMA_OPMODE_IO_TO_IO		= 3UL,			///< Peripheral to Peripheral operation
		NX_DMA_OPMODE_IO_TO_SIO	= 4UL,			///< Peripheral to Peripheral operation destination sync mode
		NX_DMA_OPMODE_MEM_TO_SIO	= 5UL,			///< Memory to Peripheral operation destination sync mode
		NX_DMA_OPMODE_SIO_TO_MEM	= 6UL,			///< Peripheral to Memory operation source sync mode
		NX_DMA_OPMODE_SIO_TO_IO	= 7UL			///< Peripheral to Peripheral operation source sync mode
	} NX_DMA_OPMODE ;

	//-------------------------------------------------------------------
	typedef struct NX_DMA_Infomration
	{
		U32* 				SrcAddr;
		U32*				DstAddr;
        // Channel Control
		U32					TxSize;
		NX_DMA_BURST_SIZE	SrcBurstSize;
		NX_DMA_BURST_SIZE	DstBurstSize;
		NX_DMA_DATAWIDTH	SrcWidth;
		NX_DMA_DATAWIDTH	DstWidth;
		CBOOL				SrcAHBSel;
		CBOOL				DstAHBSel;
		CBOOL				SrcAddrInc;		
		CBOOL				DstAddrInc; 	
        // Channel Configuration.
		U32					SrcPeriID;
		U32					DstPeriID;
		NX_DMA_OPMODE		FlowCtrl;
		CBOOL				IntITC;
		CBOOL				IntIE;
		CBOOL               fInfinite;
        // LLI Address
		struct NX_DMALLI_RegisterSet* HwLLI;
        
		U32 				Control;
		U32 				Configuration;

		U32					OffsetPerTx;
		U32					SizePerTx;
		U32					BoffSetPerTx;
	} NX_DMA_INFO;

	//-------------------------------------------------------------------

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{

CBOOL	NX_DMA_Initialize( void );
U32		NX_DMA_GetNumberOfModule( void );
U32		NX_DMA_GetNumberOfChannel( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{

U32		NX_DMA_GetPhysicalAddress( U32 ModuleIndex );
U32		NX_DMA_GetSizeOfRegisterSet( void );
void	NX_DMA_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_DMA_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_DMA_OpenModule( U32 ModuleIndex );
CBOOL	NX_DMA_CloseModule( U32 ModuleIndex );
CBOOL	NX_DMA_CheckBusy( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_DMA_GetInterruptNumber( U32 nChannel );

void	NX_DMA_SetInterruptEnable( U32 nChannel, U32 IntNum, CBOOL Enable );
CBOOL	NX_DMA_GetInterruptEnable( U32 nChannel, U32 IntNum );
CBOOL	NX_DMA_GetInterruptPending( U32 nChannel, U32 IntNum);
void	NX_DMA_ClearInterruptPending( U32 nChannel, U32 IntNum );

void	NX_DMA_SetInterruptEnableAll( CBOOL Enable );
CBOOL	NX_DMA_GetInterruptEnableAll( void );
CBOOL	NX_DMA_GetInterruptPendingAll( void );
void	NX_DMA_ClearInterruptPendingAll( void );

U32		NX_DMA_GetInterruptPendingNumber( U32 nChannel );	// -1 if None
//@}

//------------------------------------------------------------------------------
///	@name	Not Release(Prototype) DMA Configuration Function
//@{
void    NX_DMA_SetAttribute( U32 nChannel, NX_DMA_INFO *pInfoSet );
void* 	NX_DMA_Build_LLI( U32 nChannel, NX_DMA_INFO *pInfoSet );
void    NX_DMA_Transfer( U32 nChannel, NX_DMA_INFO *pInfoSet );
//@}

//------------------------------------------------------------------------------
///	@name	DMA Operation Function
//@{
void	NX_DMA_Run( U32 nChannel );
U32		NX_DMA_CheckRunning ( U32 nChannel );
void	NX_DMA_Stop ( U32 nChannel, CBOOL Enable );
//@}

//------------------------------------------------------------------------------
///	@name	DMA LLI Allocate Function ( Not-Cahe Area )
//@{
void    NX_DMA_CreateLLI( U32 start, U32 size );
void* 	NX_DMA_LLIuAlloc( U32 size, U32 alignment );
void    NX_DMA_LLIuFree( void *addr );
void    NX_DMA_SetLLIAddress( U32 LLIAddress, U32 LLISize );
U32     NX_DMA_GetLLIAddress( void );
void	NX_DMA_Stop ( U32 nChannel, CBOOL Enable );
//@}

//@}

#ifdef	__cplusplus
}
#endif

#endif // __NX_DMA_H__
