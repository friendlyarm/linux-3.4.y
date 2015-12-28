//
//	Module : Nexell VPU Device Driver for CNM Coda960
//	Description : Device Driver (Firmware) Layer Interface
//	Author : SeongO-Park ( ray@nexell.co.kr )
//

#ifndef UNUSED
#define UNUSED(p) ((void)(p))
#endif

#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <linux/pm.h>
#include <linux/mutex.h>
#include <mach/soc.h>
#if defined (CONFIG_ARCH_S5P6818)
#include <mach/s5p6818.h>
#endif
#if defined (CONFIG_ARCH_S5P4418)
#include <mach/s5p4418.h>
#endif
#include <mach/devices.h>



#include "vpu_hw_interface.h"			//	Register Access
//#include "../firmware/blackbird.h"	//	v2.1.9
#include "../firmware/blackbird_v2.3.10.h"	//	v2.3.10
//#include "../firmware/coda960_v3.1.13.h"		// new version
#include "../include/nx_vpu_api.h"
#include "../include/nx_vpu_config.h"

//	system
#include "../include/nx_alloc_mem.h"
#include "../include/vpu_types.h"


#define	NX_DTAG		"[DRV|VPU_API]"
#include "../include/drv_osapi.h"

#define	DBG_POWER		0
#define	DBG_CLOCK		0
#define DBG_USERDATA	0
#define DBG_REGISTER    0
#define DBG_ES_ADDR		0


#include "nx_vpu_gdi.h"

#if ( ( defined(__LINUX__) || defined(linux) || defined(__linux__) )&& defined(__KERNEL__) )
#include <mach/soc.h>
#include <mach/platform.h>
#endif


#define	ENABLE_INTERRUPT_MODE
#define	INFO_MSG			0

//-----------------------------------------------------------------------------
//		Macro & Definitions

//	Interrupt Register Bit Enumerate
enum {
	VPU_INT_BIT_INIT            = 0,
	VPU_INT_BIT_SEQ_INIT        = 1,
	VPU_INT_BIT_SEQ_END         = 2,
	VPU_INT_BIT_PIC_RUN         = 3,
	VPU_INT_BIT_FRAMEBUF_SET    = 4,
	VPU_INT_BIT_ENC_HEADER      = 5,
	VPU_INT_BIT_DEC_PARA_SET    = 7,
	VPU_INT_BIT_DEC_BUF_FLUSH   = 8,
	VPU_INT_BIT_USERDATA        = 9,
	VPU_INT_BIT_DEC_FIELD       = 10,
	VPU_INT_BIT_DEC_MB_ROWS     = 13,
	VPU_INT_BIT_BIT_BUF_EMPTY   = 14,		//	Bit Stream Empty
	VPU_INT_BIT_BIT_BUF_FULL    = 15		//	Bit Stream Full
};


//----------------------------------------------------------------------------
//	Static Global Variables
static int gstIsVPUOn = 0;
static int gstIsInitialized	= 0;
static unsigned int	gstVpuRegStore[64];

static NX_VpuCodecInst gstVpuInstance[NX_MAX_VPU_INST_SPACE];


//----------------------------------------------------------------------------
//		Define Static Functions
static unsigned int VPU_IsBusy( void );
static NX_VPU_RET VPU_WaitVpuBusy( int mSeconds, unsigned int busyFlagReg );

//	Encoder Functions
static void VPU_EncDefaultParam(VpuEncInfo *pInfo);
static NX_VPU_RET VPU_EncSeqCommand(NX_VpuCodecInst *pInst);
static NX_VPU_RET VPU_EncSetFrameBufCommand(NX_VpuCodecInst *pInst);
static NX_VPU_RET VPU_EncOneFrameCommand( NX_VpuCodecInst *pInst, VPU_ENC_RUN_FRAME_ARG *runArg );
static NX_VPU_RET VPU_EncChangeParameterCommand( NX_VpuCodecInst *pInst, VPU_ENC_CHG_PARA_ARG *chgArg );
static NX_VPU_RET VPU_EncGetHeaderCommand(NX_VpuCodecInst *pInst, unsigned int headerType, unsigned char **ptr, int *size);
static NX_VPU_RET VPU_EncCloseCommand(NX_VpuCodecInst *pInst);

//	Decoder Functions
static NX_VPU_RET VPU_DecSeqInitCommand(NX_VpuCodecInst *pInst, VPU_DEC_SEQ_INIT_ARG *pArg);
static NX_VPU_RET VPU_DecSeqComplete( NX_VpuCodecInst *pInst, VPU_DEC_SEQ_INIT_ARG *pArg );
static NX_VPU_RET VPU_DecRegisterFrameBufCommand( NX_VpuCodecInst *pInst, VPU_DEC_REG_FRAME_ARG *pArg );
static NX_VPU_RET VPU_DecStartOneFrameCommand(NX_VpuCodecInst *pInst, VPU_DEC_DEC_FRAME_ARG *pArg);
static NX_VPU_RET VPU_DecGetOutputInfo(NX_VpuCodecInst *pInst, VPU_DEC_DEC_FRAME_ARG *pArg);
static NX_VPU_RET VPU_DecCloseCommand(NX_VpuCodecInst *pInst);

static int swap_endian(unsigned char *data, int len);
//static void DumpData( void *data, int len );


//----------------------------------------------------------------------------
//		Nexell Specific VPU Hardware On/Off Logic
//----------------------------------------------------------------------------

#define	VPU_ALIVEGATE_REG		0xF0010800
#define VPU_NISOLATE_REG		0xF0010D00
#define VPU_NPRECHARGE_REG		0xF0010D04
#define VPU_NPOWERUP_REG		0xF0010D08
#define VPU_NPOWERACK_REG		0xF0010D0C
#define CODA960CLKENB_REG		0xF00C7000

#define	POWER_PMU_VPU_MASK		0x00000002

#if defined (CONFIG_ARCH_S5P6818)
#define	TIEOFF_REG131			0xF001120C
#define	VPU_ASYNCXUI1_CACTIVE	(1<<17)
#define	VPU_ASYNCXUI1_CSYSACK	(1<<16)

#define	VPU_ASYNCXUI0_CACTIVE	(1<<15)
#define	VPU_ASYNCXUI0_CSYSACK	(1<<14)

#define	TIEOFF_REG69			0xF0011114
#define	VPU_ASYNCXUI1_CSYSREQ	(1<<4)
#define	VPU_ASYNCXUI0_CSYSREQ	(1<<3)

//
//		Async XUI Power Down
//
//	Step 1. Waiting until CACTIVE to High
//	Step 2. Set CSYSREQ to Low
//	Step 3. Waiting until CSYSACK to Low
static void NX_ASYNCXUI_PowerDown(void)
{
	int32_t tmpVal;
	FUNCIN();

	//	Apply To Async XUI 0

	//	Step 1. Waiting until CACTIVE to High
	do{
		tmpVal = VpuReadReg(TIEOFF_REG131);
	}while(!(tmpVal&VPU_ASYNCXUI0_CACTIVE));

	//	Step 2. Set CSYSREQ to Low
	tmpVal = VpuReadReg(TIEOFF_REG69);
	tmpVal &= (~VPU_ASYNCXUI0_CSYSREQ);
	VpuWriteReg(TIEOFF_REG69, tmpVal);

	//	Step 3. Waiting until CSYSACK to Low
	do{
		tmpVal = VpuReadReg(TIEOFF_REG131);
	}while(tmpVal&VPU_ASYNCXUI0_CSYSACK);


	//	Apply To Async XUI 1

	//	Step 1. Waiting until CACTIVE to High
	do{
		tmpVal = VpuReadReg(TIEOFF_REG131);
	}while(!(tmpVal&VPU_ASYNCXUI1_CACTIVE));

	//	Step 2. Set CSYSREQ to Low
	tmpVal = VpuReadReg(TIEOFF_REG69);
	tmpVal &= (~VPU_ASYNCXUI1_CSYSREQ);
	VpuWriteReg(TIEOFF_REG69, tmpVal);

	//	Step 3. Waiting until CSYSACK to Low
	do{
		tmpVal = VpuReadReg(TIEOFF_REG131);
	}while(tmpVal&VPU_ASYNCXUI1_CSYSACK);
	FUNCOUT();
}

//
//		Async XUI Power Up
//
//	Step 1. Set CSYSREQ to High
//	Step 2. Waiting until CSYSACK to High
static void NX_ASYNCXUI_PowerUp(void)
{
	int32_t tmpVal;

	FUNCIN();
	//	Apply To Async XUI 0

	//	Step 1. Set CSYSREQ to High
	tmpVal = VpuReadReg(TIEOFF_REG69);
	tmpVal |= VPU_ASYNCXUI0_CSYSREQ;
	VpuWriteReg(TIEOFF_REG69, tmpVal);

	//	Step 2. Waiting until CSYSACK to High
	do{
		tmpVal = VpuReadReg(TIEOFF_REG131);
	}while(!(tmpVal&VPU_ASYNCXUI0_CSYSACK));

	//	Apply To Async XUI 1

	//	Step 1. Set CSYSREQ to High
	tmpVal = VpuReadReg(TIEOFF_REG69);
	tmpVal |= VPU_ASYNCXUI1_CSYSREQ;
	VpuWriteReg(TIEOFF_REG69, tmpVal);

	//	Step 2. Waiting until CSYSACK to High
	do{
		tmpVal = VpuReadReg(TIEOFF_REG131);
	}while(!(tmpVal&VPU_ASYNCXUI1_CSYSACK));
	FUNCOUT();
}

#endif

void NX_VPU_HwOn(void)
{
	unsigned int tmpVal;
	NX_DbgMsg( DBG_POWER, ("NX_VPU_HwOn() ++\n") );

	//	Already Power On
	if( gstIsVPUOn )
	{
		return;
	}

	//	H/W Reset
	nxp_soc_peri_reset_enter(RESET_ID_CODA_C);			//	63
	nxp_soc_peri_reset_enter(RESET_ID_CODA_A);			//	61
	nxp_soc_peri_reset_enter(RESET_ID_CODA_P);			//	62

	VpuWriteReg( VPU_ALIVEGATE_REG,  0x3 );

	//	Enable PreCharge
	tmpVal = VpuReadReg(VPU_NPRECHARGE_REG);
	tmpVal &= (~POWER_PMU_VPU_MASK);
	VpuWriteReg( VPU_NPRECHARGE_REG, tmpVal );		//	Pre Charge

	//	Enable Power On
	tmpVal = VpuReadReg(VPU_NPOWERUP_REG);
	tmpVal &= (~POWER_PMU_VPU_MASK);
	VpuWriteReg( VPU_NPOWERUP_REG, tmpVal );

	//	Disable ISolate
	tmpVal = VpuReadReg(VPU_NISOLATE_REG);
	tmpVal |= (POWER_PMU_VPU_MASK);
	VpuWriteReg( VPU_NISOLATE_REG, tmpVal );

	mdelay( 1 );

	NX_VPU_Clock( 1 );

#if defined (CONFIG_ARCH_S5P6818)
	NX_ASYNCXUI_PowerUp();
#endif
	//	Release Reset
	nxp_soc_peri_reset_exit(RESET_ID_CODA_P);			//	62
	nxp_soc_peri_reset_exit(RESET_ID_CODA_A);			//	61
	nxp_soc_peri_reset_exit(RESET_ID_CODA_C);			//	63

	gstIsVPUOn = 1;

	NX_DbgMsg( DBG_POWER, ("NX_VPU_HwOn() --\n") );
}

void NX_VPU_HWOff(void)
{
	FUNCIN();
	if( gstIsVPUOn )
	{
		unsigned int tmpVal;

#if defined (CONFIG_ARCH_S5P6818)
		NX_ASYNCXUI_PowerDown();
#endif
		//	H/W Reset
		nxp_soc_peri_reset_enter(RESET_ID_CODA_C);			//	63
		nxp_soc_peri_reset_enter(RESET_ID_CODA_A);			//	61
		nxp_soc_peri_reset_enter(RESET_ID_CODA_P);			//	62

		NX_DbgMsg( DBG_POWER, ("NX_VPU_HWOff() ++\n") );

		//	Enter Coda Reset State
		VpuWriteReg( VPU_ALIVEGATE_REG,  0x3 );

		//	Isolate VPU H/W
		tmpVal = VpuReadReg( VPU_NISOLATE_REG );
		tmpVal &= (~POWER_PMU_VPU_MASK);
		VpuWriteReg( VPU_NISOLATE_REG,   tmpVal );		//	Isolate

		//	Pre Charget Off
		tmpVal = VpuReadReg( VPU_NPRECHARGE_REG );
		tmpVal |= POWER_PMU_VPU_MASK;
		VpuWriteReg( VPU_NPRECHARGE_REG, tmpVal );

		//	Power Down
		tmpVal = VpuReadReg( VPU_NPOWERUP_REG );
		tmpVal |= POWER_PMU_VPU_MASK;
		VpuWriteReg( VPU_NPOWERUP_REG,   tmpVal );

		//	Isolate VPU H/W
		// tmpVal = VpuReadReg( VPU_NISOLATE_REG );
		// tmpVal &= (~POWER_PMU_VPU_MASK);
		// VpuWriteReg( VPU_NISOLATE_REG,   tmpVal );		//	Isolate

		gstIsVPUOn = 0;
		NX_DbgMsg( DBG_POWER, ("NX_VPU_HWOff() --\n") );
	}
	FUNCOUT();
}

int NX_VPU_GetCurPowerState(void)
{
	return gstIsVPUOn;
}

void NX_VPU_Clock( int on )
{
	FUNCIN();
	if( on )
	{
		VpuWriteReg( CODA960CLKENB_REG, 0x0000000F );		//	BCLK[0:2] : 3(Always), PCLK[4] : 1(Enable)
		NX_DbgMsg( DBG_CLOCK, ("NX_VPU_Clock() ON\n") );
	}
	else
	{
		VpuWriteReg( CODA960CLKENB_REG, 0x00000000 );		//	BCLK[0:2] : 3(Always), PCLK[4] : 1(Enable)
		NX_DbgMsg( DBG_CLOCK, ("NX_VPU_Clock() OFF\n") );
	}
	FUNCOUT();
}


//----------------------------------------------------------------------------
//
//			Linux VPU Interrupt Handler
//

static DECLARE_WAIT_QUEUE_HEAD(gst_VPU_WaitQueue);
static atomic_t gVpuEventPresent =  ATOMIC_INIT(0);

static irqreturn_t VPU_InterruptHandler( int irq, void *desc )
{
	VpuWriteReg(BIT_INT_CLEAR, 0x1);
	atomic_set(&gVpuEventPresent, 1);
	wake_up_interruptible(&gst_VPU_WaitQueue);
	return IRQ_HANDLED;
}


//
//	Initialize Interrupt
//
int VPU_InitInterrupt(void)
{
#ifdef ENABLE_INTERRUPT_MODE
	//	Initialize Interrupt
	if(request_irq( IRQ_PHY_CODA960_HOST, VPU_InterruptHandler, 0, "VPU_CODEC_IRQ", NULL) != 0)
	{
		printk("%s:%d Cannot get IRQ %d\n", __func__, __LINE__, IRQ_PHY_CODA960_HOST );
		return -1;
	}
	init_waitqueue_head(&gst_VPU_WaitQueue);
	atomic_set(&gVpuEventPresent, 0);
#endif
	return 0;
}

void VPU_DeinitInterrupt(void)
{
#ifdef ENABLE_INTERRUPT_MODE
	free_irq(IRQ_PHY_CODA960_HOST, NULL);
#endif
}

int VPU_WaitVpuInterrupt( int timeOut )
{
	int ret = wait_event_interruptible_timeout(gst_VPU_WaitQueue, atomic_read(&gVpuEventPresent), msecs_to_jiffies(timeOut));

	if( 0 == atomic_read(&gVpuEventPresent) )
	{
		//	Error
		if( ret == 0 )
		{
			//	Time out
			NX_ErrMsg(("VPU HW Timeout!\n"));
			atomic_set(&gVpuEventPresent, 0);
			VPU_SWReset( SW_RESET_SAFETY );
			return -1;
		}
		else
		{
			while(timeOut > 0)
			{
				if( 0 != VpuReadReg(BIT_INT_REASON) )
				{
					atomic_set(&gVpuEventPresent, 0);
					return 0;
				}
				DrvMSleep( 1 );
				timeOut --;
			}
			//	Time out
			NX_ErrMsg(("VPU HW Error!!\n"));
			VPU_SWReset( SW_RESET_SAFETY );
			atomic_set(&gVpuEventPresent, 0);
			return -1;
		}
	}
	atomic_set(&gVpuEventPresent, 0);
	return 0;
}
//
//
//////////////////////////////////////////////////////////////////////////////



//----------------------------------------------------------------------------
static unsigned int VPU_IsBusy(void)
{
	unsigned int ret = 0;
	ret = ReadRegNoMsg(BIT_BUSY_FLAG);
	return ret != 0;
}

//----------------------------------------------------------------------------
static NX_VPU_RET VPU_WaitBusBusy( int mSeconds, unsigned int busyFlagReg )	//	Milli Sconds
{
	while(mSeconds > 0)
	{
		if( 0x77 == VpuReadReg(busyFlagReg) )
			return VPU_RET_OK;
		DrvMSleep( 1 );
		mSeconds --;
	}
	return VPU_RET_ERR_TIMEOUT;
}


//----------------------------------------------------------------------------
static NX_VPU_RET VPU_WaitVpuBusy( int mSeconds, unsigned int busyFlagReg )	//	Milli Sconds
{
	while(mSeconds > 0)
	{
		if( ReadRegNoMsg(busyFlagReg) == 0 )
			return VPU_RET_OK;
		DrvMSleep( 1 );
		mSeconds --;
	}
	return VPU_RET_ERR_TIMEOUT;
}

//----------------------------------------------------------------------------
static int VPU_WaitBitInterrupt(int mSeconds)
{
#ifdef ENABLE_INTERRUPT_MODE
	unsigned int reason=0;
	if( 0 != VPU_WaitVpuInterrupt( mSeconds ) )
	{
		reason=VpuReadReg(BIT_INT_REASON);
		VpuWriteReg(BIT_INT_REASON, 0);
		NX_ErrMsg(("VPU_WaitVpuInterrupt() TimeOut!!!(reason = 0x%.8x, CurPC(0xBD 0xBF : %x %x %x))\n", reason, VpuReadReg(BIT_CUR_PC), VpuReadReg(BIT_CUR_PC), VpuReadReg(BIT_CUR_PC) ));
		return 0;
	}
	else
	{
		//NX_ErrMsg(("Success CurPC = %x %x %x\n", VpuReadReg(BIT_CUR_PC), VpuReadReg(BIT_CUR_PC), VpuReadReg(BIT_CUR_PC) ));
		VpuWriteReg(BIT_INT_CLEAR, 1);		// clear HW signal
		reason=VpuReadReg(BIT_INT_REASON);
		VpuWriteReg(BIT_INT_REASON, 0);
		return reason;
	}
#else
	unsigned int reason=0;
	while(mSeconds > 0)
	{
		reason=VpuReadReg(BIT_INT_REASON);
		if( reason!=0 )
		{
			if (reason != (unsigned int)-1)
				VpuWriteReg(BIT_INT_CLEAR, 1);		// clear HW signal
			VpuWriteReg(BIT_INT_REASON, 0);     // tell to F/W that HOST received an interrupt.
			break;
		}
		DrvMSleep( 1 );
		mSeconds --;
	}
	return reason;
#endif
}


void VPU_GetVersionInfo(unsigned int *versionInfo, unsigned int *revision, unsigned int *productId)
{
	unsigned int ver;
	unsigned int rev;
	unsigned int pid;

	if (versionInfo && revision)
	{
		VpuWriteReg( RET_FW_VER_NUM , 0);
		VpuWriteReg(BIT_WORK_BUF_ADDR, 0);
		VpuWriteReg(BIT_BUSY_FLAG, 1);
		VpuWriteReg(BIT_RUN_INDEX, 0);
		VpuWriteReg(BIT_RUN_COD_STD, 0);
		VpuWriteReg(BIT_RUN_AUX_STD, 0);
		VpuWriteReg(BIT_RUN_COMMAND, FIRMWARE_GET);
		if ( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
		{
			NX_ErrMsg(("Version Read Failed!!!\n"));
			return;
		}

		ver = VpuReadReg(RET_FW_VER_NUM);
		rev = VpuReadReg(RET_FW_CODE_REV);

		*versionInfo = ver;
		*revision = rev;
	}

	pid = VpuReadReg(DBG_CONFIG_REPORT_1);
    if ((pid&0xff00) == 0x3200) pid = 0x3200;
	if (productId)
		*productId = pid;
}

//----------------------------------------------------------------------------
void CheckVersion(void)
{
	unsigned int version;
	unsigned int revision;
	unsigned int productId;

	VPU_GetVersionInfo(&version, &revision, &productId);

	NX_DbgMsg( 0, ("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n",
		(unsigned int)(version>>16), (unsigned int)((version>>(12))&0x0f),
		(unsigned int)((version>>(8))&0x0f), (unsigned int)((version)&0xff), revision) );
	NX_DbgMsg( 0, ("Hardware Version => %04x\n", productId));
	NX_DbgMsg( INFO_MSG, ("Build Date : %s, %s\n", __DATE__, __TIME__));
}



//----------------------------------------------------------------------------

/**
* VPU_SWReset
* IN
*    forcedReset : 1 if there is no need to waiting for BUS transaction,
*                  0 for otherwise
* OUT
*    RetCode : RETCODE_FAILURE if failed to reset,
*              RETCODE_SUCCESS for otherwise
*/
// SW Reset command
#define VPU_SW_RESET_BPU_CORE   0x008
#define VPU_SW_RESET_BPU_BUS    0x010
#define VPU_SW_RESET_VCE_CORE   0x020
#define VPU_SW_RESET_VCE_BUS    0x040
#define VPU_SW_RESET_GDI_CORE   0x080
#define VPU_SW_RESET_GDI_BUS    0x100
int VPU_SWReset(int resetMode)
{
	unsigned int cmd;
	if (resetMode == SW_RESET_SAFETY || resetMode == SW_RESET_ON_BOOT)
	{
		// Waiting for completion of bus transaction
		// Step1 : No more request
		VpuWriteReg(GDI_BUS_CTRL, 0x11);	// no more request {3'b0,no_more_req_sec,3'b0,no_more_req}

		// Step2 : Waiting for completion of bus transaction
		// while (VpuReadReg(coreIdx, GDI_BUS_STATUS) != 0x77)
		// ;
		if (VPU_WaitBusBusy(VPU_BUSY_CHECK_TIMEOUT, GDI_BUS_STATUS) == VPU_RET_ERR_TIMEOUT)
		{
			VpuWriteReg(GDI_BUS_CTRL, 0x00);
			return VPU_RET_ERR_TIMEOUT;
		}
		// Step3 : clear GDI_BUS_CTRL
		VpuWriteReg(GDI_BUS_CTRL, 0x00);
	}

	cmd = 0;
	// Software Reset Trigger
	if (resetMode != SW_RESET_ON_BOOT)
		cmd =  VPU_SW_RESET_BPU_CORE | VPU_SW_RESET_BPU_BUS;
	cmd |= VPU_SW_RESET_VCE_CORE | VPU_SW_RESET_VCE_BUS;
	if (resetMode == SW_RESET_ON_BOOT)
		cmd |= VPU_SW_RESET_GDI_CORE | VPU_SW_RESET_GDI_BUS;// If you reset GDI, tiled map should be reconfigured
	VpuWriteReg(BIT_SW_RESET, cmd);

	// wait until reset is done
	// while(VpuReadReg(coreIdx, BIT_SW_RESET_STATUS) != 0)
	//	;
	if (VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_SW_RESET_STATUS) == VPU_RET_ERR_TIMEOUT)
	{
		VpuWriteReg(BIT_SW_RESET, 0x00);
		return VPU_RET_ERR_TIMEOUT;
	}

	VpuWriteReg(BIT_SW_RESET, 0);

	return VPU_RET_OK;
}



//----------------------------------------------------------------------------
//	VPU Initialization.
//----------------------------------------------------------------------------
NX_VPU_RET	NX_VpuInit( unsigned int firmVirAddr, unsigned int firmPhyAddr )
{
	NX_VPU_RET	ret = VPU_RET_OK;
	int i;
	unsigned int tmpData;
	unsigned int codeBufAddr, tmpBufAddr, paramBufAddr;

	if( gstIsInitialized )
		return VPU_RET_OK;

	codeBufAddr = firmPhyAddr;
	tmpBufAddr  = codeBufAddr + CODE_BUF_SIZE;
	paramBufAddr = tmpBufAddr + TEMP_BUF_SIZE;

	NX_VPU_HwOn();

	VPU_SWReset( SW_RESET_ON_BOOT );

	{
		unsigned char *dst = (unsigned char *)firmVirAddr;
		int size = sizeof( bit_code ) / sizeof( bit_code[0] );
		for( i=0; i < size ; i+=4 )
		{
			*dst++ = (unsigned char)(bit_code[i+3] >> 0);
			*dst++ = (unsigned char)(bit_code[i+3] >> 8);

			*dst++ = (unsigned char)(bit_code[i+2] >> 0);
			*dst++ = (unsigned char)(bit_code[i+2] >> 8);

			*dst++ = (unsigned char)(bit_code[i+1] >> 0);
			*dst++ = (unsigned char)(bit_code[i+1] >> 8);

			*dst++ = (unsigned char)(bit_code[i+0] >> 0);
			*dst++ = (unsigned char)(bit_code[i+0] >> 8);
		}
	}

	VpuWriteReg( BIT_INT_ENABLE, 0);
	VpuWriteReg( BIT_CODE_RUN, 0);

	for( i=0 ; i<2048 ; i++ )
	{
		tmpData = bit_code[i];
		WriteRegNoMsg( BIT_CODE_DOWN, (i<<16)|tmpData );
	}

	VpuWriteReg(BIT_PARA_BUF_ADDR, paramBufAddr );
	VpuWriteReg(BIT_CODE_BUF_ADDR, codeBufAddr );
	VpuWriteReg(BIT_TEMP_BUF_ADDR, tmpBufAddr  );

	VpuWriteReg(BIT_BIT_STREAM_CTRL, VPU_STREAM_ENDIAN);
	VpuWriteReg(BIT_FRAME_MEM_CTRL, CBCR_INTERLEAVE<<2|VPU_FRAME_ENDIAN); // Interleave bit position is modified
	VpuWriteReg(BIT_BIT_STREAM_PARAM, 0);

	VpuWriteReg(BIT_AXI_SRAM_USE, 0);
	VpuWriteReg(BIT_INT_ENABLE, 0);
	VpuWriteReg(BIT_ROLLBACK_STATUS, 0);

	tmpData  = (1<<VPU_INT_BIT_BIT_BUF_FULL);		//	bitstream Full
	tmpData |= (1<<VPU_INT_BIT_BIT_BUF_EMPTY);		//	bitstream Empty
	tmpData |= (1<<VPU_INT_BIT_DEC_MB_ROWS);
	tmpData |= (1<<VPU_INT_BIT_SEQ_INIT);
	tmpData |= (1<<VPU_INT_BIT_DEC_FIELD);
	tmpData |= (1<<VPU_INT_BIT_PIC_RUN);
	VpuWriteReg(BIT_INT_ENABLE, tmpData);
	VpuWriteReg(BIT_INT_CLEAR, 0x1);

	VpuWriteReg(BIT_USE_NX_EXPND, USE_NX_EXPND);	//	Nexell AXI Expander ( Enable )
	VpuWriteReg(BIT_BUSY_FLAG, 0x1);
	VpuWriteReg(BIT_CODE_RESET, 1);
	VpuWriteReg(BIT_CODE_RUN, 1);

	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		NX_ErrMsg(("NX_VpuInit() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
		return ret;
	}

	CheckVersion();

	for( i=0 ; i<NX_MAX_VPU_INST_SPACE ; i++ )
	{
		gstVpuInstance[i].inUse = 0;
		gstVpuInstance[i].paramPhyAddr = paramBufAddr;
		gstVpuInstance[i].paramVirAddr = firmVirAddr + CODE_BUF_SIZE + TEMP_BUF_SIZE;
		gstVpuInstance[i].paramBufSize = PARA_BUF_SIZE;
	}
	gstIsInitialized = 1;

	return ret;
}


//----------------------------------------------------------------------------
NX_VPU_RET	NX_VpuDeInit( void )
{
	if( !gstIsInitialized )
	{
		NX_ErrMsg(("VPU Already Denitialized!!!\n"));
		return VPU_RET_ERR_INIT;
	}
	if( VPU_IsBusy() )
	{
		NX_ErrMsg(("NX_VpuDeInit() failed. VPU_IsBusy!!!\n"));
		return VPU_RET_BUSY;
	}
	NX_VPU_HWOff();
	gstIsInitialized = 0;
	return VPU_RET_OK;
}

#if 0
#ifdef PM_DBGOUT
#undef PM_DBGOUT
#endif

#define PM_DBGOUT	lldebugout
#endif

//----------------------------------------------------------------------------
NX_VPU_RET	NX_VpuSuspend( void )
{
	int i;
	if( !gstIsInitialized )
		return VPU_RET_ERR_INIT;

	if( VPU_IsBusy() )
		return VPU_RET_BUSY;
	for ( i = 0 ; i < 64 ; i++)
		gstVpuRegStore[i] = VpuReadReg(BIT_BASE+0x100 + (i * 4));

	NX_VPU_HWOff();

	return VPU_RET_OK;
}

//----------------------------------------------------------------------------
NX_VPU_RET	NX_VpuResume( void )
{
	int i;
	unsigned int value;

	if( !gstIsInitialized )
		return VPU_RET_ERR_INIT;

	NX_VPU_HwOn();

	VPU_SWReset(SW_RESET_ON_BOOT);

	for (i = 0 ; i < 64 ; i++)
		VpuWriteReg(BIT_BASE+0x100+(i * 4), gstVpuRegStore[i]);

	VpuWriteReg(BIT_CODE_RUN, 0);
	//	Bit Code
	for( i = 0; i <2048; i++ )
	{
		value = bit_code[i];
		VpuWriteReg(BIT_CODE_DOWN, ((i << 16) | value));
	}

	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_CODE_RESET, 1);
	VpuWriteReg(BIT_CODE_RUN, 1);

	VpuWriteReg(BIT_USE_NX_EXPND, USE_NX_EXPND);	//	Nexell AXI Expander ( Enable )

	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		NX_ErrMsg(("NX_VpuResume() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
		return VPU_RET_ERR_TIMEOUT;
	}

	return VPU_RET_OK;
}




//----------------------------------------------------------------------------
//
//							Encoder APIs


//	Encoder Open
NX_VPU_RET	NX_VpuEncOpen( VPU_OPEN_ARG *openArg, void *drvHandle, NX_VPU_INST_HANDLE *handle )
{
	int i;
	VpuEncInfo *pEncInfo;
	NX_VPU_INST_HANDLE hInst=0;
	*handle = 0;
	if( !gstIsInitialized )
		return VPU_RET_ERR_INIT;

	for( i=0; i<NX_MAX_VPU_INSTANCE ; i++ )
	{
		if( !gstVpuInstance[i].inUse )
		{
			hInst = &gstVpuInstance[i];
			break;
		}
	}

	if( NX_MAX_VPU_INSTANCE==i )
	{
		return VPU_RET_ERR_INST;
	}

	if( CODEC_STD_AVC == openArg->codecStd )
	{
		hInst->codecMode = AVC_ENC;
		hInst->auxMode = 0;
	}
	else if( CODEC_STD_MPEG4 == openArg->codecStd )
	{
		hInst->codecMode = MP4_ENC;
		hInst->auxMode = 0;
	}
	else if( CODEC_STD_H263 == openArg->codecStd )
	{
		hInst->codecMode = MP4_ENC;
		hInst->auxMode = 1;
	}
	else if( CODEC_STD_MJPG == openArg->codecStd )
	{
		hInst->codecMode = MJPG_ENC;
		hInst->auxMode = 0;
	}
	else
	{
		NX_ErrMsg(("NX_VpuEncOpen() failed. Cannot support codec standard(%d)\n", openArg->codecStd));
		return VPU_RET_ERR_PARAM;
	}

	NX_DrvMemset( &hInst->codecInfo, 0, sizeof(hInst->codecInfo) );

	hInst->inUse = 1;
	hInst->instIndex = i;
	hInst->devHandle = drvHandle;

	hInst->instBufPhyAddr = openArg->instanceBuf.phyAddr;
	hInst->instBufVirAddr = openArg->instanceBuf.virAddr;
	hInst->instBufSize    = openArg->instanceBuf.size;

	pEncInfo = &hInst->codecInfo.encInfo;
	pEncInfo->codecStd = openArg->codecStd;

	//	Set Default Codec Parameters
	VPU_EncDefaultParam( pEncInfo );
	*handle = hInst;
	return VPU_RET_OK;
}

//----------------------------------------------------------------------------
NX_VPU_RET	NX_VpuEncClose( NX_VPU_INST_HANDLE handle )
{
	NX_VPU_RET ret;

	if( MJPG_ENC == handle->codecMode )
	{
		return VPU_RET_OK;
	}

	if( VPU_RET_OK != (ret = VPU_EncCloseCommand(handle)) )
	{
		NX_ErrMsg(("NX_VpuEncClose() failed.(%d)\n", ret));
	}
	return ret;
}



NX_VPU_RET	NX_VpuEncSetSeqParam( NX_VPU_INST_HANDLE handle, VPU_ENC_SEQ_ARG *seqArg )
{
	VpuEncInfo *encInfo = &handle->codecInfo.encInfo;

	encInfo->srcWidth = seqArg->srcWidth;
	encInfo->srcHeight = seqArg->srcHeight;
	encInfo->encWidth = seqArg->srcWidth;
	encInfo->encHeight = seqArg->srcHeight;
	encInfo->gopSize = seqArg->gopSize;
	encInfo->frameRateNum = seqArg->frameRateNum;
	encInfo->frameRateDen = seqArg->frameRateDen;

	encInfo->rcEnable = ( seqArg->RCModule != 1) ? (0) : (1);
	encInfo->bitRate = (seqArg->bitrate/1024)&0x7FFF;			//	in KiloByte
	encInfo->userQpMax = seqArg->maxQP;
	encInfo->enableAutoSkip = !seqArg->disableSkip;
	encInfo->initialDelay = seqArg->initialDelay;
	encInfo->vbvBufSize = seqArg->vbvBufferSize;
	encInfo->userGamma = seqArg->gammaFactor;
	encInfo->frameQp = seqArg->initQP;

	encInfo->MESearchRange = seqArg->searchRange;
	encInfo->intraRefresh = seqArg->intraRefreshMbs;

	encInfo->cbcrInterleave = seqArg->chromaInterleave;
	encInfo->cbcrInterleaveRefFrame = seqArg->refChromaInterleave;

	encInfo->rotateAngle = seqArg->rotAngle / 90;
	encInfo->mirrorDirection = seqArg->mirDirection;

	encInfo->strmBufVirAddr = seqArg->strmBufVirAddr;
	encInfo->strmBufPhyAddr = seqArg->strmBufPhyAddr;
	encInfo->strmBufSize = seqArg->strmBufSize;

	encInfo->jpegQuality = seqArg->quality;

	encInfo->EncCodecParam.avcEncParam.audEnable = seqArg->enableAUDelimiter;

	NX_DbgMsg(INFO_MSG, ("NX_VpuEncSetSeqParam() : %dx%d, %d/%d fps, %d kbps (gop(%d), maxQ(%d), SR(%d), StreamBuffer(0x%08x, 0x%08x))\n",
		encInfo->srcWidth, encInfo->srcHeight, encInfo->frameRateNum, encInfo->frameRateDen, encInfo->bitRate,
		encInfo->gopSize, encInfo->userQpMax, encInfo->MESearchRange, encInfo->strmBufPhyAddr, encInfo->strmBufVirAddr));

	if( CODEC_STD_MJPG == encInfo->codecStd )
	{
		EncJpegInfo *pJpgInfo = &encInfo->EncCodecParam.jpgEncInfo;

		if( seqArg->quality == 0 || seqArg->quality<0 || seqArg->quality>100 )
		{
			encInfo->jpegQuality = 90;
		}
		else
		{
			encInfo->jpegQuality = seqArg->quality;
		}

		if( 0 != encInfo->rotateAngle )
		{
			pJpgInfo->rotationEnable = 1;
			pJpgInfo->rotationAngle = encInfo->rotateAngle;
		}
		if (0 != encInfo->mirrorDirection )
		{
			pJpgInfo->mirrorEnable = 1;
			pJpgInfo->mirrorDirection = encInfo->mirrorDirection;
		}

		JpuSetupTables( pJpgInfo, encInfo->jpegQuality );

		handle->isInitialized = 1;
		return VPU_RET_OK;
	}

	return VPU_EncSeqCommand( handle );
}


//
//	Frame Buffer Address Allocation은 Initialization 시에 미리 받아서 처리하게 되어 있기 때문에
//	굳이 별도로 Frame 가져올때 하지 않기로 한다.
//
NX_VPU_RET	NX_VpuEncSetFrame( NX_VPU_INST_HANDLE handle, VPU_ENC_SET_FRAME_ARG *frmArg )
{
	int i;
	VpuEncInfo *pEncInfo = &handle->codecInfo.encInfo;

	pEncInfo->minFrameBuffers = frmArg->numFrameBuffer;
	for( i=0 ; i<pEncInfo->minFrameBuffers ; i++ )
	{
		pEncInfo->frameBuffer[i] = frmArg->frameBuffer[i];
	}
	pEncInfo->subSampleAPhyAddr = frmArg->subSampleBuffer[0].phyAddr;
	pEncInfo->subSampleBPhyAddr = frmArg->subSampleBuffer[1].phyAddr;
	return VPU_EncSetFrameBufCommand( handle );
}


//
//	Real Memory Transaction을 구현하여야 함.
//
NX_VPU_RET	NX_VpuEncGetHeader( NX_VPU_INST_HANDLE handle, VPU_ENC_GET_HEADER_ARG *header )
{
	NX_VPU_RET ret = VPU_RET_OK;
	unsigned char *ptr;
	int size;

	if( AVC_ENC == handle->codecMode )
	{
		//	SPS
		if( VPU_RET_OK != (ret = VPU_EncGetHeaderCommand(handle, SPS_RBSP, &ptr, &size)) )
		{
			NX_ErrMsg(("NX_VpuEncGetHeader() SPS_RBSP Error!\n"));
			goto GET_HEADER_EXIT;
		}
		NX_DrvMemcpy( header->avcHeader.spsData, ptr, size );
		header->avcHeader.spsSize = size;
		//	PPS
		if( VPU_RET_OK != (ret = VPU_EncGetHeaderCommand(handle, PPS_RBSP, &ptr, &size)) )
		{
			NX_ErrMsg(("NX_VpuEncGetHeader() PPS_RBSP Error!\n"));
			goto GET_HEADER_EXIT;
		}
		NX_DrvMemcpy( header->avcHeader.ppsData, ptr, size );
		header->avcHeader.ppsSize = size;
	}
	else if ( MP4_ENC == handle->codecMode )
	{
		//	VOS
		if( VPU_RET_OK != (ret = VPU_EncGetHeaderCommand(handle, VOS_HEADER, &ptr, &size)) )
		{
			NX_ErrMsg(("NX_VpuEncGetHeader() VOS_HEADER Error!\n"));
			goto GET_HEADER_EXIT;
		}
		NX_DrvMemcpy( header->mp4Header.vosData, ptr, size );
		header->mp4Header.vosSize = size;
		//	VOL
		if( VPU_RET_OK != (ret = VPU_EncGetHeaderCommand(handle, VOL_HEADER, &ptr, &size)) )
		{
			NX_ErrMsg(("NX_VpuEncGetHeader() VOL_HEADER Error!\n"));
			goto GET_HEADER_EXIT;
		}
		NX_DrvMemcpy( header->mp4Header.volData, ptr, size );
		header->mp4Header.volSize = size;
	}
GET_HEADER_EXIT:
	return ret;
}

//
//
//
NX_VPU_RET	NX_VpuEncRunFrame( NX_VPU_INST_HANDLE handle, VPU_ENC_RUN_FRAME_ARG *runArg )
{
	return VPU_EncOneFrameCommand( handle, runArg );
}

NX_VPU_RET	NX_VpuEncChgParam( NX_VPU_INST_HANDLE handle, VPU_ENC_CHG_PARA_ARG *chgArg )
{
	return VPU_EncChangeParameterCommand( handle, chgArg );
}


//----------------------------------------------------------------------------
//
//						Encoder Specific Static Functions

//	Set Encoder Default Parameter
static void VPU_EncDefaultParam(VpuEncInfo *pEncInfo)
{
	//	Set Default Frame Rate
	pEncInfo->frameRateNum = 30;
	pEncInfo->frameRateDen = 1;

	//	Set Slice Mode
	pEncInfo->sliceMode = 0;		//	one slice per picture
	pEncInfo->sliceSizeMode = 0;
	pEncInfo->sliceSize = 0;

	//	Set GOP Size
	pEncInfo->gopSize = 30;

	//	Rate Control Related
	pEncInfo->rcEnable = 0;
	pEncInfo->intraRefresh = 0;	//	Disalbe Intra Refresh
	pEncInfo->rcIntraQp = -1;		//	Disable

	pEncInfo->bwbEnable = VPU_ENABLE_BWB;
	pEncInfo->cbcrInterleave = CBCR_INTERLEAVE;
	pEncInfo->cbcrInterleaveRefFrame = ENC_FRAME_BUF_CBCR_INTERLEAVE;
	pEncInfo->frameEndian = VPU_FRAME_ENDIAN;

	pEncInfo->rcIntervalMode = 0;	//	Frame Mode
	pEncInfo->rcIntraCostWeigth = 400;
	pEncInfo->enableAutoSkip = 0;
	pEncInfo->initialDelay = 0;
	pEncInfo->vbvBufSize = 0;

	pEncInfo->userGamma = (unsigned int)(0.75*32768);		//  (0*32768 < gamma < 1*32768)

	if( CODEC_STD_AVC == pEncInfo->codecStd )
	{
		EncAvcParam *pAvcParam = &pEncInfo->EncCodecParam.avcEncParam;

		pAvcParam->chromaQpOffset = 0;
		pAvcParam->constrainedIntraPredFlag = 0;
		pAvcParam->disableDeblk = 0;
		pAvcParam->deblkFilterOffsetAlpha = 0;
		pAvcParam->audEnable = 0;

		pEncInfo->userQpMax = 51;
	}
	else if(  CODEC_STD_MPEG4 == pEncInfo->codecStd )
	{
		EncMp4Param *pMp4Param = &pEncInfo->EncCodecParam.mp4EncParam;
		pMp4Param->mp4DataPartitionEnable = 0;
		pMp4Param->mp4ReversibleVlcEnable = 0;
		pMp4Param->mp4IntraDcVlcThr = 0;
		pMp4Param->mp4HecEnable	= 0;
		pMp4Param->mp4Verid = 2;

		pEncInfo->userQpMax = 31;
	}
	else if(  CODEC_STD_H263 == pEncInfo->codecStd )
	{
		EncH263Param *pH263Param = &pEncInfo->EncCodecParam.h263EncParam;
		pH263Param->h263AnnexIEnable = 0;
		pH263Param->h263AnnexJEnable = 1;
		pH263Param->h263AnnexKEnable = 0;
		pH263Param->h263AnnexTEnable = 1;

		pEncInfo->userQpMax = 31;
	}
	else if ( CODEC_STD_MJPG == pEncInfo->codecStd )
	{
		VPU_EncMjpgDefParam( pEncInfo );
	}

	//	Motion Estimation
	pEncInfo->MEUseZeroPmv = 0;
	pEncInfo->MESearchRange = 1;	//	0 : Horizontal(-128~127), Vertical(-64~63)
									//	1 : Horizontal(-64~63), Vertical(-32~31)
									//	2 : Horizontal(-32~31), Vertical(-16~15)
									//	3 : Horizontal(-16~15), Vertical(-16~15)

	pEncInfo->MEBlockMode = 0;		//	Use All Macro Block Type

	//	decoder(0), cbcr interleve(0), bypass(0), burst(0), merge(3), maptype(linear), wayshape(15)
	pEncInfo->cacheConfig = MaverickCache2Config( 0, pEncInfo->cbcrInterleave, 0, 0, 3, 0, 15 );
}

//	Encoder ENC_SEQ_INIT Command
static NX_VPU_RET VPU_EncSeqCommand(NX_VpuCodecInst *pInst)
{
	VpuEncInfo *pEncInfo = &pInst->codecInfo.encInfo;
	unsigned int tmpData;

	//	Write Bitstream Buffer Information
	VpuWriteReg(CMD_ENC_SEQ_BB_START, pEncInfo->strmBufPhyAddr);
	VpuWriteReg(CMD_ENC_SEQ_BB_SIZE, pEncInfo->strmBufSize / 1024); // size in KB

	//	Set Source Image Information
	if( 90 == pEncInfo->rotateAngle || 270 == pEncInfo->rotateAngle )
		tmpData = (pEncInfo->srcHeight<< 16) | pEncInfo->srcWidth;
	else
		tmpData = (pEncInfo->srcWidth<< 16) | pEncInfo->srcHeight;

	VpuWriteReg(CMD_ENC_SEQ_SRC_SIZE, tmpData);
	VpuWriteReg(CMD_ENC_SEQ_SRC_F_RATE, (pEncInfo->frameRateNum) | ((pEncInfo->frameRateDen-1) << 16));		//	TODO : CheckME

	if (pEncInfo->codecStd == CODEC_STD_MPEG4)
	{
		EncMp4Param *pMp4Param = &pEncInfo->EncCodecParam.mp4EncParam;
		VpuWriteReg(CMD_ENC_SEQ_COD_STD, 3);
		tmpData = pMp4Param->mp4IntraDcVlcThr << 2 |
			      pMp4Param->mp4ReversibleVlcEnable << 1 |
			      pMp4Param->mp4DataPartitionEnable;
		tmpData |= ((pMp4Param->mp4HecEnable >0)? 1:0)<<5;
		tmpData |= ((pMp4Param->mp4Verid == 2)? 0:1) << 6;
		VpuWriteReg(CMD_ENC_SEQ_MP4_PARA, tmpData);
	}
	else if (pEncInfo->codecStd == CODEC_STD_H263) {
		EncH263Param *pH263Param = &pEncInfo->EncCodecParam.h263EncParam;
		VpuWriteReg(CMD_ENC_SEQ_COD_STD, 11);
		tmpData = pH263Param->h263AnnexIEnable << 3 |
			      pH263Param->h263AnnexJEnable << 2 |
			      pH263Param->h263AnnexKEnable << 1|
			      pH263Param->h263AnnexTEnable;
		VpuWriteReg(CMD_ENC_SEQ_263_PARA, tmpData);
	}
	else if (pEncInfo->codecStd == CODEC_STD_AVC) {
		EncAvcParam *pAvcParam = &pEncInfo->EncCodecParam.avcEncParam;
		VpuWriteReg(CMD_ENC_SEQ_COD_STD, 0x0);
		tmpData = (pAvcParam->deblkFilterOffsetBeta & 15) << 12 |
				  (pAvcParam->deblkFilterOffsetAlpha & 15) << 8 |
				  pAvcParam->disableDeblk << 6 |
				  pAvcParam->constrainedIntraPredFlag << 5 |
				  (pAvcParam->chromaQpOffset & 31);
		VpuWriteReg(CMD_ENC_SEQ_264_PARA, tmpData);
	}

	//	Slice Mode
	tmpData  = pEncInfo->sliceSize << 2 |  pEncInfo->sliceSizeMode << 1 | pEncInfo->sliceMode;
	VpuWriteReg(CMD_ENC_SEQ_SLICE_MODE, tmpData);

	//	Write GOP Size
	if (pEncInfo->rcEnable)
	{
		VpuWriteReg(CMD_ENC_SEQ_GOP_NUM, pEncInfo->gopSize);
	}
	else
	{
		VpuWriteReg(CMD_ENC_SEQ_GOP_NUM, 0);
	}

	//	Rate Control
	if (pEncInfo->rcEnable)
	{
		tmpData = (!pEncInfo->enableAutoSkip) << 31 | pEncInfo->initialDelay << 16 | pEncInfo->bitRate<<1 | 1;
		VpuWriteReg(CMD_ENC_SEQ_RC_PARA, tmpData);
	}
	else
	{
		VpuWriteReg(CMD_ENC_SEQ_RC_PARA, 0);
	}

	VpuWriteReg(CMD_ENC_SEQ_RC_BUF_SIZE, pEncInfo->vbvBufSize);
	VpuWriteReg(CMD_ENC_SEQ_INTRA_REFRESH, pEncInfo->intraRefresh);

	if( pEncInfo->rcIntraQp >= 0 )
	{
		tmpData = (1 << 5);
		VpuWriteReg(CMD_ENC_SEQ_INTRA_QP, pEncInfo->rcIntraQp);
	}
	else
	{
		tmpData = 0;
		VpuWriteReg(CMD_ENC_SEQ_INTRA_QP, (unsigned int)-1);
	}

	if (pEncInfo->codecStd == CODEC_STD_AVC)
		tmpData |= (pEncInfo->EncCodecParam.avcEncParam.audEnable<< 2);

	if(pEncInfo->userQpMax>=0)
	{
		tmpData |= (1<<6);
		VpuWriteReg(CMD_ENC_SEQ_RC_QP_MAX, pEncInfo->userQpMax);
	}
	else
	{
		VpuWriteReg(CMD_ENC_SEQ_RC_QP_MAX, 0);
	}

	if(pEncInfo->userGamma >= 0)
	{
		tmpData |= (1<<7);
		VpuWriteReg(CMD_ENC_SEQ_RC_GAMMA, pEncInfo->userGamma);
	}
	else
	{
		VpuWriteReg(CMD_ENC_SEQ_RC_GAMMA, 0);
	}

	VpuWriteReg(CMD_ENC_SEQ_OPTION, tmpData);

	VpuWriteReg(CMD_ENC_SEQ_RC_INTERVAL_MODE, (pEncInfo->mbInterval<<2) | pEncInfo->rcIntervalMode);
	VpuWriteReg(CMD_ENC_SEQ_INTRA_WEIGHT, pEncInfo->rcIntraCostWeigth);
	VpuWriteReg(CMD_ENC_SEQ_ME_OPTION, (pEncInfo->MEBlockMode << 3)|(pEncInfo->MEUseZeroPmv << 2)|pEncInfo->MESearchRange );

	VpuWriteReg(BIT_WR_PTR, pEncInfo->strmBufPhyAddr);
	VpuWriteReg(BIT_RD_PTR, pEncInfo->strmBufPhyAddr);

	tmpData = 0;
	tmpData |= (pEncInfo->bwbEnable<<12)|(pEncInfo->cbcrInterleave<<2);
	tmpData |= pEncInfo->frameEndian;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, tmpData);

	//	Ring Buffer Disable
	if( pEncInfo->ringBufferEnable == 0 )
	{
		tmpData  = (0x1<<6);	//	Line Buffer Interrupt Enable
		tmpData |= (0x1<<5);	//	The value of 1 means that bitstream buffer is reset at every picture encoding command.
		tmpData |= (0x1<<4);	//	Enables dynamic picture stream buffer allocation in encoder operations.
		tmpData |= VPU_STREAM_ENDIAN;
	}
	else	//	Ring Buffer Enabled
	{
		tmpData  = (0x1<<3);
		tmpData |= VPU_STREAM_ENDIAN;
	}
	VpuWriteReg(BIT_BIT_STREAM_CTRL, tmpData);

#if (DBG_REGISTER)
	{
		int reg;
		NX_DbgMsg( DBG_REGISTER, ("[SEQ_INIT_Reg]\n") );
		for (reg = 0x180 ; reg < 0x200 ; reg += 16)
		{
			NX_DbgMsg( DBG_REGISTER, ("[Addr = %3x]%x %x %x %x \n", reg, VpuReadReg(BIT_BASE + reg), VpuReadReg(BIT_BASE + reg + 4), VpuReadReg(BIT_BASE + reg + 8), VpuReadReg(BIT_BASE + reg + 12)) );
		}
	}
#endif

	VpuBitIssueCommand(pInst, SEQ_INIT);

	if( !VPU_WaitBitInterrupt(VPU_DEC_TIMEOUT) )
	{
		NX_ErrMsg(("VPU_EncSeqCommand() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
		return VPU_RET_ERR_TIMEOUT;
	}

	//	Get Encoder Frame Buffer Information
	if (VpuReadReg(RET_ENC_SEQ_ENC_SUCCESS) & (1<<31))
        return VPU_RET_ERR_MEM_ACCESS;

	if (VpuReadReg(RET_ENC_SEQ_ENC_SUCCESS) == 0)
	{
		return VPU_RET_ERR_INIT;
	}

	if (pInst->codecMode == MJPG_ENC)
		pEncInfo->minFrameBuffers = 0;
	else
		pEncInfo->minFrameBuffers = 2; // reconstructed frame + reference frame + subsample

	pEncInfo->strmWritePrt = VpuReadReg(BIT_WR_PTR);
	pEncInfo->strmEndFlag = VpuReadReg(BIT_BIT_STREAM_PARAM);

	NX_DbgMsg( INFO_MSG, ("VPU_EncSeqCommand() Success : Writer Ptr = 0x%08x, Stream End Flag = %d\n",
		pEncInfo->strmWritePrt, pEncInfo->strmEndFlag) );

	pInst->isInitialized = 1;
	return VPU_RET_OK;
}


static NX_VPU_RET VPU_EncSetFrameBufCommand(NX_VpuCodecInst *pInst)
{
	int i;
	unsigned char frameAddr[22][3][4];
	VpuEncInfo *pEncInfo = &pInst->codecInfo.encInfo;
	unsigned int frameBufStride = ((pEncInfo->srcWidth+15)&~15);
	unsigned int val=0;

	NX_DrvMemset( frameAddr, 0, sizeof(frameAddr) );

	//	Set Second AXI Memory (SRAM) Configuration
	if( VPU_RET_OK != ConfigEncSecAXI( pEncInfo->codecStd, &pEncInfo->secAxiInfo, pEncInfo->srcWidth, pEncInfo->srcHeight ) )
	{
		return VPU_RET_ERR_SRAM;
	}

	SetTiledMapType(VPU_LINEAR_FRAME_MAP, frameBufStride, pEncInfo->cbcrInterleaveRefFrame);

	if (pEncInfo->frameBufMapType) {
		if (pEncInfo->frameBufMapType == VPU_TILED_FRAME_MB_RASTER_MAP || pEncInfo->frameBufMapType == VPU_TILED_FIELD_MB_RASTER_MAP)
			val |= (pEncInfo->cbcrInterleaveRefFrame<<11)|(0x03<<9)|(IMG_FORMAT_420<<6);
		else
			val |= (pEncInfo->cbcrInterleaveRefFrame<<11)|(0x02<<9)|(IMG_FORMAT_420<<6);
	}
	val |= (pEncInfo->cbcrInterleaveRefFrame<<2); // Interleave bit position is modified
	val |= VPU_FRAME_ENDIAN ;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, val);

	// Let the decoder know the addresses of the frame buffers.
	for (i=0; i<pEncInfo->minFrameBuffers; i++)
	{
		//	Y
		frameAddr[i][0][0] = (pEncInfo->frameBuffer[i].luPhyAddr >> 24) & 0xFF;
		frameAddr[i][0][1] = (pEncInfo->frameBuffer[i].luPhyAddr >> 16) & 0xFF;
		frameAddr[i][0][2] = (pEncInfo->frameBuffer[i].luPhyAddr >>  8) & 0xFF;
		frameAddr[i][0][3] = (pEncInfo->frameBuffer[i].luPhyAddr >>  0) & 0xFF;
		//	Cb
		frameAddr[i][1][0] = (pEncInfo->frameBuffer[i].cbPhyAddr >> 24) & 0xFF;
		frameAddr[i][1][1] = (pEncInfo->frameBuffer[i].cbPhyAddr >> 16) & 0xFF;
		frameAddr[i][1][2] = (pEncInfo->frameBuffer[i].cbPhyAddr >>  8) & 0xFF;
		frameAddr[i][1][3] = (pEncInfo->frameBuffer[i].cbPhyAddr >>  0) & 0xFF;
		//	Cr
		frameAddr[i][2][0] = (pEncInfo->frameBuffer[i].crPhyAddr >> 24) & 0xFF;
		frameAddr[i][2][1] = (pEncInfo->frameBuffer[i].crPhyAddr >> 16) & 0xFF;
		frameAddr[i][2][2] = (pEncInfo->frameBuffer[i].crPhyAddr >>  8) & 0xFF;
		frameAddr[i][2][3] = (pEncInfo->frameBuffer[i].crPhyAddr >>  0) & 0xFF;
	}

	swap_endian((unsigned char*)frameAddr, sizeof(frameAddr));
	NX_DrvMemcpy((void *)pInst->paramVirAddr, frameAddr, sizeof(frameAddr));

	// Tell the codec how much frame buffers were allocated.
	VpuWriteReg(CMD_SET_FRAME_BUF_NUM, pEncInfo->minFrameBuffers);
	VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, frameBufStride);
	VpuWriteReg(CMD_SET_FRAME_AXI_BIT_ADDR, pEncInfo->secAxiInfo.bufBitUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_IPACDC_ADDR, pEncInfo->secAxiInfo.bufIpAcDcUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_DBKY_ADDR, pEncInfo->secAxiInfo.bufDbkYUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_DBKC_ADDR, pEncInfo->secAxiInfo.bufDbkCUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_OVL_ADDR, pEncInfo->secAxiInfo.bufOvlUse);
	VpuWriteReg(CMD_SET_FRAME_CACHE_CONFIG, pEncInfo->cacheConfig);

	// Set Sub-Sampling buffer for ME-Reference and DBK-Reconstruction
	// BPU will swap below two buffer internally every pic by pic
	VpuWriteReg(CMD_SET_FRAME_SUBSAMP_A, pEncInfo->subSampleAPhyAddr);
	VpuWriteReg(CMD_SET_FRAME_SUBSAMP_B, pEncInfo->subSampleBPhyAddr);

	if (pInst->codecMode == MP4_ENC) {
        // MPEG4 Encoder Data-Partitioned bitstream temporal buffer
        VpuWriteReg(CMD_SET_FRAME_DP_BUF_BASE, pEncInfo->usbSampleDPPhyAddr);
        VpuWriteReg(CMD_SET_FRAME_DP_BUF_SIZE, pEncInfo->usbSampleDPSize);
    }

#if (DBG_REGISTER)
	{
		int reg;
		NX_DbgMsg( DBG_REGISTER, ("[ENC_SET_FRM_BUF_Reg]\n") );
		for (reg = 0x180 ; reg < 0x200 ; reg += 16)
		{
			NX_DbgMsg( DBG_REGISTER, ("[Addr = %3x]%x %x %x %x \n", reg, VpuReadReg(BIT_BASE + reg), VpuReadReg(BIT_BASE + reg + 4), VpuReadReg(BIT_BASE + reg + 8), VpuReadReg(BIT_BASE + reg + 12)) );
		}
	}
#endif

	VpuBitIssueCommand(pInst, SET_FRAME_BUF);
	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		NX_ErrMsg(("VPU_EncSetFrameBuffer() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
		return VPU_RET_ERR_TIMEOUT;
	}

	if (VpuReadReg(RET_SET_FRAME_SUCCESS) & (1<<31))
	{
        return VPU_RET_ERR_MEM_ACCESS;
	}

	return VPU_RET_OK;
}


static NX_VPU_RET VPU_EncGetHeaderCommand(NX_VpuCodecInst *pInst, unsigned int headerType, unsigned char **ptr, int *size)
{
	VpuEncInfo *pEncInfo = &pInst->codecInfo.encInfo;
	int flag=0;
	unsigned int wdPtr, rdPtr;
	int headerSize;

	if( pEncInfo->ringBufferEnable == 0 )
	{
		VpuWriteReg(CMD_ENC_HEADER_BB_START, pEncInfo->strmBufPhyAddr);
		VpuWriteReg(CMD_ENC_HEADER_BB_SIZE, pEncInfo->strmBufSize/1024);
	}

	if( pInst->codecMode == AVC_ENC && headerType == SPS_RBSP )
	{
		EncAvcParam *avcParam = &pEncInfo->EncCodecParam.avcEncParam;
		int CropH = 0, CropV = 0;

		if( pEncInfo->encWidth & 0xF )
		{
			flag = 1;
			avcParam->cropRight = 16 - (pEncInfo->encWidth&0xF);
			CropH  = avcParam->cropLeft << 16;
			CropH |= avcParam->cropRight;
		}
		if( pEncInfo->encHeight & 0xF )
		{
			flag = 1;
			avcParam->cropBottom = 16 - (pEncInfo->encHeight&0xF);
			CropV  = avcParam->cropTop << 16;
			CropV |= avcParam->cropBottom;
		}

		VpuWriteReg( CMD_ENC_HEADER_FRAME_CROP_H, CropH );
		VpuWriteReg( CMD_ENC_HEADER_FRAME_CROP_V, CropV );
	}

	VpuWriteReg(CMD_ENC_HEADER_CODE, headerType | (flag << 3)); // 0: SPS, 1: PPS

	VpuWriteReg(BIT_RD_PTR, pEncInfo->strmBufPhyAddr);
	VpuWriteReg(BIT_WR_PTR, pEncInfo->strmBufPhyAddr);

#if (DBG_REGISTER)
	{
		int reg;
		NX_DbgMsg( DBG_REGISTER, ("[ENC_HEADER_Reg]\n") );
		for (reg = 0x180 ; reg < 0x200 ; reg += 16)
		{
			NX_DbgMsg( DBG_REGISTER, ("[Addr = %3x]%x %x %x %x \n", reg, VpuReadReg(BIT_BASE + reg), VpuReadReg(BIT_BASE + reg + 4), VpuReadReg(BIT_BASE + reg + 8), VpuReadReg(BIT_BASE + reg + 12)) );
		}
	}
#endif

	VpuBitIssueCommand(pInst, ENCODE_HEADER);
	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		return VPU_RET_ERR_TIMEOUT;
	}

	if( pEncInfo->ringBufferEnable == 0 )
	{
		rdPtr = pEncInfo->strmBufPhyAddr;
		wdPtr = VpuReadReg(BIT_WR_PTR);
		headerSize = wdPtr - rdPtr;
	}
	else
	{
		rdPtr = VpuReadReg(BIT_RD_PTR);
		wdPtr = VpuReadReg(BIT_WR_PTR);
		headerSize = wdPtr - rdPtr;
	}

	pEncInfo->strmWritePrt = wdPtr;	//	Bitstream Write Ptr
	pEncInfo->strmReadPrt = rdPtr;	//	Bitstream Read Ptr

	*ptr = (unsigned char *)pEncInfo->strmBufVirAddr;
	*size = headerSize;

	return VPU_RET_OK;
}


static NX_VPU_RET VPU_EncOneFrameCommand( NX_VpuCodecInst *pInst, VPU_ENC_RUN_FRAME_ARG *runArg )
{
	unsigned int readPtr, writePtr;
	int size, picFlag/*, frameIndex*/, val, reason;
	unsigned int sliceNumber, picEncResult, picType;
	VpuEncInfo *pEncInfo = &pInst->codecInfo.encInfo;

	//	Check Dynamic Change Parameter

	//
	VpuWriteReg(GDI_TILEDBUF_BASE, 0x00);				//	For Linear Frame Buffer Mode

	//	Mirror/Rotate Mode
	if ( pEncInfo->rotateAngle == 0 && pEncInfo->mirrorDirection == 0)
		VpuWriteReg(CMD_ENC_PIC_ROT_MODE, 0);
	else
		VpuWriteReg(CMD_ENC_PIC_ROT_MODE, (1 << 4) | (pEncInfo->mirrorDirection << 2) | (pEncInfo->rotateAngle) );

	VpuWriteReg(CMD_ENC_PIC_QS, runArg->quantParam);	//	If rate control is enabled, this register is ignored.( MPEG-4/H.263 : 1~31, AVC : 0 ~51 )

	if (runArg->skipPicture) {
		VpuWriteReg(CMD_ENC_PIC_OPTION, 1);
	}
	else
	{
		// Registering Source Frame Buffer information
		// Hide GDI IF under FW level
		VpuWriteReg(CMD_ENC_PIC_SRC_INDEX, 2);	//	0/1 : reconstruct & reference buffer
		VpuWriteReg(CMD_ENC_PIC_SRC_STRIDE, runArg->inImgBuffer.luStride);
		VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_Y, runArg->inImgBuffer.luPhyAddr);
		VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_CB, runArg->inImgBuffer.cbPhyAddr);
		VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_CR, runArg->inImgBuffer.crPhyAddr);
		VpuWriteReg(CMD_ENC_PIC_OPTION, (runArg->forceIPicture << 1 & 0x2));
	}

	if (pEncInfo->ringBufferEnable == 0) {
		VpuWriteReg(CMD_ENC_PIC_BB_START, pEncInfo->strmBufPhyAddr);
		VpuWriteReg(CMD_ENC_PIC_BB_SIZE, pEncInfo->strmBufSize/1024); // size in KB
		VpuWriteReg(BIT_RD_PTR, pEncInfo->strmBufPhyAddr);
	}

	val = 0;
	val = (
		(pEncInfo->secAxiInfo.useBitEnable &0x01)<<0 |
		(pEncInfo->secAxiInfo.useIpEnable  &0x01)<<1 |
		(pEncInfo->secAxiInfo.useDbkYEnable&0x01)<<2 |
		(pEncInfo->secAxiInfo.useDbkCEnable&0x01)<<3 |

		(pEncInfo->secAxiInfo.useOvlEnable &0x01)<<4 |
		(pEncInfo->secAxiInfo.useBtpEnable &0x01)<<5 |

		(pEncInfo->secAxiInfo.useBitEnable &0x01)<<8 |
		(pEncInfo->secAxiInfo.useIpEnable  &0x01)<<9 |
		(pEncInfo->secAxiInfo.useDbkYEnable&0x01)<<10|
		(pEncInfo->secAxiInfo.useDbkCEnable&0x01)<<11|

		(pEncInfo->secAxiInfo.useOvlEnable &0x01)<<12|
		(pEncInfo->secAxiInfo.useBtpEnable &0x01)<<13 );

	VpuWriteReg(BIT_AXI_SRAM_USE, val);
	//VpuWriteReg(CMD_ENC_PIC_SUB_FRAME_SYNC, 0);			//	Sub Frame Sync

	VpuWriteReg(BIT_WR_PTR, pEncInfo->strmBufPhyAddr);
	VpuWriteReg(BIT_RD_PTR, pEncInfo->strmBufPhyAddr);
	VpuWriteReg(BIT_BIT_STREAM_PARAM, pEncInfo->strmEndFlag);

	val = 0;
	val |= (pEncInfo->bwbEnable<<12)|(pEncInfo->cbcrInterleave<<2);
	val |= pEncInfo->frameEndian;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, val);

	if (pEncInfo->ringBufferEnable == 0) {
		val = 0;
		val |= (0x1<<6);
		val |= (0x1<<5);
		val |= (0x1<<4);
	}
	else
	{
		val |= (0x1<<3);
	}
	val |= VPU_STREAM_ENDIAN;
	VpuWriteReg(BIT_BIT_STREAM_CTRL, val);

#if (DBG_REGISTER)
	{
		int reg;
		NX_DbgMsg( DBG_REGISTER, ("[ENC_RUN_Reg]\n") );
		for (reg = 0x180 ; reg < 0x200 ; reg += 16)
		{
			NX_DbgMsg( DBG_REGISTER, ("[Addr = %3x]%x %x %x %x \n", reg, VpuReadReg(BIT_BASE + reg), VpuReadReg(BIT_BASE + reg + 4), VpuReadReg(BIT_BASE + reg + 8), VpuReadReg(BIT_BASE + reg + 12)) );
		}
	}
#endif

	VpuBitIssueCommand(pInst, PIC_RUN);

	do{
		reason = VPU_WaitBitInterrupt(VPU_ENC_TIMEOUT);

		if( reason == 0 )
		{
			return VPU_RET_ERR_TIMEOUT;
		}
		if( reason & (1<<VPU_INT_BIT_PIC_RUN) )
		{
			break;
		}
		else if( reason &  (1<<VPU_INT_BIT_BIT_BUF_FULL) )
		{
			return VPU_RET_ERR_STRM_FULL;
		}
	}while(1);

	picEncResult = VpuReadReg(RET_ENC_PIC_SUCCESS);
	if (picEncResult & (1<<31))
	{
		return VPU_RET_ERR_MEM_ACCESS;
	}
	picType = VpuReadReg(RET_ENC_PIC_TYPE);
	readPtr  = VpuReadReg(BIT_RD_PTR);
	writePtr = VpuReadReg(BIT_WR_PTR);

	size = writePtr - readPtr;			//	Caculate Output Buffer Size
	sliceNumber = VpuReadReg(RET_ENC_PIC_SLICE_NUM);

	picFlag = VpuReadReg(RET_ENC_PIC_FLAG);
	runArg->reconImgIdx = VpuReadReg(RET_ENC_PIC_FRAME_IDX);	//	we don't need recon frame idx

	//	VpuReadReg(BIT_WR_PTR);				//	we don't need write ptr ( ring buffer only ? )
	pEncInfo->strmEndFlag = VpuReadReg(BIT_BIT_STREAM_PARAM);

	runArg->frameType = ( (picType&0x1) == 0 ) ? 1 : 0;
	runArg->outStreamSize = size;
	runArg->outStreamAddr = (unsigned char*)pEncInfo->strmBufVirAddr;
	NX_DbgMsg( INFO_MSG, ("Encoded Size = %d, PicType = %d, picFlag = %d, sliceNumber = %d\n", size, picType, picFlag, sliceNumber) );

	return VPU_RET_OK;
}

static NX_VPU_RET VPU_EncChangeParameterCommand( NX_VpuCodecInst *pInst, VPU_ENC_CHG_PARA_ARG *chgArg )
{
	int ret;

	if (chgArg->chgFlg | VPU_BIT_CHG_GOP)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_RC_GOP, chgArg->gopSize);
	}
	if (chgArg->chgFlg | VPU_BIT_CHG_INTRAQP)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_RC_INTRA_QP, chgArg->intraQp);
	}
	if (chgArg->chgFlg | VPU_BIT_CHG_BITRATE)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_RC_BITRATE, chgArg->bitrate);
	}
	if (chgArg->chgFlg | VPU_BIT_CHG_FRAMERATE)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_RC_FRAME_RATE, (chgArg->frameRateNum) | ((chgArg->frameRateDen-1) << 16));
	}
	if (chgArg->chgFlg | VPU_BIT_CHG_INTRARF)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_INTRA_MB_NUM, chgArg->intraRefreshMbs);
	}
	if (chgArg->chgFlg | VPU_BIT_CHG_SLICEMOD)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_SLICE_MODE, (chgArg->sliceMode) | (chgArg->sliceSizeMode << 2) | (chgArg->sliceSizeNum << 3) );
	}
	if (chgArg->chgFlg | VPU_BIT_CHG_HECMODE)
	{
		VpuWriteReg(CMD_ENC_SEQ_PARA_HEC_MODE, chgArg->hecMode);
	}

	VpuWriteReg(CMD_ENC_SEQ_PARA_CHANGE_ENABLE, chgArg->chgFlg & 0x7F);

	VpuBitIssueCommand(pInst, RC_CHANGE_PARAMETER);
	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		return VPU_RET_ERR_TIMEOUT;
	}

	ret = VpuReadReg(RET_ENC_SEQ_PARA_CHANGE_SECCESS);
	if ( (ret & 1) == 0 )
		return VPU_RET_ERR_CHG_PARAM;
	if (ret >> 31)
		return VPU_RET_ERR_MEM_ACCESS;

	return VPU_RET_OK;
}
static NX_VPU_RET VPU_EncCloseCommand(NX_VpuCodecInst *pInst)
{
	FUNCIN();
	if (pInst->isInitialized)
	{
		VpuEncInfo *pEncInfo = &pInst->codecInfo.encInfo;
		VpuWriteReg(BIT_WR_PTR, pEncInfo->strmBufPhyAddr);
		VpuWriteReg(BIT_RD_PTR, pEncInfo->strmBufPhyAddr);

		VpuBitIssueCommand(pInst, SEQ_END);
		if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
		{
			VpuWriteReg(BIT_INT_CLEAR, 0x1);	//	For Signal Break Out
			atomic_set(&gVpuEventPresent, 0);	//	Clear Atomic
			NX_ErrMsg(("VPU_EncCloseCommand() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
			VPU_SWReset( SW_RESET_SAFETY );
			pInst->isInitialized = 0;
			return VPU_RET_ERR_TIMEOUT;
		}
		pInst->isInitialized = 0;
	}
	VpuWriteReg(BIT_INT_CLEAR, 0x1);	//	For Signal Break Out
	atomic_set(&gVpuEventPresent, 0);	//	Clear Atomic

	FUNCOUT();
	return VPU_RET_OK;
}
#if 0
RetCode VPU_EncClose(EncHandle handle)
{
	CodecInst * pCodecInst;
	EncInfo * pEncInfo;
	RetCode ret;

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	EnterLock(pCodecInst->coreIdx);

	if (pEncInfo->initialInfoObtained) {

		VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr, pEncInfo->streamWrPtr);
		VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr, pEncInfo->streamRdPtr);

		BitIssueCommand(pCodecInst->coreIdx, pCodecInst, SEQ_END);
		if (vdi_wait_vpu_busy(pCodecInst->coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {
			if (pCodecInst->loggingEnable)
				vdi_log(pCodecInst->coreIdx, SEQ_END, 2);
			LeaveLock(pCodecInst->coreIdx);
			return RETCODE_VPU_RESPONSE_TIMEOUT;
		}
		if (pCodecInst->loggingEnable)
			vdi_log(pCodecInst->coreIdx, SEQ_END, 0);
		pEncInfo->streamWrPtr = VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr);
	}

	if (pEncInfo->vbScratch.size)
		vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbScratch);
	if (pEncInfo->vbWork.size)
		vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbWork);
	if (pEncInfo->vbFrame.size) {
		if (pEncInfo->frameAllocExt == 0) {
			vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbFrame);
		}
	}

	if (pEncInfo->vbPPU.size) {
		if (pEncInfo->ppuAllocExt == 0) {
			vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbPPU);
		}
	}


	if (pEncInfo->vbSubSampFrame.size)
		vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbSubSampFrame);

	LeaveLock(pCodecInst->coreIdx);

	FreeCodecInstance(pCodecInst);

	return RETCODE_SUCCESS;
}
#endif
//----------------------------------------------------------------------------







//----------------------------------------------------------------------------
//
//				Decoder APIs
//

//	Decoder Open
NX_VPU_RET	NX_VpuDecOpen( VPU_OPEN_ARG *openArg, void *drvHandle, NX_VPU_INST_HANDLE *handle )
{
	int i, val;
	VpuDecInfo *pDecInfo;
	NX_VPU_INST_HANDLE hInst=0;
	*handle = 0;

	FUNCIN();

	if( !gstIsInitialized ){
		return VPU_RET_ERR_INIT;
	}

	for( i=0; i<NX_MAX_VPU_INSTANCE ; i++ )
	{
		if( !gstVpuInstance[i].inUse )
		{
			hInst = &gstVpuInstance[i];
			break;
		}
	}

	if( NX_MAX_VPU_INSTANCE==i || !hInst )
	{
		return VPU_RET_ERR_INST;
	}

	if (openArg->codecStd == CODEC_STD_MPEG4) {
		hInst->codecMode = MP4_DEC;
		hInst->auxMode = MP4_AUX_MPEG4;
	}
	else if (openArg->codecStd == CODEC_STD_AVC) {
		hInst->codecMode = AVC_DEC;
		hInst->auxMode = AVC_AUX_AVC;
	}
	else if (openArg->codecStd == CODEC_STD_VC1) {
		hInst->codecMode = VC1_DEC;
	}
	else if (openArg->codecStd == CODEC_STD_MPEG2) {
		hInst->codecMode = MP2_DEC;
		hInst->auxMode = 0;
	}
	else if (openArg->codecStd == CODEC_STD_H263) {
		hInst->codecMode = MP4_DEC;
		hInst->auxMode = 0;
	}
	else if (openArg->codecStd == CODEC_STD_DIV3) {
		hInst->codecMode = DV3_DEC;
		hInst->auxMode = MP4_AUX_DIVX3;
	}
	else if (openArg->codecStd == CODEC_STD_RV) {
		hInst->codecMode = RV_DEC;
	}
	else if (openArg->codecStd == CODEC_STD_AVS) {
		hInst->codecMode = AVS_DEC;
	}
	//else if (codecStd == CODEC_STD_MJPG) {
	//	pCodecInst->codecMode = MJPG_DEC;
	//}
	else if (openArg->codecStd == CODEC_STD_THO) {
		hInst->codecMode = VPX_DEC;
		hInst->auxMode = VPX_AUX_THO;
	}
	else if (openArg->codecStd == CODEC_STD_VP3) {
		hInst->codecMode = VPX_DEC;
		hInst->auxMode = VPX_AUX_THO;
	}
	else if (openArg->codecStd == CODEC_STD_VP8) {
		hInst->codecMode = VPX_DEC;
		hInst->auxMode = VPX_AUX_VP8;
	}
	else
	{
		NX_ErrMsg(("NX_VpuDecOpen() failed. Cannot support codec standard(%d)\n", openArg->codecStd));
		return VPU_RET_ERR_PARAM;
	}

	//	Set Base Information
	hInst->inUse = 1;
	hInst->instIndex = i;
	hInst->devHandle = drvHandle;

	hInst->instBufPhyAddr = openArg->instanceBuf.phyAddr;
	hInst->instBufVirAddr = openArg->instanceBuf.virAddr;
	hInst->instBufSize    = openArg->instanceBuf.size;
	pDecInfo = &hInst->codecInfo.decInfo;

	//	Clrear Instnace Information
	NX_DrvMemset( &hInst->codecInfo, 0, sizeof(hInst->codecInfo) );
	pDecInfo->codecStd = openArg->codecStd;
	pDecInfo->mp4Class = openArg->mp4Class;
	pDecInfo->strmBufPhyAddr = openArg->streamBuf.phyAddr;
	pDecInfo->strmBufVirAddr = openArg->streamBuf.virAddr;
	pDecInfo->strmBufSize    = openArg->streamBuf.size;
	NX_DrvMemset( (unsigned char*)pDecInfo->strmBufVirAddr, 0, pDecInfo->strmBufSize );

	//	Set Other Parameters
	pDecInfo->frameDelay = -1;
	pDecInfo->userDataEnable = 0;
	pDecInfo->enableReordering = VPU_REORDER_ENABLE;
	pDecInfo->vc1BframeDisplayValid = 0;
	pDecInfo->avcErrorConcealMode = 0;
	pDecInfo->enableMp4Deblock = 0;

	//pDecInfo->bitstreamMode = BS_MODE_ROLLBACK;
	//pDecInfo->bitstreamMode = BS_MODE_PIC_END;
	//pDecInfo->bitstreamMode = BS_MODE_INTERRUPT;
	if( pDecInfo->codecStd == CODEC_STD_AVC )
	{
		pDecInfo->bitstreamMode = BS_MODE_PIC_END;
		//pDecInfo->bitstreamMode = BS_MODE_ROLLBACK;
	}
	else if( pDecInfo->codecStd == CODEC_STD_H263 || pDecInfo->codecStd == CODEC_STD_MPEG4 )
	{
		pDecInfo->bitstreamMode = BS_MODE_PIC_END;
	}
	//else if( pDecInfo->codecStd == CODEC_STD_VP8 )
	//{
	//	pDecInfo->bitstreamMode = BS_MODE_PIC_END;
	//}
	else
	{
		pDecInfo->bitstreamMode = BS_MODE_ROLLBACK;
	}

	pDecInfo->cbcrInterleave = openArg->chromaInterleave;
	pDecInfo->streamEndflag = 0;	//	Frame Unit Operation
	pDecInfo->bwbEnable = VPU_ENABLE_BWB;
	pDecInfo->seqInitEscape = 0;
	pDecInfo->streamEndian = VPU_STREAM_ENDIAN;
	pDecInfo->cacheConfig = MaverickCache2Config( 1, pDecInfo->cbcrInterleave, 0, 0, 3, 0, 15 );

	NX_DrvMemset( (void*)hInst->paramVirAddr, 0, PARA_BUF_SIZE);

	VpuWriteReg(BIT_WR_PTR, pDecInfo->strmBufPhyAddr);
	VpuWriteReg(BIT_RD_PTR, pDecInfo->strmBufPhyAddr);

	pDecInfo->readPos  = pDecInfo->strmBufPhyAddr;
	pDecInfo->writePos = pDecInfo->strmBufPhyAddr;

	val = VpuReadReg(BIT_BIT_STREAM_PARAM);
	val &= ~(1 << 2);
	VpuWriteReg(BIT_BIT_STREAM_PARAM, val);	// clear stream end flag at start
	pDecInfo->streamEndflag = val;

	*handle = hInst;

	NX_DbgMsg( INFO_MSG, ("===================================\n") );
	NX_DbgMsg( INFO_MSG, (" VPU Open Informations:\n") );
	NX_DbgMsg( INFO_MSG, ("  Instance Index : %d\n", hInst->instIndex        ) );
	NX_DbgMsg( INFO_MSG, ("  BitStream Mode : %d\n", pDecInfo->bitstreamMode ) );
	NX_DbgMsg( INFO_MSG, ("  Codec Standard : %d\n", hInst->codecMode        ) );
	NX_DbgMsg( INFO_MSG, ("  Codec AUX Mode : %d\n", hInst->auxMode          ) );
	NX_DbgMsg( INFO_MSG, ("===================================\n") );

	FUNCOUT();
	return VPU_RET_OK;
}

static int FillBuffer(NX_VPU_INST_HANDLE handle, unsigned char *stream, int size)
{
	unsigned int vWriteOffset, vReadOffset;	//	Virtual Read/Write Position
	int bufSize;
	VpuDecInfo *pDecInfo = &handle->codecInfo.decInfo;

	//	EOS
	if( size==0 )
		return 0;
	if( !stream || size <0 )
	{
		return -1;
	}

	vWriteOffset = pDecInfo->writePos - pDecInfo->strmBufPhyAddr;
	vReadOffset  = pDecInfo->readPos  - pDecInfo->strmBufPhyAddr;
	bufSize      = pDecInfo->strmBufSize;

	// printk("%s, StreamBuffer(Addr=x0%08x, size=%d), InBuffer(Addr=x0%08x, size=%d) vWriteOffset = %d, vReadOffset = %d\n",
	// 	__func__, pDecInfo->strmBufVirAddr, pDecInfo->strmBufSize, stream, size, vWriteOffset, vReadOffset );

	if( bufSize < vWriteOffset || bufSize < vReadOffset )
	{
		printk("%s, StreamBuffer(Addr=x0%08x, size=%d), InBuffer(Addr=x0%08x, size=%d) vWriteOffset = %d, vReadOffset = %d\n",
			__func__, pDecInfo->strmBufVirAddr, pDecInfo->strmBufSize, (int)stream, size, vWriteOffset, vReadOffset );
		return -1;
	}

	if( (bufSize - vWriteOffset) > size )
	{
		//	Just Memory Copy
		//NX_DrvMemcpy( (unsigned char*)(pDecInfo->strmBufVirAddr + vWriteOffset), stream, size );
		//printk("vWriteOffset=%d, vReadOffset = %d\n", vWriteOffset, vReadOffset );
		if( copy_from_user( (unsigned char*)(pDecInfo->strmBufVirAddr + vWriteOffset), stream, size ) )
			return -1;
		vWriteOffset += size;
	}
	else
	{
		//	Memory Copy
		int remain = bufSize - vWriteOffset;
		//NX_DrvMemcpy( (unsigned char*)(pDecInfo->strmBufVirAddr + vWriteOffset), stream, remain );
		//NX_DrvMemcpy( (unsigned char*)(pDecInfo->strmBufVirAddr), stream+remain, size-remain    );

		if( copy_from_user( (unsigned char*)(pDecInfo->strmBufVirAddr + vWriteOffset), stream, remain ) )
			return -1;
		if( copy_from_user( (unsigned char*)(pDecInfo->strmBufVirAddr), stream+remain, size-remain    ) )
			return -1;
		vWriteOffset = size-remain;
	}
	pDecInfo->writePos = vWriteOffset + pDecInfo->strmBufPhyAddr;
	return 0;
}

NX_VPU_RET	NX_VpuDecSetSeqInfo( NX_VPU_INST_HANDLE handle, VPU_DEC_SEQ_INIT_ARG *seqArg )
{
	VpuDecInfo *pInfo = &handle->codecInfo.decInfo;
	NX_VPU_RET ret;

	FUNCIN();

	//	FillBuffer
	if( 0 > FillBuffer( handle, seqArg->seqData, seqArg->seqDataSize ) )
	{
		printk("FillBuffer Error!!!\n");
		return VPU_RET_ERROR;
	}

#if 0
	if( pInfo->bitstreamMode != BS_MODE_PIC_END )
	{
		int streamSize;
		if( pInfo->writePos > pInfo->readPos )
		{
			streamSize = pInfo->writePos - pInfo->readPos;
		}
		else
		{
			streamSize = pInfo->strmBufSize - ( pInfo->readPos - pInfo->writePos );
		}

		if( streamSize < VPU_GBU_SIZE*2 )
		{
			return VPU_RET_NEED_STREAM;
		}
	}
#endif

	ret = VPU_DecSeqInitCommand( handle, seqArg );
	if( VPU_RET_OK != ret )
		return ret;

	NX_DbgMsg( DBG_ES_ADDR, ("[Init]Start = %x, [In]Rd = %x, Wr = %x [Out]Rd = %x, Wr = %x\n", pInfo->strmBufPhyAddr, pInfo->readPos, pInfo->writePos, VpuReadReg(BIT_RD_PTR), VpuReadReg(BIT_WR_PTR)) );

	FUNCOUT();
	return VPU_DecSeqComplete( handle, seqArg);
}

NX_VPU_RET	NX_VpuDecRegFrameBuf( NX_VPU_INST_HANDLE handle, VPU_DEC_REG_FRAME_ARG *frmArg)
{
	return VPU_DecRegisterFrameBufCommand( handle, frmArg );
}

NX_VPU_RET	NX_VpuDecRunFrame( NX_VPU_INST_HANDLE handle, VPU_DEC_DEC_FRAME_ARG *decArg)
{
	VpuDecInfo *pInfo = &handle->codecInfo.decInfo;
	NX_VPU_RET ret;

	UNUSED( pInfo );

	//	Fill Data
	if( 0 > FillBuffer( handle, decArg->strmData, decArg->strmDataSize ) )
	{
		NX_ErrMsg(("FillBuffer Failed.\n"));
		return VPU_RET_ERROR;
	}

#if 0
	if( pInfo->bitstreamMode != BS_MODE_PIC_END )
	{
		int streamSize;
		if( pInfo->writePos > pInfo->readPos )
		{
			streamSize = pInfo->writePos - pInfo->readPos;
		}
		else
		{
			streamSize = pInfo->strmBufSize - ( pInfo->readPos - pInfo->writePos );
		}

		if( streamSize < VPU_GBU_SIZE*2 )
		{
			decArg->indexFrameDecoded = -1;
			decArg->indexFrameDisplay = -1;
			return VPU_RET_NEED_STREAM;
		}
	}
#endif
	ret = VPU_DecStartOneFrameCommand(handle, decArg);
	if( ret != VPU_RET_OK )
		return ret;

	NX_DbgMsg( DBG_ES_ADDR, ("Start = %x, [In]Rd = %x, Wr = %x [Out]Rd = %x, Wr = %x\n", pInfo->strmBufPhyAddr, pInfo->readPos, pInfo->writePos, VpuReadReg(BIT_RD_PTR), VpuReadReg(BIT_WR_PTR)) );

	return VPU_DecGetOutputInfo( handle, decArg );
}

NX_VPU_RET	NX_VpuDecFlush( NX_VPU_INST_HANDLE handle )
{
	unsigned int val;
	VpuDecInfo *pDecInfo = &handle->codecInfo.decInfo;

	val = pDecInfo->frameDisplayFlag;
	val &= ~pDecInfo->clearDisplayIndexes;
	VpuWriteReg( BIT_FRM_DIS_FLG, val);
	pDecInfo->clearDisplayIndexes = 0;
	pDecInfo->writePos = pDecInfo->readPos;
	VpuBitIssueCommand(handle, DEC_BUF_FLUSH);

	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		return VPU_RET_ERR_TIMEOUT;
	}
	pDecInfo->frameDisplayFlag = VpuReadReg( BIT_FRM_DIS_FLG );
	pDecInfo->frameDisplayFlag = 0;
	pDecInfo->streamEndflag &= ~(1<<2);		//	Clear End of Stream
	pDecInfo->readPos = pDecInfo->strmBufPhyAddr;
	pDecInfo->writePos= pDecInfo->strmBufPhyAddr;
	NX_DrvMemset((unsigned char*)pDecInfo->strmBufVirAddr, 0, pDecInfo->strmBufSize);
	return VPU_RET_OK;
}

NX_VPU_RET	NX_VpuDecClrDspFlag( NX_VPU_INST_HANDLE handle, VPU_DEC_CLR_DSP_FLAG_ARG *pArg )
{
	VpuDecInfo *pInfo = &handle->codecInfo.decInfo;
	pInfo->clearDisplayIndexes |= (1<<pArg->indexFrameDisplay);
	return VPU_RET_OK;
}

NX_VPU_RET	NX_VpuDecClose( NX_VPU_INST_HANDLE handle )
{
	return VPU_DecCloseCommand(handle);
}


//----------------------------------------------------------------------------
//						Decoder Specific Static Functions

static NX_VPU_RET VPU_DecSeqComplete( NX_VpuCodecInst *pInst, VPU_DEC_SEQ_INIT_ARG *pArg )
{
	unsigned int val, val2;
	int errReason;
	VpuDecInfo *pInfo = &pInst->codecInfo.decInfo;

	if (pInfo->bitstreamMode == BS_MODE_INTERRUPT && pInfo->seqInitEscape) {
		pInfo->streamEndflag &= ~(3<<3);
		VpuWriteReg(BIT_BIT_STREAM_PARAM, pInfo->streamEndflag);
		pInfo->seqInitEscape = 0;
	}

	pInfo->frameDisplayFlag = VpuReadReg(BIT_FRM_DIS_FLG);
	pInfo->readPos = VpuReadReg(BIT_RD_PTR);
	pInfo->streamEndflag = VpuReadReg(BIT_BIT_STREAM_PARAM);

	errReason = 0;
	val = VpuReadReg(RET_DEC_SEQ_SUCCESS);
	if (val & (1<<31)) {
		return VPU_RET_ERR_MEM_ACCESS;
	}

	if ( pInfo->bitstreamMode == BS_MODE_PIC_END || pInfo->bitstreamMode == BS_MODE_ROLLBACK )
	{
		if (val & (1<<4))
		{
			errReason = VpuReadReg(RET_DEC_SEQ_SEQ_ERR_REASON);
			NX_ErrMsg(("Error Reason = 0x%08x\n", errReason) );
			return VPU_RET_ERROR;
		}
	}

	if (val == 0) {
		errReason = VpuReadReg(RET_DEC_SEQ_SEQ_ERR_REASON);
		NX_ErrMsg(("Error Reason = 0x%08x\n", errReason) );
		return VPU_RET_ERROR;
	}

	val = VpuReadReg(RET_DEC_SEQ_SRC_SIZE);
	pArg->outWidth  = ( (val >> 16) & 0xffff );
	pArg->outHeight = ( val & 0xffff );

	pArg->frameRateNum = VpuReadReg(RET_DEC_SEQ_FRATE_NR);
	pArg->frameRateDen = VpuReadReg(RET_DEC_SEQ_FRATE_DR);
	//printk("frameRateNum = %d(%x), frameRateDen = %d(%x)\n", pArg->frameRateNum, pArg->frameRateNum, pArg->frameRateDen, pArg->frameRateDen);

	if (pInst->codecMode == AVC_DEC && pArg->frameRateDen > 0)
		pArg->frameRateDen  *= 2;

	if (pInst->codecMode  == MP4_DEC)
	{
		val = VpuReadReg(RET_DEC_SEQ_INFO);
		pArg->mp4ShortHeader = (val >> 2) & 1;
		pArg->mp4PartitionEnable = val & 1;
		pArg->mp4ReversibleVlcEnable =
			pArg->mp4PartitionEnable ?
			((val >> 1) & 1) : 0;
		pArg->h263AnnexJEnable = (val >> 3) & 1;
	}
	else if (pInst->codecMode == VPX_DEC && pInst->auxMode == VPX_AUX_VP8)
	{
		// h_scale[31:30] v_scale[29:28] pic_width[27:14] pic_height[13:0]
		val = VpuReadReg(RET_DEC_SEQ_VP8_SCALE_INFO);
		pArg->vp8HScaleFactor = (val >> 30) & 0x03;
		pArg->vp8VScaleFactor = (val >> 28) & 0x03;
		pArg->vp8ScaleWidth = (val >> 14) & 0x3FFF;
		pArg->vp8ScaleHeight = (val >> 0) & 0x3FFF;
	}

	pArg->minFrameBufCnt = VpuReadReg(RET_DEC_SEQ_FRAME_NEED);
	pArg->frameBufDelay = VpuReadReg(RET_DEC_SEQ_FRAME_DELAY);

	if (pInst->codecMode == AVC_DEC || pInst->codecMode == MP2_DEC)
	{
		val  = VpuReadReg(RET_DEC_SEQ_CROP_LEFT_RIGHT);
		val2 = VpuReadReg(RET_DEC_SEQ_CROP_TOP_BOTTOM);
		if( val == 0 && val2 == 0 )
		{
			pArg->cropLeft = 0;
			pArg->cropRight = pArg->outWidth;
			pArg->cropTop = 0;
			pArg->cropBottom = pArg->outHeight;
		}
		else
		{
			pArg->cropLeft = ( (val>>16) & 0xFFFF );
			pArg->cropRight = pArg->outWidth - ( ( val & 0xFFFF ) );
			pArg->cropTop = ( (val2>>16) & 0xFFFF );
			pArg->cropBottom = pArg->outHeight - ( ( val2 & 0xFFFF ) );
		}

		val = (pArg->outWidth * pArg->outHeight * 3 / 2) / 1024;
		pArg->numSliceSize = val / 4;
		pArg->worstSliceSize = val / 2;
	}
	else
	{
		pArg->cropLeft = 0;
		pArg->cropRight = pArg->outWidth;
		pArg->cropTop = 0;
		pArg->cropBottom = pArg->outHeight;
	}

	val = VpuReadReg(RET_DEC_SEQ_HEADER_REPORT);
	pArg->profile                =	(val >> 0) & 0xFF;
	pArg->level                  =	(val >> 8) & 0xFF;
	pArg->interlace              =	(val >> 16) & 0x01;
	pArg->direct8x8Flag          =	(val >> 17) & 0x01;
	pArg->vc1Psf                 =	(val >> 18) & 0x01;
	pArg->constraint_set_flag[0] = 	(val >> 19) & 0x01;
	pArg->constraint_set_flag[1] = 	(val >> 20) & 0x01;
	pArg->constraint_set_flag[2] = 	(val >> 21) & 0x01;
	pArg->constraint_set_flag[3] = 	(val >> 22) & 0x01;
	pArg->avcIsExtSAR            =  (val >> 25) & 0x01;
	pArg->maxNumRefFrmFlag       =  (val >> 31) & 0x01;

	pArg->aspectRateInfo = VpuReadReg(RET_DEC_SEQ_ASPECT);

	val = VpuReadReg(RET_DEC_SEQ_BIT_RATE);
	pArg->bitrate = val;

	if (pInst->codecMode == AVC_DEC) {
		val = VpuReadReg(RET_DEC_SEQ_VUI_INFO);
		pArg->avcVuiInfo.fixedFrameRateFlag    = val &1;
		pArg->avcVuiInfo.timingInfoPresent     = (val>>1) & 0x01;
		pArg->avcVuiInfo.chromaLocBotField     = (val>>2) & 0x07;
		pArg->avcVuiInfo.chromaLocTopField     = (val>>5) & 0x07;
		pArg->avcVuiInfo.chromaLocInfoPresent  = (val>>8) & 0x01;
		pArg->avcVuiInfo.colorPrimaries        = (val>>16) & 0xff;
		pArg->avcVuiInfo.colorDescPresent      = (val>>24) & 0x01;
		pArg->avcVuiInfo.isExtSAR              = (val>>25) & 0x01;
		pArg->avcVuiInfo.vidFullRange          = (val>>26) & 0x01;
		pArg->avcVuiInfo.vidFormat             = (val>>27) & 0x07;
		pArg->avcVuiInfo.vidSigTypePresent     = (val>>30) & 0x01;
		pArg->avcVuiInfo.vuiParamPresent       = (val>>31) & 0x01;

		val = VpuReadReg(RET_DEC_SEQ_VUI_PIC_STRUCT);
		pArg->avcVuiInfo.vuiPicStructPresent = (val & 0x1);
		pArg->avcVuiInfo.vuiPicStruct = (val>>1);
	}

	if (pInst->codecMode == MP2_DEC) {
		// seq_ext info
		val = VpuReadReg(RET_DEC_SEQ_EXT_INFO);
		pArg->mp2LowDelay = val & 1;
		pArg->mp2DispVerSize = (val>>1) & 0x3fff;
		pArg->mp2DispHorSize = (val>>15) & 0x3fff;
#if 0	//	User Data
		if (pInfo->userDataEnable)
		{
			int userDataNum;
			int userDataSize;
			unsigned char tempBuf[8] = {0,};

			// user data
			NX_DrvMemcpy( tempBuf, (void*)pInfo->userDataBufVirAddr, 8 );

			val = ((tempBuf[0]<<24) & 0xFF000000) |
				((tempBuf[1]<<16) & 0x00FF0000) |
				((tempBuf[2]<< 8) & 0x0000FF00) |
				((tempBuf[3]<< 0) & 0x000000FF);

			userDataNum = (val >> 16) & 0xFFFF;
			userDataSize = (val >> 0) & 0xFFFF;
			if (userDataNum == 0)
				userDataSize = 0;

			pArg->userDataNum = userDataNum;
			pArg->userDataSize = userDataSize;

			val = ((tempBuf[4]<<24) & 0xFF000000) |
				((tempBuf[5]<<16) & 0x00FF0000) |
				((tempBuf[6]<< 8) & 0x0000FF00) |
				((tempBuf[7]<< 0) & 0x000000FF);

			if (userDataNum == 0)
				pArg->userDataBufFull = 0;
			else
				pArg->userDataBufFull = (val >> 16) & 0xFFFF;
			NX_DbgMsg( DBG_USERDATA, ("TempBuf[8] = 0x%02x%02x%02x%02x%02x%02x%02x%02x\n", tempBuf[0],tempBuf[1],tempBuf[2],tempBuf[3],tempBuf[4],tempBuf[5],tempBuf[6],tempBuf[7]) );
			NX_DbgMsg( DBG_USERDATA, ("userDataNum = %d, userDataSize = %d, pArg->userDataBufFull=%d\n", userDataNum, userDataSize, pArg->userDataBufFull) );
		}
#endif
	}

	pInfo->writePos = VpuReadReg( BIT_WR_PTR );
	pInfo->readPos  = VpuReadReg( BIT_RD_PTR );

	pInfo->width = pArg->outWidth;
	pInfo->height = pArg->outHeight;

	pInst->isInitialized = 1;

	return VPU_RET_OK;
}

static NX_VPU_RET VPU_DecSeqInitCommand(NX_VpuCodecInst *pInst, VPU_DEC_SEQ_INIT_ARG *pArg)
{
	unsigned int val, reason;
	VpuDecInfo *pInfo = &pInst->codecInfo.decInfo;

	if( pArg->disableOutReorder )
	{
		printk("======================================= Disable Out Reordering!!!\n");
		pInfo->lowDelayInfo.lowDelayEn = 1;
		pInfo->lowDelayInfo.numRows = 0;
	}

	if( pInfo->needMoreFrame )
	{
		pInfo->needMoreFrame = 0;
		VpuWriteReg(BIT_WR_PTR, pInfo->writePos);
		printk("Need More Buffer!!!!!\n");
		goto WAIT_INTERRUPT;
	}

	pInfo->enableMp4Deblock = pArg->enablePostFilter;

	VpuWriteReg(CMD_DEC_SEQ_BB_START, pInfo->strmBufPhyAddr);
	VpuWriteReg(CMD_DEC_SEQ_BB_SIZE, pInfo->strmBufSize / 1024); // size in KBytes

	//printk("hdr Addr = %x, Size = %x \n", pInfo->strmBufPhyAddr, pInfo->strmBufSize / 1024);
	//printk("CurPC = %x, WR = %x, RD =  %x, Idx = %d \n", VpuReadReg(BIT_CUR_PC), VpuReadReg(BIT_WR_PTR), VpuReadReg(BIT_RD_PTR), pInst->instIndex );

#if (DBG_REGISTER)
	{
		int i;
		VpuWriteReg(BIT_BIT_STREAM_PARAM, 0 );

		// Clear Stream end flag
		i = (int)VpuReadReg( BIT_BIT_STREAM_PARAM );
		if (i & (1 << ( pInst->instIndex + 2))) {
		i -= 1 << ( pInst->instIndex + 2);
		}
		VpuWriteReg( BIT_BIT_STREAM_PARAM, i);
	}
#endif

	if(pArg->enableUserData) {
		pInfo->userDataBufPhyAddr = pArg->userDataBuffer.phyAddr;
		pInfo->userDataBufVirAddr = pArg->userDataBuffer.virAddr;
		pInfo->userDataBufSize = pArg->userDataBuffer.size;
		pInfo->userDataEnable = 1;
		pInfo->userDataReportMode = 1;

		val  = 0;
		val |= (pInfo->userDataReportMode << 10);
//		val |= (pInfo->userDataEnable << 5);
		VpuWriteReg(CMD_DEC_SEQ_USER_DATA_OPTION, val);
		VpuWriteReg(CMD_DEC_SEQ_USER_DATA_BASE_ADDR, pInfo->userDataBufPhyAddr);
		VpuWriteReg(CMD_DEC_SEQ_USER_DATA_BUF_SIZE,  pInfo->userDataBufSize);
	}
	else {
		VpuWriteReg(CMD_DEC_SEQ_USER_DATA_OPTION, 0);
		VpuWriteReg(CMD_DEC_SEQ_USER_DATA_BASE_ADDR, 0);
		VpuWriteReg(CMD_DEC_SEQ_USER_DATA_BUF_SIZE, 0);
	}
	val  = 0;

	if (!pInfo->lowDelayInfo.lowDelayEn) {
		val |= (pInfo->enableReordering<<1) & 0x2;
	}

	val |= (pInfo->enableMp4Deblock & 0x1);
	val |= (pInfo->avcErrorConcealMode << 2);	//Enable error conceal on missing reference in h.264/AVC

	VpuWriteReg(CMD_DEC_SEQ_OPTION, val);

	switch(pInst->codecMode) {
	case VC1_DEC:
//		VpuWriteReg(CMD_DEC_SEQ_VC1_STREAM_FMT, 1);	//	Ray Park RCV V2
//		VpuWriteReg(CMD_DEC_SEQ_VC1_STREAM_FMT, 2);	//	Ray Park RCV V2
		VpuWriteReg(CMD_DEC_SEQ_VC1_STREAM_FMT, (0 << 3) & 0x08);
		break;
	case MP4_DEC:
		VpuWriteReg(CMD_DEC_SEQ_MP4_ASP_CLASS, (VPU_GMC_PROCESS_METHOD<<3)|pInfo->mp4Class);
		break;
	case AVC_DEC:
		VpuWriteReg(CMD_DEC_SEQ_X264_MV_EN, VPU_AVC_X264_SUPPORT);
		break;
	}

	if( pInst->codecMode == AVC_DEC )
		VpuWriteReg(CMD_DEC_SEQ_SPP_CHUNK_SIZE, VPU_GBU_SIZE);

	VpuWriteReg(BIT_WR_PTR, pInfo->writePos);
	VpuWriteReg(BIT_RD_PTR, pInfo->readPos);

	//	Clear Stream Flag
	pInfo->streamEndflag &= ~(1<<2);			//	Clear End of Stream
	pInfo->streamEndflag &= ~(3<<3);			//	Clear Bitstream Mode
	if (pInfo->bitstreamMode == BS_MODE_ROLLBACK)  //rollback mode
		pInfo->streamEndflag |= (1<<3);
	else if (pInfo->bitstreamMode == BS_MODE_PIC_END)
		pInfo->streamEndflag |= (2<<3);
	else {	// Interrupt Mode
		if (pInfo->seqInitEscape) {
			pInfo->streamEndflag |= (2<<3);
		}
	}
	VpuWriteReg(BIT_BIT_STREAM_PARAM, pInfo->streamEndflag);

	VpuWriteReg(BIT_BIT_STREAM_CTRL, pInfo->streamEndian);

	val = 0;
	val |= ( pInfo->bwbEnable<<12);
	val |= (pInfo->cbcrInterleave<<2); // Interleave bit position is modified
	VpuWriteReg(BIT_FRAME_MEM_CTRL, val);

	//VpuWriteReg(H_MBY_SYNC_OUT, 0x00);
	VpuWriteReg(BIT_FRM_DIS_FLG, 0);

#if (DBG_REGISTER)
	{
		int reg;
		NX_DbgMsg( DBG_REGISTER, ("[DEC_SEQ_INIT]\n") );
		NX_DbgMsg( DBG_REGISTER, ("[Strm_CTRL : 0x10C]%x \n", VpuReadReg(BIT_BIT_STREAM_CTRL) ));
		for (reg = 0x180 ; reg < 0x200 ; reg += 16)
		{
			NX_DbgMsg( DBG_REGISTER, ("[Addr = %3x]%x %x %x %x \n", reg, VpuReadReg(BIT_BASE + reg), VpuReadReg(BIT_BASE + reg + 4), VpuReadReg(BIT_BASE + reg + 8), VpuReadReg(BIT_BASE + reg + 12)) );
		}
	}
#endif

	VpuBitIssueCommand(pInst, SEQ_INIT);

WAIT_INTERRUPT:
	if( !(reason=VPU_WaitBitInterrupt(VPU_DEC_TIMEOUT)) )
	{
		NX_ErrMsg(("VPU_DecSeqInitCommand() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
		printk("WritePos = 0x%.8x, ReadPos = 0x%.8x\n", VpuReadReg(BIT_WR_PTR), VpuReadReg(BIT_RD_PTR));
		return VPU_RET_ERR_TIMEOUT;
	}

	//printk("WritePos = 0x%.8x, ReadPos = 0x%.8x\n", VpuReadReg(BIT_WR_PTR), VpuReadReg(BIT_RD_PTR));

	if( reason & (1<<VPU_INT_BIT_SEQ_INIT) )
	{
		return VPU_RET_OK;
	}
	else if( reason & (1<<VPU_INT_BIT_BIT_BUF_EMPTY) )
	{
#if 0	// for Testing
		if( pInfo->bitstreamMode == BS_MODE_INTERRUPT )
		{
			return VPU_RET_OK;
		}
#endif
		pInfo->needMoreFrame = 1;
		return VPU_RET_NEED_STREAM;
	}
	else
	{
		return VPU_RET_ERROR;
	}
}




static int swap_endian(unsigned char *data, int len)
{
	unsigned long *p;
	unsigned long v1, v2, v3;
	int i;
	int swap = 0;
	p = (unsigned long *)data;

	for (i=0; i<len/4; i+=2)
	{
		v1 = p[i];
		v2  = ( v1 >> 24) & 0xFF;
		v2 |= ((v1 >> 16) & 0xFF) <<  8;
		v2 |= ((v1 >>  8) & 0xFF) << 16;
		v2 |= ((v1 >>  0) & 0xFF) << 24;
		v3 =  v2;
		v1  = p[i+1];
		v2  = ( v1 >> 24) & 0xFF;
		v2 |= ((v1 >> 16) & 0xFF) <<  8;
		v2 |= ((v1 >>  8) & 0xFF) << 16;
		v2 |= ((v1 >>  0) & 0xFF) << 24;
		p[i]   =  v2;
		p[i+1] = v3;
	}

	return swap;

}


static NX_VPU_RET VPU_DecRegisterFrameBufCommand( NX_VpuCodecInst *pInst, VPU_DEC_REG_FRAME_ARG *pArg )
{
	VpuDecInfo *pInfo = &pInst->codecInfo.decInfo;
	unsigned char frameAddr[MAX_REG_FRAME][3][4];
	unsigned char colMvAddr[MAX_REG_FRAME][4];
	int bufferStride = pArg->frameBuffer[0].luStride;
	unsigned int val, mvStartAddr;
	int i;

	FUNCIN();

	NX_DrvMemset( frameAddr, 0, sizeof(frameAddr) );
	NX_DrvMemset( colMvAddr, 0, sizeof(colMvAddr) );

	SetTiledMapType(VPU_LINEAR_FRAME_MAP, bufferStride, pInfo->cbcrInterleave);

	for (i=0; i<pArg->numFrameBuffer; i++) {
		frameAddr[i][0][0] = (pArg->frameBuffer[i].luPhyAddr >> 24) & 0xFF;
        frameAddr[i][0][1] = (pArg->frameBuffer[i].luPhyAddr >> 16) & 0xFF;
        frameAddr[i][0][2] = (pArg->frameBuffer[i].luPhyAddr >>  8) & 0xFF;
        frameAddr[i][0][3] = (pArg->frameBuffer[i].luPhyAddr >>  0) & 0xFF;

        frameAddr[i][1][0] = (pArg->frameBuffer[i].cbPhyAddr >> 24) & 0xFF;
        frameAddr[i][1][1] = (pArg->frameBuffer[i].cbPhyAddr >> 16) & 0xFF;
        frameAddr[i][1][2] = (pArg->frameBuffer[i].cbPhyAddr >>  8) & 0xFF;
        frameAddr[i][1][3] = (pArg->frameBuffer[i].cbPhyAddr >>  0) & 0xFF;

        frameAddr[i][2][0] = (pArg->frameBuffer[i].crPhyAddr >> 24) & 0xFF;
        frameAddr[i][2][1] = (pArg->frameBuffer[i].crPhyAddr >> 16) & 0xFF;
        frameAddr[i][2][2] = (pArg->frameBuffer[i].crPhyAddr >>  8) & 0xFF;
        frameAddr[i][2][3] = (pArg->frameBuffer[i].crPhyAddr >>  0) & 0xFF;
	}
	swap_endian((unsigned char*)frameAddr, sizeof(frameAddr));
	NX_DrvMemcpy((void *)pInst->paramVirAddr, frameAddr, sizeof(frameAddr));
	NX_DrvMemcpy( pInfo->frameBuffer, pArg->frameBuffer, sizeof(pArg->frameBuffer) );

#if 0
	for( i=0 ; i<pArg->numFrameBuffer; i++ )
	{
		printk("#%02d : Phy(Lu 0x%.8x Cb 0x%.8x Cr 0x%.8x), Vir(Lu 0x%.8x Cb 0x%.8x Cr 0x%.8x)\n", i,
			pInfo->frameBuffer[i].luPhyAddr,pInfo->frameBuffer[i].cbPhyAddr,pInfo->frameBuffer[i].crPhyAddr,
			pInfo->frameBuffer[i].luVirAddr,pInfo->frameBuffer[i].cbVirAddr,pInfo->frameBuffer[i].crVirAddr);
	}
#endif

	mvStartAddr = pArg->colMvBuffer.phyAddr;

	// MV allocation and registe
	if(	pInst->codecMode == AVC_DEC ||		//	H.264
		pInst->codecMode == VC1_DEC ||		//	WMV9
		pInst->codecMode == MP4_DEC ||		//	MPEG4
		pInst->codecMode == RV_DEC  ||		//	Read Video
		pInst->codecMode == AVS_DEC )		//	AVS
	{
		//unsigned long   bufMvCol;
		int size_mvcolbuf;
		mvStartAddr = pArg->colMvBuffer.phyAddr;

		if (pInst->codecMode == AVC_DEC || pInst->codecMode == VC1_DEC ||
			pInst->codecMode == MP4_DEC || pInst->codecMode == RV_DEC || pInst->codecMode == AVS_DEC)
		{
			size_mvcolbuf =  ((pInfo->width+31)&~31)*((pInfo->height+31)&~31);
			size_mvcolbuf = (size_mvcolbuf*3)/2;
			size_mvcolbuf = (size_mvcolbuf+4) / 5;
			size_mvcolbuf = ((size_mvcolbuf+7)/ 8) * 8;

			if (pInst->codecMode == AVC_DEC)
			{
				for (i=0; i<pArg->numFrameBuffer; i++)
				{
					colMvAddr[i][0] = (mvStartAddr >> 24) & 0xFF;
					colMvAddr[i][1] = (mvStartAddr >> 16) & 0xFF;
					colMvAddr[i][2] = (mvStartAddr >>  8) & 0xFF;
					colMvAddr[i][3] = (mvStartAddr >>  0) & 0xFF;
					mvStartAddr += size_mvcolbuf;
				}
			}
			else
			{
				colMvAddr[0][0] = (mvStartAddr >> 24) & 0xFF;
				colMvAddr[0][1] = (mvStartAddr >> 16) & 0xFF;
				colMvAddr[0][2] = (mvStartAddr >>  8) & 0xFF;
				colMvAddr[0][3] = (mvStartAddr >>  0) & 0xFF;
			}
		}
		swap_endian((unsigned char*)colMvAddr, sizeof(colMvAddr));
		NX_DrvMemcpy((void *)(pInst->paramVirAddr+384), colMvAddr, sizeof(colMvAddr));
	}

	if (!ConfigDecSecAXI(pInfo->codecStd, &pInfo->secAxiInfo, pInfo->width, pInfo->height)) {
		NX_ErrMsg(("ConfigDecSecAXI() failed !!! Width = %d, Heigth = %d\n", pInfo->width, pInfo->height));
		return VPU_RET_ERR_SRAM;
	}

	// Tell the decoder how much frame buffers were allocated.
	VpuWriteReg(CMD_SET_FRAME_BUF_NUM, pArg->numFrameBuffer);
	VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, bufferStride);
	VpuWriteReg(CMD_SET_FRAME_AXI_BIT_ADDR, pInfo->secAxiInfo.bufBitUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_IPACDC_ADDR, pInfo->secAxiInfo.bufIpAcDcUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_DBKY_ADDR, pInfo->secAxiInfo.bufDbkYUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_DBKC_ADDR, pInfo->secAxiInfo.bufDbkCUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_OVL_ADDR, pInfo->secAxiInfo.bufOvlUse);
	VpuWriteReg(CMD_SET_FRAME_AXI_BTP_ADDR, pInfo->secAxiInfo.bufBtpUse);
	VpuWriteReg(CMD_SET_FRAME_DELAY, pInfo->frameDelay);

	// Maverick Cache Configuration
	VpuWriteReg(CMD_SET_FRAME_CACHE_CONFIG, pInfo->cacheConfig);

    if (pInst->codecMode == VPX_DEC) {
		VpuWriteReg(CMD_SET_FRAME_MB_BUF_BASE, pArg->pvbSliceBuffer.phyAddr);
    }

	if( pInst->codecMode == AVC_DEC ) {
		VpuWriteReg( CMD_SET_FRAME_SLICE_BB_START, pArg->sliceBuffer.phyAddr );
		VpuWriteReg( CMD_SET_FRAME_SLICE_BB_SIZE, pArg->sliceBuffer.size/1024 );
	}

	val = 0;
	val |= (VPU_ENABLE_BWB<<12);
	val |= (pInfo->cbcrInterleave<<2); // Interleave bit position is modified
	val |= VPU_FRAME_ENDIAN;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, val);
	VpuWriteReg(CMD_SET_FRAME_MAX_DEC_SIZE, 0);  // set 1FE0_78_44 for 1920x1088

	VpuBitIssueCommand(pInst, SET_FRAME_BUF);

#if (DBG_REGISTER)
	{
		int reg;
		NX_DbgMsg( DBG_REGISTER, ("[DEC_SET_FRM_BUF_Reg]\n") );
		for (reg = 0x180 ; reg < 0x200 ; reg += 16)
		{
			NX_DbgMsg( DBG_REGISTER, ("[Addr = %3x]%x %x %x %x \n", reg, VpuReadReg(BIT_BASE + reg), VpuReadReg(BIT_BASE + reg + 4), VpuReadReg(BIT_BASE + reg + 8), VpuReadReg(BIT_BASE + reg + 12)) );
		}
	}
#endif

	if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
	{
		NX_ErrMsg(("Error VPU_DecRegisterFrameBufCommand failed!!!\n"));
		return VPU_RET_ERR_INIT;
	}

	if (VpuReadReg(RET_SET_FRAME_SUCCESS) & (1<<31))
	{
        return VPU_RET_ERR_MEM_ACCESS;
	}

	FUNCOUT();
	return VPU_RET_OK;
}

static NX_VPU_RET VPU_DecGetOutputInfo(NX_VpuCodecInst *pInst, VPU_DEC_DEC_FRAME_ARG *pArg)
{
	VpuDecInfo *pInfo = &pInst->codecInfo.decInfo;
	unsigned int val, val2;
	VPU_RECT rectInfo;

	val = VpuReadReg(RET_DEC_PIC_SUCCESS);
	if (val & (1<<31))
	{
		pArg->errReason  = VpuReadReg(GDI_WPROT_ERR_RSN);
		pArg->errAddress = VpuReadReg(GDI_WPROT_ERR_ADR);
		return VPU_RET_ERR_MEM_ACCESS;
	}

	if( pInst->codecMode == AVC_DEC ) {
		pArg->notSufficientPsBuffer = (val >> 3) & 0x1;
		pArg->notSufficientSliceBuffer = (val >> 2) & 0x1;
	}

	pArg->indexFrameDecoded	= VpuReadReg(RET_DEC_PIC_DECODED_IDX);
	pArg->indexFrameDisplay	= VpuReadReg(RET_DEC_PIC_DISPLAY_IDX);

	val = VpuReadReg(RET_DEC_PIC_SIZE); // decoding picture size
	pArg->outWidth  = (val>>16) & 0xFFFF;
	pArg->outHeight = (val) & 0xFFFF;

	//if (pArg->indexFrameDecoded >= 0 && pArg->indexFrameDecoded < MAX_REG_FRAME)
	{
		if( (pInst->codecMode == VPX_DEC) && (pInst->auxMode == VPX_AUX_VP8) )
		{
			// VP8 specific header information
			// h_scale[31:30] v_scale[29:28] pic_width[27:14] pic_height[13:0]
			val = VpuReadReg(RET_DEC_PIC_VP8_SCALE_INFO);
			pArg->vp8ScaleInfo.hScaleFactor = (val >> 30) & 0x03;
			pArg->vp8ScaleInfo.vScaleFactor = (val >> 28) & 0x03;
			pArg->vp8ScaleInfo.picWidth = (val >> 14) & 0x3FFF;
			pArg->vp8ScaleInfo.picHeight = (val >> 0) & 0x3FFF;
			// ref_idx_gold[31:24], ref_idx_altr[23:16], ref_idx_last[15: 8],
			// version_number[3:1], show_frame[0]
			val = VpuReadReg(RET_DEC_PIC_VP8_PIC_REPORT);
			pArg->vp8PicInfo.refIdxGold = (val >> 24) & 0x0FF;
			pArg->vp8PicInfo.refIdxAltr = (val >> 16) & 0x0FF;
			pArg->vp8PicInfo.refIdxLast = (val >> 8) & 0x0FF;
			pArg->vp8PicInfo.versionNumber = (val >> 1) & 0x07;
			pArg->vp8PicInfo.showFrame = (val >> 0) & 0x01;
		}

		//default value
		rectInfo.left   = 0;
		rectInfo.right  = pArg->outWidth;
		rectInfo.top    = 0;
		rectInfo.bottom = pArg->outHeight;

		if (pInst->codecMode == AVC_DEC || pInst->codecMode == MP2_DEC)
		{
			val = VpuReadReg(RET_DEC_PIC_CROP_LEFT_RIGHT);				// frame crop information(left, right)
			val2 = VpuReadReg(RET_DEC_PIC_CROP_TOP_BOTTOM);			// frame crop information(top, bottom)

			if (val == (unsigned int)-1 || val == 0)
			{
				rectInfo.left   = 0;
				rectInfo.right  = pArg->outWidth;
			}
			else
			{
				rectInfo.left    = ((val>>16) & 0xFFFF);
				rectInfo.right   = pArg->outWidth - (val&0xFFFF);
			}
			if (val2 == (unsigned int)-1 || val2 == 0)
			{
				rectInfo.top    = 0;
				rectInfo.bottom = pArg->outHeight;
			}
			else
			{
				rectInfo.top     = ((val2>>16) & 0xFFFF);
				rectInfo.bottom	= pArg->outHeight - (val2&0xFFFF);
			}
		}
		pArg->outRect.left   = rectInfo.left  ;
		pArg->outRect.top    = rectInfo.top   ;
		pArg->outRect.right  = rectInfo.right ;
		pArg->outRect.bottom = rectInfo.bottom;
	}
	//else
	//{
	//	pArg->outRect.left   = 0;
	//	pArg->outRect.top    = 0;
	//	pArg->outRect.right  = pInfo->width;
	//	pArg->outRect.bottom = pInfo->height;
	//}

	val = VpuReadReg(RET_DEC_PIC_TYPE);
	pArg->isInterace = (val >> 18) & 0x1;

	if(pInst->codecMode == MP2_DEC)
	{
		pArg->progressiveFrame  = (val >> 23) & 0x0003;
		pArg->isInterace = (pArg->progressiveFrame == 0) ? (1) : (0);
		pArg->picStructure  = (val >> 19) & 0x0003;
	}

	if (pArg->isInterace)
	{
		pArg->topFieldFirst     = (val >> 21) & 0x0001;	// TopFieldFirst[21]
		pArg->picTypeFirst      = (val & 0x38) >> 3;	// pic_type of 1st field
		pArg->picType           = val & 0x7;			// pic_type of 2nd field
		pArg->npf               = (val >> 15) & 1;
	}
	else {
		pArg->topFieldFirst     = 0;
		pArg->picTypeFirst   = 6;	// no meaning
		pArg->picType = val & 0x7;
	}

	if( pInst->codecMode == AVC_DEC )
	{
		if (val & 0x40) { // 6th bit
			if (pArg->isInterace)
			{
				pArg->picTypeFirst = 6;		//	IDR
				pArg->picType = 6;
			}
			else
				pArg->picType = 6;			//	IDR
		}
	}

#if 0
	pArg->picStructure  = (val >> 19) & 0x0003;	// MbAffFlag[17], FieldPicFlag[16]
	pArg->repeatFirstField  = (val >> 22) & 0x0001;

	if( pInst->codecMode == AVC_DEC)
	{
		info->decFrameInfo     = (val >> 15) & 0x0001;
		info->picStrPresent     = (val >> 27) & 0x0001;
		info->picTimingStruct   = (val >> 28) & 0x000f;
		//update picture type when IDR frame
		if (val & 0x40) { // 6th bit
			if (info->interlacedFrame)
				info->picTypeFirst = PIC_TYPE_IDR;
			else
				info->picType = PIC_TYPE_IDR;

		}
		info->avcNpfFieldInfo  = (val >> 16) & 0x0003;
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_HRD_INFO);
		info->avcHrdInfo.cpbMinus1 = val>>2;
		info->avcHrdInfo.vclHrdParamFlag = (val>>1)&1;
		info->avcHrdInfo.nalHrdParamFlag = val&1;

		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_VUI_INFO);
		info->avcVuiInfo.fixedFrameRateFlag    = val &1;
		info->avcVuiInfo.timingInfoPresent     = (val>>1) & 0x01;
		info->avcVuiInfo.chromaLocBotField     = (val>>2) & 0x07;
		info->avcVuiInfo.chromaLocTopField     = (val>>5) & 0x07;
		info->avcVuiInfo.chromaLocInfoPresent  = (val>>8) & 0x01;
		info->avcVuiInfo.colorPrimaries        = (val>>16) & 0xff;
		info->avcVuiInfo.colorDescPresent      = (val>>24) & 0x01;
		info->avcVuiInfo.isExtSAR              = (val>>25) & 0x01;
		info->avcVuiInfo.vidFullRange          = (val>>26) & 0x01;
		info->avcVuiInfo.vidFormat             = (val>>27) & 0x07;
		info->avcVuiInfo.vidSigTypePresent     = (val>>30) & 0x01;
		info->avcVuiInfo.vuiParamPresent       = (val>>31) & 0x01;
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_VUI_PIC_STRUCT);
		info->avcVuiInfo.vuiPicStructPresent = (val & 0x1);
		info->avcVuiInfo.vuiPicStruct = (val>>1);
	}
	if( pCodecInst->codecMode == MP2_DEC)
	{
		info->fieldSequence       = (val >> 25) & 0x0007;
		info->frameDct            = (val >> 28) & 0x0001;
		info->progressiveSequence = (val >> 29) & 0x0001;
	}
#endif

	pArg->fRateNumerator    = VpuReadReg(RET_DEC_PIC_FRATE_NR); //Frame rate, Aspect ratio can be changed frame by frame.
	pArg->fRateDenominator  = VpuReadReg(RET_DEC_PIC_FRATE_DR);
	if (pInst->codecMode == AVC_DEC && pArg->fRateDenominator > 0)
		pArg->fRateDenominator  *= 2;
	if (pInst->codecMode == MP4_DEC)
	{
		pArg->mp4ModuloTimeBase = VpuReadReg(RET_DEC_PIC_MODULO_TIME_BASE);
		pArg->mp4TimeIncrement  = VpuReadReg(RET_DEC_PIC_VOP_TIME_INCREMENT);
	}

	if( pInst->codecMode == VPX_DEC )
		pArg->aspectRateInfo = 0;
	else
		pArg->aspectRateInfo = VpuReadReg(RET_DEC_PIC_ASPECT);

#if 0	// MPEG2 User Data
	// User Data
	if (pInfo->userDataEnable) {
		int userDataNum;
		int userDataSize;
		unsigned char tempBuf[8] = {0,};

		NX_DrvMemcpy( tempBuf, (void*)pInfo->userDataBufVirAddr, 8 );

		val = ((tempBuf[0]<<24) & 0xFF000000) |
			  ((tempBuf[1]<<16) & 0x00FF0000) |
			  ((tempBuf[2]<< 8) & 0x0000FF00) |
			  ((tempBuf[3]<< 0) & 0x000000FF);

		userDataNum = (val >> 16) & 0xFFFF;
		userDataSize = (val >> 0) & 0xFFFF;
		if (userDataNum == 0)
			userDataSize = 0;

		pArg->userDataNum = userDataNum;
		pArg->userDataSize = userDataSize;

		val =	((tempBuf[4]<<24) & 0xFF000000) |
			((tempBuf[5]<<16) & 0x00FF0000) |
			((tempBuf[6]<< 8) & 0x0000FF00) |
			((tempBuf[7]<< 0) & 0x000000FF);

		if (userDataNum == 0)
			pArg->userDataBufFull = 0;
		else
			pArg->userDataBufFull = (val >> 16) & 0xFFFF;

		pArg->activeFormat = VpuReadReg(RET_DEC_PIC_ATSC_USER_DATA_INFO)&0xf;
		NX_DbgMsg( DBG_USERDATA, ("userDataNum = %d, userDataSize = %d, activeFormat = %d\n", userDataNum, userDataSize, pArg->activeFormat) );
	}
#endif

	pArg->numOfErrMBs		= VpuReadReg(RET_DEC_PIC_ERR_MB);
	val                     = VpuReadReg(RET_DEC_PIC_SUCCESS);
	pArg->isSuccess	= val;
	pArg->sequenceChanged = ((val>>20) & 0x1);

	if (pArg->indexFrameDisplay>=0 && pArg->indexFrameDisplay < pInfo->numFrameBuffer){
		pArg->outFrameBuffer = pInfo->frameBuffer[pArg->indexFrameDisplay];
	}

	if (pInst->codecMode == VC1_DEC && pArg->indexFrameDisplay != -3) {
		if (pInfo->vc1BframeDisplayValid == 0) {
			if (pArg->picType == 2) {
				pArg->indexFrameDisplay = -3;
			} else {
				pInfo->vc1BframeDisplayValid = 1;
			}
		}
	}

#if 0
	if (pCodecInst->codecMode == AVC_DEC && pCodecInst->codecModeAux == AVC_AUX_MVC)
	{
		int val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_MVC_REPORT);
		info->mvcPicInfo.viewIdxDisplay = (val>>0) & 1;
		info->mvcPicInfo.viewIdxDecoded = (val>>1) & 1;
	}

	if (pCodecInst->codecMode == AVC_DEC)
	{
		int val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_AVC_FPA_SEI0);

		if (val < 0)
		{
			info->avcFpaSei.exist = 0;
		}
		else
		{
			info->avcFpaSei.exist = 1;
			info->avcFpaSei.framePackingArrangementId = val;
			val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_AVC_FPA_SEI1);
			info->avcFpaSei.contentInterpretationType               = val&0x3F; // [5:0]
			info->avcFpaSei.framePackingArrangementType             = (val >> 6)&0x7F; // [12:6]
			info->avcFpaSei.framePackingArrangementExtensionFlag    = (val >> 13)&0x01; // [13]
			info->avcFpaSei.frame1SelfContainedFlag                 = (val >> 14)&0x01; // [14]
			info->avcFpaSei.frame0SelfContainedFlag                 = (val >> 15)&0x01; // [15]
			info->avcFpaSei.currentFrameIsFrame0Flag                = (val >> 16)&0x01; // [16]
			info->avcFpaSei.fieldViewsFlag                          = (val >> 17)&0x01; // [17]
			info->avcFpaSei.frame0FlippedFlag                       = (val >> 18)&0x01; // [18]
			info->avcFpaSei.spatialFlippingFlag                     = (val >> 19)&0x01; // [19]
			info->avcFpaSei.quincunxSamplingFlag                    = (val >> 20)&0x01; // [20]
			info->avcFpaSei.framePackingArrangementCancelFlag       = (val >> 21)&0x01; // [21]
			val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_AVC_FPA_SEI2);
			info->avcFpaSei.framePackingArrangementRepetitionPeriod = val&0x7FFF;       // [14:0]
			info->avcFpaSei.frame1GridPositionY                     = (val >> 16)&0x0F; // [19:16]
			info->avcFpaSei.frame1GridPositionX                     = (val >> 20)&0x0F; // [23:20]
			info->avcFpaSei.frame0GridPositionY                     = (val >> 24)&0x0F; // [27:24]
			info->avcFpaSei.frame0GridPositionX                     = (val >> 28)&0x0F; // [31:28]
		}

		info->avcPocTop = VpuReadReg(RET_DEC_PIC_POC_TOP);
		info->avcPocBot = VpuReadReg(RET_DEC_PIC_POC_BOT);

        if (info->interlacedFrame)
        {
            if (info->avcPocTop > info->avcPocBot)
			{
                info->avcPocPic = info->avcPocBot;
            }
			else
			{
                info->avcPocPic = info->avcPocTop;
            }
		}
		else
		{
            info->avcPocPic = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_POC);
		}
	}

#endif

	if (pInfo->codecStd == CODEC_STD_VC1)
	{
		pArg->multiRes = (VpuReadReg(RET_DEC_PIC_POST) >> 1) & 3;
	}

	pInfo->readPos          = VpuReadReg( BIT_RD_PTR      );
	pInfo->frameDisplayFlag = VpuReadReg( BIT_FRM_DIS_FLG );

	/*
	pInfo->frameEndPos = pInfo->streamRdPtr;

	if (pInfo->frameEndPos < pInfo->frameStartPos) {
		pInfo->consumedByte = pInfo->frameEndPos + pInfo->streamBufSize - pInfo->frameStartPos;
	}
	else {
		pInfo->consumedByte = pInfo->frameEndPos - pInfo->frameStartPos;
	}
	*/
	pInfo->bytePosFrameStart = VpuReadReg(BIT_BYTE_POS_FRAME_START);
	pInfo->bytePosFrameEnd   = VpuReadReg(BIT_BYTE_POS_FRAME_END);

	pArg->strmReadPos  = pInfo->readPos  - pInfo->strmBufPhyAddr;
	pArg->strmWritePos = pInfo->writePos - pInfo->strmBufPhyAddr;
	//printk("pArg->picType = %d displayIdx = %d, decodeIdx = %d, readPos = 0x%.8x, writePos = 0x%.8x\n",	pArg->picType, pArg->indexFrameDisplay, pArg->indexFrameDecoded, pInfo->readPos, pInfo->writePos );
	return VPU_RET_OK;
}

static NX_VPU_RET VPU_DecStartOneFrameCommand(NX_VpuCodecInst *pInst, VPU_DEC_DEC_FRAME_ARG *pArg)
{
	VpuDecInfo *pInfo = &pInst->codecInfo.decInfo;
	unsigned int val, reason=0;

	if( pInfo->needMoreFrame )
	{
		pInfo->needMoreFrame = 0;
		VpuWriteReg(BIT_WR_PTR, pInfo->writePos);
//		printk("Need More Buffer!!!!!\n");
		goto WAIT_INTERRUPT;
	}

	VpuWriteReg(RET_DEC_PIC_CROP_LEFT_RIGHT, 0);				// frame crop information(left, right)
	VpuWriteReg(RET_DEC_PIC_CROP_TOP_BOTTOM, 0);				// frame crop information(top, bottom)

	VpuWriteReg(GDI_TILEDBUF_BASE, 0);

	if ( (pInfo->enableMp4Deblock & 2) && ((pInfo->codecStd == CODEC_STD_MPEG4) || (pInfo->codecStd == CODEC_STD_MPEG2) || (pInfo->codecStd == CODEC_STD_H263) || (pInfo->codecStd == CODEC_STD_DIV3)) )
	{
		VpuWriteReg(CMD_DEC_PIC_ROT_MODE, 1 << 5);
		//VpuWriteReg(CMD_DEC_PIC_ROT_INDEX, 0);
		//VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_Y, 0);
		//VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CB, 0);
		//VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CR, 0);
	}
	else
	{
		VpuWriteReg(CMD_DEC_PIC_ROT_MODE, 0);
	}

	if(pInfo->userDataEnable) {
		VpuWriteReg(CMD_DEC_PIC_USER_DATA_BASE_ADDR, pInfo->userDataBufPhyAddr);
		VpuWriteReg(CMD_DEC_PIC_USER_DATA_BUF_SIZE, pInfo->userDataBufSize);
		NX_DbgMsg( DBG_USERDATA, (" User Data : Buffer(0x%08x), Size(0x%08x)\n", pInfo->userDataBufPhyAddr, pInfo->userDataBufSize) );
	}
	else {
		VpuWriteReg(CMD_DEC_PIC_USER_DATA_BASE_ADDR, 0);
		VpuWriteReg(CMD_DEC_PIC_USER_DATA_BUF_SIZE, 0);
	}

	val = 0;
	if (pArg->iFrameSearchEnable != 0) // if iframeSearch is Enable, other bit is ignore;
	{
		val |= (pInfo->userDataReportMode		<<10 );
		if (pInst->codecMode == AVC_DEC)
		{
			if (pArg->iFrameSearchEnable==1)
				val |= (1<< 11) | (1<<2);
			else if (pArg->iFrameSearchEnable==2)
				val |= (1<<2);
		}
		else
			val |= (( pArg->iFrameSearchEnable &0x1)   << 2 );
	}
	else
	{
		val |= (pInfo->userDataReportMode	<<10 );
		if (!pArg->skipFrameMode)
			val |= (pInfo->userDataEnable	<< 5 );
		val |= (pArg->skipFrameMode			<< 3 );
	}

	if (pInst->codecMode == AVC_DEC && pInfo->lowDelayInfo.lowDelayEn)
		val |= (pInfo->lowDelayInfo.lowDelayEn <<18 );
	//if (pInst->codecMode == MP2_DEC)
	//	val |= ((pArg->DecStdParam.mp2PicFlush&1)<<15);
	//if (pInst->codecMode == RV_DEC)
	//	val |= ((param->DecStdParam.rvDbkMode&0x0f)<<16);

	//val |= ((1<<8)|(1<<0));
	VpuWriteReg(CMD_DEC_PIC_OPTION, val);

 	if (pInfo->lowDelayInfo.lowDelayEn)
 		VpuWriteReg(CMD_DEC_PIC_NUM_ROWS, pInfo->lowDelayInfo.numRows);
    else
 		VpuWriteReg(CMD_DEC_PIC_NUM_ROWS, 0);

	val = 0;
	val = (
		(pInfo->secAxiInfo.useBitEnable&0x01)<<0 |
		(pInfo->secAxiInfo.useIpEnable&0x01)<<1 |
		(pInfo->secAxiInfo.useDbkYEnable&0x01)<<2 |
		(pInfo->secAxiInfo.useDbkCEnable&0x01)<<3 |
		(pInfo->secAxiInfo.useOvlEnable&0x01)<<4 |
		(pInfo->secAxiInfo.useBtpEnable&0x01)<<5 |
		(pInfo->secAxiInfo.useBitEnable&0x01)<<8 |
		(pInfo->secAxiInfo.useIpEnable&0x01)<<9 |
		(pInfo->secAxiInfo.useDbkYEnable&0x01)<<10 |
		(pInfo->secAxiInfo.useDbkCEnable&0x01)<<11 |
		(pInfo->secAxiInfo.useOvlEnable&0x01)<<12 |
		(pInfo->secAxiInfo.useBtpEnable&0x01)<<13 );

	VpuWriteReg(BIT_AXI_SRAM_USE, val);

	VpuWriteReg(BIT_WR_PTR, pInfo->writePos);
	VpuWriteReg(BIT_RD_PTR, pInfo->readPos );

	val = pInfo->frameDisplayFlag;
	val &= ~pInfo->clearDisplayIndexes;
	VpuWriteReg(BIT_FRM_DIS_FLG, val);
	pInfo->clearDisplayIndexes = 0;

	if( pArg->eos )
	//if ( (pArg->strmDataSize == 0) || (pArg->strmData == NULL) )
		pInfo->streamEndflag |= 1<<2;
	else
		pInfo->streamEndflag &= ~(1<<2);

	pInfo->streamEndflag &= ~(3<<3);
	if (pInfo->bitstreamMode == BS_MODE_ROLLBACK)  //rollback mode
		pInfo->streamEndflag |= (1<<3);
	else if (pInfo->bitstreamMode == BS_MODE_PIC_END)
		pInfo->streamEndflag |= (2<<3);

	VpuWriteReg(BIT_BIT_STREAM_PARAM, pInfo->streamEndflag);

	val = 0;
	val |= (pInfo->bwbEnable<<12);
	val |= ((pInfo->cbcrInterleave)<<2); // Interleave bit position is modified
//	val |= pInfo->frameEndian;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, val);

	val = pInfo->streamEndian;
	VpuWriteReg(BIT_BIT_STREAM_CTRL, val);

	VpuBitIssueCommand(pInst, PIC_RUN);

WAIT_INTERRUPT:
	if( !(reason=VPU_WaitBitInterrupt(VPU_DEC_TIMEOUT)) )
	{
		NX_ErrMsg(("VPU_DecStartOneFrameCommand() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
		return VPU_RET_ERR_TIMEOUT;
	}

	if( reason & (1<<VPU_INT_BIT_PIC_RUN) )
	{
		//NX_DbgMsg(1, ("VPU_INT_BIT_PIC_RUN !!!(reason=0x%08x)\n", reason) );
		return VPU_RET_OK;
	}
	else if( reason &  (1<<VPU_INT_BIT_BIT_BUF_EMPTY) )
	{
		pInfo->needMoreFrame = 1;
		return VPU_RET_NEED_STREAM;
	}
	else
	{
		return VPU_RET_ERROR;
	}
}

static NX_VPU_RET VPU_DecCloseCommand(NX_VpuCodecInst *pInst)
{
	FUNCIN();
	if (pInst->isInitialized)
	{
		//NX_DbgMsg( 1, ("Interrupt Atomic Wait Value = %d, intr Reason = 0x%08x\n", atomic_read(&gVpuEventPresent), VpuReadReg(BIT_INT_REASON)) );
		VpuBitIssueCommand(pInst, SEQ_END);
		if( VPU_RET_OK != VPU_WaitVpuBusy(VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) )
		{
			VpuWriteReg(BIT_INT_CLEAR, 0x1);	//	For Signal Break Out
			atomic_set(&gVpuEventPresent, 0);	//	Clear Atomic
			NX_ErrMsg(("VPU_DecCloseCommand() Failed. Timeout(%d)\n", VPU_BUSY_CHECK_TIMEOUT));
			VPU_SWReset( SW_RESET_SAFETY );
			pInst->isInitialized = 0;
			return VPU_RET_ERR_TIMEOUT;
		}
		pInst->isInitialized = 0;
	}
	VpuWriteReg(BIT_INT_CLEAR, 0x1);	//	For Signal Break Out
	atomic_set(&gVpuEventPresent, 0);	//	Clear Atomic
	FUNCOUT();
	return VPU_RET_OK;
}

#if 0
static void DumpData( void *data, int len )
{
	int i=0;
	unsigned char *byte = (unsigned char *)data;
	printk("Dump Data : ");
	for( i=0 ; i<len ; i ++ )
	{
		if( i%16 == 0 )	printk("\n%.8x  ", i);
		printk("%.2x", byte[i] );
		if( i%4 == 3 ) printk(" ");
	}
	printk("\n");
}
#endif
