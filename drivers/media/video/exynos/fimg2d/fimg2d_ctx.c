/* linux/drivers/media/video/samsung/fimg2d4x/fimg2d_ctx.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *	http://www.samsung.com/
 *
 * Samsung Graphics 2D driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <plat/fimg2d.h>
#include <plat/iovmm.h>
#include "fimg2d.h"
#include "fimg2d_ctx.h"
#include "fimg2d_helper.h"

static inline int is_yuvfmt(enum color_format fmt)
{
	switch (fmt) {
	case CF_YCBCR_420:
	case CF_YCBCR_422:
	case CF_YCBCR_444:
		return 1;
	default:
		return 0;
	}
}

static int fimg2d_check_params(struct fimg2d_control *info,
		struct fimg2d_bltcmd *cmd)
{
	int w, h, i;
	struct fimg2d_param *p = &cmd->param;
	struct fimg2d_image *img;
	struct fimg2d_scale *scl;
	struct fimg2d_clip *clp;
	struct fimg2d_rect *r;

	/* dst is mandatory */
	if (!cmd->image[IDST].addr.type)
		return -1;

	/* DST op makes no effect */
	if (cmd->op < 0 || cmd->op == BLIT_OP_DST || cmd->op >= BLIT_OP_END)
		return -1;

	for (i = 0; i < MAX_IMAGES; i++) {
		img = &cmd->image[i];
		if (!img->addr.type)
			continue;

		w = img->width;
		h = img->height;
		r = &img->rect;

		/* 8000: max width & height */
		if (w > 8000 || h > 8000) {
			dev_err(info->dev, "%s too large (w = %d, h = %d)\n",
					imagename(i), w, h);
			return -1;
		}

		if (r->x1 < 0 || r->y1 < 0 ||
			r->x1 >= w || r->y1 >= h ||
			r->x1 >= r->x2 || r->y1 >= r->y2) {
			dev_err(info->dev,
					"%s has invalid clipping rectangle (r = [%d, %d, %d, %d], w = %d, h = %d)\n",
					imagename(i), r->x1, r->y1, r->x2,
					r->y2, w, h);
			return -1;
		}

		if (is_yuvfmt(img->fmt) && img->stride != w) {
			dev_err(info->dev,
					"YUV %s stride and width not equal (stride = %d, w = %d)\n",
					imagename(i), img->stride, w);
			return -1;
		}

		if (img->fmt == CF_YCBCR_422 ||
				img->fmt == CF_YCBCR_420) {
			const char *fmtname = img->fmt == CF_YCBCR_422 ?
					"YCbCr422" : "YCbCr420";

			if ((r->x1 & 1) ||
					(r->x2 & 1)) {
				dev_err(info->dev,
						"%s %s clipping rectangle X coordinates not even (x1 = %d, x2 = %d)\n",
						fmtname, imagename(i),
						r->x1, r->x2);
				return -1;

			}

			if (img->stride & 1) {
				dev_err(info->dev,
						"%s %s stride not even (stride = %d)\n",
						fmtname, imagename(i),
						img->stride);
				return -1;
			}
		}

		if (img->fmt == CF_YCBCR_420) {
			if ((r->y1 & 1) ||
					(r->y2 & 1)) {
				dev_err(info->dev,
						"YCbCr420 %s clipping rectangle Y coordinates not even (y1 = %d, y2 = %d)\n",
						imagename(i),
						r->y1, r->y2);
				return -1;

			}
		}
	}

	clp = &p->clipping;
	if (clp->enable) {
		img = &cmd->image[IDST];

		w = img->width;
		h = img->height;
		r = &img->rect;

		if (clp->x1 < 0 || clp->y1 < 0 ||
			clp->x1 >= w || clp->y1 >= h ||
			clp->x1 >= clp->x2 || clp->y1 >= clp->y2 ||
			clp->x1 >= r->x2 || clp->x2 <= r->x1 ||
			clp->y1 >= r->y2 || clp->y2 <= r->y1)
			return -1;
	}

	scl = &p->scaling;
	if (scl->mode) {
		if (!scl->src_w || !scl->src_h || !scl->dst_w || !scl->dst_h)
			return -1;
	}

	return 0;
}

static void fimg2d_fixup_params(struct fimg2d_bltcmd *cmd)
{
	struct fimg2d_param *p = &cmd->param;
	struct fimg2d_image *img;
	struct fimg2d_scale *scl;
	struct fimg2d_clip *clp;
	struct fimg2d_rect *r;
	int i;

	clp = &p->clipping;
	scl = &p->scaling;

	/* fix dst/clip rect */
	for (i = 0; i < MAX_IMAGES; i++) {
		img = &cmd->image[i];
		if (!img->addr.type)
			continue;

		r = &img->rect;

		if (i == IMAGE_DST && clp->enable) {
			if (clp->x2 > img->width)
				clp->x2 = img->width;
			if (clp->y2 > img->height)
				clp->y2 = img->height;
		} else {
			if (r->x2 > img->width)
				r->x2 = img->width;
			if (r->y2 > img->height)
				r->y2 = img->height;
		}
	}

	/* avoid devided-by-zero */
	if (scl->mode &&
		(scl->src_w == scl->dst_w && scl->src_h == scl->dst_h))
		scl->mode = NO_SCALING;
}

/**
 * @plane: 0 for 1st plane, 1 for 2nd plane
 */
static int yuv_stride(int width, enum color_format cf, enum pixel_order order,
		int plane)
{
	int bpp;

	switch (cf) {
	case CF_YCBCR_420:
		bpp = (!plane) ? 8 : 4;
		break;
	case CF_YCBCR_422:
		if (order == P2_CRCB || order == P2_CBCR)
			bpp = 8;
		else
			bpp = (!plane) ? 16 : 0;
		break;
	case CF_YCBCR_444:
		bpp = (!plane) ? 8 : 16;
		break;
	default:
		bpp = 0;
		break;
	}

	return width * bpp >> 3;
}

static inline enum dma_data_direction fimg2d_dma_direction(enum image_object i)
{
	switch (i) {
	case IMAGE_DST:
		return DMA_FROM_DEVICE;
	case IMAGE_SRC:
	case IMAGE_MSK:
		return DMA_TO_DEVICE;
	case IMAGE_TMP:
		return DMA_BIDIRECTIONAL;
	default:
		return DMA_NONE;
	}
}

static unsigned int fimg2d_map_dma_buf(struct fimg2d_control *info,
		struct fimg2d_dma *dma, int fd,
		enum dma_data_direction direction)
{
	dma_addr_t dma_addr;

	dma->direction = direction;
	dma->dma_buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dma->dma_buf)) {
		dev_err(info->dev, "dma_buf_get() failed: %ld\n",
				PTR_ERR(dma->dma_buf));
		goto err_buf_get;
	}

	dma->attachment = dma_buf_attach(dma->dma_buf, info->dev);
	if (IS_ERR_OR_NULL(dma->attachment)) {
		dev_err(info->dev, "dma_buf_attach() failed: %ld\n",
				PTR_ERR(dma->attachment));
		goto err_buf_attach;
	}

	dma->sg_table = dma_buf_map_attachment(dma->attachment,
			direction);
	if (IS_ERR_OR_NULL(dma->sg_table)) {
		dev_err(info->dev, "dma_buf_map_attachment() failed: %ld\n",
				PTR_ERR(dma->sg_table));
		goto err_buf_map_attachment;
	}

	dma_addr = iovmm_map(info->dev, dma->sg_table->sgl, 0,
			dma->dma_buf->size);
	if (!dma_addr || IS_ERR_VALUE(dma_addr)) {
		dev_err(info->dev, "iovmm_map() failed: %d\n", dma->dma_addr);
		goto err_iovmm_map;
	}

	dma->dma_addr = dma_addr;
	return dma->dma_buf->size;

err_iovmm_map:
	dma_buf_unmap_attachment(dma->attachment, dma->sg_table,
			direction);
err_buf_map_attachment:
	dma_buf_detach(dma->dma_buf, dma->attachment);
err_buf_attach:
	dma_buf_put(dma->dma_buf);
err_buf_get:
	return 0;
}

static void fimg2d_unmap_dma_buf(struct fimg2d_control *info,
		struct fimg2d_dma *dma)
{
	if (!dma->dma_addr)
		return;

	iovmm_unmap(info->dev, dma->dma_addr);
	dma_buf_unmap_attachment(dma->attachment, dma->sg_table,
			dma->direction);
	dma_buf_detach(dma->dma_buf, dma->attachment);
	dma_buf_put(dma->dma_buf);
	memset(dma, 0, sizeof(struct fimg2d_dma));
}

static int fimg2d_import_bufs(struct fimg2d_control *info,
		struct fimg2d_bltcmd *cmd)
{
	int ret = 0;
	int i;
	size_t j;

	for (i = 0; i < MAX_IMAGES; i++) {
		struct fimg2d_image *img = &cmd->image[i];
		size_t num_planes = fimg2d_num_planes(img);

		for (j = 0; j < num_planes; j++) {
			int stride;
			unsigned int buf_size = fimg2d_map_dma_buf(info,
					&cmd->dma[i][j], img->addr.fd[j],
					fimg2d_dma_direction(i));
			if (!buf_size) {
				ret = -ENOMEM;
				goto err;
			}

			if (is_yuvfmt(img->fmt)) {
				stride = yuv_stride(img->width, img->fmt,
						img->order, j);
			} else {
				stride = img->stride;
			}

			if (buf_size < stride * img->height) {
				dev_err(info->dev,
						"%s plane %u too small (height = %d, stride = %d, buf_size = %u)\n",
						imagename(i), j,
						img->height, stride, buf_size);
				ret = -EINVAL;
				goto err;
			}
		}
	}

err:
	if (ret)
		for (i = 0; i < MAX_IMAGES; i++)
			for (j = 0; j < FIMG2D_MAX_PLANES; j++)
				fimg2d_unmap_dma_buf(info, &cmd->dma[i][j]);

	return ret;
}

int fimg2d_add_command(struct fimg2d_control *info, struct fimg2d_context *ctx,
			struct fimg2d_blit *blit)
{
	int i, ret;
	struct fimg2d_image *buf[MAX_IMAGES] = image_table(blit);
	struct fimg2d_bltcmd *cmd;

	cmd = kzalloc(sizeof(*cmd), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd->ctx = ctx;
	cmd->op = blit->op;
	cmd->sync = blit->sync;
	cmd->seq_no = blit->seq_no;
	memcpy(&cmd->param, &blit->param, sizeof(cmd->param));

	for (i = 0; i < MAX_IMAGES; i++) {
		if (!buf[i])
			continue;

		if (copy_from_user(&cmd->image[i], buf[i],
					sizeof(struct fimg2d_image))) {
			ret = -EFAULT;
			goto err_user;
		}
	}

#ifdef CONFIG_VIDEO_FIMG2D_DEBUG
	fimg2d_dump_command(cmd);
#endif

	if (fimg2d_check_params(info, cmd)) {
		printk(KERN_ERR "[%s] invalid params\n", __func__);
		fimg2d_dump_command(cmd);
		ret = -EINVAL;
		goto err_user;
	}

	fimg2d_fixup_params(cmd);

	ret = fimg2d_import_bufs(info, cmd);
	if (ret)
		goto err_user;

	/* add command node and increase ncmd */
	spin_lock(&info->bltlock);
	if (atomic_read(&info->suspended)) {
		fimg2d_debug("fimg2d suspended, do sw fallback\n");
		spin_unlock(&info->bltlock);
		ret = -EFAULT;
		goto err_user;
	}
	atomic_inc(&ctx->ncmd);
	fimg2d_enqueue(&cmd->node, &info->cmd_q);
	fimg2d_debug("ctx %p ncmd(%d) seq_no(%u)\n",
			cmd->ctx, atomic_read(&ctx->ncmd), cmd->seq_no);
	spin_unlock(&info->bltlock);

	return 0;

err_user:
	kfree(cmd);
	return ret;
}

void fimg2d_del_command(struct fimg2d_control *info, struct fimg2d_bltcmd *cmd)
{
	int i;
	size_t j;

	struct fimg2d_context *ctx = cmd->ctx;

	for (i = 0; i < MAX_IMAGES; i++)
		for (j = 0; j < FIMG2D_MAX_PLANES; j++)
			fimg2d_unmap_dma_buf(info, &cmd->dma[i][j]);

	spin_lock(&info->bltlock);
	fimg2d_dequeue(&cmd->node);
	kfree(cmd);
	atomic_dec(&ctx->ncmd);
	spin_unlock(&info->bltlock);
}

void fimg2d_add_context(struct fimg2d_control *info, struct fimg2d_context *ctx)
{
	atomic_set(&ctx->ncmd, 0);
	init_waitqueue_head(&ctx->wait_q);

	atomic_inc(&info->nctx);
	fimg2d_debug("ctx %p nctx(%d)\n", ctx, atomic_read(&info->nctx));
}

void fimg2d_del_context(struct fimg2d_control *info, struct fimg2d_context *ctx)
{
	atomic_dec(&info->nctx);
	fimg2d_debug("ctx %p nctx(%d)\n", ctx, atomic_read(&info->nctx));
}
