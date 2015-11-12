#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/atomic.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/suspend.h>
#include <linux/notifier.h>

// serializing
#include <linux/list.h>

// test cpu down, up
#include <linux/smp.h>
#include <linux/cpu.h>
/*#include <linux/freezer.h>*/
// for only test
#include <linux/kthread.h>
#include <linux/dma-mapping.h>

#include <mach/platform.h>
#include <mach/soc.h>
#include <mach/nxp-dfs-bclk.h>
#include <mach/s5p4418_bus.h>

#ifdef CONFIG_NXP_DFS_BCLK

#define BCLK_MIN    120000000
/*#define BCLK_MIN    100000000*/
#define BCLK_MEDIUM 200000000
#define BCLK_MAX    400000000

static int default_dfs_bclk_func(uint32_t pll_num, uint32_t counter, uint32_t user_bitmap, uint32_t current_bclk);

static struct dfs_bclk_manager {
    uint32_t bclk_pll_num;
    atomic_t counter;
    atomic_t user_bitmap;
    uint32_t current_bclk;
    int suspended;
    struct timer_list pm_timer;
    struct notifier_block pm_notifier;
    struct delayed_work delayed_work;

    dfs_bclk_func func;
} dfs_bclk_manager = {
#ifdef CONFIG_S5P4418_DFS_BCLK_PLL_0
    .bclk_pll_num = 0,
#else
    .bclk_pll_num = 1,
#endif
    .counter = ATOMIC_INIT(0),
    .user_bitmap = ATOMIC_INIT(0),
    .current_bclk = BCLK_MAX,
    .suspended = 0,
    .func = default_dfs_bclk_func,
};

/**
 * notifier interface
 */
static ATOMIC_NOTIFIER_HEAD(bclk_dfs_notifier_list);

void bclk_dfs_register_notify(struct notifier_block *nb)
{
     atomic_notifier_chain_register(&bclk_dfs_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(bclk_dfs_register_notify);

void bclk_dfs_unregister_notify(struct notifier_block *nb)
{
     atomic_notifier_chain_unregister(&bclk_dfs_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(bclk_dfs_unregister_notify);

void bclk_dfs_notify_change(uint32_t *clk_hz)
{
    atomic_notifier_call_chain(&bclk_dfs_notifier_list, BCLK_CHANGED, clk_hz);
}
/**
 * sysfs attributes
 */
static uint32_t bclk_min = BCLK_MIN;
/*static uint32_t bclk_medium = BCLK_MEDIUM;*/
static uint32_t bclk_max = BCLK_MAX;
static uint32_t enable = true;

static struct task_struct *bclk_dfs_task;

#define ATOMIC_SET_MASK(PTR, MASK)  \
    do { \
        unsigned long oldval = atomic_read(PTR); \
        unsigned long newval = oldval | MASK; \
        atomic_cmpxchg(PTR, oldval, newval); \
    } while (0)

#define ATOMIC_CLEAR_MASK(PTR, MASK) \
    atomic_clear_mask(MASK, (unsigned long *)&((PTR)->counter))

struct pll_pms {
    long rate;	/* unint Khz */
    int	P;
    int	M;
    int	S;
};

// PLL 0,1
static struct pll_pms pll0_1_pms [] =
{
    [ 0] = { .rate = 2000000, .P = 6, .M = 500, .S = 0, },
    [ 1] = { .rate = 1900000, .P = 6, .M = 475, .S = 0, },
    [ 2] = { .rate = 1800000, .P = 4, .M = 300, .S = 0, },
    [ 3] = { .rate = 1700000, .P = 6, .M = 425, .S = 0, },
    [ 4] = { .rate = 1600000, .P = 3, .M = 400, .S = 1, },
    [ 5] = { .rate = 1500000, .P = 3, .M = 375, .S = 1, },
    [ 6] = { .rate = 1400000, .P = 3, .M = 350, .S = 1, },
    [ 7] = { .rate = 1300000, .P = 3, .M = 325, .S = 1, },
    [ 8] = { .rate = 1200000, .P = 3, .M = 300, .S = 1, },
    [ 9] = { .rate = 1100000, .P = 6, .M = 550, .S = 1, },
    [10] = { .rate = 1000000, .P = 6, .M = 500, .S = 1, },
    [11] = { .rate =  900000, .P = 4, .M = 300, .S = 1, },
    [12] = { .rate =  800000, .P = 6, .M = 400, .S = 1, },
    [13] = { .rate =  780000, .P = 4, .M = 260, .S = 1, },
    [14] = { .rate =  760000, .P = 6, .M = 380, .S = 1, },
    [15] = { .rate =  740000, .P = 6, .M = 370, .S = 1, },
    [16] = { .rate =  720000, .P = 4, .M = 240, .S = 1, },
    [17] = { .rate =  562000, .P = 6, .M = 562, .S = 2, },
    [18] = { .rate =  533000, .P = 6, .M = 533, .S = 2, },
    [19] = { .rate =  490000, .P = 6, .M = 490, .S = 2, },
    [20] = { .rate =  470000, .P = 6, .M = 470, .S = 2, },
    [21] = { .rate =  460000, .P = 6, .M = 460, .S = 2, },
    [22] = { .rate =  450000, .P = 4, .M = 300, .S = 2, },
    [23] = { .rate =  440000, .P = 6, .M = 440, .S = 2, },
    [24] = { .rate =  430000, .P = 6, .M = 430, .S = 2, },
    [25] = { .rate =  420000, .P = 4, .M = 280, .S = 2, },
    [26] = { .rate =  410000, .P = 6, .M = 410, .S = 2, },
    /*[27] = { .rate =  400000, .P = 6, .M = 400, .S = 2, },*/
    [27] = { .rate =  400000, .P = 3, .M = 200, .S = 2, },
    [28] = { .rate =  399000, .P = 4, .M = 266, .S = 2, },
    [29] = { .rate =  390000, .P = 4, .M = 260, .S = 2, },
    [30] = { .rate =  384000, .P = 4, .M = 256, .S = 2, },
    [31] = { .rate =  350000, .P = 6, .M = 350, .S = 2, },
    [32] = { .rate =  330000, .P = 4, .M = 220, .S = 2, },
    [33] = { .rate =  300000, .P = 4, .M = 400, .S = 3, },
    [34] = { .rate =  266000, .P = 6, .M = 532, .S = 3, },
    [35] = { .rate =  250000, .P = 6, .M = 500, .S = 3, },
    [36] = { .rate =  220000, .P = 6, .M = 440, .S = 3, },
    /*[37] = { .rate =  200000, .P = 6, .M = 400, .S = 3, },*/
    [37] = { .rate =  200000, .P = 3, .M = 200, .S = 3, },
    [38] = { .rate =  166000, .P = 6, .M = 332, .S = 3, },
    [39] = { .rate =  147500, .P = 6, .M = 590, .S = 4, },	// 147.456000
    [40] = { .rate =  133000, .P = 6, .M = 532, .S = 4, },
    /*[41] = { .rate =  125000, .P = 6, .M = 500, .S = 4, },*/
    [41] = { .rate =  120000, .P = 3, .M = 200, .S = 4, },
    [42] = { .rate =  100000, .P = 6, .M = 400, .S = 4, },
    [43] = { .rate =   96000, .P = 4, .M = 256, .S = 4, },
    [44] = { .rate =   48000, .P = 3, .M =  96, .S = 4, },
};

// PLL 2,3
static struct pll_pms pll2_3_pms [] =
{
    [ 0] = { .rate = 2000000, .P = 3, .M = 250, .S = 0, },
    [ 1] = { .rate = 1900000, .P = 4, .M = 317, .S = 0, },
    [ 2] = { .rate = 1800000, .P = 3, .M = 225, .S = 0, },
    [ 3] = { .rate = 1700000, .P = 4, .M = 283, .S = 0, },
    [ 4] = { .rate = 1600000, .P = 3, .M = 200, .S = 0, },
    [ 5] = { .rate = 1500000, .P = 4, .M = 250, .S = 0, },
    [ 6] = { .rate = 1400000, .P = 3, .M = 175, .S = 0, },
    [ 7] = { .rate = 1300000, .P = 4, .M = 217, .S = 0, },
    [ 8] = { .rate = 1200000, .P = 3, .M = 150, .S = 0, },
    [ 9] = { .rate = 1100000, .P = 3, .M = 275, .S = 1, },
    [10] = { .rate = 1000000, .P = 3, .M = 250, .S = 1, },
    [11] = { .rate =  900000, .P = 3, .M = 225, .S = 1, },
    [12] = { .rate =  800000, .P = 3, .M = 200, .S = 1, },
    [13] = { .rate =  780000, .P = 3, .M = 195, .S = 1, },
    [14] = { .rate =  760000, .P = 3, .M = 190, .S = 1, },
    [15] = { .rate =  740000, .P = 3, .M = 185, .S = 1, },
    [16] = { .rate =  720000, .P = 3, .M = 180, .S = 1, },
    [17] = { .rate =  562000, .P = 4, .M = 187, .S = 1, },
    [18] = { .rate =  533000, .P = 4, .M = 355, .S = 2, },
    [19] = { .rate =  490000, .P = 3, .M = 245, .S = 2, },
    [20] = { .rate =  470000, .P = 3, .M = 235, .S = 2, },
    [21] = { .rate =  460000, .P = 3, .M = 230, .S = 2, },
    [22] = { .rate =  450000, .P = 3, .M = 225, .S = 2, },
    [23] = { .rate =  440000, .P = 3, .M = 220, .S = 2, },
    [24] = { .rate =  430000, .P = 3, .M = 215, .S = 2, },
    [25] = { .rate =  420000, .P = 3, .M = 210, .S = 2, },
    [26] = { .rate =  410000, .P = 3, .M = 205, .S = 2, },
    [27] = { .rate =  400000, .P = 3, .M = 200, .S = 2, },
    [28] = { .rate =  399000, .P = 4, .M = 266, .S = 2, },
    [29] = { .rate =  390000, .P = 3, .M = 195, .S = 2, },
    [30] = { .rate =  384000, .P = 3, .M = 192, .S = 2, },
    [31] = { .rate =  350000, .P = 3, .M = 175, .S = 2, },
    [32] = { .rate =  330000, .P = 3, .M = 165, .S = 2, },
    [33] = { .rate =  300000, .P = 3, .M = 150, .S = 2, },
    [34] = { .rate =  266000, .P = 3, .M = 266, .S = 3, },
    [35] = { .rate =  250000, .P = 3, .M = 250, .S = 3, },
    [36] = { .rate =  220000, .P = 3, .M = 220, .S = 3, },
    [37] = { .rate =  200000, .P = 3, .M = 200, .S = 3, },
    [38] = { .rate =  166000, .P = 3, .M = 166, .S = 3, },
    [39] = { .rate =  147500, .P = 3, .M = 147, .S = 3, },	// 147456
    [40] = { .rate =  133000, .P = 3, .M = 266, .S = 4, },
    [41] = { .rate =  125000, .P = 3, .M = 250, .S = 4, },
    [42] = { .rate =  100000, .P = 3, .M = 200, .S = 4, },
    [43] = { .rate =   96000, .P = 3, .M = 192, .S = 4, },
    [44] = { .rate =   48000, .P = 3, .M =  96, .S = 4, },
};

#define	PLL0_1_SIZE		ARRAY_SIZE(pll0_1_pms)
#define	PLL2_3_SIZE		ARRAY_SIZE(pll2_3_pms)

#define	PMS_RATE(p, i)	((&p[i])->rate)
#define	PMS_P(p, i)		((&p[i])->P)
#define	PMS_M(p, i)		((&p[i])->M)
#define	PMS_S(p, i)		((&p[i])->S)


#define PLL_S_BITPOS	0
#define PLL_M_BITPOS    8
#define PLL_P_BITPOS    18

#include <linux/clocksource.h>

static struct pll_pms *s_p;
static int s_l;
static volatile struct NX_CLKPWR_RegisterSet *clkpwr;

static unsigned long _cpu_pll_change_frequency(int no, unsigned long rate)
{
    struct pll_pms *p;
    int len, i = 0, n = 0, l = 0;
    long freq = 0;

    rate /= 1000;
    printk("PLL.%d, %ld\n", no, rate);

    switch (no) {
        case 0 :
        case 1 : p = pll0_1_pms; len = PLL0_1_SIZE; break;
        case 2 :
        case 3 : p = pll2_3_pms; len = PLL2_3_SIZE; break;
        default: printk(KERN_ERR "Not support pll.%d (0~3)\n", no);
                 return 0;
    }

    i = len/2;
    while (1) {
        l = n + i;
        freq = PMS_RATE(p, l);
        if (freq == rate)
            break;

        if (rate > freq)
            len -= i, i >>= 1;
        else
            n += i, i = (len-n-1)>>1;

        if (0 == i) {
            int k = l;
            if (abs(rate - freq) > abs(rate - PMS_RATE(p, k+1)))
                k += 1;
            if (abs(rate - PMS_RATE(p, k)) >= abs(rate - PMS_RATE(p, k-1)))
                k -= 1;
            l = k;
            break;
        }
    }

    s_p = p;
    s_l = l;
    return 0;
}

#if 1
/*u32 pll_data = (P << 24) | (M << 8) | (S << 2) | pll_num;*/
void _real_change_pll(volatile u32 *clkpwr_reg, u32 pll_data)
{
    // here : block other ip bus access
    uint32_t pll_num = pll_data & 0x00000003;
    uint32_t s       = (pll_data & 0x000000fc) >> 2;
    uint32_t m       = (pll_data & 0x00ffff00) >> 8;
    uint32_t p       = (pll_data & 0xff000000) >> 24;
    volatile u32 *pllset_reg = (clkpwr_reg + 2 + pll_num);
    /*printk("clkpwr reg %p, pllset reg %p, plldata 0x%x, p %d, m %d, s %d\n", clkpwr_reg, pllset_reg, pll_data, p, m, s);*/

    *pllset_reg &= ~(1 << 28);
    *clkpwr_reg  = (1 << pll_num);
    while(*clkpwr_reg & (1<<31));

    *pllset_reg  = ((1UL << 29) |
            (0UL << 28) |
            (s   << 0)  |
            (m   << 8)  |
            (p   << 18));
    *clkpwr_reg  = (1 << pll_num);
    while(*clkpwr_reg & (1<<31));

    *pllset_reg &= ~((u32)(1UL<<29));
    *clkpwr_reg  = (1 << pll_num);
    while(*clkpwr_reg & (1<<31));

    *pllset_reg |= (1 << 28);
    *clkpwr_reg  = (1 << pll_num);
    while(*clkpwr_reg & (1<<31));
    // here : unblock other ip bus access
}
#endif

// PHYCONTROL0
#define PHY_STARPOINT    8
#define PHY_INC            16
#define PHY_DQSDLY        4
#define PHY_DIFDQS        3
#define PHY_DLLON        1
#define PHY_START        0
// PHYCONTROL1
#define PHY_OFFSETD        16
#define PHY_OFFSETC        8
#define PHY_REF            4
#define PHY_SHIFTC        0
// CONCONTROL
#define RD_FETCH        12 // read data fetch cycles
#define QOS_FASTEN        11 //
#define CHIP1_EMPTY        9 //
#define CHIP0_EMPTY        8 //
#define AREFEN            5 // read data fetch cycles
#define DQ_PULLDOWNDIS    4 // read data fetch cycles
#define IO_PD_CON        3 // read data fetch cycles
// MEMCONTROL
#define MRR_BYTE        25//
#define MEM_BL            20//
#define MEM_WIDTH        12
#define MEM_TYPE        8

// MEMCONFIG0
#define CHIP0_BASE        24
#define CHIP0_MASK        16
#define CHIP0_MAP        12
#define CHIP0_COLADBITS    8    // column address bits
#define CHIP0_ROWADBITS    4
#define CHIP0_BANK        0
// TIMINGROW
#define T_RFC            24
#define T_RRD            20
#define T_RP            16
#define T_RCD            12
#define T_RC            6
#define T_RAS            0

// TIMINGDATA
#define T_WTR            28
#define T_WR            24
#define T_RTP            20
#define T_CL            16
#define T_WL            8
#define T_RL            0

// Direct Command
#define DIRCMD_TYPE_SHIFT       (24)
#define DIRCMD_TYPE_MASK        (0xF)
    #define DIRCMD_MRS_EMRS     (0x0)
    #define DIRCMD_PALL         (0x1)
    #define DIRCMD_PRE          (0x2)
    #define DIRCMD_DPD          (0x3)
    #define DIRCMD_REFS         (0x4)
    #define DIRCMD_REFA         (0x5)
    #define DIRCMD_CKEL         (0x6)
    #define DIRCMD_NOP          (0x7)
    #define DIRCMD_REFSX        (0x8)
    #define DIRCMD_MRR          (0x9)
    #define DIRCMD_ZQINIT       (0xA)
    #define DIRCMD_ZQOPER       (0xB)
    #define DIRCMD_ZQCS         (0xC)

#define DIRCMD_CHIP_SHIFT       (20)
#define DIRCMD_CHIP_MASK        (0x1)
    #define DIRCMD_CHIP_1       (0x1)
    #define DIRCMD_CHIP_0       (0x0)

#define DIRCMD_BANK_SHIFT       (16)
#define DIRCMD_BANK_MASK        (0x7)

#define DIRCMD_ADDR_SHIFT       (0)
#define DIRCMD_ADDR_MASK        (0xFF)

#define DIRCMD_MRS_MODE_SHIFT   (16)
#define DIRCMD_MRS_MODE_MASK    (0x3)
    #define DIRCMD_MR0          (0x0)
    #define DIRCMD_MR1          (0x1)
    #define DIRCMD_MR2          (0x2)
    #define DIRCMD_MR3          (0x3)

#define DIRCMD_MRS_DATA_SHIFT   (0x0)
#define DIRCMD_MRS_DATA_MASK    (0xFF)



// DDR PHY
// DLL Lock
#define PHYCON12            0x30    // start point, inc, force
#define ctrl_startpoint     24
#define ctrl_inc            16
#define ctrl_start          6
#define ctrl_dllon          5
#define ctrl_ref            1

#define PHYCON5             0x14
#define PHYCON8             0x20
#define PHYCON1             0x4
#define PHYCON2             0x8
#define OFF                 0x0
#define ON                  0xFFFFFFFF
#define C5_DEFAULT_RD       0xF
#define C5_VWM_S_FAIL_RD    0xC
#define PHYCON14            0x38
#define PHYCON32            0x84
#define PHYCON33            0x88
#define PHYCON34            0x8C
#define PHYCON16            0x40
#define PHYCON17            0x48
#define PHYCON18            0x4C
#define PHYCON12            0x30
#define PHYCON13            0x34
#define PHYCON26            0x6C
#define PHYCON24            0x64
#define PHYCON42            0xAC
#define PHYCON0             0x0
#define    DDR3_CON0            0x17020A40


#define DOWN    1
#if DOWN
static int cpu_down_force(void)
{
    int cpu, cur = raw_smp_processor_id();
    int stopped = 0;
    for_each_present_cpu(cpu) {
        if (cpu == cur)
            continue;
        if (!cpu_online(cpu))
            continue;
        stopped |= 1<<cpu;
        cpu_down(cpu);

    }
    return stopped;

}
static void cpu_up_force(int stopped)
{
    int cpu;
    for_each_present_cpu(cpu) {
        if (stopped & 1<<cpu)
            cpu_up(cpu);

    }

}
#else
static int  cpu_down_force(void)            { return 0;  }
static void cpu_up_force(int stopped)   {  }
#endif

static inline void _disable_irq_and_set(uint32_t pll_num, uint32_t bclk)
{
    bool pending = false;
    volatile NX_DREX_REG *pdrex = (volatile NX_DREX_REG *)IO_ADDRESS(PHY_BASEADDR_DREX);
    volatile u32 *pl301_bot = (volatile u32 *)IO_ADDRESS(0xC0050000);
    int stop;
    volatile u32 tmp;

	if (dfs_bclk_manager.suspended)
		return;

    _cpu_pll_change_frequency(pll_num, bclk);

    WARN(0 != raw_smp_processor_id(), "BCLK Dynamic Frequency CPU.%d  conflict with BCLK DFS...\n",
            raw_smp_processor_id());

    stop = cpu_down_force();

    preempt_disable();
    local_irq_disable();

    tmp = *((u32*)clkpwr);
    mb();
    tmp = *((u32*)pl301_bot);
    mb();

    pdrex->DIRECTCMD = ((DIRCMD_PALL << DIRCMD_TYPE_SHIFT) | (DIRCMD_CHIP_0 << DIRCMD_CHIP_SHIFT)); // precharge all cmd
    pdrex->CONCONTROL &= ~(0x1 << 5);

    NX_DPC_SetInterruptEnableAll(0, false);

    do {
        pending = NX_DPC_GetInterruptPendingAll(0);
    } while (!pending);

    {
        extern void (*do_suspend)(ulong, ulong);
        void (*real_change_pll)(u32*, u32*, u32*, u32) = (void (*)(u32 *, u32 *, u32 *, u32))((ulong)do_suspend + 0x224);
        uint32_t P = PMS_P(s_p, s_l);
        uint32_t M = PMS_M(s_p, s_l);
        uint32_t S = PMS_S(s_p, s_l);
        u32 pll_data = (P << 24) | (M << 8) | (S << 2) | pll_num;
        real_change_pll((u32 *)clkpwr, (u32 *)pl301_bot, (u32 *)pdrex, pll_data);
        /*real_change_pll((u32 *)clkpwr, (u32 *)do_suspend, (u32 *)pdrex, pll_data);*/
        /*_real_change_pll((u32 *)clkpwr, pll_data);*/
    }

    NX_DPC_ClearInterruptPendingAll(0);
    NX_DPC_SetInterruptEnableAll(0, true);

    pdrex->DIRECTCMD = ((DIRCMD_PALL << DIRCMD_TYPE_SHIFT) | (DIRCMD_CHIP_0 << DIRCMD_CHIP_SHIFT)); // precharge all cmd
    pdrex->CONCONTROL |= (0x1 << 5);

    local_irq_enable();
    preempt_enable();

    cpu_up_force(stop);
}

static int bclk_dfs_thread(void *unused)
{
    /*uint32_t pll = dfs_bclk_manager.bclk_pll_num;*/
    while (!kthread_should_stop()) {
        uint32_t bclk = dfs_bclk_manager.current_bclk;

        set_current_state(TASK_UNINTERRUPTIBLE);

        dfs_bclk_manager.current_bclk =
            dfs_bclk_manager.func(
                    dfs_bclk_manager.bclk_pll_num,
                    atomic_read(&dfs_bclk_manager.counter),
                    atomic_read(&dfs_bclk_manager.user_bitmap),
                    dfs_bclk_manager.current_bclk
                    );
        if (bclk != dfs_bclk_manager.current_bclk)
            bclk_dfs_notify_change(&dfs_bclk_manager.current_bclk);
        /*_disable_irq_and_set(pll, bclk);*/
        schedule();

        if (kthread_should_stop())
            break;

        set_current_state(TASK_INTERRUPTIBLE);

    }

    return 0;
}

static void bclk_dfs_pm_timer(unsigned long p)
{
	struct dfs_bclk_manager *manager = (struct dfs_bclk_manager *)p;
	manager->suspended = 0;
}

static int bclk_dfs_pm_notify(struct notifier_block *this,
        unsigned long mode, void *unused)
{
	struct dfs_bclk_manager *manager = container_of(this,
					struct dfs_bclk_manager, pm_notifier);
	struct timer_list *timer = &manager->pm_timer;

    switch(mode) {
    case PM_SUSPEND_PREPARE:
		del_timer(timer);
    	manager->suspended = 1;
    	break;
    case PM_POST_SUSPEND:
		timer->expires = get_jiffies_64() + msecs_to_jiffies(5000);
		add_timer(timer);
        break;
    }
    return 0;
}

static void _delayed_work_func(struct work_struct *work)
{
    wake_up_process(bclk_dfs_task);
}

static int default_dfs_bclk_func(uint32_t pll_num, uint32_t counter, uint32_t user_bitmap, uint32_t current_bclk)
{
    uint32_t bclk = current_bclk;
    if (counter > 0) {
        if (user_bitmap & ((1 << BCLK_USER_MPEG) | (1 << BCLK_USER_OGL))) {
            if (bclk != bclk_max) {
                bclk = bclk_max;
            }
        } else if (user_bitmap & (1 << BCLK_USER_DMA)) {
            bclk = BCLK_MEDIUM;
        }
    } else {
        bclk = bclk_min;
    }

    if (bclk != current_bclk) {
        _disable_irq_and_set(pll_num, bclk);
        /*bclk_dfs_notify_change(&dfs_bclk_manager.current_bclk);*/
    }

    return bclk;
}

extern void pm_suspend_data_save(void *mem);
extern void pm_suspend_data_restore(void *mem);

int bclk_get(uint32_t user)
{
    if (enable) {
        printk("%s: %d, %d\n", __func__, user, atomic_read(&dfs_bclk_manager.counter));
        /*cancel_delayed_work_sync(&dfs_bclk_manager.delayed_work);*/
        cancel_delayed_work(&dfs_bclk_manager.delayed_work);
        atomic_inc(&dfs_bclk_manager.counter);
        ATOMIC_SET_MASK(&dfs_bclk_manager.user_bitmap, 1<<user);
        if (user == BCLK_USER_MPEG)
            pm_suspend_data_save(NULL);
        /*dfs_bclk_manager.current_bclk =*/
            /*dfs_bclk_manager.func(*/
                    /*dfs_bclk_manager.bclk_pll_num,*/
                    /*atomic_read(&dfs_bclk_manager.counter),*/
                    /*atomic_read(&dfs_bclk_manager.user_bitmap),*/
                    /*dfs_bclk_manager.current_bclk*/
                    /*);*/
        wake_up_process(bclk_dfs_task);
    }
    return 0;
}

int bclk_put(uint32_t user)
{
    if (enable) {
        printk("%s: %d, %d\n", __func__, user, atomic_read(&dfs_bclk_manager.counter));
        /*cancel_delayed_work_sync(&dfs_bclk_manager.delayed_work);*/
        cancel_delayed_work(&dfs_bclk_manager.delayed_work);
        atomic_dec(&dfs_bclk_manager.counter);
        ATOMIC_CLEAR_MASK(&dfs_bclk_manager.user_bitmap, 1<<user);
        if (user == BCLK_USER_MPEG)
            pm_suspend_data_restore(NULL);
#if 1
        if (user != BCLK_USER_DMA) {
            /*dfs_bclk_manager.current_bclk =*/
                /*dfs_bclk_manager.func(*/
                        /*dfs_bclk_manager.bclk_pll_num,*/
                        /*atomic_read(&dfs_bclk_manager.counter),*/
                        /*atomic_read(&dfs_bclk_manager.user_bitmap),*/
                        /*dfs_bclk_manager.current_bclk*/
                        /*);*/
            wake_up_process(bclk_dfs_task);
        } else {
            queue_delayed_work(system_nrt_wq, &dfs_bclk_manager.delayed_work, msecs_to_jiffies(1000));
        }
#else
        /*dfs_bclk_manager.current_bclk =*/
            /*dfs_bclk_manager.func(*/
                    /*dfs_bclk_manager.bclk_pll_num,*/
                    /*atomic_read(&dfs_bclk_manager.counter),*/
                    /*atomic_read(&dfs_bclk_manager.user_bitmap),*/
                    /*dfs_bclk_manager.current_bclk*/
                    /*);*/
        wake_up_process(bclk_dfs_task);
#endif
    }
    return 0;
}

int register_dfs_bclk_func(dfs_bclk_func func)
{
    dfs_bclk_manager.func = func;
    return 0;
}

EXPORT_SYMBOL(bclk_get);
EXPORT_SYMBOL(bclk_put);
EXPORT_SYMBOL(register_dfs_bclk_func);

static ssize_t max_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", bclk_max);
}

static ssize_t max_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t n)
{
    uint32_t val;
    sscanf(buf, "%d", &val);
    if (val >= bclk_min && val <= BCLK_MAX) {
        printk("%s: bclk_max set to %d\n", __func__, val);
        bclk_max = val;
    } else {
        printk("%s: invalid value %d(%d-%d)\n", __func__, val, bclk_min, BCLK_MAX);
    }
    return n;
}

static ssize_t min_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", bclk_min);
}

static ssize_t min_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t n)
{
    uint32_t val;
    sscanf(buf, "%d", &val);
    if (val >= BCLK_MIN && val <= bclk_max) {
        printk("%s: bclk_min set to %d\n", __func__, val);
        bclk_min = val;
    } else {
        printk("%s: invalid value %d(%d-%d)\n", __func__, val, BCLK_MIN, bclk_max);
    }
    return n;
}

static ssize_t enable_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", enable);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t n)
{
    uint32_t val;
    sscanf(buf, "%d", &val);
    if (val > 0)
        enable = 1;
    else
        enable = 0;
    return n;
}

static ssize_t force_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t n)
{
    uint32_t val;
    sscanf(buf, "%d", &val);
    /*_set_and_wait(dfs_bclk_manager.bclk_pll_num, val);*/
    _disable_irq_and_set(dfs_bclk_manager.bclk_pll_num, val);
    return n;
}

static struct device_attribute max_attr =
__ATTR(max, S_IRUGO | S_IWUSR, max_show, max_store);
static struct device_attribute min_attr =
__ATTR(min, S_IRUGO | S_IWUSR, min_show, min_store);
static struct device_attribute enable_attr =
__ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);
static struct device_attribute force_attr =
__ATTR(force, S_IRUGO | S_IWUSR, NULL, force_store);

static struct attribute *attrs[] = {
    &max_attr.attr,
    &min_attr.attr,
    &enable_attr.attr,
    &force_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = (struct attribute **)attrs,
};

#if 0
// for only test
#include <mach/pm.h>
static int test_kthread_func(void *arg)
{
    volatile unsigned char *pvirt = NULL;
    dma_addr_t phys;
    volatile unsigned char *p = NULL;
    unsigned int count = 0;
    volatile unsigned char data;

    pvirt = dma_alloc_coherent(NULL, PAGE_SIZE, &phys, GFP_KERNEL);
    if (!pvirt) {
        printk("%s: can't alloc physical address\n", __func__);
        return -1;
    }

    p = pvirt;
    printk("%s run~~~\n", __func__);
    while (1) {
        *p++ = count++;
        mb();
        if (count >= PAGE_SIZE) {
            p = pvirt;
            count = 0;
        }
        data = *p;
        mb();

        __raw_writel((-1UL), SCR_USER_SIG8_RESET);
		__raw_writel(0xffaabbcc, SCR_USER_SIG8_SET);
		__raw_readl (SCR_USER_SIG8_READ);	/* verify */
    }

    return 0;
}
#endif

static int __init dfs_bclk_init(void)
{
    struct kobject *kobj = NULL;
    int ret = 0;

    kobj = kobject_create_and_add("dfs-bclk", &platform_bus.kobj);
    if (!kobj) {
        printk(KERN_ERR "%s: Failed to create kobject for dfs-bclk\n", __func__);
        return -EINVAL;
    }

    ret = sysfs_create_group(kobj, &attr_group);
    if (ret) {
        printk(KERN_ERR "%s: Failed to sysfs_create_group for dfs-bclk\n", __func__);
        kobject_del(kobj);
        return -ret;
    }

    clkpwr = (struct NX_CLKPWR_RegisterSet*)IO_ADDRESS(PHY_BASEADDR_CLKPWR_MODULE);

    INIT_DELAYED_WORK(&dfs_bclk_manager.delayed_work, _delayed_work_func);

    {
		static struct notifier_block *pm_notifier = &dfs_bclk_manager.pm_notifier;
		struct timer_list *timer = &dfs_bclk_manager.pm_timer;
		struct task_struct *p = NULL;
        int cpu = 0, err = 0;

		pm_notifier->notifier_call = bclk_dfs_pm_notify;
		err = register_pm_notifier(pm_notifier);
		if (err) {
			pr_err("BCLK pm notifier for cpu.%d failed\n", cpu);
			return PTR_ERR(p);
		}
		init_timer(timer);
		timer->function = bclk_dfs_pm_timer;
		timer->data = (unsigned long)&dfs_bclk_manager;

        p = kthread_create_on_node(bclk_dfs_thread, NULL, cpu_to_node(cpu), "bclk_dfs");
        if (IS_ERR(p)) {
            pr_err("BCLK Dynamic Frequency for cpu.%d failed\n", cpu);
            return PTR_ERR(p);

        }
        bclk_dfs_task = p;
        kthread_bind(p, cpu);
        wake_up_process(p);
    }
    // only test
#if 0
    {
        struct task_struct *thread_id = kthread_run(test_kthread_func, NULL, "kthread_dfs_test");
    }
#endif

    return 0;
}
module_init(dfs_bclk_init);

MODULE_AUTHOR("swpark <swpark@nexell.co.kr>");
MODULE_DESCRIPTION("DFS BCLK Manger for S5P4418");
MODULE_LICENSE("GPL");
#else
int bclk_get(uint32_t user) {
    return 0;
}
int bclk_put(uint32_t user) {
    return 0;
}
int register_dfs_bclk_func(dfs_bclk_func func) {
    return 0;
}
EXPORT_SYMBOL(bclk_get);
EXPORT_SYMBOL(bclk_put);
EXPORT_SYMBOL(register_dfs_bclk_func);
#endif

