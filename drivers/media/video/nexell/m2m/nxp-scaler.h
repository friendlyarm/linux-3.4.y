#ifndef _NXP_SCALER_H
#define _NXP_SCALER_H

#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

// #define SIMULATION_SCALER   1

struct list_head;
struct nxp_video;
struct nxp_video_buffer;

#ifdef SIMULATION_SCALER
struct timer_list;
#endif

enum nxp_scaler_input_type {
    NXP_SCALER_INPUT_TYPE_NONE,
    NXP_SCALER_INPUT_TYPE_VIDEO, /* m2m video device */
    NXP_SCALER_INPUT_TYPE_SUBDEV /* clipper video, decimator video */
};

enum nxp_scaler_pad_type {
    NXP_SCALER_PAD_SINK,
    NXP_SCALER_PAD_SOURCE,
    NXP_SCALER_PAD_MAX
};

#ifdef CONFIG_ENABLE_SCALER_MISC_DEVICE
struct completion;
#endif

struct nxp_scaler {
    spinlock_t slock;
    struct list_head src_buffer_list;
    struct nxp_video_buffer *cur_src_buf;

    spinlock_t dlock;
    struct nxp_video_buffer *cur_dst_buf;
    struct list_head dst_buffer_list;

    /* input buffer consumer
       from clipper video or decimator video */
    struct nxp_buffer_consumer *consumer;

    u32 input_type; /* NXP_VIDEO or NXP_SUBDEV */

    /* only support set format
       not support crop */
    struct v4l2_mbus_framefmt format[2];

    struct v4l2_subdev subdev;
    struct media_pad pads[NXP_SCALER_PAD_MAX];

    struct nxp_video *video;

    int irq;
    bool irq_alive;

    bool streaming;

#ifdef SIMULATION_SCALER
    struct timer_list timer;
    u32 timeout; /* msec */
#endif

    unsigned int *command_buffer_vir;
    dma_addr_t  command_buffer_phy;

#ifdef CONFIG_ENABLE_SCALER_MISC_DEVICE
    atomic_t open_count;
    struct completion work_done;
    wait_queue_head_t wq_start;
    wait_queue_head_t wq_end;
    struct nxp_scaler_ioctl_data *ioctl_data;
    spinlock_t running_lock;
    atomic_t running;
#endif

#ifndef CONFIG_USE_SCALER_COMMAND_BUFFER
    int run_count; /* y:1, cb:2, cr:3 */
#endif
};

struct nxp_scaler *create_nxp_scaler(void);
void release_nxp_scaler(struct nxp_scaler *);
int register_nxp_scaler(struct nxp_scaler *);
void unregister_nxp_scaler(struct nxp_scaler *);
#ifdef CONFIG_PM
int suspend_nxp_scaler(struct nxp_scaler *);
int resume_nxp_scaler(struct nxp_scaler *);
#endif

#endif
