#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/spi/ads7846.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/amba/pl022.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#define DRVNAME "ads7846_device"

#ifdef CONFIG_MATRIX_ADS7846_MODULE
static struct spi_device *spi_device;
static void ads7846_device_spi_delete(struct spi_master *master, unsigned cs)
{
	struct device *dev;
	char str[32];

	snprintf(str, sizeof(str), "%s.%u", dev_name(&master->dev), cs);

	dev = bus_find_device_by_name(&spi_bus_type, NULL, str);
	if (dev) {
		pr_info(DRVNAME": Deleting %s\n", str);
		device_del(dev);
	}
}

static int ads7846_device_spi_device_register(struct spi_board_info *spi)
{
	struct spi_master *master;

	master = spi_busnum_to_master(spi->bus_num);
	if (!master) {
		pr_err(DRVNAME ":  spi_busnum_to_master(%d) returned NULL\n",
								spi->bus_num);
		return -EINVAL;
	}
	/* make sure it's available */
	ads7846_device_spi_delete(master, spi->chip_select);
	spi_device = spi_new_device(master, spi);
	put_device(&master->dev);
	if (!spi_device) {
		pr_err(DRVNAME ":    spi_new_device() returned NULL\n");
		return -EPERM;
	}
	return 0;
}
#else
static int ads7846_device_spi_device_register(struct spi_board_info *spi)
{
	return spi_register_board_info(spi, 1);
}
#endif

#define ADS_CS		(PAD_GPIO_B+30)
#define ADS_IRQ		(PAD_GPIO_B+31)

static int ads7846_get_pendown_state(void)
{
	return !gpio_get_value(ADS_IRQ);
}

static void spi0_ads7846_cs(u32 chipselect)
{
#if (CFG_SPI0_CS_GPIO_MODE)
	if (nxp_soc_gpio_get_io_func(ADS_CS) != nxp_soc_gpio_get_altnum(ADS_CS))
		nxp_soc_gpio_set_io_func(ADS_CS, nxp_soc_gpio_get_altnum(ADS_CS));

	nxp_soc_gpio_set_io_dir(ADS_CS, 1);
	nxp_soc_gpio_set_out_value(ADS_CS, chipselect);
#endif
}

struct pl022_config_chip spi0_ads7846_info = {
	/* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
	.com_mode = CFG_SPI0_COM_MODE,
	.iface = SSP_INTERFACE_MOTOROLA_SPI,
	/* We can only act as master but SSP_SLAVE is possible in theory */
	.hierarchy = SSP_MASTER,
	/* 0 = drive TX even as slave, 1 = do not drive TX as slave */
	.slave_tx_disable = 1,
	.rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
	.tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
	.ctrl_len = SSP_BITS_8,
	.wait_state = SSP_MWIRE_WAIT_ZERO,
	.duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
	/*
	 * This is where you insert a call to a function to enable CS
	 * (usually GPIO) for a certain chip.
	 */
#if (CFG_SPI0_CS_GPIO_MODE)
	.cs_control = spi0_ads7846_cs,
#endif
	.clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,
};

static const struct ads7846_platform_data ads7846_ts_info = {
	.model		= 7846,
	.x_min		= 100,
	.y_min		= 100,
	.x_max		= 0x0fff,
	.y_max		= 0x0fff,
	.vref_mv	= 3300,
	.x_plate_ohms	= 256,
	.penirq_recheck_delay_usecs = 10,
	.settle_delay_usecs = 100,
	.keep_vref_on	= 1,
	.pressure_max	= 1500,
	.debounce_max	= 10,
	.debounce_tol	= 30,
	.debounce_rep	= 1,
	.get_pendown_state	= ads7846_get_pendown_state,    
};

static struct spi_board_info ads7846_boardinfo __initdata = {
	.modalias        = "ads7846",    /* fixup */
	.irq			 = ADS_IRQ + IRQ_GPIO_START,
	.platform_data          = &ads7846_ts_info,
	.max_speed_hz    = 800 * 1000,  /* max spi clock (SCK) speed in HZ */
	.bus_num         = 0,           /* Note> set bus num, must be smaller than ARRAY_SIZE(spi_plat_device) */
	.chip_select     = 1,           /* Note> set chip select num, must be smaller than spi cs_num */
	.controller_data = &spi0_ads7846_info,
};
static int __init ads7846_dev_init(void)
{	
	ads7846_device_spi_device_register(&ads7846_boardinfo);
	return 0;
}

static void __exit ads7846_dev_exit(void)
{
	if (spi_device) {
		if (spi_device->master->cleanup) {
			spi_device->master->cleanup(spi_device);
		}	
		device_del(&spi_device->dev);
		kfree(spi_device);
	}
}

module_init(ads7846_dev_init);
module_exit(ads7846_dev_exit);
MODULE_DESCRIPTION("Add a ads7846 spi device.");
MODULE_AUTHOR("FriendlyARM");
MODULE_LICENSE("GPL");
