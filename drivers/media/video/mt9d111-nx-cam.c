/* linux/drivers/media/video/mt9d111.c
 *
 *
 * Driver for MT9D111 from Micron
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
// psw0523 for debugging sleep
#include <mach/platform.h>
#include <mach/s5p6818.h>
#include "mt9d111-nx-preset.h"


/* psw0523 add for initializing worker thread */
/* #define USE_INITIAL_WORKER_THREAD */

#ifdef USE_INITIAL_WORKER_THREAD
#include <linux/workqueue.h>
#endif

#define MT9D111_CAM_DRIVER_NAME	"MT9D111"

#if 0//defined(CONFIG_I2C_NXP_PORT1_GPIO_MODE)||defined(CONFIG_I2C_NXP_PORT2_GPIO_MODE)
static int mt9d111_i2c_read_word(const struct i2c_client *client, u8 reg, uint16_t *val);
#endif

static inline struct mt9d111_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct mt9d111_state, sd);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct mt9d111_state, handler)->sd;
}

static struct v4l2_rect *_get_pad_crop(struct mt9d111_state *me, struct v4l2_subdev_fh *fh,
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

#if 0//defined(CONFIG_I2C_NXP_PORT1_GPIO_MODE)||defined(CONFIG_I2C_NXP_PORT2_GPIO_MODE)

static int mt9d111_i2c_write_word(const struct i2c_client *client, u8 reg, uint16_t val)
{
	int i = 0;
	int ret = 0;
	uint16_t temp = 0x0;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		//v4l_info(client, "mt9d111_i2c_write_word reg:0x%02x val:0x%04x \n", reg, val);
		temp = ((val&0xff00)>>8)|((val&0x00ff)<<8);
		ret = i2c_smbus_write_word_data(client, reg, temp);
		if (ret < 0)
		{
			uint16_t val1=0;
			dev_err(&client->dev, "\e[31mfailed write_word reg:0x%02x val:0x%04x, cnt:%d\e[0m\n", reg, val, i);
			mdelay(1);
			//mt9d111_i2c_read_word(client, reg, &val1);
			//dev_err(&client->dev, "\e[31mread reg:0x%02x val:0x%04x\e[0m\n", reg, val1);
		}
		else
		{
			//uint16_t val1=0;
			//mdelay(1);
			//mt9d111_i2c_read_word(client, reg, &val1);
			//if(val != val1)
			//	dev_err(&client->dev, "\e[31mread reg:0x%02x val:0x%04x, readval1:%d\e[0m\n", reg, val, val1);
			break;
		}
	}

	return ret;
}

static int mt9d111_i2c_read_word(const struct i2c_client *client, u8 reg, uint16_t *val)
{
	int i = 0;
	int ret = 0;
	uint16_t temp = 0x0;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_smbus_read_word_data(client, reg);
		if (ret < 0)
		{
			dev_err(&client->dev, "\e[31mread_word failed reg:0x%02x, cnt:%d\e[0m\n", reg, i);
			mdelay(1);
		}
		else
		{
			temp = ((ret&0xff00)>>8)|((ret&0x00ff)<<8);
			*val = (uint16_t)temp;
			break;
		}
	}

	return ret;
}

#else
static int mt9d111_i2c_read_word(struct i2c_client *client, u8 subaddr, u16 *data)
{
	int err;
	unsigned char buf[2];
	struct i2c_msg msg[2];

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &subaddr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = buf;

	err = i2c_transfer(client->adapter, msg, 2);

	if (unlikely(err != 2))
	{
		dev_err(&client->dev, "\e[31mread_word failed reg:0x%02x \e[0m\n", subaddr);
		return -EIO;
	}

	*data = ((buf[0] << 8) | buf[1]);

	return 0;
}

static int mt9d111_i2c_write_word(struct i2c_client *client, u8 addr, u16 w_data)
{
	int i = 0;
	int ret = 0;
	uint16_t val=0;
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 3, buf};

	buf[0] = addr;
	buf[1] = w_data >> 8;
	buf[2] = w_data & 0xff;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		mdelay(50);
		//dev_err(&client->dev, "\e[31mfailed write_word reg:0x%02x write:0x%04x, read:0x%04x, retry:%d\e[0m\n", addr, w_data, val, i);
	}

	if (ret != 1) {
		mt9d111_i2c_read_word(client, addr, &val);
		dev_err(&client->dev, "\e[31mfailed write_word reg:0x%02x write:0x%04x, read:0x%04x, retry:%d\e[0m\n", addr, w_data, val, i);
		return -EIO;
	}

	return 0;
}

#endif

static int mt9d111_write_regs(struct i2c_client *client, struct reg_val regvals[], int size)
{
	//unsigned char tmp[4];
	int i;
	s32 err = 0;
	struct reg_val *regval;

	for (i = 0; i < size ; i++) {
		regval = &regvals[i];
		if(regval->page == I2C_TERM)
			break;
		else if (regval->page == I2C_DELAY)
			mdelay(regval->val);
		else {
			err = mt9d111_i2c_write_word(client, regval->reg, regval->val);
#if 0//def DBG_I2C_READ
			{
				u8 val[2];
				if (tmp[0] != 0xF0) {
					err = mt9d111_i2c_read_block(client, regval->reg, 2, val);
					if (err < 0)
						v4l_info(client,"\e[31m%s: register set failed\e[0m(i2c_recv{0x%02X, \e[31m0x%02X%02X\e[0m})\n", __func__, regval->reg, val[0], val[1]);
					//printk(KERN_ERR"0x%02X: 0x%04X = 0x%02X%02X", regval->reg, regval->val, val[0], val[1]);

					if ((tmp[1] != val[0]) || (tmp[2] != val[1])) {
						if (tmp[0] == 0x0D)
							v4l_info(client,"\e[31mmt9d111 Soft reset \e[0m \n");
						else if (tmp[0] == 0xC3)
							v4l_info(client,"\e[31mmt9d111 MCU reset \e[0m \n");
					}

				}
			}
#endif
		}
	}

	return err;
}

static int mt9d111_set_frame_size(struct v4l2_subdev *sd, int mode, int width, int height)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	//struct reg_val *array;
	//int size;

	v4l_info(client,"%s: mode %d, width %d, height %d\n", __func__, mode, width, height);
	if (mode == PREVIEW_MODE) {
		//array = mt9d111_reg_preview;
		//size = ARRAY_SIZE(mt9d111_reg_preview);
	} else {
		//array = mt9d111_reg_capture;
		//size = ARRAY_SIZE(mt9d111_reg_capture);
	}
	//array[3].val = width;
	//array[4].val = height;


	return 0;//mt9d111_write_regs(client, array, size);
}

static int mt9d111_set_zoom_crop(struct mt9d111_state *state, int zoom_type, int left, int top, int width, int height)
{
	//struct i2c_client *client = v4l2_get_subdevdata(&state->sd);
	//struct reg_val *array = mt9d111_reg_zoom_template;
	//int read_val;

	//array[3].val = zoom_type;
	//array[5].val = width;
	//array[6].val = height;
	//array[7].val = left;
	//array[8].val = top;

#if 0// PJSIN 20140212 add-- [ 1
	mt9d111_write_regs(client, array, ARRAY_SIZE(mt9d111_reg_zoom_template));

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
#endif// ]-- end

	return 0;
}

static int mt9d111_set_crop(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
	struct mt9d111_state *state = to_state(sd);
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
			mt9d111_set_zoom_crop(state, ZPT_IDLE, 0, 0, 0, 0);
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
		mt9d111_set_zoom_crop(state, ZPT_INPUT_SAMPLES, crop_real_left, crop_real_top, crop_real_width, crop_real_height);
		/* mt9d111_set_zoom_crop(state, ZPT_INPUT_SAMPLES, crop_real_left, crop_real_top, state->width, state->height); */
	}

	*__crop = crop->rect;
	return 0;
}

static int mt9d111_get_crop(struct v4l2_subdev *sd,
									struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
	struct mt9d111_state *state = to_state(sd);
	struct v4l2_rect *__crop = _get_pad_crop(state, fh, crop->pad, crop->which);
	crop->rect = *__crop;
	return 0;
}

static int mt9d111_set_fmt(struct v4l2_subdev *sd,
								struct v4l2_subdev_fh *fh, struct v4l2_subdev_format *fmt)
{
	int err = 0;
	struct v4l2_mbus_framefmt *_fmt = &fmt->format;
	struct mt9d111_state *state = to_state(sd);

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
	/* err = mt9d111_set_frame_size(sd, state->mode, state->width, state->height); */
	return err;
}

static int mt9d111_set_af(struct v4l2_subdev *sd, int value)
{
	/* int i; */
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	pr_debug("%s: %d\n", __func__, value);

	if (value == 0) {
		// off
		err = 0;//mt9d111_write_regs(client, mt9d111_reg_af_off, ARRAY_SIZE(mt9d111_reg_af_off));
		if (err < 0) {
			pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
			return err;
		}
	} else {
		// af do
		err = 0;//mt9d111_write_regs(client, mt9d111_reg_af_do, ARRAY_SIZE(mt9d111_reg_af_do));
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



static int mt9d111_set_wb(struct v4l2_subdev *sd, int value)
{
    int err;
    //struct i2c_client *client = v4l2_get_subdevdata(sd);
    //struct reg_val *reg_val;
    //int size;

    if (unlikely(value < 0 || value >= WB_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case WB_AUTO:
        //reg_val = mt9d111_reg_wb_auto;
        //size = ARRAY_SIZE(mt9d111_reg_wb_auto);
        break;

    case WB_DAYLIGHT:
        //reg_val = mt9d111_reg_wb_daylight;
        //size = ARRAY_SIZE(mt9d111_reg_wb_daylight);
        break;

    case WB_CLOUDY:
        //reg_val = mt9d111_reg_wb_cloudy;
        //size = ARRAY_SIZE(mt9d111_reg_wb_cloudy);
        break;

    case WB_FLUORESCENT:
        //reg_val = mt9d111_reg_wb_fluorescent;
        //size = ARRAY_SIZE(mt9d111_reg_wb_fluorescent);
        break;

    case WB_INCANDESCENT:
        //reg_val = mt9d111_reg_wb_incandescent;
        //size = ARRAY_SIZE(mt9d111_reg_wb_incandescent);
        break;

    default:
        return 0;
    }

    err = 0;//mt9d111_write_regs(client, reg_val, size);
    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}



static int mt9d111_set_colorfx(struct v4l2_subdev *sd, int value)
{
    int err;
    //struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < 0 || value >= COLORFX_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case COLORFX_NONE:
        //mt9d111_reg_color_template[2].val = 0x0;
        break;

    case COLORFX_SEPIA:
        //mt9d111_reg_color_template[2].val = 0x4;
        break;

    case COLORFX_AQUA:
        //mt9d111_reg_color_template[2].val = 0x5;
        break;

    case COLORFX_MONO:
        //mt9d111_reg_color_template[2].val = 0x1;
        break;

    case COLORFX_NEGATIVE:
        //mt9d111_reg_color_template[2].val = 0x3;
        break;

    case COLORFX_SKETCH:
        //mt9d111_reg_color_template[2].val = 0x6;
        break;

    default:
        return 0;
    }

    err = 0;//mt9d111_write_regs(client, mt9d111_reg_color_template, ARRAY_SIZE(mt9d111_reg_color_template));
    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

static int mt9d111_set_exposure(struct v4l2_subdev *sd, int value)
{
    int err;
    //struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < MIN_EXPOSURE || value > MAX_EXPOSURE)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case -4:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_m_4, ARRAY_SIZE(mt9d111_reg_brightness_m_4));
        break;

    case -3:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_m_3, ARRAY_SIZE(mt9d111_reg_brightness_m_3));
        break;

    case -2:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_m_2, ARRAY_SIZE(mt9d111_reg_brightness_m_2));
        break;

    case -1:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_m_1, ARRAY_SIZE(mt9d111_reg_brightness_m_1));
        break;

    case 0:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_0, ARRAY_SIZE(mt9d111_reg_brightness_0));
        break;

    case 1:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_p_1, ARRAY_SIZE(mt9d111_reg_brightness_p_1));
        break;

    case 2:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_p_2, ARRAY_SIZE(mt9d111_reg_brightness_p_2));
        break;

    case 3:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_p_3, ARRAY_SIZE(mt9d111_reg_brightness_p_3));
        break;

    case 4:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_brightness_p_4, ARRAY_SIZE(mt9d111_reg_brightness_p_4));
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


static int mt9d111_set_scene(struct v4l2_subdev *sd, int value)
{
    int err;
    //struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < SCENE_OFF || value >= SCENE_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    printk("%s: %d\n", __func__, value);

    switch (value) {
    case SCENE_OFF:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_scene_off, ARRAY_SIZE(mt9d111_reg_scene_off));
        break;

    case SCENE_PORTRAIT:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_scene_portrait, ARRAY_SIZE(mt9d111_reg_scene_portrait));
        break;

    case SCENE_LANDSCAPE:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_scene_landscape, ARRAY_SIZE(mt9d111_reg_scene_landscape));
        break;

    case SCENE_SPORTS:
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_scene_sports, ARRAY_SIZE(mt9d111_reg_scene_sports));
        break;

    case SCENE_NIGHTSHOT:
        //err = mt9d111_write_regs(client, mt9d111_reg_scene_nightshot, ARRAY_SIZE(mt9d111_reg_scene_nightshot));
        err = 0;//mt9d111_write_regs(client, mt9d111_reg_scene_nightshot, ARRAY_SIZE(mt9d111_reg_scene_sports));
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



static int mt9d111_set_antishake(struct v4l2_subdev *sd, int value)
{
    int err;
    //struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (unlikely(value < ANTI_SHAKE_50Hz || value >= ANTI_SHAKE_MAX)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    switch (value) {
    case ANTI_SHAKE_OFF:
        //mt9d111_reg_AntiBanding_template[4].val = 0;
        break;

    case ANTI_SHAKE_50Hz:
        //mt9d111_reg_AntiBanding_template[4].val = 1;
        break;

    case ANTI_SHAKE_60Hz:
        //mt9d111_reg_AntiBanding_template[4].val = 2;
        break;

    default:
        return 0;
    }

    err = 0;//mt9d111_write_regs(client, mt9d111_reg_AntiBanding_template, ARRAY_SIZE(mt9d111_reg_AntiBanding_template));
    if (err < 0) {
        pr_err("%s: failed to write regs(err: %d)\n", __func__, err);
        return err;
    }

    return 0;
}

static int mt9d111_mode_change(struct v4l2_subdev *sd, int value)
{
    struct mt9d111_state *state = to_state(sd);
    int err = 0;

    printk("%s: mode %d\n", __func__, value);

    if (unlikely(value < PREVIEW_MODE || value > CAPTURE_MODE)) {
        pr_err("%s: invalid value(%d)\n", __func__, value);
        return -EINVAL;
    }

    if (state->mode != value) {
        state->mode = value;
        err = mt9d111_set_frame_size(sd, value, state->width, state->height);
    }

    return err;
}


static int mt9d111_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    //int value = ctrl->val;
    int err = 0;

	v4l_info(client,"%s: ctrl->id:%d\n", __func__, ctrl->id);

    switch (ctrl->id) {
        case V4L2_CID_FOCUS_AUTO:
            //err = mt9d111_set_af(sd, value);
            break;

        case V4L2_CID_DO_WHITE_BALANCE:
            //err = mt9d111_set_wb(sd, value);
            break;

        case V4L2_CID_COLORFX:
            //err = mt9d111_set_colorfx(sd, value);
            break;

        case V4L2_CID_EXPOSURE:
            //err = mt9d111_set_exposure(sd, value);
            break;

        /* custom */
        case V4L2_CID_CAMERA_SCENE_MODE:
            //err = mt9d111_set_scene(sd, value);
            break;

        case V4L2_CID_CAMERA_ANTI_SHAKE:
            //err = mt9d111_set_antishake(sd, value);
            break;

        case V4L2_CID_CAMERA_MODE_CHANGE:
            //err = mt9d111_mode_change(sd, value);
            break;

        default:
            pr_err("%s: no such control\n", __func__);
            break;
    }

    return err;
}

static const struct v4l2_ctrl_ops mt9d111_ctrl_ops = {
	.s_ctrl = mt9d111_s_ctrl,
};

static const struct v4l2_ctrl_config mt9d111_custom_ctrls[] = {
    {
        .ops    = &mt9d111_ctrl_ops,
        .id     = V4L2_CID_CAMERA_SCENE_MODE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "SceneMode",
        .min    = 0,
        .max    = SCENE_MAX - 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &mt9d111_ctrl_ops,
        .id     = V4L2_CID_CAMERA_ANTI_SHAKE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "AntiShake",
        .min    = 0,
        .max    = ANTI_SHAKE_MAX - 1,
        .def    = 0,
        .step   = 1,
    }, {
        .ops    = &mt9d111_ctrl_ops,
        .id     = V4L2_CID_CAMERA_MODE_CHANGE,
        .type   = V4L2_CTRL_TYPE_INTEGER,
        .name   = "ModeChange",
        .min    = 0,
        .max    = 1,
        .def    = 0,
        .step   = 1,
    },
};

static int mt9d111_initialize_ctrls(struct mt9d111_state *state)
{
    v4l2_ctrl_handler_init(&state->handler, NUM_CTRLS);

    /* standard */
    state->focus = v4l2_ctrl_new_std(&state->handler, &mt9d111_ctrl_ops,
            V4L2_CID_FOCUS_AUTO, 0, 1, 1, 0);
    if (!state->focus) {
        pr_err("%s: failed to create focus ctrl\n", __func__);
        return -1;
    }
    state->wb = v4l2_ctrl_new_std(&state->handler, &mt9d111_ctrl_ops,
            V4L2_CID_DO_WHITE_BALANCE, WB_AUTO, WB_MAX - 1, 1, WB_AUTO);
    if (!state->wb) {
        pr_err("%s: failed to create wb ctrl\n", __func__);
        return -1;
    }
    state->color_effect = v4l2_ctrl_new_std_menu(&state->handler, &mt9d111_ctrl_ops,
            V4L2_CID_COLORFX, COLORFX_MAX - 1, 0, COLORFX_NONE);
    if (!state->color_effect) {
        pr_err("%s: failed to create color_effect ctrl\n", __func__);
        return -1;
    }
    state->exposure = v4l2_ctrl_new_std_menu(&state->handler, &mt9d111_ctrl_ops,
            V4L2_CID_EXPOSURE, MAX_EXPOSURE, 0, 0);
    if (!state->exposure) {
        pr_err("%s: failed to create exposure ctrl\n", __func__);
        return -1;
    }

    /* custom */
    state->scene_mode = v4l2_ctrl_new_custom(&state->handler, &mt9d111_custom_ctrls[0], NULL);
    if (!state->scene_mode) {
        pr_err("%s: failed to create scene_mode ctrl\n", __func__);
        return -1;
    }
    state->anti_shake = v4l2_ctrl_new_custom(&state->handler, &mt9d111_custom_ctrls[1], NULL);
    if (!state->anti_shake) {
        pr_err("%s: failed to create anti_shake ctrl\n", __func__);
        return -1;
    }
    state->mode_change = v4l2_ctrl_new_custom(&state->handler, &mt9d111_custom_ctrls[2], NULL);
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


static int mt9d111_connected_check(struct i2c_client *client)
{
	uint16_t val=0;
	uint16_t val1=0;

#if 0// test register
	mt9d111_i2c_read_word(client, 0x65, &val);

	mt9d111_i2c_write_word(client, 0xf0, 0x0000);
	mt9d111_i2c_write_word(client, 0x65, 0xA000);

	mt9d111_i2c_read_word(client, 0x65, &val1);
	v4l_info(client,"#  Reg 0x65 : before:0x%04x, after:0x%04x\n", val, val1);

	mt9d111_i2c_write_word(client, 0xf0, 0x0000);
	mt9d111_i2c_write_word(client, 0x65, 0xE000);
#endif// ]-- end
	mt9d111_i2c_write_word(client, 0xf0, 0x0000);
	mdelay(5);

	mt9d111_i2c_read_word(client, 0x00, &val);
	mdelay(5);

	mt9d111_i2c_read_word(client, 0xFF, &val1);
	mdelay(5);

	v4l_info(client,"################################## \n");
	v4l_info(client,"#  Check for mt9d111 \n");
	v4l_info(client,"#  Read ID : 0x00:0x%04x, 0x%04x \n", val, val1);
	v4l_info(client,"################################## \n");

	v4l_info(client,"value 0x%4X or 0x%4X\n", val, val1);

	if(val != 0x1519 || val1 != 0x1519 )
	{

		v4l_info(client,"Doesn't match value 0x%4X or 0x%4X\n", val, val1);

		return -1;
	}
	return 0;
}

#ifdef USE_INITIAL_WORKER_THREAD
static void mt9d111_init_work(struct work_struct *work)
{
	struct mt9d111_state *state = container_of(work, struct mt9d111_state, init_work);
	struct i2c_client *client = v4l2_get_subdevdata(&state->sd);
	int ret = 0;//mt9d111_write_regs(client, mt9d111_reg_init, ARRAY_SIZE(mt9d111_reg_init));
	if (ret < 0)
		printk(KERN_ERR "%s: failed to mt9d111_reg_init!!!\n", __func__);
	printk("%s success!!!\n", __func__);
	state->inited = true;
}
#endif

static int mt9d111_init(struct v4l2_subdev *sd, u32 val)
{
#ifndef USE_INITIAL_WORKER_THREAD
	int err = 0;
#endif
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9d111_state *state = to_state(sd);
	//uint16_t value=0;

	u64 start_time;
	u64 end_time;
	u64 delta_jiffies;
	unsigned int time_msec;

	if (!state->inited) {
		v4l_info(client,"mt9d111_init\n");

		printk(KERN_INFO "mt9d111_init\n");

		start_time = get_jiffies_64(); /* read the current time */
#if 0
		client->addr = (0xBA>>1);
#endif

		if(mt9d111_connected_check(client) < 0) {
			v4l_info(client, "%s: camera not connected..\n", __func__);
			printk(KERN_INFO "%s: camera not connected..\n", __func__);
			return -1;
		}

#ifndef USE_INITIAL_WORKER_THREAD
		err = mt9d111_write_regs(client, mt9d111_reg_init, ARRAY_SIZE(mt9d111_reg_init));
		//err = mt9d111_write_regs(client, mt9d111_reg_init_rotate, ARRAY_SIZE(mt9d111_reg_init_rotate));
		//err = mt9d111_write_regs(client, mt9d111_reg_init_1285x725, ARRAY_SIZE(mt9d111_reg_init_1285x725));

		mdelay(60); //keun 2015.04.28

		end_time = get_jiffies_64(); /* read the current time */
		delta_jiffies = end_time - start_time;
		printk(KERN_ERR "%s() = \e[32m%d[ms]\e[0m \n", __func__, jiffies_to_msecs(delta_jiffies));

		if (err < 0) {
			pr_err("%s: write reg error(err: %d)\n", __func__, err);
			printk(KERN_INFO "%s: write reg error(err: %d)\n", __func__, err); 
			return err;
		}
		state->inited = true;

#else
		printk("queue_work ---> mt9d111_init_work\n");
		queue_work(state->init_wq, &state->init_work);
#endif
	}

	return 0;
}

static int mt9d111_s_power(struct v4l2_subdev *sd, int on)
{
#if 0
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9d111_state *state = to_state(sd);
	int i2c_id = i2c_adapter_id(client->adapter);
	static bool is_first = true;
	unsigned int reset_en=0;
	int pwm_ch=0;

	v4l_info(client,"%s() id:%d, on:%d, is_first:%d\n", __func__, i2c_id, on, is_first);

    switch (i2c_id) {
		case 1:
			pwm_ch = 0;
			reset_en = CFG_IO_CAM0_RESET;
			break;
		case 2:
			pwm_ch = 1;
			reset_en = CFG_IO_CAM1_RESET;
			break;
		default:
			v4l_info(client,"%s() id:%d, unkown id!!!\n", __func__, i2c_id);
			return -1;
    }

	if (on)
	{
		if(is_first)
		{
			nxp_soc_gpio_set_io_dir(reset_en, 1);
			nxp_soc_gpio_set_io_func(reset_en, nxp_soc_gpio_get_altnum(reset_en));
			nxp_soc_gpio_set_out_value(reset_en, 0);
			mdelay(10);
			nxp_soc_gpio_set_out_value(reset_en, 1);
			mdelay(1);
			nxp_soc_pwm_set_frequency(pwm_ch, 24000000, 50);
			is_first = false;
		}
	}
	else
	{
		nxp_soc_pwm_set_frequency(pwm_ch, 0, 0);
		mdelay(1);
		nxp_soc_gpio_set_out_value(reset_en, 0);
		state->inited = false;
		is_first = true;
	}
#endif
	return 0;
}

static void mt9d111_set_power(struct v4l2_subdev *sd, int on)
{
#if 1
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9d111_state *state = to_state(sd);
	int i2c_id = i2c_adapter_id(client->adapter);
	unsigned int reset_en=0;
	int pwm_ch=0;

	v4l_info(client,"%s() id:%d, on:%d\n", __func__, i2c_id, on);

    switch (i2c_id) {
		case 1:
			pwm_ch = 0;
			//reset_en = CFG_IO_CAM0_RESET;
			reset_en = CFG_IO_CAMERA_FRONT_RESET;
			break;
		case 2:
			pwm_ch = 1;
			//reset_en = CFG_IO_CAM1_RESET;
			reset_en = CFG_IO_CAMERA_FRONT_RESET;
			break;
		default:
			v4l_info(client,"%s() id:%d, unkown id!!!\n", __func__, i2c_id);
			return;
    }

	if (on)
	{
		//nxp_soc_gpio_set_io_dir(reset_en, 1);
		nxp_soc_gpio_set_io_dir(reset_en, 0);
		nxp_soc_gpio_set_io_func(reset_en, nxp_soc_gpio_get_altnum(reset_en));

		nxp_soc_gpio_set_out_value(reset_en, 1);
		mdelay(10);

		nxp_soc_gpio_set_out_value(reset_en, 0);
		mdelay(10);

		nxp_soc_gpio_set_out_value(reset_en, 1);
		mdelay(1);
		//nxp_soc_pwm_set_frequency(pwm_ch, 24000000, 50);
	}
	else
	{
	//	nxp_soc_pwm_set_frequency(pwm_ch, 0, 0);
		mdelay(1);
		nxp_soc_gpio_set_out_value(reset_en, 0);
		state->inited = false;
	}
#endif

}

static int mt9d111_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct mt9d111_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i2c_id = i2c_adapter_id(client->adapter);

	if(i2c_id==1)
		v4l_info(client,"%s: \e[31mi2c_id=%d, enable=%d +++++\e[0m\n", __func__, i2c_id, enable);
	else
		v4l_info(client,"%s: \e[32mi2c_id=%d, enable=%d +++++\e[0m\n", __func__, i2c_id, enable);

	if (enable) {
		//mt9d111_set_power(sd, 1);
		mt9d111_init(sd, enable);
		mt9d111_set_frame_size(sd, state->mode, state->width, state->height);
	} else {
		//mt9d111_set_power(sd, 0);
		//mt9d111_write_regs(client, mt9d111_reg_disable, ARRAY_SIZE(mt9d111_reg_disable));
	}

	if(i2c_id==1)
		v4l_info(client,"%s: \e[31mi2c_id=%d, enable=%d -----\e[0m\n", __func__, i2c_id, enable);
	else
		v4l_info(client,"%s: \e[32mi2c_id=%d, enable=%d -----\e[0m\n", __func__, i2c_id, enable);

	return 0;
}

static const struct v4l2_subdev_core_ops mt9d111_core_ops = {
	.s_power = mt9d111_s_power,
	.s_ctrl = v4l2_subdev_s_ctrl,
};

static const struct v4l2_subdev_pad_ops mt9d111_pad_ops = {
	.set_fmt  = mt9d111_set_fmt,
	.set_crop = mt9d111_set_crop,
	.get_crop = mt9d111_get_crop,
};

static const struct v4l2_subdev_video_ops mt9d111_video_ops = {
	.s_stream = mt9d111_s_stream,
};

static const struct v4l2_subdev_ops mt9d111_ops = {
	.core = &mt9d111_core_ops,
	.video = &mt9d111_video_ops,
	.pad = &mt9d111_pad_ops,
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

static const struct media_entity_operations mt9d111_media_ops = {
	.link_setup = _link_setup,
};
/*
 * mt9d111_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int mt9d111_probe(struct i2c_client *client,
								const struct i2c_device_id *id)
{
	struct mt9d111_state *state;
	struct v4l2_subdev *sd;
	int ret;

	state = kzalloc(sizeof(struct mt9d111_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	sd = &state->sd;
	strcpy(sd->name, id->name);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &mt9d111_ops);

	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	state->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
	sd->entity.ops = &mt9d111_media_ops;
	if (media_entity_init(&sd->entity, 1, &state->pad, 0)) {
		dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
		kfree(state);
		return -ENOENT;
	}

	ret = mt9d111_initialize_ctrls(state);
	if (ret < 0) {
		pr_err("%s: failed to initialize controls\n", __func__);
		kfree(state);
		return ret;
	}

#ifdef USE_INITIAL_WORKER_THREAD
	state->init_wq = create_singlethread_workqueue("mt9d111-init");
	if (!state->init_wq) {
		pr_err("%s: error create work queue for init\n", __func__);
		return -ENOMEM;
	}
	INIT_WORK(&state->init_work, mt9d111_init_work);
#endif

	dev_info(&client->dev, "mt9d111 has been probed\n");
	return 0;
}


static int mt9d111_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	v4l2_device_unregister_subdev(sd);
	v4l2_ctrl_handler_free(sd->ctrl_handler);
	media_entity_cleanup(&sd->entity);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id mt9d111_id[] = {
	{ MT9D111_CAM_DRIVER_NAME, 0 },
	{ },
};


MODULE_DEVICE_TABLE(i2c, mt9d111_id);

static struct i2c_driver _i2c_driver = {
	.driver = {
		.name = MT9D111_CAM_DRIVER_NAME,
	},
	.probe    = mt9d111_probe,
	.remove   = mt9d111_remove,
	.id_table = mt9d111_id,
};

module_i2c_driver(_i2c_driver);

MODULE_DESCRIPTION("micron MT9D111 camera driver");
MODULE_AUTHOR("   <    @nexell.co.kr>");
MODULE_LICENSE("GPL");
