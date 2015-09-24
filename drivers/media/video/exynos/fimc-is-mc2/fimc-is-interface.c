#include <linux/workqueue.h>

#include "fimc-is-time.h"
#include "fimc-is-core.h"
#include "fimc-is-cmd.h"
#include "fimc-is-regs.h"
#include "fimc-is-err.h"

#include "fimc-is-interface.h"

u32 __iomem *notify_fcount;

#define init_request_barrier(itf) mutex_init(&itf->request_barrier)
#define enter_request_barrier(itf) mutex_lock(&itf->request_barrier);
#define exit_request_barrier(itf) mutex_unlock(&itf->request_barrier);
#define init_process_barrier(itf) spin_lock_init(&itf->process_barrier);
#define enter_process_barrier(itf) spin_lock_irq(&itf->process_barrier);
#define exit_process_barrier(itf) spin_unlock_irq(&itf->process_barrier);

int print_fre_work_list(struct fimc_is_work_list *this)
{
	struct list_head *temp;
	struct fimc_is_work *work;

	if (!(this->id & TRACE_WORK_ID_MASK))
		return 0;

	printk(KERN_CONT "[INF] fre(%02X, %02d) :",
		this->id, this->work_free_cnt);

	list_for_each(temp, &this->work_free_head) {
		work = list_entry(temp, struct fimc_is_work, list);
		printk(KERN_CONT "%X(%d)->", work->msg.command, work->fcount);
	}

	printk(KERN_CONT "X\n");

	return 0;
}

static int set_free_work(struct fimc_is_work_list *this,
	struct fimc_is_work *work)
{
	int ret = 0;
	unsigned long flags;

	if (work) {
		spin_lock_irqsave(&this->slock_free, flags);

		list_add_tail(&work->list, &this->work_free_head);
		this->work_free_cnt++;
#ifdef TRACE_WORK
		print_fre_work_list(this);
#endif

		spin_unlock_irqrestore(&this->slock_free, flags);
	} else {
		ret = -EFAULT;
		err("item is null ptr\n");
	}

	return ret;
}

static int get_free_work(struct fimc_is_work_list *this,
	struct fimc_is_work **work)
{
	int ret = 0;
	unsigned long flags;

	if (work) {
		spin_lock_irqsave(&this->slock_free, flags);

		if (this->work_free_cnt) {
			*work = container_of(this->work_free_head.next,
					struct fimc_is_work, list);
			list_del(&(*work)->list);
			this->work_free_cnt--;
		} else
			*work = NULL;

		spin_unlock_irqrestore(&this->slock_free, flags);
	} else {
		ret = EFAULT;
		err("item is null ptr");
	}

	return ret;
}

static int get_free_work_irq(struct fimc_is_work_list *this,
	struct fimc_is_work **work)
{
	int ret = 0;

	if (work) {
		spin_lock(&this->slock_free);

		if (this->work_free_cnt) {
			*work = container_of(this->work_free_head.next,
					struct fimc_is_work, list);
			list_del(&(*work)->list);
			this->work_free_cnt--;
		} else
			*work = NULL;

		spin_unlock(&this->slock_free);
	} else {
		ret = EFAULT;
		err("item is null ptr");
	}

	return ret;
}

int print_req_work_list(struct fimc_is_work_list *this)
{
	struct list_head *temp;
	struct fimc_is_work *work;

	if (!(this->id & TRACE_WORK_ID_MASK))
		return 0;

	printk(KERN_CONT "[INF] req(%02X, %02d) :",
		this->id, this->work_request_cnt);

	list_for_each(temp, &this->work_request_head) {
		work = list_entry(temp, struct fimc_is_work, list);
		printk(KERN_CONT "%X(%d)->", work->msg.command, work->fcount);
	}

	printk(KERN_CONT "X\n");

	return 0;
}

static int set_req_work(struct fimc_is_work_list *this,
	struct fimc_is_work *work)
{
	int ret = 0;
	unsigned long flags;

	if (work) {
		spin_lock_irqsave(&this->slock_request, flags);

		list_add_tail(&work->list, &this->work_request_head);
		this->work_request_cnt++;
#ifdef TRACE_WORK
		print_req_work_list(this);
#endif

		spin_unlock_irqrestore(&this->slock_request, flags);
	} else {
		ret = EFAULT;
		err("item is null ptr\n");
	}

	return ret;
}

static int set_req_work_irq(struct fimc_is_work_list *this,
	struct fimc_is_work *work)
{
	int ret = 0;

	if (work) {
		spin_lock(&this->slock_request);

		list_add_tail(&work->list, &this->work_request_head);
		this->work_request_cnt++;
#ifdef TRACE_WORK
		print_req_work_list(this);
#endif

		spin_unlock(&this->slock_request);
	} else {
		ret = EFAULT;
		err("item is null ptr\n");
	}

	return ret;
}

static int get_req_work(struct fimc_is_work_list *this,
	struct fimc_is_work **work)
{
	int ret = 0;
	unsigned long flags;

	if (work) {
		spin_lock_irqsave(&this->slock_request, flags);

		if (this->work_request_cnt) {
			*work = container_of(this->work_request_head.next,
					struct fimc_is_work, list);
			list_del(&(*work)->list);
			this->work_request_cnt--;
		} else
			*work = NULL;

		spin_unlock_irqrestore(&this->slock_request, flags);
	} else {
		ret = EFAULT;
		err("item is null ptr\n");
	}

	return ret;
}

static void init_work_list(struct fimc_is_work_list *this, u32 id, u32 count)
{
	u32 i;

	this->id = id;
	this->work_free_cnt	= 0;
	this->work_request_cnt	= 0;
	INIT_LIST_HEAD(&this->work_free_head);
	INIT_LIST_HEAD(&this->work_request_head);
	spin_lock_init(&this->slock_free);
	spin_lock_init(&this->slock_request);
	for (i = 0; i < count; ++i)
		set_free_work(this, &this->work[i]);

	init_waitqueue_head(&this->wait_queue);
}

static void set_state(struct fimc_is_interface *this,
	unsigned long state)
{
	spin_lock(&this->slock_state);
	set_bit(state, &this->state);
	spin_unlock(&this->slock_state);
}

static void clr_state(struct fimc_is_interface *this,
	unsigned long state)
{
	spin_lock(&this->slock_state);
	clear_bit(state, &this->state);
	spin_unlock(&this->slock_state);
}

static int test_state(struct fimc_is_interface *this,
	unsigned long state)
{
	int ret = 0;

	spin_lock(&this->slock_state);
	ret = test_bit(state, &this->state);
	spin_unlock(&this->slock_state);

	return ret;
}

static int wait_idlestate(struct fimc_is_interface *this)
{
	int ret = 0;

	ret = wait_event_timeout(this->wait_queue,
		!test_state(this, IS_IF_STATE_BUSY), FIMC_IS_COMMAND_TIMEOUT);

	if (ret)
		ret = 0;
	else {
		err("timeout");
		ret = -ETIME;
	}

	return ret;
}

static int wait_initstate(struct fimc_is_interface *this)
{
	int ret = 0;

	ret = wait_event_timeout(this->init_wait_queue,
		test_state(this, IS_IF_STATE_START), FIMC_IS_STARTUP_TIMEOUT);

	if (ret)
		ret = 0;
	else {
		err("timeout");
		ret = -ETIME;
	}

	return ret;
}

static int testnset_state(struct fimc_is_interface *this,
	unsigned long state)
{
	int ret = 0;

	spin_lock(&this->slock_state);

	if (test_bit(state, &this->state)) {
		ret = -EINVAL;
		goto exit;
	}
	set_bit(state, &this->state);

exit:
	spin_unlock(&this->slock_state);
	return ret;
}

static int testnclr_state(struct fimc_is_interface *this,
	unsigned long state)
{
	int ret = 0;

	spin_lock(&this->slock_state);

	if (!test_bit(state, &this->state)) {
		ret = -EINVAL;
		goto exit;
	}
	clear_bit(state, &this->state);

exit:
	spin_unlock(&this->slock_state);
	return ret;
}

static void testnclr_wakeup(struct fimc_is_interface *this)
{
	int ret = 0;

	ret = testnclr_state(this, IS_IF_STATE_BUSY);
	if (ret)
		err("current state is not invalid(%ld)", this->state);
	else
		wake_up(&this->wait_queue);
}

static int waiting_is_ready(struct fimc_is_interface *interface)
{
	int ret = 0;
	u32 try_count = TRY_RECV_AWARE_COUNT;
	u32 cfg = readl(interface->regs + INTMSR0);
	u32 status = INTMSR0_GET_INTMSD0(cfg);

	while (status) {
		cfg = readl(interface->regs + INTMSR0);
		status = INTMSR0_GET_INTMSD0(cfg);

		if (try_count-- == 0) {
			try_count = TRY_RECV_AWARE_COUNT;
			err("INTMSR0's 0 bit is not cleared.");
			ret = -EINVAL;
			break;
		}
	}

	return ret;
}

static void send_interrupt(struct fimc_is_interface *interface)
{
	writel(INTGR0_INTGD0, interface->regs + INTGR0);
}

static int fimc_is_set_cmd(struct fimc_is_interface *itf,
	struct fimc_is_msg *msg)
{
	int ret = 0;
	bool block_io, send_cmd;

	dbg_interface("TP#1\n");
	enter_request_barrier(itf);
	dbg_interface("TP#2\n");

	switch (msg->command) {
	case HIC_STREAM_ON:
		if (itf->streaming == IS_IF_STREAMING_ON) {
			send_cmd = false;
		} else {
			send_cmd = true;
			block_io = true;
		}
		break;
	case HIC_STREAM_OFF:
		if (itf->streaming == IS_IF_STREAMING_OFF) {
			send_cmd = false;
		} else {
			send_cmd = true;
			block_io = true;
		}
		break;
	case HIC_PROCESS_START:
		if (itf->processing == IS_IF_PROCESSING_ON) {
			send_cmd = false;
		} else {
			send_cmd = true;
			block_io = true;
		}
		break;
	case HIC_PROCESS_STOP:
		if (itf->processing == IS_IF_PROCESSING_OFF) {
			send_cmd = false;
		} else {
			send_cmd = true;
			block_io = true;
		}
		break;
	case HIC_POWER_DOWN:
		if (itf->pdown_ready == IS_IF_POWER_DOWN_READY) {
			send_cmd = false;
		} else {
			send_cmd = true;
			block_io = true;
		}
		break;
	case HIC_OPEN_SENSOR:
	case HIC_GET_SET_FILE_ADDR:
	case HIC_SET_PARAMETER:
	case HIC_PREVIEW_STILL:
	case HIC_GET_STATIC_METADATA:
	case HIC_SET_A5_MEM_ACCESS:
	case HIC_SET_CAM_CONTROL:
		send_cmd = true;
		block_io = true;
		break;
	case HIC_SHOT:
	case ISR_DONE:
		send_cmd = true;
		block_io = false;
		break;
	default:
		send_cmd = true;
		block_io = true;
		break;
	}

	if (!send_cmd) {
		dbg_interface("skipped\n");
		goto exit;
	}

	enter_process_barrier(itf);

	ret = waiting_is_ready(itf);
	if (ret) {
		err("waiting for ready is fail");
		ret = -EBUSY;
		exit_process_barrier(itf);
		goto exit;
	}

	set_state(itf, IS_IF_STATE_BUSY);
	itf->com_regs->hicmd = msg->command;
	itf->com_regs->hic_sensorid = msg->instance;
	itf->com_regs->hic_param1 = msg->parameter1;
	itf->com_regs->hic_param2 = msg->parameter2;
	itf->com_regs->hic_param3 = msg->parameter3;
	itf->com_regs->hic_param4 = msg->parameter4;
	send_interrupt(itf);

	exit_process_barrier(itf);

	if (!block_io)
		goto exit;

	ret = wait_idlestate(itf);
	if (ret) {
		err("%d command is timeout", msg->command);
		clr_state(itf, IS_IF_STATE_BUSY);
		ret = -ETIME;
		goto exit;
	}

	if (itf->reply.command == ISR_DONE) {
		switch (msg->command) {
		case HIC_STREAM_ON:
			itf->streaming = IS_IF_STREAMING_ON;
			break;
		case HIC_STREAM_OFF:
			itf->streaming = IS_IF_STREAMING_OFF;
			break;
		case HIC_PROCESS_START:
			itf->processing = IS_IF_PROCESSING_ON;
			break;
		case HIC_PROCESS_STOP:
			itf->processing = IS_IF_PROCESSING_OFF;
			break;
		case HIC_POWER_DOWN:
			itf->pdown_ready = IS_IF_POWER_DOWN_READY;
			break;
		case HIC_OPEN_SENSOR:
			if (itf->reply.parameter1 == HIC_POWER_DOWN) {
				err("firmware power down");
				itf->pdown_ready = IS_IF_POWER_DOWN_READY;
				ret = -ECANCELED;
				goto exit;
			} else
				itf->pdown_ready = IS_IF_POWER_DOWN_NREADY;
			break;
		default:
			break;
		}
	} else {
		err("ISR_NDONE is occured");
		ret = -EINVAL;
	}

exit:
	exit_request_barrier(itf);

	if (ret)
		fimc_is_hw_print(itf);

	return ret;
}

static int fimc_is_set_cmd_shot(struct fimc_is_interface *this,
	struct fimc_is_msg *msg)
{
	int ret = 0;

	enter_process_barrier(this);

	ret = waiting_is_ready(this);
	if (ret) {
		err("waiting for ready is fail");
		ret = -EBUSY;
		goto exit;
	}

	this->com_regs->hicmd = msg->command;
	this->com_regs->hic_sensorid = msg->instance;
	this->com_regs->hic_param1 = msg->parameter1;
	this->com_regs->hic_param2 = msg->parameter2;
	this->com_regs->hic_param3 = msg->parameter3;
	this->com_regs->hic_param4 = msg->parameter4;
	send_interrupt(this);

exit:
	exit_process_barrier(this);
	return ret;
}

static int fimc_is_set_cmd_nblk(struct fimc_is_interface *this,
	struct fimc_is_work *work)
{
	int ret = 0;
	struct fimc_is_msg *msg;

	msg = &work->msg;
	switch (msg->command) {
	case HIC_SET_CAM_CONTROL:
		set_req_work(&this->nblk_cam_ctrl, work);
		break;
	default:
		err("unresolved command\n");
		break;
	}

	enter_process_barrier(this);

	ret = waiting_is_ready(this);
	if (ret) {
		err("waiting for ready is fail");
		ret = -EBUSY;
		goto exit;
	}

	this->com_regs->hicmd = msg->command;
	this->com_regs->hic_sensorid = msg->instance;
	this->com_regs->hic_param1 = msg->parameter1;
	this->com_regs->hic_param2 = msg->parameter2;
	this->com_regs->hic_param3 = msg->parameter3;
	this->com_regs->hic_param4 = msg->parameter4;
	send_interrupt(this);

exit:
	exit_process_barrier(this);
	return ret;
}

static inline void fimc_is_get_cmd(struct fimc_is_interface *itf,
	struct fimc_is_msg *msg, u32 index)
{
	struct is_common_reg __iomem *com_regs = itf->com_regs;

	switch (index) {
	case INTR_GENERAL:
		msg->id = 0;
		msg->command = com_regs->ihcmd;
		msg->instance = com_regs->ihc_sensorid;
		msg->parameter1 = com_regs->ihc_param1;
		msg->parameter2 = com_regs->ihc_param2;
		msg->parameter3 = com_regs->ihc_param3;
		msg->parameter4 = com_regs->ihc_param4;
		break;
	case INTR_SCC_FDONE:
		msg->id = 0;
		msg->command = IHC_FRAME_DONE;
		msg->instance = com_regs->scc_sensor_id;
		msg->parameter1 = com_regs->scc_param1;
		msg->parameter2 = com_regs->scc_param2;
		msg->parameter3 = com_regs->scc_param3;
		msg->parameter4 = 0;
		break;
	case INTR_SCP_FDONE:
		msg->id = 0;
		msg->command = IHC_FRAME_DONE;
		msg->instance = com_regs->scp_sensor_id;
		msg->parameter1 = com_regs->scp_param1;
		msg->parameter2 = com_regs->scp_param2;
		msg->parameter3 = com_regs->scp_param3;
		msg->parameter4 = 0;
		break;
	case INTR_META_DONE:
		msg->id = 0;
		msg->command = IHC_FRAME_DONE;
		msg->instance = com_regs->meta_sensor_id;
		msg->parameter1 = com_regs->meta_param1;
		msg->parameter2 = 0;
		msg->parameter3 = 0;
		msg->parameter4 = 0;
		break;
	case INTR_SHOT_DONE:
		msg->id = 0;
		msg->command = IHC_FRAME_DONE;
		msg->instance = com_regs->shot_sensor_id;
		msg->parameter1 = com_regs->shot_param1;
		msg->parameter2 = com_regs->shot_param2;
		msg->parameter3 = 0;
		msg->parameter4 = 0;
		break;
	default:
		msg->id = 0;
		msg->command = 0;
		msg->instance = 0;
		msg->parameter1 = 0;
		msg->parameter2 = 0;
		msg->parameter3 = 0;
		msg->parameter4 = 0;
		err("unknown command getting\n");
		break;
	}
}

static inline u32 fimc_is_get_intr(struct fimc_is_interface *itf)
{
	u32 status;
	struct is_common_reg __iomem *com_regs = itf->com_regs;

	status = readl(itf->regs + INTMSR1) | com_regs->ihcmd_iflag |
		com_regs->scc_iflag |
		com_regs->scp_iflag |
		com_regs->meta_iflag |
		com_regs->shot_iflag;

	return status;
}

static inline void fimc_is_clr_intr(struct fimc_is_interface *itf,
	u32 index)
{
	struct is_common_reg __iomem *com_regs = itf->com_regs;

	switch (index) {
	case INTR_GENERAL:
		writel((1<<INTR_GENERAL), itf->regs + INTCR1);
		com_regs->ihcmd_iflag = 0;
		break;
	case INTR_SCC_FDONE:
		writel((1<<INTR_SCC_FDONE), itf->regs + INTCR1);
		com_regs->scc_iflag = 0;
		break;
	case INTR_SCP_FDONE:
		writel((1<<INTR_SCP_FDONE), itf->regs + INTCR1);
		com_regs->scp_iflag = 0;
		break;
	case INTR_META_DONE:
		writel((1<<INTR_META_DONE), itf->regs + INTCR1);
		com_regs->meta_iflag = 0;
		break;
	case INTR_SHOT_DONE:
		writel((1<<INTR_SHOT_DONE), itf->regs + INTCR1);
		com_regs->shot_iflag = 0;
		break;
	default:
		err("unknown command clear\n");
		break;
	}
}

static void wq_func_general(struct work_struct *data)
{
	struct fimc_is_interface *itf;
	struct fimc_is_msg *msg;
	struct fimc_is_work *work;
	struct fimc_is_work *nblk_work;

	itf = container_of(data, struct fimc_is_interface,
		work_queue[INTR_GENERAL]);

	get_req_work(&itf->work_list[INTR_GENERAL], &work);
	while (work) {
		msg = &work->msg;
		switch (msg->command) {
		case IHC_GET_SENSOR_NUMBER:
			printk(KERN_INFO "IS version : %d.%d\n",
				ISDRV_VERSION, msg->parameter1);
			set_state(itf, IS_IF_STATE_START);
			wake_up(&itf->init_wait_queue);
			break;
		case ISR_DONE:
			switch (msg->parameter1) {
			case HIC_OPEN_SENSOR:
				dbg_interface("open done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_GET_SET_FILE_ADDR:
				dbg_interface("saddr(%p) done\n",
					(void *)msg->parameter2);
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_LOAD_SET_FILE:
				dbg_interface("setfile done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_SET_A5_MEM_ACCESS:
				dbg_interface("cfgmem done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_PROCESS_START:
				dbg_interface("process_on done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_PROCESS_STOP:
				dbg_interface("process_off done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_STREAM_ON:
				dbg_interface("stream_on done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_STREAM_OFF:
				dbg_interface("stream_off done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_SET_PARAMETER:
				dbg_interface("s_param done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_GET_STATIC_METADATA:
				dbg_interface("g_capability done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_PREVIEW_STILL:
				dbg_interface("a_param(%dx%d) done\n",
					msg->parameter2,
					msg->parameter3);
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			case HIC_POWER_DOWN:
				dbg_interface("powerdown done\n");
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			/*non-blocking command*/
			case HIC_SHOT:
				err("shot done is not acceptable\n");
				break;
			case HIC_SET_CAM_CONTROL:
				/* this code will be used latter */
#if 0
				dbg_interface("camctrl done\n");
				get_req_work(&itf->nblk_cam_ctrl , &nblk_work);
				if (nblk_work) {
					nblk_work->msg.command = ISR_DONE;
					set_free_work(&itf->nblk_cam_ctrl,
						nblk_work);
				} else {
					err("nblk camctrl request is empty");
					print_fre_work_list(
						&itf->nblk_cam_ctrl);
					print_req_work_list(
						&itf->nblk_cam_ctrl);
				}
#else
				err("camctrl is not acceptable\n");
#endif
				break;
			default:
				err("unknown done is invokded\n");
				break;
			}
			break;
		case ISR_NDONE:
			switch (msg->parameter1) {
			case HIC_SHOT:
				err("shot NOT done is not acceptable\n");
				break;
			case HIC_SET_CAM_CONTROL:
				dbg_interface("camctrl NOT done\n");
				get_req_work(&itf->nblk_cam_ctrl , &nblk_work);
				nblk_work->msg.command = ISR_NDONE;
				set_free_work(&itf->nblk_cam_ctrl, nblk_work);
				break;
			case HIC_SET_PARAMETER:
				err("s_param NOT done");
				err("param2 : 0x%08X", msg->parameter2);
				err("param3 : 0x%08X", msg->parameter3);
				err("param4 : 0x%08X", msg->parameter4);
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			default:
				err("a command(%d) not done", msg->parameter1);
				memcpy(&itf->reply, msg,
					sizeof(struct fimc_is_msg));
				testnclr_wakeup(itf);
				break;
			}
			break;
		case IHC_SET_FACE_MARK:
			err("FACE_MARK(%d,%d,%d) is not acceptable\n",
				msg->parameter1,
				msg->parameter2,
				msg->parameter3);
			break;
		case IHC_AA_DONE:
			err("AA_DONE(%d,%d,%d) is not acceptable\n",
				msg->parameter1,
				msg->parameter2,
				msg->parameter3);
			break;
		case IHC_FLASH_READY:
			err("IHC_FLASH_READY is not acceptable");
			break;
		case IHC_NOT_READY:
			err("IHC_NOT_READY is occured, need reset");
			break;
		default:
			err("func_general unknown(0x%08X) end\n", msg->command);
			break;
		}

		set_free_work(&itf->work_list[INTR_GENERAL], work);
		get_req_work(&itf->work_list[INTR_GENERAL], &work);
	}
}

static void wq_func_scc(struct work_struct *data)
{
	struct fimc_is_interface *itf;
	struct fimc_is_msg *msg;
	struct fimc_is_framemgr *isp_framemgr;
	struct fimc_is_framemgr *scc_framemgr;
	struct fimc_is_frame_shot *isp_frame;
	struct fimc_is_frame_shot *scc_frame;
	struct fimc_is_device_ischain *ischain;
	struct fimc_is_ischain_dev *dev;
	struct fimc_is_video_common *video;
	struct fimc_is_work *work;
	unsigned long flags;
	u32 fcount;
	u32 rcount;
	u32 findex;

	itf = container_of(data, struct fimc_is_interface,
		work_queue[INTR_SCC_FDONE]);
	video = itf->video_scc;
	ischain = video->device;
	dev = &ischain->scc;
	isp_framemgr = ischain->framemgr;
	scc_framemgr = &dev->framemgr;

	get_req_work(&itf->work_list[INTR_SCC_FDONE], &work);
	while (work) {
		msg = &work->msg;
		fcount = msg->parameter1;
		rcount = msg->parameter3;

		framemgr_e_barrier_irqs(scc_framemgr, FMGR_IDX_4, flags);

		fimc_is_frame_process_head(scc_framemgr, &scc_frame);
		if (scc_frame && test_bit(REQ_FRAME, &scc_frame->req_flag)) {
			clear_bit(REQ_FRAME, &scc_frame->req_flag);

#ifdef DBG_STREAMING
			printk(KERN_INFO "C%d(%d,%d)\n", scc_frame->index,
				fcount, rcount);
#endif
#ifdef USE_FRAME_SYNC
			findex = scc_frame->stream->findex;
			isp_frame = &isp_framemgr->frame[findex];
			if (isp_frame->fcount != fcount)
				err("scc mismatched(%d, %d)",
					isp_frame->fcount, fcount);

			scc_frame->stream->fcount = fcount;
			scc_frame->stream->rcount = rcount;
			isp_frame->scc_out = FIMC_IS_FOUT_DONE;
#endif

			fimc_is_frame_trans_pro_to_com(scc_framemgr, scc_frame);
			buffer_done(video, scc_frame->index);
		} else
			err("done(%p) is occured without request\n", scc_frame);

		framemgr_x_barrier_irqr(scc_framemgr, FMGR_IDX_4, flags);

		set_free_work(&itf->work_list[INTR_SCC_FDONE], work);
		get_req_work(&itf->work_list[INTR_SCC_FDONE], &work);
	}
}

static void wq_func_scp(struct work_struct *data)
{
	struct fimc_is_interface *itf;
	struct fimc_is_msg *msg;
	struct fimc_is_framemgr *isp_framemgr;
	struct fimc_is_framemgr *scp_framemgr;
	struct fimc_is_frame_shot *isp_frame;
	struct fimc_is_frame_shot *scp_frame;
	struct fimc_is_device_ischain *ischain;
	struct fimc_is_ischain_dev *dev;
	struct fimc_is_video_common *video;
	struct fimc_is_work *work;
	unsigned long flags;
	u32 fcount;
	u32 rcount;
	u32 findex;

	itf = container_of(data, struct fimc_is_interface,
		work_queue[INTR_SCP_FDONE]);
	video = itf->video_scp;
	ischain = video->device;
	dev = &ischain->scp;
	isp_framemgr = ischain->framemgr;
	scp_framemgr = &dev->framemgr;

	get_req_work(&itf->work_list[INTR_SCP_FDONE], &work);
	while (work) {
		msg = &work->msg;
		fcount = msg->parameter1;
		rcount = msg->parameter3;

		framemgr_e_barrier_irqs(scp_framemgr, FMGR_IDX_4, flags);

		fimc_is_frame_process_head(scp_framemgr, &scp_frame);
		if (scp_frame && test_bit(REQ_FRAME, &scp_frame->req_flag)) {
			clear_bit(REQ_FRAME, &scp_frame->req_flag);

#ifdef DBG_STREAMING
			printk(KERN_INFO "P%d(%d,%d)\n", scp_frame->index,
				fcount, rcount);
#endif
#ifdef USE_FRAME_SYNC
			findex = scp_frame->stream->findex;
			isp_frame = &isp_framemgr->frame[findex];
			if (isp_frame->fcount != fcount)
				err("scp mismatched(%d, %d)",
					isp_frame->fcount, fcount);

			scp_frame->stream->fcount = fcount;
			scp_frame->stream->rcount = rcount;
			isp_frame->scp_out = FIMC_IS_FOUT_DONE;
#endif

			fimc_is_frame_trans_pro_to_com(scp_framemgr, scp_frame);
			buffer_done(video, scp_frame->index);
		} else
			err("done is occured without request(%p)", scp_frame);

		framemgr_x_barrier_irqr(scp_framemgr, FMGR_IDX_4, flags);

		set_free_work(&itf->work_list[INTR_SCP_FDONE], work);
		get_req_work(&itf->work_list[INTR_SCP_FDONE], &work);
	}
}

static void wq_func_isp(struct fimc_is_interface *itf,
	struct fimc_is_framemgr *framemgr,
	struct fimc_is_frame_shot *frame)
{
	u32 index;
	struct camera2_lens_uctl *isp_lens_uctl;
	struct camera2_lens_uctl *lens_uctl;
	struct camera2_sensor_uctl *isp_sensor_uctl;
	struct camera2_sensor_uctl *sensor_uctl;
	struct camera2_flash_uctl *isp_flash_uctl;
	struct camera2_flash_uctl *flash_uctl;

	index = frame->index;
	isp_lens_uctl = &itf->isp_peri_ctl.lensUd;
	isp_sensor_uctl = &itf->isp_peri_ctl.sensorUd;
	isp_flash_uctl = &itf->isp_peri_ctl.flashUd;

#ifdef DBG_STREAMING
	printk(KERN_CONT "M%d %d\n", index, frame->fcount);
#endif

	/* Cache Invalidation */
	vb2_ion_sync_for_device((void *)frame->cookie_shot, 0, frame->shot_size,
		DMA_FROM_DEVICE);

	if (frame->shot->uctl.uUpdateBitMap & CAM_SENSOR_CMD) {
		sensor_uctl = &frame->shot->uctl.sensorUd;
		isp_sensor_uctl->ctl.exposureTime =
			sensor_uctl->ctl.exposureTime;
		isp_sensor_uctl->ctl.frameDuration =
			sensor_uctl->ctl.frameDuration;
		isp_sensor_uctl->ctl.sensitivity =
			sensor_uctl->ctl.sensitivity;

		frame->shot->uctl.uUpdateBitMap &=
			~CAM_SENSOR_CMD;
	}

	if (frame->shot->uctl.uUpdateBitMap & CAM_LENS_CMD) {
		lens_uctl = &frame->shot->uctl.lensUd;
		isp_lens_uctl->ctl.focusDistance =
			lens_uctl->ctl.focusDistance;
		isp_lens_uctl->maxPos =
			lens_uctl->maxPos;
		isp_lens_uctl->slewRate =
			lens_uctl->slewRate;

		frame->shot->uctl.uUpdateBitMap &=
			~CAM_LENS_CMD;
	}

	if (frame->shot->uctl.uUpdateBitMap & CAM_FLASH_CMD) {
		flash_uctl = &frame->shot->uctl.flashUd;
		isp_flash_uctl->ctl.flashMode =
			flash_uctl->ctl.flashMode;
		isp_flash_uctl->ctl.firingPower =
			flash_uctl->ctl.firingPower;
		isp_flash_uctl->ctl.firingTime =
			flash_uctl->ctl.firingTime;

		frame->shot->uctl.uUpdateBitMap &=
			~CAM_FLASH_CMD;
	}

#ifdef AUTO_MODE
	fimc_is_frame_trans_pro_to_fre(framemgr, frame);
	buffer_done(itf->video_sensor, index);
#else
	fimc_is_frame_trans_pro_to_com(framemgr, frame);
	buffer_done(itf->video_isp, index);
#endif
}

static void wq_func_meta(struct work_struct *data)
{
	struct fimc_is_interface *itf;
	struct fimc_is_msg *msg;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_device_ischain *ischain;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_work_list *work_list;
	struct fimc_is_work *work;
	unsigned long flags;
	u32 fcount;
	bool req_done;

	req_done = false;
	itf = container_of(data, struct fimc_is_interface,
		work_queue[INTR_META_DONE]);
	work_list = &itf->work_list[INTR_META_DONE];
	framemgr = itf->framemgr;
	ischain = itf->video_isp->device;

	get_req_work(work_list, &work);
	while (work) {
		msg = &work->msg;
		fcount = msg->parameter1;

		framemgr_e_barrier_irqs(framemgr, FMGR_IDX_7, flags);

		fimc_is_frame_process_head(framemgr, &frame);
		if (frame) {
			/* comparing input fcount and output fcount
			just error printing although this error is occured */
			if (fcount != frame->fcount)
				err("frame mismatch(%d != %d)",
					fcount, frame->fcount);

			if (test_bit(REQ_FRAME, &frame->req_flag)) {
				clear_bit(REQ_FRAME, &frame->req_flag);
				if (!frame->req_flag) {
					wq_func_isp(itf, framemgr, frame);
					req_done = true;
				}
			}
		}

		framemgr_x_barrier_irqr(framemgr, FMGR_IDX_7, flags);

		if (req_done) {
			mutex_lock(&ischain->mutex_state);
			fimc_is_ischain_callback(ischain);
			mutex_unlock(&ischain->mutex_state);
		}

		set_free_work(work_list, work);
		get_req_work(work_list, &work);
	}
}

static void wq_func_shot(struct work_struct *data)
{
	struct fimc_is_interface *itf;
	struct fimc_is_msg *msg;
	struct fimc_is_framemgr *framemgr;
	struct fimc_is_device_ischain *ischain;
	struct fimc_is_frame_shot *frame;
	struct fimc_is_work_list *work_list;
	struct fimc_is_work *work;
	unsigned long flags;
	u32 fcount;
	u32 status;
	bool req_done;

	req_done = false;
	itf = container_of(data, struct fimc_is_interface,
		work_queue[INTR_SHOT_DONE]);
	work_list = &itf->work_list[INTR_SHOT_DONE];
	framemgr = itf->framemgr;
	ischain = itf->video_isp->device;

	get_req_work(work_list, &work);
	while (work) {
		msg = &work->msg;
		fcount = msg->parameter1;
		status = msg->parameter2;

		framemgr_e_barrier_irqs(framemgr, FMGR_IDX_7, flags);

		fimc_is_frame_process_head(framemgr, &frame);
		if (frame) {
#ifdef MEASURE_TIME
#ifdef INTERNAL_TIME
			do_gettimeofday(&frame->time_shotdone);
#else
			do_gettimeofday(&frame->tzone[TM_SHOT_D]);
#endif
#endif
			/* comparing input fcount and output fcount
			just error printing although this error is occured */
			if (fcount != frame->fcount)
				err("frame mismatch(%d != %d)",
					fcount, frame->fcount);

			/* need error handling */
			if (status != ISR_DONE)
				err("shot done is invalid(0x%08X)", status);

			if (test_bit(REQ_SHOT, &frame->req_flag)) {
				clear_bit(REQ_SHOT, &frame->req_flag);
				if (!frame->req_flag) {
					wq_func_isp(itf, framemgr, frame);
					req_done = true;
				}
			}
		} else {
			err("Shot done is occured without request2");
			fimc_is_frame_print_all(framemgr);
		}

		framemgr_x_barrier_irqr(framemgr, FMGR_IDX_7, flags);

		if (req_done) {
			mutex_lock(&ischain->mutex_state);
			fimc_is_ischain_callback(ischain);
			mutex_unlock(&ischain->mutex_state);
		}

		set_free_work(work_list, work);
		get_req_work(work_list, &work);
	}
}

static irqreturn_t interface_isr(int irq, void *data)
{
	struct fimc_is_interface *itf;
	struct work_struct *work_queue;
	struct fimc_is_work_list *work_list;
	struct fimc_is_work *work;
	u32 status;

	itf = (struct fimc_is_interface *)data;
	status = fimc_is_get_intr(itf);

	if (status & (1<<INTR_SHOT_DONE)) {
		work_queue = &itf->work_queue[INTR_SHOT_DONE];
		work_list = &itf->work_list[INTR_SHOT_DONE];

		get_free_work_irq(work_list, &work);
		if (work) {
			fimc_is_get_cmd(itf, &work->msg, INTR_SHOT_DONE);
			work->fcount = work->msg.parameter1;
			set_req_work_irq(work_list, work);

			if (!work_pending(work_queue))
				schedule_work(work_queue);
		} else
			err("free work item is empty5");

		status &= ~(1<<INTR_SHOT_DONE);
		fimc_is_clr_intr(itf, INTR_SHOT_DONE);
	}

	if (status & (1<<INTR_GENERAL)) {
		work_queue = &itf->work_queue[INTR_GENERAL];
		work_list = &itf->work_list[INTR_GENERAL];

		get_free_work_irq(&itf->work_list[INTR_GENERAL], &work);
		if (work) {
			fimc_is_get_cmd(itf, &work->msg, INTR_GENERAL);
			set_req_work_irq(work_list, work);

			if (!work_pending(work_queue))
				schedule_work(work_queue);
		} else
			err("free work item is empty1");

		status &= ~(1<<INTR_GENERAL);
		fimc_is_clr_intr(itf, INTR_GENERAL);
	}

	if (status & (1<<INTR_SCC_FDONE)) {
		work_queue = &itf->work_queue[INTR_SCC_FDONE];
		work_list = &itf->work_list[INTR_SCC_FDONE];

		get_free_work_irq(work_list, &work);
		if (work) {
			fimc_is_get_cmd(itf, &work->msg, INTR_SCC_FDONE);
			set_req_work_irq(work_list, work);

			if (!work_pending(work_queue))
				schedule_work(work_queue);
		} else
			err("free work item is empty2");

		status &= ~(1<<INTR_SCC_FDONE);
		fimc_is_clr_intr(itf, INTR_SCC_FDONE);
	}

	if (status & (1<<INTR_SCP_FDONE)) {
		work_queue = &itf->work_queue[INTR_SCP_FDONE];
		work_list = &itf->work_list[INTR_SCP_FDONE];

		get_free_work_irq(work_list, &work);
		if (work) {
			fimc_is_get_cmd(itf, &work->msg, INTR_SCP_FDONE);
			set_req_work_irq(work_list, work);

			if (!work_pending(work_queue))
				schedule_work(work_queue);
		} else
			err("free work item is empty3");

		status &= ~(1<<INTR_SCP_FDONE);
		fimc_is_clr_intr(itf, INTR_SCP_FDONE);
	}

	if (status & (1<<INTR_META_DONE)) {
		work_queue = &itf->work_queue[INTR_META_DONE];
		work_list = &itf->work_list[INTR_META_DONE];

		/* meta done is not needed now
		this can be used in the future */
#if 0
		get_free_work_irq(work_list, &work);
		if (work) {
			fimc_is_get_cmd(itf, &work->msg, INTR_META_DONE);
			work->fcount = work->msg.parameter1;
			set_req_work_irq(work_list, work);

			if (!work_pending(work_queue))
				schedule_work(work_queue);
		} else
			err("free work item is empty4");
#endif
		status &= ~(1<<INTR_META_DONE);
		fimc_is_clr_intr(itf, INTR_META_DONE);
	}

	if (status != 0)
		err("status is NOT all clear(0x%08X)", status);

	return IRQ_HANDLED;
}


int fimc_is_interface_probe(struct fimc_is_interface *this,
	struct fimc_is_framemgr *framemgr,
	u32 regs,
	u32 irq,
	void *core_data)
{
	int ret = 0;
	struct fimc_is_core *core = (struct fimc_is_core *)core_data;

	dbg_interface("%s\n", __func__);

	init_request_barrier(this);
	init_process_barrier(this);
	spin_lock_init(&this->slock_state);
	init_waitqueue_head(&this->init_wait_queue);
	init_waitqueue_head(&this->wait_queue);

	INIT_WORK(&this->work_queue[INTR_GENERAL], wq_func_general);
	INIT_WORK(&this->work_queue[INTR_SCC_FDONE], wq_func_scc);
	INIT_WORK(&this->work_queue[INTR_SCP_FDONE], wq_func_scp);
	INIT_WORK(&this->work_queue[INTR_META_DONE], wq_func_meta);
	INIT_WORK(&this->work_queue[INTR_SHOT_DONE], wq_func_shot);

	this->regs = (void *)regs;
	this->com_regs = (struct is_common_reg *)(regs + ISSR0);

	ret = request_irq(irq, interface_isr, 0, "mcuctl", this);
	if (ret)
		err("request_irq failed\n");

	notify_fcount			= &this->com_regs->fcount;
	this->framemgr			= framemgr;
	this->core			= (void *)core;
	this->video_isp			= &core->video_isp.common;
	this->video_sensor		= &core->video_sensor.common;
	this->video_scp			= &core->video_scp.common;
	this->video_scc			= &core->video_scc.common;
	clear_bit(IS_IF_STATE_OPEN, &this->state);
	clear_bit(IS_IF_STATE_START, &this->state);
	clear_bit(IS_IF_STATE_BUSY, &this->state);

	init_work_list(&this->nblk_cam_ctrl, TRACE_WORK_ID_CAMCTRL,
		MAX_NBLOCKING_COUNT);
	init_work_list(&this->work_list[INTR_GENERAL],
		TRACE_WORK_ID_GENERAL, MAX_WORK_COUNT);
	init_work_list(&this->work_list[INTR_SCC_FDONE],
		TRACE_WORK_ID_SCC, MAX_WORK_COUNT);
	init_work_list(&this->work_list[INTR_SCP_FDONE],
		TRACE_WORK_ID_SCP, MAX_WORK_COUNT);
	init_work_list(&this->work_list[INTR_META_DONE],
		TRACE_WORK_ID_META, MAX_WORK_COUNT);
	init_work_list(&this->work_list[INTR_SHOT_DONE],
		TRACE_WORK_ID_SHOT, MAX_WORK_COUNT);

	return ret;
}

int fimc_is_interface_open(struct fimc_is_interface *this)
{
	int ret = 0;

	if (testnset_state(this, IS_IF_STATE_OPEN)) {
		err("already open");
		ret = -EMFILE;
		goto exit;
	}

	dbg_interface("%s\n", __func__);

	/* common register init */
	this->com_regs->ihcmd_iflag = 0;
	this->com_regs->scc_iflag = 0;
	this->com_regs->scp_iflag = 0;
	this->com_regs->meta_iflag = 0;
	this->com_regs->shot_iflag = 0;

	this->streaming = IS_IF_STREAMING_INIT;
	this->processing = IS_IF_PROCESSING_INIT;
	this->pdown_ready = IS_IF_POWER_DOWN_READY;
	clr_state(this, IS_IF_STATE_START);
	set_state(this, IS_IF_STATE_BUSY);

exit:
	return ret;
}

int fimc_is_interface_close(struct fimc_is_interface *this)
{
	int ret = 0;
	int retry;

	if (testnclr_state(this, IS_IF_STATE_OPEN)) {
		err("already close");
		ret = -EMFILE;
		goto exit;
	}

	retry = 10;
	while (test_state(this, IS_IF_STATE_BUSY) && retry) {
		err("interface is busy");
		msleep(20);
		retry--;
	}

	if (!retry)
		err("waiting idle is fail");

	dbg_interface("%s\n", __func__);

exit:
	return ret;
}

int fimc_is_hw_print(struct fimc_is_interface *this)
{
	int debug_cnt;
	char *debug;
	char letter;
	int count = 0, i;
	struct fimc_is_device_ischain *ischain;

	if (!test_state(this, IS_IF_STATE_OPEN)) {
		err("interface is closed");
		return 0;
	}

	ischain = this->video_isp->device;

	vb2_ion_sync_for_device(ischain->minfo.fw_cookie,
		DEBUG_OFFSET, DEBUG_CNT, DMA_FROM_DEVICE);

	debug = (char *)(ischain->minfo.kvaddr + DEBUG_OFFSET);
	debug_cnt = *((int *)(ischain->minfo.kvaddr + DEBUGCTL_OFFSET))
			- DEBUG_OFFSET;

	if (ischain->debug_cnt > debug_cnt)
		count = (DEBUG_CNT - ischain->debug_cnt) + debug_cnt;
	else
		count = debug_cnt - ischain->debug_cnt;

	if (count) {
		printk(KERN_INFO "start(%d %d)\n", debug_cnt, count);
		for (i = ischain->debug_cnt; count > 0; count--) {
			letter = debug[i];
			if (letter)
				printk(KERN_CONT "%c", letter);
			i++;
			if (i > DEBUG_CNT)
				i = 0;
		}
		ischain->debug_cnt = debug_cnt;
		printk(KERN_INFO "end\n");
	}

	return count;
}

int fimc_is_hw_enum(struct fimc_is_interface *this)
{
	int ret = 0;
	struct fimc_is_msg msg;

	dbg_interface("enum(%d)\n", instances);

	ret = wait_initstate(this);
	if (ret) {
		err("enum time out");
		ret = -ETIME;
		goto exit;
	}

	msg.id = 0;
	msg.command = ISR_DONE;
	msg.instance = 0;
	msg.parameter1 = IHC_GET_SENSOR_NUMBER;
	/* this mean sensor numbers */
	msg.parameter2 = 1;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	waiting_is_ready(this);
	this->com_regs->hicmd = msg.command;
	this->com_regs->hic_sensorid = msg.instance;
	this->com_regs->hic_param1 = msg.parameter1;
	this->com_regs->hic_param2 = msg.parameter2;
	this->com_regs->hic_param3 = msg.parameter3;
	this->com_regs->hic_param4 = msg.parameter4;
	send_interrupt(this);

exit:
	return ret;
}

int fimc_is_hw_saddr(struct fimc_is_interface *this,
	u32 instance, u32 *setfile_addr)
{
	struct fimc_is_msg msg;

	dbg_interface("saddr(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_GET_SET_FILE_ADDR;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	fimc_is_set_cmd(this, &msg);
	*setfile_addr = this->reply.parameter2;

	return 0;
}

int fimc_is_hw_setfile(struct fimc_is_interface *this,
	u32 instance)
{
	struct fimc_is_msg msg;

	dbg_interface("setfile(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_LOAD_SET_FILE;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	fimc_is_set_cmd(this, &msg);

	return 0;
}

int fimc_is_hw_open(struct fimc_is_interface *this,
	u32 instance, u32 sensor, u32 channel, u32 ext,
	u32 *mwidth, u32 *mheight)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("open(%d)\n", sensor);

	msg.id = 0;
	msg.command = HIC_OPEN_SENSOR;
	msg.instance = instance;
	msg.parameter1 = sensor;
	msg.parameter2 = channel;
	msg.parameter3 = ext;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	*mwidth = this->reply.parameter2;
	*mheight = this->reply.parameter3;

	return ret;
}

int fimc_is_hw_stream_on(struct fimc_is_interface *this,
	u32 instance)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("stream_on(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_STREAM_ON;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_stream_off(struct fimc_is_interface *this,
	u32 instance)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("stream_off(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_STREAM_OFF;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_process_on(struct fimc_is_interface *this,
	u32 instance)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("process_on(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_PROCESS_START;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_process_off(struct fimc_is_interface *this,
	u32 instance)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("process_off(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_PROCESS_STOP;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_s_param(struct fimc_is_interface *this,
	u32 instance, u32 indexes, u32 lindex, u32 hindex)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("s_param(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_SET_PARAMETER;
	msg.instance = instance;
	msg.parameter1 = ISS_PREVIEW_STILL;
	msg.parameter2 = indexes;
	msg.parameter3 = lindex;
	msg.parameter4 = hindex;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_a_param(struct fimc_is_interface *this,
	u32 instance, u32 mode, u32 sub_mode)
{
	int ret = 0;
	struct fimc_is_msg msg;

	dbg_interface("a_param(%d)\n", instance);

	msg.id = 0;
	msg.command = mode;
	msg.instance = instance;
	msg.parameter1 = sub_mode;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_f_param(struct fimc_is_interface *this,
	u32 instance)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("f_param(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_PREVIEW_STILL;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_g_capability(struct fimc_is_interface *this,
	u32 instance, u32 address)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("g_capability(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_GET_STATIC_METADATA;
	msg.instance = instance;
	msg.parameter1 = address;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_cfg_mem(struct fimc_is_interface *this,
	u32 instance, u32 address, u32 size)
{
	int ret;
	struct fimc_is_msg msg;

	dbg_interface("cfg_mem(%d, 0x%08X)\n", instance, address);

	msg.id = 0;
	msg.command = HIC_SET_A5_MEM_ACCESS;
	msg.instance = instance;
	msg.parameter1 = address;
	msg.parameter2 = size;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_power_down(struct fimc_is_interface *this,
	u32 instance)
{
	int ret = 0;
	struct fimc_is_msg msg;

	dbg_interface("pwr_down(%d)\n", instance);

	msg.id = 0;
	msg.command = HIC_POWER_DOWN;
	msg.instance = instance;
	msg.parameter1 = 0;
	msg.parameter2 = 0;
	msg.parameter3 = 0;
	msg.parameter4 = 0;

	ret = fimc_is_set_cmd(this, &msg);

	return ret;
}

int fimc_is_hw_shot_nblk(struct fimc_is_interface *this,
	u32 instance, u32 bayer, u32 shot, u32 fcount, u32 rcount)
{
	int ret = 0;
	struct fimc_is_msg msg;

	/*dbg_interface("shot_nblk(%d, %d)\n", instance, fcount);*/

	msg.id = 0;
	msg.command = HIC_SHOT;
	msg.instance = instance;
	msg.parameter1 = bayer;
	msg.parameter2 = shot;
	msg.parameter3 = fcount;
	msg.parameter4 = rcount;

	ret = fimc_is_set_cmd_shot(this, &msg);

	return ret;
}

int fimc_is_hw_s_camctrl_nblk(struct fimc_is_interface *this,
	u32 instance, u32 address, u32 fcount)
{
	int ret = 0;
	struct fimc_is_work *work;
	struct fimc_is_msg *msg;

	dbg_interface("cam_ctrl_nblk(%d)\n", instance);

	get_free_work(&this->nblk_cam_ctrl, &work);

	if (work) {
		work->fcount = fcount;
		msg = &work->msg;
		msg->id = 0;
		msg->command = HIC_SET_CAM_CONTROL;
		msg->instance = instance;
		msg->parameter1 = address;
		msg->parameter2 = fcount;
		msg->parameter3 = 0;
		msg->parameter4 = 0;

		ret = fimc_is_set_cmd_nblk(this, work);
	} else {
		err("g_free_nblk return NULL");
		print_fre_work_list(&this->nblk_cam_ctrl);
		print_req_work_list(&this->nblk_cam_ctrl);
		ret = 1;
	}

	return ret;
}
