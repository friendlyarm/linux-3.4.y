/* linux/drivers/media/video/samsung/fimg2d4x/fimg2d_helper.c
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

#include "fimg2d.h"
#include "fimg2d_helper.h"

void fimg2d_dump_command(struct fimg2d_bltcmd *cmd)
{
	int i;
	struct fimg2d_param *p = &cmd->param;
	struct fimg2d_image *img;
	struct fimg2d_rect *r;

	printk(KERN_INFO " op: %d\n", cmd->op);
	printk(KERN_INFO " solid color: 0x%lx\n", p->solid_color);
	printk(KERN_INFO " g_alpha: 0x%x\n", p->g_alpha);
	printk(KERN_INFO " premultiplied: %d\n", p->premult);
	if (p->dither)
		printk(KERN_INFO " dither: %d\n", p->dither);
	if (p->rotate)
		printk(KERN_INFO " rotate: %d\n", p->rotate);
	if (p->repeat.mode) {
		printk(KERN_INFO " repeat: %d, pad color: 0x%lx\n",
				p->repeat.mode, p->repeat.pad_color);
	}
	if (p->bluscr.mode) {
		printk(KERN_INFO " bluescreen mode: %d, bs_color: 0x%lx " \
				"bg_color: 0x%lx\n",
				p->bluscr.mode, p->bluscr.bs_color,
				p->bluscr.bg_color);
	}
	if (p->scaling.mode) {
		printk(KERN_INFO " scaling %d, s:%d,%d d:%d,%d\n",
				p->scaling.mode,
				p->scaling.src_w, p->scaling.src_h,
				p->scaling.dst_w, p->scaling.dst_h);
	}
	if (p->clipping.enable) {
		printk(KERN_INFO " clipping LT(%d,%d) RB(%d,%d) WH(%d,%d)\n",
				p->clipping.x1, p->clipping.y1,
				p->clipping.x2, p->clipping.y2,
				rect_w(&p->clipping), rect_h(&p->clipping));
	}

	for (i = 0; i < MAX_IMAGES; i++) {
		size_t num_planes, j;
		img = &cmd->image[i];
		if (!img->addr.type)
			continue;

		r = &img->rect;

		num_planes = fimg2d_num_planes(img);
		for (j = 0; j < num_planes; j++) {
			printk(KERN_INFO " %s fd[%u]: %d\n",
					imagename(i), j, img->addr.fd[j]);
		}

		printk(KERN_INFO " %s width: %d height: %d "
				"stride: %d order: %d format: %d\n",
				imagename(i), img->width, img->height,
				img->stride, img->order, img->fmt);
		printk(KERN_INFO " %s rect LT(%d,%d) RB(%d,%d) WH(%d,%d)\n",
				imagename(i), r->x1, r->y1, r->x2, r->y2,
				rect_w(r), rect_h(r));
	}

	printk(KERN_INFO " ctx: %p seq_no(%u) sync(%d)\n",
				cmd->ctx, cmd->seq_no, cmd->sync);
}
