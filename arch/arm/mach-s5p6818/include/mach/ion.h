#ifndef __MACH_NXP_ION_H__
#define __MACH_NXP_ION_H__

struct platform_device;

#ifdef CONFIG_ION_NXP
extern struct platform_device nxp_device_ion;
void nxp_ion_set_platdata(void);
#else
static inline void nxp_ion_set_platdata(void)
{
}
#endif

#endif
