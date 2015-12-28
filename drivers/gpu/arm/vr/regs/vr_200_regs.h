/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2007-2010, 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef _VR200_REGS_H_
#define _VR200_REGS_H_

/**
 *  Enum for management register addresses.
 */
enum vr200_mgmt_reg {
	VR200_REG_ADDR_MGMT_VERSION                              = 0x1000,
	VR200_REG_ADDR_MGMT_CURRENT_REND_LIST_ADDR               = 0x1004,
	VR200_REG_ADDR_MGMT_STATUS                               = 0x1008,
	VR200_REG_ADDR_MGMT_CTRL_MGMT                            = 0x100c,

	VR200_REG_ADDR_MGMT_INT_RAWSTAT                          = 0x1020,
	VR200_REG_ADDR_MGMT_INT_CLEAR                            = 0x1024,
	VR200_REG_ADDR_MGMT_INT_MASK                             = 0x1028,
	VR200_REG_ADDR_MGMT_INT_STATUS                           = 0x102c,

	VR200_REG_ADDR_MGMT_WRITE_BOUNDARY_LOW                   = 0x1044,

	VR200_REG_ADDR_MGMT_BUS_ERROR_STATUS                     = 0x1050,

	VR200_REG_ADDR_MGMT_PERF_CNT_0_ENABLE                    = 0x1080,
	VR200_REG_ADDR_MGMT_PERF_CNT_0_SRC                       = 0x1084,
	VR200_REG_ADDR_MGMT_PERF_CNT_0_VALUE                     = 0x108c,

	VR200_REG_ADDR_MGMT_PERF_CNT_1_ENABLE                    = 0x10a0,
	VR200_REG_ADDR_MGMT_PERF_CNT_1_SRC                       = 0x10a4,
	VR200_REG_ADDR_MGMT_PERF_CNT_1_VALUE                     = 0x10ac,

	VR200_REG_ADDR_MGMT_PERFMON_CONTR                        = 0x10b0,
	VR200_REG_ADDR_MGMT_PERFMON_BASE                         = 0x10b4,

	VR200_REG_SIZEOF_REGISTER_BANK                           = 0x10f0

};

#define VR200_REG_VAL_PERF_CNT_ENABLE 1

enum vr200_mgmt_ctrl_mgmt {
	VR200_REG_VAL_CTRL_MGMT_STOP_BUS         = (1<<0),
	VR200_REG_VAL_CTRL_MGMT_FLUSH_CACHES     = (1<<3),
	VR200_REG_VAL_CTRL_MGMT_FORCE_RESET      = (1<<5),
	VR200_REG_VAL_CTRL_MGMT_START_RENDERING  = (1<<6),
	VR400PP_REG_VAL_CTRL_MGMT_SOFT_RESET     = (1<<7), /* Only valid for Vr-300 and later */
};

enum vr200_mgmt_irq {
	VR200_REG_VAL_IRQ_END_OF_FRAME          = (1<<0),
	VR200_REG_VAL_IRQ_END_OF_TILE           = (1<<1),
	VR200_REG_VAL_IRQ_HANG                  = (1<<2),
	VR200_REG_VAL_IRQ_FORCE_HANG            = (1<<3),
	VR200_REG_VAL_IRQ_BUS_ERROR             = (1<<4),
	VR200_REG_VAL_IRQ_BUS_STOP              = (1<<5),
	VR200_REG_VAL_IRQ_CNT_0_LIMIT           = (1<<6),
	VR200_REG_VAL_IRQ_CNT_1_LIMIT           = (1<<7),
	VR200_REG_VAL_IRQ_WRITE_BOUNDARY_ERROR  = (1<<8),
	VR400PP_REG_VAL_IRQ_INVALID_PLIST_COMMAND = (1<<9),
	VR400PP_REG_VAL_IRQ_CALL_STACK_UNDERFLOW  = (1<<10),
	VR400PP_REG_VAL_IRQ_CALL_STACK_OVERFLOW   = (1<<11),
	VR400PP_REG_VAL_IRQ_RESET_COMPLETED       = (1<<12),
};

#define VR200_REG_VAL_IRQ_MASK_ALL  ((enum vr200_mgmt_irq) (\
    VR200_REG_VAL_IRQ_END_OF_FRAME                           |\
    VR200_REG_VAL_IRQ_END_OF_TILE                            |\
    VR200_REG_VAL_IRQ_HANG                                   |\
    VR200_REG_VAL_IRQ_FORCE_HANG                             |\
    VR200_REG_VAL_IRQ_BUS_ERROR                              |\
    VR200_REG_VAL_IRQ_BUS_STOP                               |\
    VR200_REG_VAL_IRQ_CNT_0_LIMIT                            |\
    VR200_REG_VAL_IRQ_CNT_1_LIMIT                            |\
    VR200_REG_VAL_IRQ_WRITE_BOUNDARY_ERROR                   |\
    VR400PP_REG_VAL_IRQ_INVALID_PLIST_COMMAND                  |\
    VR400PP_REG_VAL_IRQ_CALL_STACK_UNDERFLOW                   |\
    VR400PP_REG_VAL_IRQ_CALL_STACK_OVERFLOW                    |\
    VR400PP_REG_VAL_IRQ_RESET_COMPLETED))

#define VR200_REG_VAL_IRQ_MASK_USED ((enum vr200_mgmt_irq) (\
    VR200_REG_VAL_IRQ_END_OF_FRAME                           |\
    VR200_REG_VAL_IRQ_FORCE_HANG                             |\
    VR200_REG_VAL_IRQ_BUS_ERROR                              |\
    VR200_REG_VAL_IRQ_WRITE_BOUNDARY_ERROR                   |\
    VR400PP_REG_VAL_IRQ_INVALID_PLIST_COMMAND                  |\
    VR400PP_REG_VAL_IRQ_CALL_STACK_UNDERFLOW                   |\
    VR400PP_REG_VAL_IRQ_CALL_STACK_OVERFLOW))

#define VR200_REG_VAL_IRQ_MASK_NONE ((enum vr200_mgmt_irq)(0))

enum vr200_mgmt_status {
	VR200_REG_VAL_STATUS_RENDERING_ACTIVE     = (1<<0),
	VR200_REG_VAL_STATUS_BUS_STOPPED          = (1<<4),
};

enum vr200_render_unit {
	VR200_REG_ADDR_FRAME = 0x0000,
	VR200_REG_ADDR_RSW   = 0x0004,
	VR200_REG_ADDR_STACK = 0x0030,
	VR200_REG_ADDR_STACK_SIZE = 0x0034,
	VR200_REG_ADDR_ORIGIN_OFFSET_X  = 0x0040
};

enum vr200_wb_unit {
	VR200_REG_ADDR_WB0 = 0x0100,
	VR200_REG_ADDR_WB1 = 0x0200,
	VR200_REG_ADDR_WB2 = 0x0300
};

enum vr200_wb_unit_regs {
	VR200_REG_ADDR_WB_SOURCE_SELECT = 0x0000,
	VR200_REG_ADDR_WB_SOURCE_ADDR   = 0x0004,
};

/* This should be in the top 16 bit of the version register of Vr PP */
#define VR200_PP_PRODUCT_ID 0xC807
#define VR300_PP_PRODUCT_ID 0xCE07
#define VR400_PP_PRODUCT_ID 0xCD07
#define VR450_PP_PRODUCT_ID 0xCF07


#endif /* _VR200_REGS_H_ */
