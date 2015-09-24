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

#include "fimc-is-time.h"
#include "fimc-is-core.h"
#include "fimc-is-param.h"
#include "fimc-is-cmd.h"
#include "fimc-is-regs.h"
#include "fimc-is-err.h"

#ifdef USE_FRAME_SYNC
#define SPARE_PLANE 1
#define SPARE_SIZE (4*1024)
#else
#define SPARE_PLANE 0
#define SPARE_SIZE 0
#endif

struct fimc_is_fmt fimc_is_formats[] = {
	 {
		.name		= "YUV 4:2:2 packed, YCbYCr",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.num_planes	= 1 + SPARE_PLANE,
		.mbus_code	= V4L2_MBUS_FMT_YUYV8_2X8,
	}, {
		.name		= "YUV 4:2:2 packed, CbYCrY",
		.pixelformat	= V4L2_PIX_FMT_UYVY,
		.num_planes	= 1 + SPARE_PLANE,
		.mbus_code	= V4L2_MBUS_FMT_UYVY8_2X8,
	}, {
		.name		= "YUV 4:2:2 planar, Y/Cb/Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV422P,
		.num_planes	= 1 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 planar, YCbCr",
		.pixelformat	= V4L2_PIX_FMT_YUV420,
		.num_planes	= 1 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 planar, YCbCr",
		.pixelformat	= V4L2_PIX_FMT_YVU420,
		.num_planes	= 1 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 planar, Y/CbCr",
		.pixelformat	= V4L2_PIX_FMT_NV12,
		.num_planes	= 1 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 planar, Y/CrCb",
		.pixelformat	= V4L2_PIX_FMT_NV21,
		.num_planes	= 1 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 non-contiguous 2-planar, Y/CbCr",
		.pixelformat	= V4L2_PIX_FMT_NV12M,
		.num_planes	= 2 + SPARE_PLANE,
	}, {
		.name		= "YVU 4:2:0 non-contiguous 2-planar, Y/CrCb",
		.pixelformat	= V4L2_PIX_FMT_NV21M,
		.num_planes	= 2 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV420M,
		.num_planes	= 3 + SPARE_PLANE,
	}, {
		.name		= "YUV 4:2:0 non-contiguous 3-planar, Y/Cr/Cb",
		.pixelformat	= V4L2_PIX_FMT_YVU420M,
		.num_planes	= 3 + SPARE_PLANE,
	}, {
		.name		= "BAYER 10 bit",
		.pixelformat	= V4L2_PIX_FMT_SBGGR10,
		.num_planes	= 2,
	}, {
		.name		= "BAYER 12 bit",
		.pixelformat	= V4L2_PIX_FMT_SBGGR12,
		.num_planes	= 2,
	}, {
		.name		= "BAYER 16 bit",
		.pixelformat	= V4L2_PIX_FMT_SBGGR16,
		.num_planes	= 2,
	},
};

struct fimc_is_fmt *fimc_is_find_format(u32 *pixelformat,
	u32 *mbus_code, int index)
{
	struct fimc_is_fmt *fmt, *def_fmt = NULL;
	unsigned int i;

	if (index >= ARRAY_SIZE(fimc_is_formats))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(fimc_is_formats); ++i) {
		fmt = &fimc_is_formats[i];
		if (pixelformat && fmt->pixelformat == *pixelformat)
			return fmt;
		if (mbus_code && fmt->mbus_code == *mbus_code)
			return fmt;
		if (index == i)
			def_fmt = fmt;
	}
	return def_fmt;

}

void fimc_is_set_plane_size(struct fimc_is_frame *frame, unsigned int sizes[])
{
	u32 plane;
	u32 width[FIMC_IS_MAX_PLANES];

	for (plane = 0; plane < FIMC_IS_MAX_PLANES; ++plane)
		width[plane] = frame->width + frame->width_stride[plane];

	switch (frame->format.pixelformat) {
	case V4L2_PIX_FMT_YUYV:
		dbg("V4L2_PIX_FMT_YUYV(w:%d)(h:%d)\n",
				frame->width, frame->height);
		sizes[0] = width[0]*frame->height*2;
#ifdef USE_FRAME_SYNC
		sizes[1] = SPARE_SIZE;
#endif
		break;
	case V4L2_PIX_FMT_NV12M:
		dbg("V4L2_PIX_FMT_NV12M(w:%d)(h:%d)\n",
				frame->width, frame->height);
		sizes[0] = width[0]*frame->height;
		sizes[1] = width[1]*frame->height/2;
#ifdef USE_FRAME_SYNC
		sizes[2] = SPARE_SIZE;
#endif
		break;
	case V4L2_PIX_FMT_YUV420M:
	case V4L2_PIX_FMT_YVU420M:
		dbg("V4L2_PIX_FMT_YVU420M(w:%d)(h:%d)\n",
				frame->width, frame->height);
		sizes[0] = width[0]*frame->height;
		sizes[1] = width[1]*frame->height/4;
		sizes[2] = width[2]*frame->height/4;
#ifdef USE_FRAME_SYNC
		sizes[3] = SPARE_SIZE;
#endif
		break;
	case V4L2_PIX_FMT_SBGGR10:
		dbg("V4L2_PIX_FMT_SBGGR10(w:%d)(h:%d)\n",
				frame->width, frame->height);
		sizes[0] = frame->width*frame->height*2;
		sizes[1] = 8*1024;
		break;
	case V4L2_PIX_FMT_SBGGR16:
		dbg("V4L2_PIX_FMT_SBGGR16(w:%d)(h:%d)\n",
				frame->width, frame->height);
		sizes[0] = frame->width*frame->height*2;
		sizes[1] = 8*1024;
		break;
	case V4L2_PIX_FMT_SBGGR12:
		dbg("V4L2_PIX_FMT_SBGGR12(w:%d)(h:%d)\n",
				frame->width, frame->height);
		sizes[0] = frame->width*frame->height*2;
		sizes[1] = 8*1024;
		break;
	default:
		err("unknown pixelformat\n");
		break;
	}
}

int fimc_is_video_probe(struct fimc_is_video_common *video,
	void *core_data,
	void *video_data,
	char *video_name,
	u32 video_number,
	u32 vbq_type,
	const struct v4l2_file_operations *fops,
	const struct v4l2_ioctl_ops *ioctl_ops,
	const struct vb2_ops *vb2_ops)
{
	int ret = 0;
	struct fimc_is_core *core = core_data;
	struct vb2_queue *vbq;

	snprintf(video->vd.name, sizeof(video->vd.name),
		"%s", video_name);

	mutex_init(&video->lock);

	video->core			= core;
	video->vb2			= core->mem.vb2;
	video->vd.fops			= fops;
	video->vd.ioctl_ops		= ioctl_ops;
	video->vd.v4l2_dev		= &core->mdev->v4l2_dev;
	video->vd.minor			= -1;
	video->vd.release		= video_device_release;
	video->vd.lock			= &video->lock;
	video_set_drvdata(&video->vd, core);

	vbq				= &video->vbq;
	memset(vbq, 0, sizeof(struct vb2_queue));
	vbq->type			= vbq_type;
	vbq->io_modes			= VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	vbq->drv_priv			= video_data;
	vbq->ops			= vb2_ops;
	vbq->mem_ops			= core->mem.vb2->ops;

	vb2_queue_init(vbq);

	ret = video_register_device(&video->vd,
				VFL_TYPE_GRABBER,
				video_number
				+ EXYNOS_VIDEONODE_FIMC_IS);
	if (ret) {
		err("Failed to register ScalerP video device\n");
		goto p_err;
	}

	video->pads.flags = MEDIA_PAD_FL_SINK;
	ret = media_entity_init(&video->vd.entity,
			1, &video->pads, 0);
	if (ret) {
		err("Failed to media_entity_init ScalerP video device\n");
		goto p_err;
	}

p_err:
	return ret;
}

int fimc_is_video_open(struct fimc_is_video_common *video,
	void *device, u32 buffers_ready)
{
	int ret = 0;

	video->buffers = 0;
	video->buffers_ready = buffers_ready;
	video->buf_ref_cnt = 0;
	video->device = device;

	clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &video->state);
	clear_bit(FIMC_IS_VIDEO_BUFFER_READY, &video->state);
	clear_bit(FIMC_IS_VIDEO_STREAM_ON, &video->state);

	memset(&video->frame, 0, sizeof(struct fimc_is_frame));

	return ret;
}

int fimc_is_video_close(struct fimc_is_video_common *video,
	struct fimc_is_framemgr *framemgr)
{
	int ret = 0;

	video->buffers = 0;
	video->buf_ref_cnt = 0;
	video->device = 0;

	clear_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &video->state);
	clear_bit(FIMC_IS_VIDEO_BUFFER_READY, &video->state);
	clear_bit(FIMC_IS_VIDEO_STREAM_ON, &video->state);
	fimc_is_frame_close(framemgr);

	vb2_queue_release(&video->vbq);

	return ret;
}

int fimc_is_video_reqbufs(struct fimc_is_video_common *video,
	struct fimc_is_framemgr *framemgr,
	struct v4l2_requestbuffers *request)
{
	int ret = 0;

	if (test_bit(FIMC_IS_VIDEO_STREAM_ON, &video->state)) {
		err("video is stream on, not applied");
		ret = -EINVAL;
		goto exit;
	}

	ret = vb2_reqbufs(&video->vbq, request);
	if (ret) {
		err("(type, mem) : vbq(%d,%d) != req(%d,%d)",
			video->vbq.type, video->vbq.memory,
			request->type, request->memory);
		goto exit;
	}

	video->buffers = request->count;

	if (video->buffers == 0) {
		video->buf_ref_cnt = 0;
		fimc_is_frame_close(framemgr);
	} else {
		if (video->buffers < video->buffers_ready) {
			err("buffer count is not invalid(%d < %d)",
				video->buffers, video->buffers_ready);
			ret = -EINVAL;
			goto exit;
		}
		fimc_is_frame_open(framemgr, video->buffers);
	}

exit:
	return ret;
}

int fimc_is_video_set_format_mplane(struct fimc_is_video_common *video,
	struct v4l2_format *format)
{
	int ret = 0;
	u32 plane;
	struct v4l2_pix_format_mplane *pix;
	struct fimc_is_fmt *frame;

	pix = &format->fmt.pix_mp;
	frame = fimc_is_find_format(&pix->pixelformat, NULL, 0);
	if (!frame) {
		err("pixel format is not found\n");
		ret = -EINVAL;
		goto p_err;
	}

	video->frame.format.pixelformat = frame->pixelformat;
	video->frame.format.mbus_code	= frame->mbus_code;
	video->frame.format.num_planes	= frame->num_planes;
	video->frame.width		= pix->width;
	video->frame.height		= pix->height;

	for (plane = 0; plane < frame->num_planes; ++plane) {
		if (pix->plane_fmt[plane].bytesperline)
			video->frame.width_stride[plane] =
				pix->plane_fmt[plane].bytesperline - pix->width;
		else
			video->frame.width_stride[plane] = 0;
	}

p_err:
	return ret;
}

int fimc_is_video_qbuf(struct fimc_is_video_common *video,
	struct v4l2_buffer *buf)
{
	int ret = 0;

	buf->flags &= ~V4L2_BUF_FLAG_USE_SYNC;

	ret = vb2_qbuf(&video->vbq, buf);
	if (ret)
		err("vb2_qbuf is failed(%d)", ret);

	return ret;
}

int fimc_is_video_dqbuf(struct fimc_is_video_common *video,
	struct v4l2_buffer *buf, bool blocking)
{
	int ret = 0;

	ret = vb2_dqbuf(&video->vbq, buf, blocking);
	if (ret)
		err("vb2_dqbuf is failed(%d)", ret);

	video->buf_mask &= ~(1<<buf->index);

	return ret;
}

int fimc_is_video_streamon(struct fimc_is_video_common *video,
	enum v4l2_buf_type type)
{
	int ret = 0;

	ret = vb2_streamon(&video->vbq, type);
	if (ret)
		err("vb2_streamon is failed(%d)", ret);

	return ret;
}

int fimc_is_video_streamoff(struct fimc_is_video_common *video,
	enum v4l2_buf_type type)
{
	int ret = 0;

	ret = vb2_streamoff(&video->vbq, type);
	if (ret)
		err("vb2_streamoff is failed(%d)", ret);

	return ret;
}

int fimc_is_video_queue_setup(struct fimc_is_video_common *video,
	unsigned int *num_planes,
	unsigned int sizes[],
	void *allocators[])
{
	u32 ret = 0, i;
	struct fimc_is_core *core = video->core;

	*num_planes = (unsigned int)(video->frame.format.num_planes);
	fimc_is_set_plane_size(&video->frame, sizes);

	for (i = 0; i < *num_planes; i++) {
		allocators[i] =  core->mem.alloc_ctx;
		video->frame.size[i] = sizes[i];
	}

	return ret;
}

int fimc_is_video_buffer_queue(struct fimc_is_video_common *video,
	struct vb2_buffer *vb, struct fimc_is_framemgr *framemgr)
{
	u32 ret = 0, i;
	u32 index;
	u32 ext_size;
	u32 spare;
	struct fimc_is_core *core = video->core;
	struct fimc_is_frame_shot *frame;

	index = vb->v4l2_buf.index;

	if (!framemgr) {
		err("framemgr is null");
		ret = -EINVAL;
		goto exit;
	}

	/* plane address is updated for checking everytime */
	if (video->frame.format.pixelformat == V4L2_PIX_FMT_YVU420M) {
		video->buf_dva[index][0] = core->mem.vb2->plane_addr(vb, 0);
		video->buf_kva[index][0] = core->mem.vb2->plane_kvaddr(vb, 0);

		video->buf_dva[index][1] = core->mem.vb2->plane_addr(vb, 2);
		video->buf_kva[index][1] = core->mem.vb2->plane_kvaddr(vb, 2);

		video->buf_dva[index][2] = core->mem.vb2->plane_addr(vb, 1);
		video->buf_kva[index][2] = core->mem.vb2->plane_kvaddr(vb, 1);

#ifdef USE_FRAME_SYNC
		video->buf_dva[index][3] = core->mem.vb2->plane_addr(vb, 3);
		video->buf_kva[index][3] = core->mem.vb2->plane_kvaddr(vb, 3);
#endif
	} else {
		for (i = 0; i < vb->num_planes; i++) {
			video->buf_dva[index][i]
				= core->mem.vb2->plane_addr(vb, i);
			video->buf_kva[index][i]
				= core->mem.vb2->plane_kvaddr(vb, i);
		}
	}

	frame = &framemgr->frame[index];

	/* uninitialized frame need to get info */
	if (frame->init == FRAME_UNI_MEM)
		goto set_info;

	/* plane count check */
	if (frame->planes != vb->num_planes) {
		err("plane count is changed(%08X != %08X)",
			frame->planes, vb->num_planes);
		ret = -EINVAL;
		goto exit;
	}

	/* plane address check */
	for (i = 0; i < frame->planes; i++) {
		if (frame->kvaddr_buffer[i] !=
			video->buf_kva[index][i]) {
			err("buffer %d plane %d is changed(%08X != %08X)",
				index, i,
				frame->kvaddr_buffer[i],
				video->buf_kva[index][i]);
			ret = -EINVAL;
			goto exit;
		}
	}

	goto exit;

set_info:
	if (test_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &video->state)) {
		err("already prepared but new index(%d) is came", index);
		ret = -EINVAL;
		goto exit;
	}

	frame->vb = vb;
	frame->planes = vb->num_planes;
	spare = frame->planes - 1;

	for (i = 0; i < frame->planes; i++) {
		frame->dvaddr_buffer[i] = video->buf_dva[index][i];
		frame->kvaddr_buffer[i] = video->buf_kva[index][i];
#ifdef PRINT_BUFADDR
		printk(KERN_INFO "0x%04X buf[%d][%d] : 0x%08X\n", framemgr->id,
			index, i, frame->dvaddr_buffer[i]);
#endif
	}

	if (framemgr->id & (FRAMEMGR_ID_SENSOR | FRAMEMGR_ID_ISP)) {
		ext_size = sizeof(struct camera2_shot_ext) -
			sizeof(struct camera2_shot);

		frame->dvaddr_shot = video->buf_dva[index][spare] + ext_size;
		frame->kvaddr_shot = video->buf_kva[index][spare] + ext_size;
		frame->cookie_shot = (u32)vb2_plane_cookie(vb, spare);
		frame->shot = (struct camera2_shot *)frame->kvaddr_shot;
		frame->shot_ext = (struct camera2_shot_ext *)
			(video->buf_kva[index][spare]);
		frame->shot_size = video->frame.size[spare];
#ifdef MEASURE_TIME
		frame->tzone = (struct timeval *)frame->shot_ext->timeZone;
#endif
	} else {
#ifdef USE_FRAME_SYNC
		frame->stream = (struct camera2_stream *)
			video->buf_kva[index][spare];
		frame->stream->address = video->buf_kva[index][spare];
		frame->stream_size = video->frame.size[spare];
#else
		frame->stream = NULL;
		frame->stream_size = 0;
#endif
	}

	frame->init = FRAME_INI_MEM;

	video->buf_ref_cnt++;

	if (video->buffers_ready == video->buf_ref_cnt)
		set_bit(FIMC_IS_VIDEO_BUFFER_READY, &video->state);

	if (video->buffers == video->buf_ref_cnt)
		set_bit(FIMC_IS_VIDEO_BUFFER_PREPARED, &video->state);

exit:
	return ret;
}

int buffer_done(struct fimc_is_video_common *video, u32 index)
{
	int ret = 0;

	if (!video) {
		err("video is NULL");
		ret = -EINVAL;
		goto exit;
	}

	if (index == FIMC_IS_INVALID_BUF_INDEX) {
		err("buffer done had invalid index(%d)", index);
		ret = -EINVAL;
		goto exit;
	}

	if (!test_bit(FIMC_IS_VIDEO_STREAM_ON, &video->state)) {
		warn("video state is not stream on");
		ret = -EINVAL;
		goto exit;
	}

	vb2_buffer_done(video->vbq.bufs[index], VB2_BUF_STATE_DONE);

exit:
	return ret;
}
