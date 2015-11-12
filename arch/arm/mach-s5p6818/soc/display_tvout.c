#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <mach/platform.h>
#include <linux/platform_device.h>

#include <mach/devices.h>
#include <mach/soc.h>

enum DPC_CLKSRC
{
    DPC_CLKSRC_PLL0     = 0,
    DPC_CLKSRC_PLL1     = 1,
    DPC_CLKSRC_PLL2     = 2,
    DPC_CLKSRC_i_VCLK   = 3,
    DPC_CLKSRC_HDMICLK  = 4,
    DPC_CLKSRC_i_VCLK27 = 5,
    DPC_CLKSRC_PLL3     = 6,
    DPC_CLKSRC_CLK0     = 7,
    DPC_CLKSRC_FORCE32  = 0x7fffffff
};

typedef struct tag_NX_DISPLAY_TFTLCD
{
    // Output Format
    U32		dwOutputMode;
    CBOOL	bInterlace;
    CBOOL	bInvertField;

    // Clock Settings
    U32		dwClockSource;
    U32		dwClockDivider;
    U32		dwClockDelay;
    CBOOL	bClockRisingEdge;
    CBOOL	bDualEdge;

    // Dual View
    CBOOL	bDualView;

    // Horizontal Sync Timing
    U32		dwHorActive;
    U32		dwHorFrontPorch;
    U32		dwHorSyncWidth;
    U32		dwHorBackPorch;
    CBOOL	bHorSyncHighActive;

    // Vertical Sync Timing
    U32		dwVerActive;
    U32		dwVerFrontPorch;
    U32		dwVerSyncWidth;
    U32		dwVerBackPorch;
    CBOOL	bVerSyncHighActive;

    U32		dwEvenVerActive;
    U32		dwEvenVerFrontPorch;
    U32		dwEvenVerSyncWidth;
    U32		dwEvenVerBackPorch;

    U32		dwVerSyncStartOffset;
    U32		dwVerSyncEndOffset;
    U32		dwEvenVerSyncStartOffset;
    U32		dwEvenVerSyncEndOffset;

} NX_DISPLAY_TFTLCD;

//------------------------------------------------------------------------------
typedef struct tag_NX_ENCODER_MODE
{
    U32		dwBroadcast;
    CBOOL   bPedestal;
    U32 	dwYBandWidth;
    U32		dwCBandWidth;

    U32		dwHorSyncStart;
    U32		dwHorSyncEnd;
    U32		dwVerSyncStart;
    U32		dwVerSyncEnd;

} NX_ENCODER_MODE;

//------------------------------------------------------------------------------
typedef struct tag_NX_DISPLAY_MODE
{
    NX_DISPLAY_TFTLCD	*pTFTLCD;
    NX_ENCODER_MODE		*pEncoderMode;
} NX_DISPLAY_MODE;

static void _release_reset(void)
{
    NX_RSTCON_SetBaseAddress((U32)IO_ADDRESS(NX_RSTCON_GetPhysicalAddress()));

    /*NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_Top_nRST        , 1);*/
    /*NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_DualDisplay_nRST, 1);*/
    /*NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_ResConv_nRST    , 1);*/
    /*NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_LCDIF_nRST      , 1);*/
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_nRST       , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_VIDEO_nRST , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_SPDIF_nRST , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_TMDS_nRST  , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_PHY_nRST   , 1);
}

static void _set_hdmi_clk_27MHz(void)
{
	NX_HDMI_SetBaseAddress(0, (U32)IO_ADDRESS(NX_HDMI_GetPhysicalAddress(0)));

    NX_TIEOFF_Initialize();
    NX_TIEOFF_SetBaseAddress((U32)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress()));
    NX_TIEOFF_Set(TIEOFFINDEX_OF_DISPLAYTOP0_i_HDMI_PHY_REFCLK_SEL, 1);

    // HDMI PCLK Enable
    NX_DISPTOP_CLKGEN_SetBaseAddress(HDMI_CLKGEN, (U32)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(HDMI_CLKGEN)));
    NX_DISPTOP_CLKGEN_SetClockPClkMode(HDMI_CLKGEN, NX_PCLKMODE_ALWAYS);

    // Enter Reset
    NX_RSTCON_SetRST  (NX_HDMI_GetResetNumber(0, i_nRST_PHY) , 0);
    NX_RSTCON_SetRST  (NX_HDMI_GetResetNumber(0, i_nRST)     , 0); // APB

    // Release Reset
    NX_RSTCON_SetRST  (NX_HDMI_GetResetNumber(0, i_nRST_PHY) , 1);
    NX_RSTCON_SetRST  (NX_HDMI_GetResetNumber(0, i_nRST)     , 1); // APB

    NX_DISPTOP_CLKGEN_SetClockPClkMode      (HDMI_CLKGEN, NX_PCLKMODE_ALWAYS);

    NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (0<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (0<<7) );    /// MODE_SET_DONE : APB Set
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, (0<<4) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, (0<<4) );    ///CLK_SEL : REF OSC or INT_CLK
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, (1<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, (1<<7) );    // INT REFCLK : ³»ºÎÀÇ syscon¿¡¼­ ¹Þ´Â clock
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, 0xD1   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg04, 0xD1   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg08, 0x22   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg08, 0x22   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg0C, 0x51   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg0C, 0x51   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg10, 0x40   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg10, 0x40   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg14, 0x8    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg14, 0x8    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg18, 0xFC   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg18, 0xFC   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg1C, 0xE0   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg1C, 0xE0   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg20, 0x98   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg20, 0x98   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, 0xE8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg24, 0xE8   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg28, 0xCB   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg28, 0xCB   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg2C, 0xD8   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg2C, 0xD8   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg30, 0x45   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg30, 0x45   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg34, 0xA0   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg34, 0xA0   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg38, 0xAC   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg38, 0xAC   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg3C, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg3C, 0x80   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0x6    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg40, 0x6    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg44, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg44, 0x80   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg48, 0x9    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg48, 0x9    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg4C, 0x84   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg4C, 0x84   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg50, 0x5    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg50, 0x5    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg54, 0x22   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg54, 0x22   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg58, 0x24   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg58, 0x24   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0x86   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg5C, 0x86   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg60, 0x54   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg60, 0x54   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg64, 0xE4   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg64, 0xE4   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg68, 0x24   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg68, 0x24   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg6C, 0x0    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg6C, 0x0    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg70, 0x0    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg70, 0x0    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg74, 0x0    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg74, 0x0    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg78, 0x1    ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg78, 0x1    );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, 0x80   ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, 0x80   );
    NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (1<<7) ); NX_HDMI_SetReg( 0, HDMI_PHY_Reg7C, (1<<7) );    /// MODE_SET_DONE : APB Set Done

    // wait phy ready
    {
        U32 Is_HDMI_PHY_READY = CFALSE;
        while(Is_HDMI_PHY_READY == CFALSE) {
            if(NX_HDMI_GetReg( 0, HDMI_LINK_PHY_STATUS_0 ) & 0x01) {
                Is_HDMI_PHY_READY = CTRUE;
            }
        }
    }
}

static void _dpc_clk_enable(void)
{
    int module = 1;
    NX_DPC_SetClockDivisorEnable(module, CFALSE);
    NX_DPC_SetClockOutEnb(module,  0, CFALSE );
    NX_DPC_SetClockOutEnb(module, 1, CFALSE );
    NX_MLC_SetClockBClkMode(module, NX_BCLKMODE_ALWAYS);
    NX_DPC_SetClockDivisor (module, 0, 1 );
    NX_DPC_SetClockSource  (module, 0, DPC_CLKSRC_HDMICLK ); //
    NX_DPC_SetClockOutInv  (module,0, 0);        // Out Pad VCLK2 Inverse
    NX_DPC_SetClockDivisor (module, 1, 1 );
    NX_DPC_SetClockSource  (module, 1, DPC_CLKSRC_CLK0 ); //
    NX_DPC_SetClockOutEnb  (module, 1, CTRUE);
    NX_DPC_SetClockDivisorEnable(module,CTRUE);
}

static void _dpc_set_encoder(void)
{
    int module = 1;

    NX_DPC_SetOutputFormat(module, OUTPUTFORMAT_CCIR601_16B, 1);

    NX_DPC_SetEncoderControlReg		(module, 0x40, 0, 0);
    NX_DPC_SetEncoderSHCPhaseControl(module, 0x3f);
    NX_DPC_SetEncoderTimingConfigReg(module, 0x7);
    NX_DPC_SetEncoderDACOutputSelect(module, 1, 2, 4, 5, 0, 0);
    NX_DPC_SetEncoderSyncLocation	(module, 1715, 63, 0, 3, 0, CFALSE);
    NX_DPC_SetEncoderDACPowerEnable	(module, 0x30);
    NX_DPC_SetLumaGain              (module, 30);
}

static void _dac_power_control(bool power_on)
{
    volatile U32 * reg = (U32*)(IO_ADDRESS(0xC00110F8));
    if (power_on)
        *reg = *reg | ((1<<4)); // DAC.PD set LOW !! (LOW : Normal Operation, HIGH : Power Down)
    else
        *reg = *reg & (~(1<<4)); // DAC.PD set HIGH !! (LOW : Normal Operation, HIGH : Power Down)
}

static CBOOL _dpc_set_display_mode( NX_DISPLAY_MODE *pDisMode )
{
    NX_DISPLAY_TFTLCD	*pTFTLCD        = pDisMode->pTFTLCD;
    NX_ENCODER_MODE		*pEncoderMode	= pDisMode->pEncoderMode;

    U32 				dwVCLKDivider=0, dwSyncDelay=0;
    NX_DPC_DITHER 	    RDither, GDither, BDither;
    CBOOL				bEmbeddedSync, bRGBMode;
    int g_DPCIndex = 1;
    NX_DPC_PADCLK clock = NX_DPC_PADCLK_VCLK;

    RDither = GDither = BDither = NX_DPC_DITHER_5BIT;
    bEmbeddedSync = bRGBMode = CFALSE;

    NX_DPC_SetHorizontalUpScaler( g_DPCIndex, CFALSE, 720, 720 );

    NX_DPC_SetDPCEnable( g_DPCIndex, CFALSE );
    NX_DPC_SetClockDivisorEnable(g_DPCIndex, CFALSE);

    // VCLK = VCLK2 / ?
    if( ((U32)NX_DPC_FORMAT_RGB888   >= pTFTLCD->dwOutputMode)  ||
            ((U32)NX_DPC_FORMAT_CCIR601A   == pTFTLCD->dwOutputMode)   )
    {
        dwVCLKDivider = 1;
    }
    else if((U32)NX_DPC_FORMAT_SRGB888 == pTFTLCD->dwOutputMode )
    {
        dwVCLKDivider = 6;
    }
    else if((U32)NX_DPC_FORMAT_SRGBD8888 == pTFTLCD->dwOutputMode )
    {
        dwVCLKDivider = 4;
    }
    else
    {
        dwVCLKDivider = 2;
    }

    RDither = GDither = BDither = NX_DPC_DITHER_BYPASS;

    if( ((U32)NX_DPC_FORMAT_CCIR656 == pTFTLCD->dwOutputMode ) )
        bEmbeddedSync = CTRUE;
    else	bEmbeddedSync = CFALSE;

    // Sync Delay?
    if( bRGBMode )
    {
        /*			2009/5/28 NXC-1000 databook 26.4.6 Delay
                    if( 0 == g_DPCIndex )   dwSyncDelay = 7 * dwVCLKDivider;    // Primary DPC
                    else                    dwSyncDelay = 4 * dwVCLKDivider;    // Secondary DPC
                    */
        dwSyncDelay = 7 * dwVCLKDivider;

        //NX_CONSOLE_Printf( "SyncDelay:%d, VCLKDivider:%d \n", dwSyncDelay, dwVCLKDivider );

    }
    else
    {
        dwSyncDelay = 6 * dwVCLKDivider;
    }

    //--------------------------------------------------------------------------
    // VCLK2 : CLKGEN0
    NX_DPC_SetClockSource  (g_DPCIndex, 0, pTFTLCD->dwClockSource);		// CLKSRCSEL
    NX_DPC_SetClockDivisor (g_DPCIndex, 0, pTFTLCD->dwClockDivider);		// CLKDIV
    NX_DPC_SetClockOutDelay(g_DPCIndex, 0, pTFTLCD->dwClockDelay); 		// OUTCLKDELAY

    // VCLK : CLKGEN1
    NX_DPC_SetClockSource  (g_DPCIndex, 1, 7);								// CLKSRCSEL  : CLKGEN0's out
    NX_DPC_SetClockDivisor (g_DPCIndex, 1, dwVCLKDivider);					// CLKDIV
    NX_DPC_SetClockOutDelay(g_DPCIndex, 1, pTFTLCD->dwClockDelay); 		// OUTCLKDELAY

    //--------------------------------------------------------------------------
    clock = (pTFTLCD->bDualEdge) ? NX_DPC_PADCLK_VCLK : NX_DPC_PADCLK_VCLK2;

    if(pTFTLCD->dwOutputMode == NX_DPC_FORMAT_SRGBD8888)
        clock = NX_DPC_PADCLK_VCLK;
    else if(pTFTLCD->dwOutputMode == NX_DPC_FORMAT_SRGB888)
        clock = NX_DPC_PADCLK_VCLK3;


    NX_DPC_SetMode(g_DPCIndex,
            (NX_DPC_FORMAT)pTFTLCD->dwOutputMode,		// FORMAT
            pTFTLCD->bInterlace,     					// SCANMODE
            pTFTLCD->bInvertField,   					// POLFIELD
            bRGBMode, 									// RGBMODE
            CFALSE,       								// SWAPRB
            NX_DPC_YCORDER_CbYCrY ,					// YCORDER
            (bEmbeddedSync) ? CTRUE : CFALSE,			// YCCLIP
            bEmbeddedSync,  							// Embedded sync
            clock,		// PADCLKSEL
            pTFTLCD->bClockRisingEdge,					// PADCLKINV
            pTFTLCD->bDualView
            );
    NX_DPC_SetHSync( g_DPCIndex,
            pTFTLCD->dwHorActive,
            pTFTLCD->dwHorSyncWidth,
            pTFTLCD->dwHorFrontPorch,
            pTFTLCD->dwHorBackPorch,
            pTFTLCD->bHorSyncHighActive );

    NX_DPC_SetVSync( g_DPCIndex,
            pTFTLCD->dwVerActive,
            pTFTLCD->dwVerSyncWidth,
            pTFTLCD->dwVerFrontPorch,
            pTFTLCD->dwVerBackPorch,
            pTFTLCD->bVerSyncHighActive,
            pTFTLCD->dwEvenVerActive,
            pTFTLCD->dwEvenVerSyncWidth,
            pTFTLCD->dwEvenVerFrontPorch,
            pTFTLCD->dwEvenVerBackPorch );

    NX_DPC_SetVSyncOffset( g_DPCIndex,
            pTFTLCD->dwVerSyncStartOffset,
            pTFTLCD->dwVerSyncEndOffset,
            pTFTLCD->dwEvenVerSyncStartOffset,
            pTFTLCD->dwEvenVerSyncEndOffset );

    NX_DPC_SetDelay(g_DPCIndex,
            0,				// DELAYRGB
            dwSyncDelay,		// DELAYHS_CP1
            dwSyncDelay,		// DELAYVS_FRAM
            dwSyncDelay );		// DELAYDE_CP2

    if( bRGBMode )
    {
        NX_DPC_SetDither( g_DPCIndex, RDither, GDither, BDither );
    }
    else
    {
        NX_DPC_SetDither( g_DPCIndex, NX_DPC_DITHER_BYPASS, NX_DPC_DITHER_BYPASS, NX_DPC_DITHER_BYPASS );
    }

    //--------------------------------------------------------------------------
    if( 1 == g_DPCIndex )
    {
        if( CNULL != pEncoderMode )
        {
            NX_DPC_SetENCEnable( g_DPCIndex, CTRUE );

            //NX_TIMER_TickCountDelay( 100 );
            //NX_DPC_SetClockDivisorEnable( g_DPCIndex, CTRUE );
            //NX_TIMER_TickCountDelay( 100 );
            //NX_DPC_SetENCEnable( g_DPCIndex, CFALSE );
            //NX_TIMER_TickCountDelay( 100 );
            //NX_DPC_SetClockDivisorEnable( g_DPCIndex, CFALSE );
            //NX_TIMER_TickCountDelay( 100 );
            NX_DPC_SetClockDivisorEnable(g_DPCIndex, CTRUE);	// CLKENB : Provides internal operating clock.

            NX_DPC_SetENCEnable( g_DPCIndex, CTRUE );

            //NX_DPC_SetVideoEncoderPowerDown(  g_DPCIndex, CTRUE );
            NX_DPC_SetVideoEncoderMode( g_DPCIndex, (NX_DPC_VBS)pEncoderMode->dwBroadcast, pEncoderMode->bPedestal ) ;
            NX_DPC_SetVideoEncoderFSCAdjust( g_DPCIndex, 0 );
            NX_DPC_SetVideoEncoderBandwidth( g_DPCIndex, (NX_DPC_BANDWIDTH)pEncoderMode->dwYBandWidth,
                    (NX_DPC_BANDWIDTH)pEncoderMode->dwCBandWidth ) ;
            NX_DPC_SetVideoEncoderColorControl( g_DPCIndex, 0, 0, 0, 0, 0 );
            NX_DPC_SetVideoEncoderTiming( g_DPCIndex,
                    pEncoderMode->dwHorSyncStart,
                    pEncoderMode->dwHorSyncEnd,
                    pEncoderMode->dwVerSyncStart,
                    pEncoderMode->dwVerSyncEnd );
            NX_DPC_SetVideoEncoderPowerDown( g_DPCIndex, CFALSE );
            NX_DPC_SetENCEnable( g_DPCIndex, CTRUE );
        }
        else
        {
            NX_DPC_SetENCEnable( g_DPCIndex, CFALSE );
        }
    }

    //--------------------------------------------------------------------------
    NX_DPC_SetClockDivisorEnable(g_DPCIndex, CTRUE);	// CLKENB : Provides internal operating clock.

    return CTRUE;
}

static void _mlc_set(void)
{
    int module = 1;

    NX_MLC_SetMLCTopControlParameter(module,
            CTRUE, // Field Enable(interlace or progressive)
            CTRUE, // MLC Operation Enable
            1,  // Layer preference decide
            SECON );

    NX_MLC_SetScreenSize(module, 720, 480);
    NX_MLC_SetBackground(module, 0xFFFFFF);
    NX_MLC_SetSRAMMODE(module, TOPMLC, SLEEPMODE);
    NX_MLC_SetSRAMMODE(module, TOPMLC, RUN);

    NX_MLC_SetRGB0LayerControlParameter(module,
            CTRUE, CFALSE,
            CFALSE, 0x0,
            CFALSE, 0x0,
            CFALSE, 0x0,
            RGBFMT_A8B8G8R8, LOCKSIZE_16);

    NX_MLC_SetLayerRegFinish(module, TOPMLC);
    NX_MLC_SetLayerRegFinish(module, RGB0);
}

static int tvout_enable(struct disp_process_dev *pdev, int enable)
{
    printk("%s entered, enable %d\n", __func__, enable);

    if (!enable)
        return 0;

    _release_reset();
    _set_hdmi_clk_27MHz();
    _dpc_clk_enable();
    _dpc_set_encoder();
    _dac_power_control(false);
    _dac_power_control(true);

    {
        NX_DISPLAY_MODE *pDisMode;
        NX_DISPLAY_TFTLCD  MODE_InternalVideoEncoder_NTSC_TFTLCD =
        {
            // Output Format
            (U32)NX_DPC_FORMAT_CCIR601B,	    // dwOutputMode
            CTRUE,		// bInterlace			: Interlace Scan mode
            CFALSE,		// bInvertField			: Bypass

            DPC_CLKSRC_HDMICLK,			// dwClockSource		: HDMI PHY
            1,			// dwClockDivider		: 1
            0,			// dwClockDelay			: Not used.
            CFALSE,		// bClockRisingEdge		: Not used.
            CFALSE,		// bDualEdge			: Not used.

            CFALSE,		// bDualView			: Not Dual View LCD

            720,		// dwHorActive
            16,			// dwHorFrontPorch
            33,			// dwHorSyncWidth
            90,			// dwHorBackPorch
            CFALSE,		// bHorSyncHighActive	: Not used.

            240,		// dwVerActive
            4,			// dwVerFrontPorch
            3,			// dwVerSyncWidth
            15,			// dwVerBackPorch
            CFALSE,		// bVerSyncHighActive	: Not used.

            240,		// dwEvenVerActive
            4,			// dwEvenVerFrontPorch
            3,			// dwEvenVerSyncWidth
            15,			// dwEvenVerBackPorch

            0,			// dwVerSyncStartOffset
            0,			// dwVerSyncEndOffset
            0,			// dwEvenVerSyncStartOffset
            0			// dwEvenVerSyncEndOffset
        };

        NX_ENCODER_MODE	MODE_InternalVideoEncoder_NTSC_EncoderMode =
        {
            (U32)NX_DPC_VBS_NTSC_M ,		// dwOutputMode
            CTRUE,                          // bPedestal
            //(U32)NX_DPC_BANDWIDTH_LOW ,	// dwYBandWidth
            //(U32)NX_DPC_BANDWIDTH_LOW ,	// dwCBandWidth
            (U32)NX_DPC_BANDWIDTH_HIGH ,	// dwYBandWidth
            (U32)NX_DPC_BANDWIDTH_HIGH ,	// dwCBandWidth

            63,			// dwHorSyncStart
            1715,		// dwHorSyncEnd
            0,			// dwVerSyncStart
            3			// dwVerSyncEnd
        };


        NX_DISPLAY_MODE 	MODE_InternalVideoEncoder_NTSC =
        {
            &MODE_InternalVideoEncoder_NTSC_TFTLCD,         // pTFTLCD
            &MODE_InternalVideoEncoder_NTSC_EncoderMode		// pEncoderMode
        };

        pDisMode = &MODE_InternalVideoEncoder_NTSC;
        _dpc_set_display_mode(pDisMode);
    }

    NX_DPC_SetDPCEnable(1, CTRUE);

    _mlc_set();

    printk("%s exit\n", __func__);
    return 0;
}

static struct disp_process_ops tvout_ops = {
    .enable = tvout_enable,
};

static struct switch_dev tvout_switch;
static ssize_t _enable_tvout(struct device *pdev,
        struct device_attribute *attr, const char *buf, size_t n)
{
    if (!strncmp(buf, "1", 1)) {
        printk("%s: enable tvout\n", __func__);
        if (!switch_get_state(&tvout_switch)) {
            nxp_soc_disp_device_connect_to(DISP_DEVICE_TVOUT, DISP_DEVICE_SYNCGEN1, NULL);
            switch_set_state(&tvout_switch, 1);
        }
    } else {
        printk("%s: disable tvout\n", __func__);
        if (switch_get_state(&tvout_switch)) {
            switch_set_state(&tvout_switch, 0);
        }
    }
    return n;
}

static struct device_attribute tvout_attr = __ATTR(enable, 0666, NULL, _enable_tvout);
static struct attribute *attrs[] = {
    &tvout_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = (struct attribute **)attrs,
};

static int _create_sysfs(void)
{
    struct kobject *kobj = NULL;
    int ret = 0;

    kobj = kobject_create_and_add("tvout", &platform_bus.kobj);
    if (! kobj) {
        printk(KERN_ERR "Fail, create kobject for tvout\n");
        return -ret;
    }

    ret = sysfs_create_group(kobj, &attr_group);
    if (ret) {
        printk(KERN_ERR "Fail, create sysfs group for tvout\n");
        kobject_del(kobj);
        return -ret;
    }

    return 0;
}

>>>>>>> 83322c7... [FIX] tvout dynamic enable/disable
static int tvout_probe(struct platform_device *pdev)
{
    nxp_soc_disp_register_proc_ops(DISP_DEVICE_TVOUT, &tvout_ops);
    return 0;
}

static struct platform_driver tvout_driver = {
    .driver = {
        .name  = DEV_NAME_TVOUT,
        .owner = THIS_MODULE,
    },
    .probe = tvout_probe,
};
module_platform_driver(tvout_driver);

MODULE_AUTHOR("swpark <swparknexell.co.kr>");
MODULE_DESCRIPTION("Display TVOUT driver for the Nexell");
MODULE_LICENSE("GPL");
