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
#include <mach/serial.h>

    /* note, for the boot process to work we have to keep the UART
     * virtual address aligned to an 1MiB boundary for the L1
     * mapping the head code makes. We keep the UART virtual address
     * aligned and add in the offset when we load the value here.
     */

    .macro addruart, rp, rv, tmp
#if defined(CONFIG_DEBUG_LL) && !defined(CONFIG_DEBUG_LL_UART_NONE)
	ldr	\rp, = PB_UART_PHYS_BASE		@ physical address
	ldr	\rv, = PB_UART_VIRT_BASE 		@ virtual address
#endif
    .endm

#define fifo_full fifo_full_s5pv210
#define fifo_level fifo_level_s5pv210

	.macro fifo_level_s5pv210 rd, rx
		ldr	\rd, [\rx, # S3C2410_UFSTAT]
		and	\rd, \rd, #S5PV210_UFSTAT_TXMASK
	.endm

	.macro  fifo_full_s5pv210 rd, rx
		ldr	\rd, [\rx, # S3C2410_UFSTAT]
		tst	\rd, #S5PV210_UFSTAT_TXFULL
	.endm

	.macro	senduart,rd,rx
		strb 	\rd, [\rx, # S3C2410_UTXH]
	.endm

	.macro	busyuart, rd, rx
		ldr	\rd, [\rx, # S3C2410_UFCON]
		tst	\rd, #S3C2410_UFCON_FIFOMODE	@ fifo enabled?
		beq	1001f				@
		@ FIFO enabled...
1003:
		fifo_full \rd, \rx
		bne	1003b
		b	1002f

1001:
		@ busy waiting for non fifo
		ldr	\rd, [\rx, # S3C2410_UTRSTAT]
		tst	\rd, #S3C2410_UTRSTAT_TXFE
		beq	1001b

1002:		@ exit busyuart
	.endm

	.macro	waituart,rd,rx
		ldr	\rd, [\rx, # S3C2410_UFCON]
		tst	\rd, #S3C2410_UFCON_FIFOMODE	@ fifo enabled?
		beq	1001f				@
		@ FIFO enabled...
1003:
		fifo_level \rd, \rx
		teq	\rd, #0
		bne	1003b
		b	1002f
1001:
		@ idle waiting for non fifo
		ldr	\rd, [\rx, # S3C2410_UTRSTAT]
		tst	\rd, #S3C2410_UTRSTAT_TXFE
		beq	1001b

1002:		@ exit busyuart
	.endm

