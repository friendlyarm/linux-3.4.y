/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <mach/regs-pmu.h>
#include <mach/regs-usb-phy.h>
#include <mach/regs-usb3-exynos-drd-phy.h>
#include <plat/cpu.h>
#include <plat/usb-phy.h>
#include <plat/udc-hs.h>

#define EXYNOS5_USB_CFG	(S3C_VA_SYS + 0x230)
#define PHY_ENABLE	(1 << 0)
#define PHY_DISABLE	(0 << 0)

enum usb_phy_type {
	USB_PHY		= (0x1 << 0),
	USB_PHY0	= (0x1 << 0),
	USB_PHY1	= (0x1 << 1),
	USB_PHY_HSIC0	= (0x1 << 1),
	USB_PHY_HSIC1	= (0x1 << 2),
};

struct exynos_usb_phy {
	struct clk *phy_clk;
	struct mutex phy_lock;
	u32 flags;
};

static struct exynos_usb_phy usb_phy_control = {
	.phy_lock = __MUTEX_INITIALIZER(usb_phy_control.phy_lock)
};

static atomic_t host_usage;

static void exynos_usb_mux_change(struct platform_device *pdev, int val)
{
	u32 is_host;

	/*
	 * Exynos5250 has a USB 2.0 PHY for host and device.
	 * So, host and device cannot be used simultaneously except HSIC.
	 * USB mode can be changed by USB_CFG register.
	 * USB_CFG 1:host mode, 0:device mode.
	 */
	is_host = readl(EXYNOS5_USB_CFG);
	writel(val, EXYNOS5_USB_CFG);

	if (is_host != val)
		dev_dbg(&pdev->dev, "Change USB MUX from %s to %s",
			is_host ? "Host" : "Device",
			val ? "Host" : "Device");
}

static int exynos_usb_phy_clock_enable(struct platform_device *pdev)
{
	struct clk *clk;

	if (!usb_phy_control.phy_clk) {
		/*
		 * PHY clock domain is 'usbhost' on exynos5250.
		 * But, PHY clock domain is 'otg' on others.
		 */
		if (soc_is_exynos5250())
			clk = clk_get(&pdev->dev, "usbhost");
		else
			clk = clk_get(&pdev->dev, "otg");

		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "Failed to get phy clock\n");
			return PTR_ERR(clk);
		} else
			usb_phy_control.phy_clk = clk;

	}

	return clk_enable(usb_phy_control.phy_clk);
}

static int exynos_usb_phy_clock_disable(struct platform_device *pdev)
{
	struct clk *clk;

	if (!usb_phy_control.phy_clk) {
		if (soc_is_exynos5250())
			clk = clk_get(&pdev->dev, "usbhost");
		else
			clk = clk_get(&pdev->dev, "otg");
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "Failed to get phy clock\n");
			return PTR_ERR(clk);
		} else
			usb_phy_control.phy_clk = clk;
	}

	clk_disable(usb_phy_control.phy_clk);

	return 0;
}

static u32 exynos_usb_phy_set_clock(struct platform_device *pdev)
{
	struct clk *ref_clk;
	u32 refclk_freq = 0;

	ref_clk = clk_get(&pdev->dev, "ext_xtal");

	if (IS_ERR(ref_clk)) {
		dev_err(&pdev->dev, "Failed to get reference clock\n");
		return PTR_ERR(ref_clk);
	}

	switch (clk_get_rate(ref_clk)) {
	case 96 * 100000:
		refclk_freq = EXYNOS5_CLKSEL_9600K;
		break;
	case 10 * MHZ:
		refclk_freq = EXYNOS5_CLKSEL_10M;
		break;
	case 12 * MHZ:
		refclk_freq = EXYNOS5_CLKSEL_12M;
		break;
	case 192 * 100000:
		refclk_freq = EXYNOS5_CLKSEL_19200K;
		break;
	case 20 * MHZ:
		refclk_freq = EXYNOS5_CLKSEL_20M;
		break;
	case 50 * MHZ:
		refclk_freq = EXYNOS5_CLKSEL_50M;
		break;
	case 24 * MHZ:
	default:
		/* default reference clock */
		refclk_freq = EXYNOS5_CLKSEL_24M;
		break;
	}
	clk_put(ref_clk);

	return refclk_freq;
}

static int exynos4_usb_host_phy_is_on(void)
{
	return (readl(EXYNOS4_PHYPWR) & PHY1_STD_ANALOG_POWERDOWN) ? 0 : 1;
}

static int exynos5_usb_host_phy20_is_on(void)
{
	return (readl(EXYNOS5_PHY_HOST_CTRL0) & HOST_CTRL0_PHYSWRSTALL) ? 0 : 1;
}

static void exynos_usb_phy_control(enum usb_phy_type phy_type , int on)
{
	if (phy_type & USB_PHY0)
		writel(on, EXYNOS5_USBDEV_PHY_CONTROL);
	if (phy_type & USB_PHY1)
		writel(on, EXYNOS5_USBHOST_PHY_CONTROL);
}

static int exynos4_usb_phy1_init(struct platform_device *pdev)
{
	struct clk *otg_clk;
	struct clk *xusbxti_clk;
	u32 phyclk;
	u32 rstcon;
	int err;

	atomic_inc(&host_usage);

	otg_clk = clk_get(&pdev->dev, "otg");
	if (IS_ERR(otg_clk)) {
		dev_err(&pdev->dev, "Failed to get otg clock\n");
		return PTR_ERR(otg_clk);
	}

	err = clk_enable(otg_clk);
	if (err) {
		clk_put(otg_clk);
		return err;
	}

	if (exynos4_usb_host_phy_is_on())
		return 0;

	writel(readl(EXYNOS4210_USBHOST_PHY_CONTROL) |
		     EXYNOS4210_USBHOST_PHY_ENABLE,
		     EXYNOS4210_USBHOST_PHY_CONTROL);

	/* set clock frequency for PLL */
	phyclk = readl(EXYNOS4_PHYCLK) & ~CLKSEL_MASK;

	xusbxti_clk = clk_get(&pdev->dev, "xusbxti");
	if (xusbxti_clk && !IS_ERR(xusbxti_clk)) {
		switch (clk_get_rate(xusbxti_clk)) {
		case 12 * MHZ:
			phyclk |= CLKSEL_12M;
			break;
		case 24 * MHZ:
			phyclk |= CLKSEL_24M;
			break;
		default:
		case 48 * MHZ:
			/* default reference clock */
			break;
		}
		clk_put(xusbxti_clk);
	}

	writel(phyclk, EXYNOS4_PHYCLK);

	/* floating prevention logic: disable */
	writel((readl(EXYNOS4_PHY1CON) | FPENABLEN), EXYNOS4_PHY1CON);

	/* set to normal HSIC 0 and 1 of PHY1 */
	writel((readl(EXYNOS4_PHYPWR) & ~PHY1_HSIC_NORMAL_MASK),
			EXYNOS4_PHYPWR);

	/* set to normal standard USB of PHY1 */
	writel((readl(EXYNOS4_PHYPWR) & ~PHY1_STD_NORMAL_MASK), EXYNOS4_PHYPWR);

	/* reset all ports of both PHY and Link */
	rstcon = readl(EXYNOS4_RSTCON) | HOST_LINK_PORT_SWRST_MASK |
		PHY1_SWRST_MASK;
	writel(rstcon, EXYNOS4_RSTCON);
	udelay(10);

	rstcon &= ~(HOST_LINK_PORT_SWRST_MASK | PHY1_SWRST_MASK);
	writel(rstcon, EXYNOS4_RSTCON);
	udelay(80);

	clk_disable(otg_clk);
	clk_put(otg_clk);

	return 0;
}

static int exynos4_usb_phy1_exit(struct platform_device *pdev)
{
	struct clk *otg_clk;
	int err;

	if (atomic_dec_return(&host_usage) > 0)
		return 0;

	otg_clk = clk_get(&pdev->dev, "otg");
	if (IS_ERR(otg_clk)) {
		dev_err(&pdev->dev, "Failed to get otg clock\n");
		return PTR_ERR(otg_clk);
	}

	err = clk_enable(otg_clk);
	if (err) {
		clk_put(otg_clk);
		return err;
	}

	writel((readl(EXYNOS4_PHYPWR) | PHY1_STD_ANALOG_POWERDOWN),
			EXYNOS4_PHYPWR);

	writel(readl(EXYNOS4210_USBHOST_PHY_CONTROL) &
		    ~EXYNOS4210_USBHOST_PHY_ENABLE,
		     EXYNOS4210_USBHOST_PHY_CONTROL);

	clk_disable(otg_clk);
	clk_put(otg_clk);

	return 0;
}

static int exynos5_usb_phy20_init(struct platform_device *pdev)
{
	u32 refclk_freq;
	u32 hostphy_ctrl0, otgphy_sys, hsic_ctrl, ehcictrl;

	atomic_inc(&host_usage);

	if (exynos5_usb_host_phy20_is_on()) {
		dev_err(&pdev->dev, "Already power on PHY\n");
		return 0;
	}

	exynos_usb_phy_control(USB_PHY1, PHY_ENABLE);

	/* Host and Device should be set at the same time */
	hostphy_ctrl0 = readl(EXYNOS5_PHY_HOST_CTRL0);
	hostphy_ctrl0 &= ~(HOST_CTRL0_FSEL_MASK);
	otgphy_sys = readl(EXYNOS5_PHY_OTG_SYS);
	otgphy_sys &= ~(OTG_SYS_CTRL0_FSEL_MASK);

	/* 2.0 phy reference clock configuration */
	refclk_freq = exynos_usb_phy_set_clock(pdev);
	hostphy_ctrl0 |= (refclk_freq << HOST_CTRL0_CLKSEL_SHIFT);
	otgphy_sys |= (refclk_freq << OTG_SYS_CLKSEL_SHIFT);

	/* COMMON Block configuration during suspend */
	hostphy_ctrl0 &= ~(HOST_CTRL0_COMMONON_N);
	otgphy_sys |= (OTG_SYS_COMMON_ON);

	/* otg phy reset */
	otgphy_sys &= ~(OTG_SYS_FORCE_SUSPEND |
			OTG_SYS_SIDDQ_UOTG |
			OTG_SYS_FORCE_SLEEP);
	otgphy_sys &= ~(OTG_SYS_REF_CLK_SEL_MASK);
	otgphy_sys |= (OTG_SYS_REF_CLK_SEL(0x2) | OTG_SYS_OTGDISABLE);
	otgphy_sys |= (OTG_SYS_PHY0_SW_RST |
			OTG_SYS_LINK_SW_RST_UOTG |
			OTG_SYS_PHYLINK_SW_RESET);
	writel(otgphy_sys, EXYNOS5_PHY_OTG_SYS);
	udelay(10);
	otgphy_sys &= ~(OTG_SYS_PHY0_SW_RST |
			OTG_SYS_LINK_SW_RST_UOTG |
			OTG_SYS_PHYLINK_SW_RESET);
	writel(otgphy_sys, EXYNOS5_PHY_OTG_SYS);

	/* host phy reset */
	hostphy_ctrl0 &= ~(HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL |
			HOST_CTRL0_SIDDQ);
	hostphy_ctrl0 &= ~(HOST_CTRL0_FORCESUSPEND | HOST_CTRL0_FORCESLEEP);
	hostphy_ctrl0 |= (HOST_CTRL0_LINKSWRST | HOST_CTRL0_UTMISWRST);
	writel(hostphy_ctrl0, EXYNOS5_PHY_HOST_CTRL0);
	udelay(10);
	hostphy_ctrl0 &= ~(HOST_CTRL0_LINKSWRST | HOST_CTRL0_UTMISWRST);
	writel(hostphy_ctrl0, EXYNOS5_PHY_HOST_CTRL0);

	/* HSIC phy reset */
	hsic_ctrl = (HSIC_CTRL_REFCLKDIV(0x24) | HSIC_CTRL_REFCLKSEL(0x2) |
		HSIC_CTRL_PHYSWRST);
	writel(hsic_ctrl, EXYNOS5_PHY_HSIC_CTRL1);
	writel(hsic_ctrl, EXYNOS5_PHY_HSIC_CTRL2);
	udelay(10);
	hsic_ctrl &= ~(HSIC_CTRL_PHYSWRST);
	writel(hsic_ctrl, EXYNOS5_PHY_HSIC_CTRL1);
	writel(hsic_ctrl, EXYNOS5_PHY_HSIC_CTRL2);

	udelay(80);

	/* Enable DMA burst bus configuration */
	ehcictrl = readl(EXYNOS5_PHY_HOST_EHCICTRL);
	ehcictrl |= (EHCICTRL_ENAINCRXALIGN | EHCICTRL_ENAINCR4 |
			EHCICTRL_ENAINCR8 | EHCICTRL_ENAINCR16);
	writel(ehcictrl, EXYNOS5_PHY_HOST_EHCICTRL);

	return 0;
}

static int exynos5_usb_phy20_exit(struct platform_device *pdev)
{
	u32 hostphy_ctrl0, otgphy_sys, hsic_ctrl;

	if (atomic_dec_return(&host_usage) > 0) {
		dev_info(&pdev->dev, "still being used\n");
		return -EBUSY;
	}

	hsic_ctrl = (HSIC_CTRL_REFCLKDIV(0x24) |
			HSIC_CTRL_REFCLKSEL(0x2) |
			HSIC_CTRL_SIDDQ |
			HSIC_CTRL_FORCESLEEP |
			HSIC_CTRL_FORCESUSPEND);
	writel(hsic_ctrl, EXYNOS5_PHY_HSIC_CTRL1);
	writel(hsic_ctrl, EXYNOS5_PHY_HSIC_CTRL2);

	hostphy_ctrl0 = readl(EXYNOS5_PHY_HOST_CTRL0);
	hostphy_ctrl0 |= (HOST_CTRL0_SIDDQ);
	hostphy_ctrl0 |= (HOST_CTRL0_FORCESUSPEND | HOST_CTRL0_FORCESLEEP);
	hostphy_ctrl0 |= (HOST_CTRL0_PHYSWRST | HOST_CTRL0_PHYSWRSTALL);
	writel(hostphy_ctrl0, EXYNOS5_PHY_HOST_CTRL0);

	otgphy_sys = readl(EXYNOS5_PHY_OTG_SYS);

	/* Issue a OTG_SYS_PHYLINK_SW_RESET to release pulldowns on D+/D- */
	writel(otgphy_sys | OTG_SYS_PHYLINK_SW_RESET, EXYNOS5_PHY_OTG_SYS);
	udelay(10);
	writel(otgphy_sys, EXYNOS5_PHY_OTG_SYS);

	otgphy_sys |= (OTG_SYS_FORCE_SUSPEND |
			OTG_SYS_SIDDQ_UOTG |
			OTG_SYS_FORCE_SLEEP);
	writel(otgphy_sys, EXYNOS5_PHY_OTG_SYS);

	exynos_usb_phy_control(USB_PHY1, PHY_DISABLE);

	return 0;
}

static int s5p_usb_otg_phy_tune(struct s3c_hsotg_plat *pdata, int def_mode)
{
	u32 phytune;

	if (!pdata)
		return -EINVAL;

	pr_debug("usb: %s read original tune\n", __func__);
	phytune = readl(EXYNOS5_PHY_OTG_TUNE);
	if (!pdata->def_phytune) {
		pdata->def_phytune = phytune;
		pr_debug("usb: %s save default phytune (0x%x)\n",
				__func__, pdata->def_phytune);
	}

	pr_debug("usb: %s original tune=0x%x\n",
			__func__, phytune);

	pr_debug("usb: %s tune_mask=0x%x, tune=0x%x\n",
			__func__, pdata->phy_tune_mask, pdata->phy_tune);

	if (pdata->phy_tune_mask) {
		if (def_mode) {
			pr_debug("usb: %s set defult tune=0x%x\n",
					__func__, pdata->def_phytune);
			writel(pdata->def_phytune, EXYNOS5_PHY_OTG_TUNE);
		} else {
			phytune &= ~(pdata->phy_tune_mask);
			phytune |= pdata->phy_tune;
			udelay(10);
			pr_debug("usb: %s custom tune=0x%x\n",
					__func__, phytune);
			writel(phytune, EXYNOS5_PHY_OTG_TUNE);
		}
		phytune = readl(EXYNOS5_PHY_OTG_TUNE);
		pr_debug("usb: %s modified tune=0x%x\n",
				__func__, phytune);
	} else {
		pr_debug("usb: %s default tune\n", __func__);
	}

	return 0;
}

static void set_exynos5_usb_host_phy_tune(void)
{
	u32 phytune;

	phytune = readl(EXYNOS5_PHY_HOST_TUNE0);
	pr_debug("usb: %s old phy tune for host =0x%x\n",
			__func__, phytune);

	/* sqrxtune [14:12] 3b110 : -15% */
	phytune &= ~HOST_TUNE0_SQRXTUNE(0x7);
	phytune |= HOST_TUNE0_SQRXTUNE(0x6);
	udelay(10);
	writel(phytune, EXYNOS5_PHY_HOST_TUNE0);
	phytune = readl(EXYNOS5_PHY_HOST_TUNE0);

	pr_debug("usb: %s new phy tune for host =0x%x\n",
				__func__, phytune);
}

static void set_exynos5_usb_device_phy_tune(void)
{
	u32 phytune;

	phytune = readl(EXYNOS5_PHY_OTG_TUNE);
	pr_debug("usb: %s old phy tune for device =0x%x\n",
				__func__, phytune);

	/* sqrxtune [13:11] 3b110 : -15% */
	phytune &= ~OTG_TUNE_SQRXTUNE(0x7);
	phytune |= OTG_TUNE_SQRXTUNE(0x6);
	/* txpreempamptune [22:21] 2b10 : 2X */
	phytune &= ~OTG_TUNE_TXPREEMPAMPTUNE(0x3);
	phytune |= OTG_TUNE_TXPREEMPAMPTUNE(0x2);
	/* txvreftune [ 3: 0] 8b1000 : +10% */
	phytune &= ~OTG_TUNE_TXVREFTUNE(0xf);
	phytune |= OTG_TUNE_TXVREFTUNE(0x8);
	udelay(10);
	writel(phytune, EXYNOS5_PHY_OTG_TUNE);
	phytune = readl(EXYNOS5_PHY_OTG_TUNE);

	pr_debug("usb: %s new phy tune for device =0x%x\n",
				__func__, phytune);
}

static int exynos5_usb_host_phy20_init(struct platform_device *pdev)
{
	if (exynos_usb_phy_clock_enable(pdev))
		return -EINVAL;

	/* usb mode change from device to host */
	exynos_usb_mux_change(pdev, 1);

	exynos5_usb_phy20_init(pdev);

	/* usb host phy tune */
	set_exynos5_usb_host_phy_tune();

	return 0;
}

static int exynos5_usb_host_phy20_exit(struct platform_device *pdev)
{
	exynos5_usb_phy20_exit(pdev);

	/* usb mode change from host to device */
	exynos_usb_mux_change(pdev, 0);

	exynos_usb_phy_clock_disable(pdev);

	return 0;
}

static int exynos_usb_dev_phy20_init(struct platform_device *pdev)
{
	int ret = 0;

	if (exynos_usb_phy_clock_enable(pdev))
		return -EINVAL;

	/* usb mode change from host to device */
	exynos_usb_mux_change(pdev, 0);

	exynos5_usb_phy20_init(pdev);

	/* usb device phy tune */
	set_exynos5_usb_device_phy_tune();
	/* set custom usb device phy tune */
	if (pdev->dev.platform_data)
		ret = s5p_usb_otg_phy_tune(pdev->dev.platform_data, 0);

	exynos_usb_phy_clock_disable(pdev);

	return ret;
}

static int exynos_usb_dev_phy20_exit(struct platform_device *pdev)
{
	int ret = 0;

	if (exynos_usb_phy_clock_enable(pdev))
		return -EINVAL;

	/* set custom usb device phy tune */
	if (pdev->dev.platform_data)
		ret = s5p_usb_otg_phy_tune(pdev->dev.platform_data, 1);

	exynos5_usb_phy20_exit(pdev);

	exynos_usb_phy_clock_disable(pdev);

	return ret;
}

static int exynos5_usb_phy30_init(struct platform_device *pdev)
{
	u32 reg;

	exynos_usb_phy_control(USB_PHY0, PHY_ENABLE);

	/* Reset USB 3.0 PHY */
	writel(0x087fffc0, EXYNOS_USB3_LINKSYSTEM);
	writel(0x00000000, EXYNOS_USB3_PHYREG0);
	writel(0x24d4e6e4, EXYNOS_USB3_PHYPARAM0);
	writel(0x03fff820, EXYNOS_USB3_PHYPARAM1);
	writel(0x00000000, EXYNOS_USB3_PHYBATCHG);
	writel(0x00000000, EXYNOS_USB3_PHYRESUME);
	/* REVISIT : Over-current pin is inactive on SMDK5250 */
	if (soc_is_exynos5250())
		writel((readl(EXYNOS_USB3_LINKPORT) & ~(0x3<<4)) |
			(0x3<<2), EXYNOS_USB3_LINKPORT);

	/* UTMI Power Control */
	writel(EXYNOS_USB3_PHYUTMI_OTGDISABLE, EXYNOS_USB3_PHYUTMI);

	/* Set 100MHz external clock */
	reg = EXYNOS_USB3_PHYCLKRST_PORTRESET |
		/* HS PLL uses ref_pad_clk{p,m} or ref_alt_clk_{p,m}
		* as reference */
		EXYNOS_USB3_PHYCLKRST_REFCLKSEL(2) |
		/* Digital power supply in normal operating mode */
		EXYNOS_USB3_PHYCLKRST_RETENABLEN |
		/* 0x27-100MHz, 0x2a-24MHz, 0x31-20MHz, 0x38-19.2MHz */
		EXYNOS_USB3_PHYCLKRST_FSEL(0x27) |
		/* 0x19-100MHz, 0x68-24MHz, 0x7d-20Mhz */
		EXYNOS_USB3_PHYCLKRST_MPLL_MULTIPLIER(0x19) |
		/* Enable ref clock for SS function */
		EXYNOS_USB3_PHYCLKRST_REF_SSP_EN |
		/* Enable spread spectrum */
		EXYNOS_USB3_PHYCLKRST_SSC_EN;

	writel(reg, EXYNOS_USB3_PHYCLKRST);

	udelay(10);

	reg &= ~(EXYNOS_USB3_PHYCLKRST_PORTRESET);
	writel(reg, EXYNOS_USB3_PHYCLKRST);

	return 0;
}

static int exynos5_usb_phy30_exit(struct platform_device *pdev)
{
	u32 reg;

	reg = EXYNOS_USB3_PHYUTMI_OTGDISABLE |
		EXYNOS_USB3_PHYUTMI_FORCESUSPEND |
		EXYNOS_USB3_PHYUTMI_FORCESLEEP;
	writel(reg, EXYNOS_USB3_PHYUTMI);

	exynos_usb_phy_control(USB_PHY0, PHY_DISABLE);

	return 0;
}

int s5p_usb_phy_init(struct platform_device *pdev, int type)
{
	int ret = -EINVAL;

	if (type == S5P_USB_PHY_HOST) {
		if (soc_is_exynos5250())
			ret = exynos5_usb_host_phy20_init(pdev);
		else
			ret = exynos4_usb_phy1_init(pdev);
	} else if (type == S5P_USB_PHY_DEVICE) {
		ret = exynos_usb_dev_phy20_init(pdev);
	} else if (type == S5P_USB_PHY_DRD)
		ret = exynos5_usb_phy30_init(pdev);

	return ret;
}

int s5p_usb_phy_exit(struct platform_device *pdev, int type)
{
	int ret = -EINVAL;

	if (type == S5P_USB_PHY_HOST) {
		if (soc_is_exynos5250())
			ret = exynos5_usb_host_phy20_exit(pdev);
		else
			ret = exynos4_usb_phy1_exit(pdev);
	} else if (type == S5P_USB_PHY_DEVICE) {
		ret = exynos_usb_dev_phy20_exit(pdev);
	} else if (type == S5P_USB_PHY_DRD) {
		ret = exynos5_usb_phy30_exit(pdev);
	}

	return ret;
}
