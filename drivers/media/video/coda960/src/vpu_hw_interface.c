#include "vpu_hw_interface.h"

#define	NX_DTAG		"[DRV|REG_ACC]"
#include "../include/drv_osapi.h"

#define DBG_VBS 0

//----------------------------------------------------------------------------
//						Register Interface
void __VpuWriteReg( unsigned int address, unsigned int value )
{
//	NX_DbgMsg(1, ("write( 0x%08x, 0x%08x )\n", address, value));
	NX_DbgMsg(NX_REG_EN_MSG, ("write( 0x%08x, 0x%08x )\n", address, value));
	(*((volatile unsigned int *)(address))) = value;
}

unsigned int __VpuReadReg( unsigned int address )
{
	unsigned int value = (*((volatile unsigned int *)(address)));
//	NX_DbgMsg(1, ("read ( 0x%08x, 0x%08x )\n", address, value));
	NX_DbgMsg(NX_REG_EN_MSG, ("read( 0x%08x, 0x%08x )\n", address, value));
	return value;
}


//----------------------------------------------------------------------------
//						Host Command
void VpuBitIssueCommand(NX_VpuCodecInst *inst, NX_VPU_CMD cmd)
{
	NX_DbgMsg( DBG_VBS, ("VpuBitIssueCommand : cmd = %d, address=0x%.8x, instIndex=%d, codecMode=%d, auxMode=%d\n", 
		cmd, inst->instBufPhyAddr, inst->instIndex, inst->codecMode, inst->auxMode) );

	VpuWriteReg(BIT_WORK_BUF_ADDR, inst->instBufPhyAddr);
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_RUN_INDEX, inst->instIndex);
	VpuWriteReg(BIT_RUN_COD_STD, inst->codecMode);
	VpuWriteReg(BIT_RUN_AUX_STD, inst->auxMode);
	VpuWriteReg(BIT_RUN_COMMAND, cmd);
}
