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
int fimc_is_sensor_video_probe(void *data)
{
	int ret = 0;
	struct fimc_is_core *core = (struct fimc_is_core *)data;
	struct fimc_is_video_sensor *video = &core->video_sensor;

	dbg_sensor("%s\n", __func__);

	ret = fimc_is_video_probe(&video->common,
		data,
		video,
		FIMC_IS_VIDEO_BAYER_NAME,
		FIMC_IS_VIDEO_NUM_BAYER,
		V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
		&fimc_is_bayer_video_fops,
		&fimc_is_bayer_video_ioctl_ops,
		&fimc_is_bayer_qops);

	return ret;
}

static int fimc_is_bayer_video_open(struct file *file)
{
	struct fimc_is_core *core = video_drvdata(file);
	struct fimc_is_video_sensor *video = &core->video_sensor;
	struct fimc_is_device_sensor *sensor = &core->sensor;

	dbg_sensor("%s\n", __func__);

	file->private_data = video;
	fimc_is_video_open(&video->common, sensor,
		VIDEO_SENSOR_READY_BUFFERS);
	fimc_is_sensor_open(sensor);

	return 0;
}

static int fimc_is_bayer_video_close(struct file *file)
{
	struct fimc_is_video_sensor *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_sensor *sensor = common->device;

	printk(KERN_INFO "%s\n", __func__);

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state))
		clear_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);

	file->private_data = 0;
	fimc_is_sensor_close(sensor);
	fimc_is_video_close(&video->common, sensor->framemgr);

	return 0;
}

static unsigned int fimc_is_bayer_video_poll(struct file *file,
				      struct poll_table_struct *wait)
{
	struct fimc_is_core *isp = video_drvdata(file);

	dbg_sensor("%s\n", __func__);
	return vb2_poll(&isp->video_sensor.common.vbq, file, wait);
}

static int fimc_is_bayer_video_mmap(struct file *file,
					struct vm_area_struct *vma)
{
	struct fimc_is_core *is = video_drvdata(file);

	return vb2_mmap(&is->video_sensor.common.vbq, vma);
}

/*************************************************************************/
/* video ioctl operation						*/
/************************************************************************/

static int fimc_is_bayer_video_querycap(struct file *file, void *fh,
					struct v4l2_capability *cap)
{
	struct fimc_is_core *isp = video_drvdata(file);

	strncpy(cap->driver, isp->pdev->name, sizeof(cap->driver) - 1);

	dbg_sensor("%s(devname : %s)\n", __func__, cap->driver);
	strncpy(cap->card, isp->pdev->name, sizeof(cap->card) - 1);
	cap->bus_info[0] = 0;
	cap->version = KERNEL_VERSION(1, 0, 0);
	cap->capabilities = V4L2_CAP_STREAMING
				| V4L2_CAP_VIDEO_CAPTURE
				| V4L2_CAP_VIDEO_CAPTURE_MPLANE;

	return 0;
}

static int fimc_is_bayer_video_enum_fmt_mplane(struct file *file, void *priv,
				    struct v4l2_fmtdesc *f)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_get_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_set_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;

	dbg_scp("%s\n", __func__);

	ret = fimc_is_video_set_format_mplane(&video->common, format);

	dbg_sensor("req w : %d req h : %d\n",
		video->common.frame.width,
		video->common.frame.height);

	return ret;
}

static int fimc_is_bayer_video_try_format_mplane(struct file *file, void *fh,
						struct v4l2_format *format)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_cropcap(struct file *file, void *fh,
						struct v4l2_cropcap *cropcap)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_get_crop(struct file *file, void *fh,
						struct v4l2_crop *crop)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_set_crop(struct file *file, void *fh,
						struct v4l2_crop *crop)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_reqbufs(struct file *file, void *priv,
	struct v4l2_requestbuffers *buf)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_sensor *sensor = common->device;

	dbg_sensor("%s(buffers : %d)\n", __func__, buf->count);

	ret = fimc_is_video_reqbufs(common, sensor->framemgr, buf);
	if (ret)
		err("fimc_is_video_reqbufs is fail(error %d)", ret);

	return ret;
}

static int fimc_is_bayer_video_querybuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret;
	struct fimc_is_video_sensor *video = file->private_data;

	dbg_sensor("%s\n", __func__);

	ret = vb2_querybuf(&video->common.vbq, buf);

	return ret;
}

static int fimc_is_bayer_video_qbuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;

#ifdef DBG_STREAMING
	/*dbg_sensor("%s\n", __func__);*/
#endif

	ret = fimc_is_video_qbuf(&video->common, buf);

	return ret;
}

static int fimc_is_bayer_video_dqbuf(struct file *file, void *priv,
						struct v4l2_buffer *buf)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;
	bool blocking = file->f_flags & O_NONBLOCK;

	ret = fimc_is_video_dqbuf(&video->common, buf, blocking);

#ifdef DBG_STREAMING
	/*dbg_sensor("%s(index : %d)\n", __func__, buf->index);*/
#endif

	return ret;
}

static int fimc_is_bayer_video_streamon(struct file *file, void *priv,
	enum v4l2_buf_type type)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;

	dbg_sensor("%s\n", __func__);

	ret = fimc_is_video_streamon(&video->common, type);

	return ret;
}

static int fimc_is_bayer_video_streamoff(struct file *file, void *priv,
	enum v4l2_buf_type type)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;

	dbg_sensor("%s\n", __func__);

	ret = fimc_is_video_streamoff(&video->common, type);

	return ret;
}

static int fimc_is_bayer_video_enum_input(struct file *file, void *priv,
						struct v4l2_input *input)
{
	struct fimc_is_core *isp = video_drvdata(file);
	struct exynos5_fimc_is_sensor_info *sensor_info;

	sensor_info = isp->pdata->sensor_info[input->index];

	dbg_sensor("index(%d) sensor(%s)\n",
		input->index, sensor_info->sensor_name);
	dbg_sensor("pos(%d) sensor_id(%d)\n",
		sensor_info->sensor_position, sensor_info->sensor_id);
	dbg_sensor("csi_id(%d) flite_id(%d)\n",
		sensor_info->csi_id, sensor_info->flite_id);
	dbg_sensor("i2c_ch(%d)\n", sensor_info->i2c_channel);

	if (input->index >= FIMC_IS_MAX_CAMIF_CLIENTS)
		return -EINVAL;

	input->type = V4L2_INPUT_TYPE_CAMERA;

	strncpy(input->name, sensor_info->sensor_name,
					FIMC_IS_MAX_SENSOR_NAME_LEN);
	return 0;
}

static int fimc_is_bayer_video_g_input(struct file *file, void *priv,
						unsigned int *input)
{
	dbg_sensor("%s\n", __func__);
	return 0;
}

static int fimc_is_bayer_video_s_input(struct file *file, void *priv,
	unsigned int input)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;
	struct fimc_is_device_sensor *sensor = video->common.device;

	dbg_sensor("%s(input : %d)\n", __func__, input);

	fimc_is_sensor_s_active_sensor(sensor, input);

	return ret;
}

static int fimc_is_bayer_video_s_ctrl(struct file *file, void *priv,
	struct v4l2_control *ctrl)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = file->private_data;
	struct fimc_is_device_sensor *sensor = video->common.device;

	switch (ctrl->id) {
	case V4L2_CID_IS_S_STREAM:
		if (ctrl->value == IS_ENABLE_STREAM)
			ret = fimc_is_sensor_front_start(sensor);
		else
			ret = fimc_is_sensor_front_stop(sensor);
		break;
	}

	return ret;
}

const struct v4l2_file_operations fimc_is_bayer_video_fops = {
	.owner		= THIS_MODULE,
	.open		= fimc_is_bayer_video_open,
	.release	= fimc_is_bayer_video_close,
	.poll		= fimc_is_bayer_video_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= fimc_is_bayer_video_mmap,
};

const struct v4l2_ioctl_ops fimc_is_bayer_video_ioctl_ops = {
	.vidioc_querycap		= fimc_is_bayer_video_querycap,
	.vidioc_enum_fmt_vid_cap_mplane	= fimc_is_bayer_video_enum_fmt_mplane,
	.vidioc_g_fmt_vid_cap_mplane	= fimc_is_bayer_video_get_format_mplane,
	.vidioc_s_fmt_vid_cap_mplane	= fimc_is_bayer_video_set_format_mplane,
	.vidioc_try_fmt_vid_cap_mplane	= fimc_is_bayer_video_try_format_mplane,
	.vidioc_cropcap			= fimc_is_bayer_video_cropcap,
	.vidioc_g_crop			= fimc_is_bayer_video_get_crop,
	.vidioc_s_crop			= fimc_is_bayer_video_set_crop,
	.vidioc_reqbufs			= fimc_is_bayer_video_reqbufs,
	.vidioc_querybuf		= fimc_is_bayer_video_querybuf,
	.vidioc_qbuf			= fimc_is_bayer_video_qbuf,
	.vidioc_dqbuf			= fimc_is_bayer_video_dqbuf,
	.vidioc_streamon		= fimc_is_bayer_video_streamon,
	.vidioc_streamoff		= fimc_is_bayer_video_streamoff,
	.vidioc_enum_input		= fimc_is_bayer_video_enum_input,
	.vidioc_g_input			= fimc_is_bayer_video_g_input,
	.vidioc_s_input			= fimc_is_bayer_video_s_input,
	.vidioc_s_ctrl			= fimc_is_bayer_video_s_ctrl
};

static int fimc_is_bayer_queue_setup(struct vb2_queue *vq,
	const struct v4l2_format *fmt,
	unsigned int *num_buffers, unsigned int *num_planes,
	unsigned int sizes[],
	void *allocators[])
{
	int ret = 0;
	struct fimc_is_video_sensor *video = vq->drv_priv;

	dbg_sensor("%s\n", __func__);

	ret = fimc_is_video_queue_setup(&video->common,
		num_planes,
		sizes,
		allocators);

	dbg_sensor("(num_planes : %d)(size : %d)\n",
		(int)*num_planes, (int)sizes[0]);

	return ret;
}

static int fimc_is_bayer_buffer_prepare(struct vb2_buffer *vb)
{
	/*dbg_sensor("-%s\n", __func__);*/

	return 0;
}

static inline void fimc_is_bayer_wait_prepare(struct vb2_queue *q)
{
}

static inline void fimc_is_bayer_wait_finish(struct vb2_queue *q)
{
}

static int fimc_is_bayer_start_streaming(struct vb2_queue *q,
	unsigned int count)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = q->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_sensor *sensor = common->device;

	dbg_sensor("%s\n", __func__);

	if (!test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state) &&
		test_bit(FIMC_IS_VIDEO_BUFFER_READY, &common->state)) {
		set_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);
		fimc_is_sensor_back_start(sensor, common);
	} else {
		err("already stream on or buffer is not ready(%ld)",
			common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_READY, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &common->state);
		fimc_is_sensor_back_stop(sensor);
		ret = -EINVAL;
	}

	return 0;
}

static int fimc_is_bayer_stop_streaming(struct vb2_queue *q)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = q->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_sensor *sensor = video->common.device;

	dbg_sensor("%s\n", __func__);

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state)) {
		clear_bit(FIMC_IS_VIDEO_STREAM_ON, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_READY, &common->state);
		clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &common->state);
		ret = fimc_is_sensor_back_stop(sensor);
	} else {
		err("already stream off");
		ret = -EINVAL;
	}

	return ret;
}

static void fimc_is_bayer_buffer_queue(struct vb2_buffer *vb)
{
	struct fimc_is_video_sensor *video = vb->vb2_queue->drv_priv;
	struct fimc_is_video_common *common = &video->common;
	struct fimc_is_device_sensor *sensor = common->device;

#ifdef DBG_STREAMING
	dbg_sensor("%s(%d)\n", __func__, vb->v4l2_buf.index);
#endif

	fimc_is_video_buffer_queue(common, vb, sensor->framemgr);
	fimc_is_sensor_buffer_queue(sensor, vb->v4l2_buf.index);
}

static int fimc_is_bayer_buffer_finish(struct vb2_buffer *vb)
{
	int ret = 0;
	struct fimc_is_video_sensor *video = vb->vb2_queue->drv_priv;
	struct fimc_is_device_sensor *sensor = video->common.device;

#ifdef DBG_STREAMING
	dbg_sensor("%s(%d)\n", __func__, vb->v4l2_buf.index);
#endif

	ret = fimc_is_sensor_buffer_finish(
		sensor,
		vb->v4l2_buf.index);

	return 0;
}

const struct vb2_ops fimc_is_bayer_qops = {
	.queue_setup		= fimc_is_bayer_queue_setup,
	.buf_prepare		= fimc_is_bayer_buffer_prepare,
	.buf_queue		= fimc_is_bayer_buffer_queue,
	.buf_finish		= fimc_is_bayer_buffer_finish,
	.wait_prepare		= fimc_is_bayer_wait_prepare,
	.wait_finish		= fimc_is_bayer_wait_finish,
	.start_streaming	= fimc_is_bayer_start_streaming,
	.stop_streaming		= fimc_is_bayer_stop_streaming,
};
