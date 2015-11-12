/*
 * Copyright (C) 2013 Nexell Co.Ltd
 * Author: bongKwan Kook <kook@nexell.co.kr>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <mach/usb-phy.h>

#include <mach/platform.h>
#include <mach/iomap.h>
#include <mach/s5p4418.h>
#include <mach/s5p4418_irq.h>
#include <mach/soc.h>
#include <nx_tieoff.h>
#include <nx_gpio.h>

#define SOC_PA_RSTCON		PHY_BASEADDR_RSTCON
#define	SOC_VA_RSTCON		IO_ADDRESS(SOC_PA_RSTCON)

#define SOC_PA_TIEOFF		PHY_BASEADDR_TIEOFF
#define	SOC_VA_TIEOFF		IO_ADDRESS(SOC_PA_TIEOFF)

#define USB2_HOST_CLKGEN     IO_ADDRESS(PHY_BASEADDR_CLKGEN32)

#define HOST_SS_BUS_WIDTH16			(1)
#define HOST_SS_ENA_INCR16			(0x1 << 25)
#define HOST_SS_ENA_INCR8			(0x1 << 26)
#define HOST_SS_ENA_INCR4			(0x1 << 27)
#define HOST_SS_ENA_INCRX_ALIGN		(0x1 << 28)
#define HOST_SS_DMA_BURST_MASK	\
	(HOST_SS_ENA_INCR16 | HOST_SS_ENA_INCR8 |	\
	 HOST_SS_ENA_INCR4 | HOST_SS_ENA_INCRX_ALIGN)

#define EHCI_SS_ENABLE_DMA_BURST	\
	(HOST_SS_ENA_INCR16 | HOST_SS_ENA_INCR8 |	\
	 HOST_SS_ENA_INCR4 | HOST_SS_ENA_INCRX_ALIGN)
#define OHCI_SS_ENABLE_DMA_BURST	\
	(HOST_SS_ENA_INCR4 | HOST_SS_ENA_INCRX_ALIGN)


int nxp_usb_phy_init(struct platform_device *pdev, int type)
{
	PM_DBGOUT("++ %s\n", __func__);

	if (!pdev)
		return -EINVAL;

	if( type == NXP_USB_PHY_OTG )
	{
		u32 temp;

		// 1. Release otg common reset
		writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<25), SOC_VA_RSTCON + 0x04);     // reset on
		udelay(1);
		writel(readl(SOC_VA_RSTCON + 0x04) |  (1<<25), SOC_VA_RSTCON + 0x04);     // reset off

		// 2. Program scale mode to real mode
		writel(readl(SOC_VA_TIEOFF + 0x30) & ~(3<<0), SOC_VA_TIEOFF + 0x30);

		// 3. Select word interface and enable word interface selection
#if (HOST_SS_BUS_WIDTH16 == 1)
		writel(readl(SOC_VA_TIEOFF + 0x38) |  (3<<8), SOC_VA_TIEOFF + 0x38);        // 2'b01 8bit, 2'b11 16bit word
#else
		writel(readl(SOC_VA_TIEOFF + 0x38) & ~(3<<8), SOC_VA_TIEOFF + 0x38);
#endif

		// 4. Select VBUS
		temp    = readl(SOC_VA_TIEOFF + 0x34);
		temp   &= ~(3<<24);     /* Analog 5V */
//		temp   |=  (3<<24);     /* Digital 3.3V */
		writel(temp, SOC_VA_TIEOFF + 0x34);

		// 5. POR of PHY
		temp   &= ~(3<<7);
		temp   |=  (1<<7);
		writel(temp, SOC_VA_TIEOFF + 0x34);
#if 1
		udelay(1);
		writel(readl(SOC_VA_TIEOFF + 0x34) |  (3<<7), SOC_VA_TIEOFF + 0x34);
#endif
		udelay(10); // 40us delay need.

		// 6. UTMI reset
		temp   |=  (1<<3);
		writel(temp, SOC_VA_TIEOFF + 0x34);
		udelay(1); // 10 clock need

		// 7. AHB reset
		temp   |=  (1<<2);
		writel(temp, SOC_VA_TIEOFF + 0x34);
		udelay(1); // 10 clock need
	}
	else
	{
		u32 fladj_val, bit_num, bit_pos = 21;	// fladj_val0
		u32 temp1, temp2, temp3;

		// 0. Set FLADJ Register.
		fladj_val = 0x20;
#if 0
		temp1 = readl(SOC_VA_TIEOFF + 0x1C) & ~(0x1FFFFFF);
		temp2 = temp1 | fladj_val;
#else
		temp2 = fladj_val;
#endif
		for (bit_num = 0; bit_num < 6; bit_num++)
		{
			if (fladj_val & (1<<bit_num))
				temp2 |= (0x7 << bit_pos);

			bit_pos -= 3;
		}
		writel(temp2, SOC_VA_TIEOFF + 0x1C);

		// 1. Release common reset of host controller
		writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<24), SOC_VA_RSTCON + 0x04);			// reset on
		udelay(1);

		if (type == NXP_USB_PHY_HSIC) {
#if defined (CFG_GPIO_HSIC_EXTHUB_RESET)
		    // GPIO Reset
			nxp_soc_gpio_set_io_dir(CFG_GPIO_HSIC_EXTHUB_RESET, CTRUE);
		    nxp_soc_gpio_set_io_pull_enb(CFG_GPIO_HSIC_EXTHUB_RESET, CTRUE);
		    nxp_soc_gpio_set_out_value(CFG_GPIO_HSIC_EXTHUB_RESET, CTRUE);
			udelay( 10 );
		    nxp_soc_gpio_set_out_value(CFG_GPIO_HSIC_EXTHUB_RESET, CFALSE);
		    udelay( 10 );
		    nxp_soc_gpio_set_out_value(CFG_GPIO_HSIC_EXTHUB_RESET, CTRUE);
#else
			printk("is there extern hub on hsic port???\n");
#endif
		}

		writel(readl(SOC_VA_RSTCON + 0x04) |  (1<<24), SOC_VA_RSTCON + 0x04);			// reset off

		if (type == NXP_USB_PHY_HSIC) {
		    // HSIC 12M rerference Clock setting
		    writel( 0x02, USB2_HOST_CLKGEN);
		    writel( 0x0C, USB2_HOST_CLKGEN + 0x4); // 8 : ok, c : no
		    writel( 0x10, USB2_HOST_CLKGEN + 0xc);
		    writel( 0x30, USB2_HOST_CLKGEN + 0xc);
		    writel( 0x06, USB2_HOST_CLKGEN);

		    // HSIC 480M clock setting
			writel(readl(SOC_VA_TIEOFF + 0x14) & ~(3<<23), SOC_VA_TIEOFF + 0x14);
			writel(readl(SOC_VA_TIEOFF + 0x14) | (2<<23), SOC_VA_TIEOFF + 0x14);

		    // HSIC Enable in PORT1 of LINK
			writel(readl(SOC_VA_TIEOFF + 0x14) & ~(7<<14), SOC_VA_TIEOFF + 0x14);
			writel(readl(SOC_VA_TIEOFF + 0x14) | (2<<14), SOC_VA_TIEOFF + 0x14);
		}

		// 2. Program AHB Burst type
		temp1 = readl(SOC_VA_TIEOFF + 0x1C) & ~HOST_SS_DMA_BURST_MASK;
		if (type == NXP_USB_PHY_OHCI)
			writel(temp1 | OHCI_SS_ENABLE_DMA_BURST, SOC_VA_TIEOFF + 0x1C);
		else
			writel(temp1 | EHCI_SS_ENABLE_DMA_BURST, SOC_VA_TIEOFF + 0x1C);

		// 3. Select word interface and enable word interface selection
		temp1 = readl(SOC_VA_TIEOFF + 0x14) | (3<<25);	// 2'b01 8bit, 2'b11 16bit word
		temp2 = readl(SOC_VA_TIEOFF + 0x24) | (3<<8);	// 2'b01 8bit, 2'b11 16bit word
		temp3 = readl(SOC_VA_TIEOFF + 0x2C) | (3<<12);	// 2'b01 8bit, 2'b11 16bit word
#if (HOST_SS_BUS_WIDTH16 == 0)
		temp1 &= ~(2<<25);
		temp2 &= ~(2<<8);
		temp3 &= ~(2<<12);
#endif

		writel(temp1, SOC_VA_TIEOFF + 0x14);
		writel(temp2, SOC_VA_TIEOFF + 0x24);
		if (type == NXP_USB_PHY_HSIC)
			writel(temp3, SOC_VA_TIEOFF + 0x2C);

		// 4. POR of PHY
		temp1   = readl(SOC_VA_TIEOFF + 0x20);
		temp1  &= ~(3<<7);
		temp1  |=  (1<<7);
		writel(temp1, SOC_VA_TIEOFF + 0x20);
		udelay(10); // 40us delay need.

		if (type == NXP_USB_PHY_HSIC) {
			// Set HSIC mode
			writel(readl(SOC_VA_TIEOFF + 0x14) |  (3<<23), SOC_VA_TIEOFF + 0x14);

			// POR of HSIC PHY
			writel(readl(SOC_VA_TIEOFF + 0x28) & ~(3<<18), SOC_VA_TIEOFF + 0x28);
			writel(readl(SOC_VA_TIEOFF + 0x28) |  (1<<18), SOC_VA_TIEOFF + 0x28);

			// Wait clock of PHY - about 40 micro seconds
			udelay(100); // 40us delay need.
		}

		// 5. Release utmi reset
		temp1 = readl(SOC_VA_TIEOFF + 0x14) | (7<<20);
		if (type == NXP_USB_PHY_HSIC)
			writel(temp1, SOC_VA_TIEOFF + 0x14);
		else
			writel(temp1 & ~(4<<20), SOC_VA_TIEOFF + 0x14);

		//6. Release ahb reset of EHCI, OHCI
		//writel(readl(SOC_VA_TIEOFF + 0x14) & ~(7<<17), SOC_VA_TIEOFF + 0x14);
		writel(readl(SOC_VA_TIEOFF + 0x14) |  (7<<17), SOC_VA_TIEOFF + 0x14);
	}

	PM_DBGOUT("-- %s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(nxp_usb_phy_init);

int nxp_usb_phy_exit(struct platform_device *pdev, int type)
{
	u32 temp;

	PM_DBGOUT("++ %s\n", __func__);

	if (!pdev)
		return -EINVAL;

	if( type == NXP_USB_PHY_OTG )
	{
		temp    = readl(SOC_VA_TIEOFF + 0x34);

		// 0. Select VBUS
		temp   |=  (3<<24);   /* Select VBUS 3.3V */
//		temp   &= ~(3<<24);   /* Select VBUS 5V */
		writel(temp, SOC_VA_TIEOFF + 0x34);

		// 1. UTMI reset
		temp   &= ~(1<<3);
		writel(temp, SOC_VA_TIEOFF + 0x34);

		// 2. AHB reset
		temp   &= ~(1<<2);
		writel(temp, SOC_VA_TIEOFF + 0x34);

		// 3. POR of PHY
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

		// OTG reset on
		writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<25), SOC_VA_RSTCON + 0x04);
	}
	else
	{
#if 0
		// EHCI, OHCI reset on
		writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<24), SOC_VA_RSTCON + 0x04);
#else
		// EHCI, OHCI reset on
		writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<24), SOC_VA_RSTCON + 0x04);

		// 6. Release ahb reset of EHCI, OHCI
		writel(readl(SOC_VA_TIEOFF + 0x14) & ~(7<<17), SOC_VA_TIEOFF + 0x14);

		// 5. Release utmi reset
		writel(readl(SOC_VA_TIEOFF + 0x14) & ~(7<<20), SOC_VA_TIEOFF + 0x14);

		// 4. POR of PHY
#if 0
		writel(readl(SOC_VA_TIEOFF + 0x20) | (3<<7), SOC_VA_TIEOFF + 0x20);
#else
		temp    = readl(SOC_VA_TIEOFF + 0x20);
		temp   &= ~(3<<7);
		temp   |=  (1<<7);
		writel(temp, SOC_VA_TIEOFF + 0x20);
		udelay(1);
		temp   |=  (3<<7);
		writel(temp, SOC_VA_TIEOFF + 0x20);
		udelay(1);
		temp   &= ~(2<<7);
		writel(temp, SOC_VA_TIEOFF + 0x20);
#endif
		if (type == NXP_USB_PHY_HSIC) {
			// Clear HSIC mode
			writel(readl(SOC_VA_TIEOFF + 0x14) & ~(3<<23), SOC_VA_TIEOFF + 0x14);

			// POR of HSIC PHY
			writel(readl(SOC_VA_TIEOFF + 0x28) |  (3<<18), SOC_VA_TIEOFF + 0x28);
		}

		// Wait clock of PHY - about 40 micro seconds
		udelay(10); // 40us delay need.

		// EHCI, OHCI reset on
//		writel(readl(SOC_VA_RSTCON + 0x04) & ~(1<<24), SOC_VA_RSTCON + 0x04);
#endif
	}

	PM_DBGOUT("-- %s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(nxp_usb_phy_exit);
