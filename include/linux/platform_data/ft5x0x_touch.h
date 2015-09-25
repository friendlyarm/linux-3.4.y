/*
 * include/linux/platform_data/ft5x0x_touch.h
 *
 * Copyright (C) 2015 FriendlyARM (www.arm9.net)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __PLAT_FT5X0X_TOUCH_H__
#define __PLAT_FT5X0X_TOUCH_H__


struct ft5x0x_i2c_platform_data {
	uint32_t gpio_irq;			// IRQ port
	uint32_t irq_cfg;

	uint32_t gpio_wakeup;		// Wakeup support
	uint32_t wakeup_cfg;

	uint32_t gpio_reset;		// Reset support
	uint32_t reset_cfg;

	int screen_max_x;
	int screen_max_y;
	int pressure_max;
};

#endif	// __PLAT_FT5X0X_TOUCH_H__

