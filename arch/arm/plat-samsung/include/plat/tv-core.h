/*
 * arch/arm/plat-samsung/include/plat/tv.h
 *
 * Copyright 2011 Samsung Electronics Co., Ltd.
 *	Tomasz Stanislawski <t.stanislaws@samsung.com>
 *
 * Samsung TV driver core functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SAMSUNG_PLAT_TV_H
#define __SAMSUNG_PLAT_TV_H __FILE__

/*
 * These functions are only for use with the core support code, such as
 * the CPU-specific initialization code.
 */

/* Re-define device name to differentiate the subsystem in various SoCs. */
static inline void s5p_hdmi_setname(char *name)
{
#ifdef CONFIG_S5P_DEV_TV
	s5p_device_hdmi.name = name;
#endif
}

static inline void s5p_mixer_setname(char *name)
{
#ifdef CONFIG_S5P_DEV_TV
	s5p_device_mixer.name = name;
#endif
}

static inline void s5p_sdo_setname(char *name)
{
#ifdef CONFIG_S5P_DEV_TV
	s5p_device_sdo.name = name;
#endif
}

struct s5p_platform_cec {
#ifdef CONFIG_S5P_DEV_TV
	void    (*cfg_gpio)(struct platform_device *pdev);
#endif
};

struct s5p_hdmi_platdata {
	void (*hdmiphy_enable)(struct platform_device *pdev, int en);
};

extern void s5p_hdmi_set_platdata(struct s5p_hdmi_platdata *pd);
#ifdef CONFIG_S5P_DEV_TV
extern void __init s5p_hdmi_cec_set_platdata(struct s5p_platform_cec *pd);
extern void s5p_cec_cfg_gpio(struct platform_device *pdev);
extern void s5p_int_src_hdmi_hpd(struct platform_device *pdev);
extern void s5p_int_src_ext_hpd(struct platform_device *pdev);
extern int s5p_hpd_read_gpio(struct platform_device *pdev);
extern int s5p_v4l2_hpd_read_gpio(void);
extern void s5p_v4l2_int_src_hdmi_hpd(void);
extern void s5p_v4l2_int_src_ext_hpd(void);
extern void s5p_cec_cfg_gpio(struct platform_device *pdev);
extern void s5p_hdmiphy_enable(struct platform_device *pdev, int en);
#endif

#endif /* __SAMSUNG_PLAT_TV_H */
