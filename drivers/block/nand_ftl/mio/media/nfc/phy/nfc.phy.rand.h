/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.rand.h
 * Date         : 2014.09.02
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.09.02, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_RAND_GLOBAL__
#define NFC_PHY_RAND_EXT
#else
#define NFC_PHY_RAND_EXT extern
#endif

NFC_PHY_RAND_EXT int NFC_PHY_RAND_Init(int _buf_size);
NFC_PHY_RAND_EXT void NFC_PHY_RAND_DeInit(void);

NFC_PHY_RAND_EXT void NFC_PHY_RAND_Enable(unsigned char _enable);
NFC_PHY_RAND_EXT int NFC_PHY_RAND_IsEnable(void);

NFC_PHY_RAND_EXT void NFC_PHY_RAND_SetPageSeed(int _page);
NFC_PHY_RAND_EXT void * NFC_PHY_RAND_Randomize(void *_buf, int _buf_size, unsigned char keep);
