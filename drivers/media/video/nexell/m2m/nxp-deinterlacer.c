#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/parport.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <asm/system.h>
#include <linux/hrtimer.h>

#include <linux/kfifo.h>

#include <linux/sync.h>
#include <linux/sw_sync.h>

#include <linux/file.h>

#include <linux/miscdevice.h>
#include <linux/dma-buf.h>

#include <mach/nxp-deinterlacer.h>
#include <mach/ion.h>

#include "../../../drivers/gpu/ion/ion_priv.h"

#include <linux/ion.h>
#include <linux/nxp_ion.h>

#include <nx_rstcon.h>
#include <nx_clkgen.h>

#include <nx_deinterlace.h>
#include <mach/nxp-deinterlacer.h>
#include <mach/platform.h>

#include "nxp-deinterlacer.h"

#define	DEVICE_NAME	"deinterlacer"


static ACT_MODE act_mode	=	ACT_THREAD;
static nxp_deinterlace *_deinterlace	=	NULL;

#define	TIMEOUT_NEVER	-1

#define DATA_QUEUE_SIZE		1024
#define DST_QUEUE_SIZE		1024
#define QUEUE_SIZE				1024

#define MINIMUM_INQUEUE_COUNT	SRC_BUFFER_COUNT

#define DUMP_REGISTER 1
#if (DUMP_REGISTER)
#define MY_DBGOUT(args...) printk(args)
#include <nx_deinterlace.h>
#else
#define MY_DBGOUT(args...)
#endif

#define DUMP_REG(name)  MY_DBGOUT("\t"#name "\t(0x%08x) = 0x%08x\n", ((unsigned int)&(pReg->name)), pReg->name)

#define SOURCE_WIDTH     352
#define SOURCE_HEIGHT    120

#define SOURCE_Y_STRIDE  352
#define SOURCE_C_STRIDE  176

#define DEST_Y_STRIDE    512
#define DEST_C_STRIDE    256

void dump_deinterlace_register(int module)
{
    struct NX_DEINTERLACE_RegisterSet *pReg =
        (struct NX_DEINTERLACE_RegisterSet *)NX_DEINTERLACE_GetBaseAddress();
    module = module;
    MY_DBGOUT("=========================================================\n");
    MY_DBGOUT("                DEINTERLACE REGISTER DUMP\n");
    MY_DBGOUT("=========================================================\n");
    MY_DBGOUT("-------> BaseAddress: 0x%04x\n", NX_DEINTERLACE_GetPhysicalAddress());
    DUMP_REG(START);
    DUMP_REG(MODE);
    DUMP_REG(INTENB);
    DUMP_REG(INTPEND);
    DUMP_REG(TSPARA);
    DUMP_REG(TMPARA);
    DUMP_REG(TIPARA);
    DUMP_REG(TPARA);
    DUMP_REG(BLENDPARA);
    DUMP_REG(SRCSIZE_Y);
    DUMP_REG(SRCADDRP_Y);
    DUMP_REG(SRCADDRC_Y);
    DUMP_REG(SRCADDRN_Y);
    DUMP_REG(SRCSTRD_Y);
    DUMP_REG(DESTADDRF_Y);
    DUMP_REG(DESTADDRD_Y);
    DUMP_REG(DESTSTRD_Y);
    DUMP_REG(SRCSIZE_CB);
    DUMP_REG(SRCADDRC_CB);
    DUMP_REG(SRCSTRD_CB);
    DUMP_REG(DESTADDRF_CB);
    DUMP_REG(DESTADDRD_CB);
    DUMP_REG(DESTSTRD_CB);
    DUMP_REG(SRCSIZE_CR);
    DUMP_REG(SRCADDRC_CR);
    DUMP_REG(SRCSTRD_CR);
    DUMP_REG(DESTADDRF_CR);
    DUMP_REG(DESTADDRD_CR);
    DUMP_REG(DESTSTRD_CR);
    MY_DBGOUT("\n");
}

static long read_file(char *FileName, char *data)
{
	int iFileCheck=0;
	int fd=-1;
	struct kstat  sbuf;
	mm_segment_t old_fs;
	long lFileSize=0;
	long lRtn=0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(FileName, O_RDONLY, 0);
	if(fd >= 0)
	{
 		if(vfs_lstat(FileName, &sbuf) == 0)
  	{
    	lFileSize = (long)sbuf.size;
    	sys_read(fd, data, lFileSize);

			*(data+lFileSize) = '\0';

			lRtn = lFileSize;
  	}
		else
			iFileCheck = 1;

  	sys_close(fd);
	}
	else
	{
		printk("File isn't Exitentace\n");
	}

	set_fs(old_fs);

	return lRtn;
}

void write_file(char *filename, char *data, long lFileSize)
{
	struct file *file;
	loff_t pos = 0;
	int fd;

	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(filename, O_WRONLY|O_CREAT, 0644);

	if (fd >= 0) {
		sys_write(fd, data, lFileSize);
		file = fget(fd);

		if (file) {
			vfs_write(file, data, lFileSize, &pos);
			fput(file);
		}
	 sys_close(fd);
	}
	set_fs(old_fs);
}

static int wait_for_end_of_work(wait_queue_head_t *wq, atomic_t *status, int init_status, int end_work_status, int elapse)
{
	long timeout = 0;

	if (atomic_read(status) == init_status) {
		timeout = wait_event_interruptible_timeout(*wq, atomic_read(status) == end_work_status, elapse);
		if(timeout <= 0)
		{
			printk(KERN_INFO "The timeout elapsed before the condition evaluated to true!! Condition Status = %d, timeout : %ld\n", atomic_read(status), timeout);
			return -EBUSY;
		}
	}
	else
		return -1;

	return 0;
}

static int check_work_duplicating(wait_queue_head_t *wq, atomic_t *status, int init_status, int end_work_status, int elapse)
{
	if(atomic_read(status) == init_status)
	{
		if (wait_event_interruptible_timeout(*wq, atomic_read(status)==end_work_status, elapse)<=0)
		{
			printk("Wait timeout for starting\n");
			return -EBUSY;
		}
	}

	atomic_set(status, init_status);

	return 0;
}

static void set_irq(void)
{
	tag_msg(KERN_INFO "%s++>\n", __func__);

	_deinterlace->irq = NX_DEINTERLACE_GetInterruptNumber();
	_deinterlace->irq += 32;

	tag_msg(KERN_INFO "%s<--\n", __func__);
}

static void _initialize(void)
{
	tag_msg(KERN_INFO "%s++>\n", __func__);

	NX_DEINTERLACE_Initialize();
  NX_DEINTERLACE_SetBaseAddress( (void*)IO_ADDRESS(NX_DEINTERLACE_GetPhysicalAddress()) );

	NX_CLKGEN_SetBaseAddress( NX_DEINTERLACE_GetClockNumber(), (void*)IO_ADDRESS(NX_CLKGEN_GetPhysicalAddress(NX_DEINTERLACE_GetClockNumber())));
	NX_CLKGEN_SetClockBClkMode( NX_DEINTERLACE_GetClockNumber(), NX_BCLKMODE_ALWAYS );

#if 0
	NX_RSTCON_SetRST( NX_DEINTERLACE_GetResetNumber(), RSTCON_ASSERT );
	NX_RSTCON_SetRST( NX_DEINTERLACE_GetResetNumber(), RSTCON_NEGATE );
#else
	NX_RSTCON_SetRST(NX_DEINTERLACE_GetResetNumber(), RSTCON_NEGATE);
#endif

  NX_DEINTERLACE_OpenModule();
  NX_DEINTERLACE_SetInterruptEnableAll( CFALSE );
  NX_DEINTERLACE_ClearInterruptPendingAll();

	tag_msg(KERN_INFO "%s<--\n", __func__);
}

static void _deinitialize(void)
{
	tag_msg(KERN_INFO "%s++>\n", __func__);

	NX_DEINTERLACE_SetInterruptEnableAll(CFALSE);
	NX_DEINTERLACE_ClearInterruptPendingAll();

	tag_msg(KERN_INFO "%s<--\n", __func__);
}

static void SetDeInterlace( U16 Height, U16 Width,
                     U32 Y_PrevAddr,  U32 Y_CurrAddr,   U32 Y_NextAddr, U32 Y_SrcStride, U32 Y_DstAddr, U32 Y_DstStride,
                     U32 CB_CurrAddr, U32 CB_SrcStride, U32 CB_DstAddr, U32 CB_DstStride,
                     U32 CR_CurrAddr, U32 CR_SrcStride, U32 CR_DstAddr, U32 CR_DstStride,
                     int IsODD)
{

    U32 YDstFieldStride     = (Y_DstStride*2);
    U32 CbDstFieldStride    = (CB_DstStride*2);
    U32 CrDstFieldStride    = (CR_DstStride*2);
    U16 CWidth              = (U16)(Width/2);
    U32 CHeight             = (U16)(Height/2);

    // Y Register Setting
    NX_DEINTERLACE_SetYSrcImageSize ( Height, Width );
    NX_DEINTERLACE_SetYSrcAddrPrev  ( Y_PrevAddr );
    NX_DEINTERLACE_SetYSrcAddrCurr  ( Y_CurrAddr );
    NX_DEINTERLACE_SetYSrcAddrNext  ( Y_NextAddr );
    NX_DEINTERLACE_SetYSrcStride    ( Y_SrcStride );
    NX_DEINTERLACE_SetYDestStride   ( YDstFieldStride );

    // CB Regiseter Setting
    NX_DEINTERLACE_SetCBSrcImageSize( CHeight, CWidth );
    NX_DEINTERLACE_SetCBSrcAddrCurr ( CB_CurrAddr );
    NX_DEINTERLACE_SetCBSrcStride   ( CB_SrcStride );
    NX_DEINTERLACE_SetCBDestStride  ( CbDstFieldStride );

    // CR Regiseter Setting
    NX_DEINTERLACE_SetCRSrcImageSize( CHeight, CWidth );
    NX_DEINTERLACE_SetCRSrcAddrCurr ( CR_CurrAddr );
    NX_DEINTERLACE_SetCRSrcStride   ( CR_SrcStride );
    NX_DEINTERLACE_SetCRDestStride  ( CrDstFieldStride );

    // Parameter Setting
    NX_DEINTERLACE_SetASParameter   (  10,  18 );
    NX_DEINTERLACE_SetMDSADParameter(   8,  16 );
    NX_DEINTERLACE_SetMIParameter   (  50, 306 );
    NX_DEINTERLACE_SetYSParameter   ( 434, 466 );
    NX_DEINTERLACE_SetBLENDParameter(        3 );


    if (IsODD) {
        // Y Register Set
        NX_DEINTERLACE_SetYDestAddrDIT  ( (Y_DstAddr+Y_DstStride ) );
        NX_DEINTERLACE_SetYDestAddrFil  (  Y_DstAddr               );
        // CB Register Set
        NX_DEINTERLACE_SetCBDestAddrDIT ( (CB_DstAddr+CB_DstStride) );
        NX_DEINTERLACE_SetCBDestAddrFil (  CB_DstAddr               );
        // CR Register Set
        NX_DEINTERLACE_SetCRDestAddrDIT ( (CR_DstAddr+CR_DstStride) );
        NX_DEINTERLACE_SetCRDestAddrFil (  CR_DstAddr               );
        // Start
        NX_DEINTERLACE_SetYCBCREnable   ( CTRUE, CTRUE, CTRUE );
        NX_DEINTERLACE_SetYCBCRField    ( NX_DEINTERLACE_FIELD_EVEN, NX_DEINTERLACE_FIELD_EVEN, NX_DEINTERLACE_FIELD_EVEN );
    } else {
        // Y Register Set
        NX_DEINTERLACE_SetYDestAddrDIT  (  Y_DstAddr              );
        NX_DEINTERLACE_SetYDestAddrFil  ( (Y_DstAddr+Y_DstStride) );
        // CB Register Set
        NX_DEINTERLACE_SetCBDestAddrDIT (  CB_DstAddr               );
        NX_DEINTERLACE_SetCBDestAddrFil ( (CB_DstAddr+CB_DstStride) );
        // CR Register Set
        NX_DEINTERLACE_SetCRDestAddrDIT (  CR_DstAddr               );
        NX_DEINTERLACE_SetCRDestAddrFil ( (CR_DstAddr+CR_DstStride) );
        // Start
        NX_DEINTERLACE_SetYCBCREnable   ( CTRUE, CTRUE, CTRUE );
        NX_DEINTERLACE_SetYCBCRField    ( NX_DEINTERLACE_FIELD_ODD, NX_DEINTERLACE_FIELD_ODD, NX_DEINTERLACE_FIELD_ODD );
    }
}

static void SetDeInterlaceCbCr( U16 Height, U16 Width,
                     U32 Y_PrevAddr,  U32 Y_CurrAddr,   U32 Y_NextAddr, U32 Y_SrcStride, U32 Y_DstAddr, U32 Y_DstStride,
                     U32 CBCR_CurrAddr, U32 CBCR_SrcStride, U32 CBCR_DstAddr, U32 CBCR_DstStride,
                     int IsODD)
{
    U32 YDstFieldStride     = (Y_DstStride*2);
    U32 CbCrDstFieldStride  = (CBCR_DstStride*2);
    U16 CWidth              = (U16)(Width);
    U32 CHeight             = (U16)(Height/2);

    // Y Register Setting
    NX_DEINTERLACE_SetYSrcImageSize ( Height, Width );
    NX_DEINTERLACE_SetYSrcAddrPrev  ( Y_PrevAddr );
    NX_DEINTERLACE_SetYSrcAddrCurr  ( Y_CurrAddr );
    NX_DEINTERLACE_SetYSrcAddrNext  ( Y_NextAddr );
    NX_DEINTERLACE_SetYSrcStride    ( Y_SrcStride );
    NX_DEINTERLACE_SetYDestStride   ( YDstFieldStride );

    // CB Regiseter Setting
    NX_DEINTERLACE_SetCBSrcImageSize( CHeight, CWidth );
    NX_DEINTERLACE_SetCBSrcAddrCurr ( CBCR_CurrAddr );
    NX_DEINTERLACE_SetCBSrcStride   ( CBCR_SrcStride );
    NX_DEINTERLACE_SetCBDestStride  ( CbCrDstFieldStride );

    // Parameter Setting
    NX_DEINTERLACE_SetASParameter   (  10,  18 );
    NX_DEINTERLACE_SetMDSADParameter(   8,  16 );
    NX_DEINTERLACE_SetMIParameter   (  50, 306 );
    NX_DEINTERLACE_SetYSParameter   ( 434, 466 );
    NX_DEINTERLACE_SetBLENDParameter(        3 );

    if (IsODD) {
        // Y Register Set
        NX_DEINTERLACE_SetYDestAddrDIT  ( (Y_DstAddr+Y_DstStride ) );
        NX_DEINTERLACE_SetYDestAddrFil  (  Y_DstAddr               );
        // CB Register Set
        NX_DEINTERLACE_SetCBDestAddrDIT ( (CBCR_DstAddr+CBCR_DstStride) );
        NX_DEINTERLACE_SetCBDestAddrFil (  CBCR_DstAddr               );
        // Start
        NX_DEINTERLACE_SetYCBCREnable   ( CTRUE, CTRUE, CFALSE );
        NX_DEINTERLACE_SetYCBCRField    ( NX_DEINTERLACE_FIELD_EVEN, NX_DEINTERLACE_FIELD_EVEN, NX_DEINTERLACE_FIELD_EVEN );
    } else {
        // Y Register Set
        NX_DEINTERLACE_SetYDestAddrDIT  (  Y_DstAddr              );
        NX_DEINTERLACE_SetYDestAddrFil  ( (Y_DstAddr+Y_DstStride) );
        // CB Register Set
        NX_DEINTERLACE_SetCBDestAddrDIT (  CBCR_DstAddr               );
        NX_DEINTERLACE_SetCBDestAddrFil ( (CBCR_DstAddr+CBCR_DstStride) );
        // Start
        NX_DEINTERLACE_SetYCBCREnable   ( CTRUE, CTRUE, CFALSE );
        NX_DEINTERLACE_SetYCBCRField    ( NX_DEINTERLACE_FIELD_ODD, NX_DEINTERLACE_FIELD_ODD, NX_DEINTERLACE_FIELD_ODD );
    }
}

static int alloc_app_data(unsigned char **dst, unsigned char **src, dma_addr_t *dst_phy, int buf_size)
{
	int err;
	unsigned char **dst_buf_vir;
	unsigned char **src_buf_vir;
	dma_addr_t *dst_buf_phy;
	int buf_len=0;

	dst_buf_vir = dst;
	src_buf_vir = src;
	dst_buf_phy	=	dst_phy;
	buf_len	=	buf_size;

	if(*src == NULL)
	{
		pr_err("%s: src buffer error!!!\n", __func__);
		return -1;
	}

	*dst_buf_vir = dma_alloc_coherent(NULL, buf_len, dst_buf_phy, GFP_KERNEL);
	if(dst_buf_vir == NULL)
	{
		pr_err("%s: failed to allocate command buffer!!!\n", __func__);
		return -1;
	}

	err = copy_from_user(*dst_buf_vir, *src_buf_vir, buf_len);
	if(err < 0)
	{
		printk(KERN_ERR "copy from user error!\n");
		return -2;
	}

	return 0;
}

static int dealloc_app_data(unsigned char **release_buf, dma_addr_t *release_buf_phy, int buf_size)
{
	unsigned char **dst_buf;
	dma_addr_t *dst_buf_phy;

	dst_buf = release_buf;
	dst_buf_phy = release_buf_phy;

	if( dst_buf_phy )
	{
		dma_free_coherent(NULL, buf_size, *dst_buf, *dst_buf_phy);
		*dst_buf = NULL;
		*dst_buf_phy =	0;
	}

	return 0;
}

static int alloc_dst_data(unsigned char **dst, unsigned char **src, int dst_size, int src_size, int dst_stride_val, int src_stride_val)
{
	int i=0;
	int	src_data_count = 0;
	int dst_data_size = 0;

	unsigned char *dst_real_data = NULL;
	unsigned char **src_data	=	NULL;
	unsigned char **dst_data	=	NULL;

	int dst_stride = 0;
	int src_stride = 0;
	int src_data_size = 0;

	src_data = src;
	src_data_size = src_size;
	dst_data = dst;
	dst_data_size = dst_size;
	dst_stride = dst_stride_val;
	src_stride = src_stride_val;

	src_data_count = (src_data_size/dst_stride) ;
	dst_real_data = kzalloc(dst_data_size, GFP_KERNEL);
	if( !dst_real_data )
	{
		printk(KERN_ERR "Real Data kzalloc allocation error!!\n");
		return -ENOMEM;
	}

	for(i=0 ; i<src_data_count ; i++)
		memcpy(dst_real_data+(i * src_stride), *src_data+(i * dst_stride), src_stride);

	*dst_data = dst_real_data;

	return 0;
}

static void dealloc_dst_data(unsigned char **dst)
{
	unsigned char **dst_data;

	dst_data = dst;

	if( *dst_data )
	{
		kfree(*dst_data);
		*dst_data = NULL;
	}
}

struct sw_sync_timeline *sw_sync_timeline_fdget(int fd)
{
	struct file *file = fget(fd);

	tag_msg(KERN_INFO "[%s++>]fd1 : %d\n", __func__, fd);

	if(file == NULL)
		return NULL;

	tag_msg(KERN_INFO "[%s<--]fd2 : %d\n", __func__, fd);

	return file->private_data;
}

static int insert_fd_list(int fd)
{
	FD_LIST fd_list;
	int ret;

	fd_list.fence_fd = fd;

	ret = kfifo_in_spinlocked(&_deinterlace->fd_queue, &fd_list, sizeof(FD_LIST), &_deinterlace->queue_fd_list_lock);
	if(ret != sizeof(FD_LIST))
	{
		printk(KERN_ERR "fd queue kfifo in error\n");
		return -1;
	}

	return 0;
}

static int release_fd(int fd)
{
	int ret;
	FD_LIST fd_list;

	ret = kfifo_out_spinlocked(&_deinterlace->fd_queue, &fd_list, sizeof(FD_LIST), &_deinterlace->queue_fd_list_lock);
	if(ret != sizeof(FD_LIST))
	{
		printk(KERN_ERR "fd queue kfifo out error\n");
		return -1;
	}

	if( fd_list.fence_fd == fd )
		put_unused_fd(fd_list.fence_fd);

	return 0;
}

static long create_fence(DQ_BUF *dqbuf)
{
  int err=0;

  char str[256];
  char str_t[256];

  struct sync_pt *pt;
  struct sync_fence *fence;
  struct sync_fence *fence_t;

	int new_fence_fd=-1;
	int old_fence_fd=-1;
	int fd_idx=0;

  tag_msg(KERN_INFO "%s++>\n", __func__);

  if(_deinterlace->dq_timeline == NULL)
  {
    printk(KERN_ERR "[ERR]timeline init error %d\n", dqbuf->fence_fd);
    return -ENOMEM;
  }

	old_fence_fd = dqbuf->fence_fd;

  _deinterlace->dq_timeline_max++;

	sprintf(str, "dst_fence_%d", _deinterlace->dq_timeline_max);
  pt = sw_sync_pt_create(_deinterlace->dq_timeline, _deinterlace->dq_timeline_max);
  if(pt == NULL)
  {
    printk(KERN_ERR "[err]sync pt create!!\n");
    err = -ENOMEM;
    goto err;
  }

  fence = sync_fence_create(str, pt);
  if (fence == NULL) {
    goto fence_err;
  }

  if(old_fence_fd>=0)
  {
		sprintf(str_t, "sync_fence_merge_%d", _deinterlace->dq_timeline_max);

    fence_t = sync_fence_merge(str_t, dqbuf->fence, fence);
    sync_fence_put(dqbuf->fence);
    sync_fence_put(fence);

		sys_close(old_fence_fd);

		new_fence_fd = get_unused_fd();
  	if( new_fence_fd<0 )
		{
  		tag_msg(KERN_INFO "[%s] FENCE ERROR!!!\n", __func__);
			goto fd_err;
		}


    dqbuf->pt = pt;
    dqbuf->fence = fence_t;
		dqbuf->fence_fd = new_fence_fd;

		fd_idx = _deinterlace->fd_idx;
		if(fd_idx == _deinterlace->dqset.buf_cnt) _deinterlace->fd_idx = 0;
		memcpy(((_deinterlace->dst_dqbuf_all)+(_deinterlace->fd_idx++)),  dqbuf, sizeof(DQ_BUF));

		dbg_msg(KERN_INFO "[%s]1 fd_idx : %d, dqbuf->fence_fd : %d, dst_dqbuf_all fence fd : %d\n", __func__, fd_idx, (_deinterlace->fd_idx)-1, ((_deinterlace->dst_dqbuf_all)+((_deinterlace->fd_idx)-1))->fence_fd);
  }
  else
  {
    dqbuf->pt = pt;
    dqbuf->fence = fence;

		new_fence_fd = get_unused_fd();
  	if( new_fence_fd<0 ) goto fd_err;

		dqbuf->fence_fd = new_fence_fd;
		memcpy(((_deinterlace->dst_dqbuf_all)+(_deinterlace->fd_idx++)),  dqbuf, sizeof(DQ_BUF));

		dbg_msg(KERN_INFO "[%s]0  fd_idx : %d, dqbuf->fence_fd : %d, dst_dqbuf_all fence fd : %d\n", __func__, new_fence_fd, (_deinterlace->fd_idx)-1, ((_deinterlace->dst_dqbuf_all)+((_deinterlace->fd_idx)-1))->fence_fd);
  }

	sync_fence_install(dqbuf->fence, dqbuf->fence_fd);

  tag_msg(KERN_INFO "%s<--\n", __func__);

  return 0;

fd_err:
  put_unused_fd(new_fence_fd);

fence_err:
  sync_pt_free(pt);

err:

  return -1;
}

static long pending_processing(DQ_BUF *dqbuf)
{
	int err=0;

	tag_msg(KERN_INFO "%s++>\n", __func__);

	if(sync_fence_fdget(dqbuf->fence_fd)){
		err = sync_fence_wait(sync_fence_fdget(dqbuf->fence_fd), 1000);
		if(err == -ETIME) {
			err = sync_fence_wait(sync_fence_fdget(dqbuf->fence_fd), TIMEOUT_NEVER);
			if (err < 0) {
				printk(KERN_ERR "dst fence fd :  failed: %d\n", err);
			} else {
				printk(KERN_INFO "dst fence fd :  done\n");
			}
		}
	}

	tag_msg(KERN_INFO "%s<--\n", __func__);

	return err;
}

static int get_phy_addr_from_fd(int fd, unsigned long *phyaddr)
{

	struct dma_buf *dmabuf;
	struct ion_buffer *buffer;
	unsigned long result;
	int fdesc = 0;

	fdesc = fd;

	dmabuf = dma_buf_get(fd);
	if( IS_ERR_OR_NULL(dmabuf))
	{
		pr_err("%s: can't get dmabuf : fd{%d)\n", __func__, fd);
		return -EINVAL;
	}

	buffer =(struct ion_buffer *)dmabuf->priv;
	result =(unsigned long)buffer->priv_phys;
	*phyaddr = result;

	dma_buf_put(dmabuf);

	return 0;
}

static int get_count_data_queue(void)
{
	int enqueue_cnt = 0;

	enqueue_cnt = atomic_read(&_deinterlace->src_buf_count);

	return enqueue_cnt;
}

static int get_count_data_dequeue(void)
{
	int dequeue_cnt = 0;

	dequeue_cnt = kfifo_len(&_deinterlace->data_dequeue)/sizeof(DQ_BUF);

	return dequeue_cnt;
}

static int get_status(void)
{
	return atomic_read(&_deinterlace->status);
}

static int get_hw_status(void)
{
	return atomic_read(&_deinterlace->hw_status);
}

static void set_hw_status(int hw_status)
{
	atomic_set(&_deinterlace->hw_status, hw_status);
}

static bool is_sw_running(void)
{

	if( atomic_read(&_deinterlace->status) == PROCESSING_READY) return true;

	return false;
}

static bool is_dst_buf_full(void)
{

	tag_msg(KERN_INFO "%s++>\n", __func__);

	if( atomic_read(&_deinterlace->status) == PROCESSING_PENDING) return true;

	tag_msg(KERN_INFO "%s<--\n", __func__);

	return false;
}

static int set_process_status(int status, bool forced)
{
	int ret_status = -1;

	if( !forced )
	{
		if(atomic_read(&_deinterlace->status) != PROCESSING_FINISH)
			atomic_set(&_deinterlace->status, status);
	}
	else
		atomic_set(&_deinterlace->status, status);

	ret_status = status;

	return ret_status;
}

static void init_state(void)
{
	int i=0;

	tag_msg(KERN_INFO "%s++>\n", __func__);

	set_process_status(PROCESSING_FINISH, true);
	set_hw_status(DEINTERLACING_STOP);

	atomic_set(&_deinterlace->src_buf_count, 0);
	atomic_set(&_deinterlace->dst_buf_count, 0);

	for(i=0; i<MAX_BUFFER_PLANES ; i++)
	{
		_deinterlace->qbuf_temp[i].src_phys[0] = 0;
		_deinterlace->qbuf_temp[i].src_phys[1] = 0;
		_deinterlace->qbuf_temp[i].src_phys[2] = 0;
	}

	tag_msg(KERN_INFO "%s<--\n", __func__);
}

static int check_exit_data(Q_BUF *qbuf)
{

	if(
		(qbuf->src_phys[0] == 0) && (qbuf->src_phys[1] == 0) && (qbuf->src_phys[2] == 0)
		&& (qbuf->src_virt[0] == NULL) && (qbuf->src_virt[1] == NULL) && (qbuf->src_virt[2] == NULL)
	)
	{
		atomic_set(&_deinterlace->exit_flag, 1);
		wake_up_interruptible(&_deinterlace->wq_dst);

		return -1;
	}

	return 0;
}

static void dealloc_dst_buf(void)
{
	if( _deinterlace->dst_dqbuf_all != NULL)
	{
		kfree(_deinterlace->dst_dqbuf_all);
		_deinterlace->dst_dqbuf_all = NULL;
	}
}

static int alloc_dst_buf(DQ_SET *dqset)
{
	int size=0;
	DQ_BUF  *dst_dqbuf;

	tag_msg(KERN_INFO "%s++>\n", __func__);

	if(_deinterlace==NULL)
	{
		printk("_deinterlace is null!!");
		return -ENOMEM;
	}

	dealloc_dst_buf();

	size  = sizeof(DQ_BUF) * dqset->buf_cnt;
	dst_dqbuf = (DQ_BUF *)kzalloc(size, GFP_KERNEL);
//	dst_dqbuf = (DQ_BUF *)kzalloc(size, GFP_ATOMIC);

	printk(KERN_INFO "[%s] Trace-2\n", __func__);
	if(dst_dqbuf == NULL)
	{
		printk(KERN_ERR "[%s] dequeue kzalloc allocation error!!\n", __func__);
		return -ENOMEM;
	}

	if( mutex_lock_interruptible(&_deinterlace->read_lock))
		return -EINTR;

	if(copy_from_user(dst_dqbuf, (DQ_BUF *)dqset->dst_buf, size))
	{
		mutex_unlock(&_deinterlace->read_lock);
		printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
		return -EFAULT;
	}

	_deinterlace->dst_dqbuf_all = dst_dqbuf;

	mutex_unlock(&_deinterlace->read_lock);

	tag_msg(KERN_INFO "%s<--\n", __func__);

	return 0;
}

static int set_dst_buf(DQ_SET *dqset)
{
	int i=0;
	int ret=0;

	DQ_BUF  *dst_dqbuf;

	tag_msg(KERN_INFO "%s++>\n", __func__);

	dst_dqbuf = _deinterlace->dst_dqbuf_all;

	if(dst_dqbuf == NULL)
	{
		printk(KERN_ERR "[%s]dequeue kzalloc allocation error!!\n", __func__);
		return -ENOMEM;
	}

	for(i=0 ; i<dqset->buf_cnt ; i++)
	{
		(dst_dqbuf+i)->fence_fd = -1;
		ret = kfifo_in_spinlocked(&_deinterlace->data_dequeue_unused, dst_dqbuf+i, sizeof(DQ_BUF), &_deinterlace->dequeue_unused_lock);
		if(ret != sizeof(DQ_BUF))
		{
			printk(KERN_ERR "dst unused kfifo in error\n");
			return -1;
		}
	}

	tag_msg(KERN_INFO "%s<--\n", __func__);

	return 0;
}

static void sw_start(void)
{
	tag_msg(KERN_INFO "%s++>\n", __func__);

	atomic_set(&_deinterlace->exit_flag, 0);
	_deinterlace->fd_idx = 0;
	_deinterlace->is_waitting = false;

	set_process_status(PROCESSING_READY, true);
	set_dst_buf(&_deinterlace->dqset);

	tag_msg(KERN_INFO "[%s]IRQ : %d\n", __func__, _deinterlace->irq);
	enable_irq(_deinterlace->irq); //error

	tag_msg(KERN_INFO "%s-2\n", __func__);
	if( get_count_data_queue() >= MINIMUM_INQUEUE_COUNT)
	{
		tag_msg(KERN_INFO "%s-3\n", __func__);
		queue_work(_deinterlace->wq_proc, &_deinterlace->w_proc);
	}

	tag_msg(KERN_INFO "%s--\n", __func__);
}

static void sw_stop(void)
{
	tag_msg(KERN_INFO "%s++\n", __func__);

	//*. release item
	//*. flush_workqueue, timeline, fence(timeline max value), queue, dequeue,
	set_process_status(PROCESSING_FINISH, true);
	disable_irq(_deinterlace->irq);
	//disable_irq_nosync(_deinterlace->irq);

	cancel_work_sync(&_deinterlace->w_proc);

	//clear fd
	queue_work(_deinterlace->wq_proc, &_deinterlace->w_proc);
	flush_workqueue(_deinterlace->wq_proc);

	kfifo_reset(&_deinterlace->data_queue);
	kfifo_reset(&_deinterlace->data_dequeue_unused);
	kfifo_reset(&_deinterlace->data_dequeue);
	kfifo_reset(&_deinterlace->fd_queue);

	wake_up_interruptible(&_deinterlace->wq_start);
	wake_up_interruptible(&_deinterlace->wq_end);

	atomic_set(&_deinterlace->exit_flag, 1);
	wake_up_interruptible(&_deinterlace->wq_dst);

	init_state();

	tag_msg(KERN_INFO "%s--\n", __func__);
}

static int get_next_dstbuf(DQ_BUF *dqbuf)
{
	int ret=0;

	tag_msg(KERN_INFO "%s++\n", __func__);

	if(kfifo_is_empty(&_deinterlace->data_dequeue_unused))
	{
		printk(KERN_ERR "empty unused queue error\n");
		return -1;
	}

	ret = kfifo_out_spinlocked(&_deinterlace->data_dequeue_unused, dqbuf, sizeof(DQ_BUF), &_deinterlace->dequeue_unused_lock);
	if(ret != sizeof(DQ_BUF))
	{
		printk(KERN_ERR "unused queue kfifo out error\n");
		return -1;
	}

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int insert_queue_frame(Q_BUF *qbuf)
{
	int ret=0;

	tag_msg(KERN_INFO "%s++\n", __func__);

	if((qbuf->src_phys[0] <= 0) || (qbuf->src_phys[0] <= 0) || (qbuf->src_phys[0] <= 0))
		return -1;

	ret = kfifo_in_spinlocked(&_deinterlace->data_queue, qbuf, sizeof(Q_BUF), &_deinterlace->queue_lock);
	if(ret != sizeof(Q_BUF))
	{
		return -1;
	}

	atomic_inc(&_deinterlace->src_buf_count);

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int _queue_frame_data(Q_BUF *qbuf)
{
	Q_BUF qbuf_t;
	int ret=0;

	tag_msg(KERN_INFO "%s++\n", __func__);

	if( get_status() == PROCESSING_FINISH )
	{
		return -1;
	}

	qbuf_t = *qbuf;

	ret = insert_queue_frame(&qbuf_t);
	if(ret != 0) return ret;

  _deinterlace->q_timeline_max++;

	if( get_count_data_queue() >= MINIMUM_INQUEUE_COUNT && is_sw_running() )
	{
		queue_work(_deinterlace->wq_proc, &_deinterlace->w_proc);
	}

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int _dequeue_frame_data(DQ_BUF *dqbuf)
{
	int ret=0;
	DQ_BUF dqbuf_t;

	if(kfifo_is_empty(&_deinterlace->data_dequeue))
	{
		ret = wait_event_interruptible(_deinterlace->wq_dst, ((atomic_read(&_deinterlace->exit_flag)>0) || (!kfifo_is_empty(&_deinterlace->data_dequeue))));
		if (ret == -ERESTARTSYS)
		{
			pr_err("interrupted\n");
			return -1;
		}

		if( (atomic_read(&_deinterlace->exit_flag)>0) || kfifo_is_empty(&_deinterlace->data_dequeue))
		{
			atomic_set(&_deinterlace->exit_flag, 0);
			return -1;
		}
	}

	dqbuf_t = *dqbuf;

	ret = kfifo_out_spinlocked(&_deinterlace->data_dequeue, &dqbuf_t, sizeof(DQ_BUF), &_deinterlace->dequeue_lock);
	if(ret != sizeof(DQ_BUF))
	{
		printk(KERN_ERR "dequeue kfifo out error\n");
		return -1;
	}

	if(_deinterlace->ctx.format_type == FORMAT_NV21)
		dqbuf_t.dst_phys[2] = 0x0;

	*dqbuf = dqbuf_t;

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int  get_out_data_target(int frame_kind)
{
	int ret;
	int frame_y, frame_cb, frame_cr, frame_cbcr;

	tag_msg(KERN_INFO "%s++\n", __func__);

	ret = QUEUE_DATA;

	if( frame_kind < FRAME_NEXT)
	{
		switch(_deinterlace->ctx.format_type)
		{
			case FORMAT_NV21:
				frame_y = _deinterlace->qbuf_temp[frame_kind].src_phys[0];
				frame_cbcr = _deinterlace->qbuf_temp[frame_kind].src_phys[1];

				if( frame_y > 0 && frame_cbcr > 0)
					ret = BUFFER_DATA;
				break;
			case FORMAT_YV12:
				frame_y = _deinterlace->qbuf_temp[frame_kind].src_phys[0];
				frame_cb = _deinterlace->qbuf_temp[frame_kind].src_phys[1];
				frame_cr = _deinterlace->qbuf_temp[frame_kind].src_phys[2];

				if( frame_y > 0 && frame_cb > 0 && frame_cr > 0)
					ret = BUFFER_DATA;
				break;
		}
	}

	tag_msg(KERN_INFO "%s--\n", __func__);

	return ret;
}

static int get_queue_data(Q_BUF *qbuf, int frame_kind)
{
	int ret=0;
	int data_target;

	data_target = get_out_data_target(frame_kind);

	if( data_target == QUEUE_DATA)
	{
		ret = kfifo_out_spinlocked(&_deinterlace->data_queue, qbuf, sizeof(Q_BUF), &_deinterlace->queue_lock);
		if(ret != sizeof(Q_BUF))
		{
			printk(KERN_ERR "queue kfifo out error\n");
			return -1;
		}
	}
	else
	{
		*qbuf = _deinterlace->qbuf_temp[frame_kind+1];
	}

	return 0;
}

static bool process_next_item(wait_queue_head_t *wq_start, bool *is_waitting)
{
	if( *is_waitting )
	{
		wake_up_interruptible(wq_start);
		*is_waitting = false;

		return true;
	}

	return false;
}

static int processing_2plane(DQ_BUF *dqbuf)
{
	unsigned long src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy;
	unsigned long src_prev_cbcr_data_phy, src_curr_cbcr_data_phy, src_next_cbcr_data_phy;

	unsigned long dst_y_data_phy, dst_cbcr_data_phy;
	int width, height, src_y_stride, src_c_stride, dst_y_stride, dst_c_stride;

	tag_msg(KERN_INFO "%s++\n", __func__);

	if( get_queue_data(&_deinterlace->qbuf_temp[0], FRAME_PREV) < 0 ||
			get_queue_data(&_deinterlace->qbuf_temp[1], FRAME_CURR) < 0 ||
			get_queue_data(&_deinterlace->qbuf_temp[2], FRAME_NEXT) < 0 )
	{
		printk(KERN_INFO "[%s]get queue data error!!\n", __func__);
		return -1;
	}

	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);

	src_prev_y_data_phy		= _deinterlace->qbuf_temp[0].src_phys[0];
	src_prev_cbcr_data_phy	= _deinterlace->qbuf_temp[0].src_phys[1];

	src_curr_y_data_phy		=	_deinterlace->qbuf_temp[1].src_phys[0];
	src_curr_cbcr_data_phy	=	_deinterlace->qbuf_temp[1].src_phys[1];

	src_next_y_data_phy		=	_deinterlace->qbuf_temp[2].src_phys[0];
	src_next_cbcr_data_phy	=	_deinterlace->qbuf_temp[2].src_phys[1];

	width = _deinterlace->ctx.width;
	height = _deinterlace->ctx.height;

	src_y_stride = ALIGN(width, _deinterlace->ctx.src_stride_factor);
	src_c_stride = ALIGN(width, _deinterlace->ctx.src_stride_factor)/2;

	dst_y_stride = ALIGN(width, _deinterlace->ctx.dst_stride_factor);
	dst_c_stride = ALIGN(width, _deinterlace->ctx.dst_stride_factor)/2;

	dst_y_data_phy	=	dqbuf->dst_phys[0];
	dst_cbcr_data_phy	=	dqbuf->dst_phys[1];

	SetDeInterlaceCbCr(height, width,
                 		src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_y_stride, dst_y_data_phy, dst_y_stride,
                 		src_curr_cbcr_data_phy, src_y_stride, dst_cbcr_data_phy, dst_y_stride,
                 		atomic_read(&_deinterlace->even_or_odd));

	 atomic_set(&_deinterlace->even_or_odd, ((atomic_read(&_deinterlace->even_or_odd) == 0) ? 1 : 0));

	//dump_deinterlace_register(0);
	NX_DEINTERLACE_DeinterlaceStart();

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int processing_3plane(DQ_BUF *dqbuf)
{
	unsigned long src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy;
	unsigned long src_prev_cb_data_phy, src_curr_cb_data_phy, src_next_cb_data_phy;
	unsigned long src_prev_cr_data_phy, src_curr_cr_data_phy, src_next_cr_data_phy;

	unsigned long dst_y_data_phy, dst_cb_data_phy, dst_cr_data_phy;
	int width, height, src_y_stride, src_c_stride, dst_y_stride, dst_c_stride;

	tag_msg(KERN_INFO "%s++\n", __func__);

	if( get_queue_data(&_deinterlace->qbuf_temp[0], FRAME_PREV) < 0 ||
			get_queue_data(&_deinterlace->qbuf_temp[1], FRAME_CURR) < 0 ||
			get_queue_data(&_deinterlace->qbuf_temp[2], FRAME_NEXT) < 0 )
	{
		return -1;
	}

	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);

	src_prev_y_data_phy		= _deinterlace->qbuf_temp[0].src_phys[0];
	src_prev_cb_data_phy	= _deinterlace->qbuf_temp[0].src_phys[1];
	src_prev_cr_data_phy	= _deinterlace->qbuf_temp[0].src_phys[2];

	src_curr_y_data_phy		=	_deinterlace->qbuf_temp[1].src_phys[0];
	src_curr_cb_data_phy	=	_deinterlace->qbuf_temp[1].src_phys[1];
	src_curr_cr_data_phy	=	_deinterlace->qbuf_temp[1].src_phys[2];

	src_next_y_data_phy		=	_deinterlace->qbuf_temp[2].src_phys[0];
	src_next_cb_data_phy	=	_deinterlace->qbuf_temp[2].src_phys[1];
	src_next_cr_data_phy	=	_deinterlace->qbuf_temp[2].src_phys[2];

	width = _deinterlace->ctx.width;
	height = _deinterlace->ctx.height;

	src_y_stride = ALIGN(width, _deinterlace->ctx.src_stride_factor);
	src_c_stride = ALIGN(width, _deinterlace->ctx.src_stride_factor)/2;

	dst_y_stride = ALIGN(width, _deinterlace->ctx.dst_stride_factor);
	dst_c_stride = ALIGN(width, _deinterlace->ctx.dst_stride_factor)/2;

	dst_y_data_phy	=	dqbuf->dst_phys[0];
	dst_cb_data_phy	=	dqbuf->dst_phys[1];
	dst_cr_data_phy	=	dqbuf->dst_phys[2];

	SetDeInterlace(height, width,
                 src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_y_stride, dst_y_data_phy, dst_y_stride,
                 src_curr_cb_data_phy, src_c_stride, dst_cb_data_phy, dst_c_stride,
                 src_curr_cr_data_phy, src_c_stride, dst_cr_data_phy, dst_c_stride,
                 atomic_read(&_deinterlace->even_or_odd));

	 atomic_set(&_deinterlace->even_or_odd, ((atomic_read(&_deinterlace->even_or_odd) == 0) ? 1 : 0));

	//dump_deinterlace_register(0);
	NX_DEINTERLACE_DeinterlaceStart();

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int start_deinterlacing(nxp_deinterlace *m_ctx)
{
	int ret=0;
	DQ_BUF *dqbuf;
	DQ_BUF *dst_dqbuf;
	int i=0;

	tag_msg(KERN_INFO "%s++>\n", __func__);

	if(get_status() == PROCESSING_FINISH)
	{
		dst_dqbuf = _deinterlace->dst_dqbuf_all;

		dbg_msg(KERN_INFO "[%s] DQueue Buffer Count : %d\n", __func__, _deinterlace->dqset.buf_cnt);

		for( i=0; i<_deinterlace->dqset.buf_cnt; i++)
		{
			if( ((dst_dqbuf+i)->fence_fd != -1) && (dst_dqbuf+i)->fence)
			{
				dbg_msg(KERN_INFO "[%s] Release Fence FD %d : %d\n", __func__, i, (dst_dqbuf+i)->fence_fd);

				sys_close((dst_dqbuf+i)->fence_fd);
				(dst_dqbuf+i)->fence_fd = -1;
			}
		}

		return -1;
	}

	if(get_hw_status() == DEINTERLACING_START)
	{
		if( !_deinterlace->is_waitting )
			_deinterlace->is_waitting = true;

		ret = wait_event_interruptible(_deinterlace->wq_start, get_hw_status() == DEINTERLACING_STOP);
		if (ret == -ERESTARTSYS)
		{
			pr_err("deinterlace start interrupted\n");
			return -1;
		}

		if(get_status() == PROCESSING_FINISH)
		{
			_deinterlace->is_waitting = false;
		 return -1;
		}
	}

	set_hw_status(DEINTERLACING_START);

	dqbuf = _deinterlace->dst_dqbuf;
 	if( get_next_dstbuf(dqbuf) < 0 )
	{
		set_process_status(PROCESSING_PENDING, false);
		wake_up_interruptible(&_deinterlace->wq_dst);
		set_hw_status(DEINTERLACING_STOP);//CJK
		process_next_item(&_deinterlace->wq_start, &_deinterlace->is_waitting);

#if 0
		_deinterlace->is_waitting = false;
#endif

		printk(KERN_ERR "get next dstbuf error\n");
		return -1;
	}

  if( dqbuf->fence_fd != -1 )
  {
		pending_processing(dqbuf);
  }

	create_fence(dqbuf);
	tag_msg(KERN_INFO "[%s] : format_type : %d\n", __func__, m_ctx->ctx.format_type);

	switch(m_ctx->ctx.format_type)
	{
		case FORMAT_YV12: //3
			ret = processing_3plane(dqbuf);
			break;
		case FORMAT_NV21: //2
			ret = processing_2plane(dqbuf);
			break;
	}

	if( ret<0 )
	{
		printk(KERN_INFO "[%s] : start deinterlace processing ret : %d\n", __func__, ret);

		wake_up_interruptible(&_deinterlace->wq_dst);
		set_hw_status(DEINTERLACING_STOP);
		process_next_item(&_deinterlace->wq_start, &_deinterlace->is_waitting);

#if 0
		_deinterlace->is_waitting = false;
#endif

 		return ret;
	}

	ret = wait_event_interruptible(_deinterlace->wq_end, get_hw_status() == DEINTERLACING_STOP);
	if (ret == -ERESTARTSYS)
	{
		pr_err("deinterlace end interrupted\n");
		return -1;
	}

	if(get_status() == PROCESSING_FINISH)
	{
		_deinterlace->is_waitting = false;
		return -1;
	}

	if( insert_dequeue_frame() >= 0) //need mutex_lock for src_buf_count
	{
		wake_up_interruptible(&_deinterlace->wq_dst);
		sw_sync_timeline_inc(_deinterlace->q_timeline, 1);
	}

	process_next_item(&_deinterlace->wq_start, &_deinterlace->is_waitting);

#if 0
	if( _deinterlace->is_waitting )
	{
		wake_up_interruptible(&_deinterlace->wq_start);
		_deinterlace->is_waitting = false;
	}
#endif

	tag_msg(KERN_INFO "%s<--\n", __func__);

	return 0;
}

static int _onetime_test_2plane_direction_from_fd(frame_data_info *frame)
{
	unsigned long src_phys[SRC_BUFFER_COUNT * MAX_BUFFER_PLANES];
	unsigned long dst_phys[DST_BUFFER_COUNT * MAX_BUFFER_PLANES];

	unsigned long src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_curr_cbcr_data_phy;
	unsigned long dst_y_data_phy, dst_cbcr_data_phy;
	int width, height, src_y_stride, src_c_stride, dst_y_stride, dst_c_stride;

	int dst_y_data_size		=	0;
	int dst_cbcr_data_size	=	0;

	int i=0, j=0;
	int idx = 0;
	int ret = -1;

	for(i=0; i<SRC_BUFFER_COUNT ; i++)
	{
		for(j=0; j<frame->plane_mode ; j++)
		{
			get_phy_addr_from_fd(frame->src_bufs[i].plane2.fds[j], &src_phys[idx]);
			idx++;
		}
	}

	idx = 0;
	for(i=0; i<DST_BUFFER_COUNT ; i++)
	{
		for(j=0; j<frame->plane_mode ; j++)
		{
			get_phy_addr_from_fd(frame->dst_bufs[i].plane2.fds[j], &dst_phys[idx]);
			idx++;
		}
	}

	ret = check_work_duplicating(&_deinterlace->wq_start, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	width					= frame->width;
	height				=	frame->height;

	src_y_stride	=	frame->src_bufs[0].plane2.src_stride[0];
	src_c_stride	=	frame->src_bufs[0].plane2.src_stride[1];

	dst_y_stride	=	frame->dst_bufs[0].plane2.dst_stride[0];
	dst_c_stride	=	frame->dst_bufs[0].plane2.dst_stride[1];

	src_prev_y_data_phy		=	src_phys[0];
	src_curr_y_data_phy		=	src_phys[2];
	src_next_y_data_phy		=	src_phys[4];

	src_curr_cbcr_data_phy	=	src_phys[3];

	dst_y_data_phy	=	dst_phys[0];
	dst_cbcr_data_phy	=	dst_phys[1];

	dst_y_data_size = frame->dst_bufs[0].plane2.sizes[0];
	dst_cbcr_data_size = frame->dst_bufs[0].plane2.sizes[1];

#if 1
	SetDeInterlaceCbCr(height, width,
                 		src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_y_stride, dst_y_data_phy, dst_y_stride,
                 		src_curr_cbcr_data_phy, src_y_stride, dst_cbcr_data_phy, dst_y_stride,
                 		frame->src_bufs[1].frame_num % 2);
#else
	SetDeInterlace(SOURCE_HEIGHT, SOURCE_WIDTH,
                   0x7FFF0000, 0x7FFC0000, 0x7FF90000, SOURCE_Y_STRIDE, dst_y_data_phy, DEST_Y_STRIDE,
                   0x7FFB0000, SOURCE_C_STRIDE, dst_cb_data_phy, DEST_C_STRIDE,
                   0x7FFA0000, SOURCE_C_STRIDE, dst_cr_data_phy, DEST_C_STRIDE,
                   frame_num % 2);
#endif

	//dump_deinterlace_register(0);
	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);
	NX_DEINTERLACE_DeinterlaceStart();

	ret =	wait_for_end_of_work(&_deinterlace->wq_end, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[0], frame->dst_bufs[0].sizes[0]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[1], frame->dst_bufs[0].sizes[1]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[2], frame->dst_bufs[0].sizes[2]);

	return 0;
}

static int _onetime_test_3plane_direction_from_fd(frame_data_info *frame)
{
	unsigned long src_phys[SRC_BUFFER_COUNT * MAX_BUFFER_PLANES];
	unsigned long dst_phys[DST_BUFFER_COUNT * MAX_BUFFER_PLANES];

	unsigned long src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_curr_cb_data_phy, src_curr_cr_data_phy;
	unsigned long dst_y_data_phy, dst_cb_data_phy, dst_cr_data_phy;
	int width, height, src_y_stride, src_c_stride, dst_y_stride, dst_c_stride;

	int dst_y_data_size		=	0;
	int dst_cb_data_size	=	0;
	int dst_cr_data_size	=	0;

	int i=0, j=0;
	int idx = 0;
	int ret=-1;

	for(i=0; i<SRC_BUFFER_COUNT ; i++)
	{
		for(j=0 ; j<frame->plane_mode ; j++)
		{
			get_phy_addr_from_fd(frame->src_bufs[i].plane3.fds[j], &src_phys[idx]);
			idx++;
		}
	}

	idx = 0;
	for(i=0; i<DST_BUFFER_COUNT ; i++)
	{
		for(j=0; j<frame->plane_mode; j++)
		{
			get_phy_addr_from_fd(frame->dst_bufs[i].plane3.fds[j], &dst_phys[idx]);
			idx++;
		}
	}

	ret = check_work_duplicating(&_deinterlace->wq_start, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	width					= frame->width;
	height				=	frame->height;

	src_y_stride	=	frame->src_bufs[0].plane3.src_stride[0];
	src_c_stride	=	frame->src_bufs[0].plane3.src_stride[1];

	dst_y_stride	=	frame->dst_bufs[0].plane3.dst_stride[0];
	dst_c_stride	=	frame->dst_bufs[0].plane3.dst_stride[1];

	src_prev_y_data_phy		=	src_phys[0];
	src_curr_y_data_phy		=	src_phys[3];
	src_next_y_data_phy		=	src_phys[6];

	src_curr_cb_data_phy	=	src_phys[4];
	src_curr_cr_data_phy	=	src_phys[5];

	dst_y_data_phy	=	dst_phys[0];
	dst_cb_data_phy	=	dst_phys[1];
	dst_cr_data_phy	=	dst_phys[2];

	dst_y_data_size = frame->dst_bufs[0].plane3.sizes[0];
	dst_y_data_size = frame->dst_bufs[0].plane3.sizes[0];
	dst_cb_data_size = frame->dst_bufs[0].plane3.sizes[1];
	dst_cr_data_size = frame->dst_bufs[0].plane3.sizes[2];


#if 1
	SetDeInterlace(height, width,
                 src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_y_stride, dst_y_data_phy, dst_y_stride,
                 src_curr_cb_data_phy, src_c_stride, dst_cb_data_phy, dst_c_stride,
                 src_curr_cr_data_phy, src_c_stride, dst_cr_data_phy, dst_c_stride,
	               frame->src_bufs[1].frame_num % 2);

#else
	SetDeInterlace(SOURCE_HEIGHT, SOURCE_WIDTH,
                   0x7FFF0000, 0x7FFC0000, 0x7FF90000, SOURCE_Y_STRIDE, dst_y_data_phy, DEST_Y_STRIDE,
                   0x7FFB0000, SOURCE_C_STRIDE, dst_cb_data_phy, DEST_C_STRIDE,
                   0x7FFA0000, SOURCE_C_STRIDE, dst_cr_data_phy, DEST_C_STRIDE,
                   frame_num % 2);
#endif

	//dump_deinterlace_register(0);
	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);
	NX_DEINTERLACE_DeinterlaceStart();

	ret =	wait_for_end_of_work(&_deinterlace->wq_end, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[0], frame->dst_bufs[0].sizes[0]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[1], frame->dst_bufs[0].sizes[1]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[2], frame->dst_bufs[0].sizes[2]);

	return 0;
}

static int _onetime_test_2plane_direction(frame_data_info *frame)
{
	unsigned long src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_curr_cbcr_data_phy;
	unsigned long dst_y_data_phy, dst_cbcr_data_phy;
	int width, height, src_y_stride, src_c_stride, dst_y_stride, dst_c_stride;

	int dst_y_data_size		=	0;
	int dst_cbcr_data_size	=	0;
	int ret=-1;

	ret = check_work_duplicating(&_deinterlace->wq_start, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	width					= frame->width;
	height				=	frame->height;

	src_y_stride	=	frame->src_bufs[0].plane2.src_stride[0];
	src_c_stride	=	frame->src_bufs[0].plane2.src_stride[1];

	dst_y_stride	=	frame->dst_bufs[0].plane2.dst_stride[0];
	dst_c_stride	=	frame->dst_bufs[0].plane2.dst_stride[1];

	src_prev_y_data_phy		=	frame->src_bufs[0].plane2.phys[0];
	src_curr_y_data_phy		=	frame->src_bufs[1].plane2.phys[0];
	src_next_y_data_phy		=	frame->src_bufs[2].plane2.phys[0];

	src_curr_cbcr_data_phy	=	frame->src_bufs[1].plane2.phys[1];

	dst_y_data_phy	=	frame->dst_bufs[0].plane2.phys[0];
	dst_cbcr_data_phy	=	frame->dst_bufs[0].plane2.phys[1];

	dst_y_data_size = frame->dst_bufs[0].plane2.sizes[0];
	dst_cbcr_data_size = frame->dst_bufs[0].plane2.sizes[1];

	SetDeInterlaceCbCr(height, width,
                 		src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_y_stride, dst_y_data_phy, dst_y_stride,
                 		src_curr_cbcr_data_phy, src_y_stride, dst_cbcr_data_phy, dst_y_stride,
                 		frame->src_bufs[1].frame_num % 2);

	//dump_deinterlace_register(0);
	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);
	NX_DEINTERLACE_DeinterlaceStart();

	ret =	wait_for_end_of_work(&_deinterlace->wq_end, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[0], frame->dst_bufs[0].sizes[0]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[1], frame->dst_bufs[0].sizes[1]);

	return 0;
}

static int _onetime_test_3plane_direction(frame_data_info *frame)
{
	unsigned long src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_curr_cb_data_phy, src_curr_cr_data_phy;
	unsigned long dst_y_data_phy, dst_cb_data_phy, dst_cr_data_phy;
	int width, height, src_y_stride, src_c_stride, dst_y_stride, dst_c_stride;

	int dst_y_data_size		=	0;
	int dst_cb_data_size	=	0;
	int dst_cr_data_size	=	0;

	int ret=-1;

	ret = check_work_duplicating(&_deinterlace->wq_start, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	width					= frame->width;
	height				=	frame->height;

	src_y_stride	=	frame->src_bufs[0].plane3.src_stride[0];
	src_c_stride	=	frame->src_bufs[0].plane3.src_stride[1];

	dst_y_stride	=	frame->dst_bufs[0].plane3.dst_stride[0];
	dst_c_stride	=	frame->dst_bufs[0].plane3.dst_stride[1];

	src_prev_y_data_phy		=	frame->src_bufs[0].plane3.phys[0];
	src_curr_y_data_phy		=	frame->src_bufs[1].plane3.phys[0];
	src_next_y_data_phy		=	frame->src_bufs[2].plane3.phys[0];

	src_curr_cb_data_phy	=	frame->src_bufs[1].plane3.phys[1];
	src_curr_cr_data_phy	=	frame->src_bufs[1].plane3.phys[2];

	dst_y_data_phy	=	frame->dst_bufs[0].plane3.phys[0];
	dst_cb_data_phy	=	frame->dst_bufs[0].plane3.phys[1];
	dst_cr_data_phy	=	frame->dst_bufs[0].plane3.phys[2];

	dst_y_data_size = frame->dst_bufs[0].plane3.sizes[0];
	dst_cb_data_size = frame->dst_bufs[0].plane3.sizes[1];
	dst_cr_data_size = frame->dst_bufs[0].plane3.sizes[2];

#if 1
	SetDeInterlace(height, width,
                 src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, src_y_stride, dst_y_data_phy, dst_y_stride,
                 src_curr_cb_data_phy, src_c_stride, dst_cb_data_phy, dst_c_stride,
                 src_curr_cr_data_phy, src_c_stride, dst_cr_data_phy, dst_c_stride,
                 frame->src_bufs[1].frame_num % 2);

#else
	SetDeInterlace(SOURCE_HEIGHT, SOURCE_WIDTH,
                   0x7FFF0000, 0x7FFC0000, 0x7FF90000, SOURCE_Y_STRIDE, dst_y_data_phy, DEST_Y_STRIDE,
                   0x7FFB0000, SOURCE_C_STRIDE, dst_cb_data_phy, DEST_C_STRIDE,
                   0x7FFA0000, SOURCE_C_STRIDE, dst_cr_data_phy, DEST_C_STRIDE,
                   frame_num % 2);
#endif

	//dump_deinterlace_register(0);
	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);
	NX_DEINTERLACE_DeinterlaceStart();

	ret = wait_for_end_of_work(&_deinterlace->wq_end, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[0], frame->dst_bufs[0].sizes[0]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[1], frame->dst_bufs[0].sizes[1]);
	//write_file("/mnt/mmc/dst_y_data.bin", frame->dst_bufs[0].virt[2], frame->dst_bufs[0].sizes[2]);

	return 0;
}

static int _onetime_test_2plane(frame_data_info *frame)
{
	unsigned char *src_prev_y_data=NULL;
	unsigned char *src_curr_y_data=NULL;
	unsigned char *src_next_y_data=NULL;

	unsigned char *src_prev_cbcr_data=NULL;
	unsigned char *src_curr_cbcr_data=NULL;
	unsigned char *src_next_cbcr_data=NULL;

	unsigned char *dst_y_data		=NULL;
	unsigned char *dst_cbcr_data	=NULL;

	dma_addr_t  src_prev_y_data_phy;
	dma_addr_t	src_curr_y_data_phy;
	dma_addr_t	src_next_y_data_phy;

	dma_addr_t  src_prev_cbcr_data_phy;
	dma_addr_t	src_curr_cbcr_data_phy;
	dma_addr_t	src_next_cbcr_data_phy;

	dma_addr_t	dst_y_data_phy;
	dma_addr_t	dst_cbcr_data_phy;

#if 0
	unsigned char *dst_y_real_data	= NULL;
	unsigned char *dst_cbcr_real_data	= NULL;

	int dst_y_data_size		=	0;
	int dst_cbcr_data_size	=	0;
#endif

	int err=0;
	int ret=-1;

	ret = check_work_duplicating(&_deinterlace->wq_start, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	// Y
	if(alloc_app_data(&src_prev_y_data, &frame->src_bufs[0].plane2.virt[0], &src_prev_y_data_phy, frame->src_bufs[0].plane2.sizes[0]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_curr_y_data, &frame->src_bufs[1].plane2.virt[0], &src_curr_y_data_phy, frame->src_bufs[1].plane2.sizes[0]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_next_y_data, &frame->src_bufs[2].plane2.virt[0], &src_next_y_data_phy, frame->src_bufs[2].plane2.sizes[0]) < 0)
		return -ENOMEM;

	// CBCR
	if(alloc_app_data(&src_prev_cbcr_data, &frame->src_bufs[0].plane2.virt[1], &src_prev_cbcr_data_phy, frame->src_bufs[0].plane2.sizes[1]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_curr_cbcr_data, &frame->src_bufs[1].plane2.virt[1], &src_curr_cbcr_data_phy, frame->src_bufs[1].plane2.sizes[1]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_next_cbcr_data, &frame->src_bufs[2].plane2.virt[1], &src_next_cbcr_data_phy, frame->src_bufs[2].plane2.sizes[1]) < 0)
		return -ENOMEM;

	// DST
	if(alloc_app_data(&dst_y_data, &frame->dst_bufs[0].plane2.virt[0], &dst_y_data_phy, frame->dst_bufs[0].plane2.sizes[0]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&dst_cbcr_data, &frame->dst_bufs[0].plane2.virt[1], &dst_cbcr_data_phy, frame->dst_bufs[0].plane2.sizes[1]) < 0)
		return -ENOMEM;

	SetDeInterlaceCbCr(frame->height, frame->width,
                 		src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, frame->src_bufs[1].plane2.src_stride[0], dst_y_data_phy, frame->dst_bufs[0].plane2.dst_stride[0],
                 		src_curr_cbcr_data_phy, frame->src_bufs[1].plane2.src_stride[0], dst_cbcr_data_phy, frame->dst_bufs[0].plane2.dst_stride[0],
                 		frame->src_bufs[1].frame_num % 2);

#if 0
	SetDeInterlaceCbCr(frame->height, frame->width,
                 		src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, frame->src_y_stride, dst_y_data_phy, frame->dst_y_stride,
                 		src_curr_cbcr_data_phy, frame->src_y_stride, dst_cbcr_data_phy, frame->dst_c_stride,
                 		frame->frame_num % 2);
#endif

	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);
	NX_DEINTERLACE_DeinterlaceStart();
	//dump_deinterlace_register(0);

#if 0
  // printk("dma_addr_t: %#llx\n", (u64)addr);

  printk(KERN_INFO "PREV - Y Virt : 0x%p, Y Phys : 0x%lX\n", src_prev_y_data, (unsigned long)src_prev_y_data_phy);
  printk(KERN_INFO "CURR - Y Virt : 0x%p, Y Phys : 0x%lX\n", src_curr_y_data, (unsigned long)src_curr_y_data_phy);
  printk(KERN_INFO "NEXT - Y Virt : 0x%p, Y Phys : 0x%lX\n", src_next_y_data, (unsigned long)src_next_y_data_phy);
  printk(KERN_INFO "SRC Y Stride : %ld\n\n", frame->src_bufs[1].plane2.src_stride[0]);

  printk(KERN_INFO "PREV - CBCR Virt : 0x%p, CBCR Phys : 0x%lX\n", src_prev_cbcr_data, (unsigned long)src_prev_cbcr_data_phy);
  printk(KERN_INFO "CURR - CBCR Virt : 0x%p, CBCR Phys : 0x%lX\n", src_curr_cbcr_data, (unsigned long)src_curr_cbcr_data_phy);
  printk(KERN_INFO "NEXT - CBCR Virt : 0x%p, CBCR Phys : 0x%lX\n", src_next_cbcr_data, (unsigned long)src_next_cbcr_data_phy);
  printk(KERN_INFO "SRC CBCR Stride : %ld\n\n", frame->src_bufs[1].plane2.src_stride[1]);

  printk(KERN_INFO "DST - Y Virt : 0x%p, Y Phys : 0x%lX\n", dst_y_data, (unsigned long)dst_y_data_phy);
  printk(KERN_INFO "DST Y Stride : %ld\n\n", frame->dst_bufs[0].plane2.dst_stride[0]);

  printk(KERN_INFO "DST - CBCR Virt : 0x%p, CBCR Phys : 0x%lX\n", dst_cbcr_data, (unsigned long)dst_cbcr_data_phy);
  printk(KERN_INFO "DST CBCR Stride : %ld\n\n", frame->dst_bufs[0].plane2.dst_stride[1]);
#endif

	ret =  	wait_for_end_of_work(&_deinterlace->wq_end, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	//write_file("/mnt/mmc/dst_y_data_org.bin", dst_y_data, frame->dst_y_data_size);
	//write_file("/mnt/mmc/dst_cbcr_data_org.bin", dst_cbcr_data, frame->dst_cbcr_data_size);

#if 0
	alloc_dst_data(&dst_y_real_data, &dst_y_data, frame->dst_y_data_size, dst_y_data_size, frame->dst_y_stride, frame->src_y_stride);
	alloc_dst_data(&dst_cbcr_real_data, &dst_cbcr_data, frame->dst_cbcr_data_size, dst_cbcr_data_size, frame->dst_c_stride, frame->src_c_stride);
#endif

	//write_file("/mnt/mmc/dst_y_data.bin", dst_y_real_data, frame->dst_y_data_size);
	//write_file("/mnt/mmc/dst_cb_data.bin", dst_cb_real_data, frame->dst_cb_data_size);
	//write_file("/mnt/mmc/dst_cr_data.bin", dst_cr_real_data, frame->dst_cr_data_size);

	err = copy_to_user(frame->dst_bufs[0].plane2.virt[0], dst_y_data, frame->dst_bufs[0].plane2.sizes[0]);
	if(err < 0)
	{
		printk(KERN_ERR "dst Y data copy to user error!\n");
		return -EACCES;
	}

	err = copy_to_user(frame->dst_bufs[0].plane2.virt[1], dst_cbcr_data, frame->dst_bufs[0].plane2.sizes[1]);
	if(err < 0)
	{
		printk(KERN_ERR "dst cbcr data copy to user error!\n");
		return -EACCES;
	}

#if 0
	dealloc_dst_data(&dst_y_real_data);
	dealloc_dst_data(&dst_cbcr_real_data);
#endif

	dealloc_app_data(&src_prev_y_data, &src_prev_y_data_phy, frame->src_bufs[0].plane2.sizes[0]);
	dealloc_app_data(&src_curr_y_data, &src_curr_y_data_phy, frame->src_bufs[1].plane2.sizes[0]);
	dealloc_app_data(&src_next_y_data, &src_next_y_data_phy, frame->src_bufs[2].plane2.sizes[0]);

	dealloc_app_data(&src_prev_cbcr_data, &src_prev_cbcr_data_phy, frame->src_bufs[0].plane2.sizes[1]);
	dealloc_app_data(&src_curr_cbcr_data, &src_curr_cbcr_data_phy, frame->src_bufs[1].plane2.sizes[1]);
	dealloc_app_data(&src_next_cbcr_data, &src_next_cbcr_data_phy, frame->src_bufs[2].plane2.sizes[1]);

	dealloc_app_data(&dst_y_data, &dst_y_data_phy, frame->dst_bufs[0].plane2.sizes[0]);
	dealloc_app_data(&dst_cbcr_data, &dst_cbcr_data_phy, frame->dst_bufs[0].plane2.sizes[1]);

	return 0;
}

static int _onetime_test_3plane(frame_data_info *frame)
{
	unsigned char *src_prev_y_data	=	NULL;
	unsigned char *src_curr_y_data	=	NULL;
	unsigned char *src_next_y_data	=	NULL;

	unsigned char *src_prev_cb_data	=	NULL;
	unsigned char *src_curr_cb_data	=	NULL;
	unsigned char *src_next_cb_data	=	NULL;

	unsigned char *src_prev_cr_data	=	NULL;
	unsigned char *src_curr_cr_data	=	NULL;
	unsigned char *src_next_cr_data	=	NULL;

	unsigned char *dst_y_data		=	NULL;
	unsigned char *dst_cb_data	=	NULL;
	unsigned char *dst_cr_data	=	NULL;

	dma_addr_t  src_prev_y_data_phy;
	dma_addr_t	src_curr_y_data_phy;
	dma_addr_t	src_next_y_data_phy;

	dma_addr_t  src_prev_cb_data_phy;
	dma_addr_t	src_curr_cb_data_phy;
	dma_addr_t	src_next_cb_data_phy;

	dma_addr_t  src_prev_cr_data_phy;
	dma_addr_t	src_curr_cr_data_phy;
	dma_addr_t	src_next_cr_data_phy;

	dma_addr_t	dst_y_data_phy;
	dma_addr_t	dst_cb_data_phy;
	dma_addr_t	dst_cr_data_phy;

#if 0
	unsigned char *dst_y_real_data	= NULL;
	unsigned char *dst_cb_real_data	= NULL;
	unsigned char *dst_cr_real_data	= NULL;

	int dst_y_data_size		=	0;
	int dst_cb_data_size	=	0;
	int dst_cr_data_size	=	0;
#endif

	int err=0;
	int ret=-1;

	ret = check_work_duplicating(&_deinterlace->wq_start, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

	// Y
	if(alloc_app_data(&src_prev_y_data, &frame->src_bufs[0].plane3.virt[0], &src_prev_y_data_phy, frame->src_bufs[0].plane3.sizes[0]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_curr_y_data, &frame->src_bufs[1].plane3.virt[0], &src_curr_y_data_phy, frame->src_bufs[1].plane3.sizes[0]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_next_y_data, &frame->src_bufs[2].plane3.virt[0], &src_next_y_data_phy, frame->src_bufs[2].plane3.sizes[0]) < 0)
		return -ENOMEM;

	// CB
	if(alloc_app_data(&src_prev_cb_data, &frame->src_bufs[0].plane3.virt[1], &src_prev_cb_data_phy, frame->src_bufs[0].plane3.sizes[1]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_curr_cb_data, &frame->src_bufs[1].plane3.virt[1], &src_curr_cb_data_phy, frame->src_bufs[1].plane3.sizes[1]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_next_cb_data, &frame->src_bufs[2].plane3.virt[1], &src_next_cb_data_phy, frame->src_bufs[2].plane3.sizes[1]) < 0)
		return -ENOMEM;

	//	CR
	if(alloc_app_data(&src_prev_cr_data, &frame->src_bufs[0].plane3.virt[2], &src_prev_cr_data_phy, frame->src_bufs[0].plane3.sizes[2]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_curr_cr_data, &frame->src_bufs[1].plane3.virt[2], &src_curr_cr_data_phy, frame->src_bufs[1].plane3.sizes[2]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&src_next_cr_data, &frame->src_bufs[2].plane3.virt[2],  &src_next_cr_data_phy, frame->src_bufs[2].plane3.sizes[2]) < 0)
		return -ENOMEM;

	// DST
	if(alloc_app_data(&dst_y_data, &frame->dst_bufs[0].plane3.virt[0], &dst_y_data_phy, frame->dst_bufs[0].plane3.sizes[0]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&dst_cb_data, &frame->dst_bufs[0].plane3.virt[1], &dst_cb_data_phy, frame->dst_bufs[0].plane3.sizes[1]) < 0)
		return -ENOMEM;

	if(alloc_app_data(&dst_cr_data, &frame->dst_bufs[0].plane3.virt[2], &dst_cr_data_phy, frame->dst_bufs[0].plane3.sizes[2]) < 0)
		return -ENOMEM;

#if 0
	SetDeInterlace(SOURCE_HEIGHT, SOURCE_WIDTH,
                   0x7FFF0000, 0x7FFC0000, 0x7FF90000, SOURCE_Y_STRIDE, dst_y_data_phy, DEST_Y_STRIDE,
                   0x7FFB0000, SOURCE_C_STRIDE, dst_cb_data_phy, DEST_C_STRIDE,
                   0x7FFA0000, SOURCE_C_STRIDE, dst_cr_data_phy, DEST_C_STRIDE,
                   frame_num % 2);
#endif

#if 0
	// printk("dma_addr_t: %#llx\n", (u64)addr);

	printk(KERN_INFO "PREV - Y Virt : 0x%p, Y Phys : 0x%lX\n", src_prev_y_data, (unsigned long)src_prev_y_data_phy);
	printk(KERN_INFO "CURR - Y Virt : 0x%p, Y Phys : 0x%lX\n", src_curr_y_data, (unsigned long)src_curr_y_data_phy);
	printk(KERN_INFO "NEXT - Y Virt : 0x%p, Y Phys : 0x%lX\n", src_next_y_data, (unsigned long)src_next_y_data_phy);
	printk(KERN_INFO "SRC Y Stride : %ld\n\n", frame->src_bufs[1].plane3.src_stride[0]);
	printk(KERN_INFO "DST - Y Virt : 0x%p, Y Phys : 0x%lX\n", dst_y_data, (unsigned long)dst_y_data_phy);
	printk(KERN_INFO "DST Y Stride : %ld\n\n", frame->dst_bufs[0].plane3.dst_stride[0]);
#endif

	SetDeInterlace(frame->height, frame->width,
                 src_prev_y_data_phy, src_curr_y_data_phy, src_next_y_data_phy, frame->src_bufs[1].plane3.src_stride[0], dst_y_data_phy, frame->dst_bufs[0].plane3.dst_stride[0],
                 src_curr_cb_data_phy, frame->src_bufs[1].plane3.src_stride[1], dst_cb_data_phy, frame->dst_bufs[0].plane3.dst_stride[1],
                 src_curr_cr_data_phy, frame->src_bufs[1].plane3.src_stride[2], dst_cr_data_phy, frame->dst_bufs[0].plane3.dst_stride[2],
                 frame->src_bufs[1].frame_num % 2);

	NX_DEINTERLACE_SetInterruptEnable(0, CTRUE);
	NX_DEINTERLACE_DeinterlaceStart();
	//dump_deinterlace_register(0);

	ret = wait_for_end_of_work(&_deinterlace->wq_end, &_deinterlace->status, PROCESSING_START, PROCESSING_STOP, HZ/10);
	if( ret<0 ) return ret;

#if 0
	write_file("/mnt/mmc/p_src_prev_y_data.bin", src_prev_y_data, frame->src_bufs[0].plane3.sizes[0]);
	write_file("/mnt/mmc/p_src_curr_y_data.bin", src_curr_y_data, frame->src_bufs[1].plane3.sizes[0]);
	write_file("/mnt/mmc/p_src_next_y_data.bin", src_next_y_data, frame->src_bufs[2].plane3.sizes[0]);

	write_file("/mnt/mmc/src_prev_y_data.bin", frame->src_bufs[0].plane3.virt[0], frame->src_bufs[0].plane3.sizes[0]);
	write_file("/mnt/mmc/src_prev_cb_data.bin", frame->src_bufs[0].plane3.virt[1], frame->src_bufs[0].plane3.sizes[1]);
	write_file("/mnt/mmc/src_prev_cr_data.bin", frame->src_bufs[0].plane3.virt[2], frame->src_bufs[0].plane3.sizes[2]);

	write_file("/mnt/mmc/src_curr_y_data.bin", frame->src_bufs[1].plane3.virt[0], frame->src_bufs[1].plane3.sizes[0]);
	write_file("/mnt/mmc/src_curr_cb_data.bin", frame->src_bufs[1].plane3.virt[1], frame->src_bufs[1].plane3.sizes[1]);
	write_file("/mnt/mmc/src_curr_cr_data.bin", frame->src_bufs[1].plane3.virt[2], frame->src_bufs[1].plane3.sizes[2]);

	write_file("/mnt/mmc/src_next_y_data.bin", frame->src_bufs[2].plane3.virt[0], frame->src_bufs[2].plane3.sizes[0]);
	write_file("/mnt/mmc/src_next_cb_data.bin", frame->src_bufs[2].plane3.virt[1], frame->src_bufs[2].plane3.sizes[1]);
	write_file("/mnt/mmc/src_next_cr_data.bin", frame->src_bufs[2].plane3.virt[2], frame->src_bufs[2].plane3.sizes[2]);

	printk(KERN_INFO "DST END - Y Virt : 0x%p, Y Phys : 0x%lX, dst_y_data_size : %ld\n", dst_y_data, (unsigned long)dst_y_data_phy, frame->dst_bufs[0].plane3.sizes[0]);

	write_file("/mnt/mmc/dst_y_data.bin", dst_y_data, frame->dst_bufs[0].plane3.sizes[0]);
	write_file("/mnt/mmc/dst_cb_data.bin", dst_cb_data, frame->dst_bufs[0].plane3.sizes[1]);
	write_file("/mnt/mmc/dst_cr_data.bin", dst_cr_data, frame->dst_bufs[0].plane3.sizes[2]);
#endif


#if 0
	alloc_dst_data(&dst_y_real_data, &dst_y_data, frame->dst_bufs[0].plane3.sizes[0]);
	alloc_dst_data(&dst_cb_real_data, &dst_cb_data, frame->dst_bufs[0].plane3.sizes[1]);
	alloc_dst_data(&dst_cr_real_data, &dst_cr_data, frame->dst_bufs[0].plane3.sizes[2]);
#endif

	err = copy_to_user(frame->dst_bufs[0].plane3.virt[0], dst_y_data, frame->dst_bufs[0].plane3.sizes[0]);
	if(err < 0)
	{
		printk(KERN_ERR "dst Y data copy to user error!\n");
		return -EACCES;
	}

	err = copy_to_user(frame->dst_bufs[0].plane3.virt[1], dst_cb_data, frame->dst_bufs[0].plane3.sizes[1]);
	if(err < 0)
	{
		printk(KERN_ERR "dst cb data copy to user error!\n");
		return -EACCES;
	}

	err = copy_to_user(frame->dst_bufs[0].plane3.virt[2], dst_cr_data, frame->dst_bufs[0].plane3.sizes[2]);
	if(err < 0)
	{
		printk(KERN_ERR "dst cr data copy to user error!\n");
		return -EACCES;
	}

#if 0
	dealloc_dst_data(&dst_y_real_data);
	dealloc_dst_data(&dst_cb_real_data);
	dealloc_dst_data(&dst_cr_real_data);
#endif

	dealloc_app_data(&src_prev_y_data, &src_prev_y_data_phy, frame->src_bufs[0].plane3.sizes[0]);
	dealloc_app_data(&src_curr_y_data, &src_curr_y_data_phy, frame->src_bufs[1].plane3.sizes[0]);
	dealloc_app_data(&src_next_y_data, &src_next_y_data_phy, frame->src_bufs[2].plane3.sizes[0]);

	dealloc_app_data(&src_prev_cb_data, &src_prev_cb_data_phy, frame->src_bufs[0].plane3.sizes[1]);
	dealloc_app_data(&src_curr_cb_data, &src_curr_cb_data_phy, frame->src_bufs[1].plane3.sizes[1]);
	dealloc_app_data(&src_next_cb_data, &src_next_cb_data_phy, frame->src_bufs[2].plane3.sizes[1]);

	dealloc_app_data(&src_prev_cr_data, &src_prev_cr_data_phy, frame->src_bufs[0].plane3.sizes[2]);
	dealloc_app_data(&src_curr_cr_data, &src_curr_cr_data_phy, frame->src_bufs[1].plane3.sizes[2]);
	dealloc_app_data(&src_next_cr_data, &src_next_cr_data_phy, frame->src_bufs[2].plane3.sizes[2]);

	dealloc_app_data(&dst_y_data, &dst_y_data_phy, frame->dst_bufs[0].plane3.sizes[0]);
	dealloc_app_data(&dst_cb_data, &dst_cb_data_phy, frame->dst_bufs[0].plane3.sizes[1]);
	dealloc_app_data(&dst_cr_data, &dst_cr_data_phy, frame->dst_bufs[0].plane3.sizes[2]);

	return 0;
}

static int insert_dequeue_frame(void)
{
	int ret=0;

	tag_msg(KERN_INFO "%s++\n", __func__);

	ret = kfifo_in_spinlocked(&_deinterlace->data_dequeue, _deinterlace->dst_dqbuf, sizeof(DQ_BUF), &_deinterlace->dequeue_lock);
	if(ret != sizeof(DQ_BUF))
	{
		printk(KERN_ERR "dequeue kfifo in error\n");
		return -1;
	}

	atomic_dec(&_deinterlace->src_buf_count);

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int set_unused_dstuf(DQ_BUF *dqbuf)
{
	int ret=0;
	DQ_BUF dqbuf_t;

	tag_msg(KERN_INFO "%s++\n", __func__);

	memcpy(&dqbuf_t, dqbuf, sizeof(DQ_BUF));

	ret = kfifo_in_spinlocked(&_deinterlace->data_dequeue_unused, &dqbuf_t, sizeof(DQ_BUF), &_deinterlace->dequeue_unused_lock);
	if(ret != sizeof(DQ_BUF))
	{
		printk(KERN_ERR "set unused_detbuf kfifo in error : %d\n", ret);
		return -1;
	}

	if( is_dst_buf_full() )
	{
		queue_work(_deinterlace->wq_proc, &_deinterlace->w_proc);
	}

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static void work_processing(struct work_struct *work)
{
	nxp_deinterlace *ctx = container_of(work, nxp_deinterlace, w_proc);

	tag_msg(KERN_INFO "%s++\n", __func__);

	mutex_lock(&_deinterlace->mtx_proc);
	if(( get_count_data_queue() >= MINIMUM_INQUEUE_COUNT) || (get_status() == PROCESSING_FINISH))
		start_deinterlacing(ctx); //keun
	mutex_unlock(&_deinterlace->mtx_proc);

	tag_msg(KERN_INFO "%s--\n", __func__);
}

static irqreturn_t _irq_handler_deinterlace(int irq, void *param)
{
	unsigned long flags;
	int mode;

	nxp_deinterlace *me = (nxp_deinterlace *)param;

	tag_msg(KERN_INFO "%s++\n", __func__);
	printk(KERN_INFO "%s++\n", __func__);

	spin_lock_irqsave(&me->irq_lock, flags);
	NX_DEINTERLACE_ClearInterruptPendingAll();

	mode = act_mode;
	switch( mode )
	{
		case ACT_COPY:
		case ACT_DIRECT:
		case ACT_DIRECT_FD:
			atomic_set(&me->status, PROCESSING_STOP);
			wake_up_interruptible(&me->wq_end);

			printk(KERN_INFO "ACT MODE : %d\n", mode);
			break;
		case ACT_THREAD:
			set_hw_status(DEINTERLACING_STOP);
			wake_up_interruptible(&me->wq_end);

			if( get_count_data_queue() >= MINIMUM_INQUEUE_COUNT )
			{
				queue_work(me->wq_proc, &me->w_proc);
			}
			else
			{
				set_process_status(PROCESSING_READY, false);
			}
			break;
	}

	spin_unlock_irqrestore(&me->irq_lock, flags);

	printk(KERN_INFO "%s--\n", __func__);
	tag_msg(KERN_INFO "%s--\n", __func__);

	return IRQ_HANDLED;
}

static long deinterlace_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret=0;

	frame_data_info *frame_info;
//	frame_data_direct_info frame_direction_info;
	//frame_data_direct_info_from_fd frame_direction_info_from_fd;

	Q_BUF 	qbuf;

	DQ_BUF	dqbuf;
	DQ_BUF	dqbuf_t;

	printk(KERN_INFO "ioctl : %s\n", __func__);

	switch(cmd)
	{
		case IOCTL_DEINTERLACE_SET_AND_RUN:
		{
			act_mode	= ACT_COPY;

			frame_info = (frame_data_info *)kzalloc(sizeof(frame_data_info), GFP_KERNEL);

			if(copy_from_user(frame_info, (frame_data_info *)arg, sizeof(frame_data_info)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				return -EFAULT;
			}

			printk(KERN_INFO "command : %d\n", frame_info->command);

			enable_irq(_deinterlace->irq);
			switch( frame_info->command )
			{
			case ACT_COPY:
				switch( frame_info->plane_mode )
				{
				case PLANE3:
					 _onetime_test_3plane(frame_info);
					break;
				case PLANE2:
					 _onetime_test_2plane(frame_info);
					break;
				}
				break;
			case ACT_DIRECT:
				switch( frame_info->plane_mode )
				{
				case PLANE3:
					_onetime_test_3plane_direction(frame_info);
					break;
				case PLANE2:
					_onetime_test_2plane_direction(frame_info);
					break;
				}
				break;
			case ACT_DIRECT_FD:
				switch( frame_info->plane_mode )
				{
				case PLANE3:
					_onetime_test_3plane_direction_from_fd(frame_info);
					break;
				case PLANE2:
					_onetime_test_2plane_direction_from_fd(frame_info);
					break;
				}
				break;
			}
			disable_irq(_deinterlace->irq);

			if( frame_info )
			{
				kfree(frame_info);
				frame_info = NULL;
			}
		}
		break;
#if 0
		case IOCTL_DEINTERLACE_DIRECTION_SET_AND_RUN:
		{
			act_mode	= ACT_DIRECT;

			if(copy_from_user(&frame_direction_info, (frame_data_direct_info *)arg, sizeof(frame_data_direct_info)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				return -EFAULT;
			}

			switch( frame_direction_info.plane_count )
			{
				case PLANE3:
					_onetime_test_3plane_direction(&frame_direction_info);
					break;
				case PLANE2:
					_onetime_test_2plane_direction(&frame_direction_info);
					break;
			}
		}
		break;
		case IOCTL_DEINTERLACE_DIRECTION_FROM_FD_SET_AND_RUN:
		{
			act_mode	= ACT_DIRECT_FD;

			if(copy_from_user(&frame_direction_info_from_fd, (frame_data_direct_info_from_fd *)arg, sizeof(frame_data_direct_info_from_fd)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				return -EFAULT;
			}

			switch( frame_direction_info.plane_count )
			{
				case PLANE3:
					_onetime_test_3plane_direction_from_fd(&frame_direction_info_from_fd);
					break;
				case PLANE2:
					_onetime_test_2plane_direction_from_fd(&frame_direction_info_from_fd);
					break;
			}
		}
		break;
#endif
		case IOCTL_SET_CONTEXT:
			act_mode	= ACT_THREAD;

			if( mutex_lock_interruptible(&_deinterlace->read_lock))
				return -EINTR;
			if(copy_from_user(&_deinterlace->ctx, (CONTEXT *)arg, sizeof(CONTEXT)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				mutex_unlock(&_deinterlace->read_lock);
				return -EFAULT;
			}

			mutex_unlock(&_deinterlace->read_lock);
			dbg_msg(KERN_INFO "[%s]: format type : 0x%X\n", __func__, _deinterlace->ctx.format_type);
		break;
		case IOCTL_SET_DST_BUF:

			if( mutex_lock_interruptible(&_deinterlace->read_lock))
				return -EINTR;

			if(copy_from_user(&_deinterlace->dqset, (DQ_SET *)arg, sizeof(DQ_SET)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				mutex_unlock(&_deinterlace->read_lock);
				return -EFAULT;
			}
			mutex_unlock(&_deinterlace->read_lock);

			alloc_dst_buf(&_deinterlace->dqset);
		break;

		case IOCTL_QBUF:
			if( mutex_lock_interruptible(&_deinterlace->write_lock))
				return -EINTR;

			if(copy_from_user(&qbuf, (Q_BUF *)arg, sizeof(Q_BUF)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				mutex_unlock(&_deinterlace->write_lock);
				return -EFAULT;
			}
			mutex_unlock(&_deinterlace->write_lock);

			ret = check_exit_data(&qbuf);
			if(ret == -1)
			{
				printk(KERN_INFO "[%s]queue error......!!\n", __func__);
				return -1;
			}

			ret = _queue_frame_data(&qbuf);
			if(ret == -1)
			{
				printk(KERN_INFO "[%s]queue error......!!\n", __func__);
				return -1;
			}

			if(ret > 0)	return ret;
		break;
		case IOCTL_DQBUF:
			ret = _dequeue_frame_data(&dqbuf);

			if( ret < 0 )
			{
				dbg_msg(KERN_INFO "[%s]STOP Dequeue!\n", __func__);
				return ret;
			}
#if 0
	printk(KERN_INFO "[%s] phys y : 0x%lX, phys cb : 0x%lX, phys cr : 0x%lX\n",
										__func__, dqbuf.dst_phys[0], dqbuf.dst_phys[1], dqbuf.dst_phys[2]);
#endif
			dqbuf_t = dqbuf;

			if(mutex_lock_interruptible(&_deinterlace->read_lock))
				return -EINTR;

			if (copy_to_user((DQ_BUF *)arg, &dqbuf, sizeof(DQ_BUF)))
			{
				mutex_unlock(&_deinterlace->read_lock);
				return -EACCES;
			}

			mutex_unlock(&_deinterlace->read_lock);

			set_unused_dstuf(&dqbuf_t);
			break;
		case IOCTL_START:
			act_mode	= ACT_THREAD;

			if(copy_from_user(&_deinterlace->e_ctx, (EXE_CONTEXT *)arg, sizeof(EXE_CONTEXT)))
			{
				printk(KERN_ERR "%s: failed to copy_from_user()\n", __func__);
				return -EFAULT;
			}

			_deinterlace->dq_timeline = sw_sync_timeline_fdget(_deinterlace->e_ctx.dq_sync_timeline_fd);
			_deinterlace->q_timeline = sw_sync_timeline_fdget(_deinterlace->e_ctx.q_sync_timeline_fd);

			sw_start();
			break;
		case IOCTL_STOP:
			sw_stop();
			break;
		default:
			break;
	}

	return ret;
}

static int deinterlace_open(struct inode *inode, struct file *file)
{
	int ret;
	nxp_deinterlace *me = _deinterlace;
	file->private_data = me;

	tag_msg(KERN_INFO "%s++\n", __func__);

	if(atomic_read(&me->open_count) > 0)
	{
		atomic_inc(&me->open_count);
		printk("%s: oepn count %d\n", __func__, atomic_read(&me->open_count));
		return 0;
	}

	_initialize();
	set_irq();

	ret = request_irq(me->irq, _irq_handler_deinterlace, IRQF_DISABLED, DEVICE_NAME, me);
	if( ret<0 )
	{
		pr_err("%s: failed to request_irq()\n", __func__);
		return ret;
	}

	disable_irq_nosync(me->irq);

	if( kfifo_alloc(&me->data_queue, DATA_QUEUE_SIZE, GFP_KERNEL) )
	//if( kfifo_alloc(&me->data_queue, DATA_QUEUE_SIZE, GFP_ATOMIC) )
	{
		printk(KERN_WARNING "error kfifo_alloc \n");
		return -ENOMEM;
	}

	if( kfifo_alloc(&me->data_dequeue_unused, DST_QUEUE_SIZE, GFP_KERNEL) )
//	if( kfifo_alloc(&me->data_dequeue_unused, DST_QUEUE_SIZE, GFP_ATOMIC) )
	{
		printk(KERN_WARNING "error kfifo_alloc_unused \n");
		return -ENOMEM;
	}

	if( kfifo_alloc(&me->data_dequeue, DST_QUEUE_SIZE, GFP_KERNEL) )
//	if( kfifo_alloc(&me->data_dequeue, DST_QUEUE_SIZE, GFP_ATOMIC) )
	{
		printk(KERN_WARNING "error kfifo_alloc \n");
		return -ENOMEM;
	}

	if( kfifo_alloc(&me->fd_queue, QUEUE_SIZE, GFP_KERNEL) )
//	if( kfifo_alloc(&me->fd_queue, QUEUE_SIZE, GFP_ATOMIC) )
	{
		printk(KERN_WARNING "error fd_queue kfifo_alloc \n");
		return -ENOMEM;
	}

	me->dst_dqbuf = (DQ_BUF *)kzalloc(sizeof(DQ_BUF), GFP_KERNEL);
//	me->dst_dqbuf = (DQ_BUF *)kzalloc(sizeof(DQ_BUF), GFP_ATOMIC);
	if( !me->dst_dqbuf )
	{
		printk(KERN_ERR "[Error]dequeue kzalloc allocation error!!\n");
		return -ENOMEM;
	}

	//me->wq_proc = create_workqueue("wq_proc");
	me->wq_proc = create_singlethread_workqueue("wq_proc");
	//me->wq_proc = alloc_workqueue("wq_proc", WQ_CPU_INTENSIVE | WQ_HIGHPRI, 1);
	if( !me->wq_proc )
	{
		printk(KERN_ERR "[Error]create workqueue error!!\n");
		return -ENOMEM;
	}

	INIT_WORK(&me->w_proc, work_processing);

	mutex_init(&me->mtx_proc);
	mutex_init(&me->read_lock);
	mutex_init(&me->write_lock);

	seqlock_init(&_deinterlace->wq_lock);

	atomic_inc(&me->open_count);
	atomic_set(&me->even_or_odd, 0);

	atomic_set(&me->exit_flag, 0);

	_deinterlace->dq_timeline = NULL;
	_deinterlace->q_timeline = NULL;

	_deinterlace->q_timeline_max = 0;
	_deinterlace->dq_timeline_max = 0;

	init_state();

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static int deinterlace_close(struct inode *inode, struct file *file)
{
	nxp_deinterlace *me = (nxp_deinterlace *)file->private_data;

	tag_msg(KERN_INFO "%s++\n", __func__);

	atomic_dec(&me->open_count);

	if(atomic_read(&me->open_count) == 0)
	{
		free_irq(me->irq, me);
		_deinitialize();

		cancel_work_sync(&me->w_proc);
		flush_workqueue(me->wq_proc);
		destroy_workqueue(me->wq_proc);

		kfifo_free(&me->data_queue);
		kfifo_free(&me->data_dequeue);
		kfifo_free(&me->data_dequeue_unused);
		kfifo_free(&me->fd_queue);

		if( me->dst_dqbuf )
		{
			 kfree(me->dst_dqbuf);
			 me->dst_dqbuf = NULL;
		}

		dealloc_dst_buf();

		init_state();
	}

	tag_msg(KERN_INFO "%s--\n", __func__);

	return 0;
}

static struct file_operations deinterlace_fops =
{
	.open						=	deinterlace_open,
	.release				=	deinterlace_close,
	.unlocked_ioctl	=	deinterlace_ioctl
};

static struct miscdevice nxp_deinterlace_misc_device =
{
		.minor			=	MISC_DYNAMIC_MINOR,
		.name				=	DEVICE_NAME,
		.fops				=	&deinterlace_fops,
};

static int deinterlace_probe(struct platform_device *dev)
{
	nxp_deinterlace *me = NULL;

	me = (nxp_deinterlace *)kzalloc(sizeof(nxp_deinterlace), GFP_KERNEL);
//	me = (nxp_deinterlace *)kzalloc(sizeof(nxp_deinterlace), GFP_ATOMIC);

	if( !me )
	{
		pr_err("%s: failed to alloc nxp_deinterlace\n", __func__);
		return -1;
	}

	_deinterlace = me;

	init_waitqueue_head(&(_deinterlace->wq_start));
	init_waitqueue_head(&(_deinterlace->wq_end));

	init_waitqueue_head(&_deinterlace->wq_dst);

	spin_lock_init(&_deinterlace->irq_lock);
	spin_lock_init(&_deinterlace->queue_lock);
	spin_lock_init(&_deinterlace->dequeue_lock);
	spin_lock_init(&_deinterlace->queue_fd_list_lock);
	spin_lock_init(&_deinterlace->dequeue_unused_lock);

	spin_lock_init(&_deinterlace->process_lock);
	spin_lock_init(&_deinterlace->ion_lock);

	return 0;
}

static int deinterlace_remove(struct platform_device *dev)
{
	if( _deinterlace )
	{
		kfree(_deinterlace);
		_deinterlace = NULL;
	}

	return 0;
}

static struct platform_driver deinterlace_driver = {
	.probe = deinterlace_probe,
	.remove = deinterlace_remove,
	.driver	=	{
		.name	= DEVICE_NAME,
		.owner	=	THIS_MODULE,
	},
};

static struct platform_device deinterlace_device = {
	.name						=	DEVICE_NAME,
	.id							=	0,
	.num_resources	=	0,
};

static int __init deinterlace_init(void)
{
	int ret;

	ret = misc_register(&nxp_deinterlace_misc_device);
	if( ret )
	{
		printk(KERN_ERR "%s: failed to misc_register()\n", __func__);

		return -ENODEV;
	}

	ret = platform_driver_register(&deinterlace_driver);
	if( !ret )
	{
		ret = platform_device_register(&deinterlace_device);
		if( ret )
		{
			printk(KERN_ERR "platform driver register error : %d\n", ret);
			platform_driver_unregister(&deinterlace_driver);
		}
	}
	else
		printk(KERN_INFO DEVICE_NAME ": Error registering platform driver!\n");

	return ret;
}

static void __exit deinterlace_exit(void)
{
	platform_device_unregister(&deinterlace_device);
	platform_driver_unregister(&deinterlace_driver);
	misc_deregister(&nxp_deinterlace_misc_device);
}

module_init(deinterlace_init);
module_exit(deinterlace_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jong Keun Choi <jkchoi@nexell.co.kr>");
MODULE_DESCRIPTION("deinterlace Driver");
MODULE_VERSION("v0.1");
