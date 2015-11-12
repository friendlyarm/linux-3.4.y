#ifndef __LOOPBACK_SENSOR_H__
#define  __LOOPBACK_SENSOR_H__

#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

struct nxp_capture_platformdata;

struct nxp_loopback_sensor {
    struct media_pad pad;
    struct v4l2_subdev sd;
    int width;
    int height;

    int dpc_module;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// display part

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

struct nxp_loopback_sensor *create_nxp_loopback_sensor(struct nxp_capture_platformdata *pdata);
void release_nxp_loopback_sensor(struct nxp_loopback_sensor *me);
int register_nxp_loopback_sensor(struct nxp_loopback_sensor *me);
void unregister_nxp_loopback_sensor(struct nxp_loopback_sensor *me);

#endif
