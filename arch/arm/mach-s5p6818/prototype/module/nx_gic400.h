//	Copyright (C) 2009 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: GIC 400 interrupt controller
//	File		: nx_gic400.h
//	Description	:
//	Author		: Firmware Team
//	History		:
//				2014.09.10 Hans create
//------------------------------------------------------------------------------
#ifndef __NX_GIC400_H__
#define __NX_GIC400_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	GIC-400 Base
//------------------------------------------------------------------------------
//@{

	struct NX_GIC400_GICD_RegisterSet	// Distributor
	{
		volatile U32 CTLR;				// 0x000 Distributor Control
		volatile U32 TYPER;				// 0x004 Interrupt Controller Type
		volatile U32 IIDR;				// 0x008 Identification 0x0200143B
		volatile U32 _Reserved0[29];	// 0x00C~0x07C
		volatile U32 IGROUPR[16];		// 0x080~0x0BC Interrupt Group
		volatile U32 _Reserved1[16];	// 0x0C0~0x0FC
		volatile U32 ISENABLER[16];		// 0x100~0x13C Interrupt Set-Enable
		volatile U32 _Reserved2[16];	// 0x140~0x17C
		volatile U32 ICENABLER[16];		// 0x180~0x1BC Interrupt Clear-Enable
		volatile U32 _Reserved3[16];	// 0x1C0~0x1FC
		volatile U32 ISPENDR[16];		// 0x200~0x23C Interrupt Set-Pending
		volatile U32 _Reserved4[16];	// 0x240~0x27C
		volatile U32 ICPENDR[16];		// 0x280~0x2BC Interrupt Clear-Pending
		volatile U32 _Reserved5[16];	// 0x2C0~0x2FC
		volatile U32 ISACTIVER[16];		// 0x300~0x33C Interrupt Set-Active
		volatile U32 _Reserved6[16];	// 0x340~0x37C
		volatile U32 ICACTIVER[16];		// 0x380~0x3BC Interrupt Clear-Active
		volatile U32 _Reserved7[16];	// 0x3C0~0x3FC
		volatile U32 IPRIORITYR[128];	// 0x400~0x5FC Interrupt Priority
		volatile U32 _Reserved8[128];	// 0x600~0x7FC
		volatile U32 ITARGETSR[8];		// 0x800~0x81C Interrupt Processor Targets
		volatile U32 _Reserved9[248];	// 0x820~0xBFC
		volatile U32 ICFGR[32];			// 0xC00~0xC7C Interrupt Configuration
		volatile U32 _Reserved10[32];	// 0xC80~0xCFC
		volatile U32 PPISR[16];			// 0xD00~0xD3C Private & Shared Peripheral Interrupt Status
		volatile U32 _Reserved11[112];	// 0xD40~0xEFC
		volatile U32 SGIR;				// 0xF00 Software Generated Interrupt
		volatile U32 _Reserved12[3];	// 0xF04~0xF0C
		volatile U32 CPENDSGIR[4];		// 0xF10~0xF1C SGI Clear-Pending
		volatile U32 SPENDSGIR[4];		// 0xF20~0xF2C SGI Set-Pending
		volatile U32 _Reserved13[40];	// 0xF30~0xFCC
		volatile U32 PIDR[8];			// 0xFD0~0xFEC Peripheral IDx
		volatile U32 CIDR[4];			// 0xFF0~0xFFC Component IDx
	};

	struct NX_GIC400_GICC_RegisterSet	// CPU Interface
	{
		volatile U32 CTLR;				// 0x0000 CPU Interface Control
		volatile U32 PMR;				// 0x0004 Interrupt Priority Mask
		volatile U32 BPR;				// 0x0008 Binary Point
		volatile U32 IAR;				// 0x000C Interrupt Acknowledge
		volatile U32 EOIR;				// 0x0010 End of Interrupt
		volatile U32 RPR;				// 0x0014 Running Priority
		volatile U32 HPPIR;				// 0x0018 Highest Priority Pending Interrupt
		volatile U32 ABPR;				// 0x001C Aliased Binary Point
		volatile U32 AIAR;				// 0x0020 Aliased Interrupt Acknowledge
		volatile U32 AEOIR;				// 0x0024 Aliased End of Interrupt
		volatile U32 AHPPIR;			// 0x0028 Aliased Highest Priority Pending Interrupt
		volatile U32 _Reserved0[41];	// 0x002C~0x00CC
		volatile U32 APR0;				// 0x00D0 Active Priority
		volatile U32 _Reserved1[3];		// 0x00D4~0x00DC
		volatile U32 NSAPR0;			// 0x00E0 Non-Secure Active Priority
		volatile U32 _Reserved2[6];		// 0x00E4~0x00F8
		volatile U32 IIDR;				// 0x00FC CPU Interface Identification, 0x0202143B
		volatile U32 _Reserved3[0x3C0];	// 0x0100~0x0FFC
		volatile U32 DIR;				// 0x1000 Deactivate Interrupt
		volatile U32 _Reserved4[0x3FF];	// 0x1004~0x1FFC
	};

	struct NX_GIC400_GICH_RegisterSet	// Virtual Interface Control
	{
		volatile U32 HCR;				// 0x000 Hypervisor Control
		volatile U32 VTR;				// 0x004 VGIC Type
		volatile U32 VMCR;				// 0x008 Virtual Machine Control
		volatile U32 _Reserved0;		// 0x00C
		volatile U32 MISR;				// 0x010 Maintenance Interrupt Status
		volatile U32 _Reserved1[3];		// 0x014~0x01C
		volatile U32 EISR0;				// 0x020 End of Interrupt Status
		volatile U32 _Reserved2[3];		// 0x024~0x02C
		volatile U32 ELSR0;				// 0x030 Empty List register Status
		volatile U32 _Reserved3[47];	// 0x034~0x0EC
		volatile U32 APR0;				// 0x0F0 Active Priority
		volatile U32 _Reserved4[3];		// 0x0F4~0x0FC
		volatile U32 LR[4];				// 0x100~0x10C List
		volatile U32 _Reserved5[0x3C];	// 0x110~0xFFC
	};

	struct	NX_GIC400_RegisterSet
	{
		volatile U32 _Reserved0[0x400];				// 0x0000~0x0FFC Reserved
		struct NX_GIC400_GICD_RegisterSet GICD;		// 0x1000~0x1FFC Distributor
		struct NX_GIC400_GICC_RegisterSet GICC;		// 0x2000~0x3FFC CPU interfaces
		struct NX_GIC400_GICH_RegisterSet GICH;		// 0x4000~0x41FC Virtual interface control block, for the processor that is performing the access
		volatile U32 _Reserved1[0x380];				// 0x4200~0x4FFC
		struct NX_GIC400_GICH_RegisterSet GICHv[8];	// 0x5000~0x5FFC Virtual interface control block, for the processor selected by address bits[11:9]
		struct NX_GIC400_GICC_RegisterSet GICCv;	// 0x6000~0x7FFC Virtual CPU interface
	};



//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_GIC400_Initialize( void );
U32		NX_GIC400_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_GIC400_GetPhysicalAddress( void );
U32		NX_GIC400_GetSizeOfRegisterSet( void );

void	NX_GIC400_SetBaseAddress( void* BaseAddress );
void*	NX_GIC400_GetBaseAddress( void );

CBOOL	NX_GIC400_OpenModule( void );
CBOOL	NX_GIC400_CloseModule( void );
CBOOL	NX_GIC400_CheckBusy( void );
CBOOL	NX_GIC400_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
// there is no interrupt interface
//@}

//------------------------------------------------------------------------------
/// @name	Clock Management
//@{
// there is no clock interface
//@}

//------------------------------------------------------------------------------
/// @name	Power Management
//@{
//@}

//------------------------------------------------------------------------------
/// @name	Reset Management
//@{
// will find reset feature
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_GIC400_H__

