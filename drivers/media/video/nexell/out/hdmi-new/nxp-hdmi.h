#ifndef _NXP_HDMI_H
#define _NXP_HDMI_H

#ifdef CONFIG_ANDROID
#include <linux/wakelock.h>
#endif

#include <mach/display.h>
#include <mach/hdmi/nxp-hdmi-context.h>

struct nxp_hdmi_platformdata;
struct media_pad;
struct v4l2_subdev;

//struct nxp_hdmi_context;

struct nxp_hdmi {
    struct v4l2_subdev sd;
    struct media_pad pad;
    struct nxp_hdmi_context ctx;
    struct nxp_hdmi_platformdata *plat_data;
    bool   streaming;

#ifdef CONFIG_ANDROID
    struct wake_lock wake_lock;
#endif
};

/**
 * publi api
 */
struct nxp_hdmi *create_nxp_hdmi(struct nxp_hdmi_platformdata *);
void release_nxp_hdmi(struct nxp_hdmi *);
int register_nxp_hdmi(struct nxp_hdmi *);
void unregister_nxp_hdmi(struct nxp_hdmi *);
int suspend_nxp_hdmi(struct nxp_hdmi *);
int resume_nxp_hdmi(struct nxp_hdmi *);

#endif
