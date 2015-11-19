#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

#define DEVICE_NAME		"sensor"

struct sensor {
    int gpio;
    int int_type;
};


struct sensor_desc {
    int ID;
	int gpio;	
    int int_type;
	char *name;	
	int is_used;
	struct timer_list timer;
};

static size_t cur_sensor_num = -1;
static int is_started = 0;

static struct sensor_desc sensor_dev[8] = {
	{ 0, -1, -1, "sensor1", 0},
	{ 1, -1, -1, "sensor2", 0 },
	{ 2, -1, -1, "sensor3", 0 },
	{ 3, -1, -1, "sensor4", 0 },
	{ 4, -1, -1, "sensor5", 0 },
	{ 5, -1, -1, "sensor6", 0 },
	{ 6, -1, -1, "sensor7", 0 },
	{ 7, -1, -1, "sensor8", 0 },
};

static volatile char sensor_dev_value[8] = {
};

static DECLARE_WAIT_QUEUE_HEAD(sensor_dev_waitq);

static volatile int ev_press = 0;

// static int timer_counter = 0;
static void gpio_int_sensors_timer(unsigned long _data)
{

	struct sensor_desc *bdata = (struct sensor_desc *)_data;
	int ID;
	unsigned tmp = gpio_get_value(bdata->gpio);
	ID = bdata->ID;

	// printk("%s %d value=%d int_type=%d\n", __FUNCTION__, timer_counter++, tmp, bdata->int_type);
	if (bdata->int_type == IRQ_TYPE_EDGE_RISING) {
		if(tmp == 1) {
			sensor_dev_value[ID] = 1;
			ev_press = 1;
			wake_up_interruptible(&sensor_dev_waitq);
		}
	} else if (bdata->int_type == IRQ_TYPE_EDGE_FALLING) {	
		if(tmp == 0) {
			sensor_dev_value[ID] = 1;
			ev_press = 1;
			wake_up_interruptible(&sensor_dev_waitq);
		}
	} else if (bdata->int_type == IRQ_TYPE_EDGE_BOTH) {
		sensor_dev_value[ID] = tmp;
		ev_press = 1;
		wake_up_interruptible(&sensor_dev_waitq);
	}
}

// static int int_counter=0;
static irqreturn_t button_interrupt(int irq, void *dev_id)
{
	// printk("%s %d\n", __FUNCTION__, int_counter++);
	struct sensor_desc *bdata = (struct sensor_desc *)dev_id;
	mod_timer(&bdata->timer, jiffies + msecs_to_jiffies(100));
	return IRQ_HANDLED;
}

static int start_sensor(void)
{
	int irq;
	int i;
	int err = 0;

	for (i = 0; i < cur_sensor_num; i++) {
		if (!sensor_dev[i].gpio)
			continue;

		setup_timer(&sensor_dev[i].timer, gpio_int_sensors_timer,
				(unsigned long)&sensor_dev[i]);

		err = gpio_request(sensor_dev[i].gpio, "button");			
		if (err) {
			printk("%s gpio_request %d fail\n", __FUNCTION__, sensor_dev[i].gpio);
            break;
		}

		irq = gpio_to_irq(sensor_dev[i].gpio);
		err = request_irq(irq, button_interrupt, IRQ_TYPE_EDGE_FALLING, 
				sensor_dev[i].name, (void *)&sensor_dev[i]);
		if (err) {
		    printk("%s request_irq %d fail\n", __FUNCTION__, irq);
            break;
		}
	}

	if (err) {
		i--;
		for (; i >= 0; i--) {
			if (!sensor_dev[i].gpio)
				continue;

			gpio_free(sensor_dev[i].gpio);
			irq = gpio_to_irq(sensor_dev[i].gpio);
			disable_irq(irq);
			free_irq(irq, (void *)&sensor_dev[i]);

			del_timer_sync(&sensor_dev[i].timer);
		}

		return -EBUSY;
	}

	ev_press = 0;
	return 0;
}

static int gpio_int_sensors_open(struct inode *inode, struct file *file)
{
    if(cur_sensor_num != -1)
    	return -EBUSY;
    cur_sensor_num = 0;
    is_started = 0;
	return 0;
}

static int stop_sensor(void)
{
	int irq, i;

	for (i = 0; i < cur_sensor_num; i++) {
		if (!sensor_dev[i].gpio)
			continue;

		gpio_free(sensor_dev[i].gpio);
		irq = gpio_to_irq(sensor_dev[i].gpio);
		free_irq(irq, (void *)&sensor_dev[i]);

		del_timer_sync(&sensor_dev[i].timer);
	}
	return 0;
}

static int gpio_int_sensors_close(struct inode *inode, struct file *file)
{
	// printk("%s cur_sensor_num=%d is_started=%d\n",__FUNCTION__, cur_sensor_num, is_started);
	if (is_started && cur_sensor_num > 0) {
		stop_sensor();
	}
	cur_sensor_num = -1;
	is_started = 0;	
	ev_press = 0;
	return 0;
}

// static int read_counter = 0;
static int gpio_int_sensors_read(struct file *filp, char __user *buff,
		size_t count, loff_t *offp)
{
	unsigned long err;
	int i = 0;

	// printk("%s cur_sensor_num=%d read_counter=%d ev_press=%d\n",__FUNCTION__, cur_sensor_num, read_counter++, ev_press);

	if (!ev_press) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else
			wait_event_interruptible(sensor_dev_waitq, ev_press);
	}

	ev_press = 0;

	err = copy_to_user((void *)buff, (const void *)(&sensor_dev_value),
			min(cur_sensor_num, count));
	for (i = 0; i<cur_sensor_num; i++) {
		sensor_dev_value[i] = 0;
	}

	return err ? -EFAULT : min(cur_sensor_num, count);
}

static unsigned int gpio_int_sensors_poll( struct file *file,
		struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &sensor_dev_waitq, wait);
	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static long gpio_int_sensors_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
#define ADD_SENSOR                  (0)
#define DEL_SENSOR                  (1)
#define START_ALL_SENSOR            (4)
#define STOP_ALL_SENSOR             (8)

	struct sensor *sen = NULL;
	//printk("%s cmd=%d\n", __FUNCTION__, cmd);
	switch(cmd) {
		case ADD_SENSOR:
			if (is_started == 0) {
				if ((void*)arg != NULL) {
					sen = (struct sensor *)arg;				
					sensor_dev[cur_sensor_num].gpio = sen->gpio;
					sensor_dev[cur_sensor_num].int_type= sen->int_type;
					sensor_dev[cur_sensor_num].is_used = 1;
					cur_sensor_num++;
				}
				else {
					return -EINVAL;
				}	
			} else {
				return -EINVAL;
			}
			break;
		case DEL_SENSOR:
			if (is_started == 0) {
				sen = (struct sensor *)arg;
				sensor_dev[cur_sensor_num].gpio = -1;					
				sensor_dev[cur_sensor_num].int_type= -1;
				sensor_dev[cur_sensor_num].is_used = 0;
				cur_sensor_num--;
			} else {
				return -EINVAL;
			}
			break;
		case START_ALL_SENSOR:
			if (is_started == 0) {
				if(start_sensor() < 0)
					return -EINVAL;
			} else {
				return -EINVAL;
			}
			is_started = 1;
			break;
		case STOP_ALL_SENSOR:
			if (is_started == 1) {
				if(stop_sensor() < 0)
					return -EINVAL;
			} else {
				return -EINVAL;
			}
			is_started = 0;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}


static struct file_operations dev_fops = {
	.owner		= THIS_MODULE,
	.open		= gpio_int_sensors_open,
	.release	= gpio_int_sensors_close, 
	.read		= gpio_int_sensors_read,
	.poll		= gpio_int_sensors_poll,
	.unlocked_ioctl	= gpio_int_sensors_ioctl,
};

static struct miscdevice misc = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DEVICE_NAME,
	.fops		= &dev_fops,
};

static int __init sensor_dev_init(void)
{
	int ret;

	ret = misc_register(&misc);

	printk(DEVICE_NAME"\tinitialized\n");

	return ret;
}

static void __exit sensor_dev_exit(void)
{
	misc_deregister(&misc);
}

module_init(sensor_dev_init);
module_exit(sensor_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");

