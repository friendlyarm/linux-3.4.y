#ifndef _MACH_NXP_HDMI_CEC_H
#define _MACH_NXP_HDMI_CEC_H

#include "ioc_magic.h"

enum {
    IOCTL_HDMI_CEC_SETLADDR    =  _IOW(IOC_NX_MAGIC, 1, unsigned int),
    IOCTL_HDMI_CEC_GETPADDR    =  _IOR(IOC_NX_MAGIC, 2, unsigned int),
};

#define NXP_HDMI_CEC_DRV_NAME       "nxp-hdmi-cec"

#endif

