//***************************************************************************
//!file     si_ipv_internal.h
//!brief    Silicon Image InstaPrevue Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_IPV_INTERNAL_H__
#define __SI_IPV_INTERNAL_H__
#include "si_device_config.h"
#include "si_ipv_component.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  IPV Component Instance Data
//------------------------------------------------------------------------------

typedef struct _IpvInstanceData_t
{
    int                 structVersion;
    int                 instanceIndex;
    SiiIpvResultCodes_t lastResultCode;     // Contains the result of the last API function called
    uint16_t            statusFlags;

    bool_t              isEnabled;
    SiiIpvTheme_t       lastTheme;
    SiiIpvTheme_t       currentTheme;
    SiiWinCoords_t      currentResolution;
    bool_t              currentIsInterlaced;
    SiiIpvWindowSpecs_t windowSpecs;
    uint8_t             borderColors[SiiIpvBorderColorCount][YCBCR_LEN];    // Display boundary enables for each window type
    SiiWinCoords_t      pipWindow;
    SiiWinSize_t        pipSizes;
    bool_t              highLightEnabled;
    int_t               highLightIndex;
    int_t               highLightHoriz;
    int_t               highLightVert;

    bool_t              alphaSteppingEnabled;
    bool_t              isAlphaStepping;
    int_t               alphaStepRemainingDelay;
    uint8_t             alphaCurrentValue[IPV_WINDOW_COUNT];

    bool_t              allowBounce;
    bool_t              isBounceEffect;
    int_t               bounceStep;

}   IpvInstanceData_t;

extern IpvInstanceData_t ipvInstance[SII_NUM_IPV];
extern IpvInstanceData_t *pIpv;

//------------------------------------------------------------------------------
// Data
//------------------------------------------------------------------------------


#endif // __SI_IPV_INTERNAL_H__
