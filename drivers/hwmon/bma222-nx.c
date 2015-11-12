/* 
 * linux/drivers/char/sensor_bma222.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux BACK-LIGHT DRIVER
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
//#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/mach-types.h>
#include <linux/delay.h>
//#include <mach/bsp.h>

#ifdef CONFIG_I2C
#include <linux/i2c.h>
#endif
#include <asm/gpio.h>
#include <linux/input.h>
#include <linux/slab.h>
//#include <linux/sensor_ioctl.h>

//#include "bma222e_sensor_event.h"


typedef struct {
    int x;
    int y;
    int z;
    int resolution;
    int delay_time;
} sensor_accel_t;

#define SENSOR_CONTROL_IOC_MAGIC 			'S'

#define IOCTL_SENSOR_SET_INPUTDEVICE		_IOW(SENSOR_CONTROL_IOC_MAGIC, 0, unsigned int)
#define IOCTL_SENSOR_SET_DELAY_ACCEL		_IOW(SENSOR_CONTROL_IOC_MAGIC, 1, unsigned int)
#define IOCTL_SENSOR_SET_CALIB_ACCEL		_IOW(SENSOR_CONTROL_IOC_MAGIC, 2, sensor_accel_t)
//#define IOCTL_SENSOR_SET_EVENT_THRES		_IOW(SENSOR_CONTROL_IOC_MAGIC, 3, unsigned int)

#define IOCTL_SENSOR_GET_RESOLUTION		_IOR(SENSOR_CONTROL_IOC_MAGIC, 4, unsigned int)
#define IOCTL_SENSOR_GET_TEMP_INFO		_IOR(SENSOR_CONTROL_IOC_MAGIC, 5, unsigned int)
#define IOCTL_SENSOR_GET_DATA_ACCEL		_IOR(SENSOR_CONTROL_IOC_MAGIC, 6, sensor_accel_t)
#define IOCTL_SENSOR_GET_SENSOR_TYPE		_IOR(SENSOR_CONTROL_IOC_MAGIC, 7, int)



#define SENSOR_DEV_NAME		"accel"
#define SENSOR_DEV_MAJOR		249
#define SENSOR_DEV_MINOR		1

#define SENSOR_ENABLE			1
#define SENSOR_DISABLE			0

#define SENSOR_SUPPORT			1
#define SENSOR_NOT_SUPPORT	0

#define SENSOR_G_MASK			0x1
#define SENSOR_T_MASK			0x2

#define BMA_DEBUG    	0
#define BMA_DEBUG_D  	0

#if BMA_DEBUG
#define sensor_dbg(fmt, arg...)     printk(fmt, ##arg)
#else
#define sensor_dbg(arg...)
#endif

#if BMA_DEBUG_D
#define sensor_dbg_d(fmt, arg...)     printk(fmt, ##arg)
#else
#define sensor_dbg_d(arg...)
#endif

//#define CONFIG_TCC_BB
//#define CONFIG_GSEN_DTYPE_DELTA

#if defined(CONFIG_TCC_BB)
/* X-axis */
#if defined(CONFIG_GSEN_X_X)
#define MAT_11	1
#elif defined(CONFIG_GSEN_X_mX)
#define MAT_11	-1
#else
#define MAT_11	0
#endif
#if defined(CONFIG_GSEN_X_Y)
#define MAT_12	1
#elif defined(CONFIG_GSEN_X_mY)
#define MAT_12	-1
#else
#define MAT_12	0
#endif
#if defined(CONFIG_GSEN_X_Z)
#define MAT_13	1
#elif defined(CONFIG_GSEN_X_mZ)
#define MAT_13	-1
#else
#define MAT_13	0
#endif

/* Y-axis */
#if defined(CONFIG_GSEN_Y_X)
#define MAT_21	1
#elif defined(CONFIG_GSEN_Y_mX)
#define MAT_21	-1
#else
#define MAT_21	0
#endif
#if defined(CONFIG_GSEN_Y_Y)
#define MAT_22	1
#elif defined(CONFIG_GSEN_Y_mY)
#define MAT_22	-1
#else
#define MAT_22	0
#endif
#if defined(CONFIG_GSEN_Y_Z)
#define MAT_23	1
#elif defined(CONFIG_GSEN_Y_mZ)
#define MAT_23	-1
#else
#define MAT_23	0
#endif

/* Z-axis */
#if defined(CONFIG_GSEN_Z_X)
#define MAT_31	1
#elif defined(CONFIG_GSEN_Z_mX)
#define MAT_31	-1
#else
#define MAT_31	0
#endif
#if defined(CONFIG_GSEN_Z_Y)
#define MAT_32	1
#elif defined(CONFIG_GSEN_Z_mY)
#define MAT_32	-1
#else
#define MAT_32	0
#endif
#if defined(CONFIG_GSEN_Z_Z)
#define MAT_33	1
#elif defined(CONFIG_GSEN_Z_mZ)
#define MAT_33	-1
#else
#define MAT_33	0
#endif
#endif	//CONFIG_TCC_BB

typedef struct _matrix3by3 {
	short	_11;
	short	_12;
	short	_13;
	short	_21;
	short 	_22;
	short 	_23;
	short	_31;
	short 	_32;
	short 	_33;
} matrix3by3;

#define LAYOUT_CHIP2HAL	0
#define LAYOUT_HAL2CHIP	1

#if defined(CONFIG_TCC_BB)
void print_matrix(matrix3by3 *layout)
{
	printk("bma222 3-axis direction\n");
	printk(" %2d, %2d, %2d\n", layout->_11, layout->_12, layout->_13);
	printk(" %2d, %2d, %2d\n", layout->_21, layout->_22, layout->_23);
	printk(" %2d, %2d, %2d\n", layout->_31, layout->_32, layout->_33);
}

matrix3by3 gsenlayout[2] = {
	{
	MAT_11, MAT_12, MAT_13, 
	MAT_21, MAT_22, MAT_23, 
	MAT_31, MAT_32, MAT_33
	},
	{
	MAT_11, MAT_12, MAT_13, 
	MAT_21, MAT_22, MAT_23, 
	MAT_31, MAT_32, MAT_33
	}
};
#else
matrix3by3 gsenlayout[2] = 
{
    // LAYOUT_CHIP2HAL
    // LAYOUT_HAL2CHIP
#if defined(CONFIG_GSEN_TOP)
    #if defined(CONFIG_GSEN_ROTATE_0)
    {1,	0,	0,	0,	1,	0,	0,	0,	1},
    {1,	0,	0,	0,	1,	0,	0,	0,	1},
    #elif defined(CONFIG_GSEN_ROTATE_90)
    {0,	1,	0,	-1,	0,	0,	0,	0,	1},
    {0,	-1,	0,	1,	0,	0,	0,	0,	1},
    #elif defined(CONFIG_GSEN_ROTATE_180)
    {-1,	0,	0,	0,	-1,	0,	0,	0,	1},
    {-1,	0,	0,	0,	-1,	0,	0,	0,	1},
    #else // CONFIG_GSEN_ROTATE_270
    {0,	-1,	0,	1,	0,	0,	0,	0,	1},
    {0,	1,	0,	-1,	0,	0,	0,	0,	1},
    #endif
#else // CONFIG_GSEN_BOTTOM
    #if defined(CONFIG_GSEN_ROTATE_0)
    {-1,	0,	0,	0,	1,	0,	0,	0,	-1},
    {-1,	0,	0,	0,	1,	0,	0,	0,	-1},
    #elif defined(CONFIG_GSEN_ROTATE_90)
    {0,	-1,	0,	-1,	0,	0,	0,	0,	-1},
    {0,	1,	0,	1,	0,	0,	0,	0,	-1},
    #elif defined(CONFIG_GSEN_ROTATE_180)
    {1,	0,	0,	0,	-1,	0,	0,	0,	-1},
    {1,	0,	0,	0,	-1,	0,	0,	0,	-1},
    #else // CONFIG_GSEN_ROTATE_270
    {0,	1,	0,	1,	0,	0,	0,	0,	-1},
    {0,	-1,	0,	-1,	0,	0,	0,	0,	-1},
    #endif
#endif
};
#endif

struct bmasensoracc{
	short	x;
	short	y;
	short	z;
} ;

struct bmacalib{
	atomic_t x;
	atomic_t y;
	atomic_t z;
};

enum{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
	AXIS_MAX
};

enum{
	DEVICE_TYPE_CHAR,
	DEVICE_TYPE_INPUT,
	DEVICE_TYPE_MAX
};

enum{
	CALIBRATION_SOFT,
	CALIBRATION_HW_AUTO,
	CALIBRATION_HW_MANUAL,
	CALIBRATION_MAX
};

//#define SENSOR_TUNING

#ifdef SENSOR_TUNING
#define SENSOR_TUNING_DIVIDE_2	2  	// 0.5
#define SENSOR_TUNING_DIVIDE_4	4 	// 0.25
#define SENSOR_TUNING_DIVIDE_5	5 	// 0.2
#define SENSOR_TUNING_DIVIDE_10	10 	// 0.1

#define SENSOR_TUNING_DIVIDE 	SENSOR_TUNING_DIVIDE_5

#define SENSOR_TUNING_BIT_WIDTH_EXTEND
#endif 

struct bmasensor_data {
	struct i2c_client * i2cClient;
	#ifdef SENSOR_TUNING
	atomic_t realDelayCnt;
	atomic_t realDelayTriggerCnt;
	atomic_t realDelay;
	struct bmasensoracc avgValue;
	#endif
	atomic_t delay;
	atomic_t enable;
	atomic_t suspend;
	atomic_t inputDevice;
	atomic_t resolution;
	atomic_t calibMode;
	unsigned short mode;
	struct input_dev *input;
	struct bmasensoracc value;
	struct bmacalib calibOffset;
	struct mutex value_mutex;
	struct mutex enable_mutex;
	struct mutex suspend_mutex;
	struct mutex mode_mutex;
	struct delayed_work work;
	//struct work_struct irq_work;
	int IRQ;
#if defined(CONFIG_TCC_BB)
	int sensor_type;			/* type of sensor */
        wait_queue_head_t wq;           /* for poll */
        atomic_t count;                         /* for poll count */
        int flag;                                       /* check first data */
        unsigned int event_thres;       /* event threshold */
	struct bmasensoracc prev;	/* previous data for CONFIG_GSEN_DTYPE_DELTA */
#endif
};

struct bmasensor_data *mData;

#if defined(CONFIG_TCC_BB)
#include <linux/poll.h>
#define MAXEVENTS 100
sensor_accel_t *sbuf = NULL;
int idx = 0;
/* tuning for error event */
//#define EVENT_TUNING
#endif

static int sensor_used_count=0;

#define BMA222_GET_BITSLICE(regvar, bitname)\
		   (regvar & bitname##__MSK) >> bitname##__POS

#define BMA222_SET_BITSLICE(regvar, bitname, val)\
		  (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)  


#define BMA222_OFFSET_Z_RESULT_REG	0x3A
#define BMA222_OFFSET_Y_RESULT_REG	0x39
#define BMA222_OFFSET_X_RESULT_REG	0x38
#define BMA222_OFFSET_TARGET_REG		0x37

#define BMA222_PMU_SELF_TEST			0x32

#define BMA222_FIFO_CONFIG_0			0x30

#define BMA222_INT_C 					0x2E
#define BMA222_INT_B 					0x2D
#define BMA222_INT_A 					0x2C
#define BMA222_INT_9 					0x2B
#define BMA222_INT_8 					0x2A
#define BMA222_INT_7 					0x29
#define BMA222_INT_6 					0x28
#define BMA222_INT_5 					0x27
#define BMA222_INT_4 					0x26
#define BMA222_INT_3 					0x25
#define BMA222_INT_2 					0x24
#define BMA222_INT_1 					0x23
#define BMA222_INT_0 					0x22


#define BMA222_INT_RST_LATCH			0x21

#define BMA222_INT_OUT_CTRL			0x20

#define BMA222_INT_SRC					0x1E

#define BMA222_INT_MAP_2				0x1B
#define BMA222_INT_MAP_1				0x1A
#define BMA222_INT_MAP_0				0x19

#define BMA222_INT_EN_2				0x18
#define BMA222_INT_EN_1				0x17
#define BMA222_INT_EN_0				0x16

#define BMA222_BGW_SOFTRESET			0x14

#define BMA222_BANDWIDTH_CONFIG_REG	0x10
#define BMA222_RANGE_REG				0x0F

#define BMA222_FIFO_STATE				0x0E

#define BMA222_INT_STATE_3				0x0C
#define BMA222_INT_STATE_2				0x0B
#define BMA222_INT_STATE_1				0x0A
#define BMA222_INT_STATE_0				0x09

#define BMA222_TEMP_RD_REG			0x08

#define BMA222_Z_AXIS_DATA_REG		0x07
#define BMA222_Y_AXIS_DATA_REG		0x05
#define BMA222_X_AXIS_DATA_REG		0x03

#define BMA222_NEWZ_AXIS_DATA_REG	0x06
#define BMA222_NEWY_AXIS_DATA_REG	0x04
#define BMA222_NEWX_AXIS_DATA_REG	0x02

#define BMA222_CHIP_ID_REG				0x00

/** CAL_MANUAL
*/
#define BMA222_CAL_MANUAL__POS		7
#define BMA222_CAL_MANUAL__MSK		0x80
#define BMA222_CAL_MANUAL__LEN		1
#define BMA222_CAL_MANUAL__REG		BMA222_OFFSET_TARGET_REG

/** OFFSET_X
*/
#define BMA222_OFFSET_X__POS			0
#define BMA222_OFFSET_X__MSK			0xFF
#define BMA222_OFFSET_X__LEN			8
#define BMA222_OFFSET_X__REG			BMA222_OFFSET_X_RESULT_REG

/** OFFSET_Y
*/
#define BMA222_OFFSET_Y__POS			0
#define BMA222_OFFSET_Y__MSK			0xFF
#define BMA222_OFFSET_Y__LEN			8
#define BMA222_OFFSET_Y__REG			BMA222_OFFSET_Y_RESULT_REG

/** OFFSET_Z
*/
#define BMA222_OFFSET_Z__POS			0
#define BMA222_OFFSET_Z__MSK			0xFF
#define BMA222_OFFSET_Z__LEN			8
#define BMA222_OFFSET_Z__REG			BMA222_OFFSET_Z_RESULT_REG

/** RANGE
*/
#define BMA222_RANGE__POS				0
#define BMA222_RANGE__MSK				0x0F
#define BMA222_RANGE__LEN				4
#define BMA222_RANGE__REG				BMA222_RANGE_REG

/** SC_FILT_CONFIG
*/
#define BMA222_SC_FILT_CONFIG__POS	0
#define BMA222_SC_FILT_CONFIG__MSK	0x1F
#define BMA222_SC_FILT_CONFIG__LEN  	5
#define BMA222_SC_FILT_CONFIG__REG	BMA222_BANDWIDTH_CONFIG_REG

/** ACC_X
*/
#define BMA222_ACC_X__POS				0
#define BMA222_ACC_X__MSK				0xFF
#define BMA222_ACC_X__LEN				8
#define BMA222_ACC_X__REG				BMA222_X_AXIS_DATA_REG

 /** ACC_Y
*/
#define BMA222_ACC_Y__POS				0
#define BMA222_ACC_Y__MSK				0xFF
#define BMA222_ACC_Y__LEN				8
#define BMA222_ACC_Y__REG				BMA222_Y_AXIS_DATA_REG

 /** ACC_Z
*/
#define BMA222_ACC_Z__POS				0
#define BMA222_ACC_Z__MSK				0xFF
#define BMA222_ACC_Z__LEN				8
#define BMA222_ACC_Z__REG				BMA222_Z_AXIS_DATA_REG

/** RANGE setting*/
#define BMA222_RANGE_2G				0x03
#define BMA222_RANGE_4G				0x05
#define BMA222_RANGE_8G				0x08
#define BMA222_RANGE_16G				0x0C

/** BANDWIDTH constants for internal SC filter
\note when setting RC_FILT_EN is set to '1' the setting of internal BW is set to 1kHz
*/
#define BMA222_BANDWIDTH_31HZ			0x0a
#define BMA222_BANDWIDTH_62HZ			0x0b
#define BMA222_BANDWIDTH_125HZ			0x0c
#define BMA222_BANDWIDTH_250HZ			0x0d
#define BMA222_BANDWIDTH_500HZ			0x0e
#define BMA222_BANDWIDTH_1000HZ			0x0f


#define BMA_MAX_DELAY					200 
#define BMA_DEFAULT_DELAY				20 

#define BMA_ORG_DATA_BIT_WIDTH			8

#ifdef SENSOR_TUNING_BIT_WIDTH_EXTEND
#define BMA_DATA_TUNING_WIDTH				2
#else
#define BMA_DATA_TUNING_WIDTH				0
#endif
#define BMA_DATA_BIT_WIDTH				(BMA_ORG_DATA_BIT_WIDTH + BMA_DATA_TUNING_WIDTH)
#define BMA_1G_RESOLUTIOIN    				(1<<(BMA_DATA_BIT_WIDTH-2))
#define BMA_DATA_ABS						(1<<(BMA_DATA_BIT_WIDTH-1))
#define ABSMIN								(-BMA_DATA_ABS)
#define ABSMAX								(BMA_DATA_ABS)
#define BMA_TEMPERATURE_RESOLUTION		2
#define BMA_TEMPERATURE_CENTER			(-30)

#define BMA222_I2C_ADDRESS      				0x18
#define SENSOR_CHIP_ID_REG					(BMA222_CHIP_ID_REG)

#define SENSOR_I2C_ADDRESS					(BMA222_I2C_ADDRESS)
#define SENSOR_DEFAULT_RANGE				0//(BMA222_RANGE_2G)
#define SENSOR_DEFAULT_BW					0//(BMA222_BANDWIDTH_31HZ)
#define BMA222E_NAME 						"bma222e"
#define SENSOR_HAL_NAME					"BMA222 3-axis Accelerometer"
#define SENSOR_CALIBRATION_MODE			(CALIBRATION_HW_MANUAL)
#define SENSOR_DEF_DEVICE_TYPE			(DEVICE_TYPE_CHAR)
#define SENSOR_TEMPERATURE_SUPPORT		(SENSOR_NOT_SUPPORT)
#define SENSOR_TEMPERATURE_RESOLUTION 	(BMA_TEMPERATURE_RESOLUTION)
#define SENSOR_TEMPERATURE_CENTER			(BMA_TEMPERATURE_CENTER)

static int bma222_read_accel_xyz(struct i2c_client *client,	struct bmasensoracc *acc);
static void bma222e_sensor_convertCoordination(struct bmasensoracc *sensor_accel, matrix3by3 *layout);

#if 0
static int bma222e_write_byte(struct i2c_client *client,unsigned char reg, unsigned char val)
{
	int i = 0;
	int ret = 0;

	for(i=0; i<3; i++)
	{
		ret = i2c_smbus_write_byte_data(client, reg, val);
		if (ret < 0) 
		{
			dev_err(&client->dev, "\e[31mfailed write_byte reg:0x%02x val:0x%02x, cnt:%d\e[0m\n", reg, val, i);
			msleep(1);
		}
		else
		{
			break;
		}
	}

	return ret;
}



static unsigned char bma222e_read_byte(struct i2c_client *client,unsigned char reg)
{
	int i = 0;
	int ret = 0;

	for(i=0; i<3; i++)
	{
		ret = i2c_smbus_read_byte_data(client, reg);
		if (ret < 0)
		{
			dev_err(&client->dev, "\e[31mfailed read_byte reg:0x%02x, cnt:%d\e[0m\n", reg, i);
			msleep(1);
		}
		else
		{
			break;
		}
	}
	return (unsigned char)ret;
}

static unsigned char bma222e_read_byte2(struct i2c_client *client,unsigned char reg)
{
	int i = 0;
	int ret = 0;

	for(i=0; i<3; i++)
	{
		ret = i2c_smbus_read_byte_data(client, reg);
		if (ret < 0)
		{
			dev_err(&client->dev, "\e[31mfailed read_byte reg:0x%02x, cnt:%d\e[0m\n", reg, i);
			msleep(1);
		}
		else
		{
			break;
		}
	}
	return ret;
}

static int bma222e_set_reg_bits(struct i2c_client *client, int val, int shift, u8 mask, u8 reg)
{
	int data;

	data = bma222e_read_byte(client, reg);
	if (data < 0)
		return data;

	data = (data & ~mask) | ((val << shift) & mask);
	return bma222e_write_byte(client, reg, data);
}
#else
unsigned char bma222e_read_byte(struct i2c_client *client, u8 subaddr)
{
	int err;
	unsigned char buf;
	struct i2c_msg msg[2];

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &subaddr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &buf;

	err = i2c_transfer(client->adapter, msg, 2);

	if (unlikely(err != 2)) 
	{
		dev_err(&client->dev, "\e[31mread_word failed reg:0x%02x \e[0m\n", subaddr);
		return 0;
	}

	return buf;
}

static int bma222e_write_byte(struct i2c_client *client, u8 addr, u8 w_data)
{
	int i = 0;
	int ret = 0;
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 2, buf};

	buf[0] = addr;
	buf[1] = w_data >> 8;
	buf[2] = w_data & 0xff;

	for(i=0; i<3; i++)
	{
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		mdelay(50);
		//dev_err(&client->dev, "\e[31mfailed write_word reg:0x%02x write:0x%04x, retry:%d\e[0m\n", addr, w_data, i);
	}

	if (ret != 1) {
		//dev_err(&client->dev, "%s: I2C is not working.\n", __func__);
		dev_err(&client->dev, "\e[31mfailed write_word reg:0x%02x write:0x%04x, retry:%d\e[0m\n", addr, w_data, i);
		return -EIO;
	}

	return 0;
}

#endif

 static int bma222_read_accel_xyz(struct i2c_client *client,
		struct bmasensoracc *acc)
{
	signed char data;
	unsigned char new_data;


	new_data = bma222e_read_byte(client,BMA222_NEWX_AXIS_DATA_REG);
	if(new_data & 0x01)
	{
		data = (signed char)bma222e_read_byte(client,BMA222_ACC_X__REG);
		acc->x = (short)data;
	}
	new_data = bma222e_read_byte(client,BMA222_NEWY_AXIS_DATA_REG);
	if(new_data & 0x01)
	{
		data = (signed char)bma222e_read_byte(client,BMA222_ACC_Y__REG);
		acc->y = (short)data;
	}
	new_data = bma222e_read_byte(client,BMA222_NEWZ_AXIS_DATA_REG);
	if(new_data & 0x01)
	{
		data = (signed char)bma222e_read_byte(client,BMA222_ACC_Z__REG);
		acc->z = (short)data;
	}

	sensor_dbg_d(KERN_INFO "%s %d %d %d \n", __FUNCTION__,acc->x,acc->y,acc->z);

	return 0;
}

int bma222_set_range(struct i2c_client *client,char range) 
{			
	unsigned char data;
	int newRange = 0;

	if (range < 4) {
		data = bma222e_read_byte(client,BMA222_RANGE__REG);
		switch (range) {
			case 0:
				newRange  = BMA222_RANGE_2G;
				break;
			case 1:
				newRange  = BMA222_RANGE_4G;
				break;
			case 2:
				newRange  = BMA222_RANGE_8G;
				break;
			case 3:
				newRange  = BMA222_RANGE_16G;
				break;			
			default:
				break;
		}
		data = BMA222_SET_BITSLICE(data, BMA222_RANGE, newRange);
		bma222e_write_byte(client,BMA222_RANGE__REG, data);
	} else{
		return -1;
	}

	return 0;
}

int bma222_set_bandwidth(struct i2c_client *client,char bw) 
{
	unsigned char data;
	int Bandwidth = 0;

	if (bw < 6) {
		switch (bw) {
			case 0:
				Bandwidth = BMA222_BANDWIDTH_31HZ;
				break;
			case 1:
				Bandwidth = BMA222_BANDWIDTH_62HZ;
				break;
			case 2:
				Bandwidth = BMA222_BANDWIDTH_125HZ;
				break;
			case 3:
				Bandwidth = BMA222_BANDWIDTH_250HZ;
				break;
			case 4:
				Bandwidth = BMA222_BANDWIDTH_500HZ;
				break;
			case 5:
				Bandwidth = BMA222_BANDWIDTH_1000HZ;
				break;		
			default:
				break;
		}
		data = bma222e_read_byte(client,BMA222_SC_FILT_CONFIG__REG);

		data = BMA222_SET_BITSLICE(data, BMA222_SC_FILT_CONFIG, Bandwidth);
		bma222e_write_byte(client,BMA222_SC_FILT_CONFIG__REG, data);
	} else{
		return -1;
	}

	return 0;
}

static int __devinit bma222e_set_low_g_interrupt(struct i2c_client *client, u8 enable, u8 hyst, u8 dur, u8 thres)
{
	int error;

#if 0
	error = bma222e_set_reg_bits(client, hyst, 0x0, 0x3, BMA222_INT_2);
	if (error)
		return error;

	error = bma222e_write_byte(client, BMA222_INT_0, dur);
	if (error)
		return error;

	error = bma222e_write_byte(client, BMA222_INT_1, thres);
	if (error)
		return error;

	return bma222e_set_reg_bits(client, !!enable, 0x0, 0x1, BMA222_INT_MAP_0);
#else
	return error;
#endif
}

static int __devinit bma222e_set_high_g_interrupt(struct i2c_client *client, u8 enable, u8 hyst, u8 dur, u8 thres)
{
	int error;

#if 0
	// (client, 1, 0, 150, 160);
	error = bma222e_set_reg_bits(client, hyst, 0x6, 0xC0, BMA222_INT_2);
	if (error)
		return error;

	error = bma222e_write_byte(client, BMA222_INT_3, dur);
	if (error)
		return error;

	error = bma222e_write_byte(client, BMA222_INT_4, thres);
	if (error)
		return error;

	return bma222e_set_reg_bits(client, !!enable, 0x1, 0x2, BMA222_INT_MAP_0);
#else
	return error;
#endif
}


static int __devinit bma222e_set_any_motion_interrupt(struct i2c_client *client, u8 enable, u8 dur, u8 thres)
{
	int error = 0;

#if 0
	error = bma222e_set_reg_bits(client, dur,
				BMA150_ANY_MOTION_DUR_POS,
				BMA150_ANY_MOTION_DUR_MSK,
				BMA150_ANY_MOTION_DUR_REG);
	if (error)
		return error;

	error = bma222e_write_byte(client,
				BMA150_ANY_MOTION_THRES_REG, thres);
	if (error)
		return error;

	error = bma222e_set_reg_bits(client, !!enable,
				BMA150_ADV_INT_EN_POS, BMA150_ADV_INT_EN_MSK,
				BMA150_ADV_INT_EN_REG);
	if (error)
		return error;

	return bma222e_set_reg_bits(client, !!enable,
				BMA150_ANY_MOTION_EN_POS,
				BMA150_ANY_MOTION_EN_MSK,
				BMA150_ANY_MOTION_EN_REG);
#else
	return error;
#endif
}


static int bma222_read_temperature(struct i2c_client *client,
		signed char *temperature)
{
	return 0;
}

int bma222e_sensor_auto_calibration(struct bmasensor_data *sensor_data)
{
	return 0;
}

int bma222e_sensor_calibration(struct bmasensor_data *sensor_data, short x_offset,short y_offset, short z_offset)
{
	int calibMode = atomic_read(&sensor_data->calibMode);
	struct bmasensoracc calib; // calibOffset

#if defined(CONFIG_TCC_BB)
	calib.x = x_offset;
	calib.y = y_offset;
	calib.z = z_offset;
#else	
	calib.x = (BMA_1G_RESOLUTIOIN * x_offset)/100;
	calib.y = (BMA_1G_RESOLUTIOIN * y_offset)/100;
	calib.z = (BMA_1G_RESOLUTIOIN * z_offset)/100;
#endif

	sensor_dbg_d("%s: A %d %d %d\n", __func__, calib.x,calib.y,calib.z);

	atomic_set(&sensor_data->calibOffset.x,calib.x);
	atomic_set(&sensor_data->calibOffset.y,calib.y);
	atomic_set(&sensor_data->calibOffset.z,calib.z);

	if(calibMode == CALIBRATION_HW_MANUAL){
		#ifdef SENSOR_TUNING_BIT_WIDTH_EXTEND
		calib.x= calib.x >> BMA_DATA_TUNING_WIDTH;
		calib.y= calib.y >> BMA_DATA_TUNING_WIDTH;
		calib.z= calib.z >> BMA_DATA_TUNING_WIDTH;
		#endif
		bma222e_sensor_convertCoordination(&calib, &gsenlayout[LAYOUT_HAL2CHIP]);

		sensor_dbg_d("%s: B %d %d %d\n", __func__, calib.x,calib.y,calib.z);
	}

	return 0;
}

#if defined(CONFIG_TCC_BB) && defined(EVENT_TUNING)
static int bma222e_sensor_read_accel_xyz_tuning(struct i2c_client *client,
		struct bmasensoracc *acc, unsigned int thres)
{
	int ret = 0;
	struct bmasensoracc acc_tmp;

	bma222_read_accel_xyz(client, acc);
	bma222_read_accel_xyz(client, &acc_tmp);

	if (abs(acc->x - acc_tmp.x) > thres)
		ret |= 0x1;
	else if (abs(acc->y - acc_tmp.y) > thres)
		ret |= 0x10;
	else if (abs(acc->z - acc_tmp.z) > thres)
		ret |= 0x100;

	return ret;
}
#else
static int bma222e_sensor_read_accel_xyz(struct i2c_client *client,
		struct bmasensoracc *acc)
{
	// TODO : get the accelerometer data form chip
	return bma222_read_accel_xyz(client, acc);
}
#endif

static int bma222e_sensor_read_temperature(struct i2c_client *client,
		signed char *temperature)
{
	// TODO : get the accelerometer data form chip
	return bma222_read_temperature(client, temperature);
}


static int __devinit bma222e_soft_reset(struct i2c_client *client)
{
	int error;

	error = bma222e_write_byte(client, BMA222_BGW_SOFTRESET, 0xB6);
	if (error)
		return error;

	msleep(2);
	return 0;
}

static int bma222e_sensor_chip_init(struct bmasensor_data *data, struct i2c_client *client)
{
	int err = 0;
	unsigned char tempvalue;
	struct bmasensoracc acc={0,};

	/* read chip id */
	tempvalue = bma222e_read_byte(client,SENSOR_CHIP_ID_REG);


	if (tempvalue == 0xF8) {
		printk(KERN_INFO "Bosch Sensortec Device detected!\n" 
				"BMA222 registered I2C driver!\n");
	} else{
		printk(KERN_INFO "Bosch Sensortec Device not found"
				"i2c error %d \n", tempvalue);
		err = -ENODEV;
		return err;
	}

	err = bma222e_soft_reset(client);
	if (err)
		return -EIO;


//	tempvalue = bma222e_read_byte(client,0x01);	
#if defined(CONFIG_TCC_BB)
        bma222_set_range(client,0);  //BMA222_RANGE_2G
        bma222_set_bandwidth(client,1); //BMA222_BANDWIDTH_62HZ
#else
       err = bma222_set_bandwidth(client, SENSOR_DEFAULT_BW);		
	//if (err)
	//	return err;

       err = bma222_set_range(client, SENSOR_DEFAULT_RANGE);
	//if (err)
	//	return err;

	if(0)//client->irq) 
	{
		err = bma222e_set_any_motion_interrupt(client, 1, 0, 0);
		//if (err)
		//	return err;

		err = bma222e_set_high_g_interrupt(client, 1, 0, 150, 160);
		//if (err)
		//	return err;

		err = bma222e_set_low_g_interrupt(client, 1, 0, 150, 20);
		//if (err)
		//	return err;
	}
#endif

	//acc.x = acc.y = acc.z = 0;
	//atomic_set(&data->calibMode, CALIBRATION_SOFT);
	//bma222e_sensor_calibration(data, (short)acc.x, (short)acc.y, (short)acc.z);
	//printk("%s: calibration offset (%d, %d, %d)\n", __func__, 
	//								atomic_read(&data->calibOffset.x), 
	//								atomic_read(&data->calibOffset.y), 
	//								atomic_read(&data->calibOffset.z));
	//msleep(1);
	// TODO : get the accelerometer data form chip
	//bma222_read_accel_xyz(client, &acc);

	return 0;
}


static ssize_t bma222e_sensor_attr_autocalibration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", (atomic_read(&sensor_data->calibMode) == CALIBRATION_HW_AUTO)? 1:0);
}

static ssize_t bma222e_sensor_attr_autocalibration_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	bma222e_sensor_auto_calibration(sensor_data);

	return count;
}

static ssize_t bma222e_sensor_attr_calibration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", 0);
}

static ssize_t bma222e_sensor_attr_calibration_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	long data1,data2,data3;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	sensor_dbg(KERN_INFO "%s  count = %d\n", __FUNCTION__,count);	
	//error = strict_strtol(buf, 10, &data);
	error = sscanf(buf, "%ld %ld %ld\n",&data1,&data2,&data3);
	sensor_dbg(KERN_INFO "%s : data = %d %d %d error = %d \n", __FUNCTION__,data1,data2,data3,error);	
	if (error != 3)
		return error;

	bma222e_sensor_calibration(sensor_data,data1,data2,data3);

	sensor_dbg(KERN_INFO "%s end\n", __FUNCTION__);	

	return count;
}

static ssize_t bma222e_sensor_attr_resolution_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&sensor_data->resolution));

}

static ssize_t bma222e_sensor_attr_resolution_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	sensor_dbg(KERN_INFO "%s : data = %d error = %d \n", __FUNCTION__,data,error);	
	if (error)
		return error;
	if (data > BMA_MAX_DELAY)
		data = BMA_MAX_DELAY;
	atomic_set(&sensor_data->resolution, (unsigned int) data);

	return count;
}

static ssize_t bma222e_sensor_attr_inputdevice_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&sensor_data->inputDevice));

}

static ssize_t bma222e_sensor_attr_inputdevice_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	sensor_dbg(KERN_INFO "%s : data = %d error = %d \n", __FUNCTION__,data,error);	
	if (error)
		return error;
	if (data > BMA_MAX_DELAY)
		data = BMA_MAX_DELAY;
	atomic_set(&sensor_data->inputDevice, (unsigned int) data);

	return count;
}

static ssize_t bma222e_sensor_attr_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&sensor_data->delay));

}

static ssize_t bma222e_sensor_attr_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	sensor_dbg(KERN_INFO "%s : data = %d error = %d \n", __FUNCTION__,data,error);	
	if (error)
		return error;
	if (data > BMA_MAX_DELAY)
		data = BMA_MAX_DELAY;
	atomic_set(&sensor_data->delay, (unsigned int) data);
#ifdef SENSOR_TUNING
	atomic_set(&sensor_data->realDelay, (unsigned int) data/SENSOR_TUNING_DIVIDE);
	atomic_set(&sensor_data->realDelayCnt, 0);
	atomic_set(&sensor_data->realDelayTriggerCnt, SENSOR_TUNING_DIVIDE);
#endif
	return count;
}

static ssize_t bma222e_sensor_attr_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&sensor_data->enable));

}

static void bma222e_sensor_attr_set_enable_by_client(struct bmasensor_data *sensor_data, int enable)
{
	int pre_enable = atomic_read(&sensor_data->enable);
	int what = ((enable&SENSOR_T_MASK) != 0)? 2:1;
	int enableDisable = enable&SENSOR_ENABLE;

	sensor_dbg(KERN_INFO "%s : enable = %d \n", __FUNCTION__,enable);	

	mutex_lock(&sensor_data->enable_mutex);
	
	if (enableDisable) {
		if (pre_enable ==0) {
			#ifdef SENSOR_TUNING
			schedule_delayed_work(&sensor_data->work,
					msecs_to_jiffies(atomic_read(&sensor_data->realDelay)));
			#else			
			schedule_delayed_work(&sensor_data->work,
					msecs_to_jiffies(atomic_read(&sensor_data->delay)));
			#endif
		}
		pre_enable |= what;
		atomic_set(&sensor_data->enable, pre_enable);
	} else {
		pre_enable = pre_enable&(~what);
		if (pre_enable ==0) {
			// cancel_delayed_work_sync(&sensor_data->work);
		}
		atomic_set(&sensor_data->enable, pre_enable);
	}
	//printk(" ~~~~set enable = 0x%x enableDisable=0x%x pre_enable =0x%x\n",enable,enableDisable,pre_enable);
	mutex_unlock(&sensor_data->enable_mutex);

}

static void bma222e_sensor_attr_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);
	bma222e_sensor_attr_set_enable_by_client(sensor_data,enable);
}

static ssize_t bma222e_sensor_attr_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);

	sensor_dbg(KERN_INFO "%s : data = %d error = %d  \n", __FUNCTION__,data,error);	
	
	if (error)
		return error;
	if ((data >= 0) && (data<4)) {
		bma222e_sensor_attr_set_enable(dev,data);
	}

	return count;
}

static ssize_t bma222e_sensor_attr_halname_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", SENSOR_HAL_NAME);
}

static ssize_t bma222e_sensor_attr_halname_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	return 0;
}

static ssize_t bma222e_sensor_attr_temperature_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d %d %d\n", SENSOR_TEMPERATURE_SUPPORT, 
		SENSOR_TEMPERATURE_CENTER,SENSOR_TEMPERATURE_RESOLUTION);
	//return sprintf(buf, "%d %d %d\n", 1,24,2);
}

static ssize_t bma222e_sensor_attr_temperature_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	return 0;
}

static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_delay_show, bma222e_sensor_attr_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_enable_show, bma222e_sensor_attr_enable_store);
static DEVICE_ATTR(resolution, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_resolution_show, bma222e_sensor_attr_resolution_store);
static DEVICE_ATTR(inputdevice, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_inputdevice_show, bma222e_sensor_attr_inputdevice_store);
static DEVICE_ATTR(calibration, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_calibration_show, bma222e_sensor_attr_calibration_store);
static DEVICE_ATTR(autocalibration, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_autocalibration_show, bma222e_sensor_attr_autocalibration_store);
static DEVICE_ATTR(halname, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_halname_show, bma222e_sensor_attr_halname_store);
static DEVICE_ATTR(temperature, S_IRUGO|S_IWUSR|S_IWGRP,
		bma222e_sensor_attr_temperature_show, bma222e_sensor_attr_temperature_store);
	
static struct attribute *bma222e_sensor_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_resolution.attr,
	&dev_attr_inputdevice.attr,
	&dev_attr_calibration.attr,
	&dev_attr_autocalibration.attr,
	&dev_attr_halname.attr,
	&dev_attr_temperature.attr,
	NULL
};

static struct attribute_group bma222e_sensor_attribute_group = {
	.attrs = bma222e_sensor_attributes
};

#if 0
static int bma222e_register(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;

    memset(&info, 0, sizeof(struct i2c_board_info));
    info.addr = SENSOR_I2C_ADDRESS;
    strlcpy(info.type, BMA222E_NAME, I2C_NAME_SIZE);

    sensor_dbg(KERN_INFO "%s \n",__FUNCTION__);
    	  
#if defined(CONFIG_ARCH_TCC93XX)
    adapter = i2c_get_adapter(3);
#else
    if(machine_is_m57te()){
        adapter = i2c_get_adapter(1);
    }
    else if(machine_is_m801())  // 89_froyo
    {
        adapter = i2c_get_adapter(0);
    }
    else{
	#if defined(CONFIG_TCC_BB)
//	adapter = i2c_get_adapter(2);	// I2C_18      //chage for TNN, 130319, collition with camera
//        sensor_dbg(KERN_INFO "%s : i2c_get_adapter(2)\n", __func__);

        sensor_dbg(KERN_INFO "%s : i2c_get_adapter(0)\n", __FUNCTION__);
        adapter = i2c_get_adapter(0);

	#else
        sensor_dbg(KERN_INFO "%s : i2c_get_adapter(0)\n", __FUNCTION__);
        adapter = i2c_get_adapter(0);
	#endif
    }
#endif
    if (!adapter) 
    {
        sensor_dbg(KERN_ERR "can't get i2c adapter 0 for tcc-accel-sensor\n");
        return -ENODEV;
    }

    client = i2c_new_device(adapter, &info);
    i2c_put_adapter(adapter);
    if (!client) 
    {
        sensor_dbg(KERN_ERR "can't add i2c device at 0x%x\n", (unsigned int)info.addr);
        return -ENODEV;
    }

    return 0;
}
#endif

static void bma222e_sensor_convertCoordination(struct bmasensoracc *sensor_accel, matrix3by3 *layout)
{
    short x,y,z;
    if(sensor_accel == NULL)
        return;
    x = sensor_accel->x;
    y = sensor_accel->y;
    z = sensor_accel->z;

    sensor_accel->x = x*layout->_11 + y*layout->_12 + z*layout->_13;
    sensor_accel->y = x*layout->_21 + y*layout->_22 + z*layout->_23;
    sensor_accel->z = x*layout->_31 + y*layout->_32 + z*layout->_33;	
}

static void bma222e_sensor_compensation(struct bmasensor_data *sensor_data,struct bmasensoracc *accelData)
{
	if(atomic_read(&sensor_data->calibMode) == CALIBRATION_SOFT){
printk("CALIBRATION_SOFT\n");
		accelData->x += (short)atomic_read(&sensor_data->calibOffset.x);
		accelData->y += (short)atomic_read(&sensor_data->calibOffset.y);
		accelData->z += (short)atomic_read(&sensor_data->calibOffset.z);
	}
}

#if defined(CONFIG_TCC_BB)
static void bma222e_sensor_event(struct bmasensor_data *sensor_data)
{
	unsigned int event = 0;
#if defined(CONFIG_GSEN_DTYPE_DELTA)
	if (abs(sbuf[idx].x) > sensor_data->event_thres) {
		event = 1;
	} else if (abs(sbuf[idx].y) > sensor_data->event_thres) {
		event = 1;
	} else if (abs(sbuf[idx].z) > sensor_data->event_thres) {
		event = 1;
	}
#else
	#if 0
	int old, new;

	if (idx) {
		new = idx;
		old = idx - 1;
	} else {
		new = idx;
		old = MAXEVENTS - 1;
	}

	if (abs(sbuf[new].x - sbuf[old].x) > sensor_data->event_thres) {
		event = 1;
	} else if (abs(sbuf[new].y - sbuf[old].y) > sensor_data->event_thres) {
		event = 1;
	} else if (abs(sbuf[new].z - sbuf[old].z) > sensor_data->event_thres) {
		event = 1;
	}
	#else
	event = bma222e_sensor_event_ex(sbuf[idx].x, sbuf[idx].y,
		((unsigned int)sbuf[idx].timestamp.tv_sec) * 1000 + ((unsigned int)sbuf[idx].timestamp.tv_usec) / 1000);
	#endif
#endif

	if (event && sensor_data->flag) {
		atomic_inc(&sensor_data->count);
		wake_up_interruptible(&(sensor_data->wq));
		//printk("[%s] detect +-%d (acc-x: %d, acc-y: %d, acc-z: %d\n", 
		//	__func__, sensor_data->event_thres, sbuf[idx].x, sbuf[idx].y, sbuf[idx].z);
	}

	sensor_data->flag = 1;
}

static unsigned int bma222e_sensor_poll(struct file *filp, poll_table *wait)
{
	struct bmasensor_data *sensor_data = (struct bmasensor_data *)filp->private_data;

	if (sensor_data == NULL)
		return -EFAULT;

	if (atomic_read(&sensor_data->count) > 0) {
		atomic_dec(&sensor_data->count);
		return (POLLIN | POLLRDNORM);
	}

	poll_wait(filp, &(sensor_data->wq), wait);

	if (atomic_read(&sensor_data->count) > 0) {
		atomic_dec(&sensor_data->count);
		return (POLLIN | POLLRDNORM);
	} else {
		return 0;
	}
}
#endif

static void bma222e_sensor_set_data(struct bmasensor_data *sensor_data, struct bmasensoracc *raw_accelData, struct bmasensoracc *accelData)
{
	short x,y,z;
	struct bmasensoracc tmpData;

	x = raw_accelData->x;
	y = raw_accelData->y;
	z = raw_accelData->z;

	tmpData.x = x;
	tmpData.y = y;
	tmpData.z = z;

#if 0
	bma222e_sensor_convertCoordination(&tmpData, &gsenlayout[LAYOUT_CHIP2HAL]);

	bma222e_sensor_compensation(sensor_data,&tmpData);
#endif

	accelData->x = tmpData.x;
	accelData->y = tmpData.y;
	accelData->z = tmpData.z;
}

#ifdef SENSOR_TUNING
static void bma222e_sensor_avg_data(struct bmasensoracc *avgAcc,struct bmasensoracc *acc)
{
	int data[3];
	int readData[3];
	int avgData[3];
	int alpha = 100/SENSOR_TUNING_DIVIDE;

	readData[0] = (int)acc->x;
	readData[1] = (int)acc->y;
	readData[2] = (int)acc->z;

	#ifdef SENSOR_TUNING_BIT_WIDTH_EXTEND
	readData[0] = readData[0]<<BMA_DATA_TUNING_WIDTH;
	readData[1] = readData[1]<<BMA_DATA_TUNING_WIDTH;
	readData[2] = readData[2]<<BMA_DATA_TUNING_WIDTH;
	#endif

	avgData[0] = (int)avgAcc->x;
	avgData[1] = (int)avgAcc->y;
	avgData[2] = (int)avgAcc->z;	
	
	data[0] = ((int)(avgData[0] * (100 - alpha)) + (int)(readData[0] * (alpha)))/100;
	data[1] = ((int)(avgData[1] * (100 - alpha)) + (int)(readData[1] * (alpha)))/100;
	data[2] = ((int)(avgData[2]* (100 - alpha)) + (int)(readData[2] * (alpha)))/100;
	avgAcc->x = (short)data[0];
	avgAcc->y = (short)data[1];
	avgAcc->z = (short)data[2];
	acc->x = (short)data[0];
	acc->y = (short)data[1];
	acc->z = (short)data[2];
}
#endif

static struct bmasensoracc raw_acc;
static void bma222e_sensor_work_func(struct work_struct *work)
{
	int pre_enable;
	int usedAsInputDevice = 0;

	struct bmasensor_data *sensor_data = container_of((struct delayed_work *)work,
			struct bmasensor_data, work);
	static struct bmasensoracc acc;
	#ifdef SENSOR_TUNING
	struct bmasensoracc *avgAcc = &sensor_data->avgValue;
	unsigned long delay = msecs_to_jiffies(atomic_read(&sensor_data->realDelay));
	int checkCnt = atomic_read(&sensor_data->realDelayCnt);
	int triggerCnt = atomic_read(&sensor_data->realDelayTriggerCnt); 
	#else	
	unsigned long delay = msecs_to_jiffies(atomic_read(&sensor_data->delay));
	#endif			
	struct i2c_client *client = sensor_data->i2cClient;
	int sensorWhat = 0;

	mutex_lock(&sensor_data->enable_mutex);
	sensorWhat = atomic_read(&sensor_data->enable);
	mutex_unlock(&sensor_data->enable_mutex);

	sensor_dbg_d("%s sensorWhat(%x)\n", __func__, sensorWhat);
	if(sensorWhat & SENSOR_G_MASK){
#if defined(CONFIG_TCC_BB) && defined(EVENT_TUNING)
		if (bma222e_sensor_read_accel_xyz_tuning(client, &raw_acc, sensor_data->event_thres)) {
			sensor_dbg("abnormal event occur, so skip\n");
			goto set_schedule;
		}
#else
		bma222e_sensor_read_accel_xyz(client, &raw_acc);
#endif

		bma222e_sensor_set_data(sensor_data, &raw_acc, &acc);

	#ifdef SENSOR_TUNING
		if(delay == 0)
			delay = 1;
		bma222e_sensor_avg_data(avgAcc,&acc);
		checkCnt++;
		atomic_set(&sensor_data->realDelayCnt, checkCnt);
		if(checkCnt < triggerCnt){
			goto set_schedule;
		}
		atomic_set(&sensor_data->realDelayCnt, 0);
	#endif

#if defined(CONFIG_TCC_BB)
		/* 
		 * store buffer 
		 */
		idx = (idx == MAXEVENTS) ? 0 : idx;
	#if defined(CONFIG_GSEN_DTYPE_DELTA)
		sbuf[idx].x = acc.x - sensor_data->prev.x;
		sbuf[idx].y = acc.y - sensor_data->prev.y;
		sbuf[idx].z = acc.z - sensor_data->prev.z;
		sensor_data->prev.x = acc.x;
		sensor_data->prev.y = acc.y;
		sensor_data->prev.z = acc.z;
	#else
		sbuf[idx].x = acc.x;
		sbuf[idx].y = acc.y;
		sbuf[idx].z = acc.z;
	#endif
		do_gettimeofday(&(sbuf[idx].timestamp));
		/* 
		 * check event threshold 
		 */
		bma222e_sensor_event(sensor_data);
		idx++;
#endif
	}

	sensor_dbg(KERN_INFO "%s %d %d %d \n", __FUNCTION__,acc.x,acc.y,acc.z);
	
	usedAsInputDevice = atomic_read(&sensor_data->inputDevice);

	//if(SENSOR_TEMPERATURE_SUPPORT == SENSOR_SUPPORT)
	if(sensorWhat & SENSOR_T_MASK)
	{
		signed char temperature = 0;		
		bma222e_sensor_read_temperature(client, &temperature);		
		input_report_abs(sensor_data->input, ABS_THROTTLE, (int)temperature);
	}
	
       if(usedAsInputDevice && (sensorWhat & SENSOR_G_MASK))
       {
	    input_report_abs(sensor_data->input, ABS_X, acc.x);
	    input_report_abs(sensor_data->input, ABS_Y, acc.y);
	    input_report_abs(sensor_data->input, ABS_Z, acc.z);
       }

	if((usedAsInputDevice && (sensorWhat & SENSOR_G_MASK)) || (sensorWhat & SENSOR_T_MASK))
	{
	    input_sync(sensor_data->input);
	}

	mutex_lock(&sensor_data->value_mutex);
	sensor_data->value = acc;
	mutex_unlock(&sensor_data->value_mutex);
#ifdef SENSOR_TUNING	
set_schedule:
#endif
#if defined(CONFIG_TCC_BB) && defined(EVENT_TUNING)
set_schedule:
#endif
	mutex_lock(&sensor_data->enable_mutex);
	pre_enable = atomic_read(&sensor_data->enable);
	if(pre_enable != 0){
		schedule_delayed_work(&sensor_data->work, delay);
	}
	mutex_unlock(&sensor_data->enable_mutex);
}


static irqreturn_t bma222e_irq_thread(int irq, void *dev)
{
	struct bmasensor_data *data = dev;

	schedule_delayed_work(&data->work, msecs_to_jiffies(100));

	return IRQ_HANDLED;
}


static int sensor_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	int tempvalue;
	struct bmasensor_data *data;
	struct input_dev *dev;

	sensor_dbg(KERN_INFO "%s  :  %s \n", __func__,client->name);

	data = kzalloc(sizeof(struct bmasensor_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	atomic_set(&data->resolution, BMA_1G_RESOLUTIOIN);
	atomic_set(&data->calibOffset.x, 0);
	atomic_set(&data->calibOffset.y, 0);
	atomic_set(&data->calibOffset.z, 0);
	atomic_set(&data->inputDevice, SENSOR_DEF_DEVICE_TYPE);
	atomic_set(&data->calibMode,SENSOR_CALIBRATION_MODE);

	data->i2cClient = client;
	i2c_set_clientdata(client, data);
	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	mutex_init(&data->suspend_mutex);

	mData = NULL;	

	INIT_DELAYED_WORK(&data->work, bma222e_sensor_work_func);
	atomic_set(&data->delay, BMA_MAX_DELAY);
	#ifdef SENSOR_TUNING
	atomic_set(&data->realDelay, BMA_MAX_DELAY/SENSOR_TUNING_DIVIDE);
	atomic_set(&data->realDelayCnt, 0);
	atomic_set(&data->realDelayTriggerCnt, SENSOR_TUNING_DIVIDE);
	#endif
	atomic_set(&data->enable, 0);
	atomic_set(&data->suspend, 0);

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = BMA222E_NAME;
	dev->id.bustype = BUS_I2C;

	set_bit(EV_ABS, dev->evbit);
	input_set_abs_params(dev, ABS_X, ABSMIN, ABSMAX, 0, 0);
	input_set_abs_params(dev, ABS_Y, ABSMIN, ABSMAX, 0, 0);
	input_set_abs_params(dev, ABS_Z, ABSMIN, ABSMAX, 0, 0);
	input_set_abs_params(dev, ABS_THROTTLE, -128, 128, 0, 0);

	input_set_drvdata(dev, data);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		goto kfree_exit;
	}

	data->input = dev;

	err = sysfs_create_group(&data->input->dev.kobj, &bma222e_sensor_attribute_group);
	if (err < 0)
		goto error_sysfs;

	tempvalue = bma222e_sensor_chip_init(data, data->i2cClient);
	if( tempvalue < 0){
		printk(KERN_INFO "sensor_init fail %d \n", tempvalue);
		// err = -ENODEV;
		//goto kfree_exit;
	}
	else
		mData = (struct bmasensor_data *)input_get_drvdata(dev);

#if 0
	err = request_threaded_irq(client->irq, NULL, 
							bma222e_irq_thread, 
							IRQF_TRIGGER_RISING | IRQF_ONESHOT, 
							BMA222E_NAME, data);
	if (err) {
		dev_err(&client->dev, "irq request failed %d, error %d\n", client->irq, err);
		goto error_sysfs;
	}

	// schedule_delayed_work(&data->work, msecs_to_jiffies(100));
#endif

	return 0;

error_sysfs:
	input_unregister_device(data->input);

kfree_exit:
	kfree(data);
exit:
	return err;
}

static int sensor_i2c_remove(struct i2c_client *client)
{
    struct bmasensor_data *data  = i2c_get_clientdata(client);
    sysfs_remove_group(&data->input->dev.kobj, &bma222e_sensor_attribute_group);
    input_unregister_device(data->input);
    kfree(data);
    mData = NULL;

    return 0;
}

#if 0
#ifdef CONFIG_PM
static int sensor_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);
	int enableFlag = 0;

	mutex_lock(&sensor_data->enable_mutex);
	enableFlag = atomic_read(&sensor_data->enable);
	if (enableFlag !=0) {
		//cancel_delayed_work_sync(&sensor_data->work);
		mutex_lock(&sensor_data->suspend_mutex);
		atomic_set(&sensor_data->suspend,enableFlag);
		atomic_set(&sensor_data->enable,0);
		mutex_unlock(&sensor_data->suspend_mutex);
	}
	mutex_unlock(&sensor_data->enable_mutex);

	return 0;
}

static int sensor_i2c_resume(struct i2c_client *client)
{
	struct bmasensor_data *sensor_data = i2c_get_clientdata(client);
	int suspendFlag = 0;
	mutex_lock(&sensor_data->suspend_mutex);
	suspendFlag = atomic_read(&sensor_data->suspend);
	if (suspendFlag!=0) {
	mutex_lock(&sensor_data->enable_mutex);
		atomic_set(&sensor_data->enable,suspendFlag);
		mutex_unlock(&sensor_data->enable_mutex);
		#ifdef SENSOR_TUNING
		schedule_delayed_work(&sensor_data->work,
				msecs_to_jiffies(atomic_read(&sensor_data->realDelay)));	
		#else
		schedule_delayed_work(&sensor_data->work,
				msecs_to_jiffies(atomic_read(&sensor_data->delay)));
		#endif
	}
	mutex_unlock(&sensor_data->suspend_mutex);

	return 0;
}

#else
#define sensor_i2c_suspend		NULL
#define sensor_i2c_resume		NULL
#endif /* CONFIG_PM */
#endif


static const struct i2c_device_id sensor_i2c_id[] = {
    { BMA222E_NAME, 0, },
    { }
};


MODULE_DEVICE_TABLE(i2c, sensor_i2c_id);

/* bmaxxx i2c control layer */
static struct i2c_driver bma222e_driver = {
    .driver = {
	.owner	= THIS_MODULE,
	.name	= BMA222E_NAME,
    },
    .probe      = sensor_i2c_probe,
    .remove     = sensor_i2c_remove,
//    .suspend   = sensor_i2c_suspend,
//    .resume   = sensor_i2c_resume,
    .id_table   = sensor_i2c_id,
};
static ssize_t bma222e_sensor_write(struct file *file, const char __user *user, size_t size, loff_t *o)
{
    sensor_dbg("%s\n", __func__);
    return 0;
 }

static ssize_t bma222e_sensor_read(struct file *file, char __user *user, size_t size, loff_t *o)
{
#if defined(CONFIG_TCC_BB)
        int n_event = idx;
        if (copy_to_user(user, sbuf, sizeof(sensor_accel_t) * idx)) {
                return -EFAULT;
        }
        idx = 0;
        return n_event;
#else

    sensor_accel_t tccData;
    sensor_dbg("%s\n", __func__);    

    mutex_lock(&mData->value_mutex);
    tccData.x = mData->value.x;
    tccData.y = mData->value.y;
    tccData.z = mData->value.z;
    mutex_unlock(&mData->value_mutex);

    tccData.delay_time = (int)atomic_read(&mData->delay);
    tccData.resolution = (int)atomic_read(&mData->resolution);	
    sensor_dbg("%s: %d, %d, %d\n", __func__, tccData.x, tccData.y, tccData.z);

    if(copy_to_user(( sensor_accel_t*) user, (const void *)&tccData, sizeof( sensor_accel_t))!=0)
    {
        sensor_dbg("bma222e_gsensor_read error\n");
    }

    return 0;
#endif
}

long bma222e_sensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *) arg;

	sensor_dbg("%s  (0x%x)  \n", __FUNCTION__, cmd);
	
#if 1
	switch (cmd)
	{
		case IOCTL_SENSOR_SET_INPUTDEVICE:
			{
				unsigned int value = 0;
				if(copy_from_user((void *)&value, argp, sizeof(unsigned int))!=0)
				{
					sensor_dbg("copy_from error\n");
				}					
				sensor_dbg(KERN_INFO "%s:  IOCTL_SENSOR_SET_INPUTDEVICE (0x%x) %d \n", __FUNCTION__, cmd, value);
				atomic_set(&mData->inputDevice, value);		
			}
			break;

		case IOCTL_SENSOR_GET_RESOLUTION:
			{
				unsigned int value = 0;
				value = BMA_1G_RESOLUTIOIN;
				if(copy_to_user(argp, (const void *)&value, sizeof(unsigned int))!=0)
				{
					sensor_dbg("copy_to error\n");
				}			
				sensor_dbg(KERN_INFO "%s:  IOCTL_SENSOR_GET_RESOLUTION (0x%x) %d \n", __FUNCTION__, cmd, value);
			}
			break;

		case IOCTL_SENSOR_GET_TEMP_INFO:
			{
				unsigned int value[3];
				value[0] = SENSOR_TEMPERATURE_SUPPORT;
				value[1] = SENSOR_TEMPERATURE_CENTER;
				value[2] = SENSOR_TEMPERATURE_RESOLUTION;

				if(copy_to_user(argp, (const void *)&value[0], sizeof(unsigned int)*3)!=0)
				{
					sensor_dbg("copy_to error\n");
				}			
				sensor_dbg(KERN_INFO "%s:  IOCTL_SENSOR_GET_TEMP_INFO (0x%x) %d %d %d \n", __FUNCTION__, cmd, value[0],value[1],value[2]);				
			}
			break;  

		case IOCTL_SENSOR_GET_DATA_ACCEL:
			{
				sensor_accel_t accel;
				struct bmasensor_data *data = (struct bmasensor_data *)mData;
				accel.x = data->value.x;
				accel.y = data->value.y;
				accel.z = data->value.z;

				if(copy_to_user(argp, (const void *)&accel, sizeof(sensor_accel_t)) != 0) {
					printk("%s: error IOCTL_SENSOR_GET_DATA_ACCEL\n", __func__);
					return -EFAULT;
				}
				sensor_dbg("%s: IOCTL_SENSOR_GET_DATA_ACCEL (%d, %d, %d)\n", __func__, accel.x, accel.y, accel.z);
			}
			break;

		case IOCTL_SENSOR_SET_DELAY_ACCEL:
			{
				unsigned int val;
				struct bmasensor_data *data = (struct bmasensor_data *)mData;
				if(copy_from_user((void *)&val, argp, sizeof(unsigned int)) != 0) {
					printk("%s: error IOCTL_SENSOR_SET_DELAY_ACCEL\n", __func__);
					return -EFAULT;
				}
				atomic_set(&data->delay, val);
				printk("%s:  IOCTL_SENSOR_SET_DELAY_ACCEL (%d) \n", __func__, atomic_read(&data->delay));
			}
			break;

		case IOCTL_SENSOR_SET_CALIB_ACCEL:
			{
				struct bmasensor_data *data = (struct bmasensor_data *)mData;
				sensor_accel_t accel;
				accel.x = 0;
				accel.y = 0;
				accel.z = 0;
				if(copy_from_user((void *)&accel, argp, sizeof(sensor_accel_t)) != 0) 
				{
					printk("%s: error IOCTL_SENSOR_SET_CALIB_ACCEL\n", __func__);
					return -EFAULT;
				}
				atomic_set(&data->calibMode, CALIBRATION_SOFT);
				bma222e_sensor_calibration(data, (short)accel.x, (short)accel.y, (short)accel.z);
				printk("%s: calibration offset (%d, %d, %d)\n", __func__, 
												atomic_read(&data->calibOffset.x), 
												atomic_read(&data->calibOffset.y), 
												atomic_read(&data->calibOffset.z));
			}
			break;

#if 0
		case IOCTL_SENSOR_SET_EVENT_THRES:
			{
				unsigned int val;
				struct bmasensor_data *data = (struct bmasensor_data *)mData;
				if (copy_from_user((void *)&val, argp, sizeof(unsigned int)) != 0) {
					printk("%s: error IOCTL_SENSOR_SET_EVENT_THRES\n", __func__);
					return -EFAULT;
				}
				data->event_thres = val;
				//bma222e_sensor_event_set_thres(val);
				sensor_dbg("%s: IOCTL_SENSOR_SET_EVENT_THRES (%d)\n", __func__, data->event_thres);
			}
			break;

		case IOCTL_SENSOR_GET_SENSOR_TYPE:
			{
				struct bmasensor_data *sdata = (struct bmasensor_data *)mData;
				if (copy_to_user(argp, &sdata->sensor_type, sizeof(sdata->sensor_type)) != 0) {
					printk("%s: error IOCTL_SENSOR_GET_SENSOR_TYPE\n", __func__);
					return -EFAULT;
				}
			}
			break;

#endif

		default:
			sensor_dbg("%s(): unrecognized ioctl (0x%x)\n", __func__, cmd); 
			return -EINVAL;
	}
#endif
	return 0;
}

static int bma222e_sensor_release(struct inode *inode, struct file *filp)
{
    sensor_dbg("%s (%d)\n", __FUNCTION__, sensor_used_count);
    sensor_used_count--;
    if (sensor_used_count < 0) {
        sensor_dbg("sensor: release error (over)\n"); 
        sensor_used_count = 0;
    }

    if (sensor_used_count == 0)
    {
        if(mData != NULL && mData->i2cClient != NULL)
            bma222e_sensor_attr_set_enable_by_client(mData, SENSOR_DISABLE);
        //atomic_set(&mData->inputDevice,DEVICE_TYPE_INPUT);
    }

#if defined(CONFIG_TCC_BB)
	/*
	 * BlackBox's sensor_used_count is only one.
	 */
	{
		struct bmasensor_data *data = (struct bmasensor_data *)filp->private_data;
		atomic_set(&data->calibOffset.x, 0);
		atomic_set(&data->calibOffset.y, 0);
		atomic_set(&data->calibOffset.z, 0);
		kfree(sbuf);
	}
#endif

    return 0;
}

static int bma222e_sensor_open(struct inode *inode, struct file *filp)
{
    sensor_dbg("%s : \n", __FUNCTION__);

    if(mData == NULL)
    {
        printk(KERN_INFO "%s:   mData == NULL(%d)\n", __func__,(mData ==NULL)? 1: 0);
        return -1;
    }

    if(mData->i2cClient == NULL)
    {
        printk(KERN_INFO "%s:   Data.i2cClient == NULL(%d)\n", __func__,(mData->i2cClient == NULL)? 1:0);
        return -1;
    }
    if (sensor_used_count == 0)
    {
        atomic_set(&mData->delay, BMA_DEFAULT_DELAY);
	#ifdef SENSOR_TUNING
	atomic_set(&mData->realDelay, BMA_MAX_DELAY/SENSOR_TUNING_DIVIDE);
	atomic_set(&mData->realDelayCnt, 0);
	atomic_set(&mData->realDelayTriggerCnt, SENSOR_TUNING_DIVIDE);
	#endif		
        bma222e_sensor_attr_set_enable_by_client(mData, SENSOR_ENABLE);
        sensor_used_count++;
    }

#if defined(CONFIG_TCC_BB)
	/*
	 * BlackBox's sensor_used_count is only one.
	 */
//	atomic_set(&mData->delay, READ_DEFAULT_DELAY);
//	atomic_set(&mData->resolution, RESOLUTIOIN_1G);
//	atomic_set(&mData->calibOffset.x, 0);
//	atomic_set(&mData->calibOffset.y, 0);
//	atomic_set(&mData->calibOffset.z, 0);
//	atomic_set(&mData->inputDevice, DEVICE_TYPE_CHAR);
//	atomic_set(&mData->calibMode, CALIBRATION_SOFT);
//	bma222e_sensor_attr_set_enable_by_client(mData, SENSOR_ENABLE);
	filp->private_data = (void *)mData;
	atomic_set(&mData->count, 0);

//	mData->event_thres = 26;	/* ADC 26 is 1.2168G (26 * 0.0468) for mma7660 */
	mData->event_thres = 77;	/* ADC 77 is 1.20351G (77 * 0.01563) for bma222, RANGE 2G */

	init_waitqueue_head(&(mData->wq));
	idx = 0;
	sbuf = kzalloc(sizeof(sensor_accel_t) * MAXEVENTS, GFP_KERNEL);

#if 0
	mData->sensor_type = bma222e_sensor_type();
	bma222e_sensor_event_init();
#endif

	#if BMA_DEBUG
	print_matrix(&gsenlayout[LAYOUT_CHIP2HAL]);
	#endif
#endif

    //atomic_set(&mData->inputDevice,0);
    return 0;
}

struct file_operations bma222e_sensor_fops =
{
    .owner    = THIS_MODULE,
    .open     = bma222e_sensor_open,
    .release  = bma222e_sensor_release,
    .unlocked_ioctl    = bma222e_sensor_ioctl,
    .read     = bma222e_sensor_read,
    .write    = bma222e_sensor_write,	
#if defined(CONFIG_TCC_BB)
	.poll			= bma222e_sensor_poll,
#endif
};

int __init bma222e_sensor_init(void)
{
    int ret,ret1;
    struct class *sensor_class;

    sensor_dbg(KERN_INFO "%s \n", __FUNCTION__);

#if 0
#ifdef CONFIG_I2C
        // Initialize I2C driver for BMA222
        ret = i2c_add_driver(&bma222e_driver);
        if(ret < 0) 
        {
            sensor_dbg("%s() [Error] failed i2c_add_driver() = %d\n", __func__, ret);
            return ret;
        }
        ret = bma222e_bma222e_register();   // call register after ( called probe )
        if(ret < 0) 
        {
            sensor_dbg("%s() [Error] Failed register i2c client driver for bma222, return is %d\n", __func__, ret);
            return ret;
        }
        sensor_dbg("%s: post bmaxx_i2c_register : %x\n", __func__, ret);
#endif
#else
        ret = i2c_add_driver(&bma222e_driver);
#endif

    ret1 = register_chrdev(SENSOR_DEV_MAJOR, SENSOR_DEV_NAME, &bma222e_sensor_fops);
    sensor_dbg("%s: register_chrdev ret1 : %d\n", __func__, ret1);
    if(ret1 >= 0)
    {
        sensor_class = class_create(THIS_MODULE, SENSOR_DEV_NAME);
        device_create(sensor_class,NULL,MKDEV(SENSOR_DEV_MAJOR,SENSOR_DEV_MINOR),NULL,SENSOR_DEV_NAME);
    }

    return ret;
}

void __exit bma222e_sensor_exit(void)
{
	i2c_del_driver(&bma222e_driver);
	unregister_chrdev(SENSOR_DEV_MAJOR, SENSOR_DEV_NAME);
}

module_init(bma222e_sensor_init);
module_exit(bma222e_sensor_exit);

MODULE_AUTHOR("  linux@ ");
MODULE_DESCRIPTION("BMA222E 3-Axis Orientation/Motion Detection Sensor driver");
MODULE_LICENSE("GPL");

