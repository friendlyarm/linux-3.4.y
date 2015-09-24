#ifndef _HDMI_PRESET_H
#define _HDMI_PRESET_H

#include <linux/kernel.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-device.h>

struct hdmi_tg_regs {
    u8 cmd;
    u8 h_fsz_l;
    u8 h_fsz_h;
    u8 hact_st_l;
    u8 hact_st_h;
    u8 hact_sz_l;
    u8 hact_sz_h;
    u8 v_fsz_l;
    u8 v_fsz_h;
    u8 vsync_l;
    u8 vsync_h;
    u8 vsync2_l;
    u8 vsync2_h;
    u8 vact_st_l;
    u8 vact_st_h;
    u8 vact_sz_l;
    u8 vact_sz_h;
    u8 field_chg_l;
    u8 field_chg_h;
    u8 vact_st2_l;
    u8 vact_st2_h;
    u8 vact_st3_l;
    u8 vact_st3_h;
    u8 vact_st4_l;
    u8 vact_st4_h;
    u8 vsync_top_hdmi_l;
    u8 vsync_top_hdmi_h;
    u8 vsync_bot_hdmi_l;
    u8 vsync_bot_hdmi_h;
    u8 field_top_hdmi_l;
    u8 field_top_hdmi_h;
    u8 field_bot_hdmi_l;
    u8 field_bot_hdmi_h;
    u8 tg_3d;
};

struct hdmi_core_regs {
    u8 h_blank[2];
    u8 v2_blank[2];
    u8 v1_blank[2];
    u8 v_line[2];
    u8 h_line[2];
    u8 hsync_pol[1];
    u8 vsync_pol[1];
    u8 int_pro_mode[1];
    u8 v_blank_f0[2];
    u8 v_blank_f1[2];
    u8 h_sync_start[2];
    u8 h_sync_end[2];
    u8 v_sync_line_bef_2[2];
    u8 v_sync_line_bef_1[2];
    u8 v_sync_line_aft_2[2];
    u8 v_sync_line_aft_1[2];
    u8 v_sync_line_aft_pxl_2[2];
    u8 v_sync_line_aft_pxl_1[2];
    u8 v_blank_f2[2]; /* for 3D mode */
    u8 v_blank_f3[2]; /* for 3D mode */
    u8 v_blank_f4[2]; /* for 3D mode */
    u8 v_blank_f5[2]; /* for 3D mode */
    u8 v_sync_line_aft_3[2];
    u8 v_sync_line_aft_4[2];
    u8 v_sync_line_aft_5[2];
    u8 v_sync_line_aft_6[2];
    u8 v_sync_line_aft_pxl_3[2];
    u8 v_sync_line_aft_pxl_4[2];
    u8 v_sync_line_aft_pxl_5[2];
    u8 v_sync_line_aft_pxl_6[2];
    u8 vact_space_1[2];
    u8 vact_space_2[2];
    u8 vact_space_3[2];
    u8 vact_space_4[2];
    u8 vact_space_5[2];
    u8 vact_space_6[2];
};

struct hdmi_conf {
    u32 preset;
    const struct hdmi_preset_conf *conf;
    const struct hdmi_3d_info *info;
};

struct hdmi_preset_conf {
	struct hdmi_core_regs core;
	struct hdmi_tg_regs tg;
	struct v4l2_mbus_framefmt mbus_fmt;
	u8 vic;
};

extern const struct hdmi_conf hdmi_conf[];
extern const int hdmi_pre_cnt;

#endif
