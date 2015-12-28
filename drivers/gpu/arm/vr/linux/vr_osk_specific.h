/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2010, 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_osk_specific.h
 * Defines per-OS Kernel level specifics, such as unusual workarounds for
 * certain OSs.
 */

#ifndef __VR_OSK_SPECIFIC_H__
#define __VR_OSK_SPECIFIC_H__

#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/dmapool.h>
#include <linux/gfp.h>
#include <linux/hardirq.h>

#include "vr_osk_types.h"
#include "vr_kernel_linux.h"

#define VR_STATIC_INLINE static inline
#define VR_NON_STATIC_INLINE inline

typedef struct dma_pool * vr_dma_pool;


VR_STATIC_INLINE vr_dma_pool vr_dma_pool_create(u32 size, u32 alignment, u32 boundary)
{
	return dma_pool_create("vr-dma", &vr_platform_device->dev, size, alignment, boundary);
}

VR_STATIC_INLINE void vr_dma_pool_destroy(vr_dma_pool pool)
{
	dma_pool_destroy(pool);
}

VR_STATIC_INLINE vr_io_address vr_dma_pool_alloc(vr_dma_pool pool, u32 *phys_addr)
{
	return dma_pool_alloc(pool, GFP_KERNEL, phys_addr);
}

VR_STATIC_INLINE void vr_dma_pool_free(vr_dma_pool pool, void* virt_addr, u32 phys_addr)
{
	dma_pool_free(pool, virt_addr, phys_addr);
}


#if VR_ENABLE_CPU_CYCLES
/* Reads out the clock cycle performance counter of the current cpu.
   It is useful for cost-free (2 cycle) measuring of the time spent
   in a code path. Sample before and after, the diff number of cycles.
   When the CPU is idle it will not increase this clock counter.
   It means that the counter is accurate if only spin-locks are used,
   but mutexes may lead to too low values since the cpu might "idle"
   waiting for the mutex to become available.
   The clock source is configured on the CPU during vr module load,
   but will not give useful output after a CPU has been power cycled.
   It is therefore important to configure the system to not turn of
   the cpu cores when using this functionallity.*/
static inline unsigned int vr_get_cpu_cyclecount(void)
{
	unsigned int value;
	/* Reading the CCNT Register - CPU clock counter */
	asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value));
	return value;
}

void vr_init_cpu_time_counters(int reset, int enable_divide_by_64);
#endif


VR_STATIC_INLINE u32 _vr_osk_copy_from_user(void *to, void *from, u32 n)
{
	return (u32)copy_from_user(to, from, (unsigned long)n);
}

VR_STATIC_INLINE vr_bool _vr_osk_in_atomic(void)
{
	return in_atomic();
}

#define _vr_osk_put_user(x, ptr) put_user(x, ptr)

#endif /* __VR_OSK_SPECIFIC_H__ */
