/* linux/arch/arm/mach-exynos/include/mach/pmu.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - PMU(Power Management Unit) support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_PMU_H
#define __ASM_ARCH_PMU_H __FILE__

#define PMU_TABLE_END	NULL

enum sys_powerdown {
	SYS_AFTR,
	SYS_LPA,
	SYS_SLEEP,
	NUM_SYS_POWERDOWN,
};

extern unsigned long l2x0_regs_phys;
struct exynos_pmu_conf {
	void __iomem *reg;
	unsigned int val[NUM_SYS_POWERDOWN];
};

extern void exynos_sys_powerdown_conf(enum sys_powerdown mode);
extern void exynos_xxti_sys_powerdown(bool enable);
extern void s3c_cpu_resume(void);
extern void exynos_reset_assert_ctrl(bool on);

#endif /* __ASM_ARCH_PMU_H */
