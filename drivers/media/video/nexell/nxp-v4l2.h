#ifndef _NXP_V4L2_H
#define _NXP_V4L2_H

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/irqreturn.h>
#include <media/media-device.h>
#include <media/media-entity.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

/* group id */
/* TODO: necessary??? */
#define NXP_CAPTURE_SUBDEV_GROUP_ID (1 << 16)
#define NXP_M2M_SUBDEV_GROUP_ID     (1 << 17)
#define NXP_OUT_SUBDEV_GROUP_ID     (1 << 18)

#define NXP_MAX_CAPTURE_NUM     2

struct nxp_v4l2_platformdata;
struct nxp_capture;
struct nxp_scaler;
struct nxp_out;
struct nxp_loopback_sensor;

struct nxp_v4l2 {
    struct media_device media_dev;
    struct v4l2_device  v4l2_dev;
    struct platform_device *pdev;

    struct nxp_v4l2_platformdata *pdata;

    /* child */
#ifdef CONFIG_VIDEO_NXP_CAPTURE
    struct nxp_capture *capture[NXP_MAX_CAPTURE_NUM];
#endif
#ifdef CONFIG_NXP_M2M_SCALER
    struct nxp_scaler *scaler;
#endif
#ifdef CONFIG_VIDEO_NXP_OUT
    struct nxp_out *out;
#endif
#ifdef CONFIG_LOOPBACK_SENSOR_DRIVER
		struct nxp_loopback_sensor *loopback_sensor;
#endif

    void *alloc_ctx;
};

struct media_device *nxp_v4l2_get_media_device(void);
struct v4l2_device  *nxp_v4l2_get_v4l2_device(void);
void *nxp_v4l2_get_alloc_ctx(void);

/* for debugging */
//#define VERBOSE_NXP_V4L2
#ifdef VERBOSE_NXP_V4L2
#define vmsg(a...)  printk(a)
#else
#define vmsg(a...)
#endif

#endif
