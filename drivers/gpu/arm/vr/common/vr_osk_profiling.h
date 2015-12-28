/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2010-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_OSK_PROFILING_H__
#define __VR_OSK_PROFILING_H__

#if defined(CONFIG_VR400_PROFILING) && defined (CONFIG_TRACEPOINTS)

#include "vr_linux_trace.h"
#include "vr_profiling_events.h"
#include "vr_profiling_gator_api.h"

#define VR_PROFILING_MAX_BUFFER_ENTRIES 1048576

#define VR_PROFILING_NO_HW_COUNTER = ((u32)-1)

/** @defgroup _vr_osk_profiling External profiling connectivity
 * @{ */

/**
 * Initialize the profiling module.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t _vr_osk_profiling_init(vr_bool auto_start);

/*
 * Terminate the profiling module.
 */
void _vr_osk_profiling_term(void);

/**
 * Start recording profiling data
 *
 * The specified limit will determine how large the capture buffer is.
 * VR_PROFILING_MAX_BUFFER_ENTRIES determines the maximum size allowed by the device driver.
 *
 * @param limit The desired maximum number of events to record on input, the actual maximum on output.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t _vr_osk_profiling_start(u32 * limit);

/**
 * Add an profiling event
 *
 * @param event_id The event identificator.
 * @param data0 First data parameter, depending on event_id specified.
 * @param data1 Second data parameter, depending on event_id specified.
 * @param data2 Third data parameter, depending on event_id specified.
 * @param data3 Fourth data parameter, depending on event_id specified.
 * @param data4 Fifth data parameter, depending on event_id specified.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
/* Call Linux tracepoint directly */
#define _vr_osk_profiling_add_event(event_id, data0, data1, data2, data3, data4) trace_vr_timeline_event((event_id), (data0), (data1), (data2), (data3), (data4))

/**
 * Report a hardware counter event.
 *
 * @param counter_id The ID of the counter.
 * @param value The value of the counter.
 */

/* Call Linux tracepoint directly */
#define _vr_osk_profiling_report_hw_counter(counter_id, value) trace_vr_hw_counter(counter_id, value)

/**
 * Report SW counters
 *
 * @param counters array of counter values
 */
void _vr_osk_profiling_report_sw_counters(u32 *counters);

/**
 * Stop recording profiling data
 *
 * @param count Returns the number of recorded events.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t _vr_osk_profiling_stop(u32 * count);

/**
 * Retrieves the number of events that can be retrieved
 *
 * @return The number of recorded events that can be retrieved.
 */
u32 _vr_osk_profiling_get_count(void);

/**
 * Retrieve an event
 *
 * @param index Event index (start with 0 and continue until this function fails to retrieve all events)
 * @param timestamp The timestamp for the retrieved event will be stored here.
 * @param event_id The event ID for the retrieved event will be stored here.
 * @param data The 5 data values for the retrieved event will be stored here.
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t _vr_osk_profiling_get_event(u32 index, u64* timestamp, u32* event_id, u32 data[5]);

/**
 * Clear the recorded buffer.
 *
 * This is needed in order to start another recording.
 *
 * @return _VR_OSK_ERR_OK on success, otherwise failure.
 */
_vr_osk_errcode_t _vr_osk_profiling_clear(void);

/**
 * Checks if a recording of profiling data is in progress
 *
 * @return VR_TRUE if recording of profiling data is in progress, VR_FALSE if not
 */
vr_bool _vr_osk_profiling_is_recording(void);

/**
 * Checks if profiling data is available for retrival
 *
 * @return VR_TRUE if profiling data is avaiable, VR_FALSE if not
 */
vr_bool _vr_osk_profiling_have_recording(void);

/** @} */ /* end group _vr_osk_profiling */

#else /* defined(CONFIG_VR400_PROFILING)  && defined(CONFIG_TRACEPOINTS) */

/* Dummy add_event, for when profiling is disabled. */

#define _vr_osk_profiling_add_event(event_id, data0, data1, data2, data3, data4)

#endif /* defined(CONFIG_VR400_PROFILING)  && defined(CONFIG_TRACEPOINTS) */

#endif /* __VR_OSK_PROFILING_H__ */


