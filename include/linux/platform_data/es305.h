/*
 * include/linux/platform_data/es305.h - Audience ES305 Voice Processor driver
 *
 * Copyright (C) 2012 Google, Inc.
 * Copyright (C) 2012 Samsung Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ES305_H__
#define __ES305_H__

struct es305_platform_data {
	int gpio_wakeup;
	int gpio_reset;

	void (*clk_enable)(bool enable);

	/* PORT A = 1, B = 2, C = 3, D = 4 */
	int passthrough_src;
	int passthrough_dst;
};

#endif
