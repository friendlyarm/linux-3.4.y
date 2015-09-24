#include <linux/init.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

static struct device *gps_dev;

static struct class *gps_class;

#define GPIO_GPS_PWR_EN  EXYNOS5_GPE1(0)
#define GPIO_GPS_nRST	 EXYNOS5_GPE0(6)
#define GPIO_GPS_CTS	EXYNOS5_GPD0(2)
#define GPIO_GPS_RTS	EXYNOS5_GPD0(3)
#define GPIO_GPS_RXD	EXYNOS5_GPD0(0)
#define GPIO_GPS_TXD	EXYNOS5_GPD0(1)

#define GPIO_GPS_CTS_AF   2
#define GPIO_GPS_RTS_AF   2
#define GPIO_GPS_RXD_AF   2
#define GPIO_GPS_TXD_AF   2

void __init exynos5_manta_gps_init(void)
{
	int n_rst_pin = 0;
	int n_rst_nc_pin = 0;
	gps_class = class_create(THIS_MODULE, "gps");
	if (IS_ERR(gps_class)) {
		pr_err("Failed to create class(sec)!\n");
		return;
	}
	BUG_ON(!gps_class);
	gps_dev = device_create(gps_class, NULL, 0, NULL, "bcm475x");
	BUG_ON(!gps_dev);
	s3c_gpio_cfgpin(GPIO_GPS_RXD, S3C_GPIO_SFN(GPIO_GPS_RXD_AF));
	s3c_gpio_setpull(GPIO_GPS_RXD, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(GPIO_GPS_TXD, S3C_GPIO_SFN(GPIO_GPS_TXD_AF));
	s3c_gpio_setpull(GPIO_GPS_TXD, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_GPS_CTS, S3C_GPIO_SFN(GPIO_GPS_CTS_AF));
	s3c_gpio_setpull(GPIO_GPS_CTS, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_GPS_RTS, S3C_GPIO_SFN(GPIO_GPS_RTS_AF));
	s3c_gpio_setpull(GPIO_GPS_RTS, S3C_GPIO_PULL_NONE);

	n_rst_pin = GPIO_GPS_nRST;
	n_rst_nc_pin = 0;

	if (gpio_request(n_rst_pin, "GPS_nRST"))
		WARN(1, "fail to request gpio (GPS_nRST)\n");

	s3c_gpio_setpull(n_rst_pin, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(n_rst_pin, S3C_GPIO_OUTPUT);
	gpio_direction_output(n_rst_pin, 1);

	if (gpio_request(GPIO_GPS_PWR_EN, "GPS_PWR_EN"))
		WARN(1, "fail to request gpio (GPS_PWR_EN)\n");

	s3c_gpio_setpull(GPIO_GPS_PWR_EN, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_GPS_PWR_EN, S3C_GPIO_OUTPUT);
	gpio_direction_output(GPIO_GPS_PWR_EN, 0);
	gpio_export(n_rst_pin, 1);
	gpio_export(GPIO_GPS_PWR_EN, 1);

	gpio_export_link(gps_dev, "GPS_nRST", n_rst_pin);
	gpio_export_link(gps_dev, "GPS_PWR_EN", GPIO_GPS_PWR_EN);
}
