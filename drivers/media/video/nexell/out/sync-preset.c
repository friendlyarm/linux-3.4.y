#include "sync-preset.h"

static uint32_t _dyntbl_1280_800_1280_720[] = {31, 31, 32};

struct sync_preset nxp_out_sync_presets[] = {
    /*
     * MLC         RESC       HDMI
     * 1280x800 | 1280x720 | 1280x720
     */
    {
        .index     = 0,
        .mlc_resol = {
            .width  = 1280,
            .height = 800
        },
        .resc_resol = {
            .width  = 1280,
            .height = 720
        },
        .hdmi_resol = {
            .width  = 1280,
            .height = 720
        },
        .dpc_sync_param = {
            .hact   = 1280,
            .vact   = 800,
            .hfp    = 1,
            .hsw    = 203,
            .hbp    = 1,
            .vfp    = 1,
            /* .vsw    = 32, */
            .vsw    = 31,
            .vbp    = 1,
            .use_dynamic    = true,
            .dynamic_change_num = 3,
            .dynamic_tbl    = _dyntbl_1280_800_1280_720,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 1280,
            .vact   = 720,
            .hfp    = 110,
            .hsw    = 40,
            .hbp    = 220,
            .vfp    = 5,
            .vsw    = 5,
            .vbp    = 20,
            .hdelay = 7,
            .hoffset = 255
        }
    },
    /*
     * MLC         RESC       HDMI
     * 1280x800 | 1152x720 | 1280x720
     */
    {
        .index     = 1,
        .mlc_resol = {
            .width  = 1280,
            .height = 800
        },
        .resc_resol = {
            .width  = 1152,
            .height = 720
        },
        .hdmi_resol = {
            .width  = 1280,
            .height = 720
        },
        .dpc_sync_param = {
            .hact   = 1280,
            .vact   = 800,
            .hfp    = 1,
            .hsw    = 203,
            .hbp    = 1,
            .vfp    = 1,
            /* .vsw    = 32, */
            .vsw    = 31,
            .vbp    = 1,
            .use_dynamic    = true,
            .dynamic_change_num = 3,
            .dynamic_tbl    = _dyntbl_1280_800_1280_720,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 1280,
            .vact   = 720,
            .hfp    = 110,
            .hsw    = 40,
            .hbp    = 220,
            .vfp    = 5,
            .vsw    = 5,
            .vbp    = 20,
            .hdelay = 7,
            .hoffset = 255
        }
    },
    /*
     * MLC         RESC       HDMI
     * 1024x600 | 1280x720 | 1280x720
     */
    {
        .index     = 2,
        .mlc_resol = {
            .width  = 1024,
            .height = 600
        },
        .resc_resol = {
            .width  = 1280,
            .height = 720
        },
        .hdmi_resol = {
            .width  = 1280,
            .height = 720
        },
        .dpc_sync_param = {
            .hact   = 1024,
            .vact   = 600,
            .hfp    = 1,
            .hsw    = 954,
            .hbp    = 1,
            .vfp    = 4,
            .vsw    = 15,
            .vbp    = 6,
            .use_dynamic    = false,
            .dynamic_change_num = 0,
            .dynamic_tbl    = NULL,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 1280,
            .vact   = 720,
            .hfp    = 110,
            .hsw    = 40,
            .hbp    = 220,
            .vfp    = 5,
            .vsw    = 5,
            .vbp    = 20,
            .hdelay = 3,
            .hoffset = 0
        }
    },
    /*
     * MLC         RESC       HDMI
     * 1280x720 | 1280x720 | 1280x720
     */
    {
        .index     = 3,
        .mlc_resol = {
            .width  = 1280,
            .height = 720
        },
        .resc_resol = {
            .width  = 1280,
            .height = 720
        },
        .hdmi_resol = {
            .width  = 1280,
            .height = 720
        },
        .dpc_sync_param = {
            .hact   = 1280,
            .vact   = 720,
            .hfp    = 1,
            .hsw    = 368,
            .hbp    = 1,
            .vfp    = 1,
            .vsw    = 28,
            .vbp    = 1,
            .use_dynamic    = false,
            .dynamic_change_num = 0,
            .dynamic_tbl    = NULL,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 1280,
            .vact   = 720,
            .hfp    = 110,
            .hsw    = 40,
            .hbp    = 220,
            .vfp    = 5,
            .vsw    = 5,
            .vbp    = 20,
            .hdelay = 3,
            .hoffset = 0
        }
    },
    /*
     * MLC         RESC       HDMI
     * 1920x1080 | 1728x972 | 1920x1080
     */
    {
        .index     = 4,
        .mlc_resol = {
            .width  = 1920,
            .height = 1080
        },
        .resc_resol = {
            .width  = 1728,
            .height = 972
        },
        .hdmi_resol = {
            .width  = 1920,
            .height = 1080
        },
        .dpc_sync_param = {
            .hact   = 1920,
            .vact   = 1080,
            .hfp    = 20,
            .hsw    = 20,
            .hbp    = 20,
            .vfp    = 67,
            .vsw    = 5,
            .vbp    = 98,
            .use_dynamic    = false,
            .dynamic_change_num = 0,
            .dynamic_tbl    = NULL,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 1728,
            .vact   = 972,
            .hfp    = 184,
            .hsw    = 44,
            .hbp    = 244,
            .vfp    = 58,
            .vsw    = 5,
            .vbp    = 90,
            .hdelay = 0,
            .hoffset = 0
        }
    },
    /*
     * MLC         RESC       HDMI
     * 1280x720 | 1066x600 | 1280x720
     */
    {
        .index     = 5,
        .mlc_resol = {
            .width  = 1280,
            .height = 720
        },
        .resc_resol = {
            .width  = 1066,
            .height = 600,
        },
        .hdmi_resol = {
            .width  = 1280,
            .height = 720
        },
        .dpc_sync_param = {
            .hact   = 1280,
            .vact   = 720,
            .hfp    = 32,
            .hsw    = 31,
            .hbp    = 32,
            .vfp    = 85,
            .vsw    = 5,
            .vbp    = 90,
            .use_dynamic    = false,
            .dynamic_change_num = 0,
            .dynamic_tbl    = NULL,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 1066,
            .vact   = 600,
            .hfp    = 217,
            .hsw    = 140,
            .hbp    = 227,
            .vfp    = 65,
            .vsw    = 5,
            .vbp    = 80,
            .hdelay = 0,
            .hoffset = 0
        }
    },
    /*
     * MLC         RESC       HDMI
     * 720x576 | 624x500 | 720x576
     */
    {
        .index     = 6,
        .mlc_resol = {
            .width  = 720,
            .height = 576
        },
        .resc_resol = {
            .width  = 624,
            .height = 500,
        },
        .hdmi_resol = {
            .width  = 720,
            .height = 576
        },
        .dpc_sync_param = {
#if 0
            .hact   = 720,
            .vact   = 576,
            .hfp    = 32,
            .hsw    = 32,
            .hbp    = 32,
            .vfp    = 51,
            .vsw    = 5,
            .vbp    = 88,
#else
            .hact   = 720,
            .vact   = 576,
            .hfp    = 12,
            .hsw    = 64,
            .hbp    = 68,
            .vfp    = 5,
            .vsw    = 5,
            .vbp    = 39,
#endif
            .use_dynamic    = false,
            .dynamic_change_num = 0,
            .dynamic_tbl    = NULL,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
#if 0
            .hact   = 624,
            .vact   = 500,
            .hfp    = 12,
            .hsw    = 64,
            .hbp    = 68,
            .vfp    = 43,
            .vsw    = 5,
            .vbp    = 77,
#else
            .hact   = 720,
            .vact   = 576,
            .hfp    = 12,
            .hsw    = 64,
            .hbp    = 68,
            .vfp    = 5,
            .vsw    = 5,
            .vbp    = 39,
#endif
            .hdelay = 0,
            .hoffset = 0
        }
    },
    /*
     * MLC         RESC       HDMI
     * 720x480 | 720x480 | 720x480
     */
    {
        .index     = 7,
        .mlc_resol = {
            .width  = 720,
            .height = 480
        },
        .resc_resol = {
            .width  = 720,
            .height = 480,
        },
        .hdmi_resol = {
            .width  = 720,
            .height = 480
        },
        .dpc_sync_param = {
            .hact   = 720,
            .vact   = 480,
            .hfp    = 16,
            .hsw    = 62,
            .hbp    = 60,
            .vfp    = 9,
            .vsw    = 6,
            .vbp    = 30,
            .use_dynamic    = false,
            .dynamic_change_num = 0,
            .dynamic_tbl    = NULL,
            .clk_src_lv0 = 4,
            .clk_div_lv0 = 1,
            .clk_src_lv1 = 4,
            .clk_div_lv1 = 1
        },
        .resc_sync_param = {
            .hact   = 720,
            .vact   = 480,
            .hfp    = 16,
            .hsw    = 62,
            .hbp    = 60,
            .vfp    = 9,
            .vsw    = 6,
            .vbp    = 30,
            .hdelay = 0,
            .hoffset = 0
        }
    },
    {
        .index = -1,
    },
};

struct sync_preset *nxp_out_find_sync_preset(uint32_t mlc_width,
        uint32_t mlc_height,
        uint32_t resc_width,
        uint32_t resc_height,
        uint32_t hdmi_width,
        uint32_t hdmi_height)
{
    struct sync_preset *preset = &nxp_out_sync_presets[0];
    while (preset->index != -1) {
        if (preset->mlc_resol.width == mlc_width &&
            preset->mlc_resol.height == mlc_height &&
            preset->resc_resol.width == resc_width &&
            preset->resc_resol.height == resc_height &&
            preset->hdmi_resol.width == hdmi_width &&
            preset->hdmi_resol.height == hdmi_height) {
            return preset;
        }
        preset++;
    }
    return NULL;
}
