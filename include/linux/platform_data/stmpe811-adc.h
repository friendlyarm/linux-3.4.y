/*
 * stmpe811-adc.h
 *
 * Copyright (C) 2012 Samsung Electronics
 * SangYoung Son <hello.son@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __STMPE811_ADC_H_
#define __STMPE811_ADC_H_

struct stmpe811_callbacks {
	int (*get_adc_data)(u8 channel);
};

struct stmpe811_platform_data {
	void (*register_cb)(struct stmpe811_callbacks *);
};

#endif
