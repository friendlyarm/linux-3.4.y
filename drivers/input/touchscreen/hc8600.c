/*
 * This is the HC5xx Touchscreen driver template
 *
 * Copyright (C) 2012 Shanghai Haier IC Corp., Ltd.
 * Author: Xiaq <xiaq@ichaier.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <mach/platform.h>

#define HC548_DRIVER_NAME   "HC548"
#define HC5XX_NAME	        "hc5xx_ts"

#define IRQ_PORT		PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT)//IRQ_EINT(8)

/* Version */
#define HC548_ID		0x86

/* Firmware */
#define HC548_FW_ID		

/* Button define */
//#define KEY_MAP_INIT

/* Enable upgrade */
#define ENABLE_UPGRADE

//#define ENABLE_AllConfigureInOne

//#define RST_PORT			( 8+('a'-'a')*32+7 )
//#define INT_PORT			( 8+('a'-'a')*32+6 )

/************************Registers Address Definitions ******************************/
//       register name                address      default data
#define HC548_PRODUCT_REV              0x00         // 0x65
#define HC548_HW_FW_REV                0x01         // 0x2F
#define HC548_BOOT_STAT                0x04        
#define HC548_ERROR_ID                 0x05         // 0x00
#define HC548_ORIENTATION              0x10         // 0x00
#define HC548_FW_UPGRADE               0x1d
#define HC548_DEBUG                    0x3d
#define HC548_DOWNLOAD_CTL             0x52         // N/A
#define HC548_DOWNLOAD_DATA            0x53         // N/A
#define HC548_TA_ENABLE                0x54         // 3C to enable
#define HC548_TA_ADDR0                 0x55         
#define HC548_TA_ADDR1                 0x56
#define HC548_TA_ADDR2                 0x57
#define HC548_TA_ADDR3                 0x58
#define HC548_CAL_CONTROL              0x5F          
#define HC548_STATUS                   0x7c         
#define HC548_INT_DEASSERT             0x7e         // 0x3F
#define HC548_INT_MASK                 0x7f         // 0x17
/* 20drive*12sense maxTPOINT=10 */
#define HC548_ROW_NUM                  0x11         // 0x0C
#define HC548_COL_NUM                  0x12         // 0x14
#define HC548_TOUCH_ID_COUNT           0x13         // 0x0B
#define HC548_TPOINTS                  0x14         // 0x0A
/* PIXEL_HEIGHT*PIXEL_WIDTH */
#define HC548_HEIGHT_HI                0x17         // 0x01
#define HC548_HEIGHT_LO                0x16         // 0x68
#define HC548_WIDTH_HI                 0x19         // 0x02
#define HC548_WIDTH_LO                 0x18         // 0x80

#define HC548_WD_DISABLE               0x02         //
#define HC548_FLASH_EN                 0x03         //
#define HC548_FLASH_CTL                0x67
#define HC548_MOTION_CTL               0x5C         //
#define HC548_MOTION_DAT               0x5D         //
#define HC548_SHUTDOWN                 0x7B         // 0x55 shutdown
                                                    // 0x99 hard reset
						    						// 0x77 patrial reset
						    						// 0x66 resume
/************************************************************************************/

/************************* Touchscreen absolute values ******************************/
#define HC548_MAX_X                   1024   //The LCD Max X resolution
#define HC548_MAX_Y                   600//600    //The LCD MAX Y resolution
#define I2C_BUS_NUM					  1//0
#define HC548_SLAVE_ADDR			  0x40  // I2C SLAVE Address

#define HC548_MAX_FINGER               10     //Max finger num
#define HC548_MAX_BUTTON               4      //Max button num
#define HC548_FRAME_SIZE               2+9*HC548_MAX_FINGER+1+3*HC548_MAX_BUTTON  //Touch point data length
/************************************************************************************/

//#define HC548_FRAME_ADDR               0x00
//#define HC548_FORCE_LIMIT              0xFF     //MAX FORCE VALUE: 0x0F0A
//#define HC548_MAX_AREA                 0xFF
#define HC548_RST_INIT                 0x55
#define HC548_RST_HARD                 0x99
#define HC548_RST_PARTIAL              0x77
#define HC548_RST_RESUME               0x66

#define HC548_RESET_TIME               65      /* msec */
#define HC548_FWRESET_TIME             175     /* msec */

/*    KEY_CONFIG    */ 
#ifdef KEY_MAP_INIT
	#define KEY_MENU	139//59
	#define KEY_HOME	102
	#define KEY_BACK	158
	#define KEY_SEARCH	217
	static unsigned int HC548_key_map[] = {KEY_MENU, KEY_HOME, KEY_BACK, KEY_SEARCH};
#endif

//#undef DEBUG
#define DEBUG   0
/*     debug infomation       */
#ifdef DEBUG
#define pr_debug_info(fmt, arg ...) \
	printk(KERN_ERR fmt, ##arg)
#define pr_red_info(fmt, arg ...) \
	printk(KERN_ERR "\033[31m" fmt "\033[0m\n", ##arg)
#define pr_green_info(fmt, arg ...) \
	printk(KERN_ERR "\033[32m" fmt "\033[0m\n", ##arg)
#define pr_pos_info() \
	printk(KERN_INFO "%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)
#else
#define pr_debug_info(fmt, arg ...)
#define pr_red_info(fmt, arg ...)
#define pr_green_info(fmt, arg ...)
#define pr_pos_info()
#endif

#pragma pack(1)
struct HC548_touch_point {
	u8 tp_id;
	u8 x_hi;
	u8 x_lo;
	u8 y_hi;
	u8 y_lo;
	u8 z_hi;
	u8 z_lo;
	u8 a_hi;
	u8 a_lo;
};

struct HC548_button_point {
	u8 button_id;
	u8 bz_hi;
	u8 bz_lo;
};

struct HC548_data_package {
	u8 status;
	u8 touch_num;
	struct HC548_touch_point touch_point[HC548_MAX_FINGER];
	u8 button_num;
	struct HC548_button_point button_point[HC548_MAX_BUTTON];
};
#pragma pack()

static struct i2c_client *this_client;

/* Each client has this additional data */
struct HC548_touchscreen_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct completion event_completion;
	struct workqueue_struct *ts_wq;
	struct work_struct work;
	struct task_struct *tstask;
	struct HC548_data_package HC548_data;
	int irq;
	int reset;
	const struct HC548_platform_data *pdata;
#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_suspend;
#endif

    struct work_struct	resume_work;
};	

typedef unsigned char         HC_BYTE;     //8 bit
typedef unsigned short        HC_WORD;    //16 bit
typedef unsigned int          HC_DWRD;    //16 bit
typedef unsigned char         HC_BOOL;    //8 bit 

#define HC_NULL                0x0
#define HC_TRUE                0x01
#define HC_FALSE               0x0

#define I2C_CTPM_ADDRESS        (0x80>>1)

#ifdef ENABLE_UPGRADE
static u8 HC548_fw_data[] = {
	#include "hc8600_fw.h"
};

/***
static u8 HC548_config_data[] = {
	#include "HC548_config.h"
};
****/
#endif

#if 1
/*******************************************************************************
	Define I2C read write function
********************************************************************************/
/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :  the address of the ctpm;
    pbt_buf[in]         :  data buffer;
    dw_lenth[in]        :  the length of the data buffer;
[return]:
    HC_TRUE     :success;
    HC_FALSE    :fail;
*/
int i2c_write_interface(u8 bt_ctpm_addr, u8* pbt_buf, u16 dw_lenth)
{
	int ret;
    
	ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
	if(ret != dw_lenth){
		pr_err("i2c_write_interface error,ret = %d.\n",ret);
		return HC_FALSE;
	}

	return HC_TRUE;
}

/*
[function]: 
    write a value to register.
[parameters]:
    e_reg_name[in]     : register name;
    pbt_buf[in]        : the returned register value;
[return]:
    HC_TRUE    	: success;
    HC_FALSE    : io fail;
*/
int hc_register_write(u8 e_reg_name, u8 bt_value)
{
	HC_BYTE write_cmd[2] = {0};

	write_cmd[0] = e_reg_name;
	write_cmd[1] = bt_value;

	/*call the write callback function*/
	return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, 2);
}


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    HC_TRUE     :success;
    HC_FALSE    :fail;
*/

int i2c_read_interface(u8 bt_ctpm_addr, u8* pbt_buf, u16 dw_lenth)
{
	int ret;

	ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

	if(ret != dw_lenth){
		pr_err("i2c_read_interface error,ret = %d.\n",ret);
		return HC_FALSE;
	}

	return HC_TRUE;
}

/*
[function]: 
    read out the register value.
[parameters]:
    e_reg_name[in]    :register name;
    pbt_buf[out]    :the returned register value;
    bt_len[in]        :length of pbt_buf, should be set to 2;        
[return]:
    HC_TRUE    :success;
    HC_FALSE    :io fail;
*/

u8 hc_register_read(u8 e_reg_name, u8* pbt_buf, u8 bt_len)
{
#if 1
	u8 read_cmd[3]= {0};
	u8 cmd_len     = 0;

	read_cmd[0] = e_reg_name;
	cmd_len = 1;    

	if(!i2c_write_interface(I2C_CTPM_ADDRESS, read_cmd, cmd_len))	{
		return HC_FALSE;
	}

	/*call the read callback function to get the register value*/        
	if(!i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len)){
		return HC_FALSE;
	}
	return HC_TRUE;
#else
	u8 read_cmd[3] = {0};
	u8 i = 0;
	
	for(i=0; i<bt_len; i++) {
		read_cmd[0] = e_reg_name + i;

		if(!i2c_write_interface(I2C_CTPM_ADDRESS, read_cmd, 1))	{
			return HC_FALSE;
		}

		/*call the read callback function to get the register value*/        
		if(!i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf+i, 1)){
			return HC_FALSE;
		}
	}

	return HC_TRUE;
#endif
}
#endif

static void HC548_soft_reset(struct i2c_client * client)
{
	u8 val;
	val = 0x55;
    
	hc_register_write(0x7b,val);
	msleep(5);
	val = 0x99;
	hc_register_write(0x7b,val);
	msleep(100);
}

#if 0
static void HC548_hard_reset(struct HC548_touchscreen_data *data)
{
	if (data->reset)
	{
		gpio_direction_output(data->reset, 0);
		msleep(10);
		gpio_direction_output(data->reset, 1);
		msleep(500);
	}
}
#endif

#ifdef KEY_MAP_INIT
static void HC548_key_map_init(struct input_dev *input_dev)
{
	u8 i;
    
	__set_bit(EV_KEY,input_dev->evbit);

	for(i=0; i<ARRAY_SIZE(HC548_key_map); i++)
	{
		__set_bit(HC548_key_map[i],input_dev->keybit);
	}
}
#endif

static void HC548_start(struct HC548_touchscreen_data *data)
{

}

static void HC548_stop(struct HC548_touchscreen_data *data)
{

}

static int HC548_input_open(struct input_dev *dev)
{
	struct HC548_touchscreen_data *data = input_get_drvdata(dev);
    
	pr_pos_info();
    
	HC548_start(data);
    
	return 0;
}

static void HC548_input_close(struct input_dev *dev)
{
      struct HC548_touchscreen_data *data = input_get_drvdata(dev);

      HC548_stop(data);
} 

#define HC8600_FRAME_SIZE 50

static void HC548_poscheck(struct work_struct *work)
{
	u8 val;
	u8 stat;
	u8 id,touch_num;
	int x_pos;
	int y_pos;
	int z;
#ifdef KEY_MAP_INIT
       u8 button_num;
	int b_id;
#endif
	int i;

	char data[HC8600_FRAME_SIZE] = {0};

	struct HC548_touchscreen_data *tdata;
	struct HC548_data_package *ddata;
	struct HC548_touch_point *pdata;
	struct HC548_button_point *bdata;

//       pr_pos_info();

	tdata = container_of(work, struct HC548_touchscreen_data,work);
	ddata= &(tdata->HC548_data);
	pdata = ddata->touch_point;   //pdata point to the touch point data package
	bdata = ddata->button_point;  //bdata point to the button data package

/*****************************   The process of reading TP report data  ******************************/
//1: read the 0x7c register
	hc_register_read(0x7c, &val, 1);

//2: set the bit[7] 1 in data of 0x5c register
	hc_register_read(0x5c, &val, 1);
	val |= (1<<7);
	hc_register_write(0x5c,val);

//3: read the first byte from 0x5d register
	hc_register_read(0x5d, &val, 1);
	stat = val;
//	pr_debug_info("HC548 status reg is %x\n",stat);


//4: if stat==0xff,then reset the chip
	if(stat == 0xff)
	{
//		goto schedule;
      return;
	}	

//5:if the stat bit[7] is set 1,then process the normal touch report 
	if((stat & 0x80) == 0x80)
	{
		hc_register_read(0x5d, &val, 1);
		touch_num = val;
//		pr_debug_info("HC548 touch num is %d\n",touch_num);
/*
		for(i=0;i<touch_num;i++)
		{
			hc_register_read(0x5d,((u8*) (pdata+i)), 9);
		}
*/
		for(i = 0; i < touch_num*9; i ++)
		{
			hc_register_read(0x5d,&data[2+i],1);
		}

			
		hc_register_read(0x5c, &val, 1);
		val &= ~(1<<7);
		hc_register_write(0x5c,val);	

		for(i=0; i< touch_num; i++)
		{
/*
			x_pos = ((pdata+i)->x_hi) <<8 | ((pdata+i)->x_lo);
			y_pos = ((pdata+i)->y_hi) <<8 | ((pdata+i)->y_lo);
			z = ((pdata+i)->z_hi <<8 | ((pdata+i)->z_lo));
*/
    	 id = ((data[2 + 9*i] & 0x7f) + 1);
    	 x_pos = 1024 - ((data[3 + 9*i] << 8) | (data[4 + 9*i]));
	  	 y_pos = ((data[5 + 9*i] << 8) | (data[6 + 9*i]));

//			pr_debug_info("x_hi=%d, x_lo=%d , y_hi=%d, y_lo=%d\n",(pdata+i)->x_hi,(pdata+i)->x_lo,(pdata+i)->y_hi,(pdata+i)->y_lo);
//			pr_debug_info("HC548 x=%d , y=%d , z=%d,id=%d\n",x_pos,y_pos,z,id);

            input_report_abs(tdata->input_dev, ABS_MT_TOUCH_MAJOR, 50);
			input_report_abs(tdata->input_dev, ABS_MT_POSITION_X, x_pos);
			input_report_abs(tdata->input_dev, ABS_MT_POSITION_Y, y_pos);
//			input_report_abs(tdata->input_dev, ABS_MT_TRACKING_ID, ((pdata+i)->tp_id)&0x7f);
			input_report_abs(tdata->input_dev, ABS_MT_TRACKING_ID, id);
			input_mt_sync(tdata->input_dev);
		}
	}

//6: if the stat bit[5] is set 1,then process the button touch report
#ifdef KEY_MAP_INIT
	if((stat & 0x20) == 0x20)
	{
		hc_register_read(0x5d, &val, 1);
		button_num = val;
		hc_register_read(0x5d,((u8*) bdata),button_num*3);
		hc_register_read(0x5c, &val, 1);
		val &= ~(1<<7);
		hc_register_write(0x5c,val);

		for(i=0;i<button_num;i++)
		{
			b_id = (((bdata+i)->button_id) & 0x0F) -1;
			input_event(tdata->input_dev, EV_KEY,HC548_key_map[b_id],1);
		}
	}
#endif

//7: if the stat==0x00,it means the finger put up  
	if(stat == 0x00)
	{
		input_mt_sync(tdata->input_dev);
#ifdef KEY_MAP_INIT
		for(i=0;i<HC548_MAX_BUTTON;i++)
			input_event(tdata->input_dev,EV_KEY,HC548_key_map[i],0);
#endif	
	}
	input_sync(tdata->input_dev);

//schedule:
//	enable_irq(tdata->client->irq);
}

static irqreturn_t HC548_interrupt(int irq, void* handle)
{	
	struct HC548_touchscreen_data *data = (struct HC548_touchscreen_data *)handle;

//	pr_green_info("HC548 enter interrupt\n");

//	disable_irq_nosync(data->client->irq);

//       if (!work_pending(&data->work)) 
//	{
	    queue_work(data->ts_wq,&data->work);
//       }

	return IRQ_HANDLED;
}



#ifdef ENABLE_UPGRADE
static int HC548_upgrade(struct i2c_client * client)
{
  int i,j,flash_size,config_size,haptic_size,button_slider_size,delta_table_size;
  u8 val,drive_sense_size,reg_size,ex_reg_size;
  u8 buf_1,buf_2;
  u8 retry,down_fail;
  u8 *fw_data;
  u8 *config_data;
  u8 *reg_data;
  u8 *ex_reg_data;
  u8 *config_tail;

  u8 buf[]={0x55,0x77,0xAA,0x00,0xe3,0x08,0x2e,0xA0,0x90,0x84,0x88,0x82};

  u8 reg_addr_list[]={0x15,0x14,0x13,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x25,0x24,0x27,0x26,0x1a,0x1b,0x1c,
  		     0x2f,0x30,0x31,0x32,0x33,0x1e,0x1f,0x22,0x23,0x11,0x12,0x7f,0x7d,0x7e,
  	  	     0x5b,0x68,0x17,0x16,0x19,0x18,0x08,0x09,0x0a,0x0b,0x0d,0x0c,0x0f,0x0e,0x10,0x40,0x41,0x44,
  		     0x43,0x42,0x48,0x47,0x46,0x4d,0x4b,0x4a,0x5c,0x34,0x35,0x36,0x37,0x5e,0x61,0x60,0x6c,0x79,
  		     0x78,0x70};
  u8 ex_reg_addr_list[]={  0x17,0x18,0x1a,0x19,0x1c,0x1b,0x1d,0x06,0x08,0x07,0x1f,0x1e,0x09,0x0a,0x0b,0x15,0x20,0x0c,0x16,0x21,0x22,0x23,
	     		   0x24,0x0d,0x0e,0x26,0x25,0x2a,0x29,0x2c,0x2b,0x0f,0x10,0x11,0x12,0x2d,0x2e,0x13,0x14,0x2f,0x30,0x32,0x31,0x34,
			   0x33,0x37,0x36,0x01,0x00,0x05,0x04,0x35,0x38,0x39,0x43,0x42,0x45,0x44,0x47};
/***************  is there any need to upgrade falsh??***********/

  button_slider_size = 0;
  config_size = 0;
  config_tail = 0;
  haptic_size=0;

/******************  prepare to download flash  *****************/
  flash_size =  (((int) HC548_fw_data[1+1]) << 8) | ((int) HC548_fw_data[2+1]);
  fw_data = &HC548_fw_data[3+1];
  printk(KERN_ERR "flash_size = %d\n",flash_size);

  if(flash_size)
  {
  	for(retry=0;retry<3;retry++)
  	{
  		hc_register_write(HC548_SHUTDOWN,buf[0]);
		  msleep(10);
		  hc_register_write(HC548_SHUTDOWN,buf[1]);
		  msleep(10);//400
	
	    hc_register_read(HC548_HW_FW_REV, &val, 1);
		  while(val != 0x21)
		  {
		  	hc_register_write(HC548_SHUTDOWN,buf[0]);
		 	  msleep(10);
		 	  hc_register_write(HC548_SHUTDOWN,buf[1]);
		 	  msleep(10);//300
		 	  hc_register_read(HC548_HW_FW_REV, &val, 1);
		  }
	
		  
	    hc_register_write(HC548_WD_DISABLE,buf[2]);
		  hc_register_write(HC548_BOOT_STAT,buf[3]);
		  hc_register_read(HC548_ERROR_ID, &val,1);
		  hc_register_write(HC548_FLASH_EN,buf[4]);
		  hc_register_write(HC548_DEBUG,buf[5]);
		  hc_register_write(HC548_DOWNLOAD_CTL,buf[6]);
	
		  mdelay(10);//100
		  hc_register_read(HC548_DOWNLOAD_CTL, &val,1);
		  if(val == 0x2e)
		      printk(KERN_ERR "HC548 ERASE SUCESS!\n");
		  else
		  {
		      printk(KERN_ERR "HC548 ERASE FAIL!\n");
		      return -1;
		  }
		  hc_register_read(HC548_ERROR_ID, &val,1);
		/*********************  flash downloading  **********************/

		  for(i=0;i<flash_size;i++)
		  {
		  	int ret = 0;
		  	//do
		  	{
	        	ret = hc_register_write(HC548_DOWNLOAD_DATA, *(fw_data+i));
		    	if(ret == HC_FALSE)
		    	{   		
		    		mdelay(5);
		    		printk("===========>Try download again!!\n");
		    		return -1;
		    	}
		    
		    }
		  }

		/*************** check flash download sucess or fail ************/
		  mdelay(40);//800
		  hc_register_read(HC548_BOOT_STAT, &val,1);
		  if((val & 0x02) == 0x02)
		  {
		      printk(KERN_ERR "HC548 flash down sucess!\n");
		  }
		  else
		  {
		      printk(KERN_ERR "HC548 flash down FAIL!\n");
		      return -1;
		  }
		  hc_register_read(HC548_HW_FW_REV, &val,1);
		  printk(KERN_ERR "reg 0x01 = 0x%x\n",val);
		  
		  if(val == 0x2F)
		  	break;
		 }
  }

/**********  is there any need to upgrade config file  *********/

/*************  prepare to download config file  ***************/
  config_size = (((int) HC548_fw_data[flash_size + 3+1]) << 8) | ((int) HC548_fw_data[flash_size + 4+1]);
  printk(KERN_ERR "config_size = %d\n",config_size);
  if(config_size)
  {
  	for(retry=0;retry<3;retry++)
  	{
		  config_data = fw_data + flash_size + 2;
		  config_tail = config_data + config_size;
		  hc_register_write(HC548_WD_DISABLE,buf[2]);
		  hc_register_write(HC548_BOOT_STAT,buf[3]);
		  hc_register_write(HC548_FLASH_EN,buf[4]);
		 
	
		  for(j=0;j<4;j++)
		  {
		//    printk(KERN_ERR "HC548 config_data = %p",config_data);
		    if(config_data == config_tail)
		    {
		      printk(KERN_ERR "Succeed! HC548 all config file have downloaded!\n");
		      break;
		    }
	
		    switch(*(config_data))
		    {
		      case 0x29:
		      {
			haptic_size |= *(config_data+3);
			haptic_size <<= 8;
			haptic_size |= *(config_data+2);
			hc_register_write(HC548_DOWNLOAD_CTL,*config_data);
			for(i=1;i<haptic_size+1;i++)
				hc_register_write(HC548_DOWNLOAD_DATA,*(config_data+i));
			hc_register_write(HC548_FLASH_CTL,buf[7]);
			config_data += haptic_size+1;
			printk(KERN_ERR "HC548 haptic config file downloading...\n");
		      }
		      break;
		      case 0x2A:
		      {
			button_slider_size |= *(config_data+3);
			button_slider_size <<= 8;
			button_slider_size |= *(config_data+2);
			hc_register_write(HC548_DOWNLOAD_CTL,*config_data);
			for(i=1;i<button_slider_size+1;i++)
				hc_register_write(HC548_DOWNLOAD_DATA,*(config_data+i));
			hc_register_write(HC548_FLASH_CTL,buf[8]);
			config_data += button_slider_size+1;
			printk(KERN_ERR "HC548 button/slider config file downloading...\n");
		      }
		      break;
		      case 0x2C:
		      {
			drive_sense_size = 28;
			hc_register_write(HC548_DOWNLOAD_CTL,*config_data);
			for(i=1;i<drive_sense_size;i++)
				hc_register_write(HC548_DOWNLOAD_DATA,*(config_data+i));
			hc_register_write(HC548_FLASH_CTL,buf[9]);
			config_data += drive_sense_size;
			printk(KERN_ERR "HC548 drive/sense config file downloading...\n");
		      }
		      break;
		      case 0x2D:
		      {
			delta_table_size = 482;
			hc_register_write(HC548_DOWNLOAD_CTL,*config_data);
			for(i=1;i<delta_table_size;i++)
				hc_register_write(HC548_DOWNLOAD_DATA,*(config_data+i));
			hc_register_write(HC548_FLASH_CTL,buf[10]);
			config_data += delta_table_size;
			printk(KERN_ERR "HC548 delta table config file downloading...\n");
		      }
		      break;
		      default:
			printk(KERN_ERR "HC548 valid config file!!!\n");
		      break;
		    }
		    
		    mdelay(10);//100
		    hc_register_read(HC548_BOOT_STAT, &val,1);
		    if((val & 0x40) == 0x40)
			printk(KERN_ERR "current config file download sucess!\n");
		    else
		    {
		        printk(KERN_ERR "current config file download FAIL!\n");
		        down_fail = 1;
		    }
		  }
		  if(!down_fail)
		  	break;
	  }
  }

/*************  prepare to download IO register  ***************/
//  HC548_soft_reset(client);
  mdelay(10);//300
  reg_size = HC548_fw_data[flash_size + config_size + 5 +1];
  printk(KERN_ERR "reg size = %d\n",reg_size);
  reg_data = config_tail +1;

  if(reg_size)
  {
  	hc_register_write(HC548_WD_DISABLE,buf[2]);
	  hc_register_write(HC548_BOOT_STAT,buf[3]);
		hc_register_write(HC548_FLASH_EN,buf[4]);
	for(i=0;i<reg_size;i++)
	{
		hc_register_write(reg_addr_list[i],*(reg_data+i));
	}
       hc_register_write(HC548_FLASH_CTL,buf[11]);
	mdelay(10);//200
  }

/************  prepare to download extend register  **************/
  ex_reg_size = HC548_fw_data[flash_size + config_size + reg_size + 6 +1];
  printk(KERN_ERR "ex_reg_size = %d\n",ex_reg_size);
//  ex_reg_data = reg_data +reg_size +11;
//  printk(KERN_ERR "first ex_reg_data = %d\n",*ex_reg_data);
  if(ex_reg_size)
  {
	buf_1 = 0x17;
        buf_2 = 0x19;	
  hc_register_write(HC548_WD_DISABLE,buf[2]);
	hc_register_write(HC548_BOOT_STAT,buf[3]);
	hc_register_write(HC548_FLASH_EN,buf[4]);
    	for(i=0;i<ex_reg_size-10;i++)
	{	
		hc_register_write(0x50,buf_1);
		hc_register_write(0x51,ex_reg_addr_list[i]);
		hc_register_write(0x50,buf_2);
		hc_register_write(0x51,*(ex_reg_data+i));
		
	}

  hc_register_write(HC548_FLASH_CTL,buf[11]);
	mdelay(10);//200

  }

  hc_register_write(HC548_FLASH_EN,buf[3]);
  hc_register_write(HC548_WD_DISABLE,buf[3]);

  HC548_soft_reset(client);
//  mdelay(300);

  printk(KERN_ERR "HC548 upgrade file download sucess!\n");
  
  return 0;
}
#endif

#ifdef CONFIG_HC8600_SLEEP_KEEP_POWER
static void ts_work_resume(struct work_struct *work)
{
    int i;
    struct HC548_touchscreen_data *ts_data = container_of(work, struct HC548_touchscreen_data,resume_work);
    printk("Enter %s\n", __func__);
#ifdef ENABLE_UPGRADE
	HC548_upgrade(ts_data->client);
#endif    

	enable_irq(ts_data->client->irq);
}
#endif

#ifdef CONFIG_PM
static int HC548_suspend(struct device *dev)//(struct i2c_client *client , pm_message_t mesg)
{
	struct i2c_client *client = to_i2c_client(dev);
//	u8 val;
//	val = 0x55;
    	printk("====CTP  %s\n",__func__);
//	hc_register_write(0x7b,val);
	disable_irq_nosync(client->irq);//irq
#if 0
	gpio_direction_output(RK30_PIN0_PB6, 0);
	gpio_set_value(RK30_PIN0_PB6,0);
	//i2c_smbus_write_i2c_block_data(client,0x7b,1,&val);
#endif	
	return 0;
}

static int HC548_resume(struct device *dev)//(struct i2c_client *client)
{
	struct i2c_client *client = to_i2c_client(dev);

#ifdef CONFIG_HC8600_SLEEP_KEEP_POWER
	struct HC548_touchscreen_data *ts_data = i2c_get_clientdata(client);
	schedule_work(&ts_data->resume_work);
#else
//	u8 val;
//	val = 0x99;
    printk("====CTP  %s\n",__func__);
#if 0
    gpio_direction_output(RK30_PIN0_PB6, 1);
	gpio_set_value(RK30_PIN0_PB6,1);
#endif
	enable_irq(client->irq);
	
//	hc_register_write(0x7b,val);	
	
	//i2c_smbus_write_i2c_block_data(client,0x7b,1,&val);
#endif

	return 0;
}

static struct dev_pm_ops ts_pmops = {
	.suspend	= HC548_suspend,
	.resume		= HC548_resume,
};
#endif

static int __devinit HC548_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct HC548_touchscreen_data *data; 
	struct input_dev *input_dev;
	u8 buf,buf1;
	int error;
#ifdef ENABLE_UPGRADE
	int times = 0;
#endif

	pr_pos_info();

	data = kzalloc(sizeof(struct HC548_touchscreen_data), GFP_KERNEL);
	if(data == NULL)
	{
		pr_red_info("HC548 kzalloc touchscreen_data failed!\n");
		error = -ENOMEM;
		goto exit_alloc_data_failed;
	}
    
	input_dev = input_allocate_device();
	if(input_dev == NULL)
	{
		pr_red_info("HC548 input allocate device failed!\n");
		error = -ENOMEM;
		goto exit_input_dev_alloc_failed;
	}

	data->input_dev = input_dev;
	input_set_drvdata(input_dev, data);

	input_dev->name = HC5XX_NAME;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = HC548_input_open;
	input_dev->close = HC548_input_close;

	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
//	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_dev->propbit[0] = BIT(INPUT_PROP_DIRECT);

	input_dev->absbit[0] =  BIT_MASK(ABS_MT_TRACKING_ID) |
			BIT_MASK(ABS_MT_POSITION_X) | BIT_MASK(ABS_MT_POSITION_Y)|
			BIT_MASK(ABS_MT_TOUCH_MAJOR);// | BIT_MASK(ABS_MT_WIDTH_MAJOR);
	
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, HC548_MAX_X, 0 , 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, HC548_MAX_Y, 0 , 0);
	input_set_abs_params(input_dev,ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
//	input_set_abs_params(input_dev,ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 5, 0, 0);

#ifdef KEY_MAP_INIT
	HC548_key_map_init(input_dev);
#endif

	error = input_register_device(input_dev);
	if(error < 0)
	{
		pr_red_info("HC548 input_register_device failed!\n");
		goto err_unregister_device; 
	}

	data->ts_wq = create_singlethread_workqueue("kworkqueue_ts");
	if (!data->ts_wq){
		dev_err(&client->dev, "Could not create workqueue\n");
		goto error_wq_create;
	}
	flush_workqueue(data->ts_wq);
    
	INIT_WORK(&data->work, HC548_poscheck);	
	data->client = client;
	i2c_set_clientdata(client,data);
	this_client = client; 

        /*  configure rest pin */
#if 0
	data->reset = RK30_PIN0_PB6;//RK2928_PIN1_PB1; //INVALID_GPIO;//S5PV210_GPH2(5);
	if (data->reset != INVALID_GPIO) {
		error = gpio_request(data->reset, NULL);
		if (error < 0){
		      pr_red_info("Unable to request GPIO pin %d to act as reset\n",data->reset);
                    goto exit_rst_gpio_request_failed;
		}
  	}
	HC548_hard_reset(data);
#endif
	
	hc_register_read(0x00, &buf, 1);
	if(buf == 0x86)
		pr_green_info("HC548: I2C communication is OK, TP chip ID is %x\n",buf);
	else
		pr_red_info("HC548: I2C ERROR!! TP chip ID is %x\n",buf);

#ifdef ENABLE_UPGRADE
		pr_green_info("HC548: HC548_fw_data[0] is %x\n",HC548_fw_data[0]);
	if(HC548_fw_data[0] == 0xff)   //debug_control,if data is 0xff,TP drive always upgrade
	{		
		while(1)
		{
			int ret = 0;
			ret = HC548_upgrade(client);
			if(ret < 0){
				times ++;
				printk("times = %d\n",times);
			}else{
				times = 0;
				break;
			}
			if(times >= 10)
				break;				
		}
	}else{
		__u8 buf;
		//i2c_smbus_read_i2c_block_data(client,0x75,1,&buf);
		hc_register_read(0x75, &buf, 1);
		hc_register_read(0x73, &buf1, 1);
		if((buf < HC548_fw_data[0]) || ((buf == HC548_fw_data[0]) && (buf1 < HC548_fw_data[1])))
//	if(buf < HC548_fw_data[0])
		{
			while(1)
			{
				int ret = 0;
				ret = HC548_upgrade(client);
				if(ret < 0){
					times ++;
					printk("times = %d\n",times);
				}else{
					times = 0;
					break;
				}
				if(times >= 10)
					break;					
			}
		}
	}
	printk("====CTP download times = %d\n",times);
#endif

	/* configure interrupt pin */
#if 0
	data->irq = RK30_PIN1_PB7;//RK2928_PIN1_PB3;//RK2928_PIN1_PB0;
//S5PV210_GPH2(4);
	error = gpio_request(data->irq, "touch_irq_key");
	if (error < 0) {
		pr_red_info("Unable to request GPIO pin %d. error %d.\n", data->irq, error);
		goto exit_irq_gpio_request_failed;
	}
	error = gpio_direction_input(data->irq);
	if (error < 0) {
		pr_red_info("Failed to configure input direction for GPIO %d, error %d\n", data->irq, error);
		goto err_free_irq_gpio;
	}
#endif
	client->irq = IRQ_PORT;//gpio_to_irq(data->irq);
	if (client->irq < 0) {
		error = client->irq;
		pr_red_info( "Unable to get irq number for GPIO %d, error %d\n", data->irq, error);
		goto err_free_irq;
	}
	
//	error = request_threaded_irq(client->irq, NULL, HC548_interrupt, IRQF_TRIGGER_RISING, client->dev.driver->name, data);
	error = request_threaded_irq(client->irq, NULL, HC548_interrupt, IRQF_TRIGGER_FALLING, client->dev.driver->name, data);
	if(error)
	{
		pr_red_info("HC548 failed to register irq!\n");
		goto exit_irq_request_failed;
	}
#ifdef CONFIG_HC8600_SLEEP_KEEP_POWER
    INIT_WORK(&data->resume_work, ts_work_resume);
#endif
	pr_green_info("HC548_probe ok!\n");

       return 0;

exit_irq_request_failed:
    cancel_work_sync(&data->work);
err_free_irq:
    free_irq(client->irq, data);
//err_free_irq_gpio:
//    if (data->irq)
 //       gpio_free(data->irq);
//exit_irq_gpio_request_failed:  
//    HC548_hard_reset(data);
//    if (data->reset)
 //       gpio_free(data->reset);
exit_rst_gpio_request_failed:   
    destroy_workqueue(data->ts_wq);
    i2c_set_clientdata(client,NULL);
    this_client = NULL; 
error_wq_create:
    input_unregister_device(input_dev); 
err_unregister_device:
    input_free_device(input_dev);
exit_input_dev_alloc_failed:
    kfree(data);
exit_alloc_data_failed: 
    return error;
}

static int HC548_remove(struct i2c_client *client)
{
    struct HC548_touchscreen_data *data = i2c_get_clientdata(client);

    cancel_work_sync(&data->work);
    free_irq(client->irq, data);
    destroy_workqueue(data->ts_wq);
    input_unregister_device(data->input_dev); 
    if (data->reset)
        gpio_free(data->reset);
    if (data->irq)
        gpio_free(data->irq);

    kfree(data);

    return 0;
}

static const struct i2c_device_id HC548_id[] = {
	{ "HC548", 1 },
	{ }
};

MODULE_DEVICE_TABLE(i2c,HC548_id);

static struct i2c_driver HC548_driver = {
	.driver = {
		.name = HC5XX_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &ts_pmops,
#endif
	},
	.probe = HC548_probe,
	.remove = HC548_remove,
//	.suspend = HC548_suspend,
//	.resume = HC548_resume,
	.id_table = HC548_id,
};

int HC548_attach_adapter(void)
{
	struct i2c_board_info info;
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	int ret1;

	pr_pos_info();

	adapter = i2c_get_adapter(I2C_BUS_NUM);
	if(adapter == NULL)
	{
		pr_red_info("HC548 I2C get adapter %d failed\n", I2C_BUS_NUM);
		ret1 = -ENODEV;
		goto out_exit;	
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	strlcpy(info.type, "HC548", I2C_NAME_SIZE);
	info.addr = HC548_SLAVE_ADDR;
	client = i2c_new_device(adapter, &info);
	if(client == NULL)
	{
		pr_red_info("HC548 I2C new device failed\n");
		ret1 = -ENODEV;
		goto out_put_adapter;
	}
	return 0;

out_put_adapter:
	i2c_put_adapter(adapter);
out_exit:
	return ret1;	
	
}

static int __init HC548_init(void)
{
	int ret;
    
	pr_pos_info();

	ret = HC548_attach_adapter();
	if(ret < 0)
	{
		pr_red_info("HC548 attach adapter failed!\n");
		return ret;
	}

	ret = i2c_add_driver(&HC548_driver);
	if(ret < 0)
	{
		pr_red_info("i2c_add_driver failed!\n");
		return ret;
	}

	return 0;
}

static void __exit HC548_exit(void)
{  
	pr_pos_info();
    
       i2c_del_driver(&HC548_driver);
}

module_init(HC548_init);
module_exit(HC548_exit);

/*  Module information */
MODULE_AUTHOR("xiaq <xiaq@ichaier.com>");
MODULE_DESCRIPTION("HC548 Touchscreen driver");
MODULE_LICENSE("GPL");
