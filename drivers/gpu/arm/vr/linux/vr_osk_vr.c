/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_vr.c
 * Implementation of the OS abstraction layer which is specific for the Vr kernel device driver
 */
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/vr/vr_utgard.h>

#include "vr_osk_vr.h"
#include "vr_kernel_common.h" /* VR_xxx macros */
#include "vr_osk.h"           /* kernel side OS functions */
#include "vr_kernel_linux.h"

_vr_osk_errcode_t _vr_osk_resource_find(u32 addr, _vr_osk_resource_t *res)
{
	int i;

	if (NULL == vr_platform_device) {
		/* Not connected to a device */
		return _VR_OSK_ERR_ITEM_NOT_FOUND;
	}

	for (i = 0; i < vr_platform_device->num_resources; i++) {
		if (IORESOURCE_MEM == resource_type(&(vr_platform_device->resource[i])) &&
		    vr_platform_device->resource[i].start == addr) {
			if (NULL != res) {
				res->base = addr;
				res->description = vr_platform_device->resource[i].name;

				/* Any (optional) IRQ resource belonging to this resource will follow */
				if ((i + 1) < vr_platform_device->num_resources &&
				    IORESOURCE_IRQ == resource_type(&(vr_platform_device->resource[i+1]))) {
					res->irq = vr_platform_device->resource[i+1].start;
				} else {
					res->irq = -1;
				}
			}
			return _VR_OSK_ERR_OK;
		}
	}

	return _VR_OSK_ERR_ITEM_NOT_FOUND;
}

u32 _vr_osk_resource_base_address(void)
{
	u32 lowest_addr = 0xFFFFFFFF;
	u32 ret = 0;

	if (NULL != vr_platform_device) {
		int i;
		for (i = 0; i < vr_platform_device->num_resources; i++) {
			if (vr_platform_device->resource[i].flags & IORESOURCE_MEM &&
			    vr_platform_device->resource[i].start < lowest_addr) {
				lowest_addr = vr_platform_device->resource[i].start;
				ret = lowest_addr;
			}
		}
	}

	return ret;
}

/* NEXELL_FEATURE_PORTING */
/* nexell add */	
#include <linux/fb.h> 

_vr_osk_errcode_t _vr_osk_device_data_get(struct _vr_osk_device_data *data)
{
	VR_DEBUG_ASSERT_POINTER(data);

	if (NULL != vr_platform_device) {
		struct vr_gpu_device_data* os_data = NULL;

		os_data = (struct vr_gpu_device_data*)vr_platform_device->dev.platform_data;
		if (NULL != os_data) {
			/* Copy data from OS dependant struct to Vr neutral struct (identical!) */
			data->dedicated_mem_start = os_data->dedicated_mem_start;
			data->dedicated_mem_size = os_data->dedicated_mem_size;
			data->shared_mem_size = os_data->shared_mem_size;
			#if 0 /* org */
			data->fb_start = os_data->fb_start;
			data->fb_size = os_data->fb_size;
			#else /* nexell add */	
			{				
				/* NEXELL_FEATURE_PORTING */
				unsigned long temp_fb_start[2] = {0,};
				unsigned long temp_fb_size[2] = {0,};
				unsigned char is_fb_used[2] = {0,};
				data->fb_start = 0;
				data->fb_size = 0;
				int fb_num;
				for(fb_num = 0 ; fb_num < 2 ; fb_num++)
				{
					struct fb_info *info = registered_fb[fb_num];
					if(info)
					{
						if(info->fix.smem_start && info->var.yres_virtual && info->var.yres && info->fix.smem_len)
						{
							is_fb_used[fb_num] = 1;
							temp_fb_start[fb_num] = info->fix.smem_start;
							temp_fb_size[fb_num] = (info->var.yres_virtual/info->var.yres) * info->fix.smem_len;
						}	
					}
				}
				if(is_fb_used[0] && !is_fb_used[1])
				{
					data->fb_start = temp_fb_start[0];
					data->fb_size = temp_fb_size[0];
				}
				else if(!is_fb_used[0] && is_fb_used[1])
				{
					data->fb_start = temp_fb_start[1];
					data->fb_size = temp_fb_size[1];
				}
				else if(is_fb_used[0] && is_fb_used[1])
				{
					if(temp_fb_start[0] < temp_fb_start[1])
					{
						data->fb_start = temp_fb_start[0]; 
						data->fb_size = temp_fb_start[1] - temp_fb_start[0] + temp_fb_size[1];
					}
					else
					{
						data->fb_start = temp_fb_start[1];
						data->fb_size = temp_fb_start[0] - temp_fb_start[1] + temp_fb_size[0];
					}						
				}	
				else
				{
					printk("################ ERROR! There is no available FB ###############\n");
					return _VR_OSK_ERR_ITEM_NOT_FOUND;
				}	
				
				if(!data->fb_start || !data->fb_size)	
				{
					printk("################ ERROR! wrong FB%d start(0x%x), size(0x%x)###############\n", is_fb_used[0]? 0 : 1, data->fb_start, data->fb_size);
					return _VR_OSK_ERR_ITEM_NOT_FOUND;
				}	
			}
			#endif
			data->max_job_runtime = os_data->max_job_runtime;
			data->utilization_interval = os_data->utilization_interval;
			data->utilization_callback = os_data->utilization_callback;
			data->pmu_switch_delay = os_data->pmu_switch_delay;
			data->set_freq_callback = os_data->set_freq_callback;

			memcpy(data->pmu_domain_config, os_data->pmu_domain_config, sizeof(os_data->pmu_domain_config));
			return _VR_OSK_ERR_OK;
		}
	}

	return _VR_OSK_ERR_ITEM_NOT_FOUND;
}

vr_bool _vr_osk_shared_interrupts(void)
{
	u32 irqs[128];
	u32 i, j, irq, num_irqs_found = 0;

	VR_DEBUG_ASSERT_POINTER(vr_platform_device);
	VR_DEBUG_ASSERT(128 >= vr_platform_device->num_resources);

	for (i = 0; i < vr_platform_device->num_resources; i++) {
		if (IORESOURCE_IRQ & vr_platform_device->resource[i].flags) {
			irq = vr_platform_device->resource[i].start;

			for (j = 0; j < num_irqs_found; ++j) {
				if (irq == irqs[j]) {
					return VR_TRUE;
				}
			}

			irqs[num_irqs_found++] = irq;
		}
	}

	return VR_FALSE;
}
