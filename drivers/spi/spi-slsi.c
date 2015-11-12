/*
 * Copyright (C) 2009 Samsung Electronics Ltd.
 *	Jaswinder Singh <jassi.brar@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>

#include <mach/slsi-spi.h>
#include <linux/dmaengine.h>

/*
 * The type of reading going on on this chip
 */
enum ssp_reading {
    READING_NULL,
    READING_U8,
    READING_U16,
    READING_U32
};

/**
 * The type of writing going on on this chip
 */
enum ssp_writing {
    WRITING_NULL,
    WRITING_U8,
    WRITING_U16,
    WRITING_U32
};

/**
 * enum ssp_rx_level_trig - receive FIFO watermark level which triggers
 * IT: Interrupt fires when _N_ or more elements in RX FIFO.
 */
enum ssp_rx_level_trig {
    SSP_RX_1_OR_MORE_ELEM,
    SSP_RX_4_OR_MORE_ELEM,
    SSP_RX_8_OR_MORE_ELEM,
    SSP_RX_16_OR_MORE_ELEM,
    SSP_RX_32_OR_MORE_ELEM
};

/**
 * Transmit FIFO watermark level which triggers (IT Interrupt fires
 * when _N_ or more empty locations in TX FIFO)
 */
enum ssp_tx_level_trig {
    SSP_TX_1_OR_MORE_EMPTY_LOC,
    SSP_TX_4_OR_MORE_EMPTY_LOC,
    SSP_TX_8_OR_MORE_EMPTY_LOC,
    SSP_TX_16_OR_MORE_EMPTY_LOC,
    SSP_TX_32_OR_MORE_EMPTY_LOC
};

/* Registers and bit-fields */

#define STATE_START         ((void *) 0)
#define STATE_RUNNING       ((void *) 1)
#define STATE_DONE          ((void *) 2)
#define STATE_ERROR         ((void *) -1)

#define S3C64XX_SPI_CH_CFG		0x00
#define S3C64XX_SPI_CLK_CFG		0x04
#define S3C64XX_SPI_MODE_CFG	0x08
#define S3C64XX_SPI_SLAVE_SEL	0x0C
#define S3C64XX_SPI_INT_EN		0x10
#define S3C64XX_SPI_STATUS		0x14
#define S3C64XX_SPI_TX_DATA		0x18
#define S3C64XX_SPI_RX_DATA		0x1C
#define S3C64XX_SPI_PACKET_CNT	0x20
#define S3C64XX_SPI_PENDING_CLR	0x24
#define S3C64XX_SPI_SWAP_CFG	0x28
#define S3C64XX_SPI_FB_CLK		0x2C

#define S3C64XX_SPI_CH_HS_EN		(1<<6)	/* High Speed Enable */
#define S3C64XX_SPI_CH_SW_RST		(1<<5)
#define S3C64XX_SPI_CH_SLAVE		(1<<4)
#define S3C64XX_SPI_CPOL_L		(1<<3)
#define S3C64XX_SPI_CPHA_B		(1<<2)
#define S3C64XX_SPI_CH_RXCH_ON		(1<<1)
#define S3C64XX_SPI_CH_TXCH_ON		(1<<0)

#define S3C64XX_SPI_CLKSEL_SRCMSK	(3<<9)
#define S3C64XX_SPI_CLKSEL_SRCSHFT	9
#define S3C64XX_SPI_ENCLK_ENABLE	(1<<8)
#define S3C64XX_SPI_PSR_MASK 		0xff

#define S3C64XX_SPI_MODE_CH_TSZ_BYTE		(0<<29)
#define S3C64XX_SPI_MODE_CH_TSZ_HALFWORD	(1<<29)
#define S3C64XX_SPI_MODE_CH_TSZ_WORD		(2<<29)
#define S3C64XX_SPI_MODE_CH_TSZ_MASK		(3<<29)
#define S3C64XX_SPI_MODE_BUS_TSZ_BYTE		(0<<17)
#define S3C64XX_SPI_MODE_BUS_TSZ_HALFWORD	(1<<17)
#define S3C64XX_SPI_MODE_BUS_TSZ_WORD		(2<<17)
#define S3C64XX_SPI_MODE_BUS_TSZ_MASK		(3<<17)
#define S3C64XX_SPI_MODE_RXDMA_ON		(1<<2)
#define S3C64XX_SPI_MODE_TXDMA_ON		(1<<1)
#define S3C64XX_SPI_MODE_4BURST			(1<<0)

#define S3C64XX_SPI_SLAVE_AUTO			(1<<1)
#define S3C64XX_SPI_SLAVE_SIG_INACT		(1<<0)

#define S3C64XX_SPI_AUTO(c) writel(S3C64XX_SPI_SLAVE_AUTO, (c)->regs + S3C64XX_SPI_SLAVE_SEL)
#define S3C64XX_SPI_ACT(c) writel(0, (c)->regs + S3C64XX_SPI_SLAVE_SEL)

#define S3C64XX_SPI_DEACT(c) writel(S3C64XX_SPI_SLAVE_SIG_INACT, \
					(c)->regs + S3C64XX_SPI_SLAVE_SEL)

#define S3C64XX_SPI_INT_TRAILING_EN		(1<<6)
#define S3C64XX_SPI_INT_RX_OVERRUN_EN		(1<<5)
#define S3C64XX_SPI_INT_RX_UNDERRUN_EN		(1<<4)
#define S3C64XX_SPI_INT_TX_OVERRUN_EN		(1<<3)
#define S3C64XX_SPI_INT_TX_UNDERRUN_EN		(1<<2)
#define S3C64XX_SPI_INT_RX_FIFORDY_EN		(1<<1)
#define S3C64XX_SPI_INT_TX_FIFORDY_EN		(1<<0)

#define S3C64XX_SPI_ST_RX_OVERRUN_ERR		(1<<5)
#define S3C64XX_SPI_ST_RX_UNDERRUN_ERR	(1<<4)
#define S3C64XX_SPI_ST_TX_OVERRUN_ERR		(1<<3)
#define S3C64XX_SPI_ST_TX_UNDERRUN_ERR	(1<<2)
#define S3C64XX_SPI_ST_RX_FIFORDY		(1<<1)
#define S3C64XX_SPI_ST_TX_FIFORDY		(1<<0)

#define S3C64XX_SPI_PACKET_CNT_EN		(1<<16)

#define S3C64XX_SPI_PND_TX_UNDERRUN_CLR		(1<<4)
#define S3C64XX_SPI_PND_TX_OVERRUN_CLR		(1<<3)
#define S3C64XX_SPI_PND_RX_UNDERRUN_CLR		(1<<2)
#define S3C64XX_SPI_PND_RX_OVERRUN_CLR		(1<<1)
#define S3C64XX_SPI_PND_TRAILING_CLR		(1<<0)

#define S3C64XX_SPI_SWAP_RX_HALF_WORD		(1<<7)
#define S3C64XX_SPI_SWAP_RX_BYTE		(1<<6)
#define S3C64XX_SPI_SWAP_RX_BIT			(1<<5)
#define S3C64XX_SPI_SWAP_RX_EN			(1<<4)
#define S3C64XX_SPI_SWAP_TX_HALF_WORD		(1<<3)
#define S3C64XX_SPI_SWAP_TX_BYTE		(1<<2)
#define S3C64XX_SPI_SWAP_TX_BIT			(1<<1)
#define S3C64XX_SPI_SWAP_TX_EN			(1<<0)

#define S3C64XX_SPI_FBCLK_MSK		(3<<0)

#define S3C64XX_SPI_ST_TRLCNTZ(v, i) ((((v) >> (i)->rx_lvl_offset) & \
					(((i)->fifo_lvl_mask + 1))) \
					? 1 : 0)

#define S3C64XX_SPI_ST_TX_DONE(v, i) (((v) & (1 << (i)->tx_st_done)) ? 1 : 0)
#define TX_FIFO_LVL(v, i) (((v) >> 6) & (i)->fifo_lvl_mask)
#define RX_FIFO_LVL(v, i) (((v) >> (i)->rx_lvl_offset) & (i)->fifo_lvl_mask)

#define S3C64XX_SPI_MAX_TRAILCNT	0x3ff
#define S3C64XX_SPI_TRAILCNT_OFF	19

#define S3C64XX_SPI_TRAILCNT		S3C64XX_SPI_MAX_TRAILCNT

#define msecs_to_loops(t) (loops_per_jiffy / 1000 * HZ * t)

#define RXBUSY    (1<<2)
#define TXBUSY    (1<<3)

/**
 * struct s3c64xx_spi_driver_data - Runtime info holder for SPI driver.
 * @clk: Pointer to the spi clock.
 * @src_clk: Pointer to the clock used to generate SPI signals.
 * @master: Pointer to the SPI Protocol master.
 * @cntrlr_info: Platform specific data for the controller this driver manages.
 * @tgl_spi: Pointer to the last CS left untoggled by the cs_change hint.
 * @queue: To log SPI xfer requests.
 * @lock: Controller specific lock.
 * @state: Set of FLAGS to indicate status.
 * @rx_dmach: Controller's DMA channel for Rx.
 * @tx_dmach: Controller's DMA channel for Tx.
 * @sfr_start: BUS address of SPI controller regs.
 * @regs: Pointer to ioremap'ed controller registers.
 * @irq: interrupt
 * @xfer_completion: To indicate completion of xfer task.
 * @cur_mode: Stores the active configuration of the controller.
 * @cur_bpw: Stores the active bits per word settings.
 * @cur_speed: Stores the active xfer clock speed.
 */
struct s3c64xx_spi_driver_data {
	void __iomem                    *regs;
	resource_size_t         		phybase;
	struct clk                      *clk;
	struct clk                      *src_clk;
	struct platform_device          *pdev;
	struct spi_master               *master;
	struct s3c64xx_spi_info  *cntrlr_info;
	struct spi_device               *tgl_spi;
	struct list_head                queue;
	spinlock_t                      lock;
	unsigned long                   sfr_start;
	struct completion               xfer_completion;
	unsigned                        state;
	unsigned                        cur_mode, cur_bpw;
	unsigned                        cur_speed;
	
#ifdef CONFIG_DMA_ENGINE
	void                			*tx;
	void                			*tx_end;
	void                			*rx;
	void                			*rx_end;

	struct spi_message      		*cur_msg;
	struct spi_transfer     		*cur_transfer;
	struct chip_data        		*cur_chip;
	
	struct tasklet_struct       	pump_transfers;

    struct dma_chan         		*dma_rx_channel;
    struct dma_chan         		*dma_tx_channel;
    struct sg_table         		sgt_rx;
    struct sg_table         		sgt_tx;
    char                			*dummypage;
    bool                			dma_running;
#endif
};
struct chip_data {
    u32 cr0;
    u16 cr1;
    u16 dmacr;
    u16 cpsr;
    u8 n_bytes;
    bool enable_dma;
    enum ssp_reading read;
    enum ssp_writing write;
    void (*cs_control) (u32 command);
    int xfer_type;
 };

//static struct s3c2410_dma_client s3c64xx_spi_dma_client = {
//	.name = "samsung-spi-dma",
//};
static int set_up_next_transfer(struct s3c64xx_spi_driver_data *sdd,
				struct spi_transfer *transfer);

static void *next_transfer(struct s3c64xx_spi_driver_data *sdd)
{
	struct spi_message *msg = sdd->cur_msg;
	struct spi_transfer *trans = sdd->cur_transfer;

	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		sdd->cur_transfer =
		    list_entry(trans->transfer_list.next,
			       struct spi_transfer, transfer_list);
		return STATE_RUNNING;
	}
	return STATE_DONE;
}
static void unmap_free_dma_scatter(struct s3c64xx_spi_driver_data *sdd)
{
	/* Unmap and free the SG tables */
   	dma_unmap_sg(sdd->dma_tx_channel->device->dev, sdd->sgt_tx.sgl,
		     sdd->sgt_tx.nents, DMA_TO_DEVICE);
	dma_unmap_sg(sdd->dma_rx_channel->device->dev, sdd->sgt_rx.sgl,
		     sdd->sgt_rx.nents, DMA_FROM_DEVICE);
	sg_free_table(&sdd->sgt_rx);
	sg_free_table(&sdd->sgt_tx);
}

static void dma_callback(void *data)
{
	struct s3c64xx_spi_driver_data *sdd = data;
	struct spi_message *msg = sdd->cur_msg;

	BUG_ON(!sdd->sgt_rx.sgl);

	complete(&sdd->xfer_completion);
#ifdef VERBOSE_DEBUG
	/*
	 * Optionally dump out buffers to inspect contents, this is
	 * good if you want to convince yourself that the loopback
	 * read/write contents are the same, when adopting to a new
	 * DMA engine.
	 */
	{
		struct scatterlist *sg;
		unsigned int i;

		dma_sync_sg_for_cpu(&sdd->pdev->dev,
				    sdd->sgt_rx.sgl,
				    sdd->sgt_rx.nents,
				    DMA_FROM_DEVICE);

		for_each_sg(sdd->sgt_rx.sgl, sg, sdd->sgt_rx.nents, i) {
			dev_dbg(&sdd->pdev->dev, "SPI RX SG ENTRY: %d", i);
			print_hex_dump(KERN_ERR, "SPI RX: ",
				       DUMP_PREFIX_OFFSET,
				       16,
				       1,
				       sg_virt(sg),
				       sg_dma_len(sg),
				       1);
		}
		for_each_sg(sdd->sgt_tx.sgl, sg, sdd->sgt_tx.nents, i) {
			dev_dbg(&sdd->pdev->dev, "SPI TX SG ENTRY: %d", i);
			print_hex_dump(KERN_ERR, "SPI TX: ",
				       DUMP_PREFIX_OFFSET,
				       16,
				       1,
				       sg_virt(sg),
				       sg_dma_len(sg),
				       1);
		}
	}
#endif

	unmap_free_dma_scatter(sdd);

	/* Update total bytes transferred */
	msg->actual_length += sdd->cur_transfer->len;
	

	/* Move to next transfer */
	msg->state = next_transfer(sdd);

}

static void setup_dma_scatter(struct s3c64xx_spi_driver_data *sdd,
			      void *buffer,
			      unsigned int length,
			      struct sg_table *sgtab)
{
	struct scatterlist *sg;
	int bytesleft = length;
	void *bufp = buffer;
	int mapbytes;
	int i;

	if (buffer) {
		for_each_sg(sgtab->sgl, sg, sgtab->nents, i) {
			/*
			 * If there are less bytes left than what fits
			 * in the current page (plus page alignment offset)
			 * we just feed in this, else we stuff in as much
			 * as we can.
			 */
				mapbytes = bytesleft;
			sg_set_page(sg, virt_to_page(bufp),
				    mapbytes, offset_in_page(bufp));
			bufp += mapbytes;
			bytesleft -= mapbytes;
			dev_dbg(&sdd->pdev->dev,
				"set RX/TX target page @ %p, %d bytes, %d left\n",
				bufp, mapbytes, bytesleft);
		}
	} else {
		/* Map the dummy buffer on every page */
		for_each_sg(sgtab->sgl, sg, sgtab->nents, i) {
			if (bytesleft < PAGE_SIZE)
				mapbytes = bytesleft;
			else
				mapbytes = PAGE_SIZE;
			sg_set_page(sg, virt_to_page(sdd->dummypage),
				    mapbytes, 0);
			bytesleft -= mapbytes;
			dev_dbg(&sdd->pdev->dev,
				"set RX/TX to dummy page %d bytes, %d left\n",
				mapbytes, bytesleft);

		}
	}
	//BUG_ON(bytesleft);
}

static int configure_dma(struct s3c64xx_spi_driver_data *sdd, struct spi_transfer *xfer)
{
	struct dma_slave_config rx_conf = {
		.src_addr = sdd->phybase+S3C64XX_SPI_RX_DATA,
		.direction = DMA_DEV_TO_MEM,
		.device_fc = false,
	};
	struct dma_slave_config tx_conf = {
		.dst_addr = sdd->phybase+S3C64XX_SPI_TX_DATA,
		.direction = DMA_MEM_TO_DEV,
		.device_fc = false,
	};
	unsigned int pages;
	int ret;
	int rx_sglen, tx_sglen;
	struct dma_chan *rxchan = sdd->dma_rx_channel;
	struct dma_chan *txchan = sdd->dma_tx_channel;
	struct dma_async_tx_descriptor *rxdesc;
	struct dma_async_tx_descriptor *txdesc;

	/* Check that the channels are available */
	if (!rxchan || !txchan)
		return -ENODEV;
	/*
	 * If supplied, the DMA burstsize should equal the FIFO trigger level.
	 * Notice that the DMA engine uses one-to-one mapping. Since we can
	 * not trigger on 2 elements this needs explicit mapping rather than
	 * calculation.
	 */
	
		rx_conf.src_maxburst = 1;
		tx_conf.dst_maxburst = 1;

		rx_conf.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		tx_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;

	/* SPI pecularity: we need to read and write the same width */
	if (rx_conf.src_addr_width == DMA_SLAVE_BUSWIDTH_UNDEFINED)
		rx_conf.src_addr_width = tx_conf.dst_addr_width;
	if (tx_conf.dst_addr_width == DMA_SLAVE_BUSWIDTH_UNDEFINED)
		tx_conf.dst_addr_width = rx_conf.src_addr_width;
	BUG_ON(rx_conf.src_addr_width != tx_conf.dst_addr_width);

	dmaengine_slave_config(txchan, &tx_conf);
	dmaengine_slave_config(rxchan, &rx_conf);

	/* Create sglists for the transfers */
	pages = DIV_ROUND_UP(xfer->len, PAGE_SIZE);
	dev_dbg(&sdd->pdev->dev, "using %d pages for transfer\n", pages);

	ret = sg_alloc_table(&sdd->sgt_rx, pages, GFP_ATOMIC);
	if (ret)
		goto err_alloc_rx_sg;

	ret = sg_alloc_table(&sdd->sgt_tx, pages, GFP_ATOMIC);
	if (ret)
		goto err_alloc_tx_sg;

	/* Fill in the scatterlists for the RX+TX buffers */
	setup_dma_scatter(sdd, sdd->tx,
		  sdd->cur_transfer->len, &sdd->sgt_tx);

	setup_dma_scatter(sdd, sdd->rx,
		  sdd->cur_transfer->len, &sdd->sgt_rx);


	/* Map DMA buffers */
	tx_sglen = dma_map_sg(txchan->device->dev, sdd->sgt_tx.sgl,
			   sdd->sgt_tx.nents, DMA_TO_DEVICE);
	if (!tx_sglen)
		goto err_tx_sgmap;
	
	rx_sglen = dma_map_sg(rxchan->device->dev, sdd->sgt_rx.sgl,
			   sdd->sgt_rx.nents, DMA_FROM_DEVICE);
	if (!rx_sglen)
		goto err_rx_sgmap;
	
	/* Send both scatterlists */
	txdesc = dmaengine_prep_slave_sg(txchan,
				      sdd->sgt_tx.sgl,
				      tx_sglen,
				      DMA_MEM_TO_DEV,
				      DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!txdesc)
		goto err_txdesc;
	rxdesc = dmaengine_prep_slave_sg(rxchan,
				      sdd->sgt_rx.sgl,
				      rx_sglen,
				      DMA_DEV_TO_MEM,
				      DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!rxdesc)
		goto err_rxdesc;


	/* Put the callback on the RX transfer only, that should finish last */
	rxdesc->callback = dma_callback;
	rxdesc->callback_param = sdd;

	/* Submit and fire RX and TX with TX last so we're ready to read! */
	dmaengine_submit(txdesc);
	dmaengine_submit(rxdesc);
	dma_async_issue_pending(txchan);
	dma_async_issue_pending(rxchan);
	sdd->dma_running = true;

	writel(S3C64XX_SPI_INT_RX_OVERRUN_EN | S3C64XX_SPI_INT_RX_UNDERRUN_EN |
	      	S3C64XX_SPI_INT_TX_OVERRUN_EN,// | S3C64XX_SPI_INT_TX_UNDERRUN_EN,
		  	sdd->regs + S3C64XX_SPI_INT_EN);
	return 0;

err_txdesc:
	dmaengine_terminate_all(txchan);
err_rxdesc:
	dmaengine_terminate_all(rxchan);
	dma_unmap_sg(txchan->device->dev, sdd->sgt_tx.sgl,
		     sdd->sgt_tx.nents, DMA_TO_DEVICE);
err_tx_sgmap:
	dma_unmap_sg(rxchan->device->dev, sdd->sgt_rx.sgl,
		     sdd->sgt_tx.nents, DMA_FROM_DEVICE);
err_rx_sgmap:
	sg_free_table(&sdd->sgt_tx);
err_alloc_tx_sg:
	sg_free_table(&sdd->sgt_rx);
err_alloc_rx_sg:
	return -ENOMEM;
}
static void stop_dma(struct s3c64xx_spi_driver_data *sdd)
{
	struct dma_chan *rxchan = sdd->dma_rx_channel;
	struct dma_chan *txchan = sdd->dma_tx_channel;
	
	dmaengine_terminate_all(txchan);
	dmaengine_terminate_all(rxchan);
	dma_unmap_sg(txchan->device->dev, sdd->sgt_tx.sgl,
		sdd->sgt_tx.nents, DMA_TO_DEVICE);
	dma_unmap_sg(rxchan->device->dev, sdd->sgt_rx.sgl,
		sdd->sgt_tx.nents, DMA_FROM_DEVICE);
	sg_free_table(&sdd->sgt_tx);
	sg_free_table(&sdd->sgt_rx);
			
	kfree(sdd->dummypage);
}
static void flush_fifo(struct s3c64xx_spi_driver_data *sdd)
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	void __iomem *regs = sdd->regs;
	unsigned long loops;
	u32 val;

	writel(0, regs + S3C64XX_SPI_PACKET_CNT);

	val = readl(regs + S3C64XX_SPI_CH_CFG);
	val &= ~(S3C64XX_SPI_CH_RXCH_ON | S3C64XX_SPI_CH_TXCH_ON);
	writel(val, regs + S3C64XX_SPI_CH_CFG);

	val = readl(regs + S3C64XX_SPI_CH_CFG);
	val |= S3C64XX_SPI_CH_SW_RST;
	val &= ~S3C64XX_SPI_CH_HS_EN;
	writel(val, regs + S3C64XX_SPI_CH_CFG);

	/* Flush TxFIFO*/
	loops = msecs_to_loops(1);
	do {
		val = readl(regs + S3C64XX_SPI_STATUS);
	} while (TX_FIFO_LVL(val, sci) && loops--);

	if (loops == 0)
		dev_warn(&sdd->pdev->dev, "Timed out flushing TX FIFO\n");

	/* Flush RxFIFO*/
	loops = msecs_to_loops(1);
	do {
		val = readl(regs + S3C64XX_SPI_STATUS);
		if (RX_FIFO_LVL(val, sci))
			readl(regs + S3C64XX_SPI_RX_DATA);
		else
			break;
	} while (loops--);

	if (loops == 0)
		dev_warn(&sdd->pdev->dev, "Timed out flushing RX FIFO\n");

	val = readl(regs + S3C64XX_SPI_CH_CFG);
	val &= ~S3C64XX_SPI_CH_SW_RST;
	writel(val, regs + S3C64XX_SPI_CH_CFG);

	val = readl(regs + S3C64XX_SPI_MODE_CFG);
	val &= ~(S3C64XX_SPI_MODE_TXDMA_ON | S3C64XX_SPI_MODE_RXDMA_ON);
	writel(val, regs + S3C64XX_SPI_MODE_CFG);
}

static void enable_datapath(struct s3c64xx_spi_driver_data *sdd,
				struct spi_device *spi,
				struct spi_transfer *xfer, int dma_mode)
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	void __iomem *regs = sdd->regs;
	u32 modecfg, chcfg;

	modecfg = readl(regs + S3C64XX_SPI_MODE_CFG);
	modecfg &= ~(S3C64XX_SPI_MODE_TXDMA_ON | S3C64XX_SPI_MODE_RXDMA_ON);

	chcfg = readl(regs + S3C64XX_SPI_CH_CFG);
	chcfg &= ~S3C64XX_SPI_CH_TXCH_ON;

	if (dma_mode) {
		set_up_next_transfer(sdd,xfer);
		configure_dma(sdd,xfer);
		chcfg &= ~S3C64XX_SPI_CH_RXCH_ON;
	} else {
		/* Always shift in data in FIFO, even if xfer is Tx only,
		 * this helps setting PCKT_CNT value for generating clocks
		 * as exactly needed.
		 */
		chcfg |= S3C64XX_SPI_CH_RXCH_ON;
		writel(((xfer->len * 8 / sdd->cur_bpw) & 0xffff)
					| S3C64XX_SPI_PACKET_CNT_EN,
					regs + S3C64XX_SPI_PACKET_CNT);
	}

	if (xfer->tx_buf != NULL) {
		sdd->state |= TXBUSY;
		chcfg |= S3C64XX_SPI_CH_TXCH_ON;
		if (dma_mode) {
			modecfg |= S3C64XX_SPI_MODE_TXDMA_ON;
		} else {
			switch (sdd->cur_bpw) {
			case 32:
				iowrite32_rep(regs + S3C64XX_SPI_TX_DATA,
					xfer->tx_buf, xfer->len / 4);
				break;
			case 16:
				iowrite16_rep(regs + S3C64XX_SPI_TX_DATA,
					xfer->tx_buf, xfer->len / 2);
				break;
			default:
				iowrite8_rep(regs + S3C64XX_SPI_TX_DATA,
					xfer->tx_buf, xfer->len);
				break;
			}
		}
	}

	if (xfer->rx_buf != NULL) {
		sdd->state |= RXBUSY;

		if (sci->high_speed && sdd->cur_speed >= 30000000UL
					&& !(sdd->cur_mode & SPI_CPHA))
			chcfg |= S3C64XX_SPI_CH_HS_EN;

		if (dma_mode) {
			modecfg |= S3C64XX_SPI_MODE_RXDMA_ON;
			chcfg |= S3C64XX_SPI_CH_RXCH_ON;
			writel(((xfer->len * 8 / sdd->cur_bpw) & 0xffff)
					| S3C64XX_SPI_PACKET_CNT_EN,
					regs + S3C64XX_SPI_PACKET_CNT);
		}
	}

	writel(modecfg, regs + S3C64XX_SPI_MODE_CFG);
	writel(chcfg, regs + S3C64XX_SPI_CH_CFG);
}

static inline void enable_cs(struct s3c64xx_spi_driver_data *sdd,
						struct spi_device *spi)
{
	struct s3c64xx_spi_csinfo *cs;

	if (sdd->tgl_spi != NULL) { /* If last device toggled after mssg */
		if (sdd->tgl_spi != spi) { /* if last mssg on diff device */
			/* Deselect the last toggled device */
			cs = sdd->tgl_spi->controller_data;
			cs->set_level(cs->line,
					spi->mode & SPI_CS_HIGH ? 0 : 1);
		}
		sdd->tgl_spi = NULL;
	}

	cs = spi->controller_data;
	cs->set_level(cs->line, spi->mode & SPI_CS_HIGH ? 1 : 0);
}

static int wait_for_xfer(struct s3c64xx_spi_driver_data *sdd,
				struct spi_transfer *xfer, int dma_mode)
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	void __iomem *regs = sdd->regs;
	unsigned long val;
	int ms;

	/* millisecs to xfer 'len' bytes @ 'cur_speed' */
	ms = xfer->len * 8 * 1000 / sdd->cur_speed;

	if (dma_mode) {
		ms += 1000; /* some tolerance */
		val = msecs_to_jiffies(ms) + 10;
		val = wait_for_completion_timeout(&sdd->xfer_completion, val);
	} else {
		u32 status;
		ms += 10000; /* some tolerance */
		val = msecs_to_loops(ms);
		do {
			status = readl(regs + S3C64XX_SPI_STATUS);
		} while (RX_FIFO_LVL(status, sci) < xfer->len && --val);
	}

	if (!val)
		return -EIO;

	if (dma_mode) {
		u32 status;

		/*
		 * DmaTx returns after simply writing data in the FIFO,
		 * w/o waiting for real transmission on the bus to finish.
		 * DmaRx returns only after Dma read data from FIFO which
		 * needs bus transmission to finish, so we don't worry if
		 * Xfer involved Rx(with or without Tx).
		 */
		if (xfer->rx_buf == NULL) {
			val = msecs_to_loops(10);
			status = readl(regs + S3C64XX_SPI_STATUS);
			
			while ((TX_FIFO_LVL(status, sci)
				|| !S3C64XX_SPI_ST_TX_DONE(status, sci))
					&& --val) {
				cpu_relax();
				status = readl(regs + S3C64XX_SPI_STATUS);
			}

			if (!val)
				return -EIO;
			
		}
		else
		{

				status = readl(regs + S3C64XX_SPI_STATUS);
		}
	} else {
		/* If it was only Tx */
		if (xfer->rx_buf == NULL) {
			sdd->state &= ~TXBUSY;
			return 0;
		}

		switch (sdd->cur_bpw) {
		case 32:
			ioread32_rep(regs + S3C64XX_SPI_RX_DATA,
				xfer->rx_buf, xfer->len / 4);
			break;
		case 16:
			ioread16_rep(regs + S3C64XX_SPI_RX_DATA,
				xfer->rx_buf, xfer->len / 2);
			break;
		default:
			ioread8_rep(regs + S3C64XX_SPI_RX_DATA,
				xfer->rx_buf, xfer->len);
			break;
		}
		sdd->state &= ~RXBUSY;
	}

	return 0;
}

static inline void disable_cs(struct s3c64xx_spi_driver_data *sdd,
						struct spi_device *spi)
{
	struct s3c64xx_spi_csinfo *cs = spi->controller_data;

	if (sdd->tgl_spi == spi)
		sdd->tgl_spi = NULL;

	cs->set_level(cs->line, spi->mode & SPI_CS_HIGH ? 0 : 1);
}

static void s3c64xx_spi_config(struct s3c64xx_spi_driver_data *sdd,struct s3c64xx_spi_csinfo *cs )
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	void __iomem *regs = sdd->regs;
	u32 val;

	/* Disable Clock */
	if (sci->clk_from_cmu) {
		clk_disable(sdd->src_clk);
	} else {
		val = readl(regs + S3C64XX_SPI_CLK_CFG);
		val &= ~S3C64XX_SPI_ENCLK_ENABLE;
		writel(val, regs + S3C64XX_SPI_CLK_CFG);
	}

	/* Set Polarity and Phase */
	val = readl(regs + S3C64XX_SPI_CH_CFG);
	val &= ~(S3C64XX_SPI_CH_SLAVE |
			S3C64XX_SPI_CPOL_L |
			S3C64XX_SPI_CPHA_B);

	if(cs->hierarchy == SSP_SLAVE)
		val |= S3C64XX_SPI_CH_SLAVE;

	if (sdd->cur_mode & SPI_CPOL)
		val |= S3C64XX_SPI_CPOL_L;

	if (sdd->cur_mode & SPI_CPHA)
		val |= S3C64XX_SPI_CPHA_B;

	writel(val, regs + S3C64XX_SPI_CH_CFG);

	/* Set Channel & DMA Mode */
	val = readl(regs + S3C64XX_SPI_MODE_CFG);
	val &= ~(S3C64XX_SPI_MODE_BUS_TSZ_MASK
			| S3C64XX_SPI_MODE_CH_TSZ_MASK);

	switch (sdd->cur_bpw) {
	case 32:
		val |= S3C64XX_SPI_MODE_BUS_TSZ_WORD;
		val |= S3C64XX_SPI_MODE_CH_TSZ_WORD;
		break;
	case 16:
		val |= S3C64XX_SPI_MODE_BUS_TSZ_HALFWORD;
		val |= S3C64XX_SPI_MODE_CH_TSZ_HALFWORD;
		break;
	default:
		val |= S3C64XX_SPI_MODE_BUS_TSZ_BYTE;
		val |= S3C64XX_SPI_MODE_CH_TSZ_BYTE;
		break;
	}

	writel(val, regs + S3C64XX_SPI_MODE_CFG);

	if (sci->clk_from_cmu) {
		/* Configure Clock */
		/* There is half-multiplier before the SPI */
		clk_set_rate(sdd->src_clk, sdd->cur_speed * 2);
		/* Enable Clock */
		clk_enable(sdd->src_clk);
	} else {
		/* Configure Clock */
		val = readl(regs + S3C64XX_SPI_CLK_CFG);
		val &= ~S3C64XX_SPI_PSR_MASK;
		val |= ((clk_get_rate(sdd->src_clk) / sdd->cur_speed / 2 - 1)
				& S3C64XX_SPI_PSR_MASK);
		writel(val, regs + S3C64XX_SPI_CLK_CFG);

		/* Enable Clock */
		val = readl(regs + S3C64XX_SPI_CLK_CFG);
		val |= S3C64XX_SPI_ENCLK_ENABLE;
		writel(val, regs + S3C64XX_SPI_CLK_CFG);
	}
}

#define XFER_DMAADDR_INVALID DMA_BIT_MASK(32)

static int s3c64xx_spi_map_mssg(struct s3c64xx_spi_driver_data *sdd,
						struct spi_message *msg)
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	struct device *dev = &sdd->pdev->dev;
	struct spi_transfer *xfer;

	if (msg->is_dma_mapped)
		return 0;

	/* First mark all xfer unmapped */
	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		xfer->rx_dma = XFER_DMAADDR_INVALID;
		xfer->tx_dma = XFER_DMAADDR_INVALID;
	}

	/* Map until end or first fail */
	list_for_each_entry(xfer, &msg->transfers, transfer_list) {

		if (xfer->len <= ((sci->fifo_lvl_mask >> 1) + 1))
			continue;

		if (xfer->tx_buf != NULL) {
			xfer->tx_dma = dma_map_single(dev,
					(void *)xfer->tx_buf, xfer->len,
					DMA_TO_DEVICE);
			if (dma_mapping_error(dev, xfer->tx_dma)) {
				dev_err(dev, "dma_map_single Tx failed\n");
				xfer->tx_dma = XFER_DMAADDR_INVALID;
				return -ENOMEM;
			}
		}

		if (xfer->rx_buf != NULL) {
			xfer->rx_dma = dma_map_single(dev, xfer->rx_buf,
						xfer->len, DMA_FROM_DEVICE);
			if (dma_mapping_error(dev, xfer->rx_dma)) {
				dev_err(dev, "dma_map_single Rx failed\n");
				dma_unmap_single(dev, xfer->tx_dma,
						xfer->len, DMA_TO_DEVICE);
				xfer->tx_dma = XFER_DMAADDR_INVALID;
				xfer->rx_dma = XFER_DMAADDR_INVALID;
				return -ENOMEM;
			}
		}
	}

	return 0;
}

/**
 * giveback - current spi_message is over, schedule next message and call
 * callback of this message. Assumes that caller already
 * set message->status; dma and pio irqs are blocked
 * @pl022: SSP driver private data structure
 */
static void giveback(struct s3c64xx_spi_driver_data *sdd)
{
	struct spi_transfer *last_transfer;
	//sdd->next_msg_cs_active = false;
	last_transfer = list_entry(sdd->cur_msg->transfers.prev,
					struct spi_transfer,
					transfer_list);

	/* Delay if requested before any change in chip select */
	if (last_transfer->delay_usecs)
		/*
		 * FIXME: This runs in interrupt context.
		 * Is this really smart?
		 */
		udelay(last_transfer->delay_usecs);

	if (!last_transfer->cs_change) {
		struct spi_message *next_msg;

		/*
		 * cs_change was not set. We can keep the chip select
		 * enabled if there is message in the queue and it is
		 * for the same spi device.
		 *
		 * We cannot postpone this until pump_messages, because
		 * after calling msg->complete (below) the driver that
		 * sent the current message could be unloaded, which
		 * could invalidate the cs_control() callback...
		 */
		/* get a pointer to the next message, if any */
		next_msg = spi_get_next_queued_message(sdd->master);

		/*
		 * see if the next and current messages point
		 * to the same spi device.
		 */
		if (next_msg && next_msg->spi != sdd->cur_msg->spi)
			next_msg = NULL;
		//if (!next_msg || sdd->cur_msg->state == STATE_ERROR)
		//	sdd->cur_chip->cs_control(SSP_CHIP_DESELECT);
		//else
		//	sdd->next_msg_cs_active = true;

	}

	sdd->cur_msg = NULL;
	sdd->cur_transfer = NULL;
}
/**
 * This sets up the pointers to memory for the next message to
 * send out on the SPI bus.
 */
static int set_up_next_transfer(struct s3c64xx_spi_driver_data *sdd,
				struct spi_transfer *transfer)
{
	int residue;

	/* Sanity check the message for this bus width */
	residue = sdd->cur_transfer->len % 1;// sdd->cur_chip->n_bytes;
	
	if (unlikely(residue != 0)) {
		dev_err(&sdd->pdev->dev,
			"message of %u bytes to transmit but the current "
			"chip bus has a data width of %u bytes!\n",
			sdd->cur_transfer->len,
			//sdd->cur_chip->n_bytes);
			1);
		dev_err(&sdd->pdev->dev, "skipping this message\n");
		return -EIO;
	}
	sdd->tx = (void *)transfer->tx_buf;
	sdd->tx_end = sdd->tx + sdd->cur_transfer->len;
	sdd->rx = (void *)transfer->rx_buf;
	sdd->rx_end = sdd->rx + sdd->cur_transfer->len;
	return 0;
}


static void pump_transfers(unsigned long data)
{
	struct s3c64xx_spi_driver_data *sdd = (struct s3c64xx_spi_driver_data *) data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;

	/* Get current state information */
	message = sdd->cur_msg;
	transfer = sdd->cur_transfer;

	/* Handle for abort */
	if (message->state == STATE_ERROR) {
		message->status = -EIO;
		giveback(sdd);
		return;
	}

	/* Handle end of message */
	if (message->state == STATE_DONE) {
		message->status = 0;
		giveback(sdd);
		return;
	}

	/* Delay if requested at end of transfer before CS change */
	if (message->state == STATE_RUNNING) {
		previous = list_entry(transfer->transfer_list.prev,
					struct spi_transfer,
					transfer_list);
		if (previous->delay_usecs)
			/*
			 * FIXME: This runs in interrupt context.
			 * Is this really smart?
			 */
			udelay(previous->delay_usecs);
		/* Reselect chip select only if cs_change was requested */
		if (previous->cs_change)
		//	sdd->cur_chip->cs_control(SSP_CHIP_SELECT);
			enable_cs(sdd,sdd->tgl_spi);
		} else {
		/* STATE_START */
		message->state = STATE_RUNNING;
	}

	if (set_up_next_transfer(sdd, transfer)) {
		message->state = STATE_ERROR;
		message->status = -EIO;
		giveback(sdd);
		return;
	}
	/* Flush the FIFOs and let's go! */
	//flush_fifo(sdd);

	if (sdd->cntrlr_info->enable_dma) {
		if (configure_dma(sdd, transfer)) {
			dev_dbg(&sdd->pdev->dev,
				"configuration of DMA failed, fall back to interrupt mode\n");
			goto err_config_dma;
		}
		return;
	}

err_config_dma:
	/* enable all interrupts except RX */
	/* add  spi int off */
	
     writel(~S3C64XX_SPI_INT_RX_OVERRUN_EN | ~S3C64XX_SPI_INT_RX_UNDERRUN_EN |
            ~S3C64XX_SPI_INT_TX_OVERRUN_EN | ~S3C64XX_SPI_INT_TX_UNDERRUN_EN,
            sdd->regs + S3C64XX_SPI_INT_EN);

}

static int dma_probe(struct s3c64xx_spi_driver_data *sdd)
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;

	dma_cap_mask_t mask;

	/* Try to acquire a generic DMA engine slave channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	sdd->dma_rx_channel = dma_request_channel(mask,
	                  sci->dma_filter,
	                  sci->dma_rx_param);

 	if (!sdd->dma_rx_channel) {
		dev_dbg(&sdd->pdev->dev, "no RX DMA channel!\n");
		goto err_no_rxchan;
	}

	sdd->dma_tx_channel = dma_request_channel(mask,
	                     sci->dma_filter,
	                     sci->dma_tx_param);

	if (!sdd->dma_tx_channel) {
	    dev_dbg(&sdd->pdev->dev, "no TX DMA channel!\n");
	    goto err_no_txchan;
    }
	
	sdd->dummypage = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!sdd->dummypage) {
	 	dev_dbg(&sdd->pdev->dev, "no DMA dummypage!\n");
	   	 goto err_no_dummypage;
	}    

	return 0;
err_no_dummypage:
	dma_release_channel(sdd->dma_tx_channel);
err_no_txchan:
    dma_release_channel(sdd->dma_rx_channel);
    sdd->dma_rx_channel = NULL;
err_no_rxchan:
    dev_err(&sdd->pdev->dev,
    		"Failed to work in dma mode, work without dma!\n");

	return -ENODEV;
}
static int s3c64xx_spi_transfer_one_message(struct spi_master *master,
					    struct spi_message *msg)
{
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(master);
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	struct spi_device *spi = msg->spi;
	struct s3c64xx_spi_csinfo *cs = spi->controller_data;
	struct spi_transfer *xfer;
	int status = 0, cs_toggle = 0;
	u32 speed;
	u8 bpw;
	
	sdd->cur_msg = msg;
	msg->state = STATE_START;
	sdd->cur_transfer = list_entry(msg->transfers.next,
	  	                struct spi_transfer, transfer_list);

	/* If Master's(controller) state differs from that needed by Slave */
	if (sdd->cur_speed != spi->max_speed_hz
			|| sdd->cur_mode != spi->mode
			|| sdd->cur_bpw != spi->bits_per_word) {
		sdd->cur_bpw = spi->bits_per_word;
		sdd->cur_speed = spi->max_speed_hz;
		sdd->cur_mode = spi->mode;
		
			    /* Setup the SPI using the per chip configuration */

		s3c64xx_spi_config(sdd, cs);
	}

	if(0 != dma_probe(sdd)){
		printk(	"DMA : DMA CH set err!\n");
	}
	/* Map all the transfers if needed */
	if (s3c64xx_spi_map_mssg(sdd, msg)) {
		dev_err(&spi->dev,
			"Xfer: Unable to map message buffers!\n");
		status = -ENOMEM;
		goto out;
	}

	/* Configure feedback delay */
	writel(cs->fb_delay & 0x3, sdd->regs + S3C64XX_SPI_FB_CLK);

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {

		unsigned long flags;
		int use_dma;

		INIT_COMPLETION(sdd->xfer_completion);

		/* Only BPW and Speed may change across transfers */
		bpw = xfer->bits_per_word ? : spi->bits_per_word;
		speed = xfer->speed_hz ? : spi->max_speed_hz;

		if (xfer->len % (bpw / 8)) {
			dev_err(&spi->dev,
				"Xfer length(%u) not a multiple of word size(%u)\n",
				xfer->len, bpw / 8);
			status = -EIO;
			goto out;
		}

		if (bpw != sdd->cur_bpw || speed != sdd->cur_speed) {
			sdd->cur_bpw = bpw;
			sdd->cur_speed = speed;
			s3c64xx_spi_config(sdd,cs);
		}

		/* Polling method for xfers not bigger than FIFO capacity */
		if (xfer->len <= ((sci->fifo_lvl_mask >> 1) + 1))
			use_dma = 0;
		else
			use_dma = 1;
			//use_dma = 0;

		//set_up_next_transfer(sdd,xfer);
		spin_lock_irqsave(&sdd->lock, flags);

		/* Pending only which is to be done */
		sdd->state &= ~RXBUSY;
		sdd->state &= ~TXBUSY;

		enable_datapath(sdd, spi, xfer, use_dma);
		/* Slave Select */
		//if(cs->hierarchy == SSP_MASTER)
		enable_cs(sdd, spi);

		/* Start the signals */
		if(cs->hierarchy == SSP_MASTER)
		S3C64XX_SPI_ACT(sdd);
		else
		S3C64XX_SPI_AUTO(sdd);

		spin_unlock_irqrestore(&sdd->lock, flags);
		
		status = wait_for_xfer(sdd, xfer, use_dma);

		/* Quiese the signals */
		S3C64XX_SPI_DEACT(sdd);
		
		if (status) {
			dev_err(&spi->dev, "I/O Error: "
				"rx-%d tx-%d res:rx-%c tx-%c len-%d\n",
				xfer->rx_buf ? 1 : 0, xfer->tx_buf ? 1 : 0,
				(sdd->state & RXBUSY) ? 'f' : 'p',
				(sdd->state & TXBUSY) ? 'f' : 'p',
				xfer->len);

			if(use_dma) {
				stop_dma(sdd);
			}
			goto out;
		}
		if (xfer->delay_usecs)
			udelay(xfer->delay_usecs);

		if (xfer->cs_change) {
			/* Hint that the next mssg is gonna be
			   for the same device */
			if (list_is_last(&xfer->transfer_list,
						&msg->transfers))
				cs_toggle = 1;
		}

		msg->actual_length += xfer->len;

	}

out:
	flush_fifo(sdd);
	if (!cs_toggle || status)
		disable_cs(sdd, spi);
	else
		sdd->tgl_spi = spi;
	
		/* Pending only which is to be done */
		sdd->state &= ~RXBUSY;
		sdd->state &= ~TXBUSY;

	msg->status = status;

	/* Free DMA channels */

	dma_release_channel(sdd->dma_tx_channel);
	dma_release_channel(sdd->dma_rx_channel);	
	kfree(sdd->dummypage);

	spi_finalize_current_message(master);

	return 0;
}

static int s3c64xx_spi_prepare_transfer(struct spi_master *spi)
{
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(spi);

	pm_runtime_get_sync(&sdd->pdev->dev);

	return 0;
}

static int s3c64xx_spi_unprepare_transfer(struct spi_master *spi)
{
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(spi);

	pm_runtime_put(&sdd->pdev->dev);

	return 0;
}

/*
 * Here we only check the validity of requested configuration
 * and save the configuration in a local data-structure.
 * The controller is actually configured only just before we
 * get a message to transfer.
 */
static int s3c64xx_spi_setup(struct spi_device *spi)
{
	struct s3c64xx_spi_csinfo *cs = spi->controller_data;
	struct s3c64xx_spi_driver_data *sdd;
	struct s3c64xx_spi_info *sci;
	struct spi_message *msg;
	unsigned long flags;
	int err = 0;
	
	if (cs == NULL || cs->set_level == NULL) {
		dev_err(&spi->dev, "No CS for SPI(%d)\n", spi->chip_select);
		return -ENODEV;
	}

	sdd = spi_master_get_devdata(spi->master);
	sci = sdd->cntrlr_info;

	spin_lock_irqsave(&sdd->lock, flags);

	list_for_each_entry(msg, &sdd->queue, queue) {
		/* Is some mssg is already queued for this device */
		if (msg->spi == spi) {
			dev_err(&spi->dev,
				"setup: attempt while mssg in queue!\n");
			spin_unlock_irqrestore(&sdd->lock, flags);
			return -EBUSY;
		}
	}

	spin_unlock_irqrestore(&sdd->lock, flags);

	if (spi->bits_per_word != 8
			&& spi->bits_per_word != 16
			&& spi->bits_per_word != 32) {
		dev_err(&spi->dev, "setup: %dbits/wrd not supported!\n",
							spi->bits_per_word);
		err = -EINVAL;
		goto setup_exit;
	}

	pm_runtime_get_sync(&sdd->pdev->dev);

	/* Check if we can provide the requested rate */
	if (!sci->clk_from_cmu) {
		u32 psr, speed;

		/* Max possible */
		speed = clk_get_rate(sdd->src_clk) / 2 / (0 + 1);

		if (spi->max_speed_hz > speed)
			spi->max_speed_hz = speed;

		psr = clk_get_rate(sdd->src_clk) / 2 / spi->max_speed_hz - 1;
		psr &= S3C64XX_SPI_PSR_MASK;
		if (psr == S3C64XX_SPI_PSR_MASK)
			psr--;

		speed = clk_get_rate(sdd->src_clk) / 2 / (psr + 1);
		if (spi->max_speed_hz < speed) {
			if (psr+1 < S3C64XX_SPI_PSR_MASK) {
				psr++;
			} else {
				err = -EINVAL;
				goto setup_exit;
			}
		}

		speed = clk_get_rate(sdd->src_clk) / 2 / (psr + 1);
		if (spi->max_speed_hz >= speed)
			spi->max_speed_hz = speed;
		else
			err = -EINVAL;
	}

	pm_runtime_put(&sdd->pdev->dev);

setup_exit:

	/* setup() returns with device de-selected */
	disable_cs(sdd, spi);

	return err;
}

static irqreturn_t s3c64xx_spi_irq(int irq, void *data)
{
	struct s3c64xx_spi_driver_data *sdd = data;
	struct spi_master *spi = sdd->master;
	//struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	unsigned int val, status;

	val = readl(sdd->regs + S3C64XX_SPI_PENDING_CLR);
	status = readl(sdd->regs + S3C64XX_SPI_STATUS);

	val &= S3C64XX_SPI_PND_RX_OVERRUN_CLR |
		S3C64XX_SPI_PND_RX_UNDERRUN_CLR |
		S3C64XX_SPI_PND_TX_OVERRUN_CLR |
		S3C64XX_SPI_PND_TX_UNDERRUN_CLR;

	if(status &  S3C64XX_SPI_ST_RX_OVERRUN_ERR) 
		val |= S3C64XX_SPI_PND_RX_OVERRUN_CLR;
	if(status &  S3C64XX_SPI_ST_RX_UNDERRUN_ERR) 
		val |= S3C64XX_SPI_PND_RX_UNDERRUN_CLR;
	if(status &  S3C64XX_SPI_ST_TX_OVERRUN_ERR) 
		val |= S3C64XX_SPI_PND_TX_OVERRUN_CLR;
	if(status &  S3C64XX_SPI_ST_TX_UNDERRUN_ERR) 
		val |= S3C64XX_SPI_PND_TX_UNDERRUN_CLR;

	writel(val, sdd->regs + S3C64XX_SPI_PENDING_CLR);

	if (val & S3C64XX_SPI_PND_RX_OVERRUN_CLR)
		dev_err(&spi->dev, "RX overrun\n");
	if (val & S3C64XX_SPI_PND_RX_UNDERRUN_CLR)
		dev_err(&spi->dev, "RX underrun\n");
	if (val & S3C64XX_SPI_PND_TX_OVERRUN_CLR)
		dev_err(&spi->dev, "TX overrun\n");
	if (val & S3C64XX_SPI_PND_TX_UNDERRUN_CLR)
		dev_err(&spi->dev, "TX underrun\n");

	//tasklet_schedule(&sdd->pump_transfers);
	return IRQ_HANDLED;
}

static void s3c64xx_spi_hwinit(struct s3c64xx_spi_driver_data *sdd, int channel)
{
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;
	void __iomem *regs = sdd->regs;
	unsigned int val;
	
	sdd->cur_speed = 0;

	S3C64XX_SPI_DEACT(sdd);

	/* Disable Interrupts - we use Polling if not DMA mode */
	writel(0, regs + S3C64XX_SPI_INT_EN);

	if (!sci->clk_from_cmu)
		writel(sci->src_clk_nr << S3C64XX_SPI_CLKSEL_SRCSHFT,
				regs + S3C64XX_SPI_CLK_CFG);
	writel(0, regs + S3C64XX_SPI_MODE_CFG);
	writel(0, regs + S3C64XX_SPI_PACKET_CNT);

	/* Clear any irq pending bits */
	writel(readl(regs + S3C64XX_SPI_PENDING_CLR),
				regs + S3C64XX_SPI_PENDING_CLR);

	writel(0, regs + S3C64XX_SPI_SWAP_CFG);

	val = readl(regs + S3C64XX_SPI_MODE_CFG);
	val &= ~S3C64XX_SPI_MODE_4BURST;
	val &= ~(S3C64XX_SPI_MAX_TRAILCNT << S3C64XX_SPI_TRAILCNT_OFF);
	val |= (S3C64XX_SPI_TRAILCNT << S3C64XX_SPI_TRAILCNT_OFF);
	writel(val, regs + S3C64XX_SPI_MODE_CFG);

	flush_fifo(sdd);
}

static int __init s3c64xx_spi_probe(struct platform_device *pdev)
{
	struct resource	*mem_res, *dmatx_res, *dmarx_res;
	struct s3c64xx_spi_driver_data *sdd;
	struct s3c64xx_spi_info *sci;
	struct spi_master *master;
	int ret, irq;
	char clk_name[16];
	
	if (pdev->id < 0) {
		dev_err(&pdev->dev,
				"Invalid platform device id-%d\n", pdev->id);
		return -ENODEV;
	}

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "platform_data missing!\n");
		return -ENODEV;
	}

	sci = pdev->dev.platform_data;
	sci->spi_init(pdev->id);

	/* Check for availability of necessary resource */

	dmatx_res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (dmatx_res == NULL) {
		dev_err(&pdev->dev, "Unable to get SPI-Tx dma resource\n");
		return -ENXIO;
	}

	dmarx_res = platform_get_resource(pdev, IORESOURCE_DMA, 1);
	if (dmarx_res == NULL) {
		dev_err(&pdev->dev, "Unable to get SPI-Rx dma resource\n");
		return -ENXIO;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res == NULL) {
		dev_err(&pdev->dev, "Unable to get SPI MEM resource\n");
		return -ENXIO;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_warn(&pdev->dev, "Failed to get IRQ: %d\n", irq);
		return irq;
	}

	master = spi_alloc_master(&pdev->dev,
				sizeof(struct s3c64xx_spi_driver_data));
	if (master == NULL) {
		dev_err(&pdev->dev, "Unable to allocate SPI Master\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, master);

	sdd = spi_master_get_devdata(master);
	sdd->master = master;
	sdd->cntrlr_info = sci;
	sdd->pdev = pdev;
	sdd->sfr_start = mem_res->start;

	sdd->cur_bpw = 8;

	master->bus_num = pdev->id;
	master->setup = s3c64xx_spi_setup;
	master->prepare_transfer_hardware = s3c64xx_spi_prepare_transfer;
	master->transfer_one_message = s3c64xx_spi_transfer_one_message;
	master->unprepare_transfer_hardware = s3c64xx_spi_unprepare_transfer;
	master->num_chipselect = sci->num_cs;
	master->dma_alignment = 8;
	/* the spi->mode bits understood by this driver: */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
//	if(0!=dma_probe(sdd)){
//		printk(	"DMA : DMA CH set err!\n");
//	}

	if (request_mem_region(mem_res->start,
			resource_size(mem_res), pdev->name) == NULL) {
		dev_err(&pdev->dev, "Req mem region failed\n");
		ret = -ENXIO;
		goto err0;
	}

	sdd->regs = ioremap(mem_res->start, resource_size(mem_res));
	sdd->phybase = mem_res->start;
	
	if (sdd->regs == NULL) {
		dev_err(&pdev->dev, "Unable to remap IO\n");
		ret = -ENXIO;
		goto err1;
	}

	if (sci->cfg_gpio == NULL || sci->cfg_gpio(pdev)) {
		dev_err(&pdev->dev, "Unable to config gpio\n");
		ret = -EBUSY;
		goto err2;
	}

	/* Setup clocks */
	sdd->clk = clk_get(&pdev->dev, "spi");
	if (IS_ERR(sdd->clk)) {
		dev_err(&pdev->dev, "Unable to acquire clock 'spi'\n");
		ret = PTR_ERR(sdd->clk);
		goto err3;
	}

	sprintf(clk_name, "nxp_spi%d", sci->src_clk_nr);
	sdd->src_clk = clk_get(&pdev->dev, clk_name);
	if (IS_ERR(sdd->src_clk)) {
		dev_err(&pdev->dev,
			"Unable to acquire clock '%s'\n", clk_name);
		ret = PTR_ERR(sdd->src_clk);
		goto err4;
	}

	pm_runtime_enable(&pdev->dev);

#if defined(CONFIG_PM_RUNTIME)
	pm_runtime_get_sync(&pdev->dev);
#else
	clk_enable(sdd->clk);
	clk_enable(sdd->src_clk);
#endif

	/* Setup Deufult Mode */
	s3c64xx_spi_hwinit(sdd, pdev->id);

	spin_lock_init(&sdd->lock);
	init_completion(&sdd->xfer_completion);
	INIT_LIST_HEAD(&sdd->queue);
 	
	tasklet_init(&sdd->pump_transfers, pump_transfers,
			(unsigned long)sdd);


	ret = request_irq(irq, s3c64xx_spi_irq, 0, "spi-s3c64xx", sdd);
	if (ret != 0) {
		dev_err(&pdev->dev, "Failed to request IRQ %d: %d\n",
			irq, ret);
		goto err5;
	}

	writel(S3C64XX_SPI_INT_RX_OVERRUN_EN | S3C64XX_SPI_INT_RX_UNDERRUN_EN |
	       S3C64XX_SPI_INT_TX_OVERRUN_EN ,//| S3C64XX_SPI_INT_TX_UNDERRUN_EN,
	       sdd->regs + S3C64XX_SPI_INT_EN);

	if (spi_register_master(master)) {
		dev_err(&pdev->dev, "cannot register SPI master\n");
		ret = -EBUSY;
		goto err6;
	}

	dev_dbg(&pdev->dev, "Samsung SoC SPI Driver loaded for Bus SPI-%d "
					"with %d Slaves attached\n",
					pdev->id, master->num_chipselect);
//	dev_dbg(&pdev->dev, "\tIOmem=[0x%x-0x%x]\tDMA=[Rx-%d, Tx-%d]\n",
//					mem_res->end, mem_res->start,
//					sdd->rx_dma.dmach, sdd->tx_dma.dmach);

	pm_runtime_put(&pdev->dev);

	return 0;

err6:
	free_irq(irq, sdd);
err5:
#if defined(CONFIG_PM_RUNTIME)
	pm_runtime_put(&pdev->dev);
#else
	clk_disable(sdd->src_clk);
	clk_disable(sdd->clk);
#endif
	pm_runtime_disable(&pdev->dev);
	clk_put(sdd->src_clk);
err4:
	clk_put(sdd->clk);
err3:
err2:
	iounmap((void *) sdd->regs);
err1:
	release_mem_region(mem_res->start, resource_size(mem_res));
err0:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return ret;
}

static int s3c64xx_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = spi_master_get(platform_get_drvdata(pdev));
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(master);
	struct resource	*mem_res;

	spi_unregister_master(master);

	writel(0, sdd->regs + S3C64XX_SPI_INT_EN);

	free_irq(platform_get_irq(pdev, 0), sdd);

#ifndef CONFIG_PM_RUNTIME
	clk_disable(sdd->src_clk);
	clk_disable(sdd->clk);
#endif

	pm_runtime_disable(&pdev->dev);

	clk_put(sdd->src_clk);
	clk_put(sdd->clk);

	iounmap((void *) sdd->regs);

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res != NULL)
		release_mem_region(mem_res->start, resource_size(mem_res));

	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return 0;
}

#ifdef CONFIG_PM
static int s3c64xx_spi_suspend(struct device *dev)
{
	struct spi_master *master = spi_master_get(dev_get_drvdata(dev));
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(master);

	spi_master_suspend(master);

#ifndef CONFIG_PM_RUNTIME
	/* Disable the clock */
	clk_disable(sdd->src_clk);
	clk_disable(sdd->clk);
#endif

	clk_disable(sdd->src_clk);
	clk_disable(sdd->clk);
	sdd->cur_speed = 0; /* Output Clock is stopped */

	return 0;
}

static int s3c64xx_spi_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct spi_master *master = spi_master_get(dev_get_drvdata(dev));
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(master);
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;

	sci->cfg_gpio(pdev);

#if defined(CONFIG_PM_RUNTIME)
	pm_runtime_get_sync(&sdd->pdev->dev);
#else
	/* Enable the clock */
	clk_enable(sdd->src_clk);
	clk_enable(sdd->clk);
#endif
	sci->spi_init(pdev->id);
	s3c64xx_spi_hwinit(sdd, pdev->id);

	spi_master_resume(master);

	pm_runtime_put(&sdd->pdev->dev);

	return 0;
}
#endif /* CONFIG_PM */

#ifdef CONFIG_PM_RUNTIME
static int s3c64xx_spi_runtime_suspend(struct device *dev)
{
	struct spi_master *master = spi_master_get(dev_get_drvdata(dev));
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(master);
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;

	if (sci->gpio_pull_up)
		sci->gpio_pull_up(false);

	clk_disable(sdd->clk);
	clk_disable(sdd->src_clk);

	return 0;
}

static int s3c64xx_spi_runtime_resume(struct device *dev)
{
	struct spi_master *master = spi_master_get(dev_get_drvdata(dev));
	struct s3c64xx_spi_driver_data *sdd = spi_master_get_devdata(master);
	struct s3c64xx_spi_info *sci = sdd->cntrlr_info;

	clk_enable(sdd->src_clk);
	clk_enable(sdd->clk);

	if (sci->gpio_pull_up)
		sci->gpio_pull_up(true);
	
	return 0;
}
#endif /* CONFIG_PM_RUNTIME */

static const struct dev_pm_ops s3c64xx_spi_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(s3c64xx_spi_suspend, s3c64xx_spi_resume)
	SET_RUNTIME_PM_OPS(s3c64xx_spi_runtime_suspend,
			   s3c64xx_spi_runtime_resume, NULL)
};

static struct platform_driver s3c64xx_spi_driver = {
	.driver = {
		.name	= "s3c64xx-spi",
		.owner = THIS_MODULE,
		.pm = &s3c64xx_spi_pm,
	},
	.remove = s3c64xx_spi_remove,
};
MODULE_ALIAS("platform:s3c64xx-spi");

static int __init s3c64xx_spi_init(void)
{
	return platform_driver_probe(&s3c64xx_spi_driver, s3c64xx_spi_probe);
}
subsys_initcall(s3c64xx_spi_init);

static void __exit s3c64xx_spi_exit(void)
{
	platform_driver_unregister(&s3c64xx_spi_driver);
}
module_exit(s3c64xx_spi_exit);

MODULE_AUTHOR("Jaswinder Singh <jassi.brar@samsung.com>");
MODULE_DESCRIPTION("S3C64XX SPI Controller Driver");
MODULE_LICENSE("GPL");
