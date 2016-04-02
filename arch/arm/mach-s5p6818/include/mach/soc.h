/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __MACH_SOC_H__
#define __MACH_SOC_H__

/*
 *
 * 	Gpio control interface functions for nexell cpu
 *
 */

/* Nomal */
extern void 		nxp_soc_gpio_device_init(void);
extern int 			nxp_soc_gpio_get_altnum(unsigned int io);
extern void 		nxp_soc_gpio_set_io_func(unsigned int io, unsigned int func);
extern unsigned int nxp_soc_gpio_get_io_func(unsigned int io);
extern void 		nxp_soc_gpio_set_io_dir(unsigned int io, int out);
extern int 		 	nxp_soc_gpio_get_io_dir(unsigned int io);
extern void 		nxp_soc_gpio_set_io_pull_enb(unsigned int io, int on);
extern int 		 	nxp_soc_gpio_get_io_pull_enb(unsigned int io);
extern void 		nxp_soc_gpio_set_io_pull_sel(unsigned int io, int on);
extern int 		 	nxp_soc_gpio_get_io_pull_sel(unsigned int io);
extern void			nxp_soc_gpio_set_io_drv(int gpio, int mode);
extern int			nxp_soc_gpio_get_io_drv(int gpio);
extern void			nxp_soc_gpio_set_out_value(unsigned int io, int high);
extern int 		 	nxp_soc_gpio_get_out_value(unsigned int io);
extern int 		 	nxp_soc_gpio_get_in_value(unsigned int io);

/* Interrupt */
extern void 		nxp_soc_gpio_set_int_enable(unsigned int io, int on);
extern int 		 	nxp_soc_gpio_get_int_enable(unsigned int io);
extern void 		nxp_soc_gpio_set_int_mode(unsigned int io, unsigned int mode);
extern int 		 	nxp_soc_gpio_get_int_mode(unsigned int io);
extern int 		 	nxp_soc_gpio_get_int_pend(unsigned int io);
extern void 		nxp_soc_gpio_clr_int_pend(unsigned int io);

/* Alive */
extern void 		nxp_soc_alive_set_det_enable(unsigned int io, int on);
extern int 		 	nxp_soc_alive_get_det_enable(unsigned int io);
extern void 		nxp_soc_alive_set_det_mode(unsigned int io, unsigned int mode, int on);
extern int 		 	nxp_soc_alive_get_det_mode(unsigned int io, unsigned int mode);
extern int 		 	nxp_soc_alive_get_int_pend(unsigned int io);
extern void 		nxp_soc_alive_clr_int_pend(unsigned int io);

/*
 *
 * 	PWM interface functions for nexell cpu
 *
 */
void			nxp_soc_pwm_start(int ch, int irqon);
void			nxp_soc_pwm_stop(int ch, int irqon);
unsigned long	nxp_soc_pwm_set_frequency(int ch, unsigned int request, unsigned int duty);
void 			nxp_soc_pwm_get_frequency(int ch, unsigned int *freq, unsigned int *duty);

/*
 *
 * 	Display control interface functions for nexell cpu
 *
 */
#include "display.h"

/* rgb layer control on multi layer */
int  		 nxp_soc_disp_rgb_set_fblayer	(int module, int layer);
int  		 nxp_soc_disp_rgb_get_fblayer	(int module);
int  		 nxp_soc_disp_rgb_set_format	(int module, int layer, unsigned int format,
											int image_w, int image_h, int pixelbyte);
void    	 nxp_soc_disp_rgb_get_format	(int module, int layer, unsigned int *format,
											int *image_w, int *image_h, int *pixelbyte);
int 		 nxp_soc_disp_rgb_set_position	(int module, int layer, int x, int y, int waitvsync);
int 		 nxp_soc_disp_rgb_get_position	(int module, int layer, int *x, int *y);
int 		 nxp_soc_disp_rgb_set_clipping	(int module, int layer, int left, int top, int width, int height);
void 		 nxp_soc_disp_rgb_get_clipping	(int module, int layer, int *left, int *top, int *width, int *height);
void 		 nxp_soc_disp_rgb_set_address	(int module, int layer,
											unsigned int phyaddr, unsigned int pixelbyte, unsigned int stride,
											int waitvsync);
void 		 nxp_soc_disp_rgb_get_address	(int module, int layer,
											unsigned int *phyaddr,unsigned int *pixelbyte, unsigned int*stride);
void  		 nxp_soc_disp_rgb_set_color		(int module, int layer, unsigned int type, unsigned int color, int enable);
unsigned int nxp_soc_disp_rgb_get_color		(int module, int layer, unsigned int type);
void 		 nxp_soc_disp_rgb_set_enable	(int module, int layer, int enable);
int 		 nxp_soc_disp_rgb_stat_enable	(int module, int layer);

/* video layer control on multi layer */
int 		 nxp_soc_disp_video_set_format	(int module, unsigned int fourcc, int image_w, int image_h);
void 		 nxp_soc_disp_video_get_format	(int module, unsigned int *fourcc, int *image_w, int *image_h);
int 		 nxp_soc_disp_video_set_position(int module, int left, int top, int right, int bottom, int waitvsync);
int 		 nxp_soc_disp_video_get_position(int module, int*left, int*top, int*right, int*bottom);

void 		 nxp_soc_disp_video_set_address	(int module, unsigned int lu_a, unsigned int lu_s,
											unsigned int cb_a, unsigned int cb_s, unsigned int cr_a, unsigned int cr_s,
											int waitvsync);
void 		 nxp_soc_disp_video_get_address	(int module, unsigned int *lu_a, unsigned int *lu_s,
											unsigned int *cb_a, unsigned int *cb_s, unsigned int *cr_a,	unsigned int*cr_s);
void 		 nxp_soc_disp_video_set_priority(int module, int prior);
int  		 nxp_soc_disp_video_get_priority(int module);
void 		 nxp_soc_disp_video_set_colorkey(int module, unsigned int color, int enable);
unsigned int nxp_soc_disp_video_get_colorkey(int module);
void 		 nxp_soc_disp_video_set_color	(int module, unsigned int type, unsigned int color, int enable);
unsigned int nxp_soc_disp_video_get_color	(int module, unsigned int type);
void 		 nxp_soc_disp_video_set_hfilter	(int module, int enable);
void 		 nxp_soc_disp_video_set_vfilter	(int module, int enable);
unsigned int nxp_soc_disp_video_stat_hfilter(int module);
unsigned int nxp_soc_disp_video_stat_vfilter(int module);

void 		 nxp_soc_disp_video_set_enable	(int module, int enable);
int 		 nxp_soc_disp_video_stat_enable	(int module);

/* psw0523 add for video out source crop */
void         nxp_soc_disp_video_set_crop(int module, bool enable, int left, int top, int width, int height, int waitvsync);

/* top layer control on multi layer */
void 		 nxp_soc_disp_get_resolution(int module, int *w, int *h);
void 		 nxp_soc_disp_set_bg_color(int module, unsigned int color);
unsigned int nxp_soc_disp_get_bg_color(int module);
void 		 nxp_soc_disp_layer_set_enable (int module, int layer, int enable);
int 		 nxp_soc_disp_layer_stat_enable(int module, int layer);
int 		 nxp_soc_disp_wait_vertical_sync(int module);	/* syncgen's interrupt */
struct disp_irq_callback *nxp_soc_disp_register_irq_callback(int module, void (*callback)(void *), void *data);
void		 nxp_soc_disp_unregister_irq_callback(int module, struct disp_irq_callback *);

/*
 * display device control: refer to display.h
 * device 0=resconv, 1=lcd, 2=hdmi, 3=mipi, 4=lvds, 5=syncgen0, 6=syncgen1
 */
void 		 nxp_soc_disp_register_proc_ops(enum disp_dev_type device, struct disp_process_ops *ops);
void         nxp_soc_disp_register_priv(enum disp_dev_type device, void *priv);
int			 nxp_soc_disp_device_connect_to(enum disp_dev_type device, enum disp_dev_type to, struct disp_vsync_info *vsync);
void	     nxp_soc_disp_device_disconnect(enum disp_dev_type device, enum disp_dev_type to);

int			 nxp_soc_disp_device_set_sync_param	(enum disp_dev_type device, struct disp_syncgen_par *sync_par);
int			 nxp_soc_disp_device_get_sync_param	(enum disp_dev_type device, struct disp_syncgen_par *sync_par);
int			 nxp_soc_disp_device_set_vsync_info	(enum disp_dev_type device, struct disp_vsync_info *vsync);
int			 nxp_soc_disp_device_get_vsync_info	(enum disp_dev_type device, struct disp_vsync_info *vsync);

int			 nxp_soc_disp_device_set_dev_param  (enum disp_dev_type device, void *param);

int 		 nxp_soc_disp_device_enable		(enum disp_dev_type device, int enable);
int 		 nxp_soc_disp_device_stat_enable(enum disp_dev_type device);

int 		 nxp_soc_disp_device_suspend	(enum disp_dev_type device);
void 		 nxp_soc_disp_device_resume 	(enum disp_dev_type device);

void		 nxp_soc_disp_device_framebuffer(int module, int fb);
int 		 nxp_soc_disp_device_enable_all	(int module, int enable);
int 		 nxp_soc_disp_device_suspend_all(int module);
void 		 nxp_soc_disp_device_resume_all	(int module);
void 		 nxp_soc_disp_device_reset_top	(void);
int 		 nxp_soc_disp_device_enable_all_saved(int module, int enable);

void 		 nxp_soc_disp_register_lcd_ops(int module, struct lcd_operation *pops);

/*
 *
 * 	Reset interface functions for nexell cpu
 *
 */
void		nxp_soc_peri_reset_enter(int id);
void		nxp_soc_peri_reset_exit(int id);
void		nxp_soc_peri_reset_set(int id);
int			nxp_soc_peri_reset_status(int id);	/* 1: power on, 0: power down */

#endif	/* __SOC_H__ */

