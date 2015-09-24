/**
 * Header file of MobiCore Driver Kernel Module.
 *
 * @addtogroup MCD_MCDIMPL_KMOD_IMPL
 * @{
 * Internal structures of the McDrvModule
 * @file
 *
 * Header file the MobiCore Driver Kernel Module,
 * its internal structures and defines.
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _MC_PM_H_
#define _MC_PM_H_

#include "main.h"

#define NO_SLEEP_REQ	0
#define REQ_TO_SLEEP	1

#define NORMAL_EXECUTION	0
#define READY_TO_SLEEP		1

/* How much time after resume the daemon should backoff */
#define DAEMON_BACKOFF_TIME	1000

/* Initialize Power Management */
int mc_pm_initialize(struct mc_context *context);
/* Free all Power Management resources*/
int mc_pm_free(void);

#endif /* _MC_PM_H_ */
/** @} */
