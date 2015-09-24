/** MobiCore driver module.(interface to the secure world SWD)
 * @addtogroup MCD_MCDIMPL_KMOD_IMPL
 * @{
 * @file
 * MobiCore Driver Kernel Module.
 * This module is written as a Linux device driver.
 * This driver represents the command proxy on the lowest layer, from the
 * secure world to the non secure world, and vice versa.
 * This driver is located in the non secure world (Linux).
 * This driver offers IOCTL commands, for access to the secure world, and has
 * the interface from the secure world to the normal world.
 * The access to the driver is possible with a file descriptor,
 * which has to be created by the fd = open(/dev/mobicore) command.
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/suspend.h>


#include "pm.h"
#include "main.h"
#include "fastcall.h"
#include "ops.h"
#include "logging.h"
#include "debug.h"

#ifdef MC_PM_RUNTIME

static struct mc_context *ctx;

static struct timer_list resume_timer;

static void mc_resume_handler(unsigned long data)
{
	if (!ctx->mcp)
		return;

	ctx->mcp->flags.sleep_mode.SleepReq = 0;
}

static void mc_suspend_handler(struct work_struct *work)
{
	if (!ctx->mcp)
		return;
	ctx->mcp->flags.sleep_mode.SleepReq = REQ_TO_SLEEP;
	mc_nsiq();
}
DECLARE_WORK(suspend_work, mc_suspend_handler);

static inline void dump_sleep_params(struct mc_flags *flags)
{
	MCDRV_DBG("MobiCore IDLE=%d!", flags->schedule);
	MCDRV_DBG("MobiCore Request Sleep=%d!", flags->sleep_mode.SleepReq);
	MCDRV_DBG("MobiCore Sleep Ready=%d!", flags->sleep_mode.ReadyToSleep);
}

static int mc_suspend_notifier(struct notifier_block *nb,
	unsigned long event, void *dummy)
{
	struct mc_mcp_buffer *mcp = ctx->mcp;
	/* We have noting to say if MobiCore is not initialized */
	if (!mcp)
		return 0;

#ifdef MC_MEM_TRACES
	mobicore_log_read();
#endif

	switch (event) {
	case PM_SUSPEND_PREPARE:
		/* Make sure we have finished all the work otherwise
		 * we end up in a race condition */
		mod_timer(&resume_timer, 0);
		cancel_work_sync(&suspend_work);
		/* We can't go to sleep if MobiCore is not IDLE
		 * or not Ready to sleep */
		dump_sleep_params(&mcp->flags);
		if (!(mcp->flags.sleep_mode.ReadyToSleep & READY_TO_SLEEP)) {
			schedule_work_on(0, &suspend_work);
			dump_sleep_params(&mcp->flags);
			MCDRV_DBG_ERROR("MobiCore can't SLEEP yet!");
			return NOTIFY_BAD;
		}
		break;
	case PM_POST_SUSPEND:
		MCDRV_DBG("Resume MobiCore system!");
		mod_timer(&resume_timer, jiffies +
			msecs_to_jiffies(DAEMON_BACKOFF_TIME));
		break;
	default:
		break;
	}
	return 0;
}

static struct notifier_block mc_notif_block = {
	.notifier_call = mc_suspend_notifier,
};

int mc_pm_initialize(struct mc_context *context)
{
	int ret = 0;

	ctx = context;
	setup_timer(&resume_timer, mc_resume_handler, 0);
	ret = register_pm_notifier(&mc_notif_block);
	if (ret)
		MCDRV_DBG_ERROR("device pm register failed\n");
	return ret;
}

int mc_pm_free(void)
{
	int ret = unregister_pm_notifier(&mc_notif_block);
	if (ret)
		MCDRV_DBG_ERROR("device pm unregister failed\n");
	del_timer(&resume_timer);
	return ret;
}

#endif /* MC_PM_RUNTIME */
