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

#include "regs-hdmi.h"
#include "hdmi-priv.h"
#include "nxp-hdcp.h"

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

    pr_debug("%s: read data ok\n", __func__);

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

    pr_debug("%s: write data ok\n", __func__);
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
        pr_debug("%s: %s[%d] : 0x%02x\n", __func__,
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

    hdmi_writeb(HDMI_HDCP_BCAPS, bcaps);

    if (bcaps & HDMI_HDCP_BCAPS_REPEATER)
        me->is_repeater = true;
    else
        me->is_repeater = false;

    pr_debug("%s: device is %s\n", __func__,
            me->is_repeater ? "REPEAT" : "SINK");
    pr_debug("%s: [i2c] bcaps : 0x%02x\n", __func__, bcaps);

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
            pr_debug("%s: i2c read: bksv[%d]: 0x%x\n",
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

        if ((zero == 20) && (one == 20)) {
            hdmi_write_bytes(HDMI_HDCP_BKSV_(0), bksv, BKSV_SIZE);
            break;
        }

        pr_debug("%s: invalid bksv, retry : %d\n", __func__, cnt);

        msleep(BKSV_DELAY);
        cnt++;
    } while (cnt < BKSV_RETRY_CNT);

    if (cnt == BKSV_RETRY_CNT) {
        pr_err("%s: read timeout\n", __func__);
        return -ETIMEDOUT;
    }

    pr_debug("%s: bksv read OK, retry : %d\n", __func__, cnt);
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

    pr_debug("%s: Rx -> rj[0]: 0x%02x, rj[1]: 0x%02x\n", __func__,
            rj[0], rj[1]);
    pr_debug("%s: Tx -> ri[0]: 0x%02x, ri[1]: 0x%02x\n", __func__,
            ri[0], ri[1]);

    if ((ri[0] == rj[0]) && (ri[1] == rj[1]) && (ri[0] | ri[1]))
        hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_RI_MATCH_RESULT_Y);
    else {
        hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_RI_MATCH_RESULT_N);
        pr_err("%s: compare error\n", __func__);
        goto compare_err;
    }

    pr_debug("%s: ri and rj are matched\n", __func__);
    return 0;

compare_err:
    me->event = HDCP_EVENT_STOP;
    me->auth_state = NOT_AUTHENTICATED;
    msleep(10);
    return -EINVAL;
}

static void _hdcp_sw_reset(struct nxp_hdcp *me)
{
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

    pr_debug("%s: reset authentication\n", __func__);

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

    pr_debug("%s: load key is ok\n", __func__);
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

    pr_debug("%s: encryption is start\n", __func__);
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

    pr_debug("%s: repeater : ksv fifo ready\n", __func__);

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

    pr_debug("%s: status[0] :0x%02x\n", __func__, status[0]);
    pr_debug("%s: status[1] :0x%02x\n", __func__, status[1]);

    dev_cnt = status[0] & 0x7f;
    pr_debug("%s: repeater : dev cnt = %d\n", __func__, dev_cnt);

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
        pr_debug("%s: [i2c] SHA-1 rx :: %02x\n", __func__, rx_v[i]);
#endif

    hdmi_write_bytes(HDMI_HDCP_SHA1_(0), rx_v, SHA_1_HASH_SIZE);

    val = hdmi_readb(HDMI_HDCP_SHA_RESULT);
    if (val & HDMI_HDCP_SHA_VALID_RD) {
        if (val & HDMI_HDCP_SHA_VALID) {
            pr_debug("%s: SHA-1 result is ok\n", __func__);
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

    pr_debug("%s: check repeater is ok\n", __func__);
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

    me->auth_state = BKSV_READ_DONE;
    pr_debug("%s: bksv start is ok\n", __func__);
    return 0;

bksv_start_err:
    pr_err("%s: failed to start bskv\n", __func__);
    msleep(100);
    return -1;
}

static int _hdcp_second_auth(struct nxp_hdcp *me)
{
    int ret = 0;

    pr_debug("%s\n", __func__);

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

    pr_debug("%s: second authentication is OK\n", __func__);
    return 0;
}

static int _hdcp_write_aksv(struct nxp_hdcp *me)
{
    pr_debug("%s\n", __func__);

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
    pr_debug("%s: write AN is done\n", __func__);

    if (_hdcp_write_key(me, AKSV_SIZE, HDMI_HDCP_AKSV_(0), HDCP_AKSV) < 0) {
        pr_err("%s: failed to _hdcp_write_key() HDCP_AKSV\n", __func__);
        return -EINVAL;
    }

    msleep(100);

    me->auth_state = AKSV_WRITE_DONE;

    pr_debug("%s success!!!\n", __func__);
    return 0;
}

static int _hdcp_check_ri(struct nxp_hdcp *me)
{
    pr_debug("%s\n", __func__);

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

    pr_debug("%s: ri check is OK\n", __func__);
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

    if (flag & HDMI_WTFORACTIVERX_INT_OCC) {
        event |= HDCP_EVENT_READ_BKSV_START;
        hdmi_write_mask(HDMI_STATUS, ~0, HDMI_WTFORACTIVERX_INT_OCC);
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

    pr_debug("%s\n", __func__);
    INIT_WORK(&me->work, _hdcp_work);
    return 0;
}

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

static int nxp_hdcp_stop(struct nxp_hdcp *me)
{
    u32 val;

    if (!me->start)
        return 0;

    pr_debug("%s\n", __func__);

    /* first stop workqueue */
    cancel_work_sync(&me->work);

    hdmi_set_int_mask(HDMI_INTC_EN_HDCP, 0);

    me->event       = HDCP_EVENT_STOP;
    me->auth_state  = NOT_AUTHENTICATED;
    me->is_start    = false;

    hdmi_writeb(HDMI_HDCP_CTRL1, 0x0);

    hdmi_sw_hpd_enable(false);

    val = HDMI_UPDATE_RI_INT_EN | HDMI_WRITE_INT_EN |
        HDMI_WATCHDOG_INT_EN | HDMI_WTFORACTIVERX_INT_EN;
    hdmi_write_mask(HDMI_STATUS_EN, 0, val);
    hdmi_write_mask(HDMI_STATUS_EN, ~0, val);

    hdmi_write_mask(HDMI_STATUS, ~0, HDMI_INT_EN_ALL);

    _hdcp_encryption(me, false);

    hdmi_writeb(HDMI_HDCP_CHECK_RESULT, HDMI_HDCP_CLR_ALL_RESULTS);

    return 0;
}

static int nxp_hdcp_suspend(struct nxp_hdcp *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int nxp_hdcp_resume(struct nxp_hdcp *me)
{
    pr_debug("%s\n", __func__);
    return 0;
}

/**
 * i2c driver
 */
static int __devinit _hdcp_i2c_probe(struct i2c_client *client,
        const struct i2c_device_id *dev_id)
{
    pr_debug("%s\n", __func__);
    return 0;
}

static int _hdcp_i2c_remove(struct i2c_client *client)
{
    pr_debug("%s\n", __func__);
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

    pr_debug("%s\n", __func__);

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
    pr_debug("%s\n", __func__);

    i2c_del_driver(&_hdcp_driver);

    if (me->client) {
        i2c_unregister_device(me->client);
        me->client = NULL;
    }
}
