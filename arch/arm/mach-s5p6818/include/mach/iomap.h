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
#ifndef __ASM_ARCH_IOMAP_H
#define __ASM_ARCH_IOMAP_H

#ifdef	PB_IO_MAP
#undef	PB_IO_MAP
#endif

#ifdef __ASSEMBLY__

#define PB_IO_MAP(_n_, _v_, _p_, _s_, _t_)			\
	.equ	__PB_IO_MAP_ ## _n_ ## _PHYS,	_p_;	\
	.equ	__PB_IO_MAP_ ## _n_ ## _VIRT,	_v_;	\

#else	/* __ASSEMBLY__ */

#define PB_IO_MAP(_n_, _v_, _p_, _s_, _t_)		\
	enum { 										\
		__PB_IO_MAP_ ## _n_ ## _VIRT	= _v_,	\
		__PB_IO_MAP_ ## _n_ ## _PHYS	= _p_,	\
		};

#endif	/* ! __ASSEMBLY__ */

#include <mach/s5p6818_iomap.h>

#endif	/* __ASM_ARCH_IOMAP_H */

