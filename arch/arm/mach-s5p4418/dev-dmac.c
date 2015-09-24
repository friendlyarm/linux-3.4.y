/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/amba/bus.h>
#include <mach/devices.h>

/*
 * AMBA-PL08X DMA platform device
 */
#if defined(CONFIG_AMBA_PL08X)
#define PRIMECELL_SLAVE_CCTL 	(PL080_BSIZE_8 << PL080_CONTROL_SB_SIZE_SHIFT | \
			PL080_BSIZE_8 << PL080_CONTROL_DB_SIZE_SHIFT | \
			PL080_WIDTH_32BIT << PL080_CONTROL_SWIDTH_SHIFT | \
			PL080_WIDTH_32BIT << PL080_CONTROL_DWIDTH_SHIFT | \
			PL080_CONTROL_PROT_SYS)

#define PRIMECELL_MEMORU_CCTL 	(PL080_BSIZE_256 << PL080_CONTROL_SB_SIZE_SHIFT |	\
			PL080_BSIZE_256 << PL080_CONTROL_DB_SIZE_SHIFT |	\
		 	PL080_WIDTH_32BIT << PL080_CONTROL_SWIDTH_SHIFT |	\
			PL080_WIDTH_32BIT << PL080_CONTROL_DWIDTH_SHIFT |	\
		 	PL080_CONTROL_PROT_BUFF |				\
		 	PL080_CONTROL_PROT_CACHE |				\
		 	PL080_CONTROL_PROT_SYS)

/* Get DMA Peripheral ID */
static int pl080_get_signal(struct pl08x_dma_chan *ch)
{
	struct pl08x_channel_data *cd = (struct pl08x_channel_data *)ch->cd;
	return cd->min_signal;	/* return Peripheral ID */
}

/* DMA CH 0 */
static struct pl08x_channel_data dmac0_channels[] = {
	[0] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_UART1_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_UART1_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[1] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_UART1_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_UART1_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[2] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_UART0_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_UART0_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[3] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_UART0_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_UART0_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[4] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_UART2_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_UART2_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[5] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_UART2_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_UART2_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[6] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SSP0_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_SSP0_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[7] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SSP0_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_SSP0_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[8] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SSP1_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_SSP1_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[9] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SSP1_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_SSP1_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[10] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SSP2_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_SSP2_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[11] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SSP2_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_SSP2_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[12] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_I2S0_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_I2S0_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[13] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_I2S0_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_I2S0_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[14] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_I2S1_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_I2S1_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[15] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_I2S1_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_I2S1_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
};

/* DMA CH 1 */
static struct pl08x_channel_data dmac1_channels[] = {
	[0] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_I2S2_TX,
		.min_signal 	= DMA_PERIPHERAL_ID_I2S2_TX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[1] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_I2S2_RX,
		.min_signal 	= DMA_PERIPHERAL_ID_I2S2_RX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[2] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_AC97_PCMOUT,
		.min_signal 	= DMA_PERIPHERAL_ID_AC97_PCMOUT,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[3] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_AC97_PCMIN,
		.min_signal 	= DMA_PERIPHERAL_ID_AC97_PCMIN,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[4] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_AC97_MICIN,
		.min_signal 	= DMA_PERIPHERAL_ID_AC97_MICIN,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[5] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SPDIFRX,
		.min_signal 	= DMA_PERIPHERAL_ID_SPDIFRX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[6] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_SPDIFTX,
		.min_signal 	= DMA_PERIPHERAL_ID_SPDIFTX,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[7] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_MPEGTSI0,
		.min_signal 	= DMA_PERIPHERAL_ID_MPEGTSI0,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[8] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_MPEGTSI1,
		.min_signal 	= DMA_PERIPHERAL_ID_MPEGTSI1,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[9] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_MPEGTSI2,
		.min_signal 	= DMA_PERIPHERAL_ID_MPEGTSI2,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[10] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_MPEGTSI3,
		.min_signal 	= DMA_PERIPHERAL_ID_MPEGTSI3,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[11] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_CRYPTO_BR,
		.min_signal 	= DMA_PERIPHERAL_ID_CRYPTO_BR,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[12] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_CRYPTO_BW,
		.min_signal 	= DMA_PERIPHERAL_ID_CRYPTO_BW,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[13] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_CRYPTO_HR,
		.min_signal 	= DMA_PERIPHERAL_ID_CRYPTO_HR,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
	[14] = {
		.bus_id 		= DMA_PERIPHERAL_NAME_PDM,
		.min_signal 	= DMA_PERIPHERAL_ID_PDM,
		.periph_buses 	= PL08X_AHB2,		// CCTL AHB Master
	},
};

struct pl08x_platform_data pl08x_dmac0_plat_data = {
	.slave_channels = dmac0_channels,
	.num_slave_channels = ARRAY_SIZE(dmac0_channels),
	.memcpy_channel = {
		.bus_id = "memcpy",
		.cctl = PRIMECELL_MEMORU_CCTL,
	},
	.get_signal = pl080_get_signal,
	.lli_buses	= PL08X_AHB1,		// LLI AHB Master (Next LLI)
	.mem_buses	= PL08X_AHB1,		// LLI AHB Master (Next LLI)
};

struct pl08x_platform_data pl08x_dmac1_plat_data = {
	.slave_channels = dmac1_channels,
	.num_slave_channels = ARRAY_SIZE(dmac1_channels),
	.memcpy_channel = {
		.bus_id = "memcpy",
		.cctl = PRIMECELL_MEMORU_CCTL,
	},
	.get_signal = pl080_get_signal,
	/* check amba-pl08x.c probe */
	.lli_buses	= PL08X_AHB1,		// DUAL MASTER AHB2 (Peri)
	.mem_buses	= PL08X_AHB1,		// DUAL MASTER AHB1 (mem)
};

static AMBA_AHB_DEVICE(dmac0, "pl08xdmac.0", 0, PHY_BASEADDR_DMA0, {IRQ_PHY_DMA0}, &pl08x_dmac0_plat_data);
static AMBA_AHB_DEVICE(dmac1, "pl08xdmac.1", 0, PHY_BASEADDR_DMA1, {IRQ_PHY_DMA1}, &pl08x_dmac1_plat_data);

#endif	// CONFIG_AMBA_PL08X

