#define DEBUG 1

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#include <mach/nxp-v4l2-platformdata.h>

#include "nxp-v4l2-common.h"

#include <mach/hdmi/regs-hdmi.h>
#include <mach/hdmi/hdmi-priv.h>
#include <mach/hdmi/nxp-hdcp.h>

/**
 * defines
 */
#define AN_SIZE                     8
#define AKSV_SIZE                   5
#define BKSV_SIZE                   5
#define MAX_KEY_SIZE                16

#define BKSV_RETRY_CNT              14
#define BKSV_DELAY                  100

#define DDC_RETRY_CNT               400000
#define DDC_DELAY                   25

#define KEY_LOAD_RETRY_CNT          1000
#define ENCRYPT_CHECK_CNT           10

#define KSV_FIFO_RETRY_CNT          50
#define KSV_FIFO_CHK_DELAY          100 /* ms */
#define KSV_LIST_RETRY_CNT          10000

#define BCAPS_SIZE                  1
#define BSTATUS_SIZE                2
#define SHA_1_HASH_SIZE             20
#define HDCP_MAX_DEVS               128
#define HDCP_KSV_SIZE               5

/* offset of HDCP port */
#define HDCP_BKSV                   0x00
#define HDCP_RI                     0x08
#define HDCP_AKSV                   0x10
#define HDCP_AN                     0x18
#define HDCP_SHA1                   0x20
#define HDCP_BCAPS                  0x40
#define HDCP_BSTATUS                0x41
#define HDCP_KSVFIFO                0x43

#define KSV_FIFO_READY              (0x1 << 5)

#define MAX_CASCADE_EXCEEDED_ERROR  (-2)
#define MAX_DEVS_EXCEEDED_ERROR     (-3)
#define REPEATER_ILLEGAL_DEVICE_ERROR   (-4)
#define REPEATER_TIMEOUT_ERROR      (-5)

#define MAX_CASCADE_EXCEEDED        (0x1 << 3)
#define MAX_DEVS_EXCEEDED           (0x1 << 7)

#define nxp_hdcp_to_parent(me)       \
           container_of(me, struct nxp_hdmi, hdcp)
/* TODO */
#if 0
#define IS_HDMI_RUNNING(me) \
            nxp_hdcp_to_parent(me)->is_streaming(nxp_hdcp_to_parent(me))
#else
#define IS_HDMI_RUNNING(me) hdmi_hpd_status()
#endif

/**
 * internal functions
 */
static int
_hdcp_i2c_read(struct nxp_hdcp *me, u8 offset, int bytes, u8 *buf)
{
    struct i2c_client *client = me->client;
    int ret, cnt = 0;

    struct i2c_msg msg[] = {
        [0] = {
            .addr  = client->addr,
            .flags = 0,
            .len   = 1,
            .buf   = &offset
        },
        [1] = {
            .addr  = client->addr,
            .flags = I2C_M_RD,
            .len   = bytes,
            .buf   = buf
        }
    };

    do {
        if (!IS_HDMI_RUNNING(me))
            return -ENODEV;

        ret = i2c_transfer(client->adapter, msg, 2);

        if (ret < 0 || ret != 2)
            pr_err("%s: can't read data, retry %d\n", __func__, cnt);
        else
            break;

        if (me->auth_state == FIRST_AUTHENTICATION_DONE ||
            me->auth_state == SECOND_AUTHENTICATION_DONE)
            goto ddc_read_err;

        msleep(DDC_DELAY);
        cnt++;
    } while (cnt < DDC_RETRY_CNT);

    if (cnt == DDC_RETRY_CNT)
        goto ddc_read_err;

    printk("%s: read data ok\n", __func__);

    return 0;

ddc_read_err:
    pr_err("%s: can't read data, timeout\n", __func__);
    return -ETIMEDOUT;
}

static int _hdcp_i2c_write(struct nxp_hdcp *me, u8 offset, int bytes, u8 *buf)
{
    struct i2c_client *client = me->client;
    u8 msg[bytes + 1];
    int ret, cnt = 0;

    msg[0] = offset;
    memcpy(&msg[1], buf, bytes);

    do {
        if (!IS_HDMI_RUNNING(me))
            return -ENODEV;

        ret = i2c_master_send(client, msg, bytes + 1);

        if (ret < 0 || ret < bytes + 1)
            pr_err("%s: can't write data, retry %d\n", __func__, cnt);
        else
            break;

        msleep(DDC_DELAY);
        cnt++;
    } while (cnt < DDC_RETRY_CNT);

    if (cnt == DDC_RETRY_CNT)
        goto ddc_write_err;

    printk("%s: write data ok\n", __func__);
    return 0;

ddc_write_err:
    pr_err("%s: can't write data, timeout\n", __func__);
    return -ETIMEDOUT;
}

static void _hdcp_encryption(struct nxp_hdcp *me, bool on)
{
    if (on)
        hdmi_write_mask(HDMI_ENC_EN, ~0, HDMI_HDCP_ENC_ENABLE);
    else
        hdmi_write_mask(HDMI_ENC_EN, 0, HDMI_HDCP_ENC_ENABLE);

    hdmi_mute(!on);
}

static int _hdcp_write_key(struct nxp_hdcp *me, int size, int reg, int offset)
{
    u8 buf[MAX_KEY_SIZE];
    int cnt, zero = 0;
    int i;

    memset(buf, 0, sizeof(buf));
    hdmi_read_bytes(reg, buf, size);

    for (cnt = 0; cnt < size; cnt++)
        if (buf[cnt] == 0)
            zero++;

    if (zero == size) {
        pr_err("%s: %s is null\n", __func__,
                offset == HDCP_AN ? "An" : "Aksv");
        goto write_key_err;
    }

    if (_hdcp_i2c_write(me, offset, size, buf) < 0) {
        pr_err("%s: failed to i2c write\n", __func__);
        goto write_key_err;
    }

#ifdef DEBUG
    for (i = 0; i < size + 1; i++) {
        printk("%s: %s[%d] : 0x%02x\n", __func__,
                offset == HDCP_AN ? "An" : "Aksv", i, buf[i]);
    }
#endif

    return 0;

write_key_err:
    return -1;
}

static int _hdcp_read_bcaps(struct nxp_hdcp *me)
{
    u8 bcaps = 0;

    if (_hdcp_i2c_read(me, HDCP_BCAPS, BCAPS_SIZE, &bcaps) < 0) {
        pr_err("%s: failed to i2c read\n", __func__);
        return -ETIMEDOUT;
    }

    if (!IS_HDMI_RUNNING(me)) {
        pr_err("%s: hdmi is not streaming!!!\n", __func__);
        return -ENODEV;
    }

    printk("%s: bcaps 0x%x\n", __func__, bcaps);
    hdmi_writeb(HDMI_HDCP_BCAPS, bcaps);

    if (bcaps & HDMI_HDCP_BCAPS_REPEATER)
        me->is_repeater = true;
    else
        me->is_repeater = false;

    printk("%s: device is %s\n", __func__,
            me->is_repeater ? "REPEAT" : "SINK");
    printk("%s: [i2c] bcaps : 0x%02x\n", __func__, bcaps);

    return 0;
}

static int _hdcp_read_bksv(struct nxp_hdcp *me)
{
    u8 bksv[BKSV_SIZE];
    int i, j;
    u32 one = 0, zero = 0, result = 0;
    u32 cnt = 0;

    memset(bksv, 0, sizeof(bksv));

    do {
        if (_hdcp_i2c_read(me, HDCP_BKSV, BKSV_SIZE, bksv) < 0) {
            pr_err("%s: failed to i2c read\n", __func__);
            return -ETIMEDOUT;
        }

#ifdef DEBUG
        for (i = 0; i < BKSV_SIZE; i++) {
            printk("%s: i2c read: bksv[%d]: 0x%x\n",
                    __func__, i, bksv[i]);
        }
#endif

        for (i = 0; i < BKSV_SIZE; i++) {
            for (j = 0; j < 8; j++) {
                result = bksv[i] & (0x1 << j);
                if (result)
                    one++;
                else
                    zero++;
            }
        }

        if (!IS_HDMI_RUNNING(me)) {
            pr_err("%s: hdmi is not running\n", __func__);
            return -ENODEV;
        }

#if 0
        if ((zero == 20) && (one == 20)) {
            hdmi_write_bytes(HDMI_HDCP_BKSV_(0), bksv, BKSV_SIZE);
            break;
        }
#else
        {
            u8 bksvs[BKSV_SIZE] = {0xcd, 0x1a, 0xf2, 0x1e, 0x51};
            hdmi_write_bytes(HDMI_HDCP_BKSV_(0), bksvs, BKSV_SIZE);
            break;
        }
#endif

        printk("%s: invalid bksv, retry : %d\n", __func__, cnt);

        msleep(BKSV_DELAY);
        cnt++;
    } while (cnt < BKSV_RETRY_CNT);

    if (cnt == BKSV_RETRY_CNT) {
        pr_err("%s: read timeout\n", __func__);
        return -ETIMEDOUT;
    }

    printk("%s: bksv read OK, retry : %d\n", __func__, cnt);
    return 0;
}

static int _hdcp_read_ri(struct nxp_hdcp *me)
{
    u8 ri[2] = {0, 0};
    u8 rj[2] = {0, 0};

    ri[0] = hdmi_readb(HDMI_HDCP_RI_0);
    ri[1] = hdmi_readb(HDMI_HDCP_RI_1);

    if (_hdcp_i2c_read(me, HDCP_RI, 2, rj) < 0) {
        pr_err("%s: failed to i2c read\n", __func__);
        return -ETIMEDOUT;
    }

    printk("%s: Rx -> rj[0]: 0x%02x, rj[1]: 0x%02x\n", __func__,
            rj[0], rj[1]);
    printk("%s: Tx -> ri[0]: 0x%02x, ri[1]: 0x%02x\n", __func__,
            ri[0], ri[1]);

    if ((ri[0] == rj[0]) && (ri[1] == rj[1]) && (ri[0] | ri[1]))
        hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_RI_MATCH_RESULT_Y);
    else {
        hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_RI_MATCH_RESULT_N);
        pr_err("%s: compare error\n", __func__);
        goto compare_err;
    }

    printk("%s: ri and rj are matched\n", __func__);
    return 0;

compare_err:
    me->event = HDCP_EVENT_STOP;
    me->auth_state = NOT_AUTHENTICATED;
    msleep(10);
    return -EINVAL;
}

static void _hdcp_sw_reset(struct nxp_hdcp *me)
{
#if 0
    u32 val = hdmi_get_int_mask();

    hdmi_set_int_mask(HDMI_INTC_EN_HPD_PLUG, false);
    hdmi_set_int_mask(HDMI_INTC_EN_HPD_UNPLUG, false);

    hdmi_sw_hpd_enable(true);
    hdmi_sw_hpd_plug(false);
    hdmi_sw_hpd_plug(true);
    hdmi_sw_hpd_enable(false);

    if (val & HDMI_INTC_EN_HPD_PLUG)
        hdmi_set_int_mask(HDMI_INTC_EN_HPD_PLUG, true);
    if (val & HDMI_INTC_EN_HPD_UNPLUG)
        hdmi_set_int_mask(HDMI_INTC_EN_HPD_UNPLUG, true);
#endif
}

static int _hdcp_reset_auth(struct nxp_hdcp *me)
{
    u32 val;

    if (!IS_HDMI_RUNNING(me))
        return -ENODEV;

    mutex_lock(&me->mutex);

    me->event      = HDCP_EVENT_STOP;
    me->auth_state = NOT_AUTHENTICATED;

    hdmi_write(HDMI_HDCP_CTRL1, 0x0);
    hdmi_write(HDMI_HDCP_CTRL2, 0x0);
    hdmi_mute(true);

    _hdcp_encryption(me, false);

    printk("%s: reset authentication\n", __func__);

    val = HDMI_UPDATE_RI_INT_EN | HDMI_WRITE_INT_EN |
        HDMI_WATCHDOG_INT_EN | HDMI_WTFORACTIVERX_INT_EN;
    hdmi_write_mask(HDMI_STATUS_EN, 0, val);

    hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_CLR_ALL_RESULTS);

    /* delat 1 frame */
    msleep(16);

    _hdcp_sw_reset(me);

    val = HDMI_UPDATE_RI_INT_EN | HDMI_WRITE_INT_EN |
        HDMI_WATCHDOG_INT_EN | HDMI_WTFORACTIVERX_INT_EN;
    hdmi_write_mask(HDMI_STATUS_EN, ~0, val);
    hdmi_write_mask(HDMI_HDCP_CTRL1, ~0, HDMI_HDCP_CP_DESIRED_EN);

    mutex_unlock(&me->mutex);

    return 0;
}

// this is test
#define HDCP_KEY_SIZE 288

// golden key
static const unsigned char _hdcp_golden_key[HDCP_KEY_SIZE] = {
    0x14,0xF7,0x61,0x03,0xB7,0x69,0x1E,0x13,0x8F,0x58,0xA4,0x4D,0x09,0x50,0xE6,0x58,//10
    0x35,0x82,0x1F,0x0D,0x98,0xB9,0xAB,0x47,0x6A,0x8A,0xCA,0xC5,0xCB,0x52,0x1B,0x18,//20
    0xF3,0xB4,0xD8,0x96,0x68,0x7F,0x14,0xFB,0x81,0x8F,0x48,0x78,0xC9,0x8B,0xE0,0x41,//30
    0x2C,0x11,0xC8,0x64,0xD0,0xA0,0x44,0x20,0x24,0x28,0x5A,0x9D,0xB3,0x6B,0x56,0xAD,//40
    0xBD,0xB2,0x28,0xB9,0xF6,0xE4,0x6C,0x4A,0x7B,0xA4,0x91,0x58,0x9D,0x5E,0x20,0xF8,//50
    0x00,0x56,0xA0,0x3F,0xEE,0x06,0xB7,0x7F,0x8C,0x28,0xBC,0x7C,0x9D,0x8C,0x2D,0xC0,//60
    0x05,0x9F,0x4B,0xE5,0x61,0x12,0x56,0xCB,0xC1,0xCA,0x8C,0xDE,0xF0,0x74,0x6A,0xDB,//70
    0xFC,0x0E,0xF6,0xB8,0x3B,0xD7,0x2F,0xB2,0x16,0xBB,0x2B,0xA0,0x98,0x54,0x78,0x46,//80
    0x8E,0x2F,0x48,0x38,0x47,0x27,0x62,0x25,0xAE,0x66,0xF2,0xDD,0x23,0xA3,0x52,0x49,//90
    0x3D,0x54,0x3A,0x7B,0x76,0x31,0xD2,0xE2,0x25,0x61,0xE6,0xED,0x1A,0x58,0x4D,0xF7,//a0
    0x22,0x7B,0xBF,0x82,0x60,0x32,0x6B,0xCE,0x30,0x35,0x46,0x1B,0xF6,0x6B,0x97,0xD7,//b0
    0xF0,0x09,0x04,0x36,0xF9,0x49,0x8D,0x61,0x05,0xE1,0xA1,0x06,0x34,0x05,0xD1,0x9D,//c0
    0x8E,0xC9,0x90,0x61,0x42,0x94,0x67,0xC3,0x20,0xC3,0x4F,0xAC,0xCE,0x51,0x44,0x96,//d0
    0x8A,0x8C,0xE1,0x04,0x45,0x90,0x3E,0xFC,0x2D,0x9C,0x57,0x10,0x00,0x29,0x80,0xB1,//e0
    0xE5,0x69,0x3B,0x94,0xD7,0x43,0x7B,0xDD,0x5B,0xEA,0xC7,0x54,0xBA,0x90,0xC7,0x87,//f0
    0x58,0xFB,0x74,0xE0,0x1D,0x4E,0x36,0xFA,0x5C,0x93,0xAE,0x11,0x9A,0x15,0x5E,0x07,//100
    0x03,0x01,0xFB,0x78,0x8A,0x40,0xD3,0x05,0xB3,0x4D,0xA0,0xD7,0xA5,0x59,0x00,0x40,//110
    0x9E,0x2C,0x4A,0x63,0x3B,0x37,0x41,0x20,0x56,0xB4,0xBB,0x73,0x25,0x00,0x00,0x00,//120
};
// public key (0x0) encrypted key
static const unsigned char _hdcp_encrypted_by_0_key[HDCP_KEY_SIZE] = {
    0x3B,0x13,0x2F,0xE0,0xAE,0x1C,0x6C,0x5E,0x16,0xFC,0x61,0xEC,0xC5,0x3D,0xE4,0x8B,//20
    0xEC,0x30,0x4F,0x05,0x78,0x5D,0x56,0x19,0x84,0x65,0x20,0x6D,0xDA,0xBF,0x8D,0x1B,//30
    0x2F,0x4E,0xC6,0xE8,0x61,0x85,0x63,0x64,0x68,0xF7,0x14,0x11,0x19,0x02,0x60,0xAF,//40
    0xCE,0xD4,0x2D,0xE1,0x17,0x87,0xB8,0xCF,0x2D,0x04,0x2A,0x94,0x65,0x9B,0xB5,0x25,//50
    0x91,0x5F,0x65,0x31,0x1A,0x4A,0x98,0x4B,0x4F,0xEC,0x47,0xF0,0xF8,0x45,0x26,0xC6,//60
    0x44,0x04,0xAD,0xAD,0xA5,0x6E,0x57,0x04,0x5D,0x69,0x96,0x47,0x24,0xEC,0x70,0xB7,//70
    0x7B,0x3A,0xF3,0x3F,0x7A,0x05,0xE9,0x7E,0xEA,0x6D,0xEB,0x0A,0x99,0xF3,0x67,0x37,//80
    0x78,0xC3,0x19,0x45,0xC5,0x83,0x15,0x17,0xEB,0x44,0x80,0xE3,0x63,0x0A,0x30,0xE3,//90
    0xAC,0xD9,0x95,0xE9,0xC4,0xBE,0x8A,0x4F,0x4D,0x5F,0x51,0xA2,0xF2,0x5B,0x5E,0x59,//a0
    0xD0,0x17,0x36,0x78,0x7C,0xEB,0xFD,0x0A,0x89,0x52,0x07,0x5F,0x91,0xE7,0xF0,0xAF,//b0
    0x68,0x62,0xDC,0x43,0xE2,0x9C,0x72,0xE9,0x91,0x5C,0xC2,0x43,0x3A,0x9B,0x01,0x30,//c0
    0xF5,0x39,0x7A,0x8C,0x2E,0x31,0x2C,0xC1,0x67,0xC2,0xFA,0x56,0x41,0xAA,0x84,0xCC,//d0
    0xF9,0x0C,0x4B,0x3A,0xE7,0x01,0x24,0x30,0x7A,0x1A,0xA5,0x31,0x8E,0x63,0xCD,0xA2,//e0
    0xA7,0x8A,0xE9,0xC4,0x0B,0x01,0x66,0x8B,0xBF,0xA1,0x3D,0x18,0x67,0xAE,0x8C,0x35,//f0
    0x01,0xEC,0x4C,0xD2,0x2E,0x0E,0x4F,0xBA,0xC5,0xA3,0xD1,0xE8,0x3E,0xD3,0x94,0x9D,//100
    0x70,0x91,0xA0,0xE6,0x5E,0xBA,0xD4,0x60,0x89,0x0A,0x14,0xB7,0x7F,0xF1,0xD0,0x86,//110
    0x56,0xDA,0x7F,0x42,0x88,0xAA,0xE1,0xF6,0x32,0x30,0xFC,0x16,0x37,0x0C,0x20,0x3D,//120
    0x9D,0xB2,0x6E,0xE9,0x76,0x99,0x0E,0x31,0x06,0xED,0x6A,0x9C,0xD6,0x1A,0x4C,0x04,//130
};
// public key(guid) encrypted key
static const unsigned char _hdcp_encrypted_by_guid_key[HDCP_KEY_SIZE] = {
    0xC7,0xF6,0xFB,0x0E,0x16,0x61,0xE4,0x8B,0xE4,0x29,0x7A,0xF8,0xAF,0x59,0x74,0xD7,//20
    0x07,0x7A,0x1B,0x09,0x85,0xF1,0xE5,0x47,0xED,0xCE,0x08,0x18,0x35,0x25,0x12,0x7F,//30
    0xCA,0xAD,0x98,0x7C,0xB2,0xA4,0x85,0xBD,0x28,0x85,0x10,0xFB,0x28,0xB7,0xFA,0xE4,//40
    0xD1,0x77,0x61,0x07,0xA6,0x53,0x97,0x41,0x00,0x06,0xDE,0x55,0x87,0xE3,0xF9,0x53,//50
    0x6D,0x10,0x84,0xCD,0xBD,0x17,0x5A,0xFD,0xD2,0xF3,0x9A,0x86,0x9D,0x2A,0x45,0xC3,//60
    0x7B,0xB9,0x68,0x9D,0xA3,0x90,0x09,0x91,0xBC,0x70,0xF7,0x28,0x49,0xEE,0x31,0xCC,//70
    0x5E,0xC6,0x5C,0x9A,0x75,0x76,0x88,0xF9,0x7C,0x0F,0xDF,0x61,0x08,0xC2,0x29,0x77,//80
    0xF4,0x4E,0x8C,0xE4,0x43,0x34,0x39,0x46,0xA8,0xF9,0xFE,0xD4,0xA3,0x58,0x6F,0xBC,//90
    0x43,0x63,0x23,0xB8,0x90,0xDC,0x59,0xA1,0x93,0x23,0x84,0x1C,0x77,0x3D,0xD4,0x68,//a0
    0x41,0xF5,0x32,0x0E,0x23,0x7E,0x38,0x16,0x91,0x04,0xAF,0xA5,0x32,0x42,0x57,0x04,//b0
    0x6F,0x10,0xCF,0x51,0xC8,0x84,0xA0,0x89,0x16,0x42,0xE4,0x2B,0xEB,0x1E,0x55,0x1E,//c0
    0xCF,0x2C,0x7E,0x1D,0xEE,0x02,0xE4,0xDB,0x4B,0xE4,0x8C,0x87,0x1E,0x85,0x2E,0x92,//d0
    0x80,0xCA,0xCB,0xF6,0x16,0x84,0xC7,0xD6,0xD8,0xEA,0xF7,0xA9,0xCA,0x6E,0xC2,0xB2,//e0
    0xC7,0x8F,0x2E,0x2E,0xF1,0x5F,0xF4,0x98,0xC7,0x30,0x41,0x66,0xBC,0xBE,0x0B,0x70,//f0
    0xD7,0x5E,0x65,0xEF,0x64,0x91,0xCF,0xA9,0x78,0x40,0x58,0x43,0x42,0x04,0x05,0x58,//100
    0xCF,0x5D,0x27,0x52,0x6C,0x75,0x4F,0xE5,0xAA,0x8F,0x53,0x87,0x09,0x47,0x1A,0xA7,//110
    0x74,0x5D,0xC5,0x1E,0xED,0x97,0xFF,0x57,0xBB,0x5C,0x8B,0x22,0x31,0xD9,0x99,0x48,//120
    0x8E,0x2B,0x66,0xFD,0x15,0xF8,0x5D,0xD2,0x6E,0xE2,0xD7,0x93,0x6F,0x3E,0xFB,0x76,//130
};

	    U32 encrypted[400] =
	    	{ 0x48, 0xf8, 0x11, 0xb6, 0x85, 0x66, 0x9b, 0x65,
	    	  0x0b, 0x9f, 0x5a, 0x01, 0xb4, 0x43, 0xaf, 0xd7,
	    	  0x34, 0xeb, 0xbe, 0xe0, 0x52, 0xfb, 0x85, 0xfe,
	    	  0xfa, 0xb1, 0x2f, 0xe4, 0xc3, 0xce, 0xa9, 0x27,
	    	  0x33, 0x74, 0x97, 0xd8, 0xfc, 0x62, 0xb8, 0x92,
	    	  0x4a, 0xb6, 0xce, 0x7b, 0xb8, 0xda, 0x67, 0xbf,
	    	  0xda, 0xea, 0xbf, 0xa9, 0xc0, 0x2a, 0xc8, 0xf6,
	    	  0x44, 0x41, 0x5a, 0x10, 0x59, 0x88, 0x54, 0xcf,
	    	  0x51, 0x91, 0x12, 0xd5, 0xa8, 0x41, 0x3a, 0x8a,
	    	  0x88, 0xd1, 0x5a, 0x9a, 0x55, 0xc1, 0xbb, 0x5e,
	    	  0x8a, 0xa0, 0x84, 0x1b, 0xa8, 0xea, 0x31, 0x59,
	    	  0xea, 0x71, 0x0c, 0xcf, 0x59, 0xf5, 0xa8, 0x32,
	    	  0x57, 0xbb, 0xd4, 0xa0, 0x5b, 0x88, 0x44, 0x66,
	    	  0xd6, 0x80, 0xfa, 0xe9, 0x18, 0xe0, 0x50, 0x73,
	    	  0x92, 0x63, 0xe1, 0x5c, 0x13, 0xbf, 0x7d, 0x0d,
	    	  0x70, 0x0b, 0xf8, 0x25, 0x4a, 0x3b, 0x9c, 0x17,
	    	  0x56, 0xb3, 0x71, 0x2b, 0xfe, 0x3c, 0xcb, 0x7c,
	    	  0x19, 0x28, 0x53, 0xa7, 0x5c, 0x57, 0x47, 0xe3,
	    	  0xe1, 0x4c, 0x76, 0x62, 0x0a, 0x40, 0x30, 0xcf,
	    	  0xbe, 0x51, 0xaf, 0x0d, 0x11, 0x73, 0xd6, 0x6a,
	    	  0xc2, 0xbf, 0x4f, 0xc1, 0x88, 0x8d, 0x14, 0xa6,
	    	  0xd1, 0x92, 0x6c, 0xf7, 0x8a, 0xe6, 0x9c, 0x96,
	    	  0xc5, 0xc4, 0x5c, 0x36, 0xf6, 0xfb, 0x39, 0xf4,
	    	  0x79, 0x3f, 0x7a, 0x30, 0x71, 0x5e, 0x3e, 0xfe,
	    	  0xf3, 0x4d, 0x0c, 0x02, 0x55, 0xeb, 0x08, 0x24,
	    	  0x5f, 0x64, 0xd7, 0xcf, 0xf3, 0x48, 0x35, 0x03,
	    	  0xc4, 0xc8, 0x29, 0xf7, 0x9d, 0xcf, 0x21, 0xb8,
	    	  0x67, 0x05, 0xc6, 0x47, 0x05, 0x1b, 0x5f, 0xf3,
	    	  0xa7, 0xbc, 0x23, 0xf0, 0x09, 0xc4, 0x90, 0x44,
	    	  0x5d, 0x3f, 0xf9, 0x79, 0x74, 0xea, 0x7b, 0x42,
	    	  0x57, 0x88, 0xce, 0x32, 0x43, 0xa5, 0xf4, 0x4e,
	    	  0x05, 0xc9, 0x73, 0xc2, 0x49, 0x94, 0x85, 0x5c,
	    	  0xa2, 0x11, 0x91, 0x1f, 0x9e, 0xe3, 0x21, 0xbe,
	    	  0xe9, 0x36, 0x52, 0xec, 0x4b, 0xa6, 0x7d, 0xf6,
	    	  0x8a, 0x85, 0xb9, 0xe1, 0xc7, 0x6e, 0x6b, 0x08,
	    	  0x9d, 0xf2, 0xee, 0x7d, 0x28, 0xbd, 0xf0, 0x9d } ;

static int _hdcp_loadkey(struct nxp_hdcp *me)
{
    /* TODO */
#if 0
    u32 val;
    int cnt = 0;

    hdmi_write_mask(HDMI_EFUSE_CTRL, ~0, HDMI_EFUSE_CTRL_HDCP_KEY_READ);

    do {
        val = hdmi_readb(HDMI_EFUSE_STATUS);
        if (val & HDMI_EFUSE_ECC_DONE)
            break;
        cnt++;
        mdelay(1);
    } while (cnt < KEY_LOAD_RETRY_CNT);

    if (cnt == KEY_LOAD_RETRY_CNT) {
        pr_err("%s: error HDMI_EFUSE_ECC_DONE\n", __func__);
        return -ETIMEDOUT;
    }

    val = hdmi_readb(HDMI_EFUSE_STATUS);
    if (val & HDMI_EFUSE_ECC_FAIL) {
        pr_err("%s: HDMI_EFUSE_ECC_FAIL\n", __func__);
        return -1;
    }

    printk("%s: load key is ok\n", __func__);
#else
#if 0
    int i = 0;
    /*const char *hdcp_key_table = _hdcp_encrypted_by_guid_key;*/
    const char *hdcp_key_table = _hdcp_encrypted_by_0_key;
    for (i = 0; i < HDCP_KEY_SIZE; i++) {
         hdmi_writeb(HDMI_AES_DATA, hdcp_key_table[i]);
    }
    hdmi_writeb(HDMI_AES_START, 0x01);
    do {
        if(!hdmi_readb(HDMI_AES_START))
            break;
    } while (1);
#else

    U32 r_di;
    u32 regvalue;
    u32 cnt;
    for (cnt=0; cnt<288; cnt = cnt+1)
    {
        r_di = encrypted[cnt];
        hdmi_writeb (HDMI_AES_DATA, r_di);
    }

    //NX_CONSOLE_Printf ("\n[DEBUG] ###############  AES Start  ################");
    hdmi_writeb(HDMI_AES_START, 0x01);

    regvalue = hdmi_readb(HDMI_AES_START );
    while( regvalue != 0x00 )
    {
        // decrypt°¡ ¿Ï·áµÇ¸é 0¹ø ºñÆ®°¡ 0ÀÌ µÈ´Ù. ( AES decryption complete )
        regvalue = hdmi_readb(HDMI_AES_START );
    }
#endif
#endif
    return 0;
}

static int _hdcp_start_encryption(struct nxp_hdcp *me)
{
    u32 val;
    u32 cnt = 0;

    do {
        val = hdmi_readb(HDMI_STATUS);

        if (val & HDMI_AUTHEN_ACK_AUTH) {
            _hdcp_encryption(me, true);
            break;
        }

        mdelay(1);
        cnt++;
    } while (cnt < ENCRYPT_CHECK_CNT);

    if (cnt == ENCRYPT_CHECK_CNT) {
        pr_err("%s: error timeout\n", __func__);
        _hdcp_encryption(me, false);
        return -ETIMEDOUT;
    }

    printk("%s: encryption is start\n", __func__);
    return 0;
}

static int _hdcp_check_repeater(struct nxp_hdcp *me)
{
    int val, i;
    int cnt = 0, cnt2 = 0;

    u8 bcaps = 0;
    u8 status[BSTATUS_SIZE];
    u8 rx_v[SHA_1_HASH_SIZE];
    u8 ksv_list[HDCP_MAX_DEVS * HDCP_KSV_SIZE];

    u32 dev_cnt;
    int ret = 0;

    memset(status, 0, sizeof(status));
    memset(rx_v, 0, sizeof(rx_v));
    memset(ksv_list, 0, sizeof(ksv_list));

    do {
        if (_hdcp_read_bcaps(me) < 0)
            goto check_repeater_err;

        bcaps = hdmi_readb(HDMI_HDCP_BCAPS);

        if (bcaps & KSV_FIFO_READY)
            break;

        msleep(KSV_FIFO_CHK_DELAY);
        cnt++;
    } while (cnt < KSV_FIFO_RETRY_CNT);

    if (cnt == KSV_FIFO_RETRY_CNT) {
        ret = REPEATER_TIMEOUT_ERROR;
        goto check_repeater_err;
    }

    printk("%s: repeater : ksv fifo ready\n", __func__);

    if (_hdcp_i2c_read(me, HDCP_BSTATUS, BSTATUS_SIZE, status) < 0) {
        ret = -ETIMEDOUT;
        goto check_repeater_err;
    }

    if (status[1] & MAX_CASCADE_EXCEEDED) {
        ret = MAX_CASCADE_EXCEEDED_ERROR;
        goto check_repeater_err;
    } else if (status[0] & MAX_DEVS_EXCEEDED) {
        ret = MAX_DEVS_EXCEEDED_ERROR;
        goto check_repeater_err;
    }

    hdmi_writeb(HDMI_HDCP_BSTATUS_0, status[0]);
    hdmi_writeb(HDMI_HDCP_BSTATUS_1, status[1]);

    printk("%s: status[0] :0x%02x\n", __func__, status[0]);
    printk("%s: status[1] :0x%02x\n", __func__, status[1]);

    dev_cnt = status[0] & 0x7f;
    printk("%s: repeater : dev cnt = %d\n", __func__, dev_cnt);

    if (dev_cnt) {
        if (_hdcp_i2c_read(me, HDCP_KSVFIFO, dev_cnt * HDCP_KSV_SIZE,
                    ksv_list) < 0) {
            pr_err("%s: failed to i2c read HDCP_KSVFIFO\n", __func__);
            ret = -ETIMEDOUT;
            goto check_repeater_err;
        }

        cnt = 0;

        do {
            hdmi_write_bytes(HDMI_HDCP_KSV_LIST_(0),
                    &ksv_list[cnt * 5], HDCP_KSV_SIZE);

            val = HDMI_HDCP_KSV_WRITE_DONE;

            if (cnt == dev_cnt - 1)
                val |= HDMI_HDCP_KSV_END;

            hdmi_write(HDMI_HDCP_KSV_LIST_CON, val);

            if (cnt < dev_cnt - 1) {
                cnt2 = 0;
                do {
                    val = hdmi_readb(HDMI_HDCP_KSV_LIST_CON);
                    if (val & HDMI_HDCP_KSV_READ)
                        break;
                    cnt2++;
                } while (cnt2 < KSV_LIST_RETRY_CNT);

                if (cnt2 == KSV_LIST_RETRY_CNT)
                    pr_err("%s: error, ksv list not readed\n", __func__);
            }
            cnt++;
        } while (cnt < dev_cnt);
    } else {
        hdmi_writeb(HDMI_HDCP_KSV_LIST_CON, HDMI_HDCP_KSV_LIST_EMPTY);
    }

    if (_hdcp_i2c_read(me, HDCP_SHA1, SHA_1_HASH_SIZE, rx_v) < 0) {
        pr_err("%s: failed to i2c read HDCP_SHA1\n", __func__);
        ret = -ETIMEDOUT;
        goto check_repeater_err;
    }

#ifdef DEBUG
    for (i = 0; i < SHA_1_HASH_SIZE; i++)
        printk("%s: [i2c] SHA-1 rx :: %02x\n", __func__, rx_v[i]);
#endif

    hdmi_write_bytes(HDMI_HDCP_SHA1_(0), rx_v, SHA_1_HASH_SIZE);

    val = hdmi_readb(HDMI_HDCP_SHA_RESULT);
    if (val & HDMI_HDCP_SHA_VALID_RD) {
        if (val & HDMI_HDCP_SHA_VALID) {
            printk("%s: SHA-1 result is ok\n", __func__);
            hdmi_writeb(HDMI_HDCP_SHA_RESULT, 0x0);
        } else {
            pr_err("%s: SHA-1 result is not valid\n", __func__);
            hdmi_writeb(HDMI_HDCP_SHA_RESULT, 0x0);
            ret = -EINVAL;
            goto check_repeater_err;
        }
    } else {
        pr_err("%s: SHA-1 result is not ready\n", __func__);
        hdmi_writeb(HDMI_HDCP_SHA_RESULT, 0x0);
        ret = -ETIMEDOUT;
        goto check_repeater_err;
    }

    printk("%s: check repeater is ok\n", __func__);
    return 0;

check_repeater_err:
    pr_err("%s: failed(err: %d)\n", __func__, ret);
    return ret;
}

static int _hdcp_bksv(struct nxp_hdcp *me)
{
    me->auth_state = RECEIVER_READ_READY;

    if (_hdcp_read_bcaps(me) < 0)
        goto bksv_start_err;

    me->auth_state = BCAPS_READ_DONE;

    if (_hdcp_read_bksv(me) < 0)
        goto bksv_start_err;

    hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_CLR_ALL_RESULTS);

    me->auth_state = BKSV_READ_DONE;
    printk("%s: bksv start is ok\n", __func__);
    return 0;

bksv_start_err:
    pr_err("%s: failed to start bskv\n", __func__);
    msleep(100);
    return -1;
}

static int _hdcp_second_auth(struct nxp_hdcp *me)
{
    int ret = 0;

    printk("%s\n", __func__);

    if (!me->is_start) {
        pr_err("%s: hdcp is not started\n", __func__);
        return -EINVAL;
    }

    if (!IS_HDMI_RUNNING(me)) {
        pr_err("%s: hdmi is not running\n", __func__);
        return -EINVAL;
    }

    ret = _hdcp_check_repeater(me);

    if (!ret) {
        me->auth_state = SECOND_AUTHENTICATION_DONE;
        _hdcp_start_encryption(me);
    } else {
        switch (ret) {
        case REPEATER_ILLEGAL_DEVICE_ERROR:
            hdmi_writeb(HDMI_HDCP_CTRL2, 0x1);
            mdelay(1);
            hdmi_writeb(HDMI_HDCP_CTRL2, 0x0);
            pr_err("%s: repeater : illegal device\n", __func__);
            break;

        case REPEATER_TIMEOUT_ERROR:
            hdmi_write_mask(HDMI_HDCP_CTRL1, ~0,
                    HDMI_HDCP_SET_REPEATER_TIMEOUT);
            hdmi_write_mask(HDMI_HDCP_CTRL1, 0,
                    HDMI_HDCP_SET_REPEATER_TIMEOUT);
            pr_err("%s: repeater : timeout\n", __func__);
            break;

        case MAX_CASCADE_EXCEEDED_ERROR:
            pr_err("%s: repeater : exceeded MAX_CASCADE\n", __func__);
            break;

        case MAX_DEVS_EXCEEDED_ERROR:
            pr_err("%s: repeater : exceeded MAX_DEVS\n", __func__);
            break;

        default:
            break;
        }

        me->auth_state = NOT_AUTHENTICATED;
        return -EINVAL;
    }

    printk("%s: second authentication is OK\n", __func__);
    return 0;
}

static int _hdcp_write_aksv(struct nxp_hdcp *me)
{
    printk("%s\n", __func__);

    if (me->auth_state != BKSV_READ_DONE) {
        pr_err("%s: bksv is not ready\n", __func__);
        return -EINVAL;
    }

    if (!IS_HDMI_RUNNING(me)) {
        pr_err("%s: hdmi is not running\n", __func__);
        return -EINVAL;
    }

    if (_hdcp_write_key(me, AN_SIZE, HDMI_HDCP_AN_(0), HDCP_AN) < 0) {
        pr_err("%s: failed to _hdcp_write_key() HDCP_AN\n", __func__);
        return -EINVAL;
    }

    me->auth_state = AN_WRITE_DONE;
    printk("%s: write AN is done\n", __func__);

    if (_hdcp_write_key(me, AKSV_SIZE, HDMI_HDCP_AKSV_(0), HDCP_AKSV) < 0) {
        pr_err("%s: failed to _hdcp_write_key() HDCP_AKSV\n", __func__);
        return -EINVAL;
    }

    msleep(100);

    me->auth_state = AKSV_WRITE_DONE;

    printk("%s success!!!\n", __func__);
    return 0;
}

static int _hdcp_check_ri(struct nxp_hdcp *me)
{
    printk("%s\n", __func__);

    if (me->auth_state < AKSV_WRITE_DONE) {
        pr_err("%s: ri check is not ready\n", __func__);
        return -EINVAL;
    }

    if (!IS_HDMI_RUNNING(me)) {
        pr_err("%s: hdmi is not running\n", __func__);
        return -EINVAL;
    }

    if (_hdcp_read_ri(me) < 0) {
        pr_err("%s: failed to _hdcp_read_ri()\n", __func__);
        return -EINVAL;
    }

    if (me->is_repeater)
        me->auth_state = SECOND_AUTHENTICATION_RDY;
    else {
        me->auth_state = FIRST_AUTHENTICATION_DONE;
        _hdcp_start_encryption(me);
    }

    printk("%s: ri check is OK\n", __func__);
    return 0;
}

/**
 * work
 */
static void _hdcp_work(struct work_struct *work)
{
    struct nxp_hdcp *me = container_of(work, struct nxp_hdcp, work);

    if (!me->is_start)
        return;

    if (!IS_HDMI_RUNNING(me))
        return;

    if (me->event & HDCP_EVENT_READ_BKSV_START) {
        if (_hdcp_bksv(me) < 0)
            goto work_err;
        else
            me->event &= ~HDCP_EVENT_READ_BKSV_START;
    }

    if (me->event & HDCP_EVENT_SECOND_AUTH_START) {
        if (_hdcp_second_auth(me) < 0)
            goto work_err;
        else
            me->event &= ~HDCP_EVENT_SECOND_AUTH_START;
    }

    if (me->event & HDCP_EVENT_WRITE_AKSV_START) {
        if (_hdcp_write_aksv(me) < 0)
            goto work_err;
        else
            me->event &= HDCP_EVENT_WRITE_AKSV_START;
    }

    if (me->event & HDCP_EVENT_CHECK_RI_START) {
        if (_hdcp_check_ri(me) < 0)
            goto work_err;
        else
            me->event &= ~HDCP_EVENT_CHECK_RI_START;
    }

    return;
work_err:
    if (!me->is_start)
        return;
    if (!IS_HDMI_RUNNING(me))
        return;

    _hdcp_reset_auth(me);
    ;
}

/**
 * member functions
 */

static irqreturn_t
nxp_hdcp_irq_handler(struct nxp_hdcp *me)
{
    u32 event = 0;
    u8 flag;

    if (!IS_HDMI_RUNNING(me)) {
        me->event       = HDCP_EVENT_STOP;
        me->auth_state  = NOT_AUTHENTICATED;
        return IRQ_HANDLED;
    }

    flag = hdmi_readb(HDMI_STATUS);

    printk("%s: flag 0x%x\n", __func__, flag);

    if (flag & HDMI_WTFORACTIVERX_INT_OCC) {
        event |= HDCP_EVENT_READ_BKSV_START;
        /*hdmi_write_mask(HDMI_STATUS, ~0, HDMI_WTFORACTIVERX_INT_OCC);*/
        hdmi_write(HDMI_STATUS, (1<<0));
        hdmi_write(HDMI_HDCP_I2C_INT, 0x0);
    }

    if (flag & HDMI_WRITE_INT_OCC) {
        event |= HDCP_EVENT_WRITE_AKSV_START;
        hdmi_write_mask(HDMI_STATUS, ~0, HDMI_WRITE_INT_OCC);
        hdmi_write(HDMI_HDCP_AN_INT, 0x0);
    }

    if (flag & HDMI_UPDATE_RI_INT_OCC) {
        event |= HDCP_EVENT_WRITE_AKSV_START;
        hdmi_write_mask(HDMI_STATUS, ~0, HDMI_UPDATE_RI_INT_OCC);
        hdmi_write(HDMI_HDCP_RI_INT, 0x0);
    }

    if (flag & HDMI_WATCHDOG_INT_OCC) {
        event |= HDCP_EVENT_SECOND_AUTH_START;
        hdmi_write_mask(HDMI_STATUS, ~0, HDMI_WATCHDOG_INT_OCC);
        hdmi_write(HDMI_HDCP_WDT_INT, 0x0);
    }

    if (!event) {
        pr_err("%s: unknown irq\n", __func__);
        return IRQ_HANDLED;
    }

    if (IS_HDMI_RUNNING(me)) {
        me->event |= event;
        queue_work(me->wq, &me->work);
    } else {
        me->event = HDCP_EVENT_STOP;
        me->auth_state = NOT_AUTHENTICATED;
    }

    return IRQ_HANDLED;
}

static int nxp_hdcp_prepare(struct nxp_hdcp *me)
{
    me->wq = create_singlethread_workqueue("khdcpd");
    if (!me->wq)
        return -ENOMEM;

    printk("%s\n", __func__);
    INIT_WORK(&me->work, _hdcp_work);
    return 0;
}

#if 1
// org code
static int nxp_hdcp_start(struct nxp_hdcp *me)
{
    if (me->is_start)
        return 0;

    me->event = HDCP_EVENT_STOP;
    me->auth_state = NOT_AUTHENTICATED;

    pr_debug("%s\n", __func__);

    _hdcp_sw_reset(me);
    _hdcp_encryption(me, false);

    msleep(120);

    if (_hdcp_loadkey(me) < 0) {
        pr_err("%s: failed to _hdcp_loadkey()\n", __func__);
        return -1;
    }

    hdmi_write(HDMI_GCP_CON, HDMI_GCP_CON_NO_TRAN);
    hdmi_write(HDMI_STATUS_EN, HDMI_INT_EN_ALL);

    hdmi_write(HDMI_HDCP_CTRL1, HDMI_HDCP_CP_DESIRED_EN);
    me->is_start = true;

    hdmi_set_int_mask(HDMI_INTC_EN_HDCP, 1);

    return 0;
}

#else
// test code
#if 1
#include <mach/platform.h>
#endif
static int nxp_hdcp_start(struct nxp_hdcp *me)
{
    if (me->is_start)
        return 0;

    me->event = HDCP_EVENT_STOP;
    me->auth_state = NOT_AUTHENTICATED;

    printk("%s\n", __func__);

    /*_hdcp_sw_reset(me);*/
    /*_hdcp_encryption(me, false);*/

    // test code : pclk to 12MHz
#if 0
    {
        volatile struct NX_CLKPWR_RegisterSet *clkpwr;
        volatile u32 *clkpwr_reg;
        volatile u32 *pllset_reg;
        clkpwr = (volatile struct NX_CLKPWR_RegisterSet*)IO_ADDRESS(PHY_BASEADDR_CLKPWR_MODULE);
        clkpwr_reg = (volatile u32 *)clkpwr;
        pllset_reg = (clkpwr_reg + 2 + 0);
        *pllset_reg &= ~(1 << 28);
        *clkpwr_reg  = (1 << 0);
        while(*clkpwr_reg & (1<<31));
    }
#endif

    // randomness
    hdmi_write(HDMI_AN_SEED_SEL, 0xFE);
    hdmi_write(HDMI_AN_SEED_0, 0x00);
    hdmi_write(HDMI_AN_SEED_1, 0x00);
    hdmi_write(HDMI_AN_SEED_2, 0x80);
    hdmi_write(HDMI_AN_SEED_3, 0x00);

    msleep(120);

    if (_hdcp_loadkey(me) < 0) {
        pr_err("%s: failed to _hdcp_loadkey()\n", __func__);
        return -1;
    }

    hdmi_write(HDMI_HDCP_OFFSET_TX_0, 0x00);
    hdmi_write(HDMI_HDCP_OFFSET_TX_1, 0xa0);
    hdmi_write(HDMI_HDCP_OFFSET_TX_2, 0x00);
    hdmi_write(HDMI_HDCP_OFFSET_TX_3, 0x00);
    hdmi_write(HDMI_HDCP_CYCLE_AA, 0x8f);

    hdmi_write(HDMI_HDCP_BCAPS, 0x40);

    hdmi_write(HDMI_HDCP_SHA1_REN0, 0xf8);
    hdmi_write(HDMI_HDCP_SHA1_REN1, 0xbf);

    /*hdmi_write(HDMI_GCP_CON, HDMI_GCP_CON_NO_TRAN);*/
    /*hdmi_write(HDMI_STATUS_EN, HDMI_INT_EN_ALL);*/
    hdmi_write(HDMI_STATUS_EN, 0x17);
    printk("STATUS_EN 0x%x\n", hdmi_readb(HDMI_STATUS_EN));

    /*hdmi_write(HDMI_HDCP_CTRL1, HDMI_HDCP_CP_DESIRED_EN);*/
    hdmi_write(HDMI_HDCP_CTRL1, 0x82);
    me->is_start = true;

    hdmi_set_int_mask(HDMI_INTC_EN_HDCP, 1);

    // this is test
#if 0
    {
        U32 regvalue = hdmi_readb( HDMI_STATUS) & 0x01; // ´ëÃ¼°¡´ÉÇÑÁö?
        while( (regvalue & 0x01) == 0)
            regvalue = hdmi_readb( HDMI_STATUS) & 0x01; // ´ëÃ¼°¡´ÉÇÑÁö?

        hdmi_writeb(HDMI_HDCP_CHECK_RESULT, 0x00);
        hdmi_writeb(HDMI_HDCP_I2C_INT, 0x00);
        hdmi_writeb(HDMI_STATUS, 0x01);

        hdmi_writeb(HDMI_HDCP_BKSV_(0), 0xcd);
        hdmi_writeb(HDMI_HDCP_BKSV_(1), 0x1a);
        hdmi_writeb(HDMI_HDCP_BKSV_(2), 0xf2);
        hdmi_writeb(HDMI_HDCP_BKSV_(3), 0x1e);
        hdmi_writeb(HDMI_HDCP_BKSV_(4), 0x51);

        hdmi_writeb(HDMI_HDCP_BCAPS, 0x40);


        regvalue =  hdmi_readb(HDMI_HDCP_AN_INT ) >> 0;
        while( (regvalue & 0x01) == 0 )
        {
            regvalue =  hdmi_readb(HDMI_HDCP_AN_INT ) >> 0;
            printk("hdcp an int 0x%x, status 0x%x\n", regvalue, hdmi_readb(HDMI_STATUS));
        }
    }
#endif

    return 0;
}
#endif

static int nxp_hdcp_stop(struct nxp_hdcp *me)
{
    u32 val;

    if (!me->start)
        return 0;

    printk("%s\n", __func__);

    /* first stop workqueue */
    /*cancel_work_sync(&me->work);*/
    flush_work(&me->work);

    /*hdmi_set_int_mask(HDMI_INTC_EN_HDCP, 0);*/

    me->event       = HDCP_EVENT_STOP;
    me->auth_state  = NOT_AUTHENTICATED;
    me->is_start    = false;

    hdmi_writeb(HDMI_HDCP_CTRL1, 0x0);

    /*hdmi_sw_hpd_enable(false);*/

    val = HDMI_UPDATE_RI_INT_EN | HDMI_WRITE_INT_EN |
        HDMI_WATCHDOG_INT_EN | HDMI_WTFORACTIVERX_INT_EN;
    hdmi_write_mask(HDMI_STATUS_EN, 0, val);
    hdmi_write_mask(HDMI_STATUS_EN, ~0, val);

    hdmi_write_mask(HDMI_STATUS, ~0, HDMI_INT_EN_ALL);

    _hdcp_encryption(me, false);

    hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_CLR_ALL_RESULTS);

    me->start = 0;

    return 0;
}

static int nxp_hdcp_suspend(struct nxp_hdcp *me)
{
    printk("%s\n", __func__);
    return 0;
}

static int nxp_hdcp_resume(struct nxp_hdcp *me)
{
    printk("%s\n", __func__);
    return 0;
}

/**
 * i2c driver
 */
static int __devinit _hdcp_i2c_probe(struct i2c_client *client,
        const struct i2c_device_id *dev_id)
{
    printk("%s\n", __func__);
    return 0;
}

static int _hdcp_i2c_remove(struct i2c_client *client)
{
    printk("%s\n", __func__);
    return 0;
}

static struct i2c_device_id _hdcp_idtable[] = {
    {"nxp_hdcp", 0},
};
MODULE_DEVICE_TABLE(i2c, _hdcp_idtable);

static struct i2c_driver _hdcp_driver = {
    .driver = {
        .name  = "nxp_hdcp",
        .owner = THIS_MODULE,
    },
    .id_table  = _hdcp_idtable,
    .probe     = _hdcp_i2c_probe,
    .remove    = _hdcp_i2c_remove,
};

/**
 * public api
 */
int nxp_hdcp_init(struct nxp_hdcp *me, struct nxp_v4l2_i2c_board_info *i2c_info)
{
    int ret = 0;

    printk("%s\n", __func__);

    memset(me, 0, sizeof(struct nxp_hdcp));

    ret = i2c_add_driver(&_hdcp_driver);
    if (ret < 0) {
        pr_err("%s: failed to i2c_add_driver()\n", __func__);
        return -EINVAL;
    }

    me->client = nxp_v4l2_get_i2c_client(i2c_info);
    if (!me->client) {
        pr_err("%s: can't find hdcp i2c device\n", __func__);
        return -EINVAL;
    }

    me->irq_handler = nxp_hdcp_irq_handler;
    me->prepare = nxp_hdcp_prepare;
    me->start   = nxp_hdcp_start;
    me->stop    = nxp_hdcp_stop;
    me->suspend = nxp_hdcp_suspend;
    me->resume  = nxp_hdcp_resume;

    mutex_init(&me->mutex);

    return 0;
}

void nxp_hdcp_cleanup(struct nxp_hdcp *me)
{
    printk("%s\n", __func__);

    i2c_del_driver(&_hdcp_driver);

    if (me->client) {
        i2c_unregister_device(me->client);
        me->client = NULL;
    }
}
