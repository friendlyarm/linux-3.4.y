/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from NEXELL Limited
 * (C) COPYRIGHT 2007-2013 NEXELL Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from NEXELL Limited.
 */

#ifndef __VR_UTGARD_IOCTL_H__
#define __VR_UTGARD_IOCTL_H__

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/fs.h>       /* file system operations */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file vr_kernel_ioctl.h
 * Interface to the Linux device driver.
 * This file describes the interface needed to use the Linux device driver.
 * Its interface is designed to used by the HAL implementation through a thin arch layer.
 */

/**
 * ioctl commands
 */

#define VR_IOC_BASE           0x82
#define VR_IOC_CORE_BASE      (_VR_UK_CORE_SUBSYSTEM      + VR_IOC_BASE)
#define VR_IOC_MEMORY_BASE    (_VR_UK_MEMORY_SUBSYSTEM    + VR_IOC_BASE)
#define VR_IOC_PP_BASE        (_VR_UK_PP_SUBSYSTEM        + VR_IOC_BASE)
#define VR_IOC_GP_BASE        (_VR_UK_GP_SUBSYSTEM        + VR_IOC_BASE)
#define VR_IOC_PROFILING_BASE (_VR_UK_PROFILING_SUBSYSTEM + VR_IOC_BASE)
#define VR_IOC_VSYNC_BASE     (_VR_UK_VSYNC_SUBSYSTEM + VR_IOC_BASE)

#define VR_IOC_WAIT_FOR_NOTIFICATION      _IOWR(VR_IOC_CORE_BASE, _VR_UK_WAIT_FOR_NOTIFICATION, _vr_uk_wait_for_notification_s *)
#define VR_IOC_GET_API_VERSION            _IOWR(VR_IOC_CORE_BASE, _VR_UK_GET_API_VERSION, _vr_uk_get_api_version_s *)
#define VR_IOC_POST_NOTIFICATION          _IOWR(VR_IOC_CORE_BASE, _VR_UK_POST_NOTIFICATION, _vr_uk_post_notification_s *)
#define VR_IOC_GET_USER_SETTING           _IOWR(VR_IOC_CORE_BASE, _VR_UK_GET_USER_SETTING, _vr_uk_get_user_setting_s *)
#define VR_IOC_GET_USER_SETTINGS          _IOWR(VR_IOC_CORE_BASE, _VR_UK_GET_USER_SETTINGS, _vr_uk_get_user_settings_s *)
#define VR_IOC_REQUEST_HIGH_PRIORITY      _IOW (VR_IOC_CORE_BASE, _VR_UK_REQUEST_HIGH_PRIORITY, _vr_uk_request_high_priority_s *)
#define VR_IOC_TIMELINE_GET_LATEST_POINT  _IOWR(VR_IOC_CORE_BASE, _VR_UK_TIMELINE_GET_LATEST_POINT, _vr_uk_timeline_get_latest_point_s *)
#define VR_IOC_TIMELINE_WAIT              _IOWR(VR_IOC_CORE_BASE, _VR_UK_TIMELINE_WAIT, _vr_uk_timeline_wait_s *)
#define VR_IOC_TIMELINE_CREATE_SYNC_FENCE _IOWR(VR_IOC_CORE_BASE, _VR_UK_TIMELINE_CREATE_SYNC_FENCE, _vr_uk_timeline_create_sync_fence_s *)
#define VR_IOC_SOFT_JOB_START             _IOWR(VR_IOC_CORE_BASE, _VR_UK_SOFT_JOB_START, _vr_uk_soft_job_start_s *)
#define VR_IOC_SOFT_JOB_SIGNAL            _IOWR(VR_IOC_CORE_BASE, _VR_UK_SOFT_JOB_SIGNAL, _vr_uk_soft_job_signal_s *)

#define VR_IOC_MEM_MAP_EXT                _IOWR(VR_IOC_MEMORY_BASE, _VR_UK_MAP_EXT_MEM, _vr_uk_map_external_mem_s *)
#define VR_IOC_MEM_UNMAP_EXT              _IOW (VR_IOC_MEMORY_BASE, _VR_UK_UNMAP_EXT_MEM, _vr_uk_unmap_external_mem_s *)
#define VR_IOC_MEM_ATTACH_DMA_BUF         _IOWR(VR_IOC_MEMORY_BASE, _VR_UK_ATTACH_DMA_BUF, _vr_uk_attach_dma_buf_s *)
#define VR_IOC_MEM_RELEASE_DMA_BUF        _IOW(VR_IOC_MEMORY_BASE, _VR_UK_RELEASE_DMA_BUF, _vr_uk_release_dma_buf_s *)
#define VR_IOC_MEM_DMA_BUF_GET_SIZE       _IOR(VR_IOC_MEMORY_BASE, _VR_UK_DMA_BUF_GET_SIZE, _vr_uk_dma_buf_get_size_s *)
#define VR_IOC_MEM_ATTACH_UMP             _IOWR(VR_IOC_MEMORY_BASE, _VR_UK_ATTACH_UMP_MEM, _vr_uk_attach_ump_mem_s *)
#define VR_IOC_MEM_RELEASE_UMP            _IOW(VR_IOC_MEMORY_BASE, _VR_UK_RELEASE_UMP_MEM, _vr_uk_release_ump_mem_s *)
#define VR_IOC_MEM_QUERY_MMU_PAGE_TABLE_DUMP_SIZE _IOR (VR_IOC_MEMORY_BASE, _VR_UK_QUERY_MMU_PAGE_TABLE_DUMP_SIZE, _vr_uk_query_mmu_page_table_dump_size_s *)
#define VR_IOC_MEM_DUMP_MMU_PAGE_TABLE    _IOWR(VR_IOC_MEMORY_BASE, _VR_UK_DUMP_MMU_PAGE_TABLE, _vr_uk_dump_mmu_page_table_s *)
#define VR_IOC_MEM_WRITE_SAFE             _IOWR(VR_IOC_MEMORY_BASE, _VR_UK_MEM_WRITE_SAFE, _vr_uk_mem_write_safe_s *)

#define VR_IOC_PP_START_JOB               _IOWR(VR_IOC_PP_BASE, _VR_UK_PP_START_JOB, _vr_uk_pp_start_job_s *)
#define VR_IOC_PP_AND_GP_START_JOB        _IOWR(VR_IOC_PP_BASE, _VR_UK_PP_AND_GP_START_JOB, _vr_uk_pp_and_gp_start_job_s *)
#define VR_IOC_PP_NUMBER_OF_CORES_GET	    _IOR (VR_IOC_PP_BASE, _VR_UK_GET_PP_NUMBER_OF_CORES, _vr_uk_get_pp_number_of_cores_s *)
#define VR_IOC_PP_CORE_VERSION_GET	    _IOR (VR_IOC_PP_BASE, _VR_UK_GET_PP_CORE_VERSION, _vr_uk_get_pp_core_version_s * )
#define VR_IOC_PP_DISABLE_WB              _IOW (VR_IOC_PP_BASE, _VR_UK_PP_DISABLE_WB, _vr_uk_pp_disable_wb_s * )

#define VR_IOC_GP2_START_JOB              _IOWR(VR_IOC_GP_BASE, _VR_UK_GP_START_JOB, _vr_uk_gp_start_job_s *)
#define VR_IOC_GP2_NUMBER_OF_CORES_GET    _IOR (VR_IOC_GP_BASE, _VR_UK_GET_GP_NUMBER_OF_CORES, _vr_uk_get_gp_number_of_cores_s *)
#define VR_IOC_GP2_CORE_VERSION_GET	    _IOR (VR_IOC_GP_BASE, _VR_UK_GET_GP_CORE_VERSION, _vr_uk_get_gp_core_version_s *)
#define VR_IOC_GP2_SUSPEND_RESPONSE	    _IOW (VR_IOC_GP_BASE, _VR_UK_GP_SUSPEND_RESPONSE,_vr_uk_gp_suspend_response_s *)

#define VR_IOC_PROFILING_START            _IOWR(VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_START, _vr_uk_profiling_start_s *)
#define VR_IOC_PROFILING_ADD_EVENT        _IOWR(VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_ADD_EVENT, _vr_uk_profiling_add_event_s*)
#define VR_IOC_PROFILING_STOP             _IOWR(VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_STOP, _vr_uk_profiling_stop_s *)
#define VR_IOC_PROFILING_GET_EVENT        _IOWR(VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_GET_EVENT, _vr_uk_profiling_get_event_s *)
#define VR_IOC_PROFILING_CLEAR            _IOWR(VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_CLEAR, _vr_uk_profiling_clear_s *)
#define VR_IOC_PROFILING_GET_CONFIG       _IOWR(VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_GET_CONFIG, _vr_uk_get_user_settings_s *)
#define VR_IOC_PROFILING_REPORT_SW_COUNTERS  _IOW (VR_IOC_PROFILING_BASE, _VR_UK_PROFILING_REPORT_SW_COUNTERS, _vr_uk_sw_counters_report_s *)

#define VR_IOC_VSYNC_EVENT_REPORT         _IOW (VR_IOC_VSYNC_BASE, _VR_UK_VSYNC_EVENT_REPORT, _vr_uk_vsync_event_report_s *)

/* Deprecated ioctls */
#define VR_IOC_MEM_GET_BIG_BLOCK          _IOWR(VR_IOC_MEMORY_BASE, _VR_UK_GET_BIG_BLOCK, void *)
#define VR_IOC_MEM_FREE_BIG_BLOCK         _IOW (VR_IOC_MEMORY_BASE, _VR_UK_FREE_BIG_BLOCK, void *)
#define VR_IOC_MEM_INIT                   _IOR (VR_IOC_MEMORY_BASE, _VR_UK_INIT_MEM, void *)
#define VR_IOC_MEM_TERM                   _IOW (VR_IOC_MEMORY_BASE, _VR_UK_TERM_MEM, void *)

#ifdef __cplusplus
}
#endif

#endif /* __VR_UTGARD_IOCTL_H__ */
