//***************************************************************************
//!file     si_cra_cfg.c
//!brief    Silicon Image Si9535 Starter Kit Firmware CRA configuration data.
//
// No part of this work may be reproduced, modified, distributed, 
// transmitted, transcribed, or translated into any language or computer 
// format, in any form or by any means without written permission of 
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_cra.h"
#include "si_cra_cfg.h"
#include "si_regs_pp953x.h"


//------------------------------------------------------------------------------
// si_cra_cfg.h
//------------------------------------------------------------------------------

// Index to this array is the virtual page number in the MSB of the REG_xxx address values
// Indexed with siiRegPageIndex_t value shifted right 8 bits
// DEV_PAGE values must correspond to the order specified in the SiiRegPageIndex_t enum list
pageConfig_t    g_addrDescriptor[SII_CRA_MAX_DEVICE_INSTANCES][SII_CRA_DEVICE_PAGE_COUNT] =
{
    // Instance 0
    {
    { DEV_I2C_0,        DEV_PAGE_PP_0   },    // Port Processor
    { DEV_I2C_0,        0   },
    { DEV_I2C_0,        DEV_PAGE_PP_2   },    // RX TMDS
    { DEV_I2C_0,        DEV_PAGE_IPV    },    // InstaPreview
    { DEV_I2C_0,        DEV_PAGE_PP_4   },    // RX TMDS
    { DEV_I2C_0,        DEV_PAGE_PP_5   },    // PAUTH0
    { DEV_I2C_0,        DEV_PAGE_PP_6   },    // PAUTH1
    { DEV_I2C_0,        DEV_PAGE_PP_7   },    // PAUTH2
    { DEV_I2C_0,        DEV_PAGE_PP_8   },    // CPI 0
    { DEV_I2C_0,        DEV_PAGE_PP_9   },    // NVRAM
    { DEV_I2C_0,        DEV_PAGE_PP_A   },    // RX TMDS
    { DEV_I2C_0,        DEV_PAGE_PP_B   },    // TX TMDS
    { DEV_I2C_0,        DEV_PAGE_CBUS   },    // CBUS 0
    { DEV_I2C_0,        DEV_PAGE_HEAC   },    // HEAC
    { DEV_I2C_0,        DEV_PAGE_OSD    },    // OSD
    { DEV_I2C_OFFSET,   0x0000 + DEV_PAGE_AUDIO },    // Audio Extraction instance 0

    { DEV_I2C_0,        DEV_PAGE_TPI_0    },  // TPI 0
    { DEV_I2C_0,        DEV_PAGE_TX_L0_0  },  // TX 0 Legacy 0
    { DEV_I2C_0,        DEV_PAGE_TX_L1_0  },  // TX 0 Legacy 1
    { DEV_I2C_0,        DEV_PAGE_TX_2_0   },  // TX 0 Page 2 (not legacy)

    { DEV_DDC_0,        DEV_PAGE_DDC_EDID },  // TX EDID DDC 0
    { DEV_DDC_0,        DEV_PAGE_DDC_SEGM },  // TX EDID DDC 0

    { DEV_I2C_OFFSET,   0x0000 + DEV_PAGE_AUDIO },    // Audio Mixer instance 0
    { DEV_I2C_OFFSET,   0x0000 + DEV_PAGE_PP_0  },    // Audio Interrupts instance 0
    { DEV_I2C_0,        DEV_PAGE_AUDIO },    // Audio Mixer instance 1
    },

    // Instance 1
    {
    { DEV_I2C_0,        DEV_PAGE_PP_0     },
    { DEV_I2C_0,        0   },
    { DEV_I2C_0,        DEV_PAGE_PP_2     },
    { DEV_I2C_0,        DEV_PAGE_IPV      },
    { DEV_I2C_0,        DEV_PAGE_PP_4     },
    { DEV_I2C_0,        DEV_PAGE_PP_5     },
    { DEV_I2C_0,        DEV_PAGE_PP_6     },
    { DEV_I2C_0,        DEV_PAGE_PP_7     },
    { DEV_I2C_0,        DEV_PAGE_CPI_TX   },    // CPI 1
    { DEV_I2C_0,        DEV_PAGE_PP_9     },
    { DEV_I2C_0,        DEV_PAGE_PP_A     },
    { DEV_I2C_0,        DEV_PAGE_PP_B     },
    { DEV_I2C_0,        DEV_PAGE_CBUS_1   },   // CBUS 1
    { DEV_I2C_0,        DEV_PAGE_HEAC     },
    { DEV_I2C_0,        DEV_PAGE_OSD      },
    { DEV_I2C_OFFSET,   0x8000 + DEV_PAGE_AUDIO },    // Audio Extraction instance 1

    { DEV_I2C_0,        DEV_PAGE_TPI_1    },  // TPI 1
    { DEV_I2C_0,        DEV_PAGE_TX_L0_1  },  // TX 1 Legacy 0
    { DEV_I2C_0,        DEV_PAGE_TX_L1_1  },  // TX 1 Legacy 1
    { DEV_I2C_0,        DEV_PAGE_TX_2_1   },  // TX 1 Page 2 (not legacy)

    { DEV_DDC_1,        DEV_PAGE_DDC_EDID },  // TX EDID DDC 1
    { DEV_DDC_1,        DEV_PAGE_DDC_SEGM },  // TX EDID DDC 1

    { DEV_I2C_OFFSET,   0x0800 + DEV_PAGE_AUDIO },    // Audio Mixer instance 1
    { DEV_I2C_OFFSET,   0x0600 + DEV_PAGE_PP_0  },    // Audio Interrupts instance 1
    { DEV_I2C_0,          DEV_PAGE_AUDIO },    // Audio Mixer instance 1
    }
};

// Register addresses for re-assigning page base addresses
// These registers specify the I2C address that the SI device will
// respond to for the specific control register page
SiiReg_t g_siiRegPageBaseRegs [SII_CRA_DEVICE_PAGE_COUNT] =
{
    PP_PAGE | 0xFF,     // Device Base  - Cannot be reassigned
    PP_PAGE | 0xFF,     // Unused       - Cannot be reassigned
    REG_SLAVE_ADDR6,    // RX TMDS
    REG_SLAVE_ADDR13,   // IPV
    REG_SLAVE_ADDR7,    // RX TMDS
    REG_SLAVE_ADDR8,    // PA Pages 5,6,7 have special mapping requirements
    REG_SLAVE_ADDR8,    // PA
    REG_SLAVE_ADDR8,    // PA
    REG_SLAVE_ADDR2,    // CPI
    REG_SLAVE_ADDR3,    // NVRAM/GPIO
    REG_SLAVE_ADDR4,    // RX TMDS
    REG_SLAVE_ADDR5,    // TX TMDS
    REG_SLAVE_ADDR1,    // CBUS
    REG_SLAVE_ADDR0,    // HEAC
    REG_SLAVE_ADDR11,   // OSD
    REG_SLAVE_ADDR12,   // Audio extraction
    REG_SLAVE_ADDR_TX0_0,   // TPI Tx0
    REG_SLAVE_ADDR_TX0_0,   // Tx0 Legacy page 0
    REG_SLAVE_ADDR_TX0_1,   // Tx0 Legacy page 1
    PP_PAGE | 0xFF,     // Tx0 page 2    - Cannot be reassigned
    PP_PAGE | 0xFF,     // TX DDC EDID  - Cannot be reassigned
    PP_PAGE | 0xFF,     // TX DDC EDID Segment address  - Cannot be reassigned
};

// TODO:OEM - Add entries to reassign register page base addresses if needed
SiiReg_t g_siiRegPageBaseReassign [] =
{
//        PP_PAGE_3 | 0xFC,       // Example of changing default page 3 device ID (0xFA) to 0xFC
//                                // Note that the DEV_PAGE_IPV enum value must also be changed

#if (FPGA_BUILD == ENABLE)
        PP_PAGE_2 | 0xAA,       // Change default 0x66 to 0xAA
        PP_PAGE_A | 0xBB,       // Change default 0x64 to 0xBB
#endif

        0xFFFF      // End of reassignment list
};
