/*
 * Driver for Samsung SoC onboard UARTs.
 *
 * Ben Dooks, Copyright (c) 2003-2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __NXP_S3C_H__
#define __NXP_S3C_H__


#ifdef CONFIG_SERIAL_NXP_S3C_UARTS
#define	UART_NR		CONFIG_SERIAL_NXP_S3C_UARTS
#else
#define	UART_NR		6
#endif

struct s3c24xx_uart_info {
	char *name;
	unsigned int type;
	unsigned int fifosize;
	unsigned long rx_fifomask;
	unsigned long rx_fifoshift;
	unsigned long rx_fifofull;
	unsigned long tx_fifomask;
	unsigned long tx_fifoshift;
	unsigned long tx_fifofull;
	unsigned int  def_clk_sel;
	unsigned long num_clks;
	unsigned long clksel_mask;
	unsigned long clksel_shift;
	/* uart port features */
	unsigned int has_divslot:1;
	/* uart controls */
	int (*reset_port)(struct uart_port *, struct s3c24xx_uart_platdata *);
};

struct s3c24xx_serial_drv_data {
	struct s3c24xx_uart_info *info;
	struct s3c24xx_uart_platdata *def_data;
	unsigned int fifosize[UART_NR];
};
#ifdef CONFIG_DMA_ENGINE
struct s3c24xx_sgbuf {
	struct scatterlist sg;
	char *buf;
};

struct s3c24xx_dmarx_data {
	struct dma_chan		*chan;
	struct completion	complete;
	bool			use_buf_b;
	struct s3c24xx_sgbuf	sgbuf_a;
	struct s3c24xx_sgbuf	sgbuf_b;
	dma_cookie_t		cookie;
	bool			running;
};

struct s3c24xx_dmatx_data {
	struct dma_chan		*chan;
	struct scatterlist	sg;
	char			*buf;
	bool			queued;
};

#endif

struct s3c24xx_uart_port {
	unsigned char rx_claimed;
	unsigned char tx_claimed;
	unsigned int pm_level;
	unsigned long baudclk_rate;
	unsigned int rx_irq;
	unsigned int tx_irq;
	struct s3c24xx_uart_info *info;
	struct clk *clk;
	struct clk *baudclk;
	struct uart_port port;
	struct s3c24xx_serial_drv_data *drv_data;
	struct delayed_work resume_work;
	struct wake_lock resume_lock;
	/* reference to platform data */
	struct s3c24xx_uart_platdata *data;

	unsigned int im;
	unsigned int dmacr;
	/* DMA stuff */
#ifdef CONFIG_DMA_ENGINE
	bool			using_tx_dma;
	bool			using_rx_dma;
	struct s3c24xx_dmarx_data dmarx;
	struct s3c24xx_dmatx_data	dmatx;
#endif
};

/* conversion functions */

#define s3c24xx_dev_to_port(__dev) (struct uart_port *)dev_get_drvdata(__dev)

/* register access controls */

#define portaddr(port, reg) ((port)->membase + (reg))
#define portaddrl(port, reg) ((unsigned long *)((port)->membase + (reg)))

#define rd_regb(port, reg) (__raw_readb(portaddr(port, reg)))
#define rd_regl(port, reg) (__raw_readl(portaddr(port, reg)))

#define wr_regb(port, reg, val) __raw_writeb(val, portaddr(port, reg))
#define wr_regl(port, reg, val) __raw_writel(val, portaddr(port, reg))

#if 0
extern void printascii(const char *);
static void dbg(const char *fmt, ...)
{
	va_list va;
	char buff[256];

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	printascii(buff);
}
#endif

#endif	/* __NXP_S3C_H__ */
