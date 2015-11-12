#define DEBUG 1

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/slab.h>
#include <linux/videodev2.h>

#include "nxp-v4l2-common.h"

#include "regs-hdmi.h"
#include "hdmi-priv.h"
#include "nxp-edid.h"

#define EDID_SEGMENT_ADDR   (0x60 >> 1)
#define EDID_ADDR           (0xA0 >> 1)
#define EDID_BLOCK_SIZE     (128)
#define EDID_SEGMENT(x)     ((x) >> 1)
#define EDID_OFFSET(x)      (((x) & 1) * EDID_BLOCK_SIZE)
#define EDID_EXTENSION_FLAG (0x7E)

#define EDID_MAX_I2C_RETRY_CNT (5)
static struct edid_preset {
    u32 preset;
    u16 xres;
    u16 yres;
    u16 refresh;
    char *name;
    bool supported;
} edid_presets[] = {
    { V4L2_DV_480P59_94,  720,  480,  59, "480p@59.94"},
    { V4L2_DV_576P50,     720,  576,  50, "576p@50" },
    { V4L2_DV_720P24,     1280, 720,  24, "720p@24" },
    { V4L2_DV_720P25,     1280, 720,  25, "720p@25" },
    { V4L2_DV_720P30,     1280, 720,  30, "720p@30" },
    { V4L2_DV_720P50,     1280, 720,  50, "720p@50" },
    { V4L2_DV_720P59_94,  1280, 720,  59, "720p@59.94" },
    { V4L2_DV_720P60,     1280, 720,  60, "720p@60" },
    { V4L2_DV_1080P24,    1920, 1080, 24, "1080p@24" },
    { V4L2_DV_1080P25,    1920, 1080, 25, "1080p@25" },
    { V4L2_DV_1080P30,    1920, 1080, 30, "1080p@30" },
    { V4L2_DV_1080P50,    1920, 1080, 50, "1080p@50" },
    { V4L2_DV_1080P60,    1920, 1080, 60, "1080p@60" },
};

/**
 * internal function
 */
static int _edid_i2c_read(struct nxp_edid *me, u8 segment, u8 offset,
        u8 *buf, size_t len)
{
    struct i2c_client *client = me->client;
    int cnt = 0;
    int ret;
    unsigned char start = segment * EDID_BLOCK_SIZE;
    struct i2c_msg msg[] = {
        {
            .addr = EDID_ADDR,
            .flags = 0,
            .len  = 1,
            .buf  = &start,
        },
        {
            .addr = EDID_ADDR,
            .flags = I2C_M_RD,
            .len  = len,
            .buf  = buf,
        }
    };

    pr_debug("%s\n", __func__);
    if (!client) {
        pr_err("%s: No i2c client\n", __func__);
        return -ENODEV;
    }

    do {
        ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
        if (ret == ARRAY_SIZE(msg))
            break;

        pr_err("%s: can't read data, retry %d\n", __func__, cnt);
        msleep(25);
        cnt++;
        // psw0523 test
    } while (cnt < EDID_MAX_I2C_RETRY_CNT);
    /* } while (1); */

    if (cnt == EDID_MAX_I2C_RETRY_CNT) {
        pr_err("%s: read timeout\n", __func__);
        return -ETIMEDOUT;
    }

    return 0;
}

static int _edid_read_block(struct nxp_edid *me, int block, u8 *buf, size_t len)
{
    int ret, i;
    u8 segment = EDID_SEGMENT(block);
    u8 offset = EDID_OFFSET(block);
    u8 sum = 0;

    if (len < EDID_BLOCK_SIZE)
        return -EINVAL;

    ret = _edid_i2c_read(me, segment, offset, buf, EDID_BLOCK_SIZE);
    if (ret)
        return ret;

    for (i = 0; i < EDID_BLOCK_SIZE; i++)
        sum += buf[i];

    if (sum) {
        pr_err("%s: checksum error block=%d sum=%d\n", __func__,
                block, sum);
        return -EPROTO;
    }

    pr_debug("%s: sum 0x%x\n", __func__, sum);

    return 0;
}

static int _edid_read(struct nxp_edid *me, u8 **data)
{
    u8 block0[EDID_BLOCK_SIZE];
    u8 *edid;
    int block = 0;
    int block_cnt, ret;

    ret = _edid_read_block(me, 0, block0, sizeof(block0));
    if (ret)
        return ret;

    block_cnt = block0[EDID_EXTENSION_FLAG] + 1;

    edid = kmalloc(block_cnt * EDID_BLOCK_SIZE, GFP_KERNEL);
    if (!edid) {
        pr_err("%s: failed to allocate edid\n", __func__);
        return -ENOMEM;
    }

    memcpy(edid, block0, sizeof(block0));

    while (++block < block_cnt) {
        ret = _edid_read_block(me, block,
                edid + block * EDID_BLOCK_SIZE,
                EDID_BLOCK_SIZE);
        if (ret) {
            kfree(edid);
            return ret;
        }
    }

    *data = edid;
    return block_cnt;
}

static struct edid_preset *_edid_find_preset(struct fb_videomode *mode)
{
    struct edid_preset *preset = edid_presets;
    int i;

    if (mode->vmode & FB_VMODE_INTERLACED)
        return NULL;

    for (i = 0; i < ARRAY_SIZE(edid_presets); i++, preset++) {
        if (mode->refresh == preset->refresh &&
            mode->xres    == preset->xres &&
            mode->yres    == preset->yres) {
            return preset;
        }
    }

    return NULL;
}

static void _edid_use_default_preset(struct nxp_edid *me)
{
    int i;
    me->preset = HDMI_DEFAULT_PRESET;
    for (i = 0; i < ARRAY_SIZE(edid_presets); i++) {
        edid_presets[i].supported =
            (edid_presets[i].preset == me->preset);
    }
    me->max_audio_ch = 2;
}

/**
 * member functions
 */
static int nxp_edid_update(struct nxp_edid *me)
{
    struct fb_monspecs specs;
    struct edid_preset *preset;
    bool first = true;
    u8 *edid = NULL;
    int channels_max = 0;
    int ret = 0;
    int i;

    pr_debug("%s\n", __func__);

    me->edid_misc = 0;

    ret = _edid_read(me, &edid);
    if (ret < 0)
        goto out;

    print_hex_dump_bytes("EDID: ", DUMP_PREFIX_OFFSET, edid,
            ret * EDID_BLOCK_SIZE);

    fb_edid_to_monspecs(edid, &specs);
    for (i = 0; i < ret; i++)
        fb_edid_add_monspecs(edid + i * EDID_BLOCK_SIZE, &specs);

    me->preset = V4L2_DV_INVALID;
    for (i = 0; i < ARRAY_SIZE(edid_presets); i++)
        edid_presets[i].supported = false;

    for (i = 0; i < specs.modedb_len; i++) {
        preset = _edid_find_preset(&specs.modedb[i]);
        if (preset) {
            pr_debug("%s: EDID found %s\n", __func__, preset->name);
            preset->supported = true;
            if (first) {
                me->preset = preset->preset;
                first = false;
            }
        }
    }

    me->edid_misc = specs.misc;
    pr_debug("%s: EDID misc flags 0x%x)\n", __func__, me->edid_misc);

    for (i = 0; i < specs.audiodb_len; i++) {
        if (specs.audiodb[i].format != FB_AUDIO_LPCM)
            continue;
        if (specs.audiodb[i].channel_count > channels_max)
            channels_max = specs.audiodb[i].channel_count;
    }

    if (me->edid_misc & FB_MISC_HDMI) {
        if (channels_max)
            me->max_audio_ch = channels_max;
        else
            me->max_audio_ch = 2;
    } else {
        me->max_audio_ch = 2;
    }
    pr_debug("%s: EDID Audio channels %d\n", __func__, me->max_audio_ch);

out:
    if (first)
        _edid_use_default_preset(me);

    kfree(edid);
    return ret;
}

static u32 nxp_edid_enum_presets(struct nxp_edid *me, int index)
{
    int i, j = 0;

    pr_debug("%s\n", __func__);

    for (i = 0; i < ARRAY_SIZE(edid_presets); i++) {
        if (edid_presets[i].supported) {
            if (j++ == index)
                return edid_presets[i].preset;
        }
    }

    return V4L2_DV_INVALID;
}

static u32 nxp_edid_preferred_preset(struct nxp_edid *me)
{
    pr_debug("%s\n", __func__);
    return me->preset;
}

static bool nxp_edid_supports_hdmi(struct nxp_edid *me)
{
    pr_debug("%s\n", __func__);
    return me->edid_misc & FB_MISC_HDMI;
}

static int nxp_edid_max_audio_channels(struct nxp_edid *me)
{
    pr_debug("%s\n", __func__);
    return me->max_audio_ch;
}

static int nxp_edid_suspend(struct nxp_edid *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_edid_resume(struct nxp_edid *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

/**
 * i2c driver
 */
static int __devinit _edid_i2c_probe(struct i2c_client *client,
        const struct i2c_device_id *dev_id)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int _edid_i2c_remove(struct i2c_client *client)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static struct i2c_device_id _edid_idtable[] = {
    {"nxp_edid", 0},
};
MODULE_DEVICE_TABLE(i2c, _edid_idtable);

static struct i2c_driver _edid_driver = {
    .driver = {
        .name  = "nxp_edid",
        .owner = THIS_MODULE,
    },
    .id_table  = _edid_idtable,
    .probe     = _edid_i2c_probe,
    .remove    = _edid_i2c_remove,
};

/**
 * publi api
 */
int nxp_edid_init(struct nxp_edid *me, struct nxp_v4l2_i2c_board_info *i2c_info)
{
    int ret = 0;

    pr_debug("%s\n", __func__);

    memset(me, 0, sizeof(struct nxp_edid));

    ret = i2c_add_driver(&_edid_driver);
    if (ret < 0) {
        pr_err("%s: failed to i2c_add_driver()\n", __func__);
        return -EINVAL;
    }

    me->client = nxp_v4l2_get_i2c_client(i2c_info);
    if (!me->client) {
        pr_err("%s: can't find edid i2c device\n", __func__);
        return -EINVAL;
    }

    me->update = nxp_edid_update;
    me->enum_presets = nxp_edid_enum_presets;
    me->preferred_preset = nxp_edid_preferred_preset;
    me->supports_hdmi = nxp_edid_supports_hdmi;
    me->max_audio_channels = nxp_edid_max_audio_channels;
    me->suspend = nxp_edid_suspend;
    me->resume = nxp_edid_resume;

    _edid_use_default_preset(me);

    pr_debug("%s: success!!!\n", __func__);
    return 0;
}

void nxp_edid_cleanup(struct nxp_edid *me)
{
    pr_debug("%s\n", __func__);

    i2c_del_driver(&_edid_driver);

    if (me->client) {
        i2c_unregister_device(me->client);
        me->client = NULL;
    }
}

