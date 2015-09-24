/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.readretry.h
 * Date         : 2014.08.29
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.25, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_READRETRY_GLOBAL__
#define NFC_PHY_READRETRY_EXT
#else
#define NFC_PHY_READRETRY_EXT extern
#endif



//==============================================================================
// THE FOLLOWINGS ARE ONLY FOR MICRON NAND.
//==============================================================================
typedef struct _NAND_MICRON_READRETRY_
{
  //unsigned int this_size;
  //unsigned int this_size_inverse;
  //unsigned char signature[4];
  //unsigned char this_version[4];

    unsigned char max_channels;
    unsigned char max_ways;
    unsigned char phyway_map[8];

    unsigned char readretry_type;
    unsigned char reserved1[3];

    unsigned char total_readretry_cnt;
    unsigned char curr_readretry_cnt[8][10];       // [max_ways][max_channels]
    unsigned char reserved0[3];

} NAND_MICRON_READRETRY;

/******************************************************************************
 * Global variables
 ******************************************************************************/
NFC_PHY_READRETRY_EXT NAND_MICRON_READRETRY *micron_readretry;



//==============================================================================
// THE FOLLOWINGS ARE ONLY FOR HYNIX NAND.
//==============================================================================
#define NAND_PHY_HYNIX_READRETRY_TOTAL_CNT      (12)
#define NAND_PHY_HYNIX_READRETRY_REG_CNT        (10)

typedef struct _NAND_HYNIX_READRETRY_REG_ADDRESS_
{
    unsigned int this_size;
    unsigned int this_size_inverse;

    unsigned char addr[NAND_PHY_HYNIX_READRETRY_REG_CNT];
    unsigned char reserved[4 - NAND_PHY_HYNIX_READRETRY_REG_CNT%4]; // padding for 4 bytes align

}NAND_HYNIX_READRETRY_REG_ADDRESS;

typedef struct _NAND_HYNIX_READRETRY_REG_DATA_
{
    unsigned int this_size;
    unsigned int this_size_inverse;

    unsigned char total_readretry_cnt;
    unsigned char readretry_reg_cnt;
    unsigned char curr_readretry_cnt;
    unsigned char reserved0;

    unsigned char table[NAND_PHY_HYNIX_READRETRY_TOTAL_CNT][NAND_PHY_HYNIX_READRETRY_REG_CNT];
    unsigned char reserved1[4 - (NAND_PHY_HYNIX_READRETRY_TOTAL_CNT*NAND_PHY_HYNIX_READRETRY_REG_CNT)%4]; // padding for 4 bytes align

}NAND_HYNIX_READRETRY_REG_DATA;

typedef struct _NAND_HYNIX_READRETRY_
{
    unsigned int this_size;
    unsigned int this_size_inverse;

    unsigned char this_signature[4];
    unsigned char this_version[4];

    unsigned char max_channels;
    unsigned char max_ways;
    unsigned char phyway_map[8];

    unsigned char readretry_type;
    unsigned char reserved;

    NAND_HYNIX_READRETRY_REG_ADDRESS reg_addr;
    NAND_HYNIX_READRETRY_REG_DATA **reg_data; // [max_ways][max_channels]

} NAND_HYNIX_READRETRY;

/******************************************************************************
 * Global variables
 ******************************************************************************/
NFC_PHY_READRETRY_EXT NAND_HYNIX_READRETRY hynix_readretry;

/******************************************************************************
 * Global functions
 ******************************************************************************/
NFC_PHY_READRETRY_EXT int NFC_PHY_HYNIX_READRETRY_MakeRegAll(void);

NFC_PHY_READRETRY_EXT void *NFC_PHY_HYNIX_READRETRY_GetAddress(void);
NFC_PHY_READRETRY_EXT void *NFC_PHY_HYNIX_READRETRY_GetRegDataAddress(unsigned int _channel, unsigned int _way);
NFC_PHY_READRETRY_EXT void NFC_PHY_HYNIX_READRETRY_ClearAllCurrReadRetryCount(void);



//==============================================================================
// THE FOLLOWINGS ARE ONLY FOR TOSHIBA NAND.
//==============================================================================
#define NAND_PHY_TOSHIBA_READRETRY_TOTAL_CNT        (10)
#define NAND_PHY_TOSHIBA_READRETRY_PARAMETER_CNT    (10)
#define NAND_PHY_TOSHIBA_READRETRY_CMD_CNT          (5)

typedef struct _NAND_TOSHIBA_READRETRY_
{
  //unsigned int this_size;
  //unsigned int this_size_inverse;
  //unsigned char signature[4];
  //unsigned char this_version[4];

    unsigned char max_channels;
    unsigned char max_ways;
    unsigned char phyway_map[8];

    unsigned char readretry_type;
    unsigned char reserved1[3];

    unsigned char total_readretry_cnt;
    unsigned char curr_readretry_cnt[8][10];       // [max_ways][max_channels]
    unsigned char reserved0[3];

    struct
    {
        unsigned char addr_cnt;
        unsigned char command_cnt;
        unsigned char reserved[2];

        unsigned char addr[NAND_PHY_TOSHIBA_READRETRY_PARAMETER_CNT];
        unsigned char data[NAND_PHY_TOSHIBA_READRETRY_PARAMETER_CNT];
        unsigned char command[NAND_PHY_TOSHIBA_READRETRY_CMD_CNT];
        unsigned char reserved1[4 - (NAND_PHY_TOSHIBA_READRETRY_CMD_CNT%4)]; // padding for 4 bytes align

    } param[NAND_PHY_TOSHIBA_READRETRY_TOTAL_CNT];

    unsigned char exitdata[NAND_PHY_TOSHIBA_READRETRY_PARAMETER_CNT];

} NAND_TOSHIBA_READRETRY;

/******************************************************************************
 * Global variables
 ******************************************************************************/
NFC_PHY_READRETRY_EXT NAND_TOSHIBA_READRETRY *toshiba_readretry;



//==============================================================================
// THE FOLLOWINGS ARE NANDS THAT SUPPORT READ RETRY FEATURES.
//==============================================================================
NFC_PHY_READRETRY_EXT int NFC_PHY_READRETRY_Init(unsigned int _max_channels, unsigned int _max_ways, const unsigned char *_way_map, unsigned char _readretry_type);
NFC_PHY_READRETRY_EXT void NFC_PHY_READRETRY_DeInit(void);
NFC_PHY_READRETRY_EXT int NFC_PHY_READRETRY_GetTotalReadRetryCount(unsigned int _channel, unsigned int _way);
NFC_PHY_READRETRY_EXT void NFC_PHY_READRETRY_SetParameter(unsigned int _channel, unsigned int _phyway);
NFC_PHY_READRETRY_EXT void NFC_PHY_READRETRY_Post(unsigned int _channel, unsigned int _phyway);
NFC_PHY_READRETRY_EXT void NFC_PHY_READRETRY_PrintTable(void);



