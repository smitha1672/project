//***************************************************************************
//!file     sk9589_app_thx.c
//!brief    Wraps board and device functions for the THX application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "sk_application.h"
#include "si_thx_component.h"
#include "string.h"

#define THX_TIME_INTERVAL 		900
#define THX_NO_PKTS 			2			// no. of specific THX VSIFs we are looking for

// Array of THX specific VSIF headers
const uint8_t thxVSIFHdrs[THX_NO_PKTS * THX_PKT_HDR_LEN] =
{
		0xFA, 0x12, 0x00, 0x01,
		0xFA, 0x12, 0x00, 0x0B
};

//------------------------------------------------------------------------------
// Function:    SkAppThxTimerSet
// Description: Elapsed timers for THX use
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
static void ThxTimerSet ( clock_time_t msDelay  )
 {
    pApp->msThxDelay = msDelay;
    pApp->msThxStart = SiiOsTimerTotalElapsed();
 }

static bool_t ThxTimerElapsed ( void )
 {
    clock_time_t thisTime = SiiOsTimerTotalElapsed();
    return(( thisTime - pApp->msThxStart) >= pApp->msThxDelay);
 }

//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitThx
// Description: Initialize THX VSIF headers that we will look for in incoming info frames
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

bool_t SkAppDeviceInitThx ( void )
{
    if ( pApp->isThxDemo )
    {
        SiiThxInitialize(&thxVSIFHdrs[0], THX_NO_PKTS);
    }

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskThx
// Description: This task is periodically called from the main loop
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskThx ( void )
{
    if ( !ThxTimerElapsed())
    {
        return;
    }
    ThxTimerSet(THX_TIME_INTERVAL);

    // call the main THX component layer routine to analyze the incoming infoframes
    SiiThxAnalyzePkt();
}
