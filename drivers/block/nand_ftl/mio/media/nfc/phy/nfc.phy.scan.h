/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.scan.h
 * Date         : 2014.09.18
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.09.18, SD.LEE)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_SCAN_GLOBAL__
#define NFC_PHY_SCAN_EXT
#else
#define NFC_PHY_SCAN_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanSec(unsigned char * _id, unsigned char * _jedec_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanToshiba(unsigned char * _id, unsigned char * _jedec_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanMicron(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanSkhynix(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanIntel(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanFeature(unsigned int _scan_format);
