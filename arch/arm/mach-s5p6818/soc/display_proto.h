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

/*
 * display device type for binding
 */
#ifndef __DISPLAY_PROTO_H__
#define __DISPLAY_PROTO_H__

/* ADD: MLC/DPC  */
#define	NX_MLC_GetRGBLayerStride(m, l, hs, vs)	do { \
		unsigned int _hs, _vs;	\
		_hs = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCRGBLAYER[l].MLCHSTRIDE));	\
		_vs = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCRGBLAYER[l].MLCVSTRIDE));	\
		*(unsigned int*)hs = _hs, *(unsigned int*)vs = _vs;	\
		} while (0)

#define	NX_MLC_GetRGBLayerAddress(m, l, pa)	do {	\
		unsigned int _pa;	\
		_pa = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCRGBLAYER[l].MLCADDRESS));	\
		*(unsigned int*)pa = _pa;	\
		} while (0)

#define	NX_MLC_GetPosition(m, l, L, T, R, B) do {	\
		int _lr, _tb;	\
		_lr = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCRGBLAYER[l].MLCLEFTRIGHT));	\
		_tb = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCRGBLAYER[l].MLCTOPBOTTOM));	\
		*(unsigned int*)L = ((_lr >> 16) & 0xFFUL);	\
		*(unsigned int*)T = ((_tb >> 16) & 0xFFUL);	\
		*(unsigned int*)R = ((_lr >>  0) & 0xFFUL);	\
		*(unsigned int*)B = ((_tb >>  0) & 0xFFUL);	\
		} while (0)

#define	NX_MLC_GetVideoLayerAddressYUYV(m, a, s)	do { \
		unsigned int _a, _s;	\
		_a = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCADDRESS));	\
		_s = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCVSTRIDE));	\
		*(unsigned int*)a = _a, *(unsigned int*)s = _s;	\
		} while (0)


#define	NX_MLC_GetVideoLayerAddress(m, lua, cba, cra)do { \
		unsigned int _lua, _cba, _cra;	\
		_lua = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCADDRESS));	\
		_cba = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCADDRESSCB));	\
		_cra = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCADDRESSCR));	\
		*(unsigned int*)lua = _lua, *(unsigned int*)cba = _cba, *(unsigned int*)cra = _cra;	\
		} while (0)

#define	NX_MLC_GetVideoLayerStride(m, lus, cbs, crs)do { \
		unsigned int _lus, _cbs, _crs;	\
		_lus = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCVSTRIDE));	\
		_cbs = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCVSTRIDECB));	\
		_crs = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCVSTRIDECR));	\
		*(unsigned int*)lus = _lus, *(unsigned int*)cbs = _cbs, *(unsigned int*)crs = _crs;	\
		} while (0)

#define	NX_MLC_GetVideoPosition(m, L, T, R, B) do {	\
		int _lr, _tb;	\
		_lr = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCLEFTRIGHT));	\
		_tb = readl((volatile U32 *)&(((struct NX_MLC_RegisterSet*)IO_ADDRESS(NX_MLC_GetPhysicalAddress(m)))->MLCVIDEOLAYER.MLCTOPBOTTOM));	\
		*(unsigned int*)L = ((_lr >> 16) & 0xFFUL);	\
		*(unsigned int*)T = ((_tb >> 16) & 0xFFUL);	\
		*(unsigned int*)R = ((_lr >>  0) & 0xFFUL);	\
		*(unsigned int*)B = ((_tb >>  0) & 0xFFUL);	\
		} while (0)

#endif /* __DISPLAY_PROTO_H__ */
