//------------------------------------------------------------------------------
//
//	Copyright (C) Nexell Co. 2012
//
//  This confidential and proprietary software may be used only as authorized by a
//  licensing agreement from Nexell Co.
//  The entire notice above must be reproduced on all authorized copies and copies
//  may only be made to the extent permitted by a licensing agreement from Nexell Co.
//
//	Module		:
//	File		: nx_hdmi.h
//	Description	:
//	Author		:
//	History		:
//------------------------------------------------------------------------------
#ifndef __NX_HDMI_H__
#define __NX_HDMI_H__

#include "../base/nx_prototype.h"

//---------------
// DisplayTop이 있다면 Include한다.
#ifdef NUMBER_OF_DISPLAYTOP_MODULE
#include "nx_displaytop.h"
#endif
//---------------

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	HDMI
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
// register가 너무 많으므로 아래와 같이 define문을 통해 접근하도록 한다.
#define  HDMI_LINK_INTC_CON_0                       (HDMI_ADDR_OFFSET + 0x00000000)    // Interrupt Control Register 0                                                 0x00
#define  HDMI_LINK_INTC_FLAG_0                      (HDMI_ADDR_OFFSET + 0x00000004)    // Interrupt Flag Register 0                                                    0x00
#define  HDMI_LINK_AESKEY_VALID                     (HDMI_ADDR_OFFSET + 0x00000008)    // i_aeskey_valid value                                                         0x0X
#define  HDMI_LINK_HPD                              (HDMI_ADDR_OFFSET + 0x0000000C)    // HPD signal                                                                   0x0X
#define  HDMI_LINK_INTC_CON_1                       (HDMI_ADDR_OFFSET + 0x00000010)    // Interrupt Control Register 1                                                 0x00
#define  HDMI_LINK_INTC_FLAG_1                      (HDMI_ADDR_OFFSET + 0x00000014)    // Interrupt Flag Register 1                                                    0x0X
#define  HDMI_LINK_PHY_STATUS_0                     (HDMI_ADDR_OFFSET + 0x00000020)    // PHY status Register 0                                                        0x0X
#define  HDMI_LINK_PHY_STATUS_CMU                   (HDMI_ADDR_OFFSET + 0x00000024)    // PHY CMU status Register                                                      0xXX
#define  HDMI_LINK_PHY_STATUS_PLL                   (HDMI_ADDR_OFFSET + 0x00000028)    // PHY PLL status Register                                                      0xXX
#define  HDMI_LINK_PHY_CON_0                        (HDMI_ADDR_OFFSET + 0x00000030)    // PHY Control Register                                                         0xXX
#define  HDMI_LINK_HPD_CTRL                         (HDMI_ADDR_OFFSET + 0x00000040)    // HPD Signal Control Register                                                  0xXX
#define  HDMI_LINK_HPD_STATUS                       (HDMI_ADDR_OFFSET + 0x00000044)    // HPD Status Register                                                          0xXX
#define  HDMI_LINK_HPD_TH_x                         (HDMI_ADDR_OFFSET + 0x00000050)    // HPD Status Register (HPD_TH_0~3)                                             0xXX

#define  HDMI_LINK_HDMI_CON_0                       (HDMI_ADDR_OFFSET + 0x00010000)    // HDMI system control register 0                                         0x00
#define  HDMI_LINK_HDMI_CON_1                       (HDMI_ADDR_OFFSET + 0x00010004)    // HDMI system control register 1                                         0x00
#define  HDMI_LINK_HDMI_CON_2                       (HDMI_ADDR_OFFSET + 0x00010008)    // HDMI system control register 2                                         0x00
#define  HDMI_LINK_STATUS                           (HDMI_ADDR_OFFSET + 0x00010010)    // HDMI system status register                                            0x00
#define  HDMI_LINK_STATUS_EN                        (HDMI_ADDR_OFFSET + 0x00010020)    // HDMI system status enable register                                     0x00

#define  HDMI_LINK_HDCP_SHA1_REN0 	                (HDMI_ADDR_OFFSET + 0x00010024)
#define  HDMI_LINK_HDCP_SHA1_REN1 	                (HDMI_ADDR_OFFSET + 0x00010028)

#define  HDMI_LINK_MODE_SEL                         (HDMI_ADDR_OFFSET + 0x00010040)    // HDMI/DVI mode selection                                                0x00
#define  HDMI_LINK_ENC_EN                           (HDMI_ADDR_OFFSET + 0x00010044)    // HDCP encryption enable register                                        0x00
#define  HDMI_LINK_HDMI_YMAX                        (HDMI_ADDR_OFFSET + 0x00010060)    // Maximum Y (or R,G,B) pixel value                                       0xEB
#define  HDMI_LINK_HDMI_YMIN                        (HDMI_ADDR_OFFSET + 0x00010064)    // Minimum Y (or R,G,B) pixel value                                       0x10
#define  HDMI_LINK_HDMI_CMAX                        (HDMI_ADDR_OFFSET + 0x00010068)    // Maximum Cb/Cr pixel value                                              0xF0
#define  HDMI_LINK_HDMI_CMIN                        (HDMI_ADDR_OFFSET + 0x0001006C)    // Minimum Cb/Cr pixel value                                              0x10
#define  HDMI_LINK_H_BLANK_0                        (HDMI_ADDR_OFFSET + 0x000100A0)    // Horizontal blanking setting                                            0x00
#define  HDMI_LINK_H_BLANK_1                        (HDMI_ADDR_OFFSET + 0x000100A4)    // Horizontal blanking setting                                            0x00
#define  HDMI_LINK_V2_BLANK_0                       (HDMI_ADDR_OFFSET + 0x000100B0)    // Vertical blanking setting                                              0x00
#define  HDMI_LINK_V2_BLANK_1                       (HDMI_ADDR_OFFSET + 0x000100B4)    // Vertical blanking setting                                              0x00
#define  HDMI_LINK_V1_BLANK_0                       (HDMI_ADDR_OFFSET + 0x000100B8)    // Vertical blanking setting                                              0x00
#define  HDMI_LINK_V1_BLANK_1                       (HDMI_ADDR_OFFSET + 0x000100BC)    // Vertical blanking setting                                              0x00
#define  HDMI_LINK_V_LINE_0                         (HDMI_ADDR_OFFSET + 0x000100C0)    // vertical line setting                                                  0x00
#define  HDMI_LINK_V_LINE_1                         (HDMI_ADDR_OFFSET + 0x000100C4)    // vertical line setting                                                  0x00
#define  HDMI_LINK_H_LINE_0                         (HDMI_ADDR_OFFSET + 0x000100C8)    // Horizontal line setting                                                0x00
#define  HDMI_LINK_H_LINE_1                         (HDMI_ADDR_OFFSET + 0x000100CC)    // Horizontal line setting                                                0x00
#define  HDMI_LINK_HSYNC_POL                        (HDMI_ADDR_OFFSET + 0x000100E0)    // Horizontal sync polarity control register                              0x00
#define  HDMI_LINK_VSYNC_POL                        (HDMI_ADDR_OFFSET + 0x000100E4)    // Vertical sync polarity control register                                0x00
#define  HDMI_LINK_INT_PRO_MODE                     (HDMI_ADDR_OFFSET + 0x000100E8)    // Interlace/Progressive control register                                 0x00
#define  HDMI_LINK_SEND_START_0                     (HDMI_ADDR_OFFSET + 0x000100F0)    //@modified choiyk 2012-12-19 오후 5:25:51
#define  HDMI_LINK_SEND_START_1                     (HDMI_ADDR_OFFSET + 0x000100F4)
#define  HDMI_LINK_SEND_END_0                       (HDMI_ADDR_OFFSET + 0x00010100)
#define  HDMI_LINK_SEND_END_1                       (HDMI_ADDR_OFFSET + 0x00010104)
#define  HDMI_LINK_SEND_END_2                       (HDMI_ADDR_OFFSET + 0x00010108)
#define  HDMI_LINK_V_BLANK_F0_0                     (HDMI_ADDR_OFFSET + 0x00010110)    // Vertical blanking setting for bottom field                             0xff
#define  HDMI_LINK_V_BLANK_F0_1                     (HDMI_ADDR_OFFSET + 0x00010114)    // Vertical blanking setting for bottom field                             0x1f
#define  HDMI_LINK_V_BLANK_F1_0                     (HDMI_ADDR_OFFSET + 0x00010118)    // Vertical blanking setting for bottom field                             0xff
#define  HDMI_LINK_V_BLANK_F1_1                     (HDMI_ADDR_OFFSET + 0x0001011C)    // Vertical blanking setting for bottom field                             0x1f
#define  HDMI_LINK_H_SYNC_START_0                   (HDMI_ADDR_OFFSET + 0x00010120)    // Horizontal sync generation setting                                     0x00
#define  HDMI_LINK_H_SYNC_START_1                   (HDMI_ADDR_OFFSET + 0x00010124)    // Horizontal sync generation setting                                     0x00
#define  HDMI_LINK_H_SYNC_END_0                     (HDMI_ADDR_OFFSET + 0x00010128)      // Horizontal sync generation setting                                       0x00
#define  HDMI_LINK_H_SYNC_END_1                     (HDMI_ADDR_OFFSET + 0x0001012C)      // Horizontal sync generation setting                                       0x00
#define  HDMI_LINK_V_SYNC_LINE_BEF_2_0              (HDMI_ADDR_OFFSET + 0x00010130)      // Vertical sync generation for top field or frame                          0xff
#define  HDMI_LINK_V_SYNC_LINE_BEF_2_1              (HDMI_ADDR_OFFSET + 0x00010134)      // Vertical sync generation for top field or frame                          0x1f
#define  HDMI_LINK_V_SYNC_LINE_BEF_1_0              (HDMI_ADDR_OFFSET + 0x00010138)      // Vertical sync generation for top field or frame                          0xff
#define  HDMI_LINK_V_SYNC_LINE_BEF_1_1              (HDMI_ADDR_OFFSET + 0x0001013C)      // Vertical sync generation for top field or frame                          0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_2_0              (HDMI_ADDR_OFFSET + 0x00010140)      // Vertical sync generation for bottom field - vertical position            0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_2_1              (HDMI_ADDR_OFFSET + 0x00010144)      // Vertical sync generation for bottom field - vertical position            0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_1_0              (HDMI_ADDR_OFFSET + 0x00010148)      // Vertical sync generation for bottom field - vertical position            0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_1_1              (HDMI_ADDR_OFFSET + 0x0001014C)      // Vertical sync generation for bottom field - vertical position            0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_0          (HDMI_ADDR_OFFSET + 0x00010150)      // Vertical sync generation for bottom field - horizontal position          0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_1          (HDMI_ADDR_OFFSET + 0x00010154)      // Vertical sync generation for bottom field - horizontal position          0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_0          (HDMI_ADDR_OFFSET + 0x00010158)      // Vertical sync generation for bottom field - horizontal position          0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_1          (HDMI_ADDR_OFFSET + 0x0001015C)      // Vertical sync generation for bottom field - horizontal position          0x1f
#define  HDMI_LINK_V_BLANK_F2_0                     (HDMI_ADDR_OFFSET + 0x00010160)      // Vertical blanking setting for third field                                0xff
#define  HDMI_LINK_V_BLANK_F2_1                     (HDMI_ADDR_OFFSET + 0x00010164)      // Vertical blanking setting for third field                                0x1f
#define  HDMI_LINK_V_BLANK_F3_0                     (HDMI_ADDR_OFFSET + 0x00010168)      // Vertical blanking setting for thrid field                                0xff
#define  HDMI_LINK_V_BLANK_F3_1                     (HDMI_ADDR_OFFSET + 0x0001016C)      // Vertical blanking setting for thrid field                                0x1f
#define  HDMI_LINK_V_BLANK_F4_0                     (HDMI_ADDR_OFFSET + 0x00010170)      // Vertical blanking setting for fourth field                               0xff
#define  HDMI_LINK_V_BLANK_F4_1                     (HDMI_ADDR_OFFSET + 0x00010174)      // Vertical blanking setting for fourth field                               0x1f
#define  HDMI_LINK_V_BLANK_F5_0                     (HDMI_ADDR_OFFSET + 0x00010178)      // Vertical blanking setting for fourth field                               0xff
#define  HDMI_LINK_V_BLANK_F5_1                     (HDMI_ADDR_OFFSET + 0x0001017C)      // Vertical blanking setting for fourth field                               0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_3_0              (HDMI_ADDR_OFFSET + 0x00010180)      // Vertical sync generation for third field - vertical position             0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_3_1              (HDMI_ADDR_OFFSET + 0x00010184)      // Vertical sync generation for third field - vertical position             0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_4_0              (HDMI_ADDR_OFFSET + 0x00010188)      // Vertical sync generation for third field - vertical position             0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_4_1              (HDMI_ADDR_OFFSET + 0x0001018C)      // Vertical sync generation for third field - vertical position             0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_5_0              (HDMI_ADDR_OFFSET + 0x00010190)      // Vertical sync generation for fourth field - vertical position            0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_5_1              (HDMI_ADDR_OFFSET + 0x00010194)      // Vertical sync generation for fourth field - vertical position            0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_6_0              (HDMI_ADDR_OFFSET + 0x00010198)      // Vertical sync generation for fourth field - vertical position            0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_6_1              (HDMI_ADDR_OFFSET + 0x0001019C)      // Vertical sync generation for fourth field - vertical position            0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_0          (HDMI_ADDR_OFFSET + 0x000101A0)      // Vertical sync generation for third field - horizontal position           0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_1          (HDMI_ADDR_OFFSET + 0x000101A4)      // Vertical sync generation for third field - horizontal position           0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_0          (HDMI_ADDR_OFFSET + 0x000101A8)      // Vertical sync generation for third field - horizontal position           0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_1          (HDMI_ADDR_OFFSET + 0x000101AC)      // Vertical sync generation for third field - horizontal position         0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_0          (HDMI_ADDR_OFFSET + 0x000101B0)      // Vertical sync generation for fourth field - horizontal position        0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_1          (HDMI_ADDR_OFFSET + 0x000101B4)      // Vertical sync generation for fourth field - horizontal position        0x1f
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_0          (HDMI_ADDR_OFFSET + 0x000101B8)      // Vertical sync generation for fourth field - horizontal position        0xff
#define  HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_1          (HDMI_ADDR_OFFSET + 0x000101BC)      // Vertical sync generation for fourth field - horizontal position        0x1f
#define  HDMI_LINK_VACT_SPACE1_0                    (HDMI_ADDR_OFFSET + 0x000101C0)      // 1st Vertical Active Space start line                                   0xff
#define  HDMI_LINK_VACT_SPACE1_1                    (HDMI_ADDR_OFFSET + 0x000101C4)      // 1st Vertical Active Space end line                                     0x1f
#define  HDMI_LINK_VACT_SPACE2_0                    (HDMI_ADDR_OFFSET + 0x000101C8)      // 1st Vertical Active Space start line                                   0xff
#define  HDMI_LINK_VACT_SPACE2_1                    (HDMI_ADDR_OFFSET + 0x000101CC)      // 1st Vertical Active Space end line                                     0x1f
#define  HDMI_LINK_VACT_SPACE3_0                    (HDMI_ADDR_OFFSET + 0x000101D0)      // 2nd Vertical Active Space start line                                   0xff
#define  HDMI_LINK_VACT_SPACE3_1                    (HDMI_ADDR_OFFSET + 0x000101D4)      // 2nd Vertical Active Space end line                                     0x1f
#define  HDMI_LINK_VACT_SPACE4_0                    (HDMI_ADDR_OFFSET + 0x000101D8)      // 2nd Vertical Active Space start line                                   0xff
#define  HDMI_LINK_VACT_SPACE4_1                    (HDMI_ADDR_OFFSET + 0x000101DC)      // 2nd Vertical Active Space end line                                     0x1f
#define  HDMI_LINK_VACT_SPACE5_0                    (HDMI_ADDR_OFFSET + 0x000101E0)      // 3rd Vertical Active Space start line                                   0xff
#define  HDMI_LINK_VACT_SPACE5_1                    (HDMI_ADDR_OFFSET + 0x000101E4)      // 3rd Vertical Active Space end line                                     0x1f
#define  HDMI_LINK_VACT_SPACE6_0                    (HDMI_ADDR_OFFSET + 0x000101E8)      // 3rd Vertical Active Space start line                                   0xff
#define  HDMI_LINK_VACT_SPACE6_1                    (HDMI_ADDR_OFFSET + 0x000101EC)      // 3rd Vertical Active Space end line                                     0x1f

#define  HDMI_LINK_CSC_MUX                          (HDMI_ADDR_OFFSET + 0x000101F0)      // //@added choiyk 2012-12-19 오후 5:18:09 (tbench에는 있는데.. databook에는 왜 없는지..
#define  HDMI_LINK_SYNC_GEN_MUX                     (HDMI_ADDR_OFFSET + 0x000101F4)      // @modified choiyk

#define  HDMI_LINK_GCP_CON                          (HDMI_ADDR_OFFSET + 0x00010200)      // GCP packet control register                                            0x04
#define  HDMI_LINK_GCP_BYTE1                        (HDMI_ADDR_OFFSET + 0x00010210)      // GCP packet body                                                        0x00
#define  HDMI_LINK_GCP_BYTE2                        (HDMI_ADDR_OFFSET + 0x00010214)      // GCP packet body                                                        0x00
#define  HDMI_LINK_GCP_BYTE3                        (HDMI_ADDR_OFFSET + 0x00010218)      // GCP packet body                                                        0x00
#define  HDMI_LINK_ASP_CON                          (HDMI_ADDR_OFFSET + 0x00010300)      // ASP packet control register                                            0x00
#define  HDMI_LINK_ASP_SP_FLAT                      (HDMI_ADDR_OFFSET + 0x00010304)      // ASP packet sp_flat bit control                                         0x00
#define  HDMI_LINK_ASP_CHCFG0                       (HDMI_ADDR_OFFSET + 0x00010310)      // ASP audio channel configuration                                        0x08
#define  HDMI_LINK_ASP_CHCFG1                       (HDMI_ADDR_OFFSET + 0x00010314)      // ASP audio channel configuration                                        0x1A
#define  HDMI_LINK_ASP_CHCFG2                       (HDMI_ADDR_OFFSET + 0x00010318)      // ASP audio channel configuration                                        0x2C
#define  HDMI_LINK_ASP_CHCFG3                       (HDMI_ADDR_OFFSET + 0x0001031C)      // ASP audio channel configuration                                        0x3E
#define  HDMI_LINK_ACR_CON                          (HDMI_ADDR_OFFSET + 0x00010400)      // ACR packet control register                                            0x00
#define  HDMI_LINK_ACR_MCTS0                        (HDMI_ADDR_OFFSET + 0x00010410)      // Measured CTS value                                                     0x01
#define  HDMI_LINK_ACR_MCTS1                        (HDMI_ADDR_OFFSET + 0x00010414)      // Measured CTS value                                                     0x00
#define  HDMI_LINK_ACR_MCTS2                        (HDMI_ADDR_OFFSET + 0x00010418)      // Measured CTS value                                                     0x00
#define  HDMI_LINK_ACR_N0                           (HDMI_ADDR_OFFSET + 0x00010430)      // N value for ACR packet                                                 0xE8
#define  HDMI_LINK_ACR_N1                           (HDMI_ADDR_OFFSET + 0x00010434)      // N value for ACR packet                                                 0x03
#define  HDMI_LINK_ACR_N2                           (HDMI_ADDR_OFFSET + 0x00010438)      // N value for ACR packet                                              0x00
#define  HDMI_LINK_ACP_CON                          (HDMI_ADDR_OFFSET + 0x00010500)      // ACP packet control register                                         0x00
#define  HDMI_LINK_ACP_TYPE                         (HDMI_ADDR_OFFSET + 0x00010514)      // ACP packet header                                                   0x00
#define  HDMI_LINK_ACP_DATAx                        (HDMI_ADDR_OFFSET + 0x00010520)      // ACP packet body                                                     0x00
#define  HDMI_LINK_ISRC_CON                         (HDMI_ADDR_OFFSET + 0x00010600)      // ACR packet control register                                         0x00
#define  HDMI_LINK_ISRC1_HEADER1                    (HDMI_ADDR_OFFSET + 0x00010614)      // ISCR1 packet header                                                 0x00
#define  HDMI_LINK_ISRC1_DATAx                      (HDMI_ADDR_OFFSET + 0x00010620)      // ISRC1 packet body                                                   0x00
#define  HDMI_LINK_ISRC2_DATAx                      (HDMI_ADDR_OFFSET + 0x000106A0)      // ISRC2 packet body                                                   0x00
#define  HDMI_LINK_AVI_CON                          (HDMI_ADDR_OFFSET + 0x00010700)      // AVI packet control register                                         0x00
#define  HDMI_LINK_AVI_HEADER0                      (HDMI_ADDR_OFFSET + 0x00010710)      // AVI packet header                                                   0x00
#define  HDMI_LINK_AVI_HEADER1                      (HDMI_ADDR_OFFSET + 0x00010714)      // AVI packet header                                                   0x00
#define  HDMI_LINK_AVI_HEADER2                      (HDMI_ADDR_OFFSET + 0x00010718)      // AVI packet header                                                   0x00
#define  HDMI_LINK_AVI_CHECK_SUM                    (HDMI_ADDR_OFFSET + 0x0001071C)      // AVI packet checksum                                                 0x00
#define  HDMI_LINK_AVI_BYTEx                        (HDMI_ADDR_OFFSET + 0x00010720)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE00                       (HDMI_ADDR_OFFSET + 0x00010720)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE01                       (HDMI_ADDR_OFFSET + 0x00010724)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE02                       (HDMI_ADDR_OFFSET + 0x00010728)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE03                       (HDMI_ADDR_OFFSET + 0x0001073C)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE04                       (HDMI_ADDR_OFFSET + 0x00010730)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE05                       (HDMI_ADDR_OFFSET + 0x00010734)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE06                       (HDMI_ADDR_OFFSET + 0x00010738)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE07                       (HDMI_ADDR_OFFSET + 0x0001074C)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE08                       (HDMI_ADDR_OFFSET + 0x00010740)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE09                       (HDMI_ADDR_OFFSET + 0x00010744)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE10                       (HDMI_ADDR_OFFSET + 0x00010748)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE11                       (HDMI_ADDR_OFFSET + 0x0001074C)      // AVI packet body                                                     0x00
#define  HDMI_LINK_AVI_BYTE12                       (HDMI_ADDR_OFFSET + 0x00010750)
#define  HDMI_LINK_AUI_CON                          (HDMI_ADDR_OFFSET + 0x00010800)      // AUI packet control register                                         0x00
#define  HDMI_LINK_AUI_HEADER0                      (HDMI_ADDR_OFFSET + 0x00010810)      // AUI packet header                                                   0x00
#define  HDMI_LINK_AUI_HEADER1                      (HDMI_ADDR_OFFSET + 0x00010814)      // AUI packet header                                                   0x00
#define  HDMI_LINK_AUI_HEADER2                      (HDMI_ADDR_OFFSET + 0x00010818)      // AUI packet header                                                   0x00
#define  HDMI_LINK_AUI_CHECK_SUM                    (HDMI_ADDR_OFFSET + 0x0001081C)      // AUI packet checksum                                                 0x00
#define  HDMI_LINK_AUI_BYTEx                        (HDMI_ADDR_OFFSET + 0x00010820)      // AUI packet body                                                     0x00
#define  HDMI_LINK_MPG_CON                          (HDMI_ADDR_OFFSET + 0x00010900)      // ACR packet control register                                         0x00
#define  HDMI_LINK_MPG_CHECK_SUM                    (HDMI_ADDR_OFFSET + 0x0001091C)      // MPG packet checksum                                                 0x00
#define  HDMI_LINK_MPG_DATAx                        (HDMI_ADDR_OFFSET + 0x00010920)      // MPG packet body                                                     0x00
#define  HDMI_LINK_SPD_CON                          (HDMI_ADDR_OFFSET + 0x00010A00)      // SPD packet control register                                         0x00
#define  HDMI_LINK_SPD_HEADER0                      (HDMI_ADDR_OFFSET + 0x00010A10)      // SPD packet header                                                   0x00
#define  HDMI_LINK_SPD_HEADER1                      (HDMI_ADDR_OFFSET + 0x00010A14)      // SPD packet header                                                   0x00
#define  HDMI_LINK_SPD_HEADER2                      (HDMI_ADDR_OFFSET + 0x00010A18)      // SPD packet header                                                   0x00
#define  HDMI_LINK_SPD_DATAx                        (HDMI_ADDR_OFFSET + 0x00010A20)      // SPD packet body                                                     0x00
#define  HDMI_LINK_GAMUT_CON                        (HDMI_ADDR_OFFSET + 0x00010B00)      // GAMUT packet control register                                       0x00
#define  HDMI_LINK_GAMUT_HEADER0                    (HDMI_ADDR_OFFSET + 0x00010B10)      // GAMUT packet header                                                 0x00
#define  HDMI_LINK_GAMUT_HEADER1                    (HDMI_ADDR_OFFSET + 0x00010B14)      // GAMUT packet header                                                 0x00
#define  HDMI_LINK_GAMUT_HEADER2                    (HDMI_ADDR_OFFSET + 0x00010B18)      // GAMUT packet header                                                 0x00
#define  HDMI_LINK_GAMUT_METADATAx                  (HDMI_ADDR_OFFSET + 0x00010B20)      // GAMUT packet body                                                   0x00
#define  HDMI_LINK_VSI_CON                          (HDMI_ADDR_OFFSET + 0x00010C00)      // VSI packet control register                                          0x00
#define  HDMI_LINK_VSI_HEADER0                      (HDMI_ADDR_OFFSET + 0x00010C10)      // VSI packet header                                                    0x00
#define  HDMI_LINK_VSI_HEADER1                      (HDMI_ADDR_OFFSET + 0x00010C14)      // VSI packet header                                                    0x00
#define  HDMI_LINK_VSI_HEADER2                      (HDMI_ADDR_OFFSET + 0x00010C18)      // VSI packet header                                                    0x00
#define  HDMI_LINK_VSI_DATAx                        (HDMI_ADDR_OFFSET + 0x00010C20)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA00                       (HDMI_ADDR_OFFSET + 0x00010C20)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA01                       (HDMI_ADDR_OFFSET + 0x00010C24)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA02                       (HDMI_ADDR_OFFSET + 0x00010C28)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA03                       (HDMI_ADDR_OFFSET + 0x00010C2C)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA04                       (HDMI_ADDR_OFFSET + 0x00010C30)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA05                       (HDMI_ADDR_OFFSET + 0x00010C34)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA06                       (HDMI_ADDR_OFFSET + 0x00010C38)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA07                       (HDMI_ADDR_OFFSET + 0x00010C3C)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA08                       (HDMI_ADDR_OFFSET + 0x00010C40)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA09                       (HDMI_ADDR_OFFSET + 0x00010C44)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA10                       (HDMI_ADDR_OFFSET + 0x00010C48)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA11                       (HDMI_ADDR_OFFSET + 0x00010c4c)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA12                       (HDMI_ADDR_OFFSET + 0x00010C50)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA13                       (HDMI_ADDR_OFFSET + 0x00010C54)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA14                       (HDMI_ADDR_OFFSET + 0x00010C58)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA15                       (HDMI_ADDR_OFFSET + 0x00010C5c)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA16                       (HDMI_ADDR_OFFSET + 0x00010C60)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA17                       (HDMI_ADDR_OFFSET + 0x00010C64)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA18                       (HDMI_ADDR_OFFSET + 0x00010C68)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA19                       (HDMI_ADDR_OFFSET + 0x00010C6c)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA20                       (HDMI_ADDR_OFFSET + 0x00010C70)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA21                       (HDMI_ADDR_OFFSET + 0x00010c74)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA22                       (HDMI_ADDR_OFFSET + 0x00010C78)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA23                       (HDMI_ADDR_OFFSET + 0x00010C7c)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA24                       (HDMI_ADDR_OFFSET + 0x00010C80)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA25                       (HDMI_ADDR_OFFSET + 0x00010C84)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA26                       (HDMI_ADDR_OFFSET + 0x00010C88)      // VSI packet body                                                      0x00
#define  HDMI_LINK_VSI_DATA27                       (HDMI_ADDR_OFFSET + 0x00010C8C)      // VSI packet body                                                      0x00
#define  HDMI_LINK_DC_CONTROL                       (HDMI_ADDR_OFFSET + 0x00010D00)      // Deep Color Control Register                                         0x00
#define  HDMI_LINK_VIDEO_PATTERN_GEN                (HDMI_ADDR_OFFSET + 0x00010D04)      // Video Pattern Generation Register                                    0x00
#define  HDMI_LINK_An_Seed_Sel                      (HDMI_ADDR_OFFSET + 0x00010E48)      // An seed selection register.                                          0xFF
#define  HDMI_LINK_An_Seed_0                        (HDMI_ADDR_OFFSET + 0x00010E58)      // An seed value register                                               0x00
#define  HDMI_LINK_An_Seed_1                        (HDMI_ADDR_OFFSET + 0x00010E5C)      // An seed value register                                               0x00
#define  HDMI_LINK_An_Seed_2                        (HDMI_ADDR_OFFSET + 0x00010E60)      // An seed value register                                               0x00
#define  HDMI_LINK_An_Seed_3                        (HDMI_ADDR_OFFSET + 0x00010E64)      // An seed value register                                               0x00
#define  HDMI_LINK_HDCP_SHA1_x                      (HDMI_ADDR_OFFSET + 0x00017000)      // SHA-1 value from repeater                                           0x00

#define  HDMI_LINK_HDCP_SHA1_0_0  HDMI_LINK_HDCP_SHA1_x   + 0x00
#define  HDMI_LINK_HDCP_SHA1_0_1  HDMI_LINK_HDCP_SHA1_0_0 + 0x04
#define  HDMI_LINK_HDCP_SHA1_0_2  HDMI_LINK_HDCP_SHA1_0_0 + 0x08
#define  HDMI_LINK_HDCP_SHA1_0_3  HDMI_LINK_HDCP_SHA1_0_0 + 0x0C
#define  HDMI_LINK_HDCP_SHA1_1_0  HDMI_LINK_HDCP_SHA1_x   + 0x10
#define  HDMI_LINK_HDCP_SHA1_1_1  HDMI_LINK_HDCP_SHA1_1_0 + 0x04
#define  HDMI_LINK_HDCP_SHA1_1_2  HDMI_LINK_HDCP_SHA1_1_0 + 0x08
#define  HDMI_LINK_HDCP_SHA1_1_3  HDMI_LINK_HDCP_SHA1_1_0 + 0x0C
#define  HDMI_LINK_HDCP_SHA1_2_0  HDMI_LINK_HDCP_SHA1_x   + 0x20
#define  HDMI_LINK_HDCP_SHA1_2_1  HDMI_LINK_HDCP_SHA1_2_0 + 0x04
#define  HDMI_LINK_HDCP_SHA1_2_2  HDMI_LINK_HDCP_SHA1_2_0 + 0x08
#define  HDMI_LINK_HDCP_SHA1_2_3  HDMI_LINK_HDCP_SHA1_2_0 + 0x0C
#define  HDMI_LINK_HDCP_SHA1_3_0  HDMI_LINK_HDCP_SHA1_x   + 0x30
#define  HDMI_LINK_HDCP_SHA1_3_1  HDMI_LINK_HDCP_SHA1_3_0 + 0x04
#define  HDMI_LINK_HDCP_SHA1_3_2  HDMI_LINK_HDCP_SHA1_3_0 + 0x08
#define  HDMI_LINK_HDCP_SHA1_3_3  HDMI_LINK_HDCP_SHA1_3_0 + 0x0C
#define  HDMI_LINK_HDCP_SHA1_4_0  HDMI_LINK_HDCP_SHA1_x   + 0x40
#define  HDMI_LINK_HDCP_SHA1_4_1  HDMI_LINK_HDCP_SHA1_4_0 + 0x04
#define  HDMI_LINK_HDCP_SHA1_4_2  HDMI_LINK_HDCP_SHA1_4_0 + 0x08
#define  HDMI_LINK_HDCP_SHA1_4_3  HDMI_LINK_HDCP_SHA1_4_0 + 0x0C

#define  HDMI_LINK_HDCP_KSV_LIST_x                  (HDMI_ADDR_OFFSET + 0x00017050)      // KSV list from repeater                                              0x00

#define  HDMI_LINK_HDCP_KSV_0_0  HDMI_LINK_HDCP_KSV_LIST_x + 0x00
#define  HDMI_LINK_HDCP_KSV_0_1  HDMI_LINK_HDCP_KSV_LIST_x + 0x04
#define  HDMI_LINK_HDCP_KSV_0_2  HDMI_LINK_HDCP_KSV_LIST_x + 0x08
#define  HDMI_LINK_HDCP_KSV_0_3  HDMI_LINK_HDCP_KSV_LIST_x + 0x0C
#define  HDMI_LINK_HDCP_KSV_1_0  HDMI_LINK_HDCP_KSV_LIST_x + 0x10
#define  HDMI_LINK_HDCP_KSV_1_1  HDMI_LINK_HDCP_KSV_LIST_x + 0x14

#define  HDMI_LINK_HDCP_KSV_LIST_0_0    HDMI_LINK_HDCP_KSV_LIST_x + 0x00
#define  HDMI_LINK_HDCP_KSV_LIST_0_1    HDMI_LINK_HDCP_KSV_LIST_x + 0x04
#define  HDMI_LINK_HDCP_KSV_LIST_0_2    HDMI_LINK_HDCP_KSV_LIST_x + 0x08
#define  HDMI_LINK_HDCP_KSV_LIST_0_3    HDMI_LINK_HDCP_KSV_LIST_x + 0x0C
#define  HDMI_LINK_HDCP_KSV_LIST_1_0    HDMI_LINK_HDCP_KSV_LIST_x + 0x10
#define  HDMI_LINK_HDCP_KSV_LIST_1_1    HDMI_LINK_HDCP_KSV_LIST_x + 0x14

#define  HDMI_LINK_HDCP_KSV_LIST_CON                (HDMI_ADDR_OFFSET + 0x00017064)      // KSV list control                                                    0x00
#define  HDMI_LINK_HDCP_SHA_RESULT                  (HDMI_ADDR_OFFSET + 0x00017070)      // SHA-1 checking result register                                      0x00
#define  HDMI_LINK_HDCP_CTRL1                       (HDMI_ADDR_OFFSET + 0x00017080)      // HDCP control register1                                              0x00
#define  HDMI_LINK_HDCP_CTRL2                       (HDMI_ADDR_OFFSET + 0x00017084)      // HDCP control register2                                              0x00
#define  HDMI_LINK_HDCP_CHECK_RESULT                (HDMI_ADDR_OFFSET + 0x00017090)      // Ri and Pj value checking result                                     0x00
#define  HDMI_LINK_HDCP_BKSV_x                      (HDMI_ADDR_OFFSET + 0x000170A0)      // KSV of Rx                                                           0x00

#define  HDMI_LINK_HDCP_BKSV0_0                      (HDMI_ADDR_OFFSET + 0x000170A0)      // KSV of Rx                                                           0x00
#define  HDMI_LINK_HDCP_BKSV0_1                      (HDMI_ADDR_OFFSET + 0x000170A4)      // KSV of Rx                                                           0x00
#define  HDMI_LINK_HDCP_BKSV0_2                      (HDMI_ADDR_OFFSET + 0x000170A8)      // KSV of Rx                                                           0x00
#define  HDMI_LINK_HDCP_BKSV0_3                      (HDMI_ADDR_OFFSET + 0x000170AC)      // KSV of Rx                                                           0x00
#define  HDMI_LINK_HDCP_BKSV1                        (HDMI_ADDR_OFFSET + 0x000170B0)      // KSV of Rx                                                           0x00

#define  HDMI_LINK_HDCP_AKSV_x                      (HDMI_ADDR_OFFSET + 0x000170C0)      // KSV of Tx                                                           0x00
#define  HDMI_LINK_HDCP_An_x                        (HDMI_ADDR_OFFSET + 0x000170E0)      // An value                                                             0x00
#define  HDMI_LINK_HDCP_BCAPS                       (HDMI_ADDR_OFFSET + 0x00017100)      // BCAPS from Rx                                                       0x00
#define  HDMI_LINK_HDCP_BSTATUS_0                   (HDMI_ADDR_OFFSET + 0x00017110)      // BSTATUS from Rx                                                     0x00
#define  HDMI_LINK_HDCP_BSTATUS_1                   (HDMI_ADDR_OFFSET + 0x00017114)      // BSTATUS from Rx                                                     0x00
#define  HDMI_LINK_HDCP_Ri_0                        (HDMI_ADDR_OFFSET + 0x00017140)      // Ri value of Tx                                                      0x00
#define  HDMI_LINK_HDCP_Ri_1                        (HDMI_ADDR_OFFSET + 0x00017144)      // Ri value of Tx                                                      0x00

#define  HDMI_LINK_HDCP_OFFSET_TX_0	                    (HDMI_ADDR_OFFSET + 0x00017160)
#define  HDMI_LINK_HDCP_OFFSET_TX_1	                    (HDMI_ADDR_OFFSET + 0x00017164)
#define  HDMI_LINK_HDCP_OFFSET_TX_2	                    (HDMI_ADDR_OFFSET + 0x00017168)
#define  HDMI_LINK_HDCP_OFFSET_TX_3	                    (HDMI_ADDR_OFFSET + 0x0001716C)
#define  HDMI_LINK_HDCP_CYCLE_AA	                        (HDMI_ADDR_OFFSET + 0x00017170)

#define  HDMI_LINK_HDCP_I2C_INT                     (HDMI_ADDR_OFFSET + 0x00017180)      // I2C interrupt flag                                                  0x00
#define  HDMI_LINK_HDCP_AN_INT                      (HDMI_ADDR_OFFSET + 0x00017190)      // An value ready interrupt flag                                        0x00
#define  HDMI_LINK_HDCP_WATCHDOG_INT                (HDMI_ADDR_OFFSET + 0x000171A0)      // Wachdog interrupt flag                                               0x00
#define  HDMI_LINK_HDCP_Ri_INT                      (HDMI_ADDR_OFFSET + 0x000171B0)      // Ri value update interrupt flag                                      0x00
#define  HDMI_LINK_HDCP_Ri_Compare_0                (HDMI_ADDR_OFFSET + 0x000171D0)      // HDCP Ri Interrupt Frame number index register 0                     0x80
#define  HDMI_LINK_HDCP_Ri_Compare_1                (HDMI_ADDR_OFFSET + 0x000171D4)      // HDCP Ri Interrupt Frame number index register 1                     0x7F

#define  HDMI_LINK_HDCP_RI_INT                      (HDMI_ADDR_OFFSET + 0x000171B0)      // Ri value update interrupt flag                                      0x00
#define  HDMI_LINK_HDCP_RI_Compare_0                (HDMI_ADDR_OFFSET + 0x000171D0)      // HDCP Ri Interrupt Frame number index register 0                     0x80
#define  HDMI_LINK_HDCP_RI_Compare_1                (HDMI_ADDR_OFFSET + 0x000171D4)      // HDCP Ri Interrupt Frame number index register 1                     0x7F

#define  HDMI_LINK_HDCP_Frame_Count                 (HDMI_ADDR_OFFSET + 0x000171E0)      // Current value of the frame count index in the hardware              0x00
#define  HDMI_LINK_RGB_ROUND_EN                     (HDMI_ADDR_OFFSET + 0x0001D500)      // round enable for 8/10 bit R/G/B in video_receiver                   0x00
#define  HDMI_LINK_VACT_SPACE_R_0                   (HDMI_ADDR_OFFSET + 0x0001D504)      // vertical active space R                                              0x00
#define  HDMI_LINK_VACT_SPACE_R_1                   (HDMI_ADDR_OFFSET + 0x0001D508)      // vertical active space R                                              0x00
#define  HDMI_LINK_VACT_SPACE_G_0                   (HDMI_ADDR_OFFSET + 0x0001D50C)      // vertical active space G                                              0x00
#define  HDMI_LINK_VACT_SPACE_G_1                   (HDMI_ADDR_OFFSET + 0x0001D510)      // vertical active space G                                              0x00
#define  HDMI_LINK_VACT_SPACE_B_0                   (HDMI_ADDR_OFFSET + 0x0001D514)      // vertical active space B                                              0x00
#define  HDMI_LINK_VACT_SPACE_B_1                   (HDMI_ADDR_OFFSET + 0x0001D518)      // vertical active space B                                              0x00
#define  HDMI_LINK_BLUE_SCREEN_R_0                  (HDMI_ADDR_OFFSET + 0x0001D520)      // R Pixel values for blue screen [3:0]                                0x00
#define  HDMI_LINK_BLUE_SCREEN_R_1                  (HDMI_ADDR_OFFSET + 0x0001D524)      // R Pixel values for blue screen [11:4]                               0x00
#define  HDMI_LINK_BLUE_SCREEN_G_0                  (HDMI_ADDR_OFFSET + 0x0001D528)      // G Pixel values for blue screen [3:0]                                0x00
#define  HDMI_LINK_BLUE_SCREEN_G_1                  (HDMI_ADDR_OFFSET + 0x0001D52C)      // G Pixel values for blue screen [11:4]                               0x00
#define  HDMI_LINK_BLUE_SCREEN_B_0                  (HDMI_ADDR_OFFSET + 0x0001D530)      // B Pixel values for blue screen [3:0]                                0x00
#define  HDMI_LINK_BLUE_SCREEN_B_1                  (HDMI_ADDR_OFFSET + 0x0001D534)      // B Pixel values for blue screen [11:4]                               0x00
#define  HDMI_LINK_AES_START           (HDMI_ADDR_OFFSET +  0x00020000)    //AES_START        0x00
#define  HDMI_LINK_AES_DATA_SIZE_L     (HDMI_ADDR_OFFSET +  0x00020020)    //AES_DATA_SIZE_L  0x20
#define  HDMI_LINK_AES_DATA_SIZE_H     (HDMI_ADDR_OFFSET +  0x00020024)    //AES_DATA_SIZE_H  0x01
#define  HDMI_LINK_AES_DATA            (HDMI_ADDR_OFFSET +  0x00020040)    //AES_DATA         0x00
#define  HDMI_LINK_SPDIFIN_CLK_CTRL              (HDMI_ADDR_OFFSET + 0x00030000)   //SPDIFIN Clock Control Register                                       0x02
#define  HDMI_LINK_SPDIFIN_OP_CTRL               (HDMI_ADDR_OFFSET + 0x00030004)   //SPDIFIN Operation Control Register 1                                 0x00
#define  HDMI_LINK_SPDIFIN_IRQ_MASK              (HDMI_ADDR_OFFSET + 0x00030008)   //SPDIFIN Interrupt Request Mask Register                              0x00
#define  HDMI_LINK_SPDIFIN_IRQ_STATUS            (HDMI_ADDR_OFFSET + 0x0003000C)   //SPDIFIN Interrupt Request Status Register                            0x00
#define  HDMI_LINK_SPDIFIN_CONFIG_1              (HDMI_ADDR_OFFSET + 0x00030010)   //SPDIFIN Configuration Register 1                                     0x02
#define  HDMI_LINK_SPDIFIN_CONFIG_2              (HDMI_ADDR_OFFSET + 0x00030014)   //SPDIFIN Configuration Register 2                                     0x00
#define  HDMI_LINK_SPDIFIN_USER_VALUE_1          (HDMI_ADDR_OFFSET + 0x00030020)   //SPDIFIN User Value Register 1                                        0x00
#define  HDMI_LINK_SPDIFIN_USER_VALUE_2          (HDMI_ADDR_OFFSET + 0x00030024)   //SPDIFIN User Value Register 2                                        0x00
#define  HDMI_LINK_SPDIFIN_USER_VALUE_3          (HDMI_ADDR_OFFSET + 0x00030028)   //SPDIFIN User Value Register 3                                        0x00
#define  HDMI_LINK_SPDIFIN_USER_VALUE_4          (HDMI_ADDR_OFFSET + 0x0003002C)   //SPDIFIN User Value Register 4                                        0x00
#define  HDMI_LINK_SPDIFIN_CH_STATUS_0_1         (HDMI_ADDR_OFFSET + 0x00030030)   //SPDIFIN Channel Status Register 0-1                                  0x00
#define  HDMI_LINK_SPDIFIN_CH_STATUS_0_2         (HDMI_ADDR_OFFSET + 0x00030034)   //SPDIFIN Channel Status Register 0-2                                  0x00
#define  HDMI_LINK_SPDIFIN_CH_STATUS_0_3         (HDMI_ADDR_OFFSET + 0x00030038)   //SPDIFIN Channel Status Register 0-3                                  0x00
#define  HDMI_LINK_SPDIFIN_CH_STATUS_0_4         (HDMI_ADDR_OFFSET + 0x0003003C)   //SPDIFIN Channel Status Register 0-4                                  0x00
#define  HDMI_LINK_SPDIFIN_CH_STATUS_1           (HDMI_ADDR_OFFSET + 0x00030040)   //SPDIFIN Channel Status Register 1                                    0x00
#define  HDMI_LINK_SPDIFIN_FRAME_PERIOD_1        (HDMI_ADDR_OFFSET + 0x00030048)   //SPDIFIN Frame Period Register 1                                      0x00
#define  HDMI_LINK_SPDIFIN_FRAME_PERIOD_2        (HDMI_ADDR_OFFSET + 0x0003004C)   //SPDIFIN Frame Period Register 2                                      0x00
#define  HDMI_LINK_SPDIFIN_Pc_INFO_1             (HDMI_ADDR_OFFSET + 0x00030050)   //SPDIFIN Pc Info Register 1                                           0x00
#define  HDMI_LINK_SPDIFIN_Pc_INFO_2             (HDMI_ADDR_OFFSET + 0x00030054)   //SPDIFIN Pc Info Register 2                                           0x00
#define  HDMI_LINK_SPDIFIN_Pd_INFO_1             (HDMI_ADDR_OFFSET + 0x00030058)   //SPDIFIN Pd Info Register 1                                           0x00
#define  HDMI_LINK_SPDIFIN_Pd_INFO_2             (HDMI_ADDR_OFFSET + 0x0003005C)   //SPDIFIN Pd Info Register 2                                           0x00
#define  HDMI_LINK_SPDIFIN_DATA_BUF_0_1          (HDMI_ADDR_OFFSET + 0x00030060)   //SPDIFIN Data Buffer Register 0_1                                     0x00
#define  HDMI_LINK_SPDIFIN_DATA_BUF_0_2          (HDMI_ADDR_OFFSET + 0x00030064)   //SPDIFIN Data Buffer Register 0_2                                     0x00
#define  HDMI_LINK_SPDIFIN_DATA_BUF_0_3          (HDMI_ADDR_OFFSET + 0x00030068)   //SPDIFIN Data Buffer Register 0_3                                     0x00
#define  HDMI_LINK_SPDIFIN_USER_BUF_0            (HDMI_ADDR_OFFSET + 0x0003006C)   //SPDIFIN User Buffer Register 0                                       0x00
#define  HDMI_LINK_SPDIFIN_DATA_BUF_1_1          (HDMI_ADDR_OFFSET + 0x00030070)   //SPDIFIN Data Buffer Register 1_1                                     0x00
#define  HDMI_LINK_SPDIFIN_DATA_BUF_1_2          (HDMI_ADDR_OFFSET + 0x00030074)   //SPDIFIN Data Buffer Register 1_2                                     0x00
#define  HDMI_LINK_SPDIFIN_DATA_BUF_1_3          (HDMI_ADDR_OFFSET + 0x00030078)   //SPDIFIN Data Buffer Register 1_3                                     0x00
#define  HDMI_LINK_SPDIFIN_USER_BUF_1            (HDMI_ADDR_OFFSET + 0x0003007C)   //SPDIFIN User Buffer Register 1                                       0x00
#define  HDMI_LINK_I2S_CLK_CON         (HDMI_ADDR_OFFSET + 0x00040000)    //I2S Clock Enable Register                                                       0x00
#define  HDMI_LINK_I2S_CON_1           (HDMI_ADDR_OFFSET + 0x00040004)    //I2S Control Register 1                                                          0x00
#define  HDMI_LINK_I2S_CON_2           (HDMI_ADDR_OFFSET + 0x00040008)    //I2S Control Register 2                                                          0x16
#define  HDMI_LINK_I2S_PIN_SEL_0       (HDMI_ADDR_OFFSET + 0x0004000C)    //I2S Input Pin Selection Register 0                                              0x77
#define  HDMI_LINK_I2S_PIN_SEL_1       (HDMI_ADDR_OFFSET + 0x00040010)    //I2S Input Pin Selection Register 1                                              0x77
#define  HDMI_LINK_I2S_PIN_SEL_2       (HDMI_ADDR_OFFSET + 0x00040014)    //I2S Input Pin Selection Register 2                                              0x77
#define  HDMI_LINK_I2S_PIN_SEL_3       (HDMI_ADDR_OFFSET + 0x00040018)    //I2S Input Pin Selection Register 3                                              0x07
#define  HDMI_LINK_I2S_DSD_CON         (HDMI_ADDR_OFFSET + 0x0004001C)    //I2S DSD Control Register                                                        0x02
#define  HDMI_LINK_I2S_MUX_CON         (HDMI_ADDR_OFFSET + 0x00040020)    //I2S In/Mux Control Register                                                     0x60
#define  HDMI_LINK_I2S_CH_ST_CON       (HDMI_ADDR_OFFSET + 0x00040024)    //I2S Channel Status Control Register                                             0x00
#define  HDMI_LINK_I2S_CH_ST_0         (HDMI_ADDR_OFFSET + 0x00040028)    //I2S Channel Status Block 0                                                      0x00
#define  HDMI_LINK_I2S_CH_ST_1         (HDMI_ADDR_OFFSET + 0x0004002C)    //I2S Channel Status Block 1                                                      0x00
#define  HDMI_LINK_I2S_CH_ST_2         (HDMI_ADDR_OFFSET + 0x00040030)    //I2S Channel Status Block 2                                                      0x00
#define  HDMI_LINK_I2S_CH_ST_3         (HDMI_ADDR_OFFSET + 0x00040034)    //I2S Channel Status Block 3                                                      0x00
#define  HDMI_LINK_I2S_CH_ST_4         (HDMI_ADDR_OFFSET + 0x00040038)    //I2S Channel Status Block 4                                                      0x00
#define  HDMI_LINK_I2S_CH_ST_SH_0      (HDMI_ADDR_OFFSET + 0x0004003C)    //I2S Channel Status Block Shadow Register 0                                      0x00
#define  HDMI_LINK_I2S_CH_ST_SH_1      (HDMI_ADDR_OFFSET + 0x00040040)    //I2S Channel Status Block Shadow Register 1                                      0x00
#define  HDMI_LINK_I2S_CH_ST_SH_2      (HDMI_ADDR_OFFSET + 0x00040044)    //I2S Channel Status Block Shadow Register 2                                      0x00
#define  HDMI_LINK_I2S_CH_ST_SH_3      (HDMI_ADDR_OFFSET + 0x00040048)    //I2S Channel Status Block Shadow Register 3                                      0x00
#define  HDMI_LINK_I2S_CH_ST_SH_4      (HDMI_ADDR_OFFSET + 0x0004004C)    //I2S Channel Status Block Shadow Register 4                                      0x00
#define  HDMI_LINK_I2S_VD_DATA         (HDMI_ADDR_OFFSET + 0x00040050)    //I2S Audio Sample Validity Register                                              0x00
#define  HDMI_LINK_I2S_MUX_CH          (HDMI_ADDR_OFFSET + 0x00040054)    //I2S Channel Enable Register                                                     0x03
#define  HDMI_LINK_I2S_MUX_CUV         (HDMI_ADDR_OFFSET + 0x00040058)    //I2S CUV Enable Register                                                         0x03
#define  HDMI_LINK_I2S_CH0_L_0         (HDMI_ADDR_OFFSET + 0x00040064)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH0_L_1         (HDMI_ADDR_OFFSET + 0x00040068)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH0_L_2         (HDMI_ADDR_OFFSET + 0x0004006C)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH0_R_0         (HDMI_ADDR_OFFSET + 0x00040074)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH0_R_1         (HDMI_ADDR_OFFSET + 0x00040078)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH0_R_2         (HDMI_ADDR_OFFSET + 0x0004007C)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH0_R_3         (HDMI_ADDR_OFFSET + 0x00040080)    //I2S PCM Output Data Register                                                    0x00
#define  HDMI_LINK_I2S_CH1_L_0         (HDMI_ADDR_OFFSET + 0x00040084)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_L_1         (HDMI_ADDR_OFFSET + 0x00040088)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_L_2         (HDMI_ADDR_OFFSET + 0x0004008C)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_L_3         (HDMI_ADDR_OFFSET + 0x00040090)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_R_0         (HDMI_ADDR_OFFSET + 0x00040094)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_R_1         (HDMI_ADDR_OFFSET + 0x00040098)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_R_2         (HDMI_ADDR_OFFSET + 0x0004009C)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH1_R_3         (HDMI_ADDR_OFFSET + 0x000400A0)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_L_0         (HDMI_ADDR_OFFSET + 0x000400A4)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_L_1         (HDMI_ADDR_OFFSET + 0x000400A8)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_L_2         (HDMI_ADDR_OFFSET + 0x000400AC)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_L_3         (HDMI_ADDR_OFFSET + 0x000400B0)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_R_0         (HDMI_ADDR_OFFSET + 0x000400B4)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_R_1         (HDMI_ADDR_OFFSET + 0x000400B8)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH2_R_2         (HDMI_ADDR_OFFSET + 0x000400BC)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_Ch2_R_3         (HDMI_ADDR_OFFSET + 0x000400C0)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH3_L_0         (HDMI_ADDR_OFFSET + 0x000400C4)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH3_L_1         (HDMI_ADDR_OFFSET + 0x000400C8)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH3_L_2         (HDMI_ADDR_OFFSET + 0x000400CC)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH3_R_0         (HDMI_ADDR_OFFSET + 0x000400D0)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH3_R_1         (HDMI_ADDR_OFFSET + 0x000400D4)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CH3_R_2         (HDMI_ADDR_OFFSET + 0x000400D8)    //I2S PCM Output Data Register                                                  0x00
#define  HDMI_LINK_I2S_CUV_L_R         (HDMI_ADDR_OFFSET + 0x000400DC)    //I2S CUV Output Data Register                                                  0x00




// cec는   1MB ~ (1MB+2KB)
#define  HDMI_CEC_TX_STATUS_0         (OTHER_ADDR_OFFSET + 0x00000000)    //CEC Tx status register 0.                                                       0x00
#define  HDMI_CEC_TX_STATUS_1         (OTHER_ADDR_OFFSET + 0x00000004)    //CEC Tx status register 1. Number of blocks transferred.                         0x00
#define  HDMI_CEC_RX_STATUS_0         (OTHER_ADDR_OFFSET + 0x00000008)    //CEC Rx status register 0.                                                       0x00
#define  HDMI_CEC_RX_STATUS_1         (OTHER_ADDR_OFFSET + 0x0000000C)    //CEC Rx status register 1. Number of blocks received.                            0x00
#define  HDMI_CEC_INTR_MASK           (OTHER_ADDR_OFFSET + 0x00000010)    //CEC interrupt mask register                                                     0x00
#define  HDMI_CEC_INTR_CLEAR          (OTHER_ADDR_OFFSET + 0x00000014)    //CEC interrupt clear register                                                    0x00
#define  HDMI_CEC_LOGIC_ADDR          (OTHER_ADDR_OFFSET + 0x00000020)    // HDMI Tx logical address register                                               0x0F
#define  HDMI_CEC_DIVISOR_0           (OTHER_ADDR_OFFSET + 0x00000030)    //Clock divisor for 0.05ms period count ([7:0] of 32-bit)                         0x00
#define  HDMI_CEC_DIVISOR_1           (OTHER_ADDR_OFFSET + 0x00000034)    //Clock divisor for 0.05ms period count ([15:8] of 32-bit)                        0x00
#define  HDMI_CEC_DIVISOR_2           (OTHER_ADDR_OFFSET + 0x00000038)    //Clock divisor for 0.05ms period count ([23:16] of 32-bit)                       0x00
#define  HDMI_CEC_DIVISOR_3           (OTHER_ADDR_OFFSET + 0x0000003C)    //Clock divisor for 0.05ms period count ([31:24] of 32-bit)                       0x00
#define  HDMI_CEC_TX_CTRL             (OTHER_ADDR_OFFSET + 0x00000040)    //CEC Tx control register                                                         0x10
#define  HDMI_CEC_TX_BYTE_NUM         (OTHER_ADDR_OFFSET + 0x00000044)    // Number of blocks in a message to be transferred                                0x00
#define  HDMI_CEC_TX_STATUS_2         (OTHER_ADDR_OFFSET + 0x00000060)    //CEC Tx status register 2                                                        0x00
#define  HDMI_CEC_TX_STATUS_3         (OTHER_ADDR_OFFSET + 0x00000064)    //CEC Tx status register 3                                                        0x00
#define  HDMI_CEC_TX_BUFFER_x         (OTHER_ADDR_OFFSET + 0x00000080)    // Byte #0 ~ #15 of CEC message to be transferred. (#0 is transferred 1st )       0x00
#define  HDMI_CEC_TX_BUFFER00         (OTHER_ADDR_OFFSET + 0x00000080)    // Byte #0 ~ #15 of CEC message to be transferred. (#0 is transferred 1st )       0x00
#define  HDMI_CEC_RX_CTRL             (OTHER_ADDR_OFFSET + 0x000000C0)    //CEC Rx control register                                                         0x00
#define  HDMI_CEC_RX_STATUS_2         (OTHER_ADDR_OFFSET + 0x000000E0)    //CEC Rx status register 2                                                        0x00
#define  HDMI_CEC_RX_STATUS_3         (OTHER_ADDR_OFFSET + 0x000000E4)    //CEC Rx status register 3                                                        0x00
#define  HDMI_CEC_RX_BUFFER_x         (OTHER_ADDR_OFFSET + 0x00000100)    // Byte #0 ~ #15 of CEC message received (#0 is received 1st )                    0x00
#define  HDMI_CEC_FILTER_CTRL         (OTHER_ADDR_OFFSET + 0x00000180)    //CEC Filter control register                                                     0x81
#define  HDMI_CEC_FILTER_TH           (OTHER_ADDR_OFFSET + 0x00000184)    //CEC Filter Threshold register                                                   0x03


// HDMI는 각 비트별로.. 8비트까지 사용한다.
// PHY는 (1MB+2KB ~ 1MB+4KB)
#define HDMI_PHY_OFFSET 0x400 // [19:10] == 0x01

#define  HDMI_PHY_REG00 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000000)  // 0x01
#define  HDMI_PHY_Reg04 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000004)  //  1  0  0  1  0  0  0  1      Reserved     Reserved  CLK_SEL[1]  CLK_SEL[0]   Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg08 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000008)  //  0  0  0  1  1  1  1  1      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg0C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000000C)  //  0  0  0  1  0  0  0  0      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg10 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000010)  //  0  1  0  0  0  0  0  0      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg14 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000014)  //  0  1  0  1  1  0  1  1      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg18 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000018)  //  1  1  1  0  1  1  1  1      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg1C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000001C)  //  0  0  0  0  1  0  0  0      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg20 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000020)  //  1  0  0  0  0  0  0  1      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg24 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000024)  //  0  0  1  0  1  0  0  0      REF_CKO_SEL    Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg28 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000028)  //  1  0  1  1  1  0  0  1      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg2C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000002C)  //  1  1  0  1  1  0  0  0      Reserved     Reserved    Reserved   Reserved    Reserved    Reserved     Reserved    Reserved
#define  HDMI_PHY_Reg30 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000030)  //  0  1  0  0  0  1  0  1      Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg34 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000034)  //  1  0  1  0  0  0  0  0      Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg38 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000038)  //  1  0  1  0  1  1  0  0      Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg3C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000003C)  //  1  0  0  1  0  0  0  0      TX_AMP_LVL[0]    Reserved     TX_RES[1]    TX_RES[0]     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg40 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000040)  //  0  0  0  0  1  0  0  0      TX_EMP_LVL[3] TX_EMP_LVL[2] TX_EMP_LVL[1] TX_EMP_LVL[0] TX_AMP_LVL[4] TX_AMP_LVL[3] TX_AMP_LVL[2] TX_AMP_LVL[1]
#define  HDMI_PHY_Reg44 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000044)  //  1  0  0  0  0  0  0  0        Reserved      Reserved     Reserved      Reserved     RX_LPEN[1]    RX_LPEN[0]     TX_LPEN[1]    TX_LPEN[0]
#define  HDMI_PHY_Reg48 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000048)  //  0  0  0  0  0  0  0  1        Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg4C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000004C)  //  1  0  0  0  0  1  0  0        Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg50 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000050)  //  0  0  0  0  0  1  0  1        Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg54 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000054)  //  0  0  1  0  0  0  0  1        Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg58 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000058)  //  0  0  1  0  0  1  0  0        Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg5C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000005C)  //  1  0  0  0  0  1  1  0      TX_CLK_LVL[4] TX_CLK_LVL[3] TX_CLK_LVL[2] TX_CLK_LVL[1] TX_CLK_LVL[0] Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg60 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000060)  //  0  1  0  1  0  1  0  0        Reserved      Reserved     Reserved      Reserved     Reserved      Reserved       Reserved      Reserved
#define  HDMI_PHY_Reg64 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000064)  //  1  0  1  0  0  1  1  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg68 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000068)  //  0  0  1  0  0  1  0  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg6C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000006C)  //  0  0  0  0  0  0  0  1       Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg70 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000070)  //  0  0  0  0  0  0  0  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg74 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000074)  //  0  0  0  0  0  0  0  0    APB_PDEN        PLL PD   TX CLK SER PD TX CLK DRV PD Reserved  TX DRV PD   TX_SER_PD    TX_CLK_PD
#define  HDMI_PHY_Reg78 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000078)  //  0  0  0  0  0  0  0  1      TESTEN       TEST[6]     TEST[5]     TEST[4]     TEST[3]     TEST[2]      TEST[1]     TEST[0]
#define  HDMI_PHY_Reg7C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000007C)  //  1  0  0  0  0  0  0  0    MODE SET DONE  Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg80 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000080)  //  0  0  0  0  0  0  0  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg84 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000084)  //  0  0  0  0  0  0  0  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg88 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000088)  //  0  0  0  0  0  0  0  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg8C (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x0000008C)  //  0  0  0  1  0  0  0  0       Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved
#define  HDMI_PHY_Reg90 (OTHER_ADDR_OFFSET + HDMI_PHY_OFFSET + 0x00000090)  //  0  0  0  0  0  0  0  0      Reserved    Reserved    Reserved    Reserved    Reserved     Reserved    Reserved     Reserved











































//------------------------------------------------------------------------------
///	@name	HDMI Interface
//------------------------------------------------------------------------------
//@{
// 위에 정의된 Define을 넣어주도록 한다.
CBOOL NX_HDMI_InitRegTest( U32 ModuleIndex );
CBOOL NX_HDMI_PHY_InitRegTest( U32 ModuleIndex ); // PHY는 먼저 확인해봐야 한다.
U32  NX_HDMI_GetReg( U32 ModuleIndex, U32 Offset );
void NX_HDMI_SetReg( U32 ModuleIndex, U32 Offset, U32 regvalue );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_HDMI_Initialize( void );
U32   NX_HDMI_GetNumberOfModule( void );

U32   NX_HDMI_GetSizeOfRegisterSet( void );
void  NX_HDMI_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*  NX_HDMI_GetBaseAddress( U32 ModuleIndex );
U32   NX_HDMI_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_HDMI_OpenModule( U32 ModuleIndex );
CBOOL NX_HDMI_CloseModule( U32 ModuleIndex );
CBOOL NX_HDMI_CheckBusy( U32 ModuleIndex );
//@}


//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
enum HDMI_RESET{
	i_nRST       = 0,
	i_nRST_VIDEO = 1,
	i_nRST_SPDIF = 2,
	i_nRST_TMDS  = 3,
	i_nRST_PHY   = 4,
};
U32 NX_HDMI_GetResetNumber ( U32 ModuleIndex, U32 ChannelNumber );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
U32   NX_HDMI_GetInterruptNumber ( U32 ModuleIndex );
void  NX_HDMI_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL NX_HDMI_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL NX_HDMI_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_HDMI_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void  NX_HDMI_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL NX_HDMI_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL NX_HDMI_GetInterruptPendingAll( U32 ModuleIndex );
void  NX_HDMI_ClearInterruptPendingAll( U32 ModuleIndex );
S32   NX_HDMI_GetInterruptPendingNumber( U32 ModuleIndex );
//@}


#ifdef	__cplusplus
}
#endif


#endif // __NX_HDMI_H__
