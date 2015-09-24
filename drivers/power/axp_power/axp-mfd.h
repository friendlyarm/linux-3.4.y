#ifndef __LINUX_AXP_MFD_H_
#define __LINUX_AXP_MFD_H_

#include "axp-mfd-22.h"

#define AXP_MFD_ATTR(_name)					\
{									\
	.attr = { .name = #_name,.mode = 0644 },					\
	.show =  _name##_show,				\
	.store = _name##_store, \
}

/* AXP battery charger data */
struct power_supply_info;

struct axp_supply_init_data {
	/* battery parameters */
	struct power_supply_info *battery_info;

	/* current and voltage to use for battery charging */
	unsigned int chgcur;
	unsigned int chgvol;
	unsigned int chgend;
	/*charger control*/
	bool chgen;
	bool limit_on;
	/*charger time */
	int chgpretime;
	int chgcsttime;

	/*adc sample time */
	unsigned int sample_time;

	/* platform callbacks for battery low and critical IRQs */
	void (*battery_low)(void);
	void (*battery_critical)(void);
};

struct axp_funcdev_info {
	int		id;
	const char	*name;
	void	*platform_data;
};

struct axp_platform_data {
	int num_regl_devs;
	int num_sply_devs;
	int num_gpio_devs;
	int gpio_base;
	struct axp_funcdev_info *regl_devs;
	struct axp_funcdev_info *sply_devs;
	struct axp_funcdev_info *gpio_devs;

};

struct axp_mfd_chip {
	struct i2c_client	*client;
	struct device		*dev;
	struct axp_mfd_chip_ops	*ops;

	int			type;
	uint64_t		irqs_enabled;

	struct mutex		lock;
	struct work_struct	irq_work;

	struct blocking_notifier_head notifier_list;
};

struct axp_mfd_chip_ops {
	int	(*init_chip)(struct axp_mfd_chip *);
	int	(*enable_irqs)(struct axp_mfd_chip *, uint64_t irqs);
	int	(*disable_irqs)(struct axp_mfd_chip *, uint64_t irqs);
	int	(*read_irqs)(struct axp_mfd_chip *, uint64_t *irqs);
};

extern struct device *axp_get_dev(void);
extern int axp_register_notifier(struct device *dev,
		struct notifier_block *nb, uint64_t irqs);
extern int axp_unregister_notifier(struct device *dev,
		struct notifier_block *nb, uint64_t irqs);


/* NOTE: the functions below are not intended for use outside
 * of the AXP sub-device drivers
 */
extern int axp_write(struct device *dev, int reg, uint8_t val);
extern int axp_writes(struct device *dev, int reg, int len, uint8_t *val);
extern int axp_read(struct device *dev, int reg, uint8_t *val);
extern int axp_reads(struct device *dev, int reg, int len, uint8_t *val);
extern int axp_update(struct device *dev, int reg, uint8_t val, uint8_t mask);
extern int axp_set_bits(struct device *dev, int reg, uint8_t bit_mask);
extern int axp_clr_bits(struct device *dev, int reg, uint8_t bit_mask);
extern struct i2c_client *axp;
#endif /* __LINUX_PMIC_AXP_H */
