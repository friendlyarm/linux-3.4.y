/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */
#include "vr_kernel_common.h"
#include "vr_osk.h"
#include "vr_l2_cache.h"
#include "vr_hw_core.h"
#include "vr_scheduler.h"
#include "vr_pm_domain.h"

/**
 * Size of the Vr L2 cache registers in bytes
 */
#define VR400_L2_CACHE_REGISTERS_SIZE 0x30

/**
 * Vr L2 cache register numbers
 * Used in the register read/write routines.
 * See the hardware documentation for more information about each register
 */
typedef enum vr_l2_cache_register {
	VR400_L2_CACHE_REGISTER_SIZE         = 0x0004,
	VR400_L2_CACHE_REGISTER_STATUS       = 0x0008,
	/*unused                               = 0x000C */
	VR400_L2_CACHE_REGISTER_COMMAND      = 0x0010, /**< Misc cache commands, e.g. clear */
	VR400_L2_CACHE_REGISTER_CLEAR_PAGE   = 0x0014,
	VR400_L2_CACHE_REGISTER_MAX_READS    = 0x0018, /**< Limit of outstanding read requests */
	VR400_L2_CACHE_REGISTER_ENABLE       = 0x001C, /**< Enable misc cache features */
	VR400_L2_CACHE_REGISTER_PERFCNT_SRC0 = 0x0020,
	VR400_L2_CACHE_REGISTER_PERFCNT_VAL0 = 0x0024,
	VR400_L2_CACHE_REGISTER_PERFCNT_SRC1 = 0x0028,
	VR400_L2_CACHE_REGISTER_PERFCNT_VAL1 = 0x002C,
} vr_l2_cache_register;

/**
 * Vr L2 cache commands
 * These are the commands that can be sent to the Vr L2 cache unit
 */
typedef enum vr_l2_cache_command {
	VR400_L2_CACHE_COMMAND_CLEAR_ALL = 0x01, /**< Clear the entire cache */
	/* Read HW TRM carefully before adding/using other commands than the clear above */
} vr_l2_cache_command;

/**
 * Vr L2 cache commands
 * These are the commands that can be sent to the Vr L2 cache unit
 */
typedef enum vr_l2_cache_enable {
	VR400_L2_CACHE_ENABLE_DEFAULT = 0x0, /**< Default state of enable register */
	VR400_L2_CACHE_ENABLE_ACCESS = 0x01, /**< Permit cacheable accesses */
	VR400_L2_CACHE_ENABLE_READ_ALLOCATE = 0x02, /**< Permit cache read allocate */
} vr_l2_cache_enable;

/**
 * Vr L2 cache status bits
 */
typedef enum vr_l2_cache_status {
	VR400_L2_CACHE_STATUS_COMMAND_BUSY = 0x01, /**< Command handler of L2 cache is busy */
	VR400_L2_CACHE_STATUS_DATA_BUSY    = 0x02, /**< L2 cache is busy handling data requests */
} vr_l2_cache_status;

#define VR400_L2_MAX_READS_DEFAULT 0x1C

static struct vr_l2_cache_core *vr_global_l2_cache_cores[VR_MAX_NUMBER_OF_L2_CACHE_CORES] = { NULL, };
static u32 vr_global_num_l2_cache_cores = 0;

int vr_l2_max_reads = VR400_L2_MAX_READS_DEFAULT;


/* Local helper functions */
static _vr_osk_errcode_t vr_l2_cache_send_command(struct vr_l2_cache_core *cache, u32 reg, u32 val);


static void vr_l2_cache_counter_lock(struct vr_l2_cache_core *cache)
{
#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_lock(cache->counter_lock);
#else
	_vr_osk_spinlock_lock(cache->counter_lock);
#endif
}

static void vr_l2_cache_counter_unlock(struct vr_l2_cache_core *cache)
{
#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_unlock(cache->counter_lock);
#else
	_vr_osk_spinlock_unlock(cache->counter_lock);
#endif
}

static void vr_l2_cache_command_lock(struct vr_l2_cache_core *cache)
{
#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_lock(cache->command_lock);
#else
	_vr_osk_spinlock_lock(cache->command_lock);
#endif
}

static void vr_l2_cache_command_unlock(struct vr_l2_cache_core *cache)
{
#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_unlock(cache->command_lock);
#else
	_vr_osk_spinlock_unlock(cache->command_lock);
#endif
}

struct vr_l2_cache_core *vr_l2_cache_create(_vr_osk_resource_t *resource)
{
	struct vr_l2_cache_core *cache = NULL;

	VR_DEBUG_PRINT(4, ("Vr L2 cache: Creating Vr L2 cache: %s\n", resource->description));

	if (vr_global_num_l2_cache_cores >= VR_MAX_NUMBER_OF_L2_CACHE_CORES) {
		VR_PRINT_ERROR(("Vr L2 cache: Too many L2 cache core objects created\n"));
		return NULL;
	}

	cache = _vr_osk_malloc(sizeof(struct vr_l2_cache_core));
	if (NULL != cache) {
		cache->core_id =  vr_global_num_l2_cache_cores;
		cache->counter_src0 = VR_HW_CORE_NO_COUNTER;
		cache->counter_src1 = VR_HW_CORE_NO_COUNTER;
		cache->pm_domain = NULL;
		cache->vr_l2_status = VR_L2_NORMAL;
		if (_VR_OSK_ERR_OK == vr_hw_core_create(&cache->hw_core, resource, VR400_L2_CACHE_REGISTERS_SIZE)) {
			VR_DEBUG_CODE(u32 cache_size = vr_hw_core_register_read(&cache->hw_core, VR400_L2_CACHE_REGISTER_SIZE));
			VR_DEBUG_PRINT(2, ("Vr L2 cache: Created %s: % 3uK, %u-way, % 2ubyte cache line, % 3ubit external bus\n",
			                     resource->description,
			                     1 << (((cache_size >> 16) & 0xff) - 10),
			                     1 << ((cache_size >> 8) & 0xff),
			                     1 << (cache_size & 0xff),
			                     1 << ((cache_size >> 24) & 0xff)));

#ifdef VR_UPPER_HALF_SCHEDULING
			cache->command_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_L2_COMMAND);
#else
			cache->command_lock = _vr_osk_spinlock_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_L2_COMMAND);
#endif
			if (NULL != cache->command_lock) {
#ifdef VR_UPPER_HALF_SCHEDULING
				cache->counter_lock = _vr_osk_spinlock_irq_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_L2_COMMAND);
#else
				cache->counter_lock = _vr_osk_spinlock_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_L2_COMMAND);
#endif
				if (NULL != cache->counter_lock) {
					vr_l2_cache_reset(cache);

					cache->last_invalidated_id = 0;

					vr_global_l2_cache_cores[vr_global_num_l2_cache_cores] = cache;
					vr_global_num_l2_cache_cores++;

					return cache;
				} else {
					VR_PRINT_ERROR(("Vr L2 cache: Failed to create counter lock for L2 cache core %s\n", cache->hw_core.description));
				}
#ifdef VR_UPPER_HALF_SCHEDULING
				_vr_osk_spinlock_irq_term(cache->command_lock);
#else
				_vr_osk_spinlock_term(cache->command_lock);
#endif
			} else {
				VR_PRINT_ERROR(("Vr L2 cache: Failed to create command lock for L2 cache core %s\n", cache->hw_core.description));
			}

			vr_hw_core_delete(&cache->hw_core);
		}

		_vr_osk_free(cache);
	} else {
		VR_PRINT_ERROR(("Vr L2 cache: Failed to allocate memory for L2 cache core\n"));
	}

	return NULL;
}

void vr_l2_cache_delete(struct vr_l2_cache_core *cache)
{
	u32 i;

	/* reset to defaults */
	vr_hw_core_register_write(&cache->hw_core, VR400_L2_CACHE_REGISTER_MAX_READS, (u32)VR400_L2_MAX_READS_DEFAULT);
	vr_hw_core_register_write(&cache->hw_core, VR400_L2_CACHE_REGISTER_ENABLE, (u32)VR400_L2_CACHE_ENABLE_DEFAULT);

#ifdef VR_UPPER_HALF_SCHEDULING
	_vr_osk_spinlock_irq_term(cache->counter_lock);
	_vr_osk_spinlock_irq_term(cache->command_lock);
#else
	_vr_osk_spinlock_term(cache->command_lock);
	_vr_osk_spinlock_term(cache->counter_lock);
#endif

	vr_hw_core_delete(&cache->hw_core);

	for (i = 0; i < vr_global_num_l2_cache_cores; i++) {
		if (vr_global_l2_cache_cores[i] == cache) {
			vr_global_l2_cache_cores[i] = NULL;
			vr_global_num_l2_cache_cores--;

			if (i != vr_global_num_l2_cache_cores) {
				/* We removed a l2 cache from the middle of the array -- move the last
				 * l2 cache to the current position to close the gap */
				vr_global_l2_cache_cores[i] = vr_global_l2_cache_cores[vr_global_num_l2_cache_cores];
				vr_global_l2_cache_cores[vr_global_num_l2_cache_cores] = NULL;
			}

			break;
		}
	}

	_vr_osk_free(cache);
}

u32 vr_l2_cache_get_id(struct vr_l2_cache_core *cache)
{
	return cache->core_id;
}

static void vr_l2_cache_core_set_counter_internal(struct vr_l2_cache_core *cache, u32 source_id, u32 counter)
{
	u32 value = 0; /* disabled src */
	u32 reg_offset = 0;
	vr_bool core_is_on;

	VR_DEBUG_ASSERT_POINTER(cache);

	core_is_on = vr_l2_cache_lock_power_state(cache);

	vr_l2_cache_counter_lock(cache);

	switch (source_id) {
	case 0:
		cache->counter_src0 = counter;
		reg_offset = VR400_L2_CACHE_REGISTER_PERFCNT_SRC0;
		break;

	case 1:
		cache->counter_src1 = counter;
		reg_offset = VR400_L2_CACHE_REGISTER_PERFCNT_SRC1;
		break;

	default:
		VR_DEBUG_ASSERT(0);
		break;
	}

	if (VR_L2_PAUSE == cache->vr_l2_status) {
		vr_l2_cache_counter_unlock(cache);
		vr_l2_cache_unlock_power_state(cache);
		return;
	}

	if (VR_HW_CORE_NO_COUNTER != counter) {
		value = counter;
	}

	if (VR_TRUE == core_is_on) {
		vr_hw_core_register_write(&cache->hw_core, reg_offset, value);
	}

	vr_l2_cache_counter_unlock(cache);
	vr_l2_cache_unlock_power_state(cache);
}

void vr_l2_cache_core_set_counter_src0(struct vr_l2_cache_core *cache, u32 counter)
{
	vr_l2_cache_core_set_counter_internal(cache, 0, counter);
}

void vr_l2_cache_core_set_counter_src1(struct vr_l2_cache_core *cache, u32 counter)
{
	vr_l2_cache_core_set_counter_internal(cache, 1, counter);
}

u32 vr_l2_cache_core_get_counter_src0(struct vr_l2_cache_core *cache)
{
	return cache->counter_src0;
}

u32 vr_l2_cache_core_get_counter_src1(struct vr_l2_cache_core *cache)
{
	return cache->counter_src1;
}

void vr_l2_cache_core_get_counter_values(struct vr_l2_cache_core *cache, u32 *src0, u32 *value0, u32 *src1, u32 *value1)
{
	VR_DEBUG_ASSERT(NULL != src0);
	VR_DEBUG_ASSERT(NULL != value0);
	VR_DEBUG_ASSERT(NULL != src1);
	VR_DEBUG_ASSERT(NULL != value1);

	/* Caller must hold the PM lock and know that we are powered on */

	vr_l2_cache_counter_lock(cache);

	if (VR_L2_PAUSE == cache->vr_l2_status) {
		vr_l2_cache_counter_unlock(cache);

		return;
	}

	*src0 = cache->counter_src0;
	*src1 = cache->counter_src1;

	if (cache->counter_src0 != VR_HW_CORE_NO_COUNTER) {
		*value0 = vr_hw_core_register_read(&cache->hw_core, VR400_L2_CACHE_REGISTER_PERFCNT_VAL0);
	}

	if (cache->counter_src1 != VR_HW_CORE_NO_COUNTER) {
		*value1 = vr_hw_core_register_read(&cache->hw_core, VR400_L2_CACHE_REGISTER_PERFCNT_VAL1);
	}

	vr_l2_cache_counter_unlock(cache);
}

static void vr_l2_cache_reset_counters_all(void)
{
	int i;
	u32 value;
	struct vr_l2_cache_core *cache;
	u32 num_cores = vr_l2_cache_core_get_glob_num_l2_cores();

	for (i = 0; i < num_cores; i++) {
		cache = vr_l2_cache_core_get_glob_l2_core(i);
		if (VR_TRUE == vr_l2_cache_lock_power_state(cache)) {
			vr_l2_cache_counter_lock(cache);

			if (VR_L2_PAUSE == cache->vr_l2_status) {
				vr_l2_cache_counter_unlock(cache);
				vr_l2_cache_unlock_power_state(cache);
				return;
			}

			/* Reset performance counters */
			if (VR_HW_CORE_NO_COUNTER == cache->counter_src0) {
				value = 0;
			} else {
				value = cache->counter_src0;
			}
			vr_hw_core_register_write(&cache->hw_core,
			                            VR400_L2_CACHE_REGISTER_PERFCNT_SRC0, value);

			if (VR_HW_CORE_NO_COUNTER == cache->counter_src1) {
				value = 0;
			} else {
				value = cache->counter_src1;
			}
			vr_hw_core_register_write(&cache->hw_core,
			                            VR400_L2_CACHE_REGISTER_PERFCNT_SRC1, value);

			vr_l2_cache_counter_unlock(cache);
		}

		vr_l2_cache_unlock_power_state(cache);
	}
}


struct vr_l2_cache_core *vr_l2_cache_core_get_glob_l2_core(u32 index)
{
	if (vr_global_num_l2_cache_cores > index) {
		return vr_global_l2_cache_cores[index];
	}

	return NULL;
}

u32 vr_l2_cache_core_get_glob_num_l2_cores(void)
{
	return vr_global_num_l2_cache_cores;
}

void vr_l2_cache_reset(struct vr_l2_cache_core *cache)
{
	/* Invalidate cache (just to keep it in a known state at startup) */
	vr_l2_cache_send_command(cache, VR400_L2_CACHE_REGISTER_COMMAND, VR400_L2_CACHE_COMMAND_CLEAR_ALL);

	vr_l2_cache_counter_lock(cache);

	if (VR_L2_PAUSE == cache->vr_l2_status) {
		vr_l2_cache_counter_unlock(cache);

		return;
	}

	/* Enable cache */
	vr_hw_core_register_write(&cache->hw_core, VR400_L2_CACHE_REGISTER_ENABLE, (u32)VR400_L2_CACHE_ENABLE_ACCESS | (u32)VR400_L2_CACHE_ENABLE_READ_ALLOCATE);
	vr_hw_core_register_write(&cache->hw_core, VR400_L2_CACHE_REGISTER_MAX_READS, (u32)vr_l2_max_reads);

	/* Restart any performance counters (if enabled) */
	if (cache->counter_src0 != VR_HW_CORE_NO_COUNTER) {
		vr_hw_core_register_write(&cache->hw_core, VR400_L2_CACHE_REGISTER_PERFCNT_SRC0, cache->counter_src0);
	}

	if (cache->counter_src1 != VR_HW_CORE_NO_COUNTER) {
		vr_hw_core_register_write(&cache->hw_core, VR400_L2_CACHE_REGISTER_PERFCNT_SRC1, cache->counter_src1);
	}

	vr_l2_cache_counter_unlock(cache);
}

void vr_l2_cache_reset_all(void)
{
	int i;
	u32 num_cores = vr_l2_cache_core_get_glob_num_l2_cores();

	for (i = 0; i < num_cores; i++) {
		vr_l2_cache_reset(vr_l2_cache_core_get_glob_l2_core(i));
	}
}

void vr_l2_cache_invalidate(struct vr_l2_cache_core *cache)
{
	VR_DEBUG_ASSERT_POINTER(cache);

	if (NULL != cache) {
		cache->last_invalidated_id = vr_scheduler_get_new_cache_order();
		vr_l2_cache_send_command(cache, VR400_L2_CACHE_REGISTER_COMMAND, VR400_L2_CACHE_COMMAND_CLEAR_ALL);
	}
}

vr_bool vr_l2_cache_invalidate_conditional(struct vr_l2_cache_core *cache, u32 id)
{
	VR_DEBUG_ASSERT_POINTER(cache);

	if (NULL != cache) {
		/* If the last cache invalidation was done by a job with a higher id we
		 * don't have to flush. Since user space will store jobs w/ their
		 * corresponding memory in sequence (first job #0, then job #1, ...),
		 * we don't have to flush for job n-1 if job n has already invalidated
		 * the cache since we know for sure that job n-1's memory was already
		 * written when job n was started. */
		if (((s32)id) <= ((s32)cache->last_invalidated_id)) {
			return VR_FALSE;
		} else {
			cache->last_invalidated_id = vr_scheduler_get_new_cache_order();
		}

		vr_l2_cache_send_command(cache, VR400_L2_CACHE_REGISTER_COMMAND, VR400_L2_CACHE_COMMAND_CLEAR_ALL);
	}
	return VR_TRUE;
}

void vr_l2_cache_invalidate_all(void)
{
	u32 i;
	for (i = 0; i < vr_global_num_l2_cache_cores; i++) {
		/*additional check*/
		if (VR_TRUE == vr_l2_cache_lock_power_state(vr_global_l2_cache_cores[i])) {
			_vr_osk_errcode_t ret;
			vr_global_l2_cache_cores[i]->last_invalidated_id = vr_scheduler_get_new_cache_order();
			ret = vr_l2_cache_send_command(vr_global_l2_cache_cores[i], VR400_L2_CACHE_REGISTER_COMMAND, VR400_L2_CACHE_COMMAND_CLEAR_ALL);
			if (_VR_OSK_ERR_OK != ret) {
				VR_PRINT_ERROR(("Failed to invalidate cache\n"));
			}
		}
		vr_l2_cache_unlock_power_state(vr_global_l2_cache_cores[i]);
	}
}

void vr_l2_cache_invalidate_all_pages(u32 *pages, u32 num_pages)
{
	u32 i;
	for (i = 0; i < vr_global_num_l2_cache_cores; i++) {
		/*additional check*/
		if (VR_TRUE == vr_l2_cache_lock_power_state(vr_global_l2_cache_cores[i])) {
			u32 j;
			for (j = 0; j < num_pages; j++) {
				_vr_osk_errcode_t ret;
				ret = vr_l2_cache_send_command(vr_global_l2_cache_cores[i], VR400_L2_CACHE_REGISTER_CLEAR_PAGE, pages[j]);
				if (_VR_OSK_ERR_OK != ret) {
					VR_PRINT_ERROR(("Failed to invalidate page cache\n"));
				}
			}
		}
		vr_l2_cache_unlock_power_state(vr_global_l2_cache_cores[i]);
	}
}

vr_bool vr_l2_cache_lock_power_state(struct vr_l2_cache_core *cache)
{
	return vr_pm_domain_lock_state(cache->pm_domain);
}

void vr_l2_cache_unlock_power_state(struct vr_l2_cache_core *cache)
{
	return vr_pm_domain_unlock_state(cache->pm_domain);
}

/* -------- local helper functions below -------- */


static _vr_osk_errcode_t vr_l2_cache_send_command(struct vr_l2_cache_core *cache, u32 reg, u32 val)
{
	int i = 0;
	const int loop_count = 100000;

	/*
	 * Grab lock in order to send commands to the L2 cache in a serialized fashion.
	 * The L2 cache will ignore commands if it is busy.
	 */
	vr_l2_cache_command_lock(cache);

	if (VR_L2_PAUSE == cache->vr_l2_status) {
		vr_l2_cache_command_unlock(cache);
		VR_DEBUG_PRINT(1, ( "Vr L2 cache: aborting wait for L2 come back\n"));

		VR_ERROR( _VR_OSK_ERR_BUSY );
	}

	/* First, wait for L2 cache command handler to go idle */

	for (i = 0; i < loop_count; i++) {
		if (!(vr_hw_core_register_read(&cache->hw_core, VR400_L2_CACHE_REGISTER_STATUS) & (u32)VR400_L2_CACHE_STATUS_COMMAND_BUSY)) {
			break;
		}
	}

	if (i == loop_count) {
		vr_l2_cache_command_unlock(cache);
		VR_DEBUG_PRINT(1, ( "Vr L2 cache: aborting wait for command interface to go idle\n"));
		VR_ERROR( _VR_OSK_ERR_FAULT );
	}

	/* then issue the command */
	vr_hw_core_register_write(&cache->hw_core, reg, val);

	vr_l2_cache_command_unlock(cache);

	VR_SUCCESS;
}

void vr_l2_cache_pause_all(vr_bool pause)
{
	int i;
	struct vr_l2_cache_core * cache;
	u32 num_cores = vr_l2_cache_core_get_glob_num_l2_cores();
	vr_l2_power_status status = VR_L2_NORMAL;

	if (pause) {
		status = VR_L2_PAUSE;
	}

	for (i = 0; i < num_cores; i++) {
		cache = vr_l2_cache_core_get_glob_l2_core(i);
		if (NULL != cache) {
			cache->vr_l2_status = status;

			/* Take and release the counter and command locks to
			 * ensure there are no active threads that didn't get
			 * the status flag update.
			 *
			 * The locks will also ensure the necessary memory
			 * barriers are done on SMP systems.
			 */
			vr_l2_cache_counter_lock(cache);
			vr_l2_cache_counter_unlock(cache);

			vr_l2_cache_command_lock(cache);
			vr_l2_cache_command_unlock(cache);
		}
	}

	/* Resume from pause: do the cache invalidation here to prevent any
	 * loss of cache operation during the pause period to make sure the SW
	 * status is consistent with L2 cache status.
	 */
	if(!pause) {
		vr_l2_cache_invalidate_all();
		vr_l2_cache_reset_counters_all();
	}
}
