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

#ifndef __S5P6818_INTR_H__
#define __S5P6818_INTR_H__

/*
 * GIC Interrupt (0 ~ 32), must be align 32
 */
#define IRQ_GIC_START			(0)
#define IRQ_GIC_PPI_START		(IRQ_GIC_START  +  16)
#define IRQ_GIC_PPI_PVT			(IRQ_GIC_START  +  29)
#define IRQ_GIC_PPI_WDT			(IRQ_GIC_START  +  30)
#define IRQ_GIC_PPI_VIC			(IRQ_GIC_START  +  31)
#define IRQ_GIC_END				(IRQ_GIC_START  +  32)

/*
 * Physical Interrupt Number 64 (0~63)
 */
#define IRQ_PHY_MCUSTOP					(0  + 32)
#define IRQ_PHY_DMA0					(1  + 32)
#define IRQ_PHY_DMA1					(2  + 32)
#define IRQ_PHY_CLKPWR_INTREQPWR		(3  + 32)
#define IRQ_PHY_CLKPWR_ALIVEIRQ			(4  + 32)
#define IRQ_PHY_CLKPWR_RTCIRQ			(5  + 32)
#define IRQ_PHY_UART1					(6	+ 32) // pl01115_Uart_modem
#define IRQ_PHY_UART0					(7	+ 32) // UART0_MODULE
#define IRQ_PHY_UART2					(8	+ 32) // UART1_MODULE
#define IRQ_PHY_UART3					(9	+ 32) // pl01115_Uart_nodma0
#define IRQ_PHY_UART4					(10 + 32)	// pl01115_Uart_nodma1
#define IRQ_PHY_UART5					(11 + 32)	// pl01115_Uart_nodma2
#define IRQ_PHY_SSP0					(12 + 32)
#define IRQ_PHY_SSP1					(13 + 32)
#define IRQ_PHY_SSP2					(14 + 32)
#define IRQ_PHY_I2C0					(15 + 32)
#define IRQ_PHY_I2C1					(16 + 32)
#define IRQ_PHY_I2C2					(17 + 32)
#define IRQ_PHY_DEINTERLACE				(18 + 32)
#define IRQ_PHY_SCALER					(19 + 32)
#define IRQ_PHY_AC97					(20 + 32)
#define IRQ_PHY_SPDIFRX					(21 + 32)
#define IRQ_PHY_SPDIFTX					(22 + 32)
#define IRQ_PHY_TIMER_INT0				(23 + 32)
#define IRQ_PHY_TIMER_INT1				(24 + 32)
#define IRQ_PHY_TIMER_INT2				(25 + 32)
#define IRQ_PHY_TIMER_INT3				(26 + 32)
#define IRQ_PHY_PWM_INT0				(27 + 32)
#define IRQ_PHY_PWM_INT1				(28 + 32)
#define IRQ_PHY_PWM_INT2				(29 + 32)
#define IRQ_PHY_PWM_INT3				(30 + 32)
#define IRQ_PHY_WDT						(31 + 32)
#define IRQ_PHY_MPEGTSI					(32 + 32)
#define IRQ_PHY_DPC_P					(33 + 32)
#define IRQ_PHY_DPC_S					(34 + 32)
#define IRQ_PHY_RESCONV					(35 + 32)
#define IRQ_PHY_HDMI					(36 + 32)
#define IRQ_PHY_VIP0					(37 + 32)
#define IRQ_PHY_VIP1					(38 + 32)
#define IRQ_PHY_MIPI					(39 + 32)
#define IRQ_PHY_VR						(40 + 32)
#define IRQ_PHY_ADC						(41 + 32)
#define IRQ_PHY_PPM						(42 + 32)
#define IRQ_PHY_SDMMC0					(43 + 32)
#define IRQ_PHY_SDMMC1					(44 + 32)
#define IRQ_PHY_SDMMC2					(45 + 32)
#define IRQ_PHY_CODA960_HOST			(46 + 32)
#define IRQ_PHY_CODA960_JPG				(47 + 32)
#define IRQ_PHY_GMAC					(48 + 32)
#define IRQ_PHY_USB20OTG				(49 + 32)
#define IRQ_PHY_USB20HOST				(50 + 32)
#define IRQ_PHY_CAN0					(51 + 32)
#define IRQ_PHY_CAN1					(52 + 32)
#define IRQ_PHY_GPIOA					(53 + 32)
#define IRQ_PHY_GPIOB					(54 + 32)
#define IRQ_PHY_GPIOC					(55 + 32)
#define IRQ_PHY_GPIOD					(56 + 32)
#define IRQ_PHY_GPIOE					(57 + 32)
#define IRQ_PHY_CRYPTO					(58 + 32)
#define IRQ_PHY_PDM						(59 + 32)
#define IRQ_PHY_TMU0                    (60 + 32)
#define IRQ_PHY_TMU1                    (61 + 32)
#define IRQ_PHY_VIP2					(72 + 32)

#define IRQ_PHY_MAX_COUNT       		(74 + 32) // ADD GIC IRQ

/*
 * GPIO Interrupt Number 160 (106~265)
 */
#define IRQ_GPIO_START			IRQ_PHY_MAX_COUNT
#define IRQ_GPIO_END			(IRQ_GPIO_START + 32 * 5)	// Group: A,B,C,D,E

#define IRQ_GPIO_A_START		(IRQ_GPIO_START + PAD_GPIO_A)
#define IRQ_GPIO_B_START		(IRQ_GPIO_START + PAD_GPIO_B)
#define IRQ_GPIO_C_START		(IRQ_GPIO_START + PAD_GPIO_C)
#define IRQ_GPIO_D_START		(IRQ_GPIO_START + PAD_GPIO_D)
#define IRQ_GPIO_E_START		(IRQ_GPIO_START + PAD_GPIO_E)

/*
 * ALIVE Interrupt Number 6 (266~271)
 */
#define IRQ_ALIVE_START			IRQ_GPIO_END
#define IRQ_ALIVE_END			(IRQ_ALIVE_START + 6)

#define IRQ_ALIVE_0				(IRQ_ALIVE_START + 0)
#define IRQ_ALIVE_1				(IRQ_ALIVE_START + 1)
#define IRQ_ALIVE_2				(IRQ_ALIVE_START + 2)
#define IRQ_ALIVE_3				(IRQ_ALIVE_START + 3)
#define IRQ_ALIVE_4				(IRQ_ALIVE_START + 4)
#define IRQ_ALIVE_5				(IRQ_ALIVE_START + 5)

/*
 * MAX(Physical+Virtual) Interrupt Number
 */
#define IRQ_SYSTEM_END			IRQ_ALIVE_END

#if defined (CONFIG_REGULATOR_NXE2000)
#define	IRQ_RESERVED_OFFSET		72		// refer NXE2000_NR_IRQS <linux/mfd/nxe2000.h>
#else
#define	IRQ_RESERVED_OFFSET		0
#endif
#define IRQ_SYSTEM_RESERVED		IRQ_RESERVED_OFFSET

#define IRQ_TOTAL_MAX_COUNT  	(IRQ_SYSTEM_END + IRQ_SYSTEM_RESERVED)

#endif //__S5P6818_INTR_H__