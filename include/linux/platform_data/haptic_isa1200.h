/*
 *  haptic_isa1200.h - ISA1200 Haptic Motor driver
 *
 * Copyright (C) 2012 Samsung Electronics Co. Ltd. All Rights Reserved.
 * Author: Vishnudev Ramakrishnan <vramakri@sta.samsung.com>
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

#ifndef _LINUX_HAPTIC_ISA1200_H
#define _LINUX_HAPTIC_ISA1200_H

struct isa1200_platform_data {
	int pwm_ch;
	int hap_en_gpio;
	int max_timeout;
};

#endif /* _LINUX_HAPTIC_ISA1200_H */
