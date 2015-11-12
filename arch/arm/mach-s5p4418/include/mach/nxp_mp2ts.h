#ifndef _NXP__TS_HEADER__
#define __NXP_TS_HEADER__


#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include "ioc_magic.h"

#define CFG_MPEGTS_IDMA_MODE	(0)


#define DMA_DOUBLE_BUFFER   (1)
#define MULTI_TS_DRV        (0)

#define NX_NO_PID           (0x1FFF)

#define NX_CAP_VALID_PID    (1 << 13)

#define NX_CSA_TYPE_PID     (1 << 15)
#define NX_AES_TYPE_PID     (0 << 15)

#define NX_AES_EVEN_PID     (0 << 14)
#define NX_AES_ODD_PID      (1 << 14)

#define NX_PARAM_TYPE_PID   (0)
#define NX_PARAM_TYPE_CAS   (1)

#define TS_PARAM_CAS_SIZE   (4 * 8) // 32byte

//#define TS_CAPx_PID_MAX     (128)   // Unit: 2byte, 128 fileld
#define TS_CAPx_PID_MAX     (64)    // Unit: 2byte, 128 fileld
#define TS_CORE_PID_MAX     (16)    // Unit: 4byte, 16 fileld

#define TS_PACKET_SIZE      (188)   //(CFG_MPEGTS_WORDCNT*4*16*8)
#if (CFG_MPEGTS_IDMA_MODE == 1)
#define TS_PACKET_NUM       (128)
#define TS_PAGE_NUM         (3)
#else
#define TS_PACKET_NUM       (80)	// Maximum 80
#define TS_PAGE_NUM         (36)
#endif
#define TS_PAGE_SIZE        (TS_PACKET_NUM * TS_PACKET_SIZE)
#define TS_BUF_TOTAL_SIZE   (TS_PAGE_NUM * TS_PAGE_SIZE)

/*------------------------------------------------------------------------------
 * MPEG TS IOCTL Define
 */
enum
{
    IOCTL_MPEGTS_RUN                =   _IO(IOC_NX_MAGIC, 1),
    IOCTL_MPEGTS_STOP               =   _IO(IOC_NX_MAGIC, 2),
    IOCTL_MPEGTS_READ_BUF           =   _IO(IOC_NX_MAGIC, 3),
    IOCTL_MPEGTS_WRITE_BUF          =   _IO(IOC_NX_MAGIC, 4),
    IOCTL_MPEGTS_READ_BUF_STATUS    =   _IO(IOC_NX_MAGIC, 5),
    IOCTL_MPEGTS_POWER_ON           =   _IO(IOC_NX_MAGIC, 6),
    IOCTL_MPEGTS_POWER_OFF          =   _IO(IOC_NX_MAGIC, 7),
    IOCTL_MPEGTS_DO_ALLOC           =   _IO(IOC_NX_MAGIC, 8),
    IOCTL_MPEGTS_DO_DEALLOC         =   _IO(IOC_NX_MAGIC, 9),
    IOCTL_MPEGTS_SET_CONFIG         =   _IO(IOC_NX_MAGIC, 10),
    IOCTL_MPEGTS_GET_CONFIG         =   _IO(IOC_NX_MAGIC, 11),
    IOCTL_MPEGTS_SET_PARAM          =   _IO(IOC_NX_MAGIC, 12),
    IOCTL_MPEGTS_GET_PARAM          =   _IO(IOC_NX_MAGIC, 13),
    IOCTL_MPEGTS_CLR_PARAM          =   _IO(IOC_NX_MAGIC, 14),
    IOCTL_MPEGTS_GET_LOCK_STATUS    =   _IO(IOC_NX_MAGIC, 15),
    IOCTL_MPEGTS_DECRY_TEST         =   _IO(IOC_NX_MAGIC, 16),
    IOCTL_MPEGTS_ENCRY_TEST         =   _IO(IOC_NX_MAGIC, 17),

};


/*------------------------------------------------------------------------------
 * MPEG TS Channel ID
 */
enum {
    NXP_MP2TS_ID_CAP0,
    NXP_MP2TS_ID_CAP1,
    NXP_MP2TS_ID_CORE,
    NXP_MP2TS_ID_MAX
};

enum {
    NXP_IDMA_CH0,
    NXP_IDMA_CH1,
    NXP_IDMA_SRC,
    NXP_IDMA_DST,
    NXP_IDMA_MAX
};

enum {
    NXP_MP2TS_PARAM_TYPE_PID,
//    NXP_MP2TS_PARAM_TYPE_AES,   /* Core only        */
//    NXP_MP2TS_PARAM_TYPE_CSA,   /* Core only        */
    NXP_MP2TS_PARAM_TYPE_CAS,   /* Core only        */
    NXP_MP2TS_PARAM_TYPE_BUF,   /* Read Buffer only */
};


/*------------------------------------------------------------------------------
 * MPEG TS Structs
 */
struct ts_op_mode {
    unsigned char   ch_num;
    unsigned char   tx_mode;                /* 0: rx mod, 1: tx mode    */
};

struct ts_config_descr {
    unsigned char       ch_num;
    union {
        unsigned char   data;
        struct {
        unsigned char   clock_pol   : 1;    /* Cpature 0, 1     */ /* 0: Invert, 1: Bypass */
        unsigned char   valid_pol   : 1;    /* Cpature 0, 1     */ /* 0: Active Low, 1: Active High */
        unsigned char   sync_pol    : 1;    /* Cpature 0, 1     */ /* 0: Active Low, 1: Active High */
        unsigned char   err_pol     : 1;    /* Cpature 0, 1     */ /* 0: Active Low, 1: Active High */
        unsigned char   data_width1 : 1;    /* Cpature 0, 1     */
        unsigned char   bypass_enb  : 1;    /* Cpature 0, 1     */
        unsigned char   xfer_mode   : 1;    /* Cpature 1,  Get status only  */
        unsigned char   xfer_clk_pol: 1;    /* Cpature 1        */
        unsigned char   encry_on    : 1;    /* Core             */
        } bits;
    } un;
};

struct ts_param_info {
    union {
        unsigned int    data;
        struct {
        unsigned int    index       : 7;    /* CSA type : (CSA_IDX << 1) | (0 or 1) */
        unsigned int    type        : 2;    /* 0 : PID,     1 : CAS    */
        unsigned int    ch_num      : 2;    /* 0 : Ch0,     1 : Ch1,    2 : Core    */
        unsigned int    reserved    : 21;
        } bits;
    } un;
};

struct ts_param_descr {
    struct ts_param_info    info;
    void                   *buf;
    int                     buf_size;
    int                     wait_time;      // Uint : 10ms
    int                     read_count;     // for debug
    int                     ret_value;
};

struct ts_buf_init_info {
    unsigned char   ch_num;
    unsigned int    packet_size;
    unsigned int    packet_num;
    unsigned int    page_size;
    unsigned int    page_num;
};

struct ts_ring_buf {
    unsigned int    captured_addr;
    unsigned int    cnt;
    unsigned int    wPos;
    unsigned int    rPos;
    unsigned char  *ts_packet[TS_PACKET_NUM];
    unsigned char  *ts_phy_packet[TS_PACKET_NUM];
};

struct ts_channel_info {
#if (CFG_MPEGTS_IDMA_MODE == 0)
    struct dma_chan     *dma_chan;	/* DMA param */
    struct dma_async_tx_descriptor *desc;
    void                *filter_data;
    dma_addr_t          peri_addr;
#endif

    int                 is_running;
    int                 is_malloced;
    int                 is_first;
    int                 do_continue;

    int                 wait_time;

    unsigned char      *buf;
    unsigned char       tx_mode;
    unsigned int        dma_virt;
    dma_addr_t          dma_phy;
    unsigned int        alloc_size;
    unsigned int        alloc_align;
    int                 cnt;
    int                 w_pos;
    int                 r_pos;
    int                 page_size;
    int                 page_num;

    wait_queue_head_t   wait; // read, write wait
};

struct ts_drv_context {
    struct device  *dev;
    void __iomem   *baseaddr;
    int             irq_dma;

    unsigned char           swich_ch;
    struct ts_channel_info  ch_info[4];
#if (MULTI_TS_DRV == 1)
    struct miscdevice       mp2ts_miscdev;
#endif

    int     is_opened;
    char    cap_ch_num;
    char    drv_name[20];
};

struct ts_packet_data {
    int size;
    int num;
};


/*------------------------------------------------------------------------------
 * Return values
 */
#define ETS_NOERROR     0
#define ETS_WRITEBUF    1   /* copy_to_user or copy_from_user   */
#define ETS_READBUF     2   /* memory alloc */
#define ETS_FAULT       3   /* copy_to_user or copy_from_user   */
#define ETS_ALLOC       4   /* memory alloc */
#define ETS_RUNNING     5
#define ETS_TYPE        6
#define ETS_TIMEOUT     7


#endif  //__NXP_TS_HEADER__

