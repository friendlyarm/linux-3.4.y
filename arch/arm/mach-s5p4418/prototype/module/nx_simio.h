//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module     : rtl simulator i/o
//	File       : mes_simio00.h
//	Description:
//	Author     : Gamza(nick@nexell.co.kr)
//	Export     :
//	History    :
//     2012-06-25 Gamza convert to C prototype
//------------------------------------------------------------------------------
//	Customizing parameters
//		ENV_SIMULATION : RTL 시뮬레이터와 통신을 한다.
//						 이것이 정의되지 않으면 CPU로 자체처리하게 됨.
//------------------------------------------------------------------------------
#ifndef __NX_SIMIO_H__
#define __NX_SIMIO_H__

//------------------------------------------------------------------------------
//	includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
//	SIMIO00 register set structure
//------------------------------------------------------------------------------
struct NX_SIMIO_RegisterSet // 64 bytes
{
	volatile U32 TIMEOUTVALUE ;
	volatile U32 COMMAND      ;
	volatile U32 RESULT       ;
	volatile U32 PARAM[16-3]  ;
	volatile U32 STANDBYWFI   ; // 64
};

// @modified matin 2008/01/28
enum COMMAND
{
	COMMAND_DEBUGPRINT		= 0,
	COMMAND_LOADHEXFILE		= 1,
	COMMAND_SAVEHEXFILE		= 2,
	COMMAND_COMPAREMEMORY	= 3,
	COMMAND_DEBUGBREAK		= 4,
	COMMAND_EXIT			= 5,
	COMMAND_LOADIMGFILE		= 6,
	COMMAND_SAVEIMGFILE		= 7,
	COMMAND_COMPAREIMAGE	= 8,
	COMMAND_QUERYPERFORMANCEFREQUENCY =  9,
	COMMAND_QUERYPERFORMANCECOUNTER   = 10,
	COMMAND_LOADTEXTHEXFILE		= 11,
	COMMAND_SAVETEXTHEXFILE		= 12,
	COMMAND_COMPARETEXTMEMORY	= 13,
	COMMAND_LOADTEXTIMGFILE		= 14,
	COMMAND_SAVETEXTIMGFILE		= 15,
	COMMAND_COMPARETEXTIMAGE	= 16,
	
	COMMAND_SAVETEXTIMGFILE_EX	= 17,
	COMMAND_COMPARETEXTIMAGE_EX	= 18,
	COMMAND_LOADTEXTIMGFILE_EX  = 19,
	
	COMMAND_FILLMEMORY  = 20,
	
	COMMAND_TD_READ = 21,
	COMMAND_TD_WRITE = 22,
	
	// @added yoan 2012/07/23
	COMMAND_LOADCODECBITCODE							= 32,
	COMMAND_LOADCODECYUV								= 33,	
	COMMAND_LOADCODECSTREAM								= 34,	
	COMMAND_SAVECODECYUV								= 35,
	COMMAND_SAVECODECSTREAM								= 36,
	COMMAND_MEMCOPY										= 37,	
	COMMAND_CMPBINFILE									= 38,	

	COMMAND_LOADIMGFILE_A8 = 40,
	
	COMMAND_EOS = 41,

	COMMAND_FORCEU32		= 0x7FFFFFFF
};

/// @brief	Image data layout.
typedef enum
{
	NX_IMAGELAYOUT_LINEAR = 0UL,	///< linear mode
	NX_IMAGELAYOUT_VR     ,		    ///< swizzled image for VR
	NX_IMAGELAYOUT_MALI   ,		    ///< swizzled image for VR
} NX_IMAGELAYOUT ;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_SIMIO_Initialize( void );
//@}


//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_SIMIO_GetPhysicalAddress( void );
U32		NX_SIMIO_GetSizeOfRegisterSet( void );
void	NX_SIMIO_SetBaseAddress( void* BaseAddress );
void*	NX_SIMIO_GetBaseAddress( void );
CBOOL	NX_SIMIO_OpenModule( void );
CBOOL	NX_SIMIO_CloseModule( void );
CBOOL	NX_SIMIO_CheckBusy( void );
CBOOL	NX_SIMIO_CanPowerDown( void );
//@}

//--------------------------------------------------------------------------
///	@name	Service
//--------------------------------------------------------------------------
//@{
void NX_SIMIO_PushParameters( void );
void NX_SIMIO_PopParameters( void );

void NX_SIMIO_PutString( const char* const pString );

void  NX_SIMIO_SetTimeOutValue( U32 KClockValue );
void  NX_SIMIO_DebugBreak( void );
void  NX_SIMIO_Exit( void );
CBOOL NX_SIMIO_EOS( void );

CBOOL NX_SIMIO_QueryPerformanceFrequency( U32* piFrequency );
CBOOL NX_SIMIO_QueryPerformanceCounter( U32* piCounter );

CBOOL NX_SIMIO_FillMemory( void* const pMemory, U8 Data, U32 ByteSize );

U32   NX_SIMIO_LoadHexFile( const char* const FileName, void* const pMemory );
CBOOL NX_SIMIO_LoadImgFile( const char* const FileName, U32 Stride, U32* pWidth, U32* pHeight, U32* pBPP, void* const pMemory );
CBOOL NX_SIMIO_LoadImgFileEx( const char* const FileName, NX_IMAGELAYOUT Layout, U32 SwizzleSize, U32 Stride, U32* pWidth, U32* pHeight, U32* pBPP, void* const pMemory );
    
CBOOL NX_SIMIO_SaveHexFile( const char* const FileName, void* const pMemory, U32 ByteSize );
CBOOL NX_SIMIO_SaveImgFile( const char* const FileName, U32 Stride, U32  Width, U32  Height, U32 BPP, void* const pMemory );
CBOOL NX_SIMIO_SaveImgFileEx( const char* const FileName, NX_IMAGELAYOUT Layout, U32 SwizzleSize, U32 Stride, U32  Width, U32  Height, U32 BPP, void* const pMemory );

CBOOL NX_SIMIO_CompareMemory( const void* const pMemory0, const void* const pMemory1, U32 ByteSize );
CBOOL NX_SIMIO_CompareImage( const void* Source0, U32 Source0Stride, const void* Source1, U32 Source1Stride, U32 Width, U32 Height, U32 BPP );
//@}
//--------------------------------------------------------------------------
//  @added yoan 2012-07-23
//--------------------------------------------------------------------------
//FILEPTR NX_SIMIO_Fopen( const char* const FileName);
//CBOOL NX_SIMIO_Fclose( FILEPTR fp);
U32 	NX_SIMIO_LoadCodecBitCode	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
U32 	NX_SIMIO_LoadCodecYUV	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
U32 	NX_SIMIO_LoadCodecStream	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
CBOOL NX_SIMIO_SaveCodecYUV	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
CBOOL NX_SIMIO_SaveCodecStream	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
CBOOL NX_SIMIO_MEMCOPY	( const void* const pMemory0, const void* const pMemory1, U32 ByteSize );
CBOOL NX_SIMIO_CMPBINFILE	( const char* const FileName0, const char* const FileName1, U32 ByteSize );




//--------------------------------------------------------------------------
//  @deprecated Gamza 2012-06-27
//--------------------------------------------------------------------------
//@{
// @modified matin 05/05/28
CBOOL NX_SIMIO_LoadImgFileBin( const char* const FileName, U32 Stride,
					U32* pWidth, U32* pHeight, U32* pBPP,
					void* const pMemory );
					
// @modified matin 2008/01/28 text array function add
U32   NX_SIMIO_LoadTextHexFile( const char* const FileName, void* const pMemory );
CBOOL NX_SIMIO_SaveTextHexFile( const char* const FileName, void* const pMemory,
				   U32 ByteSize );
CBOOL NX_SIMIO_CompareTextMemory( const void* const pMemory0,
					  const void* const pMemory1,
					  U32 ByteSize );
CBOOL NX_SIMIO_LoadTextImgFile( const char* const FileName, U32 Stride,
					U32* pWidth, U32* pHeight, U32* pBPP,
					void* const pMemory );
CBOOL NX_SIMIO_SaveTextImgFile( const char* const FileName, U32 Stride,
					U32  Width, U32  Height, U32 BPP,
					void* const pMemory );
CBOOL NX_SIMIO_CompareTextImage( const void* Source0, U32 Source0Stride,
					const void* Source1, U32 Source1Stride,
					U32 Width, U32 Height, U32 BPP );
CBOOL NX_SIMIO_SaveTextImgFileEx( const char* const FileName, U32 Stride,
					U32  Width, U32  Height, U32 BPP,
					void* const pMemory,
					U32 Log2BlockSize );
CBOOL NX_SIMIO_CompareTextImageEx( const void* Source0, U32 Source0Stride, U32 Log2BlockSize0,
					      const void* Source1, U32 Source1Stride, U32 Log2BlockSize1,
					      U32 Width, U32 Height, U32 BPP);			
//@}

//@choiyk
U32 NX_SIMIO_TD_READ ( U32  Addr );
CBOOL NX_SIMIO_TD_WRITE( U32  Addr, U32 Data );

//@modified choiyk : Alpha 8bit 지정 가능한 저장 포맷.
CBOOL NX_SIMIO_LoadImgFile_A8( 	const char* const FileName, U32 Stride, 
								U32* pWidth, U32* pHeight, U32* pBPP, 
								void* const pMemory, U32 Alpha);

void NX_SIMIO_SetWFI ( U32 WFI ) ;

#ifdef	__cplusplus
}
#endif

#endif // __NX_SIMIO_H__
