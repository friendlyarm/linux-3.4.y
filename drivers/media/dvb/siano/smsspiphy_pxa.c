/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
Copyright (C) 2006-2008, Uri Shkolnik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************/
#define PXA_310_LV

#include <linux/kernel.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#ifdef PXA_310_LV
#include <asm/arch/ssp.h>
#include <asm/arch/mfp.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pxa3xx_gpio.h>
#endif
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <asm/dma.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include "smsdbg_prn.h"
#include <linux/slab.h>

#ifdef PXA_310_LV

#define SSP_PORT 4
#define SSP_CKEN CKEN_SSP4
#define SMS_IRQ_GPIO MFP_PIN_GPIO93

#if (SSP_PORT == 1)
#define SDCMR_RX DRCMRRXSSDR
#define SDCMR_TX DRCMRTXSSDR
#else
#if (SSP_PORT == 2)
#define SDCMR_RX DRCMR15
#define SDCMR_TX DRCMR16
#else
#if (SSP_PORT == 3)
#define SDCMR_RX DRCMR66
#define SDCMR_TX DRCMR67
#else
#if (SSP_PORT == 4)
#define SDCMR_RX DRCMRRXSADR
#define SDCMR_TX DRCMRTXSADR
#endif
#endif
#endif
#endif
#else /*PXA_310_LV */
#define SSP_PORT 1
#define SDCMR_RX DRCMRRXSSDR
#define SDCMR_TX DRCMRTXSSDR

#endif /*PXA_310_LV */

/* Macros defining physical layer behaviour*/
#ifdef PXA_310_LV
#define CLOCK_FACTOR 1
#else /*PXA_310_LV */
#define CLOCK_FACTOR 2
#endif /*PXA_310_LV */

/* Macros for coding reuse */

/*! macro to align the divider to the proper offset in the register bits */
#define CLOCK_DIVIDER(i)((i-1)<<8)	/* 1-4096 */

/*! DMA related macros */
#define DMA_INT_MASK (DCSR_ENDINTR | DCSR_STARTINTR | DCSR_BUSERR)
#define RESET_DMA_CHANNEL (DCSR_NODESC | DMA_INT_MASK)

#define SSP_TIMEOUT_SCALE (769)
#define SSP_TIMEOUT(x) ((x*10000)/SSP_TIMEOUT_SCALE)

#define SPI_PACKET_SIZE 256

/* physical layer variables */
/*! global bus data */
struct spiphy_dev_s {
	struct ssp_dev sspdev;	/*!< ssp port configuration */
	struct completion transfer_in_process;
	void (*interruptHandler) (void *);
	void *intr_context;
	struct device *dev;	/*!< device model stuff */
	int rx_dma_channel;
	int tx_dma_channel;
	int rx_buf_len;
	int tx_buf_len;
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
		*ptr = invert_bo(*ptr);

	return 4 * ((len + 3) & (~3));
}

/*! Map DMA buffers when request starts

\return	error status
*/
static unsigned long dma_map_buf(struct spiphy_dev_s *spiphy_dev, char *buf,
		int len, int direction)
{
	unsigned long phyaddr;
	/* map dma buffers */
	if (!buf) {
		PERROR(" NULL buffers to map\n");
		return 0;
	}
	/* map buffer */
	phyaddr = dma_map_single(spiphy_dev->dev, buf, len, direction);
	if (dma_mapping_error(phyaddr)) {
		PERROR("exiting  with error\n");
		return 0;
	}
	return phyaddr;
}

static irqreturn_t spibus_interrupt(int irq, void *context)
{
	struct spiphy_dev_s *spiphy_dev = (struct spiphy_dev_s *) context;
	PDEBUG("recieved interrupt from device dev=%p.\n", context);
	if (spiphy_dev->interruptHandler)
		spiphy_dev->interruptHandler(spiphy_dev->intr_context);
	return IRQ_HANDLED;

}

/*!	DMA controller callback - called only on BUS error condition

\param[in]	channel: DMA channel with error
\param[in]	data: Unused
\param[in]	regs: Unused
\return		void
*/
static void spibus_dma_handler(int channel, void *context)
{
	struct spiphy_dev_s *spiphy_dev = (struct spiphy_dev_s *) context;
	u32 irq_status = DCSR(channel) & DMA_INT_MASK;

	PDEBUG("recieved interrupt from dma channel %d irq status %x.\n",
	       channel, irq_status);
	if (irq_status & DCSR_BUSERR) {
		PERROR("bus error!!! resetting channel %d\n", channel);

		DCSR(spiphy_dev->rx_dma_channel) = RESET_DMA_CHANNEL;
		DCSR(spiphy_dev->tx_dma_channel) = RESET_DMA_CHANNEL;
	}
	DCSR(spiphy_dev->rx_dma_channel) = RESET_DMA_CHANNEL;
	complete(&spiphy_dev->transfer_in_process);
}

void smsspibus_xfer(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len)
{
	/* DMA burst is 8 bytes, therefore we need tmp buffer that size. */
	unsigned long tmp[2];
	unsigned long txdma;
	struct spiphy_dev_s *spiphy_dev = (struct spiphy_dev_s *) context;

	/* program the controller */
	if (txbuf)
		invert_endianness(txbuf, len);

	tmp[0] = -1;
	tmp[1] = -1;

	/* map RX buffer */

	if (!txbuf)
		txdma =
		    dma_map_buf(spiphy_dev, (char *)tmp, sizeof(tmp),
				DMA_TO_DEVICE);
	else
		txdma = txbuf_phy_addr;

	init_completion(&spiphy_dev->transfer_in_process);
	/* configure DMA Controller */
	DCSR(spiphy_dev->rx_dma_channel) = RESET_DMA_CHANNEL;
	DSADR(spiphy_dev->rx_dma_channel) = __PREG(SSDR_P(SSP_PORT));
	DTADR(spiphy_dev->rx_dma_channel) = rxbuf_phy_addr;
	DCMD(spiphy_dev->rx_dma_channel) = DCMD_INCTRGADDR | DCMD_FLOWSRC
	    | DCMD_WIDTH4 | DCMD_ENDIRQEN | DCMD_BURST8 | len;
	PDEBUG("rx channel=%d, src=0x%x, dst=0x%x, cmd=0x%x\n",
	       spiphy_dev->rx_dma_channel, __PREG(SSDR_P(SSP_PORT)),
	       (unsigned int)rxbuf_phy_addr, DCMD(spiphy_dev->rx_dma_channel));
	spiphy_dev->rx_buf_len = len;

	DCSR(spiphy_dev->tx_dma_channel) = RESET_DMA_CHANNEL;
	DTADR(spiphy_dev->tx_dma_channel) = __PREG(SSDR_P(SSP_PORT));
	DSADR(spiphy_dev->tx_dma_channel) = txdma;
	if (txbuf) {
		DCMD(spiphy_dev->tx_dma_channel) =
		    DCMD_INCSRCADDR | DCMD_FLOWTRG | DCMD_WIDTH4
		    /* | DCMD_ENDIRQEN */  | DCMD_BURST8 | len;
		spiphy_dev->tx_buf_len = len;
	} else {
		DCMD(spiphy_dev->tx_dma_channel) = DCMD_FLOWTRG
		    | DCMD_WIDTH4 /* | DCMD_ENDIRQEN */  | DCMD_BURST8 | len;
		spiphy_dev->tx_buf_len = 4;
	}

	PDEBUG("tx channel=%d, src=0x%x, dst=0x%x, cmd=0x%x\n",
	       spiphy_dev->tx_dma_channel, (unsigned int)txdma,
	       __PREG(SSDR_P(SSP_PORT)), DCMD(spiphy_dev->tx_dma_channel));
	/* DALGN - DMA ALIGNMENT REG. */
	if (rxbuf_phy_addr & 0x7)
		DALGN |= (1 << spiphy_dev->rx_dma_channel);
	else
		DALGN &= ~(1 << spiphy_dev->rx_dma_channel);
	if (txdma & 0x7)
		DALGN |= (1 << spiphy_dev->tx_dma_channel);
	else
		DALGN &= ~(1 << spiphy_dev->tx_dma_channel);

	/* Start DMA controller */
	DCSR(spiphy_dev->rx_dma_channel) |= DCSR_RUN;
	DCSR(spiphy_dev->tx_dma_channel) |= DCSR_RUN;
	PDEBUG("DMA running. wait for completion.\n");
	wait_for_completion(&spiphy_dev->transfer_in_process);
	PDEBUG("DMA complete.\n");
	invert_endianness(rxbuf, len);
}

void smschipreset(void *context)
{

}

void *smsspiphy_init(void *context, void (*smsspi_interruptHandler) (void *),
		     void *intr_context)
{
	int ret;
	struct spiphy_dev_s *spiphy_dev;
	u32 mode = 0, flags = 0, psp_flags = 0, speed = 0;
	PDEBUG("entering\n");

	spiphy_dev = kmalloc(sizeof(struct spiphy_dev_s), GFP_KERNEL);

	ret = ssp_init(&spiphy_dev->sspdev, SSP_PORT, 0);
	if (ret) {
		PERROR("ssp_init failed. error %d", ret);
		goto error_sspinit;
	}
#ifdef PXA_310_LV
	pxa3xx_mfp_set_afds(SMS_IRQ_GPIO, MFP_AF0, MFP_DS03X);
	pxa3xx_gpio_set_rising_edge_detect(SMS_IRQ_GPIO, 1);
	pxa3xx_gpio_set_direction(SMS_IRQ_GPIO, GPIO_DIR_IN);
#else /*PXA_310_LV */
	/* receive input interrupts from the SMS 1000 on J32 pin 11 */
	pxa_gpio_mode(22 | GPIO_IN);
#endif /*PXA_310_LV */
	speed = CLOCK_DIVIDER(CLOCK_FACTOR); /* clock divisor for this mode */
	/* 32bit words in the fifo */
	mode = SSCR0_Motorola | SSCR0_DataSize(16) | SSCR0_EDSS;
	/* SSCR1 = flags */
	flags = SSCR1_RxTresh(1) | SSCR1_TxTresh(1) | SSCR1_TSRE |
	 SSCR1_RSRE | SSCR1_RIE | SSCR1_TRAIL;	/* | SSCR1_TIE */

	ssp_config(&spiphy_dev->sspdev, mode, flags, psp_flags, speed);
	ssp_disable(&(spiphy_dev->sspdev));
#ifdef PXA_310_LV

	pxa3xx_mfp_set_afds(MFP_PIN_GPIO95, MFP_AF1, MFP_DS03X);
	pxa3xx_mfp_set_afds(MFP_PIN_GPIO96, MFP_AF1, MFP_DS03X);
	pxa3xx_mfp_set_afds(MFP_PIN_GPIO97, MFP_AF1, MFP_DS03X);
	pxa3xx_mfp_set_afds(MFP_PIN_GPIO98, MFP_AF1, MFP_DS03X);
#else /*PXA_310_LV */
	pxa_gpio_mode(GPIO23_SCLK_MD);
	pxa_gpio_mode(GPIO24_SFRM_MD);
	pxa_gpio_mode(GPIO25_STXD_MD);
	pxa_gpio_mode(GPIO26_SRXD_MD);
#endif /*PXA_310_LV */
	/* setup the dma */
	spiphy_dev->rx_dma_channel =
	    pxa_request_dma("spibusdrv_rx", DMA_PRIO_HIGH, spibus_dma_handler,
			    spiphy_dev);
	if (spiphy_dev->rx_dma_channel < 0) {
		ret = -EBUSY;
		PERROR("Could not get RX DMA channel.\n");
		goto error_rxdma;
	}
	spiphy_dev->tx_dma_channel =
	    pxa_request_dma("spibusdrv_tx", DMA_PRIO_HIGH, spibus_dma_handler,
			    spiphy_dev);
	if (spiphy_dev->tx_dma_channel < 0) {
		ret = -EBUSY;
		PERROR("Could not get TX DMA channel.\n");
		goto error_txdma;
	}

	SDCMR_RX = DRCMR_MAPVLD | spiphy_dev->rx_dma_channel;
	SDCMR_TX = DRCMR_MAPVLD | spiphy_dev->tx_dma_channel;

	PDEBUG("dma rx channel: %d, dma tx channel: %d\n",
	       spiphy_dev->rx_dma_channel, spiphy_dev->tx_dma_channel);
	/* enable the clock */

	spiphy_dev->interruptHandler = smsspi_interruptHandler;
	spiphy_dev->intr_context = intr_context;
#ifdef PXA_310_LV
	set_irq_type(IRQ_GPIO(MFP2GPIO(SMS_IRQ_GPIO)), IRQT_FALLING);
	ret =
	    request_irq(IRQ_GPIO(MFP2GPIO(SMS_IRQ_GPIO)), spibus_interrupt,
			SA_INTERRUPT, "SMSSPI", spiphy_dev);
#else /*PXA_310_LV */
	set_irq_type(IRQ_GPIO(22), IRQT_FALLING);
	ret =
	    request_irq(IRQ_GPIO(22), spibus_interrupt, SA_INTERRUPT, "SMSSPI",
			&(g_spidata.sspdev));
#endif /*PXA_310_LV */
	if (ret) {
		PERROR("Could not get interrupt for SMS device. status =%d\n",
		       ret);
		goto error_irq;
	}

	ssp_enable(&(spiphy_dev->sspdev));
	PDEBUG("exiting\n");
	return spiphy_dev;
error_irq:
	if (spiphy_dev->tx_dma_channel >= 0)
		pxa_free_dma(spiphy_dev->tx_dma_channel);

error_txdma:
	if (spiphy_dev->rx_dma_channel >= 0)
		pxa_free_dma(spiphy_dev->rx_dma_channel);

error_rxdma:
	ssp_exit(&spiphy_dev->sspdev);
error_sspinit:
	PDEBUG("exiting on error\n");
	return 0;
}

int smsspiphy_deinit(void *context)
{
	struct spiphy_dev_s *spiphy_dev = (struct spiphy_dev_s *) context;
	PDEBUG("entering\n");

	/* disable the spi port */
	ssp_flush(&spiphy_dev->sspdev);
	ssp_disable(&spiphy_dev->sspdev);

	/*  release DMA resources */
	if (spiphy_dev->rx_dma_channel >= 0)
		pxa_free_dma(spiphy_dev->rx_dma_channel);

	if (spiphy_dev->tx_dma_channel >= 0)
		pxa_free_dma(spiphy_dev->tx_dma_channel);

	/* release Memory resources */
#ifdef PXA_310_LV
	free_irq(IRQ_GPIO(MFP2GPIO(SMS_IRQ_GPIO)), spiphy_dev);
#else /*PXA_310_LV */
	free_irq(IRQ_GPIO(22), &spiphy_dev->sspdev);
#endif /*PXA_310_LV */
	ssp_exit(&spiphy_dev->sspdev);
	PDEBUG("exiting\n");
	return 0;
}

void smsspiphy_set_config(struct spiphy_dev_s *spiphy_dev, int clock_divider)
{
	u32 mode, flags, speed, psp_flags = 0;
	ssp_disable(&spiphy_dev->sspdev);
	/* clock divisor for this mode. */
	speed = CLOCK_DIVIDER(clock_divider);
	/* 32bit words in the fifo */
	mode = SSCR0_Motorola | SSCR0_DataSize(16) | SSCR0_EDSS;
	flags = SSCR1_RxTresh(1) | SSCR1_TxTresh(1) | SSCR1_TSRE |
		 SSCR1_RSRE | SSCR1_RIE | SSCR1_TRAIL;	/* | SSCR1_TIE */
	ssp_config(&spiphy_dev->sspdev, mode, flags, psp_flags, speed);
	ssp_enable(&spiphy_dev->sspdev);
}

void prepareForFWDnl(void *context)
{
	struct spiphy_dev_s *spiphy_dev = (struct spiphy_dev_s *) context;
	smsspiphy_set_config(spiphy_dev, 2);
	msleep(100);
}

void fwDnlComplete(void *context, int App)
{
	struct spiphy_dev_s *spiphy_dev = (struct spiphy_dev_s *) context;
	smsspiphy_set_config(spiphy_dev, 1);
	msleep(100);
}
