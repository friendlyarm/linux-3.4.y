#ifndef FIMC_IS_VIDEO_H
#define FIMC_IS_VIDEO_H

#define FIMC_IS_MAX_BUF_NUM			(16)
#define FIMC_IS_MAX_PLANES			(4)
#define FIMC_IS_INVALID_BUF_INDEX		(0xFF)

enum fimc_is_video_state {
	FIMC_IS_VIDEO_BUFFER_PREPARED,
	FIMC_IS_VIDEO_BUFFER_READY,
	FIMC_IS_VIDEO_STREAM_ON
};

struct fimc_is_fmt {
	enum v4l2_mbus_pixelcode	mbus_code;
	char				*name;
	u32				pixelformat;
	u16				num_planes;
};

struct fimc_is_frame {
	struct fimc_is_fmt	format;
	u16			width;
	u16			height;
	u16			width_stride[FIMC_IS_MAX_PLANES];
	u32			size[FIMC_IS_MAX_PLANES];
};

struct fimc_is_video_common {
	struct video_device		vd;
	struct media_pad		pads;
	struct vb2_queue		vbq;
	const struct fimc_is_vb2	*vb2;
	struct mutex			lock;
	struct fimc_is_frame		frame;
	u32				buffers;
	u32				buffers_ready;
	u32				buf_ref_cnt;
	u32				buf_mask;

	unsigned long			state;

	void				*core;
	void				*device;

	u32 buf_dva[FIMC_IS_MAX_BUF_NUM][FIMC_IS_MAX_PLANES];
	u32 buf_kva[FIMC_IS_MAX_BUF_NUM][FIMC_IS_MAX_PLANES];
};

struct fimc_is_fmt *fimc_is_find_format(u32 *pixelformat,
	u32 *mbus_code, int index);
void fimc_is_set_plane_size(struct fimc_is_frame *frame,
						unsigned int sizes[]);

int fimc_is_video_probe(struct fimc_is_video_common *video,
	void *core_data,
	void *video_data,
	char *video_name,
	u32 video_number,
	u32 vbq_type,
	const struct v4l2_file_operations *fops,
	const struct v4l2_ioctl_ops *ioctl_ops,
	const struct vb2_ops *vb2_ops);
int fimc_is_video_open(struct fimc_is_video_common *video,
	void *device, u32 buffers_ready);
int fimc_is_video_close(struct fimc_is_video_common *video,
	struct fimc_is_framemgr *framemgr);
int fimc_is_video_reqbufs(struct fimc_is_video_common *video,
	struct fimc_is_framemgr *framemgr,
	struct v4l2_requestbuffers *request);
int fimc_is_video_set_format_mplane(struct fimc_is_video_common *video,
	struct v4l2_format *format);
int fimc_is_video_qbuf(struct fimc_is_video_common *video,
	struct v4l2_buffer *buf);
int fimc_is_video_dqbuf(struct fimc_is_video_common *video,
	struct v4l2_buffer *buf, bool blocking);
int fimc_is_video_streamon(struct fimc_is_video_common *video,
	enum v4l2_buf_type type);
int fimc_is_video_streamoff(struct fimc_is_video_common *video,
	enum v4l2_buf_type type);
int fimc_is_video_queue_setup(struct fimc_is_video_common *video,
	unsigned int *num_planes,
	unsigned int sizes[],
	void *allocators[]);
int fimc_is_video_buffer_queue(struct fimc_is_video_common *video,
	struct vb2_buffer *vb, struct fimc_is_framemgr *framemgr);

int buffer_done(struct fimc_is_video_common *video, u32 index);
#endif
