
#ifndef MEDIA_TW9992_H
#define MEDIA_TW9992_H

#define TW9992_DRIVER_NAME	"tw9992"

struct tw9992_platform_data {
	unsigned int default_width;
	unsigned int default_height;
	unsigned int pixelformat;
	int freq;	/* MCLK in Hz */

	/* This SoC supports Parallel & CSI-2 */
	int is_mipi;

	int (*flash_onoff)(int);
	int (*af_assist_onoff)(int);
	int (*torch_onoff)(int);

	/* ISP interrupt */
	int (*config_isp_irq)(void);
	int irq;

	int (*set_power)(struct device *dev, int on);
	int	gpio_rst;
	bool	enable_rst;

};

extern int tw9992_init4(struct v4l2_subdev *sd);

/**
* struct tw9992_platform_data - platform data for M5MOLS driver
* @irq:   GPIO getting the irq pin of TW9992
* @gpio_rst:  GPIO driving the reset pin of TW9992
 * @enable_rst:	the pin state when reset pin is enabled
* @set_power:	an additional callback to a board setup code
 *		to be called after enabling and before disabling
*		the sensor device supply regulators
 */

#endif	/* MEDIA_TW9992_H */
