/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS5250 - ASV(Adaptive Support Voltage) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <mach/asv-exynos.h>
#include <mach/asv-exynos5250.h>

#include <mach/map.h>
#include <mach/regs-pmu.h>
#include <mach/abb-exynos.h>

#include <plat/cpu.h>

/* ASV function for Fused Chip */
#define IDS_ARM_OFFSET			24
#define IDS_ARM_MASK			0xFF
#define HPM_OFFSET			12
#define HPM_MASK			0x1F
#define FUSED_ASV_GROUP_OFFSET		3
#define ARM_ORG_ASV_GROUP_OFFSET	17
#define ARM_ORG_ASV_MASK		0xF
#define ARM_DIFF_ASV_GROUP_OFFSET	21
#define ARM_DIFF_ASV_MASK		0x7
#define MIF_ORG_ASV_GROUP_OFFSET	26
#define MIF_ORG_ASV_MASK		0x3
#define MIF_DIFF_ASV_GROUP_OFFSET	28
#define MIF_DIFF_ASV_MASK		0x3
#define FUSED_MIF_VOL_LOCK_OFFSET	7
#define FUSED_INT_VOL_LOCK_OFFSET	8
#define FUSED_G3D_VOL_LOCK_OFFSET	9
#define FUSED_ARM_VOL_LOCK_OFFSET	10
#define MIF_VOL_OFFSET			50000
#define INT_VOL_OFFSET			25000
#define G3D_VOL_OFFSET			25000
#define ARM_FREQ_800MHZ			800000
#define ARM_FREQ_1000MHZ		1000000
#define ARM_FREQ_1100MHZ		1100000

#define CHIP_ID_REG		(S5P_VA_CHIPID + 0x4)

enum exynos5250_fused_vol_lock_t {
	FUSED_MIF_VOL_LOCK = 0,
	FUSED_INT_VOL_LOCK,
	FUSED_G3D_VOL_LOCK,
	FUSED_ARM_800MHZ_VOL_LOCK,
	FUSED_ARM_1000MHZ_VOL_LOCK,
	FUSED_ARM_1100MHZ_VOL_LOCK,
	FUSED_VOL_LOCK_END,
};

static bool fused_vol_locked[FUSED_VOL_LOCK_END] =
		{false, false, false, false, false, false};
static unsigned int arm_vol_lock_freq;
static int arm_vol_lock_level;
static unsigned int asv_group[ID_END];

static unsigned int exynos5250_default_asv_max_volt[] = {
	[ID_ARM] = 1300000,
	[ID_INT] = 1037500,
	[ID_MIF] = 1125000,
	[ID_G3D] = 1200000,
};

static unsigned int asv_group_nr[] = {
	[ID_ARM] = ARM_ASV_GRP_NR,
	[ID_INT] = INT_ASV_GRP_NR,
	[ID_MIF] = MIF_ASV_GRP_NR,
	[ID_G3D] = G3D_ASV_GRP_NR,
};

static unsigned int dvfs_level_nr[] = {
	[ID_ARM] = ARM_DVFS_LEVEL_NR,
	[ID_INT] = INT_DVFS_LEVEL_NR,
	[ID_MIF] = MIF_DVFS_LEVEL_NR,
	[ID_G3D] = G3D_DVFS_LEVEL_NR,
};

typedef unsigned int (*refer_table_get_asv)[MAX_ASV_GRP_NR];

refer_table_get_asv refer_table[] = {
	[ID_ARM] = arm_refer_table_get_asv,
	[ID_INT] = int_refer_table_get_asv,
	[ID_MIF] = mif_refer_table_get_asv,
	[ID_G3D] = g3d_refer_table_get_asv,
};

typedef unsigned int (*asv_volt_info)[MAX_ASV_GRP_NR + 1];

asv_volt_info volt_table[] = {
	[ID_ARM] = arm_asv_volt_info,
	[ID_INT] = int_asv_volt_info,
	[ID_MIF] = mif_asv_volt_info,
	[ID_G3D] = g3d_asv_volt_info,
};

static void exynos5250_pre_set_abb(unsigned int asv_group_number)
{
	switch (asv_group_number) {
	case 0:
	case 1:
		set_abb_member(ABB_ARM, ABB_MODE_080V);
		set_abb_member(ABB_INT, ABB_MODE_080V);
		set_abb_member(ABB_G3D, ABB_MODE_080V);
		break;
	default:
		set_abb_member(ABB_ARM, ABB_MODE_BYPASS);
		set_abb_member(ABB_INT, ABB_MODE_BYPASS);
		set_abb_member(ABB_G3D, ABB_MODE_BYPASS);
		break;
	}

	set_abb_member(ABB_MIF, ABB_MODE_130V);
}
static unsigned int exynos5250_get_asv_group(unsigned int ids,
			unsigned int hpm, enum asv_type_id target_type)
{
	unsigned int i;
	unsigned int refer_ids;
	unsigned int refer_hpm;

	for (i = 0; i < asv_group_nr[target_type]; i++) {
		if (target_type != ID_MIF) {
			refer_ids = refer_table[target_type][0][i];
			refer_hpm = refer_table[target_type][1][i];

			if ((ids <= refer_ids) || (hpm <= refer_hpm))
				return i;
		} else {
			refer_hpm = refer_table[target_type][0][i];

			if (hpm <= refer_hpm)
				return i;
		}
	}

	/* Default max asv group */
	return 0;
}

unsigned int exynos5250_get_volt(enum asv_type_id target_type, unsigned int target_freq)
{
	int i;
	unsigned int group = asv_group[target_type];
	unsigned int offset = 0;

	for (i = 0; i < dvfs_level_nr[target_type]; i++) {
		if (volt_table[target_type][i][0] == target_freq) {
			if (target_type == ID_MIF &&
					fused_vol_locked[FUSED_MIF_VOL_LOCK]) {
				offset = MIF_VOL_OFFSET;
			} else if (target_type == ID_INT &&
					fused_vol_locked[FUSED_INT_VOL_LOCK]) {
				offset = INT_VOL_OFFSET;
			} else if (target_type == ID_G3D &&
					fused_vol_locked[FUSED_G3D_VOL_LOCK]) {
				offset = G3D_VOL_OFFSET;
			} else if (target_type == ID_ARM) {
				if ((fused_vol_locked[FUSED_ARM_800MHZ_VOL_LOCK] ||
					fused_vol_locked[FUSED_ARM_1000MHZ_VOL_LOCK] ||
					fused_vol_locked[FUSED_ARM_1100MHZ_VOL_LOCK]) &&
					(target_freq < arm_vol_lock_freq))
					i = arm_vol_lock_level;
			}

			return volt_table[target_type][i][group + 1] + offset;
		}
	}

	return exynos5250_default_asv_max_volt[target_type];
}

unsigned int exynos5250_set_volt(enum asv_type_id target_type,
				unsigned int target_freq, unsigned int group,
				unsigned int volt)
{
	int i;

	if (target_type >= ID_END)
		return -EINVAL;

	if (group == ~0)
		group = asv_group[target_type];

	if (group >= asv_group_nr[target_type])
		return -EINVAL;

	for (i = 0; i < dvfs_level_nr[target_type]; i++) {
		if (volt_table[target_type][i][0] == target_freq) {
			volt_table[target_type][i][group + 1] = volt;
			return 0;
		}
	}

	return -EINVAL;
}

int exynos5250_init_asv(struct asv_common *asv_info)
{
	int i;
	unsigned int tmp1, tmp2;
	unsigned hpm_value, ids_value;

	/* read IDS and HPM value from  CHIP ID */
	tmp1 = __raw_readl(CHIP_ID_REG);

	/* ASV group is decided by direct fused asv number
	 * or calcualting with ARM_IDS and HPM value.
	 */
	if ((tmp1 >> FUSED_ASV_GROUP_OFFSET) & 0x1) {
		tmp2 =  __raw_readl(CHIP_ID_REG + 0x4);
		asv_group[ID_ARM] = ((tmp1 >> ARM_ORG_ASV_GROUP_OFFSET) & ARM_ORG_ASV_MASK)
				- ((tmp1 >> ARM_DIFF_ASV_GROUP_OFFSET) & ARM_DIFF_ASV_MASK);
		asv_group[ID_INT] = asv_group[ID_ARM];
		asv_group[ID_G3D] = asv_group[ID_ARM];
		asv_group[ID_MIF] = ((tmp2 >> MIF_ORG_ASV_GROUP_OFFSET) & MIF_ORG_ASV_MASK)
				- ((tmp2 >> MIF_DIFF_ASV_GROUP_OFFSET) & MIF_DIFF_ASV_MASK);
		pr_info("EXYNOS5250 ASV(ARM : %d MIF : %d) using fused group\n",
			asv_group[ID_ARM], asv_group[ID_MIF]);
	} else {
		hpm_value = (tmp1 >> HPM_OFFSET) & HPM_MASK;
		ids_value = (tmp1 >> IDS_ARM_OFFSET) & IDS_ARM_MASK;

		for (i = ID_ARM; i < ID_END; i++)
			asv_group[i] = exynos5250_get_asv_group(ids_value, hpm_value, i);

		pr_info("EXYNOS5250 ASV(ARM : %d MIF : %d) using IDS : %d HPM : %d\n",
			asv_group[ID_ARM], asv_group[ID_MIF], ids_value, hpm_value);
	}

	if ((tmp1 >> FUSED_MIF_VOL_LOCK_OFFSET) & 0x1)
		fused_vol_locked[FUSED_MIF_VOL_LOCK] = true;

	if ((tmp1 >> FUSED_INT_VOL_LOCK_OFFSET) & 0x1)
		fused_vol_locked[FUSED_INT_VOL_LOCK] = true;

	if ((tmp1 >> FUSED_G3D_VOL_LOCK_OFFSET) & 0x1)
		fused_vol_locked[FUSED_G3D_VOL_LOCK] = true;

	if (((tmp1 >> FUSED_ARM_VOL_LOCK_OFFSET) & 0x3) == 0x1) {
		fused_vol_locked[FUSED_ARM_800MHZ_VOL_LOCK] = true;
		arm_vol_lock_freq = ARM_FREQ_800MHZ;
	} else if (((tmp1 >> FUSED_ARM_VOL_LOCK_OFFSET) & 0x3) == 0x2) {
		fused_vol_locked[FUSED_ARM_1000MHZ_VOL_LOCK] = true;
		arm_vol_lock_freq = ARM_FREQ_1000MHZ;
	} else if (((tmp1 >> FUSED_ARM_VOL_LOCK_OFFSET) & 0x3) == 0x3) {
		fused_vol_locked[FUSED_ARM_1100MHZ_VOL_LOCK] = true;
		arm_vol_lock_freq = ARM_FREQ_1100MHZ;
	}

	if (fused_vol_locked[FUSED_ARM_800MHZ_VOL_LOCK] ||
		fused_vol_locked[FUSED_ARM_1000MHZ_VOL_LOCK] ||
		fused_vol_locked[FUSED_ARM_1100MHZ_VOL_LOCK]) {
		for (i = 0; i < dvfs_level_nr[ID_ARM]; i++) {
			if (volt_table[ID_ARM][i][0] == arm_vol_lock_freq) {
				arm_vol_lock_level = i;
				break;
			}
		}
	}

	exynos5250_pre_set_abb(asv_group[ID_ARM]);

	asv_info->get_voltage = exynos5250_get_volt;
	asv_info->init_done = true;

	return 0;
}
