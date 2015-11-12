/**
 * Header file of MobiCore Driver Kernel Module.
 *
 * @addtogroup MobiCore_Driver_Kernel_Module
 * @{
 * Internal structures of the McDrvModule
 * @file
 *
 * MobiCore Fast Call interface
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _MC_FASTCALL_H_
#define _MC_FASTCALL_H_

#include "debug.h"

/**
 * MobiCore SMCs
 */
#define MC_SMC_N_YIELD		0x3 /**< Yield to switch from NWd to SWd. */
#define MC_SMC_N_SIQ		0x4  /**< SIQ to switch from NWd to SWd. */

/**
 * MobiCore fast calls. See MCI documentation
 */
#define MC_FC_INIT		-1
#define MC_FC_INFO		-2
#define MC_FC_POWER		-3
#define MC_FC_DUMP		-4
#define MC_FC_NWD_TRACE		-31 /**< Mem trace setup fastcall */


/**
 * return code for fast calls
 */
#define MC_FC_RET_OK				0
#define MC_FC_RET_ERR_INVALID			1
#define MC_FC_RET_ERR_ALREADY_INITIALIZED	5


/* structure wrappers for specific fastcalls */

/** generic fast call parameters */
union fc_generic {
	struct {
		uint32_t cmd;
		uint32_t param[3];
	} as_in;
	struct {
		uint32_t resp;
		uint32_t ret;
		uint32_t param[2];
	} as_out;
};


/** fast call init */
union mc_fc_init {
	union fc_generic as_generic;
	struct {
		uint32_t cmd;
		uint32_t base;
		uint32_t nq_info;
		uint32_t mcp_info;
	} as_in;
	struct {
		uint32_t resp;
		uint32_t ret;
		uint32_t rfu[2];
	} as_out;
};


/** fast call info parameters */
union mc_fc_info {
	union fc_generic as_generic;
	struct {
		uint32_t cmd;
		uint32_t ext_info_id;
		uint32_t rfu[2];
	} as_in;
	struct {
		uint32_t resp;
		uint32_t ret;
		uint32_t state;
		uint32_t ext_info;
	} as_out;
};

/**
 * fast call to MobiCore
 *
 * @param fc_generic pointer to fast call data
 */
static inline long _smc(void *data)
{
	union fc_generic *fc_generic = data;
	MCDRV_ASSERT(fc_generic != NULL);
	/* We only expect to make smc calls on CPU0 otherwise something wrong
	 * will happen */
	MCDRV_ASSERT(raw_smp_processor_id() == 0);
#ifdef MC_SMC_FASTCALL
	{
		int ret = 0;
		ret = smc_fastcall((void *)fc_generic, sizeof(*fc_generic));
	}
#else
	{
		/* SVC expect values in r0-r3 */
		register u32 reg0 __asm__("r0") = fc_generic->as_in.cmd;
		register u32 reg1 __asm__("r1") = fc_generic->as_in.param[0];
		register u32 reg2 __asm__("r2") = fc_generic->as_in.param[1];
		register u32 reg3 __asm__("r3") = fc_generic->as_in.param[2];

		/* one of the famous preprocessor hacks to stringify things.*/
#define __STR2(x)	#x
#define __STR(x)	__STR2(x)

		/* compiler does not support certain instructions
		 * "SMC": secure monitor call.*/
#define ASM_ARM_SMC	0xE1600070
		/*   "BPKT": debugging breakpoint.
		 * We keep this, as is comes quite handy for debugging. */
#define ASM_ARM_BPKT	0xE1200070
#define ASM_THUMB_BPKT	0xBE00


		__asm__ volatile (
#ifdef CONFIG_ARM_ERRATA_766421
			"dmb\n"
#endif
			".word " __STR(ASM_ARM_SMC) "\n"
#ifdef CONFIG_ARM_ERRATA_766421
			"dmb\n"
#endif
			: "+r"(reg0), "+r"(reg1), "+r"(reg2), "+r"(reg3)
		);

		/* set response */
		fc_generic->as_out.resp     = reg0;
		fc_generic->as_out.ret      = reg1;
		fc_generic->as_out.param[0] = reg2;
		fc_generic->as_out.param[1] = reg3;
	}
#endif
	return 0;
}

/**
 * convert fast call return code to linux driver module error code
 *
 */
static inline int convert_fc_ret(uint32_t sret)
{
	int ret = -EFAULT;

	switch (sret) {
	case MC_FC_RET_OK:
		ret = 0;
		break;

	case MC_FC_RET_ERR_INVALID:
		ret = -EINVAL;
		break;

	case MC_FC_RET_ERR_ALREADY_INITIALIZED:
		ret = -EBUSY;
		break;
	}
	return ret;
}

#endif /* _MC_FASTCALL_H_ */
/** @} */
