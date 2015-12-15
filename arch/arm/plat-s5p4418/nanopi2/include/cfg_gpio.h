/*------------------------------------------------------------------------------
 *
 *	Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
 *	Nexell Co. Proprietary & Confidential
 *
 *	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *	FOR A PARTICULAR PURPOSE.
 *
 *	Module     : System memory config
 *	Description:
 *	Author     : Platform Team
 *	Export     :
 *	History    :
 *	   2009/05/13 first implementation
 ------------------------------------------------------------------------------*/
#ifndef __CFG_GPIO_H__
#define __CFG_GPIO_H__

/*------------------------------------------------------------------------------
 *
 *	(GROUP_A)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOA0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: PVCLK               ,2:_                    ,3: TESTMODE[4]         =
#define PAD_GPIOA1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[0]          ,2:_                    ,3:_                    =
#define PAD_GPIOA2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[1]          ,2:_                    ,3: TESTMODE[0]         =
#define PAD_GPIOA3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[2]          ,2:_                    ,3: TESTMODE[1]         =
#define PAD_GPIOA4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[3]          ,2:_                    ,3: TESTMODE[2]         =
#define PAD_GPIOA5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[4]          ,2:_                    ,3: TESTMODE[3]         =
#define PAD_GPIOA6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[5]          ,2:_                    ,3:_                    =
#define PAD_GPIOA7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[6]          ,2:_                    ,3:_                    =
#define PAD_GPIOA8      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[7]          ,2:_                    ,3:_                    =
#define PAD_GPIOA9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[8]          ,2:_                    ,3:_                    =
#define PAD_GPIOA10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[9]          ,2:_                    ,3:_                    =
#define PAD_GPIOA11     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[10]         ,2:_                    ,3:_                    =
#define PAD_GPIOA12     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[11]         ,2:_                    ,3:_                    =
#define PAD_GPIOA13     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[12]         ,2:_                    ,3:_                    =
#define PAD_GPIOA14     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[13]         ,2:_                    ,3:_                    =
#define PAD_GPIOA15     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[14]         ,2:_                    ,3:_                    =
#define PAD_GPIOA16     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[15]         ,2:_                    ,3:-                    =
#define PAD_GPIOA17     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[16]         ,2:_                    ,3:_                    =
#define PAD_GPIOA18     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[17]         ,2:_                    ,3:_                    =
#define PAD_GPIOA19     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[18]         ,2:_                    ,3:_                    =
#define PAD_GPIOA20     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[19]         ,2:_                    ,3:_                    =
#define PAD_GPIOA21     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[20]         ,2:_                    ,3:_                    =
#define PAD_GPIOA22     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[21]         ,2:_                    ,3:_                    =
#define PAD_GPIOA23     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[22]         ,2:_                    ,3:_                    =
#define PAD_GPIOA24     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[23]         ,2:_                    ,3:_                    =
#define PAD_GPIOA25     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDVSYNC             ,2:_                    ,3:_                    =
#define PAD_GPIOA26     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDHSYNC             ,2:_                    ,3:_                    =
#define PAD_GPIOA27     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDDE                ,2:_                    ,3:_                    =
#define PAD_GPIOA28     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VP0_EXTCLK          ,2: I2S2_CLK            ,3: I2S1_CLK            =
#define PAD_GPIOA29     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CCLK         ,2:_                    ,3:_                    =
#define PAD_GPIOA30     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[0]          ,2: SDEX[0]             ,3: I2S1_BCLK           =
#define PAD_GPIOA31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CMD          ,2:_                    ,3:_                    =

/*------------------------------------------------------------------------------
 *	(GROUP_B)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOB0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[1]          ,2: SDEX[1]             ,3: I2S1_LRCLK          =
#define PAD_GPIOB1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[0]     ,2:_                    ,3:_                    =
#define PAD_GPIOB2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[2]          ,2: SDEX[2]             ,3: I2S2_BCLK           =
#define PAD_GPIOB3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[1]     ,2:_                    ,3:_                    =
#define PAD_GPIOB4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[3]          ,2: SDEX[3]             ,3: I2S2_LRCLK          =
#define PAD_GPIOB5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[2]     ,2:_                    ,3:_                    =
#define PAD_GPIOB6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[4]          ,2: SDEX[4]             ,3: I2S1SDO             =
#define PAD_GPIOB7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[3]     ,2:_                    ,3:_                    =
#define PAD_GPIOB8      (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[5]          ,2: SDEX[5]             ,3: I2S2SDO             =
#define PAD_GPIOB9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[6]          ,2: SDEX[6]             ,3: I2S1SDI             =
#define PAD_GPIOB10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[7]          ,2: SDEX[7]             ,3: I2S2SDI             =
#define PAD_GPIOB11     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: CLE           ,1: CLE1                ,2: GPIO                ,3:_                    =
#define PAD_GPIOB12     (PAD_MODE_OUT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: ALE           ,1: ALE1                ,2: GPIO                ,3:_                    =
#define PAD_GPIOB13     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[0]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB14     (PAD_MODE_OUT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_RnB      ,1: MCUS_RnB1           ,2: GPIO                ,3:_                    =
#define PAD_GPIOB15     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[1]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB16     (PAD_MODE_OUT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nNOFE    ,1: MCUS_nNOFE1         ,2: GPIO                ,3:_                    =
#define PAD_GPIOB17     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[2]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB18     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nNFWE    ,1: MCUS_nNFWE1         ,2: GPIO                ,3:_                    =
#define PAD_GPIOB19     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[3]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB20     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[4]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB21     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[5]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB22     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[6]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB23     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[7]    ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOB24     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[8]    ,1: GPIO                ,2: MPEGTSI0_TDATA[0]   ,3:_                    =
#define PAD_GPIOB25     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: MCUS_SD[9]    ,1: GPIO                ,2: MPEGTSI0_TDATA[1]   ,3:_                    =
#define PAD_GPIOB26     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[10]   ,1: GPIO                ,2: MPEGTSI0_TDATA[2]   ,3: ECID_BONDING_ID[2]  =
#define PAD_GPIOB27     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[11]   ,1: GPIO                ,2: MPEGTSI0_TDATA[3]   ,3:_                    =
#define PAD_GPIOB28     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[12]   ,1: GPIO                ,2: MPEGTSI0_TDATA[4]   ,3: UART4_RXD           =
#define PAD_GPIOB29     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[13]   ,1: GPIO                ,2: MPEGTSI0_TDATA[5]   ,3: UART4_TXD           =
#define PAD_GPIOB30     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[14]   ,1: GPIO                ,2: MPEGTSI0_TDATA[6]   ,3: UART5_RXD           =
#define PAD_GPIOB31     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[15]   ,1: GPIO                ,2: MPEGTSI0_TDATA[7]   ,3: UART5_TXD           =

/*------------------------------------------------------------------------------
 *	(GROUP_C)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOC0      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[0]  ,1: GPIO                ,2: MPEGTSI0_TSERR      ,3:_                    =
#define PAD_GPIOC1      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[1]  ,1: GPIO                ,2: MPEGTSI1_TSERR      ,3:_                    =
#define PAD_GPIOC2      (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[2]  ,1: GPIO                ,2:_                    ,3:_                    =
#define PAD_GPIOC3      (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[3]  ,1: GPIO                ,2: HDMI_CEC            ,3: SDMMC0_nRST         =
#define PAD_GPIOC4      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[4]  ,1: GPIO                ,2: UART1_DCD           ,3: SDMMC0_CARD_nint    =
#define PAD_GPIOC5      (PAD_MODE_IN  | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[5]  ,1: GPIO                ,2: UART1_CTS           ,3: SDMMC0_WP           =
#define PAD_GPIOC6      (PAD_MODE_OUT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[6]  ,1: GPIO                ,2: UART1_RTS           ,3: SDMMC0_DETECT       =
#define PAD_GPIOC7      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[7]  ,1: GPIO                ,2: UART1_DSR           ,3: SDMMC1_nRST         =
#define PAD_GPIOC8      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[8]  ,1: GPIO                ,2: UART1_DTR           ,3: SDMMC1_CARD_nint    =
#define PAD_GPIOC9      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[9]  ,1: GPIO                ,2: SSP2_CLK_IO         ,3: PDM_STROBE          =
#define PAD_GPIOC10     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[10] ,1: GPIO                ,2: SSP2_FSS            ,3: MCUS_nNCS[2]        =
#define PAD_GPIOC11     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[11] ,1: GPIO                ,2: SSP2_RXD            ,3: USB2.0OTG0_DRVBUS   =
#define PAD_GPIOC12     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[12] ,1: GPIO                ,2: SSP2_TXD            ,3: SDMMC2_nRST         =
#define PAD_GPIOC13     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[13] ,1: GPIO                ,2: PWM1_OUT            ,3: SDMMC2_CARD_nint    =
#define PAD_GPIOC14     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[14] ,1: GPIO                ,2: PWM2_OUT            ,3: VIP0_ExtCLK2        =
#define PAD_GPIOC15     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[15] ,1: GPIO                ,2: MPEGTSI0_TSCLK      ,3: VIP0_HSYNC2         =
#define PAD_GPIOC16     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[16] ,1: GPIO                ,2: MPEGTSI0_TSYNC0     ,3: VIP0_VSYNC2         =
#define PAD_GPIOC17     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[17] ,1: GPIO                ,2: MPEGTSI0_TDP0       ,3: VIP0_VD2[0]         =
#define PAD_GPIOC18     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[18] ,1: GPIO                ,2: SDMMC2_CCLK         ,3: VIP0_VD2[1]         =
#define PAD_GPIOC19     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[19] ,1: GPIO                ,2: SDMMC2_CMD          ,3: VIP0_VD2[2]         =
#define PAD_GPIOC20     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[20] ,1: GPIO                ,2: SDMMC2_CDATA[0]     ,3: VIP0_VD2[3]         =
#define PAD_GPIOC21     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[21] ,1: GPIO                ,2: SDMMC2_CDATA[1]     ,3: VIP0_VD2[4]         =
#define PAD_GPIOC22     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[22] ,1: GPIO                ,2: SDMMC2_CDATA[2]     ,3: VIP0_VD2[5]         =
#define PAD_GPIOC23     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[23] ,1: GPIO                ,2: SDMMC2_CDATA[3]     ,3: VIP0_VD2[6]         =
#define PAD_GPIOC24     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_LATADDR  ,1: GPIO                ,2: SPDIFIN             ,3: VIP0_VD2[7]         =
#define PAD_GPIOC25     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_nSWAIT   ,1: GPIO                ,2: SPDIF_DATA          ,3:_                    =
#define PAD_GPIOC26     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_RDnWR    ,1: GPIO                ,2: PDM_DATA0           ,3:_                    =
#define PAD_GPIOC27     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: MCUS_nSDQM1   ,1: GPIO                ,2: PDM_DATA1           ,3:_                    =
#define PAD_GPIOC28     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: MCUS_nSCS[1]        ,2: UART1_TRI           ,3:_                    =
#define PAD_GPIOC29     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_CLKIO          ,2:_                    ,3:_                    =
#define PAD_GPIOC30     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_FSS            ,2:_                    ,3:_                    =
#define PAD_GPIOC31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_TXD            ,2:_                    ,3:_                    =

/*------------------------------------------------------------------------------
 *	(GROUP_D)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOD0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_RXD            ,2: PWM3_OUT            ,3:_                    =
#define PAD_GPIOD1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PWM0_OUT            ,2: MCUS_ADDR[25]       ,3:_                    =
#define PAD_GPIOD2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: I2C0_SCL            ,2: UART4_SMCAYEN       ,3:_                    =
#define PAD_GPIOD3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: I2C0_SDA            ,2: UART5_SMCAYEN       ,3:_                    =
#define PAD_GPIOD4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: I2C1_SCL            ,2:_                    ,3:_                    =
#define PAD_GPIOD5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: I2C1_SDA            ,2:_                    ,3:_                    =
#define PAD_GPIOD6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: I2C2_SCL            ,2:_                    ,3:_                    =
#define PAD_GPIOD7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: I2C2_SDA            ,2:_                    ,3:_                    =
#define PAD_GPIOD8      (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: PPM_IN              ,2:_                    ,3:_                    =
#define PAD_GPIOD9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_SDO            ,2: AC97_ACSDATAOUT     ,3:_                    =
#define PAD_GPIOD10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_BCLK           ,2: AC97_ACBITCLK       ,3:_                    =
#define PAD_GPIOD11     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_SDI            ,2: AC97_ACSDATAIN      ,3:_                    =
#define PAD_GPIOD12     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_LRCLK          ,2: AC97_ACSYNC         ,3:_                    =
#define PAD_GPIOD13     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_CODCLK         ,2: AC97_nACRESET       ,3:_                    =
#define PAD_GPIOD14     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART0RXD            ,2: UART1_SMCAYEN       ,3:_                    =
#define PAD_GPIOD15     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART1RXD            ,2: UART2_SMCAYEN       ,3:_                    =
#define PAD_GPIOD16     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART2RXD            ,2: CAN0_TX             ,3:_                    =
#define PAD_GPIOD17     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART3RXD            ,2: CAN1_TX             ,3:_                    =
#define PAD_GPIOD18     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART0TXD            ,2:_                    ,3: SDnCD2              =
#define PAD_GPIOD19     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART1TXD            ,2:_                    ,3:_                    =
#define PAD_GPIOD20     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART2TXD            ,2:_                    ,3:_                    =
#define PAD_GPIOD21     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART3TXD            ,2:_                    ,3:_                    =
#define PAD_GPIOD22     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SDMMC1_CCLK         ,2:_                    ,3:_                    =
#define PAD_GPIOD23     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SDMMC1_CMD          ,2:_                    ,3:_                    =
#define PAD_GPIOD24     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SDMMC1_CDATA[0]     ,2:_                    ,3:_                    =
#define PAD_GPIOD25     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SDMMC1_CDATA[1]     ,2:_                    ,3:_                    =
#define PAD_GPIOD26     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SDMMC1_CDATA[3]     ,2:_                    ,3:_                    =
#define PAD_GPIOD27     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SDMMC1_CDATA[3]     ,2:_                    ,3:_                    =
#define PAD_GPIOD28     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[0]          ,2: MPEGTSI_TDATA1[0]   ,3: MCUS_ADDR[24]       =
#define PAD_GPIOD29     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[1]          ,2: MPEGTSI_TDATA1[1]   ,3:_                    =
#define PAD_GPIOD30     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[2]          ,2: MPEGTSI_TDATA1[2]   ,3:_                    =
#define PAD_GPIOD31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[3]          ,2: MPEGTSI_TDATA1[3]   ,3:_                    =

/*------------------------------------------------------------------------------
 *	(GROUP_E)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOE0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[4]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    =
#define PAD_GPIOE1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[5]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    =
#define PAD_GPIOE2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[6]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    =
#define PAD_GPIOE3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[7]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    =
#define PAD_GPIOE4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_ExtCLK          ,2: MPEGTSI_TCLK1       ,3:_                    =
#define PAD_GPIOE5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_HSYNC           ,2: MPEGTSI_TSYNC1      ,3:_                    =
#define PAD_GPIOE6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP_VSYNC            ,2: MPEGTSI_TDP1        ,3:_                    =
#define PAD_GPIOE7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_TXD[0]     ,2: VIP0_Ext_VSYNC      ,3:_                    =
#define PAD_GPIOE8      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_TXD[1]     ,2:_                    ,3:_                    =
#define PAD_GPIOE9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_TXD[2]     ,2:_                    ,3:_                    =
#define PAD_GPIOE10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_TXD[3]     ,2:_                    ,3:_                    =
#define PAD_GPIOE11     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_TXEN       ,2:_                    ,3:_                    =
#define PAD_GPIOE12     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_TXER       ,2:_                    ,3:_                    =
#define PAD_GPIOE13     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_COL        ,2: VIP0_Ext_HSYNC      ,3:_                    =
#define PAD_GPIOE14     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_RXD[0]     ,2: SSP1_CLKIO          ,3:_                    =
#define PAD_GPIOE15     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_RXD[1]     ,2: SSP1_FSS            ,3:_                    =
#define PAD_GPIOE16     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_RXD[2]     ,2:_                    ,3:_                    = VG_EN
#define PAD_GPIOE17     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_RXD[3]     ,2:_                    ,3:_                    =
#define PAD_GPIOE18     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_CLK_RX         ,2: SSP1_RXD            ,3:_                    =
#define PAD_GPIOE19     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_PHY_RX_DV      ,2: SSP1_TXD            ,3:_                    =
#define PAD_GPIOE20     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_GMII_MDC       ,2:_                    ,3:_                    =
#define PAD_GPIOE21     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_2)     // 0: GPIO          1: GMAC0_GMII_MDI       ,2: SDMMC2_CDATA[4]     ,3:_                    =
#define PAD_GPIOE22     (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_RXER       ,2: SDMMC2_CDATA[5]     ,3:_                    =
#define PAD_GPIOE23     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_CRS        ,2: SDMMC2_CDATA[6]     ,3:_                    =
#define PAD_GPIOE24     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_GTX_CLK        ,2: SDMMC2_CDATA[7]     ,3:_                    =
#define PAD_GPIOE25     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: nTRST         1: GPIO                 ,2:_                    ,3:_                    =
#define PAD_GPIOE26     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TMS           1: GPIO                 ,2:_                    ,3:_                    =
#define PAD_GPIOE27     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TDI           1: GPIO                 ,2:_                    ,3:_                    =
#define PAD_GPIOE28     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TCLK          1: GPIO                 ,2:_                    ,3:_                    =
#define PAD_GPIOE29     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TDO           1: GPIO                 ,2:_                    ,3:_                    =
#define PAD_GPIOE30     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nSOE     1: GPIO                 ,2:_                    ,3:_                    =
#define PAD_GPIOE31     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nSWE     1: GPIO                 ,2:_                    ,3:_                    =

/*------------------------------------------------------------------------------
 *	(GROUPALV)
 *	0                     4                             8        12
 *	| MODE(IN/OUT/DETECT) | ALIVE OUT or ALIVE DETMODE0 | PullUp |
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOALV0    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				//
#define PAD_GPIOALV1    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_OFF)				//
#define PAD_GPIOALV2    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				//
#define PAD_GPIOALV3    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				//
#define PAD_GPIOALV4    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				//
#define PAD_GPIOALV5    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				//

/*------------------------------------------------------------------------------
 *	TOUCH
 */
#define CFG_IO_TOUCH_IRQ					(PAD_GPIO_C + 16)

/*------------------------------------------------------------------------------
 *	GPIO EEPROM
 */
#define CFG_IO_SPI_EEPROM_WP				((PAD_GPIO_B + 26) | PAD_FUNC_ALT1)		/* GPIO */

/*------------------------------------------------------------------------------
 *	CAMERA Back Power Down PN
 */
#define CFG_IO_CAMERA_BACK_POWER_DOWN		((PAD_GPIO_B + 14) | PAD_FUNC_ALT0)

/*------------------------------------------------------------------------------
 *	CAMERA Front Power Down PD
 */ 
#define CFG_IO_CAMERA_FRONT_POWER_DOWN		((PAD_GPIO_C + 6) | PAD_FUNC_ALT0)		/* GPIO */

/*------------------------------------------------------------------------------
 *	CAMERA Reset
 */
#define CFG_IO_CAMERA_RESET					((PAD_GPIO_B + 16) | PAD_FUNC_ALT0)		/* GPIO */

/*------------------------------------------------------------------------------
 *	AUDIO AMP for es8316
 */
#define CFG_IO_AUDIO_AMP_POWER				(PAD_GPIO_D + 19)		/* GPIO */


#endif	/* __CFG_GPIO_H__ */

