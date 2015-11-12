/* linux/arch/arm/mach-exynos/setup-fimc-is.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * FIMC-IS gpio and clock configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <mach/regs-gpio.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <plat/clock.h>
#include <plat/gpio-cfg.h>
#include <plat/map-s5p.h>
#include <plat/cpu.h>
#include <media/exynos_fimc_is.h>

/*#define USE_UART_DEBUG*/

struct platform_device; /* don't need the contents */

/*------------------------------------------------------*/
/*		Exynos5 series - FIMC-IS		*/
/*------------------------------------------------------*/
static int cfg_gpio(struct gpio_set *gpio, int value)
{
	int ret;

	pr_debug("gpio.pin:%d gpio.name:%s\n", gpio->pin, gpio->name);
	ret = gpio_request(gpio->pin, gpio->name);
	if (ret) {
		pr_err("Request GPIO error(%s)\n", gpio->name);
		return -1;
	}

	switch (gpio->act) {
	case GPIO_PULL_NONE:
		s3c_gpio_cfgpin(gpio->pin, value);
		s3c_gpio_setpull(gpio->pin, S3C_GPIO_PULL_NONE);
		break;
	case GPIO_OUTPUT:
		s3c_gpio_cfgpin(gpio->pin, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(gpio->pin, S3C_GPIO_PULL_NONE);
		gpio_set_value(gpio->pin, value);
		break;
	case GPIO_RESET:
		s3c_gpio_setpull(gpio->pin, S3C_GPIO_PULL_NONE);
		gpio_direction_output(gpio->pin, value);
		break;
	default:
		pr_err("unknown act for gpio\n");
		return -1;
	}
	gpio_free(gpio->pin);

	return 0;

}

static int power_control_sensor(char *regulator_name, int on)
{
	struct regulator *regulator = NULL;

	pr_debug("regulator:%s on:%d\n", regulator_name, on);
	regulator = regulator_get(NULL, regulator_name);
	if (IS_ERR(regulator)) {
		pr_err("%s : regulator_get fail\n", __func__);
		return PTR_ERR(regulator);
	}

	if (on)
		regulator_enable(regulator);
	else{
		if (regulator_is_enabled(regulator))
			regulator_disable(regulator);
	}

	regulator_put(regulator);
	return 0;
}

int exynos5_fimc_is_cfg_clk(struct platform_device *pdev)
{
	struct clk *aclk_mcuisp = NULL;
	struct clk *aclk_266 = NULL;
	struct clk *aclk_mcuisp_div0 = NULL;
	struct clk *aclk_mcuisp_div1 = NULL;
	struct clk *aclk_266_div0 = NULL;
	struct clk *aclk_266_div1 = NULL;
	struct clk *aclk_266_mpwm = NULL;
#ifdef USE_UART_DEBUG
	struct clk *sclk_uart_isp = NULL;
	struct clk *sclk_uart_isp_div = NULL;
#endif
	struct clk *mout_mpll = NULL;
	struct clk *sclk_mipi0 = NULL;
	struct clk *sclk_mipi1 = NULL;
	struct clk *cam_src = NULL;
	struct clk *cam_A_clk = NULL;
	unsigned long mcu_isp_400;
	unsigned long isp_266;
	unsigned long isp_uart;
	unsigned long mipi;
	unsigned long xxti;

	/* 1. MCUISP */
	aclk_mcuisp = clk_get(&pdev->dev, "aclk_400_isp");
	if (IS_ERR(aclk_mcuisp))
		return PTR_ERR(aclk_mcuisp);

	aclk_mcuisp_div0 = clk_get(&pdev->dev, "aclk_400_isp_div0");
	if (IS_ERR(aclk_mcuisp_div0)) {
		clk_put(aclk_mcuisp);
		return PTR_ERR(aclk_mcuisp_div0);
	}

	aclk_mcuisp_div1 = clk_get(&pdev->dev, "aclk_400_isp_div1");
	if (IS_ERR(aclk_mcuisp_div1)) {
		clk_put(aclk_mcuisp);
		clk_put(aclk_mcuisp_div0);
		return PTR_ERR(aclk_mcuisp_div1);
	}

	clk_set_rate(aclk_mcuisp_div0, 200 * 1000000);
	clk_set_rate(aclk_mcuisp_div1, 100 * 1000000);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp);
	pr_debug("mcu_isp_400 : %ld\n", mcu_isp_400);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp_div0);
	pr_debug("mcu_isp_400_div0 : %ld\n", mcu_isp_400);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp_div1);
	pr_debug("aclk_mcuisp_div1 : %ld\n", mcu_isp_400);

	clk_put(aclk_mcuisp);
	clk_put(aclk_mcuisp_div0);
	clk_put(aclk_mcuisp_div1);

	/* 2. ACLK_ISP */
	aclk_266 = clk_get(&pdev->dev, "aclk_266_isp");
	if (IS_ERR(aclk_266))
		return PTR_ERR(aclk_266);

	aclk_266_div0 = clk_get(&pdev->dev, "aclk_266_isp_div0");
	if (IS_ERR(aclk_266_div0)) {
		clk_put(aclk_266);
		return PTR_ERR(aclk_266_div0);
	}

	aclk_266_div1 = clk_get(&pdev->dev, "aclk_266_isp_div1");
	if (IS_ERR(aclk_266_div1)) {
		clk_put(aclk_266);
		clk_put(aclk_266_div0);
		return PTR_ERR(aclk_266_div1);
	}

	aclk_266_mpwm = clk_get(&pdev->dev, "aclk_266_isp_divmpwm");
	if (IS_ERR(aclk_266_mpwm)) {
		clk_put(aclk_266);
		clk_put(aclk_266_div0);
		clk_put(aclk_266_div1);
		return PTR_ERR(aclk_266_mpwm);
	}

	clk_set_rate(aclk_266_div0, 134 * 1000000);
	clk_set_rate(aclk_266_div1, 68 * 1000000);
	clk_set_rate(aclk_266_mpwm, 34 * 1000000);

	isp_266 = clk_get_rate(aclk_266);
	pr_debug("isp_266 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_div0);
	pr_debug("isp_266_div0 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_div1);
	pr_debug("isp_266_div1 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_mpwm);
	pr_debug("isp_266_mpwm : %ld\n", isp_266);

	clk_put(aclk_266);
	clk_put(aclk_266_div0);
	clk_put(aclk_266_div1);
	clk_put(aclk_266_mpwm);

#ifdef USE_UART_DEBUG
	/* 3. UART-ISP */
	sclk_uart_isp = clk_get(&pdev->dev, "sclk_uart_src_isp");
	if (IS_ERR(sclk_uart_isp))
		return PTR_ERR(sclk_uart_isp);

	sclk_uart_isp_div = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(sclk_uart_isp_div)) {
		clk_put(sclk_uart_isp);
		return PTR_ERR(sclk_uart_isp_div);
	}

	clk_set_parent(sclk_uart_isp, clk_get(&pdev->dev, "mout_mpll_user"));
	clk_set_parent(sclk_uart_isp_div, sclk_uart_isp);
	clk_set_rate(sclk_uart_isp_div, 50 * 1000000);

	isp_uart = clk_get_rate(sclk_uart_isp);
	pr_debug("isp_uart : %ld\n", isp_uart);
	isp_uart = clk_get_rate(sclk_uart_isp_div);
	pr_debug("isp_uart_div : %ld\n", isp_uart);

	clk_put(sclk_uart_isp);
	clk_put(sclk_uart_isp_div);
#endif

	/* 4. MIPI-CSI */
	mout_mpll = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(mout_mpll))
		return PTR_ERR(mout_mpll);

	sclk_mipi0 = clk_get(&pdev->dev, "sclk_gscl_wrap0");
	if (IS_ERR(sclk_mipi0)) {
		clk_put(mout_mpll);
		return PTR_ERR(sclk_mipi0);
	}

	clk_set_parent(sclk_mipi0, mout_mpll);
	clk_set_rate(sclk_mipi0, 267 * 1000000);

	clk_put(mout_mpll);
	clk_put(sclk_mipi0);

	mout_mpll = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(mout_mpll))
		return PTR_ERR(mout_mpll);

	sclk_mipi1 = clk_get(&pdev->dev, "sclk_gscl_wrap1");
	if (IS_ERR(sclk_mipi1)) {
		clk_put(mout_mpll);
		return PTR_ERR(sclk_mipi1);
	}

	clk_set_parent(sclk_mipi1, mout_mpll);
	clk_set_rate(sclk_mipi1, 267 * 1000000);

	mipi = clk_get_rate(mout_mpll);
	pr_debug("mipi_src : %ld\n", mipi);
	mipi = clk_get_rate(sclk_mipi1);
	pr_debug("mipi_div : %ld\n", mipi);

	clk_put(mout_mpll);
	clk_put(sclk_mipi1);

	/* 5. Camera A */
	cam_src = clk_get(&pdev->dev, "xxti");
	if (IS_ERR(cam_src))
		return PTR_ERR(cam_src);

	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk)) {
		clk_put(cam_src);
		return PTR_ERR(cam_A_clk);
	}

	xxti = clk_get_rate(cam_src);
	pr_debug("xxti : %ld\n", xxti);

	clk_set_parent(cam_A_clk, cam_src);
	clk_set_rate(cam_A_clk, 24 * 1000000);

	clk_put(cam_src);
	clk_put(cam_A_clk);

	/* 6. Camera B */
	cam_src = clk_get(&pdev->dev, "xxti");
	if (IS_ERR(cam_src))
		return PTR_ERR(cam_src);

	cam_A_clk = clk_get(&pdev->dev, "sclk_bayer");
	if (IS_ERR(cam_A_clk)) {
		clk_put(cam_src);
		return PTR_ERR(cam_A_clk);
	}

	xxti = clk_get_rate(cam_src);
	pr_debug("xxti : %ld\n", xxti);

	clk_set_parent(cam_A_clk, cam_src);
	clk_set_rate(cam_A_clk, 24 * 1000000);

	clk_put(cam_src);
	clk_put(cam_A_clk);

	return 0;
}

int exynos5_fimc_is_clk_on(struct platform_device *pdev, int sensor_id)
{
	struct clk *gsc_ctrl = NULL;
	struct clk *isp_ctrl = NULL;
	struct clk *mipi_ctrl = NULL;
	struct clk *cam_if_top = NULL;
	struct clk *cam_clk = NULL;
	struct clk *isp_400_src = NULL;
	struct clk *isp_266_src = NULL;
	struct clk *isp_400_clk = NULL;
	struct clk *isp_266_clk = NULL;
	struct exynos5_platform_fimc_is *dev = pdev->dev.platform_data;
	struct exynos5_fimc_is_sensor_info *sensor =
						dev->sensor_info[sensor_id];

	gsc_ctrl = clk_get(&pdev->dev, "gscl");
	if (IS_ERR(gsc_ctrl))
		return PTR_ERR(gsc_ctrl);

	clk_enable(gsc_ctrl);
	clk_put(gsc_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp0");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_enable(isp_ctrl);
	clk_put(isp_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp1");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_enable(isp_ctrl);
	clk_put(isp_ctrl);

	cam_if_top = clk_get(&pdev->dev, "camif_top");
	if (IS_ERR(cam_if_top))
		return PTR_ERR(cam_if_top);

	clk_enable(cam_if_top);
	clk_put(cam_if_top);

	if (sensor->csi_id == CSI_ID_A) {
		mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap0");
		if (IS_ERR(mipi_ctrl))
			return PTR_ERR(mipi_ctrl);

		clk_enable(mipi_ctrl);
		clk_put(mipi_ctrl);

		cam_clk = clk_get(&pdev->dev, "sclk_cam0");
		if (IS_ERR(cam_clk))
			return PTR_ERR(cam_clk);

		clk_enable(cam_clk);
		clk_put(cam_clk);
	}

	if (sensor->csi_id == CSI_ID_B) {
		mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap1");
		if (IS_ERR(mipi_ctrl))
			return PTR_ERR(mipi_ctrl);

		clk_enable(mipi_ctrl);
		clk_put(mipi_ctrl);

		cam_clk = clk_get(&pdev->dev, "sclk_bayer");
		if (IS_ERR(cam_clk))
			return PTR_ERR(cam_clk);

		clk_enable(cam_clk);
		clk_put(cam_clk);
	}

	/*isp sub src selection*/
	isp_400_src = clk_get(&pdev->dev, "aclk_400_isp");
	if (IS_ERR(isp_400_src))
		return PTR_ERR(isp_400_src);

	isp_266_src = clk_get(&pdev->dev, "aclk_266_isp");
	if (IS_ERR(isp_266_src)) {
		clk_put(isp_400_src);
		return PTR_ERR(isp_266_src);
	}

	isp_400_clk = clk_get(&pdev->dev, "dout_aclk_400_isp");
	if (IS_ERR(isp_400_clk)) {
		clk_put(isp_400_src);
		clk_put(isp_266_src);
		return PTR_ERR(isp_400_clk);
	}

	isp_266_clk = clk_get(&pdev->dev, "aclk_266");
	if (IS_ERR(isp_266_clk)) {
		clk_put(isp_400_src);
		clk_put(isp_266_src);
		clk_put(isp_400_clk);
		return PTR_ERR(isp_266_clk);
	}

	clk_set_parent(isp_400_src, isp_400_clk);
	clk_set_parent(isp_266_src, isp_266_clk);

	clk_put(isp_400_src);
	clk_put(isp_266_src);
	clk_put(isp_400_clk);
	clk_put(isp_266_clk);

	return 0;
}

int exynos5_fimc_is_clk_off(struct platform_device *pdev, int sensor_id)
{
	struct clk *gsc_ctrl = NULL;
	struct clk *isp_ctrl = NULL;
	struct clk *mipi_ctrl = NULL;
	struct clk *cam_if_top = NULL;
	struct clk *cam_clk = NULL;
	struct clk *isp_400_src = NULL;
	struct clk *isp_266_src = NULL;
	struct clk *xtal_clk = NULL;
	struct exynos5_platform_fimc_is *dev = pdev->dev.platform_data;
	struct exynos5_fimc_is_sensor_info *sensor =
						dev->sensor_info[sensor_id];

	if (sensor->csi_id == CSI_ID_A) {
		cam_clk = clk_get(&pdev->dev, "sclk_cam0");
		if (IS_ERR(cam_clk))
			return PTR_ERR(cam_clk);

		clk_disable(cam_clk);
		clk_put(cam_clk);

		mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap0");
		if (IS_ERR(mipi_ctrl))
			return PTR_ERR(mipi_ctrl);

		clk_disable(mipi_ctrl);
		clk_put(mipi_ctrl);
	}

	if (sensor->csi_id == CSI_ID_B) {
		cam_clk = clk_get(&pdev->dev, "sclk_bayer");
		if (IS_ERR(cam_clk))
			return PTR_ERR(cam_clk);

		clk_disable(cam_clk);
		clk_put(cam_clk);

		mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap1");
		if (IS_ERR(mipi_ctrl))
			return PTR_ERR(mipi_ctrl);

		clk_disable(mipi_ctrl);
		clk_put(mipi_ctrl);
	}

	cam_if_top = clk_get(&pdev->dev, "camif_top");
	if (IS_ERR(cam_if_top))
		return PTR_ERR(cam_if_top);

	clk_disable(cam_if_top);
	clk_put(cam_if_top);

	isp_ctrl = clk_get(&pdev->dev, "isp0");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_disable(isp_ctrl);
	clk_put(isp_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp1");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_disable(isp_ctrl);
	clk_put(isp_ctrl);

	gsc_ctrl = clk_get(&pdev->dev, "gscl");
	if (IS_ERR(gsc_ctrl))
		return PTR_ERR(gsc_ctrl);

	clk_disable(gsc_ctrl);
	clk_put(gsc_ctrl);

	/*isp sub src selection*/
	isp_400_src = clk_get(&pdev->dev, "aclk_400_isp");
	if (IS_ERR(isp_400_src))
		return PTR_ERR(isp_400_src);

	isp_266_src = clk_get(&pdev->dev, "aclk_266_isp");
	if (IS_ERR(isp_266_src)) {
		clk_put(isp_400_src);
		return PTR_ERR(isp_266_src);
	}

	xtal_clk = clk_get(&pdev->dev, "ext_xtal");
	if (IS_ERR(xtal_clk)) {
		clk_put(isp_400_src);
		clk_put(isp_266_src);
		return PTR_ERR(xtal_clk);
	}

	clk_set_parent(isp_400_src, xtal_clk);
	clk_set_parent(isp_266_src, xtal_clk);

	clk_put(isp_400_src);
	clk_put(isp_266_src);
	clk_put(xtal_clk);

	return 0;
}

/* sequence is important, don't change order */
int exynos5_fimc_is_sensor_power_on(struct platform_device *pdev,
						int sensor_id)
{

	struct exynos5_platform_fimc_is *dev = pdev->dev.platform_data;
	struct exynos5_fimc_is_sensor_info *sensor =
						dev->sensor_info[sensor_id];
	int i;

	pr_debug("exynos5_fimc_is_sensor_power_on(%d)\n",
					sensor_id);
	switch (sensor->sensor_id) {
	case SENSOR_NAME_S5K4E5:
		if (sensor->sensor_gpio.reset_peer.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_peer, 0)))
				goto error_sensor_power_on;

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 0)))
				goto error_sensor_power_on;

		if (sensor->sensor_power.cam_core)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_core, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_gpio.power.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.power, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_io_myself)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_myself, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_io_peer)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_peer, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		for (i = 0; i < FIMC_IS_MAX_GPIO_NUM; i++) {
			if (!sensor->sensor_gpio.cfg[i].pin)
				continue;
			if (IS_ERR_VALUE(cfg_gpio(&sensor->sensor_gpio.cfg[i],
					sensor->sensor_gpio.cfg[i].value)))
				goto error_sensor_power_on;
		}

		if (sensor->sensor_power.cam_af)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_af, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 1)))
				goto error_sensor_power_on;
		usleep_range(10, 100);

		break;

	case SENSOR_NAME_S5K6A3:
		if (sensor->sensor_gpio.reset_peer.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_peer, 0)))
				goto error_sensor_power_on;

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 0)))
				goto error_sensor_power_on;

		if (sensor->sensor_power.cam_core)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_core, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		for (i = 0; i < FIMC_IS_MAX_GPIO_NUM; i++) {
			if (!sensor->sensor_gpio.cfg[i].pin)
				continue;
			if (IS_ERR_VALUE(cfg_gpio(&sensor->sensor_gpio.cfg[i],
					sensor->sensor_gpio.cfg[i].value)))
				goto error_sensor_power_on;
		}

		if (sensor->sensor_gpio.power.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.power, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_io_myself)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_myself, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_io_peer)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_peer, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_gpio.reset_peer.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_peer, 1)))
				goto error_sensor_power_on;
		usleep_range(1200, 2000); /* must stay here more than 1msec */

		if (sensor->sensor_gpio.reset_peer.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_peer, 0)))
				goto error_sensor_power_on;
		usleep_range(1000, 1500);

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 1)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 0)))
				goto error_sensor_power_on;
		usleep_range(500, 1000);

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 1)))
				goto error_sensor_power_on;
		usleep_range(10, 100);

		break;
	default:
		pr_err("Bad camera senosr ID(%d)",
				sensor->sensor_id);
		goto error_sensor_power_on;
	}
	return 0;

error_sensor_power_on:
	return -1;

}

/* sequence is important, don't change order */
int exynos5_fimc_is_sensor_power_off(struct platform_device *pdev,
						int sensor_id)
{
	struct exynos5_platform_fimc_is *dev = pdev->dev.platform_data;
	struct exynos5_fimc_is_sensor_info *sensor
					= dev->sensor_info[sensor_id];

	pr_debug("exynos5_fimc_is_sensor_power_off(%d)\n", sensor_id);

	switch (sensor->sensor_id) {
	case SENSOR_NAME_S5K4E5:
		if (sensor->sensor_gpio.reset_peer.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_peer, 0)))
				goto error_sensor_power_off;

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 0)))
				goto error_sensor_power_off;

		if (sensor->sensor_gpio.power.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.power, 0)))
				goto error_sensor_power_off;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_core)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_core, 0)))
				goto error_sensor_power_off;

		usleep_range(500, 1000);
		if (sensor->sensor_power.cam_io_myself)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_myself, 0)))
				goto error_sensor_power_off;

		usleep_range(500, 1000);
			if (sensor->sensor_power.cam_io_peer)
				if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_peer, 0)))
					goto error_sensor_power_off;

		usleep_range(500, 1000);
		if (sensor->sensor_power.cam_af)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_af, 0)))
				goto error_sensor_power_off;

		usleep_range(500, 1000);
		break;

	case SENSOR_NAME_S5K6A3:
		if (sensor->sensor_gpio.reset_peer.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_peer, 0)))
				goto error_sensor_power_off;

		if (sensor->sensor_gpio.reset_myself.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.reset_myself, 0)))
				goto error_sensor_power_off;

		if (sensor->sensor_gpio.power.pin)
			if (IS_ERR_VALUE(cfg_gpio(
					&sensor->sensor_gpio.power, 0)))
				goto error_sensor_power_off;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_core)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_core, 0)))
				goto error_sensor_power_off;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_io_myself)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_myself, 0)))
				goto error_sensor_power_off;
		usleep_range(500, 1000);

		if (sensor->sensor_power.cam_io_peer)
			if (IS_ERR_VALUE(power_control_sensor(
					sensor->sensor_power.cam_io_peer, 0)))
				goto error_sensor_power_off;
		usleep_range(500, 1000);
		break;
	default:
		pr_err("Bad camera senosr ID(%d)",
				sensor->sensor_id);
		goto error_sensor_power_off;
	}
	return 0;

error_sensor_power_off:
	return -1;

}
