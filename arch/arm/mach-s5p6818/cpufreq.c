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
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/delay.h>
#include <mach/platform.h>

#ifdef CONFIG_ARM_NXP_CPUFREQ_DEBUG
#define	DBGOUT(msg...)		printk(msg)
#else
#define	DBGOUT(msg...)
#endif

struct pll_pms {
	long rate;	/* unint Khz */
	int	P;
	int	M;
	int	S;
	int K;
};

// PLL 0,1
static struct pll_pms pll0_1_pms [] =
{
    [ 0] = { .rate = 1600000, .P = 6, .M =  400, .S = 0, .K = 0 },
    [ 1] = { .rate = 1500000, .P = 6, .M =  375, .S = 0, .K = 0 },
    [ 2] = { .rate = 1400000, .P = 6, .M =  350, .S = 0, .K = 0 },
    [ 3] = { .rate = 1300000, .P = 6, .M =  325, .S = 0, .K = 0 },
    [ 4] = { .rate = 1200000, .P = 3, .M =  300, .S = 1, .K = 0 },
    [ 5] = { .rate = 1100000, .P = 3, .M =  275, .S = 1, .K = 0 },
    [ 6] = { .rate = 1000000, .P = 3, .M =  250, .S = 1, .K = 0 },
    [ 7] = { .rate =  900000, .P = 3, .M =  225, .S = 1, .K = 0 },
    [ 8] = { .rate =  800000, .P = 3, .M =  200, .S = 1, .K = 0 },
    [ 9] = { .rate =  700000, .P = 3, .M =  175, .S = 1, .K = 0 },
    [10] = { .rate =  600000, .P = 2, .M =  200, .S = 2, .K = 0 },
    [11] = { .rate =  500000, .P = 3, .M =  250, .S = 2, .K = 0 },
    [12] = { .rate =  400000, .P = 3, .M =  200, .S = 2, .K = 0 },
    [13] = { .rate =  300000, .P = 2, .M =  200, .S = 3, .K = 0 },
    [14] = { .rate =  200000, .P = 3, .M =  200, .S = 3, .K = 0 },
    [15] = { .rate =  100000, .P = 3, .M =  200, .S = 4, .K = 0 },
};

// PLL 2,3
static struct pll_pms pll2_3_pms [] =
{
    [ 0] = { .rate = 1600000, .P = 3, .M = 400, .S = 1, .K = 0 },
    [ 1] = { .rate = 1500000, .P = 3, .M = 375, .S = 1, .K = 0 },
    [ 2] = { .rate = 1400000, .P = 3, .M = 350, .S = 1, .K = 0 },
    [ 3] = { .rate = 1300000, .P = 3, .M = 325, .S = 1, .K = 0 },
    [ 4] = { .rate = 1200000, .P = 3, .M = 300, .S = 1, .K = 0 },
    [ 5] = { .rate = 1100000, .P = 3, .M = 275, .S = 1, .K = 0 },
    [ 6] = { .rate = 1000000, .P = 3, .M = 250, .S = 1, .K = 0 },
    [ 7] = { .rate =  900000, .P = 3, .M = 225, .S = 1, .K = 0 },
    [ 8] = { .rate =  800000, .P = 3, .M = 200, .S = 1, .K = 0 },
    [ 9] = { .rate =  700000, .P = 3, .M = 175, .S = 1, .K = 0 },
    [10] = { .rate =  600000, .P = 3, .M = 150, .S = 1, .K = 0 },
    [11] = { .rate =  500000, .P = 3, .M = 250, .S = 2, .K = 0 },
    [12] = { .rate =  400000, .P = 3, .M = 200, .S = 2, .K = 0 },
    [13] = { .rate =  300000, .P = 3, .M = 150, .S = 2, .K = 0 },
    [14] = { .rate =  200000, .P = 3, .M = 200, .S = 3, .K = 0 },
    [15] = { .rate =  100000, .P = 3, .M = 200, .S = 4, .K = 0 },
};

#if defined(CONFIG_SND_NXP_DFS)
static struct pll_pms audio_dfs_pms [] =
{
	[ 0] = { .rate = 73728000, .P = 3, .M = 147, .S = 1, .K = 29884 }, 
	[ 1] = { .rate = 67737600, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[ 2] = { .rate = 49152000, .P = 3, .M = 147, .S = 1, .K = 29884 }, 
	[ 3] = { .rate = 45158400, .P = 3, .M = 158, .S = 1, .K =  3565 }, 
	[ 4] = { .rate = 36864000, .P = 3, .M = 147, .S = 1, .K = 29884 }, 
	[ 5] = { .rate = 33868800, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[ 6] = { .rate = 24576000, .P = 2, .M = 106, .S = 1, .K = 32506 }, 
	[ 7] = { .rate = 22579200, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[ 8] = { .rate = 18432000, .P = 3, .M = 147, .S = 1, .K = 29884 }, 
	[ 9] = { .rate = 16934400, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[10] = { .rate = 16384000, .P = 3, .M = 147, .S = 1, .K = 29884 }, 
	[11] = { .rate = 12288000, .P = 2, .M = 102, .S = 1, .K = 26214 }, 
	[12] = { .rate = 11289600, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[13] = { .rate =  8467200, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[14] = { .rate =  8192000, .P = 2, .M = 106, .S = 1, .K = 32506 }, 
	[15] = { .rate =  6144000, .P = 2, .M = 102, .S = 1, .K = 26214 }, 
	[16] = { .rate =  5644800, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[17] = { .rate =  4233600, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
	[18] = { .rate =  4096000, .P = 2, .M = 102, .S = 1, .K = 26214 }, 
	[19] = { .rate =  3072000, .P = 2, .M = 102, .S = 1, .K = 26214 }, 
    [20] = { .rate =  2822400, .P = 3, .M = 169, .S = 1, .K = 22544 }, 
    [21] = { .rate =  2048000, .P = 2, .M = 102, .S = 1, .K = 26214 }, 
};
#endif

#define	PLL0_1_SIZE		ARRAY_SIZE(pll0_1_pms)
#define	PLL2_3_SIZE		ARRAY_SIZE(pll2_3_pms)
#if defined(CONFIG_SND_NXP_DFS)
#define	AUDIO_DFS_SIZE	ARRAY_SIZE(audio_dfs_pms)
#endif

#define	PMS_RATE(p, i)	((&p[i])->rate)
#define	PMS_P(p, i)		((&p[i])->P)
#define	PMS_M(p, i)		((&p[i])->M)
#define	PMS_S(p, i)		((&p[i])->S)
#define	PMS_K(p, i)		((&p[i])->K)

#define PLL_S_BITPOS	0
#define PLL_M_BITPOS    8
#define PLL_P_BITPOS    18
#define PLL_K_BITPOS    16

static void core_pll_change(int PLL, int P, int M, int S, int K)
{
	struct NX_CLKPWR_RegisterSet *clkpwr =
	(struct NX_CLKPWR_RegisterSet*)IO_ADDRESS(PHY_BASEADDR_CLKPWR_MODULE);

#if defined(CONFIG_SND_NXP_DFS)
	if (PLL > 3)
		PLL = CONFIG_SND_NXP_PLLDEV;
#endif

	DBGOUT("%s [PLL %d][P:%d,M:%d,S:%d,K:%d]\n", __func__, PLL, P, M, S, K);
	// 1. change PLL0 clock to Oscillator Clock
	clkpwr->PLLSETREG[PLL] &= ~(1 << 28); 	// pll bypass on, xtal clock use
	clkpwr->CLKMODEREG0 = (1 << PLL); 		// update pll
	while(clkpwr->CLKMODEREG0 & (1<<31)); 	// wait for change update pll

	// 2. PLL Power Down & PMS value setting
	clkpwr->PLLSETREG[PLL] =((1UL << 29)			| // power down
							 (0UL << 28)    		| // clock bypass on, xtal clock use
							 (S   << PLL_S_BITPOS) 	|
							 (M   << PLL_M_BITPOS) 	|
							 (P   << PLL_P_BITPOS));
#if defined(CONFIG_SND_NXP_DFS)
	if (PLL > 1)
		clkpwr->PLLSETREG_SSCG[PLL] = (K << PLL_K_BITPOS) | (2<<0);
#endif
	clkpwr->CLKMODEREG0 = (1 << PLL); 				// update pll

	while(clkpwr->CLKMODEREG0 & (1<<31)); 			// wait for change update pll

	udelay(1);

	// 3. Update PLL & wait PLL locking
	clkpwr->PLLSETREG[PLL] &= ~((U32)(1UL<<29)); // pll power up
	clkpwr->CLKMODEREG0 = (1 << PLL); 			// update pll
	while(clkpwr->CLKMODEREG0 & (1<<31)); 		// wait for change update pll

	udelay(10);	// 1000us

	// 4. Change to PLL clock
	clkpwr->PLLSETREG[PLL] |= (1<<28); 			// pll bypass off, pll clock use
	clkpwr->CLKMODEREG0 = (1<<PLL); 			// update pll

	while(clkpwr->CLKMODEREG0 & (1<<31)); 		// wait for change update pll
}

static DEFINE_SPINLOCK(_pll_lock);
static unsigned long lock_flags;

static inline void core_pll_change_lock(bool lock)
{
	if (lock) {
		preempt_disable();
		spin_lock_irqsave(&_pll_lock, lock_flags);
	} else {
		spin_unlock_irqrestore(&_pll_lock, lock_flags);
		preempt_enable();
	}
}

static unsigned long cpu_pll_round(int pllno, unsigned long rate, int *p, int *m, int *s, int *k)
{
	struct pll_pms *pms;
	int len, idx = 0, n = 0, l = 0;
	long freq = 0;

	if (pllno < 4)
		rate /= 1000;

	DBGOUT("PLL.%d, %ld", pllno, rate);

	switch (pllno) {
	case 0 :
	case 1 : pms = pll0_1_pms; len = PLL0_1_SIZE; break;
	case 2 :
	case 3 : pms = pll2_3_pms; len = PLL2_3_SIZE; break;
#if defined(CONFIG_SND_NXP_DFS)
	case 5 : pms = audio_dfs_pms; len = AUDIO_DFS_SIZE; break; // only for audio dfs pms
#endif
	default: printk(KERN_ERR "Not support pll.%d (0~3)\n", pllno);
		return 0;
	}

	/* array index so -1 */
	idx = (len/2) - 1;

	while (1) {
		l = n + idx;
		freq = PMS_RATE(pms, l);
		if (freq == rate)
			break;

		if (rate > freq)
			len -= idx, idx >>= 1;
		else
			n += idx, idx = (len-n-1)>>1;

		if (0 == idx) {
			int q = l;
			if (abs(rate - freq) > abs(rate - PMS_RATE(pms, q+1)))
				q += 1;
			if (abs(rate - PMS_RATE(pms, q)) >= abs(rate - PMS_RATE(pms, q-1)))
				q -= 1;
			l = q;
			break;
		}
	}

	if (p) *p = PMS_P(pms, l);
	if (m) *m = PMS_M(pms, l);
	if (s) *s = PMS_S(pms, l);
	if (k) *k = PMS_K(pms, l);

	DBGOUT("(real %ld Khz, P=%d ,M=%3d, S=%d)\n",
		PMS_RATE(pms, l), PMS_P(pms, l), PMS_M(pms, l), PMS_S(pms, l));

	return PMS_RATE(pms, l);
}

static DEFINE_SPINLOCK(lock);

unsigned long nxp_cpu_pll_change_frequency(int pllno, unsigned long rate)
{
	int p = 0, m = 0, s = 0, k = 0;
	unsigned long freq;
	u_long flags;

	DBGOUT("%s %d %ld\n", __func__, pllno, rate);

#if defined(CONFIG_NXP_DFS_BCLK) || defined(CONFIG_SND_NXP_DFS)
    mdelay(1);
#endif

	spin_lock_irqsave(&lock, flags);

	freq = cpu_pll_round(pllno, rate, &p, &m, &s, &k);
	core_pll_change(pllno, p, m, s, k);

	spin_unlock_irqrestore(&lock, flags);

#if defined(CONFIG_NXP_DFS_BCLK) || defined(CONFIG_SND_NXP_DFS)
    mdelay(1);
#endif

	return freq;
}
