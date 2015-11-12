#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/uaccess.h>

#include <linux/kernel.h>

typedef signed char		VINT8;
typedef unsigned char	VUINT8;
typedef signed short	VINT16;
typedef unsigned short	VUINT16;
typedef unsigned long	VUINT32;
typedef signed long		VINT32;

#define _MMA7660_LOW_PASS_FILTER_
#define LowPassFactor 3
#define RawDataLength 4

#define MMA7660_Sin30pos 10
#define MMA7660_Sin30neg -10

VINT8 Xraw[RawDataLength];
VINT8 Yraw[RawDataLength];
VINT8 Zraw[RawDataLength];

VINT8 RawDataPointer = 0;

VINT8 Xnew8, Ynew8, Znew8;
VINT8 Xavg8, Yavg8, Zavg8;
#ifdef _MMA7660_LOW_PASS_FILTER_
VINT8 Xflt8, Yflt8, Zflt8;
VINT8 Xrsdl, Yrsdl, Zrsdl;
#endif

void XYZ_Filter(VINT8 *X, VINT8 *Y, VINT8 *Z);

void XYZ_Filter(VINT8 *X, VINT8 *Y, VINT8 *Z)
{
	VUINT8 i;
	VINT8 temp8_1, temp8_2;
	VINT16 temp16;

	Xnew8 = *X;
	Ynew8 = *Y;
	Znew8 = *Z;

	//printk(KERN_ERR "filting...\n");

#ifdef _MMA7660_LOW_PASS_FILTER_

	if((++RawDataPointer)>=RawDataLength) RawDataPointer = 0;

	Xraw[RawDataPointer] = Xnew8;
	Yraw[RawDataPointer] = Ynew8;
	Zraw[RawDataPointer] = Znew8;

	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Xraw[i];
	}
	Xflt8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Yraw[i];
	}
	Yflt8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Zraw[i];
	}
	Zflt8 = (VINT8)(temp16/RawDataLength);

	temp8_1 = Xflt8 - Xavg8;
	temp8_2 = temp8_1 / (1 + LowPassFactor);
	Xavg8 += temp8_2;
	temp8_1 -= temp8_2 * (1 + LowPassFactor);   //Current Residual
	Xrsdl += temp8_1;                           //Overall Residual
	temp8_2 = Xrsdl / (1 + LowPassFactor);
	Xavg8 += temp8_2;
	Xrsdl -= temp8_2 * (1 + LowPassFactor);

	temp8_1 = Yflt8 - Yavg8;
	temp8_2 = temp8_1 / (1 + LowPassFactor);
	Yavg8 += temp8_2;
	temp8_1 -= temp8_2 * (1 + LowPassFactor);
	Yrsdl += temp8_1;
	temp8_2 = Yrsdl / (1 + LowPassFactor);
	Yavg8 += temp8_2;
	Yrsdl -= temp8_2 * (1 + LowPassFactor);

	temp8_1 = Zflt8 - Zavg8;
	temp8_2 = temp8_1 / (1 + LowPassFactor);
	Zavg8 += temp8_2;
	temp8_1 -= temp8_2 * (1 + LowPassFactor);
	Zrsdl += temp8_1;
	temp8_2 = Zrsdl / (1 + LowPassFactor);
	Zavg8 += temp8_2;
	Zrsdl -= temp8_2 * (1 + LowPassFactor);

	*X = Xavg8;
	*Y = Yavg8;
	*Z = Zavg8;

#else
	if((++RawDataPointer)>=RawDataLength) RawDataPointer = 0;

	Xraw[RawDataPointer] = Xnew8;
	Yraw[RawDataPointer] = Ynew8;
	Zraw[RawDataPointer] = Znew8;

	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Xraw[i];
	}
	Xavg8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Yraw[i];
	}
	Yavg8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Zraw[i];
	}
	Zavg8 = (VINT8)(temp16/RawDataLength);

	*X = Xavg8;
	*Y = Yavg8;
	*Z = Zavg8;
#endif
}

enum {
	MMA865X_STATUS = 0x00,
	MMA865X_OUT_X_MSB,
	MMA865X_OUT_X_LSB,
	MMA865X_OUT_Y_MSB,
	MMA865X_OUT_Y_LSB,
	MMA865X_OUT_Z_MSB,
	MMA865X_OUT_Z_LSB,

	MMA865X_F_SETUP = 0x09,
	MMA865X_TRIG_CFG,
	MMA865X_SYSMOD,
	MMA865X_INT_SOURCE,
	MMA865X_WHO_AM_I,
	MMA865X_XYZ_DATA_CFG,
	MMA865X_HP_FILTER_CUTOFF,

	MMA865X_PL_STATUS,
	MMA865X_PL_CFG,
	MMA865X_PL_COUNT,
	MMA865X_PL_BF_ZCOMP,
	MMA865X_P_L_THS_REG,

	MMA865X_FF_MT_CFG,
	MMA865X_FF_MT_SRC,
	MMA865X_FF_MT_THS,
	MMA865X_FF_MT_COUNT,

	MMA865X_TRANSIENT_CFG = 0x1D,
	MMA865X_TRANSIENT_SRC,
	MMA865X_TRANSIENT_THS,
	MMA865X_TRANSIENT_COUNT,

	MMA865X_PULSE_CFG,
	MMA865X_PULSE_SRC,
	MMA865X_PULSE_THSX,
	MMA865X_PULSE_THSY,
	MMA865X_PULSE_THSZ,
	MMA865X_PULSE_TMLT,
	MMA865X_PULSE_LTCY,
	MMA865X_PULSE_WIND,

	MMA865X_ASLP_COUNT,
	MMA865X_CTRL_REG1,
	MMA865X_CTRL_REG2,
	MMA865X_CTRL_REG3,
	MMA865X_CTRL_REG4,
	MMA865X_CTRL_REG5,

	MMA865X_OFF_X,
	MMA865X_OFF_Y,
	MMA865X_OFF_Z,

	MMA865X_REG_END,
};


// device info
#define SENSOR_NAME					"mma8653"
#define SENSOR_I2C_ADDR				0x1D
#define ABSMIN						-32
#define ABSMAX						31
#define FUZZ						1
#define LSG							21
#define MAX_DELAY					50

// constant define
#define MMA8653_MODE_STANDBY		0x00
#define MMA8653_MODE_ACTIVE			0x01

#define MMA8653_RATE_120			0x00
#define MMA8653_RATE_64				0x01
#define MMA8653_RATE_32				0x02
#define MMA8653_RATE_16				0x03
#define MMA8653_RATE_8				0x04

// register define
#define MMA8653_XOUT_REG			0x00
#define MMA8653_YOUT_REG			0x01
#define MMA8653_ZOUT_REG			0x02
#define MMA8653_MODE_REG			MMA865X_CTRL_REG1
#define MMA8653_SR_REG				0x08

// register bits define
#define MMA8653_XOUT_BIT__POS		0
#define MMA8653_XOUT_BIT__LEN		6
#define MMA8653_XOUT_BIT__MSK		0x3f
#define MMA8653_XOUT_BIT__REG		MMA8653_XOUT_REG

#define MMA8653_XALERT_BIT__POS		6
#define MMA8653_XALERT_BIT__LEN		1
#define MMA8653_XALERT_BIT__MSK		0x40
#define MMA8653_XALERT_BIT__REG		MMA8653_XOUT_REG

#define MMA8653_MODE_BIT__POS		0
#define MMA8653_MODE_BIT__LEN		1
#define MMA8653_MODE_BIT__MSK		0x01
#define MMA8653_MODE_BIT__REG		MMA8653_MODE_REG

#define MMA8653_RATE_BIT__POS		0
#define MMA8653_RATE_BIT__LEN		3
#define MMA8653_RATE_BIT__MSK		0x07
#define MMA8653_RATE_BIT__REG		MMA8653_SR_REG

#define MMA8653_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

#define MMA8653_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))


struct mma8653_acc{
	s16    x;
	s16    y;
	s16    z;
} ;

struct mma8653_data {
	struct i2c_client *mma8653_client;
	struct input_dev *input;
	atomic_t delay;
	atomic_t enable;
	struct mutex enable_mutex;
	struct delayed_work work;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	atomic_t position;
	atomic_t calibrated;
	struct mma8653_acc offset;
	atomic_t fuzz;
};

// cfg data : 1 -- used
#define CFG_GSENSOR_USE_CONFIG  0

// calibration file path
#define CFG_GSENSOR_CALIBFILE   "/data/data/com.actions.sensor.calib/files/gsensor_calib.txt"

/*******************************************
 * for xml cfg
 *******************************************/
#define CFG_GSENSOR_ADAP_ID          "gsensor.i2c_adap_id"
#define CFG_GSENSOR_POSITION         "gsensor.position"
#define CFG_GSENSOR_CALIBRATION      "gsensor.calibration"

extern int get_config(const char *key, char *buff, int len);
/*******************************************
 * end for xml cfg
 *******************************************/

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mma8653_early_suspend(struct early_suspend *h);
static void mma8653_early_resume(struct early_suspend *h);
#endif

static int mma8653_axis_remap(struct i2c_client *client, struct mma8653_acc *acc);

static int mma8653_smbus_read_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
	*data = dummy & 0x000000ff;

	return 0;
}

static int mma8653_smbus_write_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

/*
static int mma8653_smbus_read_byte_block(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int mma8653_smbus_write_byte_block(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_write_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}
*/

static int mma8653_set_mode(struct i2c_client *client, unsigned char mode)
{
	int comres = 0;
	unsigned char data = 0;

	comres = mma8653_smbus_read_byte(client, MMA8653_MODE_BIT__REG, &data);
	data  = MMA8653_SET_BITSLICE(data, MMA8653_MODE_BIT, mode);
	comres += mma8653_smbus_write_byte(client, MMA8653_MODE_BIT__REG, &data);

	return comres;
}

static int mma8653_get_mode(struct i2c_client *client, unsigned char *mode)
{
	int comres = 0;
	unsigned char data = 0;

	comres = mma8653_smbus_read_byte(client, MMA8653_MODE_BIT__REG, &data);
	*mode  = MMA8653_GET_BITSLICE(data, MMA8653_MODE_BIT);

	return comres;
}

static int mma8653_set_rate(struct i2c_client *client, unsigned char rate)
{
	int comres = 0;
	unsigned char data = 0;

	//comres += mma8653_smbus_read_byte(client, MMA8653_RATE_BIT__REG, &data);
	//data  = MMA8653_SET_BITSLICE(data, MMA8653_RATE_BIT, rate);
	//comres += mma8653_smbus_write_byte(client, MMA8653_RATE_BIT__REG, &data);

	return comres;
}

static int mma8653_get_rate(struct i2c_client *client, unsigned char *rate)
{
	int comres = 0;
	unsigned char data = 0;

	//comres = mma8653_smbus_read_byte(client, MMA8653_RATE_BIT__REG, &data);
	//*rate  = MMA8653_GET_BITSLICE(data, MMA8653_RATE_BIT);

	return comres;
}

static int mma8653_hw_init(struct i2c_client *client)
{
	int comres = 0;
    int ret;

	// sample rate: 32
	//comres += mma8653_set_rate(client, MMA8653_RATE_120);

    ret = i2c_smbus_read_byte_data(client,MMA865X_WHO_AM_I);
	printk("%s:addr = 0x%x, Read ID value is :%d\n",
		        __func__, client->addr, ret);

	i2c_smbus_write_byte_data(client, MMA865X_CTRL_REG1, 0);
	i2c_smbus_write_byte_data(client, MMA865X_XYZ_DATA_CFG,0);

	return comres;
}
/*
static int mma8653_read_data(struct i2c_client *client, struct mma8653_acc *acc)
{
	int comres = 0;
	unsigned char data[3];
	int idx, alert;
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	for (idx = 0; idx < 3; idx ++) {
		do {
			comres += mma8653_smbus_read_byte(client, idx+MMA8653_XOUT_BIT__REG, &(data[idx]));
			if (comres < 0) {
				printk(KERN_ERR"read data fail! comres=%d\n", comres);
				break;
			} else {
				alert = MMA8653_GET_BITSLICE(data[idx], MMA8653_XALERT_BIT);
			}
		} while (alert); //read again if alert

		data[idx] = MMA8653_GET_BITSLICE(data[idx], MMA8653_XOUT_BIT);
		// process symbol bit
		if (data[idx] & (1 << (MMA8653_XOUT_BIT__LEN-1))) {
			data[idx] |= ~MMA8653_XOUT_BIT__MSK;
		}
	}

	acc->x =  (signed char)data[0];
	acc->y =  (signed char)data[1];
	acc->z =  (signed char)data[2];

	// calibrate offset
	acc->x += mma8653->offset.x;
	acc->y += mma8653->offset.y;
	acc->z += mma8653->offset.z;

	return comres;
}*/

static int mma8653_read_data(struct i2c_client *client, struct mma8653_acc *acc) 
{
	u8	tmp_data[7];
    //printk("%s....................%d.......................\n",__func__,__LINE__);
	if (i2c_smbus_read_i2c_block_data(client,MMA865X_OUT_X_MSB,7,tmp_data) < 7) {
		dev_err(&client->dev, "i2c block read failed\n");
			return -3;
	}

	acc->x = ((tmp_data[0] << 8) & 0xff00) | tmp_data[1];
	acc->y = ((tmp_data[2] << 8) & 0xff00) | tmp_data[3];
	acc->z = ((tmp_data[4] << 8) & 0xff00) | tmp_data[5];

	acc->x = -(short)(acc->x) >> 6;
	acc->y = -(short)(acc->y) >> 6;
	acc->z =  (short)(acc->z) >> 6;


	/*if (mma_status.mode == MODE_4G){
		(*x)=(*x)<<1;
		(*y)=(*y)<<1;
		(*z)=(*z)<<1;
	}
	else if (mma_status.mode == MODE_8G){
		(*x)=(*x)<<2;
		(*y)=(*y)<<2;
		(*z)=(*z)<<2;
	}*/

	return 0;
}


static ssize_t mma8653_register_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int address, value;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	sscanf(buf, "[0x%x]=0x%x", &address, &value);

	if (mma8653_smbus_write_byte(mma8653->mma8653_client, (unsigned char)address,
				(unsigned char *)&value) < 0)
		return -EINVAL;

	return count;
}

static ssize_t mma8653_register_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);
	size_t count = 0;
	u8 reg[MMA865X_REG_END];
	int i;

	for (i = 0 ; i < MMA865X_REG_END; i++) {
		mma8653_smbus_read_byte(mma8653->mma8653_client, i, reg+i);
		count += sprintf(&buf[count], "0x%x: 0x%x\n", i, reg[i]);
	}

	return count;
}

static ssize_t mma8653_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	if (mma8653_get_mode(mma8653->mma8653_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t mma8653_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (mma8653_set_mode(mma8653->mma8653_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t mma8653_rate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	if (mma8653_get_rate(mma8653->mma8653_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t mma8653_rate_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (mma8653_set_rate(mma8653->mma8653_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t mma8653_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct mma8653_data *mma8653 = input_get_drvdata(input);
	struct mma8653_acc acc;

	mma8653_read_data(mma8653->mma8653_client, &acc);
	mma8653_axis_remap(mma8653->mma8653_client, &acc);

	return sprintf(buf, "%d %d %d\n", acc.x, acc.y, acc.z);
}

static ssize_t mma8653_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&mma8653->delay));

}

static ssize_t mma8653_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	atomic_set(&mma8653->delay, (unsigned int) data);

	return count;
}


static ssize_t mma8653_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&mma8653->enable));

}

static void mma8653_do_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	if (enable) {
		mma8653_set_mode(mma8653->mma8653_client, MMA8653_MODE_ACTIVE);
		schedule_delayed_work(&mma8653->work,
				msecs_to_jiffies(atomic_read(&mma8653->delay)));
	} else {
		mma8653_set_mode(mma8653->mma8653_client, MMA8653_MODE_STANDBY);
		cancel_delayed_work_sync(&mma8653->work);
	}
}

static void mma8653_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&mma8653->enable);

	mutex_lock(&mma8653->enable_mutex);
	if (enable != pre_enable) {
		mma8653_do_enable(dev, enable);
		atomic_set(&mma8653->enable, enable);
	}
	mutex_unlock(&mma8653->enable_mutex);
}

static ssize_t mma8653_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if ((data == 0) || (data == 1))
		mma8653_set_enable(dev, data);

	return count;
}

static ssize_t mma8653_fuzz_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&mma8653->fuzz));

}

static ssize_t mma8653_fuzz_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	atomic_set(&(mma8653->fuzz), (int) data);

	if(mma8653->input != NULL) {
		mma8653->input->absinfo[ABS_X].fuzz = data;
		mma8653->input->absinfo[ABS_Y].fuzz = data;
		mma8653->input->absinfo[ABS_Z].fuzz = data;
	}

	return count;
}

static ssize_t mma8653_board_position_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int data;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	data = atomic_read(&(mma8653->position));

	return sprintf(buf, "%d\n", data);
}

static ssize_t mma8653_board_position_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	error = strict_strtol(buf, 10, &data);
	if (error)
		return error;

	atomic_set(&(mma8653->position), (int) data);

	return count;
}

static ssize_t mma8653_calibration_run_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int cfg_calibration[3];
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);
	struct mma8653_acc acc;

	mma8653_read_data(mma8653->mma8653_client, &acc);

	mma8653->offset.x = 0 - acc.x;
	mma8653->offset.y = 0 - acc.y;
	if (atomic_read(&mma8653->position) > 0) {
		mma8653->offset.z = LSG - acc.z;
	} else {
		mma8653->offset.z = (-LSG) - acc.z;
	}

	printk(KERN_INFO "fast calibration: %d %d %d\n", mma8653->offset.x,
			mma8653->offset.y, mma8653->offset.z);

	cfg_calibration[0] = mma8653->offset.x;
	cfg_calibration[1] = mma8653->offset.y;
	cfg_calibration[2] = mma8653->offset.z;

	printk(KERN_INFO "run fast calibration finished\n");
	return count;
}

static ssize_t mma8653_calibration_reset_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int cfg_calibration[3];
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	memset(&(mma8653->offset), 0, sizeof(struct mma8653_acc));
	memset(cfg_calibration, 0, sizeof(cfg_calibration));

	printk(KERN_INFO "reset fast calibration finished\n");
	return count;
}

static ssize_t mma8653_calibration_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	return sprintf(buf, "%d %d %d\n", mma8653->offset.x,
			mma8653->offset.y, mma8653->offset.z);
}

static ssize_t mma8653_calibration_value_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int data[3];
	struct i2c_client *client = to_i2c_client(dev);
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);

	sscanf(buf, "%d %d %d", &data[0], &data[1], &data[2]);
	mma8653->offset.x = (signed short) data[0];
	mma8653->offset.y = (signed short) data[1];
	mma8653->offset.z = (signed short) data[2];

	printk(KERN_INFO "set fast calibration finished\n");
	return count;
}

#if (0)
static DEVICE_ATTR(reg, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_register_show, mma8653_register_store);
static DEVICE_ATTR(mode, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_mode_show, mma8653_mode_store);
static DEVICE_ATTR(rate, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_rate_show, mma8653_rate_store);
static DEVICE_ATTR(value, S_IRUGO,
		mma8653_value_show, NULL);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_delay_show, mma8653_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_enable_show, mma8653_enable_store);
static DEVICE_ATTR(fuzz, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_fuzz_show, mma8653_fuzz_store);
static DEVICE_ATTR(board_position, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_board_position_show, mma8653_board_position_store);
static DEVICE_ATTR(calibration_run, S_IWUSR|S_IWGRP|S_IWOTH,
		NULL, mma8653_calibration_run_store);
static DEVICE_ATTR(calibration_reset, S_IWUSR|S_IWGRP|S_IWOTH,
		NULL, mma8653_calibration_reset_store);
static DEVICE_ATTR(calibration_value, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma8653_calibration_value_show,
		mma8653_calibration_value_store);
#else
static DEVICE_ATTR(reg, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_register_show, mma8653_register_store);
static DEVICE_ATTR(mode, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_mode_show, mma8653_mode_store);
static DEVICE_ATTR(rate, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_rate_show, mma8653_rate_store);
static DEVICE_ATTR(value, S_IRUGO,
		mma8653_value_show, NULL);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_delay_show, mma8653_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_enable_show, mma8653_enable_store);
static DEVICE_ATTR(fuzz, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_fuzz_show, mma8653_fuzz_store);
static DEVICE_ATTR(board_position, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_board_position_show, mma8653_board_position_store);
static DEVICE_ATTR(calibration_run, S_IWUSR|S_IWGRP,
		NULL, mma8653_calibration_run_store);
static DEVICE_ATTR(calibration_reset, S_IWUSR|S_IWGRP,
		NULL, mma8653_calibration_reset_store);
static DEVICE_ATTR(calibration_value, S_IRUGO|S_IWUSR|S_IWGRP,
		mma8653_calibration_value_show,
		mma8653_calibration_value_store);
#endif

static struct attribute *mma8653_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_mode.attr,
	&dev_attr_rate.attr,
	&dev_attr_value.attr,
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_fuzz.attr,
	&dev_attr_board_position.attr,
	&dev_attr_calibration_run.attr,
	&dev_attr_calibration_reset.attr,
	&dev_attr_calibration_value.attr,
	NULL
};

static struct attribute_group mma8653_attribute_group = {
	.attrs = mma8653_attributes
};

static int mma8653_read_file(char *path, char *buf, int size)
{
	struct file *filp;
	loff_t len, offset;
	int ret=0;
	mm_segment_t fs;

	filp = filp_open(path, O_RDWR, 0777);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		goto out;
	}

	len = vfs_llseek(filp, 0, SEEK_END);
	if (len > size) {
		len = size;
	}

	offset = vfs_llseek(filp, 0, SEEK_SET);

	fs=get_fs();
	set_fs(KERNEL_DS);

	ret = vfs_read(filp, (char __user *)buf, (size_t)len, &(filp->f_pos));

	set_fs(fs);

	filp_close(filp, NULL);
out:
	return ret;
}

static int mma8653_load_user_calibration(struct i2c_client *client)
{
	char buffer[16];
	int ret = 0;
	int data[3];
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);
	int calibrated = atomic_read(&mma8653->calibrated);

	// only calibrate once
	if (calibrated) {
		goto usr_calib_end;
	} else {
		atomic_set(&mma8653->calibrated, 1);
	}

	ret = mma8653_read_file(CFG_GSENSOR_CALIBFILE, buffer, sizeof(buffer));
	if (ret <= 0) {
		printk(KERN_ERR "gsensor calibration file not exist!\n");
		goto usr_calib_end;
	}

	sscanf(buffer, "%d %d %d", &data[0], &data[1], &data[2]);
	mma8653->offset.x = (signed short) data[0];
	mma8653->offset.y = (signed short) data[1];
	mma8653->offset.z = (signed short) data[2];

	printk(KERN_INFO "user cfg_calibration: %d %d %d\n", data[0], data[1], data[2]);

usr_calib_end:
	return ret;
}

static int mma8653_axis_remap(struct i2c_client *client, struct mma8653_acc *acc)
{
	s16 swap;
	struct mma8653_data *mma8653 = i2c_get_clientdata(client);
	int position = atomic_read(&mma8653->position);

	switch (abs(position)) {
		case 1:
			break;
		case 2:
			swap = acc->x;
			acc->x = acc->y;
			acc->y = -swap;
			break;
		case 3:
			acc->x = -(acc->x);
			acc->y = -(acc->y);
			break;
		case 4:
			swap = acc->x;
			acc->x = -acc->y;
			acc->y = swap;
			break;
	}

	if (position < 0) {
		acc->z = -(acc->z);
		acc->x = -(acc->x);
	}

	return 0;
}

static void mma8653_work_func(struct work_struct *work)
{
	struct mma8653_data *mma8653 = container_of((struct delayed_work *)work,
			struct mma8653_data, work);
	static struct mma8653_acc acc;
	int result;
	unsigned long delay = msecs_to_jiffies(atomic_read(&mma8653->delay));
	//unsigned long delay = msecs_to_jiffies(10);

	mma8653_load_user_calibration(mma8653->mma8653_client);

	result = mma8653_read_data(mma8653->mma8653_client, &acc);
	if (result == 0) {
		VINT8 x, y, z;
		mma8653_axis_remap(mma8653->mma8653_client, &acc);

		//printk(KERN_ERR "before %d,%d,%d!\n",acc.x,acc.y,acc.z);
		/*x = acc.x & 0xff;
		y = acc.y & 0xff;
		z = acc.z & 0xff;

		XYZ_Filter(&x,&y,&z);

		acc.x = x;
		acc.y = y;
		acc.z = z;*/

		//printk(KERN_ERR "after  %d,%d,%d!\n",acc.x,acc.y,acc.z);

		input_report_abs(mma8653->input, ABS_X, acc.x);
		input_report_abs(mma8653->input, ABS_Y, acc.y);
		input_report_abs(mma8653->input, ABS_Z, acc.z);
		input_sync(mma8653->input);
	}
	schedule_delayed_work(&mma8653->work, delay);
}

static int mma8653_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0;
	struct mma8653_data *data;
	struct input_dev *dev;
	int cfg_position;
	int cfg_calibration[3];
	struct mma8653_private_data *pdata;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct mma8653_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->mma8653_client = client;
	mutex_init(&data->enable_mutex);

	INIT_DELAYED_WORK(&data->work, mma8653_work_func);
	atomic_set(&data->delay, MAX_DELAY);
	atomic_set(&data->enable, 0);

#if CFG_GSENSOR_USE_CONFIG > 0
	/*get xml cfg*/
	err = get_config(CFG_GSENSOR_POSITION, (char *)(&cfg_position), sizeof(int));
	if (err != 0) {
		printk(KERN_ERR"get position %d fail\n", cfg_position);
		goto kfree_exit;
	}
#else
	pdata = client->dev.platform_data;
	//if(pdata)
	//	cfg_position = pdata->position;
	//else
		cfg_position = -3;
#endif
	atomic_set(&data->position, cfg_position);
	atomic_set(&data->calibrated, 0);
	atomic_set(&data->fuzz, FUZZ);

	//power on init regs
	err = mma8653_hw_init(data->mma8653_client);
	if (err < 0) {
		printk(KERN_ERR"mma8653 probe fail! err:%d\n", err);
		goto kfree_exit;
	}

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_abs_params(dev, ABS_X, ABSMIN, ABSMAX, FUZZ, 0);
	input_set_abs_params(dev, ABS_Y, ABSMIN, ABSMAX, FUZZ, 0);
	input_set_abs_params(dev, ABS_Z, ABSMIN, ABSMAX, FUZZ, 0);
	input_set_drvdata(dev, data);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		goto kfree_exit;
	}

	data->input = dev;

	err = sysfs_create_group(&data->input->dev.kobj,
			&mma8653_attribute_group);
	if (err < 0)
		goto error_sysfs;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = mma8653_early_suspend;
	data->early_suspend.resume = mma8653_early_resume;
	register_early_suspend(&data->early_suspend);
#endif

#if CFG_GSENSOR_USE_CONFIG > 0
	/*get xml cfg*/
	err = get_config(CFG_GSENSOR_CALIBRATION, (char *)cfg_calibration, sizeof(cfg_calibration));
	if (err != 0) {
		printk(KERN_ERR"get calibration fail\n");
		goto error_sysfs;
	}
#else
	memset(cfg_calibration, 0, sizeof(cfg_calibration));
#endif

	data->offset.x = (signed short) cfg_calibration[0];
	data->offset.y = (signed short) cfg_calibration[1];
	data->offset.z = (signed short) cfg_calibration[2];

	/* default enable */
	mma8653_do_enable(&client->dev, 1);

	return 0;

error_sysfs:
	input_unregister_device(data->input);

kfree_exit:
	kfree(data);
exit:
	return err;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mma8653_early_suspend(struct early_suspend *h)
{
	// sensor hal will disable when early suspend
}


static void mma8653_early_resume(struct early_suspend *h)
{
	// sensor hal will enable when early resume
}
#endif

static int __devexit mma8653_remove(struct i2c_client *client)
{
	struct mma8653_data *data = i2c_get_clientdata(client);

	mma8653_set_enable(&client->dev, 0);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	sysfs_remove_group(&data->input->dev.kobj, &mma8653_attribute_group);
	input_unregister_device(data->input);
	kfree(data);

	return 0;
}

#ifdef CONFIG_PM

static int mma8653_suspend(struct i2c_client *client, pm_message_t state)
{
	mma8653_do_enable(&client->dev, 0);

	return 0;
}

static int mma8653_resume(struct i2c_client *client)
{
	struct mma8653_data *data = i2c_get_clientdata(client);
	struct device *dev = &client->dev;

	//power on init regs
	mma8653_hw_init(data->mma8653_client);
	mma8653_do_enable(dev, atomic_read(&data->enable));

	return 0;
}

#else

#define mma8653_suspend			NULL
#define mma8653_resume			NULL

#endif /* CONFIG_PM */

/*
static const unsigned short  mma8653_addresses[] = {
	SENSOR_I2C_ADDR,
	I2C_CLIENT_END,
};
*/

static const struct i2c_device_id mma8653_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, mma8653_id);

static struct i2c_driver mma8653_driver = {
	.driver = {
		.owner		= THIS_MODULE,
		.name		= SENSOR_NAME,
	},
	.class			= I2C_CLASS_HWMON,
	//.address_list    = mma8653_addresses,
	.id_table		= mma8653_id,
	.probe			= mma8653_probe,
	.remove			= __devexit_p(mma8653_remove),
	.suspend 	= mma8653_suspend,
	.resume  	= mma8653_resume,
};

#if CFG_GSENSOR_USE_CONFIG > 0
static struct i2c_board_info mma8653_board_info={
	.type = SENSOR_NAME,
	.addr = SENSOR_I2C_ADDR,
};
#endif

#if CFG_GSENSOR_USE_CONFIG > 0
static struct i2c_client *mma8653_client;
#endif

static int __init mma8653_init(void)
{
#if CFG_GSENSOR_USE_CONFIG > 0
	struct i2c_adapter *i2c_adap;
	unsigned int cfg_i2c_adap_id;
	int ret;

	ret = get_config(CFG_GSENSOR_ADAP_ID, (char *)(&cfg_i2c_adap_id), sizeof(unsigned int));
	if (ret != 0) {
		printk(KERN_ERR"get i2c_adap_id %d fail\n", cfg_i2c_adap_id);
		return ret;
	}

	i2c_adap = i2c_get_adapter(cfg_i2c_adap_id);
	mma8653_client = i2c_new_device(i2c_adap, &mma8653_board_info);
	i2c_put_adapter(i2c_adap);
#endif
	return i2c_add_driver(&mma8653_driver);
}

static void __exit mma8653_exit(void)
{
#if CFG_GSENSOR_USE_CONFIG > 0
	i2c_unregister_device(mma8653_client);
#endif
	i2c_del_driver(&mma8653_driver);
}

MODULE_AUTHOR("Zhining Song <songzhining@actions-semi.com>");
MODULE_DESCRIPTION("MMA8653 3-Axis Orientation/Motion Detection Sensor driver");
MODULE_LICENSE("GPL");

module_init(mma8653_init);
module_exit(mma8653_exit);

