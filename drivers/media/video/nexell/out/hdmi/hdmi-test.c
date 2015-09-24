//========================
// HDMI Integ Test
// Basic Test.
// HDMI I2S. real firmware environment (no NX_SIMIO in real)
//========================

//---------
// include header
//---------
//#include <nx_test_unit.h> : this test is not unit test
/* #include <nx_DualDisplay.h> */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <mach/platform.h>
#include <mach/soc.h>

#include <nx_displaytop.h>
#include <nx_disptop_clkgen.h>
#include <nx_lcdif.h>
#include <nx_lvds.h>
#include <nx_hdmi.h>
/* #include <nx_hdmi_devmodel.h> */
#include <nx_tieoff.h>
/* #include <math.h> */
#include <nx_simio.h>
#include <nx_i2s.h>
#include <nx_spdiftx.h>
#include <nx_rstcon.h>
#include <nx_clkgen.h>
#include <nx_resconv.h>
#include <nx_ecid.h>

enum {
    RESOL_720P,
    RESOL_1080P
} DISPLAY_RESOLUTION;

#define CURRENT_RESOLUTION  RESOL_1080P

void Set_DisplayCLK(void);
void Set_DisplayTop(void);
void Set_DualDisplay(void);
void Start_DualDisplay(void);
void Finish_DualDisplay(void);
void Set_HDMI(void);
void Start_HDMI(void);
void Finish_HDMI(void);
void Set_I2S(void);
void Start_I2S(void);
void Finish_I2S(void);
void Set_ECID(void);
void Start_ECID(void);
void Finish_ECID(void);


//=====
// DualDisplay
//=====
void Set_Prim_DualDisplay(void);
void Set_Second_DualDisplay(void);

//-----
// Global Variables
//-----
static CBOOL g_hot_plug_detect_failed;
static CBOOL g_ResConv_irq_called;


// set 148MHz
#define HDMI_148MHZ 0
#define HDMI_74_25MHZ 1
static void NX_HDMI_ConfigPHY(U32 Mode)
{
	// Set BaseAddr
	U32 BaseAddr = NX_HDMI_GetPhysicalAddress( 0 );
	NX_HDMI_SetBaseAddress( 0 , (U32)IO_ADDRESS(BaseAddr));

	if( Mode == HDMI_148MHZ )
	{
		// 따라서 해당 설정은 Internal Reference Clock이 되어야 한다.
		// HDMI PHY는 write나 read시에 꼭 2번씩 access 해야한다. ( setup/hold violation을 피하는 유일한 방법 )
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (0<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (0<<7) );    /// MODE_SET_DONE : APB Set
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, (0<<4) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, (0<<4) );    ///CLK_SEL : REF OSC or INT_CLK
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, (1<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, (1<<7) );    // INT REFCLK : 내부의 syscon에서 받는 clock
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, 0xD1   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, 0xD1   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg08, 0x1F   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg08, 0x1F   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg0C, 0x00   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg0C, 0x00   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg10, 0x40   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg10, 0x40   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg14, 0x40   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg14, 0x40   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg18, 0xF8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg18, 0xF8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg1C, 0xC8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg1C, 0xC8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg20, 0x81   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg20, 0x81   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, 0xE8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, 0xE8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg28, 0xBA   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg28, 0xBA   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg2C, 0xD8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg2C, 0xD8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg30, 0x45   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg30, 0x45   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg34, 0xA0   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg34, 0xA0   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg38, 0xAC   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg38, 0xAC   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg3C, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg3C, 0x80   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0x66   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0x66   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg44, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg44, 0x80   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg48, 0x09   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg48, 0x09   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg4C, 0x84   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg4C, 0x84   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg50, 0x05   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg50, 0x05   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg54, 0x22   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg54, 0x22   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg58, 0x24   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg58, 0x24   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0x86   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0x86   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg60, 0x54   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg60, 0x54   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg64, 0x4B   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg64, 0x4B   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg68, 0x25   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg68, 0x25   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg6C, 0x03   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg6C, 0x03   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg70, 0x00   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg70, 0x00   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg74, 0x00   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg74, 0x00   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg78, 0x01   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg78, 0x01   );
		/* NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, 0x80   ); */
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (1<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (1<<7) );    /// MODE_SET_DONE : APB Set Done

		//NX_HDMI_SetReg( 0, HDMI_PHY_Reg8C, 0x10 );
		// Internal Clock : 148.5 MHZ, 8bit

	} else if( Mode == HDMI_74_25MHZ )
	{
        printk("============> 74_25!!!\n");
		// 따라서 해당 설정은 Internal Reference Clock이 되어야 한다.
		// HDMI PHY는 write나 read시에 꼭 2번씩 access 해야한다. ( setup/hold violation을 피하는 유일한 방법 )
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (0<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (0<<7) );    /// MODE_SET_DONE : APB Set
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, (0<<4) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, (0<<4) );    ///CLK_SEL : REF OSC or INT_CLK
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, (1<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, (1<<7) );    // INT REFCLK : 내부의 syscon에서 받는 clock
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, 0xD1   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, 0xD1   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg08, 0x1F   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg08, 0x1F   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg0C, 0x10   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg0C, 0x10   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg10, 0x40   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg10, 0x40   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg14, 0x40   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg14, 0x40   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg18, 0xF8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg18, 0xF8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg1C, 0xC8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg1C, 0xC8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg20, 0x81   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg20, 0x81   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, 0xE8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, 0xE8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg28, 0xBA   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg28, 0xBA   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg2C, 0xD8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg2C, 0xD8   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg30, 0x45   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg30, 0x45   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg34, 0xA0   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg34, 0xA0   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg38, 0xAC   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg38, 0xAC   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg3C, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg3C, 0x80   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0x56   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0x56   );
		//NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0xFF   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0xFF   ); // Max amp
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg44, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg44, 0x80   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg48, 0x09   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg48, 0x09   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg4C, 0x84   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg4C, 0x84   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg50, 0x05   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg50, 0x05   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg54, 0x22   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg54, 0x22   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg58, 0x24   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg58, 0x24   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0x86   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0x86   );
		//NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0xFE   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0xFE   ); // Max amp
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg60, 0x54   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg60, 0x54   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg64, 0xA5   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg64, 0xA5   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg68, 0x24   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg68, 0x24   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg6C, 0x01   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg6C, 0x01   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg70, 0x00   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg70, 0x00   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg74, 0x00   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg74, 0x00   );
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg78, 0x01   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg78, 0x01   );
		/* NX_HDMI_SetReg( 0, HDMI_PHY_Reg8C, 0x10   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg8C, 0x10   ); */
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, 0x80   );
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg04, NX_HDMI_GetReg(0, HDMI_PHY_Reg04), NX_HDMI_GetReg(0, HDMI_PHY_Reg04));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg08, NX_HDMI_GetReg(0, HDMI_PHY_Reg08), NX_HDMI_GetReg(0, HDMI_PHY_Reg08));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg0C, NX_HDMI_GetReg(0, HDMI_PHY_Reg0C), NX_HDMI_GetReg(0, HDMI_PHY_Reg0C));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg10, NX_HDMI_GetReg(0, HDMI_PHY_Reg10), NX_HDMI_GetReg(0, HDMI_PHY_Reg10));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg14, NX_HDMI_GetReg(0, HDMI_PHY_Reg14), NX_HDMI_GetReg(0, HDMI_PHY_Reg14));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg18, NX_HDMI_GetReg(0, HDMI_PHY_Reg18), NX_HDMI_GetReg(0, HDMI_PHY_Reg18));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg1C, NX_HDMI_GetReg(0, HDMI_PHY_Reg1C), NX_HDMI_GetReg(0, HDMI_PHY_Reg1C));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg60, NX_HDMI_GetReg(0, HDMI_PHY_Reg60), NX_HDMI_GetReg(0, HDMI_PHY_Reg60));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg64, NX_HDMI_GetReg(0, HDMI_PHY_Reg64), NX_HDMI_GetReg(0, HDMI_PHY_Reg64));
        printk("reg 0x%x: %x %x\n", HDMI_PHY_Reg68, NX_HDMI_GetReg(0, HDMI_PHY_Reg68), NX_HDMI_GetReg(0, HDMI_PHY_Reg68));
		NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (1<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (1<<7) );    /// MODE_SET_DONE : APB Set Done

		//NX_HDMI_SetReg( 0, HDMI_PHY_Reg8C, 0x10 );
		// Internal Clock : 148.5 MHZ, 8bit
	}
}

#if 0
static void dump_hdmi(void)
{
    U32 addr = (U32)IO_ADDRESS(HDMI_ADDR_OFFSET);
    int i;
    int size = 0x50/4;
    printk("==========================================\n");
    printk("DUMP HDMI REGISTERS\n");
    printk("\n PHY_BASEADDR_HDMI_MODULE = %x", PHY_BASEADDR_HDMI_MODULE);
    printk("\n HDMI_ADDR_OFFSET = %x\n", HDMI_ADDR_OFFSET);
    for (i = 0; i < size; i++) {
        printk("0x%x ===> 0x%x\n",
        		(addr+PHY_BASEADDR_HDMI_MODULE),
        		NX_HDMI_GetReg(0, addr));

        addr += 4;
    }
    printk("==========================================\n");
}
#endif

int test_hdmi(void)
{
    printk("%s: %d\n", __func__, __LINE__);

	//============
	// DisplayTop Reset
	//============
    NX_RSTCON_SetnRST(NX_DISPLAYTOP_GetResetNumber(), RSTCON_nDISABLE);
    NX_RSTCON_SetnRST(NX_DISPLAYTOP_GetResetNumber(), RSTCON_nENABLE);
    printk("%s: %d\n", __func__, __LINE__);
	//========
	// HDMI PHY Config ( PLL Lock되는데 시간이 필요하므로 )
	//========

	// HDMI PCLK Enable
    U32 BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( HDMI_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress        ( HDMI_CLKGEN, (u32)IO_ADDRESS(BaseAddr));
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( HDMI_CLKGEN, NX_PCLKMODE_ALWAYS );

    printk("%s: %d\n", __func__, __LINE__);

	// REF CLK에 INT_CLK을 사용하도록 설정. TIEOFF를 나중에 설정해준다.
	NX_TIEOFF_Initialize();
	NX_TIEOFF_Set( TIEOFFINDEX_OF_DISPLAYTOP0_i_HDMI_PHY_REFCLK_SEL, 1 );
    printk("%s: %d\n", __func__, __LINE__);

	NX_RSTCON_SetnRST  ( NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE );
	NX_RSTCON_SetnRST  ( NX_HDMI_GetResetNumber(0, i_nRST),     RSTCON_nENABLE); // APB
    printk("%s: %d\n", __func__, __LINE__);

	NX_RSTCON_SetnRST  ( NX_HDMI_GetResetNumber(0, i_nRST_PHY), RSTCON_nENABLE );
	NX_RSTCON_SetnRST  ( NX_HDMI_GetResetNumber(0, i_nRST), RSTCON_nENABLE     ); // APB
    printk("%s: %d\n", __func__, __LINE__);

#if 1
	NX_HDMI_ConfigPHY( HDMI_74_25MHZ );
#else
	NX_HDMI_ConfigPHY( HDMI_148MHZ );
#endif

    printk("%s: %d\n", __func__, __LINE__);

	Set_ECID();     // ECID는 Ready가 되는데 오래 걸리므로 가장 먼저 켜준다.
    printk("%s: %d\n", __func__, __LINE__);
	/* Set_I2S(); */
    /* printk("%s: %d\n", __func__, __LINE__); */
	/* Start_I2S(); */
    /* printk("%s: %d\n", __func__, __LINE__); */
	Start_ECID();   // ECID Ready
    printk("%s: %d\n", __func__, __LINE__);

	//----------
	// HDMI Reset
	//----------
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nENABLE);
    printk("%s: %d\n", __func__, __LINE__);

    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_VIDEO), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_SPDIF), RSTCON_nENABLE);
    NX_RSTCON_SetnRST(NX_HDMI_GetResetNumber(0, i_nRST_TMDS), RSTCON_nENABLE);
    printk("%s: %d\n", __func__, __LINE__);

	//==================
	// HDMI PHY_READY가 1이 되었는지 확인
	//==================

	U32 Is_HDMI_PHY_READY = CFALSE;
	while( Is_HDMI_PHY_READY == CFALSE )
	{
        u32 regval = NX_HDMI_GetReg( 0, HDMI_LINK_PHY_STATUS_0 );
        printk("===> PHY: 0x%x\n", regval);
        if (regval & 0x01) {
            printk("PHY READY!!!!\n");
            break;
        } else {
            mdelay(10);
        }
	}
    printk("%s: %d\n", __func__, __LINE__);

	//=========
	// Display Test 시작.
	//=========
	Set_DisplayCLK();
    printk("%s: %d\n", __func__, __LINE__);
	Set_HDMI();
    printk("%s: %d\n", __func__, __LINE__);
	Set_DualDisplay();
    printk("%s: %d\n", __func__, __LINE__);
	Start_DualDisplay();
    printk("%s: %d\n", __func__, __LINE__);
	Start_HDMI();
    printk("%s: %d\n", __func__, __LINE__);

#if 0
	//=========
	// GPIO Alt Setting
	//=========
	U32 gpio_i=0;
	for(gpio_i=0; gpio_i<5; gpio_i++)
	{
		NX_GPIO_SetBaseAddress( gpio_i, NX_GPIO_GetPhysicalAddress(gpio_i) );

	}
	NX_GPIO_SetPadFunction( 0, 0, 1 ); // VCLK
	NX_GPIO_SetPadFunction( 0, 25, 1 ); // VSync
	NX_GPIO_SetPadFunction( 0, 26, 1 ); // HSync
	NX_GPIO_SetPadFunction( 0, 27, 1 ); // PADDE

	for(gpio_i=1; gpio_i<=24; gpio_i++)
	{
		NX_GPIO_SetPadFunction( 0, gpio_i, 1 ); // RGB24[gpio_i]
	}

	// Display 사용을 위해서는 GPIO의 DRV와 SLEW를 설정해 주어야 한다.
	NX_GPIO_SetDRV1	( 0, 0xFFFFFFFF );
	NX_GPIO_SetDRV0	( 0, 0xFFFFFFFF );
	NX_GPIO_SetSLEW	( 0, 0x00000000 );

	volatile U32 * gpio_reg = 0xC001A020;
	//*gpio_reg = 0x55555555;
	printk("%x: %x", gpio_reg, *gpio_reg);

	gpio_reg = 0xC001A024;
	//*gpio_reg = 0x55555555;
	printk("%x: %x", gpio_reg, *gpio_reg);

	printk("%s: %d\n", __func__, __LINE__);

    /* dump_hdmi(); */
#endif

	return 0;
}

//===========
// ECID Reset , only one module
//===========
void Set_ECID()
{
	NX_ECID_SetBaseAddress( (U32)IO_ADDRESS(NX_ECID_GetPhysicalAddress()));

}

void Start_ECID()
{
	U32 Is_KeyReady = CFALSE;

	while( CFALSE == Is_KeyReady )
	{
		Is_KeyReady = NX_ECID_GetKeyReady();
	}

	return;

}

void Finish_ECID()
{
	return;
}

//==========
// I2S, 0~255 의 값을 DMA에서 PERI로 전송해준다.
//==========

// I2S Global Variable
static U32         g_I2S_DMA_Channel[NUMBER_OF_I2S_MODULE];
/* static NX_Memory1D g_I2S_DMA_MEM    [NUMBER_OF_I2S_MODULE]; */
static U8 *g_I2S_DMA_MEM[NUMBER_OF_I2S_MODULE];
static const U32   c_I2S_DMALength = 1024; // 4*256;


#if 0
void Set_I2S()
{
	// struct
	NX_I2S_Initialize();
	struct
	{
	    struct NX_I2S_RegisterSet *pRegister;
	} pI2SReg[NUMBER_OF_I2S_MODULE] = { {CNULL,}, };

    U32  ModuleIndex ;


    if( NX_I2S_GetNumberOfModule() < 1 )
    {

	}

    for( ModuleIndex =0;  ModuleIndex <NX_I2S_GetNumberOfModule();  ModuleIndex++)
    {
    	//-------
    	// I2S CLKGEN Setting
    	//-------
    	// 32 kHz, 12.28MHz : 256fs.
    	// Test로 일단 147.5/12 = 12.28MHz를 넣어준다. 2013-03-07 오후 2:47:27
    	NX_CLKGEN_SetClockDivisorEnable ( NX_I2S_GetClockNumber(ModuleIndex), CFALSE      );

    	NX_CLKGEN_SetClockSource        ( NX_I2S_GetClockNumber(ModuleIndex), 0, 2   ); // PLL 2번. ( 147 MHz )
    	NX_CLKGEN_SetClockDivisor       ( NX_I2S_GetClockNumber(ModuleIndex), 0, 12 ); // Div 12
    	//NX_CLKGEN_SetClockOutInv        ( NX_I2S_GetClockNumber(ModuleIndex), 0, 1   );
    	NX_CLKGEN_SetClockSource        ( NX_I2S_GetClockNumber(ModuleIndex), 1, 7   ); // CLKOUT[0]
    	NX_CLKGEN_SetClockDivisor       ( NX_I2S_GetClockNumber(ModuleIndex), 1, 1<<0); // Div 1
    	//NX_CLKGEN_SetClockOutInv        ( NX_I2S_GetClockNumber(ModuleIndex), 1, 1   );
    	NX_CLKGEN_SetClockDivisorEnable ( NX_I2S_GetClockNumber(ModuleIndex), 1      );


		//=========
		//@modified choiyk 2013-03-13 오전 10:33:36
        //=========
        /* NX_I2S_EnablePAD( ModuleIndex ); */


		// I2S Reset
        /* NX_RESET_EnterReset( NX_I2S_GetResetNumber (  ModuleIndex  ) ); */
        /* NX_RESET_LeaveReset( NX_I2S_GetResetNumber (  ModuleIndex  ) ); */
        NX_RSTCON_SetnRST( NX_I2S_GetResetNumber (  ModuleIndex  ), RSTCON_nENABLE );
        NX_RSTCON_SetnRST( NX_I2S_GetResetNumber (  ModuleIndex  ), RSTCON_nENABLE );

        NX_I2S_SetBaseAddress(  ModuleIndex , NX_I2S_GetPhysicalAddress( ModuleIndex ) );
        pI2SReg[ ModuleIndex ].pRegister = ( struct NX_I2S_RegisterSet * )NX_I2S_GetPhysicalAddress(  ModuleIndex  );   // register pointer

	    // I2S setup
        pI2SReg[ ModuleIndex ].pRegister->CON = 0x00000000; // power down

        pI2SReg[ ModuleIndex ].pRegister->FIC = (1<<15)|(1<<7);
        U32 Data32 = pI2SReg[ ModuleIndex ].pRegister->FIC;
        pI2SReg[ ModuleIndex ].pRegister->FIC = 0;

	    // 24bit, MSB first , Root fs : 768fs, Bit fs : 48 fs
        U32 BLC	= 2;    //[14:13] 0 : 16              1 : 8               2 : 24    3 : x		//Bit Length Control
	    U32 IMS = 1;    //[11:10] 0 : internal master 1 : external master 2 : slave 3 : slave	//I2S Master or Slave
	    U32 TXR = 0;    //[ 9: 8] 0 : tx              1 : rx              2 : dual  3 : x		//Transmit or Recevie
	    U32 LRP	= 0;    //[ 7]    0 : low for left	  1 : high for left
	    U32 SDF = 0;    //[ 6: 5] 0 : i2s             1 : msb             2 : lsb   3 : x 		//Serial data format
	    U32 RFS = 2;    //[ 4: 3] 0 : 256fs           1 : 512fs           2 : 384fs 3 : 768fs
	    U32 BFS = 1;    //[ 2: 1] 0 : 32fs            1 : 48fs            2 : 16fs  3 : 24fs

        Data32 = (BLC<<13)|(IMS<<10)|(TXR<<8)|(LRP<<7)|(SDF<<5)|(RFS<<3)|(BFS<<1);
        pI2SReg[ ModuleIndex ].pRegister->MOD = Data32;

	    // DMA Setting.
        // TODO : where is prototype code???
#if 0
		g_I2S_DMA_Channel[ ModuleIndex ] = NX_PERIDMA_GetUnLockChannel(NX_I2S_GetDMAIndex_PCMOut( ModuleIndex ));   // Tx DMA Channel Number
    	if( !NX_DMA_Lock( g_I2S_DMA_Channel[ ModuleIndex ] ) )
    	{

    	}

    	/* NX_Malloc1D(0, c_I2S_DMALength, &g_I2S_DMA_MEM[ ModuleIndex ]  ); */
    	/* U32 DMA_Addr = g_I2S_DMA_MEM[ ModuleIndex ].Address; */
        g_I2S_DMA_MEM[ModuleIndex] = (U8 *)malloc(c_I2S_DMALength);
    	U32 *pt_DMA = (U32 *) g_I2S_DMA_MEM[ModuleIndex] ;

    	// 0 ~ 255 까지 쓴다.
        U32 jj = 0;
    	for(jj=0; jj<256; jj++)
    	{
    		pt_DMA[jj] = jj;
    	}



    	//-------------------
    	// Memory -> I2S : TX
    	//-------------------
    	NX_DMA_TransferMemToIO( g_I2S_DMA_Channel[ ModuleIndex ] ,                             // DMA channel
							    (void *)pt_DMA,                           // source address
    	                        (void *)(NX_I2S_GetPhysicalAddress( ModuleIndex )+0x10),    // destination address
							    NX_I2S_GetDMAIndex_PCMOut( ModuleIndex ),                   // peri DMA channel
							    NX_I2S_GetDMABusWidth( ModuleIndex ),                       // source bit width
							    c_I2S_DMALength );                               // transfer size
#endif

    }
}

void Start_I2S()
{
// struct
	NX_I2S_Initialize();
	struct
	{
	    struct NX_I2S_RegisterSet *pRegister;
	} pI2SReg[NUMBER_OF_I2S_MODULE] = { {CNULL,}, };

    U32  ModuleIndex ;
    for( ModuleIndex =0;  ModuleIndex <NX_I2S_GetNumberOfModule();  ModuleIndex++)
    {
        NX_I2S_SetBaseAddress(  ModuleIndex , NX_I2S_GetPhysicalAddress( ModuleIndex ) );
        pI2SReg[ ModuleIndex ].pRegister = ( struct NX_I2S_RegisterSet * )NX_I2S_GetPhysicalAddress(  ModuleIndex  );   // register pointer

	    // I2S on
        pI2SReg[ ModuleIndex ].pRegister->CON = 0x00000007; // 0x01은 단순히 키기. 0x07 은 DMA 까지 .
    }
}

void Finish_I2S()
{

	// struct
	NX_I2S_Initialize();
	struct
	{
	    struct NX_I2S_RegisterSet *pRegister;
	} pI2SReg[NUMBER_OF_I2S_MODULE] = { {CNULL,}, };

    U32  ModuleIndex ;
    for( ModuleIndex =0;  ModuleIndex <NX_I2S_GetNumberOfModule();  ModuleIndex++)
    {
        NX_I2S_SetBaseAddress(  ModuleIndex , NX_I2S_GetPhysicalAddress( ModuleIndex ) );
        pI2SReg[ ModuleIndex ].pRegister = ( struct NX_I2S_RegisterSet * )NX_I2S_GetPhysicalAddress(  ModuleIndex  );   // register pointer

	    // I2S on
        pI2SReg[ ModuleIndex ].pRegister->CON = 0x00000000; // 0x01은 단순히 키기. 0x07 은 DMA 까지 .

        // @todo
        // I2S DMA, Mem Free..

    }
}
#endif

//========================
//
// HDMI Integ Test, DisplayTop도 같이 동시에 테스트 한다.
//
// PAD : ResConv 출력
// DualDisplay : Prim : 720x480, 3 Layer,   148.5MHz ( HDMI )
//               Second : 1920x16, 2 Layer, CLKGEN, PLL2 ( 147.5MHz )
// ResConv : Prim을 받아서 360x240으로 축소.CLKGEN, PLL2 ( 147.5MHz ) - DIV 4
// LCDIF : RGB888 형식으로 출력.
// LVDS : Second를 받아서 Location 사용 출력.
// HDMI : Prim을 받아서 148.5MHz, 480p, I2S, HDCP
// ToMIPI : Second로 내보내줌.
//
// I2S, SPDIF : 0~255 의 값을 DMA에서 PERI로 전송해준다.
// ECID : HDCP Key는 아래와 같이 설정한다. ( param으로 설정 가능함. )
//
// Inst_ECID_SBOOT=128'h020A26E4_08012112_0804C089_FFFFFFFF
// Inst_ECID_SJTAG=128'h020A26E4_08012112_0804C089_FFFFFFFF
// Inst_ECID_GUID =128'h00000000_00000000_00000000_00000000
//
// @choiyk 2013-01-15 오후 8:06:16
//========================

//==========================
// DisplayTop Config : Module은 한개뿐이다.
//==========================
// DisplayTop은 RESET을 풀어줘서는 안된다.
void Set_DisplayTop()
{
	// MUX Select

	// // DisplayTop Reset
	// U32 ResetNum = NX_DISPLAYTOP_GetResetNumber( 0 ) ;
    // NX_RESET_EnterReset( ResetNum );
    // NX_RESET_LeaveReset( ResetNum );

    // DisplayTop Set BaseAddr
    U32 BaseAddr = NX_DISPLAYTOP_GetPhysicalAddress();
	NX_DISPLAYTOP_SetBaseAddress(BaseAddr);

	//-------------------
	// DisplayTop MUX Enable : Primary
	//-------------------
	NX_DISPLAYTOP_SetRESCONVMUX	 ( CTRUE, PrimaryMLC );
	NX_DISPLAYTOP_SetHDMIMUX     ( CTRUE, PrimaryMLC );
	NX_DISPLAYTOP_SetMIPIMUX     ( CTRUE, PrimaryMLC );
	NX_DISPLAYTOP_SetLVDSMUX     ( CTRUE, PrimaryMLC );
	NX_DISPLAYTOP_SetPrimaryMUX  ( PADMUX_PrimaryMLC );
	//*************************************

	////-------------------------------------------------
	//// FakeLCD Setting : 하나뿐임. PADMUX가 Second이므로 Second에 맞게 설정해줘야함.
	////-------------------------------------------------
#if 0
	U32 ModuleIndex = 0; // FakeLCD가 하나뿐이므로 일단은..

	NX_FAKELCD_Set_LCDType(  ModuleIndex, NX_FAKELCD_TFT );
	NX_FAKELCD_Set_TFT_CTRL0(  ModuleIndex,  0); // Frame Count Init
	U32 regvalue = (1<<5) | (0<<4) | (0x3<<0);  // FrameTest, No Interlace, RGB888
	NX_FAKELCD_Set_TFT_CTRL0(  ModuleIndex,  regvalue);
	regvalue = 0; // no shift
	NX_FAKELCD_Set_RGBSHIFT(  ModuleIndex,  regvalue);
	NX_FAKELCD_SetFileNameLIST( ModuleIndex, 0 ); //File Name List Sel
#endif

	//----
	// 각 CLKGEN을 여기에서 설정해준다.
	//----	ResConv_CLKGEN, LCDIF_CLKGEN, ToMIPI_CLKGEN, ToLVDS_CLKGEN, HDMI_CLKGEN
	enum DISPTOP_CLKGEN_CLKSRC
	{
		CLKSRC_PLL0     = 0,
		CLKSRC_PLL1     = 1,
		CLKSRC_PLL2     = 2,
		CLKSRC_PLL3     = 3,
		CLKSRC_HDMICLK  = 4,
	};

	// ResConv Clock 설정.
#if 0
	BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( ResConv_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress( ResConv_CLKGEN, BaseAddr );

	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ResConv_CLKGEN, CFALSE);
	NX_DISPTOP_CLKGEN_SetClockPClkMode      ( ResConv_CLKGEN, NX_PCLKMODE_ALWAYS );
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ResConv_CLKGEN, CFALSE );
	NX_DISPTOP_CLKGEN_SetClockSource        ( ResConv_CLKGEN, ResConv_ICLK, CLKSRC_HDMICLK );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( ResConv_CLKGEN, ResConv_ICLK, 1 ); // 147MHz/2

	// Core Clock은 꼭 333MHz ( PCLK의 2배 ) 로 넣어주어야 한다. 그렇지 않으면 필터 설정 부분에서 violation이 발생하게 된다.
	NX_DISPTOP_CLKGEN_SetClockSource        ( ResConv_CLKGEN, ResConv_CCLK, CLKSRC_HDMICLK );  // Input Clock을 쓰면 underflow가 발생한다.
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( ResConv_CLKGEN, ResConv_CCLK, 1 ); // //
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ResConv_CLKGEN, CTRUE);
#endif


    BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( HDMI_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress               ( HDMI_CLKGEN, BaseAddr );

	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( HDMI_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( HDMI_CLKGEN, NX_PCLKMODE_ALWAYS );
#if 0
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( HDMI_CLKGEN, CFALSE );
	NX_DISPTOP_CLKGEN_SetClockSource        ( HDMI_CLKGEN, HDMI_i_VCLK_CLKOUT, CLKSRC_HDMICLK );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( HDMI_CLKGEN, HDMI_i_VCLK_CLKOUT, 1 ); // 148.5MHz = 148.5MHz
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( HDMI_CLKGEN, CTRUE);
#endif

#if 0
    BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( LCDIF_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress               ( LCDIF_CLKGEN, BaseAddr );

    NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( LCDIF_CLKGEN, NX_PCLKMODE_ALWAYS );
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CFALSE );
	NX_DISPTOP_CLKGEN_SetClockSource        ( LCDIF_CLKGEN, 0, CLKSRC_HDMICLK );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( LCDIF_CLKGEN, 0, 4 ); // 147MHz/6
	NX_DISPTOP_CLKGEN_SetClockSource        ( LCDIF_CLKGEN, 1, 7 );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( LCDIF_CLKGEN, 1, 1 ); // 147MHz/6 -- ResConv Output
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( LCDIF_CLKGEN, CTRUE);
#endif

    BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( ToMIPI_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress               ( ToMIPI_CLKGEN, BaseAddr );

	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToMIPI_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( ToMIPI_CLKGEN, NX_PCLKMODE_ALWAYS );

	// MIPI 의 0번이 HDMI SPDIF Clock
	NX_DISPTOP_CLKGEN_SetClockSource        ( ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, CLKSRC_PLL2 ); // CLKSRC_PLL2
    NX_DISPTOP_CLKGEN_SetClockDivisor       ( ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, 2           ); // SPDIF는 100MHz이하.

	// MIPI 의 1 번이 MIPI로 나가는 Clock.
	NX_DISPTOP_CLKGEN_SetClockSource        ( ToMIPI_CLKGEN, 1, 7 );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( ToMIPI_CLKGEN, 1, 2 ); // 147MHz/6 = 147MHz
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToMIPI_CLKGEN, CTRUE);

#if 0
    BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( ToLVDS_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress               ( ToLVDS_CLKGEN, BaseAddr );

    NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToLVDS_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( ToLVDS_CLKGEN, NX_PCLKMODE_ALWAYS );
    NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToLVDS_CLKGEN, CFALSE );
	NX_DISPTOP_CLKGEN_SetClockSource        ( ToLVDS_CLKGEN, 0, CLKSRC_HDMICLK );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( ToLVDS_CLKGEN, 0, 4 ); // 147MHz/6
	NX_DISPTOP_CLKGEN_SetClockSource        ( ToLVDS_CLKGEN, 1, 7 );  // CLKSRC_PLL2
	NX_DISPTOP_CLKGEN_SetClockDivisor       ( ToLVDS_CLKGEN, 1, 1 ); // 147MHz/6
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToLVDS_CLKGEN, CTRUE);
#endif


	// must initialize this !!
	NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd( 0, 0 );
	NX_DISPLAYTOP_HDMI_SetVSyncStart     ( 0 ); // from posedge VSync
	NX_DISPLAYTOP_HDMI_SetHActiveStart   ( 0 ); // from posedge HSync
	NX_DISPLAYTOP_HDMI_SetHActiveEnd     ( 0 ); // from posedge HSync
}

void Set_DisplayCLK()
{
    // DisplayTop Set BaseAddr
    U32 BaseAddr = NX_DISPLAYTOP_GetPhysicalAddress();
	NX_DISPLAYTOP_SetBaseAddress((U32)IO_ADDRESS(BaseAddr));

	//-------------------
	// DisplayTop MUX Enable : Primary
	//-------------------
	NX_DISPLAYTOP_SetHDMIMUX     ( CTRUE , PrimaryMLC );
	NX_DISPLAYTOP_SetRESCONVMUX	 ( CFALSE, PrimaryMLC );
	NX_DISPLAYTOP_SetMIPIMUX     ( CFALSE, PrimaryMLC );
	NX_DISPLAYTOP_SetLVDSMUX     ( CFALSE, PrimaryMLC );
	NX_DISPLAYTOP_SetPrimaryMUX  ( PADMUX_PrimaryMLC  );
	//*************************************
	//----
	// 각 CLKGEN을 여기에서 설정해준다.
	//----	ResConv_CLKGEN, LCDIF_CLKGEN, ToMIPI_CLKGEN, ToLVDS_CLKGEN, HDMI_CLKGEN
	enum DISPTOP_CLKGEN_CLKSRC
	{
		CLKSRC_PLL0     = 0,
		CLKSRC_PLL1     = 1,
		CLKSRC_PLL2     = 2,
		CLKSRC_PLL3     = 3,
		CLKSRC_HDMICLK  = 4,
	};

    BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( HDMI_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress               ( HDMI_CLKGEN, (U32)IO_ADDRESS(BaseAddr));

	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( HDMI_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( HDMI_CLKGEN, NX_PCLKMODE_ALWAYS );

    BaseAddr = NX_DISPTOP_CLKGEN_GetPhysicalAddress( ToMIPI_CLKGEN );
	NX_DISPTOP_CLKGEN_SetBaseAddress               ( ToMIPI_CLKGEN, (U32)IO_ADDRESS(BaseAddr));

	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToMIPI_CLKGEN, CFALSE);
    NX_DISPTOP_CLKGEN_SetClockPClkMode      ( ToMIPI_CLKGEN, NX_PCLKMODE_ALWAYS );

	// MIPI 의 0번이 HDMI SPDIF Clock
	NX_DISPTOP_CLKGEN_SetClockSource        ( ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, CLKSRC_PLL2 ); // CLKSRC_PLL2
    NX_DISPTOP_CLKGEN_SetClockDivisor       ( ToMIPI_CLKGEN, HDMI_SPDIF_CLKOUT, 2           ); // SPDIF는 100MHz이하.

	// MIPI 의 1 번이 MIPI로 나가는 Clock.
	// - NX_DISPTOP_CLKGEN_SetClockSource        ( ToMIPI_CLKGEN, 1, 7 );  // CLKSRC_PLL2
	// - NX_DISPTOP_CLKGEN_SetClockDivisor       ( ToMIPI_CLKGEN, 1, 2 ); // 147MHz/6 = 147MHz
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable ( ToMIPI_CLKGEN, CTRUE);


	// must initialize this !!
	NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd( 0, 0 );
	NX_DISPLAYTOP_HDMI_SetVSyncStart     ( 0 ); // from posedge VSync
	NX_DISPLAYTOP_HDMI_SetHActiveStart   ( 0 ); // from posedge HSync
	NX_DISPLAYTOP_HDMI_SetHActiveEnd     ( 0 ); // from posedge HSync
}

//==========================
// DualDisplay Config : 2개인것을 알고 각각을 Config 해준다..
//==========================

// DualDisplay Global Variable
/* static NX_Memory1D g_MLC_IMAGEBASE [NUMBER_OF_MLC_MODULE]; */
char *g_MLC_IMAGEBASE[NUMBER_OF_MLC_MODULE];


static U32 DualDisplay_IRQ_Count   [NUMBER_OF_MLC_MODULE] = {0,};


void Set_DualDisplay()
{
	//-----------
	// Reset
	//-----------
	NX_RSTCON_SetnRST ( RESETINDEX_OF_DISPLAYTOP_MODULE_i_DualDisplay_nRST, RSTCON_nENABLE );
	NX_RSTCON_SetnRST ( RESETINDEX_OF_DISPLAYTOP_MODULE_i_DualDisplay_nRST, RSTCON_nENABLE );

	//-----
	// Primary MLC & SyncGen Config
	//-----
	Set_Prim_DualDisplay();
}

void Start_DualDisplay()
{
	U32 ModuleIndex;

	//-------------------------------------------------
    // Syncgen을 Enable 함으로써 동작한다.
    //-------------------------------------------------

    // Primary
    ModuleIndex = 0;

	NX_DPC_ClearInterruptPending(ModuleIndex, 0); // IntNum은 의미 없음.
	NX_DPC_SetEnable(ModuleIndex, CTRUE, CTRUE, CFALSE, CFALSE, CFALSE);
	DualDisplay_IRQ_Count[ ModuleIndex ] = 0;

}

void Finish_DualDisplay()
{
    // Primary
    U32 ModuleIndex = 0;

	NX_DPC_ClearInterruptPending(ModuleIndex, 0); // IntNum은 의미 없음.
	NX_DPC_SetEnable(ModuleIndex, CFALSE, CFALSE, CFALSE, CFALSE, CFALSE);

}


//======================================
// choiyk : R-REC-BT.1120-8-201201-I!!PDF-E.pdf 참조. ( http://www.itu.int/rec/R-REC-BT.1120-8-201201-I/en )
// HDMI급 해상도의 Clock과 Blank를 설정해서 확인해본다.
//**********************************************************************************************
//**********************************************************************************************

void Set_Prim_DualDisplay()
{
	volatile U32 * addr_reg = NULL;

	//------------
	// Primary Display
	//------------
	U32 ModuleIndex = 0;

	//-----------
	// Set BaseAddr
	//-----------
	U32 BaseAddr = NX_MLC_GetPhysicalAddress(ModuleIndex);
	NX_MLC_SetBaseAddress(ModuleIndex, (U32)IO_ADDRESS(BaseAddr));
	BaseAddr = NX_DPC_GetPhysicalAddress(ModuleIndex);
	NX_DPC_SetBaseAddress(ModuleIndex, (U32)IO_ADDRESS(BaseAddr));

	//------------
	// Clock Config.
	//------------
	// MLC Clock Mode Setting : BCLK, PCLK, Always
	NX_MLC_SetClockPClkMode( ModuleIndex, NX_PCLKMODE_ALWAYS );
	NX_MLC_SetClockBClkMode( ModuleIndex, NX_BCLKMODE_ALWAYS );

	enum DPC_CLKSRC
	{
		CLKSRC_PLL0     = 0,
		CLKSRC_PLL1     = 1,
		CLKSRC_PLL2     = 2,
		CLKSRC_i_VCLK   = 3, // Not Used
		CLKSRC_HDMICLK  = 4,
		CLKSRC_i_VCLK27 = 5, // Not Used
		CLKSRC_PLL3     = 7,
		CLKSRC_FORCE32  = 0x7fffffff
	};

	//U32 Divisor = FPLL_CLK_IN_MHZ / CyclesPerSecond;

	NX_DPC_SetClockDivisorEnable(ModuleIndex, CFALSE);

	NX_DPC_SetClockOutEnb(ModuleIndex,  0, CFALSE );
	NX_DPC_SetClockOutEnb(ModuleIndex, 1, CFALSE );

	NX_DPC_SetOutVideoClkSelect(ModuleIndex, PADVCLK); // pixel clock is pad clock

	NX_DPC_SetClockOutInv(ModuleIndex,0, 0);        // Out Pad VCLK2 Inverse
    NX_DPC_SetClockSource(ModuleIndex, 0, CLKSRC_HDMICLK ); // HDMI Clock !
    NX_DPC_SetClockDivisor(ModuleIndex, 0, 1 );

    NX_DPC_SetClockSource(ModuleIndex, 1, 7 ); // DPC CLKGEN 2 config
    NX_DPC_SetClockDivisor(ModuleIndex, 1, 1 );


    NX_DPC_SetClockDivisorEnable(ModuleIndex,CTRUE);


	//===============
	// Video Config.
	//===============
/* #if (CURRENT_RESOLUTION == RESOL_720P) */
#if 1
	U32 Width = 1280;
	U32 Height =720;
#else
	//U32 Width = 1920;
	U32 Width = 1920 ;
	U32 Height = 1080;
#endif

	U32 RGB0_ImgX    =   0; U32 RGB0_ImgY    = 0; char *RGB0_file      = "../common_image/beach1920x1080.bmp_RGB888.hex";

	//------------
	// Image Load Section
	//------------
	U32 s_RGB0_Address;
	//========================================================
	// 이부분을 소프트웨어에서 쓸수있도록 수정하시면 됩니다.
	//========================================================
	// NX_1D Malloc
    g_MLC_IMAGEBASE[ModuleIndex] = (char *)0x58600000;
    /* BaseAddr = (U32)g_MLC_IMAGEBASE[ ModuleIndex ].Address; */
    BaseAddr = (U32)g_MLC_IMAGEBASE[ ModuleIndex ];
	U32 ImgX, ImgY, ImgAddr, ImgWidth, ImgHeight, ImgBPP;

	// RGB0
	ImgX = RGB0_ImgX; ImgY = RGB0_ImgY;

	//========================================================
	s_RGB0_Address = BaseAddr;

	printk( "================================>\n");
	printk( "BaseAddr = %x\n", BaseAddr);
	printk( "================================>\n");
	// memory picture
	//volatile U32 * addr_reg = BaseAddr;
	addr_reg = (volatile U32 *)BaseAddr;
    addr_reg = (volatile U32 *)phys_to_virt((U32)addr_reg);
    printk("virt %p\n", addr_reg);
	U32 y,x;


	// V Stride : 2048*4

	U32 VStride = 2048*4;

	// initialize memory space for MLC
	for(y=0; y<2000; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0xFFFFFFFF;
		}
	}

	for(y=0; y<100; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x00FF0000;
		}
	}
	for(y=100; y<200; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x0000FF00;
		}
	}
	for(y=200; y<300; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x000000FF;
		}
	}

	for(y=300; y<400; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x00FF0000;
		}
	}
	for(y=400; y<500; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x0000FF00;
		}
	}
	for(y=600; y<700; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x000000FF;
		}
	}
	for(y=700; y<720; y++)
	{
		for(x=0; x<1280*sizeof(U32); x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x/sizeof(U32)) = 0x0000000F << ( 4*((y)%4) );
		}
	}

	for(y=0; y<720; y++)
	{
		for(x=1000; x<1100; x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x) = 0x000000FF;
		}
		for(x=1100; x<1200; x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x) = 0x0000FF00;
		}
		for(x=1200; x<1300; x++)
		{
			*(addr_reg + y*VStride/sizeof(U32) + x) = 0x00FF0000;
		}
	}
    printk("===> after draw\n");

	//------------
	// MLC Config
	//------------

    // GetDirtyFlag가 0이여야 함.
    if (0x00 != NX_MLC_GetTopDirtyFlag(ModuleIndex))	{ while(1); /*NX_CONSOLE_printk( "[MESLOG] *E, TOPMLC Error!!\n");    */ }
    if (0x00 != NX_MLC_GetDirtyFlag(ModuleIndex, 0))	{ while(1); /*NX_CONSOLE_printk( "[MESLOG] *E, FRISTRGB0 Error!!\n"); */ }
    if (0x00 != NX_MLC_GetDirtyFlag(ModuleIndex, 1))	{ while(1); /*NX_CONSOLE_printk( "[MESLOG] *E, SECONDRGB1 Error!!\n");*/ }
    if (0x00 != NX_MLC_GetDirtyFlag(ModuleIndex, 2))	{ while(1); /*NX_CONSOLE_printk( "[MESLOG] *E, ThRGB2 Error!!\n");    */ }
    if (0x00 != NX_MLC_GetDirtyFlag(ModuleIndex, 3))	{ while(1); /*NX_CONSOLE_printk( "[MESLOG] *E, VIDEO Error!!\n");     */ }
    // MLC Top Parameter

    U16 ScreenWith = Width;
    U16 ScreenHeight = Height;
    U32 DefaultColor = 0xFF00FF;


// Layer 우선 순위 결정
// 0 : video layer -> layer0 -> layer1 -> layer2
// 1 : layer0 -> video layer -> layer1 -> layer2
// 2 : layer0 -> layer1 -> video layer -> layer2
// 3 : layer0 -> layer1 -> layer2 -> video layer

	// Prim 0 :
    NX_MLC_SetMLCTopControlParameter(ModuleIndex,  CFALSE, CTRUE, 1, PRIM );
    NX_MLC_SetScreenSize(ModuleIndex,  ScreenWith, ScreenHeight );
    NX_MLC_SetBackground(ModuleIndex, DefaultColor );

    printk("%s: %d\n", __func__, __LINE__);
	//-----------------------------
	// RGB layer 0
    //-----------------------------
    // First Layer
    //S16 FirstLayerLeft=0;  S16 FirstLayerRight=Width-1;
    //S16 FirstLayerTop = 0; S16 FirstLayerBottom=Height-1;
    S16 FirstLayerLeft=0;  S16 FirstLayerRight=1280-1;
    S16 FirstLayerTop = 0; S16 FirstLayerBottom=720-1;

	U32	FirstLayerTpColor =	0x000000;
	U32	FirstLayerInvColor = 0x000000;
	U8	FirstLayerAlpha	= 0xFf;
	S32	FirstLayerVstride =	VStride; S32 FirstLayerHstride	= 4;
	U32	FirstLayerBaseAddress =	(U32)(s_RGB0_Address);

	NX_MLC_SetPosition(ModuleIndex, 0,  FirstLayerLeft,	FirstLayerTop,
									    FirstLayerRight, FirstLayerBottom);
	NX_MLC_SetRGB0LayerControlParameter(ModuleIndex,
										CTRUE, CFALSE,
										CFALSE, FirstLayerTpColor,
	                                    CFALSE, FirstLayerInvColor,
	                                    CFALSE, FirstLayerAlpha, // ALpha
										RGBFMT_B8G8R8,  LOCKSIZE_16 );

	NX_MLC_SetRGBLayerStride(ModuleIndex, 0,  FirstLayerHstride, FirstLayerVstride	);
	NX_MLC_SetRGBLayerAddress(ModuleIndex, 0, 	FirstLayerBaseAddress );

    printk("%s: %d\n", __func__, __LINE__);

	// Pixel Buffer Power On/Off
	NX_MLC_SetSRAMMODE(ModuleIndex, TOPMLC, SLEEPMODE);
	NX_MLC_SetSRAMMODE(ModuleIndex, TOPMLC, RUN);
	NX_MLC_SetSRAMMODE(ModuleIndex, VIDEO,  SLEEPMODE);
	NX_MLC_SetSRAMMODE(ModuleIndex, VIDEO,  RUN);

	NX_MLC_SetLayerRegFinish(ModuleIndex, TOPMLC);
	NX_MLC_SetLayerRegFinish(ModuleIndex, RGB0);
	NX_MLC_SetLayerRegFinish(ModuleIndex, RGB1);
	NX_MLC_SetLayerRegFinish(ModuleIndex, RGB2);
    NX_MLC_SetLayerRegFinish(ModuleIndex, VIDEO);

    printk("%s: %d\n", __func__, __LINE__);
	//---------
	// DPC Config
	//---------
 	//                 |<-SW->|
	// Sync   ---------+      +------------------------------+      +-
	//                 +------+                              +------+
	//          |<-FP->|      |<-BP->|<------AV------>|
	// Active --+                    +----------------+
	// Video    +--------------------+                +-----------------
	//
 	//                 |<-SW->|
	// HSync  ---------+      +------------------------------+      +-
	//                 +------+                              +------+
	//                 |<-VSP->|
	//                         +---------------------------------------
	// VSync ------------------+                     |<-VCP->|
	//       ----------------------------------------+
	// VSync                                         +-----------------
	U32 AVWidth  = Width;      // HDMI Link 의 규격에 맞도록 !
	U32 AVHeight = Height;
/* #if (CURRENT_RESOLUTION == RESOL_720P) */
#if 1
	U32 HSW      = 40;
	U32 HBP      = 220;
	U32 HFP      = 110;
	U32 VSW      = 5;
	U32 VBP      = 20;
	U32 VFP      = 5;
	U32 EvenVSW      = 0;
	U32 EvenVFP      = 0;
	U32 EvenVBP      = 0;
	U32 VSP         = 0;
	U32 VCP         = 0;
	U32 EvenVSP     = 0;
	U32 EvenVCP     = 0;
	U32 DelayRGB=0, DelayDE=7;
	U32 DelayHSync=0, DelayVSync=0;
#else
	U32 HFP      = 88;
	U32 HSW      = 48;
	U32 HBP      = 144;
	U32 VFP      = 4;
	U32 VSW      = 5;
	U32 VBP      = 36;
	U32 EvenVSW      = 0;
	U32 EvenVFP      = 0;
	U32 EvenVBP      = 0;
	U32 VSP         = 0;
	U32 VCP         = 0;
	U32 EvenVSP     = 0;
	U32 EvenVCP     = 0;
	U32 DelayRGB=0, DelayDE=7;
	U32 DelayHSync=0, DelayVSync=0;
#endif


	POLARITY FieldPolarity = POLARITY_ACTIVEHIGH;
	POLARITY HSyncPolarity = POLARITY_ACTIVEHIGH;
	POLARITY VSyncPolarity = POLARITY_ACTIVEHIGH;
	NX_DPC_SetSync ( ModuleIndex,
	                 PROGRESSIVE,
                     AVWidth, AVHeight, HSW, HFP, HBP, VSW, VFP, VBP,
					 FieldPolarity, HSyncPolarity, VSyncPolarity,
					 EvenVSW, EvenVFP, EvenVBP, VSP, VCP, EvenVSP, EvenVCP);

	NX_DPC_SetDelay ( ModuleIndex, DelayRGB, DelayHSync, DelayVSync, DelayDE );

	NX_DPC_SetOutputFormat( ModuleIndex, OUTPUTFORMAT_RGB888, 0 );

	QMODE RGB2YC = QMODE_256;
	QMODE YC2RGB = QMODE_256;
	NX_DPC_SetQuantizationMode(ModuleIndex, RGB2YC, YC2RGB );
    printk("%s: %d\n", __func__, __LINE__);
}


//**********************************************************************************************
//**********************************************************************************************



//*************************************************************************
//==========================
// HDMI Config : 1개인것을 알고 Config 해준다..
//==========================
//*************************************************************************
static U32 g_HDCP_auth_ack  = 0;
static U32 g_HDCP_I2C_Start = 0;

static U32 g_VSyncStart       = 0;
static U32 g_HActiveStart     = 0;
static U32 g_HActiveEnd       = 0;
static U32 g_VSyncHSStartEnd0 = 0;
static U32 g_VSyncHSStartEnd1 = 0;


void Set_HDMI()
{
	// HDMI 는 1개이다.
	U32 ModuleIndex = 0;
	U32 HFP ;
	U32 HSW ;
	U32 HBP ;
	U32 VFP ;
	U32 VSW ;
	U32 VBP ;
	U32 Width, Height;

	//========
	// @modified choiyk 2013-02-07 오후 3:44:20
	// Hot Plug 여부부터 확인해본다.
	// Reset을 풀어줄때 hot plug를 같이 넣어주므로, 여기서는 hot plug가 0이 되어야 한다. !
	//========
	g_hot_plug_detect_failed = CFALSE;
	if( NX_HDMI_GetReg( 0, HDMI_LINK_HPD_STATUS ) == 1 )
	{
		g_hot_plug_detect_failed = CTRUE;
	}
	else
	{
		g_hot_plug_detect_failed = CFALSE;
	}
    printk("=========> hot plug 0x%x\n", NX_HDMI_GetReg(0, HDMI_LINK_HPD_STATUS));

	//----------
	// PAD Enable  : HDMI Reset전에 PAD를 Enable 시켜서.. ( CEC 때문에 먼저 PAD를 Enable 시켜놓아야 한다. )
	//----------
	/* NX_HDMI_EnablePAD(  ModuleIndex, NX_HDMI_GetNumberOfPADMode( ModuleIndex ) ); // CEC는 입력을 받으므로 취약함. */


	// Set BaseAddr
	U32 BaseAddr = NX_HDMI_GetPhysicalAddress(ModuleIndex);
	NX_HDMI_SetBaseAddress(ModuleIndex, (U32)IO_ADDRESS(BaseAddr));


	//-----------------------------
	// HDMI Video Config
	// 160x120 Sync는 다 10
	// 일단 BLANK 등의 설정은 기존의 테스트벤치대로 하도록 한다.
	//-----------------------------
/* #if (CURRENT_RESOLUTION == RESOL_720P) */

#if  1

	Width    = 1280;
	Height   = 720;
	HFP      = 110;
	HSW      = 40;
	HBP      = 220;
	VFP      = 5;
	VSW      = 5;
	VBP      = 20;

	U32 g_H_BLANK = HFP + HSW + HBP;
	U32 g_V_BLANK = VFP + VSW + VBP;
	U32 g_V_ACTLINE  = Height;
	U32 V2_BLANK = Height + VFP + VSW + VBP;
	U32 V_LINE   = Height + VFP + VSW + VBP; // total
	U32 H_LINE   = Width  + HFP + HSW + HBP; // total
	U32 H_SYNC_START = HFP;
	U32 H_SYNC_END   = HFP + HSW;
	U32 V_SYNC_LINE_BEF_1 = VFP; 	// vsync start, end ? ???? 2012-12-24 오후 4:16:35
	U32 V_SYNC_LINE_BEF_2 = VFP + VSW;
#else // 1080 p
	Width    = 1920;
	Height   = 1080;
	HFP      = 88;
	HSW      = 48;
	HBP      = 144;
	VFP      = 4;
	VSW      = 5;
	VBP      = 36;

	U32 g_H_BLANK = HFP + HSW + HBP;
	U32 g_V_BLANK = VFP + VSW + VBP;
	U32 g_V_ACTLINE  = Height;
	U32 V2_BLANK = Height + VFP + VSW + VBP;
	U32 V_LINE   = Height + VFP + VSW + VBP; // total
	U32 H_LINE   = Width  + HFP + HSW + HBP; // total
	U32 H_SYNC_START = HFP;
	U32 H_SYNC_END   = HFP + HSW;
	U32 V_SYNC_LINE_BEF_1 = VFP; 	// vsync start, end ? ???? 2012-12-24 오후 4:16:35
	U32 V_SYNC_LINE_BEF_2 = VFP + VSW;

#endif

	NX_HDMI_SetReg( 0, HDMI_LINK_HDMI_CON_0, (0<<5)|(1<<4)); //NO BLUE SCREEN MODE, encoding order as is it .
	// 2013/06/27 pm1121 bluescreen is also working ! (checked by choiyk)
	// if you want to run with bluescreen,
	// > mw c0210000 31

	// BLUE SCREEN Mode의 경우는 Input Pixel을 Discard하고 BlueScreen Pixel을 내보낸다.
	// BLUE SCREEN Mode Pixel
	NX_HDMI_SetReg( 0, HDMI_LINK_BLUE_SCREEN_R_0, 0x5555);
	NX_HDMI_SetReg( 0, HDMI_LINK_BLUE_SCREEN_R_1, 0x5555);
	NX_HDMI_SetReg( 0, HDMI_LINK_BLUE_SCREEN_G_0, 0x5555);
	NX_HDMI_SetReg( 0, HDMI_LINK_BLUE_SCREEN_G_1, 0x5555);
	NX_HDMI_SetReg( 0, HDMI_LINK_BLUE_SCREEN_B_0, 0x5555);
	NX_HDMI_SetReg( 0, HDMI_LINK_BLUE_SCREEN_B_1, 0x5555);
	NX_HDMI_SetReg( 0, HDMI_LINK_HDMI_CON_1, 0); // no pixel repetition ( 720 to 1440 )
	NX_HDMI_SetReg( 0, HDMI_LINK_HDMI_CON_2, 0); // Video Preamble is applied( HDMI mode )
	// Audio buffer Overflow, UPDATA_RI_INT, AN_WRITE_INT, WATCHDOG_INT, I2C_INT Enable
	NX_HDMI_SetReg( 0, HDMI_LINK_INTC_CON_0, 0x53); // All Interutp Enable
	NX_HDMI_SetReg( 0, HDMI_LINK_STATUS_EN, 0x17);
	NX_HDMI_SetReg( 0, HDMI_LINK_HPD, 0); // Value of HPD signal : Unplugged
	NX_HDMI_SetReg( 0, HDMI_LINK_MODE_SEL, 2 ); // HDMI Mode Enable, DVI Mode Disable

	NX_HDMI_SetReg( 0, HDMI_LINK_H_BLANK_0, g_H_BLANK%256 + 2); // Blank Size [ 7: 0]
	NX_HDMI_SetReg( 0, HDMI_LINK_H_BLANK_1, g_H_BLANK>>8   ); // Blank Size [12: 8]
	NX_HDMI_SetReg( 0, HDMI_LINK_V1_BLANK_0, g_V_BLANK%256 ); // Blank Size [ 7: 0]
	NX_HDMI_SetReg( 0, HDMI_LINK_V1_BLANK_1, g_V_BLANK>>8  ); // Blank Size [12: 8]
	NX_HDMI_SetReg( 0, HDMI_LINK_V2_BLANK_0, V2_BLANK%256 ); // Blank Size [ 7: 0]
	NX_HDMI_SetReg( 0, HDMI_LINK_V2_BLANK_1, V2_BLANK>>8 ); // Blank Size [12: 8]
	NX_HDMI_SetReg( 0, HDMI_LINK_V_LINE_0, V_LINE%256 ); // v line
	NX_HDMI_SetReg( 0, HDMI_LINK_V_LINE_1, V_LINE>>8 );  //
	NX_HDMI_SetReg( 0, HDMI_LINK_H_LINE_0, H_LINE%256 ); // h line
	NX_HDMI_SetReg( 0, HDMI_LINK_H_LINE_1, H_LINE>>8 );  //

	NX_HDMI_SetReg( 0, HDMI_LINK_HSYNC_POL, 1 ); // active high
	NX_HDMI_SetReg( 0, HDMI_LINK_VSYNC_POL, 1 );  // active high
	NX_HDMI_SetReg( 0, HDMI_LINK_INT_PRO_MODE, 0 ); // progressive
	// Progressive의 경우 V_BLANK_F0, F1, F2.. 등은 사용되지 않는다.
	NX_HDMI_SetReg( 0, HDMI_LINK_H_SYNC_START_0, H_SYNC_START%256 ); // vsync start. ( Back Porch? )
	NX_HDMI_SetReg( 0, HDMI_LINK_H_SYNC_START_1, H_SYNC_START>>8 ); // vsync start. ( Back Porch? )
	NX_HDMI_SetReg( 0, HDMI_LINK_H_SYNC_END_0, H_SYNC_END%256 ); // vsync end. (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_H_SYNC_END_1, H_SYNC_END>>8 ); // vsync end.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_BEF_1_0, V_SYNC_LINE_BEF_1%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_BEF_1_1, V_SYNC_LINE_BEF_1>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_BEF_2_0, V_SYNC_LINE_BEF_2%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_BEF_2_1, V_SYNC_LINE_BEF_2>>8 ); // vsync end line number.  (  )
	U32 V_SYNC_LINE_AFT_1 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_2 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_3 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_4 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_5 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_6 = 0xFFFF;
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_1_0, V_SYNC_LINE_AFT_1%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_1_1, V_SYNC_LINE_AFT_1>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_2_0, V_SYNC_LINE_AFT_2%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_2_1, V_SYNC_LINE_AFT_2>>8 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_3_0, V_SYNC_LINE_AFT_3%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_3_1, V_SYNC_LINE_AFT_3>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_4_0, V_SYNC_LINE_AFT_4%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_4_1, V_SYNC_LINE_AFT_4>>8 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_5_0, V_SYNC_LINE_AFT_5%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_5_1, V_SYNC_LINE_AFT_5>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_6_0, V_SYNC_LINE_AFT_6%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_6_1, V_SYNC_LINE_AFT_6>>8 ); // vsync end line number.  (  )

	U32 V_SYNC_LINE_AFT_PXL_1 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_PXL_2 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_PXL_3 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_PXL_4 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_PXL_5 = 0xFFFF;
	U32 V_SYNC_LINE_AFT_PXL_6 = 0xFFFF;

	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_0, V_SYNC_LINE_AFT_PXL_1%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_1, V_SYNC_LINE_AFT_PXL_1>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_0, V_SYNC_LINE_AFT_PXL_2%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_1, V_SYNC_LINE_AFT_PXL_2>>8 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_0, V_SYNC_LINE_AFT_PXL_3%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_1, V_SYNC_LINE_AFT_PXL_3>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_0, V_SYNC_LINE_AFT_PXL_4%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_1, V_SYNC_LINE_AFT_PXL_4>>8 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_0, V_SYNC_LINE_AFT_PXL_5%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_1, V_SYNC_LINE_AFT_PXL_5>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_0, V_SYNC_LINE_AFT_PXL_6%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_1, V_SYNC_LINE_AFT_PXL_6>>8 ); // vsync end line number.  (  )

	U32 VACT_SPACE1 = 0xFFFF;
	U32 VACT_SPACE2 = 0xFFFF;
	U32 VACT_SPACE3 = 0xFFFF;
	U32 VACT_SPACE4 = 0xFFFF;
	U32 VACT_SPACE5 = 0xFFFF;
	U32 VACT_SPACE6 = 0xFFFF;

	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE1_0, VACT_SPACE1%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE1_1, VACT_SPACE1>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE2_0, VACT_SPACE2%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE2_1, VACT_SPACE2>>8 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE3_0, VACT_SPACE3%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE3_1, VACT_SPACE3>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE4_0, VACT_SPACE4%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE4_1, VACT_SPACE4>>8 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE5_0, VACT_SPACE5%256 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE5_1, VACT_SPACE5>>8 ); // vsync start line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE6_0, VACT_SPACE6%256 ); // vsync end line number.  (  )
	NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE6_1, VACT_SPACE6>>8 ); // vsync end line number.  (  )


	// first active space start line number
	// NX_HDMI_SetReg( 0, HDMI_LINK_VACT_SPACE_1_0, 0 );

	NX_HDMI_SetReg( 0, HDMI_LINK_CSC_MUX, 0 );  // ?
	NX_HDMI_SetReg( 0, HDMI_LINK_SYNC_GEN_MUX, 0 ); // ?

	// 일단 이게 뭐하는건지 모르므로..
	NX_HDMI_SetReg( 0, HDMI_LINK_SEND_START_0	, 0xfd ); // send_start ?
	NX_HDMI_SetReg( 0, HDMI_LINK_SEND_START_1	, 0x01 ); //
	NX_HDMI_SetReg( 0, HDMI_LINK_SEND_END_0		, 0x0d ); // send_end ?
	NX_HDMI_SetReg( 0, HDMI_LINK_SEND_END_1		, 0x3a ); //
	NX_HDMI_SetReg( 0, HDMI_LINK_SEND_END_2		, 0x08 ); //


	NX_HDMI_SetReg( 0, HDMI_LINK_DC_CONTROL, 0x00 ); // 00 : 8bits/pixel
	// Internal Video Pattern :
	// [1] 0 Ext Off, 1 Ext En
	// [0] 0 Disable, 1 Use Internally generated video pattern
	NX_HDMI_SetReg( 0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x00 );
	/* NX_HDMI_SetReg( 0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x01 ); */


	// //=============================
	// // Sync to HDMI Sync Format Converter Setting
	// //=============================
	// //@modified choiyk 2012-12-24 오전 11:13:51
	// //HDMI Sync Set. //일단 사용. - vsync만 일단 적당히 빨리 띄어주기.

	g_VSyncStart       = VSW + VBP + Height - 1;
	g_HActiveStart     = HSW + HBP ;
	//g_HActiveEnd     = Width + HSW + HBP ;
	g_HActiveEnd       = Width + HSW + HBP ;
	g_VSyncHSStartEnd0 = HSW + HBP + 1;
	g_VSyncHSStartEnd1 = HSW + HBP + 2;

	//-------------------------------------------------
    // HDCP Interrupt Handler Register ( func : InterruptHandler_HDMI )
    //-------------------------------------------------
	g_HDCP_auth_ack = 0;
	g_HDCP_I2C_Start = 0;
}


void Start_HDMI()
{
	//------------
	// HDMI system enable
	//------------
	U32 regvalue = NX_HDMI_GetReg( 0, HDMI_LINK_HDMI_CON_0 );
	regvalue = regvalue | 0x01; // HDMI system enable
	NX_HDMI_SetReg( 0, HDMI_LINK_HDMI_CON_0, regvalue  );

	// 모든 과정이 끝난후에 HDMI로 video를 넣어주어야 한다
	NX_DISPLAYTOP_HDMI_SetVSyncStart     ( g_VSyncStart    ); // from posedge VSync
	NX_DISPLAYTOP_HDMI_SetHActiveStart   ( g_HActiveStart  ); // from posedge HSync
	NX_DISPLAYTOP_HDMI_SetHActiveEnd     ( g_HActiveEnd    ); // from posedge HSync
	NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd( g_VSyncHSStartEnd0, g_VSyncHSStartEnd1 );

	return;
}

void Finish_HDMI()
{
	return;
}

//-------------------------
// HDMI InterruptHandler Function
//-------------------------
void HDCP_WATCHDOG_FUNC();
void HDCP_I2C_FUNC();
void HDCP_An_FUNC();
void HDCP_Ri_FUNC();
void HDCP_Authen_FUNC();

//==========
// SPDIF, 0~255 의 값을 DMA에서 PERI로 전송해준다.
//==========

// SPDIF Global Variable
static U32         g_SPDIF_DMA_Channel[NUMBER_OF_SPDIFTX_MODULE];
/* static NX_Memory1D g_SPDIF_DMA_MEM    [NUMBER_OF_SPDIFTX_MODULE]; */
static const U32   c_SPDIF_DMALength = 2048; // 8*256;



#if 0
void Set_SPDIF()
{

	NX_SPDIFTX_Initialize();

	U32 SPDIFTX_BaseAddress[NUMBER_OF_SPDIFTX_MODULE];

    U32  ModuleIndex ;
    for( ModuleIndex =0;  ModuleIndex <NX_SPDIFTX_GetNumberOfModule();  ModuleIndex++)
    {
  		// SPDIF ClockGen Set
  		U32 SPDIFTX_CLKGEN =  NX_SPDIFTX_GetClockNumber ( ModuleIndex ) ;


		NX_CLOCK_SetClockDivisorEnable( SPDIFTX_CLKGEN, CFALSE );

		// 빠른 테스트를 위해서 일단은 147MHz 그대로 사용한다.
		// 차후에는 실제 Clk에 맞추어서 바궈주어야 한다.
		// 일단 실제로 값을 넣어주게 한다.
		// 12분주를 하면 12Mhz가 나오고 이는 48KHz의 256배이다.
		// 이 값이 MCLK로 들어가면 된다. ( 48kHz * 256fs )

		// 일단 최대 주파수로.
		NX_CLKGEN_SetClockSource        ( SPDIFTX_CLKGEN, 0, 2);	// 147.456MHz
		NX_CLKGEN_SetClockDivisor       ( SPDIFTX_CLKGEN, 0, 4);	// 1분주 => 147.5MHz

		NX_CLKGEN_SetClockOutInv        ( SPDIFTX_CLKGEN, 0, 0); // CLK INV를 왜 시켜주지?
		// 2단이 없는데 왜..
		NX_CLKGEN_SetClockSource        ( SPDIFTX_CLKGEN, 1, 7);
		NX_CLKGEN_SetClockDivisor       ( SPDIFTX_CLKGEN, 1, 1<<2);
		NX_CLKGEN_SetClockOutInv        ( SPDIFTX_CLKGEN, 1, 1);
		NX_CLKGEN_SetClockDivisorEnable ( SPDIFTX_CLKGEN, 1);

  		// SPDIF Reset
        /* NX_RESET_EnterReset( NX_SPDIFTX_GetResetNumber( ModuleIndex, 0) ); */
        /* NX_RESET_LeaveReset( NX_SPDIFTX_GetResetNumber( ModuleIndex, 0) ); */
        NX_RSTCON_SetnRST( NX_SPDIFTX_GetResetNumber( ModuleIndex, 0), RSTCON_nENABLE );
        NX_RSTCON_SetnRST( NX_SPDIFTX_GetResetNumber( ModuleIndex, 0) RSTCON_nENABLE);

  		NX_CLOCK_SetClockDivisorEnable( NX_SPDIFTX_GetClockNumber ( ModuleIndex ), CTRUE );

  		// Set BaseAddress, PAD enable
  		SPDIFTX_BaseAddress[ModuleIndex] = NX_SPDIFTX_GetPhysicalAddress( (U32)ModuleIndex );
  		NX_SPDIFTX_SetBaseAddress( (U32)ModuleIndex, (U32)SPDIFTX_BaseAddress[ModuleIndex] );

		// SPDIF_CLKCON
		NX_SPDIFTX_SetSPDCLKCON( (U32)ModuleIndex, 0x0 );	// power off
		// SPDIF_CON
		U32 regvalue = 	(0x7<<19) | 	// FIFO level threshold			- 15-FIFO Level
					(0x1<<17) |     // FIFO transfer mode			- DMA transfer mode(2'b00)
					(0x0<<13) |     // endian format				- big endian(2'b00)
					(0x1<<12) |     // user_data_attach				- User data is stored in 23rd bit of audio data
					(0x0<<5) |      // software reset				- no
					(0x0<<3) |      // Main audio clock frequency	- 256fs/128fs - div2 : default
					(0x2<<1) |      // PCM data size				- 24bit
					(0x1<<0);       // PCM or stream				- PCM.
					// stream 데이터는 16bit이다!!!! 2013-02-21 오후 1:51:08

		NX_SPDIFTX_SetSPDCON( (U32)ModuleIndex, (U32)regvalue );

		// SPDIF_BSTAS
		regvalue = 0x0;	// reset value
		NX_SPDIFTX_SetSPDBSTAS( (U32)ModuleIndex, (U32)regvalue );

		// SPDIF_CSTAS
		regvalue = (0x0<<24) |  // 44.1kHz
		           (0x0<< 1)    // Linear-PCM
		           ;	// reset value : 0x0
		NX_SPDIFTX_SetSPDCSTAS( (U32)ModuleIndex, (U32)regvalue );

		// SPDIF Out Buffer에 쓰기.. ( 임시로 )
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
		NX_SPDIFTX_SetSPDDAT( (U32)ModuleIndex, (U32)0x00555555 );
	}
}

void Start_SPDIF()
{
    U32  ModuleIndex ;
    for( ModuleIndex =0;  ModuleIndex <NX_SPDIFTX_GetNumberOfModule();  ModuleIndex++)
    {
		// SPDIF_CLKCON
		NX_SPDIFTX_SetSPDCLKCON( (U32)ModuleIndex, 0x1 );	// power on
	}
}


void Finish_SPDIF()
{
    U32  ModuleIndex ;
    for( ModuleIndex =0;  ModuleIndex <NX_SPDIFTX_GetNumberOfModule();  ModuleIndex++)
    {
		// SPDIF_CLKCON
		NX_SPDIFTX_SetSPDCLKCON( (U32)ModuleIndex, 0x0 );	// power off

		// @todo
        // SPDIF DMA, Mem Free..

	}
}
#endif




