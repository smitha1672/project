//***************************************************************************
//!file     si_regs_rxedid953x.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_RXEDID_H__
#define __SI_9535REGS_RXEDID_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 9      (0xE0)
//------------------------------------------------------------------------------


#define REG_EDID_FIFO_ADDR          (PP_PAGE_EDID | 0x02)
#define VAL_FIFO_ADDR_00                0x00
#define VAL_FIFO_ADDR_80                0x80

#define REG_EDID_FIFO_DATA          (PP_PAGE_EDID | 0x03)
#define REG_EDID_FIFO_SEL           (PP_PAGE_EDID | 0x04)
#define MSK_SEL_EDID_FIFO               0x07
#define BIT_SEL_DEVBOOT                 0x10
#define BIT_SEL_SSTD_TX1_FIRST          0x20
#define MSK_KSV_SELECT                  0xC0
#define VAL_KSV_SELECT_TX1              0x80

#define REG_NVM_COMMAND             (PP_PAGE_EDID | 0x05)
#define VAL_PRG_EDID                    0x03
#define VAL_PRG_DEVBOOT                 0x04
#define VAL_PRG_VGA                     0x13
#define VAL_COPY_EDID                   0x05
#define VAL_COPY_DEVBOOT                0x06
#define VAL_COPY_VGA                    0x07


#define REG_NVM_COPYTO              (PP_PAGE_EDID | 0x06)
#define MSK_NVM_COPYTO                  0x7F
#define BIT_NVM_COPYTO_PORT0            0x01
#define BIT_NVM_COPYTO_PORT1            0x02
#define BIT_NVM_COPYTO_PORT2            0x04
#define BIT_NVM_COPYTO_PORT3            0x08
#define BIT_NVM_COPYTO_PORT4            0x10
#define BIT_NVM_COPYTO_PORT5            0x20
#define BIT_NVM_COPYTO_PORT6            0x40
#define BIT_NVM_COPYTO_VGA              0x40

#define REG_NVM_COMMAND_DONE        (PP_PAGE_EDID | 0x07)
#define BIT_NVM_COMMAND_DONE            0x01

#define REG_BSM_INIT                (PP_PAGE_EDID | 0x08)
#define BIT_BSM_INIT                    0x01

#define REG_BSM_STAT                (PP_PAGE_EDID | 0x09)
#define BIT_BOOT_IN_PROGRESS            0x08
#define BIT_BOOT_DONE                   0x04
#define BIT_BOOT_ERROR                  0x01
#define BIT_EDID_COPY_ERROR             0x02
#define MSK_BOOT_ERROR                  0x03

#define REG_DDC_NVRAM_CONFIG        (PP_PAGE_EDID | 0x0A)
#define BIT_DDC_NVRAM_ENABLE            0x01

#define REG_DDC_NVRAM_ADDR          (PP_PAGE_EDID | 0x0B)

#define REG_NVM_STAT                (PP_PAGE_EDID | 0x10)
#define BIT_NVM_EDID_VALID              0x01
#define BIT_NVM_DEVBOOT_VALID           0x02
#define VAL_NVM_VALID                   (BIT_NVM_EDID_VALID | BIT_NVM_DEVBOOT_VALID)

#define REG_NVM_BOOT_SEL            (PP_PAGE_EDID | 0x12)
#define MSK_NVM_BOOT_SEL                0x7F

#define REG_HPE_HW_CTRL_MODE        (PP_PAGE_EDID | 0x13)

#define REG_HPE_HW_CTRL_OVR         (PP_PAGE_EDID | 0x14)
#define MSK_INVALIDATE_HW_HPD_CTRL      0x3F

#define REG_CECPA_CTRL              (PP_PAGE_EDID | 0x19)
#define VAL_PA_CS_NVRAM					0x00
#define VAL_PA_CS_TX0					0x01
#define VAL_PA_CS_TX1					0x02
#define BIT_TX0_SRC_REG                 BIT2
#define BIT_CEC_PULSE					BIT6
#define BIT_CEC_DEST_TX1				BIT7

#define REG_CECPA_ADDR              (PP_PAGE_EDID | 0x1A)

#define REG_CECPAD_L_CH0            (PP_PAGE_EDID | 0x1C)
#define REG_CECPAD_H_CH0            (PP_PAGE_EDID | 0x1D)

#define REG_CHECKSUM_CH0            (PP_PAGE_EDID | 0x2C)

#define REG_AUTO_CONFIG             (PP_PAGE_EDID | 0x40)
#define BIT_AUTO_TERM_EN                0x01

//#define REG_I2C_PT_CTRL             (PP_PAGE_EDID | 0x46)
//#define BIT_PT_MODE_FLUSH               0x01
//#define BIT_PT_ENABLE                   0x02
//#define MSK_PT_PORT_SEL                 0xE0

#define REG_SCRATCH0                (PP_PAGE_EDID | 0x50)
#define REG_SCRATCH1                (PP_PAGE_EDID | 0x51)

#define REG_EDID_CTRL_XTRA          (PP_PAGE_EDID | 0x70)
#define BIT_TWO_EDID_MODE               0x10

#define REG_CEC_PA_ADDR         (PP_PAGE_EDID | 0x90)
#define REG_CEC_PA_L_CH0        (PP_PAGE_EDID | 0x92)
#define REG_CEC_PA_H_CH0        (PP_PAGE_EDID | 0x93)
#define REG_CEC_PA_L_CH1        (PP_PAGE_EDID | 0x94)
#define REG_CEC_PA_H_CH1        (PP_PAGE_EDID | 0x95)
#define REG_CEC_PA_L_CH2        (PP_PAGE_EDID | 0x96)
#define REG_CEC_PA_H_CH2        (PP_PAGE_EDID | 0x97)
#define REG_CEC_PA_L_CH3        (PP_PAGE_EDID | 0x98)
#define REG_CEC_PA_H_CH3        (PP_PAGE_EDID | 0x99)

#define REG_CEC_CHECKSUM_CH0        (PP_PAGE_EDID | 0xA2)
#define REG_CEC_CHECKSUM_CH1        (PP_PAGE_EDID | 0xA3)
#define REG_CEC_CHECKSUM_CH2        (PP_PAGE_EDID | 0xA4)
#define REG_CEC_CHECKSUM_CH3        (PP_PAGE_EDID | 0xA5)

#endif  // __SI_953xREGS_RXEDID_H__

