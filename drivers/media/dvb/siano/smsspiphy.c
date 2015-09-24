#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/dma-mapping.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include "smscoreapi.h"

#include <linux/gpio.h>

/************************************************************/
/*Platform specific defaults - can be changes by parameters */
/*or in compilation at this section                         */
/************************************************************/

/*Host GPIO pin used for SMS interrupt*/
#define HOST_INTERRUPT_PIN 	135

/*Host SPI bus number used for SMS*/
#define HOST_SPI_BUS_NUM	0

/*Host SPI CS used by SPI bus*/
#define HOST_SPI_CS_NUM		0


/*Maximum SPI speed during download (may be lower tha active state)*/
#define MAX_SPEED_DURING_DOWNLOAD	6000000

/*Maximum SPI speed during active state*/
#define MAX_SPEED_DURING_WORK		6000000	

/*Default SMS device type connected to SPI bus.*/
#define DEFAULT_SMS_DEVICE_TYPE		 SMS_RIO	

/*************************************/
/*End of platform specific parameters*/ 
/*************************************/


#define SPI_PACKET_SIZE 		256	

int host_spi_intr_pin = HOST_INTERRUPT_PIN;
int host_spi_bus = HOST_SPI_BUS_NUM;
int host_spi_cs = HOST_SPI_CS_NUM;
int spi_max_speed = MAX_SPEED_DURING_WORK;
int spi_default_type =DEFAULT_SMS_DEVICE_TYPE;

struct sms_spi {
	struct spi_device	*spi_dev;
	char			*zero_txbuf;
	dma_addr_t 		zero_txbuf_phy_addr;
	int 			bus_speed;
	void (*interruptHandler) (void *);
	void			*intr_context;
};

/*!
invert the endianness of a single 32it integer

\param[in]		u: word to invert

\return		the inverted word
*/
static inline u32 invert_bo(u32 u)
{
	return ((u & 0xff) << 24) | ((u & 0xff00) << 8) | ((u & 0xff0000) >> 8)
		| ((u & 0xff000000) >> 24);
}

/*!
invert the endianness of a data buffer

\param[in]		buf: buffer to invert
\param[in]		len: buffer length

\return		the inverted word
*/


static int invert_endianness(char *buf, int len)
{
	int i;
	u32 *ptr = (u32 *) buf;

	len = (len + 3) / 4;
	for (i = 0; i < len; i++, ptr++)
	{
		*ptr = invert_bo(*ptr);
	}
	
	return 4 * ((len + 3) & (~3));
}

static irqreturn_t spibus_interrupt(int irq, void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_debug ("SPI interrupt recieved.");
	if (sms_spi->interruptHandler)
		sms_spi->interruptHandler(sms_spi->intr_context);
	return IRQ_HANDLED;

}

void prepareForFWDnl(void *context)
{
	/*Reduce clock rate for FW download*/
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_spi->bus_speed = MAX_SPEED_DURING_DOWNLOAD;
	sms_spi->spi_dev->max_speed_hz = sms_spi->bus_speed;
	if (spi_setup(sms_spi->spi_dev))
	{
		sms_err("SMS device setup failed");
	}

	sms_debug ("Start FW download.");
	msleep(100);
	sms_debug ("done sleeping.");
}

void fwDnlComplete(void *context, int App)
{
	/*Set clock rate for working mode*/
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_spi->bus_speed = spi_max_speed;
	sms_spi->spi_dev->max_speed_hz = sms_spi->bus_speed;
	if (spi_setup(sms_spi->spi_dev))
	{
		sms_err("SMS device setup failed");
	}
	sms_debug ("FW download complete.");
	msleep(100);
}


void smsspibus_xfer(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.tx_buf = txbuf,
		.rx_buf = rxbuf,
		.len = len,
		.tx_dma = txbuf_phy_addr,
		.rx_dma = rxbuf_phy_addr,
		.cs_change = 0,
	};
	if (txbuf)
	{
		invert_endianness(txbuf, len);
	}

	if (!txbuf)
	{
		xfer.tx_buf = sms_spi->zero_txbuf;
		xfer.tx_dma = sms_spi->zero_txbuf_phy_addr;
		
	}
	spi_message_init(&msg);
	msg.is_dma_mapped = 1;
	spi_message_add_tail(&xfer, &msg);
	spi_sync (sms_spi->spi_dev, &msg);
	invert_endianness(rxbuf, len);

}

int smsspiphy_is_device_exists(void)
{
	int i = 0;

	/* Check 3 times if the interrupt pin is 0. if it never goes down - 
	there is not SPI device,*/
	for (i = 0; i < 3; i++)
	{
		if (gpio_get_value(host_spi_intr_pin) == 0)
		{
			return 1;
		}
		msleep(1);
	}
	return 0;
}


int smsspiphy_init(void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	int ret;
	struct spi_device *sms_device;

	struct spi_master *master = spi_busnum_to_master(host_spi_bus);
	struct spi_board_info sms_chip = {
		.modalias = "SmsSPI",
		.platform_data 	= NULL,
		.controller_data = NULL,
		.irq		= 0, 
		.max_speed_hz	= MAX_SPEED_DURING_DOWNLOAD,
		.bus_num	= host_spi_bus,
		.chip_select 	= host_spi_cs,
		.mode		= SPI_MODE_0,
	};
	if (!master)
	{
		sms_err("Could not find SPI master device.");
		ret = -ENODEV;
		goto no_spi_master;
	}
	

	sms_device = spi_new_device(master, &sms_chip);	
	if (!sms_device)
	{
		sms_err("Failed on allocating new SPI device for SMS");
		ret = -ENODEV;
		goto no_spi_master;
	}

	sms_device->bits_per_word = 32;
	if (spi_setup(sms_device))
	{
		sms_err("SMS device setup failed");
		ret = -ENODEV;
		goto spi_setup_failed;
	}


	sms_spi->spi_dev = sms_device;
	sms_spi->bus_speed = MAX_SPEED_DURING_DOWNLOAD;
	sms_debug("after init sms_spi=0x%x, spi_dev = 0x%x", (int)sms_spi, (int)sms_spi->spi_dev);

	return 0;

spi_setup_failed:
	spi_unregister_device(sms_device);
no_spi_master:
	return ret;
}

void smsspiphy_deinit(void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_debug("smsspiphy_deinit\n");
	/*Release the SPI device*/
	spi_unregister_device(sms_spi->spi_dev);
	sms_spi->spi_dev = NULL;

}


void *smsspiphy_register(void *context, void (*smsspi_interruptHandler) (void *), 
		     void *intr_context)
{
	int ret;
	struct sms_spi *sms_spi; 
	sms_spi = kzalloc(sizeof(struct sms_spi), GFP_KERNEL);
	if (!sms_spi)
	{
		sms_err("SMS device mempory allocating");
		goto memory_allocate_failed;

	}

	sms_spi->zero_txbuf =  dma_alloc_coherent(NULL, SPI_PACKET_SIZE,
			       &sms_spi->zero_txbuf_phy_addr,
			       GFP_KERNEL | GFP_DMA);
	if (!sms_spi->zero_txbuf) {
		sms_err ("dma_alloc_coherent(...) failed\n");
		goto dma_allocate_failed;
	}
	memset (sms_spi->zero_txbuf, 0, SPI_PACKET_SIZE);
	sms_spi->interruptHandler = smsspi_interruptHandler;
	sms_spi->intr_context = intr_context;


	if (gpio_request(host_spi_intr_pin, "SMSSPI"))
	{
		sms_err("Could not get GPIO for SMS device intr.\n");
		goto request_gpio_failed;
	}
	gpio_direction_input(host_spi_intr_pin);
	gpio_export(host_spi_intr_pin, 1);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)
	irq_set_irq_type(gpio_to_irq(host_spi_intr_pin), IRQ_TYPE_EDGE_FALLING);
#else
	set_irq_type(gpio_to_irq(host_spi_intr_pin), IRQ_TYPE_EDGE_FALLING);
#endif
	ret = request_irq(gpio_to_irq(host_spi_intr_pin), spibus_interrupt, IRQF_TRIGGER_FALLING, "SMSSPI", sms_spi);
	if (ret) {
		sms_err("Could not get interrupt for SMS device. status =%d\n", ret);
		goto request_irq_failed;
	}
	return sms_spi;

request_irq_failed:
	gpio_free(host_spi_intr_pin);
request_gpio_failed:
	dma_free_coherent(NULL, SPI_PACKET_SIZE, sms_spi->zero_txbuf, sms_spi->zero_txbuf_phy_addr);
dma_allocate_failed:
	kfree(sms_spi);
memory_allocate_failed:
	return NULL;
}

void smsspiphy_unregister(void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	/*Release the IRQ line*/
	free_irq(gpio_to_irq(host_spi_intr_pin), sms_spi);
	/*Release the GPIO lines*/
	gpio_free(host_spi_intr_pin);
	/*Release the DMA buffer*/
	dma_free_coherent(NULL, SPI_PACKET_SIZE, sms_spi->zero_txbuf, sms_spi->zero_txbuf_phy_addr);
	/*Release memory*/
	kfree(sms_spi);
}

void smschipreset(void *context)
{
	msleep(100);
}


