/*------------------------------------------------------------------------------
 *
 *	Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
 *	Nexell Co. Proprietary & Confidential
 *
 *	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *  FOR A PARTICULAR PURPOSE.
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
#define PAD_GPIOA0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PVCLK               ,2:_                    ,3: TESTMODE[4]         =	DISD_CLK	, ITU656_CLK
#define PAD_GPIOA1      (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[0]          ,2:_                    ,3:_                    =	DISD_D
#define PAD_GPIOA2      (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[1]          ,2:_                    ,3: TESTMODE[0]         =	DISD_D1
#define PAD_GPIOA3      (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[2]          ,2:_                    ,3: TESTMODE[1]         =	DISD_D2
#define PAD_GPIOA4      (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[3]          ,2:_                    ,3: TESTMODE[2]         =	DISD_D3
#define PAD_GPIOA5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[4]          ,2:_                    ,3: TESTMODE[3]         =	DISD_D4
#define PAD_GPIOA6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[5]          ,2:_                    ,3:_                    =	DISD_D5
#define PAD_GPIOA7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[6]          ,2:_                    ,3:_                    =	DISD_D6
#define PAD_GPIOA8      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[7]          ,2:_                    ,3:_                    =	DISD_D7
#define PAD_GPIOA9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[8]          ,2:_                    ,3:_                    =	DISD_D8 	, ITU656_D0		, GPIOA9_RTL_nINT
#define PAD_GPIOA10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[9]          ,2:_                    ,3:_                    =	DISD_D9 	, ITU656_D1		, GPIOA10_RTL_nRST
#define PAD_GPIOA11     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[10]         ,2:_                    ,3:_                    =	DISD_D10	, ITU656_D2
#define PAD_GPIOA12     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[11]         ,2:_                    ,3:_                    =	DISD_D11	, ITU656_D3
#define PAD_GPIOA13     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[12]         ,2:_                    ,3:_                    =	DISD_D12	, ITU656_D4
#define PAD_GPIOA14     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[13]         ,2:_                    ,3:_                    =	DISD_D13	, ITU656_D5
#define PAD_GPIOA15     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[14]         ,2:_                    ,3:_                    =	DISD_D14	, ITU656_D6
#define PAD_GPIOA16     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          ,1: PDRGB24[15]         ,2:_                    ,3:-                    =	DISD_D15	, ITU656_D7
#define PAD_GPIOA17     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[16]         ,2:_                    ,3:_                    =	DISD_D16
#define PAD_GPIOA18     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[17]         ,2:_                    ,3:_                    =	DISD_D17
#define PAD_GPIOA19     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[18]         ,2:_                    ,3:_                    =	DISD_D18
#define PAD_GPIOA20     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[19]         ,2:_                    ,3:_                    =	DISD_D19
#define PAD_GPIOA21     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[20]         ,2:_                    ,3:_                    =	DISD_D20
#define PAD_GPIOA22     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[21]         ,2:_                    ,3:_                    =	DISD_D21
#define PAD_GPIOA23     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[22]         ,2:_                    ,3:_                    =	DISD_D22
#define PAD_GPIOA24     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDRGB24[23]         ,2:_                    ,3:_                    =	DISD_D23
#define PAD_GPIOA25     (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDVSYNC             ,2:_                    ,3:_                    =	DISD_VSYNC	, ITU601_VSYNC
#define PAD_GPIOA26     (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDHSYNC             ,2:_                    ,3:_                    =	DISD_HSYNC	, ITU601_HSYNC
#define PAD_GPIOA27     (PAD_MODE_IN  | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PDDE                ,2:_                    ,3:_                    =	DISD_DE
#define PAD_GPIOA28     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VP0_EXTCLK          ,2: I2S2_CLK            ,3: I2S1_CLK            =	VID1_PCLK	, ITU656_CLK	, I2S_MCLK, I2S0_MCLK_LOOP
#define PAD_GPIOA29     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CCLK         ,2:_                    ,3:_                    =	SD0_CLK
#define PAD_GPIOA30     (PAD_MODE_ALT | PAD_FUNC_ALT3 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[0]          ,2: SDEX[0]             ,3: I2S1_BCLK           =	VID1_D0		, ITU656_D0		, I2S_BCLK
#define PAD_GPIOA31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CMD          ,2:_                    ,3:_                    =	SD0_CMD

/*------------------------------------------------------------------------------
 *	(GROUP_B)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOB0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[1]          ,2: SDEX[1]             ,3: I2S1_LRCLK          =	VID1_D1		, ITU656_D1		, I2S_LRCK
#define PAD_GPIOB1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[0]     ,2:_                    ,3:_                    =	SD0_D0
#define PAD_GPIOB2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[2]          ,2: SDEX[2]             ,3: I2S2_BCLK           =	VID1_D2		, ITU656_D2		, I2S_BCLK		,  I2S_BCLK_LOOP
#define PAD_GPIOB3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[1]     ,2:_                    ,3:_                    =	SD0_D1
#define PAD_GPIOB4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[3]          ,2: SDEX[3]             ,3: I2S2_LRCLK          =	VID1_D3		, I2S_LRCK		, I2S_LRCK_LOOP	, I2S_DOUT
#define PAD_GPIOB5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[2]     ,2:_                    ,3:_                    =	SD0_D2
#define PAD_GPIOB6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[4]          ,2: SDEX[4]             ,3: I2S1SDO             = 	VID1_D4,	, ITU656_D4		, I2S_DOUT
#define PAD_GPIOB7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC0_CDATA[3]     ,2:_                    ,3:_                    =	SD0_D3
#define PAD_GPIOB8      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[5]          ,2: SDEX[5]             ,3: I2S2SDO             =	VID1_D5		, ITU656_D5		, I2S_DOUT		, I2S_DIN_LOOP
#define PAD_GPIOB9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[6]          ,2: SDEX[6]             ,3: I2S1SDI             =	VID1_D6		, ITU656_D6		, I2S_DIN
#define PAD_GPIOB10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP0_VD[7]          ,2: SDEX[7]             ,3: I2S2SDI             =	VID1_D7		, ITU656_D7 	, I2S_DIN		, I2S_DOUT_LOOP
#define PAD_GPIOB11     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: CLE           ,1: CLE1                ,2: GPIO                ,3:_                    =	CLE0
#define PAD_GPIOB12     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: ALE           ,1: ALE1                ,2: GPIO                ,3:_                    =	ALE0
#define PAD_GPIOB13     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: MCUS_SD[0]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D0
#define PAD_GPIOB14     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_RnB      ,1: MCUS_RnB1           ,2: GPIO                ,3:_                    =	RnB0
#define PAD_GPIOB15     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[1]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D1
#define PAD_GPIOB16     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nNOFE    ,1: MCUS_nNOFE1         ,2: GPIO                ,3:_                    =	nNFOE0
#define PAD_GPIOB17     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[2]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D2
#define PAD_GPIOB18     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nNFWE    ,1: MCUS_nNFWE1         ,2: GPIO                ,3:_                    =	nNFWE0
#define PAD_GPIOB19     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[3]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D3
#define PAD_GPIOB20     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[4]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D4
#define PAD_GPIOB21     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[5]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D5
#define PAD_GPIOB22     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[6]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D6
#define PAD_GPIOB23     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[7]    ,1: GPIO                ,2:_                    ,3:_                    =	MCU_STATIC_D7
#define PAD_GPIOB24     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[8]    ,1: GPIO                ,2: MPEGTSI0_TDATA[0]   ,3:_                    =	SD8		, GPIOB24_CAM2_DEC_PDN, TSID_D0
#define PAD_GPIOB25     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_HIGH  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[9]    ,1: GPIO                ,2: MPEGTSI0_TDATA[1]   ,3:_                    =	SD9		, TSID_D1
#define PAD_GPIOB26     (PAD_MODE_INT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[10]   ,1: GPIO                ,2: MPEGTSI0_TDATA[2]   ,3: ECID_BONDING_ID[2]  =	SD10	, TSID_D2
#define PAD_GPIOB27     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[11]   ,1: GPIO                ,2: MPEGTSI0_TDATA[3]   ,3:_                    =	SD11	, TSID_D3
#define PAD_GPIOB28     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[12]   ,1: GPIO                ,2: MPEGTSI0_TDATA[4]   ,3: UART4_RXD           =	SD12	, TSID_D4	, UART4_RX
#define PAD_GPIOB29     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[13]   ,1: GPIO                ,2: MPEGTSI0_TDATA[5]   ,3: UART4_TXD           =	SD13	, TSID_D5	, UART4_TX
#define PAD_GPIOB30     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[14]   ,1: GPIO                ,2: MPEGTSI0_TDATA[6]   ,3: UART5_RXD           =	SD14	, TSID_D6	, TEST_RX
#define PAD_GPIOB31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_SD[15]   ,1: GPIO                ,2: MPEGTSI0_TDATA[7]   ,3: UART5_TXD           =	SD15	, TSID_D7	, TEST_TX

/*------------------------------------------------------------------------------
 *	(GROUP_C)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOC0      (PAD_MODE_INT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[0]  ,1: GPIO                ,2: MPEGTSI0_TSERR      ,3:_                    =	MCU_SA0, TSI_ERR0
#define PAD_GPIOC1      (PAD_MODE_INT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[1]  ,1: GPIO                ,2: MPEGTSI1_TSERR      ,3:_                    =	MCU_SA1, TSI_ERR1
#define PAD_GPIOC2      (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[2]  ,1: GPIO                ,2:_                    ,3:_                    =	MCU_SA2
#define PAD_GPIOC3      (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[3]  ,1: GPIO                ,2: HDMI_CEC            ,3: SDMMC0_nRST         =	SA3		, GPIOC3_SD0nRST	, HDMI_CEC
#define PAD_GPIOC4      (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[4]  ,1: GPIO                ,2: UART1_DCD           ,3: SDMMC0_CARD_nint    =	SA4		, UART1_nDCD
#define PAD_GPIOC5      (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[5]  ,1: GPIO                ,2: UART1_CTS           ,3: SDMMC0_WP           =	SA5		, UART1_nCTS
#define PAD_GPIOC6      (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[6]  ,1: GPIO                ,2: UART1_RTS           ,3: SDMMC0_DETECT       =	SA6		, UART1_nRTS		, GPIOC6_SD0nCD
#define PAD_GPIOC7      (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[7]  ,1: GPIO                ,2: UART1_DSR           ,3: SDMMC1_nRST         =	SA7		, UART1_nDSR
#define PAD_GPIOC8      (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF  | PAD_STRENGTH_0)     // 0: MCUS_ADDR[8]  ,1: GPIO                ,2: UART1_DTR           ,3: SDMMC1_CARD_nint    =	SA8		, UART1_nDTR
#define PAD_GPIOC9      (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[9]  ,1: GPIO                ,2: SSP2_CLK_IO         ,3: PDM_STROBE          =	SA9		, SPI2_CLK			, PDM_STROBE
#define PAD_GPIOC10     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[10] ,1: GPIO                ,2: SSP2_FSS            ,3: MCUS_nNCS[2]        =	SA10	, SPI2_FRM
#define PAD_GPIOC11     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[11] ,1: GPIO                ,2: SSP2_RXD            ,3: USB2.0OTG0_DRVBUS   =	SA11	, SPI2_RX			, OTG_DVBUS
#define PAD_GPIOC12     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[12] ,1: GPIO                ,2: SSP2_TXD            ,3: SDMMC2_nRST         =	SA12	, SPI2_TX
#define PAD_GPIOC13     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[13] ,1: GPIO                ,2: PWM1_OUT            ,3: SDMMC2_CARD_nint    =	SA13	, GPIOC13_PWM1
#define PAD_GPIOC14     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[14] ,1: GPIO                ,2: PWM2_OUT            ,3: VIP0_ExtCLK2        =	SA14	, GPIOC14_MTV7		, GPIOC14_PWM2	, VID2_PCLK
#define PAD_GPIOC15     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[15] ,1: GPIO                ,2: MPEGTSI0_TSCLK      ,3: VIP0_HSYNC2         =	SA15	, SD2_DS			, TSI0_CLK		, VID2_HSYNC
#define PAD_GPIOC16     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[16] ,1: GPIO                ,2: MPEGTSI0_TSYNC0     ,3: VIP0_VSYNC2         =	SA16	, AC97IRQ			, TSI0_SYNC		, VID2_VSYNC
#define PAD_GPIOC17     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[17] ,1: GPIO                ,2: MPEGTSI0_TDP0       ,3: VIP0_VD2[0]         =	SA17	, TSI0_DP			, VID2_D0
#define PAD_GPIOC18     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[18] ,1: GPIO                ,2: SDMMC2_CCLK         ,3: VIP0_VD2[1]         =	SA18	, GPIOC18_MTV0		, SD2_CLK		, VID2_D1
#define PAD_GPIOC19     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[19] ,1: GPIO                ,2: SDMMC2_CMD          ,3: VIP0_VD2[2]         =	SA19	, GPIOC19_MTV1		, SD2_CMD		, VID2_D2
#define PAD_GPIOC20     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[20] ,1: GPIO                ,2: SDMMC2_CDATA[0]     ,3: VIP0_VD2[3]         =	SA20	, GPIOC20_MTV2		, SD2_D0		, VID2_D3
#define PAD_GPIOC21     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[21] ,1: GPIO                ,2: SDMMC2_CDATA[1]     ,3: VIP0_VD2[4]         =	SA21	, GPIOC21_MTV3		, SD2_D1		, VID2_D4
#define PAD_GPIOC22     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[22] ,1: GPIO                ,2: SDMMC2_CDATA[2]     ,3: VIP0_VD2[5]         =	SA22	, GPIOC22_MTV4		, SD2_D2		, VID2_D5
#define PAD_GPIOC23     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_ADDR[23] ,1: GPIO                ,2: SDMMC2_CDATA[3]     ,3: VIP0_VD2[6]         =	SA23	, GPIOC23_MTV5		, SD2_D3		, VID2_D6
#define PAD_GPIOC24     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_LATADDR  ,1: GPIO                ,2: SPDIFIN             ,3: VIP0_VD2[7]         =	GPIOC24_MTV6		, SPDIF_RX		, VID2_D7
#define PAD_GPIOC25     (PAD_MODE_ALT | PAD_FUNC_ALT2 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nSWAIT   ,1: GPIO                ,2: SPDIF_DATA          ,3:_                    =	nSWAIT	, SPDIF_TX
#define PAD_GPIOC26     (PAD_MODE_IN  | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_RDnWR    ,1: GPIO                ,2: PDM_DATA0           ,3:_                    =	GPIOC26_ETHERIRQ	, PDM_D0
#define PAD_GPIOC27     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nSDQM1   ,1: GPIO                ,2: PDM_DATA1           ,3:_                    =	GPIOC27_NANDWP		, PDM_D1
#define PAD_GPIOC28     (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: MCUS_nSCS[1]        ,2: UART1_TRI           ,3:_                    =	nSCS1				, UART1_nRIO
#define PAD_GPIOC29     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_CLKIO          ,2:_                    ,3:_                    =	SPI0_CLK			, GPIOC29_CAM2nRST
#define PAD_GPIOC30     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_FSS            ,2:_                    ,3:_                    =	SPI0_FRM			, GPIOC30_CAM0nRST
#define PAD_GPIOC31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_TXD            ,2:_                    ,3:_                    =	SPI0TX

/*------------------------------------------------------------------------------
 *	(GROUP_D)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOD0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: SSP0_RXD            ,2: PWM3_OUT            ,3:_                    =	PWM3, SPI0_RX
#define PAD_GPIOD1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: PWM0_OUT            ,2: MCUS_ADDR[25]       ,3:_                    =	PWM0, SA25
#define PAD_GPIOD2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2C0_SCL            ,2: UART4_SMCAYEN       ,3:_                    =	I2C0_SCL, ISO7816
#define PAD_GPIOD3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2C0_SDA            ,2: UART5_SMCAYEN       ,3:_                    =	I2C0_SDA, ISO7816
#define PAD_GPIOD4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2C1_SCL            ,2:_                    ,3:_                    =	I2C1_SCL, I2C2SCL_LOOP
#define PAD_GPIOD5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2C1_SDA            ,2:_                    ,3:_                    =	I2C1_SDA, I2C2SDA_LOOP
#define PAD_GPIOD6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2C2_SCL            ,2:_                    ,3:_                    =	I2C2_SCL, I2C2SCL_LOOP
#define PAD_GPIOD7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2C2_SDA            ,2:_                    ,3:_                    =	I2C2_SDA, I2C2SDA_LOOP
#define PAD_GPIOD8      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: PPM_IN              ,2:_                    ,3:_                    =	PPM_IR_REV, GPIOC14_PWM2
#define PAD_GPIOD9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_SDO            ,2: AC97_ACSDATAOUT     ,3:_                    =	I2S_DOUT, AC97_DOUT, I2S0_DOUT_LOOP
#define PAD_GPIOD10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_BCLK           ,2: AC97_ACBITCLK       ,3:_                    =	I2S_BCLK, AC97_BCLK, I2S0_BCLK_LOOP
#define PAD_GPIOD11     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_SDI            ,2: AC97_ACSDATAIN      ,3:_                    =	I2S_DIN, AC97_DIN, I2S0_DIN_LOOP
#define PAD_GPIOD12     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_LRCLK          ,2: AC97_ACSYNC         ,3:_                    =	I2S_LRCK, AC97_SYNC, I2S0_LRCK_LOOP
#define PAD_GPIOD13     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          ,1: I2S0_CODCLK         ,2: AC97_nACRESET       ,3:_                    =	I2S_MCLK, AC97_nRST, I2S0_MCLK_LOOP
#define PAD_GPIOD14     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART0RXD            ,2: UART1_SMCAYEN       ,3:_                    =	UART0_RX, ISO7816
#define PAD_GPIOD15     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART1RXD            ,2: UART2_SMCAYEN       ,3:_                    =	GPIOD15_SD2nWP, UART1_RX, ISO7816
#define PAD_GPIOD16     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART2RXD            ,2: CAN0_TX             ,3:_                    =	UART2_RX, UART3_TX_LOOP
#define PAD_GPIOD17     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART3RXD            ,2: CAN1_TX             ,3:_                    =	OTG_FLAGB, UART3_RX, UART2_TX_LOOP
#define PAD_GPIOD18     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART0TXD            ,2:_                    ,3: SDnCD2              =	UART0_TX, ISO7816, SD2_nWP
#define PAD_GPIOD19     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART1TXD            ,2:_                    ,3:_                    =	UART1_TX, ISO7816, SD2_nCD
#define PAD_GPIOD20     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART2TXD            ,2:_                    ,3:_                    =	GPIOD20_SD1nWP, UART2_TX, UART2_TX_LOOP
#define PAD_GPIOD21     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          ,1: UART3TXD            ,2:_                    ,3:_                    =	GPIOD21_SD1nCD, UART3_TX, UART3_TX_LOOP
#define PAD_GPIOD22     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC1_CCLK         ,2:_                    ,3:_                    =	SD1_CLK
#define PAD_GPIOD23     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC1_CMD          ,2:_                    ,3:_                    =	SD1_CMD
#define PAD_GPIOD24     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC1_CDATA[0]     ,2:_                    ,3:_                    =	SD1_D0
#define PAD_GPIOD25     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC1_CDATA[1]     ,2:_                    ,3:_                    =	SD1_D1
#define PAD_GPIOD26     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC1_CDATA[2]     ,2:_                    ,3:_                    =	SD1_D2
#define PAD_GPIOD27     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_1)     // 0: GPIO          ,1: SDMMC1_CDATA[3]     ,2:_                    ,3:_                    =	SD1_D3
#define PAD_GPIOD28     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[0]          ,2: MPEGTSI_TDATA1[0]   ,3: MCUS_ADDR[24]       =	GPD28, VID0_D0, TSI1_D0, SA24
#define PAD_GPIOD29     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[1]          ,2: MPEGTSI_TDATA1[1]   ,3:_                    =	VID0_D1, TSI1_D1
#define PAD_GPIOD30     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[2]          ,2: MPEGTSI_TDATA1[2]   ,3:_                    =	VID0_D2, TSI1_D2
#define PAD_GPIOD31     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_DN  | PAD_STRENGTH_0)     // 0: GPIO          ,1: VIP1_VD[3]          ,2: MPEGTSI_TDATA1[3]   ,3:_                    =	VID0_D3, TSI1_D3

/*------------------------------------------------------------------------------
 *	(GROUP_E)
 *
 *	0 bit           8 bit                   12 bit          16 bit              20 bit
 *	| PAD_MODE_XXX  | PAD_FUNC_ALT(0,1,2,3) | PAD_LEVEL_XXX | PAD_PULL_UP,OFF | PAD_STRENGTH_0,1,2,3
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOE0      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[4]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    = 	VID0_D4				, TSI1_D4
#define PAD_GPIOE1      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[5]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    = 	VID0_D5				, TSI1_D5
#define PAD_GPIOE2      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[6]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    = 	VID0_D6				, TSI1_D6
#define PAD_GPIOE3      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_VD[7]           ,2: MPEGTSI_TDATA1[0]   ,3:_                    = 	VID0_D7				, TSI1_D7
#define PAD_GPIOE4      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_ExtCLK          ,2: MPEGTSI_TCLK1       ,3:_                    = 	VID0_PCLK			, TSI1_CLK
#define PAD_GPIOE5      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP1_HSYNC           ,2: MPEGTSI_TSYNC1      ,3:_                    =	VID0_HSYNC			, TSI1_SYNC
#define PAD_GPIOE6      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: VIP_VSYNC            ,2: MPEGTSI_TDP1        ,3:_                    =	VID0_VSYNC			, TSI1_DP
#define PAD_GPIOE7      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_TXD[0]     ,2: VIP0_Ext_VSYNC      ,3:_                    =	VID1_VSYNC			, ITU601_VSYNC, RTL_TX0
#define PAD_GPIOE8      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_TXD[1]     ,2:_                    ,3:_                    =	GPIOE8_HP_DET		, RTL_TX1
#define PAD_GPIOE9      (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_TXD[2]     ,2:_                    ,3:_                    =	GPIOE9_I2S_IRQ		, RTL_TX2
#define PAD_GPIOE10     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_TXD[3]     ,2:_                    ,3:_                    =	GPIOE10_LCD_DISP	, RTL_TX3
#define PAD_GPIOE11     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_TXEN       ,2:_                    ,3:_                    =	GPIOE11_mTOUCH_INT	, RTL_TXEN
#define PAD_GPIOE12     (PAD_MODE_IN | PAD_FUNC_ALT0 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_TXER       ,2:_                    ,3:_                    =	GPIOE12_LCDGATE_EN
#define PAD_GPIOE13     (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_COL        ,2: VIP0_Ext_HSYNC      ,3:_                    =	VID1_HSYNC			, ITU601_HSYNC
#define PAD_GPIOE14     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_RXD[0]     ,2: SSP1_CLKIO          ,3:_                    =	SPI1_CLK			, RTL_RX0
#define PAD_GPIOE15     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_RXD[1]     ,2: SSP1_FSS            ,3:_                    =	SPI1_FRM			, RTL_RX1
#define PAD_GPIOE16     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_RXD[2]     ,2:_                    ,3:_                    =	GPIOE16_MIPIC_nRST	, RTL_RX2
#define PAD_GPIOE17     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_RXD[3]     ,2:_                    ,3:_                    =	GPIOE17_rTOUCH_nPIRQ, RTL_RX3
#define PAD_GPIOE18     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_CLK_RX         ,2: SSP1_RXD            ,3:_                    =	SPI1_RX				, RTL_RXCLK
#define PAD_GPIOE19     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_PHY_RX_DV      ,2: SSP1_TXD            ,3:_                    =	SPI1_TX				, RTL_RXDV
#define PAD_GPIOE20     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_GMII_MDC       ,2:_                    ,3:_                    =	GPIOE20_MIPID_EN	, RTL_MDC
#define PAD_GPIOE21     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_GMII_MDI       ,2:_                    ,3:_                    =	SD2_D4				, RTL_MDIO
#define PAD_GPIOE22     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_HIGH | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_RXER       ,2:_                    ,3:_                    =	GPIOE22_HSICnRST	, SD2_D5
#define PAD_GPIOE23     (PAD_MODE_OUT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_UP  | PAD_STRENGTH_0)     // 0: GPIO          1: GMAC0_PHY_CRS        ,2:_                    ,3:_                    =	SD2_D6				, GPIOE23_OTGPWREN
#define PAD_GPIOE24     (PAD_MODE_ALT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_3)     // 0: GPIO          1: GMAC0_GTX_CLK        ,2:_                    ,3:_                    =	RTL_GTXCLK			, SD2_D7
#define PAD_GPIOE25     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: nTRST         1: GPIO                 ,2:_                    ,3:_                    =	MCU_nTRST
#define PAD_GPIOE26     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TMS           1: GPIO                 ,2:_                    ,3:_                    =	MCU_TMS
#define PAD_GPIOE27     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TDI           1: GPIO                 ,2:_                    ,3:_                    =	MCU_TDI
#define PAD_GPIOE28     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TCLK          1: GPIO                 ,2:_                    ,3:_                    =	MCU_TCLK
#define PAD_GPIOE29     (PAD_MODE_ALT | PAD_FUNC_ALT0 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: TDO           1: GPIO                 ,2:_                    ,3:_                    =	MCU_TDO
#define PAD_GPIOE30     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nSOE     1: GPIO                 ,2:_                    ,3:_                    =	MCU_nSOE
#define PAD_GPIOE31     (PAD_MODE_OUT | PAD_FUNC_ALT1 | PAD_LEVEL_LOW  | PAD_PULL_OFF | PAD_STRENGTH_0)     // 0: MCUS_nSWE     1: GPIO                 ,2:_                    ,3:_                    =	MCU_nSWE

/*------------------------------------------------------------------------------
 *	(GROUPALV)
 *	0                     4                             8        12
 *	| MODE(IN/OUT/DETECT) | ALIVE OUT or ALIVE DETMODE0 | PullUp |
 *
 -----------------------------------------------------------------------------*/
#define PAD_GPIOALV0    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				// POWER_KEY
#define PAD_GPIOALV1    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				// SD0_CD
#define PAD_GPIOALV2    (PAD_MODE_OUT | PAD_LEVEL_HIGH | PAD_PULL_UP )				// DUT_RDY
#define PAD_GPIOALV3    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				// BASE_RDY
#define PAD_GPIOALV4    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				// PMIC INT
#define PAD_GPIOALV5    (PAD_MODE_IN  | PAD_LEVEL_LOW  | PAD_PULL_UP )				// BASE5

/*------------------------------------------------------------------------------
 *	GPIO LCD (Backlight Enable/PWM)
 */
#define	CFG_IO_LCD_PWR_ENB					((PAD_GPIO_E + 12) | PAD_FUNC_ALT0)		/* GPIO */

/*------------------------------------------------------------------------------
 *	GPIO EEPROM
 */
#define CFG_IO_SPI_EEPROM_WP				((PAD_GPIO_C + 27) | PAD_FUNC_ALT1)		/* GPIO */

/*------------------------------------------------------------------------------
 *  CAMERA Back Power Down
 */
#define CFG_IO_CAMERA_BACK_POWER_DOWN     ((PAD_GPIO_B + 9) | PAD_FUNC_ALT0) 		/* GPIO */

/*------------------------------------------------------------------------------
 *  CAMERA Front Power Down
 */
#define CFG_IO_CAMERA_FRONT_POWER_DOWN     ((PAD_GPIO_B + 24) | PAD_FUNC_ALT1)    /* GPIO */

/*------------------------------------------------------------------------------
 *  CAMERA Front Reset
 */
#define CFG_IO_CAMERA_FRONT_RESET                 ((PAD_GPIO_C + 29) | PAD_FUNC_ALT0)   /* GPIO */

/*------------------------------------------------------------------------------
 *  CAMERA Back Reset
 */
#define CFG_IO_CAMERA_BACK_RESET                 ((PAD_GPIO_E + 16) | PAD_FUNC_ALT0)   /* GPIO */




#endif	/* __CFG_GPIO_H__ */

