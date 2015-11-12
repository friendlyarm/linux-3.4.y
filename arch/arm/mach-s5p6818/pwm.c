#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/pwm.h>
#include <linux/slab.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

/* refer pwm prototype */
#define	PWM_MODULES		4

struct pwm_device {
	struct list_head list;
	struct device *dev;
	const char *label;
	unsigned int period_ns;
	unsigned int duty_ns;
	unsigned char use_count;
	unsigned char pwm_id;
};

static DEFINE_MUTEX(pwm_lock); /* lock for pwm_list */
static LIST_HEAD(pwm_list);

struct pwm_device *pwm_request(int pwm_id, const char *label)
{
	struct pwm_device *pwm = NULL;
	int found = 0;

	mutex_lock(&pwm_lock);

	list_for_each_entry(pwm, &pwm_list, list) {
		if (pwm->pwm_id == pwm_id) {
			found = 1;
			break;
		}
	}

	if (found) {
		pwm->use_count++;
	} else {
		pwm = ERR_PTR(-ENOENT);
	}

	mutex_unlock(&pwm_lock);
	return pwm;
}
EXPORT_SYMBOL(pwm_request);

void pwm_free(struct pwm_device *pwm)
{
	mutex_lock(&pwm_lock);

	if (pwm->use_count) {
		pwm->use_count--;
	} else {
		printk(KERN_ERR "PWM%d device already freed\n", pwm->pwm_id);
	}

	mutex_unlock(&pwm_lock);
}
EXPORT_SYMBOL(pwm_free);

int pwm_enable(struct pwm_device *pwm)
{
	return 0;
}
EXPORT_SYMBOL(pwm_enable);

void pwm_disable(struct pwm_device *pwm)
{
}
EXPORT_SYMBOL(pwm_disable);

#define NS_IN_HZ (1000000000UL)
#define TO_PERCENT(duty, period)		(((duty)*100)/(period))
#define TO_HZ(period)					(NS_IN_HZ/(period))

int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns)
{
	u32 duty_percent = 0;
	u32 period_hz = 0;

	if (period_ns > NS_IN_HZ || duty_ns > NS_IN_HZ)
		return -ERANGE;

	if (duty_ns > period_ns) {
		printk("%s error: duty is too big!!!(%d/%d)\n", __func__, duty_ns, period_ns);
		return -ERANGE;
	}

	if (period_ns == pwm->period_ns && duty_ns == pwm->duty_ns) {
		return 0;
	}

	pwm->period_ns = period_ns;
	pwm->duty_ns = duty_ns;

	period_hz = TO_HZ(pwm->period_ns);
	duty_percent = TO_PERCENT(pwm->duty_ns, pwm->period_ns);

	pr_debug("pwm ch[%d]: set duty(%u percent), frequency(%u HZ)\n",
		pwm->pwm_id, duty_percent, period_hz);
	nxp_soc_pwm_set_frequency(pwm->pwm_id, period_hz, duty_percent);

	return 0;
}
EXPORT_SYMBOL(pwm_config);

static int pwm_register(struct pwm_device *pwm)
{
	pwm->duty_ns = -1;
	pwm->period_ns = -1;

	mutex_lock(&pwm_lock);
	list_add_tail(&pwm->list, &pwm_list);
	mutex_unlock(&pwm_lock);

	return 0;
}

static int pwm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pwm_device *pwm;
	unsigned int id = pdev->id;
	int ret;

	if (id > (PWM_MODULES - 1)) {
		dev_err(dev, "id %d is out of range\n", id);
		return -EINVAL;
	}

	pwm = kzalloc(sizeof(struct pwm_device), GFP_KERNEL);
	if (!pwm) {
		dev_err(dev, "failed to allocate pwm_device\n");
		return -ENOMEM;
	}

	pwm->dev = dev;
	pwm->pwm_id = id;

	ret = pwm_register(pwm);
	if (ret) {
		dev_err(dev, "failed to register pwm\n");
		goto err_alloc;
	}
	platform_set_drvdata(pdev, pwm);

	return 0;

err_alloc:
	kfree(pwm);
	return ret;
}

static int __devexit pwm_remove(struct platform_device *pdev)
{
	struct pwm_device *pwm = platform_get_drvdata(pdev);
	kfree(pwm);

	return 0;
}

#ifdef CONFIG_PM
static int pwm_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int pwm_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define pwm_suspend 		NULL
#define pwm_resume			NULL
#endif

static struct platform_driver pwm_driver = {
	.driver = {
		.name 	= DEV_NAME_PWM,
		.owner 	= THIS_MODULE,
	},
	.probe		= pwm_probe,
	.remove		= __devexit_p(pwm_remove),
	.suspend	= pwm_suspend,
	.resume		= pwm_resume,
};

static int __init pwm_init(void)
{
	return platform_driver_register(&pwm_driver);
}
arch_initcall(pwm_init);
