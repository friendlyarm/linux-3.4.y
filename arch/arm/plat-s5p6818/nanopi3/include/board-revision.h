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
#define CFG_IO_HW_PCBD	(PAD_GPIO_E + 13)


/* Board revision list: <PCB3 | PCB2 | PCB1>
 *  0b001 - NanoPC-T3
 *  0b100 - NanoPC-T3 Trunk
 *  0b101 - NanoPi 3 Fire
 *  0b110 - SOM6818
 *  0b111 - NanoPi M3
 *  0b010 - NanoPi M3B
 *  0b011 - Smart6818
 */

extern int board_get_revision(void);

#define board_is_nanopc()	(board_get_revision() == 0x01)
#define board_is_t3trunk()	(board_get_revision() == 0x04)
#define board_is_fire()		(board_get_revision() == 0x05)
#define board_is_som6818()	(board_get_revision() == 0x06)
#define board_is_M3()		(board_get_revision() == 0x07)
#define board_is_M3B()		(board_get_revision() == 0x02)
#define board_is_smart6818()	(board_get_revision() == 0x03)


static inline int board_with_emmc(void) {
	return (board_is_nanopc() || board_is_M3B() ||
			board_is_t3trunk() || board_is_smart6818() || board_is_som6818());
}

static inline int board_with_ap6212(void) {
	return (board_is_nanopc() || board_is_M3() || board_is_M3B() ||
			board_is_smart6818());
}

static inline int board_with_es8316(void) {
	return (board_is_nanopc() || board_is_M3() || board_is_M3B() ||
			board_is_t3trunk() || board_is_smart6818() || board_is_som6818());
}

static inline int board_with_gmac_eth(void) {
	return 1;
}

static inline int board_get_ads7846_CS(void) {
	if (board_is_M3() || board_is_fire())
		return (PAD_GPIO_B + 26);
	else
		return -1;
}

#endif /* __BOARD_REVISION_H__ */

