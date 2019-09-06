//***************************************************************************
//!file     si_regs_cpi.h
//!brief    CPI Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_REGS_CPI_H__
#define __SI_REGS_CPI_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 8
//------------------------------------------------------------------------------

#define REG_CEC_DEBUG_2             (CPI_PAGE | 0x86)

#define REG_CEC_DEBUG_3             (CPI_PAGE | 0x87)
#define BIT_SNOOP_EN                        0x01
#define BIT_FLUSH_TX_FIFO                   0x80

#define REG_CEC_TX_INIT             (CPI_PAGE | 0x88)
#define BIT_SEND_POLL                       0x80

#define REG_CEC_TX_DEST             (CPI_PAGE | 0x89)

#define REG_CEC_CONFIG_CPI          (CPI_PAGE | 0x8E)

#define REG_CEC_TX_COMMAND          (CPI_PAGE | 0x8F)
#define REG_CEC_TX_OPERAND_0        (CPI_PAGE | 0x90)

#define REG_CEC_TRANSMIT_DATA       (CPI_PAGE | 0x9F)
#define BIT_TX_BFR_ACCESS                   0x40
#define BIT_TX_AUTO_CALC                    0x20
#define BIT_TRANSMIT_CMD                    0x10

#define REG_CEC_CAPTURE_ID0         (CPI_PAGE | 0xA2)
#define REG_CEC_CAPTURE_ID1         (CPI_PAGE | 0xA3)
#define BIT_VIRTUAL_DEVICE                  BIT7

#define REG_CEC_INT_ENABLE_0        (CPI_PAGE | 0xA4)
#define REG_CEC_INT_ENABLE_1        (CPI_PAGE | 0xA5)

// 0xA6 CPI Interrupt Status Register (R/W)
#define REG_CEC_INT_STATUS_0        (CPI_PAGE | 0xA6)
#define BIT_CEC_LINE_STATE                  0x80
#define BIT_TX_BUFFER_FULL                  0x40
#define BIT_TX_MESSAGE_SENT                 0x20
#define BIT_TX_FIFO_EMPTY                   0x04
#define BIT_RX_MSG_RECEIVED                 0x02
#define BIT_CMD_RECEIVED                    0x01

// 0xA7 CPI Interrupt Status Register (R/W)
#define REG_CEC_INT_STATUS_1        (CPI_PAGE | 0xA7)
#define BIT_RX_FIFO_OVERRUN                 0x08
#define BIT_SHORT_PULSE_DET                 0x04
#define BIT_FRAME_RETRANSM_OV               0x02
#define BIT_START_IRREGULAR                 0x01

#define REG_CEC_RX_CONTROL          (CPI_PAGE | 0xAC)
// CEC  CEC_RX_CONTROL bits
#define BIT_CLR_RX_FIFO_CUR                 0x01
#define BIT_CLR_RX_FIFO_ALL                 0x02

#define REG_CEC_RX_COUNT            (CPI_PAGE | 0xAD)
#define BIT_MSG_ERROR                       0x80


#define REG_CEC_RX_CMD_HEADER       (CPI_PAGE | 0xAE)
#define REG_CEC_RX_OPCODE           (CPI_PAGE | 0xAF)
#define REG_CEC_RX_OPERAND_0        (CPI_PAGE | 0xB0)


#define REG_CEC_RX_COUNT            (CPI_PAGE | 0xAD)

#define CEC_OP_ABORT_0              (CPI_PAGE | 0xC0)
#define CEC_OP_ABORT_1              (CPI_PAGE | 0xC1)
#define CEC_OP_ABORT_8              (CPI_PAGE | 0xC8)
#define CEC_OP_ABORT_14             (CPI_PAGE | 0xCE)
#define CEC_OP_ABORT_16             (CPI_PAGE | 0xD0)
#define CEC_OP_ABORT_17             (CPI_PAGE | 0xD1)
#define CEC_OP_ABORT_31             (CPI_PAGE | 0xDF)


#define REG_CEC_AUTO_DISCOVER_CMD   (CPI_PAGE | 0xE0)
#define BIT_CEC_AUTO_DISCOVER_START         0x01
#define BIT_CEC_AUTO_DISCOVER_CLEAR         0x02
#define BIT_CEC_AUTO_DISCOVER_DONE          0x80

#define REG_CEC_AUTO_DISCOVER_MAP0  (CPI_PAGE | 0xE1)
#define REG_CEC_AUTO_DISCOVER_MAP1  (CPI_PAGE | 0xE2)

#endif  // __SI_REGS_CPI_H__

