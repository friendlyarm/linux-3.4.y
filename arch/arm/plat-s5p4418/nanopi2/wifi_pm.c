/*
 * sdio wifi power management API
 */
#include <linux/blkdev.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/ratelimit.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

int wifi_pm_gpio_ctrl(char* name, int level)
{
	printk("wifi_pm: %s: %d\n", name, level);

	nxp_soc_gpio_set_out_value(CFG_WIFI_POWER_IO, level);
	msleep(50);

	return 0;
}
EXPORT_SYMBOL(wifi_pm_gpio_ctrl);

int get_host_wake_irq(void)
{
	return PB_PIO_IRQ(CFG_WL_HOST_WAKE);
}
EXPORT_SYMBOL(get_host_wake_irq);
