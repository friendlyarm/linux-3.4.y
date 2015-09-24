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

/*
 * DW MMC (Synopsys DesignWare Memory Card Interface)
 */
#if defined(CONFIG_MMC_DW)

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>


static u64 dwmci_dmamask = DMA_BIT_MASK(32);

static int __dwmci_init(u32 slot_id, irq_handler_t handler, void *data)
{
	return 0;
}

static int __dwmci_get_bus_wd(u32 slot_id)
{
	return 4;
}

static void __dwmci_set_io_timing(void *data, unsigned char timing)
{
}

static int __dwmci_get_ocr(u32 slot_id)
{
	u32 ocr_avail = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	return ocr_avail;
}

static int __dwmci_initialize(int ch, ulong rate)
{
	struct clk *clk = NULL;
	int reset = RESET_ID_SDMMC0 + ch;
	char s[20];

	nxp_soc_peri_reset_set(reset);

	sprintf(s, "%s.%d", DEV_NAME_SDHC, ch);
	clk = clk_get(NULL, s);
	clk_set_rate(clk, rate);
	clk_enable(clk);
	return 0;
}

static void __dwmci_suspend(struct dw_mci *host)
{
	struct clk *clk = NULL;
	int ch = host->dev.id;
	char s[20];

	sprintf(s, "%s.%d", DEV_NAME_SDHC, ch);
	clk = clk_get(NULL, s);
	clk_disable(clk);
}

static void __dwmci_resume(struct dw_mci *host)
{
	struct device *dev = &host->dev;
	struct dw_mci_board *brd = host->pdata;

	PM_DBGOUT("%s: dw_mmc.%d, %d\n", __func__, dev->id, brd->bus_hz);
	__dwmci_initialize(dev->id, brd->bus_hz);

	if (brd->late_resume)
		brd->late_resume(host);
}

#define DW_MCI_PLAT_RES(_ch)		\
static struct resource dwmci##_ch##_resource[] = {		\
	[0] = DEFINE_RES_MEM(PHY_BASEADDR_SDMMC##_ch, SZ_4K),	\
	[1] = DEFINE_RES_IRQ(IRQ_PHY_SDMMC##_ch),				\
}

#define DW_MCI_PLAT_DAT(_ch)		\
static struct dw_mci_board dwmci##_ch##_data = {		\
	.num_slots		= 1,									\
	.bus_hz			= 100 * 1000 * 1000,					\
	.max_bus_hz		= 200 * 1000 * 1000,					\
	.fifo_depth		= 0x20,									\
	.detect_delay_ms= 200,									\
	.init			= __dwmci_init,							\
	.get_bus_wd		= __dwmci_get_bus_wd,					\
	.set_io_timing	= __dwmci_set_io_timing,				\
	.get_ocr		= __dwmci_get_ocr,						\
	.cd_type		= DW_MCI_CD_PERMANENT,					\
	.suspend		= __dwmci_suspend,						\
	.resume			= __dwmci_resume,						\
	.clk_dly		= 0,									\
	.mode           = DMA_MODE,                             \
}

#define DW_MCI_PLAT_DEV(_ch)	\
/*static*/ struct platform_device dwmci_dev_ch##_ch = {		\
	.name			= "dw_mmc",								\
	.id				= _ch,									\
	.num_resources	= ARRAY_SIZE(dwmci##_ch##_resource),	\
	.resource		= dwmci##_ch##_resource,				\
	.dev			= {										\
	.id					= _ch,								\
	.dma_mask			= &dwmci_dmamask,					\
	.coherent_dma_mask	= DMA_BIT_MASK(32),					\
	.platform_data		= &dwmci##_ch##_data,				\
	},														\
}

#define	DW_MCI_ITEM_CHECK(src, dst, item)	\
	if(src->item && src->item != dst->item)		\
		dst->item = src->item;

#ifdef CONFIG_MMC_NXP_CH0
DW_MCI_PLAT_RES(0);
DW_MCI_PLAT_DAT(0);
DW_MCI_PLAT_DEV(0);
#endif

#ifdef CONFIG_MMC_NXP_CH1
DW_MCI_PLAT_RES(1);
DW_MCI_PLAT_DAT(1);
DW_MCI_PLAT_DEV(1);
#endif

#ifdef CONFIG_MMC_NXP_CH2
DW_MCI_PLAT_RES(2);
DW_MCI_PLAT_DAT(2);
DW_MCI_PLAT_DEV(2);
#endif

static struct platform_device *dwmci_devices[] = {
#if defined(CONFIG_MMC_NXP_CH0)
	&dwmci_dev_ch0,
#endif
#if defined(CONFIG_MMC_NXP_CH1)
	&dwmci_dev_ch1,
#endif
#if defined(CONFIG_MMC_NXP_CH2)
	&dwmci_dev_ch2,
#endif
};

void __init nxp_mmc_add_device(int ch, struct dw_mci_board *mci)
{
	struct platform_device **dev = dwmci_devices;
	struct dw_mci_board *dst = NULL;
	int id = 0;

	for (id = 0; ARRAY_SIZE(dwmci_devices); id++) {
		if (dev[id]->id == ch) {
			dst = dev[id]->dev.platform_data;
			break;
		}
	}

	if (NULL == dst) {
		pr_err("%s: mmc ch %d is not support...\n", __func__, ch);
		return;
	}

	DW_MCI_ITEM_CHECK(mci, dst, num_slots);
	DW_MCI_ITEM_CHECK(mci, dst, quirks);
	DW_MCI_ITEM_CHECK(mci, dst, bus_hz);
	DW_MCI_ITEM_CHECK(mci, dst, max_bus_hz);
	DW_MCI_ITEM_CHECK(mci, dst, caps);
	DW_MCI_ITEM_CHECK(mci, dst, caps2);
	DW_MCI_ITEM_CHECK(mci, dst, pm_caps);
	DW_MCI_ITEM_CHECK(mci, dst, fifo_depth);
	DW_MCI_ITEM_CHECK(mci, dst, detect_delay_ms);
	DW_MCI_ITEM_CHECK(mci, dst, init);
	DW_MCI_ITEM_CHECK(mci, dst, get_ro);
	DW_MCI_ITEM_CHECK(mci, dst, get_cd);
	DW_MCI_ITEM_CHECK(mci, dst, get_ocr);
	DW_MCI_ITEM_CHECK(mci, dst, get_bus_wd);
	DW_MCI_ITEM_CHECK(mci, dst, cfg_gpio);
	DW_MCI_ITEM_CHECK(mci, dst, hw_reset);
	DW_MCI_ITEM_CHECK(mci, dst, set_io_timing);
	DW_MCI_ITEM_CHECK(mci, dst, sdr_timing);
	DW_MCI_ITEM_CHECK(mci, dst, ddr_timing);
	DW_MCI_ITEM_CHECK(mci, dst, clk_drv);
	DW_MCI_ITEM_CHECK(mci, dst, clk_smpl);
	DW_MCI_ITEM_CHECK(mci, dst, tuned);
	DW_MCI_ITEM_CHECK(mci, dst, cd_type);
	DW_MCI_ITEM_CHECK(mci, dst, ext_cd_init);
	DW_MCI_ITEM_CHECK(mci, dst, ext_cd_cleanup);
	DW_MCI_ITEM_CHECK(mci, dst, setpower);
	DW_MCI_ITEM_CHECK(mci, dst, exit);
	DW_MCI_ITEM_CHECK(mci, dst, select_slot);
	DW_MCI_ITEM_CHECK(mci, dst, dma_ops);
	DW_MCI_ITEM_CHECK(mci, dst, data);
	DW_MCI_ITEM_CHECK(mci, dst, blk_settings);
	DW_MCI_ITEM_CHECK(mci, dst, clk_dly);
	DW_MCI_ITEM_CHECK(mci, dst, suspend);
	DW_MCI_ITEM_CHECK(mci, dst, resume);
	DW_MCI_ITEM_CHECK(mci, dst, late_resume);
	DW_MCI_ITEM_CHECK(mci, dst, hs_over_clk);
	DW_MCI_ITEM_CHECK(mci, dst, hw_timeout);
	DW_MCI_ITEM_CHECK(mci, dst,mode);//bok add

	printk("plat: add device sdmmc [%d]\n", ch);
    platform_device_register(dwmci_devices[id]);

    /* rese and clock */
   	__dwmci_initialize(ch, dst->bus_hz);
}

#endif /* CONFIG_MMC_DW */
