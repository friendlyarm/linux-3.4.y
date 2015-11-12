/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.scan.h
 * Date         : 2014.09.18
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.09.18, SD.LEE)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_SCAN_GLOBAL__
#define NFC_PHY_SCAN_EXT
#else
#define NFC_PHY_SCAN_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
#include "../../exchange.h"

/******************************************************************************
 *
 ******************************************************************************/
#define NAND_MAKER_SEC          (0xEC)
#define NAND_MAKER_TOSHIBA      (0x98)
#define NAND_MAKER_SKHYNIX      (0xAD)
#define NAND_MAKER_MICRON       (0x2C)
#define NAND_MAKER_INTEL        (0x89)

/******************************************************************************
 *
 ******************************************************************************/
#define NAND_FIELD_GENERATION   (16)
#define NAND_FIELD_MAKER        (24)

#define NAND_MASK_GENERATION    (0xFF<<NAND_FIELD_GENERATION)
#define NAND_MASK_MAKER         (0xFF<<NAND_FIELD_MAKER)

/******************************************************************************
 * NAND Maker Samsung
 ******************************************************************************/

/******************************************************************************
 * NAND Maker Toshiba, Not Support
 ******************************************************************************/
// NAND Generation
#define NAND_TOSHIBA_24NM                       ((NAND_MAKER_TOSHIBA<<NAND_FIELD_MAKER) | (0x01<<NAND_FIELD_GENERATION))
#define NAND_TOSHIBA_19NM                       ((NAND_MAKER_TOSHIBA<<NAND_FIELD_MAKER) | (0x02<<NAND_FIELD_GENERATION))
#define NAND_TOSHIBA_1YNM                       ((NAND_MAKER_TOSHIBA<<NAND_FIELD_MAKER) | (0x04<<NAND_FIELD_GENERATION))

// NAND Model
#define NAND_TOSHIBA_TC58TEG6DDKTA00            (NAND_TOSHIBA_1YNM | (1<<0x00))     // Toshiba MLC 1Ynm [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Toggle,
#define NAND_TOSHIBA_TC58TEG6DDKTAI0            (NAND_TOSHIBA_1YNM | (1<<0x01))
#define NAND_TOSHIBA_TH58TEG7DDKTA20            (NAND_TOSHIBA_1YNM | (1<<0x02))
#define NAND_TOSHIBA_TH58TEG7DDKTAK0            (NAND_TOSHIBA_1YNM | (1<<0x03))
#define NAND_TOSHIBA_TH58TEG8DDKTA20            (NAND_TOSHIBA_1YNM | (1<<0x04))
#define NAND_TOSHIBA_TH58TEG8DDKTAK0            (NAND_TOSHIBA_1YNM | (1<<0x05))

#define NAND_TOSHIBA_TC58TEG5DCKTA00            (NAND_TOSHIBA_1YNM | (1<<0x06))     // Toshiba MLC 1Ynm [ 1 DIE, 1 CE, ? R/B, Common IO   ]     32Gb, Toggle,
#define NAND_TOSHIBA_TC58TEG5DCKTAI0            (NAND_TOSHIBA_1YNM | (1<<0x07))

#define NAND_TOSHIBA_TC58TEG6DCJTA00            (NAND_TOSHIBA_19NM | (1<<0x00))     // Toshiba MLC 19nm [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Toggle,
#define NAND_TOSHIBA_TC58TEG6DCJTAI0            (NAND_TOSHIBA_19NM | (1<<0x01))
#define NAND_TOSHIBA_TH58TEG7DCJTA20            (NAND_TOSHIBA_19NM | (1<<0x02))     // Toshiba MLC 19nm [ 2 DIE, 2 CE, 2 R/B, Common IO   ]     128Gb, Toggle,
#define NAND_TOSHIBA_TH58TEG7DCJTAK0            (NAND_TOSHIBA_19NM | (1<<0x03))
#define NAND_TOSHIBA_TH58TEG8DCJTA20            (NAND_TOSHIBA_19NM | (1<<0x04))     // Toshiba MLC 19nm [ 4 DIE, 2 CE, 2 R/B, Common IO   ]     256Gb, Toggle,
#define NAND_TOSHIBA_TH58TEG8DCJTAK0            (NAND_TOSHIBA_19NM | (1<<0x05))

#define NAND_TOSHIBA_TC58TEG5DCJTA00			(NAND_TOSHIBA_19NM | (1<<0x06))		// Toshiba MLC 19nm

/******************************************************************************
 * NAND Maker SKHynix
 ******************************************************************************/

// NAND Generation
#define NAND_HYNIX_20NM_A_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0x01<<NAND_FIELD_GENERATION))
#define NAND_HYNIX_20NM_B_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0x02<<NAND_FIELD_GENERATION))
#define NAND_HYNIX_20NM_C_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0x04<<NAND_FIELD_GENERATION))

#define NAND_HYNIX_16NM_A_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0x11<<NAND_FIELD_GENERATION))
#define NAND_HYNIX_16NM_B_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0x12<<NAND_FIELD_GENERATION))
#define NAND_HYNIX_16NM_C_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0x14<<NAND_FIELD_GENERATION))
#define NAND_HYNIX_16NM_x_DIE                   ((NAND_MAKER_SKHYNIX<<NAND_FIELD_MAKER) | (0xFF<<NAND_FIELD_GENERATION)) // die is not recognized. this is temporary.

// NAND Model
#define NAND_HYNIX_H27UCG8T2ATR                 (NAND_HYNIX_20NM_A_DIE | (1<<0))    // Hynix MLC 20nm       [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async
#define NAND_HYNIX_H27UCG8T2BTR                 (NAND_HYNIX_20NM_B_DIE | (1<<0))    // Hynix MLC 20nm       [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async
#define NAND_HYNIX_H27UBG8T2CTR                 (NAND_HYNIX_20NM_C_DIE | (1<<0))    // Hynix MLC 20nm       [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     32Gb, Async
#define NAND_HYNIX_H27UBG8T2DTR                 (NAND_HYNIX_16NM_x_DIE | (1<<0))    // Hynix MLC 16nm       [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     32Gb, Async
#define NAND_HYNIX_H27UCG8T2ETR                 (NAND_HYNIX_16NM_B_DIE | (1<<0))    // Hynix MLC 16nm       [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async

/******************************************************************************
 * NAND Maker Micron
 ******************************************************************************/

// NAND Generation
#define NAND_MICRON_L74A                        ((NAND_MAKER_MICRON<<NAND_FIELD_MAKER) | (0x01<<NAND_FIELD_GENERATION))
#define NAND_MICRON_L83A                        ((NAND_MAKER_MICRON<<NAND_FIELD_MAKER) | (0x02<<NAND_FIELD_GENERATION))
#define NAND_MICRON_L84A                        ((NAND_MAKER_MICRON<<NAND_FIELD_MAKER) | (0x04<<NAND_FIELD_GENERATION))
#define NAND_MICRON_L85A                        ((NAND_MAKER_MICRON<<NAND_FIELD_MAKER) | (0x08<<NAND_FIELD_GENERATION))

// NAND Model
#define NAND_MICRON_L74A_MT29F64G08CBAAA        (NAND_MICRON_L74A | (1<<0x00))      // Micron MLC L74A      [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F64G08CBCAB        (NAND_MICRON_L74A | (1<<0x01))      // Micron MLC L74A      [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async/Sync, ONFI 2.2,
#define NAND_MICRON_L74A_MT29F128G08CEAAA       (NAND_MICRON_L74A | (1<<0x02))      // Micron MLC L74A      [ 2 DIE, 2 CE, 2 R/B, Seperate IO ]    128Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F128G08CFAAA       (NAND_MICRON_L74A | (1<<0x03))      // Micron MLC L74A      [ 2 DIE, 2 CE, 2 R/B, Common IO   ]    128Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F128G08CECAB       (NAND_MICRON_L74A | (1<<0x04))      // Micron MLC L74A      [ 2 DIE, 2 CE, 2 R/B, Seperate IO ]    128Gb, Async/Sync, ONFI 2.2,
#define NAND_MICRON_L74A_MT29F256G08CJAAA       (NAND_MICRON_L74A | (1<<0x05))      // Micron MLC L74A      [ 4 DIE, 2 CE, 2 R/B, Common IO   ]    256Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F256G08CKAAA       (NAND_MICRON_L74A | (1<<0x06))      // Micron MLC L74A      [ 4 DIE, 2 CE, 2 R/B, Seperate IO ]    256Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F256G08CKCAB       (NAND_MICRON_L74A | (1<<0x07))      // Micron MLC L74A      [ 4 DIE, 2 CE, 2 R/B, Seperate IO ]    256Gb, Async/Sync, ONFI 2.2,
#define NAND_MICRON_L74A_MT29F256G08CMAAA       (NAND_MICRON_L74A | (1<<0x08))      // Micron MLC L74A      [ 4 DIE, 4 CE, 4 R/B, Seperate IO ]    256Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F256G08CMCAB       (NAND_MICRON_L74A | (1<<0x09))      // Micron MLC L74A      [ 4 DIE, 4 CE, 4 R/B, Seperate IO ]    256Gb, Async/Sync, ONFI 2.2,
#define NAND_MICRON_L74A_MT29F512G08CUAAA       (NAND_MICRON_L74A | (1<<0x0A))      // Micron MLC L74A      [ 8 DIE, 4 CE, 4 R/B, Seperate IO ]    512Gb, Async,      ONFI 2.2,
#define NAND_MICRON_L74A_MT29F512G08CUCAB       (NAND_MICRON_L74A | (1<<0x0B))      // Micron MLC L74A      [ 8 DIE, 4 CE, 4 R/B, Seperate IO ]    512Gb, Async/Sync, ONFI 2.2,
#define NAND_MICRON_L74A_MT29F128G08CFAAB       (NAND_MICRON_L74A | (1<<0x0C))      // Micron MLC L74A      [ 2 DIE, 2 CE, 2 R/B, Common IO   ]    128Gb, Async/Sync, ONFI 2.2,

#define NAND_MICRON_L83A_MT29F32G08CBADA        (NAND_MICRON_L83A | (1<<0x00))
#define NAND_MICRON_L83A_MT29F32G08CBADB        (NAND_MICRON_L83A | (1<<0x01))      // Micron MLC L83A      [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     32Gb, Async/Sync, ONFI 2.3,
#define NAND_MICRON_L83A_MT29F64G08CFADA        (NAND_MICRON_L83A | (1<<0x02))
#define NAND_MICRON_L83A_MT29F64G08CFADB        (NAND_MICRON_L83A | (1<<0x03))
#define NAND_MICRON_L83A_MT29F64G08CECDB        (NAND_MICRON_L83A | (1<<0x04))
#define NAND_MICRON_L83A_MT29F128G08CMCDB       (NAND_MICRON_L83A | (1<<0x05))
#define NAND_MICRON_L83A_MT29F256G08CUCDB       (NAND_MICRON_L83A | (1<<0x06))

#define NAND_MICRON_L84A_MT29F64G08CBABA        (NAND_MICRON_L84A | (1<<0x00))      // Micron MLC L84A      [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async,      ONFI 2.3,
#define NAND_MICRON_L84A_MT29F64G08CBABB        (NAND_MICRON_L84A | (1<<0x01))
#define NAND_MICRON_L84A_MT29F64G08CBCBB        (NAND_MICRON_L84A | (1<<0x02))
#define NAND_MICRON_L84A_MT29F128G08CECBB       (NAND_MICRON_L84A | (1<<0x03))
#define NAND_MICRON_L84A_MT29F128G08CFABA       (NAND_MICRON_L84A | (1<<0x04))
#define NAND_MICRON_L84A_MT29F128G08CFABB       (NAND_MICRON_L84A | (1<<0x05))
#define NAND_MICRON_L84A_MT29F256G08CJABA       (NAND_MICRON_L84A | (1<<0x06))
#define NAND_MICRON_L84A_MT29F256G08CJABB       (NAND_MICRON_L84A | (1<<0x07))
#define NAND_MICRON_L84A_MT29F256G08CKCBB       (NAND_MICRON_L84A | (1<<0x08))
#define NAND_MICRON_L84A_MT29F256G08CMCBB       (NAND_MICRON_L84A | (1<<0x09))

#define NAND_MICRON_L85A_MT29F128G08CBCAB       (NAND_MICRON_L85A | (1<<0x00))
#define NAND_MICRON_L85A_MT29F128G08CBEAB       (NAND_MICRON_L85A | (1<<0x01))
#define NAND_MICRON_L85A_MT29F256G08CECAB       (NAND_MICRON_L85A | (1<<0x02))
#define NAND_MICRON_L85A_MT29F256G08CEEAB       (NAND_MICRON_L85A | (1<<0x03))
#define NAND_MICRON_L85A_MT29F512G08CKCAB       (NAND_MICRON_L85A | (1<<0x04))
#define NAND_MICRON_L85A_MT29F512G08CKEAB       (NAND_MICRON_L85A | (1<<0x05))
#define NAND_MICRON_L85A_MT29F512G08CMCAB       (NAND_MICRON_L85A | (1<<0x06))      // Microm MLC L85A      [ 4 DIE, 4 CE, 4 R/B, Seperate IO ]    512Gb, Async/Sync, ONFI 3.0,
#define NAND_MICRON_L85A_MT29F512G08CMEAB       (NAND_MICRON_L85A | (1<<0x07))
#define NAND_MICRON_L85A_MT29F1T08CUCAB         (NAND_MICRON_L85A | (1<<0x08))      // Microm MLC L85A      [ 8 DIE, 4 CE, 4 R/B, Seperate IO ]      1Tb, Async/Sync, ONFI 3.0,
#define NAND_MICRON_L85A_MT29F1T08CUEAB         (NAND_MICRON_L85A | (1<<0x09))

/******************************************************************************
 * NAND Maker Intel
 ******************************************************************************/

// NAND Generation
#define NAND_INTEL_25NM_MLC                     ((NAND_MAKER_INTEL<<NAND_FIELD_MAKER) | (0x01<<NAND_FIELD_GENERATION))
#define NAND_INTEL_25NM_SLC                     ((NAND_MAKER_INTEL<<NAND_FIELD_MAKER) | (0x02<<NAND_FIELD_GENERATION))

// NAND Model
#define NAND_INTEL_JS29F64G08AAME1              (NAND_INTEL_25NM_MLC | (1<0x00))    // Intel 25nm MLC       [ 1 DIE, 1 CE, 1 R/B, Common IO   ]     64Gb, Async,      ONFI 2.2
#define NAND_INTEL_JS29F16B08CAME1              (NAND_INTEL_25NM_MLC | (1<0x01))    // Intel 25nm MLC       [ 2 DIE, 2 CE, 2 R/B, Common IO   ]    128Gb, Async,      ONFI 2.2
#define NAND_INTEL_JS29F32B08JAME1              (NAND_INTEL_25NM_MLC | (1<0x02))    // Intel 25nm MLC       [ 4 DIE, 4 CE, 4 R/B, Common IO   ]    256Gb, Async,      ONFI 2.2
#define NAND_INTEL_JS29F64G08CCNE1              (NAND_INTEL_25NM_SLC | (1<0x03))    // Intel 25nm SLC       [ 2 DIE, 2 CE, 2 R/B, Common IO   ]    128Gb, Async/Sync, ONFI 2.2
#define NAND_INTEL_JS29F16B08JCNE1              (NAND_INTEL_25NM_SLC | (1<0x04))    // Intel 25nm SLC       [ 4 DIE, 4 CE, 4 R/B, Common IO   ]    128Gb, Async/Sync, ONFI 2.2

/******************************************************************************
 *
 * Various Parameter Data Structure
 *
 ******************************************************************************/
#pragma pack(1)
typedef union __ONFI_PARAMETER__
{
    unsigned char _c[256];

    struct
    {
        /**********************************************************************
         * Revision information and features block
         **********************************************************************/
        unsigned char parameter_page_signature[4];

        struct
        {
            unsigned short _rsvd0           : 1;
            unsigned short onfi_version_1_0 : 1;
            unsigned short onfi_version_2_0 : 1;
            unsigned short onfi_version_2_1 : 1;
            unsigned short onfi_version_2_2 : 1;
            unsigned short onfi_version_2_3 : 1;
            unsigned short onfi_version_3_0 : 1;
            unsigned short onfi_version_3_1 : 1;
            unsigned short onfi_version_3_2 : 1;
            unsigned short onfi_version_4_0 : 1;
            unsigned short _rsvd1           : 6;

        } revision_number;

        struct
        {
            unsigned short data_bus_width_16                        : 1;
            unsigned short multiple_lun_operations                  : 1;
            unsigned short non_sequential_page_programming          : 1;
            unsigned short multi_plane_program_and_erase_operations : 1;
            unsigned short odd_to_even_page_copyback                : 1;
            unsigned short synchronous_interface                    : 1;
            unsigned short multi_plane_read_operations              : 1;
            unsigned short extended_parameter_page                  : 1;
            unsigned short program_page_register_clear_enhancement  : 1;
            unsigned short ez_nand                                  : 1;
            unsigned short nv_ddr2                                  : 1;
            unsigned short volume_addressing                        : 1;
            unsigned short external_vpp                             : 1;
            unsigned short nv_ddr3                                  : 1;
            unsigned short zq_calibration                           : 1;
            unsigned short package_electrical_specification         : 1;

        } features_supported;

        struct
        {
            unsigned short page_cache_program_command    : 1;
            unsigned short read_cache_commands           : 1;
            unsigned short get_features_and_set_features : 1;
            unsigned short read_status_enhanced          : 1;
            unsigned short copyback                      : 1;
            unsigned short read_unique_id                : 1;
            unsigned short change_read_column_enhanced   : 1;
            unsigned short change_row_address            : 1;
            unsigned short small_data_move               : 1;
            unsigned short reset_lun                     : 1;
            unsigned short volume_select                 : 1;
            unsigned short odt_configure                 : 1;
            unsigned short lun_get_and_lun_set_features  : 1;
            unsigned short zq_calibration                : 1;
            unsigned short _rsvd0                        : 2;

        } optional_commands_supported;

        struct
        {
            unsigned char multi_plane_block_erase      : 1;
            unsigned char multi_plane_copyback_program : 1;
            unsigned char multi_plane_page_program     : 1;
            unsigned char random_data_out              : 1;
            unsigned char _rsvd0                       : 4;

        } onfi_jedec_jtg_primary_advanced_command_support;

        unsigned char _rsvd0;
        unsigned short extended_parameter_page_length;
        unsigned char number_of_parameter_pages;
        unsigned char _rsvd1[17];

        /**********************************************************************
         * Manufacturer information block
         **********************************************************************/
        unsigned char device_manufacturer[12];
        unsigned char device_model[20];
        unsigned char jedec_manufacturer_id;
        unsigned char date_code[2];
        unsigned char _rsvd2[13];

        /**********************************************************************
         * Memory organization block
         **********************************************************************/
        unsigned int  number_of_data_bytes_per_page;
        unsigned short number_of_spare_bytes_per_page;
        unsigned char _rsvd3[6];
        unsigned int  number_of_pages_per_block;
        unsigned int  number_of_blocks_per_lun;
        unsigned char number_of_luns_per_chip_enable;

        struct
        {
            unsigned char row_address_cycle    : 4;
            unsigned char column_address_cycle : 4;

        } number_of_address_cycles;

        unsigned char number_of_bits_per_cell;
        unsigned short bad_blocks_maximum_per_lun;
        unsigned char block_endurance[2];
        unsigned char guaranteed_valid_blocks_at_beginning_of_target;
        unsigned short block_endurance_for_guaranteed_valid_blocks;
        unsigned char number_of_programs_per_page;
        unsigned char _rsvd4;
        unsigned char number_of_bits_ecc_correctability;
        unsigned char number_of_plane_address_bits;

        struct
        {
            unsigned char overlapped_concurrent_multi_plane_support : 1;
            unsigned char no_block_address_restrictions             : 1;
            unsigned char program_cache_supported                   : 1;
            unsigned char address_restrictions_for_cache_operations : 1;
            unsigned char read_cache_supported                      : 1;
            unsigned char lower_bit_xnor_block_address_restriction  : 1;
            unsigned char _rsvd0                                    : 2;

        } multi_plane_operation_attributes;

        unsigned char ez_nand_support;

        unsigned char _rsvd5[12];

        /**********************************************************************
         * Electrical parameters block
         **********************************************************************/
        unsigned char io_pin_capacitance_maximum;
        unsigned short sdr_timing_mode_support;
        unsigned char _rsvd6[2];
        unsigned short tPROG_maximum_page_program_time_us;
        unsigned short tBERS_maximum_block_erase_time_us;
        unsigned short tR_maximum_page_read_time_us;
        unsigned short tCCS_minimum_change_column_setup_time_ns;
        unsigned char nvddr_timing_mode_support;
        unsigned char nvddr2_timing_mode_support;
        unsigned char nvddr_nvddr2_features;
        unsigned short clk_input_pin_capacitance_typical;
        unsigned short io_pin_capacitance_typical;
        unsigned short input_pin_capacitance_typical;
        unsigned char input_pin_capacitance_maximum;

        struct
        {
            unsigned char driver_strength_settings : 1;
            unsigned char drive_strength_25_ohm    : 1;
            unsigned char drive_strength_18_ohm    : 1;
            unsigned char _rsvd0                   : 5;

        } driver_strength_support;

        unsigned short tR_maximum_multi_plane_page_read_time_us;
        unsigned short tADL_program_page_register_clear_enhancement_tADL_value_ns;
        unsigned short tR_typical_page_read_time_for_eznand_us;
        unsigned char nvddr2_3_features;
        unsigned char nvddr2_3_warmup_cycles;
        unsigned short nvddr3_timing_mode_support;
        unsigned char nvddr2_timing_mode_ext_support;
        unsigned char _rsvd8;

        /**********************************************************************
         * Vendor block
         **********************************************************************/
        unsigned char vendor_specific_revision_number[2];
        unsigned char vendor_specific[88];
        unsigned short integrity_crc;

    } _f;

} ONFI_PARAMETER;
#pragma pack()

#pragma pack(1)
typedef union __ONFI_EXT_PARAMETER__
{
    unsigned char _c[48];

    struct
    {
        unsigned short extended_parameter_page_integrity_crc;
        unsigned char  extended_parameter_page_signature[4];
        unsigned char  _rsvd0[10];
        // Section Type & Length
        unsigned char  section_0_type;
        unsigned char  section_0_length;
        unsigned char  section_1_type;
        unsigned char  section_1_length;
        unsigned char  section_2_type;
        unsigned char  section_2_length;
        unsigned char  section_3_type;
        unsigned char  section_3_length;
        unsigned char  section_4_type;
        unsigned char  section_4_length;
        unsigned char  section_5_type;
        unsigned char  section_5_length;
        unsigned char  section_6_type;
        unsigned char  section_6_length;
        unsigned char  section_7_type;
        unsigned char  section_7_length;
        // Section Information
        unsigned char  number_of_bits_ecc_correctability;
        unsigned char  ecc_codeword_size;
        unsigned short bad_blocks_maximum_per_lun;
        unsigned char  block_endurance[2];
        unsigned char  _rsvd2[10];

    } _f;

} ONFI_EXT_PARAMETER;
#pragma pack()

#pragma pack(1)
typedef union __JEDEC_PARAMETER__
{
    unsigned char _c[512];

    struct
    {
        /**********************************************************************
         * Revision Information and Features Block
         **********************************************************************/
        struct
        {
            unsigned char  parameter_page_signature[4];
            unsigned short revision_number;

            struct
            {
                unsigned short data_bus_width_16                        : 1;
                unsigned short multiple_lun_operations                  : 1;
                unsigned short non_sequential_page_programming          : 1;
                unsigned short multi_plane_program_and_erase_operations : 1;
                unsigned short multi_plane_read_operations              : 1;
                unsigned short synchronous_ddr                          : 1;
                unsigned short toggle_Mode_ddr                          : 1;
                unsigned short external_vpp                             : 1;
                unsigned short program_page_register_clear_enhancement  : 1;
                unsigned short _rsvd                                    : 7;

            } features_supported;

            struct
            {
                unsigned short supports_page_cache_program_command    : 1;
                unsigned short supports_read_cache_commands           : 1;
                unsigned short supports_get_features_and_set_features : 1;
                unsigned short supports_read_status_enhanced          : 1;
                unsigned short supports_copyback                      : 1;
                unsigned short supports_read_unique_id                : 1;
                unsigned short supports_random_data_out               : 1;
                unsigned short supports_multi_plane_copyback_program  : 1;
                unsigned short supports_small_data_move               : 1;
                unsigned short supports_reset_lun                     : 1;
                unsigned short supports_synchronous_reset             : 1;
                unsigned short _rsvd0                                 : 13;

            } optional_commands_supported;

            unsigned short secondary_commands_supported;
            unsigned char  number_of_parameter_pages;
            unsigned char  _rsvd0[18];

        } revision_info_and_features;

        /**********************************************************************
         * Manufacturer information block
         **********************************************************************/
        struct
        {
            unsigned char device_manufacturer[12];
            unsigned char device_model[20];
            unsigned char jedec_manufacturer_id[6];
            unsigned char _rsvd0[10];

        } manufacturer_information;

        /**********************************************************************
         * Memory organization block
         **********************************************************************/
        struct
        {
            unsigned int   number_of_data_bytes_per_page;
            unsigned short number_of_spare_bytes_per_page;
            unsigned char  _rsvd0[6];
            unsigned int   number_of_pages_per_block;
            unsigned int   number_of_blocks_per_lun;
            unsigned char  number_of_luns_per_chip_enable;

            unsigned char  number_of_address_cycles;
            unsigned char  number_of_bits_per_cell;
            unsigned char  number_of_programs_per_page;

            struct
            {
                unsigned char number_of_plane_address_bits : 4;
                unsigned char _rsvd0                       : 4;

            } multi_plane_operation_addressing;

            struct
            {
                unsigned char no_multi_plane_block_address_restrictions : 1;
                unsigned char program_cache_supported                   : 1;
                unsigned char read_cache_supported                      : 1;
                unsigned char _rsvd0                                    : 5;

            } multi_plane_operation_attributes;

            unsigned char  _rsvd1[38];

        } memory_organization;

        /**********************************************************************
         * Electrical parameters block
         **********************************************************************/
        struct
        {
            struct
            {
                unsigned short supports_100ns_speed_grade : 1;
                unsigned short supports_50ns_speed_grade  : 1;
                unsigned short supports_35ns_speed_grade  : 1;
                unsigned short supports_30ns_speed_grade  : 1;
                unsigned short supports_25ns_speed_grade  : 1;
                unsigned short supports_20ns_speed_grade  : 1;
                unsigned short _rsvd0                     : 10;

            } asynchronous_sdr_speed_grade;
        
            struct
            {
                unsigned short supports_30ns_speed_grade : 1;
                unsigned short supports_25ns_speed_grade : 1;
                unsigned short supports_15ns_speed_grade : 1;
                unsigned short supports_12ns_speed_grade : 1;
                unsigned short supports_10ns_speed_grade : 1;
                unsigned short _rsvd0                    : 11;

            } toggle_ddr_speed_grade;

            struct
            {
                unsigned short supports_50ns_speed_grade : 1;
                unsigned short supports_30ns_speed_grade : 1;
                unsigned short supports_20ns_speed_grade : 1;
                unsigned short supports_15ns_speed_grade : 1;
                unsigned short supports_12ns_speed_grade : 1;
                unsigned short supports_10ns_speed_grade : 1;
                unsigned short _rsvd0                    : 10;

            } synchronous_ddr_speed_grade;

            unsigned char  asynchronous_sdr_features;
            unsigned char  toggle_mode_ddr_features;
            unsigned char  synchronous_ddr_features;

            unsigned short tPROG;
            unsigned short tBERS;
            unsigned short tR;
            unsigned short tR_multi_plane;
            unsigned short tCCS;
            unsigned short io_pin_capacitance;
            unsigned short input_capacitance;
            unsigned short clk_pin_capacitance;

            struct
            {
                unsigned char supports_35ohm_or_50ohm : 1;
                unsigned char supports_25ohm          : 1;
                unsigned char supports_18ohm          : 1;
                unsigned char _rsvd0                  : 5;

            } driver_strength_support;

            unsigned short tADL;

            unsigned char  _rsvd0[36];

        } electrical_parameters;

        /**********************************************************************
         * ECC and endurance block
         **********************************************************************/
        struct
        {
            unsigned char  guaranteed_valid_blocks_of_target;
            unsigned short block_endurance_for_guaranteed_valid_blocks;

            struct
            {
                unsigned char  number_of_bits_ecc_correctability;
                unsigned char  codeword_size;
                unsigned short maximum_value_of_average_bad_blocks_per_lun;
                unsigned char  block_endurance[2];
                unsigned char  _rsvd0[2];

            } information_block_0;

            struct
            {
                unsigned char  number_of_bits_ecc_correctability;
                unsigned char  codeword_size;
                unsigned short maximum_value_of_average_bad_blocks_per_lun;
                unsigned char  block_endurance[2];
                unsigned char  _rsvd0[2];

            } information_block_1;

            struct
            {
                unsigned char  number_of_bits_ecc_correctability;
                unsigned char  codeword_size;
                unsigned short maximum_value_of_average_bad_blocks_per_lun;
                unsigned char  block_endurance[2];
                unsigned char  _rsvd0[2];

            } information_block_2;

            struct
            {
                unsigned char  number_of_bits_ecc_correctability;
                unsigned char  codeword_size;
                unsigned short maximum_value_of_average_bad_blocks_per_lun;
                unsigned char  block_endurance[2];
                unsigned char  _rsvd0[2];

            } information_block_3;

            unsigned char _rsvd6[177];

        } ecc_and_endurance;

        /**********************************************************************
         * Vendor specific block
         **********************************************************************/
        unsigned char vendor_specific_revision_number[2];
        unsigned char _rsvd0[88];

        /**********************************************************************
         * CRC for Parameter Page
         **********************************************************************/
        unsigned short integrity_crc;

    } _f;

} JEDEC_PARAMETER;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/
typedef struct __PHY_FEATURES__
{
    unsigned int max_channel;
    unsigned int max_way;

    NAND nand_config;

    ONFI_PARAMETER onfi_param;
    ONFI_EXT_PARAMETER onfi_ext_param;
    JEDEC_PARAMETER jedec_param;

} PHY_FEATURES;

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_SCAN_EXT PHY_FEATURES phy_features;

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanSec(unsigned char * _id, unsigned char * _jedec_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanToshiba(unsigned char * _id, unsigned char * _jedec_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanMicron(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanSkhynix(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format);
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanIntel(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format);

/******************************************************************************
 *
 ******************************************************************************/
NFC_PHY_SCAN_EXT unsigned int NFC_PHY_ScanFeature(unsigned int _scan_format);
