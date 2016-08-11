/*
 * include/mach/board-revision.h
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

#ifndef __BOARD_REVISION_H__
#define __BOARD_REVISION_H__


#define CFG_IO_HW_PCB1	(PAD_GPIO_C + 26)
#define CFG_IO_HW_PCB2	(PAD_GPIO_C + 27)
#define CFG_IO_HW_PCB3	(PAD_GPIO_C + 25)
#define CFG_IO_HW_PCBD	(PAD_GPIO_E + 13)


/* Board revision list: <PCB3 | PCB2 | PCB1>
 *  0b000 - NanoPi 2
 *  0b001 - NanoPC-T2
 *  0b011 - Smart4418 (core)
 *  0b100 - NanoPi 2 Fire
 *  0b101 - NanoPi M2 (Hello)
 *  0b111 - NanoPi M2A
 *  0b010 - NanoPi S2
 *
 * Extented revision:
 *  0x103 - Smart4418 (SDK)
 */
#define HWREV_A			0x0
#define HWREV_B			0x1
#define HWREV_C			0x3
#define HWREV_D			0x4
#define HWREV_E			0x5
#define HWREV_F			0x6
#define HWREV_G			0x7
#define HWREV_H			0x2
#define HWREV_SDK		((1 << 8) | HWREV_C)


extern int board_get_revision(void);

#define board_is_nanopi()	(board_get_revision() == HWREV_A)
#define board_is_nanopc()	(board_get_revision() == HWREV_B)
#define board_is_smart4418()	(board_get_revision() == HWREV_C)
#define board_is_smart4418sdk()	(board_get_revision() == HWREV_SDK)
#define board_is_fire()		(board_get_revision() == HWREV_D)
#define board_is_M2()		(board_get_revision() == HWREV_E)
#define board_is_M2A()		(board_get_revision() == HWREV_G)
#define board_is_S2()		(board_get_revision() == HWREV_H)


static inline int board_with_ap6212(void) {
	return (board_is_nanopi() || board_is_nanopc() || board_is_S2() ||
			board_is_M2A() || board_is_smart4418sdk());
}

static inline int board_with_es8316(void) {
	return (board_is_nanopc() || board_is_S2() ||
			board_is_M2() || board_is_M2A() ||
			board_is_smart4418() || board_is_smart4418sdk());
}

static inline int board_with_rtl8211(void) {
	return !(board_is_nanopi() || board_is_S2());
}

#endif /* __BOARD_REVISION_H__ */

