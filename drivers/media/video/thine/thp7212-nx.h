
#ifndef MEDIA_THP7212_H
#define MEDIA_THP7212_H

#define THP7212_DRIVER_NAME	"THP7212NX"

#define CONFIG_VIDEO_THP7212_V_1_1 //many add

#include <mach/soc.h>

struct thp7212_platform_data {
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

extern int thp7212_init4(struct v4l2_subdev *sd);

/**
* struct thp7212_platform_data - platform data for M5MOLS driver
* @irq:   GPIO getting the irq pin of THP7212
* @gpio_rst:  GPIO driving the reset pin of THP7212
 * @enable_rst:	the pin state when reset pin is enabled
* @set_power:	an additional callback to a board setup code
 *		to be called after enabling and before disabling
*		the sensor device supply regulators
 */
#endif	/* MEDIA_S5K4EC_H */
