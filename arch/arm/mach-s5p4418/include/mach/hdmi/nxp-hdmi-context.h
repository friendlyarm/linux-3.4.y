#ifndef __NXP_MACH_HDMI__
#define __NXP_MACH_HDMI__ __FILE__

#include <linux/mutex.h>
#include <linux/switch.h>
#include <linux/list.h>

#include <mach/hdmi/regs-hdmi.h>
#include <mach/hdmi/hdmi-priv.h>
#include <mach/hdmi/hdmi-preset.h>
#include <mach/hdmi/nxp-hdmiphy.h>
#include <mach/hdmi/nxp-edid.h>
#include <mach/hdmi/nxp-hdcp.h>

enum {
    HDMI_UNPLUGGED  = (1 << 0),
    HDMI_PLUGGED    = (1 << 1),
    HDMI_STOPPED    = (1 << 2),
    HDMI_RUNNING    = (1 << 3),
};

struct hdmi_sync_param {
    u32 v_sync_start;
    u32 h_active_start;
    u32 h_active_end;
    u32 v_sync_hs_start_end0;
    u32 v_sync_hs_start_end1;
};

struct hdmi_irq_callback {
    struct list_head list;
    int (*callback)(void *);
    void *data;
    int irq_num;
};

struct delayed_work;
struct nxp_hdmi_platformdata;

struct nxp_hdmi_context {
    struct mutex mutex;

    bool initialized;
    atomic_t state;
    int irq;

    u32 audio_codec;
    bool audio_enable;
    u32 audio_channel_count;
    int sample_rate;
    int bits_per_sample;

    u32 aspect;
    int color_range;
    bool is_dvi;
    struct hdmi_infoframe infoframe[2];

    u32 cur_preset;
    struct hdmi_preset_conf *cur_conf;

    struct delayed_work hpd_work;
    struct switch_dev hpd_switch;

    /* sync */
    u32 source_dpc_module_num;
    int source_device;
    struct disp_vsync_info dpc_sync_info;
    struct disp_syncgen_par dpc_sync_param;
    struct hdmi_sync_param  hdmi_sync_param;

    /* HDCP */
    struct nxp_hdcp hdcp;

    /* PHY */
    struct nxp_hdmiphy phy;

    /* EDID */
    struct nxp_edid edid;

    /* irq callback */
    spinlock_t lock_callback;
    struct list_head callback_list;

    /* hook : client must set this field */
    void (*set_hdmi_mux)(struct nxp_hdmi_context *);

    /* hook ; hpd change notifier */
    void (*notify_hpd_changed)(void *, int);
    void *notify_data;
};

int hdmi_init_context(struct nxp_hdmi_context *, struct nxp_v4l2_i2c_board_info *, struct nxp_v4l2_i2c_board_info *);
void hdmi_deinit_context(struct nxp_hdmi_context *);
void hdmi_get_vsync(struct nxp_hdmi_context *);
void hdmi_hook_set_mux(struct nxp_hdmi_context *, void (*)(struct nxp_hdmi_context *));
int  hdmi_run(struct nxp_hdmi_context *, bool);
void hdmi_stop(struct nxp_hdmi_context *);
void hdmi_enable_audio(struct nxp_hdmi_context *, bool);
void hdmi_enable_pattern_gen(struct nxp_hdmi_context *, bool);
void hdmi_set_infoframe(struct nxp_hdmi_context *);
//int  hdmi_set_dv_preset(struct nxp_hdmi_context *, u32);
int  hdmi_set_dv_preset(struct nxp_hdmi_context *, struct v4l2_dv_preset *);
int  hdmi_set_preset(struct nxp_hdmi_context *, uint32_t );
bool hdmi_is_connected(void);
uint32_t hdmi_get_edid_preset(struct nxp_hdmi_context *, uint32_t);
struct hdmi_irq_callback *hdmi_register_irq_callback(int , int (*)(void *), void *);
void hdmi_unregister_irq_callback(struct hdmi_irq_callback *);
void hdmi_register_notifier_hpd_changed(struct nxp_hdmi_context *, void (*)(void *, int), void *);
int hdmi_suspend(struct nxp_hdmi_context *);
int hdmi_resume(struct nxp_hdmi_context *);
u32 hdmi_get_edid_cec_phy_address(void);

#endif
