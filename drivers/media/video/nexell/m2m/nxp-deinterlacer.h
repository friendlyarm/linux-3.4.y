#ifndef _NXP_DEINTERLACER_H
#define _NXP_DEINTERLACER_H

#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/kfifo.h>
#include <linux/workqueue.h>
#include <linux/list.h>

typedef struct {
	int fence_fd;
	int idx;
} FD_LIST;

typedef struct{
	struct work_struct	w_proc;
	struct workqueue_struct *wq_proc;

	atomic_t open_count;

	atomic_t	status; 
	atomic_t	hw_status; 
	atomic_t	even_or_odd;
	atomic_t	exit_flag;	

	atomic_t	src_buf_count;
	atomic_t	dst_buf_count;

	atomic_t	dq_init_pending;

	wait_queue_head_t	wq_start;
	wait_queue_head_t	wq_end;

	wait_queue_head_t wq_dst;

	spinlock_t queue_lock;
	spinlock_t dequeue_lock;
	spinlock_t dequeue_unused_lock;
	spinlock_t queue_fd_list_lock;

	spinlock_t ion_lock;
	spinlock_t irq_lock;

	spinlock_t process_lock;

	struct kfifo data_queue;
	struct kfifo data_dequeue_unused;
	struct kfifo data_dequeue;
	struct kfifo fd_queue;
	
	CONTEXT	ctx;

	Q_BUF	qbuf_temp[MAX_BUFFER_PLANES];

	DQ_BUF	*dst_dqbuf_all;
	DQ_BUF	*dst_dqbuf;

	DQ_SET	dqset;
	
	int irq;

	struct mutex	mtx_proc;

	struct mutex read_lock;
	struct mutex write_lock;

	seqlock_t		wq_lock;

	int fence_fd;

	EXE_CONTEXT e_ctx;
	
	struct sw_sync_timeline *dq_timeline;
  int    dq_timeline_max;
	struct sw_sync_timeline *q_timeline;
  int    q_timeline_max;

	struct sync_pt *pt;
	struct sync_fence *fence;

	bool is_waitting;
	int fd_idx;

	FD_LIST fd_list;
} nxp_deinterlace;

#if 0
typedef enum
{
	ACT_COPY=0,
	ACT_DIRECT,
	ACT_DIRECT_FD,
	ACT_THREAD
} ACT_MODE;
#endif

enum
{
	PROCESSING_STOP,
	PROCESSING_START
};

enum 
{
	FRAME_PREV=0,
	FRAME_CURR,
	FRAME_NEXT
};

enum
{
	QUEUE_DATA = 0,
	BUFFER_DATA,
};

enum E_Status{
	PROCESSING_READY,
	PROCESSING_RUNNING,
	PROCESSING_PENDING,
	PROCESSING_FINISH
};

enum {
	DEINTERLACING_START,
	DEINTERLACING_STOP
};

//#define DEBUG_ZONE

#if defined( DEBUG_ZONE )
	#define tag_msg(fmt, args...) \
		printk(fmt, ##args)
	#define dbg_msg(fmt, args...) \
		printk(fmt, ##args)
#else
	#define tag_msg(fmt, args...)
	#define dbg_msg(fmt, args...)
#endif

static int insert_dequeue_frame(void);
static int start_deinterlacing(nxp_deinterlace *m_ctx);
static int processing_3plane(DQ_BUF *dqbuf);
static int processing_2plane(DQ_BUF *dqbuf);

#endif
