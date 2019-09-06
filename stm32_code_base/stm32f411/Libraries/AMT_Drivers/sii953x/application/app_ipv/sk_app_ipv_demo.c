//***************************************************************************
//!file     sk_app_ipv_demo.c
//!brief    InstaPrevue Demo
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"

#if INC_IPV
//------------------------------------------------------------------------------
// Function:    SkAppIvDemo
// Description: InstaPrevue demo command options
// Returns:
//------------------------------------------------------------------------------
static char *pIpvDemoMenu [] =
{
    "\nInstaPrevue Demo",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    "1 - Top Center",
    "2 - Bottom Center",
    "3 - Left Middle",
    "4 - Right Middle",
    "5 - Mode: All Ports",
    "6 - Mode: Only Active Ports",
    "7 - Mode: Picture-In-Picture",
    "8 - InstaPrevue Off",
    NULL
};

bool_t SkAppIpvDemo ( RC5Commands_t key )
{
    int_t   port;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pIpvDemoMenu );
            break;

        case RC5_0:
            pApp->demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;
        case RC5_1:
            SiiIpvModeSet( SiiIpvModeALL );
            SkAppIpvEnable( false );
            SiiIpvThemeSet( SiiIpvTOP_CENTER );
            SkAppIpvEnable( true );
            app.ipvTheme = SiiIpvTOP_CENTER;
            break;
        case RC5_2:
            SiiIpvModeSet( SiiIpvModeALL );
            SkAppIpvEnable( false );
            SiiIpvThemeSet( SiiIpvBOTTOM_CENTER );
            SkAppIpvEnable( true );
            app.ipvTheme = SiiIpvBOTTOM_CENTER;
            break;
        case RC5_3:
            SiiIpvModeSet( SiiIpvModeALL );
            SkAppIpvEnable( false );
            SiiIpvThemeSet( SiiIpvLEFT_MIDDLE );
            SkAppIpvEnable( true );
            app.ipvTheme = SiiIpvLEFT_MIDDLE;
            break;
        case RC5_4:
            SiiIpvModeSet( SiiIpvModeALL );
            SkAppIpvEnable( false );
            SiiIpvThemeSet( SiiIpvRIGHT_MIDDLE );
            SkAppIpvEnable( true );
            app.ipvTheme = SiiIpvRIGHT_MIDDLE;
            break;
        case RC5_5:
            SiiIpvModeSet( SiiIpvModeALL );
            break;
        case RC5_6:
            SiiIpvModeSet( SiiIpvModeACTIVE );
            break;
        case RC5_7:

            DEBUG_PRINT( MSG_ALWAYS, " PIP Port #(0-%d): \n", SII_INPUT_PORT_COUNT - 1 );

            port = SkAppIrGetSingleDigit( SII_INPUT_PORT_COUNT - 1 );
            SkAppIpvEnable( false );
            SkAppIpvPipModeEnable( port );
            break;
        case RC5_8:
            SkAppIpvEnable( false );
            break;
        default:
            break;
    }

    return( false );
}
#endif
