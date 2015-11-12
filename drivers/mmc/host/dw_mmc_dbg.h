/*
 * Synopsys DesignWare Multimedia Card Interface driver
 *  (Based on NXP driver for lpc 31xx)
 *
 * Copyright (C) 2009 NXP Semiconductors
 * Copyright (C) 2009, 2010 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/bitops.h>

#include "dw_mmc.h"

/*
#define	pr_debug	printk
*/
#define	DW_MMC_EVENT_PENDING(host)	{	\
	if (host->pending_events)	{		\
		pr_debug("PEND EVENT: [0x%x] ", (unsigned)host->pending_events);	\
		if ((1<<EVENT_CMD_COMPLETE) & host->pending_events)		\
			pr_debug("CMD_COMP, ");							\
		if ((1<<EVENT_XFER_COMPLETE) & host->pending_events)		\
			pr_debug("XFR_COMP, ");							\
		if ((1<<EVENT_DATA_COMPLETE) & host->pending_events)		\
			pr_debug("DAT_COMP, ");							\
		if ((1<<EVENT_DATA_ERROR) & host->pending_events)		\
			pr_debug("DAT_ERRO, ");							\
		if ((1<<EVENT_XFER_ERROR) & host->pending_events)		\
			pr_debug("XFR_ERRO  ");							\
		pr_debug("\n");										\
	}	\
	if (host->completed_events)	{						\
		pr_debug("CMD  EVENT: [0x%x] ", (unsigned)host->completed_events);	\
		if ((1<<EVENT_CMD_COMPLETE) & host->completed_events)	\
			pr_debug("CMD_COMP, ");							\
		if ((1<<EVENT_XFER_COMPLETE) & host->completed_events)	\
			pr_debug("XFR_COMP, ");							\
		if ((1<<EVENT_DATA_COMPLETE) & host->completed_events)	\
			pr_debug("DAT_COMP, ");							\
		if ((1<<EVENT_DATA_ERROR) & host->completed_events)		\
			pr_debug("DAT_ERRO, ");							\
		if ((1<<EVENT_XFER_ERROR) & host->completed_events)		\
			pr_debug("XFR_ERRO, ");							\
	pr_debug("\n");						\
		}	\
	}

#define	DW_MMC_HOST_STATE(cnd, host) if (cnd) {\
			pr_debug("HOST STATE: ");			\
		if (STATE_IDLE == host->state)			\
			pr_debug("__IDLE__\n");				\
		if (STATE_SENDING_CMD == host->state)	\
			pr_debug("SEND_CMD\n");				\
		if (STATE_SENDING_DATA == host->state)	\
			pr_debug("SEND_DATA\n");			\
		if (STATE_DATA_BUSY == host->state)		\
			pr_debug("DATA_BUSY\n");			\
		if (STATE_SENDING_STOP == host->state)	\
			pr_debug("SEND_STOP\n");			\
		if (STATE_DATA_ERROR == host->state)	\
			pr_debug("DATA_ERRO\n");			\
		}

#define	DW_MMC_HOST_INTSTS(cnd, host)	\
	if (cnd && host->data_status)	{						\
		pr_debug("HOST RINTSTS: [0x%x] ", host->data_status);	\
		if ((1<<0) & host->data_status)	\
			pr_debug("CDINT: CARD DETECT, ");	\
		if ((1<<1) & host->data_status)	\
			pr_debug("RE: RESP ERROR, ");		\
		if ((1<<2) & host->data_status)	\
			pr_debug("CD: COMMAND DONE, ");		\
		if ((1<<3) & host->data_status)	\
			pr_debug("DTO: TRANS OVER, ");		\
		if ((1<<4) & host->data_status)	\
			pr_debug("TXDR: TR FIFO, ");		\
		if ((1<<5) & host->data_status)	\
			pr_debug("RXDR: RX FIFO, ");		\
		if ((1<<6) & host->data_status)	\
			pr_debug("RCRC: RESP CRC, ");		\
		if ((1<<7) & host->data_status)	\
			pr_debug("DCRC: DATA CRC, ");		\
		if ((1<<8) & host->data_status)	\
			pr_debug("RTO: RESP TOUT, ");		\
		if ((1<<9) & host->data_status)	\
			pr_debug("DRTO: DATA TOUT, ");		\
		if ((1<<10) & host->data_status)	\
			pr_debug("HTO: HOST TOUT, ");		\
		if ((1<<11) & host->data_status)	\
			pr_debug("FRUN: FIFO UN/OV, ");		\
		if ((1<<12) & host->data_status)	\
			pr_debug("HLE: HW LOCK, ");			\
		if ((1<<13) & host->data_status)	\
			pr_debug("SBE: START BIT ERR, ");	\
		if ((1<<14) & host->data_status)	\
			pr_debug("ACD: AUTO CMD DONE, ");	\
		if ((1<<15) & host->data_status)	\
			pr_debug("EBE: END BIT ERR, ");		\
		if ((1<<16) & host->data_status)	\
			pr_debug("SDIOINT: SDIO INT, ");	\
		pr_debug("\n");						\
		}

#define	DW_MMC_R1_RESP(resp)		{ \
	if (resp & R1_OUT_OF_RANGE		)	pr_debug(" R1:R1_OUT_OF_RANGE \n");	\
	if (resp & R1_ADDRESS_ERROR		)	pr_debug(" R1:R1_ADDRESS_ERROR \n"); \
	if (resp & R1_BLOCK_LEN_ERROR	)	pr_debug(" R1:R1_BLOCK_LEN_ERROR \n"); \
	if (resp & R1_ERASE_SEQ_ERROR   )   pr_debug(" R1:R1_ERASE_SEQ_ERROR \n"); \
	if (resp & R1_ERASE_PARAM		)	pr_debug(" R1:R1_ERASE_PARAM \n"); \
	if (resp & R1_WP_VIOLATION		)	pr_debug(" R1:R1_WP_VIOLATION \n"); \
	if (resp & R1_CARD_IS_LOCKED	)	pr_debug(" R1:R1_CARD_IS_LOCKED \n"); \
	if (resp & R1_LOCK_UNLOCK_FAILED)	pr_debug(" R1:R1_LOCK_UNLOCK_FAILED \n"); \
	if (resp & R1_COM_CRC_ERROR		)	pr_debug(" R1:R1_COM_CRC_ERROR \n"); \
	if (resp & R1_ILLEGAL_COMMAND	)	pr_debug(" R1:R1_ILLEGAL_COMMAND \n"); \
	if (resp & R1_CARD_ECC_FAILED	)	pr_debug(" R1:R1_CARD_ECC_FAILED \n"); \
	if (resp & R1_CC_ERROR			)	pr_debug(" R1:R1_CC_ERROR \n"); \
	if (resp & R1_ERROR				)	pr_debug(" R1:R1_ERROR \n"); \
	if (resp & R1_UNDERRUN			)	pr_debug(" R1:R1_UNDERRUN \n"); \
	if (resp & R1_OVERRUN			)	pr_debug(" R1:R1_OVERRUN \n"); \
	if (resp & R1_CID_CSD_OVERWRITE	)	pr_debug(" R1:R1_CID_CSD_OVERWRITE \n"); \
	if (resp & R1_WP_ERASE_SKIP		)	pr_debug(" R1:R1_WP_ERASE_SKIP \n"); \
	if (resp & R1_CARD_ECC_DISABLED	)	pr_debug(" R1:R1_CARD_ECC_DISABLED \n"); \
	if (resp & R1_ERASE_RESET		)	pr_debug(" R1:R1_ERASE_RESET \n"); \
	if (resp & R1_READY_FOR_DATA	)	pr_debug(" R1:R1_READY_FOR_DATA \n"); \
	if (resp & R1_SWITCH_ERROR		)	pr_debug(" R1:R1_SWITCH_ERROR \n"); \
	if (resp & R1_APP_CMD			)	pr_debug(" R1:R1_APP_CMD \n"); \
	}
