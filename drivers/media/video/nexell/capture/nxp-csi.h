#ifndef _NXP_MIPI_CSI_H
#define _NXP_MIPI_CSI_H

#include <linux/atomic.h>

enum nxp_csi_pads {
    NXP_CSI_PAD_SINK = 0,
    NXP_CSI_PAD_SOURCE,
    NXP_CSI_PAD_MAX,
};

enum nxp_csi_state {
    //NXP_CSI_STATE_POWERED   = (1 << 0),
    NXP_CSI_STATE_INPUT_CONNECTED  = (1 << 1),
    NXP_CSI_STATE_OUTPUT_CONNECTED = (1 << 2),
    NXP_CSI_STATE_READY =
        (NXP_CSI_STATE_INPUT_CONNECTED | NXP_CSI_STATE_OUTPUT_CONNECTED),
    NXP_CSI_STATE_RUNNING = (1 << 4),
    NXP_CSI_STATE_SUSPEND = (1 << 31),
};

struct nxp_mipi_csi_platformdata;
struct nxp_csi_pix_format;

struct nxp_csi {
    int  module; /* for later extension */

    void __iomem *regs;

    atomic_long_t state;

    int irq;

    struct v4l2_mbus_framefmt format;
    struct nxp_csi_pix_format *csi_format;

    struct media_pad pads[NXP_CSI_PAD_MAX];
    struct v4l2_subdev subdev;

    struct nxp_mipi_csi_platformdata *platdata;
};


/* public api */
int  nxp_csi_init(struct nxp_csi *, struct nxp_mipi_csi_platformdata *);
void nxp_csi_cleanup(struct nxp_csi *);
int  nxp_csi_register(struct nxp_csi *);
void nxp_csi_unregister(struct nxp_csi *);
int  nxp_csi_suspend(struct nxp_csi *);
int  nxp_csi_resume(struct nxp_csi *);

/* helper macros */
#define subdev_to_nxp_csi(subdev) container_of(subdev, struct nxp_csi, subdev)
#define nxp_csi_to_parent(me)     container_of(me, struct nxp_capture, csi)

#endif
