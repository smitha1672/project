//***************************************************************************
//!file     si_drv_cra_internal.h
//!brief    Silicon Image CRA internal driver functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CRA_DRV_INTERNAL_H__
#define __SI_CRA_DRV_INTERNAL_H__

//-------------------------------------------------------------------------------
// CRA Enums and manifest constants
//-------------------------------------------------------------------------------

typedef enum _SiiDrvCraError_t
{
    RESULT_CRA_SUCCESS,             // Success result code
    RESULT_CRA_FAIL,                // General Failure result code
    RESULT_CRA_INVALID_PARAMETER,   // One or more invalid parameters
} SiiDrvCraError_t;

//------------------------------------------------------------------------------
//  CRA Driver Instance Data
//------------------------------------------------------------------------------

typedef struct _CraInstanceData_t
{
    int                 structVersion;
    int                 instanceIndex;
    SiiDrvCraError_t    lastResultCode;     // Contains the result of the last API function called
    uint16_t            statusFlags;
}	CraInstanceData_t;

extern CraInstanceData_t craInstance;


#endif // __SI_CRA_DRV_INTERNAL_H__
