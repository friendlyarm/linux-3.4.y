/* linux/drivers/media/video/s5k5cagx.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 * 		http://www.samsung.com/
 *
 * Driver for S5K5CAGX (UXGA camera) from Samsung Electronics
 * 1/4" 2.0Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define DEBUG 1

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <linux/v4l2-subdev.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <media/s5k5cagx_platform.h>
#include <linux/completion.h>
#include <media/v4l2-ctrls.h>

#include "s5k5cagx-preset.h"

/* psw0523 add for initializing worker thread */
/* #define USE_INITIAL_WORKER_THREAD */

#ifdef USE_INITIAL_WORKER_THREAD
#include <linux/workqueue.h>
#endif

#define ADDR_S5K5CAGX 0x3c

#define S5K4EAGX ADDR_S5K4EAGX
#define S5K5CAGX ADDR_S5K5CAGX

#define S5K5CAGX_DRIVER_NAME	"S5K5CAGX"

#define NUM_CTRLS           7

#define PREVIEW_MODE        0
#define CAPTURE_MODE        1

#define MAX_WIDTH           2048
#define MAX_HEIGHT          1536

enum S5K5CAGX_ZOOM_TYPE {
    ZPT_IDLE = 0,
    ZPT_INPUT_SAMPLES,
    ZPT_FACTORS,
    ZPT_SINGLE_STEP,
    ZPT_MOVE_TO_TARGET,
    ZPT_MOVE
};

//#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static int read_device_id_for_s5k5cagx(struct i2c_client *client);

/*
 * Specification
 * Parallel : ITU-R. 656/601 YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Serial : MIPI CSI2 (single lane) YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Resolution : 1280 (H) x 1024 (V)
 * Image control : Brightness, Contrast, Saturation, Sharpness, Glamour
 * Effect : Mono, Negative, Sepia, Aqua, Sketch
 * FPS : 15fps @full resolution, 30fps @VGA, 24fps @720p
 * Max. pixel clock frequency : 48MHz(upto)
 * Internal PLL (6MHz to 27MHz input frequency)
 */
struct s5k5cagx_state {
    struct v4l2_subdev sd;
    struct media_pad pad;
    struct v4l2_ctrl_handler handler;
    /* standard */
    struct v4l2_ctrl *focus;
    struct v4l2_ctrl *wb;
    struct v4l2_ctrl *color_effect;
    struct v4l2_ctrl *exposure;
    /* custom */
    struct v4l2_ctrl *scene_mode;
    struct v4l2_ctrl *anti_shake;
    struct v4l2_ctrl *mode_change;

    bool inited;
    int width;
    int height;
    int mode; // PREVIEW or CAPTURE

    /* for zoom */
    struct v4l2_rect crop;

#ifdef USE_INITIAL_WORKER_THREAD
    struct workqueue_struct *init_wq;
    struct work_struct init_work;
#endif
};

static inline struct s5k5cagx_state *to_state(struct v4l2_subdev *sd)
{
    return container_of(sd, struct s5k5cagx_state, sd);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct s5k5cagx_state, handler)->sd;
}

static unsigned short i2c_read_reg(struct i2c_client *client, unsigned short reg_h, unsigned short reg)
{
    int ret;
    unsigned char i2c_data[10];

    //{0x00, 0x28, 0x70, 0x00},
    i2c_data[0] = 0x00;
    i2c_data[1] = 0x2c;
    i2c_data[2] = reg_h >>8;		//0x70;
    i2c_data[3] = reg_h & 0xff;	//0x00;

    i2c_master_send(client, i2c_data, 4);

    i2c_data[0] = 0x00;
    i2c_data[1] = 0x2e;
    i2c_data[2] = (unsigned char)((reg >> 8) & 0xff);
    i2c_data[3] = (unsigned char)(reg & 0xff);

    i2c_master_send(client, i2c_data, 4);

    i2c_data[0] = 0x0f;
    i2c_data[1] = 0x12;
    i2c_master_send(client, i2c_data, 2);

    ret = i2c_master_recv(client, i2c_data, 2);

    return i2c_data[0]<<8 | i2c_data[1];
}

static inline int s5k5cagx_i2c_write(struct i2c_client *client, unsigned char buf[], int length)
{
    struct i2c_msg msg = {client->addr, 0, length, buf};
    return i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

static int s5k5cagx_write_regs(struct i2c_client *client, struct reg_val regvals[], int size)
{
    unsigned char tmp[4];
    int i, err = 0;
    struct reg_val *regval;
    for (i = 0; i < size ; i++) {
        regval = &regvals[i];
        if (regval->reg == 0xffff)
            mdelay(regval->val);
        else {
            tmp[0] =(unsigned char)( regval->reg >> 8) ;
            tmp[1] =(unsigned char)( regval->reg & 0xff);
            tmp[2] =(unsigned char)( regval->val >> 8) ;
            tmp[3] =(unsigned char)( regval->val & 0xff);
            err = s5k5cagx_i2c_write(client, tmp , 4);
            if (err < 0)
                pr_err("%s: register set failed\n", __func__);
        }
    }

    return err;
}

static struct v4l2_rect *
_get_pad_crop(struct s5k5cagx_state *me, struct v4l2_subdev_fh *fh,
        unsigned int pad, enum v4l2_subdev_format_whence which)
{
    switch (which) {
    case V4L2_SUBDEV_FORMAT_TRY:
        return v4l2_subdev_get_try_crop(fh, pad);
    case V4L2_SUBDEV_FORMAT_ACTIVE:
        return &me->crop;
    default:
        pr_err("%s: invalid which %d\n", __func__, which);
        return &me->crop;
    }
}

static int s5k5cagx_set_frame_size(struct v4l2_subdev *sd, int mode, int width, int height)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct reg_val *array;
    int size;

    printk("%s: mode %d, width %d, height %d\n", __func__, mode, width, height);
    if (mode == PREVIEW_MODE) {
        array = s5k5cagx_reg_preview;
        size = ARRAY_SIZE(s5k5cagx_reg_preview);
    } else {
        array = s5k5cagx_reg_capture;
        size = ARRAY_SIZE(s5k5cagx_reg_capture);
    }
    array[3].val = width;
    array[4].val = height;
    return s5k5cagx_write_regs(client, array, size);
}

static int s5k5cagx_set_zoom_crop(struct s5k5cagx_state *state, int zoom_type, int left, int top, int width, int height)
{
    struct i2c_client *client = v4l2_get_subdevdata(&state->sd);
    struct reg_val *array = s5k5cagx_reg_zoom_template;
    int read_val;

    array[3].val = zoom_type;
    array[5].val = width;
    array[6].val = height;
    array[7].val = left;
    array[8].val = top;

    s5k5cagx_write_regs(client, array, ARRAY_SIZE(s5k5cagx_reg_zoom_template));

    /* check NewZoomSync to 0 */
    do {
        mdelay(100);
        read_val = i2c_read_reg(client, 0x7000, 0x046a);
    } while (read_val != 0);

    /* check success : ErroZoom */
    read_val = i2c_read_reg(client, 0x7000, 0x0468);
    if (read_val != 0) {
        printk("%s: failed error val %d\n", __func__, read_val);
        return -EIO;
    }

    return 0;
}

static int s5k5cagx_s_crop(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct s5k5cagx_state *state = to_state(sd);
    struct v4l2_rect *__crop = _get_pad_crop(state, fh, crop->pad, crop->which);


    if ((crop->rect.left + crop->rect.width) > MAX_WIDTH ||
        (crop->rect.top + crop->rect.height) > MAX_HEIGHT) {
        pr_err("%s: invalid crop rect(left %d, width %d, max width %d, top %d, height %d, max height %d\n)",
                __func__, crop->rect.left, crop->rect.width, MAX_WIDTH,
                crop->rect.top, crop->rect.height, MAX_HEIGHT);
        return -EINVAL;
    }

    if (crop->rect.width == MAX_WIDTH) {
        /* no zoom */
        if (__crop->width != MAX_WIDTH) {
            /* set zoom type to ZPT_IDLE */
            printk("%s: zoom clear!!!\n", __func__);
            s5k5cagx_set_zoom_crop(state, ZPT_IDLE, 0, 0, 0, 0);
        }
    } else {
        int crop_real_width, crop_real_height, crop_real_left, crop_real_top;
        /* crop width's reference is MAX_WIDTH, calculate real width */
        crop_real_width = (state->width*crop->rect.width)/MAX_WIDTH;
        crop_real_height = (state->width*crop->rect.height)/MAX_WIDTH;
        crop_real_left = (state->width*crop->rect.left)/MAX_WIDTH;
        crop_real_top = (state->width*crop->rect.top)/MAX_WIDTH;

        printk("%s: crop(%d:%d-%d:%d)\n", __func__,
                crop_real_left, crop_real_top, crop_real_width, crop_real_height);
        s5k5cagx_set_zoom_crop(state, ZPT_INPUT_SAMPLES, crop_real_left, crop_real_top, crop_real_width, crop_real_height);
        /* s5k5cagx_set_zoom_crop(state, ZPT_INPUT_SAMPLES, crop_real_left, crop_real_top, state->width, state->height); */
    }

    *__crop = crop->rect;
    return 0;
}

static int s5k5cagx_g_crop(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
    struct s5k5cagx_state *state = to_state(sd);
    struct v4l2_rect *__crop = _get_pad_crop(state, fh, crop->pad, crop->which);
    crop->rect = *__crop;
    return 0;
}

static int s5k5cagx_s_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *fmt)
{
    int err = 0;
    struct v4l2_mbus_framefmt *_fmt = &fmt->format;
    struct s5k5cagx_state *state = to_state(sd);

    /* printk("==========> %s\n", __func__); */
#if 0
    if (!state->inited) {
        pr_err("%s: device is not initialized!!!\n", __func__);
        return -EINVAL;
    }
#endif
    pr_debug("%s: %dx%d\n", __func__, _fmt->width, _fmt->height);
    state->width = _fmt->width;
    state->height = _fmt->height;
    printk("%s: mode %d, %dx%d\n", __func__, state->mode, state->width, state->height);
    /* err = s5k5cagx_set_frame_size(sd, state->mode, state->width, state->height); */
    return err;
}

static int s5k5cagx_set_af(struct v4l2_subdev *sd, int value)
{
    /* int i; */
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;

    pr_debug("%s: %d\n", __func__, value);

    if (value == 0) {
        // off
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_af_off, ARRAY_SIZE(s5k5cagx_reg_af_off));
        if (err < 0) {
            pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
            return err;
        }
    } else {
        // af do
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_af_do, ARRAY_SIZE(s5k5cagx_reg_af_do));
        if (err < 0) {
            pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
            return err;
        }

#if 0
        for (i = 0; i < 30; i++) {
            /* read AF status reg.*/
            err = i2c_read_reg(client, 0x7000, 0x26fe);

            if(err == 0x0000) {
                printk("Idle AF search \n");
                mdelay(100);
            } else if(err == 0x0001) {
                printk("AF search in progress\n");
                mdelay(100);
            } else if(err == 0x0002) {
                printk("AF search success\n");
                break;
            } else if(err == 0x0003) {
                printk("Low confidence position\n");
                break;
            } else if(err == 0x0004) {
                printk("AF search is cancelled\n");
                break;
            } else {
                break;
            }
        }
#endif
    }

    return 0;
}

enum {
    WB_AUTO = 0,
    WB_DAYLIGHT,
    WB_CLOUDY,
    WB_FLUORESCENT,
    WB_INCANDESCENT,
    WB_MAX
};

static int s5k5cagx_set_wb(struct v4l2_subdev *sd, int value)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct reg_val *reg_val;
    int size;

    if (unlikely(value < 0 || value >= WB_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case WB_AUTO:
        reg_val = s5k5cagx_reg_wb_auto;
        size = ARRAY_SIZE(s5k5cagx_reg_wb_auto);
        break;

    case WB_DAYLIGHT:
        reg_val = s5k5cagx_reg_wb_daylight;
        size = ARRAY_SIZE(s5k5cagx_reg_wb_daylight);
        break;

    case WB_CLOUDY:
        reg_val = s5k5cagx_reg_wb_cloudy;
        size = ARRAY_SIZE(s5k5cagx_reg_wb_cloudy);
        break;

    case WB_FLUORESCENT:
        reg_val = s5k5cagx_reg_wb_fluorescent;
        size = ARRAY_SIZE(s5k5cagx_reg_wb_fluorescent);
        break;

    case WB_INCANDESCENT:
        reg_val = s5k5cagx_reg_wb_incandescent;
        size = ARRAY_SIZE(s5k5cagx_reg_wb_incandescent);
        break;

    default:
        return 0;
    }

    err = s5k5cagx_write_regs(client, reg_val, size);
    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

enum {
    COLORFX_NONE = 0,
    COLORFX_SEPIA,
    COLORFX_AQUA,
    COLORFX_MONO,
    COLORFX_NEGATIVE,
    COLORFX_SKETCH,
    COLORFX_MAX
};

static int s5k5cagx_set_colorfx(struct v4l2_subdev *sd, int value)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < 0 || value >= COLORFX_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case COLORFX_NONE:
        s5k5cagx_reg_color_template[2].val = 0x0;
        break;

    case COLORFX_SEPIA:
        s5k5cagx_reg_color_template[2].val = 0x4;
        break;

    case COLORFX_AQUA:
        s5k5cagx_reg_color_template[2].val = 0x5;
        break;

    case COLORFX_MONO:
        s5k5cagx_reg_color_template[2].val = 0x1;
        break;

    case COLORFX_NEGATIVE:
        s5k5cagx_reg_color_template[2].val = 0x3;
        break;

    case COLORFX_SKETCH:
        s5k5cagx_reg_color_template[2].val = 0x6;
        break;

    default:
        return 0;
    }

    err = s5k5cagx_write_regs(client, s5k5cagx_reg_color_template, ARRAY_SIZE(s5k5cagx_reg_color_template));
    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

#define MIN_EXPOSURE    -4
#define MAX_EXPOSURE     4
static int s5k5cagx_set_exposure(struct v4l2_subdev *sd, int value)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < MIN_EXPOSURE || value > MAX_EXPOSURE)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case -4:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_m_4, ARRAY_SIZE(s5k5cagx_reg_brightness_m_4));
        break;

    case -3:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_m_3, ARRAY_SIZE(s5k5cagx_reg_brightness_m_3));
        break;

    case -2:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_m_2, ARRAY_SIZE(s5k5cagx_reg_brightness_m_2));
        break;

    case -1:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_m_1, ARRAY_SIZE(s5k5cagx_reg_brightness_m_1));
        break;

    case 0:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_0, ARRAY_SIZE(s5k5cagx_reg_brightness_0));
        break;

    case 1:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_p_1, ARRAY_SIZE(s5k5cagx_reg_brightness_p_1));
        break;

    case 2:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_p_2, ARRAY_SIZE(s5k5cagx_reg_brightness_p_2));
        break;

    case 3:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_p_3, ARRAY_SIZE(s5k5cagx_reg_brightness_p_3));
        break;

    case 4:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_brightness_p_4, ARRAY_SIZE(s5k5cagx_reg_brightness_p_4));
        break;

    default:
        return 0;
    }

    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

enum {
    SCENE_OFF = 0,
    SCENE_PORTRAIT,
    SCENE_LANDSCAPE,
    SCENE_SPORTS,
    SCENE_NIGHTSHOT,
    SCENE_MAX
};

static int s5k5cagx_set_scene(struct v4l2_subdev *sd, int value)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < SCENE_OFF || value >= SCENE_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    printk("%s: %d\n", __func__, value);

    switch (value) {
    case SCENE_OFF:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_scene_off, ARRAY_SIZE(s5k5cagx_reg_scene_off));
        break;

    case SCENE_PORTRAIT:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_scene_portrait, ARRAY_SIZE(s5k5cagx_reg_scene_portrait));
        break;

    case SCENE_LANDSCAPE:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_scene_landscape, ARRAY_SIZE(s5k5cagx_reg_scene_landscape));
        break;

    case SCENE_SPORTS:
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_scene_sports, ARRAY_SIZE(s5k5cagx_reg_scene_sports));
        break;

    case SCENE_NIGHTSHOT:
        //err = s5k5cagx_write_regs(client, s5k5cagx_reg_scene_nightshot, ARRAY_SIZE(s5k5cagx_reg_scene_nightshot));
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_scene_nightshot, ARRAY_SIZE(s5k5cagx_reg_scene_sports));
        break;

    default:
        return 0;
    }

    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

enum {
    ANTI_SHAKE_OFF = 0,
    ANTI_SHAKE_50Hz,
    ANTI_SHAKE_60Hz,
    ANTI_SHAKE_MAX
};

static int s5k5cagx_set_antishake(struct v4l2_subdev *sd, int value)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < ANTI_SHAKE_50Hz || value >= ANTI_SHAKE_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case ANTI_SHAKE_OFF:
        s5k5cagx_reg_AntiBanding_template[4].val = 0;
        break;

    case ANTI_SHAKE_50Hz:
        s5k5cagx_reg_AntiBanding_template[4].val = 1;
        break;

    case ANTI_SHAKE_60Hz:
        s5k5cagx_reg_AntiBanding_template[4].val = 2;
        break;

    default:
        return 0;
    }

    err = s5k5cagx_write_regs(client, s5k5cagx_reg_AntiBanding_template, ARRAY_SIZE(s5k5cagx_reg_AntiBanding_template));
    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

static int s5k5cagx_mode_change(struct v4l2_subdev *sd, int value)
{
    struct s5k5cagx_state *state = to_state(sd);
    int err = 0;

    printk("%s: mode %d\n", __func__, value);

    if (unlikely(value < PREVIEW_MODE || value > CAPTURE_MODE)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    if (state->mode != value) {
        state->mode = value;
        err = s5k5cagx_set_frame_size(sd, value, state->width, state->height);
    }

    return err;
}

#define V4L2_CID_CAMERA_SCENE_MODE		(V4L2_CTRL_CLASS_CAMERA | 0x1001)
#define V4L2_CID_CAMERA_ANTI_SHAKE		(V4L2_CTRL_CLASS_CAMERA | 0x1002)
#define V4L2_CID_CAMERA_MODE_CHANGE		(V4L2_CTRL_CLASS_CAMERA | 0x1003)

static int s5k5cagx_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
    int value = ctrl->val;
    int err = 0;

    /* printk("%s: id(0x%x), value(%d)\n", __func__, ctrl->id, value); */

    switch (ctrl->id) {
        case V4L2_CID_FOCUS_AUTO:
            err = s5k5cagx_set_af(sd, value);
            break;

        case V4L2_CID_DO_WHITE_BALANCE:
            err = s5k5cagx_set_wb(sd, value);
            break;

        case V4L2_CID_COLORFX:
            err = s5k5cagx_set_colorfx(sd, value);
            break;

        case V4L2_CID_EXPOSURE:
            err = s5k5cagx_set_exposure(sd, value);
            break;

        /* custom */
        case V4L2_CID_CAMERA_SCENE_MODE:
            err = s5k5cagx_set_scene(sd, value);
            break;

        case V4L2_CID_CAMERA_ANTI_SHAKE:
            err = s5k5cagx_set_antishake(sd, value);
            break;

        case V4L2_CID_CAMERA_MODE_CHANGE:
            err = s5k5cagx_mode_change(sd, value);
            break;

        default:
            pr_err("%s: no such control\n", __func__);
            break;
    }

    return err;
}

static const struct v4l2_ctrl_ops s5k5cagx_ctrl_ops = {
    .s_ctrl = s5k5cagx_s_ctrl,
};

static const struct v4l2_ctrl_config s5k5cagx_custom_ctrls[] = {
    {
        .ops    = &s5k5cagx_ctrl_ops,
        .id     = V4L2_CID_CAMERA_SCENE_MODE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "SceneMode",
        .min    = 0,
        .max    = SCENE_MAX - 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &s5k5cagx_ctrl_ops,
        .id     = V4L2_CID_CAMERA_ANTI_SHAKE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "AntiShake",
        .min    = 0,
        .max    = ANTI_SHAKE_MAX - 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &s5k5cagx_ctrl_ops,
        .id     = V4L2_CID_CAMERA_MODE_CHANGE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "ModeChange",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    },
};

static int s5k5cagx_initialize_ctrls(struct s5k5cagx_state *state)
{
    v4l2_ctrl_handler_init(&state->handler, NUM_CTRLS);

    /* standard */
    state->focus = v4l2_ctrl_new_std(&state->handler, &s5k5cagx_ctrl_ops,
            V4L2_CID_FOCUS_AUTO, 0, 1, 1, 0);
    if (!state->focus) {
        pr_err("%s: failed to create focus ctrl\n", __func__);
        return -1;
    }
    state->wb = v4l2_ctrl_new_std(&state->handler, &s5k5cagx_ctrl_ops,
            V4L2_CID_DO_WHITE_BALANCE, WB_AUTO, WB_MAX - 1, 1, WB_AUTO);
    if (!state->wb) {
        pr_err("%s: failed to create wb ctrl\n", __func__);
        return -1;
    }
    state->color_effect = v4l2_ctrl_new_std_menu(&state->handler, &s5k5cagx_ctrl_ops,
            V4L2_CID_COLORFX, COLORFX_MAX - 1, 0, COLORFX_NONE);
    if (!state->color_effect) {
        pr_err("%s: failed to create color_effect ctrl\n", __func__);
        return -1;
    }
    state->exposure = v4l2_ctrl_new_std_menu(&state->handler, &s5k5cagx_ctrl_ops,
            V4L2_CID_EXPOSURE, MAX_EXPOSURE, 0, 0);
    if (!state->exposure) {
        pr_err("%s: failed to create exposure ctrl\n", __func__);
        return -1;
    }

    /* custom */
    state->scene_mode = v4l2_ctrl_new_custom(&state->handler, &s5k5cagx_custom_ctrls[0], NULL);
    if (!state->scene_mode) {
        pr_err("%s: failed to create scene_mode ctrl\n", __func__);
        return -1;
    }
    state->anti_shake = v4l2_ctrl_new_custom(&state->handler, &s5k5cagx_custom_ctrls[1], NULL);
    if (!state->anti_shake) {
        pr_err("%s: failed to create anti_shake ctrl\n", __func__);
        return -1;
    }
    state->mode_change = v4l2_ctrl_new_custom(&state->handler, &s5k5cagx_custom_ctrls[2], NULL);
    if (!state->mode_change) {
        pr_err("%s: failed to create mode_change ctrl\n", __func__);
        return -1;
    }

    state->sd.ctrl_handler = &state->handler;
    if (state->handler.error) {
        printk("%s: ctrl handler error(%d)\n", __func__, state->handler.error);
        v4l2_ctrl_handler_free(&state->handler);
        return -1;
    }

    return 0;
}


static int s5k5cagx_connected_check(struct i2c_client *client)
{
#if 1
    if (read_device_id_for_s5k5cagx(client) == 0x5ca)
        return 0;
    return -1;
#else
    u32 id;

    while (1) {
        id = read_device_id_for_s5k5cagx(client);
        if (id != 0x5ca)
            msleep(10);
        else
            break;
    }

    return 0;
#endif
}

#ifdef USE_INITIAL_WORKER_THREAD
static void s5k5cagx_init_work(struct work_struct *work)
{
    struct s5k5cagx_state *state = container_of(work, struct s5k5cagx_state, init_work);
    struct i2c_client *client = v4l2_get_subdevdata(&state->sd);
    int ret = s5k5cagx_write_regs(client, s5k5cagx_reg_init, ARRAY_SIZE(s5k5cagx_reg_init));
    if (ret < 0)
        printk(KERN_ERR "%s: failed to s5k5cagx_reg_init!!!\n", __func__);
    printk("%s success!!!\n", __func__);
    state->inited = true;
}
#endif

// psw0523 for debugging sleep
#include <mach/platform.h>
static int s5k5cagx_init(struct v4l2_subdev *sd, u32 val)
{
#ifndef USE_INITIAL_WORKER_THREAD
    int err = 0;
#endif
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct s5k5cagx_state *state = to_state(sd);

    if (!state->inited) {
        PM_DBGOUT("s5k5cagx_init\n");
        if(s5k5cagx_connected_check(client) < 0) {
            v4l_info(client, "%s: camera not connected..\n", __func__);
            return -1;
        }
        PM_DBGOUT("%s 1\n", __func__);
#ifndef USE_INITIAL_WORKER_THREAD
        err = s5k5cagx_write_regs(client, s5k5cagx_reg_init, ARRAY_SIZE(s5k5cagx_reg_init));
        if (err < 0) {
            pr_err("%s: write reg error(err: %d)\n", __func__, err);
            return err;
        }
        PM_DBGOUT("%s 2\n", __func__);
        state->inited = true;
        s5k5cagx_set_frame_size(sd, state->mode, state->width, state->height);
        PM_DBGOUT("%s 3\n", __func__);
#else
        printk("queue_work ---> s5k5cagx_init_work\n");
        queue_work(state->init_wq, &state->init_work);
#endif
    }

    return 0;
}

static int s5k5cagx_s_power(struct v4l2_subdev *sd, int on)
{
    PM_DBGOUT("%s: on %d\n", __func__, on);
#if 0
    if (on)
        s5k5cagx_init(sd, 0);
    else {
        struct s5k5cagx_state *state = to_state(sd);
        state->inited = false;
    }
#else
    if (!on) {
        struct s5k5cagx_state *state = to_state(sd);
        state->inited = false;
    }
#endif
    return 0;
}

static int s5k5cagx_s_stream(struct v4l2_subdev *sd, int enable)
{
    printk("%s %d\n", __func__, enable);
    if (enable) {
        struct s5k5cagx_state *state = to_state(sd);
        s5k5cagx_init(sd, enable);
        s5k5cagx_set_frame_size(sd, state->mode, state->width, state->height);
    } else {
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        s5k5cagx_write_regs(client, s5k5cagx_reg_disable, ARRAY_SIZE(s5k5cagx_reg_disable));
    }

    return 0;
}

static const struct v4l2_subdev_core_ops s5k5cagx_core_ops = {
    .s_power = s5k5cagx_s_power,
    .s_ctrl = v4l2_subdev_s_ctrl,
};

static const struct v4l2_subdev_pad_ops s5k5cagx_pad_ops = {
    .set_fmt  = s5k5cagx_s_fmt,
    .set_crop = s5k5cagx_s_crop,
    .get_crop = s5k5cagx_g_crop,
};

static const struct v4l2_subdev_video_ops s5k5cagx_video_ops = {
    .s_stream = s5k5cagx_s_stream,
};

static const struct v4l2_subdev_ops s5k5cagx_ops = {
    .core = &s5k5cagx_core_ops,
    .video = &s5k5cagx_video_ops,
    .pad = &s5k5cagx_pad_ops,
};

/**
 * media_entity_operations
 */
static int _link_setup(struct media_entity *entity,
        const struct media_pad *local,
        const struct media_pad *remote, u32 flags)
{
    /* printk("%s: entered\n", __func__); */
    return 0;
}

static const struct media_entity_operations s5k5cagx_media_ops = {
    .link_setup = _link_setup,
};
/*
 * s5k5cagx_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int s5k5cagx_probe(struct i2c_client *client,
        const struct i2c_device_id *id)
{
    struct s5k5cagx_state *state;
    struct v4l2_subdev *sd;
    int ret;

    state = kzalloc(sizeof(struct s5k5cagx_state), GFP_KERNEL);
    if (state == NULL)
        return -ENOMEM;

    sd = &state->sd;
    strcpy(sd->name, S5K5CAGX_DRIVER_NAME);

    /* Registering subdev */
    v4l2_i2c_subdev_init(sd, client, &s5k5cagx_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    state->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    sd->entity.ops = &s5k5cagx_media_ops;
    if (media_entity_init(&sd->entity, 1, &state->pad, 0)) {
        dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
        kfree(state);
        return -ENOENT;
    }

    ret = s5k5cagx_initialize_ctrls(state);
    if (ret < 0) {
        pr_err("%s: failed to initialize controls\n", __func__);
        kfree(state);
        return ret;
    }

#ifdef USE_INITIAL_WORKER_THREAD
    state->init_wq = create_singlethread_workqueue("s5k5cagx-init");
    if (!state->init_wq) {
        pr_err("%s: error create work queue for init\n", __func__);
        return -ENOMEM;
    }
    INIT_WORK(&state->init_work, s5k5cagx_init_work);
#endif

    dev_info(&client->dev, "s5k5cagx has been probed\n");

    return 0;
}


static int s5k5cagx_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);

    v4l2_device_unregister_subdev(sd);
    v4l2_ctrl_handler_free(sd->ctrl_handler);
    media_entity_cleanup(&sd->entity);
    kfree(to_state(sd));
    return 0;
}

static const struct i2c_device_id s5k5cagx_id[] = {
    { S5K5CAGX_DRIVER_NAME, 0 },
    { },
};

static int read_device_id_for_s5k5cagx(struct i2c_client *client)
{
    int id;
    client->addr = ADDR_S5K5CAGX;
    id = i2c_read_reg(client,0x0, 0x40);
    v4l_info(client,"Check for s5k5cagx \n");
    v4l_info(client,"Chip ID 0x00000040 :0x%x \n", id);
    v4l_info(client,"Chip Revision  0x00000042 :0x%x \n",	i2c_read_reg(client,0x0, 0x42));
    v4l_info(client,"FW version control revision  0x00000048 :%d \n",i2c_read_reg(client,0x0, 0x48));
    v4l_info(client,"FW compilation date(0xYMDD) 0x0000004e :0x%x \n",i2c_read_reg(client,0x0, 0x4e));

    if(id == 0x5ca) return id;

    return -1;
}


MODULE_DEVICE_TABLE(i2c, s5k5cagx_id);

static struct i2c_driver _i2c_driver = {
    .driver = {
        .name = S5K5CAGX_DRIVER_NAME,
    },
    .probe    = s5k5cagx_probe,
    .remove   = s5k5cagx_remove,
    .id_table = s5k5cagx_id,
};

module_i2c_driver(_i2c_driver);

MODULE_DESCRIPTION("Samsung Electronics S5K5CAGX UXGA camera driver");
MODULE_AUTHOR("Jinsung Yang <jsgood.yang@samsung.com>");
MODULE_LICENSE("GPL");
