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

#include <linux/version.h>
#include <mach/platform.h>

	.macro  addruart, rp, rv, tmp

#if defined(CONFIG_DEBUG_LL) && !defined(CONFIG_DEBUG_LL_UART_NONE)
	/* If we move the adress using MMU, use this. */
	ldr	\rp, = PB_UART_PHYS_BASE		@ physical address
	ldr	\rv, = PB_UART_VIRT_BASE 		@ virtual address
#endif
	.endm

	#include <asm/hardware/debug-pl01x.S>
