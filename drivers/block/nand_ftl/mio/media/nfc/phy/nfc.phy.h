/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.h
 * Date         : 2014.07.07
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.07, SD.LEE)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_GLOBAL__
#define NFC_PHY_EXT
#else
#define NFC_PHY_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
#include "../../exchange.h"

/******************************************************************************
 *
 * NAND Command
 *
 ******************************************************************************/
#define NF_CMD_READ_1ST             0x00
#define NF_CMD_READ_2ND             0x30
#define NF_CMD_READ_COPYBACK_2ND    0x35
#define NF_CMD_READ_CACHE_1ST       0x31
#define NF_CMD_READ_LAST_CACHE_1ST  0x3F
#define NF_CMD_PROG_1ST             0x80
#define NF_CMD_PROG_2ND             0x10
#define NF_CMD_PROG_CACHE_2ND       0x15
#define NF_CMD_PROG_COPYBACK_1ST    0x85
#define NF_CMD_ERASE_1ST            0x60
#define NF_CMD_ERASE_2ND            0xD0
#define NF_CMD_ERASE_PLANE_2ND      0xD1
#define NF_CMD_READ_RANDOM_1ST      0x05
#define NF_CMD_READ_RANDOM_2ND      0xE0
#define NF_CMD_PROG_RANDOM_1ST      0x85
#define NF_CMD_READ_PLANE_1ST       0x60
#define NF_CMD_READ_CACHE_PLANE_2ND 0x33
#define NF_CMD_PROG_PLANE_1ST_80    0x80
#define NF_CMD_PROG_PLANE_1ST_81    0x81
#define NF_CMD_PROG_PLANE_2ND       0x11

#define NF_CMD_READ_ID              0x90
#define NF_CMD_READ_ID_ONFI         0x20
#define NF_CMD_READ_PARAM           0xEC
#define NF_CMD_READ_UNIQUE_ID       0xED
#define NF_CMD_GET_FEATURE          0xEE
#define NF_CMD_SET_FEATURE          0xEF

#define NF_CMD_READ_STATUS          0x70
#define NF_CMD_READ_STATUS1         0xF1
#define NF_CMD_READ_STATUS2         0xF2
#define NF_CMD_RESET                0xFF

/******************************************************************************
 *
 * NAND Status
 *
 ******************************************************************************/
#define NF_BIT_STATUS_FAIL          (1<<0)
#define NF_BIT_STATUS_PRE_FAIL      (1<<1)
#define NF_BIT_STATUS_TRUE_READY    (1<<5)
#define NF_BIT_STATUS_READY         (1<<6)
#define NF_BIT_STATUS_NOWP          (1<<7)

/******************************************************************************
 *
 * AC Timing for CPU Delay
 *
 ******************************************************************************/
typedef struct __NFC_PHY_AC_TIME__
{
    unsigned int tRWC;
    unsigned int tR;
    unsigned int tWB;
    unsigned int tCCS;
    unsigned int tCCS2;
    unsigned int tADL;
    unsigned int tRHW;
    unsigned int tWHR;
    unsigned int tWW;
    unsigned int tRR;
    unsigned int tFEAT;

    unsigned int tParity;

} NFC_PHY_AC_TIME;

/******************************************************************************
 *
 * NFC Timing Register
 *
 ******************************************************************************/
typedef struct __NF_TIME_REGS__
{
	volatile unsigned int nftacs;
	volatile unsigned int nftcos;
	volatile unsigned int nftacc;
	volatile unsigned int nftoch;
	volatile unsigned int nftcah;

} NF_TIME_REGS;

NFC_PHY_EXT int NFC_PHY_ForceSet_Nftime(NF_TIME_REGS _t);


/******************************************************************************
 *
 * NFC DEBUG GPIO
 *
 ******************************************************************************/
//#define DEBUG_TRIGGER_GPIO


/******************************************************************************
 *
 * ECC error information.
 *
 ******************************************************************************/
typedef struct __NFC_ECC_STATUS__
{
    unsigned char max_channels;
    unsigned char max_ways;
    unsigned char phyway_map[8];

    unsigned int **level_error;     // [max_ways][max_channels]
    unsigned int **error;           // [max_ways][max_channels]
    unsigned int **correct_sector;  // [max_ways][max_channels]
    unsigned int **correct_bit;     // [max_ways][max_channels]
    unsigned int **max_correct_bit; // [max_ways][max_channels]

} NFC_ECC_STATUS;

/******************************************************************************
 * Do Not Optimize The NFC Time.
 ******************************************************************************/
NFC_PHY_EXT volatile NFC_PHY_AC_TIME NfcTime;

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT NFC_ECC_STATUS NfcEccStatus;

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_tDelay(unsigned int _tDelay);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT volatile unsigned int NFC_PHY_NfcontrolResetBit(volatile unsigned int _regval);
NFC_PHY_EXT void NFC_PHY_SetAutoResetEnable(int _enable);
NFC_PHY_EXT void NFC_PHY_ClearInterruptPending(int _intnum);
NFC_PHY_EXT void NFC_PHY_SetInterruptEnableAll(int _enable);
NFC_PHY_EXT void NFC_PHY_SetNFBank(unsigned int _bank);
NFC_PHY_EXT void NFC_PHY_SetNFCSEnable(unsigned int _select);
NFC_PHY_EXT void NFC_PHY_SetEccBitMode(unsigned int _bytes_per_ecc, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_SetBytesPerEcc(unsigned int _bytes_per_ecc);
NFC_PHY_EXT void NFC_PHY_SetBytesPerParity(unsigned int _bytes_per_parity);
NFC_PHY_EXT void NFC_PHY_SetElpNum(unsigned int _elpnum);
NFC_PHY_EXT void NFC_PHY_SetEccMode(unsigned int _mode);
NFC_PHY_EXT void NFC_PHY_RunEcc(void);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_EccEncoderSetup(unsigned int _bytes_per_ecc, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_EccEncoderEnable(void);
NFC_PHY_EXT void NFC_PHY_EccEncoderReadParity(unsigned int * _parity, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_EccEncoderWaitDone(void);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_EccDecoderReset(unsigned int _bytes_per_ecc, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_EccDecoderSetOrg(unsigned int * _parity, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_EccDecoderEnable(unsigned int _bytes_per_ecc, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_EccDecoderWaitDone(void);
NFC_PHY_EXT unsigned int NFC_PHY_EccDecoderHasError(void);
NFC_PHY_EXT void NFC_PHY_EccDecoderCorrection(unsigned int _bytes_per_ecc, unsigned int _ecc_bits);
NFC_PHY_EXT void NFC_PHY_EccDecoderWaitCorrectionDone(void);
NFC_PHY_EXT int NFC_PHY_EccDecoderGetErrorLocation(unsigned int * _errorlocationbuffer, unsigned int _ecc_bits);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT unsigned int NFC_PHY_GetEccParitySize(unsigned int _ecc_codeword_size, unsigned int _eccbits);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_SporInit(void);
NFC_PHY_EXT void NFC_PHY_Spor(void);
NFC_PHY_EXT void NFC_PHY_WriteProtect(unsigned int _wp_enable);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_ChipSelect(unsigned int _channel, unsigned int _way, unsigned int _select);
NFC_PHY_EXT void NFC_PHY_Cmd(unsigned char _cmd);
NFC_PHY_EXT void NFC_PHY_Addr(unsigned char _addr);
NFC_PHY_EXT void NFC_PHY_WData(unsigned char _data);
NFC_PHY_EXT void NFC_PHY_WData16(unsigned short _data);
NFC_PHY_EXT void NFC_PHY_WData32(unsigned int _data);
NFC_PHY_EXT unsigned char NFC_PHY_RData(void);
NFC_PHY_EXT unsigned short NFC_PHY_RData16(void);
NFC_PHY_EXT unsigned int NFC_PHY_RData32(void);

/******************************************************************************
 * BoostOn / BoostOff
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_Boost_time_regval(NF_TIME_REGS _t);
NFC_PHY_EXT void NFC_PHY_Origin_time_regval(NF_TIME_REGS _t);
NFC_PHY_EXT void NFC_PHY_BoostOn(void);
NFC_PHY_EXT void NFC_PHY_BoostOff(void);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT unsigned int NFC_PHY_Init(unsigned int _scan_format);
NFC_PHY_EXT void NFC_PHY_DeInit(void);
NFC_PHY_EXT void NFC_PHY_Suspend(void);
NFC_PHY_EXT void NFC_PHY_Resume(void);
NFC_PHY_EXT void NFC_PHY_GetFeatures(unsigned int * _max_channel, unsigned int * _max_way, void * _nand_config);
NFC_PHY_EXT void NFC_PHY_AdjustFeatures(void);
NFC_PHY_EXT void NFC_PHY_SetFeatures(unsigned int _max_channel, unsigned int _max_way, void * _nand_config);
NFC_PHY_EXT int NFC_PHY_EccInfoInit(unsigned int _max_channel, unsigned int _max_way, const unsigned char *_way_map);
NFC_PHY_EXT void NFC_PHY_EccInfoDeInit(void);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT int NFC_PHY_ReadId(unsigned int _channel, unsigned int _way, char * _id, char * _onfi_id, char * _jedec_id);
NFC_PHY_EXT void NFC_PHY_NandReset(unsigned int _channel, unsigned int _way);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT void NFC_PHY_SetOnfiFeature(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned int _parameter);
NFC_PHY_EXT void NFC_PHY_GetOnfiFeature(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned int * _parameter);
NFC_PHY_EXT void NFC_PHY_GetStandardParameter(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned char * _parameter, unsigned char * _ext_parameter);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT int NFC_PHY_StatusIsFAIL(unsigned char _status);
NFC_PHY_EXT int NFC_PHY_StatusIsFAILC(unsigned char _status);
NFC_PHY_EXT int NFC_PHY_StatusIsARDY(unsigned char _status);
NFC_PHY_EXT int NFC_PHY_StatusIsRDY(unsigned char _status);
NFC_PHY_EXT int NFC_PHY_StatusIsWP(unsigned char _status);
NFC_PHY_EXT unsigned char NFC_PHY_StatusRead(unsigned int _channel, unsigned int _way);
NFC_PHY_EXT unsigned char NFC_PHY_StatusData(unsigned int _channel, unsigned int _way);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT int NFC_PHY_1stRead(unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col);
NFC_PHY_EXT int NFC_PHY_2ndReadDataNoEcc(unsigned int _channel, unsigned int _way, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, void * _data_buffer, unsigned int _bytes_spare, void * _spare_buffer);
NFC_PHY_EXT int NFC_PHY_2ndRandomReadDataNoEcc(unsigned int _channel, unsigned int _way, unsigned int _col, unsigned int _bytes_to_read, void * _data_buffer);
NFC_PHY_EXT int NFC_PHY_2ndReadLog(unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col, unsigned int _log_loop_count, unsigned int _bytes_per_log_ecc, unsigned int _bytes_per_log_parity, unsigned int _log_ecc_bits, void * _log_buffer, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer, unsigned int _retryable);
NFC_PHY_EXT int NFC_PHY_2ndReadData(unsigned int _stage, unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col, unsigned int _fake_spare_row, unsigned int _fake_spare_col, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer, unsigned int _bytes_per_spare_ecc, unsigned int _bytes_per_spare_parity, unsigned int _spare_ecc_bits, void * _spare_buffer, unsigned int _retryable);
NFC_PHY_EXT int NFC_PHY_3rdRead(unsigned int _channel, unsigned int _way);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT int NFC_PHY_1stWriteLog(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _col, unsigned int _multi_plane_write_cmd, unsigned int _cache_write_cmd, unsigned int _log_loop_count, unsigned int _bytes_per_log_ecc, unsigned int _bytes_per_log_parity, unsigned int _log_ecc_bits, void * _log_buffer, unsigned int _data_loop_count0, unsigned int _data_loop_count1, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer0, void * _data_buffer1);
NFC_PHY_EXT int NFC_PHY_1stWriteRoot(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _col, unsigned int _root_loop_unit, unsigned int _bytes_per_root_ecc, unsigned int _bytes_per_root_parity, unsigned int _root_ecc_bits, void * _root_buffer);
NFC_PHY_EXT int NFC_PHY_1stWriteData(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _col, unsigned int _multi_plane_write_cmd, unsigned int _cache_write_cmd, unsigned int _data_loop_count0, unsigned int _data_loop_count1, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer0, void * _data_buffer1, unsigned int _bytes_per_spare_ecc, unsigned int _bytes_per_spare_parity, unsigned int _spare_ecc_bits, void * _spare_buffer);
NFC_PHY_EXT int NFC_PHY_1stWriteDataNoEcc(unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, void * _data_buffer, unsigned int _bytes_spare, void * _spare_buffer);
NFC_PHY_EXT int NFC_PHY_2ndWrite(unsigned int _channel, unsigned int _way);
NFC_PHY_EXT unsigned char NFC_PHY_3rdWrite(unsigned int _channel, unsigned int _way);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_EXT int NFC_PHY_1stErase(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _multi_plane_erase_cmd);
NFC_PHY_EXT int NFC_PHY_2ndErase(unsigned int _channel, unsigned int _way);
NFC_PHY_EXT unsigned char NFC_PHY_3rdErase(unsigned int _channel, unsigned int _way);
