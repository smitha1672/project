//***************************************************************************
//!file     si_eeprom.h
//!brief    Silicon Image Starter Kit EEPROM interface
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_EEPROM_H__
#define __SI_EEPROM_H__

#include "si_common.h"

#define EEPROM_I2C_ADDRESS      0xA8

#define EE_CONFIG_VALID         0x0000
#define CONFIG_VALID            0x55        // Contents of configuration block have been initialized.

#define EE_FW_CONTROL           0x0001
#define EEFLAG_HOST_ENABLE      0x01        // 1 - Host Control, 0 ?Internal Control
#define EEFLAG_IR_ENABLE        0x02        // 1 ?Enable IR receiver
#define EEFLAG_ROTARY_ENABLE    0x04        // 1 ?Enable on-board rotary switch
#define EEFLAG_CEC_ENABLE       0x08        // 1 ?Enable CEC parsing
#define EEFLAG_CEC_PRINT        0x10        // 1 - Enable CEC DEBUG_PRINT messages
#define EEFLAG_CEC_ENUM         0x20        // 1 - Enable CEC enumeration every 20 seconds
                                            // 0 - Enable CEC enumeration only at startup
#define EEFLAG_NVRAM_IGNORE     0x40        // 1 - Don't read or write NVRAM in normal operation
#define EEFLAG_DBG_MSG_COLORING 0x80        // 1 - Enable DEBUG_PRINT message coloring
                                            // 0 - Disable DEBUG_PRINT message coloring


#define EEPROM_EDID_LOADSEL     0x0002      // Select which EDID rams to load from EEPROM
#define EE_EDID0_LOAD           0x01        // Load EDID ram 0 from EEPROM EDID 0
#define EE_EDID1_LOAD           0x02        // Load EDID ram 1 from EEPROM EDID 1
#define EE_EDID2_LOAD           0x04        // Load EDID ram 2 from EEPROM EDID 2
#define EE_EDID3_LOAD           0x08        // Load EDID ram 3 from EEPROM EDID 3
#define EE_EDID4_LOAD           0x10        // Load EDID ram 4 from EEPROM EDID VGA
#define EE_EDID5_LOAD           0x20        // Load EDID ram 4 from EEPROM EDID VGA

#define EE_DB_VERSION           0x0003
#define EEPROM_DB_VERSION       0x10        // Database version 1.0

#define EE_UART_MSG_LEVEL       0x0004
#define MSG_ALWAYS              0x00
#define MSG_ERR                 0x01
#define MSG_STAT                0x02
#define MSG_DBG                 0x03
#define MSG_PRINT_ALL           0xFF

#define EE_EDID_VALID           0x0010
#define EE_EDIDVGA_VALID        0x0011
#define EE_DEVBOOTDATA_VALID    0x0012

#define EE_EDID0_VALID          EE_EDID_VALID
#define EE_EDID1_VALID          0x0013
#define EE_EDID2_VALID          0x0014
#define EE_EDID3_VALID          0x0015
#define EE_EDID4_VALID          EE_EDIDVGA_VALID
#define EE_EDID5_VALID          0x0016


#define EE_EDID_OFFSET          0x0100
#define EE_EDIDVGA_OFFSET       (EE_EDID_OFFSET + EDID_TABLE_LEN)
#define EE_DEVBOOT_OFFSET       (EE_EDIDVGA_OFFSET + EDID_TABLE_LEN)
#define CBUS_SEL_OFFSET         0x24    // Offset of CBUS port select byte in NVRAM boot data

#define EE_EDID0_OFFSET         EE_EDID_OFFSET
#define EE_EDID1_OFFSET         0x400
#define EE_EDID2_OFFSET         (EE_EDID1_OFFSET + EDID_TABLE_LEN)
#define EE_EDID3_OFFSET         (EE_EDID2_OFFSET + EDID_TABLE_LEN)
#define EE_EDID5_OFFSET         (EE_EDID3_OFFSET + EDID_TABLE_LEN)
#define EE_EDID4_OFFSET         EE_EDIDVGA_OFFSET



//------------------
// Repeater configuration
// (Registers 0x0020...0x003F are reserved for repeater support)

#define EE_REPEATER_BASE        0x0020

#define EE_REPEATER_SIGNATURE_OFFSET (EE_REPEATER_BASE + 0x0000)
#define EE_REPEATER_SIGNATURE_LENGTH 5

#define EE_REPEATER_MODE_OFFSET (EE_REPEATER_BASE + 0x0005)

#define EE_KSV_COUNT_OFFSET     (EE_REPEATER_BASE + 0x0006)

#define EE_KSV_DATA             (EE_REPEATER_BASE + 0x0010)

//-------------------------------------------------------------------------------
//
//  EEPROM Functions
//
//-------------------------------------------------------------------------------

uint8_t SiiPlatformEepromReadByte( uint16_t addr );
void	SiiPlatformEepromWriteByte( uint16_t addr, uint8_t value );
bool_t	SiiPlatformEepromReadBlock( uint16_t addr, uint8_t *pRdData, uint16_t nBytes );
bool_t	SiiPlatformEepromWriteBlock( uint16_t addr, const uint8_t *pWrData, uint16_t nBytes );

#endif  // __SI_EEPROM_H__

