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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/persistent_ram.h>
#include <linux/memblock.h>
#include "../../../drivers/staging/android/ram_console.h"

/*
 * Android Persistent Ram/Ram Console
 */
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
#ifdef CONFIG_ANDROID_PERSISTENT_RAM_CONS_AREA
#define PERSISTENT_RAM_AREA		CONFIG_ANDROID_PERSISTENT_RAM_CONS_AREA
#else
#define PERSISTENT_RAM_AREA		0x0	 // MB
#endif

#ifdef CONFIG_ANDROID_PERSISTENT_RAM_CONS_SIZE
#define PERSISTENT_RAM_SIZE		(CONFIG_ANDROID_PERSISTENT_RAM_CONS_SIZE * 1024)
#else
#define PERSISTENT_RAM_SIZE		SZ_16K
#endif

static struct persistent_ram_descriptor persistent_ram_desc = {
	.name = "ram_console",
    .size = PERSISTENT_RAM_SIZE,
};

static struct persistent_ram persistent_ram_data = {
    .descs 		= &persistent_ram_desc,
    .num_descs 	= 1,
    .start 		= PERSISTENT_RAM_AREA,
	.size  		= PERSISTENT_RAM_SIZE,
};

static char __ram_bootinfo[1024];
static struct ram_console_platform_data ram_console_data = {
	.bootinfo = __ram_bootinfo,
};

static struct platform_device ram_console_device = {
	.name	= "ram_console",
	.id     = -1,
    .dev    = {
        .platform_data = &ram_console_data,
    },
};

int __init persistent_ram_console_reserve(void)
{
	struct persistent_ram *ram = &persistent_ram_data;
	phys_addr_t start = ram->start, size = PAGE_ALIGN(ram->size);
	phys_addr_t new = start;

	if (!start || memblock_is_region_reserved(start, size)) {
		new = memblock_find_in_range(CFG_MEM_PHY_SYSTEM_BASE,
				MEMBLOCK_ALLOC_ACCESSIBLE, size, PAGE_SIZE);
	}
	ram->start = new;
	ram->size  = size;
	printk("rquest persistent memory from %08lx (%08lx)-%08lx\n",
			(long)new, (long)start, (long)size);

	persistent_ram_early_init(ram);
	return 0;
}

static __init int persistent_ram_console_init(void)
{
    return platform_device_register(&ram_console_device);
}

core_initcall(persistent_ram_console_init);
#endif