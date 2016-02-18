/*
 * (C) Copyright 2010
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/memblock.h>
#include <linux/highmem.h>
#include <linux/fdtable.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <linux/major.h>
#include <linux/pm.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#ifdef CONFIG_FB_NXP_ION_MEM
#include <linux/dma-buf.h>
#include <linux/nxp_ion.h>
#include <linux/ion.h>

extern struct ion_device *get_global_ion_device(void);
#endif
extern int nx_fb_init_sysfs(struct fb_info *fb_info);

/*
 *  Alters the hardware state
 */

#define SUPPORT_ALTER_HARDWARE_STATE

#if (0)
#define	DUMP_VAR_SCREENINFO
#define pr_debug		printk
#endif

#define	FB_DEV_PIXELCLOCK	(27000000)	/* 27Mhz */
#define	FB_CLEAR_COLOR		(0x0)
#define FB_PALETTE_CLEAR 	(0x80000000)

/*------------------------------------------------------------------------------
 * framebuffer struct
 */

#ifdef CONFIG_FB_NXP_ION_MEM
#define MAX_DMABUF_CONTEXT      3

struct dma_buf_context {
    struct ion_handle *ion_handle;
    struct dma_buf    *dma_buf;
    struct dma_buf_attachment *attachment;
    struct sg_table   *sg_table;
    dma_addr_t         dma_addr;
    void              *virt;
    int                user_fd;
};

struct nxp_fb_dma_buf_data {
    struct ion_client *ion_client;
    struct dma_buf_context context[MAX_DMABUF_CONTEXT];
};
#endif

struct nxp_fb_device {
	int			  fb_id;
	int 		  device_id;	/* 0: SYNGEN0 1: SYNGEN1 */
	int			  layer;
	int			  x_resol_max;	/* x max resolution */
	int			  y_resol_max;	/* y max resolution */
	int			  x_resol;		/* x resolution */
	int			  y_resol;		/* y resolution */
	int			  x_virt;		/* virtual x resolution */
	int			  y_virt;		/* virtual y resolution */
	int			  buffer_num;
	int			  pixelbit;		/* bit per pixel */
	unsigned int  format;		/* frame buffer format */
	unsigned int  bgcolor;
	int			  lcd_w_mm;		/* width  mm for dpi */
	int			  lcd_h_mm;		/* hegiht  mm for dpi */
	unsigned int  hs_left;
	unsigned int  hs_right;
	unsigned int  vs_upper;
	unsigned int  vs_lower;
	unsigned int  pixelclk;
	/* frame buffer */
	unsigned int  fb_phy_base;
	unsigned int  fb_phy_end;
	unsigned int  fb_phy_len;
	void		 *fb_vir_base;
	unsigned int  fb_pan_phys;
	int			  fb_remapped;
	int			  skip_vsync;
#ifdef CONFIG_FB_NXP_ION_MEM
    struct device *dev;
    struct nxp_fb_dma_buf_data dma_buf_data;
#endif
};

struct nxp_fb_param {
	struct fb_info	*		info;
	u32						palette_buffer[256];
	u32						pseudo_pal[16];

	/* fb hw device */
	struct nxp_fb_device 	fb_dev;
	unsigned int			status;
};

#define	_R_BIT(bpi)	 	(16 == bpi ? 5 : 8)
#define	_G_BIT(bpi)	 	(16 == bpi ? 6 : 8)
#define	_B_BIT(bpi)	 	(16 == bpi ? 5 : 8)
#define	_T_BIT(bpi)	 	(32 == bpi ? 8 : 0)

#define	FB_STAT_INIT	(1)

/*---------------------------------------------------------------------------------------------
 * 	FB device
 */
static int nxp_fb_dev_get_vsync(int module, int fb, struct disp_vsync_info *vsi)
{
#ifdef CONFIG_NXP_DISPLAY
	enum disp_dev_type device = DISP_DEVICE_SYNCGEN0;

	if (-1 == module)
		return 0;

	if (1 == module)
		device = DISP_DEVICE_SYNCGEN1;

	if (0 > nxp_soc_disp_device_get_vsync_info(device, vsi)) {
		printk(KERN_ERR "Fail to get framebuffer video sync (display.%d)\n", module);
		return -EINVAL;
	}

	nxp_soc_disp_device_framebuffer(module, fb);
#endif
	return 0;
}

static int nxp_fb_dev_setup(struct nxp_fb_param *par)
{
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	int layer = par->fb_dev.layer;
	int xres = par->fb_dev.x_resol;
	int yres = par->fb_dev.y_resol;
	int pixel = par->fb_dev.pixelbit >> 3;
	u32 phys = par->fb_dev.fb_phy_base;

	if (-1 == module)
		return 0;

	nxp_soc_disp_set_bg_color(module, par->fb_dev.bgcolor);
	nxp_soc_disp_rgb_set_format(module, layer, par->fb_dev.format, xres, yres, pixel);
	nxp_soc_disp_rgb_set_address(module, layer, phys, pixel, xres*pixel, 1);
	nxp_soc_disp_rgb_set_enable(module, layer, 1);
#endif
	return 0;
}

static unsigned nxp_fb_dev_set_layer(struct nxp_fb_param *par)
{
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	int layer = par->fb_dev.layer;

	if (-1 == module)
		return 0;

	return nxp_soc_disp_rgb_set_fblayer(module, layer);
#else
	return 0;
#endif
}

static void nxp_fb_dev_set_addr(struct nxp_fb_param *par, unsigned phys, int waitvsync)
{
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	int layer  = par->fb_dev.layer;
	int xres = par->fb_dev.x_resol;
	int pixel = par->fb_dev.pixelbit >> 3;

	if (-1 == module)
		return;

	nxp_soc_disp_rgb_set_address(module, layer, phys, pixel, xres*pixel, waitvsync);
#endif
}

static int nxp_fb_dev_enable(struct nxp_fb_param *par, bool on, int force)
{
#if defined CONFIG_NXP_DISPLAY && !defined(CONFIG_LOGO_NXP_COPY)
	int module = par->fb_dev.device_id;
	int stat = 0;

	if (-1 == module)
		return 0;

	if (!force)
		stat = nxp_soc_disp_device_stat_enable(DISP_DEVICE_SYNCGEN0 + module);

	if (!stat)
		nxp_soc_disp_device_enable_all(module, on ? 1: 0);
#endif
	return 0;
}

static int nxp_fb_dev_pos(struct nxp_fb_param *par, int left, int right, int waitvsync)
{
	int module = par->fb_dev.device_id;
	int layer  = par->fb_dev.layer;

	if (-1 == module)
		return 0;
	printk("[%s] %d.%d %d,%d\n", __func__, module, layer, left, right);
	return nxp_soc_disp_rgb_set_position(module, layer, left, right, waitvsync);
}

static int nxp_fb_dev_suspend(struct nxp_fb_param *par)
{
	int ret = 0;
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	if (-1 == module)
		return 0;

	PM_DBGOUT("%s\n", __func__);
	ret = nxp_soc_disp_device_suspend_all(module);
#endif
	return ret;
}

static int nxp_fb_dev_resume(struct nxp_fb_param *par)
{
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	if (-1 == module)
		return 0;

	PM_DBGOUT("%s\n", __func__);

	nxp_soc_disp_device_resume_all(module);
#endif
	return 0;
}

static int nxp_fb_dev_output(struct nxp_fb_param *par, int enable)
{
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	if (-1 == module)
		return 0;

	pr_debug("%s: %d %s\n", __func__, module, enable?"ON":"OFF");
	return nxp_soc_disp_device_enable_all_saved(module, enable);
#endif
}

static inline void *fb_copy_map(struct page *page, unsigned int phys, int size)
{
	unsigned long num_pages, i;
	struct page **pages;
	void *virt = NULL;

	num_pages = size >> PAGE_SHIFT;
	pages = kmalloc(num_pages * sizeof(struct page *), GFP_KERNEL);

	if (!pages)
		return NULL;

	for (i = 0; i < num_pages; i++)
		pages[i] = pfn_to_page((phys >> PAGE_SHIFT) + i);

	virt = vmap(pages, num_pages, VM_MAP, pgprot_writecombine(PAGE_KERNEL));
	if (!virt) {
		kfree(pages);
		return NULL;
	}
	kfree(pages);

	return virt;
}

static inline  void fb_copy_unmap(struct page *page, void *virt)
{
	vunmap((void*)virt);
}

#if defined(CONFIG_LOGO_NXP_COPY)
static unsigned nxp_fb_dev_get_addr(struct nxp_fb_param *par)
{
	unsigned int phyaddr = 0;
#ifdef CONFIG_NXP_DISPLAY
	int module = par->fb_dev.device_id;
	int layer = par->fb_dev.layer;

	if (-1 == module)
		return 0;

	nxp_soc_disp_rgb_get_address(module, layer, &phyaddr, NULL, NULL);
#endif
	return phyaddr;
}

static void nxp_fb_copy_boot_logo(struct nxp_fb_param *par, int size)
{
	unsigned base = par->fb_dev.fb_phy_base;
	unsigned dest = (unsigned)par->fb_dev.fb_vir_base;
	unsigned phys = 0;
	void *virt = NULL;
	struct page *page;

	phys = nxp_fb_dev_get_addr(par);

	if (phys) {
		int reserved = memblock_is_region_reserved(phys, size);
		int new = 0;
		if (!reserved) {
			memblock_reserve(phys, size);
			new = memblock_is_region_reserved(phys, size);
		}
		/* __memblock_dump_all(); */

		if (reserved || new)
			virt = fb_copy_map(page, phys, size);

		if (virt) {
			memcpy((void*)dest, (const void*)virt, size);
			dmac_map_area((void*)dest, size, DMA_TO_DEVICE);	/* L1 flush cache */
			outer_clean_range(base, base + size);				/* L2 flush cache */

			fb_copy_unmap(page, virt);
			if (new)
				memblock_free(phys, size);
		}
	}

 	if (phys && 0 == virt) {
		printk("%s Fail boot logo copy from 0x%08x to 0x%08x (0x%08x)\n",
			__func__, phys, base, dest);
		__memblock_dump_all();
	}
}
#endif

static void inline
nxp_fb_init_display(struct fb_info *info)
{
	struct nxp_fb_param  *par = info->par;
	int module = par->fb_dev.device_id;
	int xres   = info->var.xres;
	int yres   = info->var.yres;
	int pixel  = info->var.bits_per_pixel >> 3;

	nxp_fb_dev_set_layer(par);

	#if defined(CONFIG_LOGO_NXP_COPY)
	nxp_fb_copy_boot_logo(par, (xres * yres * pixel));
	#else
	memset((void*)par->fb_dev.fb_vir_base,
			FB_CLEAR_COLOR, par->fb_dev.fb_phy_len);
	#endif

	#if !defined(CONFIG_BACKLIGHT_PWM) || !defined(CONFIG_LOGO_NXP_COPY)
	nxp_fb_dev_enable(par, false, 1);	/* display out : off */
    #endif

	nxp_fb_dev_setup(par);
	nxp_fb_dev_enable(par, true, 0);

	par->status = FB_STAT_INIT;

	printk(KERN_INFO "%s: out[%d], %d * %d - %d bpp (phys:%08x virt:0x%p max:%d)\n",
		info->fix.id, module, xres, yres, pixel<<3,
		par->fb_dev.fb_phy_base, par->fb_dev.fb_vir_base, par->fb_dev.fb_phy_len);
}

static void inline nxp_fb_set_base(struct fb_info *info)
{
	struct nxp_fb_param *par = info->par;
	u32 phys = par->fb_dev.fb_pan_phys;

	pr_debug("%s: %d * %d - %d bpp (phys:%08x)\n",
		info->fix.id, par->fb_dev.x_resol, par->fb_dev.y_resol,
		par->fb_dev.pixelbit, par->fb_dev.fb_phy_base);

	nxp_fb_dev_set_addr(par, phys, 1);

	par->status = FB_STAT_INIT;
}

static void inline nxp_fb_update_buffer(struct fb_info *info, int waitvsync)
{
	struct nxp_fb_param *par = info->par;
	int phys = par->fb_dev.fb_pan_phys;

	waitvsync = par->fb_dev.skip_vsync ? 0 : waitvsync;

	if (par->status & FB_STAT_INIT)
		nxp_fb_dev_set_addr(par, phys, waitvsync);

	pr_debug("%s: %s, phys=0x%08x (init=%s)\n", __func__,
		info->fix.id, phys, par->status&FB_STAT_INIT?"yes":"no");
}

static int inline nxp_fb_verify_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	return 0;
}

#ifdef DUMP_VAR_SCREENINFO
static void nxp_fb_dump_var(struct fb_var_screeninfo *var)
{
	pr_debug(": var->xres                 = %d\n", var->xres);
	pr_debug(": var->tres                 = %d\n", var->yres);
	pr_debug(": var->xres_virtual         = %d\n", var->xres_virtual);
	pr_debug(": var->yres_virtual         = %d\n", var->yres_virtual);
	pr_debug(": var->xoffset              = %d\n", var->xoffset);
	pr_debug(": var->yoffset              = %d\n", var->yoffset);
	pr_debug(": var->bits_per_pixel       = %d\n", var->bits_per_pixel);
	pr_debug(": var->grayscale            = %d\n", var->grayscale);
	pr_debug(": var->nonstd               = %d\n", var->nonstd);
	pr_debug(": var->activate             = %d\n", var->activate);
	pr_debug(": var->height               = %d\n", var->height);
	pr_debug(": var->width                = %d\n", var->width);
	pr_debug(": var->accel_flags          = %d\n", var->accel_flags);
	pr_debug(": var->pixclock             = %d\n", var->pixclock);
	pr_debug(": var->left_margin          = %d\n", var->left_margin);
	pr_debug(": var->right_margin         = %d\n", var->right_margin);
	pr_debug(": var->upper_margin         = %d\n", var->upper_margin);
	pr_debug(": var->lower_margin         = %d\n", var->lower_margin);
	pr_debug(": var->hsync_len            = %d\n", var->hsync_len);
	pr_debug(": var->vsync_len            = %d\n", var->vsync_len);
	pr_debug(": var->sync                 = %d\n", var->sync);
	pr_debug(": var->vmode                = %d\n", var->vmode);
	pr_debug(": var->rotate               = %d\n", var->rotate);
	pr_debug(": var->red.offset           = %d\n", var->red.offset);
	pr_debug(": var->red.length           = %d\n", var->red.length);
	pr_debug(": var->red.msb_right        = %d\n", var->red.msb_right);
	pr_debug(": var->green.offset         = %d\n", var->green.offset);
	pr_debug(": var->green.length         = %d\n", var->green.length);
	pr_debug(": var->green.msb_right      = %d\n", var->green.msb_right);
	pr_debug(": var->blue.offset          = %d\n", var->blue.offset);
	pr_debug(": var->blue.length          = %d\n", var->blue.length);
	pr_debug(": var->blue.msb_right       = %d\n", var->blue.msb_right);
	pr_debug(": var->transp.offset        = %d\n", var->transp.offset);
	pr_debug(": var->transp.length        = %d\n", var->transp.length);
	pr_debug(": var->transp.msb_right     = %d\n", var->transp.msb_right);
}
#endif

/*---------------------------------------------------------------------------------------------
 * 	FB
 */
static struct fb_ops nxp_fb_ops;
static struct fb_info *nxp_fb_init_fb(int fb, struct device *dev)
{
	struct fb_info *info = NULL;
	char name[256];

	/*
	 * 	allocate :
	 *	fb_info + sizeof(struct nxp_fb_param)
	 *	info->par = struct nxp_fb_param
	 *
	 */
	info = framebuffer_alloc(sizeof(struct nxp_fb_param), dev);
	if (! info) {
		printk(KERN_ERR "Fail, unable to allocate frame buffer(%d) info ...\n", fb);
		return NULL;
	}

	/* fb_info members */
	sprintf(name, "%s %d", DEV_NAME_FB, fb);
	strcpy(info->fix.id, name);

	/* default fixed information */
	info->fix.type	    	= FB_TYPE_PACKED_PIXELS;
	info->fix.type_aux		= 0;
	info->fix.xpanstep		= 0;	/* not suppor horizontal pan (fb_mem.c) */
	info->fix.ypanstep		= 1;	/* when pan, check yoffset align with ypanstep value (fb_mem.c) */
	info->fix.ywrapstep		= 0;
	info->fix.accel	    	= FB_ACCEL_NONE;

	/* default variable information */
	info->var.nonstd	   	= 0;
	info->var.activate		= FB_ACTIVATE_NOW;
	info->var.accel_flags	= 0;
	info->var.vmode	    	= FB_VMODE_NONINTERLACED;

	/* link machind file operation to fb_info's file operation */
	info->fbops	= &nxp_fb_ops;
	info->flags	= FBINFO_FLAG_DEFAULT;

	/* default colormap: palette */
	if (fb_alloc_cmap(&info->cmap, 256, 0)) {
		printk(KERN_ERR "Fail, unable to allocate cmap for frame buffer(%d) ...\n", fb);
		framebuffer_release(info);
		return NULL;
	}
	return info;
}

static void nxp_fb_exit_fb(struct fb_info *info)
{
	fb_dealloc_cmap(&info->cmap);
	framebuffer_release(info);
}

static int nxp_fb_setup_param(int fb, struct fb_info *info, void *data)
{
	struct nxp_fb_plat_data *plat = data;
	struct nxp_fb_param  *par = info->par;
	struct nxp_fb_device *dev = &par->fb_dev;
	struct disp_vsync_info vsi = { 0, };
	int x_resol, y_resol;
	int i, ret;

	/* get from output device */
	ret = nxp_fb_dev_get_vsync(plat->module, fb, &vsi);
	if (0 > ret)
		return ret;

	/* get from output device sync */
	x_resol = vsi.h_active_len ? vsi.h_active_len : plat->x_resol;
	y_resol = vsi.v_active_len ? vsi.v_active_len : plat->y_resol;

	/* clear palette buffer */
	par->info = info;
	par->status = 0;
	for (i = 0; i < 256; i++)
		par->palette_buffer[i] = FB_PALETTE_CLEAR;

	/* set hw variables */
	dev->fb_id    	  = fb;
	dev->device_id    = plat->module;
	dev->layer    	  = plat->layer;
	dev->x_resol_max  = plat->x_resol_max ? plat->x_resol_max : x_resol;
	dev->y_resol_max  = plat->y_resol_max ? plat->y_resol_max : y_resol;
	dev->x_resol 	  = x_resol;
	dev->y_resol 	  = y_resol;
	dev->x_virt		  = x_resol;
	dev->y_virt		  = y_resol * plat->buffers;
	dev->buffer_num	  = plat->buffers;
	dev->pixelbit 	  = plat->bitperpixel;
	dev->format		  = plat->format;
	dev->bgcolor	  = plat->bgcolor;
	dev->lcd_w_mm 	  = plat->lcd_with_mm;
	dev->lcd_h_mm	  = plat->lcd_height_mm;
	dev->fb_phy_base  = plat->fb_mem_base;
	dev->fb_phy_end   = plat->fb_mem_end;
	dev->fb_pan_phys  = plat->fb_mem_base;

	dev->hs_left	  = vsi.h_sync_width + vsi.h_back_porch;
	dev->hs_right	  = vsi.h_front_porch;
	dev->vs_upper	  = vsi.v_sync_width + vsi.v_back_porch;
	dev->vs_lower	  = vsi.v_front_porch;
	dev->pixelclk	  = vsi.pixel_clock_hz ? vsi.pixel_clock_hz : FB_DEV_PIXELCLOCK;
	dev->skip_vsync   = plat->skip_pan_vsync ? 1 : 0;

	dev->fb_vir_base  = 0;
	dev->fb_remapped  = 0;

	pr_debug("%s (out dev=%d)\n", __func__, dev->device_id);
	return 0;
}

static void nxp_fb_setup_info(struct fb_info *info)
{
	struct nxp_fb_param  *par = info->par; /* get nxp_fb_param base */
	struct nxp_fb_device *dev = &par->fb_dev;

	int x_v, y_v, bpp = 0;

	pr_debug("%s\n", __func__);

	bpp = dev->pixelbit;
	x_v = dev->x_resol > dev->x_virt ? dev->x_resol : dev->x_virt;
	y_v = dev->y_resol > dev->y_virt ? dev->y_resol : dev->y_virt;

	/* other variable information */
	info->var.width	    	= dev->lcd_w_mm; 	/* width  mm for dpi */
	info->var.height	    = dev->lcd_h_mm; 	/* height  mm for dpi */
	info->var.bits_per_pixel= bpp;

	/* get frame rate */
	info->var.left_margin	= dev->hs_left;
	info->var.right_margin	= dev->hs_right;
	info->var.upper_margin	= dev->vs_upper;
	info->var.lower_margin	= dev->vs_lower;
	info->var.pixclock		= KHZ2PICOS(dev->pixelclk/1000);	/* pico second */

	/* get resolution */
	info->var.xres	    	= dev->x_resol;
	info->var.yres	    	= dev->y_resol;
	info->var.bits_per_pixel= dev->pixelbit;
	info->var.xres_virtual 	= x_v;
	info->var.yres_virtual 	= y_v;

	/* get pixel format */
	info->var.red.offset  	= _G_BIT(bpp) + _B_BIT(bpp);
	info->var.green.offset  = _B_BIT(bpp);
	info->var.blue.offset   = 0;
	info->var.transp.offset = _T_BIT(bpp) ? (_R_BIT(bpp) + _G_BIT(bpp) + _B_BIT(bpp)) : 0;
	info->var.red.length    = _R_BIT(bpp);
	info->var.green.length  = _G_BIT(bpp);
	info->var.blue.length   = _B_BIT(bpp);
	info->var.transp.length = _T_BIT(bpp);

	/* other palette & fixed */
	info->pseudo_palette  = &par->pseudo_pal;
	info->fix.smem_len    = x_v * y_v * (bpp >> 3);
	info->fix.line_length = (info->var.xres * info->var.bits_per_pixel) >> 3;
	switch (bpp) {
		case 32:
		case 24:
		case 16:
			info->fix.visual = FB_VISUAL_TRUECOLOR;
			break;
		case 1:
			info->fix.visual = FB_VISUAL_MONO01;
			break;
		default:
			info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
			break;
	}

	pr_debug("res: %d by %d (%dEA), virtual: %d by %d, length:%d\n",
		dev->x_resol, dev->y_resol, dev->buffer_num, x_v, y_v, info->fix.smem_len);
}

#ifdef CONFIG_FB_NXP_ION_MEM
static int nxp_fb_setup_ion(struct nxp_fb_dma_buf_data *d)
{
    struct ion_device *ion_dev = get_global_ion_device();
    memset(d, 0, sizeof(*d));

    if (!ion_dev) {
        pr_err("%s Error: no ion device!!!\n", __func__);
        return -EINVAL;
    }

    d->ion_client = ion_client_create(ion_dev, "nxp-fb");
    if (IS_ERR(d->ion_client)) {
        pr_err("%s Error: ion_client_create()\n", __func__);
        return -EINVAL;
    }

    return 0;
}

static int nxp_fb_map_ion_handle(struct nxp_fb_device *fb_dev,
        struct dma_buf_context *ctx,
        struct ion_handle *ion_handle, struct dma_buf *buf)
{
    int ret = 0;

    ctx->dma_buf = buf;

    ctx->attachment = dma_buf_attach(ctx->dma_buf, fb_dev->dev);

    if (IS_ERR_OR_NULL(ctx->attachment)) {
        pr_err("%s Error: fail to dma_buf_attach()\n", __func__);
        ret = -EINVAL;
        goto err_attachment;
    }

    ctx->sg_table = dma_buf_map_attachment(ctx->attachment,
            DMA_BIDIRECTIONAL);

    if (IS_ERR_OR_NULL(ctx->sg_table)) {
        pr_err("%s Error: dma_buf_map_attachment()\n", __func__);
        ret = -EINVAL;
        goto err_map_attachment;
    }

    ctx->dma_addr = sg_phys(ctx->sg_table->sgl);
    ctx->virt     = sg_virt(ctx->sg_table->sgl);
    ctx->ion_handle = ion_handle;

    printk(KERN_INFO "%s.%d: dma addr = 0x%x, buf[0x%08x]\n",
        DEV_NAME_FB, fb_dev->device_id, ctx->dma_addr, (uint)buf);
    return 0;

err_map_attachment:
    dma_buf_detach(ctx->dma_buf, ctx->attachment);
    ctx->attachment = NULL;
err_attachment:
    return ret;
}

static void nxp_fb_free_dma_buf(struct nxp_fb_device *fb_dev,
        struct nxp_fb_dma_buf_data *d)
{
    int i;
    struct dma_buf_context *ctx;

    for (i = 0, ctx = &d->context[0]; i < fb_dev->buffer_num; i++, ctx++) {
        if (!ctx->dma_addr)
            continue;

        dma_buf_unmap_attachment(ctx->attachment, ctx->sg_table,
                DMA_BIDIRECTIONAL);

        ctx->dma_addr = 0;
        ctx->virt     = NULL;
        ctx->sg_table = NULL;

        dma_buf_detach(ctx->dma_buf, ctx->attachment);

        ctx->attachment = NULL;
        dma_buf_put(ctx->dma_buf);

        ctx->dma_buf = NULL;
        ion_free(d->ion_client, ctx->ion_handle);

        ctx->ion_handle = NULL;
    }
}

static int nxp_fb_ion_alloc_mem(struct nxp_fb_device *fb_dev)
{
    int ret;
    struct dma_buf *buf;
    unsigned int size;
    struct ion_handle *handle;
    struct nxp_fb_dma_buf_data *d = &fb_dev->dma_buf_data;
    struct dma_buf_context *ctx;
    int x_resol = fb_dev->x_resol_max ? : fb_dev->x_resol;
    int y_resol = fb_dev->y_resol_max ? : fb_dev->y_resol;
    int length = ((x_resol * y_resol * fb_dev->pixelbit)>>3);
    int i;

    size = PAGE_ALIGN(length);

    for (i = 0, ctx = &d->context[0]; i < fb_dev->buffer_num; i++, ctx++) {

        handle = ion_alloc(d->ion_client, (size_t)size, 0, ION_HEAP_NXP_CONTIG_MASK, 0);
        if (IS_ERR(handle)) {
            pr_err("%s Error: ion_alloc()\n", __func__);
            return -ENOMEM;
        }

        buf = ion_share_dma_buf(d->ion_client, handle);
        if (IS_ERR_OR_NULL(buf)) {
            pr_err("%s Error: fail to ion_share_dma_buf()\n", __func__);
            ret = -EINVAL;
            goto err_share_dma_buf;
        }

        ret = nxp_fb_map_ion_handle(fb_dev, ctx, handle, buf);
        if (ret) {
            pr_err("%s Error: nxp_fb_map_ion_handle()\n", __func__);
            goto err_map;
        }
        ctx->user_fd = -1;
    }
    return 0;

err_map:
    dma_buf_put(buf);
err_share_dma_buf:
    ion_free(d->ion_client, handle);

    return ret;
}
#endif // CONFIG_FB_NXP_ION_MEM

static int nxp_fb_alloc_mem(struct fb_info *info)
{
	struct nxp_fb_param  *par = info->par;
	struct nxp_fb_device *dev = &par->fb_dev;
    int x_resol = dev->x_resol_max ? : dev->x_resol;
    int y_resol = dev->y_resol_max ? : dev->y_resol;
	unsigned int length;

	length = ((x_resol * y_resol * dev->pixelbit)>>3) * dev->buffer_num;

	if (!length)
		return 0;

	pr_debug("%s: %s fb %d (%d * %d - %d bpp), len:%d[align:%d], fb_phys=0x%x...\n",
		__func__, dev_name(par->info->device), dev->device_id,
		x_resol, y_resol, dev->pixelbit, length, PAGE_ALIGN(length),
		dev->fb_phy_base);

#ifdef CONFIG_FB_NXP_ION_MEM
    if (nxp_fb_ion_alloc_mem(dev)) {
        printk(KERN_ERR "Fail to nxp_fb_ion_alloc_mem()\n");
        return -ENOMEM;
    }

    dev->fb_phy_base = dev->dma_buf_data.context[0].dma_addr;
    dev->fb_pan_phys = dev->fb_phy_base;
    dev->fb_phy_len  = PAGE_ALIGN(length);
    dev->fb_vir_base = dev->dma_buf_data.context[0].virt;
    dev->fb_remapped = 0;

#ifdef CONFIG_FRAMEBUFFER_CONSOLE
    dev->fb_vir_base = fb_copy_map(NULL, dev->fb_phy_base, dev->fb_phy_len);
#endif

#else
	if (dev->fb_phy_base) {
		/*
		 * remmap fb memory
		 */
		 if ((dev->fb_phy_base + length) > dev->fb_phy_end) {
		 	printk(KERN_ERR "Error: request fb mem 0x%x~0x%x execced fb region 0x%x~0x%x\n",
		 		dev->fb_phy_base, dev->fb_phy_base + length,
		 		dev->fb_phy_base, dev->fb_phy_end);
		 	return -ENOMEM;
		 }
		 dev->fb_vir_base = ioremap_nocache(dev->fb_phy_base, length);
		 dev->fb_pan_phys = dev->fb_phy_base;
		 dev->fb_phy_len  = PAGE_ALIGN(length);
		 dev->fb_remapped = 1;
	} else {
		struct device *device = par->info->device;
		/*
		 * allocate from system memory
		 */
		dev->fb_phy_len  = PAGE_ALIGN(length);
		dev->fb_vir_base = dma_alloc_coherent(device, dev->fb_phy_len,
								&dev->fb_phy_base, GFP_KERNEL);
		dev->fb_pan_phys = dev->fb_phy_base;
		dev->fb_remapped = 0;
	}
#endif
	if (dev->fb_vir_base) {
		info->screen_base = dev->fb_vir_base;
		info->fix.smem_start = dev->fb_phy_base;
	}

	return dev->fb_vir_base ? 0 : -ENOMEM;
}

static void nxp_fb_free_mem(struct fb_info *info)
{
	struct nxp_fb_param *par = info->par;
	struct nxp_fb_device *dev = &par->fb_dev;
	pr_debug("%s\n", __func__);

	if (dev->fb_vir_base) {
#ifdef CONFIG_FB_NXP_ION_MEM
#ifdef CONFIG_FRAMEBUFFER_CONSOLE
        fb_copy_unmap(NULL, dev->fb_vir_base);
#endif
        nxp_fb_free_dma_buf(dev, &dev->dma_buf_data);
#else
		if (dev->fb_remapped) {
			iounmap(dev->fb_vir_base);
		} else {
			struct device *device = par->info->device;
			dma_free_writecombine(device, dev->fb_phy_len,
					dev->fb_vir_base, dev->fb_phy_base);
		}
#endif
		dev->fb_vir_base = 0;
		dev->fb_remapped  = 0;
		dev->fb_phy_base = 0;
		dev->fb_phy_end = 0;
		dev->x_resol_max = 0;
		dev->y_resol_max = 0;
	}
}

static int nxp_fb_set_var_pixfmt(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct nxp_fb_param  *par = info->par;
	pr_debug("%s (bpp:%d)\n", __func__, var->bits_per_pixel);

	switch (var->bits_per_pixel) {
		case 1:
		case 2:
		case 4:
			var->red.offset    	= 0;
			var->red.length    	= var->bits_per_pixel;
			var->green         	= var->red;
			var->blue          	= var->red;
			var->transp.offset 	= 0;
			var->transp.length 	= 0;
			break;
		case 8:
			var->red.offset    	= 0;
			var->red.length    	= var->bits_per_pixel;
			var->green         	= var->red;
			var->blue          	= var->red;
			var->transp.offset 	= 0;
			var->transp.length 	= 0;
			break;
		case 16:
			/* 16 bpp, 565 format */
			var->red.length		= 5;
			var->red.offset		= 11;
			var->green.length	= 6;
			var->green.offset	= 5;
			var->blue.length	= 5;
			var->blue.offset	= 0;
			var->transp.length	= 0;
			par->fb_dev.format  = MLC_RGBFMT_R5G6B5;
			break;
		case 24:
			/* 24 bpp 888 */
			var->red.length		= 8;
			var->red.offset		= 16;
			var->green.length	= 8;
			var->green.offset	= 8;
			var->blue.length	= 8;
			var->blue.offset	= 0;
			var->transp.length	= 0;
			par->fb_dev.format  = MLC_RGBFMT_R8G8B8;
			break;
		case 32:
			/* 32 bpp 888 */
			var->red.length		= info->var.red.length;
			var->red.offset		= info->var.red.offset;
			var->green.length	= info->var.green.length;
			var->green.offset	= info->var.green.offset;
			var->blue.length	= info->var.blue.length;
			var->blue.offset	= info->var.blue.offset;
			var->transp.length	= info->var.transp.length;
			var->transp.offset	= info->var.transp.offset;
			par->fb_dev.format  = MLC_RGBFMT_A8R8G8B8;
			break;
		default:
			printk(KERN_ERR "Error, not support fb bpp (%d)\n", var->bits_per_pixel);
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------
 * 	FB ops functions
 */
/*
 *	nxp_fb_check_var:
 *	Get the video params out of 'var'. If a value doesn't fit, round it up,
 *	if it's too big, return -EINVAL.
 *
 */
static int nxp_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int ret = 0;
	pr_debug("%s (xres:%d, yres:%d, bpp:%d)\n",
		__func__, var->xres, var->yres, var->bits_per_pixel);

	ret = nxp_fb_verify_var(var, info);
	if (0 > ret)
		return ret;

	nxp_fb_set_var_pixfmt(var, info);
	return ret;
}

/*
 *	nxp_fb_set_par - Optional function. Alters the hardware state.
 *  @info: frame buffer structure that represents a single frame buffer
 *
 */
static int nxp_fb_set_par(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;
#ifdef SUPPORT_ALTER_HARDWARE_STATE
	struct nxp_fb_param  *par = info->par;
	struct nxp_fb_device *dev = &par->fb_dev;
	int ret;
#endif

	pr_debug("%s (xres:%d, yres:%d, bpp:%d)\n",
		__func__, var->xres, var->yres, var->bits_per_pixel);

	switch (var->bits_per_pixel) {
		case 32:
		case 24:
		case 16:
			info->fix.visual = FB_VISUAL_TRUECOLOR;
			break;
		case 1:
			info->fix.visual = FB_VISUAL_MONO01;
			break;
		default:
			info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
			break;
	}

	info->fix.line_length = (var->xres * var->bits_per_pixel) >> 3;
#ifdef SUPPORT_ALTER_HARDWARE_STATE
	info->fix.ypanstep  = 1;	/* when pan, check yoffset align with ypanstep value (fb_mem.c) */
    if (dev->x_resol != var->xres ||
        dev->y_resol != var->yres ||
        dev->pixelbit != var->bits_per_pixel) {
        printk("%s: resetting xres(%d) yres(%d) bps(%d)\n",
                __func__, var->xres, var->yres, var->bits_per_pixel);

		nxp_fb_free_mem(info);

        dev->x_resol     = var->xres;
        dev->y_resol     = var->yres;
        dev->pixelbit    = var->bits_per_pixel;
        dev->x_virt	     = dev->x_resol;
        dev->y_virt	     = dev->y_resol * dev->buffer_num;
        dev->fb_pan_phys = dev->fb_phy_base;	/* pan restore */

        nxp_fb_setup_info(info);
		ret = nxp_fb_alloc_mem(info);
		if (ret) {
			printk("Fail, unable to allcate frame buffer...\n");
			return -EINVAL;
		}
        nxp_fb_dev_setup(par);
    } else {
        if (par->status != FB_STAT_INIT) {
            nxp_fb_setup_info(info);
            nxp_fb_set_base(info);
        }
    }
#endif

#ifdef DUMP_VAR_SCREENINFO
	nxp_fb_dump_var(var);
#endif
	return 0;
}

/**
 *  nxp_fb_blank
 *	@blank_mode: the blank mode we want.
 *	@info: frame buffer structure that represents a single frame buffer
 *
 *	Blank the screen if blank_mode != 0, else unblank. Return 0 if
 *	blanking succeeded, != 0 if un-/blanking failed due to e.g. a
 *	video mode which doesn't support it. Implements VESA suspend
 *	and powerdown modes on hardware that supports disabling hsync/vsync:
 *	blank_mode == 2: suspend vsync
 *	blank_mode == 3: suspend hsync
 *	blank_mode == 4: powerdown
 *
 *	Returns negative errno on error, or zero on success.
 *
 */
static int nxp_fb_blank(int blank_mode, struct fb_info *info)
{
	struct nxp_fb_param  *par = info->par;
	int enable = 0;

	pr_debug("%s (blank_mode:%d)\n", __func__, blank_mode);

	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
	case FB_BLANK_NORMAL:
			enable = 1; break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_POWERDOWN:
			enable = 0; break;
	default:
			return -1;
	}

	return nxp_fb_dev_output(par, enable);
}

/**
 *	nxp_fb_setcolreg
 *	- framebuffer layer request to change palette.
 * 	@regno: The palette index to change.
 * 	@red: The red field for the palette data.
 * 	@green: The green field for the palette data.
 * 	@blue: The blue field for the palette data.
 * 	@trans: The transparency (alpha) field for the palette data.
 * 	@info: The framebuffer being changed.
 */
inline static unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static void schedule_palette_update(struct fb_info *info, unsigned int regno, unsigned int val)
{
	unsigned long flags;
	struct nxp_fb_param *par = info->par;

	local_irq_save(flags);
	par->palette_buffer[regno] = val;
	local_irq_restore(flags);
}

static int nxp_fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	struct nxp_fb_param *par = info->par;
	unsigned int val;

	/*
	pr_debug("%s (setcol: regno=%d, r=0x%x, g=0x%x, b=0x%x, t=0x%x)\n",
		__func__, regno, red, green, blue, transp);
	*/

	switch (par->info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseuo-palette */
		if (regno < 16) {
			u32 *pal = par->info->pseudo_palette;
			val  = chan_to_field(red,   &par->info->var.red);
			val |= chan_to_field(green, &par->info->var.green);
			val |= chan_to_field(blue,  &par->info->var.blue);
			pal[regno] = val;
		}
		break;
	case FB_VISUAL_PSEUDOCOLOR:
		if (regno < 256) {
			/* currently assume RGB 5-6-5 mode */
			val  = ((red   >>  0) & 0xf800);
			val |= ((green >>  5) & 0x07e0);
			val |= ((blue  >> 11) & 0x001f);
			schedule_palette_update(info, regno, val);
		}
		break;
	default:
		printk(KERN_ERR "Fail, setcolreg return unknown type\n");
		return 1;   /* unknown type */
	}
	return 0;
}

int nxp_fb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	/* not implementation */
	return 0;
}

static int nxp_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct nxp_fb_param  *par = info->par;
	struct nxp_fb_device *dev = &par->fb_dev;
	unsigned int offset = 0;
	unsigned int fb_old = dev->fb_pan_phys;
	unsigned int align  = (dev->x_resol * dev->y_resol * dev->pixelbit) >> 3;

	offset = (var->yoffset * info->fix.line_length) +
			 (var->xoffset * (var->bits_per_pixel>>3));
	if (offset % align)
		offset = (offset + align - 1)/align * align;

#ifdef CONFIG_FB_NXP_ION_MEM
    if (var->bits_per_pixel == 32) {
        unsigned int index = offset / align;
        dev->fb_pan_phys = dev->dma_buf_data.context[index].dma_addr;
    } else {
        dev->fb_pan_phys = info->fix.smem_start + offset;
    }
#else
	dev->fb_pan_phys = info->fix.smem_start + offset;
#endif

	pr_debug("%s (offset:0x%x, %s)\n",
		__func__, offset, fb_old!=dev->fb_pan_phys?"up":"pass");

	/* change window layer base */
	if (fb_old != dev->fb_pan_phys)
		nxp_fb_update_buffer(info, 1);

	return 0;
}

#define NXPFB_SET_POS	 	_IOW('N', 104, __u32)
#ifdef CONFIG_FB_NXP_ION_MEM
#define NXPFB_GET_FB_FD 	_IOWR('N', 101, __u32)
#define NXPFB_SET_FB_FD 	_IOW('N', 102, __u32)
#define NXPFB_GET_ACTIVE	_IOR('N', 103, __u32)
#endif

static int nxp_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	struct nxp_fb_param *par = info->par;
#ifdef CONFIG_FB_NXP_ION_MEM
	struct nxp_fb_device *dev = &par->fb_dev;
#endif	
    int ret = 0;
	pr_debug("%s (cmd:0x%x, type:%c, nr:%d) \n\n", __func__, cmd, _IOC_TYPE(cmd), _IOC_NR(cmd));

    switch (cmd) {
    case NXPFB_SET_POS:
        {
            int pos[3] = { 0, };	/* left, right, waitsycn */

            if (!access_ok(VERIFY_READ, arg, sizeof(pos)))
            	memcpy((void*)pos, (void*)arg, sizeof(pos));
            else {
            	if (copy_from_user(pos, (void __user *)arg, sizeof(pos))) {
                	ret = -EFAULT;
                	break;
                }
            }
            ret = nxp_fb_dev_pos(par, pos[0], pos[1], pos[2]);
        }
        break;
#ifdef CONFIG_FB_NXP_ION_MEM
    case NXPFB_GET_FB_FD:
        {
            int index;
            struct nxp_fb_dma_buf_data *d = &dev->dma_buf_data;
            int fd;

            if (get_user(index, (u32 __user *)arg)) {
                ret = -EFAULT;
                break;
            }
            pr_debug("%s: NXPFB_GET_FB_FD current %p, index(%d), client %p, handle %p\n",
            	__func__, current, index, d->ion_client, d->context[index].ion_handle);
            /*if (d->context[index].user_fd == 0) {*/
                fd = ion_share_dma_buf_fd(d->ion_client, d->context[index].ion_handle);
                if (fd < 0) {
                    printk("%s NXPFB_GET_FB_FD failed: Fail to dma_buf_fd()\n", __func__);
                    ret = -EINVAL;
                } else {
                    pr_debug("fd: %d\n", fd);
                    d->context[index].user_fd = fd;
                    if (put_user(d->context[index].user_fd, (int __user *)arg)) {
                        ret = -EFAULT;
                        break;
                    }
                }
            /*}*/
            pr_debug("success!!!\n");
        }
        break;
    case NXPFB_GET_ACTIVE:
	    {
	        unsigned int offset = dev->fb_pan_phys - dev->dma_buf_data.context[0].dma_addr;
	        unsigned int align  = (dev->x_resol * dev->y_resol * dev->pixelbit) >> 3;
	        unsigned int index = offset / align;
	        pr_debug("%s: NXPFB_GET_ACTIVE %d\n", __func__, index);
	        if (put_user(index, (int __user *)arg)) {
	            ret = -EFAULT;
	            break;
	        }
	        pr_debug("success!!!\n");
	    }
        break;
    case NXPFB_SET_FB_FD:
        {
	#if 0
            u32 import_fd;
            struct ion_handle *handle;
            struct nxp_fb_dma_buf_data dma_buf_data;
            struct nxp_fb_dma_buf_data *d = &dev->dma_buf_data;

            if (get_user(import_fd, (u32 __user *)arg)) {
                ret = -EFAULT;
                break;
            }

            handle = ion_import_dma_buf(d->ion_client, import_fd);
            if (IS_ERR(handle)) {
                pr_err("%s Error: NXPFB_SET_FB_FD, ion_import_dma_buf()\n", __func__);
                ret = PTR_ERR(handle);
                break;
            }

            if (nxp_fb_map_ion_handle(dev, &dma_buf_data, handle, import_fd)) {
                pr_err("%s Error: NXPFB_SET_FB_FD, nxp_fb_map_ion_handle()\n", __func__);
                ion_free(d->ion_client, handle);
                ret = -EINVAL;
                break;
            }

            nxp_fb_update_from_dma_buf_data(info, &dma_buf_data);
            nxp_fb_copy_dma_buf_data(dev, &dma_buf_data);
	#else
            u32 import_fd;
            struct nxp_fb_dma_buf_data *d;
            int i;
            if (get_user(import_fd, (u32 __user *)arg)) {
                ret = -EFAULT;
                break;
            }
            d = &dev->dma_buf_data;
            for (i = 0; i < 3; i++) {
                if (d->context[i].user_fd == import_fd)
                    break;
            }
            if (i >= 3) {
                printk("%s: can't find index for user fd %d\n", __func__, import_fd);
                ret = -EINVAL;
            } else {
                if (dev->fb_pan_phys != d->context[i].dma_addr) {
                    dev->fb_pan_phys = d->context[i].dma_addr;
                    nxp_fb_update_buffer(info, 1);
                }
            }
	#endif
        }
        break;
#endif
    }
	return ret;
}

#ifdef CONFIG_FB_NXP_ION_MEM
int nxp_fb_open(struct fb_info *info, int user)
{
    return 0;
}

int nxp_fb_release(struct fb_info *info, int user)
{
	#if 0
	struct nxp_fb_param *par = info->par;
	struct nxp_fb_device *dev = &par->fb_dev;
    struct nxp_fb_dma_buf_data *d = &dev->dma_buf_data;
	int i;
    for (i = 0; i < MAX_DMABUF_CONTEXT; i++) {
        if (d->context[i].user_fd > 0) {
            put_unused_fd(d->context[i].user_fd);
            d->context[i].user_fd = -1;
        }
    }
	#endif
    return 0;
}
#endif

static struct fb_ops nxp_fb_ops = {
	.owner			= THIS_MODULE,
#ifdef CONFIG_FB_NXP_ION_MEM
    .fb_open        = nxp_fb_open,
    .fb_release     = nxp_fb_release,
#endif
	.fb_check_var	= nxp_fb_check_var,
	.fb_set_par		= nxp_fb_set_par,
	.fb_blank		= nxp_fb_blank,
	.fb_setcolreg	= nxp_fb_setcolreg,
	.fb_cursor		= nxp_fb_cursor,		/* Optional !!! */
	.fb_pan_display	= nxp_fb_pan_display,
	.fb_ioctl		= nxp_fb_ioctl,
	/* Call FB function */
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/*--------------------------------------------------------------------------------
 * Platform driver
 */
static int nxp_fb_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct fb_info *info = platform_get_drvdata(pdev);
	return nxp_fb_dev_suspend(info->par);
}

static int nxp_fb_resume(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);
	return nxp_fb_dev_resume(info->par);
}

static int nxp_fb_probe(struct platform_device *pdev)
{
	struct nxp_fb_plat_data *plat = pdev->dev.platform_data;
	struct fb_info *info = NULL;
#ifdef CONFIG_FB_NXP_ION_MEM
    struct nxp_fb_device *fbdev;
	struct nxp_fb_param *fbpar;
#endif
	int i = 0, ret = 0;

	pr_debug("\n%s (name=%s, id=%d)\n", __func__, dev_name(&pdev->dev), pdev->id);

	/*	allocate fb_info and init */
	info = nxp_fb_init_fb(pdev->id, &pdev->dev);
	if (!info) {
		ret = -ENOMEM;
		goto err_fb;
	}

	ret = nxp_fb_setup_param(pdev->id, info, plat);
	if (0 > ret)
		goto err_map;

	nxp_fb_setup_info(info);

#ifdef CONFIG_FB_NXP_ION_MEM
	fbpar = info->par;
	fbdev = &fbpar->fb_dev;
	fbdev->dev = &pdev->dev;

    ret = nxp_fb_setup_ion(&fbpar->fb_dev.dma_buf_data);
	if (ret) {
        printk(KERN_ERR "Fail to setup ion\n");
        goto err_map;
    }
#endif

	/*	allocate frame buffer memory from here */
	ret = nxp_fb_alloc_mem(info);
	if (ret) {
		printk(KERN_ERR "Fail, unable to allcate frame buffer (%d)\n", pdev->id);
		goto err_map;
	}
	nxp_fb_init_display(info);

	/*
 	 * 	device_create '/proc/fb0' & fb class
	 * 	register machine file operation to frame buffer file operation
 	 * 	registered_fb[]
 	 * 	(drivers/video/fbmem.c)
 	 */
	if (pdev->id != 0) {
		for (i = 0; pdev->id > i; i++) {
			if (!registered_fb[i]) {
				printk("FB: Reserve dev/node [%d]\n", i);
				registered_fb[i] = info;
			}
		}
	}

	ret = register_framebuffer(info);
	if (ret < 0) {
		printk(KERN_ERR "Fail, unable to register frame buffer(%d)\n", pdev->id);
		goto err_reg;
	}

	nx_fb_init_sysfs(info);

	/* register to driver data, use platform_get_drvdata */
	platform_set_drvdata(pdev, info);
	return ret;

err_reg:
	unregister_framebuffer(info);
err_map:
	nxp_fb_free_mem(info);
err_fb:
	nxp_fb_exit_fb(info);

	return ret;
}

static int nxp_fb_remove(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);

	pr_debug("%s\n", __func__);

	unregister_framebuffer(info);
	nxp_fb_free_mem(info);
	nxp_fb_exit_fb(info);

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver fb_plat_driver = {
	.probe		= nxp_fb_probe,
	.remove		= __devexit_p(nxp_fb_remove),
	.suspend	= nxp_fb_suspend,
	.resume		= nxp_fb_resume,
	.driver		= {
		.name	= DEV_NAME_FB,
		.owner	= THIS_MODULE,
	},
};

static int __init nxp_fb_init(void)
{
	pr_debug("%s\n", __func__);
	return platform_driver_register(&fb_plat_driver);
}

static void __exit nxp_fb_exit(void)
{
	pr_debug("%s\n", __func__);
	platform_driver_unregister(&fb_plat_driver);
}

module_init(nxp_fb_init);
module_exit(nxp_fb_exit);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Framebuffer driver for the Nexell");
MODULE_LICENSE("GPL");

