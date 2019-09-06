//***************************************************************************
//!file     si_drv_ipv_internal.h
//!brief    SiI9535 InstaPrevue driver functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_IPV_INTERNAL_H__
#define __SI_DRV_IPV_INTERNAL_H__

#include "si_ipv_config.h"
#include "si_regs_ipv953x.h"
#include "si_drv_ipv.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  IPV Driver Instance Data
//------------------------------------------------------------------------------

typedef struct _IpvDrvInstanceData_t
{
    int                     structVersion;
    int                     instanceIndex;
    SiiDrvIpvResultCodes_t  lastResultCode;     // Contains the result of the last API function called
    uint16_t                statusFlags;

    SiiIpvMode_t            ipvMode;
    uint_t                  enableMask;
    bool_t                  interruptPending;
    bool_t                  isEnabled;
    bool_t                  wasEnabled;         // Saves state during main port unstable periods.
    bool_t                  isAnimationEnabled;

    uint8_t                 alphaStep[IPV_WINDOW_COUNT];
    uint8_t                 pipSpSource;

}   IpvDrvInstanceData_t;

extern IpvDrvInstanceData_t drvIpvInstance[SII_NUM_IPV];
extern IpvDrvInstanceData_t *pDrvIpv;

//-------------------------------------------------------------------------------
//  Internal Driver Function Prototypes
//-------------------------------------------------------------------------------

void    SiiDrvIpvProcessInterrupts( void );
int_t   SiiDrvIpvActiveWindowsGet( void );
int_t   SiiDrvMainPipePortGet( void );

#endif  // __SI_DRV_IPV_INTERNAL_H__



