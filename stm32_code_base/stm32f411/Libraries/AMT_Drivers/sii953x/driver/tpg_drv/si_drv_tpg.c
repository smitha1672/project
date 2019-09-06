//***************************************************************************
//! @file     si_drv_tpg.c
//! @brief    Video Generator driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <string.h>
#include "si_drv_device.h"
#include "si_regs_tpg953x.h"
#include "si_regs_gpio953x.h"
#include "si_regs_pp953x.h"
#include "si_drv_tpi_access.h"
#include "si_drv_internal.h"
#include "si_drv_tpg.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Set format (resolution and frame rate) of generated video.
//!
//! @param[in]  format - format ID. @see tpgVideoFormat_t.
//! @param[in]  clockSrc  - video clock source.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpgVideoFormatSet(tpgVideoFormat_t format, tpgClockSource_t clockSrc)
{

    // SWWA:22565
    // Main pipe must be set to forced port selection mode and the selected port has to have active connection.
    // This prevents APLL1 from being reset by PA events.
    if (clockSrc == SI_TPG_CLK_P0)
    {
        uint8_t ckdt;
        int_t   port = 1;

        ckdt = SiiRegRead(REG_CLKDETECT_STATUS) & MSK_CKDT_ALL;
        if (ckdt)
        {
            // find first available active port
            while (!(ckdt & 0x1))
            {
                port <<= 1;
                ckdt >>= 1;
            }
            // force port selection in digital
            SiiRegWrite(REG_PAUTH_RPOVR, 0x80 | port);
            // ... and analog domains
            SiiRegWrite(REG_PAUTH_RP_AOVR, port);
        }
        else
        {
            // if no input connection has been detected,
            // resort to XCLK option
            clockSrc = SI_TPG_CLK_XCLK;
            DEBUG_PRINT( MSG_ERR, "Using APLL1 requires at least one active HDMI input!\n");
        }
    }
    else
    {
        // clear forced port selection
        SiiRegWrite(REG_PAUTH_RPOVR, 0);
        SiiRegWrite(REG_PAUTH_RP_AOVR, 0);
    }

    SiiRegModify( REG_VPG_CTRL_3, MSK_VPG_FORMAT | MSK_VPG_CLK_SEL, (clockSrc << 1) | (format << VAL_VPG_FORMAT_SHIFT));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set video test pattern to be generated.
//!
//! @param[in]  pattern - pattern ID. @see tpgVideoPattern_t.
//-------------------------------------------------------------------------------------------------

void SiiDrvExtTpgPatternSet(tpgVideoPattern_t pattern)
{
    pattern <<= VAL_VPG_PATTERN_SHIFT;
    SiiRegModify( REG_VPG_CTRL_2, MSK_VPG_PATTERN, pattern );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable or disable test video pattern generation.
//!
//!             The pattern is NTSC color bars.
//!
//! @param[in]  isEnabled - enable flag.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpgEnable(bool_t isEnabled)
{
    if (isEnabled)
    {
        DEBUG_PRINT( MSG_DBG, ("TPG Enabled\n"));
        SiiRegBitsSet(REG_VPG_CTRL_3, BIT_VPG_EN, true );
    }
    else
    {
        DEBUG_PRINT( MSG_DBG, ("TPG Disabled\n"));
        SiiRegBitsSet(REG_VPG_CTRL_3, BIT_VPG_EN, false );
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable or disable extended test video pattern generation.
//!
//!             The pattern is selectable by SiiDrvExtTpgPatternSet() function.
//!
//! @param[in]  isEnabled - enable flag.
//-------------------------------------------------------------------------------------------------

void SiiDrvExtTpgEnable(bool_t isEnabled)
{
	 //DEBUG_PRINT(MSG_DBG,"\n\n SiiDrvExtTpgEnable:%x\n\n",isEnabled);
    if (isEnabled)
    {
        SiiRegBitsSet(REG_VPG_CTRL_3, BIT_VPG_EN, false );      // Disable basic patterns
        SiiRegBitsSet( REG_VPG_CTRL_1, BIT_VPG_EXT_EN, true );  // Enable extended patterns
        DEBUG_PRINT( MSG_DBG, ("ETPG Enabled\n"));
    }
    else
    {
        SiiRegBitsSet( REG_VPG_CTRL_1, BIT_VPG_EXT_EN, false ); // Disable extended patterns
        DEBUG_PRINT( MSG_DBG, ("ETPG Disabled\n"));

        // SWWA:22565 Second part: release PA from forced port selection mode
        {
            // clear forced port selection
            SiiRegWrite(REG_PAUTH_RPOVR, 0);
            SiiRegWrite(REG_PAUTH_RP_AOVR, 0);
        }
    }
}

