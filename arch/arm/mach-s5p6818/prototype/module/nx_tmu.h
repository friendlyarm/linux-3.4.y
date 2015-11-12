#ifndef _NX_TMU_H
#define _NX_TMU_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

/******************************************************************************/
/******************** REGISTER BIT MASKS **************************************/
/******************************************************************************/
 
/* For UCON_REG Register */ 
//#define DISABLE_TRANSMIT		0x0
//#define POLLING_MODE_TRANSMIT	0x1
//#define DMA_MODE_TRANSMIT		0x2
//#define DISABLE_RECIEVE			0x0
//#define POLLING_MODE_RECIEVE	0x1
//#define DMA_MODE_RECIEVE		0x2

//------------------------------------------------------------------------------
/// @brief  TMU register set structure
//------------------------------------------------------------------------------
struct NX_TMU_RegisterSet
{

	volatile U32	TRIMINFO           ;	// 0x00	// E-fuse infomation for trimming sensor data
	volatile U32	RESERVED_0[3]      ;	// 0x04~0x0C 
	volatile U32	TRIMINFO_CONFIG    ;	// 0x10	// TrimInfo configure register (not used)
	volatile U32	TRIMINFO_CONTROL   ;	// 0x14	// TrimInfo control register (not used)
	volatile U32	RESERVED_1[2]      ;	// 0x18~0x1C 
	volatile U32	TMU_CONTROL        ;	// 0x20	// TMU control register
	volatile U32	TMU_CONTROL1       ;	// 0x24	// TMU control register1
	volatile U32	TMU_STATUS         ;	// 0x28	// TMU status register
	volatile U32	SAMPLING_INTERVAL  ;	// 0x2C	// TMU sampling interval control between adjacent sampling points
	volatile U32	COUNTER_VALUE0     ;	// 0x30	// Set time to control EN_TEMP_SEN_ON/OFF
	volatile U32	COUNTER_VALUE1     ;	// 0x34	// Set time to control CLK_SENSE_ON/OFF
	volatile U32	RESERVED_2[2]      ;	// 0x38~0x3C 
	volatile U32	CURRENT_TEMP0      ;	// 0x40	// Current temperature information for internal probe
	volatile U32	CURRENT_TEMP1      ;	// 0x44	// Current temperature information for remote probe
	volatile U32	RESERVED_3[2]      ;	// 0x48~0x4C 
	volatile U32	THRESHOLD_TEMP_RISE;	// 0x50	// Threshold for temperature rising
	volatile U32	THRESHOLD_TEMP_FALL;	// 0x54	// Threshold for temperature falling
	volatile U32	THRESHOLD_TQ       ;	// 0x58	// ???
	volatile U32	TQ_STATUS          ;	// 0x5C	// ???
	volatile U32	P0_PAST_TEMP3_0    ;	// 0x60	// Probe #0 Past temperature 3_0   for tracing temperature
	volatile U32	P0_PAST_TEMP7_4    ;	// 0x64	// Probe #0 Past temperature 7_4   for tracing temperature
	volatile U32	P0_PAST_TEMP11_8   ;	// 0x68	// Probe #0 Past temperature 11_8  for tracing temperature
	volatile U32	P0_PAST_TEMP15_12  ;	// 0x6C	// Probe #0 Past temperature 15_12 for tracing temperature
	volatile U32	P1_PAST_TEMP3_0    ;	// 0x70	// Probe #1 Past temperature 3_0   for tracing temperature
	volatile U32	P1_PAST_TEMP7_4    ;	// 0x74	// Probe #1 Past temperature 7_4   for tracing temperature
	volatile U32	P1_PAST_TEMP11_8   ;	// 0x78	// Probe #1 Past temperature 11_8  for tracing temperature
	volatile U32	P1_PAST_TEMP15_12  ;	// 0x7C	// Probe #1 Past temperature 15_12 for tracing temperature
	volatile U32	P2_PAST_TEMP3_0    ;	// 0x80	// Probe #2 Past temperature 3_0   for tracing temperature
	volatile U32	P2_PAST_TEMP7_4    ;	// 0x84	// Probe #2 Past temperature 7_4   for tracing temperature
	volatile U32	P2_PAST_TEMP11_8   ;	// 0x88	// Probe #2 Past temperature 11_8  for tracing temperature
	volatile U32	P2_PAST_TEMP15_12  ;	// 0x8C	// Probe #2 Past temperature 15_12 for tracing temperature
	volatile U32	P3_PAST_TEMP3_0    ;	// 0x90	// Probe #3 Past temperature 3_0   for tracing temperature
	volatile U32	P3_PAST_TEMP7_4    ;	// 0x94	// Probe #3 Past temperature 7_4   for tracing temperature
	volatile U32	P3_PAST_TEMP11_8   ;	// 0x98	// Probe #3 Past temperature 11_8  for tracing temperature
	volatile U32	P3_PAST_TEMP15_12  ;	// 0x9C	// Probe #3 Past temperature 15_12 for tracing temperature
	volatile U32	P4_PAST_TEMP3_0    ;	// 0xA0	// Probe #4 Past temperature 3_0   for tracing temperature
	volatile U32	P4_PAST_TEMP7_4    ;	// 0xA4	// Probe #4 Past temperature 7_4   for tracing temperature
	volatile U32	P4_PAST_TEMP11_8   ;	// 0xA8	// Probe #4 Past temperature 11_8  for tracing temperature
	volatile U32	P4_PAST_TEMP15_12  ;	// 0xAC	// Probe #4 Past temperature 15_12 for tracing temperature
	volatile U32	P0_INTEN           ;	// 0xB0	// P0_Interrupt enable register
	volatile U32	P0_INTSTAT         ;	// 0xB4	// P0_Interrupt status register
	volatile U32	P0_INTCLEAR        ;	// 0xB8	// P0_Interrupt clear register
	volatile U32	RESERVED_4         ;	// 0xBC 
	volatile U32	P1_INTEN           ;	// 0xC0	// P1_Interrupt enable register
	volatile U32	P1_INTSTAT         ;	// 0xC4	// P1_Interrupt status register
	volatile U32	P1_INTCLEAR        ;	// 0xC8	// P1_Interrupt clear register
	volatile U32	RESERVED_5         ;	// 0xCC 
	volatile U32	P2_INTEN           ;	// 0xD0	// P2_Interrupt enable register
	volatile U32	P2_INTSTAT         ;	// 0xD4	// P2_Interrupt status register
	volatile U32	P2_INTCLEAR        ;	// 0xD8	// P2_Interrupt clear register
	volatile U32	RESERVED_6         ;	// 0xDC 
	volatile U32	P3_INTEN           ;	// 0xE0	// P3_Interrupt enable register
	volatile U32	P3_INTSTAT         ;	// 0xE4	// P3_Interrupt status register
	volatile U32	P3_INTCLEAR        ;	// 0xE8	// P3_Interrupt clear register
	volatile U32	RESERVED_7         ;	// 0xEC 
	volatile U32	P4_INTEN           ;	// 0xF0	// P4_Interrupt enable register
	volatile U32	P4_INTSTAT         ;	// 0xF4	// P4_Interrupt status register
	volatile U32	P4_INTCLEAR        ;	// 0xF8	// P4_Interrupt clear register
	volatile U32	RESERVED_8         ;	// 0xFC 
	volatile U32	EMUL_CON           ;	// 0x100// Emulation control register
};

typedef enum
{
    NX_TMU_INT_RISE0    = 0,
    NX_TMU_INT_RISE1    = 4,
    NX_TMU_INT_RISE2    = 8,
    NX_TMU_INT_FALLING0 = 16,
    NX_TMU_INT_FALLING1 = 20,
    NX_TMU_INT_FALLING2 = 24,

} NX_TMU_INT_MASK;

typedef enum
{
    NX_TMU_TRIP_ONLYCURTEMP = 0,
    NX_TMU_TRIP_TEMP3_0     = 4,
    NX_TMU_TRIP_TEMP7_4     = 5,
    NX_TMU_TRIP_TEMP11_8    = 6,
    NX_TMU_TRIP_TEMP15_12   = 7
    
} NX_TMU_TRIPMODE ;
    

//---------------- TRIMINFO enum ----------------//
//typedef enum
//{
//	NX_TMU_TRIMINFO_85	= 0UL<<6, 
//	NX_TMU_TRIMINFO_25	= 1UL<<6 
//};

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TMU_Initialize( void );
U32		NX_TMU_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TMU_GetPhysicalAddress( U32 ModuleIndex );

//U32		NX_TMU_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
U32		NX_TMU_GetNumberOfReset( void );
U32		NX_TMU_GetSizeOfRegisterSet( void );
void	NX_TMU_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_TMU_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_TMU_OpenModule( U32 ModuleIndex );
CBOOL	NX_TMU_CloseModule( U32 ModuleIndex );
CBOOL	NX_TMU_CheckBusy( U32 ModuleIndex );

CBOOL	NX_TMU_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_TMU_GetInterruptNumber( U32 ModuleIndex );
void	NX_TMU_SetInterruptEnable		( U32 ModuleIndex, NX_TMU_INT_MASK IntNum, CBOOL Enable );
//CBOOL 	NX_TMU_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL   NX_TMU_GetInterruptPending( U32 ModuleIndex, NX_TMU_INT_MASK IntNum );
void    NX_TMU_ClearInterruptPending( U32 ModuleIndex, NX_TMU_INT_MASK IntNum );

void  	NX_TMU_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL 	NX_TMU_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL 	NX_TMU_GetInterruptPendingAll( U32 ModuleIndex );
void  	NX_TMU_ClearInterruptPendingAll( U32 ModuleIndex );
//S32   	NX_TMU_GetInterruptPendingNumber( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{

U32	    NX_TMU_GetTriminfo85	( U32 ModuleIndex );
U32	    NX_TMU_GetTriminfo25	( U32 ModuleIndex );


U32		NX_TMU_IsBusy	( U32 ModuleIndex );


void	NX_TMU_SetThresholdLevelRise0	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelRise0	( U32 ModuleIndex );
void	NX_TMU_SetThresholdLevelRise1	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelRise1	( U32 ModuleIndex );
void	NX_TMU_SetThresholdLevelRise2	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelRise2	( U32 ModuleIndex );
void	NX_TMU_SetThresholdLevelRise3	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelRise3	( U32 ModuleIndex );

void	NX_TMU_SetThresholdLevelFalling0	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelFalling0	( U32 ModuleIndex );
void	NX_TMU_SetThresholdLevelFalling1	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelFalling1	( U32 ModuleIndex );
void	NX_TMU_SetThresholdLevelFalling2	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelFalling2	( U32 ModuleIndex );
void	NX_TMU_SetThresholdLevelFalling3	( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetThresholdLevelFalling3	( U32 ModuleIndex );

void	NX_TMU_SetTmuTripMode	( U32 ModuleIndex, NX_TMU_TRIPMODE TripMode );	// TMU_CONTROL[15:13]
void	NX_TMU_SetTmuTripEn		( U32 ModuleIndex, U32 value );	// TMU_CONTROL[12]

void	NX_TMU_SetBufSlopeSel		( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetBufSlopeSel	    ( U32 ModuleIndex )	;

void	NX_TMU_SetVRefVBESel		( U32 ModuleIndex, U32 value );
U32     NX_TMU_GetVRefVBESel	    ( U32 ModuleIndex );



void	NX_TMU_SetTmuStart		( U32 ModuleIndex, CBOOL value );	// TMU_CONTROL[0]

void	NX_TMU_SetTmuEmulEn		( U32 ModuleIndex, CBOOL value );	// EMUL_CON[0]
U32	    NX_TMU_GetTmuEmulEn		( U32 ModuleIndex );


void	NX_TMU_SetTmuNextTime	( U32 ModuleIndex, U16 value );	// EMUL_CON[31:16]
void	NX_TMU_SetTmuNextData	( U32 ModuleIndex, U32 value );	// EMUL_CON[15:8]

//@}

//--------------------------------------------------------------------------
/// @name	Register handling
//--------------------------------------------------------------------------
//@{
U32		NX_TMU_GetTriminfo	( U32 ModuleIndex );

void	NX_TMU_SetTmuControl	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetTmuControl	( U32 ModuleIndex );

void	NX_TMU_SetTmuControl1	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetTmuControl1	( U32 ModuleIndex );

U32		NX_TMU_GetTmuStatus	( U32 ModuleIndex );

void	NX_TMU_SetSamplingInterval	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetSamplingInterval	( U32 ModuleIndex );

void	NX_TMU_SetCounterValue0	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetCounterValue0	( U32 ModuleIndex );

void	NX_TMU_SetCounterValue1	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetCounterValue1	( U32 ModuleIndex );

U32		NX_TMU_GetCurrentTemp0	( U32 ModuleIndex );

U32		NX_TMU_GetCurrentTemp1	( U32 ModuleIndex );

void	NX_TMU_SetThresholdTempRise	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetThresholdTempRise	( U32 ModuleIndex );

void	NX_TMU_SetThresholdTempFall	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetThresholdTempFall	( U32 ModuleIndex );

void	NX_TMU_SetThresholdTq	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetThresholdTq	( U32 ModuleIndex );

U32		NX_TMU_GetTqStatus	( U32 ModuleIndex );

U32		NX_TMU_GetP0PastTemp3_0	( U32 ModuleIndex );

U32		NX_TMU_GetP0PastTemp7_4	( U32 ModuleIndex );

U32		NX_TMU_GetP0PastTemp11_8	( U32 ModuleIndex );

U32		NX_TMU_GetP0PastTemp15_12	( U32 ModuleIndex );

void	NX_TMU_SetP0IntEn	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetP0IntEn	( U32 ModuleIndex );

U32		NX_TMU_GetP0IntStat	( U32 ModuleIndex );

void	NX_TMU_SetP0IntClear	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetP0IntClear	( U32 ModuleIndex );

void	NX_TMU_SetEmulCon	( U32 ModuleIndex, U32 value );
U32		NX_TMU_GetEmulCon	( U32 ModuleIndex );

//@}


#ifdef	__cplusplus
}
#endif

#endif // _NX_TMU_H
