/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/amba/serial.h>

#include <mach/platform.h>
#include <mach/soc.h>

/*
 * Macro
 */
#define	UART_DEBUG_HZ				CFG_UART_CLKGEN_CLOCK_HZ
#define	UART_DEBUG_BAUDRATE			CFG_UART_DEBUG_BAUDRATE
#if	defined  (CONFIG_DEBUG_NXP_UART_CH0)
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART0)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN22)
	#define	RESET_UART_ID		RESET_ID_UART0
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART0_USESMC
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART0_SMCTXENB
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART0_SMCRXENB
#elif defined(CONFIG_DEBUG_NXP_UART_CH1)
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART1)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN24)
	#define	RESET_UART_ID		RESET_ID_UART1
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART_MODEM0_USESMC
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART_MODEM0_SMCTXENB
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART_MODEM0_SMCRXENB
#elif defined(CONFIG_DEBUG_NXP_UART_CH2)
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART2)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN23)
	#define	RESET_UART_ID		RESET_ID_UART2
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART1_USESMC
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART1_SMCTXENB
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART1_SMCRXENB
#elif defined(CONFIG_DEBUG_NXP_UART_CH3)
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART3)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN25)
	#define	RESET_UART_ID		RESET_ID_UART3
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART_NODMA0_USESMC
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART_NODMA0_SMCTXENB
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART_NODMA0_SMCRXENB
#elif defined(CONFIG_DEBUG_NXP_UART_CH4)
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART4)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN26)
	#define	RESET_UART_ID		RESET_ID_UART4
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART_NODMA1_USESMC
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART_NODMA1_SMCTXENB
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART_NODMA1_SMCRXENB
#elif defined(CONFIG_DEBUG_NXP_UART_CH5)
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART5)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN27)
	#define	RESET_UART_ID		RESET_ID_UART5
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART_NODMA2_USESMC
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART_NODMA2_SMCTXENB
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART_NODMA2_SMCRXENB
#else
	#define	UART_PHYS_BASE		IO_ADDRESS(PHY_BASEADDR_UART0)
	#define	UART_CLKG_BASE		IO_ADDRESS(PHY_BASEADDR_CLKGEN22)
	#define	RESET_UART_ID		RESET_ID_UART0
	#define	TIEOFF_USESMC		TIEOFFINDEX_OF_UART0_USESMC		/* Use UART for SmartCard Interface */
	#define	TIEOFF_SMCTXENB		TIEOFFINDEX_OF_UART0_SMCTXENB	/* SmartCard Interface TX mode enable */
	#define	TIEOFF_SMCRXENB		TIEOFFINDEX_OF_UART0_SMCRXENB	/* SmartCard Interface RX mode enable */
#endif


/*
 * Registers
 */
#define UART_PL01x_FR_TXFE              0x80
#define UART_PL01x_FR_RXFF              0x40
#define UART_PL01x_FR_TXFF              0x20
#define UART_PL01x_FR_RXFE              0x10
#define UART_PL01x_FR_BUSY              0x08
#define UART_PL01x_FR_TMSK              (UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

#define UART_PL011_LCRH_SPS             (1 << 7)
#define UART_PL011_LCRH_WLEN_8          (3 << 5)
#define UART_PL011_LCRH_WLEN_7          (2 << 5)
#define UART_PL011_LCRH_WLEN_6          (1 << 5)
#define UART_PL011_LCRH_WLEN_5          (0 << 5)
#define UART_PL011_LCRH_FEN             (1 << 4)
#define UART_PL011_LCRH_STP2            (1 << 3)
#define UART_PL011_LCRH_EPS             (1 << 2)
#define UART_PL011_LCRH_PEN             (1 << 1)
#define UART_PL011_LCRH_BRK             (1 << 0)

#define UART_PL011_CR_CTSEN      	(1 << 15)
#define UART_PL011_CR_RTSEN         (1 << 14)
#define UART_PL011_CR_OUT2          (1 << 13)
#define UART_PL011_CR_OUT1          (1 << 12)
#define UART_PL011_CR_RTS           (1 << 11)
#define UART_PL011_CR_DTR           (1 << 10)
#define UART_PL011_CR_RXE           (1 << 9)
#define UART_PL011_CR_TXE           (1 << 8)
#define UART_PL011_CR_LPE           (1 << 7)
#define UART_PL011_CR_IIRLP         (1 << 2)
#define UART_PL011_CR_SIREN         (1 << 1)
#define UART_PL011_CR_UARTEN        (1 << 0)

struct pl01x_regs {
    u32 dr;    		/* 0x00 Data register */
    u32 ecr;        /* 0x04 Error clear register (Write) */
    u32 pl010_lcrh; /* 0x08 Line control register, high byte */
    u32 pl010_lcrm; /* 0x0C Line control register, middle byte */
    u32 pl010_lcrl; /* 0x10 Line control register, low byte */
    u32 pl010_cr;   /* 0x14 Control register */
    u32 fr;     	/* 0x18 Flag register (Read only) */
#ifdef CONFIG_PL011_SERIAL_RLCR
    u32 pl011_rlcr; /* 0x1c Receive line control register */
#else
    u32 reserved;
#endif
    u32 ilpr;       /* 0x20 IrDA low-power counter register */
    u32 pl011_ibrd; /* 0x24 Integer baud rate register */
    u32 pl011_fbrd; /* 0x28 Fractional baud rate register */
    u32 pl011_lcrh; /* 0x2C Line control register */
    u32 pl011_cr;   /* 0x30 Control register */
};

struct uart_data {
	/* clkgen */
	int pll;
	int div;
	long rate;
	/* uart */
	unsigned int divider;
	unsigned int fraction;
	unsigned int lcr;
	unsigned int cr;
};

/*
 * Low level debug function.
 * default debug port is '0'
 */
static struct uart_data clk = { 0 ,};

#define	MAX_DIVIDER			((1<<8) - 1)	// 256, align 2
#define	DIVIDER_ALIGN		2

static long calc_uart_clock(long request, int *pllsel, int *plldiv)
{
	struct clk *clk;
	unsigned long rate = 0, clkhz[3], freqhz = 0, pllhz;
	int pll = 0, div = 0, divide, maxdiv, align, n;

	clk = clk_get(NULL, "pll0"), clkhz[0] = clk_get_rate(clk), clk_put(clk);
	clk = clk_get(NULL, "pll1"), clkhz[1] = clk_get_rate(clk), clk_put(clk);
	clk = clk_get(NULL, "pll2"), clkhz[2] = clk_get_rate(clk), clk_put(clk);

	for (n = 0; ARRAY_SIZE(clkhz) > n; n++) {
	#ifdef  CONFIG_ARM_NXP_CPUFREQ
		if (n == CONFIG_NXP_CPUFREQ_PLLDEV)
			continue;
	#endif
		pllhz = clkhz[n];
		divide = (pllhz/request);
		maxdiv = MAX_DIVIDER & ~(DIVIDER_ALIGN-1);
		align = (divide & ~(DIVIDER_ALIGN-1)) + DIVIDER_ALIGN;

		if (!divide) {
			divide = 1;
		} else {
			if (1 != divide)
				divide &= ~(DIVIDER_ALIGN-1);

			if (divide != align &&
				abs(request - pllhz/divide) >
				abs(request - pllhz/align))
				divide = align;

			divide = (divide > maxdiv ? maxdiv : divide);
		}
		freqhz = pllhz / divide;

		if (rate && (abs(freqhz-request) > abs(rate-request)))
			continue;

		rate = freqhz;
		div = divide;
		pll = n;
	}

	if (pllsel)
		*pllsel = pll;

	if (plldiv)
		*plldiv = div;

	return rate;
}

inline static void uart_init(void)
{
	U32 CLKENB = UART_CLKG_BASE;
	U32 CLKGEN = UART_CLKG_BASE + 0x04;
	struct uart_data *pdat = &clk;
	struct pl01x_regs *regs = (struct pl01x_regs *)UART_PHYS_BASE;
	unsigned int baudrate = UART_DEBUG_BAUDRATE;
	unsigned int temp;

	/*
	 * Clock Generotor & reset
	 */
	if (0 == pdat->rate) {
		unsigned int remainder;

		/*
		 * Set baud rate
	 	 *
	 	 * IBRD = UART_CLK / (16 * BAUD_RATE)
	 	 * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
 	 	 */
		pdat->rate = calc_uart_clock(UART_DEBUG_HZ, &pdat->pll, &pdat->div);
		temp = 16 * baudrate;
		pdat->divider = pdat->rate / temp;
		remainder = pdat->rate % temp;
		temp = (8 * remainder) / baudrate;
		pdat->fraction = (temp >> 1) + (temp & 1);
		pdat->lcr = UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN;
		pdat->cr = UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE |
	   				UART_PL011_CR_RTS;
	}

	/* check reset */
	if (!nxp_soc_peri_reset_status(RESET_UART_ID)) {
		NX_TIEOFF_Set(TIEOFF_USESMC, 0);
		NX_TIEOFF_Set(TIEOFF_SMCTXENB, 0);
		NX_TIEOFF_Set(TIEOFF_SMCRXENB, 0);
		nxp_soc_peri_reset_set(RESET_UART_ID);
	}

	/* check pll */
	if (!(pdat->pll & (readl(CLKGEN)>>2 & 0x7))) {
		writel(readl(CLKENB) & ~(1<<2), CLKENB);
		temp = readl(CLKGEN) & ~(0x07<<2) & ~(0xFF<<5);
		writel((temp|(pdat->pll<<2)|((pdat->div-1)<<5)), CLKGEN);
	}

	/*
	 * Uart
	 */
	writel(0, &regs->pl011_cr);	// First, disable everything
	writel(pdat->divider, &regs->pl011_ibrd);
	writel(pdat->fraction, &regs->pl011_fbrd);
	writel(pdat->lcr, &regs->pl011_lcrh);	// Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled
	writel(pdat->cr, &regs->pl011_cr);	// Finally, enable the UART

	/*
	 * alaway enable clkgen
	 */
	writel((readl(CLKENB)|(1<<2)), CLKENB);
}

inline static void uart_putc(char ch)
{
	struct pl01x_regs *regs = (struct pl01x_regs *)UART_PHYS_BASE;
	unsigned int status;

	/* Wait until there is space in the FIFO */
	while (readl(&regs->fr) & UART_PL01x_FR_TXFF)
	{ ; }

	/* Send the character */
	writel(ch, &regs->dr);

	/*
     *  Finally, wait for transmitter to become empty
 	 */
    do {
        status = readw(&regs->fr);
    } while (status & UART_PL01x_FR_BUSY);
}

inline static char uart_getc(void)
{
	struct pl01x_regs *regs = (struct pl01x_regs *)UART_PHYS_BASE;
	unsigned int data;

	/* Wait until there is data in the FIFO */
	while (readl(&regs->fr) & UART_PL01x_FR_RXFE)
	{ ; }

	data = readl(&regs->dr);

	/* Check for an error flag */
	if (data & 0xFFFFFF00) {
		/* Clear the error */
		writel(0xFFFFFFFF, &regs->ecr);
		return -1;
	}

	return (int) data;
}

inline static int uart_tstc(void)
{
	struct pl01x_regs *regs = (struct pl01x_regs *)UART_PHYS_BASE;
	return !(readl(&regs->fr) & UART_PL01x_FR_RXFE);
}

/*
 * Low level uart interface
 */
void lldebug_init(void)
{
	uart_init();
}

void lldebug_putc(const char ch)
{
   /* If \n, also do \r */
	if (ch == '\n')
    	uart_putc('\r');
	uart_putc(ch);
}

int lldebug_getc(void)
{
	return uart_getc();
}

void lldebug_puts(const char *str)
{
	while (*str)
		lldebug_putc(*str++);
}

int lldebug_tstc(void)
{
	return uart_tstc();
}

/*
 * Low level debug interface.
 */
static DEFINE_SPINLOCK(dlock);

void lldebugout(const char *fmt, ...)
{
	va_list va;
	char buff[256];
	u_long flags;

	spin_lock_irqsave(&dlock, flags);

	lldebug_init();

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	/* direct debug out */
	lldebug_puts(buff);

	spin_unlock_irqrestore(&dlock, flags);
}
EXPORT_SYMBOL_GPL(lldebugout);

/*
 * Time chec
 */
#define	CFG_TIMER_DBG_TICK_CH		2

#if (CFG_TIMER_DBG_TICK_CH == CFG_TIMER_SYS_TICK_CH)
#error	"Debug timer and System source timer must be different."
#endif

#if (CFG_TIMER_DBG_TICK_CH == CFG_TIMER_EVT_TICK_CH)
#error	"Debug timer and System event timer must be different."
#endif

#if	  (CFG_TIMER_SYS_TICK_CH == 0)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN14)
#elif (CFG_TIMER_SYS_TICK_CH == 1)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN0)
#elif (CFG_TIMER_SYS_TICK_CH == 2)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN1)
#elif (CFG_TIMER_SYS_TICK_CH == 3)
#define	TIMER_SYS_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN2)
#endif

#if	  (CFG_TIMER_DBG_TICK_CH == 0)
#define	TIMER_DBG_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN14)
#elif (CFG_TIMER_DBG_TICK_CH == 1)
#define	TIMER_DBG_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN0)
#elif (CFG_TIMER_DBG_TICK_CH == 2)
#define	TIMER_DBG_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN1)
#elif (CFG_TIMER_DBG_TICK_CH == 3)
#define	TIMER_DBG_CLKGEN		IO_ADDRESS(PHY_BASEADDR_CLKGEN2)
#endif

#define	CLKGEN_ENB		(0x0)
#define	CLKGEN_CLR		(0x4)

#define	TIMER_CFG0		(0x00)
#define	TIMER_CFG1		(0x04)
#define	TIMER_TCON		(0x08)
#define	TIMER_CNTB		(0x0C)
#define	TIMER_CMPB		(0x10)
#define	TIMER_CNTO		(0x14)
#define	TIMER_STAT		(0x44)

#define	TCON_AUTO		(1<<3)
#define	TCON_INVT		(1<<2)
#define	TCON_UP			(1<<1)
#define	TCON_RUN		(1<<0)
#define CFG0_CH(ch)		(ch == 0 || ch == 1 ? 0 : 8)
#define CFG1_CH(ch)		(ch * 4)
#define TCON_CH(ch)		(ch ? ch * 4  + 4 : 0)
#define TINT_CH(ch)		(ch)
#define TINT_CS_CH(ch)	(ch + 5)
#define	TINT_CS_MASK	(0x1F)
#define TIMER_CH_OFFS	(0xC)

#define	TIMER_BASE		IO_ADDRESS(PHY_BASEADDR_TIMER)
#define	TIMER_READ(ch)	(readl(TIMER_BASE + TIMER_CNTO + (TIMER_CH_OFFS * ch)))

#define	TIMER_CLOCK_HZ	(10*1000000)				/* 10MHZ */
#define	TIMER_SYS_HZ	(TIMER_CLOCK_HZ/(1000000))

static bool __dbg_timer_run = false;

/* defined timer.c */
extern int __timer_sys_mux_val;
extern int __timer_sys_scl_val;
extern int __timer_sys_clk_clr;

void lltime_start(void)
{
	int ch = CFG_TIMER_DBG_TICK_CH;
	ulong count = (-1UL), val;
	int mux = __timer_sys_mux_val;
	int scl = __timer_sys_scl_val;

	if (true == __dbg_timer_run)
		return;

	if (!nxp_soc_peri_reset_status(RESET_ID_TIMER))
		nxp_soc_peri_reset_set(RESET_ID_TIMER);

	/* clock gen : enable */
	if (5 == mux) {
		uint clr = __timer_sys_clk_clr;
		uint enb = readl(TIMER_SYS_CLKGEN + CLKGEN_ENB);
		writel(clr , TIMER_DBG_CLKGEN + CLKGEN_CLR);
		writel((enb | 1<<2), TIMER_DBG_CLKGEN + CLKGEN_ENB);
	}

	/* Timer : stop */
	val  = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(TCON_RUN << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	/* Timer : prescaler (clock) */
	val = readl(TIMER_BASE + TIMER_CFG0);
	val &= ~(0xFF   << CFG0_CH(ch));
	val |=  ((scl-1)<< CFG0_CH(ch));
	writel(val, (TIMER_BASE + TIMER_CFG0));

	/* Timer : mux (clock) */
	val = readl(TIMER_BASE + TIMER_CFG1);
	val &= ~(0xF << CFG1_CH(ch));
	val |=  (mux << CFG1_CH(ch));
	writel(val, (TIMER_BASE + TIMER_CFG1));

	/* Timer : count */
	writel(count, TIMER_BASE + TIMER_CNTB + (TIMER_CH_OFFS * ch));
	writel(count, TIMER_BASE + TIMER_CMPB + (TIMER_CH_OFFS * ch));

	/* Timer : start */
	val  = readl(TIMER_BASE + TIMER_STAT);
	val &= ~(TINT_CS_MASK<<5 | 0x1 << TINT_CH(ch));
	val |=  (0x1 << TINT_CS_CH(ch) | 0 << TINT_CH(ch));
	writel(val, TIMER_BASE + TIMER_STAT);

	val = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(0xE << TCON_CH(ch));
	val |=  (TCON_UP << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	val &= ~(TCON_UP << TCON_CH(ch));
	val |=  ((TCON_AUTO | TCON_RUN)  << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	__dbg_timer_run = true;
}

void lltime_stop(void)
{
	int ch = CFG_TIMER_DBG_TICK_CH;
	int mux = __timer_sys_mux_val;
	unsigned int val;

	if (false == __dbg_timer_run)
		return;

	/* Timer : stop */
	val  = readl(TIMER_BASE + TIMER_STAT);
	val &= ~(TINT_CS_MASK<<5 | 0x1 << TINT_CH(ch));
	val |=  (0x1 << TINT_CS_CH(ch) | 0 << TINT_CH(ch));
	writel(val, TIMER_BASE + TIMER_STAT);

	val  = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(TCON_RUN << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	/* clock gen : disable */
	if (5 == mux)
		writel(readl(TIMER_DBG_CLKGEN + CLKGEN_ENB) & ~(1<<2), TIMER_DBG_CLKGEN + CLKGEN_ENB);

	__dbg_timer_run = false;
}

void lltime_reset(void)
{
	int ch = CFG_TIMER_DBG_TICK_CH;
	ulong count = (-1UL), val;

	if (false == __dbg_timer_run) {
		lldebugout("low level timer.%d not run to reset ...\n", ch);
		return;
	}

	/* Timer : stop */
	val  = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(TCON_RUN << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	/* Timer : count */
	writel(count, TIMER_BASE + TIMER_CNTB + (TIMER_CH_OFFS * ch));
	writel(count, TIMER_BASE + TIMER_CMPB + (TIMER_CH_OFFS * ch));

	/* Timer : start */
	val = readl(TIMER_BASE + TIMER_TCON);
	val &= ~(0xE << TCON_CH(ch));
	val |=  (TCON_UP << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);

	val &= ~(TCON_UP << TCON_CH(ch));
	val |=  ((TCON_AUTO | TCON_RUN)  << TCON_CH(ch));
	writel(val, TIMER_BASE + TIMER_TCON);
}

ulong lltime_get(void)
{
	int ch = CFG_TIMER_DBG_TICK_CH;
	ulong count = (-1UL);
	ulong tcnt = TIMER_READ(ch);

	if (false == __dbg_timer_run)
		return 0;

	return (count - tcnt)/TIMER_SYS_HZ;	/* unit us */
}
