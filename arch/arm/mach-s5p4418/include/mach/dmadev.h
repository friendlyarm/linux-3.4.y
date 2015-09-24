/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __DMADEV_H__
#define __DMADEV_H__

#include <linux/dmaengine.h>
#include <linux/amba/pl08x.h>
#include <asm/hardware/pl080.h>

/* DMA0 peripheral ID */
#define	DMA_PERIPHERAL_ID_UART1_TX				0	// pl01115_Uart_modem_MODULE
#define	DMA_PERIPHERAL_ID_UART1_RX				1	// pl01115_Uart_modem_MODULE
#define	DMA_PERIPHERAL_ID_UART0_TX				2	// UART0_MODULE
#define	DMA_PERIPHERAL_ID_UART0_RX				3	// UART0_MODULE
#define	DMA_PERIPHERAL_ID_UART2_TX				4	// UART1_MODULE
#define	DMA_PERIPHERAL_ID_UART2_RX				5	// UART1_MODULE
#define	DMA_PERIPHERAL_ID_SSP0_TX				6
#define	DMA_PERIPHERAL_ID_SSP0_RX				7
#define	DMA_PERIPHERAL_ID_SSP1_TX				8
#define	DMA_PERIPHERAL_ID_SSP1_RX				9
#define	DMA_PERIPHERAL_ID_SSP2_TX				10
#define	DMA_PERIPHERAL_ID_SSP2_RX				11
#define	DMA_PERIPHERAL_ID_I2S0_TX				12
#define	DMA_PERIPHERAL_ID_I2S0_RX				13
#define	DMA_PERIPHERAL_ID_I2S1_TX				14
#define	DMA_PERIPHERAL_ID_I2S1_RX				15

/* DMA1 peripheral ID */
#define	DMA_PERIPHERAL_ID_I2S2_TX				16
#define	DMA_PERIPHERAL_ID_I2S2_RX				17
#define	DMA_PERIPHERAL_ID_AC97_PCMOUT			18
#define	DMA_PERIPHERAL_ID_AC97_PCMIN			19
#define	DMA_PERIPHERAL_ID_AC97_MICIN			20
#define	DMA_PERIPHERAL_ID_SPDIFRX				21
#define	DMA_PERIPHERAL_ID_SPDIFTX				22
#define	DMA_PERIPHERAL_ID_MPEGTSI0				23
#define	DMA_PERIPHERAL_ID_MPEGTSI1				24
#define	DMA_PERIPHERAL_ID_MPEGTSI2				25
#define	DMA_PERIPHERAL_ID_MPEGTSI3				26
#define	DMA_PERIPHERAL_ID_CRYPTO_BR				27
#define	DMA_PERIPHERAL_ID_CRYPTO_BW				28
#define	DMA_PERIPHERAL_ID_CRYPTO_HR				29
#define	DMA_PERIPHERAL_ID_PDM					30

/* DMA0 peripheral NAME */
#define	DMA_PERIPHERAL_NAME_UART1_TX			"uart1_tx"			// ID: 0, pl01115_Uart_modem_MODULE
#define	DMA_PERIPHERAL_NAME_UART1_RX			"uart1_rx"     		// ID: 1, pl01115_Uart_modem_MODULE
#define	DMA_PERIPHERAL_NAME_UART0_TX			"uart0_tx"          // ID: 2, UART0_MODULE
#define	DMA_PERIPHERAL_NAME_UART0_RX			"uart0_rx"          // ID: 3, UART0_MODULE
#define	DMA_PERIPHERAL_NAME_UART2_TX			"uart2_tx"          // ID: 4, UART1_MODULE
#define	DMA_PERIPHERAL_NAME_UART2_RX			"uart2_rx"          // ID: 5, UART1_MODULE
#define	DMA_PERIPHERAL_NAME_SSP0_TX				"ssp0_tx"           // ID: 6
#define	DMA_PERIPHERAL_NAME_SSP0_RX				"ssp0_rx"           // ID: 7
#define	DMA_PERIPHERAL_NAME_SSP1_TX				"ssp1_tx"           // ID: 8
#define	DMA_PERIPHERAL_NAME_SSP1_RX				"ssp1_rx"           // ID: 9
#define	DMA_PERIPHERAL_NAME_SSP2_TX				"ssp2_tx"           // ID: 10
#define	DMA_PERIPHERAL_NAME_SSP2_RX				"ssp2_rx"           // ID: 11
#define	DMA_PERIPHERAL_NAME_I2S0_TX				"i2s0_tx"           // ID: 12
#define	DMA_PERIPHERAL_NAME_I2S0_RX				"i2s0_rx"           // ID: 13
#define	DMA_PERIPHERAL_NAME_I2S1_TX				"i2s1_tx"           // ID: 14
#define	DMA_PERIPHERAL_NAME_I2S1_RX				"i2s1_rx"           // ID: 15

/* DMA1 peripheral NAME */
#define	DMA_PERIPHERAL_NAME_I2S2_TX				"i2s2_tx"			// ID: 16
#define	DMA_PERIPHERAL_NAME_I2S2_RX				"i2s2_rx"           // ID: 17
#define	DMA_PERIPHERAL_NAME_AC97_PCMOUT			"ac97_pcmout"       // ID: 18
#define	DMA_PERIPHERAL_NAME_AC97_PCMIN			"ac97_pcmin"        // ID: 19
#define	DMA_PERIPHERAL_NAME_AC97_MICIN			"ac97_micin"        // ID: 20
#define	DMA_PERIPHERAL_NAME_SPDIFRX				"spdif_rx"          // ID: 21
#define	DMA_PERIPHERAL_NAME_SPDIFTX				"spdif_tx"          // ID: 22
#define	DMA_PERIPHERAL_NAME_MPEGTSI0			"mpegtsi0"          // ID: 23
#define	DMA_PERIPHERAL_NAME_MPEGTSI1			"mpegtsi1"          // ID: 24
#define	DMA_PERIPHERAL_NAME_MPEGTSI2			"mpegtsi2"          // ID: 25
#define	DMA_PERIPHERAL_NAME_MPEGTSI3			"mpegtsi3"          // ID: 26
#define	DMA_PERIPHERAL_NAME_CRYPTO_BR			"crypto_br"         // ID: 27
#define	DMA_PERIPHERAL_NAME_CRYPTO_BW			"crypto_bw"         // ID: 28
#define	DMA_PERIPHERAL_NAME_CRYPTO_HR			"crypto_hr"         // ID: 29
#define	DMA_PERIPHERAL_NAME_PDM					"pdm"               // ID: 30

#endif //__DMADEV_H__