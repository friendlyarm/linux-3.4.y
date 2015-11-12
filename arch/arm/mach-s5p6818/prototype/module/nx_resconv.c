//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		:
//	File		: nx_resconv.c
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#include "nx_resconv.h"


#ifdef NUMBER_OF_DISPLAYTOP_MODULE
#include "nx_displaytop.h"
#endif

static	NX_RESCONV_RegisterSet *__g_pRegister[NUMBER_OF_RESCONV_MODULE];


//------------------------------------------------------------------------------
//
//	RESCONV Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	It is a sample function
 *	@return a register value
 */
/*
U32   NX_RESCONV_GetTEMP( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return __g_pRegister[ModuleIndex]->TEMP;
}
 */

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return  CTRUE	indicate that Initialize is successed.
 *			 CFALSE	indicate that Initialize is failed.
 *	@see	NX_RESCONV_GetNumberOfModule
 */
CBOOL	NX_RESCONV_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
        for( i=0; i < NUMBER_OF_LVDS_MODULE; i++ )
        {
            __g_pRegister[i] = CNULL;
        }

        bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. 
 *				It is equal to NUMBER_OF_RESCONV_MODULE in <nx_chip.h>.
 *	@see		NX_RESCONV_Initialize
 */
U32		NX_RESCONV_GetNumberOfModule( void )
{
	return NUMBER_OF_RESCONV_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_RESCONV_GetSizeOfRegisterSet( void )
{
	return sizeof( NX_RESCONV_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */

void	NX_RESCONV_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] = (NX_RESCONV_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_RESCONV_GetBaseAddress( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_RESCONV?_MODULE in <nx_chip.h>.
 */
U32		NX_RESCONV_GetPhysicalAddress( U32 ModuleIndex )
{
    const U32 PhysicalAddr[NUMBER_OF_RESCONV_MODULE] =
	{
		PHY_BASEADDR_LIST( RESCONV )
		//PHY_BASEADDR_WITH_CHANNEL_LIST( RESCONV )
	};
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		 CTRUE	indicate that Initialize is successed. 
 *				 CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_RESCONV_OpenModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		 CTRUE	indicate that Deinitialize is successed. 
 *				 CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_RESCONV_CloseModule( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		 CTRUE	indicate that Module is Busy. 
 *				 CFALSE	indicate that Module is NOT Busy.
 */
CBOOL	NX_RESCONV_CheckBusy( U32 ModuleIndex )
{
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}

//------------------------------------------------------------------------------
//	clock Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's clock index.
 *	@return		Module's clock index.
 *				It is equal to CLOCKINDEX_OF_RESCONV?_MODULE in <nx_chip.h>.
 */
/*
U32 NX_RESCONV_GetClockNumber ( U32 ModuleIndex )
{
	const U32 ClockNumber[] =
	{
		CLOCKINDEX_LIST( RESCONV )
	};
	NX_CASSERT( NUMBER_OF_RESCONV_MODULE == (sizeof(ClockNumber)/sizeof(ClockNumber[0])) );
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	return	ClockNumber[ModuleIndex];
}
 */

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_RESCONV?_MODULE_i_nRST in <nx_chip.h>.
 */
/*
U32 NX_RESCONV_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( RESCONV, IPnRST0 )
	};
	NX_CASSERT( NUMBER_OF_RESCONV_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}
 */

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number for the interrupt controller.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		A interrupt number.
 *				It is equal to INTNUM_OF_RESCONV?_MODULE in <nx_chip.h>.
 */
U32 	NX_RESCONV_GetInterruptNumber( U32 ModuleIndex )
{
    const U32 InterruptNumber[] = { INTNUM_LIST( RESCONV ) };
    //const U32 InterruptNumber[] = { INTNUM_WITH_CHANNEL_LIST( RESCONV, INTR0 ) };
    NX_CASSERT( NUMBER_OF_RESCONV_MODULE == (sizeof(InterruptNumber)/sizeof(InterruptNumber[0])) );
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    // NX_ASSERT( INTNUM_OF_RESCONV0_MODULE == InterruptNumber[0] );
    // ...
    return InterruptNumber[ModuleIndex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a specified interrupt to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number .
 *						refer to NX_RESCONV_INTCH_xxx in <nx_resconv.h>
 *	@param[in]	Enable	 Set as CTRUE to enable a specified interrupt. 
 *						 Set as CFALSE to disable a specified interrupt.
 *	@return		None.
 */
void	NX_RESCONV_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable )
{
	register NX_RESCONV_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->ADDR_RC_REG02;

	regvalue &=	~( 1UL << IntNum );
	regvalue |= (U32)Enable << IntNum;

	WriteIO32(&pRegister->ADDR_RC_REG02, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_RESCONV_INTCH_xxx in <nx_resconv.h>
 *	@return		 CTRUE	indicates that a specified interrupt is enabled. 
 *				 CFALSE	indicates that a specified interrupt is disabled.

 */
CBOOL	NX_RESCONV_GetInterruptEnable( U32 ModuleIndex, U32 IntNum )
{
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)( ( __g_pRegister[ModuleIndex]->ADDR_RC_REG02 >> (IntNum+16) ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether a specified interrupt is pended or not
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt Number.
 *						refer to NX_RESCONV_INTCH_xxx in <nx_resconv.h>
 *	@return		 CTRUE	indicates that a specified interrupt is pended. 
 *				 CFALSE	indicates that a specified interrupt is not pended.

 */
CBOOL	NX_RESCONV_GetInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_RESCONV_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->ADDR_RC_REG02;
	regvalue = (regvalue>>16) & (pRegister->ADDR_RC_REG02);

	return (CBOOL)( ( regvalue >> IntNum ) & 0x01 );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear a pending state of specified interrupt.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	IntNum	a interrupt number.
 *						refer to NX_RESCONV_INTCH_xxx in <nx_resconv.h>
 *	@return		None.

 */
void	NX_RESCONV_ClearInterruptPending( U32 ModuleIndex, U32 IntNum )
{
	register NX_RESCONV_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->ADDR_RC_REG02, 1UL << IntNum);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set all interrupts to be enabled or disabled.
 *	@param[in]	ModuleIndex		an index of module.
 *	@param[in]	Enable	 Set as CTRUE to enable all interrupts. 
 *						 Set as CFALSE to disable all interrupts.
 *	@return		None.

 */
void	NX_RESCONV_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable )
{
	register NX_RESCONV_RegisterSet* pRegister;
	register U32	regvalue;

	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( (0==Enable) || (1==Enable) );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = Enable ? 0x000F0000 : 0 ;

	WriteIO32(&pRegister->ADDR_RC_REG02, regvalue);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are enabled or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are enabled. 
 *				 CFALSE	indicates that all interrupts are disabled.

 */
CBOOL	NX_RESCONV_GetInterruptEnableAll( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	return (CBOOL)(0!=( __g_pRegister[ModuleIndex]->ADDR_RC_REG02 ));
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether some of interrupts are pended or not.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		 CTRUE	indicates that one or more interrupts are pended. 
 *				 CFALSE	indicates that no interrupt is pended.

 */
CBOOL	NX_RESCONV_GetInterruptPendingAll( U32 ModuleIndex )
{
	register NX_RESCONV_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->ADDR_RC_REG02;
	regvalue = (regvalue>>16) & (pRegister->ADDR_RC_REG02);
	return (CBOOL)( 0 != ( regvalue ) );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Clear pending state of all interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		None.

 */
void	NX_RESCONV_ClearInterruptPendingAll( U32 ModuleIndex )
{
	register NX_RESCONV_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->ADDR_RC_REG02, 0x0000000F);	// just write operation make pending clear
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a interrupt number which has the most prority of pended interrupts.
 *	@param[in]	ModuleIndex		an index of module.
 *	@return		a interrupt number. A value of '-1' means that no interrupt is pended.
 *				refer to NX_RESCONV_INTCH_xxx in <nx_resconv.h>

 */
S32		NX_RESCONV_GetInterruptPendingNumber( U32 ModuleIndex )	// -1 if None
{
	int i;
	register NX_RESCONV_RegisterSet* pRegister;
	register U32	regvalue;
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];
	regvalue  = pRegister->ADDR_RC_REG02;
	regvalue = (regvalue>>16) & (pRegister->ADDR_RC_REG02);
	if( 0!=regvalue )
	{
		for( i=0; i<32; i++ )
		{
			if( regvalue & 1UL )
			{
				return i;
			}
			regvalue>>=1;
		}
	}
	return -1;
}

void	NX_RESCONV_DOWN_INIT ( U32 ModuleIndex )
{

	int	SRC_HSIZE, SRC_VSIZE;
	int	DST_HSIZE, DST_VSIZE;

	int FIXED_POINT = 65536;
	int DELTA_X, DELTA_Y;

	int SoftV = 8;
	int SoftH = 16;

    int SG_VFP;	// ResConv SyncGen V Front Porch
    int SG_VBP;	// ResConv SyncGen V Back  Porch
    int SG_VS ;	// ResConv SyncGen V Sync
//  int SG_VAV;	// ResConv SyncGen V Active

    int SG_HFP;	// ResConv SyncGen H Front Porch
    int SG_HBP;	// ResConv SyncGen H Back  Porch
    int SG_HS ;	// ResConv SyncGen H Sync
//  int SG_HAV;	// ResConv SyncGen H Active

	int SG_S2IN_VS;
	int SG_HOFFSET;
	int SG_HDELAY ;

    SG_VFP = 1;
    SG_VBP = 1;
    SG_VS  = 4;

    SG_HFP = 1;
    SG_HBP = 1;
    SG_HS  = 22;

    SG_S2IN_VS = 0;
    SG_HOFFSET = 0;
    SG_HDELAY  = 1;

	SRC_HSIZE = 320;
	SRC_VSIZE = 240;
	DST_HSIZE = 320;
	DST_VSIZE = 240;

	DELTA_X = (SRC_HSIZE*FIXED_POINT)/(DST_HSIZE-1);
	DELTA_Y = (SRC_VSIZE*FIXED_POINT)/(DST_VSIZE-1);

	register NX_RESCONV_RegisterSet* pRegister;
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	pRegister = __g_pRegister[ModuleIndex];

        WriteIO32(&pRegister->ADDR_RC_REG01, (	(0			<< 21) 	|
												(0			<< 16) 	|
												(0			<< 14) 	|
												(0			<<  8) 	|
												(0			<<  5) 	|
												(0			<<  2) 	|
												(3			<<  0))	);   // Interrupt Register

        WriteIO32(&pRegister->ADDR_RC_REG02, (	(0			<< 20) 	|
												(2			<< 16) 	|
												(0			<<  0))	);          //2

        WriteIO32(&pRegister->ADDR_RC_REG03, DELTA_X );	// Delta X

        WriteIO32(&pRegister->ADDR_RC_REG04, DELTA_Y );	// Delta Y

        WriteIO32(&pRegister->ADDR_RC_REG05, (  (SoftV		<< 16) 	|
												(SoftH		<<  0))	);//5

        WriteIO32(&pRegister->ADDR_RC_REG06, (	(SRC_VSIZE 	<< 16) 	|
												(SRC_HSIZE  <<  0))	);	// Source Size

        WriteIO32(&pRegister->ADDR_RC_REG07, (	(DST_VSIZE 	<< 16) 	|
												(DST_HSIZE  <<  0))	);	// Destination Size

        WriteIO32(&pRegister->ADDR_RC_REG09, (	((SG_HFP)	<< 24) 	|
												((SG_HBP)	<< 16) 	|
												((SG_HS )	<<  0))	);   // HSync Timing

        WriteIO32(&pRegister->ADDR_RC_REG10, (	((SG_VFP)	<< 24) 	|
												((SG_VBP)	<< 16) 	|
												((SG_VS )	<<  0))	);    // VSync Timing

        WriteIO32(&pRegister->ADDR_RC_REG11, (	(SG_S2IN_VS << 24) 	|
												(SG_HOFFSET <<  8) 	|
												(SG_HDELAY  <<  0))	);    // Sync Gen Delay

        WriteIO32(&pRegister->ADDR_RC_REG00, (	(1	        <<  8) 	|
												(1	        <<  0))	);   // Scaler Continuous Frame, Scaler Run

        WriteIO32(&pRegister->ADDR_RC_REG08, (	 1			<<  0)	);            // SyncGen Run

}

void	NX_RESCONV_INIT (	U32 ModuleIndex,
							int SRC_HSIZE,
							int SRC_VSIZE,
							int SRC_CROP_L,
							int SRC_CROP_R,
							int SRC_CROP_T,
							int SRC_CROP_B,
							int DST_HSIZE,
							int DST_VSIZE,
							int FIXED_POINT,
							int DELTA_X,
							int DELTA_Y,
							int SoftV,
							int SoftH,
							int SG_VFP, // ResConv SyncGen V Front Porch
							int SG_VBP, // ResConv SyncGen V Back  Porch
							int SG_VS , // ResConv SyncGen V Sync
							int SG_VAV, // ResConv SyncGen V Active
							int SG_HFP, // ResConv SyncGen H Front Porch
							int SG_HBP, // ResConv SyncGen H Back  Porch
							int SG_HS , // ResConv SyncGen H Sync
							int SG_HAV, // ResConv SyncGen H Active
							int SG_S2IN_VS,
							int SG_HOFFSET,
							int SG_HDELAY
)
{
    register NX_RESCONV_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    pRegister = __g_pRegister[ModuleIndex];

        WriteIO32(&pRegister->ADDR_RC_REG01, (  (0          << 21)  |
                                                (0          << 16)  |
                                                (0          << 14)  |
                                                (0          <<  8)  |
                                                (0          <<  5)  |
                                                (0          <<  2)  |
                                                (3          <<  0)) );   // Interrupt Register

        WriteIO32(&pRegister->ADDR_RC_REG02, (  (0          << 20)  |
                                                (2          << 16)  |
                                                (0          <<  0)) );          //2

        WriteIO32(&pRegister->ADDR_RC_REG03, DELTA_X ); // Delta X

        WriteIO32(&pRegister->ADDR_RC_REG04, DELTA_Y ); // Delta Y

        WriteIO32(&pRegister->ADDR_RC_REG05, (  (SoftV      << 16)  |
                                                (SoftH      <<  0)) );//5

        WriteIO32(&pRegister->ADDR_RC_REG06, (  (SRC_VSIZE  << 16)  |
                                                (SRC_HSIZE  <<  0)) );  // Source Size

        WriteIO32(&pRegister->ADDR_RC_REG07, (  (DST_VSIZE  << 16)  |
                                                (DST_HSIZE  <<  0)) );  // Destination Size

        WriteIO32(&pRegister->ADDR_RC_REG09, (  ((SG_HFP)   << 24)  |
                                                ((SG_HBP)   << 16)  |
                                                ((SG_HS )   <<  0)) );   // HSync Timing

        WriteIO32(&pRegister->ADDR_RC_REG10, (  ((SG_VFP)   << 24)  |
                                                ((SG_VBP)   << 16)  |
                                                ((SG_VS )   <<  0)) );    // VSync Timing

        WriteIO32(&pRegister->ADDR_RC_REG11, (  (SG_S2IN_VS << 24)  |
                                                (SG_HOFFSET <<  8)  |
                                                (SG_HDELAY  <<  0)) );    // Sync Gen Delay

        WriteIO32(&pRegister->ADDR_RC_REG12, (  (SRC_CROP_B << 24)  |
                                                (SRC_CROP_T << 16)  |
                                                (SRC_CROP_R <<  8)  |
                                                (SRC_CROP_L <<  0)) );    // Source Image Crop


}

void    NX_RESCONV_FINIT ( U32 ModuleIndex )
{

    int YVFinit[24] =   {(( 52 << 24) | ( 55 << 16) | ( 58 << 8) | 61),
                         (( 42 << 24) | ( 45 << 16) | ( 48 << 8) | 50),
                         (( 32 << 24) | ( 35 << 16) | ( 38 << 8) | 40),
                         (( 22 << 24) | ( 25 << 16) | ( 28 << 8) | 30),
                         (( 13 << 24) | ( 14 << 16) | ( 16 << 8) | 18),
                         ((  9 << 24) | ( 10 << 16) | ( 11 << 8) | 12),
                         ((  5 << 24) | (  6 << 16) | (  7 << 8) |  8),
                         ((  1 << 24) | (  2 << 16) | (  3 << 8) |  4),

                         (( 72 << 24) | ( 70 << 16) | ( 68 << 8) | 66),
                         (( 78 << 24) | ( 76 << 16) | ( 74 << 8) | 73),
                         (( 84 << 24) | ( 82 << 16) | ( 80 << 8) | 79),
                         (( 88 << 24) | ( 87 << 16) | ( 86 << 8) | 85),
                         (( 85 << 24) | ( 86 << 16) | ( 87 << 8) | 88),
                         (( 79 << 24) | ( 80 << 16) | ( 82 << 8) | 84),
                         (( 73 << 24) | ( 74 << 16) | ( 76 << 8) | 78),
                         (( 66 << 24) | ( 68 << 16) | ( 70 << 8) | 72),

                         ((  4 << 24) | (  3 << 16) | (  2 << 8) |  1),
                         ((  8 << 24) | (  7 << 16) | (  6 << 8) |  5),
                         (( 12 << 24) | ( 11 << 16) | ( 10 << 8) |  9),
                         (( 18 << 24) | ( 16 << 16) | ( 14 << 8) | 13),
                         (( 30 << 24) | ( 28 << 16) | ( 25 << 8) | 22),
                         (( 40 << 24) | ( 38 << 16) | ( 35 << 8) | 32),
                         (( 50 << 24) | ( 48 << 16) | ( 45 << 8) | 42),
                         (( 61 << 24) | ( 58 << 16) | ( 55 << 8) | 52)};

	int	YHFinit[160] =  {(( -5 << 16) | (0xFFFF & -2)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 ((-10 << 16) | (0xFFFF & -8)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-11 << 16) | (0xFFFF &-12)),
						 (( -9 << 16) | (0xFFFF &-10)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -6 << 16) | (0xFFFF & -7)),
						 (( -4 << 16) | (0xFFFF & -5)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -2 << 16) | (0xFFFF & -3)),
						 (( -2 << 16) | (0xFFFF & -2)),
						 ((  0 << 16) | (0xFFFF & -1)),
						 ((248 << 16) | (0xFFFF &252)),
						 ((238 << 16) | (0xFFFF &244)),
						 ((227 << 16) | (0xFFFF &232)),
						 ((216 << 16) | (0xFFFF &222)),
						 ((204 << 16) | (0xFFFF &210)),
						 ((192 << 16) | (0xFFFF &198)),
						 ((181 << 16) | (0xFFFF &186)),
						 ((170 << 16) | (0xFFFF &176)),
						 ((158 << 16) | (0xFFFF &164)),
						 ((145 << 16) | (0xFFFF &152)),
						 ((133 << 16) | (0xFFFF &138)),
						 ((123 << 16) | (0xFFFF &128)),
						 ((113 << 16) | (0xFFFF &118)),
						 ((102 << 16) | (0xFFFF &108)),
						 (( 92 << 16) | (0xFFFF & 96)),
						 (( 88 << 16) | (0xFFFF & 88)),
						 (( 80 << 16) | (0xFFFF & 86)),
						 (( 70 << 16) | (0xFFFF & 75)),
						 (( 64 << 16) | (0xFFFF & 67)),
						 (( 58 << 16) | (0xFFFF & 61)),
						 (( 50 << 16) | (0xFFFF & 54)),
						 (( 38 << 16) | (0xFFFF & 44)),
						 (( 32 << 16) | (0xFFFF & 35)),
						 (( 28 << 16) | (0xFFFF & 30)),
						 (( 24 << 16) | (0xFFFF & 26)),
						 (( 20 << 16) | (0xFFFF & 22)),
						 (( 16 << 16) | (0xFFFF & 18)),
						 (( 12 << 16) | (0xFFFF & 14)),
						 ((  8 << 16) | (0xFFFF & 10)),
						 ((  4 << 16) | (0xFFFF &  6)),
						 ((  0 << 16) | (0xFFFF &  2)),
						 (( -2 << 16) | (0xFFFF & -1)),
						 ((271 << 16) | (0xFFFF &264)),
						 ((282 << 16) | (0xFFFF &278)),
						 ((292 << 16) | (0xFFFF &286)),
						 ((302 << 16) | (0xFFFF &298)),
						 ((310 << 16) | (0xFFFF &306)),
						 ((318 << 16) | (0xFFFF &314)),
						 ((325 << 16) | (0xFFFF &322)),
						 ((332 << 16) | (0xFFFF &328)),
						 ((340 << 16) | (0xFFFF &336)),
						 ((348 << 16) | (0xFFFF &344)),
						 ((352 << 16) | (0xFFFF &352)),
						 ((354 << 16) | (0xFFFF &352)),
						 ((358 << 16) | (0xFFFF &356)),
						 ((362 << 16) | (0xFFFF &360)),
						 ((362 << 16) | (0xFFFF &364)),
						 ((354 << 16) | (0xFFFF &360)),
						 ((360 << 16) | (0xFFFF &354)),
						 ((364 << 16) | (0xFFFF &362)),
						 ((360 << 16) | (0xFFFF &362)),
						 ((356 << 16) | (0xFFFF &358)),
						 ((352 << 16) | (0xFFFF &354)),
						 ((352 << 16) | (0xFFFF &352)),
						 ((344 << 16) | (0xFFFF &348)),
						 ((336 << 16) | (0xFFFF &340)),
						 ((328 << 16) | (0xFFFF &332)),
						 ((322 << 16) | (0xFFFF &325)),
						 ((314 << 16) | (0xFFFF &318)),
						 ((306 << 16) | (0xFFFF &310)),
						 ((298 << 16) | (0xFFFF &302)),
						 ((286 << 16) | (0xFFFF &292)),
						 ((278 << 16) | (0xFFFF &282)),
						 ((264 << 16) | (0xFFFF &271)),
						 (( -1 << 16) | (0xFFFF & -2)),
						 ((  2 << 16) | (0xFFFF &  0)),
						 ((  6 << 16) | (0xFFFF &  4)),
						 (( 10 << 16) | (0xFFFF &  8)),
						 (( 14 << 16) | (0xFFFF & 12)),
						 (( 18 << 16) | (0xFFFF & 16)),
						 (( 22 << 16) | (0xFFFF & 20)),
						 (( 26 << 16) | (0xFFFF & 24)),
						 (( 30 << 16) | (0xFFFF & 28)),
						 (( 35 << 16) | (0xFFFF & 32)),
						 (( 44 << 16) | (0xFFFF & 38)),
						 (( 54 << 16) | (0xFFFF & 50)),
						 (( 61 << 16) | (0xFFFF & 58)),
						 (( 67 << 16) | (0xFFFF & 64)),
						 (( 75 << 16) | (0xFFFF & 70)),
						 (( 86 << 16) | (0xFFFF & 80)),
						 (( 88 << 16) | (0xFFFF & 88)),
						 (( 96 << 16) | (0xFFFF & 92)),
						 ((108 << 16) | (0xFFFF &102)),
						 ((118 << 16) | (0xFFFF &113)),
						 ((128 << 16) | (0xFFFF &123)),
						 ((138 << 16) | (0xFFFF &133)),
						 ((152 << 16) | (0xFFFF &145)),
						 ((164 << 16) | (0xFFFF &158)),
						 ((176 << 16) | (0xFFFF &170)),
						 ((186 << 16) | (0xFFFF &181)),
						 ((198 << 16) | (0xFFFF &192)),
						 ((210 << 16) | (0xFFFF &204)),
						 ((222 << 16) | (0xFFFF &216)),
						 ((232 << 16) | (0xFFFF &227)),
						 ((244 << 16) | (0xFFFF &238)),
						 ((252 << 16) | (0xFFFF &248)),
						 (( -1 << 16) | (0xFFFF &  0)),
						 (( -2 << 16) | (0xFFFF & -2)),
						 (( -3 << 16) | (0xFFFF & -2)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -4 << 16) | (0xFFFF & -4)),
						 (( -5 << 16) | (0xFFFF & -4)),
						 (( -7 << 16) | (0xFFFF & -6)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 ((-10 << 16) | (0xFFFF & -9)),
						 ((-12 << 16) | (0xFFFF &-11)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 ((-12 << 16) | (0xFFFF &-12)),
						 (( -8 << 16) | (0xFFFF &-10)),
						 (( -8 << 16) | (0xFFFF & -8)),
						 (( -2 << 16) | (0xFFFF & -5))};

	int	i, j;

    register NX_RESCONV_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    pRegister = __g_pRegister[ModuleIndex];

		for (i=0;i<24;i++) {
        	WriteIO32(&pRegister->ADDR_RC_YVFILTER[i], YVFinit[i]);
		}
		for (j=0;j<160;j++) {
        	WriteIO32(&pRegister->ADDR_RC_YHFILTER[j], YHFinit[j]);
		}
}

#if defined(RESCONV_REISTER_TEST)
void    NX_RESCONV_REG_RD_TEST ( U32 ModuleIndex )
{
    int i;
    register NX_RESCONV_RegisterSet* pRegister;
    register U32    regvalue;
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    pRegister = __g_pRegister[ModuleIndex];


		regvalue  = pRegister->ADDR_RC_REG00;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  0, regvalue );
		regvalue  = pRegister->ADDR_RC_REG01;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  1, regvalue );
		regvalue  = pRegister->ADDR_RC_REG02;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  2, regvalue );
		regvalue  = pRegister->ADDR_RC_REG03;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  3, regvalue );
		regvalue  = pRegister->ADDR_RC_REG04;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  4, regvalue );
		regvalue  = pRegister->ADDR_RC_REG05;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  5, regvalue );
		regvalue  = pRegister->ADDR_RC_REG06;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  6, regvalue );
		regvalue  = pRegister->ADDR_RC_REG07;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  7, regvalue );
		regvalue  = pRegister->ADDR_RC_REG08;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  8, regvalue );
		regvalue  = pRegister->ADDR_RC_REG09;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n",  9, regvalue );
		regvalue  = pRegister->ADDR_RC_REG10;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n", 10, regvalue );
		regvalue  = pRegister->ADDR_RC_REG11;
		NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n", 11, regvalue );

        for( i=0; i<24; i++ ) {
    		regvalue  = pRegister->ADDR_RC_YVFILTER[i];
			NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n", i, regvalue );
        }

        for( i=0; i<160; i++ ) {
    		regvalue  = pRegister->ADDR_RC_YHFILTER[i];
			NX_CONSOLE_Printf("REG_RD : addr = %d : data = %x\n", i, regvalue );
        }
}
#endif

void    NX_RESCONV_RUN ( U32 ModuleIndex )
{
    register NX_RESCONV_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    pRegister = __g_pRegister[ModuleIndex];

        WriteIO32(&pRegister->ADDR_RC_REG00, (  (1          <<  8)  |
                                                (1          <<  0)) );   // Scaler Continuous Frame, Scaler Run

        WriteIO32(&pRegister->ADDR_RC_REG08, (   1          <<  0)  );            // SyncGen Run

}


void    NX_RESCONV_STOP ( U32 ModuleIndex )
{
    register NX_RESCONV_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    pRegister = __g_pRegister[ModuleIndex];

        WriteIO32(&pRegister->ADDR_RC_REG08, (   0          <<  0)  );            // SyncGen Run

        WriteIO32(&pRegister->ADDR_RC_REG00, (  (0          <<  8)  |
                                                (0          <<  0)) );   // Scaler Continuous Frame, Scaler Run

}


void    NX_RESCONV_INTCLEAR ( U32 ModuleIndex )
{
    register NX_RESCONV_RegisterSet* pRegister;
    NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
    NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
    pRegister = __g_pRegister[ModuleIndex];

        // Interrupt Clear
        WriteIO32(&pRegister->ADDR_RC_REG02, (  (0          << 20)  |
                                                (2          << 16)  |
                                                (1          << 11)  |
                                                (1          << 10)  |
                                                (1          <<  9)  |
                                                (1          <<  8)  |
                                                (0          <<  0)) );

        //WriteIO32(&pRegister->ADDR_RC_REG02, (  (0          << 20)  |
        //                                        (2          << 16)  |
        //                                        (0          << 11)  |
        //                                        (0          << 10)  |
        //                                        (0          <<  9)  |
        //                                        (0          <<  8)  |
        //                                        (0          <<  0)) );

}

//------------------------------------------------------------------------------
// DMA Interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get DMA peripheral index
 *	@param[in]	ModuleIndex	an index of module.
 *	@param[in]	ChannelIndex refer to NX_RESCONV_DMACH_xxx in <nx_resconv.h>
 *	@return		DMA peripheral index.
 *	@see		NX_DMA_TransferMemToIO(DestinationPeriID),
 *				NX_DMA_TransferIOToMem(SourcePeriID)
 */
/*
U32 NX_RESCONV_GetDMANumber ( U32 ModuleIndex , U32 ChannelIndex )
{
	const U32 DMANumber[][NUMBER_OF_RESCONV_MODULE] =
	{
	    { DMAINDEX_WITH_CHANNEL_LIST( RESCONV, TXDMA ) }, // DMAINDEX_OF_RESCONV?_MODULE_TEST0
	    { DMAINDEX_WITH_CHANNEL_LIST( RESCONV, RXDMA ) }, // DMAINDEX_OF_RESCONV?_MODULE_TEST1
	};
	NX_CASSERT( NUMBER_OF_RESCONV_MODULE == (sizeof(DMANumber[0])/sizeof(DMANumber[0][0])) );
	NX_ASSERT( NUMBER_OF_RESCONV_MODULE > ModuleIndex );
	NX_ASSERT( (sizeof(DMANumber)/sizeof(DMANumber[0])) > ChannelIndex );
	// NX_ASSERT( DMAINDEX_OF_RESCONV0_MODULE_TEST0 == DMANumber[0][0] );
	// NX_ASSERT( DMAINDEX_OF_RESCONV1_MODULE_TEST0 == DMANumber[1][0] );
	// ...
	return DMANumber[ChannelIndex][ModuleIndex];
}
 */
