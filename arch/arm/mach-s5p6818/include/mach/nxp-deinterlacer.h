#ifndef __KDEINTERLACE_H__
#define __KDEINTERLACE_H__

#include "ioc_magic.h"

#define MAX_BUFFER_PLANES	3

#define SRC_BUFFER_COUNT  3
#define DST_BUFFER_COUNT  1

#define DST_BUFFER_POOL_COUNT	4

typedef struct
{
	int frame_num;
	int plane_num;
	int frame_type;
	int frame_factor;

	union {
		struct {
			unsigned char *virt[MAX_BUFFER_PLANES];
			unsigned long sizes[MAX_BUFFER_PLANES];
			unsigned long src_stride[MAX_BUFFER_PLANES];
			unsigned long dst_stride[MAX_BUFFER_PLANES];

			int fds[MAX_BUFFER_PLANES];
			unsigned long phys[MAX_BUFFER_PLANES];
		}plane3;

		struct {
			unsigned char *virt[MAX_BUFFER_PLANES-1];
			unsigned long sizes[MAX_BUFFER_PLANES-1];
			unsigned long src_stride[MAX_BUFFER_PLANES-1];
			unsigned long dst_stride[MAX_BUFFER_PLANES-1];

			int fds[MAX_BUFFER_PLANES-1];
			unsigned long phys[MAX_BUFFER_PLANES-1];
		}plane2;
	};
}frame_data;

typedef struct
{
	//command
	int command;

	int width;
	int height;
	int plane_mode;

	frame_data dst_bufs[DST_BUFFER_COUNT];
	frame_data src_bufs[SRC_BUFFER_COUNT];
}frame_data_info;
	

//////////////////////////////////////////////////////////
typedef struct {
  int width;
  int height;
  int src_stride_factor;
  int dst_stride_factor;
  int format_type;
} CONTEXT_L;

typedef struct {
  int src_fds[3];
  unsigned char* src_virt[3];
} Q_BUF_L;

typedef struct {
  int dst_fds[3]; 
  unsigned char* dst_virt[3];
} DQ_BUF_L;

typedef struct {
    DQ_BUF_L dst_buf[DST_BUFFER_POOL_COUNT];
} DST_BUF_L;

typedef struct {
  DST_BUF_L *dst_buf;
  int buf_cnt;
} DQ_SET_L;

////////////////////////////////////////////////

typedef struct {
	int width;
	int height;
	int src_stride_factor;
	int dst_stride_factor;
	int format_type;
} CONTEXT;

typedef struct __EXE_CONTEXT{
	int dq_sync_timeline_fd;
	int q_sync_timeline_fd;
} EXE_CONTEXT;

typedef struct {
	int fence_fd;
	int fence_idx;
	unsigned long src_phys[3];
	unsigned char* src_virt[3];

	struct sync_pt *pt;
	struct sync_fence *fence;
} Q_BUF;

typedef struct {
	int fence_fd;
	int fence_idx;
	unsigned long dst_phys[3];
	unsigned char* dst_virt[3];

	struct sync_pt *pt;
	struct sync_fence *fence;
} DQ_BUF;

typedef struct {
	  DQ_BUF dst_buf[DST_BUFFER_POOL_COUNT];
} DST_BUF;

typedef struct {
	DST_BUF *dst_buf;
	int buf_cnt;
} DQ_SET;

//////////////////////////////////////////////////////////////

enum 
{
	IOCTL_DEINTERLACE_SET_AND_RUN										=	_IO(IOC_NX_MAGIC, 1),
	IOCTL_SET_CONTEXT																=	_IO(IOC_NX_MAGIC, 2),
	IOCTL_SET_DST_BUF																=	_IO(IOC_NX_MAGIC, 3),
	IOCTL_QBUF																			=	_IO(IOC_NX_MAGIC, 4),
	IOCTL_DQBUF																			=	_IO(IOC_NX_MAGIC, 5),
	IOCTL_START																			=	_IO(IOC_NX_MAGIC, 6),
	IOCTL_STOP																			=	_IO(IOC_NX_MAGIC, 7),
};

typedef enum 
{ 
  ACT_COPY=0, 
  ACT_DIRECT, 
  ACT_DIRECT_FD, 
  ACT_THREAD 
} ACT_MODE; 


enum FRAME_TYPE 
{
	FMT_YUYV = 0,
	FMT_YUV420M,
	FMT_YUV422P,
	FMT_YUV444
};

enum
{
	FRAME_SRC=1,
	FRAME_DST
};

enum
{
	Y_FRAME=0,
	CB_FRAME,
	CR_FRAME
};

enum
{
	PLANE2=2,
	PLANE3
};

enum 
{
	FORMAT_NV21=2,
	FORMAT_YV12	
};

enum
{
	FALSE=0,
	TRUE
};

#endif 
