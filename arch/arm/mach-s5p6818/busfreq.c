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

#include <mach/busfreq.h>

int bclk_get(uint32_t user)
{
    return 0;
}

int bclk_put(uint32_t user)
{
    return 0;
}

int register_dfs_bclk_func(dfs_bclk_func func)
{
    return 0;
}

EXPORT_SYMBOL(bclk_get);
EXPORT_SYMBOL(bclk_put);
EXPORT_SYMBOL(register_dfs_bclk_func);


