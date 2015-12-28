#ifndef __NX_VPU_CONFIG_H__
#define __NX_VPU_CONFIG_H__



#define	NX_MAX_VPU_INST_SPACE	5
#define	NX_MAX_VPU_INSTANCE		5
#define	NX_MAX_VPU_ENC_INST		3
#define	NX_MAX_VPU_DEC_INST		2
#define	NX_MAX_VPU_JPG_INST		1

#if	defined(linux) || defined(__linux) || defined(__LINUX__)
#define NX_VPU_DRIVER_MAJOR		242
#define	NX_VPU_DRIVER_NAME		"nx_vpu"
#endif

#if defined(WIN32) || defined(_WIN32)
#define	NX_VPU_DRIVER_NAME		"NXVPU"
#endif

//	VPU Internal SRAM
#define	VPU_SRAM_PHYSICAL_BASE	0xFFFF0000
#if defined (CONFIG_ARCH_S5P4418)
#define VPU_SRAM_SIZE			(0x10000)
#endif
#if defined (CONFIG_ARCH_S5P6818)
#define VPU_SRAM_SIZE			(0x8000)
#endif

//	VPU Clock Gating
#define	ENABLE_CLOCK_GATING
#define	ENABLE_POWER_SAVING


#define	NX_DBG_INFO			1

//	Debug Register Access
#define	NX_REG_ACC_DEBUG	1			//	Use Debug Function
#define	NX_REG_EN_MSG		0			//	Enable Debug Message


//
//	Memory Size Config
//
//	 -----------------------   High Address
//	| Instance       (1MB)  |
//	 -----------------------
//	| Param Buffer   (1MB)  |
//	 -----------------------
//	| Temp Buf       (1MB)  |
//	 -----------------------
//	| Working Buffer (1MB)  |
//	 -----------------------
//	| Code Buffer    (1MB)  |
//	 -----------------------   Low Address

//#define VPU_MAX_BUF_SIZE		(1024*1024*5)

#define PARA_BUF_SIZE		(12 *1024)
#define	TEMP_BUF_SIZE		(204*1024)
#define	CODE_BUF_SIZE		(260*1024)

#define	COMMON_BUF_SIZE		(CODE_BUF_SIZE+TEMP_BUF_SIZE+PARA_BUF_SIZE)

#define	WORK_BUF_SIZE		(80 *1024)
#define	INST_BUF_SIZE		(NX_MAX_VPU_INST_SPACE*WORK_BUF_SIZE)

#define	DEC_STREAM_SIZE		(1*1024*1024)

#define	VPU_LITTLE_ENDIAN		0
#define	VPU_BIG_ENDIAN			1

#define VPU_FRAME_ENDIAN		VPU_LITTLE_ENDIAN
#define	VPU_FRAME_BUFFER_ENDIAN	VPU_LITTLE_ENDIAN
#define VPU_STREAM_ENDIAN		VPU_LITTLE_ENDIAN


#define	CBCR_INTERLEAVE			0
#define VPU_ENABLE_BWB			1
#define	ENC_FRAME_BUF_CBCR_INTERLEAVE	1


// AXI Expander Select
#define	USE_NX_EXPND			1

//	Timeout
#define	VPU_BUSY_CHECK_TIMEOUT	500			//	500 msec
#define VPU_ENC_TIMEOUT			1000		//	1 sec
#define VPU_DEC_TIMEOUT			300			//	300 msec
#define JPU_ENC_TIMEOUT			1000		//	1 sec



#define VPU_GBU_SIZE    1024	//No modification required
#define JPU_GBU_SIZE	512		//No modification required


//
//
#define	MAX_REG_FRAME			31


//----------------------------------------------------------------------------
//	Encoder Configurations
#define	VPU_ENC_MAX_FRAME_BUF	3

#define VPU_ME_LINEBUFFER_MODE	2


//----------------------------------------------------------------------------
//	Decoder Configurations
#define	VPU_REORDER_ENABLE		1
#define VPU_GMC_PROCESS_METHOD  0
#define VPU_AVC_X264_SUPPORT	1

#define VPU_SPP_CHUNK_SIZE		1024		//	AVC SPP

#endif	//	__NX_VPU_CONFIG_H__
