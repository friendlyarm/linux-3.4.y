/*
 * include/linux/platform_data/ctouch.h
 *
 * Copyright (C) 2015 FriendlyARM (www.arm9.net)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __PLAT_CTOUCH_H__
#define __PLAT_CTOUCH_H__

enum {
	CTP_NONE = 0,
	CTP_AUTO,
	CTP_FT5X06,
	CTP_ITE7260,
	CTP_MAX
};

#if defined(CONFIG_PLAT_S5P4418_NANOPI2)
extern unsigned int nanopi2_get_ctp(void);
extern void nanopi2_set_ctp(int type);

static inline int board_get_ctp(void) {
	return nanopi2_get_ctp();
}

static inline void board_set_ctp(int type) {
	nanopi2_set_ctp(type);
}

#elif defined(CONFIG_PLAT_S5P6818_NANOPI3)
extern unsigned int nanopi3_get_ctp(void);
extern void nanopi3_set_ctp(int type);

static inline int board_get_ctp(void) {
	return nanopi3_get_ctp();
}

static inline void board_set_ctp(int type) {
	nanopi3_set_ctp(type);
}

#else

/* Stubs */
#define board_get_ctp()		(CTP_AUTO)
#define board_set_ctp(type)	\
	do { } while (0)

#endif

#endif	// __PLAT_CTOUCH_H__

