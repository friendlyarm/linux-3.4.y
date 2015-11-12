#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/switch.h>
#include <linux/delay.h>

#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>

//#include <nexell/platform.h>

/*#include <mach/platform.h>*/
/*#include <mach/soc.h>*/

/*#define DEBUG_TW9900*/
#ifdef DEBUG_TW9900
#define vmsg(a...)  printk(a)
#else
#define vmsg(a...)
#endif

#if 0
extern void nxp_soc_gpio_set_out_value(unsigned int io, int high);
extern int  nxp_soc_gpio_get_altnum(unsigned int io);
extern void nxp_soc_gpio_set_io_dir(unsigned int io, int out);
extern void nxp_soc_gpio_set_io_func(unsigned int io, unsigned int func);
#endif

extern void dump_register(int module);

#define DEFAULT_BRIGHTNESS  0x1e
struct tw9900_state {
    struct media_pad pad;
    struct v4l2_subdev sd;
    struct switch_dev switch_dev;
    bool first;

    struct i2c_client *i2c_client;

    struct v4l2_ctrl_handler handler;
    /* custom control */
    struct v4l2_ctrl *ctrl_mux;
    struct v4l2_ctrl *ctrl_status;
    /* standard control */
    struct v4l2_ctrl *ctrl_brightness;
    char brightness;
};

struct reg_val {
    uint8_t reg;
    uint8_t val;
};

#define END_MARKER {0xff, 0xff}

static struct reg_val _sensor_init_data[] =
{
    {0x02, 0x40},
    {0x03, 0xa2},
    {0x07, 0x02},
    {0x08, 0x12},
    {0x09, 0xf0},
    {0x0a, 0x1c},
    /*{0x0b, 0xd0}, // 720 */
    {0x0b, 0xc0}, // 704
    {0x1b, 0x00},
    /*{0x10, 0xfa},*/
    {0x10, 0x1e},
    {0x11, 0x64},
    {0x2f, 0xe6},
    {0x55, 0x00},
#if 1
    /*{0xb1, 0x20},*/
    /*{0xb1, 0x02},*/
    {0xaf, 0x00},
    {0xb1, 0x20},
    {0xb4, 0x20},
    /*{0x06, 0x80},*/
#endif
    /*{0xaf, 0x40},*/
    /*{0xaf, 0x00},*/
    /*{0xaf, 0x80},*/
    END_MARKER
};

static struct tw9900_state _state;

void tw9900_external_set_brightness(char brightness) {
    _state.brightness = brightness;
}
EXPORT_SYMBOL(tw9900_external_set_brightness);

#if 0
/**
 * hw function
 */
/* debugging */
#define DUMP_REGISTER 1
void dump_register(int module)
{
#if (DUMP_REGISTER)
#define DBGOUT(args...)  printk(args)
    NX_VIP_RegisterSet *pREG =
        (NX_VIP_RegisterSet*)NX_VIP_GetBaseAddress(module);

    DBGOUT("BASE ADDRESS: %p\n", pREG);
#if defined(CONFIG_ARCH_S5P4418)
    DBGOUT(" VIP_CONFIG     = 0x%04x\r\n", pREG->VIP_CONFIG);
    DBGOUT(" VIP_HVINT      = 0x%04x\r\n", pREG->VIP_HVINT);
    DBGOUT(" VIP_SYNCCTRL   = 0x%04x\r\n", pREG->VIP_SYNCCTRL);
    DBGOUT(" VIP_SYNCMON    = 0x%04x\r\n", pREG->VIP_SYNCMON);
    DBGOUT(" VIP_VBEGIN     = 0x%04x\r\n", pREG->VIP_VBEGIN);
    DBGOUT(" VIP_VEND       = 0x%04x\r\n", pREG->VIP_VEND);
    DBGOUT(" VIP_HBEGIN     = 0x%04x\r\n", pREG->VIP_HBEGIN);
    DBGOUT(" VIP_HEND       = 0x%04x\r\n", pREG->VIP_HEND);
    DBGOUT(" VIP_FIFOCTRL   = 0x%04x\r\n", pREG->VIP_FIFOCTRL);
    DBGOUT(" VIP_HCOUNT     = 0x%04x\r\n", pREG->VIP_HCOUNT);
    DBGOUT(" VIP_VCOUNT     = 0x%04x\r\n", pREG->VIP_VCOUNT);
    DBGOUT(" VIP_CDENB      = 0x%04x\r\n", pREG->VIP_CDENB);
    DBGOUT(" VIP_ODINT      = 0x%04x\r\n", pREG->VIP_ODINT);
    DBGOUT(" VIP_IMGWIDTH   = 0x%04x\r\n", pREG->VIP_IMGWIDTH);
    DBGOUT(" VIP_IMGHEIGHT  = 0x%04x\r\n", pREG->VIP_IMGHEIGHT);
    DBGOUT(" CLIP_LEFT      = 0x%04x\r\n", pREG->CLIP_LEFT);
    DBGOUT(" CLIP_RIGHT     = 0x%04x\r\n", pREG->CLIP_RIGHT);
    DBGOUT(" CLIP_TOP       = 0x%04x\r\n", pREG->CLIP_TOP);
    DBGOUT(" CLIP_BOTTOM    = 0x%04x\r\n", pREG->CLIP_BOTTOM);
    DBGOUT(" DECI_TARGETW   = 0x%04x\r\n", pREG->DECI_TARGETW);
    DBGOUT(" DECI_TARGETH   = 0x%04x\r\n", pREG->DECI_TARGETH);
    DBGOUT(" DECI_DELTAW    = 0x%04x\r\n", pREG->DECI_DELTAW);
    DBGOUT(" DECI_DELTAH    = 0x%04x\r\n", pREG->DECI_DELTAH);
    DBGOUT(" DECI_CLEARW    = 0x%04x\r\n", pREG->DECI_CLEARW);
    DBGOUT(" DECI_CLEARH    = 0x%04x\r\n", pREG->DECI_CLEARH);
    DBGOUT(" DECI_LUSEG     = 0x%04x\r\n", pREG->DECI_LUSEG);
    DBGOUT(" DECI_CRSEG     = 0x%04x\r\n", pREG->DECI_CRSEG);
    DBGOUT(" DECI_CBSEG     = 0x%04x\r\n", pREG->DECI_CBSEG);
    DBGOUT(" DECI_FORMAT    = 0x%04x\r\n", pREG->DECI_FORMAT);
    DBGOUT(" DECI_ROTFLIP   = 0x%04x\r\n", pREG->DECI_ROTFLIP);
    DBGOUT(" DECI_LULEFT    = 0x%04x\r\n", pREG->DECI_LULEFT);
    DBGOUT(" DECI_CRLEFT    = 0x%04x\r\n", pREG->DECI_CRLEFT);
    DBGOUT(" DECI_CBLEFT    = 0x%04x\r\n", pREG->DECI_CBLEFT);
    DBGOUT(" DECI_LURIGHT   = 0x%04x\r\n", pREG->DECI_LURIGHT);
    DBGOUT(" DECI_CRRIGHT   = 0x%04x\r\n", pREG->DECI_CRRIGHT);
    DBGOUT(" DECI_CBRIGHT   = 0x%04x\r\n", pREG->DECI_CBRIGHT);
    DBGOUT(" DECI_LUTOP     = 0x%04x\r\n", pREG->DECI_LUTOP);
    DBGOUT(" DECI_CRTOP     = 0x%04x\r\n", pREG->DECI_CRTOP);
    DBGOUT(" DECI_CBTOP     = 0x%04x\r\n", pREG->DECI_CBTOP);
    DBGOUT(" DECI_LUBOTTOM  = 0x%04x\r\n", pREG->DECI_LUBOTTOM);
    DBGOUT(" DECI_CRBOTTOM  = 0x%04x\r\n", pREG->DECI_CRBOTTOM);
    DBGOUT(" DECI_CBBOTTOM  = 0x%04x\r\n", pREG->DECI_CBBOTTOM);
    DBGOUT(" CLIP_LUSEG     = 0x%04x\r\n", pREG->CLIP_LUSEG);
    DBGOUT(" CLIP_CRSEG     = 0x%04x\r\n", pREG->CLIP_CRSEG);
    DBGOUT(" CLIP_CBSEG     = 0x%04x\r\n", pREG->CLIP_CBSEG);
    DBGOUT(" CLIP_FORMAT    = 0x%04x\r\n", pREG->CLIP_FORMAT);
    DBGOUT(" CLIP_ROTFLIP   = 0x%04x\r\n", pREG->CLIP_ROTFLIP);
    DBGOUT(" CLIP_LULEFT    = 0x%04x\r\n", pREG->CLIP_LULEFT);
    DBGOUT(" CLIP_CRLEFT    = 0x%04x\r\n", pREG->CLIP_CRLEFT);
    DBGOUT(" CLIP_CBLEFT    = 0x%04x\r\n", pREG->CLIP_CBLEFT);
    DBGOUT(" CLIP_LURIGHT   = 0x%04x\r\n", pREG->CLIP_LURIGHT);
    DBGOUT(" CLIP_CRRIGHT   = 0x%04x\r\n", pREG->CLIP_CRRIGHT);
    DBGOUT(" CLIP_CBRIGHT   = 0x%04x\r\n", pREG->CLIP_CBRIGHT);
    DBGOUT(" CLIP_LUTOP     = 0x%04x\r\n", pREG->CLIP_LUTOP);
    DBGOUT(" CLIP_CRTOP     = 0x%04x\r\n", pREG->CLIP_CRTOP);
    DBGOUT(" CLIP_CBTOP     = 0x%04x\r\n", pREG->CLIP_CBTOP);
    DBGOUT(" CLIP_LUBOTTOM  = 0x%04x\r\n", pREG->CLIP_LUBOTTOM);
    DBGOUT(" CLIP_CRBOTTOM  = 0x%04x\r\n", pREG->CLIP_CRBOTTOM);
    DBGOUT(" CLIP_CBBOTTOM  = 0x%04x\r\n", pREG->CLIP_CBBOTTOM);
    DBGOUT(" VIP_SCANMODE   = 0x%04x\r\n", pREG->VIP_SCANMODE);
    DBGOUT(" CLIP_YUYVENB   = 0x%04x\r\n", pREG->CLIP_YUYVENB);
    DBGOUT(" CLIP_BASEADDRH = 0x%04x\r\n", pREG->CLIP_BASEADDRH);
    DBGOUT(" CLIP_BASEADDRL = 0x%04x\r\n", pREG->CLIP_BASEADDRL);
    DBGOUT(" CLIP_STRIDEH   = 0x%04x\r\n", pREG->CLIP_STRIDEH);
    DBGOUT(" CLIP_STRIDEL   = 0x%04x\r\n", pREG->CLIP_STRIDEL);
    DBGOUT(" VIP_VIP1       = 0x%04x\r\n", pREG->VIP_VIP1);
    /* DBGOUT(" VIPCLKENB      = 0x%04x\r\n", pREG->VIPCLKENB); */
    /* DBGOUT(" VIPCLKGEN[0][0]= 0x%04x\r\n", pREG->VIPCLKGEN[0][0]); */
    /* DBGOUT(" VIPCLKGEN[0][1]= 0x%04x\r\n", pREG->VIPCLKGEN[0][1]); */
    /* DBGOUT(" VIPCLKGEN[1][0]= 0x%04x\r\n", pREG->VIPCLKGEN[1][0]); */
    /* DBGOUT(" VIPCLKGEN[1][1]= 0x%04x\r\n", pREG->VIPCLKGEN[1][1]); */
#elif defined(CONFIG_ARCH_S5P6818)
    DBGOUT(" VIP_CONFIG     = 0x%04x\r\n", pREG->VIP_CONFIG);
    DBGOUT(" VIP_HVINT      = 0x%04x\r\n", pREG->VIP_HVINT);
    DBGOUT(" VIP_SYNCCTRL   = 0x%04x\r\n", pREG->VIP_SYNCCTRL);
    DBGOUT(" VIP_SYNCMON    = 0x%04x\r\n", pREG->VIP_SYNCMON);
    DBGOUT(" VIP_VBEGIN     = 0x%04x\r\n", pREG->VIP_VBEGIN);
    DBGOUT(" VIP_VEND       = 0x%04x\r\n", pREG->VIP_VEND);
    DBGOUT(" VIP_HBEGIN     = 0x%04x\r\n", pREG->VIP_HBEGIN);
    DBGOUT(" VIP_HEND       = 0x%04x\r\n", pREG->VIP_HEND);
    DBGOUT(" VIP_FIFOCTRL   = 0x%04x\r\n", pREG->VIP_FIFOCTRL);
    DBGOUT(" VIP_HCOUNT     = 0x%04x\r\n", pREG->VIP_HCOUNT);
    DBGOUT(" VIP_VCOUNT     = 0x%04x\r\n", pREG->VIP_VCOUNT);
    DBGOUT(" VIP_PADCLK_SEL = 0x%04x\r\n", pREG->VIP_PADCLK_SEL);
    DBGOUT(" VIP_INFIFOCLR  = 0x%04x\r\n", pREG->VIP_INFIFOCLR);
    DBGOUT(" VIP_CDENB      = 0x%04x\r\n", pREG->VIP_CDENB);
    DBGOUT(" VIP_ODINT      = 0x%04x\r\n", pREG->VIP_ODINT);
    DBGOUT(" VIP_IMGWIDTH   = 0x%04x\r\n", pREG->VIP_IMGWIDTH);
    DBGOUT(" VIP_IMGHEIGHT  = 0x%04x\r\n", pREG->VIP_IMGHEIGHT);
    DBGOUT(" CLIP_LEFT      = 0x%04x\r\n", pREG->CLIP_LEFT);
    DBGOUT(" CLIP_RIGHT     = 0x%04x\r\n", pREG->CLIP_RIGHT);
    DBGOUT(" CLIP_TOP       = 0x%04x\r\n", pREG->CLIP_TOP);
    DBGOUT(" CLIP_BOTTOM    = 0x%04x\r\n", pREG->CLIP_BOTTOM);
    DBGOUT(" DECI_TARGETW   = 0x%04x\r\n", pREG->DECI_TARGETW);
    DBGOUT(" DECI_TARGETH   = 0x%04x\r\n", pREG->DECI_TARGETH);
    DBGOUT(" DECI_DELTAW    = 0x%04x\r\n", pREG->DECI_DELTAW);
    DBGOUT(" DECI_DELTAH    = 0x%04x\r\n", pREG->DECI_DELTAH);
    DBGOUT(" DECI_CLEARW    = 0x%04x\r\n", pREG->DECI_CLEARW);
    DBGOUT(" DECI_CLEARH    = 0x%04x\r\n", pREG->DECI_CLEARH);
    DBGOUT(" DECI_FORMAT    = 0x%04x\r\n", pREG->DECI_FORMAT);
    DBGOUT(" DECI_LUADDR    = 0x%04x\r\n", pREG->DECI_LUADDR);
    DBGOUT(" DECI_LUSTRIDE  = 0x%04x\r\n", pREG->DECI_LUSTRIDE);
    DBGOUT(" DECI_CRADDR    = 0x%04x\r\n", pREG->DECI_CRADDR);
    DBGOUT(" DECI_CRSTRIDE  = 0x%04x\r\n", pREG->DECI_CRSTRIDE);
    DBGOUT(" DECI_CBADDR    = 0x%04x\r\n", pREG->DECI_CBADDR);
    DBGOUT(" DECI_CBSTRIDE  = 0x%04x\r\n", pREG->DECI_CBSTRIDE);
    DBGOUT(" CLIP_FORMAT    = 0x%04x\r\n", pREG->CLIP_FORMAT);
    DBGOUT(" CLIP_LUADDR    = 0x%04x\r\n", pREG->CLIP_LUADDR);
    DBGOUT(" CLIP_LUSTRIDE  = 0x%04x\r\n", pREG->CLIP_LUSTRIDE);
    DBGOUT(" CLIP_CRADDR    = 0x%04x\r\n", pREG->CLIP_CRADDR);
    DBGOUT(" CLIP_CRSTRIDE  = 0x%04x\r\n", pREG->CLIP_CRSTRIDE);
    DBGOUT(" CLIP_CBADDR    = 0x%04x\r\n", pREG->CLIP_CBADDR);
    DBGOUT(" CLIP_CBSTRIDE  = 0x%04x\r\n", pREG->CLIP_CBSTRIDE);
    DBGOUT(" VIP_SCANMODE   = 0x%04x\r\n", pREG->VIP_SCANMODE);
    DBGOUT(" VIP_VIP1       = 0x%04x\r\n", pREG->VIP_VIP1);
#endif

#endif
}
#endif

/**
 * util functions
 */
static inline struct tw9900_state *ctrl_to_me(struct v4l2_ctrl *ctrl)
{
    return container_of(ctrl->handler, struct tw9900_state, handler);
}

#define THINE_I2C_RETRY_CNT				10
static int _i2c_read_byte(struct i2c_client *client, u8 addr, u8 *data)
{
	s8 i = 0;
	s8 ret = 0;
	u8 buf = 0;
	struct i2c_msg msg[2];

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &buf;

	for(i=0; i<THINE_I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(client->adapter, msg, 2);
		if (likely(ret == 2))
			break;
	}

	if (unlikely(ret != 2)) {
		dev_err(&client->dev, "_i2c_read_byte failed reg:0x%02x\n", addr);
		return -EIO;
	}

	*data = buf;
	return 0;
}

static int _i2c_write_byte(struct i2c_client *client, u8 addr, u8 val)
{
	s8 i = 0;
	s8 ret = 0;
	u8 buf[2];
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = buf;

	buf[0] = addr;
	buf[1] = val ;

	for(i=0; i<THINE_I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
		{
			/*printk("[%s]i2c write succeed! :  addr - 0x%X, Val - 0x%X\n", __func__, addr, val);*/
			break;
		}
	}

	if (ret != 1) {
        printk(KERN_ERR "%s: failed to write addr 0x%x, val 0x%x\n", __func__, addr, val);
		return -EIO;
	}

	return 0;
}

#define V4L2_CID_MUX        (V4L2_CTRL_CLASS_USER | 0x1001)
#define V4L2_CID_STATUS     (V4L2_CTRL_CLASS_USER | 0x1002)

static int tw9900_set_mux(struct v4l2_ctrl *ctrl)
{
    struct tw9900_state *me = ctrl_to_me(ctrl);
    /*printk("%s: val %d\n", __func__, ctrl->val);*/
    if (ctrl->val == 0) {
        // MUX 0
        if (me->brightness != DEFAULT_BRIGHTNESS)
            _i2c_write_byte(me->i2c_client, 0x10, DEFAULT_BRIGHTNESS);
        _i2c_write_byte(me->i2c_client, 0x02, 0x40);
    } else {
        // MUX 1
        if (me->brightness != DEFAULT_BRIGHTNESS)
            _i2c_write_byte(me->i2c_client, 0x10, me->brightness);
        _i2c_write_byte(me->i2c_client, 0x02, 0x44);
    }

    return 0;
}

static int tw9900_set_brightness(struct v4l2_ctrl *ctrl)
{
    struct tw9900_state *me = ctrl_to_me(ctrl);
    if (ctrl->val != me->brightness) {
        _i2c_write_byte(me->i2c_client, 0x10, ctrl->val);
        me->brightness = ctrl->val;
    }
    return 0;
}

static int tw9900_get_status(struct v4l2_ctrl *ctrl)
{
    struct tw9900_state *me = ctrl_to_me(ctrl);
    u8 data = 0;
    u8 mux;
    u8 val = 0;

    _i2c_read_byte(me->i2c_client, 0x02, &data);
    mux = (data & 0x0c) >> 2;
    if (mux == 0) {
        _i2c_read_byte(me->i2c_client, 0x01, &data);
        if (!(data & 0x80))
            val |= 1 << 0;

        _i2c_read_byte(me->i2c_client, 0x16, &data);
        if (data & 0x40)
            val |= 1 << 1;
    } else {
        _i2c_read_byte(me->i2c_client, 0x01, &data);
        if (!(data & 0x80))
            val |= 1 << 1;
    }

    ctrl->val = val;

    return 0;
}

static int tw9900_s_ctrl(struct v4l2_ctrl *ctrl)
{
    switch (ctrl->id) {
    case V4L2_CID_MUX:
        return tw9900_set_mux(ctrl);
    case V4L2_CID_BRIGHTNESS:
        return tw9900_set_brightness(ctrl);
    default:
        printk(KERN_ERR "%s: invalid control id 0x%x\n", __func__, ctrl->id);
        return -EINVAL;
    }
}

static int tw9900_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
    switch (ctrl->id) {
    case V4L2_CID_STATUS:
        return tw9900_get_status(ctrl);
    default:
        printk(KERN_ERR "%s: invalid control id 0x%x\n", __func__, ctrl->id);
        return -EINVAL;
    }
}

static const struct v4l2_ctrl_ops tw9900_ctrl_ops = {
     .s_ctrl = tw9900_s_ctrl,
     .g_volatile_ctrl = tw9900_g_volatile_ctrl,
};

static const struct v4l2_ctrl_config tw9900_custom_ctrls[] = {
    {
        .ops  = &tw9900_ctrl_ops,
        .id   = V4L2_CID_MUX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "MuxControl",
        .min  = 0,
        .max  = 1,
        .def  = 1,
        .step = 1,
    },
    {
        .ops  = &tw9900_ctrl_ops,
        .id   = V4L2_CID_STATUS,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "Status",
        .min  = 0,
        .max  = 1,
        .def  = 1,
        .step = 1,
        .flags = V4L2_CTRL_FLAG_VOLATILE,
    }
};

#define NUM_CTRLS 3
static int tw9900_initialize_ctrls(struct tw9900_state *me)
{
    v4l2_ctrl_handler_init(&me->handler, NUM_CTRLS);


    me->ctrl_mux = v4l2_ctrl_new_custom(&me->handler, &tw9900_custom_ctrls[0], NULL);
    if (!me->ctrl_mux) {
         printk(KERN_ERR "%s: failed to v4l2_ctrl_new_custom for mux\n", __func__);
         return -ENOENT;
    }

    me->ctrl_status = v4l2_ctrl_new_custom(&me->handler, &tw9900_custom_ctrls[1], NULL);
    if (!me->ctrl_status) {
         printk(KERN_ERR "%s: failed to v4l2_ctrl_new_custom for status\n", __func__);
         return -ENOENT;
    }

    me->ctrl_brightness = v4l2_ctrl_new_std(&me->handler, &tw9900_ctrl_ops,
            V4L2_CID_BRIGHTNESS, -128, 127, 1, 0x1e);
    if (!me->ctrl_brightness) {
        printk(KERN_ERR "%s: failed to v4l2_ctrl_new_std for brightness\n", __func__);
        return -ENOENT;
    }

    me->sd.ctrl_handler = &me->handler;
    if (me->handler.error) {
        printk(KERN_ERR "%s: ctrl handler error(%d)\n", __func__, me->handler.error);
        v4l2_ctrl_handler_free(&me->handler);
        return -EINVAL;
    }

    return 0;
}

static void read_register(void)
{
	uint8_t addr[] =
	{
		0x02,
    	0x03,
    	0x07,
    	0x08,
    	0x09,
    	0x0a,
    	0x0b,
    	0x1b,
    	0x10,
    	0x11,
    	0x2f,
    	0x55,
    	0xaf,
    	0xb1,
    	0xb4,
    	0x52
	};

	int i=0;
	uint8_t result;
    struct tw9900_state *me = &_state;

	for(i=0; i<(sizeof(addr)/sizeof(addr[0])); i++)
	{
		_i2c_read_byte(me->i2c_client, addr[i], &result);
		mdelay(10);
		printk(KERN_INFO "[%s] ADDR : 0x%02X, VAL : 0x%02X\n", __func__, addr[i], result);
	}
}

static int tw9900_s_stream(struct v4l2_subdev *sd, int enable)
{
	if (enable) {
  	if (_state.first) {
#if 0
    	/*int ret = request_irq(IRQ_ALIVE_4, _irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "tw9900", &_state);*/
      int ret = request_irq(IRQ_GPIO_A_START + 3, _irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "tw9900", &_state);
      if (ret) {
     		pr_err("%s: failed to request_irq(irqnum %d)\n", __func__, IRQ_ALIVE_4);
     		return -1;
      }
#else
			int i=0;
			struct tw9900_state *me = &_state;
			struct reg_val *reg_val = _sensor_init_data;

			while (reg_val->reg != 0xff) 
			{
				 _i2c_write_byte(me->i2c_client, reg_val->reg, reg_val->val);
				mdelay(10);
				i++;
				reg_val++;
			}
#endif
			//read_register();
     	//_state.first = false;
		}
	}


	return 0;
}

static int tw9900_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
        struct v4l2_subdev_format *fmt)
{
    vmsg("%s\n", __func__);

    return 0;
}

static int tw9900_s_power(struct v4l2_subdev *sd, int on)
{
    vmsg("%s: %d\n", __func__, on);
    return 0;
}

static const struct v4l2_subdev_core_ops tw9900_subdev_core_ops = {
    .s_power = tw9900_s_power,
};

static const struct v4l2_subdev_pad_ops tw9900_subdev_pad_ops = {
    .set_fmt = tw9900_s_fmt,
};

static const struct v4l2_subdev_video_ops tw9900_subdev_video_ops = {
    .s_stream = tw9900_s_stream,
};

static const struct v4l2_subdev_ops tw9900_ops = {
    .core  = &tw9900_subdev_core_ops,
    .video = &tw9900_subdev_video_ops,
    .pad   = &tw9900_subdev_pad_ops,
};

static int tw9900_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct v4l2_subdev *sd;
    struct tw9900_state *state = &_state;
    int ret;

    vmsg("%s entered\n", __func__);

    sd = &state->sd;
    strcpy(sd->name, "tw9900");

    v4l2_i2c_subdev_init(sd, client, &tw9900_ops);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    state->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    ret = media_entity_init(&sd->entity, 1, &state->pad, 0);
    if (ret < 0) {
        dev_err(&client->dev, "%s: failed to media_entity_init()\n", __func__);
        return ret;
    }

    ret = tw9900_initialize_ctrls(state);
    if (ret < 0) {
        printk(KERN_ERR "%s: failed to initialize controls\n", __func__);
        return ret;
    }

    i2c_set_clientdata(client, sd);
    state->i2c_client = client;

    state->switch_dev.name = "tw9900";
    switch_dev_register(&state->switch_dev);
    switch_set_state(&state->switch_dev, 0);

    state->first = true;

    vmsg("%s exit\n", __func__);

    return 0;
}

static int tw9900_remove(struct i2c_client *client)
{
    struct tw9900_state *state = &_state;
    v4l2_device_unregister_subdev(&state->sd);
    return 0;
}

static const struct i2c_device_id tw9900_id[] = {
    { "tw9900", 0 },
    {}
};

MODULE_DEVICE_TABLE(i2c, tw9900_id);

static struct i2c_driver tw9900_i2c_driver = {
    .driver = {
        .name = "tw9900",
    },
    .probe = tw9900_probe,
    .remove = tw9900_remove,
    .id_table = tw9900_id,
};

static int __init tw9900_init(void)
{
    return i2c_add_driver(&tw9900_i2c_driver);
}

static void __exit tw9900_exit(void)
{
    i2c_del_driver(&tw9900_i2c_driver);
}

module_init(tw9900_init);
module_exit(tw9900_exit);

MODULE_DESCRIPTION("TW9900 Camera Sensor Driver for only FINE");
MODULE_AUTHOR("<swpark@nexell.co.kr>");
MODULE_LICENSE("GPL");
