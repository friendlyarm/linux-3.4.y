/*
 * STM32 based PMIC chip client interface
 *
 * Copyright (C) Guangzhou FriendlyElec Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LINUX_REGULATOR_SPU1705_H
#define __LINUX_REGULATOR_SPU1705_H

#include <linux/regulator/machine.h>

#define SPU1705_DCDC1		0
#define SPU1705_DCDC2		1

#define SPU1705_NUM_REGULATORS	2


struct spu1705_regulator_subdev {
	int id;
	struct regulator_init_data *initdata;
};

struct spu1705_platform_data {
	int num_regulators;
	struct spu1705_regulator_subdev *regulators;
};

#endif /* __LINUX_REGULATOR_SPU1705_H */
