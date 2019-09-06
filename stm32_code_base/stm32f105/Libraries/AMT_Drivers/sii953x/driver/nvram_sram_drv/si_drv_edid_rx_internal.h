//***************************************************************************
//!file     si_drv_edid_rx_internal.h
//!brief    Silicon Image NVRAM and SRAM driver.  Contains functions for
//          reading and writing Silicon Image device NVRAM and port SRAMS
//          as well as boot data.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_EDIDRX_INTERNAL_H__
#define __SI_DRV_EDIDRX_INTERNAL_H__
#include "si_common.h"
#include "si_drv_edid_rx_config.h"

//------------------------------------------------------------------------------
//  EDID RX driver Instance Data
//------------------------------------------------------------------------------

typedef struct
{
    int                         structVersion;
    int                         instanceIndex;
    SiiDrvNvramResultCodes_t    lastResultCode;
    uint16_t                    statusFlags;

    SiiNvramEdidMode_t          edidMode;       // Legacy or Two-EDID mode
}   NvramDrvInstanceData_t;


#endif      // __SI_DRV_EDIDRX_INTERNAL_H__
