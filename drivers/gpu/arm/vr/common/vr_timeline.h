/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_TIMELINE_H__
#define __VR_TIMELINE_H__

#include "vr_osk.h"
#include "vr_ukk.h"
#include "vr_session.h"
#include "vr_kernel_common.h"
#include "vr_spinlock_reentrant.h"
#include "vr_sync.h"
#include "vr_scheduler_types.h"

/**
 * Soft job timeout.
 *
 * Soft jobs have to be signaled as complete after activation.  Normally this is done by user space,
 * but in order to guarantee that every soft job is completed, we also have a timer.
 */
#define VR_TIMELINE_TIMEOUT_HZ ((u32) (HZ * 3 / 2)) /* 1500 ms. */

/**
 * Timeline type.
 */
typedef enum vr_timeline_id {
	VR_TIMELINE_GP   = VR_UK_TIMELINE_GP,   /**< GP job timeline. */
	VR_TIMELINE_PP   = VR_UK_TIMELINE_PP,   /**< PP job timeline. */
	VR_TIMELINE_SOFT = VR_UK_TIMELINE_SOFT, /**< Soft job timeline. */
	VR_TIMELINE_MAX  = VR_UK_TIMELINE_MAX
} vr_timeline_id;

/**
 * Used by trackers that should not be added to a timeline (@ref vr_timeline_system_add_tracker).
 */
#define VR_TIMELINE_NONE VR_TIMELINE_MAX

/**
 * Tracker type.
 */
typedef enum vr_timeline_tracker_type {
	VR_TIMELINE_TRACKER_GP   = 0, /**< Tracker used by GP jobs. */
	VR_TIMELINE_TRACKER_PP   = 1, /**< Tracker used by PP jobs. */
	VR_TIMELINE_TRACKER_SOFT = 2, /**< Tracker used by soft jobs. */
	VR_TIMELINE_TRACKER_WAIT = 3, /**< Tracker used for fence wait. */
	VR_TIMELINE_TRACKER_SYNC = 4, /**< Tracker used for sync fence. */
	VR_TIMELINE_TRACKER_MAX  = 5,
} vr_timeline_tracker_type;

/**
 * Tracker activation error.
 */
typedef u32 vr_timeline_activation_error;
#define VR_TIMELINE_ACTIVATION_ERROR_NONE      0
#define VR_TIMELINE_ACTIVATION_ERROR_SYNC_BIT  (1<<1)
#define VR_TIMELINE_ACTIVATION_ERROR_FATAL_BIT (1<<0)

/**
 * Type used to represent a point on a timeline.
 */
typedef u32 vr_timeline_point;

/**
 * Used to represent that no point on a timeline.
 */
#define VR_TIMELINE_NO_POINT ((vr_timeline_point) 0)

/**
 * The maximum span of points on a timeline.  A timeline will be considered full if the difference
 * between the oldest and newest points is equal or larger to this value.
 */
#define VR_TIMELINE_MAX_POINT_SPAN 65536

/**
 * Magic value used to assert on validity of trackers.
 */
#define VR_TIMELINE_TRACKER_MAGIC 0xabcdabcd

struct vr_timeline;
struct vr_timeline_waiter;
struct vr_timeline_tracker;

/**
 * Timeline fence.
 */
struct vr_timeline_fence {
	vr_timeline_point points[VR_TIMELINE_MAX]; /**< For each timeline, a point or VR_TIMELINE_NO_POINT. */
	s32                 sync_fd;                   /**< A file descriptor representing a sync fence, or -1. */
};

/**
 * Timeline system.
 *
 * The Timeline system has a set of timelines associated with a session.
 */
struct vr_timeline_system {
	struct vr_spinlock_reentrant *spinlock;   /**< Spin lock protecting the timeline system */
	struct vr_timeline           *timelines[VR_TIMELINE_MAX]; /**< The timelines in this system */

	/* Single-linked list of unused waiter objects.  Uses the tracker_next field in tracker. */
	struct vr_timeline_waiter    *waiter_empty_list;

	struct vr_session_data       *session;    /**< Session that owns this system. */

	vr_bool                       timer_enabled; /**< Set to VR_TRUE if soft job timer should be enabled, VR_FALSE if not. */

	_vr_osk_wait_queue_t         *wait_queue; /**< Wait queue. */

#if defined(CONFIG_SYNC)
	struct sync_timeline           *signaled_sync_tl; /**< Special sync timeline used to create pre-signaled sync fences */
#endif /* defined(CONFIG_SYNC) */
};

/**
 * Timeline.  Each Timeline system will have VR_TIMELINE_MAX timelines.
 */
struct vr_timeline {
	vr_timeline_point           point_next;   /**< The next available point. */
	vr_timeline_point           point_oldest; /**< The oldest point not released. */

	/* Double-linked list of trackers.  Sorted in ascending order by tracker->time_number with
	 * tail pointing to the tracker with the oldest time. */
	struct vr_timeline_tracker *tracker_head;
	struct vr_timeline_tracker *tracker_tail;

	/* Double-linked list of waiters.  Sorted in ascending order by waiter->time_number_wait
	 * with tail pointing to the waiter with oldest wait time. */
	struct vr_timeline_waiter  *waiter_head;
	struct vr_timeline_waiter  *waiter_tail;

	struct vr_timeline_system  *system;       /**< Timeline system this timeline belongs to. */
	enum vr_timeline_id         id;           /**< Timeline type. */

#if defined(CONFIG_SYNC)
	struct sync_timeline         *sync_tl;      /**< Sync timeline that corresponds to this timeline. */
#endif /* defined(CONFIG_SYNC) */

	/* The following fields are used to time out soft job trackers. */
	_vr_osk_wq_delayed_work_t  *delayed_work;
	vr_bool                     timer_active;
};

/**
 * Timeline waiter.
 */
struct vr_timeline_waiter {
	vr_timeline_point           point;         /**< Point on timeline we are waiting for to be released. */
	struct vr_timeline_tracker *tracker;       /**< Tracker that is waiting. */

	struct vr_timeline_waiter  *timeline_next; /**< Next waiter on timeline's waiter list. */
	struct vr_timeline_waiter  *timeline_prev; /**< Previous waiter on timeline's waiter list. */

	struct vr_timeline_waiter  *tracker_next;  /**< Next waiter on tracker's waiter list. */
};

/**
 * Timeline tracker.
 */
struct vr_timeline_tracker {
	VR_DEBUG_CODE(u32            magic); /**< Should always be VR_TIMELINE_TRACKER_MAGIC for a valid tracker. */

	vr_timeline_point            point; /**< Point on timeline for this tracker */

	struct vr_timeline_tracker  *timeline_next; /**< Next tracker on timeline's tracker list */
	struct vr_timeline_tracker  *timeline_prev; /**< Previous tracker on timeline's tracker list */

	u32                            trigger_ref_count; /**< When zero tracker will be activated */
	vr_timeline_activation_error activation_error;  /**< Activation error. */
	struct vr_timeline_fence     fence;             /**< Fence used to create this tracker */

	/* Single-linked list of waiters.  Sorted in order of insertions with
	 * tail pointing to first waiter. */
	struct vr_timeline_waiter   *waiter_head;
	struct vr_timeline_waiter   *waiter_tail;

#if defined(CONFIG_SYNC)
	/* These are only used if the tracker is waiting on a sync fence. */
	struct vr_timeline_waiter   *waiter_sync; /**< A direct pointer to timeline waiter representing sync fence. */
	struct sync_fence_waiter       sync_fence_waiter; /**< Used to connect sync fence and tracker in sync fence wait callback. */
	struct sync_fence             *sync_fence;   /**< The sync fence this tracker is waiting on. */
	_vr_osk_list_t               sync_fence_cancel_list; /**< List node used to cancel sync fence waiters. */
#endif /* defined(CONFIG_SYNC) */

	struct vr_timeline_system   *system;       /**< Timeline system. */
	struct vr_timeline          *timeline;     /**< Timeline, or NULL if not on a timeline. */
	enum vr_timeline_tracker_type type;        /**< Type of tracker. */
	void                          *job;          /**< Owner of tracker. */

	/* The following fields are used to time out soft job trackers. */
	u32                           os_tick_create;
	u32                           os_tick_activate;
	vr_bool                     timer_active;
};

/**
 * What follows is a set of functions to check the state of a timeline and to determine where on a
 * timeline a given point is.  Most of these checks will translate the timeline so the oldest point
 * on the timeline is aligned with zero.  Remember that all of these calculation are done on
 * unsigned integers.
 *
 * The following example illustrates the three different states a point can be in.  The timeline has
 * been translated to put the oldest point at zero:
 *
 *
 *
 *                               [ point is in forbidden zone ]
 *                                          64k wide
 *                                VR_TIMELINE_MAX_POINT_SPAN
 *
 *    [ point is on timeline     )                            ( point is released ]
 *
 *    0--------------------------##############################--------------------2^32 - 1
 *    ^                          ^
 *    \                          |
 *     oldest point on timeline  |
 *                               \
 *                                next point on timeline
 */

/**
 * Compare two timeline points
 *
 * Returns true if a is after b, false if a is before or equal to b.
 *
 * This funcion ignores VR_TIMELINE_MAX_POINT_SPAN. Wrapping is supported and
 * the result will be correct if the points is less then UINT_MAX/2 apart.
 *
 * @param a Point on timeline
 * @param b Point on timeline
 * @return VR_TRUE if a is after b
 */
VR_STATIC_INLINE vr_bool vr_timeline_point_after(vr_timeline_point a, vr_timeline_point b)
{
	return 0 > ((s32)b) - ((s32)a);
}

/**
 * Check if a point is on timeline.  A point is on a timeline if it is greater than, or equal to,
 * the oldest point, and less than the next point.
 *
 * @param timeline Timeline.
 * @param point Point on timeline.
 * @return VR_TRUE if point is on timeline, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_timeline_is_point_on(struct vr_timeline *timeline, vr_timeline_point point)
{
	VR_DEBUG_ASSERT_POINTER(timeline);
	VR_DEBUG_ASSERT(VR_TIMELINE_NO_POINT != point);

	return (point - timeline->point_oldest) < (timeline->point_next - timeline->point_oldest);
}

/**
 * Check if a point has been released.  A point is released if it is older than the oldest point on
 * the timeline, newer than the next point, and also not in the forbidden zone.
 *
 * @param timeline Timeline.
 * @param point Point on timeline.
 * @return VR_TRUE if point has been release, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_timeline_is_point_released(struct vr_timeline *timeline, vr_timeline_point point)
{
	vr_timeline_point point_norvrzed;
	vr_timeline_point next_norvrzed;

	VR_DEBUG_ASSERT_POINTER(timeline);
	VR_DEBUG_ASSERT(VR_TIMELINE_NO_POINT != point);

	point_norvrzed = point - timeline->point_oldest;
	next_norvrzed = timeline->point_next - timeline->point_oldest;

	return point_norvrzed > (next_norvrzed + VR_TIMELINE_MAX_POINT_SPAN);
}

/**
 * Check if a point is valid.  A point is valid if is on the timeline or has been released.
 *
 * @param timeline Timeline.
 * @param point Point on timeline.
 * @return VR_TRUE if point is valid, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_timeline_is_point_valid(struct vr_timeline *timeline, vr_timeline_point point)
{
	VR_DEBUG_ASSERT_POINTER(timeline);
	return vr_timeline_is_point_on(timeline, point) || vr_timeline_is_point_released(timeline, point);
}

/**
 * Check if timeline is empty (has no points on it).  A timeline is empty if next == oldest.
 *
 * @param timeline Timeline.
 * @return VR_TRUE if timeline is empty, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_timeline_is_empty(struct vr_timeline *timeline)
{
	VR_DEBUG_ASSERT_POINTER(timeline);
	return timeline->point_next == timeline->point_oldest;
}

/**
 * Check if timeline is full.  A valid timeline cannot span more than 64k points (@ref
 * VR_TIMELINE_MAX_POINT_SPAN).
 *
 * @param timeline Timeline.
 * @return VR_TRUE if timeline is full, VR_FALSE if not.
 */
VR_STATIC_INLINE vr_bool vr_timeline_is_full(struct vr_timeline *timeline)
{
	VR_DEBUG_ASSERT_POINTER(timeline);
	return VR_TIMELINE_MAX_POINT_SPAN <= (timeline->point_next - timeline->point_oldest);
}

/**
 * Create a new timeline system.
 *
 * @param session The session this timeline system will belong to.
 * @return New timeline system.
 */
struct vr_timeline_system *vr_timeline_system_create(struct vr_session_data *session);

/**
 * Abort timeline system.
 *
 * This will release all pending waiters in the timeline system causing all trackers to be
 * activated.
 *
 * @param system Timeline system to abort all jobs from.
 */
void vr_timeline_system_abort(struct vr_timeline_system *system);

/**
 * Destroy an empty timeline system.
 *
 * @note @ref vr_timeline_system_abort() should be called prior to this function.
 *
 * @param system Timeline system to destroy.
 */
void vr_timeline_system_destroy(struct vr_timeline_system *system);

/**
 * Stop the soft job timer.
 *
 * @param system Timeline system
 */
void vr_timeline_system_stop_timer(struct vr_timeline_system *system);

/**
 * Add a tracker to a timeline system and optionally also on a timeline.
 *
 * Once added to the timeline system, the tracker is guaranteed to be activated.  The tracker can be
 * activated before this function returns.  Thus, it is also possible that the tracker is released
 * before this function returns, depending on the tracker type.
 *
 * @note Tracker must be initialized (@ref vr_timeline_tracker_init) before being added to the
 * timeline system.
 *
 * @param system Timeline system the tracker will be added to.
 * @param tracker The tracker to be added.
 * @param timeline_id Id of the timeline the tracker will be added to, or
 *                    VR_TIMELINE_NONE if it should not be added on a timeline.
 * @return Point on timeline identifying this tracker, or VR_TIMELINE_NO_POINT if not on timeline.
 */
vr_timeline_point vr_timeline_system_add_tracker(struct vr_timeline_system *system,
        struct vr_timeline_tracker *tracker,
        enum vr_timeline_id timeline_id);

/**
 * Get latest point on timeline.
 *
 * @param system Timeline system.
 * @param timeline_id Id of timeline to get latest point from.
 * @return Latest point on timeline, or VR_TIMELINE_NO_POINT if the timeline is empty.
 */
vr_timeline_point vr_timeline_system_get_latest_point(struct vr_timeline_system *system,
        enum vr_timeline_id timeline_id);

/**
 * Initialize tracker.
 *
 * Must be called before tracker is added to timeline system (@ref vr_timeline_system_add_tracker).
 *
 * @param tracker Tracker to initialize.
 * @param type Type of tracker.
 * @param fence Fence used to set up dependencies for tracker.
 * @param job Pointer to job struct this tracker is associated with.
 */
void vr_timeline_tracker_init(struct vr_timeline_tracker *tracker,
                                vr_timeline_tracker_type type,
                                struct vr_timeline_fence *fence,
                                void *job);

/**
 * Grab trigger ref count on tracker.
 *
 * This will prevent tracker from being activated until the trigger ref count reaches zero.
 *
 * @note Tracker must have been initialized (@ref vr_timeline_tracker_init).
 *
 * @param system Timeline system.
 * @param tracker Tracker.
 */
void vr_timeline_system_tracker_get(struct vr_timeline_system *system, struct vr_timeline_tracker *tracker);

/**
 * Release trigger ref count on tracker.
 *
 * If the trigger ref count reaches zero, the tracker will be activated.
 *
 * @param system Timeline system.
 * @param tracker Tracker.
 * @param activation_error Error bitmask if activated with error, or VR_TIMELINE_ACTIVATION_ERROR_NONE if no error.
 * @return Scheduling bitmask.
 */
vr_scheduler_mask vr_timeline_system_tracker_put(struct vr_timeline_system *system, struct vr_timeline_tracker *tracker, vr_timeline_activation_error activation_error);

/**
 * Release a tracker from the timeline system.
 *
 * This is used to signal that the job being tracker is finished, either due to normal circumstances
 * (job complete/abort) or due to a timeout.
 *
 * We may need to schedule some subsystems after a tracker has been released and the returned
 * bitmask will tell us if it is necessary.  If the return value is non-zero, this value needs to be
 * sent as an input parameter to @ref vr_scheduler_schedule_from_mask() to do the scheduling.
 *
 * @note Tracker must have been activated before being released.
 * @warning Not calling @ref vr_scheduler_schedule_from_mask() after releasing a tracker can lead
 * to a deadlock.
 *
 * @param tracker Tracker being released.
 * @return Scheduling bitmask.
 */
vr_scheduler_mask vr_timeline_tracker_release(struct vr_timeline_tracker *tracker);

/**
 * Copy data from a UK fence to a Timeline fence.
 *
 * @param fence Timeline fence.
 * @param uk_fence UK fence.
 */
void vr_timeline_fence_copy_uk_fence(struct vr_timeline_fence *fence, _vr_uk_fence_t *uk_fence);

#define VR_TIMELINE_DEBUG_FUNCTIONS
#if defined(VR_TIMELINE_DEBUG_FUNCTIONS)

/**
 * Tracker state.  Used for debug printing.
 */
typedef enum vr_timeline_tracker_state {
	VR_TIMELINE_TS_INIT    = 0,
	VR_TIMELINE_TS_WAITING = 1,
	VR_TIMELINE_TS_ACTIVE  = 2,
	VR_TIMELINE_TS_FINISH  = 3,
} vr_timeline_tracker_state;

/**
 * Get tracker state.
 *
 * @param tracker Tracker to check.
 * @return State of tracker.
 */
vr_timeline_tracker_state vr_timeline_debug_get_tracker_state(struct vr_timeline_tracker *tracker);

/**
 * Print debug information about tracker.
 *
 * @param tracker Tracker to print.
 */
void vr_timeline_debug_print_tracker(struct vr_timeline_tracker *tracker);

/**
 * Print debug information about timeline.
 *
 * @param timeline Timeline to print.
 */
void vr_timeline_debug_print_timeline(struct vr_timeline *timeline);

/**
 * Print debug information about timeline system.
 *
 * @param system Timeline system to print.
 */
void vr_timeline_debug_print_system(struct vr_timeline_system *system);

#endif /* defined(VR_TIMELINE_DEBUG_FUNCTIONS) */

#endif /* __VR_TIMELINE_H__ */
