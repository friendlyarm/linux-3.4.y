/* linux/arch/arm/mach-exynos/setup-adc.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *	http://www.samsung.com/
 *
 * Base Samsung Exynos ADC configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/io.h>
#include <mach/regs-pmu.h>

void s3c_adc_phy_init(void)
{
	u32 reg;

	reg = __raw_readl(EXYNOS5_ADC_PHY_CONTROL);
	reg |= EXYNOS5_ADC_PHY_ENABLE;
	__raw_writel(reg, EXYNOS5_ADC_PHY_CONTROL);
}

void s3c_adc_phy_exit(void)
{
	u32 reg;

	reg = __raw_readl(EXYNOS5_ADC_PHY_CONTROL);
	reg &= ~EXYNOS5_ADC_PHY_ENABLE;
	__raw_writel(reg, EXYNOS5_ADC_PHY_CONTROL);
}
