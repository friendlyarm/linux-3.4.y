#ifndef _SYNC_PRESET_H
#define _SYNC_PRESET_H

#include <linux/kernel.h>

enum HDMI_CLK_ENUM {
    HDMI_CLK_74_25 = 0,
    HDMI_CLK_147
};

struct sync_resolution {
    uint32_t width;
    uint32_t height;
};

struct dpc_sync_param {
    uint32_t hact;
    uint32_t vact;
    uint32_t hfp;
    uint32_t hsw;
    uint32_t hbp;
    uint32_t vfp;
    uint32_t vsw;
    uint32_t vbp;
    bool     use_dynamic;
    uint32_t dynamic_change_num;
    uint32_t *dynamic_tbl;
    uint32_t clk_src_lv0;
    uint32_t clk_div_lv0;
    uint32_t clk_src_lv1;
    uint32_t clk_div_lv1;
};

struct resc_sync_param {
    uint32_t hact;
    uint32_t vact;
    uint32_t hfp;
    uint32_t hsw;
    uint32_t hbp;
    uint32_t vfp;
    uint32_t vsw;
    uint32_t vbp;
    uint32_t hdelay;
    uint32_t hoffset;
};

struct sync_preset {
    int    index;
    struct sync_resolution mlc_resol;
    struct sync_resolution resc_resol;
    struct sync_resolution hdmi_resol;
    struct dpc_sync_param dpc_sync_param;
    struct resc_sync_param resc_sync_param;
};

extern struct sync_preset nxp_out_sync_presets[];

struct sync_preset *nxp_out_find_sync_preset(uint32_t mlc_width,
        uint32_t mlc_height,
        uint32_t resc_width,
        uint32_t resc_height,
        uint32_t hdmi_width,
        uint32_t hdmi_height);

#endif
