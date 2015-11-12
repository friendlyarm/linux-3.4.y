//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: base
//	File		: nx_debug.h
//	Description: support assert, trace routine
//	Author		: Goofy
//	Export		:
//	History	:
//		2010.04.27	Hans
//		2007.04.04	Goofy First draft
//------------------------------------------------------------------------------
#ifndef __NX_DEBUG_H__
#define	__NX_DEBUG_H__

//------------------------------------------------------------------------------
//	Platform check
//------------------------------------------------------------------------------
// Microsoft 32bits Windows
#if defined(WIN32) && !defined(UNDER_CE)
	#ifdef	_NX_PLATFORM_
		#error "WIN32 and __arm and UNDER_CE predefine must use exclusively!!!"
	#endif

	#define _NX_PLATFORM_

	// Debug break definition, default is noexist
	#ifdef NX_DEBUG
		#define	NX_DEBUG_BREAK()	{ __asm int 3 }
	#endif
#endif	// WIN32 && !UNDER_CE

//------------------------------------------------------------------------------
// ARM compiler
#ifdef __arm
	#ifdef	_NX_PLATFORM_
		#error "WIN32 and __arm and UNDER_CE predefine must use exclusively!!!"
	#endif

	#define _NX_PLATFORM_

	// Debug break definition, default is noexist
	#ifdef NX_DEBUG
	#define	NX_DEBUG_BREAK()	{ static volatile int i = 1; while(i); }
	#endif
#endif	// __arm

//------------------------------------------------------------------------------
// Microsoft Window CE
#ifdef UNDER_CE
	#ifdef	_NX_PLATFORM_
		#error "WIN32 and __arm and UNDER_CE predefine must use exclusively!!!"
	#endif

	#define _NX_PLATFORM_

	#include <windows.h>

	// Debug break definition, default is noexist
	#ifdef NX_DEBUG
		#define	NX_DEBUG_BREAK	DebugBreak
	#endif
#endif	// UNDER_CE

//------------------------------------------------------------------------------
// LINUX / GNU C Compiler
#if defined(__GNUC__) && defined(__LINUX__)
	#ifdef	_NX_PLATFORM_
		#error "__LINUX__ and __arm and __GNUC__ predefine must use exclusively!!!"
	#endif

	#define _NX_PLATFORM_

	// Debug break definition, default is noexist
	#ifdef NX_DEBUG
	#define	NX_DEBUG_BREAK() 	{ static volatile int i = 1; while(i); }
	#endif
#endif	// LINUX / GNU C Compiler

//------------------------------------------------------------------------------
// SOC simulation platform
#ifdef __SOC__

//@added choiyk 2013-01-16 오후 3:53:35
#include<nx_simio.h>

	#ifdef	_NX_PLATFORM_
		#error "WIN32 and __arm and UNDER_CE predefine must use exclusively!!!"
	#endif

	#define _NX_PLATFORM_

	// Debug break definition, default is noexist
	#ifdef NX_DEBUG

	// 다시 무한대기로 변경..
	//#define	NX_DEBUG_BREAK()	{ NX_SIMIO_Exit(); }
	#define	NX_DEBUG_BREAK()	{ static volatile int i = 1; while(i); }

	#endif
#endif	// __SOC__

#ifndef _NX_PLATFORM_
	#error "Unknown platform"
#endif

//------------------------------------------------------------------------------
//	compile mode check
//------------------------------------------------------------------------------
#ifdef NX_DEBUG
	#ifdef _NX_COMPILE_MODE_
		#error "NX_DEBUG and NX_RELEASE predefine must use exclusively!!!"
	#endif

	#define _NX_COMPILE_MODE_
#endif

#ifdef NX_RELEASE
	#ifdef _NX_COMPILE_MODE_
		#error "NX_DEBUG and NX_RELEASE predefine must use exclusively!!!"
	#endif

	#define _NX_COMPILE_MODE_
#endif

#ifndef _NX_COMPILE_MODE_
	#error "You must have pre-definition ( NX_DEBUG or NX_RELEASE )!!!"
#endif


//------------------------------------------------------------------------------
///	@name	Trace macro
/// @brief	NX_TRACE print formated string through user print function.
//------------------------------------------------------------------------------
/// @{
#ifdef NX_DEBUG
	#ifdef UNDER_CE
		#define	NX_TRACE(expr)		RETAILMSG( 1, expr )

	#elif __LINUX__
		#define TEXT(str)			str
		#define NX_TRACE(expr)		NX_DBG_TRACE	expr

		#ifdef __PRINTK__
			#include <linux/kernel.h>
			#define NX_DBG_TRACE(expr...)	printk(KERN_INFO expr)
		#else
			#ifdef USE_HOSTCC
			#include <stdio.h>
			#else
			#ifndef printf
			extern int printf(const char *fmt, ...);
			#endif
			#endif

			#define NX_DBG_TRACE(expr...)	printf(expr)
		#endif
	#else

		//----------------------------------------------------------------------
		///	@brief Extern function for print formated string
		/// You have to implement NX_DEBUG_PRINTF function for NX_TRACE.
		//----------------------------------------------------------------------
		#ifdef	__cplusplus
			extern "C" void NX_DEBUG_PRINTF(const char * const, ... );
		#else
			extern void NX_DEBUG_PRINTF(const char * const, ... );
		#endif

		#define NX_TRACE(expr)		NX_DEBUG_PRINTF expr
	#endif
#else
	#define NX_TRACE(expr)
#endif // NX_DEBUG
/// @}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
///	@name	Assert macro
/// @brief	Assert macro is composed by NX_ASSERT NX_CHECK NX_DEBUG_CODE.
//------------------------------------------------------------------------------
/// @{

#ifdef NX_DEBUG
	#ifndef NX_DEBUG_BREAK
		#error "there is no debug break !!!"
	#endif
#endif

#ifndef	TEXT
	#define TEXT
#endif

//------------------------------------------------------------------------------
/// NX_ASSERT
/// @brief	runtime assert in normal case
/// @code
///	int UnKnownFunction(int a, int b) {
///
///		int Result;
///	...
///
///		for(int i = 0; i < 10; i++) {
///			NX_ASSERT(i < MAXNUM_OF_NKNOWNARRY);
///			UnknownArray[i] = RandomNumberGen();
///		}
///
///		...
/// }
/// @endcode
//
//------------------------------------------------------------------------------
#ifdef NX_DEBUG
	#define NX_ASSERT(expr)																\
	{																					\
		if (!(expr))																	\
		{																				\
			NX_TRACE((TEXT("%s(%d) : %s %s (%s)\n"),									\
				TEXT(__FILE__), __LINE__, __FUNCTION__, TEXT("ASSERT"), TEXT(#expr)));	\
			NX_DEBUG_BREAK();															\
		}																				\
	}
#else
	#define NX_ASSERT(expr)
#endif // NX_DEBUG


//------------------------------------------------------------------------------
/// NX_CHECK
/// @brief	Note that this assertion preserves the expression when disabled.
///
/// @code
///		NX_CHECK((fp = fopen("file", "rb+")) != CNULL); // don't occur
///															// compile time error
///															// int NX_RELEASE
///		...
///		fclose(fp);
/// @endcode
//------------------------------------------------------------------------------
#ifdef NX_DEBUG
	#define NX_CHECK(expr)																\
	{																					\
		if (!(expr))																	\
		{																				\
			NX_TRACE((TEXT("%s(%d) : %s %s (%s)\n"),									\
				TEXT(__FILE__), __LINE__, __FUNCTION__, TEXT("CHECK"),TEXT(#expr))); 	\
			NX_DEBUG_BREAK();															\
		}																				\
	}
#else
	#define NX_CHECK(expr)													\
	{																		\
		expr ;																\
	}
#endif // NX_DEBUG


//------------------------------------------------------------------------------
///	NX_DEBUG_CODE
/// @brief	for debug code or debug varible declare
///
/// @code
/// {
///	NX_DEBUG_CODE(int iDebugCounter = 0);
///		...
///		NX_DEBUG_CODE(iDebugCount++);
///		...
///		NX_DEBUG_CODE(
///			if(iDebugCount == x)
///				NX_TRACE("... found!!!\n");
///		)
/// }
/// @endcode
//------------------------------------------------------------------------------
#ifdef NX_DEBUG
	#define NX_DEBUG_CODE(code) code
#else
	#define NX_DEBUG_CODE(code)
#endif // NX_DEBUG
/// @}
//------------------------------------------------------------------------------

#define NX_DISABLE_UNUSED_VAR_WARNING(X)	((void)(&(X)))

#endif	//	__NX_DEBUG_H__

