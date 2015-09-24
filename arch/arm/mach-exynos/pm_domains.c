/*
 * Exynos Generic power domain support.
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Implementation of Exynos specific power domain control which is used in
 * conjunction with runtime-pm. Support for both device-tree and non-device-tree
 * based power domain support is included.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/pm_domain.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/list.h>

#include <mach/regs-pmu.h>
#include <mach/regs-clock.h>
#include <mach/exynos5_bus.h>
#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/bts.h>

#define MAX_PD_CLKS 2
#define EXYNOS5_INT_MIN_FREQ	266000

/*
 * Exynos specific wrapper around the generic power domain
 */
struct exynos_pm_domain {
	struct list_head list;
	void __iomem *base;
	bool is_off;
	struct generic_pm_domain pd;
	const char *pclk_name;
	struct clk *pclk;
	unsigned long saved_pclk_rate;
	const char *clk_name[MAX_PD_CLKS];
	struct clk *clk[MAX_PD_CLKS];
	struct clk *saved_parent_clk[MAX_PD_CLKS];
	struct exynos5_bus_int_handle *int_min_hd;
};

struct exynos_pm_clk {
	struct list_head node;
	struct clk *clk;
};

struct exynos_pm_dev {
	struct exynos_pm_domain *pd;
	struct platform_device *pdev;
	char const *con_id;
};

#define EXYNOS_PM_DEV(NAME, PD, DEV, CON)		\
static struct exynos_pm_dev exynos5_pm_dev_##NAME = {	\
	.pd = &exynos5_pd_##PD,				\
	.pdev = DEV,					\
	.con_id = CON,					\
}

static void exynos_pd_clk_parent_save(struct exynos_pm_domain *pd)
{
	int i;
	int r;

	for (i = 0; i < MAX_PD_CLKS; i++) {
		if (pd->clk[i]) {
			pd->saved_parent_clk[i] = clk_get_parent(pd->clk[i]);
			if (IS_ERR(pd->saved_parent_clk)) {
				pr_err("Failed to save parent clk of %s for pd %s\n",
					pd->clk_name[i], pd->pd.name);
				pd->saved_parent_clk[i] = NULL;
			}
		}
	}

	if (pd->pclk) {
		if (!pd->int_min_hd) {
			pd->int_min_hd = exynos5_bus_int_min(EXYNOS5_INT_MIN_FREQ);
			if (!pd->int_min_hd)
				pr_err("Failed to request int min_freq\n");
		}

		pd->saved_pclk_rate = clk_get_rate(pd->pclk);
		if (!pd->saved_pclk_rate) {
			pr_err("Failed to save rate of %s for pd %s\n",
					pd->pclk_name, pd->pd.name);
			pd->saved_pclk_rate = 0;
		}

		r = clk_set_rate(pd->pclk, clk_get_rate(pd->pclk->parent));
		if (r)
			pr_err("Failed to set rate of parent clk of %s for pd %s\n",
				pd->pclk_name, pd->pd.name);

		if (pd->int_min_hd) {
			exynos5_bus_int_put(pd->int_min_hd);
			pd->int_min_hd = NULL;
		}
	}
}

static void exynos_pd_clk_parent_restore(struct exynos_pm_domain *pd)
{
	int i;
	int r;

	for (i = 0; i < MAX_PD_CLKS; i++) {
		if (pd->clk[i] && pd->saved_parent_clk[i]) {
			r = clk_set_parent(pd->clk[i], pd->saved_parent_clk[i]);
			if (r)
				pr_err("Failed to restore parent clk of %s for pd %s\n",
					pd->clk_name[i], pd->pd.name);
			pd->saved_parent_clk[i] = NULL;
		}
	}

	if (pd->pclk) {
		if (!pd->int_min_hd) {
			pd->int_min_hd = exynos5_bus_int_min(EXYNOS5_INT_MIN_FREQ);
			if (!pd->int_min_hd)
				pr_err("Failed to request int min_freq\n");
		}

		if (pd->saved_pclk_rate) {
			r = clk_set_rate(pd->pclk, pd->saved_pclk_rate);
			if (r)
				pr_err("Failed to restore rate of %s for pd %s\n",
					pd->pclk_name, pd->pd.name);
			pd->saved_pclk_rate = 0;
		}

		if (pd->int_min_hd) {
			exynos5_bus_int_put(pd->int_min_hd);
			pd->int_min_hd = NULL;
		}
	}
}

static int exynos_pd_power(struct generic_pm_domain *domain, bool power_on)
{
	struct gpd_link *link;
	struct exynos_pm_domain *pd, *spd;
	struct exynos_pm_clk *pclk, *spclk;
	void __iomem *base;
	u32 timeout, pwr;
	char *op;
	int ret = 0;

	pd = container_of(domain, struct exynos_pm_domain, pd);
	base = pd->base;

	if (!base) {
		pr_err("%s: Failed to get %s power domain base address\n",
			__func__, domain->name);
		return -EINVAL;
	}

	/* Enable all the clocks of IPs in power domain */
	list_for_each_entry(pclk, &pd->list, node) {
		if (clk_enable(pclk->clk)) {
			ret = -EINVAL;
			goto unwind;
		}
	}

	list_for_each_entry(link, &domain->master_links, master_node) {
		spd = container_of(link->slave, struct exynos_pm_domain, pd);
		list_for_each_entry(spclk, &spd->list, node) {
			if (clk_enable(spclk->clk)) {
				ret = -EINVAL;
				goto s_unwind;
			}
		}
	}

	if (soc_is_exynos5250() &&
		!power_on && base == EXYNOS5_ISP_CONFIGURATION)
		__raw_writel(0x0, EXYNOS5_CMU_RESET_ISP_SYS_PWR_REG);

	if (soc_is_exynos5250() &&
		!power_on && base == EXYNOS5_MAU_CONFIGURATION) {
		__raw_writel(0x0, EXYNOS5_CMU_CLKSTOP_MAU_SYS_PWR_REG);
		__raw_writel(0x0, EXYNOS5_CMU_RESET_MAU_SYS_PWR_REG);
		__raw_writel(0x0, EXYNOS5_PAD_RETENTION_MAU_SYS_PWR_REG);
	}

	if (!power_on)
		exynos_pd_clk_parent_save(pd);

	if (soc_is_exynos5250() &&
		(base == EXYNOS5_MFC_CONFIGURATION ||
		base == EXYNOS5_G3D_CONFIGURATION ||
		base == EXYNOS5_GSCL_CONFIGURATION))
		exynos5_mif_used_dev(power_on);

	pwr = power_on ? EXYNOS_INT_LOCAL_PWR_EN : 0;

	__raw_writel(pwr, base);

	/* Wait max 1ms */
	timeout = 10;

	while ((__raw_readl(base + 0x4) & EXYNOS_INT_LOCAL_PWR_EN) != pwr) {
		if (!timeout) {
			op = (power_on) ? "enable" : "disable";
			pr_err("Power domain %s %s failed\n", domain->name, op);
			ret = -ETIMEDOUT;
			break;
		}
		timeout--;
		cpu_relax();
		usleep_range(80, 100);
	}

	if (soc_is_exynos5250() && !timeout)
		pr_err("%s power domain state (0x%02x)\n", domain->name,
			(__raw_readl(base + 0x4) >> EXYNOS5_LOCAL_POWER_STATE_SHIFT) &
			EXYNOS5_LOCAL_POWER_STATE_MASK);

	if (power_on)
		exynos_pd_clk_parent_restore(pd);

	if (soc_is_exynos5250() &&
		power_on && base == EXYNOS5_MAU_CONFIGURATION) {
		__raw_writel(0x10000000, EXYNOS_PAD_RET_MAUDIO_OPTION);
	}

	/* Disable all the clocks of IPs in power domain */
	list_for_each_entry(link, &domain->master_links, master_node) {
		spd = container_of(link->slave, struct exynos_pm_domain, pd);
		list_for_each_entry(spclk, &spd->list, node) {
			clk_disable(spclk->clk);
		}
	}

	list_for_each_entry(pclk, &pd->list, node)
		clk_disable(pclk->clk);

	bts_initialize(pd->pd.name, power_on);

	return ret;

s_unwind:
	list_for_each_entry_continue_reverse(link, &domain->master_links, master_node) {
		spd = container_of(link->slave, struct exynos_pm_domain, pd);
		list_for_each_entry_continue_reverse(spclk, &spd->list, node) {
			clk_disable(spclk->clk);
		}
	}
unwind:
	list_for_each_entry_continue_reverse(pclk, &pd->list, node)
		clk_disable(pclk->clk);

	return ret;
}

static int exynos_pd_power_on(struct generic_pm_domain *domain)
{
	return exynos_pd_power(domain, true);
}

static int exynos_pd_power_off(struct generic_pm_domain *domain)
{
	return exynos_pd_power(domain, false);
}

static int exynos_sub_power_on(struct generic_pm_domain *domain)
{
	return 0;
}

static int exynos_sub_power_off(struct generic_pm_domain *domain)
{
	return 0;
}

#define EXYNOS_GPD(PD, BASE, NAME, PCLK, CLKS...)	\
static struct exynos_pm_domain PD = {			\
	.list = LIST_HEAD_INIT((PD).list),		\
	.base = (void __iomem *)BASE,			\
	.pd = {						\
		.name = NAME,				\
		.power_off = exynos_pd_power_off,	\
		.power_on = exynos_pd_power_on,		\
	},						\
	.pclk_name = PCLK,				\
	.clk_name = { CLKS }				\
}

#define EXYNOS_SUB_GPD(PD, NAME)			\
static struct exynos_pm_domain PD = {			\
	.list = LIST_HEAD_INIT((PD).list),		\
	.pd = {						\
		.name = NAME,					\
		.power_off = exynos_sub_power_off,	\
		.power_on = exynos_sub_power_on,	\
	},						\
}

#ifdef CONFIG_OF
static __init int exynos_pm_dt_parse_domains(void)
{
	struct device_node *np;

	for_each_compatible_node(np, NULL, "samsung,exynos4210-pd") {
		struct exynos_pm_domain *pd;

		pd = kzalloc(sizeof(*pd), GFP_KERNEL);
		if (!pd) {
			pr_err("%s: failed to allocate memory for domain\n",
					__func__);
			return -ENOMEM;
		}

		if (of_get_property(np, "samsung,exynos4210-pd-off", NULL))
			pd->is_off = true;
		pd->name = np->name;
		pd->base = of_iomap(np, 0);
		pd->pd.power_off = exynos_pd_power_off;
		pd->pd.power_on = exynos_pd_power_on;
		pd->pd.of_node = np;
		pm_genpd_init(&pd->pd, NULL, false);
	}
	return 0;
}
#else
static __init int exynos_pm_dt_parse_domains(void)
{
	return 0;
}
#endif /* CONFIG_OF */

static __init void exynos_pm_add_subdomain_to_genpd(struct generic_pm_domain *genpd,
						struct generic_pm_domain *subdomain)
{
	if (pm_genpd_add_subdomain(genpd, subdomain))
		pr_info("%s: error in adding %s subdomain to %s power "
			"domain\n", __func__, subdomain->name, genpd->name);
}

static __init void exynos_pm_add_dev_to_genpd(struct platform_device *pdev,
						struct exynos_pm_domain *pd)
{
	if (pdev->dev.bus) {
		if (!pm_genpd_add_device(&pd->pd, &pdev->dev))
			pm_genpd_dev_need_restore(&pdev->dev, true);
		else
			pr_info("%s: error in adding %s device to %s power"
				"domain\n", __func__, dev_name(&pdev->dev),
				pd->pd.name);
	}
}

/* For EXYNOS4 */
EXYNOS_GPD(exynos4_pd_mfc, EXYNOS4_MFC_CONFIGURATION, "pd-mfc", NULL);
EXYNOS_GPD(exynos4_pd_g3d, EXYNOS4_G3D_CONFIGURATION, "pd-g3d", NULL);
EXYNOS_GPD(exynos4_pd_lcd0, EXYNOS4_LCD0_CONFIGURATION, "pd-lcd0", NULL);
EXYNOS_GPD(exynos4_pd_tv, EXYNOS4_TV_CONFIGURATION, "pd-tv", NULL);
EXYNOS_GPD(exynos4_pd_cam, EXYNOS4_CAM_CONFIGURATION, "pd-cam", NULL);
EXYNOS_GPD(exynos4_pd_gps, EXYNOS4_GPS_CONFIGURATION, "pd-gps", NULL);

/* For EXYNOS4210 */
EXYNOS_GPD(exynos4210_pd_lcd1, EXYNOS4210_LCD1_CONFIGURATION, "pd-lcd1", NULL);

static struct exynos_pm_domain *exynos4_pm_domains[] = {
	&exynos4_pd_mfc,
	&exynos4_pd_g3d,
	&exynos4_pd_lcd0,
	&exynos4_pd_tv,
	&exynos4_pd_cam,
	&exynos4_pd_gps,
};

static struct exynos_pm_domain *exynos4210_pm_domains[] = {
	&exynos4210_pd_lcd1,
};

static __init int exynos4_pm_init_power_domain(void)
{
	int idx;

	if (of_have_populated_dt())
		return exynos_pm_dt_parse_domains();

	for (idx = 0; idx < ARRAY_SIZE(exynos4_pm_domains); idx++)
		pm_genpd_init(&exynos4_pm_domains[idx]->pd, NULL,
				exynos4_pm_domains[idx]->is_off);

	if (soc_is_exynos4210())
		for (idx = 0; idx < ARRAY_SIZE(exynos4210_pm_domains); idx++)
			pm_genpd_init(&exynos4210_pm_domains[idx]->pd, NULL,
					exynos4210_pm_domains[idx]->is_off);

#ifdef CONFIG_S5P_DEV_FIMD0
	exynos_pm_add_dev_to_genpd(&s5p_device_fimd0, &exynos4_pd_lcd0);
#endif
#ifdef CONFIG_S5P_DEV_TV
	exynos_pm_add_dev_to_genpd(&s5p_device_hdmi, &exynos4_pd_tv);
	exynos_pm_add_dev_to_genpd(&s5p_device_mixer, &exynos4_pd_tv);
#endif
#ifdef CONFIG_S5P_DEV_MFC
	exynos_pm_add_dev_to_genpd(&s5p_device_mfc, &exynos4_pd_mfc);
#endif
#ifdef CONFIG_S5P_DEV_FIMC0
	exynos_pm_add_dev_to_genpd(&s5p_device_fimc0, &exynos4_pd_cam);
#endif
#ifdef CONFIG_S5P_DEV_FIMC1
	exynos_pm_add_dev_to_genpd(&s5p_device_fimc1, &exynos4_pd_cam);
#endif
#ifdef CONFIG_S5P_DEV_FIMC2
	exynos_pm_add_dev_to_genpd(&s5p_device_fimc2, &exynos4_pd_cam);
#endif
#ifdef CONFIG_S5P_DEV_FIMC3
	exynos_pm_add_dev_to_genpd(&s5p_device_fimc3, &exynos4_pd_cam);
#endif
#ifdef CONFIG_S5P_DEV_CSIS0
	exynos_pm_add_dev_to_genpd(&s5p_device_mipi_csis0, &exynos4_pd_cam);
#endif
#ifdef CONFIG_S5P_DEV_CSIS1
	exynos_pm_add_dev_to_genpd(&s5p_device_mipi_csis1, &exynos4_pd_cam);
#endif
#ifdef CONFIG_S5P_DEV_G2D
	exynos_pm_add_dev_to_genpd(&s5p_device_g2d, &exynos4_pd_lcd0);
#endif
#ifdef CONFIG_S5P_DEV_JPEG
	exynos_pm_add_dev_to_genpd(&s5p_device_jpeg, &exynos4_pd_cam);
#endif
	return 0;
}

/* For EXYNOS5 */
EXYNOS_GPD(exynos5_pd_mfc, EXYNOS5_MFC_CONFIGURATION, "pd-mfc",
			"pclk_83_mfc", "aclk_333");
EXYNOS_GPD(exynos5_pd_maudio, EXYNOS5_MAU_CONFIGURATION, "pd-maudio", NULL);
EXYNOS_GPD(exynos5_pd_disp1, EXYNOS5_DISP1_CONFIGURATION, "pd-disp1",
			"pclk_100_disp1", "aclk_200_disp1", "aclk_300_disp1");
EXYNOS_SUB_GPD(exynos5_pd_fimd1, "pd-fimd1");
EXYNOS_SUB_GPD(exynos5_pd_hdmi, "pd-hdmi");
EXYNOS_SUB_GPD(exynos5_pd_mixer, "pd-mixer");
EXYNOS_SUB_GPD(exynos5_pd_dp, "pd-dp");
EXYNOS_GPD(exynos5_pd_gscl, EXYNOS5_GSCL_CONFIGURATION, "pd-gscl",
			"pclk_133_gscl", "aclk_266_gscl", "aclk_300_gscl");
EXYNOS_SUB_GPD(exynos5_pd_gscl0, "pd-gscl0");
EXYNOS_SUB_GPD(exynos5_pd_gscl1, "pd-gscl1");
EXYNOS_SUB_GPD(exynos5_pd_gscl2, "pd-gscl2");
EXYNOS_SUB_GPD(exynos5_pd_gscl3, "pd-gscl3");
EXYNOS_GPD(exynos5_pd_isp, EXYNOS5_ISP_CONFIGURATION, "pd-isp", NULL);
EXYNOS_GPD(exynos5_pd_g3d, EXYNOS5_G3D_CONFIGURATION, "pd-g3d", NULL);

static struct exynos_pm_domain *exynos5_pm_domains[] = {
	&exynos5_pd_mfc,
	&exynos5_pd_maudio,
	&exynos5_pd_disp1,
	&exynos5_pd_fimd1,
	&exynos5_pd_hdmi,
	&exynos5_pd_mixer,
	&exynos5_pd_dp,
	&exynos5_pd_gscl,
	&exynos5_pd_gscl0,
	&exynos5_pd_gscl1,
	&exynos5_pd_gscl2,
	&exynos5_pd_gscl3,
	&exynos5_pd_isp,
	&exynos5_pd_g3d,
};

#ifdef CONFIG_S5P_DEV_MFC
EXYNOS_PM_DEV(mfc, mfc, &s5p_device_mfc, "mfc");
#endif
#ifdef CONFIG_SND_SAMSUNG_I2S
EXYNOS_PM_DEV(maudio, maudio, &exynos5_device_i2s0, NULL);
#endif
#ifdef CONFIG_S5P_DEV_FIMD1
EXYNOS_PM_DEV(fimd1, fimd1, &s5p_device_fimd1, "fimd");
#endif
#ifdef CONFIG_S5P_DEV_TV
EXYNOS_PM_DEV(hdmi, hdmi, &s5p_device_hdmi, NULL);
EXYNOS_PM_DEV(mixer, mixer, &s5p_device_mixer, "mixer");
#endif
#ifdef CONFIG_S5P_DEV_DP
EXYNOS_PM_DEV(dp, dp, &s5p_device_dp, "dp");
#endif
#ifdef CONFIG_EXYNOS5_DEV_GSC
EXYNOS_PM_DEV(gscl0, gscl0, &exynos5_device_gsc0, "gscl");
EXYNOS_PM_DEV(gscl1, gscl1, &exynos5_device_gsc1, "gscl");
EXYNOS_PM_DEV(gscl2, gscl2, &exynos5_device_gsc2, "gscl");
EXYNOS_PM_DEV(gscl3, gscl3, &exynos5_device_gsc3, "gscl");
#endif
#ifdef CONFIG_EXYNOS4_DEV_FIMC_IS
EXYNOS_PM_DEV(isp, isp, &exynos5_device_fimc_is, NULL);
#endif
#ifdef CONFIG_MALI_T6XX
EXYNOS_PM_DEV(g3d, g3d, &exynos5_device_g3d, "g3d");
#endif

static struct exynos_pm_dev *exynos_pm_devs[] = {
#ifdef CONFIG_S5P_DEV_MFC
	&exynos5_pm_dev_mfc,
#endif
#ifdef CONFIG_SND_SAMSUNG_I2S
	&exynos5_pm_dev_maudio,
#endif
#ifdef CONFIG_S5P_DEV_FIMD1
	&exynos5_pm_dev_fimd1,
#endif
#ifdef CONFIG_S5P_DEV_TV
	&exynos5_pm_dev_hdmi,
	&exynos5_pm_dev_mixer,
#endif
#ifdef CONFIG_S5P_DEV_DP
	&exynos5_pm_dev_dp,
#endif
#ifdef CONFIG_EXYNOS5_DEV_GSC
	&exynos5_pm_dev_gscl0,
	&exynos5_pm_dev_gscl1,
	&exynos5_pm_dev_gscl2,
	&exynos5_pm_dev_gscl3,
#endif
#ifdef CONFIG_EXYNOS4_DEV_FIMC_IS
	&exynos5_pm_dev_isp,
#endif
#ifdef CONFIG_MALI_T6XX
	&exynos5_pm_dev_g3d,
#endif
};

static void __init exynos5_add_device_to_pd(struct exynos_pm_dev **pm_dev, int size)
{
	struct exynos_pm_dev *tdev;
	struct exynos_pm_clk *pclk;
	struct clk *clk;
	int i;

	for (i = 0; i < size; i++) {
		tdev = pm_dev[i];

		if (!tdev->con_id)
			continue;

		pclk = kzalloc(sizeof(struct exynos_pm_clk), GFP_KERNEL);

		if (!pclk) {
			pr_err("Unable to create new exynos_pm_clk\n");
			continue;
		}

		clk = clk_get(&tdev->pdev->dev, tdev->con_id);

		if (!IS_ERR(clk)) {
			pclk->clk =  clk;
			list_add(&pclk->node, &tdev->pd->list);
		} else {
			pr_err("Failed to get %s clock\n", dev_name(&tdev->pdev->dev));
			kfree(pclk);
		}

	}
}

static void __init exynos5_pm_init_one_pd(struct exynos_pm_domain *pd)
{
	int i;

	pm_genpd_init(&pd->pd, NULL, pd->is_off);

	for (i = 0; i < MAX_PD_CLKS; i++)
		if (pd->clk_name[i])
			pd->clk[i] = clk_get(NULL, pd->clk_name[i]);

	if (pd->pclk_name)
		pd->pclk = clk_get(NULL, pd->pclk_name);
}

static int __init exynos5_pm_init_power_domain(void)
{
	int idx;

	if (of_have_populated_dt())
		return exynos_pm_dt_parse_domains();

	for (idx = 0; idx < ARRAY_SIZE(exynos5_pm_domains); idx++)
		exynos5_pm_init_one_pd(exynos5_pm_domains[idx]);

#ifdef CONFIG_S5P_DEV_MFC
	exynos_pm_add_dev_to_genpd(&s5p_device_mfc, &exynos5_pd_mfc);
#endif
#ifdef CONFIG_SND_SAMSUNG_I2S
	exynos_pm_add_dev_to_genpd(&exynos5_device_i2s0, &exynos5_pd_maudio);
#endif
#ifdef CONFIG_S5P_DEV_FIMD1
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_disp1.pd, &exynos5_pd_fimd1.pd);
	exynos_pm_add_dev_to_genpd(&s5p_device_fimd1, &exynos5_pd_fimd1);
#endif
#ifdef CONFIG_S5P_DEV_TV
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_disp1.pd, &exynos5_pd_hdmi.pd);
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_disp1.pd, &exynos5_pd_mixer.pd);
	exynos_pm_add_dev_to_genpd(&s5p_device_hdmi, &exynos5_pd_hdmi);
	exynos_pm_add_dev_to_genpd(&s5p_device_mixer, &exynos5_pd_mixer);
#endif
#ifdef CONFIG_S5P_DEV_DP
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_disp1.pd, &exynos5_pd_dp.pd);
	exynos_pm_add_dev_to_genpd(&s5p_device_dp, &exynos5_pd_dp);
#endif
#ifdef CONFIG_EXYNOS5_DEV_GSC
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_gscl.pd, &exynos5_pd_gscl0.pd);
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_gscl.pd, &exynos5_pd_gscl1.pd);
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_gscl.pd, &exynos5_pd_gscl2.pd);
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_gscl.pd, &exynos5_pd_gscl3.pd);
	exynos_pm_add_dev_to_genpd(&exynos5_device_gsc0, &exynos5_pd_gscl0);
	exynos_pm_add_dev_to_genpd(&exynos5_device_gsc1, &exynos5_pd_gscl1);
	exynos_pm_add_dev_to_genpd(&exynos5_device_gsc2, &exynos5_pd_gscl2);
	exynos_pm_add_dev_to_genpd(&exynos5_device_gsc3, &exynos5_pd_gscl3);
#endif
#ifdef CONFIG_EXYNOS4_DEV_FIMC_IS
	exynos_pm_add_dev_to_genpd(&exynos5_device_fimc_is, &exynos5_pd_isp);
	exynos_pm_add_subdomain_to_genpd(&exynos5_pd_gscl.pd, &exynos5_pd_isp.pd);
#endif
#ifdef CONFIG_MALI_T6XX
	exynos_pm_add_dev_to_genpd(&exynos5_device_g3d, &exynos5_pd_g3d);
#endif

	exynos5_add_device_to_pd(exynos_pm_devs, ARRAY_SIZE(exynos_pm_devs));

	return 0;
}

static int __init exynos_pm_init_power_domain(void)
{
	if (soc_is_exynos5250())
		return exynos5_pm_init_power_domain();
	else
		return exynos4_pm_init_power_domain();
}
arch_initcall(exynos_pm_init_power_domain);

static __init int exynos_pm_late_initcall(void)
{
	pm_genpd_poweroff_unused();
	return 0;
}
late_initcall(exynos_pm_late_initcall);
