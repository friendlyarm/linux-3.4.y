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


/* Board revision list: <PCB3 | PCB2 | PCB1>
 *  0b000 - NanoPi 2
 *  0b001 - NanoPC-T2
 *  0b011 - Smart4418
 *  0b100 - NanoPi 2 Fire
 *  0b101 - NanoPi 2 Hello
 */
#define HWREV_A			0x0
#define HWREV_B			0x1
#define HWREV_C			0x3
#define HWREV_D			0x4
#define HWREV_E			0x5


extern int board_get_revision(void);

#define board_is_nanopi()	(board_get_revision() == HWREV_A)
#define board_is_nanopc()	(board_get_revision() == HWREV_B)
#define board_is_smart4418()	(board_get_revision() == HWREV_C)
#define board_is_fire()		(board_get_revision() == HWREV_D)
#define board_is_hello()	(board_get_revision() == HWREV_E)


static inline int board_with_ap6212(void) {
	return board_is_nanopi() || board_is_nanopc();
}

#endif /* __BOARD_REVISION_H__ */

