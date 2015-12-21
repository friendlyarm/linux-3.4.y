#ifndef __NX_VPU_API_H__
#define	__NX_VPU_API_H__

#include "../include/nx_alloc_mem.h"
#include "../include/vpu_types.h"


//	Codec Mode
enum {
	AVC_DEC = 0,
	VC1_DEC = 1,
	MP2_DEC = 2,
	MP4_DEC = 3,
	DV3_DEC = 3,
	RV_DEC = 4,
	AVS_DEC = 5,
	VPX_DEC = 7,

	AVC_ENC = 8,
	MP4_ENC = 11,
    MJPG_ENC = 13
};


//	MPEG4 Aux Mode
enum {
    MP4_AUX_MPEG4 = 0,
	MP4_AUX_DIVX3 = 1
};

//	VPX Aux Mode
enum {
    VPX_AUX_THO = 0,
	VPX_AUX_VP6 = 1,
	VPX_AUX_VP8 = 2
};

//	AVC Aux Mode
enum {
    AVC_AUX_AVC = 0,
    AVC_AUX_MVC = 1
};

enum {
	SW_RESET_SAFETY,
	SW_RESET_FORCE,
	SW_RESET_ON_BOOT
};


typedef enum {
	VPU_BIT_CHG_GOP			= 1,		//	GOP
	VPU_BIT_CHG_INTRAQP		= (1 << 1),		//	Intra Qp
	VPU_BIT_CHG_BITRATE		= (1 << 2),		//	Bit Rate
	VPU_BIT_CHG_FRAMERATE	= (1 << 3),		//	Frame Rate
	VPU_BIT_CHG_INTRARF		= (1 << 4),		//	Intra Refresh
	VPU_BIT_CHG_SLICEMOD	= (1 << 5),		//	Slice Mode
	VPU_BIT_CHG_HECMODE		= (1 << 6),		//	HEC(Header Extenstion Code) Mode
} NX_VPU_BIT_CHG_PARAM;



typedef enum {
	BS_MODE_INTERRUPT,
	BS_MODE_ROLLBACK,
	BS_MODE_PIC_END
}BitStreamMode;



typedef enum {
    VOL_HEADER,
    VOS_HEADER,
    VIS_HEADER
} Mp4HeaderType;

typedef enum {
    SPS_RBSP,
    PPS_RBSP,
    END_SEQ_RBSP,
    END_STREAM_RBSP,
    SPS_RBSP_MVC,
    PPS_RBSP_MVC,
} AvcHeaderType;




//
//	Define VPU Low-Level Return Value
//
typedef enum {
	VPU_RET_ERR_STRM_FULL = -24,		//	Bitstream Full
	VPU_RET_ERR_SRAM	= -23,			//	VPU SRAM Configruation Failed
	VPU_RET_ERR_INST	= -22,			//	VPU Have No Instance Space
	VPU_RET_BUSY		= -21,			//	VPU H/W Busy
	VPU_RET_ERR_TIMEOUT	= -20,			//	VPU Wait Timeout
	VPU_RET_ERR_MEM_ACCESS= -19,		//	Memory Access Violation

	VPU_RET_ERR_CHG_PARAM = -6,			// 	VPU Not Changed
	VPU_RET_ERR_WRONG_SEQ = -5,			//	Wrong Sequence
	VPU_RET_ERR_PARAM	= -4,			//	VPU Invalid Parameter
	VPU_RET_ERR_RUN		= -3,
	VPU_RET_ERR_INIT	= -2,			//	VPU Not Initialized
	VPU_RET_ERROR		= -1,			//	General operation failed
	VPU_RET_OK			= 0,
	VPU_RET_NEED_STREAM	= 1,			//	Need More Stream
} NX_VPU_RET;



//
//	Common Memory Information
//


//
//
//
typedef	struct {
	int useBitEnable;
	int useIpEnable;
	int useDbkYEnable;
	int useDbkCEnable;
	int useOvlEnable;
	int useBtpEnable;
	unsigned int bufBitUse;
	unsigned int bufIpAcDcUse;
	unsigned int bufDbkYUse;
	unsigned int bufDbkCUse;
	unsigned int bufOvlUse;
	unsigned int bufBtpUse;
	int bufSize;
} SecAxiInfo;


typedef struct {
	int mp4DataPartitionEnable;
	int mp4ReversibleVlcEnable;
	int mp4IntraDcVlcThr;
	int mp4HecEnable;
	int mp4Verid;
} EncMp4Param;

typedef struct {
	int h263AnnexIEnable;
	int h263AnnexJEnable;
	int h263AnnexKEnable;
	int h263AnnexTEnable;
} EncH263Param;

typedef struct {

	//	CMD_ENC_SEQ_264_PARA Register ( 0x1A0 )
	int chromaQpOffset;				//	bit [4:0]
	int constrainedIntraPredFlag;	//	bit [5]
	int disableDeblk;				//	bit [7:6]
	int deblkFilterOffsetAlpha;		//	bit [11:8]
	int deblkFilterOffsetBeta;		//	bit [15:12]

	//	CMD_ENC_SEQ_OPTION Register
	int audEnable;					//	bit[2] AUD(Access Unit Delimiter)

	//	Crop Info
	int enableCrop;
	int cropLeft;
	int cropTop;
	int cropRight;
	int cropBottom;

} EncAvcParam;

typedef struct {
	int picWidth;
	int picHeight;
	int alignedWidth;
	int alignedHeight;
	int seqInited;
	int frameIdx;
	int format;

	int rotationEnable;
	int rotationAngle;
	int mirrorEnable;
	int mirrorDirection;

	int rstIntval;
	int busReqNum;
	int mcuBlockNum;
	int compNum;
	int compInfo[3];

	unsigned int huffCode[4][256];
	unsigned int huffSize[4][256];
	unsigned char huffVal[4][162];
	unsigned char huffBits[4][256];
	unsigned char qMatTab[4][64];;
	unsigned char cInfoTab[4][6];
} EncJpegInfo;

typedef struct {
	int	minFrameBuffers;			//	Minimum Frame Buffers : MJPEG(0) other (3)
	int stride;
	NX_VID_MEMORY_HANDLE frameBuffer[3];
} EncFrameBuffer;

typedef struct tagVpuEncInfo
{
	int codecStd;	//	Codec Standard

	//	input picture
	int srcWidth;
	int srcHeight;

	//	encoding image size
	int	encWidth;
	int	encHeight;

	//
	int gopSize;				//	GOP size
	int bitRate;				//	Bitrate
	int	frameRateNum;			//	framerate
	int frameRateDen;

	int rotateAngle;			//	0/90/180/270
	int mirrorDirection;		//	0/1/2/3

	int	sliceMode;
	int	sliceSizeMode;
	int	sliceSize;

	int bwbEnable;
	int cbcrInterleave;			//	Input Frame's CbCrInterleave
	int cbcrInterleaveRefFrame;	//	Reference Frame's CbCrInterleave
	int frameEndian;

	int frameQp;
	int jpegQuality;

	//
	//	Frame Buffer / Working Buffer
	//
	//	Frame Buffers
	int	minFrameBuffers;			//	Minimum Frame Buffers : MJPEG(0) other (3)
	int frameBufMapType;
	NX_VID_MEMORY_INFO frameBuffer[3];
	SecAxiInfo secAxiInfo;			//	Secondary Working Buffer Information
	unsigned int cacheConfig;		//	for CMD_SET_FRAME_CACHE_CONFIG register
	unsigned int subSampleAPhyAddr;	//	Subsample buffer A Physicall Address
	unsigned int subSampleBPhyAddr;	//	Subsample buffer A Physicall Address

	//	Mpeg4 Encoder Only
	unsigned int usbSampleDPPhyAddr;	//	Data Partition temporal buffer address
	unsigned int usbSampleDPSize;		//	Data Partition temporal buffer size

	int linear2TiledEnable;

	//	Output Stream Buffer's Address & Size
	unsigned int strmBufVirAddr; //	stream buffer start address(virtual)
	unsigned int strmBufPhyAddr; //	stream buffer start address(physical)
	int strmBufSize;			//	stream buffer size

	unsigned int ringBufferEnable;
	unsigned int strmWritePrt;	//	Bitstream Write Ptr
	unsigned int strmReadPrt;	//	Bitstream Read Ptr
	unsigned int strmEndFlag;	//	Bitstream End Flag

	int	userQpMax;				//	User Max Quantization
	int	userGamma;				//	User Gamma Factor

	//	Rate Control
	int rcEnable;				//	Rate Control Enable
	int rcIntraQp;
	int rcIntervalMode;			//	(MB Mode(0), Frame Mode(1), Slice Mode(2), MB-NumMode(3)
	int mbInterval;
	int rcIntraCostWeigth;
	int	enableAutoSkip;			//	Auto Skip
	int	initialDelay;			//
	int	vbvBufSize;				//	VBV buffer size
	int intraRefresh;

	union{
		EncAvcParam avcEncParam;
		EncMp4Param mp4EncParam;
		EncH263Param h263EncParam;
		EncJpegInfo jpgEncInfo;
	}EncCodecParam;

	//	Motion Estimation
	int MEUseZeroPmv;
	int MESearchRange;			//	ME Search Range
	int MEBlockMode;

} VpuEncInfo;


typedef struct {
	int lowDelayEn;
	int numRows;
} LowDelayInfo;



typedef struct tagVpuDecInfo
{
	int codecStd;	//	Codec Standard

	int width;
	int height;

	//	Input Stream Buffer Address
	unsigned int strmBufPhyAddr;
	unsigned int strmBufVirAddr;
	int needMoreFrame;			//
	int strmBufSize;
	int bitstreamMode;

	int bwbEnable;
	int cbcrInterleave;

	int seqInitEscape;

	//	User Data
	int userDataEnable;
	int userDataReportMode;
	unsigned int userDataBufPhyAddr;
	unsigned int userDataBufVirAddr;
	int	userDataBufSize;

	//	Low Dealy Information
	LowDelayInfo lowDelayInfo;

	unsigned int readPos;
	unsigned int writePos;

	//	Frame Buffer Information ( Instance Global )
	int numFrameBuffer;
	NX_VID_MEMORY_INFO frameBuffer[MAX_REG_FRAME];	//	MAX REG Frame Buffer == 31
	SecAxiInfo secAxiInfo;			//	Secondary Working Buffer Information
	int cacheConfig;				//	for CMD_SET_FRAME_CACHE_CONFIG register

	int bytePosFrameStart;
	int bytePosFrameEnd;

	//	Options
	int enableReordering;			//	enable reordering
	int enableMp4Deblock;			//	Mpeg4 Deblocking Option
	int mp4Class;					//	MPEG-4/Divx5.0 or Higher/XVID/Divx4.0/old XVID ( Compress Type --> class )

	//	VC1 Specific Information
	int	vc1BframeDisplayValid;

	//	AVC Specific Information
	int avcErrorConcealMode;

	//
	int frameDelay;
	int streamEndflag;
	int streamEndian;
	int frameDisplayFlag;
	int clearDisplayIndexes;

} VpuDecInfo;

typedef struct tagNX_VpuCodecInst{
	void *devHandle;
	int inUse;
	int instIndex;
	int isInitialized;
	int codecMode;
	int auxMode;
	unsigned int paramPhyAddr;		//	Common Area
	unsigned int paramVirAddr;
	unsigned int paramBufSize;
	unsigned int instBufPhyAddr;	//	Physical
	unsigned int instBufVirAddr;	//	Virtual
	unsigned int instBufSize;
	union{
		VpuDecInfo decInfo;
		VpuEncInfo encInfo;
	} codecInfo;
}NX_VpuCodecInst, *NX_VPU_INST_HANDLE;


// BIT_RUN command
typedef enum {
	SEQ_INIT = 1,
	SEQ_END = 2,
	PIC_RUN = 3,
	SET_FRAME_BUF = 4,
	ENCODE_HEADER = 5,
	ENC_PARA_SET = 6,
	DEC_PARA_SET = 7,
	DEC_BUF_FLUSH = 8,
	RC_CHANGE_PARAMETER	= 9,
	VPU_SLEEP = 10,
	VPU_WAKE = 11,
	ENC_ROI_INIT = 12,
	FIRMWARE_GET = 0xf
}NX_VPU_CMD;

typedef enum {
	VPU_LINEAR_FRAME_MAP  = 0,
	VPU_TILED_FRAME_V_MAP = 1,
	VPU_TILED_FRAME_H_MAP = 2,
	VPU_TILED_FIELD_V_MAP = 3,
	VPU_TILED_MIXED_V_MAP = 4,
	VPU_TILED_FRAME_MB_RASTER_MAP = 5,
	VPU_TILED_FIELD_MB_RASTER_MAP = 6,
	VPU_TILED_MAP_TYPE_MAX
} VPU_GDI_TILED_MAP_TYPE;





//
//	H/W Level APIs
//
void NX_VPU_HwOn(void);
void NX_VPU_HWOff(void);
int NX_VPU_GetCurPowerState(void);
void NX_VPU_Clock( int on );
int VPU_SWReset(int resetMode);

NX_VPU_RET	NX_VpuInit( unsigned int firmVirAddr, unsigned int firmPhyAddr );
NX_VPU_RET	NX_VpuDeInit( void );

int VPU_InitInterrupt(void);
void VPU_DeinitInterrupt(void);

NX_VPU_RET	NX_VpuSuspend( void );
NX_VPU_RET	NX_VpuResume( void );

//
//	Encoder Specific APIs
//
NX_VPU_RET	NX_VpuEncOpen( VPU_OPEN_ARG *openArg, void *devHandle, NX_VPU_INST_HANDLE *handle );
NX_VPU_RET	NX_VpuEncClose( NX_VPU_INST_HANDLE handle );
NX_VPU_RET	NX_VpuEncSetSeqParam( NX_VPU_INST_HANDLE handle, VPU_ENC_SEQ_ARG *seqArg );
NX_VPU_RET	NX_VpuEncSetFrame( NX_VPU_INST_HANDLE handle, VPU_ENC_SET_FRAME_ARG *frmArg );
NX_VPU_RET	NX_VpuEncGetHeader( NX_VPU_INST_HANDLE handle, VPU_ENC_GET_HEADER_ARG *header );
NX_VPU_RET	NX_VpuEncRunFrame( NX_VPU_INST_HANDLE handle, VPU_ENC_RUN_FRAME_ARG *runArg );
NX_VPU_RET	NX_VpuEncChgParam( NX_VPU_INST_HANDLE handle, VPU_ENC_CHG_PARA_ARG *chgArg );


//
//	Decoder Specific APIs
//
NX_VPU_RET	NX_VpuDecOpen( VPU_OPEN_ARG *openArg, void *devHandle, NX_VPU_INST_HANDLE *handle );
NX_VPU_RET	NX_VpuDecClose( NX_VPU_INST_HANDLE handle );
NX_VPU_RET	NX_VpuDecSetSeqInfo( NX_VPU_INST_HANDLE handle, VPU_DEC_SEQ_INIT_ARG *seqArg );
NX_VPU_RET	NX_VpuDecRegFrameBuf( NX_VPU_INST_HANDLE handle, VPU_DEC_REG_FRAME_ARG *frmArg);
NX_VPU_RET	NX_VpuDecRunFrame( NX_VPU_INST_HANDLE handle, VPU_DEC_DEC_FRAME_ARG *pArg);
NX_VPU_RET	NX_VpuDecFlush( NX_VPU_INST_HANDLE handle );
NX_VPU_RET	NX_VpuDecClrDspFlag( NX_VPU_INST_HANDLE handle, VPU_DEC_CLR_DSP_FLAG_ARG *pArg );


//
//	Jpeg Encoder APIs
//

void JPU_DeinitInterrupt(void);
int JPU_InitInterrupt(void);

int JpuSetupTables(EncJpegInfo *pJpgInfo, int quality);

NX_VPU_RET VPU_EncMjpgDefParam( VpuEncInfo *pInfo );
NX_VPU_RET NX_VpuJpegGetHeader( NX_VPU_INST_HANDLE handle, VPU_ENC_GET_HEADER_ARG *pArg );
NX_VPU_RET NX_VpuJpegRunFrame( NX_VPU_INST_HANDLE handle, VPU_ENC_RUN_FRAME_ARG *runArg );


#endif	//	__NX_VPU_API_H__
