#ifndef _NXP_VIN_CLIPPER_H
#define _NXP_VIN_CLIPPER_H

#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/semaphore.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

// #define DEBUG_SYNC

struct list_head;
struct nxp_capture;
struct nxp_vin_platformdata;
struct nxp_buffer_consumer;
struct nxp_v4l2_irq_entry;
struct nxp_video;
struct nxp_video_buffer;
struct timer_list;
struct delayed_work;

enum nxp_vin_pad_type {
    NXP_VIN_PAD_SINK,
    NXP_VIN_PAD_SOURCE_MEM,
    NXP_VIN_PAD_SOURCE_DECIMATOR,
    NXP_VIN_PAD_MAX
};

struct nxp_vin_clipper {
    struct list_head buffer_list;
    spinlock_t  slock; /* for buffer_list */
    struct nxp_video_buffer *cur_buf;
    u32    buffer_count;

    atomic_t state;

    struct nxp_buffer_consumer *consumer;
    struct nxp_v4l2_irq_entry *irq_entry;

    u32 input_type;
    u32 output_type;

    /* input width, height = format[0].width, format[0].height */
    struct v4l2_mbus_framefmt format[NXP_VIN_PAD_MAX];

    /* crop */
    /* output width, heigth = crop.width, crop.height */
    struct v4l2_rect crop;

    struct v4l2_subdev subdev;
    struct media_pad pads[NXP_VIN_PAD_MAX];

    struct nxp_video *video;
    struct nxp_vin_platformdata *platdata;

    /* s_power call */
    u32    ref_count;

    /* s_stream */
    u32     link_count;
    u32     enable_count;

#ifdef DEBUG_SYNC
    struct timer_list timer;
#endif

    /* for late power down */
    struct delayed_work work_power_down;

    /* for stop condition */
    struct completion stop_done;

    /* for serialization s_stream() */
    struct semaphore s_stream_sem;
};

int  nxp_vin_clipper_init(struct nxp_vin_clipper *, struct nxp_vin_platformdata *);
void nxp_vin_clipper_cleanup(struct nxp_vin_clipper *);
int  nxp_vin_clipper_register(struct nxp_vin_clipper *);
void nxp_vin_clipper_unregister(struct nxp_vin_clipper *);
int  nxp_vin_clipper_suspend(struct nxp_vin_clipper *);
int  nxp_vin_clipper_resume(struct nxp_vin_clipper *);

#endif
