/*#define DEBUG 0*/

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/v4l2-mediabus.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>

#include <media/media-entity.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-subdev.h>

#include <mach/nxp-v4l2-platformdata.h>
#include <mach/platform.h>

#include "nxp-video.h"
#include "nxp-vin-clipper.h"
#include "nxp-csi.h"
#include "nxp-decimator.h"

#include "nxp-v4l2.h"
#include "nxp-v4l2-common.h"

#include "nxp-capture.h"

/* for nexell specific prototype */
#ifdef CONFIG_ARCH_NXP4330_3200
#include <mach/nxp3200.h>
#else
#include <mach/platform.h>
#endif

/*
 * child bitmap
 */
enum {
    CAPTURE_CHILD_CLIPPER = (1 << 0),
    CAPTURE_CHILD_DECIMATOR = (1 << 1),
};

/*
 * sensor information
 */
#define MAX_SENSOR_NAME     32
static struct sensor_sysfs_info {
    int is_mipi;
    char name[MAX_SENSOR_NAME];
} _sensor_info[2] = {
    {
        .is_mipi = 0,
        .name = {0, },
    }, {
        .is_mipi = 0,
        .name = {0, },
    }
};

/**
 * hw function
 */
/* debugging */
#define DUMP_REGISTER 1
void dump_register(int module)
{
#if (DUMP_REGISTER)
#define DBGOUT(args...)  printk(args)
    NX_VIP_RegisterSet *pREG =
        (NX_VIP_RegisterSet*)NX_VIP_GetBaseAddress(module);

    DBGOUT("BASE ADDRESS: %p\n", pREG);
#if defined(CONFIG_ARCH_S5P4418)
    DBGOUT(" VIP_CONFIG     = 0x%04x\r\n", pREG->VIP_CONFIG);
    DBGOUT(" VIP_HVINT      = 0x%04x\r\n", pREG->VIP_HVINT);
    DBGOUT(" VIP_SYNCCTRL   = 0x%04x\r\n", pREG->VIP_SYNCCTRL);
    DBGOUT(" VIP_SYNCMON    = 0x%04x\r\n", pREG->VIP_SYNCMON);
    DBGOUT(" VIP_VBEGIN     = 0x%04x\r\n", pREG->VIP_VBEGIN);
    DBGOUT(" VIP_VEND       = 0x%04x\r\n", pREG->VIP_VEND);
    DBGOUT(" VIP_HBEGIN     = 0x%04x\r\n", pREG->VIP_HBEGIN);
    DBGOUT(" VIP_HEND       = 0x%04x\r\n", pREG->VIP_HEND);
    DBGOUT(" VIP_FIFOCTRL   = 0x%04x\r\n", pREG->VIP_FIFOCTRL);
    DBGOUT(" VIP_HCOUNT     = 0x%04x\r\n", pREG->VIP_HCOUNT);
    DBGOUT(" VIP_VCOUNT     = 0x%04x\r\n", pREG->VIP_VCOUNT);
    DBGOUT(" VIP_CDENB      = 0x%04x\r\n", pREG->VIP_CDENB);
    DBGOUT(" VIP_ODINT      = 0x%04x\r\n", pREG->VIP_ODINT);
    DBGOUT(" VIP_IMGWIDTH   = 0x%04x\r\n", pREG->VIP_IMGWIDTH);
    DBGOUT(" VIP_IMGHEIGHT  = 0x%04x\r\n", pREG->VIP_IMGHEIGHT);
    DBGOUT(" CLIP_LEFT      = 0x%04x\r\n", pREG->CLIP_LEFT);
    DBGOUT(" CLIP_RIGHT     = 0x%04x\r\n", pREG->CLIP_RIGHT);
    DBGOUT(" CLIP_TOP       = 0x%04x\r\n", pREG->CLIP_TOP);
    DBGOUT(" CLIP_BOTTOM    = 0x%04x\r\n", pREG->CLIP_BOTTOM);
    DBGOUT(" DECI_TARGETW   = 0x%04x\r\n", pREG->DECI_TARGETW);
    DBGOUT(" DECI_TARGETH   = 0x%04x\r\n", pREG->DECI_TARGETH);
    DBGOUT(" DECI_DELTAW    = 0x%04x\r\n", pREG->DECI_DELTAW);
    DBGOUT(" DECI_DELTAH    = 0x%04x\r\n", pREG->DECI_DELTAH);
    DBGOUT(" DECI_CLEARW    = 0x%04x\r\n", pREG->DECI_CLEARW);
    DBGOUT(" DECI_CLEARH    = 0x%04x\r\n", pREG->DECI_CLEARH);
    DBGOUT(" DECI_LUSEG     = 0x%04x\r\n", pREG->DECI_LUSEG);
    DBGOUT(" DECI_CRSEG     = 0x%04x\r\n", pREG->DECI_CRSEG);
    DBGOUT(" DECI_CBSEG     = 0x%04x\r\n", pREG->DECI_CBSEG);
    DBGOUT(" DECI_FORMAT    = 0x%04x\r\n", pREG->DECI_FORMAT);
    DBGOUT(" DECI_ROTFLIP   = 0x%04x\r\n", pREG->DECI_ROTFLIP);
    DBGOUT(" DECI_LULEFT    = 0x%04x\r\n", pREG->DECI_LULEFT);
    DBGOUT(" DECI_CRLEFT    = 0x%04x\r\n", pREG->DECI_CRLEFT);
    DBGOUT(" DECI_CBLEFT    = 0x%04x\r\n", pREG->DECI_CBLEFT);
    DBGOUT(" DECI_LURIGHT   = 0x%04x\r\n", pREG->DECI_LURIGHT);
    DBGOUT(" DECI_CRRIGHT   = 0x%04x\r\n", pREG->DECI_CRRIGHT);
    DBGOUT(" DECI_CBRIGHT   = 0x%04x\r\n", pREG->DECI_CBRIGHT);
    DBGOUT(" DECI_LUTOP     = 0x%04x\r\n", pREG->DECI_LUTOP);
    DBGOUT(" DECI_CRTOP     = 0x%04x\r\n", pREG->DECI_CRTOP);
    DBGOUT(" DECI_CBTOP     = 0x%04x\r\n", pREG->DECI_CBTOP);
    DBGOUT(" DECI_LUBOTTOM  = 0x%04x\r\n", pREG->DECI_LUBOTTOM);
    DBGOUT(" DECI_CRBOTTOM  = 0x%04x\r\n", pREG->DECI_CRBOTTOM);
    DBGOUT(" DECI_CBBOTTOM  = 0x%04x\r\n", pREG->DECI_CBBOTTOM);
    DBGOUT(" CLIP_LUSEG     = 0x%04x\r\n", pREG->CLIP_LUSEG);
    DBGOUT(" CLIP_CRSEG     = 0x%04x\r\n", pREG->CLIP_CRSEG);
    DBGOUT(" CLIP_CBSEG     = 0x%04x\r\n", pREG->CLIP_CBSEG);
    DBGOUT(" CLIP_FORMAT    = 0x%04x\r\n", pREG->CLIP_FORMAT);
    DBGOUT(" CLIP_ROTFLIP   = 0x%04x\r\n", pREG->CLIP_ROTFLIP);
    DBGOUT(" CLIP_LULEFT    = 0x%04x\r\n", pREG->CLIP_LULEFT);
    DBGOUT(" CLIP_CRLEFT    = 0x%04x\r\n", pREG->CLIP_CRLEFT);
    DBGOUT(" CLIP_CBLEFT    = 0x%04x\r\n", pREG->CLIP_CBLEFT);
    DBGOUT(" CLIP_LURIGHT   = 0x%04x\r\n", pREG->CLIP_LURIGHT);
    DBGOUT(" CLIP_CRRIGHT   = 0x%04x\r\n", pREG->CLIP_CRRIGHT);
    DBGOUT(" CLIP_CBRIGHT   = 0x%04x\r\n", pREG->CLIP_CBRIGHT);
    DBGOUT(" CLIP_LUTOP     = 0x%04x\r\n", pREG->CLIP_LUTOP);
    DBGOUT(" CLIP_CRTOP     = 0x%04x\r\n", pREG->CLIP_CRTOP);
    DBGOUT(" CLIP_CBTOP     = 0x%04x\r\n", pREG->CLIP_CBTOP);
    DBGOUT(" CLIP_LUBOTTOM  = 0x%04x\r\n", pREG->CLIP_LUBOTTOM);
    DBGOUT(" CLIP_CRBOTTOM  = 0x%04x\r\n", pREG->CLIP_CRBOTTOM);
    DBGOUT(" CLIP_CBBOTTOM  = 0x%04x\r\n", pREG->CLIP_CBBOTTOM);
    DBGOUT(" VIP_SCANMODE   = 0x%04x\r\n", pREG->VIP_SCANMODE);
    DBGOUT(" CLIP_YUYVENB   = 0x%04x\r\n", pREG->CLIP_YUYVENB);
    DBGOUT(" CLIP_BASEADDRH = 0x%04x\r\n", pREG->CLIP_BASEADDRH);
    DBGOUT(" CLIP_BASEADDRL = 0x%04x\r\n", pREG->CLIP_BASEADDRL);
    DBGOUT(" CLIP_STRIDEH   = 0x%04x\r\n", pREG->CLIP_STRIDEH);
    DBGOUT(" CLIP_STRIDEL   = 0x%04x\r\n", pREG->CLIP_STRIDEL);
    DBGOUT(" VIP_VIP1       = 0x%04x\r\n", pREG->VIP_VIP1);
    /* DBGOUT(" VIPCLKENB      = 0x%04x\r\n", pREG->VIPCLKENB); */
    /* DBGOUT(" VIPCLKGEN[0][0]= 0x%04x\r\n", pREG->VIPCLKGEN[0][0]); */
    /* DBGOUT(" VIPCLKGEN[0][1]= 0x%04x\r\n", pREG->VIPCLKGEN[0][1]); */
    /* DBGOUT(" VIPCLKGEN[1][0]= 0x%04x\r\n", pREG->VIPCLKGEN[1][0]); */
    /* DBGOUT(" VIPCLKGEN[1][1]= 0x%04x\r\n", pREG->VIPCLKGEN[1][1]); */
#elif defined(CONFIG_ARCH_S5P6818)
    DBGOUT(" VIP_CONFIG     = 0x%04x\r\n", pREG->VIP_CONFIG);
    DBGOUT(" VIP_HVINT      = 0x%04x\r\n", pREG->VIP_HVINT);
    DBGOUT(" VIP_SYNCCTRL   = 0x%04x\r\n", pREG->VIP_SYNCCTRL);
    DBGOUT(" VIP_SYNCMON    = 0x%04x\r\n", pREG->VIP_SYNCMON);
    DBGOUT(" VIP_VBEGIN     = 0x%04x\r\n", pREG->VIP_VBEGIN);
    DBGOUT(" VIP_VEND       = 0x%04x\r\n", pREG->VIP_VEND);
    DBGOUT(" VIP_HBEGIN     = 0x%04x\r\n", pREG->VIP_HBEGIN);
    DBGOUT(" VIP_HEND       = 0x%04x\r\n", pREG->VIP_HEND);
    DBGOUT(" VIP_FIFOCTRL   = 0x%04x\r\n", pREG->VIP_FIFOCTRL);
    DBGOUT(" VIP_HCOUNT     = 0x%04x\r\n", pREG->VIP_HCOUNT);
    DBGOUT(" VIP_VCOUNT     = 0x%04x\r\n", pREG->VIP_VCOUNT);
    DBGOUT(" VIP_PADCLK_SEL = 0x%04x\r\n", pREG->VIP_PADCLK_SEL);
    DBGOUT(" VIP_INFIFOCLR  = 0x%04x\r\n", pREG->VIP_INFIFOCLR);
    DBGOUT(" VIP_CDENB      = 0x%04x\r\n", pREG->VIP_CDENB);
    DBGOUT(" VIP_ODINT      = 0x%04x\r\n", pREG->VIP_ODINT);
    DBGOUT(" VIP_IMGWIDTH   = 0x%04x\r\n", pREG->VIP_IMGWIDTH);
    DBGOUT(" VIP_IMGHEIGHT  = 0x%04x\r\n", pREG->VIP_IMGHEIGHT);
    DBGOUT(" CLIP_LEFT      = 0x%04x\r\n", pREG->CLIP_LEFT);
    DBGOUT(" CLIP_RIGHT     = 0x%04x\r\n", pREG->CLIP_RIGHT);
    DBGOUT(" CLIP_TOP       = 0x%04x\r\n", pREG->CLIP_TOP);
    DBGOUT(" CLIP_BOTTOM    = 0x%04x\r\n", pREG->CLIP_BOTTOM);
    DBGOUT(" DECI_TARGETW   = 0x%04x\r\n", pREG->DECI_TARGETW);
    DBGOUT(" DECI_TARGETH   = 0x%04x\r\n", pREG->DECI_TARGETH);
    DBGOUT(" DECI_DELTAW    = 0x%04x\r\n", pREG->DECI_DELTAW);
    DBGOUT(" DECI_DELTAH    = 0x%04x\r\n", pREG->DECI_DELTAH);
    DBGOUT(" DECI_CLEARW    = 0x%04x\r\n", pREG->DECI_CLEARW);
    DBGOUT(" DECI_CLEARH    = 0x%04x\r\n", pREG->DECI_CLEARH);
    DBGOUT(" DECI_FORMAT    = 0x%04x\r\n", pREG->DECI_FORMAT);
    DBGOUT(" DECI_LUADDR    = 0x%04x\r\n", pREG->DECI_LUADDR);
    DBGOUT(" DECI_LUSTRIDE  = 0x%04x\r\n", pREG->DECI_LUSTRIDE);
    DBGOUT(" DECI_CRADDR    = 0x%04x\r\n", pREG->DECI_CRADDR);
    DBGOUT(" DECI_CRSTRIDE  = 0x%04x\r\n", pREG->DECI_CRSTRIDE);
    DBGOUT(" DECI_CBADDR    = 0x%04x\r\n", pREG->DECI_CBADDR);
    DBGOUT(" DECI_CBSTRIDE  = 0x%04x\r\n", pREG->DECI_CBSTRIDE);
    DBGOUT(" CLIP_FORMAT    = 0x%04x\r\n", pREG->CLIP_FORMAT);
    DBGOUT(" CLIP_LUADDR    = 0x%04x\r\n", pREG->CLIP_LUADDR);
    DBGOUT(" CLIP_LUSTRIDE  = 0x%04x\r\n", pREG->CLIP_LUSTRIDE);
    DBGOUT(" CLIP_CRADDR    = 0x%04x\r\n", pREG->CLIP_CRADDR);
    DBGOUT(" CLIP_CRSTRIDE  = 0x%04x\r\n", pREG->CLIP_CRSTRIDE);
    DBGOUT(" CLIP_CBADDR    = 0x%04x\r\n", pREG->CLIP_CBADDR);
    DBGOUT(" CLIP_CBSTRIDE  = 0x%04x\r\n", pREG->CLIP_CBSTRIDE);
    DBGOUT(" VIP_SCANMODE   = 0x%04x\r\n", pREG->VIP_SCANMODE);
    DBGOUT(" VIP_VIP1       = 0x%04x\r\n", pREG->VIP_VIP1);
#endif

#endif
}

static int _hw_get_irq_num(struct nxp_capture *me)
{
#if defined(CONFIG_ARCH_S5P4418)
    return NX_VIP_GetInterruptNumber(me->module);
#elif defined(CONFIG_ARCH_S5P6818)
    return NX_VIP_GetInterruptNumber(me->module) + 32;
#endif
}

#if defined(CONFIG_TURNAROUND_VIP_RESET)
static NX_VIP_RegisterSet s_reg_backup[3];
static void _backup_register(int module)
{
    NX_VIP_RegisterSet *pREG = (NX_VIP_RegisterSet*)NX_VIP_GetBaseAddress(module);
    NX_VIP_RegisterSet *pBackupReg = &s_reg_backup[module];
#if defined(CONFIG_ARCH_S5P4418)
    pBackupReg->VIP_CONFIG = pREG->VIP_CONFIG;
    pBackupReg->VIP_SYNCCTRL = pREG->VIP_SYNCCTRL;
    pBackupReg->VIP_VBEGIN = pREG->VIP_VBEGIN;
    pBackupReg->VIP_VEND = pREG->VIP_VEND;
    pBackupReg->VIP_HBEGIN = pREG->VIP_HBEGIN;
    pBackupReg->VIP_HEND = pREG->VIP_HEND;
    pBackupReg->VIP_FIFOCTRL = pREG->VIP_FIFOCTRL;
    pBackupReg->VIP_CDENB = pREG->VIP_CDENB;
    pBackupReg->VIP_IMGWIDTH = pREG->VIP_IMGWIDTH;
    pBackupReg->VIP_IMGHEIGHT = pREG->VIP_IMGHEIGHT;
    pBackupReg->CLIP_LEFT = pREG->CLIP_LEFT;
    pBackupReg->CLIP_RIGHT = pREG->CLIP_RIGHT;
    pBackupReg->CLIP_TOP = pREG->CLIP_TOP;
    pBackupReg->CLIP_BOTTOM = pREG->CLIP_BOTTOM;
    pBackupReg->DECI_TARGETW = pREG->DECI_TARGETW;
    pBackupReg->DECI_TARGETH = pREG->DECI_TARGETH;
    pBackupReg->DECI_DELTAW = pREG->DECI_DELTAW;
    pBackupReg->DECI_DELTAH = pREG->DECI_DELTAH;
    pBackupReg->DECI_CLEARW = pREG->DECI_CLEARW;
    pBackupReg->DECI_CLEARH = pREG->DECI_CLEARH;
    pBackupReg->DECI_LUSEG = pREG->DECI_LUSEG;
    pBackupReg->DECI_CRSEG = pREG->DECI_CRSEG;
    pBackupReg->DECI_CBSEG = pREG->DECI_CBSEG;
    pBackupReg->DECI_FORMAT = pREG->DECI_FORMAT;
    pBackupReg->DECI_ROTFLIP = pREG->DECI_ROTFLIP;
    pBackupReg->DECI_LULEFT = pREG->DECI_LULEFT;
    pBackupReg->DECI_CRLEFT = pREG->DECI_CRLEFT;
    pBackupReg->DECI_CBLEFT = pREG->DECI_CBLEFT;
    pBackupReg->DECI_LURIGHT = pREG->DECI_LURIGHT;
    pBackupReg->DECI_CRRIGHT = pREG->DECI_CRRIGHT;
    pBackupReg->DECI_CBRIGHT = pREG->DECI_CBRIGHT;
    pBackupReg->DECI_LUTOP = pREG->DECI_LUTOP;
    pBackupReg->DECI_CRTOP = pREG->DECI_CRTOP;
    pBackupReg->DECI_CBTOP = pREG->DECI_CBTOP;
    pBackupReg->DECI_LUBOTTOM = pREG->DECI_LUBOTTOM;
    pBackupReg->DECI_CRBOTTOM = pREG->DECI_CRBOTTOM;
    pBackupReg->DECI_CBBOTTOM = pREG->DECI_CBBOTTOM;
    pBackupReg->CLIP_LUSEG = pREG->DECI_LUSEG;
    pBackupReg->CLIP_CRSEG = pREG->CLIP_CRSEG;
    pBackupReg->CLIP_CBSEG = pREG->CLIP_CBSEG;
    pBackupReg->CLIP_FORMAT = pREG->CLIP_FORMAT;
    pBackupReg->CLIP_ROTFLIP = pREG->CLIP_ROTFLIP;
    pBackupReg->CLIP_LULEFT = pREG->CLIP_LULEFT;
    pBackupReg->CLIP_CRLEFT = pREG->CLIP_CRLEFT;
    pBackupReg->CLIP_CBLEFT = pREG->CLIP_CBLEFT;
    pBackupReg->CLIP_LURIGHT = pREG->CLIP_LURIGHT;
    pBackupReg->CLIP_CRRIGHT = pREG->CLIP_CRRIGHT;
    pBackupReg->CLIP_CBRIGHT = pREG->CLIP_CBRIGHT;
    pBackupReg->CLIP_LUTOP = pREG->CLIP_LUTOP;
    pBackupReg->CLIP_CRTOP = pREG->CLIP_CRTOP;
    pBackupReg->CLIP_CBTOP = pREG->CLIP_CBTOP;
    pBackupReg->CLIP_LUBOTTOM = pREG->CLIP_LUBOTTOM;
    pBackupReg->CLIP_CRBOTTOM = pREG->CLIP_CRBOTTOM;
    pBackupReg->CLIP_CBBOTTOM = pREG->CLIP_CBBOTTOM;
    pBackupReg->VIP_SCANMODE = pREG->VIP_SCANMODE;
    pBackupReg->CLIP_YUYVENB = pREG->CLIP_YUYVENB;
    pBackupReg->CLIP_BASEADDRH = pREG->CLIP_BASEADDRH;
    pBackupReg->CLIP_BASEADDRL = pREG->CLIP_BASEADDRL;
    pBackupReg->CLIP_STRIDEH = pREG->CLIP_STRIDEH;
    pBackupReg->CLIP_STRIDEL = pREG->CLIP_STRIDEL;
    pBackupReg->VIP_VIP1 = pREG->VIP_VIP1;
#elif defined(CONFIG_ARCH_S5P6818)
    pBackupReg->VIP_CONFIG = pREG->VIP_CONFIG;
    pBackupReg->VIP_SYNCCTRL = pREG->VIP_SYNCCTRL;
    pBackupReg->VIP_VBEGIN = pREG->VIP_VBEGIN;
    pBackupReg->VIP_VEND = pREG->VIP_VEND;
    pBackupReg->VIP_HBEGIN = pREG->VIP_HBEGIN;
    pBackupReg->VIP_HEND = pREG->VIP_HEND;
    pBackupReg->VIP_FIFOCTRL = pREG->VIP_FIFOCTRL;
    pBackupReg->VIP_PADCLK_SEL = pREG->VIP_PADCLK_SEL;
    pBackupReg->VIP_INFIFOCLR = pREG->VIP_INFIFOCLR;
    pBackupReg->VIP_CDENB = pREG->VIP_CDENB;
    pBackupReg->VIP_IMGWIDTH = pREG->VIP_IMGWIDTH;
    pBackupReg->VIP_IMGHEIGHT = pREG->VIP_IMGHEIGHT;
    pBackupReg->CLIP_LEFT = pREG->CLIP_LEFT;
    pBackupReg->CLIP_RIGHT = pREG->CLIP_RIGHT;
    pBackupReg->CLIP_TOP = pREG->CLIP_TOP;
    pBackupReg->CLIP_BOTTOM = pREG->CLIP_BOTTOM;
    pBackupReg->DECI_TARGETW = pREG->DECI_TARGETW;
    pBackupReg->DECI_TARGETH = pREG->DECI_TARGETH;
    pBackupReg->DECI_DELTAW = pREG->DECI_DELTAW;
    pBackupReg->DECI_DELTAH = pREG->DECI_DELTAH;
    pBackupReg->DECI_CLEARW = pREG->DECI_CLEARW;
    pBackupReg->DECI_CLEARH = pREG->DECI_CLEARH;
    pBackupReg->DECI_FORMAT = pREG->DECI_FORMAT;
    pBackupReg->DECI_LUADDR = pREG->DECI_LUADDR;
    pBackupReg->DECI_LUSTRIDE = pREG->DECI_LUSTRIDE;
    pBackupReg->DECI_CRADDR = pREG->DECI_CRADDR;
    pBackupReg->DECI_CRSTRIDE = pREG->DECI_CRSTRIDE;
    pBackupReg->DECI_CBADDR = pREG->DECI_CBADDR;
    pBackupReg->DECI_CBSTRIDE = pREG->DECI_CBSTRIDE;
    pBackupReg->CLIP_FORMAT = pREG->CLIP_FORMAT;
    pBackupReg->CLIP_LUADDR = pREG->CLIP_LUADDR;
    pBackupReg->CLIP_LUSTRIDE = pREG->CLIP_LUSTRIDE;
    pBackupReg->CLIP_CRADDR = pREG->CLIP_CRADDR;
    pBackupReg->CLIP_CRSTRIDE = pREG->CLIP_CRSTRIDE;
    pBackupReg->CLIP_CBADDR = pREG->CLIP_CBADDR;
    pBackupReg->CLIP_CBSTRIDE = pREG->CLIP_CBSTRIDE;
    pBackupReg->VIP_SCANMODE = pREG->VIP_SCANMODE;
    pBackupReg->VIP_VIP1 = pREG->VIP_VIP1;
#endif
}

static void _restore_register(int module)
{
    NX_VIP_RegisterSet *pREG = (NX_VIP_RegisterSet*)NX_VIP_GetBaseAddress(module);
    NX_VIP_RegisterSet *pBackupReg = &s_reg_backup[module];
#if defined(CONFIG_ARCH_S5P4418)
    pREG->VIP_SYNCCTRL = pBackupReg->VIP_SYNCCTRL;
    pREG->VIP_VBEGIN = pBackupReg->VIP_VBEGIN;
    pREG->VIP_VEND = pBackupReg->VIP_VEND;
    pREG->VIP_HBEGIN = pBackupReg->VIP_HBEGIN;
    pREG->VIP_HEND = pBackupReg->VIP_HEND;
    pREG->VIP_FIFOCTRL = pBackupReg->VIP_FIFOCTRL;
    pREG->VIP_IMGWIDTH = pBackupReg->VIP_IMGWIDTH;
    pREG->VIP_IMGHEIGHT = pBackupReg->VIP_IMGHEIGHT;
    pREG->CLIP_LEFT = pBackupReg->CLIP_LEFT;
    pREG->CLIP_RIGHT = pBackupReg->CLIP_RIGHT;
    pREG->CLIP_TOP = pBackupReg->CLIP_TOP;
    pREG->CLIP_BOTTOM = pBackupReg->CLIP_BOTTOM;
    pREG->DECI_TARGETW = pBackupReg->DECI_TARGETW;
    pREG->DECI_TARGETH = pBackupReg->DECI_TARGETH;
    pREG->DECI_DELTAW = pBackupReg->DECI_DELTAW;
    pREG->DECI_DELTAH = pBackupReg->DECI_DELTAH;
    pREG->DECI_CLEARW = pBackupReg->DECI_CLEARW;
    pREG->DECI_CLEARH = pBackupReg->DECI_CLEARH;
    pREG->DECI_LUSEG = pBackupReg->DECI_LUSEG;
    pREG->DECI_CRSEG = pBackupReg->DECI_CRSEG;
    pREG->DECI_CBSEG = pBackupReg->DECI_CBSEG;
    pREG->DECI_FORMAT = pBackupReg->DECI_FORMAT;
    pREG->DECI_ROTFLIP = pBackupReg->DECI_ROTFLIP;
    pREG->DECI_LULEFT = pBackupReg->DECI_LULEFT;
    pREG->DECI_CRLEFT = pBackupReg->DECI_CRLEFT;
    pREG->DECI_CBLEFT = pBackupReg->DECI_CBLEFT;
    pREG->DECI_LURIGHT = pBackupReg->DECI_LURIGHT;
    pREG->DECI_CRRIGHT = pBackupReg->DECI_CRRIGHT;
    pREG->DECI_CBRIGHT = pBackupReg->DECI_CBRIGHT;
    pREG->DECI_LUTOP = pBackupReg->DECI_LUTOP;
    pREG->DECI_CRTOP = pBackupReg->DECI_CRTOP;
    pREG->DECI_CBTOP = pBackupReg->DECI_CBTOP;
    pREG->DECI_LUBOTTOM = pBackupReg->DECI_LUBOTTOM;
    pREG->DECI_CRBOTTOM = pBackupReg->DECI_CRBOTTOM;
    pREG->DECI_CBBOTTOM = pBackupReg->DECI_CBBOTTOM;
    pREG->CLIP_LUSEG = pBackupReg->DECI_LUSEG;
    pREG->CLIP_CRSEG = pBackupReg->CLIP_CRSEG;
    pREG->CLIP_CBSEG = pBackupReg->CLIP_CBSEG;
    pREG->CLIP_FORMAT = pBackupReg->CLIP_FORMAT;
    pREG->CLIP_ROTFLIP = pBackupReg->CLIP_ROTFLIP;
    pREG->CLIP_LULEFT = pBackupReg->CLIP_LULEFT;
    pREG->CLIP_CRLEFT = pBackupReg->CLIP_CRLEFT;
    pREG->CLIP_CBLEFT = pBackupReg->CLIP_CBLEFT;
    pREG->CLIP_LURIGHT = pBackupReg->CLIP_LURIGHT;
    pREG->CLIP_CRRIGHT = pBackupReg->CLIP_CRRIGHT;
    pREG->CLIP_CBRIGHT = pBackupReg->CLIP_CBRIGHT;
    pREG->CLIP_LUTOP = pBackupReg->CLIP_LUTOP;
    pREG->CLIP_CRTOP = pBackupReg->CLIP_CRTOP;
    pREG->CLIP_CBTOP = pBackupReg->CLIP_CBTOP;
    pREG->CLIP_LUBOTTOM = pBackupReg->CLIP_LUBOTTOM;
    pREG->CLIP_CRBOTTOM = pBackupReg->CLIP_CRBOTTOM;
    pREG->CLIP_CBBOTTOM = pBackupReg->CLIP_CBBOTTOM;
    pREG->VIP_SCANMODE = pBackupReg->VIP_SCANMODE;
    pREG->CLIP_YUYVENB = pBackupReg->CLIP_YUYVENB;
    pREG->CLIP_BASEADDRH = pBackupReg->CLIP_BASEADDRH;
    pREG->CLIP_BASEADDRL = pBackupReg->CLIP_BASEADDRL;
    pREG->CLIP_STRIDEH = pBackupReg->CLIP_STRIDEH;
    pREG->CLIP_STRIDEL = pBackupReg->CLIP_STRIDEL;
    pREG->VIP_CDENB = pBackupReg->VIP_CDENB;
    pREG->VIP_CONFIG = pBackupReg->VIP_CONFIG;
#elif defined(CONFIG_ARCH_S5P6818)
    pREG->VIP_SYNCCTRL = pBackupReg->VIP_SYNCCTRL;
    pREG->VIP_VBEGIN = pBackupReg->VIP_VBEGIN;
    pREG->VIP_VEND = pBackupReg->VIP_VEND;
    pREG->VIP_HBEGIN = pBackupReg->VIP_HBEGIN;
    pREG->VIP_HEND = pBackupReg->VIP_HEND;
    pREG->VIP_FIFOCTRL = pBackupReg->VIP_FIFOCTRL;
    pREG->VIP_PADCLK_SEL = pBackupReg->VIP_PADCLK_SEL;
    pREG->VIP_INFIFOCLR = pBackupReg->VIP_INFIFOCLR;
    pREG->VIP_IMGWIDTH = pBackupReg->VIP_IMGWIDTH;
    pREG->VIP_IMGHEIGHT = pBackupReg->VIP_IMGHEIGHT;
    pREG->CLIP_LEFT = pBackupReg->CLIP_LEFT;
    pREG->CLIP_RIGHT = pBackupReg->CLIP_RIGHT;
    pREG->CLIP_TOP = pBackupReg->CLIP_TOP;
    pREG->CLIP_BOTTOM = pBackupReg->CLIP_BOTTOM;
    pREG->DECI_TARGETW = pBackupReg->DECI_TARGETW;
    pREG->DECI_TARGETH = pBackupReg->DECI_TARGETH;
    pREG->DECI_DELTAW = pBackupReg->DECI_DELTAW;
    pREG->DECI_DELTAH = pBackupReg->DECI_DELTAH;
    pREG->DECI_CLEARW = pBackupReg->DECI_CLEARW;
    pREG->DECI_CLEARH = pBackupReg->DECI_CLEARH;
    pREG->DECI_FORMAT = pBackupReg->DECI_FORMAT;
    pREG->DECI_LUADDR = pBackupReg->DECI_LUADDR;
    pREG->DECI_LUSTRIDE = pBackupReg->DECI_LUSTRIDE;
    pREG->DECI_CRADDR = pBackupReg->DECI_CRADDR;
    pREG->DECI_CRSTRIDE = pBackupReg->DECI_CRSTRIDE;
    pREG->DECI_CBADDR = pBackupReg->DECI_CBADDR;
    pREG->DECI_CBSTRIDE = pBackupReg->DECI_CBSTRIDE;
    pREG->CLIP_FORMAT = pBackupReg->CLIP_FORMAT;
    pREG->CLIP_LUADDR = pBackupReg->CLIP_LUADDR;
    pREG->CLIP_LUSTRIDE = pBackupReg->CLIP_LUSTRIDE;
    pREG->CLIP_CRADDR = pBackupReg->CLIP_CRADDR;
    pREG->CLIP_CRSTRIDE = pBackupReg->CLIP_CRSTRIDE;
    pREG->CLIP_CBADDR = pBackupReg->CLIP_CBADDR;
    pREG->CLIP_CBSTRIDE = pBackupReg->CLIP_CBSTRIDE;
    pREG->VIP_SCANMODE = pBackupReg->VIP_SCANMODE;
    pREG->VIP_VIP1 = pBackupReg->VIP_VIP1;
    pREG->VIP_CDENB = pBackupReg->VIP_CDENB;
    pREG->VIP_CONFIG = pBackupReg->VIP_CONFIG;
#endif
}

static void _backup_reset_restore_register(int module)
{
  //  printk("%s!\n", __func__);
    _backup_register(module);
    NX_VIP_SetVIPEnable(module, CFALSE, CFALSE, CFALSE, CFALSE);
    NX_VIP_ResetFIFO(module);
#if 0
    mdelay(100);
#endif
#if defined(CONFIG_ARCH_S5P4418)
    NX_RSTCON_SetnRST(NX_VIP_GetResetNumber(module), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_VIP_GetResetNumber(module), RSTCON_nENABLE);
#elif defined(CONFIG_ARCH_S5P6818)
    NX_RSTCON_SetRST(NX_VIP_GetResetNumber(module), RSTCON_ASSERT);
    NX_RSTCON_SetRST(NX_VIP_GetResetNumber(module), RSTCON_NEGATE);
#endif
    _restore_register(module);
}

#endif

static void _hw_child_enable(struct nxp_capture *me, u32 child, bool on)
{
    CBOOL clip_enable = CFALSE;
    CBOOL deci_enable = CFALSE;


    if (child & CAPTURE_CHILD_CLIPPER) {
        clip_enable = CTRUE;
    }
    if (child & CAPTURE_CHILD_DECIMATOR) {
        /* clip_enable = CTRUE; */
        deci_enable = CTRUE;
    }

    if (me->clip_enable != clip_enable || me->deci_enable != deci_enable) {
        vmsg("%s: module %d changed clip %d-->%d, deci %d-->%d\n",
                __func__, me->module, me->clip_enable, clip_enable, me->deci_enable, deci_enable);
        if (clip_enable || deci_enable) {
            NX_VIP_SetInterruptEnableAll(me->module, CFALSE);
            NX_VIP_ClearInterruptPendingAll(me->module);
#ifdef CONFIG_TURNAROUND_VIP_RESET
            if (me->deci_enable == false && me->clip_enable == false ) {
                vmsg("%s: RESET & restore\n", __func__);
                _backup_reset_restore_register(me->module);
            }
#endif
            NX_VIP_SetVIPEnable(me->module, CTRUE, CTRUE, clip_enable, deci_enable);
            NX_VIP_SetInterruptEnable(me->module, CAPTURE_CLIPPER_INT, CTRUE);
        } else {
            NX_VIP_SetVIPEnable(me->module, CFALSE, CFALSE, CFALSE, CFALSE);
        }
        /*dump_register(me->module);*/

        me->clip_enable = clip_enable;
        me->deci_enable = deci_enable;

        vmsg("%s: exit\n", __func__);
    }

}

static void _hw_run(struct nxp_capture *me, bool on)
{
    if (on) {
        /* vmsg("%s: capture child 0x%x\n", __func__, me->running_child_bitmap); */
        vmsg("vip %d run!!!\n", me->module);
        NX_VIP_SetInterruptEnableAll(me->module, CFALSE);
        NX_VIP_ClearInterruptPendingAll(me->module);
        NX_VIP_SetInterruptEnable(me->module, CAPTURE_CLIPPER_INT, CTRUE);
        /* NX_VIP_SetInterruptEnable(me->module, NX_VIP_INT_VSYNC, CTRUE); */
        if (NXP_ATOMIC_READ(&me->running_child_bitmap) & CAPTURE_CHILD_DECIMATOR) {
            if (NXP_ATOMIC_READ(&me->running_child_bitmap) & CAPTURE_CHILD_CLIPPER)
                NX_VIP_SetVIPEnable(me->module, CTRUE, CTRUE, CTRUE, CTRUE);
            else
                NX_VIP_SetVIPEnable(me->module, CTRUE, CTRUE, CFALSE, CTRUE);
        } else {
            NX_VIP_SetVIPEnable(me->module, CTRUE, CTRUE, CTRUE, CFALSE);
        }
        /* dump_register(0); */
    } else {
        NX_VIP_SetVIPEnable(me->module, CFALSE, CFALSE, CFALSE, CFALSE);
        NX_VIP_SetInterruptEnableAll(me->module, CFALSE);
        NX_VIP_ClearInterruptPendingAll(me->module);
        vmsg("vip %d stopped!!!\n", me->module);
    }
}

/**
 * irq handler
 */
static irqreturn_t _irq_handler(int irq, void *desc)
{
    struct nxp_capture *me = desc;
    struct nxp_v4l2_irq_entry *e;
    unsigned long flags;
    int handle_count = 0;

    NX_VIP_ClearInterruptPendingAll(me->module);

    spin_lock_irqsave(&me->lock, flags);

    if (!list_empty(&me->irq_entry_list))
        list_for_each_entry(e, &me->irq_entry_list, entry) {
            e->handler(e->priv);
            handle_count++;
            if (handle_count >= me->irq_entry_count)
                goto OUT;
        }

OUT:
    spin_unlock_irqrestore(&me->lock, flags);

    return IRQ_HANDLED;
}

/**
 * callback functions
 */
static struct media_device *_get_media_device(struct nxp_capture *me)
{
    return nxp_v4l2_get_media_device();
}

static struct v4l2_device *_get_v4l2_device(struct nxp_capture *me)
{
    return nxp_v4l2_get_v4l2_device();
}

static int _get_module_num(struct nxp_capture *me)
{
    return me->module;
}

static int _get_index_num(struct nxp_capture *me)
{
    return me->index;
}

static void *_get_alloc_ctx(struct nxp_capture *me)
{
    return nxp_v4l2_get_alloc_ctx();
}

static int _run(struct nxp_capture *me, void *child)
{
    vmsg("%s: %p\n", __func__, child);
    if (&me->vin_clipper == child)
        NXP_ATOMIC_SET_MASK(CAPTURE_CHILD_CLIPPER, &me->running_child_bitmap);
    else if (&me->decimator == child)
        NXP_ATOMIC_SET_MASK(CAPTURE_CHILD_DECIMATOR, &me->running_child_bitmap);
    else
        return 0;
    _hw_child_enable(me, NXP_ATOMIC_READ(&me->running_child_bitmap), true);

    return 0;
}

static void _stop(struct nxp_capture *me, void *child)
{
    if (&me->vin_clipper == child)
        NXP_ATOMIC_CLEAR_MASK(CAPTURE_CHILD_CLIPPER, &me->running_child_bitmap);
    else if (&me->decimator == child)
        NXP_ATOMIC_CLEAR_MASK(CAPTURE_CHILD_DECIMATOR, &me->running_child_bitmap);
    else
        return;
    _hw_child_enable(me, NXP_ATOMIC_READ(&me->running_child_bitmap), false);
}

static int _register_irq_entry(struct nxp_capture *me,
        struct nxp_v4l2_irq_entry *entry)
{
    unsigned long flags;

    spin_lock_irqsave(&me->lock, flags);
    if (me->irq_entry_count >= 2) {
        printk("%s: INVALID CALL!!!\n", __func__);
        WARN_ON(1);
        spin_unlock_irqrestore(&me->lock, flags);
        return 0;
    }
    list_add_tail(&entry->entry, &me->irq_entry_list);
    me->irq_entry_count++;
    vmsg("%s: irq_entry %d\n", __func__, me->irq_entry_count);
    spin_unlock_irqrestore(&me->lock, flags);

    return 0;
}

/* must be called irq context, and get lock */
static void _unregister_irq_entry(struct nxp_capture *me,
        struct nxp_v4l2_irq_entry *entry)
{
    if (me->irq_entry_count <= 0) {
        printk("%s: INVALID CALL!!!\n", __func__);
        WARN_ON(1);
        return;
    }
    list_del(&entry->entry);
    me->irq_entry_count--;
    vmsg("%s: irq_entry %d\n", __func__, me->irq_entry_count);
}

static struct v4l2_subdev *
_get_csi_subdev(struct nxp_capture *me)
{
    return &me->csi.subdev;
}

static struct v4l2_subdev *
_get_sensor_subdev(struct nxp_capture *me)
{
    return me->sensor;
}

/**
 * helper functions
 */
static void _set_sensor_entity_name(int module, char *type, int i2c_adapter_num, int i2c_addr)
{
    char *p_entity_name;

    if (module == 0)
        p_entity_name = _sensor_info[0].name;
    else if (module == 1)
        p_entity_name = _sensor_info[1].name;
    else
        return;

    sprintf(p_entity_name, "%s %d-00%x", type, i2c_adapter_num, i2c_addr);
    vmsg("%s: module %d, sensor name %s\n", __func__, module, p_entity_name);
}

static void set_sensor_entity_name_without_i2c(int module)
{
    char *p_entity_name;

    if (module == 0)
        p_entity_name = _sensor_info[0].name;
    else if (module == 1)
        p_entity_name = _sensor_info[1].name;
    else
        return;

    sprintf(p_entity_name, "%s", "loopback-sensor");
    vmsg("%s: module %d, sensor name %s\n", __func__, module, p_entity_name);
}

static void _set_sensor_mipi_info(int module, int is_mipi)
{
    if (module == 0 || module == 1)
        _sensor_info[module].is_mipi = is_mipi;
}

static struct v4l2_subdev *external_sensor = NULL;
void nxp_v4l2_capture_set_sensor_subdev(struct v4l2_subdev *sd)
{
    external_sensor = sd;
}
/*EXPORT_SYMBOL(nxp_v4l2_capture_set_sensor_subdev);*/

static struct v4l2_subdev *_register_sensor(struct nxp_capture *me,
        struct nxp_v4l2_i2c_board_info *board_info)
{
    struct v4l2_subdev *sensor = NULL;
    struct i2c_adapter *adapter;
    struct media_entity *input;
    u32 pad;
    u32 flags;
    int ret;

    static int sensor_index = 0;

	
    if (board_info && board_info->board_info) {
        adapter = i2c_get_adapter(board_info->i2c_adapter_id);
        if (!adapter) {
            pr_err("%s: unable to get i2c adapter %d for device %s\n",
                    __func__,
                    board_info->i2c_adapter_id,
                    board_info->board_info->type);
            return NULL;
        }

        sensor = v4l2_i2c_new_subdev_board(me->get_v4l2_device(me),
                adapter, board_info->board_info, NULL);
        if (!sensor) {
            pr_err("%s: unable to register subdev %s\n",
                    __func__, board_info->board_info->type);
            return NULL;
        }
    } else {
        if (external_sensor)
            sensor = external_sensor;
        else {
             pr_err("%s: external sensor is NULL!!!\n", __func__);
             return NULL;
        }

    }

    sensor->host_priv = board_info;

    if (me->interface_type == NXP_CAPTURE_INF_CSI) {
        input = &me->csi.subdev.entity;
        pad = NXP_CSI_PAD_SINK;
        flags = 0;
    } else {
        input = &me->vin_clipper.subdev.entity;
        pad = NXP_VIN_PAD_SINK;
        flags = 0;
    }

    ret = media_entity_create_link(&sensor->entity, 0,
            input, pad, flags);
    if (ret < 0) {
        pr_err("%s: failed to media_entity_create_link()\n", __func__);
        return NULL;
    }

	if (board_info && board_info->board_info) {
   	/* _set_sensor_entity_name(me->module, board_info->board_info->type, board_info->i2c_adapter_id, board_info->board_info->addr); */
   	_set_sensor_entity_name(sensor_index, board_info->board_info->type, board_info->i2c_adapter_id, board_info->board_info->addr);
	} else {
    //	_set_sensor_entity_name(sensor_index, "loopback-sensor", 0, 0x00);
		set_sensor_entity_name_without_i2c(sensor_index);
	}

    sensor_index++;

    return sensor;
}

static void _unregister_sensor(struct nxp_capture *me)
{
    /* TODO */
}

/**
 * sysfs interface for sensor
 */
static ssize_t sensor_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct attribute *at = &attr->attr;
    const char *c = &at->name[strlen("sensor.")];
    int m = simple_strtoul(c, NULL, 10);
    return scnprintf(buf, PAGE_SIZE, "is_mipi:%d,name:%s", _sensor_info[m].is_mipi, _sensor_info[m].name);
}

static struct device_attribute sensor0_attr = __ATTR(sensor.0, 0644, sensor_show, NULL);
static struct device_attribute sensor1_attr = __ATTR(sensor.1, 0644, sensor_show, NULL);

static struct attribute *attrs[] = {
    &sensor0_attr.attr, &sensor1_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = (struct attribute **)attrs,
};

static int create_sensor_sysfs(int module)
{
    int ret;
    static bool sysfs_created = false;
    if (!sysfs_created) {
        struct kobject *kobj = kobject_create_and_add("camera sensor", &platform_bus.kobj);
        if (!kobj) {
            pr_err("%s: failed to kobject_create_and_add for camera sensor\n", __func__);
            return -EINVAL;
        }

        ret = sysfs_create_group(kobj, &attr_group);
        if (ret) {
            pr_err("%s: failed to sysfs_create_group for camera sensor\n", __func__);
            kobject_del(kobj);
            return -EINVAL;
        }

        sysfs_created = true;
    }
    return 0;
}

/**
 * public api
 */
struct nxp_capture *create_nxp_capture(int index,
        int module,
        struct nxp_capture_platformdata *pdata)
{
    struct nxp_capture *me;
    int ret;
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    bool csi_enabled = pdata->type == NXP_CAPTURE_INF_CSI;
#endif

    vmsg("%s entered\n", __func__);

    me = kzalloc(sizeof(*me), GFP_KERNEL);
    if (!me) {
        pr_err("%s: failed to allocate me\n", __func__);
        return NULL;
    }

    INIT_LIST_HEAD(&me->irq_entry_list);
    spin_lock_init(&me->lock);

    me->index  = index;
    me->module = module;
    me->irq    = _hw_get_irq_num(me);
    me->interface_type = pdata->type;
    me->platdata = pdata;

    /* initialize callback */
    me->get_media_device = _get_media_device;
    me->get_v4l2_device  = _get_v4l2_device;
    me->get_module_num   = _get_module_num;
    me->get_index_num    = _get_index_num;
    me->get_alloc_ctx    = _get_alloc_ctx;
    me->run              = _run;
    me->stop             = _stop;
    me->register_irq_entry   = _register_irq_entry;
    me->unregister_irq_entry = _unregister_irq_entry;
    me->get_csi_subdev    = _get_csi_subdev;
    me->get_sensor_subdev = _get_sensor_subdev;

#ifdef CONFIG_TURNAROUND_VIP_RESET
    me->backup_reset_restore_register = _backup_reset_restore_register;
#endif

    init_waitqueue_head(&me->wait_change_context);
    me->clip_enable = false;
    me->deci_enable = false;
    me->context_changed = false;

#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    /* init children */
    if (csi_enabled) {
        if (!pdata->csi) {
            pr_err("%s: no csi platform data\n", __func__);
            goto error_csi;
        }
        ret = nxp_csi_init(&me->csi, pdata->csi);
        if (ret < 0) {
            pr_err("%s: failed to nxp_csi_init()\n", __func__);
            goto error_csi;
        }
    }
#endif

    ret = nxp_vin_clipper_init(&me->vin_clipper, &pdata->parallel);
    if (ret < 0) {
        pr_err("%s: failed to nxp_vin_clipper_init()\n", __func__);
        goto error_vin;
    }

#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    ret = nxp_decimator_init(&me->decimator, &pdata->deci);
    if (ret < 0) {
        pr_err("%s: failed to nxp_decimator_init()\n", __func__);
        goto error_decimator;
    }
#endif

    /* create link */
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (csi_enabled) {
        ret = media_entity_create_link(
                &me->csi.subdev.entity, NXP_CSI_PAD_SOURCE,
                &me->vin_clipper.subdev.entity, NXP_VIN_PAD_SINK, 0);
        if (ret < 0) {
            pr_err("%s: failed to link csi source to vin sink\n", __func__);
            goto error_link;
        }

    }
    _set_sensor_mipi_info(index, csi_enabled);
#else
    _set_sensor_mipi_info(index, 0);
#endif

#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    ret = media_entity_create_link(
            &me->vin_clipper.subdev.entity, NXP_VIN_PAD_SOURCE_DECIMATOR,
            &me->decimator.subdev.entity, NXP_DECIMATOR_PAD_SINK, 0);
    if (ret < 0) {
        pr_err("%s: failed to link vin source to decimator sink\n", __func__);
        goto error_link;
    }
#endif

    /*
     * sysfs
     */
    create_sensor_sysfs(me->module);

    vmsg("%s exit(%p)\n", __func__, me);

    return me;

error_link:
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (csi_enabled)
        nxp_csi_cleanup(&me->csi);
error_csi:
#endif
#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    nxp_decimator_cleanup(&me->decimator);
error_decimator:
#endif
    nxp_vin_clipper_cleanup(&me->vin_clipper);
error_vin:
    kfree(me);
    return NULL;
}

void release_nxp_capture(struct nxp_capture *me)
{
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (me->interface_type == NXP_CAPTURE_INF_CSI)
        nxp_csi_cleanup(&me->csi);
#endif

#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    nxp_decimator_cleanup(&me->decimator);
#endif
    nxp_vin_clipper_cleanup(&me->vin_clipper);

    kfree(me);
}

int register_nxp_capture(struct nxp_capture *me)
{
    int ret;
    struct nxp_v4l2_i2c_board_info *sensor_info;
    struct v4l2_subdev *sensor;
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    bool csi_enabled = me->interface_type == NXP_CAPTURE_INF_CSI;
#endif

    vmsg("%s entered\n", __func__);

#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (csi_enabled) {
        ret = nxp_csi_register(&me->csi);
        if (ret < 0) {
            pr_err("%s: failed to nxp_csi_register()\n", __func__);
            return ret;
        }
    }
#endif

    ret = nxp_vin_clipper_register(&me->vin_clipper);
    if (ret < 0) {
        pr_err("%s: failed to nxp_vin_clipper_register()\n", __func__);
        goto error_vin;
    }

#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    ret = nxp_decimator_register(&me->decimator);
    if (ret < 0) {
        pr_err("%s: failed to nxp_decimator_register()\n", __func__);
        goto error_decimator;
    }
#endif

    /* find sensor subdev */
#if 0
    sensor_info = me->platdata->sensor;
    if (!sensor_info) {
        pr_err("%s: can't find sensor platdata\n", __func__);
        goto error_sensor;
    }

    sensor = _register_sensor(me, sensor_info);
    if (!sensor) {
        pr_err("%s: can't register sensor subdev\n", __func__);
        goto error_sensor;
    }
#else
    if (NULL == _register_sensor(me, me->platdata->sensor)) {
        pr_err("%s: can't register sensor subdev\n", __func__);
        goto error_sensor;
    }
#endif

    // psw0523 fix for urbetter
    /* ret = request_irq(me->irq, &_irq_handler, IRQF_DISABLED, "nxp-capture", me); */
    ret = request_irq(me->irq, &_irq_handler, IRQF_SHARED, "nxp-capture", me);
    if (ret) {
        pr_err("%s: failed to request_irq()\n", __func__);
        goto error_irq;
    }

    vmsg("%s success!!!\n", __func__);
    return 0;

error_irq:
    _unregister_sensor(me);
error_sensor:
#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    nxp_decimator_unregister(&me->decimator);
error_decimator:
#endif
    nxp_vin_clipper_unregister(&me->vin_clipper);
error_vin:
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (csi_enabled)
        nxp_csi_unregister(&me->csi);
#endif

    return ret;
}

void unregister_nxp_capture(struct nxp_capture *me)
{
    free_irq(me->irq, me);
    _unregister_sensor(me);
#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    nxp_decimator_unregister(&me->decimator);
#endif
    nxp_vin_clipper_unregister(&me->vin_clipper);
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (me->interface_type == NXP_CAPTURE_INF_CSI)
        nxp_csi_unregister(&me->csi);
#endif
}

#ifdef CONFIG_PM
int suspend_nxp_capture(struct nxp_capture *me)
{
    int ret;
    PM_DBGOUT("+%s\n", __func__);
#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    ret = nxp_decimator_suspend(&me->decimator);
    if (ret) {
        PM_DBGOUT("%s: failed to nxp_decimator_suspend() ret %d\n", __func__, ret);
        return ret;
    }
#endif
    ret = nxp_vin_clipper_suspend(&me->vin_clipper);
    if (ret) {
        PM_DBGOUT("%s: failed to nxp_vin_clipper_suspend() ret %d\n", __func__, ret);
        return ret;
    }
#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (me->interface_type == NXP_CAPTURE_INF_CSI) {
        ret = nxp_csi_suspend(&me->csi);
        if (ret) {
            PM_DBGOUT("%s: failed to nxp_csi_suspend() ret %d\n", __func__, ret);
            return ret;
        }
    }
#endif

    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

int resume_nxp_capture(struct nxp_capture *me)
{
    PM_DBGOUT("+%s\n", __func__);

    nxp_vin_clipper_resume(&me->vin_clipper);

#ifdef CONFIG_NXP_CAPTURE_DECIMATOR
    nxp_decimator_resume(&me->decimator);
#endif

#ifdef CONFIG_NXP_CAPTURE_MIPI_CSI
    if (me->interface_type == NXP_CAPTURE_INF_CSI)
        nxp_csi_resume(&me->csi);
#endif

    if (NXP_ATOMIC_READ(&me->running_child_bitmap)) {
        PM_DBGOUT("%s: call _hw_chile_enable()\n", __func__);
        /* _hw_run(me, true); */
        _hw_child_enable(me, NXP_ATOMIC_READ(&me->running_child_bitmap), true);
    }

    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif
