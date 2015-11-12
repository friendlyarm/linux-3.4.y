/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : exchange.config.h
 * Date         : 2014.07.11
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.11 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#ifdef __EXCHANGE_CONFIG_GLOBAL__
#define EXCHANGE_CONFIG_EXT
#else
#define EXCHANGE_CONFIG_EXT extern
#endif

/******************************************************************************
 * SOC Support List
 *
 *  __SUPPORT_MIO_KERNEL__
 *  __SUPPORT_MIO_UBOOT__
 *
 ******************************************************************************/

#if defined (__SUPPORT_MIO_UBOOT__)

#if defined (NX_DEBUG)
#define __SUPPORT_MIO_DEBUG__
#elif defined (NX_RELEASE)
#define __SUPPORT_MIO_RELEASE__
#endif

#else

#define __SUPPORT_MIO_KERNEL__

#endif


/******************************************************************************
 * Register Width
 ******************************************************************************/
#if defined(__x86_64__) || defined(__ia64__) || defined(_M_AMD64) || defined(_M_IA64) || defined(_WIN64) || defined(__alpha__) || defined(__s390__) || \
    defined(CONFIG_ARM64) /* ... */
#define __SUPPORT_X64_OS__
#endif

#if defined (__SUPPORT_X64_OS__)
    #define __COMPILE_MODE_X64__
#endif

/******************************************************************************
 * FTL/NFC Mode List
 *
 *  __BUILD_MODE_X86_WINDOWS_SIMULATOR__
 *  __BUILD_MODE_X86_LINUX_DEVICE_DRIVER__
 *  __BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__
 *  __BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__
 *
 *  __COMPILE_MODE_FORMAT__
 *  __COMPILE_MODE_DEBUG__
 *  __COMPILE_MODE_RELEASE__
 *  __COMPILE_MODE_HALT_ERROR__     : Do Not Use Release Mode
 *
 ******************************************************************************/

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__SUPPORT_MIO_KERNEL__)

  //#define __BUILD_MODE_X86_WINDOWS_SIMULATOR__
  //#define __BUILD_MODE_X86_LINUX_DEVICE_DRIVER__
    #define __BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__
  //#define __BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__

  //#define __COMPILE_MODE_FORMAT__
    #define __COMPILE_MODE_HALT_ERROR__
  //#define __COMPILE_MODE_BEST_DEBUGGING__
    #define __COMPILE_MODE_ELAPSE_T__
  //#define __COMPILE_MODE_READONLY__

  //#define __MIO_UNIT_TEST_RANDOMIZER__
  //#define __MIO_UNIT_TEST_THREAD__
  //#define __MIO_UNIT_TEST_SLEEP__

	#ifndef CONFIG_LOCAL_TIMERS
    #define __USING_DELAY_FOR_SHORT_SLEEP__
	#endif

#elif defined (__SUPPORT_MIO_UBOOT__)

  //#define __BUILD_MODE_X86_WINDOWS_SIMULATOR__
  //#define __BUILD_MODE_X86_LINUX_DEVICE_DRIVER__
  //#define __BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__
    #define __BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__

    #define __COMPILE_MODE_FORMAT__
  //#define __COMPILE_MODE_HALT_ERROR__
  //#define __COMPILE_MODE_BEST_DEBUGGING__
  //#define __COMPILE_MODE_ELAPSE_T__
  //#define __COMPILE_MODE_RW_TEST__
  //#define __COMPILE_MODE_READONLY__

#endif

/******************************************************************************
 * Release / Debug
 ******************************************************************************/
#if defined (__SUPPORT_MIO_DEBUG__)
    #define __COMPILE_MODE_DEBUG__
  //#define __COMPILE_MODE_DEBUG_PRINT__
#else
    #define __COMPILE_MODE_RELEASE__
#endif
