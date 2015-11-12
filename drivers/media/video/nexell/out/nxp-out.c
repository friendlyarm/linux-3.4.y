#define DEBUG 1

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-subdev.h>

#include <mach/nxp-v4l2-platformdata.h>
#include <mach/platform.h>

#include "nxp-mlc.h"
#if defined(CONFIG_NXP_OUT_HDMI)
#include "nxp-hdmi.h"
#endif
#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
#include "nxp-resc.h"
#endif
#if defined(CONFIG_NXP_OUT_TVOUT)
#include "nxp-tvout.h"
#endif

#include "nxp-out.h"

/**
 * public api
 */
struct nxp_out *create_nxp_out(struct nxp_out_platformdata *pdata)
{
    struct nxp_out *me = kzalloc(sizeof(struct nxp_out), GFP_KERNEL);

    pr_debug("%s\n", __func__);
    if (!me) {
        pr_err("%s: can't allocate me!!!\n", __func__);
        return NULL;
    }

    me->mlcs[0] = create_nxp_mlc(0);
    if (!me->mlcs[0]) {
        pr_err("%s: failed to create_nxp_mlc(0)\n", __func__);
        goto error_out;
    }

    me->mlcs[1] = create_nxp_mlc(1);
    if (!me->mlcs[1]) {
        pr_err("%s: failed to create_nxp_mlc(1)\n", __func__);
        goto error_out;
    }

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    me->resc = create_nxp_resc(0);
    if (!me->resc) {
        pr_err("%s: failed to create_nxp_resc()\n", __func__);
        goto error_out;
    }
#endif

#if defined(CONFIG_NXP_OUT_HDMI)
    me->hdmi = create_nxp_hdmi(&pdata->hdmi);
    if (!me->hdmi) {
        pr_err("%s: failed to create_nxp_hdmi()\n", __func__);
        goto error_out;
    }
#endif

#if defined(CONFIG_NXP_OUT_TVOUT)
    me->tvout = create_nxp_tvout();
    if (!me->tvout) {
        pr_err("%s: failed to create_nxp_tvout()\n", __func__);
        goto error_out;
    }
#endif

    return me;

error_out:
    if (me->mlcs[0])
        kfree(me->mlcs[0]);
    if (me->mlcs[1])
        kfree(me->mlcs[1]);
#if defined(CONFIG_NXP_OUT_TVOUT)
    if (me->tvout)
        kfree(me->tvout);
#endif
#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    if (me->resc)
        kfree(me->resc);
#endif
#if defined(CONFIG_NXP_OUT_HDMI)
    if (me->hdmi)
        kfree(me->hdmi);
#endif
    if (me)
        kfree(me);
    return NULL;
}

void release_nxp_out(struct nxp_out *me)
{
    pr_debug("%s\n", __func__);

#if defined(CONFIG_NXP_OUT_TVOUT)
    if (me->tvout)
        release_nxp_tvout(me->tvout);
#endif

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    if (me->resc)
        release_nxp_resc(me->resc);
#endif

#if defined(CONFIG_NXP_OUT_HDMI)
    if (me->hdmi)
        release_nxp_hdmi(me->hdmi);
#endif

    if (me->mlcs[1])
        release_nxp_mlc(me->mlcs[1]);

    if (me->mlcs[0])
        release_nxp_mlc(me->mlcs[0]);
}

int register_nxp_out(struct nxp_out *me)
{
    int ret;

    pr_debug("%s\n", __func__);

    ret = register_nxp_mlc(me->mlcs[0]);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_mlc(0)\n", __func__);
        goto error_out;
    }

    ret = register_nxp_mlc(me->mlcs[1]);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_mlc(1)\n", __func__);
        goto error_out;
    }

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    ret = register_nxp_resc(me->resc);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_resc()\n", __func__);
        goto error_out;
    }

    /* create link */
    /* link mlc0 pad source to hdmi pad sink */
    ret = media_entity_create_link(
            &me->mlcs[0]->subdev.entity, NXP_MLC_PAD_SOURCE,
            &me->resc->subdev.entity, 0,
            0);
    if (ret < 0) {
        pr_err("%s: failed to link mlc0 to resc\n", __func__);
        goto error_out;
    }
    /* link mlc1 pad source to resc pad sink */
    ret = media_entity_create_link(
            &me->mlcs[1]->subdev.entity, NXP_MLC_PAD_SOURCE,
            &me->resc->subdev.entity, 0,
            0);
    if (ret < 0) {
        pr_err("%s: failed to link mlc1 to resc\n", __func__);
        goto error_out;
    }
#endif

#if defined(CONFIG_NXP_OUT_HDMI)
    ret = register_nxp_hdmi(me->hdmi);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_hdmi()\n", __func__);
        goto error_out;
    }

    /* create link */
    /* link mlc0 pad source to hdmi pad sink */
    ret = media_entity_create_link(
            &me->mlcs[0]->subdev.entity, NXP_MLC_PAD_SOURCE,
            &me->hdmi->sd.entity, 0,
            0);
    if (ret < 0) {
        pr_err("%s: failed to link mlc0 to hdmi\n", __func__);
        goto error_out;
    }
    /* link mlc1 pad source to hdmi pad sink */
    ret = media_entity_create_link(
            &me->mlcs[1]->subdev.entity, NXP_MLC_PAD_SOURCE,
            &me->hdmi->sd.entity, 0,
            0);
    if (ret < 0) {
        pr_err("%s: failed to link mlc1 to hdmi\n", __func__);
        goto error_out;
    }

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    /* link resc pad source to hdmi pad sink */
    ret = media_entity_create_link(
            &me->resc->subdev.entity, NXP_RESC_PAD_SOURCE,
            &me->hdmi->sd.entity, 0,
            0);
    if (ret < 0) {
        pr_err("%s: failed to link resc to hdmi\n", __func__);
        goto error_out;
    }
#endif

#endif

#if defined(CONFIG_NXP_OUT_TVOUT)
    ret = register_nxp_tvout(me->tvout);
    if (ret < 0) {
        pr_err("%s: failed to register_nxp_tvout()\n", __func__);
        goto error_out;
    }

    /* create link */
    /* link mlc1 pad source to tvout pad sink */
    ret = media_entity_create_link(
            &me->mlcs[1]->subdev.entity, NXP_MLC_PAD_SOURCE,
            &me->tvout->sd.entity, 0,
            0);
    if (ret < 0) {
        pr_err("%s: failed to link mlc1 to tvout\n", __func__);
        goto error_out;
    }
#endif


    return 0;

error_out:
#if defined(CONFIG_NXP_OUT_TVOUT)
    unregister_nxp_tvout(me->tvout);
#endif

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    unregister_nxp_resc(me->resc);
#endif
#if defined(CONFIG_NXP_OUT_HDMI)
    unregister_nxp_hdmi(me->hdmi);
#endif
    unregister_nxp_mlc(me->mlcs[1]);
    unregister_nxp_mlc(me->mlcs[0]);
    return ret;
}

void unregister_nxp_out(struct nxp_out *me)
{
    pr_debug("%s\n", __func__);
#if defined(CONFIG_NXP_OUT_TVOUT)
    unregister_nxp_tvout(me->tvout);
#endif
#if defined(CONFIG_NXP_OUT_HDMI)
    unregister_nxp_hdmi(me->hdmi);
#endif
    unregister_nxp_mlc(me->mlcs[1]);
    unregister_nxp_mlc(me->mlcs[0]);
}

#ifdef CONFIG_PM
int suspend_nxp_out(struct nxp_out *me)
{
    int ret;
    PM_DBGOUT("+%s\n", __func__);

#if defined(CONFIG_NXP_OUT_TVOUT)
    ret = suspend_nxp_tvout(me->tvout);
    if (ret) {
        PM_DBGOUT("%s: failed to suspend_nxp_tvout, ret %d\n", __func__, ret);
        return ret;
    }
#endif

#if defined(CONFIG_NXP_OUT_HDMI)
    ret = suspend_nxp_hdmi(me->hdmi);
    if (ret) {
        PM_DBGOUT("%s: failed to suspend_nxp_hdmi, ret %d\n", __func__, ret);
        return ret;
    }
#endif

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    ret = suspend_nxp_resc(me->resc);
    if (ret) {
        PM_DBGOUT("%s: failed to suspend_nxp_resc, ret %d\n", __func__, ret);
        return ret;
    }
#endif

    ret = suspend_nxp_mlc(me->mlcs[0]);
    if (ret) {
        PM_DBGOUT("%s: failed to suspend_nxp_mlc0, ret %d\n", __func__, ret);
        return ret;
    }

    ret = suspend_nxp_mlc(me->mlcs[1]);
    if (ret) {
        PM_DBGOUT("%s: failed to suspend_nxp_mlc1, ret %d\n", __func__, ret);
        return ret;
    }

    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

int resume_nxp_out(struct nxp_out *me)
{
    PM_DBGOUT("+%s\n", __func__);

    resume_nxp_mlc(me->mlcs[0]);
    resume_nxp_mlc(me->mlcs[1]);
#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    resume_nxp_resc(me->resc);
#endif
#if defined(CONFIG_NXP_OUT_HDMI)
    resume_nxp_hdmi(me->hdmi);
#endif
#if defined(CONFIG_NXP_OUT_TVOUT)
    resume_nxp_tvout(me->tvout);
#endif

    PM_DBGOUT("-%s\n", __func__);

    return 0;
}
#endif
