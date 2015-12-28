/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2010, 2012-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#include "vr_kernel_common.h"
#include "vr_kernel_descriptor_mapping.h"
#include "vr_osk.h"
#include "vr_osk_bitops.h"

#define VR_PAD_INT(x) (((x) + (BITS_PER_LONG - 1)) & ~(BITS_PER_LONG - 1))

/**
 * Allocate a descriptor table capable of holding 'count' mappings
 * @param count Number of mappings in the table
 * @return Pointer to a new table, NULL on error
 */
static vr_descriptor_table * descriptor_table_alloc(int count);

/**
 * Free a descriptor table
 * @param table The table to free
 */
static void descriptor_table_free(vr_descriptor_table * table);

vr_descriptor_mapping * vr_descriptor_mapping_create(int init_entries, int max_entries)
{
	vr_descriptor_mapping * map = _vr_osk_calloc(1, sizeof(vr_descriptor_mapping));

	init_entries = VR_PAD_INT(init_entries);
	max_entries = VR_PAD_INT(max_entries);

	if (NULL != map) {
		map->table = descriptor_table_alloc(init_entries);
		if (NULL != map->table) {
			map->lock = _vr_osk_mutex_rw_init(_VR_OSK_LOCKFLAG_ORDERED, _VR_OSK_LOCK_ORDER_DESCRIPTOR_MAP);
			if (NULL != map->lock) {
				_vr_osk_set_nonatomic_bit(0, map->table->usage); /* reserve bit 0 to prevent NULL/zero logic to kick in */
				map->max_nr_mappings_allowed = max_entries;
				map->current_nr_mappings = init_entries;
				return map;
			}
			descriptor_table_free(map->table);
		}
		_vr_osk_free(map);
	}
	return NULL;
}

void vr_descriptor_mapping_destroy(vr_descriptor_mapping * map)
{
	descriptor_table_free(map->table);
	_vr_osk_mutex_rw_term(map->lock);
	_vr_osk_free(map);
}

_vr_osk_errcode_t vr_descriptor_mapping_allocate_mapping(vr_descriptor_mapping * map, void * target, int *odescriptor)
{
	_vr_osk_errcode_t err = _VR_OSK_ERR_FAULT;
	int new_descriptor;

	VR_DEBUG_ASSERT_POINTER(map);
	VR_DEBUG_ASSERT_POINTER(odescriptor);

	_vr_osk_mutex_rw_wait(map->lock, _VR_OSK_LOCKMODE_RW);
	new_descriptor = _vr_osk_find_first_zero_bit(map->table->usage, map->current_nr_mappings);
	if (new_descriptor == map->current_nr_mappings) {
		/* no free descriptor, try to expand the table */
		vr_descriptor_table * new_table, * old_table;
		if (map->current_nr_mappings >= map->max_nr_mappings_allowed) goto unlock_and_exit;

		map->current_nr_mappings += BITS_PER_LONG;
		new_table = descriptor_table_alloc(map->current_nr_mappings);
		if (NULL == new_table) goto unlock_and_exit;

		old_table = map->table;
		_vr_osk_memcpy(new_table->usage, old_table->usage, (sizeof(unsigned long)*map->current_nr_mappings) / BITS_PER_LONG);
		_vr_osk_memcpy(new_table->mappings, old_table->mappings, map->current_nr_mappings * sizeof(void*));
		map->table = new_table;
		descriptor_table_free(old_table);
	}

	/* we have found a valid descriptor, set the value and usage bit */
	_vr_osk_set_nonatomic_bit(new_descriptor, map->table->usage);
	map->table->mappings[new_descriptor] = target;
	*odescriptor = new_descriptor;
	err = _VR_OSK_ERR_OK;

unlock_and_exit:
	_vr_osk_mutex_rw_signal(map->lock, _VR_OSK_LOCKMODE_RW);
	VR_ERROR(err);
}

void vr_descriptor_mapping_call_for_each(vr_descriptor_mapping * map, void (*callback)(int, void*))
{
	int i;

	VR_DEBUG_ASSERT_POINTER(map);
	VR_DEBUG_ASSERT_POINTER(callback);

	_vr_osk_mutex_rw_wait(map->lock, _VR_OSK_LOCKMODE_RO);
	/* id 0 is skipped as it's an reserved ID not mapping to anything */
	for (i = 1; i < map->current_nr_mappings; ++i) {
		if (_vr_osk_test_bit(i, map->table->usage)) {
			callback(i, map->table->mappings[i]);
		}
	}
	_vr_osk_mutex_rw_signal(map->lock, _VR_OSK_LOCKMODE_RO);
}

_vr_osk_errcode_t vr_descriptor_mapping_get(vr_descriptor_mapping * map, int descriptor, void** target)
{
	_vr_osk_errcode_t result = _VR_OSK_ERR_FAULT;
	VR_DEBUG_ASSERT_POINTER(map);
	_vr_osk_mutex_rw_wait(map->lock, _VR_OSK_LOCKMODE_RO);
	if ( (descriptor >= 0) && (descriptor < map->current_nr_mappings) && _vr_osk_test_bit(descriptor, map->table->usage) ) {
		*target = map->table->mappings[descriptor];
		result = _VR_OSK_ERR_OK;
	} else *target = NULL;
	_vr_osk_mutex_rw_signal(map->lock, _VR_OSK_LOCKMODE_RO);
	VR_ERROR(result);
}

_vr_osk_errcode_t vr_descriptor_mapping_set(vr_descriptor_mapping * map, int descriptor, void * target)
{
	_vr_osk_errcode_t result = _VR_OSK_ERR_FAULT;
	_vr_osk_mutex_rw_wait(map->lock, _VR_OSK_LOCKMODE_RO);
	if ( (descriptor >= 0) && (descriptor < map->current_nr_mappings) && _vr_osk_test_bit(descriptor, map->table->usage) ) {
		map->table->mappings[descriptor] = target;
		result = _VR_OSK_ERR_OK;
	}
	_vr_osk_mutex_rw_signal(map->lock, _VR_OSK_LOCKMODE_RO);
	VR_ERROR(result);
}

void *vr_descriptor_mapping_free(vr_descriptor_mapping * map, int descriptor)
{
	void *old_value = NULL;

	_vr_osk_mutex_rw_wait(map->lock, _VR_OSK_LOCKMODE_RW);
	if ( (descriptor >= 0) && (descriptor < map->current_nr_mappings) && _vr_osk_test_bit(descriptor, map->table->usage) ) {
		old_value = map->table->mappings[descriptor];
		map->table->mappings[descriptor] = NULL;
		_vr_osk_clear_nonatomic_bit(descriptor, map->table->usage);
	}
	_vr_osk_mutex_rw_signal(map->lock, _VR_OSK_LOCKMODE_RW);

	return old_value;
}

static vr_descriptor_table * descriptor_table_alloc(int count)
{
	vr_descriptor_table * table;

	table = _vr_osk_calloc(1, sizeof(vr_descriptor_table) + ((sizeof(unsigned long) * count)/BITS_PER_LONG) + (sizeof(void*) * count));

	if (NULL != table) {
		table->usage = (u32*)((u8*)table + sizeof(vr_descriptor_table));
		table->mappings = (void**)((u8*)table + sizeof(vr_descriptor_table) + ((sizeof(unsigned long) * count)/BITS_PER_LONG));
	}

	return table;
}

static void descriptor_table_free(vr_descriptor_table * table)
{
	_vr_osk_free(table);
}
