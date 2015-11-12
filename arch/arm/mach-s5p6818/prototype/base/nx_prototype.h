//------------------------------------------------------------------------------
//
//  Copyright (C) 2009 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : base
//  File        : nx_prototype.h
//  Description : include header files for base.
//  Author      : Goofy
//  Export      :
//  History     :
//      2012-07-25  Tony     append PHY_BASEADDR_LIST_ALPHA( x ) : alphabetic version.
//      2012-06-27  Gamza   append PHY_BASEADDR_LIST( x )
//      2010.10.05  Hans    add global io access function
//      2010.04.27  Hans
//      2007.04.04  Goofy   First draft
//------------------------------------------------------------------------------
#ifndef __NX_PROTOTYPE_H__
#define __NX_PROTOTYPE_H__

#include "nx_type.h"
#include "nx_debug.h"
#include "nx_chip.h"
#include "nx_clockcontrol.h"
#include "nx_bit_accessor.h"

#define ReadIO32(Addr)          (*(volatile unsigned int  *)(Addr))
#define ReadIO16(Addr)          (*(volatile unsigned short*)(Addr))
#define ReadIO8(Addr)           (*(volatile unsigned char *)(Addr))

#define WriteIO32(Addr,Data)    (*(volatile unsigned int  *)(Addr))  =  ((unsigned int  )(Data))
#define WriteIO16(Addr,Data)    (*(volatile unsigned short*)(Addr))  =  ((unsigned short)(Data))
#define WriteIO8(Addr,Data)     (*(volatile unsigned char *)(Addr))  =  ((unsigned char )(Data))

#define SetIO32(Addr,Data)      (*(volatile unsigned int  *)(Addr)) |=  ((unsigned int  )(Data))
#define SetIO16(Addr,Data)      (*(volatile unsigned short*)(Addr)) |=  ((unsigned short)(Data))
#define SetIO8(Addr,Data)       (*(volatile unsigned char *)(Addr)) |=  ((unsigned char )(Data))

#define ClearIO32(Addr,Data)    (*(volatile unsigned int  *)(Addr)) &= ~((unsigned int  )(Data))
#define ClearIO16(Addr,Data)    (*(volatile unsigned short*)(Addr)) &= ~((unsigned short)(Data))
#define ClearIO8(Addr,Data)     (*(volatile unsigned char *)(Addr)) &= ~((unsigned char )(Data))

//@choiyk : 2012/07/20 :
#define Devmodel_ReadIO32(Addr)         NX_SIMIO_TD_READ ((volatile unsigned int*)(Addr))
#define Devmodel_WriteIO32(Addr,Data)   NX_SIMIO_TD_WRITE((volatile unsigned int*)(Addr), (unsigned int)(Data))

//------------------------------------------------------------------------------
//  NX_????_GetPhysicalAddress
//------------------------------------------------------------------------------
//  (*) 칩내에 인스턴스가 한개이상 만들어지지 않는다고 확신되는 모듈
//  U32  NX_INTC_GetPhysicalAddress( void )
//  {
//      const U32 PhysicalAddr[] =  {   PHY_BASEADDR_LIST( INTC )  }; // PHY_BASEADDR_INTC_MODULE
//      NX_CASSERT( 1 == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
//      NX_ASSERT( PHY_BASEADDR_INTC_MODULE == PhysicalAddr[0] );
//      return PhysicalAddr[0];
//  }
//  (*) 칩내에 인스턴스가 다수개 만들어질 지도 모르는 모듈
//  U32 NX_UART_GetPhysicalAddress( U32 ModuleIndex )
//  {
//      static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( UART ) }; // PHY_BASEADDR_UART?_MODULE
//      NX_CASSERT( NUMBER_UART_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
//      NX_ASSERT( NUMBER_UART_MODULE > ModuleIndex );
//      // NX_ASSERT( PHY_BASEADDR_UART0_MODULE == PhysicalAddr[0] );
//      // NX_ASSERT( PHY_BASEADDR_UART1_MODULE == PhysicalAddr[1] );
//      // ...
//      return (U32)PhysicalAddr[ModuleIndex];
//  }
//
//------------------------------------------------------------------------------
//  NX_????_GetInterruptNumber
//------------------------------------------------------------------------------
//  (*) 칩내에 인스턴스가 한개이상 만들어지지 않고, 모듈이 하나의 인터럽트 채널을 갖는 경우.
//  U32  NX_MALI_GetInterruptNumber( void )
//  {
//      const U32 InterruptNumber[] =  {   INTNUM_LIST( MALI )  }; // INTNUM_OF_MALI_MODULE
//      NX_CASSERT( 1 == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
//      NX_ASSERT( INTNUM_OF_MALI_MODULE == InterruptNumber[0] );
//      return InterruptNumber[0];
//  }
//
//  (*) 칩내에 인스턴스가 다수개 만들어질 지도 모르고, 모듈이 하나의 인터럽트 채널을 갖는 경우.
//  U32  NX_UART_GetInterruptNumber( U32 ModuleIndex )
//  {
//      const U32 InterruptNumber[] =  {   INTNUM_LIST( UART )  };  // INTNUM_OF_UART?_MODULE
//      NX_CASSERT( NUMBER_UART_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
//      NX_ASSERT( NUMBER_UART_MODULE > ModuleIndex );
//      // NX_ASSERT( INTNUM_OF_UART0_MODULE == InterruptNumber[0] );
//      // NX_ASSERT( INTNUM_OF_UART1_MODULE == InterruptNumber[1] );
//      // ...
//      return InterruptNumber[ModuleIndex];
//  }
//
//  (*) 칩내에 인스턴스가 다수개 만들어질 지도 모르고, 모듈이 다수개의 인터럽트 채널을 갖는 경우.
//  /// @brief  CMDPROC interrupt channel number
//  /// in "nx_cmdproc.h"
//  enum
//  {
//      NX_CMDPROC_INTCH_DONE   = 0, ///< Done  channel
//      NX_CMDPROC_INTCH_ABORT  = 1, ///< Abort channel
//  };
//  /// in "nx_cmdproc.cpp"
//  U32  NX_CMDPROC_GetInterruptNumber( U32 ModuleIndex, U32 ChannelIndex )
//  {
//      const U32 InterruptNumber[2][] =
//      {
//          { INTNUM_WITH_CHANNEL_LIST( CMDPROC, DONE  ) }, // INTNUM_OF_CMDPROC?_MODULE_DONE
//          { INTNUM_WITH_CHANNEL_LIST( CMDPROC, ABORT ) }, // INTNUM_OF_CMDPROC?_MODULE_ABORT
//      };
//      NX_CASSERT( NUMBER_CMDPROC_MODULE == (sizeof(InterruptNumber[0])/sizeof(InterruptNumber[0][0])) );
//      NX_ASSERT( NUMBER_CMDPROC_MODULE > ModuleIndex );
//      NX_ASSERT( (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) > ChannelIndex );
//      // NX_ASSERT( INTNUM_OF_CMDPROC0_MODULE_DONE == InterruptNumber[0][0] );
//      // NX_ASSERT( INTNUM_OF_CMDPROC1_MODULE_DONE == InterruptNumber[0][1] );
//      // ...
//      // NX_ASSERT( INTNUM_OF_CMDPROC0_MODULE_ABORT == InterruptNumber[1][0] );
//      // NX_ASSERT( INTNUM_OF_CMDPROC1_MODULE_ABORT == InterruptNumber[1][1] );
//      // ...
//      return InterruptNumber[ChannelIndex][ModuleIndex];
//  }
//
//------------------------------------------------------------------------------
//  NX_????_GetDMANumber
//------------------------------------------------------------------------------
//  (*) 칩내에 인스턴스가 한개이상 만들어지지 않고, 모듈이 하나의 DMA 채널을 갖는 경우.
//  U32  NX_MALI_GetDMANumber( void )
//  {
//      const U32 DMANumber[] =  {   DMAINDEX_LIST( MALI )  }; // DMAINDEX_OF_MALI_MODULE
//      NX_CASSERT( 1 == (sizeof(DMANumber)/sizeof(DMANumber[0])) );
//      NX_ASSERT( DMAINDEX_OF_MALI_MODULE == DMANumber[0] );
//      return DMANumber[0];
//  }
//
//  (*) 칩내에 인스턴스가 다수개 만들어질 지도 모르고, 모듈이 하나의 DMA 채널을 갖는 경우.
//  U32  NX_SDHC_GetDMANumber( U32 ModuleIndex )
//  {
//      const U32 DMANumber[] =  {   DMAINDEX_LIST( SDHC )  };  //DMAINDEX_OF_SDHC?_MODULE
//      NX_CASSERT( NUMBER_SDHC_MODULE == (sizeof(DMANumber)/sizeof(DMANumber[0])) );
//      NX_ASSERT( NUMBER_SDHC_MODULE > ModuleIndex );
//      // NX_ASSERT( DMAINDEX_OF_SDHC0_MODULE == DMANumber[0] );
//      // NX_ASSERT( DMAINDEX_OF_SDHC1_MODULE == DMANumber[1] );
//      // ...
//      return DMANumber[ModuleIndex];
//  }
//
//  (*) 칩내에 인스턴스가 다수개 만들어질 지도 모르고, 모듈이 다수개의 DMA 채널을 갖는 경우.
//  /// @brief  UART DMA channel number
//  /// in "nx_uart.h"
//  enum
//  {
//      NX_UART_DMACH_TX = 0, ///< Tx channel
//      NX_UART_DMACH_RX = 1, ///< Rx channel
//  };
//  /// in "nx_uart.cpp"
//  U32  NX_UART_GetDMANumber( U32 ModuleIndex, U32 ChannelIndex )
//  {
//      const U32 DMANumber[2][] =
//      {
//          { DMAINDEX_WITH_CHANNEL_LIST( UART, TX ) }, // DMAINDEX_OF_UART?_MODULE_TX
//          { DMAINDEX_WITH_CHANNEL_LIST( UART, RX ) }, // DMAINDEX_OF_UART?_MODULE_RX
//      };
//      NX_CASSERT( NUMBER_UART_MODULE == (sizeof(DMANumber[0])/sizeof(DMANumber[0][0])) );
//      NX_ASSERT( NUMBER_UART_MODULE > ModuleIndex );
//      NX_ASSERT( (sizeof(DMANumber)/sizeof(DMANumber[0])) > ChannelIndex );
//      // NX_ASSERT( DMAINDEX_OF_UART0_MODULE_TX == DMANumber[0][0] );
//      // NX_ASSERT( DMAINDEX_OF_UART1_MODULE_TX == DMANumber[0][1] );
//      // ...
//      // NX_ASSERT( DMAINDEX_OF_UART0_MODULE_RX == DMANumber[0][0] );
//      // NX_ASSERT( DMAINDEX_OF_UART1_MODULE_RX == DMANumber[0][1] );
//      // ...
//      return DMANumber[ChannelIndex][ModuleIndex];
//  }
//
//------------------------------------------------------------------------------
#define _GET_MACRO_LIST_1(NAME,PRE,POST)    PRE ## NAME ## POST
#define _GET_MACRO_LIST_2(NAME,PRE,POST)    PRE ## NAME ## 0 ## POST , PRE ## NAME ## 1 ## POST
#define _GET_MACRO_LIST_3(NAME,PRE,POST)    _GET_MACRO_LIST_2(NAME,PRE,POST) , PRE ## NAME ## 2 ## POST
#define _GET_MACRO_LIST_4(NAME,PRE,POST)    _GET_MACRO_LIST_3(NAME,PRE,POST) , PRE ## NAME ## 3 ## POST
#define _GET_MACRO_LIST_5(NAME,PRE,POST)    _GET_MACRO_LIST_4(NAME,PRE,POST) , PRE ## NAME ## 4 ## POST
#define _GET_MACRO_LIST_6(NAME,PRE,POST)    _GET_MACRO_LIST_5(NAME,PRE,POST) , PRE ## NAME ## 5 ## POST
#define _GET_MACRO_LIST_7(NAME,PRE,POST)    _GET_MACRO_LIST_6(NAME,PRE,POST) , PRE ## NAME ## 6 ## POST
#define _GET_MACRO_LIST_8(NAME,PRE,POST)    _GET_MACRO_LIST_7(NAME,PRE,POST) , PRE ## NAME ## 7 ## POST
#define _GET_MACRO_LIST_9(NAME,PRE,POST)    _GET_MACRO_LIST_8(NAME,PRE,POST) , PRE ## NAME ## 8 ## POST
#define _GET_MACRO_LIST_10(NAME,PRE,POST)   _GET_MACRO_LIST_9(NAME,PRE,POST) , PRE ## NAME ## 9 ## POST
#define _GET_MACRO_LIST_11(NAME,PRE,POST)   _GET_MACRO_LIST_10(NAME,PRE,POST), PRE ## NAME ## 10 ## POST
#define _GET_MACRO_LIST_12(NAME,PRE,POST)   _GET_MACRO_LIST_11(NAME,PRE,POST), PRE ## NAME ## 11 ## POST
#define _GET_MACRO_LIST_13(NAME,PRE,POST)   _GET_MACRO_LIST_12(NAME,PRE,POST), PRE ## NAME ## 12 ## POST
#define _GET_MACRO_LIST_14(NAME,PRE,POST)   _GET_MACRO_LIST_13(NAME,PRE,POST), PRE ## NAME ## 13 ## POST
#define _GET_MACRO_LIST_15(NAME,PRE,POST)   _GET_MACRO_LIST_14(NAME,PRE,POST), PRE ## NAME ## 14 ## POST
#define _GET_MACRO_LIST_16(NAME,PRE,POST)   _GET_MACRO_LIST_15(NAME,PRE,POST), PRE ## NAME ## 15 ## POST
#define _GET_MACRO_LIST_17(NAME,PRE,POST)   _GET_MACRO_LIST_16(NAME,PRE,POST), PRE ## NAME ## 16 ## POST
#define _GET_MACRO_LIST_18(NAME,PRE,POST)   _GET_MACRO_LIST_17(NAME,PRE,POST), PRE ## NAME ## 17 ## POST
#define _GET_MACRO_LIST_19(NAME,PRE,POST)   _GET_MACRO_LIST_18(NAME,PRE,POST), PRE ## NAME ## 18 ## POST
#define _GET_MACRO_LIST_20(NAME,PRE,POST)   _GET_MACRO_LIST_19(NAME,PRE,POST), PRE ## NAME ## 19 ## POST
#define _GET_MACRO_LIST_21(NAME,PRE,POST)   _GET_MACRO_LIST_20(NAME,PRE,POST), PRE ## NAME ## 20 ## POST
#define _GET_MACRO_LIST_22(NAME,PRE,POST)   _GET_MACRO_LIST_21(NAME,PRE,POST), PRE ## NAME ## 21 ## POST
#define _GET_MACRO_LIST_23(NAME,PRE,POST)   _GET_MACRO_LIST_22(NAME,PRE,POST), PRE ## NAME ## 22 ## POST
#define _GET_MACRO_LIST_24(NAME,PRE,POST)   _GET_MACRO_LIST_23(NAME,PRE,POST), PRE ## NAME ## 23 ## POST
#define _GET_MACRO_LIST_25(NAME,PRE,POST)   _GET_MACRO_LIST_24(NAME,PRE,POST), PRE ## NAME ## 24 ## POST
#define _GET_MACRO_LIST_26(NAME,PRE,POST)   _GET_MACRO_LIST_25(NAME,PRE,POST), PRE ## NAME ## 25 ## POST
#define _GET_MACRO_LIST_27(NAME,PRE,POST)   _GET_MACRO_LIST_26(NAME,PRE,POST), PRE ## NAME ## 26 ## POST
#define _GET_MACRO_LIST_28(NAME,PRE,POST)   _GET_MACRO_LIST_27(NAME,PRE,POST), PRE ## NAME ## 27 ## POST
#define _GET_MACRO_LIST_29(NAME,PRE,POST)   _GET_MACRO_LIST_28(NAME,PRE,POST), PRE ## NAME ## 28 ## POST
#define _GET_MACRO_LIST_30(NAME,PRE,POST)   _GET_MACRO_LIST_29(NAME,PRE,POST), PRE ## NAME ## 29 ## POST
#define _GET_MACRO_LIST_31(NAME,PRE,POST)   _GET_MACRO_LIST_30(NAME,PRE,POST), PRE ## NAME ## 30 ## POST
#define _GET_MACRO_LIST_32(NAME,PRE,POST)   _GET_MACRO_LIST_31(NAME,PRE,POST), PRE ## NAME ## 31 ## POST
#define _GET_MACRO_LIST_33(NAME,PRE,POST)   _GET_MACRO_LIST_32(NAME,PRE,POST), PRE ## NAME ## 32 ## POST
#define _GET_MACRO_LIST_34(NAME,PRE,POST)   _GET_MACRO_LIST_33(NAME,PRE,POST), PRE ## NAME ## 33 ## POST
#define _GET_MACRO_LIST_35(NAME,PRE,POST)   _GET_MACRO_LIST_34(NAME,PRE,POST), PRE ## NAME ## 34 ## POST
#define _GET_MACRO_LIST_36(NAME,PRE,POST)   _GET_MACRO_LIST_35(NAME,PRE,POST), PRE ## NAME ## 35 ## POST
#define _GET_MACRO_LIST_37(NAME,PRE,POST)   _GET_MACRO_LIST_36(NAME,PRE,POST), PRE ## NAME ## 36 ## POST
#define _GET_MACRO_LIST_38(NAME,PRE,POST)   _GET_MACRO_LIST_37(NAME,PRE,POST), PRE ## NAME ## 37 ## POST
#define _GET_MACRO_LIST_39(NAME,PRE,POST)   _GET_MACRO_LIST_38(NAME,PRE,POST), PRE ## NAME ## 38 ## POST
#define _GET_MACRO_LIST_40(NAME,PRE,POST)   _GET_MACRO_LIST_39(NAME,PRE,POST), PRE ## NAME ## 39 ## POST
#define _GET_MACRO_LIST_41(NAME,PRE,POST)   _GET_MACRO_LIST_40(NAME,PRE,POST), PRE ## NAME ## 40 ## POST
#define _GET_MACRO_LIST_42(NAME,PRE,POST)   _GET_MACRO_LIST_41(NAME,PRE,POST), PRE ## NAME ## 41 ## POST
#define _GET_MACRO_LIST_43(NAME,PRE,POST)   _GET_MACRO_LIST_42(NAME,PRE,POST), PRE ## NAME ## 42 ## POST
#define _GET_MACRO_LIST_44(NAME,PRE,POST)   _GET_MACRO_LIST_43(NAME,PRE,POST), PRE ## NAME ## 43 ## POST
#define _GET_MACRO_LIST_45(NAME,PRE,POST)   _GET_MACRO_LIST_44(NAME,PRE,POST), PRE ## NAME ## 44 ## POST
#define _GET_MACRO_LIST_46(NAME,PRE,POST)   _GET_MACRO_LIST_45(NAME,PRE,POST), PRE ## NAME ## 45 ## POST
#define _GET_MACRO_LIST_47(NAME,PRE,POST)   _GET_MACRO_LIST_46(NAME,PRE,POST), PRE ## NAME ## 46 ## POST
#define _GET_MACRO_LIST_48(NAME,PRE,POST)   _GET_MACRO_LIST_47(NAME,PRE,POST), PRE ## NAME ## 47 ## POST
#define _GET_MACRO_LIST_49(NAME,PRE,POST)   _GET_MACRO_LIST_48(NAME,PRE,POST), PRE ## NAME ## 48 ## POST
#define _GET_MACRO_LIST_50(NAME,PRE,POST)   _GET_MACRO_LIST_49(NAME,PRE,POST), PRE ## NAME ## 49 ## POST
#define _GET_MACRO_LIST_51(NAME,PRE,POST)   _GET_MACRO_LIST_50(NAME,PRE,POST), PRE ## NAME ## 50 ## POST
#define _GET_MACRO_LIST_52(NAME,PRE,POST)   _GET_MACRO_LIST_51(NAME,PRE,POST), PRE ## NAME ## 51 ## POST
#define _GET_MACRO_LIST_53(NAME,PRE,POST)   _GET_MACRO_LIST_52(NAME,PRE,POST), PRE ## NAME ## 52 ## POST
#define _GET_MACRO_LIST_54(NAME,PRE,POST)   _GET_MACRO_LIST_53(NAME,PRE,POST), PRE ## NAME ## 53 ## POST
#define _GET_MACRO_LIST_55(NAME,PRE,POST)   _GET_MACRO_LIST_54(NAME,PRE,POST), PRE ## NAME ## 54 ## POST
#define _GET_MACRO_LIST_56(NAME,PRE,POST)   _GET_MACRO_LIST_55(NAME,PRE,POST), PRE ## NAME ## 55 ## POST
#define _GET_MACRO_LIST_57(NAME,PRE,POST)   _GET_MACRO_LIST_56(NAME,PRE,POST), PRE ## NAME ## 56 ## POST
#define _GET_MACRO_LIST_58(NAME,PRE,POST)   _GET_MACRO_LIST_57(NAME,PRE,POST), PRE ## NAME ## 57 ## POST
#define _GET_MACRO_LIST_59(NAME,PRE,POST)   _GET_MACRO_LIST_58(NAME,PRE,POST), PRE ## NAME ## 58 ## POST
#define _GET_MACRO_LIST_60(NAME,PRE,POST)   _GET_MACRO_LIST_59(NAME,PRE,POST), PRE ## NAME ## 59 ## POST
#define _GET_MACRO_LIST_61(NAME,PRE,POST)   _GET_MACRO_LIST_60(NAME,PRE,POST), PRE ## NAME ## 60 ## POST
#define _GET_MACRO_LIST_62(NAME,PRE,POST)   _GET_MACRO_LIST_61(NAME,PRE,POST), PRE ## NAME ## 61 ## POST
#define _GET_MACRO_LIST_63(NAME,PRE,POST)   _GET_MACRO_LIST_62(NAME,PRE,POST), PRE ## NAME ## 62 ## POST
#define _GET_MACRO_LIST_64(NAME,PRE,POST)   _GET_MACRO_LIST_63(NAME,PRE,POST), PRE ## NAME ## 63 ## POST
#define _GET_MACRO_LIST_65(NAME,PRE,POST)   _GET_MACRO_LIST_64(NAME,PRE,POST), PRE ## NAME ## 64 ## POST
#define _GET_MACRO_LIST_66(NAME,PRE,POST)   _GET_MACRO_LIST_65(NAME,PRE,POST), PRE ## NAME ## 65 ## POST
#define _GET_MACRO_LIST_67(NAME,PRE,POST)   _GET_MACRO_LIST_66(NAME,PRE,POST), PRE ## NAME ## 66 ## POST
#define _GET_MACRO_LIST_68(NAME,PRE,POST)   _GET_MACRO_LIST_67(NAME,PRE,POST), PRE ## NAME ## 67 ## POST
#define _GET_MACRO_LIST_69(NAME,PRE,POST)   _GET_MACRO_LIST_68(NAME,PRE,POST), PRE ## NAME ## 68 ## POST
#define _GET_MACRO_LIST_70(NAME,PRE,POST)   _GET_MACRO_LIST_69(NAME,PRE,POST), PRE ## NAME ## 69 ## POST
#define _GET_MACRO_LIST_71(NAME,PRE,POST)   _GET_MACRO_LIST_70(NAME,PRE,POST), PRE ## NAME ## 70 ## POST
#define _GET_MACRO_LIST_72(NAME,PRE,POST)   _GET_MACRO_LIST_71(NAME,PRE,POST), PRE ## NAME ## 71 ## POST
#define _GET_MACRO_LIST_73(NAME,PRE,POST)   _GET_MACRO_LIST_72(NAME,PRE,POST), PRE ## NAME ## 72 ## POST
#define _GET_MACRO_LIST_74(NAME,PRE,POST)   _GET_MACRO_LIST_73(NAME,PRE,POST), PRE ## NAME ## 73 ## POST
#define _GET_MACRO_LIST_75(NAME,PRE,POST)   _GET_MACRO_LIST_74(NAME,PRE,POST), PRE ## NAME ## 74 ## POST
#define _GET_MACRO_LIST_76(NAME,PRE,POST)   _GET_MACRO_LIST_75(NAME,PRE,POST), PRE ## NAME ## 75 ## POST
#define _GET_MACRO_LIST_77(NAME,PRE,POST)   _GET_MACRO_LIST_76(NAME,PRE,POST), PRE ## NAME ## 76 ## POST
#define _GET_MACRO_LIST_78(NAME,PRE,POST)   _GET_MACRO_LIST_77(NAME,PRE,POST), PRE ## NAME ## 77 ## POST
#define _GET_MACRO_LIST_79(NAME,PRE,POST)   _GET_MACRO_LIST_78(NAME,PRE,POST), PRE ## NAME ## 78 ## POST
#define _GET_MACRO_LIST_80(NAME,PRE,POST)   _GET_MACRO_LIST_79(NAME,PRE,POST), PRE ## NAME ## 79 ## POST
#define _GET_MACRO_LIST_81(NAME,PRE,POST)   _GET_MACRO_LIST_80(NAME,PRE,POST), PRE ## NAME ## 80 ## POST
#define _GET_MACRO_LIST_82(NAME,PRE,POST)   _GET_MACRO_LIST_81(NAME,PRE,POST), PRE ## NAME ## 81 ## POST
#define _GET_MACRO_LIST_83(NAME,PRE,POST)   _GET_MACRO_LIST_82(NAME,PRE,POST), PRE ## NAME ## 82 ## POST
#define _GET_MACRO_LIST_84(NAME,PRE,POST)   _GET_MACRO_LIST_83(NAME,PRE,POST), PRE ## NAME ## 83 ## POST
#define _GET_MACRO_LIST_85(NAME,PRE,POST)   _GET_MACRO_LIST_84(NAME,PRE,POST), PRE ## NAME ## 84 ## POST
#define _GET_MACRO_LIST_86(NAME,PRE,POST)   _GET_MACRO_LIST_85(NAME,PRE,POST), PRE ## NAME ## 85 ## POST
#define _GET_MACRO_LIST_87(NAME,PRE,POST)   _GET_MACRO_LIST_86(NAME,PRE,POST), PRE ## NAME ## 86 ## POST
#define _GET_MACRO_LIST_88(NAME,PRE,POST)   _GET_MACRO_LIST_87(NAME,PRE,POST), PRE ## NAME ## 87 ## POST
#define _GET_MACRO_LIST_89(NAME,PRE,POST)   _GET_MACRO_LIST_88(NAME,PRE,POST), PRE ## NAME ## 88 ## POST
#define _GET_MACRO_LIST_90(NAME,PRE,POST)   _GET_MACRO_LIST_89(NAME,PRE,POST), PRE ## NAME ## 89 ## POST
#define _GET_MACRO_LIST_91(NAME,PRE,POST)   _GET_MACRO_LIST_90(NAME,PRE,POST), PRE ## NAME ## 90 ## POST
#define _GET_MACRO_LIST_92(NAME,PRE,POST)   _GET_MACRO_LIST_91(NAME,PRE,POST), PRE ## NAME ## 91 ## POST
#define _GET_MACRO_LIST_93(NAME,PRE,POST)   _GET_MACRO_LIST_92(NAME,PRE,POST), PRE ## NAME ## 92 ## POST
#define _GET_MACRO_LIST_94(NAME,PRE,POST)   _GET_MACRO_LIST_93(NAME,PRE,POST), PRE ## NAME ## 93 ## POST
#define _GET_MACRO_LIST_95(NAME,PRE,POST)   _GET_MACRO_LIST_94(NAME,PRE,POST), PRE ## NAME ## 94 ## POST
#define _GET_MACRO_LIST_96(NAME,PRE,POST)   _GET_MACRO_LIST_95(NAME,PRE,POST), PRE ## NAME ## 95 ## POST
#define _GET_MACRO_LIST_97(NAME,PRE,POST)   _GET_MACRO_LIST_96(NAME,PRE,POST), PRE ## NAME ## 96 ## POST
#define _GET_MACRO_LIST_98(NAME,PRE,POST)   _GET_MACRO_LIST_97(NAME,PRE,POST), PRE ## NAME ## 97 ## POST
#define _GET_MACRO_LIST_99(NAME,PRE,POST)   _GET_MACRO_LIST_98(NAME,PRE,POST), PRE ## NAME ## 98 ## POST
#define _GET_MACRO_LIST_100(NAME,PRE,POST)  _GET_MACRO_LIST_99(NAME,PRE,POST), PRE ## NAME ## 99 ## POST

// Alphabetic version.
#define _GET_MACRO_LIST_ALPHA_1(NAME,PRE,POST)      PRE ## NAME ## A ## POST
#define _GET_MACRO_LIST_ALPHA_2(NAME,PRE,POST)      PRE ## NAME ## A ## POST , PRE ## NAME ## B ## POST
#define _GET_MACRO_LIST_ALPHA_3(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_2(NAME,PRE,POST) , PRE ## NAME ## C ## POST
#define _GET_MACRO_LIST_ALPHA_4(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_3(NAME,PRE,POST) , PRE ## NAME ## D ## POST
#define _GET_MACRO_LIST_ALPHA_5(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_4(NAME,PRE,POST) , PRE ## NAME ## E ## POST
#define _GET_MACRO_LIST_ALPHA_6(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_5(NAME,PRE,POST) , PRE ## NAME ## F ## POST
#define _GET_MACRO_LIST_ALPHA_7(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_6(NAME,PRE,POST) , PRE ## NAME ## G ## POST
#define _GET_MACRO_LIST_ALPHA_8(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_7(NAME,PRE,POST) , PRE ## NAME ## H ## POST
#define _GET_MACRO_LIST_ALPHA_9(NAME,PRE,POST)      _GET_MACRO_LIST_ALPHA_8(NAME,PRE,POST) , PRE ## NAME ## I ## POST
#define _GET_MACRO_LIST_ALPHA_10(NAME,PRE,POST)     _GET_MACRO_LIST_ALPHA_9(NAME,PRE,POST) , PRE ## NAME ## J ## POST
#define _GET_MACRO_LIST_ALPHA_11(NAME,PRE,POST)     _GET_MACRO_LIST_ALPHA_10(NAME,PRE,POST), PRE ## NAME ## K ## POST
#define _GET_MACRO_LIST_ALPHA_12(NAME,PRE,POST)     _GET_MACRO_LIST_ALPHA_11(NAME,PRE,POST), PRE ## NAME ## L ## POST
#define _GET_MACRO_LIST_ALPHA_13(NAME,PRE,POST)     _GET_MACRO_LIST_ALPHA_12(NAME,PRE,POST), PRE ## NAME ## M ## POST
#define _GET_MACRO_LIST_ALPHA_14(NAME,PRE,POST)     _GET_MACRO_LIST_ALPHA_13(NAME,PRE,POST), PRE ## NAME ## N ## POST
#define _GET_MACRO_LIST_ALPHA_15(NAME,PRE,POST)     _GET_MACRO_LIST_ALPHA_14(NAME,PRE,POST), PRE ## NAME ## O ## POST

#define CAT(a, ...) a ## __VA_ARGS__
#define _GET_MACRO_LIST(NAME,PRE,POST,COUNT)        CAT( _GET_MACRO_LIST_, COUNT )(NAME,PRE,POST)
#define _GET_MACRO_LIST_ALPHA(NAME,PRE,POST,COUNT)  CAT( _GET_MACRO_LIST_ALPHA_, COUNT )(NAME,PRE,POST)

#define PHY_BASEADDR_LIST(NAME)                         _GET_MACRO_LIST( NAME, PHY_BASEADDR_  , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define PHY_BASEADDR_WITH_CHANNEL_LIST(NAME,CHANNEL)    _GET_MACRO_LIST( NAME, PHY_BASEADDR_  , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
#define INTNUM_LIST(NAME)                               _GET_MACRO_LIST( NAME, INTNUM_OF_     , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define INTNUM_WITH_CHANNEL_LIST(NAME,CHANNEL)          _GET_MACRO_LIST( NAME, INTNUM_OF_     , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
#define DMAINDEX_LIST(NAME)                             _GET_MACRO_LIST( NAME, DMAINDEX_OF_   , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define DMAINDEX_WITH_CHANNEL_LIST(NAME,CHANNEL)        _GET_MACRO_LIST( NAME, DMAINDEX_OF_   , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
#define PADINDEX_LIST(NAME)                             _GET_MACRO_LIST( NAME, PADINDEX_OF_   ,  , NUMBER_OF_ ## NAME ## _MODULE )
#define PADINDEX_WITH_CHANNEL_LIST(NAME,CHANNEL)        _GET_MACRO_LIST( NAME, PADINDEX_OF_   , _ ## CHANNEL , NUMBER_OF_ ## NAME ## _MODULE )
#define CLOCKINDEX_LIST(NAME)                           _GET_MACRO_LIST( NAME, CLOCKINDEX_OF_ , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define RESETINDEX_LIST(NAME,CHANNEL)                   _GET_MACRO_LIST( NAME, RESETINDEX_OF_ , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
//@modified martin 2012/8/6
#define TIEOFFINDEX_WITH_CHANNEL_LIST(NAME,CHANNEL)             _GET_MACRO_LIST( NAME, TIEOFFINDEX_OF_   , _ ## CHANNEL , NUMBER_OF_ ## NAME ## _MODULE )

#define PHY_BASEADDR_LIST_ALPHA(NAME)                           _GET_MACRO_LIST_ALPHA( NAME, PHY_BASEADDR_  , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define PHY_BASEADDR_WITH_CHANNEL_LIST_ALPHA(NAME,CHANNEL)      _GET_MACRO_LIST_ALPHA( NAME, PHY_BASEADDR_  , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
#define INTNUM_LIST_ALPHA(NAME)                                 _GET_MACRO_LIST_ALPHA( NAME, INTNUM_OF_     , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define INTNUM_WITH_CHANNEL_LIST_ALPHA(NAME,CHANNEL)            _GET_MACRO_LIST_ALPHA( NAME, INTNUM_OF_     , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
#define DMAINDEX_LIST_ALPHA(NAME)                               _GET_MACRO_LIST_ALPHA( NAME, DMAINDEX_OF_   , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define DMAINDEX_WITH_CHANNEL_LIST_ALPHA(NAME,CHANNEL)          _GET_MACRO_LIST_ALPHA( NAME, DMAINDEX_OF_   , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
#define PADINDEX_LIST_ALPHA(NAME)                               _GET_MACRO_LIST_ALPHA( NAME, PADINDEX_OF_   ,  , NUMBER_OF_ ## NAME ## _MODULE )
#define PADINDEX_WITH_CHANNEL_LIST_ALPHA(NAME,CHANNEL)          _GET_MACRO_LIST_ALPHA( NAME, PADINDEX_OF_   , _ ## CHANNEL , NUMBER_OF_ ## NAME ## _MODULE )
#define CLOCKINDEX_LIST_ALPHA(NAME)                             _GET_MACRO_LIST_ALPHA( NAME, CLOCKINDEX_OF_ , _MODULE, NUMBER_OF_ ## NAME ## _MODULE )
#define RESETINDEX_LIST_ALPHA(NAME,CHANNEL)                     _GET_MACRO_LIST_ALPHA( NAME, RESETINDEX_OF_ , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )
//@modified martin 2012/8/6
#define TIEOFFINDEX_LIST_ALPHA(NAME,CHANNEL)                    _GET_MACRO_LIST_ALPHA( NAME, TIEOFFINDEX_OF_ , _MODULE_ ## CHANNEL, NUMBER_OF_ ## NAME ## _MODULE )

#define NX_BIT_GETBIT_RANGE32( Value,  MSB, LSB )               NX_BIT_GetBitRange32( Value,  MSB, LSB )
#define NX_BIT_SETBIT_RANGE32( OldValue, BitValue,  BitNumber ) NX_BIT_SetBitRange32( OldValue, BitValue,  BitNumber )

#endif  // __NX_PROTOTYPE_H__
