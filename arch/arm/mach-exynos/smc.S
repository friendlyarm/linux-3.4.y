/*
 * Copyright (C) 2012 Google, Inc.
 *
 * This program is free software,you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#include <linux/init.h>

ENTRY(exynos_smc)
	stmfd	sp!, {lr}
#if defined(CONFIG_ARM_ERRATA_766421)
	dmb
#endif
	smc	0
#if defined(CONFIG_ARM_ERRATA_766421)
	dmb
#endif
	ldmfd	sp!, {pc}
ENDPROC(exynos_smc)

ENTRY(exynos_smc_read_sfr)
	stmfd	sp!, {r4, lr}
	mov	r4, r2
#if defined(CONFIG_ARM_ERRATA_766421)
	dmb
#endif
	smc	0
#if defined(CONFIG_ARM_ERRATA_766421)
	dmb
#endif
	str	r2, [r4]
	ldmfd	sp!, {r4, pc}
ENDPROC(exynos_smc_read_sfr)
