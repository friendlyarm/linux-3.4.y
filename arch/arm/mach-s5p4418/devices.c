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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/amba/bus.h>
#include <linux/amba/serial.h>
#include <linux/dma-mapping.h>
#include <linux/pm_runtime.h>

/* slsi soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>
#include <linux/vr/vr_utgard.h>

/*------------------------------------------------------------------------------
 * Serial platform device
 */
#if defined(CONFIG_SERIAL_AMBA_PL011)

#define	NX_UART_CH_INIT(ch) do { \
	struct clk *clk;									\
	char name[16];										\
	sprintf(name, "nxp-uart.%d", ch);					\
	clk = clk_get(NULL, name);							\
	if (!nxp_soc_peri_reset_status(RESET_ID_UART## ch)) {		\
		NX_TIEOFF_Set(TIEOFF_UART## ch ##_USERSMC , 0);	\
		NX_TIEOFF_Set(TIEOFF_UART## ch ##_SMCTXENB, 0);	\
		NX_TIEOFF_Set(TIEOFF_UART## ch ##_SMCRXENB, 0);	\
		nxp_soc_peri_reset_set(RESET_ID_UART## ch);			\
	}													\
	clk_set_rate(clk, CFG_UART_CLKGEN_CLOCK_HZ);		\
	clk_enable(clk);								\
	} while (0)

#if defined(CONFIG_SERIAL_NXP_UART0)
static void __pl011_uart0_prepare(void)
{
	NX_UART_CH_INIT(0);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 14, NX_GPIO_PADFUNC_1);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 18, NX_GPIO_PADFUNC_1);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 14, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 18, CTRUE);
}

static void __pl011_uart0_wake_peer(void *uport) { }
static void __pl011_uart0_exit(void) { }

void pl011_uart0_prepare(void)			__attribute__((weak, alias("__pl011_uart0_prepare")));
void pl011_uart0_wake_peer(void *uport)	__attribute__((weak, alias("__pl011_uart0_wake_peer")));
void pl011_uart0_exit(void)				__attribute__((weak, alias("__pl011_uart0_exit")));

static struct amba_pl011_data  pl011_data_ch0 = {
	#if defined(CONFIG_SERIAL_NXP_UART0_DMA)
 	.dma_filter = pl08x_filter_id,
 	.dma_rx_param = (void *) DMA_PERIPHERAL_NAME_UART0_RX,
 	.dma_tx_param = (void *) DMA_PERIPHERAL_NAME_UART0_TX,
 	#endif
	.init = pl011_uart0_prepare,
	.exit = pl011_uart0_exit,
	.wake_peer = pl011_uart0_wake_peer,
};
static AMBA_AHB_DEVICE(uart0, "uart-pl011.0", 0, PHY_BASEADDR_UART0, {IRQ_PHY_UART0}, &pl011_data_ch0);
#endif

#if defined(CONFIG_SERIAL_NXP_UART1)
static void __pl011_uart1_prepare(void)
{
	NX_UART_CH_INIT(1);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 15, NX_GPIO_PADFUNC_1);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 19, NX_GPIO_PADFUNC_1);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 15, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 19, CTRUE);
}

static void __pl011_uart1_wake_peer(void *uport) { }
static void __pl011_uart1_exit(void) { }

void pl011_uart1_prepare(void)			__attribute__((weak, alias("__pl011_uart1_prepare")));
void pl011_uart1_wake_peer(void *uport)	__attribute__((weak, alias("__pl011_uart1_wake_peer")));
void pl011_uart1_exit(void)				__attribute__((weak, alias("__pl011_uart1_exit")));

static struct amba_pl011_data  pl011_data_ch1 = {
	#if defined(CONFIG_SERIAL_NXP_UART1_DMA)
 	.dma_filter = pl08x_filter_id,
 	.dma_rx_param = (void *) DMA_PERIPHERAL_NAME_UART1_RX,
 	.dma_tx_param = (void *) DMA_PERIPHERAL_NAME_UART1_TX,
 	#endif
	.init = pl011_uart1_prepare,
	.exit = pl011_uart1_exit,
	.wake_peer = pl011_uart1_wake_peer,
};
static AMBA_AHB_DEVICE(uart1, "uart-pl011.1", 0, PHY_BASEADDR_UART1, {IRQ_PHY_UART1}, &pl011_data_ch1);
#endif

#if defined(CONFIG_SERIAL_NXP_UART2)
static void __pl011_uart2_prepare(void)
{
	NX_UART_CH_INIT(2);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 16, NX_GPIO_PADFUNC_1);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 20, NX_GPIO_PADFUNC_1);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 16, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 20, CTRUE);
}

static void __pl011_uart2_wake_peer(void *uport) { }
static void __pl011_uart2_exit(void) { }

void pl011_uart2_prepare(void)			__attribute__((weak, alias("__pl011_uart2_prepare")));
void pl011_uart2_wake_peer(void *uport)	__attribute__((weak, alias("__pl011_uart2_wake_peer")));
void pl011_uart2_exit(void)				__attribute__((weak, alias("__pl011_uart2_exit")));

static struct amba_pl011_data  pl011_data_ch2 = {
	#if defined(CONFIG_SERIAL_NXP_UART2_DMA)
 	.dma_filter = pl08x_filter_id,
 	.dma_rx_param = (void *) DMA_PERIPHERAL_NAME_UART2_RX,
 	.dma_tx_param = (void *) DMA_PERIPHERAL_NAME_UART2_TX,
 	#endif
	.init = pl011_uart2_prepare,
	.exit = pl011_uart2_exit,
	.wake_peer = pl011_uart2_wake_peer,
};
static AMBA_AHB_DEVICE(uart2, "uart-pl011.2", 0, PHY_BASEADDR_UART2, {IRQ_PHY_UART2}, &pl011_data_ch2);
#endif

#if defined(CONFIG_SERIAL_NXP_UART3)
static void __pl011_uart3_prepare(void)
{
	NX_UART_CH_INIT(3);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 17, NX_GPIO_PADFUNC_1);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_D), 21, NX_GPIO_PADFUNC_1);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 17, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_D), 21, CTRUE);
}

static void __pl011_uart3_wake_peer(void *uport) { }
static void __pl011_uart3_exit(void) { }

void pl011_uart3_prepare(void)			__attribute__((weak, alias("__pl011_uart3_prepare")));
void pl011_uart3_wake_peer(void *uport)	__attribute__((weak, alias("__pl011_uart3_wake_peer")));
void pl011_uart3_exit(void)				__attribute__((weak, alias("__pl011_uart3_exit")));

static struct amba_pl011_data  pl011_data_ch3 = {
	.init = pl011_uart3_prepare,
	.exit = pl011_uart3_exit,
	.wake_peer = pl011_uart3_wake_peer,
};

static AMBA_AHB_DEVICE(uart3, "uart-pl011.3", 0, PHY_BASEADDR_UART3, {IRQ_PHY_UART3}, &pl011_data_ch3);
#endif

#if defined(CONFIG_SERIAL_NXP_UART4)
static void __pl011_uart4_prepare(void)
{
	NX_UART_CH_INIT(4);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_B), 28, NX_GPIO_PADFUNC_3);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_B), 29, NX_GPIO_PADFUNC_3);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_B), 28, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_B), 29, CTRUE);
}

static void __pl011_uart4_wake_peer(void *uport) { }
static void __pl011_uart4_exit(void) { }

void pl011_uart4_prepare(void)			__attribute__((weak, alias("__pl011_uart4_prepare")));
void pl011_uart4_wake_peer(void *uport)	__attribute__((weak, alias("__pl011_uart4_wake_peer")));
void pl011_uart4_exit(void)				__attribute__((weak, alias("__pl011_uart4_exit")));

static struct amba_pl011_data  pl011_data_ch4 = {
	.init = pl011_uart4_prepare,
	.exit = pl011_uart4_exit,
	.wake_peer = pl011_uart4_wake_peer,
};

static AMBA_AHB_DEVICE(uart4, "uart-pl011.4", 0, PHY_BASEADDR_UART4, {IRQ_PHY_UART4}, &pl011_data_ch4);
#endif

#if defined(CONFIG_SERIAL_NXP_UART5)
static void __pl011_uart5_prepare(void)
{
	NX_UART_CH_INIT(5);
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_B), 30, NX_GPIO_PADFUNC_3);	// RX
	NX_GPIO_SetPadFunction (PAD_GET_GROUP(PAD_GPIO_B), 31, NX_GPIO_PADFUNC_3);	// TX
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_B), 30, CFALSE);
	NX_GPIO_SetOutputEnable(PAD_GET_GROUP(PAD_GPIO_B), 31, CTRUE);
}

static void __pl011_uart5_wake_peer(void *uport) { }
static void __pl011_uart5_exit(void) { }

void pl011_uart5_prepare(void)			__attribute__((weak, alias("__pl011_uart5_prepare")));
void pl011_uart5_wake_peer(void *uport)	__attribute__((weak, alias("__pl011_uart5_wake_peer")));
void pl011_uart5_exit(void)				__attribute__((weak, alias("__pl011_uart5_exit")));

static struct amba_pl011_data  pl011_data_ch5 = {
	.init = pl011_uart5_prepare,
	.exit = pl011_uart5_exit,
	.wake_peer = pl011_uart5_wake_peer,
};

static AMBA_AHB_DEVICE(uart5, "uart-pl011.5", 0, PHY_BASEADDR_UART5, {IRQ_PHY_UART5}, &pl011_data_ch5);
#endif

#endif /* CONFIG_SERIAL_AMBA_PL011 */

/*------------------------------------------------------------------------------
 * I2C Bus platform device
 */

#if defined(CONFIG_I2C_NXP)
#define I2CUDELAY(x)	1000000/x/2
/* gpio i2c 0 */
#ifdef CFG_IO_I2C0_SCL
#define	I2C0_SCL	CFG_IO_I2C0_SCL
#else
#define	I2C0_SCL	NXP_I2C0_MOD_SCL
#endif
#ifdef CFG_IO_I2C0_SDA
#define	I2C0_SDA	CFG_IO_I2C0_SDA
#else
#define	I2C0_SDA	NXP_I2C0_MOD_SDA
#endif
/* gpio i2c 1 */
#ifdef CFG_IO_I2C1_SCL
#define	I2C1_SCL	CFG_IO_I2C1_SCL
#else
#define	I2C1_SCL	NXP_I2C1_MOD_SCL
#endif
#ifdef CFG_IO_I2C1_SDA
#define	I2C1_SDA	CFG_IO_I2C1_SDA
#else
#define	I2C1_SDA	NXP_I2C1_MOD_SDA
#endif
/* gpio i2c 2 */
#ifdef CFG_IO_I2C2_SCL
#define	I2C2_SCL	CFG_IO_I2C2_SCL
#else
#define	I2C2_SCL	NXP_I2C2_MOD_SCL
#endif
#ifdef CFG_IO_I2C2_SDA
#define	I2C2_SDA	CFG_IO_I2C2_SDA
#else
#define	I2C2_SDA	NXP_I2C2_MOD_SDA
#endif

#ifdef CFG_I2C0_RETRY_CNT
#define I2C0_RETRY_CNT CFG_I2C0_RETRY_CNT
#else
#define I2C0_RETRY_CNT 3
#endif

#ifdef CFG_I2C0_RETRY_DELAY
#define I2C0_RETRY_DELAY CFG_I2C0_RETRY_DELAY
#else
#define I2C0_RETRY_DELAY 0
#endif

#ifdef CFG_I2C1_RETRY_CNT
#define I2C1_RETRY_CNT CFG_I2C1_RETRY_CNT
#else
#define I2C1_RETRY_CNT 3
#endif

#ifdef CFG_I2C1_RETRY_DELAY_
#define I2C1_RETRY_DELAY CFG_I2C1_RETRY_DELAY
#else
#define I2C1_RETRY_DELAY 0
#endif


#ifdef CFG_I2C2_RETRY_CNT
#define I2C2_RETRY_CNT CFG_I2C2_RETRY_CNT
#else
#define I2C2_RETRY_CNT 3
#endif

#ifdef CFG_I2C2_RETRY_DELAY_
#define I2C2_RETRY_DELAY CFG_I2C2_RETRY_DELAY
#else
#define I2C2_RETRY_DELAY 0
#endif


#if	defined(CONFIG_I2C_NXP_PORT0)

static struct i2c_gpio_platform_data nxp_i2c_gpio_port0 = {
	.sda_pin	= I2C0_SDA,
	.scl_pin	= I2C0_SCL,
	.udelay		= I2CUDELAY(CFG_I2C0_CLK),				/* Gpio_mode CLK Rate = 1/( udelay*2) * 1000000 */
	.timeout	= 10,
};

#if  defined(CONFIG_I2C_NXP_PORT0_GPIO_MODE)

static struct platform_device i2c_device_ch0 = {
	.name	= "i2c-gpio",
	.id		= 0,
	.dev    = {
		.platform_data	= &nxp_i2c_gpio_port0,
	},
};
#else
static struct nxp_i2c_plat_data i2c_data_ch0 = {
	.port		= 0,
	.irq		= IRQ_PHY_I2C0,
	.gpio 		= &nxp_i2c_gpio_port0,
	.base_addr	= PHY_BASEADDR_I2C0,
	.rate 		= CFG_I2C0_CLK,
	.retry_cnt  =  I2C0_RETRY_CNT, 
	.retry_delay=  I2C0_RETRY_DELAY, 
};

static struct platform_device i2c_device_ch0 = {
	.name	= DEV_NAME_I2C,
	.id		= 0,
	.dev    = {
		.platform_data	= &i2c_data_ch0
	},
};
#endif /* CONFIG_I2C_NXP_PORT0 */


#endif

#if	defined(CONFIG_I2C_NXP_PORT1)
static struct i2c_gpio_platform_data nxp_i2c_gpio_port1 = {
	.sda_pin	= I2C1_SDA,
	.scl_pin	= I2C1_SCL,
	.udelay		= I2CUDELAY(CFG_I2C1_CLK),				/* Gpio_mode CLK Rate = 1/( udelay*2) * 1000000 */
	.timeout	= 10,
};


#if  defined(CONFIG_I2C_NXP_PORT1_GPIO_MODE)
static struct platform_device i2c_device_ch1 = {
	.name	= "i2c-gpio",
	.id		= 1,
	.dev    = {
		.platform_data	= &nxp_i2c_gpio_port1,
	},
};

#else
static struct nxp_i2c_plat_data i2c_data_ch1 = {
	.port		= 1,
	.irq		= IRQ_PHY_I2C1,
	.gpio 		= &nxp_i2c_gpio_port1,
	.base_addr	= PHY_BASEADDR_I2C1,
	.rate 		= CFG_I2C1_CLK,
	.retry_cnt  =  I2C1_RETRY_CNT, 
	.retry_delay=  I2C1_RETRY_DELAY, 
};

static struct platform_device i2c_device_ch1 = {
	.name	= DEV_NAME_I2C,
	.id		= 1,
	.dev    = {
		.platform_data	= &i2c_data_ch1
	},
};
#endif
#endif

#if	defined(CONFIG_I2C_NXP_PORT2)
static struct i2c_gpio_platform_data nxp_i2c_gpio_port2 = {
	.sda_pin	= I2C2_SDA,
	.scl_pin	= I2C2_SCL,
	.udelay		= I2CUDELAY(CFG_I2C2_CLK),				/* Gpio_mode CLK Rate = 1/( udelay*2) * 1000000 */

	.timeout	= 10,
};


#if  defined(CONFIG_I2C_NXP_PORT2_GPIO_MODE)
static struct platform_device i2c_device_ch2 = {
	.name	= "i2c-gpio",
	.id		= 2,
	.dev    = {
		.platform_data	= &nxp_i2c_gpio_port2,
	},
};

#else
static struct nxp_i2c_plat_data i2c_data_ch2 = {
	.port		= 2,
	.irq		= IRQ_PHY_I2C2,
	.gpio 		= &nxp_i2c_gpio_port2,
	.base_addr	= PHY_BASEADDR_I2C2,
	.rate 		= CFG_I2C2_CLK,
	.retry_cnt  =  I2C2_RETRY_CNT, 
	.retry_delay=  I2C2_RETRY_DELAY, 
};

static struct platform_device i2c_device_ch2 = {
	.name	= DEV_NAME_I2C,
	.id		= 2,
	.dev    = {
		.platform_data	= &i2c_data_ch2
	},
};
#endif
#endif

static struct platform_device *i2c_devices[] = {
	#if	defined(CONFIG_I2C_NXP_PORT0)
	&i2c_device_ch0,
	#endif
	#if	defined(CONFIG_I2C_NXP_PORT1)
	&i2c_device_ch1,
	#endif
	#if	defined(CONFIG_I2C_NXP_PORT2)
	&i2c_device_ch2,
	#endif
};
#endif /* CONFIG_I2C_NXP */

/*------------------------------------------------------------------------------
   * RTC (Real Time Clock) platform device
    */
#if defined(CONFIG_RTC_DRV_NXP)
static struct platform_device rtc_plat_device = {
	.name   = DEV_NAME_RTC,
	.id     = 0,
};
#endif  /* CONFIG_RTC_DRV_NXP */

/*------------------------------------------------------------------------------
 * PWM platform device
 */
#if defined(CONFIG_HAVE_PWM)

#if defined(CONFIG_HAVE_PWM_CH0)
static struct platform_device pwm_device_ch0 = {
	.name	= DEV_NAME_PWM,
	.id		= 0,
};
#endif
#if defined(CONFIG_HAVE_PWM_CH1)
static struct platform_device pwm_device_ch1 = {
	.name	= DEV_NAME_PWM,
	.id		= 1,
};
#endif
#if defined(CONFIG_HAVE_PWM_CH2)
static struct platform_device pwm_device_ch2 = {
	.name	= DEV_NAME_PWM,
	.id		= 2,
};
#endif
#if defined(CONFIG_HAVE_PWM_CH3)
static struct platform_device pwm_device_ch3 = {
	.name	= DEV_NAME_PWM,
	.id		= 3,
};
#endif

static struct platform_device *pwm_devices[] = {
	#if	defined(CONFIG_HAVE_PWM_CH0)
	&pwm_device_ch0,
	#endif
	#if	defined(CONFIG_HAVE_PWM_CH1)
	&pwm_device_ch1,
	#endif
	#if	defined(CONFIG_HAVE_PWM_CH2)
	&pwm_device_ch2,
	#endif
	#if	defined(CONFIG_HAVE_PWM_CH3)
	&pwm_device_ch3,
	#endif
};
#endif	/* CONFIG_HAVE_PWM */

/*------------------------------------------------------------------------------
 * GPIO device
 */
#if defined(CONFIG_GPIO_NXP)
#if	defined(CONFIG_GPIO_NXP_GROUP_A)
static struct resource gpio_resource_A = {
		.start  	=  0,
		.end   	 	= 32,
		.flags  	= IORESOURCE_IO,
};
static struct platform_device gpio_device_A = {
	.name			= DEV_NAME_GPIO,
	.id				= 0,
	.resource		= &gpio_resource_A,
	.num_resources  = 1,
};
#endif
#if	defined(CONFIG_GPIO_NXP_GROUP_B)
static struct resource gpio_resource_B = {
		.start  	=  0,
		.end   	 	= 32,
		.flags  	= IORESOURCE_IO,
};
static struct platform_device gpio_device_B = {
	.name			= DEV_NAME_GPIO,
	.id				= 1,
	.resource		= &gpio_resource_B,
	.num_resources  = 1,
};
#endif
#if	defined(CONFIG_GPIO_NXP_GROUP_C)
static struct resource gpio_resource_C = {
		.start  	=  0,
		.end   	 	= 32,
		.flags  	= IORESOURCE_IO,
};
static struct platform_device gpio_device_C  = {
	.name			= DEV_NAME_GPIO,
	.id				= 2,
	.resource		= &gpio_resource_C,
	.num_resources  = 1,
};
#endif
#if	defined(CONFIG_GPIO_NXP_GROUP_D)
static struct resource gpio_resource_D = {
		.start  	=  0,
		.end   	 	= 32,
		.flags  	= IORESOURCE_IO,
};
static struct platform_device gpio_device_D  = {
	.name			= DEV_NAME_GPIO,
	.id				= 3,
	.resource		= &gpio_resource_D,
	.num_resources  = 1,
};
#endif
#if	defined(CONFIG_GPIO_NXP_GROUP_E)
static struct resource gpio_resource_E = {
		.start  	=  0,
		.end   	 	= 32,
		.flags  	= IORESOURCE_IO,
};
static struct platform_device gpio_device_E  = {
	.name			= DEV_NAME_GPIO,
	.id				= 4,
	.resource		= &gpio_resource_E,
	.num_resources  = 1,
};
#endif
#if	defined(CONFIG_GPIO_NXP_GROUP_ALV)
static struct resource gpio_resource_ALV = {
		.start  	=  0,
		.end   	 	=  8,
		.flags  	= IORESOURCE_IO,
};
static struct platform_device gpio_device_ALV  = {
	.name			= DEV_NAME_GPIO,
	.id				= 5,
	.resource		= &gpio_resource_ALV,
	.num_resources  = 1,
};
#endif

static struct platform_device *gpio_devices[] = {
	#if	defined(CONFIG_GPIO_NXP_GROUP_A)
	&gpio_device_A,
	#endif
	#if	defined(CONFIG_GPIO_NXP_GROUP_B)
	&gpio_device_B,
	#endif
	#if	defined(CONFIG_GPIO_NXP_GROUP_C)
	&gpio_device_C,
	#endif
	#if	defined(CONFIG_GPIO_NXP_GROUP_D)
	&gpio_device_D,
	#endif
	#if	defined(CONFIG_GPIO_NXP_GROUP_E)
	&gpio_device_E,
	#endif
	#if	defined(CONFIG_GPIO_NXP_GROUP_ALV)
	&gpio_device_ALV,
	#endif
};
#endif	/* CONFIG_GPIO_NXP */

/*------------------------------------------------------------------------------
 * Graphic OpenGL|ES platform device(vr400)
 */
#define VR_MEM_SIZE_DEFAULT CFG_MEM_PHY_SYSTEM_SIZE
#if defined( CFG_MEM_PHY_DMAZONE_SIZE )
#undef  VR_MEM_SIZE
#define VR_MEM_SIZE 	(VR_MEM_SIZE_DEFAULT + CFG_MEM_PHY_DMAZONE_SIZE)
#endif
#if defined( CONFIG_ION_NXP_CONTIGHEAP_SIZE )
#undef  VR_MEM_SIZE
#define VR_MEM_SIZE 	(VR_MEM_SIZE_DEFAULT - (CONFIG_ION_NXP_CONTIGHEAP_SIZE * 1024))
#endif

#define S5P4418_DTK_3D_IRQ     			(40)
static struct vr_gpu_device_data vr_gpu_data =
{
	.shared_mem_size = VR_MEM_SIZE,
#if 0
	.dedicated_mem_start = 0x5B400000, /* Reserved for graphic */
	.dedicated_mem_size = 0x04C00000, /* 76MB */
#endif

	/* DVFS */
#if 0
	.utilization_interval = 1000, /* ms */
	.utilization_handler =
#endif
	/* PMU power domain confituration */
	/* Mali Dynamic power domain configuration in sequence from 0-11
	 *  GP  PP0 PP1  PP2  PP3  PP4  PP5  PP6  PP7, L2$0 L2$1 L2$2
	 */
	.pmu_domain_config = {0x1, 0x2, 0x4, 0, 0, 0, 0, 0, 0, 0x1, 0, 0},
};

static struct resource vr_gpu_resources[] =
{
	VR_GPU_RESOURCES_VR400_MP2_PMU(PHY_BASEADDR_VR, S5P4418_DTK_3D_IRQ,
			S5P4418_DTK_3D_IRQ, S5P4418_DTK_3D_IRQ, S5P4418_DTK_3D_IRQ,
			S5P4418_DTK_3D_IRQ, S5P4418_DTK_3D_IRQ)
};

static struct platform_device vr_gpu_device =
{
	.name = VR_GPU_NAME_UTGARD,
	.id = 0,
	.dev.coherent_dma_mask = DMA_BIT_MASK(32),

	.dev.platform_data = &vr_gpu_data,

	.num_resources = ARRAY_SIZE(vr_gpu_resources),
	.resource = vr_gpu_resources,
};

/*------------------------------------------------------------------------------
 * Alsa sound platform device (I2S-PCM)
 */
#if defined(CONFIG_SND_NXP_I2S) || defined(CONFIG_SND_NXP_I2S_MODULE)
#ifndef CFG_AUDIO_I2S0_MASTER_CLOCK_IN
#define CFG_AUDIO_I2S0_MASTER_CLOCK_IN		0
#endif

#if defined (CONFIG_SND_NXP_I2S_CH0)
static struct nxp_i2s_plat_data i2s_data_ch0 = {
	.master_mode		= CFG_AUDIO_I2S0_MASTER_MODE,
	.master_clock_in	= CFG_AUDIO_I2S0_MASTER_CLOCK_IN,
	.trans_mode			= CFG_AUDIO_I2S0_TRANS_MODE,
	.frame_bit			= CFG_AUDIO_I2S0_FRAME_BIT,
	.sample_rate		= CFG_AUDIO_I2S0_SAMPLE_RATE,
	.pre_supply_mclk 	= CFG_AUDIO_I2S0_PRE_SUPPLY_MCLK,
	/* DMA */
	.dma_filter			= pl08x_filter_id,
	.dma_play_ch		= DMA_PERIPHERAL_NAME_I2S0_TX,
	.dma_capt_ch		= DMA_PERIPHERAL_NAME_I2S0_RX,
};

static struct platform_device i2s_device_ch0 = {
	.name	= DEV_NAME_I2S,
	.id		= 0,	/* channel */
	.dev    = {
		.platform_data	= &i2s_data_ch0
	},
};
#endif
#if defined (CONFIG_SND_NXP_I2S_CH1)
#ifndef CFG_AUDIO_I2S1_MASTER_CLOCK_IN
#define CFG_AUDIO_I2S1_MASTER_CLOCK_IN		0
#endif

static struct nxp_i2s_plat_data i2s_data_ch1 = {
	.master_mode		= CFG_AUDIO_I2S1_MASTER_MODE,
	.master_clock_in	= CFG_AUDIO_I2S1_MASTER_CLOCK_IN,
	.trans_mode			= CFG_AUDIO_I2S1_TRANS_MODE,
	.frame_bit			= CFG_AUDIO_I2S1_FRAME_BIT,
	.sample_rate		= CFG_AUDIO_I2S1_SAMPLE_RATE,
	.pre_supply_mclk 	= CFG_AUDIO_I2S1_PRE_SUPPLY_MCLK,
	/* DMA */
	.dma_filter			= pl08x_filter_id,
	.dma_play_ch		= DMA_PERIPHERAL_NAME_I2S1_TX,
	.dma_capt_ch		= DMA_PERIPHERAL_NAME_I2S1_RX,
};

static struct platform_device i2s_device_ch1 = {
	.name	= DEV_NAME_I2S,
	.id		= 1,	/* channel */
	.dev    = {
		.platform_data	= &i2s_data_ch1
	},
};
#endif
#if defined (CONFIG_SND_NXP_I2S_CH2)
#ifndef CFG_AUDIO_I2S2_MASTER_CLOCK_IN
#define CFG_AUDIO_I2S2_MASTER_CLOCK_IN		0
#endif

static struct nxp_i2s_plat_data i2s_data_ch2 = {
	.master_mode		= CFG_AUDIO_I2S2_MASTER_MODE,
	.master_clock_in	= CFG_AUDIO_I2S2_MASTER_CLOCK_IN,
	.trans_mode			= CFG_AUDIO_I2S2_TRANS_MODE,
	.frame_bit			= CFG_AUDIO_I2S2_FRAME_BIT,
	.sample_rate		= CFG_AUDIO_I2S2_SAMPLE_RATE,
	.pre_supply_mclk 	= CFG_AUDIO_I2S2_PRE_SUPPLY_MCLK,
	/* DMA */
	.dma_filter			= pl08x_filter_id,
	.dma_play_ch		= DMA_PERIPHERAL_NAME_I2S2_TX,
	.dma_capt_ch		= DMA_PERIPHERAL_NAME_I2S2_RX,
};

static struct platform_device i2s_device_ch2 = {
	.name	= DEV_NAME_I2S,
	.id		= 2,	/* channel */
	.dev    = {
		.platform_data	= &i2s_data_ch2
	},
};
#endif

static struct platform_device *i2s_devices[] __initdata = {
	#if defined (CONFIG_SND_NXP_I2S_CH0)
	&i2s_device_ch0,
	#endif
	#if defined (CONFIG_SND_NXP_I2S_CH1)
	&i2s_device_ch1,
	#endif
	#if defined (CONFIG_SND_NXP_I2S_CH2)
	&i2s_device_ch2,
	#endif
};
#endif /* CONFIG_SND_NXP_I2S || CONFIG_SND_NXP_I2S_MODULE */

/*------------------------------------------------------------------------------
 * Alsa sound platform device (SPDIF)
 */
#if defined(CONFIG_SND_NXP_SPDIF_TX) || defined(CONFIG_SND_NXP_SPDIF_TX_MODULE)
static struct nxp_spdif_plat_data spdif_data_tx = {
	.sample_rate	= CFG_AUDIO_SPDIF_TX_SAMPLE_RATE,
	.hdmi_out		= CFG_AUDIO_SPDIF_TX_HDMI_OUT,
	.dma_filter		= pl08x_filter_id,
	.dma_ch			= DMA_PERIPHERAL_NAME_SPDIFTX,
};

static struct platform_device spdif_device_tx = {
	.name	= DEV_NAME_SPDIF_TX,
	.id		= -1,
	.dev    = {
		.platform_data	= &spdif_data_tx
	},
};
#endif	/* CONFIG_SND_NXP_SPDIF_TX || CONFIG_SND_NXP_SPDIF_TX_MODULE */

#if defined(CONFIG_SND_NXP_SPDIF_RX) || defined(CONFIG_SND_NXP_SPDIF_RX_MODULE)
static struct nxp_spdif_plat_data spdif_data_rx = {
	.sample_rate	= CFG_AUDIO_SPDIF_RX_SAMPLE_RATE,
	.dma_filter		= pl08x_filter_id,
	.dma_ch			= DMA_PERIPHERAL_NAME_SPDIFRX,
};

static struct platform_device spdif_device_rx = {
	.name	= DEV_NAME_SPDIF_RX,
	.id		= -1,
	.dev    = {
		.platform_data	= &spdif_data_rx
	},
};
#endif	/* CONFIG_SND_NXP_SPDIF_RX || CONFIG_SND_NXP_SPDIF_RX_MODULE */

/*------------------------------------------------------------------------------
 * Alsa sound platform device (PDM)
 */
#if defined(CONFIG_SND_NXP_PDM) || defined(CONFIG_SND_NXP_PDM_MODULE)
static struct nxp_pdm_plat_data pdm_data = {
	.sample_rate	= CFG_AUDIO_PDM_SAMPLE_RATE,
	.dma_filter		= pl08x_filter_id,
	.dma_ch			= DMA_PERIPHERAL_NAME_PDM,
};

static struct platform_device pdm_device = {
	.name	= DEV_NAME_PDM,
	.id		= -1,
	.dev    = {
		.platform_data	= &pdm_data
	},
};
#endif	/* CONFIG_SND_NXP_PDM || CONFIG_SND_NXP_PDM_MODULE */

/*------------------------------------------------------------------------------
 * SSP/SPI
 */
#if defined(CONFIG_SPI_PL022) || defined(CONFIG_SPI_PL022_MODULE)
#include <linux/amba/pl022.h>

static	const int reset[3][2] = {
    {RESET_ID_SSP0_P,RESET_ID_SSP0} ,
    {RESET_ID_SSP1_P,RESET_ID_SSP1} ,
    {RESET_ID_SSP2_P,RESET_ID_SSP2} ,
};
static void spi_init(int ch)
{
	char name[10] = {0};
	int req_clk = 0;
	struct clk *clk ;

	if(0 == ch)
			req_clk = CFG_SPI0_CLK;
	else if(1 == ch)
			req_clk = CFG_SPI1_CLK;
	else if(2 == ch)
			req_clk = CFG_SPI2_CLK;

	sprintf(name,"nxp-spi.%d",(unsigned char)ch);

	clk = clk_get(NULL,name);
	clk_set_rate(clk,req_clk);

	nxp_soc_peri_reset_enter(reset[ch][0]);
    nxp_soc_peri_reset_enter(reset[ch][1]);
	nxp_soc_peri_reset_exit (reset[ch][0]);
    nxp_soc_peri_reset_exit (reset[ch][1]);
	clk_enable(clk);
}

#if defined(CONFIG_SPI_PL022_PORT0)

static struct pl022_ssp_controller ssp0_platform_data = {
    .bus_id         = 0,
    .num_chipselect = 1,
#if defined(CONFIG_USE_DMA_PORT0) && defined(CONFIG_AMBA_PL08X)
    .enable_dma     = 1,
    .dma_filter     = pl08x_filter_id,
    .dma_rx_param   = (void *)DMA_PERIPHERAL_NAME_SSP0_RX,
    .dma_tx_param   = (void *)DMA_PERIPHERAL_NAME_SSP0_TX,
    .autosuspend_delay 	= 10,
    .rt = 0,
#else
    .enable_dma     = 0,
#endif
	.init 			= (void *)spi_init,
};

static struct amba_device spi0_device = {
    .dev = {
	        .init_name          = "ssp-pl022.0",
	        .coherent_dma_mask  = 0x000fffffUL,
	        .platform_data      = &ssp0_platform_data,
	        .id = 0,
		   },
	.res = {
	        .start = PHY_BASEADDR_SSP0 ,
	        .end   = PHY_BASEADDR_SSP0  + SZ_1K - 1,
	        .flags = IORESOURCE_MEM,
		    },

	.irq = {IRQ_PHY_SSP0 },
	.periphid = 0x00041022,                     /* ST-Ericsson modified id */
};
#endif


#if defined(CONFIG_SPI_PL022_PORT1)

static struct pl022_ssp_controller ssp1_platform_data = {
    .bus_id         = 1,
    .num_chipselect = 2,
#if defined(CONFIG_USE_DMA_PORT1) && defined(CONFIG_AMBA_PL08X)
    .enable_dma     = 1,
    .dma_filter     = pl08x_filter_id,
    .dma_rx_param   = DMA_PERIPHERAL_NAME_SSP1_RX,
    .dma_tx_param   = DMA_PERIPHERAL_NAME_SSP1_TX,
#else
    .enable_dma     = 0,
#endif
	.init 			= (void *)spi_init,
};

static struct amba_device spi1_device = {
    .dev = {
	        .init_name          = "ssp-pl022.1",
	        .coherent_dma_mask  = 0x000fffffUL,
	        .platform_data      = &ssp1_platform_data,
	        .id = 1,
		   },
	.res = {
	        .start = PHY_BASEADDR_SSP1 ,
	        .end   = PHY_BASEADDR_SSP1  + SZ_1K - 1,
	        .flags = IORESOURCE_MEM,
		    },

	.irq = {IRQ_PHY_SSP1 },
	.periphid = 0x00041022,                     /* ST-Ericsson modified id */
};
#endif


#if defined(CONFIG_SPI_PL022_PORT2)

static struct pl022_ssp_controller ssp2_platform_data = {
    .bus_id         = 2,
    .num_chipselect = 3,
#if defined(CONFIG_USE_DMA_PORT2) && defined(CONFIG_AMBA_PL08X)
    .enable_dma     = 1,
    .dma_filter     = pl08x_filter_id,
    .dma_rx_param   = DMA_PERIPHERAL_NAME_SSP2_RX,
    .dma_tx_param   = DMA_PERIPHERAL_NAME_SSP2_TX,
#else
    .enable_dma     = 0,
#endif
	.init 			= (void *)spi_init,
};

static struct amba_device spi2_device = {
    .dev = {
	        .init_name          = "ssp-pl022.2",
	        .coherent_dma_mask  = 0x000fffffUL,
	        .platform_data      = &ssp2_platform_data,
	        .id = 1,
		   },
	.res = {
	        .start = PHY_BASEADDR_SSP2 ,
	        .end   = PHY_BASEADDR_SSP2  + SZ_1K - 1,
	        .flags = IORESOURCE_MEM,
		    },

	.irq = {IRQ_PHY_SSP2 },
	.periphid = 0x00041022,                     /* ST-Ericsson modified id */
};
#endif

#endif /* CONFIG_SPI_PL022 */

/*------------------------------------------------------------------------------
 * USB device (EHCI/OHCI)
 */

#if defined(CONFIG_USB_EHCI_SYNOPSYS)
#include <mach/usb-phy.h>

extern int nxp_hsic_phy_pwr_on(struct platform_device *pdev, bool on);


/* USB EHCI Host Controller registration */

static struct resource nxp_ehci_resource[] = {
    [0] = DEFINE_RES_MEM(PHY_BASEADDR_EHCI, SZ_256),
    [1] = DEFINE_RES_IRQ(IRQ_PHY_USB20HOST),
};

struct nxp_ehci_platdata nxp_ehci_plat_data = {
    .phy_init = nxp_usb_phy_init,
    .phy_exit = nxp_usb_phy_exit,
    .hsic_phy_pwr_on = nxp_hsic_phy_pwr_on,
};

static u64 nxp_ehci_dmamask = DMA_BIT_MASK(32);

struct platform_device nxp_device_ehci = {
    .name           = "nxp-ehci",
    .id             = -1,
    .num_resources  = ARRAY_SIZE(nxp_ehci_resource),
    .resource       = nxp_ehci_resource,
    .dev            = {
        .dma_mask           = &nxp_ehci_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
        .platform_data      = &nxp_ehci_plat_data,
    }
};
#endif  /* CONFIG_USB_EHCI_SYNOPSYS */

#if defined(CONFIG_USB_OHCI_SYNOPSYS)
#include <mach/usb-phy.h>

/* USB OHCI Host Controller registration */

static struct resource nxp_ohci_resource[] = {
    [0] = DEFINE_RES_MEM(PHY_BASEADDR_OHCI, SZ_256),
    [1] = DEFINE_RES_IRQ(IRQ_PHY_USB20HOST),
};

struct nxp_ohci_platdata nxp_ohci_plat_data = {
    .phy_init = nxp_usb_phy_init,
    .phy_exit = nxp_usb_phy_exit,
};

static u64 nxp_ohci_dmamask = DMA_BIT_MASK(32);

struct platform_device nxp_device_ohci = {
    .name           = "nxp-ohci",
    .id             = -1,
    .num_resources  = ARRAY_SIZE(nxp_ohci_resource),
    .resource       = nxp_ohci_resource,
    .dev            = {
        .dma_mask           = &nxp_ohci_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
        .platform_data      = &nxp_ohci_plat_data,
    }
};
#endif  /* CONFIG_USB_OHCI_SYNOPSYS */

/*------------------------------------------------------------------------------
 * USB OTG Host or Gadget
 */
#if defined(CONFIG_USB_DWCOTG)

#include <linux/delay.h>
#include <asm/io.h>
#include <linux/usb/ch9.h>

#define SOC_PA_HSOTG        PHY_BASEADDR_HSOTG
#define SOC_VA_HSOTG        IO_ADDRESS(SOC_PA_HSOTG)

#define SOC_PA_RSTCON       PHY_BASEADDR_RSTCON
#define SOC_VA_RSTCON       IO_ADDRESS(SOC_PA_RSTCON)

#define SOC_PA_TIEOFF       PHY_BASEADDR_TIEOFF
#define SOC_VA_TIEOFF       IO_ADDRESS(SOC_PA_TIEOFF)

void otg_clk_enable(void)
{
#if 0
    struct clk *hsotg_clk;

    // Clock control
    hsotg_clk = clk_get(NULL, DEV_NAME_USB2HOST);
    if (hsotg_clk)
        clk_enable(hsotg_clk);
#endif
}
EXPORT_SYMBOL(otg_clk_enable);

void otg_clk_disable(void)
{
#if 0
    struct clk *hsotg_clk;

    // Clock control
    hsotg_clk = clk_get(NULL, DEV_NAME_USB2HOST);
    if (hsotg_clk)
        clk_disable(hsotg_clk);
#endif
}
EXPORT_SYMBOL(otg_clk_disable);

/* Initializes OTG Phy. */
void otg_phy_init(void)
{
    u32 temp;

    PM_DBGOUT("+%s\n", __func__);

    writel(readl(SOC_VA_RSTCON + 0x3C) & ~0xF800, SOC_VA_RSTCON + 0x3C);

    // 1. Release otg common reset
    writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<25), SOC_VA_RSTCON + 0x04);     // reset on
    udelay(10);
    writel(readl(SOC_VA_RSTCON + 0x04) |  (1<<25), SOC_VA_RSTCON + 0x04);     // reset off
    udelay(10);

    // 1-1. VBUS reconfig - Over current Issue
#if 1
    temp  = readl(SOC_VA_TIEOFF + 0x38) & ~(0x7<<23);
#if defined(CFG_OTG_OVC_VALUE)
    temp |= (CFG_OTG_OVC_VALUE << 23);
#else
    temp |= (0x3<<23);
#endif
    writel(temp, SOC_VA_TIEOFF + 0x38);
#endif

    // 2. Program scale mode to real mode
    writel(readl(SOC_VA_TIEOFF + 0x30) & ~(3<<0), SOC_VA_TIEOFF + 0x30);

    // 3. Select word interface and enable word interface selection
#if 0
    writel(readl(SOC_VA_TIEOFF + 0x38) & ~(3<<8), SOC_VA_TIEOFF + 0x38);
    writel(readl(SOC_VA_TIEOFF + 0x38) |  (1<<8), SOC_VA_TIEOFF + 0x38);        // 2'b01 8bit, 2'b11 16bit word
#else
    writel(readl(SOC_VA_TIEOFF + 0x38) |  (3<<8), SOC_VA_TIEOFF + 0x38);        // 2'b01 8bit, 2'b11 16bit word
#endif


    // 4. Select VBUS
//    writel(readl(SOC_VA_TIEOFF + 0x34) |  (3<<24), SOC_VA_TIEOFF + 0x34);   /* Select VBUS 3.3V */
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(3<<24), SOC_VA_TIEOFF + 0x34);   /* Select VBUS 5V */

    // 5. POR of PHY
#if 0
    writel(readl(SOC_VA_TIEOFF + 0x34) |  (3<<7), SOC_VA_TIEOFF + 0x34);
#else
    temp    = readl(SOC_VA_TIEOFF + 0x34);
    temp   &= ~(3<<7);
    temp   |=  (1<<7);
    writel(temp, SOC_VA_TIEOFF + 0x34);
    udelay(1);
    temp   |=  (3<<7);
    writel(temp, SOC_VA_TIEOFF + 0x34);
    udelay(1);
    temp   &= ~(2<<7);
    writel(temp, SOC_VA_TIEOFF + 0x34);
#endif
    udelay(10); // 40us delay need.

    // 6. UTMI reset
    writel(readl(SOC_VA_TIEOFF + 0x34) | (1<<3), SOC_VA_TIEOFF + 0x34);
    udelay(1);  // 10 clock need

    // 7. AHB reset
    writel(readl(SOC_VA_TIEOFF + 0x34) | (1<<2), SOC_VA_TIEOFF + 0x34);
    udelay(1);  // 10 clock need
}
EXPORT_SYMBOL(otg_phy_init);

void otg_phy_off(void)
{
    PM_DBGOUT("+%s\n", __func__);

    // 0. Select VBUS
    writel(readl(SOC_VA_TIEOFF + 0x34) |  (3<<24), SOC_VA_TIEOFF + 0x34);   /* Select VBUS 3.3V */
//    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(3<<24), SOC_VA_TIEOFF + 0x34);   /* Select VBUS 5V */

    // 1. UTMI reset
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(1<<3), SOC_VA_TIEOFF + 0x34);
    udelay(10); // 10 clock need

    // 2. AHB reset
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(1<<2), SOC_VA_TIEOFF + 0x34);
    udelay(10); // 10 clock need

    // 3. POR of PHY
    writel(readl(SOC_VA_TIEOFF + 0x34) |  (3<<7), SOC_VA_TIEOFF + 0x34);
    udelay(10); // 40us delay need.

    // 4. Release otg common reset
    writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<25), SOC_VA_RSTCON + 0x04);     // reset on
    udelay(10);
}
EXPORT_SYMBOL(otg_phy_off);

void otg_phy_suspend(void)
{
    u32 temp;

    // 0. Select VBUS
    writel(readl(SOC_VA_TIEOFF + 0x34) |  (3<<24), SOC_VA_TIEOFF + 0x34);   /* Select VBUS 3.3V */
//    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(3<<24), SOC_VA_TIEOFF + 0x34);   /* Select VBUS 5V */

    // 1. AHB reset
#if 0
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(1<<2), SOC_VA_TIEOFF + 0x34);
    udelay(10); // 10 clock need
#endif

    // 2. UTMI reset
#if 0
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(1<<3), SOC_VA_TIEOFF + 0x34);
    udelay(10); // 10 clock need
#endif

    // 3. Release otg common reset
    writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<25), SOC_VA_RSTCON + 0x04);     // reset on
    udelay(10);

    // 4. POR of PHY
#if 0
#if 0
    writel(readl(SOC_VA_TIEOFF + 0x34) |  (2<<7), SOC_VA_TIEOFF + 0x34);
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(1<<7), SOC_VA_TIEOFF + 0x34);
#else
    writel(readl(SOC_VA_TIEOFF + 0x34) & ~(3<<7), SOC_VA_TIEOFF + 0x34);
    writel(readl(SOC_VA_TIEOFF + 0x34) |  (2<<7), SOC_VA_TIEOFF + 0x34);
#endif
#else

    temp    = readl(SOC_VA_TIEOFF + 0x34);
    temp   &= ~(3<<7);
    temp   |=  (2<<7);
    writel(temp, SOC_VA_TIEOFF + 0x34);
#endif
    udelay(40); // 40us delay need.
}
EXPORT_SYMBOL(otg_phy_suspend);

static struct resource otg_resources[] = {
    [0] = DEFINE_RES_MEM(PHY_BASEADDR_HSOTG, SZ_64K + SZ_4K),
    [1] = DEFINE_RES_IRQ(IRQ_PHY_USB20OTG),
};

static u64 otg_dmamask = DMA_BIT_MASK(32);

static struct platform_device otg_plat_device = {
    .name   = "dwc_otg",
    .id     = -1,
    .dev    = {
        .dma_mask = &otg_dmamask,
        .coherent_dma_mask = 0xffffffffUL
    },
    .num_resources  = ARRAY_SIZE(otg_resources),
    .resource       = otg_resources,
};

#define CFG_SWITCH_USB_5V_EN        (PAD_GPIO_D + 10)
#define CFG_SWITCH_USB_HOST_DEVICE  (PAD_GPIO_D + 11)
#define CFG_OTG_MODE_HOST           1
#define CFG_OTG_MODE_DEVICE         0
#define CFG_OTG_BOOT_MODE           CFG_OTG_MODE_DEVICE

static int cur_otg_mode = CFG_OTG_BOOT_MODE;

unsigned int get_otg_mode(void)
{
    return cur_otg_mode;
}

void set_otg_mode(unsigned int mode, int is_force)
{
    if (mode > CFG_OTG_MODE_HOST) return;

    if ((mode == cur_otg_mode) && !is_force) return;

    cur_otg_mode = mode;

    return;
}

EXPORT_SYMBOL(get_otg_mode);
EXPORT_SYMBOL(set_otg_mode);
#endif/* CONFIG_USB_DWCOTG */

/*------------------------------------------------------------------------------
 * The order of device declaration may be important, since some devices
 * have dependencies on other devices being initialized first.
 */
#if defined(CONFIG_ARM_AMBA)

#include "dev-dmac.c"

static struct amba_device *amba_devices[] __initdata = {
#if defined(CONFIG_SERIAL_AMBA_PL011)
	#if defined(CONFIG_SERIAL_NXP_UART0)
	&uart0_device,
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART1)
	&uart1_device,
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART2)
	&uart2_device,
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART3)
	&uart3_device,
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART4)
	&uart4_device,
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART5)
	&uart5_device,
	#endif
#endif

#if defined (CONFIG_AMBA_PL08X)
	&dmac0_device,
	&dmac1_device,
#endif

#if defined(CONFIG_SPI_PL022_PORT0)
	&spi0_device,
#endif

#if defined(CONFIG_SPI_PL022_PORT1)
	&spi1_device,
#endif

#if defined(CONFIG_SPI_PL022_PORT2)
	&spi2_device,
#endif
};
#endif

/*------------------------------------------------------------------------------
 *  * ION device
 *   */
#if defined(CONFIG_ION_NXP)

#include <mach/ion.h>

struct platform_device nxp_device_ion = {
    .name   = "ion-nxp",
    .id     = -1,
};

#endif /* CONFIG_ION_NXP */

/*------------------------------------------------------------------------------
 * ADC
 */
#if defined(CONFIG_NXP_ADC)

#ifndef CFG_ADC_SAMPLE_RATE
#define CFG_ADC_SAMPLE_RATE		200000	/* 200K */
#endif
static unsigned long adc_sample_rate = CFG_ADC_SAMPLE_RATE;

static struct resource nxp_adc_resource[] = {
	[0] = DEFINE_RES_MEM(PHY_BASEADDR_ADC, SZ_1K),
	[1] = DEFINE_RES_IRQ(IRQ_PHY_ADC),
};

static struct platform_device nxp_adc_device = {
	.name			= DEV_NAME_ADC,
	.id				= -1,
	.num_resources	= ARRAY_SIZE(nxp_adc_resource),
	.resource		= nxp_adc_resource,
	.dev  			= {
		.platform_data	= &adc_sample_rate,
	},
};
#endif /* CONFIG_NXP_ADC */

/*------------------------------------------------------------------------------
 * Display
 */
#include "dev-display.c"

/*------------------------------------------------------------------------------
 * Watchdog Timer
 */

#if defined(CONFIG_NXP_WDT)

static struct resource nxp_wdt_resource[] = {
        [0] = DEFINE_RES_MEM(PHY_BASEADDR_WDT, SZ_1K),
        [1] = DEFINE_RES_IRQ(IRQ_PHY_WDT),
};

struct platform_device nxp_device_wdt = {
        .name           = DEV_NAME_WDT,
        .id             = -1,
        .num_resources  = ARRAY_SIZE(nxp_wdt_resource),
        .resource       = nxp_wdt_resource,
};
#endif

/*------------------------------------------------------------------------------
 * register cpu platform devices
 */
void __init nxp_cpu_devices_register(void)
{
	int i = 0;
	printk("[Register machine platform devices]\n");
#if defined(CONFIG_ARM_AMBA)
	for (i = 0; i < ARRAY_SIZE(amba_devices); i++) {
	struct amba_device *d = amba_devices[i];
	printk("mach: add amba device %s \n", d->dev.init_name);
	amba_device_register(d, &iomem_resource);
	}
#endif
	/* default uart hw prepare */
	#if defined(CONFIG_SERIAL_NXP_UART0)
	NX_UART_CH_INIT(0);
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART1)
	NX_UART_CH_INIT(1);
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART2)
	NX_UART_CH_INIT(2);
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART3)
	NX_UART_CH_INIT(3);
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART4)
	NX_UART_CH_INIT(4);
	#endif
	#if defined(CONFIG_SERIAL_NXP_UART5)
	NX_UART_CH_INIT(5);
	#endif

#if defined(CONFIG_NXP_DISPLAY)
    printk("mach: add device syncgen [%d]\n", ARRAY_SIZE(syncgen_devices));
    platform_add_devices(syncgen_devices, ARRAY_SIZE(syncgen_devices));
#endif

#if defined(CONFIG_NXP_DISPLAY_LCD)
	printk("mach: add device lcd \n");
	platform_device_register(&lcd_device);
#endif

#if defined(CONFIG_NXP_DISPLAY_LVDS)
	printk("mach: add device lvds \n");
	platform_device_register(&lvds_device);
#endif

#if defined(CONFIG_NXP_DISPLAY_MIPI)
	printk("mach: add device mipi \n");
	platform_device_register(&mipi_device);
#endif

#if defined(CONFIG_NXP_DISPLAY_HDMI)
	printk("mach: add device hdmi \n");
	platform_device_register(&hdmi_device);
#endif

#if defined(CONFIG_NXP_DISPLAY_RESC)
	printk("mach: add device resolution convertor \n");
	platform_device_register(&resc_device);
#endif

#if defined(CONFIG_SERIAL_NXP)
    printk("mach: add device serial (array:%d)\n", ARRAY_SIZE(uart_devices));
    platform_add_devices(uart_devices, ARRAY_SIZE(uart_devices));
#endif

#if defined(CONFIG_I2C_NXP)
    printk("mach: add device i2c bus (array:%d) \n", ARRAY_SIZE(i2c_devices));
    platform_add_devices(i2c_devices, ARRAY_SIZE(i2c_devices));
#endif
#if defined(CONFIG_RTC_DRV_NXP)
    printk("mach: add device Real Time Clock  \n");
    platform_device_register(&rtc_plat_device);
#endif

#if defined(CONFIG_HAVE_PWM)
    printk("mach: add device generic pwm (array:%d)\n", ARRAY_SIZE(pwm_devices));
    platform_add_devices(pwm_devices, ARRAY_SIZE(pwm_devices));
#endif

#if defined(CONFIG_GPIO_NXP)
    printk("mach: add device generic gpio (array:%d)\n", ARRAY_SIZE(gpio_devices));
    platform_add_devices(gpio_devices, ARRAY_SIZE(gpio_devices));
#endif

#if defined(CONFIG_SND_NXP_I2S) || defined(CONFIG_SND_NXP_I2S_MODULE)
    printk("mach: add device i2s (array:%d) \n", ARRAY_SIZE(i2s_devices));
    platform_add_devices(i2s_devices, ARRAY_SIZE(i2s_devices));
#endif

#if defined(CONFIG_SND_NXP_SPDIF_TX) || defined(CONFIG_SND_NXP_SPDIF_TX_MODULE)
    printk("mach: add device spdif tx\n");
    platform_device_register(&spdif_device_tx);
#endif

#if defined(CONFIG_SND_NXP_SPDIF_RX) || defined(CONFIG_SND_NXP_SPDIF_RX_MODULE)
    printk("mach: add device spdif rx\n");
    platform_device_register(&spdif_device_rx);
#endif

#if defined(CONFIG_SND_NXP_PDM) || defined(CONFIG_SND_NXP_PDM_MODULE)
    printk("mach: add device pdm\n");
    platform_device_register(&pdm_device);
#endif

#if defined(CONFIG_USB_EHCI_SYNOPSYS)
    printk("mach: add device usb_ehci\n");
    platform_device_register(&nxp_device_ehci);
#endif

#if defined(CONFIG_USB_OHCI_SYNOPSYS)
    printk("mach: add device usb_ohci\n");
    platform_device_register(&nxp_device_ohci);
#endif

#if defined(CONFIG_USB_DWCOTG)
    printk("mach: add device usb otg\n");
    platform_device_register(&otg_plat_device);
#endif

#if defined(CONFIG_ION_NXP)
    printk("mach: add device ion-nxp\n");
    nxp_ion_set_platdata();
    platform_device_register(&nxp_device_ion);
#endif

#if defined(CONFIG_NXP_ADC)
    printk("mach: add device adc\n");
    platform_device_register(&nxp_adc_device);
#endif
    /* Register the platform devices */
    printk("mach: add graphic device opengl|es\n");
    platform_device_register(&vr_gpu_device);

#if defined(CONFIG_NXP_WDT)
    printk("mach: add device watchdog\n");
    platform_device_register(&nxp_device_wdt);
#endif

#ifdef CONFIG_PM_RUNTIME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		pm_runtime_set_autosuspend_delay(&(vr_gpu_device.dev), 1000);
		pm_runtime_use_autosuspend(&(vr_gpu_device.dev));
#endif
		pm_runtime_enable(&(vr_gpu_device.dev));
#endif
}
