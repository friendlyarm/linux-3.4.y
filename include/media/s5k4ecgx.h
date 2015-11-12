/* include/media/s5k4ecgx.h
 *
 * Copyright (C) 2010, SAMSUNG ELECTRONICS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef MEDIA_S5K4EC_H
#define MEDIA_S5K4EC_H

#define S5K4ECGX_DRIVER_NAME	"S5K4ECGX"

#define CONFIG_VIDEO_S5K4ECGX_V_1_1 //many add

struct s5k4ecgx_platform_data {
	unsigned int default_width;
	unsigned int default_height;
	unsigned int pixelformat;
	int freq;	/* MCLK in Hz */

#if 1//many add
	/* This SoC supports Parallel & CSI-2 */
	int is_mipi;
#endif

#if 1 //many del//zhuxuezhen 2012-12-27 recovery
	int (*flash_onoff)(int);
	int (*af_assist_onoff)(int);
	int (*torch_onoff)(int);
#endif
	/* ISP interrupt */
	int (*config_isp_irq)(void);
	int irq;

	int (*set_power)(struct device *dev, int on);
	int	gpio_rst;
	bool	enable_rst;

};

extern int s5k4ecgx_init4(struct v4l2_subdev *sd);

/**
* struct s5k4ecgx_platform_data - platform data for M5MOLS driver
* @irq:   GPIO getting the irq pin of S5K4ECGX
* @gpio_rst:  GPIO driving the reset pin of S5K4ECGX
 * @enable_rst:	the pin state when reset pin is enabled
* @set_power:	an additional callback to a board setup code
 *		to be called after enabling and before disabling
*		the sensor device supply regulators
 */
#endif	/* MEDIA_S5K4EC_H */
