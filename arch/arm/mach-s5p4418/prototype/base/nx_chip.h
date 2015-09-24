
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
//	File		: nx_chip.h
//	Description	: support chip information
//	Author		: Goofy
//	Export		:
//	History		:
//      2012-06-27  Gamza remove OFFSET_OF_
//                        refer to PHY_BASEADDR_LIST( x ) in nx_prototype.h
//		2010.07.19	Hans modify to nxp-2120
//		2008.04.17	Goofy first draft.
//------------------------------------------------------------------------------



// SIMIO는 STACK_BASE에 설정한다.


//------------------------------------------------------------------------------
// BAGL RENDERING SETTING
//
// DRAM BASE ADDRESS : 0x80000000
// SIMIO BASE ADDRESS: 0x82000000
// INTC PORT NAME    : intr
// DMA PORT NAME     : dma
// is_unit_test =
//------------------------------------------------------------------------------


#ifndef __NX_CHIP_P2120_H__
#define __NX_CHIP_P2120_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// To remove following waring on RVDS compiler
//		Warning : #66-D: enumeration value is out of "int" range
//------------------------------------------------------------------------------
//#ifdef __arm
//#pragma diag_remark 66		// disable #66 warining
//#endif

//------------------------------------------------------------------------------
// Module declaration.
//------------------------------------------------------------------------------
//      NUMBER_OF_XXX_MODULE
//      PHY_BASEADDR_XXX_MODULE
//------------------------------------------------------------------------------
//	REF EXAMPLE
//	#define NUMBER_OF_SIMIO_MODULE     1
//	#define PHY_BASEADDR_SIMIO_MODULE  0x00100000
//
//	#define NUMBER_OF_OJTDMA_MODULE    1
//	#define PHY_BASEADDR_OJTDMA_MODULE 0xC0017000
//
//	#define NUMBER_OF_INTC_MODULE      1
//	#define PHY_BASEADDR_INTC_MODULE   0xC0010000
// config만 다를 경우, (ex) clkgen_cfg0, clkgen_cfg1 이럴 경우는 어떻게 해야 하는가..?


//------------------------------------------------------------------------------
// BAGL RENDERING
//------------------------------------------------------------------------------

#define NUMBER_OF_SIMIO_MODULE     1
#define PHY_BASEADDR_SIMIO_MODULE  0x82000000

#define NUMBER_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE		1
#define PHY_BASEADDR_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE		0x40000000

#define NUMBER_OF_MCUSTOP_MODULE		1
#define PHY_BASEADDR_MCUSTOP_MODULE		0xC0051000

#define NUMBER_OF_AXISRAM_MODULE		1
#define PHY_BASEADDR_AXISRAM_MODULE		0xFFFF0000

#define NUMBER_OF_DMA_MODULE		2
#define PHY_BASEADDR_DMA0_MODULE		0xC0000000


#define PHY_BASEADDR_DMA1_MODULE		0xC0001000

#define NUMBER_OF_DREX_MODULE		1
#define PHY_BASEADDR_DREX_MODULE_CH0_APB		0xC00E0000
#define PHY_BASEADDR_DREX_MODULE_CH1_APB		0xC00E1000

#define NUMBER_OF_CLKPWR_MODULE		1
#define PHY_BASEADDR_CLKPWR_MODULE		0xC0010000

#define NUMBER_OF_INTC_MODULE		1
#define PHY_BASEADDR_INTC_MODULE		0xC0002000

#define NUMBER_OF_pl01115_Uart_modem_MODULE		1
#define PHY_BASEADDR_pl01115_Uart_modem_MODULE		0xC00A0000

#define NUMBER_OF_UART_MODULE		2
#define PHY_BASEADDR_UART0_MODULE		0xC00A1000


#define PHY_BASEADDR_UART1_MODULE		0xC00A2000

#define NUMBER_OF_pl01115_Uart_nodma_MODULE		3
#define PHY_BASEADDR_pl01115_Uart_nodma0_MODULE		0xC00A3000


#define PHY_BASEADDR_pl01115_Uart_nodma1_MODULE		0xC006D000


#define PHY_BASEADDR_pl01115_Uart_nodma2_MODULE		0xC006F000

#define NUMBER_OF_SSP_MODULE		3
#define PHY_BASEADDR_SSP0_MODULE		0xC005B000


#define PHY_BASEADDR_SSP1_MODULE		0xC005C000


#define PHY_BASEADDR_SSP2_MODULE		0xC005F000

#define NUMBER_OF_I2C_MODULE		3
#define PHY_BASEADDR_I2C0_MODULE		0xC00A4000


#define PHY_BASEADDR_I2C1_MODULE		0xC00A5000


#define PHY_BASEADDR_I2C2_MODULE		0xC00A6000

#define NUMBER_OF_I2S_MODULE		3
#define PHY_BASEADDR_I2S0_MODULE		0xC0055000


#define PHY_BASEADDR_I2S1_MODULE		0xC0056000


#define PHY_BASEADDR_I2S2_MODULE		0xC0057000

#define NUMBER_OF_DEINTERLACE_MODULE		1
#define PHY_BASEADDR_DEINTERLACE_MODULE		0xC0065000

#define NUMBER_OF_SCALER_MODULE		1
#define PHY_BASEADDR_SCALER_MODULE		0xC0066000

#define NUMBER_OF_AC97_MODULE		1
#define PHY_BASEADDR_AC97_MODULE		0xC0058000

#define NUMBER_OF_SPDIFRX_MODULE		1
#define PHY_BASEADDR_SPDIFRX_MODULE		0xC005A000

#define NUMBER_OF_SPDIFTX_MODULE		1
#define PHY_BASEADDR_SPDIFTX_MODULE		0xC0059000

#define NUMBER_OF_TIMER_MODULE		1
#define PHY_BASEADDR_TIMER_MODULE		0xC0017000

#define NUMBER_OF_PWM_MODULE		1
#define PHY_BASEADDR_PWM_MODULE		0xC0018000

#define NUMBER_OF_CLKGEN_MODULE		40
#define PHY_BASEADDR_CLKGEN0_MODULE		0xC00BB000


#define PHY_BASEADDR_CLKGEN1_MODULE		0xC00BC000


#define PHY_BASEADDR_CLKGEN2_MODULE		0xC00BD000


#define PHY_BASEADDR_CLKGEN3_MODULE		0xC00BE000


#define PHY_BASEADDR_CLKGEN4_MODULE		0xC00BF000


#define PHY_BASEADDR_CLKGEN5_MODULE		0xC00C0000

#define NUMBER_OF_WDT_MODULE		1
#define PHY_BASEADDR_WDT_MODULE		0xC0019000

#define NUMBER_OF_MPEGTSI_MODULE		1
#define PHY_BASEADDR_MPEGTSI_MODULE		0xC005D000

#define NUMBER_OF_DISPLAYTOP_MODULE		1
#define PHY_BASEADDR_DISPLAYTOP_MODULE		0xC0100000

#define NUMBER_OF_VIP_MODULE		2
#if 0
#define PHY_BASEADDR_VIP0_MODULE		0xC0063000
#define PHY_BASEADDR_VIP1_MODULE		0xC0064000
#else
#define PHY_BASEADDR_VIP0_MODULE		0xC0064000
#define PHY_BASEADDR_VIP1_MODULE		0xC0063000
#endif

#define NUMBER_OF_MIPI_MODULE		1
#define PHY_BASEADDR_MIPI_MODULE		0xC00D0000

#define NUMBER_OF_MALI400_MODULE		1
#define PHY_BASEADDR_MALI400_MODULE		0xC0070000

#define NUMBER_OF_ADC_MODULE		1
#define PHY_BASEADDR_ADC_MODULE		0xC0053000

#define NUMBER_OF_PPM_MODULE		1
#define PHY_BASEADDR_PPM_MODULE		0xC0054000

#define NUMBER_OF_SDMMC_MODULE		3
#define PHY_BASEADDR_SDMMC0_MODULE		0xC0062000


#define PHY_BASEADDR_SDMMC1_MODULE		0xC0068000


#define PHY_BASEADDR_SDMMC2_MODULE		0xC0069000

#define NUMBER_OF_CODA960_MODULE		1
#define PHY_BASEADDR_CODA960_MODULE_APB0		0xC0080000
#define PHY_BASEADDR_CODA960_MODULE_APB1		0xC0081000
#define PHY_BASEADDR_CODA960_MODULE_APB2		0xC0082000
#define PHY_BASEADDR_CODA960_MODULE_APB3		0xC0083000

#define NUMBER_OF_DWC_GMAC_MODULE		1
#define PHY_BASEADDR_DWC_GMAC_MODULE_APB0		0xC0060000
#define PHY_BASEADDR_DWC_GMAC_MODULE_APB1		0xC0061000

#define NUMBER_OF_USB20OTG_MODULE		1
#define PHY_BASEADDR_USB20OTG_MODULE_AHBS0		0xC0040000
#define PHY_BASEADDR_USB20OTG_MODULE_APB		0xC006C000

#define NUMBER_OF_USB20HOST_MODULE		1
#define PHY_BASEADDR_USB20HOST_MODULE_EHCI_S_ABH		0xC0030000
#define PHY_BASEADDR_USB20HOST_MODULE_OHCI_S_ABH		0xC0020000
#define PHY_BASEADDR_USB20HOST_MODULE_APB		0xC006A000

#define NUMBER_OF_CAN_MODULE		2
#define PHY_BASEADDR_CAN0_MODULE		0xC00CE000


#define PHY_BASEADDR_CAN1_MODULE		0xC00CF000

#define NUMBER_OF_ECID_MODULE		1
#define PHY_BASEADDR_ECID_MODULE		0xC0067000

#define NUMBER_OF_RSTCON_MODULE		1
#define PHY_BASEADDR_RSTCON_MODULE		0xC0012000

#define NUMBER_OF_A3BM_AXI_TOP_MASTER_BUS_MODULE		1
#define PHY_BASEADDR_A3BM_AXI_TOP_MASTER_BUS_MODULE		0xC0052000

#define NUMBER_OF_A3BM_AXI_BOTTOM_MASTER_BUS_MODULE		1
#define PHY_BASEADDR_A3BM_AXI_BOTTOM_MASTER_BUS_MODULE_S3		0x0
#define PHY_BASEADDR_A3BM_AXI_BOTTOM_MASTER_BUS_MODULE_APB_BOTTOMBUS		0xC0050000

#define NUMBER_OF_A3BM_AXI_PERI_BUS_MODULE		1
#define PHY_BASEADDR_A3BM_AXI_PERI_BUS_MODULE		0xC0000000

#define NUMBER_OF_A3BM_AXI_DISPLAY_BUS_MODULE		1
#define PHY_BASEADDR_A3BM_AXI_DISPLAY_BUS_MODULE		0xC005E000

#define NUMBER_OF_GPIO_MODULE		5
#define PHY_BASEADDR_GPIOA_MODULE		0xC001A000


#define PHY_BASEADDR_GPIOB_MODULE		0xC001B000


#define PHY_BASEADDR_GPIOC_MODULE		0xC001C000


#define PHY_BASEADDR_GPIOD_MODULE		0xC001D000


#define PHY_BASEADDR_GPIOE_MODULE		0xC001E000

#define NUMBER_OF_CRYPTO_MODULE		1
#define PHY_BASEADDR_CRYPTO_MODULE		0xC0015000

#define NUMBER_OF_PDM_MODULE		1
#define PHY_BASEADDR_PDM_MODULE		0xC0014000

#define NUMBER_OF_TIEOFF_MODULE		1
#define PHY_BASEADDR_TIEOFF_MODULE		0xC0011000


#define PHY_BASEADDR_CLKGEN6_MODULE		0xC00AE000


#define PHY_BASEADDR_CLKGEN7_MODULE		0xC00AF000


#define PHY_BASEADDR_CLKGEN8_MODULE		0xC00B0000


#define PHY_BASEADDR_CLKGEN9_MODULE		0xC00CA000


#define PHY_BASEADDR_CLKGEN10_MODULE		0xC00C8000


#define PHY_BASEADDR_CLKGEN11_MODULE		0xC00B8000


#define PHY_BASEADDR_CLKGEN12_MODULE		0xC00B7000


#define PHY_BASEADDR_CLKGEN13_MODULE		0xC00BA000


#define PHY_BASEADDR_CLKGEN14_MODULE		0xC00B9000


#define PHY_BASEADDR_CLKGEN15_MODULE		0xC00B2000


#define PHY_BASEADDR_CLKGEN16_MODULE		0xC00B3000


#define PHY_BASEADDR_CLKGEN17_MODULE		0xC00B4000


#define PHY_BASEADDR_CLKGEN18_MODULE		0xC00C5000


#define PHY_BASEADDR_CLKGEN19_MODULE		0xC00CC000


#define PHY_BASEADDR_CLKGEN20_MODULE		0xC00CD000


#define PHY_BASEADDR_CLKGEN21_MODULE		0xC00C3000


#define PHY_BASEADDR_CLKGEN22_MODULE		0xC00A9000


#define PHY_BASEADDR_CLKGEN23_MODULE		0xC00AA000


#define PHY_BASEADDR_CLKGEN24_MODULE		0xC00A8000


#define PHY_BASEADDR_CLKGEN25_MODULE		0xC00AB000


#define PHY_BASEADDR_CLKGEN26_MODULE		0xC006E000


#define PHY_BASEADDR_CLKGEN27_MODULE		0xC0084000


#define PHY_BASEADDR_CLKGEN28_MODULE		0xC00B5000


#define PHY_BASEADDR_CLKGEN29_MODULE		0xC00C4000


#if 0
#define PHY_BASEADDR_CLKGEN30_MODULE		0xC00C1000
#define PHY_BASEADDR_CLKGEN31_MODULE		0xC00C2000
#else
#define PHY_BASEADDR_CLKGEN30_MODULE		0xC00C2000
#define PHY_BASEADDR_CLKGEN31_MODULE		0xC00C1000
#endif


#define PHY_BASEADDR_CLKGEN32_MODULE		0xC006B000


#define PHY_BASEADDR_CLKGEN33_MODULE		0xC00C7000


#define PHY_BASEADDR_CLKGEN34_MODULE		0xC00C6000


#define PHY_BASEADDR_CLKGEN35_MODULE		0xC00B6000


#define PHY_BASEADDR_CLKGEN36_MODULE		0xC00CB000


#define PHY_BASEADDR_CLKGEN37_MODULE		0xC00AC000


#define PHY_BASEADDR_CLKGEN38_MODULE		0xC00AD000


#define PHY_BASEADDR_CLKGEN39_MODULE		0xC00A7000
//------------------------------------------------------------------------------
// BAGL RENDERING END
//------------------------------------------------------------------------------

//#ifdef __arm	// for RVDS
//#pragma diag_default 66		// return to default setting for #66 warning
//#endif



//------------------------------------------------------------------------------
// Interrupt Number of mudules for the interrupt controller.
//------------------------------------------------------------------------------
//	REF EXAMPLE :
//	enum {
//		INTNUM_OF_OJTDMA_MODULE_IRQ		= 0
//		,INTNUM_OF_OJTDMA_MODULE_DMAIRQ	= 1
//		};





// NX_INTC_NUM_OF_INT
// Total Interrupt Number : Valid

#define NX_INTC_NUM_OF_INT	60

enum {
//------------------------------------------------------------------------------
// BAGL RENDERING
//------------------------------------------------------------------------------


INTNUM_OF_MCUSTOP_MODULE		= 0
,INTNUM_OF_DMA0_MODULE		= 1
,INTNUM_OF_DMA1_MODULE		= 2
,INTNUM_OF_CLKPWR_MODULE_INTREQPWR		= 3
,INTNUM_OF_CLKPWR_MODULE_ALIVEIRQ		= 4
,INTNUM_OF_CLKPWR_MODULE_RTCIRQ		= 5
,INTNUM_OF_pl01115_Uart_modem_MODULE		= 6
,INTNUM_OF_UART0_MODULE		= 7
,INTNUM_OF_UART1_MODULE		= 8
,INTNUM_OF_pl01115_Uart_nodma0_MODULE		= 9
,INTNUM_OF_pl01115_Uart_nodma1_MODULE		= 10
,INTNUM_OF_pl01115_Uart_nodma2_MODULE		= 11
,INTNUM_OF_SSP0_MODULE		= 12
,INTNUM_OF_SSP1_MODULE		= 13
,INTNUM_OF_SSP2_MODULE		= 14
,INTNUM_OF_I2C0_MODULE		= 15
,INTNUM_OF_I2C1_MODULE		= 16
,INTNUM_OF_I2C2_MODULE		= 17
,INTNUM_OF_DEINTERLACE_MODULE		= 18
,INTNUM_OF_SCALER_MODULE		= 19
,INTNUM_OF_AC97_MODULE		= 20
,INTNUM_OF_SPDIFRX_MODULE		= 21
,INTNUM_OF_SPDIFTX_MODULE		= 22
,INTNUM_OF_TIMER_MODULE_INT0		= 23
,INTNUM_OF_TIMER_MODULE_INT1		= 24
,INTNUM_OF_TIMER_MODULE_INT2		= 25
,INTNUM_OF_TIMER_MODULE_INT3		= 26
,INTNUM_OF_PWM_MODULE_INT0		= 27
,INTNUM_OF_PWM_MODULE_INT1		= 28
,INTNUM_OF_PWM_MODULE_INT2		= 29
,INTNUM_OF_PWM_MODULE_INT3		= 30
,INTNUM_OF_WDT_MODULE		= 31
,INTNUM_OF_MPEGTSI_MODULE		= 32
,INTNUM_OF_DISPLAYTOP_MODULE_DUALDISPLAY_PRIMIRQ		= 33
,INTNUM_OF_DISPLAYTOP_MODULE_DUALDISPLAY_SECONDIRQ		= 34
,INTNUM_OF_DISPLAYTOP_MODULE_RESCONV_IRQ		= 35
,INTNUM_OF_DISPLAYTOP_MODULE_HDMI_IRQ		= 36
#if 0
,INTNUM_OF_VIP0_MODULE		= 37
,INTNUM_OF_VIP1_MODULE		= 38
#else
,INTNUM_OF_VIP0_MODULE		= 38
,INTNUM_OF_VIP1_MODULE		= 37
#endif
,INTNUM_OF_MIPI_MODULE		= 39
,INTNUM_OF_MALI400_MODULE		= 40
,INTNUM_OF_ADC_MODULE		= 41
,INTNUM_OF_PPM_MODULE		= 42
,INTNUM_OF_SDMMC0_MODULE		= 43
,INTNUM_OF_SDMMC1_MODULE		= 44
,INTNUM_OF_SDMMC2_MODULE		= 45
,INTNUM_OF_CODA960_MODULE_HOST_INTRPT		= 46
,INTNUM_OF_CODA960_MODULE_JPG_INTRPT		= 47
,INTNUM_OF_DWC_GMAC_MODULE		= 48
,INTNUM_OF_USB20OTG_MODULE		= 49
,INTNUM_OF_USB20HOST_MODULE		= 50
,INTNUM_OF_CAN0_MODULE		= 51
,INTNUM_OF_CAN1_MODULE		= 52
,INTNUM_OF_GPIOA_MODULE		= 53
,INTNUM_OF_GPIOB_MODULE		= 54
,INTNUM_OF_GPIOC_MODULE		= 55
,INTNUM_OF_GPIOD_MODULE		= 56
,INTNUM_OF_GPIOE_MODULE		= 57
,INTNUM_OF_CRYPTO_MODULE		= 58
,INTNUM_OF_PDM_MODULE		= 59



//------------------------------------------------------------------------------
// BAGL RENDERING END
//------------------------------------------------------------------------------

};


//------------------------------------------------------------------------------
// Reset Controller : Number of Reset
//------------------------------------------------------------------------------
#define NUMBER_OF_RESET_MODULE_PIN 69
enum {
// xl50200_AC97_cfg0
RESETINDEX_OF_AC97_MODULE_PRESETn = 0
,// nx01301_CORTEXA9MP_TOP_QUADL2C
RESETINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE_nCPURESET1 = 1
,// nx01301_CORTEXA9MP_TOP_QUADL2C
RESETINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE_nCPURESET2 = 2
,// nx01301_CORTEXA9MP_TOP_QUADL2C
RESETINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE_nCPURESET3 = 3
,// nx01301_CORTEXA9MP_TOP_QUADL2C
RESETINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE_nWDRESET1 = 4
,// nx01301_CORTEXA9MP_TOP_QUADL2C
RESETINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE_nWDRESET2 = 5
,// nx01301_CORTEXA9MP_TOP_QUADL2C
RESETINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_MODULE_nWDRESET3 = 6
,// nx02600_CRYPTO_cfg0
RESETINDEX_OF_CRYPTO_MODULE_i_nRST = 7
,// nx01501_DEINTERLACE_cfg0
RESETINDEX_OF_DEINTERLACE_MODULE_i_nRST = 8
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_Top_nRST = 9
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_DualDisplay_nRST = 10
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_ResConv_nRST = 11
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_LCDIF_nRST = 12
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_nRST = 13
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_VIDEO_nRST = 14
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_SPDIF_nRST = 15
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_TMDS_nRST = 16
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_PHY_nRST = 17
,// nx71000_DisplayTop_cfg5
RESETINDEX_OF_DISPLAYTOP_MODULE_i_LVDS_nRST = 18
,// nx50100_ECID_128bit
RESETINDEX_OF_ECID_MODULE_i_nRST = 19
,// xl00100_I2C_cfg0
RESETINDEX_OF_I2C0_MODULE_PRESETn = 20
,// xl00100_I2C_cfg0
RESETINDEX_OF_I2C1_MODULE_PRESETn = 21
,// xl00100_I2C_cfg0
RESETINDEX_OF_I2C2_MODULE_PRESETn = 22
,// xl00300_I2S_cfg1
RESETINDEX_OF_I2S0_MODULE_PRESETn = 23
,// xl00300_I2S_cfg1
RESETINDEX_OF_I2S1_MODULE_PRESETn = 24
,// xl00300_I2S_cfg1
RESETINDEX_OF_I2S2_MODULE_PRESETn = 25
,// xl00112_DREX_cfg1
RESETINDEX_OF_DREX_MODULE_CRESETn = 26
,// xl00112_DREX_cfg1
RESETINDEX_OF_DREX_MODULE_ARESETn = 27
,// xl00112_DREX_cfg1
RESETINDEX_OF_DREX_MODULE_nPRST = 28
,// nx71100_mipi_cfg1
RESETINDEX_OF_MIPI_MODULE_i_nRST = 29
,// nx71100_mipi_cfg1
RESETINDEX_OF_MIPI_MODULE_i_DSI_I_PRESETn = 30
,// nx71100_mipi_cfg1
RESETINDEX_OF_MIPI_MODULE_i_CSI_I_PRESETn = 31
,// nx71100_mipi_cfg1
RESETINDEX_OF_MIPI_MODULE_i_PHY_S_RESETN = 32
,// nx71100_mipi_cfg1
RESETINDEX_OF_MIPI_MODULE_i_PHY_M_RESETN = 33
,// nx01910_MPEGTSI_cfg0
RESETINDEX_OF_MPEGTSI_MODULE_i_nRST = 34
,// nx02500_PDM_cfg0
RESETINDEX_OF_PDM_MODULE_i_nRST = 35
,// xl50010_PWMTIMER_usetimer
RESETINDEX_OF_TIMER_MODULE_PRESETn = 36
,// xl50010_PWMTIMER_usepwm
RESETINDEX_OF_PWM_MODULE_PRESETn = 37
,// nx01400_SCALER_cfg0
RESETINDEX_OF_SCALER_MODULE_i_nRST = 38
,// xl00500_SDMMC_cfg0
RESETINDEX_OF_SDMMC0_MODULE_i_nRST = 39
,// xl00500_SDMMC_cfg0
RESETINDEX_OF_SDMMC1_MODULE_i_nRST = 40
,// xl00500_SDMMC_cfg0
RESETINDEX_OF_SDMMC2_MODULE_i_nRST = 41
,// nx01600_SPDIFRX_cfg0
RESETINDEX_OF_SPDIFRX_MODULE_PRESETn = 42
,// xl50300_SPDIFTX_hdmipinout
RESETINDEX_OF_SPDIFTX_MODULE_PRESETn = 43
,// pl02212_Ssp_cfg1
RESETINDEX_OF_SSP0_MODULE_PRESETn = 44
,// pl02212_Ssp_cfg1
RESETINDEX_OF_SSP0_MODULE_nSSPRST = 45
,// pl02212_Ssp_cfg1
RESETINDEX_OF_SSP1_MODULE_PRESETn = 46
,// pl02212_Ssp_cfg1
RESETINDEX_OF_SSP1_MODULE_nSSPRST = 47
,// pl02212_Ssp_cfg1
RESETINDEX_OF_SSP2_MODULE_PRESETn = 48
,// pl02212_Ssp_cfg1
RESETINDEX_OF_SSP2_MODULE_nSSPRST = 49
,// pl01115_Uart_cfg0
RESETINDEX_OF_UART0_MODULE_nUARTRST = 50
,// pl01115_Uart_modem
RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST = 51
,// pl01115_Uart_cfg0
RESETINDEX_OF_UART1_MODULE_nUARTRST = 52
,// pl01115_Uart_nodma
RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST = 53
,// pl01115_Uart_nodma
RESETINDEX_OF_pl01115_Uart_nodma1_MODULE_nUARTRST = 54
,// pl01115_Uart_nodma
RESETINDEX_OF_pl01115_Uart_nodma2_MODULE_nUARTRST = 55
,// xl00700_USB20HOST_cfg0
RESETINDEX_OF_USB20HOST_MODULE_i_nRST = 56
,// xl00600_USB20OTG_cfg0
RESETINDEX_OF_USB20OTG_MODULE_i_nRST = 57
,// xl50500_WDT_cfg0
RESETINDEX_OF_WDT_MODULE_PRESETn = 58
,// xl50500_WDT_cfg0
RESETINDEX_OF_WDT_MODULE_nPOR = 59
,// nx01700_adc_cfg0
RESETINDEX_OF_ADC_MODULE_nRST = 60
,// xl07000_coda960_cfg1
RESETINDEX_OF_CODA960_MODULE_i_areset_n = 61
,// xl07000_coda960_cfg1
RESETINDEX_OF_CODA960_MODULE_i_preset_n = 62
,// xl07000_coda960_cfg1
RESETINDEX_OF_CODA960_MODULE_i_creset_n = 63
,// xl50400_DWC_GMAC_RGMII
RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i = 64
,// xl06000_mali400_cfg1
RESETINDEX_OF_MALI400_MODULE_nRST = 65
,// nx02300_ppm_cfg0
RESETINDEX_OF_PPM_MODULE_i_nRST = 66
#if 0
,// nx01800_vip_twopadwrapper
RESETINDEX_OF_VIP1_MODULE_i_nRST = 67
,// nx01800_vip_cfg3
RESETINDEX_OF_VIP0_MODULE_i_nRST = 68
#else
,// nx01800_vip_twopadwrapper
RESETINDEX_OF_VIP1_MODULE_i_nRST = 68
,// nx01800_vip_cfg3
RESETINDEX_OF_VIP0_MODULE_i_nRST = 67
#endif
};


//------------------------------------------------------------------------------
// Clock Generator : Module Index of Clock Generator
//------------------------------------------------------------------------------
#define NUMBER_OF_CLKGEN_MODULE 40
enum {
CLOCKINDEX_OF_Inst_TIMER01_MODULE = 0
,CLOCKINDEX_OF_Inst_TIMER02_MODULE = 1
,CLOCKINDEX_OF_Inst_TIMER03_MODULE = 2
,CLOCKINDEX_OF_Inst_PWM01_MODULE = 3
,CLOCKINDEX_OF_Inst_PWM02_MODULE = 4
,CLOCKINDEX_OF_Inst_PWM03_MODULE = 5
,CLOCKINDEX_OF_I2C0_MODULE = 6
,CLOCKINDEX_OF_I2C1_MODULE = 7
,CLOCKINDEX_OF_I2C2_MODULE = 8
,CLOCKINDEX_OF_MIPI_MODULE = 9
,CLOCKINDEX_OF_DWC_GMAC_MODULE = 10
,CLOCKINDEX_OF_SPDIFTX_MODULE = 11
,CLOCKINDEX_OF_MPEGTSI_MODULE = 12
,CLOCKINDEX_OF_PWM_MODULE = 13
,CLOCKINDEX_OF_TIMER_MODULE = 14
,CLOCKINDEX_OF_I2S0_MODULE = 15
,CLOCKINDEX_OF_I2S1_MODULE = 16
,CLOCKINDEX_OF_I2S2_MODULE = 17
,CLOCKINDEX_OF_SDMMC0_MODULE = 18
,CLOCKINDEX_OF_SDMMC1_MODULE = 19
,CLOCKINDEX_OF_SDMMC2_MODULE = 20
,CLOCKINDEX_OF_MALI400_MODULE = 21
,CLOCKINDEX_OF_UART0_MODULE = 22
,CLOCKINDEX_OF_UART1_MODULE = 23
,CLOCKINDEX_OF_pl01115_Uart_modem_MODULE = 24
,CLOCKINDEX_OF_pl01115_Uart_nodma0_MODULE = 25
,CLOCKINDEX_OF_pl01115_Uart_nodma1_MODULE = 26
,CLOCKINDEX_OF_pl01115_Uart_nodma2_MODULE = 27
,CLOCKINDEX_OF_DEINTERLACE_MODULE = 28
,CLOCKINDEX_OF_PPM_MODULE = 29
,CLOCKINDEX_OF_VIP0_MODULE = 30
,CLOCKINDEX_OF_VIP1_MODULE = 31
,CLOCKINDEX_OF_USB20HOST_MODULE = 32
,CLOCKINDEX_OF_CODA960_MODULE = 33
,CLOCKINDEX_OF_CRYPTO_MODULE = 34
,CLOCKINDEX_OF_SCALER_MODULE = 35
,CLOCKINDEX_OF_PDM_MODULE = 36
,CLOCKINDEX_OF_SSP0_MODULE = 37
,CLOCKINDEX_OF_SSP1_MODULE = 38
,CLOCKINDEX_OF_SSP2_MODULE = 39
};


//------------------------------------------------------------------------------
// DMA peripheral index of modules for the DMA controller.
//------------------------------------------------------------------------------



enum {
//------------------------------------------------------------------------------
// BAGL RENDERING
//------------------------------------------------------------------------------

DMAINDEX_OF_pl01115_Uart_modem_MODULE_UARTTXDMA		= 0
,DMAINDEX_OF_pl01115_Uart_modem_MODULE_UARTRXDMA		= 1
,DMAINDEX_OF_UART0_MODULE_UARTTXDMA		= 2
,DMAINDEX_OF_UART0_MODULE_UARTRXDMA		= 3
,DMAINDEX_OF_UART1_MODULE_UARTTXDMA		= 4
,DMAINDEX_OF_UART1_MODULE_UARTRXDMA		= 5
,DMAINDEX_OF_SSP0_MODULE_SSPTXDMA		= 6
,DMAINDEX_OF_SSP0_MODULE_SSPRXDMA		= 7
,DMAINDEX_OF_SSP1_MODULE_SSPTXDMA		= 8
,DMAINDEX_OF_SSP1_MODULE_SSPRXDMA		= 9
,DMAINDEX_OF_SSP2_MODULE_SSPTXDMA		= 10
,DMAINDEX_OF_SSP2_MODULE_SSPRXDMA		= 11
,DMAINDEX_OF_I2S0_MODULE_I2STXDMA		= 12
,DMAINDEX_OF_I2S0_MODULE_I2SRXDMA		= 13
,DMAINDEX_OF_I2S1_MODULE_I2STXDMA		= 14
,DMAINDEX_OF_I2S1_MODULE_I2SRXDMA		= 15
,DMAINDEX_OF_I2S2_MODULE_I2STXDMA		= 16
,DMAINDEX_OF_I2S2_MODULE_I2SRXDMA		= 17
,DMAINDEX_OF_AC97_MODULE_PCMOUTDMA		= 18
,DMAINDEX_OF_AC97_MODULE_PCMINDMA		= 19
,DMAINDEX_OF_AC97_MODULE_MICINDMA		= 20
,DMAINDEX_OF_SPDIFRX_MODULE		= 21
,DMAINDEX_OF_SPDIFTX_MODULE		= 22
,DMAINDEX_OF_MPEGTSI_MODULE_MPTSIDMA0		= 23
,DMAINDEX_OF_MPEGTSI_MODULE_MPTSIDMA1		= 24
,DMAINDEX_OF_MPEGTSI_MODULE_MPTSIDMA2		= 25
,DMAINDEX_OF_MPEGTSI_MODULE_MPTSIDMA3		= 26
,DMAINDEX_OF_CRYPTO_MODULE_CRYPDMA_BR		= 27
,DMAINDEX_OF_CRYPTO_MODULE_CRYPDMA_BW		= 28
,DMAINDEX_OF_CRYPTO_MODULE_CRYPDMA_HR		= 29
,DMAINDEX_OF_PDM_MODULE		= 30
//------------------------------------------------------------------------------
// BAGL RENDERING END
//------------------------------------------------------------------------------
};














//------------------------------------------------------------------------------
// PAD INFORMATION ( design의 PAD 부분을 보고 렌더링 한다. )
//
// #define 	PAD_CLKRST_MANAGER_i_BCLK				1
// #define 	PAD_SSP_SSPCLK				5
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// BAGL RENDERING
//------------------------------------------------------------------------------

#define	PADINDEX_OF_USB20OTG_i_IdPin				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20OTG_io_VBUS				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20OTG_io_DM				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20OTG_io_DP				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20OTG_io_RKELVIN				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20HOST_io_DP0				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20HOST_io_DM0				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20HOST_io_RKELVIN				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20HOST_io_STROBE1				( (0 << 16) | 0 )
#define	PADINDEX_OF_USB20HOST_io_DATA1				( (0 << 16) | 0 )
#define	PADINDEX_OF_GPIOA_GPIO_0_				( (1 << 16) | (0 << 8) | (0 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PADPrimVCLK				( (1 << 16) | (0 << 8) | (0 << 3) | 1 )
#define	PADINDEX_OF_CLKPWR_i_PADTESTMODE_4_				( (1 << 16) | (0 << 8) | (0 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_1_				( (1 << 16) | (0 << 8) | (1 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_0_				( (1 << 16) | (0 << 8) | (1 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_2_				( (1 << 16) | (0 << 8) | (2 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_1_				( (1 << 16) | (0 << 8) | (2 << 3) | 1 )
#define	PADINDEX_OF_CLKPWR_i_PADTESTMODE_0_				( (1 << 16) | (0 << 8) | (2 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_3_				( (1 << 16) | (0 << 8) | (3 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_2_				( (1 << 16) | (0 << 8) | (3 << 3) | 1 )
#define	PADINDEX_OF_CLKPWR_i_PADTESTMODE_1_				( (1 << 16) | (0 << 8) | (3 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_4_				( (1 << 16) | (0 << 8) | (4 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_3_				( (1 << 16) | (0 << 8) | (4 << 3) | 1 )
#define	PADINDEX_OF_CLKPWR_i_PADTESTMODE_2_				( (1 << 16) | (0 << 8) | (4 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_5_				( (1 << 16) | (0 << 8) | (5 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_4_				( (1 << 16) | (0 << 8) | (5 << 3) | 1 )
#define	PADINDEX_OF_CLKPWR_i_PADTESTMODE_3_				( (1 << 16) | (0 << 8) | (5 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_6_				( (1 << 16) | (0 << 8) | (6 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_5_				( (1 << 16) | (0 << 8) | (6 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_7_				( (1 << 16) | (0 << 8) | (7 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_6_				( (1 << 16) | (0 << 8) | (7 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_8_				( (1 << 16) | (0 << 8) | (8 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_7_				( (1 << 16) | (0 << 8) | (8 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_9_				( (1 << 16) | (0 << 8) | (9 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_8_				( (1 << 16) | (0 << 8) | (9 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_10_				( (1 << 16) | (0 << 8) | (10 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_9_				( (1 << 16) | (0 << 8) | (10 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_11_				( (1 << 16) | (0 << 8) | (11 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_10_				( (1 << 16) | (0 << 8) | (11 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_12_				( (1 << 16) | (0 << 8) | (12 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_11_				( (1 << 16) | (0 << 8) | (12 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_13_				( (1 << 16) | (0 << 8) | (13 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_12_				( (1 << 16) | (0 << 8) | (13 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_14_				( (1 << 16) | (0 << 8) | (14 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_13_				( (1 << 16) | (0 << 8) | (14 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_15_				( (1 << 16) | (0 << 8) | (15 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_14_				( (1 << 16) | (0 << 8) | (15 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_16_				( (1 << 16) | (0 << 8) | (16 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_15_				( (1 << 16) | (0 << 8) | (16 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_17_				( (1 << 16) | (0 << 8) | (17 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_16_				( (1 << 16) | (0 << 8) | (17 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_18_				( (1 << 16) | (0 << 8) | (18 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_17_				( (1 << 16) | (0 << 8) | (18 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_19_				( (1 << 16) | (0 << 8) | (19 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_18_				( (1 << 16) | (0 << 8) | (19 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_20_				( (1 << 16) | (0 << 8) | (20 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_19_				( (1 << 16) | (0 << 8) | (20 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_21_				( (1 << 16) | (0 << 8) | (21 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_20_				( (1 << 16) | (0 << 8) | (21 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_22_				( (1 << 16) | (0 << 8) | (22 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_21_				( (1 << 16) | (0 << 8) | (22 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_23_				( (1 << 16) | (0 << 8) | (23 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_22_				( (1 << 16) | (0 << 8) | (23 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_24_				( (1 << 16) | (0 << 8) | (24 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_DualDisplay_PrimPADRGB24_23_				( (1 << 16) | (0 << 8) | (24 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_25_				( (1 << 16) | (0 << 8) | (25 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADnVSync				( (1 << 16) | (0 << 8) | (25 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_26_				( (1 << 16) | (0 << 8) | (26 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADnHSync				( (1 << 16) | (0 << 8) | (26 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_27_				( (1 << 16) | (0 << 8) | (27 << 3) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_DualDisplay_PrimPADDE				( (1 << 16) | (0 << 8) | (27 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_28_				( (1 << 16) | (0 << 8) | (28 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_ExtCLK				( (1 << 16) | (0 << 8) | (28 << 3) | 1 )
#define	PADINDEX_OF_I2S2_I2SCODCLK				( (1 << 16) | (0 << 8) | (28 << 3) | 2 )
#define	PADINDEX_OF_I2S1_I2SCODCLK				( (1 << 16) | (0 << 8) | (28 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_30_				( (1 << 16) | (0 << 8) | (30 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_0_				( (1 << 16) | (0 << 8) | (30 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_0_				( (1 << 16) | (0 << 8) | (30 << 3) | 2 )
#define	PADINDEX_OF_I2S1_I2SBCLK				( (1 << 16) | (0 << 8) | (30 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_0_				( (1 << 16) | (1 << 8) | (0 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_1_				( (1 << 16) | (1 << 8) | (0 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_1_				( (1 << 16) | (1 << 8) | (0 << 3) | 2 )
#define	PADINDEX_OF_I2S1_I2SLRCLK				( (1 << 16) | (1 << 8) | (0 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_2_				( (1 << 16) | (1 << 8) | (2 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_2_				( (1 << 16) | (1 << 8) | (2 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_2_				( (1 << 16) | (1 << 8) | (2 << 3) | 2 )
#define	PADINDEX_OF_I2S2_I2SBCLK				( (1 << 16) | (1 << 8) | (2 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_4_				( (1 << 16) | (1 << 8) | (4 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_3_				( (1 << 16) | (1 << 8) | (4 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_3_				( (1 << 16) | (1 << 8) | (4 << 3) | 2 )
#define	PADINDEX_OF_I2S2_I2SLRCLK				( (1 << 16) | (1 << 8) | (4 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_6_				( (1 << 16) | (1 << 8) | (6 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_4_				( (1 << 16) | (1 << 8) | (6 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_4_				( (1 << 16) | (1 << 8) | (6 << 3) | 2 )
#define	PADINDEX_OF_I2S1_I2SSDO				( (1 << 16) | (1 << 8) | (6 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_8_				( (1 << 16) | (1 << 8) | (8 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_5_				( (1 << 16) | (1 << 8) | (8 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_5_				( (1 << 16) | (1 << 8) | (8 << 3) | 2 )
#define	PADINDEX_OF_I2S2_I2SSDO				( (1 << 16) | (1 << 8) | (8 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_9_				( (1 << 16) | (1 << 8) | (9 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_6_				( (1 << 16) | (1 << 8) | (9 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_6_				( (1 << 16) | (1 << 8) | (9 << 3) | 2 )
#define	PADINDEX_OF_I2S1_I2SSDI				( (1 << 16) | (1 << 8) | (9 << 3) | 3 )
#define	PADINDEX_OF_GPIOB_GPIO_10_				( (1 << 16) | (1 << 8) | (10 << 3) | 0 )
#define	PADINDEX_OF_VIP1_i_VD_7_				( (1 << 16) | (1 << 8) | (10 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SDEX_7_				( (1 << 16) | (1 << 8) | (10 << 3) | 2 )
#define	PADINDEX_OF_I2S2_I2SSDI				( (1 << 16) | (1 << 8) | (10 << 3) | 3 )
#define	PADINDEX_OF_GPIOA_GPIO_29_				( (1 << 16) | (0 << 8) | (29 << 3) | 0 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CCLK				( (1 << 16) | (0 << 8) | (29 << 3) | 1 )
#define	PADINDEX_OF_GPIOA_GPIO_31_				( (1 << 16) | (0 << 8) | (31 << 3) | 0 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CMD				( (1 << 16) | (0 << 8) | (31 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_1_				( (1 << 16) | (1 << 8) | (1 << 3) | 0 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CDATA_0_				( (1 << 16) | (1 << 8) | (1 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_3_				( (1 << 16) | (1 << 8) | (3 << 3) | 0 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CDATA_1_				( (1 << 16) | (1 << 8) | (3 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_5_				( (1 << 16) | (1 << 8) | (5 << 3) | 0 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CDATA_2_				( (1 << 16) | (1 << 8) | (5 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_7_				( (1 << 16) | (1 << 8) | (7 << 3) | 0 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CDATA_3_				( (1 << 16) | (1 << 8) | (7 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_CLE				( (1 << 16) | (1 << 8) | (11 << 3) | 0 )
#define	PADINDEX_OF_MCUSTOP_CLE1				( (1 << 16) | (1 << 8) | (11 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_11_				( (1 << 16) | (1 << 8) | (11 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_ALE				( (1 << 16) | (1 << 8) | (12 << 3) | 0 )
#define	PADINDEX_OF_MCUSTOP_ALE1				( (1 << 16) | (1 << 8) | (12 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_12_				( (1 << 16) | (1 << 8) | (12 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_0_				( (1 << 16) | (1 << 8) | (13 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_13_				( (1 << 16) | (1 << 8) | (13 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_RnB				( (1 << 16) | (1 << 8) | (14 << 3) | 0 )
#define	PADINDEX_OF_MCUSTOP_RnB1				( (1 << 16) | (1 << 8) | (14 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_14_				( (1 << 16) | (1 << 8) | (14 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_1_				( (1 << 16) | (1 << 8) | (15 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_15_				( (1 << 16) | (1 << 8) | (15 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_nNFOE				( (1 << 16) | (1 << 8) | (16 << 3) | 0 )
#define	PADINDEX_OF_MCUSTOP_nNFOE1				( (1 << 16) | (1 << 8) | (16 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_16_				( (1 << 16) | (1 << 8) | (16 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_2_				( (1 << 16) | (1 << 8) | (17 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_17_				( (1 << 16) | (1 << 8) | (17 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_nNFWE				( (1 << 16) | (1 << 8) | (18 << 3) | 0 )
#define	PADINDEX_OF_MCUSTOP_nNFWE1				( (1 << 16) | (1 << 8) | (18 << 3) | 1 )
#define	PADINDEX_OF_GPIOB_GPIO_18_				( (1 << 16) | (1 << 8) | (18 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_3_				( (1 << 16) | (1 << 8) | (19 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_19_				( (1 << 16) | (1 << 8) | (19 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_nNCS_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_MCUSTOP_SD_4_				( (1 << 16) | (1 << 8) | (20 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_20_				( (1 << 16) | (1 << 8) | (20 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_nNCS_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_MCUSTOP_SD_5_				( (1 << 16) | (1 << 8) | (21 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_21_				( (1 << 16) | (1 << 8) | (21 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SD_6_				( (1 << 16) | (1 << 8) | (22 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_22_				( (1 << 16) | (1 << 8) | (22 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SD_7_				( (1 << 16) | (1 << 8) | (23 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_23_				( (1 << 16) | (1 << 8) | (23 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_SD_8_				( (1 << 16) | (1 << 8) | (24 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_24_				( (1 << 16) | (1 << 8) | (24 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_0_				( (1 << 16) | (1 << 8) | (24 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_9_				( (1 << 16) | (1 << 8) | (25 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_25_				( (1 << 16) | (1 << 8) | (25 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_1_				( (1 << 16) | (1 << 8) | (25 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_10_				( (1 << 16) | (1 << 8) | (26 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_26_				( (1 << 16) | (1 << 8) | (26 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_2_				( (1 << 16) | (1 << 8) | (26 << 3) | 2 )
#define	PADINDEX_OF_ECID_PAD_BONDING_ID_2_				( (1 << 16) | (1 << 8) | (26 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_SD_11_				( (1 << 16) | (1 << 8) | (27 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_27_				( (1 << 16) | (1 << 8) | (27 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_3_				( (1 << 16) | (1 << 8) | (27 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_SD_12_				( (1 << 16) | (1 << 8) | (28 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_28_				( (1 << 16) | (1 << 8) | (28 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_4_				( (1 << 16) | (1 << 8) | (28 << 3) | 2 )
#define	PADINDEX_OF_pl01115_Uart_nodma1_UARTRXD				( (1 << 16) | (1 << 8) | (28 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_SD_13_				( (1 << 16) | (1 << 8) | (29 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_29_				( (1 << 16) | (1 << 8) | (29 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_5_				( (1 << 16) | (1 << 8) | (29 << 3) | 2 )
#define	PADINDEX_OF_pl01115_Uart_nodma1_UARTTXD				( (1 << 16) | (1 << 8) | (29 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_SD_14_				( (1 << 16) | (1 << 8) | (30 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_30_				( (1 << 16) | (1 << 8) | (30 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_6_				( (1 << 16) | (1 << 8) | (30 << 3) | 2 )
#define	PADINDEX_OF_pl01115_Uart_nodma2_UARTRXD				( (1 << 16) | (1 << 8) | (30 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_SD_15_				( (1 << 16) | (1 << 8) | (31 << 3) | 0 )
#define	PADINDEX_OF_GPIOB_GPIO_31_				( (1 << 16) | (1 << 8) | (31 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA0_7_				( (1 << 16) | (1 << 8) | (31 << 3) | 2 )
#define	PADINDEX_OF_pl01115_Uart_nodma2_UARTTXD				( (1 << 16) | (1 << 8) | (31 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_0_				( (1 << 16) | (2 << 8) | (0 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_0_				( (1 << 16) | (2 << 8) | (0 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_1_				( (1 << 16) | (2 << 8) | (1 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_1_				( (1 << 16) | (2 << 8) | (1 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_2_				( (1 << 16) | (2 << 8) | (2 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_2_				( (1 << 16) | (2 << 8) | (2 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_3_				( (1 << 16) | (2 << 8) | (3 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_3_				( (1 << 16) | (2 << 8) | (3 << 3) | 1 )
#define	PADINDEX_OF_DISPLAYTOP_io_HDMI_CEC				( (1 << 16) | (2 << 8) | (3 << 3) | 2 )
#define	PADINDEX_OF_SDMMC0_SDMMC_nRST				( (1 << 16) | (2 << 8) | (3 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_4_				( (1 << 16) | (2 << 8) | (4 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_4_				( (1 << 16) | (2 << 8) | (4 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_nUARTDCD				( (1 << 16) | (2 << 8) | (4 << 3) | 2 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CARD_nInt				( (1 << 16) | (2 << 8) | (4 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_5_				( (1 << 16) | (2 << 8) | (5 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_5_				( (1 << 16) | (2 << 8) | (5 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_nUARTCTS				( (1 << 16) | (2 << 8) | (5 << 3) | 2 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CARD_WritePrt				( (1 << 16) | (2 << 8) | (5 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_6_				( (1 << 16) | (2 << 8) | (6 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_6_				( (1 << 16) | (2 << 8) | (6 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_nUARTRTS				( (1 << 16) | (2 << 8) | (6 << 3) | 2 )
#define	PADINDEX_OF_SDMMC0_SDMMC_CARD_nDetect				( (1 << 16) | (2 << 8) | (6 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_7_				( (1 << 16) | (2 << 8) | (7 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_7_				( (1 << 16) | (2 << 8) | (7 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_nUARTDSR				( (1 << 16) | (2 << 8) | (7 << 3) | 2 )
#define	PADINDEX_OF_SDMMC1_SDMMC_nRST				( (1 << 16) | (2 << 8) | (7 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_8_				( (1 << 16) | (2 << 8) | (8 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_8_				( (1 << 16) | (2 << 8) | (8 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_nUARTDTR				( (1 << 16) | (2 << 8) | (8 << 3) | 2 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CARD_nInt				( (1 << 16) | (2 << 8) | (8 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_9_				( (1 << 16) | (2 << 8) | (9 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_9_				( (1 << 16) | (2 << 8) | (9 << 3) | 1 )
#define	PADINDEX_OF_SSP2_SSPCLK_IO				( (1 << 16) | (2 << 8) | (9 << 3) | 2 )
#define	PADINDEX_OF_PDM_o_Strobe				( (1 << 16) | (2 << 8) | (9 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_10_				( (1 << 16) | (2 << 8) | (10 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_10_				( (1 << 16) | (2 << 8) | (10 << 3) | 1 )
#define	PADINDEX_OF_SSP2_SSPFSS				( (1 << 16) | (2 << 8) | (10 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_nNCS_2_				( (1 << 16) | (2 << 8) | (10 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_11_				( (1 << 16) | (2 << 8) | (11 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_11_				( (1 << 16) | (2 << 8) | (11 << 3) | 1 )
#define	PADINDEX_OF_SSP2_SSPRXD				( (1 << 16) | (2 << 8) | (11 << 3) | 2 )
#define	PADINDEX_OF_USB20OTG_o_DrvVBUS				( (1 << 16) | (2 << 8) | (11 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_12_				( (1 << 16) | (2 << 8) | (12 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_12_				( (1 << 16) | (2 << 8) | (12 << 3) | 1 )
#define	PADINDEX_OF_SSP2_SSPTXD				( (1 << 16) | (2 << 8) | (12 << 3) | 2 )
#define	PADINDEX_OF_SDMMC2_SDMMC_nRST				( (1 << 16) | (2 << 8) | (12 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_13_				( (1 << 16) | (2 << 8) | (13 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_13_				( (1 << 16) | (2 << 8) | (13 << 3) | 1 )
#define	PADINDEX_OF_PWM_TOUT1				( (1 << 16) | (2 << 8) | (13 << 3) | 2 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CARD_nInt				( (1 << 16) | (2 << 8) | (13 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_18_				( (1 << 16) | (3 << 8) | (18 << 3) | 0 )
#define	PADINDEX_OF_UART0_UARTTXD				( (1 << 16) | (3 << 8) | (18 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_nodma0_SMCAYEN				( (1 << 16) | (3 << 8) | (18 << 3) | 2 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CARD_WritePrt				( (1 << 16) | (3 << 8) | (18 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_19_				( (1 << 16) | (3 << 8) | (19 << 3) | 0 )
#define	PADINDEX_OF_pl01115_Uart_modem_UARTTXD				( (1 << 16) | (3 << 8) | (19 << 3) | 1 )
#define	PADINDEX_OF_UART0_SMCAYEN				( (1 << 16) | (3 << 8) | (19 << 3) | 2 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CARD_nDetect				( (1 << 16) | (3 << 8) | (19 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_20_				( (1 << 16) | (3 << 8) | (20 << 3) | 0 )
#define	PADINDEX_OF_UART1_UARTTXD				( (1 << 16) | (3 << 8) | (20 << 3) | 1 )
#define	PADINDEX_OF_CAN0_o_TX				( (1 << 16) | (3 << 8) | (20 << 3) | 2 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CARD_WritePrt				( (1 << 16) | (3 << 8) | (20 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_21_				( (1 << 16) | (3 << 8) | (21 << 3) | 0 )
#define	PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD				( (1 << 16) | (3 << 8) | (21 << 3) | 1 )
#define	PADINDEX_OF_CAN1_o_TX				( (1 << 16) | (3 << 8) | (21 << 3) | 2 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CARD_nDetect				( (1 << 16) | (3 << 8) | (21 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_22_				( (1 << 16) | (3 << 8) | (22 << 3) | 0 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CCLK				( (1 << 16) | (3 << 8) | (22 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_23_				( (1 << 16) | (3 << 8) | (23 << 3) | 0 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CMD				( (1 << 16) | (3 << 8) | (23 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_24_				( (1 << 16) | (3 << 8) | (24 << 3) | 0 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CDATA_0_				( (1 << 16) | (3 << 8) | (24 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_25_				( (1 << 16) | (3 << 8) | (25 << 3) | 0 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CDATA_1_				( (1 << 16) | (3 << 8) | (25 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_26_				( (1 << 16) | (3 << 8) | (26 << 3) | 0 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CDATA_2_				( (1 << 16) | (3 << 8) | (26 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_27_				( (1 << 16) | (3 << 8) | (27 << 3) | 0 )
#define	PADINDEX_OF_SDMMC1_SDMMC_CDATA_3_				( (1 << 16) | (3 << 8) | (27 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_nSWAIT				( (1 << 16) | (2 << 8) | (25 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_25_				( (1 << 16) | (2 << 8) | (25 << 3) | 1 )
#define	PADINDEX_OF_SPDIFTX_SPDIF_DATA				( (1 << 16) | (2 << 8) | (25 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_nSOE				( (1 << 16) | (4 << 8) | (30 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_30_				( (1 << 16) | (4 << 8) | (30 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_nSWE				( (1 << 16) | (4 << 8) | (31 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_31_				( (1 << 16) | (4 << 8) | (31 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_RDnWR				( (1 << 16) | (2 << 8) | (26 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_26_				( (1 << 16) | (2 << 8) | (26 << 3) | 1 )
#define	PADINDEX_OF_PDM_i_Data0				( (1 << 16) | (2 << 8) | (26 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_nSDQM1				( (1 << 16) | (2 << 8) | (27 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_27_				( (1 << 16) | (2 << 8) | (27 << 3) | 1 )
#define	PADINDEX_OF_PDM_i_Data1				( (1 << 16) | (2 << 8) | (27 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_nSCS_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_28_				( (1 << 16) | (2 << 8) | (28 << 3) | 0 )
#define	PADINDEX_OF_MCUSTOP_nSCS_1_				( (1 << 16) | (2 << 8) | (28 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_nUARTRI				( (1 << 16) | (2 << 8) | (28 << 3) | 2 )
#define	PADINDEX_OF_GPIOC_GPIO_29_				( (1 << 16) | (2 << 8) | (29 << 3) | 0 )
#define	PADINDEX_OF_SSP0_SSPCLK_IO				( (1 << 16) | (2 << 8) | (29 << 3) | 1 )
#define	PADINDEX_OF_GPIOC_GPIO_30_				( (1 << 16) | (2 << 8) | (30 << 3) | 0 )
#define	PADINDEX_OF_SSP0_SSPFSS				( (1 << 16) | (2 << 8) | (30 << 3) | 1 )
#define	PADINDEX_OF_GPIOC_GPIO_31_				( (1 << 16) | (2 << 8) | (31 << 3) | 0 )
#define	PADINDEX_OF_SSP0_SSPTXD				( (1 << 16) | (2 << 8) | (31 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_0_				( (1 << 16) | (3 << 8) | (0 << 3) | 0 )
#define	PADINDEX_OF_SSP0_SSPRXD				( (1 << 16) | (3 << 8) | (0 << 3) | 1 )
#define	PADINDEX_OF_PWM_TOUT3				( (1 << 16) | (3 << 8) | (0 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_1_				( (1 << 16) | (3 << 8) | (1 << 3) | 0 )
#define	PADINDEX_OF_PWM_TOUT0				( (1 << 16) | (3 << 8) | (1 << 3) | 1 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_25_				( (1 << 16) | (3 << 8) | (1 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_2_				( (1 << 16) | (3 << 8) | (2 << 3) | 0 )
#define	PADINDEX_OF_I2C0_SCL				( (1 << 16) | (3 << 8) | (2 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_nodma1_SMCAYEN				( (1 << 16) | (3 << 8) | (2 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_3_				( (1 << 16) | (3 << 8) | (3 << 3) | 0 )
#define	PADINDEX_OF_I2C0_SDA				( (1 << 16) | (3 << 8) | (3 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_nodma2_SMCAYEN				( (1 << 16) | (3 << 8) | (3 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_4_				( (1 << 16) | (3 << 8) | (4 << 3) | 0 )
#define	PADINDEX_OF_I2C1_SCL				( (1 << 16) | (3 << 8) | (4 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_5_				( (1 << 16) | (3 << 8) | (5 << 3) | 0 )
#define	PADINDEX_OF_I2C1_SDA				( (1 << 16) | (3 << 8) | (5 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_6_				( (1 << 16) | (3 << 8) | (6 << 3) | 0 )
#define	PADINDEX_OF_I2C2_SCL				( (1 << 16) | (3 << 8) | (6 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_7_				( (1 << 16) | (3 << 8) | (7 << 3) | 0 )
#define	PADINDEX_OF_I2C2_SDA				( (1 << 16) | (3 << 8) | (7 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_8_				( (1 << 16) | (3 << 8) | (8 << 3) | 0 )
#define	PADINDEX_OF_PPM_i_PPMIn				( (1 << 16) | (3 << 8) | (8 << 3) | 1 )
#define	PADINDEX_OF_GPIOD_GPIO_9_				( (1 << 16) | (3 << 8) | (9 << 3) | 0 )
#define	PADINDEX_OF_I2S0_I2SSDO				( (1 << 16) | (3 << 8) | (9 << 3) | 1 )
#define	PADINDEX_OF_AC97_ACSDATAOUT				( (1 << 16) | (3 << 8) | (9 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_10_				( (1 << 16) | (3 << 8) | (10 << 3) | 0 )
#define	PADINDEX_OF_I2S0_I2SBCLK				( (1 << 16) | (3 << 8) | (10 << 3) | 1 )
#define	PADINDEX_OF_AC97_ACBITCLK				( (1 << 16) | (3 << 8) | (10 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_11_				( (1 << 16) | (3 << 8) | (11 << 3) | 0 )
#define	PADINDEX_OF_I2S0_I2SSDI				( (1 << 16) | (3 << 8) | (11 << 3) | 1 )
#define	PADINDEX_OF_AC97_ACSDATAIN				( (1 << 16) | (3 << 8) | (11 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_12_				( (1 << 16) | (3 << 8) | (12 << 3) | 0 )
#define	PADINDEX_OF_I2S0_I2SLRCLK				( (1 << 16) | (3 << 8) | (12 << 3) | 1 )
#define	PADINDEX_OF_AC97_ACSYNC				( (1 << 16) | (3 << 8) | (12 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_13_				( (1 << 16) | (3 << 8) | (13 << 3) | 0 )
#define	PADINDEX_OF_I2S0_I2SCODCLK				( (1 << 16) | (3 << 8) | (13 << 3) | 1 )
#define	PADINDEX_OF_AC97_nACRESET				( (1 << 16) | (3 << 8) | (13 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_14_				( (1 << 16) | (3 << 8) | (14 << 3) | 0 )
#define	PADINDEX_OF_UART0_UARTRXD				( (1 << 16) | (3 << 8) | (14 << 3) | 1 )
#define	PADINDEX_OF_pl01115_Uart_modem_SMCAYEN				( (1 << 16) | (3 << 8) | (14 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_15_				( (1 << 16) | (3 << 8) | (15 << 3) | 0 )
#define	PADINDEX_OF_pl01115_Uart_modem_UARTRXD				( (1 << 16) | (3 << 8) | (15 << 3) | 1 )
#define	PADINDEX_OF_UART1_SMCAYEN				( (1 << 16) | (3 << 8) | (15 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_16_				( (1 << 16) | (3 << 8) | (16 << 3) | 0 )
#define	PADINDEX_OF_UART1_UARTRXD				( (1 << 16) | (3 << 8) | (16 << 3) | 1 )
#define	PADINDEX_OF_CAN0_i_RX				( (1 << 16) | (3 << 8) | (16 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_17_				( (1 << 16) | (3 << 8) | (17 << 3) | 0 )
#define	PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD				( (1 << 16) | (3 << 8) | (17 << 3) | 1 )
#define	PADINDEX_OF_CAN1_i_RX				( (1 << 16) | (3 << 8) | (17 << 3) | 2 )
#define	PADINDEX_OF_USB20OTG_i_VBUS				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_i_HDMI_hotplug_5V				( (0 << 16) | 0 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_14_				( (1 << 16) | (2 << 8) | (14 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_14_				( (1 << 16) | (2 << 8) | (14 << 3) | 1 )
#define	PADINDEX_OF_PWM_TOUT2				( (1 << 16) | (2 << 8) | (14 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_ExtCLK2				( (1 << 16) | (2 << 8) | (14 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_15_				( (1 << 16) | (2 << 8) | (15 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_15_				( (1 << 16) | (2 << 8) | (15 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TSCLK0				( (1 << 16) | (2 << 8) | (15 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_ExtHSYNC2				( (1 << 16) | (2 << 8) | (15 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_16_				( (1 << 16) | (2 << 8) | (16 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_16_				( (1 << 16) | (2 << 8) | (16 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TSYNC0				( (1 << 16) | (2 << 8) | (16 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_ExtVSYNC2				( (1 << 16) | (2 << 8) | (16 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_17_				( (1 << 16) | (2 << 8) | (17 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_17_				( (1 << 16) | (2 << 8) | (17 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDP0				( (1 << 16) | (2 << 8) | (17 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_0_				( (1 << 16) | (2 << 8) | (17 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_18_				( (1 << 16) | (2 << 8) | (18 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_18_				( (1 << 16) | (2 << 8) | (18 << 3) | 1 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CCLK				( (1 << 16) | (2 << 8) | (18 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_1_				( (1 << 16) | (2 << 8) | (18 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_19_				( (1 << 16) | (2 << 8) | (19 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_19_				( (1 << 16) | (2 << 8) | (19 << 3) | 1 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CMD				( (1 << 16) | (2 << 8) | (19 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_2_				( (1 << 16) | (2 << 8) | (19 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_20_				( (1 << 16) | (2 << 8) | (20 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_20_				( (1 << 16) | (2 << 8) | (20 << 3) | 1 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CDATA_0_				( (1 << 16) | (2 << 8) | (20 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_3_				( (1 << 16) | (2 << 8) | (20 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_21_				( (1 << 16) | (2 << 8) | (21 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_21_				( (1 << 16) | (2 << 8) | (21 << 3) | 1 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CDATA_1_				( (1 << 16) | (2 << 8) | (21 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_4_				( (1 << 16) | (2 << 8) | (21 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_22_				( (1 << 16) | (2 << 8) | (22 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_22_				( (1 << 16) | (2 << 8) | (22 << 3) | 1 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CDATA_2_				( (1 << 16) | (2 << 8) | (22 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_5_				( (1 << 16) | (2 << 8) | (22 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_23_				( (1 << 16) | (2 << 8) | (23 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_23_				( (1 << 16) | (2 << 8) | (23 << 3) | 1 )
#define	PADINDEX_OF_SDMMC2_SDMMC_CDATA_3_				( (1 << 16) | (2 << 8) | (23 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_6_				( (1 << 16) | (2 << 8) | (23 << 3) | 3 )
#define	PADINDEX_OF_MCUSTOP_LATADDR				( (1 << 16) | (2 << 8) | (24 << 3) | 0 )
#define	PADINDEX_OF_GPIOC_GPIO_24_				( (1 << 16) | (2 << 8) | (24 << 3) | 1 )
#define	PADINDEX_OF_SPDIFRX_SPDIFIN				( (1 << 16) | (2 << 8) | (24 << 3) | 2 )
#define	PADINDEX_OF_VIP1_i_VD2_7_				( (1 << 16) | (2 << 8) | (24 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_28_				( (1 << 16) | (3 << 8) | (28 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_0_				( (1 << 16) | (3 << 8) | (28 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_0_				( (1 << 16) | (3 << 8) | (28 << 3) | 2 )
#define	PADINDEX_OF_MCUSTOP_o_ADDR_24_				( (1 << 16) | (3 << 8) | (28 << 3) | 3 )
#define	PADINDEX_OF_GPIOD_GPIO_29_				( (1 << 16) | (3 << 8) | (29 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_1_				( (1 << 16) | (3 << 8) | (29 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_1_				( (1 << 16) | (3 << 8) | (29 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_30_				( (1 << 16) | (3 << 8) | (30 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_2_				( (1 << 16) | (3 << 8) | (30 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_2_				( (1 << 16) | (3 << 8) | (30 << 3) | 2 )
#define	PADINDEX_OF_GPIOD_GPIO_31_				( (1 << 16) | (3 << 8) | (31 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_3_				( (1 << 16) | (3 << 8) | (31 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_3_				( (1 << 16) | (3 << 8) | (31 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_0_				( (1 << 16) | (4 << 8) | (0 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_4_				( (1 << 16) | (4 << 8) | (0 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_4_				( (1 << 16) | (4 << 8) | (0 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_1_				( (1 << 16) | (4 << 8) | (1 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_5_				( (1 << 16) | (4 << 8) | (1 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_5_				( (1 << 16) | (4 << 8) | (1 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_2_				( (1 << 16) | (4 << 8) | (2 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_6_				( (1 << 16) | (4 << 8) | (2 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_6_				( (1 << 16) | (4 << 8) | (2 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_3_				( (1 << 16) | (4 << 8) | (3 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_VD_7_				( (1 << 16) | (4 << 8) | (3 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDATA1_7_				( (1 << 16) | (4 << 8) | (3 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_4_				( (1 << 16) | (4 << 8) | (4 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_ExtCLK				( (1 << 16) | (4 << 8) | (4 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TSCLK1				( (1 << 16) | (4 << 8) | (4 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_5_				( (1 << 16) | (4 << 8) | (5 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_ExtHSYNC				( (1 << 16) | (4 << 8) | (5 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TSYNC1				( (1 << 16) | (4 << 8) | (5 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_6_				( (1 << 16) | (4 << 8) | (6 << 3) | 0 )
#define	PADINDEX_OF_VIP0_i_ExtVSYNC				( (1 << 16) | (4 << 8) | (6 << 3) | 1 )
#define	PADINDEX_OF_MPEGTSI_TDP1				( (1 << 16) | (4 << 8) | (6 << 3) | 2 )
#define	PADINDEX_OF_ECID_PAD_EFUSE_FSOURCE				( (0 << 16) | 0 )
#define	PADINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_nTRST				( (1 << 16) | (4 << 8) | (25 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_25_				( (1 << 16) | (4 << 8) | (25 << 3) | 1 )
#define	PADINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_TMS				( (1 << 16) | (4 << 8) | (26 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_26_				( (1 << 16) | (4 << 8) | (26 << 3) | 1 )
#define	PADINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_TDI				( (1 << 16) | (4 << 8) | (27 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_27_				( (1 << 16) | (4 << 8) | (27 << 3) | 1 )
#define	PADINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_TCLK				( (1 << 16) | (4 << 8) | (28 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_28_				( (1 << 16) | (4 << 8) | (28 << 3) | 1 )
#define	PADINDEX_OF_nx01301_CORTEXA9MP_TOP_QUADL2C_TDO				( (1 << 16) | (4 << 8) | (29 << 3) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_29_				( (1 << 16) | (4 << 8) | (29 << 3) | 1 )
#define	PADINDEX_OF_ECID_PAD_BONDING_ID_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ECID_PAD_BONDING_ID_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_nGRESETOUT				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_AliveGPIO_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_AliveGPIO_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_AliveGPIO_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_AliveGPIO_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_AliveGPIO_4_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_AliveGPIO_5_				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_nRESET				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_TEST_EN				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_nBATF				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_VDDPWRON				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_VDDPWRON_DDR				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_nVDDPWRTOGGLE				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_XTIRTC				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_4_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_5_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_6_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AIN_7_				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_VREF				( (0 << 16) | 0 )
#define	PADINDEX_OF_ADC_AGND				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_ZQ				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_31_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_30_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_29_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_28_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_27_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_26_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_25_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_24_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_PDQS_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_NDQS_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_DM_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_VREF1				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_15_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_14_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_13_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_12_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_11_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_10_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_9_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_8_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_PDQS_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_NDQS_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_DM_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_WE				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CAS				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_RAS				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ODT_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ODT_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_14_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_15_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_4_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CKE_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_RESET				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CKE_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CK				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CKB				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CS_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_CS_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_5_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_6_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_7_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_8_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_9_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_10_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_11_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_12_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_ADDR_13_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_BADDR_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_BADDR_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_BADDR_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_DM_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_NDQS_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_PDQS_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_7_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_6_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_5_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_4_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_3_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_1_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_0_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_VREF3				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_o_DM_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_NDQS_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_PDQS_2_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_23_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_22_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_21_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_20_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_19_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_18_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_17_				( (0 << 16) | 0 )
#define	PADINDEX_OF_DREX_io_DQ_16_				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DPDATA3				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DNDATA3				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DPDATA2				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DNDATA2				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DPCLK				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DNCLK				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DPDATA1				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DNDATA1				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DPDATA0				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_S_DNDATA0				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DPDATA3				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DNDATA3				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DPDATA2				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DNDATA2				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DPCLK				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DNCLK				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_VREG_0P4V				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DPDATA1				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DNDATA1				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DPDATA0				( (0 << 16) | 0 )
#define	PADINDEX_OF_MIPI_io_PHY_M_DNDATA0				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_ROUT				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXN_A				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXP_A				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXN_B				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXP_B				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXN_C				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXP_C				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXN_CLK				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXP_CLK				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXN_D				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXP_D				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXN_E				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_LVDS_TXP_E				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_XTI				( (0 << 16) | 0 )
#define	PADINDEX_OF_CLKPWR_XTI_not_used				( (0 << 16) | 0 )
#define	PADINDEX_OF_GPIOE_GPIO_7_				( (1 << 16) | (4 << 8) | (7 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_TXD_0_				( (1 << 16) | (4 << 8) | (7 << 3) | 1 )
#define	PADINDEX_OF_VIP1_i_ExtVSYNC				( (1 << 16) | (4 << 8) | (7 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_8_				( (1 << 16) | (4 << 8) | (8 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_TXD_1_				( (1 << 16) | (4 << 8) | (8 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_9_				( (1 << 16) | (4 << 8) | (9 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_TXD_2_				( (1 << 16) | (4 << 8) | (9 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_10_				( (1 << 16) | (4 << 8) | (10 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_TXD_3_				( (1 << 16) | (4 << 8) | (10 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_11_				( (1 << 16) | (4 << 8) | (11 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_TXEN				( (1 << 16) | (4 << 8) | (11 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_12_				( (1 << 16) | (4 << 8) | (12 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_TXER				( (1 << 16) | (4 << 8) | (12 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_13_				( (1 << 16) | (4 << 8) | (13 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_COL				( (1 << 16) | (4 << 8) | (13 << 3) | 1 )
#define	PADINDEX_OF_VIP1_i_ExtHSYNC				( (1 << 16) | (4 << 8) | (13 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_14_				( (1 << 16) | (4 << 8) | (14 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_RXD_0_				( (1 << 16) | (4 << 8) | (14 << 3) | 1 )
#define	PADINDEX_OF_SSP1_SSPCLK_IO				( (1 << 16) | (4 << 8) | (14 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_15_				( (1 << 16) | (4 << 8) | (15 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_RXD_1_				( (1 << 16) | (4 << 8) | (15 << 3) | 1 )
#define	PADINDEX_OF_SSP1_SSPFSS				( (1 << 16) | (4 << 8) | (15 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_16_				( (1 << 16) | (4 << 8) | (16 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_RXD_2_				( (1 << 16) | (4 << 8) | (16 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_17_				( (1 << 16) | (4 << 8) | (17 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_RXD_3_				( (1 << 16) | (4 << 8) | (17 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_18_				( (1 << 16) | (4 << 8) | (18 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_CLK_RX				( (1 << 16) | (4 << 8) | (18 << 3) | 1 )
#define	PADINDEX_OF_SSP1_SSPRXD				( (1 << 16) | (4 << 8) | (18 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_19_				( (1 << 16) | (4 << 8) | (19 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_RXDV				( (1 << 16) | (4 << 8) | (19 << 3) | 1 )
#define	PADINDEX_OF_SSP1_SSPTXD				( (1 << 16) | (4 << 8) | (19 << 3) | 2 )
#define	PADINDEX_OF_GPIOE_GPIO_20_				( (1 << 16) | (4 << 8) | (20 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_GMII_MDC				( (1 << 16) | (4 << 8) | (20 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_21_				( (1 << 16) | (4 << 8) | (21 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_GMII_MDI				( (1 << 16) | (4 << 8) | (21 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_22_				( (1 << 16) | (4 << 8) | (22 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_RXER				( (1 << 16) | (4 << 8) | (22 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_23_				( (1 << 16) | (4 << 8) | (23 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_PHY_CRS				( (1 << 16) | (4 << 8) | (23 << 3) | 1 )
#define	PADINDEX_OF_GPIOE_GPIO_24_				( (1 << 16) | (4 << 8) | (24 << 3) | 0 )
#define	PADINDEX_OF_DWC_GMAC_GTX_CLK				( (1 << 16) | (4 << 8) | (24 << 3) | 1 )
#define	PADINDEX_OF_DISPLAYTOP_io_HDMI_REXT				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TX0P				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TX0N				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TX1P				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TX1N				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TX2P				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TX2N				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TXCP				( (0 << 16) | 0 )
#define	PADINDEX_OF_DISPLAYTOP_o_HDMI_TXCN				( (0 << 16) | 0 )
//------------------------------------------------------------------------------
// BAGL RENDERING END
//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // __NX_CHIP_P2120_H__


