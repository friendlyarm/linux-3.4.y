/*
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#ifndef _LINUX_AS3668_H
#define _LINUX_AS3668_H

#define AS3668_VMON_VBAT_2_0V			0x00
#define AS3668_VMON_VBAT_3_0V			0x01
#define AS3668_VMON_VBAT_3_15V			0x02
#define AS3668_VMON_VBAT_3_3V			0x03

#define AS3668_SHUTDOWN_ENABLE_ON		1
#define AS3668_SHUTDOWN_ENABLE_OFF		0

#define AS3668_PATTERN_START_SOURCE_SW		0
#define AS3668_PATTERN_START_SOURCE_GPIO	1

#define AS3668_PWM_SOURCE_INTERNAL		0
#define AS3668_PWM_SOURCE_EXTERNAL		1

#define AS3668_GPIO_INPUT_NONINVERT		0
#define AS3668_GPIO_INPUT_INVERT		1

#define AS3668_GPIO_INPUT_MODE_ANALOG		0
#define AS3668_GPIO_INPUT_MODE_DIGITAL		1

#define AS3668_GPIO_MODE_INPUT_ONLY		0
#define AS3668_GPIO_MODE_OUTPUT			1

#define AS3668_AUDIO_CTRL_INPUT_GPIO		0
#define AS3668_AUDIO_CTRL_INPUT_CURR4		1

#define AS3668_AUDIO_CTRL_PLDN_ENABLE		0
#define AS3668_AUDIO_CTRL_PLDN_DISABLE		1

#define AS3668_AUDIO_CTRL_ADC_CHAR_250		0
#define AS3668_AUDIO_CTRL_ADC_CHAR_50		1

#define AS3668_AUDIO_INPUT_CAP_PRECHARGE	0
#define AS3668_AUDIO_INPUT_CAP_NO_PRECHARGE	1

#define AS3668_AUDIO_INPUT_AUTO_PRECHARGE	0
#define AS3668_AUDIO_INPUT_MANUAL_PRECHARGE	1

#define AS3668_RED				24
#define AS3668_GREEN				16
#define AS3668_BLUE				8
#define AS3668_WHITE				0

#define AS3668_LED_NUM				4

struct as3668_platform_data {
	u8 led_array[AS3668_LED_NUM];
	u16 vbat_monitor_voltage_index:2;
	u16 shutdown_enable:1;
	u16 pattern_start_source:1;
	u16 pwm_source:1;
	u16 gpio_input_invert:1;
	u16 gpio_input_mode:1;
	u16 gpio_mode:1;
	u16 audio_input_pin:1;
	u16 audio_pulldown_off:1;
	u16 audio_adc_characteristic:1;
	u16 audio_dis_start:1;
	u16 audio_man_start:1;
} __packed;
#endif
