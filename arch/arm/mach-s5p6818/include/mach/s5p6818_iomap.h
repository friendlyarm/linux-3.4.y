/*
 *	Length must be aligned 1MB
 *
 *	Refer to mach/iomap.h
 *
 *	Physical : __PB_IO_MAP_ ## _n_ ## _PHYS
 *	Virtual  : __PB_IO_MAP_ ## _n_ ## _VIRT
 *
 *	name	.virtual, 	.pfn, 		.length,	.type
 */
PB_IO_MAP( 	REGS, 	0xF0000000,	0xC0000000,	0x00300000,	MT_DEVICE )		/* NOMAL IO, Reserved */
PB_IO_MAP( 	CCI4, 	0xF0300000,	0xE0000000,	0x00100000,	MT_DEVICE )		/* CCI-400 */
PB_IO_MAP( 	SRAM, 	0xF0400000,	0xFFF00000,	0x00100000,	MT_DEVICE )		/* SRAM */
PB_IO_MAP( 	NAND, 	0xF0500000,	0x2C000000,	0x00100000,	MT_DEVICE )		/* NAND  */
PB_IO_MAP( 	IROM, 	0xF0600000,	0x00000000,	0x00100000,	MT_DEVICE )		/* IROM  */

