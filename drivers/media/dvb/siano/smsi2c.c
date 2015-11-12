/*
 *  smsi2c.c - Siano I2C interface driver
 *
 *  Copyright 2011 Siano Mobile Silicon, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 *
 */

#include <linux/moduleparam.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/slab.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include "smscoreapi.h"
#include "sms-cards.h"

/************************************************************/
/*Platform specific defaults - can be changes by parameters */
/*or in compilation at this section                         */
/************************************************************/
/*Host GPIO pin used for SMS interrupt*/
#define HOST_INTERRUPT_PIN 	(PAD_GPIO_C + 10)	// by kook - for lynx

/*Host GPIO pin used to reset SMS device*/
#define HOST_SMS_RESET_PIN	(PAD_GPIO_A + 23)	// by kook - for lynx

/*Host SPI bus number used for SMS*/
#define HOST_I2C_CTRL_NUM	0

/*SMS device I2C address*/
#define SMSI2C_CLIENT_ADDR		0x68

/*Default SMS device type connected to SPI bus.*/
#define DEFAULT_SMS_DEVICE_TYPE		 SMS_SANTOS

/*************************************/
/*End of platform specific parameters*/ 
/*************************************/


static int host_i2c_intr_pin = HOST_INTERRUPT_PIN;
static int host_i2c_ctrl = HOST_I2C_CTRL_NUM;
static int sms_i2c_addr = SMSI2C_CLIENT_ADDR;
static int i2c_default_type = DEFAULT_SMS_DEVICE_TYPE;

module_param(host_i2c_intr_pin, int, S_IRUGO);
MODULE_PARM_DESC(host_i2c_intr_pin, "interrupt pin number used by Host to be interrupted by SMS.");

module_param(host_i2c_ctrl, int, S_IRUGO);
MODULE_PARM_DESC(host_i2c_ctrl, "Number of I2C Controllers used for SMS.");

module_param(sms_i2c_addr, int, S_IRUGO);
MODULE_PARM_DESC(sms_i2c_addr, "I2C Address of SMS device.");

module_param(i2c_default_type, int, S_IRUGO);
MODULE_PARM_DESC(i2c_default_type, "default SMS device type for I2C bus.");


/* Registers */

struct smsi2c_device {

	struct i2c_adapter *adap;
	struct i2c_client *client;

	void *coredev;
	
	struct completion version_ex_done;
};

struct smsi2c_device *g_smsi2c_device;

static void smsi2c_worker_thread(void *arg);
static DECLARE_WORK(smsi2c_work_queue, (void *)smsi2c_worker_thread);

/*******************************************************************/
/* Siano core callbacks                                            */
/*******************************************************************/

static int smsi2c_sendrequest(void *context, void *buffer, size_t size)
{
	int ret;
	struct smsi2c_device *smsdev = (struct smsi2c_device *)context;
	
	if (!smsdev)
	{
		sms_err("smsi2c_sendrequest smsdev NULL!!\n");
		return -ENODEV;
	}
		
	ret = i2c_master_send(smsdev->client, buffer, (int)size);
	sms_debug("i2c_master_send returned %d", ret);
	return ret;
}


#define MAX_CHUNK_SIZE  (30*1024)

static int smsi2c_loadfirmware_handler(void *context, void* p_data, u32 fw_size)
{
	int ret;
	u8* fw_buf = (u8*)p_data;
	
	struct SmsMsgHdr_S BackdoorMsg = {
		MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ, 0, HIF_TASK,
			sizeof(struct SmsMsgHdr_S), 0};
	
	struct SmsTsEnable_S TsEnableMsg = {{MSG_SMS_ENBALE_TS_INTERFACE_REQ, 
					0, 
					11,
					sizeof(struct SmsTsEnable_S),
					0},
					40000000,	//15000000,						// by kook
					16,	//TSI_SERIAL_ON_SDIO,					// by kook
					TSI_SIGNALS_ACTIVE_LOW,
					0,
					TSI_SIG_OUT_FALL_EDGE,
					TSI_BIT0_IS_MSB,
					0,
					TSI_TRANSPARENT,	//TSI_ENCAPSULATED,		// by kook
					21};
	
	u32 sizeToCalc;
	u8* ptr;
    u32 checkSum;
    u32 i;
	
	u32* pFirstHeader = (u32*)fw_buf;
    	u32 SecondHeader[3];
	
	if (fw_size > 60*1024)
	{
		/* Send last part */
		SecondHeader[0] = 0xffffffff;        /* Impossible checksum */
		SecondHeader[1] = pFirstHeader[1] - 2*MAX_CHUNK_SIZE; /* Length of second chunk */
		SecondHeader[2] = pFirstHeader[2] + 2*MAX_CHUNK_SIZE; /* Address of second chunk */
		
		ret = smsi2c_sendrequest(context, &BackdoorMsg, sizeof(BackdoorMsg));
		msleep(50);

		ret = smsi2c_sendrequest(context, (u8*)(SecondHeader), 12);
		msleep(50);
		
		ret = smsi2c_sendrequest(context, 
						(u8*)(fw_buf+12+2*MAX_CHUNK_SIZE), 
						pFirstHeader[1] - 2*MAX_CHUNK_SIZE);
		msleep(50);
		
		/* Send middle part */
		SecondHeader[0] = 0xffffffff;             
		SecondHeader[1] = MAX_CHUNK_SIZE;         
		SecondHeader[2] = pFirstHeader[2] + MAX_CHUNK_SIZE;         
		
		ret = smsi2c_sendrequest(context, &BackdoorMsg, sizeof(BackdoorMsg));
		msleep(50);

		ret = smsi2c_sendrequest(context, (u8*)(SecondHeader), 12);
		msleep(50);
		
		ret = smsi2c_sendrequest(context, 
							(u8*)(fw_buf+12+MAX_CHUNK_SIZE), 
							MAX_CHUNK_SIZE);
		msleep(50);
		
		/* Send first part
		   Fix checksum */
		pFirstHeader[1] = MAX_CHUNK_SIZE;
		sizeToCalc = pFirstHeader[1] + 8;
		ptr = fw_buf + 4;

		checkSum = 0;

		for (i = 0; i < sizeToCalc; i++)
		{
			  checkSum ^= *(ptr + i);
		}

		pFirstHeader[0] = checkSum;                     

		ret = smsi2c_sendrequest(context, &BackdoorMsg, sizeof(BackdoorMsg));
		msleep(50);

		ret = smsi2c_sendrequest(context, (u8*)(pFirstHeader), 12);
		msleep(50);
		
		ret = smsi2c_sendrequest(context, 
								(u8*)(fw_buf+12),
								MAX_CHUNK_SIZE);
	}
	else
	{
		ret = smsi2c_sendrequest(context, (u8*)(fw_buf), 12);
		msleep(20);
			
		ret = smsi2c_sendrequest(context, (u8*)(fw_buf+12), fw_size-12);
	}
	
	msleep(20);
						
	ret = smsi2c_sendrequest(context, &TsEnableMsg, sizeof(TsEnableMsg));
	
	return 0;
}


/*******************************************************************/
/* i2c callbacks                                                  */
/*******************************************************************/

static void smsi2c_interrupt(void *context)
{
	
	struct smsi2c_device *smsdev = (struct smsi2c_device *)context;
	sms_debug("Recieved interrupt from SMS.\n");
	if (!smsdev)
		return;
	
	schedule_work(&smsi2c_work_queue);
}

static void smsi2c_worker_thread(void *args) 
{
	struct smscore_buffer_t *cb;
	struct SmsMsgHdr_S *phdr;
	u16 len;
	int ret;

	sms_debug("Worker thread is running.\n");
	cb = smscore_getbuffer(g_smsi2c_device->coredev);
	if (!cb) {
		sms_err("Unable to allocate data buffer!\n");
		goto exit;
	}
	
	phdr = (struct SmsMsgHdr_S *)cb->p;
	sms_debug("Recieve the message header.....\n");
	memset(cb->p, 0, (int)sizeof(struct SmsMsgHdr_S));
	sms_debug("buf before: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
		((u8*)phdr)[0], ((u8*)phdr)[1], ((u8*)phdr)[2], ((u8*)phdr)[3], 
		((u8*)phdr)[4], ((u8*)phdr)[5], ((u8*)phdr)[6], ((u8*)phdr)[7]);
	ret = i2c_master_recv(g_smsi2c_device->client, 
							cb->p, 
							(int)sizeof(struct SmsMsgHdr_S));
	if (ret < 0) {
		sms_err("Unable to read sms header! ret=%d\n", ret);
		goto exit;
	}

	sms_debug("hdr: type=%d, src=%d, dst=%d, len=%d, flag=0x%x\n", 
		phdr->msgType, phdr->msgSrcId, phdr->msgDstId, phdr->msgLength, phdr->msgFlags);
	sms_debug("buf: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
		((u8*)phdr)[0], ((u8*)phdr)[1], ((u8*)phdr)[2], ((u8*)phdr)[3], 
		((u8*)phdr)[4], ((u8*)phdr)[5], ((u8*)phdr)[6], ((u8*)phdr)[7]);
	sms_debug("Recieve the rest of the message.....\n");
	len = phdr->msgLength;
	
	if (len > sizeof(struct SmsMsgHdr_S))
	{
		ret = i2c_master_recv(g_smsi2c_device->client, 
								(u8*)(phdr+1), 
								len - (int)sizeof(struct SmsMsgHdr_S));
		sms_debug("recv of data returned %d", ret);
		if (ret < 0) {
			sms_err("Unable to read sms payload!\n");
			goto exit;
		}
	}
	
	switch (phdr->msgType)
	{
		case MSG_SMS_GET_VERSION_EX_RES: 
		{
			struct SmsVersionRes_S *ver =
					(struct SmsVersionRes_S *) phdr;
			sms_debug("MSG_SMS_GET_VERSION_EX_RES "
					"id %d prots 0x%x ver %d.%d",
					ver->xVersion.FirmwareId,
					ver->xVersion.SupportedProtocols,
					ver->xVersion.RomVer.Major,
					ver->xVersion.RomVer.Minor);
#if 0			
			smscore_set_device_mode(g_smsi2c_device->coredev, 
					ver->xVersion.FirmwareId == 255 ?
					SMSHOSTLIB_DEVMD_NONE : ver->xVersion.FirmwareId);
#endif					
			complete(&g_smsi2c_device->version_ex_done);
			break;
		}
	}
	sms_debug("Message recieved. Sending to callback.....\n");	
	cb->offset = 0;
	cb->size = len;
	smscore_onresponse(g_smsi2c_device->coredev, cb);
	
exit:
	return;
}


// allocate and init i2c dev descriptor
// update i2c client params
// 
static int smsi2c_probe(void)
{
	int ret;

	struct smsi2c_device *smsdev;
	struct smsdevice_params_t params;

	    struct i2c_board_info smsi2c_info = {
		I2C_BOARD_INFO("smsi2c", sms_i2c_addr),
	    };
	
	smsdev = kzalloc(sizeof(struct smsi2c_device), GFP_KERNEL);
	if (!smsdev)
	{
		sms_err("Cannot allocate memory for I2C device driver.\n");
		return -ENOMEM;
	}
		
	g_smsi2c_device = smsdev;
	sms_debug ("Memory allocated");
	smsdev->adap = i2c_get_adapter(host_i2c_ctrl);
	if (!smsdev->adap) {
		sms_err("Cannot get adapter #%d.\n", host_i2c_ctrl);
		ret = -ENODEV;
		goto failed_allocate_adapter;
	}
	sms_debug ("Got the adapter");

	smsi2c_info.platform_data = smsdev;

	smsdev->client = i2c_new_device(smsdev->adap, &smsi2c_info);

	if (!smsdev->client) {
		sms_err("Cannot register I2C device with addr 0x%x.\n", sms_i2c_addr);
		 ret = -ENODEV;
		 goto failed_allocate_device;
	}
	sms_debug ("Got the device");

//-> kook
	nxp_soc_gpio_set_io_func(HOST_INTERRUPT_PIN, nxp_soc_gpio_get_altnum(HOST_INTERRUPT_PIN));
	nxp_soc_gpio_set_io_dir(HOST_INTERRUPT_PIN, 0);       // input mode
	nxp_soc_gpio_set_int_enable(HOST_INTERRUPT_PIN, 0);

	nxp_soc_gpio_set_io_func(HOST_SMS_RESET_PIN, nxp_soc_gpio_get_altnum(HOST_SMS_RESET_PIN));
	nxp_soc_gpio_set_io_dir(HOST_SMS_RESET_PIN, 1);       // output mode

	nxp_soc_gpio_set_out_value(HOST_SMS_RESET_PIN, 0);
	msleep(50);
	nxp_soc_gpio_set_out_value(HOST_SMS_RESET_PIN, 1);
	msleep(10);
//<- kook

	ret = gpio_request(host_i2c_intr_pin, "sms_gpio");
	if (ret) {
		sms_err("failed to get sms_gpio\n");
		 goto failed_allocate_gpio;
	}	
	ret = gpio_direction_input(host_i2c_intr_pin);
	if (ret) {
		sms_err("failed to gpio_direction_input\n");
		 goto failed_allocate_gpio;
	}	
	gpio_export(host_i2c_intr_pin, 0);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
	ret = irq_set_irq_type(gpio_to_irq(host_i2c_intr_pin), IRQ_TYPE_EDGE_FALLING);	
	if (ret) {
		sms_err("failed to irq_set_irq_type, %d\n", ret);
		 goto failed_allocate_gpio;
	}
#else
	ret = set_irq_type(gpio_to_irq(host_i2c_intr_pin), IRQ_TYPE_EDGE_FALLING);
	if (ret) {
		sms_err("failed to set_irq_type, %d\n", ret);
		 goto failed_allocate_gpio;
	}
#endif
	/*register irq*/
#if 0
// org
	ret = request_irq( gpio_to_irq(host_i2c_intr_pin), (irq_handler_t)smsi2c_interrupt,
		 IRQF_TRIGGER_RISING, "SMSI2C", smsdev);
#else
// kook
	ret = request_irq( gpio_to_irq(host_i2c_intr_pin), (irq_handler_t)smsi2c_interrupt,
		 IRQ_TYPE_EDGE_RISING, "SMSI2C", smsdev);
#endif
	if (ret < 0) {
		sms_err("failed to allocate interrupt for SMS\n");
		 goto failed_allocate_interrupt;
	}	

//-> kook
	nxp_soc_gpio_set_int_enable(HOST_INTERRUPT_PIN, 1);
	nxp_soc_gpio_clr_int_pend(HOST_INTERRUPT_PIN);
//<- kook

	memset(&params, 0, sizeof(struct smsdevice_params_t));

	params.device = (struct device *)smsdev->client;
	params.buffer_size = 0x400;	
	params.num_buffers = 20;	
	params.context = smsdev;

	snprintf(params.devpath, sizeof(params.devpath),
		 "i2c\\%s", "smsi2c");

	params.sendrequest_handler  = smsi2c_sendrequest;
	params.loadfirmware_handler = smsi2c_loadfirmware_handler;
	params.device_type = i2c_default_type;

	/* Use SMS_DEVICE_FAMILY2 for firmware download over SMS MSGs
	   SMS_DEVICE_FAMILY1 for backdoor I2C firmware download */
	/* params.flags |= SMS_DEVICE_FAMILY2; */
	
	/* Device protocol completion events */
	init_completion(&smsdev->version_ex_done);

	ret = smscore_register_device(&params, &smsdev->coredev);
	if (ret < 0)
        {
	        printk(KERN_INFO "smscore_register_device error\n");
		goto failed_registering_coredev;
        }

	ret = smscore_start_device(smsdev->coredev);
	if (ret < 0)
        {
		printk(KERN_INFO "smscore_start_device error\n");
		goto failed_device_start;
        }

	return 0;
failed_device_start:
	smscore_unregister_device(smsdev->coredev);
failed_registering_coredev:
	free_irq(gpio_to_irq(host_i2c_intr_pin), smsdev);
failed_allocate_interrupt:
	gpio_free(host_i2c_intr_pin);
failed_allocate_gpio:
	i2c_unregister_device(smsdev->client);
failed_allocate_device:
	i2c_put_adapter(smsdev->adap);
failed_allocate_adapter:
	g_smsi2c_device = NULL;
	kfree(smsdev);

	return ret;
}



/*******************************************************************/
/* Module functions                                                */
/*******************************************************************/

int smsi2c_register(void)
{
	int ret = 0;
	
	printk(KERN_INFO "smsi2c: Siano SMS1xxx I2c driver\n");
	
	ret = smsi2c_probe();
	
	return ret;
}

void smsi2c_unregister(void)
{
	//need to save smsdev and check for null
	smscore_unregister_device(g_smsi2c_device->coredev);
	i2c_unregister_device(g_smsi2c_device->client);
	i2c_put_adapter(g_smsi2c_device->adap);

	free_irq(gpio_to_irq(host_i2c_intr_pin), g_smsi2c_device);
	gpio_free(host_i2c_intr_pin);
	gpio_free(HOST_SMS_RESET_PIN);

	kfree(g_smsi2c_device);
	g_smsi2c_device = NULL;	
}


