#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <media/rc-core.h>

#include <mach/platform.h>
#include <mach/soc.h>

#include <mach/devices.h>
#include <linux/clk.h>

#include <mach/ppm.h>

#define NXP_IR_DEVICE_NAME	"nxp_ir_recv"
#define RAW_BUFFER_SIZE 100


#define PPM_NS(x)	(1000000000/CFG_PPM_CLK*x)

struct nxp_rc_dev {
	struct rc_dev *rcdev;
	struct work_struct 	ppm_event_work;
	struct workqueue_struct *ppm_workqueue;

	struct ppm_data{
		struct ir_raw_event ev[RAW_BUFFER_SIZE];
		u32 count;
	}data;
	NX_PPM_INPUTPOL input_polarity;
};

struct nxp_rc_dev *nxp_ppm_dev= NULL;

#ifdef CONFIG_PPM_SYSFS
static ssize_t ppm_duty(struct kobject *kobj, struct kobj_attribute *attr,char *buf);

static struct kobj_attribute ppm_attr =
	__ATTR(duty, 0644, ppm_duty, NULL);


static struct attribute * g[] = {
		    &ppm_attr.attr,
		    NULL,
};

static struct attribute_group attr_group = {
    .attrs = g,
};

static ssize_t ppm_duty(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i = 0;
	int dur0=0, dur1=0;
	int duty;
	char *s  = buf;
	ssize_t status;
	struct nxp_rc_dev *nxp_dev = nxp_ppm_dev;
	for(i=0; i<RAW_BUFFER_SIZE; i++)
	{
		if((nxp_dev->data.ev[i].pulse == 0) && (dur0 == 0)){
			dur0 = nxp_dev->data.ev[i].duration;
		}
		else if((nxp_dev->data.ev[i].pulse == 1) && (dur1 == 0)){
			dur1 = nxp_dev->data.ev[i].duration;
		}
		if((dur0 != 0) && (dur1 != 0)){
			for(i=0; i<RAW_BUFFER_SIZE; i++){
				nxp_dev->data.ev[i].duration = 0;
			}
			break;
		}
	}

	if((dur0 == 0) && (dur1 == 0)){
		duty = 0;	
		goto err;
	}
	else if(abs(dur0 - dur1) == 100){
		duty = 100;
		goto err;
	}

	if(NX_PPM_GetInputSignalPolarity(0) == NX_PPM_INPUTPOL_INVERT)
		duty =	( dur0 * 100) / (dur0 + dur1);
	else
		duty =	( dur1 * 100) / (dur0 + dur1);

err:
    status = sprintf(s, "%d\n",duty );
    return status;

}
#endif

static void ppm_irq_work(struct work_struct *work)
{
	int i=0;
	struct nxp_rc_dev *nxp_dev = container_of(work, struct nxp_rc_dev, ppm_event_work);
	DEFINE_IR_RAW_EVENT(ev);

	if(!nxp_dev->data.count)
		return;

	for(i=0; i<nxp_dev->data.count; i++)
	{
		//printk("%d: %d %d ns \n", i/2,nxp_dev->data.ev[i].pulse, nxp_dev->data.ev[i].duration);
		ir_raw_event_store(nxp_dev->rcdev, &nxp_dev->data.ev[i]);
		ir_raw_event_handle(nxp_dev->rcdev);
	}

	if(NX_PPM_IsHighOverflow(0))
		ev.pulse = true;//true;
	else
		ev.pulse = false;
	ev.duration = PPM_NS(0xffff);//change to ns
	ir_raw_event_store(nxp_dev->rcdev, &ev);
	ir_raw_event_handle(nxp_dev->rcdev);

	nxp_dev->data.count = 0;
}

static irqreturn_t nxp_ir_recv_irq(int irq, void *dev_id)
{
	struct nxp_rc_dev *nxp_dev = dev_id;

	switch(NX_PPM_GetInterruptPendingNumber(0)){
	case NX_PPM_INT_RISEEDGE://space
		NX_PPM_ClearInterruptPending(0, NX_PPM_INT_RISEEDGE);
		nxp_dev->data.ev[nxp_dev->data.count].pulse = false;
		nxp_dev->data.ev[nxp_dev->data.count].duration = PPM_NS(NX_PPM_GetPPMLowPeriodValue(0));
		nxp_dev->data.count++;
		break;

	case NX_PPM_INT_FALLEDGE://pluse
		NX_PPM_ClearInterruptPending(0, NX_PPM_INT_FALLEDGE);
		nxp_dev->data.ev[nxp_dev->data.count].pulse = true;
		nxp_dev->data.ev[nxp_dev->data.count].duration = PPM_NS(NX_PPM_GetPPMHighPeriodValue(0));
		nxp_dev->data.count++;

		break;

	case NX_PPM_INT_OVERFLOW:
		queue_work(nxp_dev->ppm_workqueue, &nxp_dev->ppm_event_work);
		NX_PPM_ClearInterruptPending(0, NX_PPM_INT_OVERFLOW);
		break;

	default:
		NX_PPM_ClearInterruptPendingAll(0);
		break;
	}

	if (nxp_dev->data.count > RAW_BUFFER_SIZE)
		nxp_dev->data.count=0;

	return IRQ_HANDLED;
}

static int __devinit nxp_ir_recv_probe(struct platform_device *pdev)
{
	struct nxp_rc_dev *nxp_dev;
	struct rc_dev *rcdev;
	const struct nxp_ppm_platform_data *pdata =
					pdev->dev.platform_data;
	struct clk *clk;
	int rc;
#ifdef CONFIG_PPM_SYSFS
	struct kobject * kobj;
#endif

	if (!pdata)
		return -EINVAL;

	nxp_soc_peri_reset_set(RESET_ID_PPM);
   	clk = clk_get(NULL, DEV_NAME_PPM);
	clk_set_rate(clk, CFG_PPM_CLK);
	clk_enable(clk);

    NX_PPM_Initialize();
    NX_PPM_SetBaseAddress(0, (void*)IO_ADDRESS(NX_PPM_GetPhysicalAddress(0)));

//	NX_PPM_OpenModule(0);
	nxp_dev = kzalloc(sizeof(struct nxp_rc_dev), GFP_KERNEL);
	if (!nxp_dev)
		return -ENOMEM;

	rcdev = rc_allocate_device();
	if (!rcdev) {
		rc = -ENOMEM;
		goto err_allocate_device;
	}

	rcdev->driver_type = RC_DRIVER_IR_RAW;
	rcdev->allowed_protos = RC_TYPE_ALL;
	rcdev->input_name = NXP_IR_DEVICE_NAME;
	rcdev->input_id.bustype = BUS_HOST;
	rcdev->driver_name = DEV_NAME_PPM;
	rcdev->map_name = RC_MAP_NEC_TERRATEC_CINERGY_XS;

	nxp_dev->rcdev = rcdev;
	nxp_dev->data.count = 0;
	nxp_dev->input_polarity = pdata->input_polarity;
	NX_PPM_SetInputSignalPolarity(0, nxp_dev->input_polarity);

	rc = rc_register_device(rcdev);
	if (rc < 0) {
		dev_err(&pdev->dev, "failed to register rc device\n");
		goto err_register_rc_device;
	}

	platform_set_drvdata(pdev, nxp_dev);

	INIT_WORK(&nxp_dev->ppm_event_work, ppm_irq_work);
	nxp_dev->ppm_workqueue = create_singlethread_workqueue(DEV_NAME_PPM);
	if (!nxp_dev->ppm_workqueue) {
		rc = -ESRCH;
		goto exit_create_singlethread;
	}

	rc = request_irq(IRQ_PHY_PPM,
			nxp_ir_recv_irq, 0, "nxp-ir-recv-irq", nxp_dev);
	if (rc < 0)
		goto err_request_irq;

	NX_PPM_SetInterruptEnableAll(0,true);

	NX_PPM_SetPPMEnable(0, true);

	nxp_ppm_dev = nxp_dev;

#ifdef CONFIG_PPM_SYSFS
	kobj = kobject_create_and_add("ppm",&platform_bus.kobj);
    if (! kobj)
		return -ENOMEM;
	rc = sysfs_create_group(kobj, &attr_group);
#endif

	return 0;

err_request_irq:
	cancel_work_sync(&nxp_dev->ppm_event_work);
	destroy_workqueue(nxp_dev->ppm_workqueue);
exit_create_singlethread:
	platform_set_drvdata(pdev, NULL);
	rc_unregister_device(rcdev);
err_register_rc_device:
	rc_free_device(rcdev);
	rcdev = NULL;
err_allocate_device:
	kfree(nxp_dev);
	return rc;
}

static int __devexit nxp_ir_recv_remove(struct platform_device *pdev)
{
	struct nxp_rc_dev *nxp_dev = platform_get_drvdata(pdev);

	free_irq(IRQ_PHY_PPM, nxp_dev);
	cancel_work_sync(&nxp_dev->ppm_event_work);
	destroy_workqueue(nxp_dev->ppm_workqueue);
	platform_set_drvdata(pdev, NULL);
	rc_unregister_device(nxp_dev->rcdev);
	rc_free_device(nxp_dev->rcdev);
	kfree(nxp_dev);
	return 0;
}

#ifdef CONFIG_PM
static int nxp_ir_recv_suspend(struct device *dev)
{
	disable_irq(IRQ_PHY_PPM);

	return 0;
}

static int nxp_ir_recv_resume(struct device *dev)
{
	struct nxp_rc_dev *nxp_dev = nxp_ppm_dev;
	struct clk *clk;

	nxp_soc_peri_reset_set(RESET_ID_PPM);

   	clk = clk_get(NULL, DEV_NAME_PPM);
	clk_set_rate(clk, CFG_PPM_CLK);
	clk_enable(clk);

	NX_PPM_SetInputSignalPolarity(0, nxp_dev->input_polarity);
	NX_PPM_SetInterruptEnableAll(0,true);
	NX_PPM_SetPPMEnable(0, true);

	enable_irq(IRQ_PHY_PPM);
	return 0;
}

static const struct dev_pm_ops nxp_ir_recv_pm_ops = {
	.suspend        = nxp_ir_recv_suspend,
	.resume         = nxp_ir_recv_resume,
};
#endif

static struct platform_driver nxp_ir_recv_driver = {
	.probe  = nxp_ir_recv_probe,
	.remove = __devexit_p(nxp_ir_recv_remove),
	.driver = {
		.name   = DEV_NAME_PPM,
		.owner  = THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &nxp_ir_recv_pm_ops,
#endif
	},
};

static int __init nxp_ir_recv_init(void)
{
	return platform_driver_register(&nxp_ir_recv_driver);
}
module_init(nxp_ir_recv_init);

static void __exit nxp_ir_recv_exit(void)
{
	platform_driver_unregister(&nxp_ir_recv_driver);
}
module_exit(nxp_ir_recv_exit);

MODULE_DESCRIPTION("NXP IR Receiver driver");
MODULE_LICENSE("GPL v2");
