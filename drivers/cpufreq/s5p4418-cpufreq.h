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
#ifndef __S5P4418_ASV_H__
#define __S5P4418_ASV_H__

extern void nxp_cpu_id_ecid(u32 ecid[4]);
extern void nxp_cpu_id_string(u32 string[12]);

#define	CPU_ID_S5P4418		(0xE4418000)
#define	VOLTAGE_STEP_UV		(12500)	/* 12.5 mV */
#define	ASV_DEFAULT_LEVEL	(0)		/* for 4330 */

#define ASV_TABLE_COND(id)	(id == CPU_ID_S5P4418)

/*
 *	=============================================================================
 * 	|	ASV Group	|	ASV0	|	ASV1	|	ASV2	|	ASV3	|	ASV4	|
 *	-----------------------------------------------------------------------------
 * 	|	ARM_IDS		|	<= 10mA	|	<= 15mA	|	<= 20mA	|	<= 50mA	|	> 50mA	|
 *	-----------------------------------------------------------------------------
 * 	|	ARM_RO		|	<= 110	|	<= 130	|	<= 140	|	<= 170 	|	170 >	|
 *	=============================================================================
 * 	|	EMA_CPU		|	EMA:001	|	EMA:001	|	EMA:001	|	EMA:011	|	EMA:011	|
 *	=============================================================================
 * 	|  0: 1400 MHZ	|	1,350 mV|	1,300 mV|	1,250 mV|	1,200 mV|	1,175 mV|
 *	-----------------------------------------------------------------------------
 * 	|  1: 1300 MHZ	|	1,300 mV|	1,250 mV|	1,200 mV|	1,150 mV|	1,100 mV|
 *	-----------------------------------------------------------------------------
 * 	|  2: 1200 MHZ	|	1,250 mV|	1,200 mV|	1,150 mV|	1,100 mV|	1,050 mV|
 *	-----------------------------------------------------------------------------
 * 	|  3: 1100 MHZ	|	1,200 mV|	1,150 mV|	1,100 mV|	1,050 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  4: 1000 MHZ	|	1,175 mV|	1,125 mV|	1,075 mV|	1,025 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  5: 900 MHZ	|	1,150 mV|	1,100 mV|	1,050 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  6: 800 MHZ	|	1,125 mV|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  7: 700 MHZ	|	1,100 mV|	1,050 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  8: 6500 MHZ	|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  9: 500 MHZ	|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	| 10: 400 MHZ	|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	=============================================================================
 */

#define	FREQ_MAX_FREQ_KHZ	(1400*1000)
#define	FREQ_ARRAY_SIZE		(13)
#define	UV(v)				(v*1000)

struct asv_tb_info {
	int ids;
	int ro;
	long Mhz[FREQ_ARRAY_SIZE];
	long uV [FREQ_ARRAY_SIZE];
};

#define	ASB_FREQ_MHZ {	\
	[ 0] = 1600,	\
	[ 1] = 1500,	\
	[ 2] = 1400,	\
	[ 3] = 1300,	\
	[ 4] = 1200,	\
	[ 5] = 1100,	\
	[ 6] = 1000,	\
	[ 7] =  900,	\
	[ 8] =  800,	\
	[ 9] =  700,	\
	[10] =  600,	\
	[11] =  500,	\
	[12] =  400,	\
	}

static struct asv_tb_info asv_tables[] = {
	[0] = {	.ids = 10, .ro = 110,
			.Mhz = ASB_FREQ_MHZ,
			.uV  = { UV(1350), UV(1350),	/* OVER FREQ */
					 UV(1350), UV(1300), UV(1250), UV(1200), UV(1175), UV(1150),
					 UV(1125), UV(1100), UV(1075), UV(1075), UV(1075) },
	},
	[1] = {	.ids = 15, .ro = 130,
			.Mhz = ASB_FREQ_MHZ,
			.uV  = { UV(1300), UV(1300),	/* OVER FREQ */
				     UV(1300), UV(1250), UV(1200), UV(1150), UV(1125), UV(1100),
					 UV(1075), UV(1050), UV(1025), UV(1025), UV(1025) },
	},
	[2] = {	.ids = 20, .ro = 140,
			.Mhz = ASB_FREQ_MHZ,
			.uV  = { UV(1250), UV(1250),	/* OVER FREQ */
					 UV(1250), UV(1200), UV(1150), UV(1100), UV(1075), UV(1050),
					 UV(1025), UV(1000), UV(1000), UV(1000), UV(1000) },
	},
	[3] = {	.ids = 50, .ro = 170,
			.Mhz = ASB_FREQ_MHZ,
			.uV  = { UV(1200), UV(1200),	/* OVER FREQ */
					 UV(1200), UV(1150), UV(1100), UV(1050), UV(1025), UV(1000),
					 UV(1000), UV(1000), UV(1000), UV(1000), UV(1000) },
	},
	[4] = {	.ids = 50, .ro = 170,
			.Mhz = ASB_FREQ_MHZ,
			.uV  = { UV(1175), UV(1175),	/* OVER FREQ */
					 UV(1175), UV(1100), UV(1050), UV(1000), UV(1000), UV(1000),
					 UV(1000), UV(1000), UV(1000), UV(1000), UV(1000) },
	},
};
#define	ASV_ARRAY_SIZE	ARRAY_SIZE(asv_tables)

struct asv_param {
	int level;
	int ids, ro;
	int flag, group, shift;
};

static struct asv_tb_info *pAsv_Table = NULL;
static struct asv_param	Asv_Param = { 0, };

static inline unsigned int MtoL(unsigned int data, int bits)
{
	unsigned int result = 0;
	unsigned int mask = 1;
	int i = 0;
	for (i = 0; i<bits ; i++) {
		if (data&(1<<i))
			result |= mask<<(bits-i-1);
	}
	return result;
}

static int s5p4418_asv_setup_table(unsigned long (*freq_tables)[2])
{
	unsigned int ecid[4] = { 0, };
	unsigned int string[12] = { 0, };
	int i, ids = 0, ro = 0;
	int idslv, rolv, asvlv;

	if (2 > nxp_cpu_version()) {
		asvlv = ASV_DEFAULT_LEVEL;
		goto asv_find;
	}

	nxp_cpu_id_string(string);
	nxp_cpu_id_ecid(ecid);

	/* Use Fusing Flags */
	if ((ecid[2] & (1<<0))) {
		int gs = MtoL((ecid[2]>>1) & 0x07, 3);
		int ag = MtoL((ecid[2]>>4) & 0x0F, 4);

		Asv_Param.level = (ag - gs);
		if (0 > Asv_Param.level)
		    Asv_Param.level = 0;

		Asv_Param.flag = 1;
		Asv_Param.group = ag;
		Asv_Param.shift = gs;
		pAsv_Table = &asv_tables[Asv_Param.level];
		printk("DVFS: ASV[%d] IDS(%dmA) Ro(%d), Fusing Shift(%d), Group(%d)\n",
			Asv_Param.level, pAsv_Table->ids, pAsv_Table->ro, gs, ag);
		goto asv_done;
	}

	/* Use IDS/Ro */
	ids = MtoL((ecid[1]>>16) & 0xFF, 8);
	ro  = MtoL((ecid[1]>>24) & 0xFF, 8);

	/* find IDS Level */
	for (i=0; (ASV_ARRAY_SIZE-1) > i; i++) {
		pAsv_Table = &asv_tables[i];
		if (pAsv_Table->ids >= ids)
			break;
	}
	idslv = ASV_ARRAY_SIZE != i ? i: (ASV_ARRAY_SIZE-1);

	/* find RO Level */
	for (i=0; (ASV_ARRAY_SIZE-1) > i; i++) {
		pAsv_Table = &asv_tables[i];
		if (pAsv_Table->ro >= ro)
			break;
	}
	rolv = ASV_ARRAY_SIZE != i ? i: (ASV_ARRAY_SIZE-1);

	/* find Lowest ASV Level */
	asvlv = idslv > rolv ? rolv: idslv;

asv_find:
	pAsv_Table = &asv_tables[asvlv];
	Asv_Param.level = asvlv;
	Asv_Param.ids = ids;
	Asv_Param.ro  = ro;
	printk("DVFS: ASV[%d] IDS(%dmA,%3d) Ro(%d,%3d)\n",
			Asv_Param.level, pAsv_Table->ids, ids, pAsv_Table->ro, ro);

asv_done:
	for (i=0; FREQ_ARRAY_SIZE > i; i++) {
		freq_tables[i][0] = pAsv_Table->Mhz[i] * 1000;	/* frequency */
		freq_tables[i][1] = pAsv_Table->uV [i];			/* voltage */
	}

	return FREQ_ARRAY_SIZE;
}

static long s5p4418_asv_get_voltage(long freqkhz)
{
	long uV = 0;
	int i = 0;

	if (NULL == pAsv_Table)
		return -EINVAL;

	for (i = 0; FREQ_ARRAY_SIZE > i; i++) {
		if (freqkhz == (pAsv_Table->Mhz[i]*1000)) {
			uV = pAsv_Table->uV[i];
			break;
		}
	}

	if (0 == uV) {
		printk("FAIL: %ldkhz is not exist on the ASV TABLEs !!!\n", freqkhz);
		return -EINVAL;
	}

	return uV;
}

static int s5p4418_asv_modify_vol_table(unsigned long (*freq_tables)[2], int table_size,
				long value, bool down, bool percent)
{
	long step_vol = VOLTAGE_STEP_UV;
	long uV, dv, new;
	int i = 0, n = 0;

	if (NULL == freq_tables ||
		NULL == pAsv_Table || (0 > value))
		return -EINVAL;

	/* initialzie */
	for (i = 0; table_size > i; i++) {
		for (n = 0; FREQ_ARRAY_SIZE > n; n++) {
			if (freq_tables[i][0] == (pAsv_Table->Mhz[n]*1000)) {
				freq_tables[i][1] = pAsv_Table->uV[n];
				break;
			}
		}
	}
	printk("DVFS:%s%ld%s\n", down?"-":"+", value, percent?"%":"mV");

	/* new voltage */
	for (i = 0; table_size > i; i++) {
		int al = 0;
		uV = freq_tables[i][1];
		dv = percent ? ((uV/100) * value) : (value*1000);
		new = down ? uV - dv : uV + dv;

		if ((new % step_vol)) {
			new = (new / step_vol) * step_vol;
			al = 1;
			if (down) new += step_vol;	/* Upper */
		}

		printk("%7ldkhz, %7ld (%s%ld) align %ld (%s) -> %7ld\n",
			freq_tables[i][0], freq_tables[i][1],
			down?"-":"+", dv, step_vol, al?"X":"O", new);

		freq_tables[i][1] = new;
	}
	return 0;
}

static long s5p4418_asv_get_vol_margin(long uV, long value, bool down, bool percent)
{
	long step_vol = VOLTAGE_STEP_UV;
	long dv = percent ? ((uV/100) * value) : (value*1000);
	long new = down ? uV - dv : uV + dv;
	int al = 0;

	if (NULL == pAsv_Table)
		return -EINVAL;

	if ((new % step_vol)) {
		new = (new / step_vol) * step_vol;
		al = 1;
		if (down) new += step_vol;	/* Upper */
	}
	return new;
}

static int s5p4418_asv_current_label(char *buf)
{
	char *s = buf;

	if (NULL == pAsv_Table)
		return -EINVAL;

	if (s && pAsv_Table) {
		 if (!Asv_Param.flag) {
			s += sprintf(s, "%d:%dmA,%d\n",
					Asv_Param.level, Asv_Param.ids, Asv_Param.ro);
		} else {
			s += sprintf(s, "%d:G%d,S%d\n",
					Asv_Param.level, Asv_Param.group, Asv_Param.shift);
		}
	}
	return (s - buf);
}

static struct cpufreq_asv_ops asv_ops = {
	.setup_table = s5p4418_asv_setup_table,
	.get_voltage = s5p4418_asv_get_voltage,
	.modify_vol_table = s5p4418_asv_modify_vol_table,
	.current_label = s5p4418_asv_current_label,
	.get_vol_margin = s5p4418_asv_get_vol_margin,
};

#endif


