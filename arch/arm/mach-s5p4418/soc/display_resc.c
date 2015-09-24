/*
 * (C) Copyright 2009
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
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <mach/platform.h>
#include <linux/platform_device.h>

#include <mach/devices.h>
#include <mach/soc.h>
#include "display_4418.h"

#if (0)
#define DBGOUT(msg...)		{ printk(KERN_INFO msg); }
#else
#define DBGOUT(msg...)		do {} while (0)
#endif
#define ERROUT(msg...)		{ printk(KERN_ERR msg); }

static int  resc_set_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
	RET_ASSERT_VAL(pdev && psync, -EINVAL);
	DBGOUT("%s: %s\n", __func__, dev_to_str(pdev->dev_id));

	pdev->status |= PROC_STATUS_READY;
	memcpy(&pdev->vsync, psync , sizeof(*psync));

	return 0;
}

static int resc_get_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
	printk("%s: %s\n", __func__, dev_to_str(pdev->dev_id));
	RET_ASSERT_VAL(pdev, -EINVAL);

	if (psync)
		memcpy(psync, &pdev->vsync, sizeof(*psync));

	return 0;
}

static int  resc_enable(struct disp_process_dev *pdev, int enable)
{
	DBGOUT("%s %s, %s\n", __func__, dev_to_str(pdev->dev_id), enable?"ON":"OFF");

	if (! enable) {
  		pdev->status &= ~PROC_STATUS_ENABLE;
	} else {
		pdev->status |=  PROC_STATUS_ENABLE;
  	}
  	return 0;
}

static int  resc_prepare(struct disp_process_dev *pdev)
{
	struct disp_vsync_info svs, dvs;
	enum disp_dev_type inp, out;

	inp = pdev->dev_in;
	out = pdev->dev_out;

	DBGOUT("%s: RESC [%s] link [%s]\n", __func__, dev_to_str(inp), dev_to_str(out));

	nxp_soc_disp_device_get_vsync_info(inp, &svs);
	nxp_soc_disp_device_get_vsync_info(out, &dvs);

	DBGOUT("%s: IN  (x=%4d, hfp=%3d, hbp=%3d, hsw=%3d), (y=%4d, vfp=%3d, vbp=%3d, vsw=%3d)\n",
		__func__, svs.h_active_len, svs.h_front_porch, svs.h_back_porch, svs.h_sync_width,
		svs.v_active_len, svs.v_front_porch, svs.v_back_porch, svs.v_sync_width);
	DBGOUT("%s: OUT (x=%4d, hfp=%3d, hbp=%3d, hsw=%3d), (y=%4d, vfp=%3d, vbp=%3d, vsw=%3d)\n",
		__func__, dvs.h_active_len, dvs.h_front_porch, dvs.h_back_porch, dvs.h_sync_width,
		dvs.v_active_len, dvs.v_front_porch, dvs.v_back_porch, dvs.v_sync_width);

	return 0;
}

static int  resc_stat_enable(struct disp_process_dev *pdev)
{
	DBGOUT("%s\n", __func__);
	return 0;
}

static int  resc_suspend(struct disp_process_dev *pdev)
{
	PM_DBGOUT("%s\n", __func__);
	return resc_enable(pdev, 0);
}

static void resc_resume(struct disp_process_dev *pdev)
{
	PM_DBGOUT("%s\n", __func__);
	resc_enable(pdev, 1);
}

static struct disp_process_ops resc_ops = {
	.set_vsync 	= resc_set_vsync,
	.get_vsync  = resc_get_vsync,
	.prepare 	= resc_prepare,
	.enable 	= resc_enable,
	.stat_enable= resc_stat_enable,
	.suspend	= resc_suspend,
	.resume	  	= resc_resume,
};

static int resc_probe(struct platform_device *pdev)
{
	struct nxp_lcd_plat_data *plat = pdev->dev.platform_data;
	struct disp_resc_param *prsc = (struct disp_resc_param *)plat->dev_param;
	int device = DISP_DEVICE_RESCONV;
	int input;

	RET_ASSERT_VAL(plat, -EINVAL);
	RET_ASSERT_VAL(plat->display_in == DISP_DEVICE_SYNCGEN0 ||
				   plat->display_in == DISP_DEVICE_SYNCGEN1 ||
				   plat->display_dev == DISP_DEVICE_RESCONV ||
				   plat->display_in == DISP_DEVICE_RESCONV, -EINVAL);
	RET_ASSERT_VAL(plat->vsync, -EINVAL);

	prsc = kzalloc(sizeof(*prsc), GFP_KERNEL);
	RET_ASSERT_VAL(prsc, -EINVAL);

	if (plat->dev_param)
		memcpy(prsc, plat->dev_param, sizeof(*prsc));

	input = plat->display_in;

	nxp_soc_disp_register_proc_ops(device, &resc_ops);
	nxp_soc_disp_device_connect_to(device, input, NULL);
	nxp_soc_disp_device_set_dev_param(device, prsc);

	printk("RESC: [%d]=%s connect to [%d]=%s\n",
		device, dev_to_str(device), input, dev_to_str(input));

	return 0;
}

static struct platform_driver resc_driver = {
	.driver	= {
	.name	= DEV_NAME_RESCONV,
	.owner	= THIS_MODULE,
	},
	.probe	= resc_probe,
};
module_platform_driver(resc_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Display RGB driver for the Nexell");
MODULE_LICENSE("GPL");
