#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/cma.h>
#include <linux/mm.h>
#include <linux/pm.h>

#define	NX_DTAG	"[VPU|DRV]"

#include "../include/nx_vpu_config.h"
#include "../include/drv_osapi.h"
#include "../include/nx_vpu_api.h"

#include "../include/vpu_drv_ioctl.h"
#include "../include/vpu_types.h"

#include <mach/soc.h>

#if defined (CONFIG_ARCH_S5P6818)
#include <mach/s5p6818.h>
#endif

#if defined (CONFIG_ARCH_S5P4418)
#include <mach/s5p4418.h>
#endif

#include <mach/devices.h>
#include <linux/clk.h>

#ifdef CONFIG_NEXELL_DFS_BCLK
#include <mach/nxp-dfs-bclk.h>
#endif

//----------------------------------------------------------------------------
//								Global Variables

//	Instance Management
static int	gstCurNumInstance = 0;		//	Maximum Instances         : NX_MAX_VPU_INSTANCE
static int	gstCruEncInstance = 0;		//	Maximum Encoder Instances : NX_MAX_VPU_ENC_INST	//	0~1
static int	gstCruDecInstance = 0;		//	Maximum Decoder Instances : NX_MAX_VPU_DEC_INST	//	2~3
static int	gstCurJPGInstance = 0;		//	Maximum JPG Enc Instances : NX_MAX_VPU_JPG_INST	//	4

//	Firmware Physical & Virtual Address
static dma_addr_t	gstFirmPhyAddress = 0;		//	Fimmware Physical Address
static unsigned int	gstFirmVirAddress = 0;		//	Firmware Virtual Address

//	Mutex
static LinuxMutex gstDrvMutex;	//	Inter IOCTL Lock

//	Platform device
static struct platform_device *nx_vpu_device;


//----------------------------------------------------------------------------
//					Device Driver Source Code


//----------------------------------------------------------------------------
static int nx_vpu_open( struct inode *inode, struct file *flip )
{
	DrvMutexLock( &gstDrvMutex );

	if( gstCurNumInstance >= NX_MAX_VPU_INSTANCE )
	{
		NX_ErrMsg( ("Cannot open vpu driver.(gstCurNumInstance=%d)\n", gstCurNumInstance) );
		DrvMutexUnlock( &gstDrvMutex );
		return -1;
	}

#ifdef ENABLE_POWER_SAVING
	if( gstCurNumInstance == 0 )
	{
		//	H/W Power On
#ifdef CONFIG_NEXELL_DFS_BCLK
        bclk_get(BCLK_USER_MPEG);
#endif
		NX_VPU_Clock( 1 );
		NX_VpuInit( gstFirmVirAddress, gstFirmPhyAddress );
#ifdef ENABLE_CLOCK_GATING
		NX_VPU_Clock( 0 );
#endif
	}
#endif

	gstCurNumInstance ++;
	flip->private_data = 0;
	DrvMutexUnlock( &gstDrvMutex );
	return 0;
}

//----------------------------------------------------------------------------
static int nx_vpu_close( struct inode *inode, struct file *filp )
{
	NX_VPU_INST_HANDLE hInst = (NX_VPU_INST_HANDLE)filp->private_data;
	DrvMutexLock( &gstDrvMutex );

#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock( 1 );
#endif

	if( hInst && hInst->inUse )
	{
		hInst->inUse = 0;
		if( hInst->codecMode < AVC_ENC )	gstCruDecInstance --;
		else								gstCruEncInstance --;

		if( hInst->isInitialized )
		{
			if( hInst->codecMode < AVC_ENC )
				NX_VpuDecClose( hInst );
			else
				NX_VpuEncClose( hInst );
			hInst->isInitialized = 0;
		}
		NX_DrvMemset( &hInst->codecInfo, 0, sizeof(hInst->codecInfo) );
	}
	gstCurNumInstance --;

#ifdef ENABLE_POWER_SAVING
	if( gstCurNumInstance == 0 )
	{
		//	H/W Power Off
		NX_VPU_Clock( 1 );
		NX_VpuDeInit();
#ifdef ENABLE_CLOCK_GATING
		NX_VPU_Clock( 0 );
#endif
		NX_DbgMsg(0, ("End of power saving!!!\n"));
#ifdef CONFIG_NEXELL_DFS_BCLK
        bclk_put(BCLK_USER_MPEG);
#endif
	}
#endif

#ifdef ENABLE_CLOCK_GATING
		NX_VPU_Clock( 0 );
#endif

	DrvMutexUnlock( &gstDrvMutex );
	return 0;
}


//----------------------------------------------------------------------------
static long nx_vpu_ioctl( struct file *filp, unsigned int cmd, unsigned long arg )
{
	int ret = 0;
	NX_VPU_INST_HANDLE hInst = (NX_VPU_INST_HANDLE)filp->private_data;

	DrvMutexLock( &gstDrvMutex );

	FUNCIN();
#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock(1);
#endif

	switch( cmd )
	{
	case IOCTL_VPU_OPEN_INSTANCE:
		{
			NX_VPU_RET vpuRet;
			NX_VPU_INST_HANDLE hInst = 0;
			VPU_OPEN_ARG openArg;

			if( 0 != copy_from_user( &openArg, (void*)arg, sizeof(openArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_OPEN_INSTANCE : copy_from_user failed!!\n"));
				ret = -1;
				break;
			}

			if( openArg.isEncoder )
			{
				if( gstCruEncInstance >= NX_MAX_VPU_ENC_INST )
				{
					NX_ErrMsg( ("Maximum encoder limit error. Current(%d) vs. MAX(%d)!!!", gstCruEncInstance, NX_MAX_VPU_ENC_INST) );
					ret = -1;
					break;
				}
				vpuRet = NX_VpuEncOpen( &openArg, &nx_vpu_device->dev, &hInst );
			}
			else
			{
				if( gstCruDecInstance >= NX_MAX_VPU_DEC_INST )
				{
					NX_ErrMsg( ("Maximum decoder limit error. Current(%d) vs. MAX(%d)!!!", gstCruDecInstance, NX_MAX_VPU_DEC_INST) );
					ret = -1;
					break;
				}
				vpuRet = NX_VpuDecOpen( &openArg, &nx_vpu_device->dev, &hInst );
			}

			if( (VPU_RET_OK!=vpuRet) || (0==hInst) )
			{
				NX_ErrMsg(("Cannot open VPU Instance( codecStd=%d, isEncoder=%d, hInst=%p )\n", openArg.codecStd, openArg.isEncoder, hInst));
				ret = -1;
				break;
			}

			openArg.instIndex = hInst->instIndex;

			if( 0 != copy_to_user( (void*)arg, &openArg, sizeof(openArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_OPEN_INSTANCE : copy_to_user failed!!\n"));
				ret = -1;
				break;
			}
			filp->private_data = hInst;
			if( ret == 0 )
			{
				if( openArg.isEncoder )	gstCruEncInstance ++;
				else					gstCruDecInstance ++;
			}
		}
		break;

	case IOCTL_VPU_CLOSE_INSTANCE:
		{
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = -1;
				break;
			}

			if( (MP4_ENC == hInst->codecMode) ||  (AVC_ENC == hInst->codecMode) )
			{
				NX_VpuEncClose( hInst );
			}
			else if( MJPG_ENC == hInst->codecMode )
			{
				//	N/A yet
			}
			else
			{
				NX_VpuDecClose( hInst );
			}
		}
		break;


		//////////////////////////////////////////////////////////////////////
		//
		//		Encoder Commands
		//
	case IOCTL_VPU_ENC_SET_SEQ_PARAM:
		{
			NX_VPU_RET vpuRet;
			VPU_ENC_SEQ_ARG seqArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = VPU_RET_ERR_INST;
				break;
			}

			if( 0 != copy_from_user( &seqArg, (void*)arg, sizeof(seqArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_SEQ_PARAM : copy_from_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuEncSetSeqParam( hInst, &seqArg )) )
			{
				NX_ErrMsg(("NX_VpuEncSetSeqParam() failed.(ErrorCode=%d)\n", vpuRet));
				ret = vpuRet;
				break;
			}

			if( 0 != copy_to_user( (void*)arg, &seqArg, sizeof(seqArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_SEQ_PARAM : copy_to_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}
		}
		break;

	case IOCTL_VPU_ENC_SET_FRAME_BUF:
		{
			NX_VPU_RET vpuRet;
			VPU_ENC_SET_FRAME_ARG frmArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = VPU_RET_ERR_INST;
				break;
			}

			if( 0 != copy_from_user( &frmArg, (void*)arg, sizeof(frmArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_FRAME_BUF : copy_from_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuEncSetFrame( hInst, &frmArg )) )
			{
				NX_ErrMsg(("NX_VpuEncSetFrame() failed.(ErrorCode=%d)\n", vpuRet));
				ret = vpuRet;
				break;
			}

			if( 0 != copy_to_user( (void*)arg, &frmArg, sizeof(frmArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_FRAME_BUF : copy_to_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}
		}
		break;

	case IOCTL_VPU_ENC_GET_HEADER:
		{
			NX_VPU_RET vpuRet;
			static VPU_ENC_GET_HEADER_ARG hdrArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = VPU_RET_ERR_INST;
				break;
			}

			if( 0 != copy_from_user( &hdrArg, (void*)arg, sizeof(hdrArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_FRAME_BUF : copy_from_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuEncGetHeader( hInst, &hdrArg )) )
			{
				NX_ErrMsg(("NX_VpuEncGetHeader() failed.(ErrorCode=%d)\n", vpuRet));
				ret = vpuRet;
				break;
			}

			if( 0 != copy_to_user( (void*)arg, &hdrArg, sizeof(hdrArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_FRAME_BUF : copy_to_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}
		}
		break;

	case IOCTL_VPU_ENC_RUN_FRAME:
		{
			NX_VPU_RET vpuRet;
			VPU_ENC_RUN_FRAME_ARG runArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = VPU_RET_ERR_INST;
				break;
			}

			if( 0 != copy_from_user( &runArg, (void*)arg, sizeof(runArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_RUN_FRAME : copy_from_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuEncRunFrame( hInst, &runArg )) )
			{
				NX_ErrMsg(("NX_VpuEncRunFrame() failed.(ErrorCode=%d)\n", vpuRet));
				ret = vpuRet;
				break;
			}

			if( 0 != copy_to_user( (void*)arg, &runArg, sizeof(runArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_RUN_FRAME : copy_to_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}
		}
		break;

	case IOCTL_VPU_ENC_CHG_PARAM:
		{
			NX_VPU_RET vpuRet;
			VPU_ENC_CHG_PARA_ARG chgArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = VPU_RET_ERR_INST;
				break;
			}

			if( 0 != copy_from_user( &chgArg, (void*)arg, sizeof(chgArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_CHG_PARAM : copy_from_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuEncChgParam( hInst, &chgArg )) )
			{
				NX_ErrMsg(("NX_VpuEncChgParam() failed.(ErrorCode=%d)\n", vpuRet));
				ret = vpuRet;
				break;
			}

			if( 0 != copy_to_user( (void*)arg, &chgArg, sizeof(chgArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_CHG_PARAM : copy_to_user failed!!\n"));
				ret = VPU_RET_ERROR;
				break;
			}
		}
		break;

		//////////////////////////////////////////////////////////////////////
		//
		//		Decoder Commands
		//
	case IOCTL_VPU_DEC_SET_SEQ_INFO:
		{
			NX_VPU_RET vpuRet;
			VPU_DEC_SEQ_INIT_ARG seqArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_SET_SEQ_INFO : Invalid Instance!!\n"));
				ret = -1;
				break;
			}
			if( 0 != copy_from_user( &seqArg, (void*)arg, sizeof(seqArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_SET_SEQ_INFO : copy_from_user failed!!\n"));
				ret = -1;
				break;
			}

			if(0)
			{
				unsigned char *pbuf;
				pbuf = seqArg.seqData;
				printk("--------------- %s(%d) size(%d)  : 0x%02x%02x%02x%02x 0x%02x%02x%02x%02x 0x%02x%02x%02x%02x 0x%02x%02x%02x%02x\n",
					__func__, __LINE__, seqArg.seqDataSize,
					pbuf[ 0],pbuf[ 1],pbuf[ 2],pbuf[ 3], pbuf[ 4],pbuf[ 5],pbuf[ 6],pbuf[ 7],
					pbuf[ 8],pbuf[ 9],pbuf[10],pbuf[11], pbuf[12],pbuf[13],pbuf[14],pbuf[15]);
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuDecSetSeqInfo( hInst, &seqArg )) )
			{
				NX_ErrMsg(("NX_VpuDecSetSeqInfo() failed.(ErrorCode=%d)\n", vpuRet));
				ret = vpuRet;
				break;
			}
			if( 0 != copy_to_user( (void*)arg, &seqArg, sizeof(seqArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_SET_SEQ_INFO : copy_to_user failed!!\n"));
				ret = -1;
				break;
			}
		}
		break;

	case IOCTL_VPU_DEC_REG_FRAME_BUF:
		{
			NX_VPU_RET vpuRet;
			static VPU_DEC_REG_FRAME_ARG frmArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_REG_FRAME_BUF : Invalid Instance!!\n"));
				ret = -1;
				break;
			}
			if( 0 != copy_from_user( &frmArg, (void*)arg, sizeof(frmArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_REG_FRAME_BUF : copy_from_user failed!!\n"));
				ret = -1;
				break;
			}
			if( VPU_RET_OK != (vpuRet = NX_VpuDecRegFrameBuf( hInst, &frmArg )) )
			{
				NX_ErrMsg(("NX_VpuDecRegFrameBuf() failed.(ErrorCode=%d)\n", vpuRet));
				ret = -1;
				break;
			}
		}
		break;
	case IOCTL_VPU_DEC_RUN_FRAME:
		{
			VPU_DEC_DEC_FRAME_ARG decArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_RUN_FRAME : Invalid Instance!!\n"));
				ret = -1;
				break;
			}
			if( 0 != copy_from_user( &decArg, (void*)arg, sizeof(decArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_RUN_FRAME : copy_from_user failed!!\n"));
				ret = -1;
				break;
			}
			ret = NX_VpuDecRunFrame( hInst, &decArg );
			if( ret < 0 )
			{
				NX_ErrMsg(("NX_VpuDecRunFrame() failed.(ErrorCode=%d)\n", ret));
				ret = -1;
				break;
			}
			if( 0 != copy_to_user( (void*)arg, &decArg, sizeof(decArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_RUN_FRAME : copy_to_user failed!!\n"));
				ret = -1;
				break;
			}
		}
		break;
	case IOCTL_VPU_DEC_FLUSH:
		{
			ret = NX_VpuDecFlush( hInst );
		}
		break;
	case IOCTL_VPU_DEC_CLR_DSP_FLAG:
		{
			NX_VPU_RET vpuRet;
			VPU_DEC_CLR_DSP_FLAG_ARG clrDspArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_CLR_DSP_FLAG : Invalid Instance!!\n"));
				ret = -1;
				break;
			}
			if( 0 != copy_from_user( &clrDspArg, (void*)arg, sizeof(clrDspArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_CLR_DSP_FLAG : copy_from_user failed!!\n"));
				ret = -1;
				break;
			}
			if( VPU_RET_OK != (vpuRet = NX_VpuDecClrDspFlag( hInst, &clrDspArg )) )
			{
				NX_ErrMsg(("NX_VpuDecClrDspFlag() failed.(ErrorCode=%d)\n", vpuRet));
				ret = -1;
				break;
			}
			if( 0 != copy_to_user( (void*)arg, &clrDspArg, sizeof(clrDspArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_CLR_DSP_FLAG : copy_to_user failed!!\n"));
				ret = -1;
				break;
			}
		}
		break;
	case IOCTL_VPU_JPG_GET_HEADER:
		{
			NX_VPU_RET vpuRet;
			VPU_ENC_GET_HEADER_ARG *pArg = (VPU_ENC_GET_HEADER_ARG *)NX_DrvMalloc( sizeof(VPU_ENC_GET_HEADER_ARG) );
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_CLOSE_INSTANCE : Invalid Instance!!\n"));
				ret = -1;
				if(pArg)	NX_DrvFree(pArg);
				break;
			}

			if( VPU_RET_OK != (vpuRet = NX_VpuJpegGetHeader( hInst, pArg )) )
			{
				NX_ErrMsg(("NX_VpuDecClrDspFlag() failed.(ErrorCode=%d)\n", vpuRet));
				ret = -1;
				if( pArg ) NX_DrvFree( pArg );
				break;
			}

			if( 0 != copy_to_user( (void*)arg, pArg, sizeof(VPU_ENC_GET_HEADER_ARG) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_ENC_SET_FRAME_BUF : copy_to_user failed!!\n"));
				ret = -1;
				if(pArg)	NX_DrvFree(pArg);
				break;
			}
			if(pArg)	NX_DrvFree(pArg);
			break;
		}
	case IOCTL_VPU_JPG_RUN_FRAME:
		{
			VPU_ENC_RUN_FRAME_ARG encArg;
			if( !hInst )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_RUN_FRAME : Invalid Instance!!\n"));
				ret = -1;
				break;
			}
			if( 0 != copy_from_user( &encArg, (void*)arg, sizeof(encArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_RUN_FRAME : copy_from_user failed!!\n"));
				ret = -1;
				break;
			}
			ret = NX_VpuJpegRunFrame( hInst, &encArg );
			if( ret < 0 )
			{
				NX_ErrMsg(("NX_VpuDecRunFrame() failed.(ErrorCode=%d)\n", ret));
				ret = -1;
				break;
			}
			if( 0 != copy_to_user( (void*)arg, &encArg, sizeof(encArg) ) )
			{
				NX_ErrMsg(("IOCTL_VPU_DEC_RUN_FRAME : copy_to_user failed!!\n"));
				ret = -1;
				break;
			}
		}
		break;
	default:
		break;
	}

#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock(0);
#endif
	FUNCOUT();

	DrvMutexUnlock( &gstDrvMutex );

	return ret;
}

struct file_operations nx_vpu_ops = {
	.owner			= THIS_MODULE,
	.open			= nx_vpu_open,
	.release			= nx_vpu_close,
	.unlocked_ioctl	= nx_vpu_ioctl,
};

//----------------------------------------------------------------------------
//					Module Device Loader

static struct class *nx_vpu_class = 0;
static int nx_vpu_drv_probe( struct platform_device *pdev )
{
	int	res;
	res = register_chrdev(NX_VPU_DRIVER_MAJOR, NX_VPU_DRIVER_NAME, &nx_vpu_ops);
	if(res<0)
	{
		NX_ErrMsg(( "register_chrdev() failed!!!\n" ));
		return res;
	}
	nx_vpu_class = class_create(THIS_MODULE, NX_VPU_DRIVER_NAME);
	device_create(nx_vpu_class, NULL, MKDEV(NX_VPU_DRIVER_MAJOR, 0), NULL, NX_VPU_DRIVER_NAME);
	return 0;
}

static int nx_vpu_drv_remove( struct platform_device *pdev )
{
	unregister_chrdev(NX_VPU_DRIVER_MAJOR, NX_VPU_DRIVER_NAME);
	class_destroy(nx_vpu_class);
	return 0;
}

/* suspend and resume support for the video firmware */
static int nx_vpu_drv_suspend(struct platform_device *dev, pm_message_t state)
{
	DrvMutexLock( &gstDrvMutex );
	PM_DBGOUT("nx_vpu_drv_suspend++\n");
	NX_VPU_Clock( 1 );

	NX_VpuSuspend();
#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock( 0 );
#endif
	PM_DBGOUT("nx_vpu_drv_suspend--\n");
	DrvMutexUnlock( &gstDrvMutex );
	return 0;
}

static int nx_vpu_drv_resume(struct platform_device *dev)
{
	DrvMutexLock( &gstDrvMutex );
	PM_DBGOUT("nx_vpu_drv_resume++\n");
	NX_VPU_Clock( 1 );
	NX_VpuResume();
#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock( 0 );
#endif
	PM_DBGOUT("nx_vpu_drv_resume--\n");
	DrvMutexUnlock( &gstDrvMutex );
	return 0;
}

static struct platform_driver nx_vpu_driver = {
	.probe		= nx_vpu_drv_probe,
	.remove		= nx_vpu_drv_remove,
	.suspend	= nx_vpu_drv_suspend,
	.resume		= nx_vpu_drv_resume,
	.driver		= {
		.name	= NX_VPU_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

#include "../src/vpu_hw_interface.h"			//	Register Access
//----------------------------------------------------------------------------
//				Nexell VPU Initialize & Release

static int __devinit nx_vpu_init(void)
{
	int res = -1;
	DrvInitMutex( &gstDrvMutex, "NX_VPU_MUTEX" );


	/* register platform device */
	nx_vpu_device = platform_device_register_simple(NX_VPU_DRIVER_NAME, -1, NULL, 0);
	if(IS_ERR(nx_vpu_device)){
		NX_ErrMsg( ("Failed to add platform device %s \n", NX_VPU_DRIVER_NAME) );
		return res;
	}

	JPU_InitInterrupt();
	VPU_InitInterrupt();

	NX_LinearAlloc( &nx_vpu_device->dev, &gstFirmPhyAddress, &gstFirmVirAddress, COMMON_BUF_SIZE, 4096 );
	NX_DrvMemset( (void*)gstFirmVirAddress, 0, COMMON_BUF_SIZE );

	NX_VPU_Clock( 1 );
	NX_VpuInit( gstFirmVirAddress, gstFirmPhyAddress );

#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock( 0 );
#endif

	/* register platform driver, exec platform_driver probe */
	res =  platform_driver_register(&nx_vpu_driver);
	if(res)
	{
		platform_device_unregister(nx_vpu_device);
		NX_ErrMsg( ("Failed to add platrom driver %s (%d) \n", nx_vpu_driver.driver.name, res) );
	}
	return 0;
}

static void __exit nx_vpu_exit(void)
{
	DrvMutexLock( &gstDrvMutex );
	if(gstCurNumInstance > 0)
	{
		NX_DbgMsg( NX_DBG_INFO, ("Warning Video Frimware is running.(Total(%d)=Enc(%d)+Dec(%d)+Jpeg(%d)\n" ,
			gstCurNumInstance, gstCruEncInstance, gstCruDecInstance, gstCurJPGInstance) );
	}
	device_destroy(nx_vpu_class, MKDEV(NX_VPU_DRIVER_MAJOR, 0));

	platform_driver_unregister(&nx_vpu_driver);
	platform_device_unregister(nx_vpu_device);

	JPU_DeinitInterrupt();
	VPU_DeinitInterrupt();

	NX_VPU_Clock( 1 );
	NX_VpuDeInit();
#ifdef ENABLE_CLOCK_GATING
	NX_VPU_Clock( 0 );
#endif
	if( 0 != gstFirmPhyAddress )
	{
		NX_LinearFree( &nx_vpu_device->dev, gstFirmPhyAddress, gstFirmVirAddress );
	}
	DrvMutexUnlock( &gstDrvMutex );

	DrvCloseMutex( &gstDrvMutex );

	NX_DbgMsg( NX_DBG_INFO, ( TEXT("Nexell VPU Driver Unloaded\n") ) );
}

module_init(nx_vpu_init);
module_exit(nx_vpu_exit);


MODULE_AUTHOR("SeongO Park <ray@nexell.co.kr>");
MODULE_DESCRIPTION("Nexell Video En/Decoder Accelerator Device Driver");
MODULE_LICENSE("GPL");
