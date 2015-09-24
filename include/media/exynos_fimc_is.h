/* linux/arch/arm/plat-s5p/include/plat/fimc_is.h
 *
 * Copyright (C) 2011 Samsung Electronics, Co. Ltd
 *
 * Exynos 4 series FIMC-IS slave device support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef EXYNOS_FIMC_IS_H_
#define EXYNOS_FIMC_IS_H_ __FILE__

#include <linux/videodev2.h>

#if defined CONFIG_VIDEO_EXYNOS5_FIMC_IS
#define FIMC_IS_MODULE_NAME			"exynos5-fimc-is"
#elif defined CONFIG_VIDEO_EXYNOS5_FIMC_IS2
#define FIMC_IS_MODULE_NAME			"exynos5-fimc-is2"
#endif

#define FIMC_IS_MAX_CAMIF_CLIENTS	2
#define FIMC_IS_MAX_NAME_LEN		32
#define FIMC_IS_MAX_GPIO_NUM		32
#define UART_ISP_SEL			0
#define UART_ISP_RATIO			1

#define to_fimc_is_plat(d)	(to_platform_device(d)->dev.platform_data)

enum exynos5_csi_id {
	CSI_ID_A = 0,
	CSI_ID_B
};

enum exynos5_flite_id {
	FLITE_ID_A = 0,
	FLITE_ID_B
};

enum exynos5_sensor_position {
	SENSOR_POSITION_REAR = 0,
	SENSOR_POSITION_FRONT
};
enum exynos5_sensor_id {
	SENSOR_NAME_S5K3H2	 = 1,
	SENSOR_NAME_S5K6A3	 = 2,
	SENSOR_NAME_S5K4E5	 = 3,
	SENSOR_NAME_S5K3H7	 = 4,
	SENSOR_NAME_CUSTOM	= 100,
	SENSOR_NAME_END
};

struct exynos5_sensor_power_info {
	char cam_core[FIMC_IS_MAX_NAME_LEN];
	char cam_io_myself[FIMC_IS_MAX_NAME_LEN];
	char cam_io_peer[FIMC_IS_MAX_NAME_LEN];
	char cam_af[FIMC_IS_MAX_NAME_LEN];
};

enum actuator_name {
	ACTUATOR_NAME_AD5823	= 1,
	ACTUATOR_NAME_DWXXXX	= 2,
	ACTUATOR_NAME_AK7343	= 3,
	ACTUATOR_NAME_HYBRIDVCA	= 4,
	ACTUATOR_NAME_NOTHING	= 100,
	ACTUATOR_NAME_END
};

enum flash_drv_name {
	FLADRV_NAME_KTD267	= 1,
	FLADRV_NAME_NOTHING	= 100,
	FLADRV_NAME_END
};

enum from_name {
	FROMDRV_NAME_W25Q80BW	= 1,
	FROMDRV_NAME_NOTHING
};

enum sensor_peri_type {
	SE_I2C,
	SE_SPI,
	SE_GPIO,
	SE_MPWM,
	SE_ADC,
	SE_NULL
};

struct i2c_type {
	u32 channel;
	u32 slave_address;
	u32 speed;
};

struct spi_type {
	u32 channel;
};

struct gpio_type {
	u32 first_gpio_port_no;
	u32 second_gpio_port_no;
};

union sensor_peri_format {
	struct i2c_type i2c;
	struct spi_type spi;
	struct gpio_type gpio;
};

struct sensor_protocol {
	u32 product_name;
	enum sensor_peri_type peri_type;
	union sensor_peri_format peri_setting;
};

enum exynos5_sensor_channel {
	SENSOR_CONTROL_I2C0	 = 0,
	SENSOR_CONTROL_I2C1	 = 1
};

enum gpio_act {
	GPIO_PULL_NONE = 0,
	GPIO_OUTPUT,
	GPIO_RESET
};

struct gpio_set {
	unsigned int pin;
	char name[FIMC_IS_MAX_NAME_LEN];
	unsigned int value;
	unsigned int act;
};

struct exynos5_sensor_gpio_info {
	struct gpio_set cfg[FIMC_IS_MAX_GPIO_NUM];
	struct gpio_set reset_myself;
	struct gpio_set reset_peer;
	struct gpio_set power;
};

struct platform_device;

 /**
  * struct exynos5_fimc_is_sensor_info	- image sensor information required for host
  *			       interace configuration.
 */
struct exynos5_fimc_is_sensor_info {
	char sensor_name[FIMC_IS_MAX_NAME_LEN];
	enum exynos5_sensor_position sensor_position;
	enum exynos5_sensor_id sensor_id;
	enum exynos5_csi_id csi_id;
	enum exynos5_flite_id flite_id;
	enum exynos5_sensor_channel i2c_channel;
	struct exynos5_sensor_power_info sensor_power;
	struct exynos5_sensor_gpio_info sensor_gpio;

	int max_width;
	int max_height;
	int max_frame_rate;

	int mipi_lanes;     /* MIPI data lanes */
	int mipi_settle;    /* MIPI settle */
	int mipi_align;     /* MIPI data align: 24/32 */
};

struct sensor_open_extended {
	struct sensor_protocol actuator_con;
	struct sensor_protocol flash_con;
	struct sensor_protocol from_con;

	u32 mclk;
	u32 mipi_lane_num;
	u32 mipi_speed;
	/* Skip setfile loading when fast_open_sensor is not 0 */
	u32 fast_open_sensor;
	/* Activatiing sensor self calibration mode (6A3) */
	u32 self_calibration_mode;
};

/**
* struct exynos5_platform_fimc_is - camera host interface platform data
*
* @isp_info: properties of camera sensor required for host interface setup
*/
struct exynos5_platform_fimc_is {
	int	 hw_ver;
	struct exynos5_fimc_is_sensor_info
		*sensor_info[FIMC_IS_MAX_CAMIF_CLIENTS];
	int	 (*clk_cfg)(struct platform_device *pdev);
	int	 (*clk_on)(struct platform_device *pdev, int sensor_id);
	int	 (*clk_off)(struct platform_device *pdev, int sensor_id);
	int	 (*sensor_power_on)(struct platform_device *pdev,
							int sensor_id);
	int	 (*sensor_power_off)(struct platform_device *pdev,
							int sensor_id);
};

extern void exynos5_fimc_is_set_platdata(struct exynos5_platform_fimc_is *pd);

/* platform specific clock functions */
extern int exynos5_fimc_is_cfg_clk(struct platform_device *pdev);
extern int exynos5_fimc_is_clk_on(struct platform_device *pdev, int sensor_id);
extern int exynos5_fimc_is_clk_off(struct platform_device *pdev, int sensor_id);
extern int exynos5_fimc_is_sensor_power_on(struct platform_device *pdev,
							int sensor_id);
extern int exynos5_fimc_is_sensor_power_off(struct platform_device *pdev,
							int sensor_id);

#endif /* EXYNOS_FIMC_IS_H_ */
