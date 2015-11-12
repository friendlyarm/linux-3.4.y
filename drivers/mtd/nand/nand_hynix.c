/*
 * Copyright (C) 2014 Boris BREZILLON <b.brezillon.dev@gmail.com>
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
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mtd/nand.h>
#include <linux/slab.h>

#define mlc_20nm_a_die		0
#define mlc_20nm_b_die		1
#define mlc_16nm_x_die		5


static u8 hynix_mlc_read_retry_regs[][8] = {
	[mlc_20nm_a_die] = 
		{ 0xcc, 0xbf, 0xaa, 0xab, 0xcd, 0xad, 0xae, 0xaf },
	[mlc_20nm_b_die] = 
		{ 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7 },
	[mlc_16nm_x_die] =
		{ 0x38, 0x39, 0x3a, 0x3b },
};

struct hynix_read_retry {
	u8 *regs;
	int regs_cnt;
	int retry_type;
	u8 values[64];
};

struct hynix_nand {
	struct hynix_read_retry read_retry;
};

static void nand_direct_command(struct mtd_info *mtd, unsigned int command,
			    int column, int page_addr)
{
	register struct nand_chip *chip = mtd->priv;

	if (column != -1 || page_addr != -1) {
		int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

		/* Serially input address */
		if (column != -1) {
			/* Adjust columns for 16 bit buswidth */
			chip->cmd_ctrl(mtd, column, ctrl);
			//ctrl &= ~NAND_CTRL_CHANGE;
			//chip->cmd_ctrl(mtd, column >> 8, ctrl);
		}
	}
}

int nand_setup_read_retry_hynix(struct mtd_info *mtd, int retry_mode)
{
	struct nand_chip *chip = mtd->priv;
	struct hynix_nand *hynix = chip->manuf_priv;
	int i;
	int regs_cnt = hynix->read_retry.regs_cnt;
	int offset = retry_mode * regs_cnt;


	chip->cmdfunc(mtd, 0x36, -1, -1);

	for (i = 0; i < regs_cnt; i++) {
		int column = hynix->read_retry.regs[i];
		nand_direct_command(mtd, NAND_CMD_NONE, column, -1);
		chip->write_byte(mtd, hynix->read_retry.values[offset + i]);
		//printk(" %02x", hynix->read_retry.values[offset + i]);	/* FIXME: remove debug msg */
	}
	//printk ("\n");												/* FIXME: remove debug msg */
	chip->cmdfunc(mtd, 0x16, -1, -1);

	ndelay(100);

#if 0
	/* read back */
	do {
		u8 arr[8] = { 0, };

		chip->cmdfunc(mtd, 0x37, -1, -1);

		for (i = 0; i < regs_cnt; i++) {
			int column = hynix->read_retry.regs[i];
			nand_direct_command(mtd, NAND_CMD_NONE, column, -1);
			arr[i] = chip->read_byte(mtd);
		}

		printk ("\tnew RR reg : ");
		for (i = 0; i < regs_cnt; i++)
			printk ("%02x,", arr[i]);
		printk ("\n");
	} while (0);
#endif

	return 0;
}

static void hynix_mlc_cleanup(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	kfree(chip->manuf_priv);
}


u8 rrt_read_buf[1024] = { 0, };

static int select_rrt_set_16nm(u8 *result)
{
	int set, origin, idx;
	int bytearr[8];
	int bitarr[8];
	u8 (*p)[2][32] = (u8 (*)[2][32])rrt_read_buf;


	for (idx = 0; idx < 32; idx++) {	// N
		int m;

		for (m = 0; m < 8; m++) {	// M
			int z_rep_max = 0, o_rep_max = 0;
			int z_rep = 0, o_rep = 0;
			int prev = -1;
			int i;

			origin = 0;
inverse:
			/* select bits from each byte */
			for (set = 0; set < 8; set++) {
				bytearr[set] = p[set][origin][idx];
				bitarr[set] = (bytearr[set] >> (7 - m)) & 1;
			}

			/* choice most repeated byte bit */
			for (i = 0, prev = bitarr[0]; i < 8; i++) {
				if (prev == bitarr[i]) {
					if (bitarr[i] & 1)
						o_rep++;
					else
						z_rep++;

					if (o_rep > o_rep_max)
						o_rep_max = o_rep;
					if (z_rep > z_rep_max)
						z_rep_max = z_rep;
				}
				else {
					o_rep = 1;
					z_rep = 1;
				}

				prev = bitarr[i];
			}

			if (o_rep_max <= 4 && z_rep_max <= 4) {
				if (origin)
					goto fail;

				origin ^= 1;
				goto inverse;
			}

			if (o_rep_max > z_rep_max)
				result[idx] |= 1 << (7 - m);
		}
	}
	

	/* debug */
	#if 0
	for (idx = 0; idx < 32; idx++) {
		printk ("0x%02x, ", result[idx]);
	}
	printk ("\n");
	#endif

	return 0;

fail:
	printk ("failed!\n");
	return -1;
}

static int hynix_mlc_get_rrt_value(struct mtd_info *mtd, struct hynix_nand *hynix)
{
	struct nand_chip *chip = mtd->priv;
	int rrt_total_len;
	int rrt_val_len;
	int retry_type = hynix->read_retry.retry_type;
	int i;
	int set = 0;
	int ret;


	chip->select_chip(mtd, 0);

	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	if (retry_type == mlc_16nm_x_die) {
		u8 rrt_buf[32] = { 0, };

		rrt_total_len = 512;
		rrt_val_len = 32;

		chip->cmdfunc(mtd, 0x36, 0x38, -1);
		chip->write_byte(mtd, 0x52);

		chip->cmdfunc(mtd, 0x16, -1, -1);
		chip->cmdfunc(mtd, 0x17, -1, -1);
		chip->cmdfunc(mtd, 0x04, -1, -1);
		chip->cmdfunc(mtd, 0x19, -1, -1);
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, 0x200);


		chip->read_buf(mtd, rrt_read_buf, 16);
		//if (rrt_read_buf[0] != 0x8 || rrt_read_buf[8] != 0x4)
		//	return -EINVAL;

		chip->read_buf(mtd, rrt_read_buf, rrt_total_len);

		chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

		/* debug */
		#if 0
		do {
			int j;
			for (j = 0; j < rrt_total_len; j++) {
				printk ("%02x ", rrt_read_buf[j]);
				if ((j+1) % 32 == 0)
					printk ("\n");
			}
		} while (0);
		#endif


		chip->cmdfunc(mtd, 0x36, 0x38, -1);
		chip->write_byte(mtd, 0x00);
		chip->cmdfunc(mtd, 0x16, -1, -1);
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, 0x200);

		/* select rrt set */
		ret = select_rrt_set_16nm(rrt_buf);
		if (!ret)
			memcpy(hynix->read_retry.values, rrt_buf, rrt_val_len);
	}
	else {	/* mlc_20nm_a_die, mlc_20nm_b_die */
		rrt_total_len = 1024;
		rrt_val_len = 64;

		if (retry_type == mlc_20nm_a_die) {
			chip->cmdfunc(mtd, 0x36, 0xff, -1);
			chip->write_byte(mtd, 0x40);
			chip->cmdfunc(mtd, NAND_CMD_NONE, 0xcc, -1);
		}
		else {
			chip->cmdfunc(mtd, 0x36, 0xae, -1);
			chip->write_byte(mtd, 0x00);
			chip->cmdfunc(mtd, NAND_CMD_NONE, 0xb0, -1);
		}
		chip->write_byte(mtd, 0x4d);

		chip->cmdfunc(mtd, 0x16, -1, -1);
		chip->cmdfunc(mtd, 0x17, -1, -1);
		chip->cmdfunc(mtd, 0x04, -1, -1);
		chip->cmdfunc(mtd, 0x19, -1, -1);
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, 0x200);

		chip->read_buf(mtd, rrt_read_buf, 2);
		//if (rrt_read_buf[0] != 0x8 || rrt_read_buf[1] != 0x8)
		//	return -EINVAL;
		chip->read_buf(mtd, rrt_read_buf, rrt_total_len);

		chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

		/* debug */
		#if 0
		do {
		int j;
		for (j = 0; j < rrt_total_len; j++) {
			printk ("%02x ", rrt_read_buf[j]);
			if ((j+1) % 32 == 0)
				printk ("\n");
		} while (0);
		#endif

		chip->cmdfunc(mtd, 0x38, -1, -1);

		/* select rrt set */
		ret = -EINVAL;
		for (set = 0; set < 8; set++) {
			u8 *origin, *inverse;

			origin = rrt_read_buf + (rrt_val_len * 2 * set);
			inverse = origin + rrt_val_len;

			for (i = 0; i < rrt_val_len; i++) {
				if ((origin[i] ^ inverse[i]) == 0xff) {
					ret = 0;
					/* debug */
					#if 0
					printk ("  found! set: %d\n", set);
					#endif
					break;
				}
			}

			if (!ret)
				break;
		}

		memcpy(hynix->read_retry.values, rrt_read_buf + (rrt_val_len * 2 * set), rrt_val_len);
	}

	chip->select_chip(mtd, -1);

	return ret;
}

static int hynix_mlc_init(struct mtd_info *mtd, const uint8_t *id, int retry_type)
{
	struct nand_chip *chip = mtd->priv;
	struct hynix_nand *hynix;
	int ret;


	hynix = kzalloc(sizeof(*hynix), GFP_KERNEL);
	if (!hynix)
		return -ENOMEM;


	if (retry_type == mlc_16nm_x_die) {
		hynix->read_retry.regs_cnt = 4;
		chip->need_save_read_retries = 1;
	}
	else {
		hynix->read_retry.regs_cnt = 8;
		chip->need_save_read_retries = 0;
	}
	hynix->read_retry.retry_type = retry_type;
	hynix->read_retry.regs = hynix_mlc_read_retry_regs[retry_type];
	ret = hynix_mlc_get_rrt_value (mtd, hynix);
	if (ret)
		return ret;


	chip->manuf_priv = hynix;
	chip->setup_read_retry = nand_setup_read_retry_hynix;
	chip->read_retries = 8;
	chip->read_retry_mode = 0;
	chip->manuf_cleanup = hynix_mlc_cleanup;


	ret = chip->setup_read_retry(mtd, 0);
	if (ret < 0)
		printk ("  %s: default read retry setup failed\n", __func__);

	return ret;
}


struct hynix_nand_initializer {
	u8 id[6];
	//int (*init)(struct mtd_info *mtd, const uint8_t *id);
	int retry_type;
};

struct hynix_nand_initializer initializers[] = {
	{
		.id = {NAND_MFR_HYNIX, 0xde, 0x94, 0xda, 0x74, 0xc4},	/* h27ucg8t2atr */
		.retry_type = mlc_20nm_a_die,
	},
	{
		.id = {NAND_MFR_HYNIX, 0xde, 0x94, 0xeb, 0x74, 0x44},	/* h27ucg8t2btr */
		.retry_type = mlc_20nm_b_die,
	},
	{
		.id = {NAND_MFR_HYNIX, 0xd7, 0x94, 0x91, 0x60, 0x44},	/* h27ubg8t2ctr */
		.retry_type = mlc_20nm_b_die,
	},
	{
		.id = {NAND_MFR_HYNIX, 0xd7, 0x14, 0x9E, 0x34, 0x4a},	/* h27ubg8t2dtr */
		.retry_type = mlc_16nm_x_die,
	},
	{
		.id = {NAND_MFR_HYNIX, 0xde, 0x14, 0xa7, 0x42, 0x4a},	/* h27ucg8t2etr */
		.retry_type = mlc_16nm_x_die,
	}
};

int hynix_nand_init(struct mtd_info *mtd, const uint8_t *id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(initializers); i++) {
		struct hynix_nand_initializer *initializer = &initializers[i];
		if (memcmp(id, initializer->id, sizeof(initializer->id)))
			continue;

	//	return initializer->init(mtd, id, initializer->retry_type);
		return hynix_mlc_init(mtd, id, initializer->retry_type);
	}

	return 0;
}
EXPORT_SYMBOL(hynix_nand_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Boris BREZILLON <b.brezillon.dev@gmail.com>");
MODULE_DESCRIPTION("Hynix NAND specific code");
