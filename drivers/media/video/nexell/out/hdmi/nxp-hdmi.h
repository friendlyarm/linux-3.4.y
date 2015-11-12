#ifndef _NXP_HDMI_H
#define _NXP_HDMI_H

#include <linux/mutex.h>
#include <linux/switch.h>

#ifdef CONFIG_ANDROID
#include <linux/wakelock.h>
#endif

#include "hdmi-priv.h"
#include "nxp-hdmiphy.h"
#include "nxp-edid.h"
#include "nxp-hdcp.h"

struct nxp_hdmi_platformdata;
struct media_pad;
struct v4l2_subdev;
struct work_struct;
struct delayed_work;

#define INFOFRAME_CNT       2

struct nxp_hdmi {
    /* from nxp_hdmi_platformdata */
    int internal_irq; /* hdmi hotplug disconnect irq : hdmi internal*/
    int external_irq; /* hdmi hotplug connect irq : gpio */
    void (*set_int_external)(int);
    void (*set_int_internal)(int);
    int  (*read_hpd_gpio)(int);

    bool initialized;
    bool streaming;
    bool (*is_streaming)(struct nxp_hdmi *);

    /* audio */
    u32 audio_codec; /* enum HDMI_AUDIO_CODEC */
    bool audio_enable;
    u32 audio_channel_count;
    int sample_rate;
    int bits_per_sample;

    /* display */
    u32 output_fmt; /* enum HDMI_OUTPUT_FMT */
    u32 aspect; /* enum HDMI_ASPECT_RATIO */
    int color_range;
    bool is_dvi;
    struct hdmi_infoframe infoframe[INFOFRAME_CNT];

    /* preset */
    u32 cur_preset;
    struct hdmi_preset_conf *cur_conf;

    /* HPD */
    // struct work_struct hpd_work;
    struct delayed_work hpd_work;
    struct delayed_work hpd_work_ext;
    struct switch_dev hpd_switch;

    /* HDCP */
    struct nxp_hdcp hdcp;

    /* PHY */
    struct nxp_hdmiphy phy;

    /* EDID */
    struct nxp_edid edid;

    /* v4l2 specific */
    struct v4l2_subdev sd;
    /* only 1 pad, sink
     * available source : mlc0, mlc1, resolution converter
     **/
    struct media_pad pad;

    struct mutex mutex;

    /**
     * hdmi sync variables
     */
    u32 VSyncStart;
    u32 HActiveStart;
    u32 HActiveEnd;
    u32 VSyncHSStartEnd0;
    u32 VSyncHSStartEnd1;

    /**
     * hdmi source dpc module number
     */
    u32 source_dpc_module_num;
    u32 width;
    u32 height;
    int source_device;

    /**
     * when hdmi connected, disable suspend by wake lock
     */
#ifdef CONFIG_ANDROID
    struct wake_lock wake_lock;
#endif

    int (*callback)(void *);
    void *callback_data;
    int irq_num_of_callback;
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
