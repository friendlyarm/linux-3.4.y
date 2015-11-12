//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: ECID
//	File		: nx_ecid.c
//	Description	:
//	Author		: Sei
//	History		:
//------------------------------------------------------------------------------
#include <nx_chip.h>
#include <nx_ecid.h>

static struct
{
    struct NX_ECID_RegisterSet * pRegister;
} __g_ModuleVariables = {CNULL, };

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
CBOOL	NX_ECID_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_ModuleVariables.pRegister = CNULL;
		bInit = CTRUE;
	}

	return CTRUE;
}

U32     NX_ECID_GetNumberOfModule( void )
{
    return  NUMBER_OF_ECID_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
U32		NX_ECID_GetPhysicalAddress( void )
{
	return  (U32)PHY_BASEADDR_ECID_MODULE;	
}

U32		NX_ECID_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_ECID_RegisterSet );
}

void	NX_ECID_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_ModuleVariables.pRegister = (struct NX_ECID_RegisterSet *) BaseAddress;
}

void*	NX_ECID_GetBaseAddress( void )
{
	return (void*)__g_ModuleVariables.pRegister;
}

CBOOL	NX_ECID_OpenModule( void )
{
    NX_ASSERT( __g_ModuleVariables.pRegister != CNULL );
	return CTRUE;
}

CBOOL	NX_ECID_CloseModule( void )
{
    NX_ASSERT( __g_ModuleVariables.pRegister != CNULL );
	return CTRUE;
}

CBOOL	NX_ECID_CheckBusy( void )
{
	return CFALSE;
}

CBOOL	NX_ECID_CanPowerDown( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
// Colck & Reset Control
//------------------------------------------------------------------------------
/*
U32     NX_ECID_GetClockNumber( void )
{
    static const U32 CLKGEN_ECID_LIST[] =
    {
        CLOCKINDEX_LIST( ECID )
    };

    return (U32)CLKGEN_ECID_LIST[0];
}
*/

U32     NX_ECID_GetResetNumber( void )
{
    const U32 RSTCON_ECID_LIST[] =
    {
        RESETINDEX_LIST( ECID, i_nRST )
    };

    return (U32)RSTCON_ECID_LIST[0];
}

//------------------------------------------------------------------------------
/// @brief		Get UID of chip
///	@remark		UID is created by Samsung's ECID
//------------------------------------------------------------------------------
void NX_ECID_GetECID
(
    //(&ECID)[4]    
    U32 ECID[4]
)
{
    NX_ASSERT( __g_ModuleVariables.pRegister != CNULL );

	ECID[0] = __g_ModuleVariables.pRegister->ECID[0];
	ECID[1] = __g_ModuleVariables.pRegister->ECID[1];
	ECID[2] = __g_ModuleVariables.pRegister->ECID[2];
	ECID[3] = __g_ModuleVariables.pRegister->ECID[3];
}

//------------------------------------------------------------------------------
/// @brief		Get name of chip
///	@remark		GUID is created by Microsoft's uuidgen.exe. It's 128bit integer.
//------------------------------------------------------------------------------
void NX_ECID_GetChipName
(
	//U8 (&ChipName)[64]	///< [out] name of chip (return value)
	U8 ChipName[49]
)
{
	U32 i;
	for( i = 0 ; i < 48 ; i ++ )
	{
		//ChipName[i] = m_pRegister->ChipName[i];
		ChipName[i] = __g_ModuleVariables.pRegister->CHIPNAME[i];
	}
	for( i = 0; i < 48; i++)
	{
		if( (ChipName[i] == '-') && (ChipName[i+1] == '-') ) 
		{
			ChipName[i] = 0; 
			ChipName[i+1] = 0;
		}
	}
    //ChipName[i] = 0;
}

//------------------------------------------------------------------------------
/// @brief		Get GUID of chip
///	@remark		GUID is created by Microsoft's uuidgen.exe. It's 128bit integer.
//------------------------------------------------------------------------------
void NX_ECID_GetGUID
(
	NX_GUID * guid
)
{
	guid->GUID0     = __g_ModuleVariables.pRegister->GUID0;
	guid->GUID1     = __g_ModuleVariables.pRegister->GUID1;
	guid->GUID2     = __g_ModuleVariables.pRegister->GUID2;
	guid->GUID3[0]  = __g_ModuleVariables.pRegister->GUID3[0];
	guid->GUID3[1]  = __g_ModuleVariables.pRegister->GUID3[1];
	guid->GUID3[2]  = __g_ModuleVariables.pRegister->GUID3[2];
	guid->GUID3[3]  = __g_ModuleVariables.pRegister->GUID3[3];
	guid->GUID3[4]  = __g_ModuleVariables.pRegister->GUID3[4];
	guid->GUID3[5]  = __g_ModuleVariables.pRegister->GUID3[5];
	guid->GUID3[6]  = __g_ModuleVariables.pRegister->GUID3[6];
	guid->GUID3[7]  = __g_ModuleVariables.pRegister->GUID3[7];
}

//------------------------------------------------------------------------------
/// @brief		Get BoundingID of chip
//------------------------------------------------------------------------------
// Addr
void    NX_ECID_SetA( U32 Addr )
{
    const U32   A_POS   = 7;
    const U32   A_MASK  = 7UL << A_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( Addr >= 0 && Addr < 8 );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[0];
    ReadValue  &= ~A_MASK;
    ReadValue  |= Addr << A_POS;

    WriteIO32(&pRegister->EC[0], ReadValue);
}

U32     NX_ECID_GetA( void )
{
    const U32   A_POS   = 7;
    const U32   A_MASK  = 7UL << A_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (U32)((__g_ModuleVariables.pRegister->EC[0] & A_MASK) >> A_POS);
}

// CS
void    NX_ECID_SetCS( CBOOL Enable )
{
    const U32   CS_POS  = 6;
    const U32   CS_MASK = 1UL << CS_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[0];
    ReadValue  &= ~CS_MASK;
    ReadValue  |= Enable << CS_POS;

    WriteIO32(&pRegister->EC[0], ReadValue);
}

CBOOL   NX_ECID_GetCS( void )
{
    const U32   CS_POS  = 6;
    const U32   CS_MASK = 1UL << CS_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (CBOOL)((__g_ModuleVariables.pRegister->EC[0] & CS_MASK) >> CS_POS);
}

// SIGDEV
void    NX_ECID_SetSIGDEV( CBOOL Enable )
{
    const U32   SIGDEV_POS  = 5;
    const U32   SIGDEV_MASK = 1UL << SIGDEV_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[0];
    ReadValue  &= ~SIGDEV_MASK;
    ReadValue  |= Enable << SIGDEV_POS;

    WriteIO32(&pRegister->EC[0], ReadValue);
}

CBOOL   NX_ECID_GetSIGDEV( void )
{
    const U32   SIGDEV_POS  = 5;
    const U32   SIGDEV_MASK = 1UL << SIGDEV_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (U32)((__g_ModuleVariables.pRegister->EC[0] & SIGDEV_MASK) >> SIGDEV_POS);
}

// FSET
void    NX_ECID_SetFSET( CBOOL Enable )
{
    const U32   FSET_POS    = 4;
    const U32   FSET_MASK   = 1UL << FSET_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[0];
    ReadValue  &= ~FSET_MASK;
    ReadValue  |= Enable << FSET_POS;

    WriteIO32(&pRegister->EC[0], ReadValue);
}

CBOOL   NX_ECID_GetFSET( void )
{
    const U32   FSET_POS    = 5;
    const U32   FSET_MASK   = 1UL << FSET_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (U32)((__g_ModuleVariables.pRegister->EC[0] & FSET_MASK) >> FSET_POS);
}

// PRCHG
void    NX_ECID_SetPRCHG( CBOOL Enable )
{
    const U32   PRCHG_POS   = 3;
    const U32   PRCHG_MASK  = 1UL << PRCHG_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[0];
    ReadValue  &= ~PRCHG_MASK;
    ReadValue  |= Enable << PRCHG_POS;

    WriteIO32(&pRegister->EC[0], ReadValue);
}

CBOOL   NX_ECID_GetPRCHG( void )
{
    const U32   PRCHG_POS   = 3;
    const U32   PRCHG_MASK  = 1UL << PRCHG_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (U32)((__g_ModuleVariables.pRegister->EC[0] & PRCHG_MASK) >> PRCHG_POS);
}

// Bounding ID
U32     NX_ECID_GetBondingID( void )
{
    const U32   BOND_POS   = 0;
    const U32   BOND_MASK  = 7UL << BOND_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (U32)((__g_ModuleVariables.pRegister->EC[0] & BOND_MASK) >> BOND_POS);
}

// PROG
void    NX_ECID_SetPROG( CBOOL Enable )
{
    const U32   PROG_POS    = 2;
    const U32   PROG_MASK   = 1UL << PROG_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[1];
    ReadValue  &= ~PROG_MASK;
    ReadValue  |= Enable << PROG_POS;

    WriteIO32(&pRegister->EC[1], ReadValue);
}

CBOOL   NX_ECID_GetPROG( void )
{
    const U32   PROG_POS    = 2;
    const U32   PROG_MASK   = 1UL << PROG_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (CBOOL)((__g_ModuleVariables.pRegister->EC[1] & PROG_MASK) >> PROG_POS);
}

// SCK
void    NX_ECID_SetSCK( CBOOL Enable )
{
    const U32   SCK_POS  = 1;
    const U32   SCK_MASK = 1UL << SCK_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[1];
    ReadValue  &= ~SCK_MASK;
    ReadValue  |= Enable << SCK_POS;

    WriteIO32(&pRegister->EC[1], ReadValue);
}

CBOOL   NX_ECID_GetSCK( void )
{
    const U32   SCK_POS  = 1;
    const U32   SCK_MASK = 1UL << SCK_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (CBOOL)((__g_ModuleVariables.pRegister->EC[1] & SCK_MASK) >> SCK_POS);
}

// SDI
void    NX_ECID_SetSDI( CBOOL Enable )
{
    const U32   SDI_POS  = 0;
    const U32   SDI_MASK = 1UL << SDI_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[1];
    ReadValue  &= ~SDI_MASK;
    ReadValue  |= Enable << SDI_POS;

    WriteIO32(&pRegister->EC[1], ReadValue);
}

CBOOL   NX_ECID_GetSDI( void )
{
    const U32   SDI_POS  = 0;
    const U32   SDI_MASK = 1UL << SDI_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (CBOOL)((__g_ModuleVariables.pRegister->EC[1] & SDI_MASK) >> SDI_POS);
}

// Hw Done ( Key Ready )
CBOOL   NX_ECID_GetKeyReady( void )
{
    const U32   READY_POS   = 15;
    const U32   READY_MASK  = 1UL << READY_POS;

    register U32 regVal;

    NX_ASSERT( __g_ModuleVariables.pRegister != CNULL );

    regVal = __g_ModuleVariables.pRegister->EC[2];
    return (CBOOL)((regVal & READY_MASK)>>READY_POS);
}

// CPU_HDCP_EFUSE_SEL
void    NX_ECID_SetHdcpEfuseSel( CBOOL Enable )
{
    const U32   SHES_POS  = 4;
    const U32   SHES_MASK = 1UL << SHES_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( (0==Enable) ||(1==Enable) );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[2];
    ReadValue  &= ~SHES_MASK;
    ReadValue  |= Enable << SHES_POS;

    WriteIO32(&pRegister->EC[2], ReadValue);
}

CBOOL   NX_ECID_GetHdcpEfuseSel( void )
{
    const U32   SHES_POS  = 4;
    const U32   SHES_MASK = 1UL << SHES_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (CBOOL)((__g_ModuleVariables.pRegister->EC[2] & SHES_MASK) >> SHES_POS);
}

// CPU_SEL_BANK
void    NX_ECID_SetSelectFlowingBank( U32 Sel )
{
    const U32   SELBANK_POS     = 0;
    const U32   SELBANK_MASK    = 3UL << SELBANK_POS;

    register struct NX_ECID_RegisterSet* pRegister;
    register U32 ReadValue;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    NX_ASSERT( Sel >= 0 && Sel < 4 );

    pRegister   = __g_ModuleVariables.pRegister;

    ReadValue   = pRegister->EC[2];
    ReadValue  &= ~SELBANK_MASK;
    ReadValue  |= Sel << SELBANK_POS;

    WriteIO32(&pRegister->EC[2], ReadValue);
}

U32     NX_ECID_GetSelectFlowingBank( void )
{
    const U32   SELBANK_POS     = 0;
    const U32   SELBANK_MASK    = 3UL << SELBANK_POS;

    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return (U32)((__g_ModuleVariables.pRegister->EC[2] & SELBANK_MASK) >> SELBANK_POS);
}

// 필요는 없어 보이지만 기존 구현에서 사용하고 있을지도 모르니 남겨둔다
void NX_ECID_SetBONDINGID
(
    CBOOL set_cs, CBOOL set_sigdev, CBOOL set_fset, CBOOL set_prchg
)
{
    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );
    U32 Enable;
    Enable = (U32)( (set_cs<<6) | (set_sigdev<<5) | (set_fset<<4) | (set_prchg<<3) );
    __g_ModuleVariables.pRegister->EC[0] = (U32)(Enable&0x0078);
}

U32  NX_ECID_GetBONDINGID(void)
{
    NX_ASSERT( CNULL != __g_ModuleVariables.pRegister );

    return __g_ModuleVariables.pRegister->EC[0];
}


