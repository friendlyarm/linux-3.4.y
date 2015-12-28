/*
 * arch/arm/plat-s5p4418/nanopi2/lcds.c
 *
 * Copyright (c) 2015 FriendlyARM (www.arm9.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>

#include <mach/platform.h>
#include <mach/soc.h>

#if defined(CONFIG_TOUCHSCREEN_GOODIX) || defined(CONFIG_TOUCHSCREEN_FT5X0X) || \
	defined(CONFIG_TOUCHSCREEN_1WIRE)
#include <linux/platform_data/ctouch.h>

static unsigned int ctp_type = CTP_AUTO;

static int __init nanopi2_init_ctp(char *str)
{
	unsigned int val;
	char *p = str, *end;

	val = simple_strtoul(p, &end, 10);
	if (end <= p) {
		return 1;
	}

//	if (val < CTP_MAX && nanopi2_lcd_config[lcd_idx].ctp) {
//		ctp_type = val;
//	} else if (val == CTP_NONE) {
		ctp_type = CTP_NONE;
//	}

	return 1;
}
__setup("ctp=", nanopi2_init_ctp);

unsigned int nanopi2_get_ctp(void)
{
	return ctp_type;
}
EXPORT_SYMBOL(nanopi2_get_ctp);

void nanopi2_set_ctp(int type)
{
	if (ctp_type == CTP_AUTO && type < CTP_MAX) {
		ctp_type = type;
	}
}
EXPORT_SYMBOL(nanopi2_set_ctp);
#endif

