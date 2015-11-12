/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.definition.h
 * Date         : 2014.07.11
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.11 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#include "media/exchange.config.h"

/******************************************************************************
 *
 ******************************************************************************/
#define __POW(B,P)              ((unsigned int)(B)<<(P))
#define __NROOT(B,R)            ((B)>>(R))

/******************************************************************************
 *
 ******************************************************************************/
#define __GB(X)                 __POW(X,30)
#define __MB(X)                 __POW(X,20)
#define __KB(X)                 __POW(X,10)
#define __B(X)                  __POW(X,0)

/******************************************************************************
 *
 ******************************************************************************/
#define __GHZ(X)                ((X)*1000000000)
#define __MHZ(X)                ((X)*1000000)
#define __KHZ(X)                ((X)*1000)
#define __HZ(X)                 ((X)*1)

/******************************************************************************
 *
 ******************************************************************************/
#define __SECTOR_SIZEOF(S)      __POW(S,9)      // Seccnt -> Byte
#define __SECTOR_OF_BYTE(B)     __NROOT(B,9)    // Byte -> Seccnt

/******************************************************************************
 *
 ******************************************************************************/
#define __TRUE  (1)
#define __FALSE (0)
#define __NULL  (0)

/******************************************************************************
 * Debug Print Options
 ******************************************************************************/

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#define __PRINT     printk

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#define __PRINT     printf

#endif

//#define __DBG_MIOSYS
//#define __DBG_MIOSMART

#ifndef __DBG_MIOSYS
#define DBG_MIOSYS(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_MIOSYS(fmt, args...)
#endif

#ifndef __DBG_MIOSMART
#define DBG_MIOSMART(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_MIOSMART(fmt, args...)
#endif

