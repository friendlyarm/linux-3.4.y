/* linux/drivers/media/video/samsung/fimg2d4x/fimg2d_helper.h
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

#ifndef __FIMG2D_HELPER_H
#define __FIMG2D_HELPER_H

#include <linux/sched.h>
#include "fimg2d.h"

#define rect_w(r)	((r)->x2 - (r)->x1)
#define rect_h(r)	((r)->y2 - (r)->y1)

static inline char *imagename(enum image_object image)
{
	switch (image) {
	case IDST:
		return "DST";
	case ISRC:
		return "SRC";
	case IMSK:
		return "MSK";
	default:
		return NULL;
	}
}

static inline size_t fimg2d_num_planes(struct fimg2d_image *img)
{
	if (!img->addr.type)
		return 0;

	return img->order < P1_ORDER_END ? 1 : 2;
}

void fimg2d_dump_command(struct fimg2d_bltcmd *cmd);

#endif /* __FIMG2D_HELPER_H */
