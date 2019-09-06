//******************************************************************************
// !file    si_drv_arc_internal.h
// !brief   Silicon Image ARC driver internal header
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_ARC_INTERNAL_H__
#define __SI_DRV_ARC_INTERNAL_H__
#include "si_drv_internal.h"
#include "si_drv_device.h"
#include "si_regs_arc953x.h"

//------------------------------------------------------------------------------
//  ARC Driver Instance Data
//------------------------------------------------------------------------------

typedef struct _ArcDrvInstanceData_t
{
    int         structVersion;
    int         instanceIndex;
    int         lastResultCode;         // Contains the result of the last API function called
    uint16_t    statusFlags;

}   ArcDrvInstanceData_t;

extern ArcDrvInstanceData_t arcDrvInstance[SII_NUM_ARC];
extern ArcDrvInstanceData_t *pDrvArc;

#endif  // __SI_DRV_ARC_INTERNAL_H__
