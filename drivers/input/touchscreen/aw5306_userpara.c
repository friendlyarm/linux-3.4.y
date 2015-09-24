#include "aw5306_reg.h"
#include "aw5306_drv.h"
#include <linux/string.h>
#include "aw5306_userpara.h"

#define	POS_PRECISION				64

extern AW5306_UCF	AWTPCfg;
extern STRUCTCALI	AW_Cali;
extern char AW5306_WorkMode;
extern STRUCTNOISE		AW_Noise;

extern void AW5306_CLB_WriteCfg(void);
extern int AW_I2C_WriteByte(unsigned char addr, unsigned char data);
extern unsigned char AW_I2C_ReadByte(unsigned char addr);
extern unsigned char AW_I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len);
extern unsigned char AW5306_RAWDATACHK(void);

const STRUCTCALI Default_Cali1 =
{
	"AWINIC TP CALI",
	//{0x33,0x23,0x22,0x22,0x22,0x22,0x22,0x02,0x22,0x22},       //TXOFFSET
	{0x32,0x32,0x23,0x32,0x33,0x33,0x33,0x03,0x22,0x22},       //TXOFFSET
	//{0x9A,0xA9,0xAA,0xA9,0x9B,0x00},                             //RXOFFSET
    {0x35,0x44,0x55,0x54,0x34,0x00},                             //RXOFFSET
    //{0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c},//TXCAC
    {0x2C,0x2B,0x2B,0x2A,0x2A,0x2C,0x2C,0x2C,0x2C,0x2C,0x2D,0x2D,0x2D,0x2D,0x31,0x2C,0x2C,0x2C,0x2C,0x2C},//TXCAC
    //{0x3d,0x3c,0x3c,0x3c,0x3e,0x3a,0x3a,0x3e,0x3c,0x3b,0x3c,0x3c},//RXCAC
    {0x84,0x84,0x82,0x82,0x80,0x86,0x86,0x80,0x8C,0x82,0x84,0x84},//RXCAC
    //{0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x2e,0x2e,0x0e,0x0e,0x0e,0x0e,0x0e},//TXGAIN
    {0x88,0x88,0x88,0x88,0x88,0x68,0x68,0x68,0x68,0x68,0x48,0x48,0x48,0x48,0x28,0x08,0x08,0x08,0x08,0x08},//TXGAIN
};


const AW5306_UCF Default_UCF =
{
	18,	//TX_NUM
	12,	//RX_LOCAL

	{0,1,2,3,4,5,6,7,17,16,15,14,13,12,11,10,9,8,0},	// B', A		//TX_ORDER
	{11,10,9,8,7,6,5,4,3,2,1,0},	//RX_ORDER

	0,	//RX_START0,1

	0,	//HAVE_KEY_LINE
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},	//KeyLineValid

	 800,	//MAPPING_MAX_X
	1280,	//MAPPING_MAX_Y

	400,	//GainClbDeltaMin
	500,	//GainClbDeltaMax
	200,	//KeyLineDeltaMin
	300,	//KeyLineDeltaMax
	8300,	//OffsetClbExpectedMin
	8500,	//OffsetClbExpectedMax
	300,	//RawDataDeviation
	10,		//CacMultiCoef

	7000,	//RawDataCheckMin
	10000,	//RawDataCheckMax

	200,	//FLYING_TH
	100,	//MOVING_TH
	50,		//MOVING_ACCELER

	70,		//PEAK_TH
	80,		//GROUP_TH
	90,		//BIGAREA_TH
	25,		//BIGAREA_CNT
	100,	//BIGAREA_FRESHCNT

	1,		//CACULATE_COEF

	1,	//FIRST_CALI
	0,	//RAWDATA_DUMP_SWITCH

	0,	//MULTI_SCANFREQ
	10,	//BASE_FREQ
	0x84,	//FREQ_OFFSET

	1,	//ESD_PROTECT

	0,	//MARGIN_COMPENSATE
	0,	//MARGIN_COMP_DATA_UP
	0,	//MARGIN_COMP_DATA_DOWN
	0,	//MARGIN_COMP_DATA_LEFT
	0,	//MARGIN_COMP_DATA_RIGHT

	1,	//POINT_RELEASEHOLD
	0,	//MARGIN_RELEASEHOLD
	0,	//POINT_PRESSHOLD
	1,	//KEY_PRESSHOLD

	0,	//PEAK_ROW_COMPENSATE
	0,	//PEAK_COL_COMPENSATE
	3,	//PEAK_COMPENSATE_COEF

	0,	//LCD_NOISE_PROCESS
	50,	//LCD_NOISETH

	1,	//FALSE_PEAK_PROCESS
	100,	//FALSE_PEAK_TH

	2,	//STABLE_DELTA_X
	2,	//STABLE_DELTA_Y

	0,	//DEBUG_LEVEL

	50,	//FAST_FRAME
	20,	//SLOW_FRAME

	1,	//GAIN_CLB_SEPERATE
	0,	//MARGIN_PREFILTER
	0,	//BIGAREA_HOLDPOINT
	0,	//CHARGE_NOISE
	0	//FREQ_JUMP
};

unsigned char cpfreq = 0x00;

void AW5306_User_Cfg1(void)
{
	unsigned char i;

	for(i=0;i<AWTPCfg.TX_LOCAL;i++)
	{
		AW_I2C_WriteByte(SA_TX_INDEX0+i,AWTPCfg.TX_ORDER[i]);			//TX REVERT
	}

	AW_I2C_WriteByte(SA_TX_NUM,AWTPCfg.TX_LOCAL);
	AW_I2C_WriteByte(SA_RX_NUM,AWTPCfg.RX_LOCAL);

	if(1 == AWTPCfg.MULTI_SCANFREQ)
	{
		AW_I2C_WriteByte(SA_SCANFREQ1,AWTPCfg.BASE_FREQ);
		AW_I2C_WriteByte(SA_SCANFREQ2,AWTPCfg.BASE_FREQ);
		AW_I2C_WriteByte(SA_SCANFREQ3,AWTPCfg.BASE_FREQ);
	}
	else
	{
		AW_I2C_WriteByte(SA_SCANFREQ1,AWTPCfg.BASE_FREQ);		//3-5
	}
	//AW_I2C_WriteByte(SA_TCLKDLY,1);
	AW_I2C_WriteByte(SA_RX_START,AWTPCfg.RX_START);
	AW_I2C_WriteByte(SA_SCANTIM,4);		// set to 32 TX cycles mode

	AW_I2C_WriteByte(SA_PAGE,1);
	AW_I2C_WriteByte(SA_CHAMPCFG,0x2b);	//
	AW_I2C_WriteByte(SA_OSCCFG1,AWTPCfg.FREQ_OFFSET);			//
	AW_I2C_WriteByte(SA_CPFREQ,0x00);
	AW_I2C_WriteByte(SA_PAGE,0);

	AW5306_CLB_WriteCfg();
}

void AW5306_User_Init(void)
{
	unsigned char ret;

	ret = 0;

	AW5306_WorkMode = DeltaMode;	//DeltaMode: chip output delta data  RawDataMode: chip output rawdata

	memcpy(&AWTPCfg,&Default_UCF,sizeof(AW5306_UCF));
	memcpy(&AW_Cali,&Default_Cali1,sizeof(STRUCTCALI));		//load default cali value

	AW5306_User_Cfg1();

	AW_Noise.FrmState = NOISE_FRM_NORMAL;
	AW_Noise.WorkFreqID = 16;
	AW_Noise.ScanFreqID = AW_Noise.WorkFreqID;
	AW_Noise.State = NOISE_LISTENING;
	AW_Noise.NoiseTh1 = 60;
	AW_Noise.JumpTh = 5;
	AW_Noise.Better_NoiseScan = 1000;
	ret = AW5306_RAWDATACHK();
}

