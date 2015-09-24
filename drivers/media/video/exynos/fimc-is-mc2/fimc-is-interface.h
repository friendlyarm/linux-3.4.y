#ifndef FIMC_IS_INTERFACE_H
#define FIMC_IS_INTERFACE_H

#include "fimc-is-metadata.h"
#include "fimc-is-framemgr.h"
#include "fimc-is-video.h"

/*#define TRACE_WORK*/
/* cam_ctrl : 1
   shot :     2 */
#define TRACE_WORK_ID_CAMCTRL	0x1
#define TRACE_WORK_ID_SHOT	0x2
#define TRACE_WORK_ID_GENERAL	0x4
#define TRACE_WORK_ID_SCC	0x8
#define TRACE_WORK_ID_SCP	0x10
#define TRACE_WORK_ID_META	0x20
#define TRACE_WORK_ID_MASK	0xFF

#define MAX_NBLOCKING_COUNT	3
#define MAX_WORK_COUNT		10

#define TRY_RECV_AWARE_COUNT	100

#define LOWBIT_OF(num)	(num >= 32 ? 0 : (u32)1<<num)
#define HIGHBIT_OF(num)	(num >= 32 ? (u32)1<<(num-32) : 0)

enum fimc_is_interface_state {
	IS_IF_STATE_OPEN,
	IS_IF_STATE_START,
	IS_IF_STATE_BUSY
};

enum interrupt_map {
	INTR_GENERAL		= 0,
	INTR_ISP_FDONE		= 1,
	INTR_SCC_FDONE		= 2,
	INTR_DNR_FDONE		= 3,
	INTR_SCP_FDONE		= 4,
	/* 5 is ISP YUV DONE */
	INTR_META_DONE		= 6,
	INTR_SHOT_DONE		= 7,
	INTR_MAX_MAP
};

enum streaming_state {
	IS_IF_STREAMING_INIT,
	IS_IF_STREAMING_OFF,
	IS_IF_STREAMING_ON
};

enum processing_state {
	IS_IF_PROCESSING_INIT,
	IS_IF_PROCESSING_OFF,
	IS_IF_PROCESSING_ON
};

enum pdown_ready_state {
	IS_IF_POWER_DOWN_READY,
	IS_IF_POWER_DOWN_NREADY
};

struct fimc_is_msg {
	u32	id;
	u32	command;
	u32	instance;
	u32	parameter1;
	u32	parameter2;
	u32	parameter3;
	u32	parameter4;
};

struct fimc_is_work {
	struct list_head		list;
	struct fimc_is_msg		msg;
	u32				fcount;
	struct fimc_is_frame_shot	*frame;
};

struct fimc_is_work_list {
	u32				id;
	struct fimc_is_work		work[MAX_WORK_COUNT];
	spinlock_t			slock_free;
	spinlock_t			slock_request;
	struct list_head		work_free_head;
	u32				work_free_cnt;
	struct list_head		work_request_head;
	u32				work_request_cnt;
	wait_queue_head_t		wait_queue;
};

struct fimc_is_interface {
	void __iomem			*regs;
	struct is_common_reg __iomem	*com_regs;
	unsigned long			state;
	/* this spinlock is needed for data coincidence.
	it need to update SCU tag between different thread */
	spinlock_t			slock_state;
	spinlock_t			process_barrier;
	struct mutex			request_barrier;

	wait_queue_head_t		init_wait_queue;
	wait_queue_head_t		wait_queue;
	struct fimc_is_msg		reply;

	struct work_struct		work_queue[INTR_MAX_MAP];
	struct fimc_is_work_list	work_list[INTR_MAX_MAP];

	/* sensor streaming flag */
	enum streaming_state		streaming;
	/* firmware processing flag */
	enum processing_state		processing;
	/* frrmware power down ready flag */
	enum pdown_ready_state		pdown_ready;

	struct fimc_is_framemgr		*framemgr;

	void				*core;
	struct fimc_is_video_common	*video_sensor;
	struct fimc_is_video_common	*video_isp;
	struct fimc_is_video_common	*video_scc;
	struct fimc_is_video_common	*video_scp;

	struct fimc_is_work_list	nblk_cam_ctrl;

	struct camera2_uctl		isp_peri_ctl;
};

int fimc_is_interface_probe(struct fimc_is_interface *this,
	struct fimc_is_framemgr *framemgr,
	u32 regs,
	u32 irq,
	void *core_data);
int fimc_is_interface_open(struct fimc_is_interface *this);
int fimc_is_interface_close(struct fimc_is_interface *this);

/*for debugging*/
int print_fre_work_list(struct fimc_is_work_list *this);
int print_req_work_list(struct fimc_is_work_list *this);

int fimc_is_hw_print(struct fimc_is_interface *this);
int fimc_is_hw_enum(struct fimc_is_interface *this);
int fimc_is_hw_open(struct fimc_is_interface *this,
	u32 instance, u32 sensor, u32 channel, u32 ext,
	u32 *mwidth, u32 *mheight);
int fimc_is_hw_saddr(struct fimc_is_interface *interface,
	u32 instance, u32 *setfile_addr);
int fimc_is_hw_setfile(struct fimc_is_interface *interface,
	u32 instance);
int fimc_is_hw_process_on(struct fimc_is_interface *interface,
	u32 instance);
int fimc_is_hw_process_off(struct fimc_is_interface *interface,
	u32 instance);
int fimc_is_hw_stream_on(struct fimc_is_interface *interface,
	u32 instance);
int fimc_is_hw_stream_off(struct fimc_is_interface *interface,
	u32 instance);
int fimc_is_hw_s_param(struct fimc_is_interface *interface,
	u32 instance, u32 indexes, u32 lindex, u32 hindex);
int fimc_is_hw_a_param(struct fimc_is_interface *this,
	u32 instance, u32 mode, u32 sub_mode);
int fimc_is_hw_f_param(struct fimc_is_interface *interface,
	u32 instance);
int fimc_is_hw_g_capability(struct fimc_is_interface *this,
	u32 instance, u32 address);
int fimc_is_hw_cfg_mem(struct fimc_is_interface *interface,
	u32 instance, u32 address, u32 size);
int fimc_is_hw_power_down(struct fimc_is_interface *interface,
	u32 instance);

int fimc_is_hw_shot_nblk(struct fimc_is_interface *this,
	u32 instance, u32 bayer, u32 shot, u32 fcount, u32 rcount);
int fimc_is_hw_s_camctrl_nblk(struct fimc_is_interface *this,
	u32 instance, u32 address, u32 fcount);

#endif
