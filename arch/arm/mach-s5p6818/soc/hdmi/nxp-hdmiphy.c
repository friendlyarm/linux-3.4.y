#define DEBUG 1

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <linux/delay.h>

/* for prototype */
#include <nx_hdmi.h>

#include <mach/hdmi/regs-hdmi.h>
#include <mach/hdmi/hdmi-priv.h>
#include <mach/hdmi/nxp-hdmiphy.h>

#include <nx_rstcon.h>

#define NXP_HDMIPHY_PRESET_TABLE_SIZE   30

/**
 * preset tables : TODO
 */
static const u8 hdmiphy_preset25_2[32] = {
	0x52, 0x3f, 0x55, 0x40, 0x01, 0x00, 0xc8, 0x82,
    0xc8, 0xbd, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x06,
    0x80, 0x01, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xf4, 0x24, 0x00, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset25_175[32] = {
    0xd1, 0x1f, 0x50, 0x40, 0x20, 0x1e, 0xc8, 0x81,
    0xe8, 0xbd, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x06,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xf4, 0x24, 0x00, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset27[32] = {
    0xd1, 0x22, 0x51, 0x40, 0x08, 0xfc, 0xe0, 0x98,
    0xe8, 0xcb, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x06,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xe4, 0x24, 0x00, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset27_027[32] = {
    0xd1, 0x2d, 0x72, 0x40, 0x64, 0x12, 0xc8, 0x43,
    0xe8, 0x0e, 0xd9, 0x45, 0xa0, 0xac, 0x80, 0x06,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xe3, 0x24, 0x00, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset54[32] = {
    0x54, 0x2d, 0x35, 0x40, 0x01, 0x00, 0xc8, 0x82,
    0xc8, 0x0e, 0xd9, 0x45, 0xa0, 0xac, 0x80, 0x06,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xe4, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset54_054[32] = {
    0xd1, 0x2d, 0x32, 0x40, 0x64, 0x12, 0xc8, 0x43,
    0xe8, 0x0e, 0xd9, 0x45, 0xa0, 0xac, 0x80, 0x06,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xe3, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset74_175[32] = {
    0xd1, 0x1f, 0x10, 0x40, 0x5b, 0xef, 0xc8, 0x81,
    0xe8, 0xb9, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x56,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xa6, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset74_25[32] = {
    0xd1, 0x1f, 0x10, 0x40, 0x40, 0xf8, 0xc8, 0x81,
    0xe8, 0xba, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x56,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0xa5, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset148_352[32] = {
    0xd1, 0x1f, 0x00, 0x40, 0x5b, 0xef, 0xc8, 0x81,
    0xe8, 0xb9, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x66,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0x4b, 0x25, 0x03, 0x00, 0x00, 0x01, 0x80, 0x10,
};

static const u8 hdmiphy_preset148_5[32] = {
    0xd1, 0x1f, 0x00, 0x40, 0x40, 0xf8, 0xc8, 0x81,
    0xe8, 0xba, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x6b,
    0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
    0x4b, 0x25, 0x03, 0x00, 0x00, 0x01, 0x80, 0x10,
};

const struct hdmiphy_preset hdmiphy_preset[] = {
	{ V4L2_DV_480P59_94, hdmiphy_preset27 },
	{ V4L2_DV_480P60, hdmiphy_preset27_027 },
	{ V4L2_DV_576P50, hdmiphy_preset27 },
	{ V4L2_DV_720P50, hdmiphy_preset74_25 },
	{ V4L2_DV_720P59_94, hdmiphy_preset74_175 },
	{ V4L2_DV_720P60, hdmiphy_preset74_25 },
	{ V4L2_DV_1080P50, hdmiphy_preset148_5 },
	{ V4L2_DV_1080P59_94, hdmiphy_preset148_352 },
	{ V4L2_DV_1080P60, hdmiphy_preset148_5 },
};

const int hdmiphy_preset_cnt = ARRAY_SIZE(hdmiphy_preset);

static const u8 *_hdmiphy_preset2conf(u32 preset)
{
    int i;
    for (i = 0; i < hdmiphy_preset_cnt; i++) {
        if (hdmiphy_preset[i].preset == preset)
            return hdmiphy_preset[i].data;
    }
    return NULL;
}

/* TODO */
static int _hdmiphy_enable_pad(struct nxp_hdmiphy *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

/* TODO */
static int _hdmiphy_reset(struct nxp_hdmiphy *me)
{
    pr_debug("%s\n", __func__);
#if defined (CONFIG_ARCH_NXP4330)
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nDISABLE);
#elif defined (CONFIG_ARCH_S5P6818)
	NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_ASSERT);
	mdelay(1);
	NX_RSTCON_SetRST(NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_NEGATE);
#endif
    return 0;
}

/* TODO */
static int _hdmiphy_clk_enable(struct nxp_hdmiphy *me, int enable)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int _hdmiphy_reg_set(struct nxp_hdmiphy *me,
        const u8 *data, size_t size)
{
    int i;
    u32 reg_addr;
    pr_debug("%s\n", __func__);

    // for s5p6818
    NX_HDMI_SetReg(0, HDMI_PHY_Reg7C, (0<<7)); //NX_HDMI_SetReg(0, HDMI_PHY_Reg7C, (0<<7));
    NX_HDMI_SetReg(0, HDMI_PHY_Reg04, (0<<4)); //NX_HDMI_SetReg(0, HDMI_PHY_Reg04, (0<<4));
    NX_HDMI_SetReg(0, HDMI_PHY_Reg24, (1<<7)); //NX_HDMI_SetReg(0, HDMI_PHY_Reg24, (1<<7));

    for (i = 0, reg_addr = HDMI_PHY_Reg04; i < size; i++, reg_addr += 4) {
        NX_HDMI_SetReg(0, reg_addr, data[i]);
        // for s5p6818
        /*NX_HDMI_SetReg(0, reg_addr, data[i]);*/
        /*printk("reg 0x%x: write 0x%x, read 0x%x\n", reg_addr, data[i], NX_HDMI_GetReg(0, reg_addr));*/
    }
    NX_HDMI_SetReg(0, HDMI_PHY_Reg7C, 0x80); //NX_HDMI_SetReg(0, HDMI_PHY_Reg7C, 0x80);
    NX_HDMI_SetReg(0, HDMI_PHY_Reg7C, (1<<7)); //NX_HDMI_SetReg(0, HDMI_PHY_Reg7C, (1<<7));
    return 0;
}

/**
 * member function
 */
static int nxp_hdmiphy_s_power(struct nxp_hdmiphy *me, int on)
{
    //int ret;

    pr_debug("%s: %d\n", __func__, on);

    if (on) {
        _hdmiphy_enable_pad(me);
        _hdmiphy_reset(me);
    }
    _hdmiphy_clk_enable(me, on);

    return 0;
}

static int nxp_hdmiphy_s_dv_preset(struct nxp_hdmiphy *me,
        struct v4l2_dv_preset *preset)
{
    const u8 *data;

    pr_debug("%s: preset(%d)\n", __func__, preset->preset);

    data = _hdmiphy_preset2conf(preset->preset);;
    if (!data) {
        pr_err("%s: can't find preset\n", __func__);
        return -EINVAL;
    }

    me->preset = (u8 *)data;

    return 0;
}

static int nxp_hdmiphy_s_stream(struct nxp_hdmiphy *me, int enable)
{
    int ret;
    pr_debug("%s: %d\n", __func__, enable);

    if (enable) {
        ret = _hdmiphy_reg_set(me, me->preset, NXP_HDMIPHY_PRESET_TABLE_SIZE);
        if (ret < 0) {
            pr_err("%s: failed to _hdmiphy_reg_set()\n", __func__);
            return ret;
        }
    } else {
        _hdmiphy_reset(me);
    }

    return 0;
}

static int nxp_hdmiphy_suspend(struct nxp_hdmiphy *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_hdmiphy_resume(struct nxp_hdmiphy *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

/**
 * public api
 */
int nxp_hdmiphy_init(struct nxp_hdmiphy *me)
{
    int ret = 0;

    pr_debug("%s\n", __func__);

    me->s_power = nxp_hdmiphy_s_power;
    me->s_dv_preset = nxp_hdmiphy_s_dv_preset;
    me->s_stream = nxp_hdmiphy_s_stream;
    me->suspend = nxp_hdmiphy_suspend;
    me->resume = nxp_hdmiphy_resume;

    return ret;
}

void nxp_hdmiphy_cleanup(struct nxp_hdmiphy *me)
{
    pr_debug("%s\n", __func__);
}
