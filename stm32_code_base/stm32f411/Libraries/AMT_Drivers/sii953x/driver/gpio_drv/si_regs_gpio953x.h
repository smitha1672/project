//***************************************************************************
//!file     si_regs_gpio953x.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_GPIO_H__
#define __SI_9535REGS_GPIO_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 0      (0xB0)
//------------------------------------------------------------------------------

#define REG_INT_STATUS_9            (PP_PAGE | 0x94)
#define REG_INT_ENABLE_9            (PP_PAGE | 0xA0)
#define BIT_GPIO_INT_0                  0x01
#define BIT_GPIO_INT_1                  0x02
#define MSK_GPIO_INT_1_0                0x03

#define REG_INT_STATUS_GPIO_1       (PP_PAGE | 0xAE)
#define REG_INT_ENABLE_GPIO_1       (PP_PAGE | 0xB0)
#define BIT_GPIO_INT_2                  0x01
#define BIT_GPIO_INT_3                  0x02
#define MSK_GPIO_INT_9_2                0xFF

#define REG_INT_STATUS_GPIO_2       (PP_PAGE | 0xAF)
#define REG_INT_ENABLE_GPIO_2       (PP_PAGE | 0xB1)
#define BIT_GPIO_INT_10                 0x01
#define BIT_GPIO_INT_11                 0x02
#define MSK_GPIO_INT_12_10              0x07

//------------------------------------------------------------------------------
// Registers in Page 9      (0xE0)
//------------------------------------------------------------------------------

#define REG_GPIO_AON_CTRL0          (PP_PAGE_GPIO | 0xC0)
#define MSK_ALTEN_1_0                   0x03
#define MSK_ALTEN_0                     0x01
#define MSK_DIREN_1_0                   0x0C
#define MSK_INT_EDGE_RISE_1_0           0x30
#define MSK_INT_EDGE_FALL_1_0           0xC0

#define REG_GPIO_AON_CTRL1          (PP_PAGE_GPIO | 0xC1)
#define MSK_WRITE_1_0                   0x03
#define MSK_READ_0                      0x04
#define MSK_READ_1_0                    0x0C
#define MSK_PAD_PU_1_0                  0x30        // Pull up/down
#define MSK_PAD_PE_1_0                  0xC0        // Pull up/down enable

#define REG_GPIO_ALTEN              (PP_PAGE_GPIO | 0xC2)
#define MSK_ALTEN_9_2                   0xFF
#define REG_GPIO_ALTEN_DIR          (PP_PAGE_GPIO | 0xC3)
#define MSK_ALTEN_12_10                 0x07
#define MSK_GPIO_11_10                  0x03
#define MSK_DIR_6_2                     0xF8
#define REG_GPIO_DIR                (PP_PAGE_GPIO | 0xC4)
#define MSK_DIR_12_7                    0x3F

#define REG_GPIO_INT_RE_EN          (PP_PAGE_GPIO | 0xC5)
#define MSK_INT_EDGE_RISE_9_2           0xFF
#define REG_GPIO_INT_RE_FE_EN       (PP_PAGE_GPIO | 0xC6)
#define MSK_INT_EDGE_RISE_12_10         0x07
#define MSK_INT_EDGE_FALL_6_2           0xF8
#define REG_GPIO_INT_FE_EN          (PP_PAGE_GPIO | 0xC7)
#define MSK_INT_EDGE_FALL_12_7          0x3F

#define REG_GPIO_WR                 (PP_PAGE_GPIO | 0xC8)
#define MSK_WRITE_9_2                   0xFF
#define REG_GPIO_WR_RD              (PP_PAGE_GPIO | 0xC9)
#define MSK_WRITE_12_10                 0x07
#define MSK_READ_6_2                    0xF8
#define REG_GPIO_RD                 (PP_PAGE_GPIO | 0xCA)
#define MSK_READ_12_7                   0x3F

#define REG_GPIO_PU_SEL             (PP_PAGE_GPIO | 0xCB)
#define MSK_GPIO_9_2                    0xFF
#define REG_GPIO_PU_SEL_EN          (PP_PAGE_GPIO | 0xCC)
#define MSK_GPIO_12_10                  0x07
#define MSK_GPIO_6_2                    0xF8
#define REG_GPIO_PU_EN              (PP_PAGE_GPIO | 0xCD)
#define MSK_GPIO_12_7                   0x3F
#define REG_GPIO_PRI_SEC_SEL        (PP_PAGE_GPIO | 0xCE)
#define BIT_GPIO_SEC_SEL0               0x01
#define BIT_GPIO_SEC_SEL1               0x02
#define BIT_GPIO_SEC_SEL2               0x04

#endif  // __SI_9535REGS_GPIO_H__

