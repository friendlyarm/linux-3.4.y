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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/types.h>	/* size_t */
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/system.h>		/* cli(), *_flags */
#include <linux/uaccess.h>	/* copy_*_user */
#include <linux/slab.h>
#include <linux/gpio.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include "smscoreapi.h"
#include "smscharioctl.h"

/* max number of packets allowed to be pending on queue*/
#define SMS_CHR_MAX_Q_LEN	15
#define SMSCHAR_NR_NODES	17
#define SMSCHAR_NR_SOCKETS	4

struct smschar_device_t {
	struct cdev cdev;	/*!< Char device structure */
	wait_queue_head_t waitq;	/* Processes waiting */
	int cancel_waitq;
	spinlock_t lock;	/*!< critical section */
	int pending_count;
	struct list_head pending_data;	/*!< list of pending data */
	struct smscore_buffer_t *currentcb;
	int device_index;
	int socket_num;
	void *coredev;
	struct smscore_client_t *smsclient;
	struct class *mdtvNode;

};


/*Host GPIO pin used to reset SMS device*/
#define HOST_SMS_RESET_PIN	(PAD_GPIO_A + 23)


/*!  Holds the major number of the device node. may be changed at load
time.*/
int smschar_major = 0;

/*!  Holds the first minor number of the device node.
may be changed at load time.*/
int smschar_minor;  /*= 0*/

int host_sms_reset_pin = HOST_SMS_RESET_PIN;

/* macros that allow the load time parameters change*/
module_param(smschar_minor, int, S_IRUGO);
MODULE_PARM_DESC(smschar_minor, "Set minor number for first char device. default is 0");

module_param(smschar_major, int, S_IRUGO);
MODULE_PARM_DESC(smschar_major, "Set major number for char device. 0(default) for automatic fetching from kernel.");

module_param(host_sms_reset_pin, int, S_IRUGO);
MODULE_PARM_DESC(host_sms_reset_pin, "GPIO pin number used by Host to reset SMS device.");


static enum smschar_pnp_event_t g_smschar_pnp_event = 0;

static int g_pnp_status_changed = 0;
wait_queue_head_t g_pnp_event;


struct smschar_socket_t {
	void *coredev;
	struct class *mdtvNode[SMSCHAR_NR_NODES];
	struct smschar_device_t char_dev[SMSCHAR_NR_NODES];
};


static struct smschar_socket_t g_smschar_sockets[SMSCHAR_NR_SOCKETS];


/**
 * gets firmware filename from look-up table in case 
 * "request_firmware" is not supported by kernel.
 * 
 * @param dev pointer to smschar parameters block
 * @param up pointer to a struct that contains the requested 
 *           mode of operation and a pointer to the filename 
 *           in user space
 *
 * @return 0 on success, <0 on error.
 */
int smschar_get_fw_filename(struct smschar_device_t *dev,
		struct smschar_get_fw_filename_ioctl_t *up) {
	int rc = 0;
	char tmpname[200];

#ifndef REQUEST_FIRMWARE_SUPPORTED
	int mode = up->mode;
	char *fw_filename = smscore_get_fw_filename(dev->coredev, mode,0);
	sprintf(tmpname, "%s/%s", DEFAULT_FW_FILE_PATH, fw_filename);
	sms_debug("need to send fw file %s, mode %d", tmpname, mode);
#else
	/* driver not need file system services */
	tmpname[0] = '\0';
	sms_debug("don't need to send fw file, request firmware supported");
#endif
	if (copy_to_user(up->filename, tmpname, strlen(tmpname) + 1)) {
		sms_err("Failed copy file path to user buffer");
		return -EFAULT;
	}
	return rc;
}

/**
 * copies a firmware buffer from user to kernel space and 
 * keeps a pointer and size on core device.
 *
 * NOTE: this mode type is encouraged and should be used 
 *       only when "request_firmware" is not supported by kernel.
 *
 * @param dev pointer to smschar parameters block
 * @param up pointer to a struct that contains the requested 
 *           pointer to user space fw buffer and it size in bytes
 *
 * @return 0 on success, <0 on error.
 */
int smschar_send_fw_file(struct smschar_device_t *dev,
		struct smschar_send_fw_file_ioctl_t *up) {
	int rc = 0;

	sms_debug("fw buffer = 0x%p, size = 0x%x", up->fw_buf, up->fw_size);
	/* free old buffer */
	if (sms_static_firmware != NULL) {
		kfree(sms_static_firmware);
		sms_static_firmware = NULL;
	}

	sms_static_firmware = kmalloc(ALIGN(up->fw_size + sizeof(sms_static_firmware), SMS_ALLOC_ALIGNMENT), GFP_KERNEL
			| GFP_DMA);
	if (!sms_static_firmware) {
		sms_err("failed to allocate memory for fw buffer");
		return -ENOMEM;
	}

	if (copy_from_user(sms_static_firmware->data, up->fw_buf, up->fw_size)) {
		sms_err("failed to copy fw from user buffer");
		kfree(sms_static_firmware);
	        sms_static_firmware = NULL;
		return -EFAULT;
	}
	sms_static_firmware->fw_buf_size = up->fw_size;

	return rc;
}

int smschar_send_fw_chunk(struct smschar_device_t *dev,
		struct smschar_send_fw_chunk_ioctl_t *up) 
{
	struct smscore_device_t *coredev = dev->coredev;

	return smscore_send_fw_chunk(coredev, up->buf, up->size);
}

int smschar_send_last_fw_chunk(struct smschar_device_t *dev,
		struct smschar_send_fw_chunk_ioctl_t *up) 
{
	struct smscore_device_t *coredev = dev->coredev;

	return smscore_send_last_fw_chunk(coredev, up->buf, up->size);
}

/**
 * unregisters sms client and returns all queued buffers
 *
 * @param dev pointer to the client context (smschar parameters block)
 *
 */
static void smschar_unregister_client(struct smschar_device_t *dev)
{
	unsigned long flags;

	dev->cancel_waitq = 1;

	spin_lock_irqsave(&dev->lock, flags);

	if (dev->coredev)
	{
		while (!list_empty(&dev->pending_data)) {
			struct smscore_buffer_t *cb =
				(struct smscore_buffer_t *)dev->pending_data.next;
			list_del(&cb->entry);

			smscore_putbuffer(dev->coredev, cb);
			dev->pending_count--;
		}
	}
	
	if (dev->currentcb) {
		smscore_putbuffer(dev->coredev, dev->currentcb);
		dev->currentcb = NULL;
		dev->pending_count--;
	}

	wake_up_interruptible(&dev->waitq);

	if (dev->smsclient)
		smscore_unregister_client(dev->smsclient);
		
	dev->smsclient = NULL;
	dev->coredev = NULL;

	spin_unlock_irqrestore(&dev->lock, flags);
}

/**
 * queues incoming buffers into buffers queue
 *
 * @param context pointer to the client context (smschar parameters block)
 * @param cb pointer to incoming buffer descriptor
 *
 * @return 0 on success, <0 on queue overflow.
 */
static int smschar_onresponse(void *context, struct smscore_buffer_t *cb)
{
	struct smschar_device_t *dev = context;
	unsigned long flags;

	if (!dev) {
		sms_err("recieved bad dev pointer");
		return -EFAULT;
	}
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->pending_count > SMS_CHR_MAX_Q_LEN) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return -EBUSY;
	}

	dev->pending_count++;
	/* if data channel, remove header */
	if (dev->device_index) {
		cb->size -= sizeof(struct SmsMsgHdr_S);
		cb->offset += sizeof(struct SmsMsgHdr_S);
	}

	list_add_tail(&cb->entry, &dev->pending_data);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (waitqueue_active(&dev->waitq))
		wake_up_interruptible(&dev->waitq);

	return 0;
}

/**
 * handles device removal event
 *
 * @param context pointer to the client context (smschar parameters block)
 *
 */
static void smschar_onremove(void *context)
{
	struct smschar_device_t *dev = (struct smschar_device_t *)context;
	smschar_unregister_client(dev);
	sms_info ("unregistering the client");	

}

/**
 * registers client associated with the node
 *
 * @param inode Inode concerned.
 * @param file File concerned.
 *
 * @return 0 on success, <0 on error.
 */
static int smschar_open(struct inode *inode, struct file *file)
{
	struct smschar_device_t *dev = container_of(inode->i_cdev,
						    struct smschar_device_t,
						    cdev);
	if (!dev->coredev)
	{
		sms_info("no device");
		return -ENODEV;
	}
	file->private_data = dev;
	g_pnp_status_changed = 0;
	dev->cancel_waitq = 0;

        return 0;
}

/**
 * unregisters client associated with the node
 *
 * @param inode Inode concerned.
 * @param file File concerned.
 *
 */
static int smschar_release(struct inode *inode, struct file *file)
{
	struct smschar_device_t *dev = file->private_data;
	sms_info("smschar_release dev 0x%p node=%d", dev, dev->device_index);
	file->private_data = NULL;
	return 0;
}

/**
 * copies data from buffers in incoming queue into a user buffer
 *
 * @param file File structure.
 * @param buf Source buffer.
 * @param count Size of source buffer.
 * @param f_pos Position in file (ignored).
 *
 * @return Number of bytes read, or <0 on error.
 */
static ssize_t smschar_read(struct file *file, char __user *buf,
			    size_t count, loff_t *f_pos)
{
	struct smschar_device_t *dev = file->private_data;
	unsigned long flags;
	int rc, copied = 0;

	if (!buf) {
		sms_err("bad pointer recieved from user");
		return -EFAULT;
	}
	if (!dev->coredev || !dev->smsclient) {
		sms_err("no client\n");
		return -ENODEV;
	}
	rc = wait_event_interruptible(dev->waitq,
				      !list_empty(&dev->pending_data)
				      || (dev->cancel_waitq));
	if (rc < 0) {
		sms_err("wait_event_interruptible error %d\n", rc);
		return rc;
	}
	if (dev->cancel_waitq)
		return 0;
	if (!dev->smsclient) {
		sms_err("no client\n");
		return -ENODEV;
	}
	spin_lock_irqsave(&dev->lock, flags);

	while (!list_empty(&dev->pending_data) && (copied < count)) {
		struct smscore_buffer_t *cb =
		    (struct smscore_buffer_t *)dev->pending_data.next;
		int actual_size = min(((int)count - copied), cb->size);
		if (copy_to_user(&buf[copied], &((char *)cb->p)[cb->offset],
				 actual_size)) {
			sms_err("copy_to_user failed\n");
			spin_unlock_irqrestore(&dev->lock, flags);
			return -EFAULT;
		}
		copied += actual_size;
		cb->offset += actual_size;
		cb->size -= actual_size;

		if (!cb->size) {
			list_del(&cb->entry);
			smscore_putbuffer(dev->coredev, cb);
			dev->pending_count--;
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return copied;
}

/**
 * sends the buffer to the associated device
 *
 * @param file File structure.
 * @param buf Source buffer.
 * @param count Size of source buffer.
 * @param f_pos Position in file (ignored).
 *
 * @return Number of bytes read, or <0 on error.
 */
static ssize_t smschar_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *f_pos)
{
	struct smschar_device_t *dev;
	void *buffer;

	if (file == NULL) {
		sms_err("file is NULL\n");
		return EINVAL;
	}

	if (file->private_data == NULL) {
		sms_err("file->private_data is NULL\n");
		return -EINVAL;
	}

	dev = file->private_data;
	if (!dev->coredev || !dev->smsclient) {
		sms_err("no client\n");
		return -ENODEV;
	}

	buffer = kmalloc(ALIGN(count, SMS_ALLOC_ALIGNMENT) + SMS_DMA_ALIGNMENT,
			 GFP_KERNEL | GFP_DMA);
	if (buffer) {
		void *msg_buffer = (void *)SMS_ALIGN_ADDRESS(buffer);

		if (!copy_from_user(msg_buffer, buf, count))
			smsclient_sendrequest(dev->smsclient,
					      msg_buffer, count);
		else
			count = 0;

		kfree(buffer);
	}

	return count;
}

static int smschar_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct smschar_device_t *dev = file->private_data;	
	if (!dev->coredev || !dev->smsclient) {
		sms_err("no client\n");
		return -ENODEV;
	}
	return smscore_map_common_buffer(dev->coredev, vma);
}

/**
 * waits until buffer inserted into a queue. when inserted buffer offset
 * are reportedto the calling process. previously reported buffer is
 * returned to smscore pool.
 *
 * @param dev pointer to smschar parameters block
 * @param touser pointer to a structure that receives incoming buffer offsets
 *
 * @return 0 on success, <0 on error.
 */
static int smschar_wait_get_buffer(struct smschar_device_t *dev,
				   struct smschar_buffer_t *touser)
{
	unsigned long flags;
	int rc;

	spin_lock_irqsave(&dev->lock, flags);

	if (dev->currentcb) {
		smscore_putbuffer(dev->coredev, dev->currentcb);
		dev->currentcb = NULL;
		dev->pending_count--;
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	rc = wait_event_interruptible(dev->waitq,
				      !list_empty(&dev->pending_data)
				      || (dev->cancel_waitq));
	if (rc < 0) {
		sms_err("wait_event_interruptible error, rc = %d", rc);
		return rc;
	}

	if (!dev->smsclient || dev->cancel_waitq) {
		sms_err("no client\n");
		return -ENODEV;
	}

	spin_lock_irqsave(&dev->lock, flags);

	if (!list_empty(&dev->pending_data)) {
		struct smscore_buffer_t *cb =
		    (struct smscore_buffer_t *)dev->pending_data.next;
		touser->offset = cb->offset_in_common + cb->offset;
		touser->size = cb->size;

		list_del(&cb->entry);

		dev->currentcb = cb;
	} else {
		touser->offset = 0;
		touser->size = 0;
	}


	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/**
 * poll for data availability
 *
 * @param file File structure.
 * @param wait kernel polling table.
 *
 * @return (POLLIN | POLLRDNORM) flags if read data is available.
 *          POLLNVAL flag if wait_queue was cancelled.
 *	    <0 on error.
 */
static unsigned int smschar_poll(struct file *file,
				 struct poll_table_struct *wait)
{
	struct smschar_device_t *dev;
	int events = 0;

	if (file == NULL) {
		sms_err("file is NULL");
		return EINVAL;
	}

	dev = file->private_data;
	if (dev == NULL) {
		sms_err("dev is NULL");
		return -EINVAL;
	}

	if (!dev->coredev || !dev->smsclient) {
		sms_debug("no client\n");
		return -ENODEV;
	}
	if (dev->cancel_waitq) {
		/*sms_debug("returning POLLNVAL");*/
		events |= POLLNVAL;
		return events;
	}

	/*
 	 * make the system call to wait to wait_queue wakeup if there is
	 * no data
	 * cancel_waitq is checked again to prevenet reace condition (wait
	 * to cancalled wait_queue)
	 */
	if (list_empty(&dev->pending_data) && (!dev->cancel_waitq)) {
		poll_wait(file, &dev->waitq, wait);
	}

	/*
	 * pending data, raise relevant flags
	 */
	if (!list_empty(&dev->pending_data)) {
		events |= (POLLIN | POLLRDNORM);
	}
	return events;
}

static long smschar_ioctl(struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	struct smschar_device_t *dev = file->private_data;
	void __user *up = (void __user *)arg;

	switch (cmd) {
	case SMSCHAR_IS_DEVICE_PNP_EVENT:
		{
			sms_info("Waiting for PnP event.\n");
			wait_event_interruptible(g_pnp_event,
						 g_pnp_status_changed);
			sms_info("PnP event: g_pnp_status_changed=%d\n", g_pnp_status_changed);
			g_pnp_status_changed = 0;
			sms_info("PnP Event %d.\n", g_smschar_pnp_event);
			if (put_user(g_smschar_pnp_event, (int *)up))
			{
				sms_info("put_user returned error status\n");
				return -EFAULT;
			}
			g_smschar_pnp_event = SMSCHAR_UNKNOWN_EVENT;
			return 0;
		}
	case SMSCHAR_IS_CANCEL_DEVICE_PNP_EVENT:
		{
			g_smschar_pnp_event = SMSCHAR_TERMINATE_EVENT;
			g_pnp_status_changed = 1;
			wake_up_interruptible(&g_pnp_event);
			return 0;
		}
	default:
		break;
	}
	
	if (!dev->coredev || !dev->smsclient) {
		sms_debug("no client\n");
		return -ENODEV;
	}

	switch (cmd) {
	case SMSCHAR_SET_DEVICE_MODE:
		return smscore_set_device_mode(dev->coredev, (int)arg);

	case SMSCHAR_GET_DEVICE_MODE:
		{
			struct sms_properties_t properties;
			smscore_get_device_properties(dev->coredev, &properties);
			if (put_user(properties.mode,
				     (int *)up))
				return -EFAULT;
			break;
		}
	case SMSCHAR_GET_BUFFER_SIZE:
		{
			struct sms_properties_t properties;
			smscore_get_device_properties(dev->coredev, &properties);
			if (put_user
			    (smscore_get_common_buffer_size(dev->coredev),
			     (int *)up))
				return -EFAULT;

			break;
		}

	case SMSCHAR_WAIT_GET_BUFFER:
		{
			struct smschar_buffer_t touser;
			int rc;

			rc = smschar_wait_get_buffer(dev, &touser);
			if (rc < 0)
				return rc;

			if (copy_to_user(up, &touser,
					 sizeof(struct smschar_buffer_t)))
				return -EFAULT;

			break;
		}
	case SMSCHAR_CANCEL_WAIT_BUFFER:
		{
			dev->cancel_waitq = 1;
			wake_up_interruptible(&dev->waitq);
			break;
		}
	case SMSCHAR_CANCEL_POLL:
		{
			/*obsollete*/
			break;		
		}
	case SMSCHAR_GET_FW_FILE_NAME:
		{
			if (!up)
				return -EINVAL;
			return smschar_get_fw_filename(dev,
				       (struct smschar_get_fw_filename_ioctl_t*)up);
		}
	case SMSCHAR_SEND_FW_FILE:
		{
			if (!up)
				return -EINVAL;
			return smschar_send_fw_file(dev,
					(struct smschar_send_fw_file_ioctl_t*)up);
		}
	case SMSCHAR_SEND_FW_CHUNK:
		{
			if (!up)
				return -EINVAL;
			return smschar_send_fw_chunk(dev,
					(struct smschar_send_fw_chunk_ioctl_t*)up);
			break;
		}
	case SMSCHAR_SEND_LAST_FW_CHUNK:
		{
			if (!up)
				return -EINVAL;
			return smschar_send_last_fw_chunk(dev,
					(struct smschar_send_fw_chunk_ioctl_t*)up);
			break;
		}
	case SMSCHAR_RESET_GPIO_VALUE_CHANGE:
		{
			sms_info("SMSCHAR_RESET_GPIO_VALUE_CHANGE.\n");
			if (!up)
				return -EINVAL;
			if (host_sms_reset_pin == 0xFFFFFFFF)
			{	
				sms_err ("Reset pin was not assigned properly");
				return -EINVAL;
			}
			if ( *(int*)arg != 0 &&  *(int*)arg != 1)
			{	
				sms_err ("Bad value (%d)" , *(int*)arg);
				return -EINVAL;
			}
			gpio_set_value(host_sms_reset_pin, *(int*)arg);
		}
	default:
		return -ENOIOCTLCMD;
	}

	return 0;
}

/**
 * char device file operations
 */
struct file_operations smschar_fops = {
	.owner = THIS_MODULE,
	.read = smschar_read,
	.write = smschar_write,
	.open = smschar_open,
	.release = smschar_release,
	.mmap = smschar_mmap,
	.poll = smschar_poll,
	.unlocked_ioctl = smschar_ioctl,
};

static int smschar_setup_cdev(struct smschar_device_t *dev, int index, int socket)
{
	int rc, devno = MKDEV(smschar_major, smschar_minor + index + socket * SMSCHAR_NR_NODES);

	INIT_LIST_HEAD(&dev->pending_data);
	spin_lock_init(&dev->lock);
	init_waitqueue_head(&dev->waitq);

	sms_info ("setting up char device %d node %d on socket %d device 0x%p", devno, index, socket, &dev->cdev);
	cdev_init(&dev->cdev, &smschar_fops);

	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &smschar_fops;

	kobject_set_name(&dev->cdev.kobj, "Siano_sms%d", index+ socket * SMSCHAR_NR_NODES);
	rc = cdev_add(&dev->cdev, devno, 1);
	return rc;
}

static int smschar_power_mode_handler(enum sms_power_mode_st mode)
{
	switch (mode) {
	case SMS_POWER_MODE_ACTIVE:
		g_smschar_pnp_event = SMSCHAR_RESUME_EVENT;
		break;
	case SMS_POWER_MODE_SUSPENDED:
		g_smschar_pnp_event = SMSCHAR_SUSPEND_EVENT;
		break;
	default:
		return -EINVAL;
	}
	g_pnp_status_changed = 1;
	wake_up_interruptible(&g_pnp_event);
	
	return 0;
}

/**
 * smschar callback that called when device plugged in/out. the function
 * register or unregisters char device interface according to plug in/out
 *
 * @param coredev pointer to device that is being plugged in/out
 * @param device pointer to system device object
 * @param arrival 1 on plug-on, 0 othewise
 *
 * @return 0 on success, <0 on error.
 */
static int smschar_hotplug(void *coredev,
			   struct device *device, int arrival)
{
	int rc = 0, i, socket_num;
	struct smsclient_params_t params;
	bool device_exists = 0;


	sms_info("entering %d", arrival);

	/*Search char device list if device already exists*/
	socket_num = 0xffff;
	for (i = 0; i < SMSCHAR_NR_SOCKETS; i++) {
		if (g_smschar_sockets[i].coredev == coredev)
		{
			device_exists = 1;
			socket_num = i;
			break;
		}
		else if ((socket_num == 0xffff) && (g_smschar_sockets[i].coredev == NULL))
			socket_num = i;
	}

	if (arrival)
	{
		g_smschar_pnp_event = SMSCHAR_PLUG_IN_EVENT;
		if (device_exists)
		{
			sms_err ("error - hotplug notifies same device twice!");
			return -EINVAL;
		}
		if (socket_num == 0xffff)
		{
			sms_err ("error - all sockets are in use.");
			return -EINVAL;
		}
		g_smschar_sockets[socket_num].coredev = coredev;

		/* Create the char device itself and the udev nodes*/
		for (i = 0; i < SMSCHAR_NR_NODES; i++)
		{
			g_smschar_sockets[socket_num].char_dev[i].coredev = coredev;
			g_smschar_sockets[socket_num].char_dev[i].device_index = i;
			g_smschar_sockets[socket_num].char_dev[i].socket_num = socket_num;

			params.initial_id = i ? i: SMS_HOST_LIB;
			params.data_type = i ? MSG_SMS_DAB_CHANNEL : 0;
			params.onresponse_handler = smschar_onresponse;
			params.onremove_handler = smschar_onremove;
			params.context = &g_smschar_sockets[socket_num].char_dev[i];

			rc = smscore_register_client(coredev, &params,
					&g_smschar_sockets[socket_num].char_dev[i].smsclient);
		}
	}
	else
	{
		sms_info ("device is out");
		g_smschar_pnp_event = SMSCHAR_PLUG_OUT_EVENT;
		for (i = 0; i < SMSCHAR_NR_NODES; i++)
		{
			g_smschar_sockets[socket_num].char_dev[i].coredev = NULL;
		}
		if (!device_exists)
		{
			sms_err ("error - hotplug notifies plugout of unexisted device");
			return -EINVAL;
		}
		g_smschar_sockets[socket_num].coredev = NULL;
	}
	g_pnp_status_changed = 1;
	wake_up_interruptible(&g_pnp_event);
	sms_info("exiting, rc %d", rc);

	return rc;		/* succeed */
}

int smschar_register(void)
{
	int rc;
	int i, j, devno;
	char mdtv_name[12];


	sms_info("registering device major=%d minor=%d", smschar_major,
		 smschar_minor);

	memset (g_smschar_sockets, 0, sizeof(g_smschar_sockets));
	init_waitqueue_head(&g_pnp_event);
	
	devno = MKDEV(smschar_major, smschar_minor);
	smscore_register_power_mode_handler(smschar_power_mode_handler);
		
	rc = smscore_register_hotplug(smschar_hotplug);

	if (smschar_major) {
		rc = register_chrdev_region(devno, SMSCHAR_NR_NODES*SMSCHAR_NR_SOCKETS, "smschar");
	} else {
		rc = alloc_chrdev_region(&devno, smschar_minor,	SMSCHAR_NR_NODES * SMSCHAR_NR_SOCKETS, "smschar");
		smschar_major = MAJOR(devno);
	}

	/* socket 0*/
	for (i = 0; i < SMSCHAR_NR_NODES ; i++)
	{
		devno = MKDEV(smschar_major, smschar_minor + i);
		if (i==0)
			sprintf(mdtv_name, "mdtvctrl");
		else
			sprintf(mdtv_name, "mdtv%d", i);
		g_smschar_sockets[0].mdtvNode[i] = class_create(THIS_MODULE, mdtv_name);
		device_create(g_smschar_sockets[0].mdtvNode[i], NULL, devno, NULL, mdtv_name);
		smschar_setup_cdev(&g_smschar_sockets[0].char_dev[i], i, 0);
	}

	
	for (j = 1; j<SMSCHAR_NR_SOCKETS ; j++)
	{
		for (i = 0; i < SMSCHAR_NR_NODES ; i++)
		{
			devno = MKDEV(smschar_major, smschar_minor + i + j * SMSCHAR_NR_NODES);
			if (i==0)
				sprintf(mdtv_name, "mdtvctrl-%d", j);
			else
				sprintf(mdtv_name, "mdtv%d-%d", j, i);
			g_smschar_sockets[j].mdtvNode[i] = class_create(THIS_MODULE, mdtv_name);
			device_create(g_smschar_sockets[j].mdtvNode[i], NULL, devno, NULL, mdtv_name);
			smschar_setup_cdev(&g_smschar_sockets[j].char_dev[i], i, j);
	
		}
sms_info("created nodes for socket %d", j);
	}

	/* Reset pin will be used only if valid value exists.*/
	if (host_sms_reset_pin != 0xFFFFFFFF)
	{
		if (gpio_request(host_sms_reset_pin, "SMSSPI"))
		{
			sms_err("Could not get GPIO for SMS reset. Device will work but reset is not supported.\n");
			host_sms_reset_pin = 0xFFFFFFFF;
		}
		else
		{
			gpio_direction_output(host_sms_reset_pin, 1);
			gpio_export(host_sms_reset_pin, 1);
		}
	}


	return rc;
}

void smschar_unregister(void)
{
	int i, j, devno;
	if (host_sms_reset_pin != 0xFFFFFFFF)
		gpio_free(host_sms_reset_pin);

	for (j = 0; j<SMSCHAR_NR_SOCKETS ; j++)
	{
		for (i = 0; i < SMSCHAR_NR_NODES; i++)
		{
			devno = MKDEV(smschar_major, smschar_minor + i + j * SMSCHAR_NR_NODES);
			cdev_del(&g_smschar_sockets[j].char_dev[i].cdev);
			device_destroy(g_smschar_sockets[j].mdtvNode[i], devno);
			class_unregister(g_smschar_sockets[j].mdtvNode[i]);
			class_destroy(g_smschar_sockets[j].mdtvNode[i]);
			sms_info("destroyed device class 0x%p ", g_smschar_sockets[j].mdtvNode[i]);

		}
	}
	devno = MKDEV(smschar_major, smschar_minor);
	unregister_chrdev_region(devno, SMSCHAR_NR_NODES*SMSCHAR_NR_SOCKETS);
	smscore_unregister_hotplug(smschar_hotplug);
	smscore_un_register_power_mode_handler();
	sms_info("unregistered");
}
