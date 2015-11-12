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
#ifndef __IOC_MAGIC_H__
#define __IOC_MAGIC_H__

/*	IOC MACRO  */
#ifndef _IO
#define	TYPE_SHIFT		8
#define _IO(type,nr)	((type<<TYPE_SHIFT) | nr)
#define _IOC_TYPE(nr)	((nr>>TYPE_SHIFT) & 0xFF)
#define _IOC_NR(nr)		(nr & 0xFF)
#endif

/*
 * 	_IOC_TYPE = 0x78 : 	IOC_NX_MAGIC & 0xFF
 */
#define	IOC_NX_MAGIC	0x6e78	/* "nx" */

#endif /* __IOC_MAGIC_H__ */
