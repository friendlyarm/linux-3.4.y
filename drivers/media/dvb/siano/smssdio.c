/*
 *  smssdio.c - Siano 1xxx SDIO interface driver
 *
 *  Copyright 2008 Pierre Ossman
 *
 * Copyright (C) 2006-2011, Siano Mobile Silicon (Doron Cohen)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 *
 * This hardware is a bit odd in that all transfers should be done
 * to/from the SMSSDIO_DATA register, yet the "increase address" bit
 * always needs to be set.
 *
 * Also, buffers from the card are always aligned to 128 byte
 * boundaries.
 */

/*
 * General cleanup notes:
 *
 * - only typedefs should be name *_t
 *
 * - use ERR_PTR and friends for smscore_register_device()
 *
 * - smscore_getbuffer should zero fields
 *
 * Fix stop command
 */

#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>

#include "smscoreapi.h"
#include "sms-cards.h"

/* Registers */

#define SMSSDIO_DATA		0x00
#define SMSSDIO_INT		0x04
#define SMSSDIO_AHB_CNT		0x1C
#define SMSSDIO_BLOCK_SIZE	128
#define MAX_SDIO_BUF_SIZE	0x5000

static const struct sdio_device_id smssdio_ids[] __devinitconst = {
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, SDIO_DEVICE_ID_SIANO_STELLAR),
	 .driver_data = SMS1XXX_BOARD_SIANO_STELLAR},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, SDIO_DEVICE_ID_SIANO_NOVA_A0),
	 .driver_data = SMS1XXX_BOARD_SIANO_NOVA_A},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, SDIO_DEVICE_ID_SIANO_NOVA_B0),
	 .driver_data = SMS1XXX_BOARD_SIANO_NOVA_B},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, SDIO_DEVICE_ID_SIANO_VEGA_A0),
	 .driver_data = SMS1XXX_BOARD_SIANO_VEGA},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, SDIO_DEVICE_ID_SIANO_VENICE),
	 .driver_data = SMS1XXX_BOARD_SIANO_VEGA},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x302),
	 .driver_data = SMS1XXX_BOARD_SIANO_MING},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x320),
	 .driver_data = SMS1XXX_BOARD_SIANO_QING},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x400),
	 .driver_data = SMS1XXX_BOARD_SIANO_MING},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x500),
	 .driver_data = SMS1XXX_BOARD_SIANO_PELE},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x510),
	 .driver_data = SMS1XXX_BOARD_SIANO_ZICO},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x600),
	 .driver_data = SMS1XXX_BOARD_SIANO_RIO},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x610),
	 .driver_data = SMS1XXX_BOARD_SIANO_SANTOS},
    {SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x700),
	 .driver_data = SMS1XXX_BOARD_SIANO_DENVER_2160},
	{SDIO_DEVICE(SDIO_VENDOR_ID_SIANO, 0x800),
	 .driver_data = SMS1XXX_BOARD_SIANO_DENVER_1530},
	{ /* end: all zeroes */ },
};

MODULE_DEVICE_TABLE(sdio, smssdio_ids);

struct smssdio_device {
	struct sdio_func *func;
	struct work_struct work_thread;
	void *coredev;

	struct smscore_buffer_t *split_cb;
};

static u32 sdio_use_workthread;

module_param(sdio_use_workthread, int, S_IRUGO);
MODULE_PARM_DESC(sdio_use_workthread, "Use workthread for sdio interupt handling. Required for specific host drivers (defaule 0)");

static u32 sdio_plugin_delay = 0;

module_param(sdio_plugin_delay, int, S_IRUGO);
MODULE_PARM_DESC(sdio_plugin_delay, "Specify the sleep time in mSec need to wait before communicating with plugged in device. 0 means no wait. (defaule 0)");


/*******************************************************************/
/* Siano core callbacks                                            */
/*******************************************************************/

static int smssdio_sendrequest(void *context, void *buffer, size_t size)
{
	int ret = 0;
	struct smssdio_device *smsdev;
	void* auxbuf = NULL;

	smsdev = context;
	
	if (size & 3)
	{
		/* Make sure size is aligned to 32 bits, round up if required*/			
		auxbuf = kmalloc((size + 3) & 0xfffffffc, GFP_KERNEL);
		memcpy (auxbuf, buffer, size);
		buffer = auxbuf;
		size = (size + 3) & 0xfffffffc;
	}


	sdio_claim_host(smsdev->func);

	while (size >= smsdev->func->cur_blksize) {
		ret = sdio_memcpy_toio(smsdev->func, SMSSDIO_DATA,
					buffer, smsdev->func->cur_blksize);
		if (ret)
			goto out;

		buffer += smsdev->func->cur_blksize;
		size -= smsdev->func->cur_blksize;
	}

	if (size) {
		ret = sdio_memcpy_toio(smsdev->func, SMSSDIO_DATA,
					buffer, size);
	}

out:
	if (auxbuf)
		kfree(auxbuf);
	sdio_release_host(smsdev->func);

	return ret;
}

/*******************************************************************/
/* SDIO callbacks                                                  */
/*******************************************************************/


static int verify_valid_hdr(struct SmsMsgHdr_S *hdr)
{
	if ((hdr->msgType < MSG_TYPE_BASE_VAL) || (hdr->msgType > MSG_LAST_MSG_TYPE))
	{
		sms_debug("Msg has bad type");
		return -EINVAL;
	}
	if (hdr->msgLength > MAX_SDIO_BUF_SIZE)
	{
		sms_debug("Msg has bad length");
		return -EINVAL;
	}
	if ((hdr->msgFlags & MSG_HDR_FLAG_SPLIT_MSG) &&
	    !((hdr->msgType == MSG_SMS_DAB_CHANNEL) || (hdr->msgType == MSG_SMS_DVBT_BDA_DATA)))
	{
		sms_debug("Control Msg has split");
		return -EINVAL;
	}
	return 0;


}

static void smssdio_work_thread(struct work_struct *arg)
{
	int ret, isr, trnsfr_cnt;
	int bytes_transfered;
	char tmp_buf[SMSSDIO_BLOCK_SIZE];

        struct smscore_buffer_t *cb = NULL;
	struct SmsMsgHdr_S *hdr;
	size_t size;
        struct smssdio_device *smsdev = container_of(arg, struct smssdio_device, work_thread);
	 

	/*
	 * The interrupt register has no defined meaning. It is just
	 * a way of turning of the level triggered interrupt.
	 */
	sdio_claim_host(smsdev->func);

	bytes_transfered = 0;
	trnsfr_cnt = 	sdio_readb(smsdev->func, SMSSDIO_AHB_CNT+2, &ret) << 16 | 
			sdio_readb(smsdev->func, SMSSDIO_AHB_CNT+1, &ret) << 8 | 
			sdio_readb(smsdev->func, SMSSDIO_AHB_CNT, &ret);

	isr = sdio_readb(smsdev->func, SMSSDIO_INT, &ret);
	if (ret) {
		sms_err("Got error reading interrupt status=%d, isr=%d\n", ret, isr);
		isr = sdio_readb(smsdev->func, SMSSDIO_INT, &ret);
		if (ret)
		{
			sms_err("Second read also failed, try to recover\n");
			goto exit_with_error;
		}
		sms_err("Second read succeed status=%d, isr=%d (continue)\n", ret, isr);
	}
	

	if (smsdev->split_cb == NULL) {		
		cb = smscore_getbuffer(smsdev->coredev);
		if (!cb) {
			sms_err("Unable to allocate data buffer!\n");
			goto exit_with_error;
		}

		ret = sdio_memcpy_fromio(smsdev->func,
					 cb->p,
					 SMSSDIO_DATA,
					 SMSSDIO_BLOCK_SIZE);
		bytes_transfered += SMSSDIO_BLOCK_SIZE;
		if (ret) {
			sms_err("Error %d reading initial block\n", ret);
			goto exit_with_error;
		}

		hdr = cb->p;
		if (verify_valid_hdr(hdr))
		{
			sms_err("recieved bad header. abort the io operation\n");
			goto exit_with_error;
		}
		if (hdr->msgFlags & MSG_HDR_FLAG_SPLIT_MSG) {
			smsdev->split_cb = cb;
			sdio_release_host(smsdev->func);
			return;
		}

		if (hdr->msgLength > smsdev->func->cur_blksize)
			size = hdr->msgLength - smsdev->func->cur_blksize;
		else
			size = 0;
	} else {
		cb = smsdev->split_cb;
		hdr = cb->p;

		size = hdr->msgLength - sizeof(struct SmsMsgHdr_S);

		smsdev->split_cb = NULL;
	}
	if (size) {
		void *buffer;

		buffer = cb->p + (hdr->msgLength - size);
		size = ALIGN(size, SMSSDIO_BLOCK_SIZE);

		BUG_ON(smsdev->func->cur_blksize != SMSSDIO_BLOCK_SIZE);

		/*
		 * Read one block at a time in order to know how many transfered...
		 */
		while (size) {
			ret = sdio_memcpy_fromio(smsdev->func,
					  buffer, SMSSDIO_DATA,
					  SMSSDIO_BLOCK_SIZE);
			if (ret) {
					smscore_putbuffer(smsdev->coredev, cb);
					sms_err("Error %d reading "
						"data from card!\n", ret);
					goto exit_with_error;
				}

				buffer += SMSSDIO_BLOCK_SIZE;
				bytes_transfered += SMSSDIO_BLOCK_SIZE;
				if (size > smsdev->func->cur_blksize)
					size -= SMSSDIO_BLOCK_SIZE;
				else
					size = 0;
			}
	}

	sdio_release_host(smsdev->func);
	cb->size = hdr->msgLength;
	cb->offset = 0;	

	smscore_onresponse(smsdev->coredev, cb);
	return;

exit_with_error:
	
	while (bytes_transfered < trnsfr_cnt)
	{
		sdio_memcpy_fromio(smsdev->func,
				  tmp_buf, SMSSDIO_DATA,
				  SMSSDIO_BLOCK_SIZE);
		bytes_transfered += SMSSDIO_BLOCK_SIZE;
	}
	if (cb && cb != smsdev->split_cb)
		smscore_putbuffer(smsdev->coredev, cb);
	if (smsdev->split_cb)
	{ /*If header kept before, the bad message should drop the kept header*/
		smscore_putbuffer(smsdev->coredev, smsdev->split_cb);
		smsdev->split_cb = NULL;
	}
	sdio_release_host(smsdev->func);
	return;

}


static void smssdio_interrupt(struct sdio_func *func)
{
	struct smssdio_device *smsdev = sdio_get_drvdata(func);
	if (sdio_use_workthread == 0) /*When not required - handle everything from interrupt content*/
	{
		smssdio_work_thread(&smsdev->work_thread);
	}
	else
	{
		schedule_work(&smsdev->work_thread);
	}
}

static int __devinit smssdio_probe(struct sdio_func *func,
			 const struct sdio_device_id *id)
{
	int ret;

	int board_id;
	struct smssdio_device *smsdev;
	struct smsdevice_params_t params;

	board_id = id->driver_data;

	smsdev = kzalloc(sizeof(struct smssdio_device), GFP_KERNEL);
	if (!smsdev)
		return -ENOMEM;

	smsdev->func = func;
        INIT_WORK(&smsdev->work_thread, smssdio_work_thread);

	memset(&params, 0, sizeof(struct smsdevice_params_t));

	params.device = &func->dev;
	params.buffer_size = MAX_SDIO_BUF_SIZE;	/* ?? */
	params.num_buffers = 22;	/* ?? */
	params.context = smsdev;

	snprintf(params.devpath, sizeof(params.devpath),
		 "sdio\\%s", sdio_func_id(func));

	params.sendrequest_handler = smssdio_sendrequest;

	params.device_type = sms_get_board(board_id)->type;
	params.require_node_buffer = 1;

	if (params.device_type != SMS_STELLAR)
		params.flags |= SMS_DEVICE_FAMILY2;
	else {
		/*
		 * FIXME: Stellar needs special handling...
		 */
		ret = -ENODEV;
		goto free;
	}
	if (sdio_plugin_delay)
		msleep (min((u32)1000, sdio_plugin_delay));
	ret = smscore_register_device(&params, &smsdev->coredev);
	if (ret < 0)
		goto free;

	smscore_set_board_id(smsdev->coredev, board_id);

	sdio_claim_host(func);

	ret = sdio_enable_func(func);
	if (ret)
		goto release;

	ret = sdio_set_block_size(func, SMSSDIO_BLOCK_SIZE);
	if (ret)
		goto disable;

	ret = sdio_claim_irq(func, smssdio_interrupt);
	if (ret)
		goto disable;

	sdio_set_drvdata(func, smsdev);

	sdio_release_host(func);

	ret = smscore_start_device(smsdev->coredev);
	if (ret < 0)
		goto reclaim;

	return 0;

reclaim:
	sdio_claim_host(func);
	sdio_release_irq(func);
disable:
	sdio_disable_func(func);
release:
	sdio_release_host(func);
	smscore_unregister_device(smsdev->coredev);
free:
	kfree(smsdev);

	return ret;
}

static void smssdio_remove(struct sdio_func *func)
{
	struct smssdio_device *smsdev;

	smsdev = sdio_get_drvdata(func);
	sdio_claim_host(func);

	sdio_release_irq(func);
	sdio_disable_func(func);
	sdio_release_host(func);

	if (smsdev->split_cb)
		smscore_putbuffer(smsdev->coredev, smsdev->split_cb);

	smscore_unregister_device(smsdev->coredev);

	kfree(smsdev);
}

static struct sdio_driver smssdio_driver = {
	.name = "smssdio",
	.id_table = smssdio_ids,
	.probe = smssdio_probe,
	.remove = smssdio_remove,
};

/*******************************************************************/
/* Module functions                                                */
/*******************************************************************/

int smssdio_register(void)
{
	int ret = 0;

	ret = sdio_register_driver(&smssdio_driver);

	return ret;
}

void smssdio_unregister(void)
{
	sdio_unregister_driver(&smssdio_driver);
}


