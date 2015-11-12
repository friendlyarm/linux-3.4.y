/*
 * Base driver for X-Powers AXP
 *
 * Copyright (C) 2013 X-Powers, Ltd.
 *  Zhang Donglu <zhangdonglu@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c.h>
#include <mach/irqs.h>
#include <linux/power_supply.h>
#include <linux/apm_bios.h>
#include <linux/apm-emulation.h>
#include <linux/module.h>

#include "axp-cfg.h"
#include "axp-mfd.h"
#include "axp-regu.h"

/* Reverse engineered partly from Platformx drivers */
enum axp_regls{

	vcc_ldo1,
	vcc_ldo2,
	vcc_ldo3,
	vcc_ldo4,
	vcc_ldo5,
	vcc_ldo6,
	vcc_ldo7,
	vcc_ldo8,
	vcc_ldo9,
	vcc_ldo10,
	vcc_ldo11,
	vcc_ldo12,
	
	vcc_DCDC1,
	vcc_DCDC2,
	vcc_DCDC3,
	vcc_DCDC4,
	vcc_DCDC5,
	vcc_ldoio0,
	vcc_ldoio1,
};

/* The values of the various regulator constraints are obviously dependent
 * on exactly what is wired to each ldo.  Unfortunately this information is
 * not generally available.  More information has been requested from Xbow
 * but as of yet they haven't been forthcoming.
 *
 * Some of these are clearly Stargate 2 related (no way of plugging
 * in an lcd on the IM2 for example!).
 */

static struct regulator_consumer_supply ldo1_data[] = {
		{
			.supply = "axp22_rtc",
		},
	};


static struct regulator_consumer_supply ldo2_data[] = {
		{
			.supply = AXP_ALDO1_NAME,//"axp22_aldo1",
		},
	};

static struct regulator_consumer_supply ldo3_data[] = {
		{
			.supply = AXP_ALDO2_NAME,//"axp22_aldo2",
		},
	};

static struct regulator_consumer_supply ldo4_data[] = {
		{
			.supply = AXP_ALDO3_NAME,//"axp22_aldo3",
		},
	};

static struct regulator_consumer_supply ldo5_data[] = {
		{
			.supply = AXP_DLDO1_NAME,//"axp22_dldo1",
		},
	};


static struct regulator_consumer_supply ldo6_data[] = {
		{
			.supply = AXP_DLDO2_NAME,//"axp22_dldo2",
		},
	};

static struct regulator_consumer_supply ldo7_data[] = {
		{
			.supply = AXP_DLDO3_NAME,//"axp22_dldo3",
		},
	};

static struct regulator_consumer_supply ldo8_data[] = {
		{
			.supply = AXP_DLDO4_NAME,//"axp22_dldo4",
		},
	};

static struct regulator_consumer_supply ldo9_data[] = {
		{
			.supply = AXP_ELDO1_NAME,//"axp22_eldo1",
		},
	};


static struct regulator_consumer_supply ldo10_data[] = {
		{
			.supply = AXP_ELDO2_NAME,//"axp22_eldo2",
		},
	};

static struct regulator_consumer_supply ldo11_data[] = {
		{
			.supply = AXP_ELDO3_NAME,//"axp22_eldo3",
		},
	};

static struct regulator_consumer_supply ldo12_data[] = {
		{
			.supply = AXP_DC5LDO_NAME,//"axp22_dc5ldo",
		},
	};
	
static struct regulator_consumer_supply ldoio0_data[] = {
		{
			.supply = "axp22_ldoio0",
		},
	};

static struct regulator_consumer_supply ldoio1_data[] = {
		{
			.supply = "axp22_ldoio1",
		},
	};

static struct regulator_consumer_supply DCDC1_data[] = {
		{
			.supply = AXP_DCDC1_NAME,//"axp22_dcdc1",
		},
	};

static struct regulator_consumer_supply DCDC2_data[] = {
		{
			.supply = AXP_DCDC2_NAME,//"axp22_dcdc2",
		},
	};

static struct regulator_consumer_supply DCDC3_data[] = {
		{
			.supply = AXP_DCDC3_NAME,//"axp22_dcdc3",
		},
	};

static struct regulator_consumer_supply DCDC4_data[] = {
		{
			.supply = AXP_DCDC4_NAME,//"axp22_dcdc4",
		},
	};

static struct regulator_consumer_supply DCDC5_data[] = {
		{
			.supply = AXP_DCDC5_NAME,//"axp22_dcdc5",
		},
	};


static struct regulator_init_data axp_regl_init_data[] = {
	[vcc_ldo1] = {
		.constraints = { 
			.name = "axp22_ldo1",
			.min_uV =  AXP22_LDO1_MIN,
			.max_uV =  AXP22_LDO1_MAX,
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo1_data),
		.consumer_supplies = ldo1_data,
	},
	[vcc_ldo2] = {
		.constraints = { 
			.name = "axp22_aldo1",
			.min_uV = AXP22_LDO2_MIN,
			.max_uV = AXP22_LDO2_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_ALDO1_VALUE,
				.enabled = AXP_ALDO1_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo2_data),
		.consumer_supplies = ldo2_data,
	},
	[vcc_ldo3] = {
		.constraints = {
			.name = "axp22_aldo2",
			.min_uV =  AXP22_LDO3_MIN,
			.max_uV =  AXP22_LDO3_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_ALDO2_VALUE,
				.enabled = AXP_ALDO2_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo3_data),
		.consumer_supplies = ldo3_data,
	},
	[vcc_ldo4] = {
		.constraints = {
			.name = "axp22_aldo3",
			.min_uV = AXP22_LDO4_MIN,
			.max_uV = AXP22_LDO4_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_ALDO3_VALUE,
				.enabled = AXP_ALDO3_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo4_data),
		.consumer_supplies = ldo4_data,
	},
	[vcc_ldo5] = {
		.constraints = { 
			.name = "axp22_dldo1",
			.min_uV = AXP22_LDO5_MIN,
			.max_uV = AXP22_LDO5_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DLDO1_VALUE,
				.enabled = AXP_DLDO1_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo5_data),
		.consumer_supplies = ldo5_data,
	},
	[vcc_ldo6] = {
		.constraints = { 
			.name = "axp22_dldo2",
			.min_uV = AXP22_LDO6_MIN,
			.max_uV = AXP22_LDO6_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DLDO2_VALUE,
				.enabled = AXP_DLDO2_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo6_data),
		.consumer_supplies = ldo6_data,
	},
	[vcc_ldo7] = {
		.constraints = {
			.name = "axp22_dldo3",
			.min_uV =  AXP22_LDO7_MIN,
			.max_uV =  AXP22_LDO7_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DLDO3_VALUE,
				.enabled = AXP_DLDO3_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo7_data),
		.consumer_supplies = ldo7_data,
	},
	[vcc_ldo8] = {
		.constraints = {
			.name = "axp22_dldo4",
			.min_uV = AXP22_LDO8_MIN,
			.max_uV = AXP22_LDO8_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DLDO4_VALUE,
				.enabled = AXP_DLDO4_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo8_data),
		.consumer_supplies = ldo8_data,
	},
	[vcc_ldo9] = {
		.constraints = { 
			.name = "axp22_eldo1",
			.min_uV = AXP22_LDO9_MIN,
			.max_uV = AXP22_LDO9_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_ELDO1_VALUE,
				.enabled = AXP_ELDO1_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo9_data),
		.consumer_supplies = ldo9_data,
	},
	[vcc_ldo10] = {
		.constraints = {
			.name = "axp22_eldo2",
			.min_uV = AXP22_LDO10_MIN,
			.max_uV = AXP22_LDO10_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_ELDO2_VALUE,
				.enabled = AXP_ELDO2_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo10_data),
		.consumer_supplies = ldo10_data,
	},
	[vcc_ldo11] = {
		.constraints = {
			.name = "axp22_eldo3",
			.min_uV =  AXP22_LDO11_MIN,
			.max_uV =  AXP22_LDO11_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_ELDO3_VALUE,
				.enabled = AXP_ELDO3_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo11_data),
		.consumer_supplies = ldo11_data,
	},
	[vcc_ldo12] = {
		.constraints = {
			.name = "axp22_dc5ldo",
			.min_uV = AXP22_LDO12_MIN,
			.max_uV = AXP22_LDO12_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DC5LDO_VALUE,
				.enabled = AXP_DC5LDO_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo12_data),
		.consumer_supplies = ldo12_data,
	},
	[vcc_DCDC1] = {
		.constraints = {
			.name = "axp22_dcdc1",
			.min_uV = AXP22_DCDC1_MIN,
			.max_uV = AXP22_DCDC1_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DCDC1_VALUE,
				.enabled = AXP_DCDC1_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC1_data),
		.consumer_supplies = DCDC1_data,
	},
	[vcc_DCDC2] = {
		.constraints = {
			.name = "axp22_dcdc2",
			.min_uV = AXP22_DCDC2_MIN,
			.max_uV = AXP22_DCDC2_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT) && defined(CONFIG_ENABLE_INIT_VOLTAGE)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DCDC2_VALUE,
				.enabled = AXP_DCDC2_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC2_data),
		.consumer_supplies = DCDC2_data,
	},
	[vcc_DCDC3] = {
		.constraints = { 
			.name = "axp22_dcdc3",
			.min_uV = AXP22_DCDC3_MIN,
			.max_uV = AXP22_DCDC3_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT) && defined(CONFIG_ENABLE_INIT_VOLTAGE)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DCDC3_VALUE,
				.enabled = AXP_DCDC3_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC3_data),
		.consumer_supplies = DCDC3_data,
	},
	[vcc_DCDC4] = {
		.constraints = { 
			.name = "axp22_dcdc4",
			.min_uV = AXP22_DCDC4_MIN,
			.max_uV = AXP22_DCDC4_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DCDC4_VALUE,
				.enabled = AXP_DCDC4_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC4_data),
		.consumer_supplies = DCDC4_data,
	},
	[vcc_DCDC5] = {
		.constraints = { 
			.name = "axp22_dcdc5",
			.min_uV = AXP22_DCDC5_MIN,
			.max_uV = AXP22_DCDC5_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
#if defined (CONFIG_KP_OUTPUTINIT)
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				.uV = AXP_DCDC5_VALUE,
				.enabled = AXP_DCDC5_ENABLE,
			}
#endif
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC5_data),
		.consumer_supplies = DCDC5_data,
	},
	[vcc_ldoio0] = {
		.constraints = {
			.name = "axp22_ldoio0",
			.min_uV = AXP22_LDOIO0_MIN,
			.max_uV = AXP22_LDOIO0_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(ldoio0_data),
		.consumer_supplies = ldoio0_data,
	},
	[vcc_ldoio1] = {
		.constraints = {
			.name = "axp22_ldoio1",
			.min_uV = AXP22_LDOIO1_MIN,
			.max_uV = AXP22_LDOIO1_MAX,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(ldoio1_data),
		.consumer_supplies = ldoio1_data,
	},
};

static struct axp_funcdev_info axp_regldevs[] = {
	{
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO1,
		.platform_data = &axp_regl_init_data[vcc_ldo1],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO2,
		.platform_data = &axp_regl_init_data[vcc_ldo2],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO3,
		.platform_data = &axp_regl_init_data[vcc_ldo3],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO4,
		.platform_data = &axp_regl_init_data[vcc_ldo4],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO5,
		.platform_data = &axp_regl_init_data[vcc_ldo5],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO6,
		.platform_data = &axp_regl_init_data[vcc_ldo6],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO7,
		.platform_data = &axp_regl_init_data[vcc_ldo7],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO8,
		.platform_data = &axp_regl_init_data[vcc_ldo8],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO9,
		.platform_data = &axp_regl_init_data[vcc_ldo9],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO10,
		.platform_data = &axp_regl_init_data[vcc_ldo10],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO11,
		.platform_data = &axp_regl_init_data[vcc_ldo11],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDO12,
		.platform_data = &axp_regl_init_data[vcc_ldo12],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_DCDC1,
		.platform_data = &axp_regl_init_data[vcc_DCDC1],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_DCDC2,
		.platform_data = &axp_regl_init_data[vcc_DCDC2],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_DCDC3,
		.platform_data = &axp_regl_init_data[vcc_DCDC3],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_DCDC4,
		.platform_data = &axp_regl_init_data[vcc_DCDC4],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_DCDC5,
		.platform_data = &axp_regl_init_data[vcc_DCDC5],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDOIO0,
		.platform_data = &axp_regl_init_data[vcc_ldoio0],
	}, {
		.name = "axp22-regulator",
		.id = AXP22_ID_LDOIO1,
		.platform_data = &axp_regl_init_data[vcc_ldoio1],
	},
};

static struct power_supply_info battery_data ={
		.name ="PTI PL336078",
		.technology = POWER_SUPPLY_TECHNOLOGY_LION,
		.voltage_max_design = CHGVOL,
		.voltage_min_design = SHUTDOWNVOL,
		.energy_full_design = BATCAP,
		.use_for_apm = 1,
};


static struct axp_supply_init_data axp_sply_init_data = {
	.battery_info = &battery_data,
	.chgcur = STACHGCUR,
	.chgvol = CHGVOL,
	.chgend = ENDCHGRATE,
	.chgen = CHGEN,
	.sample_time = ADCFREQ,
	.chgpretime = CHGPRETIME,
	.chgcsttime = CHGCSTTIME,
};

static struct axp_funcdev_info axp_splydev[]={
   	{
   		.name = "axp22-supplyer",
		.id = AXP22_ID_SUPPLY,
		.platform_data = &axp_sply_init_data,
    },
};

static struct axp_funcdev_info axp_gpiodev[]={
   	{  
		.name = "axp22-gpio",
   		.id = AXP22_ID_GPIO,
    },
};

static struct axp_platform_data axp_pdata = {
	.num_regl_devs = ARRAY_SIZE(axp_regldevs),
	.num_sply_devs = ARRAY_SIZE(axp_splydev),
	.num_gpio_devs = ARRAY_SIZE(axp_gpiodev),
	.regl_devs = axp_regldevs,
	.sply_devs = axp_splydev,
	.gpio_devs = axp_gpiodev,
	.gpio_base = 0,
};


static struct i2c_board_info __initdata axp_mfd_i2c_board_info[] = {

	{
		.type = "axp22_mfd",
		.addr = AXP_DEVICES_ADDR,
		.platform_data = &axp_pdata,
		.irq = AXP_IRQNO,
	},
};

static int __init axp22_board_init(void)
{
        return i2c_register_board_info(AXP_I2CBUS, axp_mfd_i2c_board_info, ARRAY_SIZE(axp_mfd_i2c_board_info));
}
arch_initcall(axp22_board_init);

MODULE_DESCRIPTION("X-powers axp board");
MODULE_AUTHOR("Weijin Zhong");
MODULE_LICENSE("GPL");
