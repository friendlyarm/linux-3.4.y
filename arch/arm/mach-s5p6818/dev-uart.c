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

#include <mach/serial.h>
#include <mach/s5p6818.h>

#ifndef CFG_UART_CLKGEN_CLOCK_HZ
#define CFG_UART_CLKGEN_CLOCK_HZ                50000000 
#endif

#ifndef CFG_UART0_CLKGEN_CLOCK_HZ
#define CFG_UART0_CLKGEN_CLOCK_HZ CFG_UART_CLKGEN_CLOCK_HZ
#endif

#ifndef CFG_UART1_CLKGEN_CLOCK_HZ
#define CFG_UART1_CLKGEN_CLOCK_HZ CFG_UART_CLKGEN_CLOCK_HZ
#endif

#ifndef CFG_UART2_CLKGEN_CLOCK_HZ
#define CFG_UART2_CLKGEN_CLOCK_HZ CFG_UART_CLKGEN_CLOCK_HZ
#endif

#ifndef CFG_UART3_CLKGEN_CLOCK_HZ
#define CFG_UART3_CLKGEN_CLOCK_HZ CFG_UART_CLKGEN_CLOCK_HZ
#endif

#ifndef CFG_UART4_CLKGEN_CLOCK_HZ
#define CFG_UART4_CLKGEN_CLOCK_HZ CFG_UART_CLKGEN_CLOCK_HZ
#endif

#ifndef CFG_UART5_CLKGEN_CLOCK_HZ
#define CFG_UART5_CLKGEN_CLOCK_HZ CFG_UART_CLKGEN_CLOCK_HZ
#endif

#define UART_RESOURCE(device, base, irqnr)  \
struct resource device##_resource[] = {	\
		[0] = { .start	= base, .end = base + 0x40, .flags = IORESOURCE_MEM,	},	\
		[1] = {	.start	= irqnr , .end = irqnr, .flags = IORESOURCE_IRQ, },	\
	};

#define UART_PLDEVICE(device, dev_name, ch, res, pdata)  \
struct platform_device device##_device = {	\
	    .name = dev_name,          				\
	    .id = ch,								\
		.num_resources = ARRAY_SIZE(res),		\
		.resource = res,						\
	    .dev = { .platform_data = pdata,	},	\
	};

#define	UART_CHARNNEL_INIT(ch)	{ \
		char name[16] = { 'n','x','p','-','u','a','r','t','.', (48 + ch) };	\
		struct clk *clk = clk_get(NULL, name);	\
		if (!nxp_soc_peri_reset_status(RESET_ID_UART## ch)) {		\
			NX_TIEOFF_Set(TIEOFF_UART## ch ##_USERSMC , 0);	\
			NX_TIEOFF_Set(TIEOFF_UART## ch ##_SMCTXENB, 0);	\
			NX_TIEOFF_Set(TIEOFF_UART## ch ##_SMCRXENB, 0);	\
			nxp_soc_peri_reset_set(RESET_ID_UART## ch);			\
		}													\
		clk_set_rate(clk, CFG_UART## ch ##_CLKGEN_CLOCK_HZ);		\
		clk_enable(clk);								\
	};

#define	uart_device_register(ch) 		{	\
		UART_CHARNNEL_INIT(ch);	\
		platform_device_register(&uart## ch ##_device);	\
	}
static const unsigned char uart_port[6][5] =
{
	{ PAD_GPIO_D, 14,PAD_GPIO_D, 18, NX_GPIO_PADFUNC_1},
	{ PAD_GPIO_D, 15,PAD_GPIO_D, 19, NX_GPIO_PADFUNC_1},
	{ PAD_GPIO_D, 16,PAD_GPIO_D, 20, NX_GPIO_PADFUNC_1},
	{ PAD_GPIO_D, 17,PAD_GPIO_D, 21, NX_GPIO_PADFUNC_1},
	{ PAD_GPIO_B, 28,PAD_GPIO_B, 29, NX_GPIO_PADFUNC_3},
	{ PAD_GPIO_B, 30,PAD_GPIO_B, 31, NX_GPIO_PADFUNC_3},
};
static void uart_device_init(int hwport)
{

	switch(hwport)
	{
		case 0: UART_CHARNNEL_INIT(0);break;
		case 1: UART_CHARNNEL_INIT(1);break;
		case 2: UART_CHARNNEL_INIT(2);break;
		case 3: UART_CHARNNEL_INIT(3);break;
		case 4: UART_CHARNNEL_INIT(4);break;
		case 5: UART_CHARNNEL_INIT(5);break;
	}		
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(uart_port[hwport][0]), uart_port[hwport][1], uart_port[hwport][4] );	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(uart_port[hwport][2]), uart_port[hwport][3], uart_port[hwport][4] );	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(uart_port[hwport][0]), uart_port[hwport][1], CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(uart_port[hwport][2]), uart_port[hwport][3], CTRUE);
}

static void uart_device_exit(int hwport) { }
static void uart_device_wake_peer(struct uart_port *uport) { }

/*------------------------------------------------------------------------------
 * Serial platform device
 */
#if defined(CONFIG_SERIAL_NXP_UART0)
void uport0_weak_alias_init(int hwport)	__attribute__((weak, alias("uart_device_init")));
void uport0_weak_alias_exit(int hwport)	__attribute__((weak, alias("uart_device_exit")));
void uport0_weak_alias_wake_peer(struct uart_port *uport)
		__attribute__((weak, alias("uart_device_wake_peer")));

static struct s3c24xx_uart_platdata  uart0_data = {
	.hwport = 0,
	.init = uport0_weak_alias_init,
	.exit = uport0_weak_alias_exit,
	.wake_peer = uport0_weak_alias_wake_peer,
	.ucon = S5PV210_UCON_DEFAULT,
	.ufcon = S5PV210_UFCON_DEFAULT,
	.has_fracval = 1,
	#if defined(CONFIG_SERIAL_NXP_UART0_DMA) && defined(CONFIG_DMA_ENGINE)
	.enable_dma = 1,
	.dma_filter = pl08x_filter_id,
 	.dma_rx_param = (void *) DMA_PERIPHERAL_NAME_UART0_RX,
 	.dma_tx_param = (void *) DMA_PERIPHERAL_NAME_UART0_TX,
	#else
	.enable_dma = 0,
 	#endif
};

static UART_RESOURCE(uart0, PHY_BASEADDR_UART0, IRQ_PHY_UART0);
static UART_PLDEVICE(uart0, "nxp-uart", 0, uart0_resource, &uart0_data);
#endif

#if defined(CONFIG_SERIAL_NXP_UART1)
void uport1_weak_alias_init(int hwport)	__attribute__((weak, alias("uart_device_init")));
void uport1_weak_alias_exit(int hwport)	__attribute__((weak, alias("uart_device_exit")));
void uport1_weak_alias_wake_peer(struct uart_port *uport)
		__attribute__((weak, alias("uart_device_wake_peer")));

static struct s3c24xx_uart_platdata  uart1_data = {
	.hwport = 1,
	.init = uport1_weak_alias_init,
	.exit = uport1_weak_alias_exit,
	.wake_peer = uport1_weak_alias_wake_peer,
	.ucon = S5PV210_UCON_DEFAULT,
	.ufcon = S5PV210_UFCON_DEFAULT,
	.has_fracval = 1,
	#if defined(CONFIG_SERIAL_NXP_UART1_DMA) && defined(CONFIG_DMA_ENGINE)
	.enable_dma = 1,
 	.dma_filter = pl08x_filter_id,
 	.dma_rx_param = (void *) DMA_PERIPHERAL_NAME_UART1_RX,
 	.dma_tx_param = (void *) DMA_PERIPHERAL_NAME_UART1_TX,
	#else
	.enable_dma = 0,
 	#endif
};

static UART_RESOURCE(uart1, PHY_BASEADDR_UART1, IRQ_PHY_UART1);
static UART_PLDEVICE(uart1, "nxp-uart", 1, uart1_resource, &uart1_data);
#endif




#if defined(CONFIG_SERIAL_NXP_UART2)
void uport2_weak_alias_init(int hwport)	__attribute__((weak, alias("uart_device_init")));
void uport2_weak_alias_exit(int hwport)	__attribute__((weak, alias("uart_device_exit")));
void uport2_weak_alias_wake_peer(struct uart_port *uport)
		__attribute__((weak, alias("uart_device_wake_peer")));

static struct s3c24xx_uart_platdata  uart2_data = {
	.hwport = 2,
	.init = uport2_weak_alias_init,
	.exit = uport2_weak_alias_exit,
	.wake_peer = uport2_weak_alias_wake_peer,
	.ucon = S5PV210_UCON_DEFAULT,
	.ufcon = S5PV210_UFCON_DEFAULT,
	.has_fracval = 1,
	#if defined(CONFIG_SERIAL_NXP_UART2_DMA) && defined(CONFIG_DMA_ENGINE)
	.enable_dma = 1,
 	.dma_filter = pl08x_filter_id,
 	.dma_rx_param = (void *) DMA_PERIPHERAL_NAME_UART2_RX,
 	.dma_tx_param = (void *) DMA_PERIPHERAL_NAME_UART2_TX,
	#else
	.enable_dma = 0,
 	#endif
};

static UART_RESOURCE(uart2, PHY_BASEADDR_UART2, IRQ_PHY_UART2);
static UART_PLDEVICE(uart2, "nxp-uart", 2, uart2_resource, &uart2_data);
#endif


#if defined(CONFIG_SERIAL_NXP_UART3)
void uport3_weak_alias_init(int hwport)	__attribute__((weak, alias("uart_device_init")));
void uport3_weak_alias_exit(int hwport)	__attribute__((weak, alias("uart_device_exit")));
void uport3_weak_alias_wake_peer(struct uart_port *uport)
		__attribute__((weak, alias("uart_device_wake_peer")));

static struct s3c24xx_uart_platdata  uart3_data = {
	.hwport = 3,
	.init = uport3_weak_alias_init,
	.exit = uport3_weak_alias_exit,
	.wake_peer = uport3_weak_alias_wake_peer,
	.ucon = S5PV210_UCON_DEFAULT,
	.ufcon = S5PV210_UFCON_DEFAULT,
	.has_fracval = 1,
	.enable_dma = 0,
 };

static UART_RESOURCE(uart3, PHY_BASEADDR_UART3, IRQ_PHY_UART3);
static UART_PLDEVICE(uart3, "nxp-uart", 3, uart3_resource, &uart3_data);
#endif

#if defined(CONFIG_SERIAL_NXP_UART4)
void uport4_weak_alias_init(int hwport)	__attribute__((weak, alias("uart_device_init")));
void uport4_weak_alias_exit(int hwport)	__attribute__((weak, alias("uart_device_exit")));
void uport4_weak_alias_wake_peer(struct uart_port *uport)
		__attribute__((weak, alias("uart_device_wake_peer")));

static struct s3c24xx_uart_platdata  uart4_data = {
	.hwport = 4,
	.init = uport4_weak_alias_init,
	.exit = uport4_weak_alias_exit,
	.wake_peer = uport4_weak_alias_wake_peer,
	.ucon = S5PV210_UCON_DEFAULT,
	.ufcon = S5PV210_UFCON_DEFAULT,
	.has_fracval = 1,
	.enable_dma = 0,
};

static UART_RESOURCE(uart4, PHY_BASEADDR_UART4, IRQ_PHY_UART4);
static UART_PLDEVICE(uart4, "nxp-uart", 4, uart4_resource, &uart4_data);
#endif

#if defined(CONFIG_SERIAL_NXP_UART5)
void uport5_weak_alias_init(int hwport)	__attribute__((weak, alias("uart_device_init")));
void uport5_weak_alias_exit(int hwport)	__attribute__((weak, alias("uart_device_exit")));
void uport5_weak_alias_wake_peer(struct uart_port *uport)
		__attribute__((weak, alias("uart_device_wake_peer")));

static struct s3c24xx_uart_platdata  uart5_data = {
	.hwport = 5,
	.init = uport5_weak_alias_init,
	.exit = uport5_weak_alias_exit,
	.wake_peer = uport5_weak_alias_wake_peer,
	.ucon = S5PV210_UCON_DEFAULT,
	.ufcon = S5PV210_UFCON_DEFAULT,
	.has_fracval = 1,
	.enable_dma = 0,
};

static UART_RESOURCE(uart5, PHY_BASEADDR_UART5, IRQ_PHY_UART5);
static UART_PLDEVICE(uart5, "nxp-uart", 5, uart5_resource, &uart5_data);
#endif
