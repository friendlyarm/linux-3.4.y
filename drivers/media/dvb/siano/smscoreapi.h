/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
Copyright (C) 2006-2011, Doron Cohen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************/

#ifndef __SMS_CORE_API_H__
#define __SMS_CORE_API_H__

#include <linux/version.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/timer.h>

#include <asm/page.h>

#ifdef SMS_RC_SUPPORT_SUBSYS
#include "smsir.h"
#endif

#define MAJOR_VERSION 3
#define MINOR_VERSION 0
#define SUB_VERSION 9

#define STRINGIZE2(z) #z
#define STRINGIZE(z) STRINGIZE2(z)

#define VERSION_STRING "Version: " STRINGIZE(MAJOR_VERSION) "." STRINGIZE(MINOR_VERSION) "." STRINGIZE(SUB_VERSION)
#define MODULE_AUTHOR_STRING "Siano Mobile Silicon, Inc. (doronc@siano-ms.com)"

/************************************************************************/
/* Defines, types and structures taken fron Siano SmsHostLibTypes.h,	*/
/* specifying required API with FW					*/
/************************************************************************/

typedef enum
{
	SMSHOSTLIB_DEVMD_DVBT,
	SMSHOSTLIB_DEVMD_DVBH,
	SMSHOSTLIB_DEVMD_DAB_TDMB,
	SMSHOSTLIB_DEVMD_DAB_TDMB_DABIP,
	SMSHOSTLIB_DEVMD_DVBT_BDA,
	SMSHOSTLIB_DEVMD_ISDBT,
	SMSHOSTLIB_DEVMD_ISDBT_BDA,
	SMSHOSTLIB_DEVMD_CMMB,
	SMSHOSTLIB_DEVMD_RAW_TUNER,
	SMSHOSTLIB_DEVMD_FM_RADIO,
	SMSHOSTLIB_DEVMD_FM_RADIO_BDA,
	SMSHOSTLIB_DEVMD_ATSC,
	SMSHOSTLIB_DEVMD_MAX,
	SMSHOSTLIB_DEVMD_NONE = 0xFFFFFFFF
} SMSHOSTLIB_DEVICE_MODES_E;

typedef enum SMSHOSTLIB_FREQ_BANDWIDTH_E
{
	BW_8_MHZ		= 0,
	BW_7_MHZ		= 1,
	BW_6_MHZ		= 2,
	BW_5_MHZ		= 3,
	BW_ISDBT_1SEG	= 4,
	BW_ISDBT_3SEG	= 5,
	BW_2_MHZ		= 6,
	BW_FM_RADIO		= 7,
	BW_ISDBT_13SEG	= 8,
	BW_1_5_MHZ		= 15,
	BW_UNKNOWN		= 0xFFFF

} SMSHOSTLIB_FREQ_BANDWIDTH_ET;

#define HIF_TASK			11		// Firmware messages processor task IS
#define DLU_TASK			20		// Firmware messages processor task IS
#define HIF_TASK_SLAVE			22
#define HIF_TASK_SLAVE2			33
#define HIF_TASK_SLAVE3			44
#define SMS_HOST_LIB			150

#define MSG_HDR_FLAG_SPLIT_MSG		4

typedef struct SmsMsgHdr_S
{
	u16 	msgType;
	u8	msgSrcId;
	u8	msgDstId;
	u16	msgLength;	// Length is of the entire message, including header
	u16	msgFlags;
} SmsMsgHdr_ST;

typedef struct SmsMsgData_S
{
	SmsMsgHdr_ST	xMsgHeader;
	u32			msgData[1];
} SmsMsgData_ST;

typedef struct SmsMsgData2Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	u32			msgData[2];
} SmsMsgData2Args_ST;


typedef struct SmsMsgData3Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	u32			msgData[3];
} SmsMsgData3Args_ST;

typedef struct SmsMsgData4Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	u32			msgData[4];
} SmsMsgData4Args_ST;


// Definitions of the message types.
// For each type, the format used (excluding the header) is specified
// The message direction is also specified

typedef enum MsgTypes_E
{
	MSG_TYPE_BASE_VAL = 500,

	//MSG_SMS_RESERVED1 = 501,				//		
	//MSG_SMS_RESERVED1 = 502,				//

	MSG_SMS_GET_VERSION_REQ = 503,			// Get version
											// Format: None
											// Direction: Host->SMS

	MSG_SMS_GET_VERSION_RES = 504,			// The response to MSG_SMS_GET_VERSION_REQ
											// Format:	8-bit - Version string
											// Direction: SMS->Host

	MSG_SMS_MULTI_BRIDGE_CFG	= 505,		// Multi bridge configuration message
											// Format: 
											//		32 Bit Config type
											//		Rest - depends on config type.
	MSG_SMS_GPIO_CONFIG_REQ		= 507,		// Configure pin for GPIO 
	MSG_SMS_GPIO_CONFIG_RES		= 508,
	//MSG_SMS_RESERVED1 = 506,				//

	MSG_SMS_GPIO_SET_LEVEL_REQ = 509,		// Set GPIO level high / low
											// Format: Data[0] = u32 PinNum
											//		   Data[1] = u32 NewLevel
											// Direction: Host-->FW

	MSG_SMS_GPIO_SET_LEVEL_RES = 510,		// The response to MSG_SMS_GPIO_SET_LEVEL_REQ
											// Direction: FW-->Host

	MSG_SMS_GPIO_GET_LEVEL_REQ = 511,		// Get GPIO level high / low
											// Format: Data[0] = u32 PinNum
											//		   Data[1] = 0
											// Direction: Host-->FW
											  
	MSG_SMS_GPIO_GET_LEVEL_RES = 512,		// The response to MSG_SMS_GPIO_GET_LEVEL_REQ
											// Direction: FW-->Host

	MSG_SMS_EEPROM_BURN_IND = 513,				//

	MSG_SMS_LOG_ENABLE_CHANGE_REQ = 514,	// Change the state of (enable/disable) log messages flow from SMS to Host (MSG_SMS_LOG_ITEM)
											// Format: 32-bit address value for g_log_enable
											// Direction: Host->SMS

	MSG_SMS_LOG_ENABLE_CHANGE_RES = 515,	// A reply to MSG_SMS_LOG_ENABLE_CHANGE_REQ
											// Format: 32-bit address value for g_log_enable
											// Direction: SMS->Host

	MSG_SMS_SET_MAX_TX_MSG_LEN_REQ = 516,	// Set the maximum length of a receiver message
											// Format: 32-bit value of length in bytes, must be modulo of 4
	MSG_SMS_SET_MAX_TX_MSG_LEN_RES = 517,	// ACK/ERR for MSG_SMS_SET_MAX_TX_MSG_LEN_REQ

	MSG_SMS_SPI_HALFDUPLEX_TOKEN_HOST_TO_DEVICE	= 518,	// SPI Half-Duplex protocol
	MSG_SMS_SPI_HALFDUPLEX_TOKEN_DEVICE_TO_HOST	= 519,  //

	// DVB-T MRC background scan messages
	MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_REQ	= 520, 
	MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_RES = 521, 
	MSG_SMS_BACKGROUND_SCAN_SIGNAL_DETECTED_IND		= 522,  
	MSG_SMS_BACKGROUND_SCAN_NO_SIGNAL_IND			= 523,  

	MSG_SMS_CONFIGURE_RF_SWITCH_REQ		= 524,
	MSG_SMS_CONFIGURE_RF_SWITCH_RES		= 525,

	MSG_SMS_MRC_PATH_DISCONNECT_REQ		= 526,
	MSG_SMS_MRC_PATH_DISCONNECT_RES		= 527,


	MSG_SMS_RECEIVE_1SEG_THROUGH_FULLSEG_REQ = 528, // Application: ISDB-T on SMS2270
													// Description: In full segment application, 
													//				enable reception of 1seg service even if full segment service is not received
	MSG_SMS_RECEIVE_1SEG_THROUGH_FULLSEG_RES = 529, // Application: ISDB-T on SMS2270
													// Description: In full segment application, 
													// enable reception of 1seg service even if
// Firmware messages processor task IS
	MSG_SMS_RECEIVE_VHF_VIA_VHF_INPUT_REQ = 530,	// Application: All on SMS2270
													// Enable VHF signal (170-240MHz) via VHF input
	
	MSG_SMS_RECEIVE_VHF_VIA_VHF_INPUT_RES = 531,	// Application: All on SMS2270
							// Enable VHF signal (170-240MHz) via VHF input
	
	//MSG_SMS_RESERVED1 = 532,			// 
													

	MSG_WR_REG_RFT_REQ   =533,			// Write value to a given RFT register
										// Format: 32-bit address of register, following header
										//		   32-bit of value, following address
										// Direction: Host->SMS

	MSG_WR_REG_RFT_RES   =534,			// Response to MSG_WR_REG_RFT_REQ message
										// Format: Status of write operation, following header
										// Direction: SMS->Host

	MSG_RD_REG_RFT_REQ   =535,			// Read the value of a given RFT register
										// Format: 32-bit address of the register, following header
										// Direction: Host->SMS

	MSG_RD_REG_RFT_RES   =536,			// Response to MSG_RD_REG_RFT_RES message
										// Format: 32-bit value of register, following header
										// Direction: SMS->Host

	MSG_RD_REG_ALL_RFT_REQ=537,			// Read all 16 RFT registers
										// Format: N/A (nothing after the header)
										// Direction: Host->SMS

	MSG_RD_REG_ALL_RFT_RES=538,			// Response to MSG_RD_REG_ALL_RFT_REQ message
										// Format: For each register, 32-bit address followed by 32-bit value (following header)
										// Direction: SMS->Host

	MSG_HELP_INT          =539,			// Internal (SmsMonitor) message
										// Format: N/A (nothing after header)
										// Direction: Host->Host

	MSG_RUN_SCRIPT_INT    =540,			// Internal (SmsMonitor) message
										// Format: Name of script(file) to run, immediately following header
										// direction: N/A

	MSG_SMS_EWS_INBAND_REQ = 541,		//	Format: 32-bit value. 1 = EWS messages are sent in-band with the data, over TS packets. 0 = EWS packets are not sent
	MSG_SMS_EWS_INBAND_RES = 542,		// 
	
	MSG_SMS_RFS_SELECT_REQ = 543,		// Application: ISDB-T on SMS2130
										// Description: select RFS resistor value (if the HW of two 60kohm paralel resistor exist)
										// Format: Data[0] = u32 GPIO number, Data[1] = u32 selected RFS value, 0: select 30kohm, 1: select 60kohm
										// Direction: Host-->FW
										 
	MSG_SMS_RFS_SELECT_RES = 544,		// Application: ISDB-T on SMS2130
										// Description: Response to MSG_SMS_RFS_SELECT_REQ
										// Direction: FW-->Host
	
	MSG_SMS_MB_GET_VER_REQ = 545,			// 
	MSG_SMS_MB_GET_VER_RES = 546,			//  
	MSG_SMS_MB_WRITE_CFGFILE_REQ = 547,		// 
	MSG_SMS_MB_WRITE_CFGFILE_RES = 548,		//
	MSG_SMS_MB_READ_CFGFILE_REQ = 549,		// 
	MSG_SMS_MB_READ_CFGFILE_RES = 550,		//
	//MSG_SMS_RESERVED1 = 551,			// 

	MSG_SMS_RD_MEM_REQ    =552,			// A request to read address in memory
										// Format: 32-bit of address, followed by 32-bit of range (following header)
										// Direction: Host->SMS

	MSG_SMS_RD_MEM_RES    =553,			// The response to MSG_SMS_RD_MEM_REQ
										// Format: 32-bit of data X range, following header
										// Direction: SMS->Host

	MSG_SMS_WR_MEM_REQ    =554,			// A request to write data to memory
										// Format:	32-bit of address
										//			32-bit of range (in bytes)
										//			32-bit of value
										// Direction: Host->SMS

	MSG_SMS_WR_MEM_RES    =555,			// Response to MSG_SMS_WR_MEM_REQ
										// Format: 32-bit of result
										// Direction: SMS->Host

	MSG_SMS_UPDATE_MEM_REQ = 556,
	MSG_SMS_UPDATE_MEM_RES = 557,
	
	MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_REQ = 558, // Application: ISDB-T on SMS2270
													// Description: A request to enable the recpetion of mode 1, 2 
													// and guard 1/32 which are disabled by default
	MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_RES = 559, // Application: ISDB-T on SMS2270
													// Description: A response to MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_REQ
													
	//MSG_SMS_RESERVED1 = 560,			//
	
	MSG_SMS_RF_TUNE_REQ=561,			// Application: CMMB, DVBT/H 
										// A request to tune to a new frequency
										// Format:	32-bit - Frequency in Hz
										//			32-bit - Bandwidth (in CMMB always use BW_8_MHZ)
										//			32-bit - Crystal (Use 0 for default, always 0 in CMMB)
										// Direction: Host->SMS

	MSG_SMS_RF_TUNE_RES=562,			// Application: CMMB, DVBT/H 
										// A response to MSG_SMS_RF_TUNE_REQ
										// In DVBT/H this only indicates that the tune request
										// was received.
										// In CMMB, the response returns after the demod has determined
										// if there is a valid CMMB transmission on the frequency
										//
										// Format:
										//	DVBT/H:
										//		32-bit Return status. Should be SMSHOSTLIB_ERR_OK.
										//	CMMB:
										//		32-bit CMMB signal status - SMSHOSTLIB_ERR_OK means that the 
										//					frequency has a valid CMMB signal
										// 
										// Direction: SMS->Host
	
	MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_REQ = 563,	// Application: ISDB-T on SMS2270
														// Description: A request to enable high mobility performance
														//
	MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_RES = 564,	// Application: ISDB-T on SMS2270
														// Description: A response to MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_REQ

	MSG_SMS_ISDBT_SB_RECEPTION_REQ = 565,	// Application: ISDB-T on SMS2270
											// Description: A request to receive independent 1seg transmission via tune to 13seg.
	//
	MSG_SMS_ISDBT_SB_RECEPTION_RES = 566,	// Application: ISDB-T on SMS2270
											// Description: A response to MSG_SMS_ISDBT_SB_RECEPTION_REQ

	MSG_SMS_GENERIC_EPROM_WRITE_REQ = 567,		//Write to EPROM.
	MSG_SMS_GENERIC_EPROM_WRITE_RES = 568,					//

	MSG_SMS_GENERIC_EPROM_READ_REQ = 569,			// A request to read from the EPROM
	MSG_SMS_GENERIC_EPROM_READ_RES = 570,			// 

	MSG_SMS_EEPROM_WRITE_REQ=571,		// A request to program the EEPROM
										// Format:	32-bit - Section status indication (0-first,running index,0xFFFFFFFF -last)
										//			32-bit - (optional) Image CRC or checksum
										//			32-bit - Total image length, in bytes, immediately following this DWORD
										//			32-bit - Actual section length, in bytes, immediately following this DWORD
										// Direction: Host->SMS

	MSG_SMS_EEPROM_WRITE_RES=572,		// The status response to MSG_SMS_EEPROM_WRITE_REQ
										// Format:	32-bit of the response
										// Direction: SMS->Host

	//MSG_SMS_RESERVED1 =573, 			// 
	MSG_SMS_CUSTOM_READ_REQ =574,			// 
	MSG_SMS_CUSTOM_READ_RES =575,			// 
	MSG_SMS_CUSTOM_WRITE_REQ =576,			// 
	MSG_SMS_CUSTOM_WRITE_RES =577,			//

	MSG_SMS_INIT_DEVICE_REQ=578,		// A request to init device
										// Format: 32-bit - device mode (DVBT,DVBH,TDMB,DAB)
										//		   32-bit - Crystal
										//		   32-bit - Clk Division
										//		   32-bit - Ref Division
										// Direction: Host->SMS

	MSG_SMS_INIT_DEVICE_RES=579,		// The response to MSG_SMS_INIT_DEVICE_REQ
										// Format:	32-bit - status
										// Direction: SMS->Host

	MSG_SMS_ATSC_SET_ALL_IP_REQ =580,			
	MSG_SMS_ATSC_SET_ALL_IP_RES =581,			

	MSG_SMS_ATSC_START_ENSEMBLE_REQ = 582,
	MSG_SMS_ATSC_START_ENSEMBLE_RES = 583,

	MSG_SMS_SET_OUTPUT_MODE_REQ	= 584,
	MSG_SMS_SET_OUTPUT_MODE_RES	= 585,

	MSG_SMS_ATSC_IP_FILTER_GET_LIST_REQ = 586,
	MSG_SMS_ATSC_IP_FILTER_GET_LIST_RES = 587,

	//MSG_SMS_RESERVED1 =588,			//

	MSG_SMS_SUB_CHANNEL_START_REQ =589,	// DAB
	MSG_SMS_SUB_CHANNEL_START_RES =590,	// DAB

	MSG_SMS_SUB_CHANNEL_STOP_REQ =591,	// DAB
	MSG_SMS_SUB_CHANNEL_STOP_RES =592,	// DAB

	MSG_SMS_ATSC_IP_FILTER_ADD_REQ = 593,
	MSG_SMS_ATSC_IP_FILTER_ADD_RES = 594,
	MSG_SMS_ATSC_IP_FILTER_REMOVE_REQ = 595,
	MSG_SMS_ATSC_IP_FILTER_REMOVE_RES = 596,
	MSG_SMS_ATSC_IP_FILTER_REMOVE_ALL_REQ = 597,
	MSG_SMS_ATSC_IP_FILTER_REMOVE_ALL_RES = 598,

	MSG_SMS_WAIT_CMD =599,				// Internal (SmsMonitor) message
										// Format: Name of script(file) to run, immediately following header
										// direction: N/A
	//MSG_SMS_RESERVED1 = 600,			// 

	MSG_SMS_ADD_PID_FILTER_REQ=601,		// Application: DVB-T/DVB-H
										// Add PID to filter list
										// Format: 32-bit PID
										// Direction: Host->SMS

	MSG_SMS_ADD_PID_FILTER_RES=602,		// Application: DVB-T/DVB-H
										// The response to MSG_SMS_ADD_PID_FILTER_REQ
										// Format:	32-bit - Status
										// Direction: SMS->Host

	MSG_SMS_REMOVE_PID_FILTER_REQ=603,	// Application: DVB-T/DVB-H
										// Remove PID from filter list
										// Format: 32-bit PID
										// Direction: Host->SMS

	MSG_SMS_REMOVE_PID_FILTER_RES=604,	// Application: DVB-T/DVB-H
										// The response to MSG_SMS_REMOVE_PID_FILTER_REQ
										// Format:	32-bit - Status
										// Direction: SMS->Host

	MSG_SMS_FAST_INFORMATION_CHANNEL_REQ=605,// Application: DAB
										     // A request for a of a Fast Information Channel (FIC)
											 // Direction: Host->SMS

	MSG_SMS_FAST_INFORMATION_CHANNEL_RES=606,// Application: DAB, ATSC M/H
										     // Forwarding of a Fast Information Channel (FIC)
											 // Format:	Sequence counter and FIC bytes with Fast Information Blocks { FIBs  as described in "ETSI EN 300 401 V1.3.3 (2001-05)":5.2.1 Fast Information Block (FIB))
											 // Direction: SMS->Host

	MSG_SMS_DAB_CHANNEL=607,			// Application: All
										// Forwarding of a played channel
										// Format:	H.264
										// Direction: SMS->Host

	MSG_SMS_GET_PID_FILTER_LIST_REQ=608,// Application: DVB-T
										// Request to get current PID filter list
										// Format: None
										// Direction: Host->SMS

	MSG_SMS_GET_PID_FILTER_LIST_RES=609,// Application: DVB-T
										// The response to MSG_SMS_GET_PID_FILTER_LIST_REQ
										// Format:	array of 32-bit of PIDs
										// Direction: SMS->Host

	MSG_SMS_POWER_DOWN_REQ = 610,		// Request from the host to the chip to enter minimal power mode (as close to zero as possible)
	MSG_SMS_POWER_DOWN_RES = 611,   	//

	MSG_SMS_ATSC_SLT_EXIST_IND = 612, 		// Application: ATSC M/H
	MSG_SMS_ATSC_NO_SLT_IND  = 613,	// Indication of SLT existence in the parade
	//MSG_SMS_RESERVED1 = 614,			//

	MSG_SMS_GET_STATISTICS_REQ=615,		// Application: DVB-T / DAB
										// Request statistics information 
										// In DVB-T uses only at the driver level (BDA)
										// Direction: Host->FW

	MSG_SMS_GET_STATISTICS_RES=616,		// Application: DVB-T / DAB
										// The response to MSG_SMS_GET_STATISTICS_REQ
										// Format:	SmsMsgStatisticsInfo_ST
										// Direction: SMS->Host

	MSG_SMS_SEND_DUMP=617,				// uses for - Dump msgs
										// Direction: SMS->Host

	MSG_SMS_SCAN_START_REQ=618,			// Application: CMMB
										// Start Scan
										// Format:
										//			32-bit - Bandwidth
										//			32-bit - Scan Flags
										//			32-bit - Param Type
										// In CMMB Param type must be 0 - because of CMRI spec, 
										//	and only range is supported.
										//
										// In other standards:
										// If Param Type is SCAN_PARAM_TABLE:
										//			32-bit - Number of frequencies N
										//			N*32-bits - List of frequencies
										// If Param Type is SCAN_PARAM_RANGE:
										//			32-bit - Start Frequency
										//			32-bit - Gap between frequencies
										//			32-bit - End Frequency
										// Direction: Host->SMS

	MSG_SMS_SCAN_START_RES=619,			// Application: CMMB
										// Scan Start Reply
										// Format:	32-bit - ACK/NACK
										// Direction: SMS->Host

	MSG_SMS_SCAN_STOP_REQ=620,			// Application: CMMB
										// Stop Scan
										// Direction: Host->SMS

	MSG_SMS_SCAN_STOP_RES=621,			// Application: CMMB
										// Scan Stop Reply
										// Format:	32-bit - ACK/NACK
										// Direction: SMS->Host

	MSG_SMS_SCAN_PROGRESS_IND=622,		// Application: CMMB
										// Scan progress indications
										// Format:
										//		32-bit RetCode: SMSHOSTLIB_ERR_OK means that the frequency is Locked
										//		32-bit Current frequency 
										//		32-bit Number of frequencies remaining for scan
										//		32-bit NetworkID of the current frequency - if locked. If not locked - 0.
										
	MSG_SMS_SCAN_COMPLETE_IND=623,		// Application: CMMB
										// Scan completed
										// Format: Same as SCAN_PROGRESS_IND

	MSG_SMS_LOG_ITEM = 624,             // Application: All
										// Format:	SMSHOSTLIB_LOG_ITEM_ST.
										// Actual size depend on the number of parameters
										// Direction: Host->SMS

	//MSG_SMS_RESERVED1  = 625,			//
	//MSG_SMS_RESERVED1  = 626,			//
	//MSG_SMS_RESERVED1 = 627,			//  

	MSG_SMS_DAB_SUBCHANNEL_RECONFIG_REQ = 628,	// Application: DAB
	MSG_SMS_DAB_SUBCHANNEL_RECONFIG_RES = 629,	// Application: DAB

	// Handover - start (630)
	MSG_SMS_HO_PER_SLICES_IND		= 630,		// Application: DVB-H 
												// Direction: FW-->Host

	MSG_SMS_HO_INBAND_POWER_IND		= 631,		// Application: DVB-H 
												// Direction: FW-->Host

	MSG_SMS_MANUAL_DEMOD_REQ		= 632,		// Application: DVB-H
												// Debug msg 
												// Direction: Host-->FW

	//MSG_SMS_HO_RESERVED1_RES		= 633,		// Application: DVB-H  
	//MSG_SMS_HO_RESERVED2_RES		= 634,		// Application: DVB-H  
	//MSG_SMS_HO_RESERVED3_RES		= 635,		// Application: DVB-H 

	MSG_SMS_HO_TUNE_ON_REQ			= 636,		// Application: DVB-H  
	MSG_SMS_HO_TUNE_ON_RES			= 637,		// Application: DVB-H  
	MSG_SMS_HO_TUNE_OFF_REQ			= 638,		// Application: DVB-H 	
	MSG_SMS_HO_TUNE_OFF_RES			= 639,		// Application: DVB-H  
	MSG_SMS_HO_PEEK_FREQ_REQ		= 640,		// Application: DVB-H 
	MSG_SMS_HO_PEEK_FREQ_RES		= 641,		// Application: DVB-H  
	MSG_SMS_HO_PEEK_FREQ_IND		= 642,		// Application: DVB-H 
	// Handover - end (642)

	MSG_SMS_MB_ATTEN_SET_REQ		= 643,		// 
	MSG_SMS_MB_ATTEN_SET_RES		= 644,		//

	//MSG_SMS_RESERVED1				= 645,		//
	//MSG_SMS_RESERVED1				= 646,		//						
	//MSG_SMS_RESERVED1				= 647,		//	
	//MSG_SMS_RESERVED1				= 648,		//

	MSG_SMS_ENABLE_STAT_IN_I2C_REQ = 649,		// Application: DVB-T (backdoor)
												// Enable async statistics in I2C polling 
												// Direction: Host->FW

	MSG_SMS_ENABLE_STAT_IN_I2C_RES = 650,		// Application: DVB-T
												// Response to MSG_SMS_ENABLE_STAT_IN_I2C_REQ
												// Format: N/A
												// Direction: FW->Host

	MSG_SMS_GET_STATISTICS_EX_REQ   = 653,		// Application: ISDBT / FM
												// Request for statistics 
												// Direction: Host-->FW

	MSG_SMS_GET_STATISTICS_EX_RES   = 654,		// Application: ISDBT / FM
												// Format:
												// 32 bit ErrCode
												// The rest: A mode-specific statistics struct starting
												// with a 32 bits type field.
												// Direction: FW-->Host

	MSG_SMS_SLEEP_RESUME_COMP_IND	= 655,		// Application: All
												// Indicates that a resume from sleep has been completed
												// Uses for Debug only
												// Direction: FW-->Host

	MSG_SMS_SWITCH_HOST_INTERFACE_REQ	= 656,		// Application: All
	MSG_SMS_SWITCH_HOST_INTERFACE_RES	= 657,		// Request the FW to switch off the current host I/F and activate a new one
													// Format: one u32 parameter in SMSHOSTLIB_COMM_TYPES_E format

	MSG_SMS_DATA_DOWNLOAD_REQ		= 660,		// Application: All
												// Direction: Host-->FW

	MSG_SMS_DATA_DOWNLOAD_RES		= 661,		// Application: All
												// Direction: FW-->Host

	MSG_SMS_DATA_VALIDITY_REQ		= 662,		// Application: All
												// Direction: Host-->FW
												
	MSG_SMS_DATA_VALIDITY_RES		= 663,		// Application: All
												// Direction: FW-->Host
												
	MSG_SMS_SWDOWNLOAD_TRIGGER_REQ	= 664,		// Application: All
												// Direction: Host-->FW
												
	MSG_SMS_SWDOWNLOAD_TRIGGER_RES	= 665,		// Application: All
												// Direction: FW-->Host

	MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ	= 666,		// Application: All
												// Direction: Host-->FW
	
	MSG_SMS_SWDOWNLOAD_BACKDOOR_RES	= 667,		// Application: All
												// Direction: FW-->Host

	MSG_SMS_GET_VERSION_EX_REQ		= 668,		// Application: All Except CMMB
												// Direction: Host-->FW

	MSG_SMS_GET_VERSION_EX_RES		= 669,		// Application: All Except CMMB
												// Direction: FW-->Host

	MSG_SMS_CLOCK_OUTPUT_CONFIG_REQ = 670,		// Application: All 
												// Request to clock signal output from SMS
												// Format: 32-bit - Enable/Disable clock signal
												//         32-bit - Requested clock frequency
												// Direction: Host-->FW

	MSG_SMS_CLOCK_OUTPUT_CONFIG_RES = 671,		// Application: All
												// Response to clock signal output config request
												// Format: 32-bit - Status
												// Direction: FW-->Host

	MSG_SMS_I2C_SET_FREQ_REQ		= 685,		// Application: All 
												// Request to start I2C configure with new clock Frequency
												// Format: 32-bit - Requested clock frequency
												// Direction: Host-->FW

	MSG_SMS_I2C_SET_FREQ_RES		= 686,		// Application: All 
												// Response to MSG_SMS_I2C_SET_FREQ_REQ
												// Format: 32-bit - Status
												// Direction: FW-->Host

	MSG_SMS_GENERIC_I2C_REQ			= 687,		// Application: All 
												// Request to write buffer through I2C
												// Format: 32-bit - device address
												//		   32-bit - write size
												//		   32-bit - requested read size
												//		   n * 8-bit - write buffer

	MSG_SMS_GENERIC_I2C_RES			= 688,		// Application: All 
												// Response to MSG_SMS_GENERIC_I2C_REQ
												// Format: 32-bit - Status
												//		   32-bit - read size
												//         n * 8-bit - read data

	//MSG_SMS_RESERVED1				= 689,		// 
	//MSG_SMS_RESERVED1				= 690,		// 
	//MSG_SMS_RESERVED1				= 691,		// 
	//MSG_SMS_RESERVED1				= 692,		// 

	MSG_SMS_DVBT_BDA_DATA			= 693,		// Application: All (BDA)
												// Direction: FW-->Host
												
	//MSG_SMS_RESERVED1				= 694,		//
	//MSG_SMS_RESERVED1				= 695,		//
	//MSG_SMS_RESERVED1				= 696,		//
	MSG_SW_RELOAD_REQ			= 697,			//Reload request
	//MSG_SMS_RESERVED1				= 698,		//

	MSG_SMS_DATA_MSG				= 699,		// Application: All
												// Direction: FW-->Host

	///  NOTE: Opcodes targeted for Stellar cannot exceed 700
	MSG_TABLE_UPLOAD_REQ			= 700,		// Request for PSI/SI tables in DVB-H
												// Format: 
												// Direction Host->SMS

	MSG_TABLE_UPLOAD_RES			= 701,		// Reply to MSG_TABLE_UPLOAD_REQ
												// Format: 
												// Direction SMS->Host

	// reload without reseting the interface
	MSG_SW_RELOAD_START_REQ			= 702,		// Request to prepare to reload 
	MSG_SW_RELOAD_START_RES			= 703,		// Response to 
	MSG_SW_RELOAD_EXEC_REQ			= 704,		// Request to start reload
	MSG_SW_RELOAD_EXEC_RES			= 705,		// Response to MSG_SW_RELOAD_EXEC_REQ

	//MSG_SMS_RESERVED1				= 706,		//
	//MSG_SMS_RESERVED1				= 707,		//
	//MSG_SMS_RESERVED1				= 708,		//
	//MSG_SMS_RESERVED1				= 709,		//

	MSG_SMS_SPI_INT_LINE_SET_REQ	= 710,		//
	MSG_SMS_SPI_INT_LINE_SET_RES	= 711,		//

	MSG_SMS_GPIO_CONFIG_EX_REQ		= 712,		//
	MSG_SMS_GPIO_CONFIG_EX_RES		= 713,		//

	//MSG_SMS_RESERVED1				= 714,		//
	//MSG_SMS_RESERVED1  			= 715,		//

	MSG_SMS_WATCHDOG_ACT_REQ		= 716,		//
	MSG_SMS_WATCHDOG_ACT_RES		= 717,		//

	MSG_SMS_LOOPBACK_REQ			= 718,		//
	MSG_SMS_LOOPBACK_RES			= 719,		//  

	MSG_SMS_RAW_CAPTURE_START_REQ	= 720,  	//
	MSG_SMS_RAW_CAPTURE_START_RES	= 721,  	//

	MSG_SMS_RAW_CAPTURE_ABORT_REQ	= 722,  	//
	MSG_SMS_RAW_CAPTURE_ABORT_RES	= 723,  	//

	//MSG_SMS_RESERVED1				=  724,  	//
	//MSG_SMS_RESERVED1				=  725,  	//
	//MSG_SMS_RESERVED1				=  726,  	// 
	//MSG_SMS_RESERVED1				=  727,  	//

	MSG_SMS_RAW_CAPTURE_COMPLETE_IND = 728, 	//

	MSG_SMS_DATA_PUMP_IND			= 729,  	// USB debug - _TEST_DATA_PUMP 
	MSG_SMS_DATA_PUMP_REQ			= 730,  	// USB debug - _TEST_DATA_PUMP 
	MSG_SMS_DATA_PUMP_RES			= 731,  	// USB debug - _TEST_DATA_PUMP 

	MSG_SMS_FLASH_DL_REQ			= 732,		// A request to program the FLASH
												// Format:	32-bit - Section status indication (0-first,running index,0xFFFFFFFF -last)
												//			32-bit - (optional) Image CRC or checksum
												//			32-bit - Total image length, in bytes, immediately following this DWORD
												//			32-bit - Actual section length, in bytes, immediately following this DWORD
												// Direction: Host->SMS

	MSG_SMS_FLASH_DL_RES			= 733,		// The status response to MSG_SMS_FLASH_DL_REQ
												// Format:	32-bit of the response
												// Direction: SMS->Host

	MSG_SMS_EXEC_TEST_1_REQ			= 734,		// USB debug - _TEST_DATA_PUMP 
	MSG_SMS_EXEC_TEST_1_RES			= 735,  	// USB debug - _TEST_DATA_PUMP 

	MSG_SMS_ENBALE_TS_INTERFACE_REQ	= 736,		// A request set TS interface as the DATA(!) output interface
												// Format:	32-bit - Requested Clock speed in Hz(0-disable)
												//			32-bit - transmission mode (Serial or Parallel)
												// Direction: Host->SMS

	MSG_SMS_ENBALE_TS_INTERFACE_RES	= 737,  	//

	MSG_SMS_SPI_SET_BUS_WIDTH_REQ	= 738,  	//
	MSG_SMS_SPI_SET_BUS_WIDTH_RES	= 739,  	//

	MSG_SMS_SEND_EMM_REQ 			= 740,  	//  Request to process Emm from API
	MSG_SMS_SEND_EMM_RES			= 741,  	//	Response to MSG_SMS_SEND_EMM_REQ

	MSG_SMS_DISABLE_TS_INTERFACE_REQ = 742, 	//
	MSG_SMS_DISABLE_TS_INTERFACE_RES = 743, 	//

	MSG_SMS_IS_BUF_FREE_REQ			= 744,    	//Request to check is CaBuf is free for EMM from API
	MSG_SMS_IS_BUF_FREE_RES			= 745,    	//Response to MSG_SMS_IS_BUF_FREE_RES

	MSG_SMS_EXT_ANTENNA_REQ			= 746,  	//Activate external antenna search algorithm 
	MSG_SMS_EXT_ANTENNA_RES			= 747,  	//confirmation 

	MSG_SMS_CMMB_GET_NET_OF_FREQ_REQ_OBSOLETE= 748,		// Obsolete
	MSG_SMS_CMMB_GET_NET_OF_FREQ_RES_OBSOLETE= 749,	    // Obsolete

	MSG_SMS_BATTERY_LEVEL_REQ		= 750,		//Request to get battery charge level
	MSG_SMS_BATTERY_LEVEL_RES		= 751,		//Response to MSG_SMS_BATTERY_LEVEL_REQ

	MSG_SMS_CMMB_INJECT_TABLE_REQ_OBSOLETE	= 752,		// Obsolete
	MSG_SMS_CMMB_INJECT_TABLE_RES_OBSOLETE	= 753,		// Obsolete
	
	MSG_SMS_FM_RADIO_BLOCK_IND		= 754,		// Application: FM_RADIO
												// Description: RDS blocks
												// Format: Data[0] = 	
												// Direction: FW-->Host

	MSG_SMS_HOST_NOTIFICATION_IND 	= 755,		// Application: CMMB
												// Description: F/W notification to host
												// Data[0]:	SMSHOSTLIB_CMMB_HOST_NOTIFICATION_TYPE_ET
												// Direction: FW-->Host

	MSG_SMS_CMMB_GET_CONTROL_TABLE_REQ_OBSOLETE	= 756,	// Obsolete
	MSG_SMS_CMMB_GET_CONTROL_TABLE_RES_OBSOLETE = 757,	// Obsolete


	//MSG_SMS_RESERVED1				= 758,	// 
	//MSG_SMS_RESERVED1				= 759,	// 

	MSG_SMS_CMMB_GET_NETWORKS_REQ	= 760,	// Data[0]: Reserved - has to be 0
	MSG_SMS_CMMB_GET_NETWORKS_RES	= 761,	// Data[0]: RetCode
											// Data[1]: Number of networks (N)
											// Followed by N * SmsCmmbNetworkInfo_ST

	MSG_SMS_CMMB_START_SERVICE_REQ	= 762,	// Data[0]: u32 Reserved 0xFFFFFFFF (was NetworkLevel)
											// Data[1]: u32 Reserved 0xFFFFFFFF (was NetworkNumber)
											// Data[2]: u32 ServiceId

	MSG_SMS_CMMB_START_SERVICE_RES	= 763,	// Data[0]: u32 RetCode
											// Data[1]: u32 ServiceHandle
											// Data[2]: u32 Service sub frame index
											//		The index of the sub frame that contains the service
											//      inside the multiplex frame. Usually 0.
											// Data[1]: u32 Service ID
											//		The started service ID 

	MSG_SMS_CMMB_STOP_SERVICE_REQ	= 764,	// Data[0]: u32 ServiceHandle
	MSG_SMS_CMMB_STOP_SERVICE_RES	= 765,	// Data[0]: u32 RetCode

	MSG_SMS_CMMB_ADD_CHANNEL_FILTER_REQ		= 768,	// Data[0]: u32 Channel ID
	MSG_SMS_CMMB_ADD_CHANNEL_FILTER_RES		= 769,	// Data[0]: u32 RetCode

	MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_REQ	= 770,	// Data[0]: u32 Channel ID
	MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_RES	= 771,	// Data[0]: u32 RetCode

	MSG_SMS_CMMB_START_CONTROL_INFO_REQ		= 772,	// Format:	
													// Data[0]: u32 Reserved 0xFFFFFFFF (was NetworkLevel)
													// Data[1]: u32 Reserved 0xFFFFFFFF (was NetworkNumber)

	MSG_SMS_CMMB_START_CONTROL_INFO_RES		= 773,	// Format:	Data[0]: u32 RetCode

	MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ		= 774,	// Format: No Payload
	MSG_SMS_CMMB_STOP_CONTROL_INFO_RES		= 775,	// Format: Data[0]: u32 RetCode

	MSG_SMS_ISDBT_TUNE_REQ			= 776,	// Application Type: ISDB-T
											// Description: A request to tune to a new frequency
											// Format:	Data[0]:	u32 Frequency
											//			Data[1]:	u32 Bandwidth
											//			Data[2]:	u32 Crystal
											//			Data[3]:	u32 Segment number
											// Direction: Host->SMS

	MSG_SMS_ISDBT_TUNE_RES			= 777,	// Application Type: ISDB-T
											// Data[0]:	u32 RetCode
											// Direction: SMS->Host

	//MSG_SMS_RESERVED1		        = 778,	// 
	//MSG_SMS_RESERVED1             = 779,	// 
	//MSG_SMS_RESERVED1				= 780,	// 
	//MSG_SMS_RESERVED1         	= 781,	// 

	MSG_SMS_TRANSMISSION_IND		= 782,  // Application Type: DVB-T/DVB-H 
											// Description: Send statistics info using the following structure:
											// TRANSMISSION_STATISTICS_ST
											//	 Data[0] = u32 Frequency																
											//   Data[1] = u32 Bandwidth				
											//   Data[2] = u32 TransmissionMode		
											//   Data[3] = u32 GuardInterval			
											//   Data[4] = u32 CodeRate				
											//   Data[5] = u32 LPCodeRate				
											//   Data[6] = u32 Hierarchy				
											//   Data[7] = u32 Constellation			
											//   Data[8] = u32 CellId					
											//   Data[9] = u32 DvbhSrvIndHP			
											//   Data[10]= u32 DvbhSrvIndLP			
											//   Data[11]= u32 IsDemodLocked			
											// Direction: FW-->Host
												
	MSG_SMS_PID_STATISTICS_IND		= 783,	// Application Type: DVB-H 
											// Description: Send PID statistics info using the following structure:
											// PID_DATA_ST
											//	 Data[0] = u32 pid
											//   Data[1] = u32 num rows 
											//   Data[2] = u32 size  
											//   Data[3] = u32 padding_cols
											//   Data[4] = u32 punct_cols
											//   Data[5] = u32 duration
											//   Data[6] = u32 cycle
											//   Data[7] = u32 calc_cycle
											//   Data[8] = u32 tot_tbl_cnt 
											//   Data[9] = u32 invalid_tbl_cnt 
											//   Data[10]= u32 tot_cor_tbl
											// Direction: FW-->Host

	MSG_SMS_POWER_DOWN_IND			= 784,	// Application Type: DVB-H 
											// Description: Indicates start of the power down to sleep mode procedure
											//  data[0] - requestId, 
											//  data[1] - message quarantine time
											// Direction: FW-->Host

	MSG_SMS_POWER_DOWN_CONF			= 785,	// Application Type: DVB-H 
											// Description: confirms the power down procedure, 
											// data[0] - requestId, 
											// data[1] - quarantine time
											// Direction: Host-->FW 

	MSG_SMS_POWER_UP_IND			= 786,	// Application Type: DVB-H 
											// Description: Indicates end of sleep mode,       
											// data[0] - requestId
											// Direction: FW-->Host

	MSG_SMS_POWER_UP_CONF			= 787,	// Application Type: DVB-H 
											// Description: confirms the end of sleep mode,    
											// data[0] - requestId
											// Direction: Host-->FW 

	//MSG_SMS_RESERVED1             = 788,	//
	//MSG_SMS_RESERVED1				= 789,	//

	MSG_SMS_POWER_MODE_SET_REQ		= 790,	// Application: DVB-H 
											// Description: set the inter slice power down (sleep) mode (Enable/Disable)
											// Format: Data[0] = u32 sleep mode
											// Direction: Host-->FW 

	MSG_SMS_POWER_MODE_SET_RES		= 791,	// Application: DVB-H
											// Description: response to the previous request
											// Direction: FW-->Host

	MSG_SMS_DEBUG_HOST_EVENT_REQ	= 792,	// Application: CMMB (Internal) 
											// Description: An opaque event host-> FW for debugging internal purposes (CMMB)
											// Format:	data[0] = Event type (enum)
											//			data[1] = Param

	MSG_SMS_DEBUG_HOST_EVENT_RES	= 793,	// Application: CMMB (Internal)
											// Description: Response. 
											// Format:  data[0] = RetCode, 
											//			data[1] = RetParam


	MSG_SMS_NEW_CRYSTAL_REQ			= 794,	// Application: All 
											// report crystal input to FW
											// Format:  data[0] = u32 crystal 
											// Direction: Host-->FW 

	MSG_SMS_NEW_CRYSTAL_RES			= 795,  // Application Type: All 
											// Response to MSG_SMS_NEW_CRYSTAL_REQ
											// Direction: FW-->Host

	MSG_SMS_CONFIG_SPI_REQ			= 796,	// Application: All 
											// Configure SPI interface (also activates I2C slave interface)
											// Format:	data[0] = SPI Controller (u32)
											//			data[1] = SPI Mode - Master/Slave (u32)
											//			data[2] = SPI Type - Mot/TI (u32)
											//			data[3] = SPI Width - 8bit/32bit (u32)
											//			data[4] = SPI Clock - in Hz (u32)
											// Direction: Host-->FW

	MSG_SMS_CONFIG_SPI_RES			= 797,	// Application: All 
											// Response to MSG_SMS_CONFIG_SPI_RES
											// Direction: FW-->Host

	MSG_SMS_I2C_SHORT_STAT_IND		= 798,	// Application Type: DVB-T/ISDB-T 
											// Format: ShortStatMsg_ST
											//		Data[0] = u16 msgType
											//		Data[1] = u8	msgSrcId
											//		Data[2] = u8	msgDstId
											//		Data[3] = u16	msgLength	
											//		Data[4] = u16	msgFlags
											//  The following parameters relevant in DVB-T only - in isdb-t should be Zero
											//		Data[5] = u32 IsDemodLocked;
											//		Data[6] = u32 InBandPwr;
											//		Data[7] = u32 BER;
											//		Data[8] = u32 SNR;
											//		Data[9] = u32 TotalTsPackets;
											//		Data[10]= u32 ErrorTSPackets;
											// Direction: FW-->Host

	MSG_SMS_START_IR_REQ			= 800,  // Application: All
											// Description: request to start sampling IR controller
											// Format: Data[0] = irController;
											//		   Data[1] = irTimeout;
											// Direction: Host-->FW

	MSG_SMS_START_IR_RES			= 801,  // Application: All
											// Response to MSG_SMS_START_IR_REQ
											// Direction: FW-->Host

	MSG_SMS_IR_SAMPLES_IND			= 802,  // Application: All
											// Send IR samples to Host
											// Format: Data[] = 128 * u32 
											// Direction: FW-->Host
	
	MSG_SMS_CMMB_CA_SERVICE_IND		= 803,	// Format:	u32 data[0] u32 Indication type, according to
											//					SmsCaServiceIndicationTypes_EN enum
											//			u32 data[1] u32 Service ID

	MSG_SMS_SLAVE_DEVICE_DETECTED	= 804,  // Application: DVB-T MRC
											// Description: FW indicate that Slave exist in MRC - DVB-T application
											// Direction: FW->Host

	MSG_SMS_INTERFACE_LOCK_IND		= 805,	// Application: All
											// Description: firmware requests that the host does not transmit anything on the interface
											// Direction: FW->Host

	MSG_SMS_INTERFACE_UNLOCK_IND	= 806,	// Application: All
											// Description: firmware signals that the host may resume transmission
											// Direction: FW->Host

	//MSG_SMS_RESERVED1				= 807,	// 
	//MSG_SMS_RESERVED1				= 808,	// 
	//MSG_SMS_RESERVED1				= 809,	//

	MSG_SMS_SEND_ROSUM_BUFF_REQ		= 810,  // Application: Rosum
											// Description: Host send buffer to Rosum internal module in FW 
											// Format: msg structure is proprietary to rosum, size can be up to 240
											// Direction: Host-->FW

	MSG_SMS_SEND_ROSUM_BUFF_RES		= 811,  // Application: Rosum
											// Response to MSG_SMS_SEND_ROSUM_BUFF_RES
											// Direction: FW->Host

	MSG_SMS_ROSUM_BUFF				= 812,  // Application: Rosum
											// Description: Rosum internal module in FW  send buffer to Host
											// Format: msg structure is proprietary to rosum, size can be up to 240
											// Direction: FW->Host

	//MSG_SMS_RESERVED1				= 813,	// 
	//MSG_SMS_RESERVED1				= 814,	// 

	MSG_SMS_SET_AES128_KEY_REQ		= 815,  // Application: ISDB-T
											// Description: Host send key for AES128
											// Format: String
											// Direction: Host-->FW

	MSG_SMS_SET_AES128_KEY_RES		= 816,  // Application: ISDB-T
											// Description: response to MSG_SMS_SET_AES128_KEY_REQ
											// Direction: FW-->Host

	MSG_SMS_MBBMS_WRITE_REQ			= 817,	// MBBMS-FW communication message - downstream
	MSG_SMS_MBBMS_WRITE_RES			= 818,	// MBBMS-FW communication message - downstream response
	MSG_SMS_MBBMS_READ_IND			= 819,	// MBBMS-FW communication message - upstream

	MSG_SMS_IQ_STREAM_START_REQ		= 820,  // Application: Streamer
	MSG_SMS_IQ_STREAM_START_RES		= 821,  // Application: Streamer
	MSG_SMS_IQ_STREAM_STOP_REQ		= 822,  // Application: Streamer
	MSG_SMS_IQ_STREAM_STOP_RES		= 823,  // Application: Streamer
	MSG_SMS_IQ_STREAM_DATA_BLOCK	= 824,  // Application: Streamer

	MSG_SMS_GET_EEPROM_VERSION_REQ  = 825,	// Request to get EEPROM version string

	MSG_SMS_GET_EEPROM_VERSION_RES  = 826,	// Response to get EEPROM version string request
											// Format: 32-bit - Status
											//         32-bit - Length of string
											//         N*bytes - EEPROM version string

	MSG_SMS_SIGNAL_DETECTED_IND		= 827,  // Application: DVB-T/ISDB-T/TDMB
											// Description: Indication on good signal - after Tune 
											// Direction: FW-->Host

	MSG_SMS_NO_SIGNAL_IND			= 828,  // Application: DVB-T/ISDB-T/TDMB
											// Description: Indication on bad signal - after Tune 
											// Direction: FW-->Host

	//MSG_SMS_RESERVED1				= 829,	//	

	MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ	= 830,	// Application: DVB-T MRC
											// Description: Power down MRC slave to save power
											// Direction: Host-->FW

	MSG_SMS_MRC_SHUTDOWN_SLAVE_RES	= 831,	// Application: DVB-T MRC
											// Description: response to MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ 
											// Direction: FW-->Host

	MSG_SMS_MRC_BRINGUP_SLAVE_REQ	= 832,	// Application: DVB-T MRC
											// Description: Return back the MRC slave to operation
											// Direction: Host-->FW

	MSG_SMS_MRC_BRINGUP_SLAVE_RES	= 833,  // Application: DVB-T MRC
											// Description: response to MSG_SMS_MRC_BRINGUP_SLAVE_REQ 
											// Direction: FW-->Host

	MSG_SMS_EXTERNAL_LNA_CTRL_REQ   = 834,  // APPLICATION: DVB-T 
											// Description: request from driver to control external LNA
											// Direction: Host-->FW

	MSG_SMS_EXTERNAL_LNA_CTRL_RES   = 835,  // APPLICATION: DVB-T 
											// Description: response to MSG_SMS_EXTERNAL_LNA_CTRL_REQ
											// Direction: FW-->Host

	MSG_SMS_SET_PERIODIC_STATISTICS_REQ		= 836,	// Application: CMMB
													// Description: Enable/Disable periodic statistics.
													// Format:	32 bit enable flag. 0 - Disable, 1- Enable 
													// Direction: Host-->FW

	MSG_SMS_SET_PERIODIC_STATISTICS_RES		= 837,  // Application: CMMB
													// Description: response to MSG_SMS_SET_PERIODIC_STATISTICS_REQ 
													// Direction: FW-->Host

	MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ	= 838,	// Application: CMMB
													// Description: Enable/Disable auto output of TS0
													// Format: 32 bit enable flag. 0 - Disable, 1- Enable 
													// Direction: Host-->FW

	MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_RES	= 839,  // Application: CMMB
													// Description: response to MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ
	MSG_SMS_MRC_MODE_CHANGE_REQ		= 840,	
	MSG_SMS_MRC_MODE_CHANGE_RES		= 841,
 
													// Direction: FW-->Host

	LOCAL_TUNE						= 850,	// Application: DVB-T (Internal)
											// Description: Internal message sent by the demod after tune/resync
											// Direction: FW-->FW	

	LOCAL_IFFT_H_ICI				= 851,  // Application: DVB-T (Internal)
											// Direction: FW-->FW

	MSG_RESYNC_REQ					= 852,	// Application: DVB-T (Internal)
											// Description: Internal resync request used by the MRC master
											// Direction: FW-->FW

	MSG_SMS_CMMB_GET_MRC_STATISTICS_REQ		= 853,	// Application: CMMB (Internal)
													// Description: MRC statistics request (internal debug, not exposed to users)
                                                    // Format 
                                                    // 32-bit   IsDemodLocked;			//!< 0 - not locked, 1 - locked
                                                    // 32-bit   SNR dB
                                                    // 32-bit   RSSI dBm
                                                    // 32-bit   InBandPwr In band power in dBM
                                                    // 32-bit   CarrierOffset Carrier Offset in Hz
													// Direction: Host-->FW
	MSG_SMS_CMMB_GET_MRC_STATISTICS_RES		= 854,	// Description: MRC statistics response (internal debug, not exposed to users)
													// Direction: FW-->Host

	MSG_SMS_LOG_EX_ITEM				= 855,  // Application: All
											// Format:	32-bit - number of log messages
											//			followed by N  SMSHOSTLIB_LOG_ITEM_ST  
											// Direction: FW-->Host

	MSG_SMS_DEVICE_DATA_LOSS_IND	= 856,  // Application: LBS
											// Description: Indication on data loss on the device level
											// Direction: FW-->Host

	MSG_SMS_MRC_WATCHDOG_TRIGGERED_IND	= 857,  // 

	MSG_SMS_USER_MSG_REQ			= 858,  // Application: All
											// Description: Data message for Data Cards internal 
											// Direction: Host-->Data card 

	MSG_SMS_USER_MSG_RES			= 859,  // Application: All 
											// Data message response from Data card to host.
											// Direction: Data card-->Host

	MSG_SMS_SMART_CARD_INIT_REQ		= 860, 	// ISO-7816 SmartCard access routines
	MSG_SMS_SMART_CARD_INIT_RES		= 861,  //
	MSG_SMS_SMART_CARD_WRITE_REQ	= 862,  //
	MSG_SMS_SMART_CARD_WRITE_RES	= 863,  //
	MSG_SMS_SMART_CARD_READ_IND		= 864,  //

	MSG_SMS_TSE_ENABLE_REQ			= 866,	// Application: DVB-T/ISDB-T 
											// Description: Send this command in case the Host wants to handle TS with Error Bit enable
											// Direction: Host-->FW

	MSG_SMS_TSE_ENABLE_RES			= 867,	// Application: DVB-T/ISDB-T 
											// Description: Response to MSG_SMS_TSE_ENABLE_REQ 
											// Direction: FW-->Host

	MSG_SMS_CMMB_GET_SHORT_STATISTICS_REQ	= 868,  // Application: CMMB
													// Description: Short statistics for CMRI standard.
													// Direction: Host-->FW
													// supported only in Venice

	MSG_SMS_CMMB_GET_SHORT_STATISTICS_RES	= 869,  // Description: Short statistics response
													// Format: SMSHOSTLIB_CMMB_SHORT_STATISTICS_ST
													// (No return code).

	MSG_SMS_LED_CONFIG_REQ			= 870,	// Application: DVB-T/ISDB-T
											// Description: uses for LED reception indication
											// Format: Data[0] = u32 GPIO number
											// Direction: Host-->FW

	MSG_SMS_LED_CONFIG_RES			= 871,	// Application: DVB-T/ISDB-T
											// Description: Response to MSG_SMS_LED_CONFIG_REQ
											// Direction: FW-->Host

	// Chen Temp for PCTV PWM FOR ANTENNA
	MSG_PWM_ANTENNA_REQ				= 872,  // antenna array reception request
	MSG_PWM_ANTENNA_RES				= 873,  // antenna array reception response
	
	MSG_SMS_CMMB_SMD_SN_REQ			= 874,  // Application: CMMB
											// Description: Get SMD serial number
											// Direction: Host-->FW
											// supported only by SMD firmware 

								
	MSG_SMS_CMMB_SMD_SN_RES			= 875,  // Application: CMMB
											// Description: Get SMD serial number response
											// Format: 
											// u32 RetCode
											// u8 SmdSerialNumber[SMS_CMMB_SMD_SN_LEN==8]

	MSG_SMS_CMMB_SET_CA_CW_REQ		= 876,  // Application: CMMB
											// Description: Set current and next CA control words 
											//	for firmware descrambler
											// Format: SMSHOSTLIB_CA_CW_PAIR_ST

	MSG_SMS_CMMB_SET_CA_CW_RES		= 877,  // Application: CMMB
											// Description: Set control words response
											// Format: u32 RetCode

	MSG_SMS_CMMB_SET_CA_SALT_REQ	= 878,  // Application: CMMB
											// Description: Set Set CA salt key for 
											// firmware descrambler
											// Format: SMSHOSTLIB_CA_SALT_ST
	MSG_SMS_CMMB_SET_CA_SALT_RES	= 879,	// Application: CMMB
											// Description: Set salt keys response
											// Format: u32 RetCode
	
	MSG_SMS_NSCD_INIT_REQ			= 880, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_INIT_RES			= 881, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_PROCESS_SECTION_REQ= 882, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_PROCESS_SECTION_RES= 883, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CREATE_OBJECT_REQ	= 884, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CREATE_OBJECT_RES	= 885, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CONFIGURE_REQ		= 886, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CONFIGURE_RES		= 887, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_SET_KEYS_REQ		= 888, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_SET_KEYS_RES		= 889, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_HEADER_REQ	= 890, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_HEADER_RES	= 891, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_DATA_REQ	= 892, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_DATA_RES	= 893, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_GET_DATA_REQ= 894, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_GET_DATA_RES= 895, //NSCD injector(Internal debug fw versions only) 


	MSG_SMS_NSCD_OPEN_SESSION_REQ	= 896, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_OPEN_SESSION_RES	= 897, //NSCD injector(Internal debug fw versions only)

    MSG_SMS_SEND_HOST_DATA_TO_DEMUX_REQ		= 898, // CMMB Data to Demux injector (Internal debug fw versions only)
    MSG_SMS_SEND_HOST_DATA_TO_DEMUX_RES		= 899, // CMMB Data to Demux injector (Internal debug fw versions only)

	MSG_LAST_MSG_TYPE				= 900  // Note: Stellar ROM limits this number to 700, other chip sets to 900


}MsgTypes_ET;

typedef struct SMSHOSTLIB_VERSIONING_S
{
	u8			Major;
	u8			Minor;
	u8			Patch;
	u8			FieldPatch;
} SMSHOSTLIB_VERSIONING_ST;

typedef struct SMSHOSTLIB_VERSION_S
{
	u16				ChipModel;		//!< e.g. 0x1102 for SMS-1102 "Nova"
	u8				Step;			//!< 0 - Step A
	u8				MetalFix;		//!< 0 - Metal 0
	u8				FirmwareId;		//!< 0xFF - ROM or see #SMSHOSTLIB_DEVICE_MODES_E
	u8				SupportedProtocols;	/*!< Bitwise OR combination of supported protocols, see #SMSHOSTLIB_DEVICE_MODES_E */
	SMSHOSTLIB_VERSIONING_ST	FwVer;			//!< Firmware version
	SMSHOSTLIB_VERSIONING_ST	RomVer;			//!< ROM version
	u8				TextLabel[34];		//!< Text label
	SMSHOSTLIB_VERSIONING_ST	RFVer;			//!< RF tuner version
	u32				PkgVer;                 //!< SMS11xx Package Version
	u32				Reserved[9];            //!< Reserved for future use
}SMSHOSTLIB_VERSION_ST;

typedef struct SmsVersionRes_S
{
	SmsMsgHdr_ST				xMsgHeader;
	SMSHOSTLIB_VERSION_ST			xVersion;
} SmsVersionRes_ST;


//! DVBT Statistics

#define	SRVM_MAX_PID_FILTERS							8

typedef struct RECEPTION_STATISTICS_S
{
	u32 IsRfLocked;				//!< 0 - not locked, 1 - locked
	u32 IsDemodLocked;			//!< 0 - not locked, 1 - locked
	u32 IsExternalLNAOn;			//!< 0 - external LNA off, 1 - external LNA on

	u32 ModemState;				//!< from SMSHOSTLIB_DVB_MODEM_STATE_ET
	s32  SNR;						//!< dB
	u32 BER;						//!< Post Viterbi BER [1E-5]
	u32 BERErrorCount;			//!< Number of erroneous SYNC bits.
	u32 BERBitCount;				//!< Total number of SYNC bits.
	u32 TS_PER;					//!< Transport stream PER, 0xFFFFFFFF indicate N/A
	u32 MFER;					//!< DVB-H frame error rate in percentage, 0xFFFFFFFF indicate N/A, valid only for DVB-H
	s32  RSSI;					//!< dBm
	s32  InBandPwr;				//!< In band power in dBM
	s32  CarrierOffset;			//!< Carrier Offset in bin/1024
	u32 ErrorTSPackets;			//!< Number of erroneous transport-stream packets
	u32 TotalTSPackets;			//!< Total number of transport-stream packets

	s32  RefDevPPM;
	s32  FreqDevHz;

	s32  MRC_SNR;					//!< dB
	s32  MRC_RSSI;				//!< dBm
	s32  MRC_InBandPwr;			//!< In band power in dBM

}RECEPTION_STATISTICS_ST;

typedef struct TRANSMISSION_STATISTICS_S
{
	u32 Frequency;				//!< Frequency in Hz
	u32 Bandwidth;				//!< Bandwidth in MHz
	u32 TransmissionMode;		//!< FFT mode carriers in Kilos
	u32 GuardInterval;			//!< Guard Interval from SMSHOSTLIB_GUARD_INTERVALS_ET
	u32 CodeRate;				//!< Code Rate from SMSHOSTLIB_CODE_RATE_ET
	u32 LPCodeRate;				//!< Low Priority Code Rate from SMSHOSTLIB_CODE_RATE_ET
	u32 Hierarchy;				//!< Hierarchy from SMSHOSTLIB_HIERARCHY_ET
	u32 Constellation;			//!< Constellation from SMSHOSTLIB_CONSTELLATION_ET

	// DVB-H TPS parameters
	u32 CellId;					//!< TPS Cell ID in bits 15..0, bits 31..16 zero; if set to 0xFFFFFFFF cell_id not yet recovered
	u32 DvbhSrvIndHP;			//!< DVB-H service indication info, bit 1 - Time Slicing indicator, bit 0 - MPE-FEC indicator
	u32 DvbhSrvIndLP;			//!< DVB-H service indication info, bit 1 - Time Slicing indicator, bit 0 - MPE-FEC indicator
	u32 IsDemodLocked;			//!< 0 - not locked, 1 - locked

}TRANSMISSION_STATISTICS_ST;


typedef struct PID_STATISTICS_DATA_S
{
	struct PID_BURST_S
	{
		u32	size;
		u32	padding_cols;
		u32	punct_cols;
		u32	duration;
		u32	cycle;
		u32	calc_cycle;
	} burst;

	u32	tot_tbl_cnt;
	u32	invalid_tbl_cnt;
	u32  tot_cor_tbl;

} PID_STATISTICS_DATA_ST;


typedef struct PID_DATA_S
{
	u32 pid;
	u32 num_rows;
	PID_STATISTICS_DATA_ST pid_statistics;

}PID_DATA_ST;


/// Statistics information returned as response for SmsHostApiGetStatistics_Req
typedef struct SMSHOSTLIB_STATISTICS_S
{
	u32 Reserved;				//!< Reserved

	// Common parameters
	u32 IsRfLocked;				//!< 0 - not locked, 1 - locked
	u32 IsDemodLocked;			//!< 0 - not locked, 1 - locked
	u32 IsExternalLNAOn;			//!< 0 - external LNA off, 1 - external LNA on

	// Reception quality
	s32  SNR;				//!< dB
	u32 BER;				//!< Post Viterbi BER [1E-5]
	u32 FIB_CRC;				//!< CRC errors percentage, valid only for DAB
	u32 TS_PER;				//!< Transport stream PER, 0xFFFFFFFF indicate N/A, valid only for DVB-T/H
	u32 MFER;		//!< DVB-H frame error rate in percentage, 0xFFFFFFFF indicate N/A, valid only for DVB-H
	s32  RSSI;				//!< dBm
	s32  InBandPwr;			//!< In band power in dBM
	s32  CarrierOffset;			//!< Carrier Offset in bin/1024

	// Transmission parameters
	u32 Frequency;				//!< Frequency in Hz
	u32 Bandwidth;				//!< Bandwidth in MHz, valid only for DVB-T/H
	u32 TransmissionMode;		//!< Transmission Mode, for DAB modes 1-4, for DVB-T/H FFT mode carriers in Kilos
	u32 ModemState;				//!< from SMSHOSTLIB_DVB_MODEM_STATE_ET , valid only for DVB-T/H
	u32 GuardInterval;			//!< Guard Interval from SMSHOSTLIB_GUARD_INTERVALS_ET, valid only for DVB-T/H
	u32 CodeRate;				//!< Code Rate from SMSHOSTLIB_CODE_RATE_ET, valid only for DVB-T/H
	u32 LPCodeRate;				//!< Low Priority Code Rate from SMSHOSTLIB_CODE_RATE_ET, valid only for DVB-T/H
	u32 Hierarchy;				//!< Hierarchy from SMSHOSTLIB_HIERARCHY_ET, valid only for DVB-T/H
	u32 Constellation;			//!< Constellation from SMSHOSTLIB_CONSTELLATION_ET, valid only for DVB-T/H

	// Burst parameters, valid only for DVB-H
	u32 BurstSize;				//!< Current burst size in bytes, valid only for DVB-H
	u32 BurstDuration;			//!< Current burst duration in mSec, valid only for DVB-H
	u32 BurstCycleTime;			//!< Current burst cycle time in mSec, valid only for DVB-H
	u32 CalculatedBurstCycleTime;//!< Current burst cycle time in mSec, as calculated by demodulator, valid only for DVB-H
	u32 NumOfRows;				//!< Number of rows in MPE table, valid only for DVB-H
	u32 NumOfPaddCols;			//!< Number of padding columns in MPE table, valid only for DVB-H
	u32 NumOfPunctCols;			//!< Number of puncturing columns in MPE table, valid only for DVB-H
	u32 ErrorTSPackets;			//!< Number of erroneous transport-stream packets
	u32 TotalTSPackets;			//!< Total number of transport-stream packets
	u32 NumOfValidMpeTlbs;		//!< Number of MPE tables which do not include errors after MPE RS decoding
	u32 NumOfInvalidMpeTlbs;		//!< Number of MPE tables which include errors after MPE RS decoding
	u32 NumOfCorrectedMpeTlbs;	//!< Number of MPE tables which were corrected by MPE RS decoding
	// Common params
	u32 BERErrorCount;			//!< Number of errornous SYNC bits.
	u32 BERBitCount;				//!< Total number of SYNC bits.

	// Interface information
	u32 SmsToHostTxErrors;		//!< Total number of transmission errors.

	// DAB/T-DMB
	u32 PreBER; 					//!< DAB/T-DMB only: Pre Viterbi BER [1E-5]

	// DVB-H TPS parameters
	u32 CellId;	//!< TPS Cell ID in bits 15..0, bits 31..16 zero; if set to 0xFFFFFFFF cell_id not yet recovered
	u32 DvbhSrvIndHP;	//!< DVB-H service indication info, bit 1 - Time Slicing indicator, bit 0 - MPE-FEC indicator
	u32 DvbhSrvIndLP;	//!< DVB-H service indication info, bit 1 - Time Slicing indicator, bit 0 - MPE-FEC indicator

	u32 NumMPEReceived;			//!< DVB-H, Num MPE section received

	u32 ErrorsCounter;			//fw errors counter
	u8  ErrorsHistory[8];	//fw errors
	u32 ReservedFields[7];		//!< Reserved

} SMSHOSTLIB_STATISTICS_ST;


// Helper struct for ISDB-T statistics
typedef struct SMSHOSTLIB_ISDBT_LAYER_STAT_S
{
	// Per-layer information
	u32 CodeRate;			//!< Code Rate from SMSHOSTLIB_CODE_RATE_ET, 255 means layer does not exist
	u32 Constellation;		//!< Constellation from SMSHOSTLIB_CONSTELLATION_ET, 255 means layer does not exist
	u32 BER;					//!< Post Viterbi BER [1E-5], 0xFFFFFFFF indicate N/A
	u32 BERErrorCount;		//!< Post Viterbi Error Bits Count
	u32 BERBitCount;			//!< Post Viterbi Total Bits Count
	u32 PreBER; 				//!< Pre Viterbi BER [1E-5], 0xFFFFFFFF indicate N/A
	u32 TS_PER;				//!< Transport stream PER [%], 0xFFFFFFFF indicate N/A
	u32 ErrorTSPackets;		//!< Number of erroneous transport-stream packets
	u32 TotalTSPackets;		//!< Total number of transport-stream packets
	u32 TILdepthI;			//!< Time interleaver depth I parameter, 255 means layer does not exist
	u32 NumberOfSegments;	//!< Number of segments in layer A, 255 means layer does not exist
	u32 TMCCErrors;			//!< TMCC errors
} SMSHOSTLIB_ISDBT_LAYER_STAT_ST;

// Statistics information returned as response for SmsHostApiGetStatisticsEx_Req for ISDB-T applications, SMS1100 and up
typedef struct SMSHOSTLIB_STATISTICS_ISDBT_S
{
	u32 StatisticsType;			//!< Enumerator identifying the type of the structure.  Values are the same as SMSHOSTLIB_DEVICE_MODES_E
	//!< This field MUST always first in any statistics structure

	u32 FullSize;				//!< Total size of the structure returned by the modem.  If the size requested by
	//!< the host is smaller than FullSize, the struct will be truncated

	// Common parameters
	u32 IsRfLocked;				//!< 0 - not locked, 1 - locked
	u32 IsDemodLocked;			//!< 0 - not locked, 1 - locked
	u32 IsExternalLNAOn;			//!< 0 - external LNA off, 1 - external LNA on

	// Reception quality
	s32  SNR;						//!< dB
	s32  RSSI;					//!< dBm
	s32  InBandPwr;				//!< In band power in dBM
	s32  CarrierOffset;			//!< Carrier Offset in Hz

	// Transmission parameters
	u32 Frequency;				//!< Frequency in Hz
	u32 Bandwidth;				//!< Bandwidth in MHz
	u32 TransmissionMode;		//!< ISDB-T transmission mode
	u32 ModemState;				//!< 0 - Acquisition, 1 - Locked
	u32 GuardInterval;			//!< Guard Interval, 1 divided by value
	u32 SystemType;				//!< ISDB-T system type (ISDB-T / ISDB-Tsb)
	u32 PartialReception;		//!< TRUE - partial reception, FALSE otherwise
	u32 NumOfLayers;				//!< Number of ISDB-T layers in the network
	u32 SegmentNumber;			//!< Segment number for ISDB-Tsb
	u32 TuneBW;					//!< Tuned bandwidth - BW_ISDBT_1SEG / BW_ISDBT_3SEG

	// Per-layer information
	// Layers A, B and C
	SMSHOSTLIB_ISDBT_LAYER_STAT_ST	LayerInfo[3];	//!< Per-layer statistics, see SMSHOSTLIB_ISDBT_LAYER_STAT_ST

	// Interface information
	u32 Reserved1;				// Was SmsToHostTxErrors - obsolete .

	// Proprietary information	
	u32 ExtAntenna;				// Obsolete field.

	u32 ReceptionQuality;

	// EWS
	u32 EwsAlertActive;			//!< Signals if EWS alert is currently on

	// LNA on/off					//!< Internal LNA state: 0: OFF, 1: ON
	u32 LNAOnOff;
	
	// RF AGC Level					// !< RF AGC level [linear units], full gain = 65535 (20dB)
	u32 RfAgcLevel;

	// BB AGC Level
	u32 BbAgcLevel;				// !< Baseband AGC level [linear units], full gain = 65535 (71.5dB)

	u32 FwErrorsCounter;			// !< FW Application errors - should be always zero
	u8 FwErrorsHistoryArr[8];	// !< Last FW errors IDs - first is most recent, last is oldest
									// !< This field was ExtAntenna, and was not used
	s32  MRC_SNR;					// !< dB
	u32 SNRFullRes;				// !< dB x 65536
	u32 Reserved4[4];			

} SMSHOSTLIB_STATISTICS_ISDBT_ST;



typedef struct SmsTsEnable_S
{
	struct SmsMsgHdr_S	xMsgHeader;
	u32			TsClock;                // TS Clock Speed in Hz
	u32			eTsiMode;               // TS Mode of operation Serial (on SDIO or HIF Pins), or Parallel
	u32		    eTsiSignals;            // Level of Valid, Sync and Error signals when active
	u32			nTsiPcktDelay;          // number of delay bytes between TS packets (for 204bytes mode set to 16)
	u32		    eTsClockPolarity;		// Clock edge to sample data
	u32		    TsBitOrder;				// Bit order in TS output
    u32         EnableControlOverTs;    // Enable Control messages over TS interface
    u32         TsiEncapsulationFormat; // TS encapsulation method
	u32			TsiPaddingPackets;		// Number of TS padding packets appended to control messages
} SmsTsEnable_ST;

typedef enum SmsTsiMode_E
{
	TSI_SERIAL_ON_SDIO
}SmsTsiMode_ET;

typedef enum SmsTsiFormat_E
{
	TSI_TRANSPARENT,
	TSI_ENCAPSULATED,
	TSI_MAX_FORMAT
}SmsTsiFormat_ET;

typedef enum SmsTsiErrActive_E
{
	TSI_ERR_NOT_ACTIVE,
	TSI_ERR_ACTIVE,
	TSI_MAX_ERR_ACTIVE
}SmsTsiErrActive_ET;

typedef enum SmsTsiSigActive_E
{
	TSI_SIGNALS_ACTIVE_LOW,
	TSI_SIGNALS_ACTIVE_HIGH,
	TSI_MAX_SIG_ACTIVE
}SmsTsiSigActive_ET;

typedef enum SmsTsiClockKeepGo_E
{
	TSI_CLK_STAY_LOW_GO_NO_PKT,
	TSI_CLK_KEEP_GO_NO_PKT,
	TSI_MAX_CLK_ON
}SmsTsiClockKeepGo_ET;

typedef enum SmsTsiSensPolar_E
{
	TSI_SIG_OUT_FALL_EDGE,
	TSI_SIG_OUT_RISE_EDGE,
	TSI_MAX_CLK_POLAR
}SmsTsiSensPolar_ET;

typedef enum SmsTsiBitOrder_E
{
	TSI_BIT7_IS_MSB,
	TSI_BIT0_IS_MSB,
	TSI_MAX_BIT_ORDER
}SmsTsiBitOrder_ET;


/************************************************************************/
/* Defines, types and structures for siano core device driver		*/
/************************************************************************/

#define SMS_MAX_PAYLOAD_SIZE		240

typedef struct SmsDataDownload_S
{
	SmsMsgHdr_ST		xMsgHeader;
	u32			MemAddr;
	u32			Payload[SMS_MAX_PAYLOAD_SIZE/4];
} SmsDataDownload_ST;


#define kmutex_init(_p_) mutex_init(_p_)
#define kmutex_lock(_p_) mutex_lock(_p_)
#define kmutex_trylock(_p_) mutex_trylock(_p_)
#define kmutex_unlock(_p_) mutex_unlock(_p_)

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define SMS_PROTOCOL_MAX_RAOUNDTRIP_MS			(10000)
#define SMS_ALLOC_ALIGNMENT				128
#define SMS_DMA_ALIGNMENT				16
#define SMS_ALIGN_ADDRESS(addr) \
	((((uintptr_t)(addr)) + (SMS_DMA_ALIGNMENT-1)) & ~(SMS_DMA_ALIGNMENT-1))

#define SMS_DEVICE_FAMILY1				0
#define SMS_DEVICE_FAMILY2				1
#define SMS_ROM_NO_RESPONSE				2
#define SMS_DEVICE_NOT_READY				0x8000000

#if defined(CONFIG_FW_LOADER) || (defined(CONFIG_FW_LOADER_MODULE) && defined(MODULE))
//#define REQUEST_FIRMWARE_SUPPORTED
#define DEFAULT_FW_FILE_PATH "/etc/firmware"	// by kook
#else
#define DEFAULT_FW_FILE_PATH "/etc/firmware"
#endif

#define DVBT_BDA_CONTROL_MSG_ID				201

struct smscore_gpio_config {
#define SMS_GPIO_DIRECTION_INPUT  0
#define SMS_GPIO_DIRECTION_OUTPUT 1
	u8 direction;

#define SMS_GPIO_PULLUPDOWN_NONE     0
#define SMS_GPIO_PULLUPDOWN_PULLDOWN 1
#define SMS_GPIO_PULLUPDOWN_PULLUP   2
#define SMS_GPIO_PULLUPDOWN_KEEPER   3
	u8 pull_up_down;

#define SMS_GPIO_INPUTCHARACTERISTICS_NORMAL  0
#define SMS_GPIO_INPUTCHARACTERISTICS_SCHMITT 1
	u8 input_characteristics;

#define SMS_GPIO_OUTPUTSLEWRATE_SLOW		0 /* 10xx */
#define SMS_GPIO_OUTPUTSLEWRATE_FAST		1 /* 10xx */

#define SMS_GPIO_OUTPUTSLEWRATE_0_45_V_NS	0 /* 11xx */
#define SMS_GPIO_OUTPUTSLEWRATE_0_9_V_NS	1 /* 11xx */
#define SMS_GPIO_OUTPUTSLEWRATE_1_7_V_NS	2 /* 11xx */
#define SMS_GPIO_OUTPUTSLEWRATE_3_3_V_NS	3 /* 11xx */
	u8 output_slew_rate;

#define SMS_GPIO_OUTPUTDRIVING_S_4mA		0 /* 10xx */
#define SMS_GPIO_OUTPUTDRIVING_S_8mA		1 /* 10xx */
#define SMS_GPIO_OUTPUTDRIVING_S_12mA		2 /* 10xx */
#define SMS_GPIO_OUTPUTDRIVING_S_16mA		3 /* 10xx */

#define SMS_GPIO_OUTPUTDRIVING_1_5mA		0 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_2_8mA		1 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_4mA			2 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_7mA			3 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_10mA			4 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_11mA			5 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_14mA			6 /* 11xx */
#define SMS_GPIO_OUTPUTDRIVING_16mA			7 /* 11xx */
	u8 output_driving;
};

#define SMS_INIT_MSG_EX(ptr, type, src, dst, len)  \
	(ptr)->msgType = type; (ptr)->msgSrcId = src; (ptr)->msgDstId = dst; \
	(ptr)->msgLength = len; (ptr)->msgFlags = 0; 

#define SMS_INIT_MSG(ptr, type, len) \
	SMS_INIT_MSG_EX(ptr, type, 0, HIF_TASK, len)


struct SmsFirmware_ST {
	u32 CheckSum;
	u32 Length;
	u32 StartAddress;
	u8 Payload[1];
};


enum sms_device_type_st {
	SMS_UNKNOWN_TYPE = -1,
	SMS_STELLAR = 0,
	SMS_NOVA_A0,
	SMS_NOVA_B0,
	SMS_VEGA,
	SMS_VENICE,
	SMS_MING,
	SMS_PELE,
	SMS_RIO,
	SMS_DENVER_1530,
	SMS_DENVER_2160,
	SMS_QING,
	SMS_ZICO,
	SMS_SANTOS,
	SMS_NUM_OF_DEVICE_TYPES
};

enum sms_power_mode_st {
	SMS_POWER_MODE_ACTIVE,
	SMS_POWER_MODE_SUSPENDED
};

struct smscore_client_t;
struct smscore_buffer_t;

typedef int (*hotplug_t)(void *coredev,
			 struct device *device, int arrival);

typedef int (*powermode_t)(enum sms_power_mode_st mode);

typedef int (*sendrequest_t)(void *context, void *buffer, size_t size);
typedef int (*loadfirmware_t)(void *context, void *buffer, size_t size);
typedef int (*preload_t)(void *context);
typedef int (*postload_t)(void *context);


typedef int (*onresponse_t)(void *context, struct smscore_buffer_t *cb);
typedef void (*onremove_t)(void *context);

struct smsmdtv_version_t {
	int major;
	int minor;
	int revision;
};

struct smscore_buffer_t {
	/* public members, once passed to clients can be changed freely */
	struct list_head entry;
	int size;
	int offset;

	/* private members, read-only for clients */
	void *p;
	dma_addr_t phys;
	unsigned long offset_in_common;
};

struct smsdevice_params_t {
	struct device *device;

	int buffer_size;
	int num_buffers;
	int require_node_buffer;

	char devpath[32];
	unsigned long flags;

	loadfirmware_t loadfirmware_handler;
	sendrequest_t sendrequest_handler;
	preload_t preload_handler;
	postload_t postload_handler;


	void *context;
	enum sms_device_type_st device_type;
};

struct smsclient_params_t {
	int initial_id;
	int data_type;
	onresponse_t onresponse_handler;
	onremove_t onremove_handler;
	void *context;
};

struct sms_properties_t {
	int mode;
	int num_of_tuners;
	int board_id;
};

struct sms_firmware_t {
	char file_name[64];
	u32 fw_buf_size;
	char data[1];
};

extern struct sms_firmware_t *sms_static_firmware;

extern void smscore_registry_setmode(char *devpath, int mode);
extern int smscore_registry_getmode(char *devpath);

extern int smscore_register_hotplug(hotplug_t hotplug);
extern void smscore_unregister_hotplug(hotplug_t hotplug);

extern int smscore_register_device(struct smsdevice_params_t *params,void **coredev);
extern void smscore_unregister_device(void *coredev);

extern int smscore_start_device(void *coredev);

extern int smscore_set_device_mode(void *coredev, int mode);
extern int smscore_get_device_mode(void *coredev);

extern int smscore_configure_board(void *coredev);

extern int smscore_register_client(void *coredev,
		struct smsclient_params_t *params,
		struct smscore_client_t **client);
extern void smscore_unregister_client(struct smscore_client_t *client);

extern int smsclient_sendrequest(struct smscore_client_t *client, void *buffer,
		size_t size);
extern void smscore_onresponse(void *coredev,
		struct smscore_buffer_t *cb);


extern int smscore_get_common_buffer_size(void *coredev);
extern int smscore_map_common_buffer(void *coredev,
		struct vm_area_struct *vma);
extern char *smscore_get_fw_filename(void *coredev, int mode, int lookup);

extern int smscore_send_fw_file(void *coredev, u8 *ufwbuf,
		int size);

extern int smscore_send_fw_chunk(void *coredev,
		void *buffer, size_t size);
extern int smscore_send_last_fw_chunk(void *coredev,
		void *buffer, size_t size);
		
extern int smscore_register_client(void *coredev,
		struct smsclient_params_t *params,
		struct smscore_client_t **client);
extern void smscore_unregister_client(struct smscore_client_t *client);

extern int smsclient_sendrequest(struct smscore_client_t *client, void *buffer,
		size_t size);

extern int smscore_register_device(struct smsdevice_params_t *params,
		void **coredev);

extern void smscore_onresponse(void *coredev,
			       struct smscore_buffer_t *cb);

extern int smscore_get_common_buffer_size(void *coredev);
extern int smscore_map_common_buffer(void *coredev,
				      struct vm_area_struct *vma);
extern int smscore_send_fw_file(void *coredev,
				u8 *ufwbuf, int size);

extern struct smscore_buffer_t *smscore_getbuffer(
		void *coredev);
extern void smscore_putbuffer(void *coredev,
		struct smscore_buffer_t *cb);

int smscore_gpio_configure(void *coredev, u8 PinNum,
		struct smscore_gpio_config *pGpioConfig);
int smscore_gpio_set_level(void *coredev, u8 PinNum,
		u8 NewLevel);
int smscore_gpio_get_level(void *coredev, u8 PinNum,
		u8 *level);

void smscore_set_board_id(void *core, int id);
int smscore_led_state(void *core, int led);

int smscore_set_power_mode(enum sms_power_mode_st mode);

int smscore_register_power_mode_handler(powermode_t powermode_handler);

int smscore_un_register_power_mode_handler(void);

int smscore_get_device_properties(void* dev, struct sms_properties_t *properties);

int smscore_set_fw_file(void* dev, void *buffer, size_t size) ;
	


#ifdef SMS_HOSTLIB_SUBSYS
extern int smschar_register(void);
extern void smschar_unregister(void);
#endif

#ifdef SMS_NET_SUBSYS
extern int smsnet_register(void);
extern void smsnet_unregister(void);
#endif

#ifdef SMS_DVB3_SUBSYS
extern int smsdvb_register(void);
extern void smsdvb_unregister(void);
#endif

#ifdef SMS_USB_DRV
extern int smsusb_register(void);
extern void smsusb_unregister(void);
#endif

#ifdef SMS_SDIO_DRV
extern int smssdio_register(void);
extern void smssdio_unregister(void);
#endif

#ifdef SMS_SPI_DRV
extern int smsspi_register(void);

extern void smsspi_unregister(void);
#endif

#ifdef SMS_I2C_DRV
extern int smsi2c_register(void);
extern void smsi2c_unregister(void);
#endif

/* ------------------------------------------------------------------------ */

extern int sms_debug;

#define DBG_INFO 1
#define DBG_ADV  2

#define sms_printk(kern, fmt, arg...) \
	printk(kern "%s: " fmt "\n", __func__, ##arg)

#define dprintk(kern, lvl, fmt, arg...) do {\
	if (sms_debug & lvl) \
		sms_printk(kern, fmt, ##arg); } while (0)

#define sms_log(fmt, arg...) sms_printk(KERN_INFO, fmt, ##arg)
#define sms_err(fmt, arg...) \
	sms_printk(KERN_ERR, "line: %d: " fmt, __LINE__, ##arg)
#define sms_warn(fmt, arg...)  sms_printk(KERN_WARNING, fmt, ##arg)
#define sms_info(fmt, arg...) \
	dprintk(KERN_INFO, DBG_INFO, fmt, ##arg)
#define sms_debug(fmt, arg...) \
	dprintk(KERN_DEBUG, DBG_ADV, fmt, ##arg)


#endif /* __SMS_CORE_API_H__ */
