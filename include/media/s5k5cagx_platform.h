/* linux/include/media/s5k4eagx_platform.h
 *
 * Copyright (c) 2010 Hardkernel Co., Ltd.
 * 		http://www.hardkernel.com/
 *
 * Driver for S5K4EAGX from Samsung Electronics
 * CMOS Image Sensor SoC with an Embedded Image Processor
 * supporting PVI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

struct s5k5cagx_platform_data {
	unsigned int default_width;
	unsigned int default_height;
	unsigned int pixelformat;
	int freq;	/* MCLK in KHz */

	/* This SoC supports Parallel & CSI-2 */
	int is_mipi;
};

