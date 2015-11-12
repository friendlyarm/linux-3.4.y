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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/delay.h>	/* mdelay */
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/pm.h>
#include <mach/s5p4418_bus.h>

//#define DEBUG_BUS_CONF

#if (0)	/* default on */
#define DBGOUT(msg...)		{ printk("cpu: " msg); }
#else
#define DBGOUT(msg...)		do {} while (0)
#endif

#if (CFG_BUS_RECONFIG_ENB == 1)
void nxp_set_bus_config(void)
{
    volatile NX_DREX_REG *pdrex = (volatile NX_DREX_REG *)NX_VA_BASE_REG_DREX;
	u32 val;
	u32 num_si, num_mi;
	u32 i_slot, temp;
#if ((CFG_DREX_PORT0_QOS_ENB == 1) || (CFG_DREX_PORT1_QOS_ENB == 1))
	u32 drex_qos_bits = 0;
#endif
	u16 __g_DrexQoS[2] = { g_DrexQoS[0], g_DrexQoS[1] };

#if (CFG_DREX_PORT0_QOS_ENB == 1)
	drex_qos_bits  |= (1<<4) | (1<<0);
#endif
#if (CFG_DREX_PORT1_QOS_ENB == 1)
	drex_qos_bits  |= (1<<12) | (1<<8);
#endif

#if 0
	writel( 0xFFF1FFF1,     &pdrex->BRBRSVCONFIG );
#else

	temp    = ( 0xFF00FF00
			| ((g_DrexBRB_WR[1] & 0xF) <<   20)
			| ((g_DrexBRB_WR[0] & 0xF) <<   16)
			| ((g_DrexBRB_RD[1] & 0xF) <<    4)
			| ((g_DrexBRB_RD[0] & 0xF) <<    0));
	writel( temp,           &pdrex->BRBRSVCONFIG );
#endif
	writel( 0x00000033,     &pdrex->BRBRSVCONTROL );
#ifdef DEBUG_BUS_CONF
	printk("  ... BRBRSVCONFIG [%x], BRBRSVCONTROL [%x]\n", (pdrex->BRBRSVCONFIG), (pdrex->BRBRSVCONTROL));
#endif

#if ((CFG_DREX_PORT0_QOS_ENB == 1) || (CFG_DREX_PORT1_QOS_ENB == 1))
#if (CFG_DREX_PORT0_QOS_ENB == 1)
	__g_DrexQoS[0] = (u16)0x0000;
#endif
#if (CFG_DREX_PORT1_QOS_ENB == 1)
	__g_DrexQoS[1] = (u16)0x0000;
#endif
	writel( drex_qos_bits,  NX_VA_BASE_REG_TIEOFF + NX_TIEOFF_DREX_SLAVE_OFFSET );
#ifdef DEBUG_BUS_CONF
	printk("  ... TIEOFF DREX SLAVE [%x]\n", *(U32 *)(NX_VA_BASE_REG_TIEOFF + NX_TIEOFF_DREX_SLAVE_OFFSET));
#endif
#endif

	/* ------------- DREX QoS -------------- */
#if 1   //(CFG_BUS_RECONFIG_DREXQOS == 1)
	for (i_slot = 0; i_slot < 2; i_slot++)
	{
		val = readl(NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET + (i_slot<<3));
		if (val != __g_DrexQoS[i_slot])
			writel( __g_DrexQoS[i_slot], (NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET + (i_slot<<3)) );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < 2; i_slot++)
	{
		val = readl(NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET + (i_slot<<3));
		printk("  ... DREX QoS slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif
#endif /* (CFG_BUS_RECONFIG_DREXQOS == 1) */

	/* ------------- Bottom BUS ------------ */
	/* MI1 - Set SI QoS */
#if (CFG_BUS_RECONFIG_BOTTOMBUSQOS == 1)
	val = readl(NX_BASE_REG_PL301_BOTT_QOS_TRDMARK + 0x20);
	if (val != g_BottomQoSSI[0])
		writel(g_BottomQoSSI[0], (NX_BASE_REG_PL301_BOTT_QOS_TRDMARK + 0x20) );

	val = readl(NX_BASE_REG_PL301_BOTT_QOS_CTRL + 0x20);
	if (val != g_BottomQoSSI[1])
		writel(g_BottomQoSSI[1], (NX_BASE_REG_PL301_BOTT_QOS_CTRL + 0x20) );
#ifdef DEBUG_BUS_CONF
	val = readl(NX_BASE_REG_PL301_BOTT_QOS_TRDMARK + 0x20);
	printk("  ... BOTT QOS TRDMARK : %x\n", val);
	val = readl(NX_BASE_REG_PL301_BOTT_QOS_CTRL + 0x20);
	printk("  ... BOTT QOS CTRL : %x\n", val);
#endif
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1)
	num_si = readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC4);

	/* Set progamming for AR */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_BOTT_AR );
		val = readl(NX_BASE_REG_PL301_BOTT_AR);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  NX_BASE_REG_PL301_BOTT_AR );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_BOTT_AR );
		val = readl(NX_BASE_REG_PL301_BOTT_AR);
		printk("  ... MI0 BOTT AR slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif

	// MI1 - Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AR + 0x20) );
		val = readl(NX_BASE_REG_PL301_BOTT_AR + 0x20);
		if (val != g_BottomBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_BottomBusSI[i_slot] << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_BOTT_AR + 0x20) );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AR + 0x20) );
		val = readl(NX_BASE_REG_PL301_BOTT_AR + 0x20);
		printk("  ... MI1 BOTT AR slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif

	/* Set progamming for AW */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_BOTT_AW );
		val = readl(NX_BASE_REG_PL301_BOTT_AW);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  NX_BASE_REG_PL301_BOTT_AW );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AW) );
		val = readl(NX_BASE_REG_PL301_BOTT_AW);
		printk("  ... MI0 BOTT AW slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif

	// MI1 - Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AW + 0x20) );
		val = readl(NX_BASE_REG_PL301_BOTT_AW + 0x20);
		if (val != g_BottomBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_BottomBusSI[i_slot] << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_BOTT_AW + 0x20) );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AW + 0x20) );
		val = readl(NX_BASE_REG_PL301_BOTT_AW + 0x20);
		printk("  ... MI1 BOTT AW slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif
#endif /* (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1) */

	/* ------------- Top BUS ------------ */
#if (CFG_BUS_RECONFIG_TOPBUSQOS == 1)
	/* MI0 - Set SI QoS */
	val = readl(NX_BASE_REG_PL301_TOP_QOS_TRDMARK);
	if (val != g_TopQoSSI[0])
		writel(g_TopQoSSI[0], NX_BASE_REG_PL301_TOP_QOS_TRDMARK);
#ifdef DEBUG_BUS_CONF
	val = readl(NX_BASE_REG_PL301_TOP_QOS_TRDMARK);
	printk(" ... TOP QOS TRDMARK : %x\n", val);
#endif

	val = readl(NX_BASE_REG_PL301_TOP_QOS_CTRL);
	if (val != g_TopQoSSI[1])
		writel(g_TopQoSSI[1], NX_BASE_REG_PL301_TOP_QOS_CTRL);
#ifdef DEBUG_BUS_CONF
	val = readl(NX_BASE_REG_PL301_TOP_QOS_CTRL);
	printk(" ... TOP QOS CTRL: %x\n", val);
#endif
#endif

#if (CFG_BUS_RECONFIG_TOPBUSSI == 1)
	num_si = readl(NX_VA_BASE_REG_PL301_TOP + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_TOP + 0xFC4);

	/* Set progamming for AR */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_TOP_AR );
		val = readl(NX_BASE_REG_PL301_TOP_AR);
		if (val != g_TopBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_TopBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_TOP_AR );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_TOP_AR );
		val = readl(NX_BASE_REG_PL301_TOP_AR);
		printk("  ... TOP AR slot: [%d], val: [%x]\n", i_slot, val);

	}
#endif

	// MI1 - Slave Interface
#if 0
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_TOP_AR + 0x20) );
		val = readl(NX_BASE_REG_PL301_TOP_AR + 0x20);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_TOP_AR + 0x20) );
	}
#endif

	/* Set progamming for AW */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_TOP_AW );
		val = readl(NX_BASE_REG_PL301_TOP_AW);
		if (val != g_TopBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_TopBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_TOP_AW );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_mi; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_TOP_AW );
		val = readl(NX_BASE_REG_PL301_TOP_AW);
		printk("  ... TOP AW slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif

	// MI1 - Slave Interface
#if 0
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_TOP_AW + 0x20) );
		val = readl(NX_BASE_REG_PL301_TOP_AW + 0x20);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_TOP_AW + 0x20) );
	}
#endif
#endif /* (CFG_BUS_RECONFIG_TOPBUSSI == 1) */

	/* ------------- Display BUS ----------- */
#if (CFG_BUS_RECONFIG_DISPBUSSI == 1)
	num_si = readl(NX_VA_BASE_REG_PL301_DISP + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_DISP + 0xFC4);

	/* Set progamming for AR */
	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_DISP_AR);
		val = readl(NX_BASE_REG_PL301_DISP_AR);
		if (val != g_DispBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_DispBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_DISP_AR );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_DISP_AR );
		val = readl(NX_BASE_REG_PL301_DISP_AR);
		printk("  ... DISP AR slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif

	/* Set progamming for AW */
	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_DISP_AW);
		val = readl(NX_BASE_REG_PL301_DISP_AW);
		if (val != g_DispBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_DispBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_DISP_AW );
	}
#ifdef DEBUG_BUS_CONF
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_DISP_AW);
		val = readl(NX_BASE_REG_PL301_DISP_AW);
		printk("  ... DISP AW slot: [%d], val: [%x]\n", i_slot, val);
	}
#endif
#endif /* (CFG_BUS_RECONFIG_DISPBUSSI == 1) */

	return;
}

#endif	/* #if (CFG_BUS_RECONFIG_ENB == 1) */

static void cpu_base_init(void)
{
	U32 tie_reg, val;
	int i = 0;

	NX_RSTCON_Initialize();
	NX_RSTCON_SetBaseAddress((void*)IO_ADDRESS(NX_RSTCON_GetPhysicalAddress()));

	NX_TIEOFF_Initialize();
	NX_TIEOFF_SetBaseAddress((void*)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress()));

	NX_GPIO_Initialize();
	for (i = 0; NX_GPIO_GetNumberOfModule() > i; i++) {
		NX_GPIO_SetBaseAddress(i, (void*)IO_ADDRESS(NX_GPIO_GetPhysicalAddress(i)));
		NX_GPIO_OpenModule(i);
	}

	NX_ALIVE_Initialize();
	NX_ALIVE_SetBaseAddress((void*)IO_ADDRESS(NX_ALIVE_GetPhysicalAddress()));
	NX_ALIVE_OpenModule();

	NX_CLKPWR_Initialize();
	NX_CLKPWR_SetBaseAddress((void*)IO_ADDRESS(NX_CLKPWR_GetPhysicalAddress()));
	NX_CLKPWR_OpenModule();

	NX_ECID_Initialize();
	NX_ECID_SetBaseAddress((void*)IO_ADDRESS(NX_ECID_GetPhysicalAddress()));

	/*
	 * NOTE> ALIVE Power Gate must enable for RTC register access.
	 * 		 must be clear wfi jump address
	 */
	NX_ALIVE_SetWriteEnable(CTRUE);
	__raw_writel(0xFFFFFFFF, SCR_ARM_SECOND_BOOT);

	/*
	 * NOTE> Control for ACP register access.
	 */
	tie_reg = (U32)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress());

	val = __raw_readl(tie_reg + 0x70) & ~((3 << 30) | (3 << 10));
	writel(val, (tie_reg + 0x70));

	val = __raw_readl(tie_reg + 0x80) & ~(3 << 3);
	writel(val, (tie_reg + 0x80));
}

static void cpu_bus_init(void)
{
	/* MCUS for Static Memory. */
	NX_MCUS_Initialize();
	NX_MCUS_SetBaseAddress((void*)IO_ADDRESS(NX_MCUS_GetPhysicalAddress()));
	NX_MCUS_OpenModule();

	/*
	 * MCU-Static config: Static Bus #0 ~ #1
	 */
	#define STATIC_BUS_CONFIGUTATION( _n_ )								\
	NX_MCUS_SetStaticBUSConfig											\
	( 																	\
		NX_MCUS_SBUSID_STATIC ## _n_, 									\
		CFG_SYS_STATIC ## _n_ ## _BW, 									\
		CFG_SYS_STATIC ## _n_ ## _TACS, 								\
		CFG_SYS_STATIC ## _n_ ## _TCAH, 								\
		CFG_SYS_STATIC ## _n_ ## _TCOS, 								\
		CFG_SYS_STATIC ## _n_ ## _TCOH, 								\
		CFG_SYS_STATIC ## _n_ ## _TACC, 								\
		CFG_SYS_STATIC ## _n_ ## _TSACC,								\
		(NX_MCUS_WAITMODE ) CFG_SYS_STATIC ## _n_ ## _WAITMODE, 		\
		(NX_MCUS_BURSTMODE) CFG_SYS_STATIC ## _n_ ## _RBURST, 			\
		(NX_MCUS_BURSTMODE) CFG_SYS_STATIC ## _n_ ## _WBURST			\
	);

	STATIC_BUS_CONFIGUTATION( 0);
	STATIC_BUS_CONFIGUTATION( 1);
}

/*
 * 	cpu core shutdown/reset
 */
void (*nxp_board_shutdown)(void) = NULL;
void (*nxp_board_reset)(char str, const char *cmd) = NULL;

static unsigned int core_power[][2] = {
	[0] = { TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CLAMPCPU0,
		    TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CPU0PWRDOWN },
	[1] = { TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CLAMPCPU1,
		    TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CPU1PWRDOWN },
	[2] = { TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CLAMPCPU2,
		    TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CPU2PWRDOWN },
	[3] = { TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CLAMPCPU3,
		    TIEOFFINDEX_OF_CORTEXA9MP_TOP_QUADL2C_CPU3PWRDOWN },
};

void nxp_cpu_core_shutdown(int core)
{
	printk(KERN_INFO "cpu.%d shutdown ...\n", core);
	NX_TIEOFF_Set(core_power[core][0], 1);
	NX_TIEOFF_Set(core_power[core][1], 1);
}

void nxp_cpu_shutdown(void)
{
	int cpu, cur = smp_processor_id();

	if (nxp_board_shutdown)
		nxp_board_shutdown();

	for_each_present_cpu(cpu) {
		if (cpu == cur)
			continue;
		nxp_cpu_core_shutdown(cpu);
	}

	printk(KERN_INFO "cpu.%d shutdown ...\n", cur);
	NX_ALIVE_SetWriteEnable(CTRUE);			/* close alive gate */
	NX_ALIVE_SetVDDPWRON(CFALSE, CFALSE);	/* Core power down */
	NX_ALIVE_SetWriteEnable(CFALSE);			/* close alive gate */
	nxp_cpu_core_shutdown(cur);
	halt();
}

void nxp_cpu_reset(char str, const char *cmd)
{
	printk(KERN_INFO "system reset: %s ...\n", cmd);

	if (nxp_board_reset)
		nxp_board_reset(str, cmd);

	__raw_writel((-1UL), SCR_RESET_SIG_RESET);
	if (cmd && !strcmp(cmd, "recovery")) {
		__raw_writel(RECOVERY_SIGNATURE, SCR_RESET_SIG_SET);
		__raw_readl (SCR_RESET_SIG_READ);	/* verify */
	}

	if (cmd && !strcmp(cmd, "usbboot")) {
		__raw_writel(USBBOOT_SIGNATURE, SCR_RESET_SIG_SET);
		__raw_readl (SCR_RESET_SIG_READ);	/* verify */
	}
	printk("recovery sign [0x%x:0x%x] \n", SCR_RESET_SIG_READ, readl(SCR_RESET_SIG_READ));

	NX_ALIVE_SetWriteEnable(CFALSE);	/* close alive gate */
	NX_CLKPWR_SetSoftwareResetEnable(CTRUE);
	NX_CLKPWR_DoSoftwareReset();
}

void nxp_cpu_id_guid(u32 guid[4])
{
	unsigned long start = jiffies;
	int timeout = 1;

	if (NULL == guid) {
		printk("Error: %s no input params ....\n", __func__);
		return;
	}

	while (!NX_ECID_GetKeyReady()) {
		if (time_after(jiffies, start + timeout)) {
			if (NX_ECID_GetKeyReady())
				break;
			printk("Error: %s not key ready for CHIP GUID ...\n", __func__);
			return;
		}
		cpu_relax();
	}
	NX_ECID_GetGUID((NX_GUID*)guid);
}

void nxp_cpu_id_ecid(u32 ecid[4])
{
	unsigned long start = jiffies;
	int timeout = 1;

	if (NULL == ecid) {
		printk("Error: %s no input params ...\n", __func__);
		return;
	}

	while (!NX_ECID_GetKeyReady()) {
		if (time_after(jiffies, start + timeout)) {
			if (NX_ECID_GetKeyReady())
				break;
			printk("Error: %s not key ready for CHIP ECID ...\n", __func__);
			return;
		}
		cpu_relax();
	}
	NX_ECID_GetECID(ecid);
}

void nxp_cpu_id_string(u32 *string)
{
	unsigned long start = jiffies;
	int timeout = 1;

	if (NULL == string) {
		printk("Error: %s no input params ...\n", __func__);
		return;
	}

	while (!NX_ECID_GetKeyReady()) {
		if (time_after(jiffies, start + timeout)) {
			if (NX_ECID_GetKeyReady())
				break;
			printk("Error: %s not key ready for CHIP STRING ...\n", __func__);
			return;
		}
		cpu_relax();
	}
	NX_ECID_GetChipName((char*)string);
}

/*
 * Notify cpu version
 *
 * /sys/devices/platform/cpu/version
 */
static unsigned int cpu_version = -1;
static ssize_t version_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	char *s = buf;
	s += sprintf(s, "%d\n", nxp_cpu_version());
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

/*
 * Notify cpu GUID
 * /sys/devices/platform/cpu/guid
 */
static ssize_t guid_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	char *s = buf;
	u32 guid[4];

	nxp_cpu_id_guid(guid);
	s += sprintf(s, "%08x:%08x:%08x:%08x\n", guid[0], guid[1], guid[2], guid[3]);
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

/*
 * Notify cpu UUID
 * /sys/devices/platform/cpu/guid
 */
static ssize_t uuid_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	char *s = buf;
	u32 euid[4];

	nxp_cpu_id_ecid(euid);

	s += sprintf(s, "%08x:%08x:%08x:%08x\n", euid[0], euid[1], euid[2], euid[3]);
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

/*
 * Notify cpu chip name
 * /sys/devices/platform/cpu/name
 */
static ssize_t name_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	char *s = buf;
	u32 name[12] = {0,};
	int i = 0;
	size_t count = 0;

	nxp_cpu_id_string(name);

	if (0xE4418000 == name[0]) {
		for (i=0; ARRAY_SIZE(name) > i; i++)
			count += sprintf(&buf[count], "%x:", name[i]);
		count--;
		count += sprintf(&buf[count], "\n");
	} else {
		s += sprintf(s, "%s\n", (char*)name);
		if (s != buf)
			*(s-1) = '\n';
		count = (s - buf);
	}

	return count;
}

/*
 * Notify board mem phsical size
 * HEX value
 * /sys/devices/platform/cpu/mem_size
 */
extern struct meminfo meminfo;
static ssize_t mem_size_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	struct meminfo *mi = &meminfo;
	char *s = buf;
	int len = (int)(mi->bank[0].size/SZ_1M);

	s += sprintf(s, "%d\n", len);	/* unit is Mbyte */
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

/*
 * Notify board mem clock frequency
 * HEX value
 * /sys/devices/platform/cpu/mem_clock
 */
static ssize_t mem_clock_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	unsigned int mclk = nxp_cpu_clock_hz(5);
	int khz = (int)(mclk/1000);
	char *s = buf;

	s += sprintf(s, "%d\n", khz);	/* unit is Khz */
	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static struct device_attribute vers_attr = __ATTR(version, 0664, version_show, NULL);
static struct device_attribute guid_attr = __ATTR(guid, 0664, guid_show, NULL);
static struct device_attribute uuid_attr = __ATTR(uuid, 0664, uuid_show, NULL);
static struct device_attribute name_attr = __ATTR(name, 0664, name_show, NULL);
static struct device_attribute mem_size_attr = __ATTR(mem_size, 0664, mem_size_show, NULL);
static struct device_attribute mem_clock_attr = __ATTR(mem_clock, 0664, mem_clock_show, NULL);

static struct attribute *attrs[] = {
	&vers_attr.attr,
	&guid_attr.attr,
	&uuid_attr.attr,
	&name_attr.attr,
	&mem_size_attr.attr,
	&mem_clock_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = (struct attribute **)attrs,
};

static int __init cpu_sys_init(void)
{
	struct kobject *kobj = NULL;
	int ret = 0;

	/* create attribute interface */
	kobj = kobject_create_and_add("cpu", &platform_bus.kobj);
	if (! kobj) {
		printk(KERN_ERR "Fail, create kobject for cpu\n");
		return -ret;
	}

	ret = sysfs_create_group(kobj, &attr_group);
	if (ret) {
		printk(KERN_ERR "Fail, create sysfs group for cpu\n");
		kobject_del(kobj);
		return -ret;
	}
	return ret;
}
module_init(cpu_sys_init);

unsigned int nxp_cpu_version(void)
{
	return cpu_version;
}

/*
 * 	cpu func.
 */
#if defined(CONFIG_PROTOTYPE_RELEASE)
#define	DEBUG_PROTOTYPE		0
#else
#define	DEBUG_PROTOTYPE		1
#endif

void nxp_cpu_base_init(void)
{
	unsigned int  rev = 0;
	unsigned int string[12] = { 0, };
#ifdef CONFIG_SMP
	unsigned int scu_ctrl = 0x0009;
#endif

	cpu_base_init();
	cpu_bus_init();

#ifdef CONFIG_SMP
	writel(0x0000, __PB_IO_MAP_REGS_VIRT + 0x11080);	// ACP Bus Disable

	#if defined (CONFIG_CPU_S5P4418_EX_PERI_BUS)
	writel(0xC0000000, __PB_IO_MAP_MPPR_VIRT + 0x40);   // SCU Address Filtering
    writel(0xCFF00000, __PB_IO_MAP_MPPR_VIRT + 0x44);
    scu_ctrl |= (1<<1);									// SCU Address Filtering Enable
	#endif

	writel(0xffff, __PB_IO_MAP_MPPR_VIRT + 0x0c);		// SCU
	writel(scu_ctrl, __PB_IO_MAP_MPPR_VIRT + 0x00);		// SCU L2 Spec... Enable.
#endif

#if (CFG_BUS_RECONFIG_ENB == 1)
	nxp_set_bus_config();
#endif

	/* Check version */
	if (-1 != cpu_version)
		return;

	nxp_cpu_id_string(string);
	rev = __raw_readl(__PB_IO_MAP_IROM_VIRT + 0x0100);
	switch(rev) {
		case 0xe153000a:
				 cpu_version = 1; break;
		default: cpu_version = 0; break;
	}

	if (0xE4418000 == string[0])
		cpu_version = 2;

	printk(KERN_INFO "CPU : VERSION = %u (0x%X)", cpu_version, rev);
#if defined (CONFIG_CPU_S5P4418_EX_PERI_BUS)
	printk(", Assign Peripheral Exclusive Bus");
#endif
	printk("\n");
}

