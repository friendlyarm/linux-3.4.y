#ifndef _NXP_DECIMATOR_H
#define _NXP_DECIMATOR_H

#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

struct list_head;
struct nxp_capture;
struct nxp_buffer_consumer;
struct nxp_v4l2_irq_entry;
struct nxp_video;
#ifdef CONFIG_ARCH_NXP4330_3200
struct work_struct;
#endif

enum nxp_decimator_pad_type {
    NXP_DECIMATOR_PAD_SINK,
    NXP_DECIMATOR_PAD_SOURCE_MEM,
    NXP_DECIMATOR_PAD_MAX
};

#define DECIMATOR_MAX_NAME_SIZE     32

struct nxp_decimator {
    struct list_head buffer_list;
    spinlock_t slock; /* for buffer_list */
    struct nxp_video_buffer *cur_buf;

    atomic_t state;

    struct nxp_buffer_consumer *consumer;
    struct nxp_v4l2_irq_entry *irq_entry;

    // struct v4l2_mbus_framefmt format[NXP_DECIMATOR_PAD_MAX];
    u32  src_width;
    u32  src_height;
    u32  target_width;
    u32  target_height;
    u32  code;

    struct v4l2_subdev subdev;
    struct media_pad pads[NXP_DECIMATOR_PAD_MAX];

    struct nxp_video *video;
    struct nxp_decimator_platformdata *platdata;

    u32 ref_count;

    char name[DECIMATOR_MAX_NAME_SIZE];

#ifdef CONFIG_ARCH_NXP4330_3200
    struct work_struct work;
#endif

    /* for stop condition */
    struct completion stop_done;
};

int  nxp_decimator_init(struct nxp_decimator *, struct nxp_decimator_platformdata *);
void nxp_decimator_cleanup(struct nxp_decimator *);
int  nxp_decimator_register(struct nxp_decimator *);
void nxp_decimator_unregister(struct nxp_decimator *);
int  nxp_decimator_suspend(struct nxp_decimator *);
int  nxp_decimator_resume(struct nxp_decimator *);

#endif
