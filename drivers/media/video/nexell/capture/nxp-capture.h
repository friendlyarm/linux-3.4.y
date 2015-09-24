#ifndef _NXP_CAPTURE_H
#define _NXP_CAPTURE_H

#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include "nxp-vin-clipper.h"
#include "nxp-decimator.h"
#include "nxp-csi.h"

//#define NXP_CAPTURE_SUBDEV_GROUP_ID (1 << 16)

/**
 * interrupt flags : prototype/module/nx_vip.h
 **/
enum nxp_capture_irq_flags {
    CAPTURE_VSINT = (0UL),
    CAPTURE_HSINT = (1UL),
    CAPTURE_CLIPPER_INT = (2UL),
    CAPTURE_DECIMATOR_INT = (2UL)
};

struct nxp_vin_clipper;
struct nxp_decimator;
struct nxp_csi;
struct nxp_v4l2_irq_entry;
struct nxp_capture_platformdata;

struct nxp_capture {
    atomic_t running_child_bitmap; /* video device active child : clipper, decimator */

    spinlock_t lock; /* lock for irq_entry_list */
    struct list_head irq_entry_list;
    int irq_entry_count;

    int module; /* module index */
    int irq;
    int index;

    int interface_type; /* csi or parallel */
    struct nxp_capture_platformdata *platdata;

    /* callbacks */
    struct media_device *(*get_media_device)(struct nxp_capture *);
    struct v4l2_device *(*get_v4l2_device)(struct nxp_capture *);
    int (*get_module_num)(struct nxp_capture *);
    int (*get_index_num)(struct nxp_capture *);
    void *(*get_alloc_ctx)(struct nxp_capture *);
    int (*run)(struct nxp_capture *, void *);
    void (*stop)(struct nxp_capture *, void *);
    int (*register_irq_entry)(struct nxp_capture *, struct nxp_v4l2_irq_entry *);
    void (*unregister_irq_entry)(struct nxp_capture *, struct nxp_v4l2_irq_entry *);
    struct v4l2_subdev *(*get_csi_subdev)(struct nxp_capture *);
    struct v4l2_subdev *(*get_sensor_subdev)(struct nxp_capture *);
#ifdef CONFIG_TURNAROUND_VIP_RESET
    void (*backup_reset_restore_register)(int);
#endif

    /* children */
    struct nxp_vin_clipper vin_clipper;
    struct nxp_decimator decimator;
    struct nxp_csi csi;
    struct v4l2_subdev *sensor;

    /* context */
    bool context_changed;
    bool clip_enable;
    bool deci_enable;
    wait_queue_head_t wait_change_context;
};

/* parent use this api */
struct nxp_capture *create_nxp_capture(int index, int module, struct nxp_capture_platformdata *);
void release_nxp_capture(struct nxp_capture *);
int register_nxp_capture(struct nxp_capture *);
void unregister_nxp_capture(struct nxp_capture *);
int suspend_nxp_capture(struct nxp_capture *);
int resume_nxp_capture(struct nxp_capture *);

#endif
