#ifndef _NXP_EDID_H
#define _NXP_EDID_H

struct i2c_client;
struct nxp_v4l2_i2c_board_info;

struct nxp_edid {
    u32 preset;
    u32 edid_misc;
    int max_audio_ch;
    int bus_id;
    struct i2c_client *client;

    int (*update)(struct nxp_edid *);
    u32 (*enum_presets)(struct nxp_edid *, int);
    u32 (*preferred_preset)(struct nxp_edid *);
    bool (*supports_hdmi)(struct nxp_edid *);
    int (*max_audio_channels)(struct nxp_edid *);
    int (*suspend)(struct nxp_edid *);
    int (*resume)(struct nxp_edid *);
};

/**
 * public api
 */
int  nxp_edid_init(struct nxp_edid *, struct nxp_v4l2_i2c_board_info *);
void nxp_edid_cleanup(struct nxp_edid *);

#endif
