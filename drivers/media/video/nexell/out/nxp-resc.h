#ifndef _NXP_RESC_H
#define _NXP_RESC_H

#include <linux/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

struct sync_preset;

enum nxp_resc_pad_type {
    NXP_RESC_PAD_SINK,
    NXP_RESC_PAD_SOURCE,
    NXP_RESC_PAD_MAX
};

struct nxp_resc {
    int dynamic_change_count;

    int id;
    bool enabled;

    struct v4l2_subdev subdev;
    struct media_pad pads[NXP_RESC_PAD_MAX];

    struct v4l2_mbus_framefmt format[2]; /* source/sink */
    struct v4l2_rect crop[2]; /* source/sink */

    int source_device;
    int dpc_module_num;

    struct sync_preset *preset;
    uint32_t remote_preset;

    bool streaming;

    struct disp_irq_callback *callback;
};

/**
 * public api
 */
struct nxp_resc *create_nxp_resc(int id);
void release_nxp_resc(struct nxp_resc *);
int  register_nxp_resc(struct nxp_resc *);
void unregister_nxp_resc(struct nxp_resc *);
#ifdef CONFIG_PM
int  suspend_nxp_resc(struct nxp_resc *);
int  resume_nxp_resc(struct nxp_resc *);
#endif

#endif
