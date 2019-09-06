//***************************************************************************
//!file     si_osd_component.c
//!brief    Silicon Image OSD Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_debug.h"
#include "si_osd_internal.h"

//------------------------------------------------------------------------------
//  OSD Component Instance Data
//------------------------------------------------------------------------------

OsdInstanceData_t osdInstance[1];
OsdInstanceData_t *pOsd = &osdInstance[0];

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the result of the last call to an OSD component function.
//! @return     SiiOsdResultCodes_t result code.
//! @retval     SII_OSD_SUCCESS: Success
//! @retval     SII_OSD_ERR_FAIL: General failure
//-------------------------------------------------------------------------------------------------
SiiOsdResultCodes_t SiiOsdGetLastResult( void )
{
    return( pOsd->lastResultCode );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize the OSD component
//! @return     Success or fail result
//! @retval     true: initialization successful
//! @retval     false: initialization failed
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdInitialize ( int functionCount, SiiOsdUserFunction_t *pUserFuncs )
{
    // Initialize the internal data structures
    memset( pOsd, 0, sizeof( OsdInstanceData_t ));

    if ( SiiDrvOsdInitialize())
    {
        pOsd->pUserFuncs    = pUserFuncs;
        pOsd->functionCount = functionCount;
        pOsd->focusWinRef   = -1;
        pOsd->fontWidth     = SiiDrvOsdGetFontWidth();
        pOsd->fontHeight    = SiiDrvOsdGetFontHeight();
        return( true );
    }

    return( false );
}

