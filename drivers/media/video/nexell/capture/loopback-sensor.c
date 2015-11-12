#include <linux/kernel.h>
#include <linux/module.h>

#include <mach/platform.h>
#include <mach/soc.h>

#include "nxp-v4l2.h"
#include "loopback-sensor.h"




#if 1
#define DUMP_REGISTER 1
void dump_register_dpc(int module)
{
#if (DUMP_REGISTER)
#define DBGOUT(args...)  printk(args)
    struct NX_DPC_RegisterSet *pREG =
        (struct NX_DPC_RegisterSet*)NX_DPC_GetBaseAddress(module);
		NX_DPC_SetBaseAddress(module, (void *)IO_ADDRESS(NX_DPC_GetPhysicalAddress(module)));
	
    DBGOUT("DPC%d BASE ADDRESS: %p\n", module, pREG);
    DBGOUT(" DPCCTRL0     = 0x%04x\r\n", pREG->DPCCTRL0);
#endif
}
#endif

static CBOOL	SetDisplayMode( NX_DISPLAY_MODE *pDisMode )
{
	U32 g_DPCIndex = 1;
	NX_DPC_PADCLK clock = NX_DPC_PADCLK_VCLK;

	NX_DISPLAY_TFTLCD	*pTFTLCD        = pDisMode->pTFTLCD;

	U32 				dwVCLKDivider=0, dwSyncDelay=0;
	NX_DPC_DITHER 	    RDither, GDither, BDither;
	CBOOL				bEmbeddedSync, bRGBMode;

	RDither = GDither = BDither = NX_DPC_DITHER_5BIT;
	bEmbeddedSync = bRGBMode = CFALSE;

/*
	if( g_DPCIndex )
	{
		//NX_DPC_SetHorizontalUpScaler( g_DPCIndex, CTRUE, 320, 720 );		// DPC1's upscale test
		NX_DPC_SetHorizontalUpScaler( g_DPCIndex, CFALSE, 720, 720 );
	}
	else
		NX_DPC_SetHorizontalUpScaler( g_DPCIndex, CFALSE, 2, 2 );
*/

	//--------------------------------------------------------------------------
	NX_DPC_SetDPCEnable( g_DPCIndex, CFALSE );
	NX_DPC_SetClockDivisorEnable(g_DPCIndex, CFALSE);

	//--------------------------------------------------------------------------
	// RGB or YUV?
	NX_ASSERT( CNULL != pTFTLCD );

	if( (U32)NX_DPC_FORMAT_MRGB888B  >= pTFTLCD->dwOutputMode || (U32)NX_DPC_FORMAT_SRGB888  == pTFTLCD->dwOutputMode)
	{
		bRGBMode = CTRUE;
	}
	else
	{
		bRGBMode = CFALSE;
	}

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

	// Dithering
    if( ((U32)NX_DPC_FORMAT_RGB555   == pTFTLCD->dwOutputMode) ||
		((U32)NX_DPC_FORMAT_MRGB555A == pTFTLCD->dwOutputMode) ||
		((U32)NX_DPC_FORMAT_MRGB555B == pTFTLCD->dwOutputMode) )
	{
		RDither = GDither = BDither = NX_DPC_DITHER_5BIT;
	}
	else if( ((U32)NX_DPC_FORMAT_RGB565  == pTFTLCD->dwOutputMode) ||
			 ((U32)NX_DPC_FORMAT_MRGB565 == pTFTLCD->dwOutputMode) )
	{
		RDither = BDither = NX_DPC_DITHER_5BIT;
		GDither           = NX_DPC_DITHER_6BIT;
	}
	else if( ((U32)NX_DPC_FORMAT_RGB666  == pTFTLCD->dwOutputMode) ||
			 ((U32)NX_DPC_FORMAT_MRGB666 == pTFTLCD->dwOutputMode) )
	{
		RDither = GDither = BDither = NX_DPC_DITHER_6BIT;
	}
	else
	{
		RDither = GDither = BDither = NX_DPC_DITHER_BYPASS;
	}

	// Embedded Sync?
	if( ((U32)NX_DPC_FORMAT_CCIR656 == pTFTLCD->dwOutputMode ) )
			bEmbeddedSync = CTRUE;
	else	bEmbeddedSync = CFALSE;

/*
	if( bEmbeddedSync )
		printk(KERN_INFO "[%s]DPC Format 656!!\n", __func__);
*/

	// Sync Delay?
	if( bRGBMode )
	{
/*			2009/5/28 NXC-1000 databook 26.4.6 Delay
		if( 0 == g_DPCIndex )   dwSyncDelay = 7 * dwVCLKDivider;    // Primary DPC
		else                    dwSyncDelay = 4 * dwVCLKDivider;    // Secondary DPC
*/
		dwSyncDelay = 7 * dwVCLKDivider;
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
/*
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
*/
	//--------------------------------------------------------------------------
	NX_DPC_SetClockDivisorEnable(g_DPCIndex, CTRUE);	// CLKENB : Provides internal operating clock.
	NX_DISPLAYTOP_SetPrimaryMUX(2 * g_DPCIndex); 
	NX_DISPLAYTOP_SetPADClock(PADMUX_SecondaryMLC, PADCLK_InvCLK ); 
//	NX_DPC_SetDPCEnable( g_DPCIndex, CTRUE );
	NX_DPC_SetEnable( g_DPCIndex, CTRUE, CFALSE, CFALSE, CFALSE, CTRUE );

	return CTRUE;
}

static void _dpc_mode1(void)
{
	NX_DISPLAY_MODE *pDisMode;
	/*U32		modenum;*/

	/*U32 g_DisplayMode = 0;*/

	//------------------------------------------------------------------------------
	// NXP3200 Internal Video Encoder - NTSC
	//------------------------------------------------------------------------------
	// HTotal : 858 = 720 + 16 + 32 + 90
	// VTotal : 525 = (240 + 4 + 3 + 15) + (240 + 5 + 3 + 15)
	// Desired CLK = 27,000,000 Hz
	// XTI / 1 = 27,000,000 / 1 = 27,000,000 Hz
	// Actual : 27,000,000 / (858 x 525 x 2) = 29.97 Hz
	//------------------------------------------------------------------------------
	NX_DISPLAY_TFTLCD  MODE_InternalVideoEncoder_TFTLCD =
	{
		// Output Format
		(U32)NX_DPC_FORMAT_CCIR656,	    // dwOutputMode
		//3,	    // dwOutputMode
		/*CTRUE,		// bInterlace			: Interlace Scan mode*/
		CFALSE,		// bInterlace			: Interlace Scan mode
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

		480,		// dwVerActive
		4,			// dwVerFrontPorch
		3,			// dwVerSyncWidth
		15,			// dwVerBackPorch
		CFALSE,		// bVerSyncHighActive	: Not used.

		/*240,		// dwEvenVerActive*/
		/*4,			// dwEvenVerFrontPorch*/
		/*3,			// dwEvenVerSyncWidth*/
		/*15,			// dwEvenVerBackPorch*/

		0,			// dwVerSyncStartOffset
		0,			// dwVerSyncEndOffset
		0,			// dwEvenVerSyncStartOffset
		0			// dwEvenVerSyncEndOffset
	};

	/*NX_ENCODER_MODE	MODE_InternalVideoEncoder_EncoderMode =*/
	/*{*/
		/*(U32)NX_DPC_VBS_NTSC_M ,		// dwOutputMode*/
		/*CTRUE,                          // bPedestal*/
		/*//(U32)NX_DPC_BANDWIDTH_LOW ,	// dwYBandWidth*/
		/*//(U32)NX_DPC_BANDWIDTH_LOW ,	// dwCBandWidth*/
		/*(U32)NX_DPC_BANDWIDTH_HIGH ,	// dwYBandWidth*/
		/*(U32)NX_DPC_BANDWIDTH_HIGH ,	// dwCBandWidth*/

		/*63,			// dwHorSyncStart*/
		/*1715,		// dwHorSyncEnd*/
		/*0,			// dwVerSyncStart*/
		/*3			// dwVerSyncEnd*/
	/*};*/


	NX_DISPLAY_MODE 	MODE_InternalVideoEncoder =
	{
		&MODE_InternalVideoEncoder_TFTLCD,         // pTFTLCD
		/*&MODE_InternalVideoEncoder_EncoderMode		// pEncoderMode*/
        NULL
	};

	pDisMode = &MODE_InternalVideoEncoder;

	//--------------------------------------------------------------------------
	// Share variables
	// SetDelay
	/*U32 	DelayRGB_PVD, DelayHS_CP1, DelayVS_FRAM, DelayDE_CP2;*/
	// SetDither
	/*NX_DPC_DITHER DitherR, DitherG, DitherB;*/

	SetDisplayMode( pDisMode );
}

static void _release_reset(void)
{
    NX_RSTCON_SetBaseAddress((void *)IO_ADDRESS(NX_RSTCON_GetPhysicalAddress()));
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_nRST       , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_VIDEO_nRST , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_SPDIF_nRST , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_TMDS_nRST  , 1);
    NX_RSTCON_SetRST(RESETINDEX_OF_DISPLAYTOP_MODULE_i_HDMI_PHY_nRST   , 1);
}

static void _release_clk(int module)
{
	NX_DISPTOP_CLKGEN_SetBaseAddress(HDMI_CLKGEN, (void *)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(HDMI_CLKGEN)));
	NX_DISPTOP_CLKGEN_SetClockDivisorEnable(HDMI_CLKGEN, CFALSE );	
  NX_DPC_SetClockDivisorEnable(module, CFALSE);
}

static void _set_hdmi_clk_27MHz(void)
{
    NX_HDMI_SetBaseAddress(0, (void *)IO_ADDRESS(NX_HDMI_GetPhysicalAddress(0)));

    NX_TIEOFF_Initialize();
    NX_TIEOFF_SetBaseAddress((void *)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress()));
    NX_TIEOFF_Set(TIEOFFINDEX_OF_DISPLAYTOP0_i_HDMI_PHY_REFCLK_SEL, 1);

    // HDMI PCLK Enable
    NX_DISPTOP_CLKGEN_SetBaseAddress(HDMI_CLKGEN, (void *)IO_ADDRESS(NX_DISPTOP_CLKGEN_GetPhysicalAddress(HDMI_CLKGEN)));
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

static void _dpc_clk_enable(int module)
{
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

static void _mlc_set(int module, int width, int height)
{
    NX_MLC_SetMLCTopControlParameter(module,
            /*CTRUE, // Field Enable(interlace or progressive)*/
            CFALSE, // Field Enable(interlace or progressive)
            CTRUE, // MLC Operation Enable
            1,  // Layer preference decide
            SECON );

    NX_MLC_SetScreenSize(module, width, height);
    NX_MLC_SetBackground(module, 0xFFFFFF);
    
    NX_MLC_SetSRAMMODE(module, TOPMLC, RUN);

#if 0
    NX_MLC_SetRGB0LayerControlParameter(module,
            CTRUE, CFALSE,
            CFALSE, 0x0,
            CFALSE, 0x0,
            CFALSE, 0x0,
            RGBFMT_A8B8G8R8, LOCKSIZE_16);
#else
    // only enable background
    NX_MLC_SetRGB0LayerControlParameter(module,
            CFALSE, CFALSE,
            CFALSE, 0x0,
            CFALSE, 0x0,
            CFALSE, 0x0,
            RGBFMT_A8B8G8R8, LOCKSIZE_16);
#endif

    NX_MLC_SetLayerRegFinish(module, TOPMLC);
    NX_MLC_SetLayerRegFinish(module, RGB0);
}

static void display_out_bt656(int module, int width, int height, int on)
{
    /* printk("[%s] module : %d, width : %d, hegiht : %d, Enable : %d\n", __func__, module, width, height, on); */

    if (on) {
        _release_reset();
        _set_hdmi_clk_27MHz();
			 	//dump_register_dpc(module);
        _dpc_clk_enable(module);
        _dpc_mode1();
        _mlc_set(module, width, height);
				//dump_register_dpc(module);
    } else {
			_release_clk(module);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// v4l2 subdev
static struct nxp_loopback_sensor _context;

static int loopback_sensor_s_power(struct v4l2_subdev *sd, int on)
{
    return 0;
}

static int loopback_sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct nxp_loopback_sensor *me = v4l2_get_subdevdata(sd);

    display_out_bt656(me->dpc_module, me->width, me->height, enable);

    return 0;
}

static int loopback_sensor_s_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
       struct v4l2_subdev_format *fmt)
{
    struct nxp_loopback_sensor *me = v4l2_get_subdevdata(sd);
    me->width  = fmt->format.width;
    me->height = fmt->format.height;
    return 0;
}

static const struct v4l2_subdev_core_ops loopback_sensor_subdev_core_ops = {
    .s_power = loopback_sensor_s_power,
};

static const struct v4l2_subdev_video_ops loopback_sensor_subdev_video_ops = {
     .s_stream = loopback_sensor_s_stream,
};

static const struct v4l2_subdev_pad_ops loopback_sensor_subdev_pad_ops = {
    .set_fmt = loopback_sensor_s_fmt,
};

static const struct v4l2_subdev_ops loopback_sensor_ops = {
    .core  = &loopback_sensor_subdev_core_ops,
    .video = &loopback_sensor_subdev_video_ops,
    .pad   = &loopback_sensor_subdev_pad_ops,
};

static int _init_entities(struct nxp_loopback_sensor *me)
{
	struct v4l2_subdev *sd = &me->sd;
  int ret;

	v4l2_subdev_init(sd, &loopback_sensor_ops);
	strlcpy(sd->name, "loopback-sensor", sizeof(sd->name));
	v4l2_set_subdevdata(sd, me);

	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	me->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
	ret = media_entity_init(&sd->entity, 1, &me->pad, 0);
	if (ret < 0) {
			 pr_err("%s: failed to media_entity_init()\n", __func__);
			 return ret;
	}

	return 0;
}

extern void nxp_v4l2_capture_set_sensor_subdev(struct v4l2_subdev *sd);

struct nxp_loopback_sensor *create_nxp_loopback_sensor(struct nxp_capture_platformdata *pdata)
{
  struct nxp_loopback_sensor *me = &_context;
  int ret;

	ret = _init_entities(me);

	return me;
}

void release_nxp_loopback_sensor(struct nxp_loopback_sensor *me)
{

	// TODO:	
}

int register_nxp_loopback_sensor(struct nxp_loopback_sensor *me)
{
	int ret = 0;

	if( nxp_v4l2_get_v4l2_device() == NULL )
		pr_err("%s: nxp_v4l2_get_v4l2_device() is null!!\n" , __func__);

	if( &me->sd == NULL ) 
		pr_err("%s: Subdev is null!!\n" , __func__);

	ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(), &me->sd);
	if (ret < 0) {
			pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
			return ret;
	}

	pr_err("%s: return : %d\n", __func__, ret);

	nxp_v4l2_capture_set_sensor_subdev(&me->sd);

	me->dpc_module = 1;

	return ret;
}

void unregister_nxp_loopback_sensor(struct nxp_loopback_sensor *me)
{
	v4l2_device_unregister_subdev(&me->sd);
}


#if 0
static int loopback_sensor_probe(struct platform_device *pdev)
{
    struct v4l2_subdev *sd;
    struct nxp_loopback_sensor *me = &_context;
    int ret;

    sd = &me->sd;
    strcpy(sd->name, "loopback-sensor");
    v4l2_subdev_init(sd, &loopback_sensor_ops);
    v4l2_set_subdevdata(sd, me);

    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    me->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
    ret = media_entity_init(&sd->entity, 1, &me->pad, 0);
    if (ret < 0) {
         pr_err("%s: failed to media_entity_init()\n", __func__);
         return ret;
    }

    pr_err("%s: v4l2 device check!!\n", __func__);

		if( nxp_v4l2_get_v4l2_device() == NULL )
		{
        pr_err("%s:  error : nxp_v4l2_get_v4l2_device function is null!!\n", __func__);
		}

    ret = v4l2_device_register_subdev(nxp_v4l2_get_v4l2_device(), &me->sd);
    if (ret < 0) {
        pr_err("%s: failed to v4l2_device_register_subdev()\n", __func__);
        return ret;
    }

    nxp_v4l2_capture_set_sensor_subdev(&me->sd);

    me->dpc_module = 1;

    return 0;
}

static int loopback_sensor_remove(struct platform_device *pdev)
{
    struct nxp_loopback_sensor *me = &_context;
    v4l2_device_unregister_subdev(&me->sd);
    return 0;
}

#endif


#if 0
static struct platform_driver loopback_sensor_platform_driver = {
    .probe = loopback_sensor_probe,
    .remove = loopback_sensor_remove,
    .driver = {
        .name = "loopback-sensor",
        .owner = THIS_MODULE,
    },
};

static int __init loopback_sensor_init(void)
{
    return platform_driver_register(&loopback_sensor_platform_driver);
}

static void __exit loopback_sensor_exit(void)
{
     platform_driver_unregister(&loopback_sensor_platform_driver);
}

module_init(loopback_sensor_init);
module_exit(loopback_sensor_exit);

MODULE_DESCRIPTION("NEXELL LoopBack Sensor Driver");
MODULE_AUTHOR("<swpark@nexell.co.kr>");
MODULE_LICENSE("GPL");
#endif

