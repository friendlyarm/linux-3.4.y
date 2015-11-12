//------------------------------------------------------------------------------
//
//  Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//  Module      :
//  File        : nx_DWC_GMAC.c
//  Description :
//  Author      :
//  History     :
//------------------------------------------------------------------------------
#include "nx_chip.h"
#include "nx_dwc_gmac.h"

static      NX_DWC_GMAC_RegisterSet * __g_pRegister[2][NUMBER_OF_DWC_GMAC_MODULE];
volatile    EMAC_HANDLE_T *hEmac[NUMBER_OF_DWC_GMAC_MODULE];

const char _default_mac_addr[6] = {00, 01, 02, 03, 04, 05};


static const char *_str_mdio_reg(int reg)
{
    switch (reg)
    {
        case REG_MII_BMCR:
            return "REG_MII_BMCR";
        case REG_MII_BMSR:
            return "REG_MII_BMSR";
        case REG_MII_PHYSID1:
            return "REG_MII_PHYSID1";
        case REG_MII_PHYSID2:
            return "REG_MII_PHYSID2";
        case REG_MII_ADVERTISE:
            return "REG_MII_ADVERTISE";
        case REG_MII_EXPANSION:
            return "REG_MII_EXPANSION";
        case REG_MII_CTRL1000:
            return "REG_MII_CTRL1000";
        case REG_MII_STAT1000:
            return "REG_MII_STAT1000";
        case REG_MII_ESTATUS:
            return "REG_MII_ESTATUS";
        case REG_MII_DCOUNTER:
            return "REG_MII_DCOUNTER";
        case REG_MII_FCSCOUNTER:
            return "REG_MII_FCSCOUNTER";
        case REG_MII_NWAYTEST:
            return "REG_MII_NWAYTEST";
        case REG_MII_RERRCOUNTER:
            return "REG_MII_RERRCOUNTER";
        case REG_MII_SREVISION:
            return "REG_MII_SREVISION";
        case REG_MII_RESV1:
            return "REG_MII_RESV1";
        case REG_MII_LBRERROR:
            return "REG_MII_LBRERROR";
        case REG_MII_PHYADDR:
            return "REG_MII_PHYADDR";
        case REG_MII_RESV2:
            return "REG_MII_RESV2";
        case REG_MII_TPISTATUS:
            return "REG_MII_TPISTATUS";
        case REG_MII_NCONFIG:
            return "REG_MII_NCONFIG";
        default:
            return "Unkown MII Register";
    }
}

static const char *_str_duplex_mode(int mode)
{
    switch (mode)
    {
        case E_EMAC_HALFDUPLEX:
            return "Half Duplex";
        case E_EMAC_FULLDUPLEX:
            return "Full Duplex";
        default:
            return "Invalid Duplex Mode";
    }
}


static const char *_str_link_speed(int speed)
{
    switch (speed)
    {
        case E_EMAC_SPEED_1000M:
            return "1000 BaseT";
        case E_EMAC_SPEED_100M:
            return "100 BaseT";
        case E_EMAC_SPEED_10M:
            return "10 BaseT";
        default:
            return "Invalid Link Speed";
    }
}


//------------------------------------------------------------------------------
//
//  DWC_GMAC Interface
//
//------------------------------------------------------------------------------
void NX_DWC_GMAC_CSR_REGS_DUMP(U32 ModuleIndex)
{
    NX_CONSOLE_Printf("\n\t====================================================================================\n");
    NX_CONSOLE_Printf("\t EMAC CSR Registers Dump\n");
    NX_CONSOLE_Printf("\t====================================================================================\n");
    U32 i= 0;

    NX_CONSOLE_Printf("REG_MAC_CONFIG           %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG          ]);
    NX_CONSOLE_Printf("REG_MAC_FRAME_FILTER     %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER    ]);
    NX_CONSOLE_Printf("REG_MAC_HASH_HIGH        %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_HASH_HIGH       ]);
    NX_CONSOLE_Printf("REG_MAC_HASH_LOW         %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_HASH_LOW        ]);
    NX_CONSOLE_Printf("REG_MAC_MII_ADDR         %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR        ]);
    NX_CONSOLE_Printf("REG_MAC_MII_DATA         %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_DATA        ]);
    NX_CONSOLE_Printf("REG_MAC_FLOW_CONTROL     %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FLOW_CONTROL    ]);
    NX_CONSOLE_Printf("REG_MAC_VLAN_TAG         %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_VLAN_TAG        ]);
    NX_CONSOLE_Printf("REG_MAC_VERSION          %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_VERSION         ]);
    NX_CONSOLE_Printf("REG_MAC_DEBUG            %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_DEBUG           ]);
    NX_CONSOLE_Printf("REG_MAC_REMOTE_WAKEUP    %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_REMOTE_WAKEUP   ]);
    NX_CONSOLE_Printf("REG_MAC_PMT_CTRL_STS     %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_PMT_CTRL_STS    ]);
    NX_CONSOLE_Printf("REG_MAC_INT_STATUS       %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_INT_STATUS      ]);
    NX_CONSOLE_Printf("REG_MAC_INT_MASK         %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_INT_MASK        ]);
    NX_CONSOLE_Printf("REG_MAC_ADDR0_HIGH       %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_HIGH      ]);
    NX_CONSOLE_Printf("REG_MAC_ADDR0_LOW        %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_LOW       ]);
    NX_CONSOLE_Printf("REG_MAC_ADDR1_HIGH       %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR1_HIGH      ]);
    NX_CONSOLE_Printf("REG_MAC_ADDR1_LOW        %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR1_LOW       ]);
    NX_CONSOLE_Printf("REG_MAC_AN_CTRL          %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_AN_CTRL         ]);
    NX_CONSOLE_Printf("REG_MAC_AN_STATUS        %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_AN_STATUS       ]);
    NX_CONSOLE_Printf("REG_MAC_AN_ADVTSMNT      %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_AN_ADVTSMNT     ]);
    NX_CONSOLE_Printf("REG_MAC_AN_LINKP_ABILITY %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_AN_LINKP_ABILITY]);
    NX_CONSOLE_Printf("REG_MAC_AN_EXPANSION     %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_AN_EXPANSION    ]);
    NX_CONSOLE_Printf("REG_MAC_TBI_STATUS       %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TBI_STATUS      ]);
    NX_CONSOLE_Printf("REG_MAC_SRGMII_STATUS    %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_SRGMII_STATUS   ]);
    NX_CONSOLE_Printf("REG_MMC_CONTROL          %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MMC_CONTROL         ]);
    NX_CONSOLE_Printf("REG_MAC_TS_CTRL          %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TS_CTRL         ]);
    NX_CONSOLE_Printf("REG_MAC_SS_INC           %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_SS_INC          ]);
    NX_CONSOLE_Printf("REG_MAC_TS_HIGH          %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TS_HIGH         ]);
    NX_CONSOLE_Printf("REG_MAC_TS_LOW           %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TS_LOW          ]);
    NX_CONSOLE_Printf("REG_MAC_TS_HIGH_UPDTE    %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TS_HIGH_UPDTE   ]);
    NX_CONSOLE_Printf("REG_MAC_TS_LOW_UPDTE     %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TS_LOW_UPDTE    ]);
    NX_CONSOLE_Printf("REG_MAC_TS_ADDED         %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TS_ADDED        ]);
    NX_CONSOLE_Printf("REG_MAC_TTIME_HIGH       %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TTIME_HIGH      ]);
    NX_CONSOLE_Printf("REG_MAC_TTIME_LOW        %02d %08x\n", i++, __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_TTIME_LOW       ]);

}
void NX_DWC_GMAC_DMA_REGS_DUMP(U32 ModuleIndex)
{
    NX_CONSOLE_Printf("\n\t====================================================================================\n");
    NX_CONSOLE_Printf("\t EMAC DMA Registers Dump\n");
    NX_CONSOLE_Printf("\t====================================================================================\n");
    U32 i= 0;

    NX_CONSOLE_Printf("REG_DMA_BUS_MODE        %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_BUS_MODE        ]);
    NX_CONSOLE_Printf("REG_DMA_XMT_POLL_DEMAND %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_XMT_POLL_DEMAND ]);
    NX_CONSOLE_Printf("REG_DMA_RCV_POLL_DEMAND %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RCV_POLL_DEMAND ]);
    NX_CONSOLE_Printf("REG_DMA_RX_BASE_ADDR    %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RX_BASE_ADDR    ]);
    NX_CONSOLE_Printf("REG_DMA_TX_BASE_ADDR    %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_TX_BASE_ADDR    ]);
    NX_CONSOLE_Printf("REG_DMA_STATUS          %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS          ]);
    NX_CONSOLE_Printf("REG_DMA_OPMODE          %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE          ]);
    NX_CONSOLE_Printf("REG_DMA_INT_ENB         %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_INT_ENB         ]);
    NX_CONSOLE_Printf("REG_DMA_MF_BO_CNT       %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_MF_BO_CNT       ]);
    NX_CONSOLE_Printf("REG_DMA_RINT_WDT        %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RINT_WDT        ]);
    NX_CONSOLE_Printf("REG_DMA_AXI_BUS_MODE    %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_AXI_BUS_MODE    ]);
    NX_CONSOLE_Printf("REG_DMA_AXI_STATUS      %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_AXI_STATUS      ]);
    NX_CONSOLE_Printf("REG_DMA_CUR_TX_DESC     %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_CUR_TX_DESC     ]);
    NX_CONSOLE_Printf("REG_DMA_CUR_RX_DESC     %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_CUR_RX_DESC     ]);
    NX_CONSOLE_Printf("REG_DMA_CUR_TX_BUF_ADDR %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_CUR_TX_BUF_ADDR ]);
    NX_CONSOLE_Printf("REG_DMA_CUR_RX_BUF_ADDR %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_CUR_RX_BUF_ADDR ]);
    NX_CONSOLE_Printf("REG_HW_FEATURE          %02d %08x\n", i++, __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_HW_FEATURE          ]);
}
//------------------------------------------------------------------------------
/**
 *  @brief  It is a sample function
 *  @return a register value
 */

 //------------------------------------------------------------------------------
/// @name   DWC_GMAC CSR Interface
//------------------------------------------------------------------------------
/*
CBOOL   NX_DWC_GMAC_CSR_Init( U32 ModuleIndex )
{
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = 0x880c;
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER] = 0x80000000;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_BUS_MODE] = 0x20100;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RX_BASE_ADDR] = (U32)&hEmac[ModuleIndex];
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_MAC_FLOW_CONTROL] = 2;

    return CTRUE;
}
*/

U64 NX_DWC_GMAC_CSR_GetMacAddr (U32 ModuleIndex)
{
    U64 hi_addr, lo_addr;
    U64 RET = 0;

    /* Read the MAC address from the hardware */

    hi_addr = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_HIGH];
    lo_addr = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_LOW];
    hi_addr &= 0x0000FFFFUL;

    RET = (hi_addr << 32) | lo_addr;
    return RET;
}

/*
CBOOL   NX_DWC_GMAC_CSR_AUTONEGOTIATION(U32 ModuleIndex, EMAC_HANDLE_T *handle, U32 timeout_ms)
{
    unsigned short temp_value;

    while(1) {
        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_BMSR, &temp_value);

        if (temp_value & BIT_BMSR_ANEGCOMPLETE)
            break;
    }
    NX_CONSOLE_Printf("[NXLOG] Auto Negotiation Success.\n" );
    return CTRUE;
}
*/

/*
void    NX_DWC_GMAC_CSR_FLUSH_TX(U32 ModuleIndex, EMAC_DMA_DESC_T *dtx)
{
    NX_DWC_GMAC_DMA_STOP_RX(ModuleIndex);
    NX_DWC_GMAC_DMA_STOP_TX(ModuleIndex);
    NX_DWC_GMAC_CSR_DISABLE(ModuleIndex);

    U32 REG_OPMODE = __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE];
    REG_OPMODE |= BIT_DMA_OPMODE_FLUSH_TX;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] = REG_OPMODE;

    while(__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] & BIT_DMA_OPMODE_FLUSH_TX);

    dtx->des0.tx.own = 0;

    NX_DWC_GMAC_DRV_INIT(ModuleIndex);

}
*/

void    NX_DWC_GMAC_CSR_ENABLE(U32 ModuleIndex)
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    U32 temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG];

    temp_value |= BIT_CONFIG_TE;
    temp_value |= BIT_CONFIG_RE;
    temp_value |= BIT_CONFIG_DM;
    temp_value |= BIT_CONFIG_DO;
    //temp_value |= BIT_CONFIG_SARC3;

    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] =  temp_value;
}

void    NX_DWC_GMAC_CSR_DISABLE(U32 ModuleIndex)
{
    U32 temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG];

    temp_value &= ~BIT_CONFIG_TE;
    temp_value &= ~BIT_CONFIG_RE;
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] =  temp_value;
}


CBOOL    NX_DWC_GMAC_CSR_TX_ERRCHK(U32 ModuleIndex, const EMAC_DMA_DESC_T *p)
{
    // @ todo
    return CTRUE;
}
CBOOL    NX_DWC_GMAC_CSR_RX_ERRCHK(U32 ModuleIndex, const EMAC_DMA_DESC_T *p)
{
    // @ todo
    return CTRUE;
}

U32   NX_DWC_GMAC_GetID( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    return __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_VERSION];
}

void    NX_DWC_GMAC_CSR_SetReceiveAll( U32 ModuleIndex, CBOOL Enable )
{
    const U32   RA_POS  = 31;
    const U32   RA_MASK = 1UL<<RA_POS;

    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    register U32 regVal;

    regVal  = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER];
    regVal &= ~RA_MASK;
    regVal |= Enable << RA_POS;

    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER] = regVal;
}

//------------------------------------------------------------------------------
/// @name   DWC_GMAC DMA Interface
//------------------------------------------------------------------------------

CBOOL   NX_DWC_GMAC_DMA_SWReset( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_BUS_MODE] = 0x1;

    while((__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_BUS_MODE] & 0x1));

    return CTRUE;
}

CBOOL   NX_DWC_GMAC_DMA_Init( U32 ModuleIndex )
{
    S32 i;
    void *rx_buf, *tx_buf;
    EMAC_DMA_DESC_T *rx_desc, *tx_desc;

    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET)
             | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    // DMA bus mode
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_BUS_MODE]  = ( BIT_DMA_BM_8xPBL
                                                                      | 16<<BIT_DMA_BM_RPBL_SHIFT
                                                                      | 16<<BIT_DMA_BM_PBL_SHIFT  );

    // Disable Int
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_INT_ENB]   = 0x0;

    // Setup DMA to Ring mode
    hEmac[ModuleIndex]->dma_rx_desc_offset = 0;
    hEmac[ModuleIndex]->dma_tx_desc_offset = 0;

    for( i=0; i<CONFIG_DMA_RX_SIZE; i++ )
    {
        rx_desc = (EMAC_DMA_DESC_T*)&(hEmac[ModuleIndex]->dma_rx_desciptor[i]);
        rx_buf  = (void*)hEmac[ModuleIndex]->dma_rx_buffer[i];

        // Initialize the contersts of the DMA buffers
//        NX_DWC_GMAC_DMA_INIT_RX_DESC( rx_desc, rx_buf, ((i==(CONFIG_DMA_RX_SIZE-1))?1:0));
#if EMAC_TX_RX_DUMP
        NX_CONSOLE_Printf("[NXLOG] rx_desc_addr : %x\n", rx_desc );
        NX_CONSOLE_Printf("[NXLOG] rx_buf       : %x\n", rx_buf  );
        NX_DWC_GMAC_DMA_DISPLAY_DESC_RING("Rx", rx_desc);
#endif
    }

    for( i=0; i<CONFIG_DMA_TX_SIZE; i++ )
    {
        tx_desc = (EMAC_DMA_DESC_T*)&(hEmac[ModuleIndex]->dma_tx_desciptor[i]);
        tx_buf  = (void*)hEmac[ModuleIndex]->dma_tx_buffer[i];

        // Initialize the contersts of the DMA buffers
//      NX_DWC_GMAC_DMA_INIT_TX_DESC( tx_desc, ((i==(CONFIG_DMA_TX_SIZE-1))?1:0));
//      NX_DWC_GMAC_DMA_INIT_TX_DESC( tx_desc, ((i==(1-1))?1:0));
#if EMAC_TX_RX_DUMP
        NX_CONSOLE_Printf("[NXLOG] tx_desc_addr : %x\n", tx_desc );
        NX_CONSOLE_Printf("[NXLOG] tx_buf       : %x\n", tx_buf  );
        NX_DWC_GMAC_DMA_DISPLAY_DESC_RING("Tx", tx_desc);
#endif
    }

    // The base address of the RX/TX descriptor
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_TX_BASE_ADDR] = (U32)&(hEmac[ModuleIndex]->dma_tx_desciptor[0]);
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RX_BASE_ADDR] = (U32)&(hEmac[ModuleIndex]->dma_rx_desciptor[0]);

    // DMA operation mode
    //REG  = __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE];
    //REG |= (BIT_DMA_OPMODE_TX_FORCE | BIT_DMA_OPMODE_RX_FORCE);
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] = 1<<BIT_DMA_OPMODE_TTC_SHIFT;

    // DMA Interrupt enable
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_INT_ENB] = ( BIT_DMA_INT_NIT | BIT_DMA_INT_AIE
                                                                    | BIT_DMA_INT_RUE | BIT_DMA_INT_TUE );

    // DMA AXI Bus
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_AXI_BUS_MODE] = ( 3<<BIT_DMA_ABM_WR_OSR_LMT_SHIFT
                                                                         | 3<<BIT_DMA_ABM_RD_OSR_LMT_SHIFT );

    return CTRUE;
}

void    NX_DWC_GMAC_DMA_DISPLAY_DESC_RING( const char *str, volatile const EMAC_DMA_DESC_T * p)
{
    NX_CONSOLE_Printf("[%s] : desc0 = 0x%08x, desc1 = 0x%08x, buffer1 = 0x%08x, buffer2 = 0x%08x\n",
			                  str, p->des0.data, p->des1.data, p->des2, p->des3);
}

void    NX_DWC_GMAC_DMA_INIT_RX_DESC(volatile EMAC_DMA_DESC_T *p, void *buffer, int end_ring)
{
    p->des0.data = p->des1.data = 0;

    p->des0.rx.own = 1;
    p->des1.rx.buffer1_size = PKTSIZE_ALIGN;
    p->des1.rx.disable_ic = 1;
    p->des1.rx.end_ring = end_ring;
    p->des2 = buffer;
    p->des3 = 0;
}

void    NX_DWC_GMAC_DMA_START_RX(U32 ModuleIndex)
{
    U32 value;

    value = __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE];
    value |= BIT_DMA_OPMODE_RX_START;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] = value;

}

/*
void    NX_DWC_GMAC_DMA_STOP_RX(U32 ModuleIndex)
{
    U32 value;

    value = __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE];
    value &= ~BIT_DMA_OPMODE_RX_START;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] = value;
}
*/

/*
void    NX_DWC_GMAC_DMA_INIT_TX_DESC(volatile EMAC_DMA_DESC_T *p, int end_ring)
{
    p->des0.data = p->des1.data = 0;

#if DESC_ENHANCED_FORMAT
    p->des0.tx.end_ring = end_ring;
#else
    p->des1.tx.end_ring = end_ring;
#endif // DESC_ENHANCED_FORMAT

    p->des2 = NULL;
    p->des3 = NULL;
}
*/

void    NX_DWC_GMAC_DMA_START_TX(U32 ModuleIndex)
{
    U32 value = __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE];
    value |= BIT_DMA_OPMODE_TX_START;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] = value;
}

/*
void    NX_DWC_GMAC_DMA_STOP_TX(U32 ModuleIndex)
{
    U32 value = __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE];
    value &= ~BIT_DMA_OPMODE_RX_START;
    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_OPMODE] = value;
}
*/

void    NX_DWC_GMAC_DMA_TxPollDemand( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET)
             | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_XMT_POLL_DEMAND] = 0x0;
}

void    NX_DWC_GMAC_DMA_RxPollDemand( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET)
             | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RCV_POLL_DEMAND] = 0x0;
}




//------------------------------------------------------------------------------
/// @name   DWC_GMAC MII Interface
//------------------------------------------------------------------------------
/*
CBOOL NX_DWC_GMAC_MII_IS_POLL_BUSY(U32 ModuleIndex, U32 TIMEOUT_MS)
{
    while(1) {
        //if (!(macr_read(REG_MAC_MII_ADDR) & BIT_MIIADR_BUSY))
        if(!(__g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR] & BIT_MIIADR_BUSY))
        {
            return CTRUE;
        }
    }
    return CTRUE;
}
*/

/*
CBOOL NX_DWC_GMAC_MII_WRITE(U32 ModuleIndex, U16 PHY_DEV_ID, U32 REG, U32 VAL)
{
    U32 MII_ADDR;
    MII_ADDR = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR];
    MII_ADDR &= BIT_MIIADR_CLK_MASK;
    MII_ADDR |= ((PHY_DEV_ID & BIT_MIIADR_PA_MASK) << BIT_MIIADR_PA_SHIFT) | ((REG & BIT_MIIADR_REG_MASK) << BIT_MIIADR_REG_SHIFT) | BIT_MIIADR_WRITE | BIT_MIIADR_BUSY;

    if (NX_DWC_GMAC_MII_IS_POLL_BUSY (ModuleIndex, EMAC_BUSY_TIMEOUT))
        return CFALSE;

    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_DATA] = VAL;
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR] = MII_ADDR;

    if (NX_DWC_GMAC_MII_IS_POLL_BUSY (ModuleIndex, EMAC_BUSY_TIMEOUT))
        return CFALSE;

    return CTRUE;
}
*/

/*
CBOOL NX_DWC_GMAC_MII_READ(U32 ModuleIndex, U16 PHY_DEV_ID, U32 REG, U16 *VAL)
{
    U32 MII_ADDR;
    MII_ADDR = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR];
    MII_ADDR &= BIT_MIIADR_CLK_MASK;
    MII_ADDR |= ((PHY_DEV_ID & BIT_MIIADR_PA_MASK) << BIT_MIIADR_PA_SHIFT) | ((REG & BIT_MIIADR_REG_MASK) << BIT_MIIADR_REG_SHIFT) | BIT_MIIADR_BUSY;

    if (NX_DWC_GMAC_MII_IS_POLL_BUSY (ModuleIndex, EMAC_BUSY_TIMEOUT))
            return CFALSE;

    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR] = MII_ADDR;

    if (NX_DWC_GMAC_MII_IS_POLL_BUSY (ModuleIndex, EMAC_BUSY_TIMEOUT))
        return CFALSE;

    VAL = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_DATA] = VAL;

    return CTRUE;
}
*/

//------------------------------------------------------------------------------
/// @name   DWC_GMAC PHY Interface
//------------------------------------------------------------------------------
/*
CBOOL   NX_DWC_GMAC_PHY_LINKUP(U32 ModuleIndex, EMAC_HANDLE_T *handle)
{
    unsigned short temp_value;
#if _SIM_
#else
    NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_SPECIFIC_STATUS, &temp_value);
    if ( (temp_value & BIT_SPCIFIC_LINK_MASK) == BIT_SPCIFIC_LINK_DOWN )
        return 0;

    NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_BMSR, &temp_value);
    if (!(temp_value & BIT_BMSR_LSTATUS))
        return 0;

#endif

    return 1;
}
CBOOL   NX_DWC_GMAC_PHY_INIT(U32 ModuleIndex, EMAC_HANDLE_T *handle)
{
    // @ todo
#if _SIM_
        handle->phy_dev_id = 1;
        handle->duplex_mode = E_EMAC_FULLDUPLEX;
        handle->link_speed = E_EMAC_SPEED_100M;
#else

        int phy_dev_id;
        unsigned short temp_value1, temp_value2;

        if ((phy_dev_id = NX_DWC_GMAC_PHY_FIND(handle, PHY_ID)) < 0)
            return CFALSE;

        handle->phy_dev_id = phy_dev_id;

        // phy reset

        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_BMCR, &temp_value1);
        temp_value1 |= BIT_BMCR_RESET;
        NX_DWC_GMAC_PHY_WRITE(ModuleIndex, handle->phy_dev_id, REG_MII_BMCR, temp_value1);
        //wait_msec(100L);                          // MUST BE DELAY HERE

        // Advertise Setting
        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_BMSR, &temp_value1);
        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_ADVERTISE, &temp_value2);
        if (temp_value1 & BIT_BMSR_100FULL)
            temp_value2 |= BIT_ADVERTISE_100FULL;
        if (temp_value1 & BIT_BMSR_100HALF)
            temp_value2 |= BIT_ADVERTISE_100HALF;
        if (temp_value1 & BIT_BMSR_10FULL)
            temp_value2 |= BIT_ADVERTISE_10FULL;
        if (temp_value1 & BIT_BMSR_10HALF)
            temp_value2 |= BIT_ADVERTISE_10HALF;
        NX_DWC_GMAC_PHY_WRITE(ModuleIndex, handle->phy_dev_id, REG_MII_ADVERTISE, temp_value2);
        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_ADVERTISE, &temp_value2);
        NX_CONSOLE_Printf("REG_MII_ADVERTISE = 0x%04X\n", temp_value2);

        // Wait Auto Negotiation
        if (NX_DWC_GMAC_CSR_AUTONEGOTIATION(ModuleIndex, handle, EMAC_AUTO_NEGO_TIMEOUT) < 0)
            return CFALSE;

        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_SPECIFIC_STATUS, &temp_value1);

        switch (temp_value1 & BIT_SPCIFIC_SPEED_MASK)
        {
            case BIT_SPCIFIC_SPEED_1000:
                NX_CONSOLE_Printf("1000M Linked\n");
                handle->link_speed = E_EMAC_SPEED_1000M;
                break;
            case BIT_SPCIFIC_SPEED_100:
                NX_CONSOLE_Printf("100M Linked\n");
                handle->link_speed = E_EMAC_SPEED_100M;
                break;
            default:
                NX_CONSOLE_Printf("Unkown Linked\n");
            case BIT_SPCIFIC_SPEED_10:
                NX_CONSOLE_Printf("10M Linked\n");
                handle->link_speed = E_EMAC_SPEED_10M;
                break;
        }

        switch (temp_value1 & BIT_SPCIFIC_DPX_MASK)
        {
            case BIT_SPCIFIC_DPX_FULL:
                NX_CONSOLE_Printf("Full Duplex\n");
                handle->duplex_mode = E_EMAC_FULLDUPLEX;
                break;
            default:
                NX_CONSOLE_Printf("Unkown Linked\n");
            case BIT_SPCIFIC_DPX_HALF:
                NX_CONSOLE_Printf("Full Duplex\n");
                handle->duplex_mode = E_EMAC_HALFDUPLEX;
                break;
        }

        // Copper Duplex Mode Set
        NX_DWC_GMAC_PHY_READ(ModuleIndex, handle->phy_dev_id, REG_MII_BMCR, &temp_value2);
        if (handle->duplex_mode == E_EMAC_HALFDUPLEX)
            temp_value2 &= ~BIT_BMCR_FULLDPLX;
        else
            temp_value2 |= BIT_BMCR_FULLDPLX;
        NX_DWC_GMAC_PHY_WRITE(ModuleIndex, handle->phy_dev_id, REG_MII_BMCR, temp_value2);
#endif

    return CTRUE;
}
*/

/*
CBOOL   NX_DWC_GMAC_PHY_FIND(U32 ModuleIndex, unsigned short id)
{
    unsigned short phy_id1, phy_id2;
    int phy_dev_id;

    for (phy_dev_id=0; phy_dev_id<32; phy_dev_id++)
    {
        NX_DWC_GMAC_MII_READ(ModuleIndex, phy_dev_id, REG_MII_PHYSID1, &phy_id1);
        NX_CONSOLE_Printf("Phy [%02d] : phy_id1 = 0x%04X\n", phy_dev_id, phy_id1);

        NX_DWC_GMAC_MII_READ(ModuleIndex, phy_dev_id, REG_MII_PHYSID2, &phy_id2);
        NX_CONSOLE_Printf("Phy [%02d] : phy_id2 = 0x%04X\n", phy_dev_id, phy_id2);

        if (phy_id1 == id)
        {
            NX_CONSOLE_Printf("PHY Found : ID1=0x%04X, ID2=0x%04X\n", phy_id1, phy_id2);
            // @ todo
            //handle->phy_id1 = phy_id1;
            //handle->phy_id2 = phy_id2;
            return phy_dev_id;
        }
    }

    NX_CONSOLE_Printf("Can not find PHY\n");
    // @ todo
    //handle->phy_id1 = -1;
    //handle->phy_id2 = -1;
    return CFALSE;
}
*/
/*
CBOOL   NX_DWC_GMAC_PHY_WRITE(U32 ModuleIndex, U16 ADDR, U8 reg, U16 *VAL)
{
    NX_ASSERT(NX_DWC_GMAC_MII_READ(ModuleIndex, ADDR, reg, VAL) == CTRUE);
    return CTRUE;
}
CBOOL   NX_DWC_GMAC_PHY_READ(U32 ModuleIndex, U16 ADDR, U8 reg, U16 VAL)
{
    NX_ASSERT(NX_DWC_GMAC_MII_READ(ModuleIndex, ADDR, reg, VAL) == CTRUE);

    return CTRUE;
}
*/

CBOOL   NX_DWC_GMAC_MDC_DIV(U32 ModuleIndex, U32 VAL)
{
    U32 temp_data;
    temp_data = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR];
    temp_data &= ~BIT_MIIADR_CLK_MASK;
    temp_data |= (VAL & 0x0f) << 2;
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_MII_ADDR] =  temp_data;
}
//------------------------------------------------------------------------------
/// @name   DWC_GMAC Extern Driver Function
//------------------------------------------------------------------------------
CBOOL NX_DWC_GMAC_DRV_GET_MAC_ADDRESS(U32 ModuleIndex, U8 *addr)
{
    U32 hi_addr, lo_addr;

    /* Read the MAC address from the hardware */
    hi_addr = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_HIGH];
    lo_addr = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_LOW];
    hi_addr &= 0x0000FFFFUL;

    addr[0] = (lo_addr >> 0 ) & 0xFFu;
    addr[1] = (lo_addr >> 8 ) & 0xFFu;
    addr[2] = (lo_addr >> 16) & 0xFFu;
    addr[3] = (lo_addr >> 24) & 0xFFu;
    addr[4] = (hi_addr >> 0 ) & 0xFFu;
    addr[5] = (hi_addr >> 8 ) & 0xFFu;
    return CTRUE;
}

void  NX_DWC_GMAC_DRV_SET_MAC_ADDRESS(U32 ModuleIndex, U8 *Addr)
{
    U64 data;
    data = (Addr[5] << 8) | Addr[4];

    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_HIGH] = data;
    data = (Addr[3] << 24) | (Addr[2] << 16) | (Addr[1] << 8) | Addr[0];
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_ADDR0_LOW] = data;

    memcpy((U8*)hEmac[ModuleIndex]->mac_addr, Addr, 6);
}

/*
void NX_DWC_GMAC_DRV_HALT(U32 ModuleIndex)
{
    NX_DWC_GMAC_DMA_STOP_RX(ModuleIndex);
    NX_DWC_GMAC_DMA_STOP_TX(ModuleIndex);
    NX_DWC_GMAC_CSR_DISABLE(ModuleIndex);

    memset(hEmac[ModuleIndex], 0, sizeof(EMAC_HANDLE_T));
}
*/

CBOOL NX_DWC_GMAC_DRV_INIT(U32 ModuleIndex, EMAC_HANDLE_T * phEmacStorage )
{
    U8 temp_mac[6];
    U32 temp_value;

    //hEmac[ModuleIndex] = (EMAC_HANDLE_T*)hEmacStorage[ModuleIndex].Address;
    hEmac[ModuleIndex] = phEmacStorage;

    NX_CONSOLE_Printf("[NXLOG] sizeof (EMAC_HANDLE_T) = %d (0x%08X)\n", sizeof(EMAC_HANDLE_T), sizeof(EMAC_HANDLE_T));
    NX_CONSOLE_Printf("[NXLOG] hEmac[%d] = 0x%08X\n", ModuleIndex, hEmac[ModuleIndex]);

    memset((U8*)hEmac[ModuleIndex], 0, sizeof(EMAC_HANDLE_T));
    memcpy((U8*)hEmac[ModuleIndex]->mac_addr, _default_mac_addr, 6);

    // HW Sequence 0 : Phy Reset Deassert
	// HW Sequence 1 : Phy Give Tx / Rx Clock to EMAC
	// HW Sequence 2 : EMAC Reset Deassert
	// HW Sequence 3 : REG_DMA_BUS_MODE:0 SWR to 0
	// if HW Sequence 3 is not activate....check HW Reset Signals...

    // (0) wait for Phy-SW Reset De-Assert & Hardware Reset(RSTCON) & Clock Divisor
    while (__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_BUS_MODE] & BIT_DMA_BM_SWR);

    // (1) Check EMAC RTL Version
    temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_VERSION];
    NX_CONSOLE_Printf("[NXLOG]EMAC Init : RTL version (%d.%d), Core version (%d.%d)\n",
                ((temp_value & 0xf0) >> 4), (temp_value & 0xf),
                ((temp_value & 0xf000) >> 12), ((temp_value & 0xf00) >> 8));

    // (2) DMA Reset
    NX_DWC_GMAC_DMA_SWReset(ModuleIndex);

    // (3) set MAC Address
    if(NX_DWC_GMAC_DRV_GET_MAC_ADDRESS(ModuleIndex, temp_mac) == CTRUE)
    {
        if (!memcmp((U8*)hEmac[ModuleIndex]->mac_addr, temp_mac, 6))
            memcpy((U8*)hEmac[ModuleIndex]->mac_addr, temp_mac, 6);
    }

    NX_DWC_GMAC_DRV_SET_MAC_ADDRESS(ModuleIndex, (U8*)hEmac[ModuleIndex]->mac_addr);
    NX_CONSOLE_Printf("[NXLOG]MAC Address Set %02X:%02X:%02X : %02X:%02X:%02X\n",
                hEmac[ModuleIndex]->mac_addr[0], hEmac[ModuleIndex]->mac_addr[1], hEmac[ModuleIndex]->mac_addr[2],
                hEmac[ModuleIndex]->mac_addr[3], hEmac[ModuleIndex]->mac_addr[4], hEmac[ModuleIndex]->mac_addr[5]);

    hEmac[ModuleIndex]->valid = CTRUE;

//    NX_DWC_GMAC_MDC_DIV(ModuleIndex, 0x02);

//    // (4) phy init
//    if(NX_DWC_GMAC_PHY_INIT(ModuleIndex, hEmac[ModuleIndex]) == CFALSE)
//        return CFALSE;

//    // (5) Link Check
//    if(NX_DWC_GMAC_PHY_LINKUP(ModuleIndex, hEmac[ModuleIndex]))
//    {
//        NX_CONSOLE_Printf("[NXLOG]Ethernet Link Up\n");
//        hEmac[ModuleIndex]->link_status = CTRUE;
//    }
//    else
//    {
//        hEmac[ModuleIndex]->link_status = CFALSE;
//        NX_CONSOLE_Printf("[NXLOG]Ethernet Link Down\n");
//        return CFALSE;
//    }

//    NX_CONSOLE_Printf("EMAC Linked : %s / %s\n", _str_duplex_mode(hEmac[ModuleIndex]->duplex_mode), _str_link_speed(hEmac[ModuleIndex]->link_speed));

    // (6) If Linked, then Setup DMA
    NX_DWC_GMAC_DMA_Init(ModuleIndex);

//    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_HASH_HIGH] = 0;
//    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_HASH_LOW] = 0;

//    if (hEmac[ModuleIndex]->duplex_mode == E_EMAC_HALFDUPLEX)
//    {
//        temp_value = BIT_CONFIG_HALFDUPLEX_INIT_VALUE;
//        temp_value |= BIT_CONFIG_IFG_64BT;
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = temp_value;

//        temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FLOW_CONTROL];
//        temp_value &= ~(BIT_FC_RFE | BIT_FC_TFE);
//        temp_value |= BIT_FC_FCB_BPA;
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FLOW_CONTROL] = temp_value;
//    }
//    else
//    {
//        temp_value = BIT_CONFIG_FULLDUPLEX_INIT_VALUE;
//        temp_value |= BIT_CONFIG_IFG_80BT;
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = temp_value;

//        temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FLOW_CONTROL];
//        temp_value |= (BIT_FC_RFE | BIT_FC_TFE);
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FLOW_CONTROL] = temp_value;
//    }

//    if (hEmac[ModuleIndex]->link_speed == E_EMAC_SPEED_1000M)
//    {
//        //macr_write(REG_MAC_CONFIG, (macr_read(REG_MAC_CONFIG) & (~BIT_CONFIG_PS)));
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] & (~BIT_CONFIG_PS);
//    }
//    else if (hEmac[ModuleIndex]->link_speed == E_EMAC_SPEED_100M)
//    {
//        //macr_write(REG_MAC_CONFIG, (macr_read(REG_MAC_CONFIG) | BIT_CONFIG_FES | BIT_CONFIG_PS));
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] | BIT_CONFIG_FES | BIT_CONFIG_PS;
//    }
//    else
//    {
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] & (~BIT_CONFIG_FES);
//        __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_CONFIG] |BIT_CONFIG_PS;
//    }

//#if 1
//    temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER];
//    temp_value |= E_BIT_BROADF_DISABLE;
//    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER] = temp_value;
//#elif 0
//    temp_value = __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER];
//    //temp_value |= E_BIT_MCFILTER_RCV_ALL;// | E_BIT_PMCOUS_ENABLE;
//    temp_value |= E_BIT_PMCOUS_ENABLE;
//    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_FRAME_FILTER] = temp_value;
//#endif

    // DWC GMAC CSR Setup
    NX_DWC_GMAC_CSR_ENABLE( ModuleIndex );
    NX_DWC_GMAC_CSR_SetReceiveAll( ModuleIndex, CTRUE );

    // CSR Interrupt Mask
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MAC_INT_MASK]      = 0x00000001;

    // MMC Interrupt Mask
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MMC_INT_MASK_RX]   = 0xFFFFFFFF;   // RX
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MMC_INT_MASK_TX]   = 0xFFFFFFFF;   // TX
    __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET[REG_MMC_INT_MASK_IPC]  = 0xFFFFFFFF;   // IPC

    return CTRUE;
}

CBOOL NX_DWC_GMAC_DRV_TX(U32 ModuleIndex, volatile void *data, U32 len)
{
    int tx_counter, end_ring;
    volatile EMAC_DMA_DESC_T *dtx;
    //int start_tick, end_tick, diff_tick, timeout_ms;

    //timeout_ms = EMAC_BUSY_TIMEOUT;

    tx_counter = hEmac[ModuleIndex]->dma_tx_desc_offset;
    dtx = &(hEmac[ModuleIndex]->dma_tx_desciptor[tx_counter]);

#if 1
    if (len > PKTSIZE_ALIGN)
    {
        NX_CONSOLE_Printf("Data Size Over (%d / %d)\n", len, PKTSIZE_ALIGN);
        return CFALSE;
    }

    memset(&hEmac[ModuleIndex]->dma_tx_buffer[tx_counter], 0,    (len > 64 ? len : 64));
    memcpy( hEmac[ModuleIndex]->dma_tx_buffer[tx_counter], data, (len > 64 ? len : 64));
//#if EMAC_TX_RX_DUMP
    NX_CONSOLE_Printf("[NXLOG] dma_tx_buffer address : %08x, %08x\n",
                        &hEmac[ModuleIndex]->dma_tx_buffer[tx_counter], hEmac[ModuleIndex]->dma_tx_buffer[tx_counter] );
//#endif

#else
    arm_dcache_flush(data, len);
#endif

    // Wait Previous Frame Send Done
    //end_tick = start_tick = systime_gettick();
    while (1)
    {
        if (!dtx->des0.tx.own)
            break;

        //end_tick = systime_gettick();
        //diff_tick = end_tick - start_tick;

        //if (diff_tick > timeout_ms)
        //{
        //  NX_CONSOLE_Printf("Previous Tx was not finished .....Now Force Stop it\n");
        //  _emac_core_flush_tx((EMAC_DMA_DESC_T *)dtx);
        //  return CFALSE;
        //}
    }

    //dtx->des2 = (void *)data;
    //dtx->des3 = (void *)data;
    dtx->des2 = (void *)(hEmac[ModuleIndex]->dma_tx_buffer[tx_counter]);
    dtx->des3 = (void *)(hEmac[ModuleIndex]->dma_tx_buffer[tx_counter]);
//#if EMAC_TX_RX_DUMP
    NX_CONSOLE_Printf("[NXLOG] dma_tx_buffer address : %08x, %08x\n", hEmac[ModuleIndex]->dma_tx_buffer[tx_counter], dtx->des2);
//#endif

    /* Clean and set the TX descriptor */
//#if DESC_ENHANCED_FORMAT
//    end_ring = dtx->des0.tx.end_ring;
//    dtx->des0.data = dtx->des1.data = 0;
//    dtx->des0.tx.interrupt = 1;
//    //dtx->des1.tx.checksum_insert = 3;
//    dtx->des0.tx.first_segment = 1;
//    dtx->des0.tx.last_segment = 1;
//    dtx->des0.tx.end_ring = end_ring;
//    dtx->des1.tx.buffer1_size = len;
//    dtx->des1.tx.buffer2_size = 0;
//    dtx->des0.tx.own = 1;
//#else
//    end_ring = dtx->des1.tx.end_ring;
//    dtx->des0.data = dtx->des1.data = 0;
//    dtx->des1.tx.interrupt = 1;
//    //dtx->des1.tx.checksum_insert = 3;
//    dtx->des1.tx.first_segment = 1;
//    dtx->des1.tx.last_segment = 1;
//    dtx->des1.tx.end_ring = end_ring;
//    //dtx->des1.tx.buffer1_size = len;
//    dtx->des1.tx.buffer1_size = 0;
//    //dtx->des1.tx.buffer2_size = 0;
//    dtx->des1.tx.buffer2_size = len;
//    dtx->des0.tx.own = 1;
//#endif  // DESC_ENHANCED_FORMAT

    // 임시로 강제값을 할당해본다
    dtx->des0.data = 0xB0000000;
    dtx->des1.data = 0x00000010;

//#if EMAC_TX_RX_DUMP
    {
        int ij;

        NX_CONSOLE_Printf("\n\n[TX]\n");

        for (ij=0; ij<len; ij++)
            NX_CONSOLE_Printf("%02X", hEmac[ModuleIndex]->dma_tx_buffer[tx_counter][ij]);

        NX_CONSOLE_Printf("\n\n");
    }
//#endif

//    /* CSR1 enables the transmit DMA to check for new descriptor */
//    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS] = BIT_DMA_STS_TI;
//    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_XMT_POLL_DEMAND] =  1;


#if EMAC_TEST_DEBUG
    /* advance to the next RX descriptor (for next time) */

#if DESC_ENHANCED_FORMAT
    if (dtx->des0.tx.end_ring)
#else
    if (dtx->des1.tx.end_ring)
#endif // DESC_ENHANCED_FORMAT
        hEmac[ModuleIndex]->dma_tx_desc_offset = 0;             /* wrap, to first */
    else
        hEmac[ModuleIndex]->dma_tx_desc_offset++;               /* advance to next */

    return 0;
#else
    // Wait while sending frame
    //start_tick = systime_gettick();

//#if EMAC_TX_RX_DUMP
#if 1
    U32 i;
    for( i=0; i<CONFIG_DMA_TX_SIZE; i++ )
    {
        void *tx_buf;
        EMAC_DMA_DESC_T *tx_desc;

        tx_desc = (EMAC_DMA_DESC_T*)(&(hEmac[ModuleIndex]->dma_tx_desciptor[i]));
        tx_buf  = (void*)hEmac[ModuleIndex]->dma_tx_buffer[i];

        NX_CONSOLE_Printf("[NXLOG] tx_desc_addr : %x\n", tx_desc );
        NX_CONSOLE_Printf("[NXLOG] tx_buf       : %x\n", tx_buf  );

        // Initialize the contersts of the DMA buffers
        NX_DWC_GMAC_DMA_DISPLAY_DESC_RING("Tx", tx_desc);
        //NX_DWC_GMAC_DMA_REGS_DUMP( ModuleIndex );
    }
#endif

    NX_DWC_GMAC_DMA_START_TX( ModuleIndex );

    // 여기서 멈춤
//    while (1)
    {
//        if (__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS] & BIT_DMA_STS_TI)
//            break;

//        if (__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS] & BIT_DMA_STS_EB_MASK)
//        {
//            NX_CONSOLE_Printf("Tx Error (status = 0x%08X)\n", __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS]);
//            //print_bits(__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS], "REG_DMA_STATUS");
//        }

        //NX_DWC_GMAC_DMA_REGS_DUMP( ModuleIndex );
        //NX_DWC_GMAC_CSR_REGS_DUMP( ModuleIndex );

    }

    //return _emac_core_tx_error_check((EMAC_DMA_DESC_T *)dtx);
    return CTRUE;
#endif
}


CBOOL NX_DWC_GMAC_DRV_RX(U32 ModuleIndex, volatile void **data, U32 *len)
{
    int rx_counter = 0;
    int frame_len = 0;
    volatile EMAC_DMA_DESC_T *drx;

    // select the RX descriptor to use
    rx_counter = hEmac[ModuleIndex]->dma_rx_desc_offset;
    drx = &hEmac[ModuleIndex]->dma_rx_desciptor[rx_counter];

    if ((rx_counter < 0) || (rx_counter >= CONFIG_DMA_RX_SIZE))
    {
        NX_CONSOLE_Printf ("%s: [dma drx = 0x%x, cur_rx=%d]\n", __FUNCTION__, (unsigned int) drx, rx_counter);
    }

    /*
    if (!(drx->des0.rx.own) && (drx->des0.rx.last_descriptor))
    {
        //NX_CONSOLE_Printf("RX descriptor ring:\n");

        // Check if the frame was not successfully received
        if (NX_DWC_GMAC_CSR_RX_ERRCHK (ModuleIndex, (EMAC_DMA_DESC_T *)drx) < 0)
        {
            drx->des0.rx.own = 1;
        }
        else if (drx->des0.rx.first_descriptor && drx->des0.rx.last_descriptor)
        {
            // FL (frame length) indicates the length in byte including the CRC
            frame_len = drx->des0.rx.frame_length;
            if ((frame_len >= 0) && (frame_len <= PKTSIZE_ALIGN))
            {
#if 0
                const unsigned char *p = hEmac[ModuleIndex]->dma_rx_buffer[rx_counter];
                NX_CONSOLE_Printf("\nRX[%d]:  0x%08x ", rx_counter, p);
                NX_CONSOLE_Printf("DA=%02x:%02x:%02x:%02x:%02x:%02x",
                    p[0], p[1], p[2], p[3], p[4], p[5]);
                p+=6;
                NX_CONSOLE_Printf(" SA=%02x:%02x:%02x:%02x:%02x:%02x",
                    p[0], p[1], p[2], p[3], p[4], p[5]);
                p+=6;
                NX_CONSOLE_Printf(" Type=%04x\n", p[0]<<8|p[1]);
#endif
                *data = &hEmac[ModuleIndex]->dma_rx_buffer[rx_counter];
                *len = frame_len;

                //arm_dcache_flush(&hEmac[ModuleIndex]->dma_rx_buffer[rx_counter], frame_len);

                //memory_dump(*data, frame_len, "Net Rx");


#if EMAC_TX_RX_DUMP
                {
                    int i;
                    NX_CONSOLE_Printf("\n\n[RX]\n");

                    for (i=0; i<frame_len; i++)
                        NX_CONSOLE_Printf("%02X", hEmac[ModuleIndex]->dma_rx_buffer[rx_counter][i]);

                    NX_CONSOLE_Printf("\n\n");
                }
#endif
            }
            else
            {
                NX_CONSOLE_Printf ("%s: Framelen %d too long\n", __FUNCTION__, frame_len);
            }
            drx->des0.rx.own = 1;
            //NX_CONSOLE_Printf ("%s: frame received \n", __FUNCTION__);
        }
        else
        {
            NX_CONSOLE_Printf ("%s: very long frame received\n", __FUNCTION__);
        }

        // advance to the next RX descriptor (for next time)
        if (drx->des1.rx.end_ring)
            hEmac[ModuleIndex]->dma_rx_desc_offset = 0;             // wrap, to first
        else
            hEmac[ModuleIndex]->dma_rx_desc_offset++;               // advance to next
    }
    else
    {
        __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_RCV_POLL_DEMAND] =  0; // request input
    }
    */

    drx->des0.data = 0x80000000;
    drx->des1.data = 0x00000010;
    drx->des2 = (void *)(hEmac[ModuleIndex]->dma_rx_buffer[0]);
    drx->des3 = (void *)(hEmac[ModuleIndex]->dma_rx_buffer[1]);

    NX_DWC_GMAC_DMA_START_RX( ModuleIndex );

    if (frame_len > 0)
        return CTRUE;
    else
        return CFALSE;
}


//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *  @brief  Initialize of prototype enviroment & local variables.
 *  @return \b CTRUE    indicate that Initialize is successed.\n
 *          \b CFALSE   indicate that Initialize is failed.
 *  @see    NX_DWC_GMAC_GetNumberOfModule
 */
CBOOL   NX_DWC_GMAC_Initialize( void )
{
    static CBOOL bInit = CFALSE;

    if( CFALSE == bInit )
    {
        memset( __g_pRegister, 0, sizeof(__g_pRegister) );
        bInit = CTRUE;
    }

    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get number of modules in the chip.
 *  @return     Module's number. \n
 *              It is equal to NUMBER_OF_DWC_GMAC_MODULE in <nx_chip.h>.
 *  @see        NX_DWC_GMAC_Initialize
 */
U32     NX_DWC_GMAC_GetNumberOfModule( void )
{
    return NUMBER_OF_DWC_GMAC_MODULE;
}

U32     NX_DWC_GMAC_GetNumberOfChennel( void )
{
    return NUMBER_OF_DWC_GMAC_CHANNEL;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a size, in byte, of register set.
 *  @return     Size of module's register set.
 *  @see        NX_DWC_GMAC_GetPhysicalAddress,
 *              NX_DWC_GMAC_SetBaseAddress,         NX_DWC_GMAC_GetBaseAddress,
 *              NX_DWC_GMAC_OpenModule,             NX_DWC_GMAC_CloseModule,
 *              NX_DWC_GMAC_CheckBusy,
 */
U32     NX_DWC_GMAC_GetSizeOfRegisterSet( void )
{
return sizeof( *__g_pRegister );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set a base address of register set.
 *  @param[in]  BaseAddress Module's base address
 *  @return     None.
 *  @see        NX_DWC_GMAC_GetPhysicalAddress,     NX_DWC_GMAC_GetSizeOfRegisterSet,
 *              NX_DWC_GMAC_GetBaseAddress,
 *              NX_DWC_GMAC_OpenModule,             NX_DWC_GMAC_CloseModule,
 *              NX_DWC_GMAC_CheckBusy,
 */
void    NX_DWC_GMAC_SetBaseAddress( U32 ModuleIndex, U32 ChannelIndex, U32* BaseAddress )
{
    NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    __g_pRegister[ChannelIndex][ModuleIndex] = (NX_DWC_GMAC_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a base address of register set
 *  @return     Module's base address.
 *  @see        NX_DWC_GMAC_GetPhysicalAddress,     NX_DWC_GMAC_GetSizeOfRegisterSet,
 *              NX_DWC_GMAC_SetBaseAddress,
 *              NX_DWC_GMAC_OpenModule,             NX_DWC_GMAC_CloseModule,
 *              NX_DWC_GMAC_CheckBusy,
 */
U32*    NX_DWC_GMAC_GetBaseAddress( U32 ModuleIndex, U32 ChannelIndex  )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    return (U32*)__g_pRegister[ChannelIndex][ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get module's physical address.
 *  @return     Module's physical address. \n
 *              It is equal to PHY_BASEADDR_DWC_GMAC?_MODULE in <nx_chip.h>.
 *  @see        NX_DWC_GMAC_GetSizeOfRegisterSet,
 *              NX_DWC_GMAC_SetBaseAddress,         NX_DWC_GMAC_GetBaseAddress,
 *              NX_DWC_GMAC_OpenModule,             NX_DWC_GMAC_CloseModule,
 *              NX_DWC_GMAC_CheckBusy,
 */
U32     NX_DWC_GMAC_GetPhysicalAddress( U32 ModuleIndex, U32 ChannelIndex )
{
    const U32 PhysicalAddr[2][NUMBER_OF_DWC_GMAC_MODULE] =
    {
        //PHY_BASEADDR_LIST( DWC_GMAC )
        PHY_BASEADDR_WITH_CHANNEL_LIST( DWC_GMAC, APB0 ),
        PHY_BASEADDR_WITH_CHANNEL_LIST( DWC_GMAC, APB1 )
    };
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    return  PhysicalAddr[ChannelIndex][ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *  @brief      Initialize selected modules with default value.
 *  @return     \b CTRUE    indicate that Initialize is successed. \n
 *              \b CFALSE   indicate that Initialize is failed.
 *  @see        NX_DWC_GMAC_GetPhysicalAddress,     NX_DWC_GMAC_GetSizeOfRegisterSet,
 *              NX_DWC_GMAC_SetBaseAddress,         NX_DWC_GMAC_GetBaseAddress,
 *              NX_DWC_GMAC_CloseModule,
 *              NX_DWC_GMAC_CheckBusy,
 */
CBOOL   NX_DWC_GMAC_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Deinitialize selected module to the proper stage.
 *  @return     \b CTRUE    indicate that Deinitialize is successed. \n
 *              \b CFALSE   indicate that Deinitialize is failed.
 *  @see        NX_DWC_GMAC_GetPhysicalAddress,     NX_DWC_GMAC_GetSizeOfRegisterSet,
 *              NX_DWC_GMAC_SetBaseAddress,         NX_DWC_GMAC_GetBaseAddress,
 *              NX_DWC_GMAC_OpenModule,
 *              NX_DWC_GMAC_CheckBusy,
 */
CBOOL   NX_DWC_GMAC_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
    return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether the selected modules is busy or not.
 *  @return     \b CTRUE    indicate that Module is Busy. \n
 *              \b CFALSE   indicate that Module is NOT Busy.
 *  @see        NX_DWC_GMAC_GetPhysicalAddress,     NX_DWC_GMAC_GetSizeOfRegisterSet,
 *              NX_DWC_GMAC_SetBaseAddress,         NX_DWC_GMAC_GetBaseAddress,
 *              NX_DWC_GMAC_OpenModule,             NX_DWC_GMAC_CloseModule,
 */
CBOOL   NX_DWC_GMAC_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
    return CFALSE;
}

//------------------------------------------------------------------------------
//  clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *  @brief      Get module's clock index.
 *  @return     Module's clock index.\n
 *              It is equal to CLOCKINDEX_OF_DWC_GMAC?_MODULE in <nx_chip.h>.
 *  @see        NX_CLKGEN_SetClockDivisorEnable,
 *              NX_CLKGEN_GetClockDivisorEnable,
 *              NX_CLKGEN_SetClockSource,
 *              NX_CLKGEN_GetClockSource,
 *              NX_CLKGEN_SetClockDivisor,
 *              NX_CLKGEN_GetClockDivisor
 */
U32 NX_DWC_GMAC_GetClockNumber ( U32 ModuleIndex )
{
    const U32 ClockNumber[] =
    {
        CLOCKINDEX_LIST( DWC_GMAC )
    };
    NX_CASSERT( NUMBER_OF_DWC_GMAC_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    return  ClockNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get module's reset index.
 *  @return     Module's reset index.\n
 *              It is equal to RESETINDEX_OF_DWC_GMAC?_MODULE_i_nRST in <nx_chip.h>.
 *  @see        NX_RSTCON_Enter,
 *              NX_RSTCON_Leave,
 *              NX_RSTCON_GetStatus
 */
U32 NX_DWC_GMAC_GetResetNumber ( U32 ModuleIndex )
{
    const U32 ResetNumber[] =
    {
        RESETINDEX_LIST( DWC_GMAC, aresetn_i )
    };
    NX_CASSERT( NUMBER_OF_DWC_GMAC_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    return  ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number for the interrupt controller.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     A interrupt number.\n
 *              It is equal to INTNUM_OF_DWC_GMAC?_MODULE in <nx_chip.h>.
 *  @see        NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber
 */
U32     NX_DWC_GMAC_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[NUMBER_OF_DWC_GMAC_MODULE] = { INTNUM_LIST( DWC_GMAC ) };

    NX_CASSERT( NUMBER_OF_DWC_GMAC_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );

    return InterruptNumber[ModuleIndex];
}

#if 0
//------------------------------------------------------------------------------
/**
 *  @brief      Set a specified interrupt to be enabled or disabled.
 *  @param[in]  ModuleIndex     an index of module.
 *  @param[in]  IntNum  a interrupt Number .\n
 *                      refer to NX_DWC_GMAC_INTCH_xxx in <nx_DWC_GMAC.h>
 *  @param[in]  Enable  \b Set as CTRUE to enable a specified interrupt. \r\n
 *                      \b Set as CFALSE to disable a specified interrupt.
 *  @return     None.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber
 */
void    NX_DWC_GMAC_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
    register NX_DWC_GMAC_RegisterSet* pRegister;
    register U32    regvalue;

    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    pRegister = __g_pRegister[ModuleIndex];
    regvalue  = pRegister->INTCTRL;

    regvalue &= ~( 1UL << IntNum );
    regvalue |= (U32)Enable << IntNum;

    WriteIO32(&pRegister->INTCTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is enabled or disabled.
 *  @param[in]  ModuleIndex     an index of module.
 *  @param[in]  IntNum  a interrupt Number.\n
 *                      refer to NX_DWC_GMAC_INTCH_xxx in <nx_DWC_GMAC.h>
 *  @return     \b CTRUE    indicates that a specified interrupt is enabled. \r\n
 *              \b CFALSE   indicates that a specified interrupt is disabled.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
CBOOL   NX_DWC_GMAC_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    return (CBOOL)( ( __g_pRegister[ModuleIndex]->INTCTRL >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether a specified interrupt is pended or not
 *  @param[in]  ModuleIndex     an index of module.
 *  @param[in]  IntNum  a interrupt Number.\n
 *                      refer to NX_DWC_GMAC_INTCH_xxx in <nx_DWC_GMAC.h>
 *  @return     \b CTRUE    indicates that a specified interrupt is pended. \r\n
 *              \b CFALSE   indicates that a specified interrupt is not pended.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
CBOOL   NX_DWC_GMAC_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
    register NX_DWC_GMAC_RegisterSet* pRegister;
    register U32    regvalue;
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    pRegister = __g_pRegister[ModuleIndex];
    regvalue  = pRegister->INTCTRL;
    regvalue &= pRegister->INTPEND;
    return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear a pending state of specified interrupt.
 *  @param[in]  ModuleIndex     an index of module.
 *  @param[in]  IntNum  a interrupt number.\n
 *                      refer to NX_DWC_GMAC_INTCH_xxx in <nx_DWC_GMAC.h>
 *  @return     None.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
void    NX_DWC_GMAC_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
    register NX_DWC_GMAC_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    pRegister = __g_pRegister[ModuleIndex];
    WriteIO32(&pRegister->INTPEND, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Set all interrupts to be enabled or disabled.
 *  @param[in]  ModuleIndex     an index of module.
 *  @param[in]  Enable  \b Set as CTRUE to enable all interrupts. \r\n
 *                      \b Set as CFALSE to disable all interrupts.
 *  @return     None.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
void    NX_DWC_GMAC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
    register NX_DWC_GMAC_RegisterSet* pRegister;
    register U32    regvalue;

    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (0==Enable) || (1==Enable) );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    pRegister = __g_pRegister[ModuleIndex];
    regvalue  = Enable ? 0xFFFFFFFF : 0 ;

    WriteIO32(&pRegister->INTCTRL, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are enabled or not.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     \b CTRUE    indicates that one or more interrupts are enabled. \r\n
 *              \b CFALSE   indicates that all interrupts are disabled.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
CBOOL   NX_DWC_GMAC_GetInterruptEnableAll( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->INTCTRL ));
}

//------------------------------------------------------------------------------
/**
 *  @brief      Indicates whether some of interrupts are pended or not.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     \b CTRUE    indicates that one or more interrupts are pended. \r\n
 *              \b CFALSE   indicates that no interrupt is pended.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
CBOOL   NX_DWC_GMAC_GetInterruptPendingAll( U32 ModuleIndex )
{
    register NX_DWC_GMAC_RegisterSet* pRegister;
    register U32    regvalue;
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    pRegister = __g_pRegister[ModuleIndex];
    regvalue  = pRegister->INTCTRL;
    regvalue &= pRegister->INTPEND;
    return (CBOOL)( 0 != ( regvalue ) );
}

//------------------------------------------------------------------------------
/**
 *  @brief      Clear pending state of all interrupts.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     None.
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_GetInterruptPendingNumber

 */
void    NX_DWC_GMAC_ClearInterruptPendingAll( U32 ModuleIndex )
{
    register NX_DWC_GMAC_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    pRegister = __g_pRegister[ModuleIndex];
    WriteIO32(&pRegister->INTPEND, 0xFFFFFFFF); // just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *  @brief      Get a interrupt number which has the most prority of pended interrupts.
 *  @param[in]  ModuleIndex     an index of module.
 *  @return     a interrupt number. A value of '-1' means that no interrupt is pended.\n
 *              refer to NX_DWC_GMAC_INTCH_xxx in <nx_DWC_GMAC.h>
 *  @see        NX_DWC_GMAC_GetInterruptNumber,
 *              NX_DWC_GMAC_SetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptEnable,
 *              NX_DWC_GMAC_GetInterruptPending,
 *              NX_DWC_GMAC_ClearInterruptPending,
 *              NX_DWC_GMAC_SetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptEnableAll,
 *              NX_DWC_GMAC_GetInterruptPendingAll,
 *              NX_DWC_GMAC_ClearInterruptPendingAll

 */
S32     NX_DWC_GMAC_GetInterruptPendingNumber( U32 ModuleIndex )    // -1 if None
{
    int i;
    register NX_DWC_GMAC_RegisterSet* pRegister;
    register U32    regvalue;
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET) | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );
    pRegister = __g_pRegister[ModuleIndex];
    regvalue  = pRegister->INTCTRL;
    regvalue &= pRegister->INTPEND;
    if( 0!=regvalue )
    {
        for( i=0; i<32; i++ )
        {
            if( regvalue & 1UL )
            {
                return i;
            }
            regvalue>>=1;
        }
    }
    return -1;
}
#endif

CBOOL   NX_DWC_GMAC_DMA_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
    U32 PEND_MASK;

    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET)
             | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    PEND_MASK = (1UL<<IntNum);

    return (CBOOL)((__g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS] & PEND_MASK)>>IntNum);
}

void    NX_DWC_GMAC_DMA_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
    U32 PEND_MASK;
    U32 RegVal;

    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (CNULL != __g_pRegister[GMAC_CSR][ModuleIndex]->REGSET)
             | (CNULL != __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET) );

    PEND_MASK = (1UL<<IntNum);

    __g_pRegister[GMAC_DMA][ModuleIndex]->REGSET[REG_DMA_STATUS] = PEND_MASK;
}

#if 0
//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *  @brief      Get DMA peripheral index
 *  @param[in]  ModuleIndex an index of module.
 *  @param[in]  ChannelIndex refer to NX_DWC_GMAC_DMACH_xxx in <nx_DWC_GMAC.h>
 *  @return     DMA peripheral index.
 *  @see        NX_DMA_TransferMemToIO(DestinationPeriID),
 *              NX_DMA_TransferIOToMem(SourcePeriID)
 */
U32 NX_DWC_GMAC_GetDMANumber ( U32 ModuleIndex , U32 ChannelIndex )
{
    const U32 DMANumber[][NUMBER_OF_DWC_GMAC_MODULE] =
    {
        { DMAINDEX_WITH_CHANNEL_LIST( DWC_GMAC, TXDMA ) }, // DMAINDEX_OF_DWC_GMAC?_MODULE_TEST0
        { DMAINDEX_WITH_CHANNEL_LIST( DWC_GMAC, RXDMA ) }, // DMAINDEX_OF_DWC_GMAC?_MODULE_TEST1
    };
    NX_CASSERT( NUMBER_OF_DWC_GMAC_MODULE == (sizeof(DMANumber[0])/sizeof(DMANumber[0][0])) );
    NX_ASSERT( NUMBER_OF_DWC_GMAC_MODULE > ModuleIndex );
    NX_ASSERT( (sizeof(DMANumber)/sizeof(DMANumber[0])) > ChannelIndex );
    // NX_ASSERT( DMAINDEX_OF_DWC_GMAC0_MODULE_TEST0 == DMANumber[0][0] );
    // NX_ASSERT( DMAINDEX_OF_DWC_GMAC1_MODULE_TEST0 == DMANumber[1][0] );
    // ...
    return DMANumber[ChannelIndex][ModuleIndex];
}

#endif
