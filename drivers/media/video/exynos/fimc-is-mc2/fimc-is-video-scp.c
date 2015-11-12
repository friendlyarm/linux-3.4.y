/*
 * Samsung Exynos5 SoC series FIMC-IS driver
 *
 * exynos5 fimc-is video functions
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/videonode.h>
#include <media/exynos_mc.h>
#include <linux/cma.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#include <linux/firmware.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/videodev2_exynos_media.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/v4l2-mediabus.h>

#include "fimc-is-core.h"
#include "fimc-is-param.h"
#include "fimc-is-cmd.h"
#include "fimc-is-regs.h"
#include "fimc-is-err.h"
#include "fimc-is-video.h"

/*************************************************************************/
/* video file opertation						 */
/************************************************************************/
int fimc_is_scp_video_probe(void *data)
{
	int ret = 0;
	struct fimc_is_core *core = (struct fimc_is_core *)data;
	struct fimc_is_video_scp *video = &core->video_scp;

	ret = fimc_is_video_probe(&video->common,
		data,
		video,
		FIMC_IS_VIDEO_SCALERP_NAME,
		FIMC_IS_VIDEO_NUM_SCALERP,
		V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
		&fimc_is_scalerp_video_fops,
		&fimc_is_scalerp_video_ioctl_ops,
		&fimc_is_scalerp_qops);

	return ret;
}

static int fimc_is_scalerp_video_open(struct file *file)
{
	struct fimc_is_core *core = video_drvdata(file);
	struct fimc_is_video_scp *video = &core->video_scp;
	struct fimc_is_device_ischain *ischain = &core->ischain;
	struct fimc_is_ischain_dev *scp = &ischain->scp;

	dbg_scp("%s\n", __func__);

	file->private_data = video;
	fimc_is_video_open(&video->common, ischain, VIDEO_SCP_READY_BUFFERS);
	fimc_is_ischain_dev_open(scp, ENTRY_SCALERP, &video->common, NULL);

	return 0;
}

static int fimc_is_scalerp_video_close(struct file *file)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;

	printk(KERN_INFO "%s\n", __func__);

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state))
		clear_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);

	file->private_data = 0;
	fimc_is_video_close(common, &scp->framemgr);

	return ret;
}

static unsigned int fimc_is_scalerp_video_poll(struct file *file,
				      struct poll_table_struct *wait)
{
	struct fimc_is_video_scp *video = file->private_data;

	dbg("%s\n", __func__);
	return vb2_poll(&video->common.vbq, file, wait);

}

static int fimc_is_scalerp_video_mmap(struct file *file,
					struct vm_area_struct *vma)
{
	struct fimc_is_video_scp *video = file->private_data;

	dbg("%s\n", __func__);
	return vb2_mmap(&video->common.vbq, vma);

}

/*************************************************************************/
/* video ioctl operation						*/
/************************************************************************/

static int fimc_is_scalerp_video_querycap(struct file *file, void *fh,
						struct v4l2_capability *cap)
{
	struct fimc_is_core *isp = video_drvdata(file);

	strncpy(cap->driver, isp->pdev->name, sizeof(cap->driver) - 1);

	dbg("%s(devname : %s)\n", __func__, cap->driver);
	strncpy(cap->card, isp->pdev->name, sizeof(cap->card) - 1);
	cap->bus_info[0] = 0;
	cap->version = KERNEL_VERSION(1, 0, 0);
	cap->capabilities = V4L2_CAP_STREAMING
					| V4L2_CAP_VIDEO_CAPTURE
					| V4L2_CAP_VIDEO_CAPTURE_MPLANE;

	return 0;
}

static int fimc_is_scalerp_video_enum_fmt_mplane(struct file *file, void *priv,
				    struct v4l2_fmtdesc *f)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_get_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_set_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;
	struct fimc_is_device_ischain *ischain = video->common.device;

	dbg_scp("%s\n", __func__);

	format->fmt.pix_mp.plane_fmt[0].bytesperline =
		ALIGN(format->fmt.pix_mp.width, 32);
	format->fmt.pix_mp.plane_fmt[1].bytesperline =
		ALIGN(format->fmt.pix_mp.width, 32);
	format->fmt.pix_mp.plane_fmt[2].bytesperline =
		ALIGN(format->fmt.pix_mp.width, 32);
	format->fmt.pix_mp.plane_fmt[3].bytesperline = 0;

	ret = fimc_is_video_set_format_mplane(&video->common, format);

	dbg_scp("req w : %d req h : %d\n",
		video->common.frame.width,
		video->common.frame.height);

	fimc_is_ischain_scp_s_format(ischain,
		video->common.frame.width,
		video->common.frame.height);

	return ret;
}

static int fimc_is_scalerp_video_try_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_cropcap(struct file *file, void *fh,
						struct v4l2_cropcap *cropcap)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_get_crop(struct file *file, void *fh,
						struct v4l2_crop *crop)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_set_crop(struct file *file, void *fh,
						struct v4l2_crop *crop)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_reqbufs(struct file *file, void *priv,
					struct v4l2_requestbuffers *buf)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *isp = &ischain->isp;
	struct fimc_is_ischain_dev *scp = &ischain->scp;

	dbg_scp("%s(buffers : %d)\n", __func__, buf->count);

	if (test_bit(FIMC_IS_ISDEV_DSTART, &isp->state)) {
		err("isp still running, not applied");
		ret = -EINVAL;
		goto exit;
	}

	ret = fimc_is_video_reqbufs(common, &scp->framemgr, buf);
	if (ret)
		err("fimc_is_video_reqbufs is fail(error %d)", ret);

exit:
	return ret;
}

static int fimc_is_scalerp_video_querybuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret;
	struct fimc_is_video_scp *video = file->private_data;

	dbg("%s\n", __func__);
	ret = vb2_querybuf(&video->common.vbq, buf);

	return ret;
}

static int fimc_is_scalerp_video_qbuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;

#ifdef DBG_STREAMING
	/*dbg_scp("%s(index : %d)\n", __func__, buf->index);*/
#endif

	ret = fimc_is_video_qbuf(&video->common, buf);

	return ret;
}

static int fimc_is_scalerp_video_dqbuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;

	ret = fimc_is_video_dqbuf(&video->common, buf,
		file->f_flags & O_NONBLOCK);

#ifdef DBG_STREAMING
	/*dbg_scp("%s(index : %d)\n", __func__, buf->index);*/
#endif

	return ret;
}

static int fimc_is_scalerp_video_streamon(struct file *file, void *priv,
						enum v4l2_buf_type type)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;

	dbg_scp("%s\n", __func__);

	ret = fimc_is_video_streamon(&video->common, type);

	return ret;
}

static int fimc_is_scalerp_video_streamoff(struct file *file, void *priv,
	enum v4l2_buf_type type)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;

	dbg_scp("%s\n", __func__);

	ret = fimc_is_video_streamoff(&video->common, type);

	return ret;
}

static int fimc_is_scalerp_video_enum_input(struct file *file, void *priv,
	struct v4l2_input *input)
{
	struct fimc_is_core *isp = video_drvdata(file);
	struct exynos5_fimc_is_sensor_info *sensor_info
			= isp->pdata->sensor_info[input->index];

	dbg("index(%d) sensor(%s)\n",
		input->index, sensor_info->sensor_name);
	dbg("pos(%d) sensor_id(%d)\n",
		sensor_info->sensor_position, sensor_info->sensor_id);
	dbg("csi_id(%d) flite_id(%d)\n",
		sensor_info->csi_id, sensor_info->flite_id);
	dbg("i2c_ch(%d)\n", sensor_info->i2c_channel);

	if (input->index >= FIMC_IS_MAX_CAMIF_CLIENTS)
		return -EINVAL;

	input->type = V4L2_INPUT_TYPE_CAMERA;

	strncpy(input->name, sensor_info->sensor_name,
					FIMC_IS_MAX_SENSOR_NAME_LEN);
	return 0;
}

static int fimc_is_scalerp_video_g_input(struct file *file, void *priv,
						unsigned int *input)
{
	dbg("%s\n", __func__);
	return 0;
}

static int fimc_is_scalerp_video_s_input(struct file *file, void *priv,
					unsigned int input)
{
	return 0;
}

static int fimc_is_scalerp_video_g_ctrl(struct file *file, void *priv,
					struct v4l2_control *ctrl)
{
	int ret = 0;
	struct fimc_is_video_scp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;
	struct fimc_is_framemgr *framemgr = &scp->framemgr;

	dbg_scp("%s\n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_IS_G_COMPLETES:
		ctrl->value = framemgr->frame_complete_cnt;
		break;
	default:
		err("unsupported ioctl(%d)\n", ctrl->id);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int fimc_is_scalerp_video_g_ext_ctrl(struct file *file, void *priv,
					struct v4l2_ext_controls *ctrls)
{
	return 0;
}

static int fimc_is_scalerp_video_s_ctrl(struct file *file, void *priv,
	struct v4l2_control *ctrl)
{
	int ret = 0;
	unsigned long flags;
	struct fimc_is_video_scp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;
	struct fimc_is_framemgr *framemgr = &scp->framemgr;
	struct fimc_is_frame_shot *frame;

	dbg_scp("%s\n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_IS_FORCE_DONE:
		if (framemgr->frame_process_cnt) {
			err("force done can be performed(process count %d)",
				framemgr->frame_process_cnt);
			ret = -EINVAL;
		} else if (!framemgr->frame_request_cnt) {
			err("force done can be performed(request count %d)",
				framemgr->frame_request_cnt);
			ret = -EINVAL;
		} else {
			framemgr_e_barrier_irqs(framemgr, 0, flags);

			fimc_is_frame_request_head(framemgr, &frame);
			if (frame) {
				fimc_is_frame_trans_req_to_com(framemgr, frame);
				buffer_done(common, frame->index);
			} else {
				err("frame is NULL");
				ret = -EINVAL;
			}

			framemgr_x_barrier_irqr(framemgr, 0, flags);
		}
		break;
	default:
		err("unsupported ioctl(%d)\n", ctrl->id);
		ret = -EINVAL;
		break;
	}

	return ret;
}

const struct v4l2_file_operations fimc_is_scalerp_video_fops = {
	.owner		= THIS_MODULE,
	.open		= fimc_is_scalerp_video_open,
	.release	= fimc_is_scalerp_video_close,
	.poll		= fimc_is_scalerp_video_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= fimc_is_scalerp_video_mmap,
};

const struct v4l2_ioctl_ops fimc_is_scalerp_video_ioctl_ops = {
	.vidioc_querycap		= fimc_is_scalerp_video_querycap,
	.vidioc_enum_fmt_vid_cap_mplane
			= fimc_is_scalerp_video_enum_fmt_mplane,
	.vidioc_g_fmt_vid_cap_mplane
			= fimc_is_scalerp_video_get_format_mplane,
	.vidioc_s_fmt_vid_cap_mplane
			= fimc_is_scalerp_video_set_format_mplane,
	.vidioc_try_fmt_vid_cap_mplane
			= fimc_is_scalerp_video_try_format_mplane,
	.vidioc_cropcap			= fimc_is_scalerp_video_cropcap,
	.vidioc_g_crop			= fimc_is_scalerp_video_get_crop,
	.vidioc_s_crop			= fimc_is_scalerp_video_set_crop,
	.vidioc_reqbufs			= fimc_is_scalerp_video_reqbufs,
	.vidioc_querybuf		= fimc_is_scalerp_video_querybuf,
	.vidioc_qbuf			= fimc_is_scalerp_video_qbuf,
	.vidioc_dqbuf			= fimc_is_scalerp_video_dqbuf,
	.vidioc_streamon		= fimc_is_scalerp_video_streamon,
	.vidioc_streamoff		= fimc_is_scalerp_video_streamoff,
	.vidioc_enum_input		= fimc_is_scalerp_video_enum_input,
	.vidioc_g_input			= fimc_is_scalerp_video_g_input,
	.vidioc_s_input			= fimc_is_scalerp_video_s_input,
	.vidioc_g_ctrl			= fimc_is_scalerp_video_g_ctrl,
	.vidioc_s_ctrl			= fimc_is_scalerp_video_s_ctrl,
	.vidioc_g_ext_ctrls		= fimc_is_scalerp_video_g_ext_ctrl,
};

static int fimc_is_scalerp_queue_setup(struct vb2_queue *vq,
	const struct v4l2_format *fmt,
	unsigned int *num_buffers,
	unsigned int *num_planes,
	unsigned int sizes[],
	void *allocators[])
{
	int ret = 0;
	struct fimc_is_video_scp *video = vq->drv_priv;

	dbg_scp("%s\n", __func__);

	ret = fimc_is_video_queue_setup(&video->common,
		num_planes,
		sizes,
		allocators);

	dbg_scp("(num_planes : %d)(size : %d)\n",
		(int)*num_planes, (int)sizes[0]);

	return ret;
}

static int fimc_is_scalerp_buffer_prepare(struct vb2_buffer *vb)
{
	return 0;
}

static inline void fimc_is_scalerp_wait_prepare(struct vb2_queue *q)
{
}

static inline void fimc_is_scalerp_wait_finish(struct vb2_queue *q)
{
}

static int fimc_is_scalerp_start_streaming(struct vb2_queue *q,
	unsigned int count)
{
	int ret = 0;
	struct fimc_is_video_scp *video = q->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;

	dbg_scp("%s\n", __func__);

	if (!test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state) &&
		test_bit(FIMC_IS_VIDEO_BUFFER_READY, &common->state)) {
		set_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);
		fimc_is_ischain_dev_start(scp);
	} else {
		err("already stream on or buffer is not ready(%ld)",
			common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_READY, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &common->state);
		fimc_is_ischain_dev_stop(scp);
		ret = -EINVAL;
	}

	return ret;
}

static int fimc_is_scalerp_stop_streaming(struct vb2_queue *q)
{
	int ret = 0;
	unsigned long flags;
	struct fimc_is_video_scp *video = q->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;
	struct fimc_is_framemgr *framemgr = &scp->framemgr;

	dbg_scp("%s\n", __func__);

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state)) {
		framemgr_e_barrier_irqs(framemgr, 0, flags);
		ret = framemgr->frame_process_cnt;
		framemgr_x_barrier_irqr(framemgr, 0, flags);
		if (ret) {
			err("being processed, can't stop");
			ret = -EINVAL;
			goto exit;
		}

		clear_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_READY, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &common->state);
		fimc_is_ischain_dev_stop(scp);
	} else {
		err("already stream off");
		ret = -EINVAL;
	}

exit:
	return ret;
}

static void fimc_is_scalerp_buffer_queue(struct vb2_buffer *vb)
{
	struct fimc_is_video_scp *video = vb->vb2_queue->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;

#ifdef DBG_STREAMING
	dbg_scp("%s\n", __func__);
#endif

	fimc_is_video_buffer_queue(common, vb, &scp->framemgr);
	fimc_is_ischain_dev_buffer_queue(scp, vb->v4l2_buf.index);
}

static int fimc_is_scalerp_buffer_finish(struct vb2_buffer *vb)
{
	int ret = 0;
	struct fimc_is_video_scc *video = vb->vb2_queue->drv_priv;
	struct fimc_is_device_ischain *ischain = video->common.device;
	struct fimc_is_ischain_dev *scp = &ischain->scp;

#ifdef DBG_STREAMING
	dbg_scp("%s(%d)\n", __func__, vb->v4l2_buf.index);
#endif

	ret = fimc_is_ischain_dev_buffer_finish(scp, vb->v4l2_buf.index);

	return ret;
}

const struct vb2_ops fimc_is_scalerp_qops = {
	.queue_setup		= fimc_is_scalerp_queue_setup,
	.buf_prepare		= fimc_is_scalerp_buffer_prepare,
	.buf_queue		= fimc_is_scalerp_buffer_queue,
	.buf_finish		= fimc_is_scalerp_buffer_finish,
	.wait_prepare		= fimc_is_scalerp_wait_prepare,
	.wait_finish		= fimc_is_scalerp_wait_finish,
	.start_streaming	= fimc_is_scalerp_start_streaming,
	.stop_streaming		= fimc_is_scalerp_stop_streaming,
};
