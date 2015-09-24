/*
 *  Siano core API module
 *
 *  This file contains implementation for the interface to sms core component
 *
 *  author: Uri Shkolnik
 *
 *  Copyright (c), 2005-2008 Siano Mobile Silicon, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <linux/firmware.h>
#include <linux/wait.h>
#include <asm/byteorder.h>
#include <asm/uaccess.h>
#include "smsendian.h"
#include "smscoreapi.h"
#include "sms-cards.h"
#ifdef SMS_RC_SUPPORT_SUBSYS
#include "smsir.h"
#endif

#define MAX_GPIO_PIN_NUMBER	31

int sms_debug=3;
module_param_named(debug, sms_debug, int, 0644);
MODULE_PARM_DESC(debug, "set debug level (info=1, adv=2 (or-able))");

struct smscore_device_notifyee_t {
	struct list_head entry;
	hotplug_t hotplug;
};

struct smscore_idlist_t {
	struct list_head entry;
	int		id;
	int		data_type;
};

struct smscore_client_t {
	struct list_head entry;
	struct smscore_device_t *coredev;
	void			*context;
	struct list_head 	idlist;
	onresponse_t	onresponse_handler;
	onremove_t		onremove_handler;
};

struct smscore_device_t {
	struct list_head entry;

	struct list_head clients;
	struct list_head subclients;
	spinlock_t clientslock;

	struct list_head buffers;
	spinlock_t bufferslock;
	int num_buffers;

	void *common_buffer;
	int common_buffer_size;
	dma_addr_t common_buffer_phys;

	void *context;
	struct device *device;

	char devpath[32];
	unsigned long device_flags;

	loadfirmware_t loadfirmware_handler;
	sendrequest_t sendrequest_handler;
	preload_t preload_handler;
	postload_t postload_handler;
	int mode, modes_supported;

	/* host <--> device messages */
	struct completion version_ex_done, data_download_done, data_validity_done, trigger_done;
	struct completion init_device_done, reload_start_done, resume_done, device_ready_done;
	struct completion gpio_configuration_done, gpio_set_level_done;
	struct completion gpio_get_level_done, ir_init_done, mrc_change_mode_done;

	/* Buffer management */
	wait_queue_head_t buffer_mng_waitq;

	/* GPIO */
	int gpio_get_res;

	/* Target hardware board */
	int board_id;

	/* Firmware */
	u8 *fw_buf;
	u32 fw_buf_size;
	u32 start_address;
	u32 current_address;

	/* Infrared (IR) */
#ifdef SMS_RC_SUPPORT_SUBSYS
	struct ir_t ir;
#endif
	int led_state;
	int multi_tuner;
	int num_of_tuners;   /*Indicates how many tuners are available on device*/
	struct delayed_work work_queue;
};


void smscore_set_board_id(void* dev, int id)
{
	struct smscore_device_t *core = (struct smscore_device_t *)dev;
	core->board_id = id;
}

int smscore_led_state(void* dev, int led)
{
	struct smscore_device_t *core = (struct smscore_device_t *)dev;
	if (led >= 0)
		core->led_state = led;
	return core->led_state;
}
EXPORT_SYMBOL_GPL(smscore_set_board_id);


struct smscore_registry_entry_t {
	struct list_head entry;
	char			devpath[32];
	int				mode;
	enum sms_device_type_st	type;
};

static struct list_head g_smscore_notifyees;
static struct list_head g_smscore_devices;
static struct mutex g_smscore_deviceslock;

static struct list_head g_smscore_registry;
static struct mutex g_smscore_registrylock;

static int default_mode = 5;

module_param(default_mode, int, 0644);
MODULE_PARM_DESC(default_mode, "default firmware id (device mode)");


static int multi_tuner = 0;

module_param(multi_tuner, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(multi_tuner, "Multi tuner (1) vs MRC (0) behavior.");


#ifdef SMS_HOSTLIB_SUBSYS
static powermode_t g_powermode_notify;
#endif
struct sms_firmware_t *sms_static_firmware;

static struct smscore_registry_entry_t *smscore_find_registry(char *devpath)
{
	struct smscore_registry_entry_t *entry;
	struct list_head *next;

	kmutex_lock(&g_smscore_registrylock);
	for (next = g_smscore_registry.next;
	     next != &g_smscore_registry;
	     next = next->next) {
		entry = (struct smscore_registry_entry_t *) next;
		if (!strcmp(entry->devpath, devpath)) {
			kmutex_unlock(&g_smscore_registrylock);
			return entry;
		}
	}
	entry = kmalloc(sizeof(struct smscore_registry_entry_t), GFP_KERNEL);
	if (entry) {
		entry->mode = default_mode;
		strcpy(entry->devpath, devpath);
		list_add(&entry->entry, &g_smscore_registry);
	} else
		sms_err("failed to create smscore_registry.");
	kmutex_unlock(&g_smscore_registrylock);
	return entry;
}

int smscore_registry_getmode(char *devpath)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		return entry->mode;
	else
		sms_err("No registry found.");

	return default_mode;
}

static enum sms_device_type_st smscore_registry_gettype(char *devpath)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		return entry->type;
	else
		sms_err("No registry found.");

	return SMS_UNKNOWN_TYPE;
}

void smscore_registry_setmode(char *devpath, int mode)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		entry->mode = mode;
	else
		sms_err("No registry found.");
}

static void smscore_registry_settype(char *devpath,
				     enum sms_device_type_st type)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		entry->type = type;
	else
		sms_err("No registry found.");
}


static void list_add_locked(struct list_head *new, struct list_head *head,
			    spinlock_t *lock)
{
	unsigned long flags;

	spin_lock_irqsave(lock, flags);

	list_add(new, head);

	spin_unlock_irqrestore(lock, flags);
}

/**
 * register a client callback that called when device plugged in/unplugged
 * NOTE: if devices exist callback is called immediately for each device
 *
 * @param hotplug callback
 *
 * @return 0 on success, <0 on error.
 */
int smscore_register_hotplug(hotplug_t hotplug)
{
	struct smscore_device_notifyee_t *notifyee;
	struct list_head *next, *first;
	int rc = 0;

	kmutex_lock(&g_smscore_deviceslock);

	notifyee = kmalloc(sizeof(struct smscore_device_notifyee_t),
			   GFP_KERNEL);
	if (notifyee) {
		/* now notify callback about existing devices */
		first = &g_smscore_devices;
		for (next = first->next;
		     next != first && !rc;
		     next = next->next) {
			struct smscore_device_t *coredev =
				(struct smscore_device_t *) next;
			rc = hotplug(coredev, coredev->device, 1);
		}

		if (rc >= 0) {
			notifyee->hotplug = hotplug;
			list_add(&notifyee->entry, &g_smscore_notifyees);
		} else
			kfree(notifyee);
	} else
		rc = -ENOMEM;

	kmutex_unlock(&g_smscore_deviceslock);

	return rc;
}

/**
 * unregister a client callback that called when device plugged in/unplugged
 *
 * @param hotplug callback
 *
 */
void smscore_unregister_hotplug(hotplug_t hotplug)
{
	struct list_head *next, *first;

	kmutex_lock(&g_smscore_deviceslock);

	first = &g_smscore_notifyees;

	for (next = first->next; next != first;) {
		struct smscore_device_notifyee_t *notifyee =
			(struct smscore_device_notifyee_t *) next;
		next = next->next;

		if (notifyee->hotplug == hotplug) {
			list_del(&notifyee->entry);
			kfree(notifyee);
		}
	}

	kmutex_unlock(&g_smscore_deviceslock);
}

static void smscore_notify_clients(struct smscore_device_t *coredev)
{
	struct smscore_client_t *client;

	/* the client must call smscore_unregister_client from remove handler */
	while (!list_empty(&coredev->clients)) {
		client = (struct smscore_client_t *) coredev->clients.next;
		client->onremove_handler(client->context);
	}
}

static int smscore_notify_callbacks(struct smscore_device_t *coredev,
				    struct device *device, int arrival)
{
	struct list_head *next, *first;
	int rc = 0;

	/* note: must be called under g_deviceslock */

	first = &g_smscore_notifyees;

	for (next = first->next; next != first; next = next->next) {
		rc = ((struct smscore_device_notifyee_t *) next)->
				hotplug(coredev, device, arrival);
		if (rc < 0)
			break;
	}

	return rc;
}


static void delayed_notify_callbacks_plugin(struct work_struct *work)
{
	struct smscore_device_t *coredev = container_of(work,
                                               struct smscore_device_t,
                                               work_queue.work);
	sms_info("calling the hotplug callbacks after the delay time=%d", jiffies_to_msecs(jiffies));
	smscore_notify_callbacks(coredev, coredev->device, 1);
}



static struct
smscore_buffer_t *smscore_createbuffer(u8 *buffer, void *common_buffer,
				       dma_addr_t common_buffer_phys)
{
	struct smscore_buffer_t *cb =
		kmalloc(sizeof(struct smscore_buffer_t), GFP_KERNEL);
	if (!cb) {
		sms_info("kmalloc(...) failed");
		return NULL;
	}

	cb->p = buffer;
	cb->offset_in_common = buffer - (u8 *) common_buffer;
	cb->phys = common_buffer_phys + cb->offset_in_common;
	cb->offset=0;

	return cb;
}

/**
 * creates coredev object for a device, prepares buffers,
 * creates buffer mappings, notifies registered hotplugs about new device.
 *
 * @param params device pointer to struct with device specific parameters
 *               and handlers
 * @param coredev pointer to a value that receives created coredev object
 *
 * @return 0 on success, <0 on error.
 */
int smscore_register_device(struct smsdevice_params_t *params, void **coredev)
{
	struct smscore_device_t *dev;
	u8 *buffer;

	dev = kzalloc(sizeof(struct smscore_device_t), GFP_KERNEL);
	if (!dev) {
		sms_info("kzalloc(...) failed");
		return -ENOMEM;
	}

	INIT_DELAYED_WORK(&dev->work_queue, delayed_notify_callbacks_plugin);
	/* init list entry so it could be safe in smscore_unregister_device */
	INIT_LIST_HEAD(&dev->entry);

	/* init queues */
	INIT_LIST_HEAD(&dev->clients);
	INIT_LIST_HEAD(&dev->buffers);

	/* init locks */
	spin_lock_init(&dev->clientslock);
	spin_lock_init(&dev->bufferslock);

	/* init completion events */
	init_completion(&dev->version_ex_done);
	init_completion(&dev->data_download_done);
	init_completion(&dev->data_validity_done);
	init_completion(&dev->trigger_done);
	init_completion(&dev->init_device_done);
	init_completion(&dev->reload_start_done);
	init_completion(&dev->resume_done);
	init_completion(&dev->gpio_configuration_done);
	init_completion(&dev->gpio_set_level_done);
	init_completion(&dev->gpio_get_level_done);
	init_completion(&dev->ir_init_done);
	init_completion(&dev->mrc_change_mode_done);
	init_completion(&dev->device_ready_done);

	/* Buffer management */
	init_waitqueue_head(&dev->buffer_mng_waitq);

	/* alloc common buffer */
	dev->common_buffer_size = params->buffer_size * params->num_buffers;
	if (params->require_node_buffer)
	{
		dev->common_buffer = kmalloc_node(dev->common_buffer_size, GFP_ATOMIC, NUMA_NO_NODE);		
		dev->common_buffer_phys = 0;
	}
	else
		dev->common_buffer = dma_alloc_coherent(NULL, dev->common_buffer_size,
						&dev->common_buffer_phys,
						GFP_KERNEL | GFP_DMA);
	if (!dev->common_buffer) {
		smscore_unregister_device(dev);
		return -ENOMEM;
	}

	/* prepare dma buffers */
	for (buffer = dev->common_buffer;
	     dev->num_buffers < params->num_buffers;
	     dev->num_buffers++, buffer += params->buffer_size) {
		struct smscore_buffer_t *cb =
			smscore_createbuffer(buffer, dev->common_buffer,
					     dev->common_buffer_phys);
		if (!cb) {
			smscore_unregister_device(dev);
			return -ENOMEM;
		}

		smscore_putbuffer(dev, cb);
	}

	sms_info("allocated %d buffers", dev->num_buffers);

	dev->mode = SMSHOSTLIB_DEVMD_NONE;
	dev->board_id = SMS_BOARD_UNKNOWN;

	dev->context = params->context;
	dev->device = params->device;
	dev->loadfirmware_handler = params->loadfirmware_handler;
	dev->sendrequest_handler = params->sendrequest_handler;
	dev->preload_handler = params->preload_handler;
	dev->postload_handler = params->postload_handler;

	dev->device_flags = params->flags;
	strcpy(dev->devpath, params->devpath);

	/* clear fw start address */
	dev->start_address = 0; 
	dev->multi_tuner = multi_tuner;

	smscore_registry_settype(dev->devpath, params->device_type);

	/* add device to devices list */
	kmutex_lock(&g_smscore_deviceslock);
	list_add(&dev->entry, &g_smscore_devices);
	kmutex_unlock(&g_smscore_deviceslock);

	*coredev = dev;

	sms_info("core device 0x%p created, mode %d, type %d devpath %s", dev, dev->mode, params->device_type, dev->devpath);

	return 0;
}
EXPORT_SYMBOL_GPL(smscore_register_device);


static int smscore_sendrequest_and_wait(void *dev,
		void *buffer, size_t size, struct completion *completion) {
	int rc;
	struct smscore_device_t *coredev = dev;

	if (completion == NULL)
		return -EINVAL;
	init_completion(completion);

	rc = coredev->sendrequest_handler(coredev->context, buffer, size);
	if (rc < 0) {
		sms_info("sendrequest returned error %d", rc);
		return rc;
	}

	return wait_for_completion_timeout(completion,
			msecs_to_jiffies(SMS_PROTOCOL_MAX_RAOUNDTRIP_MS)) ?
			0 : -ETIME;
}
#ifdef SMS_RC_SUPPORT_SUBSYS
/**
 * Starts & enables IR operations
 *
 * @return 0 on success, < 0 on error.
 */
static int smscore_init_ir(struct smscore_device_t *coredev)
{ 
	int ir_io;
	int rc;
	void *buffer;
	coredev->ir.rc_dev = NULL;
	ir_io = sms_get_board(coredev->board_id)->board_cfg.ir;
	if (ir_io) {/* only if IR port exist we use IR sub-module */
		sms_info("IR loading");
		rc = sms_ir_init(&coredev->ir, coredev, coredev->device);
		if	(rc != 0)
			sms_err("Error initialization DTV IR sub-module");
		else 
		{
			buffer = kmalloc(sizeof(struct SmsMsgData2Args_S) +
						SMS_DMA_ALIGNMENT,
						GFP_KERNEL | GFP_DMA);
			if (buffer) {
				struct SmsMsgData2Args_S *msg =
				(struct SmsMsgData2Args_S *)
				SMS_ALIGN_ADDRESS(buffer);

				SMS_INIT_MSG(&msg->xMsgHeader,
						MSG_SMS_START_IR_REQ,
						sizeof(struct SmsMsgData2Args_S));
				msg->msgData[0] = coredev->ir.controller;
				msg->msgData[1] = coredev->ir.timeout;

				smsendian_handle_tx_message(
					(struct SmsMsgHdr_S2 *)msg);
				rc = smscore_sendrequest_and_wait(coredev, msg,
						msg->xMsgHeader. msgLength,
						&coredev->ir_init_done);

				kfree(buffer);
			} else
				sms_err
				("Sending IR initialization message failed");
		}
	} else
		sms_info("IR port has not been detected");

	return 0;
}
#endif /*SMS_RC_SUPPORT_SUBSYS*/

/**
 * sets initial device mode and notifies client hotplugs that device is ready
 *
 * @param coredev pointer to a coredev object returned by
 * 		  smscore_register_device
 *
 * @return 0 on success, <0 on error.
 */
int smscore_start_device(void* dev)
{
	int rc = 0;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	int board_id = coredev->board_id;

#ifdef REQUEST_FIRMWARE_SUPPORTED
	int mode;
	int type = sms_get_board(board_id)->type;
	if (type == SMS_UNKNOWN_TYPE)
		type = smscore_registry_gettype(coredev->devpath);
	sms_info ("starting device type %d", type);
	/*
	 * first, search operation mode in the registry, with 
	 * the limitation of type-mode compatability.
	 * if firmware donload fails, get operation mode from
	 * sms_boards
	 * for spi, type = SMS_UNKNOWN_TYPE and board_id = SMS_BOARD_UNKNOWN
	 * so always default_mode is set
	 */
	switch (type) {
		case SMS_UNKNOWN_TYPE: 
			mode = default_mode;
			break;
		case SMS_STELLAR:
		case SMS_NOVA_A0:
		case SMS_NOVA_B0:
		case SMS_PELE:
        	case SMS_RIO:
        	case SMS_ZICO:
        	case SMS_SANTOS:
			mode = smscore_registry_getmode(coredev->devpath);
			sms_info ("mode for path %s in registry %d", coredev->devpath, mode);
			if (mode == SMSHOSTLIB_DEVMD_CMMB)
				mode = (default_mode == SMSHOSTLIB_DEVMD_CMMB) ? SMSHOSTLIB_DEVMD_NONE : default_mode;
			break;	
        case SMS_DENVER_1530:
            mode = SMSHOSTLIB_DEVMD_ATSC;
            break;
        case SMS_DENVER_2160:
            mode = SMSHOSTLIB_DEVMD_DAB_TDMB;
            break;
        case SMS_VEGA:
		case SMS_VENICE:
		case SMS_MING:
		case SMS_QING:
			mode = SMSHOSTLIB_DEVMD_CMMB;
			break;
		default:
			mode = SMSHOSTLIB_DEVMD_NONE;	
	}

	/* first try */
	sms_info ("mode after adjustment %d", mode);
	rc = smscore_set_device_mode(coredev, mode);	

	if (rc < 0) {
		sms_info("set device mode to %d failed", mode);

		/* 
		 * don't try again on spi mode, or if the mode from 
		 * sms_boards is identical to the previous mode
		 */
		if ((board_id == SMS_BOARD_UNKNOWN) ||
		    (mode == sms_get_board(board_id)->default_mode))
			return -ENOEXEC;

		/* second try */
		mode = sms_get_board(board_id)->default_mode;
		rc = smscore_set_device_mode(coredev, mode);	
		if (rc < 0) {
			sms_info("set device mode to %d failed", mode);
			return -ENOEXEC ;
		}
	}
	sms_info("set device mode succeeded");
	
	rc = smscore_configure_board(coredev);
	if (rc < 0) {
		sms_info("configure board failed , rc %d", rc);
		return rc;
	}
#endif

	kmutex_lock(&g_smscore_deviceslock);

	/*Sleep 2 seconds before notify about the plugin so the device */
	/*(MRC) will be ready*/
	sms_err("scheduling delayed callback to hotplugs time=%d", jiffies_to_msecs(jiffies));
	schedule_delayed_work(&coredev->work_queue, 100);
#ifdef SMS_RC_SUPPORT_SUBSYS
	smscore_init_ir(coredev);
#endif /*SMS_RC_SUPPORT_SUBSYS*/

	sms_info("device 0x%p started, rc %d", coredev, rc);

	kmutex_unlock(&g_smscore_deviceslock);

	return rc;
}

/**
 * injects firmware from a buffer to the device using data messages
 * 
 * @param coredev pointer to a coredev object returned by
 * 		  smscore_register_device
 * @param buffer pointer to a firmware buffer
 * @param size size (in bytes) of the firmware buffer
 * @return 0 on success, <0 on error.
 */
static int smscore_load_firmware_family2(struct smscore_device_t *coredev,
					 void *buffer, size_t size)
{
	struct SmsFirmware_ST *firmware = (struct SmsFirmware_ST *) buffer;
	struct SmsMsgHdr_S *msg;
	u32 mem_address,  calc_checksum = 0;
	u32 i, *ptr;
	u8 *payload = firmware->Payload;
	int rc = 0;
	firmware->StartAddress = le32_to_cpu(firmware->StartAddress);
	firmware->Length = le32_to_cpu(firmware->Length);

	mem_address = firmware->StartAddress;

	sms_info("loading FW to addr 0x%x size %d",
		 mem_address, firmware->Length);
	if (coredev->preload_handler) {
		rc = coredev->preload_handler(coredev->context);
		if (rc < 0)
			return rc;
	}

	/* PAGE_SIZE buffer shall be enough and dma aligned */
	msg = kmalloc(PAGE_SIZE, GFP_KERNEL | GFP_DMA);
	if (!msg)
		return -ENOMEM;

	if (coredev->mode != SMSHOSTLIB_DEVMD_NONE) {
		sms_debug("sending reload command.");
		SMS_INIT_MSG(msg, MSG_SW_RELOAD_START_REQ,
			     sizeof(struct SmsMsgHdr_S));
		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = smscore_sendrequest_and_wait(coredev, msg, msg->msgLength,
						  &coredev->reload_start_done);

		if (rc < 0) {				
			sms_err("device reload failed, rc %d", rc);
			goto exit_fw_download;
		}

		mem_address = *(u32 *) &payload[20];
	}

	for (i = 0, ptr = (u32*)firmware->Payload; i < firmware->Length/4 ; i++, ptr++)
		calc_checksum += *ptr;
	while (size && rc >= 0) {
		struct SmsDataDownload_S *DataMsg =
			(struct SmsDataDownload_S *) msg;
		int payload_size = min((int) size, SMS_MAX_PAYLOAD_SIZE);

		SMS_INIT_MSG(msg, MSG_SMS_DATA_DOWNLOAD_REQ,
			     (u16)(sizeof(struct SmsMsgHdr_S) +
				      sizeof(u32) + payload_size));

		DataMsg->MemAddr = mem_address;
		memcpy(DataMsg->Payload, payload, payload_size);


		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = smscore_sendrequest_and_wait(coredev, DataMsg,
				DataMsg->xMsgHeader.msgLength,
				&coredev->data_download_done);

		payload += payload_size;
		size -= payload_size;
		mem_address += payload_size;
	}

	if (rc < 0) 		
		goto exit_fw_download;

	sms_debug("sending MSG_SMS_DATA_VALIDITY_REQ expecting 0x%x", calc_checksum);
	SMS_INIT_MSG(msg, MSG_SMS_DATA_VALIDITY_REQ,
			sizeof(struct SmsMsgHdr_S) +
			sizeof(u32) * 3);
	((struct SmsMsgData_S *)msg)->msgData[0] = firmware->StartAddress;
		/* Entry point */
	((struct SmsMsgData_S *)msg)->msgData[1] = firmware->Length;
	((struct SmsMsgData_S *)msg)->msgData[2] = 0; /* Regular checksum*/
	smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
	rc = smscore_sendrequest_and_wait(coredev, msg,	((struct SmsMsgData_S *)msg)->xMsgHeader.msgLength, &coredev->data_validity_done);
	if (rc < 0) 		
		goto exit_fw_download;


		if (coredev->mode == SMSHOSTLIB_DEVMD_NONE) {
			struct SmsMsgData_S *TriggerMsg =
				(struct SmsMsgData_S *) msg;

		        sms_debug("sending MSG_SMS_SWDOWNLOAD_TRIGGER_REQ");
			SMS_INIT_MSG(msg, MSG_SMS_SWDOWNLOAD_TRIGGER_REQ,
				     sizeof(struct SmsMsgHdr_S) +
				     sizeof(u32) * 5);

			TriggerMsg->msgData[0] = firmware->StartAddress;
						/* Entry point */
		TriggerMsg->msgData[1] = 6; /* Priority */
			TriggerMsg->msgData[2] = 0x200; /* Stack size */
			TriggerMsg->msgData[3] = 0; /* Parameter */
			TriggerMsg->msgData[4] = 4; /* Task ID */

		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = smscore_sendrequest_and_wait(coredev,
			TriggerMsg,
					TriggerMsg->xMsgHeader.msgLength,
					&coredev->trigger_done);
		} else {
			SMS_INIT_MSG(msg, MSG_SW_RELOAD_EXEC_REQ,
				     sizeof(struct SmsMsgHdr_S));
		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = coredev->sendrequest_handler(coredev->context, msg,
				msg->msgLength);
	}

	if (rc < 0)
		goto exit_fw_download;
			
	/*
	 * backward compatibility - wait to device_ready_done for
	 * not more than 400 ms
	 */
	msleep(400);

exit_fw_download:
	sms_debug("rc=%d, postload=0x%p ", rc, coredev->postload_handler);

	kfree(msg);

	return ((rc >= 0) && coredev->postload_handler) ?
			coredev->postload_handler(coredev->context) : rc;
}

/**
 * loads specified firmware into a buffer and calls device loadfirmware_handler
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param mode requested mode of operation
 * @param lookup if 1, always get the fw filename from smscore_fw_lkup 
 * 	  table. if 0, try first to get from sms_boards
 * @param filename null-terminated string specifies firmware file name
 * @param loadfirmware_handler device handler that loads firmware
 *
 * @return 0 on success, <0 on error.
 */
static int smscore_load_firmware_from_file(struct smscore_device_t *coredev,
		int mode, int lookup, loadfirmware_t loadfirmware_handler) {
	int rc = -ENOENT;

#ifdef REQUEST_FIRMWARE_SUPPORTED
	const struct firmware *fw;
	char* fw_filename = smscore_get_fw_filename(coredev, mode, lookup);

	sms_info("loading firmware. static buffer=0x%p", sms_static_firmware);
	if (!strcmp(fw_filename,"none"))
		return -ENOENT;

	if (loadfirmware_handler == NULL && !(coredev->device_flags
			& SMS_DEVICE_FAMILY2))
		return -EINVAL;
	if (sms_static_firmware)
	{
		sms_info("fw buffer already exists");
		if (strcmp (sms_static_firmware->file_name, fw_filename) != 0)
		{
			sms_info("file name is different, need to create ne fw buffer.");
			kfree(sms_static_firmware);			
			sms_info("old FW buffer (0x%p) is free", sms_static_firmware);
			sms_static_firmware = NULL;
		}
	}

	if (!sms_static_firmware)
	{
		sms_info("creating new fw buffer and load file.");
		rc = request_firmware(&fw, fw_filename, coredev->device);
		if (rc < 0) {
			sms_info("failed to open \"%s\"", fw_filename);
			return rc;
		}
		sms_info("read fw %s, buffer size=0x%x", fw_filename, fw->size);
		sms_static_firmware = kmalloc(ALIGN(fw->size + sizeof(sms_static_firmware), SMS_ALLOC_ALIGNMENT),
				GFP_KERNEL | GFP_DMA);
		if (!sms_static_firmware) {
			sms_info("failed to allocate firmware buffer");
			return -ENOMEM;
		}
		sms_info("buffer 0x%p allocated. copy filename", sms_static_firmware);
		memcpy(sms_static_firmware->file_name, fw_filename, sizeof(sms_static_firmware->file_name));
		sms_info("copy data");
		memcpy(sms_static_firmware->data, fw->data, fw->size);
		sms_static_firmware->fw_buf_size = fw->size;
		sms_info("release the fw.");
		release_firmware(fw);
	}
#else
	if (!sms_static_firmware) {
		sms_info("missing fw file buffer");
		return -EINVAL;
	}
#endif

	return 0;
}


/**
 * Set external FW file when request_firmware is not supported.
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param buffer  pointer to a buffer
 * @param size    size of buffer
 *
 * @return 0 on success, <0 on error.
 */
int smscore_set_fw_file(void* dev, void *buffer, size_t size) 
{
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	sms_debug("fw buffer = 0x%p, size = 0x%x", buffer, size);

	/* free old buffer */
	if (coredev->fw_buf != NULL) {
		kfree(coredev->fw_buf);
		coredev->fw_buf = NULL;
	}

	coredev->fw_buf = kmalloc(ALIGN(size, SMS_ALLOC_ALIGNMENT), GFP_KERNEL
			| GFP_DMA);
	if (!coredev->fw_buf) {
		sms_err("failed to allocate memory for fw buffer");
		return -ENOMEM;
	}

	if (copy_from_user(coredev->fw_buf, buffer, size)) {
		sms_err("failed to copy fw from user buffer");
		kfree(coredev->fw_buf);
	        coredev->fw_buf = NULL;
		return -EFAULT;
	}
	coredev->fw_buf_size = size;

	return 0;
}
EXPORT_SYMBOL_GPL(smscore_set_fw_file);



/**
 * Send chunk of firmware data using SMS MSGs
 * The motivation is to eliminate the need of big memory allocation in kernel for firmware
 * download.
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param buffer  pointer to a buffer
 * @param size    size of buffer
 *
 * @return 0 on success, <0 on error.
 */
int smscore_send_fw_chunk(void* dev, void *buffer, size_t size) 
{

	struct SmsMsgHdr_S *msg;
	int rc = 0;
	int offset = 0;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	
	if (buffer == NULL)
	{
		sms_debug("Error: NULL buffer");
		return -1;
	}
	
	/* First chunk */
	if (coredev->start_address == 0)
	{
		struct SmsFirmware_ST *firmware = (struct SmsFirmware_ST *) buffer;
		coredev->start_address = le32_to_cpu(firmware->StartAddress);
		coredev->current_address = coredev->start_address;
		offset = 12;
		size -= 12;
		
		if (coredev->preload_handler) 
		{
			rc = coredev->preload_handler(coredev->context);
			if (rc < 0)
				return rc;
		}
	}
		
	/* PAGE_SIZE buffer shall be enough and dma aligned */
	msg = kmalloc(PAGE_SIZE, GFP_KERNEL | GFP_DMA);
	if (!msg)
		return -ENOMEM;
		
	while (size && rc >= 0) {
		int payload_size;
		struct SmsDataDownload_S *DataMsg;
		sms_debug("sending MSG_SMS_DATA_DOWNLOAD_REQ");
		DataMsg = (struct SmsDataDownload_S *) msg;
		payload_size = min((int)size, SMS_MAX_PAYLOAD_SIZE);

		SMS_INIT_MSG(msg, MSG_SMS_DATA_DOWNLOAD_REQ,
				(u16) (sizeof(struct SmsMsgHdr_S) +
						sizeof(u32) + payload_size));

		DataMsg->MemAddr = coredev->current_address;
		rc = copy_from_user(DataMsg->Payload, (u8*)(buffer + offset), payload_size);

		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = smscore_sendrequest_and_wait(coredev, DataMsg,
			DataMsg->xMsgHeader.msgLength,
			&coredev->data_download_done);

		size -= payload_size;
		offset += payload_size;
		coredev->current_address += payload_size;
	}

	kfree(msg);

	return rc;
}
EXPORT_SYMBOL_GPL(smscore_send_fw_chunk);


/**
 * Send last chunk of firmware data using SMS MSGs
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param buffer  pointer to a buffer
 * @param size    size of buffer
 *
 * @return 0 on success, <0 on error.
 */
int smscore_send_last_fw_chunk(void* dev, void *buffer, size_t size) 
{
	int rc = 0;
	struct SmsMsgHdr_S *msg;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	
	rc = smscore_send_fw_chunk(coredev, buffer, size);
	if (rc < 0)
		return rc;
	
	/* PAGE_SIZE buffer shall be enough and dma aligned */
	msg = kmalloc(PAGE_SIZE, GFP_KERNEL | GFP_DMA);
	if (!msg)
		return -ENOMEM;
	
	if (coredev->mode == SMSHOSTLIB_DEVMD_NONE) {
		struct SmsMsgData_S *TriggerMsg =
				(struct SmsMsgData_S *) msg;

		sms_debug("sending MSG_SMS_SWDOWNLOAD_TRIGGER_REQ");
		SMS_INIT_MSG(msg, MSG_SMS_SWDOWNLOAD_TRIGGER_REQ,
				sizeof(struct SmsMsgHdr_S) +
				sizeof(u32) * 5);

		TriggerMsg->msgData[0] = coredev->start_address;
		/* Entry point */
		TriggerMsg->msgData[1] = 6; /* Priority */
		TriggerMsg->msgData[2] = 0x200; /* Stack size */
		TriggerMsg->msgData[3] = 0; /* Parameter */
		TriggerMsg->msgData[4] = 4; /* Task ID */

		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = smscore_sendrequest_and_wait(coredev,
			TriggerMsg,
			TriggerMsg->xMsgHeader.msgLength,
			&coredev->trigger_done);
	} else {
		SMS_INIT_MSG(msg, MSG_SW_RELOAD_EXEC_REQ,
				sizeof(struct SmsMsgHdr_S));
		smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
		rc = coredev->sendrequest_handler(coredev->context, msg,
				msg->msgLength);
	}

	/* clear start_address */
	coredev->start_address = 0;

	if (rc < 0)
		goto exit_fw_download;
			
	/*
	 * backward compatibility - wait to device_ready_done for
	 * not more than 400 ms
	 */
	wait_for_completion_timeout(&coredev->device_ready_done,
			msecs_to_jiffies(400));		

exit_fw_download:
	sms_debug("rc=%d, postload=0x%p ", rc, coredev->postload_handler);

	kfree(msg);

	return rc;
}
EXPORT_SYMBOL_GPL(smscore_send_last_fw_chunk);


/**
 * notifies all clients registered with the device, notifies hotplugs,
 * frees all buffers and coredev object
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 *
 * @return 0 on success, <0 on error.
 */
void smscore_unregister_device(void* dev)
{
	struct smscore_buffer_t *cb;
	int num_buffers = 0;
	int retry = 0;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	kmutex_lock(&g_smscore_deviceslock);

	cancel_delayed_work(&coredev->work_queue);
	/* Release input device (IR) resources */
#ifdef SMS_RC_SUPPORT_SUBSYS
	/* Release input device (IR) resources */
	sms_ir_exit(&coredev->ir);
#endif /*SMS_RC_SUPPORT_SUBSYS*/
	smscore_notify_clients(coredev);
	smscore_notify_callbacks(coredev, NULL, 0);

	/* at this point all buffers should be back
	 * onresponse must no longer be called */

	while (1) {
		while (!list_empty(&coredev->buffers)) {
			cb = (struct smscore_buffer_t *) coredev->buffers.next;
			list_del(&cb->entry);
			kfree(cb);
			num_buffers++;
		}
		if (num_buffers == coredev->num_buffers)
			break;
		if (++retry > 10) {
			sms_info("exiting although "
				 "not all buffers released.");
			break;
		}

		sms_info("waiting for %d buffer(s)",
			 coredev->num_buffers - num_buffers);
		kmutex_unlock(&g_smscore_deviceslock);
		msleep(100);
		kmutex_lock(&g_smscore_deviceslock);
	}

	sms_info("freed %d buffers", num_buffers);

	if (coredev->common_buffer)
	{
		if (!coredev->common_buffer_phys)
			kfree(coredev->common_buffer);
		else
			dma_free_coherent(NULL, coredev->common_buffer_size,
				coredev->common_buffer, coredev->common_buffer_phys);
	}
	if (coredev->fw_buf != NULL)
		kfree(coredev->fw_buf);

	list_del(&coredev->entry);
	kfree(coredev);

	kmutex_unlock(&g_smscore_deviceslock);
}
EXPORT_SYMBOL_GPL(smscore_unregister_device);

static int smscore_detect_mode(struct smscore_device_t *coredev)
{
	void *buffer = kmalloc(sizeof(struct SmsMsgHdr_S) + SMS_DMA_ALIGNMENT,
			       GFP_KERNEL | GFP_DMA);
	struct SmsMsgHdr_S *msg =
		(struct SmsMsgHdr_S *) SMS_ALIGN_ADDRESS(buffer);
	int rc;

	if (!buffer)
		return -ENOMEM;

	SMS_INIT_MSG(msg, MSG_SMS_GET_VERSION_EX_REQ,
		     sizeof(struct SmsMsgHdr_S));

	smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
	rc = smscore_sendrequest_and_wait(coredev, msg, msg->msgLength,
					  &coredev->version_ex_done);

	if (rc < 0) {
		sms_err("detect mode failed, rc %d", rc);
	}

	if (rc == -ETIME) {
		sms_err("MSG_SMS_GET_VERSION_EX_REQ failed first try");

		if (wait_for_completion_timeout(&coredev->resume_done,
						msecs_to_jiffies(5000))) {
			rc = smscore_sendrequest_and_wait(
				coredev, msg, msg->msgLength,
				&coredev->version_ex_done);
			if (rc < 0)
				sms_err("MSG_SMS_GET_VERSION_EX_REQ failed "
					"second try, rc %d", rc);
		} else
			rc = -ETIME;
	}

	kfree(buffer);

	return rc;
}

static char *smscore_fw_lkup[][SMS_NUM_OF_DEVICE_TYPES] = {
/*Stellar, NOVA A0, Nova B0, VEGA, VENICE, MING, PELE, RIO, DENVER_1530, DENVER_2160, QING, Zico, Santos*/
	/*DVBT*/
{ "none", "dvb_nova_12mhz.inp", "dvb_nova_12mhz_b0.inp", "none", "none", "none", "none", "dvb_rio.inp", "none", "none", "none", "none", "none"},
	/*DVBH*/
{ "none", "dvb_nova_12mhz.inp", "dvb_nova_12mhz_b0.inp", "none", "none", "none", "none", "dvbh_rio.inp", "none", "none", "none", "none", "none" },
	/*TDMB*/
{ "none", "tdmb_nova_12mhz.inp", "tdmb_nova_12mhz_b0.inp", "none", "none", "none", "none", "none", "none", "tdmb_denver.inp", "none", "none", "none" },
	/*DABIP*/
{ "none", "none", "none", "none", "none", "none", "none", "none", "none", "none", "none", "none", "none" },
/*DVBT_BDA*/
{ "none", "dvb_nova_12mhz.inp", "dvb_nova_12mhz_b0.inp", "none", "none", "none", "none", "dvb_rio.inp", "none", "none", "none", "none" },
	/*ISDBT*/
{ "none", "isdbt_nova_12mhz.inp", "isdbt_nova_12mhz_b0.inp", "none", "none", "none", "isdbt_pele.inp", "isdbt_rio.inp", "none", "none", "none", "isdbt_zico.inp", "isdbt_santos.inp" },
/*ISDBT_BDA*/
{ "none", "isdbt_nova_12mhz.inp", "isdbt_nova_12mhz_b0.inp", "none", "none", "none", "isdbt_pele.inp", "isdbt_rio.inp", "none", "none", "none", "isdbt_zico.inp", "isdbt_santos.inp" },
	/*CMMB*/
{ "none", "none", "none", "cmmb_vega_12mhz.inp", "cmmb_venice_12mhz.inp", "cmmb_ming_app.inp", "none", "none", "none", "none", "cmmb_qing_app.inp", "none", "none"  },
/*RAW - not supported*/
{ "none", "none", "none", "none", "none", "none", "none", "none", "none", "none", "none", "none", "none"  },
/*FM*/
{ "none", "none", "fm_radio.inp", "none", "none", "none", "none", "fm_radio_rio.inp", "none", "none", "none", "none", "none"  },
/*FM_BDA*/
{ "none", "none", "fm_radio.inp", "none", "none", "none", "none", "fm_radio_rio.inp", "none", "none" , "none", "none", "none" },
/*ATSC*/
{ "none", "none", "none", "none", "none", "none", "none", "none", "atsc_denver.inp", "none", "none", "none", "none"  }
};

/**
 * get firmware file name from one of the two mechanisms : sms_boards or 
 * smscore_fw_lkup.

 * @param coredev pointer to a coredev object returned by
 * 		  smscore_register_device
 * @param mode requested mode of operation
 * @param lookup if 1, always get the fw filename from smscore_fw_lkup 
 * 	 table. if 0, try first to get from sms_boards
 *
 * @return 0 on success, <0 on error.
 */
char *smscore_get_fw_filename(void* dev, int mode, int lookup) {
	char **fw;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	int board_id = coredev->board_id;
	enum sms_device_type_st type = smscore_registry_gettype(coredev->devpath); 

	if ( (board_id == SMS_BOARD_UNKNOWN) || 
	     (lookup == 1) ) {
		sms_debug("trying to get fw name from lookup table mode %d type %d", mode, type);
		return smscore_fw_lkup[mode][type];
	}
	
	sms_debug("trying to get fw name from sms_boards board_id %d mode %d", board_id, mode);
	fw = sms_get_board(board_id)->fw;
	if (fw == NULL) {
		sms_debug("cannot find fw name in sms_boards, getting from lookup table mode %d type %d", mode, type);
		return smscore_fw_lkup[mode][type];
	}

	if (fw[mode] == NULL) {
		sms_debug("cannot find fw name in sms_boards, getting from lookup table mode %d type %d", mode, type);
		return smscore_fw_lkup[mode][type];
	}

	return fw[mode];
}

/**
 * send init device request and wait for response
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param mode requested mode of operation
 *
 * @return 0 on success, <0 on error.
 */
int smscore_init_device(void* dev, int mode)
{
	void* buffer;
	struct SmsMsgData_S *msg;
	int rc = 0;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;

	buffer = kmalloc(sizeof(struct SmsMsgData_S) +
			SMS_DMA_ALIGNMENT, GFP_KERNEL | GFP_DMA);
	if (!buffer) {
		sms_err("Could not allocate buffer for "
				"init device message.");
		return -ENOMEM;
	}
	coredev->num_of_tuners = 1;
	msg = (struct SmsMsgData_S *)SMS_ALIGN_ADDRESS(buffer);
	SMS_INIT_MSG(&msg->xMsgHeader, MSG_SMS_INIT_DEVICE_REQ,
			sizeof(struct SmsMsgData_S));
	msg->msgData[0] = mode;

	smsendian_handle_tx_message((struct SmsMsgHdr_S *)msg);
	rc = smscore_sendrequest_and_wait(coredev, msg,
			msg->xMsgHeader. msgLength,
			&coredev->init_device_done);
	kfree(buffer);

	if (multi_tuner)
	{/* Set Multi Tuner mode to the device*/
		struct SetGpioMsg {
			struct SmsMsgHdr_S xMsgHeader;
			u32 msgData[2];
		} *pMsg;
		msleep(2000);
		buffer = kmalloc(sizeof(struct SmsMsgHdr_S) + (1 * sizeof(u32)) + SMS_DMA_ALIGNMENT,
				GFP_KERNEL | GFP_DMA);
		if (!buffer)
			return -ENOMEM;

		pMsg = (struct SetGpioMsg *) SMS_ALIGN_ADDRESS(buffer);

		pMsg->xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
		pMsg->xMsgHeader.msgDstId = HIF_TASK;
		pMsg->xMsgHeader.msgFlags = 0;
		pMsg->xMsgHeader.msgType = MSG_SMS_MRC_MODE_CHANGE_REQ;
		pMsg->xMsgHeader.msgLength = (u16) sizeof(struct SmsMsgHdr_S) + (1 * sizeof(u32));
		pMsg->msgData[0] = 1;

		/* Send message to SMS */
		smsendian_handle_tx_message((struct SmsMsgHdr_S *)pMsg);
		rc = smscore_sendrequest_and_wait(coredev, pMsg, sizeof(struct SmsMsgHdr_S) + (1 * sizeof(u32)),
				&coredev->mrc_change_mode_done);

		if (rc != 0) {
			if (rc == -ETIME)
				sms_err("MSG_SMS_MRC_MODE_CHANGE_REQ timeout");
			else
				sms_err("MSG_SMS_MRC_MODE_CHANGE_REQ error");
		}
		kfree(buffer);

	}



	return rc;
}

/**
 * calls device handler to change mode of operation
 * NOTE: stellar/usb may disconnect when changing mode
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param mode requested mode of operation
 *
 * @return 0 on success, <0 on error.
 */
int smscore_set_device_mode(void* dev, int mode)
{
	int rc = 0;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	int initial_mode = smscore_registry_getmode(coredev->devpath);

	sms_debug("set device mode to %d", mode);


	if (!(coredev->device_flags & SMS_DEVICE_NOT_READY)) {
		rc = smscore_detect_mode(coredev);
		if (rc < 0) {
			sms_err("mode detect failed %d", rc);
			return rc;
		}
	}

	if (coredev->mode == mode) {
		rc = smscore_init_device(coredev, mode);
		if (rc < 0) 
			sms_err("device init failed, rc %d.", rc);
		sms_info("device mode %d already set.", mode);
		return 0;
	}

	smscore_registry_setmode(coredev->devpath, mode);

	if (!(coredev->modes_supported & (1 << mode))) {
		rc = smscore_load_firmware_from_file(coredev, mode, 0, NULL);

		/* 
		* try again with the default firmware -
		* get the fw filename from look-up table
		*/
		if (rc < 0) {
			sms_debug("error %d loading firmware, "
				"trying again with default firmware", rc);
			rc = smscore_load_firmware_from_file(coredev, mode, 1, NULL);
		}

		if (rc < 0) {
			smscore_registry_setmode(coredev->devpath, initial_mode);
			sms_debug("error %d loading firmware", rc);
			return rc;
		}

		sms_info("firmware reading  success");
	} 
	else 
	{
		sms_info("mode %d is already supported by running firmware", mode);
	}


	if (mode < SMSHOSTLIB_DEVMD_DVBT || mode >= SMSHOSTLIB_DEVMD_MAX) {
		sms_err("invalid mode specified %d", mode);
		return -EINVAL;
	}
	if (coredev->mode == mode && sms_static_firmware) {
		sms_info("device mode %d already set", mode);
		return 0;
	}
	sms_info("sending fw from buffer.");
	if (coredev->device_flags & SMS_DEVICE_FAMILY2)
		smscore_load_firmware_family2(coredev, 
						sms_static_firmware->data, 
						sms_static_firmware->fw_buf_size);
	else
	{
		if (!coredev->loadfirmware_handler)
		{
			sms_err("fw loading routing is not defined by phyisical driver layer.");
			return -EINVAL;
		}			
		coredev->loadfirmware_handler(coredev->context, 
					sms_static_firmware->data,
					sms_static_firmware->fw_buf_size);

	}
	rc = smscore_init_device(coredev, mode);

	if (rc < 0) 
		sms_err("device init failed, rc %d.", rc);
	else {
		coredev->mode = mode;
		coredev->device_flags &= ~SMS_DEVICE_NOT_READY;
	}

	return rc;
}
EXPORT_SYMBOL_GPL(smscore_set_device_mode);

/**
 * configures device features according to board configuration structure.
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 *
 * @return 0 on success, <0 on error.
 */
int smscore_configure_board(void* dev) {
	struct sms_board* board;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;

	board = sms_get_board(coredev->board_id);
	if (!board)
	{
		sms_err("no board configuration exist.");
		return -1;
	}
	
	if (board->mtu)
	{
		struct SmsMsgData_S MtuMsg;
		sms_debug("set max transmit unit %d", board->mtu);

		MtuMsg.xMsgHeader.msgSrcId = 0;
		MtuMsg.xMsgHeader.msgDstId = HIF_TASK;
		MtuMsg.xMsgHeader.msgFlags = 0;
		MtuMsg.xMsgHeader.msgType = MSG_SMS_SET_MAX_TX_MSG_LEN_REQ;
		MtuMsg.xMsgHeader.msgLength = sizeof(MtuMsg);
		MtuMsg.msgData[0] = board->mtu;

		smsendian_handle_tx_message((struct SmsMsgHdr_S *)&MtuMsg);
		coredev->sendrequest_handler(coredev->context, &MtuMsg, sizeof(MtuMsg));
	}

	if (board->crystal)
	{
		struct SmsMsgData_S CrysMsg;
		sms_debug("set crystal value %d", board->crystal);

		SMS_INIT_MSG(&CrysMsg.xMsgHeader, 
				MSG_SMS_NEW_CRYSTAL_REQ,
				sizeof(CrysMsg));
		CrysMsg.msgData[0] = board->crystal;

		smsendian_handle_tx_message((struct SmsMsgHdr_S *)&CrysMsg);
		coredev->sendrequest_handler(coredev->context, &CrysMsg, sizeof(CrysMsg));
	}

	return 0;
}

/**
 * calls device handler to get current mode of operation
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 *
 * @return current mode
 */
int smscore_get_device_mode(void *coredev)
{
	return ((struct smscore_device_t*)coredev)->mode;
}
EXPORT_SYMBOL_GPL(smscore_get_device_mode);

/**
 * find client by response id & type within the clients list.
 * return client handle or NULL.
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param data_type client data type (SMS_DONT_CARE for all types)
 * @param id client id (SMS_DONT_CARE for all id)
 *
 */
static struct
smscore_client_t *smscore_find_client(void* dev,
				      int data_type, int id)
{
	struct smscore_client_t *client = NULL;
	struct list_head *next, *first;
	unsigned long flags;
	struct list_head *firstid, *nextid;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;


	spin_lock_irqsave(&coredev->clientslock, flags);
	first = &coredev->clients;
	for (next = first->next;
	     (next != first) && !client;
	     next = next->next) {
		firstid = &((struct smscore_client_t *)next)->idlist;
		for (nextid = firstid->next;
		     nextid != firstid;
		     nextid = nextid->next) {
			if ((((struct smscore_idlist_t *)nextid)->id == id) &&
			    (((struct smscore_idlist_t *)nextid)->data_type == data_type ||
			    (((struct smscore_idlist_t *)nextid)->data_type == 0))) {
				client = (struct smscore_client_t *) next;
				break;
			}
		}
	}
	spin_unlock_irqrestore(&coredev->clientslock, flags);
	return client;
}

/**
 * find client by response id/type, call clients onresponse handler
 * return buffer to pool on error
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param cb pointer to response buffer descriptor
 *
 */
void smscore_onresponse(void* dev, struct smscore_buffer_t *cb) 
{
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	struct SmsMsgHdr_S *phdr = (struct SmsMsgHdr_S *) ((u8 *) cb->p
			+ cb->offset);
	struct smscore_client_t *client;
	int rc = -EBUSY;
	static unsigned long last_sample_time; /* = 0; */
	static int data_total; /* = 0; */
	unsigned long time_now = jiffies_to_msecs(jiffies);

	if (!last_sample_time)
		last_sample_time = time_now;

	if (time_now - last_sample_time > 10000) {
		sms_debug("\ndata rate %d bytes/secs",
			  (int)((data_total * 1000) /
				(time_now - last_sample_time)));

		last_sample_time = time_now;
		data_total = 0;
	}

	data_total += cb->size;
	/* Do we need to re-route? */
	if ((phdr->msgType == MSG_SMS_HO_PER_SLICES_IND) ||
			(phdr->msgType == MSG_SMS_TRANSMISSION_IND)) {
		if (coredev->mode == SMSHOSTLIB_DEVMD_DVBT_BDA)
			phdr->msgDstId = DVBT_BDA_CONTROL_MSG_ID;
	}
	client = smscore_find_client(coredev, phdr->msgType, phdr->msgDstId);
	/* If no client registered for type & id,
	 * check for control client where type is not registered */
	if (client)
		rc = client->onresponse_handler(client->context, cb);
	if (rc < 0) {
		smsendian_handle_rx_message((struct SmsMsgData_S *)phdr);

		switch (phdr->msgType) {
		case MSG_SMS_ISDBT_TUNE_RES:
			sms_debug("MSG_SMS_ISDBT_TUNE_RES");
			break;
		case MSG_SMS_RF_TUNE_RES:
			sms_debug("MSG_SMS_RF_TUNE_RES");
			break;
		case MSG_SMS_SIGNAL_DETECTED_IND:
			sms_debug("MSG_SMS_SIGNAL_DETECTED_IND");
			break;
		case MSG_SMS_NO_SIGNAL_IND:
			sms_debug("MSG_SMS_NO_SIGNAL_IND");
			break;
		case MSG_SMS_SPI_INT_LINE_SET_RES:
			sms_debug("MSG_SMS_SPI_INT_LINE_SET_RES");
			break;
		case MSG_SMS_INTERFACE_LOCK_IND: 
			sms_debug("MSG_SMS_INTERFACE_LOCK_IND");
			break;
		case MSG_SMS_INTERFACE_UNLOCK_IND:
			sms_debug("MSG_SMS_INTERFACE_UNLOCK_IND");
			break;
		case MSG_SMS_GET_VERSION_EX_RES: {
			struct SmsVersionRes_S *ver = (struct SmsVersionRes_S *)phdr;
			sms_debug("MSG_SMS_GET_VERSION_EX_RES "
				  "id %d prots 0x%x ver %d.%d",
					ver->xVersion.FirmwareId,
					ver->xVersion.SupportedProtocols,
					ver->xVersion.RomVer.Major,
					ver->xVersion.RomVer.Minor);

			coredev->mode = ver->xVersion.FirmwareId == 255 ?
				SMSHOSTLIB_DEVMD_NONE : ver->xVersion.FirmwareId;
			coredev->modes_supported = ver->xVersion.SupportedProtocols;

			complete(&coredev->version_ex_done);
			break;
		}
		case MSG_SMS_INIT_DEVICE_RES:
			sms_debug("MSG_SMS_INIT_DEVICE_RES");
			complete(&coredev->init_device_done);
			break;
		case MSG_SW_RELOAD_START_RES:
			sms_debug("MSG_SW_RELOAD_START_RES");
			complete(&coredev->reload_start_done);
			break;
		case MSG_SMS_DATA_DOWNLOAD_RES:
			complete(&coredev->data_download_done);
			break;
		case MSG_SW_RELOAD_EXEC_RES:
			sms_debug("MSG_SW_RELOAD_EXEC_RES");
			break;
		case MSG_SMS_DATA_VALIDITY_RES:
		{
			struct SmsMsgData_S *validity = (struct SmsMsgData_S *) phdr;			
			sms_debug("MSG_SMS_DATA_VALIDITY_RES, checksum = 0x%x", validity->msgData[0]);
			complete(&coredev->data_validity_done);
			break;
		}
		case MSG_SMS_SWDOWNLOAD_TRIGGER_RES:
			sms_debug("MSG_SMS_SWDOWNLOAD_TRIGGER_RES");
			complete(&coredev->trigger_done);
			break;
		case MSG_SMS_SLEEP_RESUME_COMP_IND:
			sms_debug("MSG_SMS_SLEEP_RESUME_COMP_IND");
			complete(&coredev->resume_done);
			break;
		case MSG_SMS_GPIO_CONFIG_EX_RES:
			sms_debug("MSG_SMS_GPIO_CONFIG_EX_RES");
			complete(&coredev->gpio_configuration_done);
			break;
		case MSG_SMS_GPIO_SET_LEVEL_RES:
			sms_debug("MSG_SMS_GPIO_SET_LEVEL_RES");
			complete(&coredev->gpio_set_level_done);
			break;
		case MSG_SMS_GPIO_GET_LEVEL_RES:
		{
			u32 *msgdata = (u32 *) phdr;
			coredev->gpio_get_res = msgdata[1];
			sms_debug("MSG_SMS_GPIO_GET_LEVEL_RES gpio level %d",
					coredev->gpio_get_res);
			complete(&coredev->gpio_get_level_done);
			break;
		}
		case MSG_SMS_MRC_MODE_CHANGE_RES:
			sms_debug("MSG_SMS_MRC_MODE_CHANGE_RES");
			complete(&coredev->mrc_change_mode_done);
			break;
#ifdef SMS_RC_SUPPORT_SUBSYS
		case MSG_SMS_START_IR_RES:
			complete(&coredev->ir_init_done);
			break;
		case MSG_SMS_IR_SAMPLES_IND:
			sms_ir_event(&coredev->ir,
				(const char *)
				((char *)phdr
				+ sizeof(struct SmsMsgHdr_S)),
				(int)phdr->msgLength
				- sizeof(struct SmsMsgHdr_S));
			break;
#endif
		case MSG_SMS_SLAVE_DEVICE_DETECTED:
/* In dual freq mode - each slave is introduced as separate device.*/
			sms_debug("MSG_SMS_SLAVE_DEVICE_DETECTED");
			if (coredev->multi_tuner)
				coredev->num_of_tuners++;				
		default:
			break;
		}
		smscore_putbuffer(coredev, cb);
	}
}
EXPORT_SYMBOL_GPL(smscore_onresponse);

/**
 * return pointer to next free buffer descriptor from core pool
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 *
 * @return pointer to descriptor on success, NULL on error.
 */
struct smscore_buffer_t *smscore_getbuffer(void* dev)
{
	struct smscore_buffer_t *cb = NULL;
	unsigned long flags;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;

	DEFINE_WAIT(wait);

	spin_lock_irqsave(&coredev->bufferslock, flags);

	/* set the current process state to interruptible sleep
	 * in case schedule() will be called, this process will go to sleep 
	 * and woken up only when a new buffer is available (see smscore_putbuffer)
	 */
	prepare_to_wait(&coredev->buffer_mng_waitq, &wait, TASK_INTERRUPTIBLE);

	if (list_empty(&coredev->buffers)) {
		sms_debug("no avaliable common buffer, need to schedule");

		/* 
         * before going to sleep, release the lock 
         */
		spin_unlock_irqrestore(&coredev->bufferslock, flags);

		schedule();

		sms_debug("wake up after schedule()");

		/* 
         * acquire the lock again 
         */
	spin_lock_irqsave(&coredev->bufferslock, flags);
	}

	/* 
         * in case that schedule() was skipped, set the process state to running
	 */
	finish_wait(&coredev->buffer_mng_waitq, &wait);

	/* 
         * verify that the list is not empty, since it might have been 
	 * emptied during the sleep
	 * comment : this sitation can be avoided using spin_unlock_irqrestore_exclusive	
	 */	
	if (list_empty(&coredev->buffers)) {
		sms_err("failed to allocate buffer, returning NULL");
	spin_unlock_irqrestore(&coredev->bufferslock, flags);
		return NULL;
}

	cb = (struct smscore_buffer_t *) coredev->buffers.next;
	list_del(&cb->entry);

	spin_unlock_irqrestore(&coredev->bufferslock, flags);

	return cb;
}
EXPORT_SYMBOL_GPL(smscore_getbuffer);

/**
 * return buffer descriptor to a pool
 *
 * @param coredev pointer to a coredev object returned by
 *                smscore_register_device
 * @param cb pointer buffer descriptor
 *
 */
void smscore_putbuffer(void* dev, struct smscore_buffer_t *cb) 
{
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	list_add_locked(&cb->entry, &coredev->buffers, &coredev->bufferslock);
	wake_up_interruptible(&coredev->buffer_mng_waitq);
}
EXPORT_SYMBOL_GPL(smscore_putbuffer);

static int smscore_validate_client(struct smscore_device_t *coredev,
				   struct smscore_client_t *client,
				   int data_type, int id)
{
	struct smscore_idlist_t *listentry;
	struct smscore_client_t *registered_client;

	if (!client) {
		sms_err("bad parameter.");
		return -EFAULT;
	}
	registered_client = smscore_find_client(coredev, data_type, id);
	if (registered_client == client)
		return 0;

	if (registered_client) {
		sms_err("The msg ID already registered to another client.");
		return -EEXIST;
	}
	listentry = kzalloc(sizeof(struct smscore_idlist_t), GFP_KERNEL);
	if (!listentry) {
		sms_err("Can't allocate memory for client id.");
		return -ENOMEM;
	}
	listentry->id = id;
	listentry->data_type = data_type;
	list_add_locked(&listentry->entry, &client->idlist,
			&coredev->clientslock);
	return 0;
}

/**
 * creates smsclient object, check that id is taken by another client
 *
 * @param coredev pointer to a coredev object from clients hotplug
 * @param initial_id all messages with this id would be sent to this client
 * @param data_type all messages of this type would be sent to this client
 * @param onresponse_handler client handler that is called to
 *                           process incoming messages
 * @param onremove_handler client handler that is called when device is removed
 * @param context client-specific context
 * @param client pointer to a value that receives created smsclient object
 *
 * @return 0 on success, <0 on error.
 */
int smscore_register_client(void* dev,
			    struct smsclient_params_t *params,
			    struct smscore_client_t **client)
{
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	struct smscore_client_t *newclient;
	/* check that no other channel with same parameters exists */
	if (smscore_find_client(coredev, params->data_type,
				params->initial_id)) {
		sms_err("Client already exist.");
		return -EEXIST;
	}

	newclient = kzalloc(sizeof(struct smscore_client_t), GFP_KERNEL);
	if (!newclient) {
		sms_err("Failed to allocate memory for client.");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&newclient->idlist);
	newclient->coredev = coredev;
	newclient->onresponse_handler = params->onresponse_handler;
	newclient->onremove_handler = params->onremove_handler;
	newclient->context = params->context;
	list_add_locked(&newclient->entry, &coredev->clients,
			&coredev->clientslock);
	smscore_validate_client(coredev, newclient, params->data_type,
				params->initial_id);
	*client = newclient;
	sms_debug("register new client 0x%p DT=%d ID=%d",
		params->context, params->data_type, params->initial_id);

	return 0;
}
EXPORT_SYMBOL_GPL(smscore_register_client);

/**
 * frees smsclient object and all subclients associated with it
 *
 * @param client pointer to smsclient object returned by
 *               smscore_register_client
 *
 */
void smscore_unregister_client(struct smscore_client_t *client)
{
	struct smscore_device_t *coredev = client->coredev;
	unsigned long flags;

	spin_lock_irqsave(&coredev->clientslock, flags);


	while (!list_empty(&client->idlist)) {
		struct smscore_idlist_t *identry =
			(struct smscore_idlist_t *) client->idlist.next;
		list_del(&identry->entry);
		kfree(identry);
	}

	sms_info("unregister client 0x%p", client->context);

	list_del(&client->entry);
	kfree(client);

	spin_unlock_irqrestore(&coredev->clientslock, flags);
}
EXPORT_SYMBOL_GPL(smscore_unregister_client);

/**
 * verifies that source id is not taken by another client,
 * calls device handler to send requests to the device
 *
 * @param client pointer to smsclient object returned by
 *               smscore_register_client
 * @param buffer pointer to a request buffer
 * @param size size (in bytes) of request buffer
 *
 * @return 0 on success, <0 on error.
 */
int smsclient_sendrequest(struct smscore_client_t *client,
			  void *buffer, size_t size)
{
	struct smscore_device_t *coredev;
	struct SmsMsgHdr_S *phdr = (struct SmsMsgHdr_S *) buffer;
	int rc;

	if (client == NULL) {
		sms_err("Got NULL client");
		return -EINVAL;
	}

	coredev = client->coredev;

	/* check that no other channel with same id exists */
	if (coredev == NULL) {
		sms_err("Got NULL coredev");
		return -EINVAL;
	}

	rc = smscore_validate_client(client->coredev, client, 0,
				     phdr->msgSrcId);
	if (rc < 0)
	{
		sms_err("client not validated. message discarded!");
		return rc;
	}
	

	return coredev->sendrequest_handler(coredev->context, buffer, size);
}
EXPORT_SYMBOL_GPL(smsclient_sendrequest);

#ifdef SMS_HOSTLIB_SUBSYS
/**
 * return the size of large (common) buffer
 *
 * @param coredev pointer to a coredev object from clients hotplug
 *
 * @return size (in bytes) of the buffer
 */
int smscore_get_common_buffer_size(void* dev)
{
	return ((struct smscore_device_t*)dev)->common_buffer_size;
}
EXPORT_SYMBOL_GPL(smscore_get_common_buffer_size);

/**
 * maps common buffer (if supported by platform)
 *
 * @param coredev pointer to a coredev object from clients hotplug
 * @param vma pointer to vma struct from mmap handler
 *
 * @return 0 on success, <0 on error.
 */
int smscore_map_common_buffer(void* dev,
		struct vm_area_struct *vma)
{
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	unsigned long page;
	unsigned long end = vma->vm_end,
	start = vma->vm_start,
	size = PAGE_ALIGN(coredev->common_buffer_size);

	if (!(vma->vm_flags & (VM_READ | VM_WRITE | VM_SHARED)))  {
		sms_err("invalid vm flags");
		return -EINVAL;
		}

	if ((end - start) != size) {
		sms_err("invalid size %d expected %d",
				(int)(end - start), (int)size);
		return -EINVAL;
	}
	if (!coredev->common_buffer_phys)
	{
		page =  virt_to_phys((void *)coredev->common_buffer);
	}
	else
	{
		page =  coredev->common_buffer_phys;
	}
	while (size > 0) {
		if (remap_pfn_range(vma, start, page >> PAGE_SHIFT, PAGE_SIZE, pgprot_noncached(vma->vm_page_prot)))
		{
			sms_err("remap_page_range failed");
			return -EAGAIN;
		}
	        start += PAGE_SIZE;
		page += PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(smscore_map_common_buffer);

int smscore_set_gpio(void* dev, u32 pin, int level)
{
	struct {
		struct SmsMsgHdr_S hdr;
		u32 data[3];
	} msg;

	if (pin > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	msg.hdr.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	msg.hdr.msgDstId = HIF_TASK;
	msg.hdr.msgFlags = 0;
	msg.hdr.msgType  = MSG_SMS_GPIO_SET_LEVEL_REQ;
	msg.hdr.msgLength = sizeof(msg);

	msg.data[0] = pin;
	msg.data[1] = level ? 1 : 0;
	msg.data[2] = 0;

	return ((struct smscore_device_t *)dev)->sendrequest_handler(((struct smscore_device_t *)dev)->context,
					    &msg, sizeof(msg));
}


int smscore_set_power_mode(enum sms_power_mode_st mode)
{
	if (g_powermode_notify)
		g_powermode_notify(mode);
	
	return 0;
}
EXPORT_SYMBOL_GPL(smscore_set_power_mode);

int smscore_register_power_mode_handler(powermode_t powermode_handler)
{
	if (powermode_handler)
		g_powermode_notify = powermode_handler;
		
	return 0;
}
EXPORT_SYMBOL_GPL(smscore_register_power_mode_handler);

int smscore_un_register_power_mode_handler()
{
	g_powermode_notify = NULL;
		
	return 0;
}
EXPORT_SYMBOL_GPL(smscore_un_register_power_mode_handler);

#endif /* SMS_HOSTLIB_SUBSYS */

/* new GPIO management implementation */
static int GetGpioPinParams(u32 PinNum, u32 *pTranslatedPinNum,
		u32 *pGroupNum, u32 *pGroupCfg) {

	*pGroupCfg = 1;

	if (PinNum <= 1)	{
		*pTranslatedPinNum = 0;
		*pGroupNum = 9;
		*pGroupCfg = 2;
	} else if (PinNum >= 2 && PinNum <= 6) {
		*pTranslatedPinNum = 2;
		*pGroupNum = 0;
		*pGroupCfg = 2;
	} else if (PinNum >= 7 && PinNum <= 11) {
		*pTranslatedPinNum = 7;
		*pGroupNum = 1;
	} else if (PinNum >= 12 && PinNum <= 15) {
		*pTranslatedPinNum = 12;
		*pGroupNum = 2;
		*pGroupCfg = 3;
	} else if (PinNum == 16) {
		*pTranslatedPinNum = 16;
		*pGroupNum = 23;
	} else if (PinNum >= 17 && PinNum <= 24) {
		*pTranslatedPinNum = 17;
		*pGroupNum = 3;
	} else if (PinNum == 25) {
		*pTranslatedPinNum = 25;
		*pGroupNum = 6;
	} else if (PinNum >= 26 && PinNum <= 28) {
		*pTranslatedPinNum = 26;
		*pGroupNum = 4;
	} else if (PinNum == 29) {
		*pTranslatedPinNum = 29;
		*pGroupNum = 5;
		*pGroupCfg = 2;
	} else if (PinNum == 30) {
		*pTranslatedPinNum = 30;
		*pGroupNum = 8;
	} else if (PinNum == 31) {
		*pTranslatedPinNum = 31;
		*pGroupNum = 17;
	} else
		return -1;

	*pGroupCfg <<= 24;

	return 0;
}

int smscore_gpio_configure(void* dev, u8 PinNum,
		struct smscore_gpio_config *pGpioConfig) {

	u32 totalLen;
	u32 TranslatedPinNum = 0;
	u32 GroupNum = 0;
	u32 ElectricChar;
	u32 groupCfg;
	void *buffer;
	int rc;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;

	struct SetGpioMsg {
		struct SmsMsgHdr_S xMsgHeader;
		u32 msgData[6];
	} *pMsg;


	if (PinNum > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	if (pGpioConfig == NULL)
		return -EINVAL;

	totalLen = sizeof(struct SmsMsgHdr_S) + (sizeof(u32) * 6);

	buffer = kmalloc(totalLen + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	pMsg = (struct SetGpioMsg *) SMS_ALIGN_ADDRESS(buffer);

	pMsg->xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	pMsg->xMsgHeader.msgDstId = HIF_TASK;
	pMsg->xMsgHeader.msgFlags = 0;
	pMsg->xMsgHeader.msgLength = (u16) totalLen;
	pMsg->msgData[0] = PinNum;

	if (!(coredev->device_flags & SMS_DEVICE_FAMILY2)) {
		pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_CONFIG_REQ;
		if (GetGpioPinParams(PinNum, &TranslatedPinNum, &GroupNum,
				&groupCfg) != 0) {
			rc = -EINVAL;
			goto free;
		}

		pMsg->msgData[1] = TranslatedPinNum;
		pMsg->msgData[2] = GroupNum;
		ElectricChar = (pGpioConfig->pull_up_down)
				| (pGpioConfig->input_characteristics << 2)
				| (pGpioConfig->output_slew_rate << 3)
				| (pGpioConfig->output_driving << 4);
		pMsg->msgData[3] = ElectricChar;
		pMsg->msgData[4] = pGpioConfig->direction;
		pMsg->msgData[5] = groupCfg;
	} else {
		pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_CONFIG_EX_REQ;
		pMsg->msgData[1] = pGpioConfig->pull_up_down;
		pMsg->msgData[2] = pGpioConfig->output_slew_rate;
		pMsg->msgData[3] = pGpioConfig->output_driving;
		pMsg->msgData[4] = pGpioConfig->direction;
		pMsg->msgData[5] = 0;
	}

	smsendian_handle_tx_message((struct SmsMsgHdr_S *)pMsg);
	rc = smscore_sendrequest_and_wait(coredev, pMsg, totalLen,
			&coredev->gpio_configuration_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_configure timeout");
		else
			sms_err("smscore_gpio_configure error");
	}
free:
	kfree(buffer);

	return rc;
}

int smscore_gpio_set_level(void* dev, u8 PinNum, u8 NewLevel) {

	u32 totalLen;
	int rc;
	void *buffer;
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;

	struct SetGpioMsg {
		struct SmsMsgHdr_S xMsgHeader;
		u32 msgData[3]; /* keep it 3 ! */
	} *pMsg;

	if ((NewLevel > 1) || (PinNum > MAX_GPIO_PIN_NUMBER))
		return -EINVAL;

	totalLen = sizeof(struct SmsMsgHdr_S) +
			(3 * sizeof(u32)); /* keep it 3 ! */

	buffer = kmalloc(totalLen + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	pMsg = (struct SetGpioMsg *) SMS_ALIGN_ADDRESS(buffer);

	pMsg->xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	pMsg->xMsgHeader.msgDstId = HIF_TASK;
	pMsg->xMsgHeader.msgFlags = 0;
	pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_SET_LEVEL_REQ;
	pMsg->xMsgHeader.msgLength = (u16) totalLen;
	pMsg->msgData[0] = PinNum;
	pMsg->msgData[1] = NewLevel;

	/* Send message to SMS */
	smsendian_handle_tx_message((struct SmsMsgHdr_S *)pMsg);
	rc = smscore_sendrequest_and_wait(coredev, pMsg, totalLen,
			&coredev->gpio_set_level_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_set_level timeout");
		else
			sms_err("smscore_gpio_set_level error");
	}
	kfree(buffer);

	return rc;
}

int smscore_gpio_get_level(void* dev, u8 PinNum,
		u8 *level) {

	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;
	u32 totalLen;
	int rc;
	void *buffer;

	struct SetGpioMsg {
		struct SmsMsgHdr_S xMsgHeader;
		u32 msgData[2];
	} *pMsg;


	if (PinNum > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	totalLen = sizeof(struct SmsMsgHdr_S) + (2 * sizeof(u32));

	buffer = kmalloc(totalLen + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	pMsg = (struct SetGpioMsg *) SMS_ALIGN_ADDRESS(buffer);

	pMsg->xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	pMsg->xMsgHeader.msgDstId = HIF_TASK;
	pMsg->xMsgHeader.msgFlags = 0;
	pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_GET_LEVEL_REQ;
	pMsg->xMsgHeader.msgLength = (u16) totalLen;
	pMsg->msgData[0] = PinNum;
	pMsg->msgData[1] = 0;

	/* Send message to SMS */
	smsendian_handle_tx_message((struct SmsMsgHdr_S *)pMsg);
	rc = smscore_sendrequest_and_wait(coredev, pMsg, totalLen,
			&coredev->gpio_get_level_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_get_level timeout");
		else
			sms_err("smscore_gpio_get_level error");
	}
	kfree(buffer);

	/* Its a race between other gpio_get_level() and the copy of the single
	 * global 'coredev->gpio_get_res' to  the function's variable 'level'
	 */
	*level = coredev->gpio_get_res;

	return rc;
}


/**
 * return device properties
 *
 * @param dev pointer to core device
 *
 * @param properties pointer to device properties structure where information is returned
 *
 * @return 0 on success, <0 on error.
 */
int smscore_get_device_properties(void* dev, struct sms_properties_t *properties)
{
	struct smscore_device_t *coredev = (struct smscore_device_t *)dev;

	properties->board_id = coredev ->board_id;
	properties->mode = coredev->mode;
	properties->num_of_tuners = coredev->num_of_tuners;
	return 0;
}
EXPORT_SYMBOL_GPL(smscore_get_device_properties);


static int __init smscore_module_init(void)
{
	int rc = 0;

	sms_info("");
	sms_info("smsmdtv register, version %d.%d.%d",
		MAJOR_VERSION, MINOR_VERSION, SUB_VERSION);
	/* 
	 * first, create global core device global linked lists
	 */

	INIT_LIST_HEAD(&g_smscore_notifyees);
	INIT_LIST_HEAD(&g_smscore_devices);
	kmutex_init(&g_smscore_deviceslock);

	INIT_LIST_HEAD(&g_smscore_registry);
	kmutex_init(&g_smscore_registrylock);
	sms_static_firmware = NULL; 
	sms_info("fw buffer is free.");

	g_powermode_notify = NULL;
	/* 
	 * second, register sub system adapter objects 
	 */

#ifdef SMS_NET_SUBSYS
	/* Network device register */
	rc = smsnet_register();
	sms_debug("smsnet_register, %d", rc);
	if (rc) {
		sms_err("Error registering Siano's network client.\n");
		goto smsnet_error;
	}
#endif

#ifdef SMS_HOSTLIB_SUBSYS
	/* Char device register */
	rc = smschar_register();
	sms_debug("smschar registered, %d", rc);
	if (rc) {
		sms_err("Error registering Siano's char device client.\n");
		goto smschar_error;
	}
#endif

#ifdef SMS_DVB3_SUBSYS
	/* DVB-API v.3 register */
	rc = smsdvb_register();
	sms_debug("smsdvb registered, %d", rc);
	if (rc) {
		sms_err("Error registering DVB client.\n");
		goto smsdvb_error;
	}
#endif

	/* 
	 * third, register interfaces objects 
	 */

#ifdef SMS_USB_DRV
	/* USB Register */
	rc = smsusb_register();
	sms_debug("smsusb registered, %d", rc);
	if (rc) {
		sms_err("Error registering USB bus driver.\n");
		goto smsusb_error;
	}
#endif

#ifdef SMS_SDIO_DRV
	/* SDIO Register */
	rc = smssdio_register();
	sms_debug("smssdio registered, %d", rc);
	if (rc) {
		sms_err("Error registering SDIO bus driver.\n");
		goto smssdio_error;
	}
#endif

#ifdef SMS_SPI_DRV
	/* SPI Register */
	rc = smsspi_register();
	sms_debug("smsspi registered, %d", rc);
	if (rc) {
		sms_err("Error registering SPI bus driver.\n");
		goto smsspi_error;
	}
#endif
#ifdef SMS_I2C_DRV
	rc = smsi2c_register();
	sms_debug("smsi2c registered, %d", rc);
	if (rc) {
		sms_err("Error registering I2C bus driver.\n");
		goto smsi2c_error;
	}
#endif

	sms_info("smsmdtv registered succesfully\n");
	return rc;

#ifdef SMS_I2C_DRV
	smsi2c_unregister();
smsi2c_error:
#endif

#ifdef SMS_SPI_DRV
	smsspi_unregister();
smsspi_error:
#endif

#ifdef SMS_SDIO_DRV
	smssdio_unregister();
smssdio_error:
#endif

#ifdef SMS_USB_DRV
	smsusb_unregister();
smsusb_error:
#endif

#ifdef SMS_DVB3_SUBSYS
	smsdvb_unregister();
smsdvb_error:
#endif

#ifdef SMS_HOSTLIB_SUBSYS
	smschar_unregister();
smschar_error:
#endif

#ifdef SMS_NET_SUBSYS
	smsnet_unregister();
smsnet_error:
#endif

	sms_err("rc %d", rc);
	printk(KERN_INFO "%s, rc %d\n", __func__, rc);

	return rc;
}

static void __exit smscore_module_exit(void)
{
	sms_info("");
	sms_info("smsmdtv unregister");

	/* 
	 * first, unregister interfaces objects
	 */

#ifdef SMS_USB_DRV
	/* USB unregister */
	smsusb_unregister();
#endif

#ifdef SMS_SDIO_DRV
	/* SDIO unregister */
	smssdio_unregister();
#endif

#ifdef SMS_SPI_DRV
	/* SPI unregister */
	smsspi_unregister();
#endif

#ifdef SMS_I2C_DRV
	smsi2c_unregister();
#endif


	/* 
	 * second, unregister sub system adapter objects 
	 * at this point, the core device should be unregistered and freed, and
	 * all of the clients should be notified about it.
	 * otherwise, messages can be transmitted although there is no 
	 * registerd interface.
	 */

#ifdef SMS_NET_SUBSYS
	/* Network device unregister */
	smsnet_unregister();
#endif

#ifdef SMS_HOSTLIB_SUBSYS
	/* Char device unregister */
	smschar_unregister();
#endif

#ifdef SMS_DVB3_SUBSYS
	/* DVB-API v.3 unregister */
	smsdvb_unregister();
#endif

	if (sms_static_firmware != NULL)
		kfree(sms_static_firmware);
	sms_static_firmware = NULL;

	/* 
	 * third, empty global core device global linked lists
	 */

	kmutex_lock(&g_smscore_deviceslock);
	while (!list_empty(&g_smscore_notifyees)) {
		struct smscore_device_notifyee_t *notifyee =
		(struct smscore_device_notifyee_t *)
		g_smscore_notifyees.next;

		list_del(&notifyee->entry);
		kfree(notifyee);
	}
	kmutex_unlock(&g_smscore_deviceslock);
	kmutex_lock(&g_smscore_registrylock);
	while (!list_empty(&g_smscore_registry)) {
		struct smscore_registry_entry_t *entry =
		(struct smscore_registry_entry_t *)
		g_smscore_registry.next;

		list_del(&entry->entry);
		kfree(entry);
	}
	kmutex_unlock(&g_smscore_registrylock);

	sms_info("smsmdtv unregistered\n");
}

module_init(smscore_module_init);
module_exit(smscore_module_exit);

#define MODULE_VERSION_STRING "Siano MDTV module "VERSION_STRING

MODULE_DESCRIPTION(MODULE_VERSION_STRING);
MODULE_AUTHOR(MODULE_AUTHOR_STRING);
MODULE_LICENSE("GPL");
