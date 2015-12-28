/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2008-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

/**
 * @file vr_ukk_wrappers.h
 * Defines the wrapper functions for each user-kernel function
 */

#ifndef __VR_UKK_WRAPPERS_H__
#define __VR_UKK_WRAPPERS_H__

#include "vr_uk_types.h"
#include "vr_osk.h"

#ifdef __cplusplus
extern "C" {
#endif

int wait_for_notification_wrapper(struct vr_session_data *session_data, _vr_uk_wait_for_notification_s __user *uargs);
int get_api_version_wrapper(struct vr_session_data *session_data, _vr_uk_get_api_version_s __user *uargs);
int get_user_settings_wrapper(struct vr_session_data *session_data, _vr_uk_get_user_settings_s __user *uargs);
int post_notification_wrapper(struct vr_session_data *session_data, _vr_uk_post_notification_s __user *uargs);
int request_high_priority_wrapper(struct vr_session_data *session_data, _vr_uk_request_high_priority_s __user *uargs);

int mem_write_safe_wrapper(struct vr_session_data *session_data, _vr_uk_mem_write_safe_s __user * uargs);
int mem_map_ext_wrapper(struct vr_session_data *session_data, _vr_uk_map_external_mem_s __user * argument);
int mem_unmap_ext_wrapper(struct vr_session_data *session_data, _vr_uk_unmap_external_mem_s __user * argument);
int mem_query_mmu_page_table_dump_size_wrapper(struct vr_session_data *session_data, _vr_uk_query_mmu_page_table_dump_size_s __user * uargs);
int mem_dump_mmu_page_table_wrapper(struct vr_session_data *session_data, _vr_uk_dump_mmu_page_table_s __user * uargs);

int timeline_get_latest_point_wrapper(struct vr_session_data *session, _vr_uk_timeline_get_latest_point_s __user *uargs);
int timeline_wait_wrapper(struct vr_session_data *session, _vr_uk_timeline_wait_s __user *uargs);
int timeline_create_sync_fence_wrapper(struct vr_session_data *session, _vr_uk_timeline_create_sync_fence_s __user *uargs);
int soft_job_start_wrapper(struct vr_session_data *session, _vr_uk_soft_job_start_s __user *uargs);
int soft_job_signal_wrapper(struct vr_session_data *session, _vr_uk_soft_job_signal_s __user *uargs);

#if defined(CONFIG_VR400_UMP)
int mem_attach_ump_wrapper(struct vr_session_data *session_data, _vr_uk_attach_ump_mem_s __user * argument);
int mem_release_ump_wrapper(struct vr_session_data *session_data, _vr_uk_release_ump_mem_s __user * argument);
#endif

int pp_start_job_wrapper(struct vr_session_data *session_data, _vr_uk_pp_start_job_s __user *uargs);
int pp_and_gp_start_job_wrapper(struct vr_session_data *session_data, _vr_uk_pp_and_gp_start_job_s __user *uargs);
int pp_get_number_of_cores_wrapper(struct vr_session_data *session_data, _vr_uk_get_pp_number_of_cores_s __user *uargs);
int pp_get_core_version_wrapper(struct vr_session_data *session_data, _vr_uk_get_pp_core_version_s __user *uargs);
int pp_disable_wb_wrapper(struct vr_session_data *session_data, _vr_uk_pp_disable_wb_s __user *uargs);
int gp_start_job_wrapper(struct vr_session_data *session_data, _vr_uk_gp_start_job_s __user *uargs);
int gp_get_number_of_cores_wrapper(struct vr_session_data *session_data, _vr_uk_get_gp_number_of_cores_s __user *uargs);
int gp_get_core_version_wrapper(struct vr_session_data *session_data, _vr_uk_get_gp_core_version_s __user *uargs);
int gp_suspend_response_wrapper(struct vr_session_data *session_data, _vr_uk_gp_suspend_response_s __user *uargs);

int profiling_start_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_start_s __user *uargs);
int profiling_add_event_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_add_event_s __user *uargs);
int profiling_stop_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_stop_s __user *uargs);
int profiling_get_event_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_get_event_s __user *uargs);
int profiling_clear_wrapper(struct vr_session_data *session_data, _vr_uk_profiling_clear_s __user *uargs);
int profiling_report_sw_counters_wrapper(struct vr_session_data *session_data, _vr_uk_sw_counters_report_s __user *uargs);

int vsync_event_report_wrapper(struct vr_session_data *session_data, _vr_uk_vsync_event_report_s __user *uargs);


int map_errcode( _vr_osk_errcode_t err );

#ifdef __cplusplus
}
#endif

#endif /* __VR_UKK_WRAPPERS_H__ */
