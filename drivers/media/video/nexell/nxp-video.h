#ifndef _NXP_VIDEO_H
#define _NXP_VIDEO_H

#define NXP_VIDEO_MAX_NAME_SIZE         (32)
#define NXP_VIDEO_MAX_PLANES            (3)
#define NXP_VIDEO_MAX_PADS              (2)
#define NXP_VIDEO_MAX_BUFFERS           (16)

#include <linux/spinlock.h>

struct nxp_video;

struct nxp_video_format {
    char *name;
    u32   pixelformat;
    u32   mbus_code;
    u32   num_planes;
    u32   num_sw_planes;
    bool  is_separated;
};

struct nxp_video_frame {
    u16 width;
    u16 height;
    u16 stride[NXP_VIDEO_MAX_PLANES];
    u32 size[NXP_VIDEO_MAX_PLANES];
    struct nxp_video_format format;
};

struct nxp_video_buffer;
typedef int (*nxp_video_buf_done)(struct nxp_video_buffer *);

struct nxp_video_buffer {
    struct list_head list;
    int consumer_index; /* consumer increment this field after consuming */
    dma_addr_t dma_addr[NXP_VIDEO_MAX_PLANES];
    u32 stride[NXP_VIDEO_MAX_PLANES];
    void *priv;   /* struct vb2_buffer */
    nxp_video_buf_done cb_buf_done;
};

struct nxp_buffer_consumer {
    struct list_head list;
    int index;
    ulong timeout;
    void *priv; /* consumer private data */
    int (*queue)(struct nxp_video_buffer *, void *);
    u32 usage_count; /* TODO reference count atomic? */
};

/* video device type : exclusive */
enum nxp_video_type {
    NXP_VIDEO_TYPE_CAPTURE = 0,
    NXP_VIDEO_TYPE_OUT,
    NXP_VIDEO_TYPE_M2M,
    NXP_VIDEO_TYPE_MAX,
};

enum nxp_buffer_consumer_type {
    /* vq type: V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
    NXP_BUFFER_CONSUMER_SINK = 0,
    /* vq type: V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE */
    NXP_BUFFER_CONSUMER_SOURCE,
    NXP_BUFFER_CONSUMER_INVALID
};

struct nxp_video {
	char name[NXP_VIDEO_MAX_NAME_SIZE];
    u32 type; /* enum nxp_video_type */

    struct nxp_video_buffer *sink_bufs[NXP_VIDEO_MAX_BUFFERS];
    struct nxp_video_buffer *source_bufs[NXP_VIDEO_MAX_BUFFERS];

    struct v4l2_device *v4l2_dev;
    struct vb2_queue *vbq;
    void *vb2_alloc_ctx;

    struct mutex lock; /* for video_device */
    struct video_device vdev;
    /**
     * pad 0 : sink
     * pad 1 : source
     */
    struct media_pad pads[NXP_VIDEO_MAX_PADS];

    /* frame[0] : sink, capture
       frame[1] : source, out */
    struct nxp_video_frame frame[2];

    /* buffer consumer */
    int (*register_buffer_consumer)(struct nxp_video *,
            struct nxp_buffer_consumer *,
            enum nxp_buffer_consumer_type);
    void (*unregister_buffer_consumer)(struct nxp_video *,
            struct nxp_buffer_consumer *,
            enum nxp_buffer_consumer_type);

    /* lock for consumer list */
    spinlock_t lock_consumer;
    /* I'm source */
    struct list_head source_consumer_list;
    int source_consumer_count;
    /* i'm sink */
    struct list_head sink_consumer_list;
    int sink_consumer_count;

    /* m2m specific */
    struct v4l2_m2m_dev *m2m_dev;
    struct v4l2_m2m_ctx *m2m_ctx;

    uint32_t open_count;

};

/* macros */
#define vdev_to_nxp_video(vdev) container_of(vdev, struct nxp_video, video)
#define vbq_to_nxp_video(vbq)   container_of(vbq, struct nxp_video, vbq)

/* public functions */
struct nxp_video *create_nxp_video(char *, u32 type, struct v4l2_device *, void *);
void release_nxp_video(struct nxp_video *);
int register_nxp_video(struct nxp_video *);
void unregister_nxp_video(struct nxp_video *);
//int register_buffer_consumer(struct nxp_video *, struct nxp_buffer_consumer *);

#endif
