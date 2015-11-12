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
#ifndef __MACH_GPIO_DESC_H__
#define __MACH_GPIO_DESC_H__

enum {
	ALT_NO_0 = 0,
	ALT_NO_1 = 1,
	ALT_NO_2 = 2,
	ALT_NO_3 = 3,
};

#define	ALT_NO_GPIO_A	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 	\

#define	ALT_NO_GPIO_B	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_2 , ALT_NO_2 , ALT_NO_1 , ALT_NO_2 , ALT_NO_1 ,	\
	  ALT_NO_2 , ALT_NO_1 , ALT_NO_2 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 ,	\
	  ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1		\

#define	ALT_NO_GPIO_C	\
	  ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 ,	\
	  ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 ,	\
	  ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 ,	\
	  ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1,  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0		\

#define	ALT_NO_GPIO_D	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0		\

#define	ALT_NO_GPIO_E	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1 , ALT_NO_1		\

#define	ALT_NO_ALIVE		\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 ,	\
	  ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0 , ALT_NO_0		\

#endif