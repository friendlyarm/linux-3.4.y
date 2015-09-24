/**************************************************************************
*  AW5306_ts.c
*
*  AW5306 ALLWIN sample code version 1.0
*
*  Create Date : 2012/06/07
*
*  Modify Date :
*
*  Create by   : wuhaijun
*
**************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/wakelock.h>

#include <asm/uaccess.h>

#include <mach/platform.h>

#include "aw5306_drv.h"
#include "aw5306_userpara.h"


#define TOUCH_RESUME_WORK
#define TOUCH_RESET_PIN        CFG_IO_TOUCH_RESET_PIN
//#define TOUCH_WAKE_PIN       CFG_IO_TOUCH_WAKE_PIN

//#define INTMODE	/* not support */
#define CONFIG_AW5306_MULTITOUCH
#define CONFIG_AW5306_SYSFS

/*
#define PRINT_POINT
#define PRINT_DEBUG
*/

#ifdef PRINT_POINT
#define pr_point(msg...)   do{ printk("aw5306:" msg); }while(0)
#else
#define pr_point(msg...)
#endif
#ifdef PRINT_DEBUG
#define pr_debug(msg...)   do{ printk("aw5306:" msg); }while(0)
#endif

///////////////////////////////////////////////
#define AW5306_NAME			"aw5306_ts"
static struct i2c_client   *this_client;
static int Resume_Init   = 0;
static int Resume_NoInit = 0;


#define PRESS_MAX				255

#define MAX_CONTACTS 			5       //lizhen
#define MAX_FINGER_NUM			5

struct ts_event {
	int	x[5];
	int	y[5];
	int	pressure;
	int touch_ID[5];
	int touch_point;
	int pre_point;
	int previous;
	int press[5];
};

struct AW5306_ts_data {
	struct input_dev	*input;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
	struct timer_list touch_timer;
	int open_count;
	int init_stat;

	int max_x;
	int max_y;
	char x_invert_flag;
	char y_invert_flag;
	char xy_exchange_flag;

	struct wake_lock resume_lock;       /* add by jhkim */
	struct work_struct	resume_work;
};

static unsigned char suspend_flag=0; //0: sleep out; 1: sleep in
static short tp_idlecnt = 0;
static char  tp_SlowMode = 1;

extern AW5306_UCF 	AWTPCfg;
extern STRUCTCALI 	AW_Cali;
extern STRUCTBASE	AW_Base;
extern short		adbDiff[NUM_TX][NUM_RX];
extern short		AWDeltaData[32];

extern 			AW5306_UCF Default_UCF;
extern unsigned char 	cpfreq;

int AW_nvram_read(char *filename, char *buf, ssize_t len, int offset)
{
    struct file *fd;
    int retLen = -1;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    fd = filp_open(filename, O_RDONLY, 0);

    if (IS_ERR(fd)) {
        printk("[AW5306][nvram_read] : failed to open %s!!\n", filename);
        return -1;
    }

    do {
        if ((fd->f_op == NULL) || (fd->f_op->read == NULL)) {
            printk("[AW5306][nvram_read] : file can not be read!!\n");
            break;
    	}

        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        		if (fd->f_op->llseek(fd, offset, 0) != offset) {
					printk("[AW5306][nvram_read] : failed to seek!!\n");
					break;
        		}
        	} else {
        	    fd->f_pos = offset;
        	}
        }
   		retLen = fd->f_op->read(fd, buf, len, &fd->f_pos);
    } while(false);

    filp_close(fd, NULL);
    set_fs(old_fs);

    return retLen;
}

int AW_nvram_write(char *filename, char *buf, ssize_t len, int offset)
{
    struct file *fd;
    int retLen = -1;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    fd = filp_open(filename, O_WRONLY|O_CREAT, 0666);

    if (IS_ERR(fd)) {
        printk("[AW5306][nvram_write] : failed to open %s!!\n", filename);
        return -1;
    }

    do {
        if ((fd->f_op == NULL) || (fd->f_op->write == NULL)) {
        	printk("[AW5306][nvram_write] : file can not be write!!\n");
            break;
    	} /* End of if */

        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        	    if (fd->f_op->llseek(fd, offset, 0) != offset) {
				    printk("[AW5306][nvram_write] : failed to seek!!\n");
                    break;
                }
            } else {
                fd->f_pos = offset;
            }
        }
        retLen = fd->f_op->write(fd, buf, len, &fd->f_pos);
    } while(false);

    filp_close(fd, NULL);
    set_fs(old_fs);

    return retLen;
}

int AW_I2C_WriteByte(u8 addr, u8 para)
{
	int ret;
	u8 buf[3] = { addr, para, 0 };
	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("msg %s i2c write error: %d\n", __func__, ret);

	return ret;
}

unsigned char AW_I2C_ReadByte(u8 addr)
{
	int ret;
	u8 buf[2] = { addr, 0};
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	return buf[0];

}

unsigned char AW_I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
	int ret,i;
	u8 rdbuf[512] = { addr, };
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rdbuf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= len,
			.buf	= rdbuf,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	for (i = 0; i < len; i++)
		buf[i] = rdbuf[i];

    return ret;
}

unsigned char AW_I2C_WriteXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
	struct i2c_msg msg;
	u8 wdbuf[512] = { addr, };
	int ret, i;

	for (i = 0; i < len; i++)
		wdbuf[i+1] = buf[i];

	ret = i2c_transfer(this_client->adapter, &msg, 1);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

    return ret;
}

void AW_Sleep(unsigned int msec)
{
	msleep(msec);
}
static void AW5306_ts_release(void)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
#ifdef CONFIG_AW5306_MULTITOUCH
	input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);
	input_mt_sync(ts->input);
#else
	input_report_abs(ts->input, ABS_PRESSURE, 0);
	input_report_key(ts->input, BTN_TOUCH, 0);
#endif
	input_sync(ts->input);
	return;

}

static int AW5306_read_data(void)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_event *event = &ts->event;
	int Pevent;
    	int i = 0;

	AW5306_TouchProcess();
	event->touch_point = AW5306_GetPointNum();

	for (i=0; MAX_FINGER_NUM > i;i++)
		event->press[i] = 0;

	for (i = 0; event->touch_point > i;i++) {
		if(ts->xy_exchange_flag)
			AW5306_GetPoint(&event->y[i], &event->x[i], &event->touch_ID[i], &Pevent, i);
		else
               		AW5306_GetPoint(&event->x[i], &event->y[i], &event->touch_ID[i], &Pevent, i);

		if(ts->x_invert_flag)
			event->x[i] = ts->max_x - event->x[i];	/* Invert X */

		if(ts->y_invert_flag)
               		event->y[i] = ts->max_y - event->y[i];  /* Invert Y */

		event->press[i] = 1;
		pr_point("[P.%d] x=%4d, y=%4d\n", i, event->x[i], event->y[i]);
	}

	if (event->touch_point == 0) {
		if (tp_idlecnt <= AWTPCfg.FAST_FRAME*5)
			tp_idlecnt++;

		if (tp_idlecnt > AWTPCfg.FAST_FRAME*5)
			tp_SlowMode = 1;

		if (event->previous != 0) {
		    AW5306_ts_release();
			event->previous = 0;
		}
		return 1;
	} else {
		tp_SlowMode = 0;
		tp_idlecnt = 0;
		event->previous = event->touch_point;
		event->pressure = 200;
		dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
		event->x[0], event->y[0], event->x[1], event->y[1]);
		return 0;
	}
}

#ifdef CONFIG_AW5306_MULTITOUCH
static void AW5306_report_multitouch(void)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_event *event = &ts->event;
	int release = event->pre_point - event->touch_point;
	int i = 0;

	for (i = 0; event->touch_point > i; i++) {
		input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 1);
		input_report_abs(ts->input, ABS_MT_PRESSURE, (i+1));
		input_report_abs(ts->input, ABS_MT_POSITION_X, event->x[i]);
		input_report_abs(ts->input, ABS_MT_POSITION_Y, event->y[i]);
		input_mt_sync(ts->input);
	}

	for (i = 0; release > i; i++) {
		input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);
		input_mt_sync(ts->input);
	}
	input_sync(ts->input);

	event->pre_point = event->touch_point;
	event->previous  = event->touch_point;
	return;
}
#else
static void AW5306_report_singletouch(void)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
	struct ts_event *event = &ts->event;

	if (event->touch_point == 1) {
		input_report_abs(ts->input, ABS_X, event->x[0]);
		input_report_abs(ts->input, ABS_Y, event->y[0]);
		input_report_abs(ts->input, ABS_PRESSURE, event->pressure);
	}
	input_report_key(ts->input, BTN_TOUCH, 1);
	input_sync(ts->input);

	pr_point("%s: x=%4d, y=%4d \n", __func__, event->x[0], event->y[0]);
	return;
}
#endif

static void AW5306_report_value(void)
{
#ifdef CONFIG_AW5306_MULTITOUCH
	AW5306_report_multitouch();
#else
	AW5306_report_singletouch();
#endif
	return;
}

static void AW5306_ts_pen_irq_work(struct work_struct *work)
{
	int ret = -1;
	if (Resume_NoInit == 0) {
		Resume_Init++;
		if (Resume_Init>100) {
		 	 AW5306_TP_Reinit();
			 Resume_NoInit = 1;
			 printk("Resume_ReInit2222--Resume_Init=%d,Resume_NoInit=%d\n",Resume_Init,Resume_NoInit);
		}
	}

	if (suspend_flag != 1) {
		ret = AW5306_read_data();
		if (ret == 0)
			AW5306_report_value();
	} else {
		AW5306_Sleep();
	}
}

#ifdef INTMODE
static irqreturn_t AW5306_ts_interrupt(int irq, void *dev_id)
{
	struct AW5306_ts_data *ts = dev_id;
	pr_point("%s\n");

	disable_irq_nosync(irq);
	if (!work_pending(&ts->pen_event_work))
		queue_work(ts->ts_workqueue, &ts->pen_event_work);

	return IRQ_HANDLED;
}
#endif
void AW5306_tpd_polling(unsigned long data)
{
	struct AW5306_ts_data *AW5306_ts = i2c_get_clientdata(this_client);

 	if (!work_pending(&AW5306_ts->pen_event_work))
    	queue_work(AW5306_ts->ts_workqueue, &AW5306_ts->pen_event_work);

	if (suspend_flag != 1) {
	#ifdef AUTO_RUDUCEFRAME
		if (tp_SlowMode) {
			AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.SLOW_FRAME;
		} else {
			AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		}
	#else
		AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
	#endif
		add_timer(&AW5306_ts->touch_timer);
	}
}

#ifdef CONFIG_AW5306_SYSFS
static ssize_t AW5306_get_Cali(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_set_Cali(struct device* cd, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t AW5306_get_reg(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_write_reg(struct device* cd, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t AW5306_get_Base(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_Diff(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_adbBase(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_adbDiff(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_FreqScan(struct device* cd, struct device_attribute *attr, char* buf);
static ssize_t AW5306_Set_FreqScan(struct device* cd, struct device_attribute *attr, const char* buf, size_t len);

static DEVICE_ATTR(cali, S_IRUGO | S_IWUSR, AW5306_get_Cali, AW5306_set_Cali);
static DEVICE_ATTR(readreg, S_IRUGO | S_IWUSR, AW5306_get_reg, AW5306_write_reg);
static DEVICE_ATTR(base, S_IRUGO | S_IWUSR, AW5306_get_Base, NULL);
static DEVICE_ATTR(diff, S_IRUGO | S_IWUSR, AW5306_get_Diff, NULL);
static DEVICE_ATTR(adbbase,  S_IRUGO | S_IWUSR, AW5306_get_adbBase, NULL);
static DEVICE_ATTR(adbdiff, S_IRUGO | S_IWUSR, AW5306_get_adbDiff, NULL);
static DEVICE_ATTR(freqscan, S_IRUGO | S_IWUSR, AW5306_get_FreqScan, AW5306_Set_FreqScan);

static ssize_t AW5306_get_Cali(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len,"*****AW5306 Calibrate data*****\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"TXOFFSET:");
	for (i=0;i<11;i++)
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXOFFSET[i]);

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXOFFSET:");
	for (i=0;i<6;i++)
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXOFFSET[i]);

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXCAC:");
	for (i=0;i<21;i++)
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXCAC[i]);

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXCAC:");
	for (i=0;i<12;i++)
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXCAC[i]);

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXGAIN:");
	for (i=0;i<21;i++)
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXGAIN[i]);

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	for (i=0;i<AWTPCfg.TX_LOCAL;i++) {
		for (j=0;j<AWTPCfg.RX_LOCAL;j++)
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d ", AW_Cali.SOFTOFFSET[i][j]);
		len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	}
	return len;
}

static ssize_t AW5306_set_Cali(struct device* cd,struct device_attribute *attr, const char *buf, size_t count)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);

	unsigned long on_off = simple_strtoul(buf, NULL, 10);
	if (on_off == 1) {
		suspend_flag = 1;
		AW_Sleep(50);

		TP_Force_Calibration();

		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&ts->touch_timer);
	}

	return count;
}


static ssize_t AW5306_get_adbBase(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "base: \n");
	for (i=0;i< AWTPCfg.TX_LOCAL;i++) {
		for (j=0;j<AWTPCfg.RX_LOCAL;j++)
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]);
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	return len;
}

static ssize_t AW5306_get_Base(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	*(buf+len) = AWTPCfg.TX_LOCAL;
	len++;
	*(buf+len) = AWTPCfg.RX_LOCAL;
	len++;

	for (i=0;i< AWTPCfg.TX_LOCAL;i++) {
		for (j=0;j<AWTPCfg.RX_LOCAL;j++) {
			*(buf+len) = (char)(((AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]) & 0xFF00)>>8);
			len++;
			*(buf+len) = (char)((AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]) & 0x00FF);
			len++;
		}
	}
	return len;

}

static ssize_t AW5306_get_adbDiff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "Diff: \n");
	for (i=0;i< AWTPCfg.TX_LOCAL;i++) {
		for (j=0;j<AWTPCfg.RX_LOCAL;j++)
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",adbDiff[i][j]);
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}

	return len;
}

static ssize_t AW5306_get_Diff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	*(buf+len) = AWTPCfg.TX_LOCAL;
	len++;
	*(buf+len) = AWTPCfg.RX_LOCAL;
	len++;

	for (i=0;i< AWTPCfg.TX_LOCAL;i++) {
		for (j=0;j<AWTPCfg.RX_LOCAL;j++) {
			*(buf+len) = (char)((adbDiff[i][j] & 0xFF00)>>8);
			len++;
			*(buf+len) = (char)(adbDiff[i][j] & 0x00FF);
			len++;
		}
	}
	return len;
}

static ssize_t AW5306_get_FreqScan(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i;
	ssize_t len = 0;

	for (i=0;i< 32;i++) {
		//*(buf+len) = (char)((AWDeltaData[i] & 0xFF00)>>8);
		//len++;
		//*(buf+len) = (char)(AWDeltaData[i] & 0x00FF);
		//len++;
		len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",AWDeltaData[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	return len;
}

static ssize_t AW5306_Set_FreqScan(struct device* cd, struct device_attribute *attr,
		       const char* buf, size_t len)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
	unsigned long Basefreq = simple_strtoul(buf, NULL, 10);

	if (Basefreq < 10) {
		suspend_flag = 1;
		AW_Sleep(50);

		FreqScan(Basefreq);

		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&ts->touch_timer);
	}
	return len;
}

static ssize_t AW5306_get_reg(struct device* cd,struct device_attribute *attr, char* buf)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
	u8 reg_val[128];
	ssize_t len = 0;
	u8 i;

	suspend_flag = 1;

	AW_Sleep(50);

	AW_I2C_ReadXByte(reg_val,0,127);

	AW5306_TP_Reinit();
	tp_idlecnt = 0;
	tp_SlowMode = 0;
	suspend_flag = 0;
	ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
	add_timer(&ts->touch_timer);

	for (i=0;i<0x7F;i++) {
		reg_val[0] = AW_I2C_ReadByte(i);
		len += snprintf(buf+len, PAGE_SIZE-len, "reg%02X = 0x%02X, ", i,reg_val[0]);
	}

	return len;

}

static ssize_t AW5306_write_reg(struct device* cd,struct device_attribute *attr, const char *buf, size_t count)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
	int databuf[2];

	if (2 == sscanf(buf, "%d %d", &databuf[0], &databuf[1])) {
		suspend_flag = 1;
		AW_Sleep(50);

		AW_I2C_WriteByte((u8)databuf[0],(u8)databuf[1]);

		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&ts->touch_timer);
	} else {
		printk("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;
}

static int AW5306_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);
	pr_debug("%s", __func__);

	err = device_create_file(dev, &dev_attr_cali);
	err = device_create_file(dev, &dev_attr_readreg);
	err = device_create_file(dev, &dev_attr_base);
	err = device_create_file(dev, &dev_attr_diff);
	err = device_create_file(dev, &dev_attr_adbbase);
	err = device_create_file(dev, &dev_attr_adbdiff);
	err = device_create_file(dev, &dev_attr_freqscan);
	return err;
}
#endif

#ifdef TOUCH_RESUME_WORK
static void AW5306_ts_work_resume(struct work_struct *work)
{
	struct AW5306_ts_data *ts = container_of(work, struct AW5306_ts_data, resume_work);
	PM_DBGOUT("+%s (flag=%d, opend=%d)\n", __func__, suspend_flag, ts->open_count);

	gpio_direction_output(TOUCH_RESET_PIN, 1);
	msleep(1);
	gpio_direction_output(TOUCH_RESET_PIN, 0);
	msleep(30);

	if (suspend_flag != 0 && ts->open_count) {
	    msleep(50);
		AW5306_User_Cfg1();
		msleep(50);
		AW5306_TP_Reinit();

		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		Resume_NoInit = 0;
		Resume_Init = 0;
		ts->touch_timer.expires = jiffies + 10;
		add_timer(&ts->touch_timer);
	}
	wake_unlock(&ts->resume_lock);

	PM_DBGOUT("-%s\n", __func__);
}
#endif

static int AW5306_ts_open(struct input_dev *dev)
{
	struct AW5306_ts_data * ts = input_get_drvdata(dev);
	pr_debug("%s (flag:%d, init:%d, opend:%d)\n", __func__,
		suspend_flag, ts->init_stat, ts->open_count);

	/* first once call */
	if (!ts->init_stat) {
		AW5306_TP_Init();
		ts->touch_timer.expires = jiffies + HZ*5;
		del_timer(&ts->touch_timer);
		add_timer(&ts->touch_timer);
		suspend_flag = 0;
	}

	/* when open */
	if (suspend_flag != 0 && ts->init_stat) {
	    msleep(50);
		AW5306_User_Cfg1();
		msleep(50);
		AW5306_TP_Reinit();

		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		Resume_NoInit = 0;
		Resume_Init = 0;
		ts->touch_timer.expires = jiffies + 2;
		del_timer(&ts->touch_timer);
		add_timer(&ts->touch_timer);
	}

	ts->init_stat = 1;
	ts->open_count++;
    return 0;
}

static void AW5306_ts_close(struct input_dev *dev)
{
	struct AW5306_ts_data * ts = input_get_drvdata(dev);

	if (suspend_flag != 1)
		suspend_flag = 1;

	ts->open_count--;
	pr_debug("%s (opend:%d)\n", __func__, ts->open_count);
}

static void AW5306_ts_power(void)
{
	int err = 0;
	pr_debug("%s, reset pin=%d\n", __func__, TOUCH_RESET_PIN);
	err = gpio_request(TOUCH_RESET_PIN, AW5306_NAME);
	if (!err) {
		pr_debug("reset pin.%d\n", TOUCH_RESET_PIN);
	  	gpio_direction_output(TOUCH_RESET_PIN, 1);
		msleep(1);
		gpio_direction_output(TOUCH_RESET_PIN, 0);
		msleep(30);
	}
}

#ifdef CONFIG_PM
static int AW5306_ts_suspend(struct i2c_client *client, pm_message_t state)
{
	PM_DBGOUT("%s (flag=%d)\n", __func__, suspend_flag);
	if (suspend_flag != 1)
		suspend_flag = 1;

	gpio_direction_output(TOUCH_RESET_PIN, 0);
	return 0;
}

static int AW5306_ts_resume(struct i2c_client *client)
{
	struct AW5306_ts_data *ts = i2c_get_clientdata(this_client);
#ifdef TOUCH_RESUME_WORK
	wake_lock(&ts->resume_lock);
	schedule_work(&ts->resume_work);
#else
	gpio_direction_output(TOUCH_RESET_PIN, 1);
	msleep(1);
	gpio_direction_output(TOUCH_RESET_PIN, 0);
	msleep(30);

	if (suspend_flag != 0 && ts->open_count) {
	    msleep(50);
		AW5306_User_Cfg1();
		msleep(50);
		AW5306_TP_Reinit();

		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		Resume_NoInit = 0;
		Resume_Init = 0;
		ts->touch_timer.expires = jiffies + 10;
		add_timer(&ts->touch_timer);
	}
#endif
	return 0;
}
#else
#define AW5306_ts_suspend       NULL
#define AW5306_ts_resume        NULL
#endif

static int
AW5306_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct aw5306_plat_data *plat = client->dev.platform_data;
	struct AW5306_ts_data *ts;
	struct input_dev *input;
	int err = 0;
    u8 val;

	pr_debug("+%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	if (plat) {
		memcpy(&Default_UCF, &plat->default_UCF, sizeof(AW5306_UCF));
		cpfreq = plat->cpfreq;
		ts->max_x = plat->max_x;
		ts->max_y = plat->max_y;
		ts->x_invert_flag = plat->x_invert_flag;
		ts->y_invert_flag = plat->y_invert_flag;
		ts->xy_exchange_flag = plat->xy_exchange_flag;
	} else {
		ts->max_x = 800;
		ts->max_y = 1280;
		ts->x_invert_flag = 0;
		ts->y_invert_flag = 1;
		ts->xy_exchange_flag = 0;
	}

	AW5306_ts_power();

	this_client = client;
	i2c_set_clientdata(client, ts);

	val = AW_I2C_ReadByte(0x01);
	pr_debug("%s : chips=0x%x, id=0x%x\n", __func__, client->addr, val);
	if (val != 0xa8) {
		client->addr = 0x39;
		pr_err("AW5306_ts_probe: chip id not corrected\n");
		goto exit_create_singlethread;
	}

	INIT_WORK(&ts->pen_event_work, AW5306_ts_pen_irq_work);
	ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}

	input = input_allocate_device();
	if (!input) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
	ts->input = input;

#ifdef CONFIG_AW5306_MULTITOUCH
    set_bit(EV_ABS, input->evbit);

    input_set_abs_params(input, ABS_MT_POSITION_X , 0, ts->max_x-1, 0, 0);
    input_set_abs_params(input, ABS_MT_POSITION_Y , 0, ts->max_y-1, 0, 0);
    input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input, ABS_MT_PRESSURE, 0, 255, 0, 0);
#else
	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	set_bit(ABS_PRESSURE, input->absbit);
	set_bit(BTN_TOUCH, input->keybit);

	input_set_abs_params(input, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, 0, PRESS_MAX, 0 , 0);

	set_bit(EV_ABS, input->evbit);
	set_bit(EV_KEY, input->evbit);
#endif

    input->open = AW5306_ts_open;
    input->close = AW5306_ts_close;
	input->name = AW5306_NAME;		//dev_name(&client->dev)
	err = input_register_device(input);
	if (err) {
		dev_err(&client->dev,
		"ft5x0x_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

#ifdef INTMODE
	err = request_irq(client->irq, AW5306_ts_interrupt,
				IRQF_TRIGGER_RISING | IRQF_SHARED, AW5306_NAME, ts);
	if (err < 0) {
		dev_err(&client->dev, "AW5306_ts_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
#else
	#ifdef CONFIG_AW5306_SYSFS
 	AW5306_create_sysfs(client);
	#endif

	#if (0)
  	AW5306_TP_Init();
	ts->touch_timer.function = AW5306_tpd_polling;
	ts->touch_timer.data = 0;
	init_timer(&ts->touch_timer);
	ts->touch_timer.expires = jiffies + HZ*5;
	add_timer(&ts->touch_timer);
	#else
	ts->touch_timer.function = AW5306_tpd_polling;
	ts->touch_timer.data = 0;
	init_timer(&ts->touch_timer);
	#endif

	input_set_drvdata(input, ts);
#endif

#ifdef TOUCH_RESUME_WORK
	INIT_WORK(&ts->resume_work, AW5306_ts_work_resume);
 	wake_lock_init(&ts->resume_lock, WAKE_LOCK_SUSPEND, "aw5306_ts");
#else
	device_enable_async_suspend(&client->dev);
#endif
	pr_debug("-%s\n", __func__);
	return 0;

#ifdef INTMODE
exit_irq_request_failed:
	cancel_work_sync(&ts->pen_event_work);
	destroy_workqueue(ts->ts_workqueue);
	enable_irq(client->irq);
#endif
exit_input_register_device_failed:
	input_free_device(input);
exit_input_dev_alloc_failed:
	free_irq(client->irq, ts);
exit_create_singlethread:
	i2c_set_clientdata(client, NULL);
	kfree(ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int __devexit AW5306_ts_remove(struct i2c_client *client)
{

	struct AW5306_ts_data *ts = i2c_get_clientdata(client);

	gpio_free(TOUCH_RESET_PIN);
//	gpio_free(TOUCH_WAKE_PIN);

	if (!ts)
		return 0;

#ifdef INTMODE
	free_irq(client->irq, ts);
#else
	del_timer(&ts->touch_timer);
#endif

	if (ts->input) {
		input_unregister_device(ts->input);
		input_free_device(ts->input);
	}
	if (ts->ts_workqueue) {
		cancel_work_sync(&ts->pen_event_work);
		destroy_workqueue(ts->ts_workqueue);
	}
	i2c_set_clientdata(client, NULL);

	kfree(ts);
	return 0;

}

static const struct i2c_device_id AW5306_ts_id[] = {
	{ AW5306_NAME, 0 }, { }
};
MODULE_DEVICE_TABLE(i2c, AW5306_ts_id);

static struct i2c_driver AW5306_ts_driver = {
	.probe		= AW5306_ts_probe,
	.remove		= __devexit_p(AW5306_ts_remove),
	.id_table	= AW5306_ts_id,
	.suspend 	= AW5306_ts_suspend,
	.resume  	= AW5306_ts_resume,
	.driver		= {
		.name	= AW5306_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init AW5306_ts_init(void)
{
	return i2c_add_driver(&AW5306_ts_driver);
}

static void __exit AW5306_ts_exit(void)
{
	i2c_del_driver(&AW5306_ts_driver);
}

module_init(AW5306_ts_init);
module_exit(AW5306_ts_exit);

MODULE_AUTHOR("<whj@AWINIC.com>");
MODULE_DESCRIPTION("AWINIC AW5306 TouchScreen driver");
MODULE_LICENSE("GPL");

