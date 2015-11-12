#define DEBUG 1

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <media/v4l2-ctrls.h>
#include <media/media-device.h>
#include <media/videobuf2-ion-nxp.h>

#include <mach/nxp-v4l2-platformdata.h>
#include <mach/platform.h>

#include "nxp-capture.h"
#include "nxp-scaler.h"
#include "nxp-out.h"
#include "nxp-v4l2.h"
#include "loopback-sensor.h"

/**
 * static variable for public api
 */
static struct nxp_v4l2 *__me = NULL;

struct media_device *nxp_v4l2_get_media_device(void)
{
    if (likely(__me))
        return &__me->media_dev;
    return NULL;
}

struct v4l2_device *nxp_v4l2_get_v4l2_device(void)
{
    if (likely(__me))
        return &__me->v4l2_dev;
    return NULL;
}

void *nxp_v4l2_get_alloc_ctx(void)
{
    if (likely(__me))
        return &__me->v4l2_dev;
    return NULL;
}

static int __devinit nxp_v4l2_probe(struct platform_device *pdev)
{
    struct v4l2_device *v4l2_dev;
    struct nxp_v4l2 *nxp_v4l2;
    struct nxp_v4l2_platformdata *pdata;
#ifdef CONFIG_VIDEO_NXP_CAPTURE
    struct nxp_capture_platformdata *capture_pdata;
    struct nxp_capture *capture;
    int i;
#endif
#ifdef CONFIG_NXP_M2M_SCALER
    struct nxp_scaler *scaler;
#endif

#ifdef CONFIG_LOOPBACK_SENSOR_DRIVER
		struct nxp_loopback_sensor *loopback_sensor = NULL;
#endif

    int ret;

    pr_debug("%s entered\n", __func__);

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "can't get platformdata\n");
        return -EINVAL;
    }

    nxp_v4l2 = kzalloc(sizeof(*nxp_v4l2), GFP_KERNEL);
    if (!nxp_v4l2) {
        pr_err("%s error: fail to kzalloc(size %d)\n", __func__, sizeof(struct nxp_v4l2));
        return -ENOMEM;
    }

    nxp_v4l2->pdev = pdev;
    nxp_v4l2->pdata = pdata;

    snprintf(nxp_v4l2->media_dev.model, sizeof(nxp_v4l2->media_dev.model), "%s",
            dev_name(&pdev->dev));

    nxp_v4l2->media_dev.dev = &pdev->dev;

    v4l2_dev       = &nxp_v4l2->v4l2_dev;
    v4l2_dev->mdev = &nxp_v4l2->media_dev;
    snprintf(v4l2_dev->name, sizeof(v4l2_dev->name), "%s",
            dev_name(&pdev->dev));

    /* alloc context : use uncached area */
    nxp_v4l2->alloc_ctx =
        vb2_ion_create_context(&pdev->dev, SZ_4K, VB2ION_CTX_UNCACHED);
    if (!nxp_v4l2->alloc_ctx) {
        pr_err("%s: failed to ion alloc context\n", __func__);
        ret = -ENOMEM;
        goto err_alloc_ctx;
    }

    ret = v4l2_device_register(&pdev->dev, &nxp_v4l2->v4l2_dev);
    if (ret < 0) {
        pr_err("%s: failed to register v4l2_device: %d\n", __func__, ret);
        goto err_v4l2_reg;
    }

    ret = media_device_register(&nxp_v4l2->media_dev);
    if (ret < 0) {
        pr_err("%s: failed to register media_device: %d\n", __func__, ret);
        goto err_media_reg;
    }

    __me = nxp_v4l2;

#ifdef CONFIG_LOOPBACK_SENSOR_DRIVER
		loopback_sensor =	create_nxp_loopback_sensor(pdata->captures);
		if (!loopback_sensor) {
				pr_err("%s: failed to create_nxp_loopback_sensor()\n", __func__);
				ret = -EINVAL;
				goto err_loopback_sensor_create;
		}

		ret = register_nxp_loopback_sensor(loopback_sensor);
		if (ret < 0) {
				pr_err("%s: failed to register_nxp_loopback_sensor()\n", __func__);
				goto err_loopback_sensor_create;
		}

    nxp_v4l2->loopback_sensor = loopback_sensor;
#endif

#ifdef CONFIG_VIDEO_NXP_CAPTURE
    /* capture */
    for (capture_pdata = pdata->captures, i = 0;
            capture_pdata->sensor;
            capture_pdata++, i++) {
        /* TODO : psw0523 test for module index problem !!! */
        /* capture = create_nxp_capture(i, capture_pdata); */
        capture = create_nxp_capture(i, capture_pdata->module, capture_pdata);
        if (!capture) {
            pr_err("%s: failed to %dth create_nxp_capture()\n", __func__, i);
            ret = -EINVAL;
            goto err_capture_create;
        }

        ret = register_nxp_capture(capture);
        if (ret < 0) {
            pr_err("%s: failed to %dth register_nxp_capture()\n", __func__, i);
            goto err_capture_create;
        }

        nxp_v4l2->capture[i] = capture;
    }
#endif

#ifdef CONFIG_NXP_M2M_SCALER
    /* m2m */
    scaler = create_nxp_scaler();
    if (!scaler) {
        pr_err("%s: failed to create_nxp_scaler()\n", __func__);
        ret = -ENOMEM;
#ifdef CONFIG_VIDEO_NXP_CAPTURE
        goto err_capture_create;
#else
        goto err_media_reg;
#endif
    }

    ret = register_nxp_scaler(scaler);
    if (ret < 0) {
        pr_err("%s: failed to nxp_scaler_register()\n", __func__);
        goto err_register_scaler;
    }
    nxp_v4l2->scaler = scaler;
#endif

#ifdef CONFIG_VIDEO_NXP_OUT
    /* out */
    nxp_v4l2->out = create_nxp_out(pdata->out);
    if (!nxp_v4l2->out) {
        pr_err("%s: failed to create_nxp_out()\n", __func__);
        goto err_create_out;
    }

    ret = register_nxp_out(nxp_v4l2->out);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_out()\n", __func__);
        goto err_register_out;
    }
#endif

    ret = v4l2_device_register_subdev_nodes(&nxp_v4l2->v4l2_dev);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev_nodes()\n", __func__);
        goto err_register_out_subdev;
    }

    platform_set_drvdata(pdev, nxp_v4l2);
   // printk("%s success!!!\n", __func__);

    return 0;

err_register_out_subdev:
#ifdef CONFIG_VIDEO_NXP_OUT
    unregister_nxp_out(nxp_v4l2->out);
err_register_out:
    release_nxp_out(nxp_v4l2->out);
err_create_out:
#endif
#ifdef CONFIG_NXP_M2M_SCALER
    unregister_nxp_scaler(scaler);
err_register_scaler:
    release_nxp_scaler(scaler);
#endif
#ifdef CONFIG_VIDEO_NXP_CAPTURE
err_capture_create:
    for (i = 0; i < NXP_MAX_CAPTURE_NUM; ++i) {
        capture = nxp_v4l2->capture[i];
        if (capture) {
            unregister_nxp_capture(capture);
            release_nxp_capture(capture);
        }
    }
    media_device_unregister(&nxp_v4l2->media_dev);
#endif
#ifdef CONFIG_LOOPBACK_SENSOR_DRIVER
err_loopback_sensor_create:
		if( loopback_sensor )
		{
			unregister_nxp_loopback_sensor(loopback_sensor);
    	release_nxp_loopback_sensor(loopback_sensor);
		}
#endif

err_media_reg:
    v4l2_device_unregister(&nxp_v4l2->v4l2_dev);
err_v4l2_reg:
    vb2_ion_destroy_context(nxp_v4l2->alloc_ctx);
err_alloc_ctx:
    kfree(nxp_v4l2);
    __me = NULL;
    return ret;
}

static int __devexit nxp_v4l2_remove(struct platform_device *pdev)
{
    struct nxp_v4l2 *nxp_v4l2 = platform_get_drvdata(pdev);
#ifdef CONFIG_VIDEO_NXP_CAPTURE
    int i;
#endif

    if (!nxp_v4l2)
        return 0;

#ifdef CONFIG_VIDEO_NXP_OUT
    unregister_nxp_out(nxp_v4l2->out);
    release_nxp_out(nxp_v4l2->out);
#endif

#ifdef CONFIG_NXP_M2M_SCALER
    if (nxp_v4l2->scaler) {
        unregister_nxp_scaler(nxp_v4l2->scaler);
        release_nxp_scaler(nxp_v4l2->scaler);
    }
#endif

#ifdef CONFIG_VIDEO_NXP_CAPTURE
    for (i = 0; i < NXP_MAX_CAPTURE_NUM; ++i) {
        struct nxp_capture *capture = nxp_v4l2->capture[i];
        if (capture) {
            unregister_nxp_capture(capture);
            release_nxp_capture(capture);
        }
    }
#endif

    media_device_unregister(&nxp_v4l2->media_dev);
    v4l2_device_unregister(&nxp_v4l2->v4l2_dev);
    vb2_ion_destroy_context(nxp_v4l2->alloc_ctx);
    kfree(nxp_v4l2);

    __me = NULL;
    return 0;
}

#ifdef CONFIG_PM
static int nxp_v4l2_suspend(struct device *dev)
{
    int ret;
    int i;

    PM_DBGOUT("+%s\n", __func__);

    if (!__me) {
        PM_DBGOUT("%s: No Exist\n", __func__);
        return 0;
    }

#ifdef CONFIG_VIDEO_NXP_CAPTURE
    for (i = 0; i < NXP_MAX_CAPTURE_NUM; i++) {
        if (__me->capture[i]) {
            ret = suspend_nxp_capture(__me->capture[i]);
            if (ret) {
                PM_DBGOUT("failed to suspend capture %d\n", i);
                return ret;
            }
        }
    }
#endif

#ifdef CONFIG_NXP_M2M_SCALER
    if (__me->scaler) {
        ret = suspend_nxp_scaler(__me->scaler);
        if (ret) {
            PM_DBGOUT("failed to suspend scaler\n");
            return ret;
        }
    }
#endif

#ifdef CONFIG_VIDEO_NXP_OUT
    if (__me->out) {
        ret = suspend_nxp_out(__me->out);
        if (ret) {
            PM_DBGOUT("failed to suspend out\n");
            return ret;
        }
    }
#endif

    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

static int nxp_v4l2_resume(struct device *dev)
{
    int ret;
    int i;

    PM_DBGOUT("+%s\n", __func__);
#ifdef CONFIG_VIDEO_NXP_CAPTURE
    for (i = 0; i < NXP_MAX_CAPTURE_NUM; i++) {
        if (__me->capture[i]) {
            ret = resume_nxp_capture(__me->capture[i]);
            if (ret) {
                PM_DBGOUT("failed to suspend capture %d\n", i);
                return ret;
            }
        }
    }
#endif

#ifdef CONFIG_NXP_M2M_SCALER
    if (__me->scaler) {
        ret = resume_nxp_scaler(__me->scaler);
        if (ret) {
            PM_DBGOUT(KERN_ERR "failed to suspend scaler\n");
            return ret;
        }
    }
#endif

#ifdef CONFIG_VIDEO_NXP_OUT
    if (__me->out) {
        ret = resume_nxp_out(__me->out);
        if (ret) {
            PM_DBGOUT(KERN_ERR "failed to suspend out\n");
            return ret;
        }
    }
#endif

    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif

/* #ifdef CONFIG_PM_RUNTIME */
/* static int nxp_v4l2_runtime_suspend(struct device *dev) */
/* { */
/*     printk("%s entered\n", __func__); */
/*     printk("%s exit\n", __func__); */
/* } */
/*  */
/* static int nxp_v4l2_runtime_resume(struct device *dev) */
/* { */
/*     printk("%s entered\n", __func__); */
/*     printk("%s exit\n", __func__); */
/* } */
/* #endif */

#ifdef CONFIG_PM
static const struct dev_pm_ops nxp_v4l2_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(nxp_v4l2_suspend, nxp_v4l2_resume)
    /* SET_RUNTIME_PM_OPS(nxp_v4l2_runtime_suspend, nxp_v4l2_runtime_resume, NULL) */
};

#define NXP_V4L2_PMOPS (&nxp_v4l2_pm_ops)
#else
#define NXP_V4L2_PMOPS NULL
#endif

static struct platform_device_id nxp_v4l2_id_table[] = {
    { NXP_V4L2_DEV_NAME, 0 },
    { },
};

static struct platform_driver nxp_v4l2_driver = {
    .probe      = nxp_v4l2_probe,
    .remove     = nxp_v4l2_remove,
    .id_table   = nxp_v4l2_id_table,
    .driver     = {
        .name   = NXP_V4L2_DEV_NAME,
        .owner  = THIS_MODULE,
        .pm = NXP_V4L2_PMOPS,
    },
};

#ifdef CONFIG_NXP_OUT_HDMI
struct nxp_hdmi *get_nxp_hdmi(void) {
     return __me->out->hdmi;
}
#endif

module_platform_driver(nxp_v4l2_driver);

MODULE_AUTHOR("swpark <swpark@nexell.co.kr>");
MODULE_DESCRIPTION("Nexell NXP series V4L2/MEDIA top device driver");
MODULE_LICENSE("GPL");
