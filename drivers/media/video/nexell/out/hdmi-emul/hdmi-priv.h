#ifndef _HDMI_PRIV_H
#define _HDMI_PRIV_H

#include <linux/io.h>

/**
 * defines
 */
#define INFOFRAME_CNT          2

#define HDMI_VSI_VERSION	0x01
#define HDMI_AVI_VERSION	0x02
#define HDMI_AUI_VERSION	0x01
#define HDMI_VSI_LENGTH		0x05
#define HDMI_AVI_LENGTH		0x0d
#define HDMI_AUI_LENGTH		0x0a

#define AVI_UNDERSCAN			(2 << 0)
#define AVI_ACTIVE_FORMAT_VALID		(1 << 4)
#define AVI_PIC_ASPECT_RATIO_4_3	(1 << 4)
#define AVI_PIC_ASPECT_RATIO_16_9	(2 << 4)
#define AVI_SAME_AS_PIC_ASPECT_RATIO	8
#define AVI_LIMITED_RANGE		(1 << 2)
#define AVI_FULL_RANGE			(2 << 2)
#define AVI_ITU709			(2 << 6)

#define HDMI_DEFAULT_PRESET V4L2_DV_720P60

enum HDMI_VIDEO_FORMAT {
	HDMI_VIDEO_FORMAT_2D = 0x0,
	/** refer to Table 8-12 HDMI_Video_Format in HDMI specification v1.4a */
	HDMI_VIDEO_FORMAT_3D = 0x2
};

enum HDMI_3D_FORMAT {
	/** refer to Table 8-13 3D_Structure in HDMI specification v1.4a */

	/** Frame Packing */
	HDMI_3D_FORMAT_FP = 0x0,
	/** Top-and-Bottom */
	HDMI_3D_FORMAT_TB = 0x6,
	/** Side-by-Side Half */
	HDMI_3D_FORMAT_SB_HALF = 0x8
};

enum HDMI_3D_EXT_DATA {
	/* refer to Table H-3 3D_Ext_Data - Additional video format
	 * information for Side-by-side(half) 3D structure */

	/** Horizontal sub-sampleing */
	HDMI_H_SUB_SAMPLE = 0x1
};

enum HDMI_OUTPUT_FMT {
	HDMI_OUTPUT_RGB888 = 0x0,
	HDMI_OUTPUT_YUV444 = 0x2
};

enum HDMI_PACKET_TYPE {
	/** refer to Table 5-8 Packet Type in HDMI specification v1.4a */

	/** InfoFrame packet type */
	HDMI_PACKET_TYPE_INFOFRAME = 0X80,
	/** Vendor-Specific InfoFrame */
	HDMI_PACKET_TYPE_VSI = HDMI_PACKET_TYPE_INFOFRAME + 1,
	/** Auxiliary Video information InfoFrame */
	HDMI_PACKET_TYPE_AVI = HDMI_PACKET_TYPE_INFOFRAME + 2,
	/** Audio information InfoFrame */
	HDMI_PACKET_TYPE_AUI = HDMI_PACKET_TYPE_INFOFRAME + 4
};

enum HDMI_AUDIO_CODEC {
	HDMI_AUDIO_PCM,
	HDMI_AUDIO_AC3,
	HDMI_AUDIO_MP3
};

enum HDMI_ASPECT_RATIO {
	HDMI_ASPECT_RATIO_16_9,
	HDMI_ASPECT_RATIO_4_3
};

struct hdmi_preset_conf;
struct hdmi_infoframe;
struct hdmi_3d_info;

struct hdmi_3d_info {
    u32 is_3d; /* enum HDMI_VIDEO_FORMAT */
    u32 fmt_3d; /* enum HDMI_3D_FORMAT */
};

struct hdmi_infoframe {
    u32 type; /* enum HDMI_PACKET_TYPE */
    u32 ver;
    u32 len;
};

extern void __iomem* hdmi_priv_io_base;

static inline 
void hdmi_set_base(void __iomem* base)
{
    hdmi_priv_io_base = base;
}

static inline
void hdmi_write(u32 reg, u32 val)
{
    writel(val, hdmi_priv_io_base + reg);
}

static inline
void hdmi_write_mask(u32 reg, u32 val, u32 mask)
{
    u32 old = readl(hdmi_priv_io_base + reg);
    val = (val & mask) | (old & ~mask);
    writel(val, hdmi_priv_io_base + reg);
}

static inline
void hdmi_writeb(u32 reg, u8 val)
{
    writeb(val, hdmi_priv_io_base + reg);
}

static inline
void hdmi_write_bytes(u32 reg, u8 *buf, int bytes)
{
    int i;
    for (i = 0; i < bytes; ++i) 
        writeb(buf[i], hdmi_priv_io_base + reg + i * 4);
}

static inline
u32 hdmi_read(u32 reg)
{
    return readl(hdmi_priv_io_base + reg);
}

static inline
u8 hdmi_readb(u32 reg)
{
    return readb(hdmi_priv_io_base + reg);
}

static inline
void hdmi_read_bytes(u32 reg, u8 *buf, int bytes)
{
    int i;
    for (i = 0; i < bytes; ++i) 
        buf[i] = readb(hdmi_priv_io_base + reg + i * 4);
}

void hdmi_set_dvi_mode(bool is_dvi);
void hdmi_conf_apply(int color_range, const struct hdmi_preset_conf *conf);
bool hdmi_is_phy_ready(void);
void hdmi_enable(bool on);
void hdmi_hpd_enable(bool on);
void hdmi_tg_enable(bool on, bool is_interlacee);
void hdmi_stop_vsi(void);
u8   hdmi_chksum(u32 start, u8 len, u32 hdr_sum);
void hdmi_set_acr(int sample_rate, bool is_dvi);
void hdmi_audio_spdif_init(int audio_codec, int bps);
void hdmi_audio_i2s_init(int sample_rate, int bps);
void hdmi_audio_enable(bool on);
void hdmi_bluescreen_enable(bool on);
void hdmi_mute(bool on);
int  hdmi_hpd_status(void);
u8   hdmi_get_int_mask(void);
void hdmi_set_int_mask(u8 mask, bool on);
void hdmi_sw_hpd_enable(bool on);
void hdmi_sw_hpd_plug(bool on);
void hdmi_phy_sw_reset(void);
void hdmi_sw_reset(void);
void hdmi_dump_regs(const char *prefix);

#endif
