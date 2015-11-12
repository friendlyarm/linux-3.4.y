#ifndef _NXP_MLC_H
#define _NXP_MLC_H

#include <linux/spinlock.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

struct list_head;
struct nxp_buffer_consumer;
struct nxp_video;
struct nxp_video_buffer;

enum nxp_mlc_pad_type {
    NXP_MLC_PAD_SINK_RGB,
    NXP_MLC_PAD_SINK_VIDEO,
    NXP_MLC_PAD_SOURCE,
    NXP_MLC_PAD_MAX
};

enum nxp_mlc_input_type {
    NXP_MLC_INPUT_TYPE_NONE,
    NXP_MLC_INPUT_TYPE_MY_VIDEO, /* mlc video device */
    NXP_MLC_INPUT_TYPE_OTHER_VIDEO /* clipper video, decimator video, m2m video */
};

struct nxp_mlc_rgb_attr {
    /* format, width, height */
    struct v4l2_mbus_framefmt format; /* code = NX_MLC_RGBFMT */
    /* position */
    struct v4l2_rect crop; /* use only left, top */
    /* alpha : 0 ~ 15, disable < 0*/
    int alpha;
    /* transcolor : R8G8B8, disable < 0 */
    int transcolor;
    /* invertcolor : R8G8B8, disable < 0 */
    int invertcolor;
    /* backcolor : R8G8B8, disable < 0 */
    int backcolor;
    /* invalidzone */
    struct v4l2_rect invalidzone;
    /* pixelbyte */
    int pixelbyte;

    /* for sleep */
    unsigned int addr;
    unsigned int stride;
};

struct nxp_mlc_vid_attr {
    /* format, width, height */
    struct v4l2_mbus_framefmt format;
    /* position */
    struct v4l2_rect crop;
    /* source crop */
    struct v4l2_rect source_crop;
    /* priority : 0 ~ 3*/
    u32 priority;
    /* colorkey : R8G8B8, disable < 0 */
    int colorkey;
    /* alpha : 0 ~ 15, diable < 0 */
    int alpha;
    /* bright : -128 ~ 128, disable < -128 */
    int brightness;
    /* hue : 0 ~ 360, disable < 0 */
    int hue;
    /* contrast : 0 ~ 8, disable < 0 */
    int contrast;
    /* saturation : -100 ~ 100, disable < -100 */
    int saturation;
    /* hfilter : bool */
    bool hfilter_enable;
    /* vfilter : bool */
    bool vfilter_enable;

    /* for sleep */
    unsigned int lu_a, cb_a, cr_a;
    unsigned int lu_s, cb_s, cr_s;
};

struct nxp_mlc {
    int id;

    int  user_count; /* s_power */
    bool enabled;

    struct v4l2_subdev subdev;
    struct media_pad pads[NXP_MLC_PAD_MAX];

    /* rgb buffer */
    spinlock_t rlock;
    struct list_head rgb_buffer_list;
    //struct nxp_video_buffer *cur_rgb_buf;

    /* video buffer */
    spinlock_t vlock;
    struct list_head vid_buffer_list;
    struct nxp_video_buffer *cur_vid_buf;
    struct nxp_video_buffer *old_vid_buf;

    /* buffer consumer */
    struct nxp_buffer_consumer *rgb_consumer;
    struct nxp_buffer_consumer *vid_consumer;

    u32 vid_input_type; /* only video can connect to others video */

    struct nxp_video *rgb_video;
    struct nxp_video *vid_video;

    /* rgb attribute */
    bool rgb_enabled;
    bool rgb_streaming;
    struct nxp_mlc_rgb_attr rgb_attr;

    /* video attribute */
    bool vid_enabled;
    bool vid_streaming;
    struct nxp_mlc_vid_attr vid_attr;

    /* irq callback */
    struct disp_irq_callback *callback;

};

/**
 * public api
 */
struct nxp_mlc *create_nxp_mlc(int id);
void release_nxp_mlc(struct nxp_mlc *);
int  register_nxp_mlc(struct nxp_mlc *);
void unregister_nxp_mlc(struct nxp_mlc *);
int  suspend_nxp_mlc(struct nxp_mlc *);
int  resume_nxp_mlc(struct nxp_mlc *);

#endif
