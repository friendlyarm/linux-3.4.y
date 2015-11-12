/* file stk8312.c
   brief This file contains all function implementations for the SK8312 in linux
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/wakelock.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define SENSOR_RESUME_WORK
#ifdef	CONFIG_SENSORS_STK8312_ROTATE
#define SKT_ROTATE		CONFIG_SENSORS_STK8312_ROTATE	// -1, 0, 1, 2, 3, 4
#else
#define SKT_ROTATE		(-1)
#endif

#define USE_FIXED_DELAY	10	// ms

//#define CONFIG_SENSORS_STK8312
//#define CONFIG_SENSORS_STK8313

#ifdef CONFIG_SENSORS_STK8312
	#include "stk8312.h"
	#define SENSOR_NAME              "stk8312"
	#define SENSOR_I2C_ADDR         0x3d
#elif defined (CONFIG_SENSORS_STK8313)
	#include "stk8313.h"
	#define SENSOR_NAME              "stk8313"
	#define SENSOR_I2C_ADDR         0x22
#endif

//#define ABSMIN                          -32
//#define ABSMAX                          31
//#define FUZZ                    1

#define STK_ACC_POLLING_MODE	1
#if (!STK_ACC_POLLING_MODE)
	#define ADDITIONAL_GPIO_CFG 1
	#define STK_INT_PIN	39
#endif

#define POSITIVE_Z_UP		0
#define NEGATIVE_Z_UP	1
#define POSITIVE_X_UP		2
#define NEGATIVE_X_UP	3
#define POSITIVE_Y_UP		4
#define NEGATIVE_Y_UP	5
static unsigned char stk831x_placement = POSITIVE_Z_UP;


#define STK_PERMISSION_THREAD
#ifndef CONFIG_SUSPEND_IDLE
#define STK_RESUME_RE_INIT
#endif

//#define STK_DEBUG_PRINT
//#define STK_DEBUG_RAWDATA
#define STK_LOWPASS
#define STK_FIR_LEN	4

#define STK831X_INIT_ODR		0		//0:100Hz, 1:50Hz, 2:25Hz
#define STK831X_SAMPLE_TIME_BASE		2
#define STK831X_SAMPLE_TIME_NO		4
const static int STK831X_SAMPLE_TIME[STK831X_SAMPLE_TIME_NO] = {10000, 20000, 40000, 80000};
static struct stk831x_data *stk831x_data_ptr;


#define STK_ACC_DRIVER_VERSION	"1.6.1 2013/05/24"

// cfg data : 1-- used
#define CFG_GSENSOR_USE_CONFIG  0

// calibration file path
#define CFG_GSENSOR_CALIBFILE   "/data/data/com.actions.sensor.calib/files/gsensor_calib.txt"

/*******************************************
* for xml cfg
*******************************************/
#define CFG_GSENSOR_ADAP_ID          "gsensor.i2c_adap_id"
#define CFG_GSENSOR_POSITION         "gsensor.position_stk8312"
#define CFG_GSENSOR_CALIBRATION      "gsensor.calibration"





#define MAX_DELAY              10

#define STK_DEBUG_CALI
#define STK_SAMPLE_NO				10
#define STK_ACC_CALI_VER0			0x3D
#define STK_ACC_CALI_VER1			0x01
//#define STK_ACC_CALI_FILE 			"/data/misc/stkacccali.conf"
#define STK_ACC_CALI_FILE 	 "/data/data/com.actions.sensor.calib/files/gsensor_calib.txt"

#define STK_ACC_CALI_FILE_SIZE 		10

#define STK_K_SUCCESS_TUNE			0x04
#define STK_K_SUCCESS_FT2			0x03
#define STK_K_SUCCESS_FT1			0x02
#define STK_K_SUCCESS_FILE			0x01
#define STK_K_NO_CALI					0xFF
#define STK_K_RUNNING				0xFE
#define STK_K_FAIL_LRG_DIFF			0xFD
#define STK_K_FAIL_OPEN_FILE			0xFC
#define STK_K_FAIL_W_FILE				0xFB
#define STK_K_FAIL_R_BACK			0xFA
#define STK_K_FAIL_R_BACK_COMP		0xF9
#define STK_K_FAIL_I2C					0xF8
#define STK_K_FAIL_K_PARA			0xF7
#define STK_K_FAIL_OTP_OUT_RG		0xF6
#define STK_K_FAIL_ENG_I2C			0xF5
#define STK_K_FAIL_FT1_USD			0xF4
#define STK_K_FAIL_FT2_USD			0xF3
#define STK_K_FAIL_WRITE_NOFST		0xF2
#define STK_K_FAIL_OTP_5T			0xF1
#define STK_K_FAIL_PLACEMENT		0xF0


#define STK_ZG_FILTER
#ifdef CONFIG_SENSORS_STK8312
	#define STK_ZG_COUNT	1
#elif defined (CONFIG_SENSORS_STK8313)
	#define STK_ZG_COUNT	4
#endif

#define STK_TUNE
#ifdef CONFIG_SENSORS_STK8312
	#define STK_TUNE_XYOFFSET 3
	#define STK_TUNE_ZOFFSET 6
	#define STK_TUNE_NOISE 5
#elif defined (CONFIG_SENSORS_STK8313)
	#define STK_TUNE_XYOFFSET 35
	#define STK_TUNE_ZOFFSET 75
	#define STK_TUNE_NOISE 20
#endif
#define STK_TUNE_NUM 125
#define STK_TUNE_DELAY 125

#ifdef STK_TUNE
static char stk_tune_offset_record[3] = {0};
static int stk_tune_offset[3] = {0};
static int stk_tune_sum[3] = {0};
static int stk_tune_max[3] = {0};
static int stk_tune_min[3] = {0};
static int stk_tune_index = 0;
static int stk_tune_done = 0;
#endif

#define DEBUG_ENABLE      1
#define stk831x_info(fmt,arg...) \
	do { if (DEBUG_ENABLE) printk("[STK831x][%s][LINE:%d]"fmt,__func__, __LINE__ , ##arg) ;} \
		while(0)

#define stk831x_err(fmt , arg...) \
	printk("[STK831X_ERROR][%s][LINE:%d]"fmt, __func__ , __LINE__ ,##arg)



#if defined(STK_LOWPASS)
#define MAX_FIR_LEN 32
struct data_filter {
    s16 raw[MAX_FIR_LEN][3];
    int sum[3];
    int num;
    int idx;
};
#endif

struct stk831x_data
{

	struct input_dev *input_dev;
    struct mutex enable_mutex;
    atomic_t selftest_result;
	int irq;
	int raw_data[3];

	atomic_t delay1;
    unsigned char delay;
    atomic_t enabled;
	bool re_enable;
	bool first_enable;
    struct input_dev *input;
    struct delayed_work work;
    unsigned char mode;
	atomic_t cali_status;
	struct mutex write_lock;
	char recv_reg;

#if STK_ACC_POLLING_MODE
	struct hrtimer acc_timer;
    struct work_struct stk_acc_work;
	struct workqueue_struct *stk_acc_wq;
	ktime_t acc_poll_delay;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
#endif
    atomic_t fuzz;
    atomic_t position;
    atomic_t calibrated;
    unsigned char offset_saved[3];
#if defined(STK_LOWPASS)
		atomic_t				firlength;
		atomic_t				fir_en;
		struct data_filter		fir;
#endif
#ifdef SENSOR_RESUME_WORK
	struct wake_lock resume_lock;
	struct work_struct	resume_work;
#endif
};



static int event_since_en = 0;

static void STK831X_Tune(struct stk831x_data *stk, int acc[]);
static int STK831X_GetEnable(struct stk831x_data *stk, char* gState);
static int STK831X_SetOffset(char buf[]);
static int STK831X_SetEnable(struct stk831x_data *stk, char en);
static int STK831X_GetCali(struct stk831x_data *stk);
static int STK831X_SetCali(struct stk831x_data *stk, char sstate);
static int STK831X_GetOffset(char buf[]);
static int STK831X_SetDelay(struct stk831x_data *stk, uint32_t sdelay_ns);
static int STK831X_GetDelay(struct stk831x_data *stk, uint32_t* gdelay_ns);
//static int STK831X_SetRange(char srange);
//static int STK831X_GetRange(char* grange);
static int32_t stk_get_file_content(char * r_buf, int8_t buf_size);
static int stk_store_in_file(char offset[], char mode);
static int STK831X_Init(struct stk831x_data *stk, struct i2c_client *client);


extern int get_config(const char *key, char *buff, int len);
/*******************************************
* end for xml cfg
*******************************************/

static struct i2c_client *this_client;

static int STK_i2c_Rx(char *rxData, int length)
{
	uint8_t retry;
	struct i2c_msg msgs[] =
	{
		{
			.addr = this_client->addr,
			.flags = 0,
			.len = 1,
			.buf = rxData,
		},
		{
			.addr = this_client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxData,
		},
	};

	for (retry = 0; retry <= 3; retry++)
	{
		if (i2c_transfer(this_client->adapter, msgs, 2) > 0)
			break;
		else
			mdelay(10);
	}

	if (retry > 3)
	{
		printk(KERN_ERR "%s: retry over 3\n", __func__);
		return -EIO;
	}
	else
		return 0;
}

static int STK_i2c_Tx(char *txData, int length)
{

	int retry;
	struct i2c_msg msg[] =
	{
		{
			.addr = this_client->addr,
			.flags = 0,
			.len = length,
			.buf = txData,
		},
	};

	for (retry = 0; retry <= 1; retry++)
	{
		if (i2c_transfer(this_client->adapter, msg, 1) > 0)
			break;
		else
		{
			return -EIO;
			mdelay(10);
		}
	}

	if(*txData >= 0x21 && *txData <= 0x3F)
	{
		if(*txData == 0x3F)
			msleep(1);
		for (retry = 0; retry <= 1; retry++)
		{
			if (i2c_transfer(this_client->adapter, msg, 1) > 0)
				break;
			else
				mdelay(10);
		}
	}

	if (retry > 1)
	{
		printk(KERN_ERR "%s: retry over 3\n", __func__);
		return -EIO;
	}
	else
		return 0;
}

static int STK831X_ReadByteOTP(char rReg, char *value)
{
	int redo = 0;
	int result;
	char buffer[2] = "";
	*value = 0;

	buffer[0] = 0x3D;
	buffer[1] = rReg;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto eng_i2c_r_err;
	}
	buffer[0] = 0x3F;
	buffer[1] = 0x02;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto eng_i2c_r_err;
	}

	msleep(1);
	do {
		buffer[0] = 0x3F;
		result = STK_i2c_Rx(buffer, 1);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed\n", __func__);
			goto eng_i2c_r_err;
		}
		if(buffer[0]& 0x80)
		{
			break;
		}
		msleep(1);
		redo++;
	}while(redo < 5);

	if(redo == 5)
	{
		printk(KERN_ERR "%s:OTP read repeat read 5 times! Failed!\n", __func__);
		return -STK_K_FAIL_OTP_5T;
	}
	buffer[0] = 0x3E;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto eng_i2c_r_err;
	}
	*value = buffer[0];
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s: read 0x%x=0x%x\n", __func__, rReg, *value);
#endif
	return 0;

eng_i2c_r_err:
	return -STK_K_FAIL_ENG_I2C;
}

static int STK831X_WriteByteOTP(char wReg, char value)
{

	int redo = 0;
	int result;
	char buffer[2] = "";
	char read_back;

	redo = 0;
	do {
		redo++;

		buffer[0] = 0x3D;
		buffer[1] = wReg;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);
			goto eng_i2c_w_err;
		}
		buffer[0] = 0x3E;
		buffer[1] = value;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);
			goto eng_i2c_w_err;
		}
		buffer[0] = 0x3F;
		buffer[1] = 0x01;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);
			goto eng_i2c_w_err;
		}
		msleep(1);

		buffer[0] = 0x3F;
		result = STK_i2c_Rx(buffer, 1);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);
			goto eng_i2c_w_err;
		}
		if(buffer[0]& 0x80)
		{
			result = STK831X_ReadByteOTP(wReg, &read_back);
			if(result < 0)
			{
				printk(KERN_ERR "%s: read back error, result=%d\n", __func__, result);
				goto eng_i2c_w_err;
			}

			if(read_back == value)
			{
#ifdef STK_DEBUG_CALI
				printk(KERN_INFO "%s: write 0x%x=0x%x successfully\n", __func__, wReg, value);
#endif
				break;
			}
			else
				printk(KERN_ERR "%s: read back mismatch, write 0x%x=0x%x, read 0x%x=0x%x, try again\n", __func__, wReg, value, wReg, read_back);
		}
		msleep(1);
	}while(redo < 5);

	if(redo == 5)
	{
		printk(KERN_ERR "%s:OTP write 0x%x repeat read 5 times! Failed!\n", __func__, wReg);
		return -STK_K_FAIL_OTP_5T;
	}
	return 0;

eng_i2c_w_err:
	return -STK_K_FAIL_ENG_I2C;
}


static int STK831X_SetVD(struct stk831x_data *stk)
{

	int result;
	char buffer[2] = "";
	char reg24;

	msleep(2);
	result = STK831X_ReadByteOTP(0x70, &reg24);
	if(result < 0)
	{
		printk(KERN_ERR "%s: read back error, result=%d\n", __func__, result);
		return result;
	}

	if(reg24 != 0)
	{
		buffer[0] = 0x24;
		buffer[1] = reg24;
		//printk(KERN_INFO "%s:write 0x%x to 0x24\n",  __func__, buffer[1]);
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed\n", __func__);
			return result;
		}
	}
	else
	{
		//printk(KERN_INFO "%s: reg24=0, do nothing\n", __func__);
		return 0;
	}

	buffer[0] = 0x24;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}
	if(buffer[0] != reg24)
	{
		printk(KERN_ERR "%s: error, reg24=0x%x, read=0x%x\n", __func__, reg24, buffer[0]);
		return -1;
	}
	//printk(KERN_INFO "%s: successfully", __func__);
	return 0;
}

#ifdef CONFIG_SENSORS_STK8312
static int STK831X_ReadSensorData(struct stk831x_data *stk)
{
	int result;
	char buffer[3] = "";
	int acc_xyz[3] = {0};
#ifdef STK_ZG_FILTER
	s16 zero_fir = 0;
#endif
#ifdef STK_LOWPASS
	int idx, firlength = atomic_read(&stk->firlength);
#endif
	int k_status = atomic_read(&stk->cali_status);
	memset(buffer, 0, 3);

	buffer[0] = STK831X_XOUT;
	result = STK_i2c_Rx(buffer, 3);
	if (result < 0)
	{
		printk(KERN_ERR "%s:i2c transfer error\n", __func__);
		return result;
	}

	if (buffer[0] & 0x80)
		acc_xyz[0] = buffer[0] - 256;
	else
		acc_xyz[0] = buffer[0];
	if (buffer[1] & 0x80)
		acc_xyz[1] = buffer[1] - 256;
	else
		acc_xyz[1] = buffer[1];
	if (buffer[2] & 0x80)
		acc_xyz[2] = buffer[2] - 256;
	else
		acc_xyz[2] = buffer[2];

#ifdef STK_DEBUG_RAWDATA
	printk(KERN_INFO "%s:RAW  %4d,%4d,%4d\n", __func__, stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif

	if(k_status == STK_K_RUNNING)
	{
		stk->raw_data[0] = acc_xyz[0];
		stk->raw_data[1] = acc_xyz[1];
		stk->raw_data[2] = acc_xyz[2];
		return 0;
	}


#ifdef STK_LOWPASS
	if(atomic_read(&stk->fir_en))
	{
		if(stk->fir.num < firlength)
		{
			stk->fir.raw[stk->fir.num][0] = acc_xyz[0];
			stk->fir.raw[stk->fir.num][1] = acc_xyz[1];
			stk->fir.raw[stk->fir.num][2] = acc_xyz[2];
			stk->fir.sum[0] += acc_xyz[0];
			stk->fir.sum[1] += acc_xyz[1];
			stk->fir.sum[2] += acc_xyz[2];
			stk->fir.num++;
			stk->fir.idx++;
		}
		else
		{
			idx = stk->fir.idx % firlength;
			stk->fir.sum[0] -= stk->fir.raw[idx][0];
			stk->fir.sum[1] -= stk->fir.raw[idx][1];
			stk->fir.sum[2] -= stk->fir.raw[idx][2];
			stk->fir.raw[idx][0] = acc_xyz[0];
			stk->fir.raw[idx][1] = acc_xyz[1];
			stk->fir.raw[idx][2] = acc_xyz[2];
			stk->fir.sum[0] += acc_xyz[0];
			stk->fir.sum[1] += acc_xyz[1];
			stk->fir.sum[2] += acc_xyz[2];
			stk->fir.idx++;
			acc_xyz[0] = stk->fir.sum[0]/firlength;
			acc_xyz[1] = stk->fir.sum[1]/firlength;
			acc_xyz[2] = stk->fir.sum[2]/firlength;
		}
	}
#ifdef STK_DEBUG_RAWDATA
	printk(KERN_INFO "%s:After FIR  %4d,%4d,%4d\n", __func__, stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif

#endif  	/* #ifdef STK_LOWPASS */



#ifdef STK_TUNE
	if((k_status&0xF0) != 0)
		STK831X_Tune(stk, acc_xyz);
#endif

#ifdef STK_ZG_FILTER
	if( abs(acc_xyz[0]) <= STK_ZG_COUNT)
		acc_xyz[0] = (acc_xyz[0]*zero_fir);
	if( abs(acc_xyz[1]) <= STK_ZG_COUNT)
		acc_xyz[1] = (acc_xyz[1]*zero_fir);
	if( abs(acc_xyz[2]) <= STK_ZG_COUNT)
		acc_xyz[2] = (acc_xyz[2]*zero_fir);
#endif 	/* #ifdef STK_ZG_FILTER */

	stk->raw_data[0] = acc_xyz[0];
	stk->raw_data[1] = acc_xyz[1];
	stk->raw_data[2] = acc_xyz[2];

	return 0;
}

#elif defined CONFIG_SENSORS_STK8313
static int STK831X_ReadSensorData(struct stk831x_data *stk)
{
	int result;
	char buffer[6] = "";
	int acc_xyz[3] = {0};
#ifdef STK_ZG_FILTER
	s16 zero_fir = 0;
#endif
#ifdef STK_LOWPASS
	int idx, firlength = atomic_read(&stk->firlength);
#endif
	int k_status = atomic_read(&stk->cali_status);

	memset(buffer, 0, 6);
	buffer[0] = STK831X_XOUT;
	result = STK_i2c_Rx(buffer, 6);
	if (result < 0)
	{
		printk(KERN_ERR "%s:i2c transfer error\n", __func__);
		return result;
	}

	if (buffer[0] & 0x80)
		acc_xyz[0] = ((int)buffer[0]<<4) + (buffer[1]>>4) - 4096;
	else
		acc_xyz[0] = ((int)buffer[0]<<4) + (buffer[1]>>4);
	if (buffer[2] & 0x80)
		acc_xyz[1] = ((int)buffer[2]<<4) + (buffer[3]>>4) - 4096;
	else
		acc_xyz[1] = ((int)buffer[2]<<4) + (buffer[3]>>4);
	if (buffer[4] & 0x80)
		acc_xyz[2] = ((int)buffer[4]<<4) + (buffer[5]>>4) - 4096;
	else
		acc_xyz[2] = ((int)buffer[4]<<4) + (buffer[5]>>4);

#ifdef STK_DEBUG_RAWDATA
	printk(KERN_INFO "%s:RAW  %4d,%4d,%4d\n", __func__, stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif

	if(k_status == STK_K_RUNNING)
	{
		stk->raw_data[0] = acc_xyz[0];
		stk->raw_data[1] = acc_xyz[1];
		stk->raw_data[2] = acc_xyz[2];
		return 0;
	}


#ifdef STK_LOWPASS
	if(atomic_read(&stk->fir_en))
	{
		if(stk->fir.num < firlength)
		{
			stk->fir.raw[stk->fir.num][0] = acc_xyz[0];
			stk->fir.raw[stk->fir.num][1] = acc_xyz[1];
			stk->fir.raw[stk->fir.num][2] = acc_xyz[2];
			stk->fir.sum[0] += acc_xyz[0];
			stk->fir.sum[1] += acc_xyz[1];
			stk->fir.sum[2] += acc_xyz[2];
			stk->fir.num++;
			stk->fir.idx++;
		}
		else
		{
			idx = stk->fir.idx % firlength;
			stk->fir.sum[0] -= stk->fir.raw[idx][0];
			stk->fir.sum[1] -= stk->fir.raw[idx][1];
			stk->fir.sum[2] -= stk->fir.raw[idx][2];
			stk->fir.raw[idx][0] = acc_xyz[0];
			stk->fir.raw[idx][1] = acc_xyz[1];
			stk->fir.raw[idx][2] = acc_xyz[2];
			stk->fir.sum[0] += acc_xyz[0];
			stk->fir.sum[1] += acc_xyz[1];
			stk->fir.sum[2] += acc_xyz[2];
			stk->fir.idx++;
			acc_xyz[0] = stk->fir.sum[0]/firlength;
			acc_xyz[1] = stk->fir.sum[1]/firlength;
			acc_xyz[2] = stk->fir.sum[2]/firlength;
		}
	}
#ifdef STK_DEBUG_RAWDATA
	printk(KERN_INFO "%s:After FIR  %4d,%4d,%4d\n", __func__, stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif

#endif  	/* #ifdef STK_LOWPASS */


#ifdef STK_TUNE
	if((k_status&0xF0) != 0)
		STK831X_Tune(stk, acc_xyz);
#endif

#ifdef STK_ZG_FILTER
	if( abs(acc_xyz[0]) <= STK_ZG_COUNT)
		acc_xyz[0] = (acc_xyz[0]*zero_fir);
	if( abs(acc_xyz[1]) <= STK_ZG_COUNT)
		acc_xyz[1] = (acc_xyz[1]*zero_fir);
	if( abs(acc_xyz[2]) <= STK_ZG_COUNT)
		acc_xyz[2] = (acc_xyz[2]*zero_fir);
#endif 	/* #ifdef STK_ZG_FILTER */

	stk->raw_data[0] = acc_xyz[0];
	stk->raw_data[1] = acc_xyz[1];
	stk->raw_data[2] = acc_xyz[2];

	return 0;
}
#endif


static int stk831x_axis_remap(struct stk831x_data *stk)
{
	s16 swap;
    int stk_position = atomic_read(&stk->position);

    switch (abs(stk_position)) {
        case 1:
            stk->raw_data[0] = -(stk->raw_data[0]);
           stk->raw_data[1] = -(stk->raw_data[1]);
            break;
        case 2:
            swap = stk->raw_data[0];
            stk->raw_data[0] = -stk->raw_data[1];
            stk->raw_data[1] = swap;
            break;
        case 3:
            break;
        case 4:
            swap = stk->raw_data[0];
            stk->raw_data[0] = stk->raw_data[1];
            stk->raw_data[1] = -swap;
            break;
    }

    if (stk_position < 0) {
        stk->raw_data[2] = -(stk->raw_data[2]);
        stk->raw_data[0] = -(stk->raw_data[0]);
    }

    return 0;
}


static int STK831X_ReportValue(struct stk831x_data *stk)
{
	if(event_since_en < 1200) {
		event_since_en++;
		if(event_since_en < 12)
			return 0;
	}

#ifdef STK_DEBUG_PRINT
	printk(KERN_INFO "report  X:%4d,Y:%4d, Z:%4d\n",stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif

	stk831x_axis_remap(stk);

	input_report_abs(stk->input_dev, ABS_X, stk->raw_data[0]);
	input_report_abs(stk->input_dev, ABS_Y, stk->raw_data[1]);
	input_report_abs(stk->input_dev, ABS_Z, stk->raw_data[2]);

	input_sync(stk->input_dev);
	return 0;
}

static int stk831x_read_file(char *path, char *buf, int size)
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


static int stk831x_load_user_calibration(struct stk831x_data *stk)
{
    char buffer[16];
    int ret = 0;
    int data[3];
    unsigned char offset[3];
   	char enable;

    int calibrated = atomic_read(&stk->calibrated);

    // only calibrate once
    if (calibrated) {
        goto usr_calib_end;
    } else {
        atomic_set(&stk->calibrated, 1);
    }

    ret = stk831x_read_file(CFG_GSENSOR_CALIBFILE, buffer, sizeof(buffer));
    if (ret <= 0) {
        printk(KERN_ERR "gsensor calibration file not exist!\n");
        goto usr_calib_end;
    }

    sscanf(buffer, "%d %d %d", &data[0], &data[1], &data[2]);
    offset[0] = (unsigned char) data[0];
    offset[1] = (unsigned char) data[1];
    offset[2] = (unsigned char) data[2];

    printk(KERN_INFO "user cfg_calibration: %d %d %d\n", offset[0], offset[1], offset[2]);

	STK831X_GetEnable(stk , &enable);

	stk831x_info("enable = %d \n" , enable);
	STK831X_SetEnable(stk , 0);
    if (STK831X_SetOffset(offset) < 0) {
        printk(KERN_ERR "set offset fail\n");
        goto usr_calib_end;
    }

    printk(KERN_INFO "load user calibration finished\n");
	STK831X_SetEnable(stk , enable);

usr_calib_end:
    return ret;
}


#if STK_ACC_POLLING_MODE
static void stk831x_work_func(struct work_struct *work)
{
	struct stk831x_data *stk = container_of((struct delayed_work *)work, struct stk831x_data, work);
    unsigned long delay = msecs_to_jiffies(atomic_read(&stk->delay1));

	STK831X_ReadSensorData(stk);
	STK831X_ReportValue(stk);

    schedule_delayed_work(&stk->work, delay);
}
#endif

static ssize_t stk831x_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	return scnprintf(buf, PAGE_SIZE,  "%d\n", atomic_read(&stk->enabled));
}

static ssize_t stk831x_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	struct stk831x_data *stk = i2c_get_clientdata(this_client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
	{
		printk(KERN_ERR "%s: strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}
	if ((data == 0)||(data==1))
		STK831X_SetEnable(stk,data);
	else
		printk(KERN_ERR "%s: invalud argument, data=%ld\n", __func__, data);
	return count;
}

static ssize_t stk831x_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	int ddata[3];

	printk(KERN_INFO "driver version:%s\n",STK_ACC_DRIVER_VERSION);
	STK831X_ReadSensorData(stk);
	ddata[0]= stk->raw_data[0];
	ddata[1]= stk->raw_data[1];
	ddata[2]= stk->raw_data[2];
	return scnprintf(buf, PAGE_SIZE,  "%d %d %d\n", ddata[0], ddata[1], ddata[2]);
}

static ssize_t stk831x_calibration_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	char buffer[3] = "";
	printk(KERN_INFO "driver version:%s\n",STK_ACC_DRIVER_VERSION);

	buffer[0] = STK831X_OFSX;
    STK_i2c_Rx(buffer, 3);

	return sprintf(buf, "%d %d %d\n", buffer[0] , buffer[1], buffer[2]);
}

static ssize_t stk831x_calibration_value_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int result;
	char buffer[4] = "";

	buffer[0] = STK831X_OFSX;
	buffer[1] = buf[0];
	buffer[2] = buf[1];
	buffer[3] = buf[2];

	result = STK_i2c_Tx(buffer, 4);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed\n", __func__);
			return result;
		}
		return 0;

}


static int STK831X_GetDelay(struct stk831x_data *stk, uint32_t* gdelay_ns)
{
	*gdelay_ns = (uint32_t) STK831X_SAMPLE_TIME[stk->delay] * 1000;
	return 0;
}

static ssize_t stk831x_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	return sprintf(buf , "%d\n", atomic_read(&stk->delay1));
}

static ssize_t stk831x_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct stk831x_data *stk = i2c_get_clientdata(this_client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
	{
		printk(KERN_ERR "%s: strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}

	if(data < 10)
		data = 10;
	else if(data > 20)
		data = 20;

#ifdef USE_FIXED_DELAY
	data = USE_FIXED_DELAY;
#endif

	atomic_set(&stk->delay1 , data);
	STK831X_SetDelay(stk, data*1000000);	// ms to ns

	return count;
}

static int STK831X_GetCali(struct stk831x_data *stk)
{

	char r_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	char offset[3], mode;
	int cnt, result;
	char regR[6];

#ifdef STK_TUNE
	printk(KERN_INFO "%s: stk_tune_done=%d, stk_tune_offset=%d,%d,%d\n", __func__, stk_tune_done, stk_tune_offset[0],
		stk_tune_offset[1], stk_tune_offset[2]);
#endif
	if ((stk_get_file_content(r_buf, STK_ACC_CALI_FILE_SIZE)) == 0)
	{
		if(r_buf[0] == STK_ACC_CALI_VER0 && r_buf[1] == STK_ACC_CALI_VER1)
		{
			offset[0] = r_buf[2];
			offset[1] = r_buf[3];
			offset[2] = r_buf[4];
			mode = r_buf[5];
			printk(KERN_INFO "%s:file offset:%#02x,%#02x,%#02x,%#02x\n", __func__, offset[0], offset[1], offset[2], mode);
		}
		else
		{
			printk(KERN_ERR "%s: cali version number error! r_buf=0x%x,0x%x,0x%x,0x%x,0x%x\n",
			__func__, r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4]);
		}
	}
	else
		printk(KERN_INFO "%s: No file offset\n", __func__);

	for(cnt=0x43;cnt<0x49;cnt++)
	{
		result = STK831X_ReadByteOTP(cnt, &(regR[cnt-0x43]));
		if(result < 0)
			printk(KERN_ERR "%s: STK831X_ReadByteOTP failed, ret=%d\n", __func__, result);
	}
	printk(KERN_INFO "%s: OTP offset 1:%#02x,%#02x,%#02x,%#02x,%#02x,%#02x\n", __func__, regR[0],
		regR[1], regR[2],regR[3], regR[4], regR[5]);

	for(cnt=0x53;cnt<0x59;cnt++)
	{
		result = STK831X_ReadByteOTP(cnt, &(regR[cnt-0x53]));
		if(result < 0)
			printk(KERN_ERR "%s: STK831X_ReadByteOTP failed, ret=%d\n", __func__, result);
	}
	printk(KERN_INFO "%s: OTP offset 2:%#02x,%#02x,%#02x,%#02x,%#02x,%#02x\n", __func__, regR[0],
		regR[1], regR[2],regR[3], regR[4], regR[5]);

	return 0;
}

static int STK831X_GetEnable(struct stk831x_data *stk, char* gState)
{


	*gState = atomic_read(&stk->enabled);
	return 0;
}


static int STK831X_VerifyCali(struct stk831x_data *stk, unsigned char en_dis)
{

	unsigned char axis, state;
	int acc_ave[3] = {0, 0, 0};
	const unsigned char verify_sample_no = 3;
#ifdef CONFIG_SENSORS_STK8313
	const unsigned char verify_diff = 20;
#elif defined CONFIG_SENSORS_STK8312
	const unsigned char verify_diff = 2;
#endif
	int result;
	char buffer[2] = "";
	int ret = 0;

	if(en_dis)
	{
		STK831X_SetDelay(stk, 10000000);
		buffer[0] = STK831X_MODE;
		result = STK_i2c_Rx(buffer, 1);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
			return -STK_K_FAIL_I2C;
		}
		buffer[1] = (buffer[0] & 0xF8) | 0x01;
		buffer[0] = STK831X_MODE;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
			return -STK_K_FAIL_I2C;
		}
		STK831X_SetVD(stk);
		msleep(150);
	}

	for(state=0;state<verify_sample_no;state++)
	{
		STK831X_ReadSensorData(stk);
		for(axis=0;axis<3;axis++)
			acc_ave[axis] += stk->raw_data[axis];
#ifdef STK_DEBUG_CALI
		printk(KERN_INFO "%s: acc=%d,%d,%d\n", __func__, stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif
		msleep(10);
	}

	for(axis=0;axis<3;axis++)
		acc_ave[axis] /= verify_sample_no;

	switch(stk831x_placement)
	{
	case POSITIVE_X_UP:
		acc_ave[0] -= STK_LSB_1G;
		break;
	case NEGATIVE_X_UP:
		acc_ave[0] += STK_LSB_1G;
		break;
	case POSITIVE_Y_UP:
		acc_ave[1] -= STK_LSB_1G;
		break;
	case NEGATIVE_Y_UP:
		acc_ave[1] += STK_LSB_1G;
		break;
	case POSITIVE_Z_UP:
		acc_ave[2] -= STK_LSB_1G;
		break;
	case NEGATIVE_Z_UP:
		acc_ave[2] += STK_LSB_1G;
		break;
	default:
		printk("%s: invalid stk831x_placement=%d\n", __func__, stk831x_placement);
		ret = -STK_K_FAIL_PLACEMENT;
		break;
	}
	if(abs(acc_ave[0]) > verify_diff || abs(acc_ave[1]) > verify_diff || abs(acc_ave[2]) > verify_diff)
	{
		printk(KERN_INFO "%s:Check data x:%d, y:%d, z:%d\n", __func__,acc_ave[0],acc_ave[1],acc_ave[2]);
		printk(KERN_ERR "%s:Check Fail, Calibration Fail\n", __func__);
		ret = -STK_K_FAIL_LRG_DIFF;
	}
#ifdef STK_DEBUG_CALI
	else
		printk(KERN_INFO "%s:Check data pass\n", __func__);
#endif
	if(en_dis)
	{
		buffer[0] = STK831X_MODE;
		result = STK_i2c_Rx(buffer, 1);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
			return -STK_K_FAIL_I2C;
		}
		buffer[1] = (buffer[0] & 0xF8);
		buffer[0] = STK831X_MODE;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
			return -STK_K_FAIL_I2C;
		}
	}

	return ret;
}

static int STK831X_SetDelay(struct stk831x_data *stk, uint32_t sdelay_ns)
{

	unsigned char sr_no;
	int result;
	char buffer[2] = "";
	uint32_t sdelay_us = sdelay_ns / 1000;

	for(sr_no=(STK831X_SAMPLE_TIME_NO-1);sr_no>0;sr_no--)
	{
		if(sdelay_us >= STK831X_SAMPLE_TIME[sr_no])
			break;
	}


#ifdef STK_DEBUG_PRINT
	printk(KERN_INFO "%s:sdelay_us=%d\n", __func__, sdelay_us);
#endif

	mutex_lock(&stk->write_lock);
	if(stk->delay == sr_no)
	{
		mutex_unlock(&stk->write_lock);
		return 0;
	}
	buffer[0] = STK831X_SR;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto d_err_i2c;
	}

	buffer[1] = (buffer[0] & 0xF8) | ((sr_no & 0x07) + STK831X_SAMPLE_TIME_BASE);
	buffer[0] = STK831X_SR;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto d_err_i2c;
	}
	stk->delay = sr_no;
#if STK_ACC_POLLING_MODE
//	atomic_set(stk->delay1 , STK831X_SAMPLE_TIME[sr_no]*USEC_PER_MSEC);
	//stk->acc_poll_delay = ns_to_ktime(STK831X_SAMPLE_TIME[sr_no]*USEC_PER_MSEC);
#endif

#if defined(STK_LOWPASS)
	stk->fir.num = 0;
	stk->fir.idx = 0;
	stk->fir.sum[0] = 0;
	stk->fir.sum[1] = 0;
	stk->fir.sum[2] = 0;
#endif
	mutex_unlock(&stk->write_lock);

	return 0;
d_err_i2c:
	mutex_unlock(&stk->write_lock);
	return result;
}



static int STK831X_WriteOffsetOTP(struct stk831x_data *stk, int FT, char offsetData[])
{


	char regR[6];
	char mode;
	int result;
	char buffer[2] = "";

//Check FT1
	if(FT==1)
	{
		result = STK831X_ReadByteOTP(0x7F, &regR[0]);
		if(result < 0)
			goto eng_i2c_err;

		if(regR[0]&0x10)
		{
			printk(KERN_ERR "%s: 0x7F=0x%x\n", __func__, regR[0]);
			return -STK_K_FAIL_FT1_USD;
		}
	}
	else if (FT == 2)
	{
		result = STK831X_ReadByteOTP(0x7F, &regR[0]);
		if(result < 0)
			goto eng_i2c_err;

		if(regR[0]&0x20)
		{
			printk(KERN_ERR "%s: 0x7F=0x%x\n", __func__, regR[0]);
			return -STK_K_FAIL_FT2_USD;
		}
	}

	buffer[0] = STK831X_MODE;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	mode = buffer[0];
	buffer[1] = (mode | 0x01);
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	msleep(2);

	result = STK831X_ReadByteOTP(0x30, &regR[0]);
	if(result < 0)
		goto eng_i2c_err;
	result = STK831X_ReadByteOTP(0x31, &regR[1]);
	if(result < 0)
		goto eng_i2c_err;
	result = STK831X_ReadByteOTP(0x32, &regR[2]);
	if(result < 0)
		goto eng_i2c_err;

	if(FT == 1)
	{
		result = STK831X_WriteByteOTP(0x40, regR[0]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x41, regR[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x42, regR[2]);
		if(result < 0)
			goto eng_i2c_err;
	}
	else if (FT == 2)
	{
		result = STK831X_WriteByteOTP(0x50, regR[0]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x51, regR[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x52, regR[2]);
		if(result < 0)
			goto eng_i2c_err;
	}
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s:OTP step1 Success!\n", __func__);
#endif
	buffer[0] = 0x2A;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	else
	{
		regR[0] = buffer[0];
	}
	buffer[0] = 0x2B;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	else
	{
		regR[1] = buffer[0];
	}
	buffer[0] = 0x2E;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	else
	{
		regR[2] = buffer[0];
	}
	buffer[0] = 0x2F;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	else
	{
		regR[3] = buffer[0];
	}



	buffer[0] = 0x32;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	else
	{
		regR[4] = buffer[0];
	}
	buffer[0] = 0x33;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto common_i2c_error;
	}
	else
	{
		regR[5] = buffer[0];
	}

	regR[1] = offsetData[0];
	regR[3] = offsetData[2];
	regR[5] = offsetData[1];
	if(FT==1)
	{
		result = STK831X_WriteByteOTP(0x43, regR[0]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x44, regR[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x45, regR[2]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x46, regR[3]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x47, regR[4]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x48, regR[5]);
		if(result < 0)
			goto eng_i2c_err;
	}
	else if (FT == 2)
	{
		result = STK831X_WriteByteOTP(0x53, regR[0]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x54, regR[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x55, regR[2]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x56, regR[3]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x57, regR[4]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831X_WriteByteOTP(0x58, regR[5]);
		if(result < 0)
			goto eng_i2c_err;
	}
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s:OTP step2 Success!\n", __func__);
#endif
	result = STK831X_ReadByteOTP(0x7F, &regR[0]);
	if(result < 0)
		goto eng_i2c_err;

	if(FT==1)
		regR[0] = regR[0]|0x10;
	else if(FT==2)
		regR[0] = regR[0]|0x20;

	result = STK831X_WriteByteOTP(0x7F, regR[0]);
	if(result < 0)
		goto eng_i2c_err;
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s:OTP step3 Success!\n", __func__);
#endif
	return 0;

eng_i2c_err:
	printk(KERN_ERR "%s: read/write eng i2c error, result=0x%x\n", __func__, result);
	return result;

common_i2c_error:
	printk(KERN_ERR "%s: read/write common i2c error, result=0x%x\n", __func__, result);
	return result;
}

static int stk_store_in_ic( struct stk831x_data *stk, char otp_offset[], char FT_index, unsigned char stk831x_placement)
{
	int result;
	char buffer[2] = "";

	buffer[0] = STK831X_MODE;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}
	buffer[1] = (buffer[0] & 0xF8) | 0x01;
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}
	STK831X_SetVD(stk);

	buffer[0] = 0x2B;
	buffer[1] = otp_offset[0];
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}
	buffer[0] = 0x2F;
	buffer[1] = otp_offset[2];
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}
	buffer[0] = 0x33;
	buffer[1] = otp_offset[1];
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}



#ifdef STK_DEBUG_CALI
	//printk(KERN_INFO "%s:Check All OTP Data after write 0x2B 0x2F 0x33\n", __func__);
	//STK831X_ReadAllOTP();
#endif

	msleep(150);
	result = STK831X_VerifyCali(stk, 0);
	if(result)
	{
		printk(KERN_ERR "%s: calibration check1 fail, FT_index=%d\n", __func__, FT_index);
		goto ic_err_misc;
	}
#ifdef STK_DEBUG_CALI
	//printk(KERN_INFO "\n%s:Check All OTP Data before write OTP\n", __func__);
	//STK831X_ReadAllOTP();

#endif
	//Write OTP
	printk(KERN_INFO "\n%s:Write offset data to FT%d OTP\n", __func__, FT_index);
	result = STK831X_WriteOffsetOTP(stk, FT_index, otp_offset);
	if(result < 0)
	{
		printk(KERN_INFO "%s: write OTP%d fail\n", __func__, FT_index);

		goto ic_err_misc;
	}

	buffer[0] = STK831X_MODE;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}
	buffer[1] = (buffer[0] & 0xF8);
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto ic_err_i2c_rw;
	}

	msleep(1);
	STK831X_Init(stk, this_client);
#ifdef STK_DEBUG_CALI
	//printk(KERN_INFO "\n%s:Check All OTP Data after write OTP and reset\n", __func__);
	//STK831X_ReadAllOTP();
#endif

	result = STK831X_VerifyCali(stk, 1);
	if(result)
	{
		printk(KERN_ERR "%s: calibration check2 fail\n", __func__);
		goto ic_err_misc;
	}
	return 0;

ic_err_misc:
	STK831X_Init(stk, this_client);
	//STK831X_SetEnable(stk , 1 );
	msleep(1);
	//atomic_set(&stk->cali_status, -result);
	return result;

ic_err_i2c_rw:
	printk(KERN_ERR "%s: i2c read/write error, err=0x%x\n", __func__, result);
	msleep(1);
	STK831X_Init(stk, this_client);
	atomic_set(&stk->cali_status, STK_K_FAIL_I2C);
	return result;
}


static int STK831X_SetCali(struct stk831x_data *stk, char sstate)
{
	char org_enable;
	int acc_ave[3] = {0, 0, 0};
	int state, axis;
	int new_offset[3];
	char char_offset[3] = {0};
	int result;
	char buffer[2] = "";
	char reg_offset[3] = {0};
	char store_location = sstate;
	uint32_t gdelay_ns;
	char offset[3];

	atomic_set(&stk->cali_status, STK_K_RUNNING);
	//sstate=1, STORE_OFFSET_IN_FILE
	//sstate=2, STORE_OFFSET_IN_IC
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s:store_location=%d, stk831x_placement=%d\n", __func__, store_location, stk831x_placement);
#endif
	printk(KERN_INFO "%s:store_location=%d, stk831x_placement=%d\n", __func__, store_location, stk831x_placement);
	if((store_location != 3 && store_location != 2 && store_location != 1) || (stk831x_placement < 0 || stk831x_placement > 5) )
	{
		printk(KERN_ERR "%s, invalid parameters\n", __func__);
		atomic_set(&stk->cali_status, STK_K_FAIL_K_PARA);
		return -STK_K_FAIL_K_PARA;
	}
	STK831X_GetDelay(stk, &gdelay_ns);
	STK831X_GetEnable(stk, &org_enable);
	if(org_enable)
		STK831X_SetEnable(stk, 0);
	STK831X_SetDelay(stk, 10000000);
	STK831X_SetOffset(reg_offset);
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto err_i2c_rw;
	}
	buffer[1] = (buffer[0] & 0xF8) | 0x01;
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto err_i2c_rw;
	}

	STK831X_SetVD(stk);
	if(store_location >= 2)
	{
		buffer[0] = 0x2B;
		buffer[1] = 0x0;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			goto err_i2c_rw;
		}
		buffer[0] = 0x2F;
		buffer[1] = 0x0;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			goto err_i2c_rw;
		}
		buffer[0] = 0x33;
		buffer[1] = 0x0;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0)
		{
			goto err_i2c_rw;
		}
	}

	msleep(150);
	for(state=0;state<STK_SAMPLE_NO;state++)
	{
		STK831X_ReadSensorData(stk);
		for(axis=0;axis<3;axis++)
			acc_ave[axis] += stk->raw_data[axis];
#ifdef STK_DEBUG_CALI
		printk(KERN_INFO "%s: acc=%d,%d,%d\n", __func__, stk->raw_data[0], stk->raw_data[1], stk->raw_data[2]);
#endif
		msleep(10);
	}
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		goto err_i2c_rw;
	}
	buffer[1] = (buffer[0] & 0xF8);
	buffer[0] = STK831X_MODE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		goto err_i2c_rw;
	}

	for(axis=0;axis<3;axis++)
		acc_ave[axis] /= STK_SAMPLE_NO;

	if(acc_ave[2]<=0)
		stk831x_placement = NEGATIVE_Z_UP;
	else
		stk831x_placement = POSITIVE_Z_UP;


	switch(stk831x_placement)
	{
	case POSITIVE_X_UP:
		acc_ave[0] -= STK_LSB_1G;
		break;
	case NEGATIVE_X_UP:
		acc_ave[0] += STK_LSB_1G;
		break;
	case POSITIVE_Y_UP:
		acc_ave[1] -= STK_LSB_1G;
		break;
	case NEGATIVE_Y_UP:
		acc_ave[1] += STK_LSB_1G;
		break;
	case POSITIVE_Z_UP:
		acc_ave[2] -= STK_LSB_1G;
		break;
	case NEGATIVE_Z_UP:
		acc_ave[2] += STK_LSB_1G;
		break;
	default:
		printk("%s: invalid stk831x_placement=%d\n", __func__, stk831x_placement);
		atomic_set(&stk->cali_status, STK_K_FAIL_PLACEMENT);
		return -STK_K_FAIL_K_PARA;
		break;
	}

	for(axis=0;axis<3;axis++)
	{
		acc_ave[axis] = -acc_ave[axis];
		new_offset[axis] = acc_ave[axis];
		char_offset[axis] = new_offset[axis];
	}
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s: New offset:%d,%d,%d\n", __func__, new_offset[0], new_offset[1], new_offset[2]);
#endif
	if(store_location == 1)
	{

		STK831X_SetOffset(char_offset);
		msleep(1);
		STK831X_GetOffset(reg_offset);
		for(axis=0;axis<3;axis++)
		{
			if(char_offset[axis] != reg_offset[axis])
			{
				printk(KERN_ERR "%s: set offset to register fail!, char_offset[%d]=%d,reg_offset[%d]=%d\n",
					__func__, axis,char_offset[axis], axis, reg_offset[axis]);
				atomic_set(&stk->cali_status, STK_K_FAIL_WRITE_NOFST);
				return -STK_K_FAIL_WRITE_NOFST;
			}
		}

		result = STK831X_VerifyCali(stk, 1);
		if(result)
		{
			printk(KERN_ERR "%s: calibration check fail, result=0x%x\n", __func__, result);
			atomic_set(&stk->cali_status, -result);
		}
		else
		{
			result = stk_store_in_file(char_offset, STK_K_SUCCESS_FILE);
			if(result)
			{
				printk(KERN_INFO "%s:write calibration failed\n", __func__);
				atomic_set(&stk->cali_status, -result);
			}
			else
			{
				printk(KERN_INFO "%s  Line %d successfully\n", __func__, __LINE__);
				atomic_set(&stk->cali_status, STK_K_SUCCESS_FILE);
			}

		}
	}
	else if(store_location >= 2)
	{
		for(axis=0; axis<3; axis++)
		{
#ifdef CONFIG_SENSORS_STK8313
			new_offset[axis]>>=2;
#endif
			char_offset[axis] = (char)new_offset[axis];
			if( (char_offset[axis]>>7)==0)
			{
				if(char_offset[axis] >= 0x20 )
				{
					printk(KERN_ERR "%s: offset[%d]=0x%x is too large, limit to 0x1f\n", __func__, axis, char_offset[axis] );
					char_offset[axis] = 0x1F;
					//atomic_set(&stk->cali_status, STK_K_FAIL_OTP_OUT_RG);
					//return -STK_K_FAIL_OTP_OUT_RG;
				}
			}
			else
			{
				if(char_offset[axis] <= 0xDF)
				{
					printk(KERN_ERR "%s: offset[%d]=0x%x is too large, limit to 0x20\n", __func__, axis, char_offset[axis]);
					char_offset[axis] = 0x20;
					//atomic_set(&stk->cali_status, STK_K_FAIL_OTP_OUT_RG);
					//return -STK_K_FAIL_OTP_OUT_RG;
				}
				else
					char_offset[axis] = char_offset[axis] & 0x3f;
			}
		}

		printk(KERN_INFO "%s: OTP offset:0x%x,0x%x,0x%x\n", __func__, char_offset[0], char_offset[1], char_offset[2]);
		if(store_location == 2)
		{
			result = stk_store_in_ic( stk, char_offset, 1, stk831x_placement);
			if(result == 0)
			{
				printk(KERN_INFO "%s successfully\n", __func__);
				atomic_set(&stk->cali_status, STK_K_SUCCESS_FT1);
			}
			else
			{
				printk(KERN_ERR "%s fail, result=%d\n", __func__, result);
			}
		}
		else if(store_location == 3)
		{
			result = stk_store_in_ic( stk, char_offset, 2, stk831x_placement);
			if(result == 0)
			{
				printk(KERN_INFO "%s successfully\n", __func__);
				atomic_set(&stk->cali_status, STK_K_SUCCESS_FT2);
			}
			else
			{
				printk(KERN_ERR "%s fail, result=%d\n", __func__, result);
			}
		}
		offset[0] = offset[1] = offset[2] = 0;
		stk_store_in_file(offset, store_location);
	}
#ifdef STK_TUNE
	stk_tune_done = 1;
#endif
	stk->first_enable = false;
	STK831X_SetDelay(stk, gdelay_ns);

	if(org_enable)
		STK831X_SetEnable(stk, 1);
	return 0;

err_i2c_rw:
	stk->first_enable = false;
	if(org_enable)
		STK831X_SetEnable(stk, 1);
	printk(KERN_ERR "%s: i2c read/write error, err=0x%x\n", __func__, result);
	atomic_set(&stk->cali_status, STK_K_FAIL_I2C);
	return result;
}


static ssize_t stk831x_cali_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	int status = atomic_read(&stk->cali_status);

	if(status != STK_K_RUNNING)
		STK831X_GetCali(stk);
	return scnprintf(buf, PAGE_SIZE,  "%02x\n", status);
}

static ssize_t stk831x_cali_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	error = strict_strtoul(buf, 10, &data);
	if (error)
	{
		printk(KERN_ERR "%s: strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}

//	if (data == 0)
//		data = 1;
//	else
//		data = 2;
	STK831X_SetCali(stk, data);

//	STK831X_SetEnable(stk , 1 );
	return count;
}

static ssize_t stk831x_send_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{


	int error, i;
	char *token[2];
	int w_reg[2];
	char buffer[2] = "";

	for (i = 0; i < 2; i++)
		token[i] = strsep((char **)&buf, " ");
	if((error = strict_strtoul(token[0], 16, (unsigned long *)&(w_reg[0]))) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}
	if((error = strict_strtoul(token[1], 16, (unsigned long *)&(w_reg[1]))) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}
	printk(KERN_INFO "%s: reg[0x%x]=0x%x\n", __func__, w_reg[0], w_reg[1]);
	buffer[0] = w_reg[0];
	buffer[1] = w_reg[1];
	error = STK_i2c_Tx(buffer, 2);
	if (error < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return error;
	}
	return count;
}

static ssize_t stk831x_recv_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{


	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	return scnprintf(buf, PAGE_SIZE,  "%02x\n", stk->recv_reg);
}

static ssize_t stk831x_recv_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{


	char buffer[2] = "";
	unsigned long data;
	int error;
	struct stk831x_data *stk = i2c_get_clientdata(this_client);

	error = strict_strtoul(buf, 16, &data);
	if (error)
	{
		printk(KERN_ERR "%s: strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}

	buffer[0] = data;
	error = STK_i2c_Rx(buffer, 2);
	if (error < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return error;
	}
	stk->recv_reg = buffer[0];
	printk(KERN_INFO "%s: reg[0x%x]=0x%x\n", __func__, (int)data , (int)buffer[0]);
	return count;
}

static ssize_t stk831x_allreg_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{


	int error;
	char buffer[16] = "";
	int aa,bb;

	for(bb=0;bb<4;bb++)
	{
		buffer[0] = bb * 0x10;
		error = STK_i2c_Rx(buffer, 16);
		if (error < 0)
		{
			printk(KERN_ERR "%s:failed\n", __func__);
			return error;
		}
		for(aa=0;aa<16;aa++)
			printk(KERN_INFO "stk reg[0x%x]=0x%x\n", (bb*0x10+aa) , buffer[aa]);
	}
	return 0;
}

static ssize_t stk831x_sendo_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{


	int error, i;
	char *token[2];
	int w_reg[2];
	char buffer[2] = "";

	for (i = 0; i < 2; i++)
		token[i] = strsep((char **)&buf, " ");
	if((error = strict_strtoul(token[0], 16, (unsigned long *)&(w_reg[0]))) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}
	if((error = strict_strtoul(token[1], 16, (unsigned long *)&(w_reg[1]))) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}
	printk(KERN_INFO "%s: reg[0x%x]=0x%x\n", __func__, w_reg[0], w_reg[1]);

	buffer[0] = w_reg[0];
	buffer[1] = w_reg[1];
	error = STK831X_WriteByteOTP(buffer[0], buffer[1]);
	if (error < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return error;
	}
	return count;
}


static ssize_t stk831x_recvo_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{


	char buffer[2] = "";
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 16, &data);
	if (error)
	{
		printk(KERN_ERR "%s: strict_strtoul failed, error=0x%x\n", __func__, error);
		return error;
	}

	buffer[0] = data;
	error = STK831X_ReadByteOTP(buffer[0], &buffer[1]);
	if (error < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return error;
	}
	printk(KERN_INFO "%s: reg[0x%x]=0x%x\n", __func__, buffer[0] , buffer[1]);
	return count;
}

static ssize_t stk831x_firlen_show(struct device *dev,
struct device_attribute *attr, char *buf)
{


#ifdef STK_LOWPASS
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	int len = atomic_read(&stk->firlength);

	if(atomic_read(&stk->firlength))
	{
		printk(KERN_INFO "len = %2d, idx = %2d\n", stk->fir.num, stk->fir.idx);
		printk(KERN_INFO "sum = [%5d %5d %5d]\n", stk->fir.sum[0], stk->fir.sum[1], stk->fir.sum[2]);
		printk(KERN_INFO "avg = [%5d %5d %5d]\n", stk->fir.sum[0]/len, stk->fir.sum[1]/len, stk->fir.sum[2]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&stk->firlength));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}

static ssize_t stk831x_firlen_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{


#ifdef STK_LOWPASS
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	int error;
	unsigned long data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
	{
		printk(KERN_ERR "%s: strict_strtoul failed, error=%d\n", __func__, error);
		return error;
	}

	if(data > MAX_FIR_LEN)
	{
		printk(KERN_ERR "%s: firlen exceed maximum filter length\n", __func__);
	}
	else if (data < 1)
	{
		atomic_set(&stk->firlength, 1);
		atomic_set(&stk->fir_en, 0);
		memset(&stk->fir, 0x00, sizeof(stk->fir));
	}
	else
	{
		atomic_set(&stk->firlength, data);
		memset(&stk->fir, 0x00, sizeof(stk->fir));
		atomic_set(&stk->fir_en, 1);
	}
#else
	printk(KERN_ERR "%s: firlen is not supported\n", __func__);
#endif
	return count;
}

static ssize_t stk831x_board_position_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    int data;
    struct stk831x_data *stk = i2c_get_clientdata(this_client);

    data = atomic_read(&(stk->position));

    return sprintf(buf, "%d\n", data);
}

static ssize_t stk831x_board_position_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    unsigned long data;
    int error;
   struct stk831x_data *stk = i2c_get_clientdata(this_client);

    error = strict_strtol(buf, 10, &data);
    if (error)
        return error;

    atomic_set(&(stk->position), (int) data);

    return count;
}

static ssize_t stk831x_calibration_run_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
	struct stk831x_data *stk = i2c_get_clientdata(this_client);

	STK831X_SetCali(stk, 1);

	return count;
}


static ssize_t stk831x_calibration_reset_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
	struct stk831x_data *stk = i2c_get_clientdata(this_client);
	char offset[3];
	char enable;

	offset[0] = offset[1] = offset[2] = 0;
	STK831X_GetEnable(stk , &enable);
	STK831X_SetEnable(stk , 0);
	STK831X_SetOffset(offset);
	stk_store_in_file(offset, STK_K_NO_CALI);
	atomic_set(&stk->cali_status, STK_K_NO_CALI);
	STK831X_SetEnable(stk , enable);

    return count;
}

#if (0)
static DEVICE_ATTR(enable, 0666, stk831x_enable_show, stk831x_enable_store);
static DEVICE_ATTR(value, 0444, stk831x_value_show, NULL);
static DEVICE_ATTR(calibration_value, 0666, stk831x_calibration_value_show, stk831x_calibration_value_store);
static DEVICE_ATTR(delay, 0666, stk831x_delay_show, stk831x_delay_store);
static DEVICE_ATTR(cali, 0666, stk831x_cali_show, stk831x_cali_store);
static DEVICE_ATTR(send, 0222, NULL, stk831x_send_store);
static DEVICE_ATTR(recv, 0666, stk831x_recv_show, stk831x_recv_store);
static DEVICE_ATTR(allreg, 0444, stk831x_allreg_show, NULL);
static DEVICE_ATTR(sendo, 0222, NULL, stk831x_sendo_store);
static DEVICE_ATTR(board_position, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
        stk831x_board_position_show, stk831x_board_position_store);

static DEVICE_ATTR(calibration_run, S_IWUSR|S_IWGRP|S_IWOTH,
        NULL, stk831x_calibration_run_store);
static DEVICE_ATTR(calibration_reset, S_IWUSR|S_IWGRP|S_IWOTH,
        NULL, stk831x_calibration_reset_store);

static DEVICE_ATTR(recvo, 0222, NULL, stk831x_recvo_store);
static DEVICE_ATTR(firlen, 0666, stk831x_firlen_show, stk831x_firlen_store);
#else
static DEVICE_ATTR(enable, 0664, stk831x_enable_show, stk831x_enable_store);
static DEVICE_ATTR(value, 0444, stk831x_value_show, NULL);
static DEVICE_ATTR(calibration_value, 0664, stk831x_calibration_value_show, stk831x_calibration_value_store);
static DEVICE_ATTR(delay, 0664, stk831x_delay_show, stk831x_delay_store);
static DEVICE_ATTR(cali, 0664, stk831x_cali_show, stk831x_cali_store);
static DEVICE_ATTR(send, 0220, NULL, stk831x_send_store);
static DEVICE_ATTR(recv, 0664, stk831x_recv_show, stk831x_recv_store);
static DEVICE_ATTR(allreg, 0444, stk831x_allreg_show, NULL);
static DEVICE_ATTR(sendo, 0220, NULL, stk831x_sendo_store);
static DEVICE_ATTR(board_position, S_IRUGO|S_IWUSR|S_IWGRP,
        stk831x_board_position_show, stk831x_board_position_store);

static DEVICE_ATTR(calibration_run, S_IWUSR|S_IWGRP,
        NULL, stk831x_calibration_run_store);
static DEVICE_ATTR(calibration_reset, S_IWUSR|S_IWGRP,
        NULL, stk831x_calibration_reset_store);

static DEVICE_ATTR(recvo, 0222, NULL, stk831x_recvo_store);
static DEVICE_ATTR(firlen, 0664, stk831x_firlen_show, stk831x_firlen_store);

#endif

static struct attribute *stk831x_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_value.attr,
	&dev_attr_delay.attr,
	&dev_attr_cali.attr,
	&dev_attr_calibration_value.attr,
	&dev_attr_send.attr,
	&dev_attr_recv.attr,
	&dev_attr_allreg.attr,
	&dev_attr_board_position.attr,
	&dev_attr_sendo.attr,
	&dev_attr_recvo.attr,
	&dev_attr_calibration_reset.attr,
	&dev_attr_calibration_run.attr,
	&dev_attr_firlen.attr,
	NULL
};



static struct attribute_group stk831x_attribute_group = {
	//.name = "driver",
	.attrs = stk831x_attributes,
};

#ifdef SENSOR_RESUME_WORK
static void STK831X_work_resume(struct work_struct *work)
{
	struct stk831x_data *stk = container_of(work, struct stk831x_data, resume_work);
#ifdef STK_RESUME_RE_INIT
	int error;
#endif

	printk(KERN_INFO "%s\n", __func__);
#ifdef STK_RESUME_RE_INIT
	error = STK831X_Init(stk, this_client);
	if (error)
	{
		printk(KERN_ERR "%s:stk831x initialization failed\n", __func__);
		return;
	}
	stk->first_enable = true;
#endif
	if(stk->re_enable)
	{
		stk->re_enable = false;
		STK831X_SetEnable(stk, 1);
	}
	wake_unlock(&stk->resume_lock);
}
#endif

static int stk831x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int error;
	struct stk831x_data *stk;
	int cfg_position = 0;

	printk(KERN_INFO "stk831x_probe: driver version:%s\n",STK_ACC_DRIVER_VERSION);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		printk(KERN_ERR "%s:i2c_check_functionality error\n", __func__);
		error = -ENODEV;
		goto exit_i2c_check_functionality_error;
	}

	stk = kzalloc(sizeof(struct stk831x_data),GFP_KERNEL);
	if (!stk)
	{
		printk(KERN_ERR "%s:memory allocation error\n", __func__);
		error = -ENOMEM;
		goto exit_kzalloc_error;
	}
	stk831x_data_ptr = stk;
	mutex_init(&stk->write_lock);

#if (STK_ACC_POLLING_MODE)

	stk831x_info("Use Polling_mode\n");

	atomic_set(&stk->delay1, MAX_DELAY);
	atomic_set(&stk->calibrated , 0);
	INIT_DELAYED_WORK(&stk->work, stk831x_work_func);

#endif	//#if STK_ACC_POLLING_MODE

	i2c_set_clientdata(client, stk);
	this_client = client;

	error = STK831X_Init(stk, client);
	if (error)
	{
		stk831x_info(KERN_ERR "%s:stk831x initialization failed\n", __func__);
		return error;
	}

#if CFG_GSENSOR_USE_CONFIG > 0
		/*get xml cfg*/
		error = get_config(CFG_GSENSOR_POSITION, (char *)(&cfg_position), sizeof(int));
		if (error != 0) {
			printk(KERN_ERR "get position %d fail\n", cfg_position);
			cfg_position = 4;
		}
#else
		cfg_position = 4;
#endif

	atomic_set(&stk->position, cfg_position);

	atomic_set(&stk->cali_status, STK_K_NO_CALI);
	stk->first_enable = true;
	stk->re_enable = false;
	atomic_set(&(stk->position), (int)SKT_ROTATE);	// add by jhkim

	stk->input_dev = input_allocate_device();
	if (!stk->input_dev)
	{
		error = -ENOMEM;
		printk(KERN_ERR "%s:input_allocate_device failed\n", __func__);
		goto exit_input_dev_alloc_error;
	}

	stk->input_dev->name = ACC_IDEVICE_NAME;

	set_bit(EV_ABS, stk->input_dev->evbit);
#ifdef CONFIG_SENSORS_STK8312
	input_set_abs_params(stk->input_dev, ABS_X, -128, 127, 0, 0);
	input_set_abs_params(stk->input_dev, ABS_Y, -128, 127, 0, 0);
	input_set_abs_params(stk->input_dev, ABS_Z, -128, 127, 0, 0);
#elif defined CONFIG_SENSORS_STK8313
	input_set_abs_params(stk->input_dev, ABS_X, -512, 511, 0, 0);
	input_set_abs_params(stk->input_dev, ABS_Y, -512, 511, 0, 0);
	input_set_abs_params(stk->input_dev, ABS_Z, -512, 511, 0, 0);
#endif

	error = input_register_device(stk->input_dev);
	if (error)
	{
		printk(KERN_ERR "%s:Unable to register input device: %s\n", __func__, stk->input_dev->name);
		goto exit_input_register_device_error;
	}


	error = sysfs_create_group(&stk->input_dev->dev.kobj, &stk831x_attribute_group);
	if (error)
	{
		printk(KERN_ERR "%s: sysfs_create_group failed\n", __func__);
		goto exit_sysfs_create_group_error;
	}

	stk831x_load_user_calibration(stk);

	/* add by jhkim, default enable */
	STK831X_SetEnable(stk, 1);
#ifdef SENSOR_RESUME_WORK
	INIT_WORK(&stk->resume_work, STK831X_work_resume);
	wake_lock_init(&stk->resume_lock, WAKE_LOCK_SUSPEND, "stk831x");
#endif

	printk(KERN_INFO "stk831x probe successfully (delay:%dms)\n", atomic_read(&stk->delay1));
	return 0;

exit_sysfs_create_group_error:
	sysfs_remove_group(&stk->input_dev->dev.kobj, &stk831x_attribute_group);
exit_input_dev_alloc_error:

exit_input_register_device_error:
	input_unregister_device(stk->input_dev);

//exit_create_workqueue_error:
	mutex_destroy(&stk->write_lock);
	kfree(stk);
	stk = NULL;
exit_kzalloc_error:
exit_i2c_check_functionality_error:
	return error;
}

static int stk831x_remove(struct i2c_client *client)
{
/*
	struct stk831x_data *stk = i2c_get_clientdata(client);

	sysfs_remove_group(&stk->input_dev->dev.kobj, &stk831x_attribute_group);
	misc_deregister(&stk_device);
	input_unregister_device(stk->input_dev);
	cancel_work_sync(&stk->stk_work);
#if (STK_ACC_POLLING_MODE)
	hrtimer_try_to_cancel(&stk->acc_timer);
	destroy_workqueue(stk->stk_acc_wq);
#else
	free_irq(client->irq, stk);
#if ADDITIONAL_GPIO_CFG
	gpio_free( STK_INT_PIN );
#endif //#if ADDITIONAL_GPIO_CFG
	if (stk_mems_work_queue)
		destroy_workqueue(stk_mems_work_queue);
#endif	//#if (!STK_ACC_POLLING_MODE)
	mutex_destroy(&stk->write_lock);
	kfree(stk);
	stk = NULL;	*/
	return 0;
}

#ifdef CONFIG_PM

static int32_t stk_get_file_content(char * r_buf, int8_t buf_size)
{
	struct file  *cali_file;
	mm_segment_t fs;
	ssize_t ret;

    cali_file = filp_open(STK_ACC_CALI_FILE, O_RDONLY,0);
    if(IS_ERR(cali_file))
	{
        printk(KERN_ERR "%s: filp_open error, no offset file!\n", __func__);
        return -ENOENT;
	}
	else
	{
		fs = get_fs();
		set_fs(get_ds());
		ret = cali_file->f_op->read(cali_file,r_buf, STK_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret < 0)
		{
			printk(KERN_ERR "%s: read error, ret=%d\n", __func__, ret);
			filp_close(cali_file,NULL);
			return -EIO;
		}
		set_fs(fs);
    }

    filp_close(cali_file,NULL);
	return 0;
}


static int STK831X_SetOffset(char buf[])
{

	int result;
	char buffer[4] = "";

	buffer[0] = STK831X_OFSX;
	buffer[1] = buf[0];
	buffer[2] = buf[1];
	buffer[3] = buf[2];


	result = STK_i2c_Tx(buffer, 4);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}
	return 0;
}

static int STK831X_GetOffset(char buf[])
{

	int result;
	char buffer[3] = "";

	buffer[0] = STK831X_OFSX;
	result = STK_i2c_Rx(buffer, 3);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}
	buf[0] = buffer[0];
	buf[1] = buffer[1];
	buf[2] = buffer[2];
	return 0;
}

static int stk_store_in_file(char offset[], char mode)
{


	struct file  *cali_file;
	char r_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	char w_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	mm_segment_t fs;
	ssize_t ret;
	int8_t i;

	w_buf[0] = STK_ACC_CALI_VER0;
	w_buf[1] = STK_ACC_CALI_VER1;
	w_buf[2] = offset[0];
	w_buf[3] = offset[1];
	w_buf[4] = offset[2];
	w_buf[5] = mode;

    cali_file = filp_open(STK_ACC_CALI_FILE, O_CREAT | O_RDWR,0664);

    if(IS_ERR(cali_file))
	{
        printk(KERN_ERR "%s: filp_open error!\n", __func__);
        return -STK_K_FAIL_OPEN_FILE;
	}
	else
	{
		fs = get_fs();
		set_fs(get_ds());

		ret = cali_file->f_op->write(cali_file,w_buf,STK_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret != STK_ACC_CALI_FILE_SIZE)
		{
			printk(KERN_ERR "%s: write error!\n", __func__);
			filp_close(cali_file,NULL);
			return -STK_K_FAIL_W_FILE;
		}
		cali_file->f_pos=0x00;
		ret = cali_file->f_op->read(cali_file,r_buf, STK_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret < 0)
		{
			printk(KERN_ERR "%s: read error!\n", __func__);
			filp_close(cali_file,NULL);
			return -STK_K_FAIL_R_BACK;
		}
		set_fs(fs);

		//printk(KERN_INFO "%s: read ret=%d!\n", __func__, ret);
		for(i=0;i<STK_ACC_CALI_FILE_SIZE;i++)
		{
			if(r_buf[i] != w_buf[i])
			{
				printk(KERN_ERR "%s: read back error, r_buf[%x](0x%x) != w_buf[%x](0x%x)\n",
					__func__, i, r_buf[i], i, w_buf[i]);
				filp_close(cali_file,NULL);
				return -STK_K_FAIL_R_BACK_COMP;
			}
		}
    }
    filp_close(cali_file,NULL);

	return 0;
}


#ifdef STK_TUNE
static void STK831X_ResetPara(void)
{



	int ii;
	for(ii=0;ii<3;ii++)
	{
		stk_tune_sum[ii] = 0;
		stk_tune_min[ii] = 4096;
		stk_tune_max[ii] = -4096;
	}
	return;
}

static void STK831X_Tune(struct stk831x_data *stk, int acc[])
{

	//

	int ii;
	char offset[3];
	char mode_reg;
	int result;
	char buffer[2] = "";

	if (stk_tune_done==0)
	{
		if( event_since_en >= STK_TUNE_DELAY)
		{
			if ((abs(acc[0]) <= STK_TUNE_XYOFFSET) && (abs(acc[1]) <= STK_TUNE_XYOFFSET)
				&& (abs(abs(acc[2])-STK_LSB_1G) <= STK_TUNE_ZOFFSET))
				stk_tune_index++;
			else
				stk_tune_index = 0;

			if (stk_tune_index==0)
				STK831X_ResetPara();
			else
			{
				for(ii=0;ii<3;ii++)
				{
					stk_tune_sum[ii] += acc[ii];
					if(acc[ii] > stk_tune_max[ii])
						stk_tune_max[ii] = acc[ii];
					if(acc[ii] < stk_tune_min[ii])
						stk_tune_min[ii] = acc[ii];
				}
			}

			if(stk_tune_index == STK_TUNE_NUM)
			{
				for(ii=0;ii<3;ii++)
				{
					if((stk_tune_max[ii] - stk_tune_min[ii]) > STK_TUNE_NOISE)
					{
						stk_tune_index = 0;
						STK831X_ResetPara();
						return;
					}
				}
				buffer[0] = STK831X_MODE;
				result = STK_i2c_Rx(buffer, 1);
				if (result < 0)
				{
					printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
					return;
				}
				mode_reg = buffer[0];
				buffer[1] = mode_reg & 0xF8;
				buffer[0] = STK831X_MODE;
				result = STK_i2c_Tx(buffer, 2);
				if (result < 0)
				{
					printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
					return;
				}

				stk_tune_offset[0] = stk_tune_sum[0]/STK_TUNE_NUM;
				stk_tune_offset[1] = stk_tune_sum[1]/STK_TUNE_NUM;
				if (acc[2] > 0)
					stk_tune_offset[2] = stk_tune_sum[2]/STK_TUNE_NUM - STK_LSB_1G;
				else
					stk_tune_offset[2] = stk_tune_sum[2]/STK_TUNE_NUM - (-STK_LSB_1G);

				offset[0] = (char) (-stk_tune_offset[0]);
				offset[1] = (char) (-stk_tune_offset[1]);
				offset[2] = (char) (-stk_tune_offset[2]);
				STK831X_SetOffset(offset);
				stk_tune_offset_record[0] = offset[0];
				stk_tune_offset_record[1] = offset[1];
				stk_tune_offset_record[2] = offset[2];

				buffer[1] = mode_reg | 0x1;
				buffer[0] = STK831X_MODE;
				result = STK_i2c_Tx(buffer, 2);
				if (result < 0)
				{
					printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
					return;
				}

				STK831X_SetVD(stk);
				stk_store_in_file(offset, STK_K_SUCCESS_TUNE);
				stk_tune_done = 1;
				atomic_set(&stk->cali_status, STK_K_SUCCESS_TUNE);
				event_since_en = 0;
				printk(KERN_INFO "%s:TUNE done, %d,%d,%d\n", __func__, offset[0], offset[1],offset[2]);
			}
		}
	}

	return;
}
#endif


static void stk_handle_first_en(struct stk831x_data *stk)
{


	char r_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	char offset[3];
	char mode;
	printk(KERN_INFO "%s: In.\n", __func__);
#ifdef STK_TUNE
	if(stk_tune_offset_record[0]!=0 || stk_tune_offset_record[1]!=0 || stk_tune_offset_record[2]!=0)
	{
		STK831X_SetOffset(stk_tune_offset_record);
		stk_tune_done = 1;
		atomic_set(&stk->cali_status, STK_K_SUCCESS_TUNE);
		printk(KERN_INFO "%s: set offset:%d,%d,%d\n", __func__, stk_tune_offset_record[0], stk_tune_offset_record[1],stk_tune_offset_record[2]);
	}
	else if ((stk_get_file_content(r_buf, STK_ACC_CALI_FILE_SIZE)) == 0)
#else
	if ((stk_get_file_content(r_buf, STK_ACC_CALI_FILE_SIZE)) == 0)
#endif
	{
		if(r_buf[0] == STK_ACC_CALI_VER0 && r_buf[1] == STK_ACC_CALI_VER1)
		{
			offset[0] = r_buf[2];
			offset[1] = r_buf[3];
			offset[2] = r_buf[4];
			mode = r_buf[5];
			STK831X_SetOffset(offset);
#ifdef STK_TUNE
			stk_tune_offset_record[0] = offset[0];
			stk_tune_offset_record[1] = offset[1];
			stk_tune_offset_record[2] = offset[2];
#endif
			printk(KERN_INFO "%s: set offset:%d,%d,%d, mode=%d\n", __func__, offset[0], offset[1], offset[2], mode);
			atomic_set(&stk->cali_status, mode);
		}
		else
		{
			printk(KERN_ERR "%s: cali version number error! r_buf=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				__func__, r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4]);
			//return -EINVAL;
		}
	}
	else
	{
		offset[0] = offset[1] = offset[2] = 0;
		stk_store_in_file(offset, STK_K_NO_CALI);
		atomic_set(&stk->cali_status, STK_K_NO_CALI);
	}
	printk(KERN_INFO "%s: Out. cali_status = 0x%x\n", __func__, atomic_read(&stk->cali_status));
	return;
}

static int32_t stk_get_ic_content(struct stk831x_data *stk)
{


	int result;
	char regR;

	result = STK831X_ReadByteOTP(0x7F, &regR);
	if(result < 0)
	{
		printk(KERN_ERR "%s: read/write eng i2c error, result=0x%x\n", __func__, result);
		return result;
	}

	if(regR&0x20)
	{
		atomic_set(&stk->cali_status, STK_K_SUCCESS_FT2);
		printk(KERN_INFO "%s: OTP 2 used\n", __func__);
		return 2;
	}
	if(regR&0x10)
	{
		atomic_set(&stk->cali_status, STK_K_SUCCESS_FT1);
		printk(KERN_INFO "%s: OTP 1 used\n", __func__);
		return 1;
	}
	return 0;
}



static int STK831X_SetEnable(struct stk831x_data *stk, char en)
{
	int result;
	char buffer[2] = "";
	int new_enabled = (en)?1:0;
	int k_status = atomic_read(&stk->cali_status);

	if(new_enabled == atomic_read(&stk->enabled))
		return 0;
	printk(KERN_INFO "%s:%x\n", __func__, en);

	//mutex_lock(&stk->write_lock);
	if(stk->first_enable && k_status != STK_K_RUNNING)
		stk_handle_first_en(stk);

	buffer[0] = STK831X_MODE;
	result = STK_i2c_Rx(buffer, 1);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto e_err_i2c;
	}
	if(en)
	{
		buffer[1] = (buffer[0] & 0xF8) | 0x01;
		event_since_en = 0;
#ifdef STK_TUNE
		if((k_status&0xF0) != 0 && stk_tune_done == 0)
		{
			stk_tune_index = 0;
			STK831X_ResetPara();
		}
#endif
	}
	else
		buffer[1] = (buffer[0] & 0xF8);

	buffer[0] = STK831X_MODE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto e_err_i2c;
	}

	if(stk->first_enable && k_status != STK_K_RUNNING)
	{
		stk->first_enable = false;
		msleep(2);
		result = stk_get_ic_content(stk);
	}
	if(en)
	{
#if STK_ACC_POLLING_MODE
//		hrtimer_start(&stk->acc_timer, stk->acc_poll_delay, HRTIMER_MODE_REL);
		unsigned long delay = msecs_to_jiffies(atomic_read(&stk->delay1));
		STK831X_SetVD(stk);
		schedule_delayed_work(&stk->work, delay);
#else
		STK831X_SetVD(stk);
		enable_irq((unsigned int)stk->irq);
#endif	//#if STK_ACC_POLLING_MODE
	}
	else
	{
#if STK_ACC_POLLING_MODE
		cancel_delayed_work_sync(&stk->work);
#else
		disable_irq((unsigned int)stk->irq);
#endif	//#if STK_ACC_POLLING_MODE
	}
	//mutex_unlock(&stk->write_lock);
	atomic_set(&stk->enabled, new_enabled);
	return 0;

e_err_i2c:
	//mutex_unlock(&stk->write_lock);
	return result;
}

static int STK831X_Init(struct stk831x_data *stk, struct i2c_client *client)
{
	int result;
	char buffer[2] = "";


#ifdef CONFIG_SENSORS_STK8312
	stk831x_info(KERN_INFO "%s: Initialize stk831x\n", __func__);
#elif defined CONFIG_SENSORS_STK8313
	stk831x_info(KERN_INFO "%s: Initialize stk8313\n", __func__);
#endif

	buffer[0] = STK831X_RESET;
	buffer[1] = 0x00;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}

	/* int pin is active high, psuh-pull */
	buffer[0] = STK831X_MODE;
	buffer[1] = 0xC0;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}

	/* 100 Hz ODR */
	stk->delay = STK831X_INIT_ODR;
	buffer[0] = STK831X_SR;
	buffer[1] = stk->delay + STK831X_SAMPLE_TIME_BASE;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		stk831x_info(KERN_ERR "%s:failed\n", __func__);
		return result;
	}

#if (!STK_ACC_POLLING_MODE)
	/* enable GINT, int after every measurement */
	buffer[0] = STK831X_INTSU;
	buffer[1] = 0x10;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:interrupt init failed\n", __func__);
		return result;
	}
#endif
	/* +- 6g mode */
	buffer[0] = STK831X_STH;
#ifdef CONFIG_SENSORS_STK8312
	buffer[1] = 0x42;
#elif defined CONFIG_SENSORS_STK8313
	buffer[1] = 0x82;
#endif
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0)
	{
		printk(KERN_ERR "%s:set range failed\n", __func__);
		return result;
	}

	atomic_set(&stk->enabled, 0);
	event_since_en = 0;

#ifdef STK_LOWPASS
	memset(&stk->fir, 0x00, sizeof(stk->fir));
	atomic_set(&stk->firlength, STK_FIR_LEN);
	atomic_set(&stk->fir_en, 1);
#endif


#ifdef STK_TUNE
	stk_tune_offset[0] = 0;
	stk_tune_offset[1] = 0;
	stk_tune_offset[2] = 0;
	stk_tune_done = 0;
#endif
	return 0;
}

static int stk831x_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct stk831x_data *stk = i2c_get_clientdata(client);
#ifdef SENSOR_RESUME_WORK
	wake_lock(&stk->resume_lock);
	schedule_work(&stk->resume_work);
#else
	#ifdef STK_RESUME_RE_INIT
	int error;
	#endif

	printk(KERN_INFO "%s\n", __func__);
	#ifdef STK_RESUME_RE_INIT
	error = STK831X_Init(stk, this_client);
	if (error)
	{
		printk(KERN_ERR "%s:stk831x initialization failed\n", __func__);
		return error;
	}
	stk->first_enable = true;
	#endif
	if(stk->re_enable)
	{
		stk->re_enable = false;
		STK831X_SetEnable(stk, 1);

	}
#endif

	return 0;
}

static int stk831x_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct stk831x_data *stk = i2c_get_clientdata(client);

	printk(KERN_INFO "%s\n", __func__);
	if(atomic_read(&stk->enabled))
	{
		STK831X_SetEnable(stk, 0);
		stk->re_enable = true;
	}
	return 0;
}

#endif /* CONFIG_PM */

static SIMPLE_DEV_PM_OPS(stk831x_pm_ops, stk831x_suspend, stk831x_resume);

static const unsigned short  bma220_addresses[] = {
    SENSOR_I2C_ADDR,
    I2C_CLIENT_END,
};

static const struct i2c_device_id stk831x_id[] = {
    { SENSOR_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, stk831x_id);

static struct i2c_driver stk831x_driver = {
	.driver = {
			   .owner   = THIS_MODULE,
			   .name    = STK831X_I2C_NAME,
			   .pm     = &stk831x_pm_ops,
		},
	.class = I2C_CLASS_HWMON,
	.probe = stk831x_probe,
	.remove = stk831x_remove,
	.id_table	= stk831x_id,

};


#if CFG_GSENSOR_USE_CONFIG > 0
static struct i2c_board_info stk831x_board_info={
    .type = SENSOR_NAME,
    .addr = SENSOR_I2C_ADDR,
};

static struct i2c_client *stk831x_client;
#endif

static int __init stk831x_init(void)
{
#if CFG_GSENSOR_USE_CONFIG > 0
    struct i2c_adapter *i2c_adap;
    unsigned int cfg_i2c_adap_id;

    /*get xml cfg*/
    int ret = get_config(CFG_GSENSOR_ADAP_ID, (char *)(&cfg_i2c_adap_id), sizeof(unsigned int));
    if (ret != 0) {
        printk(KERN_ERR "get i2c_adap_id %d fail\n", cfg_i2c_adap_id);
        return ret;
    }
    cfg_i2c_adap_id = 2;

    i2c_adap = i2c_get_adapter(cfg_i2c_adap_id);

    stk831x_client = i2c_new_device(i2c_adap, &stk831x_board_info);
    i2c_put_adapter(i2c_adap);
#endif
    return i2c_add_driver(&stk831x_driver);
}

static void __exit stk831x_exit(void)
{
#if CFG_GSENSOR_USE_CONFIG > 0
    i2c_unregister_device(stk831x_client);
#endif
    i2c_del_driver(&stk831x_driver);
}

module_init(stk831x_init);
module_exit(stk831x_exit);

MODULE_AUTHOR("Lex Hsieh / Sensortek");
MODULE_DESCRIPTION("stk831x 3-Axis accelerometer driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(STK_ACC_DRIVER_VERSION);


