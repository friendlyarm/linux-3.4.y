//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		: Display Control by neo
//	File		: nx_DisplayTop.h
//	Description	:
//	Author		: Prototype by choiyk
//	History		:
//------------------------------------------------------------------------------


#include "nx_chip.h"
#include "nx_lcdif.h"
#include "nx_displaytop.h"

//------------------------------------------------------------------------------
/// @defgroup	SOCPlatform,  2012/09/20
// 각 list 들 선언.
//------------------------------------------------------------------------------
static	NX_LCDINTERFACE_RegisterSet *__g_pRegister[NUMBER_OF_LCDINTERFACE_MODULE];




/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicate that Initialize is successed.\n
 *			CFALSE	indicate that Initialize is failed.
 *	@see	NX_LCDINTERFACE_GetNumberOfModule
 */
CBOOL NX_LCDINTERFACE_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_LCDINTERFACE_MODULE; i++)
		{
			memset( __g_pRegister, 0, sizeof(__g_pRegister) );
		}
		bInit = CTRUE;
	}
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number. \n
 *				It is equal to NUMBER_OF_LCDINTERFACE_MODULE in <nx_chip.h>.
 */
U32		NX_LCDINTERFACE_GetNumberOfModule( void )
{
	return NUMBER_OF_LCDINTERFACE_MODULE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_LCDINTERFACE_GetSizeOfRegisterSet( void )
{
	return sizeof( *__g_pRegister );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_LCDINTERFACE_SetBaseAddress( U32 ModuleIndex, void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	__g_pRegister[ModuleIndex] =
		(NX_LCDINTERFACE_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_LCDINTERFACE_GetBaseAddress( U32 ModuleIndex )
{
	NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	return (void*)__g_pRegister[ModuleIndex];
}


//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address. 
 *				It is equal to PHY_BASEADDR_DISPLAYTOP?_MODULE in <nx_chip.h>.
 */
U32		NX_LCDINTERFACE_GetPhysicalAddress( U32 ModuleIndex )
{

	const U32 PhysicalAddr[] =
	{
		PHY_BASEADDR_LIST( LCDINTERFACE )
	};
	NX_CASSERT( NUMBER_OF_LCDINTERFACE_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	return	PhysicalAddr[ModuleIndex];

}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. 
 *				CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_LCDINTERFACE_OpenModule( U32 ModuleIndex )
{

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicate that Deinitialize is successed. 
 *				CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_LCDINTERFACE_CloseModule( U32 ModuleIndex )
{

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicate that Module is Busy. 
 *				CFALSE	indicate that Module is NOT Busy.
 */

// 이 함수는 SYNCENB를 검사하지 않는다.
CBOOL	NX_LCDINTERFACE_CheckBusy( U32 ModuleIndex )
{

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
    NX_DISABLE_UNUSED_VAR_WARNING(ModuleIndex);
    // @todo Implement
	return CFALSE;
}

//------------------------------------------------------------------------------
//	clock Interface
// CLKGEN 을 포함하고 있다.
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.
 *				It is equal to RESETINDEX_OF_DISPLAYTOP?_MODULE_i_nRST in <nx_chip.h>.
 */

U32 NX_LCDINTERFACE_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( LCDINTERFACE, i_nRST )
	};

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}


//------------------------------------------------------------------------------
// Interrupt Interface
// Interrupt Not Use.
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
// Register Use Function
//------------------------------------------------------------------------------

void NX_LCDINTERFACE_Set_DISPCNTL0		( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPCNTL0, regvalue);
}

U32 NX_LCDINTERFACE_Get_DISPCNTL0		( U32 ModuleIndex)
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;
	U32 regvalue;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->DISPCNTL0);

	return regvalue;
}



void NX_LCDINTERFACE_Set_DISPCNTL1      ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPCNTL1, regvalue);
}


void NX_LCDINTERFACE_Set_DISPDELYCNT0   ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPDELYCNT0, regvalue);
}


void NX_LCDINTERFACE_Set_DISPPADLOC0    ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPPADLOC0, regvalue);
}


void NX_LCDINTERFACE_Set_DISPPADLOC1    ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPPADLOC1, regvalue);
}


void NX_LCDINTERFACE_Set_DISPPADLOC2    ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPPADLOC2, regvalue);
}


void NX_LCDINTERFACE_Set_DISPPADLOC3    ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPPADLOC3, regvalue);
}


void NX_LCDINTERFACE_Set_DISPRGBMASK    ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPRGBMASK, regvalue);
}


void NX_LCDINTERFACE_Set_I80DATARW      ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->I80DATARW, regvalue);
}


void NX_LCDINTERFACE_Set_I80REGRW       ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->I80REGRW, regvalue);
}


void NX_LCDINTERFACE_Set_I80TIMING      ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->I80TIMING, regvalue);
}


void NX_LCDINTERFACE_Set_I80POLCTRL     ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->I80POLCTRL, regvalue);
}


void NX_LCDINTERFACE_Set_DISPCMDBUFCTRL0( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPCMDBUFCTRL0, regvalue);
}


void NX_LCDINTERFACE_Set_DISPCMDBUFDATA ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPCMDBUFDATA, regvalue);
}


void NX_LCDINTERFACE_Set_DISPSOURCESEL  ( U32 ModuleIndex, U32 regvalue )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->DISPSOURCESEL, regvalue);
}



void	NX_LCDINTERFACE_Write_i80DATA( U32 ModuleIndex, U32 Data ) // RS==0
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->I80DATARW, Data);
}

U32		NX_LCDINTERFACE_Read_i80DATA( U32 ModuleIndex ) // RS==0
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	return ReadIO32(&pRegister->I80DATARW);
}

void	NX_LCDINTERFACE_Write_i80Reg( U32 ModuleIndex, U32 Data ) // RS==1
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	WriteIO32(&pRegister->I80REGRW, Data);
}

U32		NX_LCDINTERFACE_Read_i80Reg( U32 ModuleIndex ) // RS==1
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	return ReadIO32(&pRegister->I80REGRW);
}


//*********************************************************************************************
// Default Use Function
//*********************************************************************************************


void	NX_LCDINTERFACE_SetLCDIFEnable( U32 ModuleIndex, CBOOL bEnb )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;
	U32 regvalue;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (bEnb == 1) | (bEnb == 0) )

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->DISPCNTL0);
	regvalue = regvalue | (bEnb << 15);

	WriteIO32(&pRegister->DISPCNTL0, regvalue);
}

CBOOL	NX_LCDINTERFACE_GetLCDIFEnable( U32 ModuleIndex )
{
	register NX_LCDINTERFACE_RegisterSet* pRegister;
	U32 regvalue;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];
	NX_ASSERT( CNULL != pRegister );

	regvalue = ReadIO32(&pRegister->DISPCNTL0);

	if( (regvalue >> 15) & 0x01 )
	{
		return CTRUE;
	}
	else
	{
		return CFALSE;
	}
}


//------------------------------------------------------------------------------
/**
 *	@brief		Set delay values for data and sync signals.
 *	@param[in]	ModuleIndex		An index of module ( 0 : First DPC ).
 *	@param[in]	DelayRGB_PVD	Specifies the delay value for RGB/PVD signal, 0 ~ 16.
 *	@param[in]	DelayHS_CP1		Specifies the delay value for HSYNC/CP1 signal, 0 ~ 63.
 *	@param[in]	DelayVS_FRAM	Specifies the delay value for VSYNC/FRAM signal, 0 ~ 63.
 *	@param[in]	DelayDE_CP2		Specifies the delay value for DE/CP2 signal, 0 ~ 63.
 *	@return		None.
 *	@remarks	Set delay value for TFT LCD's data and sync signal.
 *				TFT LCD 
 *				The delay valus for data is generally '0' for normal operation.
 *				but the delay values for sync signals depend on the output format.
 *				The unit is VCLK2.
 *				The setting values for normal operation is as follows,
 *	@code
 *		+-----------------------+-----------+-------------------------------+
 *		|		FORMAT			| DelayRGB	| DelayHS, VS, DE				|
 *		+-----------------------+-----------+-------------------------------+
 *		| RGB					|	0		|				4				|
 *		+-----------------------+-----------+-------------------------------+
 *		| MRGB					|	0		|				8				|
 *		+-----------------------+-----------+-------------------------------+
 *		| ITU-R BT.601A			|	0		|				6				|
 *		+-----------------------+-----------+-------------------------------+
 *		| ITU-R BT.656 / 601B	|	0		|				12				|
 *		+-----------------------+-----------+-------------------------------+
 *	@endcode
 */
void	NX_LCDINTERFACE_SetDelay( U32 ModuleIndex, U32 DelayHS, U32 DelayVS, U32 DelayDE )
{
	const U32 DELYHSYNC_POS	= 0;
	const U32 DELYVSYNC_POS	= 6;
	const U32 DELYDE_POS	= 12;

	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( 64 > DelayHS );
	NX_ASSERT( 64 > DelayVS );
	NX_ASSERT( 64 > DelayDE );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = (DelayDE << DELYDE_POS) | (DelayVS << DELYVSYNC_POS) | (DelayHS << DELYHSYNC_POS);

	WriteIO32(&pRegister->DISPDELYCNT0, regvalue);
}


void	NX_LCDINTERFACE_GetDelay( U32 ModuleIndex, U32 *DelayHS, U32 *DelayVS, U32 *DelayDE )
{
	const U32 DELYHSYNC_POS	= 0;
	const U32 DELYVSYNC_POS	= 6;
	const U32 DELYDE_POS	= 12;

	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPDELYCNT0);

	*DelayDE = (regvalue>>DELYDE_POS)    & 0x3f;
	*DelayVS = (regvalue>>DELYVSYNC_POS) & 0x3f;
	*DelayHS = (regvalue>>DELYHSYNC_POS) & 0x3f;

}


void	NX_LCDINTERFACE_SetDither( U32 ModuleIndex, NX_LCDINTERFACE_DITHER DitherR, NX_LCDINTERFACE_DITHER DitherG, NX_LCDINTERFACE_DITHER DitherB )
{
	const U32 RDITH_POS	= 0;
	const U32 GDITH_POS	= 2;
	const U32 BDITH_POS	= 4;

	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	//NX_ASSERT( 1 != DitherR );
	//NX_ASSERT( 1 != DitherG );
	//NX_ASSERT( 1 != DitherB );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL1) & (~0x3f);
	regvalue |= (DitherB << BDITH_POS) | (DitherG << GDITH_POS) | (DitherR << RDITH_POS);

	WriteIO32(&pRegister->DISPCNTL1, regvalue);
}


void	NX_LCDINTERFACE_GetDither( U32 ModuleIndex, NX_LCDINTERFACE_DITHER *pDitherR, NX_LCDINTERFACE_DITHER *pDitherG, NX_LCDINTERFACE_DITHER *pDitherB )
{
	const U32 RDITH_POS	= 0;
	const U32 GDITH_POS	= 2;
	const U32 BDITH_POS	= 4;

	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL1) & (~0x3f);

	*pDitherR =  (regvalue >> RDITH_POS) & 0x03;
	*pDitherG =  (regvalue >> GDITH_POS) & 0x03;
	*pDitherB =  (regvalue >> BDITH_POS) & 0x03;

}


/*
  Input RGB
-------------------------------------------------------------------------
|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|
|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
-------------------------------------------------------------------------
*/
// RGB Format에 대해서 기본적은 포맷은 지원해준다.
void	NX_LCDINTERFACE_SetRGBMode( U32 ModuleIndex,
							NX_LCDINTERFACE_FORMAT format )
{
	U32 regvalue;
	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( format <= 7 );

	pRegister = __g_pRegister[ModuleIndex];


	if( format == NX_LCDINTERFACE_FORMAT_RGB555)
	{
		/*
		  Output RGB
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 1| 1| 1| 1| 1| 1| 1| 1| 1| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0|
		|  |  |  |  |  |  |  |  |  |R7|R6|R5|R4|R3|G7|G6|G5|G4|G3|B7|B6|B5|B4|B3|
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/
		regvalue = (11 << 25) | (7 << 20) | (6 << 15) | (5 << 10) | (4 << 5) | (3 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (20 << 25) | (19 << 20) | (15 << 15) | (14 << 10) | (13 << 5) | (12 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (2 << 25) | (1 << 20) | (0 << 15) | (23 << 10) | (22 << 5) | (21 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (18 << 25) | (17 << 20) | (16 << 15) | (10 << 10) | (9 << 5) | (8 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	(0<< 23 ) | (0<< 22 ) | (0<< 21 ) | (0<< 20 ) | (0<< 19 ) | (0<< 18 ) |
					(0<< 17 ) | (0<< 16 ) | (0<< 15 ) | (1<< 14 ) | (1<< 13 ) | (1<< 12 ) |
					(1<< 11 ) | (1<< 10 ) | (1<<  9 ) | (1<<  8 ) | (1<<  7 ) | (1<<  6 ) |
					(1<<  5 ) | (1<<  4 ) | (1<<  3 ) | (1<<  2 ) | (1<< 1) | (1<< 0) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);
		NX_LCDINTERFACE_SetDither(ModuleIndex, NX_LCDINTERFACE_DITHER_5BIT, NX_LCDINTERFACE_DITHER_5BIT, NX_LCDINTERFACE_DITHER_5BIT);
	}
	else if( format == NX_LCDINTERFACE_FORMAT_RGB565)
	{
		/*
		  Output RGB
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 0| 0| 0| 0| 0| 1| 1| 1| 0| 0| 0| 0| 0| 0| 1| 1| 0| 0| 0| 0| 0| 1| 1| 1|
		|R7|R6|R5|R4|R3|  |  |  |G7|G6|G5|G4|G3|G2|  |  |B7|B6|B5|B4|B3|  |  |  |
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/
		regvalue = (5 << 25) | (4 << 20) | (3 << 15) | (2 << 10) | (1 << 5) | (0 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (11 << 25) | (10 << 20) | (9 << 15) | (8 << 10) | (7 << 5) | (6 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (17 << 25) | (16 << 20) | (15 << 15) | (14 << 10) | (13 << 5) | (12 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (23 << 25) | (22 << 20) | (21 << 15) | (20 << 10) | (19 << 5) | (18 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 1<< 23 ) | ( 1<< 22 ) | ( 1<< 21 ) | ( 1<< 20 ) | ( 1<< 19 ) | ( 0<< 18 ) |
					( 0<< 17 ) | ( 0<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 1<< 11 ) | ( 1<< 10 ) | ( 0<<  9 ) | ( 0<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 0<<  2 ) | ( 0<<  1 ) | ( 0<<  0 ) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);
		NX_LCDINTERFACE_SetDither(ModuleIndex, NX_LCDINTERFACE_DITHER_5BIT, NX_LCDINTERFACE_DITHER_6BIT, NX_LCDINTERFACE_DITHER_5BIT);
	}
	else if ( format ==NX_LCDINTERFACE_FORMAT_RGB666	)
	{
		/*
		  Output RGB
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 0| 0| 0| 0| 0| 1| 1| 1| 0| 0| 0| 0| 0| 0| 1| 1| 0| 0| 0| 0| 0| 1| 1| 1|
		|R7|R6|R5|R4|R3|R2|  |  |G7|G6|G5|G4|G3|G2|  |  |B7|B6|B5|B4|B3|B2|  |  |
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/
		regvalue = (5 << 25) | (4 << 20) | (3 << 15) | (2 << 10) | (1 << 5) | (0 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (11 << 25) | (10 << 20) | (9 << 15) | (8 << 10) | (7 << 5) | (6 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (17 << 25) | (16 << 20) | (15 << 15) | (14 << 10) | (13 << 5) | (12 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (23 << 25) | (22 << 20) | (21 << 15) | (20 << 10) | (19 << 5) | (18 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 1<< 23 ) | ( 1<< 22 ) | ( 1<< 21 ) | ( 1<< 20 ) | ( 1<< 19 ) | ( 1<< 18 ) |
					( 0<< 17 ) | ( 0<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 1<< 11 ) | ( 1<< 10 ) | ( 0<<  9 ) | ( 0<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 1<<  2 ) | ( 0<<  1 ) | ( 0<<  0 ) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);
		NX_LCDINTERFACE_SetDither(ModuleIndex, NX_LCDINTERFACE_DITHER_6BIT, NX_LCDINTERFACE_DITHER_6BIT, NX_LCDINTERFACE_DITHER_6BIT);
	}

	else if (format == NX_LCDINTERFACE_FORMAT_RGB888	)
	{
		/*
		  Output RGB
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/
		regvalue = (5 << 25) | (4 << 20) | (3 << 15) | (2 << 10) | (1 << 5) | (0 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (11 << 25) | (10 << 20) | (9 << 15) | (8 << 10) | (7 << 5) | (6 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (17 << 25) | (16 << 20) | (15 << 15) | (14 << 10) | (13 << 5) | (12 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (23 << 25) | (22 << 20) | (21 << 15) | (20 << 10) | (19 << 5) | (18 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 1<< 23 ) | ( 1<< 22 ) | ( 1<< 21 ) | ( 1<< 20 ) | ( 1<< 19 ) | ( 1<< 18 ) |
					( 1<< 17 ) | ( 1<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 1<< 11 ) | ( 1<< 10 ) | ( 1<<  9 ) | ( 1<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 1<<  2 ) | ( 1<<  1 ) | ( 1<<  0 ) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);
	}

	else if (format == NX_LCDINTERFACE_FORMAT_MRGB565	)
	{
		/*
		  Output RGB
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 1| 1| 1| 1| 0| 0| 0| 0| 0| 0| 0| 0| 1| 1| 1| 1| 0| 0| 0| 0| 0| 0| 0| 0|

		|  |  |  |  |R7|R6|R5|R4|R3|G7|G6|G5|  |  |  |  |G4|G3|G2|B7|B6|B5|B4|B3|

1st		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |G4|G3|G2|B7|B6|B5|B4|B3|
2nd		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |R7|R6|R5|R4|R3|G7|G6|G5|
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/
		regvalue = (10 << 25) | (7 << 20) | (6 << 15) | (5 << 10) | (4 << 5) | (3 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (8 << 25) | (2 << 20) | (1 << 15) | (0 << 10) | (12 << 5) | (11 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (21 << 25) | (20 << 20) | (19 << 15) | (15 << 10) | (14 << 5) | (13 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (18 << 25) | (17 << 20) | (16 << 15) | (9 << 10) | (23 << 5) | (22 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 0<< 23 ) | ( 0<< 22 ) | ( 0<< 21 ) | ( 0<< 20 ) | ( 1<< 19 ) | ( 1<< 18 ) |
					( 1<< 17 ) | ( 1<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 0<< 11 ) | ( 0<< 10 ) | ( 0<<  9 ) | ( 0<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 1<<  2 ) | ( 1<<  1 ) | ( 1<<  0 ) ;
		NX_CONSOLE_Printf("[DEBUG] NX_LCDINTERFACE_FORMAT_MRGB565 Mask Value = %x", regvalue);
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);

		// Serial Format Setting
		regvalue = ReadIO32(&pRegister->DISPCNTL1);
		regvalue = regvalue & ~(0xf << 8); // Format Masking
		regvalue = regvalue | (NX_LCDINTERFACE_SERIAL_FORMAT_MRGB << 8);

		WriteIO32(&pRegister->DISPCNTL1, regvalue);
	}
	else if (format == NX_LCDINTERFACE_FORMAT_SRGB888	)
	{
		/*
		  Output RGB : SRGB : R, G, B
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0|
		|B7|B6|B5|B4|B3|B2|B1|B0|G7|G6|G5|G4|G3|G2|G1|G0|R7|R6|R5|R4|R3|R2|R1|R0|

1st		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |R7|R6|R5|R4|R3|R2|R1|R0|
2nd		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |G7|G6|G5|G4|G3|G2|G1|G0|
3th		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |B7|B6|B5|B4|B3|B2|B1|B0|

		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/

		regvalue = (21 << 25) | (20 << 20) | (19 << 15) | (18 << 10) | (17 << 5) | (17 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (11 << 25) | (10 << 20) | (9 << 15) | (8 << 10) | (23 << 5) | (22 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (1 << 25) | (0 << 20) | (15 << 15) | (14 << 10) | (13 << 5) | (12 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (7 << 25) | (6 << 20) | (5 << 15) | (4 << 10) | (3 << 5) | (2 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 1<< 23 ) | ( 1<< 22 ) | ( 1<< 21 ) | ( 1<< 20 ) | ( 1<< 19 ) | ( 1<< 18 ) |
					( 1<< 17 ) | ( 1<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 1<< 11 ) | ( 1<< 10 ) | ( 1<<  9 ) | ( 1<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 1<<  2 ) | ( 1<<  1 ) | ( 1<<  0 ) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);

		// Serial Format Setting
		regvalue = ReadIO32(&pRegister->DISPCNTL1);
		regvalue = regvalue & ~(0xf << 8); // Format Masking
		regvalue = regvalue | (NX_LCDINTERFACE_SERIAL_FORMAT_SRGB3 << 8);

		WriteIO32(&pRegister->DISPCNTL1, regvalue);
	}

	else if (format == NX_LCDINTERFACE_FORMAT_SRGBD8888 )
	{
		/*
		  Output RGB : R, G, B, Dummy
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0|
		|B7|B6|B5|B4|B3|B2|B1|B0|G7|G6|G5|G4|G3|G2|G1|G0|R7|R6|R5|R4|R3|R2|R1|R0|

1st		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |R7|R6|R5|R4|R3|R2|R1|R0|
2nd		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |G7|G6|G5|G4|G3|G2|G1|G0|
3th		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |B7|B6|B5|B4|B3|B2|B1|B0|
4rd		|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |

		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/

		regvalue = (21 << 25) | (20 << 20) | (19 << 15) | (18 << 10) | (17 << 5) | (17 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (11 << 25) | (10 << 20) | (9 << 15) | (8 << 10) | (23 << 5) | (22 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (1 << 25) | (0 << 20) | (15 << 15) | (14 << 10) | (13 << 5) | (12 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (7 << 25) | (6 << 20) | (5 << 15) | (4 << 10) | (3 << 5) | (2 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 1<< 23 ) | ( 1<< 22 ) | ( 1<< 21 ) | ( 1<< 20 ) | ( 1<< 19 ) | ( 1<< 18 ) |
					( 1<< 17 ) | ( 1<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 1<< 11 ) | ( 1<< 10 ) | ( 1<<  9 ) | ( 1<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 1<<  2 ) | ( 1<<  1 ) | ( 1<<  0 ) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);

		// Serial Format Setting
		regvalue = ReadIO32(&pRegister->DISPCNTL1);
		regvalue = regvalue & ~(0xf << 8); // Format Masking
		regvalue = regvalue | (NX_LCDINTERFACE_SERIAL_FORMAT_SRGB4 << 8);

		WriteIO32(&pRegister->DISPCNTL1, regvalue);
	}
	else if( format == NX_LCDINTERFACE_FORMAT_RGB565_I80)
	{
		/*
		  Output RGB
		-------------------------------------------------------------------------
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		|R7|R6|R5|R4|R3|R2|R1|R0|G7|G6|G5|G4|G3|G2|G1|G0|B7|B6|B5|B4|B3|B2|B1|B0|

Mask	| 0| 0| 0| 0| 0| 0| 0| 0| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1|
		|  |  |  |  |  |  |  |  |R7|R6|R5|R4|R3|G7|G6|G5|G4|G3|G2|B7|B6|B5|B4|B3|
		|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
		-------------------------------------------------------------------------
		*/
		regvalue = (10 << 25) | (7 << 20) | (6 << 15) | (5 << 10) | (4 << 5) | (3 << 0);
		WriteIO32(&pRegister->DISPPADLOC0, regvalue);

		regvalue = (19 << 25) | (15 << 20) | (14 << 15) | (13 << 10) | (12 << 5) | (11 << 0);
		WriteIO32(&pRegister->DISPPADLOC1, regvalue);

		regvalue = (0 << 25) | (0 << 20) | (23 << 15) | (22 << 10) | (21 << 5) | (20 << 0);
		WriteIO32(&pRegister->DISPPADLOC2, regvalue);

		regvalue = (0 << 25) | (0 << 20) | (0 << 15) | (0 << 10) | (0 << 5) | (0 << 0);
		WriteIO32(&pRegister->DISPPADLOC3, regvalue);

		regvalue = ReadIO32(&pRegister->DISPRGBMASK) & (0xff000000); // RGBMASK Masking
		regvalue |=	( 0<< 23 ) | ( 0<< 22 ) | ( 0<< 21 ) | ( 0<< 20 ) | ( 0<< 19 ) | ( 0<< 18 ) |
					( 0<< 17 ) | ( 0<< 16 ) | ( 1<< 15 ) | ( 1<< 14 ) | ( 1<< 13 ) | ( 1<< 12 ) |
					( 1<< 11 ) | ( 1<< 10 ) | ( 1<<  9 ) | ( 1<<  8 ) | ( 1<<  7 ) | ( 1<<  6 ) |
					( 1<<  5 ) | ( 1<<  4 ) | ( 1<<  3 ) | ( 1<<  2 ) | ( 1<<  1 ) | ( 1<<  0 ) ;
		WriteIO32(&pRegister->DISPRGBMASK, regvalue);
		NX_LCDINTERFACE_SetDither(ModuleIndex, NX_LCDINTERFACE_DITHER_5BIT, NX_LCDINTERFACE_DITHER_6BIT, NX_LCDINTERFACE_DITHER_5BIT);
	}

}




void	NX_LCDINTERFACE_SetRGBShift( U32 ModuleIndex, U32 RGBSHIFT )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( RGBSHIFT <= 0x1f );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPRGBMASK) & 0x01ffffff;
	regvalue |= (RGBSHIFT << 25);

	WriteIO32(&pRegister->DISPRGBMASK, regvalue);
}


U32	NX_LCDINTERFACE_GetRGBShift( U32 ModuleIndex )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = (ReadIO32(&pRegister->DISPRGBMASK) >> 25) & 0x1f;

	return regvalue;
}


void	NX_LCDINTERFACE_SetSerialFormat( U32 ModuleIndex, NX_LCDINTERFACE_SERIAL_FORMAT SerialFormat )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( SerialFormat < 4 );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL1);
	regvalue = regvalue & ~(0xf << 8); // Format Masking
	regvalue = regvalue | (SerialFormat << 8);

	WriteIO32(&pRegister->DISPCNTL1, regvalue);
}



NX_LCDINTERFACE_SERIAL_FORMAT	NX_LCDINTERFACE_GetSerialFormat( U32 ModuleIndex )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL1);
	regvalue = (regvalue >> 8) & 0x03;

	return regvalue;
}

void	NX_LCDINTERFACE_Seti80Mode( U32 ModuleIndex, CBOOL Enable )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (Enable == CTRUE) | (Enable == CFALSE) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL1);
	if( Enable == CTRUE )
		regvalue = regvalue | (1<<12);
	else
		regvalue = regvalue & ~(1<<12);

	WriteIO32(&pRegister->DISPCNTL1, regvalue);
}


CBOOL	NX_LCDINTERFACE_Geti80Mode( U32 ModuleIndex)
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL1);
	regvalue = (regvalue >> 12) & 0x01;

	return regvalue;
}

// Dirty Flag는 한 프레임에 한번만 적용되며 프레임이 끝나면 자동Clear됨.

void	NX_LCDINTERFACE_SetDirtyFlag( U32 ModuleIndex, CBOOL Enable)
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (Enable == CTRUE) | (Enable == CFALSE) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL0);
	if( Enable == CTRUE )
		regvalue = regvalue | (1<<31);
	else
		regvalue = regvalue & ~(1<<31);

	WriteIO32(&pRegister->DISPCNTL0, regvalue);
}


// DirtyFlag가 Clear되면 CTRUE
// Clear되지 않으면 CFALSE
CBOOL	NX_LCDINTERFACE_GetDirtyFlagClear( U32 ModuleIndex)
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL0);

	// Dirty Flag가 남아있으므로 Clear되지 않았으므로 CFALSE
	if( 0x01 == (regvalue >> 31 ) & 0x01  )
		return CFALSE;
	else
		return CTRUE;

}


//*********************************************************************************************
// i80 Interface
//*********************************************************************************************

//@added choiyk 2012-10-30 오후 3:41:26
void NX_LCDINTERFACE_SetCmdBufferClear( U32 ModuleIndex)
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );


	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCMDBUFCTRL0);
	regvalue = regvalue & 0x01;

	WriteIO32(&pRegister->DISPCMDBUFCTRL0, regvalue);
}

void NX_LCDINTERFACE_SetCmdBufferWrite( U32 ModuleIndex, U32 regvalue)
{

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );


	pRegister = __g_pRegister[ModuleIndex];
	WriteIO32(&pRegister->DISPCMDBUFDATA, regvalue);
}

void NX_LCDINTERFACE_SetCmdBufferFlush( U32 ModuleIndex, CBOOL Enb )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (Enb == CTRUE) | (Enb == CFALSE) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCMDBUFCTRL0);

	if( Enb == CTRUE ) regvalue = 0x02;
	else regvalue = 0;

	WriteIO32(&pRegister->DISPCMDBUFCTRL0, regvalue);
}



CBOOL NX_LCDINTERFACE_GetInterruptPendingAll( U32 ModuleIndex )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = (ReadIO32(&pRegister->DISPCNTL0) >> 10) & 0x01;

	if( regvalue == 0x01 ) return CTRUE;
	else return CFALSE;
}

void NX_LCDINTERFACE_ClearInterruptPendingAll( U32 ModuleIndex )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL0) ;

	regvalue = regvalue | (0x01<<10);

	WriteIO32(&pRegister->DISPCNTL0, regvalue);
}


void	NX_LCDINTERFACE_SetTFTPolarity( U32 ModuleIndex, CBOOL PHS, CBOOL PVS, CBOOL PDE)
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( (PHS == 1) | (PHS == 0) );
	NX_ASSERT( (PVS == 1) | (PVS == 0) );
	NX_ASSERT( (PDE == 1) | (PDE == 0) );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL0) & (~0x07);

	regvalue = regvalue | (PDE<<2) | (PVS<<1) | (PHS<<0);

	WriteIO32(&pRegister->DISPCNTL0, regvalue);
}


//typedef enum {
// NX_LCDIF_PADVCLK = 0,
// NX_LCDIF_PADPixelCLK = 1,
// NX_LCDIF_VCLK_DIV2 = 2, // SRGB888의 경우 이 클럭을 선택해야 한다.
//}PADCLK_SELECT;

// I80DATARW, I80REGRW

void	NX_LCDINTERFACE_SetReg( U32 ModuleIndex, U32 Data ) // RS==0
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->I80REGRW, Data);
}

U32		NX_LCDINTERFACE_GetStatus( U32 ModuleIndex ) // RS==0
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return ReadIO32(&pRegister->I80REGRW);
}

void	NX_LCDINTERFACE_SetData( U32 ModuleIndex, U32 Data ) // RS==1
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	WriteIO32(&pRegister->I80DATARW, Data);
}

U32		NX_LCDINTERFACE_GetData( U32 ModuleIndex ) // RS==1
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	return ReadIO32(&pRegister->I80DATARW);
}

void	NX_LCDINTERFACE_Set_i80Timing( U32 ModuleIndex, U32 tSetup, U32 tHold, U32 tAcc)
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;


    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = (tAcc <<  20)| (tHold <<  10) | (tSetup <<  0);

	WriteIO32(&pRegister->I80TIMING, regvalue);
}







void NX_LCDINTERFACE_SetOutputClock( U32 ModuleIndex, U32 SourceSel )
{
	U32 regvalue;

	register NX_LCDINTERFACE_RegisterSet*	pRegister;

    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
	NX_ASSERT( SourceSel < 4 );

	pRegister = __g_pRegister[ModuleIndex];

	regvalue = ReadIO32(&pRegister->DISPCNTL0) & (~(0x07<<16));

	regvalue = regvalue | (SourceSel <<16);

	WriteIO32(&pRegister->DISPCNTL0, regvalue);
}


//*********************************************************************************************

// @fixed choiyk 2012-10-30 오후 2:28:30 LCDIF별도로 CLKGEN이 존재한다.

////*********************************************************************************************
//// CLKGEN
////*********************************************************************************************
//void	NX_LCDINTERFACE_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode )
//{
//	//const U32 PCLKMODE_POS	=	0;
//
//	register U32 regvalue;
//	U32 clkmode=0;
//
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//
//	switch(mode)
//	{
//		case NX_BCLKMODE_DISABLE:	clkmode = 0;
//		case NX_BCLKMODE_DYNAMIC:	clkmode = 2;		break;
//		case NX_BCLKMODE_ALWAYS:	clkmode = 3;		break;
//		default: NX_ASSERT( CFALSE );
//	}
//
//	regvalue = ReadIO32(&pRegister->CLKENB);
//
//	regvalue &= ~3UL;
//	regvalue |= ( clkmode & 0x03 );
//
////	pRegister->CLKENB = regvalue;
//	WriteIO32(&pRegister->CLKENB, regvalue);
//}
//
//NX_BCLKMODE	NX_LCDINTERFACE_GetClockBClkMode( U32 ModuleIndex )
//{
//	//const U32 PCLKMODE_POS	= 0;
//	U32 mode=0;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	mode = ( ReadIO32(&pRegister->CLKENB) & 3UL );
//	switch(mode)
//	{
//		case 0: return NX_BCLKMODE_DISABLE;
//		case 2: return NX_BCLKMODE_DYNAMIC;
//		case 3: return NX_BCLKMODE_ALWAYS ;
//		default: NX_ASSERT( CFALSE );
//	}
//	return	NX_BCLKMODE_DISABLE;
//}
//
//
//void	NX_LCDINTERFACE_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode )
//{
//	const U32 PCLKMODE_POS	=	3;
//
//	register U32 regvalue;
//
//	U32 clkmode=0;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	switch(mode)
//	{
//		case NX_PCLKMODE_DYNAMIC:	clkmode = 0;		break;
//		case NX_PCLKMODE_ALWAYS:	clkmode = 1;		break;
//		default: NX_ASSERT( CFALSE );
//	}
//
//	regvalue = ReadIO32(&pRegister->CLKENB);
//
//	regvalue &= ~(1UL<<PCLKMODE_POS);
//	regvalue |= ( clkmode & 0x01 ) << PCLKMODE_POS;
//
////	pRegister->CLKENB = regvalue;
//	WriteIO32(&pRegister->CLKENB, regvalue);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Get current PCLK mode
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@return		Current PCLK mode
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,
// *				NX_LCDINTERFACE_SetClockSource,			NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_SetClockDivisor,		NX_LCDINTERFACE_GetClockDivisor,
// *				NX_LCDINTERFACE_SetClockDivisorEnable,	NX_LCDINTERFACE_GetClockDivisorEnable
// */
//NX_PCLKMODE	NX_LCDINTERFACE_GetClockPClkMode( U32 ModuleIndex )
//{
//	const U32 PCLKMODE_POS	= 3;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	if( ReadIO32(&pRegister->CLKENB) & ( 1UL << PCLKMODE_POS ) )
//	{
//		return NX_PCLKMODE_ALWAYS;
//	}
//
//	return	NX_PCLKMODE_DYNAMIC;
//}
//
//
////------------------------------------------------------------------------------
///**
// *	@brief		Set clock source of clock generator
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@param[in]	Index	Select clock generator( 0 : clock generator 0 );
// *	@param[in]	ClkSrc	Select clock source of clock generator ( 0: PLL0, 1:PLL1 ).
// *	@remarks	CLKGEN have one clock generator. so  Index must set to 0.
// *	@return		None.
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,		NX_LCDINTERFACE_GetClockPClkMode,
// *				NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_SetClockDivisor,		NX_LCDINTERFACE_GetClockDivisor,
// *				NX_LCDINTERFACE_SetClockDivisorEnable,	NX_LCDINTERFACE_GetClockDivisorEnable
// */
//void	NX_LCDINTERFACE_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc )
//{
//	const U32 CLKSRCSEL_POS		= 2;
//	const U32 CLKSRCSEL_MASK	= 0x07 << CLKSRCSEL_POS;
//
//	register U32 ReadValue;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	ReadValue = ReadIO32(&pRegister->CLKGEN[Index<<1]);
//
//	ReadValue &= ~CLKSRCSEL_MASK;
//	ReadValue |= ClkSrc << CLKSRCSEL_POS;
//
////	pRegister->CLKGEN[Index<<1] = ReadValue;
//	WriteIO32(&pRegister->CLKGEN[Index<<1], ReadValue);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Get clock source of specified clock generator.
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@param[in]	Index	Select clock generator( 0 : clock generator 0 );
// *	@return		Clock source of clock generator ( 0:PLL0, 1:PLL1 ).
// *	@remarks	CLKGEN have one clock generator. so  Index must set to 0.
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,		NX_LCDINTERFACE_GetClockPClkMode,
// *				NX_LCDINTERFACE_SetClockSource,			NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_SetClockDivisor,		NX_LCDINTERFACE_GetClockDivisor,
// *				NX_LCDINTERFACE_SetClockDivisorEnable,	NX_LCDINTERFACE_GetClockDivisorEnable
// */
//U32				NX_LCDINTERFACE_GetClockSource( U32 ModuleIndex, U32 Index )
//{
//	const U32 CLKSRCSEL_POS		= 2;
//	const U32 CLKSRCSEL_MASK	= 0x07 << CLKSRCSEL_POS;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	return ( ReadIO32(&pRegister->CLKGEN[Index<<1]) & CLKSRCSEL_MASK ) >> CLKSRCSEL_POS;
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Set clock divisor of specified clock generator.
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@param[in]	Index		Select clock generator( 0 : clock generator 0 )
// *	@param[in]	Divisor		Clock divisor ( 1 ~ 256 ).
// *	@return		None.
// *	@remarks	CLKGEN have one clock generator. so  Index must set to 0.
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,		NX_LCDINTERFACE_GetClockPClkMode,
// *				NX_LCDINTERFACE_SetClockSource,			NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_GetClockDivisor,
// *				NX_LCDINTERFACE_SetClockDivisorEnable,	NX_LCDINTERFACE_GetClockDivisorEnable
// */
//void			NX_LCDINTERFACE_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor )
//{
//	const U32 CLKDIV_POS	=	5;
//	const U32 CLKDIV_MASK	=	0xFF << CLKDIV_POS;
//
//	register U32 ReadValue;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	ReadValue	=	ReadIO32(&pRegister->CLKGEN[Index<<1]);
//
//	ReadValue	&= ~CLKDIV_MASK;
//	ReadValue	|= (Divisor-1) << CLKDIV_POS;
//
//	WriteIO32(&pRegister->CLKGEN[Index<<1], ReadValue);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Get clock divisor of specified clock generator.
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@param[in]	Index		Select clock generator( 0 : clock generator 0	);
// *	@return		Clock divisor ( 1 ~ 256 ).
// *	@remarks	CLKGEN have one clock generator. so  Index must set to 0.
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,		NX_LCDINTERFACE_GetClockPClkMode,
// *				NX_LCDINTERFACE_SetClockSource,			NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_SetClockDivisor,
// *				NX_LCDINTERFACE_SetClockDivisorEnable,	NX_LCDINTERFACE_GetClockDivisorEnable
// */
//U32				NX_LCDINTERFACE_GetClockDivisor( U32 ModuleIndex, U32 Index )
//{
//	const U32 CLKDIV_POS	=	5;
//	const U32 CLKDIV_MASK	=	0xFF << CLKDIV_POS;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	return ((ReadIO32(&pRegister->CLKGEN[Index<<1]) & CLKDIV_MASK) >> CLKDIV_POS) + 1;
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Set clock generator's operation
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@param[in]	Enable	CTRUE	indicates that Enable of clock generator. 
// *						CFALSE	indicates that Disable of clock generator.
// *	@return		None.
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,		NX_LCDINTERFACE_GetClockPClkMode,
// *				NX_LCDINTERFACE_SetClockSource,			NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_SetClockDivisor,		NX_LCDINTERFACE_GetClockDivisor,
// *				NX_LCDINTERFACE_GetClockDivisorEnable
// */
//void			NX_LCDINTERFACE_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable )
//{
//	const U32	CLKGENENB_POS	=	2;
//	const U32	CLKGENENB_MASK	=	1UL << CLKGENENB_POS;
//
//	register U32 ReadValue;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	ReadValue	=	ReadIO32(&pRegister->CLKENB);
//	//ReadValue	=	NX_LCDINTERFACE_GetClockDivisorEnable(ModuleIndex);
//	ReadValue	&=	~CLKGENENB_MASK;
//	ReadValue	|= (U32)Enable << CLKGENENB_POS;
//
////	pRegister->CLKENB	=	ReadValue;
//	WriteIO32(&pRegister->CLKENB, ReadValue);
//}
//
////------------------------------------------------------------------------------
///**
// *	@brief		Get status of clock generator's operation
// *	@param[in]	ModuleIndex		An index of module ( 0 ~ 1 ).
// *	@return		CTRUE	indicates that Clock generator is enabled. 
// *				CFALSE	indicates that Clock generator is disabled.
// *	@see		NX_LCDINTERFACE_SetClockPClkMode,		NX_LCDINTERFACE_GetClockPClkMode,
// *				NX_LCDINTERFACE_SetClockSource,			NX_LCDINTERFACE_GetClockSource,
// *				NX_LCDINTERFACE_SetClockDivisor,		NX_LCDINTERFACE_GetClockDivisor,
// *				NX_LCDINTERFACE_SetClockDivisorEnable
// */
//CBOOL			NX_LCDINTERFACE_GetClockDivisorEnable( U32 ModuleIndex )
//{
//	const U32	CLKGENENB_POS	=	2;
//	const U32	CLKGENENB_MASK	=	1UL << CLKGENENB_POS;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	return	(CBOOL)( (ReadIO32(&pRegister->CLKENB) & CLKGENENB_MASK) >> CLKGENENB_POS );
//}
//
//void			NX_LCDINTERFACE_SetClockOutInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv )
//{
//	const U32 OUTCLKINV_POS	=	1;
//	const U32 OUTCLKINV_MASK	=	1UL << OUTCLKINV_POS;
//
//	register U32 ReadValue;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	ReadValue	=	ReadIO32(&pRegister->CLKGEN[Index<<1]);
//
//	ReadValue	&=	~OUTCLKINV_MASK;
//	ReadValue	|=	OutClkInv << OUTCLKINV_POS;
//
//	//pRegister->CLKGEN[Index<<1]	=	ReadValue;
//	WriteIO32(&pRegister->CLKGEN[Index<<1], ReadValue);
//}
//
//CBOOL			NX_LCDINTERFACE_GetClockOutInv( U32 ModuleIndex, U32 Index )
//{
//	const U32 OUTCLKINV_POS		=	1;
//	const U32 OUTCLKINV_MASK	=	1UL << OUTCLKINV_POS;
//
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//	NX_ASSERT( 2 > Index );
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//	NX_ASSERT( CNULL != pRegister );
//
//	return (CBOOL)((ReadIO32(&pRegister->CLKGEN[Index<<1]) & OUTCLKINV_MASK ) >> OUTCLKINV_POS);
//}
//
////void			NX_LCDINTERFACE_SetClockInInv( U32 ModuleIndex, CBOOL InClkInv )
////{
////	const U32 INCLKINV_POS	=	4;
////	const U32 INCLKINV_MASK	=	1UL << INCLKINV_POS;
////
////	register U32 ReadValue;
////
////	NX_ASSERT( (0==InClkInv) ||(1==InClkInv) );
////	NX_ASSERT( CNULL != pRegister )
////
////	ReadValue	=	pRegister->CLKENB;
////
////	ReadValue	&=	~INCLKINV_MASK;
////	ReadValue	|=	InClkInv << INCLKINV_POS;
////
////	pRegister->CLKENB	=	ReadValue;
////	WriteIO32(&pRegister->CLKENB, ReadValue);
////}
////
////CBOOL			NX_LCDINTERFACE_GetClockInInv( U32 ModuleIndex)
////{
////	const U32 INCLKINV_POS		=	4;
////	const U32 INCLKINV_MASK	=	1UL << INCLKINV_POS;
////
////	NX_ASSERT( CNULL != pRegister );
////
////	return (CBOOL)((pRegister->CLKENB & INCLKINV_MASK ) >> INCLKINV_POS);
////}
//
//CBOOL		NX_LCDINTERFACE_SetInputInv( U32 ModuleIndex, U32 Index, CBOOL InClkInv )
//{
//	const U32 INCLKINV_POS	=	4 + Index;
//	const U32 INCLKINV_MASK	=	1UL << INCLKINV_POS;
//
//	register U32 ReadValue;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//
//
//
//	NX_ASSERT( (0==InClkInv) ||(1==InClkInv) );
//	ReadValue	=	ReadIO32(&pRegister->CLKENB);
//
//	ReadValue	&=	~INCLKINV_MASK;
//	ReadValue	|=	InClkInv << INCLKINV_POS;
//
//	//pRegister->CLKENB	=	ReadValue;
//	WriteIO32(&pRegister->CLKENB, ReadValue);
//}
//
//CBOOL		NX_LCDINTERFACE_GetInputInv( U32 ModuleIndex, U32 Index )
//{
//	const U32 INCLKINV_POS	=	4 + Index;
//	const U32 INCLKINV_MASK	=	1UL << INCLKINV_POS;
//
//	register NX_LCDINTERFACE_RegisterSet* pRegister;
//
//
//    NX_ASSERT( NUMBER_OF_LCDINTERFACE_MODULE > ModuleIndex );
//	NX_ASSERT( CNULL != __g_pRegister[ModuleIndex] );
//
//	pRegister = __g_pRegister[ModuleIndex];
//
//	return (CBOOL)((ReadIO32(&pRegister->CLKENB) & INCLKINV_MASK ) >> INCLKINV_POS);
//}
//*********************************************************************************************




