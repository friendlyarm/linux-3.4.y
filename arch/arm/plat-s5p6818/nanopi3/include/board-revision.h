/*
 * include/mach/board-revision.h
 *
 * Copyright (C) 2016 FriendlyARM (www.friendlyarm.com)
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

#ifndef __BOARD_REVISION_H__
#define __BOARD_REVISION_H__


#define CFG_IO_HW_PCB1	(PAD_GPIO_C + 26)
#define CFG_IO_HW_PCB2	(PAD_GPIO_C + 27)
#define CFG_IO_HW_PCB3	(PAD_GPIO_C + 25)


/* Board revision list: <PCB3 | PCB2 | PCB1>
 *  0b001 - NanoPC-T3
 *  0b011 - Smart6818
 *  0b111 - NanoPi M3
 *  0b010 - NanoPi S3
 */
#define HWREV_B			0x1
#define HWREV_C			0x3
#define HWREV_G			0x7
#define HWREV_H			0x2


extern int board_get_revision(void);

#define board_is_nanopc()	(board_get_revision() == HWREV_B)
#define board_is_smart6818()	(board_get_revision() == HWREV_C)
#define board_is_M3()		(board_get_revision() == HWREV_G)
#define board_is_S3()		(board_get_revision() == HWREV_H)


static inline int board_with_ap6212(void) {
	return (board_is_nanopc() || board_is_M3() || board_is_S3());
}

static inline int board_with_es8316(void) {
	return (board_is_nanopc() || board_is_M3() || board_is_smart6818());
}

static inline int board_with_gmac_eth(void) {
	return !board_is_S3();
}

#endif /* __BOARD_REVISION_H__ */

