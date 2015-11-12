/*
 * Copyright (C) 2013 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM fimg2d

#if !defined(_FIMG2D_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _FIMG2D_TRACE_H

#include <linux/tracepoint.h>

TRACE_EVENT(fimg2d_bitblt_start,
	TP_PROTO(int seq_no),
	TP_ARGS(seq_no),

	TP_STRUCT__entry(
		__field(int, seq_no)
	),
	TP_fast_assign(
		__entry->seq_no = seq_no;
	),
	TP_printk("seq_no=%d", __entry->seq_no)
);

TRACE_EVENT(fimg2d_bitblt_end,
	TP_PROTO(int seq_no),
	TP_ARGS(seq_no),

	TP_STRUCT__entry(
		__field(int, seq_no)
	),
	TP_fast_assign(
		__entry->seq_no = seq_no;
	),
	TP_printk("seq_no=%d", __entry->seq_no)
);

#endif /* _FIMG2D_TRACE_H */

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE fimg2d_trace
#include <trace/define_trace.h>
