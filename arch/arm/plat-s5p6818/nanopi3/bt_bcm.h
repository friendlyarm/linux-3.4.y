#ifndef __BT_BCM_H__
#define __BT_BCM_H__

#include <linux/rfkill.h>
#include <linux/wakelock.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/amba/serial.h>

/*
 * Bluetooth BCM struct
 */
enum bt_ctl_type {
	BT_TYPE_POWER		= 1,
	BT_TYPE_WAKE_DEVICE,
	BT_TYPE_WAKE_HOST,
};

struct bt_ctl_gpio {
	char *name;
	int  gpio;
	int  direction;	/* 0: input, 1: output */
	int  init_val;
	enum bt_ctl_type type;
};

struct bt_bcm_lpm {
	int wake;
	int host_wake;
	struct hrtimer lpm_timer;
	ktime_t lpm_delay;
	struct uart_port *uport;
	struct wake_lock wake_lock;
	struct wake_lock host_wake_lock;
};

struct plat_bt_bcm_data {
	struct bt_ctl_gpio *gpios;
	int gpio_nr;
};

struct bt_bcm_info {
	struct bt_ctl_gpio *gpios;
	int gpio_nr;
	struct rfkill *rfkill;
	struct bt_bcm_lpm lpm;
	struct mutex lock;
	bool running;
};

#endif /* __BT_BCM_H__ */
