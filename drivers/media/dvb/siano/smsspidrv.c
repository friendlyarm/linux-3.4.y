/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
 Copyright (C) 2006-2010, Erez Cohen

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
/*!
	\file	spibusdrv.c

	\brief	spi bus driver module

	This file contains implementation of the spi bus driver.
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include "smscoreapi.h"
#include "smsspicommon.h"
#include "smsspiphy.h"

#define SMS_INTR_PIN			19  /* 0 for nova sip, 26 for vega */
#define TX_BUFFER_SIZE			0x200
#define RX_BUFFER_SIZE			(0x1000 + SPI_PACKET_SIZE + 0x100)
#define NUM_RX_BUFFERS			72

struct _spi_device_st {
	struct _spi_dev dev;
	void *phy_dev;

	struct completion version_ex_done;
	struct completion write_operation;
	struct list_head tx_queue;
	struct task_struct *thread;
	struct work_struct work_thread;
	struct work_struct connect_thread;
	struct _rx_buffer_st private_buf[2];
	int cur_buf;
	bool wait_for_version_resp;
	int allocatedPackets;
	int padding_allowed;
	char *rxbuf;
	int chip_model;
	int device_initialized;
	int device_connecting;
	void *coredev;
	struct list_head txqueue;
	char *txbuf;
	dma_addr_t txbuf_phy_addr;
};

struct _smsspi_txmsg {
	struct list_head node;	/*! internal management */
	void *buffer;
	size_t size;
	int alignment;
	int add_preamble;
	struct completion completion;
	void (*prewrite) (void *);
	void (*postwrite) (void *);
};

struct _spi_device_st *spi_dev;

static u8 smsspi_preamble[] = { 0xa5, 0x5a, 0xe7, 0x7e };
static u8 smsspi_startup[] = { 0, 0, 0xde, 0xc1, 0xa5, 0x51, 0xf1, 0xed };
static u32 sms_intr_pin = SMS_INTR_PIN;
static int device_recovery_time = 30;
extern u32 host_spi_intr_pin;
extern u32 host_spi_bus;
extern u32 host_spi_cs;
extern u32 spi_max_speed;
extern u32 spi_default_type;

module_param(spi_default_type, int, S_IRUGO);
MODULE_PARM_DESC(spi_default_type, "default SMS device type for SPI bus.");

module_param(sms_intr_pin, int, S_IRUGO);
MODULE_PARM_DESC(sms_intr_pin, "interrupt pin number used by SMS chip for interrupting host (old devices).");

module_param(host_spi_intr_pin, int, S_IRUGO);
MODULE_PARM_DESC(host_spi_intr_pin, "interrupt pin number used by Host to be interrupted by SMS.");

module_param(host_spi_bus, int, S_IRUGO);
MODULE_PARM_DESC(host_spi_bus, "Number of SPI bus used for SMS.");

module_param(host_spi_cs, int, S_IRUGO);
MODULE_PARM_DESC(host_spi_cs, "Number of SPI CS pin in used SPI bus connected to SMS.");

module_param(spi_max_speed, int, S_IRUGO);
MODULE_PARM_DESC(spi_max_speed, "Maximum SPI bus speed during working state.");

module_param(device_recovery_time, int, S_IRUGO);
MODULE_PARM_DESC(device_recovery_time, "Device recovery time after power up.");

#define is_private_buf(ptr) ((ptr == (struct _rx_buffer_st *)&spi_device->private_buf[0]) || \
			     (ptr == (struct _rx_buffer_st *)&spi_device->private_buf[1]))

/******************************************/
static void spi_worker_thread(struct work_struct *arg)
{
	struct _spi_device_st *spi_device = container_of(arg, struct _spi_device_st, work_thread);
	struct _smsspi_txmsg *msg = NULL;
	struct _spi_msg txmsg;

	do {
		/* do we have a msg to write ? */
		if (!msg && !list_empty(&spi_device->txqueue))
		{
			msg = (struct _smsspi_txmsg *)
					list_entry(spi_device->txqueue.
					next, struct _smsspi_txmsg, node);
		}

		if (msg) {
			if (msg->add_preamble)
			{
				txmsg.len =
				    min(msg->size + sizeof(smsspi_preamble),
					(size_t) TX_BUFFER_SIZE);
				txmsg.buf = spi_device->txbuf;
				txmsg.buf_phy_addr = spi_device->txbuf_phy_addr;
				memcpy(txmsg.buf, smsspi_preamble,
				       sizeof(smsspi_preamble));
				memcpy(&txmsg.buf[sizeof(smsspi_preamble)],
				       msg->buffer,
				       txmsg.len - sizeof(smsspi_preamble));
				msg->add_preamble = 0;
				msg->buffer +=
				    txmsg.len - sizeof(smsspi_preamble);
				msg->size -=
				    txmsg.len - sizeof(smsspi_preamble);
				/* zero out the rest of aligned buffer */
				memset(&txmsg.buf[txmsg.len], 0,
				       TX_BUFFER_SIZE - txmsg.len);
				smsspi_common_transfer_msg(&spi_device->dev,
							   &txmsg, 1);
			} else {
				txmsg.len =
				    min(msg->size, (size_t) TX_BUFFER_SIZE);
				txmsg.buf = spi_device->txbuf;
				txmsg.buf_phy_addr = spi_device->txbuf_phy_addr;
				memcpy(txmsg.buf, msg->buffer, txmsg.len);

				msg->buffer += txmsg.len;
				msg->size -= txmsg.len;
				/* zero out the rest of aligned buffer */
				memset(&txmsg.buf[txmsg.len], 0,
				       TX_BUFFER_SIZE - txmsg.len);
				smsspi_common_transfer_msg(&spi_device->dev,
							   &txmsg, 0);
			}

		} else {
			smsspi_common_transfer_msg(&spi_device->dev, NULL, 1);
		}

		/* if there was write, have we finished ? */
		if (msg && !msg->size) {
			/* call postwrite call back */
			if (msg->postwrite)
				msg->postwrite(spi_device);
			list_del(&msg->node);
			complete(&msg->completion);
			msg = NULL;
		}
		/* if there was read, did we read anything ? */
	} while (!list_empty(&spi_device->txqueue) || msg);

}


struct _rx_buffer_st *allocate_rx_buf(void *context, int size)
{
	struct smscore_buffer_t *buf;
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;
	if (size > RX_BUFFER_SIZE) {
		sms_err("Requested size is bigger than max buffer size.\n");
		return NULL;
	}
	if (!spi_device->coredev)
	{ /* Internal messages. We only have 2 buffers for it, just use the next one*/
		spi_device->cur_buf = (spi_device->cur_buf + 1) % 2;
		return &spi_device->private_buf[spi_device->cur_buf];
	}
	buf = smscore_getbuffer(spi_device->coredev);
	sms_info("Recieved Rx buf %p physical 0x%x (contained in %p)\n", buf->p,
	       buf->phys, buf);

	/* note: this is not mistake! the rx_buffer_st is identical to part of
	   smscore_buffer_t and we return the address of the start of the
	   identical part */
	return (struct _rx_buffer_st *) &buf->p;
}

static void free_rx_buf(void *context, struct _rx_buffer_st *buf)
{
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;
	struct smscore_buffer_t *cb =
	    (struct smscore_buffer_t
	     *)(container_of(((void *)buf), struct smscore_buffer_t, p));
	if (is_private_buf(buf))
	{
		return; /*private buffer is internal and not required to be released.*/
	}
	smscore_putbuffer(spi_device->coredev, cb);
}


static void msg_found(void *context, void *buf, int offset, int len)
{
	struct SmsMsgHdr_S *phdr;
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;
	struct smscore_buffer_t *cb =
	    (struct smscore_buffer_t
	     *)(container_of(buf, struct smscore_buffer_t, p));

	sms_info("entering\n");
	if (is_private_buf(buf))
	{
		phdr = (struct SmsMsgHdr_S *)((u8*)((struct _rx_buffer_st*)buf)->ptr + offset);
	}
	else
	{
		cb->offset = offset;
		cb->size = len;
		phdr = (struct SmsMsgHdr_S *)((u8*)cb->p+cb->offset);
	}
	if (spi_device->wait_for_version_resp)
	{
		sms_info("msg type=%d\n", phdr->msgType);
		if (phdr->msgType == MSG_SMS_GET_VERSION_EX_RES)
		{ /*This was an internal command, so we won't send it out*/
			spi_device->chip_model = *((u16*)(phdr+1));
			sms_info("chip model=0x%x\n", spi_device->chip_model);
			spi_device->wait_for_version_resp = 0;
			sms_info("complete get version\n");
			complete(&spi_device->version_ex_done);
			sms_info("free the buffer (0x%p)", buf);
			free_rx_buf(spi_device, buf);
			sms_info("done.");
			return;
		}
	}
	if (spi_device->coredev)
		smscore_onresponse(spi_device->coredev, cb);

	sms_info("exiting\n");

}

static int smsspi_queue_message_and_wait(struct _spi_device_st *spi_device,
					 struct _smsspi_txmsg *msg)
{
	init_completion(&msg->completion);
	list_add_tail(&msg->node, &spi_device->txqueue);
	schedule_work(&spi_device->work_thread);
	wait_for_completion(&msg->completion);

	return 0;
}

static int smsspi_preload(void *context)
{
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;

	sms_debug("preparing for download\n");
	prepareForFWDnl(spi_device->phy_dev);
	return 0;
}

static int smsspi_postload(void *context)
{
	struct sms_properties_t properties;
	int mode;
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;
	smscore_get_device_properties(spi_device->coredev, &properties);
	mode = properties.mode;
	if ( (mode != SMSHOSTLIB_DEVMD_ISDBT) &&
	     (mode != SMSHOSTLIB_DEVMD_ISDBT_BDA) ) {
		fwDnlComplete(spi_device->phy_dev, 0);
		
	}
	
	return 0;
}

static int smsspi_write(void *context, void *txbuf, size_t len)
{
	struct _smsspi_txmsg msg;
	msg.buffer = txbuf;
	msg.size = len;
	msg.prewrite = NULL;
	msg.postwrite = NULL;
	if (len > 0x1000) {
		/* The FW is the only long message. Do not add preamble,
		and do not padd it */
		msg.alignment = 4;
		msg.add_preamble = 0;
		msg.prewrite = smschipreset;
	} else {
		msg.alignment = SPI_PACKET_SIZE;
		msg.add_preamble = 1;
	}
	sms_info("Writing message to  SPI.\n");
	sms_info("msg hdr: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x.\n",
	       ((u8 *) txbuf)[0], ((u8 *) txbuf)[1], ((u8 *) txbuf)[2],
	       ((u8 *) txbuf)[3], ((u8 *) txbuf)[4], ((u8 *) txbuf)[5],
	       ((u8 *) txbuf)[6], ((u8 *) txbuf)[7]);
	return smsspi_queue_message_and_wait(context, &msg);
}

/*! Release device STUB

\param[in]	dev:		device control block
\return		void
*/
static void smsspi_release(struct device *dev)
{
	sms_info("nothing to do\n");
	/* Nothing to release */
}


static struct platform_device smsspi_device = {
	.name = "smsspi",
	.id = 1,
	.dev = {
		.release = smsspi_release,
		},
};



static int smsspi_init_device(struct _spi_device_st *spi_device)
{
	int ret;
	struct smsdevice_params_t params;

	/* register in smscore */
	memset(&params, 0, sizeof(params));
	params.context = spi_device;
	params.device = &smsspi_device.dev;
	params.buffer_size = RX_BUFFER_SIZE;
	params.num_buffers = NUM_RX_BUFFERS;
	params.flags = 0;
	params.sendrequest_handler = smsspi_write;
	strcpy(params.devpath, "spi");
	switch(spi_device->chip_model)
	{
		case 0: params.device_type = spi_default_type; break;
		case 0x1002:
		case 0x1102:
		case 0x1004: params.device_type = SMS_NOVA_B0; break;
		case 0x1182: params.device_type = SMS_VENICE; break;
		case 0x1530: params.device_type = SMS_DENVER_1530; break;
		case 0x2130: params.device_type = SMS_PELE; break;
		case 0x2160: params.device_type = SMS_DENVER_2160; break;
		case 0x2180: params.device_type = SMS_MING; break;
		case 0x2230: params.device_type = SMS_RIO; break;
		case 0x3130: params.device_type = SMS_ZICO; break;
		case 0x3180: params.device_type = SMS_QING; break;
		case 0x3230: params.device_type = SMS_SANTOS; break;
		default: params.device_type = spi_default_type; break;
	}

	params.flags = SMS_DEVICE_FAMILY2;
	params.preload_handler = smsspi_preload;
	params.postload_handler = smsspi_postload;
	sms_info ("registering spi device type %d",params.device_type);

	ret = smscore_register_device(&params, &spi_device->coredev);
	if (ret < 0) {
		sms_err("smscore_register_device(...) failed\n");
//		goto reg_device_error;
	}

	ret = smscore_start_device(spi_device->coredev);
	if (ret < 0) {
		sms_err("smscore_start_device(...) failed unregister the device but leave driver alive. \n");
		smscore_unregister_device(spi_device->coredev);
		spi_device->coredev = NULL;
	}
	return ret;

}

static int smsspi_deinit_device(struct _spi_device_st *spi_device)
{

	if (spi_device->coredev)
	{
		smscore_unregister_device(spi_device->coredev);
		spi_device->coredev = NULL;
	}
	if (spi_device->device_initialized)
	{
		spi_device->device_initialized = 0;
		smsspiphy_deinit(spi_device->phy_dev);
	}
	return 0;

}

static int smsspi_check_version(struct _spi_device_st *spi_device)
{
	int ret = 1;
	struct SmsMsgHdr_S smsmsg;
	struct _smsspi_txmsg msg;

	spi_device->wait_for_version_resp = 1;
	init_completion(&spi_device->version_ex_done);
	SMS_INIT_MSG(&smsmsg, MSG_SMS_GET_VERSION_EX_REQ,
		     sizeof(struct SmsMsgHdr_S));

	msg.buffer = &smsmsg;
	msg.size = smsmsg.msgLength;
	msg.prewrite = NULL;
	msg.postwrite = NULL;
	msg.alignment = SPI_PACKET_SIZE;
	msg.add_preamble = 1;
	smsspi_queue_message_and_wait(spi_device, &msg);

	/*Wait for response*/
	ret = wait_for_completion_timeout(&spi_device->version_ex_done, msecs_to_jiffies(500));
	if (ret > 0)
	{ /*Got version. device is in*/
		if (!spi_device->coredev)
		{ /*New device found*/
			return smsspi_init_device(spi_device);				
		}
	}
	else
	{ /* No response recieved*/
		sms_info("No response to get version command");
		return -ENODEV;
	}
	return 0;
}


static int sms_keep_alive_thread(void *context)
{
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;
	int count = 0;
	sms_info("Keep alive thread start\n");
	for (;;)
	{
		count++;
		if ((count & 0x1f) == 0) 
			sms_info("Keep alive is running\n");
			
		msleep(50);
		if (!smsspiphy_is_device_exists())
		{
			sms_info("Device is out. deinit\n");
			smsspi_deinit_device(spi_device);
			break;
		}

		/* Response not recieved, try to send start sequence */
          	if (kthread_should_stop()) 
		{
			sms_info("Stopping thread.\n");
                       	break;
		}

	}
	sms_info("Keep alive thread END\n");
	spi_device->thread = NULL;
	return 0;
}





static int smsspi_identify_device(struct _spi_device_st *spi_device)
{
	int ret;
	struct _spi_dev_cb_st common_cb;
	struct _smsspi_txmsg msg;
	
	if (!smsspiphy_is_device_exists())
		return -ENODEV;

	/* Try to identify SMS device */
	sms_info("Trying to identify SMS device on SPI\n");
	ret = smsspiphy_init(spi_device->phy_dev);
	if (ret) {
		sms_err("smsspiphy_init(...) failed\n");
		goto phy_error;
	}
	sms_info("SPI Phy initialized.\n");
	common_cb.allocate_rx_buf = allocate_rx_buf;
	common_cb.free_rx_buf = free_rx_buf;
	common_cb.msg_found_cb = msg_found;
	common_cb.transfer_data_cb = smsspibus_xfer;

	ret =
	    smsspicommon_init(&spi_device->dev, spi_device, 
					spi_device->phy_dev,
					&common_cb);
	if (ret) {
		sms_err("smsspicommon_init(...) failed\n");
		goto common_error;
	}
	spi_device->device_initialized = 1;

	sms_info("SPI Common initialized.\n");
	if (spi_default_type <= SMS_VENICE)
	{
		 /*Before venice device there is not version response from ROM*/
		 /*There is no way to simulate PnP events. Just wake up the device*/
		 /* and report plugin. Assuming the device is in.*/
		sms_info("Old device. Just send startup sequence and report plugin.\n");
		msg.buffer = smsspi_startup;
		msg.size = sizeof(smsspi_startup);
		msg.alignment = 4;
		msg.add_preamble = 0;
		msg.prewrite = NULL;
		msg.postwrite = NULL;

		ret = smsspi_queue_message_and_wait(spi_device, &msg);
		if (ret < 0) {
			sms_err("smsspi_queue_message_and_wait error, rc = %d\n", ret);
			goto device_not_found;
		}
		ret = smsspi_init_device(spi_device);	
		if (ret < 0) {
			sms_err("smsspi_queue_message_and_wait error, rc = %d\n", ret);
			goto device_not_found;
		}
			
	}
	else
	{
		ret = smsspi_check_version(spi_device);
		if (ret < 0)
		{ 
			msg.buffer = smsspi_startup;
			msg.size = sizeof(smsspi_startup);
			msg.alignment = 4;
			msg.add_preamble = 0;
			msg.prewrite = NULL;
			msg.postwrite = NULL;

			ret = smsspi_queue_message_and_wait(spi_device, &msg);
			if (ret < 0) {
				sms_err("smsspi_queue_message_and_wait error, rc = %d\n", ret);
				goto device_not_found;
			}

			msleep(30);
			ret = smsspi_check_version(spi_device);
			if (ret < 0)
			{
				sms_err("Device does not respond. release SPI bus, but this is not an error");
				ret = 0;
				goto device_not_found;
			}

		
		}
	}
	/* Device found. start keep alive thread*/
	spi_device->thread = kthread_create(sms_keep_alive_thread, spi_device, "SMS PnP thread");
	if (IS_ERR(spi_device->thread)) {
		ret = PTR_ERR(spi_device->thread);
		goto thread_failed;
	}
	wake_up_process(spi_device->thread);
	return 0;
thread_failed:
device_not_found:
common_error:
	smsspiphy_deinit(spi_device->phy_dev);
	spi_device->device_initialized = 0;
phy_error:
	return ret;
}


static void smsspi_connect(struct work_struct *arg)
{
	struct _spi_device_st *spi_device = container_of(arg, struct _spi_device_st, connect_thread);
	sms_info("Found new SPI device (dev=0x%p). \n", spi_device);
	sms_debug("Delay %d for device wakeup", device_recovery_time);
	/* Wait a bit before trying to comunicate*/
	msleep(device_recovery_time);
	smsspi_identify_device(spi_device);
	/*After the identify we go to fwDnlComplete in order to work in full speed*/
	/*If FW will be download - speed will be reduced*/
	if (spi_device->device_initialized)
		fwDnlComplete(spi_device->phy_dev, 0);
	spi_device->device_connecting = 0;
}


static void smsspi_int_handler(void *context)
{
	struct _spi_device_st *spi_device = (struct _spi_device_st *) context;
	sms_info("interrupt init = %d\n", spi_device->device_initialized);
	if (!spi_device->device_initialized)
	{
		if (!spi_device->device_connecting)
		{
			spi_device->device_connecting = 1;
			schedule_work(&spi_device->connect_thread);
		}
	}
	else
	{ /*It could be due to interrupt bouncing that we get a second interrupt*/
	  /*when device wakes up. unless device fully identified we will ignore it*/
		schedule_work(&spi_device->work_thread);
	}
}

int smsspi_register(void)
{
	int ret;
	struct _spi_device_st *spi_device;

	sms_info("entering \n");

	spi_device =
	    kzalloc(sizeof(struct _spi_device_st), GFP_KERNEL);
	spi_dev = spi_device;

	INIT_LIST_HEAD(&spi_device->txqueue);
	INIT_WORK(&spi_device->work_thread, spi_worker_thread);
	INIT_WORK(&spi_device->connect_thread, smsspi_connect);
	spi_device->chip_model = 0;
	ret = platform_device_register(&smsspi_device);
	if (ret < 0) {
		sms_err("platform_device_register failed\n");
		return ret;
	}

	spi_device->txbuf =
	    dma_alloc_coherent(NULL, TX_BUFFER_SIZE,
			       &spi_device->txbuf_phy_addr,
			       GFP_KERNEL | GFP_DMA);
	if (!spi_device->txbuf) {
		sms_err("dma_alloc_coherent(...) failed\n");
		ret = -ENOMEM;
		goto txbuf_error;
	}
	spi_device->cur_buf = 0;
	spi_device->private_buf[0].ptr = dma_alloc_coherent(NULL, 
			RX_BUFFER_SIZE,
 			(dma_addr_t *)&spi_device->private_buf[0].phy_addr, 
			GFP_KERNEL | GFP_DMA);
	if (!spi_device->private_buf[0].ptr) {
		sms_err("dma_alloc_coherent(...) failed\n");
		ret = -ENOMEM;
		goto private_buf0_error;
	}


	spi_device->private_buf[1].ptr = dma_alloc_coherent(NULL, 
			RX_BUFFER_SIZE,
 			(dma_addr_t *)&spi_device->private_buf[1].phy_addr, 
			GFP_KERNEL | GFP_DMA);
	if (!spi_device->private_buf[1].ptr) {
		sms_err("dma_alloc_coherent(...) failed\n");
		ret = -ENOMEM;
		goto private_buf1_error;
	}

	/*Register the physical layer*/
	spi_device->phy_dev = smsspiphy_register(NULL, 
						smsspi_int_handler, 
						spi_device);
	if (spi_device->phy_dev == 0) {
		sms_err("smsspiphy_register(...) failed\n");
		ret = -EFAULT;
		goto phy_error;
	}

	/*Check if device already active than register it. If not, we wait for plugin
	event notified by interrupt falling edge.*/
	sms_info("Check if device is in.\n");
	if (smsspiphy_is_device_exists())
	{
		schedule_work(&spi_device->connect_thread);
	}

	sms_info("exiting\n");
	return 0;


phy_error:
	dma_free_coherent(NULL, RX_BUFFER_SIZE, spi_device->private_buf[1].ptr,
			  spi_device->private_buf[1].phy_addr);

private_buf1_error:
	dma_free_coherent(NULL, RX_BUFFER_SIZE, spi_device->private_buf[0].ptr,
			  spi_device->private_buf[0].phy_addr);


private_buf0_error:
	dma_free_coherent(NULL, TX_BUFFER_SIZE, spi_device->txbuf,
			  spi_device->txbuf_phy_addr);

txbuf_error:
	platform_device_unregister(&smsspi_device);

	sms_info("exiting error %d\n", ret);

	return ret;
}

void smsspi_unregister(void)
{
	struct _spi_device_st *spi_device = spi_dev;
	sms_info("entering\n");

	if (spi_device->thread)
	{
		kthread_stop(spi_device->thread);
		spi_device->thread = NULL;
	}
	smsspi_deinit_device(spi_device);
	smsspiphy_unregister(spi_device->phy_dev);

	dma_free_coherent(NULL, RX_BUFFER_SIZE, spi_device->private_buf[0].ptr,
			  spi_device->private_buf[0].phy_addr);

	dma_free_coherent(NULL, RX_BUFFER_SIZE, spi_device->private_buf[1].ptr,
			  spi_device->private_buf[1].phy_addr);

	dma_free_coherent(NULL, TX_BUFFER_SIZE, spi_device->txbuf,
			  spi_device->txbuf_phy_addr);


	platform_device_unregister(&smsspi_device);
	sms_info("exiting\n");
}


