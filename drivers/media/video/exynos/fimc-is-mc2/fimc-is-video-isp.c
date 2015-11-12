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
#include "fimc-is-metadata.h"

/*************************************************************************/
/* video file opertation						 */
/************************************************************************/
int fimc_is_isp_video_probe(void *data)
{
	int ret = 0;
	struct fimc_is_core *core = (struct fimc_is_core *)data;
	struct fimc_is_video_isp *video = &core->video_isp;

	dbg_isp("%s\n", __func__);

	ret = fimc_is_video_probe(&video->common,
		data,
		video,
		FIMC_IS_VIDEO_ISP_NAME,
		FIMC_IS_VIDEO_NUM_ISP,
		V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
		&fimc_is_isp_video_fops,
		&fimc_is_isp_video_ioctl_ops,
		&fimc_is_isp_qops);

	return ret;
}

static int fimc_is_isp_video_open(struct file *file)
{
	struct fimc_is_core *core = video_drvdata(file);
	struct fimc_is_video_isp *video = &core->video_isp;
	struct fimc_is_device_ischain *ischain = &core->ischain;

	dbg_isp("%s\n", __func__);

	file->private_data = video;
	fimc_is_video_open(&video->common, ischain, VIDEO_ISP_READY_BUFFERS);

	return 0;
}

static int fimc_is_isp_video_close(struct file *file)
{
	struct fimc_is_video_isp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;

	printk(KERN_INFO "%s\n", __func__);

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state))
		clear_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);

	file->private_data = 0;
	fimc_is_ischain_close(ischain);
	fimc_is_video_close(&video->common, ischain->framemgr);

	return 0;
}

static unsigned int fimc_is_isp_video_poll(struct file *file,
				      struct poll_table_struct *wait)
{
	struct fimc_is_video_isp *video = file->private_data;

	return vb2_poll(&video->common.vbq, file, wait);
}

static int fimc_is_isp_video_mmap(struct file *file,
					struct vm_area_struct *vma)
{
	struct fimc_is_video_isp *video = file->private_data;

	return vb2_mmap(&video->common.vbq, vma);
}

/*************************************************************************/
/* video ioctl operation						*/
/************************************************************************/

static int fimc_is_isp_video_querycap(struct file *file, void *fh,
					struct v4l2_capability *cap)
{
	struct fimc_is_core *isp = video_drvdata(file);

	strncpy(cap->driver, isp->pdev->name, sizeof(cap->driver) - 1);

	dbg_isp("%s(devname : %s)\n", __func__, cap->driver);
	strncpy(cap->card, isp->pdev->name, sizeof(cap->card) - 1);
	cap->bus_info[0] = 0;
	cap->version = KERNEL_VERSION(1, 0, 0);
	cap->capabilities = V4L2_CAP_STREAMING
				| V4L2_CAP_VIDEO_CAPTURE
				| V4L2_CAP_VIDEO_CAPTURE_MPLANE;

	return 0;
}

static int fimc_is_isp_video_enum_fmt_mplane(struct file *file, void *priv,
				    struct v4l2_fmtdesc *f)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_get_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_set_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;
	struct fimc_is_device_ischain *ischain = video->common.device;

	dbg_isp("%s\n", __func__);

	ret = fimc_is_video_set_format_mplane(&video->common, format);

	dbg_isp("req w : %d req h : %d\n",
		video->common.frame.width,
		video->common.frame.height);

	fimc_is_ischain_isp_s_format(ischain,
		video->common.frame.width,
		video->common.frame.height);

	return ret;
}

static int fimc_is_isp_video_try_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_cropcap(struct file *file, void *fh,
						struct v4l2_cropcap *cropcap)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_get_crop(struct file *file, void *fh,
						struct v4l2_crop *crop)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_set_crop(struct file *file, void *fh,
						struct v4l2_crop *crop)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_reqbufs(struct file *file, void *priv,
						struct v4l2_requestbuffers *buf)
{
	int ret;
	struct fimc_is_video_isp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;

	dbg_isp("%s(buffers : %d)\n", __func__, buf->count);

	ret = fimc_is_video_reqbufs(common, ischain->framemgr, buf);
	if (ret)
		err("fimc_is_video_reqbufs is fail(error %d)", ret);

	return ret;
}

static int fimc_is_isp_video_querybuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret;
	struct fimc_is_video_isp *video = file->private_data;

	dbg_isp("%s\n", __func__);
	ret = vb2_querybuf(&video->common.vbq, buf);

	return ret;
}

static int fimc_is_isp_video_qbuf(struct file *file, void *priv,
	struct v4l2_buffer *buf)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;

#ifdef DBG_STREAMING
	/*dbg_isp("%s\n", __func__);*/
#endif

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state))
		ret = fimc_is_video_qbuf(common, buf);
	else {
		err("stream off state, can NOT qbuf");
		ret = -EINVAL;
	}

	return ret;
}

static int fimc_is_isp_video_dqbuf(struct file *file, void *priv,
	struct v4l2_buffer *buf)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;
	bool blocking = file->f_flags & O_NONBLOCK;

	ret = fimc_is_video_dqbuf(&video->common, buf, blocking);

#ifdef DBG_STREAMING
	dbg_isp("%s(index : %d)\n", __func__, buf->index);
#endif

	return ret;
}

static int fimc_is_isp_video_streamon(struct file *file, void *priv,
	enum v4l2_buf_type type)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;

	dbg_isp("%s\n", __func__);

	ret = fimc_is_video_streamon(&video->common, type);

	return ret;
}

static int fimc_is_isp_video_streamoff(struct file *file, void *priv,
	enum v4l2_buf_type type)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;

	dbg_isp("%s\n", __func__);

	ret = fimc_is_video_streamoff(&video->common, type);

	return ret;
}

static int fimc_is_isp_video_enum_input(struct file *file, void *priv,
						struct v4l2_input *input)
{
	struct fimc_is_core *isp = video_drvdata(file);
	struct exynos5_fimc_is_sensor_info *sensor_info;

	sensor_info = isp->pdata->sensor_info[input->index];

	dbg_isp("index(%d) sensor(%s)\n",
		input->index, sensor_info->sensor_name);
	dbg_isp("pos(%d) sensor_id(%d)\n",
		sensor_info->sensor_position, sensor_info->sensor_id);
	dbg_isp("csi_id(%d) flite_id(%d)\n",
		sensor_info->csi_id, sensor_info->flite_id);
	dbg_isp("i2c_ch(%d)\n", sensor_info->i2c_channel);

	if (input->index >= FIMC_IS_MAX_CAMIF_CLIENTS)
		return -EINVAL;

	input->type = V4L2_INPUT_TYPE_CAMERA;

	strncpy(input->name, sensor_info->sensor_name,
					FIMC_IS_MAX_SENSOR_NAME_LEN);
	return 0;
}

static int fimc_is_isp_video_g_input(struct file *file, void *priv,
						unsigned int *input)
{
	dbg_isp("%s\n", __func__);
	return 0;
}

static int fimc_is_isp_video_s_input(struct file *file, void *priv,
						unsigned int input)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;
	struct fimc_is_device_ischain *ischain = video->common.device;
	struct fimc_is_core *core = video->common.core;
	struct fimc_is_device_sensor *sensor = &core->sensor;

	dbg_isp("%s(input : %d)\n", __func__, input);
	core->sensor.id_position = input;

	ret = fimc_is_ischain_open(ischain, &video->common);
	if (ret) {
		err("fimc_is_ischain_open is fail\n");
		goto exit;
	}

	ret = fimc_is_ischain_init(ischain,
		input,
		sensor->enum_sensor[input].i2c_ch,
		&sensor->enum_sensor[input].ext,
		sensor->enum_sensor[input].setfile_name);
	if (ret) {
		err("fimc_is_ischain_init is fail\n");
		goto exit;
	}

exit:
	return ret;
}

static int fimc_is_isp_video_s_ctrl(struct file *file, void *priv,
					struct v4l2_control *ctrl)
{
	int ret = 0;
	struct fimc_is_video_isp *video = file->private_data;
	struct fimc_is_device_ischain *ischain = video->common.device;

	dbg_isp("%s\n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_IS_S_STREAM:
		if (ctrl->value == IS_ENABLE_STREAM)
			ret = fimc_is_itf_process_on(ischain);
		else
			ret = fimc_is_itf_process_off(ischain);
		break;
	case V4L2_CID_IS_G_CAPABILITY:
		ret = fimc_is_ischain_g_capability(ischain, ctrl->value);
		dbg_isp("V4L2_CID_IS_G_CAPABILITY : %X\n", ctrl->value);
		break;
	default:
		err("unsupported ioctl(%d)\n", ctrl->id);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int fimc_is_isp_video_g_ctrl(struct file *file, void *priv,
	struct v4l2_control *ctrl)
{
	int ret = 0;

	dbg_isp("%s\n", __func__);

	return ret;
}

static int fimc_is_isp_video_g_ext_ctrl(struct file *file, void *priv,
	struct v4l2_ext_controls *ctrls)
{
	int ret = 0;

	return ret;
}

const struct v4l2_file_operations fimc_is_isp_video_fops = {
	.owner		= THIS_MODULE,
	.open		= fimc_is_isp_video_open,
	.release	= fimc_is_isp_video_close,
	.poll		= fimc_is_isp_video_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= fimc_is_isp_video_mmap,
};

const struct v4l2_ioctl_ops fimc_is_isp_video_ioctl_ops = {
	.vidioc_querycap		= fimc_is_isp_video_querycap,
	.vidioc_enum_fmt_vid_out_mplane	= fimc_is_isp_video_enum_fmt_mplane,
	.vidioc_g_fmt_vid_out_mplane	= fimc_is_isp_video_get_format_mplane,
	.vidioc_s_fmt_vid_out_mplane	= fimc_is_isp_video_set_format_mplane,
	.vidioc_try_fmt_vid_out_mplane	= fimc_is_isp_video_try_format_mplane,
	.vidioc_cropcap			= fimc_is_isp_video_cropcap,
	.vidioc_g_crop			= fimc_is_isp_video_get_crop,
	.vidioc_s_crop			= fimc_is_isp_video_set_crop,
	.vidioc_reqbufs			= fimc_is_isp_video_reqbufs,
	.vidioc_querybuf		= fimc_is_isp_video_querybuf,
	.vidioc_qbuf			= fimc_is_isp_video_qbuf,
	.vidioc_dqbuf			= fimc_is_isp_video_dqbuf,
	.vidioc_streamon		= fimc_is_isp_video_streamon,
	.vidioc_streamoff		= fimc_is_isp_video_streamoff,
	.vidioc_enum_input		= fimc_is_isp_video_enum_input,
	.vidioc_g_input			= fimc_is_isp_video_g_input,
	.vidioc_s_input			= fimc_is_isp_video_s_input,
	.vidioc_s_ctrl			= fimc_is_isp_video_s_ctrl,
	.vidioc_g_ctrl			= fimc_is_isp_video_g_ctrl,
	.vidioc_g_ext_ctrls		= fimc_is_isp_video_g_ext_ctrl,
};

static int fimc_is_isp_queue_setup(struct vb2_queue *vq,
	const struct v4l2_format *fmt,
	unsigned int *num_buffers, unsigned int *num_planes,
	unsigned int sizes[],
	void *allocators[])
{
	int ret = 0;
	struct fimc_is_video_isp *video = vq->drv_priv;

	dbg_sensor("%s\n", __func__);

	ret = fimc_is_video_queue_setup(&video->common,
		num_planes,
		sizes,
		allocators);

	dbg_sensor("(num_planes : %d)(size : %d)\n",
		(int)*num_planes, (int)sizes[0]);

	return ret;
}

static int fimc_is_isp_buffer_prepare(struct vb2_buffer *vb)
{
	/*dbg_isp("%s\n", __func__);*/
	return 0;
}

static inline void fimc_is_isp_wait_prepare(struct vb2_queue *q)
{
}

static inline void fimc_is_isp_wait_finish(struct vb2_queue *q)
{
}

static int fimc_is_isp_start_streaming(struct vb2_queue *q,
						unsigned int count)
{
	int ret = 0;
	struct fimc_is_video_isp *video = q->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;

	dbg_isp("%s\n", __func__);

	if (!test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state)) {
		set_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);
		fimc_is_ischain_isp_start(ischain, common);
	} else {
		err("already stream on or buffer is not ready(%ld)",
			common->state);
		ret = -EINVAL;
	}

	return ret;
}

static int fimc_is_isp_stop_streaming(struct vb2_queue *q)
{
	int ret = 0;
	struct fimc_is_video_isp *video = q->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state)) {
		clear_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &common->state);
		fimc_is_ischain_isp_stop(ischain);
	} else {
		err("already stream off");
		ret = -EINVAL;
	}

	return ret;
}

static void fimc_is_isp_buffer_queue(struct vb2_buffer *vb)
{
	struct fimc_is_video_isp *video = vb->vb2_queue->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_ischain *ischain = common->device;

#ifdef DBG_STREAMING
	dbg_isp("%s(%d)\n", __func__, index);
#endif

	fimc_is_video_buffer_queue(common, vb, ischain->framemgr);
	fimc_is_ischain_isp_buffer_queue(ischain, vb->v4l2_buf.index);
}

static int fimc_is_isp_buffer_finish(struct vb2_buffer *vb)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = vb->vb2_queue->drv_priv;
	struct fimc_is_device_ischain *ischain = video->common.device;

#ifdef DBG_STREAMING
	dbg_isp("%s(%d)\n", __func__, vb->v4l2_buf.index);
#endif

	ret = fimc_is_ischain_isp_buffer_finish(ischain, vb->v4l2_buf.index);

	return ret;
}

const struct vb2_ops fimc_is_isp_qops = {
	.queue_setup		= fimc_is_isp_queue_setup,
	.buf_prepare		= fimc_is_isp_buffer_prepare,
	.buf_queue		= fimc_is_isp_buffer_queue,
	.buf_finish		= fimc_is_isp_buffer_finish,
	.wait_prepare		= fimc_is_isp_wait_prepare,
	.wait_finish		= fimc_is_isp_wait_finish,
	.start_streaming	= fimc_is_isp_start_streaming,
	.stop_streaming		= fimc_is_isp_stop_streaming,
};
