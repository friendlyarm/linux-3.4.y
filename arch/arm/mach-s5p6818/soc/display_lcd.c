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

#if (0)
#define DBGOUT(msg...)		{ printk(KERN_INFO msg); }
#else
#define DBGOUT(msg...)		do {} while (0)
#endif
#define ERROUT(msg...)		{ printk(KERN_ERR msg); }

static int lcd_set_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
	RET_ASSERT_VAL(pdev && psync, -EINVAL);
	DBGOUT("%s: %s\n", __func__, dev_to_str(pdev->dev_id));

	pdev->status |= PROC_STATUS_READY;
	memcpy(&pdev->vsync, psync , sizeof(*psync));

	return 0;
}

static int lcd_get_vsync(struct disp_process_dev *pdev, struct disp_vsync_info *psync)
{
	printk("%s: %s\n", __func__, dev_to_str(pdev->dev_id));
	RET_ASSERT_VAL(pdev, -EINVAL);

	if (psync)
		memcpy(psync, &pdev->vsync, sizeof(*psync));

	return 0;
}

static int  lcd_prepare(struct disp_process_dev *pdev)
{
	struct disp_lcd_param *plcd = pdev->dev_param;
	int input = pdev->dev_in;
	int mpu = plcd->lcd_mpu_type;
	int rsc = 0, sel = 0;

	switch (input) {
	case DISP_DEVICE_SYNCGEN0:	input = 0; break;
	case DISP_DEVICE_SYNCGEN1:	input = 1; break;
	case DISP_DEVICE_RESCONV  :	input = 2; break;
	default:
		return -EINVAL;
	}

	switch (input) {
	case 0:	sel = mpu ? 1 : 0; break;
	case 1:	sel = rsc ? 3 : 2; break;
	default:
		printk("Fail, %s nuknown module %d\n", __func__, input);
		return -1;
	}

	NX_DISPLAYTOP_SetPrimaryMUX(sel);
	return 0;
}

static int  lcd_enable(struct disp_process_dev *pdev, int enable)
{
	PM_DBGOUT("%s %s, %s\n", __func__, dev_to_str(pdev->dev_id), enable?"ON":"OFF");

	if (! enable) {
  		pdev->status &= ~PROC_STATUS_ENABLE;
	} else {
		lcd_prepare(pdev);
		pdev->status |=  PROC_STATUS_ENABLE;
  	}
  	return 0;
}

static int  lcd_stat_enable(struct disp_process_dev *pdev)
{
	CBOOL ret = CFALSE;

	switch (pdev->dev_in) {
	case DISP_DEVICE_SYNCGEN0: ret = NX_DPC_GetDPCEnable(0); break;
	case DISP_DEVICE_SYNCGEN1: ret = NX_DPC_GetDPCEnable(1); break;
	case DISP_DEVICE_RESCONV : break;
	default:
		break;
	}

	if (CTRUE == ret)
		pdev->status |=  PROC_STATUS_ENABLE;
	else
		pdev->status &= ~PROC_STATUS_ENABLE;

	return pdev->status & PROC_STATUS_ENABLE ? 1 : 0;
}

static int  lcd_suspend(struct disp_process_dev *pdev)
{
	PM_DBGOUT("%s\n", __func__);
	return lcd_enable(pdev, 0);
}

static void lcd_resume(struct disp_process_dev *pdev)
{
	PM_DBGOUT("%s\n", __func__);
	lcd_enable(pdev, 1);
}

static struct disp_process_ops lcd_ops = {
	.set_vsync 	= lcd_set_vsync,
	.get_vsync  = lcd_get_vsync,
	.enable 	= lcd_enable,
	.stat_enable= lcd_stat_enable,
	.suspend	= lcd_suspend,
	.resume	  	= lcd_resume,
};

static int lcd_probe(struct platform_device *pdev)
{
	struct nxp_lcd_plat_data *plat = pdev->dev.platform_data;
	struct disp_lcd_param *plcd;
	struct disp_vsync_info *psync;
	struct disp_syncgen_par *sgpar;
	int device = DISP_DEVICE_LCD;
	int input;

	RET_ASSERT_VAL(plat, -EINVAL);
	RET_ASSERT_VAL(plat->display_in == DISP_DEVICE_SYNCGEN0 ||
				   plat->display_in == DISP_DEVICE_SYNCGEN1 ||
				   plat->display_dev == DISP_DEVICE_LCD ||
				   plat->display_in == DISP_DEVICE_RESCONV, -EINVAL);
	RET_ASSERT_VAL(plat->vsync, -EINVAL);

	plcd = kzalloc(sizeof(*plcd), GFP_KERNEL);
	RET_ASSERT_VAL(plcd, -EINVAL);

	if (plat->dev_param)
		memcpy(plcd, plat->dev_param, sizeof(*plcd));

	sgpar = plat->sync_gen;
	psync = plat->vsync;
	input = plat->display_in;

	nxp_soc_disp_register_proc_ops(device, &lcd_ops);
	nxp_soc_disp_device_connect_to(device, input, psync);
	nxp_soc_disp_device_set_dev_param(device, plcd);

	if (sgpar &&
		(input == DISP_DEVICE_SYNCGEN0 ||
		 input == DISP_DEVICE_SYNCGEN1))
		nxp_soc_disp_device_set_sync_param(input, sgpar);

	printk("LCD : [%d]=%s connect to [%d]=%s\n",
		device, dev_to_str(device), input, dev_to_str(input));

	return 0;
}

static struct platform_driver lcd_driver = {
	.driver	= {
	.name	= DEV_NAME_LCD,
	.owner	= THIS_MODULE,
	},
	.probe	= lcd_probe,
};
module_platform_driver(lcd_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("Display RGB driver for the Nexell");
MODULE_LICENSE("GPL");
