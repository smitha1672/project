//***************************************************************************
//!file     sk_app_msw_demo.c
//!brief    Matrix Switch Demo
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_timer.h"

#include "sk_application.h"
#include "si_drv_switch.h"


//-------------------------------------------------------------------------------------------------
//! @brief      Select Matrix Switch Both Zones two sources
//-------------------------------------------------------------------------------------------------
void SkAppMswDemoSetSources ( int_t portZone1, int_t portZone2 )
{
    SiiDrvSwitchZoneTrackingSet(portZone1 == portZone2);
    app.newSource[0] = portZone1;
    app.newSource[1] = portZone2;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Demonstrate the Matrix switch capabilities
//-------------------------------------------------------------------------------------------------
static char *pMswDemoMenu [] =
{
    "\n Matrix Switch Demo",
    " --------",
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    "1 - Single source for both Zones",
    "2 - Separate sources for Zone 1 and Zone 2",
    NULL
};

bool_t SkAppMswDemo ( RC5Commands_t key )
{
    int_t   port, portA, portB;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu ( pMswDemoMenu );
            break;

        case RC5_0:
            pApp->demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;

        case RC5_1:
            PrintAlways( "Both Zones (single source):\n" );
            DEBUG_PRINT( MSG_ALWAYS, " Port number (0-%d): \n", SII_INPUT_PORT_COUNT - 1 );
            port = SkAppIrGetSingleDigit( SII_INPUT_PORT_COUNT - 1 );

            SkAppMswDemoSetSources( port, port );
            break;

        case RC5_2:
            PrintAlways( "Both Zones (separate sources):\n" );
            DEBUG_PRINT( MSG_ALWAYS, " Zone 1 Port number (0-%d): \n", SII_INPUT_PORT_COUNT - 1 );
            portA = SkAppIrGetSingleDigit( SII_INPUT_PORT_COUNT - 1 );
            DEBUG_PRINT( MSG_ALWAYS, " Zone 2 Port number (0-%d): \n", SII_INPUT_PORT_COUNT - 1 );
            portB = SkAppIrGetSingleDigit( SII_INPUT_PORT_COUNT - 1 );

            SkAppMswDemoSetSources( portA, portB );
            break;

        default:
            break;
    }

    return( false );
}


