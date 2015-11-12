//------------------------------------------------------------------------------
//
//  Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//  Module      : ECID
//  File        : nx_ecid.h
//  Description :
//  Author      : Sei
//  History     :
//------------------------------------------------------------------------------
#ifndef __NX_ECID_H__
#define __NX_ECID_H__

#include "../base/nx_prototype.h"

#ifdef  __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup   ECID
//------------------------------------------------------------------------------
//@{

    /// @brief ECID Module's Register List
    struct  NX_ECID_RegisterSet
    {
        volatile U32 ECID[4];           ///< 0x00 ~ 0x0C    : 128bit ECID Register
        volatile U8  CHIPNAME[48];      ///< 0x10 ~ 0x3C    : Chip Name Register
        volatile U32 RESERVED;          ///< 0x40           : Reserved Region
        volatile U32 GUID0;             ///< 0x44           : GUID 0 Register
        volatile U16 GUID1;             ///< 0x48           : GUID 1 Register
        volatile U16 GUID2;             ///< 0x4A           : GUID 2 Register
        volatile U8  GUID3[8];          ///< 0x4C ~ 0x50    : GUID 3-0 ~ 3-7 Register
        volatile U32 EC[3];             ///< 0x54 ~ 0x5C    : EC 0 ~ 3 Register
    };

    /// @brief ECID Module's GUID Format
    typedef struct {
        U32 GUID0;
        U16 GUID1;
        U16 GUID2;
        U8  GUID3[8];
    } NX_GUID;

    //--------------------------------------------------------------------------
    enum CS {
        CS_DISABLE = 0,
        CS_ENABLE  = 1
    };

    enum SIGDEV {
        SIGDEV_DISABLE = 0,
        SIGDEV_ENABLE  = 1
    };

    enum FSET {
        FSET_DISABLE = 0,
        FSET_ENABLE  = 1
    };

    enum PRCHG {
        PRCHG_DISABLE = 0,
        PRCHG_ENABLE  = 1
    };


//------------------------------------------------------------------------------
/// @name   Module Interface
//@{
CBOOL   NX_ECID_Initialize( void );
U32     NX_ECID_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
/// @name   Basic Interface
//@{
U32     NX_ECID_GetPhysicalAddress( void );
U32     NX_ECID_GetSizeOfRegisterSet( void );
void    NX_ECID_SetBaseAddress( void* BaseAddress );
void*    NX_ECID_GetBaseAddress( void );
CBOOL   NX_ECID_OpenModule( void );
CBOOL   NX_ECID_CloseModule( void );
CBOOL   NX_ECID_CheckBusy( void );
CBOOL   NX_ECID_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
/// @name Clock Control Interface
//U32     NX_ECID_GetClockNumber( void );
U32     NX_ECID_GetResetNumber( void );

//------------------------------------------------------------------------------
/// @name   Module Interface
//@{

//void  NX_ECID_GetECID(U32 (&ECID)[4]);
void    NX_ECID_GetECID( U32 ECID[4] );
//void  NX_ECID_GetChipName(U8 (&ChipName)[64]);
void    NX_ECID_GetChipName(U8 ChipName[64]);
//void  NX_ECID_GetGUID(NX_GUID& guid);
void    NX_ECID_GetGUID (NX_GUID *guid);


void    NX_ECID_SetA( U32 Addr );
U32     NX_ECID_GetA( void );
void    NX_ECID_SetCS( CBOOL Enable );
CBOOL   NX_ECID_GetCS( void );
void    NX_ECID_SetSIGDEV( CBOOL Enable );
CBOOL   NX_ECID_GetSIGDEV( void );
void    NX_ECID_SetFSET( CBOOL Enable );
CBOOL   NX_ECID_GetFSET( void );
void    NX_ECID_SetPRCHG( CBOOL Enable );
CBOOL   NX_ECID_GetPRCHG( void );
U32     NX_ECID_GetBondingID( void );

void    NX_ECID_SetPROG( CBOOL Enable );
CBOOL   NX_ECID_GetPROG( void );
void    NX_ECID_SetSCK( CBOOL Enable );
CBOOL   NX_ECID_GetSCK( void );
void    NX_ECID_SetSDI( CBOOL Enable );
CBOOL   NX_ECID_GetSDI( void );

CBOOL   NX_ECID_GetKeyReady( void );
void    NX_ECID_SetHdcpEfuseSel( CBOOL Enable );
CBOOL   NX_ECID_GetHdcpEfuseSel( void );
void    NX_ECID_SetSelectFlowingBank( U32 Sel );
U32     NX_ECID_GetSelectFlowingBank( void );

void    NX_ECID_SetBONDINGID( CBOOL set_cs, CBOOL set_sigdev, CBOOL set_fset, CBOOL set_prchg);
U32     NX_ECID_GetBONDINGID ( void );


//@}

#ifdef  __cplusplus
}
#endif


#endif // __NX_ECID_H__
