//***************************************************************************
//! @file     si_drv_tpg.h
//! @brief    Video Generator driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef SI_DRV_TPG_H
#define SI_DRV_TPG_H


//-------------------------------------------------------------------------------------------------
// Type definitions
//-------------------------------------------------------------------------------------------------

typedef enum tpgVideoFormat
{
    SI_TPG_FMT_VID_720_60             = 0x00,
    SI_TPG_FMT_VID_720_50             = 0x01,
    SI_TPG_FMT_VID_576_50             = 0x02,
    SI_TPG_FMT_VID_480_60             = 0x03,
    SI_TPG_FMT_VID_1080_50_8bpp       = 0x04,
    SI_TPG_FMT_VID_1080_50_10bpp      = 0x05,
    SI_TPG_FMT_VID_1080_50_12bpp      = 0x06,
    SI_TPG_FMT_VID_1080_60_8bpp       = 0x08,
    SI_TPG_FMT_VID_1080_60_10bpp      = 0x09,
    SI_TPG_FMT_VID_1080_60_12bpp      = 0x0A

} tpgVideoFormat_t;


typedef enum tpgVideoPattern
{
    SI_ETPG_PATTERN_RED          = 0x00,
    SI_ETPG_PATTERN_GREEN        = 0x01,
    SI_ETPG_PATTERN_BLUE         = 0x02,
    SI_ETPG_PATTERN_BLACK        = 0x03,
    SI_ETPG_PATTERN_WHITE        = 0x04,
    SI_ETPG_PATTERN_RAMP         = 0x05,
    SI_ETPG_PATTERN_CHESS        = 0x06,
    SI_ETPG_PATTERN_COLORBAR     = 0x07,

} tpgVideoPattern_t;


typedef enum tpgClockSource
{
    SI_TPG_CLK_MAIN_PIPE              = 0x00,
    SI_TPG_CLK_EXTERNAL               = 0x01,
    SI_TPG_CLK_ROV_PIPE               = 0x02,

    SI_TPG_CLK_XCLK                   = 0x04,
    SI_TPG_CLK_P0                     = 0x05,
    SI_TPG_CLK_P1                     = 0x06

} tpgClockSource_t;

//-------------------------------------------------------------------------------------------------
// TPG API functions
//-------------------------------------------------------------------------------------------------

void   SiiDrvTpgVideoFormatSet(tpgVideoFormat_t format, tpgClockSource_t clockSrc);
void   SiiDrvTpgEnable(bool_t isEnabled);
void   SiiDrvExtTpgEnable(bool_t isEnabled);
void   SiiDrvExtTpgPatternSet(tpgVideoPattern_t pattern);

#endif // SI_DRV_TPG_H
