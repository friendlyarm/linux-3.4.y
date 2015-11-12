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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/seq_file.h>
#include <asm/clkdev.h>
#include <mach/platform.h>
#include <mach/devices.h>

/*
#define	pr_debug	printk
*/

/*
 * clock generator macros
 */
#define	_InPLL0_		(0)
#define	_InPLL1_		(1)
#define	_InPLL2_		(2)
#define	_InPLL3_		(3)
#define	_InEXT1_		(4)
#define	_InEXT2_		(5)
#define	_InCLKn_		(7)

#define	INPUT_CLKS		6		/* PLL0, PLL1, PLL2, PLL3, EXT1, EXT2 */

#if defined(CONFIG_NXP_DFS_BCLK)
	#if defined(CONFIG_S5P4418_DFS_BCLK_PLL_0)
	#define CONFIG_S5P4418_BCLKFREQ_PLLDEV 	0
	#elif defined(CONFIG_S5P4418_DFS_BCLK_PLL_1)
	#define CONFIG_S5P4418_BCLKFREQ_PLLDEV 	1
	#else
	#define CONFIG_S5P4418_BCLKFREQ_PLLDEV	0
	#endif
#define	DVFS_BCLK_PLL	~(1<<CONFIG_S5P4418_BCLKFREQ_PLLDEV)
#else
#define	DVFS_BCLK_PLL	(-1UL)
#endif

#ifdef  CONFIG_ARM_NXP_CPUFREQ
#define	DVFS_CPU_PLL	~(1<<CONFIG_NXP_CPUFREQ_PLLDEV)
#else
#define	DVFS_CPU_PLL	(-1UL)
#endif

#if defined(CONFIG_SND_NXP_DFS)
#define	DFS_SND_PLL	~(1<<CONFIG_SND_NXP_PLLDEV) // default PLL2
#else
#define	DFS_SND_PLL	(-1UL)
#endif

#define IGNORE_PLLs		(DVFS_CPU_PLL & DVFS_BCLK_PLL & DFS_SND_PLL)
#define	INPUT_MASK 		(((1<<INPUT_CLKS) - 1) & IGNORE_PLLs)

#define	_PLL0_ 			(1 << _InPLL0_)
#define	_PLL1_ 			(1 << _InPLL1_)
#define	_PLL2_ 			(1 << _InPLL2_)
#define	_PLL3_ 			(1 << _InPLL3_)
#define	_EXTCLK1_ 		(1 << _InEXT1_)
#define	_EXTCLK2_ 		(1 << _InEXT2_)

#define	_PLL_0_1_		(_PLL0_    | _PLL1_)
#define	_PLL_0_2_		(_PLL_0_1_ | _PLL2_)
#define	_PLL_0_3_		(_PLL_0_2_ | _PLL3_)
#define	_CLKOUTn_		(0)

#define	_PCLK_			(1<<16)
#define	_BCLK_			(1<<17)
#define	_GATE_PCLK_		(1<<20)
#define	_GATE_BCLK_		(1<<21)
#define	PLCK_MASK		(_GATE_PCLK_ | _PCLK_)
#define	BLCK_MASK		(_GATE_BCLK_ | _BCLK_)

struct nxp_clk_periph {
	const char	*dev_name;
	int	 dev_id;
	char periph_id;
	void *base_addr;
	bool enable;
	/* clock config */
	char level;
	unsigned int clk_mask0;
	unsigned int clk_mask1;
	short clk_src0;
	short clk_div0;
	char  clk_inv0;
	short clk_src1;
	short clk_div1;
	char  clk_inv1;
	long  clk_ext1;
	long  clk_ext2;
	spinlock_t lock;
};

struct nxp_clk_dev {
	struct clk  clk;
	struct clk *link;
	const char *name;
	struct nxp_clk_periph *peri;
};

struct clk_link_dev {
	char *name;
	int   id;
};

struct clk_gen_reg {
	volatile U32 CLKENB;
	volatile U32 CLKGEN[4];
};

#define CLK_PERI_1S(name, devid, id, base, mk) [id] = {	\
	.dev_name = name, .dev_id = devid,.periph_id = id, .level = 1, 	\
	.base_addr = (void*)base, .clk_mask0 = mk, .enable = false, }

#define CLK_PERI_2S(name, devid, id, base, mk, mk2) [id] = {	\
	.dev_name = name, .dev_id = devid, .periph_id = id, .level = 2, 	\
	.base_addr = (void*)base, .clk_mask0 = mk, .clk_mask1 = mk2, .enable = false, }

#define	LIST_INIT(x) 	LIST_HEAD_INIT(clk_core[x].list)

static struct clk_core {
	int id;
	const char *name;
	struct list_head list;
} clk_core[] = {
	[ 0] = { .id =  0, .name = CORECLK_NAME_PLL0, .list = LIST_INIT( 0), },
	[ 1] = { .id =  1, .name = CORECLK_NAME_PLL1, .list = LIST_INIT( 1), },
	[ 2] = { .id =  2, .name = CORECLK_NAME_PLL2, .list = LIST_INIT( 2), },
	[ 3] = { .id =  3, .name = CORECLK_NAME_PLL3, .list = LIST_INIT( 3), },
	[ 4] = { .id =  4, .name = CORECLK_NAME_FCLK, .list = LIST_INIT( 4), },
	[ 5] = { .id =  5, .name = CORECLK_NAME_MCLK, .list = LIST_INIT( 5), },
	[ 6] = { .id =  6, .name = CORECLK_NAME_BCLK, .list = LIST_INIT( 6), },
	[ 7] = { .id =  7, .name = CORECLK_NAME_PCLK, .list = LIST_INIT( 7), },
	[ 8] = { .id =  8, .name = CORECLK_NAME_HCLK, .list = LIST_INIT( 8), },
	[ 9] = { .id =  9, .name = "memdclk"	, .list = LIST_INIT( 9), },
	[10] = { .id = 10, .name = "membclk"	, .list = LIST_INIT(10), },
	[11] = { .id = 11, .name = "mempclk"	, .list = LIST_INIT(11), },
	[12] = { .id = 12, .name = "g3dbclk"	, .list = LIST_INIT(12), },
	[13] = { .id = 13, .name = "mpegbclk"	, .list = LIST_INIT(13), },
	[14] = { .id = 14, .name = "mpegpclk"	, .list = LIST_INIT(14), },
};

#define	CORE_PLL_NUM		4

static struct nxp_clk_periph clk_periphs [] = {
	CLK_PERI_1S(DEV_NAME_TIMER		,  0, CLK_ID_TIMER_0	, PHY_BASEADDR_CLKGEN14, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_TIMER		,  1, CLK_ID_TIMER_1	, PHY_BASEADDR_CLKGEN0 , (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_TIMER		,  2, CLK_ID_TIMER_2	, PHY_BASEADDR_CLKGEN1 , (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_TIMER		,  3, CLK_ID_TIMER_3	, PHY_BASEADDR_CLKGEN2 , (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_UART		,  0, CLK_ID_UART_0	    , PHY_BASEADDR_CLKGEN22, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_UART		,  1, CLK_ID_UART_1	    , PHY_BASEADDR_CLKGEN24, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_UART		,  2, CLK_ID_UART_2	    , PHY_BASEADDR_CLKGEN23, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_UART		,  3, CLK_ID_UART_3	    , PHY_BASEADDR_CLKGEN25, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_UART		,  4, CLK_ID_UART_4	    , PHY_BASEADDR_CLKGEN26, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_UART		,  5, CLK_ID_UART_5	    , PHY_BASEADDR_CLKGEN27, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_PWM		,  0, CLK_ID_PWM_0	    , PHY_BASEADDR_CLKGEN13, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_PWM		,  1, CLK_ID_PWM_1	    , PHY_BASEADDR_CLKGEN3 , (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_PWM		,  2, CLK_ID_PWM_2	    , PHY_BASEADDR_CLKGEN4 , (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_PWM		,  3, CLK_ID_PWM_3	    , PHY_BASEADDR_CLKGEN5 , (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_I2C		,  0, CLK_ID_I2C_0	    , PHY_BASEADDR_CLKGEN6 , (_GATE_PCLK_)),
	CLK_PERI_1S(DEV_NAME_I2C		,  1, CLK_ID_I2C_1	    , PHY_BASEADDR_CLKGEN7 , (_GATE_PCLK_)),
	CLK_PERI_1S(DEV_NAME_I2C		,  2, CLK_ID_I2C_2	    , PHY_BASEADDR_CLKGEN8 , (_GATE_PCLK_)),
	CLK_PERI_2S(DEV_NAME_I2S		,  0, CLK_ID_I2S_0	    , PHY_BASEADDR_CLKGEN15, (_PLL_0_3_|_EXTCLK1_), (_CLKOUTn_)),
	CLK_PERI_2S(DEV_NAME_I2S		,  1, CLK_ID_I2S_1	    , PHY_BASEADDR_CLKGEN16, (_PLL_0_3_|_EXTCLK1_), (_CLKOUTn_)),
	CLK_PERI_2S(DEV_NAME_I2S		,  2, CLK_ID_I2S_2	    , PHY_BASEADDR_CLKGEN17, (_PLL_0_3_|_EXTCLK1_), (_CLKOUTn_)),
	CLK_PERI_1S(DEV_NAME_SDHC		,  0, CLK_ID_SDHC_0	    , PHY_BASEADDR_CLKGEN18, (_PLL_0_2_|_GATE_PCLK_)),
	CLK_PERI_1S(DEV_NAME_SDHC		,  1, CLK_ID_SDHC_1	    , PHY_BASEADDR_CLKGEN19, (_PLL_0_2_|_GATE_PCLK_)),
	CLK_PERI_1S(DEV_NAME_SDHC		,  2, CLK_ID_SDHC_2	    , PHY_BASEADDR_CLKGEN20, (_PLL_0_2_|_GATE_PCLK_)),
	CLK_PERI_1S(DEV_NAME_SPI		,  0, CLK_ID_SPI_0	    , PHY_BASEADDR_CLKGEN37, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_SPI		,  1, CLK_ID_SPI_1	    , PHY_BASEADDR_CLKGEN38, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_SPI		,  2, CLK_ID_SPI_2		, PHY_BASEADDR_CLKGEN39, (_PLL_0_2_)),
	#if 0
	CLK_PERI_1S(DEV_NAME_VIP		,  0, CLK_ID_VIP_0	    , PHY_BASEADDR_CLKGEN30, (_PLL_0_3_|_EXTCLK1_|_GATE_BCLK_)),
	CLK_PERI_1S(DEV_NAME_VIP		,  1, CLK_ID_VIP_1	    , PHY_BASEADDR_CLKGEN31, (_PLL_0_3_|_EXTCLK1_|_EXTCLK2_|_GATE_BCLK_)),
	#endif
	CLK_PERI_1S(DEV_NAME_MIPI		, -1, CLK_ID_MIPI		, PHY_BASEADDR_CLKGEN9 , (_PLL_0_2_)),
	CLK_PERI_2S(DEV_NAME_GMAC		, -1, CLK_ID_GMAC		, PHY_BASEADDR_CLKGEN10, (_PLL_0_3_|_EXTCLK1_), (_CLKOUTn_)),
	CLK_PERI_1S(DEV_NAME_SPDIF_TX	, -1, CLK_ID_SPDIF_TX	, PHY_BASEADDR_CLKGEN11, (_PLL_0_2_)),
	CLK_PERI_1S(DEV_NAME_MPEGTSI	, -1, CLK_ID_MPEGTSI	, PHY_BASEADDR_CLKGEN12, (_GATE_BCLK_)),
	#if 0
	CLK_PERI_1S(DEV_NAME_MALI		, -1, CLK_ID_MALI		, PHY_BASEADDR_CLKGEN21, (_GATE_BCLK_)),
	#endif
	CLK_PERI_1S(DEV_NAME_DIT		, -1, CLK_ID_DIT		, PHY_BASEADDR_CLKGEN28, (_GATE_BCLK_)),
	CLK_PERI_1S(DEV_NAME_PPM		, -1, CLK_ID_PPM		, PHY_BASEADDR_CLKGEN29, (_PLL_0_2_)),
	#if 0
	CLK_PERI_2S(DEV_NAME_USB2HOST	, -1, CLK_ID_USB2HOST	, PHY_BASEADDR_CLKGEN32, (_PLL_0_3_), (_PLL_0_3_|_EXTCLK1_)),
	CLK_PERI_1S(DEV_NAME_CODA		, -1, CLK_ID_CODA		, PHY_BASEADDR_CLKGEN33, (_GATE_PCLK_|_GATE_BCLK_)),
	#endif
	CLK_PERI_1S(DEV_NAME_CRYPTO		, -1, CLK_ID_CRYPTO	    , PHY_BASEADDR_CLKGEN34, (_GATE_PCLK_)),
	CLK_PERI_1S(DEV_NAME_SCALER		, -1, CLK_ID_SCALER	    , PHY_BASEADDR_CLKGEN35, (_GATE_BCLK_)),
	CLK_PERI_1S(DEV_NAME_PDM		, -1, CLK_ID_PDM		, PHY_BASEADDR_CLKGEN36, (_GATE_PCLK_)),
	#if 0
	CLK_PERI_2S(DEV_NAME_USBOTG		, -1, CLK_ID_USBOTG		, PHY_BASEADDR_CLKGEN32, (_PLL_0_3_), (_PLL_0_3_|_EXTCLK1_)),
	#endif
};

static struct clk_link_dev clk_link[] = {
	{ .name = "uart-pl011.0", .id = CLK_ID_UART_0, },
	{ .name = "uart-pl011.1", .id = CLK_ID_UART_1, },
	{ .name = "uart-pl011.2", .id = CLK_ID_UART_2, },
	{ .name = "uart-pl011.3", .id = CLK_ID_UART_3, },
	{ .name = "uart-pl011.4", .id = CLK_ID_UART_4, },
	{ .name = "uart-pl011.5", .id = CLK_ID_UART_5, },
	{ .name = "ssp-pl022.0" , .id = CLK_ID_SPI_0 , },
	{ .name = "ssp-pl022.1" , .id = CLK_ID_SPI_1 , },
	{ .name = "ssp-pl022.2" , .id = CLK_ID_SPI_2 , },

};

#define	CLK_PERI_NUM		((int)ARRAY_SIZE(clk_periphs))
#define	CLK_CORE_NUM		((int)ARRAY_SIZE(clk_core))
#define	CLK_LINK_NUM		((int)ARRAY_SIZE(clk_link))
#define	CLK_DEVS_NUM		(CLK_CORE_NUM + CLK_PERI_NUM + CLK_LINK_NUM)
#define	MAX_DIVIDER			((1<<8) - 1)	// 256, align 2

static struct nxp_clk_dev	(clk_devices[CLK_DEVS_NUM]);
#define	clk_dev_get(n)		((struct nxp_clk_dev *)&clk_devices[n])
#define	clk_container(p)	(container_of(p, struct nxp_clk_dev, clk))

/* dynamic frequency pll num */
static unsigned int core_hz[15];	/* core clock */
#define	CORE_HZ_SIZE	ARRAY_SIZE(core_hz)

static unsigned int support_dvfs = 1;

/*
 * CLKGEN HW
 */
static inline void clk_gen_bclk(void *base, int on)
{
	struct clk_gen_reg *reg = base;
	unsigned int val = 0;

	val	 = readl(&reg->CLKENB) & ~(0x3);
	val |= (on ? 3 : 0) & 0x3;	/* always BCLK */

	writel(val, &reg->CLKENB);
}

static inline void clk_gen_pclk(void *base, int on)
{
	struct clk_gen_reg *reg = base;
	unsigned int val = 0;

	if (!on)
		return;

	val	 = readl(&reg->CLKENB) & ~(1 << 3);
	val |= (1 << 3);

	writel(val, &reg->CLKENB);
}

static inline void clk_gen_rate(void *base, int level, int src, int div)
{
	struct clk_gen_reg *reg = base;
	unsigned int val = 0;

	#ifdef CONFIG_NXP_CPUFREQ_PLLDEV
	if (CONFIG_NXP_CPUFREQ_PLLDEV == src)
		printk("*** %s: Fail pll.%d for CPU  DFS ***\n", __func__, src);
	#endif
	#ifdef CONFIG_S5P4418_BCLKFREQ_PLLDEV
	if (CONFIG_S5P4418_BCLKFREQ_PLLDEV == src)
		printk("*** %s: Fail pll.%d for BCLK DFS ***\n", __func__, src);
	#endif

	val  = readl(&reg->CLKGEN[level<<1]);

	val &= ~(0x07   << 2);
	val |=  (src    << 2);	/* source */
	val	&= ~(0xFF   << 5);
	val	|=  (div-1) << 5;	/* divider */

	writel(val, &reg->CLKGEN[level<<1]);
}

static inline void clk_gen_inv(void *base, int level, int inv)
{
	struct clk_gen_reg *reg = base;
	unsigned int val = 0;

	val  = readl(&reg->CLKGEN[level<<1]) & ~(1 << 1);
	val	|= (inv<< 1);

	writel(val, &reg->CLKGEN[level<<1]);
}

static inline void clk_gen_enb(void *base, int on)
{
	struct clk_gen_reg *reg = base;
	unsigned int val = 0;

	val	 = readl(&reg->CLKENB) & ~(1 << 2);
	val	|= ((on ? 1 : 0) << 2);

	writel(val, &reg->CLKENB);
}

/*
 * Core clocks
 */
static struct NX_CLKPWR_RegisterSet * const clkpwr =
	(struct NX_CLKPWR_RegisterSet *)IO_ADDRESS(PHY_BASEADDR_CLKPWR_MODULE);

static unsigned int pll_get_rate(unsigned int pllN, unsigned int xtal)
{
    unsigned int val, val1, nP, nM, nS, nK;
    val   = clkpwr->PLLSETREG[pllN];
    val1  = clkpwr->PLLSETREG_SSCG[pllN];
	xtal /= 1000;	/* Unit Khz */

    nP= (val >> 18) & 0x03F;
    nM= (val >>  8) & 0x3FF;
    nS= (val >>  0) & 0x0FF;
    nK= (val1>> 16) & 0xFFFF;

    if(pllN < 2)
        return (unsigned int)(((nM * xtal)/nP)>>nS)*1000;
    else
        return (unsigned int)((((nM * xtal)/nP)>>nS)+((((nK * xtal)/nP)>>nS)>>16))*1000;
}

/* dvo : 0=CPU, 1=BUS, 2=MEM, 3=3D, 4=MPEG */
static unsigned int pll_get_dvo(unsigned int dvo)
{
    return (clkpwr->DVOREG[dvo] & 0x7);
}

static unsigned int pll_get_div(unsigned int dvo)
{
    unsigned int val = clkpwr->DVOREG[dvo];
    unsigned int div = ((((val>>21)&0x3F)+1)<<24) |
    					((((val>>15)&0x3F)+1)<<16) |
    					((((val>> 9)&0x3F)+1)<< 8) |
    					((((val>> 3)&0x3F)+1)<< 0);
    return div;
}

#define	PLLN_RATE(n)		(pll_get_rate(n, CFG_SYS_PLLFIN))	/* 0~ 3 */
#define	FCLK_RATE(n)		(pll_get_rate(pll_get_dvo (0), CFG_SYS_PLLFIN) / ((pll_get_div(0)>> 0)&0x3F))
#define	MCLK_RATE(n)		(pll_get_rate(pll_get_dvo (2), CFG_SYS_PLLFIN) / 	\
							((pll_get_div(2)>> 0)&0x3F) / ((pll_get_div(2)>> 8)&0x3F))
#define	BCLK_RATE(n)		(pll_get_rate(pll_get_dvo (1), CFG_SYS_PLLFIN) /		\
					  		((pll_get_div(1)>> 0)&0x3F))
#define	PCLK_RATE(n)		(pll_get_rate(pll_get_dvo (1), CFG_SYS_PLLFIN) /		\
			  				((pll_get_div(1)>> 0)&0x3F) / ((pll_get_div(1)>> 8)&0x3F))
#define	HCLK_RATE(n)		(pll_get_rate(pll_get_dvo (0), CFG_SYS_PLLFIN) /		\
				  			((pll_get_div(0)>> 0)&0x3F) / ((pll_get_div(0)>> 8)&0x3F))
#define	MDCLK_RATE(n)		(pll_get_rate(pll_get_dvo (2), CFG_SYS_PLLFIN) /		\
				  			((pll_get_div(2)>> 0)&0x3F) / ((pll_get_div(2)>> 8)&0x3F))
#define	MBCLK_RATE(n)		(pll_get_rate(pll_get_dvo (2), CFG_SYS_PLLFIN) /		\
			  				((pll_get_div(2)>> 0)&0x3F) /						\
			  				((pll_get_div(2)>> 8)&0x3F) /						\
			  				((pll_get_div(2)>>16)&0x3F))
#define	MPCLK_RATE(n)		(pll_get_rate(pll_get_dvo (2), CFG_SYS_PLLFIN) /		\
			  				((pll_get_div(2)>> 0)&0x3F) /						\
			  				((pll_get_div(2)>> 8)&0x3F) /						\
			  				((pll_get_div(2)>>16)&0x3F) /						\
			  				((pll_get_div(2)>>24)&0x3F))
#define	G3D_BCLK_RATE(n)	(pll_get_rate(pll_get_dvo (3), CFG_SYS_PLLFIN) /		\
			  				((pll_get_div(3)>> 0)&0x3F))
#define	MPG_BCLK_RATE(n)	(pll_get_rate(pll_get_dvo (4), CFG_SYS_PLLFIN) /		\
			  				((pll_get_div(4)>> 0)&0x3F))
#define	MPG_PCLK_RATE(n)	(pll_get_rate(pll_get_dvo (4), CFG_SYS_PLLFIN) /		\
			  				((pll_get_div(4)>> 0)&0x3F)	/						\
			  				((pll_get_div(4)>> 8)&0x3F))

static unsigned long core_update_rate(int type)
{
	unsigned long rate = 0;
	switch (type) {
	case  0: rate = core_hz[ 0] = PLLN_RATE ( 0);    break;   	// PLL 0
	case  1: rate = core_hz[ 1] = PLLN_RATE ( 1);    break;   	// PLL 1
	case  2: rate = core_hz[ 2] = PLLN_RATE ( 2);    break;   	// PLL 2
	case  3: rate = core_hz[ 3] = PLLN_RATE ( 3);    break;   	// PLL 3
	case  4: rate = core_hz[ 4] = FCLK_RATE ( 4);    break;   	// FCLK
	case  5: rate = core_hz[ 5] = MCLK_RATE ( 5);    break;   	// MCLK
	case  6: rate = core_hz[ 6] = BCLK_RATE ( 6);    break;   	// BCLK
	case  7: rate = core_hz[ 7] = PCLK_RATE ( 7);    break;   	// PCLK
	case  8: rate = core_hz[ 8] = HCLK_RATE ( 8);    break;   	// HCLK
	case  9: rate = core_hz[ 9] = MDCLK_RATE( 9);    break;   	// MDCLK
	case 10: rate = core_hz[10] = MBCLK_RATE(10);    break;   	// MBCLK
	case 11: rate = core_hz[11] = MPCLK_RATE(11);    break;   	// MPCLK
	case 12: rate = core_hz[12] = G3D_BCLK_RATE(12); break;	// G3D BCLK
	case 13: rate = core_hz[13] = MPG_BCLK_RATE(13); break;	// MPG BCLK
	case 14: rate = core_hz[14] = MPG_PCLK_RATE(14); break;	// MPG PCLK
	};
	return rate;
}

static long core_get_rate(int type)
{
	unsigned int rate = 0;

	switch (type) {
	case  0: rate = core_hz[ 0];	break;	// PLL0
	case  1: rate = core_hz[ 1];	break;	// PLL1
	case  2: rate = core_hz[ 2];	break;	// PLL2, 295 MHZ
	case  3: rate = core_hz[ 3];	break;	// PLL3
	case  4: rate = core_hz[ 4];	break;	// FCLK
	case  5: rate = core_hz[ 5];	break;	// MCLK
	case  6: rate = core_hz[ 6];	break;	// BCLK
	case  7: rate = core_hz[ 7];	break;	// PCLK, 166500000, 100000000
	case  8: rate = core_hz[ 8];	break;	// HCLK
	case  9: rate = core_hz[ 9];	break;	// MDCLK
	case 10: rate = core_hz[10];	break;	// MBCLK
	case 11: rate = core_hz[11];	break;	// MPCLK
	case 12: rate = core_hz[12]; 	break;	// G3D BCLK
	case 13: rate = core_hz[13]; 	break;	// MPG BCLK
	case 14: rate = core_hz[14];	break;	// MPG PCLK
	default: printk("unknown core clock type %d ...\n", type);
			break;
	};
	return rate;
}

static void core_pll_update_link(int pll)
{
	struct nxp_clk_dev *cdev;
	struct clk_core *core;
	struct clk *clk;
	struct list_head *list, *head;

	if (pll > (CORE_PLL_NUM-1)) {
		printk("Not support pll number %d (0~%d) ...\n", pll, (CORE_PLL_NUM-1));
		return;
	}

	/* PLL rate */
	cdev = clk_dev_get(pll);
	clk  = &cdev->clk;
	clk->rate = core_hz[pll] = PLLN_RATE(pll);

	/* PLL connected rate */
	head = &clk_core[pll].list;
	list_for_each(list, head) {
		core = container_of(list, struct clk_core, list);
		cdev = clk_dev_get(core->id);
		clk  = &cdev->clk;

		if (core->id)
			clk->rate = core_update_rate(core->id);
		pr_debug("clk : pll.%d - %s update %u khz\n", pll, cdev->name, clk->rate/1000);
	}
}

static inline long core_set_rate(struct clk *clk, long rate)
{
#if defined(CONFIG_ARM_NXP_CPUFREQ)
	struct nxp_clk_dev *cdev = clk_container(clk);
	char * c = (char *)cdev->name;
	int pll = -1;

	if (*c++ == 'p')
	if (*c++ == 'l')
	if (*c++ == 'l')
		pll = simple_strtol(c, NULL, 10);

	pr_debug("%s change pll.%d (dvfs pll.%d) %ld \n",
		__func__, pll, CONFIG_NXP_CPUFREQ_PLLDEV, rate);

#ifdef CONFIG_NXP_DFS_BCLK
	WARN(0 != raw_smp_processor_id(), "Dynamic Frequency CPU.%d  conflict with BCLK DFS...\n",
		raw_smp_processor_id());
#endif

	if (pll != -1 &&
		pll == CONFIG_NXP_CPUFREQ_PLLDEV) {
		if (!support_dvfs) {
			printk("Can't DVFS rate %10ld with PLL %d....\n", rate, pll);
			return clk->rate;
		}
		nxp_cpu_pll_change_frequency(pll, rate);
		core_pll_update_link(pll);				/* PLL */
	}
#endif
	return clk->rate;
}

static void core_rate_init(void)
{
	int pll, i;

	for (i = 0; CORE_HZ_SIZE > i; i++)
		core_update_rate(i);

	/* CPU : FCLK, HCLK */
	pll = pll_get_dvo(0);
	list_add_tail(&clk_core[4].list, &clk_core[pll].list);
	list_add_tail(&clk_core[8].list, &clk_core[pll].list);

	/* BUS : BCLK, PCLK  */
	pll = pll_get_dvo(1);
	list_add_tail(&clk_core[6].list, &clk_core[pll].list);
	list_add_tail(&clk_core[7].list, &clk_core[pll].list);

	/* MEM : MCLK, DCLK, BCLK, PCLK */
	pll = pll_get_dvo(2);
	list_add_tail(&clk_core[ 5].list, &clk_core[pll].list);
	list_add_tail(&clk_core[ 9].list, &clk_core[pll].list);
	list_add_tail(&clk_core[10].list, &clk_core[pll].list);
	list_add_tail(&clk_core[11].list, &clk_core[pll].list);

	/* G3D : BCLK */
	pll = pll_get_dvo(3);
	list_add_tail(&clk_core[12].list, &clk_core[pll].list);

	/* MPEG : BCLK, PCLK */
	pll = pll_get_dvo(4);
	list_add_tail(&clk_core[13].list, &clk_core[pll].list);
	list_add_tail(&clk_core[14].list, &clk_core[pll].list);
}

/*
 *  Unit Khz
 *  "sys/devices/platform/pll.N"
 *
 */
static ssize_t core_pll_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	struct attribute *at = &attr->attr;
	const char *c;
	char *s = buf;
	int pll;

	c = &at->name[strlen("pll.")];
	pll = simple_strtoul(c, NULL, 10);

	if (pll > 3 || 0 > pll) {
		printk("Fail, not support pll.%d (0~3)\n", pll);
		return 0;
	}

	s += sprintf(s, "%u\n", core_hz[pll]/1000);	/* khz */

	return (s - buf);
}

static ssize_t core_pll_store(struct device *pdev,
			struct device_attribute *attr, const char *buf, size_t n)
{
	struct attribute *at = &attr->attr;
	const char *c;
	unsigned long freq;
	int pll;

	c = &at->name[strlen("pll.")];
	pll = simple_strtoul(c, NULL, 10);

	if (pll > 3 || 0 > pll) {
		printk("Fail, not support pll.%d (0~3)\n", pll);
		return 0;
	}

	sscanf(buf,"%lu", &freq);
	freq *=1000; /* khz */

#if defined (CONFIG_ARM_NXP_CPUFREQ)
	nxp_cpu_pll_change_frequency(pll, freq);
#endif

	core_pll_update_link(pll);

	return n;
}

static struct device_attribute core_pll_attr[] = {
	__ATTR(pll.0, 0664, core_pll_show, core_pll_store),
	__ATTR(pll.1, 0664, core_pll_show, core_pll_store),
	__ATTR(pll.2, 0664, core_pll_show, core_pll_store),
	__ATTR(pll.3, 0664, core_pll_show, core_pll_store),
};

static struct attribute *attrs[] = {
	&core_pll_attr[0].attr,
	&core_pll_attr[1].attr,
	&core_pll_attr[2].attr,
	&core_pll_attr[3].attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = (struct attribute **)attrs,
};

static int __init cpu_pll_sys_init(void)
{
	struct kobject *kobj = NULL;
	int ret = 0;

	/* create attribute interface */
	kobj = kobject_create_and_add("pll", &platform_bus.kobj);
	if (!kobj) {
		printk(KERN_ERR "Fail, create kobject for cpu\n");
		return -ret;
	}

	ret = sysfs_create_group(kobj, &attr_group);
	if (ret) {
		printk(KERN_ERR "Fail, create sysfs group for pll ...\n");
		kobject_del(kobj);
		return -ret;
	}
	return ret;
}
module_init(cpu_pll_sys_init);

/*
 * Clock Interfaces
 */
static inline long clk_divide(long rate, long request,
				int align, int *divide, int level)
{
	int div = (rate/request);
	int max = MAX_DIVIDER & ~(align-1);
	int adv = (div & ~(align-1)) + align;

	if (!div) {
		if (divide)
			*divide = 1;
		return rate;
	}

	if (1 != div)
		div &= ~(align-1);

	if (div != adv &&
		abs(request - rate/div) > abs(request - rate/adv))
		div = adv;

	if (level == 2)
		div = (div > max ? div/2 : div);
	else		
		div = (div > max ? max : div);
	if (divide)
		*divide = div;

	return (rate/div);
}

struct clk *clk_get_sys(const char *dev_id, const char *con_id)
{
	/* SMP private timer */
	if (! strcmp(dev_id, "smp_twd"))
		return clk_get(NULL, CORECLK_NAME_HCLK);

	return clk_get(NULL, (char *)dev_id);
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	struct nxp_clk_dev *pll = NULL, *cdev = clk_container(clk);
	struct nxp_clk_periph *peri = cdev->peri;
	unsigned long request = rate, rate_hz = 0, flags;
	unsigned long clock_hz, freq_hz = 0;
	unsigned int mask, input_mask = INPUT_MASK;
	int level, div[2] = { 0, };
	int i, n, clk2 = 0;
	short s1 = 0, s2 = 0, d1 = 0, d2 = 0;

	if (NULL == peri)
		return core_set_rate(clk, rate);

	level = peri->level;
	mask  = peri->clk_mask0;
	pr_debug("clk: %s.%d reqeust = %ld [input=0x%x]\n",
			peri->dev_name, peri->dev_id, rate, mask);

#if defined(CONFIG_SND_NXP_DFS)
    if (!strcmp(peri->dev_name, DEV_NAME_I2S))
		input_mask |= (1<<CONFIG_SND_NXP_PLLDEV);
#endif

	if (!(input_mask & mask)) {
		if (PLCK_MASK & mask)
			return core_get_rate(CORECLK_ID_PCLK);
		else if (BLCK_MASK & mask)
			return core_get_rate(CORECLK_ID_BCLK);
		else
			return clk->rate;
	}

next:
	for (n = 0; INPUT_CLKS > n; n++) {
		if (!(((mask & input_mask) >> n) & 0x1))
			continue;

		if (_InEXT1_ == n) {
			rate = peri->clk_ext1;
		} else if (_InEXT2_ == n) {
			rate = peri->clk_ext2;
		} else {
			pll  = clk_dev_get(n);
			rate = pll->clk.rate;
		}

		if (! rate)
			continue;

		clock_hz = rate;
		for (i = 0; level > i ; i++)
			rate = clk_divide(rate, request, 2, &div[i], level);

		if (rate_hz && (abs(rate-request) > abs(rate_hz-request)))
			continue;

		pr_debug("clk: %s.%d, pll.%d[%u] request[%ld] calc[%ld]\n",
			peri->dev_name, peri->dev_id, n, pll->clk.rate, request, rate);

		if (clk2) {
			s1 = -1, d1 = -1;	/* not use */
			s2 =  n, d2 = div[0];
		} else {
			s1 = n, d1 = div[0];
			s2 = _InCLKn_, d2 = div[1];
		}
		rate_hz = rate;
		freq_hz = clock_hz;
	}

	/* search 2th clock from input */
	if (!clk2 && abs(rate_hz-request) &&
		peri->clk_mask1 & ((1<<INPUT_CLKS) - 1)) {
		clk2 = 1;
		mask = peri->clk_mask1;
		level = 1;
		goto next;
	}

	spin_lock_irqsave(&peri->lock, flags);

	peri->clk_src0 = s1;
	peri->clk_div0 = d1;
	peri->clk_src1 = s2;
	peri->clk_div1 = d2;

	clk->rate = rate_hz;

	spin_unlock_irqrestore(&peri->lock, flags);

	pr_debug("clk: %s.%d, level[%d] src[%d,%d] %ld /(div0: %d * div1: %d) = %ld, %ld diff (%ld)\n",
		peri->dev_name, peri->dev_id, peri->level, peri->clk_src0, peri->clk_src1, freq_hz,
		peri->clk_div0, peri->clk_div1, rate_hz, request, abs(rate_hz-request));

	return clk->rate;
}

unsigned long clk_get_rate(struct clk *clk)
{
	struct nxp_clk_dev *cdev = clk_container(clk);

	if (cdev->link)
		clk = cdev->link;

	return clk->rate;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	struct nxp_clk_dev *cdev = clk_container(clk);
	struct nxp_clk_periph *peri = cdev->peri;
	unsigned long flags;
	int i;

	if (NULL == peri)
		return core_set_rate(clk, rate);

	clk_round_rate(clk, rate);

	spin_lock_irqsave(&peri->lock, flags);

	for (i = 0; peri->level > i ; i++)	{

		int s = (0 == i ? peri->clk_src0: peri->clk_src1);
		int d = (0 == i ? peri->clk_div0: peri->clk_div1);

		if (-1 == s)
			continue;

		clk_gen_rate(peri->base_addr, i, s, d);

		pr_debug("clk: %s.%d (%p) set_rate [%d] src[%d] div[%d]\n",
			peri->dev_name, peri->dev_id, peri->base_addr, i, s, d);
	}

	spin_unlock_irqrestore(&peri->lock, flags);

	return clk->rate;
}

void clk_put(struct clk *clk)
{
}

struct clk *clk_get(struct device *dev, const char *id)
{
	struct nxp_clk_dev *cdev = clk_dev_get(0);
    struct clk *clk = NULL;
    const char *str = NULL, *c = NULL;
	int i, devid;

	if (dev)
		str = dev_name(dev);

	if (id)
		str = id;

	for (i = 0; CLK_DEVS_NUM > i; i++, cdev++) {
		if (NULL == cdev->name)
			continue;
		if (!strncmp(cdev->name, str, strlen(cdev->name))) {
			c = strrchr((const char*)str, (int)'.');
			if (NULL == c || !cdev->peri)
				break;
	    	devid = simple_strtoul(++c, NULL, 10);
    		if (cdev->peri->dev_id == devid)
	    		break;
		}
	}

	if (CLK_DEVS_NUM > i)
		clk = &cdev->clk;
	else
		clk = &(clk_dev_get(7))->clk;	/* pclk */

	return clk ? clk : ERR_PTR(-ENOENT);
}

int clk_enable(struct clk *clk)
{
	struct nxp_clk_dev *cdev = clk_container(clk);
	struct nxp_clk_periph *peri = cdev->peri;
	unsigned long flags;
	int i = 0, inv = 0;
	unsigned int input_mask = INPUT_MASK;

	if (! peri)
		return 0;

	spin_lock_irqsave(&peri->lock, flags);
	pr_debug("clk: %s.%d enable (BCLK=%s, PCLK=%s)\n",
		peri->dev_name, peri->dev_id, _GATE_BCLK_ & peri->clk_mask0 ? "ON":"PASS",
		_GATE_PCLK_ & peri->clk_mask0 ? "ON":"PASS");

#if defined(CONFIG_SND_NXP_DFS)
    if (!strcmp(peri->dev_name, DEV_NAME_I2S))
		input_mask |= (1<<CONFIG_SND_NXP_PLLDEV);
#endif

	if (!(input_mask & peri->clk_mask0)) {
		/* Gated BCLK/PCLK enable */
		if (_GATE_BCLK_ & peri->clk_mask0)
			clk_gen_bclk(peri->base_addr, 1);

		if (_GATE_PCLK_ & peri->clk_mask0)
			clk_gen_pclk(peri->base_addr, 1);

		spin_unlock_irqrestore(&peri->lock, flags);
		return 0;
	}

	/* invert */
	inv = peri->clk_inv0;
	for (; peri->level > i; i++, inv = peri->clk_inv1)
		clk_gen_inv(peri->base_addr, i, inv);

	/* Gated BCLK/PCLK enable */
	if (_GATE_BCLK_ & peri->clk_mask0)
		clk_gen_bclk(peri->base_addr, 1);

	if (_GATE_PCLK_ & peri->clk_mask0)
		clk_gen_pclk(peri->base_addr, 1);

	/* restore clock rate */
	for (i = 0; peri->level > i ; i++)	{
		int s = (0 == i ? peri->clk_src0: peri->clk_src1);
		int d = (0 == i ? peri->clk_div0: peri->clk_div1);
		if (-1 == s)
			continue;
		clk_gen_rate(peri->base_addr, i, s, d);
	}

	clk_gen_enb(peri->base_addr, 1);
	peri->enable = true;

	spin_unlock_irqrestore(&peri->lock, flags);
	return 0;
}

void clk_disable(struct clk *clk)
{
	struct nxp_clk_dev *cdev = clk_container(clk);
	struct nxp_clk_periph *peri = cdev->peri;
	unsigned long flags;
	unsigned int input_mask = INPUT_MASK;

	if (! peri)
		return;

	spin_lock_irqsave(&peri->lock, flags);
	pr_debug("clk: %s.%d disable\n", peri->dev_name, peri->dev_id);

	peri->enable = false;

#if defined(CONFIG_SND_NXP_DFS)
    if (!strcmp(peri->dev_name, DEV_NAME_I2S))
		input_mask |= (1<<CONFIG_SND_NXP_PLLDEV);
#endif

	if (!(input_mask & peri->clk_mask0)) {
		/* Gated BCLK/PCLK disable */
		if (_GATE_BCLK_ & peri->clk_mask0)
			clk_gen_bclk(peri->base_addr, 0);

		if (_GATE_PCLK_ & peri->clk_mask0)
			clk_gen_pclk(peri->base_addr, 0);

		spin_unlock_irqrestore(&peri->lock, flags);
		return;
	}

	clk_gen_rate(peri->base_addr, 0, 7, 256);	/* for power save */
	clk_gen_enb (peri->base_addr, 0);

	/* Gated BCLK/PCLK disable */
	if (_GATE_BCLK_ & peri->clk_mask0)
		clk_gen_bclk(peri->base_addr, 0);

	if (_GATE_PCLK_ & peri->clk_mask0)
		clk_gen_pclk(peri->base_addr, 0);

	spin_unlock_irqrestore(&peri->lock, flags);
	return;
}

EXPORT_SYMBOL(clk_get_sys);
EXPORT_SYMBOL(clk_round_rate);
EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_set_rate);
EXPORT_SYMBOL(clk_put);
EXPORT_SYMBOL(clk_get);
EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_disable);

/*
 * Core clocks APIs
 */
void nxp_cpu_clock_update_rate(int pll)
{
	core_pll_update_link(pll);
}

unsigned int nxp_cpu_clock_hz(int type)
{
	return core_get_rate(type);
}

void nxp_cpu_periph_register_clock(int id, long ext1, long ext2)
{
	struct nxp_clk_periph *peri;

	if (id > CLK_ID_MAX) {
		printk("clk: invalid clock id %d (max=%d)\n", id, CLK_ID_MAX);
		return;
	}

	peri = &clk_periphs[id];

	if (ext1 &&  (peri->clk_mask0 & _EXTCLK1_))
		peri->clk_ext1 = ext1;

	if (ext2 &&  (peri->clk_mask0 & _EXTCLK2_))
		peri->clk_ext2 = ext2;

	pr_debug("clk: %s.%d register ext clock %ld,%ld\n",
		peri->dev_name, peri->dev_id, ext1, ext2);
}

void __init nxp_cpu_clock_init(void)
{
	struct nxp_clk_dev *cdev = clk_devices;
	struct nxp_clk_periph *peri = clk_periphs;
	struct clk *clk = NULL;
	int i = 0, n = 0;

	core_rate_init();

	for (i = 0; (CLK_CORE_NUM+CLK_PERI_NUM) > i; i++, cdev++) {
		if (CLK_CORE_NUM > i) {
			cdev->name = clk_core[i].name;
			clk = &cdev->clk;
			clk->rate = core_get_rate(i);
			continue;
		}

		peri = &clk_periphs[i-CLK_CORE_NUM];
		peri->base_addr = IO_ADDRESS(peri->base_addr);
		spin_lock_init(&peri->lock);

		cdev->peri = peri;
		cdev->name = peri->dev_name;

		if (!(INPUT_MASK & peri->clk_mask0)) {
			if (BLCK_MASK & peri->clk_mask0)
				cdev->clk.rate = core_get_rate(CORECLK_ID_BCLK);
			if (PLCK_MASK & peri->clk_mask0)
				cdev->clk.rate = core_get_rate(CORECLK_ID_PCLK);
		}

		/* prevent uart clock disable for low level debug message */
		#ifndef CONFIG_DEBUG_NXP_UART
		if (peri->dev_name) {
			#ifdef CONFIG_BACKLIGHT_PWM
			if (!strcmp(peri->dev_name,DEV_NAME_PWM))
				continue;
			#endif
			clk_gen_enb (peri->base_addr, 0);
			clk_gen_bclk(peri->base_addr, 0);
			clk_gen_pclk(peri->base_addr, 0);
		}
		#endif
	}

	cdev = clk_dev_get(CLK_CORE_NUM);
	for (i = 0; CLK_LINK_NUM > i; i++ ) {
		for (n = 0; CLK_PERI_NUM > n; n++, cdev++) {
			peri = cdev->peri;
			if (peri->periph_id == clk_link[i].id) {
				struct nxp_clk_dev *cl = &clk_devices[CLK_CORE_NUM+CLK_PERI_NUM+i];
				cl->name = clk_link[i].name;
				cl->link = &cdev->clk;
			#if (0) /* not support linked clock, ex uart */
				cl->peri = cdev->peri;
			#endif
			}
		}
		cdev = clk_dev_get(CLK_CORE_NUM);
	}

	printk("CPU : Clock Generator= %d EA, ", CLK_DEVS_NUM);
#ifdef CONFIG_ARM_NXP_CPUFREQ
	printk("DVFS = %s, PLL.%d\n", support_dvfs?"support":"can't support", CONFIG_NXP_CPUFREQ_PLLDEV);
#else
	printk("DVFS = Off\n");
#endif
}

void nxp_cpu_clock_print(void)
{
	int pll, cpu;

	core_rate_init();

	printk("PLL : [0] = %10u, [1] = %10u, [2] = %10u, [3] = %10u\n",
		core_hz[0], core_hz[1], core_hz[2], core_hz[3]);

	/* CPU */
	pll = pll_get_dvo (0), cpu = pll, support_dvfs = 1;
	printk("PLL%d: CPU FCLK = %10u, HCLK = %9u\n", pll, core_hz[4], core_hz[8]);

	/* BUS */
	pll = pll_get_dvo (1), support_dvfs = pll == cpu ? 0 : 1;
	printk("PLL%d: BUS BCLK = %10u, PCLK = %9u\n", pll, core_hz[6], core_hz[7]);
	if (pll == cpu) support_dvfs = 0;

	/* MEM */
	pll = pll_get_dvo (2), support_dvfs = pll == cpu ? 0 : 1;
	printk("PLL%d: MEM MCLK = %10u, DCLK = %9u, BCLK = %9u, PCLK = %9u\n",
		pll, core_hz[5], core_hz[9], core_hz[10], core_hz[11]);

	/* G3D */
	pll = pll_get_dvo (3), support_dvfs = pll == cpu ? 0 : 1;
	printk("PLL%d: G3D BCLK = %10u\n", pll, core_hz[12]);

	/* MPEG */
	pll = pll_get_dvo (4), support_dvfs = pll == cpu ? 0 : 1;
	printk("PLL%d: MPG BCLK = %10u, PCLK = %9u\n", pll, core_hz[13], core_hz[14]);
}

void nxp_cpu_clock_resume(void)
{
	struct nxp_clk_periph *peri;
	int cnt = CLK_PERI_NUM;
	int i;

	for (i = 0; cnt > i; i++) {
		peri = &clk_periphs[i];
		/* exception */
		if (_GATE_PCLK_ & peri->clk_mask0) {
	#ifdef CONFIG_I2C_NXP
			if (!strcmp("nxp-i2c", peri->dev_name))
				clk_gen_pclk(peri->base_addr, 1);
	#endif
		}
	}
}
