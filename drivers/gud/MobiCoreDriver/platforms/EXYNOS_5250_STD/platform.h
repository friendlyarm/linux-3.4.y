/**
 * Header file of MobiCore Driver Kernel Module Platform
 * specific structures
 *
 * @addtogroup MobiCore_Driver_Kernel_Module
 * @{
 * Internal structures of the McDrvModule
 * @file
 *
 * Header file the MobiCore Driver Kernel Module,
 * its internal structures and defines.
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 */

#ifndef _MC_DRV_PLATFORM_H_
#define _MC_DRV_PLATFORM_H_

#include <mach/irqs.h>

/** MobiCore Interrupt. */
#define MC_INTR_SSIQ IRQ_SPI(122)

/** Enable mobicore mem traces */
#define MC_MEM_TRACES

/** Enable Runtime Power Management */
#ifdef CONFIG_PM_RUNTIME
 #define MC_PM_RUNTIME
#endif

#endif /* _MC_DRV_PLATFORM_H_ */
/** @} */
