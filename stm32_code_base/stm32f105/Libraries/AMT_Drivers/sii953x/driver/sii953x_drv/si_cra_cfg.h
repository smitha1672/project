//***************************************************************************
//!file     si_cra_cfg.h
//!brief    Silicon Image Device CRA configuration data.
//
// No part of this work may be reproduced, modified, distributed, 
// transmitted, transcribed, or translated into any language or computer 
// format, in any form or by any means without written permission of 
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CRA_CFG_H__
#define __SI_CRA_CFG_H__
#include "si_common.h"

typedef enum _deviceBusTypes_t
{
    // The following four must remain together because they are used
    // as bus indices for the CraWriteBlockI2c and CraReadBlockI2c functions
    DEV_I2C_0,          // Main I2C bus
    DEV_I2C_1,          // Separate I2C bus
    DEV_I2C_2,          // Separate I2C bus
    DEV_I2C_3,          // Separate I2C bus

    // The following four must remain together because they are used
    // as bus indices for the CraWriteBlockI2c and CraReadBlockI2c functions
    // with the formula offsetBusTypeX - DEV_I2C_OFFSET == DEV_I2C_x
    DEV_I2C_OFFSET,     // Main I2C bus with register offset
    DEV_I2C_1_OFFSET,   // Separate I2C bus
    DEV_I2C_2_OFFSET,   // Separate I2C bus
    DEV_I2C_3_OFFSET,   // Separate I2C bus

    DEV_DDC_0,          // DDC bus for TX 0
    DEV_DDC_1,          // DDC bus for TX 1

    DEV_MMIO,           // Memory-mapped I/O

    DEV_I2C_ADDR16,     // Main I2C bus, 16 bit register address
} deviceBusTypes_t;

// Actual I2C page addresses for the various devices
// TODO:OEM - If a device ID has been reassigned, update it here AND in
//            the g_siiRegPageBaseReassign[] array
typedef enum _devicePageIds_t
{
    DEV_PAGE_PP_0       = (0xB0),
#if (FPGA_BUILD == ENABLE)    	
    DEV_PAGE_PP_2       = (0xAA),
#else
    DEV_PAGE_PP_2       = (0x66),
#endif
    DEV_PAGE_IPV        = (0xFA),
    DEV_PAGE_PP_4       = (0x68),
    DEV_PAGE_PP_5       = (0x50),
    DEV_PAGE_PP_6       = (0x52),
    DEV_PAGE_PP_7       = (0x54),
    DEV_PAGE_PP_8       = (0xC0),
    DEV_PAGE_PP_9       = (0xE0),
#if (FPGA_BUILD == ENABLE)    	    
    DEV_PAGE_PP_A       = (0xBB),
#else
    DEV_PAGE_PP_A       = (0x64),
#endif
    DEV_PAGE_PP_B       = (0x90),
    DEV_PAGE_CBUS       = (0xE6),   // PP_C
    DEV_PAGE_CBUS_1     = (0xE2),   // PP_C
    DEV_PAGE_HEAC       = (0xD0),
    DEV_PAGE_CPI_TX     = (0xC4),
    DEV_PAGE_CPI_RX     = (0xC0),
    DEV_PAGE_TPI_0      = (0x72),
    DEV_PAGE_TX_L0_0    = (0x72),
    DEV_PAGE_TX_L1_0    = (0x7A),
    DEV_PAGE_TX_2_0     = (0xFF),
    DEV_PAGE_TPI_1      = (0x70),
    DEV_PAGE_TX_L0_1    = (0x70),
    DEV_PAGE_TX_L1_1    = (0x78),
    DEV_PAGE_TX_2_1     = (0xFF),
    DEV_PAGE_OSD        = (0xF0),
    DEV_PAGE_AUDIO      = (0xF8),

    DEV_PAGE_DDC_EDID   = (0xA0),
    DEV_PAGE_DDC_SEGM   = (0x60),
} devicePageIds_t;

// Virtual page addresses.
// Index into pageConfig_t array (shifted left by 8)
typedef enum _SiiRegPageIndex_t
{
    PP_PAGE             = 0x0000,   // Port Processor base address
    PP_PAGE_ARC			=	PP_PAGE,
    PP_PAGE_1           = 0x0100,   // Unused
    PP_PAGE_2           = 0x0200,   // RX TMDS
    PP_PAGE_IPV         = 0x0300,   // IPV
    PP_PAGE_4           = 0x0400,   // RX TMDS
    PP_PAGE_5           = 0x0500,   // PA1
    PP_PAGE_PAUTH1      = 0x0500,   // same as PP_PAGE_5
    PP_PAGE_6           = 0x0600,   // PA2
    PP_PAGE_PAUTH2      = 0x0600,   // same as PP_PAGE_6
    PP_PAGE_7           = 0x0700,   // PA3
    PP_PAGE_PAUTH3      = 0x0700,   // same as PP_PAGE_7
    CPI_PAGE            = 0x0800,   // CPI
    PP_PAGE_9           = 0x0900,   // NVRAM
    PP_PAGE_EDID        = 0x0900,   // Same as PP_PAGE_9
    PP_PAGE_GPIO        = 0x0900,   // Same as PP_PAGE_9
    PP_PAGE_A           = 0x0A00,   // RX TMDS
    PP_PAGE_B           = 0x0B00,   // TX TMDS
    CBUS_PAGE           = 0x0C00,   // CBUS
    PP_PAGE_HEAC        = 0x0D00,   // HEAC
    PP_PAGE_OSD         = 0x0E00,   // OSD
    PP_PAGE_AUDIO       = 0x0F00,   // Audio extraction
    TX_PAGE_TPI         = 0x1000,   // TPI
    TX_PAGE_L0          = 0x1100,   // TX Legacy page 0
    TX_PAGE_L1          = 0x1200,   // TX Legacy page 1
    TX_PAGE_2           = 0x1300,   // TX page 2 (Not accessible directly - use indirect rd/wr)
    TX_PAGE_DDC_EDID    = 0x1400,   // TX DDC EDID
    TX_PAGE_DDC_SEGM    = 0x1500,   // TX DDC EDID Segment address
    PP_PAGE_AUDIO_MIXER = 0x1600,   // Audio mixer (part of audio extraction page)
    PP_PAGE_AUDIO_INTS  = 0x1700,   // Audio interrupts (part of page 0)

} SiiRegPageIndex_t;

#define SII_CRA_MAX_DEVICE_INSTANCES    2   // Maximum size of instance dimension of address descriptor array
#define SII_CRA_DEVICE_PAGE_COUNT       25  // Number of entries in pageConfig_t array

typedef struct pageConfig
{
    deviceBusTypes_t    busType;    // I2C
    int_t               address;    // I2C DEV ID, mem offset
} pageConfig_t;

#endif  // __SI_CRA_CFG_H__
