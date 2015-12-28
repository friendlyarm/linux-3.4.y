#ifndef __VPU_HW_INTERFACE_H__
#define	__VPU_HW_INTERFACE_H__

#include "../include/nx_vpu_config.h"
#include "regdefine.h"
#include "../include/nx_vpu_api.h"

#ifdef __cplusplus
extern "C"{
#endif

//----------------------------------------------------------------------------
//						Register Interface

#if (!defined(NX_REG_ACC_DEBUG))
	#define		VpuWriteReg( ADDR, VALUE )			(*((volatile unsigned int *)(ADDR))) = VALUE
	#define		VpuReadReg( ADDR)					(*((volatile unsigned int *)(ADDR)))

	#define		WriteRegNoMsg( ADDR, VALUE )		(*((volatile unsigned int *)(ADDR))) = VALUE
	#define		ReadRegNoMsg( ADDR)					(*((volatile unsigned int *)(ADDR)))

#else
	void __VpuWriteReg( unsigned int address, unsigned int value );
	unsigned int __VpuReadReg( unsigned int address );
	#define		VpuWriteReg( ADDR, VALUE )			__VpuWriteReg( ADDR, VALUE )
	#define		VpuReadReg( ADDR)					__VpuReadReg( ADDR )

	#define		WriteRegNoMsg( ADDR, VALUE )			(*((volatile unsigned int *)(ADDR))) = VALUE
	#define		ReadRegNoMsg( ADDR)						(*((volatile unsigned int *)(ADDR)))

#endif

//----------------------------------------------------------------------------
//					Host Command Interface
void VpuBitIssueCommand(NX_VpuCodecInst *inst, NX_VPU_CMD cmd);

#ifdef __cplusplus
};
#endif

#endif	//	__VPU_HW_INTERFACE_H__
