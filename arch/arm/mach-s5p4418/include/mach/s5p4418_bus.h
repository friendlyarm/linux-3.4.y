#ifndef __S5P4418_BUS_HEADER__
#define __S5P4418_BUS_HEADER__


#define NX_PA_BASE_REG_DREX         (0xC00E0000)
#define NX_PA_BASE_REG_DDRPHY       (0xC00E1000)
#define NX_DREX_QOS_OFFSET          (0x60)

#define NX_PA_BASE_REG_TIEOFF		(0xC0011000)
#define NX_TIEOFF_DREX_SLAVE_OFFSET	(0x60)

#define NX_VA_BASE_REG_DREX         IO_ADDRESS(NX_PA_BASE_REG_DREX)
#define NX_VA_BASE_REG_DDRPHY       IO_ADDRESS(NX_PA_BASE_REG_DDRPHY)
#define NX_VA_BASE_REG_TIEOFF		IO_ADDRESS(NX_PA_BASE_REG_TIEOFF)


#define NX_PL301_QOS_TRDMARK_OFFSET     (0x400)
#define NX_PL301_QOS_CTRL_OFFSET        (0x404)
#define NX_PL301_AR_OFFSET              (0x408)
#define NX_PL301_AW_OFFSET              (0x40C)

#define NX_PA_BASE_REG_PL301_DISP   (0xC005E000)
#define NX_PA_BASE_REG_PL301_BOTT   (0xC0050000)
#define NX_PA_BASE_REG_PL301_TOP    (0xC0052000)

#define NX_VA_BASE_REG_PL301_DISP   IO_ADDRESS(NX_PA_BASE_REG_PL301_DISP)
#define NX_VA_BASE_REG_PL301_BOTT   IO_ADDRESS(NX_PA_BASE_REG_PL301_BOTT)
#define NX_VA_BASE_REG_PL301_TOP    IO_ADDRESS(NX_PA_BASE_REG_PL301_TOP)

#define NX_BASE_REG_PL301_DISP_QOS_TRDMARK  (NX_VA_BASE_REG_PL301_DISP | NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_REG_PL301_DISP_QOS_CTRL     (NX_VA_BASE_REG_PL301_DISP | NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_REG_PL301_DISP_AR           (NX_VA_BASE_REG_PL301_DISP | NX_PL301_AR_OFFSET)
#define NX_BASE_REG_PL301_DISP_AW           (NX_VA_BASE_REG_PL301_DISP | NX_PL301_AW_OFFSET)

#define NX_BASE_REG_PL301_BOTT_QOS_TRDMARK  (NX_VA_BASE_REG_PL301_BOTT | NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_REG_PL301_BOTT_QOS_CTRL     (NX_VA_BASE_REG_PL301_BOTT | NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_REG_PL301_BOTT_AR           (NX_VA_BASE_REG_PL301_BOTT | NX_PL301_AR_OFFSET)
#define NX_BASE_REG_PL301_BOTT_AW           (NX_VA_BASE_REG_PL301_BOTT | NX_PL301_AW_OFFSET)

#define NX_BASE_REG_PL301_TOP_QOS_TRDMARK   (NX_VA_BASE_REG_PL301_TOP | NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_REG_PL301_TOP_QOS_CTRL      (NX_VA_BASE_REG_PL301_TOP | NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_REG_PL301_TOP_AR            (NX_VA_BASE_REG_PL301_TOP | NX_PL301_AR_OFFSET)
#define NX_BASE_REG_PL301_TOP_AW            (NX_VA_BASE_REG_PL301_TOP | NX_PL301_AW_OFFSET)

#define SI_IF_NUM_POS   0
#define SLOT_NUM_POS    24

// Interface slot number of  TOP_BUS.
#define TOPBUS_SI_SLOT_DMAC0        0
#define TOPBUS_SI_SLOT_MP2TS        1
#define TOPBUS_SI_SLOT_DMAC1        2
#define TOPBUS_SI_SLOT_REMAP        3
#define TOPBUS_SI_SLOT_SDMMC        4
#define TOPBUS_SI_SLOT_USBOTG       5
#define TOPBUS_SI_SLOT_USBHOST0     6
#define TOPBUS_SI_SLOT_USBHOST1     7

// Interface slot number of  BOTTOM_BUS.
#define BOTBUS_SI_SLOT_1ST_ARM      0
#define BOTBUS_SI_SLOT_2ND_ARM      1
#define BOTBUS_SI_SLOT_MALI         2
#define BOTBUS_SI_SLOT_TOP          3
#define BOTBUS_SI_SLOT_DEINTERLACE  4
#define BOTBUS_SI_SLOT_1ST_CODA     5
#define BOTBUS_SI_SLOT_2ND_CODA     6
#define BOTBUS_SI_SLOT_SCALER       7

// Interface slot number of  DISPLAY_BUS.
#define DISBUS_SI_SLOT_1ST_DISPLAY  0
#define DISBUS_SI_SLOT_2ND_DISPLAY  1
#define DISBUS_SI_SLOT_GMAC         2

/*------------------------------------------------------------------------------*/
typedef struct {
    U32 QOSCONTROL;
    U32 PAD_0;

} NX_DREX_QOS;

typedef struct {
    U32 CONCONTROL;         // 0x00
    U32 MEMCONTROL;         // 0x04
    U32 MEMCONFIG[2];       // 0x08 ~ 0x0C
    U32 DIRECTCMD;          // 0x10
    U32 PRECHCONFIG;        // 0x14
    U32 PHYCONTROL[4];      // 0x18 ~ 0x24
    U32 PWRDNCONFIG;        // 0x28
    U32 TIMINGPZQ;          // 0x2C
    U32 TIMINGAREF;         // 0x30
    U32 TIMINGROW;          // 0x34
    U32 TIMINGDATA;         // 0x38
    U32 TIMINGPOWER;        // 0x3C
    U32 PHYSTATUS;          // 0x40
    U32 PAD_0[1];           // 0x44
    U32 CHIPSTATUS;         // 0x48
    U32 PAD_1[2];           // 0x4C ~ 0x50
    U32 MRSTATUS;           // 0x54
    U32 PAD_2[2];           // 0x58 ~ 0x5C
    NX_DREX_QOS QOS[16];    // 0x60 ~ 0xDC
    U32 PAD_19[5];          // 0xE0 ~ 0xF0

    U32 WRTRA_CONFIG;       // 0xF4
    U32 RDLVL_CONFIG;       // 0xF8
    U32 PAD_20[1];          // 0xFC

    U32 BRBRSVCONTROL;      // 0x100
    U32 BRBRSVCONFIG;       // 0x104
    U32 BRBQOSCONFIG;       // 0x108
    U32 MEMBASECONFIG[2];   // 0x10C ~ 0x110
    U32 PAD_21[3];          // 0x114 ~ 0x11C

    U32 WRLVL_CONFIG[2];    // 0x120 ~ 0x124
    U32 WRLVL_STATUS[2];    // 0x128
    U32 PAD_22[9];          // 0x12C ~ 0x14C

    U32 CTRL_IO_RDATA;      // 0x150
    U32 PAD_23[3];          // 0x154 ~ 0x15C

    U32 CACAL_CONFIG[2];    // 0x160 ~ 0x164
    U32 CACAL_STATUS;       // 0x168

} NX_DREX_REG, *PNX_DREX_REG;


extern const u8		g_DrexBRB_RD[];
extern const u8		g_DrexBRB_WR[];
extern const u16	g_DrexQoS[];
extern const u8     g_TopBusSI[];
extern const u8     g_TopQoSSI[];
extern const u8     g_BottomBusSI[];
extern const u8     g_BottomQoSSI[];
extern const u8     g_DispBusSI[];


#endif  //__S5P4418_BUS_HEADER__

