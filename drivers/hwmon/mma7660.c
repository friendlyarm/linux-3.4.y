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

#include "mma7660.h"

#include "mma7660_xyz_filter.h"


// device info
#define SENSOR_NAME					"mma7660"
#define SENSOR_I2C_ADDR				0x4c
#define ABSMIN						-32
#define ABSMAX						31
#define FUZZ						1
#define LSG							21
#define MAX_DELAY					200

// constant define
#define MMA7660_MODE_STANDBY		0x00
#define MMA7660_MODE_ACTIVE			0x01

#define MMA7660_RATE_120			0x00
#define MMA7660_RATE_64				0x01
#define MMA7660_RATE_32				0x02
#define MMA7660_RATE_16				0x03
#define MMA7660_RATE_8				0x04

// register define
#define MMA7660_XOUT_REG			0x00
#define MMA7660_YOUT_REG			0x01
#define MMA7660_ZOUT_REG			0x02
#define MMA7660_MODE_REG			0x07
#define MMA7660_SR_REG				0x08

// register bits define
#define MMA7660_XOUT_BIT__POS		0
#define MMA7660_XOUT_BIT__LEN		6
#define MMA7660_XOUT_BIT__MSK		0x3f
#define MMA7660_XOUT_BIT__REG		MMA7660_XOUT_REG

#define MMA7660_XALERT_BIT__POS		6
#define MMA7660_XALERT_BIT__LEN		1
#define MMA7660_XALERT_BIT__MSK		0x40
#define MMA7660_XALERT_BIT__REG		MMA7660_XOUT_REG

#define MMA7660_MODE_BIT__POS		0
#define MMA7660_MODE_BIT__LEN		1
#define MMA7660_MODE_BIT__MSK		0x01
#define MMA7660_MODE_BIT__REG		MMA7660_MODE_REG

#define MMA7660_RATE_BIT__POS		0
#define MMA7660_RATE_BIT__LEN		3
#define MMA7660_RATE_BIT__MSK		0x07
#define MMA7660_RATE_BIT__REG		MMA7660_SR_REG

#define MMA7660_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

#define MMA7660_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))


struct mma7660_acc{
	s16    x;
	s16    y;
	s16    z;
} ;

struct mma7660_data {
	struct i2c_client *mma7660_client;
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
	struct mma7660_acc offset;
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
static void mma7660_early_suspend(struct early_suspend *h);
static void mma7660_early_resume(struct early_suspend *h);
#endif

static int mma7660_axis_remap(struct i2c_client *client, struct mma7660_acc *acc);

static int mma7660_smbus_read_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
	*data = dummy & 0x000000ff;

	return 0;
}

static int mma7660_smbus_write_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

/*
static int mma7660_smbus_read_byte_block(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int mma7660_smbus_write_byte_block(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_write_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}
*/

static int mma7660_set_mode(struct i2c_client *client, unsigned char mode)
{
	int comres = 0;
	unsigned char data = 0;

	comres = mma7660_smbus_read_byte(client, MMA7660_MODE_BIT__REG, &data);
	data  = MMA7660_SET_BITSLICE(data, MMA7660_MODE_BIT, mode);
	comres += mma7660_smbus_write_byte(client, MMA7660_MODE_BIT__REG, &data);

	return comres;
}

static int mma7660_get_mode(struct i2c_client *client, unsigned char *mode)
{
	int comres = 0;
	unsigned char data = 0;

	comres = mma7660_smbus_read_byte(client, MMA7660_MODE_BIT__REG, &data);
	*mode  = MMA7660_GET_BITSLICE(data, MMA7660_MODE_BIT);

	return comres;
}

static int mma7660_set_rate(struct i2c_client *client, unsigned char rate)
{
	int comres = 0;
	unsigned char data = 0;

	comres += mma7660_smbus_read_byte(client, MMA7660_RATE_BIT__REG, &data);
	data  = MMA7660_SET_BITSLICE(data, MMA7660_RATE_BIT, rate);
	comres += mma7660_smbus_write_byte(client, MMA7660_RATE_BIT__REG, &data);

	return comres;
}

static int mma7660_get_rate(struct i2c_client *client, unsigned char *rate)
{
	int comres = 0;
	unsigned char data = 0;

	comres = mma7660_smbus_read_byte(client, MMA7660_RATE_BIT__REG, &data);
	*rate  = MMA7660_GET_BITSLICE(data, MMA7660_RATE_BIT);

	return comres;
}

static int mma7660_hw_init(struct i2c_client *client)
{
	int comres = 0;

	// sample rate: 32
	comres += mma7660_set_rate(client, MMA7660_RATE_120);

	return comres;
}

static int mma7660_read_data(struct i2c_client *client, struct mma7660_acc *acc)
{
	int comres = 0;
	unsigned char data[3];
	int idx, alert;
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	for (idx = 0; idx < 3; idx ++) {
		do {
			comres += mma7660_smbus_read_byte(client, idx+MMA7660_XOUT_BIT__REG, &(data[idx]));
			if (comres < 0) {
				printk(KERN_ERR"read data fail! comres=%d\n", comres);
				break;
			} else {
				alert = MMA7660_GET_BITSLICE(data[idx], MMA7660_XALERT_BIT);
			}
		} while (alert); //read again if alert

		data[idx] = MMA7660_GET_BITSLICE(data[idx], MMA7660_XOUT_BIT);
		// process symbol bit
		if (data[idx] & (1 << (MMA7660_XOUT_BIT__LEN-1))) {
			data[idx] |= ~MMA7660_XOUT_BIT__MSK;
		}
	}

	acc->x =  (signed char)data[0];
	acc->y =  (signed char)data[1];
	acc->z =  (signed char)data[2];

	// calibrate offset
	acc->x += mma7660->offset.x;
	acc->y += mma7660->offset.y;
	acc->z += mma7660->offset.z;

	return comres;
}

static ssize_t mma7660_register_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int address, value;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	sscanf(buf, "[0x%x]=0x%x", &address, &value);

	if (mma7660_smbus_write_byte(mma7660->mma7660_client, (unsigned char)address,
				(unsigned char *)&value) < 0)
		return -EINVAL;

	return count;
}

static ssize_t mma7660_register_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);
	size_t count = 0;
	u8 reg[0x0b];
	int i;

	for (i = 0 ; i < 0x0b; i++) {
		mma7660_smbus_read_byte(mma7660->mma7660_client, i, reg+i);
		count += sprintf(&buf[count], "0x%x: 0x%x\n", i, reg[i]);
	}

	return count;
}

static ssize_t mma7660_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	if (mma7660_get_mode(mma7660->mma7660_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t mma7660_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (mma7660_set_mode(mma7660->mma7660_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t mma7660_rate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	if (mma7660_get_rate(mma7660->mma7660_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t mma7660_rate_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (mma7660_set_rate(mma7660->mma7660_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t mma7660_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct mma7660_data *mma7660 = input_get_drvdata(input);
	struct mma7660_acc acc;

	mma7660_read_data(mma7660->mma7660_client, &acc);
	mma7660_axis_remap(mma7660->mma7660_client, &acc);

	return sprintf(buf, "%d %d %d\n", acc.x, acc.y, acc.z);
}

static ssize_t mma7660_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&mma7660->delay));

}

static ssize_t mma7660_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	atomic_set(&mma7660->delay, (unsigned int) data);

	return count;
}


static ssize_t mma7660_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&mma7660->enable));

}

static void mma7660_do_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	if (enable) {
		mma7660_set_mode(mma7660->mma7660_client, MMA7660_MODE_ACTIVE);
		schedule_delayed_work(&mma7660->work,
				msecs_to_jiffies(atomic_read(&mma7660->delay)));
	} else {
		mma7660_set_mode(mma7660->mma7660_client, MMA7660_MODE_STANDBY);
		cancel_delayed_work_sync(&mma7660->work);
	}
}

static void mma7660_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&mma7660->enable);

	mutex_lock(&mma7660->enable_mutex);
	if (enable != pre_enable) {
		mma7660_do_enable(dev, enable);
		atomic_set(&mma7660->enable, enable);
	}
	mutex_unlock(&mma7660->enable_mutex);
}

static ssize_t mma7660_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if ((data == 0) || (data == 1))
		mma7660_set_enable(dev, data);

	return count;
}

static ssize_t mma7660_fuzz_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&mma7660->fuzz));

}

static ssize_t mma7660_fuzz_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	atomic_set(&(mma7660->fuzz), (int) data);

	if(mma7660->input != NULL) {
		mma7660->input->absinfo[ABS_X].fuzz = data;
		mma7660->input->absinfo[ABS_Y].fuzz = data;
		mma7660->input->absinfo[ABS_Z].fuzz = data;
	}

	return count;
}

static ssize_t mma7660_board_position_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int data;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	data = atomic_read(&(mma7660->position));

	return sprintf(buf, "%d\n", data);
}

static ssize_t mma7660_board_position_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	error = strict_strtol(buf, 10, &data);
	if (error)
		return error;

	atomic_set(&(mma7660->position), (int) data);

	return count;
}

static ssize_t mma7660_calibration_run_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int cfg_calibration[3];
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);
	struct mma7660_acc acc;

	mma7660_read_data(mma7660->mma7660_client, &acc);

	mma7660->offset.x = 0 - acc.x;
	mma7660->offset.y = 0 - acc.y;
	if (atomic_read(&mma7660->position) > 0) {
		mma7660->offset.z = LSG - acc.z;
	} else {
		mma7660->offset.z = (-LSG) - acc.z;
	}

	printk(KERN_INFO "fast calibration: %d %d %d\n", mma7660->offset.x,
			mma7660->offset.y, mma7660->offset.z);

	cfg_calibration[0] = mma7660->offset.x;
	cfg_calibration[1] = mma7660->offset.y;
	cfg_calibration[2] = mma7660->offset.z;

	printk(KERN_INFO "run fast calibration finished\n");
	return count;
}

static ssize_t mma7660_calibration_reset_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int cfg_calibration[3];
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	memset(&(mma7660->offset), 0, sizeof(struct mma7660_acc));
	memset(cfg_calibration, 0, sizeof(cfg_calibration));

	printk(KERN_INFO "reset fast calibration finished\n");
	return count;
}

static ssize_t mma7660_calibration_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	return sprintf(buf, "%d %d %d\n", mma7660->offset.x,
			mma7660->offset.y, mma7660->offset.z);
}

static ssize_t mma7660_calibration_value_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int data[3];
	struct i2c_client *client = to_i2c_client(dev);
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);

	sscanf(buf, "%d %d %d", &data[0], &data[1], &data[2]);
	mma7660->offset.x = (signed short) data[0];
	mma7660->offset.y = (signed short) data[1];
	mma7660->offset.z = (signed short) data[2];

	printk(KERN_INFO "set fast calibration finished\n");
	return count;
}

#if (0)
static DEVICE_ATTR(reg, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_register_show, mma7660_register_store);
static DEVICE_ATTR(mode, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_mode_show, mma7660_mode_store);
static DEVICE_ATTR(rate, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_rate_show, mma7660_rate_store);
static DEVICE_ATTR(value, S_IRUGO,
		mma7660_value_show, NULL);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_delay_show, mma7660_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_enable_show, mma7660_enable_store);
static DEVICE_ATTR(fuzz, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_fuzz_show, mma7660_fuzz_store);
static DEVICE_ATTR(board_position, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_board_position_show, mma7660_board_position_store);
static DEVICE_ATTR(calibration_run, S_IWUSR|S_IWGRP|S_IWOTH,
		NULL, mma7660_calibration_run_store);
static DEVICE_ATTR(calibration_reset, S_IWUSR|S_IWGRP|S_IWOTH,
		NULL, mma7660_calibration_reset_store);
static DEVICE_ATTR(calibration_value, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		mma7660_calibration_value_show,
		mma7660_calibration_value_store);
#else
static DEVICE_ATTR(reg, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_register_show, mma7660_register_store);
static DEVICE_ATTR(mode, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_mode_show, mma7660_mode_store);
static DEVICE_ATTR(rate, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_rate_show, mma7660_rate_store);
static DEVICE_ATTR(value, S_IRUGO,
		mma7660_value_show, NULL);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_delay_show, mma7660_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_enable_show, mma7660_enable_store);
static DEVICE_ATTR(fuzz, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_fuzz_show, mma7660_fuzz_store);
static DEVICE_ATTR(board_position, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_board_position_show, mma7660_board_position_store);
static DEVICE_ATTR(calibration_run, S_IWUSR|S_IWGRP,
		NULL, mma7660_calibration_run_store);
static DEVICE_ATTR(calibration_reset, S_IWUSR|S_IWGRP,
		NULL, mma7660_calibration_reset_store);
static DEVICE_ATTR(calibration_value, S_IRUGO|S_IWUSR|S_IWGRP,
		mma7660_calibration_value_show,
		mma7660_calibration_value_store);
#endif

static struct attribute *mma7660_attributes[] = {
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

static struct attribute_group mma7660_attribute_group = {
	.attrs = mma7660_attributes
};

static int mma7660_read_file(char *path, char *buf, int size)
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

static int mma7660_load_user_calibration(struct i2c_client *client)
{
	char buffer[16];
	int ret = 0;
	int data[3];
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);
	int calibrated = atomic_read(&mma7660->calibrated);

	// only calibrate once
	if (calibrated) {
		goto usr_calib_end;
	} else {
		atomic_set(&mma7660->calibrated, 1);
	}

	ret = mma7660_read_file(CFG_GSENSOR_CALIBFILE, buffer, sizeof(buffer));
	if (ret <= 0) {
		printk(KERN_ERR "gsensor calibration file not exist!\n");
		goto usr_calib_end;
	}

	sscanf(buffer, "%d %d %d", &data[0], &data[1], &data[2]);
	mma7660->offset.x = (signed short) data[0];
	mma7660->offset.y = (signed short) data[1];
	mma7660->offset.z = (signed short) data[2];

	printk(KERN_INFO "user cfg_calibration: %d %d %d\n", data[0], data[1], data[2]);

usr_calib_end:
	return ret;
}

static int mma7660_axis_remap(struct i2c_client *client, struct mma7660_acc *acc)
{
	s16 swap;
	struct mma7660_data *mma7660 = i2c_get_clientdata(client);
	int position = atomic_read(&mma7660->position);

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

static void mma7660_work_func(struct work_struct *work)
{
	struct mma7660_data *mma7660 = container_of((struct delayed_work *)work,
			struct mma7660_data, work);
	static struct mma7660_acc acc;
	int result;
	//unsigned long delay = msecs_to_jiffies(atomic_read(&mma7660->delay));
	unsigned long delay = msecs_to_jiffies(10);

	mma7660_load_user_calibration(mma7660->mma7660_client);

	result = mma7660_read_data(mma7660->mma7660_client, &acc);
	if (result == 0) {
		VINT8 x, y, z;
		mma7660_axis_remap(mma7660->mma7660_client, &acc);

		//printk(KERN_ERR "before %d,%d,%d!\n",acc.x,acc.y,acc.z);
		x = acc.x & 0xff;
		y = acc.y & 0xff;
		z = acc.z & 0xff;

		XYZ_Filter(&x,&y,&z);

		acc.x = x;
		acc.y = y;
		acc.z = z;

		//printk(KERN_ERR "after  %d,%d,%d!\n",acc.x,acc.y,acc.z);

		input_report_abs(mma7660->input, ABS_X, acc.x);
		input_report_abs(mma7660->input, ABS_Y, acc.y);
		input_report_abs(mma7660->input, ABS_Z, acc.z);
		input_sync(mma7660->input);
	}
	schedule_delayed_work(&mma7660->work, delay);
}

static int mma7660_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0;
	struct mma7660_data *data;
	struct input_dev *dev;
	int cfg_position;
	int cfg_calibration[3];
	struct mma7660_private_data *pdata;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct mma7660_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->mma7660_client = client;
	mutex_init(&data->enable_mutex);

	INIT_DELAYED_WORK(&data->work, mma7660_work_func);
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
	if(pdata)
		cfg_position = pdata->position;
	else
		cfg_position = -3;
#endif
	atomic_set(&data->position, cfg_position);
	atomic_set(&data->calibrated, 0);
	atomic_set(&data->fuzz, FUZZ);

	//power on init regs
	err = mma7660_hw_init(data->mma7660_client);
	if (err < 0) {
		printk(KERN_ERR"mma7660 probe fail! err:%d\n", err);
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
			&mma7660_attribute_group);
	if (err < 0)
		goto error_sysfs;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = mma7660_early_suspend;
	data->early_suspend.resume = mma7660_early_resume;
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
	mma7660_do_enable(&client->dev, 1);

	return 0;

error_sysfs:
	input_unregister_device(data->input);

kfree_exit:
	kfree(data);
exit:
	return err;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mma7660_early_suspend(struct early_suspend *h)
{
	// sensor hal will disable when early suspend
}


static void mma7660_early_resume(struct early_suspend *h)
{
	// sensor hal will enable when early resume
}
#endif

static int __devexit mma7660_remove(struct i2c_client *client)
{
	struct mma7660_data *data = i2c_get_clientdata(client);

	mma7660_set_enable(&client->dev, 0);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	sysfs_remove_group(&data->input->dev.kobj, &mma7660_attribute_group);
	input_unregister_device(data->input);
	kfree(data);

	return 0;
}

#ifdef CONFIG_PM

static int mma7660_suspend(struct i2c_client *client, pm_message_t state)
{
	mma7660_do_enable(&client->dev, 0);

	return 0;
}

static int mma7660_resume(struct i2c_client *client)
{
	struct mma7660_data *data = i2c_get_clientdata(client);
	struct device *dev = &client->dev;

	//power on init regs
	mma7660_hw_init(data->mma7660_client);
	mma7660_do_enable(dev, atomic_read(&data->enable));

	return 0;
}

#else

#define mma7660_suspend			NULL
#define mma7660_resume			NULL

#endif /* CONFIG_PM */

/*
static const unsigned short  mma7660_addresses[] = {
	SENSOR_I2C_ADDR,
	I2C_CLIENT_END,
};
*/

static const struct i2c_device_id mma7660_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, mma7660_id);

static struct i2c_driver mma7660_driver = {
	.driver = {
		.owner		= THIS_MODULE,
		.name		= SENSOR_NAME,
	},
	.class			= I2C_CLASS_HWMON,
	//.address_list    = mma7660_addresses,
	.id_table		= mma7660_id,
	.probe			= mma7660_probe,
	.remove			= __devexit_p(mma7660_remove),
	.suspend 	= mma7660_suspend,
	.resume  	= mma7660_resume,
};

#if CFG_GSENSOR_USE_CONFIG > 0
static struct i2c_board_info mma7660_board_info={
	.type = SENSOR_NAME,
	.addr = SENSOR_I2C_ADDR,
};
#endif

#if CFG_GSENSOR_USE_CONFIG > 0
static struct i2c_client *mma7660_client;
#endif

static int __init mma7660_init(void)
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
	mma7660_client = i2c_new_device(i2c_adap, &mma7660_board_info);
	i2c_put_adapter(i2c_adap);
#endif
	return i2c_add_driver(&mma7660_driver);
}

static void __exit mma7660_exit(void)
{
#if CFG_GSENSOR_USE_CONFIG > 0
	i2c_unregister_device(mma7660_client);
#endif
	i2c_del_driver(&mma7660_driver);
}

MODULE_AUTHOR("Zhining Song <songzhining@actions-semi.com>");
MODULE_DESCRIPTION("MMA7660 3-Axis Orientation/Motion Detection Sensor driver");
MODULE_LICENSE("GPL");

module_init(mma7660_init);
module_exit(mma7660_exit);
