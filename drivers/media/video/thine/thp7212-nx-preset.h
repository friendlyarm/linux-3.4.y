
#ifndef __THP7212_NX_PRESET_H__
#define __THP7212_NX_PRESET_H__

struct reg_val {
    unsigned short reg;
    unsigned short val;
};

u8 ram_7210_elf_s19_1[] = {
	0x00, 0x00,
//	#include "thp7212_fw_01.txt"
	#include "3H_fw_F4_04_01_01.1.txt"
};

u8 ram_7210_elf_s19_2[] = {
	0x00, 0x00,
//	#include "thp7212_fw_02.txt"
	#include "3H_fw_F4_04_01_01.2.txt"
};

u8 ram_7210_elf_s19_3[] = {
	0x00, 0x00,
//	#include "thp7212_fw_03.txt"
	#include "3H_fw_F4_04_01_01.3.txt"
};

#endif

