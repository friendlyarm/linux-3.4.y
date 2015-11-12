#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/videodev2_nxp_media.h>

#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#include <mach/platform.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>

#include "nxp-v4l2.h"
#include "nxp-tvout.h"

/**
 * video ops
 */
static int nxp_tvout_s_stream(struct v4l2_subdev *sd, int enable)
{
    printk("%s: %d\n", __func__, enable);
    nxp_soc_disp_device_enable_all(DISP_DEVICE_SYNCGEN1, enable);
    return 0;
}

static const struct v4l2_subdev_video_ops nxp_tvout_video_ops = {
    .s_stream = nxp_tvout_s_stream,
};

/**
 * subdev ops
 */
static const struct v4l2_subdev_ops nxp_tvout_subdev_ops = {
    .video = &nxp_tvout_video_ops,
};

static int _init_entities(struct nxp_tvout *me)
{
    int ret;
    struct v4l2_subdev *sd = &me->sd;
    struct media_pad *pad  = &me->pad;
    struct media_entity *entity = &sd->entity;

    v4l2_subdev_init(sd, &nxp_tvout_subdev_ops);

    strlcpy(sd->name, "NXP TVOUT", sizeof(sd->name));
    v4l2_set_subdevdata(sd, me);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    pad->flags = MEDIA_PAD_FL_SINK;
    ret = media_entity_init(entity, 1, pad, 0);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

    return 0;
}

struct nxp_tvout *create_nxp_tvout(void)
{
    int ret;
    struct nxp_tvout *me;

    me = kzalloc(sizeof(*me), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to alloc me!!!\n", __func__);
        return NULL;
    }

    ret = _init_entities(me);
    if (ret < 0) {
        pr_err("%s: failed to _init_entities()\n", __func__);
        kfree(me);
        return NULL;
    }

    return me;
}

void release_nxp_tvout(struct nxp_tvout *me)
{
    kfree(me);
}

int register_nxp_tvout(struct nxp_tvout *me)
{
    int ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(), &me->sd);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    return 0;
}

void unregister_nxp_tvout(struct nxp_tvout *me)
{
    v4l2_device_unregister_subdev(&me->sd);
}

int suspend_nxp_tvout(struct nxp_tvout *me)
{
    // TODO
    return 0;
}

int resume_nxp_tvout(struct nxp_tvout *me)
{
    // TODO
    return 0;
}
