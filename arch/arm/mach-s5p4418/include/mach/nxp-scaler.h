#ifndef _MACH_NXP_SCALER_H
#define _MACH_NXP_SCALER_H

#include "ioc_magic.h"

struct nxp_scaler_ioctl_data {
    uint32_t src_phys[3];
    uint32_t src_stride[3];
    unsigned int src_width;
    unsigned int src_height;
    unsigned int src_code;
    uint32_t dst_phys[3];
    uint32_t dst_stride[3];
    unsigned int dst_width;
    unsigned int dst_height;
    unsigned int dst_code;
};

enum {
    IOCTL_SCALER_SET_AND_RUN    =  _IO(IOC_NX_MAGIC, 1),
};

#endif
