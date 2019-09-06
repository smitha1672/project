//***************************************************************************
//!file     si_regs_ipv953x.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_IPV_H__
#define __SI_9535REGS_IPV_H__
#include "si_common.h"
#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 3  (0xFA)
//------------------------------------------------------------------------------

#define REG_IPV_WIN_ENABLE      (PP_PAGE_IPV | 0x03)

#if ( configSII_DEV_9535 == 0 )
#define MSK_ENABLE_MASK                 0x03
#else
#define MSK_ENABLE_MASK                 0x07
#endif

#define REG_IPV_MODE            (PP_PAGE_IPV | 0x04)

#define REG_IPV_1WIN_16x9_WIDTH (PP_PAGE_IPV | 0x05)
#define REG_IPV_2WIN_16x9_WIDTH (PP_PAGE_IPV | 0x06)
#define REG_IPV_3WIN_16x9_WIDTH (PP_PAGE_IPV | 0x07)

#define REG_IPV_1WIN_4x3_WIDTH  (PP_PAGE_IPV | 0x0A)
#define REG_IPV_2WIN_4x3_WIDTH  (PP_PAGE_IPV | 0x0B)
#define REG_IPV_3WIN_4x3_WIDTH  (PP_PAGE_IPV | 0x0C)

#define REG_IPV_1WIN_HEIGHT     (PP_PAGE_IPV | 0x0F)
#define REG_IPV_2WIN_HEIGHT     (PP_PAGE_IPV | 0x10)
#define REG_IPV_3WIN_HEIGHT     (PP_PAGE_IPV | 0x11)

// Sub-frame X locations (14 bits)
#define REG_PREVIEW_X           (PP_PAGE_IPV | 0x1F)
#define REG_IPV_WIN_X0_L        (PP_PAGE_IPV | 0x1F)
#define REG_IPV_WIN_X0_H        (PP_PAGE_IPV | 0x20)
#define REG_IPV_WIN_X1_L        (PP_PAGE_IPV | 0x21)
#define REG_IPV_WIN_X1_H        (PP_PAGE_IPV | 0x22)
#define REG_IPV_WIN_X2_L        (PP_PAGE_IPV | 0x23)
#define REG_IPV_WIN_X2_H        (PP_PAGE_IPV | 0x24)

// Sub-frame Y locations (13 bits)
#define REG_PREVIEW_Y           (PP_PAGE_IPV | 0x29)
#define REG_IPV_WIN_Y0_L        (PP_PAGE_IPV | 0x29)
#define REG_IPV_WIN_Y0_H        (PP_PAGE_IPV | 0x2A)
#define REG_IPV_WIN_Y1_L        (PP_PAGE_IPV | 0x2B)
#define REG_IPV_WIN_Y1_H        (PP_PAGE_IPV | 0x2C)
#define REG_IPV_WIN_Y2_L        (PP_PAGE_IPV | 0x2D)
#define REG_IPV_WIN_Y2_H        (PP_PAGE_IPV | 0x2E)

#define REG_IPV_ALPHA_0         (PP_PAGE_IPV | 0x3D)
#define REG_IPV_ALPHA_1         (PP_PAGE_IPV | 0x3E)
#define REG_IPV_ALPHA_2         (PP_PAGE_IPV | 0x3F)

#define IPV_WIN_3D_OFFSET0      (PP_PAGE_IPV | 0x42)
#define IPV_WIN_3D_OFFSET1      (PP_PAGE_IPV | 0x43)
#define IPV_WIN_3D_OFFSET2      (PP_PAGE_IPV | 0x44)

#define REG_IPV_WIN_STATUS      (PP_PAGE_IPV | 0x47)
#define BIT_FORCE_BLANK                 0x80
#define MSK_IPV_WIN_VALID               0x07


#define REG_IPV_BORDER_CTRL     (PP_PAGE_IPV | 0x48)
#define BIT_DB_ALL                      0x01            // Must be set to allow the rest
#define BIT_DB_ACT                      0x02
#define BIT_DB_CPB                      0x04
#define BIT_DB_USF                      0x08
#define MSK_DB                          0x0F

#define REG_PVRP_RESDET         (PP_PAGE_IPV | 0x49)
#define REG_PV_CSC_OVR1         (PP_PAGE_IPV | 0x4A)
#define REG_PV_CSC_OVR2         (PP_PAGE_IPV | 0x4B)

#define REG_PV_SUBFR_ENA        (PP_PAGE_IPV | 0x4C)


#define REG_PV_QCTL             (PP_PAGE_IPV | 0x4D)
#define REG_PV_DCTCTL           (PP_PAGE_IPV | 0x4E)

#define REG_IPV_CTRL            (PP_PAGE_IPV | 0x4F)
#define IPV_RESTART_CTRL0              0x01
#define IPV_RESTART_CTRL1              0x02


#define REG_PV_DSCFG_SH         (PP_PAGE_IPV | 0x50)
#define REG_PV_DSCFG_PH         (PP_PAGE_IPV | 0x51)
#define REG_PV_DSCFG_QH         (PP_PAGE_IPV | 0x52)
#define REG_PV_DSCFG_SV         (PP_PAGE_IPV | 0x53)
#define REG_PV_DSCFG_PV         (PP_PAGE_IPV | 0x54)
#define REG_PV_DSCFG_QV         (PP_PAGE_IPV | 0x55)

#define REG_PV_EDEG_CTL1        (PP_PAGE_IPV | 0x56)
#define REG_PV_SYNC_CTL         (PP_PAGE_IPV | 0x57)
#define REG_PV_SYNC_CTL2        (PP_PAGE_IPV | 0x58)
#define REG_PV_RP_RES_CTL       (PP_PAGE_IPV | 0x59)

#define REG_PV_RP_H_OVR_L       (PP_PAGE_IPV | 0x5A)
#define REG_PV_RP_H_OVR_U       (PP_PAGE_IPV | 0x5B)

#define REG_PV_RP_W_OVR_L       (PP_PAGE_IPV | 0x5C)
#define REG_PV_RP_W_OVR_U       (PP_PAGE_IPV | 0x5D)

#define REG_PV_FB_CTL           (PP_PAGE_IPV | 0x5E)

#define REG_PV_CFB_MAX_L        (PP_PAGE_IPV | 0x5F)
#define REG_PV_CFB_MAX_U        (PP_PAGE_IPV | 0x60)

#define REG_PV_CB_BASE_L        (PP_PAGE_IPV | 0x61)
#define REG_PV_CB_BASE_U        (PP_PAGE_IPV | 0x62)

#define REG_PV_Y_INC            (PP_PAGE_IPV | 0x63)
#define REG_PV_C_INC            (PP_PAGE_IPV | 0x64)

#define REG_PV_SEG_B_BASE_L     (PP_PAGE_IPV | 0x66)
#define REG_PV_SEG_B_BASE_U     (PP_PAGE_IPV | 0x67)
#define REG_PV_SEG_C_BASE_L     (PP_PAGE_IPV | 0x68)
#define REG_PV_SEG_C_BASE_U     (PP_PAGE_IPV | 0x69)
#define REG_PV_SEG_D_BASE_L     (PP_PAGE_IPV | 0x6A)
#define REG_PV_SEG_D_BASE_U     (PP_PAGE_IPV | 0x6B)
#define REG_PV_SEG_E_BASE_L     (PP_PAGE_IPV | 0x6C)
#define REG_PV_SEG_E_BASE_U     (PP_PAGE_IPV | 0x6D)
#define REG_PV_SEG_F_BASE_L     (PP_PAGE_IPV | 0x6E)
#define REG_PV_SEG_F_BASE_U     (PP_PAGE_IPV | 0x6F)


#define REG_PV_EODD_CTL         (PP_PAGE_IPV | 0x7B)

// CrCbY Boundary color for ALL/ACT/CPB/USF sub-frames
#define REG_IPV_ALL_BORDER_CR   (PP_PAGE_IPV | 0x80)
#define REG_IPV_ALL_BORDER_CB   (PP_PAGE_IPV | 0x81)
#define REG_IPV_ALL_BORDER_Y    (PP_PAGE_IPV | 0x82)
#define REG_IPV_ACT_BORDER_CR   (PP_PAGE_IPV | 0x83)
#define REG_IPV_ACT_BORDER_CB   (PP_PAGE_IPV | 0x84)
#define REG_IPV_ACT_BORDER_Y    (PP_PAGE_IPV | 0x85)
#define REG_IPV_NA_BORDER_CR    (PP_PAGE_IPV | 0x86)
#define REG_IPV_NA_BORDER_CB    (PP_PAGE_IPV | 0x87)
#define REG_IPV_NA_BORDER_Y     (PP_PAGE_IPV | 0x88)
#define REG_IPV_USF_BORDER_CR   (PP_PAGE_IPV | 0x89)
#define REG_IPV_USF_BORDER_CB   (PP_PAGE_IPV | 0x8A)
#define REG_IPV_USF_BORDER_Y    (PP_PAGE_IPV | 0x8B)

// Speed and acceleration of animation move
#define REG_PV_LOC_CTRL_X_SP    (PP_PAGE_IPV | 0x8C)
#define REG_PV_LOC_CTRL_X_AC    (PP_PAGE_IPV | 0x8D)
#define REG_PV_LOC_CTRL_Y_SP    (PP_PAGE_IPV | 0x8E)
#define REG_PV_LOC_CTRL_Y_AC    (PP_PAGE_IPV | 0x8F)

#define REG_PV_LOC_CTRL         (PP_PAGE_IPV | 0x90)
#define BIT_PV_SF_MOVE_START            BIT0
#define BIT_PV_SF_ANIM_ENABLE           BIT1

// CrCbY Background color for USF sub-frames
#define REG_IPV_USF_BGD_CR      (PP_PAGE_IPV | 0x93)
#define REG_IPV_USF_BGD_CB      (PP_PAGE_IPV | 0x94)
#define REG_IPV_USF_BGD_Y       (PP_PAGE_IPV | 0x95)


#endif  // __SI_9535REGS_IPV_H__

